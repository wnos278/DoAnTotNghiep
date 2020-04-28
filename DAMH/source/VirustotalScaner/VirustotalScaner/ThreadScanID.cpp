#include "StdAfx.h"
#include "ThreadScanID.h"
#include "ManagerThread.h"
#include "SupportMain.h"


CThreadScanID::CThreadScanID(void)
{

}

CThreadScanID::~CThreadScanID(void)
{
}

CThreadScanID::CThreadScanID(CString sDir, CString sApi)
{
	bRunFirst = FALSE;
	m_vt.SetDirScan(sDir);
	m_vt.SetApiKey(sApi);
}

void CThreadScanID::SetMainObjectInfo( __in CManagerThread* pMain )
{
	m_pScan = pMain;
}

BOOL CThreadScanID::Run()
{
	LIST_UPLOAD ltUpload;
	UPLOAD	upload;
	POSITION pos;
	CString sResult;
	
	if (bRunFirst == FALSE)
	{
		Sleep(30 * 1000);
		bRunFirst = TRUE;
	}

	WaitForSingleObject(m_pScan->m_hMutextSyncho, INFINITE);

	m_pScan->GetCurrentListUpload(&ltUpload);
	if (!ltUpload.GetCount() && m_pScan->IsStopUpload())
	{
		m_pScan->SetStopScanID();
		return FALSE;
	}

	pos = ltUpload.GetHeadPosition();
	for (int i = 0; i < ltUpload.GetCount(); i++)
	{
		upload = ltUpload.GetNext(pos);
		DWORD dwResut = m_vt.ScanResource(upload.file_infor.sPathFile, upload.file_infor.sMd5, upload.sScanID);
		if (dwResut != ERROR_SUCCESS)
		{
			m_pScan->AddUploadToList(upload);
			continue;
		}
		WriteLog(_T("ScanResource=%d: File = %s, ScanID = %s"),dwResut, upload.file_infor.sPathFile, upload.sScanID);
		m_pScan->Interface(upload.file_infor.sPathFile, upload.file_infor.sMd5, _T("OK!"));
		Sleep(1000);
	}

	ReleaseMutex(m_pScan->m_hMutextSyncho);

	return TRUE;
}