#ifndef PCH_H
#define PCH_H

// STL
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cassert>
#include <cmath>

// Windows
#include <windows.h>
#include <wrl.h>

// DirectX
#include <d3d11.h>
#include <dxgi.h>
#include <profileapi.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

// DirectXTK
#include "BufferHelpers.h"
#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"

// ImGUI
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#define NUM(a) (sizeof(a) / sizeof(*a))

inline HRESULT compileShader(LPCWSTR shaderLocation, LPCSTR target, LPCSTR entrypoint, ID3DBlob** shaderBlob) {
    DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> compilationMessages;

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

namespace DX {
    inline void ThrowIfFailed(HRESULT hResult) {
        if (FAILED(hResult)) {
            LPSTR errorText = NULL;
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          hResult,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPSTR)&errorText,
                          0,
                          NULL);
            if(NULL != errorText) {
                MessageBox(0, errorText, 0 , 0);
                std::cout << errorText << std::endl;
            }
        }
    }
}

#endif