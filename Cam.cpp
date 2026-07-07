#include "Cam.h"

Cam::Cam(int w,int h)
{
    screenH = h;screenW = w;
    camera.target = {0.0f,0.0f};
    camera.offset = {screenW/2.0f,screenH/2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 50.0f;
}

void Cam::update(bool blocked)
{
    if(blocked)return;
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 change = GetMouseDelta();
        change.x /= camera.zoom;
        change.y /= camera.zoom;
        camera.target.x -= change.x;
        camera.target.y -= change.y;
    }
    float scroll = GetMouseWheelMove();
    if(scroll != 0.0f)
    {
        camera.zoom *= (scroll>0)?1.1f:(1.0f/1.1f);
        if(camera.zoom < 1.0f)camera.zoom = 1.0f;
        if(camera.zoom>500.0f)camera.zoom = 500.0f;
    }
}

Camera2D &Cam::get()
{
    return camera;
}