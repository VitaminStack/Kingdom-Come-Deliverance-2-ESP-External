
#include <iostream>
#include "Overlay.h"


InitHax Hax;
DebugConsole debugConsole;


// Funktion zum manuellen Patchen von Bytes
bool PatchBytes(HANDLE hProcess, uintptr_t destinationAddress, const std::vector<uint8_t>& patchBytes) {
    DWORD oldProtect;
    SIZE_T patchSize = patchBytes.size();

    // Speicherschutz anpassen, damit wir in den Zielbereich schreiben können.
    if (!VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(destinationAddress), patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        printf("Fehler beim Ändern des Speicherschutzes an Adresse %p.\n", reinterpret_cast<void*>(destinationAddress));
        return false;
    }

    // Patch-Bytes in den Zielprozess schreiben.
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(destinationAddress), patchBytes.data(), patchSize, &bytesWritten) || bytesWritten != patchSize) {
        printf("Fehler beim Schreiben der Patch-Bytes.\n");
        // Versuche den ursprünglichen Speicherschutz wiederherzustellen.
        VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(destinationAddress), patchSize, oldProtect, &oldProtect);
        return false;
    }

    // Ursprünglichen Speicherschutz wiederherstellen.
    VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(destinationAddress), patchSize, oldProtect, &oldProtect);
    return true;
}
// 1) ReadBytes-Funktion zum Einlesen beliebiger Bytes aus einem Prozess
std::vector<uint8_t> ReadBytes(HANDLE hProcess, uintptr_t address, size_t length)
{
    std::vector<uint8_t> buffer(length);
    SIZE_T bytesRead = 0;

    if (!ReadProcessMemory(hProcess,
        reinterpret_cast<LPCVOID>(address),
        buffer.data(),
        length,
        &bytesRead))
    {
        // Fehler beim Lesen
        printf("Fehler beim Lesen von Bytes an Adresse %p.\n",
            reinterpret_cast<void*>(address));
        return {};
    }

    // Wenn weniger als angefordert gelesen wurde, Vektor entsprechend verkleinern
    if (bytesRead < length)
    {
        buffer.resize(bytesRead);
    }

    return buffer;
}

// 2) Deine WriteJump64-Funktion (unverändert)
void WriteJump64(HANDLE hProcess, uintptr_t sourceAddress, uintptr_t destinationAddress)
{
    // Absolute Jump über RIP: FF 25 00 00 00 00 + 8-Byte-Zieladresse
    BYTE jmpInstruction[14] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // FF 25 [RIP+0]
        0, 0, 0, 0, 0, 0, 0, 0              // Platz für Zieladresse (8 Bytes)
    };

    // Zieladresse in die letzten 8 Bytes kopieren
    memcpy(&jmpInstruction[6], &destinationAddress, sizeof(uintptr_t));

    // Jump an sourceAddress in den Zielprozess schreiben
    WriteProcessMemory(hProcess,
        reinterpret_cast<LPVOID>(sourceAddress),
        jmpInstruction,
        sizeof(jmpInstruction),
        nullptr);
}

// 3) Angepasste DetourHook-Funktion
bool DetourHook(HANDLE hProcess, void* pTarget, size_t byteCount)
{
    // WriteJump64 benötigt einen 14-Byte-Patch, daher muss byteCount mindestens 14 sein.
    if (byteCount < 14) {
        printf("Fehler: byteCount muss mindestens 14 Bytes betragen.\n");
        return false;
    }

    DWORD oldProtect;
    // Ändere den Speicherschutz des Zielbereichs, um Schreibzugriff zu ermöglichen.
    if (!VirtualProtectEx(hProcess, pTarget, byteCount, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        printf("Fehler beim Ändern des Speicherschutzes.\n");
        return false;
    }

    // 1. Originalcode aus dem Zielprozess lesen
    std::vector<uint8_t> originalBytes = ReadBytes(
        hProcess,
        reinterpret_cast<uintptr_t>(pTarget),
        byteCount
    );

    if (originalBytes.empty()) {
        printf("Fehler beim Lesen der Original-Bytes.\n");
        return false;
    }

    // 2. Codecave allozieren (Originalcode + 14 Bytes für Rücksprung)
    void* pCodeCave = VirtualAllocEx(
        hProcess,
        nullptr,
        byteCount + 14,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );
    if (!pCodeCave) {
        printf("Fehler beim Allokieren der Codecave.\n");
        return false;
    }

    // 3. Originalcode in die Codecave schreiben
    SIZE_T bytesWritten = 0;
    if (!WriteProcessMemory(
        hProcess,
        pCodeCave,
        originalBytes.data(),   // Wichtig: .data() für Pointer auf die Bytes
        byteCount,
        &bytesWritten
    ) || bytesWritten != byteCount)
    {
        printf("Fehler beim Schreiben der Original-Bytes in die Codecave.\n");
        return false;
    }

    // 4. Rücksprung in der Codecave einfügen
    uintptr_t codeCaveAddr = reinterpret_cast<uintptr_t>(pCodeCave);
    uintptr_t jumpBackSource = codeCaveAddr + byteCount;
    uintptr_t jumpBackDest = reinterpret_cast<uintptr_t>(pTarget) + byteCount;
    WriteJump64(hProcess, jumpBackSource, jumpBackDest);

    // 5. Originalcode überschreiben: Springe zur Codecave
    uintptr_t targetAddress = reinterpret_cast<uintptr_t>(pTarget);
    WriteJump64(hProcess, targetAddress, codeCaveAddr);

    // Falls byteCount > 14, überschüssige Bytes mit NOPs (0x90) auffüllen
    if (byteCount > 14)
    {
        size_t nopCount = byteCount - 14;
        std::vector<uint8_t> nops(nopCount, 0x90);
        SIZE_T nopBytesWritten = 0;
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(targetAddress + 14), nops.data(), nops.size(), &nopBytesWritten);
    }

    // 6. Ursprünglichen Speicherschutz wiederherstellen
    DWORD temp;
    VirtualProtectEx(hProcess, pTarget, byteCount, oldProtect, &temp);

    return true;
}

void OverlayThread() {
    RenderOverlay(Hax); // Dein Overlay-Loop
}


std::vector<uint8_t> SavedBytes;
uintptr_t targetfunction = 0x7FF80A1980CF;

void CheatsThread(HANDLE hProcess) {
    
    SavedBytes = ReadBytes(hProcess, targetfunction, 17);
	DetourHook(hProcess, (void*)targetfunction, 17);

    
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


    PatchBytes(Hax.hProcess, targetfunction, SavedBytes);

    return 0;
}
