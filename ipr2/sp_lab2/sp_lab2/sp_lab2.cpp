// sp_lab2.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "sp_lab2.h"
#include "windows.h"
#include "stdio.h"
#include "commdlg.h"
#include "fileapi.h"
#include "winioctl.h"
#include <iostream>
#include <atlstr.h>


#define MAX_LOADSTRING 64

HINSTANCE hInst;
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
WCHAR szTitle[MAX_LOADSTRING] = L"sp_lab2";
WCHAR szWindowClass[MAX_LOADSTRING] = L"sp_lab2WindowClass";

struct DiskInfo {
	LPCWSTR lpDiskName;

	ULARGE_INTEGER lpFreeBytesAvailable;
	ULARGE_INTEGER lpTotalNumberOfBytes;
	ULARGE_INTEGER lpTotalNumberOfFreeBytes;

	DWORD lpSectorsPerCluster;
	DWORD lpBytesPerSector;
	DWORD lpNumberOfFreeClusters;
	DWORD lpTotalNumberOfClusters;

	DWORD longestClustersChainLength;
	DWORD clustersChainsCount;
};

bool GetDiskInfo(DiskInfo* diskInfo) {
	// функция для объема диска
	GetDiskFreeSpaceEx(
		L"C:",
		&diskInfo->lpFreeBytesAvailable,
		&diskInfo->lpTotalNumberOfBytes,
		&diskInfo->lpTotalNumberOfFreeBytes
	);

	// подсчёт свободных кластеров
	GetDiskFreeSpace(
		L"C:",
		&diskInfo->lpSectorsPerCluster,
		&diskInfo->lpBytesPerSector,
		&diskInfo->lpNumberOfFreeClusters,
		&diskInfo->lpTotalNumberOfClusters
	);

	// открываем диск как HANDLE для DeviceIoControl, чтобы найти остальные атрибуты
	diskInfo->lpDiskName = L"\\\\.\\C:";
	HANDLE diskForCycle = CreateFile(diskInfo->lpDiskName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

	if (diskForCycle == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, L"Failed to open disk.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	DWORD tmp;
	DWORD lastError;
	DWORD clustersChainsCount = 0;
	DWORD longestClustersChainLength = 0;
	auto startingVcn = new STARTING_VCN_INPUT_BUFFER();
	auto pointersBuffer = new RETRIEVAL_POINTERS_BUFFER();
	do {
		DeviceIoControl(diskForCycle, 
			FSCTL_GET_RETRIEVAL_POINTERS, 
			startingVcn, 
			sizeof(*startingVcn),
			pointersBuffer, 
			sizeof(*pointersBuffer),
			&tmp, 
			NULL);

		lastError = GetLastError();

		if (lastError != ERROR_HANDLE_EOF) {
			++clustersChainsCount;
			auto currentClustersChainLength = pointersBuffer->Extents->NextVcn.QuadPart - 
				pointersBuffer->StartingVcn.QuadPart;
			if (currentClustersChainLength > longestClustersChainLength) {
				longestClustersChainLength = currentClustersChainLength;
			}
			startingVcn->StartingVcn.QuadPart = pointersBuffer->Extents->NextVcn.QuadPart;
		}
	} while (lastError == ERROR_MORE_DATA);

	diskInfo->clustersChainsCount = clustersChainsCount;
	diskInfo->longestClustersChainLength = longestClustersChainLength;

	CloseHandle(diskForCycle);
	return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; 
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^
		WS_THICKFRAME,
		CW_USEDEFAULT, 0, 600, 600, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hDiskDescriptionText;
	static HWND hFreeBytesCountText;
	static HWND hNumberOfFreeClustersText;
	static HWND hClustersChainsCountText;
	static HWND hLongestClustersChainLengthText;
	static HWND hStartButton;
	switch (message)
		{
		case WM_CREATE: {
			hStartButton = CreateWindow(L"button", L"Show disk space and other attributes", WS_CHILD | WS_VISIBLE, 100, 25,
				400, 25, hWnd, 0, hInst, NULL);

			hDiskDescriptionText = CreateWindow(L"static", NULL, WS_CHILD | WS_VISIBLE, 25,	75, 550, 50, hWnd, 0, hInst, NULL);
			hFreeBytesCountText = CreateWindow(L"static", NULL, WS_CHILD | WS_VISIBLE, 25, 135, 550, 50, hWnd, 0, hInst, NULL);
			hNumberOfFreeClustersText = CreateWindow(L"static", NULL, WS_CHILD | WS_VISIBLE, 25, 195, 550, 50, hWnd, 0, hInst, NULL);
			hClustersChainsCountText = CreateWindow(L"static", NULL, WS_CHILD |	WS_VISIBLE, 25, 265, 550, 50, hWnd, 0, hInst, NULL);
			hLongestClustersChainLengthText = CreateWindow(L"static", NULL, WS_CHILD |	WS_VISIBLE, 25, 325, 550, 50, hWnd,
				0, hInst, NULL);
	}break;
	case WM_COMMAND: {
		if ((HWND)lParam == hStartButton) {
			DiskInfo disk;
			GetDiskInfo(&disk);

			TCHAR attribute[64];

			swprintf_s(attribute, L"Disk name: C");
			SetWindowText(hDiskDescriptionText, attribute);

			swprintf_s(attribute, L"Disk free bytes: %lld", disk.lpFreeBytesAvailable);
			SetWindowText(hFreeBytesCountText, attribute);

			swprintf_s(attribute, L"Number of free clusters: %d", disk.lpNumberOfFreeClusters);
			SetWindowText(hNumberOfFreeClustersText, attribute);

			swprintf_s(attribute, L"Clusters Chains Count: %d", disk.clustersChainsCount);
			SetWindowText(hClustersChainsCountText, attribute);

			swprintf_s(attribute, L"Longest clusters chain length: %d", disk.longestClustersChainLength);
			SetWindowText(hLongestClustersChainLengthText, attribute);
		}
	}break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//format message