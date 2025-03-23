
#include <iostream>
#include "Overlay.h"


InitHax Hax;
DebugConsole debugConsole;


class BytePatcher {
public:
    // Wichtige Variablen – öffentlich zugänglich für spätere Anpassungen
    HANDLE hProcess;         // Handle des Zielprozesses
    uintptr_t targetAddress; // Adresse, die gepatched wird
    size_t byteCount;        // Anzahl der Bytes, die überschrieben werden sollen
    uintptr_t codecaveAddress; // Adresse der allokierten Codecave

    // Konstruktor: Erwartet einen gültigen Prozesshandle
    BytePatcher(HANDLE process)
        : hProcess(process), targetAddress(0), byteCount(0), codecaveAddress(0) {
    }

    // Funktion zum manuellen Patchen von Bytes
    bool PatchBytes(uintptr_t destinationAddress, const std::vector<uint8_t>& patchBytes) {
        DWORD oldProtect;
        SIZE_T patchSize = patchBytes.size();

        // Speicherschutz anpassen, damit wir in den Zielbereich schreiben können.
        if (!VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(destinationAddress),
            patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            printf("Fehler beim Ändern des Speicherschutzes an Adresse %p.\n", reinterpret_cast<void*>(destinationAddress));
            return false;
        }

        SIZE_T bytesWritten = 0;
        // Patch-Bytes in den Zielprozess schreiben.
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(destinationAddress),
            patchBytes.data(), patchSize, &bytesWritten) || bytesWritten != patchSize) {
            printf("Fehler beim Schreiben der Patch-Bytes.\n");
            VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(destinationAddress),
                patchSize, oldProtect, &oldProtect);
            return false;
        }

        // Ursprünglichen Speicherschutz wiederherstellen.
        VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(destinationAddress),
            patchSize, oldProtect, &oldProtect);
        return true;
    }

    // Liest 'length' Bytes aus dem Prozess und gibt sie in einem std::vector<uint8_t> zurück.
    std::vector<uint8_t> ReadBytes(uintptr_t address, size_t length) {
        std::vector<uint8_t> buffer(length);
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address),
            buffer.data(), length, &bytesRead)) {
            printf("Fehler beim Lesen von Bytes an Adresse %p.\n", reinterpret_cast<void*>(address));
            return {};
        }
        if (bytesRead < length)
            buffer.resize(bytesRead);
        return buffer;
    }

    // Schreibt einen 14-Byte-Jump (64-Bit) von sourceAddress zu destinationAddress.
    void WriteJump64(uintptr_t sourceAddress, uintptr_t destinationAddress) {
        BYTE jmpInstruction[14] = {
            0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // RIP-relative indirekter Jump
            0, 0, 0, 0, 0, 0, 0, 0              // Platz für Zieladresse (8 Bytes)
        };
        memcpy(&jmpInstruction[6], &destinationAddress, sizeof(uintptr_t));
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(sourceAddress),
            jmpInstruction, sizeof(jmpInstruction), nullptr);
    }
    // 3) Angepasste DetourHook-Funktion
    bool DetourHook(void* pTarget, size_t byteCount)
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
        WriteJump64(jumpBackSource, jumpBackDest);

        // 5. Originalcode überschreiben: Springe zur Codecave
        uintptr_t targetAddress = reinterpret_cast<uintptr_t>(pTarget);
        WriteJump64(targetAddress, codeCaveAddr);

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
    std::vector<uintptr_t> DetourHookWithRegisters(void* pTarget, size_t byteCount, const std::vector<int>& registersToSave)
    {
        if (byteCount < 14) {
            printf("Fehler: byteCount muss mindestens 14 Bytes betragen.\n");
            return {};
        }
        DWORD oldProtect;
        if (!VirtualProtectEx(hProcess, pTarget, byteCount, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            printf("Fehler beim Ändern des Speicherschutzes.\n");
            return {};
        }

        std::vector<uint8_t> originalBytes = ReadBytes(reinterpret_cast<uintptr_t>(pTarget), byteCount);
        if (originalBytes.empty()) {
            printf("Fehler beim Lesen der Original-Bytes.\n");
            return {};
        }

        // Größe des Speicherbereichs für die Registerwerte
        const size_t regDumpDataSize = registersToSave.size() * sizeof(uint64_t);
        // Erzeuge den Code zum Speichern der Register (Platzhalter)
        std::vector<uint8_t> regSaveCode = GenerateRegisterSaveCodeOnlyRSI();
        size_t regSaveCodeSize = regSaveCode.size();

        // Gesamtgröße der Codecave: [regSaveCode] + [Originalcode] + [14 Byte Rücksprung] + [Register-Daten]
        size_t newAllocationSize = regSaveCodeSize + byteCount + 14 + regDumpDataSize;
        void* pCodeCave = VirtualAllocEx(hProcess, nullptr, newAllocationSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!pCodeCave) {
            printf("Fehler beim Allokieren der Codecave.\n");
            return {};
        }
        uintptr_t pCodeCaveAddr = reinterpret_cast<uintptr_t>(pCodeCave);

        // Adresse, an der die Registerwerte abgelegt werden
        uintptr_t regDumpDataAddr = pCodeCaveAddr + regSaveCodeSize + byteCount + 14;
        // Patch: Schreibe regDumpDataAddr in den regSaveCode (bei Offset 2, 8 Byte, little-endian)
        for (int i = 0; i < 8; i++) {
            regSaveCode[2 + i] = (regDumpDataAddr >> (8 * i)) & 0xFF;
        }

        SIZE_T bytesWritten = 0;
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(pCodeCaveAddr), regSaveCode.data(), regSaveCodeSize, &bytesWritten)
            || bytesWritten != regSaveCodeSize)
        {
            printf("Fehler beim Schreiben des Register-Save-Codes.\n");
            return {};
        }

        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(pCodeCaveAddr + regSaveCodeSize), originalBytes.data(), byteCount, &bytesWritten)
            || bytesWritten != byteCount)
        {
            printf("Fehler beim Schreiben der Original-Bytes.\n");
            return {};
        }

        uintptr_t jumpBackSource = pCodeCaveAddr + regSaveCodeSize + byteCount;
        uintptr_t jumpBackDest = reinterpret_cast<uintptr_t>(pTarget) + byteCount;
        WriteJump64(jumpBackSource, jumpBackDest);

        uintptr_t targetAddress = reinterpret_cast<uintptr_t>(pTarget);
        WriteJump64(targetAddress, pCodeCaveAddr);

        if (byteCount > 14) {
            size_t nopCount = byteCount - 14;
            std::vector<uint8_t> nops(nopCount, 0x90);
            SIZE_T nopBytesWritten = 0;
            WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(targetAddress + 14), nops.data(), nops.size(), &nopBytesWritten);
        }

        DWORD temp;
        VirtualProtectEx(hProcess, pTarget, byteCount, oldProtect, &temp);

        std::vector<uintptr_t> registerAddresses;
        for (size_t i = 0; i < registersToSave.size(); i++) {
            registerAddresses.push_back(regDumpDataAddr + i * sizeof(uint64_t));
        }
        return registerAddresses;
    }

    // Erzeugt den Register-Save-Code; registersToSave legt fest, wie viele Register gespeichert werden.
    std::vector<uint8_t> GenerateRegisterSaveCode(const std::vector<int>& registersToSave)
    {
        std::vector<uint8_t> code;
        // mov r11, <reg_dump_addr> (Platzhalter: 0)
        code.push_back(0x49);
        code.push_back(0xBB);
        for (int i = 0; i < 8; i++) {
            code.push_back(0x00);
        }
        // Für jedes Register: 6 Bytes Platzhalter (NOPs)
        for (size_t i = 0; i < registersToSave.size(); i++) {
            for (int j = 0; j < 6; j++) {
                code.push_back(0x90);
            }
        }
        return code;
    }
    std::vector<uint8_t> GenerateRegisterSaveCodeOnlyRSI()
    {
        std::vector<uint8_t> code;

        // mov r11, <reg_dump_addr> (10 Bytes: 49 BB + 8-Byte-Value)
        code.push_back(0x49);
        code.push_back(0xBB);
        // Hier 8x 0x00 als Platzhalter, wird später gepatcht
        for (int i = 0; i < 8; i++)
            code.push_back(0x00);

        // mov [r11], rsi (3 Bytes: 49 89 33)
        code.push_back(0x49);
        code.push_back(0x89);
        code.push_back(0x33);

        return code;
    }
    bool DetourHookWithFillers(void* pTarget, size_t byteCount) {
        if (byteCount < 14) {
            printf("Fehler: byteCount muss mindestens 14 Bytes betragen.\n");
            return false;
        }

        this->targetAddress = reinterpret_cast<uintptr_t>(pTarget);
        this->byteCount = byteCount;

        DWORD oldProtect;
        if (!VirtualProtectEx(hProcess, pTarget, byteCount, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            printf("Fehler beim Ändern des Speicherschutzes.\n");
            return false;
        }

        // 1. Originalcode aus dem Zielprozess lesen.
        std::vector<uint8_t> originalBytes = ReadBytes(this->targetAddress, byteCount);
        if (originalBytes.empty()) {
            printf("Fehler beim Lesen des Originalcodes.\n");
            return false;
        }

        // 2. Definiere Füllbereiche (hier als Beispiel NOPs)
        std::vector<uint8_t> preOrigFill = { 0x90, 0x90, 0x90 }; // z. B. 3 NOPs vor dem Originalcode
        std::vector<uint8_t> postOrigFill = { 0x90, 0x90, 0x90 }; // z. B. 3 NOPs nach dem Originalcode
        std::vector<uint8_t> preJumpFill = { 0x90, 0x90, 0x90 }; // z. B. 3 NOPs vor dem Jump Back
        std::vector<uint8_t> postJumpFill = { 0x90, 0x90, 0x90 }; // z. B. 3 NOPs nach dem Jump Back

        // 3. Gesamtgröße der Codecave berechnen:
        //    Gesamt = preOrigFill + Originalcode + postOrigFill + preJumpFill + 14 (Jump Back) + postJumpFill
        size_t codecaveSize = preOrigFill.size() + byteCount + postOrigFill.size() +
            preJumpFill.size() + 14 + postJumpFill.size();

        void* pCodecave = VirtualAllocEx(hProcess, nullptr, codecaveSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!pCodecave) {
            printf("Fehler beim Allokieren der Codecave.\n");
            return false;
        }
        this->codecaveAddress = reinterpret_cast<uintptr_t>(pCodecave);

        SIZE_T bytesWritten = 0;
        // 4. Pre-Original-Filler schreiben.
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(this->codecaveAddress),
            preOrigFill.data(), preOrigFill.size(), &bytesWritten) || bytesWritten != preOrigFill.size()) {
            printf("Fehler beim Schreiben des preOrigFill.\n");
            return false;
        }

        // 5. Originalcode in die Codecave schreiben.
        uintptr_t origCodeAddr = this->codecaveAddress + preOrigFill.size();
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(origCodeAddr),
            originalBytes.data(), byteCount, &bytesWritten) || bytesWritten != byteCount) {
            printf("Fehler beim Schreiben des Originalcodes in die Codecave.\n");
            return false;
        }

        // 6. Post-Original-Filler schreiben.
        uintptr_t postOrigAddr = origCodeAddr + byteCount;
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(postOrigAddr),
            postOrigFill.data(), postOrigFill.size(), &bytesWritten) || bytesWritten != postOrigFill.size()) {
            printf("Fehler beim Schreiben des postOrigFill.\n");
            return false;
        }

        // 7. Pre-Jump-Filler schreiben.
        uintptr_t preJumpAddr = postOrigAddr + postOrigFill.size();
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(preJumpAddr),
            preJumpFill.data(), preJumpFill.size(), &bytesWritten) || bytesWritten != preJumpFill.size()) {
            printf("Fehler beim Schreiben des preJumpFill.\n");
            return false;
        }

        // 8. 14-Byte Jump Back schreiben (der zurück zu targetAddress + byteCount springt)
        uintptr_t jumpAddr = preJumpAddr + preJumpFill.size();
        uintptr_t jumpBackDest = this->targetAddress + byteCount;
        WriteJump64(jumpAddr, jumpBackDest);

        // 9. Post-Jump-Filler schreiben.
        uintptr_t postJumpAddr = jumpAddr + 14;
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(postJumpAddr),
            postJumpFill.data(), postJumpFill.size(), &bytesWritten) || bytesWritten != postJumpFill.size()) {
            printf("Fehler beim Schreiben des postJumpFill.\n");
            return false;
        }

        // 10. Überschreibe den Originalcode an pTarget: Setze einen Jump zur Codecave.
        WriteJump64(this->targetAddress, this->codecaveAddress);

        // 11. Überschüssige Bytes (falls vorhanden) mit NOPs füllen.
        if (byteCount > 14) {
            size_t nopCount = byteCount - 14;
            std::vector<uint8_t> nops(nopCount, 0x90);
            WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(this->targetAddress + 14),
                nops.data(), nops.size(), &bytesWritten);
        }

        // 12. Ursprünglichen Speicherschutz wiederherstellen.
        DWORD temp;
        VirtualProtectEx(hProcess, pTarget, byteCount, oldProtect, &temp);

        return true;
    }
};

void OverlayThread() {
    RenderOverlay(Hax); // Dein Overlay-Loop
}


std::vector<uint8_t> SavedBytes;
uintptr_t targetfunction = 0x7FF8355C80CF;


void CheatsThread(HANDLE hProcess, BytePatcher HaxPatcher) {
    
	MemoryManager memMan(hProcess);
    SavedBytes = HaxPatcher.ReadBytes(targetfunction, 17);
    std::vector<int> registersToSave = { 0 };
    std::vector<uintptr_t> ptrListe = HaxPatcher.DetourHookWithRegisters((void*)targetfunction, 17, registersToSave);
	
	size_t bytesRead = 0;
	;
	

    
    while (true) {

        uintptr_t ptr = 0;
        float mMatrix[16];
        uintptr_t cachePtr = ptrListe[0];
        ReadProcessMemory(hProcess, (uintptr_t*)(cachePtr), &ptr, sizeof(uintptr_t), &bytesRead);
        ReadProcessMemory(hProcess, (uintptr_t*)(ptr + 0xC0), &mMatrix, sizeof(mMatrix), &bytesRead);
        Sleep(1000);
    }
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Hax.hProcess = Hax.GetAndLoadHax(L"Kingdom Come: Deliverance II");
    debugConsole.setDebugging(true);

    BytePatcher HaxPatcher(Hax.hProcess);

    std::thread cheatsThread(CheatsThread, Hax.hProcess, HaxPatcher);
    std::thread overlayThread(OverlayThread);

    // Main wartet nur auf Overlay
    overlayThread.join();


    HaxPatcher.PatchBytes(targetfunction, SavedBytes);

    return 0;
}
