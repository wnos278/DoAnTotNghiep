////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac phuong thuc lop CTokenPointer. Lop con tro 
// lenh thuc thi cua trinh thong dich JS
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include "TokenPointer.h"
#include <sstream> // to use errorString

///////////////////////////////////////////////////////////////////////////////
/////////////////// LOCAL FUNCTION SUPPORT FIND TOKEN /////////////////////////

//------------------------------------------------------------------------------
// Return true if (ch = \t, ' ', \n, \r   
//------------------------------------------------------------------------------
bool IsWhitespace(char cCheck) 
{
	return (cCheck==' ') || (cCheck=='\t') || (cCheck=='\n') || (cCheck=='\r');
}

//------------------------------------------------------------------------------
// Return true if (ch = [0-9]) 
//------------------------------------------------------------------------------
bool IsNumeric(char cCheck) 
{
	return (cCheck>='0') && (cCheck<='9');
}

//------------------------------------------------------------------------------
// Return true if (str = "[0-9]" ) 
//------------------------------------------------------------------------------
bool IsNumber(const string &sCheck) 
{
	size_t i = 0;
	for (i = 0; i < sCheck.size(); i++)
		if (!IsNumeric(sCheck[i])) return false;
	return true;
}

//------------------------------------------------------------------------------
// return true if(chr = [0-9][a-f][A-F]
//------------------------------------------------------------------------------
bool IsHexadecimal(char cCheck) 
{
	return ((cCheck>='0') && (cCheck<='9')) ||
		((cCheck>='a') && (cCheck<='f')) ||
		((cCheck>='A') && (cCheck<='F'));
}

//------------------------------------------------------------------------------
// return true if(chr = [_][a-z][A-Z]
//------------------------------------------------------------------------------
bool IsAlpha(char cCheck)
{
	return ((cCheck >= 'a') && (cCheck <= 'z')) 
		|| ((cCheck >= 'A') && (cCheck <= 'Z'))
		|| cCheck == '_' || cCheck == '$';
}

//------------------------------------------------------------------------------
// check variable's name
//------------------------------------------------------------------------------
bool IsIDString(const char *szCheck)
{
	int nLengh = strlen(szCheck);

	if(nLengh<0 || nLengh>255)
		throw new CRuntimeException("\nSize of variable's name is too large");
	if (!IsAlpha(*szCheck))
		return false;
	while (*szCheck) 
	{
		if (!(IsAlpha(*szCheck) || IsNumeric(*szCheck)))
			return false;
		szCheck++;
	}
	return true;
}

//------------------------------------------------------------------------------
// replace "textFrom" -> "textTo" in "str"
//------------------------------------------------------------------------------
void Replace(string &sRoot, char cFrom, const char *cTo)
{
	int nLen = strlen(cTo);
	size_t nPos = sRoot.find(cFrom);

	while (nPos != string::npos) 
	{
		sRoot = sRoot.substr(0, nPos) + cTo + sRoot.substr(nPos+1);
		nPos = sRoot.find(cFrom, nPos+nLen);
	}
}

//------------------------------------------------------------------------------
// convert the given string into a quoted string suitable for javascript
//------------------------------------------------------------------------------
string GetJSString(const string &sInput)
{
	int nCh;
	const char *szReplaceWith = "";
	bool bReplace = true;
	string sJSString = sInput;
	size_t i = 0;

	for (i = 0; i < sJSString.size(); i++) 
	{

		switch (sJSString[i]) 
		{
			case '\\': szReplaceWith = "\\\\"; break;
			case '\n': szReplaceWith = "\\n"; break;
			case '\r': szReplaceWith = "\\r"; break;
			case '\a': szReplaceWith = "\\a"; break;
			case '"': szReplaceWith = "\\\""; break;
			default:
			{
					   nCh = ((int)sJSString[i]) & 0xFF;
					   if (nCh<32 || nCh>127)
					   {
						   char buffer[5];
						   _snprintf_s(buffer, 5, _TRUNCATE, "\\x%02X", nCh);
						   szReplaceWith = buffer;
					   }
					   else bReplace = false;
			}
		}

		if (bReplace) 
		{
			sJSString = sJSString.substr(0, i) + szReplaceWith + sJSString.substr(i + 1);
			i += strlen(szReplaceWith) - 1;
		}
	}
	return "\"" + sJSString + "\"";
}

//------------------------------------------------------------------------------
/** Is the string alphanumeric */
//------------------------------------------------------------------------------
bool IsAlphaNum(const string &sCheck)
{
	size_t i = 0;
	
	if (sCheck.size()==0) 
		return true;
	if (!IsAlpha(sCheck[0])) 
		return false;
	
	for (i = 0; i < sCheck.size(); i++)
	{
		if (!(IsAlpha(sCheck[i]) || IsNumeric(sCheck[i])))
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// init new token with "input"
//------------------------------------------------------------------------------
CTokenPointer::CTokenPointer(const string &sCode)
{
	AddListToken(this);
	m_szData = _strdup(sCode.c_str());		// code file .js saved in 'data' to process
	m_bDataOwned = true;
	m_nDataStart = 0;
	m_nDataEnd = strlen(m_szData);
	Reset();		// get first token
}

//------------------------------------------------------------------------------
// Tao 1 con tro lenh tu 1 con tro lenh co truoc
//------------------------------------------------------------------------------
CTokenPointer::CTokenPointer(CTokenPointer *pTPOwner, int nStartChar, int nEndChar)
{
	AddListToken(this);
	m_szData = pTPOwner->m_szData;
	m_bDataOwned = false;
	m_nDataStart = nStartChar;
	m_nDataEnd = nEndChar;
	Reset();
}

//------------------------------------------------------------------------------
// Huy con tro lenh
//------------------------------------------------------------------------------
CTokenPointer::~CTokenPointer(void)
{
	if (m_bDataOwned)
	{
		SAFE_DELETE_ARRAY(m_szData);
	}
}

//------------------------------------------------------------------------------
// getNextToken() cho den khi tim duoc token ';' hoac  '}'
// de ket thuc cau lenh hoac khoi lenh khong xu ly dc
//------------------------------------------------------------------------------
void CTokenPointer::JumpLastStatement() 
{
	int nLevel = 0;
	while (this->m_nTokenId != TK_EOF)
	{
		if (this->m_nTokenId == ';' || this->m_nTokenId == '}')
		{
			GetNextToken();		// finally
			break;
		}
		else
			GetNextToken();
	}
}


//------------------------------------------------------------------------------
// getNextToken() cho den khi ket thuc loi goi ham 
// CTokenPointer::jumpOutFuncCall(): duoc goi khi gap API chua duoc cai dat 
//
// => cho phep bo qua ham, su dung bien CVar "undefine" thay vao loi goi ham API
//    va tiep tuc xu ly cac toan tu tiep theo.
//
// vd: var k = funcName1( funcName2(), var_x ) + "API 'funcName1' chua cai dat";
//------------------------------------------------------------------------------
void CTokenPointer::JumpOutFuncCall()
{
	int nLevel = 1;
	while (this->m_nTokenId != TK_EOF )
	{
		GetNextToken();	// tim token tiep theo

		if (this->m_nTokenId == '(')// kiem tra xem co loi goi long nhau hay k
			nLevel++;
		else if (this->m_nTokenId == ')')	// kiem tra ket thuc loi goi ham
			nLevel--;

		if (nLevel == 0)
			return;
	}
}

//------------------------------------------------------------------------------
// getNextToken() cho den khi tim duoc token = TK_R_CATCH => xu ly khoi catch
//------------------------------------------------------------------------------
void CTokenPointer::JumpOutTryCatch()
{
	int nLevel = 1;
	while (this->m_nTokenId != TK_EOF)
	{
		if (this->m_nTokenId == TK_R_TRY)// kiem tra xem co loi goi long nhau hay k
			nLevel++;
		else if (this->m_nTokenId == TK_R_CATCH)	// kiem tra ket thuc loi goi ham
			nLevel--;
		if (nLevel == 0)
			return;
		GetNextToken();	// tim token tiep theo

	}
}

//------------------------------------------------------------------------------
// getNextToken() cho den khi tim duoc token can thiet
//------------------------------------------------------------------------------
void CTokenPointer::SkipTo(int nTokenId)
{
	while (this->m_nTokenId != TK_EOF && this->m_nTokenId != nTokenId)
	{
		GetNextToken();
	}
}

//------------------------------------------------------------------------------
// neu "m_nTokenId = tkProgram" thi tim kiem token tiep theo
// neu "m_nTokenId # tkProgram" -> exception
//
// -> dat ra kich ban hoat dong cho chuong trinh
//------------------------------------------------------------------------------
void CTokenPointer::Match(int nTkProgram) 
{
	int i = 0;
	int nSizMsg = 20;
	string sInfoTypeTk;
	string sTokenDataErr;

#ifdef BUID_UPDATE_CHECK_TIME_OUT
	// Kiểm tra thời gian timeout đã quét
	if ((GetTickCount() - g_dwTimeStartScan) > TIME_TO_SCAN_VIRUS)
		throw new CRuntimeException("Timeout!", EXCEPTION_TIME_OUT_SCAN);
#endif

	// check "current Token" with "token of Programing"
	if (m_nTokenId != nTkProgram)
	{
		// Ki tu ket thuc cau lenh co the la '\n' || '}'
		if (nTkProgram == ';'
			&&	(m_nTokenId == '}' || CheckHadEnterCharWhenGetNextToken()))
			return;
		
		// Cac truong hop loi
		else
		{	

#ifdef _DEBUG
			i = (m_nTokenStart > 20) ? (m_nTokenStart - 20) : 0;
			nSizMsg = (nSizMsg + i > m_nDataEnd) ? m_nDataEnd : nSizMsg + i;

			for (; i < nSizMsg; i++)
				sTokenDataErr += m_szData[i];
#endif
			sInfoTypeTk = "Loi::  tk[Dung] = " + GetTokenStr(nTkProgram) 
				+ " => tk[Sai] = " + GetTokenStr(m_nTokenId) 
				+ "\n=> viTri = " + GetPosition(m_nTokenStart);
			sInfoTypeTk += "\nm_szTokenData = \n" + sTokenDataErr;
			
			throw new CRuntimeException(sInfoTypeTk, nTkProgram);
		}
	}

	// find next token to continue Running
	GetNextToken();
}

//------------------------------------------------------------------------------
// Xuat token duoi dang String
//------------------------------------------------------------------------------
string CTokenPointer::GetTokenStr(int nToken)
{
	string sReturn = "Unknown this token: ";

	if (nToken>32 && nToken<128) 
	{
		char buf[4] = "' '";
		buf[1] = (char)nToken;
		return buf;
	}
	switch (nToken) 
	{
		case TK_EOF : return "EOF";
		case TK_ID : return "ID";
		case TK_INT : return "INT";
		case TK_FLOAT : return "FLOAT";
		case TK_STR : return "STRING";
		case TK_EQUAL : return "==";
		case TK_TYPEEQUAL : return "===";
		case TK_NEQUAL : return "!=";
		case TK_NTYPEEQUAL : return "!==";
		case TK_LEQUAL : return "<=";
		case TK_LSHIFT : return "<<";
		case TK_LSHIFTEQUAL : return "<<=";
		case TK_GEQUAL : return ">=";
		case TK_RSHIFT : return ">>";
		case TK_RSHIFTUNSIGNED : return ">>";
		case TK_RSHIFTEQUAL : return ">>=";
		case TK_PLUSEQUAL : return "+=";
		case TK_MINUSEQUAL : return "-=";
		case TK_PLUSPLUS : return "++";
		case TK_MINUSMINUS : return "--";
		case TK_ANDEQUAL : return "&=";
		case TK_ANDAND : return "&&";
		case TK_OREQUAL : return "|=";
		case TK_OROR : return "||";
		case TK_XOREQUAL : return "^=";
			// reserved words
		case TK_R_IF : return "if";
		case TK_R_ELSE : return "else";
		case TK_R_DO : return "do";
		case TK_R_WHILE : return "while";
		case TK_R_FOR : return "for";
		case TK_R_BREAK : return "break";
		case TK_R_CONTINUE : return "continue";
		case TK_R_FUNCTION : return "function";
		case TK_R_RETURN : return "return";
		case TK_R_VAR : return "var";
		case TK_R_TRUE : return "true";
		case TK_R_FALSE : return "false";
		case TK_R_NULL : return "null";
		case TK_R_UNDEFINED : return "undefined";
		case TK_R_NEW : return "new";
		case TK_R_TRY: return "try";
		case TK_R_CATCH: return "catch";
		case TK_R_THROW: return "throw";
		case TK_R_FINALLY: return "finally";
		case TK_R_TYPEOF: return "typeof";
		case TK_REGEX: return "REGEX";
	}

	return sReturn + (char)nToken;
}
//------------------------------------------------------------------------------
// Khoi phuc token ve trang thai ban dau
//------------------------------------------------------------------------------
void CTokenPointer::Reset()
{
	m_nDataPos = m_nDataStart;
	m_nTokenStart = 0;
	m_nTokenEnd = 0;
	m_nTokenLastEnd = 0;
	m_nTokenId = 0;
	m_bHadEnterCharacter = false;
	m_sTokenStr = "";
	GetNextCh();
	GetNextCh();
	GetNextToken();
}

//------------------------------------------------------------------------------
// Lay subString = m_szData[nBeginInput - vi tri ket thuc token hien tai]
//------------------------------------------------------------------------------
string CTokenPointer::GetSubString(int nBeginPosition)
{
	int nLastCharIdx = m_nTokenLastEnd+1;
	string sNewSubTkData(m_szData);

	if (nLastCharIdx < m_nDataEnd) 
	{
		sNewSubTkData = sNewSubTkData.substr(nBeginPosition, nLastCharIdx - nBeginPosition);
	} else 
	{
		//return string(&m_szData[beginPosition]);
		sNewSubTkData = sNewSubTkData.substr(nBeginPosition);
	}
	return sNewSubTkData;
}


//------------------------------------------------------------------------------
// get Next token (create New) from  [IN]m_nTokenIndex  of current Token
// current token doesn't change
//------------------------------------------------------------------------------
CTokenPointer *CTokenPointer::GetSubToken(int nBeginPosition)
{
	int nLastCharIdx = m_nTokenLastEnd+1;
	if (nLastCharIdx < m_nDataEnd)
		return new CTokenPointer(this, nBeginPosition, nLastCharIdx);
	else
		return new CTokenPointer(this, nBeginPosition, m_nDataEnd );
}

//------------------------------------------------------------------------------
// get Next token (create New) from  [IN]m_nTokenIndex = m_nDataEnd  of current Token
// current token doesn't change....[NDC]
//------------------------------------------------------------------------------
CTokenPointer *CTokenPointer::GetSubTokenForIn(int nBeginPosition)
{
		return new CTokenPointer(this, nBeginPosition, m_nDataEnd);
}

//------------------------------------------------------------------------------
// vd: bo qua khoi lenh {.......}
//-------------------------------
// Bo qua khong thuc hien lenh giua 2 token: nTypeTk1, nTypeTk2
// lay 1 doan string tron m_szToken.
//------------------------------------------------------------------------------
string	CTokenPointer::GetSubDataEqualLevel(int nTypeTk1, int nTypeTk2)
{
	string sDataOut;
	int nPosStart = m_nTokenEnd-1;
	int nPosEnd = 0;
	int nLevel = 1;

	while (this->m_nTokenId != TK_EOF){
		GetNextToken();
		if (m_nTokenId == nTypeTk1)
			nLevel++;
		else if (m_nTokenId == nTypeTk2)
			nLevel--;
		if (nLevel == 0)
			break;
	}
	nPosEnd = m_nTokenEnd;
	sDataOut = string(m_szData).substr(nPosStart, nPosEnd - nPosStart);
	return sDataOut;
}

//------------------------------------------------------------------------------
// Lay vi tri con tro lenh hien tai
//------------------------------------------------------------------------------
string CTokenPointer::GetPosition(int nPosTk)
{
	char buf[256];
	char ch = 0;
	int nLine = 1, nCol = 1;
	int i = 0;

	if (nPosTk < 0) 
		nPosTk=m_nTokenLastEnd;

	
	for (i = 0; i < nPosTk; i++)
	{
		if (i < m_nDataEnd)
			ch = m_szData[i];
		else
			ch = 0;
		nCol++;
		if (ch == '\n') 
		{
			nLine++;
			nCol = 0;
		}
	}

	_snprintf_s(buf, 256, _TRUNCATE, "(line: %d, col: %d)", nLine, nCol);
	return buf;
}

//------------------------------------------------------------------------------
// Kiem tra ki tu enter có gặp trong quá trình GetNextToken hay khong
//------------------------------------------------------------------------------
bool CTokenPointer::CheckHadEnterCharWhenGetNextToken()
{
	return m_bHadEnterCharacter;
}

//------------------------------------------------------------------------------
// Tim ki tu tiep theo
//------------------------------------------------------------------------------
void CTokenPointer::GetNextCh()
{
	m_currCh = m_nextCh;
	if (m_nDataPos < m_nDataEnd)
	{
		m_nextCh = m_szData[m_nDataPos];
		if (m_nDataPos > 2)
		{
			m_prevCh = m_szData[m_nDataPos - 2];
		}

	}
	else
		m_nextCh = 0;
	m_nDataPos++;
}

//------------------------------------------------------------------------------
// Thu tim token tiep theo & ko lam thay doi gia tri token hien tai
//------------------------------------------------------------------------------
int CTokenPointer::TryWatchNextToken()
{
	int nTypeToken;
	CTokenPointer * tmpToken = new CTokenPointer(this, m_nTokenStart, m_nDataEnd);

	tmpToken->GetNextToken();
	nTypeToken = tmpToken->m_nTokenId;
	return nTypeToken;
}

//------------------------------------------------------------------------------
// Tim token tiep theo
//------------------------------------------------------------------------------
void CTokenPointer::GetNextToken() 
{
	//if(true) writeInfo(s,"tests/outBug/fileJSBug");
	bool bIsHex = false;
	char szHexBuff[3] = "??";
	char szOctBuff[4] = "???";
	char szUnicodeBuff[5] = "????";

	m_nTokenId = TK_EOF;
	m_bHadEnterCharacter = false;
	m_sTokenStr.clear();
	// Bỏ qua các kí tự trắng " " \n \t \r
	while (m_currCh && IsWhitespace(m_currCh))
	{
		if (m_currCh == '\n')
			m_bHadEnterCharacter = true;
		GetNextCh();
	}
	// newline comments
	if (m_currCh == '/' && m_nextCh == '/') 
	{
		while (m_currCh && m_currCh != '\n') GetNextCh();
		GetNextCh();
		GetNextToken();
		m_bHadEnterCharacter = true;
		return;
	}
	// block comments
	if (m_currCh == '/' && m_nextCh == '*') 
	{
		while (m_currCh && (m_currCh != '*' || m_nextCh != '/')) GetNextCh();
		GetNextCh();
		GetNextCh();
		GetNextToken();
		return;
	}
	// record beginning of this token
	m_nTokenStart = m_nDataPos - 2;

	//  IDs
	if (IsAlpha(m_currCh)) 
	{
		while (IsAlpha(m_currCh) || IsNumeric(m_currCh)) 
		{
			m_sTokenStr += m_currCh;
			GetNextCh();
		}
		m_nTokenId = TK_ID;
		if (m_sTokenStr == "if") m_nTokenId = TK_R_IF;
		else if (m_sTokenStr == "else") m_nTokenId = TK_R_ELSE;
		else if (m_sTokenStr == "do") m_nTokenId = TK_R_DO;
		else if (m_sTokenStr == "while") m_nTokenId = TK_R_WHILE;
		else if (m_sTokenStr == "for") m_nTokenId = TK_R_FOR;
		else if (m_sTokenStr == "break") m_nTokenId = TK_R_BREAK;
		else if (m_sTokenStr == "continue") m_nTokenId = TK_R_CONTINUE;
		else if (m_sTokenStr == "function") m_nTokenId = TK_R_FUNCTION;
		else if (m_sTokenStr == "return") m_nTokenId = TK_R_RETURN;
		else if (m_sTokenStr == "var") m_nTokenId = TK_R_VAR;
		else if (m_sTokenStr == "true") m_nTokenId = TK_R_TRUE;
		else if (m_sTokenStr == "false") m_nTokenId = TK_R_FALSE;
		else if (m_sTokenStr == "null") m_nTokenId = TK_R_NULL;
		else if (m_sTokenStr == "undefined") m_nTokenId = TK_R_UNDEFINED;
		else if (m_sTokenStr == "new") m_nTokenId = TK_R_NEW;
		else if (m_sTokenStr == "delete") m_nTokenId = TK_R_DELETE;
		else if (m_sTokenStr == "try") m_nTokenId = TK_R_TRY;
		else if (m_sTokenStr == "catch") m_nTokenId = TK_R_CATCH;
		else if (m_sTokenStr == "throw") m_nTokenId = TK_R_THROW;
		else if (m_sTokenStr == "finally") m_nTokenId = TK_R_FINALLY;
		else if (m_sTokenStr == "in") m_nTokenId = TK_R_IN;
		else if (m_sTokenStr == "typeof") m_nTokenId = TK_R_TYPEOF;
	}
	// Numbers
	else if (IsNumeric(m_currCh)) 
	{
		if (m_currCh == '0') 
		{
			m_sTokenStr += m_currCh;
			GetNextCh();
		}
		if (m_currCh == 'x') 
		{
			bIsHex = true;
			m_sTokenStr += m_currCh;
			GetNextCh();
		}
		m_nTokenId = TK_INT;
		while (IsNumeric(m_currCh) || (bIsHex && IsHexadecimal(m_currCh))) {
			m_sTokenStr += m_currCh;
			GetNextCh();
		}
		if (!bIsHex && m_currCh == '.') 
		{
			m_nTokenId = TK_FLOAT;
			m_sTokenStr += '.';
			GetNextCh();
			while (IsNumeric(m_currCh)) 
			{
				m_sTokenStr += m_currCh;
				GetNextCh();
			}
		}

		// do fancy e-style floating point
		if (!bIsHex && (m_currCh == 'e' || m_currCh == 'E')) 
		{
			m_nTokenId = TK_FLOAT;
			m_sTokenStr += m_currCh; 
			GetNextCh();
			if (m_currCh == '-') 
			{ 
				m_sTokenStr += m_currCh; GetNextCh(); 
			}
			while (IsNumeric(m_currCh)) 
			{
				m_sTokenStr += m_currCh;
				GetNextCh();
			}
		}
	}
	// string regex
	else if ((!IsNumeric(m_prevCh)) 
		&& (m_currCh == '/') 
		&& (m_nTokenId_old != TK_ID) 
		&& (m_prevCh != ')') 
		&& (m_nTokenId_old != TK_FLOAT))
	{
		GetNextCh();		
		while (m_currCh && m_currCh != '/')
		{		
			m_sTokenStr += m_currCh;		
			GetNextCh();
		}
		GetNextCh();
		m_nTokenId = TK_REGEX;
		if ((m_currCh == 'i') || (m_currCh =='g'))
		{
			GetNextCh();
			if ((m_currCh == 'g') && (m_prevCh == 'i') || (m_currCh == 'i') && (m_prevCh == 'g'))
				GetNextCh();		
		}
      
	}
	// strings...
	else if (m_currCh == '"') 
	{
		GetNextCh();
		while (m_currCh && m_currCh != '"')
		{
			if (m_currCh == '\\') 
			{
				GetNextCh();
				switch (m_currCh) 
				{
					case 'n': m_sTokenStr += '\n'; break;
					case '"': m_sTokenStr += '"'; break;
					case '\\': m_sTokenStr += '\\'; break;
					case 'r': m_sTokenStr += '\r'; break; //ndc
						// truong hop chuoi doc duoc dang ascii: \x32
					case 'x':
					{
								// hex digits
								GetNextCh(); szHexBuff[0] = m_currCh;
								GetNextCh(); szHexBuff[1] = m_currCh;
								m_sTokenStr += (char)strtol(szHexBuff, 0, 16);
					}
						break;
						// truong hop chuoi doc duoc la unicode: \u32A2
					case 'u':
					{
						// hex digits
						GetNextCh(); szUnicodeBuff[0] = m_currCh;
						GetNextCh(); szUnicodeBuff[1] = m_currCh;
						GetNextCh(); szUnicodeBuff[2] = m_currCh;
						GetNextCh(); szUnicodeBuff[3] = m_currCh;
						m_sTokenStr += (char)strtol(szUnicodeBuff, 0, 16);
					}
						break;
					default:
						if (m_currCh >= '0' && m_currCh <= '7') 
						{
							// octal digits
							GetNextCh(); szOctBuff[1] = m_currCh;
							GetNextCh(); szOctBuff[2] = m_currCh;
							m_sTokenStr += (char)strtol(szOctBuff, 0, 8);
						}
						else
							m_sTokenStr += m_currCh;
				}
			}
			else 
			{
				m_sTokenStr += m_currCh;
			}
			GetNextCh();
		}
		GetNextCh();
		m_nTokenId = TK_STR;

	}
	else if (m_currCh == '\\' && m_nextCh == '\'') 
	{
		// strings again...
		GetNextCh();
		GetNextCh();
		while (m_currCh && m_nextCh != '\'') 
		{
			if (m_currCh == '\\') 
			{
				GetNextCh();
				switch (m_currCh)
				{
					case 'n': m_sTokenStr += '\n'; break;
					case 'a': m_sTokenStr += '\a'; break;
					case 'r': m_sTokenStr += '\r'; break;
					case 't': m_sTokenStr += '\t'; break;
					case '\'': m_sTokenStr += '\''; break;
					case '\\': m_sTokenStr += '\\'; break;
					case 'x':
					{
								// hex digits
								GetNextCh(); szHexBuff[0] = m_currCh;
								GetNextCh(); szHexBuff[1] = m_currCh;
								m_sTokenStr += (char)strtol(szHexBuff, 0, 16);
					}
						break;

					default:
						if (m_currCh >= '0' && m_currCh <= '7') 
						{
							// octal digits
							GetNextCh(); szOctBuff[1] = m_currCh;
							GetNextCh(); szOctBuff[2] = m_currCh;
							m_sTokenStr += (char)strtol(szOctBuff, 0, 8);
						}
						else
							m_sTokenStr += m_currCh;
				}
			}
			else 
			{
				m_sTokenStr += m_currCh;
			}
			GetNextCh();
		}
		GetNextCh();
		GetNextCh();
		m_nTokenId = TK_STR;
	}

	// strings again...
	else if (m_currCh == '\'') 
	{
		GetNextCh();
		while (m_currCh && m_currCh != '\'')
		{
			if (m_currCh == '\\') 
			{
				GetNextCh();
				switch (m_currCh)
				{
					case 'n': m_sTokenStr += '\n'; break;
					case 'a': m_sTokenStr += '\a'; break;
					case 'r': m_sTokenStr += '\r'; break;
					case 't': m_sTokenStr += '\t'; break;
					case '\'': m_sTokenStr += '\''; break;
					case '\\': m_sTokenStr += '\\'; break;
					case 'x':
					{
								// hex digits
								GetNextCh(); szHexBuff[0] = m_currCh;
								GetNextCh(); szHexBuff[1] = m_currCh;
								m_sTokenStr += (char)strtol(szHexBuff, 0, 16);
					}
						break;

					default:
						if (m_currCh >= '0' && m_currCh <= '7') 
						{
							// octal digits
							szOctBuff[0] = m_currCh;
							GetNextCh(); szOctBuff[1] = m_currCh;
							GetNextCh(); szOctBuff[2] = m_currCh;
							m_sTokenStr += (char)strtol(szOctBuff, 0, 8);
						}
						else
							m_sTokenStr += m_currCh;
				}
			}
			else 
			{
				m_sTokenStr += m_currCh;
			}
			GetNextCh();
		}
		GetNextCh();
		m_nTokenId = TK_STR;
	}
	else 
	{
		// single chars
		m_nTokenId = m_currCh;
		if (m_currCh) GetNextCh();
		if (m_nTokenId == '=' && m_currCh == '=') 
		{ 
			// ==
			m_nTokenId = TK_EQUAL;
			GetNextCh();
			if (m_currCh == '=') 
			{ 
				// ===
				m_nTokenId = TK_TYPEEQUAL;
				GetNextCh();
			}
		}
		else if (m_nTokenId == '!' && m_currCh == '=') 
		{
			// !=
			m_nTokenId = TK_NEQUAL;
			GetNextCh();
			if (m_currCh == '=') 
			{ 
				// !==
				m_nTokenId = TK_NTYPEEQUAL;
				GetNextCh();
			}
		}
		else if (m_nTokenId == '<' && m_currCh == '=') 
		{
			m_nTokenId = TK_LEQUAL;
			GetNextCh();
		}
		else if (m_nTokenId == '<' && m_currCh == '<') 
		{
			m_nTokenId = TK_LSHIFT;
			GetNextCh();
			if (m_currCh == '=') 
			{ 
				// <<=
				m_nTokenId = TK_LSHIFTEQUAL;
				GetNextCh();
			}
		}
		else if (m_nTokenId == '>' && m_currCh == '=') 
		{
			m_nTokenId = TK_GEQUAL;
			GetNextCh();
		}
		else if (m_nTokenId == '>' && m_currCh == '>') 
		{
			m_nTokenId = TK_RSHIFT;
			GetNextCh();
			if (m_currCh == '=') 
			{ 
				// >>=
				m_nTokenId = TK_RSHIFTEQUAL;
				GetNextCh();
			}
			else if (m_currCh == '>') 
			{ 
				// >>>
				m_nTokenId = TK_RSHIFTUNSIGNED;
				GetNextCh();
			}
		}
		else if (m_nTokenId == '+' && m_currCh == '=') 
		{
			m_nTokenId = TK_PLUSEQUAL;
			GetNextCh();
		}
		else if (m_nTokenId == '-' && m_currCh == '=') 
		{
			m_nTokenId = TK_MINUSEQUAL;
			GetNextCh();
		}
		else if (m_nTokenId == '+' && m_currCh == '+') 
		{
			m_nTokenId = TK_PLUSPLUS;
			GetNextCh();
		}
		else if (m_nTokenId == '-' && m_currCh == '-') 
		{
			m_nTokenId = TK_MINUSMINUS;
			GetNextCh();
		}
		else if (m_nTokenId == '&' && m_currCh == '=') 
		{
			m_nTokenId = TK_ANDEQUAL;
			GetNextCh();
		}
		else if (m_nTokenId == '&' && m_currCh == '&') 
		{
			m_nTokenId = TK_ANDAND;
			GetNextCh();
		}
		else if (m_nTokenId == '|' && m_currCh == '=') 
		{
			m_nTokenId = TK_OREQUAL;
			GetNextCh();
		}
		else if (m_nTokenId == '|' && m_currCh == '|') 
		{
			m_nTokenId = TK_OROR;
			GetNextCh();
		}
		else if (m_nTokenId == '^' && m_currCh == '=') 
		{
			m_nTokenId = TK_XOREQUAL;
			GetNextCh();
		}
	}
	m_nTokenId_old = m_nTokenId;

	/* This isn't quite right yet */
	m_nTokenLastEnd = m_nTokenEnd;
	m_nTokenEnd = m_nDataPos - 3;
}

