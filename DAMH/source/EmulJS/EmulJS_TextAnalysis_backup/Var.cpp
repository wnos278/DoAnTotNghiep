////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac phuong thuc lop CVar. Lop luu tru cac kieu 
// du lieu cua ngon ngu JS.
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include "Var.h"
#include <sstream>
#include <stdio.h>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG


//------------------------------------------------------------------------------
// Tao du lieu Undefined
//------------------------------------------------------------------------------
CVar::CVar()
{
#if SAFE_MEMORY
	AddTempStack(this);
#endif
	Init();
	m_nTypeVar = SCRIPTVAR_UNDEFINED;
}

//------------------------------------------------------------------------------
// Tao du lieu kieu string
//------------------------------------------------------------------------------
CVar::CVar(const string &sData) 
{
#if SAFE_MEMORY
	AddTempStack(this);
#endif
	Init();
	m_nTypeVar = SCRIPTVAR_STRING;
	m_sData = sData;
}

//------------------------------------------------------------------------------
// Tao du lieu string - kieu doi tuong = nVarFlag
//------------------------------------------------------------------------------
CVar::CVar(const string &sVarData, int nTypeVar)
{
#if SAFE_MEMORY
	AddTempStack(this);
#endif
	
	Init();
	m_nTypeVar = nTypeVar;
	
	if (nTypeVar & SCRIPTVAR_INTEGER) 
	{
		m_lData = strtol(sVarData.c_str(),0,0);
	} 
	else if (nTypeVar & SCRIPTVAR_DOUBLE) 
	{
		m_fData = strtod(sVarData.c_str(),0);
	} else
		m_sData = sVarData;
}

//------------------------------------------------------------------------------
// Tao du lieu kieu double
//------------------------------------------------------------------------------
CVar::CVar(double fVal) 
{
#if SAFE_MEMORY
	AddTempStack(this);
#endif
	Init();
	SetDouble(fVal);
}

//------------------------------------------------------------------------------
// Constructor: tao du lieu kieu Int
//------------------------------------------------------------------------------
CVar::CVar(int nVal) 
{
#if SAFE_MEMORY
	AddTempStack(this);
#endif
	Init();
	SetInt(nVal);
}

//------------------------------------------------------------------------------
// Huy doi tuong
//------------------------------------------------------------------------------
CVar::~CVar(void)
{
#if SAFE_MEMORY
	// go ra khoi tempStack 
	//RemoveTempStack(this);
	
#endif

	//RemoveAllChildren();
	SAFE_DELETE_ARRAY(m_pBuffDefineFunc);
}

//------------------------------------------------------------------------------
// Cai dat khoi tao gia tri ban dau cho cac doi tuong CVar khi duoc cap phat
//------------------------------------------------------------------------------
void CVar::Init()
{
	m_pVLFirstChild = 0;
	m_pVLLastChild = 0;
	m_pBuffDefineFunc = NULL;
	
	m_nTypeVar = 0;
	m_fnCallback = 0;
	m_fnCallbackUserData = 0;
	
	m_sData = STR_BLANK_DATA;
	m_lData = 0;
	m_fData = 0;
	
	m_nRefs = 0;
	m_bIsDeleting = FALSE;
}

//------------------------------------------------------------------------------
// Lay du lieu tu lien ket bien "return"
//------------------------------------------------------------------------------
CVar *CVar::GetReturnVar() 
{
	return GetParameter(STR_RETURN_VAR);
}

//------------------------------------------------------------------------------
// Cai du lieu cho lien ket bien "return"
//------------------------------------------------------------------------------
void CVar::SetReturnVar(CVar *pVar)
{
	FindChildOrCreate(STR_RETURN_VAR)->ReplaceWith(pVar);
}

//------------------------------------------------------------------------------
// get Variable Obj by name in ObjLink
//------------------------------------------------------------------------------
CVar *CVar::GetParameter(const string &sName)
{
	return FindChildOrCreate(sName)->m_pVar;
}

//------------------------------------------------------------------------------
// return child have the same Name || return NULL if NOT FOUND
//------------------------------------------------------------------------------
CVarLink *CVar::FindChild(const string &sChildName) 
{
	CVarLink *pTmpVarLinkSearch = NULL;
	
	if (sChildName.size() == 0)
		goto _COMPLETE;

	pTmpVarLinkSearch = m_pVLFirstChild;
	while (pTmpVarLinkSearch) 
	{
		if (pTmpVarLinkSearch->m_sName.compare(sChildName)==0)
			goto _COMPLETE;
		pTmpVarLinkSearch = pTmpVarLinkSearch->m_pNextSibling;
	}

_COMPLETE:
	return pTmpVarLinkSearch;
}

//------------------------------------------------------------------------------
// Tim lien ket bien cua 1 bien dua vao ten Neu chua co thi tao moi
//------------------------------------------------------------------------------
CVarLink *CVar::FindChildOrCreate(const string &sChildName, const string &sAliasName, int nTypeVar)
{
	CVarLink *pVarLinkFound = FindChild(sChildName);
	if (pVarLinkFound) 
		return pVarLinkFound;

	return AddChild(sChildName, new CVar(STR_BLANK_DATA, nTypeVar), sAliasName);
}

//------------------------------------------------------------------------------
// Tim lien ket bien cua 1 bien dua vao duong dan
// Neu chua co thi tao moi
//------------------------------------------------------------------------------
CVarLink *CVar::FindChildOrCreateByPath(const string &sPath)
{
	size_t nPosDots = sPath.find('.');
	if (nPosDots == string::npos)
		return FindChildOrCreate(sPath);

	return FindChildOrCreate(sPath.substr(0,nPosDots), "", SCRIPTVAR_OBJECT)->m_pVar->
		FindChildOrCreateByPath(sPath.substr(nPosDots+1));
}

//------------------------------------------------------------------------------
// Them 1 con moi vao vi tri cuoi cung ds con
//------------------------------------------------------------------------------
CVarLink *CVar::AddChild(const string &sChildName, CVar *pVNewChild, const string &sAliasName, BOOL bRemove)
{
	CVarLink *pVLNewChild = NULL;

	if (IsUndefined()) 
	{
		m_nTypeVar = SCRIPTVAR_OBJECT;
	}
	// if no child supplied, create one
	if (!pVNewChild)
		pVNewChild = new CVar();
	pVLNewChild = new CVarLink(pVNewChild, sChildName, sAliasName);

	// co (flag) : bao du lieu nam trong LocalStack
	pVLNewChild->m_bOwned = true;

#if SAFE_MEMORY
	if (bRemove)
	{
		RemoveTempStack(pVLNewChild);
		RemoveTempStack(pVNewChild);
	}
#endif // SAFE_MEMORY


	if (m_pVLLastChild) 
	{
		m_pVLLastChild->m_pNextSibling = pVLNewChild;
		pVLNewChild->m_pPrevSibling = m_pVLLastChild;
		m_pVLLastChild = pVLNewChild;
	}
	else 
	{
		m_pVLFirstChild = pVLNewChild;
		m_pVLLastChild = pVLNewChild;
	}
	return pVLNewChild;
}

//------------------------------------------------------------------------------
// Them 1 con moi vao ko trung lap voi ds con da co. Neu da co con co ten trung 
// voi phan tu moi => thay the 
//------------------------------------------------------------------------------
CVarLink *CVar::AddChildNoDup(const string &sChildName, CVar *pVChild, const string &sAliasName, BOOL bRemove)
{
	CVarLink *pVLChild = NULL;

	// if child == null create New 
	if (!pVChild)
		pVChild = new CVar();

	// if child supplied with same name, replace or add New
	pVLChild = FindChild(sChildName);
	// tim ten cua tham so, neu chua co thi them
	if (pVLChild) 
	{
		pVLChild->ReplaceWith(pVChild);
	} else 
	{
		pVLChild = AddChild(sChildName, pVChild, sAliasName, bRemove);
	}

	return pVLChild;
}

//------------------------------------------------------------------------------
// Xoa bo 1 lien ket bien co lien ket toi 1 du lieu X
//------------------------------------------------------------------------------
void CVar::RemoveChild(CVar *pVDeleteObj) 
{
	CVarLink *pVLDeleteObj = m_pVLFirstChild;

	while (pVLDeleteObj) 
	{
		if (pVLDeleteObj->m_pVar == pVDeleteObj)
			break;
		pVLDeleteObj = pVLDeleteObj->m_pNextSibling;
	}
	if(pVLDeleteObj)
		RemoveLink(pVLDeleteObj);
}

//------------------------------------------------------------------------------
// Go 1 lien ket bien ra khoi ds con cua doi tuong
//------------------------------------------------------------------------------
void CVar::RemoveLink(CVarLink* &pVLObjDelete)
{
	try
	{
		if (!pVLObjDelete) return;
		if (pVLObjDelete->m_pNextSibling)
			pVLObjDelete->m_pNextSibling->m_pPrevSibling = pVLObjDelete->m_pPrevSibling;
		if (pVLObjDelete->m_pPrevSibling)
			pVLObjDelete->m_pPrevSibling->m_pNextSibling = pVLObjDelete->m_pNextSibling;
		if (m_pVLLastChild == pVLObjDelete)
			m_pVLLastChild = pVLObjDelete->m_pPrevSibling;
		if (m_pVLFirstChild == pVLObjDelete)
			m_pVLFirstChild = pVLObjDelete->m_pNextSibling;
		pVLObjDelete = NULL;
	}
	catch(...)
	{

	}
}

//------------------------------------------------------------------------------
// remove list Child of this Obj -> backStrack destructor
//------------------------------------------------------------------------------
void CVar::RemoveAllChildren()
{
	CVarLink *pVLObjDelete = m_pVLFirstChild;
	CVarLink *pVLTmp = NULL;

	while (pVLObjDelete) 
	{
		pVLTmp = pVLObjDelete->m_pNextSibling;
		pVLObjDelete = pVLTmp;
	}
	m_pVLFirstChild = 0;
	m_pVLLastChild = 0;
}

//------------------------------------------------------------------------------
// Tim phan tu trong Mang dua vao vi tri lien ket bien
//------------------------------------------------------------------------------
CVar *CVar::GetArrayIndex(int nIdx) 
{
	char szIdx[64];
	CVarLink *pVLTmp = NULL;

	_snprintf_s(szIdx, sizeof(szIdx), _TRUNCATE, "%d", nIdx);
	pVLTmp = FindChild(szIdx);

	if (pVLTmp) 
		return pVLTmp->m_pVar;
	else 
		return new CVar(STR_BLANK_DATA, SCRIPTVAR_NULL); // undefined
}

//------------------------------------------------------------------------------
// Cai dat du lieu cho 1 lien ket bien co vi tri xac dinh trong mang
//------------------------------------------------------------------------------
void CVar::SetArrayIndex(int nIdx, CVar *pVValue) 
{
	char sIdx[64];
	CVarLink *pVLTmp = NULL;

	_snprintf_s(sIdx, sizeof(sIdx), _TRUNCATE, "%d", nIdx);
	pVLTmp = FindChild(sIdx);

	if (pVLTmp) 
	{
		if (pVValue->IsUndefined())
			RemoveLink(pVLTmp);
		else
			pVLTmp->ReplaceWith(pVValue);
	} 
	else 
	{
		if (!pVValue->IsUndefined())
			AddChild(sIdx, pVValue);
	}
}

//------------------------------------------------------------------------------
// Lay kich thuong mang
//------------------------------------------------------------------------------
int CVar::GetArrayLength()
{
	int nVal = 0;
	int nHighest = -1;
	CVarLink *pVLTmp = NULL;

	if (!IsArray()) return 0;
	pVLTmp = m_pVLFirstChild;

	while (pVLTmp) 
	{
		if (IsNumber(pVLTmp->m_sName)) 
		{
			nVal = atoi(pVLTmp->m_sName.c_str());
			if (nVal > nHighest) 
				nHighest = nVal;
		}
		pVLTmp = pVLTmp->m_pNextSibling;
	}
	return nHighest+1;
}

//------------------------------------------------------------------------------
// Dem so phan tu con cua doi tuong
//------------------------------------------------------------------------------
int CVar::GetNumberOfChildren() 
{
	int n = 0;
	CVarLink *pVLTmp = m_pVLFirstChild;

	while (pVLTmp)
	{
		n++;
		pVLTmp = pVLTmp->m_pNextSibling;
	}
	return n;
}

//------------------------------------------------------------------------------
// Tra ve gia tri noi dung cua bien : neu la Number
// Tra ve 1 : Neu la kieu dl khac
// Tra ve 0 : Neu bien chua dinh nghia
//------------------------------------------------------------------------------
int CVar::GetInt() 
{
	/* strtol understands about hex and octal */
	if (IsInt()) return m_lData;
	if (IsNull()) return 0;
	if (IsUndefined()) return 0;
	if (IsDouble()) return (int)m_fData;
	if (IsString()) return this->GetString() != "undefined";
	if (IsFunction()) return 1;
	if (IsArray())	return 1;
	if (IsObject())	return 1;

	return 0;
}

//------------------------------------------------------------------------------
// Tra ve gia tri noi dung cua bien : neu la Number
// Tra ve 0 : Neu la kieu dl khac
//------------------------------------------------------------------------------
double CVar::GetDouble() 
{
	if (IsDouble()) return m_fData;
	if (IsInt()) return m_lData;
	if (IsNull()) return 0;
	if (IsUndefined()) return 0;
	return 0; /* or NaN? */
}

string CVar::GetRegex()
{
	string sRet = "null";
	if (IsRegex()) return m_sData;
	if (IsUndefined()) return "";
	return sRet;
}

// -----------------------------------------------------------------------------
// tra ve data Kieu string cua 1 doi tuong CVar
// -----------------------------------------------------------------------------
/*const string &*/string CVar::GetString() 
{
	/* Because we can't return a string that is generated on demand.
	* I should really just use char* :) */
	char buffer[32];
	string sNull = "null";
	string sUndefined = "undefined";
	string sArrayToString = "";
	CVarLink* pVLChild = NULL;

	// Kieu INT ----------------------------------------------------------------
	if (IsInt()) 
	{
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%ld", m_lData);
		m_sData = buffer;
		goto _COMPLETE;
	}

	// Kieu Double -------------------------------------------------------------
	if (IsDouble()) 
	{
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%f", m_fData);
		m_sData = buffer;
		goto _COMPLETE;
	}

	// Kieu Mang ---------------------------------------------------------------
	if (IsArray()) 
	{
		pVLChild = m_pVLFirstChild;
		while (pVLChild){
			if (pVLChild->m_pVar)
			{
				sArrayToString += pVLChild->m_pVar->GetString();
				if (pVLChild != m_pVLLastChild)
					sArrayToString += ",";
			}
			pVLChild = pVLChild->m_pNextSibling;
		}
		m_sData = sArrayToString;
		goto _COMPLETE;
	}

	// Kieu NativeFunction -----------------------------------------------------
	if (IsNative())
	{
		goto _COMPLETE;
	}

	// Kieu NULL ---------------------------------------------------------------
	if (IsNull())
	{
		m_sData = sNull;
		goto _COMPLETE;
	}

	// Chua dinh nghia ---------------------------------------------------------
	if (IsUndefined())
	{
		m_sData = sUndefined;
		goto _COMPLETE;
	}

	// Kieu String -------------------------------------------------------------

_COMPLETE:
	return m_sData;
}

// -----------------------------------------------------------------------------
// Xuat thuoc tinh: "dinh nghia ham" cua doi tuong.
// -----------------------------------------------------------------------------
string	CVar::GetStringDefineFunc()
{
	return string((char*)this->m_pBuffDefineFunc);
}

// -----------------------------------------------------------------------------
// Cai dat du lieu kieu Int cho doi tuong
// -----------------------------------------------------------------------------
void CVar::SetInt(int nVal)
{
	m_nTypeVar = SCRIPTVAR_INTEGER;  //xx
	m_lData = nVal;
	m_fData = 0;
	m_sData = STR_BLANK_DATA;
}

// -----------------------------------------------------------------------------
// Cai dat du lieu double
// -----------------------------------------------------------------------------
void CVar::SetDouble(double fVal)
{
	//m_nTypeVar = (m_nTypeVar&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_DOUBLE;
	m_nTypeVar = SCRIPTVAR_DOUBLE;	//xx
	m_fData = fVal;
	m_lData = 0;
	m_sData = STR_BLANK_DATA;
}

// -----------------------------------------------------------------------------
// Cai dat du lieu string
// -----------------------------------------------------------------------------
void CVar::SetString(const string &sData)
{
	// name sure it's not still a number or integer
	m_nTypeVar = SCRIPTVAR_STRING;
	m_sData = sData;
	m_lData = 0;
	m_fData = 0;
}
// -----------------------------------------------------------------------------
// Cai dat du lieu regex
// -----------------------------------------------------------------------------
void CVar::SetRegex(const string &sData)
{
	// name sure it's not still a number or integer
	m_nTypeVar = SCRIPTVAR_REGEX;
	m_sData = sData;
	m_lData = 0;
	m_fData = 0;
}

// -----------------------------------------------------------------------------
// Cai dat du lieu Undefined
// -----------------------------------------------------------------------------
void CVar::SetUndefined()
{
	// name sure it's not still a number or integer
	m_nTypeVar = SCRIPTVAR_UNDEFINED;
	m_sData = STR_BLANK_DATA;
	m_lData = 0;
	m_fData = 0;
	RemoveAllChildren();
}

// -----------------------------------------------------------------------------
// Cai dat du lieu doi tuong Array
// -----------------------------------------------------------------------------
void CVar::SetArray() 
{
	// name sure it's not still a number or integer
	//    m_nTypeVar = (m_nTypeVar&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_ARRAY;
	m_nTypeVar = SCRIPTVAR_ARRAY;
	m_sData = STR_BLANK_DATA;
	m_lData = 0;
	m_fData = 0;
	RemoveAllChildren();
}

// -----------------------------------------------------------------------------
// Cai dat du lieu trong m_sData
// -----------------------------------------------------------------------------
void CVar::SetDataString(string sData)
{
	this->m_sData = sData;
}

// -----------------------------------------------------------------------------
// So sanh 2 doi tuong du lieu
// -----------------------------------------------------------------------------
bool CVar::Equals(CVar *pVEqual)
{
	CVar *resV = MathsOp(pVEqual, TK_EQUAL);
	bool res = resV->GetBool();
	return res;
}

// -----------------------------------------------------------------------------
// Thuc hien toan tu giua 2 du lieu
// ...
// exception: if op is not install -> throw new RuntimeException.
// -----------------------------------------------------------------------------
CVar *CVar::MathsOp(CVar *pVOperandRight, int nOp)
{
	int nValLeft, nValRight;
	int nTokenIdA, nTokenIdB;
	int nIndexArray = 0;
	int	nResult = 0;
	float fResult = 0;
	double fValLeft, fValRight;
	double fValReturn = 0;
	bool bEqual = false;
	bool bIsInt = false;
	bool bIsFloat = false;
	bool bIsException = false;
	string sValA, sValB;
	string sValLeft, sValRight;

	CTokenPointer* pTPValA = NULL;
	CTokenPointer* pTPValB = NULL;

	CVar* pVContents = NULL;
	CVar* pVNewChild = NULL;

	CVarLink* pVLNewChild = NULL;
	CVarLink* pVLChildB = NULL;
	CVarLink* pVLLastChildA = NULL;
	CVarLink* pVLTmp = NULL;

	CRuntimeException* pRuntimeException = NULL;

	//
	// Type equality check
	if (nOp == TK_TYPEEQUAL || nOp == TK_NTYPEEQUAL)
	{
		// check type first, then call again to check data
		bEqual = (this->IsNumeric() && pVOperandRight->IsNumeric()) ||
			((this->m_nTypeVar & SCRIPTVAR_VARTYPEMASK) == (pVOperandRight->m_nTypeVar & SCRIPTVAR_VARTYPEMASK));
		if (bEqual) 
		{
			pVContents = this->MathsOp(pVOperandRight, TK_EQUAL);
			if (!pVContents->GetBool())
				bEqual = false;
		}

		if (nOp == TK_TYPEEQUAL)
			return new CVar(bEqual);
		else
			return new CVar(!bEqual);
	}

	//
	// do maths...
	if (this->IsUndefined() && pVOperandRight->IsUndefined())
	{
		if (nOp == TK_EQUAL)
			return new CVar(true);
		else if (nOp == TK_NEQUAL)
			return new CVar(false);
		else
			return new CVar(); // undefined
	}

	//
	// a or b is Numeric..
	else if ((this->IsNumeric() || this->IsUndefined()) &&
		(pVOperandRight->IsNumeric() || pVOperandRight->IsUndefined()))
	{
		if (!this->IsDouble() && !pVOperandRight->IsDouble()) 
		{
			// use ints
			nValLeft = this->GetInt();
			nValRight = pVOperandRight->GetInt();
			switch (nOp) 
			{
			case '+': return new CVar(nValLeft + nValRight);
			case '-': return new CVar(nValLeft - nValRight);
			case '*': return new CVar(nValLeft*nValRight);
			case '/': 
			{
						  if (nValRight == 0)
							  throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + "  not supported on the Int datatype");
						 
						  return new CVar(((double)nValLeft) / nValRight); 
			}
			case '&': return new CVar(nValLeft&nValRight);
			case '|': return new CVar(nValLeft | nValRight);
			case '^': return new CVar(nValLeft^nValRight);
			case '%': 
			{
						if (nValRight == 0)
							throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + "  not supported on the Int datatype");

						return new CVar(nValLeft%nValRight);
			}
			case TK_EQUAL:     return new CVar(nValLeft == nValRight);
			case '~':     return new CVar(~nValLeft);
			case TK_NEQUAL:    return new CVar(nValLeft != nValRight);
			case '<':     return new CVar(nValLeft<nValRight);
			case TK_LEQUAL:    return new CVar(nValLeft <= nValRight);
			case '>':     return new CVar(nValLeft>nValRight);
			case TK_GEQUAL:    return new CVar(nValLeft >= nValRight);

			default:
				throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + "  not supported on the Int datatype");
			}
		}
		else 
		{
			// use doubles
			fValLeft = this->GetDouble();
			fValRight = pVOperandRight->GetDouble();
			switch (nOp) 
			{
			case '+': return new CVar(fValLeft + fValRight);
			case '-': return new CVar(fValLeft - fValRight);
			case '*': return new CVar(fValLeft*fValRight);
			case '/':
			{
						if (fValRight == 0)
							throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + "  not supported on the Int datatype");

						return new CVar(fValLeft / fValRight);
			}
			case TK_EQUAL:     return new CVar(fValLeft == fValRight);
			case TK_NEQUAL:    return new CVar(fValLeft != fValRight);
			case '<':     return new CVar(fValLeft<fValRight);
			case TK_LEQUAL:    return new CVar(fValLeft <= fValRight);
			case '>':     return new CVar(fValLeft>fValRight);
			case TK_GEQUAL:    return new CVar(fValLeft >= fValRight);
			case '%':
			{
				try
				{	
					fValReturn = fmod(fValLeft, fValRight);
				}
				catch (exception&)
				{
					// Neu xay ra loi trong thuc hien phep toan lay Modules
					fValReturn = 1;
				}
				return new CVar(fValReturn);
			}
			default:
				throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + " not supported on the Double datatype");
			}
		}
	}

	//
	// if type(a) == array -> check array == array
	else if (this->IsArray()) 
	{
		/* Just check pointers */
		switch (nOp)
		{
		// Xu ly toan tu so sanh =
		case TK_EQUAL:
			return new CVar(this == pVOperandRight);
		// Xu ly toan tu so sanh = type
		case TK_NEQUAL:
			return new CVar(this != pVOperandRight);
		case '+':
		{
					if (pVOperandRight->IsString())
					{
						sValLeft = this->GetString();
						sValRight = pVOperandRight->GetString();
						return new CVar(sValLeft + sValRight, SCRIPTVAR_STRING);
					}

					if (pVOperandRight->IsArray())
					{
						pVLTmp = pVOperandRight->m_pVLLastChild;
						pVLChildB = pVOperandRight->m_pVLFirstChild;
						while (pVLChildB != pVLTmp)
						{
							nIndexArray = 0;

							// Lay vi tri noi tiep phan tu con cua A
							pVLLastChildA = this->m_pVLLastChild;
							if (pVLLastChildA)
								nIndexArray = pVLLastChildA->GetIntName() + 1;

							// them phan tu con cua B vao trong A
							pVNewChild = new CVar();
							pVNewChild->CopySimpleData(pVLChildB->m_pVar);
							pVLNewChild = this->AddChild("", pVNewChild);
							pVLNewChild->SetIntName(nIndexArray);

							nIndexArray++;
							pVLChildB = pVLChildB->m_pNextSibling;
						}
					}
		}

		// Khong xu ly duoc toan tu
		default:
			throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + " not supported on the Array datatype");
		}
	}

	//
	// if type(a) == object  ->  check object == object
	else if (this->IsObject()) 
	{
		/* Just check pointers */
		switch (nOp) 
		{
		case TK_EQUAL: return new CVar(this == pVOperandRight);
		case TK_NEQUAL: return new CVar(this != pVOperandRight);
		case '/':
		{
					 sValA = this->GetString();
					 if (sValA.size())
					 {
						 try
						 {
							 fValLeft = stof(sValA);
							 fValRight = pVOperandRight->GetDouble();
							 if (fValRight)
								 return new CVar(fValLeft / fValRight);
						 }
						 catch (exception&)
						 {

						 }
					 }
		}
		default: throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + " not supported on the Object datatype");
		}
	}

	//
	// try convert a, b to String -> process operator of String-String
	else 
	{

		sValLeft = this->GetString();
		sValRight = pVOperandRight->GetString();
		// use strings
		switch (nOp) 
		{
		case '+':           
			return new CVar(sValLeft + sValRight, SCRIPTVAR_STRING);
		case TK_EQUAL:     
			return new CVar(sValLeft == sValRight);
		case TK_NEQUAL:    
			return new CVar(sValLeft != sValRight);
		case '<':     
			return new CVar(sValLeft<sValRight);
		case TK_LEQUAL:    
			return new CVar(sValLeft <= sValRight);
		case '>':     
			return new CVar(sValLeft>sValRight);
		case TK_GEQUAL:   
			return new CVar(sValLeft >= sValRight);
		case '*':
		case '-':
		{
					// xu ly string - string
					pTPValA = new CTokenPointer(sValLeft);
					pTPValB = new CTokenPointer(sValRight);

					sValA = pTPValA->m_sTokenStr;
					sValB = pTPValB->m_sTokenStr;
					nTokenIdA = pTPValA->m_nTokenId;
					nTokenIdB = pTPValB->m_nTokenId;
					try
					{
						while (pTPValA->m_nTokenId == '-')
						{
							sValA += '-';
							pTPValA->Match(nTokenIdA);
							nTokenIdA = pTPValA->m_nTokenId;
							sValA += pTPValA->m_sTokenStr;
						}
						while (pTPValB->m_nTokenId == '-')
						{
							sValB += '-';
							pTPValB->Match(nTokenIdB);
							nTokenIdB = pTPValB->m_nTokenId;
							sValB += pTPValB->m_sTokenStr;
						}
					}
					catch (CRuntimeException* e)
					{
						pRuntimeException = e;
					}
					SAFE_THROW(pRuntimeException);

					try
					{
						switch (nOp)
						{
						case '-':
							// Xu ly toan tu string - string
							if (nTokenIdA == TK_FLOAT || nTokenIdB == TK_FLOAT)
							{
								fResult = stof(sValA) - stof(sValB);
								bIsFloat = true;
							}

							else if (nTokenIdA == TK_INT &&  nTokenIdB == TK_INT)
							{
								nResult = stoi(sValA, 0, 0) - stoi(sValB, 0, 0);
								bIsInt = true;
							}
							break;
						case '*':
							// Xu ly toan tu string * String
							if (nTokenIdA == TK_FLOAT || nTokenIdB == TK_FLOAT)
							{
								fResult = stof(sValA) * stof(sValB);
								bIsFloat = true;
							}

							else if (nTokenIdA == TK_INT &&  nTokenIdB == TK_INT)
							{
								nResult = stoi(sValA, 0, 0) * stoi(sValB, 0, 0);
								bIsInt = true;
							}
							break;
						default:
							break;
						}
						
					}
					catch (exception&)
					{
						bIsException = true;
					}

					if (!bIsException)
					{
						if (bIsInt)
							return new CVar(nResult);
						else if (bIsFloat)
							return new CVar(fResult);
					}
					throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + " not supported on the string datatype");

		}
		default: throw new CRuntimeException("Operation " + CTokenPointer::GetTokenStr(nOp) + " not supported on the string datatype");
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
// cop *data and m_nTypeVar
//------------------------------------------------------------------------------
void CVar::CopySimpleData(CVar *pVCopyObj)
{
	int nSize = 0;

	m_sData = pVCopyObj->m_sData;
	m_lData =pVCopyObj->m_lData;
	m_fData = pVCopyObj->m_fData;

	// fix:: them dia chi ham Navtive cho doi tuong moi
// 	m_fnCallback = val->m_fnCallback;
// 	m_fnCallbackUserData = val->m_fnCallbackUserData;
	
	// neu la doi tuong ham -> coop dinh nghia ham :define Function
	if (pVCopyObj->m_pBuffDefineFunc != NULL && pVCopyObj->m_nTypeVar == SCRIPTVAR_FUNCTION)
	{
		SAFE_DELETE_ARRAY(m_pBuffDefineFunc);
		nSize = lstrlenA((char*)pVCopyObj->m_pBuffDefineFunc);
		if (nSize > 0)
		{
			m_pBuffDefineFunc = new BYTE[nSize + 1];
			memcpy(m_pBuffDefineFunc, pVCopyObj->m_pBuffDefineFunc, nSize);
			m_pBuffDefineFunc[nSize] = 0;
		}
	}

//	m_nTypeVar = (m_nTypeVar & ~SCRIPTVAR_VARTYPEMASK) | (val->m_nTypeVar & SCRIPTVAR_VARTYPEMASK);
	m_nTypeVar = (pVCopyObj->m_nTypeVar & SCRIPTVAR_VARTYPEMASK);
}

//------------------------------------------------------------------------------
// copy all child + value
// Copy du lieu + tat ca con cua doi tuong dau vao
//------------------------------------------------------------------------------
void CVar::CopyValue(CVar *pVCopy) 
{
	CVar *pVCopied = NULL;
	CVarLink *pVLChild = NULL;

	if (pVCopy)
	{
		CopySimpleData(pVCopy);

		// remove all current children
		RemoveAllChildren();

		// copy children of 'val'
		pVLChild = pVCopy->m_pVLFirstChild;

		while (pVLChild)
		{
			// don't copy the 'parent' object...
			if (pVLChild->m_sName != STR_PROTOTYPE_CLASS)
				pVCopied = pVLChild->m_pVar->DeepCopy();
			else
				pVCopied = pVLChild->m_pVar;

			AddChild(pVLChild->m_sName, pVCopied);
			pVLChild = pVLChild->m_pNextSibling;
		}
	}
	else {
		SetUndefined();
	}
}

//------------------------------------------------------------------------------
// copy all child
//------------------------------------------------------------------------------
CVar *CVar::DeepCopy()
{
	CVar *newVar = new CVar();
	CVar *pVCopied = NULL;
	CVarLink *pVLChild = NULL;

	newVar->CopySimpleData(this);
	
	// copy children
	pVLChild = m_pVLFirstChild;
	while (pVLChild) 
	{
		// don't copy the 'parent' object...
		if (pVLChild->m_sName != STR_PROTOTYPE_CLASS)
		  pVCopied = pVLChild->m_pVar->DeepCopy();
		else
		  pVCopied = pVLChild->m_pVar;

		newVar->AddChild(pVLChild->m_sName, pVCopied);
		pVLChild = pVLChild->m_pNextSibling;
	}
	return newVar;
}

//----------------------------------------------------------------------
// Lay thong tin prototype cua vung du lieu
//----------------------------------------------------------------------
CVar* CVar::GetPrototypeOfObject()
{
	CVar*		pVObjectPrototype = NULL;
	CVarLink*	pVLObjectPrototype = NULL;
	
	// Them thong tin the jQuery neu truyen vao 1 doi tuong HTMLElement
	pVLObjectPrototype =this->FindChild(STR_PROTOTYPE_CLASS);
	if (pVLObjectPrototype)
	{
		pVObjectPrototype = pVLObjectPrototype->m_pVar;
	}

	return pVObjectPrototype;
};

//------------------------------------------------------------------------------
///< Dump out the contents of this using trace
//------------------------------------------------------------------------------
void CVar::Trace(string sIndentIn, const string &sName)
{
	string sIndent;
	CVarLink *pVLTmp = NULL;

	OutputDebug("<<MEM_LEAK>>"+ GetFlagsAsString()+ ":: "+sIndentIn + "'" 
		+ sName + "' = '" +	GetString() + "'");
	sIndent = sIndentIn+" ";
	pVLTmp = m_pVLFirstChild;

	while (pVLTmp) 
	{
	  pVLTmp->m_pVar->Trace(sIndent, pVLTmp->m_sName);
	  pVLTmp = pVLTmp->m_pNextSibling;
	}
}

//------------------------------------------------------------------------------
// Xuat kieu du lieu bien
//------------------------------------------------------------------------------
string CVar::GetFlagsAsString()
{	
	// fix:: goi cho ham typeof
	string sFlags = "undefined";
	if (m_nTypeVar&SCRIPTVAR_FUNCTION)	sFlags = "function";
	if (m_nTypeVar&SCRIPTVAR_OBJECT)	sFlags = "object";
	if (m_nTypeVar&SCRIPTVAR_ARRAY)		sFlags = "array";
	if (m_nTypeVar&SCRIPTVAR_NATIVE)	sFlags = "native";
	if (m_nTypeVar&SCRIPTVAR_DOUBLE)	sFlags = "double";
	if (m_nTypeVar&SCRIPTVAR_INTEGER)	sFlags = "integer";
	if (m_nTypeVar&SCRIPTVAR_STRING)	sFlags = "string";
	return sFlags;
}


//------------------------------------------------------------------------------
///< get Data as a parsable javascript string
//------------------------------------------------------------------------------
string CVar::GetParsableString()
{
	ostringstream ssFunc;
	CVarLink *pVLChild = NULL;

  // Numbers can just be put in directly
  if (IsNumeric())
	return GetString();

  if (IsFunction()) 
  {
	ssFunc << "function (";
	// get list of parameters
	pVLChild = m_pVLFirstChild;
	while (pVLChild) 
	{
	  ssFunc << pVLChild->m_sName;
	  if (pVLChild->m_pNextSibling) ssFunc << ",";
	  pVLChild = pVLChild->m_pNextSibling;
	}
	// add function body
	ssFunc << ") " << GetString();
	return ssFunc.str();
  }

  // if it is a string then we quote it
  if (IsString())
	return GetJSString(GetString());

  if (IsNull())
	  return "null";
  return "undefined";
}

//------------------------------------------------------------------------------
// Xuat ds phan tu con duoi dang JSON
//------------------------------------------------------------------------------
void CVar::GetJSON(ostringstream &sDestination, const string sLinePrefix)
{
	int nLenghtArr, i = 0;
	string sIndentedLinePrefix;
	CVarLink *pVLTmp = m_pVLFirstChild;

	if (IsObject()) 
	{
		sIndentedLinePrefix = sLinePrefix+"  ";
		
		// children - handle with bracketed list
		sDestination << "{ \n";
		pVLTmp = m_pVLFirstChild;
		
		while (pVLTmp) 
		{
			sDestination << sIndentedLinePrefix;
			sDestination  << GetJSString(pVLTmp->m_sName);
			sDestination  << " : ";
			
			pVLTmp->m_pVar->GetJSON(sDestination, sIndentedLinePrefix);
			pVLTmp = pVLTmp->m_pNextSibling;
			if (pVLTmp)
			{
				sDestination  << ",\n";
			}
		}
		sDestination << "\n" << sLinePrefix << "}";
	} 
	else if (IsArray()) 
	{
		sIndentedLinePrefix = sLinePrefix+"  ";
		sDestination << "[\n";
		nLenghtArr = GetArrayLength();
		
		if (nLenghtArr>10000) nLenghtArr=10000; // we don't want to get stuck here!

		for (i = 0; i < nLenghtArr; i++)
		{
			GetArrayIndex(i)->GetJSON(sDestination, sIndentedLinePrefix);
			if (i < nLenghtArr - 1) 
				sDestination << ",\n";
		}
		sDestination << "\n" << sLinePrefix << "]";
	} 
	else 
	{
		// no children or a function... just write value directly
		sDestination << GetParsableString();
	}
}


//------------------------------------------------------------------------------
// Cai dat thuoc tinh luu dia chi ham API va DataUser
//------------------------------------------------------------------------------
void CVar::SetCallback(JSCallback pFuncCallback, void *pUserdata)
{
	m_fnCallback = pFuncCallback;
	m_fnCallbackUserData = pUserdata;
}

//------------------------------------------------------------------------------
// Tang so luong lien ket den du lieu nay
//------------------------------------------------------------------------------
CVar *CVar::Ref()
{
	m_nRefs++;
	return this;
}

//------------------------------------------------------------------------------
// Giam so luong lien ket bien den du lieu nay
// Neu ko con lien ket bien nao den du lieu nay => xoa bo du lieu
//------------------------------------------------------------------------------
void CVar::Unref()
{
	if (m_nRefs <= 0) 
	{
		OutputDebug("::Canh Bao:: Bien nay da duoc giai phong!\n");
	}

	if (m_nRefs > 1 && CVar::m_bIsResetStack)
	{
		// Thuc hien Unref() neu cac phan tu Con chua lien ket bien toi Cha
		OutputDebug("MakeAllChildUnrefsTo");
		this->MakeAllChildUnrefsTo(this);
	}

// 	if ((--m_nRefs)==0){
// 		OutputDebug("delete Cvar");
// 		delete this;
// 	}
}

//------------------------------------------------------------------------------
// Xuat so luong lien ket bien toi du lieu
//------------------------------------------------------------------------------
int CVar::GetRefs() 
{
	return m_nRefs;
}

//------------------------------------------------------------------------------
// Huy Tat ca con cua doi tuong co lien ket toi Dulieu: pVWanUnref
// Tranh deadlock, lap vo han khi Stack la cay co chu trinh
//------------------------------------------------------------------------------
void CVar::MakeAllChildUnrefsTo(CVar* pVWantUnref)
{
	CVarLink *pVLObjUnref = m_pVLFirstChild;
	CVarLink *pVLTmp = NULL;

	this->m_bIsDeleting = TRUE;
	while (pVLObjUnref && pVLObjUnref->m_pVar) 
	{
		pVLTmp = pVLObjUnref->m_pNextSibling;
		
		// Neu tuong dang xoa -> khong thuc hien xoa nua! (!!Stack Orverflow)
		if (!pVLObjUnref->m_pVar->m_bIsDeleting)
			pVLObjUnref->m_pVar->MakeAllChildUnrefsTo(pVWantUnref);

		if (pVLObjUnref->m_pVar == pVWantUnref && pVLObjUnref->IsReDefine())
			pVLObjUnref->ReplaceWith(new CVar());
		pVLObjUnref = pVLTmp;
	}
	this->m_bIsDeleting = FALSE;
}
