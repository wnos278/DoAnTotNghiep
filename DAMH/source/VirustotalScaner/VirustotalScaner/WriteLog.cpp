#include "StdAfx.h"
#include "WriteLog.h"
// #include "BkavServiceLib.h"

CWriteLog::CWriteLog(void)
{
	m_hWriteLogMutex = NULL;
	m_listLogWaiting.RemoveAll();
}

CWriteLog::~CWriteLog(void)
{
	Cleanup();
}

BOOL CWriteLog::InitAll(__in LPCTSTR pszLogPath)
{
	if (pszLogPath == NULL)
		return FALSE;
	if (!wcslen(pszLogPath))
		return FALSE;

	m_sLogPath = pszLogPath;	

	// Create mutex to synchronization write log
	m_hWriteLogMutex = CreateMutex(NULL, FALSE, NULL);
	if (m_hWriteLogMutex == NULL)
		return FALSE;

	return TRUE;
}

void CWriteLog::Cleanup()
{
	if (m_hWriteLogMutex != NULL)
	{
		CloseHandle(m_hWriteLogMutex);
		m_hWriteLogMutex = NULL;
	}

	m_listLogWaiting.RemoveAll();
}

BOOL CWriteLog::IsHaveWaitingLog()
{
	if (m_listLogWaiting.GetCount() > 0)
		return TRUE;

	// Have not log waiting to write
	return FALSE;
}

CString CWriteLog::GetWaitingLog()
{
	CString sLogWaiting;

	// Get all log is waiting to write file
	while (m_listLogWaiting.GetCount() > 0)
	{
		sLogWaiting += m_listLogWaiting.RemoveHead();
	}

	// Return all log is waiting
	return sLogWaiting;
}

void CWriteLog::SetWaitingLog(__in LPCTSTR pszWaitLog)
{
	// Validate parameter
	if (pszWaitLog == NULL)
		return;
	if (!wcslen(pszWaitLog))
		return;

	// Add log is tail of list
	m_listLogWaiting.AddTail(pszWaitLog);
}

// Write log
void CWriteLog::WriteLog(__in LPCTSTR pszFormat, ...)
{
	if (pszFormat == NULL)
		return;

	CString	sData;
	CTime	timeCurrent = CTime::GetCurrentTime();

	sData.Format(_T("%s\t%s\r\n"), timeCurrent.Format(_T("%d/%m/%Y %H:%M:%S")), pszFormat);

	// Synchronization process write log operator
	WaitForSingleObject(m_hWriteLogMutex, NUM_TIMEOUT_WRITE_LOG);

	if (IsHaveWaitingLog())
	{
		SetWaitingLog(sData);
		sData = GetWaitingLog();
	}

	if (!WriteToFile(sData))
	{
		if (sData.GetLength() < 5 * MAX_LOG_LEN)
			SetWaitingLog(sData);
	}

	ReleaseMutex(m_hWriteLogMutex);
}

// Write log of program
BOOL CWriteLog::WriteToFile(__in LPCTSTR pszData)
{
	// Validate parameter
	if (pszData == NULL)
		return TRUE;
	if (!wcslen(pszData))
		return TRUE;

	HANDLE	hFile;
	DWORD	dwSize;
	DWORD	dwSizeHi;
	DWORD	dwByteWrote;

	hFile = CreateFile(m_sLogPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
		return FALSE;

	// Check file size
	dwSize = GetFileSize(hFile, &dwSizeHi);
	if (dwSize > MAX_FILE_LOG_SIZE || dwSizeHi > 0)
	{
		CloseHandle(hFile);
		DeleteFile(m_sLogPath);

		hFile = CreateFile(m_sLogPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;

		dwSize = GetFileSize(hFile, &dwSizeHi);
	}

	if (!dwSize)
	{
		// Define unicode file
		WORD wHeader = 0xFEFF;
		if (!WriteFile(hFile, &wHeader, sizeof(WORD), &dwByteWrote, NULL))
		{
			CloseHandle(hFile);
			return FALSE;
		}
	}

	if (SetFilePointer(hFile, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	// Write data in file
	if (!WriteFile(hFile, pszData, wcslen(pszData) * sizeof(TCHAR), &dwByteWrote, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);
	return TRUE;
}


BOOL CWriteLog::WriteLogXML(__in LPCTSTR pszData)
{
	// Validate parameter
	if (pszData == NULL)
		return TRUE;
	if (!wcslen(pszData))
		return TRUE;

	HANDLE	hFile;
	DWORD	dwSize;
	DWORD	dwSizeHi;
	DWORD	dwByteWrote;

	TCHAR	szPathFileLogBackup[MAX_PATH_EX]	= {0};
	TCHAR	szPathLogXML[MAX_PATH_EX]			= {0};

	_tcscpy_s(szPathLogXML, m_sLogPath);
	_tcscat_s(szPathLogXML, MAX_PATH, _T(".xml"));

	hFile = CreateFile(szPathLogXML, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
		return FALSE;

	// Check file size
	dwSize = GetFileSize(hFile, &dwSizeHi);
	if (dwSize > MAX_FILE_LOG_SIZE || dwSizeHi > 0)
	{
		CloseHandle(hFile);

		_tcscpy_s(szPathFileLogBackup, szPathLogXML);
		_tcscat_s(szPathFileLogBackup, MAX_PATH, _T(".bka"));

		CopyFile(szPathLogXML, szPathFileLogBackup, FALSE);

		hFile = CreateFile(szPathLogXML, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;

		dwSize = GetFileSize(hFile, &dwSizeHi);
	}

	if (!dwSize)
	{
		// Define unicode file
		WORD wHeader = 0xFEFF;
		if (!WriteFile(hFile, &wHeader, sizeof(WORD), &dwByteWrote, NULL))
		{
			CloseHandle(hFile);
			return FALSE;
		}
	}

	if (SetFilePointer(hFile, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	// Write data in file
	if (!WriteFile(hFile, pszData, wcslen(pszData) * sizeof(TCHAR), &dwByteWrote, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);
	return TRUE;
}

