////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac phuong thuc lop CProgramJs.
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include <exception>
#include <vector>
#include <sstream>
#include <assert.h>
#include "tchar.h"

#include "ProgramJs.h" 

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

#define ASSERT(X) assert(X)

extern CProgramJs*	g_pProgramJs;
extern DWORD		g_dwTimeStartScan;
extern PDETECT_DOWNLOADER g_sstrDetectDownloader;
extern PDETECT_VIRUSOTHER g_sstrDetectVirusOther;

char* g_szTopOfStack;
int g_iNotEnoughMem = 0;
#ifdef KIEMVM
int g_Count = 0;
#endif // KIEMVM


#ifndef BUID_UPDATE
extern long g_lRequestNumber;
#endif // !BUID_UPDATE

CVar* CVarLink::m_pVRootStack = NULL;
BOOLEAN CVar::m_bIsResetStack = FALSE;
#pragma warning(disable:4996)

int GetCurrentStackSize()
{
	char localVar;
	int curStackSize = (&localVar) - g_szTopOfStack;
	if (curStackSize < 0) curStackSize = -curStackSize;  // in case the stack is growing down
	return curStackSize;
}

//------------------------------------------------------------------------------
// Cai dat cac thong tin thuoc tinh cho trinh thong dich: CProgramJs
// Khoi tao doi tuong CFileHtml
// Tao thuoc tinh m_pVRootStack
// Them cac doi tuong String, Array, Object , Double vao doi tuong Root Stack
//------------------------------------------------------------------------------
void CProgramJs::InitProgramJs()
{
	CVarLink* pVLWindow = NULL;
	CVarLink* pVLDcmnt = NULL;
	CVarLink* pVLDB = NULL;
	CVarLink* pVLDString = NULL;

	if (!m_pFileHtmlProcess)
		m_pFileHtmlProcess = new CFileHtml(); // Tao doi tuong Html
	SetNotRunInTryCatch();	// Khoi tao ko chay trong try-catch

	// Add built-in classes
	m_pVRootStack = new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT);
#if SAFE_MEMORY
	RemoveTempStack(m_pVRootStack);
#endif

	m_lstFuncCallBackBySetInterval.clear();
	m_pVStringClass = (new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT))->Ref();
	m_pVArrayClass = (new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT))->Ref();
	m_pVObjectClass = (new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT))->Ref();
	m_pVDoubleClass = (new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT))->Ref();
	//	m_pVWScriptClass = (new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT))->Ref();

	pVLWindow = m_pVRootStack->AddChild("window", m_pVRootStack, TRUE);
	pVLWindow->SetNotReDefine();

	// Phai cai dat sau khi AddChild("window"..) vao stack de ko them vao blacklist
	CVarLink::SetRootStack(m_pVRootStack);

	pVLDString = m_pVRootStack->AddChild("String", m_pVStringClass, TRUE);
	(m_pVRootStack->AddChild("Array", m_pVArrayClass, TRUE))->SetNotReDefine();
	(m_pVRootStack->AddChild("Object", m_pVObjectClass, TRUE))->SetNotReDefine();
	pVLDB = m_pVRootStack->AddChild("Double", m_pVDoubleClass, TRUE);

	//Them thuoc tinh String.status----Downloader
	m_pVLDStringStatus = pVLDString->m_pVar->AddChild("status", new CVar(200), TRUE);
	m_pVLDStringStatus->SetNotReDefine(); 

	m_pVLDStringresponseText = pVLDString->m_pVar->AddChild("responseText", new CVar("responseText"), TRUE);
	m_pVLDStringresponseText->SetNotReDefine();

	//Them thuoc tinh string.onreadystatechange ---Downloader
	m_pVLDStringOnreadyStateChange = pVLDString->m_pVar->AddChild("onreadystatechange", new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION), TRUE);
	m_pVLDStringOnreadyStateChange->SetNotReDefine();

	//Them thuoc tinh String.readyState
	m_pVLDStringreadyState = pVLDString->m_pVar->AddChild("readyState", new CVar(4), TRUE);
	m_pVLDStringreadyState->SetNotReDefine();

	//Them thuoc tinh double.size ----Downloader
	m_pVLDStatus = pVLDB->m_pVar->AddChild("size", new CVar(1001), TRUE);
	m_pVLDStatus->SetNotReDefine();

	// Them doi tuong Html element
	(m_pVRootStack->AddChild("element", new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), TRUE))->SetNotReDefine();

	// Them thuoc tinh window.onload
	m_pVLWindowOnload =
		m_pVRootStack->AddChild("onload", new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION), TRUE);
	m_pVLWindowOnload->SetNotReDefine();

	// Them thuoc tinh Document.onmousemove
	pVLDcmnt = m_pVRootStack->AddChild("document", new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), TRUE);
	m_pVLDcmtOnmousemove = pVLDcmnt->m_pVar->AddChild("onmousemove", new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), TRUE);
	m_pVLDcmtOnmousemove->SetNotReDefine();

	// register default API.
	RegisterFunctions(this);

	// Lay thong tin cac phan tu co kha nang redirect trang den 1 trang khac
	RegisterListAttrRedirect();

	// Cai dat vi tri Xoa cho lan quet tiep theo
	m_pVLStartReset = m_pVRootStack->m_pVLLastChild;
}

//------------------------------------------------------------------------------
// Lay thong tin cac thuoc tinh co kha nang redirect den 1 trang khac
//------------------------------------------------------------------------------
void CProgramJs::RegisterListAttrRedirect()
{
	const char* szScriptVariableAttrNeedCheck[] =
	{
		"location",
		"location.href",
		"top.location",
		"top.location.href"
	};

	vector <string> lstScriptVariableAttrNeedCheck(szScriptVariableAttrNeedCheck, szScriptVariableAttrNeedCheck + 4);
	int i = 0;
	int nSize = lstScriptVariableAttrNeedCheck.size();

	CVarLink* pVLVariable = NULL;

	for (i = 0; i < nSize; i++)
	{
		pVLVariable = this->GetVLScriptVariable(lstScriptVariableAttrNeedCheck.at(i));
		if (pVLVariable != NULL)
			this->m_lstVLRedirectLink.push_back(pVLVariable);
	}
}

//------------------------------------------------------------------------------
// Khoi tao doi tuong CFileHtml
// Cai dat cac thong tin thuoc tinh cho trinh thong dich: CProgramJs
//------------------------------------------------------------------------------
CProgramJs::CProgramJs()
{
	m_onreadystatechange = false;
	m_ForIn = false;

	// Khoi tao gia tri m_bEncodeJS = false va m_sCodeJS = ""
	m_sCodeJS = "";
	nLenghtArrayForIn = 0;
	this->InitNullForPropertyPointer();
	this->m_dwSizeData = 0;
	InitProgramJs();
}

//------------------------------------------------------------------------------
// Khoi tao doi tuong CFileHtml
// Cai dat cac thong tin thuoc tinh cho trinh thong dich: CProgramJs
//------------------------------------------------------------------------------
CProgramJs::CProgramJs(LPVOID pBuff, DWORD dwSizeBuff, int nTypeFile)
{
	m_onreadystatechange = false;
	m_ForIn = false;
	nLenghtArrayForIn = 0;
	this->InitNullForPropertyPointer();

	// -------------------------------------------------------------------------
	// dau vao la dia chi vung "data file html" + kich thuoc 
	// -------------------------------------------------------------------------
	if (nTypeFile == INPUT_DATA_FILE_HTML && pBuff)
	{
		SetData(pBuff, dwSizeBuff);
		// install actual API for programe
		InitProgramJs();
	}

	// -------------------------------------------------------------------------
	// dau vao la dia chi duong dan "file html"
	// -------------------------------------------------------------------------
#ifdef DEVELOP_HTML
	else if (nTypeFile == INPUT_PATH_FILE_HTML && pBuff)
	{
		LPCWSTR szPathFile = (LPCWSTR)pBuff;
		if (wcslen(szPathFile) <= 0)
			throw new CRuntimeException("Error: pathFile!! File doesn't exist!",
			CREATE_FILE_FALSE);

		m_pFileHtmlProcess = new CFileHtml(szPathFile);

		m_lstsCodeJs = m_pFileHtmlProcess->GetListCodeJs();

		// Cai dat cac ham API thuc su cho chuong trinh
		InitProgramJs();
	}
#endif // DEVELOP_HTML

	// -------------------------------------------------------------------------
	// dau vao LOI
	// -------------------------------------------------------------------------
	else
		throw new CRuntimeException("Error: typeFile!! File doesn't file html or js!", CREATE_FILE_FALSE);
}

//------------------------------------------------------------------------------
// Toan tu huy doi tuong
//------------------------------------------------------------------------------
CProgramJs::~CProgramJs()
{
	SAFE_DELETE(this->m_szDataFile);
	SAFE_DELETE(this->m_pFileHtmlProcess);

	m_lstStack.clear();
#if SAFE_MEMORY	
	// Xoa bo Lien ket cua cac bien toi RootStack
	CleanListPointToStack();
#endif

// 	m_pVStringClass->Unref();
// 	m_pVArrayClass->Unref();
// 	m_pVObjectClass->Unref();
// 	m_pVDoubleClass->Unref();
// 	m_pVRootStack->Unref();

#if SAFE_MEMORY
	CleanTempStack();
#endif
}

//// Lay vi tri TagName trong chuoi string
//int getPosStr(string szData, string szTagName, int nPosStart) {
//
//	transform(szData.begin(), szData.end(), szData.begin(), ::tolower);
//	return szData.find(szTagName, nPosStart);
//}

//------------------------------------------------------------------------------
// Loc code JS tu pBuff chua noi dung file HTML => mang cac doan code JS
//------------------------------------------------------------------------------
void CProgramJs::SetData(LPVOID pBuffHtmlCode, DWORD dwSizeBuff)
{
	BYTE* pBuffHtmlCodeCopy = NULL;
	string sCodeHtml;
	int i = 0;
	int		nErrId = -1;
	SetNotRunInTryCatch();	// Khoi tao ko chay trong try-catch

	// Lay ds noi dung Code Js vao m_lstSzCodeJs
	pBuffHtmlCodeCopy = new BYTE[dwSizeBuff + 1];
	if (pBuffHtmlCodeCopy == NULL)
		throw new CRuntimeException("Error: Not Enough Mem!! File doesn't Scan!",CREATE_FILE_FALSE, false);

	// Copy dữ liệu code HTML vào biến pBuffHtmlCodeCopy
	memcpy(pBuffHtmlCodeCopy, pBuffHtmlCode, dwSizeBuff);
	pBuffHtmlCodeCopy[dwSizeBuff] = 0;
	sCodeHtml = ((char*)pBuffHtmlCodeCopy); // copy lại lần nữa
	this->m_pFileHtmlProcess->SetCodeHtml(sCodeHtml); // hàm này mục đích lọc các comment trong code nhưng đã bị comment phần code đó, 
													// hiện tại không có tác dụng gì
	//try
	//{
	//	// check các chuỗi code đặc trưng có trong code, nếu có sẽ trả về loại virus
	//	this->CheckVirusStatic(this->m_pFileHtmlProcess->GetCodeHtml());
	//}
	//catch (CRuntimeException *pRE)
	//{
	//	// lấy giá trị lỗi trả về
	//	nErrId = pRE->GetErrId();
	//	if (
	//		nErrId == EXCEPTIONID_FOUND_VIRUS
	//		|| nErrId == NOT_ENOUGH_MEMORY
	//		|| nErrId == EXCEPTION_TIME_OUT_SCAN
	//		)
	//	{
	//		SAFE_DELETE_ARRAY(pBuffHtmlCodeCopy);
	//		throw pRE;	// Phat hien virus => chuyen tiep exception
	//	}
	//	SAFE_DELETE(pRE)
	//}
	SAFE_DELETE_ARRAY(pBuffHtmlCodeCopy);

	m_lstsCodeJsJob = this->m_pFileHtmlProcess->FiltersOutCodeJsJob();
	// tương tự với thẻ <script>
	m_lstsCodeJs = this->m_pFileHtmlProcess->FiltersOutCodeJs();
	if (m_lstsCodeJsJob.size() > 0)
	for (size_t i = 0; i < m_lstsCodeJsJob.size(); i++)
		{
			// vòng lặp thêm code trong thẻ <job> vào danh sách code trong thẻ script
			m_lstsCodeJs.push_back(m_lstsCodeJsJob.at(i));
		}
}

//------------------------------------------------------------------------------
// Khoi phuc chuog trinh ve trang thai ban dau, Voi cac ham API de tiep tuc quet 
// file HTML tiep theo
//------------------------------------------------------------------------------
BOOLEAN CProgramJs::ResetDatabase()
{
	BOOLEAN bResult = FALSE;
	INT i = 0, nSize = 0;
	CVarLink* pVLHtmlBody = NULL;
	CVarLink* pVLDocument = NULL;
	CVarLink* pVLObjDetete = NULL;
	CVarLink* pVLTmp = NULL;

	// Cai dat lai che do khong chay trong try-catch
	m_bRunInTryCatch = false;
	m_bHadException = false;

	m_sCodeJS = "";

	nSize = m_lstVLMsgException.size();
	for (i = 0; i < nSize; i++)
	{
		pVLTmp = m_lstVLMsgException.back();
		m_lstVLMsgException.pop_back();
		
	}

	// Khoi phuc ds ham Callback by Interval
	m_lstFuncCallBackBySetInterval.clear();

	// Khoi phuc du lieu trong thuoc tinh m_pFileHtmlProcess
	m_pFileHtmlProcess->resetDatabase();

	// Khoi phuc lai du lieu doi tuong body 
	if (!this->ResetDocumentTree()) goto _EXIT_FUNCTION;

#if SAFE_MEMORY
	CleanListPointToStack();
#endif // SAFE_MEMORY

	// Xoa ds bien trong stack
	if (m_pVLStartReset)
	{
		CVar::m_bIsResetStack = TRUE;	// Cai dat "flag" ho tro xoa chu trinh
		// neu ton tai trong Stack.
		pVLObjDetete = m_pVLStartReset->m_pNextSibling;
		while (pVLObjDetete)
		{
			pVLTmp = pVLObjDetete->m_pNextSibling;
			// Xoa tung phan tu cho den het Stack
			m_pVRootStack->RemoveLink(pVLObjDetete);
			pVLObjDetete = pVLTmp;
		}
		CVar::m_bIsResetStack = FALSE;
	}
	else
	{
		bResult = FALSE;
		goto _EXIT_FUNCTION;
	}

	this->CleanRootStack();		// Xoa cac bien sinh ra khi quet file HTML hien tai
	this->m_lstStack.clear();	// Xoa ds stack
	this->m_lstsCodeJs.clear();	// Xoa bo ds code JS cu
	this->m_lstsCodeJsJob.clear();

	SAFE_DELETE(this->m_szDataFile);

	m_pTokenPointer = NULL;
#if SAFE_MEMORY
	// Xoa cac bien tam thoi sinh ra trong qua trinh quet Vr
	CleanTempStack();
#endif // SAFE_MEMORY

	bResult = TRUE;

_EXIT_FUNCTION:
	return bResult;
}
//------------------------------------------------------------------------------
// Xoa bo cac the sau document.body.tagName
//------------------------------------------------------------------------------
BOOLEAN CProgramJs::ResetDocumentTree()
{
	BOOLEAN bRet = FALSE;
	CVar* pDocumentBody = GetScriptVariable("document.body");
	CVar* pDocumentHead = GetScriptVariable("document.head");
	CVarLink* pVLSearchOtherTag = NULL;

	if (!pDocumentBody || !pDocumentHead) goto _EXIT_FUNCTION;

	// Kiem tra tag <head> 
	pVLSearchOtherTag = pDocumentHead->FindChild("tagName");
	if (!pVLSearchOtherTag || !pVLSearchOtherTag->m_pVar) goto _EXIT_FUNCTION;

	while (pVLSearchOtherTag->m_pNextSibling)
	{
		pDocumentHead->RemoveChild(pVLSearchOtherTag->m_pNextSibling->m_pVar);
	}

	// Kiem tra tag <body>
	pVLSearchOtherTag = pDocumentBody->FindChild("tagName");
	if (!pVLSearchOtherTag || !pVLSearchOtherTag->m_pVar) goto _EXIT_FUNCTION;

	while (pVLSearchOtherTag->m_pNextSibling)
	{
		pDocumentBody->RemoveChild(pVLSearchOtherTag->m_pNextSibling->m_pVar);
	}

	// Thanh cong
	bRet = TRUE;

_EXIT_FUNCTION:
	return bRet;
}

//--------------------------------------------------------------------------
// Xoa cac bien co thuoc tinh: "CVarLink::m_bReDefine == true;"
// Khoi phuc RootStack ve trang thai ban dau
//--------------------------------------------------------------------------
void CProgramJs::CleanRootStack()
{
	CVarLink* pVLChild = NULL;
	CVarLink* pVLTmp = NULL;
	try 
	{
		pVLChild = m_pVRootStack->m_pVLFirstChild;
		while (pVLChild)
		{
			if (pVLChild->m_sName != "window")
				RemoveChildReDefine(pVLChild->m_pVar);

			if (pVLChild->m_sName == "document" && pVLChild->m_pVar)
			{
				pVLTmp = pVLChild->m_pVar->FindChild("cookie");
				if (pVLTmp && pVLTmp->m_pVar)
				{
					pVLTmp->m_pVar->SetString(SZ_VAL_COOKIE_DEFAULT);
				}
			}

			pVLChild = pVLChild->m_pNextSibling;
		}
	}
	catch (...)
	{

	}
}

//------------------------------------------------------------------------------
// Get list szCodeJavascript in Programe 	
//------------------------------------------------------------------------------
vector<string> CProgramJs::GetLstSzCodeJs(){
	return this->m_lstsCodeJs;
}

//------------------------------------------------------------------------------
// Dump out the contents of this using trace	
//------------------------------------------------------------------------------
void CProgramJs::Trace() {
	m_pVRootStack->Trace();
}

//------------------------------------------------------------------------------
// Phan tu "prototype" la con cua pVObject 
// Tim phan tu con cua "prototype" voi dau vao la ten cua phan tu con can tim
// ----
// Return:  con tro -> lien ket bien cua phan tu can tim
//			NULL -> neu khong tim dc
//------------------------------------------------------------------------------
CVarLink* CProgramJs::FindInPrototypeClasses(CVar *pVObject, const string &sName){
	// Look for links to actual parent classes
	//  [7/21/2017 KiemVM]
	if (pVObject == NULL)
	{
		return NULL;
	}

	CVarLink *pVLParentClass = pVObject->FindChild(STR_PROTOTYPE_CLASS);
	CVarLink *pVLImplementation = NULL;

	while (pVLParentClass && pVLParentClass->m_pVar)
	{
		pVLImplementation = pVLParentClass->m_pVar->FindChild(sName);
		if (pVLImplementation)
			return pVLImplementation;
		pVLParentClass = pVLParentClass->m_pVar->FindChild(STR_PROTOTYPE_CLASS);
	}
	return NULL;
}

//------------------------------------------------------------------------------
// - Muc dich:
//		Tao doi tuong HTML element tu thong tin dau vao
// - Dau vao:
//		mapInfoHtmlElemnt["tagName"] = "???";
//		mapInfoHtmlElemnt["id"] = "???";
//		...
// - Dau ra:
//		= CVarLink* -> doi tuong html luu cac thong tin can thiet 
//		= NULL
//------------------------------------------------------------------------------
CVarLink* CProgramJs::CreateNewHTMLElement(map<string, string> mapInfoHtmlElemnt)
{
	string		sKey;
	string		sValue;
	CVarLink*	pVLReturn = NULL;
	CVarLink*	pVLHtmlElementPrototype = NULL;
	std::map<string, string>::iterator itOfMap;

	if (mapInfoHtmlElemnt.size() == 0)
		goto _COMPLETE;

	pVLHtmlElementPrototype = m_pVRootStack->FindChild("element");
	if (pVLHtmlElementPrototype &&  pVLHtmlElementPrototype->m_pVar)
	{
		pVLReturn = new CVarLink(new CVar("", SCRIPTVAR_OBJECT), STR_NAME_HTML_ELEMENT);
		if (pVLReturn->m_pVar)
		{
			pVLReturn->m_pVar->AddChild(STR_PROTOTYPE_CLASS, pVLHtmlElementPrototype->m_pVar);
		}

		for (itOfMap = mapInfoHtmlElemnt.begin(); itOfMap != mapInfoHtmlElemnt.end(); ++itOfMap)
		{
			sKey = itOfMap->first;
			sValue = itOfMap->second;
			if (sKey.size() > 0 && sValue.size() > 0)
				pVLReturn->m_pVar->AddChild(sKey, new CVar(sValue));
		}
	}
_COMPLETE:
	return pVLReturn;
}

//------------------------------------------------------------------------------
// return list Html Element is Child of pVObject
//------------------------------------------------------------------------------
vector<CVarLink*> CProgramJs::GetAllChildHtmlElement(CVar *pVObject,
	vector<CVarLink*> lstVLHtmlElement)
{
	CVar* pVObjectSearch = NULL;
	CVarLink* pVLPrototypeThisObject = NULL;
	CVar* pVHtmlElementPrototype = m_pVRootStack->FindChild("element")->m_pVar;
	CVarLink* pVLFirstChild = NULL;

	if (pVObject)
		pVLFirstChild = pVObject->m_pVLFirstChild;
	//  Duyet qua cac thanh phan HTML con
	while (pVLFirstChild)
	{
		pVObjectSearch = pVLFirstChild->m_pVar;
		if (pVObjectSearch)
		{
			// kiem tra xem day co phai la thanh phan HTML element hay ko
			pVLPrototypeThisObject = pVObjectSearch->FindChild(STR_PROTOTYPE_CLASS);
			if (pVLPrototypeThisObject &&
				pVLPrototypeThisObject->m_pVar == pVHtmlElementPrototype)
			{
				// Dua phan tu con vao danh sach
				lstVLHtmlElement.push_back(pVLFirstChild);
				// Tim tiep con cua phan tu con nay
				lstVLHtmlElement = GetAllChildHtmlElement(pVLFirstChild->m_pVar, lstVLHtmlElement);
			}
		}
		pVLFirstChild = pVLFirstChild->m_pNextSibling;
	}
	return lstVLHtmlElement;
}

//------------------------------------------------------------------------------
// Y nghia:
//	- Lay The HTML trong file .html tu cau truc jQuerySelectors
// Vao: 
//	- sJQuerySelectors: cau truc jQuerySelectors
// Ra:
//	- NULL: neu khong tim thay thanh phan HTML
//	- CVarLink*: neu tim thay thanh phan HTML theo cau truc jQuerySelectors
//------------------------------------------------------------------------------
CVarLink* CProgramJs::GetInfoHtmlElementFromJQuerySelector(string sJQuerySelectors)
{
	int				nTokenId;
	int				nPosIdTagHtml = 0;		// vi tri gia tri thuoc tinh id 
	int				nPosCharTagOpen = 0;	// vi tri ki tu '<'
	int				nPosHtmlTagOpenStart = 0, nPosHtmlTagOpenEnd = 0;	// vi tri bat dau the mo, vi tri ket thuc the mo
	int				nPosHtmlTagCloseStart = 0, nPosHtmlTagCloseEnd = 0;	// ci tri bat dau the dong, vi tri ket thuc the dong
	int				nPosSubHtmlTagOpenStart;	// vi tri bat dau the mo con 
	int				nPosSubHtmlTagCloseStart;	// vi tri bat dau the dong con
	int				nPosRWordId = 0;			// vi tri tu khoa "id"
	bool			bIsTagHidden = false;		// co bao the hidden
	bool			bValidate = false, bFoundTagHtml = false;
	string			sCodeHtml;
	string			sTagName, sTmpTagName;
	string			sAttrsTagHtml;
	string			sDataTagHtml;
	string			sTagHtml;
	string			sAttrId;
	CVarLink*		pVLReturn = NULL;
	map<string, string> mapInfoHtmlTag;
	CTokenPointer*	pTPJQuerySelect = NULL;
	CRuntimeException* pRuntimeException = NULL;

	if (sJQuerySelectors.size() == 0 || m_pFileHtmlProcess == NULL) 
		goto _COMPLETE;

	sCodeHtml = m_pFileHtmlProcess->GetCodeHtml();
	pTPJQuerySelect = new CTokenPointer(sJQuerySelectors);
	try
	{
		while (pTPJQuerySelect->m_nTokenId != TK_EOF)
		{
			nTokenId = pTPJQuerySelect->m_nTokenId;

			switch (nTokenId)
			{
			case TK_ID:
				sTagName = pTPJQuerySelect->m_sTokenStr;// Lay ten the
				pTPJQuerySelect->Match(nTokenId);		// Tim token tiep theo

				nTokenId = pTPJQuerySelect->m_nTokenId;
				if (nTokenId == '[')
				{
					// Lay thong tin thuoc tinh the
					pTPJQuerySelect->Match('[');
					nTokenId = pTPJQuerySelect->m_nTokenId;
					if (nTokenId == TK_ID && pTPJQuerySelect->m_sTokenStr == "id")
					{
						pTPJQuerySelect->Match(nTokenId);
						nTokenId = pTPJQuerySelect->m_nTokenId;
						switch (nTokenId)
						{
						case TK_EQUAL:
						case TK_XOREQUAL:
							// gia tri thuoc tinh id bat dau = string
							pTPJQuerySelect->Match(TK_XOREQUAL);
							sAttrId = pTPJQuerySelect->m_sTokenStr;
							break;
						default:
							break;
						}
					}
				}
				break;

			default:
				goto _COMPLETE;
			}

			// Tim the co gia tri = sAttrId trong file HTML
			nPosIdTagHtml = 0;
			if (sAttrId.size() > 0 && sTagName.size() > 0){
				while (nPosIdTagHtml != string::npos && !bFoundTagHtml)
				{
					// Tim xau id trong file HTML
					nPosIdTagHtml = sCodeHtml.find(sAttrId, nPosIdTagHtml);
					if (nPosIdTagHtml != string::npos)
					{
						// Tim xau ten the HTML
						nPosHtmlTagOpenStart = sCodeHtml.rfind('<' + sTagName, nPosIdTagHtml);
						nPosCharTagOpen = sCodeHtml.rfind('<', nPosIdTagHtml);
						if (nPosHtmlTagOpenStart != string::npos &&
							nPosHtmlTagOpenStart == nPosCharTagOpen)
						{
							nPosHtmlTagOpenEnd = sCodeHtml.find('>', nPosIdTagHtml);

							// Loai bo cac cap the con
							nPosSubHtmlTagOpenStart = nPosHtmlTagOpenStart;
							nPosSubHtmlTagCloseStart = nPosHtmlTagOpenStart + 1;

							while (nPosSubHtmlTagOpenStart != string::npos
								&&nPosSubHtmlTagCloseStart != string::npos
								&& nPosSubHtmlTagOpenStart < nPosSubHtmlTagCloseStart
								)
							{
								nPosSubHtmlTagOpenStart++;
								nPosSubHtmlTagCloseStart++;
								nPosSubHtmlTagOpenStart = sCodeHtml.find('<' + sTagName, nPosSubHtmlTagOpenStart);
								nPosSubHtmlTagCloseStart = sCodeHtml.find("</" + sTagName, nPosSubHtmlTagCloseStart);
							}

							if (nPosSubHtmlTagCloseStart != string::npos)
							{
								// Xuat noi dung va thuoc tinh the
								sDataTagHtml = sCodeHtml.substr(nPosHtmlTagOpenEnd + 1, nPosSubHtmlTagCloseStart - 1 - nPosHtmlTagOpenEnd);
								sAttrsTagHtml = sCodeHtml.substr(nPosHtmlTagOpenStart, nPosHtmlTagOpenEnd + 1 - nPosHtmlTagOpenStart)
									+ "</" + sTagName + '>';

								// Kiem tra tinh dung dan cua the
								mapInfoHtmlTag = m_pFileHtmlProcess->GetInfoElement(sAttrsTagHtml, bValidate);

								if (bValidate && mapInfoHtmlTag.at("tagName") == sTagName)
								{
									if ((nTokenId == TK_EQUAL && mapInfoHtmlTag.at("id") == sAttrId)
										|| (nTokenId == TK_XOREQUAL && mapInfoHtmlTag.at("id").find(sAttrId) == 0))
									{
										// Tao doi tuong html element
										bFoundTagHtml = true;
										if (CheckAttrHidden(sAttrsTagHtml))
											mapInfoHtmlTag["width"] = "0";
										mapInfoHtmlTag["innerHTML"] = sDataTagHtml;
										pVLReturn = this->CreateNewHTMLElement(mapInfoHtmlTag);
									}
								}
							}

						}
						nPosIdTagHtml += sAttrId.size();
					}
				}
			}

			// Chuyen token tiep theo
			pTPJQuerySelect->Match(pTPJQuerySelect->m_nTokenId);
		}
	}
	catch (CRuntimeException* pRE)
	{
		pRuntimeException = pRE;
	}

_COMPLETE:
	SAFE_THROW(pRuntimeException);
	return pVLReturn;
}

//------------------------------------------------------------------------------
// Cai dat che do chay trong TRY-CATCH
//------------------------------------------------------------------------------
void CProgramJs::SetRunInTryCatch()
{
	CVarLink* pVLSaveMsgException = NULL;

	m_bRunInTryCatch = true;
	pVLSaveMsgException = new CVarLink(new CVar());

	// tao 1 lien ket bien san sang luu tru thong diep exception
	m_lstVLMsgException.push_back(pVLSaveMsgException);
}

//------------------------------------------------------------------------------
// Go bo che do chay trong TRY-CATCH
//------------------------------------------------------------------------------
void CProgramJs::SetNotRunInTryCatch()
{
	CVarLink* pVLSaveMsgException = NULL;

	if (IsRunInTryCatch())
	{
		if (m_lstVLMsgException.size() == 0)
		{
			m_bRunInTryCatch = false;
			m_bHadException = false;
			return;
		}
		// Huy 1 lien ket bien luu tru thong diep exception hien tai
		pVLSaveMsgException = m_lstVLMsgException.back();
		m_lstVLMsgException.pop_back();
	}

	if (m_lstVLMsgException.size() == 0)
		m_bRunInTryCatch = false;
	m_bHadException = false;
}

//------------------------------------------------------------------------------
// Kiem tra chuong trinh co dang chay trong TRY-CATCH ?
//------------------------------------------------------------------------------
bool CProgramJs::IsRunInTryCatch(){
	return m_bRunInTryCatch;
}

//------------------------------------------------------------------------------
// Neu chua co exception thi gan gia tri cho exception
// + Set den bao excption
//------------------------------------------------------------------------------
void CProgramJs::SetMsgRunInTryCatch(int nTypeException, string sId, CVar* pData){
	CVarLink* pVLMsgException = NULL;

	if (IsRunInTryCatch() && !m_bHadException)
	{
		m_bHadException = true;
		pVLMsgException = m_lstVLMsgException.back();
		if (pVLMsgException == NULL)
			return;

		if (nTypeException == EXCEPTION_ID_NOTFOUND_IN_TRYCATCH)
		{
			// them noi dung vao bien luu exception
			pVLMsgException->ReplaceWith(new CVar(CREATE_MSG_ID_UNDEFINED(sId)));
			pVLMsgException->m_pVar->AddChild("name", new CVar("ReferenceError"));
			pVLMsgException->m_pVar->AddChild("message", new CVar(sId + " is not defined"));
		}
		if (nTypeException == EXCEPTION_STATEMENT_STRUCT_ERROR)
		{
			// them noi dung vao bien luu exception
			pVLMsgException->ReplaceWith(new CVar("Exception: Fail in state struct"));
			pVLMsgException->m_pVar->AddChild("name", new CVar("Exception"));
			pVLMsgException->m_pVar->AddChild("message", new CVar("Fail in state struct"));
		}
		else if (nTypeException == EXCEPTION_THROW_IN_TRYCATCH)
		{
			// gan noi dung cho Lien ket bien luu exception hien tai
			pVLMsgException->ReplaceWith(pData);
		}
	}
}

//------------------------------------------------------------------------------
// Gan gia tri exception  = ""
// Huy den bao exception
//------------------------------------------------------------------------------
void  CProgramJs::CleanMsgRunInTryCatch(){
	if (IsRunInTryCatch())
	{
		m_bHadException = false;

		// Xoa bo noi dung cua Lien ket bien luu exception hien tai
		if (m_lstVLMsgException.size() > 0)
			m_lstVLMsgException.back()->ReplaceWith(new CVar());
	}
}

//------------------------------------------------------------------------------
// Doc tat ca cac dinh nghia ham trong code js
//------------------------------------------------------------------------------
void CProgramJs::GetAllFuncDefineInCode(const string &sCode, int nPosStart)
{
	CVarLink *pVLFunc = NULL;
	CRuntimeException* pRunTimeException = NULL;
	CTokenPointer* pTPOld = m_pTokenPointer;	// Luu con tro lenh cu
	if (pTPOld == NULL)
		return;
	// Tao con tro lenh moi
	try
	{
		if (m_pTokenPointer)
			// có bao gồm tìm token tiếp theo bằng hàm GetNextToken()
			m_pTokenPointer = new CTokenPointer(pTPOld, pTPOld->m_nTokenStart, sCode.size()); 
		else
			// có bao gồm tìm token tiếp theo bằng hàm GetNextToken()
			m_pTokenPointer = new CTokenPointer(sCode);
	}
	catch (CRuntimeException* pRE)
	{
		pRunTimeException = pRE;
	}

	// Lap tim kiem den cuoi code
	while (m_pTokenPointer->m_nTokenId != TK_EOF)
	{
		if (m_pTokenPointer->m_nTokenId == TK_R_FUNCTION)
		{
			try	
			{
				// Lay dinh nghia ham
				pVLFunc = ParseFunctionDefinition();
				if (pVLFunc && pVLFunc->m_sName.size() > 0)
				{
					// them vao stack
					if (m_lstStack.size() > 0)
						m_lstStack.back()->AddChildNoDup(pVLFunc->m_sName, pVLFunc->m_pVar);
					else
						m_pVRootStack->AddChildNoDup(pVLFunc->m_sName, pVLFunc->m_pVar);
				}
			}
			catch (CRuntimeException* pRE)
			{
				SAFE_DELETE(pRE);
			}
		}
		m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
	}
	// Khoi phuc con tro lenh cu
	m_pTokenPointer = pTPOld;

	SAFE_THROW(pRunTimeException);
}

//------------------------------------------------------------------------------
// Kiem tra sCode co phai la code JS hay khong
//
// - Doc cac token
// + false: neu co 2 token ID, TK_STR, TK_INT, TK_R_IF  canh nhau
// + true: con lai
//------------------------------------------------------------------------------
bool CProgramJs::IsCodeJavaScript(const string &sCode)
{
	CVarLink *pVLFunc = NULL;
	CTokenPointer* pTokenPointer = NULL;	// Luu con tro lenh cu
	const int iListTolen[] =
	{
		TK_ID,
		TK_STR,
		TK_INT,
		TK_R_IF
	};
	vector<int> lstTokenNeverTogether(iListTolen, iListTolen + 4);
	bool bRet = false;
	bool bCheckToken = false;	// Co bao co phai la token can kiem tra
	int nTokenPrevious;			// Gia tri cua token truoc do
	int nCountTokenNeedCheck = lstTokenNeverTogether.size();

	try
	{
		// Tao con tro lenh moi
		pTokenPointer = new CTokenPointer(sCode);

		// Lap tim kiem den cuoi code
		while (pTokenPointer->m_nTokenId != TK_EOF)
		{
			if (bCheckToken)
			{
				// Kiem tra token hien tai = token truoc do
				// Neu == thi la loi cu phap
				if (pTokenPointer->m_nTokenId == nTokenPrevious)
				{
					goto _EXIT_FUNCTION;
				}
				else{
					bCheckToken = false;
				}
			}
			for (int i = 0; i < nCountTokenNeedCheck; i++)
			{
				if (pTokenPointer->m_nTokenId == lstTokenNeverTogether.at(i))
				{
					bCheckToken = true;
					nTokenPrevious = pTokenPointer->m_nTokenId;
					break;
				}
			}
			pTokenPointer->Match(pTokenPointer->m_nTokenId);
		}

		bRet = true;
	}
	catch (CRuntimeException* pRE)
	{
		DbgPrintLnA("Exception:: Co token khong hop le!");
		SAFE_DELETE(pRE);
	}
	catch (...)
	{
		DbgPrintLnA("Crash:: IsCodeJavaScript()");
	}

_EXIT_FUNCTION:
	return bRet;
}

//------------------------------------------------------------------------------
// Start || Continue Run code java script in "szCode"
// Bắt đầu chạy hoặc chạy tiếp code js trong khối lệnh sCode
//------------------------------------------------------------------------------
void CProgramJs::Execute(const string &sCode, bool bExecute)
{
	// Luu doan sCode vao bien cuc bo m_sCodeJS de su dung cho CheckEncodeJS - SonTDc
	// Có thể sau này không cần đoạn này nữa
	

	//bool bExecute;	// flag notice Err
	CTokenPointer *pTokenPointerOld;	// Lưu con trỏ hiện tại
	vector<CVar*> lstVStackOld;		// lưu danh sách các stack hiện tại
	CRuntimeException *pREHandleProcess = NULL;

	// save searchPointer && m_listStack when run in Function
	pTokenPointerOld = m_pTokenPointer;
	lstVStackOld = m_lstStack;

	// khi chay trong function cua onreadystatechange thi khong can create new stack
	// khi chạy trong hàm của onreadystatechange thì không cần tạo stack mới ???
	if (m_onreadystatechange != true)
	{
		// Tạo stack mới
		m_lstStack.clear();
		m_lstStack.push_back(m_pVRootStack);  // ---> RootStack
	}

	try
	{
		// khởi tạo con trỏ m_TokenPointer quét code mới
		m_pTokenPointer = new CTokenPointer(sCode);

		try
		{
			// Lấy tất cả các định nghĩa hàm trong code
			GetAllFuncDefineInCode(sCode);
		}
		catch (CRuntimeException *pRE){
			SAFE_DELETE(pRE);
		}
		
		// Thực thi code
		while (m_pTokenPointer->m_nTokenId)
		{
			Statement(bExecute);
		}
		
	}
	catch (CRuntimeException *pRE)
	{
		pREHandleProcess = pRE;
	}

	// restore searchPointer && m_listStack to continute running
	m_pTokenPointer = pTokenPointerOld;
	m_lstStack = lstVStackOld;

	SAFE_THROW(pREHandleProcess);
}


//------------------------------------------------------------------------------
// xu ly triet de exception xay ra khi thuc hien cac cau lenh
//
//	+ base()			// thuc hien chuoi toan tu
//	+ addChildNoDup()	// them phan tu con 
//	+ match(;)			// tim token tiep theo theo neu tk hien tai  = ';'
//-------------------
// => tra ve 1 bien CVarLink = KQ tinh toan cho cau lenh: eavl(...)
// !! giai phong: return.m_pVar
//------------------------------------------------------------------------------
CVarLink* CProgramJs::EvaluateComplex(const string &sCode)
{
	CTokenPointer *pTokenPointerOld = m_pTokenPointer;
	vector<CVar*> lstVStackOld = m_lstStack;
	CRuntimeException * pREHandleProcess = NULL;
	CVarLink *pVLTmp = NULL;

	bool bExecute = true;
	bool bExecuteInEval = true;

	// tao con tro lenh moi + stack moi -> thuc hien code moi
	// init new m_TokenPointer for new CodeData
	m_pTokenPointer = new CTokenPointer(sCode);

	// Create New Stack
	m_lstStack.clear();
	m_lstStack.push_back(m_pVRootStack);

	// Doc tat ca dinh nghia ham trong code
	GetAllFuncDefineInCode(sCode);

	try 
	{
		do
		{
			pVLTmp = Base(bExecute, bExecuteInEval);

			// fix err tiny-JS khong them bien FUNCTION trong ham base
			if (pVLTmp && pVLTmp->m_pVar && pVLTmp->m_pVar->m_nTypeVar == SCRIPTVAR_FUNCTION)
				m_pVRootStack->AddChildNoDup(pVLTmp->m_sName, pVLTmp->m_pVar);

		} while (m_pTokenPointer && m_pTokenPointer->m_nTokenId != TK_EOF);
	}
	catch (CRuntimeException *pRE)
	{
		// hien thi loi
		if (pRE)
			OutputDebug(string("Error ") + pRE->m_sMessage + " at " + m_pTokenPointer->GetPosition());
		pREHandleProcess = pRE;
	}

	// giai phong bo nho, giai phong con tro lenh + khoi phuc stack ban dau
	m_pTokenPointer = pTokenPointerOld;
	m_lstStack = lstVStackOld;

	SAFE_THROW(pREHandleProcess);
	if (pVLTmp)
		return pVLTmp;
	else
		// return undefined...
		return new CVarLink(new CVar());
}

//------------------------------------------------------------------------------
// Thuc hien 1 khoi lenh JS trong sCode
//------------------------------------------------------------------------------
string CProgramJs::Evaluate(const string &sCode)
{
	// fix:: Xoa gia tri VLink tra ve
	string sReturn;
	CVarLink* pVLReturn = EvaluateComplex(sCode);

	if (pVLReturn && pVLReturn->m_pVar)
		sReturn = pVLReturn->m_pVar->GetString();

	return sReturn;
}

//------------------------------------------------------------------------------
// Them vao con pVLstArgument danh sach tham so 
// function TenHam(thamSo1, thamSo2);
//------------------------------------------------------------------------------
void CProgramJs::ParseFunctionArguments(CVar *pVLstArgument, bool bIsReDefine)
{
	CVarLink* pVLArg = NULL;

	if (m_pTokenPointer == NULL)
		return;

	m_pTokenPointer->Match('(');
	while (m_pTokenPointer->m_nTokenId != ')')
	{
		pVLArg = NULL;
		if (pVLstArgument)
			pVLArg = pVLstArgument->AddChildNoDup(m_pTokenPointer->m_sTokenStr, NULL, TRUE);
		if (pVLArg)
			if (!bIsReDefine)
				pVLArg->SetNotReDefine();	// Cai dat cac ham API mac dinh

		m_pTokenPointer->Match(TK_ID);
		if (m_pTokenPointer->m_nTokenId != ')')
			m_pTokenPointer->Match(',');
	}
	m_pTokenPointer->Match(')');
}

//------------------------------------------------------------------------------
// Lay danh sach phan tu con luu vao pVList
//------------------------------------------------------------------------------
void CProgramJs::GetListChildArray(CVar *pVList)
{
	int nIndex = 0;
	char idx_str[16]; // big enough for 2^32
	bool bExecute = true;
	CVarLink *pVLChild = NULL;

	if (m_pTokenPointer == NULL || m_pTokenPointer->m_nTokenId != '(')
		return;

	m_pTokenPointer->Match('(');
	while (m_pTokenPointer->m_nTokenId != ')')
	{
		_snprintf_s(idx_str, sizeof(idx_str),_TRUNCATE, "%d", nIndex);
		pVLChild = Base(bExecute);
		if (pVLChild)
			pVList->AddChild(idx_str, pVLChild->m_pVar);

		// no need to clean here, as it will definitely be used
		if (m_pTokenPointer->m_nTokenId != ')')
			m_pTokenPointer->Match(',');
		nIndex++;
	}
	m_pTokenPointer->Match(')');
}

//------------------------------------------------------------------------------
// 	programJs->AddFuncNative("function exec(jsCode)", scExec, programJs); // execute the given code
//  programJs->AddFuncNative("function String.charAt(pos)", scStringCharAt, 0);
//  string co pt con  = charAt, -> 3 phan tu con = this, pos, return
//  programJs->AddFuncNative("function element.setAttribute(szNameAttr,szValAttr)", scElementSetAttribute, programJs);
//------------------------------------------------------------------------------
void CProgramJs::AddFuncNative(const string &sFuncDesc, JSCallback pJSCallBack, void *pUserData)
{
	string sFuncName;

	CVar *pVBase = m_pVRootStack;
	CVar *pVFunc = NULL;
	CVarLink *pVLLink = NULL;
	CTokenPointer *pTPOld = m_pTokenPointer;

	if (pVBase == NULL)
		return;

	m_pTokenPointer = new CTokenPointer(sFuncDesc);
	m_pTokenPointer->Match(TK_R_FUNCTION);
	sFuncName = m_pTokenPointer->m_sTokenStr;
	m_pTokenPointer->Match(TK_ID);

	/* Check for dots, we might want to do something like function String.substring ... */
	while (m_pTokenPointer->m_nTokenId == '.')
	{
		m_pTokenPointer->Match('.');
		pVLLink = pVBase->FindChild(sFuncName);

		if (!pVLLink) 
		{
			// if it doesn't exist, make an object class
			pVLLink = pVBase->AddChild(sFuncName, new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), TRUE);
			if (pVLLink)
				pVLLink->SetNotReDefine();
		}
		if (pVLLink)
			pVBase = pVLLink->m_pVar;
		sFuncName = m_pTokenPointer->m_sTokenStr;
		m_pTokenPointer->Match(TK_ID);
	}

	if (pVBase == NULL)
		return;
	pVLLink = pVBase->FindChild(sFuncName);
	if (pVLLink && pVLLink->m_pVar)
	{
		pVFunc = pVLLink->m_pVar;
		pVFunc->m_nTypeVar = SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE;
	}
	else
	{
		// Tao doi tuong Funtion Native
		pVFunc = new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
		(pVBase->AddChildNoDup(sFuncName, pVFunc, TRUE))->SetNotReDefine();
	}

	pVFunc->SetCallback(pJSCallBack, pUserData);	// Cai dia chi ham gia lap
	ParseFunctionArguments(pVFunc, false);

	// Cai du lieu khi su dung phuong thuc Js: Object.toString (Code : GetString())
	pVFunc->SetDataString("function " + sFuncName + "() { [native code] }");

	m_pTokenPointer = pTPOld;

}

//------------------------------------------------------------------------------
// Them dinh nghia ham constructor cho 1 lop
// Cac lop phai co truoc o trong m_pVRootStack
// Array, String, Object...
//------------------------------------------------------------------------------
void CProgramJs::SetFuncNative(const string &sFuncDesc, JSCallback pJSCallBack, void *pUserData)
{
	CVar* pVFunc = NULL;
	CTokenPointer *pTPOld = m_pTokenPointer;
	string sFuncName;

	m_pTokenPointer = new CTokenPointer(sFuncDesc);
	sFuncName = m_pTokenPointer->m_sTokenStr;

	pVFunc = m_pVRootStack->FindChild(sFuncName)->m_pVar;
	if (pVFunc)
	{
		// Cai kieu du lieu Function Native cho lop String
		pVFunc->m_nTypeVar = SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE;

		// Cai du lieu khi su dung phuong thuc Js: Object.toString (Code : GetString())
		pVFunc->SetDataString("function " + sFuncName + "() { [native code] }");

		// Cai dia chi ham gia lap
		pVFunc->SetCallback(pJSCallBack, pUserData);
	}
}

//------------------------------------------------------------------------------
// Them thuoc tinh cho 1 doi tuong
//------------------------------------------------------------------------------
void CProgramJs::AddAttrNative(const string &sPathAttr, int nTypeAttr)
{
	AddAttrNative(sPathAttr, new CVar(STR_BLANK_DATA, nTypeAttr));
}

//------------------------------------------------------------------------------
// Them thuoc tinh cho doi tuong. 
// Vao: Dia chi thuoc tinh + du lieu thuoc tinh
// Ra: Thuoc tinh duoc them vao.
// ...
// vd: programJs->AddAttrNative("element.style.top", CVar* pVar);
//------------------------------------------------------------------------------
void CProgramJs::AddAttrNative(const string &sPathAttr, CVar* pVAttr)
{
	CTokenPointer *pTPOld = m_pTokenPointer;
	CRuntimeException* pRE = NULL;
	CVar* pVBase = m_pVRootStack;
	CVar* pVTmpValue = NULL;
	CVarLink *pVLNewAttr = NULL;
	try
	{
		if (pVBase == NULL)
			return;

		m_pTokenPointer = new CTokenPointer(sPathAttr);

		if (m_pTokenPointer->m_nTokenId == TK_ID)
		{
			// Lan luot truy xuat vao ds thuoc tinh
			do
			{
				pVLNewAttr = pVBase->FindChild(m_pTokenPointer->m_sTokenStr);

				if (!pVLNewAttr && m_pTokenPointer->TryWatchNextToken() != 0)
				{
					// Neu chua co thuoc tinh, mac dinh them thuoc tinh VAR_UNDEFINE
					pVTmpValue = new CVar();
					pVLNewAttr = pVBase->AddChild(m_pTokenPointer->m_sTokenStr, pVTmpValue, TRUE);
					if (pVLNewAttr)
						pVLNewAttr->SetNotReDefine();
				}
				else if (!pVLNewAttr && m_pTokenPointer->TryWatchNextToken() == 0){
					// Them thuoc tinh cuoi cung
					pVLNewAttr = pVBase->AddChild(m_pTokenPointer->m_sTokenStr, pVAttr, TRUE);
					if (pVLNewAttr)
						pVLNewAttr->SetNotReDefine();
					break;
				}

				m_pTokenPointer->Match(TK_ID);

				if (m_pTokenPointer->m_nTokenId == '.')
					m_pTokenPointer->Match('.');
				else if (m_pTokenPointer->m_nTokenId)
				{
					m_pTokenPointer = pTPOld;
					pRE = new CRuntimeException("Sai cu phap them thuoc tinh!");
					throw pRE;
				}
				if (pVLNewAttr)
					pVBase = pVLNewAttr->m_pVar;
			} while (m_pTokenPointer->m_nTokenId != TK_EOF);
		}
	}

	// CATCH
	catch (CRuntimeException *e)
	{
		pRE = e;
	}

	// FINALLY
	m_pTokenPointer = pTPOld;

	SAFE_THROW(pRE);
}

//------------------------------------------------------------------------------
// set tat ca cac thuoc tinh con tro cua doi tuong = NULL
// duoc goi khi tao doi tuong CProgramJs moi.
//------------------------------------------------------------------------------
void CProgramJs::InitNullForPropertyPointer()
{
	this->m_pVLWindowOnload = NULL;
	this->m_pVLDcmtOnmousemove = NULL;
	this->m_pVLDStatus = NULL;
	this->m_pFileHtmlProcess = NULL;
	this->m_pTokenPointer = NULL;
	this->m_szDataFile = NULL;
	this->m_pVRootStack = NULL;
	this->m_pVLStartReset = NULL;
}

//------------------------------------------------------------------------------
// Doc dinh nghia ham -> tra ve 1 lien ket bien kieu function
// vd: function abc(){t = 1;} 
// vd: function () {t = 2;} 
//------------------------------------------------------------------------------
CVarLink *CProgramJs::ParseFunctionDefinition()
{
	bool bNoexecute;
	int nFuncBegin;
	int nSize = 0;
	int nDefineFuncBegin = 0;
	string sFuncName;
	string sDefineFunc; // thuoc tinh luu dinh nghia ham
	CVarLink *pVLFunc = NULL;
	CRuntimeException *pREHandleProcess = NULL;

	if (m_pTokenPointer == NULL)
		return NULL;

	nDefineFuncBegin = m_pTokenPointer->m_nTokenStart;
	try
	{
		// get Function Name
		// actually parse a function...
		m_pTokenPointer->Match(TK_R_FUNCTION);			// giá trị trả về trong m_pTokenPointer->m_sTokenStr là tên hàm
		sFuncName = STR_TEMP_NAME;						//
														// ban đầu gán giá trị tên hàm mặc định là ""
		/* we can have functions without names */		//	Nếu sau khi hàm Match bên trên trả về giá trị tên hàm không có 
		if (m_pTokenPointer->m_nTokenId == TK_ID)		//	Trường hợp function () {} -> không có tên hàm thì mặc định gán bằng ""
		{												//	Nếu có thì sẽ gán bằng tên trả về m_pTokenPointer->m_sTokenStr
			sFuncName = m_pTokenPointer->m_sTokenStr;	//
			m_pTokenPointer->Match(TK_ID);				//
		}												//

		// Khơi tạo giá trị VarLink để lưu dữ liệu của hàm với tên đầu vào
		// thêm giá trị này vào g_VLToRootStack là stack chứa các hàm -> kể cả hàm mặc định lẫn hàm khởi tạo
		pVLFunc = new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION), sFuncName);

		// get Function Arguments
		ParseFunctionArguments(pVLFunc->m_pVar); // Lấy thông tin các tham số
		nFuncBegin = m_pTokenPointer->m_nTokenStart; // Khởi tạo giá trị ban đầu của hàm
		bNoexecute = false;

		// get szCode of Function
		BlockCode(bNoexecute); // Lấy thông tin khối code trong hàm 

		// luu string szCode dinh nghia Function 
		// lam du lieu cho thuoc tinh arguments.callee = "function tenHam() {...}"
		sDefineFunc = m_pTokenPointer->GetSubString(nDefineFuncBegin);
		nSize = sDefineFunc.size();
		if (nSize > 0)
		{
			// luu tru lai dinh nghia ham -> CVar::m_pBuffDefineFunc
			if (pVLFunc->m_pVar)
			{
				pVLFunc->m_pVar->m_pBuffDefineFunc = new BYTE[nSize + 1];
				memcpy(pVLFunc->m_pVar->m_pBuffDefineFunc, sDefineFunc.c_str(), nSize);
				pVLFunc->m_pVar->m_pBuffDefineFunc[nSize] = 0;
			}
		}

		// luu code thuc su cua than ham
		if (pVLFunc->m_pVar)
		{
			pVLFunc->m_pVar->m_sData = m_pTokenPointer->GetSubString(nFuncBegin);
		}
	}
	catch (CRuntimeException* pRE){
		pREHandleProcess = pRE;
	}

	SAFE_THROW(pREHandleProcess);
	return pVLFunc;

}


CVarLink *CProgramJs::RunFunction(CVarLink *pVLFunc,
	CVar *pVParent, CVar* pVArgumentsValue, bool bExecuteInEval)
{
	bool bExe = true;
	BOOLEAN bIsPushedLocalStack = FALSE;
	CVar *pVStackLocalFunction = NULL;
	CVarLink *pVLFirstArgument = NULL;
	CVarLink *pVLArgValue = NULL;

	CVarLink *pVLReturn = new CVarLink(new CVar()); // cai dat bien tra ve 
	CVarLink *pVLFuncReturn = NULL;

	CTokenPointer *pTokenPointerOld = NULL;
	CRuntimeException *pRuntimeException = NULL;

	//..........................................................................
	// Thuc hien code trong ham
	try
	{
		if (!pVLFunc || !pVLFunc->m_pVar || !pVLFunc->m_pVar->IsFunction())
		{
			// Cai dat exception neu codeJS dang chay trong try-catch
			if (pVLFunc)
			{
				SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, pVLFunc->m_sName, NULL);

				if (IsRunInTryCatch())
					throw new CRuntimeException("Expecting '" + pVLFunc->m_sName +
					"' to be a function", EXCEPTION_ID_NOTFOUND_IN_TRYCATCH);
				else
					throw new CRuntimeException("Expecting '" + pVLFunc->m_sName +
					"' to be a function");
			}
			else
			{
				SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, "Unknow", NULL);

				if (IsRunInTryCatch())
					throw new CRuntimeException("Expecting '" 
					"' to be a function", EXCEPTION_ID_NOTFOUND_IN_TRYCATCH);
				else
					throw new CRuntimeException("Expecting '"
					"' to be a function");
			}
		}

		// Khoi tao New Stack cho ham - luu cac bien cuc bo su dung trong ham
		pVStackLocalFunction = new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION);
		if (pVParent)
			pVStackLocalFunction->AddChildNoDup("this", pVParent);

		// Cai dat gia tri cua cac tham so dau vao 
		// Cac bien cuc bo trong ham su dung duoc huy bo bang huy(pVLocalStack)
		pVLFirstArgument = pVLFunc->m_pVar->m_pVLFirstChild;

		// tinh gia tri tham so dau vao
		if (pVArgumentsValue)
			pVLArgValue = pVArgumentsValue->m_pVLFirstChild;

		while (pVLFirstArgument)
		{
			if (pVLArgValue == NULL)
				// Neu mang gia tri ket thuc - coi nhu tham so undefined
				break;

			if (pVLArgValue->m_pVar && pVLArgValue->m_pVar->IsBasic())
				// pass by value
				// Neu tham so ko co ds con -> cop gia tri du lieu cho t.so
				pVStackLocalFunction->AddChild(pVLFirstArgument->m_sName, pVLArgValue->m_pVar->DeepCopy());
			else
				// pass by reference
				// Neu t.so co ds con -> truyen du lieu cua t.so 
				pVStackLocalFunction->AddChild(pVLFirstArgument->m_sName, pVLArgValue->m_pVar);

			// Ket thuc gan gia tri cho loi goi ham
			pVLFirstArgument = pVLFirstArgument->m_pNextSibling;
			pVLArgValue = pVLArgValue->m_pNextSibling;
		}

		// gan gia tri cho cac bien trong stack => thuc hien ham!
		// add arguments.callee ....................................
		if (!pVLFunc->m_pVar->IsNative() && pVLFunc->m_pVar->m_pBuffDefineFunc != 0)
		{
			pVLArgValue = pVStackLocalFunction->AddChild(STR_ARGUMENTS_VAR);
			pVLArgValue->m_pVar->AddChild("callee", new CVar(pVLFunc->m_pVar->GetStringDefineFunc()));
		}
		pVLFuncReturn = pVStackLocalFunction->AddChild(STR_RETURN_VAR);

		// add the function's execute space to the symbol table so we can recurse
		m_lstStack.push_back(pVStackLocalFunction);
		bIsPushedLocalStack = TRUE;

		if (pVLFunc->m_pVar->IsNative())
		{
			// func la cac API da duoc dinh nghia tu truoc
			if (pVLFunc->m_pVar->m_fnCallback)
				pVLFunc->m_pVar->m_fnCallback(pVStackLocalFunction, pVLFunc->m_pVar->m_fnCallbackUserData);
		}
		else 
		{
			// func la cac ham file JS tu dinh nghia
			/* we just want to execute the block, but something could
			* have messed up and left us with the wrong ScriptLex, so
			* we want to be careful here... */
			pTokenPointerOld = m_pTokenPointer;
			m_pTokenPointer = new CTokenPointer(pVLFunc->m_pVar->GetString());

			try
			{
				// Doc tat ca dinh nghia ham trong code
				GetAllFuncDefineInCode(m_pTokenPointer->GetCode());
			}
			catch (CRuntimeException *pRE)
			{
				SAFE_DELETE(pRE);
			}

			// thuc hien code than ham
			BlockCode(bExe);
		}

		// Luu lai gia tri tra ve truoc khi xoa LocalStack
		pVLReturn->ReplaceWith(pVLFuncReturn);
	}

	//..........................................................................
	// Xu ly cac ngoai le xay ra 
	catch (CRuntimeException *pRE)
	{
		pRuntimeException = pRE;
	}

	// or catch(...) to catch all exceptions
	catch (invalid_argument&){
		pRuntimeException = new CRuntimeException("_LOI Crash: Tham so dau vao ham stoi()!");
	}

	// Loi tran bo nho 
	catch (out_of_range&){
		pRuntimeException = new CRuntimeException("_LOI Crash: Tran bo nho()!");
	}

	catch (...){
		pRuntimeException = new CRuntimeException("_LOI Crash: exception C++");
	}

	//..........................................................................
	// giai phong du lieu + stack cuc bo cua ham

	if (pTokenPointerOld){
		// Khoi phuc con tro lenh
		m_pTokenPointer = pTokenPointerOld;
	}

	if (bIsPushedLocalStack)
		m_lstStack.pop_back();

	SAFE_THROW(pRuntimeException);
	return pVLReturn;
}

//------------------------------------------------------------------------------
// M?c dích:	Thêm 1 d?i tu?ng Function du?c cài d?t CallBack theo th?i gian vào 
//				danh sách.
// ----------------
// Vào:			Con tr? d? li?u d?i tu?ng function.
//------------------------------------------------------------------------------
void CProgramJs::AddFuncCallBackBySetInterval(CVar* pVarFunction)
{
	if (CheckFuncCallBackBySetInterval(pVarFunction) == FALSE && pVarFunction)
		m_lstFuncCallBackBySetInterval.push_back(pVarFunction);
}

//------------------------------------------------------------------------------
// Mục đích: kiểm tra đối tượng Function đã có trong DS Function Callback.
// ----------------
// Vào:	Con trỏ dữ liệu đối tượng function cần kiểm tra.
// Ra:	- True (nếu có) / False (n?u ko)
//------------------------------------------------------------------------------
BOOL CProgramJs::CheckFuncCallBackBySetInterval(CVar* pVarFunction)
{
	BOOL bRet = FALSE;
	for (size_t i = 0; i < m_lstFuncCallBackBySetInterval.size(); i++)
	{
		if (m_lstFuncCallBackBySetInterval.at(i) == pVarFunction)
		{
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}

//------------------------------------------------------------------------------
// 
/** Handle a function call (assumes we've parsed the function name and we're
* on the start bracket). 'parent' is the object that contains this method,
* if there was one (otherwise it's just a normnal function).
*/
//------------------------------------------------------------------------------
CVarLink *CProgramJs::FunctionCall(bool &bExecute, CVarLink *pVLFunc, CVar *pVParent, bool bExecuteInEval)
{
	BOOLEAN bIsPushedLocalStack = FALSE;
	BOOLEAN bFunctionNDC = FALSE;
	CVar *pVStackLocalFunction = NULL;
	CVarLink *pVLFirstArgument = NULL;
	CVarLink *pVLArgValue = NULL;

	CVarLink *pVLReturn = NULL; // cai dat bien tra ve 
	CVarLink *pVLFuncReturn = NULL;

	CTokenPointer *pTokenPointerOld = NULL;
	CRuntimeException *pRuntimeException = NULL;

	if (bExecute)
	{
		// Thuc hien code trong ham
		//......................................................................
		try	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		{
			if (!pVLFunc || !pVLFunc->m_pVar || !pVLFunc->m_pVar->IsFunction())
			{
				// Ham chua duoc dinh nghia -> bo qua thuc hien lenh khac
				m_pTokenPointer->JumpOutFuncCall();
				m_pTokenPointer->Match(')');
				//nhung ham da chua gia tri tra ve nhung chua duoc khai bao
				if (pVLFunc->m_pVar->m_sData == NDC_TODAY_GOODDAY)
				{
					bFunctionNDC = TRUE;
					pVLReturn = new CVarLink(pVLFunc->m_pVar);
				}
				// Cai dat exception neu codeJS dang chay trong try-catch
				if (!bFunctionNDC)
				{
					if (pVLFunc)
					{
						SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, pVLFunc->m_sName, NULL);

						if (IsRunInTryCatch())
							throw new CRuntimeException("Expecting '" + pVLFunc->m_sName +
							"' to be a function", EXCEPTION_ID_NOTFOUND_IN_TRYCATCH);
						else
							throw new CRuntimeException("Expecting '" + pVLFunc->m_sName +
							"' to be a function");
					}
					else
					{
						SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, "Unknow", NULL);

						if (IsRunInTryCatch())
							throw new CRuntimeException("Expecting '"
							"' to be a function", EXCEPTION_ID_NOTFOUND_IN_TRYCATCH);
						else
							throw new CRuntimeException("Expecting '"
							"' to be a function");
					}
				}
			}
			if (!bFunctionNDC)
			{
				// Khoi tao New Stack cho ham - luu cac bien cuc bo su dung trong ham
				pVStackLocalFunction = new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION);
				if (pVParent)
					pVStackLocalFunction->AddChildNoDup("this", pVParent); // Thêm vào stack phần tử this sao cho nó  khônglặp lại với phần tử this có sẵn trong đó

				// Cai dat gia tri cua cac tham so dau vao 
				// Cac bien cuc bo trong ham su dung duoc huy bo bang huy(pVLocalStack)
				if (pVLFunc && pVLFunc->m_pVar)
					pVLFirstArgument = pVLFunc->m_pVar->m_pVLFirstChild;
				if (pVLFirstArgument && pVLFirstArgument->m_sName == "ARRAY")
				{
					// So luong tham so khong co dinh
					pVLArgValue = pVStackLocalFunction->AddChild(pVLFirstArgument->m_sName);
					if (pVLArgValue && pVLArgValue->m_pVar)
					{
						pVLArgValue->m_pVar->SetArray();
						GetListChildArray(pVLArgValue->m_pVar);
					}
				}
				else
				{
					// So luong tham so co dinh
					m_pTokenPointer->Match('(');
					while (pVLFirstArgument)
					{
						// tinh gia tri tham so dau vao
						// trả về tên tham số trong pVLArgValue->m_sName, giá trị trong m_pVar
						pVLArgValue = Base(bExecute, bExecuteInEval);
						if (bExecute) 
						{
							if (pVLArgValue && pVLArgValue->m_pVar && pVLArgValue->m_pVar->IsBasic())
								// pass by value
								// Neu tham so ko co ds con -> cop gia tri du lieu cho t.so
							{
								if (pVStackLocalFunction)
									pVStackLocalFunction->AddChild(pVLFirstArgument->m_sName, pVLArgValue->m_pVar->DeepCopy());
							}
							else
								// pass by reference
								// Neu t.so co ds con -> truyen du lieu cua t.so 
							{
								if (pVStackLocalFunction)
									pVStackLocalFunction->AddChild(pVLFirstArgument->m_sName, pVLArgValue->m_pVar);
							}
						}

						// Ket thuc gan gia tri cho loi goi ham
						if (m_pTokenPointer->m_nTokenId == ')')
							break;
						// cua fix rieng cho mau JS.eIframeDownloader....[NDC]
						else if ((m_pTokenPointer->m_nTokenId == ',') && (pVLFirstArgument->m_pNextSibling == 0))
						{
							m_pTokenPointer->Match(',');
							while (m_pTokenPointer->m_nTokenId == TK_INT
								|| m_pTokenPointer->m_nTokenId == ',')
							{
								m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

							}
							if (m_pTokenPointer->m_nTokenId == ')')
								break;
						}
						else
						{
							m_pTokenPointer->Match(',');

						}
						pVLFirstArgument = pVLFirstArgument->m_pNextSibling;
					}
					m_pTokenPointer->Match(')');
				}

				// gan gia tri cho cac bien trong stack => thuc hien ham!
				// add arguments.callee ....................................
				if (pVLFunc && pVLFunc->m_pVar && !pVLFunc->m_pVar->IsNative() && pVLFunc->m_pVar->m_pBuffDefineFunc != 0)
				{
					pVLArgValue = pVStackLocalFunction->AddChild(STR_ARGUMENTS_VAR);
					pVLArgValue->m_pVar->AddChild("callee", new CVar(pVLFunc->m_pVar->GetStringDefineFunc()));
				}
				// add the function's execute space to the symbol table so we can recurse
				pVLFuncReturn = pVStackLocalFunction->AddChild(STR_RETURN_VAR);
				m_lstStack.push_back(pVStackLocalFunction);
				bIsPushedLocalStack = TRUE;

				if (pVLFunc && pVLFunc->m_pVar && pVLFunc->m_pVar->IsNative())
				{
					// func la cac API da duoc dinh nghia tu truoc
					if (pVLFunc->m_pVar->m_fnCallback)
						pVLFunc->m_pVar->m_fnCallback(pVStackLocalFunction, pVLFunc->m_pVar->m_fnCallbackUserData);
				}
				else 
				{
					// func la cac ham file JS tu dinh nghia
					/* we just want to execute the block, but something could
					* have messed up and left us with the wrong ScriptLex, so
					* we want to be careful here... */
					pTokenPointerOld = m_pTokenPointer;
					if (pVLFunc && pVLFunc->m_pVar)
						m_pTokenPointer = new CTokenPointer(pVLFunc->m_pVar->GetString());

					try
					{
						// Doc tat ca dinh nghia ham trong code
 						if (GetCurrentStackSize() > STACK_SIZE) 
 							throw std::out_of_range("Tran Stack");
						GetAllFuncDefineInCode(m_pTokenPointer->GetCode());
					}
					catch (CRuntimeException *pRE){
						SAFE_DELETE(pRE);
					}

					// thuc hien code than ham
					BlockCode(bExecute);

					// because return will probably have called this, and set execute to false
					bExecute = true;
				}

				// Luu lai gia tri tra ve truoc khi xoa LocalStack
				if (pVLFuncReturn)
					pVLReturn = new CVarLink(pVLFuncReturn->m_pVar);
			}
		}
		catch (CRuntimeException *pRE)	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		{
			// bat exception huy "stackLocalFunction"
			pRuntimeException = pRE;
		}

		// or catch(...) to catch all exceptions
		catch (invalid_argument&)
		{
			pRuntimeException = new CRuntimeException("_LOI Crash: Tham so dau vao ham stoi()!");
		}

		// Loi tran bo nho 
		catch (out_of_range&)
		{
			pRuntimeException = new CRuntimeException("_LOI Crash: Tran bo nho()!");
		}

		catch (...)
		{
			pRuntimeException = new CRuntimeException("_LOI Crash: exception C++");
		}

		// FINALLY >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>.
		// giai phong du lieu + stack cuc bo cua ham

		if (pTokenPointerOld)
		{
			// Khoi phuc con tro lenh
			m_pTokenPointer = pTokenPointerOld;
		}

		if (bIsPushedLocalStack)
			m_lstStack.pop_back();

		/* get the real return var before we remove it from our function */
		//  		if (pVLFuncReturn)
		//  			pVLReturn = new CVarLink(pVLFuncReturn->m_pVar);

		SAFE_THROW(pRuntimeException);
		return pVLReturn;
	}

	else 
	{
		// Cai dat dinh nghia ham
		//......................................................................
		// dinh nghia cac tham so dau vao -> dsCon cua Ham
		// function, but not executing - just parse args and be done
		m_pTokenPointer->Match('(');
		while (m_pTokenPointer->m_nTokenId != ')')
		{
			pVLArgValue = Base(bExecute, bExecuteInEval);
			if (m_pTokenPointer->m_nTokenId != ')')
				m_pTokenPointer->Match(',');
		}

		// luu szCode than ham
		m_pTokenPointer->Match(')');
		if (m_pTokenPointer->m_nTokenId == '{') 
		{
			BlockCode(bExecute);
		}

		/* function will be a blank scriptvarlink if we're not executing,
		* so just return it rather than an alloc/free */
		if (pVLFunc)
			return new CVarLink(pVLFunc->m_pVar, pVLFunc->m_sName);
		else
			return new CVarLink(NULL, "");
	}
}


//------------------------------------------------------------------------------
// Xu ly toan tu ke tiep Factor() 
// 
//!! Delete pVLObjectCall neu khong nam trong stack
// -----
// bool &bExecute: co bao doan Code nay co quyen thuc thi hay khong! 
// CVarLink* pVLObjectCall: lien ket bien toi doi tuong goi toan tu '(','.','['
// CVar* pVParent: du lieu doi tuong cha cua lien ket bien.
// bool bExecuteInEval: Co dang thuc hien code trong Eval hay ko.
//------------------------------------------------------------------------------
CVarLink *CProgramJs::FactorAdvance(bool &bExecute, CVarLink* pVLObjectCall, CVar* pVParent, bool bExecuteInEval)
{
	int nLenghtArray = 0;
	size_t iString = 0;
	bool bStringInt = false;
	string sNameChild;
	string sDataString;
	string sString;
	CVar* pVTmpValue = NULL;
	CVar* pVTmpValueString = NULL;
	CVarLink* pVLArraytoString = NULL;
	CVarLink* pVLIndexOfArray = NULL;
	CVarLink* pVLChild = NULL;
	CVarLink* pVLChildInPrototype = NULL;
	CVarLink* pVLChildInArray = NULL;
	CVarLink* pVLReturn = NULL;
	CVarLink* pVLObjectBeginCall = pVLObjectCall;

	CRuntimeException* pRunException = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(NULL, "");

	try
	{
		while (m_pTokenPointer->m_nTokenId == '('
			|| m_pTokenPointer->m_nTokenId == '.'
			|| m_pTokenPointer->m_nTokenId == '[')
		{
			// Function Call ---------------------------------------------------
			// m_szTokenStr la ten cua 1 ham da duoc dinh nghia || API 
			// exception neu khong tim duoc ham trong stack.
			if (m_pTokenPointer->m_nTokenId == '(')
			{
				// luu ten ham goi
				if (pVLObjectCall->m_sAliasName != "")
					m_sCodeJS += " " + pVLObjectCall->m_sAliasName +"  ";
				else 
					m_sCodeJS += " " + pVLObjectCall->m_sName + " ";
				pVLObjectCall = FunctionCall(bExecute, pVLObjectCall, pVParent, bExecuteInEval);
				pVLObjectBeginCall = pVLObjectCall;

			}

			//  Record Access --------------------------------------------------
			// m_szTokenStr la ten cua 1 bien luu trong Stack
			else if (m_pTokenPointer->m_nTokenId == '.')
			{
				// xu ly: ID.SubId ... 
				// tinh toan voi cac "thuoc tinh || phuong thuc" con cua bien ID
				m_pTokenPointer->Match('.');
				if (bExecute)
				{
					m_sCodeJS += " . "; // Luu ID.SubId neu dang o trang thai Execute 
					sNameChild = m_pTokenPointer->m_sTokenStr;
					m_sCodeJS += " " + sNameChild + " "; // Luu ten thuoc tinh || phuong thuc cua bien ID
					if (pVLObjectCall && pVLObjectCall->m_pVar)
						pVLChild = pVLObjectCall->m_pVar->FindChild(sNameChild);
					if (!pVLChild)
					{
						// tim trong prototype cua doi tuong
						if (pVLObjectCall)
							pVLChildInPrototype = FindInParentClasses(pVLObjectCall->m_pVar, sNameChild);

						if (pVLChildInPrototype && pVLChildInPrototype->m_pVar)
						{
							if (!pVLChildInPrototype->m_pVar->IsFunction())

								// them thuoc tinh rieng cho doi tuong nay 
								// neu thuoc tinh co trong prototy
								// va thuoc tinh khong phai la dia chi ham
							if (pVLObjectCall && pVLObjectCall->m_pVar && pVLChildInPrototype->m_pVar->m_nTypeVar == SCRIPTVAR_DOUBLE)
							{
								pVLChild = pVLObjectCall->m_pVar->AddChild(sNameChild,
									new CVar(pVLChildInPrototype->m_pVar->m_fData));
							}
							else if (pVLObjectCall && pVLObjectCall->m_pVar && pVLChildInPrototype->m_pVar->m_nTypeVar == SCRIPTVAR_INTEGER)
							{
								pVLChild = pVLObjectCall->m_pVar->AddChild(sNameChild,
									new CVar(pVLChildInPrototype->m_pVar->m_lData));
							}
							else
							{
								if (pVLObjectCall && pVLObjectCall->m_pVar)
									pVLChild = pVLObjectCall->m_pVar->AddChild(sNameChild,
										new CVar(pVLChildInPrototype->m_pVar->m_sData,
										pVLChildInPrototype->m_pVar->m_nTypeVar));
							}

							else
								// neu la dia chi ham 
								pVLChild = pVLChildInPrototype;
						}
					}
					if (!pVLChild)
					{
						// if we haven't found this defined yet, use the built-in
						//	'length' properly 
						if (pVLObjectCall && pVLObjectCall->m_pVar && pVLObjectCall->m_pVar->IsArray() && sNameChild == "length")
						{
							nLenghtArray = pVLObjectCall->m_pVar->GetArrayLength();
							pVLChild = new CVarLink(new CVar(nLenghtArray));
							if (nLenghtArray >= 7000)
							{
								if (g_sstrDetectDownloader)
									g_sstrDetectDownloader->bFor7k = true;
							}


						}
						else if (pVLObjectCall && pVLObjectCall->m_pVar && pVLObjectCall->m_pVar->IsString() && sNameChild == "length") 
						{
							nLenghtArray = pVLObjectCall->m_pVar->GetString().size();
							pVLChild = new CVarLink(new CVar(nLenghtArray));

						}
						else 
						{
							if (pVLObjectCall && pVLObjectCall->m_pVar)
								pVLChild = pVLObjectCall->m_pVar->AddChild(sNameChild);

							SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, m_pTokenPointer->m_sTokenStr, NULL);
						}
					}

					// duyet den phan tu con
					pVParent = pVLObjectCall->m_pVar;
					pVLObjectCall = pVLChild;
				}
				m_pTokenPointer->Match(TK_ID);
			}

			// Array Access ----------------------------------------------------
			// typeToken = ID; 
			// m_tokenStr = ten cua 1 mang
			else if (m_pTokenPointer->m_nTokenId == '[')
			{
				m_pTokenPointer->Match('[');
				pVLIndexOfArray = Base(bExecute, bExecuteInEval);
				if (bExecute && (!pVLIndexOfArray || !pVLIndexOfArray->m_pVar || pVLIndexOfArray->m_pVar->IsUndefined()))
					throw new CRuntimeException("Loi:: Mang[ViTri] khong duoc dinh nghia");
				m_sCodeJS += " Array ";
				m_pTokenPointer->Match(']');
				if (bExecute && pVLObjectCall && pVLObjectCall->m_pVar)
				{
					sNameChild = pVLIndexOfArray->m_pVar->GetString();
					if (pVLIndexOfArray->m_pVar->IsInt())
					{
						bStringInt = true;
					    iString = pVLIndexOfArray->m_pVar->GetInt();
					}
					pVLChildInArray = pVLObjectCall->m_pVar->FindChild(sNameChild);
					if (!pVLChildInArray)
					{
						if (pVLObjectCall->m_pVar->IsObject()
							|| pVLObjectCall->m_pVar->IsString()
							|| pVLObjectCall->m_pVar->IsArray()
							|| pVLObjectCall->m_pVar->IsDouble())
						{
							// Tim kiem trong prototype cua Object
							pVLChildInPrototype = FindInParentClasses(pVLObjectCall->m_pVar, sNameChild);

							if (pVLChildInPrototype &&	pVLChildInPrototype->m_pVar)
							{
								if (pVLChildInPrototype->m_pVar->IsFunction())
									pVLChildInArray = pVLChildInPrototype;
								else
									pVLChildInArray = pVLObjectCall->m_pVar->AddChild(sNameChild);
							}
						}
					}
					if (!pVLChildInArray)

						//Tra ve gia tri do dai cua mang a[length]...[NDC];
					if (pVLObjectCall->m_pVar->IsArray() && sNameChild == "length")
					{
						nLenghtArray = pVLObjectCall->m_pVar->GetArrayLength();
						pVLChildInArray = new CVarLink(new CVar(nLenghtArray));
						if (nLenghtArray >= 7000)
						{
							if (g_sstrDetectDownloader)
								g_sstrDetectDownloader->bFor7k = true;
						}
					}
					else if (bStringInt && (pVLObjectCall->m_pVar->IsString()) && (iString >= 0) && (iString < (pVLObjectCall->m_pVar->GetString().size())))
					{
						sDataString = pVLObjectCall->m_pVar->m_sData;
						sString = sDataString.at(iString);
						pVTmpValueString = new CVar(sString, SCRIPTVAR_STRING);
						pVLArraytoString = new CVarLink(pVTmpValueString);
						pVLChildInArray = pVLArraytoString;
					}
					else if (pVLObjectCall->m_pVar->IsString() && sNameChild == "length") 
					{
						nLenghtArray = pVLObjectCall->m_pVar->GetString().size();
						pVLChildInArray = new CVarLink(new CVar(nLenghtArray));

					}
					else 
					{
						pVLChildInArray = pVLObjectCall->m_pVar->FindChildOrCreate(sNameChild);
					}
					pVParent = pVLObjectCall->m_pVar;
					pVLObjectCall = pVLChildInArray;
				}
			}
		}
		if (pVLObjectBeginCall && pVLObjectCall && pVLObjectBeginCall->m_bOwned && pVLObjectCall->m_bOwned)
			pVLReturn = pVLObjectCall;
		else
			if (pVLObjectCall)
				pVLReturn = new CVarLink(pVLObjectCall->m_pVar, pVLObjectCall->m_sName);
			else
				pVLReturn = new CVarLink(NULL, "");

	}
	catch (CRuntimeException* pRE)
	{
		pRunException = pRE;
	}

	SAFE_THROW(pRunException);
	return pVLReturn;
}

//------------------------------------------------------------------------------
// Toan tu uu tien: (a)
// Toan hang la hang so: true, false, undefine
// Tim toan hang trong stack: ..a.. ;	a = function | cau truc | mang 
// Kieu du lieu: Int, float, string 
// Khoi lenh thuc thi: {...}
// Mang du lieu [....] -> VAR_R_ARRAY
// Toan tu New: tra ve doi tuong: VAR_R_OBJECT
// Toan tu Function: dinh nghia doi tuong co constructor
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Factor(bool &bExecute, bool bExecuteInEval)
{
	int nTypeTk = 0;
	int nIndexInArray = 0;
	string sTokenPointerId;
	string sNameChildThis;
	CVarLink *pVLChildInPrototype = NULL;
	CVarLink *pVLChildInArray = NULL;
	CVarLink *pVLReturn = NULL;
	CVarLink *pVLIndexOfArray = NULL;
	CVarLink *pVLIndexOfArrayThis = NULL;
	CVarLink *pVLObjClassOrFunc = NULL;
	CVarLink *pVLFuncReturn = NULL;
	CVarLink *pVLNewObj = NULL;
	CVarLink *pVLConstructor = NULL;
	CVarLink* pVLValueReturn = NULL;
	CVarLink* pVLTmpValue = NULL;
	CVar*	pVTmpValue = NULL;
	CVar*	pVParent = NULL; // The parent if we're executing a method call 
	CRuntimeException* pREHandleProcess = NULL;
	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_UNDEFINED));

	try
	{
		if (m_pTokenPointer->m_nTokenId == '(')
		{
			m_pTokenPointer->Match('('); // [NDC]
			do
			{
				// neu khai bao co kieu var a=(b+c,e+d) thi tra ve a=e+d 
				if (m_pTokenPointer->m_nTokenId == ',')
					m_pTokenPointer->Match(',');
				pVLValueReturn = Base(bExecute, bExecuteInEval);
			} while (m_pTokenPointer->m_nTokenId == ',');

			m_pTokenPointer->Match(')');

			// Thuc hien ham vua duoc dinh nghia || Truy van phan tu con 
			pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, NULL, bExecuteInEval);

			return pVLValueReturn;
		}

		if (m_pTokenPointer->m_nTokenId == TK_R_TRUE)
		{
			m_pTokenPointer->Match(TK_R_TRUE);
			return new CVarLink(new CVar(1));
		}

		if (m_pTokenPointer->m_nTokenId == TK_R_FALSE)
		{
			m_pTokenPointer->Match(TK_R_FALSE);
			return new CVarLink(new CVar(0));
		}

		if (m_pTokenPointer->m_nTokenId == TK_R_NULL)
		{
			m_pTokenPointer->Match(TK_R_NULL);
			return new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_NULL));
		}

		if (m_pTokenPointer->m_nTokenId == TK_R_UNDEFINED)
		{
			m_pTokenPointer->Match(TK_R_UNDEFINED);
			return new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_UNDEFINED));
		}

		// type token = ID : bien trong stack-----------------------------------
		if (m_pTokenPointer->m_nTokenId == TK_ID)
		{
			// Tim bien trong stack theo kieu this[FunctionName](xyz) .... [NDC]
			if (m_pTokenPointer->m_sTokenStr == "this" && m_pTokenPointer->m_currCh == '[')
			{
				m_sCodeJS += " this ";
				m_pTokenPointer->Match(TK_ID);
				m_pTokenPointer->Match('[');
				m_sCodeJS += " [ ";
				pVLIndexOfArrayThis = Base(bExecute, bExecuteInEval);
				if (pVLIndexOfArrayThis && pVLIndexOfArrayThis->m_pVar)
					sNameChildThis = pVLIndexOfArrayThis->m_pVar->GetString();
				pVLValueReturn = bExecute ? FindInScopes(sNameChildThis): new CVarLink(new CVar());
				if (bExecute && !pVLValueReturn)
				{
					/* Variable doesn't exist! JavaScript says we should create it
					* (we won't add it here. This is done in the assignment operator)*/
					pVLValueReturn = new CVarLink(new CVar(),sNameChildThis);
					SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, sNameChildThis, NULL);
				}
				m_sCodeJS += " " + sNameChildThis + " ";
				m_pTokenPointer->Match(']');
				m_sCodeJS += " ] ";
				pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, pVParent, bExecuteInEval);
			}
			else
				// Tim bien trong stack theo TokenPointer::m_szTokenStr
			{
				pVLValueReturn = bExecute ? FindInScopes(m_pTokenPointer->m_sTokenStr)
					: new CVarLink(new CVar());
						if (bExecute && !pVLValueReturn)
				{
					/* Variable doesn't exist! JavaScript says we should create it
					* (we won't add it here. This is done in the assignment operator)*/
					pVLValueReturn = new CVarLink(new CVar(), m_pTokenPointer->m_sTokenStr);
					SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, m_pTokenPointer->m_sTokenStr, NULL);
				}
				
				m_pTokenPointer->Match(TK_ID);
				pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, pVParent, bExecuteInEval);
			}
			return pVLValueReturn;
		}

		// type token = kieu du lieu -------------------------------------------
		if (m_pTokenPointer->m_nTokenId == TK_INT || m_pTokenPointer->m_nTokenId == TK_FLOAT)
		{
			pVTmpValue = new CVar(m_pTokenPointer->m_sTokenStr,
				((m_pTokenPointer->m_nTokenId == TK_INT) ? SCRIPTVAR_INTEGER : SCRIPTVAR_DOUBLE));
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			return new CVarLink(pVTmpValue);
		}

		if (m_pTokenPointer->m_nTokenId == TK_STR)
		{
			pVTmpValue = new CVar(m_pTokenPointer->m_sTokenStr, SCRIPTVAR_STRING);
			pVLValueReturn = new CVarLink(pVTmpValue);
			m_pTokenPointer->Match(TK_STR);

			pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, NULL, bExecuteInEval);
			return pVLValueReturn;
		}
		// type token = regex
		if (m_pTokenPointer->m_nTokenId == TK_REGEX)
		{
			pVTmpValue = new CVar(m_pTokenPointer->m_sTokenStr, SCRIPTVAR_REGEX);
			pVLValueReturn = new CVarLink(pVTmpValue);
			m_pTokenPointer->Match(TK_REGEX);

			pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, NULL, bExecuteInEval);
			return pVLValueReturn;

		}
		// type token = bat dau khoi lenh --------------------------------------
		if (m_pTokenPointer->m_nTokenId == '{')
		{
			pVParent = new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT);

			/* JSON-style object definition */
			m_pTokenPointer->Match('{');
			while (m_pTokenPointer->m_nTokenId != '}')
			{
				sTokenPointerId = m_pTokenPointer->m_sTokenStr;
				// we only allow strings or IDs on the left hand side of an initialisation
				if (m_pTokenPointer->m_nTokenId == TK_REGEX)
					m_pTokenPointer->Match(TK_REGEX);
				else if (m_pTokenPointer->m_nTokenId == TK_STR)
					m_pTokenPointer->Match(TK_STR);
				else
					m_pTokenPointer->Match(TK_ID);
				m_pTokenPointer->Match(':');
				if (bExecute) 
				{
					pVLTmpValue = Base(bExecute, bExecuteInEval);
					pVParent->AddChild(sTokenPointerId, pVLTmpValue->m_pVar);
				}
				// no need to clean here, as it will definitely be used
				if (m_pTokenPointer->m_nTokenId != '}')
					m_pTokenPointer->Match(',');
			}
			m_pTokenPointer->Match('}');

			// Neu truy xuat den ngay phan tu con cua doi tuong
			pVLValueReturn = new CVarLink(pVParent);
			pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, NULL, bExecuteInEval);

			return pVLValueReturn;
		}

		// type token = bat dau noi dung mang ----------------------------------
		if (m_pTokenPointer->m_nTokenId == '[')
		{
			pVParent = new CVar(STR_BLANK_DATA, SCRIPTVAR_ARRAY);
			nIndexInArray = 0;
			/* JSON-style array */
			m_pTokenPointer->Match('[');
			m_sCodeJS += " Array ";

			while (m_pTokenPointer->m_nTokenId != ']')
			{
				if (bExecute) 
				{
					pVLTmpValue = Base(bExecute, bExecuteInEval);
					if (pVLTmpValue)
					{
						CVarLink* pVar = pVParent->AddChild("", pVLTmpValue->m_pVar);
						if (pVar)
							pVar->SetIntName(nIndexInArray);
					}
				}
				// no need to clean here, as it will definitely be used
				if (m_pTokenPointer->m_nTokenId != ']')
					m_pTokenPointer->Match(',');
				nIndexInArray++;
			}

			m_pTokenPointer->Match(']');
			pVLValueReturn = new CVarLink(pVParent);
			pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, pVParent, bExecuteInEval);
			return pVLValueReturn;
		}

		//  dinh nghia ham moi -------------------------------------------------
		if (m_pTokenPointer->m_nTokenId == TK_R_FUNCTION)
		{
			pVLTmpValue = ParseFunctionDefinition();
			if (pVLTmpValue == NULL)
				return pVLTmpValue;
			//chay kieu ham function a() {...}()  ...[NDC]
			if ((m_pTokenPointer->m_nTokenId == '(') /*&& (m_pTokenPointer->m_currCh == ')')*/)
			{
				pVLValueReturn = bExecute ? FindInScopes(pVLTmpValue->m_sName): new CVarLink(new CVar());
				if (pVLValueReturn != NULL)
					pVLValueReturn = FactorAdvance(bExecute, pVLValueReturn, pVParent, bExecuteInEval);
				else
					pVLValueReturn = FactorAdvance(bExecute, pVLTmpValue, pVParent, bExecuteInEval); 
				// Chuan hoa va luu ten ham vao SCodeJS
				if (pVLValueReturn != NULL)
					if (pVLValueReturn->m_sAliasName == "")
						m_sCodeJS += " " + pVLValueReturn->m_sName + " ";
					else
						m_sCodeJS += " " + pVLValueReturn->m_sAliasName + " ";

				return pVLValueReturn;
			}
			if (pVLTmpValue->m_sName != STR_TEMP_NAME)
				OutputDebug("Functions not defined at statement-level are not meant to have a name");
			return pVLTmpValue;
		}

		// xin cap phat doi tuong ----------------------------------------------
		if (m_pTokenPointer->m_nTokenId == TK_R_NEW)
		{
			pVLReturn = new CVarLink(new CVar());

			// new -> create a new object
			m_pTokenPointer->Match(TK_R_NEW);
			m_sCodeJS += " new "; // Luu cau lnh xin cap phat doi tuong
			sTokenPointerId = m_pTokenPointer->m_sTokenStr;
			if (bExecute)
			{
				pVLObjClassOrFunc = FindInScopes(sTokenPointerId);
				if (!pVLObjClassOrFunc)
					throw new CRuntimeException(sTokenPointerId + " is not a valid class name");
				m_sCodeJS += " " + pVLObjClassOrFunc->m_sName + " "; // Them ten doi tuong khoi tao khi goi new
				m_pTokenPointer->Match(TK_ID);
				pVLNewObj = new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT));
				if (pVLObjClassOrFunc && pVLObjClassOrFunc->m_pVar && pVLObjClassOrFunc->m_pVar->IsFunction() && pVLNewObj && pVLNewObj->m_pVar)
				{
					// This Function = constructor's Object
					
					pVLFuncReturn = FunctionCall(bExecute, pVLObjClassOrFunc, pVLNewObj->m_pVar, bExecuteInEval);
					pVLNewObj->ReplaceWith(pVLFuncReturn);
				}
				else if (pVLObjClassOrFunc && pVLObjClassOrFunc->m_pVar)
				{
					// co ham khoi tao constructor rieng
					// xx check prototype function ??? 
					pVLConstructor = pVLObjClassOrFunc->m_pVar->FindChild("constructor");
					nTypeTk = this->m_pTokenPointer->TryWatchNextToken();
					if (nTypeTk != ')' && pVLConstructor != NULL)
					{
						pVLFuncReturn = FunctionCall(bExecute, pVLConstructor, pVLNewObj->m_pVar, bExecuteInEval);
						pVLNewObj->ReplaceWith(pVLFuncReturn);
					}
					else
					{
						// la doi tuong chi co con structor mac dinh
						if (pVLNewObj->m_pVar)
							pVLNewObj->m_pVar->AddChild(STR_PROTOTYPE_CLASS, pVLObjClassOrFunc->m_pVar);
						if (m_pTokenPointer->m_nTokenId == '(')
						{
							m_pTokenPointer->Match('(');
							m_pTokenPointer->Match(')');
						}
					}
				}
				pVLReturn->ReplaceAllNameWith(pVLNewObj);
				goto _COMPLETE;
			}
			else 
			{

				m_pTokenPointer->Match(TK_ID);
				if (m_pTokenPointer->m_nTokenId == '(')
				{
					m_pTokenPointer->Match('(');
					m_pTokenPointer->Match(')');
				}
			}
		}

		// Bo sung: Khi chay trong lenh Eval(); 
		if (bExecuteInEval)
		{
			if (!Statement(bExecute, bExecuteInEval))
				throw new CRuntimeException("Thuc hien lenh trong 'Eval(jsCode)' loi!!");
		}
		else
			// Nothing we can do here... just hope it's the end...
			m_pTokenPointer->Match(TK_EOF);

		return 0;
	}
	catch (CRuntimeException* pRE)
	{
		pREHandleProcess = pRE;
	}

_COMPLETE:

	// Chuyen tiep exception
	SAFE_THROW(pREHandleProcess);
	return pVLReturn;
}


//------------------------------------------------------------------------------
// Toan tu NOT: ~ , a = !x, typeof(x), ++x, ...;	
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Unary(bool &bExecute, bool bExecuteInEval) {
	int nErrId = 0;
	int nTPId = 0;

	CVarLink *pVLZero = NULL;
	CVarLink *pVLOne = NULL;
	CVarLink *pVLRes = NULL;

	CVarLink *pVLOperandLeft = NULL;
	CRuntimeException* pException = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());

	if (m_pTokenPointer->m_nTokenId == '!' ||
		m_pTokenPointer->m_nTokenId == '~' ||
		m_pTokenPointer->m_nTokenId == TK_PLUSPLUS ||
		m_pTokenPointer->m_nTokenId == TK_MINUSMINUS)
	{
		try
		{
			pVLRes = new CVarLink(new CVar());
			pVLOne = new CVarLink(new CVar(1));
			pVLZero = new CVarLink(new CVar(0));
			nTPId = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			pVLOperandLeft = Factor(bExecute, bExecuteInEval);
			if (bExecute && pVLOperandLeft && pVLOperandLeft->m_pVar)
			{

				if (nTPId == '!')
				{
					// run !x
					// so sanh voi so 0; TRUE neu == 0
					pVLRes->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLZero->m_pVar, TK_EQUAL));
					CREATE_LINK(pVLOperandLeft, pVLRes->m_pVar);
				}
				else if (nTPId == '~')
				{
					pVLRes->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLZero->m_pVar, '~'));
					CREATE_LINK(pVLOperandLeft, pVLRes->m_pVar);
				}
				else
				{
					// thuc hien ++x, hoac --x
					switch (nTPId)
					{
					case TK_PLUSPLUS:
						pVLRes->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLOne->m_pVar, '+'));
						break;
					case TK_MINUSMINUS:
						pVLRes->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLOne->m_pVar, '-'));
						break;
					default:
						break;
					}
					pVLOperandLeft->ReplaceWith(pVLRes->m_pVar);
				}

			}
		}
		catch (CRuntimeException* pRE)
		{
			pException = pRE;
		}
	}
	else if (m_pTokenPointer->m_nTokenId == TK_R_TYPEOF) 
	{

		m_pTokenPointer->Match(TK_R_TYPEOF); // binary not
		if (bExecute)
		{
			try
			{
				pVLRes = new CVarLink(new CVar());

				// Lay con tro lien ket toi toan hang sau typeof
				pVLOperandLeft = Factor(bExecute, bExecuteInEval);	// exception 
				if (pVLOperandLeft && pVLOperandLeft->m_pVar)
				{
			
					if (pVLOperandLeft->m_pVar->IsUndefined() && m_bHadException)
						CleanMsgRunInTryCatch();

					pVLRes->ReplaceWith(new CVar(pVLOperandLeft->m_pVar->GetFlagsAsString()));
					CREATE_LINK(pVLOperandLeft, pVLRes->m_pVar);
				}
			}
			catch (CRuntimeException* pRE)
			{
				nErrId = pRE->GetErrId();

				if (nErrId == EXCEPTION_ID_NOTFOUND_IN_TRYCATCH)
				{
					// Neu toan hang chua ton tai trong tack=>tra ve "undefined"
					pVLRes->ReplaceWith(new CVar("undefined"));
					CleanMsgRunInTryCatch();
					SAFE_DELETE(pRE);
					CREATE_LINK(pVLOperandLeft, pVLRes->m_pVar);
				}
				else 
				{
					pException = pRE;
				}
			}
		}
	}
	else
		pVLOperandLeft = Factor(bExecute, bExecuteInEval);

	SAFE_THROW(pException);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// Xu ly toan tu: *, /, %
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Term(bool &bExecute, bool bExecuteInEval)
{
	int nOp = 0;
	CVar *pVRes = NULL;
	CVarLink* pVLTmpValue = NULL;
	CVarLink* pVLTmpValue2 = NULL;
	CVarLink* pVLOperandRight = NULL;
	CVarLink* pVLOperandLeft = Unary(bExecute, bExecuteInEval);
	CRuntimeException* pREHandleProcess = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());

	try
	{
		pVLTmpValue = new CVarLink(NULL);
		while (m_pTokenPointer->m_nTokenId == '*'
			|| m_pTokenPointer->m_nTokenId == '/'
			|| m_pTokenPointer->m_nTokenId == '%')
		{
			nOp = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			pVLOperandRight = Unary(bExecute);

			if (bExecute && pVLOperandLeft && pVLOperandLeft->m_pVar && pVLOperandRight && pVLTmpValue)
			{
				pVLTmpValue->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLOperandRight->m_pVar, nOp));
				OutputDebug("excute ReplaceWith");
				CREATE_LINK(pVLOperandLeft, pVLTmpValue->m_pVar);
			}
		}
	}

	// Xu ly exception 
	catch (CRuntimeException* pRE)
	{
		pREHandleProcess = pRE;
	}

	SAFE_THROW(pREHandleProcess);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// Xu ly toan tu: - (lay am), 
// toan tu: term
// toan tu: +, -, ++, --
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Expression(bool &bExecute, bool bExecuteInEval)
{
	bool bNegate = false;
	int nOp = 0;
	CVar* pVTmpValueStringToInt = NULL;
	CVarLink* pVLOne = NULL;
	CVarLink* pVLZero = NULL;
	CVarLink* pVLRes = NULL;
	CVarLink* pVLTmpValue = NULL;
	CVarLink* pVLOperandLeft = NULL;
	CVarLink* pVLOperandRight = NULL;
	CVarLink* pVLOldValue = NULL;
	CVarLink* pVLValueReturnStringToInt = NULL;
	CRuntimeException *pRuntimeException = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());

	// fix:: them trycatch giai phong vung nho khi xay ra exception
	try
	{
		pVLZero = new CVarLink(new CVar(0));
		pVLOne = new CVarLink(new CVar(1));
		pVLRes = new CVarLink(new CVar());

		// fix:: them xu ly dau '+/-' cho kieu du lieu 
		if (m_pTokenPointer->m_nTokenId == '-' || m_pTokenPointer->m_nTokenId == '+')
		{
			if (m_pTokenPointer->m_nTokenId == '-')
				bNegate = true;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

		}

		pVLOperandLeft = Term(bExecute, bExecuteInEval);
		if (bNegate)
		{
			if (pVLZero->m_pVar && pVLOperandLeft)
				pVLRes->ReplaceWith(pVLZero->m_pVar->MathsOp(pVLOperandLeft->m_pVar, '-'));
			CREATE_LINK(pVLOperandLeft, pVLRes->m_pVar);
		}

		while (m_pTokenPointer->m_nTokenId == '+'
			|| m_pTokenPointer->m_nTokenId == '-'
			|| m_pTokenPointer->m_nTokenId == TK_PLUSPLUS
			|| m_pTokenPointer->m_nTokenId == TK_MINUSMINUS)
		{
			nOp = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

			if (nOp == TK_PLUSPLUS || nOp == TK_MINUSMINUS) {
				if (bExecute && pVLOperandLeft && pVLOperandLeft->m_pVar)
				{
					pVLRes->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLOne->m_pVar, nOp == TK_PLUSPLUS ? '+' : '-'));	//exception
					pVLOldValue = new CVarLink(pVLOperandLeft->m_pVar);	// luu lai gia tri cu
					
					// in-place add/subtract
					pVLOperandLeft->ReplaceWith(pVLRes->m_pVar);

					pVLOperandLeft = new CVarLink(pVLOldValue->m_pVar);	// Khoi phuc gia tri cu
					//
					pVLTmpValue = new CVarLink(new CVar());
					while (m_pTokenPointer->m_nTokenId == '*'
						|| m_pTokenPointer->m_nTokenId == '/'
						|| m_pTokenPointer->m_nTokenId == '%')
					{
						nOp = m_pTokenPointer->m_nTokenId;
						m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
						pVLOperandRight = Unary(bExecute);

						if (bExecute && pVLOperandRight)
						{
							pVLTmpValue->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLOperandRight->m_pVar, nOp));
							CREATE_LINK(pVLOperandLeft, pVLTmpValue->m_pVar);
						}
					}

				}
			}

			else
			{
				pVLOperandRight = Term(bExecute);
				if (bExecute && pVLOperandRight && pVLOperandLeft && pVLOperandLeft->m_pVar)
				{
					// not in-place, so just replace
 						pVLRes->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLOperandRight->m_pVar, nOp)); //exception
						CREATE_LINK(pVLOperandLeft, pVLRes->m_pVar);
				}
			}
		}
	}
	catch (CRuntimeException *pRE){
		pRuntimeException = pRE;
	}

	SAFE_THROW(pRuntimeException);
	return pVLOperandLeft;
}


//------------------------------------------------------------------------------
// toan tu: expression
// Xu ly toan tu dich: <<, >>, >>>
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Shift(bool &bExecute, bool bExecuteInEval)
{
	int nOp = 0;
	int nShiftVal = 0;
	int iValue = 0;

	CVarLink *pVLTmpValue = NULL;
	CVarLink *pVLOperandRight = NULL;
	CVarLink *pVLOperandLeft = Expression(bExecute, bExecuteInEval);
	CRuntimeException* pREHandleProcess = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());
	try
	{
		if (m_pTokenPointer->m_nTokenId == TK_LSHIFT ||
			m_pTokenPointer->m_nTokenId == TK_RSHIFT ||
			m_pTokenPointer->m_nTokenId == TK_RSHIFTUNSIGNED)
		{
			nOp = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(nOp);
			
			pVLOperandRight = Expression(bExecute, bExecuteInEval);
			if (pVLOperandRight && pVLOperandRight->m_pVar)
				iValue = pVLOperandRight->m_pVar->GetInt();
			nShiftVal = bExecute ? iValue : 0;

			if (bExecute)
			{
				// fix:: a->m_pVar->setInt(...)
				pVLTmpValue = new CVarLink(new CVar());

				// thuc hien << || >> || >>> 
				if (pVLTmpValue->m_pVar && pVLOperandLeft && pVLOperandLeft->m_pVar)
				{
					if (nOp == TK_LSHIFT )
						pVLTmpValue->m_pVar->SetInt(pVLOperandLeft->m_pVar->GetInt() << nShiftVal);
					if (nOp == TK_RSHIFT)
						pVLTmpValue->m_pVar->SetInt(pVLOperandLeft->m_pVar->GetInt() >> nShiftVal);
					if (nOp == TK_RSHIFTUNSIGNED)
						pVLTmpValue->m_pVar->SetInt(((unsigned int)pVLOperandLeft->m_pVar->GetInt()) >> nShiftVal);
				}

				// tao lien ket tra ve gia tri cho loi goi ham
				CREATE_LINK(pVLOperandLeft, pVLTmpValue->m_pVar);
			}
		}
	}
	catch (CRuntimeException* pRE)
	{
		pREHandleProcess = pRE;
	}

	SAFE_THROW(pREHandleProcess);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// toan tu: Condition
// Xu ly toan dieu kien: ==, !=, ===, !==, <=, >=, <, >
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Condition(bool &bExecute, bool bExecuteInEval)
{
	int nOp = 0;
	CVar *pVRes = NULL;
	CVarLink *pVLOperandLeft = Shift(bExecute, bExecuteInEval);
	CVarLink *pVLOperandRight = NULL;
	CRuntimeException* pREHandleProcess = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());

	try
	{
		while (m_pTokenPointer->m_nTokenId == TK_EQUAL
			|| m_pTokenPointer->m_nTokenId == TK_NEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_TYPEEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_NTYPEEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_LEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_GEQUAL
			|| m_pTokenPointer->m_nTokenId == '<'
			|| m_pTokenPointer->m_nTokenId == '>')
		{
			nOp = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			pVLOperandRight = Shift(bExecute);
			if (bExecute && pVLOperandRight && pVLOperandLeft && pVLOperandLeft->m_pVar) 
			{
				pVRes = pVLOperandLeft->m_pVar->MathsOp(pVLOperandRight->m_pVar, nOp);
				CREATE_LINK(pVLOperandLeft, pVRes);
			}
		}
	}
	catch (CRuntimeException* pRE)
	{
		pREHandleProcess = pRE;
	}
	SAFE_THROW(pREHandleProcess);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// toan tu: Logic - phep toan tren bit
// Xu ly toan dieu kien: &, |, ^, &&, ||
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Logic(bool &bExecute, bool bExecuteInEval)
{
	bool bNoexecute = false;
	bool bShortCircuit = false;
	bool bBoolean = false;
	int nOp = 0;

	CVarLink *pVLRes = NULL;
	CVarLink *pVLNewB = NULL;
	CVarLink *pVLNewA = NULL;
	CVarLink *pVLOperandRight = NULL;
	CVarLink *pVLOperandLeft = NULL;
	CRuntimeException* pREHandleProcess = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());

	pVLOperandLeft = Condition(bExecute, bExecuteInEval);
	if (pVLOperandLeft == NULL || pVLOperandLeft->m_pVar == NULL)
		return new CVarLink(new CVar());
	try
	{
		while (m_pTokenPointer->m_nTokenId == '&'
			|| m_pTokenPointer->m_nTokenId == '|'
			|| m_pTokenPointer->m_nTokenId == '^'
			|| m_pTokenPointer->m_nTokenId == TK_ANDAND
			|| m_pTokenPointer->m_nTokenId == TK_OROR)
		{
			nOp = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

			bNoexecute = false;
			bShortCircuit = false;
			bBoolean = false;

			// if we have short-circuit ops, then if we know the outcome
			// we don't bother to execute the other op. Even if not
			// we need to tell mathsOp it's an & or |
			if (nOp == TK_ANDAND)
			{
				nOp = '&';
//				bShortCircuit = !pVLOperandLeft->m_pVar->GetBool();
				bBoolean = true;
			}
			else if (nOp == TK_OROR)
			{
				nOp = '|';
//				bShortCircuit = pVLOperandLeft->m_pVar->GetBool();
				bBoolean = true;
			}
		
			pVLOperandRight = Condition(bShortCircuit ? bNoexecute : bExecute);
//			pVLOperandRight = Condition(bShortCircuit ? bExecute : bNoexecute);
			if (pVLOperandRight == NULL || pVLOperandLeft->m_pVar == NULL)
				return new CVarLink(new CVar());
			if (bExecute && !bShortCircuit && pVLOperandRight) 
			{
				pVLRes = new CVarLink(new CVar());
				if (bBoolean)
				{
					pVLNewA = new CVarLink(new CVar(pVLOperandLeft->m_pVar->GetBool()));
					pVLNewB = new CVarLink(new CVar(pVLOperandRight->m_pVar->GetBool()));
					CREATE_LINK(pVLOperandLeft, pVLNewA->m_pVar);
					CREATE_LINK(pVLOperandRight, pVLNewB->m_pVar);
				}

				pVLRes->ReplaceWith(pVLOperandLeft->m_pVar->MathsOp(pVLOperandRight->m_pVar, nOp));
				CREATE_LINK(pVLOperandLeft, pVLRes->m_pVar);
				// Ket qua khi thuc hine phep tinh duoc gan vao pVLOperandLeft
			}
		}
	}
	catch (CRuntimeException* pRE)
	{
		pREHandleProcess = pRE;
	}

	SAFE_THROW(pREHandleProcess);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// toan tu: Ternary - phep toan (dk)? TRUE : FALSE
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Ternary(bool &bExecute, bool bExecuteInEval)
{
	bool bNoexec = false;
	bool bFirst;
	CVarLink *pVLTmpResult = NULL;
	CVarLink *pVLCondition = Logic(bExecute, bExecuteInEval);
	CRuntimeException* pREError = NULL;

	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());

	try
	{
		if (m_pTokenPointer->m_nTokenId == '?')
		{
			m_pTokenPointer->Match('?');
			if (!bExecute)
			{
				pVLTmpResult = Base(bNoexec, bExecuteInEval);
				m_pTokenPointer->Match(':');

				pVLTmpResult = Base(bNoexec, bExecuteInEval);
			}
			else
			{
				if (pVLCondition && pVLCondition->m_pVar)
				{
					bFirst = pVLCondition->m_pVar->GetBool();
					if (bFirst)
					{
						pVLCondition = Base(bExecute, bExecuteInEval);
						m_pTokenPointer->Match(':');

						pVLTmpResult = Base(bNoexec, bExecuteInEval);
					}
					else
					{
						pVLTmpResult = Base(bNoexec, bExecuteInEval);

						m_pTokenPointer->Match(':');
						pVLCondition = Base(bExecute, bExecuteInEval);
					}
				}
			}
		}
	}

	catch (CRuntimeException* pRE)
	{
		pREError = pRE;
	}

	SAFE_THROW(pREError);
	return pVLCondition;
}

//------------------------------------------------------------------------------
// toan tu: Base - phep toan gan
// toan tu co so: =, +=, -= |= ^= &= <<= >>=
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Base(bool &bExecute, bool bExecuteInEval)
{
	int nShiftVal = 0;
	int nOp = 0;
	string sLinkCheckRedirect;
	CVar *pVRes = NULL;
	CVar* pVArgsValue = NULL;
	CVarLink* pVLTmp = NULL;
	CVarLink *pVLRealLhs = NULL;
	CVarLink *pVLRightOperand = NULL;
	CVarLink *pVLLeftOperand = NULL;
	CRuntimeException* pException = NULL;
	CRuntimeException* pREHadVirus = NULL;

	if (m_pTokenPointer == NULL)
		return NULL;
	try
	{

		if (GetCurrentStackSize() > STACK_SIZE) 
			throw std::out_of_range("Tran Stack");

		pVLLeftOperand = Ternary(bExecute, bExecuteInEval);

		if (m_pTokenPointer->m_nTokenId == '='
			|| m_pTokenPointer->m_nTokenId == TK_PLUSEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_MINUSEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_OREQUAL
			|| m_pTokenPointer->m_nTokenId == TK_XOREQUAL
			|| m_pTokenPointer->m_nTokenId == TK_ANDEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_LSHIFTEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_RSHIFTEQUAL)
		{
			/* If we're assigning to this and we don't have a parent,
			* add it to the symbol table m_pRootStack as per JavaScript. */

			// fix:: xoa bo msgWhenRunInTryCatch neu co 
			CleanMsgRunInTryCatch();
			if (bExecute && pVLLeftOperand && !pVLLeftOperand->m_bOwned)
			{
				if (pVLLeftOperand->m_sName.length() > 0)
				{
					pVLRealLhs = m_pVRootStack->AddChildNoDup(pVLLeftOperand->m_sName, pVLLeftOperand->m_pVar);
					pVLLeftOperand = pVLRealLhs;
				}
				else
					OutputDebug("CProgramJs::base()!! Trying to assign to an un-named type\n");
			}
			// Lay toan tu 
			nOp = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

			// Lay toan hang ve phaig_sstrDetectDownloader->bFor7k
			pVLRightOperand = Base(bExecute, bExecuteInEval);

			if (bExecute && pVLRightOperand && pVLRightOperand->m_pVar && pVLLeftOperand && pVLLeftOperand->m_pVar)
			{
				// Xu ly phep toan
				if (nOp == '=')
				{
					// Neu bien window.onload = dia chi ham -> thuc hien ham
					if (pVLLeftOperand == m_pVLWindowOnload || pVLLeftOperand == m_pVLDStringOnreadyStateChange)
					{
						if (pVLRightOperand && pVLRightOperand->m_pVar
							&& pVLRightOperand->m_pVar->IsFunction())
						{
							// thuc hien ham tu dinh nghia luu trong toan hang ben phai
							if (!pVLRightOperand->m_pVar->IsNative())
							{
								if (pVLLeftOperand==m_pVLDStringOnreadyStateChange) // ...[NDC]
                                    m_onreadystatechange = true;
								// Lại check giá trị của vế phải xem có giống với đoạn code mẫu hay không
								//this->CheckVirusStatic(pVLRightOperand->m_pVar->m_sData);
								// Gọi lại hàm Execute để đệ qui
								this->Execute(pVLRightOperand->m_pVar->m_sData);
								m_onreadystatechange = false;
							}
						}
					}
					//bat virus voi dieu kien vong for >7k va co thay doi gia tri trong for..... [NDC]
					else if (pVLRightOperand->m_pVar->IsString() && g_sstrDetectDownloader && g_sstrDetectDownloader->bFor7k)
					{
						/*pREHadVirus = new CRuntimeException(VR_NAME_DOWNLOADER, EXCEPTIONID_FOUND_VIRUS, false);
						pREHadVirus->SetVirusName(VR_NAME_DOWNLOADER);
						throw pREHadVirus;*/
					}

					// Neu bien document.onmosemove = dia chi ham -> thuc hien ham
					else if (pVLLeftOperand == m_pVLDcmtOnmousemove)
					{
						if (pVLRightOperand && pVLRightOperand->m_pVar
							&& pVLRightOperand->m_pVar->IsFunction())
						{
							// thuc hien ham tu dinh nghia luu trong toan hang ben phai
							if (!pVLRightOperand->m_pVar->IsNative())
							{

								pVArgsValue = new CVar();
								pVRes = new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT);
								pVRes->AddChild("pageX", new CVar(POS_X_MOUSE));
								pVRes->AddChild("pageY", new CVar(POS_Y_MOUSE));
								pVArgsValue->AddChild("e", pVRes);

								pVLTmp = RunFunction(pVLRightOperand, NULL, pVArgsValue);
								pVRes = NULL;

								// thuc hien k.tra vi tri cac t`.phan document
								/*if (CheckVirusFollowMouse())
								{
									pREHadVirus = new CRuntimeException(VR_NAME_FACE_LIKER, EXCEPTIONID_FOUND_VIRUS, false);
									pREHadVirus->SetVirusName(VR_NAME_FACE_LIKER);
									throw pREHadVirus;
								}*/

							}
						}
					}

					else 
					{
						// Kiem tra thuoc tinh va gia tri duoc gan du lieu
						// Neu window.location.href = link -> virus
						if (pVLRightOperand->m_pVar->IsString())
						{
							/*sLinkCheckRedirect = pVLRightOperand->m_pVar->GetString();
							if (CheckVirusRedirect(pVLLeftOperand, sLinkCheckRedirect))
							{
								pREHadVirus = new CRuntimeException(VR_NAME_REDIRECT, EXCEPTIONID_FOUND_VIRUS, false);
								pREHadVirus->SetVirusName(VR_NAME_REDIRECT);
								throw pREHadVirus;
							}*/
						}
						//document.cookie duoc dinh nghia mot gia tri khac
						if (pVLLeftOperand->m_sName == "cookie")
						{
							pVLLeftOperand->m_pVar->m_sData = pVLRightOperand->m_pVar->m_sData;
						}
						else
							pVLLeftOperand->ReplaceWith(pVLRightOperand);//thuc hien gan thong thuong
					}
				}
				else if (nOp == TK_PLUSEQUAL)
				{
					pVRes = pVLLeftOperand->m_pVar->MathsOp(pVLRightOperand->m_pVar, '+');
					//bat virus voi dieu kien vong for >7k va co thay doi gia tri trong for
					/*if (pVLRightOperand->m_pVar->IsString() && g_sstrDetectDownloader && g_sstrDetectDownloader->bFor7k)
					{
						pREHadVirus = new CRuntimeException(VR_NAME_DOWNLOADER, EXCEPTIONID_FOUND_VIRUS, false);
						pREHadVirus->SetVirusName(VR_NAME_DOWNLOADER);
						throw pREHadVirus;
					}*/

					pVLLeftOperand->ReplaceWith(pVRes);

				}
				else if (nOp == TK_MINUSEQUAL)
				{
					pVRes = pVLLeftOperand->m_pVar->MathsOp(pVLRightOperand->m_pVar, '-');
					pVLLeftOperand->ReplaceWith(pVRes);
				}
				else if (nOp == TK_OREQUAL)
				{
					pVRes = pVLLeftOperand->m_pVar->MathsOp(pVLRightOperand->m_pVar, '|');
					pVLLeftOperand->ReplaceWith(pVRes);
				}
				else if (nOp == TK_XOREQUAL)
				{
					pVRes = pVLLeftOperand->m_pVar->MathsOp(pVLRightOperand->m_pVar, '^');
					pVLLeftOperand->ReplaceWith(pVRes);
				}
				else if (nOp == TK_ANDEQUAL)
				{
					pVRes = pVLLeftOperand->m_pVar->MathsOp(pVLRightOperand->m_pVar, '&');
					pVLLeftOperand->ReplaceWith(pVRes);
				}
				else if (nOp == TK_LSHIFTEQUAL)
				{
					// fix:: lay gia tri ve phai
					nShiftVal = bExecute ? pVLRightOperand->m_pVar->GetInt() : 0;

					// Tao bien luu gia tri ket qua tam thoi
					pVRes = new CVar();

					// thuc hien toan tu dich trai "<<"  
					pVRes->SetInt(pVLLeftOperand->m_pVar->GetInt() << nShiftVal);
					pVLLeftOperand->ReplaceWith(pVRes);

				}
				else if (nOp == TK_RSHIFTEQUAL)
				{
					// fix:: lay gia tri ve phai
					nShiftVal = bExecute ? pVLRightOperand->m_pVar->GetInt() : 0;

					// Tao bien luu gia tri ket qua tam thoi
					pVRes = new CVar();

					// thuc hien toan tu dich trai ">>"  
					pVRes->SetInt(pVLLeftOperand->m_pVar->GetInt() >> nShiftVal);
					pVLLeftOperand->ReplaceWith(pVRes);

				}
				else 
				{
					// toan tu moi chua cai dat
					throw CRuntimeException("Toan tu \"" + m_pTokenPointer->GetTokenStr(nOp)
						+ "\" chua duoc cai dat!");
				}
			}
		}

		// Kiem tra exception trong code js
		if (IsRunInTryCatch() && m_bHadException)
		{
			throw new CRuntimeException("Process Exception In JsCode!!", EXCEPTION_ID_NOTFOUND_IN_TRYCATCH);
		}

	}

	// xu ly try-catch cho truong hop code js tac dong vao bien mac dinh cua he thong
	// vd: window = "hihi";
	catch (CRuntimeException *pRE)
	{
		pException = pRE;
	}
	catch (out_of_range&)
	{
		pException = new CRuntimeException("_LOI Crash: Tran bo nho()!");
	}
	m_onreadystatechange = false;
	SAFE_THROW(pException);

	// xuat gia tri tra ve -> dam bao luon tra ve gia tri # 0
	if (pVLLeftOperand)
		return pVLLeftOperand;
	else
		return new CVarLink(new CVar());
}

// -----------------------------------------------------------------------------
// Xu ly khoi lenh 
// -----------------------------------------------------------------------------
bool CProgramJs::BlockCode(bool &bExecute, bool bExecuteInEval)
{
	bool bResultExeBlock = true;
	int nBracketsLevel = 1;

	if (m_pTokenPointer == NULL)
		return false;

	m_pTokenPointer->Match('{');
	if (bExecute)
	{
		// Thuc hien khoi lenh trong ngoac { ... }
		// Ket qua chay khoi lenh = false neu co 1 lenh chay sai
		while (m_pTokenPointer->m_nTokenId && m_pTokenPointer->m_nTokenId != '}')
			SAVE_BOOL_RETURN(bResultExeBlock, Statement(bExecute, bExecuteInEval));
		// End block code
		
		m_pTokenPointer->Match('}'); // Hàm này tìm kiếm token tiếp theo sau khi thực thi hết code trong khối lệnh (của một hàm tự định nghĩa)
	}
	else
	{
		// fast skip of blocksCode - Save szCode to Data && Not Run
		// Bo qua khoi lenh khong thuc hien { ... }
		nBracketsLevel = 1;
		while (m_pTokenPointer->m_nTokenId && nBracketsLevel)
		{
			if (m_pTokenPointer->m_nTokenId == '{')
				nBracketsLevel++;

			if (m_pTokenPointer->m_nTokenId == '}')
				nBracketsLevel--;

			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
		}
	}
	return bResultExeBlock;
}

// -----------------------------------------------------------------------------
// Xu ly tung doan lenh
// -----------------------------------------------------------------------------
bool CProgramJs::Statement(bool &bExecute, bool bExecuteInEval)
{
	CRuntimeException *pExceptionVr = NULL;

	CTokenPointer* pTokenParent = m_pTokenPointer;
	CTokenPointer* pTokenParent2 = NULL;
	CTokenPointer *pTokenCond = NULL;	
	CTokenPointer *pTokenIter = NULL;	
	CTokenPointer *pTokenBody = NULL;	
	CTokenPointer *pTokenForIn = NULL;
	CTokenPointer *pTokenOnlyBreak = NULL;
	char buffer[50];
	int nErrId = 0;
	int nWhileCondStart;
	int nWhileBodyStart;
	int nGetTickCountStart;
	int nGetTickCountEnd;
	int nTimeLoop;

	int nDoWhileCondStart;
	int nDoWhileBodyStart;

	int nForCondStart;
	int nForIterStart;
	int nForBodyStart;
	int nLoopCount = JS_LOOP_MAX_ITERATIONS; // So lan lap toi da
	int nArrayForInStart;
	int nForCondStartForIn;

	bool bResultRunStatement = true;
	bool bCondIf;
	bool bCondElse = false;
	bool bNoexecute = false;
	bool bLoopCond = false;
	bool bForSpecial = false;
	bool bExeBodyCode = true;
	bool bExeInterator = true;
	bool bForIn = false;
	bool bBreakFor = false;
	string sMsgMsgRunInTryCatch;
	string sNameVarCatch;
	
	CVar *pVTmpValue = NULL;

	CVarLink *pVLCond = NULL;
	CVarLink *pVLTmpValue = NULL;
	CVarLink *pVLValueInit = NULL;
	CVarLink *pVLAfterDots = NULL;
	CVarLink *pVLReturn = NULL;
	CVarLink* pVLSaveMsgException = NULL;
	CVarLink* pVLMsgException = NULL;
	CVarLink* pVLValueForInArray = NULL;
	CVarLink* pVLValueForIn = NULL;
	CVarLink* pVTmpValueForIn = NULL;
	CRuntimeException * pREContinue = NULL;

	if (m_pTokenPointer == NULL)
		return false;
	//TRY >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	try
	{

		//----------------------------------------------------------------------
		/* Execute a simple statement that only contains basic arithmetic... */
		if (m_pTokenPointer->m_nTokenId == TK_ID
			|| m_pTokenPointer->m_nTokenId == TK_INT
			|| m_pTokenPointer->m_nTokenId == TK_FLOAT
			|| m_pTokenPointer->m_nTokenId == TK_STR
			|| m_pTokenPointer->m_nTokenId == TK_R_NEW
			|| m_pTokenPointer->m_nTokenId == '-'
			|| m_pTokenPointer->m_nTokenId == '('
			|| m_pTokenPointer->m_nTokenId == TK_PLUSPLUS
			|| m_pTokenPointer->m_nTokenId == TK_MINUSMINUS
			|| m_pTokenPointer->m_nTokenId == TK_R_NULL)
		{
			pVLTmpValue = Base(bExecute, bExecuteInEval);
			m_pTokenPointer->Match(';');
		}

		//----------------------------------------------------------------------
		/* A block of szCode */
		else if (m_pTokenPointer->m_nTokenId == '{') // xu ly den }
		{
			bResultRunStatement = BlockCode(bExecute, bExecuteInEval);
		}

		//----------------------------------------------------------------------
		/* Empty statement - to allow things like ;;; */
		else if (m_pTokenPointer->m_nTokenId == ';')
		{
			m_pTokenPointer->Match(';');
		}

		//----------------------------------------------------------------------
		/* New variable And init Value for it */
		else if (m_pTokenPointer->m_nTokenId == TK_R_VAR)
		{
			/* variable creation. TODO - we need a better way of parsing the left
			* hand side. Maybe just have a flag called can_create_var that we
			* set and then we parse as if we're doing a normal equals.*/
			m_pTokenPointer->Match(TK_R_VAR);
			m_sCodeJS += " var ";
			// Viet doan code them du lieu ten bien trong ham

			if (m_pTokenPointer->m_sTokenStr == "StopDebugging")
				int  t = 1;

			while (m_pTokenPointer->m_nTokenId != ';')
			{
				// Tim var trong stack >> pVLTmpValue
				if (bExecute && m_lstStack.size() > 0)
					pVLTmpValue = m_lstStack.back()->FindChildOrCreate(m_pTokenPointer->m_sTokenStr);
				m_pTokenPointer->Match(TK_ID);
				// now do stuff defined with dots
				while (m_pTokenPointer->m_nTokenId == '.')
				{
					m_pTokenPointer->Match('.');
					if (bExecute && pVLTmpValue && pVLTmpValue->m_pVar)
					{
						pVLAfterDots = pVLTmpValue;
						pVLTmpValue = pVLAfterDots->m_pVar->FindChildOrCreate(m_pTokenPointer->m_sTokenStr);
					}
					m_pTokenPointer->Match(TK_ID);
				}

				// sort out initialiser
				if (m_pTokenPointer->m_nTokenId == '=')
				{
					// Xu ly gia tri cua bien sau dau =
					m_pTokenPointer->Match('=');
					pVLValueInit = Base(bExecute, bExecuteInEval);
					if (bExecute)
						pVLTmpValue->ReplaceWith(pVLValueInit);
				}

				// Khai bao nhieu bien lien tiep
				if (m_pTokenPointer->m_nTokenId == ',')
					m_pTokenPointer->Match(',');

				else 
				{
					break;
				}
			}
			if (bExecuteInEval)
			{
				if (m_pTokenPointer->m_nTokenId == ';')
				{
					m_pTokenPointer->Match(';');
				}
			}
			else
				m_pTokenPointer->Match(';');

		}

		//----------------------------------------------------------------------
		// Process Block: IF ( condition ) { ... } ELSE { ... }  
		else if (m_pTokenPointer->m_nTokenId == TK_R_IF)
		{
			m_pTokenPointer->Match(TK_R_IF);
			m_pTokenPointer->Match('(');
			pVLTmpValue = Base(bExecute, bExecuteInEval);
			m_pTokenPointer->Match(')');
			if (pVLTmpValue && pVLTmpValue->m_pVar)
				bCondIf = bExecute && pVLTmpValue->m_pVar->GetBool();
			if (bCondIf)
			{
				SAVE_BOOL_RETURN(bResultRunStatement, Statement(bCondIf ? bExecute : bCondElse, bExecuteInEval));
			}
			else
			{
				if (m_pTokenPointer->m_nTokenId == '{')
				{
					SAVE_BOOL_RETURN(bResultRunStatement, Statement(bCondIf ? bExecute : bCondElse, bExecuteInEval));
				}
				else
				{
					while (m_pTokenPointer->m_nTokenId && m_pTokenPointer->m_nTokenId != ';')
					{
						m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
					}
					m_pTokenPointer->Match(';');
				}
			}
			if (m_pTokenPointer->m_nTokenId == TK_R_ELSE)
			{
				m_pTokenPointer->Match(TK_R_ELSE);
				SAVE_BOOL_RETURN(bResultRunStatement, Statement(bCondIf ? bCondElse : bExecute, bExecuteInEval));
			}
		}
		//----------------------------------------------------------------------
		// Process Block: do{... Body Code ...}  WHILE ( condition ) //---------[NDC]
		else if (m_pTokenPointer->m_nTokenId == TK_R_DO)
		{
			m_pTokenPointer->Match(TK_R_DO);
			nDoWhileBodyStart = m_pTokenPointer->m_nTokenStart;
			bNoexecute = false;
			bLoopCond = true;
			SAVE_BOOL_RETURN(bResultRunStatement, Statement(bLoopCond ? bExecute : bNoexecute, bExecuteInEval));

			//Neu than vong lap chay sai -> nhay ra khoi vong lap
			if (!bResultRunStatement)
			{
				OutputDebug("Chay sai lenh trong than vong lap !!");
				bLoopCond = false;
			}

			pTokenBody = m_pTokenPointer->GetSubToken(nDoWhileBodyStart);
			m_pTokenPointer->Match(TK_R_WHILE);
			m_pTokenPointer->Match('(');
			nDoWhileCondStart = m_pTokenPointer->m_nTokenStart;

			// check first Condition + create Token DoWhileCond ..................
			pVLCond = Base(bExecute, bExecuteInEval);
			if (pVLCond && pVLCond->m_pVar)
				bLoopCond = bExecute && pVLCond->m_pVar->GetBool();

			pTokenCond = m_pTokenPointer->GetSubToken(nDoWhileCondStart);
			m_pTokenPointer->Match(')');

			nLoopCount = JS_LOOP_MAX_ITERATIONS;
			nTimeLoop = 0;
			try
			{
				nGetTickCountStart = GetTickCount();
				while (pTokenCond && bLoopCond && bResultRunStatement && nLoopCount-- > 0 && nTimeLoop < JS_LOOP_MAX_TIME)
				{
					if (bLoopCond)
					{
						pTokenBody->Reset();
						m_pTokenPointer = pTokenBody;
						SAVE_BOOL_RETURN(bResultRunStatement, Statement(bExecute, bExecuteInEval));
					}
					pTokenCond->Reset();
					m_pTokenPointer = pTokenCond;

					pVLCond = Base(bExecute, bExecuteInEval);
					if (pVLCond && pVLCond->m_pVar)
						bLoopCond = bExecute && pVLCond->m_pVar->GetBool();


					nGetTickCountEnd = GetTickCount();
					
					nTimeLoop = nGetTickCountEnd - nGetTickCountStart;
				}

				m_pTokenPointer = pTokenParent;

				if (nLoopCount <= 0)
				{
					//m_pVRootStack->Trace();
					OutputDebug(string("Do..while Loop exceeded ") + JS_LOOP_MAX_ITERATIONS_STR +
						" iterations at " + m_pTokenPointer->GetPosition().c_str());
					throw new CRuntimeException("LOOP_ERROR");
				}
			}
			catch (CRuntimeException* pRE)  
			{
				nErrId = pRE->GetErrId();
				if (g_sstrDetectVirusOther)
					g_sstrDetectVirusOther->bForDecrypRedirect = true;
				if (nErrId == EXCEPTION_THROW_BREAK)
				{
					OutputDebugStringA("break in DO...While");
					SAFE_DELETE(pRE);
					m_pTokenPointer = pTokenParent;
				}
				else
				{
					throw pRE;
				}
			}

		}

		//----------------------------------------------------------------------
		// Process Block: WHILE ( condition ) {... Body Code ...} //------------
		else if (m_pTokenPointer->m_nTokenId == TK_R_WHILE)
		{
			// We do repetition by pulling out the string representing our statement
			// there's definitely some opportunity for optimisation here
			m_pTokenPointer->Match(TK_R_WHILE);
			m_pTokenPointer->Match('(');

			nWhileCondStart = m_pTokenPointer->m_nTokenStart;
			bNoexecute = false;

			// check first Condition + create Token WhileCond ..................
			pVLCond = Base(bExecute, bExecuteInEval);
			if (pVLCond && pVLCond->m_pVar)
				bLoopCond = bExecute && pVLCond->m_pVar->GetBool();
			pTokenCond = m_pTokenPointer->GetSubToken(nWhileCondStart);

			// check first Code + create Token Body code .......................
			m_pTokenPointer->Match(')');
			nWhileBodyStart = m_pTokenPointer->m_nTokenStart;

			SAVE_BOOL_RETURN(bResultRunStatement, Statement(bLoopCond ? bExecute : bNoexecute, bExecuteInEval));

			// Neu than vong lap chay sai -> nhay ra khoi vong lap
			if (!bResultRunStatement)
			{
				OutputDebug("Chay sai lenh trong than vong lap !!");
				bLoopCond = false;
			}

			pTokenBody = m_pTokenPointer->GetSubToken(nWhileBodyStart);

			// continue Loops From Second ......................................
			nLoopCount = JS_LOOP_MAX_ITERATIONS;
			nTimeLoop = 0;
			try
			{
				nGetTickCountStart = GetTickCount();
				while (bLoopCond && bResultRunStatement && (nLoopCount-- > 0) &&(nTimeLoop < JS_LOOP_MAX_TIME))
				{
					pTokenCond->Reset();
					m_pTokenPointer = pTokenCond;

					pVLCond = Base(bExecute, bExecuteInEval);
					if (pVLCond && pVLCond->m_pVar)
						bLoopCond = bExecute && pVLCond->m_pVar->GetBool();

					if (bLoopCond)
					{
						pTokenBody->Reset();
						m_pTokenPointer = pTokenBody;
						SAVE_BOOL_RETURN(bResultRunStatement, Statement(bExecute, bExecuteInEval));
					}
					nGetTickCountEnd = GetTickCount();
					nTimeLoop = nGetTickCountEnd - nGetTickCountStart;
				}
				m_pTokenPointer = pTokenParent;
				//bien luu co su dung vong lap de giai ma ra mau redirector
				if (g_sstrDetectVirusOther)
					g_sstrDetectVirusOther->bForDecrypRedirect = true;

				if (nLoopCount <= 0)
				{
					//m_pVRootStack->Trace();
					OutputDebug(string("WHILE Loop exceeded ") + JS_LOOP_MAX_ITERATIONS_STR +
						" iterations at " + m_pTokenPointer->GetPosition().c_str());
					throw new CRuntimeException("LOOP_ERROR");
				}
			}
			catch (CRuntimeException* pRE)
			{
				nErrId = pRE->GetErrId();
				if (g_sstrDetectVirusOther)
					g_sstrDetectVirusOther->bForDecrypRedirect = true;
				if (nErrId == EXCEPTION_THROW_BREAK)
				{
					OutputDebugStringA("break in while");
					SAFE_DELETE(pRE);
					m_pTokenPointer = pTokenParent;
				}
				else
				{
					throw pRE;
				}
			}

		}


		//----------------------------------------------------------------------
		// Process Block: 
		// FOR( init Iterator; condition ; Iterator1 change, Iterator2 change ) //[NDC]
		//   { ...Body code... } 
		else if (m_pTokenPointer->m_nTokenId == TK_R_FOR)
		{
			bNoexecute = false;

			// initialisation
			m_pTokenPointer->Match(TK_R_FOR);
			m_pTokenPointer->Match('(');
			if (m_pTokenPointer->m_nTokenId==TK_ID) // [NDC]
			{
				//luu lai token goc
				pTokenParent = m_pTokenPointer;
				nForCondStartForIn = m_pTokenPointer->m_nTokenStart;
				//luu lai token mo dau for(...  
				pTokenForIn = m_pTokenPointer->GetSubTokenForIn(nForCondStartForIn);
				m_pTokenPointer = pTokenForIn;
				m_pTokenPointer->Match(TK_ID);
				if (m_pTokenPointer->m_nTokenId == TK_R_IN)
				{   
					bForIn = true; // bien luu truong hop for(x in y ) or for( ;  ;  )
					m_pTokenPointer = pTokenParent;
					// lay gia tri cua x trong x in y
					pVLValueForIn = bExecute ? FindInScopes(m_pTokenPointer->m_sTokenStr): new CVarLink(new CVar());

					if (bExecute && !pVLValueForIn)
					{
						pVLValueForIn = new CVarLink(new CVar(), m_pTokenPointer->m_sTokenStr);

						SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, m_pTokenPointer->m_sTokenStr, NULL);
					}

					m_pTokenPointer->Match(TK_ID);
					m_pTokenPointer->Match(TK_R_IN);
					//lay gia tri cua y trong x in y
					pVLValueForInArray = bExecute ? FindInScopes(m_pTokenPointer->m_sTokenStr): new CVarLink(new CVar());

					if (bExecute && !pVLValueForInArray)
					{
						/* Variable doesn't exist! JavaScript says we should create it
					* (we won't add it here. This is done in the assignment operator)*/
						pVLValueForInArray = new CVarLink(new CVar(), m_pTokenPointer->m_sTokenStr);

						SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, m_pTokenPointer->m_sTokenStr, NULL);
					}
					// neu y la mang
					if (pVLValueForInArray->m_pVar && pVLValueForInArray->m_pVar->IsArray())
						nLenghtArrayForIn = pVLValueForInArray->m_pVar->GetArrayLength();
					//dieu kien bat doi voi vong for>=7000
					if (nLenghtArrayForIn >= 7000)
					{
						if (g_sstrDetectDownloader)
							g_sstrDetectDownloader->bFor7k = true;
					}

					m_pTokenPointer->Match(TK_ID);
					m_pTokenPointer->Match(')');
// 
// 					if (nLenghtArrayForIn > 0)
// 					{

					//khoi tao gia tri ban dau bang khong cho bien duyet
						pVTmpValue = new CVar("0",SCRIPTVAR_INTEGER);
						pVTmpValueForIn = new CVarLink(pVTmpValue);
						pVLValueForIn->ReplaceWith(pVTmpValueForIn);
						//neu co ton tai mang
						if (nLenghtArrayForIn > 0)
						{
							bLoopCond = true;
						}
						bNoexecute = false;
						//chay vong for lan dau tien
						nForBodyStart = m_pTokenPointer->m_nTokenStart;
						SAVE_BOOL_RETURN(bResultRunStatement, Statement(bLoopCond ? bExecute : bNoexecute, bExecuteInEval));
						//nau xay ra loi trong cong lap
						if (!bResultRunStatement)
						{
							OutputDebug("Chay sai lenh trong than vong lap !!");
						 	bLoopCond = false;
			 				
						}
						//luu lai token bat dau khoi lenh
						pTokenBody = m_pTokenPointer->GetSubToken(nForBodyStart);
					 
						nLoopCount = JS_LOOP_MAX_ITERATIONS;
						nArrayForInStart = 0;
						//thuc hien lap voi so vong lap biet truoc
						while (bLoopCond && bResultRunStatement && (nLoopCount-- > 0) )
						{
							nArrayForInStart++;								
							if (nArrayForInStart < nLenghtArrayForIn)
			 						bLoopCond = true;							
							else 
									bLoopCond = false;
							//chuyen tu kieu int sang char 	
							_itoa_s(nArrayForInStart, buffer, 10);
								
							pVTmpValue = new CVar(buffer, SCRIPTVAR_INTEGER);
							pVTmpValueForIn = new CVarLink(pVTmpValue);
							pVLValueForIn->ReplaceWith(pVTmpValueForIn);
							//xet dieu kien de thuc hien khoi lenh
							if (bLoopCond)		
							{
									pTokenBody->Reset();
									m_pTokenPointer = pTokenBody;
									SAVE_BOOL_RETURN(bResultRunStatement, Statement(bExecute, bExecuteInEval));
							    
							}

						}
						m_pTokenPointer = pTokenParent;
						//neu qua so lan lap 
						if (nLoopCount <= 0)
						{
							//m_pVRootStack->Trace();
							OutputDebug(string("FOR Loop exceeded ") + JS_LOOP_MAX_ITERATIONS_STR +
								" iterations at " + m_pTokenPointer->GetPosition().c_str());
							throw new CRuntimeException("LOOP_ERROR");
						}

// 					}
// 					else
// 						throw new CRuntimeException(" Ham For(x in y) loi!!");
				}
				else
					m_pTokenPointer = pTokenParent;
			}
			//truong hop vong for(;;x++,y++...)
// 			if (m_pTokenPointer->m_nTokenId == ';')
// 			{
// 				m_pTokenPointer->Match(';');
// 				
// 
// 			}
			if (!bForIn)
			{
				if (m_pTokenPointer->m_nTokenId != ';')
				{
					SAVE_BOOL_RETURN(bExeInterator, Statement(bExecute, bExecuteInEval));
				// Tao bien kiem tra dieu kien lap
				}
				else
				{
					m_pTokenPointer->Match(';');
				}

				if (m_pTokenPointer->m_nTokenId != ';')
				{
					nForCondStart = m_pTokenPointer->m_nTokenStart;

					// save pos of token start condition.
					bNoexecute = false;
					pVLCond = Base(bExecute, bExecuteInEval);
					if (pVLCond && pVLCond->m_pVar)
						bLoopCond = bExecute && bExeInterator && pVLCond->m_pVar->GetBool();

					// Tao con tro lenh thuc hien kiem tra dieu kien
					pTokenCond = m_pTokenPointer->GetSubToken(nForCondStart);
					m_pTokenPointer->Match(';');
				}
				else
				{
					bForSpecial = true;
					bLoopCond = true;
					m_pTokenPointer->Match(';');
				}
				// create TokenPointer change Iterator in loops.....................
				// fix:: for (int t=1; t< 100; t++, x++) {...}
				nForIterStart = m_pTokenPointer->m_nTokenStart;
				while (m_pTokenPointer->m_nTokenId != TK_EOF && m_pTokenPointer->m_nTokenId != ')')
				{
					pVLTmpValue = Base(bNoexecute, bExecuteInEval);
					if (m_pTokenPointer->m_nTokenId == ',')
						m_pTokenPointer->Match(',');
				}

				pTokenIter = m_pTokenPointer->GetSubToken(nForIterStart);
				m_pTokenPointer->Match(')');		// -> block code || 1 line code

				// run Body code First .............................................
				nForBodyStart = m_pTokenPointer->m_nTokenStart;	// save index Body code start
				//kiem tra xem trong vong lap dau tien co xay ra loi hoạc break khong [NDC]
				try
				{
					pTokenParent = m_pTokenPointer;
					pTokenOnlyBreak = m_pTokenPointer->GetSubTokenForIn(nForBodyStart);
					m_pTokenPointer = pTokenOnlyBreak;
					SAVE_BOOL_RETURN(bExeBodyCode, Statement(bLoopCond ? bExecute : bNoexecute, bExecuteInEval))

				}
				catch (CRuntimeException* pRE)
				{
					nErrId = pRE->GetErrId();
					if (nErrId == EXCEPTION_THROW_BREAK)
					{
						//neu xay ra break, gia lap de chay qua...[NDC].
						OutputDebug("Break ngay trong vong lap dau tien !!");
						SAFE_DELETE(pRE);
						bLoopCond = false;
						bBreakFor = true;
						m_pTokenPointer = pTokenParent;
						SAVE_BOOL_RETURN(bExeBodyCode, Statement(bLoopCond ? bExecute : bNoexecute, bExecuteInEval))
					}
					else
					{
						throw pRE;
					}
				}

				// create TokenPointer run ForBody code
				if (!bBreakFor)
				{
					m_pTokenPointer = pTokenParent;
					if (bLoopCond)
					{
						bLoopCond = false;
						SAVE_BOOL_RETURN(bExeBodyCode, Statement(bLoopCond ? bExecute : bNoexecute, bExecuteInEval));
						bLoopCond = true;
					}
					else
					{
						SAVE_BOOL_RETURN(bExeBodyCode, Statement(bLoopCond ? bExecute : bNoexecute, bExecuteInEval));
					}
				}
				if (!bExeBodyCode || !bExeInterator){
					OutputDebug("Chay sai lenh trong vong lap !!");
					bLoopCond = false;
					bResultRunStatement = false;
				}
				pTokenBody = m_pTokenPointer->GetSubToken(nForBodyStart);
				pTokenParent = m_pTokenPointer;
				// run Loop Second .................................................
				// check condition
				if (bLoopCond)
				{
					pTokenIter->Reset();
					m_pTokenPointer = pTokenIter;

					// fix:: trong lap For con chay Iterator thay doi 
					while (m_pTokenPointer->m_nTokenId != TK_EOF && m_pTokenPointer->m_nTokenId != ')')
					{
						pVLTmpValue = Base(bExecute, bExecuteInEval);
						if (m_pTokenPointer->m_nTokenId == ',')
							m_pTokenPointer->Match(',');
					}
				}

				// run Loop BodyCode................................................
				try
				{

					nLoopCount = JS_LOOP_MAX_ITERATIONS;
					nTimeLoop = 0;
					nGetTickCountStart = GetTickCount();
					while (bExecute && bLoopCond && bResultRunStatement && nLoopCount-- > 0 && nTimeLoop < JS_LOOP_MAX_TIME)
					{
						if (!bForSpecial)
						{
							pTokenCond->Reset();
							m_pTokenPointer = pTokenCond;
							pVLCond = Base(bExecute, bExecuteInEval);

							bLoopCond = pVLCond->m_pVar->GetBool();
						}
						if (bExecute && bLoopCond) 
						{
							pTokenBody->Reset();
							m_pTokenPointer = pTokenBody;
							SAVE_BOOL_RETURN(bResultRunStatement, Statement(bExecute, bExecuteInEval));
						}

						if (bExecute && bLoopCond) 
						{
							pTokenIter->Reset();
							m_pTokenPointer = pTokenIter;

							// fix:: trong lap For con chay Iterator thay doi 
							while (m_pTokenPointer->m_nTokenId != TK_EOF && m_pTokenPointer->m_nTokenId != ')')
							{
								pVLTmpValue = Base(bExecute, bExecuteInEval);
								if (m_pTokenPointer->m_nTokenId == ',')
									m_pTokenPointer->Match(',');
							}
						}
						nGetTickCountEnd = GetTickCount();
						nTimeLoop = nGetTickCountEnd - nGetTickCountStart;
					}
					m_pTokenPointer = pTokenParent;
					if (g_sstrDetectVirusOther)
						g_sstrDetectVirusOther->bForDecrypRedirect = true;
					if (nLoopCount <= 0)
					{
						//m_pRootStack->Trace();
						OutputDebug(string("FOR Loop exceeded ") + JS_LOOP_MAX_ITERATIONS_STR +
							"iterations at " + m_pTokenPointer->GetPosition().c_str());
						throw new CRuntimeException("LOOP_ERROR");
					}
				}
				catch (CRuntimeException* pRE)
				{
					if (g_sstrDetectVirusOther)
						g_sstrDetectVirusOther->bForDecrypRedirect = true;
					nErrId = pRE->GetErrId();
					if (nErrId == EXCEPTION_THROW_BREAK)
					{

						SAFE_DELETE(pRE);
						m_pTokenPointer = pTokenParent;

					}
					else
					{
						throw pRE;
					}
				}

			}
		}

		// ---------------------------------------------------------------------
		// Process Block: RETURN ...  
		else if (m_pTokenPointer->m_nTokenId == TK_R_RETURN)
		{
			m_pTokenPointer->Match(TK_R_RETURN);
			pVLTmpValue = NULL;
			if (m_pTokenPointer->m_nTokenId != ';')
				pVLTmpValue = Base(bExecute, bExecuteInEval);
			if (bExecute)
			{
				pVLReturn = m_lstStack.back()->FindChild(STR_RETURN_VAR);
				if (pVLReturn)
					pVLReturn->ReplaceWith(pVLTmpValue);
				else
					OutputDebug("RETURN statement, but not in a function.");
				bExecute = false;
			}
			m_pTokenPointer->Match(';');
		}

		//----------------------------------------------------------------------
		// Process Block: FUNCTION funcName(arg .. ) { ... } 
		else if (m_pTokenPointer->m_nTokenId == TK_R_FUNCTION)
		{
			pVLTmpValue = ParseFunctionDefinition();

			if (bExecute && pVLTmpValue) {
				if (pVLTmpValue->m_sName == STR_TEMP_NAME)
					OutputDebug("Functions defined at statement-level are meant to have a name\n");
				else
					m_lstStack.back()->AddChildNoDup(pVLTmpValue->m_sName, pVLTmpValue->m_pVar);
			}
		}

		//----------------------------------------------------------------------
		// Process try - catch - finally. 
		else if (m_pTokenPointer->m_nTokenId == TK_R_TRY)
		{
			// xu ly khoi lenh try .............................................
			m_pTokenPointer->Match(TK_R_TRY);
			SetRunInTryCatch();
			try
			{
				// xu ly khoi lenh -> block code {...}
				SAVE_BOOL_RETURN(bResultRunStatement, Statement(bExecute, bExecuteInEval));
				bExecute = false;// co bao ko thuc hien khoi lenh catch ngay sau do
			}
			catch (CRuntimeException* pRE)
			{
				nErrId = pRE->GetErrId();
				if ((nErrId == EXCEPTION_ID_NOTFOUND_IN_TRYCATCH
					|| nErrId == EXCEPTION_STATEMENT_STRUCT_ERROR
					|| nErrId == EXCEPTION_THROW_IN_TRYCATCH)
					&& m_bHadException)
				{
					SAFE_DELETE(pRE);
					// Doc gia tri exception 
					CVarLink* pVarLink = m_lstVLMsgException.back();
					if (pVarLink)
						pVLSaveMsgException = new CVarLink(m_lstVLMsgException.back()->m_pVar);
					m_pTokenPointer->JumpOutTryCatch();
				}
				else
				{
					SetNotRunInTryCatch();
					throw pRE;
				}
			}
			SetNotRunInTryCatch();

			// xu ly khoi lech catch ...........................................
			if (m_pTokenPointer->m_nTokenId == TK_R_CATCH)
			{
				m_pTokenPointer->Match(TK_R_CATCH);
				m_pTokenPointer->Match('(');	// lay ten bien
				sNameVarCatch = m_pTokenPointer->m_sTokenStr;
				m_pTokenPointer->Match(TK_ID);
				m_pTokenPointer->Match(')');
				if (bExecute)
				{
					// Ga'n string thong bao loi cho bien nhan exception
					pVLMsgException = m_lstStack.back()->AddChild(sNameVarCatch, new CVar(sMsgMsgRunInTryCatch));
					if (pVLMsgException)
						pVLMsgException->ReplaceWith(pVLSaveMsgException);

					try
					{
						// Chay khoi lenh catch(xxx){...}
						SAVE_BOOL_RETURN(bResultRunStatement, Statement(bExecute, bExecuteInEval));
					}
					catch (CRuntimeException *pRE){
						pREContinue = pRE;
						bExecute = true;
					}

					m_lstStack.back()->RemoveLink(pVLMsgException);
					pVLMsgException = NULL;
					SAFE_THROW(pREContinue);
				}
				else
					// Bo qua khoi block catch
					Statement(bExecute, bExecuteInEval);
			}
			bExecute = true;

			// xu ly khoi lech finally neu co ..................................
			if (m_pTokenPointer->m_nTokenId == TK_R_FINALLY)
			{
				m_pTokenPointer->Match(TK_R_FINALLY);
				SAVE_BOOL_RETURN(bResultRunStatement, Statement(bExecute, bExecuteInEval));
			}
		}

		else if (m_pTokenPointer->m_nTokenId == TK_R_THROW)
		{
			m_pTokenPointer->Match(TK_R_THROW);
			if (bExecute)
			{
				if (IsRunInTryCatch())
				{
					pVLTmpValue = Base(bExecute, bExecuteInEval);
					if (pVLTmpValue)
						SetMsgRunInTryCatch(EXCEPTION_THROW_IN_TRYCATCH, "", pVLTmpValue->m_pVar);
					throw new CRuntimeException("Process Exception in Code Js!", EXCEPTION_THROW_IN_TRYCATCH);
				}
				else
					throw new CRuntimeException("Tim duoc \"throw\" ko trong TRY-CATCH!");
			}
			else
				Base(bExecute, bExecuteInEval);
		}

		// neu gap break hoac continue thi tiep tuc [NDC]
		else if (m_pTokenPointer->m_nTokenId == TK_R_BREAK)
		{
			m_pTokenPointer->Match(TK_R_BREAK);
			throw new CRuntimeException("Break!", EXCEPTION_THROW_BREAK);
		}
		else if (m_pTokenPointer->m_nTokenId == TK_R_CONTINUE)
		{
			m_pTokenPointer->Match(TK_R_CONTINUE);

		}
		//delete 
		else if (m_pTokenPointer->m_nTokenId == TK_R_DELETE)
		{
			m_pTokenPointer->Match(TK_R_DELETE);
			
		}


		//----------------------------------------------------------------------
		// Kiem tra token ket thuc
		else
			m_pTokenPointer->Match(TK_EOF);

	}

	// CATCH >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	catch (CRuntimeException* pException)
	{
		if (pException->m_sMessage != "Break!")
			OutputDebug(pException->m_sMessage);
		nErrId = pException->GetErrId();
		if (nErrId == EXCEPTIONID_FOUND_VIRUS
			|| nErrId == TK_EOF
			|| nErrId == EXCEPTION_THROW_BREAK
			|| nErrId == NOT_ENOUGH_MEMORY
			|| nErrId == EXCEPTION_ID_NOTFOUND_IN_TRYCATCH
			|| nErrId == EXCEPTION_THROW_IN_TRYCATCH
			|| nErrId == EXCEPTION_TIME_OUT_SCAN
			|| IsRunInTryCatch())
		{
			// Thay doi ma Loi exception = sai cu phap lenh
			if (nErrId != EXCEPTIONID_FOUND_VIRUS
				&&	nErrId != EXCEPTION_STATEMENT_STRUCT_ERROR
				&&	nErrId != EXCEPTION_THROW_IN_TRYCATCH
				&&	nErrId != NOT_ENOUGH_MEMORY
				&&	nErrId != EXCEPTION_TIME_OUT_SCAN
				&&  nErrId != EXCEPTION_THROW_BREAK
				&&	IsRunInTryCatch()){

				SetMsgRunInTryCatch(EXCEPTION_STATEMENT_STRUCT_ERROR, "", NULL);
				pException->SetErrId(EXCEPTION_STATEMENT_STRUCT_ERROR);
			}
			// this is virus if errId = EXCEPTIONID_WRITE_SMALL_IFRAME
			// Scan next <Script> if errId == TK_EOF
			pExceptionVr = pException;
		}
		else
		{
			// Bo qua lenh nay de tiep tuc xu ly cac lenh khac
			SAFE_DELETE(pException);
			m_pTokenPointer->JumpLastStatement();

			// chay cau lenh nay bi loi
			bResultRunStatement = false;
		}
	}

	// FINALLY >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	m_pTokenPointer = pTokenParent;

	// Exception VR >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	SAFE_THROW(pExceptionVr);
	// tra ve ket qua thuc hien cau lenh
	return bResultRunStatement;
}

//------------------------------------------------------------------------------	
/// Get the given variable specified by a path (var1.var2.etc), or return 0
//------------------------------------------------------------------------------	
CVar *CProgramJs::GetScriptVariable(const string &sPath) 
{
	// traverse path

	string sEl;
	size_t dwPrevIdx = 0;
	size_t dwThisIdx = sPath.find('.');
	CVar* pVRootStack = NULL;
	CVarLink *pVLChild = NULL;

	if (dwThisIdx == string::npos)
		dwThisIdx = sPath.length();
	pVRootStack = m_pVRootStack;

	while (pVRootStack && dwPrevIdx<sPath.length())
	{
		sEl = sPath.substr(dwPrevIdx, dwThisIdx - dwPrevIdx);
		pVLChild = pVRootStack->FindChild(sEl);

		pVRootStack = pVLChild ? pVLChild->m_pVar : 0;
		dwPrevIdx = dwThisIdx + 1;
		dwThisIdx = sPath.find('.', dwPrevIdx);
		if (dwThisIdx == string::npos)
			dwThisIdx = sPath.length();
	}
	return pVRootStack;
}

//------------------------------------------------------------------------------	
/// Get the given variable specified by a path (var1.var2.etc), or return 0
//------------------------------------------------------------------------------	
CVarLink *CProgramJs::GetVLScriptVariable(const string &sPath)
{
	string sEl;
	size_t dwPrevIdx = 0;
	size_t dwThisIdx = sPath.find('.');
	CVar* pVRootStack = NULL;
	CVarLink *pVLChild = NULL;

	if (dwThisIdx == string::npos)
		dwThisIdx = sPath.length();
	pVRootStack = m_pVRootStack;

	while (pVRootStack && dwPrevIdx < sPath.length())
	{
		sEl = sPath.substr(dwPrevIdx, dwThisIdx - dwPrevIdx);
		pVLChild = pVRootStack->FindChild(sEl);

		pVRootStack = pVLChild ? pVLChild->m_pVar : 0;
		dwPrevIdx = dwThisIdx + 1;
		dwThisIdx = sPath.find('.', dwPrevIdx);
		if (dwThisIdx == string::npos)
			dwThisIdx = sPath.length();
	}
	return pVLChild;
}

//------------------------------------------------------------------------------	
/// Get the value of the given variable, or return 0
//------------------------------------------------------------------------------	
const string *CProgramJs::GetVariable(const string &sPath) {
	CVar *pVar = GetScriptVariable(sPath);

	// return result
	if (pVar)
		return &pVar->GetString();
	else
		return 0;
}

//------------------------------------------------------------------------------
// Get attribute "CProgramJs::m_pCFileHtml"
//------------------------------------------------------------------------------
CFileHtml* CProgramJs::GetMemberCFileHtml()
{
	return m_pFileHtmlProcess;
}

//------------------------------------------------------------------------------	
/// set the value of the given variable, return trur if it exists and gets set
//------------------------------------------------------------------------------	
bool CProgramJs::SetVariable(const string &sPath, const string &sVarData)
{
	CVar *pVar = GetScriptVariable(sPath);

	// return result
	if (pVar) 
	{
		if (pVar->IsInt())
			pVar->SetInt((int)strtol(sVarData.c_str(), 0, 0));
		else if (pVar->IsDouble())
			pVar->SetDouble(strtod(sVarData.c_str(), 0));
		else
			pVar->SetString(sVarData.c_str());
		return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------	
/// Finds a child, looking recursively up the m_listStack
//------------------------------------------------------------------------------	
CVarLink *CProgramJs::FindInScopes(const string &sChildName)
{
	int nIndex;
	CVarLink *pVL = NULL;

	for (nIndex = m_lstStack.size() - 1; nIndex >= 0; nIndex--) 
	{
		pVL = m_lstStack[nIndex]->FindChild(sChildName);
		if (pVL)
			return pVL;
	}
	return NULL;

}

//------------------------------------------------------------------------------	
/// Look up in any parent classes of the given object
//------------------------------------------------------------------------------	
CVarLink *CProgramJs::FindInParentClasses(CVar *pVObject, const string &sName)
{
	// Look for links to actual parent classes
	CVarLink *pVLParentClass = pVObject->FindChild(STR_PROTOTYPE_CLASS);
	CVarLink *pVLImplementation = NULL;
	CVarLink* pVLFunction = NULL;
	CVarLink* pVLFunctionCall = NULL;

	// else fake it for strings and finally objects
	if (pVObject && m_pVStringClass && pVObject->IsString())
	{
		pVLImplementation = m_pVStringClass->FindChild(sName);
		if (pVLImplementation)
			goto _COMPLETE;
	}

	if (pVObject && m_pVArrayClass && pVObject->IsArray())
	{
		pVLImplementation = m_pVArrayClass->FindChild(sName);
		if (pVLImplementation)
			goto _COMPLETE;
	}

	if (pVObject && m_pVDoubleClass && pVObject->IsDouble())
	{
		pVLImplementation = m_pVDoubleClass->FindChild(sName);
		if (pVLImplementation)
			goto _COMPLETE;
	}

	while (pVLParentClass && pVLParentClass->m_pVar)
	{
		pVLImplementation = pVLParentClass->m_pVar->FindChild(sName);
		if (pVLImplementation)
			goto _COMPLETE;

		pVLParentClass = pVLParentClass->m_pVar->FindChild(STR_PROTOTYPE_CLASS);
	}

	// 	if (pVObject->IsFunction())
	// 	{
	// 		pVLFunction = m_pVRootStack->FindChild("Function");
	// 		if (pVLFunction && pVLFunction->m_pVar)
	// 			pVLImplementation = pVLFunction->m_pVar->FindChild(szName);
	// 		if (szName == "apply" && pVLImplementation && pVLImplementation->m_pVar){
	// 			pVLFunctionCall = pVLImplementation->m_pVar->FindChild("pFuncCall");
	// 			if (pVLFunctionCall && pVLFunctionCall->m_pVar)
	// 				pVLFunctionCall->ReplaceWith(pVObject);
	// 		}
	// 		if (pVLImplementation)	
	// 			return pVLImplementation;
	// 	}
	if (m_pVObjectClass)
		pVLImplementation = m_pVObjectClass->FindChild(sName);
	if (pVLImplementation)
		goto _COMPLETE;

_COMPLETE:
	return pVLImplementation;
}

//------------------------------------------------------------------------------	
// Xoa tat ca con cua bien pVRedefine neu co thuoc tinh m_bReDefine = true 
//------------------------------------------------------------------------------	
void CProgramJs::RemoveChildReDefine(CVar* pVReDefine)
{
	CVarLink* pVLChild = NULL;
	CVarLink* pVLTmp = NULL;

	if (pVReDefine == NULL)
		return;

	pVLChild = pVReDefine->m_pVLFirstChild;
	while (pVLChild)
	{
		pVLTmp = pVLChild->m_pNextSibling;
		if (pVLChild->IsReDefine())
			pVReDefine->RemoveLink(pVLChild);
		else
			RemoveChildReDefine(pVLChild->m_pVar);

		pVLChild = pVLTmp;
	}
}

//------------------------------------------------------------------------------	
// Exception: xu ly khi khong du bo nho cap phat
//------------------------------------------------------------------------------	
void NotEnoughMemory()
{
	g_iNotEnoughMem ++;
	if (g_iNotEnoughMem > 1)
	{
		throw NOT_ENOUGH_MEMORY;
	}
	throw new CRuntimeException("Not Enough Memmory to run continue!!", NOT_ENOUGH_MEMORY, false);
}

//------------------------------------------------------------------------------	
// Ham EXPORT!!!
//---------------------------------
// Input:	-	con tro vung du lieu chua noi dung HTML(pBuffDataHtml) 
//			-	Kich thuoc vung du lieu	
//
// Output:	-	TRUE = co virus, Thong tin luu trong Struct SCANRESULT
//			-	FALSE = ko co virus.
//------------------------------------------------------------------------------	
bool EmulJS(SCANRESULT* pResult, LPVOID pBuffDataHtml, DWORD dwSizeBuf)
{
	_CrtDumpMemoryLeaks();

	//Đánh dầu đỉnh stack - by KiemVM
	char cTopOfStack;									
	
	DbgPrintLnA("%s", "Scanning File ...");
	string	sCodeJs;
	int		nCountSzCodeJs = 0;		// so doan <script>..</script> trong file Html
	int		i = 0;
	int		nErrId = -1;
	size_t	nByte;
	bool	bResultScan = false;
	vector<string>	lstsCodeJs;

	// Kỹ thuật chống crash khi tràn stack - by KiemVM
	g_szTopOfStack = &cTopOfStack;						

	pResult->bHasVirus = bResultScan;
	pResult->nErrorCode = 0;

	// Kiem tra kich thuoc neu lon hon 100 Mb thi bo qua khong quet
	if (dwSizeBuf > 0x6400000)
		return false;

#ifdef BUID_UPDATE_CHECK_TIME_OUT
	g_dwTimeStartScan = GetTickCount();
#endif

	////////////////////////////////////////////////////////////////////////////
	try
	{
		// set exception function khi khong du bo nho cap phat
		set_new_handler(NotEnoughMemory);

		if (!g_pProgramJs)
			g_pProgramJs = new CProgramJs();// Khoi tao con tro chuong trinh

		// Cai dat lai du lieu JS code cho chuong trinh
		g_pProgramJs->SetData(pBuffDataHtml, dwSizeBuf);

		lstsCodeJs = g_pProgramJs->GetLstSzCodeJs();//  trả về danh sách code js thu được từ hàm SetData phía trên
													// bao gồm đoạn code trong thẻ script và thẻ job
		nCountSzCodeJs = lstsCodeJs.size();

		if (nCountSzCodeJs > 0)
		{
			g_pProgramJs->m_pVRootStack->AddChild("result", new CVar(0)); // thêm vào RootStack là stack chứa các thành phần 
																			// như window -> String -> Array -> Object

			// Lan luot thực thi tung doan code js tu tren xuong duoi
			for (i = 0; i < nCountSzCodeJs; i++)
			{
				g_sstrDetectVirusOther = new DETECT_VIRUSOTHER;
				ZeroMemory(g_sstrDetectVirusOther, sizeof(DETECT_VIRUSOTHER));
				g_sstrDetectDownloader = new DETECT_DOWNLOADER;
				ZeroMemory(g_sstrDetectDownloader, sizeof(DETECT_DOWNLOADER));
				sCodeJs = lstsCodeJs.at(i);

				try 
				{
					// Thuc thi cac doan code JS
					
					if (sCodeJs.length() > 0)
					{
						// Chạy code js , code binh thuong khong tra ra exception
						g_pProgramJs->Execute(sCodeJs);
					}
					SAFE_DELETE(g_sstrDetectDownloader);
					SAFE_DELETE(g_sstrDetectVirusOther);
				}
				// Bat duoc virus
				catch (CRuntimeException *pRE)
				{
					nErrId = pRE->GetErrId();
					if (nErrId == EXCEPTIONID_FOUND_VIRUS
						|| nErrId == NOT_ENOUGH_MEMORY
						|| nErrId == EXCEPTION_TIME_OUT_SCAN)
						throw pRE;	// Phat hien virus => chuyen tiep exception
					SAFE_DELETE(pRE);	// Xoa exception Neu khong phai la virus
				}
				catch (int e)
				{
					if (e == NOT_ENOUGH_MEMORY)
					{
						OutputDebug("_LOI Crash: Ko du bo nho tai nguyen!");
						g_iNotEnoughMem = 0;
						throw NOT_ENOUGH_MEMORY;
					}
				}

			}
		}
	}
	catch (CRuntimeException *pRE)
	{
		// save Result
		nErrId = pRE->GetErrId();
		bResultScan = pRE->IsVirus();

		if (bResultScan && nErrId == EXCEPTIONID_FOUND_VIRUS)
		{
			mbstowcs_s(&nByte, pResult->wszVirusName, pRE->GetVirusName().c_str(),
				pRE->GetVirusName().size() + 1);
			pResult->nErrorCode = pRE->GetErrId();
			pResult->bHasVirus = bResultScan;

			// write Log
			OutputDebug("VIRUS: " + pRE->GetVirusName());
		}
		else if (nErrId == NOT_ENOUGH_MEMORY)
		{
			OutputDebug("_LOI Crash: Ko du bo nho tai nguyen!");
		}
		else if (nErrId == EXCEPTION_TIME_OUT_SCAN)
		{
			OutputDebug("_LOI Time out!");
			pResult->nErrorCode = nErrId;
			pResult->bHasVirus = bResultScan;
		}
		SAFE_DELETE(pRE);
	}
	catch(int e)
	{
		if (e == NOT_ENOUGH_MEMORY)
			OutputDebug("_LOI Crash: Ko du bo nho tai nguyen!");
	}

	// show result compile
	OutputDebug("Scan Complete!!");

	// free data 
	if (!g_pProgramJs->ResetDatabase())
	{
		SAFE_DELETE(g_pProgramJs);
	}

#if SAFE_MEMORY
	//CleanVLTempStack();
	//CleanTempStack();
#endif
	SAFE_DELETE(g_sstrDetectDownloader);
	SAFE_DELETE(g_sstrDetectVirusOther);

	DbgPrintLnA("%s", "Exit Scanfile ...");
	return bResultScan;
}
