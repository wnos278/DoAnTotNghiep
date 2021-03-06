#include "StdAfx.h"
#include "SupportMain.h"
#include "IniFile.h"
#include <ShlObj.h>

// Contructor
CSupportMain::CSupportMain(void)
{
}

// Destructor
CSupportMain::~CSupportMain(void)
{
}

BOOL CSupportMain::InitAll()
{
	TCHAR szLogPath[MAX_PATH + 1];

	if (!InitAppDir())
		return FALSE;

	if (!MakeInfoFile())
		return FALSE;

	if (!LoadInfoFile())
		return FALSE;

	// Set path file log and init all
	swprintf_s(szLogPath, MAX_PATH, _T("%s\\%s"), m_sAppDir.GetBuffer(), NAME_FILE_LOG);
	if (!m_writeLog.InitAll(szLogPath))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CSupportMain::InitAppDir()
{
	TCHAR szPath[MAX_PATH] = {0};

	if (!GetCurrentDirectory(MAX_PATH, szPath))
		return FALSE;

	m_sAppDir.Format(_T("%s"), szPath);

	return TRUE;
}

BOOL CSupportMain::MakeInfoFile()
{
	CString		sFullPath;
	CIniFile	iniFileInfo;

	// Create full path of file
	sFullPath.Format(_T("%s\\%s"), m_sAppDir.GetBuffer(), NAME_FILE_INFO);
	if (PathFileExists(sFullPath))
		return TRUE;

	// Create file information for Service
	iniFileInfo.SetFileName(sFullPath);

	// Init all keys


	if (!iniFileInfo.WriteString(SECTION_INFO, KEY_API_KEY, MY_APIKEY))
		return FALSE;

	return TRUE;
}

BOOL CSupportMain::LoadInfoFile()
{
	CString		sValue;
	CString		sFullPath;
	CIniFile	iniFileInfo;

	// Set path of file information of service
	sFullPath.Format(_T("%s\\%s"), m_sAppDir.GetBuffer(), NAME_FILE_INFO);

	// Read all info
	iniFileInfo.SetFileName(sFullPath);

	if (!iniFileInfo.GetString(SECTION_INFO, KEY_API_KEY, m_sMyApiKey, MY_APIKEY))
		return FALSE;


	return TRUE;
}

//-------------------------------------------------
//Name:			CSupportMain::GetSizeOfFile
//Description:	Get size of file
//Param:		sPath - path of file
//Return:		DWORD - size of file
DWORD CSupportMain::GetSizeOfFile(CString sPath)
{
	HANDLE	hFile;
	DWORD	dwSizeLow  = 0;
	DWORD	dwSizeHigh = 0;

	hFile = CreateFile( sPath,
						GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		WriteLog(_T("Can not open file %s. Erorr code = %d"), sPath, GetLastError());
		SAFE_CLOSE(hFile);
		return 0;
	}

	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);

	if (dwSizeLow == INVALID_FILE_SIZE)
	{
		WriteLog(_T("Get file size file %s. Error code = %d"), sPath, GetLastError());
		SAFE_CLOSE(hFile);
		return 0;
	}

	SAFE_CLOSE(hFile);

	return dwSizeLow;
}

BOOL CSupportMain::ReadFileToBuffer(CString sPath, CHAR* pszBuffer, DWORD dwSizeFile)
{
	HANDLE	hFile;
	DWORD   dwByteOfRead = 0;
	BOOL    bRead;

	hFile = CreateFile( sPath,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		WriteLog(_T("Can not open file %s. Erorr code = %d"), sPath, GetLastError());
		SAFE_CLOSE(hFile);
		return 0;
	}

	bRead = ReadFile(hFile, (LPVOID)pszBuffer, dwSizeFile , &dwByteOfRead, 0);
	if (bRead == FALSE || dwByteOfRead == 0)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);

	return TRUE;
}

void WriteLog(__in LPCTSTR pszFormat, ...)
{
/*
#ifndef F_WRITE_LOG
	return;
#endif*/

	if (pszFormat == NULL)
		return;

	va_list	va;
	TCHAR	szMsg[MAX_LOG_LEN + 10];

	// Format data input
	va_start(va, pszFormat);
	vswprintf_s(szMsg, MAX_LOG_LEN, pszFormat, va);

#ifdef F_WRITE_LOG_DBG_PRINT
	OutputDebugStringW(szMsg);
	return;
#endif

	g_support.m_writeLog.WriteLog(szMsg);
}

//--------------------------------------------------------------------------------------
// Name:		UnicodeToAnsi
// Description: Application entry point
// Param:		szUnicode: input unicode string
// Return:		pointer to output asc string (USER MUST MANUALLY FREE THIS BUFFER)	
//--------------------------------------------------------------------------------------
CHAR* UnicodeToAnsi(const WCHAR* szUnicode)
{
	CHAR*	szAnsi = NULL;
	int		iSize;

	iSize = WideCharToMultiByte(CP_ACP, NULL, szUnicode, -1, NULL, 0, NULL, NULL);
	if (iSize == 0) return NULL;

	iSize++;
	szAnsi = (CHAR*) malloc(iSize * sizeof(CHAR));
	if (szAnsi == NULL)
		return NULL;

	iSize = WideCharToMultiByte(CP_ACP, NULL, szUnicode, -1, szAnsi, iSize, NULL, NULL);
	if (!iSize)
	{
		free(szAnsi);
		return NULL;
	}

	szAnsi[iSize] = 0;
	return szAnsi;
}

//---------------------------------------------------------------------------------
// Name:		AnsiToUnicode
// Description: Convert Ascii string to Unicode string
// Param:		szUnicode: input unicode string
// Return:		pointer to output asc string (USER MUST MANUALLY FREE THIS BUFFER)		
//---------------------------------------------------------------------------------
WCHAR* AnsiToUnicode(const CHAR* szAnsi)
{
	WCHAR* szUnicode = NULL;
	int iSize;

	iSize = MultiByteToWideChar(CP_ACP, NULL, szAnsi, -1, NULL, 0);
	if (iSize == 0) return NULL;

	iSize++;
	szUnicode = (WCHAR*) malloc(iSize * sizeof(WCHAR) + 5);
	if (szUnicode == NULL)
		return NULL;

	iSize = MultiByteToWideChar(CP_ACP, NULL, szAnsi, -1, szUnicode, iSize);
	if (!iSize)
	{
		free(szUnicode);
		return NULL;
	}

	szUnicode[iSize] = 0;
	return szUnicode;
}

CString	MakePath(LPCTSTR pszDir, LPCTSTR pszFileName)
{
	CString	strFullPath;
	size_t	iLen;

	// Validate input
	if (pszDir == NULL || pszFileName == NULL) return _T("");

	// Check directory name
	iLen = 0;
	iLen = _tcslen(pszDir);
	if (iLen <= 0 || iLen >= MAX_PATH_EX - 1) return _T("");
	strFullPath = pszDir;
	if (strFullPath[iLen - 1] != '\\') strFullPath += _T("\\");

	// Check file name
	iLen = 0;
	iLen = _tcslen(pszFileName);
	if (iLen <= 0 || iLen >= MAX_PATH_EX - 1) return _T("");

	// Now make path
	strFullPath += pszFileName;	

	return strFullPath;
}

// ======================================================================================
// All globals variables
// ======================================================================================
CSupportMain g_support;


