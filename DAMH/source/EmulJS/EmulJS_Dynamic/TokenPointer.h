////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia lop CTokenPointer; Lop dai dien cho con tro lenh
// thuc hien cua chuong trinh thong dich JS
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef  TOKEN_POITER_H
#define  TOKEN_POITER_H

#include "DATA_DEF.h"
#include "RuntimeException.h"

#include <windows.h>
#include <string>
#include <iostream>
using namespace std ;

///////////////////////////////////////////////////////////////////////////////
/////////////////// LOCAL FUNCTION SUPPORT FIND TOKEN /////////////////////////

extern DWORD g_dwTimeStartScan;

// Return true if (ch = \t, ' ', \n, \r   
bool IsWhitespace(char cCheck);

// Return true if (ch = [0-9]) 
bool IsNumeric(char cCheck);

// Return true if (str = "[0-9]" ) 
bool IsNumber(const string &sCheck);

// return true if(chr = [0-9][a-f][A-F]
bool IsHexadecimal(char cCheck);

// return true if(chr = [_][a-z][A-Z]
bool IsAlpha(char cCheck);

// check variable's name
bool IsIDString(const char *szCheck);

// replace "textFrom" -> "textTo" in "str"
void Replace(string &sRoot, char cFrom, const char *cTo);

// convert the given string into a quoted string suitable for javascript
string GetJSString(const string &sInput);

/** Is the string alphanumeric */
bool IsAlphaNum(const string &sCheck);

//------------------------------------------------------------------------------
// Name: CTokenPointer
// Description: Save Current token's info,  and find next Token. 
//------------------------------------------------------------------------------
class CTokenPointer
{
public:
	CTokenPointer(const string &sCodeJs);
	CTokenPointer(CTokenPointer *pTPOwner, int nStartChar, int nEndChar);
	~CTokenPointer(void);

	char m_currCh, m_nextCh,m_prevCh;
	int m_nTokenId;		///< The type of the token that we have
	int m_nTokenId_old;
	int m_nTokenStart;	///< Position in the data at the beginning of the token we have here
	int m_nTokenEnd;	///< Position in the data at the last character of the token we have here
	int m_nTokenLastEnd;///< Position in the data at the last character of the last token
	string m_sTokenStr;///< Data contained in the token we have here

	
	void	JumpLastStatement(); /// jmp to last Statement
	void	JumpOutFuncCall();		// bo qua loi goi ham <cac ham chua dc dinh nghia>
	void	JumpOutTryCatch();
	
	void	SkipTo(int nTokenId);	// bo qua cho den khi m_nTokenId = nTokenId

	void	Match(int nExpectedTk); ///< Lexical match wotsit
	static string GetTokenStr(int nToken); ///< Get the string representation of the given token
	void	Reset(); ///< Reset this lex so we can start again
	string		GetSubString(int nPos); ///< Return a sub-string from the given position up until right now
	string		GetCode(){ return m_szData; };
	CTokenPointer*	GetSubToken(int nLastPosition); ///< Return a sub-lexer from the given position up until right now
	CTokenPointer*  GetSubTokenForIn(int nLastPositionForIn);
	string		GetSubDataEqualLevel(int nTypeTk1, int nTypeTk2);
	int			TryWatchNextToken();	// return token Id next

	string		GetPosition(int nPos=-1); ///< Return a string representing the position in lines and columns of the character pos given
	
	// Kiem tra ki tu enter có gặp trong quá trình GetNextToken hay khong
	bool		CheckHadEnterCharWhenGetNextToken();	

protected:
	/* When we go into a loop, we use getSubLex to get a lexer for just the sub-part of the
	   relevant string. This doesn't re-allocate and copy the string, but instead copies
	   the data pointer and sets dataOwned to false, and dataStart/dataEnd to the relevant things. */
	char*	m_szData; ///< Data string to get tokens from
	int		m_nDataStart, m_nDataEnd; ///< Start and end position in data string
	bool	m_bDataOwned; ///< Do we own this data string?
	bool	m_bHadEnterCharacter;

	int m_nDataPos; ///< Position in data (we CAN go past the end of the string here)

	void GetNextCh();		///< Get the text char from our text string
	void GetNextToken();	///< Get the text token from our text string

};

#endif	