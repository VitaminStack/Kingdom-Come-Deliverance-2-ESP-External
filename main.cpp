// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "Helper.h"
#include <sstream>
#pragma comment(lib, "dwmapi.lib")
#include "Dwmapi.h"



// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
static ID3D11BlendState*        m_pBlendState = nullptr;


uintptr_t ModuleBaseAdresse = 0x0;
const wchar_t* ModuleName = L"WHGame.DLL";
uintptr_t CamPosAdr = 0x5209DA8;
uintptr_t CutsceneActive = 0x528C8D8;
static float maxDistance = 200.0f;
bool flyhack = false;

Vector3 CamPos = { 0.0f, 0.0f,0.0f };
float FOV = 0.0f;
Vector2 ScreenPos = { 0.f, 0.f };

uintptr_t MatrixAdr = 0x0;
float Matrix[16];

Vector2 Screen = { 2560, 1440 };
float FPSCap = 169.f;
bool openMenu = false;
bool Clickability = false;
bool demoWindow = false;
bool checkAdress = false;
bool useCutsceneCheck = true;
bool esp = true;

struct Ent {
    Vector3 Pos;
    uintptr_t EntBase;
    uintptr_t EntPosAdr;
};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void ChangeClickability(bool canclick, HWND ownd)
{
    long style = GetWindowLong(ownd, GWL_EXSTYLE);
    if (canclick) {
        style &= ~WS_EX_LAYERED;
        SetWindowLong(ownd, GWL_EXSTYLE, style);
        SetForegroundWindow(ownd);
        //windowstate = 1;
    }
    else {
        style |= WS_EX_LAYERED;
        SetWindowLong(ownd, GWL_EXSTYLE, style);
        //windowstate = 0;
    }
}
void SetOverlayToTarget(HWND WindowHandle, HWND OverlayHandle, Vector2& ScreenXY)
{
    RECT rect;
    GetWindowRect(WindowHandle, &rect);

    int Breite = (rect.right - rect.left);
    int Höhe = (rect.bottom - rect.top);

    ScreenXY.x = static_cast<float>(rect.right - rect.left);
    ScreenXY.y = static_cast<float>(rect.bottom - rect.top);

    MoveWindow(OverlayHandle, rect.left, rect.top, Breite, Höhe, true);
}
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();    
    
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), ACS_TRANSPARENT, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, _T("Overlay"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT, wc.lpszClassName, L"DebugOverlay", WS_POPUP, 0, 0, Screen.x, Screen.y, nullptr, nullptr, nullptr, nullptr);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);   
    ::UpdateWindow(hwnd);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_None;     // Enable Keyboard Controls

	ImFont* DefaultFont = io.Fonts->AddFontDefault();
    ImFont* MyArialFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arialbd.ttf", 18.0f); // Arial Bold
	
    
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    // Our state
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Main loop
    bool done = false;
    
    InitHax Hax;
    Hax.hProcess = Hax.GetAndLoadHax(L"Kingdom Come: Deliverance II");
    DWORD size;
    int validEnts = 0;
	ModuleBaseAdresse = GetModuleBaseAddressEx(ModuleName, Hax.ProcID, size);
    ExBytePatcher patcher(Hax.hProcess, ModuleBaseAdresse + 0x4504B9, 6);  // 6-Byte NOP Patch
    


    

            
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;       
        
        
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            Clickability = !Clickability;
            openMenu = !openMenu;
        }
        
        ChangeClickability(Clickability, hwnd);
        SetOverlayToTarget(Hax.TargetHWND, hwnd, Screen);


        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();        
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImDrawList* Drawlist = ImGui::GetForegroundDrawList();
        

        DefaultFont->Scale = 1.0f;
        ImGui::PushFont(DefaultFont);

        const char* MenuBar;
        if (Hax.ProcID)
        {
            MenuBar = "Game found!";
        }
        else
        {
            MenuBar = "Game not found!";
        }
        ImGui::Begin(MenuBar);
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::SliderFloat("FPS", &FPSCap, 1.f, 170.f);
        ImGui::ColorEdit4("color", (float*)&clear_color);

        // Slider für maximale Distance
		ImGui::Checkbox("ESP", &esp);
        ImGui::SliderFloat("Max Distance", &maxDistance, 50.f, 5000.f);
		ImGui::Checkbox("Use Cutscene Check", &useCutsceneCheck);
        if (ImGui::Checkbox("Flyhack", &flyhack))
        {
            patcher.TogglePatch(flyhack);
        }
        // Display CamPos
        ImGui::Text("CamPos: (%.3f, %.3f, %.3f)", CamPos.x, CamPos.y, CamPos.z);
        // Display TestPos
        ImGui::Text("ScreenPos: (%.3f, %.3f)", ScreenPos.x, ScreenPos.y);
        ImGui::Text("Valid Ents: %d", validEnts);
        ImGui::End();

        if (io.Framerate > FPSCap && FPSCap != 170.f)
        {
            Sleep(1);
        }       
        ImGui::PopFont();
		
        
        MyArialFont->Scale = 0.8f;
        ImGui::PushFont(MyArialFont);
        
        



        if (flyhack)
        {
            FlyHack flyhack(Hax.hProcess, ModuleBaseAdresse);
            flyhack.Update();
        }
        if (Hax.ProcID && esp)
        {
			CamPos = Hax.Read<Vector3>(CamPosAdr + ModuleBaseAdresse);
			bool Cutscene = Hax.Read<bool>(CutsceneActive + ModuleBaseAdresse);
                                    
			if (!Cutscene || !useCutsceneCheck)
			{
                const int entityCount = 2500;
                Ent entityArray[entityCount];
                ReadProcessMemory(Hax.hProcess, (LPVOID)MatrixAdr, &Matrix, sizeof(Matrix), NULL);
                MatrixAdr = FindDMAAddy(Hax.hProcess, (ModuleBaseAdresse + 0x0526C9A0), { 0x50,0x120,0xB0,0x4C8 });
                uintptr_t EntListAdr = FindDMAAddy(Hax.hProcess, (ModuleBaseAdresse + 0x052A39D0), { 0x0 });

                validEnts = 0;
                for (int i = 0; i < entityCount; ++i) {
                    uintptr_t pointerAddress = EntListAdr + (i * 0x8);
                    uintptr_t EntBase = 0x0;
                    if (ReadProcessMemory(Hax.hProcess, (LPVOID)(pointerAddress), &EntBase, sizeof(EntBase), nullptr)) {
                        if (!IsBadReadPtr(&EntBase, sizeof(uintptr_t)))
                        {
                            entityArray[i].EntBase = EntBase;
                            entityArray[i].EntPosAdr = FindDMAAddy(Hax.hProcess, entityArray[i].EntBase + 0x18, { 0x30 });

                            Vector3 pos = { 0,0,0 };

                            if (ReadProcessMemory(Hax.hProcess, (LPVOID)(entityArray[i].EntPosAdr), &pos, sizeof(pos), nullptr)) {
                                float Distance = Distance3D(CamPos, pos);
                                if (Distance < maxDistance)
                                {
                                    validEnts++;
                                    entityArray[i].Pos = pos;
                                    uintptr_t NameAdr = FindDMAAddy(Hax.hProcess, entityArray[i].EntBase + 0x18, { 0xE8, 0x0 });
                                    std::string EntName = Hax.ReadStringFromMemory(Hax.hProcess, NameAdr, 30);

                                    if (EntName.find("tvez") != std::string::npos || EntName.find("dog") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Dog " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 0, 255, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Deer") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Deer " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 255, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Hare") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Hare " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 255, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Cow") != std::string::npos || EntName.find("cow") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Cow " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 255, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Bull") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Bull " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 255, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Pig") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Pig " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 255, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("WildDog") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "WildDog " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(255, 0, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Sheep") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Sheep " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 255, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Wolf") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Wolf " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(255, 0, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Enemy") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Enemy " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(255, 0, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("man") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Man " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 0, 255, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("woman") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Woman " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 0, 255, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    if (EntName.find("Horse") != std::string::npos || EntName.find("horse") != std::string::npos)
                                    {
                                        std::ostringstream oss;
                                        oss << "Horse " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(0, 0, 255, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                    else
                                    {
                                        std::ostringstream oss;
                                        oss << EntName << " " << std::fixed << std::setprecision(2) << Distance << "m";
                                        std::string Text = oss.str();

                                        if (WorldToScreenFarCry(entityArray[i].Pos, ScreenPos, Matrix, Screen.x, Screen.y))
                                        {
                                            if (ScreenPos.x < 2560 && ScreenPos.x > 0 && ScreenPos.y < 1440 && ScreenPos.y > 0)
                                            {
                                                Drawlist->AddText(ImVec2(CalcMiddlePos(ScreenPos.x, Text.c_str()), ScreenPos.y), IM_COL32(255, 255, 0, 255), Text.c_str());
                                                continue;
                                            }

                                        }
                                    }
                                }
                            }
                        }       
                        
                    }
                }
			}           			
		}
		
		
        
        
        





        ImGui::PopFont();
        // Rendering
        // Clear render target with transparent color
        float TransparentColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, TransparentColor);
        ImGui::Render();
        float blend[4] = { 0 };
        

        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        //g_pSwapChain->Present(1, 0); // Present with vsync
        g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    // beende MinHook
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CloseHandle(Hax.hProcess);

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}
void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}
void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}
void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
