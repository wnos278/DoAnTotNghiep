///////////////////////////////////////////////////////////////////////////
//
// Description: extern registerFunctions(CFileInput *) - have some 
//				API default was define in APISupport.cpp 
// Author: PhuongNMe
// Copyright, Bkav, 2013-2014. All rights reserved
// Additional information: Cover project "tinyJS" 
//////////////////////////////////////////////////////////////////////////

#pragma once

// APISupport  define all API which java Script used
// example: String.IndexOf(), ...
#ifndef API_SUPPORT_H
#define API_SUPPORT_H

#include "Var.h"
#include "DATA_DEF.h"
#include "ProgramJs.h"
#include <regex>
//------------------------------------------------------------------------------
/// Register useful functions with the JS interpreter
//------------------------------------------------------------------------------
extern void RegisterFunctions(CProgramJs *pFileJS);

//------------------------------------------------------------------------------
// Ham toan cuc ho tro xu ly String
//------------------------------------------------------------------------------
string StrimString(string sInput, char cCheck);
string Unescape(string sInputCodeHtml);
vector<string> SplitString(string sInput, char cBoundary);

//------------------------------------------------------------------------------
// Ham toan cuc ho tro lay xau string dau tien ke tu vi tri dwPosBegin
//------------------------------------------------------------------------------
string GetFirstString(string sInput, DWORD dwPosBegin = 0);

//------------------------------------------------------------------------------
// Ham toan cuc ho tro loc "chu so" tu String
//------------------------------------------------------------------------------
string GetSzDigit(string sInput); 
typedef struct strDetectDownlaoder 
{
	bool bCreateObject;
	bool bOpen;
	bool bsaveToFile;
	bool bRun;
	bool bString;
	bool bFor7k;
}DETECT_DOWNLOADER, *PDETECT_DOWNLOADER;
typedef struct strDetectVirusOther
{
	bool bForDecrypRedirect;
	bool bDocumentWriteRedirect;
}DETECT_VIRUSOTHER, *PDETECT_VIRUSOTHER;

#endif




