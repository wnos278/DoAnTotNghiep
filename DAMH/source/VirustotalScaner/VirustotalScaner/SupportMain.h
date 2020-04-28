#pragma once

#include "WriteLog.h"
#include <list>
#include <atlstr.h>

//------------------------------------------------------------------------
// Name:		fnWriteLog
// Decryption:	Write Log when run
#ifndef DEBUG
#define fnWriteLog	WriteLog
#else	
#define  fnWriteLog	
#endif // DEBUG

#ifdef DEBUG
#define		fnWriteLogOpenXML								WriteLogXML
#define		fnWriteLogCloseXML(szFunctionName, dwResult)	CWriteLogMV writeLogMV(szFunctionName, dwResult);
#else	
#define		fnWriteLogOpenXML	
#define		fnWriteLogCloseXML
#endif // DEBUG

// #define F_WRITE_LOG
// #define F_WRITE_LOG_DBG_PRINT

//========================== define alll ================================
// define size path
#ifndef MAX_PATH_EX
#define MAX_PATH_EX						MAX_PATH + 10
#endif



//================================All function global=====================================
void	WriteLog(__in LPCTSTR pszFormat, ...);
CHAR* UnicodeToAnsi(const WCHAR* szUnicode);
WCHAR* AnsiToUnicode(const CHAR* szAsc);
DWORD GetPostFile();
VOID SetPostFile(__in DWORD dwID);
CString MakePath(LPCTSTR pszDir, LPCTSTR pszFileName);

//==================================define macro safe==================================
#define SAFE_RELEASE(x)			\
			if (x)				\
		{						\
			delete x;			\
			x = NULL;			\
		}

#define SAFE_CLOSE(x)			\
			if (x)				\
		{						\
			CloseHandle(x);		\
			x = NULL;			\
		}	
#define SAFE_FREE(x)			\
		if (x)					\
		{						\
			free(x);			\
			x= NULL;			\
		}						\


#define MAX_LOADER_DB_COUNTER	 20

#define SZ_DIR_VIRUSTOTAL		_T("Virustotal")
#define NAME_FILE_INFO			_T("VirustotalConfig.ini")
#define NAME_FILE_LOG			_T("VirustotalLog.log")

// Define all key and section of config file
#define SECTION_INFO				_T("Virustotal Info")
#define KEY_API_KEY					_T("Key")
#define MY_APIKEY					_T("33643fb1ded95274e227d632ec3b3625fb8d7dc1469a2c71ff77862d2467f679")

class CSupportMain
{
public:
	CWriteLog		m_writeLog;

	CString			m_sAppDir;
	CString			m_sMyApiKey;

public:

	CSupportMain(void);
	~CSupportMain(void);

	BOOL InitAll();
	BOOL InitAppDir();
	BOOL MakeInfoFile();
	BOOL LoadInfoFile();

	DWORD  GetSizeOfFile(CString sPath);
	BOOL   ReadFileToBuffer(CString sPath, CHAR* pszBuffer, DWORD dwSizeFile);

};

// ======================================================================================
// All globals variables
// ======================================================================================
extern CSupportMain g_support;