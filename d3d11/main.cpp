//
// Created by yuxuw on 8/4/2022.
//

#include "pch.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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

// Triangle Data
Vertex triangleVertices[] = {
        {XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)}, // bottom Left, red
        {XMFLOAT3(0.45f, -0.5, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)}, // Top middle, green
        {XMFLOAT3(-0.45f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)} // bottom right, blue
};
UINT triIndices[] = {
        0,1,2
};



XMVECTOR cameraPosition = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

HRESULT compileShader(LPCWSTR shaderLocation, LPCSTR target, LPCSTR entrypoint, ID3DBlob** shaderBlob) {
    DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ComPtr<ID3DBlob> compilationMessages;

    HRESULT hr = D3DCompileFromFile(
            shaderLocation,
            nullptr,
            nullptr,
            entrypoint,
            target,
            shaderFlags,
            0,
            shaderBlob,
            &compilationMessages);

    if (FAILED(hr)) {
        if (compilationMessages.Get() != nullptr) {
            MessageBox(nullptr, (char*) compilationMessages->GetBufferPointer(), nullptr, 0);
            compilationMessages->Release();
        }
    }


    return hr;
}

void InitWindows() {

}

void InitDirectX() {

}

// wWinMain is specific to the VS compiler https://stackoverflow.com/a/38419618
// If you need cmdLine in Unicode instead of ANSI, convert it with GetCommandLineW()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int nCmdShow) {

    // Support for XM Math
    if(!DirectX::XMVerifyCPUSupport()) return 0;

    /// Initialize Window
    // Create Window Class with reference to WndProc handler
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = "BasicWndClass";

    if(!RegisterClass(&wc)) {
        MessageBox(nullptr, "Registration Failed", nullptr , 0);
        return -1;
    }

    mainWindow = CreateWindow(
            "BasicWndClass",
            "Graphics",
            WS_OVERLAPPEDWINDOW,
            0,
            0,
            WIDTH,
            HEIGHT,
            nullptr,
            nullptr,
            hInstance,
            nullptr);

    if (mainWindow == nullptr) {
        MessageBox(nullptr, "Create Window FAILED", nullptr, 0);
        return -1;
    }

    ShowWindow(mainWindow, nCmdShow);
    UpdateWindow(mainWindow);

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<ID3D11DeviceContext> d3dDeviceContext;

    /// Initialize D3D Device & Device Context
    DX::ThrowIfFailed(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            nullptr, 0,
            D3D11_SDK_VERSION,
            &d3dDevice,
            &featureLevel,
            &d3dDeviceContext
            ));

    if(featureLevel != D3D_FEATURE_LEVEL_11_0) {
        MessageBox(nullptr, "Direct3D Feature Level 11 unsupported.", nullptr, 0);
        return -1;
    }

    /// Create Swap Chain
    UINT m4xMsaaQuality;
    DX::ThrowIfFailed(d3dDevice->CheckMultisampleQualityLevels(
            DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality
            ));
    assert(m4xMsaaQuality > 0);

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = WIDTH;
    sd.BufferDesc.Height = HEIGHT;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = mainWindow;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;
    // msaa
    sd.SampleDesc.Count = 4;
    sd.SampleDesc.Quality = m4xMsaaQuality-1;

    // Get IDXGIFactory used to create same d3dDevice
    ComPtr<IDXGIDevice> dxgiDevice;
    DX::ThrowIfFailed(d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf())));
    ComPtr<IDXGIAdapter> dxgiAdapter;
    DX::ThrowIfFailed(dxgiDevice->GetParent(IID_PPV_ARGS(dxgiAdapter.GetAddressOf())));
    ComPtr<IDXGIFactory> dxgiFactory;
    DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

    ComPtr<IDXGISwapChain> swapChain;
    dxgiFactory->CreateSwapChain(d3dDevice.Get(), &sd, &swapChain);
    //Disable ALT+ENTER fullscreen
    dxgiFactory->MakeWindowAssociation(mainWindow, DXGI_MWA_NO_WINDOW_CHANGES);
    dxgiAdapter->Release();
    dxgiDevice->Release();

    /// Back Buffer and Depth Buffer to OM stage
    //Create Back buffer Render Target View
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv);
    backBuffer->Release();
    //Create Depth/Stencil Buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = WIDTH;
    depthStencilDesc.Height = HEIGHT;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;
    depthStencilDesc.SampleDesc.Count = 4;
    depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality-1;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ComPtr<ID3D11DepthStencilView> depthStencilView;
    DX::ThrowIfFailed(d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer));
    DX::ThrowIfFailed(d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, &depthStencilView));
    //Bind to OM stage
    d3dDeviceContext->OMSetRenderTargets(1, rtv.GetAddressOf(), depthStencilView.Get());

    /// Set Viewport
    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<float>(WIDTH);
    vp.Height = static_cast<float>(HEIGHT);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    d3dDeviceContext->RSSetViewports(1, &vp);

    /// Timing
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    __int64 startCounter;
    double msPerCounter = 1000.0/(double)frequency;
    QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
    double startTimeMs = startCounter * msPerCounter;

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
    ComPtr<ID3D11InputLayout> inputLayout;
    DX::ThrowIfFailed(d3dDevice->CreateInputLayout(
            vertexDesc,
            2,
            compiledVertexShader->GetBufferPointer(),
            compiledVertexShader->GetBufferSize(),
            &inputLayout));
    compiledPixelShader->Release();
    compiledVertexShader->Release();

    /// Create Vertex Buffer
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * 8;
    //vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vbufferData;
    vbufferData.pSysMem = boxVertices;
    //vbufferData.pSysMem = triangleVertices;

    ComPtr<ID3D11Buffer> vb;
    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&vertexBufferDesc, &vbufferData, &vb));

    /// Create Index Buffer
    CD3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = sizeof(UINT) * 36;
    //indexBufferDesc.ByteWidth = sizeof(UINT) * 3;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA ibufferData;
    ibufferData.pSysMem = boxIndices;
    //ibufferData.pSysMem = triIndices;

    ComPtr<ID3D11Buffer> ib;
    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&indexBufferDesc, &ibufferData, &ib));

    // Create Constant Buffer
    /// World View Proj Matrices
    XMMATRIX world = XMMatrixIdentity(); // Box Position
    XMMATRIX view = XMMatrixLookAtLH(cameraPosition, XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f*3.14f, static_cast<float>(WIDTH)/HEIGHT, 1.0f, 1000.0f);
    XMMATRIX worldViewProj = world*view*proj;
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

    ComPtr<ID3D11Buffer> constantBuffer;
    DX::ThrowIfFailed(d3dDevice->CreateBuffer(&constantBufferDesc, &cbufferData, &constantBuffer));


    D3D11_MAPPED_SUBRESOURCE mappedBuffer;
    DX::ThrowIfFailed(d3dDeviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer));
    d3dDeviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
    CopyMemory(mappedBuffer.pData, &worldViewProj, sizeof(XMMATRIX));
    d3dDeviceContext->Unmap(constantBuffer.Get(), 0);

    /// Prepare for drawing
    // Bind Input Layout and Buffers
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    float degreeIncrementPhi = 0.05f;
    float degreeIncrementTheta = 0.03f;
    float phi = 0.0f;
    float theta = 0.0f;
    float radius = 5.0f;


    /// Window Main Loop
    MSG msg = { };
    // Timer stuff
    __int64 currentCounter{};
    double currentTimeMs{};
    __int64 newCurrentCounter{};
    double newCurrentTimeMs{};
    double delta{};

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Start FPS timer
            QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);
            currentTimeMs = currentCounter * msPerCounter;

            /// Do game stuff...
//             Update constant buffer
            phi += degreeIncrementPhi;
            theta += degreeIncrementTheta;
            float x = radius * sinf(phi) * cosf(0);
            float y = radius * sinf(phi) * sinf(0);
            float z = radius * cos(phi);

            view = XMMatrixLookAtLH(XMVectorSet(x, y, z, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
            world =  XMMatrixRotationX(.1225*3.14) * XMMatrixTranslation(0, sinf(theta)/2, 0);
            worldViewProj = XMMatrixTranspose(world * view * proj);
            DX::ThrowIfFailed(d3dDeviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer));
            d3dDeviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
            CopyMemory(mappedBuffer.pData, &worldViewProj, sizeof(XMMATRIX));
            d3dDeviceContext->Unmap(constantBuffer.Get(), 0);

            // Draw
            d3dDeviceContext->ClearRenderTargetView(rtv.Get(), blue);
            d3dDeviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

            d3dDeviceContext->IASetInputLayout(inputLayout.Get());
            d3dDeviceContext->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);
            d3dDeviceContext->IASetIndexBuffer(ib.Get(), DXGI_FORMAT_R32_UINT, 0);
            d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            d3dDeviceContext->DrawIndexed(36, 0, 0);
            //d3dDeviceContext->Draw(3, 0);
            DX::ThrowIfFailed(swapChain->Present(0, 0));
            DX::ThrowIfFailed(d3dDevice->GetDeviceRemovedReason());



            std::this_thread::sleep_for(std::chrono::milliseconds (10));

            // End FPS timer
            QueryPerformanceCounter((LARGE_INTEGER*)&newCurrentCounter);
            newCurrentTimeMs = newCurrentCounter * msPerCounter;
            delta = newCurrentTimeMs - currentTimeMs;
            double FPS = 1000 / delta;
            //std::cout << FPS << std::endl;
        }
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_KEYDOWN:
            if(wParam == VK_ESCAPE)
                DestroyWindow(mainWindow);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}