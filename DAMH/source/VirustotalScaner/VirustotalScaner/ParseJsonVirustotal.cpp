#include "StdAfx.h"
#include "ParseJsonVirustotal.h"
#include "Jzon.h"
#include "SupportMain.h"
#include <iostream>

using namespace std;

ParseJsonVirustotal::ParseJsonVirustotal(void)
{
}

ParseJsonVirustotal::~ParseJsonVirustotal(void)
{
}

CString ParseJsonVirustotal::GetSanId(CString sJson)
{
	CString sId;
	std::string strScanId;

	CT2CA pszConvertedAnsiString (sJson);

	std::string strJson(pszConvertedAnsiString);

	Jzon::Node rootNode;
	Jzon::Parser parser;

	// parse json
	rootNode = parser.parseString(strJson);

	// get result to check reponse 
	strScanId = rootNode.get("scan_id").toString();

	CString sTmp(strScanId.c_str());

	sId = sTmp.Left(sTmp.Find(_T("-")));
	return sId;
}

DWORD ParseJsonVirustotal::ParseJsonToArray(CString sJson, CHAR* pszBuffer, DWORD dwSize)
{
	CHAR  szAvDetect[20];
	int iResult = 0;
	int iTotalAntivirus  = 0;
	int iAntivirusDetect = 0;

	CT2CA pszConvertedAnsiString (sJson);

	std::string strJson(pszConvertedAnsiString);

	Jzon::Node rootNode;
	Jzon::Parser parser;
	Jzon::Node array = Jzon::array();

	// parse json
	rootNode = parser.parseString(strJson);
	if (rootNode.isNull())
	{
		WriteLog(_T("rootNode is null"));
		return NO_SCAN_RESOURCE;
	}
	
	// get result to check reponse 
	iResult = rootNode.get("result").toInt();

	if (iResult != 1) return NO_SCAN_RESOURCE;

	// get report
	Jzon::Node nodeReport = rootNode.get("report");
	if (nodeReport.isNull())
	{
		return NO_SCAN_RESOURCE;
	}

	for (Jzon::Node::iterator it = nodeReport.begin(); it != nodeReport.end(); ++it)
	{
		Jzon::Node &node = (*it).second;
		if (node.isObject())
		{
			for (Jzon::Node::iterator i = node.begin(); i != node.end(); ++i)
			{
				iTotalAntivirus ++;

				std::string strNameAntivirus = (*i).first;
				strcat_s(pszBuffer, dwSize, strNameAntivirus.c_str());
				strcat_s(pszBuffer, dwSize, ": ");

				Jzon::Node &tmp = (*i).second;
				std::string strNameMalware = tmp.toString();

				strcat_s(pszBuffer, dwSize, strNameMalware.c_str());
				strcat_s(pszBuffer, dwSize, "\r\n");

				if (strcmp(strNameMalware.c_str(), "") != 0)
				{
					iAntivirusDetect ++;
				}
			}
		}
		
	}

	// get permalink
	Jzon::Node nodePermalink = rootNode.get("permalink");
	strcat_s(pszBuffer, dwSize, "\r\npermalink: ");
	strcat_s(pszBuffer, dwSize, nodePermalink.toString().c_str());
	strcat_s(pszBuffer, dwSize, "\r\n");

	// number antivirus detect
	strcat_s(pszBuffer, dwSize, "\r\nDetection: ");
	sprintf_s(szAvDetect, 20, "%d|%d", iAntivirusDetect, iTotalAntivirus);
	strcat_s(pszBuffer, dwSize, szAvDetect);

	return ERROR_SUCCESS;
}
