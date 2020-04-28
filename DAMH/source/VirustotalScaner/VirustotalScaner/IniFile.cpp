#include "StdAfx.h"
#include "IniFile.h"

CIniFile::~CIniFile(void)
{
}

CIniFile::CIniFile()
: m_sFileName(_T(""))
{
}

CIniFile::CIniFile(LPCTSTR lpIniFileName)
: m_sFileName(_T(""))
{
	SetFileName(lpIniFileName);
}

// Set file name function
void CIniFile::SetFileName(LPCTSTR pszIniFileName)
{
	m_sFileName = pszIniFileName;
}

// Write string to file function
BOOL CIniFile::WriteString(__in LPCTSTR pszSection, __in LPCTSTR pszKey, __in LPCTSTR pszString)
{
	return ::WritePrivateProfileString(pszSection, pszKey, pszString, m_sFileName);
}

//	Read string from file
BOOL CIniFile::GetString(LPCTSTR lpSection, LPCTSTR lpKey, CString& strGet, LPCTSTR lpDefault)
{
	ASSERT(lpDefault != NULL);
	ASSERT(_tcslen(lpDefault) < MAX_PATH);
	return CIniFile::GetString(lpSection, lpKey, strGet, lpDefault, MAX_PATH);
}

// Read string from file (help function)
BOOL CIniFile::GetString(LPCTSTR lpSection, LPCTSTR lpKey, CString& strGet, LPCTSTR lpDefault,const DWORD nSize)
{
	LPTSTR pBuffer;
	TRY
	{
		pBuffer = strGet.GetBuffer(nSize);		
	}
	CATCH(CMemoryException, pExc)	// GetBuffer() can out put error exception memmory
	{
		THROW(pExc);				// Out put error
		return FALSE;
	}
	END_CATCH

		memcpy(pBuffer + nSize - 2, "xx",2);	// initialize the last 2 characters on the buffer with something other than \0
	DWORD iRet = ::GetPrivateProfileString(lpSection, lpKey, lpDefault, pBuffer, nSize, m_sFileName);
	ASSERT(iRet < nSize);
	BOOL bRet = (memcmp(pBuffer + nSize - 2, "\0\0", 2) != 0);	// check the last 2 characters of the buffer.
	strGet.ReleaseBuffer();	
	return bRet;
}

// Read integer from file
int CIniFile::GetInt(__in LPCTSTR pszSection, __in LPCTSTR pszKey)
{
	return GetPrivateProfileInt(pszSection, pszKey, -1, m_sFileName);
}
