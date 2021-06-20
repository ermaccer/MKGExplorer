#include "..\core\App.h"
#include "MKGExplorer.h"
#include <CommCtrl.h>
#include <shlobj.h>
#include <WinUser.h>
#include <filesystem>
#include <windowsx.h>
#include "..\core\IniReader.h"
#include "..\core\FileFunctions.h"
#include "..\resource.h"
#include <memory>
#include <iostream>
#include <string>
#include "image.h"

void MKGExplorer::Init(HWND* log)
{
	eApp::bIsReady = false;
	hLogBox = log;

}


void MKGExplorer::OpenFile(std::wstring input, bool image)
{
	if (pFile.is_open())
		pFile.close();
	m_pDataEnd = 0;
	m_bIsImage = image;
	Groups.clear();
	Images.clear();
	InputPath = L" ";
	OutputPath = L" ";
	InputPath = input;
	SetWindowText(*hLogBox, L"");
	SetWindowText(GetDlgItem(eApp::hWindow, MKG_FILENAME), L"");
	SetWindowText(GetDlgItem(eApp::hWindow, MKG_PROGRESS), L"");
	ReadFile();
}


void MKGExplorer::ReadFile()
{
	if (InputPath.empty())
		return;
	pFile.open(InputPath, std::ifstream::binary);

	if (!pFile.is_open())
	{
		MessageBox(eApp::hWindow, L"Failed to open file!", L"Error", MB_ICONERROR);
		return;
	}

	if (pFile.is_open())
	{
		Log(L"Opening: " + wsplitString(InputPath, true));

		geo_header geo;
		pFile.read((char*)&geo, sizeof(geo_header));

		if (!(geo.header[0] == '0' && geo.header[1] == '.' && geo.header[2] == '1' && geo.header[3] == 'v'))
		{
			MessageBox(eApp::hWindow, L"Input file is not a valid Geometry or Image file!", L"Error", MB_ICONERROR);
			return;
		}

		m_pDataEnd = (int)pFile.tellg();

		SetWindowText(GetDlgItem(eApp::hWindow, MKG_FILENAME),InputPath.c_str());
		//m_secHeader = sec;

		GEO = geo;

		if (!m_bIsImage)
		{
			Log(L"Models: " + std::to_wstring(geo.files));
			std::vector<geo_entry> Models;
			std::vector<int> vTempFaces;
			for (int i = 0; i < geo.files; i++)
			{
				geo_entry ent;
				pFile.read((char*)&ent, sizeof(geo_entry));
				Models.push_back(ent);
			}

			for (int i = 0; i < geo.files; i++)
			{
				Log(L"Reading " + std::to_wstring(i + 1) + L"/" + std::to_wstring(geo.files));
				std::vector<v> vVerts;
				std::vector<uv> vMaps;
				std::vector<vn> vNorm;
				std::vector<face> vFaces;

				for (int a = 0; a < Models[i].models; a++)
				{
					group_info grp;
					std::string groupName = "G" + std::to_string(i) + "_" + std::to_string(a);
					if (IsDlgButtonChecked(eApp::hWindow, MKG_C_OBJ))
						groupName = "G" + std::to_string(i);

					grp.name = groupName;
					grp.belong = i;
					geometry_info info;
					pFile.read((char*)&info, sizeof(geometry_info));


					for (int x = 0; x < info.vertexes; x++)
					{
						vn norm;
						pFile.read((char*)&norm, sizeof(vn));
						vNorm.push_back(norm);

						v vert;
						pFile.read((char*)&vert, sizeof(v));
						vVerts.push_back(vert);

						uv map;
						pFile.read((char*)&map, sizeof(uv));
						vMaps.push_back(map);

						// no face data,  1 = 123, 2=  246 and so on
						vTempFaces.push_back(x);

					}

					for (int z = 0; z < vTempFaces.size() - 2; z++)
					{
						short f1, f2, f3;
						if (z & 1)
						{
							f1 = vTempFaces[z];
							f2 = vTempFaces[z + 2];
							f3 = vTempFaces[z + 1];
						}
						else
						{
							f1 = vTempFaces[z];
							f2 = vTempFaces[z + 1];
							f3 = vTempFaces[z + 2];
						}
						if (f1 == f2 || f2 == f3 || f1 == f3)
							continue;
						face face = { f1 ,f2 ,f3 };
						vFaces.push_back(face);
					}


					vTempFaces.clear();
					grp.Faces = vFaces;
					grp.Maps = vMaps;
					grp.Normals = vNorm;
					grp.Verts = vVerts;

					vFaces.clear();
					vMaps.clear();
					vNorm.clear();
					vVerts.clear();

					Groups.push_back(grp);
				}
			}
		}
		else
		{
			Log(L"Images: " + std::to_wstring(geo.files));

			geo_image_entry img;
			pFile.read((char*)&img, sizeof(geo_image_entry));
			for (int i = 0; i < geo.files; i++)
			{
				Images.push_back(img);
				Log(L"Image " + std::to_wstring(i + 1) + L"/" + std::to_wstring(geo.files) + L" " +  std::to_wstring(img.width) + L"x" + std::to_wstring(img.height));
			}
		}
		eApp::bIsReady = TRUE;

	}
}

void MKGExplorer::ExportToOBJ()
{

	std::wstring folder = SetFolderFromButton(eApp::hWindow);

	if (folder.empty())
		return;

	std::filesystem::current_path(folder);


	std::wstring str = wsplitString(InputPath, true);
	str.replace(str.length() - 4, 4, L".obj");
	std::wstring name = str;
	std::ofstream oFile(name, std::ofstream::binary);

	int baseFaceSize = 0;
	int baseVertSize = 0;
	oFile << "; obj created using mkgexplorer by ermaccer\n" << std::setprecision(4) << std::fixed;

	int todo = Groups.size();

	for (int i = 0; i < Groups.size(); i++)
	{
		todo--;
		char progress[128] = {};
		sprintf(progress, "Progress %.2f%%", (float)(100.0f - (todo * 100.0f / Groups.size())));
		std::string str(progress, strlen(progress));
		std::wstring wstr(L" ", strlen(progress));
		std::copy(str.begin(), str.end(), wstr.begin());
		SetWindowText(GetDlgItem(eApp::hWindow, MKG_PROGRESS), wstr.c_str());
	


		oFile << "; " << i + 1 << std::endl;
		for (int z = 0; z < Groups[i].Verts.size(); z++)
		{
			oFile << "v " << Groups[i].Verts[baseVertSize + z].x << " " << Groups[i].Verts[baseVertSize + z].y << " " << Groups[i].Verts[baseVertSize + z].z << std::endl;
		}
		for (int z = 0; z < Groups[i].Verts.size(); z++)
		{
			oFile << "vn " << Groups[i].Normals[baseVertSize + z].norm[0] << " " << Groups[i].Normals[baseVertSize + z].norm[1] << " " << Groups[i].Normals[baseVertSize + z].norm[2] << std::endl;
		}
		for (int z = 0; z < Groups[i].Verts.size(); z++)
		{
			oFile << "vt " << Groups[i].Maps[baseVertSize + z].u << " " << 1.0f - Groups[i].Maps[baseVertSize + z].v << std::endl;
		}

		oFile << "g " << Groups[i].name.c_str() << std::endl;

		for (int x = 0; x < Groups[i].Faces.size(); x++)
		{
			int temp[3];
			temp[0] = Groups[i].Faces[x].face[0] + baseFaceSize;
			temp[1] = Groups[i].Faces[x].face[1] + baseFaceSize;
			temp[2] = Groups[i].Faces[x].face[2] + baseFaceSize;


			oFile << "f " << temp[0] + 1 << "/" << temp[0] + 1 << "/" << temp[0] + 1
				<< " "
				<< temp[1] + 1 << "/" << temp[1] + 1 << "/" << temp[1] + 1
				<< " "
				<< temp[2] + 1 << "/" << temp[2] + 1 << "/" << temp[2] + 1 << std::endl;
		}


		baseFaceSize += Groups[i].Verts.size();

	}
	Log(L"Saved " + name);
	oFile.close();
}

void MKGExplorer::ExportToSMD()
{
	std::wstring folder = SetFolderFromButton(eApp::hWindow);

	if (folder.empty())
		return;

	std::filesystem::current_path(folder);

	std::wstring str = wsplitString(InputPath, true);
	str.replace(str.length() - 4, 4, L".smd");
	std::wstring name = str;
	std::ofstream oFile(name, std::ofstream::binary);
	oFile << "version 1\nnodes\n" << std::setprecision(4) << std::fixed;
	for (int a = 0; a < GEO.files; a++)
	{
		int parent = -1;
		oFile << a<< " \"G" << std::to_string(a).c_str() << "\" "
			<< parent << std::endl;
	}
	oFile << "end\n";
	oFile << "skeleton\n";
	oFile << "time 0\n";
	for (int a = 0; a < GEO.files; a++)
	{
		oFile << std::to_string(a).c_str() << " 0.0 0.0 0.0 0.0 0.0 0.0\n";
	}

	oFile << "end\n";
	oFile << "triangles\n";

	int todo = Groups.size();
	for (int i = 0; i < Groups.size(); i++)
	{
		todo--;
		char progress[128] = {};
		sprintf(progress, "Progress %.2f%%", (float)(100.0f - (todo * 100.0f / Groups.size())));
		std::string str(progress, strlen(progress));
		std::wstring wstr(L" ", strlen(progress));
		std::copy(str.begin(), str.end(), wstr.begin());
		SetWindowText(GetDlgItem(eApp::hWindow, MKG_PROGRESS), wstr.c_str());

		for (int a = 0; a < Groups[i].Faces.size(); a++)
		{
			oFile << "default" << std::endl;
			for (int k = 0; k < 3; k++)
			{
				v     verts;
				vn     normal;
				uv     maps;
				verts = Groups[i].Verts[Groups[i].Faces[a].face[k]];
				normal = Groups[i].Normals[Groups[i].Faces[a].face[k]];

				maps = Groups[i].Maps[Groups[i].Faces[a].face[k]];

				oFile << 0 << " " << verts.x << " " << verts.y << " " << verts.z << " "
					<< normal.norm[0] << " " << normal.norm[0] << " " << normal.norm[2] << " "
					<< maps.u << " " << 1.0 - maps.v;
				oFile << " " << 1 << " " << Groups[i].belong << " " << 1.0f;
				oFile << std::endl;
			}
		}

	}		

	oFile << "end\n";
	Log(L"Saved " + name);
	oFile.close();
}

void MKGExplorer::ExportToBMP()
{
	std::wstring folder = SetFolderFromButton(eApp::hWindow);

	if (folder.empty())
		return;

	std::filesystem::current_path(folder);

	pFile.seekg(sizeof(geo_header), pFile.beg);
	geo_image_entry img;
	pFile.read((char*)&img, sizeof(geo_image_entry));

	for (int i = 0; i < GEO.files; i++)
	{

		std::wstring str = std::to_wstring(i) + L"_";
		str += wsplitString(InputPath, true);
		str.replace(str.length() - 4, 4, L".bmp");
		std::wstring name = str;
		std::ofstream oFile(name, std::ofstream::binary);

		// create bmp
		bmp_header bmp;
		bmp_info_header bmpf;
		bmp.bfType = 'MB';
		bmp.bfSize = (img.width * img.height) * 2;
		bmp.bfReserved1 = 0;
		bmp.bfReserved2 = 0;
		bmp.bfOffBits = sizeof(bmp_header) + sizeof(bmp_info_header);
		bmpf.biSize = sizeof(bmp_info_header);
		bmpf.biWidth = img.width;
		bmpf.biHeight = img.height;
		bmpf.biPlanes = 1;
		bmpf.biBitCount = 16;
		bmpf.biCompression = 0;
		bmpf.biXPelsPerMeter = 2835;
		bmpf.biYPelsPerMeter = 2835;
		bmpf.biClrUsed = 0;
		bmpf.biClrImportant = 0;

		oFile.write((char*)&bmp, sizeof(bmp_header));
		oFile.write((char*)&bmpf, sizeof(bmp_info_header));

		std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>((img.width * img.height) * 2);

		pFile.read(dataBuff.get(), (img.width * img.height) * 2);
		oFile.write(dataBuff.get(), (img.width * img.height) * 2);


		Log(L"Saved: " + name);

	}


}


void MKGExplorer::Close()
{
	pFile.close();
	Images.clear();
	Groups.clear();
	InputPath = L" ";
	OutputPath = L" ";
	SetWindowText(*hLogBox, L"");
	SetWindowText(GetDlgItem(eApp::hWindow, MKG_FILENAME), L"");
	SetWindowText(GetDlgItem(eApp::hWindow, MKG_PROGRESS), L"");
	eApp::bIsReady = false;

}

void MKGExplorer::Log(std::wstring msg)
{
	PushLogMessage(*hLogBox, msg);
}


std::wstring SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetOpenFileName(&ofn))
		out = szBuffer;

	return out;
}

std::wstring SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetSaveFileName(&ofn))
		out = szBuffer;

	return out;
}

std::wstring SetSavePathFromButtonWithName(std::wstring name, wchar_t * filter, wchar_t * ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	wsprintf(szBuffer, name.c_str());
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetSaveFileName(&ofn))
		out = szBuffer;

	return out;
}


std::wstring   SetFolderFromButton(HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH];

	BROWSEINFO bi = {};
	bi.lpszTitle = L"Select Folder";
	bi.hwndOwner = hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST idl = SHBrowseForFolder(&bi);

	std::wstring out;

	if (idl)
	{
		SHGetPathFromIDList(idl, szBuffer);
		out = szBuffer;

	}

	return out;
}

void PushLogMessage(HWND hWnd, std::wstring msg)
{
	msg += L"\r\n";
	int len = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(hWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
	SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)msg.c_str());
}
