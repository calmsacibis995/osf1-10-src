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
**	HyperSession
**
**  Version: V1.0
**
**  Abstract:
**	Environment Window object methods
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	André Pavanello
**	Patricia Avigdor
**
**  Creation Date: 9-Nov-89
**
**  Modification History:
**	X0.7-1	23-Nov-89   ap	Add SetCurrency operation
**	X0.8-1	12-Dec-89   ap	Add open method
**
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_envwindow.h"
#include "hs_decwindows.h"

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
**  Forward Routine Declarations
*/

/*
**  Global Data Definitions
*/

_Global _EnvWindowTypeInstance;      /* Instance record for Type */
_Global _EnvWindowType;              /* Type */

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeEnvWindowInstance;
static _EnvWindowPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    EnvWindowPropertyNameTable;

static _EnvWindow EnvWindowList = (_EnvWindow) _NullObject;

/*
**  External routine declarations
*/

_DeclareFunction(_Void EnvDWEnvWindowRemoveCopyright,
    (_EnvWindow envwindow, _String title_id));
    
_DeclareFunction(_WindowPrivate EnvOpDWEnvWindowCreate,
    (_EnvWindow envwindow, _EnvContext env_context, _Widget parent));
    
_DeclareFunction(_Void EnvSvnEnvWindowUpdate,
    (_WindowPrivate private, _HsObject hs_object, _UpdateOperation update));

_DeclareFunction(_Void EnvSetCurrencies,
    (_EnvWindow env_window, _EnvContext env_context));


_Void  EnvOpEnvWindow(envwindow)
_EnvWindow envwindow;

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
    

_EnvWindow  EnvOpEnvWindowCreate(type, parent, env_context)
_Type type;
 _Widget parent;

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
    _EnvWindow volatile new_envwindow;

    /*
    ** Create an object instance.
    */
    
    new_envwindow = (_EnvWindow) _NullObject;

    _StartExceptionBlock

    /*
    ** Create the EnvWindow Object.
    */
    
    new_envwindow = (_EnvWindow) _CreateWindow(MyType);

    /*
    ** Allocate the private data.
    */
    
    _PrivateData_of(new_envwindow) =
	(_AnyPtr) EnvOpDWEnvWindowCreate(new_envwindow, env_context, parent);

    /*
    **  If any exceptions are raised, delete the new objects then reraise the
    **  exception.
    */

    _Exceptions
	_WhenOthers
	
	    _Delete(&new_envwindow);
	    new_envwindow = _NullObject;
	    
	    _Reraise;
	    
    _EndExceptionBlock

    /*
    ** Return the object to the caller.
    */

    return new_envwindow;
    }
    

_EnvWindow  EnvOpEnvWindowOpenEnv(type, parent, env_context, new_ctxt)
_Type type;
 _Widget parent;

    _EnvContext env_context;
 _Boolean new_ctxt;

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
    _EnvWindow	envwindow;
    _Boolean	state;

    _StartExceptionBlock

    /*
    **  Check if we can reuse an existing window. Note that we don't allow to
    **	have multiple environment window for V1. This is just here to allow for
    **	future extensions
    */

    _BeginCriticalSection

    envwindow = EnvWindowList;

    while (envwindow != (_EnvWindow) _NullObject) {
    
	if ((_Boolean) _SetWindowState(envwindow, _StateReuse, _StateGet)) {
	    _SetWindowState(envwindow, _StateReuse, _StateClear);
	    break;
	}

	envwindow = _Next_of(envwindow);
    }

    _EndCriticalSection	    

    /*
    **  If we found a window, then make it the right size -- Noop for V1 --
    */

    if (envwindow != (_EnvWindow) _NullObject) {
	_Raise(not_yet_impl);
    }

    else {

	/*
	**  Create a new environment window
	*/
	
	envwindow = (_EnvWindow) _CreateEnvWindow(type, parent, env_context);

	/*
	**  If a new repository has been created tell the window to display a
	**  message
	*/
	
	if (new_ctxt)
	    _SetWindowState(envwindow, _StateMessageSet, _StateSet);    

	/*
	**  Now that we have the window set the env context property
	*/
	
	_SetValue(env_context, _P_EnvWindow, hs_c_domain_environment_wnd,
	    &envwindow, hs_c_set_property);

	/*
	**  Set the currency on the window
	*/

	EnvSetCurrencies(envwindow, env_context);

	/*
	**  Load the environment attributes
	*/

	_ResetAttributes(env_context);
	    	    
	/*
	**  Add the window to the list
	*/

	_BeginCriticalSection

	_Next_of(envwindow) = EnvWindowList;

	EnvWindowList = envwindow;

	_EndCriticalSection
    }
    
    /*
    **  If any exceptions are raised, delete the new objects then reraise the
    **  exception.
    */

    _Exceptions
	_WhenOthers
	    _Status status[3];

	    status[0] = _StatusCode(startup_failed);
	    status[1] = _StatusCode(wnd_open_error);
	    status[2] = _Others;

	    _DisplayRootMessage(status, 3, hs_c_fatal_message);

    _EndExceptionBlock
    
    return envwindow;
    }
    
    
_Void  EnvOpEnvWindowClose(envwindow, recycle)
_EnvWindow envwindow;
 _Boolean recycle;

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

    /*
    **  Let the DECwindows module close the window
    */

    EnvOpDWEnvWindowClose(_PrivateData_of(envwindow));

    if (recycle)

	/*
	**  Mark the window for reuse
	*/

	_SetWindowState(envwindow, _StateReuse, _StateSet);

    else
	_Free(envwindow);
	
    return;
    }

    
_Void  EnvOpEnvWindowInitialize(envwindow, proto)
_EnvWindow envwindow;
 _EnvWindow proto;

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
    /*
    **  Initialize the properties defined by our type
    */
    
    _ClearValue(&_EnvironmentContext_of(envwindow), hs_c_domain_environment_ctxt);
    _PrivateData_of(envwindow) = (_AnyPtr) _NullObject;

    /*
    **  Invoke the Initialize operation in our supertype
    */
    
    _Initialize_S(envwindow, proto, MyType);

    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_EnvWindow) _NullObject) {
        _CopyValue(&_EnvironmentContext_of(proto),
	    &_EnvironmentContext_of(envwindow), hs_c_domain_environment_ctxt);
    }

    return;
    }


_Void  EnvOpEnvWindowFree(envwindow)
_EnvWindow envwindow;

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
    _EnvWindow	window;
    _EnvWindow	prev_window;

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_EnvironmentContext_of(envwindow),
	hs_c_domain_environment_ctxt);
    
    /*
    **  Free the private storage used by our Type.
    */

    EnvOpDWEnvWindowDelete(_PrivateData_of(envwindow));

    /*
    ** Remove the window from the list of rep windows
    */

    _BeginCriticalSection

    window = EnvWindowList;
    prev_window = window;

    /*
    ** Find the window to delete in the list
    */

    while (window != (_EnvWindow) _NullObject) {
    
	if (window == envwindow)
	    break;

	prev_window = window;
	window = _Next_of(window);
    }

    if (prev_window == window) 
	EnvWindowList = (_EnvWindow) _NullObject;
    else
	_Next_of(prev_window) = _Next_of(window);

    _EndCriticalSection	    

    /*
    **  Ask our supertype to free its properties
    */
    
    _Free_S(envwindow, MyType);

    return;
    }


_Void  EnvOpEnvWindowGetValue(envwindow, property, domain, value)
_EnvWindow envwindow;
 _String property;

    _Domain domain;
 _AnyPtr value;

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
    int index;

    /*
    **  See if the property is defined by our type.  If not, pass the request
    **  on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
        _GetValue_S(envwindow, property, domain, value, MyType);
    }
    
    else {
    
        /*
        **  Answer depends on the property
        */

        switch (index) {
	
            case _EnvironmentContextIndex :
                if (_IsDomain(hs_c_domain_environment_ctxt, domain))
                    _CopyValue(&_EnvironmentContext_of(envwindow), value,
                        hs_c_domain_environment_ctxt);
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

_Void  EnvOpEnvWindowSetValue(envwindow, property, domain, value, flag)
_EnvWindow envwindow;
 _String property;

    _Domain domain;
 _AnyPtr value;
 _SetFlag flag;

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
    int index;

    /*
    **  See if the property is defined by our type.  If not, pass the request
    **  on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
        _SetValue_S(envwindow, property, domain, value, flag, MyType);
    }
    
    else {
    
        /*
        **  Answer depends on the property
        */

        switch (index) {
	
            case _EnvironmentContextIndex :
                _SetSingleValuedProperty(&_EnvironmentContext_of(envwindow),
                    hs_c_domain_environment_ctxt, domain, value, flag, _False);
                break;
		
            default :
                _Raise(no_such_property);
                break;
        }
    }

    return;
    }
          

_Void  EnvOpEnvWindowDisplay(envwindow, environment_context)
_EnvWindow envwindow;
 _EnvContext environment_context;

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

    _StartExceptionBlock

    /*
    **	Set the environment context property of the window
    */
    
    _SetValue(envwindow, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
		&environment_context, hs_c_set_property);

    /*
    **	Display the environment context in the window
    */
    
    EnvOpDWEnvWindowDisplay(_PrivateData_of(envwindow), environment_context);

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(wnd_disp_error);
	    status[1] = _Others;
	    
	    _DisplayMessage(envwindow, status, 2);
	    
    _EndExceptionBlock

    return;
    }
    

_Void  EnvOpEnvWindowSetCurrency(envwindow, currency, value)
_EnvWindow envwindow;
 _CurrencyFlag currency;

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

    /*
    **  Let the DECwindows dependent module do the work
    */

    EnvOpDWEnvWindowSetCurrency(_PrivateData_of(envwindow), currency, value);

    return;
    }


_Void  EnvOpEnvWindowExpand(envwindow, user_data)
_EnvWindow envwindow;
 _AnyPtr user_data;

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
    _WindowPrivate private;

    _GetValue(envwindow, _P_DisplayData, hs_c_domain_any_ptr, &private);
    
    EnvSvnEnvWindowExpand(private, user_data);
    
    return;
    }
    

_Void  EnvOpEnvWindowCollapse(envwindow, user_data)
_EnvWindow envwindow;
 _AnyPtr user_data;

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
    _WindowPrivate private;

    _GetValue(envwindow, _P_DisplayData, hs_c_domain_any_ptr, &private);
    
    EnvSvnWindowCollapse(private, user_data);

    return;
    }
    

_Void  EnvOpEnvWindowUpdate(window, hs_object, update)
_EnvWindow window;
 _HsObject hs_object;

    _UpdateOperation update;

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
    **  Call the Svn dependent module which is going to do the actual update
    */

    EnvSvnEnvWindowUpdate(_PrivateData_of(window), hs_object, update);

    return;
    }
    

_AnyPtr  EnvEnvWindowGetEnvWindowPrivate()
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
    _EnvWindow	envwindow;

    _BeginCriticalSection

    envwindow = EnvWindowList;

    _EndCriticalSection
    
    return (_PrivateData_of(envwindow));
    }
    
    

_Void  EnvOpEnvWindowRemoveCopyright(window, title_id)
_EnvWindow window;
 _String title_id;

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

    EnvDWEnvWindowRemoveCopyright(_PrivateData_of(window), title_id);

    return;
    }
    
