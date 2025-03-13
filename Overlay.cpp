#include "Overlay.h"

// DirectX11 Variablen
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Overlay Fenster-Variablen

int ValidEntsLevel;
int AllEntsLevel;


int Screen_w = 2560;
int Screen_h = 1440;
bool running = true;
float fps = 100.f;
FPSLimiter fpsLimiter(60.0f);
bool openMenu = false;
bool Clickability = false;
HWND hWndOverlay;
static int selectedLevelIndex = 0;  // Aktuell ausgewählter Level
static std::vector<std::string> levelNames;  // Liste der Levelnamen


bool CreateDeviceD3D(HWND hWnd)

{

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

    D3D_FEATURE_LEVEL featureLevel;

    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

    if (res == DXGI_ERROR_UNSUPPORTED)

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


// Fenster-Callback
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

// Hauptfunktion für das Overlay
void RenderOverlay()
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), ACS_TRANSPARENT, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, _T("Overlay"), NULL };
    RegisterClassEx(&wc);
    HWND hWndOverlay = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT,wc.lpszClassName,L"Overlay",WS_POPUP,0, 0, Screen_w, Screen_h,nullptr, nullptr, nullptr, nullptr);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hWndOverlay, &margins);

    if (!CreateDeviceD3D(hWndOverlay))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    ::ShowWindow(hWndOverlay, SW_SHOWDEFAULT);
    ::UpdateWindow(hWndOverlay);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_None;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hWndOverlay);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    DWORD size;
    int validEnts = 0;
    InitHax Hax;
    Hax.hProcess = Hax.GetAndLoadHax(L"Kingdom Come: Deliverance II");    
	MemoryManager MemManager(Hax.hProcess);
	RenderHelper renderHelper(Hax.hProcess);
    ExBytePatcher patcher(Hax.hProcess, MemManager.ModuleBaseAdresse + 0x4504B9, 6);  // 6-Byte NOP Patch
	FlyHack flyhack(Hax.hProcess, MemManager.ModuleBaseAdresse);
    MemManager.ModuleBaseAdresse = MemManager.GetModuleBaseAddressEx(L"WHGame.DLL", Hax.ProcID, size);
    EntityManager entityManager(Hax.hProcess, MemManager.ModuleBaseAdresse);
    FPSLimiter fpsLimiter(100.0f);

    MSG msg;
    while (running)
    {
        // ✅ Windows-Nachrichten verarbeiten
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }
        if (!running) break;

        
        renderHelper.ChangeClickability(hWndOverlay);
        ImGuiIO& IO = ImGui::GetIO();
        fpsLimiter.cap(IO);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ✅ DrawList
        ImDrawList* drawlist = ImGui::GetForegroundDrawList();



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
        ImGui::SliderFloat("FPS Limit", &fpsLimiter.fpsValue, 1.0f, 170.0f);
        fpsLimiter.setTargetFPS(fpsLimiter.fpsValue);
        ImGui::ColorEdit4("color", (float*)&clear_color);
        ImGui::Checkbox("ESP", &renderHelper.ESP_Status);
        ImGui::SliderFloat("Max Distance", &renderHelper.maxDistance, 50.f, 5000.f);
        ImGui::Checkbox("Use Cutscene Check", &renderHelper.useCutsceneCheck);
        if (ImGui::Checkbox("Flyhack", &flyhack.flyhackstatus))
        {
            patcher.TogglePatch(flyhack.flyhackstatus);
        }
        if (ImGui::Button("screenmaker", ImVec2(100, 20)))
        {
            HWND overlayWindow = FindWindow(NULL, L"DebugOverlay");
            if (overlayWindow) {
                DebugScreenshot::SaveOverlayScreenshot(overlayWindow, L"screen");
            }
        }
        ImGui::Text("CamPos: (%.3f, %.3f, %.3f)", renderHelper.CamPos.x, renderHelper.CamPos.y, renderHelper.CamPos.z);
        ImGui::Text("Valid Ents: %d", entityManager.validEntities);
        ImGui::End();
        if (flyhack.flyhackstatus)
        {
            FlyHack flyhack(Hax.hProcess, MemManager.ModuleBaseAdresse);
            flyhack.Update();
        }
        if (Hax.ProcID && renderHelper.ESP_Status) {
            renderHelper.CamPos = Hax.Read<Vector3>(renderHelper.CamPosAdr + MemManager.ModuleBaseAdresse);
            bool cutscene = Hax.Read<bool>(renderHelper.CutsceneActive + MemManager.ModuleBaseAdresse);
            renderHelper.MatrixAdr = MemoryManager::FindDMAAddy(Hax.hProcess, (MemManager.ModuleBaseAdresse + 0x0526C9A0), { 0x50,0x120,0xB0,0x4C8 });
            ReadProcessMemory(Hax.hProcess, (LPVOID)renderHelper.MatrixAdr, &renderHelper.Matrix, sizeof(renderHelper.Matrix), NULL);
            if (!cutscene || !renderHelper.useCutsceneCheck) {
                entityManager.SetCameraPosition(renderHelper.CamPos, renderHelper.maxDistance);
                entityManager.RenderEntities(drawlist, 2560, 1440, renderHelper.Matrix);
            }
        }
		renderHelper.RenderRotatingTriangle(drawlist, ImVec2(Screen_w, Screen_h));

        

        // ✅ Rendering
        float TransparentColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, TransparentColor);
        ImGui::Render();

        const float clear_color_with_alpha[4] = {
            clear_color.x * clear_color.w,
            clear_color.y * clear_color.w,
            clear_color.z * clear_color.w,
            clear_color.w
        };

        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(0, 0);
    }

    // **Schließe Overlay korrekt**
    PostMessage(hWndOverlay, WM_CLOSE, 0, 0);
    Sleep(100);
    DestroyWindow(hWndOverlay);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    // **DirectX-Ressourcen freigeben**
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
}