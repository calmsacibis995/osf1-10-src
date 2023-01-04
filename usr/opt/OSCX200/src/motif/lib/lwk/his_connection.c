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
**	Link object methods
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
#include "connectn.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_link.h"
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

_DeclareFunction(static void MoveLink,
    (_Link volatile link, _Linknet volatile network));
_DeclareFunction(static void SetSource,
    (_Link volatile link, _Surrogate volatile old));
_DeclareFunction(static void SetTarget,
    (_Link volatile link, _Surrogate volatile old));
_DeclareFunction(static void RetrieveSource, (_Link link));
_DeclareFunction(static void RetrieveTarget, (_Link link));
_DeclareFunction(static void MoveSurrogate,
    (_Surrogate surrogate, _Linknet network));
_DeclareFunction(static void EncodeSourceKeywords,
    (_Link link, _DDIShandle handle));
_DeclareFunction(static void EncodeTargetKeywords,
    (_Link link, _DDIShandle handle));
_DeclareFunction(static _Termination _EntryPt EncodeKeyword,
    (_Closure closure, _Set keywords, _Domain domain, _DDIFString *keyword));
_DeclareFunction(static void EncodeSurrogate,
    (_Surrogate surrogate, _DDIStype type, _DDIShandle handle));
_DeclareFunction(static void DecodeSourceKeywords,
    (_Link link, _DDIShandle handle, _Boolean keys_only));
_DeclareFunction(static void DecodeTargetKeywords,
    (_Link link, _DDIShandle handle, _Boolean keys_only));
_DeclareFunction(static _Surrogate DecodeSurrogate,
    (_DDIShandle handle, _DDISlength value_length));

/*
**  Global Data Definitions
*/

_Global _LinkTypeInstance;      /* Instance record for Type */
_Global _LinkType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeLinkInstance;
static _LinkPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    LinkPropertyNameTable;


void  LwkOpLink(link)
_Link link;

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


void  LwkOpLinkInitialize(link, proto, aggregate)
_Link link;
 _Link proto;

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

    _ClearValue(&_RelationshipType_of(link), lwk_c_domain_ddif_string);
    _ClearValue(&_SourceDescription_of(link), lwk_c_domain_ddif_string);
    _ClearValue(&_SourceKeywords_of(link), lwk_c_domain_set);
    _ClearValue(&_TargetDescription_of(link), lwk_c_domain_ddif_string);
    _ClearValue(&_TargetKeywords_of(link), lwk_c_domain_set);
    _ClearValue(&_Source_of(link).surrogate, lwk_c_domain_surrogate);
    _ClearValue(&_Target_of(link).surrogate, lwk_c_domain_surrogate);
    _ClearValue(&_Linknet_of(link), lwk_c_domain_linknet);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(link, (_Object) proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_Link) _NullObject) {
	_CopyValue(&_RelationshipType_of(proto),
	    &_RelationshipType_of(link), lwk_c_domain_ddif_string);
	_CopyValue(&_SourceDescription_of(proto),
	    &_SourceDescription_of(link), lwk_c_domain_ddif_string);
	_CopyValue(&_SourceKeywords_of(proto), &_SourceKeywords_of(link),
	    lwk_c_domain_set);
	_CopyValue(&_TargetDescription_of(proto),
	    &_TargetDescription_of(link), lwk_c_domain_ddif_string);
	_CopyValue(&_TargetKeywords_of(proto), &_TargetKeywords_of(link),
	    lwk_c_domain_set);

	if (aggregate) {
	    if (_Source_of(proto).surrogate != (_Surrogate) _NullObject)
		_Source_of(link).surrogate = (_Surrogate)
		    _Copy(_Source_of(proto).surrogate, _False);

	    if (_Target_of(proto).surrogate != (_Surrogate) _NullObject)
		_Target_of(link).surrogate = (_Surrogate)
		    _Copy(_Target_of(proto).surrogate, _False);
	}
    }

    return;
    }


void  LwkOpLinkExpunge(link)
_Link link;

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
    _Linknet old_network;
    _Surrogate old_surrogate;

    /*
    ** Complement the Pending Delete state.  If it was already set, we need to
    ** complete phase 2 of the delete process.  If the Link is not Stored
    ** in a Repository, we can complete phase 2 now too.
    */

    if ((_Boolean) _SetState((_Persistent) link, _StateDeletePending,
	    _StateComplement))
	phase_2 = _True;
    else {
	_Linkbase repository;

	_GetValue((_Object) link, _P_Linkbase, lwk_c_domain_linkbase,
	    &repository);

	if (repository == (_Linkbase) _NullObject)
	    phase_2 = _True;
	else
	    phase_2 = _False;
    }

    /*
    ** Always clear the Source and Target of the Link
    */

    if ((_Boolean) _SetState((_Persistent) link, _StateSourceIsId,
	    _StateClear))
	_Source_of(link).surrogate = (_Surrogate) _NullObject;
    else {
	old_surrogate = _Source_of(link).surrogate;
	_Source_of(link).surrogate = (_Surrogate) _NullObject;
	SetSource(link, old_surrogate);
    }

    if ((_Boolean) _SetState((_Persistent) link, _StateTargetIsId,
	    _StateClear))
	_Target_of(link).surrogate = (_Surrogate) _NullObject;
    else {
	old_surrogate = _Target_of(link).surrogate;
	_Target_of(link).surrogate = (_Surrogate) _NullObject;
	SetTarget(link, old_surrogate);
    }

    /*
    ** On phase 2, remove the Link from its Network, and complete the
    ** deletion process.
    */

    if (phase_2) {
	old_network = _Linknet_of(link);
	_Linknet_of(link) = (_Linknet) _NullObject;

	MoveLink(link, old_network);

	_Expunge_S(link, MyType);
    }

    return;
    }


void  LwkOpLinkFree(link)
_Link link;

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
    _Boolean is_id;

    /*
    **  If this Link was part of a Network, remove it from the
    **  _Surrogates list of that Network.
    */

    if (_Linknet_of(link) != (_Linknet) _NullObject)
	_SetValue(_Linknet_of(link), _P_Links,
	    lwk_c_domain_link, &link, lwk_c_remove_property);

    /*
    **  If there was a _Source, remove this Link from the
    **  _InterLinks list of that Surrogate.
    */

    is_id = (_Boolean) _SetState((_Persistent) link, _StateSourceIsId,
	_StateGet);

    if (!is_id && _Source_of(link).surrogate != (_Surrogate) _NullObject)
	_SetValue(_Source_of(link).surrogate, _P_InterLinks,
	    lwk_c_domain_link, &link, lwk_c_remove_property);

    /*
    **  If there was a _Target, remove this Link from the
    **  _InterLinks list of that Surrogate.
    */

    is_id = (_Boolean) _SetState((_Persistent) link, _StateTargetIsId,
	_StateGet);

    if (!is_id && _Target_of(link).surrogate != (_Surrogate) _NullObject)
	_SetValue(_Target_of(link).surrogate, _P_InterLinks,
	    lwk_c_domain_link, &link, lwk_c_remove_property);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_RelationshipType_of(link), lwk_c_domain_ddif_string);
    _DeleteValue(&_SourceDescription_of(link), lwk_c_domain_ddif_string);
    _DeleteValue(&_SourceKeywords_of(link), lwk_c_domain_set);
    _DeleteValue(&_TargetDescription_of(link), lwk_c_domain_ddif_string);
    _DeleteValue(&_TargetKeywords_of(link), lwk_c_domain_set);
    _DeleteValue(&_Linknet_of(link), lwk_c_domain_linknet);
    _DeleteValue(&_Source_of(link).surrogate, lwk_c_domain_surrogate);
    _DeleteValue(&_Target_of(link).surrogate, lwk_c_domain_surrogate);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(link, MyType);

    return;
    }


_Set  LwkOpLinkEnumProps(link)
_Link link;

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

    set = (_Set) _EnumerateProperties_S(link, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpLinkIsMultiValued(link, property)
_Link link;
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
	answer = _IsMultiValued_S(link, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _RelationshipTypeIndex :
	    case _SourceDescriptionIndex :
	    case _TargetDescriptionIndex :
	    case _SourceIndex :
	    case _TargetIndex :
	    case _LinknetIndex :
		answer = _False;
		break;

	    case _SourceKeywordsIndex :
	    case _TargetKeywordsIndex :
		answer = _False;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpLinkGetValueDomain(link, property)
_Link link;
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
	domain = (_Domain) _GetValueDomain_S(link, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _RelationshipTypeIndex :
	    case _SourceDescriptionIndex :
	    case _TargetDescriptionIndex :
		domain = lwk_c_domain_ddif_string;
		break;

	    case _SourceKeywordsIndex :
	    case _TargetKeywordsIndex :
		domain = lwk_c_domain_set;
		break;

	    case _SourceIndex :
	    case _TargetIndex :
		domain = lwk_c_domain_surrogate;
		break;

	    case _LinknetIndex :
		domain = lwk_c_domain_linknet;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpLinkGetValue(link, property, domain, value)
_Link link;
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
	_GetValue_S(link, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _RelationshipTypeIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_RelationshipType_of(link), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);

		break;

	    case _SourceDescriptionIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_SourceDescription_of(link), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);

		break;

	    case _TargetDescriptionIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_TargetDescription_of(link), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);

		break;

	    case _SourceKeywordsIndex :
		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_SourceKeywords_of(link), value,
			lwk_c_domain_set);
		else
		    _Raise(inv_domain);

		break;

	    case _TargetKeywordsIndex :
		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_TargetKeywords_of(link), value,
			lwk_c_domain_set);
		else
		    _Raise(inv_domain);

		break;

	    case _SourceIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_Source_of(link).id, value,
			lwk_c_domain_integer);
		else if (_IsDomain(domain, lwk_c_domain_surrogate)) {
		    /*
		    **  Retrieve the Source if we only know its Id.
		    */

		    if ((_Boolean) _SetState((_Persistent) link,
			    _StateSourceIsId, _StateClear))
			RetrieveSource(link);

		    _CopyValue(&_Source_of(link).surrogate, value,
			lwk_c_domain_surrogate);
		}
		else
		    _Raise(inv_domain);

		break;

	    case _TargetIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_Target_of(link).id, value,
			lwk_c_domain_integer);
		else if (_IsDomain(domain, lwk_c_domain_surrogate)) {
		    /*
		    **  Retrieve the Target if we only know its Id.
		    */

		    if ((_Boolean) _SetState((_Persistent) link,
			    _StateTargetIsId, _StateClear))
			RetrieveTarget(link);

		    _CopyValue(&_Target_of(link).surrogate, value,
			lwk_c_domain_surrogate);
		}
		else
		    _Raise(inv_domain);

		break;

	    case _LinknetIndex :
		if (_IsDomain(domain, lwk_c_domain_linknet))
		    _CopyValue(&_Linknet_of(link), value,
			lwk_c_domain_surrogate);
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


_List  LwkOpLinkGetValueList(link, property)
_Link link;
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
	return (_List) _GetValueList_S(link, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _RelationshipTypeIndex :
		_ListValue(&_RelationshipType_of(link), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _SourceDescriptionIndex :
		_ListValue(&_SourceDescription_of(link), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _TargetDescriptionIndex :
		_ListValue(&_TargetDescription_of(link), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _SourceKeywordsIndex :
		_CopyValue(&_SourceKeywords_of(link), &value_list,
		    lwk_c_domain_set);
		break;

	    case _TargetKeywordsIndex :
		_CopyValue(&_TargetKeywords_of(link), &value_list,
		    lwk_c_domain_set);
		break;

	    case _SourceIndex :
		/*
		**  Retrieve the Source if we only know its Id.
		*/

		if ((_Boolean) _SetState((_Persistent) link,
			_StateSourceIsId, _StateClear))
		    RetrieveSource(link);

		_ListValue(&_Source_of(link).surrogate, &value_list,
		    lwk_c_domain_surrogate);
		break;

	    case _TargetIndex :
		/*
		**  Retrieve the Target if we only know its Id.
		*/

		if ((_Boolean) _SetState((_Persistent) link,
			_StateTargetIsId, _StateClear))
		    RetrieveTarget(link);

		_ListValue(&_Target_of(link).surrogate, &value_list,
		    lwk_c_domain_surrogate);
		break;

	    case _LinknetIndex :
		_ListValue(&_Linknet_of(link), &value_list,
		    lwk_c_domain_linknet);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpLinkSetValue(link, property, domain, value, flag)
_Link link;
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
    _Surrogate old_surrogate;
    _Linknet old_network;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(link, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _RelationshipTypeIndex :
		_SetSingleValuedProperty(&_RelationshipType_of(link),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _SourceDescriptionIndex :
		_SetSingleValuedProperty(&_SourceDescription_of(link),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _TargetDescriptionIndex :
		_SetSingleValuedProperty(&_TargetDescription_of(link),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _SourceKeywordsIndex :
		_SetMultiValuedProperty(&_SourceKeywords_of(link),
		    lwk_c_domain_ddif_string, domain, value, flag,
		    _False, _True);
		break;

	    case _TargetKeywordsIndex :
		_SetMultiValuedProperty(&_TargetKeywords_of(link),
		    lwk_c_domain_ddif_string, domain, value, flag,
		    _False, _True);
		break;

	    case _SourceIndex :
		/*
		**  Retrieve the Source if we only know its Id.
		*/

		if ((_Boolean) _SetState((_Persistent) link,
			_StateSourceIsId, _StateClear))
		    RetrieveSource(link);

		/*
		**  Save the old Source and then Set either the new Source, or
		**  the new Source Id.
		*/

		old_surrogate = _Source_of(link).surrogate;

		if (_IsDomain(lwk_c_domain_integer, domain)) {
		    _SetSingleValuedProperty(&_Source_of(link).id,
			lwk_c_domain_integer, domain, value, flag, _False);

		    _SetState((_Persistent) link, _StateSourceIsId,
			_StateSet);
		}
		else
		    _SetSingleValuedProperty(&_Source_of(link).surrogate,
			lwk_c_domain_surrogate, domain, value, flag, _False);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterLink property values.
		*/

		SetSource(link, old_surrogate);

		break;

	    case _TargetIndex :
		/*
		**  Retrieve the Target if we only know its Id.
		*/

		if ((_Boolean) _SetState((_Persistent) link,
			_StateTargetIsId, _StateClear))
		    RetrieveTarget(link);

		/*
		**  Save the old Target and then Set either the new Target, or
		**  the new Target Id.
		*/

		old_surrogate = _Target_of(link).surrogate;

		if (_IsDomain(lwk_c_domain_integer, domain)) {
		    _SetSingleValuedProperty(&_Target_of(link).id,
			lwk_c_domain_integer, domain, value, flag, _False);

		    _SetState((_Persistent) link, _StateTargetIsId,
			_StateSet);
		}
		else
		    _SetSingleValuedProperty(&_Target_of(link).surrogate,
			lwk_c_domain_surrogate, domain, value, flag, _False);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterLink property values.
		*/

		SetTarget(link, old_surrogate);

		break;

	    case _LinknetIndex :
		old_network = _Linknet_of(link);

		_SetSingleValuedProperty(&_Linknet_of(link),
		    lwk_c_domain_linknet, domain, value, flag, _False);

		/*
		**  Move this link from its old Network to its new
		**  Network.
		*/

		MoveLink(link, old_network);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Link Modified.
    */

    _SetState((_Persistent) link, _StateModified, _StateSet);

    return;
    }


void  LwkOpLinkSetValueList(link, property, values, flag)
_Link link;
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
    _Surrogate old_surrogate;
    _Linknet old_network;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(link, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _RelationshipTypeIndex :
		_SetSingleValuedProperty(&_RelationshipType_of(link),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, values,
		    flag, _True);
		break;

	    case _SourceDescriptionIndex :
		_SetSingleValuedProperty(&_SourceDescription_of(link),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _TargetDescriptionIndex :
		_SetSingleValuedProperty(&_TargetDescription_of(link),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _SourceKeywordsIndex :
		_SetMultiValuedProperty(&_SourceKeywords_of(link),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True, _True);
		break;

	    case _TargetKeywordsIndex :
		_SetMultiValuedProperty(&_TargetKeywords_of(link),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True, _True);
		break;

	    case _SourceIndex :
		/*
		**  Retrieve the Source if we only know its Id.
		*/

		if ((_Boolean) _SetState((_Persistent) link,
			_StateSourceIsId, _StateClear))
		    RetrieveSource(link);

		old_surrogate = _Source_of(link).surrogate;

		_SetSingleValuedProperty(&_Source_of(link).surrogate,
		    lwk_c_domain_surrogate, lwk_c_domain_list, values,
		    flag, _True);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterLink property values.
		*/

		SetSource(link, old_surrogate);

		break;

	    case _TargetIndex :
		/*
		**  Retrieve the Target if we only know its Id.
		*/

		if ((_Boolean) _SetState((_Persistent) link,
			_StateTargetIsId, _StateClear))
		    RetrieveTarget(link);

		old_surrogate = _Target_of(link).surrogate;

		_SetSingleValuedProperty(&_Target_of(link).surrogate,
		    lwk_c_domain_surrogate, lwk_c_domain_list, values,
		    flag, _True);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterLink property values.
		*/

		SetTarget(link, old_surrogate);

		break;

	    case _LinknetIndex :
		old_network = _Linknet_of(link);

		_SetSingleValuedProperty(&_Linknet_of(link),
		    lwk_c_domain_linknet, lwk_c_domain_list, values,
		    flag, _True);

		/*
		**  Move this link from its old Network to its new
		**  Network.
		*/

		MoveLink(link, old_network);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Link Modified.
    */

    _SetState((_Persistent) link, _StateModified, _StateSet);

    return;
    }


_ObjectId  LwkOpLinkGetObjectId(link)
_Link link;

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
    _Integer network_id;
    _ObjectId volatile oid;

    /*
    ** Let our supertype generate the basic ObjectId
    */
    
    oid = (_ObjectId) _GetObjectId_S(link, MyType);

    _StartExceptionBlock

    /*
    **  Find the Identifier of the Network in which the Link is stored
    */

    _GetValue(_Linknet_of(link), _P_Identifier, lwk_c_domain_integer,
	&network_id);

    /*
    **  Set the Container Identifier property of the ObjectId.
    */

    _SetValue(oid, _P_ContainerIdentifier, lwk_c_domain_integer,
	&network_id, lwk_c_set_property);

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


void  LwkOpLinkEncode(link, aggregate, handle)
_Link link;
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
    **  Begin the encoding of Link.
    */

    type = LWK_K_T_PERS_LINK;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encoding the properties of Link.
    */

    if (_RelationshipType_of(link) != (_DDIFString) _NullObject) {
	type = LWK_K_T_LINK_TYPE;
	value_length = _LengthDDIFString(_RelationshipType_of(link));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _RelationshipType_of(link), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    if (_SourceDescription_of(link) != (_DDIFString) _NullObject) {
	type = LWK_K_T_LINK_SRC_DESC;
	value_length = _LengthDDIFString(_SourceDescription_of(link));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _SourceDescription_of(link), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    EncodeSourceKeywords(link, handle);

    if (_TargetDescription_of(link) != (_DDIFString) _NullObject) {
	type = LWK_K_T_LINK_TAR_DESC;
	value_length = _LengthDDIFString(_TargetDescription_of(link));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _TargetDescription_of(link), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    EncodeTargetKeywords(link, handle);

    /*
    **  If an aggregate Link is requested, encode the Source and Target
    **	Surrogates as well.
    */

    if (aggregate) {
	if ((_Boolean) _SetState((_Persistent) link, _StateSourceIsId,
		_StateClear))
	    RetrieveSource(link);

	EncodeSurrogate(_Source_of(link).surrogate,
	    LWK_K_T_LINK_SOURCE, handle);

	if ((_Boolean) _SetState((_Persistent) link, _StateTargetIsId,
		_StateClear))
	    RetrieveTarget(link);

	EncodeSurrogate(_Target_of(link).surrogate,
	    LWK_K_T_LINK_TARGET, handle);
    }

    /*
    **  Finish the Link encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Ask our supertype to encode its properties
    */

    _Encode_S(link, aggregate, handle, MyType);

    return;
    }


void  LwkOpLinkDecode(link, handle, keys_only, properties)
_Link link;
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
    **  Begin the Connnection decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_LINK)
	_Raise(inv_encoding);

    /*
    **	Decode the Link Properties.  If extracting Key Properties, most
    **	properties can be skipped -- only Relationship Type is a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PERS_LINK :
		/*
		**  End of Link Properties.
		*/
		break;

	    case LWK_K_P_LINK_TYPE :
		_RelationshipType_of(link) =
		    _AllocateMem(value_length);

		status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		    (_DDISdata) _RelationshipType_of(link), &get_length,
		    &additional_info);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (value_length != get_length)
		    _Raise(internal_decoding_error);

		if (keys_only) {
		    _String string;
		    _Property property;

		    /*
		    **  Create a Property with the appropriate values
		    */

		    property = (_Property) _Create(_TypeProperty);

		    string = (_String) _GenericTypeKeyName;

		    _SetValue(property, _P_PropertyName, lwk_c_domain_string,
			&string, lwk_c_set_property);

		    string = _DDIFStringToString(_RelationshipType_of(link));

		    _SetValue(property, _P_Value, lwk_c_domain_string, &string,
			lwk_c_set_property);

		    _DeleteString(&string);

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

	    case LWK_K_P_LINK_SRC_DESC :
		if (!keys_only) {
		    _SourceDescription_of(link) =
			_AllocateMem(value_length);

		    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
			(_DDISdata) _SourceDescription_of(link),
			&get_length, &additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_LINK_TAR_DESC :
		if (!keys_only) {
		    _TargetDescription_of(link) =
			_AllocateMem(value_length);

		    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
			(_DDISdata) _TargetDescription_of(link),
			&get_length, &additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_LINK_SRC_KEYWORDS :
		/*
		**  Decode the Source Keyword count.
		*/

		DecodeSourceKeywords(link, handle, keys_only);

		break;

	    case LWK_K_P_LINK_TAR_KEYWORDS :
		/*
		**  Decode the Target Keyword count.
		*/

		DecodeTargetKeywords(link, handle, keys_only);

		break;

	    case LWK_K_P_LINK_SOURCE :
		/*
		**  Decode the Source
		*/

		if (!keys_only) {
		    _Source_of(link).surrogate =
			DecodeSurrogate(handle, value_length);

		    SetSource(link, (_Surrogate) _NullObject);
		}

		break;

	    case LWK_K_P_LINK_TARGET :
		/*
		**  Decode the Target
		*/

		if (!keys_only) {
		    _Target_of(link).surrogate =
			DecodeSurrogate(handle, value_length);

		    SetTarget(link, (_Surrogate) _NullObject);
		}

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_PERS_LINK);

    /*
    **  Ask our supertype to decode its properties
    */

    _Decode_S(link, handle, keys_only, properties, MyType);

    return;
    }


_Boolean  LwkOpLinkSelected(link, expression)
_Link link;
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
	    **  True if the both left and right Selection sub-Expressions are
	    **	True.
	    */

	    result = _Selected((_Persistent) link, operand_1)
		&& _Selected((_Persistent) link, operand_2);

	    break;

	case lwk_c_or :
	    /*
	    **  True if the either left and right Selection sub-Expressions is
	    **	True.
	    */

	    result = _Selected((_Persistent) link, operand_1)
		|| _Selected((_Persistent) link, operand_2);

	    break;

	case lwk_c_not :
	    /*
	    **  True if the left Selection sub-Expressions is False.
	    */

	    result = !_Selected((_Persistent) link, operand_1);

	    break;

	case lwk_c_any :
	    /*
	    **  Always True.
	    */

	    result = _True;

	    break;

	case lwk_c_identity :
	    /*
	    **  True if the left operand is this Link.
	    */

	    result = (link == (_Link) expression->lwk_operand_1);

	    break;

	case lwk_c_has_source :
	    /*
	    **	True if the left Selection sub-Expressions selects the Source
	    **	of the Link.
	    */

	    if (_Source_of(link).surrogate == (_Surrogate) _NullObject) {
		if (operand_1 == (_QueryExpression) _NullObject)
		    result = _True;
		else
		    result = _False;
	    }
	    else {
		if (operand_1->lwk_operator == lwk_c_any)
		    result = _True;
		else {
		    if ((_Boolean) _SetState((_Persistent) link,
			    _StateSourceIsId, _StateClear))
			RetrieveSource(link);

		    result = _Selected((_Persistent) _Source_of(link).surrogate,
			operand_1);
		}
	    }

	    break;

	case lwk_c_has_target :
	    /*
	    **	True if the left Selection sub-Expressions selects the Target
	    **	of the Link.
	    */

	    if (_Target_of(link).surrogate == (_Surrogate) _NullObject) {
		if (operand_1 == (_QueryExpression) _NullObject)
		    result = _True;
		else
		    result = _False;
	    }
	    else {
		if (operand_1->lwk_operator == lwk_c_any)
		    result = _True;
		else {
		    if ((_Boolean) _SetState((_Persistent) link,
			    _StateTargetIsId, _StateClear))
			RetrieveTarget(link);

		    result = _Selected((_Persistent) _Target_of(link).surrogate,
			operand_1);
		}
	    }

	    break;
	
	case lwk_c_has_properties :
	    /*
	    **  True if the properties match.
	    */

	    result = _SelectProperties((_Object) link, operand_1);

	    break;

	default :
	    _Raise(inv_query_expr);
	    break;

    }

    return result;
    }


static void  MoveLink(link, old)
_Link volatile link;
 _Linknet volatile old;

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
    **  The _Linknet property of a Link may be set only in the following
    **	cases:
    **
    **	    - The new _Linknet is the same as the old _Linknet (i.e., a NoOp)
    **	    - There was no previous _Linknet
    **	    - There are no _Source or _Target Surrogates binding the
    **	      Link in the old _Linknet
    **
    **	If these conditions are not met, the _Linknet is reset to the old
    **	value and an exception is raised.
    */

    _StartExceptionBlock

    if (old != _Linknet_of(link)) {
	if ((old == (_Linknet) _NullObject) ||
	    ((_Source_of(link).surrogate == (_Surrogate) _NullObject) &&
	     (_Target_of(link).surrogate == (_Surrogate) _NullObject))) {

	    /*
	    **	Make sure that the _Source and _Target can be moved, and if so,
	    **	do it.
	    */

	    MoveSurrogate(_Source_of(link).surrogate, _Linknet_of(link));
	    MoveSurrogate(_Target_of(link).surrogate, 	_Linknet_of(link));

	    /*
	    ** Remove this Link from the _Links property of the old
	    ** Network, if any.
	    */

	    if (old != (_Linknet) _NullObject)
		_SetValue(old , _P_Links, lwk_c_domain_link,
		    (_AnyPtr) &link, lwk_c_remove_property);

	    /*
	    **  Add this Link to the _Links property of the new
	    **  Network, if any.
	    */

	    if (_Linknet_of(link) != (_Linknet) _NullObject)
		_SetValue(_Linknet_of(link), _P_Links, lwk_c_domain_link,
		    (_AnyPtr) &link, lwk_c_add_property);
	}
	else {
	    _Raise(cannot_move_link);
	}
    }

    /*
    **  If there are any problems, reset the old _Linknet.
    */

    _Exceptions
	_WhenOthers
	    _Linknet_of(link) = old;
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  SetSource(link, old_source)
_Link volatile link;
 _Surrogate volatile old_source;

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
    _Boolean is_id;

    /*
    **  If the Source did not change, return now.
    */

    if (_Source_of(link).surrogate == old_source)
	return;

    _StartExceptionBlock

    /*
    **	Remove the Link from the _InterLinks list of the old
    **	_Source if there was one.
    */

    if (old_source != (_Surrogate) _NullObject)
	_SetValue(old_source, _P_InterLinks, lwk_c_domain_link,
	    (_AnyPtr) &link, lwk_c_remove_property);

    /*
    **  If the new _Source is not an Identifier, do some more housekeeping
    */

    is_id = (_Boolean) _SetState((_Persistent) link, _StateSourceIsId,
	_StateGet);

    if (!is_id) {
	/*
	**  Move the new _Source to the _Linknet of the Link, if there is
	**  one.
	*/

	if (_Linknet_of(link) != (_Linknet) _NullObject)
	    MoveSurrogate(_Source_of(link).surrogate, _Linknet_of(link));

	/*
	**  Add the Link to the _InterLinks list of the new
	**  _Source, if there is one.
	*/

	if (_Source_of(link).surrogate != (_Surrogate) _NullObject)
	    _SetValue(_Source_of(link).surrogate, _P_InterLinks,
		lwk_c_domain_link, (_AnyPtr) &link, lwk_c_add_property);
    }

    /*
    **  If there are any problems, reset the old _Source.
    */

    _Exceptions
	_WhenOthers
	    _Source_of(link).surrogate = old_source;
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  SetTarget(link, old_target)
_Link volatile link;

    _Surrogate volatile old_target;

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
    _Boolean is_id;

    /*
    **  If the Target did not change, return now.
    */

    if (_Target_of(link).surrogate == old_target)
	return;

    _StartExceptionBlock

    /*
    **	Remove the Link from the _InterLinks list of the old
    **	_Target if there was one.
    */

    if (old_target != (_Surrogate) _NullObject)
	_SetValue(old_target, _P_InterLinks, lwk_c_domain_link,
	    (_AnyPtr) &link, lwk_c_remove_property);

    /*
    **  If the new _Target is not an Identifier, do some more housekeeping
    */

    is_id = (_Boolean) _SetState((_Persistent) link, _StateTargetIsId,
	_StateGet);

    if (!is_id) {
	/*
	**  Move the new _Target to the _Linknet of the Link, if there is
	**  one.
	*/

	if (_Linknet_of(link) != (_Linknet) _NullObject)
	    MoveSurrogate(_Target_of(link).surrogate, 	_Linknet_of(link));

	/*
	**  Add the Link to the _InterLinks list of the new
	**  _Target, if there is one.
	*/

	if (_Target_of(link).surrogate != (_Surrogate) _NullObject)
	    _SetValue(_Target_of(link).surrogate, _P_InterLinks,
		lwk_c_domain_link, (_AnyPtr) &link, lwk_c_add_property);
    }

    /*
    **  If there are any problems, reset the old _Target.
    */

    _Exceptions
	_WhenOthers
	    _Target_of(link).surrogate = old_target;
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  RetrieveSource(link)
_Link link;

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
    _Integer id;
    _Linknet network;
    _Integer network_id;
    _Boolean conn_modified;
    _Boolean surr_modified;
    _Surrogate surrogate;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Save and clear the Source Id in case we fail to retrieve it
    */

    id = _Source_of(link).id;
    _Source_of(link).surrogate = (_Surrogate) _NullObject;

    /*
    **  Find the Repository in which the Link is stored
    */

    _GetValue((_Object) link, _P_Linkbase, lwk_c_domain_linkbase,
	&repository);

    /*
    **  Find the Identifier of the Network in which the Link is stored
    */

    _GetValue(_Linknet_of(link), _P_Identifier, lwk_c_domain_integer,
	&network_id);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Retrieve the Surrogate
    */

    surrogate = (_Surrogate) LwkLbRetrieve(repository,
	lwk_c_domain_surrogate, id, network_id);

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
    **  Preserve the Modified state of the Link and the Surrogate
    */

    conn_modified = (_Boolean) _SetState((_Persistent) link,
	_StateModified, _StateGet);

    surr_modified = (_Boolean) _SetState((_Persistent) surrogate,
	_StateModified, _StateGet);

    /*
    **  Set the Surrogate as the Source
    */

    _Source_of(link).surrogate = surrogate;

    SetSource(link, (_Surrogate) _NullObject);

    /*
    **  Restore the Modified state of the Link and the Surrogate
    */

    if (!conn_modified)
	_SetState((_Persistent) link, _StateModified, _StateClear);
    if (!surr_modified)
	_SetState((_Persistent) surrogate, _StateModified, _StateClear);

    return;
    }


static void  RetrieveTarget(link)
_Link link;

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
    _Integer id;
    _Linknet network;
    _Integer network_id;
    _Boolean conn_modified;
    _Boolean surr_modified;
    _Surrogate surrogate;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Save and clear the Target Id in case we fail to retrieve it
    */

    id = _Target_of(link).id;
    _Target_of(link).surrogate = (_Surrogate) surrogate;

    /*
    **  Find the Repository in which the Link is stored
    */

    _GetValue((_Object) link, _P_Linkbase, lwk_c_domain_linkbase,
	&repository);

    /*
    **  Find the Identifier of the Network in which the Link is stored
    */

    _GetValue(_Linknet_of(link), _P_Identifier, lwk_c_domain_integer,
	&network_id);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Retrieve the Surrogate
    */

    surrogate = (_Surrogate) LwkLbRetrieve(repository,
	lwk_c_domain_surrogate, id, network_id);

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
    **  Preserve the Modified state of the Link and the Surrogate
    */

    conn_modified = (_Boolean) _SetState((_Persistent) link,
	_StateModified, _StateGet);

    surr_modified = (_Boolean) _SetState((_Persistent) surrogate,
	_StateModified, _StateGet);

    /*
    **  Set the Surrogate as the Target
    */

    _Target_of(link).surrogate = surrogate;

    SetTarget(link, (_Surrogate) _NullObject);

    /*
    **  Restore the Modified state of the Link and the Surrogate
    */

    if (!conn_modified)
	_SetState((_Persistent) link, _StateModified, _StateClear);
    if (!surr_modified)
	_SetState((_Persistent) surrogate, _StateModified, _StateClear);

    return;
    }


static void  MoveSurrogate(surrogate, network)
_Surrogate surrogate;
 _Linknet network;

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
    ** Set the _Container of the new Surrogate to the _Linknet of the
    ** Link.
    */

    if (surrogate != (_Surrogate) _NullObject)
	_SetValue(surrogate, _P_Container, lwk_c_domain_linknet,
	    &network, lwk_c_set_property);

    return;
    }


static void  EncodeSourceKeywords(link, handle)
_Link link;
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
    _Integer count;
    _DDIStype type;
    _DDISlength value_length;

    if (_SourceKeywords_of(link) != (_Set) _NullObject) {
	_GetValue(_SourceKeywords_of(link), _P_ElementCount,
	    lwk_c_domain_integer, &count);

	if (count > 0) {
	    /*
	    **  Begin the Source Keywords Set.
	    */

	    type = LWK_K_T_LINK_SRC_KEYWORDS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of the Source Keyword Set.
	    */

	    type = LWK_K_T_LINK_SRC_KEY_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length,
		(_DDISdata) &count, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Source Keywords Set.
	    */

	    type = LWK_K_T_LINK_SRC_WORDS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Keywords Set encoding each Keyword.
	    */

	    _Iterate(_SourceKeywords_of(link), lwk_c_domain_ddif_string,
		(_Closure) handle, EncodeKeyword);

	    /*
	    **  End the Source Keywords Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  End the Source Keywords Property.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
	}
    }

    return;
    }


static void  EncodeTargetKeywords(link, handle)
_Link link;
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
    _Integer count;
    _DDIStype type;
    _DDISlength value_length;

    if (_TargetKeywords_of(link) != (_Set) _NullObject) {
	_GetValue(_TargetKeywords_of(link), _P_ElementCount,
	    lwk_c_domain_integer, &count);

	if (count > 0) {
	    /*
	    **  Begin the Target Keywords Set.
	    */

	    type = LWK_K_T_LINK_TAR_KEYWORDS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of the Target Keyword Set.
	    */

	    type = LWK_K_T_LINK_TAR_KEY_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Source Keywords Set.
	    */

	    type = LWK_K_T_LINK_TAR_WORDS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Keywords Set encoding each Keyword.
	    */

	    _Iterate(_TargetKeywords_of(link), lwk_c_domain_ddif_string,
		(_Closure) handle, EncodeKeyword);

	    /*
	    **  End the Target Keywords Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  End the Target Keywords Property.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);


	    if (!_Success(status))
		_Raise(internal_encoding_error);
	}
    }

    return;
    }


static _Termination _EntryPt  EncodeKeyword(closure, keywords, domain, keyword)
_Closure closure;
 _Set keywords;

    _Domain domain;
 _DDIFString *keyword;

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
    _DDIShandle handle;
    _DDISlength value_length;

    handle = (_DDIShandle) closure;

    /*
    **  Encode the Keyword.
    */
    
    type = DDIS_K_T_OCTET_STRING;
    value_length = _LengthDDIFString(*keyword);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) *keyword,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Continue the iteration.
    */

    return (_Termination) 0;
    }


static void  EncodeSurrogate(surrogate, type, handle)
_Surrogate surrogate;
 _DDIStype type;

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
    _DDISlength size;
    _VaryingString encoding;

    if (surrogate != (_Surrogate) _NullObject) {
	/*
	**  Export the Surrogate.
	*/

	size = _Export(surrogate, _False, &encoding);

	/*
	**  Encode the Source Surrogate.
	*/

	status = ddis_put(&handle, &type, &size, (_DDISdata) encoding,
	    (_DDISconstantPtr) 0);

	_DeleteEncoding(&encoding);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    return;
    }


static void  DecodeSourceKeywords(link, handle, keys_only)
_Link link;
 _DDIShandle handle;

    _Boolean keys_only;

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
    int i;
    _DDISstatus status;
    _Integer integer;
    _Integer count;
    _DDIFString ddifstring;
    _DDIStable  table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Get Source Keyword count
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LINK_SRC_KEY_COUNT)
	_Raise(inv_encoding);

    value_length = sizeof(_Integer);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, &cnt);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    count = (_Integer) cnt;

    /*
    **  Begin decoding the Keywords
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LINK_SRC_WORDS)
	_Raise(inv_encoding);

    /*
    **  Decode count Source Keywords, or just skip them if extracting Key
    **	Properties.
    */

    if (!keys_only)
	_SourceKeywords_of(link) =
	    (_Set) _CreateSet(_TypeSet, lwk_c_domain_ddif_string, count);

    for (i = 0; i < count; i++) {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_OCTET_STRING)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    ddifstring = _AllocateMem(value_length);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) ddifstring, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    _AddElement(_SourceKeywords_of(link),
		lwk_c_domain_ddif_string, &ddifstring, _False);
	}
    }

    /*
    **  Finish decoding the Source Keywords Set.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LINK_SRC_WORDS)
	_Raise(inv_encoding);

    /*
    **  Finish decoding the Source Keywords.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LINK_SRC_KEYWORDS)
	_Raise(inv_encoding);

    return;
    }


static void  DecodeTargetKeywords(link, handle, keys_only)
_Link link;
 _DDIShandle handle;

    _Boolean keys_only;

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
    int i;
    _DDISstatus status;
    _Integer integer;
    _Integer count;
    _DDIFString ddifstring;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Get Target Keyword count
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LINK_TAR_KEY_COUNT)
	_Raise(inv_encoding);

    value_length = sizeof(_Integer);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, &cnt);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    count = (_Integer) cnt;

    /*
    **  Begin decoding the Target Keywords.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LINK_TAR_WORDS)
	_Raise(inv_encoding);

    /*
    **  Decode count Target Keywords, or just skip them if extracting Key
    **	Properties.
    */

    if (!keys_only)
	_TargetKeywords_of(link) =
	    (_Set) _CreateSet(_TypeSet, lwk_c_domain_ddif_string, count);

    for (i = 0; i < count; i++) {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_OCTET_STRING)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    ddifstring = _AllocateMem(value_length);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) ddifstring, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    _AddElement(_TargetKeywords_of(link),
		lwk_c_domain_ddif_string, &ddifstring, _False);
	}
    }

    /*
    **  Finish decoding the Target Keywords Set.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LINK_TAR_WORDS)
	_Raise(inv_encoding);

    /*
    **  Finish decoding the Target Keywords.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LINK_TAR_KEYWORDS)
	_Raise(inv_encoding);

    return;
    }

static _Surrogate  DecodeSurrogate(handle, value_length)
_DDIShandle handle;
 _DDISlength value_length;

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
    _Surrogate surrogate;
    _VaryingString volatile encoding;
    _DDISlength get_length;
    _DDISconstant additional_info;

    encoding = (_VaryingString) _NullObject;

    _StartExceptionBlock

    encoding = (_VaryingString) _AllocateMem(value_length);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) encoding, &get_length, &additional_info);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (value_length != get_length)
	_Raise(internal_decoding_error);

    /*
    **  Import the Source Surrogate from the encoding.
    */

    surrogate = (_Surrogate) _Import(_TypeSurrogate, encoding);

    _DeleteEncoding(&encoding);

    _Exceptions
	_WhenOthers
	    _DeleteEncoding(&encoding);
	    _Reraise;
    _EndExceptionBlock

    return surrogate;
    }
