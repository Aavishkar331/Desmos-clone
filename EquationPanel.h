#pragma once
#include "raylib.h"
#include <string>
#include <vector>

using namespace std;

struct EquationEntry {
    string text;
    Color       color;
    int         cursor;    // insertion point 0..text.size()
    int         selStart;  // -1 = no selection
    int         selEnd;
};

class EquationPanel {
public:
    static const int PX  = 10;   // panel left
    static const int PY  = 10;   // panel top
    static const int PW  = 310;  // panel width
    static const int ROW = 48;   // row height
    static const int PAD = 10;   // inner padding

    EquationPanel();
    void update();
    void draw() const;

    // called by App every frame
    bool        isMouseOver()        const;
    bool        hasChanged()         const;
    int         changedIndex()       const;
    string changedExpression()  const;
    bool        entryAdded()         const;
    bool        entryDeleted()       const;
    int         deletedIndex()       const;
    int         count()              const;
    Color       colorAt(int i)       const;

private:
    vector<EquationEntry> entries;
    int  activeIdx;
    bool changed;
    int  changedIdx;
    bool added;
    bool deleted;
    int  deletedIdx;
    int  nextColorIdx;
    bool dragging;
    int  dragAnchor;   // char index where drag started

    int  panelH()              const;
    int  rowY(int i)           const;
    int  charAtPixel(const EquationEntry &e, int px) const;
    void doTextInput(EquationEntry &e);
    void deleteSelection(EquationEntry &e);
    string getSelection(const EquationEntry &e) const;

    static const Color PALETTE[6];
};