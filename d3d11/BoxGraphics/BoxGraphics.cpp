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

struct ConstBuffer {
    XMMATRIX wvpMatrix;
    XMFLOAT4 offset;
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


class BoxGraphics : public Graphics {
public:
    BoxGraphics(HINSTANCE pHinstance, const UINT& width, const UINT& height);

    void InitGraphics() override;
    void UpdateGraphics() override;
    void RenderGraphics() override;
    void RenderImgui() override;

    void HandleCharKeys(WPARAM) override;
    void HandleMouseUp(WPARAM, int, int) override;
    void HandleMouseDown(WPARAM, int, int) override;
    void HandleMouseMove(WPARAM, int, int) override;

private:
    XMMATRIX calculateViewMatrix();

    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX proj;
    XMMATRIX worldViewProj;

    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11Buffer> vb;
    ComPtr<ID3D11Buffer> ib;
    ComPtr<ID3D11Buffer> constantBuffer;
    D3D11_MAPPED_SUBRESOURCE mappedConstantBuffer;

    float boxPhi = 0.0f;
    float boxTheta = 0.0f;
    static constexpr float degreeIncrementPhi = .02f;
    static constexpr float degreeIncrementTheta = .01f;

    float cameraPosition[4] = {0.0f, 0.0f, -10.0f, 1.0f};
    float cameraForwardPhi = 0.3f;
    float cameraForwardTheta = 0.3f;
    float cameraForward[4] = {0.0f, 0.0f, 1.0f, 0.0f};

    int mousePositionX = 0;
    int mousePositionY = 0;
};

BoxGraphics::BoxGraphics(HINSTANCE pHinstance, const UINT& width, const UINT& height)
    : Graphics(pHinstance, width, height)
{ }

XMMATRIX BoxGraphics::calculateViewMatrix() {
    // Recalculate the forward vector
//    float xCamera = sinf(cameraForwardPhi) * cosf(cameraForwardTheta);
//    float yCamera = sinf(cameraForwardPhi) * sinf(cameraForwardTheta);
//    float zCamera = sinf(cameraForwardPhi);
//    cameraForward[0] = xCamera;
//    cameraForward[1] = yCamera;
//    cameraForward[2] = zCamera;

    return XMMatrixLookAtLH(
            XMVectorSet(
           cameraPosition[0],
           cameraPosition[1],
           cameraPosition[2],
           cameraPosition[3]),
           XMVectorSet(
           cameraPosition[0]+cameraForward[0],
           cameraPosition[1]+cameraForward[1],
           cameraPosition[2]+cameraForward[2],
           0.0f),
           XMVectorSet(0.0f, 1.0f,  0.0f, 0.0f));
}

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
            sizeof(vertexDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
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
    view = calculateViewMatrix();
    proj = XMMatrixPerspectiveFovLH(0.25f*3.14f, static_cast<float>(WIDTH)/HEIGHT, 1.0f, 1000.0f);
    worldViewProj = world*view*proj;
    XMFLOAT4 offset = XMFLOAT4(5.0f, 5.0f, 5.0f, 0.0f);

    D3D11_BUFFER_DESC constantBufferDesc;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(ConstBuffer);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
    constantBufferDesc.StructureByteStride = 0;

    ConstBuffer constBuffer = {worldViewProj, offset};

    D3D11_SUBRESOURCE_DATA cbufferData;
    cbufferData.pSysMem = &constBuffer;
    cbufferData.SysMemPitch = 0;
    cbufferData.SysMemSlicePitch = 0;

    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&constantBufferDesc, &cbufferData, &constantBuffer));

    /// Bind Input Layout, Vertex, Index, and Constant buffers
    UINT stride = sizeof(Vertex);
    UINT vertexBufferOffset = 0;
    ID3D11Buffer* buffers[] = {constantBuffer.Get()};
    d3dDeviceContext->IASetInputLayout(inputLayout.Get());
    d3dDeviceContext->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &vertexBufferOffset);
    d3dDeviceContext->IASetIndexBuffer(ib.Get(), DXGI_FORMAT_R32_UINT, 0);
    d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3dDeviceContext->VSSetConstantBuffers(0, sizeof(buffers)/sizeof(ID3D11Buffer*), buffers);
}

void BoxGraphics::UpdateGraphics() {
    // Update constant buffer
    boxPhi += degreeIncrementPhi;
    boxTheta += degreeIncrementTheta;
//    float x = radius * sinf(boxPhi) * cosf(0);
//    float y = radius * sinf(boxPhi) * sinf(0);
//    float z = radius * cos(boxPhi);

    view = calculateViewMatrix();
    world = XMMatrixRotationY(boxTheta * 3.14) * // lil rotation animation
            XMMatrixRotationX(.1225 * 3.14) *  // constant tilt
            XMMatrixTranslation(0, sinf(boxTheta) / 2, 0); //
    worldViewProj = XMMatrixTranspose(world * view * proj);
}

void BoxGraphics::RenderGraphics() {


    // Draw
    d3dDeviceContext->ClearRenderTargetView(rtv.Get(), blue);
    d3dDeviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Make a 3x3 cage of boxes

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            for (int k = -1; k <= 1; ++k) {
                if (!(i == 0 && j == 0 && k == 0)) {
                    // Update the const buffer with offset and wvp
                    XMFLOAT4 vectorOffset = XMFLOAT4(i*5.0f, j*5.0f, k*5.0f, 0.0f);
                    ConstBuffer constBuffer = {worldViewProj, vectorOffset};
                    DX::ThrowIfFailed(d3dDeviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstantBuffer));
                    CopyMemory(mappedConstantBuffer.pData, &constBuffer, sizeof(ConstBuffer));
                    d3dDeviceContext->Unmap(constantBuffer.Get(), 0);

                    d3dDeviceContext->DrawIndexed(sizeof(boxIndices)/sizeof(UINT), 0, 0);
                }
            }
        }
    }
}

void BoxGraphics::RenderImgui() {
    ImGui::Begin("Camera Controls", NULL ,ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SliderFloat4("Camera Position", cameraPosition, -100.0f, 100.0f);
    ImGui::SliderFloat4("Camera Forward Vector", cameraForward, -2.0f, 2.0f);
    ImGui::Text("Mouse Position X: "); ImGui::SameLine();
    ImGui::Text(std::to_string(mousePositionX).c_str()); ImGui::SameLine();
    ImGui::Text("Mouse Position Y: "); ImGui::SameLine();
    ImGui::Text(std::to_string(mousePositionY).c_str());
    ImGui::Text("CameraForward Theta: "); ImGui::SameLine();
    ImGui::Text(std::to_string(cameraForwardTheta).c_str()); ImGui::SameLine();
    ImGui::Text("CameraForward Phi: "); ImGui::SameLine();
    ImGui::Text(std::to_string(cameraForwardPhi).c_str());

    //bool t = true;
    //ImGui::ShowDemoWindow(&t);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void BoxGraphics::HandleCharKeys(WPARAM chara) {
    const float speed = 1.0f;
    std::cout << static_cast<char>(chara) << std::endl;
    switch(chara) {
        case 'w':
            /// Move camera forward
            // Move camera position along forward vector
            cameraPosition[0] += cameraForward[0] * speed;
            cameraPosition[1] += cameraForward[1] * speed;
            cameraPosition[2] += cameraForward[2] * speed;
            break;
        case 's':
            /// Move camera back
            cameraPosition[0] -= cameraForward[0] * speed;
            cameraPosition[1] -= cameraForward[1] * speed;
            cameraPosition[2] -= cameraForward[2] * speed;
            break;
        case 'a':
            /// Strafe camera left
            // Cross forward and up vector to get side vector
            // Move camera position along side vector
            break;
        case 'd':
            /// Strafe camera right
            break;
        case 'q': {
            /// Turn left
            cameraForwardTheta += 0.025;
            // Recalculate the forward vector
            float xCamera = sinf(cameraForwardPhi) * cosf(cameraForwardTheta);
            float yCamera = sinf(cameraForwardPhi) * sinf(cameraForwardTheta);
            float zCamera = sinf(cameraForwardPhi);
            cameraForward[0] = xCamera;
            cameraForward[1] = yCamera;
            cameraForward[2] = zCamera;
            break;
        }
        case 'e':
            /// Turn right
            break;
        case ' ':
            /// Reset Camera

        default:
            return;
    }
}

void BoxGraphics::HandleMouseUp(WPARAM bDownButtons, int x, int y) {
    std::cout << "Mouse up event found" << std::endl;
}

void BoxGraphics::HandleMouseDown(WPARAM bDownButtons, int x, int y) {
    if (bDownButtons & MK_LBUTTON) {
        std::cout << "Left Mouse Down" << std::endl;
    }
}

void BoxGraphics::HandleMouseMove(WPARAM bDownButtons, int x, int y) {
    /*
    int deltaX = x - mousePositionX;
    int deltaY = y - mousePositionY;

    // A change in X should correspond to a change in the horizontal axis (boxTheta)
    cameraForwardTheta += static_cast<float>(deltaX) * 0.025;
    // A change in Y should correspond to a change in the vertical axis (boxPhi)
    cameraForwardPhi += static_cast<float>(deltaY) * 0.025;

    mousePositionX = x;
    mousePositionY = y;
     */
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