#include "StdAfx.h"
#include "Virustotal.h"
#include "WinhttpWrapper.h"
#include "SupportMain.h"
#include "ParseJsonVirustotal.h"

CVirustotal::CVirustotal(void)
{

}

CVirustotal::~CVirustotal(void)
{
}

CVirustotal::CVirustotal(CString sKey)
{
	this->m_sKey = sKey;
}
void CVirustotal::SetApiKey(CString sKey)
{
	this->m_sKey = sKey;
}

void CVirustotal::SetDirScan(CString sDir)
{
	this->m_sDirScan = sDir;
}

BOOL CVirustotal::WriteFileLog(CString sMd5Folder, CHAR*pszBuffer, DWORD dwSize, CString sPathFile)
{
	CString sDir;
	CString sFileLog;
	CString sFileNew;

	LPTSTR  pszFilename;
	HANDLE  hFile;
	DWORD   dwSizeOfByte = 0;

	pszFilename = PathFindFileName(sPathFile);
	sDir.Format(_T("%s\\%s"), m_sDirScan.GetBuffer(), sMd5Folder.GetBuffer());
	sFileNew.Format(_T("%s\\%s"), sDir.GetBuffer(), pszFilename);

	if (!PathIsDirectory(sDir))
	{
		if (!CreateDirectoryW(sDir, NULL))
		{
			return FALSE;
		}
	}

	sFileLog.Format(_T("%s\\%s"), sDir.GetBuffer(), NAME_LOG_VIRUS);
	hFile = CreateFile( sFileLog,
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	WriteFile(hFile, pszBuffer, strlen(pszBuffer), &dwSizeOfByte, 0);
	CloseHandle(hFile);
	MoveFileEx(sPathFile, sFileNew, MOVEFILE_REPLACE_EXISTING);

	return TRUE;
}

DWORD CVirustotal::ScanResource(CString sPathFile, CString sMd5, CString sResource)
{
	ParseJsonVirustotal Json;
	CHAR*	pszBuffer = new CHAR[MAX_SIZE_LOG];
	ZeroMemory(pszBuffer, MAX_SIZE_LOG);
	CString sReponse = _T("");

	sReponse = SendResourceAndGetReponse(sResource);
	if (sReponse == _T(""))
	{
		WriteLog(_T("File %s is null"), sPathFile);
		delete[] pszBuffer;
		return NO_SCAN_RESOURCE;
	}

	if (Json.ParseJsonToArray(sReponse, pszBuffer, MAX_SIZE_LOG) != ERROR_SUCCESS)
	{
		delete[] pszBuffer;
		return NO_SCAN_RESOURCE;
	}

	WriteFileLog(sMd5, pszBuffer, MAX_SIZE_LOG, sPathFile);
	delete[] pszBuffer;

	return ERROR_SUCCESS;
}

CString CVirustotal::SendResourceAndGetReponse(CString sResurce)
{
	CString sReponse = _T("");
	CWinhttpWrapper* pWinhttp = NULL;
	CString		sParam;
	CHAR*		pszParam = NULL;

	pWinhttp = new CWinhttpWrapper(SZ_SERVER_VIRUSTOTAL, SZ_OBJECT_SCAN_RESOURCE, 80);
	if (pWinhttp == NULL)
	{
		return _T("");
	}

	sParam.Format(_T("resource=%s&key=%s"), sResurce.GetBuffer(), m_sKey.GetBuffer());
	pszParam = UnicodeToAnsi(sParam);

	if (pszParam == NULL)
	{
		delete pWinhttp;
		delete pszParam;
		return _T("");
	}
	
	sReponse = pWinhttp->PostDataGetReponse(NULL, NULL, pszParam, strlen(pszParam));
	if (sReponse == _T(""))
	{
		WriteLog(_T("Error winhttp. Error code = %d"), GetLastError());
	}

	delete pszParam;
	delete pWinhttp;

	return sReponse;
}

CString CVirustotal::UploadFile(CString sPath)
{
	CString sScanID;
	CString sReponse = _T("");
	CHAR	szContenType[MAX_PATH*2] = {0};

	WCHAR*	pwszContentype = NULL;

	CHAR*	pszData;
	CHAR*	pszKey = NULL;
	CHAR*	pszPath = NULL;
	CHAR	szBoundary[MAX_PATH] = {0};
	CHAR	szBoundaryHeader[MAX_PATH] = {0};
	CHAR    szInforFile[MAX_PATH*2] = {0};
	CHAR	szInforApiKey[MAX_PATH*3] = {0};

	CWinhttpWrapper* pWinhttp = NULL;

	DWORD	dwSizeFile;
	DWORD   dwSizeData;
	DWORD	dwTickCount;

	DWORD	dwLengDataPart1;
	DWORD	dwLengDataPart2;

	DWORD	dwSizeOfBoundary;
	DWORD	dwSizeOfInforFile;
	DWORD	dwSizeOfInforKey;

	ParseJsonVirustotal Json;

	pWinhttp = new CWinhttpWrapper(SZ_SERVER_VIRUSTOTAL, SZ_OBJECT_UPLOAD_FILE, 80);
	if (pWinhttp == NULL) return _T("");

	dwTickCount = GetTickCount();
	dwSizeFile = g_support.GetSizeOfFile(sPath);
	//WriteLog(_T("File upload = %s. Size = %d"), sPath, dwSizeFile);
	if (dwSizeFile == 0 || dwSizeFile > MAX_SIZE_UPLOAD)
	{
		delete pWinhttp;
		return _T("");
	}

	pszKey = UnicodeToAnsi(m_sKey);
	pszPath = UnicodeToAnsi(sPath);

	sprintf_s(szBoundaryHeader, MAX_PATH, "---------------------------%d", dwTickCount);
	sprintf_s(szBoundary, MAX_PATH, "--%s",szBoundaryHeader);
	sprintf_s(szContenType, MAX_PATH*2, "Content-Type: multipart/form-data; boundary=%s", szBoundaryHeader);
	sprintf_s(szInforFile, MAX_PATH*2, "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: application/x-msdownload\r\n\r\n", pszPath);
	sprintf_s(szInforApiKey, MAX_PATH*3, "\r\nContent-Disposition: form-data; name=\"key\"\r\n\r\n%s\r\n", pszKey);

	delete pszKey;
	delete pszPath;

	dwSizeOfBoundary = strlen(szBoundary);
	dwSizeOfInforKey = strlen(szInforApiKey);
	dwSizeOfInforFile = strlen(szInforFile);

	dwSizeData = dwSizeFile + dwSizeOfInforFile + dwSizeOfInforKey + dwSizeOfBoundary*3 + 6;

	pszData = new CHAR[dwSizeData];
	ZeroMemory(pszData, dwSizeData);

	//strcpy_s(pszData, dwSizeData, "\r\n");
	strcpy_s(pszData, dwSizeData, szBoundary);
	strcat_s(pszData, dwSizeData, szInforFile);

	dwLengDataPart1 = strlen(pszData);

	if (!g_support.ReadFileToBuffer(sPath, pszData + dwLengDataPart1, dwSizeFile))
	{
		delete pWinhttp;
		delete[]pszData;
		return _T("");
	}

	dwLengDataPart2 = dwLengDataPart1 + dwSizeFile;
	CopyMemory(pszData + dwLengDataPart2, "\r\n", 2);
	CopyMemory(pszData + dwLengDataPart2 + 2, szBoundary, dwSizeOfBoundary);
	CopyMemory(pszData + dwLengDataPart2 + 2 + dwSizeOfBoundary, szInforApiKey, dwSizeOfInforKey);
	CopyMemory(pszData + dwLengDataPart2 + 2 + dwSizeOfBoundary+ dwSizeOfInforKey, szBoundary, dwSizeOfBoundary);
	CopyMemory(pszData + dwLengDataPart2 + 2 + dwSizeOfBoundary+ dwSizeOfInforKey+ dwSizeOfBoundary, "--\r\n", 4);

	pwszContentype = AnsiToUnicode(szContenType);

	sReponse = pWinhttp->PostDataGetReponse(pwszContentype, NULL, pszData, dwSizeData);
	if (sReponse == _T(""))
	{
		WriteLog(_T("Error winhttp. Error code = %d"), GetLastError());
	}

	delete pwszContentype;
	delete pWinhttp;
	delete[] pszData;

	sScanID = Json.GetSanId(sReponse);

	return sScanID;
}
