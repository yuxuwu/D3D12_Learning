#include <windows.h>
#include <wrl.h>

#include <dxgi1_4.h>
#include <dxgi.h>

#include <d3d12.h>

#include <iostream>
#include <stdexcept>
#include <cassert>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void initD3D12();
bool initWindowsApp(HINSTANCE hInstance, int nShowCmd);
int Run();

HWND ghMainWindow;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd) {
    if(!initWindowsApp(hInstance, nShowCmd)) {
        std::cout << "Initialization failed" << std::endl;
        return 0;
    }

    initD3D12();

    return Run();
}

Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
UINT mRtvDescriptorSize = 0;
UINT mDsvDescriptorSize = 0;
UINT mCbvSrvDescriptorSize = 0;
DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
UINT m4xMsaaQuality = 0;      // quality level of 4X MSAA

void initD3D12() {
    // Enable D3D12 Debug layer if enabled
#if defined(DEBUG) || defined(_DEBUG)
    {
        //Microsoft::WRL::ComPtr<ID3D12Debug> debugController;

    }
#endif

    /// D3D Interfaces
    // Create DXGI Factory
    HRESULT factoryResult = CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory));
    if (FAILED(factoryResult)) {
        throw std::runtime_error("Failed to create D3D12 Device");
    }
    // Create D3D12 Hardware Device
    HRESULT hardwareResult = D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&md3dDevice));
    // Fallback to WARP device if failed to create Hardware Device
    if (FAILED(hardwareResult)) {
        Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
        mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
        D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                          IID_PPV_ARGS(&md3dDevice));
    }


    /// GPU/CPU Synchronization
    // Create ID3D12 Fence
    md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
    mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // Create Command Queue, Command List, and Command List Allocator
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12CommandList> mCommandList;
    D3D12_COMMAND_QUEUE_DESC queueDesc = { };
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;


    /// Buffers
    // Check for 4x MSAA antialiasing support
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = mBackBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    md3dDevice->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            &msQualityLevels,
            sizeof(msQualityLevels));
    m4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m4xMsaaQuality > 0 && "Unexpected MSAA Quality Level");

    // Create Swap Chain
    // Create Descriptor Heap
    // Create and resize Back buffer
    // Create Depth Buffer
    // Set viewport and create scissor rectangle
}

bool initWindowsApp(HINSTANCE instanceHandle, int show) {

    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instanceHandle;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "BasicWndClass";

    if(!RegisterClass(&wc)) {
        MessageBox(0, "Registration Failed", 0, 0);
        return false;
    }

    ghMainWindow = CreateWindow(
            "BasicWndClass",
            "Win32Basic",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            instanceHandle,
            0);

    if (ghMainWindow == 0) {
        MessageBox(0, "Create Window FAILED", 0, 0);
        return false;
    }

    ShowWindow(ghMainWindow, show);
    UpdateWindow(ghMainWindow);

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(ghMainWindow);
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int Run() {
    MSG msg = { 0 };

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            //do game stuff
        }
    }

    return (int)msg.wParam;
}