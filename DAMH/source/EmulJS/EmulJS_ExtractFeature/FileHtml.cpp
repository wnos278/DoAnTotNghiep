////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac phuong thuc cua lop CFileHtml
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include "FileHtml.h"
#include "DATA_DEF.h"
#include <algorithm>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

int getPosStr(string szData, string szTagName, int nPosStart);

int findPosErr(int nLine, string sData);
void fixDelBeforeTag(int nPosChar, string &sData);
bool fixValueInQuocte(long lPosLine, long lPosColumn, string &sData);
bool fixAttributeDuplicate(long lPosLine, long lPosColumn, string &sData);
bool fixWhiteSpaceNotAllow(long lPosLine, long lPosColumn, string &sData);
bool fixNoEndTag(long lPosLine, long lPosColumn, string &sData, BSTR bstrErr);
bool fixNameTagFail(long lPosLine, long lPosColumn, string &sData);

CFileHtml::CFileHtml()
{
}



CFileHtml::~CFileHtml(void)
{
}

#ifdef DEVELOP_HTML

// Create Variant from PCWSTR szFileXMLname
HRESULT CFileHtml::VariantFromString(PCWSTR wszValue, VARIANT &Variant)
{
	HRESULT hr = S_OK;
	BSTR bstr = SysAllocString(wszValue);
	CHK_ALLOC(bstr);

	V_VT(&Variant) = VT_BSTR;
	V_BSTR(&Variant) = bstr;

CleanUp:
	return hr;
}


CFileHtml::CFileHtml(wstring wszPathFile)
{
	BOOL bRead = FALSE;
	HANDLE hFile = 0;
	DWORD dwByteHigh = 0;
	DWORD dwByteRead = 0;
	BYTE* szDataHtml = NULL;

	if (wszPathFile.size() > 0)
	{
		// set new pathFile
		this->m_wsPathFile = wszPathFile;

		hFile = CreateFileW(m_wsPathFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			// get File size
			this->m_dwSizeFileHtml = 0;

			this->m_dwSizeFileHtml = GetFileSize(hFile, &dwByteHigh);
			if (this->m_dwSizeFileHtml > 0 && dwByteHigh == 0)
			{
				// create "size" byte for buff "dataFile"
				szDataHtml = new BYTE[this->m_dwSizeFileHtml + 1];
				szDataHtml[this->m_dwSizeFileHtml] = 0;

				// read data File to buff
				bRead = ReadFile(hFile, szDataHtml, this->m_dwSizeFileHtml, &dwByteRead, NULL);
				if (bRead)
				{
					m_sCodeHtml = (char*)szDataHtml;

					// get out + remove code js from file HTML in mem
					m_lstsCodeJs = CFileHtml::c_FiltersOutCodeJs(m_sCodeHtml, false);

					// prefix code html -> DOM parse
					//this->prefixHtml(m_szCodeHtml);
				}

				SAFE_DELETE(szDataHtml);
				this->m_sDocumentData = "";
			}
			else
			{
				//
				//... process file too large 
				CloseHandle(hFile);
				throw new CRuntimeException("Error: sizeFile!! File size is too large to read!",
					READ_FILE_FALSE);
			}

			CloseHandle(hFile);
		}
		else
			throw new CRuntimeException("Error: szPathFile!! File doesn't exist!",
			CREATE_FILE_FALSE);
	}
}

string CFileHtml::GetCodeJs()	// get Code Script
{
	return m_sCodeJs;
}

vector <string> CFileHtml::GetListCodeJs()	// get Code Script
{
	return m_lstsCodeJs;
}

// Map File xml into XML DOM Document: pXMLDom
HRESULT CFileHtml::LoadXMLFile(IXMLDOMDocument *pXMLDom, LPCWSTR lpszXMLFile)
{
	HRESULT hr = S_OK;
	VARIANT_BOOL varStatus;
	VARIANT varFileName;
	IXMLDOMParseError *pXMLErr = NULL;
	BSTR bstrErr = NULL;
	BSTR srcTxt = NULL;

	long errPos = -1;
	long errID = -1;

	VariantInit(&varFileName);
	CHK_HR(VariantFromString(lpszXMLFile, varFileName));
	CHK_HR(pXMLDom->load(varFileName, &varStatus));

	//load xml failed
	if (varStatus != VARIANT_TRUE)
	{
		hr = E_FAIL;
		CHK_HR(pXMLDom->get_parseError(&pXMLErr));
		CHK_HR(pXMLErr->get_line(&errPos));
		CHK_HR(pXMLErr->get_linepos(&errPos));
		CHK_HR(pXMLErr->get_filepos(&errPos));

		CHK_HR(pXMLErr->get_errorCode(&errID));
		CHK_HR(pXMLErr->get_srcText(&srcTxt));

		CHK_HR(pXMLErr->get_reason(&bstrErr));
		printf("Failed to load %S:\n%S\n", lpszXMLFile, bstrErr);
	}

CleanUp:
	SAFE_RELEASE(pXMLErr);
	SysFreeString(bstrErr);
	VariantClear(&varFileName);
	return hr;
}


void CFileHtml::PrefixHtml(string &szCodeHtml)
{
	HRESULT hr = CoInitialize(NULL);
	bool t;
	if (SUCCEEDED(hr))
	{
		//queryNodes();
		//this->LoadDOMRaw(szCodeHtml);
		t = CheckValidateHTML(szCodeHtml);
		CoUninitialize();
	}
	//system("pause");
}

bool CFileHtml::CheckValidateHTML(string szCodeHtmlCheck)
{
	HRESULT hr = S_OK;
	IXMLDOMDocument *pXMLDom = NULL;
	IXMLDOMParseError *pXMLErr = NULL;

	BSTR bstrXML = NULL;
	BSTR bstrErr = NULL;

	VARIANT_BOOL varStatus = VARIANT_FALSE;

	CHK_HR(CreateAndInitDOM(&pXMLDom));
	bstrXML = SysAllocString(wstring(szCodeHtmlCheck.begin(), szCodeHtmlCheck.end()).c_str());
	CHK_ALLOC(bstrXML);
	CHK_HR(pXMLDom->loadXML(bstrXML, &varStatus));

CleanUp:
	SAFE_RELEASE(pXMLDom);
	SAFE_RELEASE(pXMLErr);
	SysFreeString(bstrXML);
	SysFreeString(bstrErr);

	if (varStatus == VARIANT_TRUE)
	{
		return true;
	}
	else
	{
		return false;
	}

}


void CFileHtml::LoadDOMRaw(string &szCodeHtml)
{
	HRESULT hr = S_OK;
	IXMLDOMDocument *pXMLDom = NULL;
	IXMLDOMParseError *pXMLErr = NULL;

	BSTR bstrXML = NULL;
	BSTR bstrErr = NULL;
	BSTR srcTxt = NULL;
	long lErrId = -1;
	long lErrLine = -1;
	long lErrLinePos = -1;

	VARIANT_BOOL varStatus;

	CHK_HR(CreateAndInitDOM(&pXMLDom));
	// -------------------------------------------------------------------------

	do{
		// move "</body>\n</html>" to last file
		// 		if(lErrId == 0xc00ce555)
		// 		{
		// 			// have element out of <html></html>
		// 			szCodeHtml+= "\n</body>\n</html>";
		// 			posChar = findPosErr(lErrLine, szCodeHtml);
		// 			posChar = szCodeHtml.find_last_of("html",posChar);
		// 
		// 			if( posChar != string::npos)
		// 			{
		// 				fixDelBeforeTag(posChar, szCodeHtml);
		// 			}
		// 
		// 			posChar = szCodeHtml.find_last_of("body",posChar);
		// 			if( posChar != string::npos)
		// 			{
		// 				fixDelBeforeTag(posChar, szCodeHtml);
		// 			}
		// 			int k =1;
		// 		}
		// 		else if(lErrId == 0xc00ce504 || lErrId == 0xc00ce556)
		// 		{
		// 			//fixNameTagFail(lErrLine,lErrLinePos,szCodeHtml);
		// 			// start with character != <html>
		// 			posChar = szCodeHtml.find("<html");
		// 			szCodeHtml = szCodeHtml.substr(posChar,szCodeHtml.size()-posChar);
		// 		}
		// 		else
		if (lErrId == 0xc00ce502)
		{
			// value of attribute not in " "
			if (!fixValueInQuocte(lErrLine, lErrLinePos, szCodeHtml))
			{
				lErrId = -1;
				goto CleanUp;
			}
		}
		else if (lErrId == 0xc00ce554)
		{
			// dupplicate attribute
			if (!fixAttributeDuplicate(lErrLine, lErrLinePos, szCodeHtml))
			{
				lErrId = -1;
				goto CleanUp;
			}
		}
		else if (lErrId == 0xc00ce513)
		{
			// white space is not Allow
			if (!fixWhiteSpaceNotAllow(lErrLine, lErrLinePos, szCodeHtml))
			{
				lErrId = -1;
				goto CleanUp;
			}
		}
		// 		else if(lErrId == 0xc00ce56d)
		// 		{
		// 			//  tag no Close
		// 			if(!fixNoEndTag(lErrLine,lErrLinePos,szCodeHtml, bstrErr))
		// 			{
		// 				lErrId =-1;
		// 				goto CleanUp;
		// 			}
		// 		}
		else if (lErrId != -1)
		{
			// white space is not Allow
			int t = 1;
			if (!fixNameTagFail(lErrLine, lErrLinePos, szCodeHtml))
			{
				lErrId = -1;
				goto CleanUp;
			}
		}
		// XML file name to load

		bstrXML = SysAllocString(wstring(szCodeHtml.begin(), szCodeHtml.end()).c_str());
		CHK_ALLOC(bstrXML);
		CHK_HR(pXMLDom->loadXML(bstrXML, &varStatus));

		if (varStatus == VARIANT_TRUE)
		{
			int t = 1;
		}
		else
		{
			// Failed to load xml, get last parsing error
			CHK_HR(pXMLDom->get_parseError(&pXMLErr));
			CHK_HR(pXMLErr->get_errorCode(&lErrId));
			CHK_HR(pXMLErr->get_line(&lErrLine));
			CHK_HR(pXMLErr->get_linepos(&lErrLinePos));
			CHK_HR(pXMLErr->get_srcText(&srcTxt));
			CHK_HR(pXMLErr->get_reason(&bstrErr));
			printf("Failed to load DOM from stocks.xml. %S\n", bstrErr);
		}

	CleanUp:
		SAFE_RELEASE(pXMLErr);
		SysFreeString(bstrXML);
		SysFreeString(bstrErr);

	} while (lErrId != -1);

	SAFE_RELEASE(pXMLDom);
}

#endif // DEVELOP_HTML

//------------------------------------------------------------------------------
// get Code Script 
// bIsRemoveCodeJs = TRUE: delete code java script from "szCodeHtml"
// bIsRemoveCodeJs = FALSE:  not delete code java script from "szCodeHtml"
//------------------------------------------------------------------------------
string CFileHtml::c_FilterOutCodeJs(string &sCodeHtml, bool bIsRemoveCodeJs)
{
	string sCodeScript;
	vector<string> lstSzCodeJs = c_FiltersOutCodeJs(sCodeHtml, bIsRemoveCodeJs);
	int nSize = lstSzCodeJs.size();
	int i = 0;

	for (i = 0; i < nSize; i++)
		sCodeScript += lstSzCodeJs.at(i);

	return sCodeScript;
}

//------------------------------------------------------------------------------
// get Code Script -> Array[] : Array[i] = code in Element[i] <script>..</script> 
// bIsRemoveCodeJs = TRUE: delete code java script from "szCodeHtml"
// bIsRemoveCodeJs = FALSE:  not delete code java script from "szCodeHtml"
//------------------------------------------------------------------------------
vector<string> CFileHtml::c_FiltersOutCodeJs(string &sCodeHtml, bool bIsRemoveCodeJs)
{
	string sCodeScript;
	vector<string> lstSzCodeScript;
	int nPosTagScriptStart = -1;
	int nPosTagScriptEnd = -1;
	int nPosStart = -1;
	int nPosEnd = -1;
	int nSizeCodeJSRemove = 0;
	int nPosSignComment = 0;
	int nPosArrayObject = 0;
	int nSize;


	do
	{
		if (bIsRemoveCodeJs)
			nPosEnd -= nSizeCodeJSRemove;
		nPosStart = getPosStr(sCodeHtml, "<script", nPosEnd + 1);
		nPosTagScriptStart = nPosStart;
		if (nPosStart != string::npos)
		{
			nPosStart = sCodeHtml.find('>', nPosStart);

			if (nPosStart != string::npos)
			{
				nPosStart++;
				nPosEnd = getPosStr(sCodeHtml, "</script", nPosStart);
				if (nPosEnd != string::npos)
					nPosTagScriptEnd = sCodeHtml.find('>', nPosEnd + 1);
			}
			else break;
		}
		else
			break;

		
		if (nPosStart != string::npos && nPosEnd != string::npos)
		{
			if ((nPosEnd - nPosStart) > 0)
			{
				sCodeScript = sCodeHtml.substr(nPosStart, nPosEnd - nPosStart) + ";";

				// loc bo xau "<!--"
				nPosSignComment = 0;
				do
				{
					nPosSignComment = sCodeScript.find("<!--", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						// xoa bo <!-- .... \n
						nSize = sCodeScript.size();
						while (nPosSignComment < nSize &&
							sCodeScript.at(nPosSignComment) != '\n')
						{
							sCodeScript.at(nPosSignComment) = ' ';
							nPosSignComment++;
						}
					}
				} while (nPosSignComment != string::npos);

				// loc bo xau "-->"
				nPosSignComment = 0;
				do
				{
					nPosSignComment = sCodeScript.find("-->", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						sCodeScript.at(nPosSignComment) = ' ';
						sCodeScript.at(nPosSignComment + 1) = ' ';
						sCodeScript.at(nPosSignComment + 2) = ' ';
					}
				} while (nPosSignComment != string::npos);

				nPosSignComment = 0;
				do
				{
					nPosSignComment = sCodeScript.find("/*@cc_o", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						// xoa bo /*@cc_on
						nSize = sCodeScript.size();
						while (nPosSignComment < nSize &&
							sCodeScript.at(nPosSignComment) != ' '&&sCodeScript.at(nPosSignComment) != '\n')
						{
							sCodeScript.at(nPosSignComment) = ' ';
							nPosSignComment++;
						}
					}
				} while (nPosSignComment != string::npos);

				// loc bo xau "@*/"
				nPosSignComment = 0;
				do
				{
					nPosSignComment = sCodeScript.find("@*/", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						sCodeScript.at(nPosSignComment) = ' ';
						sCodeScript.at(nPosSignComment + 1) = ' ';
						sCodeScript.at(nPosSignComment + 2) = ' ';
					}
				} while (nPosSignComment != string::npos);


				nPosSignComment = 0;
				do{
					nPosSignComment = sCodeScript.find("<![CDATA[", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						// xoa bo <![CDATA[
						nSize = sCodeScript.size();
						while (nPosSignComment < nSize 
							&& sCodeScript.at(nPosSignComment) != ' '
							&& sCodeScript.at(nPosSignComment) != '\n')
						{
							sCodeScript.at(nPosSignComment) = ' ';
							nPosSignComment++;
						}
					}
				} while (nPosSignComment != string::npos);

				// loc bo xau "]]>"
				nPosSignComment = 0;
				do
				{
					nPosSignComment = sCodeScript.find("]]>", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						sCodeScript.at(nPosSignComment) = ' ';
						sCodeScript.at(nPosSignComment + 1) = ' ';
						sCodeScript.at(nPosSignComment + 2) = ' ';
					}
				} while (nPosSignComment != string::npos);

				lstSzCodeScript.push_back(sCodeScript);
			}

			if (bIsRemoveCodeJs && nPosTagScriptStart != string::npos && nPosTagScriptEnd != string::npos)
			{
				// xoa tu vi tri bat dau the <script ..abc..> => </script>
				sCodeHtml.erase(nPosTagScriptStart, nPosTagScriptEnd + 1 - nPosTagScriptStart);
				nSizeCodeJSRemove = nPosTagScriptEnd - nPosTagScriptStart + 1;
				nPosEnd = nSizeCodeJSRemove + nPosTagScriptStart - 1;
			}
			else
			{
				if (bIsRemoveCodeJs)
					sCodeHtml.erase(nPosStart, nPosEnd - nPosStart);
				nSizeCodeJSRemove = nPosEnd - nPosStart;
			}
		}
	} while (nPosStart != string::npos && nPosEnd != string::npos);

	if (lstSzCodeScript.size() == 0)
	{
		nPosSignComment = 0;
		do
		{
			nPosSignComment = sCodeHtml.find("/*@cc_o", nPosSignComment);
			if (nPosSignComment != string::npos)
			{
				// xoa bo /*@cc_on
				nSize = sCodeHtml.size();
				while (nPosSignComment < nSize 
					&& sCodeHtml.at(nPosSignComment) != ' ' 
					&& sCodeHtml.at(nPosSignComment) != '\n')
				{
					sCodeHtml.at(nPosSignComment) = ' ';
					nPosSignComment++;
				}
			}
		} while (nPosSignComment != string::npos);

		// loc bo xau "@*/"
		nPosSignComment = 0;
		do
		{
			nPosSignComment = sCodeHtml.find("@*/", nPosSignComment);
			if (nPosSignComment != string::npos)
			{
				sCodeHtml.at(nPosSignComment) = ' ';
				sCodeHtml.at(nPosSignComment + 1) = ' ';
				sCodeHtml.at(nPosSignComment + 2) = ' ';
			}
		} while (nPosSignComment != string::npos);

		lstSzCodeScript.push_back(sCodeHtml);
	}

	//// Xu ly chia chuoi o cho nay
	//vector<string> lstSzCodeJSReturn;
	//for (int i = 0; i < lstSzCodeScript.size(); i++)
	//{
	//	string codejs = lstSzCodeScript.at(i);
	//	int index = 0;
	//	// Gioi han cu 50 dong code thi se cat ra mot file rieng
	//	int limit_code = 30;
	//	int numChar = 0;
	//	int old_index = -1;
	//	while (codejs[index] != '\0')
	//	{

	//		if (codejs[index] == ';')
	//			numChar += 1;
	//		if (numChar == limit_code)
	//		{
	//			// cat code tu doan nay
	//			lstSzCodeJSReturn.push_back(codejs.substr(old_index+1, index+1));
	//			old_index = index;
	//			numChar = 0;
	//		}
	//		index += 1;
	//	}
	//	lstSzCodeJSReturn.push_back(codejs.substr(old_index+1, index));
	//}

	return lstSzCodeScript;
}
//------------------------------------------------------------------------------
// get Code Script -> Array[] : Array[i] = code in Element[i] <job>..</job> 
// bIsRemoveCodeJs = TRUE: delete code java script from "szCodeHtml"
// bIsRemoveCodeJs = FALSE:  not delete code java script from "szCodeHtml"
// Noi cac doan <script> trong  <job> ... [NDC]
//------------------------------------------------------------------------------
vector<string> CFileHtml::c_FiltersOutCodeJsJob(string &sCodeHtml, bool bIsRemoveCodeJsJob)
{
	string sCodeScriptJob;
	string sCodeScript;
	vector<string> lstSzCodeScript;
	int nPosTagJobStart = -1;
	int nPosTagJobEnd = -1;
	int nPosStart = -1;
	int nPosEnd = -1;
	int nSizeCodeJSRemove = 0;
	int nPosSignComment = 0;
	int nPosArrayObject = 0;
	int nSize;
	bool bIsRemoveCodeJs2 = false;

	do
	{
		if (bIsRemoveCodeJsJob)
			nPosEnd -= nSizeCodeJSRemove;
		nPosStart = getPosStr(sCodeHtml, "<job", nPosEnd + 1);
		nPosTagJobStart = nPosStart;
		if (nPosStart != string::npos)
		{
			nPosStart = sCodeHtml.find('>', nPosStart);

			if (nPosStart != string::npos)
			{
				nPosStart++;
				nPosEnd = getPosStr(sCodeHtml, "</job", nPosStart);
				if (nPosEnd != string::npos)
					nPosTagJobEnd = sCodeHtml.find('>', nPosEnd + 1);
			}
			else 
				break;
		}
		else
			break;

		if (nPosStart != string::npos && nPosEnd != string::npos)
		{
			if ((nPosEnd - nPosStart) > 0)
			{
				sCodeScriptJob = sCodeHtml.substr(nPosStart, nPosEnd - nPosStart) + ";";
				sCodeScript = c_FilterOutCodeJs(sCodeScriptJob, bIsRemoveCodeJs2);
				nPosSignComment = 0;
				do
				{
					nPosSignComment = sCodeScript.find("/*@cc_o", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						// xoa bo /*@cc_on
						nSize = sCodeScript.size();
						while (nPosSignComment < nSize 
							&&sCodeScript.at(nPosSignComment) != ' '
							&& sCodeScript.at(nPosSignComment) != '\n')
						{
							sCodeScript.at(nPosSignComment) = ' ';
							nPosSignComment++;
						}
					}
				} while (nPosSignComment != string::npos);

				// loc bo xau "@*/"
				nPosSignComment = 0;
				do
				{
					nPosSignComment = sCodeScript.find("@*/", nPosSignComment);
					if (nPosSignComment != string::npos)
					{
						sCodeScript.at(nPosSignComment) = ' ';
						sCodeScript.at(nPosSignComment + 1) = ' ';
						sCodeScript.at(nPosSignComment + 2) = ' ';
					}
				} while (nPosSignComment != string::npos);


				lstSzCodeScript.push_back(sCodeScript);
			}

			if (bIsRemoveCodeJsJob && nPosTagJobStart != string::npos && nPosTagJobEnd != string::npos)
			{
				// xoa tu vi tri bat dau the <job..abc..> => </job>
				sCodeHtml.erase(nPosTagJobStart, nPosTagJobEnd + 1 - nPosTagJobStart);
				nSizeCodeJSRemove = nPosTagJobEnd - nPosTagJobStart + 1;
				nPosEnd = nSizeCodeJSRemove + nPosTagJobStart - 1;
			}
			else
			{
				if (bIsRemoveCodeJsJob)
					sCodeHtml.erase(nPosStart, nPosEnd - nPosStart);
				nSizeCodeJSRemove = nPosEnd - nPosStart;
			}
		}
	} while (nPosStart != string::npos && nPosEnd != string::npos);

	// Xu ly chia chuoi o cho nay
	vector<string> lstSzCodeJSReturn;
	for (int i = 0; i < lstSzCodeScript.size(); i++)
	{
		string codejs = lstSzCodeScript.at(i);
		int index = 0;
		// Gioi han cu 50 dong code thi se cat ra mot file rieng
		int limit_code = 50;
		int numChar = 0;
		int old_index = 0;
		while (codejs[index] != '\0')
		{
			
			if (codejs[index] == ';')
				numChar += 1;
			if (numChar == limit_code)
			{
				// cat code tu doan nay
				lstSzCodeJSReturn.push_back(codejs.substr(old_index, index));
				old_index = index;
				numChar = 0;
			}
		}
		lstSzCodeJSReturn.push_back(codejs.substr(old_index, index));
	}

	return lstSzCodeJSReturn;
}


//------------------------------------------------------------------------------
// get Code Script -> Array[] : Array[i] = code in Element[i] <script>..</script> 
// bIsRemoveCodeJs = TRUE: delete code java script from "szCodeHtml"
// bIsRemoveCodeJs = FALSE:  not delete code java script from "szCodeHtml"
//------------------------------------------------------------------------------
vector<string> CFileHtml::FiltersOutCodeJs(bool bIsRemoveCodeJs)
{
	return c_FiltersOutCodeJs(m_sCodeHtml, bIsRemoveCodeJs);
}

// get Code Script -> Array[] : Array[i] = code in Element[i] <job>..</job> 
// bIsRemoveCodeJsJob = TRUE: delete code java script from "szCodeHtml"
// bIsRemoveCodeJsJob = FALSE:  not delete code java script from "szCodeHtml"
//------------------------------------------------------------------------------
vector<string> CFileHtml::FiltersOutCodeJsJob(bool bIsRemoveCodeJsJob)
{
	return c_FiltersOutCodeJsJob(m_sCodeHtml, bIsRemoveCodeJsJob);
}

//--------------------------------------------------------------------------
// Cai dat code html 
//--------------------------------------------------------------------------
void CFileHtml::SetCodeHtml(string &sCodeHtml)
{
	this->m_sCodeHtml = RemoveCommentBlock(sCodeHtml);
}

//--------------------------------------------------------------------------
// Xuat code html 
//--------------------------------------------------------------------------
string& CFileHtml::GetCodeHtml(){
	return this->m_sCodeHtml;
}

//------------------------------------------------------------------------------
// -> return "m_szDocumentData"
//------------------------------------------------------------------------------
string CFileHtml::GetDocumentData()		// get document Data
{
	return m_sDocumentData;
}


//------------------------------------------------------------------------------
// Run	-> add "szCode" to "CFileHtml::m_szDocumentData"
//------------------------------------------------------------------------------
void CFileHtml::AppendDocumentData(string szCode)
{
	if (szCode.size() > 0)
		this->m_sDocumentData += szCode;
}

//------------------------------------------------------------------------------
// Remove Comment block
//------------------------------------------------------------------------------
string CFileHtml::RemoveCommentBlock(string sCodeHtml)
{
// 	regex regexCheckComment("<!--([^]*?[^?])?-->");
// 
// 	std::sregex_iterator next(sCodeHtml.begin(), sCodeHtml.end(), regexCheckComment);
// 	string sRet;
// 
// 	try
// 	{
// 		sRet = std::regex_replace(sCodeHtml, regexCheckComment, "");
// 		goto _EXIT_FUNCTION;
// 	}
// 	catch (...)
// 	{
// 		sRet = sCodeHtml;
// 	}
// _EXIT_FUNCTION:
// 
// 	return sRet;
	return sCodeHtml;
}

//------------------------------------------------------------------------------
// Create XML DOM Document 
//------------------------------------------------------------------------------
HRESULT CFileHtml::CreateAndInitDOM(IXMLDOMDocument **ppDoc)
{
	HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
	if (SUCCEEDED(hr))
	{
		// these methods should not fail so don't inspect result
		(*ppDoc)->put_async(VARIANT_FALSE);
		(*ppDoc)->put_validateOnParse(VARIANT_FALSE);
		(*ppDoc)->put_resolveExternals(VARIANT_FALSE);
	}
	return hr;
}

//------------------------------------------------------------------------------
// Loc codeJs cua doi tuong html hien tai
//------------------------------------------------------------------------------
string CFileHtml::FilterOutCodeJsInDocument()
{
	return CFileHtml::c_FilterOutCodeJs(this->m_sDocumentData, true);
}

//------------------------------------------------------------------------------
// Khoi tao lai ds m_sDocumentData. Su dung khi reset moi truong quet
//------------------------------------------------------------------------------
void CFileHtml::resetDatabase()
{
	m_sDocumentData = "";
	m_sCodeHtml = "";

#ifdef DEVELOP_HTML
	m_lstsCodeJs.clear();
	m_sCodeJs = "";
#endif //!DEVELOP_HTML
}

//------------------------------------------------------------------------------
// Lay thong tin szNametag, szId tu noi dung codeHtml chen vao.
//------------------------------------------------------------------------------
map<string, string> CFileHtml::GetInfoElement(string szCodeHtmlCheck, bool &bIsValidate)
{
	map<string, string> mapHtmlElement;
	wstring wsTagName;
	wstring wsAttrVal;

	HRESULT hr;
	IXMLDOMElement *pXMLElement = NULL;
	IXMLDOMDocument *pXMLDom = NULL;
	IXMLDOMParseError *pXMLErr = NULL;

	BSTR bstrTagName = NULL;
	BSTR bstrAttrId = NULL;
	BSTR bstrXML = NULL;
	BSTR bstrErr = NULL;

	VARIANT vId;
	VARIANT vSrc;
	VARIANT_BOOL vbStatus = VARIANT_FALSE;

	mapHtmlElement["tagName"] = "";
	mapHtmlElement["id"] = "";
	mapHtmlElement["src"] = "";
	bIsValidate = false;

	hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		// Khoi tao cac bien kiem tra Validate code Html
		VariantInit(&vId);
		VariantInit(&vSrc);
		CHK_HR(CreateAndInitDOM(&pXMLDom));
		bstrXML = SysAllocString(wstring(szCodeHtmlCheck.begin(), szCodeHtmlCheck.end()).c_str());
		CHK_ALLOC(bstrXML);
		CHK_HR(pXMLDom->loadXML(bstrXML, &vbStatus));

		if (vbStatus == VARIANT_TRUE)
		{
			// load HTML successful -> get Element
			CHK_HR(pXMLDom->get_documentElement(&pXMLElement));
			if (pXMLElement)
			{
				CHK_HR(pXMLElement->get_tagName(&bstrTagName));
				CHK_HR(pXMLElement->getAttribute(L"id", &vId));
				CHK_HR(pXMLElement->getAttribute(L"src", &vSrc));
				bstrAttrId = vId.bstrVal;

				// Luu ket qua tra ve
				if (bstrTagName)
				{
					wsTagName = bstrTagName;
					mapHtmlElement["tagName"] = string(wsTagName.begin(), wsTagName.end());
				}

				if (vId.bstrVal)
				{
					wsAttrVal = vId.bstrVal;
					mapHtmlElement["id"] = string(wsAttrVal.begin(), wsAttrVal.end());
				}

				if (vSrc.bstrVal)
				{
					wsAttrVal = vSrc.bstrVal;
					mapHtmlElement["src"] = string(wsAttrVal.begin(), wsAttrVal.end());
				}
				bIsValidate = true;

			}
		}
	CleanUp:
		SAFE_RELEASE(pXMLDom);
		SAFE_RELEASE(pXMLErr);
		SAFE_RELEASE(pXMLElement);
		SysFreeString(bstrXML);
		SysFreeString(bstrErr);
		SysFreeString(bstrTagName);
		SysFreeString(bstrAttrId);
		VariantClear(&vId);
		VariantClear(&vSrc);

		CoUninitialize();
	}

	return mapHtmlElement;
}

map<string, string> CFileHtml::DetectAttrsValue(string sAttrs)
{
	// 	regex regexCheckElement("(\\s[^>]*?id=\"" +  + "\"[^>]*?)>([^]*?)</\\1>",
	// 		std::regex_constants::ECMAScript | std::regex_constants::icase);
	// 
	// 	std::sregex_iterator next(m_sCodeHtml.begin(), m_sCodeHtml.end(), regexCheckElement);
	// 	std::sregex_iterator end;
	// 	std::smatch smatchInnerHTMLs;
	map<string, string> lstAttrs;
	return lstAttrs;
}

//------------------------------------------------------------------------------
// Trich xuat ten the HTML
//------------------------------------------------------------------------------
string CFileHtml::GetTagNameFromOpenTag(string sOpenTag)
{
	int nSize = sOpenTag.size();
	int i = 0;
	char cCheck = 0;
	string sTagName = "";

	// Kiem tra noi dung the mo truyen vao
	if (nSize == 0 || sOpenTag.at(0) != '<') goto _EXIT_FUNCTION;

	for (i = 1; i < nSize; i++)
	{
		cCheck = sOpenTag.at(i);
		if ((cCheck > 'a' && cCheck < 'z') ||
			(cCheck > 'A' && cCheck < 'Z') ||
			(cCheck > '0' && cCheck < '9'))
		{
			sTagName += cCheck;
		}

		// Dk ket thuc
		else break;
	}
_EXIT_FUNCTION:
	return sTagName;
}

//------------------------------------------------------------------------------
// Tim the HTML theo id .
//------------------------------------------------------------------------------

string CFileHtml::GetElementById(string sId, string &sAttrsOut, string & sInnerHTMLOut, string & sTagNameOut)
{
	//regex regexCheck("<[a-zA-Z](.*?[^?])? id(\\s*)=(\\s*)\"" + sId + "\"([^>]*)>(.*?)</\1>");
	//regex regexCheck2("<([^\s]+).*?id=\"([^\"]*?)\".* ? >(. + ? )< / \1>");
	//regex regexCheck2("<([^\s]+).*?id=\""+sId+"\".* ? >(. + ? )</([^\s]+).*?>");
	string sElementRet;

	try{
		regex regexCheckElement("<(\\w+)(\\s[^>]*?id\\s*=\\s*['\"]" + sId + "['\"][^>]*?)>([^]*?)</\\1>",
			std::regex_constants::ECMAScript | std::regex_constants::icase);

		std::sregex_iterator next(m_sCodeHtml.begin(), m_sCodeHtml.end(), regexCheckElement);
		std::sregex_iterator end;
		std::smatch smatchInnerHTMLs;

		if (next == end)
			goto _EXIT_FUNCTION;

		for (std::sregex_iterator it = next; it != end; it++)
		{
			// Lay noi dung cua thanh phan HTML
			sElementRet = it->str();

			// Lay noi dung InnerHTML, TagName, Attributes
			std::regex_match(sElementRet, smatchInnerHTMLs, regexCheckElement);
			if (smatchInnerHTMLs.size() >= 3)
			{
				sInnerHTMLOut = smatchInnerHTMLs[3].str();
				sTagNameOut = smatchInnerHTMLs[1].str();
				sAttrsOut = smatchInnerHTMLs[2].str();
			}
			break;
		}
	}
	catch (...){
		// Khong lay duoc thong tin
	}
_EXIT_FUNCTION:
	return sElementRet;
}


//------------------------------------------------------------------------------
// Run like case-insensitive substring search -> find "szTagName" in "szData" from position = "posStart"  
// Output	-> return position found
// useL KMPi_Search in here
//------------------------------------------------------------------------------
int getPosStr(string szData, string szTagName, int nPosStart){

	transform(szData.begin(), szData.end(), szData.begin(), ::tolower);
	return szData.find(szTagName, nPosStart);
}



// -----------------------------------------------------------------------------
#ifdef DEVELOP_HTML
int findPosErr(int nLine, string sData)
{
	int iChar = 0;
	int nSize = sData.size();
	for (iChar = 0; iChar < nSize; iChar++)
	{
		if (sData.at(iChar) == '\n'){
			nLine--;
		}
		if (nLine == 1)
			return iChar;

	}
	return -1;
}

void fixDelBeforeTag(int nPosChar, string &sData)
{

	while (sData.at(nPosChar) != '<')
		nPosChar--;
	while (sData.at(nPosChar) != '>')
	{
		sData.at(nPosChar) = ' ';
		nPosChar++;
	}
	sData.at(nPosChar) = ' ';
}

bool fixValueInQuocte(long posLine, long posColumn, string &sData)
{
	int nPosCharChange;
	int nPosChar = findPosErr(posLine, sData);
	string sValue = "\"";
	if (nPosChar != -1)
	{
		sValue = "\"";
		nPosChar += posColumn;
		while (sData.at(nPosChar) == ' ')
			nPosChar++;
		nPosCharChange = nPosChar;
		while (sData.at(nPosChar) != ' ' && sData.at(nPosChar) != '>')
		{
			sValue += sData.at(nPosChar);
			nPosChar++;
		}
		sValue += '\"';
		sData.replace(nPosCharChange, nPosChar - nPosCharChange, sValue);
		return true;
	}
	return false;

}

bool fixAttributeDuplicate(long posLine, long posColumn, string &sData)
{
	int nPosAttrStart;
	int nPosAttrEnd;
	int nPosCharEqual = 0;
	int nPosTagStart;
	int nPosTagEnd;
	int nPosAttrValStart;
	int nPosAttrValEnd;
	string sTagValue;
	string sAttrName;
	string sAttrValue;
	int nPosChar = findPosErr(posLine, sData);

	if (nPosChar != -1)
	{
		nPosChar += (posColumn - 1);
		nPosTagStart = sData.find_last_of('<', nPosChar);
		nPosTagEnd = sData.find_last_of('>', nPosChar);

		if (nPosTagStart != string::npos && nPosTagEnd != string::npos)
		{
			sTagValue = sData.substr(nPosTagStart, nPosTagEnd - nPosTagStart + 1);

			nPosCharEqual = sTagValue.find('=', nPosCharEqual);
			while (nPosCharEqual != string::npos)
			{
				// get string attribute name
				nPosChar = nPosCharEqual - 1;
				while (sTagValue.at(nPosChar) == ' ')
					nPosChar--;
				nPosAttrEnd = nPosChar;
				nPosAttrStart = sTagValue.find_last_of(' ', nPosAttrEnd);
				if (nPosAttrStart != string::npos){
					sAttrName = sTagValue.substr(nPosAttrStart, nPosAttrEnd - nPosAttrStart + 1);
				}
				else
					return false;

				// get string attribute Value
				nPosChar = nPosCharEqual + 1;
				while (sTagValue.at(nPosChar) == ' ')
					nPosChar++;
				nPosAttrValStart = nPosChar;
				nPosAttrValEnd = sTagValue.find(' ', nPosAttrValStart);

				if (nPosAttrValEnd == string::npos)
				{
					nPosAttrValEnd = sTagValue.find('>', nPosAttrValStart);
					if (nPosAttrValEnd == string::npos)
						return false;
				}
				else
					sAttrValue = sTagValue.substr(nPosAttrValStart, nPosAttrValEnd - nPosAttrValStart);

				if (sTagValue.find(sAttrName, nPosAttrStart + 1) != string::npos)
				{
					// delete this attribute
					sTagValue.erase(nPosAttrStart, nPosAttrValEnd - nPosAttrStart);
					nPosCharEqual = nPosAttrStart;
				}
				else
					nPosCharEqual = nPosAttrValEnd;

				nPosCharEqual = sTagValue.find('=', nPosCharEqual + 1);
			}

		}
		else
			return false;

		sData.replace(nPosTagStart, nPosTagEnd - nPosTagStart + 1, sTagValue);
		return true;
	}
	return false;
}

bool fixWhiteSpaceNotAllow(long posLine, long posColumn, string &sData)
{
	string sVal;
	int nPosChar = findPosErr(posLine, sData);
	if (nPosChar != -1)
	{
		nPosChar += posColumn;
		sVal = sData.erase(nPosChar, 1);
		return true;
	}
	return false;
}

bool fixNoEndTag(long posLine, long posColumn, string &sData, BSTR bstrErr)
{
	int nPosTagErrStart;
	int nPosTagErrEnd = 0;

	int nPosTagNoCloseStart;
	int nPosTagNoCloseEnd;

	wstring wsErr(bstrErr);
	string sErr(wsErr.begin(), wsErr.end());
	string sTagErr("<");
	string sTagCloseErr("</");
	string sTagNoClose("<");
	string sTagNoCloseSave("</");

	int nSizeOld;
	int nPosChar = sErr.find('\'', 9);
	if (nPosChar != string::npos)
	{
		sTagErr += sErr.substr(9, nPosChar - 9);
		sTagCloseErr += sErr.substr(9, nPosChar - 9);
	}
	else
		return false;

	sTagNoClose += sErr.substr(nPosChar + 32, sErr.size() - nPosChar - 36);
	sTagNoCloseSave += sErr.substr(nPosChar + 32, sErr.size() - nPosChar - 36);

	sTagNoCloseSave = sTagNoClose;

	nPosChar = findPosErr(posLine, sData);
	if (nPosChar != -1)
	{
		nPosChar += posColumn;
		nPosTagErrEnd = nPosChar - 2;
	}

	// find open tag of errTag
	nPosTagErrStart = sData.rfind(sTagErr, nPosChar - 3);
	sTagErr = sData.substr(nPosTagErrStart, nPosChar - nPosTagErrStart - 2);

	// filter tag true
	do{
		nPosChar = sTagErr.rfind(sTagNoCloseSave);
		if (nPosChar != string::npos)
		{
			nPosTagErrEnd = nPosChar;
			nPosChar = sTagErr.rfind(sTagNoCloseSave, nPosChar);
		}
		else break;
	} while (nPosChar != string::npos);

	// fix tag noClose
	if (nPosTagErrStart != string::npos)
	{
		// find tag no close
		nPosTagNoCloseStart = sTagErr.rfind(sTagNoClose, nPosTagErrEnd);

		while (nPosTagNoCloseStart != string::npos)
		{
			nPosTagNoCloseEnd = sTagErr.find('>', nPosTagNoCloseStart);

			while (nPosTagNoCloseEnd != string::npos)
			{
				sTagNoClose = sTagErr.substr(nPosTagNoCloseStart, nPosTagNoCloseEnd - nPosTagNoCloseStart + 1);

				// check tag no close
				if (sTagNoClose.at(nPosTagNoCloseEnd - nPosTagNoCloseStart - 1) != (char)'//')
				{
					sTagNoClose.at(nPosTagNoCloseEnd - nPosTagNoCloseStart) = (char)'//';
					sTagNoClose += ">";

					if (CFileHtml::CheckValidateHTML(sTagNoClose))
					{
						nSizeOld = sTagErr.size();
						sTagErr.replace(nPosTagNoCloseStart,
							nPosTagNoCloseEnd - nPosTagNoCloseStart + 1, sTagNoClose);
						sData.replace(nPosTagErrStart, nSizeOld, sTagErr);
						return true;
					}

				}
				else {

					if (CFileHtml::CheckValidateHTML(sTagNoClose))
						break; // to find new pos TagNoClose

				}
				nPosTagNoCloseEnd = sTagErr.find('>', nPosTagNoCloseEnd + 1);
			}
			nPosTagNoCloseStart = sTagErr.rfind(sTagNoCloseSave, nPosTagNoCloseStart - 1);
		}

	}


	return false;
}

bool fixNameTagFail(long posLine, long posColumn, string &sData){

	int nPosChar = findPosErr(posLine, sData);
	string sVal;
	if (nPosChar != -1)
	{
		nPosChar += posColumn;
		sVal = sData.substr(nPosChar, 20);
		return true;
	}
	return false;

}

#endif //!DEVELOP_HTML