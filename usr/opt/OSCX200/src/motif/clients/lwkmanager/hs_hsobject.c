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
**	Hsobject object methods
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
#include "hs_hsobject.h"

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Global Data Definitions
*/

_Global _HsObjectTypeInstance;      /* Instance record for Type */
_Global _HsObjectType;              /* Type */

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeHsObjectInstance;
static _HsObjectPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    HsObjectPropertyNameTable;

/*
**  External function definitions
*/
/*
_DeclareFunction(_Void EnvShowPropDisplayProperties,
    (hs_object, display_data, new_object));
*/    

_Void  EnvOpHsObj(hs_object)
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
    return;
    }
    

_HsObject  EnvOpHsObjCreate(type, his_type, his_obj)
_Type type;
 _Integer his_type;
 lwk_object his_obj;

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
    _HsObject	hs_object;
    lwk_object	new_obj;

    /*
    **  Create a empty HsObject
    */

    hs_object = (_HsObject) _Create(_TypeHsObject);
    
    /*
    **  Load the properties
    */

    /*
    **  This is a special case for his object descriptor we need to make a copy
    **	because we only save a pointer and object descritors may be deleted
    **	by some his operations
    */

    if ((lwk_domain)his_type == lwk_c_domain_object_desc) {

	lwk_copy(his_obj, &his_obj);
    }

    _SetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object, &his_obj,
	hs_c_set_property);

    _SetValue(hs_object, _P_HisType, hs_c_domain_integer, &his_type,
	hs_c_set_property);

    return(hs_object);
    }
    

_Void  EnvOpHsObjInitialize(hs_object, proto)
_HsObject hs_object;
 _HsObject proto;

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

    _ClearValue(&_HisType_of(hs_object), hs_c_domain_integer);
    _ClearValue(&_HisObject_of(hs_object), hs_c_domain_object);

    _StateFlags_of(hs_object) = _InitialHsObjectStateFlags;

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(hs_object, proto, MyType);

    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_HsObject) _NullObject) {
        _CopyValue(&_HisObject_of(proto), &_HisObject_of(hs_object),
	    hs_c_domain_object);
    }

    return;
    }
    

_Void  EnvOpHsObjFree(hs_object)
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
    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_HisType_of(hs_object), hs_c_domain_integer);

    if ((lwk_domain)_HisType_of(hs_object) == lwk_c_domain_object_desc) {
    
	lwk_delete(&_HisObject_of(hs_object));
    }
    	
    _DeleteValue(&_HisObject_of(hs_object), hs_c_domain_object);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(hs_object, MyType);

    return;
    }
    

_Void  EnvOpHsObjGetValue(hs_object, property, domain, value)
_HsObject hs_object;
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

        /*
        **  If property is a base property, pass the request to our supertype.
        **  Otherwise check the non-base properties.
        */

	_GetValue_S(hs_object, property, domain, value, MyType);
    }
    else {
        /*
        **  Answer depends on the property
        */

        switch (index) {
	
            case _HisTypeIndex :
                if (_IsDomain(hs_c_domain_integer, domain))
                    _CopyValue(&_HisType_of(hs_object), value,
                        hs_c_domain_integer);
                else
                    _Raise(inv_domain);
                break;
		
            case _HisObjectIndex :
                if (_IsDomain(hs_c_domain_lwk_object, domain))
                    _CopyValue(&_HisObject_of(hs_object), value,
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
    

_Void  EnvOpHsObjSetValue(hs_object, property, domain, value, flag)
_HsObject hs_object;
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
        _SetValue_S(hs_object, property, domain, value, flag, MyType);
    }
    else {
        /*
        **  Value depends on the property
        */

        switch (index) {
	
            case _HisTypeIndex :
		_SetSingleValuedProperty(&_HisType_of(hs_object),
                    hs_c_domain_integer, domain, value, flag, _False);
		break;

            case _HisObjectIndex :
		_SetSingleValuedProperty(&_HisObject_of(hs_object),
                    hs_c_domain_lwk_object, domain, value, flag, _False);
		break;

            default :
                _Raise(no_such_property);
                break;
        }
    }

    return;
    }
    

_Void  EnvOpHsObjGetProperty(hs_object, domain, property_name, value)
_HsObject hs_object;
 lwk_domain domain;

    _String property_name;
 _AnyPtr *value;
 
/*
**++
**  Functional Description:
**	Returns for a given HsObject, the corresponding HisObject requested
**	property.
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
    lwk_object	    hisobject;
    lwk_status	    status;

    /*
    ** Get the corresponding his object
    */
    
    _GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object, &hisobject);

    /*
    ** Get the requested property
    */
    
    status = lwk_get_value(hisobject, property_name, domain,
	(lwk_any_pointer *) value);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_value_failed);
    }

    return;
    }
    

_Void  EnvOpHsObjSetProperty(hs_object, domain, property_name, value, operation)
_HsObject hs_object;
 lwk_domain domain;

    _String property_name;
 _AnyPtr *value;
 lwk_set_operation operation;
 
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
    lwk_object	    hisobject;
    lwk_status	    status;

    /*
    ** Get the corresponding his object
    */
    
    _GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object, &hisobject);

    /*
    ** Set the requested property
    */
    
    status = lwk_set_value(hisobject, property_name, domain,
	(lwk_any_pointer *) value, operation);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }

    return;
    }
    

_Void  EnvOpHsObjGetCSProperty(hs_object, property_name, cs_string)
_HsObject hs_object;
 _String property_name;

    _CString *cs_string;
 
/*
**++
**  Functional Description:
**	Returns for a given HsObject, the corresponding HisObject requested
**	compound string property.
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
    lwk_object	    hisobject;
    lwk_status	    status;
    lwk_ddif_string ddif_str;

    /*
    ** Get the corresponding his object
    */
    
    _GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object, &hisobject);

    /*
    ** Get the DDIF string property
    */
    
    status = lwk_get_value(hisobject, property_name, lwk_c_domain_ddif_string,
	(lwk_any_pointer) &ddif_str);

    if (status != lwk_s_success) {

	_SaveHisStatus(status);
	_Raise(get_value_failed);
    }

    /*
    ** Convert the string to a compound string
    */
    
    *cs_string = _DDIFStringToCString(ddif_str);
    lwk_delete_ddif_string (&ddif_str);

    return;
    }
    

_Void  EnvOpHsObjSetCSProperty(hs_object, property_name, cs_string, operation)
_HsObject hs_object;
 _String property_name;

    _CString cs_string;
 lwk_set_operation operation;
 
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
    lwk_object	    hisobject;
    lwk_status	    status;
    _DDIFString	    ddif_str;

    /*
    ** Get the corresponding his object
    */
    
    _GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object, &hisobject);

    /*
    ** Convert the compound string into DDIF format to set the property on the
    ** HIS object
    */

    ddif_str = _CStringToDDIFString(cs_string);

    /*
    ** Set the requested property
    */
    
    status = lwk_set_value(hisobject, property_name, lwk_c_domain_ddif_string,
	(lwk_any_pointer) &ddif_str, operation);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }

    _DeleteDDIFString(&ddif_str);

    return;
    }
    

_Boolean  EnvOpHsObjGetLinkbase(hs_object, linkbase_name, linkbase_id)
_HsObject hs_object;
 _CString *linkbase_name;

    _String *linkbase_id;
 
/*
**++
**  Functional Description:
**	Returns for a given HsObject, the corresponding linkbase name and
**	linkbase identifier
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
    lwk_object		    hisobject;
    lwk_status		    status;
    lwk_object_descriptor   obj_desc;
    lwk_linkbase	    linkbase;
    lwk_ddif_string	    ddif_str;
    
    *linkbase_name = (_CString) 0;    
    *linkbase_id = (_String) 0;
    
    if (_HisType_of(hs_object) == (_Integer) lwk_c_domain_linkbase)
	_Raise(inv_domain);
    
    /*
    ** If it is an object descriptor, get the info fro it otherwise get them
    ** from the linkbase object
    */

    if ((lwk_domain) _HisType_of(hs_object) == lwk_c_domain_object_desc) {

	/*
	**  Get info directly from the object descriptor
	*/
	
	status = lwk_get_value(_HisObject_of(hs_object),
	    lwk_c_p_linkbase_name, lwk_c_domain_ddif_string, &ddif_str);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed);
	}

	*linkbase_name = _DDIFStringToCString(ddif_str);
	lwk_delete_ddif_string(&ddif_str);

	status = lwk_get_value(_HisObject_of(hs_object),
	    lwk_c_p_linkbase_identifier, lwk_c_domain_string, linkbase_id);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed); /* linkbase id property query failed */
	}
    }
    else {

	/*
	**  Get the linkbase property
	*/

	status = lwk_get_value(_HisObject_of(hs_object), lwk_c_p_linkbase,
	    lwk_c_domain_linkbase, &linkbase);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed);
	}

	/*
	**  Return if no linkbase
	*/

	if (linkbase == lwk_c_null_object)
	    return(_False);
	      
	/*
	**  Get the info from the linkbase object
	*/
	
	status = lwk_get_value(linkbase, lwk_c_p_identifier,
	    lwk_c_domain_string, linkbase_id);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed); /* linkbase id property query failed */
	}

	status = lwk_get_value(linkbase, lwk_c_p_name,
	    lwk_c_domain_ddif_string, &ddif_str);
	    
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed);
	}

	*linkbase_name = _DDIFStringToCString(ddif_str);
	lwk_delete_ddif_string(&ddif_str);
    }
	
    return(_True);
    }
    

_Void  EnvOpHsObjDisplayProperties(hs_object, display_data, new_object)
_HsObject hs_object;
 _AnyPtr display_data;

    _Boolean new_object;

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
    **  Call the Show Property module for handling the request
    */
    
    EnvShowPropDisplayProperties(hs_object, display_data, new_object);
	
    return;
    }
    

_Void  EnvOpHsObjSave(hs_object, hs_linkbase, operation)
_HsObject hs_object;
 _HsObject hs_linkbase;

    _SaveOperation operation;

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
    lwk_status	    status;
    _Boolean	    state;
    
    /*
    **  Save only the HIS persistent objects 
    */
    
    if ((_HisType_of(hs_object) != (_Integer) lwk_c_domain_comp_linknet) &&
	(_HisType_of(hs_object) != (_Integer) lwk_c_domain_comp_path) &&
	(_HisType_of(hs_object) != (_Integer) lwk_c_domain_linknet) &&
	(_HisType_of(hs_object) != (_Integer) lwk_c_domain_path))
	
	_Raise(inv_domain);
	
    /*
    **  Dispatch according to the type of operation
    */

    switch (operation) {

	case _StoreObject :

	    status = lwk_store(_HisObject_of(hs_object),
		_HisObject_of(hs_linkbase));
	    state = _SetHsObjState(hs_object, _StateSaved, _StateSet);
	    
	    break;

	case _DeleteObject :
	
	    status = lwk_drop(_HisObject_of(hs_object));
	    status = lwk_delete(&_HisObject_of(hs_object));
		
	    break;

	default :
	    _Raise(inv_operation);
	    break;
    }

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(save_failed);  /* save operation failed */
    }
        
    /*
    **  Reset the flags
    */

    state = _SetHsObjState(hs_object, _StateModified, _StateClear);

    return;
    }

   
_Void  EnvOpHsObjSaveComposite(container, hs_object, operation)
_HsObject container;
 _HsObject hs_object;

    _SaveOperation operation;

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
    lwk_status	    status;

    /*
    **  Check if the object is really a composite
    */

    if ((_HisType_of(container) != (_Integer) lwk_c_domain_comp_linknet) &&
	(_HisType_of(container) != (_Integer) lwk_c_domain_comp_path))
	_Raise(inv_domain);

    if (_HisType_of(hs_object) != (_Integer) lwk_c_domain_object_desc)
	_Raise(inv_domain);

    /*
    **  Dispatch according to the type of operation
    */
    
    switch (operation) {

	case _StoreObject :
	
	    /*
	    **  For object descriptor a store operation is a set value on
	    **  its container
	    */

	    if (_HisType_of(container) == (_Integer) lwk_c_domain_comp_linknet)

		status = lwk_set_value(_HisObject_of(container),
		    lwk_c_p_linknets, lwk_c_domain_object_desc,
		    &_HisObject_of(hs_object), lwk_c_add_property);

	    else
	    
		status = lwk_set_value(_HisObject_of(container),
		    lwk_c_p_paths, lwk_c_domain_object_desc,
		    &_HisObject_of(hs_object), lwk_c_add_property);
	    
	    break;

	case _DeleteObject :
	    
		/*
		**  For object descriptor a delete store operation is a
		**  set value on its container
		*/
		
		if (_HisType_of(container) == (_Integer) lwk_c_domain_comp_linknet)

		    status = lwk_set_value(_HisObject_of(container),
			lwk_c_p_linknets, lwk_c_domain_object_desc,
			&_HisObject_of(hs_object), lwk_c_delete_property);

		else
		
		    status = lwk_set_value(_HisObject_of(container),
			lwk_c_p_paths, lwk_c_domain_object_desc,
			&_HisObject_of(hs_object), lwk_c_delete_property);
			
	    break;

	default :
	    _Raise(inv_operation);
	    break;

    }

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(save_failed);  /* save operation failed */
    }

    return;
    }
        

_Boolean  EnvOpHsObjSetHsObjState(hs_object, state, operation)
_HsObject hs_object;
 _HsObjectState state;

    _StateOperation operation;

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
    _StateFlags     flag;
    _Boolean        old_state;

    /*
    **  Turn the given State into a Flag
    */

    flag = _StateToFlag(state);

    /*
    **  Get the old State
    */

    old_state = ((_StateFlags_of(hs_object) & flag) != 0);

    /*
    **  Set the State as requested
    */

    switch (operation) {

        case _StateSet :
            _StateFlags_of(hs_object) |= flag;
            break;

        case _StateClear :
            _StateFlags_of(hs_object) &= ~flag;
            break;

        case _StateComplement :
            _StateFlags_of(hs_object) ^= flag;
            break;

        case _StateGet :

        default :
            break;
    }

    /*
    **  Return the old state
    */

    return old_state;
    }


_Void  EnvOpHsObjStore(hs_object)
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
    _CString	    cs_name;
    _String	    identifier;
    lwk_linkbase  linkbase;
    lwk_status	    status;

    /*
    **  Get the linkbase information for this object
    */
    
    _GetLinkbase(hs_object, &cs_name, &identifier);

    _DeleteCString(&cs_name);
    
    /*
    **  Open the linkbase
    */
    
    status = lwk_open(identifier, _False, &linkbase);

    _DeleteString(&identifier);
    
    /*
    **  If the linkbase doesn't exist anymore can't store the object
    */
    
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(open_lb_err);
    }

    /*
    **  Store the object
    */
        
    status = lwk_store(_HisObject_of(hs_object), linkbase);
    
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed);
    }

    /*
    **  Close the linkbase
    */
    
    status = lwk_close(linkbase);
    
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(close_lb_err);
    }

    return;
    }

