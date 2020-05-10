////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia lop CFileHtml. Xu ly luu tru noi dung khi code 
// Js su dung ham document.write(szString); 
// Noi dung szString dc them vao thuoc tinh CFileHtml::m_szDocumentData
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef FILE_HTML_H
#define FILE_HTML_H

#include "RuntimeException.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <windows.h>
#include <objbase.h>
#include <msxml6.h>
#include <regex>
#include <map>
using namespace std;

// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)

// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)

//==============================================================================
// ClassName: CFileHtml
// Description: Object process file html, filter tag script, 
//				write to CFileHtml::m_szDocumentData if programe run document.write();
//==============================================================================

class CFileHtml
{
private:
	string m_sCodeHtml;
	string m_sDocumentData;	// save Data when java Script call document.write()
	static HRESULT CreateAndInitDOM(IXMLDOMDocument **ppDoc);

#ifdef DEVELOP_HTML
	string m_sCodeJs;
	wstring m_wsPathFile;
	vector <string> m_lstsCodeJs;
	DWORD m_dwSizeFileHtml;
	static HRESULT VariantFromString(PCWSTR wszValue, VARIANT &vVariant);
	void LoadDOMRaw(string &sCodeHtml);
#endif // DEVELOP_HTML

public:
	//--------------------------------------------------------------------------
	// Khoi tao doi tuong
	//--------------------------------------------------------------------------
	CFileHtml();

	//--------------------------------------------------------------------------
	// Huy bo doi tuong
	//--------------------------------------------------------------------------
	~CFileHtml(void);

	//--------------------------------------------------------------------------
	// Cai dat code html 
	//--------------------------------------------------------------------------
	void SetCodeHtml(string &sCodeHtml);	// get document Data

	//--------------------------------------------------------------------------
	// Lay code Html
	//--------------------------------------------------------------------------
	string& GetCodeHtml();

	//--------------------------------------------------------------------------
	// Lay code HTML luu trong "FileHtmnl::m_szDocumentData"
	// Su dung khi CodeVirus ghi vao HTML nhieu lan tao cap the <script></script>
	//--------------------------------------------------------------------------
	string GetDocumentData();	// get document Data

	//--------------------------------------------------------------------------
	// Them code HTML vao "FileHtmnl::m_szDocumentData"
	// Su dung khi CodeVirus goi ham document.Write(szHTMLCode);
	//--------------------------------------------------------------------------
	void AppendDocumentData(string sCode);

	//--------------------------------------------------------------------------
	// Remove Comment block
	//--------------------------------------------------------------------------
	string RemoveCommentBlock(string sCodeHtml);

	//--------------------------------------------------------------------------
	// Lay + xoa Code JavaScript tu thuoc tinh "FileHtmnl::m_szDocumentData"
	//--------------------------------------------------------------------------
	string FilterOutCodeJsInDocument();

	//--------------------------------------------------------------------------
	// Xoa du lieu trong doi tuong - chuan bi cho lan quet tiep theo
	// Du lieu duoc cai dat lai khi quet file khac
	//--------------------------------------------------------------------------
	void resetDatabase();

	//--------------------------------------------------------------------------
	// Lay thong tin thanh phan cua the HTML: szCodeHtmlCheck
	//--------------------------------------------------------------------------
	map<string, string> GetInfoElement(string sCodeHtmlCheck, bool &bIsValidate);
	map<string, string> DetectAttrsValue(string sAttrs);

	string GetTagNameFromOpenTag(string sOpenTag);

	//--------------------------------------------------------------------------
	// Lay thong tin thanh phan cua the HTML ( id )
	//--------------------------------------------------------------------------
	string GetElementById(string sId, string &sAttrs, string &sInnerHTMLOut, string & sTagNameOut);

	//--------------------------------------------------------------------------
	// get all Code Script in <script>...</script>
	// bIsRemoveCodeJs = TRUE: delete code java script from "szCodeHtml"
	// bIsRemoveCodeJs = FALSE:  not delete code java script from "szCodeHtml"
	//--------------------------------------------------------------------------
	static string c_FilterOutCodeJs(string &sCodeHtml, bool bIsRemoveCodeJs = false);


	//--------------------------------------------------------------------------
	// get Code Script -> Array[] : Array[i] = code in Element[i] <script>..</script> 
	// bIsRemoveCodeJs = TRUE: delete code java script from "szCodeHtml"
	// bIsRemoveCodeJs = FALSE:  not delete code java script from "szCodeHtml"
	//--------------------------------------------------------------------------
	static vector<string> c_FiltersOutCodeJs(string &sCodeHtml,
		bool bIsRemoveCodeJs = false);
	static vector<string> c_FiltersOutCodeJsJob(string &sCodeHtml, bool bIsRemoveCodeJsJob = true);
	vector<string> CFileHtml::FiltersOutCodeJs(bool bIsRemoveCodeJs = true);
	vector<string> CFileHtml::FiltersOutCodeJsJob(bool bIsRemoveCodeJsJob = true);

#ifdef DEVELOP_HTML
	CFileHtml(wstring szPathFile);

	string GetCodeJs();		// get Code Script
	vector <string> GetListCodeJs(); // get list Code Script
	HRESULT LoadXMLFile(IXMLDOMDocument *pXMLDom, LPCWSTR lpszXMLFile);
	void PrefixHtml(string &sCodeHtml);
	static bool CheckValidateHTML(string sCodeHtml);
#endif // DEVELOP_HTML

};

#endif // FILE_HTML_H