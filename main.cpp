
#include <iostream>
#include "Overlay.h"


InitHax Hax;
DebugConsole debugConsole;




void OverlayThread() {
    RenderOverlay(Hax); // Dein Overlay-Loop
}

void CheatsThread(HANDLE hProcess) {
    

    while (true) {

		


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
    //cheatsThread.join();
    overlayThread.join();

    return 0;
}

