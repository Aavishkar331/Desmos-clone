#pragma once
#include "Cam.h"
#include "Expression.h"
#include "raylib.h"
#include <string>

using namespace std;

class Graph
{
    private:
        Cam *camera = nullptr;
        int screenW,screenH;
        Expression expression;
        Color color;
        RenderTexture2D implicitTex;
        bool implicitDirty;
        Vector2 lastTarget;
        float   lastZoom;

    public:
        Graph() = default;
        Graph(Cam &cam,int w,int h,const string &exprStr,Color color);
        Graph(Graph&& other) noexcept;
        Graph& operator=(Graph&& other) noexcept;
        bool expressionValid();
        void draw();
        void drawImplicit();
        void set(const string &exprStr);
        bool drawHover(Vector2 worldMouse, float zoom, float& outY);
        float evaluate(float x) const;
        bool  isExplicit() const;
        ~Graph();
};