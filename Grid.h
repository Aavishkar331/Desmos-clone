#pragma once
#include "raylib.h"
#include "Cam.h"

class Grid
{
    private:
        Cam *camera;
        int screenW,screenH;

    public:
        Grid() = default;
        Grid(Cam &cam,int w,int h);
        void draw();
};