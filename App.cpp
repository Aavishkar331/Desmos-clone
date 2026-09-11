#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif
#include "App.h"
App::App()
{
    
    #ifdef PLATFORM_WEB
        int w = EM_ASM_INT({ return window.innerWidth; });
        int h = EM_ASM_INT({ return window.innerHeight; });
        InitWindow(w, h, "Desmos Clone");
    #else
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
        InitWindow(0, 0, "Desmos Clone");
    #endif
    screenW = GetScreenWidth();
    screenH = GetScreenHeight();
    camera =  Cam(screenW,screenH);
    grid = Grid(camera, screenW, screenH);
    graphs.push_back(Graph(camera, screenW, screenH, "sin(x)", RED));
    SetTargetFPS(120);
}
void App::step() {
    update();
    draw();
}
void App::run() {
    #ifdef PLATFORM_WEB
        emscripten_set_main_loop_arg([](void* arg) {
            static_cast<App*>(arg)->step();
        }, this, 0, 1);
    #else
        while (!WindowShouldClose()) {
            update();
            draw();
        }
        cleanup();
    #endif
}
void App::update()
{
    camera.update(panel.isMouseOver());
    panel.update();
    if (panel.hasChanged())
    {
        graphs[panel.changedIndex()].set(panel.changedExpression());
    }
    if (panel.entryAdded()) 
    {
        int i = panel.count()-1;
        graphs.push_back(Graph(camera, screenW, screenH, "", panel.colorAt(i)));
    }
    if (panel.entryDeleted()) 
    {
        int i = panel.deletedIndex();
        if (i < (int)graphs.size()) 
        {
            graphs.erase(graphs.begin() + i);
        }
    }   
}

// Finds intersections between two explicit graphs and appends them to `out`
// as world-space points (Y already negated to match Raylib's coord system).
static void collectIntersections(Graph& a, Graph& b, Camera2D& cam,
                                  int screenW, vector<Vector2>& out)
{
    float left  = cam.target.x - (screenW / 2.0f) / cam.zoom;
    float right = cam.target.x + (screenW / 2.0f) / cam.zoom;
    float step  = 1.0f / cam.zoom;

    float prevDiff = a.evaluate(left) - b.evaluate(left);

    for (float x = left + step; x <= right; x += step) {
        float diff = a.evaluate(x) - b.evaluate(x);

        if (prevDiff * diff < 0.0f) {   // sign change → crossed here
            float lo = x - step, hi = x;
            for (int i = 0; i < 16; i++) {
                float mid = (lo + hi) * 0.5f;
                float d   = a.evaluate(mid) - b.evaluate(mid);
                if ((a.evaluate(lo) - b.evaluate(lo)) * d < 0.0f) hi = mid;
                else lo = mid;
            }
            float xi = (lo + hi) * 0.5f;
            float yi = -((a.evaluate(xi) + b.evaluate(xi)) * 0.5f); // world Y (flipped)
            out.push_back({xi, yi});
        }

        prevDiff = diff;
    }
}


void App::draw()
{
    Vector2 worldMouse = GetScreenToWorld2D(GetMousePosition(), camera.get());
    float zoom = camera.get().zoom;

    // ── Collect all visible intersection points (pure math, no drawing yet) ──
    vector<Vector2> intersections;
    for (int i = 0; i < (int)graphs.size(); i++)
    for (int j = i + 1; j < (int)graphs.size(); j++)
        if (graphs[i].isExplicit() && graphs[j].isExplicit())
            collectIntersections(graphs[i], graphs[j], camera.get(), screenW, intersections);

    // ── Check if mouse is within 8 screen pixels of any intersection ──────────
    // 8 screen pixels = 8/zoom world units
    int snapIdx = -1;
    for (int i = 0; i < (int)intersections.size(); i++) {
        float dx = worldMouse.x - intersections[i].x;
        float dy = worldMouse.y - intersections[i].y;
        if (sqrtf(dx*dx + dy*dy) * zoom < 8.0f) {
            snapIdx = i;
            break;
        }
    }

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(camera.get());
    grid.draw();
    for (auto &g : graphs) g.draw();

    // Curve hover — suppressed when snapping to an intersection point
    float outY = 0.0f;
    int hoveredIdx = -1;
    if (snapIdx < 0) {
        for (int i = 0; i < (int)graphs.size(); i++) {
            if (graphs[i].drawHover(worldMouse, zoom, outY))
                hoveredIdx = i;
        }
    }

    // Draw all intersection dots (small, 4px radius)
    for (auto &p : intersections)
        DrawCircleV(p, 4.0f / zoom, WHITE);

    // Snap highlight: slightly larger white ring when within 8px
    if (snapIdx >= 0)
        DrawCircleV(intersections[snapIdx], 7.0f / zoom, WHITE);

    EndMode2D();
    for (auto &g : graphs) g.drawImplicit();

    // ── Tooltip: intersection snap takes priority over curve hover ────────────
    if (snapIdx >= 0) {
        Vector2 mouse = GetMousePosition();
        char label[64];
        float mathY = -intersections[snapIdx].y;   // un-flip Y for display
        snprintf(label, sizeof(label), "(%.2f, %.2f)",
                 intersections[snapIdx].x, mathY);
        DrawRectangle(mouse.x + 12, mouse.y - 8,
                      MeasureText(label, 16) + 8, 24, Fade(BLACK, 0.7f));
        DrawText(label, mouse.x + 16, mouse.y - 4, 16, WHITE);
    } else if (hoveredIdx >= 0) {
        Vector2 mouse = GetMousePosition();
        char label[64];
        snprintf(label, sizeof(label), "(%.2f, %.2f)", worldMouse.x, outY);
        DrawRectangle(mouse.x + 12, mouse.y - 8,
                      MeasureText(label, 16) + 8, 24, Fade(BLACK, 0.7f));
        DrawText(label, mouse.x + 16, mouse.y - 4, 16, WHITE);
    }

    panel.draw();
    EndDrawing();
}

void App::cleanup()
{
    CloseWindow();
}

int main()
{
    App app;
    app.run();
    return 0;
}