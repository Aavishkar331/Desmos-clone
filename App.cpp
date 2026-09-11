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

static void drawIntersections(Graph& a, Graph& b, Camera2D& cam, int screenW) {
    float left  = cam.target.x - (screenW / 2.0f) / cam.zoom;
    float right = cam.target.x + (screenW / 2.0f) / cam.zoom;
    float step  = 1.0f / cam.zoom;

    float prevDiff = a.evaluate(left) - b.evaluate(left);

    for (float x = left + step; x <= right; x += step) {
        float diff = a.evaluate(x) - b.evaluate(x);

        if (prevDiff * diff < 0.0f) {   // sign change â crossed here
            // bisect to find precise x
            float lo = x - step, hi = x;
            for (int i = 0; i < 16; i++) {
                float mid = (lo + hi) * 0.5f;
                float d   = a.evaluate(mid) - b.evaluate(mid);
                if ((a.evaluate(lo) - b.evaluate(lo)) * d < 0.0f) hi = mid;
                else lo = mid;
            }
            float xi = (lo + hi) * 0.5f;
            float yi = -((a.evaluate(xi) + b.evaluate(xi)) * 0.5f); // world y (flipped)
            DrawCircleV({xi, yi}, 4.0f / cam.zoom, WHITE);
        }

        prevDiff = diff;
    }
}


void App::draw()
{
    Vector2 worldMouse = GetScreenToWorld2D(GetMousePosition(), camera.get());
    float zoom = camera.get().zoom;
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(camera.get());
    grid.draw();
    for (auto &g : graphs) g.draw();
    float outY;
    int hoveredIdx = -1;
    for (int i = 0; i < (int)graphs.size(); i++) {
        if (graphs[i].drawHover(worldMouse, zoom, outY))
            hoveredIdx = i;
    }
    for (int i = 0; i < (int)graphs.size(); i++)
    for (int j = i + 1; j < (int)graphs.size(); j++)
        if (graphs[i].isExplicit() && graphs[j].isExplicit())
            drawIntersections(graphs[i], graphs[j], camera.get(), screenW);
    EndMode2D();
    for (auto &g : graphs) g.drawImplicit();
    if (hoveredIdx >= 0) {
        Vector2 mouse = GetMousePosition();
        char label[64];
        snprintf(label, sizeof(label), "(%.2f, %.2f)", worldMouse.x, outY);
        DrawRectangle(mouse.x + 12, mouse.y - 8, MeasureText(label, 16) + 8, 24, Fade(BLACK, 0.7f));
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