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
**	Linkbase Window object methods
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
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_lbwindow.h"
#include "hs_decwindows.h"

/*
**  Table of Contents
*/

_DeclareFunction(static _Void EnvLbWindowSetProperties,
    (_LbWindow lbwindow, _Cardinal width, _Cardinal height, _Position x,
     _Position y, _Widget parent));

/*
**  Macro Definitions
*/

#define _SetWindowProperties(window, width, height, x, y, parent) \
    EnvLbWindowSetProperties((window), (width), (height), (x), (y), (parent));

/*
**  Type Definitions
*/

/*
**  Global Data Definitions
*/

_Global _LbWindowTypeInstance;      /* Instance record for Type */
_Global _LbWindowType;              /* Type */

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeLbWindowInstance;
static _LbWindowPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    LbWindowPropertyNameTable;

static _LbWindow LbWindowList = (_LbWindow) _NullObject;

/*
**  External function declarations
*/

_DeclareFunction(_Void EnvSvnLbWindowClear, (_WindowPrivate private));
_DeclareFunction(_Void EnvSvnLbWindowUpdate,
    (_WindowPrivate private, _HsObject hs_object, _UpdateOperation update));
    

_Void  EnvOpLbWindow(lbwindow)
_LbWindow lbwindow;

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
    

_LbWindow  EnvOpLbWindowCreate(type, width, height, x, y, parent)
_Type type;
 _Cardinal width;
 _Cardinal height;

    _Position x;
 _Position y;
 _Widget parent;

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
    _LbWindow volatile new_lbwindow;

    new_lbwindow = (_LbWindow) _NullObject;

    _StartExceptionBlock

    /*
    ** Create the LbWindow Object
    */
    
    new_lbwindow = (_LbWindow) _CreateWindow(MyType);

    /*
    **  If any exceptions are raised, delete the new objects then reraise the
    **  exception.
    */

    _Exceptions
	_WhenOthers
	
	    _Delete(new_lbwindow);
	    _Reraise;
	    
    _EndExceptionBlock

    /*
    ** Return the object to the caller.
    */

    return new_lbwindow;
    }
    

_LbWindow  EnvOpLbWindowOpen(type, width, height, x, y, parent)
_Type type;
 _Cardinal width;
 _Cardinal height;

    _Position x;
 _Position y;
 _Widget parent;

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
    _LbWindow	lbwindow;
    _Boolean	state;

    _StartExceptionBlock

    /*
    **  Check if we can reuse an existing window
    */

    _BeginCriticalSection

    lbwindow = LbWindowList;

    while (lbwindow != (_LbWindow) _NullObject) {
    
	if ((_Boolean) _SetWindowState(lbwindow, _StateReuse, _StateGet)) {
	    _SetWindowState(lbwindow, _StateReuse, _StateClear);
	    break;
	}

	lbwindow = _Next_of(lbwindow);
    }

    _EndCriticalSection	    

    /*
    **  If we found a window, then make it the right size and position
    */

    if (lbwindow != (_LbWindow) _NullObject) {

	EnvOpDWLbWindowOpen(_PrivateData_of(lbwindow), width, height, x, y);
    }

    else {

	/*
	**  Create a new linkbase window
	*/
	
	lbwindow = (_LbWindow) _CreateLbWindow(type, width, height, x, y,
	    parent); 

	/*
	** Allocate the private data
	*/
	
	_PrivateData_of(lbwindow) =
	    (_AnyPtr) EnvOpDWLbWindowCreate(lbwindow, parent);
							     
	/*
	**  Add the window to the list
	*/

	_BeginCriticalSection

	_Next_of(lbwindow) = LbWindowList;

	LbWindowList = lbwindow;

	_EndCriticalSection
    
    }

    _SetWindowProperties(lbwindow, width, height, x, y, parent);

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[1] = _StatusCode(wnd_open_error);
	    status[2] = _Others;

	     _DisplayRootMessage(status, 2, hs_c_fatal_message);
	
    _EndExceptionBlock

    return lbwindow;
    }
    
    
_Void  EnvOpLbWindowInitialize(lbwindow, proto)
_LbWindow lbwindow;
 _LbWindow proto;

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
    
    _ClearValue(&_Linkbase_of(lbwindow), hs_c_domain_hsobject);
    _PrivateData_of(lbwindow) = (_AnyPtr) _NullObject;

    /*
    **  Invoke the Initialize operation in our supertype
    */
    
    _Initialize_S(lbwindow, proto, MyType);

    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_LbWindow) _NullObject) {
        _CopyValue(&_Linkbase_of(proto), &_Linkbase_of(lbwindow),
	    hs_c_domain_hsobject);
    }

    return;
    }


_Void  EnvOpLbWindowClose(lbwindow, recycle)
_LbWindow lbwindow;
 _Boolean recycle;

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
    lwk_linkbase  linkbase;

    /*
    **  Let the DECwindows module close (pop-down) the window
    */

    EnvOpDWLbWindowClose(_PrivateData_of(lbwindow));

    /*
    **  Clear the window's content
    */

    _Clear(lbwindow);

    /*
    ** Close the linkbase
    */

    _GetValue(_Linkbase_of(lbwindow), _P_HisObject, hs_c_domain_lwk_object,
	&linkbase);

    status = lwk_close(linkbase);
    if (status != lwk_s_success)
	_Raise(close_lb_err);

    status = lwk_delete(&linkbase);
    _ClearValue(&_Linkbase_of(lbwindow), hs_c_domain_hsobject);

    if (recycle)

	/*
	**  Set the window for reuse
	*/

	_SetWindowState(lbwindow, _StateReuse, _StateSet);

    else {

        /*
	** Delete the window 
	*/

	_Free(lbwindow);
    }

    return;
    }
    

_Void  EnvOpLbWindowFree(lbwindow)
_LbWindow lbwindow;

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
    _LbWindow	    window;
    _LbWindow	    prev_window;

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_Linkbase_of(lbwindow), hs_c_domain_hsobject);

    /*
    **  Free the private storage used by our Type.
    */

    EnvOpDWLbWindowDelete(_PrivateData_of(lbwindow));

    /*
    ** Remove the window from the list of lb windows
    */

    _BeginCriticalSection

    window = LbWindowList;
    prev_window = window;

    /*
    ** Find the window to delete in the list
    */

    while (window != (_LbWindow) _NullObject) {
    
	if (window == lbwindow)
	    break;

	prev_window = window;
	window = _Next_of(window);
    }

    if (prev_window == window) 
	LbWindowList = (_LbWindow) _NullObject;
    else
	_Next_of(prev_window) = _Next_of(window);

    _EndCriticalSection	    

    /*
    **  Ask our supertype to free its properties
    */
    
    _Free_S(lbwindow, MyType);

    return;
    }
    

_Void  EnvOpLbWindowGetValue(lbwindow, property, domain, value)
_LbWindow lbwindow;
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
        _GetValue_S(lbwindow, property, domain, value, MyType);
    }
    else {
        /*
        **  Answer depends on the property
        */

        switch (index) {
	
            case _LinkbaseIndex :
                if (_IsDomain(hs_c_domain_hsobject, domain))
                    _CopyValue(&_Linkbase_of(lbwindow), value,
                        hs_c_domain_hsobject);
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

_Void  EnvOpLbWindowSetValue(lbwindow, property, domain, value, flag)
_LbWindow lbwindow;
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
        _SetValue_S(lbwindow, property, domain, value, flag, MyType);
    }
    else {
        /*
        **  Answer depends on the property
        */

        switch (index) {
	
            case _LinkbaseIndex :
                _SetSingleValuedProperty(&_Linkbase_of(lbwindow),
                    hs_c_domain_hsobject, domain, value, flag, _False);
                break;
		
            default :
                _Raise(no_such_property);
                break;
        }
    }

    return;
    }
          

_Void  EnvOpLbWindowDisplay(lbwindow, linkbase)
_LbWindow lbwindow;
 _HsObject linkbase;

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
    **	Set the linkbase property of the window
    */
    
    _SetValue(lbwindow, _P_Linkbase, hs_c_domain_hsobject, &linkbase,
	hs_c_set_property);

    /*
    **	Display the linkbase contents in the window
    */
    
    EnvOpDWLbWindowDisplay(_PrivateData_of(lbwindow), linkbase);
    
    return;
    }
    

_Void  EnvOpLbWindowExpand(lbwindow, user_data)
_LbWindow lbwindow;
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

    _GetValue(lbwindow, _P_DisplayData, hs_c_domain_any_ptr,
	&private);
    
    EnvSvnLbWindowExpand(private, user_data);
    
    return;
    }
    

_Void  EnvOpLbWindowCollapse(lbwindow, user_data)
_LbWindow lbwindow;
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

    _GetValue(lbwindow, _P_DisplayData, hs_c_domain_any_ptr,
	&private);
    
    EnvSvnWindowCollapse(private, user_data);

    return;
    }
    

_Boolean  EnvOpLbWindowLbInWindow(window, linkbase)
_LbWindow *window;

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
    _Boolean	    is_in_window;
    _HsObject	    hs_linkbase;
    lwk_linkbase  current_lb;
    
    is_in_window = _False;
    
    /*
    **  Check arguments
    */

    if (linkbase == (lwk_linkbase) _NullObject)
	_Raise(inv_object);

    /*
    **  Run down the list of window
    */

    _BeginCriticalSection

    *window = LbWindowList;

    while (*window != (_LbWindow) _NullObject) {

	/*
	**  We only look at active windows
	*/
    
	if (!(_Boolean) _SetWindowState(*window, _StateReuse, _StateGet)) {

	    _GetValue(*window, _P_Linkbase, hs_c_domain_hsobject,
		&hs_linkbase);

	    _GetValue(hs_linkbase, _P_HisObject, hs_c_domain_lwk_object,
		&current_lb);

	    if (linkbase == current_lb) {
		is_in_window = _True;
		break;
	    }
	}
    
	*window = _Next_of(*window);
    }
    
    _EndCriticalSection    

    return (is_in_window);
    }
    

static _Void  EnvLbWindowSetProperties(lbwindow, width, height, x, y, parent)
_LbWindow lbwindow;
 _Cardinal width;

    _Cardinal height;
 _Position x;
 _Position y;
 _Widget parent;

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
    ** Set the public properties
    */
    
    _SetValue(lbwindow, _P_XPosition, hs_c_domain_integer, &x,
	hs_c_set_property);
	
    _SetValue(lbwindow, _P_YPosition, hs_c_domain_integer, &y,
	hs_c_set_property);
	
    _SetValue(lbwindow, _P_Width, hs_c_domain_integer, &width,
	hs_c_set_property);
	
    _SetValue(lbwindow, _P_Height, hs_c_domain_integer, &height,
	hs_c_set_property);
	
    return;
    }
    

_Void  EnvOpLbWindowUpdate(window, hs_object, update)
_LbWindow window;
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

    EnvSvnLbWindowUpdate(_PrivateData_of(window), hs_object, update);

    return;
    }
    

_Void  EnvOpLbWindowClear(window)
_LbWindow window;

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
    **  The Svn module will clear the window's content
    */

    EnvSvnLbWindowClear(_PrivateData_of(window));

    return;
    }
    
