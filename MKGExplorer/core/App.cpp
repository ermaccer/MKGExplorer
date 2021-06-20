#include "App.h"
#include "..\code\MKGExplorer.h"
#include "..\resource.h"
#include "FileFunctions.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <filesystem>
#include "IniReader.h"

HINSTANCE        eApp::hInst;
HWND             eApp::hWindow;
HWND             eApp::hList;
HWND             eApp::hDataBox;
HWND             eApp::hLog;
HWND             eApp::hTable;
HMENU            eApp::hMenu;
DWORD            eApp::dwSel;
BOOL             eApp::bIsReady;
BOOL             eApp::bIsIni;
int              eApp::nGameMode;


MKGExplorer*  eApp::pMKGExplorer;


INT_PTR CALLBACK eApp::Process(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	hWindow = hDlg;
	HANDLE hicon = 0;

	if (!bIsReady)
	{
		EnableWindow(GetDlgItem(hDlg, MKG_OBJ), FALSE);
		EnableWindow(GetDlgItem(hDlg, MKG_SMD), FALSE);
		EnableWindow(GetDlgItem(hDlg, MKG_BMP), FALSE);
		EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_DISABLED);
	}
	else
	{
		if (pMKGExplorer)
		{
			if (!pMKGExplorer->m_bIsImage)
			{
				EnableWindow(GetDlgItem(hDlg, MKG_OBJ), TRUE);
				EnableWindow(GetDlgItem(hDlg, MKG_SMD), TRUE);
				EnableWindow(GetDlgItem(hDlg, MKG_BMP), FALSE);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, MKG_OBJ), FALSE);
				EnableWindow(GetDlgItem(hDlg, MKG_SMD), FALSE);
				EnableWindow(GetDlgItem(hDlg, MKG_BMP), TRUE);
			}

		}

		EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_ENABLED);
	}




	switch (message)
	{
	case WM_INITDIALOG:
		Reset();
		hLog = GetDlgItem(hDlg, MKG_LOG);
		PushLogMessage(hLog, L"MKG Explorer v" + (std::wstring)MKG_EXPLORER_VERSION + L" ready!");
		CreateTooltip(GetDlgItem(hDlg, MKG_C_OBJ), L"Force OBJ group submeshes to be one group");
		hMenu = GetMenu(hDlg);
		pMKGExplorer = new MKGExplorer();
		hicon = LoadImage(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_MKGEXPLORER), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hicon);

		return (INT_PTR)TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	case WM_COMMAND:

		if (LOWORD(wParam) == ID_OPEN_GEOMETRY)
		{
			pMKGExplorer = new MKGExplorer();
			pMKGExplorer->Init(&hLog);
			pMKGExplorer->OpenFile(SetPathFromButton(L"Geometry File\0*.geo\0All Files (*.*)\0*.*\0", L"geo", hDlg), false);
		}

		if (LOWORD(wParam) == ID_OPEN_IMAGE)
		{
			pMKGExplorer = new MKGExplorer();
			pMKGExplorer->Init(&hLog);
			pMKGExplorer->OpenFile(SetPathFromButton(L"Image File\0*.img\0All Files (*.*)\0*.*\0", L"img", hDlg), true);
		}

		if (wParam == IDM_ABOUT)
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);

		if (LOWORD(wParam) == ID_FILE_CLOSE)
			pMKGExplorer->Close();

		if (wParam == MKG_OBJ)
			pMKGExplorer->ExportToOBJ();

		if (wParam == MKG_SMD)
			pMKGExplorer->ExportToSMD();
		if (wParam == MKG_BMP)
			pMKGExplorer->ExportToBMP();

		if (LOWORD(wParam) == IDM_EXIT)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

	}
	return (INT_PTR)FALSE;
}

INT_PTR eApp::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void eApp::Reset()
{
	nGameMode = 0;
}

void eApp::CreateTooltip(HWND hWnd, LPCWSTR text)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP |
		TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL,
		hInst, NULL);
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = hWindow;
	ti.hinst = NULL;
	ti.uId = (UINT_PTR)hWnd;
	ti.lpszText = (LPWSTR)text;

	RECT rect;
	GetClientRect(hWnd, &rect);

	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void eApp::UpdateGameChange()
{

}

void eApp::Begin()
{
	pMKGExplorer = nullptr;
	DialogBox(hInst, MAKEINTRESOURCE(MKG_EXP), 0, Process);
}
