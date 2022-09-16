//
// Created by yuxuw on 8/4/2022.
//

#include "../pch.h"
#include "../Graphics.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


/// Globals
HWND mainWindow;
const UINT WIDTH = 1280;
const UINT HEIGHT = 720;
const float blue[4] = {0.0f, 0.2f, 0.4f, 1.0f};
__int64 frequency;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

// 3D Scene
Vertex boxVertices[] = {
        {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)}, // front-bottom-left
        {XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)}, // front-top-left
        {XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)}, // front-top-right
        {XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)}, // front-bottom-right
        {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)}, // back-bottom-left
        {XMFLOAT3(-1.0f, +1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)}, // back-top-left
        {XMFLOAT3(+1.0f, +1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)}, // back-top-right
        {XMFLOAT3(+1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)} // back-bottom-right
};

UINT boxIndices[] = {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
};


XMVECTOR cameraPosition = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);



class BoxGraphics : public Graphics {
public:
    BoxGraphics(HINSTANCE pHinstance, const UINT& width, const UINT& height);

    void InitGraphics() override;
    void UpdateGraphics() override;
    void RenderGraphics() override;
    void HandleCharKeys(WPARAM) override;

private:
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX proj;
    XMMATRIX worldViewProj;

    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11Buffer> vb;
    ComPtr<ID3D11Buffer> ib;
    ComPtr<ID3D11Buffer> constantBuffer;
    D3D11_MAPPED_SUBRESOURCE mappedBuffer;

    float phi = 0;
    float theta = 0;

    static constexpr float degreeIncrementPhi = .05f;
    static constexpr float degreeIncrementTheta = .03f;
    static constexpr float radius = 5.0f;

    float cameraX = 0.0f;
    float cameraY = 0.0f;
    float cameraZ = -10.0f;

    float upX = 0.0f;
    float upY = 1.0f;
    float upZ = 0.0f;
};

BoxGraphics::BoxGraphics(HINSTANCE pHinstance, const UINT& width, const UINT& height)
    : Graphics(pHinstance, width, height)
{ }

void BoxGraphics::InitGraphics() {
    /// Prepare for drawing
    // Bind Input Layout and Buffers
    /// Load Scene into GPU
    ComPtr<ID3DBlob> compiledPixelShader;
    ComPtr<ID3DBlob> compiledVertexShader;
    DX::ThrowIfFailed(compileShader(L"assets/basic_ps.hlsl", "ps_5_0", "PS", &compiledPixelShader));
    DX::ThrowIfFailed(compileShader(L"assets/basic_vs.hlsl", "vs_5_0", "VS", &compiledVertexShader));

    ComPtr<ID3D11PixelShader> basicPS;
    ComPtr<ID3D11VertexShader> basicVS;

    DX::ThrowIfFailed(d3dDevice->CreatePixelShader(compiledPixelShader->GetBufferPointer(),
                                                   compiledPixelShader->GetBufferSize(),
                                                   nullptr,
                                                   &basicPS));
    DX::ThrowIfFailed(d3dDevice->CreateVertexShader(compiledVertexShader->GetBufferPointer(),
                                                    compiledVertexShader->GetBufferSize(),
                                                    nullptr,
                                                    &basicVS));
    d3dDeviceContext->VSSetShader(basicVS.Get(), nullptr, 0);
    d3dDeviceContext->PSSetShader(basicPS.Get(), nullptr, 0);

    // Create Vertex input layout
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(XMFLOAT3),
                    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    DX::ThrowIfFailed(d3dDevice->CreateInputLayout(
            vertexDesc,
            2,
            compiledVertexShader->GetBufferPointer(),
            compiledVertexShader->GetBufferSize(),
            &inputLayout));

    /// Create Vertex Buffer
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * 8;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vbufferData;
    vbufferData.pSysMem = boxVertices;

    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&vertexBufferDesc, &vbufferData, &vb));

    /// Create Index Buffer
    CD3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = sizeof(UINT) * 36;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA ibufferData;
    ibufferData.pSysMem = boxIndices;

    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&indexBufferDesc, &ibufferData, &ib));

    // Create Constant Buffer
    /// World View Proj Matrices
    world = XMMatrixIdentity(); // Box Position
    view = XMMatrixLookAtLH(cameraPosition, XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    proj = XMMatrixPerspectiveFovLH(0.25f*3.14f, static_cast<float>(WIDTH)/HEIGHT, 1.0f, 1000.0f);
    worldViewProj = world*view*proj;
    D3D11_BUFFER_DESC constantBufferDesc;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
    constantBufferDesc.StructureByteStride = 0;


    D3D11_SUBRESOURCE_DATA cbufferData;
    cbufferData.pSysMem = &worldViewProj;
    cbufferData.SysMemPitch = 0;
    cbufferData.SysMemSlicePitch = 0;

    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&constantBufferDesc, &cbufferData, &constantBuffer));


    DX::ThrowIfFailed(d3dDeviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer));
    d3dDeviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
    CopyMemory(mappedBuffer.pData, &worldViewProj, sizeof(XMMATRIX));
    d3dDeviceContext->Unmap(constantBuffer.Get(), 0);

}

void BoxGraphics::UpdateGraphics() {
    // Update constant buffer
    phi += degreeIncrementPhi;
      theta += degreeIncrementTheta;
//    float x = radius * sinf(phi) * cosf(0);
//    float y = radius * sinf(phi) * sinf(0);
//    float z = radius * cos(phi);

    view = XMMatrixLookAtLH(XMVectorSet(cameraX, cameraY, cameraZ, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f,  0.0f, 0.0f));
    world = XMMatrixRotationY(theta * 3.14) * XMMatrixRotationX(.1225*3.14) * XMMatrixTranslation(0, sinf(theta)/2, 0);
    worldViewProj = XMMatrixTranspose(world * view * proj);
    DX::ThrowIfFailed(d3dDeviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer));
    CopyMemory(mappedBuffer.pData, &worldViewProj, sizeof(XMMATRIX));
    d3dDeviceContext->Unmap(constantBuffer.Get(), 0);
}

void BoxGraphics::RenderGraphics() {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    // Draw
    d3dDeviceContext->ClearRenderTargetView(rtv.Get(), blue);
    d3dDeviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    d3dDeviceContext->IASetInputLayout(inputLayout.Get());
    d3dDeviceContext->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);
    d3dDeviceContext->IASetIndexBuffer(ib.Get(), DXGI_FORMAT_R32_UINT, 0);
    d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    d3dDeviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    d3dDeviceContext->DrawIndexed(36, 0, 0);
}

void BoxGraphics::HandleCharKeys(WPARAM wParam) {
    switch(wParam) {
        case 'w':
            /// Move camera forward
            // Get forward vector

            // Move camera position along forward vector
            break;
        case 's':
            /// Move camera back
            break;

        case 'a':
            /// Strafe camera left
            // Cross forward and up vector to get side vector
            // Move camera position along side vector
            break;
        case 'd':
            /// Strafe camera right
            break;
        default:
            return;
    }
}


// wWinMain is specific to the VS compiler https://stackoverflow.com/a/38419618
// If you need cmdLine in Unicode instead of ANSI, convert it with GetCommandLineW()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int nCmdShow) {

    BoxGraphics graphics(hInstance, WIDTH, HEIGHT);
    int initWindowsResult = graphics.InitWindows();
    if (initWindowsResult != 0) return initWindowsResult;

    int initDirectXResult = graphics.InitDirectX();
    if (initDirectXResult != 0) return initDirectXResult;

    graphics.InitGraphics();


    return graphics.Run();
}