#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <d3dx9.h>
#include <d3d11.h>
#include <directxmath.h>
#include <psapi.h>
#include <iomanip>
#include <TlHelp32.h>
#include <tchar.h>  
#include <iostream>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>
#include <gdiplus.h>


#pragma comment(lib, "gdiplus.lib")
struct Vector2
{
	float x, y;
};
class Vector3
{
public:

	Vector3(void)
	{
		x = y = z = 0.0f;
	}

	Vector3(float X, float Y, float Z)
	{
		x = X; y = Y; z = Z;
	}

	Vector3(float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vector3(const float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vector3(const Vector3& v)
	{
		x = v.x; y = v.y; z = v.z;
	}

	Vector3(const Vector2& v)
	{
		x = v.x; y = v.y; z = 0.0f;
	}

	Vector3& operator=(const Vector3& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}



	Vector3& operator=(const Vector2& v)
	{
		x = v.x; y = v.y; z = 0.0f; return *this;
	}

	float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	Vector3& operator*=(const Vector3& v)
	{
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}

	Vector3& operator/=(const Vector3& v)
	{
		x /= v.x; y /= v.y; z /= v.z; return *this;
	}

	Vector3& operator==(const Vector3& v)
	{

	}

	Vector3& operator+=(float v)
	{
		x += v; y += v; z += v; return *this;
	}

	Vector3& operator-=(float v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	Vector3& operator*=(float v)
	{
		x *= v; y *= v; z *= v; return *this;
	}

	Vector3& operator/=(float v)
	{
		x /= v; y /= v; z /= v; return *this;
	}



	Vector3 operator+(const Vector3& v) const
	{
		if (this->x == 0 && this->y == 0 && this->z == 0)
			return Vector3(0.0f, 0.0f, 0.0f);

		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(const Vector3& v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(const Vector3& v) const
	{
		return Vector3(x * v.x, y * v.y, z * v.z);
	}

	Vector3 operator/(const Vector3& v) const
	{
		return Vector3(x / v.x, y / v.y, z / v.z);
	}

	Vector3 operator+(float v) const
	{
		return Vector3(x + v, y + v, z + v);
	}

	Vector3 operator-(float v) const
	{
		return Vector3(x - v, y - v, z - v);
	}

	Vector3 operator*(float v) const
	{
		return Vector3(x * v, y * v, z * v);
	}

	Vector3 operator/(float v) const
	{
		return Vector3(x / v, y / v, z / v);
	}

	void Set(float X = 0.0f, float Y = 0.0f, float Z = 0.0f)
	{
		x = X; y = Y; z = Z;
	}

	float Length(void) const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float LengthSqr(void) const
	{
		return (x * x + y * y + z * z);
	}

	float Length2d(void) const
	{
		return sqrtf(x * x + y * y);
	}

	float Length2dSqr(void) const
	{
		return (x * x + y * y);
	}

	float DistTo(const Vector3& v) const
	{
		return (*this - v).Length();
	}

	float DistToSqr(const Vector3& v) const
	{
		return (*this - v).LengthSqr();
	}

	float Dot(const Vector3& v) const
	{
		if (v.x == 0 && v.y == 0 && v.z == 0)
			return 0.0f;

		return (x * v.x + y * v.y + z * v.z);


	}


	Vector3 Cross(const Vector3& v) const
	{
		return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	bool IsZero(void) const
	{
		return (x > -0.01f && x < 0.01f &&
			y > -0.01f && y < 0.01f &&
			z > -0.01f && z < 0.01f);
	}



public:
	float x, y, z;
};

class RenderHelper {
public:
	// Berechnet die 3D-Distanz zwischen zwei Punkten
	static float Distance3D(const Vector3& point1, const Vector3& point2) {
		return point1.DistTo(point2);
	}

	// Wandelt Weltkoordinaten in Bildschirmkoordinaten um (Far Cry)
	static bool W2SCryEngine(Vector3 pos, Vector2& screen, float matrix[16], int windowWidth, int windowHeight) {
		D3DXVECTOR4 clipCoords;
		clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
		clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
		clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
		clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

		if (clipCoords.w < 0.1f) return false;

		Vector3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
		screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
		return true;
	}

	// Erstellt eine Matrix basierend auf Rotation
	static D3DXMATRIX ToMatrix(Vector3 Rotation) {
		float radPitch = (Rotation.x * float(3.1415926535897932f) / 180.f);
		float radYaw = (Rotation.y * float(3.1415926535897932f) / 180.f);
		float radRoll = (Rotation.z * float(3.1415926535897932f) / 180.f);

		float SP = sinf(radPitch);
		float CP = cosf(radPitch);
		float SY = sinf(radYaw);
		float CY = cosf(radYaw);
		float SR = sinf(radRoll);
		float CR = cosf(radRoll);

		D3DMATRIX matrix;
		matrix.m[0][0] = CP * CY;
		matrix.m[0][1] = CP * SY;
		matrix.m[0][2] = SP;
		matrix.m[0][3] = 0.f;

		matrix.m[1][0] = SR * SP * CY - CR * SY;
		matrix.m[1][1] = SR * SP * SY + CR * CY;
		matrix.m[1][2] = -SR * CP;
		matrix.m[1][3] = 0.f;

		matrix.m[2][0] = -(CR * SP * CY + SR * SY);
		matrix.m[2][1] = CY * SR - CR * SP * SY;
		matrix.m[2][2] = CR * CP;
		matrix.m[2][3] = 0.f;

		matrix.m[3][0] = 0.f;
		matrix.m[3][1] = 0.f;
		matrix.m[3][2] = 0.f;
		matrix.m[3][3] = 1.f;

		return matrix;
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

			if (!ReadProcessMemory(hProcess, (LPVOID)(boneArray + 0x8 + (i * 0x38)), &bonePos, sizeof(Vector3), nullptr)) {
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
	static std::string ReadStringFromMemory(HANDLE hProcess, uintptr_t address, size_t bufferSize = 256) {
		char* buffer = new char[bufferSize];
		SIZE_T bytesRead = 0;

		// Lese den Speicher des Zielprozesses
		if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, bufferSize - 1, &bytesRead)) {
			buffer[bytesRead] = '\0'; // Sicherstellen, dass der String nullterminiert ist
			std::string result(buffer);
			delete[] buffer;
			return result;
		}

		delete[] buffer;
		return ""; // Rückgabe eines leeren Strings bei Fehler
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

		while (Process32Next(snapshot, &processInfo))
		{
			if (!_tcscmp(processInfo.szExeFile, processName))
			{
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
	uintptr_t address;
	std::vector<BYTE> originalBytes;
	size_t patchSize;
	bool isPatched = false;  // Ob der Patch aktiv ist

public:
	ExBytePatcher(HANDLE process, uintptr_t addr, size_t size)
		: hProcess(process), address(addr), patchSize(size) {
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

	bool Patch() {
		if (originalBytes.empty()) {
			originalBytes.resize(patchSize);
			ReadProcessMemory(hProcess, (LPCVOID)address, originalBytes.data(), patchSize, NULL);
		}
		std::vector<BYTE> nopBytes(patchSize, 0x90);
		return WriteProcessMemory(hProcess, (LPVOID)address, nopBytes.data(), patchSize, NULL);
	}

	bool Restore() {
		if (originalBytes.empty()) return false;
		return WriteProcessMemory(hProcess, (LPVOID)address, originalBytes.data(), patchSize, NULL);
	}

	bool IsPatched() { return isPatched; }
};
class MemoryManager {
private:
	HANDLE hProcess;

public:
	MemoryManager(HANDLE process) : hProcess(process) {}

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

	// ✅ Detouring einer Funktion in eine CodeCave
	void DetourEx(LPVOID targetFunction, LPVOID codecaveAddress) {
		SIZE_T bytesWritten;
		BYTE clearByte[] = { 0x90 };

		// CodeCave leeren
		for (int i = 0; i < 100; i++) {
			WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)codecaveAddress + i), &clearByte, sizeof(clearByte), NULL);
		}

		// Sprung zur CodeCave
		BYTE jmpBytes[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		WriteProcessMemory(hProcess, targetFunction, jmpBytes, sizeof(jmpBytes), NULL);
		WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)targetFunction + 6), &codecaveAddress, sizeof(codecaveAddress), NULL);

		// Ursprüngliche Funktion in CodeCave wiederherstellen
		BYTE originalBytes[] = {
			0x48, 0x8B, 0x89, 0xB0, 0x00, 0x00, 0x00, 0x0F, 0x28, 0xF3, 0x49, 0x8B, 0xD8, 0x48, 0x85, 0xC9
		};
		WriteProcessMemory(hProcess, codecaveAddress, originalBytes, sizeof(originalBytes), &bytesWritten);

		// Rücksprung zu alter Adresse
		BYTE backJmpBytes[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		uintptr_t backOffset = (uintptr_t)((uintptr_t)targetFunction + 0x10);
		WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)codecaveAddress + 30), backJmpBytes, sizeof(backJmpBytes), NULL);
		WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)codecaveAddress + 36), &backOffset, sizeof(backOffset), NULL);
	}

	// ✅ Funktion mit NOP überschreiben
	void NopFunc(LPVOID targetFunction, int bytes) {
		std::vector<BYTE> nopBytes(bytes, 0x90);
		WriteProcessMemory(hProcess, targetFunction, nopBytes.data(), bytes, NULL);
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
	FlyHack(HANDLE process, uintptr_t baseAddress)
		: hProcess(process) {
		// Find the velocity memory address using pointer path
		velocityAddress = MemoryManager::FindDMAAddy(process, baseAddress + 0x053F84E0, { 0x0, 0x300, 0x38, 0xC0, 0xC4 });
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
	std::atomic<bool> stopThread{ false }; // Steuerung für den Adress-Update-Thread
	std::thread addressThread;

	std::vector<std::pair<std::string, ImU32>> entityFilters = {
		{"dog", IM_COL32(0, 0, 255, 255)},
		{"deer", IM_COL32(0, 255, 0, 255)},
		{"hare", IM_COL32(0, 255, 0, 255)},
		{"cow", IM_COL32(0, 255, 0, 255)},
		{"bull", IM_COL32(0, 255, 0, 255)},
		{"pig", IM_COL32(0, 255, 0, 255)},
		{"wilddog", IM_COL32(255, 0, 0, 255)},
		{"sheep", IM_COL32(0, 255, 0, 255)},
		{"wolf", IM_COL32(255, 0, 0, 255)},
		{"Boar", IM_COL32(255, 0, 0, 255)},
		{"enemy", IM_COL32(255, 0, 0, 255)},
		{"woman", IM_COL32(0, 0, 255, 255)},
		{"man", IM_COL32(0, 0, 255, 255)},
		{"horse", IM_COL32(0, 0, 255, 255)}
	};

public:

	int validEntites;

	EntityManager(HANDLE process, uintptr_t baseAddress)
		: hProcess(process), entityListAddress(baseAddress) {
		StartAddressUpdater();
	}

	~EntityManager() {
		stopThread = true;
		if (addressThread.joinable()) {
			addressThread.join(); // Thread sicher beenden
		}
	}

	// Starte den separaten Thread für die Adress-Aktualisierung
	void StartAddressUpdater() {
		addressThread = std::thread([this]() {
			while (!stopThread) {
				UpdateEntityAddresses();
				std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Update alle 500ms
			}
			});
	}

	// Aktualisiert nur die Adressen der Entities
	void UpdateEntityAddresses() {
		const int entityCount = 3000;
		uintptr_t entListAdr = MemoryManager::FindDMAAddy(hProcess, (entityListAddress + 0x052A39D0), { 0x0 });

		std::vector<uintptr_t> updatedAddresses;

		for (int i = 0; i < entityCount; ++i) {
			uintptr_t pointerAddress = entListAdr + (i * 0x8);
			uintptr_t entBase = 0x0;

			if (ReadProcessMemory(hProcess, (LPVOID)(pointerAddress), &entBase, sizeof(entBase), nullptr)) {
				if (!IsBadReadPtr(&entBase, sizeof(uintptr_t))) {
					updatedAddresses.push_back(entBase);
				}
			}
		}

		entityAddresses = std::move(updatedAddresses); // Adressen-Cache sicher ersetzen
	}

	// Nutzt die gespeicherten Adressen, um Entities zu aktualisieren
	void UpdateEntities(Vector3 camPos, float maxDistance) {
		validEntites = 0;
		entityCache.clear();
		for (uintptr_t entBase : entityAddresses) {
			uintptr_t posAddr = MemoryManager::FindDMAAddy(hProcess, entBase + 0x18, { 0x30 });
			uintptr_t nameAddr = MemoryManager::FindDMAAddy(hProcess, entBase + 0x18, { 0xE8, 0x0 });

			Vector3 pos;
			ReadProcessMemory(hProcess, (LPVOID)(posAddr), &pos, sizeof(pos), nullptr);
			float distance = RenderHelper::Distance3D(camPos, pos);

			if (distance < maxDistance) {
				Entity& entity = entityCache[entBase];
				entity.baseAddress = entBase;
				entity.posAddress = posAddr;
				entity.nameAddress = nameAddr;
				entity.position = pos;
				entity.distance = distance;
				validEntites++;
			}
		}

		for (auto& [entBase, entity] : entityCache) {
			entity.name = FilterEntityName(ReadEntityName(entity.nameAddress));
		}
	}

	void RenderEntities(ImDrawList* drawList, float screenWidth, float screenHeight, float* matrix) {
		for (const auto& [entBase, entity] : entityCache) {
			Vector2 screenPos;
			if (RenderHelper::W2SCryEngine(entity.position, screenPos, matrix, screenWidth, screenHeight)) {
				if (screenPos.x > 0 && screenPos.x < screenWidth && screenPos.y > 0 && screenPos.y < screenHeight) {
					ImU32 color = GetEntityColor(entity.name);
					std::ostringstream oss;
					oss << entity.name << " " << std::fixed << std::setprecision(2) << entity.distance << "m";

					drawList->AddText(ImVec2(RenderHelper::CalcMiddlePos(screenPos.x, oss.str().c_str()), screenPos.y), color, oss.str().c_str());
				}
			}
		}
	}

private:
	std::string ReadEntityName(uintptr_t nameAddr) {
		return InitHax::ReadStringFromMemory(hProcess, nameAddr, 30);
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







