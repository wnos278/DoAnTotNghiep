#pragma once
#include "MyThread.h"
#include "Virustotal.h"
#include "Md5.h"
#include "ParseJsonVirustotal.h"
#include "ThreadUpload.h"
#include "ThreadScanID.h"


#define MAX_COUNT			30
#define MAX_SLEEP			2000
#define NUM_MAX_SCAN_ID		4
#define NUM_MAX_UPLOAD		4

typedef struct FILE_INFOR
{
	CString sPathFile;
	CString sMd5;
}FILE_INFOR;

typedef struct UPLOAD
{
	FILE_INFOR file_infor;
	CString sScanID;
}UPLOAD;


typedef CList<FILE_INFOR> LIST_FILE;
typedef CList<UPLOAD> LIST_UPLOAD;

class CManagerThread:public CMyThread
{
private:
	CString m_sKey;
	CString m_sDirScan;

	CThreadUpload* m_pUpload;
	CThreadScanID* m_pScanID;
	CVirustotal* m_pVirustoal;
	CMd5		 m_Md5;
	
	// interface
	HWND*		m_pHwndDialogMain;
	CListCtrl*	m_pListCtrl;
	CButton*	m_pBtnScan;
	
	HANDLE m_hMutextList;
	HANDLE m_hMutexFile;
	HANDLE m_hEventStopUpload;
	HANDLE m_hVentStopScanID;
	
	LIST_FILE	m_ltFile;
	LIST_UPLOAD m_ltUpload;

public:
	HANDLE m_hMutextSyncho;

public:
	CManagerThread(void);
	CManagerThread(CString sDirScan);
	~CManagerThread(void);

	CString GetApiKey();
	CString GetDirScan();

	void SetHwndDialogMain(HWND* pHwnd);
	void SetListCrtl(CListCtrl* pList);
	void SetBtnScan(CButton* pBtnScan);
	void Interface(CString sPath, CString sMd5, CString sStatus);

	BOOL IsStopUpload();
	void SetStopUpload();
	void SetStopScanID();

	void GetListFileInfo(__inout LIST_FILE* pListFile);
	void AddToListFileInfo(__in FILE_INFOR fileinfo);

	void GetCurrentListUpload(__inout LIST_UPLOAD *pListUpload);
	void AddUploadToList(__in UPLOAD uploadfile);

	BOOL CallAllThreads();
	BOOL InitInstance();
	BOOL Run();

	void  Scan(CString sPathFile, int iCountFile);
	DWORD ScanResource(CString sPathFile, TCHAR* szMd5, TCHAR* szResource);
	BOOL ScanUploadFile(CString sFile, TCHAR* szMd5);

	DWORD GetListFileScan(__inout LIST_FILE* pListFile);
	BOOL CheckFileIsScan(CString sPathFile, TCHAR* pszMd5);
	BOOL WriteFileLog(TCHAR* szMd5Folder, CHAR*pszBuffer, DWORD dwSize, CString sPathFile);
};
