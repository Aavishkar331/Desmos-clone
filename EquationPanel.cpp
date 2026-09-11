#include "EquationPanel.h"
#include <algorithm>
#include <cstring>
#include <cmath>

using namespace std;

// ── Web/mobile keyboard support ─────────────────────────────────────────────
#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>

static void webShowInput(const std::string& text, int screenY)
{
    EM_ASM({
        var inp = document.getElementById('_desmos_eq_inp');
        if (!inp) return;
        inp.style.top   = $1 + 'px';
        inp.value       = UTF8ToString($0);
        var L = inp.value.length;
        inp.setSelectionRange(L, L);
        inp.focus();
        window._desmosEqText    = inp.value;
        window._desmosEqChanged = 0;
    }, text.c_str(), screenY);
}

static void webHideInput()
{
    EM_ASM({
        var inp = document.getElementById('_desmos_eq_inp');
        if (inp) {
            inp.style.top = '-200px';
            inp.blur();
        }
        window._desmosEqChanged = 0;
    });
}

static bool webInputChanged()
{
    return EM_ASM_INT({ return window._desmosEqChanged || 0; }) != 0;
}

static std::string webGetInputText()
{
    const char* s = emscripten_run_script_string("window._desmosEqText || ''");
    return std::string(s ? s : "");
}

static void webClearChanged()
{
    EM_ASM({ window._desmosEqChanged = 0; });
}
#endif
// ─────────────────────────────────────────────────────────────────────────────

const Color EquationPanel::PALETTE[6] = 
{
    RED,
    {64,  156, 255, 255},
    {80,  220, 120, 255},
    {255, 200,  50, 255},
    {200, 100, 255, 255},
    {255, 140,  50, 255},
};

EquationPanel::EquationPanel()
{
    activeIdx = -1;
    changedIdx = -1;
    nextColorIdx = 1;
    changed = false;
    added = false;
    deleted = false;
    dragging = false;
    dragAnchor = 0;
    deletedIdx = -1;
#ifdef PLATFORM_WEB
    prevActiveIdx = -1;
#endif
    entries.push_back({"sin(x)", PALETTE[0], 6, -1, -1});
}

int EquationPanel::panelH() const 
{
    return PAD + (int)entries.size() * ROW + PAD + 34 + PAD;
}

int EquationPanel::rowY(int i) const 
{
    return PY + PAD + i * ROW;
}

int EquationPanel::charAtPixel(const EquationEntry &e, int px) const 
{
    int textX = PX + PAD + 22 + PAD;
    int offset = px - textX;
    if (offset <= 0) return 0;
    for (int i = 1; i <= (int)e.text.size(); i++) 
    {
        if (MeasureText(e.text.substr(0, i).c_str(), 18) >= offset)
        {
            return i - 1;
        }
    }
    return (int)e.text.size();
}

string EquationPanel::getSelection(const EquationEntry &e) const 
{
    if (e.selStart < 0 || e.selStart == e.selEnd) return "";
    int lo = min(e.selStart, e.selEnd);
    int hi = max(e.selStart, e.selEnd);
    return e.text.substr(lo, hi - lo);
}

void EquationPanel::deleteSelection(EquationEntry &e) 
{
    if (e.selStart < 0 || e.selStart == e.selEnd) return;
    int lo = min(e.selStart, e.selEnd);
    int hi = max(e.selStart, e.selEnd);
    e.text.erase(lo, hi - lo);
    e.cursor   = lo;
    e.selStart = -1;
    e.selEnd   = -1;
}

void EquationPanel::doTextInput(EquationEntry &e) 
{
    bool ctrl  = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT)   || IsKeyDown(KEY_RIGHT_SHIFT);
    bool hasSel = (e.selStart >= 0 && e.selStart != e.selEnd);
    int  sz = (int)e.text.size();

    if (ctrl && IsKeyPressed(KEY_A)) 
    {
        e.selStart = 0; e.selEnd = sz; e.cursor = sz;
        return;
    }
    if (ctrl && IsKeyPressed(KEY_C)) 
    {
        SetClipboardText(getSelection(e).c_str());
        return;
    }
    if (ctrl && IsKeyPressed(KEY_X)) 
    {
        SetClipboardText(getSelection(e).c_str());
        deleteSelection(e);
        changed = true; changedIdx = activeIdx;
        return;
    }
    if (ctrl && IsKeyPressed(KEY_V)) 
    {
        deleteSelection(e);
        const char *clip = GetClipboardText();
        if (clip && strlen(clip) > 0) 
        {
            e.text.insert(e.cursor, clip);
            e.cursor += (int)strlen(clip);
        }
        e.selStart = -1;
        changed = true; changedIdx = activeIdx;
        return;
    }

    if (IsKeyPressed(KEY_LEFT)) 
    {
        if (hasSel && !shift) 
        {
            e.cursor = min(e.selStart, e.selEnd);
            e.selStart = -1;
        } 
        else if (e.cursor > 0) 
        {
            e.cursor--;
            if (shift) 
            {
                if (e.selStart < 0) e.selStart = e.cursor + 1;
                e.selEnd = e.cursor;
            } 
            else 
            {
                e.selStart = -1;
            }
        }
    }
    if (IsKeyPressed(KEY_RIGHT)) 
    {
        if (hasSel && !shift) 
        {
            e.cursor = max(e.selStart, e.selEnd);
            e.selStart = -1;
        } 
        else if (e.cursor < sz) 
        {
            e.cursor++;
            if (shift) 
            {
                if (e.selStart < 0) e.selStart = e.cursor - 1;
                e.selEnd = e.cursor;
            } 
            else 
            {
                e.selStart = -1;
            }
        }
    }
    if (IsKeyPressed(KEY_HOME)) 
    {
        if (shift) { if (e.selStart < 0) e.selStart = e.cursor; e.selEnd = 0; }
        else e.selStart = -1;
        e.cursor = 0;
    }
    if (IsKeyPressed(KEY_END)) 
    {
        if (shift) { if (e.selStart < 0) e.selStart = e.cursor; e.selEnd = sz; }
        else e.selStart = -1;
        e.cursor = sz;
    }

    if (IsKeyPressed(KEY_BACKSPACE)) 
    {
        if (hasSel) {
            deleteSelection(e);
        } else if (e.cursor > 0) {
            e.text.erase(e.cursor - 1, 1);
            e.cursor--;
        }
        changed = true; changedIdx = activeIdx;
    }
    if (IsKeyPressed(KEY_DELETE)) 
    {
        if (hasSel) {
            deleteSelection(e);
        } else if (e.cursor < sz) {
            e.text.erase(e.cursor, 1);
        }
        changed = true; changedIdx = activeIdx;
    }
    int c;
    while ((c = GetCharPressed()) > 0) 
    {
        deleteSelection(e);
        e.text.insert(e.cursor, 1, (char)c);
        e.cursor++;
        e.selStart = -1;
        changed = true; changedIdx = activeIdx;
    }
}

void EquationPanel::update() 
{
    changed = false; changedIdx = -1;
    added   = false;
    deleted = false; deletedIdx = -1;

    Vector2 mouse = GetMousePosition();
    int n = (int)entries.size();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
    {
        bool clickedPanel = false;

        for (int i = 0; i < n; i++) 
        {
            int ry = rowY(i);

            Rectangle delBtn = {(float)(PX + PW - 32), (float)(ry + 14), 22, 22};
            if (CheckCollisionPointRec(mouse, delBtn)) 
            {
                entries.erase(entries.begin() + i);
                if (activeIdx == i) activeIdx = -1;
                else if (activeIdx > i) activeIdx--;
                deleted = true; deletedIdx = i;
                return;
            }

            if (mouse.x >= PX && mouse.x <= PX + PW && mouse.y >= ry && mouse.y <= ry + ROW) 
            {
                clickedPanel = true;
                activeIdx = i;
                int ci = charAtPixel(entries[i], (int)mouse.x);
                entries[i].cursor   = ci;
                entries[i].selStart = ci;
                entries[i].selEnd   = ci;
                dragging   = true;
                dragAnchor = ci;
            }
        }

        int addY = PY + PAD + n * ROW + PAD;
        if (mouse.x >= PX && mouse.x <= PX + PW && mouse.y >= addY && mouse.y <= addY + 34) 
        {
            Color c = PALETTE[nextColorIdx % 6];
            nextColorIdx++;
            entries.push_back({"", c, 0, -1, -1});
            activeIdx = (int)entries.size() - 1;
            added = true;
            return;
        }

        if (!clickedPanel && mouse.x >= PX && mouse.x <= PX + PW && mouse.y >= PY && mouse.y <= PY + panelH()) 
        {
        } 
        else if (mouse.x < PX || mouse.x > PX + PW || mouse.y < PY || mouse.y > PY + panelH()) 
        {
            activeIdx = -1;
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && dragging && activeIdx >= 0) 
    {
        int ci = charAtPixel(entries[activeIdx], (int)mouse.x);
        entries[activeIdx].cursor = ci;
        entries[activeIdx].selStart = min(dragAnchor, ci);
        entries[activeIdx].selEnd   = max(dragAnchor, ci);
        if (entries[activeIdx].selStart == entries[activeIdx].selEnd)
        {
            entries[activeIdx].selStart = -1;
        }
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) dragging = false;

    if (IsKeyPressed(KEY_TAB) && n > 0) 
    {
        activeIdx = (activeIdx + 1) % n;
    }

#ifdef PLATFORM_WEB
    // Sync HTML input overlay with the active row (no-ops if overlay absent)
    if (activeIdx != prevActiveIdx)
    {
        if (activeIdx >= 0 && activeIdx < (int)entries.size())
        {
            webShowInput(entries[activeIdx].text, rowY(activeIdx) + 6);
        }
        else
        {
            webHideInput();
        }
        prevActiveIdx = activeIdx;
    }

    if (activeIdx >= 0 && webInputChanged())
    {
        std::string newText = webGetInputText();
        if (newText != entries[activeIdx].text)
        {
            entries[activeIdx].text     = newText;
            entries[activeIdx].cursor   = (int)newText.size();
            entries[activeIdx].selStart = -1;
            changed    = true;
            changedIdx = activeIdx;
        }
        webClearChanged();
    }

    // Fallback: desktop-web keyboard input via raylib directly.
    if (activeIdx >= 0 && activeIdx < (int)entries.size())
    {
        doTextInput(entries[activeIdx]);
    }
#else
    if (activeIdx >= 0 && activeIdx < (int)entries.size())
    {
        doTextInput(entries[activeIdx]);
    }
#endif
}

void EquationPanel::draw() const 
{
    int h = panelH();
    int n = (int)entries.size();

    DrawRectangle(PX, PY, PW, h, {15, 20, 30, 215});
    DrawRectangleLines(PX, PY, PW, h, {60, 80, 110, 180});

    for (int i = 0; i < n; i++) 
    {
        const EquationEntry &e = entries[i];
        int ry = rowY(i);
        bool active = (i == activeIdx);

        if (active)
        {
            DrawRectangle(PX + 2, ry + 2, PW - 4, ROW - 4, {35, 50, 70, 200});
        }

        DrawRectangle(PX + PAD, ry + 17, 14, 14, e.color);
        DrawRectangleLines(PX + PAD, ry + 17, 14, 14, {255, 255, 255, 60});

        int textX = PX + PAD + 22 + PAD;
        int textY = ry + ROW / 2 - 9;

        if (active && e.selStart >= 0 && e.selStart != e.selEnd) 
        {
            int lo = min(e.selStart, e.selEnd);
            int hi = max(e.selStart, e.selEnd);
            int x0 = MeasureText(e.text.substr(0, lo).c_str(), 18);
            int x1 = MeasureText(e.text.substr(0, hi).c_str(), 18);
            DrawRectangle(textX + x0, textY - 1, x1 - x0, 22, {70, 130, 200, 150});
        }

        if (e.text.empty()) 
        {
            DrawText("type expression...", textX, textY, 18, {80, 95, 115, 160});
        }
        else 
        {
            DrawText(e.text.c_str(), textX, textY, 18, WHITE);
        }

        if (active && fmod(GetTime(), 1.0) < 0.55) 
        {
            int cx = MeasureText(e.text.substr(0, e.cursor).c_str(), 18);
            DrawRectangle(textX + cx, textY - 2, 2, 22, WHITE);
        }

        DrawText("x", PX + PW - 26, ry + 15, 18, {180, 70, 70, 200});
    }

    int addY = PY + PAD + n * ROW + PAD;
    DrawRectangle(PX + PAD, addY, PW - 2 * PAD, 34, {35, 50, 70, 180});
    DrawText("+ add equation", PX + PW / 2 - MeasureText("+ add equation", 16) / 2, addY + 9, 16, {130, 170, 215, 210});
}

bool EquationPanel::isMouseOver() const 
{
    Vector2 m = GetMousePosition();
    return m.x >= PX && m.x <= PX+PW && m.y >= PY && m.y <= PY+panelH();
}
bool EquationPanel::hasChanged() const 
{
    return changed; 
}
int EquationPanel::changedIndex() const 
{
    return changedIdx; 
}
string EquationPanel::changedExpression() const 
{
    if (changedIdx < 0 || changedIdx >= (int)entries.size()) return "";
    return entries[changedIdx].text;
}
bool  EquationPanel::entryAdded()   const { return added; }
bool  EquationPanel::entryDeleted() const { return deleted; }
int   EquationPanel::deletedIndex() const { return deletedIdx; }
int   EquationPanel::count()        const { return (int)entries.size(); }
Color EquationPanel::colorAt(int i) const { return entries[i].color; }