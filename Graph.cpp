#include "Graph.h"
#include <cmath>

using namespace std;

Graph::Graph(Cam &cam,int w,int h,const string &exprStr,Color c)
{
    implicitTex   = LoadRenderTexture(w, h);
    implicitDirty = true;
    lastTarget    = {0, 0};
    lastZoom      = 0.0f;
    camera = &cam;
    screenW = w;
    screenH = h;
    expression.set(exprStr);
    color = c;
}

void Graph::draw()
{
    if (!expression.isValid() || expression.isImplicit()) return;
    Camera2D c = camera->get();
    float left  = c.target.x - (screenW / 2.0f) / c.zoom;
    float right = c.target.x + (screenW / 2.0f) / c.zoom;

    float step = 1.0f / c.zoom;

    Vector2 prev = { left, -expression.evaluate(left) };
    for (float x = left + step; x <= right; x += step)
    {
        Vector2 curr = { x, -expression.evaluate(x) };
        if (isfinite(prev.y) && isfinite(curr.y))
            DrawLineV(prev, curr, color);
        prev = curr;
    }
}

void Graph::set(const std::string& exprStr)
{
    expression.set(exprStr);
    implicitDirty = true;
}

bool Graph::expressionValid()
{
    return expression.isValid();
}

Graph::Graph(Graph&& other) noexcept
    : camera(other.camera),
      screenW(other.screenW),
      screenH(other.screenH),
      expression(std::move(other.expression)),
      color(other.color),
      implicitTex(other.implicitTex),
      implicitDirty(other.implicitDirty),
      lastTarget(other.lastTarget),
      lastZoom(other.lastZoom)
{
    other.camera = nullptr;  // stops source destructor from unloading the texture
}

Graph& Graph::operator=(Graph&& other) noexcept {
    if (this != &other) {
        if (camera) UnloadRenderTexture(implicitTex);
        camera        = other.camera;
        screenW       = other.screenW;
        screenH       = other.screenH;
        expression    = std::move(other.expression);
        color         = other.color;
        implicitTex   = other.implicitTex;
        implicitDirty = other.implicitDirty;
        lastTarget    = other.lastTarget;
        lastZoom      = other.lastZoom;
        other.camera  = nullptr;
    }
    return *this;
}

void Graph::drawImplicit()
{
    if (!expression.isValid() || !expression.isImplicit()) return;
    Camera2D c = camera->get();

    // check if camera moved since last render
    if (c.target.x != lastTarget.x || c.target.y != lastTarget.y || c.zoom != lastZoom)
        implicitDirty = true;

    if (implicitDirty) {
        float threshold = 1.5f / c.zoom;
        BeginTextureMode(implicitTex);
        ClearBackground({0, 0, 0, 0});
        for (int py = 0; py < screenH; py += 2) {
            for (int px = 0; px < screenW; px += 2) {
                Vector2 world = GetScreenToWorld2D({(float)px, (float)py}, c);
                float my  = -world.y;   // convert Raylib Y (down+) to math Y (up+)
                float val = expression.evaluate2D(world.x, my);
                float eps = 1.0f / c.zoom;
                float gx  = (expression.evaluate2D(world.x + eps, my) - val) / eps;
                float gy  = (expression.evaluate2D(world.x, my + eps) - val) / eps;
                float grad = sqrtf(gx*gx + gy*gy);
                float dist = (grad > 1e-6f) ? fabsf(val) / grad : fabsf(val);
                if (dist < 4.0f / c.zoom)
                    DrawRectangle(px, py, 2, 2, color);
            }
        }
        EndTextureMode();
        lastTarget    = c.target;
        lastZoom      = c.zoom;
        implicitDirty = false;
    }

    // RenderTexture is flipped vertically in Raylib
    DrawTextureRec(implicitTex.texture,{0, 0, (float)screenW, -(float)screenH},{0, 0}, WHITE);
}

bool Graph::drawHover(Vector2 worldMouse, float zoom, float& outY) {
    if (!expression.isValid() || expression.isImplicit()) return false;

    float yMath  = expression.evaluate(worldMouse.x);
    float yWorld = -yMath;                                    // flip to match draw()
    float screenDist = fabsf((worldMouse.y - yWorld) * zoom);

    if (screenDist < 8.0f) {
        DrawCircleV({worldMouse.x, yWorld}, 5.0f / zoom, color);
        outY = yMath;       // tooltip shows math value (positive = up)
        return true;
    }
    return false;
}
float Graph::evaluate(float x) const {
    return expression.evaluate(x);
}

bool Graph::isExplicit() const {
    return expression.isValid() && !expression.isImplicit();
}

Graph::~Graph() { if (camera) UnloadRenderTexture(implicitTex); }