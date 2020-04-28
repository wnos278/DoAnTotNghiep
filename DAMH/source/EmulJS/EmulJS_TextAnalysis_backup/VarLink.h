////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia lop CVarLink; Lop dai dien cho 1 lien ket bien, 
// Cac bien sinh ra trong qua trinh thong dich code JS.
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef VARIABLELINK_H
#define VARIABLELINK_H

#include "DATA_DEF.h"
#include "Var.h"
#include <string>
#include <iostream>
using namespace std ;


class CVar;

//------------------------------------------------------------------------------
// ClassName: CVarLink
// Description: have properties is a CVar,
//				CVarLink like as mask to connect other CVar 
//------------------------------------------------------------------------------
class CVarLink
{
public:
	string		m_sName;
	CVarLink*	m_pNextSibling;
	CVarLink*	m_pPrevSibling;
	CVar*		m_pVar;
	
	bool		m_bOwned;
	int			m_iSttClass;

	// add alias name
	string		m_sAliasName;

	CVarLink(CVar *pVData, const string &sName = STR_TEMP_NAME, const string &sAliasName = "");
	CVarLink(const CVarLink &pVLBefore); ///< Copy constructor
	~CVarLink();
	
	// -------------------------------------------------------------------------
	// Tao bo lien ket voi CVar truyen vao; 
	// Huy lien ket voi CVar cu
	// -------------------------------------------------------------------------
	bool Close();
	bool ReplaceWith(CVar *pVNew);		///< Replace the Variable pointed to

	// -------------------------------------------------------------------------
	// Tao lien ket voi du lieu CVar trong CVarLink
	// Huy lien ket voi du lieu CVar cu
	// -------------------------------------------------------------------------
	bool ReplaceWith(CVarLink *pVLNew); ///< Replace the Variable pointed to 
										///	(just dereferences)
	
	// -------------------------------------------------------------------------
	// Tao lien ket voi du lieu CVar trong CVarLink
	// Huy lien ket voi du lieu CVar cu
	// copy m_sName cua CVarlink cu
	// -------------------------------------------------------------------------
	bool CVarLink::ReplaceAllNameWith(CVarLink *pVLNew);

	// -------------------------------------------------------------------------
	// Xu ly index cua cac phan tu trong mang
	// -------------------------------------------------------------------------
	int	GetIntName();		///< Get the name as an integer (for arrays)
	void SetIntName(int n); ///< Set the name as an integer (for arrays)


	// -------------------------------------------------------------------------
	// Bao ve cac Ham( API support) + doi tuong duoc dinh nghia tu truoc
	// -------------------------------------------------------------------------
	void SetNotReDefine(){ m_bReDefine = false; };
	bool IsReDefine(){ return m_bReDefine; };

	// Cai dat con tro RootStack cho Lien ket bien
	static void SetRootStack(CVar* pVRootStack){ m_pVRootStack = pVRootStack; };

private:
	bool		m_bReDefine;
	static CVar* m_pVRootStack;
	static int	m_iStt;
	
};
#endif // !VARIABLELINK_H