#pragma once
#define NAME_LOG_VIRUS    _T("Log.log")

#define  SZ_SERVER_VIRUSTOTAL		_T("www.virustotal.com")
#define  SZ_OBJECT_SCAN_RESOURCE	_T("/api/get_file_report.json")
#define  SZ_OBJECT_UPLOAD_FILE		_T("/api/scan_file.json")

#define  MAX_SIZE_UPLOAD			5*1024*1024

class CVirustotal
{
private:
	CString m_sKey;
	CString m_sDirScan;

public:
	CVirustotal(void);
	CVirustotal(CString sKey);
	~CVirustotal(void);

	void SetDirScan(CString sDir);
	void SetApiKey(CString sKey);
	DWORD  ScanResource(CString sPathFile, CString sMd5, CString sResource);
	BOOL WriteFileLog(CString sMd5Folder, CHAR*pszBuffer, DWORD dwSize, CString sPathFile);
	CString SendResourceAndGetReponse(CString sResurce);
	CString UploadFile(CString sPath);
};
