
#include "ExtractFeature.h"
#include <map>
#include <cmath>
#include <fstream>

#pragma warning(disable:4996)
using namespace std;

// Tra ve gia tri entropy cua doan code ban dau
void CProgramJs::Entropy(string sCodeJS)
{
	int sizeOfCode = 0;
	map<char, int> entropies;

	// Tao mot mang chua tat ca cac ky tu ton tai trong file, 
	// ham nay dat o vi tri sau khi da xu ly het comment
	for (int i = 0; i < sCodeJS.size(); i++)
	{
		if ((sCodeJS.at(i) == ' ') 
			|| (sCodeJS.at(i) == '\n') 
			|| (sCodeJS.at(i) == '\t') 
			|| (sCodeJS.at(i) == '\0')) continue;
		std::map<char, int>::iterator it = entropies.find(sCodeJS.at(i));
		if (it != entropies.end())
			it->second += 1;
		else entropies.insert({ sCodeJS.at(i), 1 });
		sizeOfCode += 1;
	}
	// Get max count character in code
	int currentMax = 0;
	unsigned checked = 0;
	char maax_key = '\0';
	int max_value = 0;
	for (auto it = entropies.cbegin(); it != entropies.cend(); ++it)
		if (it->second > currentMax) maax_key = it->first;

	// trich chon ky tu xuat hien nhieu nhat trong file
	m_feature.maxAppearChar = maax_key;
	// Get entropy
	double entropy = 0.0;
	for (auto it = entropies.cbegin(); it != entropies.cend(); ++it)
	{
		int temp = it->second;
		entropy += (-1) * ((temp*(1.0)) / sizeOfCode) * (log10((temp*(1.0)) / sizeOfCode));
	}
	//
	m_feature.entropy = entropy;
	return;
}

// Tra ve so luong cau comment trong code js dang <!-- //-->
void CProgramJs::CountSuspiciousCommentStyle(string sCodeJS)
{
	//sCodeJS la code goc 
	int numSuspiciousComment = 0;
	int startPos = 0;
	int count1 = 0, count2 = 0;
	if (sCodeJS.find("<!--") == true && sCodeJS.find("//-->") == true)
	{
		startPos = sCodeJS.find("<!--");
		while (startPos != string::npos)
		{
			count1 += 1;
			startPos = sCodeJS.find("<!--", startPos + 1);
		}
		startPos = sCodeJS.find("//-->");;
		while (startPos != string::npos)
		{
			count2 += 1;
			startPos = sCodeJS.find("//-->", startPos + 1);
		}
		numSuspiciousComment = (count1 + count2) / 2;
	}
	// Them thong tin vao thuoc tinh chung de in ra sau nay
	m_feature.suspicious_comment += numSuspiciousComment;

	return;
}

void CProgramJs::ExportCSV(string dir, string type)
{
	// chuan bi du lieu de in ra
	string data = to_string(m_feature.entropy) + ","
		+ to_string(m_feature.maxAppearChar) + ","
		+ to_string(m_feature.suspicious_comment) + ","
		+ to_string(m_feature.sf.countCharCodeAt) + ","
		+ to_string(m_feature.sf.countConcat) + ","
		+ to_string(m_feature.sf.countCreateElement) + ","
		+ to_string(m_feature.sf.countCreateXMLHTTPRequest) + ","
		+ to_string(m_feature.sf.countDateObjectToGMTString) + ","
		+ to_string(m_feature.sf.countDocumentAddEventListener) + ","
		+ to_string(m_feature.sf.countDocumentWrite) + ","
		+ to_string(m_feature.sf.countElementAddEventListener) + ","
		+ to_string(m_feature.sf.countElementAppendChild) + ","
		+ to_string(m_feature.sf.countElementChangeAttribute) + ","
		+ to_string(m_feature.sf.countElementInnerHTML) + ","
		+ to_string(m_feature.sf.countElementInsertBefore) + ","
		+ to_string(m_feature.sf.countElementReplaceChild) + ","
		+ to_string(m_feature.sf.countEval) + ","
		+ to_string(m_feature.sf.countFromCharCode) + ","
		+ to_string(m_feature.sf.countGetAppName) + ","
		+ to_string(m_feature.sf.countGetCookie) + ","
		+ to_string(m_feature.sf.countGetUserAgent) + ","
		+ to_string(m_feature.sf.countIndexOf) + ","
		+ to_string(m_feature.sf.countLocationAssign) + ","
		+ to_string(m_feature.sf.countLocationReplace) + ","
		+ to_string(m_feature.sf.countNewActiveXObject) + ","
		+ to_string(m_feature.sf.countSetCookie) + ","
		+ to_string(m_feature.sf.countSplit) + ","
		+ to_string(m_feature.sf.countUnescape) + ","
		+ to_string(m_feature.sf.countWindowSetInterval) + ","
		+ to_string(m_feature.sf.countWindowSetTimeOut) + "," + type
		;

	fstream fout;
	fout.open(dir, ios::out | ios::app);
	fout << data << endl;
}

void CProgramJs::CheckFunctionCall(char* object_name, char* function_name)
{
	string function = function_name;
	if (function.size() == 0)
		return;
	if (function == "write" || function == "writeln")
		m_feature.sf.countDocumentWrite += 1;
	if ((function == "charCodeAt"))
		m_feature.sf.countCharCodeAt += 1;
	if ((function == "concat"))
		m_feature.sf.countConcat += 1;
	if ((function == "createElement"))
		m_feature.sf.countCreateElement += 1;
	if ((function == "indexOf"))
		m_feature.sf.countIndexOf += 1;
	if ((function == "createXMLHTTPRequest"))
		m_feature.sf.countCreateXMLHTTPRequest += 1;
	if ((function == "ObjectToGMTString"))
		m_feature.sf.countDateObjectToGMTString += 1;
	if (function == "appendChild")
		m_feature.sf.countElementAppendChild += 1;
	if (function == "addEventListener")
		m_feature.sf.countDocumentAddEventListener += 1;
	if (function == "addEventListener")
		m_feature.sf.countElementAddEventListener += 1;
	if ((function == "changeAttribute"))
		m_feature.sf.countElementChangeAttribute += 1;
	if ((function == "innerHTML"))
		m_feature.sf.countElementInnerHTML += 1;
	if ((function == "insertBefore"))
		m_feature.sf.countElementInsertBefore += 1;
	if ((function == "replaceChild"))
		m_feature.sf.countElementReplaceChild += 1;
	if ((function == "eval"))
		m_feature.sf.countEval += 1;
	if ((function == "fromCharCode"))
		m_feature.sf.countFromCharCode += 1;
	if ((function == "getAppName"))
		m_feature.sf.countGetAppName += 1;
	if ((function == "getCookie"))
		m_feature.sf.countGetCookie += 1;
	if ((function == "getUserAgent"))
		m_feature.sf.countGetUserAgent += 1;
	if ((function == "indexOf"))
		m_feature.sf.countIndexOf += 1;
	if ((function == "assign"))
		m_feature.sf.countLocationAssign += 1;
	if ((function == "replace"))
		m_feature.sf.countLocationReplace += 1;
	if ((function == "activeXObject"))
		m_feature.sf.countNewActiveXObject += 1;
	if ((function == "setCookie"))
		m_feature.sf.countSetCookie += 1;
	if ((function == "split"))
		m_feature.sf.countSplit += 1;
	if ((function == "unescape"))
		m_feature.sf.countUnescape += 1;
	if ((function == "setInterval"))
		m_feature.sf.countWindowSetInterval += 1;
	if ((function == "setTimeOut"))
		m_feature.sf.countWindowSetTimeOut += 1;
}


void CProgramJs::ResetFeature()
{
	// Khoi tao gia tri cho cau truc m_listSus
	m_feature.sf.countCharCodeAt = 0;
	m_feature.sf.countConcat = 0;
	m_feature.sf.countCreateElement = 0;
	m_feature.sf.countCreateXMLHTTPRequest = 0;
	m_feature.sf.countDateObjectToGMTString = 0;
	m_feature.sf.countDocumentAddEventListener = 0;
	m_feature.sf.countDocumentWrite = 0;
	m_feature.sf.countElementAddEventListener = 0;
	m_feature.sf.countElementAppendChild = 0;
	m_feature.sf.countElementChangeAttribute = 0;
	m_feature.sf.countElementInnerHTML = 0;
	m_feature.sf.countElementInsertBefore = 0;
	m_feature.sf.countElementReplaceChild = 0;
	m_feature.sf.countEval = 0;
	m_feature.sf.countFromCharCode = 0;
	m_feature.sf.countGetAppName = 0;
	m_feature.sf.countGetCookie = 0;
	m_feature.sf.countGetUserAgent = 0;
	m_feature.sf.countIndexOf = 0;
	m_feature.sf.countLocationAssign = 0;
	m_feature.sf.countLocationReplace = 0;
	m_feature.sf.countNewActiveXObject = 0;
	m_feature.sf.countSetCookie = 0;
	m_feature.sf.countSplit = 0;
	m_feature.sf.countUnescape = 0;
	m_feature.sf.countWindowSetInterval = 0;
	m_feature.sf.countWindowSetTimeOut = 0;

	m_feature.entropy = 0;
	m_feature.suspicious_comment = 0;
	m_feature.maxAppearChar = '\0';

	return;
}

int getPosStr1(string szData, string szTagName, int nPosStart){

	transform(szData.begin(), szData.end(), szData.begin(), ::tolower);
	return szData.find(szTagName, nPosStart);
}

void CProgramJs::PreProcess(string sCodeHtml)
{
	string sCodeScript;
	int nPosTagScriptStart = -1;
	int nPosTagScriptEnd = -1;
	int nPosStart = -1;
	int nPosEnd = -1;

	do
	{
		nPosStart = getPosStr1(sCodeHtml, "<script", nPosEnd + 1);
		nPosTagScriptStart = nPosStart;
		if (nPosStart != string::npos)
		{
			nPosStart = sCodeHtml.find('>', nPosStart);

			if (nPosStart != string::npos)
			{
				nPosStart++;
				nPosEnd = getPosStr1(sCodeHtml, "</script", nPosStart);
				if (nPosEnd != string::npos)
					nPosTagScriptEnd = sCodeHtml.find('>', nPosEnd + 1);
			}
			else break;
		}
		else
			break;

		if (nPosStart != string::npos && nPosEnd != string::npos)
		{
			if ((nPosEnd - nPosStart) > 0)
			{
				sCodeScript = sCodeHtml.substr(nPosStart, nPosEnd - nPosStart) + ";";
				CountSuspiciousCommentStyle(sCodeScript);
			}
		}
	} while (nPosStart != string::npos && nPosEnd != string::npos);
	CountSuspiciousCommentStyle(sCodeScript);
}

void CProgramJs::SaveToFile(string name_file, string data)
{
	FILE *f;
	f = fopen(name_file.c_str(), "w");
	countt2 += 1;
	fwrite(data.c_str(), data.size(), 1, f);
	fclose(f);
}