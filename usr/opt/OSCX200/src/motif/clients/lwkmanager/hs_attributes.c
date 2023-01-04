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
** COPYRIGHT (c) 1990,1991 BY
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
**	Attribute utility routines
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
**  Creation Date: 20-Mar-90
**
**  Modification History:
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

#define _DefaultLinkType	"AttrDefaultLinkType"
#define _EnvXPositionAttrValue	"EnvWindowXDefaultPosition"
#define _EnvYPositionAttrValue	"EnvWindowYDefaultPosition"
#define _EnvWidthAttrValue	"EnvWindowDefaultWidth"
#define _EnvHeightAttrValue	"EnvWindowDefaultHeight"
#define _EnvWindowSplitValue	"EnvWindowDefaultSplit"

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

/*
**  Static Data Definitions
*/

/*
**  Default values for environment attributes
*/

static lwk_object null_object = lwk_c_null_object;

static _Boolean	    _DefaultRetainSource = _False;
static _DDIFString  Connection;
static _Integer	    _DefaultHighlighting =
    (lwk_c_hl_on | lwk_c_hl_sources | lwk_c_hl_targets);

/*
**  Recipient for HyperSession's default attribute values 
*/

static _Integer	_DefaultXPosition;
static _Integer	_DefaultYPosition;
static _Integer	_DefaultEnvWidth;
static _Integer	_DefaultEnvHeight;
static _Integer	_DefaultWindowSplit;
static _Boolean	_DefaultIconized    =	_False;
	
/*
**  Attribute to property name conversion table
*/

static _String _Constant AttributeToProperty[_LastAttribute] = {
    "_HsConnectionType",
    "_HsHighlight",
    "_HsOperation",
    "_HsRetainSource",
    "_HsEnvXPosition",
    "_HsEnvYPosition",
    "_HsEnvWidth",
    "_HsEnvHeight",
    "_HsEnvWindowSplit",
    "_HsEnvIconized"
};

/*
**  Attribute to domain conversion table
*/

static _Integer _Constant AttributeToDomain[_LastAttribute] = {
    (_Integer) lwk_c_domain_ddif_string,
    (_Integer) lwk_c_domain_integer,
    (_Integer) lwk_c_domain_string,
    (_Integer) lwk_c_domain_boolean,
    (_Integer) lwk_c_domain_integer,
    (_Integer) lwk_c_domain_integer,
    (_Integer) lwk_c_domain_integer,
    (_Integer) lwk_c_domain_integer,
    (_Integer) lwk_c_domain_integer,
    (_Integer) lwk_c_domain_boolean
};

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_Void  EnvAttr()
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


_String  EnvAttrAttributeToProperty(attribute)
_Attribute attribute;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
    {

    return (AttributeToProperty[(int) attribute]);
    }


_Integer  EnvAttrAttributeToDomain(attribute)
_Attribute attribute;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
    {

    return (AttributeToDomain[(int) attribute]);
    }


_AnyPtr  EnvAttrAttributeDefaultValue(attribute)
_Attribute attribute;

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
    _AnyPtr	value;
    _CString	cstr;

    switch (attribute) {

	/*
	**  If there is no special default value, return a null value
	*/

	case _ConnectionTypeAttr :
	    EnvDWGetCStringLiteral(_DefaultLinkType, &cstr);
	    Connection = _CStringToDDIFString(cstr);
	    value = (_AnyPtr) &Connection;
	    break;
	    
	case _HighlightingAttr :
	    value = (_AnyPtr) &_DefaultHighlighting;
	    break;
	
	case _RetainSourceAttr :
	    value = (_AnyPtr) &_DefaultRetainSource;
	    break;
	    
	case _EnvXPositionAttr :
	    EnvDWGetIntegerValue(_EnvXPositionAttrValue, &_DefaultXPosition);
	    value = (_AnyPtr) &_DefaultXPosition;
	    break;

	case _EnvYPositionAttr :
	    EnvDWGetIntegerValue(_EnvYPositionAttrValue, &_DefaultYPosition);
	    value = (_AnyPtr) &_DefaultYPosition;
	    break;

	case _EnvWidthAttr :
	    EnvDWGetIntegerValue(_EnvWidthAttrValue, &_DefaultEnvWidth);
	    value = (_AnyPtr) &_DefaultEnvWidth;
	    break;

	case _EnvHeightAttr :
	    EnvDWGetIntegerValue(_EnvHeightAttrValue, &_DefaultEnvHeight);
	    value = (_AnyPtr) &_DefaultEnvHeight;
	    break;

	case _EnvWindowSplit :
	    EnvDWGetIntegerValue(_EnvWindowSplitValue, &_DefaultWindowSplit);
	    value = (_AnyPtr) &_DefaultWindowSplit;
	    break;

	case _EnvIconizedAttr :
	    value = (_AnyPtr) &_DefaultIconized;
	    break;

	default :
	    value = _NullObject;
	    break;
    }
    
    return(value);
    }


_Boolean  EnvAttrIsEnvironmentAttribute(attribute)
_Attribute attribute;

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

    if ((int) attribute > _EnvironmentAttributeCount - 1)
	return(_False);
	
    return(_True);
    }


_AnyPtr  EnvAttrSetDefAttribute(attr_cnet, attribute)
lwk_persistent attr_cnet;
 _Integer attribute;

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
    _AnyPtr	    value;
    lwk_domain	    domain;
    lwk_status	    status;

    property = _AttributeToProperty(attribute);

    domain = (lwk_domain) _AttributeToDomain(attribute);

    value = _AttributeDefaultValue(attribute);

    if (value != _NullObject) {
    
	status = lwk_set_value(attr_cnet, property, domain, value,
	    lwk_c_set_property);
    }
    else {

	status = lwk_set_value(attr_cnet, property, domain, value,
	    lwk_c_delete_property);

	/*
	**  Set the value to null
	*/
	
	value = (_AnyPtr) &null_object;
    }

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }

    return(value);
    }

