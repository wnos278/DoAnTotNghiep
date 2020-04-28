////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia lop CRuntimeException. Tao Exception rieng cua
// chuong trinh de xu ly cac loi trong qua trinh bien dich code JS
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef RUNTIME_EXCEPTION_H
#define RUNTIME_EXCEPTION_H

#include <iostream>
using namespace std;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// 1 so ham Don gian ho tro debug 
// hiển thị lên cửa sổ Output
//------------------------------------------------------------------------------
void OutputDebug(string sMsg);
void OutputDebugW(wstring sMsg);
void OutputDebugMemLeak(void* a, string sMemName = "_NONAME_");

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Cac ma loi co the xu ly khi xay ra exception 
#define CREATE_FILE_FALSE  500
#define READ_FILE_FALSE    501

#define EXCEPTIONID_FOUND_VIRUS			502
#define	EXCEPTIONID_WRITE_SCRIPT		503
#define EXCEPTIONID_EXE_FUNC_FAIL		504

#define FUNCTION_NOT_DEFINE	505
#define NOT_ENOUGH_MEMORY	506

#define EXCEPTION_ID_NOTFOUND_IN_TRYCATCH	507
#define EXCEPTION_STATEMENT_STRUCT_ERROR	508
#define EXCEPTION_THROW_IN_TRYCATCH			509
#define EXCEPTION_VAR_NOT_REDEFINE			510

#define EXCEPTION_TIME_OUT_SCAN				511
#define EXCEPTION_THROW_BREAK               512
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Macro tao exception message
#define CREATE_MSG_ID_UNDEFINED(szNameVar)  ("ReferenceError: "+(szNameVar)+ " is not defined")
//------------------------------------------------------------------------------


//==============================================================================
// Name: CRuntimeException
// Description: exception when run code Error
//				s_szMessage have info about this Error
//==============================================================================
class CRuntimeException : public exception
{
private:
	string	m_sVrName;
	int		m_nErrId;

	bool	m_bContinue;
	bool    m_isVirus;

public:
	string	m_sMessage;
	
	CRuntimeException(string sMsg, int nErrId = -1, bool bContinue = true);
	~CRuntimeException(void);

	void SetErrId(int id){m_nErrId = id;}

	void SetContinueRun(bool bContinue){m_bContinue = bContinue;}

	void SetVirusName(string sName){ m_isVirus = true; m_sVrName = sName; }

	bool IsVirus(){ return m_isVirus; }

	int	 GetErrId(){return m_nErrId;}

	bool IsContinueRun(){return m_bContinue;}

	string	 GetVirusName(){ return m_sVrName; }

	string GetMsg(){
		return m_sMessage;
	}

	void SetMsg(string sMsg){
		this->m_sMessage = sMsg;
	}

};

#endif // !RUNTIME_EXCEPTION_H