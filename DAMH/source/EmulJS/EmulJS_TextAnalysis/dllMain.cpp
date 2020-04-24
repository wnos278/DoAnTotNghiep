////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia ham DllMain. Bat + xu ly su kien khi dll duoc 
// duoc load vao hoac giai phong.
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include "ProgramJs.h"

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif // _DEBUG

CProgramJs* g_pProgramJs = NULL;
DWORD		g_dwTimeStartScan = 0;

//------------------------------------------------------------------------------
// Bat su kien khi dll duoc load vao. Thuc hien khoi tao trinh thong dich Js
// Bat su kien giai phong dll. Giai phong trinh thong dich
//------------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	//  Perform global initialization.
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DbgPrintLnA("%s", "Attach dll ...");

		g_pProgramJs = new CProgramJs();// Tao con tro chuong trinh JS
		//DisableThreadLibraryCalls(hModule);
		break;

	case DLL_PROCESS_DETACH:
		DbgPrintLnA("%s", "Detach dll ...");
		SAFE_DELETE(g_pProgramJs);
		break;
	}
	return TRUE;
}
