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

		// 		nPosValAttr = sAttrOfHtmlTag.find("style", 0);
		// 		if (nPosValAttr != string::npos)
		// 		{
		// 			nPosValAttr = sAttrOfHtmlTag.find("display:none", nPosValAttr + 20);
		// 			if (nPosValAttr != string::npos){
		// 				bHadAttrHidden = true;
		// 				goto SCAN_COMPLETE;
		// 			}
		/*		}*/
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
//// chexk virus theo đoạn ma
//bool CProgramJs::CheckVirusStatic(string sCodeJS)
//{
//
//	CRuntimeException *pREHadVirus = NULL;
//	bool bRet = false;
//	const char* args[] =
//	{
//		"function ClickJackFbHide(){",
//		"jQuery(\"div[id^=\\'clickjack-button-wrapper\\']\").hide();",
//		"function ClickJackFbShow(){",
//		"jQuery(\"div[id^=\\'clickjack-button-wrapper\\']\").show();"
//	};
//
//	const char* args1[] =
//	{
//		"document.referrer.indexOf(location.protocol",
//		"/js/jquery.min.php?",
//		"&c_utm='+encodeURIComponent(",
//		"&se_referrer='+encodeURIComponent(document.referrer)"
//	};
//	const char* args2[] =
//	{
//		
//        "#clickjack-button-wrapper-5",
//		"setTimeout(\"clickjack_hider()\",5000)",
//		".mouseout(function(){ClickJackFbShow()",
//		".mouseover(function(){ClickJackFbHide()"
//	};
//	const char* args3[]  =
//	{
//		"'9091968376','8887918192818786347374918784939277359287883421333333338896'",
//		"String.fromCharCode(parseInt(",
//		"document.write('<'+"
//
//	};
//	const char* args4[] =
//	{
//		"window.location =",
//		"setTimeout(continueExecution, 200)",
//		"timeFrame=setInterval(\"lpcAutoLike()",
//		"$(document.activeElement).attr('id')==\"fbframe"
//	};
//	const char* args5[] =
//	{
//		"(!''.replace(/^/,String)){while(c--)",
//		".replace(new RegExp(",
//		"else|window|m|true|all|c|indexOf|v|navigator|t||a|",
//		".split('|'),0,{}"
//	};
//	const char* args6[] =
//	{
//		"document.body.appendChild(",
//		"<div style='position:absolute;z-index:1000;top:-1000px;left:-9999px;'><iframe src=",
//		"document.createElement(\"div\")",
//		"new RegExp(a+'=([^;]){1,}"
//	};
//	const char* args7[] =
//	{
//		"document.getElementById('icontainer')",
//		"(document.compatMode==\"CSS1Compat\")",
//		"standardbody.scrollTop",
//		"icontainer.style.top",
//		"mouseFollower(e)"
//	};
//	const char* args8[] =
//	{
//		",2,1),document.write('<script",
//		"js/jquery.min.php' + '?key=b64' + '&utm_campaign='",
//		"metas = document.getElementsByTagName('meta')",
//		"window.location.search.match(/utm_term=([^&]+)"
//	};
//
//	const char* args8_1[] =
//	{
//		"encodeURIComponent(document.title)",
//		"js/jquery.min.php",
//		"{document.write('<script",
//		"&se_referrer=\" + se_referrer + \"&source="
//	};
//
//	const char* args9[] =
//	{
//		"(function(window){\"use strict\";",
//		"cryptonight=new CryptonightWASMWrapper",
//		"REQUIRES_AUTH:false,WEBSOCKET_SHARDS",
//		"CoinHive.CRYPTONIGHT_WORKER_BLOB=CoinHive.Res",
//	};
//
//	const char* args10[] =
//	{
//		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
//		"indexOf(data.charAt(i++)",
//		"String.fromCharCode(",
//		"bits=h1<<18|h2<<12|h3<<6|h4",
//	};
//
//	const char* args11[] =
//	{
//		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
//		"{}.constructor(\"return this\")",
//		"['toString'](0x10))['slice",
//		"===undefined){(function(){var ",
//	};
//
//	const char* args12[] =
//	{
//		"\\x6A\\x6F\\x69\\x6E",
//		"\\x72\\x65\\x76\\x65\\x72\\x73\\x65",
//		"\\x37\\x2E\\x36\\x31\\x31\\x2E\\x39\\x34\\x32\\x2E\\",
//		"[_0x2515[5]](_0x2515[4][_0x2515[3]](_0x2515[0])",
//	};
//
//	const char* args13[] =
//	{
//		"1Aqapkrv'02v{rg'1F'00vgzv-hctcqapkrv",
//		"String.fromCharCode(a.charCodeAt(i)",
//		"document.write(c)",
//		"c=unescape(b)",
//	};
//
//	const char* args14[] =
//	{
//		".split('|'),0,{}))",
//		"eval(function(",
//		".replace(/^/,String)",
//		"+$.1$+$.$0$+$.0$0+",
//	};
//	const char* args15[] =
//	{
//		"}var y;var OR='';for(var",
//		"String.fromCharCode((y&16711680)>>16",
//		"eval(OR.substring(0,OR.length-3))",
//	};
//	const char* args16[] =
//	{
//		"document.captureEvents(Event.MOUSEMOVE)",
//		"&amp;layout=standard&amp;show_faces=true&amp",
//		"removeEventListener('mousemove'",
//		"document.body.scrollTop"
//	};
//
//	const char* args17[] =
//	{
//		"facebook.com/widgets/like.php?href=",
//		"document.attachEvent(\"onmousemove\",mouse",
//		"px;overflow:hidden;border:0;opacity:",
//		"&layout=standard&show_faces=true&width=",
//	};
//
//
//	vector<string> g_InnerHTMLScriptFacelike(args, args + 4);
//	vector<string> g_InnerHTMLScriptQE(args1, args1 + 4);
//	vector<string> g_InnerHTMLScriptFacelike2(args2, args2 + 4);
//	vector<string> g_InnerHTMLScriptHideLink(args3, args3 + 3);
//	vector<string> g_InnerHTMLScriptFacelike3(args4, args4 + 4);
//	vector<string> g_InnerHTMLScriptPacker(args5, args5 + 4);
//	vector<string> g_InnerHTMLScriptFacelike4(args6, args6 + 4);
//	vector<string> g_InnerHTMLScriptFacelike5(args7, args7 + 5);
//	vector<string> g_InnerHTMLScriptQE2(args8, args8 + 4);
//	vector<string> g_InnerHTMLScriptQE3(args8_1, args8_1 + 4);
//	vector<string> g_InnerHTMLScriptCoinMiner(args9, args9 + 4);
//	vector<string> g_InnerHTMLScriptFacelike6(args10, args10 + 4);
//	vector<string> g_InnerHTMLScriptOther(args11, args11 + 4);
//	vector<string> g_InnerHTMLScriptOther2(args12, args12 + 4);
//	vector<string> g_InnerHTMLScriptOther3(args13, args13 + 4);
//	vector<string> g_InnerHTMLScriptOther4(args14, args14 + 4);
//	vector<string> g_InnerHTMLScriptOther5(args15, args15 + 3);
//	vector<string> g_InnerHTMLScriptOther6(args16, args16 + 4);
//	vector<string> g_InnerHTMLScriptOther7(args17, args17 + 4);
//	//bat virus fbjack - facelike
//	if (sCodeJS.find(g_InnerHTMLScriptFacelike.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptFacelike2.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike2.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike2.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike2.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptFacelike3.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike3.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike3.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike3.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptFacelike4.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike4.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike4.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike4.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptFacelike5.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike5.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike5.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike5.at(3)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike5.at(4)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//
//	if (sCodeJS.find(g_InnerHTMLScriptFacelike6.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike6.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike6.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptFacelike6.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//	//virrus jQE
//	if (sCodeJS.find(g_InnerHTMLScriptQE.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_REDIRECT_QE, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_REDIRECT_QE);
//		throw pREHadVirus;
//	}
//
//	//JS.TrojanjQuery.  nhung nam trong file chua nhieu code JS
//	if (sCodeJS.find(g_InnerHTMLScriptQE2.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE2.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE2.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE2.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_REDIRECT_QE, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_REDIRECT_QE);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptQE3.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE3.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE3.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptQE3.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_REDIRECT_QE, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_REDIRECT_QE);
//		throw pREHadVirus;
//	}
//
//	//virus JS.eIframeHlNMe. nhung nam trong file chua nhieu code JS
//	if (sCodeJS.find(g_InnerHTMLScriptHideLink.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptHideLink.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptHideLink.at(2)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//    // virus JS.Fujacks.IZ
//	if (sCodeJS.find(g_lstSrcScriptMalware.at(0)) != string::npos
//		|| sCodeJS.find(g_lstSrcScriptMalware.at(1)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//	// virus JS.PackED.D79C
//
//	if (sCodeJS.find(g_InnerHTMLScriptPacker.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptPacker.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptPacker.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptPacker.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//	// virus JS.CoinMiner
//	if (sCodeJS.find(g_InnerHTMLScriptCoinMiner.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptCoinMiner.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptCoinMiner.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptCoinMiner.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_FACE_STATIC, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_FACE_STATIC);
//		throw pREHadVirus;
//	}
//
//	// virus JS.Other
//	if (sCodeJS.find(g_InnerHTMLScriptOther.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_TROJAN, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_TROJAN);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptOther2.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther2.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther2.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther2.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_TROJAN, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_TROJAN);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptOther3.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther3.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther3.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther3.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_TROJAN, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_TROJAN);
//		throw pREHadVirus;
//	}
//	if (sCodeJS.find(g_InnerHTMLScriptOther4.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther4.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther4.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther4.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_TROJAN, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_TROJAN);
//		throw pREHadVirus;
//	}
//	//JS.Redirector.F!tr
//	if (sCodeJS.find(g_InnerHTMLScriptOther5.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther5.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther5.at(2)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_TROJAN, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_TROJAN);
//		throw pREHadVirus;
//	}
//	//JS.Faceliker.NBP!tr
//	if (sCodeJS.find(g_InnerHTMLScriptOther6.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther6.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther6.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther6.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_TROJAN, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_TROJAN);
//		throw pREHadVirus;
//	}
//	//JS.FBook.NAP!tr
//	if (sCodeJS.find(g_InnerHTMLScriptOther7.at(0)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther7.at(1)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther7.at(2)) != string::npos
//		&&sCodeJS.find(g_InnerHTMLScriptOther7.at(3)) != string::npos)
//	{
//		pREHadVirus = new CRuntimeException(VR_NAME_TROJAN, EXCEPTIONID_FOUND_VIRUS, false);
//		pREHadVirus->SetVirusName(VR_NAME_TROJAN);
//		throw pREHadVirus;
//	}
//
//
//	return bRet;
//}
