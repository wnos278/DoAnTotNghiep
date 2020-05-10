////////////////////////////////////////////////////////////////////////////////
// Description: File dinh nghia lop CVar. Lop du lieu dac trung cho 1 kieu du 
// lieu ngon ngu javascript (String, number, array, function, Object, ...). 
// Xay dung cac toan tu + - * / % tren lop du lieu nay
//
// Author: PhuongNMe
// Copyright, Bkav, 2015-2016. All rights reserved
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef VARIABLE_H
#define VARIABLE_H

#include "DATA_DEF.h"
#include "VarLink.h"
#include "RuntimeException.h"
#include "TokenPointer.h"
#include <string>
#include <iostream>
using namespace std ;


class CVar;
class CVarLink;
typedef void (*JSCallback)(CVar *var, void *userdata);

//------------------------------------------------------------------------------
// ClassName: CVar
// Description: save data of Variable, execute the Operator between 2 var.
//------------------------------------------------------------------------------
class CVar
{
public:
		CVarLink *m_pVLFirstChild;	// point to first CVarLink - FirstChild 
		CVarLink *m_pVLLastChild;		// point to last  CVarLink - LastChild

		//----------------------------------------------------------------------
		// constructor with PreValue
		//----------------------------------------------------------------------
		CVar();											// Create undefined
		CVar(const string &sVarData, int nVarFlags); // User defined
		CVar(const string &sData);					// Create a string
		CVar(double sVarData);							// Create a double
		CVar(int nVal);									// Create a int

		
		//----------------------------------------------------------------------
		// remove this && all List Child (-> ~VarLink()) 
		//----------------------------------------------------------------------
		~CVar(void);
		

		//----------------------------------------------------------------------
		// Get && Set - when this Var is SCRIPTVAR_FUNCTION
		//----------------------------------------------------------------------
		CVar*	GetReturnVar(); ///< If this is a function, get the result value (for use by native functions)
		void	SetReturnVar(CVar *pVar); ///< Set the result value. Use this when setting complex return data as it avoids a deepCopy()
		CVar*	GetParameter(const string &sName); ///< If this is a function, get the parameter with the given name (for use by native functions)

		CVarLink *FindChild(const string &sChildName); ///< Tries to find a child with the given name, may return 0
		CVarLink *FindChildOrCreate(const string &sChildName, const string &sAliasName = "", int nVarFlags = SCRIPTVAR_UNDEFINED); ///< Tries to find a child with the given name, or will create it with the given flags
		CVarLink *FindChildOrCreateByPath(const string &sPath); ///< Tries to find a child with the given path (separated by dots)
		CVarLink *AddChild(const string &sChildName, CVar *pVChild = NULL, const string &sAliasName = "", BOOL bRemove = FALSE); ///< add child to list Child
		CVarLink *AddChildNoDup(const string &sChildName, CVar *pVChild = NULL, const string &sAliasName = "", BOOL bRemove = FALSE); ///< add a child overwriting any with the same name
		void	RemoveChild(CVar *pVChild);
		void	RemoveLink(CVarLink* &pVLTmp); ///< Remove a specific link (this is faster than finding via a child)
		void	RemoveAllChildren();
		CVar*	GetArrayIndex(int nIdx); ///< The the value at an array index
		void	SetArrayIndex(int nIdx, CVar *pVValue); ///< Set the value at an array index
		
		int		GetArrayLength(); ///< If this is an array, return the number of items in it (else 0)
								//> get the largest Value of Array Id
		int		GetNumberOfChildren(); ///< Get the number of children
								//> count element of Array 

		//----------------------------------------------------------------------
		// Get data of this Var obj
		//----------------------------------------------------------------------
		int		GetInt();
		bool	GetBool() { return GetInt() != 0; }
		double	GetDouble();
		/*const */string /*&*/GetString();
		string GetRegex();
		string	GetStringDefineFunc();

		//----------------------------------------------------------------------
		// get Data as a parsable javascript string
		//----------------------------------------------------------------------
		string GetParsableString();
		
		//----------------------------------------------------------------------
		// set Data && typeVar this Var obj
		//----------------------------------------------------------------------
		void SetInt(int nNum);
		void SetDouble(double fVal);
		void SetString(const string &sData);
		void SetRegex(const string &sData);
		void SetUndefined();
		void SetArray();
		void SetDataString(string sData);

		//----------------------------------------------------------------------
		// compare value between 2 CVar => Run Operator '=='
		//----------------------------------------------------------------------
		bool Equals(CVar *v);

		//----------------------------------------------------------------------
		// check typeVar of this CVar
		//----------------------------------------------------------------------
		bool IsInt()		{ return (m_nTypeVar&SCRIPTVAR_INTEGER) != 0; }
		bool IsDouble()		{ return (m_nTypeVar&SCRIPTVAR_DOUBLE)	!=0; }
		bool IsString()		{ return (m_nTypeVar&SCRIPTVAR_STRING)	!=0; }
		bool IsNumeric()	{ return (m_nTypeVar&SCRIPTVAR_NUMERICMASK)!=0; }
		bool IsFunction()	{ return (m_nTypeVar&SCRIPTVAR_FUNCTION)!=0; }
		bool IsObject()		{ return (m_nTypeVar&SCRIPTVAR_OBJECT)	!=0; }
		bool IsArray()		{ return (m_nTypeVar&SCRIPTVAR_ARRAY)	!=0; }
		bool IsNative()		{ return (m_nTypeVar&SCRIPTVAR_NATIVE)	!=0; }
		bool IsUndefined()	{ return (m_nTypeVar& SCRIPTVAR_VARTYPEMASK) == SCRIPTVAR_UNDEFINED; }
		bool IsNull()		{ return (m_nTypeVar& SCRIPTVAR_NULL)	!=0; }
		bool IsBasic()		{ return m_pVLFirstChild==0; } ///< Is this *not* an array/object/etc
		bool IsRegex()      { return (m_nTypeVar& SCRIPTVAR_REGEX) !=0; }

		//----------------------------------------------------------------------
		// do a maths op with another script variable
		//----------------------------------------------------------------------
		CVar *MathsOp(CVar *pVOperand, int nOp);

		//----------------------------------------------------------------------
		// copy data from other CVar obj
		//----------------------------------------------------------------------
		void CopyValue(CVar *pVValue); ///< copy the value from the value given
		CVar *DeepCopy(); ///< deep copy this node and return the result
		
		//----------------------------------------------------------------------
		// Lay thong tin prototype cua vung du lieu
		//----------------------------------------------------------------------
		CVar* GetPrototypeOfObject();

		void Trace(string sIndent = "", const string &sName = ""); ///< Dump out the contents of this using trace
		string GetFlagsAsString(); ///< For debugging - just dump a string version of the flags
		
		//----------------------------------------------------------------------
		// Write out all the JS code needed to recreate this script variable to the stream (as JSON)
		//----------------------------------------------------------------------
		void GetJSON(ostringstream &sDestination, const string sLinePrefix = "");
		
		//----------------------------------------------------------------------
		// Set the callback for native functions
		//----------------------------------------------------------------------
		void SetCallback(JSCallback pFuncCallback, void *pUserdata);

		// For memory management/garbage collection
		CVar*	Ref();		///< Add reference to this variable
		void	Unref();	///< Remove a reference, and delete this variable if required
		int		GetRefs();	///< Get the number of references to this script variable

		void	MakeAllChildUnrefsTo(CVar* pVWantUnref);// Go tat ca cac thanh phan con co 
											// lien ket toi pVWanUnResfs

protected:
		int			m_nRefs;	///< The number of references held to this - used for garbage collection
		BOOLEAN		m_bIsDeleting;

		string		m_sData;	///< The contents of this variable if it is a string
		long		m_lData;	///< The contents of this variable if it is an int
		double		m_fData;	///< The contents of this variable if it is a double
		
		int			m_nTypeVar; ///< the flags determine the type of the variable - int/double/string/etc
		
		JSCallback	m_fnCallback; ///< Callback for native functions
		void *		m_fnCallbackUserData; ///< user data passed as second argument to native functions

		BYTE*		m_pBuffDefineFunc;	// szCode dinh nghia ham nay
				
		void		Init();		///< initialisation of data members


		//----------------------------------------------------------------------
		/** Copy the basic data and flags from the variable given, with no
		  * children. Should be used internally only - by copyValue and deepCopy 
		  */
		//----------------------------------------------------------------------
		void		CopySimpleData(CVar *val);

		friend class CProgramJs;

private:
	static BOOLEAN m_bIsResetStack;
};


#endif // VARIABLE_H