#include "StdAfx.h"
#include "MyThread.h"
#include "SupportMain.h"

CMyThread::CMyThread(void)
{
	m_hThread = NULL;
	m_bTerminate = FALSE;
	m_iLoopTimeout = 0;
	m_hStopThreadEvent = NULL;
}

CMyThread::~CMyThread(void)
{
	Cleanup();
}

BOOL CMyThread::BeginThread()
{
	// Refresh all parameters
	Cleanup();

	// Create event for stoping thread
	m_hStopThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hStopThreadEvent == NULL)
		return FALSE;

	// Create scan-sample thread
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadMain, this, 0, NULL);
	if (m_hThread == NULL)
		return FALSE;

	return TRUE;
}

// Thread scan samples
DWORD WINAPI CMyThread::ThreadMain(__in LPVOID lpParam)
{
	if (lpParam == NULL)
		return 0;

	CMyThread*	pThread = (CMyThread*) lpParam;

	if (!pThread->InitInstance())
		return 1;

	while (TRUE)
	{
		if (!pThread->Run())
			break;

		if (!pThread->WaitForTimeout(pThread->m_iLoopTimeout))
			break;
	}

	return 1;
}

BOOL CMyThread::InitInstance()
{
	return TRUE;
}

// Scan samples queue
BOOL CMyThread::Run()
{
	Sleep(10000);
	return TRUE;
}

// Set event stop-thread to end thread
void CMyThread::EndThread()
{
	if (m_hStopThreadEvent != NULL)
		SetEvent(m_hStopThreadEvent);
}

void CMyThread::SetTerminateThreadOption()
{
	m_bTerminate = TRUE;
}

void CMyThread::SelfDelete()
{
	delete this;
}

void CMyThread::SetTimeoutOption( __in int iMilliseconds )
{
	if (iMilliseconds <= 0)
		return;

	m_iLoopTimeout = iMilliseconds;
}

BOOL CMyThread::WaitForTimeout( __in int iMilliseconds )
{
	DWORD	dwResult	= ERROR_SUCCESS;

	if (iMilliseconds <= 0)
	{
		if (m_hStopThreadEvent == NULL || WaitForSingleObject(m_hStopThreadEvent, 0) == WAIT_OBJECT_0)
		{
			dwResult	= 1;

			return FALSE;
		}

		return TRUE;
	}

	int	iCount;
	int	iSleep = 100;

	iCount = iMilliseconds / iSleep;
	if (!iCount)
		iCount = 1;

	for (int i = 0; i < iCount; i++)
	{
		if (m_hStopThreadEvent == NULL || WaitForSingleObject(m_hStopThreadEvent, iSleep) == WAIT_OBJECT_0)
		{
			dwResult	= 1;
			return FALSE;
		}
	}

	return TRUE;
}

void CMyThread::Cleanup()
{
	if (m_hStopThreadEvent != NULL)
	{
		SetEvent(m_hStopThreadEvent);
		WaitForSingleObject(m_hThread, TIME_OUT_END_THREAD);	// Dùng riêng cho BCOS
		CloseHandle(m_hStopThreadEvent);
		m_hStopThreadEvent = NULL;
	}

	if (m_hThread != NULL)
	{
		if (m_bTerminate)
		{
			DWORD dwExitCode;

			WaitForSingleObject(m_hThread, TIME_OUT_END_THREAD);
			GetExitCodeThread(m_hThread, &dwExitCode);
			TerminateThread(m_hThread, dwExitCode);
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}
