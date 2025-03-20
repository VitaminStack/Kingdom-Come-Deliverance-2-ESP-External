
#include <iostream>
#include "Overlay.h"


InitHax Hax;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    //Hax.hProcess = Hax.GetAndLoadHax(L"Kingdom Come: Deliverance II");    
    Hax.hProcess = Hax.GetAndLoadHax(L"Rechner");
    RenderHelper rHelper;
    MemoryManager mManager(Hax.hProcess);


    RenderOverlay(Hax);
    return 0;
}