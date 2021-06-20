#pragma once
#include <Windows.h>
#include "..\code\MKGExplorer.h"


class eApp {
public:
	static HINSTANCE hInst;
	static HWND      hWindow;
	static HWND      hList;
	static HWND      hDataBox;
	static HWND      hLog;
	static HWND      hTable;
	static HMENU     hMenu;
	static DWORD     dwSel;
	static BOOL      bIsReady;
	static BOOL      bIsIni;
	static int       nGameMode;
	static MKGExplorer*    pMKGExplorer;
	static INT_PTR CALLBACK Process(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static void Reset();
	static void CreateTooltip(HWND hWnd, LPCWSTR text);
	static void UpdateGameChange();
	static void Begin();
};