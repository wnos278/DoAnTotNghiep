////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac phuong thuc lop CRuntimeException. 
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include "RuntimeException.h"
#include <string>
#include <sstream>
#include <windows.h>


#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG
//------------------------------------------------------------------------------
// Khoi tao doi tuong Exception, voi noi dung thong diep, loai Exception
//------------------------------------------------------------------------------
CRuntimeException::CRuntimeException(string sMsg, int nErrId, bool bContinue)
{
	m_isVirus = false;
	m_sMessage = sMsg;
	m_nErrId = nErrId;
	m_bContinue = bContinue;
}

//------------------------------------------------------------------------------
// Huy Exception
//------------------------------------------------------------------------------
CRuntimeException::~CRuntimeException(void)
{
}


//------------------------------------------------------------------------------
// hiển thị lên cửa sổ Output
//------------------------------------------------------------------------------
void OutputDebug(string sMsg)
{
#ifndef BUID_UPDATE
#ifndef _DEBUG_CHECK_MEM_LEAK
#ifdef _DEBUG
	string szMsgOut = 
		"----------------------------------------------------------------------\n__DEBUG__: "
		+ sMsg +
		"\n----------------------------------------------------------------------\n";
	OutputDebugStringA(szMsgOut.c_str());
#endif // !_DEBUG
#endif // !_DEBUG_CHECK_MEM_LEAK
#endif // !BUID_UPDATE
}

void OutputDebugW(wstring sMsg)
{
#ifdef NDEBUG
	wstring szMsgOut = 
		L"----------------------------------------------------------------------\n__DEBUG__: "
		+ sMsg +
		L"\n----------------------------------------------------------------------\n";
	OutputDebugStringW(szMsgOut.c_str());
#endif
#ifndef BUID_UPDATE
#ifndef _DEBUG_CHECK_MEM_LEAK
#ifdef _DEBUG
	wstring szMsgOut = 
		L"----------------------------------------------------------------------\n__DEBUG__: "
		+ sMsg +
		L"\n----------------------------------------------------------------------\n";
	OutputDebugStringW(szMsgOut.c_str());
#endif // !_DEBUG
#endif // !_DEBUG_CHECK_MEM_LEAK
#endif // !BUID_UPDATE
}


//------------------------------------------------------------------------------
// hiển thị lên cửa sổ Output + them dia chi mem
//------------------------------------------------------------------------------
void OutputDebugMemLeak(void* a, string sMemName)
{
#ifndef BUID_UPDATE
#ifndef _DEBUG_CHECK_MEM_LEAK
#ifdef _DEBUG
	ostringstream msg;
	int nAddr = (int)a;
	msg << "LEAK_0x" << hex << nAddr << ": " << sMemName;
	OutputDebug(msg.str());
#endif // !_DEBUG
#endif // !_DEBUG_CHECK_MEM_LEAK
#endif // !BUID_UPDATE
}