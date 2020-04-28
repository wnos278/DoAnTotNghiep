#pragma once


#define NUM_TIMEOUT_WRITE_LOG		(5 * 60 * 1000)				// milliseconds
#define MAX_LOG_LEN					(2 * 1024)					// 2KB		
#define MAX_FILE_LOG_SIZE			(40 * 1024 * 1024)			// 40MB


#ifndef MAX_PATH_EX
#define MAX_PATH_EX		MAX_PATH + 10

#endif

// Define list of string-log waitting writen
typedef CList<CString, LPCTSTR>	LOG_WAIT_LIST;


class CWriteLog
{
public:
	CWriteLog(void);
	~CWriteLog(void);

private:
	CString			m_sLogPath;
	LOG_WAIT_LIST	m_listLogWaiting;

public:
	HANDLE			m_hWriteLogMutex;

private:
	BOOL IsHaveWaitingLog();
	CString GetWaitingLog();
	void SetWaitingLog(__in LPCTSTR pszWaitLog);

public:
	BOOL InitAll(__in LPCTSTR pszLogPath);
	void Cleanup();
	void WriteLog(__in LPCTSTR pszFormat, ...);
	BOOL WriteToFile(__in LPCTSTR pszData);
	BOOL WriteLogXML(__in LPCTSTR pszData);		//Create by VietMV - 12/10/2014
};

