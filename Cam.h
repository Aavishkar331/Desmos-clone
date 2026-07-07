#pragma once
#include "raylib.h"

class Cam
{
    private:
        Camera2D camera;
        int screenW,screenH;

    public:
        Cam() = default;
        Cam(int w,int h);
        void update(bool blocked = false);
        Camera2D &get();
};