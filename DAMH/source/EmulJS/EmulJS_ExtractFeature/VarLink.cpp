////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac phuong thuc lop CVarLink. Lop lien ket bien
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include "VarLink.h"

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

int CVarLink::m_iStt = 0;

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
CVarLink::CVarLink(CVar *pVData, const string &sName)
{
 #if SAFE_MEMORY
	AddTempStack(this);
	if (pVData == m_pVRootStack && m_pVRootStack)
 		AddListPointToStack(this);
#endif
	this->m_sName = sName;
	this->m_pNextSibling = 0;
	this->m_pPrevSibling = 0;
	if (pVData)
		this->m_pVar = pVData->Ref();

	this->m_bOwned = false;
	this->m_bReDefine = true;
}

// -----------------------------------------------------------------------------
// Copy constructor
// -----------------------------------------------------------------------------
CVarLink::CVarLink(const CVarLink &pVLBefore)
{
#if SAFE_MEMORY
	AddListCVarLink(this);
	AddTempStack(this);
	if (pVLBefore.m_pVar == m_pVRootStack)
		AddListPointToStack(this);
#endif

	this->m_sName = pVLBefore.m_sName;
	this->m_pNextSibling = 0;
	this->m_pPrevSibling = 0;
	if (pVLBefore.m_pVar)
		this->m_pVar = pVLBefore.m_pVar->Ref();
	this->m_bOwned = false;
	this->m_bReDefine = true;
	m_iStt ++;
	m_iSttClass = m_iStt;
}

CVarLink::~CVarLink() 
{
#if SAFE_MEMORY
	//RemoveTempStack(this);
 	if ( m_pVar == m_pVRootStack)
	{
		OutputDebug("CVarLink - m_pVRootStack");
		RemoveFromListPointToStack(this);
	}
#endif
}

bool CVarLink::Close()
{
#if SAFE_MEMORY
	RemoveTempStack(this);
	if ( m_pVar == m_pVRootStack)
	{
		OutputDebug("CVarLink - m_pVRootStack");
		RemoveFromListPointToStack(this);
	}
#endif
	if (m_pVar)
	{
		m_pVar->Unref();
		OutputDebug("m_pVar - Unref");
	}
	return true;
}

// -----------------------------------------------------------------------------
// Tao bo lien ket voi CVar truyen vao;  Huy lien ket voi CVar cu
// ........
// exception: Thay doi noi dung bien khong duoc phep dinh nghia lai
// -----------------------------------------------------------------------------
bool CVarLink::ReplaceWith(CVar *pVNew)
{
	CVar *pVOld = NULL;
	if (IsReDefine())
	{

#if SAFE_MEMORY
		if (m_pVRootStack && pVNew == m_pVRootStack)
			AddListPointToStack(this);
		if (m_pVar == m_pVRootStack)
			RemoveFromListPointToStack(this);
// 		if (m_bOwned)
// 			RemoveTempStack(pVNew);
#endif

		pVOld = m_pVar;
		m_pVar = pVNew->Ref();
		//pVOld->Unref();
	}
	else 
		throw new CRuntimeException("Bien khong duoc phep dinh nghia lai");

	return true;
}

// -----------------------------------------------------------------------------
// Tao lien ket voi du lieu CVar trong CVarLink; Huy lien ket voi du lieu CVar cu
// ...
// Exception: Thay doi noi dung bien mac dinh (CVarLink::m_bRedefine = false)
// -----------------------------------------------------------------------------
bool CVarLink::ReplaceWith(CVarLink *pVLNew) 
{
	bool bResult;
	CVar* pVNew = NULL;

	if (pVLNew)
		bResult = ReplaceWith(pVLNew->m_pVar);
	else
	{
		pVNew = new CVar();
		bResult = ReplaceWith(pVNew);
	}
	return bResult;
}

// -----------------------------------------------------------------------------
// Tao lien ket voi du lieu CVar trong CVarLink; Huy lien ket voi du lieu CVar cu
// Copy ca ten cua lien ket bien cu
// ...
// Exception: Thay doi noi dung bien mac dinh (CVarLink::m_bRedefine = false)
// -----------------------------------------------------------------------------
bool CVarLink::ReplaceAllNameWith(CVarLink *pVLNew)
{
	bool bResult;

	if (pVLNew)
		this->m_sName = pVLNew->m_sName;
	bResult = this->ReplaceWith(pVLNew);
	return bResult;
}

// -----------------------------------------------------------------------------
// Chuyen ten bien thanh phan (trong mang) => kieu Int 
// -----------------------------------------------------------------------------
int CVarLink::GetIntName() 
{
	try
	{
		return atoi(m_sName.c_str());
	}
	catch (exception&)
	{
		throw new CRuntimeException("Name Array Error!: " + m_sName);
	}
}

// -----------------------------------------------------------------------------
// Cai dat ten bien thanh phan (trong mang) = kieu Int 
// -----------------------------------------------------------------------------
void CVarLink::SetIntName(int nIndex) 
{
	char sIdx[64];
	_snprintf_s(sIdx, sizeof(sIdx), _TRUNCATE, "%d", nIndex);
	m_sName = sIdx;
}
