#pragma once
#include <winhttp.h>
#pragma comment(lib, "Winhttp.lib")


class CWinhttpWrapper
{
private:
	CString m_sServername;
	CString m_sObjectname;
	int		m_iPort;

	HINTERNET	m_hSession;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	DWORD	g_dwNumberRequest;

public:
	CWinhttpWrapper(void);
	CWinhttpWrapper(CString sServername, CString sObjectname, int iPort);
	~CWinhttpWrapper(void);

	BOOL ConnectToServer();
	BOOL OpenRequest();
	BOOL AddRequestHeader(LPCWSTR pwszHeaderGeneral);
	BOOL SendRequest(LPCTSTR pszHeader, LPSTR pszData, int iSizeData);
	CString GetRequestResult();
	void CloseHandleWinhttp();

	CString PostDataGetReponse(LPCWSTR pwszHeaderGeneral, LPCTSTR pszHeader, LPSTR pszData, int iSizeData);
};
