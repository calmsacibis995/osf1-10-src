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
**	Surrogate object methods 
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
#include "surrogte.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_surrogate.h"
#include "his_database.h"
#include "his_ddis_encoding.h"
#endif /* MSDOS */

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void RetrieveInterLinks,
    (_Surrogate surrogate));
_DeclareFunction(static void SetInterLinks,
    (_Surrogate surrogate, _Domain value_domain, _AnyPtr value,
	_SetFlag flag, _Boolean is_list));
_DeclareFunction(static _Termination _EntryPt SetInterLinkState,
    (_Closure closure, _Set interlinks, _Domain domain,
	_Persistent *interlink));
_DeclareFunction(static _Boolean CheckInterLinks,
    (_Surrogate surrogate, lwk_query_operator operator,
    _QueryExpression expression));
_DeclareFunction(static _Termination _EntryPt SelectInterLink,
    (_Closure expression, _Set set, _Domain domain,
	_Persistent *interlink));
_DeclareFunction(static void MoveSurrogate,
    (_Surrogate surrogate, _Persistent old));

/*
**  Global Data Definitions
*/

_Global _SurrogateTypeInstance;      /* Instance record for Type */
_Global _SurrogateType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeSurrogateInstance;
static _SurrogatePropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    SurrogatePropertyNameTable;


void  LwkOpSurr(surrogate)
_Surrogate surrogate;

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


void  LwkOpSurrInitialize(surrogate, proto, aggregate)
_Surrogate surrogate;
 _Surrogate proto;

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

    _ClearValue(&_SurrogateSubType_of(surrogate), lwk_c_domain_string);
    _ClearValue(&_Container_of(surrogate), lwk_c_domain_persistent);
    _ClearValue(&_InterLinks_of(surrogate), lwk_c_domain_set);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(surrogate, (_Object) proto, aggregate, MyType);
        
    /*
    **  If a prototype object was provided, copy properties from it.
    */

    if (proto != (_Surrogate) _NullObject) {
	_CopyValue(&_SurrogateSubType_of(proto),
	    &_SurrogateSubType_of(surrogate), lwk_c_domain_string);
    }

    return;
    }


void  LwkOpSurrExpunge(surrogate)
_Surrogate surrogate;

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
    _Boolean phase_2;
    _Persistent old_container;

    /*
    **	Can not delete a Surrogate that is interconnected by (Retrieved)
    **	Links or Steps (otherwise we end up with dangling references from
    **	the Links/Steps).
    */

    if ((_Boolean) _SetState((_Persistent) surrogate,
	    _StateHaveInterLinks, _StateGet))
	if ((_Boolean) _SetState((_Persistent) surrogate, _StateHasInComming,
		    _StateGet)
	       || (_Boolean) _SetState((_Persistent) surrogate,
		    _StateHasOutGoing, _StateGet))
	    _Raise(cannot_delete_surrogate);

    /*
    ** Complement the Pending Delete state.  If it was already set, we need to
    ** complete phase 2 of the delete process.  If the Surrogate is not Stored
    ** in a Repository, we can complete phase 2 now too.
    */

    if ((_Boolean) _SetState((_Persistent) surrogate, _StateDeletePending,
	    _StateComplement))
	phase_2 = _True;
    else {
	_Linkbase repository;

	_GetValue((_Persistent) surrogate, _P_Linkbase,
	    lwk_c_domain_linkbase, &repository);

	if (repository == (_Linkbase) _NullObject)
	    phase_2 = _True;
	else
	    phase_2 = _False;
    }

    /*
    ** No phase 1 tasks other than setting Pending Delete (done above).
    */

    /*
    ** On phase 2, remove the Surrogate from its Container, and complete the
    ** deletion process.
    */

    if (phase_2) {
	old_container = _Container_of(surrogate);
	_Container_of(surrogate) = (_Persistent) _NullObject;

	MoveSurrogate(surrogate, old_container);

	_Expunge_S(surrogate, MyType);
    }

    return;
    }


void  LwkOpSurrFree(surrogate)
_Surrogate surrogate;

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
    **  If this Surrogate was part of a Network or Path, remove it from the
    **  _Surrogates list of that object.
    */

    if (_Container_of(surrogate) != (_Persistent) _NullObject)
	_SetValue(_Container_of(surrogate), _P_Surrogates,
	    lwk_c_domain_surrogate, &surrogate, lwk_c_remove_property);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_SurrogateSubType_of(surrogate), lwk_c_domain_string);
    _DeleteValue(&_InterLinks_of(surrogate), lwk_c_domain_set);
    _DeleteValue(&_Container_of(surrogate), lwk_c_domain_persistent);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(surrogate, MyType);

    return;
    }


_Set  LwkOpSurrEnumProps(surrogate)
_Surrogate surrogate;

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

    set = (_Set) _EnumerateProperties_S(surrogate, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpSurrIsMultiValued(surrogate, property)
_Surrogate surrogate;
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
	answer = _IsMultiValued_S(surrogate, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _SurrogateSubTypeIndex :
	    case _ContainerIndex :
		answer = _False;
		break;

	    case _InterLinksIndex :
		answer = _True;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpSurrGetValueDomain(surrogate, property)
_Surrogate surrogate;
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
	domain = (_Domain) _GetValueDomain_S(surrogate, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _SurrogateSubTypeIndex :
		domain = lwk_c_domain_string;
		break;

	    case _ContainerIndex :
		domain = lwk_c_domain_persistent;
		break;

	    case _InterLinksIndex :
		domain = lwk_c_domain_set;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpSurrGetValue(surrogate, property, domain, value)
_Surrogate surrogate;
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
	_GetValue_S(surrogate, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _SurrogateSubTypeIndex :
		if (_IsDomain(domain, lwk_c_domain_string))
		    _CopyValue(&_SurrogateSubType_of(surrogate), value,
			lwk_c_domain_string);
		else
		    _Raise(inv_domain);

		break;

	    case _ContainerIndex :
		if (_IsDomain(domain, lwk_c_domain_persistent))
		    _CopyValue(&_Container_of(surrogate), value,
			lwk_c_domain_persistent);
		else
		    _Raise(inv_domain);

		break;

	    case _InterLinksIndex :
		/*
		**  Retrieve the InterLinks if we don't have them.
		*/

		if (!(_Boolean) _SetState((_Persistent) surrogate,
			_StateHaveInterLinks, _StateSet))
		    RetrieveInterLinks(surrogate);

		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_InterLinks_of(surrogate), value,
			lwk_c_domain_set);
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


_List  LwkOpSurrGetValueList(surrogate, property)
_Surrogate surrogate;
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
	value_list = (_List) _GetValueList_S(surrogate, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _SurrogateSubTypeIndex :
		_ListValue(&_SurrogateSubType_of(surrogate), &value_list,
		    lwk_c_domain_string);
		break;

	    case _ContainerIndex :
		_ListValue(&_Container_of(surrogate), &value_list,
		    lwk_c_domain_persistent);
		break;

	    case _InterLinksIndex :
		/*
		**  Retrieve the InterLinks if we don't have them.
		*/

		if (!(_Boolean) _SetState((_Persistent) surrogate,
			_StateHaveInterLinks, _StateSet))
		    RetrieveInterLinks(surrogate);

		_CopyValue(&_InterLinks_of(surrogate), &value_list,
		    lwk_c_domain_set);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpSurrSetValue(surrogate, property, domain, value, flag)
_Surrogate surrogate;
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
    _Persistent old_container;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(surrogate, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _SurrogateSubTypeIndex :
		_SetSingleValuedProperty(&_SurrogateSubType_of(surrogate),
		    lwk_c_domain_string, domain, value, flag, _False);
		break;

	    case _ContainerIndex :
		old_container = _Container_of(surrogate);

		_SetSingleValuedProperty(&_Container_of(surrogate),
		    lwk_c_domain_persistent, domain, value, flag, _False);

		/*
		**  Move this Surrogate from its old Container to its new
		**  Container.
		*/

		MoveSurrogate(surrogate, old_container);

		break;

	    case _InterLinksIndex :
		SetInterLinks(surrogate, domain, value, flag, _False);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Surrogate Modified.
    */

    _SetState((_Persistent) surrogate, _StateModified, _StateSet);

    return;
    }


void  LwkOpSurrSetValueList(surrogate, property, values, flag)
_Surrogate surrogate;
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
    _Persistent old_container;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(surrogate, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _SurrogateSubTypeIndex :
		_SetSingleValuedProperty(&_SurrogateSubType_of(surrogate),
		    lwk_c_domain_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _ContainerIndex :
		old_container = _Container_of(surrogate);

		_SetSingleValuedProperty(&_Container_of(surrogate),
		    lwk_c_domain_persistent, lwk_c_domain_list, &values,
		    flag, _True);

		/*
		**  Move this Surrogate from its old Container to its new
		**  Container.
		*/

		MoveSurrogate(surrogate, old_container);

		break;

	    case _InterLinksIndex :
		SetInterLinks(surrogate, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Surrogate Modified.
    */

    _SetState((_Persistent) surrogate, _StateModified, _StateSet);

    return;
    }


_ObjectId  LwkOpSurrGetObjectId(surrogate)
_Surrogate surrogate;

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
    _Integer container_id;
    _ObjectId volatile oid;

    /*
    ** Let our supertype generate the basic ObjectId
    */
    
    oid = (_ObjectId) _GetObjectId_S(surrogate, MyType);

    _StartExceptionBlock

    /*
    **  Find the Identifier of the Container in which the Surrogate is stored
    */

    _GetValue(_Container_of(surrogate), _P_Identifier, lwk_c_domain_integer,
	&container_id);

    /*
    **  Set the Container Identifier property of the ObjectId.
    */

    _SetValue(oid, _P_ContainerIdentifier, lwk_c_domain_integer,
	&container_id, lwk_c_set_property);

    /*
    **	If any exceptions are raised, delete any new ObjectId then reraise the
    **	exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&oid);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the ObjectId.
    */

    return oid;
    }


void  LwkOpSurrEncode(surrogate, aggregate, handle)
_Surrogate surrogate;
 _Boolean aggregate;

    _DDIShandle handle;

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
    _DDISstatus status;
    _DDIStype type;
    _DDISlength value_length;

    /*
    **  Begin the encoding of Surrogate.
    */

    type = LWK_K_T_PERS_SURROGATE;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encoding the properties of Surrogate.
    */

    if (_SurrogateSubType_of(surrogate) != (_String) _NullObject) {
	type = LWK_K_T_SURR_SUBTYPE;
	value_length = _LengthString(_SurrogateSubType_of(surrogate));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _SurrogateSubType_of(surrogate), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    /*
    **  Finish the Surrogate encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Ask our supertype to encode its properties
    */

    _Encode_S(surrogate, aggregate, handle, MyType);

    return;
    }


void  LwkOpSurrDecode(surrogate, handle, keys_only, properties)
_Surrogate surrogate;
 _DDIShandle handle;

    _Boolean keys_only;
 _Set *properties;

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
    _DDISstatus status;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;

    /*
    **  Begin the Surrogate decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_SURROGATE)
	_Raise(inv_encoding);

    /*
    **	Decode the Surrogate Properties.  If extracting Key Properties, most
    **	properties can be skipped -- only Surrogate Subtype is a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PERS_SURROGATE:
		/*
		**  End of Surrogate Properties.
		*/
		break;

	    case LWK_K_P_SURR_SUBTYPE :
		_SurrogateSubType_of(surrogate) =
		    _AllocateMem(value_length + 1);

		status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		    (_DDISdata) _SurrogateSubType_of(surrogate), &get_length,
		    &additional_info);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (value_length != get_length)
		    _Raise(internal_decoding_error);

		_SurrogateSubType_of(surrogate)[get_length] = _EndOfString;

		if (keys_only) {
		    _String name;
		    _Property property;

		    /*
		    **  Create a Property with the appropriate values
		    */

		    property = (_Property) _Create(_TypeProperty);

		    name = (_String) _GenericTypeKeyName;

		    _SetValue(property, _P_PropertyName, lwk_c_domain_string,
			&name, lwk_c_set_property);

		    _SetValue(property, _P_Value, lwk_c_domain_string,
			&_SurrogateSubType_of(surrogate), lwk_c_set_property);
		    /*
		    **  Add it to the Key Property Set
		    */

		    if (*properties == (_Set) _NullObject)
			*properties = (_Set) _CreateSet(_TypeSet,
			    lwk_c_domain_property, (_Integer) 0);

		    _AddElement(*properties, lwk_c_domain_property, &property,
			_False);
		}

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_PERS_SURROGATE);


    /*
    **  Ask our supertype to decode its properties
    */

    _Decode_S(surrogate, handle, keys_only, properties, MyType);

    return;
    }


_Boolean  LwkOpSurrSelected(surrogate, expression)
_Surrogate surrogate;
 _QueryExpression expression;

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
    _Boolean result;
    _QueryExpression operand_1;
    _QueryExpression operand_2;

    /*
    **  Dispatch based on the operator in root of the Selection Expression.
    */
    
    if (expression == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    operand_1 = (_QueryExpression) expression->lwk_operand_1;
    operand_2 = (_QueryExpression) expression->lwk_operand_2;

    switch (expression->lwk_operator) {
	case lwk_c_and :
	    /*
	    **  True if both the left and right Selection sub-Expressions are
	    **	True.
	    */

	    result = _Selected((_Persistent) surrogate, operand_1)
		&& _Selected((_Persistent) surrogate, operand_2);

	    break;

	case lwk_c_or :
	    /*
	    **  True if either the left or right Selection sub-Expressions is
	    **	True.
	    */

	    result = _Selected((_Persistent) surrogate, operand_1)
		|| _Selected((_Persistent) surrogate, operand_2);

	    break;

	case lwk_c_not :
	    /*
	    **  True if the left Selection sub-Expression is False.
	    */

	    result = !_Selected((_Persistent) surrogate, operand_1);

	    break;

	case lwk_c_any :
	    /*
	    **  Always True.
	    */

	    result = _True;

	    break;

	case lwk_c_identity :
	    /*
	    **  True if the left operand is this Surrogate.
	    */

	    result = (surrogate == (_Surrogate) expression->lwk_operand_1);

	    break;

	case lwk_c_is_the_source_of :
	case lwk_c_is_the_target_of :
	case lwk_c_is_some_source_of :
	case lwk_c_is_some_target_of :
	case lwk_c_is_the_origin_of :
	case lwk_c_is_the_destination_of :
	case lwk_c_is_some_origin_of :
	case lwk_c_is_some_destination_of :
	    /*
	    **  True if this Surrogate has the specified relationship to its
	    **	Links/Steps.
	    */

	    result = CheckInterLinks(surrogate, expression->lwk_operator,
		operand_1);

	    break;

	case lwk_c_has_properties :
	    /*
	    **  True if the properties match.
	    */

	    result = _SelectProperties((_Object) surrogate, operand_1);

	    break;

	default :
	    _Raise(inv_query_expr);
	    break;

    }

    return result;
    }


static void  RetrieveInterLinks(surrogate)
_Surrogate surrogate;

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
    _Domain domain;
    _List interlinks;
    _Boolean surr_modified;
    _Boolean cont_modified;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Preserve the Modified state of the Surrogate and its Container
    */

    surr_modified = (_Boolean) _SetState((_Persistent) surrogate,
	_StateModified, _StateGet);

    cont_modified = (_Boolean) _SetState((_Persistent) _Container_of(surrogate),
	_StateModified, _StateGet);

    /*
    **  Save, then clear any List of InterLink Ids
    */

    if ((_Boolean) _SetState((_Persistent) surrogate,
	    _StateInterLinksAreIds, _StateClear)) {
	interlinks = _InterLinks_of(surrogate);
	_InterLinks_of(surrogate) = (_Set) _NullObject;
    }
    else
	interlinks = (_Set) _NullObject;

    /*
    **  Determine the type of the InterLinks and the Repository where they
    **	are stored.
    */

    domain = _TypeToDomain(_Type_of(_Container_of(surrogate)));

    /*
    **  Find the Repository in which the Surrogate is stored
    */

    _GetValue((_Persistent) surrogate, _P_Linkbase,
	lwk_c_domain_linkbase, &repository);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **	Retrieve the InterLinks, using their Ids if previously provided.
    **	Note: the List of Ids, if any, will be deleted by the Database module.
    */

    switch (domain) {
	case lwk_c_domain_linknet :
	    LwkLbRetrieveSurrLinks(repository, surrogate,
		_Container_of(surrogate), interlinks);
	    break;

#ifndef MSDOS

	case lwk_c_domain_path :
	    LwkLbRetrieveSurrSteps(repository, surrogate,
		_Container_of(surrogate), interlinks);
	    break;

#endif /* !MSDOS */

    }

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact(repository, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact(repository, lwk_c_transact_commit);

    /*
    **  Restore the Modified state of the Surrogate and its Container
    */

    if (!surr_modified)
	_SetState((_Persistent) surrogate, _StateModified, _StateClear);
    if (!cont_modified)
	_SetState((_Persistent) _Container_of(surrogate), _StateModified,
	    _StateClear);

    return;
    }

static void  SetInterLinks(surrogate, value_domain, value, flag, is_list)
_Surrogate surrogate;
 _Domain value_domain;

    _AnyPtr value;
 _SetFlag flag;
 _Boolean is_list;

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
    _Domain domain;
    _Integer integer;

    /*
    **	If the domain is Integer or List_of(Integer), these are the
    **	InterLink Ids.  Save them and mark the state accordingly.
    **	Otherwise, they must be the InterLinks themselves.  Deal with
    **	them accordingly.
    */

    domain = value_domain;

    if (_IsDomain(lwk_c_domain_list, value_domain))
	if ((_ListPtr) value != (_ListPtr) _NullObject)
	    if (*((_ListPtr) value) != (_List) _NullObject) {
		_GetValue(*((_ListPtr) value), _P_Domain,
		    lwk_c_domain_integer, &integer);

		domain = (_Domain) integer;
	    }

    if (domain == lwk_c_domain_integer) {
	_SetMultiValuedProperty(&_InterLinks_of(surrogate),
	    lwk_c_domain_integer, value_domain, value, flag, is_list, _True);

	_SetState((_Persistent) surrogate, _StateInterLinksAreIds,
	    _StateSet);
    }
    else {
	/*
	**  Retrieve the existing InterLinks if we don't have them.
	*/

	if (!(_Boolean) _SetState((_Persistent) surrogate,
		_StateHaveInterLinks, _StateSet))
	    RetrieveInterLinks(surrogate);

	_SetMultiValuedProperty(&_InterLinks_of(surrogate),
	    lwk_c_domain_persistent, value_domain, value, flag, is_list, _True);

	/*
	**  Interate over the InterLinks and update the InterLinks
	**  state appropriately.
	*/

	_SetState((_Persistent) surrogate, _StateHasInComming, _StateClear);
	_SetState((_Persistent) surrogate, _StateHasOutGoing, _StateClear);

	if (_InterLinks_of(surrogate) != (_Set) _NullObject)
	    _Iterate(_InterLinks_of(surrogate), lwk_c_domain_persistent,
		(_Closure) surrogate, SetInterLinkState);
    }

    return;
    }


static _Termination _EntryPt  SetInterLinkState(closure, interlinks, domain, interlink)
_Closure closure;

    _Set interlinks;
 _Domain domain;
 _Persistent *interlink;

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
    _Surrogate surrogate;
    _Surrogate connected;
    _Boolean has_incomming;
    _Boolean has_outgoing;
    _Domain interlink_domain;

    surrogate = (_Surrogate) closure;

    /*
    **  Get the current InterLink state of the Surrogate
    */

    has_incomming = (_Boolean) _SetState((_Persistent) surrogate,
	_StateHasInComming, _StateGet);

    has_outgoing = (_Boolean) _SetState((_Persistent) surrogate,
	_StateHasOutGoing, _StateGet);

    /*
    **  Get the real Domain of the InterLink (the one provided is the
    **	common super type: Persistent).
    */

    interlink_domain = _TypeToDomain(_Type_of(*interlink));

    /*
    **	If the Surrogate is not already known to have an outgoing
    **	InterLink, check to see if is is the Source or Origin of this
    **	InterLink.  Note that if the Source/Origin of the InterLink
    **	is an Id, then it can not be this Surrogate.
    */

    if (!has_outgoing) {
	switch (interlink_domain) {
	    case lwk_c_domain_link :
		if ((_Boolean) _SetState((_Persistent) *interlink,
			_StateSourceIsId, _StateGet))
		    connected = (_Surrogate) _NullObject;
		else
		    _GetValue(*interlink, _P_Source,
			lwk_c_domain_surrogate, &connected);
		break;

#ifndef MSDOS

	    case lwk_c_domain_step :
		if ((_Boolean) _SetState((_Persistent) *interlink,
			_StateOriginIsId, _StateGet))
		    connected = (_Surrogate) _NullObject;
		else
		    _GetValue(*interlink, _P_Origin,
			lwk_c_domain_surrogate, &connected);
		break;

#endif /* !MSDOS */

	}

	/*
	**  If it matches the given Surrogate, set the OutGoing state
	*/

	if (connected == surrogate) {
	    _SetState((_Persistent) surrogate, _StateHasOutGoing, _StateSet);
	    has_outgoing = _True;
	}
    }

    /*
    **	If the Surrogate is not already known to have an incomming
    **	InterLink, check to see if is is the Target or Destination of
    **	this InterLink.  Note that if the Target/Destination of the
    **	InterLink is an Id, then it can not be this Surrogate.
    */

    if (!has_incomming) {
	switch (interlink_domain) {
	    case lwk_c_domain_link :
		if ((_Boolean) _SetState((_Persistent) *interlink,
			_StateTargetIsId, _StateGet))
		    connected = (_Surrogate) _NullObject;
		else
		    _GetValue(*interlink, _P_Target,
			lwk_c_domain_surrogate, &connected);
		break;

#ifndef MSDOS

	    case lwk_c_domain_step :
		if ((_Boolean) _SetState((_Persistent) *interlink,
			_StateDestinationIsId, _StateGet))
		    connected = (_Surrogate) _NullObject;
		else
		    _GetValue(*interlink, _P_Destination,
			lwk_c_domain_surrogate, &connected);
		break;

#endif /* !MSDOS */

	}

	/*
	**  If it matches the given Surrogate, set the InComming flag
	*/

	if (connected == surrogate) {
	    _SetState((_Persistent) surrogate, _StateHasInComming, _StateSet);
	    has_incomming = _True;
	}
    }

    /*
    **	If we know that the given Surrogate has both InComming and OutGoing
    **	interlinks, we can terminate the iteration.  Otherwise, continue.
    */

    if (has_incomming && has_outgoing)
	return (_Termination) 1;
    else
	return (_Termination) 0;
    }


static _Boolean  CheckInterLinks(surrogate, operator, expression)
_Surrogate surrogate;

    lwk_query_operator operator;
 _QueryExpression expression;

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
    _Boolean result;
    _Integer count;
    _Domain domain;
    _Termination termination;
    _Boolean has_incomming;
    _Boolean has_outgoing;
    lwk_query_node and;
    lwk_query_node has;
    lwk_query_node identity;

    /*
    **	If the Surrogate does not participate in any InterLinks, see if
    **	that was what was selected -- if so, return True, else return False.
    */

    has_incomming = (_Boolean) _SetState((_Persistent) surrogate,
	_StateHasInComming, _StateGet);

    has_outgoing = (_Boolean) _SetState((_Persistent) surrogate,
	_StateHasOutGoing, _StateGet);

    if (!has_incomming && !has_outgoing) {
	if (expression == (_QueryExpression) _NullObject)
	    return _True;
	else
	    return _False;
    }

    /*
    **	If the relationship requires that there be only one InterLink,
    **	and this is not the case, return False.
    */

    switch (operator) {
	case lwk_c_is_the_source_of :
	case lwk_c_is_the_target_of :
	case lwk_c_is_the_origin_of :
	case lwk_c_is_the_destination_of :
	    if (!(_Boolean) _SetState((_Persistent) surrogate,
		    _StateHaveInterLinks, _StateSet))
		RetrieveInterLinks(surrogate);

	    _GetValue(_InterLinks_of(surrogate), _P_ElementCount,
		lwk_c_domain_integer, &count);

	    if (count != 1)
		return _False;

	    break;
    }

    /*
    **	If the criteria is that any InterLink will do, check to see if
    **	there are any (without necessarily having to Retrieve them).
    */

    if (expression->lwk_operator == lwk_c_any) {
	switch (operator) {
	    case lwk_c_is_the_source_of :
	    case lwk_c_is_some_source_of :
	    case lwk_c_is_the_origin_of :
	    case lwk_c_is_some_origin_of :
		if (has_outgoing)
		    return _True;
		else
		    return _False;
		break;

	    case lwk_c_is_the_target_of :
	    case lwk_c_is_some_target_of :
	    case lwk_c_is_the_destination_of :
	    case lwk_c_is_some_destination_of :
		if (has_incomming)
		    return _True;
		else
		    return _False;
		break;
	}
    }

    /*
    **	Build a new Selection Expression and use it to Select the
    **	interlink(s) associated with this Surrogate.
    */

    and.lwk_operator  = lwk_c_and;
    and.lwk_operand_1 = (lwk_any_pointer) &has;
    and.lwk_operand_2 = (lwk_any_pointer) expression;

    switch (operator) {
	case lwk_c_is_the_source_of :
	case lwk_c_is_some_source_of :
	    has.lwk_operator = lwk_c_has_source;
	    domain = lwk_c_domain_link;
	    break;

	case lwk_c_is_the_origin_of :
	case lwk_c_is_some_origin_of :
	    has.lwk_operator = lwk_c_has_origin;
	    domain = lwk_c_domain_step;
	    break;

	case lwk_c_is_the_target_of :
	case lwk_c_is_some_target_of :
	    has.lwk_operator = lwk_c_has_target;
	    domain = lwk_c_domain_link;
	    break;

	case lwk_c_is_the_destination_of :
	case lwk_c_is_some_destination_of :
	    has.lwk_operator = lwk_c_has_destination;
	    domain = lwk_c_domain_step;
	    break;
    }

    has.lwk_operand_1 = (lwk_any_pointer) &identity;
    has.lwk_operand_2 = (lwk_any_pointer) _NullObject;

    identity.lwk_operator  = lwk_c_identity;
    identity.lwk_operand_1 = (lwk_any_pointer) surrogate;
    identity.lwk_operand_2 = (lwk_any_pointer) _NullObject;

    if (!(_Boolean) _SetState((_Persistent) surrogate,
	    _StateHaveInterLinks, _StateSet))
	RetrieveInterLinks(surrogate);

    termination = _Iterate(_InterLinks_of(surrogate), domain,
	(_Closure) &and, SelectInterLink);

    result = (termination == 1);

    return result;
    }


static _Termination _EntryPt  SelectInterLink(expression, set, domain, interlink)
_Closure expression;

    _Set set;
 _Domain domain;
 _Persistent *interlink;

    {
    _Termination termination;

    if (_Selected(*interlink, (_QueryExpression) expression))
	termination = (_Termination) 1;
    else
	termination = (_Termination) 0;

    return termination;
    }


static void  MoveSurrogate(surrogate, old)
_Surrogate surrogate;
 _Persistent old;

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
    _Boolean has_incomming;
    _Boolean has_outgoing;

    /*
    **  The _Container property of a Surrogate may be set only in the following
    **	cases:
    **
    **	    - The new _Container is the same as the old _Container (i.e, a NoOp)
    **	    - There was no previous _Container
    **	    - There are no _InterLinks binding the Surrogate in the old
    **	      _Container
    **
    **	If these conditions are not met, the _Container is reset to the old
    **	value and an exception is raised.
    */

    if (old != _Container_of(surrogate)) {
	if (old != (_Persistent) _NullObject) {
	    has_incomming = (_Boolean) _SetState((_Persistent) old,
		_StateHasInComming, _StateGet);

	    has_outgoing = (_Boolean) _SetState((_Persistent) old,
		_StateHasOutGoing, _StateGet);

	    if (has_incomming || has_outgoing) {
		_Container_of(surrogate) = old;
		_Raise(cannot_move_surrogate);
	    }
	}

	/*
	** Remove this Surrrogate from the _Surrogates property of the old
	** Container if there was one.
	*/

	if (old != (_Persistent) _NullObject)
	    _SetValue(old, _P_Surrogates, lwk_c_domain_surrogate, &surrogate,
		lwk_c_remove_property);

	/*
	** Add this Surrogate to the _Surrogates property of the new
	** Container if there is one.
	*/

	if (_Container_of(surrogate) != (_Persistent) _NullObject)
	    _SetValue(_Container_of(surrogate), _P_Surrogates,
		lwk_c_domain_surrogate, &surrogate, lwk_c_add_property);
    }

    return;
    }
