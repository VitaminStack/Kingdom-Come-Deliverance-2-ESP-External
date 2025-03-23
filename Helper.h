#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <d3d11.h>
#include <directxmath.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>
#include <gdiplus.h>
#include <algorithm>
#include <mutex>
#include <emmintrin.h> // Für _mm_pause()
#include "imgui/imgui.h"
#include <tchar.h>
#include "imgui/imgui_internal.h"
#include <map>

#pragma comment(lib, "gdiplus.lib")

struct Vector2
{
	float x, y;
};

class Vector3 {
public:
	float x, y, z;

	// Standardkonstruktor mit Initialisierungsliste
	Vector3() : x(0), y(0), z(0) {}
	Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
	Vector3(const float* v) : x(v[0]), y(v[1]), z(v[2]) {}
	Vector3(const Vector3& v) = default; // Standard-Kopierkonstruktor

	// Zuweisungsoperatoren
	Vector3& operator=(const Vector3& v) = default;

	// Vergleichsoperatoren
	bool operator==(const Vector3& v) const {
		return (x == v.x && y == v.y && z == v.z);
	}
	bool operator!=(const Vector3& v) const {
		return !(*this == v);
	}

	// Arithmetische Operatoren
	Vector3 operator+(const Vector3& v) const {
		return Vector3(x + v.x, y + v.y, z + v.z);
	}
	Vector3 operator-(const Vector3& v) const {
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
	Vector3 operator*(float scalar) const {
		return Vector3(x * scalar, y * scalar, z * scalar);
	}
	Vector3 operator/(float scalar) const {
		return (scalar != 0) ? Vector3(x / scalar, y / scalar, z / scalar) : Vector3(0, 0, 0);
	}

	Vector3& operator+=(const Vector3& v) {
		x += v.x; y += v.y; z += v.z;
		return *this;
	}
	Vector3& operator-=(const Vector3& v) {
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}
	Vector3& operator*=(float scalar) {
		x *= scalar; y *= scalar; z *= scalar;
		return *this;
	}
	Vector3& operator/=(float scalar) {
		if (scalar != 0) {
			x /= scalar; y /= scalar; z /= scalar;
		}
		return *this;
	}

	// Vektoroperationen
	float length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	float lengthSquared() const {
		return x * x + y * y + z * z;
	}

	Vector3 normalize() const {
		float len = length();
		return (len != 0) ? *this / len : Vector3(0, 0, 0);
	}

	float dot(const Vector3& v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	Vector3 cross(const Vector3& v) const {
		return Vector3(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x
		);
	}

	bool isZero() const {
		return (x == 0 && y == 0 && z == 0);
	}

	void set(float X, float Y, float Z) {
		x = X; y = Y; z = Z;
	}
	float DistTo(const Vector3& v) const
	{
		return (*this - v).length();
	}
	// Ausgabe für Debugging
	friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
		os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return os;
	}
};
struct Vector4
{
	float x, y, z, w;
};


class FPSLimiter {
private:
	LARGE_INTEGER frequency;
	LARGE_INTEGER frameStart, frameEnd;
	double targetFrameTime;
	float targetFPS;

	// Berechnet die verstrichene Zeit in Millisekunden
	double getElapsedTime(LARGE_INTEGER start, LARGE_INTEGER end) {
		return (double)(end.QuadPart - start.QuadPart) * 1000.0 / (double)frequency.QuadPart;
	}

public:
	float fpsValue = 60.0f;

	// Konstruktor: Initialisiert den High-Precision Timer
	FPSLimiter(float fps) {
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&frameStart);
		setTargetFPS(fps);
	}

	// FPS-Limit setzen
	void setTargetFPS(float fps) {
		targetFPS = (fps <= 0.0f) ? 170.0f : fps;  // 170 = unbegrenzt
		targetFrameTime = 1000.0 / targetFPS;      // Ziel-Framezeit in ms
	}

	// Getter für FPS-Wert, um ImGui-Slider zu unterstützen
	float getTargetFPS() const {
		return targetFPS;
	}

	// FPS-Begrenzung anwenden
	void cap(const ImGuiIO& io) {
		if (targetFPS == 170.0f) return;  // FPS-Limiter deaktiviert

		QueryPerformanceCounter(&frameEnd);
		double elapsedTime = getElapsedTime(frameStart, frameEnd);
		double frameTime = 1000.0 / io.Framerate;
		double sleepTime = targetFrameTime - frameTime;

		if (sleepTime > 0) {
			if (sleepTime > 1.0) {
				Sleep(static_cast<DWORD>(sleepTime));
			}
			else {
				_mm_pause();  // Präzises Timing für kurze Wartezeiten
			}
		}

		QueryPerformanceCounter(&frameStart);  // Timer für den nächsten Frame neu starten
	}
};

class RenderHelper {
public:

	bool ESP_Status = true;
	bool useCutsceneCheck = true;
	float maxDistance = 5000.0f;
	bool Clickability = false;
	bool openMenu = false;
	Vector2 WindowScreen;

	float Matrix[16];
	Vector3 CamPos;

	uintptr_t MatrixAdr = 0x0;
	uintptr_t CamPosAdr = 0x5209DA8;
	uintptr_t CutsceneActive = 0x528C8D8;

	// Berechnet die 3D-Distanz zwischen zwei Punkten
	static float Distance3D(const Vector3& point1, const Vector3& point2) {
		return point1.DistTo(point2);
	}
	// Wandelt Weltkoordinaten in Bildschirmkoordinaten um (Far Cry)
	static bool W2SCryEngine(Vector3 pos, Vector2& screen, float matrix[16], int windowWidth, int windowHeight) {
		DirectX::XMFLOAT4 clipCoords;
		clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
		clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
		clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
		clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

		if (clipCoords.w < 0.1f) return false;

		Vector3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		screen.x = (windowWidth / 2) * (1 + NDC.x);
		screen.y = (windowHeight / 2) * (1 - NDC.y);  // Y-Achse in DirectX ist invertiert
		return true;
	}
	// Berechnet die mittlere Position für die Textdarstellung
	static float CalcMiddlePos(float vScreenX, const char* Text) {
		return vScreenX - ((strlen(Text) / 2) * 5);
	}
	// Debug-Funktion zum Zeichnen von Bone-Strukturen (Skelett-Rendering)
	static void DebugBones(HANDLE hProcess, ImDrawList* drawList, float viewMatrix[16], uintptr_t boneArray, int boneCount, ImU32 color = IM_COL32(255, 0, 0, 255)) {
		if (!boneArray || boneCount <= 0) return;

		for (int i = 0; i < boneCount; ++i) {
			Vector3 bonePos = { 0, 0, 0 };

			if (!ReadProcessMemory(hProcess, (LPVOID)(boneArray + (i * 0xE8)), &bonePos, sizeof(Vector3), nullptr)) {
				continue;
			}

			Vector2 screenPos;
			if (RenderHelper::W2SCryEngine(bonePos, screenPos, viewMatrix, 2560, 1440)) {
				if (screenPos.x > 0 && screenPos.x < 2560 && screenPos.y > 0 && screenPos.y < 1440) {
					drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 3.0f, color);
				}
			}
		}
	}
	void ChangeClickability(HWND ownd)
	{
		// Toggle-Funktion durch VK_INSERT
		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			Clickability = !Clickability; // Zustand umschalten
			openMenu = !openMenu;
		}

		// Fensterstil anpassen
		long style = GetWindowLong(ownd, GWL_EXSTYLE);
		if (Clickability) {
			style &= ~WS_EX_LAYERED;
			SetWindowLong(ownd, GWL_EXSTYLE, style);
			SetForegroundWindow(ownd); // Nur wenn Clickability aktiv ist
		}
		else {
			style |= WS_EX_LAYERED;
			SetWindowLong(ownd, GWL_EXSTYLE, style);
		}
	}
	void SetOverlayToTarget(HWND WindowHandle, HWND OverlayHandle, Vector2& ScreenXY)
	{
		RECT clientRect;
		POINT pt = { 0, 0 };

		// Client-Area Größe holen
		GetClientRect(WindowHandle, &clientRect);
		ClientToScreen(WindowHandle, &pt); // Obere linke Ecke in Screen-Koordinaten umrechnen

		int Breite = (clientRect.right - clientRect.left);
		int Höhe = (clientRect.bottom - clientRect.top);

		ScreenXY.x = static_cast<float>(Breite);
		ScreenXY.y = static_cast<float>(Höhe);

		// Overlay exakt auf Client-Area setzen
		MoveWindow(OverlayHandle, pt.x, pt.y, Breite, Höhe, true);
	}

	// Zeichnet ein rotierendes und bewegendes Dreieck in der oberen rechten Ecke des Fensters
	static void RenderRotatingTriangle(ImDrawList* drawList, ImVec2 screenSize) {
		static float timeElapsed = 0.0f;
		static float posXOffset = 0.0f;
		static float angle = 0.0f;

		// Zeit aktualisieren (sorgt für Bewegung & Animation)
		timeElapsed += 0.02f;

		// Leichte Bewegung von links nach rechts
		posXOffset = sin(timeElapsed) * 50.0f;

		// Rotation erhöhen
		angle += 0.03f;

		// Farben dynamisch ändern (HSL-Farbkreis -> RGB)
		ImU32 color = IM_COL32(
			static_cast<int>(sin(timeElapsed * 2.0f) * 127 + 128),  // R
			static_cast<int>(sin(timeElapsed * 2.0f + 2.0f) * 127 + 128),  // G
			static_cast<int>(sin(timeElapsed * 2.0f + 4.0f) * 127 + 128),  // B
			255  // Alpha bleibt 255
		);

		// Position des Dreiecks (rechts oben)
		ImVec2 center(screenSize.x - 200 + posXOffset, 100);

		// Berechnung der 3 Punkte des Dreiecks
		float radius = 60.0f; // Größe des Dreiecks
		ImVec2 points[3];

		for (int i = 0; i < 3; i++) {
			float theta = angle + i * (2.0f * IM_PI / 3.0f); // 120° pro Punkt
			points[i] = ImVec2(
				center.x + cos(theta) * radius,
				center.y + sin(theta) * radius
			);
		}

		// Zeichnen des Dreiecks mit der dynamischen Farbe
		drawList->AddTriangleFilled(points[0], points[1], points[2], color);
	}
private:

};

extern class InitHax
{
public:
	DWORD ProcID;
	HANDLE hProcess;
	HWND TargetHWND;
	LPCWSTR TargetGame;
	const wchar_t* ModuleBaseName;

	template <typename T>
	T Read(uintptr_t address) {
		T buffer;
		ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), nullptr);
		return buffer;
	}

	HWND FindWindowByProcessId(DWORD processId)
	{
		HWND hwnd = GetTopWindow(NULL);
		while (hwnd != NULL)
		{
			DWORD windowProcessId;
			GetWindowThreadProcessId(hwnd, &windowProcessId);

			if (windowProcessId == processId)
			{
				return hwnd; // Fenster gefunden
			}

			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}

		return NULL; // Kein passendes Fenster gefunden
	}

	DWORD FindProcessId(const _TCHAR* processName)
	{
		PROCESSENTRY32 processInfo;
		processInfo.dwSize = sizeof(processInfo);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == INVALID_HANDLE_VALUE)
			return 0;

		Process32First(snapshot, &processInfo);
		if (!_tcscmp(processInfo.szExeFile, processName))
		{
			CloseHandle(snapshot);
			return processInfo.th32ProcessID;
		}

		while (Process32Next(snapshot, &processInfo)) {
			if (_tcscmp(processInfo.szExeFile, processName) == 0) {
				CloseHandle(snapshot);
				return processInfo.th32ProcessID;
			}
		}


		CloseHandle(snapshot);
		return 0;
	}

	HANDLE GetAndLoadHax(LPCWSTR TargetGame)
	{
		TargetHWND = FindWindow(0, TargetGame);
		GetWindowThreadProcessId(TargetHWND, &ProcID);

		//ProcID = FindProcessId(TargetGame);        
		//TargetHWND = FindWindowByProcessId(ProcID);

		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcID);
		if (hProcess == NULL)
		{
			DWORD error = GetLastError();
			std::cerr << "OpenProcess failed. Error code: " << error << std::endl;
		}
		return hProcess;
	}
};
class ExBytePatcher {
private:
	HANDLE hProcess;
	uintptr_t address;        // Originaladresse, an der gepatcht wird
	std::vector<BYTE> originalBytes;  // Gesicherte Originalbytes
	size_t patchSize;         // Anzahl der überschriebenen Bytes
	bool isPatched = false;   // Ob der Patch aktiv ist

	// Code-Cave-Verwaltung
	uintptr_t codeCaveAddress = 0; // Hier wird die allozierte Code-Cave gespeichert
	size_t    codeCaveSize = 0; // Größe der alloz. Code-Cave

	// Statische Liste aller aktiven Patcher
	static inline std::vector<ExBytePatcher*> activePatchers;

public:
	ExBytePatcher(HANDLE process, uintptr_t addr, size_t size)
		: hProcess(process), address(addr), patchSize(size)
	{
		// Damit RestoreAll() am Ende weiß, welche Patcher zurückgepatcht werden müssen
		activePatchers.push_back(this);
	}

	// Destruktor: Sicher stellen, dass gepatchte Bytes zurückgesetzt werden
	// und dieses Objekt aus der aktiven Liste entfernen.
	~ExBytePatcher() {
		Restore();
		auto it = std::find(activePatchers.begin(), activePatchers.end(), this);
		if (it != activePatchers.end()) {
			activePatchers.erase(it);
		}
	}
	void TogglePatch(bool enablePatch) {
		if (enablePatch && !isPatched) {
			if (Patch()) {
				isPatched = true;
			}
		}
		else if (!enablePatch && isPatched) {
			if (Restore()) {
				isPatched = false;
			}
		}
	}
	// Statistische Methode zum Rückpatchen aller aktiven Patcher
	static void RestoreAll() {
		for (auto* patcher : activePatchers) {
			patcher->Restore();
		}
	}

	// Patch nur mit NOPs (Beispiel aus deinem Code) – hier nicht primär genutzt
	bool Patch() {
		if (originalBytes.empty()) {
			originalBytes.resize(patchSize);
			ReadProcessMemory(hProcess, (LPCVOID)address, originalBytes.data(), patchSize, NULL);
		}
		std::vector<BYTE> nopBytes(patchSize, 0x90);
		return WriteProcessMemory(hProcess, (LPVOID)address, nopBytes.data(), patchSize, NULL);
	}

	// Schreibt die ursprünglichen Bytes zurück und gibt den Code-Cave-Speicher frei
	bool Restore() {
		if (!isPatched) return false;
		if (originalBytes.empty()) return false;

		// Original-Bytes zurückschreiben
		if (!WriteProcessMemory(hProcess, (LPVOID)address, originalBytes.data(), patchSize, NULL)) {
			std::cerr << "[!] Fehler beim Zurückschreiben der Original-Bytes.\n";
			return false;
		}

		// Code-Cave-Speicher freigeben
		if (codeCaveAddress != 0) {
			VirtualFreeEx(hProcess, reinterpret_cast<LPVOID>(codeCaveAddress), 0, MEM_RELEASE);
			codeCaveAddress = 0;
			codeCaveSize = 0;
		}

		isPatched = false;
		return true;
	}

	// Hilfsfunktionen zum Schreiben von Jumps
	void WriteJump32(HANDLE hProcess, uintptr_t sourceAddress, uintptr_t destinationAddress) {
		DWORD offset = (DWORD)(destinationAddress - (sourceAddress + 5)); // Relativer Offset

		BYTE jmpInstruction[5] = { 0xE9 }; // JMP opcode
		memcpy(&jmpInstruction[1], &offset, sizeof(DWORD));

		WriteProcessMemory(hProcess, (LPVOID)sourceAddress, jmpInstruction, sizeof(jmpInstruction), NULL);
	}
	void WriteJump64(HANDLE hProcess, uintptr_t sourceAddress, uintptr_t destinationAddress) {
		// Absolute Jump über RIP: FF 25 00 00 00 00 + 8-Byte-Ziel
		BYTE jmpInstruction[14] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // FF 25 [RIP+0]
			0, 0, 0, 0, 0, 0, 0, 0              // Platz für Zieladresse (8 Bytes)
		};

		// Zieladresse in den letzten 8 Bytes
		memcpy(&jmpInstruction[6], &destinationAddress, sizeof(uintptr_t));

		// Jump an sourceAddress schreiben
		WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(sourceAddress),
			jmpInstruction, sizeof(jmpInstruction), nullptr);
	}

	// Automatischer Detour, der eine Code-Cave im Zielprozess alloziert und den Originalcode dorthin umlenkt
	bool AutoDetourFunction64(size_t stolenBytes) {
		// Wir brauchen für den absoluten Jump 14 Bytes (besser: Disassemblieren!)
		if (stolenBytes < 14) {
			std::cerr << "[!] Fehler: stolenBytes muss >= 14 sein.\n";
			return false;
		}

		// 1) Code-Cave im Zielprozess allozieren
		codeCaveSize = stolenBytes + 14; // Platz für geklaute Bytes + Jump zurück
		LPVOID cavePtr = VirtualAllocEx(hProcess, NULL, codeCaveSize,
			MEM_COMMIT | MEM_RESERVE,
			PAGE_EXECUTE_READWRITE);
		if (!cavePtr) {
			std::cerr << "[!] Fehler: Konnte Code-Cave nicht allozieren.\n";
			return false;
		}
		codeCaveAddress = reinterpret_cast<uintptr_t>(cavePtr);

		// 2) Original-Bytes lesen (falls noch nicht geschehen)
		if (originalBytes.empty()) {
			originalBytes.resize(stolenBytes);
			SIZE_T bytesRead = 0;
			if (!ReadProcessMemory(hProcess, (LPCVOID)address,
				originalBytes.data(), stolenBytes, &bytesRead)
				|| bytesRead != stolenBytes)
			{
				std::cerr << "[!] Fehler beim Lesen der Original-Bytes.\n";
				VirtualFreeEx(hProcess, cavePtr, 0, MEM_RELEASE);
				codeCaveAddress = 0;
				codeCaveSize = 0;
				return false;
			}
			patchSize = stolenBytes; // Wir merken uns, wie viele Bytes überschrieben werden
		}

		// 3) Geklaute Bytes in die Code-Cave schreiben
		SIZE_T bytesWritten = 0;
		if (!WriteProcessMemory(hProcess, cavePtr, originalBytes.data(), stolenBytes, &bytesWritten)
			|| bytesWritten != stolenBytes)
		{
			std::cerr << "[!] Fehler beim Schreiben in die Code-Cave.\n";
			VirtualFreeEx(hProcess, cavePtr, 0, MEM_RELEASE);
			codeCaveAddress = 0;
			codeCaveSize = 0;
			return false;
		}

		// 4) Jump vom Ende der Code-Cave zurück in den Originalcode (nach den gestohlenen Bytes)
		uintptr_t codeCaveJmpBack = codeCaveAddress + stolenBytes;
		uintptr_t originalReturn = address + stolenBytes;
		WriteJump64(hProcess, codeCaveJmpBack, originalReturn);

		// 5) Jump vom Originalcode in die Code-Cave
		WriteJump64(hProcess, address, codeCaveAddress);

		isPatched = true;
		return true;
	}

	bool IsPatched() const { return isPatched; }
};
class MemoryManager {
private:
	HANDLE hProcess;

public:
	MemoryManager(HANDLE process) : hProcess(process) {}
	uintptr_t ModuleBaseAdresse;

	// ✅ Statische Methode: FindDMAAddy kann direkt ohne Instanz genutzt werden
	static uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, const std::vector<unsigned int>& offsets) {
		uintptr_t addr = ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i) {
			ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
			addr += offsets[i];
		}
		return addr;
	}

	// ✅ Speicher auslesen
	void ReadMemory(const void* address, void* buffer, size_t size) {
		DWORD oldProtect;
		if (VirtualProtect((LPVOID)address, size, PAGE_READWRITE, &oldProtect)) {
			memcpy(buffer, address, size);
			VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
		}
	}
	static std::string ReadStringFromMemory(HANDLE hProcess, uintptr_t address, size_t bufferSize = 256) {
		std::vector<char> buffer(bufferSize, '\0');
		SIZE_T bytesRead;
		if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer.data(), bufferSize - 1, &bytesRead)) {
			return std::string(buffer.data());
		}
		return "";
	}
	// ✅ Modulbasisadresse & Größe holen
	static uintptr_t GetModuleBaseAddressEx(const wchar_t* moduleName, DWORD pID, DWORD& moduleSize) {
		uintptr_t baseAddress = 0;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
		MODULEENTRY32 moduleEntry = { sizeof(MODULEENTRY32) };

		if (Module32First(hSnapshot, &moduleEntry)) {
			do {
				if (wcscmp(moduleEntry.szModule, moduleName) == 0) {
					baseAddress = (uintptr_t)moduleEntry.modBaseAddr;
					moduleSize = moduleEntry.modBaseSize;
					break;
				}
			} while (Module32Next(hSnapshot, &moduleEntry));
		}
		CloseHandle(hSnapshot);
		return baseAddress;
	}

	// ✅ Dynamische Adresse auslesen mit Offsets
	uintptr_t GetDynamicAddressEx(uintptr_t baseAddress, std::vector<DWORD> offsets) {
		uintptr_t dynamicAddress = baseAddress;
		for (size_t i = 0; i < offsets.size() - 1; i++) {
			ReadProcessMemory(hProcess, (LPCVOID)(dynamicAddress + offsets[i]), &dynamicAddress, sizeof(offsets[i]), NULL);
		}
		dynamicAddress += offsets[offsets.size() - 1];
		return dynamicAddress;
	}

	// ✅ Pattern-Scanning (AOB Scan)
	uintptr_t GetAddressFromSignatureEx(std::vector<int> signature, uintptr_t baseAddress, DWORD size) {
		if (!baseAddress || !hProcess) return NULL;

		std::vector<byte> buffer(size);
		if (!ReadProcessMemory(hProcess, (LPCVOID)(baseAddress), buffer.data(), size, NULL)) {
			return NULL;
		}

		for (size_t i = 0; i < size; i++) {
			bool found = true;
			for (size_t j = 0; j < signature.size(); j++) {
				if (signature[j] != -1 && signature[j] != buffer[i + j]) {
					found = false;
					break;
				}
			}
			if (found) return baseAddress + i;
		}
		return NULL;
	}

	// ✅ Signature Scan (AOB Scan über Modul)
	uintptr_t ScanAOB(std::vector<int> signature, const wchar_t* moduleName, DWORD procID) {
		DWORD moduleSize;
		uintptr_t moduleBase = GetModuleBaseAddressEx(moduleName, procID, moduleSize);
		return GetAddressFromSignatureEx(signature, moduleBase, moduleSize);
	}

	
	
};
class FlyHack {
private:
	HANDLE hProcess;
	uintptr_t velocityAddress;
	float velocity = 0.0f;
	const float maxVelocity = 50.0f;  // Maximum speed for Z-Velocity
	const float acceleration = 10.0f; // Acceleration factor
	const float lerpSpeed = 0.1f;     // Lerp speed (higher = faster transitions)

	// Lerp function for smooth velocity transition
	float Lerp(float a, float b, float t) {
		return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
	}

public:

	bool flyhackstatus = false;

	FlyHack(HANDLE process, uintptr_t baseAddress)
		: hProcess(process) {
		// Find the velocity memory address using pointer path
		velocityAddress = MemoryManager::FindDMAAddy(process, baseAddress + 0x53F8320, { 0x78,0x28,0x7c0,0x164 });
	}

	void Update() {
		float targetVelocity = 0.0f;  // Default velocity (no input = no movement)

		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			targetVelocity = maxVelocity; // Move upwards
		}
		else if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			targetVelocity = -maxVelocity; // Move downwards
		}
		// Smoothly adjust velocity using Lerp
		velocity = Lerp(velocity, targetVelocity, lerpSpeed);

		// Write updated velocity to memory
		WriteProcessMemory(hProcess, (LPVOID)velocityAddress, &velocity, sizeof(float), nullptr);
	}
};

class EntityManager {
private:
	struct Entity {
		uintptr_t baseAddress;
		uintptr_t posAddress;
		uintptr_t nameAddress;
		Vector3 position;
		std::string name;
		float distance;
	};

	HANDLE hProcess;
	uintptr_t entityListAddress;
	std::unordered_map<uintptr_t, Entity> entityCache;
	std::vector<uintptr_t> entityAddresses;
	std::atomic<bool> stopThread{ false };
	std::thread addressThread;
	std::thread dataThread;
	std::mutex entityMutex;
	Vector3 cameraPosition;
	float maxEntityDistance = 5000.0f;

	const ImU32 FRIENDLY_COLOR = IM_COL32(0, 191, 255, 255);  // Blue
	const ImU32 ENEMY_COLOR = IM_COL32(255, 0, 0, 255);      // Red
	const ImU32 NEUTRAL_COLOR = IM_COL32(0, 255, 0, 255);    // Green

	std::vector<std::pair<std::string, ImU32>> entityFilters = {
		// Friendly entities (blue)
		{"dog", FRIENDLY_COLOR},
		{"horse", NEUTRAL_COLOR},
		{"woman", FRIENDLY_COLOR},
		{"man", FRIENDLY_COLOR},

		// Enemy entities (red)
		{"wilddog", ENEMY_COLOR},
		{"wolf", ENEMY_COLOR},
		{"boar", ENEMY_COLOR},
		{"enemy", ENEMY_COLOR},

		// Neutral entities (green)
		{"deer", NEUTRAL_COLOR},
		{"hare", NEUTRAL_COLOR},
		{"cow", NEUTRAL_COLOR},
		{"bull", NEUTRAL_COLOR},
		{"pig", NEUTRAL_COLOR},
		{"sheep", NEUTRAL_COLOR}
	};

public:
	int validEntities;

	EntityManager(HANDLE process, uintptr_t baseAddress)
		: hProcess(process), entityListAddress(baseAddress) {
		StartThreads();
	}

	~EntityManager() {
		stopThread = true;
		if (addressThread.joinable()) {
			addressThread.join();
		}
		if (dataThread.joinable()) {
			dataThread.join();
		}
	}

	void StartThreads() {
		addressThread = std::thread([this]() {
			while (!stopThread) {
				UpdateEntityAddresses();
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
			});

		dataThread = std::thread([this]() {
			while (!stopThread) {
				UpdateEntities();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			});
	}

	void SetCameraPosition(const Vector3& camPos, float maxDistance) {
		std::lock_guard<std::mutex> lock(entityMutex);
		cameraPosition = camPos;
		maxEntityDistance = maxDistance;
	}

	void UpdateEntityAddresses() {
		const int entityCount = 3000;
		uintptr_t entListAdr = MemoryManager::FindDMAAddy(hProcess, entityListAddress + 0x052A39D0, { 0x0 });

		std::vector<uintptr_t> updatedAddresses;
		updatedAddresses.reserve(entityCount);

		for (int i = 0; i < entityCount; ++i) {
			uintptr_t pointerAddress = entListAdr + (i * 0x8);
			uintptr_t entBase = 0;

			if (ReadProcessMemory(hProcess, (LPVOID)pointerAddress, &entBase, sizeof(entBase), nullptr)) {
				updatedAddresses.push_back(entBase);
			}
		}

		std::lock_guard<std::mutex> lock(entityMutex);
		entityAddresses = std::move(updatedAddresses);
	}

	void UpdateEntities() {
		validEntities = 0;
		std::unordered_map<uintptr_t, Entity> updatedCache;

		{
			std::lock_guard<std::mutex> lock(entityMutex);
			for (uintptr_t entBase : entityAddresses) {
				uintptr_t posAddr = MemoryManager::FindDMAAddy(hProcess, entBase + 0x18, { 0x30 });
				uintptr_t nameAddr = MemoryManager::FindDMAAddy(hProcess, entBase + 0x18, { 0xE8, 0x0 });

				Vector3 pos;
				ReadProcessMemory(hProcess, (LPVOID)posAddr, &pos, sizeof(pos), nullptr);
				float distance = RenderHelper::Distance3D(cameraPosition, pos);

				if (distance < maxEntityDistance) {
					Entity entity;
					entity.baseAddress = entBase;
					entity.posAddress = posAddr;
					entity.nameAddress = nameAddr;
					entity.position = pos;
					entity.distance = distance;
					updatedCache[entBase] = entity;
					validEntities++;
				}
			}
		}

		for (auto& [entBase, entity] : updatedCache) {
			entity.name = FilterEntityName(ReadEntityName(entity.nameAddress));
		}

		std::lock_guard<std::mutex> lock(entityMutex);
		entityCache = std::move(updatedCache);
	}

	void RenderEntities(ImDrawList* drawList, float screenWidth, float screenHeight, float* matrix) {
		std::lock_guard<std::mutex> lock(entityMutex);
		for (const auto& [entBase, entity] : entityCache) {
			Vector2 screenPos;
			if (RenderHelper::W2SCryEngine(entity.position, screenPos, matrix, screenWidth, screenHeight)) {
				if (screenPos.x > 0 && screenPos.x < screenWidth && screenPos.y > 0 && screenPos.y < screenHeight) {
					ImU32 color = GetEntityColor(entity.name);
					std::ostringstream oss;
					oss << entity.name << " " << std::fixed << std::setprecision(2) << entity.distance << "m";
					RenderOutlinedText(drawList, ImVec2(RenderHelper::CalcMiddlePos(screenPos.x, oss.str().c_str()), screenPos.y), color, IM_COL32(0, 0, 0, 255), oss.str().c_str());
				}
			}
		}
	}

private:

	void RenderOutlinedText(ImDrawList* draw_list, ImVec2 pos, ImU32 textColor, ImU32 outlineColor, const char* text) {
		const float offset = 1.5f; // Abstand für den Rand

		// Rand (in 4 Richtungen zeichnen)
		draw_list->AddText(ImVec2(pos.x - offset, pos.y), outlineColor, text);
		draw_list->AddText(ImVec2(pos.x + offset, pos.y), outlineColor, text);
		draw_list->AddText(ImVec2(pos.x, pos.y - offset), outlineColor, text);
		draw_list->AddText(ImVec2(pos.x, pos.y + offset), outlineColor, text);

		// Originaler Text
		draw_list->AddText(pos, textColor, text);
	}

	// Verwendung in deinem ImGui Rendering-Code:
	void RenderUI() {
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 pos = ImVec2(100, 100); // Position auf dem Bildschirm
		RenderOutlinedText(draw_list, pos, IM_COL32(255, 255, 0, 255), IM_COL32(255, 255, 255, 255), "tneb_zizka 6.84m");
	}


	std::string ReadEntityName(uintptr_t nameAddr) {
		return MemoryManager::ReadStringFromMemory(hProcess, nameAddr, 30);
	}

	ImU32 GetEntityColor(const std::string& name) {
		for (const auto& [keyword, color] : entityFilters) {
			if (name.find(keyword) != std::string::npos) {
				return color;
			}
		}
		return IM_COL32(255, 255, 0, 255);
	}

	std::string FilterEntityName(const std::string& rawName) {
		for (const auto& [keyword, _] : entityFilters) {
			std::string capitalizedKeyword = keyword;
			capitalizedKeyword[0] = std::toupper(capitalizedKeyword[0]);
			if (rawName.find(keyword) != std::string::npos || rawName.find(capitalizedKeyword) != std::string::npos) {
				return keyword;
			}
		}
		return rawName;
	}
};

class DebugScreenshot {
public:
	static void SaveOverlayScreenshot(HWND overlayWindow, const std::wstring& filename) {
		HDC hdcScreen, hdcMem;
		HBITMAP hBitmap;
		RECT overlayRect;

		// Fenstergröße des Overlays abrufen
		GetWindowRect(overlayWindow, &overlayRect);
		int width = overlayRect.right - overlayRect.left;
		int height = overlayRect.bottom - overlayRect.top;

		// Bildschirm-Kontext holen
		hdcScreen = GetDC(NULL);
		hdcMem = CreateCompatibleDC(hdcScreen);
		hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
		SelectObject(hdcMem, hBitmap);

		// Screenshot in den Speicher zeichnen
		BitBlt(hdcMem, 0, 0, width, height, hdcScreen, overlayRect.left, overlayRect.top, SRCCOPY);

		// Screenshot als PNG speichern
		SaveBitmapAsPNG(hBitmap, filename);

		// Speicher freigeben
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdcScreen);
	}

private:
	static void SaveBitmapAsPNG(HBITMAP hBitmap, const std::wstring& filename) {
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		Gdiplus::Bitmap bitmap(hBitmap, NULL);
		CLSID pngClsid;
		GetEncoderClsid(L"image/png", &pngClsid);

		std::wstring fullPath = filename + L".png";
		bitmap.Save(fullPath.c_str(), &pngClsid, NULL);

	}

	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
		UINT num = 0, size = 0;
		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0) return -1;

		Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL) return -1;

		Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
		for (UINT i = 0; i < num; ++i) {
			if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
				*pClsid = pImageCodecInfo[i].Clsid;
				free(pImageCodecInfo);
				return i;
			}
		}
		free(pImageCodecInfo);
		return -1;
	}
};
class DebugConsole {
public:
	inline static std::mutex logMutex;
	inline static bool debuggingEnabled = false;

	enum class LogLevel {
		Log_ERROR,
		Log_WARNING,
		Log_INFO
	};

	static void setDebugging(bool enable) {

		debuggingEnabled = enable;
		if (enable) {
			openConsole();
		}
		else {
			closeConsole();
		}
	}

	static void log(LogLevel level, const std::string& message) {
		if (!debuggingEnabled) return;

		std::lock_guard<std::mutex> lock(logMutex);
		std::string prefix = getLogLevelString(level);
		std::cout << getCurrentTime() << " " << prefix << ": " << message << std::endl;
	}
	static std::string formatHex(uintptr_t value) {
		std::ostringstream oss;
		oss << "0x"
			<< std::uppercase << std::hex
			<< std::setw(sizeof(uintptr_t) == 8 ? 16 : 8)
			<< std::setfill('0')
			<< value;
		return oss.str();
	}

	static void clearConsole() {
		if (!debuggingEnabled) return;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		DWORD consoleSize;
		DWORD charsWritten;

		// Holen der aktuellen Größe
		if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;

		consoleSize = csbi.dwSize.X * csbi.dwSize.Y;

		// Console mit Leerzeichen füllen
		FillConsoleOutputCharacter(hConsole, ' ', consoleSize, { 0, 0 }, &charsWritten);
		// Attribute zurücksetzen
		FillConsoleOutputAttribute(hConsole, csbi.wAttributes, consoleSize, { 0, 0 }, &charsWritten);
		// Cursor nach oben setzen
		SetConsoleCursorPosition(hConsole, { 0, 0 });
	}
private:
	
	

	static void openConsole() {
		if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
			FILE* fp;
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);
			freopen_s(&fp, "CONIN$", "r", stdin);
			std::cout << "[DEBUG CONSOLE OPENED]" << std::endl;
		}
		else {
			MessageBoxA(NULL, "Failed to open console!", "Error", MB_OK | MB_ICONERROR);
		}
	}

	static void closeConsole() {
		std::cout << "[DEBUG CONSOLE CLOSED]" << std::endl;
		FreeConsole();
	}

	static std::string getLogLevelString(LogLevel level) {
		switch (level) {
		case LogLevel::Log_ERROR: return "[ERROR]";
		case LogLevel::Log_WARNING: return "[WARNING]";
		case LogLevel::Log_INFO: return "[INFO]";
		}
		return "[UNKNOWN]";
	}

	static std::string getCurrentTime() {
		std::time_t now = std::time(nullptr);
		struct tm timeInfo;
		char buf[20];
		localtime_s(&timeInfo, &now);
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeInfo);
		return std::string(buf);
	}
};





