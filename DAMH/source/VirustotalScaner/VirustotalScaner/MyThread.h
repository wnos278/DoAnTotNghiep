#pragma once


#define TIME_OUT_END_THREAD		(30 * 1000)

class CMyThread
{
public:
	CMyThread(void);
	~CMyThread(void);

protected:
	HANDLE	m_hThread;
	BOOL	m_bTerminate;

public:
	int		m_iLoopTimeout;				// milleseconds
	HANDLE	m_hStopThreadEvent;

protected:
	static DWORD WINAPI ThreadMain(__in LPVOID lpParam);
	virtual BOOL InitInstance();
	virtual BOOL Run();
	void Cleanup();

public:
	BOOL BeginThread();
	void EndThread();
	void SetTerminateThreadOption();
	void SetTimeoutOption(__in int iMilliseconds);
	BOOL WaitForTimeout(__in int iMilliseconds);
	void SelfDelete();
};