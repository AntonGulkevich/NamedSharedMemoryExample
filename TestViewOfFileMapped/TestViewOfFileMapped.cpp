// TestViewOfFileMapped.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#define UBS_MEM_SIZE 0xffffffff
#define PAGE_SIZE 0x10000
#define MAPFILENAME TEXT("dllmemfilemap")

bool writeData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset);
bool readData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset);

#define UCU_IORD_32DIRECT(base, offset) read_dword(base, offset)
#define UCU_IOWR_32DIRECT(base, offset, data) write_dword(base, offset, data)

#define UCU_IORD_32DIRECTF(base, offset) read_float(base, offset)
#define UCU_IOWR_32DIRECTF(base, offset, data) write_float(base, offset, data)

DWORD read_dword(DWORD base, DWORD offset);
VOID write_dword(DWORD base, DWORD offset, DWORD data);

FLOAT read_float(DWORD base, DWORD offset);
VOID write_float(DWORD base, DWORD offset, FLOAT data);

bool writeData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset)
{
	DWORD multiOffset = (offset / PAGE_SIZE)*PAGE_SIZE;
	DWORD deltaOffset = offset - multiOffset;
	LPBYTE lpvMem = static_cast<LPBYTE> (MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, multiOffset, size+ deltaOffset));
	if (lpvMem == nullptr)	return FALSE;
	lpvMem += deltaOffset;
	while (size--)	*lpvMem++ = *data++;
	UnmapViewOfFile(lpvMem);
	return TRUE;
}

bool readData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset)
{
	DWORD multiOffset = (offset / PAGE_SIZE)*PAGE_SIZE;
	DWORD deltaOffset = offset - multiOffset;
	LPBYTE lpvMem = static_cast<LPBYTE> (MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, multiOffset, size + deltaOffset));
	if (lpvMem == nullptr)	return FALSE;
	lpvMem += deltaOffset;
	memcpy(data, lpvMem, size);
	UnmapViewOfFile(lpvMem);
	return TRUE;
}

FLOAT read_float(DWORD base, DWORD offset)
{
	HANDLE hMapObject = OpenFileMapping(	FILE_MAP_READ | FILE_MAP_WRITE, FALSE, MAPFILENAME);
	if (hMapObject == nullptr) return NULL;
	int size = sizeof(DWORD);
	LPBYTE buff = new BYTE[size];
	readData(hMapObject, buff, size, base + offset);
	FLOAT res;
	memcpy(&res, buff, size);
	delete[] buff;
	CloseHandle(hMapObject);	
	return res;
}
DWORD read_dword(DWORD base, DWORD offset)
{
	HANDLE hMapObject = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, MAPFILENAME);
	if (hMapObject == nullptr) return NULL;
	int size = sizeof(DWORD);
	LPBYTE buff = new BYTE[size];
	readData(hMapObject, buff, size, base+offset);
	DWORD res;
	memcpy(&res, buff, size);
	delete[] buff;
	CloseHandle(hMapObject);
	return res;
}

void write_float(DWORD base, DWORD offset, FLOAT data)
{
	HANDLE hMapObject = OpenFileMapping(	FILE_MAP_READ | FILE_MAP_WRITE, FALSE, MAPFILENAME);
	if (hMapObject == nullptr) return;
	int size = sizeof(FLOAT);
	LPBYTE buff = new BYTE[size];
	memcpy(buff, &data, size);
	writeData(hMapObject, buff, size, base + offset);
	delete[] buff;
	CloseHandle(hMapObject);
}

void write_dword(DWORD base, DWORD offset, DWORD data)
{
	HANDLE hMapObject = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, MAPFILENAME);
	if (hMapObject == nullptr) return;
	int size = sizeof(DWORD);
	LPBYTE buff = new BYTE[size];
	memcpy(buff, &data, sizeof(DWORD));
	writeData(hMapObject, buff, size, base + offset);
	delete[] buff;
	CloseHandle(hMapObject);
}

int main()
{
	HANDLE hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, UBS_MEM_SIZE, MAPFILENAME);
	if (hMapObject == nullptr) {
		std::cerr << "Could not create file: error code: " << GetLastError() << std::endl;
		system("PAUSE");
		return FALSE;
	}
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	std::cout << "Allocation Granularity = " << systemInfo.dwAllocationGranularity << std::endl;

	BYTE testStr[] = "Test.";
	int size = sizeof(testStr);
	LPBYTE test_buff = new BYTE[size];
	memcpy(test_buff,testStr, size);	
	std::cout << "Default string: " << test_buff << std::endl;
	writeData(hMapObject, test_buff, size, 0);
	LPBYTE rdStr = new BYTE[size];
	readData(hMapObject, rdStr, size, 0);
	std::cout << "Read string: " << rdStr << std::endl;

	FLOAT float_base = -12345.6789;
	DWORD dw1 = 1234567890;
	UCU_IOWR_32DIRECT(size, 0, dw1);
	UCU_IOWR_32DIRECTF(size, sizeof(DWORD), float_base);
	FLOAT float_red = UCU_IORD_32DIRECTF(size, sizeof(DWORD));
	DWORD dw = UCU_IORD_32DIRECT(size, 0);

	std::cout << "FLOAT: " << std::to_string(float_red) << std::endl << "DWORD: " << std::to_string(dw) << std::endl;
	system("PAUSE");

	CloseHandle(hMapObject);
	delete[] test_buff;
	delete[] rdStr;


	_CrtDumpMemoryLeaks();
    return 0;
}

