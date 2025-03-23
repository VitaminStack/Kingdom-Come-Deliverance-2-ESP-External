
#include <iostream>
#include "Overlay.h"
#include "Offset.h"

InitHax Hax;
DebugConsole debugConsole;

void OverlayThread() {
    RenderOverlay(Hax); // Dein Overlay-Loop
}

void CheatsThread(HANDLE hProcess) {
    
    uintptr_t hookAddress = 0x7FFC9582AFCF; // Beispiel-Adresse
    size_t stolenBytes = 16;             // Mindestens 14 für x64-Absprung


    // (3) Patcher-Objekt erzeugen
    ExBytePatcher patcher(hProcess, hookAddress, stolenBytes);

    // (4) Automatisch Code-Cave allozieren und Hook setzen
    if (patcher.AutoDetourFunction64(stolenBytes)) {
        std::cout << "[+] Detour erfolgreich gesetzt!\n";
    }
    else {
        std::cout << "[-] Detour fehlgeschlagen.\n";
    }
    while (true) {
        Sleep(1000);
    }
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Hax.hProcess = Hax.GetAndLoadHax(L"Kingdom Come: Deliverance II");
    debugConsole.setDebugging(true);

    std::thread cheatsThread(CheatsThread, Hax.hProcess);
    std::thread overlayThread(OverlayThread);

    // Main wartet nur auf Overlay
    overlayThread.join();

    // Bei Exit: ALLE Patches sauber entfernen!
    ExBytePatcher::RestoreAll();

    return 0;
}
