#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <d3dx9.h>
#include <directxmath.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <psapi.h>
#include <iomanip>
#include <TlHelp32.h>
#include <tchar.h>  
#include <iostream>
#include <cmath>





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
	std::string ReadStringFromMemory(HANDLE hProcess, uintptr_t address, size_t bufferSize = 256) {
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
uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);
uintptr_t ScanAOB(std::vector<int> signature, const wchar_t* ModBaseName, HANDLE hProcess, DWORD ProcID);
std::string getNameFromID(int ID, uintptr_t GNames);
void ReadMemory(const void* address, void* buffer, size_t size);
D3DXMATRIX ToMatrix(Vector3 Rotation);


float Distance3D(const Vector3& point1, const Vector3& point2) {
	return point1.DistTo(point2);
}
uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}
std::string getNameFromID(int ID, uintptr_t GNames)
{
	char charName[35];
	uintptr_t fNamePtr = *(uintptr_t*)(GNames + int(ID / 0x4000) * 0x8);
	uintptr_t fName = *(uintptr_t*)(fNamePtr + 0x8 * int(ID % 0x4000));
	const void* bName = (uintptr_t*)(fName + 0x10);
	ReadMemory(bName, charName, sizeof(charName));
	std::stringstream ss;
	std::string Name;
	ss << charName;
	ss >> Name;

	return Name;
}
char* getNameFromIDEx(int ID, uintptr_t GNamesAdr, HANDLE hProcess)
{
	uintptr_t GNames;
	uintptr_t fNamePtr;
	uintptr_t fName;
	char charName[20];
	ReadProcessMemory(hProcess, (uintptr_t*)(GNamesAdr + int(ID / 0x4000) * 0x8), &fNamePtr, sizeof(fNamePtr), nullptr);
	ReadProcessMemory(hProcess, (uintptr_t*)(0x147720E50 + 0x8 * int(ID % 0x4000)), &fName, sizeof(fName), nullptr);
	ReadProcessMemory(hProcess, (uintptr_t*)(fName + 0x10), &charName, sizeof(charName), nullptr);
	

	return charName;
}
char GetNameFromFName(int ID, uintptr_t GNamesAdr, HANDLE hProcess)
{
	char Name;
	DWORD_PTR fNamePtr;
	DWORD_PTR fName;
	// Read the fNamePtr
	ReadProcessMemory(hProcess, (LPCVOID)(GNamesAdr + int(ID / 0x4000) * 0x8), &fNamePtr, sizeof(DWORD_PTR), NULL);

	// Read the fName
	ReadProcessMemory(hProcess, (LPCVOID)(GNamesAdr + 0x8 * int(ID % 0x4000)), &fName, sizeof(DWORD_PTR), NULL);
	
	// Read the text struct
	ReadProcessMemory(hProcess, (LPCVOID)(fName + 0x10), &Name, sizeof(Name), NULL);
	

	return Name;

}
void ReadMemory(const void* address, void* buffer, size_t size)
{
	DWORD back = NULL;

	if (VirtualProtect((LPVOID)address, size, PAGE_READWRITE, &back))
	{
		memcpy(buffer, address, size);

		VirtualProtect((LPVOID)address, size, back, &back);
	}
}
bool UEWorldToScreen(const Vector3& worldLoc, Vector2& screenPos, Vector3 Rotation, Vector3 CamPos, float FOV, float ScreenHöhe, float ScreenBreite)
{
	D3DMATRIX tempMatrix = ToMatrix(Rotation); // Matrix

	float PI = 3.1415926535897932f;


	Vector3 vDelta;
	vDelta.x = worldLoc.x - CamPos.x;
	vDelta.y = worldLoc.y - CamPos.y;
	vDelta.z = worldLoc.z - CamPos.z;

	Vector3 vTransformed = {
		vDelta.Dot({ tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2] }),
		vDelta.Dot({ tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2] }),
		vDelta.Dot({ tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2] })
	};

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;
	float screenCenterX = ScreenBreite / 2.0f;
	float screenCenterY = ScreenHöhe / 2.0f;

	screenPos.x = (screenCenterX)+vTransformed.x * (screenCenterX / tanf(FOV * PI / 360.f)) / vTransformed.z;
	screenPos.y = (screenCenterY - vTransformed.y * (screenCenterX / tanf(FOV * PI / 360.f)) / vTransformed.z);

	if (screenPos.x > ScreenBreite || screenPos.x < 0.f || screenPos.y > ScreenHöhe || screenPos.y < 0.f)
		return false;

	return true;
}
bool WorldToScreenFarCry(Vector3 pos, Vector2& screen, float matrix[16], int windowWidth, int windowHeight)
{
	//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
	D3DXVECTOR4 clipCoords;
	clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
	clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
	clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
	clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

	if (clipCoords.w < 0.1f)
		return false;

	//perspective division, dividing by clip.W = Normalized Device Coordinates
	Vector3 NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
	return true;
}

D3DXMATRIX ToMatrix(Vector3 Rotation)
{
	Vector3 origin;
	origin.x = 0.f;
	origin.y = 0.f;
	origin.z = 0.f;
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

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;

}
std::string replaceAll(std::string& str, const std::string& from, const std::string& to) {

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return str;
}
float CalcMiddlePos(float vScreenX, const char* Text)
{
	float itextX = vScreenX - ((strlen(Text) / 2) * 5);
	return itextX;
}



uintptr_t GetModuleBaseAddressEx(const wchar_t* lpszModuleName, DWORD pID, DWORD &pSize) {
	uintptr_t dwModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
	MODULEENTRY32 ModuleEntry32 = { 0 };
	ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnapshot, &ModuleEntry32))
	{
		do {
			if (wcscmp(ModuleEntry32.szModule, lpszModuleName) == 0)
			{
				dwModuleBaseAddress = (uintptr_t)ModuleEntry32.modBaseAddr;
				pSize = ModuleEntry32.modBaseSize;
				break;
			}
		} while (Module32Next(hSnapshot, &ModuleEntry32));


	}
	CloseHandle(hSnapshot);
	return dwModuleBaseAddress;
}
uintptr_t GetDynamicAddressEx(HANDLE hProcess, uintptr_t baseAddress, std::vector<DWORD> offsets) {
	uintptr_t dynamicAddress = baseAddress;
	for (int i = 0; i < offsets.size() - 1; i++)
	{
		ReadProcessMemory(hProcess, (LPCVOID)(dynamicAddress + offsets[i]), &dynamicAddress, sizeof(offsets.at(i)), NULL);
		//std::cout << "Current Adress: " << std::hex << healthAddress << std::endl;
	}
	dynamicAddress += offsets[offsets.size() - 1];
	return dynamicAddress;
}
uintptr_t GetAddressFromSignatureEx(std::vector<int> signature, HANDLE hProcess, uintptr_t pBaseAddress, DWORD pSize) {
	if (pBaseAddress == NULL || hProcess == NULL) {
		return NULL;
	}
	std::vector<byte> memBuffer(pSize);
	if (!ReadProcessMemory(hProcess, (LPCVOID)(pBaseAddress), memBuffer.data(), pSize, NULL)) {
		return NULL;
	}
	for (int i = 0; i < pSize; i++) {
		for (uintptr_t j = 0; j < signature.size(); j++) {
			if (signature.at(j) != -1 && signature[j] != memBuffer[i + j])
				//std::cout << std::hex << signature.at(j) << std::hex << memBuffer[i + j] << std::endl;
				break;
			if (signature[j] == memBuffer[i + j] && j > 0)
				
			if (j + 1 == signature.size())
				return pBaseAddress + i;
		}
	}
	return NULL;
}
uintptr_t ScanAOB(std::vector<int> signature, const wchar_t* ModBaseName, HANDLE hProcess, DWORD ProcID)
{
	DWORD pSize;
	uintptr_t Modulebase = GetModuleBaseAddressEx(ModBaseName, ProcID, pSize);
	uintptr_t signature_pointer = GetAddressFromSignatureEx(signature, hProcess, Modulebase, pSize);

	return signature_pointer;
}
void DetourEx(HANDLE hProcess, LPVOID targetFunctionAddress, LPVOID codecaveAddress)
{
	SIZE_T bytesWritten;


	for (int i = 0; i < 100; i++)
	{
		BYTE clearbyte[] = { 0x90 };
		WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)codecaveAddress + i), &clearbyte, sizeof(clearbyte), NULL);
	}


	//Jmp zur CodeCave auf die originale Func
	BYTE jmpBytes[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	WriteProcessMemory(hProcess, targetFunctionAddress, jmpBytes, sizeof(jmpBytes), NULL);
	WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)targetFunctionAddress + 6), &codecaveAddress, sizeof(codecaveAddress), NULL);

	
	BYTE geklautebytes[] = {
		0x48, 0x8B, 0x89, 0xB0, 0x00, 0x00, 0x00, 0x0F, 0x28, 0xF3, 0x49, 0x8B, 0xD8, 0x48, 0x85, 0xC9
	};
	WriteProcessMemory(hProcess, codecaveAddress, geklautebytes, sizeof(geklautebytes), &bytesWritten); // wiederherstellen der ursprünglichen funktion in CodeCave

	
	BYTE movCode[] = {
	0x48, 0x89, 0x0D, 0x50, 0x00, 0x00, 0x00  // MOV [codecave + 0x50], RCX
	};
	WriteProcessMemory(hProcess, (LPBYTE)codecaveAddress + 20, movCode, sizeof(movCode), &bytesWritten);
	

	// Write the JMP code to the codecave	
	BYTE backjmpBytes[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uintptr_t backrelativeBackOffset = (uintptr_t)((uintptr_t)targetFunctionAddress + ((uintptr_t)0x10)); //0xE = 14 bytes
	WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)codecaveAddress + 30), backjmpBytes, sizeof(backjmpBytes), NULL);	
	WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)codecaveAddress + 36), &backrelativeBackOffset, sizeof(backrelativeBackOffset), NULL);
			

}
void NopFunc(HANDLE hProcess, LPVOID targetFunctionAddress, int bytes)
{
	byte* Nopbytes = new byte[bytes];
	for (int i = 0; i < bytes; i++)
	{
		Nopbytes[i] = 0x90;
	}
	WriteProcessMemory(hProcess, (LPVOID)((ULONGLONG)targetFunctionAddress), Nopbytes, sizeof(Nopbytes), NULL);
}


// Eine Funktion, die einen 3D-Punkt aus der Weltkoordinate in Bildschirmkoordinaten umwandelt
Vector2 WorldToScreen(Vector3 worldPoint, Vector3 CamPos, Vector3 Rotation, float FOV, float screenWidth, float screenHeight)
{
	DirectX::XMFLOAT4X4 worldMatrix;
	float AspektRat = screenHeight / screenWidth;

	// Setze die Weltmatrix auf die Einheitsmatrix
	DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());

	// Anwenden von Translation, Rotation und Skalierung auf die Weltmatrix
	DirectX::XMStoreFloat4x4(&worldMatrix,
		DirectX::XMMatrixScaling(1, 1, 1) *
		DirectX::XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z) *
		DirectX::XMMatrixTranslation(worldPoint.x, worldPoint.y, worldPoint.z));

	DirectX::XMVECTOR forward = DirectX::XMVectorSubtract(DirectX::XMVectorSet(worldPoint.x, worldPoint.y, worldPoint.z, 0), DirectX::XMVectorSet(CamPos.x, CamPos.y, CamPos.z, 0));
	forward = DirectX::XMVector3Normalize(forward);
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(forward, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	right = DirectX::XMVector3Normalize(right);
	DirectX::XMVECTOR up = DirectX::XMVector3Cross(right, forward);
	up = DirectX::XMVector3Normalize(up);

	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMStoreFloat4x4(&viewMatrix, DirectX::XMMatrixSet(
		DirectX::XMVectorGetX(right), DirectX::XMVectorGetX(up), -DirectX::XMVectorGetX(forward), 0.0f,
		DirectX::XMVectorGetY(right), DirectX::XMVectorGetY(up), -DirectX::XMVectorGetY(forward), 0.0f,
		DirectX::XMVectorGetZ(right), DirectX::XMVectorGetZ(up), -DirectX::XMVectorGetZ(forward), 0.0f,
		-DirectX::XMVectorGetX(DirectX::XMVector3Dot(right, DirectX::XMVectorSet(CamPos.x, CamPos.y, CamPos.z, 0))),
		-DirectX::XMVectorGetX(DirectX::XMVector3Dot(up, DirectX::XMVectorSet(CamPos.x, CamPos.y, CamPos.z, 0))),
		DirectX::XMVectorGetX(DirectX::XMVector3Dot(forward, DirectX::XMVectorSet(CamPos.x, CamPos.y, CamPos.z, 0))),
		1.0f));

	DirectX::XMFLOAT4X4 ProjectionMatrix;
	DirectX::XMStoreFloat4x4(&ProjectionMatrix, DirectX::XMMatrixPerspectiveFovLH(glm::radians(FOV), AspektRat, 0.1f, 100.f));
	DirectX::XMFLOAT4X4 WVPMatrix;
	DirectX::XMStoreFloat4x4(&WVPMatrix, DirectX::XMLoadFloat4x4(&worldMatrix) * DirectX::XMLoadFloat4x4(&viewMatrix) * DirectX::XMLoadFloat4x4(&ProjectionMatrix));

	DirectX::XMVECTOR worldPosition = DirectX::XMVectorSet(worldPoint.x, worldPoint.y, worldPoint.z, 1.0f);
	DirectX::XMVECTOR transformedPosition = DirectX::XMVector4Transform(worldPosition, DirectX::XMLoadFloat4x4(&WVPMatrix));

	float ndcX = DirectX::XMVectorGetX(transformedPosition) / DirectX::XMVectorGetW(transformedPosition);
	float ndcY = DirectX::XMVectorGetY(transformedPosition) / DirectX::XMVectorGetW(transformedPosition);

	Vector2 VScreen;
	VScreen.x = (ndcX + 1.0f) * 0.5f * screenWidth;
	VScreen.y = (1.0f - ndcY) * 0.5f * screenHeight;

	return VScreen;
}

void DebugBones(HANDLE hProcess, ImDrawList* drawList, float viewMatrix[16], uintptr_t boneArray, int boneCount ,ImU32 color = IM_COL32(255, 0, 0, 255)) {
	if (!boneArray || boneCount <= 0) return;

	for (int i = 0; i < boneCount; ++i) {
		Vector3 bonePos = { 0, 0, 0 };

		if (!ReadProcessMemory(hProcess, (LPVOID)(boneArray + 0x8 + (i * 0x38)), &bonePos, sizeof(Vector3), nullptr)) {
			continue;
		}

		Vector2 screenPos;
		if (WorldToScreenFarCry(bonePos, screenPos, viewMatrix, 2560, 1440)) {
			if (screenPos.x > 0 && screenPos.x < 2560 && screenPos.y > 0 && screenPos.y < 1440) {
				drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 3.0f, color);
			}
		}
	}
}