////////////////////////////////////////////////////////////////////////////////
//
// Description: define type variable, type of tokenPointer, 
// simple function to release Object, release Array, v...v..
///////////////////////////////////////////////////////////////////////////////

#ifndef DATA_DEF
#define DATA_DEF

#include <windows.h>

#define DEFAULT_ALIAS_NAME ""
//------------------------------------------------------------------------------
#define  MAX_LEN_VIRUSNAME 100

typedef struct _SCANRESULT			// Nhan ket qua quet
{
	WCHAR	wszVirusName[MAX_LEN_VIRUSNAME];	// Ten cua virus cuoi cung tim thay trong file
	BOOL	bHasVirus;						// TRUE -> co nhiem virus
	int		nErrorCode;						// Viec da lam doi voi file	
	int		nCategoryHeur;
	int		nOverSize;
	BOOL	bIsPEFile;
}SCANRESULT;

//------------------------------------------------------------------------------
#define TIME_TO_SCAN_VIRUS		(1000*1000*90)		// được phép quét vr trong 
//------------------------------------------------------------------------------
#define NDC_TODAY_GOODDAY "NDC - Today is a good day"
// Virus Type ------------------------------------------------------------------
#define MAX_SIZE_TO_DOWNLOAD	1024 * 100		// 100 kb
//------------------------------------------------------------------------------

#define POS_X_MOUSE 21294
#define POS_Y_MOUSE 51094

// type of Variable //----------------------------------------------------------
enum SCRIPTVAR_FLAGS 
{
	SCRIPTVAR_UNDEFINED   = 0,
	SCRIPTVAR_FUNCTION    = 1,
	SCRIPTVAR_OBJECT      = 2,
	SCRIPTVAR_ARRAY       = 4,
	SCRIPTVAR_DOUBLE      = 8,  // floating point double
	SCRIPTVAR_INTEGER     = 16, // integer number
	SCRIPTVAR_STRING      = 32, // string
	SCRIPTVAR_NULL        = 64, // it seems null is its own data type

	SCRIPTVAR_NATIVE      = 128, // to specify this is a native function

	SCRIPTVAR_REGEX       = 256,
	SCRIPTVAR_NUMERICMASK = SCRIPTVAR_NULL	|
							SCRIPTVAR_DOUBLE|
							SCRIPTVAR_INTEGER,

	SCRIPTVAR_VARTYPEMASK = SCRIPTVAR_DOUBLE	|
							SCRIPTVAR_INTEGER	|
							SCRIPTVAR_STRING	|
							SCRIPTVAR_REGEX     |
							SCRIPTVAR_FUNCTION	|
							SCRIPTVAR_OBJECT	|
							SCRIPTVAR_ARRAY		|
							SCRIPTVAR_NULL,

};

//------------------------------------------------------------------------------


//  TokenPointer //-------------------------------------------------------------
// Cac loai token pointer trong ngon ngu java script 
enum TK_TYPES 
{
	TK_EOF	= 0,
	TK_ID	= 256,
	TK_INT	= 257,
	TK_FLOAT= 258,
	TK_STR	= 259,

	TK_EQUAL = 260,		// ==
	TK_TYPEEQUAL= 261,	// ===
	TK_NEQUAL= 262,		// !=
	TK_NTYPEEQUAL= 263,	// !==
	TK_LEQUAL= 264,		// <=
	TK_LSHIFT= 265,		// <<
	TK_LSHIFTEQUAL= 266,// <<=	
	TK_GEQUAL= 267,		// >=
	TK_RSHIFT= 268,		// >>
	TK_RSHIFTUNSIGNED= 269,	// >>>
	TK_RSHIFTEQUAL= 270,// >>=	
	TK_PLUSEQUAL= 271,	// +=
	TK_MINUSEQUAL= 272,	// -=
	TK_PLUSPLUS= 273,	// ++
	TK_MINUSMINUS= 274,	// --
	TK_ANDEQUAL= 275,	// &=
	TK_ANDAND= 276,		// &&
	TK_OREQUAL= 277,	// |=
	TK_OROR= 278,		// ||
	TK_XOREQUAL= 279,	// ^=
	

// reserved words --------------------------------
// ds Id cua cac tu khoa trong ngon ngu JavaScript
#define TK_R_LIST_START TK_R_IF
	TK_R_IF,		//280
	TK_R_ELSE,		//281
	TK_R_DO,		//282
	TK_R_WHILE,		//283
	TK_R_FOR,		//284
	TK_R_BREAK,		//285	
	TK_R_CONTINUE,	//286
	TK_R_FUNCTION,	//287	
	TK_R_RETURN,	//288
	TK_R_VAR,		//289
	TK_R_TRUE,		//290
	TK_R_FALSE,		//291
	TK_R_NULL,		//292	
	TK_R_UNDEFINED,	//293
	TK_R_NEW,		//294
	TK_R_DELETE,
	TK_R_TRY,		//295
	TK_R_CATCH,		//296
	TK_R_THROW,		//297
	TK_R_FINALLY,	//298
	
/*	TK_REGEX,*/

	TK_R_TYPEOF,	//299
	TK_REGEX,
	TK_R_IN,
	TK_R_LIST_END /* always the last entry */

};
//------------------------------------------------------------------------------

#define SZ_VAL_COOKIE_DEFAULT	"username=phuongnme; _ga=GA1.2.1807607362.1440833609; __gads=ID=f16f977174b282e6:T=1440833704:S=ALNI_Mb9tGOTbPORjwMELF1nTsHgx1Q-7w; _gat=1"

// type to init Variable--------------------------------------------------------
#define STR_ARGUMENTS_VAR	"arguments"
#define STR_RETURN_VAR		"return"
#define STR_PROTOTYPE_CLASS "prototype"
#define STR_TEMP_NAME		""
#define STR_BLANK_DATA		""
#define STR_UNDEFINE_FUNC	"UndefineFunc"
#define STR_INFO_OBJECT		"!InfoObject"
#define STR_NAME_HTML_ELEMENT "HTMLElement_"
//------------------------------------------------------------------------------


// simple Function SAFE RELEASE MEM---------------------------------------------
#define SAFE_DELETE( p ) if( p ) {delete p; p = NULL;}
#define SAFE_DELETE_ARRAY( p ) if( p ) {delete []p; p = NULL;}
#define SAFE_THROW( pException ) { if(pException) throw pException; }
//------------------------------------------------------------------------------

// debug Memory when running for Programer.....................................
// 
#define SAFE_MEMORY 1

#if SAFE_MEMORY

#include <vector>
class CVar;
class CVarLink;
class  CTokenPointer;

//------------------------------------------------------------------------------
// Them doi tuong CToken moi cap phat vao de quan ly viec xoa sau nay
//------------------------------------------------------------------------------
void AddListToken(CTokenPointer* cToken);

//------------------------------------------------------------------------------
// Xoa tat ca doi tuong CToken da cap phat.
//------------------------------------------------------------------------------
void CleanListToken();

//------------------------------------------------------------------------------
// Them doi tuong cVar moi cap phat vao de quan ly cho viec xoa sau nay.
//------------------------------------------------------------------------------
void AddListCVar(CVar* cVar);

//------------------------------------------------------------------------------
// Them doi tuong cVarLink moi cap phat vao de quan ly cho viec xoa sau nay.
//------------------------------------------------------------------------------
void AddListCVarLink(CVarLink* cVarLink);

//------------------------------------------------------------------------------
// Xoa ta ca cac doi tuong CVar da cap phat
//------------------------------------------------------------------------------
void DeleteAllCVar();

//------------------------------------------------------------------------------
// Xoa tat ca cac doi tuong CVarLink da cap phat
//------------------------------------------------------------------------------
void DeleteAllCVarLink();

//------------------------------------------------------------------------------
// Them 1 du lieu bien vao g_AllocatedVars
//------------------------------------------------------------------------------
void AddTempStack(CVar *pVNew);

//------------------------------------------------------------------------------
// Xoa bo 1 du lieu bien ra khoi g_AllocatedVars
//------------------------------------------------------------------------------
void RemoveTempStack(CVar *pVRemove);

//------------------------------------------------------------------------------
// Them 1 lien ket bien vao g_AllocatedLinks
//------------------------------------------------------------------------------
void AddTempStack(CVarLink *pVLNew);

//------------------------------------------------------------------------------
// Xoa bo 1 lien ket bien ra khoi g_AllocatedLinks
//------------------------------------------------------------------------------
void RemoveTempStack(CVarLink *pVLRemove);

//------------------------------------------------------------------------------
// Xoa bo toan bo ds du lieu bien tam thoi luu trong g_AllocatedVar
// Xoa bo toan bo ds lien ket bien tam thoi luu trong g_AllocatedLinks
// Su dung - CPrograme::resetDatabase()
//------------------------------------------------------------------------------
void CleanTempStack();

//------------------------------------------------------------------------------
// Xoa bo toan bo ds lien ket bien tam thoi luu trong g_AllocatedLinks
//------------------------------------------------------------------------------
void CleanVLTempStack();

//------------------------------------------------------------------------------
// Them 1 bien tro toi stack trong ds g_lstVLToRootStack
//------------------------------------------------------------------------------
void AddListPointToStack(CVarLink *pVLNew);

//------------------------------------------------------------------------------
// Xoa bo 1 bien tro toi stack trong ds g_lstVLToRootStack
//------------------------------------------------------------------------------
void RemoveFromListPointToStack(CVarLink *pVLRemove);

//------------------------------------------------------------------------------
// Xoa bo tat ca ds cac bien tro toi stack trong ds g_lstVLToRootStack
//------------------------------------------------------------------------------
void CleanListPointToStack();



#endif 

void DbgPrintLnA(LPCSTR szFormat, ...);
void DbgPrintA(LPCSTR szFormat, ...);
// ............................................................................

#endif // DATA_DEF




