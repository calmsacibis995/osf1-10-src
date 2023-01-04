/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
** COPYRIGHT (c) 1988, 1991, 1992 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**++
**  Subsystem:
**	LinkWorks Services
**
**  Version: V1.1
**
**  Abstract:
**	Common header file for all modules.  This include file includes all
**	other generally useful include files.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner, MEMEX Project
**	W. Ward Clark, MEMEX Project
**
**  Creation Date: 7-Jul-88
**
**  Modification History:
**  X0.3    WWC  17-Nov-88  get HIS_DEFINITIONS.H from SYS$LIBRARY
**  X0.11   Les  11-Jun-90  Null out Const
**  X0.18   Les	  9-Apr-91  Include stdlib directly for Alpha building
**--
*/


/*
**  System include files
*/

#ifdef VMS

#include <stdlib.h>

#else

#ifdef MSDOS

#ifdef MSWINDOWS

/*
** We need the following Windows features
*/

#undef NOKERNEL
#undef NOMEMMGR
#undef NOOPENFILE

/*
** Turn off the following Windows features
*/

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOUSER
#define NOMB
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS

#include <windows.h>

#endif /* MSWINDOWS */

#include <stdlib.h>
#include <direct.h>
#include <malloc.h>
#include <sys/types.h>

#else /* !MSDOS */

#include <unistd.h>
#include <sys/types.h>

#endif /* MSDOS */
#endif /* VMS */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <errno.h>


/*
**  Macros to support extended routine declarations
*/

#define _in
#define _out
#define _inout

#define _Set_of(Domain) _Set
#define _List_of(Domain) _List

#ifdef VAXC
#define _PtrTo *
#define _Constant
#define _Void void
#define _EntryPt
#define _Prototypes
#define _DeclareFunction(Function, Arguments) Function Arguments
#elif MSDOS
#define _PtrTo _far *
#define _Constant
#define _Void void
#ifdef MSWINDOWS
#define _EntryPt _far PASCAL
#else /* !MSWINDOWS */
#define _EntryPt _far
#endif /* MSWINDOWS */
#define _Prototypes
#define _DeclareFunction(Function, Arguments) Function Arguments
#elif __osf__
#define _PtrTo *
#define _Constant
#define _Void void
#define _EntryPt
#define _DeclareFunction(Function, Arguments) Function ()
#else /* !VACX && !MSDOS */
#define _PtrTo *
#define _Constant
#define _Void char
#define _EntryPt
#define _DeclareFunction(Function, Arguments) Function ()
#endif /* VAXC */

#if defined(VAXC)
#define _Global globaldef
#define _External globalref
#elif __osf__
#define _Global
#define _External extern
#else /* !VAXC */
#define volatile
#define _Global
#define _External extern
#endif /* VMS */


/*
**  Define some system-dependent Symbols and Types
*/

#define _Char char

#ifdef MSDOS
#define _HeapMalloc _fmalloc
#define _HeapRealloc _frealloc
#define _HeapFree _ffree
#define _Memset _fmemset
#define _Memcpy _fmemcpy
#define _Strlen _fstrlen
#define _Strcmp _fstrcmp
#define _Strspn _fstrspn
#define _Strchr _fstrchr
#else
#define _HeapMalloc malloc
#define _HeapRealloc realloc
#define _HeapFree free
#define _Memset memset
#define _Memcpy memcpy
#define _Strlen strlen
#define _Strcmp strcmp
#define _Strspn strspn
#define _Strchr strchr
#endif

typedef _Char _PtrTo _CharPtr, _PtrTo _PtrTo _CharPtrPtr;
typedef struct tm _OSTm, _PtrTo _OSTmPtr;
typedef FILE _PtrTo _OSFile, _PtrTo _PtrTo _OSFilePtr;


/*
** Sub-system Include Files
*/

#ifdef MSDOS
#include "hisdefs.h"
#include "exceptns.h"
#include "objtypes.h"
#else /* !MSDOS */
#include <lwk_def.h>
#include "his_exceptions.h"
#include "lwk_object_types.h"
#endif
      

/*
**  General Macro Definitions
*/

#define _True ((_Boolean) 1)
#define _False ((_Boolean) 0)

#define _Min(X,Y) (((X) < (Y)) ? (X) : (Y))
#define _Max(X,Y) (((X) > (Y)) ? (X) : (Y))

#define _Success(Status) (((Status) & 1) == 1)

/*
**  Symbolic constants
*/

#define _EndOfString '\0'

#define _DefaultListLength 10
#define _DefaultListExtension 10
#define _EnumListLength _DefaultListLength

#define _BasePropertyPrefix "$"
#define _ApplicationDefinedKeyPrefix "%"
#define _ValidPropertyNameCharacters \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789%$_-!@#"

#define _GenericTypeKeyName "$Type"
#define _SurrogateTypeKeyName "$SurrogateSubType"
#define _LinkTypeKeyName "$RelationshipType"

#define _DefaultRelationshipType " "

#define _UnknownStatusString "Unknown Status"

#define _GenericHelpKeyResource "HlpKeyGeneric"

#define _InferredStepFollowType lwk_c_follow_implicit_go_to
#define _InferredStepOperation "View"

#define _LinkbaseHashSize 509
#define _ImportHashSize 109


/*
**  Primitive datatypes
*/

typedef _Void _PtrTo _AnyPtr, _PtrTo _PtrTo _AnyPtrPtr;
typedef long int _Integer, _PtrTo _IntegerPtr;
typedef long int _Boolean, _PtrTo _BooleanPtr;
typedef float _Float, _PtrTo _FloatPtr;

#define _NullObject 0

/*
**  DECwindows Toolkit datatype synonyms (so that the Toolkit header files do
**  not need to be included in all modules!)
*/

typedef _AnyPtr _Widget;
typedef unsigned long int _Atom;
typedef _AnyPtr _CompoundString;


/*
**  Special Types used for object properties
*/

typedef lwk_domain _Domain, _PtrTo _DomainPtr;
typedef lwk_set_operation _SetFlag, _PtrTo _SetFlagPtr;
typedef lwk_follow_type _FollowType, _PtrTo _FollowTypePtr;
typedef lwk_dxm_menu_action _Menu, _PtrTo _MenuPtr;
typedef lwk_transaction _Transaction, _PtrTo _TransactionPtr;
typedef lwk_query_node _QueryNode, _PtrTo _QueryNodePtr;
typedef lwk_query_operator _QueryOperator, _PtrTo _QueryOperatorPtr;
typedef lwk_query_expression _QueryExpression, _PtrTo _QueryExpressionPtr;
typedef _Integer _HighlightFlags, _PtrTo _HighlightFlagsPtr;


/*
**  Object Deletion Support
*/

#define _Delete(ObjPtr) \
	{ \
	    if (*((_ObjectPtr) (ObjPtr)) != (_Object) _NullObject) { \
		_Expunge(*((_ObjectPtr) (ObjPtr))); \
	    	*((_ObjectPtr) (ObjPtr)) = (_Object) _NullObject; \
	    } \
	}


/*
**  Heap support routines
*/

#ifdef MSDOS_VM
#define _Reference(Ptr) LwkReference((Ptr))
_DeclareFunction(_AnyPtr LwkReference, (_AnyPtr reference));
#else
#define _Reference
#endif

#define _AllocateMem(Size) LwkAllocateMem((Size))
#define _ReallocateMem(Ptr, Size) LwkReallocateMem((Ptr), (Size))
#define _FreeMem(Ptr) LwkFreeMem((Ptr))

#define _ClearMem(Source, Size) _Memset((Source), '\0', (size_t) (Size))
#define _CopyMem(Source, Target, Size) _Memcpy((Target), (Source), \
    (size_t) (Size))

_DeclareFunction(_AnyPtr LwkAllocateMem, (_Integer size));
_DeclareFunction(_AnyPtr LwkReallocateMem, (_AnyPtr pointer, _Integer size));
_DeclareFunction(void LwkFreeMem, (_AnyPtr pointer));


/*
**  Strings and Compound Strings
*/

typedef _Char _PtrTo _String, _PtrTo _PtrTo _StringPtr;

#ifdef MSDOS
typedef _String _CString, _PtrTo _CStringPtr;
typedef _String _DDIFString, _PtrTo _DDIFStringPtr;
#else
typedef _Char _PtrTo _CString, _PtrTo _PtrTo _CStringPtr;
typedef _Char _PtrTo _DDIFString, _PtrTo _PtrTo _DDIFStringPtr;
#endif

#define _CopyString(String) LwkCopyString((String))
#define _ConcatString(String1, String2) \
    LwkConcatString((String1), (String2))
#define _LengthString(String) LwkLengthString((String))
#define _DeleteString(String) LwkDeleteString(String)
#define _CompareString(String1, String2) \
    LwkCompareString((String1), (String2))
#define _IsCaselessEqualString(String1, String2) \
    LwkIsCaselessEqualString((String1), (String2))
#define _HasPrefixString(String, Prefix) \
    LwkHasPrefixString((String), (Prefix))
#define _ContainsString(String1, String2) \
    LwkContainsString((String1), (String2))

#ifdef MSDOS

#define _CopyCString(CString) _CopyString((CString))
#define _LengthCString(CString) _LengthString((CString))
#define _DeleteCString(CString) _DeleteString((CString))
#define _StringToCString(String) _CopyString((String))
#define _CopyDDIFString(DDIFString) _CopyString((DDIFString))
#define _ConcatDDIFString(DDIFString1, DDIFString2) \
    _ConcatString((DDIFString1), (DDIFString2))
#define _LengthDDIFString(DDIFString) (_LengthString((DDIFString)) + 1)
#define _DeleteDDIFString(DDIFString) _DeleteString((DDIFString))
#define _DDIFStringToString(DDIFString) _CopyString((DDIFString))
#define _StringToDDIFString(String) _CopyString((String))
#define _DDIFStringToCString(DDIFString) _CopyString((DDIFString))
#define _CStringToDDIFString(CString) _CopyString((CString))
#define _CompareDDIFString(DDIFString1, DDIFString2) \
    _CompareString((DDIFString1), (DDIFString2))
#define _ContainsDDIFString(DDIFString1, DDIFString2) \
    _ContainsString((DDIFString1), (DDIFString2))
#define _IsValidDDIFString(DDIFString) _True
#define _UserNameToDDIFString _UserNameToString()

#else

#define _CopyCString(CString) LwkDXmCopyCString((CString))
#define _LengthCString(CString) LwkDXmLengthCString((CString))
#define _DeleteCString(CString) LwkDXmDeleteCString(CString)
#define _StringToCString(String) LwkDXmStringToCString((String))
#define _CStringToString(CString) LwkDXmCStringToString((CString))
#define _CopyDDIFString(DDIFString) LwkDXmCopyDDIFString((DDIFString))
#define _ConcatDDIFString(DDIFString1, DDIFString2) \
    LwkDXmConcatDDIFString((DDIFString1), (DDIFString2))
#define _LengthDDIFString(DDIFString) LwkDXmLengthDDIFString((DDIFString))
#define _DeleteDDIFString(DDIFString) LwkDXmDeleteDDIFString(DDIFString)
#define _DDIFStringToString(DDIFString) LwkDXmDDIFStringToString((DDIFString))
#define _StringToDDIFString(String) LwkDXmStringToDDIFString((String))
#define _DDIFStringToCString(DDIFString) \
    LwkDXmDDIFStringToCString((DDIFString))
#define _CStringToDDIFString(CString) LwkDXmCStringToDDIFString((CString))
#define _CompareDDIFString(DDIFString1, DDIFString2) \
    LwkDXmCompareDDIFString((DDIFString1), (DDIFString2))
#define _ContainsDDIFString(DDIFString1, DDIFString2) \
    LwkDXmContainsDDIFString((DDIFString1), (DDIFString2))
#define _IsValidDDIFString(DDIFString) LwkDXmIsValidDDIFString((DDIFString))
#define _UserNameToDDIFString LwkDXmUserNameToDDIFString()

#endif /* MSDOS */

_DeclareFunction(_String LwkCopyString, (_String string));
_DeclareFunction(_String LwkConcatString, (_String string1, _String string2));
_DeclareFunction(_Integer LwkLengthString, (_String string));
_DeclareFunction(void LwkDeleteString, (_StringPtr string));
_DeclareFunction(_Integer LwkCompareString,
    (_String string1, _String string2));
_DeclareFunction(_Boolean LwkIsCaselessEqualString,
    (_String string1, _String string2));
_DeclareFunction(_Boolean LwkContainsString,
    (_String string1, _String string2));
_DeclareFunction(_Boolean LwkHasPrefixString,
    (_String string, _String prefix));

#ifndef MSDOS

_DeclareFunction(_CString LwkDXmCopyCString, (_CString cstring));
_DeclareFunction(_DDIFString LwkDXmCopyDDIFString, (_DDIFString ddifstring));
_DeclareFunction(_DDIFString LwkDXmConcatDDIFString,
    (_DDIFString ddifstring1, _DDIFString ddifstring2));
_DeclareFunction(_Integer LwkDXmLengthCString, (_CString cstring));
_DeclareFunction(_Integer LwkDXmLengthDDIFString, (_DDIFString ddifstring));
_DeclareFunction(void LwkDXmDeleteDDIFString, (_DDIFString *ddifstring));
_DeclareFunction(_String LwkDXmDDIFStringToString, (_DDIFString ddifstring));
_DeclareFunction(_CString LwkDXmStringToCString, (_CString cstring));
_DeclareFunction(_DDIFString LwkDXmStringToDDIFString, (_String string));
_DeclareFunction(_Integer LwkDXmCompareDDIFString,
    (_DDIFString ddifstring1, _DDIFString ddifstring2));
_DeclareFunction(_Boolean LwkDXmContainsDDIFString,
    (_DDIFString ddifstring1, _DDIFString ddifstring2));
_DeclareFunction(_Boolean LwkDXmIsValidDDIFString, (_DDIFString string));
_DeclareFunction(_DDIFString LwkDXmUserNameToDDIFString, (void));
_DeclareFunction(_DDIFString LwkDXmCStringToDDIFString, (_CString cstring));
_DeclareFunction(_String LwkDXmCStringToString, (_CString cstring));
_DeclareFunction(_CString LwkDXmDDIFStringToCString, (_DDIFString ddifstring));

#endif /* MSDOS */


/*
**  Dates
*/

typedef _Char _PtrTo _Date, _PtrTo _PtrTo _DatePtr;

#define _CopyDate(Date) LwkCopyString((Date))
#define _LengthDate(Date) LwkLengthString((Date))
#define _DeleteDate(Date) LwkDeleteString(Date)
#define _CompareDate(Date1, Date2) LwkCompareDate((Date1), (Date2))
#define _IsValidDate(Date) LwkIsValidDate((Date))
#define _DateToString(Date) LwkDateToString((Date))
#define _DateToDDIFString(Date) LwkDXmDateToDDIFString((Date))
#define _DateToTime(Date) LwkDateToTime((Date))
#define _TimeToDate(Time) LwkTimeToDate((Time))
#define _NowToDate LwkNowToDate()

_DeclareFunction(_Integer LwkCompareDate, (_Date date1, _Date date2));
_DeclareFunction(_Boolean LwkIsValidDate, (_Date date));
_DeclareFunction(_String LwkDateToString, (_Date date));
_DeclareFunction(_DDIFString LwkDXmDateToDDIFString, (_Date date));
_DeclareFunction(time_t LwkDateToTime, (_Date date));
_DeclareFunction(_Date LwkTimeToDate, (time_t time));
_DeclareFunction(_Date LwkNowToDate, (void));


/*
**  Status Codes
*/

typedef lwk_status _Status, _PtrTo _StatusPtr;

#if defined(MSDOS) || defined(__osf__)
#define _StatusCode(Code) lwk_s_ ## Code
#else
#define _StatusCode(Code) lwk_s_/**/Code
#endif

#define _StatusToString(Status) LwkStatusToString((Status))
#define _StatusToResource(Status) LwkStatusToResource((Status))
#define _StatusToCString(Status) LwkDXmStatusToCString((Status))
#define _StatusToHelpKeyResource(Status) LwkStatusToHelpKeyResource((Status))

#ifdef MSDOS
#define _StatusToDDIFString(Status) _StatusToString((Status))
#else
#define _StatusToDDIFString(Status) LwkDXmStatusToDDIFString((Status))
#endif

_DeclareFunction(_String LwkStatusToString, (_Status status));
_DeclareFunction(_String LwkStatusToResource, (_Status status));
_DeclareFunction(_CString LwkDXmStatusToCString, (_Status status));
_DeclareFunction(_String LwkStatusToHelpKeyResource, (_Status status));
_DeclareFunction(_DDIFString LwkDXmStatusToDDIFString, (_Status status));


/*
**  Varying String Support
*/

typedef struct __VaryingString {
	    _Integer length;
	    _Char string[1];
	} _VaryingStringInstance,
	    _PtrTo _VaryingString, _PtrTo _PtrTo _VaryingStringPtr;

#define _VaryingStringMaxSize LONG_MAX

#define _VLength_of(VString) ((_VaryingString) _Reference((VString)))->length
#define _VString_of(VString) ((_VaryingString) _Reference((VString)))->string

#define _VHeaderSize (sizeof(_VaryingStringInstance) - sizeof(_Char))

#define _VSize_of(VString) (_VHeaderSize + _VLength_of((VString)))

#define _AllocateVaryingString(VString, MaxLength) \
    { \
	(VString) = (_VaryingString) _AllocateMem(_VHeaderSize + (MaxLength)); \
	_VLength_of((VString)) = 0; \
    }

#define _ReallocateVaryingString(VString, MaxLength) \
    (VString) = (_VaryingString) _ReallocateMem((VString), \
	_VHeaderSize + (MaxLength));

#define _DeleteVaryingString(VString) \
    LwkDeleteVaryingString(VString)

#define _CreateVaryingString(VString, String, Length) \
    { \
	_AllocateVaryingString((VString), (Length)); \
	_VLength_of((VString)) = (Length); \
	_CopyMem((String), _VString_of(VString), (Length)); \
    }

#define _VaryingStringToString(VString, String) \
    { \
	(String) = (_String) _AllocateMem(_VLength_of(VString) + 1); \
	_CopyMem(_VString_of(VString), (String), _VLength_of(VString)); \
	(String)[_VLength_of(VString)] = _EndOfString; \
    }

#define _VaryingStringToDDIFString(VString, DDIFString) \
    { \
	(DDIFString) = (_DDIFString) _AllocateMem(_VLength_of(VString)); \
	_CopyMem(_VString_of(VString), (DDIFString), _VLength_of(VString)); \
    }

_DeclareFunction(void LwkDeleteVaryingString, (_VaryingStringPtr vstring));


/*
**  Dynamic Varying String Support
*/

typedef struct __DynamicVString {
	    _Integer allocation;
	    _VaryingString vstring;
	} _DynamicVStringInstance,
	    _PtrTo _DynamicVString, _PtrTo _PtrTo _DynamicVStringPtr;

#define _DVAllocation_of(DVString) \
    ((_DynamicVString) _Reference((DVString)))->allocation
#define _DVString_of(DVString) \
    ((_DynamicVString) _Reference((DVString)))->vstring

#define _CreateDVString(DVString) \
    { \
	(DVString) = (_DynamicVString) \
	    _AllocateMem(sizeof(_DynamicVStringInstance)); \
	_DVAllocation_of(DVString) = 0; \
	_DVString_of(DVString) = (_VaryingString) _NullObject;; \
    }

#define _AllocateDVString(DVString, Allocation) \
    { \
	_AllocateVaryingString(_DVString_of(DVString), (Allocation)); \
	_DVAllocation_of(DVString) = (Allocation); \
    }

#define _ReallocateDVString(DVString, Allocation) \
    { \
	_ReallocateVaryingString(_DVString_of(DVString), (Allocation)); \
	_DVAllocation_of(DVString) = (Allocation); \
    }

#define _DeleteDVString(DVString) \
    LwkDeleteDVString(DVString)

_DeclareFunction(void LwkDeleteDVString, (_DynamicVStringPtr dvstring));


/*
** DDIS encoding/decoding support
*/

#if defined(__osf__)
#include <cdatyp.h>
typedef DDISstreamhandle _DDIShandle, _PtrTo _DDIShandlePtr;
#else
typedef _AnyPtr _DDIShandle, _PtrTo _DDIShandlePtr;
#endif /* __osf__ */

#define _DeleteEncoding(Encoding) _DeleteVaryingString((Encoding))


/*
**  Callback routines
*/

typedef unsigned long int _Closure, _PtrTo _ClosurePtr;
typedef unsigned long int _Termination, _PtrTo _TerminationPtr;
typedef _Termination (_EntryPt *_Callback)();
typedef _Callback _PtrTo _CallbackPtr;


/*
**  List elements
*/

typedef union __AnyValue {
	    _Integer integer;
	    _Boolean boolean;
	    _Float float_pt;
	    _String string;
	    _DDIFString ddifstring;
	    _Date date;
	    _Object object;
	} _AnyValue, _PtrTo _AnyValuePtr;


/*
**  Hash Table Support
*/

typedef struct __HashTableEntry {
	    struct __HashTableEntry _PtrTo next;
	    _Integer key;
	    _AnyPtr item;
	} _HashTableEntryInstance,
	    _PtrTo _HashTableEntry, _PtrTo _PtrTo _HashTableEntryPtr;

#define _HashKey_of(Entry) ((_HashTableEntry) _Reference((Entry)))->key
#define _HashItem_of(Entry) ((_HashTableEntry) _Reference((Entry)))->item
#define _HashNext_of(Entry) ((_HashTableEntry) _Reference((Entry)))->next

#define _HashInitialize(Table, Size) LwkHashInitialize((Table), (Size))
#define _HashFree(Table, Size) LwkHashFree((Table), (Size))
#define _HashInsert(Table, Size, Key, Item) \
    LwkHashInsert((Table), (Size), (Key), (Item))
#define _HashSearch(Table, Size, Key, Item) \
    LwkHashSearch((Table), (Size), (Key), (Item))
#define _HashDelete(Table, Size, Key, Item) \
    LwkHashDelete((Table), (Size), (Key), (Item))

_DeclareFunction(void LwkHashInitialize,
    (_HashTableEntryPtr table, _Integer size));
_DeclareFunction(_Boolean LwkHashInsert,
    (_HashTableEntryPtr table, _Integer size, _Integer key, _AnyPtr item));
_DeclareFunction(_Boolean LwkHashSearch,
    (_HashTableEntryPtr table, _Integer size, _Integer key,
	_AnyPtrPtr item));
_DeclareFunction(_Boolean LwkHashDelete,
    (_HashTableEntryPtr table, _Integer size, _Integer key, _AnyPtr item));


/*
**  Property Name Tables
*/

#define _SearchTable(Table, Property, IncludePrivate) \
    LwkSearchTable((Table), (Property), (IncludePrivate))
#define _ListTable(Table, List, IncludePrivate) \
    LwkListTable((Table), (List), (IncludePrivate))

_DeclareFunction(int LwkSearchTable,
    (_PropertyNameTable table, _String property, _Boolean include_private));
_DeclareFunction(void LwkListTable,
    (_PropertyNameTable table, _Object list, _Boolean include_private));


/*
**  Iteration/Query Support
*/

typedef struct __QueryContext {
	    _Object  objects_processed;
	    _Object  composites_processed;
	    _Callback routine;
	    _Closure  closure;
	    _Domain   domain;
	    _QueryExpression expression;
	} _QueryContext, _PtrTo _QueryContextPtr;

#define _SelectProperties(Object, Expression) \
    LwkQuerySelectProperties((Object), (Expression))

_DeclareFunction(_Boolean LwkQuerySelectProperties,
    (_Object object, _QueryExpression expression));


/*
** Copy Aggregate Support
*/

typedef struct __Aggregate {
	    _Object aggregate;
	    _HashTableEntry _PtrTo hash_table;
	} _AggregateInstance, _PtrTo _Aggregate, _PtrTo _PtrTo _AggregatePtr;


/*
**  Domains
*/

#define _DomainToType(Domain) LwkDomainToType((Domain))
#define _TypeToDomain(Type) LwkTypeToDomain((Type))
#define _IsDomain(Domain1, Domain2) LwkIsDomain((Domain1), (Domain2))

_DeclareFunction(_Type LwkDomainToType, (_Domain domain));
_DeclareFunction(_Domain LwkTypeToDomain, (_Type type));
_DeclareFunction(_Boolean LwkIsDomain, (_Domain domain1, _Domain domain2));


/*
**  Object validity-checking support routines
*/

#define _IsValidObject(Object) \
    LwkIsValidObject((_Object) _Reference((Object)))
#define _IsValidType(Type) LwkIsValidType((Type))

_DeclareFunction(_Boolean LwkIsValidObject, (_Object object));
_DeclareFunction(_Boolean LwkIsValidType, (_Type type));


/*
**  Property setting support routines
*/

#define _IsValidPropertyName(Property) LwkIsValidPropertyName((Property))

#define _SetSingleValuedProperty(PPtr, PDom, VDom, VPtr, Flgs, VIsLst) \
    LwkSetSingleValProp((PPtr), (PDom), (VDom), (VPtr), (Flgs), (VIsLst))

#define _SetMultiValuedProperty(PPtr, PDom, VDom, VPtr, Flgs, VIsList, PIsSet) \
    LwkSetMultiValProp((PPtr), (PDom), (VDom), (VPtr), (Flgs), (VIsList), \
    (PIsSet))

#define _CompareValue(Value1Ptr, Value2Ptr, Domain) \
    LwkCompareValue((Value1Ptr), (Value2Ptr), (Domain))
#define _CopyValue(SrcPtr, DstPtr, Domain) \
    LwkCopyValue((SrcPtr), (DstPtr), (Domain))
#define _MoveValue(SrcPtr, DstPtr, Domain) \
    LwkMoveValue((SrcPtr), (DstPtr), (Domain))
#define _ListValue(ValuePtr, ListPtr, Domain) \
    LwkListValue((ValuePtr), (ListPtr), (Domain))
#define _ClearValue(ValuePtr, Domain) \
    LwkClearValue((ValuePtr), (Domain))
#define _DeleteValue(ValuePtr, Domain) \
    LwkDeleteValue((ValuePtr), (Domain))

_DeclareFunction(_Boolean LwkIsValidPropertyName, (_String property));
_DeclareFunction(void LwkSetSingleValProp,
    (_AnyPtr property, _Domain property_domain, _Domain value_domain,
	_AnyPtr value, _SetFlag flag, _Boolean value_is_list));
_DeclareFunction(void LwkSetMultiValProp,
    (_ObjectPtr list, _Domain property_domain, _Domain value_domain,
	_AnyPtr value, _SetFlag flag, _Boolean value_is_list,
	_Boolean property_is_set));

_DeclareFunction(_Integer LwkCompareValue,
    (_AnyPtr value1, _AnyPtr value2, _Domain domain));
_DeclareFunction(void LwkCopyValue,
    (_AnyPtr source, _AnyPtr target, _Domain domain));
_DeclareFunction(void LwkMoveValue,
    (_AnyPtr source, _AnyPtr target, _Domain domain));
_DeclareFunction(void LwkListValue,
    (_AnyPtr value, _ObjectPtr list, _Domain domain));
_DeclareFunction(void LwkClearValue, (_AnyPtr value, _Domain domain));
_DeclareFunction(void LwkDeleteValue, (_AnyPtr value, _Domain domain));


/*
**  Persistent Object State Support
*/

typedef unsigned short int _StateFlags, _PtrTo _StateFlagsPtr;

typedef enum __StateOperation {
	_StateGet,
	_StateSet,
	_StateClear,
	_StateComplement
    } _StateOperation;

typedef enum __PersistentState {
	_StateModified = 0,
	_StateDeletePending,
	_StateHaveAllSurrogates,
	_StateHaveAllLinks,
	_StateHaveAllSteps,
	_StateHaveInterLinks,
	_StateHasInComming,
	_StateHasOutGoing,
	_StateSourceIsId,
	_StateTargetIsId,
	_StateOriginIsId,
	_StateDestinationIsId,
	_StateNextStepIsId,
	_StatePreviousStepIsId,
	_StateInterLinksAreIds
    } _PersistentState;

#define _StateToFlag(State) ((_StateFlags) (1 << ((int) (State))))

#define _InitialStateFlags (_StateToFlag(_StateModified) | \
			    _StateToFlag(_StateHaveAllSurrogates) | \
			    _StateToFlag(_StateHaveAllLinks) | \
			    _StateToFlag(_StateHaveAllSteps) | \
			    _StateToFlag(_StateHaveInterLinks))


/*
**  Special operation to retrieve a DDIF property value into a CS value
*/

#define _GetCSValue(Object, Property, CString) \
    LwkGetCSValue((Object), (Property), (CString))

_DeclareFunction(void LwkGetCSValue,
    (_Object object, _String property, _CStringPtr cstring));


/*
**  Support for the Concert Multi-Thread Architecture (CMA)
*/

#define _BeginCriticalSection
#define _EndCriticalSection
