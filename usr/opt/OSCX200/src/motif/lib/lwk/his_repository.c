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
** COPYRIGHT (c) 1988 BY
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
**	HyperInformation Services
**
**  Version: X0.1
**
**  Abstract:
**	Linkbase object methods 
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner
**
**  Creation Date: 7-Jul-88
**
**  Modification History:
**--
*/


/*
**  Include Files
*/

#ifdef MSDOS
#include "include.h"
#include "repostry.h"
#include "database.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_linkbase.h"
#include "his_database.h"
#endif


/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void SetName, (_Linkbase linkbase));
_DeclareFunction(static void SetIdentifier, (_Linkbase linkbase));

/*
**  External Routine Declarations
*/

/*
**  Global Data Definitions
*/

_Global _LinkbaseTypeInstance;      /* Instance record for Type */
_Global _LinkbaseType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeLinkbaseInstance;
static _LinkbasePropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    LinkbasePropertyNameTable;

static _Linkbase LinkbaseList = (_Linkbase) _NullObject;


void  LwkOpLb(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    return;
    }


_Linkbase  LwkOpLbOpen(type, identifier, create)
_Type type;
 _String identifier;
 _Boolean create;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _String expanded_id;
    _Linkbase linkbase;

    /*
    **  Expand the Identifier
    */

    if (identifier == (_String) _NullObject)
	_Raise(no_such_linkbase);

    expanded_id = LwkLbExpandIdentifier(identifier);

    /*
    **  See if this is a known Linkbase.
    */

    _BeginCriticalSection

    linkbase = LinkbaseList;

    while (linkbase != (_Linkbase) _NullObject) {
	if (_CompareString(_Identifier_of(linkbase), expanded_id) == 0)
	    break;

	linkbase = _Next_of(linkbase);
    }

    _EndCriticalSection

    /*
    **	If we found the Linkbase, make sure that it is Open, otherwise create
    **	a new one.
    */

    if (linkbase != (_Linkbase) _NullObject) {
	_DeleteString(&expanded_id);

	LwkLbOpen(linkbase, create);
    }
    else {
	/*
	**  Create a new Linkbase and set it's Identifier.
	*/

	_StartExceptionBlock

	linkbase = (_Linkbase) _Create(type);

	_Identifier_of(linkbase) = expanded_id;

	/*
	**  Open the database associated with the linkbase
	*/

	LwkLbOpen(linkbase, create);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/
	
	_Exceptions
	    _WhenOthers
		_Delete(&linkbase);
		_Reraise;
	_EndExceptionBlock

	/*
	**  Add the Linkbase to the list of known Repositories
	*/

	_BeginCriticalSection

	_Next_of(linkbase) = LinkbaseList;

	LinkbaseList = linkbase;

	_EndCriticalSection
    }

    /*
    **  Commit the Transaction if one was started by the Open operation
    */

    if (_TransactionState_of(linkbase) == lwk_c_transact_commit)
	LwkLbCommit(linkbase);

    return linkbase;
    }


void  LwkOpLbInitialize(linkbase, proto, aggregate)
_Linkbase linkbase;
 _Linkbase proto;

    _Boolean aggregate;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    /*
    **  Initialize the properties defined by our type
    */

    _ClearValue(&_Identifier_of(linkbase), lwk_c_domain_string);
    _ClearValue(&_Name_of(linkbase), lwk_c_domain_ddif_string);
    _ClearValue(&_Open_of(linkbase), lwk_c_domain_boolean);

    _Next_of(linkbase) = (_Linkbase) _NullObject;
    _TransactionState_of(linkbase) = lwk_c_transact_commit;
    _File_of(linkbase) = (_AnyPtr) 0;
    _ClearValue(&_UseCount_of(linkbase), lwk_c_domain_integer);
    _ClearValue(&_CurrentUid_of(linkbase), lwk_c_domain_integer);
    _ClearValue(&_AllocatedUid_of(linkbase), lwk_c_domain_integer);
    _ClearValue(&_CacheCount_of(linkbase), lwk_c_domain_integer);

    _HashInitialize(_HashTable_of(linkbase), _LinkbaseHashSize);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(linkbase, (_Object) proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_Linkbase) _NullObject) {
	_CopyValue(&_Identifier_of(proto), &_Identifier_of(linkbase),
	    lwk_c_domain_string);
	_CopyValue(&_Name_of(proto), &_Name_of(linkbase),
	    lwk_c_domain_ddif_string);
    }

    return;
    }


void  LwkOpLbExpunge(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    /*
    **	One can not delete a Linkbase which is in use, or from which there
    **	exist persistent objects (as evidenced by their presents in the cache).
    */

    if (_UseCount_of(linkbase) > 0 || _CacheCount_of(linkbase) > 0)
	_Raise(linkbase_in_use);

    /*
    **  Ask our supertype to do the Delete
    */

    _Expunge_S(linkbase, MyType);

    return;
    }


void  LwkOpLbFree(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Linkbase previous;
    _Linkbase next;

    /*
    **  Make sure the database is closed.
    */

    LwkLbClose(linkbase);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_Identifier_of(linkbase), lwk_c_domain_string);
    _DeleteValue(&_Name_of(linkbase), lwk_c_domain_ddif_string);
    _DeleteValue(&_Open_of(linkbase), lwk_c_domain_boolean);

    _DeleteValue(&_TransactionState_of(linkbase), lwk_c_domain_integer);
    _DeleteValue(&_UseCount_of(linkbase), lwk_c_domain_integer);
    _DeleteValue(&_CurrentUid_of(linkbase), lwk_c_domain_integer);
    _DeleteValue(&_AllocatedUid_of(linkbase), lwk_c_domain_integer);
    _DeleteValue(&_CacheCount_of(linkbase), lwk_c_domain_integer);

    /*
    **  Remove the Linkbase from the list of known Repositories
    */

    _BeginCriticalSection

    next = LinkbaseList;
    previous = (_Linkbase) _NullObject;

    while (next != (_Linkbase) _NullObject) {
	if (next == linkbase) {
	    if (previous == (_Linkbase) _NullObject)
		LinkbaseList = _Next_of(next);
	    else
		_Next_of(previous) = _Next_of(next);

	    break;
	}

	previous = next;
	next = _Next_of(next);
    }

    _EndCriticalSection

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(linkbase, MyType);

    return;
    }


_Set  LwkOpLbEnumProps(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Set set;

    /*
    **  Ask our supertype to enumerate its properties.
    */

    set = (_Set) _EnumerateProperties_S(linkbase, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpLbIsMultiValued(linkbase, property)
_Linkbase linkbase;
 _String property;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int index;
    _Boolean answer;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	answer = _IsMultiValued_S(linkbase, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _IdentifierIndex :
	    case _NameIndex :
	    case _OpenIndex :
		answer = _False;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpLbGetValueDomain(linkbase, property)
_Linkbase linkbase;
 _String property;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int index;
    _Domain domain;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	domain = (_Domain) _GetValueDomain_S(linkbase, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _IdentifierIndex :
		domain = lwk_c_domain_string;
		break;

	    case _NameIndex :
		domain = lwk_c_domain_ddif_string;
		break;

	    case _OpenIndex :
		domain = lwk_c_domain_boolean;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpLbGetValue(linkbase, property, domain, value)
_Linkbase linkbase;
 _String property;
 _Domain domain;

    _AnyPtr value;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_GetValue_S(linkbase, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _IdentifierIndex :
		if (_IsDomain(domain, lwk_c_domain_string))
		    _CopyValue(&_Identifier_of(linkbase), value,
			lwk_c_domain_string);
		else
		    _Raise(inv_domain);

		break;

	    case _NameIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_Name_of(linkbase), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);

		break;

	    case _OpenIndex :
		if (_IsDomain(domain, lwk_c_domain_boolean))
		    _CopyValue(&_Open_of(linkbase), value,
			lwk_c_domain_boolean);
		else
		    _Raise(inv_domain);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_List  LwkOpLbGetValueList(linkbase, property)
_Linkbase linkbase;
 _String property;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int index;
    _List value_list;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	return (_List) _GetValueList_S(linkbase, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _IdentifierIndex :
		_ListValue(&_Identifier_of(linkbase), &value_list,
		    lwk_c_domain_string);
		break;

	    case _NameIndex :
		_ListValue(&_Name_of(linkbase), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _OpenIndex :
		_ListValue(&_Open_of(linkbase), &value_list,
		    lwk_c_domain_boolean);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpLbSetValue(linkbase, property, domain, value, flag)
_Linkbase linkbase;
 _String property;

    _Domain domain;
 _AnyPtr value;
 _SetFlag flag;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(linkbase, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _OpenIndex :
		_Raise(property_is_readonly);
		break;

	    case _IdentifierIndex :
		_SetSingleValuedProperty(&_Identifier_of(linkbase),
		    lwk_c_domain_string, domain, value, flag, _False);

		SetIdentifier(linkbase);

		break;

	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(linkbase),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);

		SetName(linkbase);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpLbSetValueList(linkbase, property, values, flag)
_Linkbase linkbase;
 _String property;

    _List values;
 _SetFlag flag;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(linkbase, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _OpenIndex :
		_Raise(property_is_readonly);
		break;

	    case _IdentifierIndex :
		_SetSingleValuedProperty(&_Identifier_of(linkbase),
		    lwk_c_domain_string, lwk_c_domain_list, &values,
		    flag, _True);

		SetIdentifier(linkbase);

		break;

	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(linkbase),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);

		SetName(linkbase);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_Termination  LwkOpLbIterate(linkbase, domain, closure, routine)
_Linkbase linkbase;
 _Domain domain;

    _Closure closure;
 _Callback routine;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Transaction old_state;
    _Termination termination;

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact((_Object) linkbase,
	lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Call the database module to do the grungy work.
    */

    termination = LwkLbIterate(linkbase, domain, closure, routine);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact((_Object) linkbase, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact((_Object) linkbase, lwk_c_transact_commit);

    /*
    **  Return the final termination value
    */

    return termination;
    }


void  LwkOpLbClose(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    /*
    **  Call the database module to do the grungy work.
    */

    LwkLbClose(linkbase);

    return;
    }


_Transaction  LwkOpLbTransact(linkbase, transaction)
_Linkbase linkbase;
 _Transaction transaction;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Transaction state;

    /*
    **  Get the current Transaction state
    */

    state = _TransactionState_of(linkbase);

    /*
    **  If necessary, ask the Database module to do the grungy work
    */

    switch (transaction) {
	case lwk_c_transact_read :
	    if (state != lwk_c_transact_read
		    && state != lwk_c_transact_read_write) {
		LwkLbStartTransaction(linkbase, lwk_c_transact_read);
		_TransactionState_of(linkbase) = lwk_c_transact_read;
	    }

	    break;

	case lwk_c_transact_read_write :
	    if (state != lwk_c_transact_read_write) {
		if (state == lwk_c_transact_read)
		    LwkLbCommit(linkbase);

		LwkLbStartTransaction(linkbase, lwk_c_transact_read_write);

		_TransactionState_of(linkbase) = lwk_c_transact_read_write;
	    }

	    break;

	case lwk_c_transact_commit :
	    if (state == lwk_c_transact_read
		    || state == lwk_c_transact_read_write) {
		LwkLbCommit(linkbase);
		_TransactionState_of(linkbase) = lwk_c_transact_commit;
	    }

	    break;

	case lwk_c_transact_rollback :
	    if (state == lwk_c_transact_read
		    || state == lwk_c_transact_read_write) {
		LwkLbRollback(linkbase);
		_TransactionState_of(linkbase) = lwk_c_transact_commit;
	    }

	    break;

	default :
	    _Raise(inv_argument);
	    break;
    }

    /*
    **  Return the old Transaction state
    */

    return state;
    }


void  LwkOpLbDrop(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    /*
    **  Call the database module to do the grungy work.
    */

    LwkLbDrop(lwk_c_domain_linkbase, (_Persistent) linkbase);

    return;
    }


_Boolean  LwkOpLbVerify(linkbase, flags, file)
_Linkbase linkbase;
 _Integer flags;
 _OSFile file;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Boolean answer;
    _Transaction old_state;

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact((_Object) linkbase,
	lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Let the database module to do the grungy work.
    */

    answer = LwkLbVerify(linkbase, flags, file);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact((_Object) linkbase, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact((_Object) linkbase, lwk_c_transact_commit);

    /*
    **  Return an indication of the whether the verification was successful
    */

    return answer;
    }


static void  SetName(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Transaction old_state;

    /*
    **  Make sure a read/write transaction is in progress
    */

    old_state = (_Transaction) _Transact((_Object) linkbase,
	lwk_c_transact_read_write);

    _StartExceptionBlock

    /*
    **  Call the database module to do the grungy work.
    */

    LwkLbSetName(linkbase);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact((_Object) linkbase, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact((_Object) linkbase, lwk_c_transact_commit);

    return;
    }


static void  SetIdentifier(linkbase)
_Linkbase linkbase;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _String expanded_id;
    _Transaction old_state;

    if (_Identifier_of(linkbase) == (_String) _NullObject)
	_Raise(db_write_error);

    /*
    **  Expand the Identifier
    */

    expanded_id = LwkLbExpandIdentifier(_Identifier_of(linkbase));

    /*
    **  Reset the Identifier of the Linkbase
    */

    _DeleteString(&_Identifier_of(linkbase));
    _Identifier_of(linkbase) = expanded_id;

    /*
    **  Make sure a read/write transaction is in progress
    */

    old_state = (_Transaction) _Transact((_Object) linkbase,
	lwk_c_transact_read_write);

    _StartExceptionBlock

    /*
    **  Call the database module to do the grungy work.
    */

    LwkLbSetIdentifier(linkbase);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact((_Object) linkbase, lwk_c_transact_rollback);

	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact((_Object) linkbase, lwk_c_transact_commit);

    return;
    }
