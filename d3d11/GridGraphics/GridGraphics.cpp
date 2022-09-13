//
// Created by yuxuw on 9/13/2022.
//

#include "../pch.h"
#include "../Graphics.h"

class GridGraphics : public Graphics {
public:
    GridGraphics(HINSTANCE, UINT, UINT);
    void InitGraphics() override;
    void UpdateGraphics() override;
    void RenderGraphics() override;
};

GridGraphics::GridGraphics(HINSTANCE hInstance, UINT width, UINT height
) : Graphics(hInstance, width, height) { }

void GridGraphics::InitGraphics() {
}

void GridGraphics::UpdateGraphics() {
}

void GridGraphics::RenderGraphics() {
}


int WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow) {
    GridGraphics app(hInstance, 1280, 720);

    app.InitWindows();
    app.InitDirectX();

    return 0;
}