//GetProcAddresses
//Argument1: hLibrary - Handle for the Library Loaded
//Argument2: lpszLibrary - Library to Load
//Argument3: nCount - Number of functions to load
//[Arguments Format]
//Argument4: Function Address - Function address we want to store
//Argument5: Function Name -  Name of the function we want
//[Repeat Format]
//
//Returns: FALSE if failure
//Returns: TRUE if successful

#pragma  comment (lib, "Shlwapi.lib")

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <tchar.h>

#include<iostream>
#include <vector>
#include <string>
#include "ProgramJs.h"

#include <vld.h>

#define MAX_LEN_VIRUSNAME 100

int ScanDir(TCHAR *szPath);
bool ScanFile(TCHAR * wszPathFile);

extern CProgramJs*	g_pProgramJs;

int main(void)
{
	int countFileVr = 0;

	TCHAR wszPathFile[512];
	_stprintf_s(wszPathFile,
		_T("S:\\Mau_phanTich\\33_Tuan\\JS.eIframeAcNMe"), 512);
	countFileVr = ScanDir(wszPathFile);

	printf("countVR = %d\n", countFileVr);
	system("pause");
	
	SAFE_DELETE(g_pProgramJs);
	return true;
}

int ScanDir(TCHAR *szPath)
{
	int countFileVirus = 0;
	if (szPath == NULL) return 0;
	if (!PathIsDirectory(szPath)) return 0;
	if (PathIsDirectoryEmpty(szPath))
	{
		return 0;
	}

	// xoá đệ quy thư mục
	WIN32_FIND_DATA	w32Data;

	TCHAR szFileName[MAX_PATH + 1];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	_stprintf_s(szFileName, MAX_PATH, _T("%s\\*.*"), szPath);

	__try
	{
		hFind = FindFirstFile(szFileName, &w32Data);
		if (hFind == INVALID_HANDLE_VALUE) return 0;

		do
		{
			if (!_tcscmp(w32Data.cFileName, _T(".")) || !_tcscmp(w32Data.cFileName, _T("..")))
				continue;
			_stprintf_s(szFileName, MAX_PATH, _T("%s\\%s"), szPath, w32Data.cFileName);

			// if it is directory
			if ((w32Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				ScanDir(szFileName);
				continue;
			}
			else{
				// xu ly file
				if (ScanFile(szFileName))
					countFileVirus++;

			}


		} while (FindNextFile(hFind, &w32Data));
	}
	__finally
	{
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
		}
		RemoveDirectory(szPath);
	}
	return countFileVirus;
}

bool ScanFile(TCHAR * wszPathFile)
{
	SCANRESULT result;

	HANDLE hFile = CreateFileW(wszPathFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD byteHigh = 0;
	DWORD dwSizeData = GetFileSize(hFile, &byteHigh);
	BYTE * pBuff = new BYTE[dwSizeData];
	BOOL bRead = ReadFile(hFile, pBuff, dwSizeData, &byteHigh, NULL);
	if (bRead)
	{
		OutputDebugString(wszPathFile);
		OutputDebugString(L"\n--------------------------------------------------\n");
		EmulJS(&result, pBuff, dwSizeData);
		int t = 1;
	}
	delete[]pBuff;
	return result.HasVirus;
}