#ifndef PCH_H
#define PCH_H

// STL
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cassert>

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