#include "Grid.h"
#include <cmath>

Grid::Grid(Cam &cam,int w,int h)
{
    camera = &cam;
    screenW = w;
    screenH = h;
}

void Grid::draw()
{
    Camera2D c = camera->get();
    float left = c.target.x - (screenW/2.0f)/c.zoom;
    float right = c.target.x + (screenW/2.0f)/c.zoom;
    float top = c.target.y - (screenH/2.0f)/c.zoom;
    float bottom = c.target.y + (screenH/2.0f)/c.zoom;

    for (int x = (int)floorf(left); x <= (int)ceilf(right); x++)
    {
        DrawLineV({(float)x, top}, {(float)x, bottom}, DARKGRAY);
    }
    for (int y = (int)floorf(top); y <= (int)ceilf(bottom); y++)
    {
        DrawLineV({left, (float)y}, {right, (float)y}, DARKGRAY);
    }

    DrawLineV({left, 0}, {right, 0}, WHITE);
    DrawLineV({0, top}, {0, bottom}, WHITE);
}