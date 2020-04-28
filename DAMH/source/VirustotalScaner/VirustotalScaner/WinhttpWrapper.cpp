#include "StdAfx.h"
#include "WinhttpWrapper.h"
#include "SupportMain.h"

CWinhttpWrapper::CWinhttpWrapper(void)
{
	WriteLog(_T("Request = %d"), g_dwNumberRequest);
}

CWinhttpWrapper::~CWinhttpWrapper(void)
{
	CloseHandleWinhttp();
}

CWinhttpWrapper::CWinhttpWrapper(CString sServername, CString sObjectname, int iPort)
{
	this->m_sServername = sServername;
	this->m_sObjectname = sObjectname;
	this->m_iPort		= iPort;

	m_hSession = NULL;
	m_hConnect = NULL;
	m_hRequest = NULL;

	g_dwNumberRequest = 0;
}

CString CWinhttpWrapper::PostDataGetReponse(LPCWSTR pwszHeaderGeneral, LPCTSTR pszHeader, LPSTR pszData, int iSizeData)
{
	CString sReponse = _T("");

	if (!ConnectToServer()) return _T("");
	if (!OpenRequest())		return _T("");

	if (pwszHeaderGeneral != NULL)
	{
		if (!AddRequestHeader(pwszHeaderGeneral)) return _T("");
	}

	if (!SendRequest(pszHeader, pszData, iSizeData)) return _T("");

	sReponse = GetRequestResult();

	g_dwNumberRequest ++;

	return sReponse;
}

BOOL CWinhttpWrapper::ConnectToServer()
{

	m_hSession = WinHttpOpen( NULL,
							  WINHTTP_ACCESS_TYPE_NO_PROXY,
							  WINHTTP_NO_PROXY_NAME,
							  WINHTTP_NO_PROXY_BYPASS,
							  0);

	if (m_hSession == NULL)
	{
		return FALSE;
	}

	m_hConnect = WinHttpConnect( m_hSession,
								 m_sServername,
								 (INTERNET_PORT) m_iPort,
								 0);

	if (m_hConnect == NULL)
	{
		return FALSE;
		WinHttpCloseHandle(m_hSession);
	}

	return TRUE;
}

BOOL CWinhttpWrapper::OpenRequest()
{
	m_hRequest = WinHttpOpenRequest( m_hConnect,
									 L"POST",
									 m_sObjectname,
									 NULL, 
									 WINHTTP_NO_REFERER,
									 WINHTTP_DEFAULT_ACCEPT_TYPES,
									 WINHTTP_FLAG_ESCAPE_DISABLE_QUERY);

	if (m_hRequest == NULL)
	{
		WinHttpCloseHandle(m_hSession);
		WinHttpCloseHandle(m_hConnect);
	}

	return TRUE;
}

BOOL CWinhttpWrapper::AddRequestHeader(LPCWSTR pwszHeaderGeneral)
{
	BOOL  bResults;

	bResults = WinHttpAddRequestHeaders( m_hRequest,
							  pwszHeaderGeneral,
							  (ULONG)-1L,
							  WINHTTP_ADDREQ_FLAG_ADD);

	if (!bResults)
	{
		DWORD x = GetLastError();
		return FALSE;
	}

	return TRUE;
}

BOOL CWinhttpWrapper::SendRequest(LPCTSTR pszHeader, LPSTR pszData, int iSizeData)
{
	BOOL		bResult;
	DWORD		dwTotal;
	DWORD		dwDataLen;
	DWORD		dwHeaderLen;

	dwDataLen = iSizeData;

	if (pszHeader == NULL)
	{
		dwHeaderLen = 0;
	}
	else
	{
		dwHeaderLen = _tcslen(pszHeader);;
	}

	dwTotal = dwHeaderLen * sizeof(TCHAR) + dwDataLen;

	bResult = WinHttpSendRequest( m_hRequest,
								  pszHeader,
								  dwHeaderLen,
								  (LPVOID) pszData,
								  dwDataLen,
								  dwTotal,
								  NULL);

	if (!bResult)
	{
		return FALSE;
		WinHttpCloseHandle(m_hSession);
		WinHttpCloseHandle(m_hConnect);
		WinHttpCloseHandle(m_hRequest);
	}

	bResult = WinHttpReceiveResponse(m_hRequest, NULL);
	if (!bResult)
	{
		WinHttpCloseHandle(m_hSession);
		WinHttpCloseHandle(m_hConnect);
		WinHttpCloseHandle(m_hSession);
		return FALSE;
	}

	return TRUE;
}

CString CWinhttpWrapper::GetRequestResult()
{
	BOOL	bResult; 
	DWORD   dwSizeAvalible;
	DWORD	dwNumberOfBytesRead = 0;
	CString sResult;
	CHAR*	pszBuffer;	

	do 
	{
		dwSizeAvalible = 0;
		pszBuffer = NULL;
		bResult = WinHttpQueryDataAvailable(m_hRequest, &dwSizeAvalible);

		if (!bResult || dwSizeAvalible == 0)
		{
			break;
		}

		pszBuffer = new CHAR[dwSizeAvalible + 1];
		if (pszBuffer == NULL)
		{
			break;
		}

		ZeroMemory(pszBuffer, dwSizeAvalible + 1);

		bResult = WinHttpReadData(m_hRequest, pszBuffer, dwSizeAvalible, &dwNumberOfBytesRead);												
		pszBuffer[dwNumberOfBytesRead] = NULL;

		sResult += pszBuffer;
		delete[] pszBuffer;

	} while(bResult && dwNumberOfBytesRead);	

	return sResult;
}

void CWinhttpWrapper::CloseHandleWinhttp()
{
	if (m_hSession != NULL)
	{
		WinHttpCloseHandle(m_hSession);
	}

	if (m_hConnect != NULL)
	{
		WinHttpCloseHandle(m_hConnect);
	}

	if (m_hRequest != NULL)
	{
		WinHttpCloseHandle(m_hRequest);
	}
}