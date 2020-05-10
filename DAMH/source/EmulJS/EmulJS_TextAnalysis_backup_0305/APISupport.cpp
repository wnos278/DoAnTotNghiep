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
// return stack call
//------------------------------------------------------------------------------
void scTrace(CVar *pVLocalStack, void *pUserData)
{
	
}

//------------------------------------------------------------------------------
// Unescape
//------------------------------------------------------------------------------
void scUnescape(CVar *pVLocalStack, void *pUserData)
{

}

void scEscape(CVar *pVLocalStack, void *pUserData)
{
	scReturnString("Escape Called");
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

}

// -----------------------------------------------------------------------------
// API Object.clone
// -----------------------------------------------------------------------------
void scObjectClone(CVar *pVLocalStack, void *) 
{

}

// -----------------------------------------------------------------------------
// API Math.rand
// -----------------------------------------------------------------------------
void scMathRand(CVar *pVLocalStack, void *) 
{

}

// -----------------------------------------------------------------------------
// API Math.randInt
// -----------------------------------------------------------------------------
void scMathRandInt(CVar *pVLocalStack, void *) 
{

}

// -----------------------------------------------------------------------------
// API charToInt
// -----------------------------------------------------------------------------
void scCharToInt(CVar *pVLocalStack, void *) 
{

}

// -----------------------------------------------------------------------------
// setInterval(arg1, arg2);
// Doi arg2 miniseconds chay ham arg1();
// ---
// Cai dat: thuc hien ham arg1() neu la doi tuong ham
// -----------------------------------------------------------------------------
void scSetInterval(CVar *pVLocalStack, void *pUserData)
{
	
}

//------------------------------------------------------------------------------
// Chuyen tu xau "str" thanh so trong he co so "radix"
// tra ve kieu so nguyen 
//------------------------------------------------------------------------------
void scParseInt(CVar *pVLocalStack, void *)
{
	
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

}

//------------------------------------------------------------------------------
// Lay 1 thanh phan HTML  -> tra ve doi tuong jQuery
//------------------------------------------------------------------------------
void scJQuery(CVar *pVLocalStack, void *pUserData)
{

}

//------------------------------------------------------------------------------
// Ham API jQuery.ready(functionObject, arg1, arg2,...)
//------------------------------------------------------------------------------
void scJQueryReady(CVar *pVLocalStack, void *pUserData)
{

}

//------------------------------------------------------------------------------
// Ham API jQuery.parent() -> return jQuery(document)
//------------------------------------------------------------------------------
void scJQueryParent(CVar *pVLocalStack, void *pUserData)
{

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

}

//------------------------------------------------------------------------------
// Ham API jQuery.hiden()
// An di doi tuong HTML
//------------------------------------------------------------------------------
void scJQueryHide(CVar *pVLocalStack, void *pUserData)
{
	
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

}
//------------------------------------------------------------------------------
// RexExp(str1,str2)
// tao lap doi tuong regex voi tham so dau vao
//------------------------------------------------------------------------------

void scRegExp(CVar *pVLocalStack, void *)
{

}
//------------------------------------------------------------------------------
// String.indexof(). Tim vi tri cua 1 xau String trong doi tuong string
//------------------------------------------------------------------------------
void scStringIndexOf(CVar *pVLocalStack, void *) 
{

}

//------------------------------------------------------------------------------
// String.subString(lo, hi). Sinh 1 xau con tu 1 xau string co truoc
//------------------------------------------------------------------------------
void scStringSubstring(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// String.subString(lo, length). Sinh 1 xau con tu 1 xau string co truoc
//------------------------------------------------------------------------------
void scStringSubstr(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// String.charAt(pos). Xuat ki tu o vi tri pos cua xau string
//------------------------------------------------------------------------------
void scStringCharAt(CVar *pVLocalStack, void *) 
{

}

//------------------------------------------------------------------------------
// String.charCodeAt(pos). Chuyen 1 ki tu chu cai -> ki tu so theo bang ma ASCII
//------------------------------------------------------------------------------
void scStringCharCodeAt(CVar *pVLocalStack, void *) 
{

}

//------------------------------------------------------------------------------
// API: String.split
//------------------------------------------------------------------------------
void scStringSplit(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// API: String.replace
//------------------------------------------------------------------------------
void scStringReplace(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// str1.concat(str2, str3,...);
// Noi cac xau string str1 = str1+str2+str3+...
//------------------------------------------------------------------------------
void scStringConcat(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// str1.match(/str/);
// str1.match(/str/i);
// str1.match(/str/g);
// str1.match(/str/gi);
//------------------------------------------------------------------------------

void scStringMatch(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// Chuyen mang Args => xau string: String.fromCharCode(65, 66, 67);  // "ABC"
//  String.fromCharCode("65", "66", "67");  // "ABC"
//------------------------------------------------------------------------------
void scStringFromCharCode(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// API: Integer.parseInt(str):: chuyen 1 xau string -> kieu Int
//------------------------------------------------------------------------------
void scIntegerParseInt(CVar *pVLocalStack, void *) 
{

}

//------------------------------------------------------------------------------
// API: Integer.Valueof(str):: chuyen 1 xau string -> kieu Int
//------------------------------------------------------------------------------
void scIntegerValueOf(CVar *pVLocalStack, void *) 
{

}

//------------------------------------------------------------------------------
// API: JSON.Stringify:: xuat tap cac doi tuong dang JSON
//------------------------------------------------------------------------------
void scJSONStringify(CVar *pVLocalStack, void *) 
{

}

//------------------------------------------------------------------------------
// Exec(jsCode);
// thuc hien jsCode trong tham so dau vao ham Exec
//------------------------------------------------------------------------------
void scExec(CVar *pVLocalStack, void *pUserData) 
{

}

//------------------------------------------------------------------------------
// var xxx = Eval(jsCode);
// thuc hien jsCode trong tham so dau vao ham Eval + co gia tri tra ve 
//------------------------------------------------------------------------------
void scEval(CVar *pVLocalStack, void *pUserData) 
{

}

//------------------------------------------------------------------------------
// Dua ra hop thoai thong bao cho nguoi dung
//------------------------------------------------------------------------------
void scAlert(CVar *pVLocalStack, void *)
{

}

//------------------------------------------------------------------------------
// API:  Array.contain
//------------------------------------------------------------------------------
void scArrayContains(CVar *pVLocalStack, void *pUserData)
{

}

//------------------------------------------------------------------------------
// API: Array.remove
//------------------------------------------------------------------------------
void scArrayRemove(CVar *pVLocalStack, void *pUserData) 
{

}

//------------------------------------------------------------------------------
// API: Array.join
//------------------------------------------------------------------------------
void scArrayJoin(CVar *pVLocalStack, void *pUserData) 
{

}

//------------------------------------------------------------------------------
// API: Array.reverse : Hàm đảo ngược xâu
//------------------------------------------------------------------------------
void scArrayReverse(CVar *pVLocalStack, void *pUserData) 
{

}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// API: Array.constructor : Ham khoi tao 1 doi tuong Array moi
//------------------------------------------------------------------------------
void scArrayConstructor(CVar *pVLocalStack, void *)
{

}


//------------------------------------------------------------------------------
// thuc hien ham hien tai
// Doi tuong ham = obj
// ds tham so = arr
//------------------------------------------------------------------------------
void scFunctionApply(CVar *pVLocalStack, void *)
{

}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Math Funtions

//------------------------------------------------------------------------------
//Math.abs(x) - returns absolute of given value
//------------------------------------------------------------------------------
void scMathAbs(CVar *pVLocalStack, void *pUserData) 
{

}

//------------------------------------------------------------------------------
//Math.round(a) - returns nearest round of given value
//------------------------------------------------------------------------------
void scMathRound(CVar *pVLocalStack, void *pUserData)
{

}

//------------------------------------------------------------------------------
//Math.ceil(a) - returns nearest larger of given value
//------------------------------------------------------------------------------
void scMathCeil(CVar *pVLocalStack, void *pUserData)
{

}

//------------------------------------------------------------------------------
//Math.min(a,b) - returns minimum of two given values 
//------------------------------------------------------------------------------
void scMathMin(CVar *pVLocalStack, void *pUserData) 
{

}

//Math.max(a,b) - returns maximum of two given values  
void scMathMax(CVar *pVLocalStack, void *pUserData) 
{

}

//------------------------------------------------------------------------------
//Math.range(x,a,b) - returns value limited between two given values  
//------------------------------------------------------------------------------
void scMathRange(CVar *pVLocalStack, void *pUserData)
{

}


//------------------------------------------------------------------------------
//Math.sign(a) - returns sign of given value (-1==negative,0=zero,1=positive)
//------------------------------------------------------------------------------
void scMathSign(CVar *pVLocalStack, void *pUserData)
{

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

}


//------------------------------------------------------------------------------
// Tao doi tuong HTML Element
//------------------------------------------------------------------------------
void scElementCreateElement(CVar*pVLocalStack, void *pUserData)
{


}

void scElementSetAttribute(CVar*pVLocalStack, void *pUserData);
//------------------------------------------------------------------------------
// Lay thanh phan Html element la con cua doi tuong goi phuong thuc nay
// voi thuoc tinh id = 'szId'
//------------------------------------------------------------------------------
void scElementGetElementById(CVar*pVLocalStack, void *pUserData)
{

}

//------------------------------------------------------------------------------
// Lay danh sach "Html element con" 
// voi thuoc tinh tagName = 'szTagName'
//------------------------------------------------------------------------------
void scElementGetElementsByTagName(CVar*pVLocalStack, void *pUserData)
{

}

//------------------------------------------------------------------------------
// cai gia tri thuoc tinh cho doi tuong HTML Element
//------------------------------------------------------------------------------
void scElementSetAttribute(CVar*pVLocalStack, void *pUserData)
{
	
}

//------------------------------------------------------------------------------
// them phan tu con cho HTML Element 
//------------------------------------------------------------------------------
void scElementAppendChild(CVar*pVLocalStack, void *pUserData)
{
	
}

//------------------------------------------------------------------------------
// Tao doi tuong Date
//------------------------------------------------------------------------------
void scDateConstructor(CVar*pVLocalStack, void *pUserData)
{

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
	scReturnString("10 10 10 1995");
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

}
//------------------------------------------------------------------------------
// WScript.CreateObject for TrojanDowloader.JS.Nemucod
//------------------------------------------------------------------------------
void scWScriptCreateObject(CVar*pVLocalStack, void *)
{
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
	pProgramJs->AddFuncNative("function escape(str)", scEscape, 0);
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