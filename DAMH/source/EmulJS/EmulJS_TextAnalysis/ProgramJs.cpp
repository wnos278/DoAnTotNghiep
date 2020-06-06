#include <exception>
#include <vector>
#include <sstream>
#include <assert.h>
#include "tchar.h"

#include "ProgramJs.h" 
#include "sha256.h"

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

#define ASSERT(X) assert(X)

extern CProgramJs*	g_pProgramJs;
extern DWORD		g_dwTimeStartScan;

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

string sSaveFileName;

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

	pVLWindow = m_pVRootStack->AddChild("window", m_pVRootStack, "", TRUE);
	pVLWindow->SetNotReDefine();

	// Phai cai dat sau khi AddChild("window"..) vao stack de ko them vao blacklist
	CVarLink::SetRootStack(m_pVRootStack);

	pVLDString = m_pVRootStack->AddChild("String", m_pVStringClass, "", TRUE);
	(m_pVRootStack->AddChild("Array", m_pVArrayClass, "", TRUE))->SetNotReDefine();
	(m_pVRootStack->AddChild("Object", m_pVObjectClass, "", TRUE))->SetNotReDefine();
	pVLDB = m_pVRootStack->AddChild("Double", m_pVDoubleClass, "", TRUE);

	//Them thuoc tinh String.status----Downloader
	m_pVLDStringStatus = pVLDString->m_pVar->AddChild("status", new CVar(200), "", TRUE);
	m_pVLDStringStatus->SetNotReDefine(); 

	m_pVLDStringresponseText = pVLDString->m_pVar->AddChild("responseText", new CVar("responseText"), "", TRUE);
	m_pVLDStringresponseText->SetNotReDefine();

	//Them thuoc tinh string.onreadystatechange ---Downloader
	m_pVLDStringOnreadyStateChange = pVLDString->m_pVar->AddChild("onreadystatechange", new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION), "", TRUE);
	m_pVLDStringOnreadyStateChange->SetNotReDefine();

	//Them thuoc tinh String.readyState
	m_pVLDStringreadyState = pVLDString->m_pVar->AddChild("readyState", new CVar(4), "", TRUE);
	m_pVLDStringreadyState->SetNotReDefine();

	// Them thuoc tinh String.length
	m_pVLDStringreadyState = pVLDString->m_pVar->AddChild("length", new CVar(4), "", TRUE);
	m_pVLDStringreadyState->SetNotReDefine();

	//Them thuoc tinh double.size ----Downloader
	m_pVLDStatus = pVLDB->m_pVar->AddChild("size", new CVar(1001), "", TRUE);
	m_pVLDStatus->SetNotReDefine();

	// Them doi tuong Html element
	(m_pVRootStack->AddChild("element", new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), "", TRUE))->SetNotReDefine();

	// Them thuoc tinh window.onload
	m_pVLWindowOnload =
		m_pVRootStack->AddChild("onload", new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION), "", TRUE);
	m_pVLWindowOnload->SetNotReDefine();

	// Them thuoc tinh Document.onmousemove
	pVLDcmnt = m_pVRootStack->AddChild("document", new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), "", TRUE);
	m_pVLDcmtOnmousemove = pVLDcmnt->m_pVar->AddChild("onmousemove", new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), "", TRUE);
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
	// func
	m_nameFuncCount = 0;
	
	indexInFunc = 0;
	m_inFunc[indexInFunc] = false;
	// var
	m_nameVarCount = 0;
	m_nameVarInFuncCount = 0;
	//catch
	m_nameVarCatchCount = 0;
	m_nameVarCatchFuncCount = 0;
	m_onreadystatechange = false;
	m_iCountArrayElement = 0;
	m_ForIn = false;
	this->InitNullForPropertyPointer();
	this->m_dwSizeData = 0;
	// make a dir to save source code
	system("mkdir \"source\"");
	InitProgramJs();
	sSaveFileName = "";
}

//------------------------------------------------------------------------------
// Khoi tao doi tuong CFileHtml
// Cai dat cac thong tin thuoc tinh cho trinh thong dich: CProgramJs
//------------------------------------------------------------------------------
CProgramJs::CProgramJs(LPVOID pBuff, DWORD dwSizeBuff, int nTypeFile)
{
	m_onreadystatechange = false;
	m_ForIn = false;
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

#if SAFE_MEMORY
	CleanTempStack();
#endif
}

//------------------------------------------------------------------------------
// Loc code JS tu pBuff chua noi dung file HTML => mang cac doan code JS
//------------------------------------------------------------------------------
void CProgramJs::SetData(LPVOID pBuffHtmlCode, DWORD dwSizeBuff)
{
	BYTE* pBuffHtmlCodeCopy = NULL;
	string sCodeHtml;
	SetNotRunInTryCatch();	// Khoi tao ko chay trong try-catch

	// Lay ds noi dung Code Js vao m_lstSzCodeJs
	pBuffHtmlCodeCopy = new BYTE[dwSizeBuff + 1];
	if (pBuffHtmlCodeCopy == NULL)
		throw new CRuntimeException("Error: Not Enough Mem!! File doesn't Scan!",CREATE_FILE_FALSE, false);

	// Copy dữ liệu code HTML vào biến pBuffHtmlCodeCopy
	memcpy(pBuffHtmlCodeCopy, pBuffHtmlCode, dwSizeBuff);
	pBuffHtmlCodeCopy[dwSizeBuff] = 0;

	sCodeHtml = ((char*)pBuffHtmlCodeCopy);
	sSaveFileName = ".\\source\\" + sha256(sCodeHtml);
	this->m_pFileHtmlProcess->SetCodeHtml(sCodeHtml); 
	
	m_lstsCodeJsJob = this->m_pFileHtmlProcess->FiltersOutCodeJsJob();
	m_lstsCodeJs = this->m_pFileHtmlProcess->FiltersOutCodeJs();
	if (m_lstsCodeJsJob.size() > 0)
	{
		for (size_t i = 0; i < m_lstsCodeJsJob.size(); i++)
		{
			m_lstsCodeJs.push_back(m_lstsCodeJsJob.at(i));
		}
	}
}

//------------------------------------------------------------------------------
// Khoi phuc chuog trinh ve trang thai ban dau, Voi cac ham API de tiep tuc quet 
// file HTML tiep theo
//------------------------------------------------------------------------------
BOOLEAN CProgramJs::ResetDatabase()
{
	BOOLEAN bResult = FALSE;
	m_sCodeSaveJS = "";
	sSaveFileName = "";
	m_nameFuncCount = 0;
	m_nameVarCount = 0;
	m_nameVarInFuncCount = 0;

	INT i = 0, nSize = 0;
	CVarLink* pVLObjDetete = NULL;
	CVarLink* pVLTmp = NULL;

	// Cai dat lai che do khong chay trong try-catch
	m_bRunInTryCatch = false;
	m_bHadException = false;

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
	try
	{
		if (m_pTokenPointer)
			m_pTokenPointer = new CTokenPointer(pTPOld, pTPOld->m_nTokenStart, sCode.size()); 
		else
			m_pTokenPointer = new CTokenPointer(sCode);
	}
	catch (CRuntimeException* pRE)
	{
		pRunTimeException = pRE;
	}
	try
	{
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
						pVLFunc->m_sAliasName = "func" + to_string(m_nameFuncCount);
						m_nameFuncCount += 1;
						if (m_lstStack.size() > 0)
							m_lstStack.back()->AddChildNoDup(pVLFunc->m_sName, pVLFunc->m_pVar, pVLFunc->m_sAliasName);
						else
							m_pVRootStack->AddChildNoDup(pVLFunc->m_sName, pVLFunc->m_pVar, pVLFunc->m_sAliasName);
					}
				}
				catch (CRuntimeException* pRE)
				{
					SAFE_DELETE(pRE);
				}
			}
			if (m_pTokenPointer->m_nTokenId == TK_R_VAR)
			{
				CVarLink *pVLTmpValue = NULL;
				CVarLink *pVLAfterDots = NULL;
				m_sCodeSaveJS += " var ";
				m_pTokenPointer->Match(TK_R_VAR);

				while (m_pTokenPointer->m_nTokenId != ';')
				{
					// Tim var trong stack >> pVLTmpValue
					if (m_lstStack.size() > 0)
					{
						// sontdc
						string aliasName = "";
						if (m_inFunc[indexInFunc] == false)
						{
							aliasName = "globalVar" + to_string(m_nameVarCount);
						}
						else
							aliasName = "funcVar" + to_string(m_nameVarInFuncCount);
						pVLTmpValue = m_lstStack.back()->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, aliasName);
						if (pVLTmpValue->m_sAliasName == aliasName) // Neu la bien moi
							if (m_inFunc[indexInFunc] == false)
								m_nameVarCount += 1;
							else m_nameVarInFuncCount += 1;
							m_sCodeSaveJS += " " + pVLTmpValue->m_sAliasName + " ";
					}

					m_pTokenPointer->Match(TK_ID);
					// now do stuff defined with dots
					while (m_pTokenPointer->m_nTokenId == '.')
					{
						m_pTokenPointer->Match('.');
						if (pVLTmpValue && pVLTmpValue->m_pVar)
						{
							pVLAfterDots = pVLTmpValue;
							pVLTmpValue = pVLAfterDots->m_pVar->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, "");
						}
						m_pTokenPointer->Match(TK_ID);
					}

					// sort out initialiser
					if (m_pTokenPointer->m_nTokenId == '=')
					{
						m_pTokenPointer->Match('=');
						m_sCodeSaveJS += " = ";
						bool bExecute = true;
						CVarLink *pVLValueInit = Base();
						if (bExecute)
							pVLTmpValue->ReplaceWith(pVLValueInit);
					}

					// Khai bao nhieu bien lien tiep
					if (m_pTokenPointer->m_nTokenId == ',')
					{
						m_sCodeSaveJS += " , ";
						m_pTokenPointer->Match(',');
					}


					else
					{
						break;
					}
				}
			}
			// type token = ID : bien trong stack-----------------------------------
			if (m_pTokenPointer->m_nTokenId == TK_ID)
			{
				CVarLink* pVLTmpValue;
				pVLTmpValue = Factor();
				if (pVLTmpValue)
				{
					m_sCodeSaveJS += " " + pVLTmpValue->m_sAliasName + " ";
					m_pTokenPointer->Match(';');
				}
				
			}
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
		}
		// Khoi phuc con tro lenh cu
		m_pTokenPointer = pTPOld;
	}
	catch (CRuntimeException* pRE)
	{
		pRunTimeException = pRE;
	}
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
void CProgramJs::Execute(const string &sCode)
{
	CTokenPointer *pTokenPointerOld;
	vector<CVar*> lstVStackOld;
	CRuntimeException *pREHandleProcess = NULL;

	pTokenPointerOld = m_pTokenPointer;
	lstVStackOld = m_lstStack;
	if (m_onreadystatechange != true)
	{
		// Tạo stack mới
		m_lstStack.clear();
		m_lstStack.push_back(m_pVRootStack);
	}
	try
	{
		m_pTokenPointer = new CTokenPointer(sCode);
		while (m_pTokenPointer->m_nTokenId)
		{
			Statement();
			m_sCodeSaveJS += " ; ";
		}
		m_sCodeSaveJS = XuLyChuoiTinh(m_sCodeSaveJS);
	}
	catch (CRuntimeException *pRE)
	{
		pREHandleProcess = pRE;
	}

	
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

	// tao con tro lenh moi + stack moi -> thuc hien code moi
	// init new m_TokenPointer for new CodeData
	m_pTokenPointer = new CTokenPointer(sCode);

	// Create New Stack
	m_lstStack.clear();
	m_lstStack.push_back(m_pVRootStack);

	// Doc tat ca dinh nghia ham trong code
	GetAllFuncDefineInCode(sCode);
	try {
		do
		{
			pVLTmp = Base();

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
	int iParamCount = 0;
	CRuntimeException* pREHandleProcess = NULL;
	if (m_pTokenPointer == NULL)
		return;
	try 
	{
		m_pTokenPointer->Match('(');
		m_sCodeSaveJS += " ( ";
		while (m_pTokenPointer->m_nTokenId != ')')
		{
			pVLArg = NULL;
			if (pVLstArgument)
				pVLArg = pVLstArgument->AddChildNoDup(m_pTokenPointer->m_sTokenStr, NULL, "param" + to_string(iParamCount), TRUE);
			iParamCount += 1;
			m_sCodeSaveJS += " " + pVLArg->m_sAliasName + " ";
			if (pVLArg)
				if (!bIsReDefine)
					pVLArg->SetNotReDefine();	// Cai dat cac ham API mac dinh

			m_pTokenPointer->Match(TK_ID);
			// Nhieu tham so duoc khai bao
			if (m_pTokenPointer->m_nTokenId != ')')
			{
				m_pTokenPointer->Match(',');
				m_sCodeSaveJS += " , ";
			}
		}
		m_pTokenPointer->Match(')');
		m_sCodeSaveJS += " ) ";
	}
	catch (CRuntimeException *pRE)
	{
		// hien thi loi
		if (pRE)
			OutputDebug(string("Error ") + pRE->m_sMessage + " at " + m_pTokenPointer->GetPosition());
		pREHandleProcess = pRE;
	}
	SAFE_THROW(pREHandleProcess);
}

//------------------------------------------------------------------------------
// Lay danh sach phan tu con luu vao pVList
//------------------------------------------------------------------------------
void CProgramJs::GetListChildArray(CVar *pVList)
{
	int nIndex = 0;
	char idx_str[16]; // big enough for 2^32
	CVarLink *pVLChild = NULL;

	if (m_pTokenPointer == NULL || m_pTokenPointer->m_nTokenId != '(')
		return;

	m_pTokenPointer->Match('(');
	m_sCodeSaveJS += " ( ";
	m_iCountArrayElement = 0;
	while (m_pTokenPointer->m_nTokenId != ')')
	{
		_snprintf_s(idx_str, sizeof(idx_str),_TRUNCATE, "%d", nIndex);
		pVLChild = Base();
		if (pVLChild)
			pVList->AddChild(idx_str, pVLChild->m_pVar);

		// no need to clean here, as it will definitely be used
		if (m_pTokenPointer->m_nTokenId != ')')
		{
			m_pTokenPointer->Match(',');
			m_iCountArrayElement += 1;
		}
			
		nIndex++;
	}
	m_sCodeSaveJS += " : ";
	m_sCodeSaveJS += to_string(m_iCountArrayElement);
	m_iCountArrayElement = 0;
	m_pTokenPointer->Match(')');
	m_sCodeSaveJS += " ) ";
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
			pVLLink = pVBase->AddChild(sFuncName, new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT), "", TRUE);
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
		(pVBase->AddChildNoDup(sFuncName, pVFunc, "", TRUE))->SetNotReDefine();
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
					pVLNewAttr = pVBase->AddChild(m_pTokenPointer->m_sTokenStr, pVTmpValue, "", TRUE);
					if (pVLNewAttr)
						pVLNewAttr->SetNotReDefine();
				}
				else if (!pVLNewAttr && m_pTokenPointer->TryWatchNextToken() == 0){
					// Them thuoc tinh cuoi cung
					pVLNewAttr = pVBase->AddChild(m_pTokenPointer->m_sTokenStr, pVAttr, "", TRUE);
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
		m_pTokenPointer->Match(TK_R_FUNCTION);
		// save ten ham vao save codejs
		sFuncName = STR_TEMP_NAME;						
														
		/* we can have functions without names */		
		if (m_pTokenPointer->m_nTokenId == TK_ID)		
		{												
			sFuncName = m_pTokenPointer->m_sTokenStr;	

			m_pTokenPointer->Match(TK_ID);	
		}												
		if (sFuncName == STR_TEMP_NAME)
			pVLFunc = new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION), sFuncName, "noname"+ to_string(m_nameFuncCount));
		else
			pVLFunc = new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION), sFuncName, "func" + to_string(m_nameFuncCount));
		m_sCodeSaveJS += " " + pVLFunc->m_sAliasName + " ";

		m_nameFuncCount += 1;

		// get Function Arguments
		ParseFunctionArguments(pVLFunc->m_pVar);
		// luu string szCode dinh nghia Function 
		// lam du lieu cho thuoc tinh arguments.callee = "function tenHam() {...}"
		sDefineFunc = m_pTokenPointer->GetSubString(nDefineFuncBegin);
		nSize = sDefineFunc.size();
	}
	catch (CRuntimeException* pRE){
		pREHandleProcess = pRE;
	}

	SAFE_THROW(pREHandleProcess);
	return pVLFunc;

}


CVarLink *CProgramJs::RunFunction(CVarLink *pVLFunc, CVar *pVParent, CVar* pVArgumentsValue)
{
	BOOLEAN bIsPushedLocalStack = FALSE;
	CVar *pVStackLocalFunction = NULL;
	CVarLink *pVLFirstArgument = NULL;
	CVarLink *pVLArgValue = NULL;

	CVarLink *pVLReturn = new CVarLink(new CVar()); // cai dat bien tra ve 
	CVarLink *pVLFuncReturn = NULL;

	CTokenPointer *pTokenPointerOld = NULL;
	CRuntimeException *pRuntimeException = NULL;
	try
	{
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
			pTokenPointerOld = m_pTokenPointer;
			m_pTokenPointer = new CTokenPointer(pVLFunc->m_pVar->GetString());
			try
			{
				GetAllFuncDefineInCode(m_pTokenPointer->GetCode());
			}
			catch (CRuntimeException *pRE)
			{
				SAFE_DELETE(pRE);
			}
			// thuc hien code than ham
			BlockCode();
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
// Muc dich:	Thêm 1 doi tuong Function duoc cai dat CallBack theo thoi gian vào 
//				danh sách.
// ----------------
// Vào:			Con tro du lieu doi tuong function
//------------------------------------------------------------------------------
void CProgramJs::AddFuncCallBackBySetInterval(CVar* pVarFunction)
{
	if (CheckFuncCallBackBySetInterval(pVarFunction) == FALSE && pVarFunction)
		m_lstFuncCallBackBySetInterval.push_back(pVarFunction);
}

//------------------------------------------------------------------------------
// M?c dích: Ki?m tra 1 d?i tu?ng Function dã có trong DS Function Callback.
// ----------------
// Vào:	Con tr? d? li?u d?i tu?ng function c?n ki?m tra.
// Ra:	- True (n?u có) / False (n?u ko)
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
CVarLink *CProgramJs::FunctionCall(CVarLink *pVLFunc, CVar *pVParent)
{
	BOOLEAN bFunctionNDC = FALSE;
	CVar *pVStackLocalFunction = NULL;
	CVarLink *pVLFirstArgument = NULL;
	CVarLink *pVLArgValue = NULL;

	CVarLink *pVLReturn = NULL; // cai dat bien tra ve 
	CVarLink *pVLFuncReturn = NULL;

	CRuntimeException *pRuntimeException = NULL;

	CTokenPointer *pTokenPointerOld = NULL;
	try
	{
		if (!pVLFunc || !pVLFunc->m_pVar || !pVLFunc->m_pVar->IsFunction())
		{

			m_pTokenPointer->Match('('); // [NDC]
			do
			{
				// neu khai bao co kieu var a=(b+c,e+d) thi tra ve a=e+d 
				if (m_pTokenPointer->m_nTokenId == ',')
					m_pTokenPointer->Match(',');
				if (m_pTokenPointer->m_nTokenId == ')')
					break;
				Base();
			} while (m_pTokenPointer->m_nTokenId == ',');

			m_pTokenPointer->Match(')');
			m_sCodeSaveJS += " ) ";
			bFunctionNDC = TRUE;
			pVLReturn = new CVarLink(new CVar());
		}
		if (!bFunctionNDC)
		{
			// Khoi tao New Stack cho ham - luu cac bien cuc bo su dung trong ham
			pVStackLocalFunction = new CVar(STR_BLANK_DATA, SCRIPTVAR_FUNCTION);
			if (pVParent)
				pVStackLocalFunction->AddChildNoDup("this", pVParent);
			if (pVLFunc && pVLFunc->m_pVar)
				pVLFirstArgument = pVLFunc->m_pVar->m_pVLFirstChild;
			if (pVLFirstArgument && pVLFirstArgument->m_sName == "ARRAY")
			{
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
				while (pVLFirstArgument && m_pTokenPointer->m_nTokenId != ')')
				{
					// Thuc hien thao tac voi cac phan  tu trong ngoac
					Base();

					if (m_pTokenPointer->m_nTokenId == ')') break;
					else if ((m_pTokenPointer->m_nTokenId == ',') && (pVLFirstArgument->m_pNextSibling == 0))
					{
						m_sCodeSaveJS += " , ";
						m_pTokenPointer->Match(',');
						while (m_pTokenPointer->m_nTokenId == TK_INT
							|| m_pTokenPointer->m_nTokenId == ',')
						{
							if (m_pTokenPointer->m_nTokenId == TK_INT) m_sCodeSaveJS += " int ";
							if (m_pTokenPointer->m_nTokenId == ',') m_sCodeSaveJS += " , ";
							m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
						}
						if (m_pTokenPointer->m_nTokenId == ')') break;
					}
					else
					{
						m_sCodeSaveJS += " , ";
						m_pTokenPointer->Match(',');
					}
					pVLFirstArgument = pVLFirstArgument->m_pNextSibling;
				}
				m_sCodeSaveJS += " ) ";
				m_pTokenPointer->Match(')');
			}
			pTokenPointerOld = m_pTokenPointer;
			if (pVLFunc && pVLFunc->m_pVar)
				m_pTokenPointer = new CTokenPointer(pVLFunc->m_pVar->GetString());
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
	if (pTokenPointerOld)
	{
		m_pTokenPointer = pTokenPointerOld;
	}
	SAFE_THROW(pRuntimeException);
	return pVLReturn;
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
CVarLink *CProgramJs::FactorAdvance(CVarLink* pVLObjectCall, CVar* pVParent)
{
	size_t iString = 0;
	bool bStringInt = false;
	string sNameChild;
	string sDataString;
	string sString;
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
			|| m_pTokenPointer->m_nTokenId == '['
			|| m_pTokenPointer->m_nTokenId == ':')
		{
			if (m_pTokenPointer->m_nTokenId == '(')
			{
				m_sCodeSaveJS += " ( ";
				pVLObjectCall = FunctionCall(pVLObjectCall, pVParent);
				pVLObjectBeginCall = pVLObjectCall;
			}
			else if (m_pTokenPointer->m_nTokenId == '.')
			{
				m_sCodeSaveJS += " . ";
				// xu ly: ID.SubId ... 
				// tinh toan voi cac "thuoc tinh || phuong thuc" con cua bien ID
				m_pTokenPointer->Match('.');
				sNameChild = m_pTokenPointer->m_sTokenStr;
				if (pVLObjectCall) // && pVLObjectCall->m_pVar
					pVLChild = pVLObjectCall->m_pVar->FindChild(sNameChild);
				if (!pVLChild)
				{
					// tim trong prototype cua doi tuong
					if (pVLObjectCall)
						pVLChildInPrototype = FindInParentClasses(pVLObjectCall->m_pVar, sNameChild);

					if (pVLChildInPrototype && pVLChildInPrototype->m_pVar)
					{
						if (!pVLChildInPrototype->m_pVar->IsFunction())
						{
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
						m_sCodeSaveJS += " length ";
						pVLChild = new CVarLink(new CVar(1));
					}
					else if (pVLObjectCall && pVLObjectCall->m_pVar && pVLObjectCall->m_pVar->IsString() && sNameChild == "length")
					{
						m_sCodeSaveJS += " length ";
						pVLChild = new CVarLink(new CVar(1));

					}
					else
					{
						m_sCodeSaveJS += " " + sNameChild + " ";

					}
				}

				// duyet den phan tu con
				/*pVParent = pVLObjectCall->m_pVar;
				pVLObjectCall = pVLChild;*/
				else if (pVLChild)
				{
					m_sCodeSaveJS += " " + pVLChild->m_sName + " ";
				}
				m_pTokenPointer->Match(TK_ID);
			}
			else if (m_pTokenPointer->m_nTokenId == '[')
			{
				m_pTokenPointer->Match('[');
				m_sCodeSaveJS += " [ ";
				pVLIndexOfArray = Base();
				/*if (!pVLIndexOfArray || !pVLIndexOfArray->m_pVar || pVLIndexOfArray->m_pVar->IsUndefined())
					throw new CRuntimeException("Loi:: Mang[ViTri] khong duoc dinh nghia");*/
				m_pTokenPointer->Match(']');
				m_sCodeSaveJS += " ] ";
				if (pVLObjectCall && pVLObjectCall->m_pVar)
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
							pVLChildInArray = new CVarLink(new CVar(1));
						}
						else if (bStringInt && (pVLObjectCall->m_pVar->IsString()) && (iString < (pVLObjectCall->m_pVar->GetString().size())))
						{
							sDataString = pVLObjectCall->m_pVar->m_sData;
							sString = sDataString.at(iString);
							pVTmpValueString = new CVar(sString, SCRIPTVAR_STRING);
							pVLArraytoString = new CVarLink(pVTmpValueString);
							pVLChildInArray = pVLArraytoString;
						}
						else if (pVLObjectCall->m_pVar->IsString() && sNameChild == "length")
						{
							pVLChildInArray = new CVarLink(new CVar(1));
						}
						else
						{
							pVLChildInArray = pVLObjectCall->m_pVar->FindChildOrCreate(sNameChild);
						}
						/*pVParent = pVLObjectCall->m_pVar;
						pVLObjectCall = pVLChildInArray;*/
				}
			}
			else if (m_pTokenPointer->m_nTokenId == ':')
			{
				// Bo sung them khai bao cau truc file json
				m_sCodeSaveJS += " : ";
				break;
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
CVarLink *CProgramJs::Factor()
{
	int nTypeTk = 0;
	int nIndexInArray = 0;
	string sTokenPointerId;
	string sNameChildThis = "";
	CVarLink *pVLReturn = NULL;
	CVarLink *pVLIndexOfArrayThis = NULL;
	CVarLink *pVLObjClassOrFunc = NULL;
	CVarLink *pVLFuncReturn = NULL;
	CVarLink *pVLNewObj = NULL;
	CVarLink *pVLConstructor = NULL;
	CVarLink* pVLValueReturn = NULL;
	CVarLink* pVLTmpValue = NULL;
	CVar*	pVTmpValue = NULL;
	CVar*	pVParent = NULL;

	CRuntimeException* pREHandleProcess = NULL;

	if (m_pTokenPointer == NULL)
		return NULL;
	try
	{
		if (m_pTokenPointer->m_nTokenId == '(')
		{
			m_pTokenPointer->Match('('); // [NDC]
			do
			{
				// neu khai bao co kieu var a=(b+c,e+d) thi tra ve a=e+d 
				if (m_pTokenPointer->m_nTokenId == ',')
				{
					m_pTokenPointer->Match(',');
					m_sCodeSaveJS += " , ";
				}

				pVLValueReturn = Base();
			} while (m_pTokenPointer->m_nTokenId == ',');

			m_pTokenPointer->Match(')');
			pVLValueReturn = FactorAdvance(pVLValueReturn, NULL);
			return pVLValueReturn;
		}
		if (m_pTokenPointer->m_nTokenId == TK_R_TRUE)
		{
			if (m_iCountArrayElement == 0)
			{
				m_sCodeSaveJS += " true ";
			}
			m_pTokenPointer->Match(TK_R_TRUE);
			return new CVarLink(new CVar(1));
		}

		if (m_pTokenPointer->m_nTokenId == TK_R_FALSE)
		{
			if (m_iCountArrayElement == 0)
			{
				m_sCodeSaveJS += " false ";
			}
			m_pTokenPointer->Match(TK_R_FALSE);
			return new CVarLink(new CVar(0));
		}

		if (m_pTokenPointer->m_nTokenId == TK_R_NULL)
		{
			if (m_iCountArrayElement == 0)
			{
				m_sCodeSaveJS += " null ";
			}
			m_pTokenPointer->Match(TK_R_NULL);
			return NULL;
		}

		if (m_pTokenPointer->m_nTokenId == TK_R_UNDEFINED)
		{
			// Khong ghi ra file neu phan tu nay trong mang
			if (m_iCountArrayElement == 0)
			{
				m_sCodeSaveJS += " undefined ";
			}
			m_pTokenPointer->Match(TK_R_UNDEFINED);
			return new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_NULL));
		}

		// type token = ID : bien trong stack-----------------------------------
		if (m_pTokenPointer->m_nTokenId == TK_ID)
		{
			if (m_pTokenPointer->m_sTokenStr == "this" && m_pTokenPointer->m_currCh == '[')
			{
				// this['function']['...']()
				m_sCodeSaveJS += " this ";
				m_pTokenPointer->Match(TK_ID);
				m_pTokenPointer->Match('[');
				m_sCodeSaveJS += " [ ";
				pVLIndexOfArrayThis = Base();
				if (pVLIndexOfArrayThis && pVLIndexOfArrayThis->m_pVar)
					sNameChildThis = pVLIndexOfArrayThis->m_pVar->GetString();
				if (sNameChildThis != "")
					pVLValueReturn = FindInScopes(sNameChildThis);
				if (!pVLValueReturn)
				{
					pVLValueReturn = new CVarLink(new CVar(), sNameChildThis);
					m_sCodeSaveJS += " " + sNameChildThis + " ";
				}
				if (m_pTokenPointer->m_nTokenId == ']')
				{
					m_sCodeSaveJS += " ] ";
					m_pTokenPointer->Match(']');
				}
				else
				{
					if (m_pTokenPointer->m_nTokenId >= 32 && m_pTokenPointer->m_nTokenId <= 126)
					{
						string temp(1, m_pTokenPointer->m_nTokenId);
						m_sCodeSaveJS += " " + temp + " ";
					}
					m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
				}


				pVLValueReturn = FactorAdvance(pVLValueReturn, pVParent);
			}
			else if (m_pTokenPointer->m_sTokenStr == "self")
			{
				m_sCodeSaveJS += " self ";
				m_pTokenPointer->Match(TK_ID);
				while (m_pTokenPointer->m_nTokenId == '.')
				{
					m_pTokenPointer->Match('.');
					m_sCodeSaveJS += " . ";
					if (pVLTmpValue && pVLTmpValue->m_pVar)
					{
						pVLTmpValue = pVLTmpValue->m_pVar->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, "");
						m_sCodeSaveJS += " " + pVLTmpValue->m_sName + " ";
					}
					m_pTokenPointer->Match(TK_ID);
				}
				// sort out initialiser
				if (m_pTokenPointer->m_nTokenId == '=')
				{
					// Xu ly gia tri cua bien sau dau =
					m_sCodeSaveJS += " = ";
					m_pTokenPointer->Match('=');
					Base();
				}
			}
			else if (m_pTokenPointer->m_sTokenStr == "length")
			{
				m_sCodeSaveJS += " length ";
				m_pTokenPointer->Match(TK_ID);
			}

			else
			{
				pVLValueReturn = FindInScopes(m_pTokenPointer->m_sTokenStr);
				if (!pVLValueReturn)
				{
					CVarLink* pVLValueInit;
					// Lenh for (id = ..; ..; ..) { }
					while (m_pTokenPointer->m_nTokenId != ';')
					{
						if (m_lstStack.size() > 0)
						{
							string aliasName = "";
							if (m_inFunc[indexInFunc] == false) aliasName = "globalVar" + to_string(m_nameVarCount);
							else aliasName = "funcVar" + to_string(m_nameVarInFuncCount);
							pVLTmpValue = m_lstStack.back()->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, aliasName);
							if (pVLTmpValue->m_sAliasName == aliasName)
								if (m_inFunc[indexInFunc] == false) m_nameVarCount += 1;
								else m_nameVarInFuncCount += 1;
								m_sCodeSaveJS += " " + pVLTmpValue->m_sAliasName + " ";
						}

						m_pTokenPointer->Match(TK_ID);
						// id.write, ... id.string.charCodeAt -> just example
						while (m_pTokenPointer->m_nTokenId == '.')
						{
							m_pTokenPointer->Match('.');
							m_sCodeSaveJS += " . ";
							if (pVLTmpValue && pVLTmpValue->m_pVar)
							{
								pVLTmpValue = pVLTmpValue->m_pVar->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, "");
								m_sCodeSaveJS += " " + pVLTmpValue->m_sName + " ";
							}
							m_pTokenPointer->Match(TK_ID);
							if (m_pTokenPointer->m_nTokenId == '(')
							{
								FactorAdvance(pVLValueReturn, pVParent);
							}
						}
						if (m_pTokenPointer->m_nTokenId == '=')
						{
							// Xu ly gia tri cua bien sau dau =
							m_sCodeSaveJS += " = ";
							m_pTokenPointer->Match('=');
							pVLValueInit = Base();
						}
						// Khai bao nhieu bien lien tiep
						if (m_pTokenPointer->m_nTokenId == ',')
						{
							m_pTokenPointer->Match(',');
							m_sCodeSaveJS += " , ";
							Factor();
							break;
						}
						else break;
					}
				}
				else
				{
					if (pVLValueReturn->m_sAliasName == "")
						m_sCodeSaveJS += " " + pVLValueReturn->m_sName + " ";
					else
						m_sCodeSaveJS += " " + pVLValueReturn->m_sAliasName + " ";

					m_pTokenPointer->Match(TK_ID);
					if (pVLValueReturn->m_sName != "Array")
						pVLValueReturn = FactorAdvance(pVLValueReturn, pVParent);
					else {
						m_sCodeSaveJS += " Array() ";
					}
				}
			}
			return pVLValueReturn;
		}
		if (m_pTokenPointer->m_nTokenId == TK_INT || m_pTokenPointer->m_nTokenId == TK_FLOAT)
		{
			// Check neu o trong mang thi khong ghi ra file text ma dem
			if (m_iCountArrayElement == 0)
			{
				if (m_pTokenPointer->m_nTokenId == TK_INT) m_sCodeSaveJS += " int ";
				else m_sCodeSaveJS += " float ";
			}

			pVTmpValue = new CVar(m_pTokenPointer->m_sTokenStr,
				((m_pTokenPointer->m_nTokenId == TK_INT) ? SCRIPTVAR_INTEGER : SCRIPTVAR_DOUBLE));
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

			return new CVarLink(pVTmpValue);
		}
		if (m_pTokenPointer->m_nTokenId == TK_STR)
		{
			pVTmpValue = new CVar(m_pTokenPointer->m_sTokenStr, SCRIPTVAR_STRING);
			// Lay thong tin chuoi xau va so sanh xau voi cac ham trong list, neu trung thi giu nguyen
			pVLValueReturn = FindInScopesAdvance(m_pTokenPointer->m_sTokenStr);

			if (m_iCountArrayElement == 0)
			{
				if (pVLValueReturn)
				{
					m_sCodeSaveJS += " " + pVLValueReturn->m_sName + " ";
				}
				else
				{
					m_sCodeSaveJS += " \"string\" ";
				}
			}

			m_pTokenPointer->Match(TK_STR);

			pVLValueReturn = FactorAdvance(pVLValueReturn, NULL);
			return pVLValueReturn;
		}
		if (m_pTokenPointer->m_nTokenId == TK_REGEX)
		{
			m_sCodeSaveJS += " regex ";
			pVTmpValue = new CVar(m_pTokenPointer->m_sTokenStr, SCRIPTVAR_REGEX);
			pVLValueReturn = new CVarLink(pVTmpValue);
			m_pTokenPointer->Match(TK_REGEX);
			pVLValueReturn = FactorAdvance(pVLValueReturn, NULL);
			return pVLValueReturn;
		}
		// type token = bat dau khoi lenh --------------------------------------
		if (m_pTokenPointer->m_nTokenId == '{')
		{
			pVParent = new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT);
			m_pTokenPointer->Match('{');
			m_sCodeSaveJS += " { ";
			while (m_pTokenPointer->m_nTokenId != '}')
			{
				sTokenPointerId = m_pTokenPointer->m_sTokenStr;
				// we only allow strings or IDs on the left hand side of an initialisation
				if (m_pTokenPointer->m_nTokenId == TK_REGEX)
				{
					m_pTokenPointer->Match(TK_REGEX);
					m_sCodeSaveJS += " regex ";
				}

				else if (m_pTokenPointer->m_nTokenId == TK_STR)
				{
					m_pTokenPointer->Match(TK_STR);
					m_sCodeSaveJS += " \"string\" ";
				}

				else
					m_pTokenPointer->Match(TK_ID);
				m_pTokenPointer->Match(':');
				m_sCodeSaveJS += " : ";
				Base();
				// pVParent->AddChild(sTokenPointerId, pVLTmpValue->m_pVar, pVLTmpValue->m_sAliasName);
				if (m_pTokenPointer->m_nTokenId != '}')
				{
					m_pTokenPointer->Match(',');
					m_sCodeSaveJS += " , ";
				}

			}
			m_pTokenPointer->Match('}');
			m_sCodeSaveJS += " } ";
			// Neu truy xuat den ngay phan tu con cua doi tuong
			pVLValueReturn = new CVarLink(pVParent);
			pVLValueReturn = FactorAdvance(pVLValueReturn, NULL);

			return pVLValueReturn;
		}

		// type token = bat dau noi dung mang ----------------------------------
		if (m_pTokenPointer->m_nTokenId == '[')
		{
			pVParent = new CVar(STR_BLANK_DATA, SCRIPTVAR_ARRAY);
			nIndexInArray = 0;
			/* JSON-style array */
			m_pTokenPointer->Match('[');
			m_sCodeSaveJS += " [ ";
			m_iCountArrayElement = 0;
			while (m_pTokenPointer->m_nTokenId != ']')
			{
				Base();
				// no need to clean here, as it will definitely be used
				if (m_pTokenPointer->m_nTokenId != ']')
				{
					m_pTokenPointer->Match(',');
					//m_sCodeSaveJS += " , ";
					m_iCountArrayElement += 1;
				}

				nIndexInArray++;
			}
			m_sCodeSaveJS += " : ";
			m_sCodeSaveJS += to_string(m_iCountArrayElement);
			m_pTokenPointer->Match(']');
			m_sCodeSaveJS += " ] ";
			m_iCountArrayElement = 0;
			pVLValueReturn = new CVarLink(pVParent);
			pVLValueReturn = FactorAdvance(pVLValueReturn, pVParent);
			return pVLValueReturn;
		}

		//  dinh nghia ham moi -------------------------------------------------
		if (m_pTokenPointer->m_nTokenId == TK_R_FUNCTION)
		{
			m_sCodeSaveJS += " function ";
			indexInFunc += 1;
			m_inFunc[indexInFunc] = true;
			pVLTmpValue = ParseFunctionDefinition();
			if (!pVLTmpValue)
				return pVLTmpValue;
			BlockCode();
			indexInFunc -= 1;
			m_inFunc[indexInFunc] = false;

			//chay kieu ham function a() {...}()  ...[NDC]
			if ((m_pTokenPointer->m_nTokenId == '('))
			{
				pVLValueReturn = FindInScopes(pVLTmpValue->m_sName);
				if (pVLValueReturn)
					pVLValueReturn = FactorAdvance(pVLValueReturn, pVParent);
				else
					pVLValueReturn = FactorAdvance(pVLTmpValue, pVParent);
				return pVLValueReturn;
			}
			if (pVLTmpValue->m_sName != STR_TEMP_NAME)
				OutputDebug("Functions not defined at statement-level are not meant to have a name");
			return pVLTmpValue;
		}

		// xin cap phat doi tuong ----------------------------------------------
		if (m_pTokenPointer->m_nTokenId == TK_R_NEW)
		{
			// save "new" keyword to code
			m_sCodeSaveJS += " new ";

			pVLReturn = new CVarLink(new CVar());

			// new -> create a new object
			m_pTokenPointer->Match(TK_R_NEW);
			sTokenPointerId = m_pTokenPointer->m_sTokenStr;

			pVLObjClassOrFunc = FindInScopes(sTokenPointerId);
			if (!pVLObjClassOrFunc)
			{
				m_sCodeSaveJS += " _UO "; // Undefined Object
				m_pTokenPointer->Match(TK_ID);
				m_pTokenPointer->Match('(');
				m_sCodeSaveJS += " ( ";
				Base();
				// co the xay ra loi
				m_pTokenPointer->Match(')');
				m_sCodeSaveJS += " ) ";
				if (m_pTokenPointer->m_nTokenId == '.' ||
					m_pTokenPointer->m_nTokenId == '[' ||
					m_pTokenPointer->m_nTokenId == '(')
					FactorAdvance(pVLValueReturn, pVParent);
				goto _COMPLETE;
			}
			else
				m_sCodeSaveJS += " " + pVLObjClassOrFunc->m_sName + " ";
			
			m_pTokenPointer->Match(TK_ID);
			pVLNewObj = new CVarLink(new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT));
			// chinh sua lai code de chi lay thong tin doi tuong va cac tham so
			if (pVLObjClassOrFunc && pVLObjClassOrFunc->m_pVar && pVLObjClassOrFunc->m_pVar->IsFunction() && pVLNewObj && pVLNewObj->m_pVar)
			{
				pVLFuncReturn = FunctionCall(pVLObjClassOrFunc, pVLNewObj->m_pVar);
				//pVLNewObj->ReplaceWith(pVLFuncReturn);
			}
			else if (pVLObjClassOrFunc && pVLObjClassOrFunc->m_pVar)
			{
				// co ham khoi tao constructor rieng
				// xx check prototype function ??? 
				pVLConstructor = pVLObjClassOrFunc->m_pVar->FindChild("constructor");
				nTypeTk = this->m_pTokenPointer->TryWatchNextToken();
				if (nTypeTk != ')' && pVLConstructor)
				{
					pVLFuncReturn = FunctionCall(pVLConstructor, pVLNewObj->m_pVar);
					pVLNewObj->ReplaceWith(pVLFuncReturn);
				}
				else
				{
					// la doi tuong chi co constructor mac dinh
					if (pVLNewObj->m_pVar)
						pVLNewObj->m_pVar->AddChild(STR_PROTOTYPE_CLASS, pVLObjClassOrFunc->m_pVar);
					if (m_pTokenPointer->m_nTokenId == '(')
					{
						m_pTokenPointer->Match('(');
						m_pTokenPointer->Match(')');
					}
				}
			}
			goto _COMPLETE;
		}
		else
		{
			if (m_pTokenPointer->m_nTokenId == TK_EOF)
				m_pTokenPointer->Match(TK_EOF);
			else
			{
				if (m_pTokenPointer->m_nTokenId >= 32 && m_pTokenPointer->m_nTokenId <= 126) // la ky tu doc duoc thi moi ghi lai
				{
					string temp(1, m_pTokenPointer->m_nTokenId);
					m_sCodeSaveJS += " " + temp + " ";
				}
				m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			}
		}
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
CVarLink *CProgramJs::Unary() {
	int nTPId = 0;

	CVarLink *pVLZero = NULL;
	CVarLink *pVLOne = NULL;
	CVarLink *pVLRes = NULL;

	CVarLink *pVLOperandLeft = NULL;
	CRuntimeException* pREHandleProcess = NULL;
	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());
	try
	{
		if (m_pTokenPointer->m_nTokenId == '!' ||
			m_pTokenPointer->m_nTokenId == '~' ||
			m_pTokenPointer->m_nTokenId == TK_PLUSPLUS ||
			m_pTokenPointer->m_nTokenId == TK_MINUSMINUS)
		{
			if (m_pTokenPointer->m_nTokenId == '!') m_sCodeSaveJS += " ! ";
			if (m_pTokenPointer->m_nTokenId == '~') m_sCodeSaveJS += " ~ ";
			if (m_pTokenPointer->m_nTokenId == TK_PLUSPLUS) m_sCodeSaveJS += " ++ ";
			if (m_pTokenPointer->m_nTokenId == TK_MINUSMINUS) m_sCodeSaveJS += " -- ";
			pVLRes = new CVarLink(new CVar());
			pVLOne = new CVarLink(new CVar(1));
			pVLZero = new CVarLink(new CVar(0));
			nTPId = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			pVLOperandLeft = Factor();
		}
		else if (m_pTokenPointer->m_nTokenId == TK_R_TYPEOF)
		{
			m_sCodeSaveJS += " typeof ";
			m_pTokenPointer->Match(TK_R_TYPEOF);
			pVLOperandLeft = Factor();

		}
		else
			pVLOperandLeft = Factor();
	}
	catch (CRuntimeException *pRE)
	{
		pREHandleProcess = pRE;
	}
	SAFE_THROW(pREHandleProcess);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// Xu ly toan tu: *, /, %
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Term()
{
	int nOp = 0;
	CVarLink* pVLTmpValue = NULL;
	CVarLink* pVLOperandRight = NULL;
	CVarLink* pVLOperandLeft = Unary();
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
			if (nOp == '*') m_sCodeSaveJS += " * ";
			else if (nOp == '/') m_sCodeSaveJS += " / ";
			else if (nOp == '%') m_sCodeSaveJS += " % ";

			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			pVLOperandRight = Unary();
		}
	}
	catch (CRuntimeException * pRE)
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
CVarLink *CProgramJs::Expression()
{
	bool bNegate = false;
	int nOp = 0;
	CVarLink* pVLOne = NULL;
	CVarLink* pVLZero = NULL;
	CVarLink* pVLRes = NULL;
	CVarLink* pVLOperandLeft = NULL;
	CRuntimeException* pRuntimeException = NULL;
	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());
	try
	{
		// fix:: them trycatch giai phong vung nho khi xay ra exception
		pVLZero = new CVarLink(new CVar(0));
		pVLOne = new CVarLink(new CVar(1));
		pVLRes = new CVarLink(new CVar());

		// fix:: them xu ly dau '+/-' cho kieu du lieu 
		if (m_pTokenPointer->m_nTokenId == '-' || m_pTokenPointer->m_nTokenId == '+')
		{
			if (m_pTokenPointer->m_nTokenId == '-')
			{
				bNegate = true;
				m_sCodeSaveJS += " - ";
			}
			else {
				m_sCodeSaveJS += " + ";
			}

			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

		}

		pVLOperandLeft = Term();

		while (m_pTokenPointer->m_nTokenId == '+'
			|| m_pTokenPointer->m_nTokenId == '-'
			|| m_pTokenPointer->m_nTokenId == TK_PLUSPLUS
			|| m_pTokenPointer->m_nTokenId == TK_MINUSMINUS)
		{
			nOp = m_pTokenPointer->m_nTokenId;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

			if (nOp == TK_PLUSPLUS || nOp == TK_MINUSMINUS) {
				if (nOp == TK_PLUSPLUS) m_sCodeSaveJS += " ++ ";
				if (nOp == TK_MINUSMINUS) m_sCodeSaveJS += " -- ";
			}

			else
			{
				if (nOp == '+')
					m_sCodeSaveJS += " + ";
				else m_sCodeSaveJS += " - ";
				Term();
			}
		}

	}
	catch (CRuntimeException* pRE)
	{
		pRuntimeException = pRE;
	}
	SAFE_THROW(pRuntimeException);
	return pVLOperandLeft;
}


//------------------------------------------------------------------------------
// toan tu: expression
// Xu ly toan tu dich: <<, >>, >>>
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Shift()
{
	CVarLink *pVLOperandRight = NULL;
	CVarLink *pVLOperandLeft = Expression();
	CRuntimeException* pRuntimeException = NULL;
	try
	{
		if (m_pTokenPointer == NULL)
			return new CVarLink(new CVar());
		if (m_pTokenPointer->m_nTokenId == TK_LSHIFT ||
			m_pTokenPointer->m_nTokenId == TK_RSHIFT ||
			m_pTokenPointer->m_nTokenId == TK_RSHIFTUNSIGNED)
		{
			if (m_pTokenPointer->m_nTokenId == TK_LSHIFT) m_sCodeSaveJS += " << ";
			if (m_pTokenPointer->m_nTokenId == TK_RSHIFT) m_sCodeSaveJS += " >> ";
			if (m_pTokenPointer->m_nTokenId == TK_RSHIFTUNSIGNED) m_sCodeSaveJS += " >>> ";

			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			pVLOperandRight = Expression();

		}
	} 
	catch (CRuntimeException* pRE)
	{
		pRuntimeException = pRE;
	}
	SAFE_THROW(pRuntimeException);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// toan tu: Condition
// Xu ly toan dieu kien: ==, !=, ===, !==, <=, >=, <, >
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Condition()
{
	CVarLink *pVLOperandLeft = Shift();

	CRuntimeException* pRuntimeException = NULL;

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
			if (m_pTokenPointer->m_nTokenId == TK_EQUAL) m_sCodeSaveJS += " == ";
			if (m_pTokenPointer->m_nTokenId == TK_NEQUAL) m_sCodeSaveJS += " != ";
			if (m_pTokenPointer->m_nTokenId == TK_TYPEEQUAL) m_sCodeSaveJS += " === ";
			if (m_pTokenPointer->m_nTokenId == TK_NTYPEEQUAL) m_sCodeSaveJS += " !== ";
			if (m_pTokenPointer->m_nTokenId == TK_LEQUAL) m_sCodeSaveJS += " <= ";
			if (m_pTokenPointer->m_nTokenId == TK_GEQUAL) m_sCodeSaveJS += " >= ";
			if (m_pTokenPointer->m_nTokenId == '<') m_sCodeSaveJS += " < ";
			if (m_pTokenPointer->m_nTokenId == '>') m_sCodeSaveJS += " > ";

			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			Shift();
		}
	}
	catch (CRuntimeException* pRE)
	{
		pRuntimeException = pRE;
	}
	SAFE_THROW(pRuntimeException);
	return pVLOperandLeft;
}

//------------------------------------------------------------------------------
// toan tu: Logic - phep toan tren bit
// Xu ly toan dieu kien: &, |, ^, &&, ||
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Logic()
{
	int nOp = 0;
	CVarLink *pVLOperandLeft = NULL;
	CRuntimeException* pREHandleProcess = NULL;
	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());
	try
	{
		Condition();
		while (m_pTokenPointer->m_nTokenId == '&'
			|| m_pTokenPointer->m_nTokenId == '|'
			|| m_pTokenPointer->m_nTokenId == '^'
			|| m_pTokenPointer->m_nTokenId == TK_ANDAND
			|| m_pTokenPointer->m_nTokenId == TK_OROR)
			//|| m_pTokenPointer->m_nTokenId == ',')
		{
			nOp = m_pTokenPointer->m_nTokenId;
			if (nOp == '&') m_sCodeSaveJS += " & ";
			if (nOp == '|') m_sCodeSaveJS += " | ";
			if (nOp == '^') m_sCodeSaveJS += " ^ ";
			if (nOp == TK_ANDAND) m_sCodeSaveJS += " && ";
			if (nOp == TK_OROR) m_sCodeSaveJS += " || ";
			// Token , duoc them vao de khai bao cau truc dac biet

			//if (nOp == ',') m_sCodeSaveJS += " , ";
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			Condition();
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
CVarLink *CProgramJs::Ternary()
{
	CVarLink *pVLCondition = Logic();
	CRuntimeException* pRuntimeException = NULL;
	if (m_pTokenPointer == NULL)
		return new CVarLink(new CVar());
	try
	{
		if (m_pTokenPointer->m_nTokenId == '?')
		{
			m_sCodeSaveJS += " ? ";
			m_pTokenPointer->Match('?');
			Base();
			m_pTokenPointer->Match(':');
			m_sCodeSaveJS += " : ";
			Base();
		}
	}
	catch (CRuntimeException * pRE)
	{
		pRuntimeException = pRE;
	}
	SAFE_THROW(pRuntimeException);
	return pVLCondition;
}

//------------------------------------------------------------------------------
// toan tu: Base - phep toan gan
// toan tu co so: =, +=, -= |= ^= &= <<= >>=
//------------------------------------------------------------------------------
CVarLink *CProgramJs::Base()
{
	string sLinkCheckRedirect;
	CVarLink *pVLRealLhs = NULL;
	CVarLink *pVLLeftOperand = NULL;

	CRuntimeException* pException = NULL;

	if (m_pTokenPointer == NULL)
		return NULL;
	try
	{
		if (GetCurrentStackSize() > STACK_SIZE)
			throw std::out_of_range("Tran Stack");

		pVLLeftOperand = Ternary();

		if (m_pTokenPointer->m_nTokenId == '='
			|| m_pTokenPointer->m_nTokenId == TK_PLUSEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_MINUSEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_OREQUAL
			|| m_pTokenPointer->m_nTokenId == TK_XOREQUAL
			|| m_pTokenPointer->m_nTokenId == TK_ANDEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_LSHIFTEQUAL
			|| m_pTokenPointer->m_nTokenId == TK_RSHIFTEQUAL)
		{
			CleanMsgRunInTryCatch();
			if (pVLLeftOperand && !pVLLeftOperand->m_bOwned)
			{
				if (pVLLeftOperand->m_sName.length() > 0)
				{
					pVLRealLhs = m_pVRootStack->AddChildNoDup(pVLLeftOperand->m_sName, pVLLeftOperand->m_pVar);
					pVLLeftOperand = pVLRealLhs;
				}
			}
			// Them vao ma nguon trich xuat
			if (m_pTokenPointer->m_nTokenId == '=') m_sCodeSaveJS += " = ";
			if (m_pTokenPointer->m_nTokenId == TK_PLUSEQUAL) m_sCodeSaveJS += " += ";
			if (m_pTokenPointer->m_nTokenId == TK_MINUSEQUAL) m_sCodeSaveJS += " -= ";
			if (m_pTokenPointer->m_nTokenId == TK_OREQUAL) m_sCodeSaveJS += " |= ";
			if (m_pTokenPointer->m_nTokenId == TK_XOREQUAL) m_sCodeSaveJS += " ^= ";
			if (m_pTokenPointer->m_nTokenId == TK_ANDEQUAL) m_sCodeSaveJS += " &= ";
			if (m_pTokenPointer->m_nTokenId == TK_LSHIFTEQUAL) m_sCodeSaveJS += " <<= ";
			if (m_pTokenPointer->m_nTokenId == TK_RSHIFTEQUAL) m_sCodeSaveJS += " >>= ";

			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			Base();
		}
	}
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

	if (pVLLeftOperand)
		return pVLLeftOperand;
	else
		return new CVarLink(new CVar());
}

// -----------------------------------------------------------------------------
// Xu ly khoi lenh 
// -----------------------------------------------------------------------------
bool CProgramJs::BlockCode()
{
	bool bResultExeBlock = true;
	CRuntimeException* pRuntimeException = NULL;
	if (m_pTokenPointer == NULL)
		return false;
	try
	{
		m_pTokenPointer->Match('{');
		m_sCodeSaveJS += " { ";
		while (m_pTokenPointer->m_nTokenId && m_pTokenPointer->m_nTokenId != '}')
		{
			Statement();
			m_sCodeSaveJS += " ";
		}
		m_pTokenPointer->Match('}');
		m_sCodeSaveJS += " } ";
		m_nameVarInFuncCount = 0;
	}
	catch (CRuntimeException *pRE) 
	{
		pRuntimeException = pRE;
	}
	SAFE_THROW(pRuntimeException);
	return bResultExeBlock;
}

// -----------------------------------------------------------------------------
// Xu ly tung doan lenh
// -----------------------------------------------------------------------------
bool CProgramJs::Statement()
{
	CTokenPointer* pTokenParent = m_pTokenPointer;
	CTokenPointer *pTokenForIn = NULL;
	int nWhileCondStart;
	int nWhileBodyStart;
	int nDoWhileBodyStart;

	int nForCondStartForIn;

	bool bResultRunStatement = true;
	bool bForIn = false;
	string sMsgMsgRunInTryCatch;

	CVarLink *pVLCond = NULL;
	CVarLink *pVLTmpValue = NULL;
	CVarLink *pVLAfterDots = NULL;
	CVarLink *pVLReturn = NULL;
	CVarLink* pVLValueForInArray = NULL;
	CVarLink* pVLValueForIn = NULL;

	CRuntimeException *pException = NULL;

	if (m_pTokenPointer == NULL)
		return false;
	try
	{
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
			pVLTmpValue = Base();
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
		}

		//----------------------------------------------------------------------
		/* A block of szCode */
		else if (m_pTokenPointer->m_nTokenId == '{') // xu ly den }
		{
			bResultRunStatement = BlockCode();
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
			m_sCodeSaveJS += " var ";

			m_pTokenPointer->Match(TK_R_VAR);
			while (m_pTokenPointer->m_nTokenId != ';')
			{
				// Tim var trong stack >> pVLTmpValue
				if (m_lstStack.size() > 0)
				{
					// sontdc
					string aliasName = "";
					if (m_inFunc[indexInFunc] == false)
					{
						aliasName = "globalVar" + to_string(m_nameVarCount);
					}
					else
						aliasName = "funcVar" + to_string(m_nameVarInFuncCount);
					pVLTmpValue = m_lstStack.back()->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, aliasName);
					if (pVLTmpValue->m_sAliasName == aliasName) // Neu la bien moi
						if (m_inFunc[indexInFunc] == false)
							m_nameVarCount += 1;
						else m_nameVarInFuncCount += 1;
					m_sCodeSaveJS += " " + pVLTmpValue->m_sAliasName + " ";
				}

				m_pTokenPointer->Match(TK_ID);
				// now do stuff defined with dots
				while (m_pTokenPointer->m_nTokenId == '.')
				{
					m_pTokenPointer->Match('.');
					if (pVLTmpValue && pVLTmpValue->m_pVar)
					{
						pVLAfterDots = pVLTmpValue;
						pVLTmpValue = pVLAfterDots->m_pVar->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, "");
					}
					m_pTokenPointer->Match(TK_ID);
				}

				// sort out initialiser
				if (m_pTokenPointer->m_nTokenId == '=')
				{
					// Xu ly gia tri cua bien sau dau =
					m_sCodeSaveJS += " = ";
					m_pTokenPointer->Match('=');
					Base();
					//pVLTmpValue->ReplaceWith(pVLValueInit);
				}

				// Khai bao nhieu bien lien tiep
				if (m_pTokenPointer->m_nTokenId == ',')
				{
					m_pTokenPointer->Match(',');
					m_sCodeSaveJS += " , ";
				}
				else
				{
					break;
				}
			}
			// Thuong thi Match tokenID la dau ;
			m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);

		}

		//----------------------------------------------------------------------
		// Process Block: IF ( condition ) { ... } ELSE { ... }  
		else if (m_pTokenPointer->m_nTokenId == TK_R_IF)
		{
			m_sCodeSaveJS += " if ";
			m_pTokenPointer->Match(TK_R_IF);

			m_sCodeSaveJS += " ( ";
			m_pTokenPointer->Match('(');

			Base();
			m_pTokenPointer->Match(')');
			m_sCodeSaveJS += " ) ";

			Statement();

			if (m_pTokenPointer->m_nTokenId == TK_R_ELSE)
			{
				m_sCodeSaveJS += " else ";
				m_pTokenPointer->Match(TK_R_ELSE);
				Statement();
			}
		}
		//----------------------------------------------------------------------
		// Process Block: do{... Body Code ...}  WHILE ( condition ) //---------[NDC]
		else if (m_pTokenPointer->m_nTokenId == TK_R_DO)
		{
			m_pTokenPointer->Match(TK_R_DO);
			m_sCodeSaveJS += " do ";
			nDoWhileBodyStart = m_pTokenPointer->m_nTokenStart;
			Statement();

		}

		//----------------------------------------------------------------------
		// Process Block: WHILE ( condition ) {... Body Code ...} //------------
		else if (m_pTokenPointer->m_nTokenId == TK_R_WHILE)
		{
			m_pTokenPointer->Match(TK_R_WHILE);
			m_sCodeSaveJS += " while ";
			m_pTokenPointer->Match('(');
			m_sCodeSaveJS += " ( ";
			Base();
			m_pTokenPointer->Match(')');
			m_sCodeSaveJS += " ) ";
			Statement();
		}
		//----------------------------------------------------------------------
		// Process Block: 
		// FOR( init Iterator; condition ; Iterator1 change, Iterator2 change ) //[NDC]
		//   { ...Body code... } 
		else if (m_pTokenPointer->m_nTokenId == TK_R_FOR)
		{
			m_sCodeSaveJS += " for ";
			m_pTokenPointer->Match(TK_R_FOR);
			m_pTokenPointer->Match('(');
			m_sCodeSaveJS += " ( ";

			if (m_pTokenPointer->m_nTokenId == TK_R_VAR)
			{
				m_pTokenPointer->Match(TK_R_VAR);
				m_sCodeSaveJS += " var ";
				// Tim trong scope de phong khai bao trung
				CVarLink* pVLValueReturn = FindInScopes(m_pTokenPointer->m_sTokenStr);
				if (!pVLValueReturn)
				{
					pVLValueReturn = new CVarLink(new CVar());
					// Tim var trong stack >> pVLTmpValue
					if (m_lstStack.size() > 0)
					{
						string aliasName = "";
						if (m_inFunc[indexInFunc] == false)
						{
							aliasName = "globalVar" + to_string(m_nameVarCount);
						}
						else
							aliasName = "funcVar" + to_string(m_nameVarInFuncCount);
						pVLTmpValue = m_lstStack.back()->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, aliasName);
						if (pVLTmpValue->m_sAliasName == aliasName) // Neu la bien moi
							if (m_inFunc[indexInFunc] == false)
								m_nameVarCount += 1;
							else m_nameVarInFuncCount += 1;
						m_sCodeSaveJS += " " + pVLTmpValue->m_sAliasName + " ";
					}
					//luu lai token goc
					pTokenParent = m_pTokenPointer;
					nForCondStartForIn = m_pTokenPointer->m_nTokenStart;
					//luu lai token mo dau for(...  
					pTokenForIn = m_pTokenPointer->GetSubTokenForIn(nForCondStartForIn);
					m_pTokenPointer = pTokenForIn;

					m_pTokenPointer->Match(TK_ID);
					if (m_pTokenPointer->m_nTokenId == TK_R_IN)
					{
						m_sCodeSaveJS += " in ";
						m_pTokenPointer->Match(TK_R_IN);
						//lay gia tri cua y trong x in y
						pVLValueForInArray = FindInScopes(m_pTokenPointer->m_sTokenStr);
						if (!pVLValueForInArray)
							m_sCodeSaveJS += " " + m_pTokenPointer->m_sTokenStr + " ";
						else
							m_sCodeSaveJS += " " + pVLValueForInArray->m_sAliasName + " ";
						m_pTokenPointer->Match(TK_ID);
						m_pTokenPointer->Match(')');
						m_sCodeSaveJS += " ) ";
						Statement();
						bForIn = true;
					}
					else m_pTokenPointer = pTokenParent;
				}
			}
			if (m_pTokenPointer->m_nTokenId == TK_ID) // [NDC]
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

					pVLValueForIn = FindInScopes(m_pTokenPointer->m_sTokenStr);

					if (!pVLValueForIn)
					{
						pVLValueForIn = new CVarLink(new CVar(), m_pTokenPointer->m_sTokenStr);

						SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, m_pTokenPointer->m_sTokenStr, NULL);
					}

					/*m_pTokenPointer->Match(TK_ID);*/
					m_pTokenPointer->Match(TK_R_IN);
					m_sCodeSaveJS += " in ";
					//lay gia tri cua y trong x in y
					pVLValueForInArray = FindInScopes(m_pTokenPointer->m_sTokenStr);

					if (!pVLValueForInArray)
					{
						/* Variable doesn't exist! JavaScript says we should create it
					* (we won't add it here. This is done in the assignment operator)*/
						pVLValueForInArray = new CVarLink(new CVar(), m_pTokenPointer->m_sTokenStr);
						m_sCodeSaveJS += " " + m_pTokenPointer->m_sTokenStr + " ";
						SetMsgRunInTryCatch(EXCEPTION_ID_NOTFOUND_IN_TRYCATCH, m_pTokenPointer->m_sTokenStr, NULL);
					}
					else
						m_sCodeSaveJS += " " + pVLValueForInArray->m_sAliasName + " ";
					m_pTokenPointer->Match(TK_ID);
					m_pTokenPointer->Match(')');
					m_sCodeSaveJS += " ) ";
					Statement();
					bForIn = true;
				}
				else m_pTokenPointer = pTokenParent;
			}
			if (!bForIn)
			{
				if (m_pTokenPointer->m_nTokenId != ';')
				{
					Statement();
					m_sCodeSaveJS += " ";
				}
				else
				{
					m_pTokenPointer->Match(';');
				}

				if (m_pTokenPointer->m_nTokenId != ';')
				{
					pVLCond = Base();
					m_pTokenPointer->Match(';');
				}
				else
				{
					m_pTokenPointer->Match(';');
				}
				while (m_pTokenPointer->m_nTokenId != TK_EOF && m_pTokenPointer->m_nTokenId != ')')
				{
					Base();
					if (m_pTokenPointer->m_nTokenId == ',')
						m_pTokenPointer->Match(',');
				}
				m_pTokenPointer->Match(')');
				m_sCodeSaveJS += " ) ";
				Statement();
			}
		}

		// ---------------------------------------------------------------------
		// Process Block: RETURN ...  
		else if (m_pTokenPointer->m_nTokenId == TK_R_RETURN)
		{
			m_pTokenPointer->Match(TK_R_RETURN);
			m_sCodeSaveJS += " return ";
			pVLTmpValue = NULL;
			if (m_pTokenPointer->m_nTokenId != ';')
				pVLTmpValue = Base();
			pVLReturn = m_lstStack.back()->FindChild(STR_RETURN_VAR);
			if (pVLReturn)
				pVLReturn->ReplaceWith(pVLTmpValue);
			m_pTokenPointer->Match(';');
		}

		//----------------------------------------------------------------------
		// Process Block: FUNCTION funcName(arg .. ) { ... } 
		else if (m_pTokenPointer->m_nTokenId == TK_R_FUNCTION)
		{
			// save 'function' keyword
			m_sCodeSaveJS += " function ";
			indexInFunc += 1;
			m_inFunc[indexInFunc] = true;
			pVLTmpValue = ParseFunctionDefinition();

			if (pVLTmpValue) 
			{
				if (pVLTmpValue->m_sName == STR_TEMP_NAME)
				{
					OutputDebug("Functions defined at statement-level are meant to have a name\n");
				}
				else
				{
					m_lstStack.back()->AddChildNoDup(pVLTmpValue->m_sName, pVLTmpValue->m_pVar, pVLTmpValue->m_sAliasName);
					m_lstStack.push_back(pVLTmpValue->m_pVar);
				}
			}
			BlockCode();
			indexInFunc -= 1;
			m_inFunc[indexInFunc] = false;
		}

		//----------------------------------------------------------------------
		// Process try - catch - finally. 
		else if (m_pTokenPointer->m_nTokenId == TK_R_TRY)
		{
			m_sCodeSaveJS += " try ";

			// xu ly khoi lenh try .............................................
			m_pTokenPointer->Match(TK_R_TRY);
			SetRunInTryCatch();
			Statement();

			// xu ly khoi lech catch ...........................................
			if (m_pTokenPointer->m_nTokenId == TK_R_CATCH)
			{
				m_sCodeSaveJS += " catch ";
				m_pTokenPointer->Match(TK_R_CATCH);
				m_pTokenPointer->Match('(');
				m_sCodeSaveJS += " ( ";
				//them bien exception

				if (m_lstStack.size() > 0)
				{
					// sontdc
					string aliasName = "";
					if (m_inFunc[indexInFunc] == false)
					{
						aliasName = "globalCatchVar" + to_string(m_nameVarCatchCount);
					}
					else
						aliasName = "catchVar" + to_string(m_nameVarCatchFuncCount);
					pVLTmpValue = m_lstStack.back()->FindChildOrCreate(m_pTokenPointer->m_sTokenStr, aliasName);
					if (pVLTmpValue->m_sAliasName == aliasName) // Neu la bien moi
						if (m_inFunc[indexInFunc] == false)
							m_nameVarCatchCount += 1;
						else m_nameVarCatchFuncCount += 1;
						m_sCodeSaveJS += " " + pVLTmpValue->m_sAliasName + " ";
				}
				m_pTokenPointer->Match(TK_ID);
				m_pTokenPointer->Match(')');
				m_sCodeSaveJS += " ) ";
				Statement();

			}

			// xu ly khoi lech finally neu co ..................................
			if (m_pTokenPointer->m_nTokenId == TK_R_FINALLY)
			{
				m_sCodeSaveJS += " finally ";
				m_pTokenPointer->Match(TK_R_FINALLY);
				Statement();
			}
		}

		else if (m_pTokenPointer->m_nTokenId == TK_R_THROW)
		{
			m_pTokenPointer->Match(TK_R_THROW);
			m_sCodeSaveJS += " throw ";
			Base();
		}

		// neu gap break hoac continue thi tiep tuc [NDC]
		else if (m_pTokenPointer->m_nTokenId == TK_R_BREAK)
		{
			m_sCodeSaveJS += " break ";
			m_pTokenPointer->Match(TK_R_BREAK);
		}
		else if (m_pTokenPointer->m_nTokenId == TK_R_CONTINUE)
		{
			m_sCodeSaveJS += " continue ";
			m_pTokenPointer->Match(TK_R_CONTINUE);
		}
		else if (m_pTokenPointer->m_nTokenId == TK_R_DELETE)
		{
			m_sCodeSaveJS += " delete ";
			m_pTokenPointer->Match(TK_R_DELETE);
		}
		// Kiem tra token ket thuc
		else if (m_pTokenPointer->m_nTokenId == '}')
		{
			m_sCodeSaveJS += " } ";
			m_pTokenPointer->Match('}');
		}
		else
		{
			if (m_pTokenPointer->m_nTokenId == TK_EOF)
				m_pTokenPointer->Match(TK_EOF);
			else
			{
				// Them ngay 11/5/20: test xem neu khong co token id phu hop thi luu token do ra ngoai code sau do tim tiep
				if (m_pTokenPointer->m_nTokenId >= 32 && m_pTokenPointer->m_nTokenId <= 126)
				{
					string temp(1, m_pTokenPointer->m_nTokenId);
					m_sCodeSaveJS += " " + temp + " ";
				}
				m_pTokenPointer->Match(m_pTokenPointer->m_nTokenId);
			}
		}
	}
	catch (CRuntimeException* pException)
	{
		// Bo qua lenh nay de tiep tuc xu ly cac lenh khac
		SAFE_DELETE(pException);
		m_pTokenPointer->JumpLastStatement();
	}
	// Exception VR >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	SAFE_THROW(pException);
	m_pTokenPointer = pTokenParent;
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
// Get attribute "CProgramJs::m_pCFileHtml"
//------------------------------------------------------------------------------
CFileHtml* CProgramJs::GetMemberCFileHtml()
{
	return m_pFileHtmlProcess;
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
/// Finds a child, looking recursively up the m_listStack
//------------------------------------------------------------------------------	
CVarLink *CProgramJs::FindInScopesAdvance(const string &sChildName)
{
	int nIndex;
	CVarLink *pVL = NULL;
	CVarLink *pVLFirst = NULL;
	for (nIndex = m_lstStack.size() - 1; nIndex >= 0; nIndex--)
	{
		pVL = m_lstStack[nIndex]->FindChild(sChildName);
		if (!pVL)
		{
			pVLFirst = m_lstStack[nIndex]->m_pVLFirstChild;
			while (pVLFirst != NULL)
			{
				pVL = pVLFirst->m_pVar->FindChild(sChildName);
				if (pVL)
					return pVL;
				pVLFirst = pVLFirst->m_pNextSibling;
			}

		}
		else return pVL;
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

	if (pVObject && m_pVDoubleClass && pVObject->IsObject())
	{
		pVLImplementation = m_pVStringClass->FindChild(sName);
		if (pVLImplementation)
			goto _COMPLETE;
		pVLImplementation = m_pVArrayClass->FindChild(sName);
		if (pVLImplementation)
			goto _COMPLETE;
		pVLImplementation = m_pVDoubleClass->FindChild(sName);
		if (pVLImplementation)
			goto _COMPLETE;
		pVLImplementation = m_pVObjectClass->FindChild(sName);
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
		set_new_handler(NotEnoughMemory);

		if (!g_pProgramJs)
			g_pProgramJs = new CProgramJs();

		// Cai dat lai du lieu JS code cho chuong trinh
		g_pProgramJs->SetData(pBuffDataHtml, dwSizeBuf);

		lstsCodeJs = g_pProgramJs->GetLstSzCodeJs();
		nCountSzCodeJs = lstsCodeJs.size();
		bool bExceptionFlag = false;
		if (nCountSzCodeJs > 0)
		{
			g_pProgramJs->m_pVRootStack->AddChild("result", new CVar(0));
			g_pProgramJs->m_sCodeSaveJS = "";
			string sFinalCodeJS = "";
			for (i = 0; i < nCountSzCodeJs; i++)
			{
				g_pProgramJs->m_sCodeSaveJS = "";
				sCodeJs = lstsCodeJs.at(i);
				try 
				{
					if (sCodeJs.length() > 0)
					{
						g_pProgramJs->Execute(sCodeJs);
					}
				}
				catch (CRuntimeException *pRE)
				{
					bExceptionFlag = true;
					nErrId = pRE->GetErrId();
					if (nErrId == EXCEPTIONID_FOUND_VIRUS
						|| nErrId == NOT_ENOUGH_MEMORY
						|| nErrId == EXCEPTION_TIME_OUT_SCAN)
						throw pRE;
					SAFE_DELETE(pRE);
				}
				catch (int e)
				{
					bExceptionFlag = true;
					if (e == NOT_ENOUGH_MEMORY)
					{
						OutputDebug("_LOI Crash: Ko du bo nho tai nguyen!");
						g_iNotEnoughMem = 0;
						throw NOT_ENOUGH_MEMORY;
					}
				}
				if (sCodeJs.size() > 0)
				{
					if (g_pProgramJs->m_sCodeSaveJS.size() > 5)
					{
						sFinalCodeJS += g_pProgramJs->m_sCodeSaveJS;
						sFinalCodeJS += " ";
					}
					
				}
			}

			// WRITE FILE
			if (sFinalCodeJS.size()>0)
			{
				HANDLE hSaveExportFile = CreateFileA(sSaveFileName.c_str(),
					GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

				DWORD dwBytes = 0;
				WriteFile(hSaveExportFile, sFinalCodeJS.c_str(), sFinalCodeJS.size(), &dwBytes, NULL);
				CloseHandle(hSaveExportFile);
				// WRITE FILE DONE
			}
			sSaveFileName = "";
		}
	}
	catch (CRuntimeException *pRE)
	{
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
	OutputDebug("Scan Complete!!");
	if (!g_pProgramJs->ResetDatabase())
	{
		SAFE_DELETE(g_pProgramJs);
	}

#if SAFE_MEMORY
#endif
	DbgPrintLnA("%s", "Exit Scanfile ...");
	return bResultScan;
}

string CProgramJs::XuLyChuoiTinh(string sCodeJS) 
{
	CRuntimeException* pRuntimeException = NULL;
	
	int pos = 0;
	int sub_pos = 0;
	sCodeJS[sCodeJS.size()] = '\0';
	try
	{

		while (pos < sCodeJS.size())
		{
			if (sCodeJS.at(pos) == ' ')
			{
				sub_pos = pos + 1;
				if (sub_pos >= sCodeJS.size()) goto _COMPLETE;
				while (sCodeJS.at(sub_pos) == ' ')
				{
					sCodeJS.replace(sub_pos, 1, "");
					sub_pos += 1;
					if (sub_pos >= sCodeJS.size()) goto _COMPLETE;
				}
				pos = sub_pos;
			}
			pos += 1;
		}
	} 
	catch (CRuntimeException * pRE)
	{
		pRuntimeException = pRE;
	}
	
_COMPLETE:
	SAFE_THROW(pRuntimeException);
	return sCodeJS;
}
