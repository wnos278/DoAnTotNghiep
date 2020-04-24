#ifndef DOWNLOAD_URL
#define DOWNLOAD_URL

#include <windows.h>
#include <wininet.h>
#include <tchar.h>


typedef LRESULT(CALLBACK *ON_PROGRESS)(DWORD dwReaded, DWORD dwTotal);

enum URL_ERROR_CODE
{
	URL_ERR_SUCCESS = 0,							// Success
	URL_ERR_INVALID_BUFFER,							// Buffer not available, or buffer size is invalid
	URL_ERR_INTERNET_OPEN_FAILED,					// InternetOpen failed
	URL_ERR_INTERNET_CONNECT_FAILED,				// InternetConnect failed
	URL_ERR_INVALID_URL,							// Invalid URL (or URL not found)
	URL_ERR_OPEN_REQUEST_FAILED,					// OpenRequest failed
	URL_ERR_CREATE_FILE_FAILED,						// Cannot create file
	URL_ERR_DATA_NOT_SUFFICIENT,					// Cannot donwload 100% of file
	URL_ERR_UNKONWN_FILE_SIZE,						// Cannot get file size
	URL_ERR_SEND_REQUEST_FAILED,					// Cannot send request
	URL_ERR_STATUS_CODE_FAILED,						// Cannot get status code
	URL_ERR_AUTH_REQ,								// Authentication required
	URL_ERR_PROXY_AUTH_REQ,							// Proxy authentication required
	URL_ERR_REQUEST_TIMEOUT,						// Request timed out
	URL_ERR_SERVER_ERROR,							// The server encountered an unexpected condition
	URL_ERR_SERVICE_UNAVAIL,						// Service unavailable
	URL_ERR_PROXY_REQ,								// Proxy required
};

#define WINHTTP_TIMEOUT	2000 // 2s

LRESULT DownloadURL(
	LPCTSTR szURL,					// URL to download
	LPCTSTR szFileName,				// Path for saving downloaded file
	HWND wndParent,					// HWND Parent
	LPCTSTR szProxyName = NULL,		// Proxy name, optional
	LPCTSTR szProxyByPass = NULL,	// Proxy by pass, optional 
	LPCTSTR szUser = NULL,			// User name, optional
	LPCTSTR szPassword = NULL,		// Password, optional
	DWORD*	bCancel = 0				// Nguoi dung bam Cancel
	);

LRESULT DownloadTextFromUrl(
	LPCTSTR szURL,					// URL to download
	LPSTR szOutText,				// pointer to output buffer
	DWORD dwMaxLen,					// Maximum size of output buffer (including NULL character)
	char cDelimiter,				// Delimiter 
	LPCTSTR szProxyName = NULL,		// Proxy name, optional
	LPCTSTR szProxyByPass = NULL,	// Proxy by pass, optional 
	LPCTSTR szUsername = NULL,		// User name, optional
	LPCTSTR szPassword = NULL		// Password, optional
	);


void MakeProxyName(LPCTSTR szProxyServer, int iPort, LPTSTR szProxyName);

#define QUERYCLOUD_CONNECTION_ERROR       -100
#define QUERYCLOUD_VIRUS                   100
#define QUERYCLOUD_NOT_VIRUS               200


int QueryFileSignatureData(LPCTSTR pszData);

#endif