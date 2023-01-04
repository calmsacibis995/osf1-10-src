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
** COPYRIGHT (c) 1989, 1991 BY
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
**	Environment Context object methods
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
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_envcontext.h"
#include "hs_decwindows.h"

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

/*
**  Retrieve support routines
*/

#define _OpenDefaultLinkbase(Linkbase, create) \
    EnvOpenDefaultLinkbase((Linkbase), (create))
#define _RetrieveEnvironment(EnvCtxt, Linkbase) \
    EnvRetrieveEnvironment((EnvCtxt), (Linkbase))

_DeclareFunction(_Boolean EnvOpenDefaultLinkbase,
    (lwk_linkbase *linkbase, _Boolean create));
_DeclareFunction(_Void EnvRetrieveEnvironment,
    (_EnvContext env_context, lwk_linkbase linkbase));

/*
**  Type Definitions
*/

typedef struct __IterateActivePath {
	    lwk_list		    list;
	    lwk_boolean		    good_sequence;
	    lwk_object		    previous_obj;
	    lwk_domain		    domain_prev_obj;
	    lwk_object		    end_seq_obj;
	} _IterateActivePathInstance, *_IterateActivePath;
	
/*
**  Global Data Definitions
*/

_Global _EnvContextTypeInstance;      /* Instance record for Type */
_Global _EnvContextType;              /* Type */

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeEnvContextInstance;
static _EnvContextPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    EnvContextPropertyNameTable;

/*
**  External function declaration
*/

_DeclareFunction(_Void EnvSvnWindowCollectObjectList,
    (lwk_list list, _AnyPtr head));

/*
** Forward routine declaration
*/

_DeclareFunction(static _Void RecreateActivePaths,
    (_EnvContext env_context));
_DeclareFunction(static lwk_termination EnvCxtPathIndex,
    (_IterateActivePath iterate_data, lwk_list active_index_list,
    lwk_domain domain, lwk_object_descriptor *object_desc));
_DeclareFunction(static lwk_termination SearchPathCtxt,
    (lwk_list list, lwk_object cpath, lwk_domain domain,
    lwk_object_descriptor *object_desc));


_Void  EnvOpEnvCtxt(env_ctxt)
_EnvContext env_ctxt;

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
    

_EnvContext  EnvOpEnvCtxtRetrieve(type, new_ctxt, create_lb)
_Type type;
 _Boolean *new_ctxt;

    _Boolean create_lb;

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
    _EnvContext		env_context;
    _Boolean		new_linkbase;
    lwk_linkbase	linkbase;

    env_context = (_EnvContext) 0;

    _StartExceptionBlock

    /*
    **  Open LinkWorks Manager's private linkbase
    */
    
    *new_ctxt = _OpenDefaultLinkbase(&linkbase, create_lb);

    /*
    ** Create the EnvContext Object
    */
    
    env_context = (_EnvContext) _Create(MyType);

    _HsLinkbase_of(env_context) = linkbase;

    /*
    **  Retrieve the various private objects from the linkbase
    */

    _RetrieveEnvironment(env_context, linkbase);

    /*
    **  If any exceptions are raised display an error message
    */

    _Exceptions
	_When(version_error)
	    _Reraise;

        _WhenOthers
	    _Status status[2];

	    if (env_context != _NullObject) {
		_Free(env_context);
		env_context = _NullObject;
	    }

	    status[0] = _StatusCode(retrieve_error);
	    status[1] = _Others;

	    _DisplayRootMessage(status, 2, hs_c_fatal_message);

    _EndExceptionBlock

    /*
    ** Return the object to the caller.
    */

    return env_context;
    }
    

_Void  EnvOpEnvCtxtInitialize(env_ctxt, proto)
_EnvContext env_ctxt;
 _EnvContext proto;

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

    _ClearValue(&_Networks_of(env_ctxt), hs_c_domain_hsobject);
    _ClearValue(&_ActiveNetworks_of(env_ctxt), hs_c_domain_hsobject);
    _ClearValue(&_CurrentNetwork_of(env_ctxt), hs_c_domain_hsobject);
    _ClearValue(&_Paths_of(env_ctxt), hs_c_domain_hsobject);
    _ClearValue(&_ActivePathIndex_of(env_ctxt), hs_c_domain_hsobject);
    _ClearValue(&_ActivePaths_of(env_ctxt), hs_c_domain_hsobject);
    _ClearValue(&_Trail_of(env_ctxt), hs_c_domain_hsobject);
    _ClearValue(&_EnvWindow_of(env_ctxt), hs_c_domain_environment_wnd);
    _ClearValue(&_Attributes_of(env_ctxt), hs_c_domain_hsobject);

    _ClearValue(&_HsLinkbase_of(env_ctxt), hs_c_domain_hsobject);

    /*
    **  Invoke the Initialize operation in our supertype
    */
    
    _Initialize_S(env_ctxt, proto, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.  We don't support
    **	this type of copy.
    */

    if (proto != (_EnvContext) _NullObject) {
	_Raise(inv_operation);
    }
    
    return;
    }
    

_Void  EnvOpEnvCtxtFree(env_ctxt)
_EnvContext env_ctxt;

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
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_Networks_of(env_ctxt), hs_c_domain_hsobject);
    _DeleteValue(&_ActiveNetworks_of(env_ctxt), hs_c_domain_hsobject);
    _DeleteValue(&_CurrentNetwork_of(env_ctxt), hs_c_domain_hsobject);
    _DeleteValue(&_Paths_of(env_ctxt), hs_c_domain_hsobject);
    _DeleteValue(&_ActivePathIndex_of(env_ctxt), hs_c_domain_hsobject);
    _DeleteValue(&_ActivePaths_of(env_ctxt), hs_c_domain_hsobject);
    _DeleteValue(&_Trail_of(env_ctxt), hs_c_domain_hsobject);
    _DeleteValue(&_EnvWindow_of(env_ctxt), hs_c_domain_environment_wnd);
    _DeleteValue(&_Attributes_of(env_ctxt), hs_c_domain_hsobject);
    
    _DeleteValue(&_HsLinkbase_of(env_ctxt), hs_c_domain_hsobject);

    /*
    **  Ask our supertype to free its properties
    */
    
    _Free_S(env_ctxt, MyType);

    return;
    }

_Void  EnvOpEnvCtxtGetValue(env_ctxt, property, domain, value)
_EnvContext env_ctxt;
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
    **  See if the property is defined by our type.  If not, pass the request
    **  on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
        _GetValue_S(env_ctxt, property, domain, value, MyType);
    }

    else {

        /*
        **  Answer depends on the property
        */

        switch (index) {

            case _NetworksIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_Networks_of(env_ctxt), value,
                        hs_c_domain_hsobject);
                else
                    _Raise(inv_domain);
                break;
	
            case _ActiveNetworksIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_ActiveNetworks_of(env_ctxt), value,
                        hs_c_domain_hsobject);
                else
                    _Raise(inv_domain);
                break;
	
            case _CurrentNetworkIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_CurrentNetwork_of(env_ctxt), value,
                        hs_c_domain_hsobject);
                else
                    _Raise(inv_domain);
                break;
	
            case _PathsIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_Paths_of(env_ctxt), value,
                        hs_c_domain_hsobject);
                else
                    _Raise(inv_domain);
                break;
	
            case _ActivePathIndexIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_ActivePathIndex_of(env_ctxt), value,
                        hs_c_domain_hsobject);
                else
                    _Raise(inv_domain);
                break;
	
            case _ActivePathsIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_ActivePaths_of(env_ctxt), value,
                        hs_c_domain_hsobject);
                else
                    _Raise(inv_domain);
                break;
	
            case _TrailIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_Trail_of(env_ctxt), value,
                        hs_c_domain_hsobject);
                else
                    _Raise(inv_domain);
                break;
	
            case _AttributesIndex :
                if (_IsDomain(hs_c_domain_environment_wnd, domain))
                    _CopyValue(&_Attributes_of(env_ctxt), value,
                        hs_c_domain_environment_wnd);
                else
                    _Raise(inv_domain);
                break;
	
            case _EnvWindowIndex :
                if (_IsDomain(hs_c_domain_lwk_object, domain))
                    _CopyValue(&_EnvWindow_of(env_ctxt), value,
                        hs_c_domain_lwk_object);
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


_Void  EnvOpEnvCtxtSetValue(env_ctxt, property, domain, value, flag)
_EnvContext env_ctxt;
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
    **  See if the property is defined by our type.  If not, pass the request
    **  on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
        _SetValue_S(env_ctxt, property, domain, value, flag, MyType);
    }

    else {

        /*
        **  Answer depends on the property
        */

        switch (index) {

            case _NetworksIndex :
		_SetSingleValuedProperty(&_Networks_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
	
            case _ActiveNetworksIndex :
		_SetSingleValuedProperty(&_ActiveNetworks_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
	
            case _CurrentNetworkIndex :
		_SetSingleValuedProperty(&_CurrentNetwork_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
	
            case _PathsIndex :
		_SetSingleValuedProperty(&_Paths_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
	
            case _ActivePathIndexIndex :
		_SetSingleValuedProperty(&_ActivePathIndex_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                /*
		** Build the active paths list
		*/
		RecreateActivePaths(env_ctxt);
		
                break;
	
            case _ActivePathsIndex :
		_SetSingleValuedProperty(&_ActivePaths_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
	
            case _TrailIndex :
		_SetSingleValuedProperty(&_Trail_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
	
            case _AttributesIndex :
		_SetSingleValuedProperty(&_Attributes_of(env_ctxt),
		    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
	
            case _EnvWindowIndex :
		_SetSingleValuedProperty(&_EnvWindow_of(env_ctxt),
		    hs_c_domain_environment_wnd, domain, value, flag, _False);
                break;
	
            default :
                _Raise(no_such_property);
                break;
        }
    }

    return;
    }
    

_Void  EnvOpEnvCtxtSetContextCurrency(env_ctxt, currency, list, hs_object)
_EnvContext env_ctxt;
 _CurrencyFlag currency;

     lwk_list list;
 _HsObject hs_object;

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
    _String	    property;
    _Integer	    list_count;
    lwk_domain	    domain;
    lwk_status	    status;
    lwk_persistent  current_obj;
    lwk_persistent  composite;

    current_obj = lwk_c_null_object;
    list_count = 0;

    /*
    **  Get the number of elements in the list. If null the current composite of
    **	type will also be set to null
    */
    
    if (list != lwk_c_null_object) 
	status = lwk_get_value(list, lwk_c_p_element_count,
	    lwk_c_domain_integer, &list_count);

    switch (currency) {

	case lwk_c_env_recording_linknet :
	
	    _GetValue(_CurrentNetwork_of(env_ctxt), _P_HisObject,
		hs_c_domain_lwk_object, &composite);
	    property = lwk_c_p_linknets;
	    domain = lwk_c_domain_set;
	    if (hs_object != _NullObject)
		_GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object,
		    &current_obj);
	    break;

	case lwk_c_env_recording_path :
	
	    _GetValue(_Trail_of(env_ctxt), _P_HisObject, hs_c_domain_lwk_object,
		&composite);
	    property = lwk_c_p_paths;
	    domain = lwk_c_domain_list;
	    if (hs_object != _NullObject)
		_GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object,
		    &current_obj);
	    break;

	case lwk_c_env_active_comp_linknet :
	
	    _GetValue(_ActiveNetworks_of(env_ctxt), _P_HisObject,
		hs_c_domain_lwk_object, &composite);
	    property = lwk_c_p_linknets;
	    domain = lwk_c_domain_set;
	    if (list_count > 0)
		current_obj = composite;
	    break;

	case lwk_c_env_active_comp_path :

            /*
	    ** Update the active paths index property on the environment context
	    */
	    _GetValue(_ActivePathIndex_of(env_ctxt), _P_HisObject,
		hs_c_domain_lwk_object, &composite);
	    property = lwk_c_p_paths;
	    domain = lwk_c_domain_list;
	    break;

	default :
	    _Raise(inv_currency);
	    break;
    }

    /*
    **  Update the composite object with the new list
    */
    status = lwk_set_value(composite, property, domain,
	(lwk_any_pointer) &list, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed); /* his set value failed */
    }

    /*
    **  Update LinkWorks Manager linkbase
    */

    status = lwk_store(composite, _HsLinkbase_of(env_ctxt));

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed); /* store operation failed */
    }

    /*
    **  Set the currency
    */
    if (currency == lwk_c_env_active_comp_path) {
    
	/*
	** Recreate the active path list
	*/
	RecreateActivePaths(env_ctxt);

	if (list_count > 0) {
	    _GetValue(_ActivePaths_of(env_ctxt), _P_HisObject,
		hs_c_domain_lwk_object, &composite);
	    current_obj = composite;
	}
    }

    _SetCurrency(_EnvWindow_of(env_ctxt), currency, (_AnyPtr) &current_obj);
    
    return;
    }
    

_Void  EnvOpEnvCtxtSaveList(env_ctxt, domain, head)
_EnvContext env_ctxt;
 lwk_domain domain;
 _AnyPtr head;

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
    _HsObject	    list_obj;
    _String	    property;
    lwk_status	    status;
    lwk_list	    list;
    lwk_persistent  composite;

    switch (domain) {

	/*
	**  Prepare for updating the property
	*/
	
	case lwk_c_domain_comp_linknet :
	case lwk_c_domain_linknet :
	
	    _GetValue(env_ctxt, _P_Networks, hs_c_domain_hsobject, &list_obj);
	    
	    status = lwk_create_set(lwk_c_domain_object_desc, 20, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed); /* his set creation failed */
	    }

	    property = lwk_c_p_linknets;
	    domain = lwk_c_domain_set;
	    break;

	case lwk_c_domain_comp_path :
	case lwk_c_domain_path :

	    _GetValue(env_ctxt, _P_Paths, hs_c_domain_hsobject, &list_obj);

	    status = lwk_create_list(lwk_c_domain_object_desc, 20, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed); /* his list creation failed */
	    }

	    property = lwk_c_p_paths;
	    domain = lwk_c_domain_list;
	    break;
    
	default :
	    _Raise(inv_domain);
	    break;
    }

    /*
    **  Fill in the list with the object descriptors
    */
    
    EnvSvnWindowCollectObjectList(list, head);

    /*
    **  Update the property on the composite object
    */
    
    _GetValue(list_obj, _P_HisObject, hs_c_domain_lwk_object, &composite);

    status = lwk_set_value(composite, property, domain, &list,
	lwk_c_set_property);
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }

    /*
    **  Update the composite object in LinkWorks Manager's linkbase
    */

    status = lwk_store(composite, _HsLinkbase_of(env_ctxt));
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed);
    }
    
    return;
    }
    

_Void  EnvOpEnvCtxtSetAttribute(env_ctxt, attribute, value)
_EnvContext env_ctxt;
 _Attribute attribute;

    _AnyPtr *value;

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
    _String	    property;
    lwk_persistent  attr_cnet;
    lwk_status	    status;
    lwk_domain	    domain;

    _GetValue(_Attributes_of(env_ctxt), _P_HisObject, hs_c_domain_lwk_object,
	&attr_cnet);

    property = _AttributeToProperty(attribute);

    domain = (lwk_domain) _AttributeToDomain(attribute);

    /*
    **  If the value is null (not a pointer to null!), the property will not be
    **	set.
    */
    
    if (value != _NullObject) {
    
	status = lwk_set_value(attr_cnet, property, domain, value,
	    lwk_c_set_property);
    }
    else {
    
	status = lwk_set_value(attr_cnet, property, domain, value,
	    lwk_c_delete_property);
    }
	    
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }
	
    /*
    **  Set the environment attribute current if it is an environment
    **  attribute 
    */

    if (_IsEnvironmentAttribute(attribute)) 
	_SetAttributeCurrent(env_ctxt, attribute, value);
    
    return;
    }
    

_Void  EnvOpEnvCtxtSetAttributeCurrent(env_context, attribute, value)
_EnvContext env_context;

    _Attribute attribute;
 _AnyPtr *value;

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
    lwk_any_pointer null_value = lwk_c_null_object;
    
    if (!_IsEnvironmentAttribute(attribute))
	return;

    /*
    **  Make value be a pointer to the null value for setting the currency
    */
    
    if (value == (_AnyPtr *) 0)
	value = (_AnyPtr *) &null_value;

    switch (attribute) {

	case _ConnectionTypeAttr :
	    _SetCurrency(_EnvWindow_of(env_context),
		lwk_c_env_default_relationship, value);
	    break;
	    
	case _HighlightingAttr :
	    _SetCurrency(_EnvWindow_of(env_context),
		lwk_c_env_default_highlight, value);
	    break;
	    
	case _OperationAttr :
	    _SetCurrency(_EnvWindow_of(env_context),
		lwk_c_env_default_operation, value);
	    break;
	    
	case _RetainSourceAttr :
	    _SetCurrency(_EnvWindow_of(env_context),
		lwk_c_env_default_retain_source, value);
	    break;
	    
    }

    return;
    }


_Void  EnvOpEnvCtxtGetAttribute(env_ctxt, attribute, value)
_EnvContext env_ctxt;
 _Attribute attribute;

    _AnyPtr *value;

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
    _String	    property;
    lwk_persistent  attr_cnet;
    lwk_status	    status;
    lwk_domain	    domain;

    _GetValue(_Attributes_of(env_ctxt), _P_HisObject, hs_c_domain_lwk_object,
	&attr_cnet);
	
    property = _AttributeToProperty(attribute);

    domain = (lwk_domain) _AttributeToDomain(attribute);

    status = lwk_get_value(attr_cnet, property, domain, value);

    if (status == lwk_s_no_such_property)
	*value = _NullObject;
    else
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed);
	}
		    
    return;
    }
    

_Void  EnvOpEnvCtxtSaveAttributes(env_ctxt)
_EnvContext env_ctxt;

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
    lwk_persistent  attr_cnet;
    lwk_status	    status;

    _GetValue(_Attributes_of(env_ctxt), _P_HisObject, hs_c_domain_lwk_object,
	&attr_cnet);

    /*
    **  Store the current Attribute object. If anything has changed it will be
    **	stored.
    */

    status = lwk_store(attr_cnet, _HsLinkbase_of(env_ctxt));

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed); /* store failed */
    }
	
    return;
    }
    

_Void  EnvOpEnvCtxtResetAttributes(env_ctxt)
_EnvContext env_ctxt;

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
    _Integer		    attribute;
    _AnyPtr		    value;
    lwk_persistent	    attr_cnet;
    lwk_object_descriptor   obj_dsc;
    lwk_status		    status;

    _GetValue(_Attributes_of(env_ctxt), _P_HisObject, hs_c_domain_lwk_object,
	&attr_cnet);

    /*
    **  Get the object descriptor for the cnet so we can retrieve it later
    */
    
    status = lwk_get_object_descriptor(attr_cnet, &obj_dsc);
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_objdsc_failed);
    }

    /*
    **  Delete the in memory copy of the cnet
    */
        
    status = lwk_delete(&attr_cnet);
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(persobj_deletion_failed);
    }

    /*
    **  Retrieve the Attribute cnet as it was last saved
    */
        	
    status = lwk_retrieve(obj_dsc, &attr_cnet);
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(object_retrieve_failed);
    }

    /*
    **  Reload the hs_object
    */

    _SetValue(_Attributes_of(env_ctxt), _P_HisObject, hs_c_domain_lwk_object,
	&attr_cnet, hs_c_set_property);

    /*
    **  Reset all the attributes as they were last saved
    */

    for (attribute = (_Integer) _FirstAttribute;
	 attribute < (_Integer) _LastAttribute;
	 attribute++) {

	_GetAttribute(env_ctxt, attribute, &value);

	_SetAttributeCurrent(env_ctxt, attribute, &value);
    }
    
    return;
    }
    

_Void  EnvOpEnvCtxtResetDefaultAttr(env_ctxt)
_EnvContext env_ctxt;

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
    _Integer	    attribute;
    _AnyPtr	    value;
    lwk_persistent  attr_cnet;

    _GetValue(_Attributes_of(env_ctxt), _P_HisObject, hs_c_domain_lwk_object,
	&attr_cnet);

    /*
    **  We reset all the attributes with the system default values
    */

    for (attribute = (_Integer) _FirstAttribute;
         attribute < (_Integer) _LastAttribute; attribute++) {

	value = (_AnyPtr) EnvAttrSetDefAttribute(attr_cnet, attribute);

	/*
	**  Set the environment attribute current if it is an environment
	**  attribute 
	*/

	if (_IsEnvironmentAttribute(attribute))
	    _SetAttributeCurrent(env_ctxt, attribute, value);
    }

    return;
    }
    
    
_Void  EnvOpEnvCtxtLoadDefaultAttr(env_context)
_EnvContext env_context;

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
    _Integer	    attribute;
    lwk_persistent  attr_cnet;
    lwk_status	    status;
    
    _GetValue(_Attributes_of(env_context), _P_HisObject, hs_c_domain_lwk_object,
	&attr_cnet);

    for (attribute = (_Integer) _FirstAttribute;
	 attribute < (_Integer) _LastAttribute;
	attribute++) {

	EnvAttrSetDefAttribute(attr_cnet, attribute);
    }

    status = lwk_store(attr_cnet, _HsLinkbase_of(env_context));

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed); /* store failed */
    }

    return;
    }

    
static _Void  RecreateActivePaths(env_context)
_EnvContext env_context;

/*
**++
**  Functional Description:
**	Recreate the active path list from the active path index property on the
**	environment context and store it in the linkbase.
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
    lwk_status		    status;
    lwk_composite_path	    active_index_cpath;
    lwk_composite_path      active_paths;
    lwk_list		    active_index_list;
    lwk_list		    list;
    lwk_integer		    count;
    lwk_termination	    return_value;
    _IterateActivePath	    iterate_data;

    /*
    ** Get the active path index property from the environment context
    */

    _GetValue(_ActivePathIndex_of(env_context), _P_HisObject,
	hs_c_domain_lwk_object, &active_index_cpath);

    /*
    ** Get the active paths from the environment context
    */
    
    _GetValue(_ActivePaths_of(env_context), _P_HisObject,
	hs_c_domain_lwk_object, &active_paths);

    /*
    ** Get the active index list and get the number of objects. If there are no
    ** elements, the current composite path contains an empty list, else it is
    ** rebuilt from the active index list.
    */

    status = lwk_get_value(active_index_cpath, lwk_c_p_paths, lwk_c_domain_list,
		&active_index_list);
		
    status = lwk_create_list(lwk_c_domain_object_desc, 10, &list);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(create_list_failed); /* his create list failed */
    }

    if (active_index_list != lwk_c_null_object) {

	status = lwk_get_value(active_index_list, lwk_c_p_element_count,
	    lwk_c_domain_integer, &count);
	
	if (count > 0) {

            /*
	    ** Allocate memory for the iterate data structure
	    */
	    iterate_data = (_IterateActivePath)
		_AllocateMem(sizeof(_IterateActivePathInstance));
	    _ClearMem(iterate_data, sizeof(_IterateActivePathInstance));

	    iterate_data->list = list;
	    iterate_data->good_sequence = _True;
	    iterate_data->previous_obj = (lwk_object) 0;
	    iterate_data->domain_prev_obj = (lwk_domain) 0;
	    iterate_data->end_seq_obj = active_index_cpath;
	    
	    status = lwk_iterate(active_index_list, lwk_c_domain_object_desc,
		iterate_data, EnvCxtPathIndex, &return_value);

	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_FreeMem(iterate_data);
		_Raise(iterate_failed); /* Iterate on environment list failed */
	    }
	    
	    _FreeMem(iterate_data);
	}
	    
    }

    status = lwk_set_value(active_paths, lwk_c_p_paths, lwk_c_domain_list,
	&list, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed); /* Set of private property failed */
    }

    /*
    ** Update the LinkWorks Manager linkbase
    */

    status = lwk_store(active_paths, _HsLinkbase_of(env_context));

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed); /* store operation failed */
    }

    return;
    }
    

static lwk_termination  EnvCxtPathIndex(iterate_data, active_index_list, domain, object_desc)
_IterateActivePath iterate_data;

    lwk_list active_index_list;
 lwk_domain domain;

    lwk_object_descriptor *object_desc;

/*
**++
**  Functional Description:
**	During the iteration we always keep track of the previous object
**	visited. When we are on the end sequence object descriptor (the object
**	descriptor representing the active path index composite path), we are
**	at the end of a sequence.
**	If the previous visited object is a path, it is included in the
**	active path list, else (it's a path list) we have to find all the paths
**	it contains and include them in the active path list.
**	The good_sequence boolean helps us to parse only sequence of retrievable
**	objects, else we go until the next sequence.
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
    lwk_persistent	    persistent_object;
    lwk_domain		    persistent_domain;
    lwk_object_descriptor   obj_desc;
    lwk_status		    status;
    lwk_termination	    return_value;

    /*
    ** Retrieve the persistent object.
    */
    status = lwk_retrieve(*object_desc, &persistent_object);

    if (status != lwk_s_success) {

        /*
	** The object cannot be retrieved, so this is a bad sequence.
	*/
	iterate_data->good_sequence = _False;
	iterate_data->previous_obj = (lwk_object) 0;
	iterate_data->domain_prev_obj = (lwk_domain) 0;
    }

    else {
    
        /*
	** Check if you are at the end of a sequence.
	*/

	if (persistent_object == iterate_data->end_seq_obj) {

            /*
	    ** If the sequence if a good one treat the previous object
	    */
	    if (iterate_data->good_sequence == _True) {

		/*
		** the previous object must be inserted in the active path list.
		** If it's a path, insert it; else retrieve all its paths.
		*/
		if (iterate_data->domain_prev_obj == lwk_c_domain_path) {
		    
		    status = lwk_get_object_descriptor(
			iterate_data->previous_obj, &obj_desc);
						 
		    if (status != lwk_s_success) {
			_SaveHisStatus(status);
			_Raise(get_objdsc_failed);
		    }

		    status = lwk_add_element(iterate_data->list,
			lwk_c_domain_object_desc, &obj_desc);
				    
		    if (status != lwk_s_success) {
			_SaveHisStatus(status);
			_Raise(add_ele_failed); /* add element to list failed */
		    }
		}

		else
		    if (iterate_data->domain_prev_obj ==
				lwk_c_domain_comp_path) {

			/*
			** Find all the objects containing in the path list,
			** add the paths to the list and propagate if a
			** path list.
			*/
			
			status = lwk_iterate(iterate_data->previous_obj,
				lwk_c_domain_object_desc,
				iterate_data->list,
				SearchPathCtxt, &return_value);

			if (status != lwk_s_success) 
			    _Raise(iterate_failed);
		    }
	    }

	    else 

                /*
		** We are at the end of a bad sequence, prepare for a new good
		** sequence
		*/
		iterate_data->good_sequence == _True;

            /*
	    ** Prepare for a new sequence.
	    */
	    
	    iterate_data->previous_obj = (lwk_object) 0;
	    iterate_data->domain_prev_obj = (lwk_domain) 0;

	}

	else {

            /*
	    ** We are not at the end of a sequence, so just save the object and
	    ** continue.
	    */

	    status = lwk_get_domain(persistent_object, &persistent_domain);
	    
	    if (status != lwk_s_success)
		_Raise(get_domain_failed); /* Get domain failed on persistent object */
	
	    iterate_data->previous_obj = persistent_object;
	    iterate_data->domain_prev_obj = persistent_domain;
	}

    }
        
    return (lwk_termination) 0;
    }



static lwk_termination  SearchPathCtxt(list, cpath, domain, object_desc)
lwk_list list;
 lwk_object cpath;

    lwk_domain domain;
 lwk_object_descriptor *object_desc;

/*
**++
**  Functional Description:
**	Find all the objects containing in the path list, add the paths to the
**	list and propagate if a path list.
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
    lwk_persistent	persistent_object;
    lwk_status		status;
    lwk_termination     return_value;
    lwk_domain		persistent_domain;   

    /*
    ** Retrieve the persistent object.
    */

    status = lwk_retrieve(*object_desc, &persistent_object);

    if (status == lwk_s_success) {

	/*
	** Get the persistent object's domain.
	*/
	
	status = lwk_get_domain(persistent_object, &persistent_domain);
	
	if (status != lwk_s_success)
	    _Raise(get_domain_failed); /* Get domain failed on persistent object */
	
	if (persistent_domain == lwk_c_domain_path) {

            /*
	    ** Add the path in the active path list
	    */
	    status = lwk_add_element(list, lwk_c_domain_object_desc,
				     object_desc);
			    
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(add_ele_failed); /* add element to list failed */
	    }

	}

	else {

            /*
	    ** It's a composite path, so iterate again on its object
	    */
	    status = lwk_iterate(persistent_object, lwk_c_domain_object_desc,
		    list, SearchPathCtxt, &return_value);

	    if (status != lwk_s_success) 
		_Raise(iterate_failed);
	    
	}
    }

    return (lwk_termination) 0;
    }


