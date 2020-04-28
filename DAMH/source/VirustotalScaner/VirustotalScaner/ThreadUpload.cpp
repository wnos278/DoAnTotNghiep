#include "StdAfx.h"
#include "ThreadUpload.h"
#include "ManagerThread.h"
#include "SupportMain.h"

CThreadUpload::CThreadUpload(void)
{
}

CThreadUpload::CThreadUpload(CString sDir, CString sApi)
{
	m_vt.SetDirScan(sDir);
	m_vt.SetApiKey(sApi);
}

CThreadUpload::~CThreadUpload(void)
{
}

void CThreadUpload::SetMainObjectInfo( __in CManagerThread* pMain )
{
	m_pScan = pMain;
}

BOOL CThreadUpload::Run()
{
	FILE_INFOR file;
	LIST_FILE ltFile;

	UPLOAD	upload;
	
	POSITION pos;
	CString sScanID;

	WaitForSingleObject(m_pScan->m_hMutextSyncho, INFINITE);

	m_pScan->GetListFileInfo(&ltFile);
	if (!ltFile.GetCount())
	{
		m_pScan->SetStopUpload();
		return FALSE;
	}

	pos = ltFile.GetHeadPosition();
	for (int i = 0; i < ltFile.GetCount(); i++)
	{
		file = ltFile.GetNext(pos);
		sScanID = m_vt.UploadFile(file.sPathFile);
		WriteLog(_T("ScanID file: %s = %s"), file.sPathFile, sScanID);
		if (sScanID == "")
		{
			m_pScan->AddToListFileInfo(file);
			continue;
		}

		upload.file_infor.sPathFile = file.sPathFile;
		upload.file_infor.sMd5 = file.sMd5;
		upload.sScanID = sScanID;

		m_pScan->AddUploadToList(upload);
		m_pScan->Interface(upload.file_infor.sPathFile, upload.file_infor.sMd5, _T("Upload..."));
		Sleep(1000);
	}

	ReleaseMutex(m_pScan->m_hMutextSyncho);

	return TRUE;
}