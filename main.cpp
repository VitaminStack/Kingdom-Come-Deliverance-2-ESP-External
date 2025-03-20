
#include <iostream>
#include "Overlay.h"


InitHax Hax;
DebugConsole debugConsole;




void OverlayThread() {
    RenderOverlay(Hax); // Dein Overlay-Loop
}

void CheatsThread(HANDLE hProcess) {
    RenderHelper rHelper;
    MemoryManager mManager(hProcess);

    std::map<std::string, uintptr_t> registerStorage;

    mManager.PatchAndCaptureAllRegisters((LPVOID)0x253F0010, 15, registerStorage);

	mManager.PatchWithCustomCode((LPVOID)0x253F003A, 15, { 0x90, 0x90, 0x90, 0x90, 0x90 });

    while (true) {

		DebugConsole::clearConsole();
        for (const auto& regPair : registerStorage) {
            DebugConsole::log(
                DebugConsole::LogLevel::Log_INFO,
                regPair.first + " Speicher-Adresse = " + DebugConsole::formatHex(regPair.second)
            );
        }


        Sleep(1000);
    }
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Prozess holen
    Hax.hProcess = Hax.GetAndLoadHax(L"Unbenannt - Editor");

    // Debug Console öffnen
    debugConsole.setDebugging(true);

    std::thread cheatsThread(CheatsThread, Hax.hProcess);
    std::thread overlayThread(OverlayThread);

    // Beide Threads laufen lassen
    cheatsThread.join();
    overlayThread.join();

    return 0;
}

