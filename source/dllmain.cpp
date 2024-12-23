/**
* Copyright (C) 2020 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/
// Fix Enhancers Harry Potter Quditch World Cup patches added 


#include "d3d8.h"
#include <d3dx8.h>
#include "iathook.h"
#include "helpers.h"
#include <windows.h> // chip
#include <iostream> // chip  
#include <vector> // chip
#include <Psapi.h> // chip
#pragma comment (lib, "d3dx8.lib")
#pragma comment (lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "winmm.lib") // needed for timeBeginPeriod()/timeEndPeriod()

// chip - adding macros
# define DX_PRINT(x) std::cout << x << std::endl;
# define DX_ERROR(x) std::cerr << x << std::endl;
#define DX_MBPRINT(x) MessageBox(NULL, x, "Message", MB_OK);
#define DX_MBERROR(x) MessageBox(NULL, x, "Error", MB_ICONERROR | MB_OK);

Direct3D8EnableMaximizedWindowedModeShimProc m_pDirect3D8EnableMaximizedWindowedModeShim;
ValidatePixelShaderProc m_pValidatePixelShader;
ValidateVertexShaderProc m_pValidateVertexShader;
DebugSetMuteProc m_pDebugSetMute;
Direct3DCreate8Proc m_pDirect3DCreate8;

HWND g_hFocusWindow = NULL;
HMODULE g_hWrapperModule = NULL;

HMODULE d3d8dll = NULL;

bool bForceWindowedMode;
bool bDirect3D8DisableMaximizedWindowedModeShim;
bool bUsePrimaryMonitor;
bool bCenterWindow;
bool bBorderlessFullscreen;
bool bAlwaysOnTop;
bool bDoNotNotifyOnTaskSwitch;
bool bDisplayFPSCounter;
float fFPSLimit;
int nFullScreenRefreshRateInHz;

char WinDir[MAX_PATH + 1];

//=======================================================================================================================================================================================
// chip - 1: stop reading resolution from screen.cli

// Function to perform the hex edit
void PerformHexEdit(LPBYTE lpAddress, DWORD moduleSize) {
    // Define the patterns to search for and their corresponding new values
    struct HexEdit {
        std::vector<BYTE> pattern;
        std::vector<BYTE> newValue;
        size_t offset; // Offset of the byte to modify within the pattern
    };

    // Define the edits
    std::vector<HexEdit> edits = {
        // stop resolution reading from the screen.cli height & width
        { { 0x83, 0x7C, 0x24, 0x08, 0x01, 0x75, 0x1F, 0xA0, 0xF8, 0x69, 0x5E, 0x00, 0x84, 0xC0,
        0x74, 0x16, 0xA0, 0xF9, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x0D, 0x8B, 0x44, 0x24, 0x0C,
        0x8B, 0x48, 0x04, 0x89, 0x0D, 0x98, 0x01, 0x5D, 0x00, 0xC3, 0x46, 0x0C, 0x0F, 0x95, 0xC0, 0x83,
        0xC4, 0x30, 0xC3, 0x83, 0x7C, 0x24, 0x08, 0x01, 0x75, 0x1F, 0xA0, 0xF8, 0x69, 0x5E, 0x00, 0x84, 0xC0,
        0x74, 0x16, 0xA0, 0xF9, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x0D, 0x8B, 0x44, 0x24, 0x0C, 0x8B, 0x48, 0x04,
        0x89, 0x0D, 0x9C, 0x01, 0x5D, 0x00, 0xC3, 0x48, 0x04, 0x89, 0x0D, 0xE9, 0xC7, 0xFF, 0xFF, 0xFF },
        { 0x83, 0x7C, 0x24, 0x08, 0x01, 0xEB, 0x1F, 0xA0, 0xF8, 0x69, 0x5E, 0x00, 0x84, 0xC0,
        0x74, 0x16, 0xA0, 0xF9, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x0D, 0x8B, 0x44, 0x24, 0x0C,
        0x8B, 0x48, 0x04, 0x89, 0x0D, 0x98, 0x01, 0x5D, 0x00, 0xC3, 0x46, 0x0C, 0x0F, 0x95, 0xC0, 0x83,
        0xC4, 0x30, 0xC3, 0x83, 0x7C, 0x24, 0x08, 0x01, 0xEB, 0x1F, 0xA0, 0xF8, 0x69, 0x5E, 0x00, 0x84, 0xC0,
        0x74, 0x16, 0xA0, 0xF9, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x0D, 0x8B, 0x44, 0x24, 0x0C, 0x8B, 0x48, 0x04,
        0x89, 0x0D, 0x9C, 0x01, 0x5D, 0x00, 0xC3, 0x48, 0x04, 0x89, 0x0D, 0xE9, 0xC7, 0xFF, 0xFF, 0xFF }, 0 },

    };

    // Iterate through the edits
    for (const auto& edit : edits) {
        // Search for the pattern in memory
        for (DWORD i = 0; i < moduleSize - edit.pattern.size(); ++i) {
            if (memcmp(lpAddress + i, edit.pattern.data(), edit.pattern.size()) == 0) {
                // Pattern found in memory
                DX_PRINT("Pattern found in memory.")

                    // Modify memory
                    LPVOID lpAddressToWrite = lpAddress + i + edit.offset;
                SIZE_T numberOfBytesWritten;
                BOOL result = WriteProcessMemory(GetCurrentProcess(), lpAddressToWrite, edit.newValue.data(), edit.newValue.size(), &numberOfBytesWritten);
                if (!result || numberOfBytesWritten != edit.newValue.size()) {
                    std::cerr << "Failed to write memory." << std::endl;
                    return;
                }
                DX_PRINT("Hex edited successfully.")
                    break;
            }
        }
    }
}

// Function to perform the hex edits
void PerformHexEdits() {
    // Get the handle to the current module
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL) {
        DX_ERROR("Failed to get module handle.")
            return;
    }

    // Get the module information
    LPBYTE lpAddress = reinterpret_cast<LPBYTE>(hModule);
    DWORD moduleSize = 0; // Placeholder for module size
    TCHAR szFileName[MAX_PATH];
    if (GetModuleFileNameEx(GetCurrentProcess(), hModule, szFileName, MAX_PATH)) {
        moduleSize = GetFileSize(szFileName, NULL);
    }
    if (moduleSize == 0) {
        DX_ERROR("Failed to get module information.")
            return;
    }

    // Perform the hex edit
    PerformHexEdit(lpAddress, moduleSize);
}

// chip - 1: stop reading resolution from screen.cli
//=======================================================================================================================================================================================

//chip - 2: window mode

const std::vector<BYTE> commonHexEdit2 = { 0x83, 0x7C, 0x24, 0x08, 0x01, 0x75, 0x1F, 0xA0, 0xF8, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x16, 0xA0, 0xF9, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x0D, 0x8B, 0x44, 0x24,
0x0C, 0x8B, 0x48, 0x04, 0x89, 0x0D, 0xE4, 0x14, 0xF9, 0x00, 0xC3, 0x48, 0x04, 0x89, 0x0D, 0x9C, 0x01, 0x5D, 0x00, 0xC3 };

struct HexEdit2 {
    std::vector<BYTE> modified2;
    size_t offset2;
};

// index for hex edits for window mode

HexEdit2 CreateHexEditFromResolution2(int windowIndex) {
    HexEdit2 edit2;

    switch (windowIndex) {
    case 1:
        edit2.modified2 = { 0x83, 0x7C, 0x24, 0x08, 0x01, 0xEB, 0x1F, 0xA0, 0xF8, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x16, 0xA0, 0xF9, 0x69, 0x5E, 0x00, 0x84, 0xC0, 0x74, 0x0D, 0x8B, 0x44, 0x24,
0x0C, 0x8B, 0x48, 0x04, 0x89, 0x0D, 0xE4, 0x14, 0xF9, 0x00, 0xC3, 0x48, 0x04, 0x89, 0x0D, 0x9C, 0x01, 0x5D, 0x00, 0xC3 };  // edit for window mode
        edit2.offset2 = 0;
        break;
    default:
        DX_PRINT("Invalid resolution index.")
            break;
    }

    return edit2;
}

void PerformHexEdit2(LPBYTE lpAddress, DWORD moduleSize, const HexEdit2& edit2) {
    for (DWORD i = 0; i < moduleSize - commonHexEdit2.size(); ++i) {
        if (memcmp(lpAddress + i, commonHexEdit2.data(), commonHexEdit2.size()) == 0) {
            DX_PRINT("Pattern found in memory.")

                LPVOID lpAddressToWrite = lpAddress + i + edit2.offset2;
            SIZE_T numberOfBytesWritten;
            BOOL result = WriteProcessMemory(GetCurrentProcess(), lpAddressToWrite, edit2.modified2.data(), edit2.modified2.size(), &numberOfBytesWritten);
            if (!result || numberOfBytesWritten != edit2.modified2.size()) {
                DX_ERROR("Failed to write memory.")
                    return;
            }
            DX_PRINT("Hex edited successfully.")
                return;
        }
    }
    DX_ERROR("Pattern not found in memory.")
}

void PerformHexEdits2() {
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL) {
        DX_ERROR("Failed to get module handle.")
            return;
    }

    // get the module information 
    LPBYTE lpAddress = reinterpret_cast<LPBYTE>(hModule);
    DWORD moduleSize = 0;
    TCHAR szFileName[MAX_PATH];
    if (GetModuleFileNameEx(GetCurrentProcess(), hModule, szFileName, MAX_PATH)) {
        moduleSize = GetFileSize(szFileName, NULL);
    }
    if (moduleSize == 0) {
        DX_ERROR("Failed to get module information.")
            return;
    }

    // ini
    char path[MAX_PATH];
    HMODULE hm = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&Direct3DCreate8, &hm);
    GetModuleFileNameA(hm, path, sizeof(path));
    strcpy(strrchr(path, '\\'), "\\d3d8.ini");

    // Read resolution index from the INI file
    int windowIndex = GetPrivateProfileInt("WINDOWMODE", "windowmode", 0, path);
    if (windowIndex == 0) {
        DX_ERROR("Failed to read resolution index from INI file.")
            return;
    }

    HexEdit2 edit = CreateHexEditFromResolution2(windowIndex);
    if (edit.modified2.empty()) {
        DX_ERROR("Failed to create hex edit for resolution index: ")
            return;
    }

    PerformHexEdit2(lpAddress, moduleSize, edit);
}

// chip - 2: window mode
//=======================================================================================================================================================================================

//chip - 3: resolution

const std::vector<BYTE> commonHexEdit3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x20, 0x03, 0x00, 0x00, 0x58, 0x02, 0x00, 0x00 };

struct HexEdit3 {
    std::vector<BYTE> modified3;
    size_t offset3;
};

// index for hex edits for resolution

HexEdit3 CreateHexEditFromResolution3(int resolutionIndex) {
    HexEdit3 edit3;

    switch (resolutionIndex) {
    case 1:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x05, 0x00, 0x00, 0xD0, 0x02, 0x00, 0x00 };  // 1280 x 720
        edit3.offset3 = 0;
        break;
    case 2:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x80, 0x07, 0x00, 0x00, 0x38, 0x04, 0x00, 0x00 };  // 1920 x 1080
        edit3.offset3 = 0;
        break;
    case 3:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x0A, 0x00, 0x00, 0xA0, 0x05, 0x00, 0x00 };  // 2560 x1440 
        edit3.offset3 = 0;
        break;
    case 4:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x0F, 0x00, 0x00, 0x70, 0x08, 0x00, 0x00 };  // 3840 x 2160
        edit3.offset3 = 0;
        break;
    case 5:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x70, 0x0D, 0x00, 0x00, 0xA0, 0x05, 0x00, 0x00 };  // 3440 x 1440
        edit3.offset3 = 0;
        break;
    case 6:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0xA0, 0x05, 0x00, 0x00, 0x84, 0x03, 0x00, 0x00 };  // 1440 x 900
        edit3.offset3 = 0;
        break;
    case 7:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x40, 0x06, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x00 };  // 1600 x 1200
        edit3.offset3 = 0;
        break;
    case 8:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 };  // 3840 x 1024
        edit3.offset3 = 0;
        break;
    case 9:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x70, 0x17, 0x00, 0x00, 0x38, 0x04, 0x00, 0x00 };  // 6000 x 1080
        edit3.offset3 = 0;
        break;
    case 10:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x0A, 0x00, 0x00, 0x38, 0x04, 0x00, 0x00 };  // 2560 x 1080
        edit3.offset3 = 0;
        break;
    case 11:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x0F, 0x00, 0x00, 0x40, 0x06, 0x00, 0x00 };  // 3840 x 1600
        edit3.offset3 = 0;
        break;
    case 12:
        edit3.modified3 = { 0xAB, 0xAA, 0xAA, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x05, 0x00, 0x00, 0x20, 0x03, 0x00, 0x00 };  // 1280 x 800
        edit3.offset3 = 0;
        break;
    default:
        DX_PRINT("Invalid resolution index.")
            break;
    }

    return edit3;
}

void PerformHexEdit3(LPBYTE lpAddress, DWORD moduleSize, const HexEdit3& edit3) {
    for (DWORD i = 0; i < moduleSize - commonHexEdit3.size(); ++i) {
        if (memcmp(lpAddress + i, commonHexEdit3.data(), commonHexEdit3.size()) == 0) {
            DX_PRINT("Pattern found in memory.")

                LPVOID lpAddressToWrite = lpAddress + i + edit3.offset3;
            SIZE_T numberOfBytesWritten;
            BOOL result = WriteProcessMemory(GetCurrentProcess(), lpAddressToWrite, edit3.modified3.data(), edit3.modified3.size(), &numberOfBytesWritten);
            if (!result || numberOfBytesWritten != edit3.modified3.size()) {
                DX_ERROR("Failed to write memory.")
                    return;
            }
            DX_PRINT("Hex edited successfully.")
                return;
        }
    }
    DX_ERROR("Pattern not found in memory.")
}

void PerformHexEdits3() {
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL) {
        DX_ERROR("Failed to get module handle.")
            return;
    }

    // get the module information 
    LPBYTE lpAddress = reinterpret_cast<LPBYTE>(hModule);
    DWORD moduleSize = 0;
    TCHAR szFileName[MAX_PATH];
    if (GetModuleFileNameEx(GetCurrentProcess(), hModule, szFileName, MAX_PATH)) {
        moduleSize = GetFileSize(szFileName, NULL);
    }
    if (moduleSize == 0) {
        DX_ERROR("Failed to get module information.")
            return;
    }

    // ini
    char path[MAX_PATH];
    HMODULE hm = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&Direct3DCreate8, &hm);
    GetModuleFileNameA(hm, path, sizeof(path));
    strcpy(strrchr(path, '\\'), "\\d3d8.ini");

    // Read resolution index from the INI file
    int resolutionIndex = GetPrivateProfileInt("FULLSCREENRESOLUTION", "fullscreenresolution", 0, path);
    if (resolutionIndex == 0) {
        DX_ERROR("Failed to read resolution index from INI file.")
            return;
    }

    HexEdit3 edit = CreateHexEditFromResolution3(resolutionIndex);
    if (edit.modified3.empty()) {
        DX_ERROR("Failed to create hex edit for resolution index: ")
            return;
    }

    PerformHexEdit3(lpAddress, moduleSize, edit);
}

// chip - 3: resolution
//=======================================================================================================================================================================================
// 
// chip - 4: aspect ratio

const std::vector<BYTE> commonHexEdit4 = { 0x24, 0x10, 0xC7, 0x47, 0x18, 0xAB, 0xAA, 0xAA, 0x3F, 0x8B, 0x5C, 0x24, 0x14, 0x8B, 0xE8, 0x69 };

struct HexEdit4 {
    std::vector<BYTE> modified4;
    size_t offset4;
};

// index for hex edits for aspect ratio 

HexEdit4 CreateHexEditFromAspect(int aspectIndex) {
    HexEdit4 edit4;

    switch (aspectIndex) {
    case 1:
        edit4.modified4 = { 0x24, 0x10, 0xC7, 0x47, 0x18, 0x39, 0x8E, 0xE3, 0x3F, 0x8B, 0x5C, 0x24, 0x14, 0x8B, 0xE8, 0x69 };                 //16:9
        edit4.offset4 = 0;
        break;
    case 2:
        edit4.modified4 = { 0x24, 0x10, 0xC7, 0x47, 0x18, 0xCD, 0xCC, 0xCC, 0x3F, 0x8B, 0x5C, 0x24, 0x14, 0x8B, 0xE8, 0x69 };                 //16:10
        edit4.offset4 = 0;
        break;
    case 3:
        edit4.modified4 = { 0x24, 0x10, 0xC7, 0x47, 0x18, 0x26, 0xB4, 0x17, 0x40, 0x8B, 0x5C, 0x24, 0x14, 0x8B, 0xE8, 0x69 };                //21:9 (2560x1080)
        edit4.offset4 = 0;
        break;
    case 4:
        edit4.modified4 = { 0x24, 0x10, 0xC7, 0x47, 0x18, 0x8E, 0xE3, 0x18, 0x40, 0x8B, 0x5C, 0x24, 0x14, 0x8B, 0xE8, 0x69 };                //21:9 (3440x1440)
        edit4.offset4 = 0;
        break;
    case 5:
        edit4.modified4 = { 0x24, 0x10, 0xC7, 0x47, 0x18, 0x9A, 0x99, 0x19, 0x40, 0x8B, 0x5C, 0x24, 0x14, 0x8B, 0xE8, 0x69 };                //21:9 (3840x1600)
        edit4.offset4 = 0;
        break;
    case 6:
        edit4.modified4 = { 0x24, 0x10, 0xC7, 0x47, 0x18, 0x39, 0x8E, 0x63, 0x40, 0x8B, 0x5C, 0x24, 0x14, 0x8B, 0xE8, 0x69 };                //32:10
        edit4.offset4 = 0;
        break;
    default:
        DX_ERROR("Invalid resolution index.")

            break;
    }

    return edit4;
}

void PerformHexEdit4(LPBYTE lpAddress, DWORD moduleSize, const HexEdit4& edit4) {
    for (DWORD i = 0; i < moduleSize - edit4.modified4.size(); ++i) {
        if (memcmp(lpAddress + i, commonHexEdit4.data(), commonHexEdit4.size()) == 0) {
            DX_PRINT("Pattern found in memory.")

                LPVOID lpAddressToWrite = lpAddress + i + edit4.offset4;
            SIZE_T numberOfBytesWritten;
            BOOL result = WriteProcessMemory(GetCurrentProcess(), lpAddressToWrite, edit4.modified4.data(), edit4.modified4.size(), &numberOfBytesWritten);
            if (!result || numberOfBytesWritten != edit4.modified4.size()) {
                DX_ERROR("Failed to write memory.")
                    return;
            }
            DX_PRINT("Hex edited successfully.")
                return;
        }
    }
    DX_ERROR("Pattern not found in memory.")
}

void PerformHexEdits4() {
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL) {
        DX_ERROR("Failed to get module handle.")
            return;
    }

    // Get the module information
    LPBYTE lpAddress = reinterpret_cast<LPBYTE>(hModule);
    DWORD moduleSize = 0;
    TCHAR szFileName[MAX_PATH];
    if (GetModuleFileNameEx(GetCurrentProcess(), hModule, szFileName, MAX_PATH)) {
        moduleSize = GetFileSize(szFileName, NULL);
    }
    if (moduleSize == 0) {
        DX_ERROR("Failed to get module information.")
            return;
    }

    // ini
    char path[MAX_PATH];
    HMODULE hm = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&Direct3DCreate8, &hm);
    GetModuleFileNameA(hm, path, sizeof(path));
    strcpy(strrchr(path, '\\'), "\\d3d8.ini");

    // Read resolution index from the INI file
    int aspectIndex = GetPrivateProfileInt("FULLSCREENASPECTRATIO", "fullscreenaspectratio", 0, path);
    if (aspectIndex == 0) {
        DX_ERROR("Failed to read aspect index from INI file.")
            return;
    }

    HexEdit4 edit = CreateHexEditFromAspect(aspectIndex);
    if (edit.modified4.empty()) {
        DX_ERROR("Failed to create hex edit for aspect index: ")
            return;
    }

    PerformHexEdit4(lpAddress, moduleSize, edit);
}

// chip - 4: aspect ratio

//=======================================================================================================================================================================================

//=======================================================================================================================================================================================
// chip - 5: fov

// Function to read the float value from the INI file and convert it to float
float ReadFloatFromINI(const char* section, const char* key, const char* iniPath) {
    int intValue = GetPrivateProfileIntA(section, key, 0, iniPath);
    return static_cast<float>(intValue);
}

// Function to perform the hex edit
void PerformHexEdit5() {
    // INI file path
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&Direct3DCreate8, &hm)) {
        DX_ERROR("Failed to get module handle.")
            return;
    }
    if (!GetModuleFileNameA(hm, path, sizeof(path))) {
        DX_ERROR("Failed to get module file name.")
            return;
    }
    char* lastBackslash = strrchr(path, '\\');
    if (lastBackslash == NULL) {
        DX_ERROR("Invalid module file path.")
            return;
    }
    strcpy_s(lastBackslash + 1, sizeof(path) - (lastBackslash - path + 1), "d3d8.ini");

    // Read the new float value from the INI file
    float floatNewValue = ReadFloatFromINI("FOV", "fov", path);
    if (floatNewValue == 0) {
        DX_ERROR("Failed to read float value from INI file.")
            return;
    }

    // Define the offset to write the new float value
    DWORD offset = 0x005BF1A8;

    // Open the process to write memory
    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, GetCurrentProcessId());
    if (hProcess == NULL) {
        DX_ERROR("Failed to open process.")
            return;
    }

    // Change memory protection to allow writing
    DWORD oldProtect;
    if (!VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(offset), sizeof(float), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        DX_ERROR("Failed to change memory protection.")
            CloseHandle(hProcess);
        return;
    }

    // Write the new value to memory
    if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(offset), &floatNewValue, sizeof(float), NULL)) {
        DX_ERROR("Failed to write memory.")
            CloseHandle(hProcess);
        return;
    }

    // Restore original protection
    DWORD dummy;
    if (!VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(offset), sizeof(float), oldProtect, &dummy)) {
        DX_ERROR("Failed to restore memory protection.")
            CloseHandle(hProcess);
        return;
    }

    DX_PRINT("Value successfully updated.")

        CloseHandle(hProcess);
}

// chip - 5: fov
//=======================================================================================================================================================================================


// List of registered window classes and procedures
// WORD classAtom, ULONG_PTR WndProcPtr
std::vector<std::pair<WORD, ULONG_PTR>> WndProcList;

void HookModule(HMODULE hmod);

class FrameLimiter
{
private:
    static inline double TIME_Frequency = 0.0;
    static inline double TIME_Ticks = 0.0;
    static inline double TIME_Frametime = 0.0;

public:
    static inline ID3DXFont* pFPSFont = nullptr;
    static inline ID3DXFont* pTimeFont = nullptr;

public:
    enum FPSLimitMode { FPS_NONE, FPS_REALTIME, FPS_ACCURATE };
    static void Init(FPSLimitMode mode)
    {
        LARGE_INTEGER frequency;

        QueryPerformanceFrequency(&frequency);
        static constexpr auto TICKS_PER_FRAME = 1;
        auto TICKS_PER_SECOND = (TICKS_PER_FRAME * fFPSLimit);
        if (mode == FPS_ACCURATE)
        {
            TIME_Frametime = 1000.0 / (double)fFPSLimit;
            TIME_Frequency = (double)frequency.QuadPart / 1000.0; // ticks are milliseconds
        }
        else // FPS_REALTIME
        {
            TIME_Frequency = (double)frequency.QuadPart / (double)TICKS_PER_SECOND; // ticks are 1/n frames (n = fFPSLimit)
        }
        Ticks();
    }
    static DWORD Sync_RT()
    {
        DWORD lastTicks, currentTicks;
        LARGE_INTEGER counter;

        QueryPerformanceCounter(&counter);
        lastTicks = (DWORD)TIME_Ticks;
        TIME_Ticks = (double)counter.QuadPart / TIME_Frequency;
        currentTicks = (DWORD)TIME_Ticks;

        return (currentTicks > lastTicks) ? currentTicks - lastTicks : 0;
    }
    static DWORD Sync_SLP()
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        double millis_current = (double)counter.QuadPart / TIME_Frequency;
        double millis_delta = millis_current - TIME_Ticks;
        if (TIME_Frametime <= millis_delta)
        {
            TIME_Ticks = millis_current;
            return 1;
        }
        else if (TIME_Frametime - millis_delta > 2.0) // > 2ms
            Sleep(1); // Sleep for ~1ms
        else
            Sleep(0); // yield thread's time-slice (does not actually sleep)

        return 0;
    }
    static void ShowFPS(LPDIRECT3DDEVICE8 device)
    {
        static std::list<int> m_times;

        //https://github.com/microsoft/VCSamples/blob/master/VC2012Samples/Windows%208%20samples/C%2B%2B/Windows%208%20app%20samples/Direct2D%20geometry%20realization%20sample%20(Windows%208)/C%2B%2B/FPSCounter.cpp#L279
        LARGE_INTEGER frequency;
        LARGE_INTEGER time;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&time);

        if (m_times.size() == 50)
            m_times.pop_front();
        m_times.push_back(static_cast<int>(time.QuadPart));

        uint32_t fps = 0;
        if (m_times.size() >= 2)
            fps = static_cast<uint32_t>(0.5f + (static_cast<float>(m_times.size() - 1) * static_cast<float>(frequency.QuadPart)) / static_cast<float>(m_times.back() - m_times.front()));

        static int space = 0;
        if (!pFPSFont || !pTimeFont)
        {
            D3DDEVICE_CREATION_PARAMETERS cparams;
            RECT rect;
            device->GetCreationParameters(&cparams);
            GetClientRect(cparams.hFocusWindow, &rect);

            LOGFONT fps_font = { rect.bottom / 20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Arial" };
            LOGFONT time_font = { rect.bottom / 35, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Arial" };
            space = rect.bottom / 20 + 5;

            if (D3DXCreateFontIndirect(device, &fps_font, &pFPSFont) != D3D_OK)
                return;

            if (D3DXCreateFontIndirect(device, &time_font, &pTimeFont) != D3D_OK)
                return;
        }
        else
        {
            auto DrawTextOutline = [](ID3DXFont* pFont, FLOAT X, FLOAT Y, D3DXCOLOR dColor, CONST PCHAR cString, ...)
                {
                    const D3DXCOLOR BLACK(D3DCOLOR_XRGB(0, 0, 0));
                    CHAR cBuffer[101] = "";

                    va_list oArgs;
                    va_start(oArgs, cString);
                    _vsnprintf((cBuffer + strlen(cBuffer)), (sizeof(cBuffer) - strlen(cBuffer)), cString, oArgs);
                    va_end(oArgs);

                    RECT Rect[5] =
                    {
                        { X - 1, Y, X + 500.0f, Y + 50.0f },
                        { X, Y - 1, X + 500.0f, Y + 50.0f },
                        { X + 1, Y, X + 500.0f, Y + 50.0f },
                        { X, Y + 1, X + 500.0f, Y + 50.0f },
                        { X, Y, X + 500.0f, Y + 50.0f },
                    };

                    pFont->Begin();
                    if (dColor != BLACK)
                    {
                        for (auto i = 0; i < 4; i++)
                            pFont->DrawText(cBuffer, -1, &Rect[i], DT_NOCLIP, BLACK);
                    }

                    pFont->DrawText(cBuffer, -1, &Rect[4], DT_NOCLIP, dColor);
                    pFont->End();
                };

            static char str_format_fps[] = "%02d";
            static char str_format_time[] = "%.01f ms";
            static const D3DXCOLOR YELLOW(D3DCOLOR_XRGB(0xF7, 0xF7, 0));
            DrawTextOutline(pFPSFont, 10, 10, YELLOW, str_format_fps, fps);
            DrawTextOutline(pTimeFont, 10, space, YELLOW, str_format_time, (1.0f / fps) * 1000.0f);
        }
    }

private:
    static void Ticks()
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        TIME_Ticks = (double)counter.QuadPart / TIME_Frequency;
    }
};

FrameLimiter::FPSLimitMode mFPSLimitMode = FrameLimiter::FPSLimitMode::FPS_NONE;

HRESULT m_IDirect3DDevice8::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
    if (mFPSLimitMode == FrameLimiter::FPSLimitMode::FPS_REALTIME)
        while (!FrameLimiter::Sync_RT());
    else if (mFPSLimitMode == FrameLimiter::FPSLimitMode::FPS_ACCURATE)
        while (!FrameLimiter::Sync_SLP());

    return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT m_IDirect3DDevice8::EndScene()
{
    if (bDisplayFPSCounter)
        FrameLimiter::ShowFPS(ProxyInterface);

    return ProxyInterface->EndScene();
}

void ForceWindowed(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    HWND hwnd = pPresentationParameters->hDeviceWindow ? pPresentationParameters->hDeviceWindow : g_hFocusWindow;
    HMONITOR monitor = MonitorFromWindow((!bUsePrimaryMonitor && hwnd) ? hwnd : GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    int DesktopResX = info.rcMonitor.right - info.rcMonitor.left;
    int DesktopResY = info.rcMonitor.bottom - info.rcMonitor.top;

    int left = (int)info.rcMonitor.left;
    int top = (int)info.rcMonitor.top;

    if (!bBorderlessFullscreen)
    {
        left += (int)(((float)DesktopResX / 2.0f) - ((float)pPresentationParameters->BackBufferWidth / 2.0f));
        top += (int)(((float)DesktopResY / 2.0f) - ((float)pPresentationParameters->BackBufferHeight / 2.0f));
    }

    pPresentationParameters->Windowed = 1;

    // These must be set to default (0) on windowed mode as per spec to prevent app freezing
    pPresentationParameters->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    pPresentationParameters->FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (hwnd != NULL)
    {
        UINT uFlags = SWP_SHOWWINDOW;
        if (bBorderlessFullscreen)
        {
            LONG lOldStyle = GetWindowLong(hwnd, GWL_STYLE);
            LONG lOldExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            LONG lNewStyle = lOldStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_DLGFRAME);
            lNewStyle |= (lOldStyle & WS_CHILD) ? 0 : WS_POPUP;
            LONG lNewExStyle = lOldExStyle & ~(WS_EX_CONTEXTHELP | WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW);
            lNewExStyle |= WS_EX_APPWINDOW;

            if (lNewStyle != lOldStyle)
            {
                SetWindowLong(hwnd, GWL_STYLE, lNewStyle);
                uFlags |= SWP_FRAMECHANGED;
            }
            if (lNewExStyle != lOldExStyle)
            {
                SetWindowLong(hwnd, GWL_EXSTYLE, lNewExStyle);
                uFlags |= SWP_FRAMECHANGED;
            }
            SetWindowPos(hwnd, bAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, left, top, DesktopResX, DesktopResY, uFlags);
        }
        else
        {
            if (!bCenterWindow)
                uFlags |= SWP_NOMOVE;

            SetWindowPos(hwnd, bAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, left, top, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, uFlags);

            // Release the cursor confinement when the game window loses focus
            if (GetForegroundWindow() != hwnd)
                ClipCursor(nullptr);
            else {
                // Clip the cursor to the game window
                RECT windowRect;
                GetClientRect(hwnd, &windowRect);
                ClientToScreen(hwnd, reinterpret_cast<POINT*>(&windowRect.left));
                ClientToScreen(hwnd, reinterpret_cast<POINT*>(&windowRect.right));
                ClipCursor(&windowRect);
            }
        }
    }
}

void ForceFullScreenRefreshRateInHz(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    if (!pPresentationParameters->Windowed)
    {
        std::vector<int> list;
        DISPLAY_DEVICE dd;
        dd.cb = sizeof(DISPLAY_DEVICE);
        DWORD deviceNum = 0;
        while (EnumDisplayDevices(NULL, deviceNum, &dd, 0))
        {
            DISPLAY_DEVICE newdd = { 0 };
            newdd.cb = sizeof(DISPLAY_DEVICE);
            DWORD monitorNum = 0;
            DEVMODE dm = { 0 };
            while (EnumDisplayDevices(dd.DeviceName, monitorNum, &newdd, 0))
            {
                for (auto iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != 0; iModeNum++)
                    list.emplace_back(dm.dmDisplayFrequency);
                monitorNum++;
            }
            deviceNum++;
        }

        std::sort(list.begin(), list.end());
        if (nFullScreenRefreshRateInHz > list.back() || nFullScreenRefreshRateInHz < list.front() || nFullScreenRefreshRateInHz < 0)
            pPresentationParameters->FullScreen_RefreshRateInHz = list.back();
        else
            pPresentationParameters->FullScreen_RefreshRateInHz = nFullScreenRefreshRateInHz;
    }
}

HRESULT m_IDirect3D8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface)
{
    g_hFocusWindow = hFocusWindow ? hFocusWindow : pPresentationParameters->hDeviceWindow;
    if (bForceWindowedMode)
    {
        ForceWindowed(pPresentationParameters);
    }

    if (nFullScreenRefreshRateInHz)
        ForceFullScreenRefreshRateInHz(pPresentationParameters);

    if (bDisplayFPSCounter)
    {
        if (FrameLimiter::pFPSFont)
            FrameLimiter::pFPSFont->Release();
        if (FrameLimiter::pTimeFont)
            FrameLimiter::pTimeFont->Release();
        FrameLimiter::pFPSFont = nullptr;
        FrameLimiter::pTimeFont = nullptr;
    }

    HRESULT hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    if (SUCCEEDED(hr) && ppReturnedDeviceInterface)
    {
        *ppReturnedDeviceInterface = new m_IDirect3DDevice8(*ppReturnedDeviceInterface, this);
    }
    return hr;
}

HRESULT m_IDirect3DDevice8::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    if (bForceWindowedMode)
        ForceWindowed(pPresentationParameters);

    if (nFullScreenRefreshRateInHz)
        ForceFullScreenRefreshRateInHz(pPresentationParameters);

    if (bDisplayFPSCounter)
    {
        if (FrameLimiter::pFPSFont)
            FrameLimiter::pFPSFont->Release();
        if (FrameLimiter::pTimeFont)
            FrameLimiter::pTimeFont->Release();
        FrameLimiter::pFPSFont = nullptr;
        FrameLimiter::pTimeFont = nullptr;
    }

    return ProxyInterface->Reset(pPresentationParameters);
}

LRESULT WINAPI CustomWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int idx)
{
    if (hWnd == g_hFocusWindow || _fnIsTopLevelWindow(hWnd)) // skip child windows like buttons, edit boxes, etc. 
    {
        if (bAlwaysOnTop)
        {
            if ((GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0)
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        }
        switch (uMsg)
        {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                if ((HWND)lParam == NULL)
                    return 0;
                DWORD dwPID = 0;
                GetWindowThreadProcessId((HWND)lParam, &dwPID);
                if (dwPID != GetCurrentProcessId())
                    return 0;
            }
            break;
        case WM_NCACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                return 0;
            break;
        case WM_ACTIVATEAPP:
            if (wParam == FALSE)
                return 0;
            break;
        case WM_KILLFOCUS:
        {
            if ((HWND)wParam == NULL)
                return 0;
            DWORD dwPID = 0;
            GetWindowThreadProcessId((HWND)wParam, &dwPID);
            if (dwPID != GetCurrentProcessId())
                return 0;
        }
        break;
        default:
            break;
        }
    }
    WNDPROC OrigProc = WNDPROC(WndProcList[idx].second);
    return OrigProc(hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI CustomWndProcA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WORD wClassAtom = GetClassWord(hWnd, GCW_ATOM);
    if (wClassAtom)
    {
        for (unsigned int i = 0; i < WndProcList.size(); i++) {
            if (WndProcList[i].first == wClassAtom) {
                return CustomWndProc(hWnd, uMsg, wParam, lParam, i);
            }
        }
    }
    // We should never reach here, but having safeguards anyway is good
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI CustomWndProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WORD wClassAtom = GetClassWord(hWnd, GCW_ATOM);
    if (wClassAtom)
    {
        for (unsigned int i = 0; i < WndProcList.size(); i++) {
            if (WndProcList[i].first == wClassAtom) {
                return CustomWndProc(hWnd, uMsg, wParam, lParam, i);
            }
        }
    }
    // We should never reach here, but having safeguards anyway is good
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

typedef ATOM(__stdcall* RegisterClassA_fn)(const WNDCLASSA*);
typedef ATOM(__stdcall* RegisterClassW_fn)(const WNDCLASSW*);
typedef ATOM(__stdcall* RegisterClassExA_fn)(const WNDCLASSEXA*);
typedef ATOM(__stdcall* RegisterClassExW_fn)(const WNDCLASSEXW*);
RegisterClassA_fn oRegisterClassA = NULL;
RegisterClassW_fn oRegisterClassW = NULL;
RegisterClassExA_fn oRegisterClassExA = NULL;
RegisterClassExW_fn oRegisterClassExW = NULL;
ATOM __stdcall hk_RegisterClassA(WNDCLASSA* lpWndClass)
{
    if (!IsValueIntAtom(DWORD(lpWndClass->lpszClassName))) { // argument is a class name
        if (IsSystemClassNameA(lpWndClass->lpszClassName)) { // skip system classes like buttons, common controls, etc.
            return oRegisterClassA(lpWndClass);
        }
    }
    ULONG_PTR pWndProc = ULONG_PTR(lpWndClass->lpfnWndProc);
    lpWndClass->lpfnWndProc = CustomWndProcA;
    WORD wClassAtom = oRegisterClassA(lpWndClass);
    if (wClassAtom != 0)
    {
        WndProcList.emplace_back(wClassAtom, pWndProc);
    }
    return wClassAtom;
}
ATOM __stdcall hk_RegisterClassW(WNDCLASSW* lpWndClass)
{
    if (!IsValueIntAtom(DWORD(lpWndClass->lpszClassName))) { // argument is a class name
        if (IsSystemClassNameW(lpWndClass->lpszClassName)) { // skip system classes like buttons, common controls, etc.
            return oRegisterClassW(lpWndClass);
        }
    }
    ULONG_PTR pWndProc = ULONG_PTR(lpWndClass->lpfnWndProc);
    lpWndClass->lpfnWndProc = CustomWndProcW;
    WORD wClassAtom = oRegisterClassW(lpWndClass);
    if (wClassAtom != 0)
    {
        WndProcList.emplace_back(wClassAtom, pWndProc);
    }
    return wClassAtom;
}
ATOM __stdcall hk_RegisterClassExA(WNDCLASSEXA* lpWndClass)
{
    if (!IsValueIntAtom(DWORD(lpWndClass->lpszClassName))) { // argument is a class name
        if (IsSystemClassNameA(lpWndClass->lpszClassName)) { // skip system classes like buttons, common controls, etc.
            return oRegisterClassExA(lpWndClass);
        }
    }
    ULONG_PTR pWndProc = ULONG_PTR(lpWndClass->lpfnWndProc);
    lpWndClass->lpfnWndProc = CustomWndProcA;
    WORD wClassAtom = oRegisterClassExA(lpWndClass);
    if (wClassAtom != 0)
    {
        WndProcList.emplace_back(wClassAtom, pWndProc);
    }
    return wClassAtom;
}
ATOM __stdcall hk_RegisterClassExW(WNDCLASSEXW* lpWndClass)
{
    if (!IsValueIntAtom(DWORD(lpWndClass->lpszClassName))) { // argument is a class name
        if (IsSystemClassNameW(lpWndClass->lpszClassName)) { // skip system classes like buttons, common controls, etc.
            return oRegisterClassExW(lpWndClass);
        }
    }
    ULONG_PTR pWndProc = ULONG_PTR(lpWndClass->lpfnWndProc);
    lpWndClass->lpfnWndProc = CustomWndProcW;
    WORD wClassAtom = oRegisterClassExW(lpWndClass);
    if (wClassAtom != 0)
    {
        WndProcList.emplace_back(wClassAtom, pWndProc);
    }
    return wClassAtom;
}

typedef HWND(__stdcall* GetForegroundWindow_fn)(void);
GetForegroundWindow_fn oGetForegroundWindow = NULL;

HWND __stdcall hk_GetForegroundWindow()
{
    if (g_hFocusWindow && IsWindow(g_hFocusWindow))
        return g_hFocusWindow;
    return oGetForegroundWindow();
}

typedef HWND(__stdcall* GetActiveWindow_fn)(void);
GetActiveWindow_fn oGetActiveWindow = NULL;

HWND __stdcall hk_GetActiveWindow(void)
{
    HWND hWndActive = oGetActiveWindow();
    if (g_hFocusWindow && hWndActive == NULL && IsWindow(g_hFocusWindow))
    {
        if (GetCurrentThreadId() == GetWindowThreadProcessId(g_hFocusWindow, NULL))
            return g_hFocusWindow;
    }
    return hWndActive;
}

typedef HWND(__stdcall* GetFocus_fn)(void);
GetFocus_fn oGetFocus = NULL;

HWND __stdcall hk_GetFocus(void)
{
    HWND hWndFocus = oGetFocus();
    if (g_hFocusWindow && hWndFocus == NULL && IsWindow(g_hFocusWindow))
    {
        if (GetCurrentThreadId() == GetWindowThreadProcessId(g_hFocusWindow, NULL))
            return g_hFocusWindow;
    }
    return hWndFocus;
}

typedef HMODULE(__stdcall* LoadLibraryA_fn)(LPCSTR lpLibFileName);
LoadLibraryA_fn oLoadLibraryA;

HMODULE __stdcall hk_LoadLibraryA(LPCSTR lpLibFileName)
{
    HMODULE hmod = oLoadLibraryA(lpLibFileName);
    if (hmod)
    {
        HookModule(hmod);
    }
    return hmod;
}

typedef HMODULE(__stdcall* LoadLibraryW_fn)(LPCWSTR lpLibFileName);
LoadLibraryW_fn oLoadLibraryW;

HMODULE __stdcall hk_LoadLibraryW(LPCWSTR lpLibFileName)
{
    HMODULE hmod = oLoadLibraryW(lpLibFileName);
    if (hmod)
    {
        HookModule(hmod);
    }
    return hmod;
}

typedef HMODULE(__stdcall* LoadLibraryExA_fn)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExA_fn oLoadLibraryExA;

HMODULE __stdcall hk_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE hmod = oLoadLibraryExA(lpLibFileName, hFile, dwFlags);
    if (hmod && ((dwFlags & (LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)) == 0))
    {
        HookModule(hmod);
    }
    return hmod;
}

typedef HMODULE(__stdcall* LoadLibraryExW_fn)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExW_fn oLoadLibraryExW;

HMODULE __stdcall hk_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE hmod = oLoadLibraryExW(lpLibFileName, hFile, dwFlags);
    if (hmod && ((dwFlags & (LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)) == 0))
    {
        HookModule(hmod);
    }
    return hmod;
}

typedef BOOL(__stdcall* FreeLibrary_fn)(HMODULE hLibModule);
FreeLibrary_fn oFreeLibrary;

BOOL __stdcall hk_FreeLibrary(HMODULE hLibModule)
{
    if (hLibModule == g_hWrapperModule)
        return TRUE; // We will stay in memory, thank you very much

    return oFreeLibrary(hLibModule);
}

FARPROC __stdcall hk_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    __try
    {
        if (!lstrcmpA(lpProcName, "RegisterClassA"))
        {
            if (oRegisterClassA == NULL)
                oRegisterClassA = (RegisterClassA_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_RegisterClassA;
        }
        if (!lstrcmpA(lpProcName, "RegisterClassW"))
        {
            if (oRegisterClassW == NULL)
                oRegisterClassW = (RegisterClassW_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_RegisterClassW;
        }
        if (!lstrcmpA(lpProcName, "RegisterClassExA"))
        {
            if (oRegisterClassExA == NULL)
                oRegisterClassExA = (RegisterClassExA_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_RegisterClassExA;
        }
        if (!lstrcmpA(lpProcName, "RegisterClassExW"))
        {
            if (oRegisterClassExW == NULL)
                oRegisterClassExW = (RegisterClassExW_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_RegisterClassExW;
        }
        if (!lstrcmpA(lpProcName, "GetForegroundWindow"))
        {
            if (oGetForegroundWindow == NULL)
                oGetForegroundWindow = (GetForegroundWindow_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_GetForegroundWindow;
        }
        if (!lstrcmpA(lpProcName, "GetActiveWindow"))
        {
            if (oGetActiveWindow == NULL)
                oGetActiveWindow = (GetActiveWindow_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_GetActiveWindow;
        }
        if (!lstrcmpA(lpProcName, "GetFocus"))
        {
            if (oGetFocus == NULL)
                oGetFocus = (GetFocus_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_GetFocus;
        }
        if (!lstrcmpA(lpProcName, "LoadLibraryA"))
        {
            if (oLoadLibraryA == NULL)
                oLoadLibraryA = (LoadLibraryA_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_LoadLibraryA;
        }
        if (!lstrcmpA(lpProcName, "LoadLibraryW"))
        {
            if (oLoadLibraryW == NULL)
                oLoadLibraryW = (LoadLibraryW_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_LoadLibraryW;
        }
        if (!lstrcmpA(lpProcName, "LoadLibraryExA"))
        {
            if (oLoadLibraryExA == NULL)
                oLoadLibraryExA = (LoadLibraryExA_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_LoadLibraryExA;
        }
        if (!lstrcmpA(lpProcName, "LoadLibraryExW"))
        {
            if (oLoadLibraryExW == NULL)
                oLoadLibraryExW = (LoadLibraryExW_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_LoadLibraryExW;
        }
        if (!lstrcmpA(lpProcName, "FreeLibrary"))
        {
            if (oFreeLibrary == NULL)
                oFreeLibrary = (FreeLibrary_fn)GetProcAddress(hModule, lpProcName);
            return (FARPROC)hk_FreeLibrary;
        }
    }
    __except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
    }

    return GetProcAddress(hModule, lpProcName);
}

void HookModule(HMODULE hmod)
{
    char modpath[MAX_PATH + 1];
    if (hmod == g_hWrapperModule) // Yeah, let's not go and hook ourselves
        return;
    if (GetModuleFileNameA(hmod, modpath, MAX_PATH)) {
        if (!_strnicmp(modpath, WinDir, strlen(WinDir))) { // skip system modules
            return;
        }
    }
    if (oRegisterClassA == NULL)
        oRegisterClassA = (RegisterClassA_fn)Iat_hook::detour_iat_ptr("RegisterClassA", (void*)hk_RegisterClassA, hmod);
    else
        Iat_hook::detour_iat_ptr("RegisterClassA", (void*)hk_RegisterClassA, hmod);

    if (oRegisterClassW == NULL)
        oRegisterClassW = (RegisterClassW_fn)Iat_hook::detour_iat_ptr("RegisterClassW", (void*)hk_RegisterClassW, hmod);
    else
        Iat_hook::detour_iat_ptr("RegisterClassW", (void*)hk_RegisterClassW, hmod);

    if (oRegisterClassExA == NULL)
        oRegisterClassExA = (RegisterClassExA_fn)Iat_hook::detour_iat_ptr("RegisterClassExA", (void*)hk_RegisterClassExA, hmod);
    else
        Iat_hook::detour_iat_ptr("RegisterClassExA", (void*)hk_RegisterClassExA, hmod);

    if (oRegisterClassExW == NULL)
        oRegisterClassExW = (RegisterClassExW_fn)Iat_hook::detour_iat_ptr("RegisterClassExW", (void*)hk_RegisterClassExW, hmod);
    else
        Iat_hook::detour_iat_ptr("RegisterClassExW", (void*)hk_RegisterClassExW, hmod);

    if (oGetForegroundWindow == NULL)
        oGetForegroundWindow = (GetForegroundWindow_fn)Iat_hook::detour_iat_ptr("GetForegroundWindow", (void*)hk_GetForegroundWindow, hmod);
    else
        Iat_hook::detour_iat_ptr("GetForegroundWindow", (void*)hk_GetForegroundWindow, hmod);

    if (oGetActiveWindow == NULL)
        oGetActiveWindow = (GetActiveWindow_fn)Iat_hook::detour_iat_ptr("GetActiveWindow", (void*)hk_GetActiveWindow, hmod);
    else
        Iat_hook::detour_iat_ptr("GetActiveWindow", (void*)hk_GetActiveWindow, hmod);

    if (oGetFocus == NULL)
        oGetFocus = (GetFocus_fn)Iat_hook::detour_iat_ptr("GetFocus", (void*)hk_GetFocus, hmod);
    else
        Iat_hook::detour_iat_ptr("GetFocus", (void*)hk_GetFocus, hmod);

    if (oLoadLibraryA == NULL)
        oLoadLibraryA = (LoadLibraryA_fn)Iat_hook::detour_iat_ptr("LoadLibraryA", (void*)hk_LoadLibraryA, hmod);
    else
        Iat_hook::detour_iat_ptr("LoadLibraryA", (void*)hk_LoadLibraryA, hmod);

    if (oLoadLibraryW == NULL)
        oLoadLibraryW = (LoadLibraryW_fn)Iat_hook::detour_iat_ptr("LoadLibraryW", (void*)hk_LoadLibraryW, hmod);
    else
        Iat_hook::detour_iat_ptr("LoadLibraryW", (void*)hk_LoadLibraryW, hmod);

    if (oLoadLibraryExA == NULL)
        oLoadLibraryExA = (LoadLibraryExA_fn)Iat_hook::detour_iat_ptr("LoadLibraryExA", (void*)hk_LoadLibraryExA, hmod);
    else
        Iat_hook::detour_iat_ptr("LoadLibraryExA", (void*)hk_LoadLibraryExA, hmod);

    if (oLoadLibraryExW == NULL)
        oLoadLibraryExW = (LoadLibraryExW_fn)Iat_hook::detour_iat_ptr("LoadLibraryExW", (void*)hk_LoadLibraryExW, hmod);
    else
        Iat_hook::detour_iat_ptr("LoadLibraryExW", (void*)hk_LoadLibraryExW, hmod);

    if (oFreeLibrary == NULL)
        oFreeLibrary = (FreeLibrary_fn)Iat_hook::detour_iat_ptr("FreeLibrary", (void*)hk_FreeLibrary, hmod);
    else
        Iat_hook::detour_iat_ptr("FreeLibrary", (void*)hk_FreeLibrary, hmod);

    Iat_hook::detour_iat_ptr("GetProcAddress", (void*)hk_GetProcAddress, hmod);
}

void HookImportedModules()
{
    HMODULE hModule;
    HMODULE hm;

    hModule = GetModuleHandle(0);

    PIMAGE_DOS_HEADER img_dos_headers = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS img_nt_headers = (PIMAGE_NT_HEADERS)((BYTE*)img_dos_headers + img_dos_headers->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR img_import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)img_dos_headers + img_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    if (img_dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
        return;

    for (IMAGE_IMPORT_DESCRIPTOR* iid = img_import_desc; iid->Name != 0; iid++) {
        char* mod_name = (char*)((size_t*)(iid->Name + (size_t)hModule));
        hm = GetModuleHandleA(mod_name);
        // ual check
        if (hm && !(GetProcAddress(hm, "DirectInput8Create") != NULL && GetProcAddress(hm, "DirectSoundCreate8") != NULL && GetProcAddress(hm, "InternetOpenA") != NULL)) {
            HookModule(hm);
        }
    }
}

bool WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    static HMODULE d3d8dll = nullptr;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hWrapperModule = hModule;

        //========================================================================================================================
        //chip

        if (dwReason == DLL_PROCESS_ATTACH)
        {
            PerformHexEdits();

            PerformHexEdits2();

            PerformHexEdits3();

            PerformHexEdits4();

            PerformHexEdit5();


        }
        //========================================================================================================================


        // Load dll
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        d3d8dll = LoadLibraryA(path);

        // Get function addresses
        m_pDirect3D8EnableMaximizedWindowedModeShim = (Direct3D8EnableMaximizedWindowedModeShimProc)GetProcAddress(d3d8dll, "Direct3D8EnableMaximizedWindowedModeShim");
        m_pValidatePixelShader = (ValidatePixelShaderProc)GetProcAddress(d3d8dll, "ValidatePixelShader");
        m_pValidateVertexShader = (ValidateVertexShaderProc)GetProcAddress(d3d8dll, "ValidateVertexShader");
        m_pDebugSetMute = (DebugSetMuteProc)GetProcAddress(d3d8dll, "DebugSetMute");
        m_pDirect3DCreate8 = (Direct3DCreate8Proc)GetProcAddress(d3d8dll, "Direct3DCreate8");

        //ini
        HMODULE hm = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&Direct3DCreate8, &hm);
        GetModuleFileNameA(hm, path, sizeof(path));
        strcpy(strrchr(path, '\\'), "\\d3d8.ini");

        bForceWindowedMode = GetPrivateProfileInt("MAIN", "ForceWindowedMode", 0, path) != 0;
        bDirect3D8DisableMaximizedWindowedModeShim = GetPrivateProfileInt("MAIN", "Direct3D8DisableMaximizedWindowedModeShim", 0, path) != 0;
        fFPSLimit = static_cast<float>(GetPrivateProfileInt("MAIN", "FPSLimit", 0, path));
        nFullScreenRefreshRateInHz = GetPrivateProfileInt("MAIN", "FullScreenRefreshRateInHz", 0, path);
        bDisplayFPSCounter = GetPrivateProfileInt("MAIN", "DisplayFPSCounter", 0, path);
        bUsePrimaryMonitor = GetPrivateProfileInt("FORCEWINDOWED", "UsePrimaryMonitor", 0, path) != 0;
        bCenterWindow = GetPrivateProfileInt("FORCEWINDOWED", "CenterWindow", 1, path) != 0;
        bBorderlessFullscreen = GetPrivateProfileInt("FORCEWINDOWED", "BorderlessFullscreen", 0, path) != 0;
        bAlwaysOnTop = GetPrivateProfileInt("FORCEWINDOWED", "AlwaysOnTop", 0, path) != 0;
        bDoNotNotifyOnTaskSwitch = GetPrivateProfileInt("FORCEWINDOWED", "DoNotNotifyOnTaskSwitch", 0, path) != 0;

        if (fFPSLimit > 0.0f)
        {
            FrameLimiter::FPSLimitMode mode = (GetPrivateProfileInt("MAIN", "FPSLimitMode", 1, path) == 2) ? FrameLimiter::FPSLimitMode::FPS_ACCURATE : FrameLimiter::FPSLimitMode::FPS_REALTIME;
            if (mode == FrameLimiter::FPSLimitMode::FPS_ACCURATE)
                timeBeginPeriod(1);

            FrameLimiter::Init(mode);
            mFPSLimitMode = mode;
        }
        else
            mFPSLimitMode = FrameLimiter::FPSLimitMode::FPS_NONE;

        if (bDirect3D8DisableMaximizedWindowedModeShim)
        {
            auto addr = (uintptr_t)GetProcAddress(d3d8dll, "Direct3D8EnableMaximizedWindowedModeShim");
            if (addr)
            {
                DWORD Protect;
                VirtualProtect((LPVOID)(addr + 6), 4, PAGE_EXECUTE_READWRITE, &Protect);
                *(unsigned*)(addr + 6) = 0;
                *(unsigned*)(*(unsigned*)(addr + 2)) = 0;
                VirtualProtect((LPVOID)(addr + 6), 4, Protect, &Protect);
                bForceWindowedMode = false;
            }
        }

        if (bDoNotNotifyOnTaskSwitch)
        {
            GetSystemWindowsDirectoryA(WinDir, MAX_PATH);

            oRegisterClassA = (RegisterClassA_fn)Iat_hook::detour_iat_ptr("RegisterClassA", (void*)hk_RegisterClassA);
            oRegisterClassW = (RegisterClassW_fn)Iat_hook::detour_iat_ptr("RegisterClassW", (void*)hk_RegisterClassW);
            oRegisterClassExA = (RegisterClassExA_fn)Iat_hook::detour_iat_ptr("RegisterClassExA", (void*)hk_RegisterClassExA);
            oRegisterClassExW = (RegisterClassExW_fn)Iat_hook::detour_iat_ptr("RegisterClassExW", (void*)hk_RegisterClassExW);
            oGetForegroundWindow = (GetForegroundWindow_fn)Iat_hook::detour_iat_ptr("GetForegroundWindow", (void*)hk_GetForegroundWindow);
            oGetActiveWindow = (GetActiveWindow_fn)Iat_hook::detour_iat_ptr("GetActiveWindow", (void*)hk_GetActiveWindow);
            oGetFocus = (GetFocus_fn)Iat_hook::detour_iat_ptr("GetFocus", (void*)hk_GetFocus);
            oLoadLibraryA = (LoadLibraryA_fn)Iat_hook::detour_iat_ptr("LoadLibraryA", (void*)hk_LoadLibraryA);
            oLoadLibraryW = (LoadLibraryW_fn)Iat_hook::detour_iat_ptr("LoadLibraryW", (void*)hk_LoadLibraryW);
            oLoadLibraryExA = (LoadLibraryExA_fn)Iat_hook::detour_iat_ptr("LoadLibraryExA", (void*)hk_LoadLibraryExA);
            oLoadLibraryExW = (LoadLibraryExW_fn)Iat_hook::detour_iat_ptr("LoadLibraryExW", (void*)hk_LoadLibraryExW);
            oFreeLibrary = (FreeLibrary_fn)Iat_hook::detour_iat_ptr("FreeLibrary", (void*)hk_FreeLibrary);

            Iat_hook::detour_iat_ptr("GetProcAddress", (void*)hk_GetProcAddress);
            Iat_hook::detour_iat_ptr("GetProcAddress", (void*)hk_GetProcAddress, d3d8dll);

            if (oGetForegroundWindow == NULL)
                oGetForegroundWindow = (GetForegroundWindow_fn)Iat_hook::detour_iat_ptr("GetForegroundWindow", (void*)hk_GetForegroundWindow, d3d8dll);
            else
                Iat_hook::detour_iat_ptr("GetForegroundWindow", (void*)hk_GetForegroundWindow, d3d8dll);

            HMODULE ole32 = GetModuleHandleA("ole32.dll");
            if (ole32) {
                if (oRegisterClassA == NULL)
                    oRegisterClassA = (RegisterClassA_fn)Iat_hook::detour_iat_ptr("RegisterClassA", (void*)hk_RegisterClassA, ole32);
                else
                    Iat_hook::detour_iat_ptr("RegisterClassA", (void*)hk_RegisterClassA, ole32);
                if (oRegisterClassW == NULL)
                    oRegisterClassW = (RegisterClassW_fn)Iat_hook::detour_iat_ptr("RegisterClassW", (void*)hk_RegisterClassW, ole32);
                else
                    Iat_hook::detour_iat_ptr("RegisterClassW", (void*)hk_RegisterClassW, ole32);
                if (oRegisterClassExA == NULL)
                    oRegisterClassExA = (RegisterClassExA_fn)Iat_hook::detour_iat_ptr("RegisterClassExA", (void*)hk_RegisterClassExA, ole32);
                else
                    Iat_hook::detour_iat_ptr("RegisterClassExA", (void*)hk_RegisterClassExA, ole32);
                if (oRegisterClassExW == NULL)
                    oRegisterClassExW = (RegisterClassExW_fn)Iat_hook::detour_iat_ptr("RegisterClassExW", (void*)hk_RegisterClassExW, ole32);
                else
                    Iat_hook::detour_iat_ptr("RegisterClassExW", (void*)hk_RegisterClassExW, ole32);
                if (oGetActiveWindow == NULL)
                    oGetActiveWindow = (GetActiveWindow_fn)Iat_hook::detour_iat_ptr("GetActiveWindow", (void*)hk_GetActiveWindow, ole32);
                else
                    Iat_hook::detour_iat_ptr("GetActiveWindow", (void*)hk_GetActiveWindow, ole32);
            }

            HookImportedModules();
        }
    }
    break;
    case DLL_PROCESS_DETACH:
    {
        if (mFPSLimitMode == FrameLimiter::FPSLimitMode::FPS_ACCURATE)
            timeEndPeriod(1);

        FreeLibrary(d3d8dll);
    }
    break;
    }

    return true;
}

int WINAPI Direct3D8EnableMaximizedWindowedModeShim(BOOL mEnable)
{
    if (!m_pDirect3D8EnableMaximizedWindowedModeShim)
    {
        return E_FAIL;
    }

    return m_pDirect3D8EnableMaximizedWindowedModeShim(mEnable);
}

HRESULT WINAPI ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto)
{
    if (!m_pValidatePixelShader)
    {
        return E_FAIL;
    }

    return m_pValidatePixelShader(pixelshader, reserved1, flag, toto);
}

HRESULT WINAPI ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto)
{
    if (!m_pValidateVertexShader)
    {
        return E_FAIL;
    }

    return m_pValidateVertexShader(vertexshader, reserved1, reserved2, flag, toto);
}

void WINAPI DebugSetMute()
{
    if (!m_pDebugSetMute)
    {
        return;
    }

    return m_pDebugSetMute();
}

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion)
{
    if (!m_pDirect3DCreate8)
    {
        return nullptr;
    }

    IDirect3D8* pD3D8 = m_pDirect3DCreate8(SDKVersion);

    if (pD3D8)
    {
        return new m_IDirect3D8(pD3D8);
    }

    return nullptr;
}
