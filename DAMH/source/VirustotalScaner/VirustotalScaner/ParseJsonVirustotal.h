#pragma once
#include <string.h>
#include <iostream>

#define ERROR_PARSE 100
#define NO_SCAN_RESOURCE	2
#define MAX_SIZE_LOG 5000

using namespace std;

class ParseJsonVirustotal
{
private:
	CHAR* m_szBufferLog;

public:
	ParseJsonVirustotal(void);
	~ParseJsonVirustotal(void);

	DWORD  ParseJsonToArray(CString json, CHAR* pszBuffer, DWORD dwSize);
	CString GetSanId(CString json);
};
