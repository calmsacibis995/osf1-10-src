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
** COPYRIGHT (c) 1991 BY
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
**	LinkWorks Manager 
**
**  Version: V1.0
**
**  Abstract:
**	Common header file for all modules
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	André Pavanello
**
**  Creation Date: 9-Nov-89
**
**  Modification History:
**  X0.7-1  Pat	17-Nov-89   remove _Delete macro
**  X0.11-1 Les 11-Jun-90   declare _Constant as nothing to avoid non-shareable
**			    psects in shareble image
**--
*/


#ifdef VAXC
#define _Constant 
#define _Void void
#define _DeclareFunction(Function, Arguments) Function Arguments
#elif __osf__
#define _Constant
#define _Void void
#define _DeclareFunction(Function, Arguments) Function ()
#else
#define _Constant
#define _Void char
#define _DeclareFunction(Function, Arguments) Function ()
#endif

#ifdef VAXC
#define _Global globaldef
#define _External globalref
#elif __osf__
#define _Global
#define _External extern
#else
#define volatile
#define _Global
#define _External extern
#endif

/*
**  System Include Files
*/

#ifdef VMS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#else
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#endif

/*
** Macro to support extended routine declarations
*/

#define _in
#define _out
#define _inout

#define _Set_of(Domain) _Set
#define _List_of(Domain) _List

#define _PtrTo *

/*
** Sub-system Include Files
*/

#include <lwk_def.h>
#include "hs_exceptions.h"
#include "hs_object_types.h"

/*
**  Resolve Incompatible System-defined Symbols
*/

#if VMS
#else
typedef struct tm tm_t;
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

/*
**  Primitive datatypes
*/

typedef _Void *_AnyPtr;
typedef long int _Integer;
typedef long int _Boolean;
typedef float _Float;

#define _NullObject 0

/*
**  DECwindows Toolkit datatype synonyms (so that the Toolkit header files do
**  not need to be included in all modules!)
*/

typedef _AnyPtr		_Widget;
typedef unsigned long int _Atom;
typedef _Integer	_Position;
typedef _Integer	_Cardinal;

/*
**  Special Types used for object properties
*/

typedef enum _hs_domain {
    hs_c_domain_min = 0,
    hs_c_domain_unknown = 0,
    hs_c_domain_integer,
    hs_c_domain_string,
    hs_c_domain_comp_string,
    hs_c_domain_boolean,
    hs_c_domain_routine,
    hs_c_domain_any_ptr,
    hs_c_domain_object,
    hs_c_domain_list,
    hs_c_domain_window,
    hs_c_domain_environment_wnd,
    hs_c_domain_linkbase_wnd,
    hs_c_domain_hsobject,
    hs_c_domain_environment_ctxt,
    hs_c_domain_lwk_object,
    hs_c_domain_max
    } hs_domain;

typedef hs_domain _Domain;

typedef enum _hs_set_operation {
    hs_c_set_property = 1,
    hs_c_clear_property,
    hs_c_delete_property,
    hs_c_add_property,
    hs_c_remove_property
    } hs_set_operation;

typedef hs_set_operation _SetFlag;

typedef enum _hs_save_operation {
    hs_c_save_object = 1
    } hs_save_operation;

typedef hs_save_operation _SaveFlag;

/*
**  Object Deletion Support
*/

#define _Delete(ObjectPtr) \
	{ \
	    if (*((_Object *) (ObjectPtr)) != (_Object) _NullObject) { \
		_DeleteObj(*((_Object *) (ObjectPtr))); \
		*((_Object *) (ObjectPtr)) = (_Object) _NullObject; \
	    } \
	}

/*
**  Strings
*/

typedef char *_String;
typedef char *_CString;
typedef	char *_DDIFString;

#define _CopyString(String) EnvCopyString((String))
#define _CopyCString(CString) EnvDWCopyCString((CString))
#define _CopyDDIFString(DDIFString) EnvDWCopyDDIFString((DDIFString))

#define _ConcatString(String1, String2) \
    EnvConcatString((String1), (String2))
#define _ConcatCString(CString1, CString2) \
    EnvDWConcatCString((CString1), (CString2))

#define _LengthString(String) EnvLengthString((String))
#define _LengthCString(CString) EnvDWLengthCString((CString))
#define _LengthDDIFString(DDIFString) EnvDWLengthDDIFString((DDIFString))
#define _IsEmptyCString(CString) EnvDWIsEmptyCString((CString))

#define _DeleteString(String) EnvDeleteString((String))
#define _DeleteCString(CString) EnvDWDeleteCString((CString))
#define _DeleteDDIFString(DDIFString) EnvDWDeleteDDIFString((DDIFString))

#define _CStringToString(CString) EnvDWCStringToString((CString))
#define _StringToCString(String) EnvDWStringToCString((String))

#define _StringToDDIFString(String) EnvDWStringToDDIFString((String))
#define _CStringToDDIFString(CString) EnvDWCStringToDDIFString((CString))
#define _DDIFStringToCString(DDIFString) EnvDWDDIFStringToCString((DDIFString))
#define _DDIFStringToString(DDIFString) EnvDWDDIFStringToString((DDIFString))

#define _CompareString(String1, String2) \
    EnvCompareString((String1), (String2))
#define _CompareCString(CString1, CString2) \
    EnvDWCompareCString((CString1), (CString2))

#define _CompareStringN(String1, String2, Length) \
    EnvCompareStringN((String1), (String2), (Length))

#define _IsCaselessEqualString(String1, String2) \
    EnvIsCaselessEqualString((String1), (String2))

#define _ContainsString(String1, String2) \
    EnvContainsString((String1), (String2))
#define _ContainsCString(CString1, CString2) \
    EnvDWContainsCString((CString1), (CString2))

#define _IsValidCString(CString) EnvDWIsValidCString((CString))

_DeclareFunction(_String EnvCopyString, (_String string));
_DeclareFunction(_CString EnvDWCopyCString, (_CString cstring));
_DeclareFunction(_DDIFString EnvDWCopyDDIFString, (_DDIFString ddif_string));
_DeclareFunction(_String EnvConcatString, (_String string1, _String string2));
_DeclareFunction(_CString EnvDWConcatCString,
    (_CString cstring1, _CString cstring2));
_DeclareFunction(_Integer EnvLengthString, (_String string));
_DeclareFunction(_Integer EnvDWLengthCString, (_CString cstring));
_DeclareFunction(_Integer EnvDWLengthDDIFString, (_DDIFString ddif_string));
_DeclareFunction(_Integer EnvDWIsEmptyCString, (_CString cstring));
_DeclareFunction(_Void EnvDeleteString, (_String *string));
_DeclareFunction(_Void EnvDWDeleteCString, (_CString *cstring));
_DeclareFunction(_Void EnvDWDeleteDDIFString, (_DDIFString *ddif_string));
_DeclareFunction(_String EnvDWCStringToString, (_CString cstring));
_DeclareFunction(_CString EnvDWStringToCString, (_String string));
_DeclareFunction(_DDIFString EnvDWCStringToDDIFString, (_CString cstring));
_DeclareFunction(_DDIFString EnvDWStringToDDIFString, (_String string));
_DeclareFunction(_CString EnvDWDDIFStringToCString, (_DDIFString ddifstring));
_DeclareFunction(_String EnvDWDDIFStringToString, (_DDIFString ddifstring));
_DeclareFunction(_Integer EnvCompareString,
    (_String string1, _String string2));
_DeclareFunction(_Integer EnvDWCompareCString,
    (_CString cstring1, _CString cstring2));
_DeclareFunction(_Integer EnvCompareStringN,
    (_String string1, _String string2, _Integer length));
_DeclareFunction(_Boolean EnvIsCaselessEqualString,
    (_String string1, _String string2));
_DeclareFunction(_Boolean EnvContainsString,
    (_String string1, _String string2));
_DeclareFunction(_Boolean EnvDWContainsCString,
    (_CString cstring1, _CString cstring2));
_DeclareFunction(_Boolean EnvDWIsValidCString, (_CString string));

/*
**  Date
*/

typedef char *_Date;

#define _DeleteDate(Date) EnvDeleteString((Date))
#define _TimeToDate(Time) EnvTimeToDate((Time))
#define _NowToDate EnvNowToDate()

_DeclareFunction(_Date EnvTimeToDate, (time_t time));
_DeclareFunction(_Date EnvNowToDate, (void));

/*
**  User name
*/

#define _UserNameToCString EnvDWUserNameToCString()

_DeclareFunction(_CString EnvDWUserNameToCString, (void));

/*
**  Status Codes
*/

typedef hs_status _Status;

#if defined(__osf__)
#define _StatusCode(Code) hs_s_ ## Code
#else
#define _StatusCode(Code) hs_s_/**/Code
#endif /* __osf__ */

#define _UnknownStatusString "Unknown Status"

#define _StatusToString(Status) EnvStatusToString((Status))
#define _StatusToResource(Status) EnvStatusToResource((Status))
#define _StatusToCString(Status) EnvDWStatusToCString((Status))
#define _SaveHisStatus(Status)	EnvMsgSaveHisStatus((Status))

_DeclareFunction(_String EnvStatusToString, (_Status status));
_DeclareFunction(_String EnvStatusToResource, (_Status status));
_DeclareFunction(_CString EnvDWStatusToCString, (_Status status));
_DeclareFunction(_Void EnvMsgSaveHisStatus, (lwk_status his_stat));

/*
**  Error Message Severity Codes
*/

typedef enum _hs_message_severity {
    hs_c_fatal_message = 1,
    hs_c_info_message
    } hs_message_severity;

typedef hs_message_severity _MsgSeverityFlag;

/*
**  Callback routines
*/

typedef unsigned long int _Closure;
typedef unsigned long int _Termination;
typedef _Termination (*_Callback)();

/*
**  Currency type
*/

typedef lwk_environment_change _CurrencyFlag;

/*
**  List elements
*/

typedef union __AnyValue {
            _Integer integer;
            _Boolean boolean;
            _String string;
            _CString cstring;
            _AnyPtr /* _Object */ object;
        } _AnyValue, *_AnyValuePtr;

/*
**  Property Name Tables
*/

#define _SearchTable(Table, Property, IncludePrivate) \
    EnvSearchTable((Table), (Property), (IncludePrivate))
#define _ListTable(Table, List, IncludePrivate) \
    EnvListTable((Table), (List), (IncludePrivate))

_DeclareFunction(_Integer EnvSearchTable,
    (_PropertyNameTable table, _String property, _Boolean include_private));
_DeclareFunction(_Void EnvListTable,
    (_PropertyNameTable table, _Object list, _Boolean include_private));

/*
**  Domains
*/

#define _DomainToType(Domain) EnvDomainToType((Domain))
#define _TypeToDomain(Type) EnvTypeToDomain((Type))
#define _IsDomain(Domain1, Domain2) EnvIsDomain((Domain1), (Domain2))

_DeclareFunction(_Type EnvDomainToType, (_Domain domain));
_DeclareFunction(_Domain EnvTypeToDomain, (_Type type));
_DeclareFunction(_Boolean EnvIsDomain, (_Domain domain1, _Domain domain2));

/*
**  Object validity-checking support routines
*/

#define _HsPrivateProperty	"HS$Private"
#define _HsActivePathsProperty  "HS$ActivePaths"

#define _IsValidObject(Object) EnvIsValidObject((Object))
#define _IsValidType(Type) EnvIsValidType((Type))
#define _IsPrivateObject(Object) EnvIsPrivateObject((Object))
#define _SetPrivateProperty(Object, Linkbase) \
    EnvSetPrivateProperty((Object), (Linkbase))

_DeclareFunction(_Boolean EnvIsValidObject, (_Object object));
_DeclareFunction(_Boolean EnvIsValidType, (_Type type));
_DeclareFunction(_Boolean EnvIsPrivateObject, (lwk_persistent object));
_DeclareFunction(_Void EnvSetPrivateProperty,
    (lwk_persistent object, lwk_linkbase linkbase));

/*
**  Heap support routines
*/

#define _AllocateMem(Size) EnvAllocateMem((Size))
#define _ReallocateMem(Object, Size) EnvReallocateMem((Object), (Size))
#define _ClearMem(Source, Size) memset((Source), '\0', (Size))
#define _CopyMem(Source, Target, Size) memcpy((Target), (Source), (Size))
#define _FreeMem(Object) EnvFreeMem((Object))

_DeclareFunction(_AnyPtr EnvAllocateMem, (const int size));
_DeclareFunction(_AnyPtr EnvReallocateMem, (_AnyPtr pointer, const int size));
_DeclareFunction(_Void EnvFreeMem, (_AnyPtr pointer));

#define _Reference

/*
**  Property setting support routines
*/

#define _SetSingleValuedProperty(PPtr, PDom, VDom, VPtr, Flgs, VIsLst) \
    EnvSetSingleValProp((PPtr), (PDom), (VDom), (VPtr), (Flgs), (VIsLst))

#define _SetMultiValuedProperty(PPtr, PDom, VDom, VPtr, Flgs, VIsList, PIsSet) \
    EnvSetMultiValProp((PPtr), (PDom), (VDom), (VPtr), (Flgs), (VIsList), \
        (PIsSet))

#define _CompareValue(Value1Ptr, Value2Ptr, Domain) \
    EnvCompareValue((Value1Ptr), (Value2Ptr), (Domain))
#define _CopyValue(SrcPtr, DstPtr, Domain) \
    EnvCopyValue((SrcPtr), (DstPtr), (Domain))
#define _MoveValue(SrcPtr, DstPtr, Domain) \
    EnvMoveValue((SrcPtr), (DstPtr), (Domain))
#define _ListValue(ValuePtr, ListPtr, Domain) \
    EnvListValue((ValuePtr), (ListPtr), (Domain))
#define _ClearValue(ValuePtr, Domain) \
    EnvClearValue((ValuePtr), (Domain))
#define _DeleteValue(ValuePtr, Domain) \
    EnvDeleteValue((ValuePtr), (Domain))

_DeclareFunction(_Void EnvSetSingleValProp,
    (_AnyValuePtr property, _Domain property_domain, _Domain value_domain,
        _AnyValuePtr value, _SetFlag flag, _Boolean value_is_list));
_DeclareFunction(_Void EnvSetMultiValProp,
    (_Object *list, _Domain property_domain, _Domain value_domain,
        _AnyValuePtr value, _SetFlag flag, _Boolean value_is_list,
        _Boolean property_is_set));

_DeclareFunction(_Integer EnvCompareValue,
    (_AnyValuePtr value1, _AnyValuePtr value2, _Domain domain));
_DeclareFunction(_Void EnvCopyValue,
    (_AnyValuePtr source, _AnyValuePtr target, _Domain domain));
_DeclareFunction(_Void EnvMoveValue,
    (_AnyValuePtr source, _AnyValuePtr target, _Domain domain));
_DeclareFunction(_Void EnvListValue,
    (_AnyValuePtr value, _Object *list, _Domain domain));
_DeclareFunction(_Void EnvClearValue, (_AnyValuePtr value, _Domain domain));
_DeclareFunction(_Void EnvDeleteValue, (_AnyValuePtr value, _Domain domain));

/*
**  Currency structure support
*/

typedef struct __CurrencyContext {
	    lwk_composite_linknet	cnet;
	    lwk_linknet		network;
	    lwk_path		trail;
	} _CurrencyContextType, *_CurrencyContext;

typedef struct __IterateData {
	    _String	    obj_name;
	    lwk_object	    object;
	} _IterateDataType, *_IterateData;
	    
/*
**  Objects State Support
*/

typedef unsigned short int _StateFlags;

typedef enum __StateOperation {
        _StateGet,
        _StateSet,
        _StateClear,
        _StateComplement
    } _StateOperation;

typedef enum __WindowState {
	_StateNew = 0,
        _StateMapped,
	_StateRealized,
	_StateIconized,
	_StateHighlightingOn,
	_StateReuse,
	_StateMessageSet,
	_StateCustomNotSaved,
	_StateActiveNetworks,
	_StateActivePaths
    } _WindowState;

typedef enum __HsObjectState {
        _StateModified = 0,
        _StateSaved
    } _HsObjectState;

typedef enum __UpdateOperation {
	_NewObject = 0,
	_ModifiedObject,
	_ModifiedProp,
	_DeletedObject
    } _UpdateOperation;

typedef enum __GetSelectOperation {
        _SingleSelection = 0,
        _FirstSelection,
        _MultipleSelection
    } _GetSelectOperation;

typedef enum __SaveOperation {
        _StoreObject = 0,
        _DeleteObject
    } _SaveOperation;

#define _StateToFlag(State) ((_StateFlags) (1 << ((int) (State))))

#define _InitialWindowStateFlags (_StateToFlag(_StateNew))

#define _InitialHsObjectStateFlags ((_StateFlags) 0)

/*
**  Attribute name definitions
*/

typedef enum __Attribute {
	_FirstAttribute = 0,
        _ConnectionTypeAttr = 0,
        _HighlightingAttr,
	_OperationAttr,
	_RetainSourceAttr,
	_EnvXPositionAttr,
	_EnvYPositionAttr,
	_EnvWidthAttr,
	_EnvHeightAttr,
	_EnvWindowSplit,
	_EnvIconizedAttr,
	_LastAttribute
    } _Attribute;

#define _EnvironmentAttributeCount 5

/*
**  Attribute support routines
*/

#define _AttributeToProperty(Attribute) \
    EnvAttrAttributeToProperty((Attribute))
#define _AttributeToDomain(Attribute) \
    EnvAttrAttributeToDomain((Attribute))
#define _AttributeDefaultValue(Attribute) \
    EnvAttrAttributeDefaultValue((Attribute))
#define _IsEnvironmentAttribute(Attribute) \
    EnvAttrIsEnvironmentAttribute((Attribute))

_DeclareFunction(_String EnvAttrAttributeToProperty, (_Attribute attribute));
_DeclareFunction(_Integer EnvAttrAttributeToDomain, (_Attribute attribute));
_DeclareFunction(_AnyPtr EnvAttrAttributeDefaultValue, (_Attribute attribute));
_DeclareFunction(_Boolean EnvAttrIsEnvironmentAttribute,
    (_Attribute attribute));
_DeclareFunction(_AnyPtr EnvAttrSetDefAttribute,
    (lwk_persistent attr_cnet, _Integer attribute));

/*
**  Map table definition and support macros
*/

typedef struct __MapTable {
	    _AnyPtr	key;
	    _Integer	count;
	    _Integer	size;
	    _AnyPtr	*data;
	} _MapTableInstance, *_MapTable;

#define _MapTableCreate(size) EnvMapTableCreate((size))
#define _MapTableAddEntry(table, key, user_data) \
    EnvMapTableAddEntry((table), (key), (user_data))
#define _MapTableGetEntry(table, key, data, count) \
    EnvMapTableGetEntry((table), (key), (data), (count))
#define _MapTableRemoveEntry(table, key) \
    EnvMapTableRemoveEntry((table), (key))
#define _MapTableRemoveEntryData(table, key, data) \
    EnvMapTableRemoveEntryData((table), (key), (data))

_DeclareFunction(_MapTable EnvMapTableCreate, (_Integer size));
_DeclareFunction(_Void EnvMapTableAddEntry,
    (_MapTable *map_table, _AnyPtr key, _AnyPtr user_data));
_DeclareFunction(_Boolean EnvMapTableGetEntry,
    (_MapTable table, _AnyPtr key, _AnyPtr *data, _Integer *count));
_DeclareFunction(_Boolean EnvMapTableRemoveEntry,
    (_MapTable table, _AnyPtr key));
_DeclareFunction(_Boolean EnvMapTableRemoveEntryData,
    (_MapTable table, _AnyPtr key, _AnyPtr data));

/*
**  Support for the Concert Multi-Thread Architecture (CMA)
*/

#define _BeginCriticalSection
#define _EndCriticalSection

/*
**  Completion codes
*/

#ifdef VMS
#define TerminationSuccess 1
#define TerminationFailure 0
#else
#define TerminationSuccess 0
#define TerminationFailure -1
#endif

/*									   
**  Cursor support
*/
                    
typedef enum __CursorType {
    _DefaultCursor,
    _InactiveCursor,
    _WaitCursor
    } _CursorType;
    
