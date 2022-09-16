//
// Created by yuxuw on 9/13/2022.
//

#include "../pch.h"
#include "../Graphics.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

constexpr UINT WindowWidth = 1280;
constexpr UINT WindowHeight = 720;
constexpr UINT GridWidth = 100;
constexpr UINT GridHeight = 50;
constexpr float blue[4] = {0.0f, 0.2f, 0.4f, 1.0f};
constexpr float red[4] = {0.4, 0.0f, 0.2f, 1.0f};

class GridGraphics : public Graphics {
public:
    GridGraphics(HINSTANCE, UINT, UINT);
    void InitGraphics() override;
    void UpdateGraphics() override;
    void RenderGraphics() override;
    void HandleCharKeys(WPARAM) override;

private:
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    ComPtr<ID3D11Buffer> constBuffer;
};

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

GridGraphics::GridGraphics(HINSTANCE hInstance, UINT width, UINT height
) : Graphics(hInstance, width, height) { }

void GridGraphics::InitGraphics() {
    // Create Vertex Data
    Vertex vertices[GridWidth * GridHeight];
    for(UINT i = 0; i < GridHeight; i++) {
        for (UINT j = 0; j < GridWidth; j++) {
            vertices[i * GridWidth + j].position = XMFLOAT3(
                    static_cast<float>(j*2),
                    0.0f,
                    static_cast<float>(i*2));
            vertices[i * GridWidth + j].color = XMFLOAT4(red);
        }
    }
    // Create Index Data
//    Vertices
//    0 1 2
//    3 4 5
//
//    Desired Indices
//    0 1 3
//    1 3 4
//    1 2 4
//    2 4 5
    UINT indices[(GridWidth-1) * (GridHeight-1) * 6];
    UINT k = 0;
    for(UINT i = 0; i < GridHeight-1; i++) {
        for(UINT j = 0; j < GridWidth-1; j++) {
            UINT currentIndex = i * (GridWidth-1) + j;
            UINT indexBelow = currentIndex + GridWidth;
            // first triangle
            indices[k] = currentIndex;
            indices[k+1] = currentIndex + 1;
            indices[k+2] = indexBelow;
            // second triangle
            indices[k+3] = indexBelow;
            indices[k+4] = currentIndex+1;
            indices[k+5] = indexBelow + 1;
            k+=6;
        }
    }


    // Load Shaders
    ComPtr<ID3DBlob> compiledVertexShader;
    ComPtr<ID3DBlob> compiledPixelShader;
    DX::ThrowIfFailed(compileShader(L"assets/basic_vs.hlsl", "vs_5_0", "VS", &compiledVertexShader));
    DX::ThrowIfFailed(compileShader(L"assets/basic_ps.hlsl", "ps_5_0", "PS", &compiledPixelShader));

    // Create Shaders
    DX::ThrowIfFailed(d3dDevice->CreateVertexShader(compiledVertexShader->GetBufferPointer(),
                                                    compiledVertexShader->GetBufferSize(),
                                                    nullptr,
                                                    &vertexShader));
    DX::ThrowIfFailed(d3dDevice->CreatePixelShader(compiledPixelShader->GetBufferPointer(),
                                                    compiledPixelShader->GetBufferSize(),
                                                    nullptr,
                                                    &pixelShader));
    // Create Input Layout
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
             D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(XMFLOAT3),
             D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    DX::ThrowIfFailed(d3dDevice->CreateInputLayout(
            inputLayoutDesc, sizeof(inputLayoutDesc)/sizeof(*inputLayoutDesc),
            compiledVertexShader->GetBufferPointer(), compiledVertexShader->GetBufferSize(),
            &inputLayout));


    // Create Vertex Buffer
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.ByteWidth = sizeof(vertices)/sizeof(*vertices) * sizeof(Vertex);
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer));

    // Create Index Buffer
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.ByteWidth = sizeof(indices)/sizeof(*indices) * sizeof(UINT);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer));

    // Create WVP matrix
    XMMATRIX world = XMMatrixIdentity();
    XMMATRIX view = XMMatrixLookAtLH(XMVectorSet(2.0f, 5.0f, -2.0f, 0.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * 3.14f, static_cast<float>(WindowWidth)/WindowHeight, 1.0f, 1000.0f);
    XMMATRIX wvpMatrix = world * view * proj;

    // Create Const Buffer
    D3D11_BUFFER_DESC constBufferDesc;
    constBufferDesc.ByteWidth = sizeof(XMMATRIX);
    constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufferDesc.MiscFlags = 0;
    constBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA constBufferData;
    constBufferData.pSysMem = &wvpMatrix;
    constBufferData.SysMemSlicePitch = 0;
    constBufferData.SysMemPitch = 0;
    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&constBufferDesc, &constBufferData, &constBuffer));

    // Set render state
    D3D11_RASTERIZER_DESC renderState;
    ComPtr<ID3D11RasterizerState> WireFrame;
    ZeroMemory(&renderState, sizeof(D3D11_RASTERIZER_DESC));
    renderState.FillMode = D3D11_FILL_WIREFRAME;
    renderState.CullMode = D3D11_CULL_NONE;
    DX::ThrowIfFailed(d3dDevice->CreateRasterizerState(&renderState, &WireFrame));
    d3dDeviceContext->RSSetState(WireFrame.Get());
}

void GridGraphics::UpdateGraphics() {

}

void GridGraphics::RenderGraphics() {
    UINT strides = sizeof(Vertex);
    UINT offsets = 0;

    // Clear Screen
    d3dDeviceContext->ClearRenderTargetView(rtv.Get(), blue);
    d3dDeviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
    // Bind Input Layout
    d3dDeviceContext->IASetInputLayout(inputLayout.Get());
    d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // Bind Vertex Buffer
    d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &strides, &offsets);
    // Bind Index Buffer
    d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // Bind Shader
    d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    d3dDeviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());
    d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

    // Draw
    d3dDeviceContext->DrawIndexed((GridWidth-1) * (GridHeight-1) * 6, 0, 0);
    std::cout << (GridWidth-1) * (GridHeight-1) * 6 << std::endl;
    DX::ThrowIfFailed(swapChain->Present(0, 0));
    DX::ThrowIfFailed(d3dDevice->GetDeviceRemovedReason());
}

void GridGraphics::HandleCharKeys(WPARAM wParam)
{
}


int WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow) {
    GridGraphics app(hInstance, WindowWidth, WindowHeight);

    app.InitWindows();
    app.InitDirectX();
    app.InitGraphics();

    MSG msg = { };
    while(msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            app.UpdateGraphics();
            app.RenderGraphics();
            std::this_thread::sleep_for(std::chrono::milliseconds (10));
        }
    }

    return 0;
}