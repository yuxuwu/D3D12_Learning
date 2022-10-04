//
// Created by yuxuw on 9/11/2022.
//

#include "Graphics.h"
#include <WindowsX.h>

using Microsoft::WRL::ComPtr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace {
    Graphics* _d3dApp = 0;
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

Graphics::Graphics(HINSTANCE hInstance, UINT width, UINT height) :
    hInstance(hInstance),
    width(width),
    height(height)
{ _d3dApp = this; }

int
Graphics::InitWindows() {
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
            width,
            height,
            nullptr,
            nullptr,
            hInstance,
            nullptr);

    if (mainWindow == nullptr) {
        MessageBox(nullptr, "Create Window FAILED", nullptr, 0);
        return -1;
    }

    RECT clientDimensions;
    GetClientRect(mainWindow, &clientDimensions);
    clientWidth = clientDimensions.right - clientDimensions.left;
    clientHeight = clientDimensions.bottom - clientDimensions.top;

    ShowWindow(mainWindow, SW_SHOW);
    UpdateWindow(mainWindow);

    return 0;
}

LRESULT Graphics::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    if (ImGui_ImplWin32_WndProcHandler(mainWindow, msg, wParam, lParam))
        return true;

    switch(msg) {
        case WM_CHAR:
            HandleCharKeys(wParam);
            return 0;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_ESCAPE:
                    DestroyWindow(mainWindow);
                    break;
            }
            return 0;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            HandleMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            HandleMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSEMOVE:
            HandleMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int
Graphics::InitDirectX() {
    // Support for XM Math
    if(!DirectX::XMVerifyCPUSupport()) return 0;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;


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
    sd.BufferDesc.Width = clientWidth;
    sd.BufferDesc.Height = clientHeight;
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
    DX::ThrowIfFailed(d3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));
    ComPtr<IDXGIAdapter> dxgiAdapter;
    DX::ThrowIfFailed(dxgiDevice->GetParent(IID_PPV_ARGS(&dxgiAdapter)));
    ComPtr<IDXGIFactory> dxgiFactory;
    DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

    DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(d3dDevice.Get(), &sd, &swapChain));
    //Disable ALT+ENTER fullscreen
    dxgiFactory->MakeWindowAssociation(mainWindow, DXGI_MWA_NO_WINDOW_CHANGES);


    /// Back Buffer and Depth Buffer to OM stage
    //Create Back buffer Render Target View
    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv);
    //Create Depth/Stencil Buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = clientWidth;
    depthStencilDesc.Height = clientHeight;
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
    DX::ThrowIfFailed(d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer));
    DX::ThrowIfFailed(d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, &depthStencilView));
    //Bind to OM stage
    d3dDeviceContext->OMSetRenderTargets(1, rtv.GetAddressOf(), depthStencilView.Get());

    /// Set Viewport
    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<float>(clientWidth);
    vp.Height = static_cast<float>(clientHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    d3dDeviceContext->RSSetViewports(1, &vp);


    /// Setup Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(mainWindow);
    ImGui_ImplDX11_Init(d3dDevice.Get(), d3dDeviceContext.Get());
    return 0;
}

int Graphics::Run() {
    /// Timing
    QueryPerformanceFrequency((LARGE_INTEGER*)&refreshFrequency);
    __int64 startCounter;
    double msPerCounter = 1000.0/(double)refreshFrequency;
    QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
    double startTimeMs = startCounter * msPerCounter;

    __int64 currentCounter{};
    double currentTimeMs{};
    __int64 newCurrentCounter{};
    double newCurrentTimeMs{};
    double delta{};


    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Start FPS timer
            QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);
            currentTimeMs = currentCounter * msPerCounter;

            // ImGui Stuff
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();


            /// Do game stuff...
            UpdateGraphics();
            RenderGraphics();
            RenderImgui();


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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return _d3dApp->WindowProc(hwnd, msg, wParam, lParam);
}