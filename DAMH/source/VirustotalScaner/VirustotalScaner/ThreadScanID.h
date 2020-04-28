#pragma once
#include "MyThread.h"
#include "Virustotal.h"

class CManagerThread;

class CThreadScanID : public CMyThread
{
private:
	BOOL				bRunFirst;
	CVirustotal			m_vt;
	CManagerThread*		m_pScan;

public:
	CThreadScanID(void);
	CThreadScanID(CString sDir, CString sApi);
	~CThreadScanID(void);
	void SetMainObjectInfo( __in CManagerThread* pMain );
	BOOL Run();
};
