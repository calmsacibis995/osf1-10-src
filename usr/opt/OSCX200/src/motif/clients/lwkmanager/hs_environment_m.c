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
** COPYRIGHT (c) 1989, 1990, 1991 BY
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
**	LinkWorks Manager environment retrieval support routines
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Andre Pavanello
**
**  Creation Date: 13-Nov-89
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

#ifdef VMS
#include <stdlib.h>	/* needed for getenv on VMS only */
#endif

/*
**  Macro Definitions
*/

#define _DisplayRootDialog(yes_cb, no_cb, identifier) \
    EnvDWRootDialogBox((yes_cb), (no_cb), (identifier))
_DeclareFunction(_Void EnvDWRootDialogBox,
    (_Callback yes_callback, _Callback no_callback, _String identifier));

/*
**  Default linkbase handling
*/

#define _LinkbaseLogicalFilename	"LWK_PERSONAL"

#ifdef VMS
#define _LinkbaseDefaultDirectory	"SYS$LOGIN"
#define _LinkbaseFilename		"LWK_PERSONAL.LINKBASE"
#else
#define _LinkbaseDefaultDirectory	"HOME"                
#define _LinkbaseFilename		"/.lwk_personal.linkbase"
#endif

         
/*
**  Type Definitions
*/

#define	_EnvironmentQueryCNetObjectCount 4
#define	_EnvironmentQueryCPathObjectCount 4

typedef struct __EnvQueryData {
	    _Integer	    count;
	    _String	    attribute_name;
	    lwk_persistent  attribute_obj;
	    _String	    net_list_name;
	    lwk_persistent  net_list_obj;
	    _String	    active_net_name;
	    lwk_persistent  active_net_obj;
	    _String	    current_net_name;
	    lwk_persistent  current_net_obj;
	    _String	    path_list_name;
	    lwk_persistent  path_list_obj;
	    _String	    active_path_name;
	    lwk_persistent  active_path_obj;
	    _String	    path_index_name;
	    lwk_persistent  path_index_obj;
	    _String	    trail_name;
	    lwk_persistent  trail_obj;
	} _EnvQueryDataInstance, *_EnvQueryData;

/*
**  Static Data Definitions
*/

/*
**  Table of Contents
*/

_DeclareFunction(static _Void EnvClearCurrency, (_EnvWindow env_window));

_DeclareFunction(static _Void EnvLoadAttributes,
    (_EnvContext env_context, lwk_linkbase linkbase, _EnvQueryData data));

_DeclareFunction(static _Void EnvLoadNetworks,
    (_EnvContext env_context, lwk_linkbase linkbase, _EnvQueryData data));

_DeclareFunction(static _Void EnvLoadActiveNets,
    (_EnvContext env_context, lwk_linkbase linkbase, _Boolean new,
     _EnvQueryData data));

_DeclareFunction(static _Void EnvLoadCurrentNet,
    (_EnvContext env_context, lwk_linkbase linkbase, _EnvQueryData data));

_DeclareFunction(static _Void EnvLoadPaths,
    (_EnvContext env_context, lwk_linkbase linkbase, _EnvQueryData data));

_DeclareFunction(static _Void EnvLoadTrail,
    (_EnvContext env_context, lwk_linkbase linkbase, _EnvQueryData data));

_DeclareFunction(static lwk_termination GetPersistent,
    (lwk_persistent *persistent, lwk_object cnet, lwk_domain domain,
     lwk_object_descriptor *object_desc));

_DeclareFunction(static lwk_persistent CreatePersistent,
    (lwk_linkbase linkbase, _String name, lwk_domain domain));

_DeclareFunction(static lwk_termination SelectObject,
    (_EnvQueryData data, lwk_object linkbase, lwk_domain domain,
     lwk_persistent *object));

_DeclareFunction(static _Boolean MatchObjectName,
    (_EnvQueryData data, _String obj_name,  lwk_persistent object));

_DeclareFunction(static _Boolean LoadObject,
    (_EnvQueryData data, lwk_persistent *obj, lwk_persistent object));

_DeclareFunction(static _Void InsertInComposite,
    (lwk_linkbase linkbase, lwk_persistent container, lwk_persistent object,
     _String property));

_DeclareFunction(static _String ResolveLinkbaseFileSpec, ());

_DeclareFunction(static _Void CleanupQueryData, (_EnvQueryData data));

_DeclareFunction(static _Void CreateLinkBase,
    (lwk_string lb_spec, lwk_linkbase *linkbase));

_DeclareFunction(static _Void EnvDwLBVersionYes,
    (Widget w, _WindowPrivate null_tag, _Reason reason));

_DeclareFunction(_Void EnvDwMessageDismissRootDb,());
    

_Void  EnvRet()
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


_EnvWindow  EnvRetDisplayEnvironment(create_lb)
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
    _EnvContext	env_context = (_EnvContext) 0;
    _EnvWindow	env_window = (_EnvWindow) 0;
    _Boolean	new_ctxt;
    Widget	toplevel;

    _StartExceptionBlock

    /*
    **  Retrieve the environment context
    */

    env_context = (_EnvContext) _Retrieve(_TypeEnvContext, &new_ctxt, create_lb);

    /*
    **  Open the environment window
    */
    
    toplevel = EnvDwMessageGetToplevel();

    if (create_lb)
	new_ctxt = _False;

    if (env_context != _NullObject) {
	env_window = (_EnvWindow) _OpenEnv(_TypeEnvWindow, toplevel,
	    env_context, new_ctxt);

	/*
	**  Display the environment context
	*/

	_Display(env_window, env_context);
    }

    _Exceptions
	_When(version_error)
	    _DisplayRootDialog(EnvDwLBVersionYes, (_Callback) 0,
		_LbNewVersionMsg);

    _EndExceptionBlock

    return env_window;
    }


_Void  EnvRetrieveEnvironment(env_context, linkbase)
_EnvContext env_context;

    lwk_linkbase linkbase;

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
    lwk_query_node  *query_expression;
    _EnvQueryData   data;
    lwk_status	    status;
    lwk_termination cb_return;

    data = (_EnvQueryData) _AllocateMem(sizeof(_EnvQueryDataInstance));
    _ClearMem(data, sizeof(_EnvQueryDataInstance));

    /*
    ** Initialize the query data strucutre
    */

    data->count = _EnvironmentQueryCNetObjectCount;
    
    EnvDWGetStringLiteral(_MrmDefaultAttributeObjectName,
	&(data->attribute_name));
    EnvDWGetStringLiteral(_MrmDefaultNetworkListName, &(data->net_list_name));
    EnvDWGetStringLiteral(_MrmDefaultActiveNetworkListName, &(data->active_net_name));
    EnvDWGetStringLiteral(_MrmDefaultCurrentNetworkName, &(data->current_net_name));

    status = lwk_iterate(linkbase, lwk_c_domain_comp_linknet, data,
	SelectObject, &cb_return);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(iterate_failed);
    }

    /*
    ** Load the composite nets
    */

    EnvLoadAttributes(env_context, linkbase, data);
    EnvLoadNetworks(env_context, linkbase, data);
    
    /*
    ** Prepare for the iterate on composite paths
    */
        
    data->count = _EnvironmentQueryCPathObjectCount;
    
    EnvDWGetStringLiteral(_MrmDefaultPathListName, &(data->path_list_name));
    EnvDWGetStringLiteral(_MrmDefaultActivePathListName, &(data->active_path_name));
    EnvDWGetStringLiteral(_MrmDefaultActivePathIndexName, &(data->path_index_name));
    EnvDWGetStringLiteral(_MrmDefaultTrailListName, &(data->trail_name));
            
    status = lwk_iterate(linkbase, lwk_c_domain_comp_path, data,
	SelectObject, &cb_return);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(iterate_failed);
    }

    /*
    ** Load the various private objects
    */
    
    EnvLoadPaths(env_context, linkbase, data);
    EnvLoadTrail(env_context, linkbase, data);

    CleanupQueryData(data);    

    return;
    }


_Boolean  EnvOpenDefaultLinkbase(linkbase, create)
lwk_linkbase *linkbase;
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
    lwk_status		status;
    _Boolean		new_linkbase;
    _String		linkbase_spec;
    _DDIFString		ddif_name;
              
    _StartExceptionBlock

    new_linkbase = _False;

    /*
    **  Resolve the default linkbase file spec
    */

    linkbase_spec = ResolveLinkbaseFileSpec();

    if (create) {

        /*
	** Create a new personal linkbase even if one already exists
	*/
	
	CreateLinkBase(linkbase_spec, linkbase);
	new_linkbase = _True;
    }
    
    else {

        /*
	** Try open an existing personal linkbase
	*/
	
	status = lwk_open(linkbase_spec, lwk_c_false, linkbase);

	if (status == lwk_s_no_such_linkbase) {

	    /*
	    **  Create a new default linkbase
	    */

	    CreateLinkBase(linkbase_spec, linkbase);
	    new_linkbase = _True;
	}

	else if (status == lwk_s_db_version_error) {
	    _Raise(version_error);
	}
	
	else if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(open_lb_err); /* Open of linkbase failed */
	}
    }

    _DeleteString(&linkbase_spec);

    _Exceptions
	_WhenOthers
	    if (linkbase_spec != (_String) 0)
		_DeleteString(&linkbase_spec);
	    _Reraise;

    _EndExceptionBlock

    return(new_linkbase);
    }


static _Void  CreateLinkBase(lb_spec, linkbase)
lwk_string lb_spec;
 lwk_linkbase *linkbase;

    {
    lwk_status	    status;
    lwk_ddif_string lwk_ddifstr;
    _String	    string;
    _DDIFString	    ddif_name;

    /*
    ** Create a new linkbase
    */
    
    status = lwk_open(lb_spec, lwk_c_true, linkbase);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(create_lb_err);  /* error creating linkbase */
    }
    
    /*
    **  Set the linkbase name property
    */

    EnvDWGetStringLiteral(_MrmLinkbaseName, &string);
    ddif_name = _StringToDDIFString(string);
    lwk_ddifstr = (lwk_ddif_string) ddif_name;
    _FreeMem(string);
    
    status = lwk_set_value(*linkbase, lwk_c_p_name,
	lwk_c_domain_ddif_string, &lwk_ddifstr, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);  /* Set value on name property failed */
    }

    return;
    }
    

_Void  EnvSetCurrencies(env_window, env_context)
_EnvWindow env_window;
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
    _HsObject		hs_obj;
    _Integer		count;
    lwk_composite_linknet	cnet;
    lwk_composite_path	cpath;
    lwk_termination	termination;
    lwk_linknet		network;
    lwk_list		list;
    lwk_status		status;

    /*
    **  Set the active network list as the current composite network
    */

    _GetValue(env_context, _P_ActiveNetworks, hs_c_domain_hsobject, &hs_obj);

    _GetValue(hs_obj, _P_HisObject, hs_c_domain_lwk_object, &cnet);

    _SetCurrency(env_window, lwk_c_env_active_comp_linknet, &cnet);

    /*
    **  Get the current network in the corresponding cnet
    */

    _GetValue(env_context, _P_CurrentNetwork, hs_c_domain_hsobject, &hs_obj);

    _GetValue(hs_obj, _P_HisObject, hs_c_domain_lwk_object, &cnet);

    network = lwk_c_null_object;

    status = lwk_iterate(cnet, lwk_c_domain_object_desc,
	(lwk_closure) &network, (lwk_callback) GetPersistent, &termination);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(iterate_failed);   /* iterate on cnet failed */
    }

    /*
    **  Set the currency for current network
    */

    _SetCurrency(env_window, lwk_c_env_recording_linknet, (_AnyPtr) &network);

    /*
    **  Set the current composite path
    */

    _GetValue(env_context, _P_ActivePaths, hs_c_domain_hsobject, &hs_obj);

    _GetValue(hs_obj, _P_HisObject, hs_c_domain_lwk_object, &cpath);

    /*
    **  If there are no composite path or no elements in the composite path
    **	don't set it as current so the Step Forward menu in the connection
    **	menu is dimed
    */

    if (cpath != lwk_c_null_object) {

	status = lwk_get_value(cpath, lwk_c_p_paths, lwk_c_domain_list, &list);

	if (list != lwk_c_null_object) {

	    status = lwk_get_value(list, lwk_c_p_element_count,
		lwk_c_domain_integer, &count);
	    
	    if (count == 0)
		cpath = lwk_c_null_object;
	}
	else
	    cpath = lwk_c_null_object;
    }

    /*
    **  Set the active path list as the current composite path
    */

    _SetCurrency(env_window, lwk_c_env_active_comp_path, &cpath);

    /*
    **  Clear all the other currencies
    */

    EnvClearCurrency(env_window);

    return;
    }


static _Void  EnvClearCurrency(env_window)
_EnvWindow env_window;

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
    lwk_object	    null_object;

    /*
    **  Clear all the other current/default attributes
    */

    null_object = lwk_c_null_object;

    _SetCurrency(env_window, lwk_c_env_appl_highlight, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_recording_path, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_active_path, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_active_path_index, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_followed_step, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_new_link, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_pending_source, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_pending_target, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_followed_link, (_AnyPtr) &null_object);

    _SetCurrency(env_window, lwk_c_env_follow_destination, (_AnyPtr) &null_object);

    return;
    }

       
static _Void  EnvLoadAttributes(env_context, linkbase, data)
_EnvContext env_context;

    lwk_linkbase linkbase;
 _EnvQueryData data;

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
    _HsObject		hs_obj;
    _Boolean		new;
    
    _StartExceptionBlock
    
    new = _False;

    /*
    **  Get the attribute composite network from the linkbase. If it is not
    **	there create a new one
    */

    if (data->attribute_obj == lwk_c_null_object) {
	
	data->attribute_obj = (lwk_composite_linknet) CreatePersistent(linkbase,
	    data->attribute_name, lwk_c_domain_comp_linknet);
	
	_SetPrivateProperty(data->attribute_obj, linkbase);

	new = _True;
    }

    /*
    **  Create the HsObject for the attribute list and set the property on the
    **	environment context object
    */

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_linknet, data->attribute_obj);

    _SetValue(env_context, _P_Attributes, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);

    /*            
    **  If a new Attribute object has been created, reset all the attributes
    **	to default values
    */

    if (new)
	_LoadDefaultAttributes(env_context);

    /*
    **  If any exceptions are raised dislplay an informational message
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	    
	    status[0] = _StatusCode(cannot_load_attr);
	    status[1] = _Others;

	    _DisplayRootMessage(status, 2, hs_c_info_message);

    _EndExceptionBlock
	
    return;
    }


static _Void  EnvLoadNetworks(env_context, linkbase, data)
_EnvContext env_context;
 lwk_linkbase linkbase;

    _EnvQueryData data;

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
    _HsObject		hs_obj;
    _Boolean		new;
    
    new = _False;

    /*
    **  Get the network list composite network from the linkbase. If it is not
    **	there create a new one
    */

    if (data->net_list_obj == lwk_c_null_object) {
	
	data->net_list_obj = (lwk_composite_linknet) CreatePersistent(linkbase,
	    data->net_list_name, lwk_c_domain_comp_linknet);
	
	_SetPrivateProperty(data->net_list_obj, linkbase);
	
	new = _True;
    }

    /*
    **  Create the HsObject for the network list and set the property on the
    **	environment context object
    */

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_linknet, data->net_list_obj);

    _SetValue(env_context, _P_Networks, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);

    /*
    **  Load the network active list
    */

    EnvLoadActiveNets(env_context, linkbase, new, data);

    /*
    **  Retrieve the current network
    */

    EnvLoadCurrentNet(env_context, linkbase, data);

    return;
    }


static _Void  EnvLoadActiveNets(env_context, linkbase, new, data)
_EnvContext env_context;

    lwk_linkbase linkbase;
 _Boolean new;
 _EnvQueryData data;

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
    lwk_status		status;
    _HsObject		hs_obj;
    
    /*
    **	Look for the active network list composite network
    */

    if (data->active_net_obj == lwk_c_null_object) {
    
	/*
	**  Create a new one if it is not there
	*/
	
	data->active_net_obj = CreatePersistent(linkbase,
	    data->active_net_name, lwk_c_domain_comp_linknet);

	_SetPrivateProperty(data->active_net_obj, linkbase);
    }

    else {

	if (new) {

	    /*
	    **  If a new network list has been created, the active list must be
	    **	obsolete, so delete it and create a new one
	    */

	    status = lwk_drop(data->active_net_obj);
	
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(persobj_deletion_failed);
	    }
	    data->active_net_obj = CreatePersistent(linkbase,
		data->active_net_name, lwk_c_domain_comp_linknet);

	    _SetPrivateProperty(data->active_net_obj, linkbase);
	}
    }

    /*
    **  Set the property on the environment context object
    */

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_linknet, data->active_net_obj);

    _SetValue(env_context, _P_ActiveNetworks, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);

    return;
    }


static _Void  EnvLoadCurrentNet(env_context, linkbase, data)
_EnvContext env_context;

    lwk_linkbase linkbase;
 _EnvQueryData data;

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
    lwk_composite_linknet	net_list;
    lwk_linknet		network;
    _HsObject		hs_obj;
    _HsObject		hs_net_list;
    _String		string;
    _String		net_string;
        
    network = (lwk_linknet) lwk_c_null_object;

    /*
    **  Look for the current network composite network
    */

    if (data->current_net_obj == lwk_c_null_object) {

	/*
	**  If it is not there create a new one with an empty network
	*/
	
	data->current_net_obj = (lwk_composite_linknet) CreatePersistent(linkbase,
	    data->current_net_name, lwk_c_domain_comp_linknet);
	
	_SetPrivateProperty(data->current_net_obj, linkbase);

        EnvDWGetStringLiteral(_MrmDefaultNetworkName, &net_string);
	
	network = (lwk_linknet) CreatePersistent(linkbase, net_string,
	    lwk_c_domain_linknet);

	_FreeMem(net_string);

	InsertInComposite(linkbase, data->current_net_obj, network,
	    lwk_c_p_linknets);

	/*
	**  Insert the new current network in the active list as default action
	*/

	_GetValue(env_context, _P_ActiveNetworks, hs_c_domain_hsobject,
	    &hs_net_list);

	_GetValue(hs_net_list, _P_HisObject, hs_c_domain_lwk_object,
	    &net_list);

	InsertInComposite(linkbase, net_list, network, lwk_c_p_linknets);
	
	/*
	**  Insert the new current network in the network list as default action
	*/

	_GetValue(env_context, _P_Networks, hs_c_domain_hsobject,
	    &hs_net_list);

	_GetValue(hs_net_list, _P_HisObject, hs_c_domain_lwk_object,
	    &net_list);

	InsertInComposite(linkbase, net_list, network, lwk_c_p_linknets);
	
    }

    /*
    **  Create an HsObject and set the property on the environment context
    **	object
    */

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_linknet, data->current_net_obj);

    _SetValue(env_context, _P_CurrentNetwork, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);

    return;
    }


static lwk_termination  GetPersistent(persistent, cnet, domain, object_desc)
lwk_persistent *persistent;

    lwk_object cnet;
 lwk_domain domain;
 lwk_object_descriptor *object_desc;

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

    status = lwk_retrieve(*object_desc, persistent);

    /*
    **  If the retrieve fails that means that the current network is not
    **	available and it won't be set current. No special action is needed
    */
/*
    if (status != lwk_s_success) {
    
	_SaveHisStatus(status);
	_Raise(object_retrieve_failed);
    }
*/    
    return (_True);
    }


static _Void  EnvLoadPaths(env_context, linkbase, data)
_EnvContext env_context;
 lwk_linkbase linkbase;

    _EnvQueryData data;

/*
**++
**  Functional Description:
**	Retrieve the paths list from the linkbase.
**	Retrieve also the active paths index to recreate the active paths list.
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
    lwk_status		status;
    _HsObject		hs_obj;

			
    _StartExceptionBlock

    /*
    **  Get the path list composite path from the linkbase. If it is not
    **	there create a new one
    */
    
    if (data->path_list_obj == lwk_c_null_object) {
	
	data->path_list_obj = (lwk_composite_path) CreatePersistent(linkbase,
	    data->path_list_name, lwk_c_domain_comp_path);

        _SetPrivateProperty(data->path_list_obj, linkbase);
    }

    /*
    ** Create the HsObject for the path list and set the property on the
    ** environment context object
    */

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_path, data->path_list_obj);

    _SetValue(env_context, _P_Paths, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);

    /*
    ** Look for the active path list composite path
    */

    if (data->active_path_obj == lwk_c_null_object) {

        /*
	** Create a new one if it is not there
	*/
	
	data->active_path_obj = CreatePersistent(linkbase,
	    data->active_path_name, lwk_c_domain_comp_path);

	_SetPrivateProperty(data->active_path_obj, linkbase);
    }

    /*
    **  Create the HsObject for the active path list and set the
    **	property on the environment context
    */

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_path, data->active_path_obj);

    _SetValue(env_context, _P_ActivePaths, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);
	
    /*
    **	Look for the active path index composite path 
    */

    if (data->path_index_obj == lwk_c_null_object) {

        /*
	** Create a new one if it is not there
	*/
	
	data->path_index_obj = CreatePersistent(linkbase,
	    data->path_index_name, lwk_c_domain_comp_path);

	_SetPrivateProperty(data->path_index_obj, linkbase);
    }
    
    /*
    **  Create the HsObject for the active path index and set the property
    **	on the environment context object. The active paths will be created again
    **	from the active paths index.
    */

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_path, data->path_index_obj);

    _SetValue(env_context, _P_ActivePathIndex, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);

    /*
    **  If any exceptions are raised dislplay an informational message
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	    
	    status[0] = _StatusCode(cannot_load_path);
	    status[1] = _Others;

	    _DisplayRootMessage(status, 2, hs_c_info_message);

    _EndExceptionBlock
		
    return;
    }


static _Void  EnvLoadTrail(env_context, linkbase, data)
_EnvContext env_context;
 lwk_linkbase linkbase;

    _EnvQueryData data;

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
    _HsObject		hs_obj;
    _HsObject		hs_path_list;

			
    _StartExceptionBlock
    
    /*
    **  Look for the trail composite path
    */

    if (data->trail_obj == lwk_c_null_object) {

	/*
	**  If it is not there create a new one with an empty trail
	*/
	
	data->trail_obj = (lwk_composite_path) CreatePersistent(linkbase,
	    data->trail_name, lwk_c_domain_comp_path);

        _SetPrivateProperty(data->trail_obj, linkbase);
    }

    /*    
    **  Create an HsObject and set the property on the environment context
    **	object
    */       

    hs_obj = (_HsObject) _CreateHsObject(_TypeHsObject,
	lwk_c_domain_comp_path, data->trail_obj);

    _SetValue(env_context, _P_Trail, hs_c_domain_hsobject, &hs_obj,
	hs_c_set_property);

    /*
    **  If any exceptions are raised dislplay an informational message
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	    
	    status[0] = _StatusCode(cannot_load_trail);
	    status[1] = _Others;

	    _DisplayRootMessage(status, 2, hs_c_info_message);

    _EndExceptionBlock
		
    return;
    }


static lwk_persistent  CreatePersistent(linkbase, name, domain)
lwk_linkbase linkbase;
 _String name;

    lwk_domain domain;

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
    _DDIFString		ddif_str;
    _Date		volatile date;
    lwk_status		status;
    lwk_ddif_string	lwk_ddifstr;
    lwk_persistent	persistent;
    _String		string;
    
    /*
    **  Create the persistent object
    */

    status = lwk_create(domain, &persistent);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(persobj_creation_failed);
    }

    /*
    **  Set the name property
    */

    ddif_str = _StringToDDIFString(name);
    lwk_ddifstr = (lwk_ddif_string) ddif_str;

    status = lwk_set_value(persistent, lwk_c_p_name, lwk_c_domain_ddif_string,
	&lwk_ddifstr, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed); /* Set Value failed */
    }
	
    _DeleteDDIFString(&ddif_str);

    /*
    **  Set the author property
    */

    EnvDWGetStringLiteral(_MrmDefaultAuthor, &string);

    ddif_str = _StringToDDIFString(string);
    lwk_ddifstr = (lwk_ddif_string) ddif_str;

    _FreeMem(string);

    status = lwk_set_value(persistent, lwk_c_p_author, lwk_c_domain_ddif_string,
	&lwk_ddifstr, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed); /* Set Value failed */
    }
	
    _DeleteDDIFString(&ddif_str);

    /*
    **  Set the description property
    */
/*
    cstring = (his_compound_string) _StringToXmString((char *) desc);

    status = lwk_set_value(persistent, lwk_c_p_description,
	lwk_c_domain_comp_string, &cstring, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }
	
    XmStringFree(cstring);

*/

    /*
    **  Set the creation date
    */

    date = (_Date) _NowToDate;

    status = lwk_set_value(persistent, lwk_c_p_creation_date, lwk_c_domain_date,
	&date, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }
	
    /*
    **  Store it
    */

    status = lwk_store(persistent, linkbase);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed); /* store failed */
    }
	
    return persistent;
    }


static lwk_termination  SelectObject(data, linkbase, domain, object)
_EnvQueryData data;
 lwk_object linkbase;

    lwk_domain domain;
 lwk_persistent *object;

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
    lwk_status		status;
    lwk_ddif_string	lwkddif_obj_name;
    _Boolean		done;
    _Integer		result;
    _String		object_name;

    status = lwk_get_value(*object, lwk_c_p_name, lwk_c_domain_ddif_string,
	&lwkddif_obj_name);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_value_failed); /* Get Value failed */
    }

    object_name = _DDIFStringToString((_DDIFString) lwkddif_obj_name);

    done = MatchObjectName(data, object_name, *object);

    _DeleteString(&object_name);
    lwk_delete_ddif_string(&lwkddif_obj_name);

    return(done);
    }


static _Boolean  MatchObjectName(data, obj_name, object)
_EnvQueryData data;
 _String obj_name;

    lwk_persistent object;

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

    if (data->attribute_obj == lwk_c_null_object)
	if(_CompareString(data->attribute_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->attribute_obj),
		object));

    if (data->net_list_obj == lwk_c_null_object)
	if(_CompareString(data->net_list_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->net_list_obj),
		object));

    if (data->active_net_obj == lwk_c_null_object)
	if(_CompareString(data->active_net_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->active_net_obj),
		object));

    if (data->current_net_obj == lwk_c_null_object)
	if(_CompareString(data->current_net_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->current_net_obj),
		object));

    if (data->path_list_obj == lwk_c_null_object)
	if(_CompareString(data->path_list_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->path_list_obj),
		object));

    if (data->active_path_obj == lwk_c_null_object)
	if(_CompareString(data->active_path_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->active_path_obj),
		object));

    if (data->path_index_obj == lwk_c_null_object)
	if(_CompareString(data->path_index_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->path_index_obj),
		object));

    if (data->trail_obj == lwk_c_null_object)
	if(_CompareString(data->trail_name, obj_name) == 0)
	    return((lwk_termination) LoadObject(data, &(data->trail_obj),
		object));

    /*
    ** Continue the iteration
    */
    
    return (lwk_termination) 0;
    }
    

static _Boolean  LoadObject(data, obj, object)
_EnvQueryData data;
 lwk_persistent *obj;

    lwk_persistent object;

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
    
    *obj = object;
    data->count--;
    
    if (data->count > 0) 
	return (lwk_termination) 0;   /* continue iteration */
    else
	return (lwk_termination) 1;   /* all objects found: stop iterate */
	
    }


static _Void  InsertInComposite(linkbase, container, object, property)
lwk_linkbase linkbase;

    lwk_persistent container;
 lwk_persistent object;
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
    lwk_status		  status;
    lwk_object_descriptor odesc;

    /*
    **  Get the object descriptor for the default network and insert it in the
    **	composite network
    */

    status = lwk_get_object_descriptor(object, &odesc);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_objdsc_failed); /* get object descriptor failed */
    }
	
    status = lwk_set_value(container, property, lwk_c_domain_object_desc,
	&odesc, lwk_c_add_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed); /* Set Value failed */
    }
	
    status = lwk_delete(&odesc);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(persobj_deletion_failed); /* delete failed */
    }
	
    status = lwk_store(container, linkbase);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed); /* store failed */
    }
	
    return;
    }


static _String  ResolveLinkbaseFileSpec()
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
    _String	value;
    _String	db;
    _String	db_filespec;

    /*
    **  Translate logical/environment name
    */

    db = (_String) getenv(_LinkbaseLogicalFilename);

    /*
    **  If no logical defined, build the default one
    */

    if (db == (_String) 0) {

	value = (_String) getenv(_LinkbaseDefaultDirectory);
	
	db_filespec = _CopyString(value);
	
	db_filespec = _ConcatString(db_filespec, _LinkbaseFilename);
    }
    else
	db_filespec = _CopyString(db);

    return db_filespec;
    }


static _Void  CleanupQueryData(data)
_EnvQueryData data;

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

    if (data->attribute_name != (_String) 0)
	_DeleteString(&data->attribute_name);

    if (data->net_list_name != (_String) 0)
	_DeleteString(&data->net_list_name);

    if (data->active_net_name != (_String) 0)
	_DeleteString(&data->active_net_name);

    if (data->current_net_name != (_String) 0)
	_DeleteString(&data->current_net_name);

    if (data->path_list_name != (_String) 0)
	_DeleteString(&data->path_list_name);

    if (data->active_path_name != (_String) 0)
	_DeleteString(&data->active_path_name);

    if (data->path_index_name != (_String) 0)
	_DeleteString(&data->path_index_name);

    if (data->trail_name != (_String) 0)
	_DeleteString(&data->trail_name);

    _FreeMem(data);

    return;
    }


static _Void  EnvDwLBVersionYes(w, null_tag, reason)
Widget w;
 _WindowPrivate null_tag;

    _Reason reason;

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
    _EnvWindow  env_window;

    EnvDwMessageDismissRootDb();

    env_window = EnvRetDisplayEnvironment(_True);

    /*
    ** Remove the copyright notice - We can't wait until a user action because
    ** the main loop is already started
    */

    if (env_window != (_EnvWindow) _NullObject)
	_RemoveCopyright(env_window, (_String) 0);

    return;
    }
    
