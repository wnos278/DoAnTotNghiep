#pragma once
#include "MyThread.h"
#include "Virustotal.h"

class CManagerThread;

class CThreadUpload : public CMyThread
{
private:
	CManagerThread* m_pScan;
	CVirustotal  m_vt;

public:
	CThreadUpload(void);
	CThreadUpload(CString sDir, CString sApi);
	~CThreadUpload(void);
	void SetMainObjectInfo( __in CManagerThread* pMain );
	BOOL Run();
};
