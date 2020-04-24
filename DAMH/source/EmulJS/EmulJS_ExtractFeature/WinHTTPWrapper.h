#pragma once

//==========================================================================
//============== define status for downloading
//==========================================================================
#define WinHTTP_Download_Status_Finish				0
#define WinHTTP_Download_Status_Check_Connect		1
#define WinHTTP_Download_Status_Parser_Link			2
#define WinHTTP_Download_Status_Connect_Server		3
#define WinHTTP_Download_Status_Open_Request		4
#define WinHTTP_Download_Status_Send_Request		5
#define WinHTTP_Download_Status_Receive				6
#define WinHTTP_Download_Status_Not_Enough_Buffer	7
#define WinHTTP_Download_Status_Get_Data			8

#define WinHTTP_Download_Status_Cancel_By_User		500

#define WinHTTP_Download_Status_Construct			999
#define WinHTTP_Download_Status_Error				1000
#define WinHTTP_Download_Status_Init				1001

//==========================================================================
//================= define status for type downloading
//==========================================================================

#define WinHTTP_Download_Type_Default				0		// try all
#define WinHTTP_Download_Type_Direct_Connect		1
#define WinHTTP_Download_Type_By_Default_Proxy		2
#define WinHTTP_Download_Type_By_IE_Proxy			3
#define WinHTTP_Download_Type_By_User_Config_Proxy	4

#define WinHTTP_MAX_RETRY_DOWNLOAD					20
#define WinHTTP_MAX_RETRY_QueryDataAvailable		3

#define WinHTTP_Retry_Download_Interval				20000
#define WinHTTP_Retry_QueryDataAvailable_Interval	3000

typedef struct __tag_DownloadContext
{
	BOOL bRetring;
	DWORD dwDownloadSizeOriginal;
	DWORD dwCurrentDownloadedOriginal;
} TDownloadContext, *PDownloadContext;

typedef struct __tag_ProxyInfo
{
	WCHAR szProxyServer[MAX_PATH];
	WCHAR szProxyUser[MAX_PATH];
	WCHAR szProxyPass[MAX_PATH];
	DWORD nPort;
} TProxyInfo, *PProxyInfo;

extern TProxyInfo g_UserConfigProxy;

class CWinHTTPWrapper
{
public:
	CWinHTTPWrapper(void);
	~CWinHTTPWrapper(void);
protected:
	PVOID m_pIEProxyConfig;
	DWORD m_dwDownloadStatus;
	DWORD m_dwDownloadType;
	int m_iResolveTimeout;
	int m_iConnectTimeout;
	int m_iSendTimeout;
	int m_iReceiveTimeout;
	// for export, some struct -> void, skip build conflict
	void CrackServerAndObjectName(LPWSTR szServerName, LPWSTR szObjectName, void * urlComponent);	//	URL_COMPONENTS * urlComponent);
	HINTERNET WrapperOpen(WCHAR* szAgent, DWORD dwDownloadType, LPCWSTR szPathUrl = NULL);
	BOOL SafeDownloadFile(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR * szPathSaveFile, HWND hwndProgress, DWORD dwRangeLow, DWORD dwRangeHigh, DWORD* pdwCancel, DWORD dwDownloadType, LPCWSTR szProxyUser, LPCWSTR szProxyPass, int * lpDownloadPercent = NULL, PDownloadContext pCtx = NULL);
	BOOL GetFileDownloadSizeEx(WCHAR* szAgent, WCHAR* szPathUrl, DWORD& dwSize, DWORD dwDownloadType, LPCWSTR szProxyUser, LPCWSTR szProxyPass);

public:
	//////////////////////////////////////////////////////////////////////////
	void SetProxyInfo(PProxyInfo pUserConfigProxy);
	void SetTimeouts(int iResolveTimeout, int iConnectTimeout, int iSendTimeout, int iReceiveTimeout);
	DWORD GetIEProxyConfig();
	// common download file
	BOOL DownloadFile(WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress , DWORD dwDownloadType , WCHAR * szAgent, int * lpDownloadPercent);
	BOOL DownloadFile(WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress , DWORD dwDownloadType , WCHAR * szAgent, DWORD dwRangeLow, DWORD dwRangeHigh, int * lpDownloadPercent);
	BOOL DownloadFile(WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress , DWORD dwDownloadType , WCHAR * szAgent, DWORD* pdwCancel, int * lpDownloadPercent);

	// normal download file
	BOOL DirectDownloadFile(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR * szPathSaveFile, HWND hwndProgress, int * lpDownloadPercent);
	BOOL DownloadFileByDefaultProxy(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress, int * lpDownloadPercent);
	BOOL DownloadFileByNamedProxy(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress, int * lpDownloadPercent);
	
	// custom download file
	BOOL DirectDownloadFile(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR * szPathSaveFile, HWND hwndProgress, DWORD dwRangeLow, DWORD dwRangeHigh, int * lpDownloadPercent);
	BOOL DownloadFileByDefaultProxy(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress, DWORD dwRangeLow, DWORD dwRangeHigh, int * lpDownloadPercent);
	BOOL DownloadFileByNamedProxy(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress, DWORD dwRangeLow, DWORD dwRangeHigh, int * lpDownloadPercent);

	// download file with check condition
	BOOL DirectDownloadFile(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR * szPathSaveFile, HWND hwndProgress, DWORD* pdwCancel, int * lpDownloadPercent);
	BOOL DownloadFileByDefaultProxy(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress, DWORD* pdwCancel, int * lpDownloadPercent);
	BOOL DownloadFileByNamedProxy(WCHAR* szAgent, WCHAR* szPathUrl, WCHAR* szPathSaveFile, HWND hwndProgress, DWORD* pdwCancel, int * lpDownloadPercent);

	// get file download size
	BOOL GetFileDownloadSizeByDirectConnect(WCHAR* szAgent, WCHAR* szPathUrl, DWORD& dwSize);
	BOOL GetFileDownloadSizeByDefaultProxy(WCHAR* szAgent, WCHAR* szPathUrl, DWORD& dwSize);
	BOOL GetFileDownloadSizeByIeProxy(WCHAR* szAgent, WCHAR* szPathUrl, DWORD& dwSize);
	// common get file download size
	BOOL GetFileDownloadSize(WCHAR* szAgent, WCHAR* szPathUrl, DWORD& dwSize, BOOL bUseProxy = FALSE);

	// DownloadTextFromUrl
	LRESULT DownloadTextFromUrl(LPCWSTR szURL, LPSTR szOutText, DWORD dwMaxLen, char cDelimiter, BOOL bUseProxy = FALSE);
	LRESULT DownloadTextFromUrlByDirectConnect(LPCWSTR szURL, LPSTR szOutText, DWORD dwMaxLen, char cDelimiter);
	LRESULT DownloadTextFromUrlByDefaultProxy(LPCWSTR szURL, LPSTR szOutText, DWORD dwMaxLen, char cDelimiter);
	LRESULT DownloadTextFromUrlByNamedProxy(LPCWSTR szURL, LPSTR szOutText, DWORD dwMaxLen, char cDelimiter);
	LRESULT DownloadTextFromUrlEx(LPCWSTR szPathUrl, LPSTR szOutText, DWORD dwMaxLen, char cDelimiter, DWORD dwDownloadType, LPCWSTR szProxyUser, LPCWSTR szProxyPass);
	
	//function util
	DWORD GetDownloadType();
	DWORD GetDownloadStatus();
	void DisplayProgess(HWND hwndProgress,int dwTotalReadSize, int dwTotalSize);
	BOOL m_bCheckCreateFile;
};