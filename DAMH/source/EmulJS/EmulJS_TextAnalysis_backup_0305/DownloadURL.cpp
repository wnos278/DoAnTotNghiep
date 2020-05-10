
#include "resource.h"
#include "DownloadURL.h"
#include <commctrl.h>
#include <stdio.h>
#include <malloc.h>
//#include "..\BkavEngine\CloudEngine.h"

#include "WinHTTPWrapper.h"
#pragma comment(lib, "Winhttp_VC9.lib")
#pragma comment(lib, "Wininet.lib")
#include <Tlhelp32.h>
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

// #ifdef _DEBUG
// #ifndef DBG_NEW
// #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// #define new DBG_NEW
// #endif
// #endif  // _DEBUG

//#define LOG_Query_Cloud


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ParseURL(LPCTSTR szURL, LPTSTR szServerName, LPTSTR szObjectName)
{
	TCHAR *pos1, *pos2;
	int iLen;

	pos1 = (TCHAR *)_tcsstr(szURL, _T("//"));
	if (pos1 == NULL)
		pos1 = (LPTSTR)szURL;
	else
	{
		pos1 += _tcslen(_T("//"));
	}

	pos2 = _tcschr(pos1, _T('/'));
	if (pos2 == NULL)
	{
		return FALSE;
	}
	int a = _tcslen(pos2);
	_tcscpy_s(szObjectName, MAX_PATH, pos2);
	iLen = pos2 - pos1;
	_tcsncpy_s(szServerName, MAX_PATH, pos1, iLen);
	szServerName[iLen] = 0;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeProxyName(LPCTSTR szProxyServer, int iPort, LPTSTR szProxyName)
{
	wsprintf(szProxyName, _T("http=http://%s:%d"), szProxyServer, iPort);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetTempDownloadFilePath(LPTSTR pszFilePath, int cchMaxPath)
{
	TCHAR	szTempDir[MAX_PATH];

	if (pszFilePath == NULL || cchMaxPath < MAX_PATH) return 0;

	if (!GetTempPath(MAX_PATH, szTempDir)) return 0;

	if (!GetTempFileName(szTempDir, _T("_dl"), 0, pszFilePath)) return 0;

	return _tcslen(pszFilePath);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT DownloadURL(LPCTSTR szURL, LPCTSTR szFileName, HWND wndParent, LPCTSTR szProxyName,
	LPCTSTR szProxyByPass, LPCTSTR szUsername, LPCTSTR szPassword, DWORD * bCancel)
{
	const DWORD dwBufSize = 100960;
	LPBYTE buffer;
	DWORD dwTotalSize, dwTotalReadSize;
	DWORD dwWritten, dwReadSize;
	HINTERNET hInternet = NULL, hConnect = NULL, hRequest = NULL;
	TCHAR szServerName[MAX_PATH];
	TCHAR szObjectName[MAX_PATH];
	TCHAR szTempFilePath[MAX_PATH];
	DWORD dwQuerySize;
	DWORD dwQueryValue, dwReturn;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LRESULT result = URL_ERR_SUCCESS;

	LPCTSTR szAcceptType[2];
	szAcceptType[0] = _T("*/*");
	szAcceptType[1] = NULL;

	if (dwBufSize == 0) return URL_ERR_INVALID_BUFFER;

	// Analyze URL
	if (!ParseURL(szURL, szServerName, szObjectName))
	{
		return URL_ERR_INVALID_URL;
	}

	// ============================================================
	// Initialize internet connection
	if (szProxyName == NULL)
		hInternet = InternetOpen(_T("DownloadURL/1.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	else
		hInternet = InternetOpen(_T("DownloadURL/1.0"), INTERNET_OPEN_TYPE_PROXY, szProxyName, szProxyByPass, 0);

	if (hInternet == NULL)	return URL_ERR_INTERNET_OPEN_FAILED;

	// ============================================================
	// Try to connect
	hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT, szUsername, szPassword,
		INTERNET_SERVICE_HTTP, 0, NULL);

	if (hConnect == NULL)
	{
		return URL_ERR_INTERNET_CONNECT_FAILED;
	}

	// ============================================================
	// InternetConnect successful
	hRequest = HttpOpenRequest(hConnect, _T("GET"), szObjectName, NULL, NULL, szAcceptType,
		INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_KEEP_CONNECTION, 0);

	//hRequest =  HttpOpenRequest(hConnect, _T("GET"), szObjectName, NULL, NULL, szAcceptType, 0, 0);
	if (hRequest == NULL)
	{
		// this method cannot work, return ERROR
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return URL_ERR_OPEN_REQUEST_FAILED;
	}

	// ============================================================
	// Try sending request	
	if (!HttpSendRequest(hRequest, NULL, 0, NULL, 0))
	{
		// Do our cleanup jop and exit
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return URL_ERR_SEND_REQUEST_FAILED;
	}


	// Request sent successfully

	// ============================================================
	// Query status code
	dwQuerySize = sizeof(dwQueryValue);
	dwQueryValue = 0;
	if (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwQueryValue, &dwQuerySize, 0))
	{
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return URL_ERR_STATUS_CODE_FAILED;
	}

	dwReturn = 0;

	switch (dwQueryValue)
	{
	case HTTP_STATUS_USE_PROXY:
		dwReturn = URL_ERR_PROXY_REQ;
		break;
	case HTTP_STATUS_DENIED:
		dwReturn = URL_ERR_AUTH_REQ;
		break;
	case HTTP_STATUS_PROXY_AUTH_REQ:
		dwReturn = URL_ERR_PROXY_AUTH_REQ;
		break;
	case HTTP_STATUS_NOT_FOUND:
		dwReturn = URL_ERR_INVALID_URL;
		break;
	case HTTP_STATUS_REQUEST_TIMEOUT:
		dwReturn = URL_ERR_REQUEST_TIMEOUT;
		break;
	case HTTP_STATUS_SERVER_ERROR:
		dwReturn = URL_ERR_SERVER_ERROR;
		break;
	case HTTP_STATUS_SERVICE_UNAVAIL:
		dwReturn = URL_ERR_SERVICE_UNAVAIL;
		break;
	}

	if (dwReturn != 0)
	{
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return dwReturn;
	}

	// ============================================================
	// Query file size
	dwQuerySize = sizeof(dwTotalSize);
	dwTotalSize = 0;
	if (!HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwTotalSize, &dwQuerySize, 0))
	{
		// Do our cleanup jop and exit
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return URL_ERR_UNKONWN_FILE_SIZE;
	}


	// ============================================================
	// Create file for storing downloaded data

	// Create temporary file for download
	if (!GetTempDownloadFilePath(szTempFilePath, MAX_PATH)) return URL_ERR_CREATE_FILE_FAILED;

	hFile = CreateFile(szTempFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// Cannot create file, do our cleanup jop and exit
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return URL_ERR_CREATE_FILE_FAILED;
	}

	// ============================================================
	// Read source file from URL and save to disk

	// Memory allocation
	buffer = (LPBYTE)malloc(dwBufSize);
	if (buffer == NULL)
	{
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		CloseHandle(hFile);
		DeleteFile(szTempFilePath);
		return URL_ERR_INVALID_BUFFER;
	}

	dwTotalReadSize = 0;		// Total number of bytes downloaded
	do
	{
		// Try to read file
		if (InternetReadFile(hRequest, buffer, dwBufSize, &dwReadSize))
		{
			if (bCancel && *bCancel > 0) // Neu nguoi dung bam nut cancel thi thoi
			{
				break;
			}
			// Reading successful
			dwTotalReadSize += dwReadSize;

			// Save data to file
			WriteFile(hFile, buffer, dwReadSize, &dwWritten, NULL);
		}
		else
			break;
	} while (dwReadSize);
	// Save file and close handle
	CloseHandle(hFile);
	free(buffer);

	// Try to move temporary file to destination file
	if (!CopyFile(szTempFilePath, szFileName, FALSE))
	{
		result = URL_ERR_CREATE_FILE_FAILED;
	}
	DeleteFile(szTempFilePath);

	// ============================================================
	// Clean up	
	if (hRequest != NULL) InternetCloseHandle(hRequest);
	if (hConnect != NULL) InternetCloseHandle(hConnect);
	if (hInternet != NULL) InternetCloseHandle(hInternet);

	if (result != URL_ERR_SUCCESS) return result;

	if (dwTotalReadSize < dwTotalSize) return URL_ERR_DATA_NOT_SUFFICIENT;
	return URL_ERR_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------------------------------------------
// AnsiToUnicode
// Info:	Convert ANSI string to Unicode string	
// Param:	
//			(in) pszAnsi: source ANSI string
//			(out) pszUnicode: destination Unicode string
//			cchUnicode: size in WCHAR of destination string, including a terminating null character
// Return:	size of destination string after conversion
// ------------------------------------------------------------------------------------------------------------
int AnsiToUnicode(LPCSTR pszAnsi, LPWSTR pszUnicode, size_t cchUnicode)
{
	if (pszAnsi == NULL || pszUnicode == NULL || cchUnicode <= 1) return 0;

	return MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, pszUnicode, cchUnicode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT DownloadTextFromUrl_Old(LPCTSTR szURL, LPTSTR szOutText, DWORD dwMaxLen, TCHAR cDelimiter, LPCTSTR szProxyName,
	LPCTSTR szProxyByPass, LPCTSTR szUsername, LPCTSTR szPassword)
{
	DWORD dwTotalRead, dwReaded, dwRemain;
	DWORD i, dwOldSize;
	HINTERNET hInternet = NULL, hConnect = NULL, hRequest = NULL;
	TCHAR szServerName[MAX_PATH];
	TCHAR szObjectName[MAX_PATH];
	DWORD dwQuerySize;
	DWORD dwQueryValue, dwReturn;
	LPCTSTR szAcceptType[2];
	szAcceptType[0] = _T("*/*");
	szAcceptType[1] = NULL;


	// Analyze URL
	if (!ParseURL(szURL, szServerName, szObjectName))
	{
		return 0;
	}

	// ============================================================
	// Initialize internet connection
	if (szProxyName == NULL)
		hInternet = InternetOpen(_T("DownloadURL/1.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	else
		hInternet = InternetOpen(_T("DownloadURL/1.0"), INTERNET_OPEN_TYPE_PROXY, szProxyName, szProxyByPass, 0);

	if (hInternet == NULL)	return 0;

	// ============================================================
	// Try to connect
	if (szProxyName == NULL)
		hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT, szUsername, szPassword,
		INTERNET_SERVICE_HTTP, 0, NULL);
	else
		hConnect = InternetConnect(hInternet, szServerName, 8080, szUsername, szPassword,
		INTERNET_SERVICE_HTTP, 0, NULL);

	if (hConnect == NULL)
	{
		return 0;
	}

	// ============================================================
	// InternetConnect successful
	hRequest = HttpOpenRequest(hConnect, _T("GET"), szObjectName, NULL, NULL, szAcceptType,
		INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION, 0);

	//hRequest =  HttpOpenRequest(hConnect, _T("GET"), szObjectName, NULL, NULL, szAcceptType, 0, 0);
	if (hRequest == NULL)
	{
		// this method cannot work, return ERROR
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return 0;
	}

	// ============================================================
	// Try sending request	
	if (!HttpSendRequest(hRequest, NULL, 0, NULL, 0))
	{
		int error = GetLastError();
		// Do our cleanup jop and exit
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return 0;
	}

	// Request sent successfully

	// ============================================================
	// Query status code
	dwQuerySize = sizeof(dwQueryValue);
	dwQueryValue = 0;
	if (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwQueryValue, &dwQuerySize, 0))
	{
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return 0;
	}

	dwReturn = 0;

	switch (dwQueryValue)
	{
	case HTTP_STATUS_USE_PROXY:
		dwReturn = URL_ERR_PROXY_REQ;
		break;
	case HTTP_STATUS_DENIED:
		dwReturn = URL_ERR_AUTH_REQ;
		break;
	case HTTP_STATUS_PROXY_AUTH_REQ:
		dwReturn = URL_ERR_PROXY_AUTH_REQ;
		break;
	case HTTP_STATUS_NOT_FOUND:
		dwReturn = URL_ERR_INVALID_URL;
		break;
	case HTTP_STATUS_REQUEST_TIMEOUT:
		dwReturn = URL_ERR_REQUEST_TIMEOUT;
		break;
	case HTTP_STATUS_SERVER_ERROR:
		dwReturn = URL_ERR_SERVER_ERROR;
		break;
	case HTTP_STATUS_SERVICE_UNAVAIL:
		dwReturn = URL_ERR_SERVICE_UNAVAIL;
		break;
	}

	if (dwReturn != 0)
	{
		if (hRequest != NULL) InternetCloseHandle(hRequest);
		if (hConnect != NULL) InternetCloseHandle(hConnect);
		if (hInternet != NULL) InternetCloseHandle(hInternet);
		return 0;
	}

	// ============================================================
	// Read source file from URL 

	dwTotalRead = 0;		// Total number of bytes downloaded
	dwRemain = dwMaxLen - 1;
	do
	{
		// Try to read file
		if (InternetReadFile(hRequest, szOutText + dwTotalRead, dwRemain, &dwReaded))
		{
			// Reading successful
			dwOldSize = dwTotalRead;
			dwTotalRead += dwReaded;
			dwRemain -= dwReaded;
			for (i = dwOldSize; i < dwTotalRead; i++)
			{
				if (szOutText[i] == cDelimiter)
				{
					dwTotalRead = i + 1;
					break;
				}
			}
			if (i < dwTotalRead)
			{
				break;
			}
		}
		else
			break;
	} while ((dwReaded > 0) && (dwRemain > 0));
	szOutText[dwTotalRead] = 0;

	// ============================================================
	// Clean up	
	if (hRequest != NULL) InternetCloseHandle(hRequest);
	if (hConnect != NULL) InternetCloseHandle(hConnect);
	if (hInternet != NULL) InternetCloseHandle(hInternet);

	return _tcslen(szOutText);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// use WinHTTP if fail
int AnsiToUnicode(LPCSTR pszAnsi, LPWSTR pszUnicode, size_t cchUnicode);

void SetProxyInfoForWinHTTP(LPCSTR szProxyServer, LPCSTR szProxyUserName, LPCSTR szProxyPassword, DWORD nProxyPort)
{
	memset(&g_UserConfigProxy, 0, sizeof(TProxyInfo));

	g_UserConfigProxy.nPort = nProxyPort;

	AnsiToUnicode(szProxyServer, g_UserConfigProxy.szProxyServer, MAX_PATH);
	AnsiToUnicode(szProxyUserName, g_UserConfigProxy.szProxyUser, MAX_PATH);
	AnsiToUnicode(szProxyPassword, g_UserConfigProxy.szProxyPass, MAX_PATH);
}

////////////////////////////////////////////////////////////////////// 
//
HANDLE GetUserToken();

void SetImpersonatePrivilege()
{
	static BOOL bHasPrivilege = FALSE;
	if (bHasPrivilege) return;

	HANDLE hProcSelf = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	HANDLE hTokenSelf;

	if (!OpenProcessToken(hProcSelf, TOKEN_ADJUST_PRIVILEGES, &hTokenSelf))
		return;

	LUID luid;
	LookupPrivilegeValue(NULL, SE_IMPERSONATE_NAME, &luid);
	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hTokenSelf, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

	CloseHandle(hTokenSelf);
	bHasPrivilege = TRUE;
}

BOOL ImpersonateExplorerUser()
{
	SetImpersonatePrivilege();

	HANDLE hExplorerToken = GetUserToken();
	if (hExplorerToken == NULL)
		return FALSE;

	if (!ImpersonateLoggedOnUser(hExplorerToken))
	{
#ifdef LOG_GET_USER_TOKEN
		GhiLogEx("ImpersonateLoggedOnUser return FALSE");
#endif
		return FALSE;
	}

#ifdef LOG_GET_USER_TOKEN
	GhiLogEx("ImpersonateLoggedOnUser return TRUE");
#endif

	return TRUE;
}

void MyRevertToSelf()
{
	RevertToSelf();
}

HANDLE GetExplorerProcessHandle();

// Get the logged-in user token

HANDLE GetUserToken()
{
	// Get the process handle for Explorer.EXE
	static HANDLE hToken = NULL;

	// 	if(hToken != NULL)
	// 		return hToken;

	HANDLE hProcess = GetExplorerProcessHandle();

	if (hProcess == NULL)
		return NULL;

	// Open the process token, and return the value
	// TOKEN_IMPERSONATE
	BOOL result = OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken);

	CloseHandle(hProcess);

	if (result)
	{
#ifdef LOG_GET_USER_TOKEN
		GhiLogEx("GetUserToken() success, found Explorer.exe token");
#endif
		return hToken;
	}
	else
	{
#ifdef LOG_GET_USER_TOKEN
		GhiLogEx("GetUserToken() failed");
#endif
		hToken = NULL;
		return NULL;
	}
}

// Get the process handle of Explorer.EXE
HANDLE GetExplorerProcessHandle()
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	CHAR szProcessName[512];

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
#ifdef LOG_GET_USER_TOKEN
		GhiLogEx("GetExplorerProcessHandle failed, EnumProcesses failed");
#endif
		return 0;
	}

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	// Find which one is the explorer process
	for (i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0) {
			// Open the Process
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcesses[i]);
			// Get the process name.
			if (NULL != hProcess)
			{
				HMODULE hMod;
				DWORD cbNeeded;
				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
					GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
			}
			// See if it is explorer
			if (!_stricmp(szProcessName, "Explorer.exe"))
			{
#ifdef LOG_GET_USER_TOKEN
				GhiLogEx("GetExplorerProcessHandle success, found Explorer.exe process");
#endif
				return(hProcess);
			}
			CloseHandle(hProcess);
		}
	}

	return NULL;
}

BOOL GetIEProxyConfigForUser(CWinHTTPWrapper *pWinHTTP)
{
	BOOL bRet = ImpersonateExplorerUser();
	if (bRet)
	{
		if (pWinHTTP->GetIEProxyConfig())
			bRet = TRUE;

		RevertToSelf();
		return bRet;
	}

	return FALSE;
}

 LRESULT DownloadTextFromUrl(LPCTSTR szURL, LPSTR szOutText, DWORD dwMaxLen, char cDelimiter, LPCTSTR szProxyName,
 	LPCTSTR szProxyByPass, LPCTSTR szUsername, LPCTSTR szPassword)
{
	LRESULT lResult = 0;

#ifdef LOG_DownloadTextFromUrl
	GhiLogEx("DownloadTextFromUrl, use WinHTTP first, szURL=%s", szURL);
#endif
	// Fix for Zamil
	// Add new DownloadTextFromUrl use WinHTTP
	CWinHTTPWrapper http;
	WCHAR w_szURL[1024];

	http.SetProxyInfo(&g_UserConfigProxy);

#ifndef _UNICODE
	AnsiToUnicode(szURL, w_szURL, strlen(szURL) + 2);
#else
	wcsncpy_s(w_szURL,MAX_PATH, szURL, 1024);
#endif

	GetIEProxyConfigForUser(&http);

#ifdef CLOUD_SCAN_FOR_VIRUS_TOTAL
	http.SetTimeouts(5000, 5000, 5000, 5000);
#endif

	if (szProxyName)
		lResult = http.DownloadTextFromUrl(w_szURL, szOutText, dwMaxLen, cDelimiter, TRUE);

	if (lResult == 0)
		lResult = http.DownloadTextFromUrl(w_szURL, szOutText, dwMaxLen, cDelimiter, FALSE);

	// fail -> try WinINet
	/*if (lResult == 0)
	lResult = DownloadTextFromUrl_Old(szURL, szOutText, dwMaxLen, cDelimiter, szProxyName, szProxyByPass, szUsername, szPassword);*/

#ifdef LOG_DownloadTextFromUrl
	if (lResult)
		GhiLogEx("DownloadTextFromUrl, szOutText=%s", szOutText);
	else
		GhiLogEx("DownloadTextFromUrl failed, szOutText=%s", szURL);
#endif

	return lResult;
}


