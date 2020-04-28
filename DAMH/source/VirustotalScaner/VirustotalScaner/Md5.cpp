#include "StdAfx.h"
#include "Md5.h"

CMd5::CMd5(void)
{
}

CMd5::~CMd5(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:	GetMD5
// Description:	Get Md5 hash string of the specified string
// Params:		szInput: input buffer
//				szResult: output string (MD5 hash)
// Return:		void
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMd5::GetMD5(LPCTSTR szInput, LPTSTR szResult)
{
	int iLen;

	iLen = _tcslen(szInput);
	GetMD5Buffer((const BYTE*)szInput, iLen, szResult, 33);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:	GetMD5Buffer
// Description:	Get Md5 hash of the specified buffer
// Params:		buffer: input buffer
//				dwBufferLen: buffer length (in byte)
//				szMD5Buffer: output string (MD5 hash)
//				dwMD5BufferMaxSize: maximum size of output buffer (in character), including the NULL-terminator
// Return:		void
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMd5::GetMD5Buffer(const BYTE* buffer, DWORD dwBufferLen, LPTSTR szMD5Buffer, DWORD dwMD5BufferMaxSize)
{
	HCRYPTPROV	hCryptProv = NULL;
	HCRYPTHASH	hHash = NULL;
	BYTE		pMd5Hash[50];
	TCHAR		szHash[20];
	DWORD		dwMd5HashSize;
	DWORD		dwSize;
	DWORD		i;
	const DWORD	dwMaxMd5HashSize = 16;

	// ================================================
	// Data validation
	if (!szMD5Buffer || dwMD5BufferMaxSize <= 1) return FALSE;

	// ================================================
	// Get a handle to a cryptography provider context
	if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) || !hCryptProv)  
	{
		return FALSE;
	}

	// ================================================
	// Create hash object
	if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)) 
	{
		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);
		return FALSE;
	}

	// ================================================
	// Add data to hash object
	if (!CryptHashData(hHash, buffer, dwBufferLen, CRYPT_USERDATA))
	{
		// Release hash object
		CryptDestroyHash(hHash);

		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);
		return FALSE;
	}

	// ================================================
	// Get hash size
	dwSize = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHSIZE, (LPBYTE)&dwMd5HashSize, &dwSize, 0) || dwMd5HashSize != dwMaxMd5HashSize)
	{
		// Release hash object
		CryptDestroyHash(hHash);

		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);
		return FALSE;
	}

	// Get hash data
	dwSize = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHVAL, (LPBYTE)pMd5Hash, &dwMd5HashSize, 0))
	{
		// Release hash object
		CryptDestroyHash(hHash);

		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);
		return FALSE;
	}

	// Convert hash data to text
	memset(szMD5Buffer, NULL, dwMD5BufferMaxSize * sizeof(TCHAR));
	dwSize = dwMD5BufferMaxSize - 1;
	for (i = 0; i < dwMd5HashSize; i++)
	{
		_stprintf_s(szHash,sizeof(szHash)/2, _T("%02X"), pMd5Hash[i]);
		_tcsncat_s(szMD5Buffer,dwMD5BufferMaxSize, szHash, dwSize);
		if (dwSize <= 2) break;
		dwSize -= 2;
	}

	// ================================================
	// Clean up
	// Release hash object
	CryptDestroyHash(hHash);

	// Release cryptography provider context
	CryptReleaseContext(hCryptProv, 0);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:	GetMD5Buffer
// Description:	Get Md5 hash of the specified file
// Params:		szFileName: input file name
//				szMD5Buffer: output string (MD5 hash)
//				dwMD5BufferMaxSize: maximum size of output buffer (in character), including the NULL-terminator
// Return:		void
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMd5::GetMD5File(LPCTSTR szFileName, LPTSTR szMD5Buffer, DWORD dwMD5BufferMaxSize)
{
	HCRYPTPROV	hCryptProv = NULL;
	HCRYPTHASH	hHash = NULL;
	TCHAR		szHash[20];
	DWORD		dwMd5HashSize;
	DWORD		dwSize, i;
	const DWORD	dwMaxMd5HashSize = 16;
	BYTE		pMd5Hash[20];
	BYTE		buffer[2048];
	DWORD		dwBufferLen;
	FILE*		pFile;

	//Check file size 0
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	
	DWORD dwFileSize = GetFileSize(hFile, NULL);
	if (dwFileSize == 0)
	{
		CloseHandle(hFile);

		_tcscpy_s(szMD5Buffer, dwMD5BufferMaxSize, _T("D41D8CD98F00B204E9800998ECF8427E"));

		return TRUE;
	}
	CloseHandle(hFile);

	// ================================================
	// Data validation
	if (!szMD5Buffer || dwMD5BufferMaxSize <= 1) return FALSE;

	// ================================================
	// Get a handle to a cryptography provider context
	if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) || !hCryptProv)  
	{
		return FALSE;
	}

	// ================================================
	// Create hash object
	if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)) 
	{
		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);
		return FALSE;
	}

	// ================================================
	// Open file for reading
	if (_tfopen_s(&pFile,szFileName, _T("rb")))
	{
		return FALSE;
	}
	
	if (!pFile)
	{
		// Release hash object
		CryptDestroyHash(hHash);

		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);
		return FALSE;
	}

	while (TRUE)
	{
		// Read data from file and store to buffer
		dwBufferLen = fread(buffer, sizeof(BYTE), 2048, pFile);

		if (!dwBufferLen)
		{
			// Cannot read file, detect error
			if (feof(pFile))
			{
				// End-of-file detected, finish reading data
				break;
			}
			else
			{
				// Otherwise error detected, return
				// Release hash object
				CryptDestroyHash(hHash);

				// Release cryptography provider context
				CryptReleaseContext(hCryptProv, 0);

				// Close file
				fclose(pFile);
				return FALSE;
			}
		}

		// ================================================
		// Add data to hash object
		if (!CryptHashData(hHash, buffer, dwBufferLen, CRYPT_USERDATA))
		{
			// Release hash object
			CryptDestroyHash(hHash);

			// Release cryptography provider context
			CryptReleaseContext(hCryptProv, 0);

			// Close file
			fclose(pFile);
			return FALSE;
		}
	}	


	// Close file
	fclose(pFile);

	// ================================================
	// Get hash size
	dwSize = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHSIZE, (LPBYTE)&dwMd5HashSize, &dwSize, 0) || dwMd5HashSize != dwMaxMd5HashSize)
	{
		// Release hash object
		CryptDestroyHash(hHash);

		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);

		return FALSE;
	}

	// Get hash data
	dwSize = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHVAL, (LPBYTE)pMd5Hash, &dwMd5HashSize, 0))
	{
		// Release hash object
		CryptDestroyHash(hHash);

		// Release cryptography provider context
		CryptReleaseContext(hCryptProv, 0);

		return FALSE;
	}

	// Convert hash data to text
	memset(szMD5Buffer, NULL, dwMD5BufferMaxSize * sizeof(TCHAR));
	dwSize = dwMD5BufferMaxSize - 1;
	for (i = 0; i < dwMd5HashSize; i++)
	{
		_stprintf_s(szHash,sizeof(szHash)/2, _T("%02X"), pMd5Hash[i]);
		_tcsncat_s(szMD5Buffer,dwMD5BufferMaxSize, szHash, dwSize);
		if (dwSize <= 2) break;
		dwSize -= 2;
	}


	// ================================================
	// Clean up
	// Release hash object
	CryptDestroyHash(hHash);

	// Release cryptography provider context
	CryptReleaseContext(hCryptProv, 0);

	return TRUE;
}