#pragma once


#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <Wincrypt.h>

#pragma comment (lib, "Advapi32.lib")


class CMd5
{
public:
	CMd5(void);
	~CMd5(void);
	BOOL	GetMD5Buffer(const BYTE* buffer, DWORD dwBufferLen, LPTSTR szMD5Buffer, DWORD dwMD5BufferMaxSize);
	BOOL	GetMD5File(LPCTSTR szFileName, LPTSTR szMD5Buffer, DWORD dwMD5BufferMaxSize);
	void	GetMD5(LPCTSTR szInput, LPTSTR szResult);
};
