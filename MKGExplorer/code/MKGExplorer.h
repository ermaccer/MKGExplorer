#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <fstream>
#include <time.h>
#include "geo.h"
#include "image.h"
#define MKG_EXPLORER_VERSION L"1.0"




class MKGExplorer {
private:
	HWND* hLogBox;
public:
	std::ifstream pFile;
	std::wstring InputPath;
	std::wstring OutputPath;

	std::vector<group_info> Groups;
	std::vector<geo_image_entry> Images;
	geo_header GEO;
	bool	   m_bIsImage;
	int		   m_pDataEnd;

	void         Init(HWND* log);

	void OpenFile(std::wstring input, bool image);
	void ReadFile();

	void ExportToOBJ();
	void ExportToSMD();
	void ExportToBMP();
	void Close();
	void Log(std::wstring msg);


};


std::wstring   SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetSavePathFromButtonWithName(std::wstring name, wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetFolderFromButton(HWND hWnd);
void		   PushLogMessage(HWND hWnd, std::wstring msg);
