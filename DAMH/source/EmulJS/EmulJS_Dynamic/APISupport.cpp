////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac ham API gia lap de ho tro cac lop String, 
// Array, Math, Object, ... trong javaScript
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include "APISupport.h"

#include <vector>
#include <math.h>
#include <cstdlib>
#include <sstream>
using namespace std;


PDETECT_DOWNLOADER g_sstrDetectDownloader;
PDETECT_VIRUSOTHER g_sstrDetectVirusOther;

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

#define k_E                 exp(1.0)
#define k_PI                3.1415926535897932384626433832795

#define F_ABS(a)            ((a)>=0 ? (a) : (-(a)))
#define F_MIN(a,b)          ((a)>(b) ? (b) : (a))
#define F_MAX(a,b)          ((a)>(b) ? (a) : (b))
#define F_SGN(a)            ((a)>0 ? 1 : ((a)<0 ? -1 : 0 ))
#define F_RNG(a,min,max)    ((a)<(min) ? min : ((a)>(max) ? max : a ))
#define F_ROUND(a)          ((a)>0 ? (int) ((a)+0.5) : (int) ((a)-0.5) )

//CScriptVar shortcut macro
#define scIsInt(a)          ( pVLocalStack->GetParameter(a)->IsInt() )
#define scIsDouble(a)       ( pVLocalStack->GetParameter(a)->IsDouble() )  
#define scGetInt(a)         ( pVLocalStack->GetParameter(a)->GetInt() )
#define scGetString(a)       ( pVLocalStack->GetParameter(a)->GetString() )
#define scGetDouble(a)      ( pVLocalStack->GetParameter(a)->GetDouble() ) 

#define scReturnInt(a)      ( pVLocalStack->GetReturnVar()->SetInt(a) )
#define scReturnDouble(a)   ( pVLocalStack->GetReturnVar()->SetDouble(a) )  
#define scReturnString(a)	( pVLocalStack->GetReturnVar()->SetString(a) )  
#define scReturnRegex(a)	( pVLocalStack->GetReturnVar()->SetRegex(a) )  
#ifndef _MSC_VER
namespace
{
	double asinh( const double &value )
	{
		double returned;

		if(value>0)
			returned = log(value + sqrt(value * value + 1));
		else
			returned = -log(-value + sqrt(value * value + 1));

		return(returned);
	}

	double acosh( const double &value )
	{
		double returned;

		if(value>0)
			returned = log(value + sqrt(value * value - 1));
		else
			returned = -log(-value + sqrt(value * value - 1));

		return(returned);
	}
}
#endif

//------------------------------------------------------------------------------
// Go bo ki tu cRemove trong szInput
//------------------------------------------------------------------------------
string StrimString(string sInput, char cRemove)
{
	int nSize = sInput.size();
	int i = 0;
	string sOutput;

	for (i = 0; i < nSize; i++)
	{
		if (sInput.at(i) != cRemove)
			sOutput += sInput.at(i);
	}

	return sOutput;
}

//------------------------------------------------------------------------------
// Tach szInput thanh mang theo ki tu phan cach
//------------------------------------------------------------------------------
vector<string> SplitString(string sInput, char cBoundary)
{
	int nSize = sInput.size();
	int i = 0;
	string sOutput = "";
	vector<string> lstSzOutput;

	for (i = 0; i < nSize; i++)
	{
		if (sInput.at(i) != cBoundary)
			sOutput += sInput.at(i);
		else 
		{
			if (sOutput.size()>0)
			{
				lstSzOutput.push_back(sOutput);
				sOutput = "";
			}
		}
	}

	if (sOutput.size() > 0)
	{
		// push phan tu cuoi cung neu khong co ki tu 'c' cuoi xau
		lstSzOutput.push_back(sOutput);
		sOutput = "";
	}
	return lstSzOutput;
}

//------------------------------------------------------------------------------
// Lay chuoi so dau tien ke tu vi tri bat dau trong xau szInput
//------------------------------------------------------------------------------
string GetSzDigit(string sInput)
{
	string sVal = "";
	int nPosValBegin = 0;
	int nSize = sInput.size();

	// bo qua cac ki tu truoc ki tu so
	while (nPosValBegin < nSize)
	{
		if ((sInput.at(nPosValBegin) >= '0' && sInput.at(nPosValBegin) <= '9')
			|| (sInput.at(nPosValBegin) == '-'))
			break;
		nPosValBegin++;
	}

	// doc ki tu so
	if (nPosValBegin<nSize && sInput.at(nPosValBegin) == '-')
	{
		sVal += sInput.at(nPosValBegin);
		sVal += "0";
		nPosValBegin++;
	}

	while (nPosValBegin < nSize)
	{
		if (sInput.at(nPosValBegin) >= '0' && sInput.at(nPosValBegin) <= '9' || sInput.at(nPosValBegin) == '.')
			sVal += sInput.at(nPosValBegin);
		else
			break;
		nPosValBegin++;
	}

	return sVal;
}

//------------------------------------------------------------------------------
// Lay chuoi ki tu dau tien [0-9][a-z][A-Z] ke tu vi tri bat dau
//------------------------------------------------------------------------------
string GetFirstString(string sInput, DWORD dwPosBegin)
{
	DWORD	dwSize = 0;
	DWORD	dwPos = dwPosBegin;
	BYTE	byCheck = 0;
	string	sVal = "";

	dwSize = sInput.size();
	if (dwPos >= dwSize)
		goto _RETURN;

	// doc ki tu trong tap [0-9][a-z][A-Z]
	byCheck = sInput.at(dwPos);
	while ((byCheck >= '0' && byCheck <= '9') || (byCheck >= 'a' && byCheck <= 'z') || (byCheck >= 'A' && byCheck <= 'Z'))
	{
		dwPos++;
		if (dwPos < dwSize)
			byCheck = sInput.at(dwPos);
		else
			byCheck = 0;
	}
	sVal = sInput.substr(dwPosBegin, dwPos - dwPosBegin);

_RETURN:
	return sVal;
}
//------------------------------------------------------------------------------
// return stack call
//------------------------------------------------------------------------------
void scTrace(CVar *pVLocalStack, void *pUserData)
{
	CProgramJs *js = (CProgramJs*)pUserData;
	js->m_pVRootStack->Trace();
}

//------------------------------------------------------------------------------
// Unescape
//------------------------------------------------------------------------------
void scUnescape(CVar *pVLocalStack, void *pUserData)
{
	CProgramJs *programJs = (CProgramJs*)pUserData;
	string sInput = scGetString("str");
	string sOutput = "";
	char szNumHex[4];
	int i = 0;
	int nSize = sInput.size();
	
	szNumHex[2] = 0;
	if (nSize >= 3)
	{
		// tim vi tri ki tu '%' -> chuyen 2 byte phia sau thanh HEX
 		for (i = 0; i < nSize; i++)
		{
			if (sInput.at(i) != '%')
				sOutput += sInput.at(i);
			else
			{
				// 2 ki tu ke tiep = 1 so HEX
					if ((sInput.at(i) == '%') && (sInput.at(i+1) == 'u'))
					{
						if (nSize - i >=6)
						{
							szNumHex[0] = sInput.at(i + 2);
							szNumHex[1] = sInput.at(i + 3);
							szNumHex[2] = sInput.at(i + 4);
							szNumHex[3] = sInput.at(i + 5);
							if ((('0' <= szNumHex[0] && szNumHex[0] <= '9')
								|| ('a' <= szNumHex[0] && szNumHex[0] <= 'f')
								|| ('A' <= szNumHex[0] && szNumHex[0] <= 'F'))
								&&
								(('0' <= szNumHex[1] && szNumHex[1] <= '9')
								|| ('a' <= szNumHex[1] && szNumHex[1] <= 'f')
								|| ('A' <= szNumHex[1] && szNumHex[1] <= 'F'))
								&&
								(('0' <= szNumHex[2] && szNumHex[2] <= '9')
								|| ('a' <= szNumHex[2] && szNumHex[2] <= 'f')
								|| ('A' <= szNumHex[2] && szNumHex[2] <= 'F'))
								&&
								(('0' <= szNumHex[3] && szNumHex[3] <= '9')
								|| ('a' <= szNumHex[3] && szNumHex[3] <= 'f')
								|| ('A' <= szNumHex[3] && szNumHex[3] <= 'F')))

								sOutput += (char)stoi(szNumHex, nullptr, 16);
							else 
							{
								CRuntimeException *pEx =
									new CRuntimeException("Loi ham:: scUnescape (" + sInput + ")", EXCEPTIONID_EXE_FUNC_FAIL);
								throw pEx;
							}

							i += 5;
						}
						else
							sOutput += sInput.at(i);
					}
	             
					else
					{
						if (nSize - i >= 3)
						{
							szNumHex[0] = sInput.at(i + 1);
							szNumHex[1] = sInput.at(i + 2);
							if ((('0' <= szNumHex[0] && szNumHex[0] <= '9')
								|| ('a' <= szNumHex[0] && szNumHex[0] <= 'f') 
								|| ('A' <= szNumHex[0] && szNumHex[0] <= 'F'))
								&& 
								(('0' <= szNumHex[1] && szNumHex[1] <= '9') 
								|| ('a' <= szNumHex[1] && szNumHex[1] <= 'f') 
								|| ('A' <= szNumHex[1] && szNumHex[1] <= 'F')))

								sOutput += (char)stoi(szNumHex, nullptr, 16);
							else 
							{
								CRuntimeException *pEx =
									new CRuntimeException("Loi ham:: scUnescape (" + sInput+")", EXCEPTIONID_EXE_FUNC_FAIL);
								throw pEx;
							}
					
							i += 2;
						} else 
							sOutput += sInput.at(i);
						}
					}
		}
		scReturnString(sOutput);
	}
	else
		//neu kich thuoc nho hon 2 thi output = input
		scReturnString(sInput);

}

////////////////////////////////////////////////////////////////////////////////
//
// List Functions: API of object
////////////////////////////////////////////////////////////////////////////////
// -----------------------------------------------------------------------------
// API Object.dump
// -----------------------------------------------------------------------------
void scObjectDump(CVar *pVLocalStack, void *)
{
	pVLocalStack->GetParameter("this")->Trace("> ");
}

// -----------------------------------------------------------------------------
// API Object.clone
// -----------------------------------------------------------------------------
void scObjectClone(CVar *pVLocalStack, void *) {
	CVar *obj = pVLocalStack->GetParameter("this");
	pVLocalStack->GetReturnVar()->CopyValue(obj);
}

// -----------------------------------------------------------------------------
// API Math.rand
// -----------------------------------------------------------------------------
void scMathRand(CVar *pVLocalStack, void *) 
{
	double randVal = (double)rand() / RAND_MAX;
	pVLocalStack->GetReturnVar()->SetDouble(randVal);
}

// -----------------------------------------------------------------------------
// API Math.randInt
// -----------------------------------------------------------------------------
void scMathRandInt(CVar *pVLocalStack, void *) 
{
	int nMin = pVLocalStack->GetParameter("min")->GetInt();
	int nMax = pVLocalStack->GetParameter("max")->GetInt();
	int nVal = nMin + (int)(rand()%(1+nMax-nMin));
	pVLocalStack->GetReturnVar()->SetInt(nVal);
}

// -----------------------------------------------------------------------------
// API charToInt
// -----------------------------------------------------------------------------
void scCharToInt(CVar *pVLocalStack, void *) 
{
	string sChar = pVLocalStack->GetParameter("ch")->GetString();
	int nVal = 0;
	if (sChar.length()>0)
		nVal = (int)sChar.c_str()[0];
	pVLocalStack->GetReturnVar()->SetInt(nVal);
}

// -----------------------------------------------------------------------------
// setInterval(arg1, arg2);
// Doi arg2 miniseconds chay ham arg1();
// ---
// Cai dat: thuc hien ham arg1() neu la doi tuong ham
// -----------------------------------------------------------------------------
void scSetInterval(CVar *pVLocalStack, void *pUserData)
{
	int nCountArgs = 0;
	bool bIsExecute = false;
	string sCodeExe;

	CVar* pVListArgs = NULL;
	CVar* pVArrayInput = NULL;  
	CVarLink* pVLFuncCall = NULL;
	CVarLink* pVLArg = NULL;
	CProgramJs *programJs = (CProgramJs*)pUserData;
	
	pVArrayInput = pVLocalStack->GetParameter("ARRAY");
	if (pVArrayInput )
		pVLFuncCall = pVArrayInput->m_pVLFirstChild;
	
	if (pVLFuncCall && pVLFuncCall->m_pVar && pVLFuncCall->m_pVar->IsFunction())
	{
		if (!pVLFuncCall->m_pVar->IsNative()){
			sCodeExe = "(" + pVLFuncCall->m_pVar->GetStringDefineFunc() + ")(";

			// Cai dat loi goi ham
			pVLArg = pVLFuncCall->m_pNextSibling; //  pVLArg = timeWait;
			if (pVLArg)
				pVLArg = pVLArg->m_pNextSibling; // arg0, arg1, ...
			while (pVLArg && pVLArg->m_pVar)
			{
				nCountArgs++;
				if (nCountArgs != 1)
					sCodeExe += ",";

				// Thuc hien ham neu tham so la String hoac number 
				if (pVLArg->m_pVar->IsString())
					sCodeExe += "\"" + pVLArg->m_pVar->GetString() + "\"";
				else if (pVLArg->m_pVar->IsNumeric())
					sCodeExe += pVLArg->m_pVar->GetString();
				else 
					return; 
			}

			sCodeExe += ");";
			if (programJs->CheckFuncCallBackBySetInterval(pVLFuncCall->m_pVar) == FALSE)
			{
				programJs->AddFuncCallBackBySetInterval(pVLFuncCall->m_pVar);
				programJs->Execute(sCodeExe);
			}
		}
	}
}

//------------------------------------------------------------------------------
// Chuyen tu xau "str" thanh so trong he co so "radix"
// tra ve kieu so nguyen 
//------------------------------------------------------------------------------
void scParseInt(CVar *pVLocalStack, void *)
{
	string	sInput = pVLocalStack->GetParameter("str")->GetString();
	int		nRadix = pVLocalStack->GetParameter("radix")->GetInt();
	int		i = 0;
	int		nSize = sInput.size();
	int		nValue = 0;

	try
	{
		if (nRadix == 0)
		{
			// Khong co tham so radix
			nValue = stoi(sInput, 0, 0);
			scReturnInt(nValue);
		}
		else
		{
			// co tham so radix
			if (nRadix >= 2 && nRadix <= 36)
			{
				nValue = stoi(sInput, 0, nRadix);
				scReturnInt(nValue);
			}
		}
	}
	catch (invalid_argument&)	{
		scReturnString("NaN");
	}
}

//------------------------------------------------------------------------------
// API.encodeURIComponent
// Dau vao:
//	- string   
// Dau ra:
//	- string = {cac ki tu chu + % HEX (ma ASCII cua cac ki tu ko phai chu cai) } 	 
//------------------------------------------------------------------------------
void scEncodeURIComponent(CVar *pVLocalStack, void *)
{
	DWORD	dwIndex;
	char	szBuf[4] = {0};
	char	cCurrentChar;
	string	sUri;
	string	sUriEscaped = "";
	CVar*	pVUri = pVLocalStack->GetParameter("sUri");
	CVar*	pVReturn = pVLocalStack->GetReturnVar();
	const   string	sUnReserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	if (pVUri)
	{
		sUri = pVUri->GetString();
		for (dwIndex = 0; dwIndex < sUri.size(); dwIndex++)
		{
			if (sUnReserved.find_first_of(sUri[dwIndex]) != string::npos)
			{
				sUriEscaped.push_back(sUri[dwIndex]);
			}
			else
			{
				sUriEscaped.append("%");
				cCurrentChar = sUri[dwIndex];
				_snprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, "%.2X", (unsigned char)cCurrentChar);
				sUriEscaped.append(szBuf);
			}
		}
	}
	pVReturn->SetString(sUriEscaped);
}

//------------------------------------------------------------------------------
// Them 1 su kien thuc hien ham khi load trang web
//------------------------------------------------------------------------------
void scAddEventListener(CVar *pVLocalStack, void *){

}

//------------------------------------------------------------------------------
// Gan 1 su kien thuc hien ham khi load trang web
//------------------------------------------------------------------------------
void scAttachEvent(CVar *pVLocalStack, void *){

}

//------------------------------------------------------------------------------
// mau phishing.M
//------------------------------------------------------------------------------
void scAesCrtDecrypt(CVar *pVLocalStack, void *){
	string stext = scGetString("str1");
	string spass = scGetString("str2");
	if (spass == "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvxyz")
	{
		scReturnString("virus");
	}
	else
		scReturnString("no virus");

}

//------------------------------------------------------------------------------
// Lay 1 thanh phan HTML  -> tra ve doi tuong jQuery
//------------------------------------------------------------------------------
void scJQuery(CVar *pVLocalStack, void *pUserData)
{
	string		sHtmlObjectName;
	CVar*		pVHtmlElement = pVLocalStack->GetParameter("objectHtml");
	CVar*		pVReturn = pVLocalStack->GetReturnVar();
	CVar*		pVHtmlElementPrototype = NULL;
	CVarLink*	pVLInfoHtmlTag = NULL;
	CVarLink*	pVLReturnPrototype = NULL;
	CVarLink*	pVLHtmlElementPrototype = NULL;
	CProgramJs*	pProgramJs = (CProgramJs*)pUserData;

	// Them prototype -> dinh danh doi tuong lop jQuery
	pVLReturnPrototype = pProgramJs->m_pVRootStack->FindChild("jQuery");
	if (pVLReturnPrototype)
	{
		pVReturn->AddChildNoDup(STR_PROTOTYPE_CLASS, DEFAULT_ALIAS_NAME, pVLReturnPrototype->m_pVar);
	}
	// Them thong tin the jQuery neu truyen vao 1 doi tuong HTMLElement
	pVLHtmlElementPrototype = pProgramJs->m_pVRootStack->FindChild("element");
	if (pVLHtmlElementPrototype)
		pVHtmlElementPrototype = pVLHtmlElementPrototype->m_pVar;
	

	// Them thong tin bo sung cua doi tuong jQuery
	if (pVHtmlElementPrototype && pVHtmlElement->IsString())
	{
		// Them thong tin the jQuery lien ket toi neu truyen vao String
		sHtmlObjectName = pVHtmlElement->GetString();
		// Lay thong tin the
		pVReturn->AddChildNoDup(STR_INFO_OBJECT, DEFAULT_ALIAS_NAME, new CVar(sHtmlObjectName));
	}
	else if (pVHtmlElement->GetPrototypeOfObject() == pVHtmlElementPrototype){
		pVReturn->AddChildNoDup(STR_INFO_OBJECT, DEFAULT_ALIAS_NAME, pVHtmlElement);
	}

}

//------------------------------------------------------------------------------
// Ham API jQuery.ready(functionObject, arg1, arg2,...)
//------------------------------------------------------------------------------
void scJQueryReady(CVar *pVLocalStack, void *pUserData)
{
	bool		bExecute = true;
	CVar*		pVArguments = pVLocalStack->GetParameter("ARRAY");
	CVar*		pVThisObj = pVLocalStack->GetParameter("this");
	CVar*		pVDocument = NULL;
	CVar*		pVJQueryObject = NULL;

	CProgramJs*	pProgramJs = (CProgramJs*)pUserData;
	CVarLink*	pVLDocument = NULL;
	CVarLink*	pVLJQueryObject = NULL;
	CVarLink*	pVLFunction = NULL;

	// Neu doi tuong goi jQuery la document. => thuc hien ham
	// Neu khong thi bo qua khong thuc hien ham
	pVLDocument = pProgramJs->m_pVRootStack->FindChild("document");
	pVLJQueryObject = pVThisObj->FindChild(STR_INFO_OBJECT);
	if (pVLDocument)
		pVDocument = pVLDocument->m_pVar;
	if (pVDocument && pVLJQueryObject )
	{
		pVJQueryObject = pVLJQueryObject->m_pVar;
		if (pVDocument == pVJQueryObject)
		{
			// Thuc hien ham neu bien dau tien la ham
			pVLFunction = pVArguments->m_pVLFirstChild;
			if (pVLFunction && pVLFunction->m_pVar
				&& pVLFunction->m_pVar->IsFunction())
			{
				pProgramJs->Execute(pVLFunction->m_pVar->GetString());
			}
		}
	}
}

//------------------------------------------------------------------------------
// Ham API jQuery.parent() -> return jQuery(document)
//------------------------------------------------------------------------------
void scJQueryParent(CVar *pVLocalStack, void *pUserData)
{
	CVar*		pVReturn = pVLocalStack->GetReturnVar();
	CVar*		pVDocument = NULL;
	CVarLink*	pVLReturnPrototype = NULL;
	CVarLink*	pVLDocument = NULL;
	CProgramJs*	pProgramJs = (CProgramJs*)pUserData;

	// Them prototype -> dinh danh doi tuong lop jQuery
	pVLReturnPrototype = pProgramJs->m_pVRootStack->FindChild("jQuery");
	if (pVLReturnPrototype)
	{
		pVReturn->AddChildNoDup(STR_PROTOTYPE_CLASS, DEFAULT_ALIAS_NAME, pVLReturnPrototype->m_pVar);
	}
	// Them thong tin the jQuery neu truyen vao 1 doi tuong HTMLElement
	pVLDocument = pProgramJs->m_pVRootStack->FindChild("document");
	if (pVLDocument)
		pVDocument = pVLDocument->m_pVar;

	// Them thong tin bo sung cua doi tuong jQuery
	pVReturn->AddChildNoDup(STR_INFO_OBJECT, DEFAULT_ALIAS_NAME, pVDocument);
}

//------------------------------------------------------------------------------
// Ham API jQuery.mouseout(functionObject, arg1, arg2,...)
//------------------------------------------------------------------------------
void scJQueryMouseout(CVar *pVLocalStack, void *){
}

//------------------------------------------------------------------------------
// Ham API jQuery.mousemove(functionObject, arg1, arg2,...)
//------------------------------------------------------------------------------
void scJQueryMousemove(CVar *pVLocalStack, void *){
}

//------------------------------------------------------------------------------
// Ham API jQuery.mouseover(functionObject, arg1, arg2,...)
//------------------------------------------------------------------------------
void scJQueryMouseover(CVar *pVLocalStack, void *pUserData)
{
	bool		bExecute = true;
	CVar*		pVArguments = pVLocalStack->GetParameter("ARRAY");
	CVar*		pVThisObj = pVLocalStack->GetParameter("this");
	CVar*		pVJQueryObject = NULL;

	CProgramJs*	pProgramJs = (CProgramJs*)pUserData;
	CVarLink*	pVLJQueryObject = NULL;
	CVarLink*	pVLFunction = NULL;

	// Neu doi tuong goi jQuery la String ("a"). => thuc hien ham
	// Neu khong thi bo qua khong thuc hien ham
	pVLJQueryObject = pVThisObj->FindChild(STR_INFO_OBJECT);

	if ( pVLJQueryObject){
		pVJQueryObject = pVLJQueryObject->m_pVar;
		if (pVJQueryObject && pVJQueryObject->GetString() == "a")
		{
			// Thuc hien ham neu bien dau tien la ham
			pVLFunction = pVArguments->m_pVLFirstChild;
			if (pVLFunction && pVLFunction->m_pVar
				&& pVLFunction->m_pVar->IsFunction())
			{
				pProgramJs->Execute(pVLFunction->m_pVar->GetString());
			}
		}
	}
}

//------------------------------------------------------------------------------
// Ham API jQuery.hiden()
// An di doi tuong HTML
//------------------------------------------------------------------------------
void scJQueryHide(CVar *pVLocalStack, void *pUserData)
{
	bool		bExecute = true;
	bool		bIsTagHtmlHidden = false;
	string		sJQuerySelector;
	string		sInnerHTML;
	CVar*		pVThisObj = pVLocalStack->GetParameter("this");
	CVar*		pVJQueryObject = NULL;

	CVarLink*	pVLJQueryObject = NULL;
	CVarLink*	pVLHtmlElement = NULL;
	CVarLink*	pVLInnerHTML = NULL;
	CProgramJs*	pProgramJs = (CProgramJs*)pUserData;
	CRuntimeException* pRuntimeException = NULL;

	try
	{
		pVLJQueryObject = pVThisObj->FindChild(STR_INFO_OBJECT);
		if (pVLJQueryObject)
		{
			pVJQueryObject = pVLJQueryObject->m_pVar;
			if (pVJQueryObject->IsString())
			{
				sJQuerySelector = pVJQueryObject->GetString();
				if (sJQuerySelector.size() > 0)
				{
					// Tim kiem doi tuong the HTML trong file HTML
					pVLHtmlElement = pProgramJs->GetInfoHtmlElementFromJQuerySelector(sJQuerySelector);
				}
			}
		}
	}
	catch(CRuntimeException* pRE)
	{
		pRuntimeException = pRE;
	}
	catch (exception&)
	{
		// khong xu ly
	}

	SAFE_THROW(pRuntimeException);
}

//------------------------------------------------------------------------------
// Ham API jQuery.mouseover(functionObject, arg1, arg2,...)
//------------------------------------------------------------------------------
void scJQueryShow(CVar *pVLocalStack, void *)
{
	// do Nothing
}


//------------------------------------------------------------------------------
// constructor string
// tao lap doi tuong string voi tham so dau vao
//------------------------------------------------------------------------------
void scStringConstructor(CVar *pVLocalStack, void *)
{
	string sData = scGetString("str");
	pVLocalStack->GetParameter("this")->SetString(sData);
	scReturnString(sData);
}
//------------------------------------------------------------------------------
// RexExp(str1,str2)
// tao lap doi tuong regex voi tham so dau vao
//------------------------------------------------------------------------------

void scRegExp(CVar *pVLocalStack, void *)
{
	string sDataRegex = scGetString("str1");
	string sOptionRegex = scGetString("str1");
// 	pVLocalStack->GetParameter("this")->SetString(sData);
	scReturnRegex(sDataRegex);
}
//------------------------------------------------------------------------------
// String.indexof(). Tim vi tri cua 1 xau String trong doi tuong string
//------------------------------------------------------------------------------
void scStringIndexOf(CVar *pVLocalStack, void *) 
{
	string sThis = pVLocalStack->GetParameter("this")->GetString();
	string sSearch = pVLocalStack->GetParameter("search")->GetString();
	int nPos = sThis.find(sSearch);
	int nVal = (nPos==string::npos) ? -1 : nPos;
	pVLocalStack->GetReturnVar()->SetInt(nVal);
}

//------------------------------------------------------------------------------
// String.subString(lo, hi). Sinh 1 xau con tu 1 xau string co truoc
//------------------------------------------------------------------------------
void scStringSubstring(CVar *pVLocalStack, void *)
{
	string sThis = pVLocalStack->GetParameter("this")->GetString();
	int nLo = pVLocalStack->GetParameter("lo")->GetInt();
	int nHi = pVLocalStack->GetParameter("hi")->GetInt();
	int nSizeSzParrent = sThis.length();

	if (nLo < 0)
		nLo = 0;
	try
	{
		pVLocalStack->GetReturnVar()->SetString(sThis.substr(nLo, nHi-nLo));
	}
	catch (out_of_range&)
	{
		pVLocalStack->GetReturnVar()->SetString("");
	}
}

//------------------------------------------------------------------------------
// String.subString(lo, length). Sinh 1 xau con tu 1 xau string co truoc
//------------------------------------------------------------------------------
void scStringSubstr(CVar *pVLocalStack, void *)
{
	string sThis = pVLocalStack->GetParameter("this")->GetString();
	int nLo = pVLocalStack->GetParameter("lo")->GetInt();
	CVar* pVLength = pVLocalStack->GetParameter("length");
	int nLength = pVLength->GetInt();
	int nSizeSzParrent = sThis.length();

	if (nLo < 0){
		nLo = nLo + nSizeSzParrent;
		if (nLo < 0)
			nLo = 0;
	}
	if (pVLength->IsUndefined())
		nLength = nSizeSzParrent;
	try
	{
		pVLocalStack->GetReturnVar()->SetString(sThis.substr(nLo, nLength));
	}
	catch (out_of_range&)
	{
		pVLocalStack->GetReturnVar()->SetString("");
	}
}

//------------------------------------------------------------------------------
// String.charAt(pos). Xuat ki tu o vi tri pos cua xau string
//------------------------------------------------------------------------------
void scStringCharAt(CVar *pVLocalStack, void *) {
	string sThis = pVLocalStack->GetParameter("this")->GetString();
	int nPos = pVLocalStack->GetParameter("pos")->GetInt();
	if (nPos>=0 && nPos<(int)sThis.length())
		pVLocalStack->GetReturnVar()->SetString(sThis.substr(nPos, 1));
	else
		pVLocalStack->GetReturnVar()->SetString("");
}

//------------------------------------------------------------------------------
// String.charCodeAt(pos). Chuyen 1 ki tu chu cai -> ki tu so theo bang ma ASCII
//------------------------------------------------------------------------------
void scStringCharCodeAt(CVar *pVLocalStack, void *) 
{
	string sThis = pVLocalStack->GetParameter("this")->GetString();
	int nPos = pVLocalStack->GetParameter("pos")->GetInt();
	if (nPos >= 0 && nPos < (int)sThis.length())
	{
 //		const char c_this = sThis.at(nPos);
 //		unsigned char chiennd = c_this;
		pVLocalStack->GetReturnVar()->SetInt((unsigned char)(sThis.at(nPos)));

	}
	else
		pVLocalStack->GetReturnVar()->SetInt(0);
}

//------------------------------------------------------------------------------
// API: String.split
//------------------------------------------------------------------------------
void scStringSplit(CVar *pVLocalStack, void *)
{
	string sThis = pVLocalStack->GetParameter("this")->GetString();
	string sSep = pVLocalStack->GetParameter("separator")->GetString();
	int nSizeSep = sSep.size();
	int nSizeThisStr = sThis.size();
	CVar *pVResult = pVLocalStack->GetReturnVar();
	size_t nPos = 0;

	pVResult->SetArray();
	int nLength = 0;

	if (nSizeThisStr > 0 && nSizeSep > 0)
	{
		nPos = sThis.find(sSep);
		while (sThis.size() > 0 && nPos != string::npos) 
		{
			pVResult->SetArrayIndex(nLength++, new CVar(sThis.substr(0, nPos)));
			sThis = sThis.substr(nPos + nSizeSep);
			nPos = sThis.find(sSep);
		}

	}

	pVResult->SetArrayIndex(nLength++, new CVar(sThis));
}

//------------------------------------------------------------------------------
// API: String.replace
//------------------------------------------------------------------------------
void scStringReplace(CVar *pVLocalStack, void *)
{
	CVar* pVsearchvalue = pVLocalStack->GetParameter("searchvalue");
	string sNewValue = scGetString("newvalue");
	string sData = scGetString("this");
	regex regexCheckComment;
	string sThisOut = "";
	if (pVsearchvalue&& pVsearchvalue->IsString())
	{
		string sSearchValue = pVsearchvalue->GetString();
		int nPosFound = sData.find(sSearchValue);
		if (nPosFound != string::npos){
			scReturnString(sData.replace(nPosFound, sSearchValue.size(), sNewValue));
		}
		else 
		{
			scReturnString(sData);
		}
	}
	else if (pVsearchvalue&& pVsearchvalue->IsRegex())
	{ 
		string sSearchValue = pVsearchvalue->GetRegex();
		try
		{
			
			regexCheckComment = regex(sSearchValue);
			//tim kiem string theo bieu thuc quy tac
			regex_replace(std::back_inserter(sThisOut), sData.begin(), sData.end(), regexCheckComment, sNewValue);
		}
		catch (...)
		{
			DbgPrintLnA("Create Regex Err");
		}
		scReturnString(sThisOut);
	}
	else
		scReturnString(sData);

	
}

//------------------------------------------------------------------------------
// str1.concat(str2, str3,...);
// Noi cac xau string str1 = str1+str2+str3+...
//------------------------------------------------------------------------------
void scStringConcat(CVar *pVLocalStack, void *)
{
	string sThisObj = scGetString("this");
	CVar *pVArrConcat = pVLocalStack->GetParameter("ARRAY");
	CVarLink* pVLArgument = NULL;

	pVLArgument = pVArrConcat->m_pVLFirstChild;
	while (pVLArgument)
	{
		// gan lan luot tat ca cac string cua cac Argument vao szThisObj
		if (pVLArgument->m_pVar)
			sThisObj += pVLArgument->m_pVar->GetString();
		pVLArgument = pVLArgument->m_pNextSibling;
	}

	scReturnString(sThisObj);
}

//------------------------------------------------------------------------------
// str1.match(/str/);
// str1.match(/str/i);
// str1.match(/str/g);
// str1.match(/str/gi);
//------------------------------------------------------------------------------

void scStringMatch(CVar *pVLocalStack, void *)
{
	string sThis; 
	string sRegex;
	string sThisOut = "";
	sThis = pVLocalStack->GetParameter("this")->GetString();
	sRegex = pVLocalStack->GetParameter("regex")->GetRegex();
	regex regexCheckComment;
	
	try
	{
		regexCheckComment = regex(sRegex);
		//tim kiem string theo bieu thuc quy tac
		std::sregex_iterator next(sThis.begin(), sThis.end(), regexCheckComment);
		std::sregex_iterator end;
		if (next != end)
			sThisOut = next->str();
	}
	catch (...)
	{
		DbgPrintLnA("Create Regex Err");
	}

	scReturnString(sThisOut);
}

//------------------------------------------------------------------------------
// Chuyen mang Args => xau string: String.fromCharCode(65, 66, 67);  // "ABC"
//  String.fromCharCode("65", "66", "67");  // "ABC"
//------------------------------------------------------------------------------
void scStringFromCharCode(CVar *pVLocalStack, void *)
{
	int nChar = 0;
	string sReturn;
	string sValNum;
	CVar *pVArrConcat = pVLocalStack->GetParameter("ARRAY");
	CVarLink* pVLArgument = NULL;

	pVLArgument = pVArrConcat->m_pVLFirstChild;
	while (pVLArgument){
		nChar = 0;
		// gan lan luot tat ca cac string cua cac Argument vao szThisObj
		if (pVLArgument->m_pVar){
			if (pVLArgument->m_pVar->IsNumeric())
			{
				// Neu la so => moi so la 1 ki tu ASCII
				nChar = pVLArgument->m_pVar->GetInt();
			}
			else if (pVLArgument->m_pVar->IsString())
			{
				// Neu la String -> convert => so => chuyen thanh ki tu
				sValNum = pVLArgument->m_pVar->GetString();
				if (sValNum.size() > 0)
				{
					try
					{
						// tham so thu 3 = 0 tu dong convert => Dec
						nChar = stoi(sValNum, 0);
					}
					catch (exception&)
					{
						nChar = 0;
					}
				}
				else
					nChar = 0;
			}
			if (nChar != 0)
				sReturn += (char)nChar;
		}
		pVLArgument = pVLArgument->m_pNextSibling;
	}

	scReturnString(sReturn);
}

//------------------------------------------------------------------------------
// API: Integer.parseInt(str):: chuyen 1 xau string -> kieu Int
//------------------------------------------------------------------------------
void scIntegerParseInt(CVar *pVLocalStack, void *) 
{
	string sStr = pVLocalStack->GetParameter("str")->GetString();
	long nVal = strtol(sStr.c_str(),0,0);
	pVLocalStack->GetReturnVar()->SetInt(nVal);
}

//------------------------------------------------------------------------------
// API: Integer.Valueof(str):: chuyen 1 xau string -> kieu Int
//------------------------------------------------------------------------------
void scIntegerValueOf(CVar *pVLocalStack, void *) 
{
	string sStr = pVLocalStack->GetParameter("str")->GetString();

	int nVal = 0;
	if (sStr.length()==1)
		nVal = sStr[0];
	pVLocalStack->GetReturnVar()->SetInt(nVal);
}

//------------------------------------------------------------------------------
// API: JSON.Stringify:: xuat tap cac doi tuong dang JSON
//------------------------------------------------------------------------------
void scJSONStringify(CVar *pVLocalStack, void *) 
{
	ostringstream sResult;
	pVLocalStack->GetParameter("obj")->GetJSON(sResult);
	pVLocalStack->GetReturnVar()->SetString(sResult.str());
}

//------------------------------------------------------------------------------
// Exec(jsCode);
// thuc hien jsCode trong tham so dau vao ham Exec
//------------------------------------------------------------------------------
void scExec(CVar *pVLocalStack, void *pUserData) 
{
	CProgramJs *pFileJS = (CProgramJs *)pUserData;
	string sJsCode = pVLocalStack->GetParameter("jsCode")->GetString();
	pFileJS->Execute(sJsCode);
}

//------------------------------------------------------------------------------
// var xxx = Eval(jsCode);
// thuc hien jsCode trong tham so dau vao ham Eval + co gia tri tra ve 
//------------------------------------------------------------------------------
void scEval(CVar *pVLocalStack, void *pUserData) 
{
	CVarLink* pVLReturn = NULL;
	CProgramJs *fileJS = (CProgramJs *)pUserData;
	string sJsCode = pVLocalStack->GetParameter("jsCode")->GetString();
	CRuntimeException* pREHadVirus = NULL;
	DWORD dwVirusType = 0;
	const char* args[] = 
	{
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
	};
	vector<string> lstVLDownloaderVirus(args, args + 1);

		for (size_t idxDownloaderVirus = 0; idxDownloaderVirus < lstVLDownloaderVirus.size(); idxDownloaderVirus++)
		{
			if (sJsCode.find(lstVLDownloaderVirus.at(idxDownloaderVirus)) != string::npos)
			{
				g_sstrDetectDownloader->bString = true;
			}
		}
		//check virus đoạn mã trong eval
		/*dwVirusType = fileJS->CheckVirusEval(sJsCode);
		if (dwVirusType)
		{
			if (dwVirusType==VR_TYPE_DOWNLOADER)
			{
				pREHadVirus = new CRuntimeException(VR_NAME_DOWNLOADER, EXCEPTIONID_FOUND_VIRUS, false);
				pREHadVirus->SetVirusName(VR_NAME_DOWNLOADER);
			}
			else if (dwVirusType == VR_TYPE_EVAL)
			{
				pREHadVirus = new CRuntimeException(VR_NAME_EVAL, EXCEPTIONID_FOUND_VIRUS, false);
				pREHadVirus->SetVirusName(VR_NAME_EVAL);
			}
			else
			{
				pREHadVirus = new CRuntimeException(VR_NAME_REDIRECT, EXCEPTIONID_FOUND_VIRUS, false);
				pREHadVirus->SetVirusName(VR_NAME_REDIRECT);
			}
			throw pREHadVirus;
		}*/


	if (sJsCode.size() > 0)
	{
		pVLReturn = fileJS->EvaluateComplex(sJsCode);
		if (pVLReturn)
			pVLocalStack->SetReturnVar(pVLReturn->m_pVar);
	}
}

//------------------------------------------------------------------------------
// Dua ra hop thoai thong bao cho nguoi dung
//------------------------------------------------------------------------------
void scAlert(CVar *pVLocalStack, void *)
{
	// Co 1 tham so sMsg. Ham nay ko cai dat.
	return;
}

//------------------------------------------------------------------------------
// API:  Array.contain
//------------------------------------------------------------------------------
void scArrayContains(CVar *pVLocalStack, void *pUserData)
{
	CVar *pVObj = pVLocalStack->GetParameter("obj");
	CVarLink *pVLChild = pVLocalStack->GetParameter("this")->m_pVLFirstChild;
	bool bContains = false;

	while (pVLChild) 
	{
		if (pVLChild->m_pVar->Equals(pVObj)) 
		{
			bContains = true;
			break;
		}
		pVLChild = pVLChild->m_pNextSibling;
	}

	pVLocalStack->GetReturnVar()->SetInt(bContains);
}

//------------------------------------------------------------------------------
// API: Array.remove
//------------------------------------------------------------------------------
void scArrayRemove(CVar *pVLocalStack, void *pUserData) 
{
	int nIndex = 0;
	int nNewIdx;
	size_t i = 0;
	CVar *pVObj = pVLocalStack->GetParameter("obj");
	vector<int> lstRemovedIndices;
	CVarLink *pVLFirstChildOfThis;

	// remove
	pVLFirstChildOfThis = pVLocalStack->GetParameter("this")->m_pVLFirstChild;
	while (pVLFirstChildOfThis) 
	{
		if (pVLFirstChildOfThis->m_pVar->Equals(pVObj)) 
		{
			lstRemovedIndices.push_back(pVLFirstChildOfThis->GetIntName());
		}
		pVLFirstChildOfThis = pVLFirstChildOfThis->m_pNextSibling;
	}
	// renumber
	pVLFirstChildOfThis = pVLocalStack->GetParameter("this")->m_pVLFirstChild;
	while (pVLFirstChildOfThis) 
	{
		nIndex = pVLFirstChildOfThis->GetIntName();
		nNewIdx = nIndex;
		for (i = 0; i < lstRemovedIndices.size(); i++)
		if (nIndex >= lstRemovedIndices[i])
			nNewIdx--;

		if (nNewIdx != nIndex)
			pVLFirstChildOfThis->SetIntName(nNewIdx);
		pVLFirstChildOfThis = pVLFirstChildOfThis->m_pNextSibling;
	}
}

//------------------------------------------------------------------------------
// API: Array.join
//------------------------------------------------------------------------------
void scArrayJoin(CVar *pVLocalStack, void *pUserData) 
{
	string sSep = pVLocalStack->GetParameter("separator")->GetString();
	CVar *pVArr = pVLocalStack->GetParameter("this");
	ostringstream sstr;
	int nLength = pVArr->GetArrayLength();
	int i = 0;

	for (i = 0; i<nLength; i++) 
	{
		if (i>0) sstr << sSep;
		sstr << pVArr->GetArrayIndex(i)->GetString();
	}

	pVLocalStack->GetReturnVar()->SetString(sstr.str());
}

//------------------------------------------------------------------------------
// API: Array.reverse : Hàm đảo ngược xâu
//------------------------------------------------------------------------------
void scArrayReverse(CVar *pVLocalStack, void *pUserData) 
{
	string sSep;
	string pVArr = pVLocalStack->GetParameter("this")->GetString();
	ostringstream sstr;
	int nLength = pVArr.size();
	int j = nLength - 1;
	int nLengthNull = 0;
	int i = 0;
	CVar *pVResult = pVLocalStack->GetReturnVar();
	pVResult->SetArray();
	while (i <= j)
	{
		char tmp = pVArr[i];
		pVArr[i] = pVArr[j];
		pVArr[j] = tmp;
		i++;
		j--;
	}

	pVResult->SetArrayIndex(nLengthNull++, new CVar(pVArr));
	//pVLocalStack->GetReturnVar()->SetString(pVArr);
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// API: Array.constructor : Ham khoi tao 1 doi tuong Array moi
//------------------------------------------------------------------------------
void scArrayConstructor(CVar *pVLocalStack, void *)
{
	CVar *pVArray = pVLocalStack->GetParameter("ARRAY");
	pVLocalStack->FindChildOrCreate("this")->ReplaceWith(pVArray);
	pVLocalStack->SetReturnVar(pVArray);
}


//------------------------------------------------------------------------------
// thuc hien ham hien tai
// Doi tuong ham = obj
// ds tham so = arr
//------------------------------------------------------------------------------
void scFunctionApply(CVar *pVLocalStack, void *)
{
	CVar *pVThisData = pVLocalStack->GetParameter("obj");
	CVar *pVArgs = pVLocalStack->GetParameter("arr");

	int t = 1;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Math Funtions

//------------------------------------------------------------------------------
//Math.abs(x) - returns absolute of given value
//------------------------------------------------------------------------------
void scMathAbs(CVar *pVLocalStack, void *pUserData) 
{
	if ( scIsInt("a") ) 
	{
		scReturnInt( F_ABS( scGetInt("a") ) );
	} 
	else if ( scIsDouble("a") ) 
	{
		scReturnDouble( F_ABS( scGetDouble("a") ) );
	}
}

//------------------------------------------------------------------------------
//Math.round(a) - returns nearest round of given value
//------------------------------------------------------------------------------
void scMathRound(CVar *pVLocalStack, void *pUserData)
{
	if ( scIsInt("a") ) 
	{
		scReturnInt( F_ROUND( scGetInt("a") ) );
	}
	else if ( scIsDouble("a") )
	{
		scReturnDouble( F_ROUND( scGetDouble("a") ) );
	}
}

//------------------------------------------------------------------------------
//Math.ceil(a) - returns nearest larger of given value
//------------------------------------------------------------------------------
void scMathCeil(CVar *pVLocalStack, void *pUserData)
{
	if (scIsInt("a")) 
	{
		scReturnInt(scGetInt("a"));
	}
	else if (scIsDouble("a")) 
	{
		scReturnDouble(ceil(scGetDouble("a")));
	}
}

//------------------------------------------------------------------------------
//Math.min(a,b) - returns minimum of two given values 
//------------------------------------------------------------------------------
void scMathMin(CVar *pVLocalStack, void *pUserData) 
{
	if ( (scIsInt("a")) && (scIsInt("b")) ) 
	{
		scReturnInt( F_MIN( scGetInt("a"), scGetInt("b") ) );
	}
	else 
	{
		scReturnDouble(F_MIN(scGetDouble("a"), scGetDouble("b")));
	}
}

//Math.max(a,b) - returns maximum of two given values  
void scMathMax(CVar *pVLocalStack, void *pUserData) 
{
	if ( (scIsInt("a")) && (scIsInt("b")) ) 
	{
		scReturnInt( F_MAX( scGetInt("a"), scGetInt("b") ) );
	} 
	else 
	{
		scReturnDouble( F_MAX( scGetDouble("a"), scGetDouble("b") ) );
	}
}

//------------------------------------------------------------------------------
//Math.range(x,a,b) - returns value limited between two given values  
//------------------------------------------------------------------------------
void scMathRange(CVar *pVLocalStack, void *pUserData)
{
	if ( (scIsInt("x")) ) 
	{
		scReturnInt( F_RNG( scGetInt("x"), scGetInt("a"), scGetInt("b") ) );
	} 
	else 
	{
		scReturnDouble( F_RNG( scGetDouble("x"), scGetDouble("a"), scGetDouble("b") ) );
	}
}


//------------------------------------------------------------------------------
//Math.sign(a) - returns sign of given value (-1==negative,0=zero,1=positive)
//------------------------------------------------------------------------------
void scMathSign(CVar *pVLocalStack, void *pUserData)
{
	if ( scIsInt("a") ) 
	{
		scReturnInt( F_SGN( scGetInt("a") ) );
	} 
	else if ( scIsDouble("a") ) 
	{
		scReturnDouble( F_SGN( scGetDouble("a") ) );
	}
}


//------------------------------------------------------------------------------
//Math.PI() - returns PI value
//------------------------------------------------------------------------------
void scMathPI(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble(k_PI);
}

//------------------------------------------------------------------------------
//Math.toDegrees(a) - returns degree value of a given angle in radians
//------------------------------------------------------------------------------
void scMathToDegrees(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( (180.0/k_PI)*( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.toRadians(a) - returns radians value of a given angle in degrees
//------------------------------------------------------------------------------
void scMathToRadians(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( (k_PI/180.0)*( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.sin(a) - returns trig. sine of given angle in radians
//------------------------------------------------------------------------------
void scMathSin(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( sin( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.asin(a) - returns trig. arcsine of given angle in radians
//------------------------------------------------------------------------------
void scMathASin(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( asin( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.cos(a) - returns trig. cosine of given angle in radians
//------------------------------------------------------------------------------
void scMathCos(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( cos( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.acos(a) - returns trig. arccosine of given angle in radians
//------------------------------------------------------------------------------
void scMathACos(CVar *pVLocalStack, void *pUserData) {
	scReturnDouble( acos( scGetDouble("a") ) );
}


//------------------------------------------------------------------------------
//Math.tan(a) - returns trig. tangent of given angle in radians
//------------------------------------------------------------------------------
void scMathTan(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( tan( scGetDouble("a") ) );
}


//------------------------------------------------------------------------------
//Math.atan(a) - returns trig. arctangent of given angle in radians
//------------------------------------------------------------------------------
void scMathATan(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( atan( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.sinh(a) - returns trig. hyperbolic sine of given angle in radians
//------------------------------------------------------------------------------
void scMathSinh(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( sinh( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.asinh(a) - returns trig. hyperbolic arcsine of given angle in radians
//------------------------------------------------------------------------------
void scMathASinh(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( asinh( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.cosh(a) - returns trig. hyperbolic cosine of given angle in radians
//------------------------------------------------------------------------------
void scMathCosh(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( cosh( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.acosh(a) - returns trig. hyperbolic arccosine of given angle in radians
//------------------------------------------------------------------------------
void scMathACosh(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( acosh( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.tanh(a) - returns trig. hyperbolic tangent of given angle in radians
//------------------------------------------------------------------------------
void scMathTanh(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( tanh( scGetDouble("a") ) );
}


//------------------------------------------------------------------------------
//Math.atan(a) - returns trig. hyperbolic arctangent of given angle in radians
//------------------------------------------------------------------------------
void scMathATanh(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( atan( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.E() - returns E Neplero value
//------------------------------------------------------------------------------
void scMathE(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble(k_E);
}

//------------------------------------------------------------------------------
//Math.log(a) - returns natural logaritm (base E) of given value
//------------------------------------------------------------------------------
void scMathLog(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( log( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.log10(a) - returns logaritm(base 10) of given value
//------------------------------------------------------------------------------
void scMathLog10(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( log10( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.exp(a) - returns e raised to the power of a given number
//------------------------------------------------------------------------------
void scMathExp(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( exp( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
//Math.pow(a,b) - returns the result of a number raised to a power (a)^(b)
//------------------------------------------------------------------------------
void scMathPow(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( pow( scGetDouble("a"), scGetDouble("b") ) );
}
//------------------------------------------------------------------------------
//Math.floor(a) - Round a number downward to its nearest integer
//------------------------------------------------------------------------------
void scMathFloor(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble(floor(scGetDouble("a")));
}

//------------------------------------------------------------------------------
//Math.sqr(a) - returns square of given value
//------------------------------------------------------------------------------
void scMathSqr(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( ( scGetDouble("a") * scGetDouble("a") ) );
}


//------------------------------------------------------------------------------
//Math.sqrt(a) - returns square m_pRootStack of given value
//------------------------------------------------------------------------------
void scMathSqrt(CVar *pVLocalStack, void *pUserData)
{
	scReturnDouble( sqrt( scGetDouble("a") ) );
}

//------------------------------------------------------------------------------
// 2 exception:
//
// + Id = EXCEPTIONID_WRITE_SCRIPT: Khong lay duoc doi tuong CFileHtml*, 
//			thuoc tinh cua CProgramJs::pFileHtmlProcess
//
// + Id = EXCEPTIONID_WRITE_SMALL_IFRAME: Ghi <Iframe> kich thuoc nho
//------------------------------------------------------------------------------
void scDocumentWrite(CVar*pVLocalStack, void *pUserData)
{
	bool bIsValidate = false; // Co bao them 1 the hop le vao file html
	DWORD dwTypeVirus = 0;
	int nPosTagOpenEnd = 0;
	int nPosTagCloseEnd = 0;
	int nPosTagCloseBegin = 0;

	string sHtmlCode(scGetString("str"));
	string sOutHtml = "";
	string sDocumentData;
	string sCodeJS;
	string sNewTagName;
	string sNewTagId;

	map<string, string> mapHtmlElement;	// Thong tin the them vao html
	
	CProgramJs *programJs = (CProgramJs*)pUserData;
	CFileHtml* pFileHtmlProcess = NULL;
	CRuntimeException* pREHadVirus = NULL;
	
	// write new code JS to "CFileHtml::m_szDocumentData"
	pFileHtmlProcess = programJs->GetMemberCFileHtml();
// 	if( pFileHtmlProcess == NULL)
// 		throw new CRuntimeException("CFileHtml::m_szDocumentData ERROR to Write new Code!!", 
// 			EXCEPTIONID_WRITE_SCRIPT);
	
	if (sHtmlCode.size() > 0)
	{
//		std::transform(sHtmlCode.begin(), sHtmlCode.end(), sHtmlCode.begin(), ::tolower);
		pFileHtmlProcess->AppendDocumentData(sHtmlCode);
	}
	//if (sHtmlCode == "virus") //???
	//{
	//	pREHadVirus = new CRuntimeException(VR_NAME_REDIRECT_QE, EXCEPTIONID_FOUND_VIRUS, false);
	//	pREHadVirus->SetVirusName(VR_NAME_REDIRECT_QE);
	//	throw pREHadVirus;
	//}

	// Neu code html ghi vao la 1 the HTML co thuoc tinh Id vao => them phan tu 
	// con vao doi tuong document.body
	do{
		// Kiem tra cap the <nameTag> </nameTag>
		if (sHtmlCode.size() > 0 && sHtmlCode.at(0) == '<')
		{
			sNewTagName = GetFirstString(sHtmlCode, 1);
			nPosTagOpenEnd = sHtmlCode.find('>');
			if (nPosTagOpenEnd != string::npos && sNewTagName.size() > 0)
			{
				// kiem tra ten the mo va ten the dong dau tien co trung nhau hay ko
				nPosTagCloseBegin = sHtmlCode.find("</", nPosTagOpenEnd + 1);
				if (nPosTagCloseBegin != string::npos && (sNewTagName == GetFirstString(sHtmlCode, nPosTagCloseBegin+2)))
				{
					nPosTagCloseEnd = sHtmlCode.find('>', nPosTagCloseBegin + 1);
					if (nPosTagCloseEnd != string::npos);
					else break;
				}
				else break;
			}
			else break;
		}
		else break;


		// Ghi vao doi tuong document.body 1 HTML element
		mapHtmlElement = pFileHtmlProcess->GetInfoElement(sHtmlCode, bIsValidate);
		
		if (bIsValidate)
		{
			// Kiem tra virus inject Script Adsware
			/*if (programJs->CheckVirusInsertScriptTag(mapHtmlElement))
			{
				pREHadVirus = new CRuntimeException(VR_NAME_INJECT_SCRIPT_ADSWARE, EXCEPTIONID_FOUND_VIRUS, false);
				pREHadVirus->SetVirusName(VR_NAME_INJECT_SCRIPT_ADSWARE);
				throw pREHadVirus;
			}*/

			sNewTagName = mapHtmlElement.at("tagName");
			sNewTagId = mapHtmlElement.at("id");
			// Neu co thuoc tinh id thi moi thuc hien them phan tu con vao 'body'
			if (sNewTagName.size() > 0 && sNewTagId.size()>0)
			{
				sCodeJS = "document.body." + sNewTagName + "=element.createElement('" + sNewTagName + "');"
					+ "\ndocument.body." + sNewTagName + ".id = '" + sNewTagId + "';";
// 				sCodeJS = "var chiennd =element.createElement('" + sNewTagName + "');"
// 					+ "\nchiennd" + ".id = '" + sNewTagId + "';";
				programJs->Execute(sCodeJS);
			}
		}
	} while (false);

	if (sHtmlCode.find("</script>") != string::npos)
	{
		sCodeJS = pFileHtmlProcess->FilterOutCodeJsInDocument();
	}
	// run new code scipt if exist

	if (sCodeJS.size() > 0)
	{
		g_sstrDetectVirusOther->bDocumentWriteRedirect = true;
		programJs->Execute(sCodeJS);
	}
	
}


//------------------------------------------------------------------------------
// Tao doi tuong HTML Element
//------------------------------------------------------------------------------
void scElementCreateElement(CVar*pVLocalStack, void *pUserData)
{
	CVarLink* pVLTmp = NULL;
	CVar* pVNewHtmlElement = NULL;
	string sNameElement = scGetString("szNameElement");
	CProgramJs *pProgramJs = (CProgramJs*)pUserData;

	// tim doi tuong element
	pVLTmp = pProgramJs->m_pVRootStack->FindChild("element");
	// copy doi tuong element voi toan bo thuoc tinh
	pVNewHtmlElement = new CVar(STR_BLANK_DATA, SCRIPTVAR_OBJECT);
	// cai dat ten cho thanh phan element
	pVNewHtmlElement->AddChild(STR_PROTOTYPE_CLASS, DEFAULT_ALIAS_NAME, pVLTmp->m_pVar);

	transform(sNameElement.begin(), sNameElement.end(), sNameElement.begin(), ::toupper);
	pVNewHtmlElement->AddChild("tagName", DEFAULT_ALIAS_NAME, new CVar(sNameElement));

	pVLocalStack->SetReturnVar(pVNewHtmlElement);

}

void scElementSetAttribute(CVar*pVLocalStack, void *pUserData);
//------------------------------------------------------------------------------
// Lay thanh phan Html element la con cua doi tuong goi phuong thuc nay
// voi thuoc tinh id = 'szId'
//------------------------------------------------------------------------------
void scElementGetElementById(CVar*pVLocalStack, void *pUserData)
{
	int i = 0;
	int nSizeLst = 0;
	bool bFoundElement = false;
	string	sId = scGetString("szId");
	string sElementHtml, sInnerHtml, sAttrsHtml, sTagName;
	
	CVar*	pVThisObject	= pVLocalStack->GetParameter("this");
	CVar*	pVNewStack = NULL;
	CVar*	pVNewElement = NULL;
	CProgramJs* pProgramJs	= (CProgramJs*)pUserData;
	CVarLink* pVLTmpSearch	= NULL;
	CVarLink* pVLTmpId = NULL;
	vector<CVarLink*> lstHTMLElement;

	scReturnInt(0);
	if (pVThisObject == NULL || sId.size() == 0)
		return;
	
	lstHTMLElement = pProgramJs->GetAllChildHtmlElement(pVThisObject, lstHTMLElement);

	nSizeLst = lstHTMLElement.size();
	
	for (i= 0; i < nSizeLst;i++)
	{
		pVLTmpSearch = lstHTMLElement.at(i);
		// Lan luot tim trong danh sach tung phan tu con
		if (pVLTmpSearch && pVLTmpSearch->m_pVar)
		{
			// Tim thuoc tinh Id trong doi tuong HTML element
			pVLTmpId = pVLTmpSearch->m_pVar->FindChild("id");
			if (pVLTmpId && pVLTmpId->m_pVar &&
				pVLTmpId->m_pVar->GetString() == sId)
			{
				// cai dat gia tri tra ve = HTML element 
				pVLocalStack->SetReturnVar(pVLTmpSearch->m_pVar);
				bFoundElement = true;
				break;
			}
		}
	}
	lstHTMLElement.clear();

	if (bFoundElement == false)
	{
		// Tim kiem phan tu tren file html
		sElementHtml = pProgramJs->GetMemberCFileHtml()
			->GetElementById(sId, sAttrsHtml, sInnerHtml, sTagName);
		
		// Neu ko lay duoc thong tin
		if (sElementHtml.size() == 0 || sTagName.size() == 0) goto _EXIT_FUNTION;

		pVNewStack = new CVar();
		pVNewStack->AddChild("szNameElement", DEFAULT_ALIAS_NAME, new CVar(sTagName));
		
		// Gia lap 1 doi tuong moi
		scElementCreateElement(pVNewStack, pProgramJs);
		pVNewElement = pVNewStack->GetParameter("return");

		// Them thuoc tinh Id cho doi tuong Element
		pVNewElement->AddChildNoDup("id", DEFAULT_ALIAS_NAME, new CVar(sId));
		pVNewElement->AddChildNoDup("innerHTML", DEFAULT_ALIAS_NAME, new CVar(sInnerHtml));

		// Kiem tra virus cua doi tuong html nay

		// Them doi tuong vao file document chuan
		pVLTmpSearch = pProgramJs->m_pVRootStack->FindChild("document");
		if (pVLTmpSearch && pVLTmpSearch->m_pVar)
			pVLTmpSearch = pVLTmpSearch->m_pVar->FindChild("body");
		if (pVLTmpSearch && pVLTmpSearch->m_pVar)
		{
			pVLTmpSearch->m_pVar->AddChildNoDup(sTagName, DEFAULT_ALIAS_NAME, pVNewElement);
		}

		pVLocalStack->SetReturnVar(pVNewElement);
	}

_EXIT_FUNTION:
	return;
}

//------------------------------------------------------------------------------
// Lay danh sach "Html element con" 
// voi thuoc tinh tagName = 'szTagName'
//------------------------------------------------------------------------------
void scElementGetElementsByTagName(CVar*pVLocalStack, void *pUserData)
{
	int i = 0;
	int nSizeLstHtmlElement = 0;
	int nIndexInArrayReturn = 0;

	string sTagName	= scGetString("szTagName");
	CVar* pVThisObject	= pVLocalStack->GetParameter("this");
	CVar* pVReturn		= pVLocalStack->GetParameter(STR_RETURN_VAR);
	CVarLink* pVLTmpElement = NULL;
	CVarLink* pVLTmpTagName = NULL;
	CVarLink* pVLNewElement = NULL;

	CProgramJs* pProgramJs	= (CProgramJs*)pUserData;
	vector<CVarLink*> lstHTMLElement;
	
	pVReturn->SetArray();
	if (pVThisObject == NULL || sTagName.size() == 0)
		return;
	transform(sTagName.begin(), sTagName.end(), sTagName.begin(), ::toupper);


	lstHTMLElement = pProgramJs->GetAllChildHtmlElement(pVThisObject, lstHTMLElement);
	nSizeLstHtmlElement = lstHTMLElement.size();
	for (i = 0; i < nSizeLstHtmlElement; i++)
	{
		pVLTmpElement = lstHTMLElement.at(i);
		// Lan luot tim trong danh sach tung phan tu con
		if (pVLTmpElement && pVLTmpElement->m_pVar)
		{
			// Tim thuoc tinh Id trong doi tuong HTML element
			pVLTmpTagName = pVLTmpElement->m_pVar->FindChild("tagName");
		
			if (pVLTmpTagName && pVLTmpTagName->m_pVar &&
				pVLTmpTagName->m_pVar->GetString() == sTagName)
			{
				// cai dat gia tri tra ve = HTML element 
				pVLNewElement = 
					pVReturn->AddChild(pVLTmpElement->m_sName, pVLTmpElement->m_sAliasName, pVLTmpElement->m_pVar);
				pVLNewElement->SetIntName(nIndexInArrayReturn);
				nIndexInArrayReturn++;
			}
		}
	}
	lstHTMLElement.clear();
}

//------------------------------------------------------------------------------
// cai gia tri thuoc tinh cho doi tuong HTML Element
//------------------------------------------------------------------------------
void scElementSetAttribute(CVar*pVLocalStack, void *pUserData)
{
	int nSizeLst = 0;
	int i = 0;
	bool bCreateNewAttr = false; // co bao quyen tao thuoc tinh moi cho doi tuong

	CVar* pVThisObject = NULL;
	CVarLink* pVLAttr = NULL;
	CVarLink* pVLSubAttr = NULL;
	vector<string> lstsSubAttr;
	vector<string> lstsNameAndVal;
	CProgramJs *programJs = (CProgramJs*)pUserData;
	string sNameAttr = scGetString("szNameAttr");
	transform(sNameAttr.begin(), sNameAttr.end(),sNameAttr.begin(), ::tolower);

	string sValAttr = StrimString(scGetString("szValAttr"), ' ');
	
	pVThisObject = pVLocalStack->GetParameter("this");
	
	// Neu ten thuoc tinh == "" -> khong add
	if (sNameAttr.size() == 0)
		return;
	
	else
	{
		// tim thuoc tinh szNameAttr
		pVLAttr = pVThisObject->FindChild(sNameAttr);

		if (!pVLAttr)
		{
			pVLAttr = programJs->FindInPrototypeClasses(pVThisObject, sNameAttr);
			if (pVLAttr)
			{
				// tao thuoc tinh moi cho this; 
				// chi nhung thuoc tinh co trong prototype moi duoc gan thuoc tinh
				bCreateNewAttr = true;
				pVLAttr = pVThisObject->AddChild(pVLAttr->m_sName, pVLAttr->m_sAliasName, pVLAttr->m_pVar->DeepCopy());
			}
		}

		if (pVLAttr != NULL)
		{
			// neu ton tai thuoc tinh -> gan gia tri cho thuoc tinh
			if (sNameAttr != "style")
			{
				// cai dat gia tri cho thuoc tinh hien tai
				pVLAttr->m_pVar->SetString(sValAttr);
			}
			else 
			{
				// neu co dau : -> set thuoc tinh con cua thuoc tinh hien tai
				lstsSubAttr = SplitString(sValAttr, ';');
				nSizeLst = lstsSubAttr.size();
				for (i = 0; i < nSizeLst; i++)
				{
					sValAttr = lstsSubAttr.at(i);
					lstsNameAndVal = SplitString(sValAttr, ':');
					if (lstsNameAndVal.size()!=2)
						continue;
					else 
					{
						// tim thuoc tinh con
						pVLSubAttr = pVLAttr->m_pVar->FindChild(lstsNameAndVal.at(0));
						if (pVLSubAttr)
						{
							// cai dat gia tri cho thuoc tinh
							pVLSubAttr->m_pVar->SetString(lstsNameAndVal.at(1));
						}
					}
				}
				// xoa bo cac phan tu 
				lstsSubAttr.clear();
				lstsNameAndVal.clear();
			}
		}
	}
}

//------------------------------------------------------------------------------
// them phan tu con cho HTML Element 
//------------------------------------------------------------------------------
void scElementAppendChild(CVar*pVLocalStack, void *pUserData)
{
	int nTypeVirus = 0;
	CProgramJs *programJs = (CProgramJs*)pUserData;
	CVar* pVThisObject = pVLocalStack->GetParameter("this");
	CVar* pVNewElement = pVLocalStack->GetParameter("newElement");
	CRuntimeException* pRE = NULL;

	pVThisObject->AddChild(STR_NAME_HTML_ELEMENT, DEFAULT_ALIAS_NAME, pVNewElement);
}

//------------------------------------------------------------------------------
// Tao doi tuong Date
//------------------------------------------------------------------------------
void scDateConstructor(CVar*pVLocalStack, void *pUserData)
{
	CProgramJs *pProgramJs = (CProgramJs*)pUserData;
	CVar* pVThisObject = pVLocalStack->GetParameter("this");
	CVarLink* pVLPrototype = NULL;

	// Cai dat ngay thang mac dinh
	pVThisObject->SetDataString("27081998");

	pVLPrototype = pProgramJs->m_pVRootStack->FindChild("Date");
	if (pVLPrototype)
		pVThisObject->AddChild(STR_PROTOTYPE_CLASS, DEFAULT_ALIAS_NAME, pVLPrototype->m_pVar);

	// Tra ve ngay thang mac dinh
	pVLocalStack->SetReturnVar(pVThisObject);
}

//------------------------------------------------------------------------------
// Tra ve thang
//------------------------------------------------------------------------------
void scDateGetMonth(CVar*pVLocalStack, void *)
{
	scReturnInt(10);
}

//------------------------------------------------------------------------------
// Tra ve ngay
//------------------------------------------------------------------------------
void scDateGetDate(CVar*pVLocalStack, void *)
{
	scReturnInt(5);
}
//------------------------------------------------------------------------------
// Tra ve thoi gian thuc hien lenh code tiep theo
//------------------------------------------------------------------------------
void scSetTimeOut(CVar*pVLocalStack, void *pUserData)
{
	scReturnInt(5);
}
//------------------------------------------------------------------------------
// Tra ve ngay
//------------------------------------------------------------------------------
void scDateGetYear(CVar*pVLocalStack, void *)
{
	scReturnInt(1994);
}

//------------------------------------------------------------------------------
// Tra ve gio
//------------------------------------------------------------------------------
void scDateGetHours(CVar*pVLocalStack, void *)
{
	scReturnInt(10);
}
//------------------------------------------------------------------------------
// Tra ve time
//------------------------------------------------------------------------------
void scDateSetTime(CVar*pVLocalStack, void *)
{
	scReturnInt(10);
}
//------------------------------------------------------------------------------
// Date.getTime()
//------------------------------------------------------------------------------
void scDateGetTime(CVar*pVLocalStack, void *)
{
	scReturnInt(10);
}
//------------------------------------------------------------------------------
// Date.toUTCString()
//------------------------------------------------------------------------------
void scDatetoUTCString(CVar *pVLocalStack, void *)
{
	scReturnString("16 27 08 1998");
}
//------------------------------------------------------------------------------
// constructor Double
// tao lap doi tuong Double voi tham so dau vao
void scDoubleConstructor(CVar *pVLocalStack, void *)
{
	double sData = scGetDouble("num");
	pVLocalStack->GetParameter("this")->SetDouble(sData);
	scReturnDouble(sData);

}
//------------------------------------------------------------------------------
// Double.toString for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scDoubletoString(CVar *pVLocalStack, void*)
{
	scReturnString(NDC_TODAY_GOODDAY);
}

//------------------------------------------------------------------------------
// WScript for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scWScriptConstructor(CVar*pVLocalStack, void *pUserData)
{
	CProgramJs *pProgramJs = (CProgramJs*)pUserData;
	CVar* pVThisObject = pVLocalStack->GetParameter("this");
	CVarLink* pVLPrototype = NULL;

	// khong co
	pVThisObject->SetDataString(NDC_TODAY_GOODDAY);

	pVLPrototype = pProgramJs->m_pVRootStack->FindChild("WScript");
	if (pVLPrototype)
		pVThisObject->AddChild(STR_PROTOTYPE_CLASS, DEFAULT_ALIAS_NAME, pVLPrototype->m_pVar, TRUE);

	// Tra ve gia tri 
	pVLocalStack->SetReturnVar(pVThisObject);
}
//------------------------------------------------------------------------------
// WScript.CreateObject for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scWScriptCreateObject(CVar*pVLocalStack, void *)
{
	string sSearchObject = scGetString("str");
	g_sstrDetectDownloader->bCreateObject = true;
	if (sSearchObject == "ADODB.Stream")
		scReturnDouble(10.10);
	else
		scReturnString(NDC_TODAY_GOODDAY);
	
}

void scWScriptSleep(CVar*pVLocalStack, void *)
{
	scReturnString(NDC_TODAY_GOODDAY);

}
//------------------------------------------------------------------------------
// CreateObject FileExists for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scStringFileExists(CVar *pVLocalStack, void*)
{
	scReturnInt(0);
}
//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scStringExpandEnvironmentStrings(CVar *pVLocalStack, void*)
{
	scReturnString(NDC_TODAY_GOODDAY);
}
//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scStringopen(CVar *pVLocalStack, void*)
{
	g_sstrDetectDownloader->bOpen = true;
	scReturnString(NDC_TODAY_GOODDAY);
}
//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------

void scDoubleopen(CVar *pVLocalStack, void*)
{
	scReturnString(NDC_TODAY_GOODDAY);
}
//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scStringsend(CVar *pVLocalStack, void *pUserData)
{	
	scReturnString(NDC_TODAY_GOODDAY);
}
//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scDoublesaveToFile(CVar *pVLocalStack, void* pUserData)
{

	g_sstrDetectDownloader->bsaveToFile = true;
	scReturnString(NDC_TODAY_GOODDAY);
}

//------------------------------------------------------------------------------
//  for TrojanDowloader
//------------------------------------------------------------------------------
void scEnviroment(CVar *pVLocalStack, void*)
{
	scReturnString(NDC_TODAY_GOODDAY);
}

//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scWScriptQuit(CVar *pVLocalStack, void*)
{
	scReturnString(NDC_TODAY_GOODDAY);
} 
  
//------------------------------------------------------------------------------
//  for TrojanDowloader
//------------------------------------------------------------------------------

void scDoublewrite(CVar *pVLocalStack, void*)
{
	scReturnString(NDC_TODAY_GOODDAY);

}
//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------

void scStringRun(CVar *pVLocalStack, void *pUserData)
{
	scReturnString(NDC_TODAY_GOODDAY);
}
//------------------------------------------------------------------------------
//  for TrojanDowloader.JS.Nemucod.AS
//------------------------------------------------------------------------------
void scActiveXObjectConstructor(CVar*pVLocalStack, void *)
{
	string sData = scGetString("ARRAY");
	g_sstrDetectDownloader->bCreateObject = true;
	if (sData == "")
		throw new CRuntimeException(" ActiveXObject loi!!");
	if (sData == "ADODB.Stream")
		scReturnDouble(10.10);
	else
		scReturnString(NDC_TODAY_GOODDAY);
}
//----

//------------------------------------------------------------------------------
// Register Functions API 
// ...
// Neu so luong tham so trong loi goi ham khong xac dinh truoc 
// (vd: str1.concat(str2, str3...);) thi cai dat ten bien la ARRAY => Tat ca cac 
// tham so trong loi goi ham se duoc luu vao phan tu con cua bien ARRAY.
//------------------------------------------------------------------------------
void RegisterFunctions(CProgramJs *pProgramJs)
{
	CVarLink* pVLHtmlElement = NULL;
	
	pProgramJs->AddFuncNative("function exec(jsCode)", scExec, pProgramJs); // execute the given code
	pProgramJs->AddFuncNative("function eval(jsCode)", scEval, pProgramJs); // execute the given string (an expression) and return the result
	pProgramJs->AddFuncNative("function alert(sMsg)", scAlert, NULL);
	pProgramJs->AddFuncNative("function trace()", scTrace, pProgramJs);
	pProgramJs->AddFuncNative("function unescape(str)", scUnescape, 0);
	pProgramJs->AddFuncNative("function charToInt(ch)", scCharToInt, 0); //  convert a character to an int - get its value
	pProgramJs->AddFuncNative("function setInterval(ARRAY)", scSetInterval, pProgramJs); //  convert a character to an int - get its value
	pProgramJs->AddFuncNative("function parseInt(str,radix)", scParseInt, 0);
	pProgramJs->AddFuncNative("function encodeURIComponent(sUri)", scEncodeURIComponent, 0);
	pProgramJs->AddFuncNative("function addEventListener(ARRAY)", scAddEventListener, 0);
	pProgramJs->AddFuncNative("function attachEvent(ARRAY)", scAttachEvent, 0);
	pProgramJs->AddFuncNative("function Aes.Ctr.decrypt(str1,str2,lo)", scAesCrtDecrypt, 0);
	
	//--------------------------------------------------------------------------
	// jQuery
	pProgramJs->AddFuncNative("function jQuery(objectHtml)", scJQuery, pProgramJs);
	pProgramJs->AddFuncNative("function $(objectHtml)", scJQuery, pProgramJs);
	pProgramJs->AddFuncNative("function jQuery.ready(ARRAY)", scJQueryReady, pProgramJs);
	pProgramJs->AddFuncNative("function jQuery.parent(ARRAY)", scJQueryParent, pProgramJs);
	pProgramJs->AddFuncNative("function jQuery.mousemove(ARRAY)", scJQueryMousemove, 0);
	pProgramJs->AddFuncNative("function jQuery.mouseout(ARRAY)", scJQueryMouseout, 0);
	pProgramJs->AddFuncNative("function jQuery.mouseover(ARRAY)", scJQueryMouseover, pProgramJs);
	pProgramJs->AddFuncNative("function jQuery.hide()", scJQueryHide, pProgramJs);
	pProgramJs->AddFuncNative("function jQuery.show()", scJQueryShow, 0);


	//--------------------------------------------------------------------------
	// Object 
	pProgramJs->AddFuncNative("function Object.dump()", scObjectDump, 0);
	pProgramJs->AddFuncNative("function Object.clone()", scObjectClone, 0);

	//--------------------------------------------------------------------------
	// Math 
	pProgramJs->AddFuncNative("function Math.random()", scMathRand, 0);
	pProgramJs->AddFuncNative("function Math.randInt(min, max)", scMathRandInt, 0);
	pProgramJs->AddFuncNative("function Math.abs(a)", scMathAbs, 0);
	pProgramJs->AddFuncNative("function Math.round(a)", scMathRound, 0);
	pProgramJs->AddFuncNative("function Math.ceil(a)", scMathCeil, 0);
	pProgramJs->AddFuncNative("function Math.min(a,b)", scMathMin, 0);
	pProgramJs->AddFuncNative("function Math.max(a,b)", scMathMax, 0);
	pProgramJs->AddFuncNative("function Math.range(x,a,b)", scMathRange, 0);
	pProgramJs->AddFuncNative("function Math.sign(a)", scMathSign, 0);

	pProgramJs->AddFuncNative("function Math.PI()", scMathPI, 0);
	pProgramJs->AddFuncNative("function Math.toDegrees(a)", scMathToDegrees, 0);
	pProgramJs->AddFuncNative("function Math.toRadians(a)", scMathToRadians, 0);
	pProgramJs->AddFuncNative("function Math.sin(a)", scMathSin, 0);
	pProgramJs->AddFuncNative("function Math.asin(a)", scMathASin, 0);
	pProgramJs->AddFuncNative("function Math.cos(a)", scMathCos, 0);
	pProgramJs->AddFuncNative("function Math.acos(a)", scMathACos, 0);
	pProgramJs->AddFuncNative("function Math.tan(a)", scMathTan, 0);
	pProgramJs->AddFuncNative("function Math.atan(a)", scMathATan, 0);
	pProgramJs->AddFuncNative("function Math.sinh(a)", scMathSinh, 0);
	pProgramJs->AddFuncNative("function Math.asinh(a)", scMathASinh, 0);
	pProgramJs->AddFuncNative("function Math.cosh(a)", scMathCosh, 0);
	pProgramJs->AddFuncNative("function Math.acosh(a)", scMathACosh, 0);
	pProgramJs->AddFuncNative("function Math.tanh(a)", scMathTanh, 0);
	pProgramJs->AddFuncNative("function Math.atanh(a)", scMathATanh, 0);
	pProgramJs->AddFuncNative("function Math.E()", scMathE, 0);
	pProgramJs->AddFuncNative("function Math.log(a)", scMathLog, 0);
	pProgramJs->AddFuncNative("function Math.log10(a)", scMathLog10, 0);
	pProgramJs->AddFuncNative("function Math.exp(a)", scMathExp, 0);
	pProgramJs->AddFuncNative("function Math.pow(a,b)", scMathPow, 0);
	pProgramJs->AddFuncNative("function Math.floor(a)", scMathFloor, 0);
	pProgramJs->AddFuncNative("function Math.sqr(a)", scMathSqr, 0);
	pProgramJs->AddFuncNative("function Math.sqrt(a)", scMathSqrt, 0);   

	//--------------------------------------------------------------------------
	// String  
	pProgramJs->AddFuncNative("function String(str)", scStringConstructor, 0); // create new String with input
	pProgramJs->AddFuncNative("function String.constructor(str)", scStringConstructor, 0); // create new String with input
	pProgramJs->AddFuncNative("function String.indexOf(search)", scStringIndexOf, 0); // find the position of a string in a string, -1 if not
	pProgramJs->AddFuncNative("function String.substring(lo,hi)", scStringSubstring, 0);
	pProgramJs->AddFuncNative("function String.substr(lo,length)", scStringSubstr, 0);
	pProgramJs->AddFuncNative("function String.charAt(pos)", scStringCharAt, 0);
	pProgramJs->AddFuncNative("function String.charCodeAt(pos)", scStringCharCodeAt, 0);
	pProgramJs->AddFuncNative("function String.fromCharCode(ARRAY)", scStringFromCharCode, 0);
	pProgramJs->AddFuncNative("function String.split(separator)", scStringSplit, 0);
	pProgramJs->AddFuncNative("function String.replace(searchvalue,newvalue)", scStringReplace, 0);
	pProgramJs->AddFuncNative("function String.concat(ARRAY)", scStringConcat, 0);
 	pProgramJs->AddFuncNative("function String.match(regex)", scStringMatch, 0);

	//--------------------------------------------------------------------------
	// Integer 
	pProgramJs->AddFuncNative("function Integer.parseInt(str)", scIntegerParseInt, 0); // string to int
	pProgramJs->AddFuncNative("function Integer.valueOf(str)", scIntegerValueOf, 0); // value of a single character

	//--------------------------------------------------------------------------
	// JSON 
	pProgramJs->AddFuncNative("function JSON.stringify(obj, replacer)", scJSONStringify, 0); // convert to JSON. replacer is ignored at the moment
	// JSON.parse is left out as you can (unsafely!) use eval instead

	//--------------------------------------------------------------------------
	// Array 
	pProgramJs->AddFuncNative("function Array(ARRAY)", scArrayConstructor, 0);
	pProgramJs->AddFuncNative("function Array.constructor(ARRAY)", scArrayConstructor, 0);
	pProgramJs->AddFuncNative("function Array.contains(obj)", scArrayContains, 0);
	pProgramJs->AddFuncNative("function Array.remove(obj)", scArrayRemove, 0);
	pProgramJs->AddFuncNative("function Array.join(separator)", scArrayJoin, 0);
	
	pProgramJs->AddFuncNative("function Array.reverse()", scArrayReverse, 0);
	//programJs->AddFuncNative("function Function.apply(obj,arr,pFuncCall)", scFunctionApply, 0);


	//--------------------------------------------------------------------------
	// element prototype 
	pProgramJs->AddFuncNative("function element.setAttribute(szNameAttr,szValAttr)", scElementSetAttribute, pProgramJs);
	pProgramJs->AddFuncNative("function element.appendChild(newElement)", scElementAppendChild, pProgramJs);
	pProgramJs->AddFuncNative("function element.createElement(szNameElement)", scElementCreateElement, pProgramJs);
	pProgramJs->AddFuncNative("function element.getElementById(szId)", scElementGetElementById, pProgramJs);
	pProgramJs->AddFuncNative("function element.getElementsByTagName(szTagName)", scElementGetElementsByTagName, pProgramJs);

	pProgramJs->AddAttrNative("element.style.top", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.style.bottom", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.style.left", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.style.right", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.style.width", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.style.height", SCRIPTVAR_STRING);

	pProgramJs->AddAttrNative("element.style.visibility", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.style.display", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.style.position", SCRIPTVAR_STRING);

	pProgramJs->AddAttrNative("element.innerHTML", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.tagName", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.src", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.width", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.height", SCRIPTVAR_STRING);
	pProgramJs->AddAttrNative("element.id", SCRIPTVAR_STRING);
	
	//--------------------------------------------------------------------------
	// document
	// tao san the: html tag
	pVLHtmlElement = pProgramJs->m_pVRootStack->FindChild("element");
	//pProgramJs->AddAttrNative("document", SCRIPTVAR_OBJECT);
	pProgramJs->AddAttrNative("document.prototype", pVLHtmlElement->m_pVar);
	pProgramJs->AddAttrNative("document.tagName", new CVar("HTML"));

	pProgramJs->AddAttrNative("document.head", SCRIPTVAR_OBJECT);
	pProgramJs->AddAttrNative("document.head.prototype", pVLHtmlElement->m_pVar);
	pProgramJs->AddAttrNative("document.head.tagName", new CVar("HEAD"));

	pProgramJs->AddAttrNative("document.body", SCRIPTVAR_OBJECT);
	pProgramJs->AddAttrNative("document.body.prototype", pVLHtmlElement->m_pVar);
	pProgramJs->AddAttrNative("document.body.tagName", new CVar("BODY"));

	pProgramJs->AddFuncNative("function document.write(str)", scDocumentWrite, pProgramJs);
	pProgramJs->AddFuncNative("function document.writeln(str)", scDocumentWrite, pProgramJs);
	pProgramJs->AddAttrNative("document.cookie", new CVar(SZ_VAL_COOKIE_DEFAULT));
	pProgramJs->AddAttrNative("document.loaded", new CVar(1));
	pProgramJs->AddAttrNative("document.referrer", new CVar("google.AutodetectvirusNDC"));

	//--------------------------------------------------------------------------
	// location
	pProgramJs->AddAttrNative("location", new CVar("location"));
	pProgramJs->AddAttrNative("top.location", new CVar("top.location"));
	pProgramJs->AddAttrNative("top.location.href", new CVar("top.location.href"));
	pProgramJs->AddAttrNative("location.href", new CVar("location.href"));
	pProgramJs->AddAttrNative("location.search", new CVar("google.AutodetectvirusNDC"));
	pProgramJs->AddAttrNative("document.location.host", new CVar("google.AutodetectvirusNDC"));
	//--------------------------------------------------------------------------
	// navigator
	pProgramJs->AddAttrNative("navigator.cookieEnabled", new CVar(1));	
	pProgramJs->AddAttrNative("navigator.appVersion", 
		new CVar("5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, \
				 like Gecko) Chrome/40.0.2214.115 Bchrome/40.0.2214.115 Safari/537.36"));

	//--------------------------------------------------------------------------
	// function for HEURTrojan.Script.Iframer_2
	pProgramJs->AddFuncNative("function Date(ARRAY)", scDateConstructor, pProgramJs);
	pProgramJs->AddFuncNative("function Date.getMonth()", scDateGetMonth, NULL);
	pProgramJs->AddFuncNative("function Date.getDate()", scDateGetDate, NULL);
	pProgramJs->AddFuncNative("function Date.getHours()", scDateGetHours, NULL);
	pProgramJs->AddFuncNative("function Date.getTime()", scDateGetTime, NULL);
	pProgramJs->AddFuncNative("function Date.setTime(strtime)", scDateSetTime, NULL);
	pProgramJs->AddFuncNative("function Date.toUTCString()", scDatetoUTCString, NULL);

	//--------------------------------------------------------------------------
	// function for 
	pProgramJs->AddFuncNative("function setTimeout(time)", scSetTimeOut, NULL);

	//--------------------------------------------------------------------------
	// Wscript for  TrojanDownloader.JS.Nemucod

	pProgramJs->AddFuncNative("function WScript(ARRAY)", scWScriptConstructor, pProgramJs);
	pProgramJs->AddFuncNative("function WScript.CreateObject(str)", scWScriptCreateObject, NULL);
    pProgramJs->AddFuncNative("function WScript.Sleep(abc)", scWScriptSleep, NULL);
	pProgramJs->AddFuncNative("function WScript.Quit(lo)", scWScriptQuit, NULL);
	//--------------------------------------------------------------------------
	// Wscript for  TrojanDownloader.JS.Nemucod.AS
	pProgramJs->AddFuncNative("function ActiveXObject(ARRAY)", scActiveXObjectConstructor, pProgramJs);





	//--------------------------------------------------------------------------
	// function for TrojanDownloader.JS.Nemucod

	pProgramJs->AddFuncNative("function Double(num)", scDoubleConstructor, 0);
	pProgramJs->AddFuncNative("function Double.constructor(str)", scDoubleConstructor, 0);
	pProgramJs->AddFuncNative("function Double.toString()", scDoubletoString, 0);
	pProgramJs->AddFuncNative("function Double.toString(radix)", scDoubletoString, 0);
	pProgramJs->AddFuncNative("function String.FileExists(str)", scStringFileExists, 0);
	pProgramJs->AddFuncNative("function String.ExpandEnvironmentStrings(str)", scStringExpandEnvironmentStrings, 0);
	pProgramJs->AddFuncNative("function String.open(str1,str2,str3)", scStringopen, 0);
	pProgramJs->AddFuncNative("function Double.open()", scDoubleopen, 0);
	pProgramJs->AddFuncNative("function String.send()", scStringsend, 0);
	pProgramJs->AddFuncNative("function Double.saveToFile(str1,str2)", scDoublesaveToFile, pProgramJs);
	pProgramJs->AddFuncNative("function Double.SaveToFile(str1,str2)", scDoublesaveToFile, pProgramJs);
	//pProgramJs->AddFuncNative("function Double.close()", scDoubleclose, 0);
	pProgramJs->AddFuncNative("function Double.write(str1)", scDoublewrite, 0);
	pProgramJs->AddFuncNative("function String.Run(str1,str2,str3)", scStringRun, pProgramJs);
	pProgramJs->AddFuncNative("function String.Environment(str1)", scEnviroment, 0);
	
	//pProgramJs->AddFuncNative("function Double.LoadFromFile(str)", scDoubleLoadFromFile, 0);
	pProgramJs->AddFuncNative("function RegExp(str1,str2)", scRegExp, 0);
}