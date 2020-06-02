#include "ProgramJs.h"


//------------------------------------------------------------------------------
// TRUE: neu noi dung the html co thuoc tinh an
// FALSE: neu noi dung the html khong co thuoc tinh an
//------------------------------------------------------------------------------
bool CProgramJs::CheckAttrHidden(string sAttrOfHtmlTag){
	int i = 0;
	int nSizeLst = 0;
	int nPosSzAttr = 0;
	int nPosValAttr = 0;
	int nSizeTagScan = 0;
	bool bHadAttrHidden = false;

	string sAttrVal;
	string sDigitVal;

	const char* szPosition[] =
	{
		"top", "bottom", "left", "right", "margin-left"
	};

	const char* szSize[] =
	{
		"width", "height"
	};
	vector<string> lstsPosition(szPosition, szPosition + 5);
	vector<string> lstsSize(szSize, szSize + 2);

	nSizeTagScan = sAttrOfHtmlTag.size();

	if (nSizeTagScan > 0)
	{
		// Kiem tra thuoc tinh display : none
		nPosValAttr = sAttrOfHtmlTag.find("display", 0);
		if (nPosValAttr != string::npos)
		{
			nPosValAttr = sAttrOfHtmlTag.find("none", nPosValAttr + 7);
			if (nPosValAttr != string::npos)
			{
				bHadAttrHidden = true;
				goto SCAN_COMPLETE;
			}
		}
		// Kiem tra thuoc tinh opacity < 0.05
		nPosValAttr = sAttrOfHtmlTag.find("opacity", 0);
		if (nPosValAttr != string::npos)
		{
			sDigitVal = GetSzDigit(sAttrOfHtmlTag.substr(nPosValAttr + 6));
			if (sDigitVal.size() > 0)
			{
				try{
					if (stof(sDigitVal) <= 0.05){
						bHadAttrHidden = true;
						goto SCAN_COMPLETE;
					}
				}
				catch (...){
				}
			}
		}

		// Kiem tra thuoc tinh visibility : hidden
		nPosValAttr = sAttrOfHtmlTag.find("visibility", 0);
		if (nPosValAttr != string::npos)
		{
			nPosValAttr = sAttrOfHtmlTag.find("hidden", nPosValAttr + 10);
			if (nPosValAttr != string::npos)
			{
				bHadAttrHidden = true;
				goto SCAN_COMPLETE;
			}
		}

		// Kiem tra thuoc tinh position = absolute | fixed
		nPosValAttr = sAttrOfHtmlTag.find("position", 0);
		if (nPosValAttr != string::npos)
		{
			nPosValAttr = sAttrOfHtmlTag.find("absolute", nPosValAttr + 8);
			if (nPosValAttr == string::npos)
			{
				nPosValAttr = sAttrOfHtmlTag.find("fixed", nPosValAttr + 8);
			}
			if (nPosValAttr != string::npos)
			{
				// Kiem tra thuoc tinh vi tri : < -30
				nSizeLst = lstsPosition.size();
				for (i = 0; i < nSizeLst; i++)
				{
					nPosSzAttr = sAttrOfHtmlTag.find(lstsPosition.at(i));
					if (nPosSzAttr != string::npos)
					{

						sAttrVal = sAttrOfHtmlTag.substr(nPosSzAttr + lstsPosition.at(i).size());
						if (sAttrVal.size() > 0)
							sDigitVal = GetSzDigit(sAttrVal);
						if (sDigitVal.size() > 0)
						{
							try
							{
								if (stoi(sDigitVal) < -30)
								{
									bHadAttrHidden = true;
									goto SCAN_COMPLETE;
								}
							}
							catch (out_of_range&)
							{
								sDigitVal = sDigitVal.substr(0, 5);
								if (stoi(sDigitVal) < -30)
								{
									bHadAttrHidden = true;
									goto SCAN_COMPLETE;
								}
							}
						}
					}
				}
			}
		}

		// Kiem tra thuoc tinh kich thuoc: < 11
		nSizeLst = lstsSize.size();
		for (i = 0; i < nSizeLst; i++)
		{
			nPosSzAttr = sAttrOfHtmlTag.find(lstsSize.at(i));
			if (nPosSzAttr != string::npos){

				sAttrVal = sAttrOfHtmlTag.substr(nPosSzAttr + lstsSize.at(i).size());
				if (sAttrVal.size() > 0)
					sDigitVal = GetSzDigit(sAttrVal);
				if (sDigitVal.size() > 0)
				{
					try
					{
						if (stoi(sDigitVal) < 11)
						{
							bHadAttrHidden = true;
							goto SCAN_COMPLETE;
						}
					}
					catch (out_of_range&)
					{
						sDigitVal = sDigitVal.substr(0, 5);
						if (stoi(sDigitVal) < 11)
						{
							bHadAttrHidden = true;
							goto SCAN_COMPLETE;
						}
					}
				}
			}
		}
	}


SCAN_COMPLETE:
	lstsSize.clear();
	lstsPosition.clear();
	return bHadAttrHidden;
}