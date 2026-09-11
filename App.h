#pragma once
#include "raylib.h"
#include "Cam.h"
#include "Grid.h"
#include "Graph.h"
// #include "UI.h"
#include "EquationPanel.h"
#include <vector>

class App
{
    private:
        int screenW,screenH;
        Cam camera;
        Grid grid;
        vector<Graph> graphs;
        EquationPanel panel;
        void update();
        void draw();

    public:
        App();
        void run();
        void step();
        void cleanup();
};