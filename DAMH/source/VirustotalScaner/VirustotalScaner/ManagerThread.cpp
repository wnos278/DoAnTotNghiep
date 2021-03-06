#include "StdAfx.h"
#include "ManagerThread.h"
#include "SupportMain.h"

CManagerThread::CManagerThread(void)
{
}

CManagerThread::~CManagerThread(void)
{
	SAFE_RELEASE(m_pUpload);
	SAFE_RELEASE(m_pScanID);
	m_ltFile.RemoveAll();
	m_ltUpload.RemoveAll();
}

CManagerThread::CManagerThread(CString sDirScan)
{
	this->m_sDirScan = sDirScan;

	this->m_pUpload = NULL;
	this->m_pScanID = NULL;
	
	m_ltFile.RemoveAll();
	m_ltUpload.RemoveAll();

}

void CManagerThread::SetHwndDialogMain(HWND* pHwnd)
{
	this->m_pHwndDialogMain = pHwnd;
}

void CManagerThread::SetListCrtl(CListCtrl* pList)
{
	this->m_pListCtrl = pList;
}

void CManagerThread::SetBtnScan(CButton* pBtnScan)
{
	this->m_pBtnScan = pBtnScan;
}

BOOL CManagerThread::CallAllThreads()
{
	m_pUpload = new CThreadUpload(GetDirScan(), GetApiKey());
	m_pScanID = new CThreadScanID(GetDirScan(), GetApiKey());

	if (m_pUpload == NULL && m_pScanID == NULL)
		return FALSE;

	m_pUpload->SetTimeoutOption(10*1000);
	m_pUpload->SetMainObjectInfo(this);
	if (!m_pUpload->BeginThread())
		return FALSE;

	m_pScanID->SetTimeoutOption(30*1000);
	m_pScanID->SetMainObjectInfo(this);
	if (!m_pScanID->BeginThread())
		return FALSE;

	return TRUE;
}

CString CManagerThread::GetApiKey()
{
	return m_sKey;
}

CString CManagerThread::GetDirScan()
{
	return m_sDirScan;
}

BOOL CManagerThread::InitInstance()
{
	if (!g_support.InitAll())
		return FALSE;

	if (GetListFileScan(&m_ltFile) == 0)
		 return FALSE;
	
	m_sKey = g_support.m_sMyApiKey;

	m_hEventStopUpload = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hEventStopUpload == NULL) 
		return FALSE;

	m_hVentStopScanID = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hVentStopScanID == NULL)
		return FALSE;

	m_hMutexFile = CreateMutex(NULL, FALSE, NULL);
	if (m_hMutexFile == NULL)
		return FALSE;

	m_hMutextSyncho = CreateMutex(NULL, FALSE, NULL);
	if (m_hMutextSyncho == NULL)
		return FALSE;

	m_hMutextSyncho = CreateMutex(NULL, FALSE, NULL);
	if (m_hMutextSyncho == NULL)
		return FALSE;

	if (!CallAllThreads())
		return FALSE;

	return TRUE;
}

void CManagerThread::SetStopUpload()
{
	SetEvent(m_hEventStopUpload);
}

void CManagerThread::SetStopScanID()
{
	SetEvent(m_hVentStopScanID);
}

BOOL CManagerThread::IsStopUpload()
{
	if (WaitForSingleObject(m_hEventStopUpload, INFINITE) == WAIT_OBJECT_0)
		return TRUE;

	return FALSE;
}

void CManagerThread::GetListFileInfo(__inout LIST_FILE* pListFile)
{
	pListFile->RemoveAll();
	WaitForSingleObject(m_hMutexFile, INFINITE);
	for (int i = 0; i < m_ltFile.GetCount(); i++)
	{
		pListFile->AddTail(m_ltFile.RemoveHead());

		if (i == NUM_MAX_UPLOAD)
			break;
	}
	ReleaseMutex(m_hMutexFile);
}

void CManagerThread::AddToListFileInfo(__in FILE_INFOR fileinfo)
{
	WaitForSingleObject(m_hMutexFile, INFINITE);
	m_ltFile.AddTail(fileinfo);
	ReleaseMutex(m_hMutexFile);
}

void CManagerThread::GetCurrentListUpload(__inout LIST_UPLOAD *pListUpload)
{
	int iCount;
	pListUpload->RemoveAll();

	WaitForSingleObject(m_hMutextList, INFINITE);

	iCount = m_ltUpload.GetCount();
	for (int i = 0; i < iCount; i++)
	{
		if (i == NUM_MAX_SCAN_ID)
			break;

		pListUpload->AddTail(m_ltUpload.RemoveHead());
	}

	ReleaseMutex(m_hMutextList);
}

void CManagerThread::AddUploadToList(__in UPLOAD uploadfile)
{

	WaitForSingleObject(m_hMutextList, INFINITE);
	m_ltUpload.AddTail(uploadfile);
	ReleaseMutex(m_hMutextList);

}


BOOL CManagerThread::CheckFileIsScan(CString sPathFile, TCHAR* pszMd5)
{
	CString sDir;
	CString sFileNew;
	LPTSTR  pszFilename;

	pszFilename = PathFindFileName(sPathFile);
	sDir.Format(_T("%s\\%s"), m_sDirScan.GetBuffer(), pszMd5);
	sFileNew.Format(_T("%s\\%s"), sDir.GetBuffer(), pszFilename);

	if (!PathIsDirectory(sDir))
	{
		return FALSE;
	}

	MoveFileEx(sPathFile, sFileNew, MOVEFILE_REPLACE_EXISTING);

	return TRUE;
}

void CManagerThread::Interface(CString sPath, CString sMd5, CString sStatus)
{
	int nItem;
	int iItemCout;
	TCHAR szPath[MAX_PATH];
	TCHAR szMd5[MAX_PATH];
	TCHAR szStatus[MAX_PATH];

	_stprintf_s(szMd5, MAX_PATH, _T("%s"), sMd5.GetBuffer());
	_stprintf_s(szPath, MAX_PATH, _T("%s"), sPath.GetBuffer());
	_stprintf_s(szStatus, MAX_PATH, _T("%s"), sStatus.GetBuffer());

	iItemCout = this->m_pListCtrl->GetItemCount();

	LVFINDINFO info;
	info.flags = LVFI_STRING;
	info.psz = sPath.GetBuffer();

	nItem = this->m_pListCtrl->FindItem(&info);
	if (nItem == -1)
	{
		this->m_pListCtrl->InsertItem(iItemCout, szPath);
		this->m_pListCtrl->SetItemText(iItemCout, 1, szMd5);
		this->m_pListCtrl->SetItemText(iItemCout, 2, szStatus);
	}
	else
	{
		this->m_pListCtrl->SetItemText(nItem, 0, szPath);
		this->m_pListCtrl->SetItemText(nItem, 1, szMd5);
		this->m_pListCtrl->SetItemText(nItem, 2, szStatus);
	}
}

DWORD CManagerThread::GetListFileScan(__inout LIST_FILE* pListFile)
{
	HANDLE			hFind;
	WIN32_FIND_DATA w32Data;

	CString			sFileName;
	CString			sMd5;
	TCHAR			szMd5[MAX_PATH];

	FILE_INFOR		file_infor;

	// Find all file in directory
	sFileName.Format(_T("%s\\*.*"), m_sDirScan.GetString());

	hFind = FindFirstFileW(sFileName, &w32Data);
	if (hFind == INVALID_HANDLE_VALUE) return 0;

	while (FindNextFileW(hFind, &w32Data))
	{

		if (!wcscmp(w32Data.cFileName, _T(".")) || !wcscmp(w32Data.cFileName, _T("..")))
			continue;

		// If it is directory
		if ((w32Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			continue;

		// If it is file.txt
		if (StrStr(w32Data.cFileName, _T(".txt")) != NULL || StrStr(w32Data.cFileName, _T(".log")) != NULL)
			continue;

		sFileName.Format(_T("%s\\%s"), m_sDirScan, w32Data.cFileName);
		m_Md5.GetMD5File(sFileName.GetBuffer(), szMd5, MAX_PATH);
		sMd5.Format(_T("%s"), szMd5);
		
		file_infor.sPathFile = sFileName;
		file_infor.sMd5 = sMd5;

		pListFile->AddTail(file_infor);
	}

	return pListFile->GetCount();
}

BOOL CManagerThread::Run()
{
	HANDLE arrEvent[2];
	arrEvent[0] = m_hEventStopUpload;
	arrEvent[1] = m_hVentStopScanID;

	WaitForMultipleObjects(2, arrEvent, TRUE, INFINITE);

	WriteLog(_T("So file khi da quet xong trong listFile=%d"), m_ltFile.GetCount());
	WriteLog(_T("So upload khi da quet xong trong listUpload=%d"), m_ltUpload.GetCount());
	m_pBtnScan->EnableWindow(TRUE);
	MessageBox(NULL, _T("Scan Finished!"), _T("VirustotalScaner"), MB_OK);
	return FALSE;
}
