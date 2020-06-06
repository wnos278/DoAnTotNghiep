////////////////////////////////////////////////////////////////////////////////
// Description: File test. Quet 1 folder o che do debug. => kiem tra  memleak
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////
#ifndef BUID_UPDATE

#pragma  comment (lib, "Shlwapi.lib")

#include <windows.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <tchar.h>

#include<iostream>
#include <sstream>
#include <vector>
#include <string>
#include "ProgramJs.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
//#include <vld.h>

int ScanDir(TCHAR *szPath);
int g_nCountFileScan = 0;
int g_nCountFileVirus = 0;
long g_lRequestNumber = 0;
BOOL ScanFile(TCHAR * wszPathFile);

extern CProgramJs*	g_pProgramJs;

//------------------------------------------------------------------------------
// Ham hook truoc toan tu 'new' Khi debug
//------------------------------------------------------------------------------
int MyAllocHook(int nAllocType, void *pUserData, size_t size, int
	nBlockType, long lRequestNumber, const unsigned char *szFilename, int
	nLineNumber)
{
	if (lRequestNumber == 3544 && size == 68){
		g_lRequestNumber = lRequestNumber;
/*		_CrtDumpMemoryLeaks();*/
	}
	return 1;
}

//------------------------------------------------------------------------------
// Bo test quet folder. Su dung ham export EmulJS
//------------------------------------------------------------------------------
int main(void)
{
	int nIsLeak;
	stringstream szResultScan;
	TCHAR wszPathFile[512];

	_CrtSetAllocHook(MyAllocHook);

	// Thu muc chua mau can test
	_stprintf_s(wszPathFile, _T(
		//"D:\\Downloads\\mauquet26_5_20\\mauvirus\\chiara2\\virus2"
		//"D:\\Downloads\\mauquet26_5_20\\mauvirus\\chiara2\\loi2"
		"C:\\Users\\sontdc\\Desktop\\virus2"
	));
	_CrtDumpMemoryLeaks();

	g_nCountFileScan = 0;
	ScanDir(wszPathFile);

	SAFE_DELETE(g_pProgramJs);
	if (_CrtDumpMemoryLeaks())
		nIsLeak = 1;

	szResultScan << "Count VR = " << g_nCountFileVirus << "/" << g_nCountFileScan;
	szResultScan << "\nTi le: " << ((float)g_nCountFileVirus) / g_nCountFileScan << "\n";
	OutputDebug(szResultScan.str().c_str());

	return true;
}

//------------------------------------------------------------------------------
// Quet 1 folder
//------------------------------------------------------------------------------
int ScanDir(TCHAR *szPath)
{

	WIN32_FIND_DATA	w32Data;
	TCHAR szFileName[MAX_PATH + 1];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	if (szPath == NULL) return 0;
	if (!PathIsDirectory(szPath)) return 0;
	if (PathIsDirectoryEmpty(szPath))
		return 0;

	_stprintf_s(szFileName, MAX_PATH, _T("%s\\*.*"), szPath);
	hFind = FindFirstFile(szFileName, &w32Data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!_tcscmp(w32Data.cFileName, _T(".")) || !_tcscmp(w32Data.cFileName, _T("..")))
				continue;
			_stprintf_s(szFileName, MAX_PATH, _T("%s\\%s"), szPath, w32Data.cFileName);

			// Neu la folder >> duyet de qui thu muc
			if ((w32Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				ScanDir(szFileName);
				continue;
			}
			else{
				// xu ly file
				OutputDebugW(szFileName);
				if (ScanFile(szFileName))
					g_nCountFileVirus++;

			}
		} while (FindNextFile(hFind, &w32Data));
	}
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
	}
	RemoveDirectory(szPath);
	return g_nCountFileVirus;
}

//------------------------------------------------------------------------------
// Quet 1 file 
//------------------------------------------------------------------------------
BOOL ScanFile(TCHAR * wszPathFile)
{
	SCANRESULT result;
	ostringstream ssMsg;
	wstring wszNameVr;
	string sNameVr;

	HANDLE hFile = NULL;
	DWORD byteHigh = 0;
	DWORD dwSizeData = 0; 
	BYTE * pBuff = NULL; 
	BOOL bRead = FALSE;

	result.bHasVirus = false;
	g_nCountFileScan++;

	//if (g_nCountFileScan > 3 && g_nCountFileScan < 5)
	{
		// Doc file va quet
		hFile = CreateFileW(wszPathFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;
		dwSizeData = GetFileSize(hFile, &byteHigh);
		pBuff = new BYTE[dwSizeData];
		bRead = ReadFile(hFile, pBuff, dwSizeData, &byteHigh, NULL);
		if (bRead == TRUE)
		{
			OutputDebugW(wszPathFile);
			ssMsg.str("");
			ssMsg << "\n" << g_nCountFileScan;
			OutputDebug(ssMsg.str().c_str());
			
			// Quet virus
			ssMsg.str("");
			if (EmulJS(&result, pBuff, dwSizeData))
			{
				wszNameVr = result.wszVirusName;
				sNameVr = string(wszNameVr.begin(), wszNameVr.end());
				ssMsg << "\nVirus: " << sNameVr;
			}
			ssMsg << "\nExit scan -------------------------------------------\n";
			OutputDebug(ssMsg.str().c_str());
		}
		CloseHandle(hFile);

	}
	SAFE_DELETE_ARRAY(pBuff);
	return result.bHasVirus;
}

#endif // !BUID_UPDATE