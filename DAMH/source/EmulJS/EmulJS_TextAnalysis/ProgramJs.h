﻿////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia lop CProgramJs. Lop CProgramJs la Moi truong gia 
// lap de cho 1 doan codeJS co the thuc thi duoc.
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef FILE_INPUT_H
#define FILE_INPUT_H

#ifndef BUID_UPDATE
#endif

#include <string>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <atlstr.h>
#include <stack>

using namespace std;

#include "FileHtml.h"
#include "Var.h"
#include "DATA_DEF.h"
#include "RuntimeException.h"
#include "TokenPointer.h"
#include "APISupport.h"
#include "FunctionExport.h"
#include "md5.h"
#include "sha256.h"

#define INPUT_PATH_FILE_JS		0
#define INPUT_PATH_FILE_HTML	1
#define INPUT_DATA_FILE_HTML	2
#define NUMBER_KB				94

#define  STACK_SIZE		256*1024


//==============================================================================
// Name: CProgrameJS
// Description: Main Class - read szCode from file.js, compile szCode
//==============================================================================
class CProgramJs
{

public:
	CVar* m_pVRootStack;	/// m_pRootStack : ds bien toan cuc cua chuong trinh
	CVarLink* m_pVLWindowOnload;// bien window.onload 
	CVarLink* m_pVLDcmtOnmousemove;
	CVarLink* m_pVLDStatus;
	CVarLink* m_pVLDStringStatus;
	CVarLink* m_pVLDStringresponseText;
	CVarLink* m_pVLDStringOnreadyStateChange;
	CVarLink* m_pVLDStringreadyState;
	//--------------------------------------------------------------------------
	// Khoi tao doi tuong moi
	//--------------------------------------------------------------------------
	CProgramJs(void);
	//--------------------------------------------------------------------------
	// Khoi tao doi tuong moi khi co bo du lieu file Html
	//--------------------------------------------------------------------------
	CProgramJs(LPVOID pBuff, DWORD dwSizeBuff = 0, int nTypeFile = INPUT_DATA_FILE_HTML);

	//--------------------------------------------------------------------------
	// huy bo doi tuong
	//--------------------------------------------------------------------------
	~CProgramJs(void);

	//--------------------------------------------------------------------------
	// Set szCodeJs for programJS from data file HTML
	//--------------------------------------------------------------------------
	void SetData(LPVOID pBuffDataHtml, DWORD dwSizeBuff = 0);

	//--------------------------------------------------------------------------
	// Get m_szDataFile 
	//--------------------------------------------------------------------------
	BOOLEAN ResetDatabase();

	//--------------------------------------------------------------------------
	// Xoa bo cac the sau document.body.tagName
	//--------------------------------------------------------------------------
	BOOLEAN ResetDocumentTree();

	//--------------------------------------------------------------------------
	// Xoa cac bien co thuoc tinh: "CVarLink::m_bReDefine == true;"
	// Khoi phuc RootStack ve trang thai ban dau
	//--------------------------------------------------------------------------
	void CleanRootStack();

	//--------------------------------------------------------------------------
	// run Programe with szCode
	//--------------------------------------------------------------------------
	void Execute(const string &sCode);

	CVarLink* EvaluateComplex(const string &sCode);

	/** Evaluate the given code and return a string. If nothing to return, will return
	* 'undefined' */
	string Evaluate(const string &sCode);

	/// add a native function to be called from ProgramJS
	void AddFuncNative(const string &sFuncDesc, JSCallback pJSCallBack, void *pUserData);
	void SetFuncNative(const string &sFuncName, JSCallback pJSCallBack, void *pUserData);
	void AddAttrNative(const string &sPathAttr, int nTypeAttr);
	void AddAttrNative(const string &sPathAttr, CVar* pVAttr);
	// get m_lstSzDataPrograme
	vector<string> GetLstSzCodeJs();

	/// Get the given variable specified by a path (var1.var2.etc), or return 0
	CVar *GetScriptVariable(const string &sPath);

	/// Get the given variable link specified by a path (var1.var2.etc), or return 0
	CVarLink *CProgramJs::GetVLScriptVariable(const string &sPath);

	// Get attribute "CProgramJs::m_pCFileHtml"
	CFileHtml* GetMemberCFileHtml();

	/// Send all variables to stdout
	void Trace();

	// check virus small Iframe in "strHtml"
	DWORD CheckVirus(string sCodeHtml);
	DWORD CheckVirus(CVar* pVHtmlElement);

	// Kiem tra tat ca cac phan tu HTML xem co phan tu nao co toa do trung voi 
	// toa do cua con chuot dang di chuyen
	DWORD CheckVirusFollowMouse();

	bool CheckVirusStatic(string sCodeJS);
	int CheckVirusEval(string sCodeJS);
	// Kiem tra virus redirect den 1 dia chi khong hop le
	bool CProgramJs::CheckVirusRedirect(CVarLink*pVLVariable, string sNewLink);

	// check attribute hidden in HTML tag
	bool CheckAttrHidden(string sAttrOfHtmlTag);
	bool CheckAttrHidden(CVar* pVHtmlElement);

	// Tim kiem phan tu con trong Prototype cua lop
	CVarLink*	FindInPrototypeClasses(CVar *pVObject, const string &sName);

	// Tao doi tuong HTML element tu thong tin dau vao
	// mapInfoHtmlElemnt["tagName"] = "";
	// mapInfoHtmlElemnt["id"] = "";
	CVarLink*	CreateNewHTMLElement(map<string, string> mapInfoHtmlElemnt);

	// Xuat ra danh sach pVL toi cac the HTML la con cua 'pVObject'
	vector<CVarLink*> GetAllChildHtmlElement(CVar *pVObject, vector<CVarLink*> lstVLHtmlElement);

	// Lay thong tin trong code file HTML
	CVarLink* GetInfoHtmlElementFromJQuerySelector(string sJQuerySelectors);

	// Doc tat ca cac dinh nghia ham trong code
	void GetAllFuncDefineInCode(const string &sCode, int nPosStart = 0);

	// [Kiem tra sCode co phai la code JS hay khong]
	// - Doc cac token
	// + false: neu co 2 token ID canh nhau
	// + true: con lai
	bool CProgramJs::IsCodeJavaScript(const string &sCode);

	// Thuc hien ham luu trong pVLFuncCall
	CVarLink* FunctionCall(CVarLink *pVLFuncCall, CVar *pVParent);

	CVarLink *RunFunction(CVarLink *pVLFunc, CVar *pVParent, CVar*pVArgumentsValue);

	void AddFuncCallBackBySetInterval(CVar* pVarFunction);
	BOOL CheckFuncCallBackBySetInterval(CVar* pVarFunction);

	// Xu ly khoi try-catch trong code chuong trinh
	void SetRunInTryCatch();
	void SetNotRunInTryCatch();
	bool IsRunInTryCatch();
	void SetMsgRunInTryCatch(int nTypeException, string sId, CVar* pVData);
	void CleanMsgRunInTryCatch();
	string XuLyChuoiTinh(string sCodeJS);
	////////////////////////////////////////////////////////////////////////////////////////
private:
	bool    m_onreadystatechange;
	bool    m_ForIn;
	bool	m_bRunInTryCatch;
	bool	m_bHadException;
	int     nLenghtArrayForIn;
	BYTE*	m_szDataFile;
	WCHAR	m_szPathFile[512];
	DWORD	m_dwSizeData;
	CFileHtml* m_pFileHtmlProcess;
	vector<string> m_lstsCodeJs;
	vector<string> m_lstsCodeJsJob;

	CTokenPointer*	m_pTokenPointer; /// current m_TokenPointer
	vector<CVar*>	m_lstStack; /// stack of m_listStack when parsing
	vector<CVar*>	m_lstFuncCallBackBySetInterval;
	vector<CVarLink*> m_lstVLRedirectLink;
	vector<CVarLink*> m_lstVLMsgException;

	CVar *m_pVStringClass; /// Built in string class
	CVar *m_pVObjectClass; /// Built in object class
	CVar *m_pVArrayClass;  /// Built in array class
	CVar *m_pVDoubleClass;
	CVarLink* m_pVLStartReset;	// Luu vi tri bat dau ds bien trong qua trinh quet

	// parsing - in order of precedence
	void	 InitProgramJs();
	void	RegisterListAttrRedirect();// ham bat mau redirect

	// Chuoi toan tu trong 1 cau lenh
	CVarLink *FactorAdvance(CVarLink* pVLObjectCall, CVar* pVParent = NULL);
	CVarLink *Factor();
	CVarLink *Unary();
	CVarLink *Term();
	CVarLink *Expression();
	CVarLink *Shift();
	CVarLink *Condition();
	CVarLink *Logic();
	CVarLink *Ternary();
	CVarLink *Base();
	bool BlockCode();
	bool Statement();
	// set tat ca cac thuoc tinh con tro cua doi tuong = NULL
	void InitNullForPropertyPointer();

	// parsing utility functions
	CVarLink*	ParseFunctionDefinition();
	void		ParseFunctionArguments(CVar *pVFunc, bool bIsReDefine = true);

	// Luu danh sach phan tu vao pVList
	void		GetListChildArray(CVar *pVList);

	// Finds a child, looking recursively up the m_listStack
	CVarLink*	FindInScopes(const string &sChildName);
	CVarLink* FindInScopesAdvance(const string &sChildName);
	// Look up in any parent classes of the given object
	CVarLink*	FindInParentClasses(CVar *pVObject, const string &sName);

	// Xoa tat ca con cua bien pVRedefine neu co thuoc tinh m_bReDefine = true 
	void RemoveChildReDefine(CVar* pVRedefine);
public:
	string m_sCodeSaveJS;
	string m_sExportFileName;

private:
	int m_nameFuncCount;
	int m_nameVarCount;
	int m_nameVarCatchCount;
	int m_nameVarCatchFuncCount;
	bool m_inFunc[20];
	int indexInFunc;
	int m_nameVarInFuncCount;
	int m_iCountArrayElement;
};

#endif 

// define some simple Function ................................................
#ifndef SIMPLE_FUNC
#define SIMPLE_FUNC
#ifdef _DEBUG
const int   JS_LOOP_MAX_TIME = 40000;
#else
const int   JS_LOOP_MAX_TIME = 20000;
#endif
const int	JS_LOOP_MAX_ITERATIONS = 8192;
#define		JS_LOOP_MAX_ITERATIONS_STR "8192"

#define CREATE_LINK(LINK, VAR) { if (!LINK || LINK->m_bOwned) LINK = new CVarLink(VAR); else LINK->ReplaceWith(VAR); }
#define CLEAN_VAR_LINK(pVarLink) {if (pVarLink && !pVarLink->m_bOwned) { delete pVarLink;  pVarLink = NULL;}}

#define SAVE_BOOL_RETURN(bReturn, StatementRun) { bool bThisStatement = (StatementRun); if(bReturn) bReturn = bThisStatement;}

#endif 
// ............................................................................


