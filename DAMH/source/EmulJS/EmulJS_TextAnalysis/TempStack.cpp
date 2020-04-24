////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia cac ham xoa bo bo nho tam luu tru ds cac bien 
// cap phat nhung chua giai phong, khong nam trong stack
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include "DATA_DEF.h"
#include "Var.h"
#include "VarLink.h"
#include <sstream>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

#if SAFE_MEMORY

vector<CVar*> g_AllocatedVars;			// ds Data tao ra ko duoc luu trong stack
vector<CVarLink*> g_AllocatedLinks;		// ds bien tao ra ko duoc luu trong stack
vector<CVarLink*> g_lstVLToRootStack;	// ds bien lien ket bien toi RootStack
vector<CTokenPointer*> g_AllocatedToken;// ds token da cap phat

// Tao danh sach doi tuong de xoa sau khi dung xong.
vector<CVar*> g_ListAllCVars;
vector<CVarLink*> g_ListAllCVarLink;

void AddListToken(CTokenPointer* cToken)
{
	g_AllocatedToken.push_back(cToken);
}

void CleanListToken()
{
	// Clean CSubFunction
	for (vector<CTokenPointer*>::iterator itorCToken = g_AllocatedToken.begin(); itorCToken != g_AllocatedToken.end(); itorCToken ++)
	{
		try
		{
			SAFE_DELETE(*itorCToken);
		}
		catch (std::exception ex)
		{
			continue;	
		}

	}
	g_AllocatedToken.clear();
}

void AddListCVar(CVar* cVar)
{
	g_ListAllCVars.push_back(cVar);
}

void AddListCVarLink(CVarLink* cVarLink)
{
	g_ListAllCVarLink.push_back(cVarLink);
}

void DeleteAllCVar()
{
	// Clean CSubFunction
	for (vector<CVar*>::iterator itorCSubFunction = g_ListAllCVars.begin(); itorCSubFunction != g_ListAllCVars.end(); itorCSubFunction ++)
	{
		try
		{
			SAFE_DELETE(*itorCSubFunction);
		}
		catch (std::exception ex)
		{
			continue;	
		}

	}
	g_ListAllCVars.clear();
}

void DeleteAllCVarLink()
{
	// Clean CSubFunction
	for (vector<CVarLink*>::iterator itorCSubFunction = g_ListAllCVarLink.begin(); itorCSubFunction != g_ListAllCVarLink.end(); itorCSubFunction ++)
	{
		try
		{
			SAFE_DELETE(*itorCSubFunction);
		}
		catch (std::exception ex)
		{
			continue;	
		}

	}
	g_ListAllCVarLink.clear();
}



//------------------------------------------------------------------------------
// Them 1 du lieu bien vao g_AllocatedVars
//------------------------------------------------------------------------------
void AddTempStack(CVar *pVNew) 
{
	g_AllocatedVars.push_back(pVNew);
	if (g_AllocatedVars.size() == 14)
		return;
}

//------------------------------------------------------------------------------
// Xoa bo 1 du lieu bien ra khoi g_AllocatedVars
//------------------------------------------------------------------------------
void RemoveTempStack(CVar *pVObjDel) 
{
	size_t i = 0;
	for (i = 0; i < g_AllocatedVars.size(); i++) 
	{
		if (g_AllocatedVars[i] == pVObjDel)
		{
			g_AllocatedVars.erase(g_AllocatedVars.begin() + i);
			break;
		}
	}
}

void CleanVTempStack()
{
	// Clean CVarLink
	for (vector<CVar*>::iterator itorCVar = g_AllocatedVars.begin(); itorCVar != g_AllocatedVars.end(); itorCVar ++)
	{
		try
		{
			SAFE_DELETE(*itorCVar);
		}
		catch (std::exception ex)
		{
			continue;	
		}

	}
	g_AllocatedVars.clear();
}
//------------------------------------------------------------------------------
// Them 1 lien ket bien vao g_AllocatedLinks
//------------------------------------------------------------------------------
void AddTempStack(CVarLink *pVLNew) 
{
	g_AllocatedLinks.push_back(pVLNew);
}

//------------------------------------------------------------------------------
// Xoa bo 1 lien ket bien ra khoi g_AllocatedLinks
//------------------------------------------------------------------------------
void RemoveTempStack(CVarLink *pVLObjDel)
{
	size_t i = 0;
	vector<CVarLink*>::iterator itor = g_AllocatedLinks.begin();
	while( itor != g_AllocatedLinks.end() ) 
	{
		if( *itor ==  pVLObjDel) 
		{
			itor = g_AllocatedLinks.erase( itor );
			break;
		} 
		else 
		{
			++itor;
		}
	}
}

//------------------------------------------------------------------------------
// Xoa bo toan bo ds lien ket bien tam thoi luu trong g_AllocatedLinks
//------------------------------------------------------------------------------
void CleanVLTempStack()
{
	// Clean CVarLink
	for (vector<CVarLink*>::iterator itorCVarLink = g_AllocatedLinks.begin(); itorCVarLink != g_AllocatedLinks.end(); itorCVarLink ++)
	{
		try
		{
			SAFE_DELETE(*itorCVarLink);
		}
		catch (std::exception ex)
		{
			continue;	
		}

	}
	g_AllocatedLinks.clear();
}

//------------------------------------------------------------------------------
// Xoa bo toan bo ds du lieu bien tam thoi luu trong g_AllocatedVar
// Xoa bo toan bo ds lien ket bien tam thoi luu trong g_AllocatedLinks
// Su dung - CPrograme::resetDatabase()
//------------------------------------------------------------------------------
void CleanTempStack() 
{
	int nSizeLink = g_AllocatedLinks.size();
	int nSizeVar = 0;
	bool bArrayChange = false;
	ostringstream ssMsg;
	CVarLink *pVLObjDelete = NULL;
	CVar *pVObjDelete = NULL;

	try 
	{
		CleanListToken();
		CleanVTempStack();
		CleanVLTempStack();
// #ifndef BUID_UPDATE
// 		// Hien thi thong bao mem co kha nang leak
// 		if (g_AllocatedVars.size()){
// 			ssMsg << "\n" << nSizeLink << " CVarLink; " << g_AllocatedVars.size() << " CVar";
// 			//OutputDebugStringA(ssMsg.str().c_str());
// 			ssMsg.clear();
// 		}
// #endif // !BUID_UPDATE
// 
// 		// Xoa bo vung nho dem luu tru lien ket bien
// 		while (nSizeLink > 0)
// 		{
// 			ssMsg.str("");
// 			ssMsg << "ALLOCATED LINK " << g_AllocatedLinks[nSizeLink-1]->m_sName <<
// 				", allocated[" << g_AllocatedLinks[nSizeLink-1]->m_pVar->GetRefs() << "] to \n";
// 			OutputDebug(ssMsg.str());
// 
// 			//	g_AllocatedLinks[i]->m_pVar->trace("  ");
// 
// 			pVLObjDelete = g_AllocatedLinks.at(nSizeLink-1);
// 			SAFE_DELETE(pVLObjDelete);
// 			nSizeLink = g_AllocatedLinks.size();
// 		}
// 
// 		// Xoa bo vung nho dem luu tru du lieu bien
// 		nSizeVar = 0;
// 		while (nSizeVar < (int)g_AllocatedVars.size())
// 		{
// 			if (bArrayChange)
// 				nSizeVar = 0;	// reset i to delete head Array
// 
// 			ssMsg.str("");
// 			ssMsg << "ALLOCATED, " << g_AllocatedVars[nSizeVar]->GetRefs() << "refs\n ";
// 			OutputDebug(ssMsg.str());
// 
// 			pVObjDelete = g_AllocatedVars.at(nSizeVar);
// 
// 			if (pVObjDelete && pVObjDelete->GetRefs() == 0)
// 			{
// 				// delete element at g_AllocatedVars[i]
// 				SAFE_DELETE(pVObjDelete);
// 				bArrayChange = true;
// 			}
// 			else {
// 				// delete next element: g_AllocatedVars[i+1]
// 				nSizeVar++;
// 				bArrayChange = false;
// 			}
// 		}
// 
// 		g_AllocatedVars.clear();
// 		g_AllocatedLinks.clear();
	}
	catch(...)
	{

	}
}

//------------------------------------------------------------------------------
// Them 1 bien tro toi stack trong ds g_lstVLToRootStack
//------------------------------------------------------------------------------
void AddListPointToStack(CVarLink *pVLNew)
{
	g_lstVLToRootStack.push_back(pVLNew);
}

//------------------------------------------------------------------------------
// Xoa bo 1 bien tro toi stack trong ds g_lstVLToRootStack
//------------------------------------------------------------------------------
void RemoveFromListPointToStack(CVarLink *pVLObjDel)
{
	size_t i = 0;
	for (i = 0; i < g_lstVLToRootStack.size(); i++) 
	{
		if (g_lstVLToRootStack[i] == pVLObjDel) {
			g_lstVLToRootStack.erase(g_lstVLToRootStack.begin() + i);
			break;
		}
	}
}

//------------------------------------------------------------------------------
// Xoa bo tat ca ds cac bien tro toi stack trong ds g_lstVLToRootStack
//------------------------------------------------------------------------------
void CleanListPointToStack()
{
	CVarLink* pVLTemp = NULL;
	try
	{
		int nSize = g_lstVLToRootStack.size();
		int i = 0;

		for (i = 0; i < nSize; i++)
		{
			pVLTemp = g_lstVLToRootStack.at(0);
			if (pVLTemp->IsReDefine())
				pVLTemp->ReplaceWith(new CVar());
		}

		g_lstVLToRootStack.clear();
	}
	catch (...)
	{

	}
}

#endif

//------------------------------------------------------------------------------
// Hàm xử lý in ra màn hình OutputDebugString
//------------------------------------------------------------------------------
void DbgPrintA(LPCSTR szFormat, ...)
{
#ifndef BUID_UPDATE
#ifndef _DEBUG_CHECK_MEM_LEAK
#if defined _DEBUG || defined _RELEASE_LOG
	// Dump debug information to Debug Window
	char szBuffer[1024];

	va_list ap;
	va_start(ap, szFormat);
	vsprintf_s(szBuffer, 1024, szFormat, ap);
	OutputDebugStringA(szBuffer);

	//write log file
	//	char szPathLogFile[MAX_PATH];
	// 	strcpy_s(szPathLogFile, "EmulPE.xml");
	// 	printf(szBuffer);
	// 	FILE* pFile = fopen(szPathLogFile, "at");
	// 	fprintf(pFile, "%s\n", szBuffer);
	// 	fclose(pFile);

	va_end(ap);
#endif
#endif // !_DEBUG_CHECK_MEM_LEAK
#endif // !BUID_UPDATE
}

//------------------------------------------------------------------------------
// Hàm xử lý in ra màn hình OutputDebugString
//------------------------------------------------------------------------------
void DbgPrintLnA(LPCSTR szFormat, ...)
{
#ifndef BUID_UPDATE
#ifndef _DEBUG_CHECK_MEM_LEAK
#if defined _DEBUG || defined _RELEASE_LOG
	// Dump debug information to Debug Window
	char szBuffer[1024];

	va_list ap;
	va_start(ap, szFormat);
	vsprintf_s(szBuffer, 1024, szFormat, ap);
	OutputDebugStringA(szBuffer);
	OutputDebugStringA("\n");

	va_end(ap);
#endif
#endif // !_DEBUG_CHECK_MEM_LEAK
#endif // !BUID_UPDATE
}

