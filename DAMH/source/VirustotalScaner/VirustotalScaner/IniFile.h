#pragma once
#include "afx.h"

class CIniFile :public CObject
{
public:
	// Constructors
	CIniFile();
	CIniFile(LPCTSTR lpIniFileName);
	~CIniFile(void);

protected:
	// Attributes
	CString m_sFileName;		// Filename

public:
	// Get file name
	CString	GetIniFileName();
	// Pass file name
	void SetFileName(LPCTSTR pszIniFileName);
	// Write string to file
	BOOL WriteString(__in LPCTSTR pszSection, __in LPCTSTR pszKey, __in LPCTSTR pszString);
	// Read string from file
	BOOL GetString(__in LPCTSTR lpSection, __in LPCTSTR lpKey, __inout CString& strGet, __in LPCTSTR lpDefault);
	// Read integer from file
	int GetInt( __in LPCTSTR pszSection, __in LPCTSTR pszKey);
	
protected:
	BOOL GetString(__in LPCTSTR lpSection, __in LPCTSTR lpKey, __inout CString& strGet, __in LPCTSTR lpDefault, __in const DWORD nSize);
};
