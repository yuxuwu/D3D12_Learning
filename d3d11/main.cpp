//
// Created by yuxuw on 8/4/2022.
//

#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <d3d11.h>
#include <wrl.h>
#include <cassert>
#include <dxgi.h>
#include <profileapi.h>
#include <thread>
#include <chrono>

using namespace Microsoft::WRL;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

inline void ThrowIfFailed(HRESULT hResult) {
    if (FAILED(hResult)) {
        throw std::exception();
    }
}

/// Globals
HWND mainWindow;
const UINT WIDTH = 1280;
const UINT HEIGHT = 720;
__int64 frequency;

// wWinMain is specific to the VS compiler https://stackoverflow.com/a/38419618
// If you need cmdLine in Unicode instead of ANSI, convert it with GetCommandLineW()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int nCmdShow) {

    /// Initialize Window
    // Create Window Class with reference to WndProc handler
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "BasicWndClass";

    if(!RegisterClass(&wc)) {
        MessageBox(0, "Registration Failed", 0 , 0);
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
            0,
            0,
            hInstance,
            0);

    if (mainWindow == 0) {
        MessageBox(0, "Create Window FAILED", 0, 0);
        return -1;
    }

    ShowWindow(mainWindow, nCmdShow);
    UpdateWindow(mainWindow);

    UINT createDeviceFlags = 0;
#ifdef defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<ID3D11DeviceContext> d3dDeviceContext;

    /// Initialize D3D Device & Device Context
    ThrowIfFailed(D3D11CreateDevice(
            0,
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            createDeviceFlags,
            0, 0,
            D3D11_SDK_VERSION,
            &d3dDevice,
            &featureLevel,
            &d3dDeviceContext
            ));

    if(featureLevel != D3D_FEATURE_LEVEL_11_0) {
        MessageBox(0, "Direct3D Feature Level 11 unsupported.", 0, 0);
        return -1;
    }

    /// Create Swap Chain
    UINT m4xMsaaQuality;
    ThrowIfFailed(d3dDevice->CheckMultisampleQualityLevels(
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
    ThrowIfFailed(d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf())));
    ComPtr<IDXGIAdapter> dxgiAdapter;
    ThrowIfFailed(dxgiDevice->GetParent(IID_PPV_ARGS(dxgiAdapter.GetAddressOf())));
    ComPtr<IDXGIFactory> dxgiFactory;
    ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

    ComPtr<IDXGISwapChain> swapChain;
    dxgiFactory->CreateSwapChain(d3dDevice.Get(), &sd, &swapChain);
    dxgiFactory->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();

    /// Back Buffer and Depth Buffer to OM stage
    //Create Back buffer Render Target View
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    d3dDevice->CreateRenderTargetView(backBuffer.Get(), 0, &rtv);
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
    ThrowIfFailed(d3dDevice->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer));
    ThrowIfFailed(d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), 0, &depthStencilView));
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


    /// Window Main Loop
    MSG msg = { };
    // Timer stuff
    __int64 currentCounter{};
    double currentTimeMs{};
    __int64 newCurrentCounter{};
    double newCurrentTimeMs{};
    double delta{};

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Start FPS timer
            QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);
            currentTimeMs = currentCounter * msPerCounter;

            /// Do game stuff...
            std::this_thread::sleep_for(std::chrono::milliseconds (10));

            // End FPS timer
            QueryPerformanceCounter((LARGE_INTEGER*)&newCurrentCounter);
            newCurrentTimeMs = newCurrentCounter * msPerCounter;
            delta = newCurrentTimeMs - currentTimeMs;
            double FPS = 1000 / delta;
            std::cout << FPS << std::endl;
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