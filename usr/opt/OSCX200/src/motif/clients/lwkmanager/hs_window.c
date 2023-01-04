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
**	Window object methods
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
#include "hs_window.h"
#include "hs_decwindows.h"



/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

#define MaxWidgetListCount 15

/*
**  Type Definitions
*/

/*
**  Global Data Definitions
*/

_Global _WindowTypeInstance;      /* Instance record for Type */
_Global _WindowType;              /* Type */

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeWindowInstance;
static _WindowPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    WindowPropertyNameTable;

static _Window WindowList = (_Window) _NullObject;

/*
**  External routine declaration for the DECwindows module
*/

_DeclareFunction(_Void EnvDWMessageBox,
    (_WindowPrivate private, _Status status, _Integer count));
_DeclareFunction(_Void EnvDWWindowRaiseWindow,
    (_WindowPrivate private));
 _DeclareFunction(_Void EnvDwLbWindowSetCursor,
    (_WindowPrivate private, _CursorType cursor_type));
_DeclareFunction(_Void EnvDwEnvWindowSetCursor,
    (_WindowPrivate private, _CursorType cursor_type));
_DeclareFunction(_Void EnvDWPositionWindow,
    (_WindowPrivate lb_win, _WindowPrivate env_win, _Integer lb_win_count));

_Void  EnvOpWindow(window)
_Window window;

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
    

_Window  EnvOpWindowCreate(type)
_Type type;

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
    _Window volatile new_window;

    new_window = (_Window) _NullObject;

    _StartExceptionBlock

    /*
    ** Create a Window Object
    */
    
    new_window = (_Window) _Create(type);

    /*
    **  Add the window to the general list
    */

    _BeginCriticalSection

    _NextWindow_of(new_window) = WindowList;

    WindowList = new_window;

    _EndCriticalSection
    
    /*
    **  If any exceptions are raised, delete the new objects then reraise the
    **  exception.
    */

    _Exceptions
	_WhenOthers
	
	    _Delete(new_window);
	    _Reraise;
	    
    _EndExceptionBlock

    /*
    ** Return the object to the caller.
    */

    return new_window;
    }
    
    
_Void  EnvOpWindowInitialize(window, proto)
_Window window;
 _Window proto;

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
    
    _ClearValue(&_XPosition_of(window), hs_c_domain_integer);
    _ClearValue(&_YPosition_of(window), hs_c_domain_integer);
    _ClearValue(&_Width_of(window), hs_c_domain_integer);
    _ClearValue(&_Height_of(window), hs_c_domain_integer);
    _ClearValue(&_DisplayData_of(window), hs_c_domain_any_ptr);

    _StateFlags_of(window) = _InitialWindowStateFlags;

    /*
    **  Invoke the Initialize operation in our supertype
    */
    
    _Initialize_S(window, proto, MyType);

    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_Window) _NullObject) {
    
        _CopyValue(&_XPosition_of(proto), &_XPosition_of(window),
	    hs_c_domain_integer);
	    
        _CopyValue(&_YPosition_of(proto), &_YPosition_of(window),
	     hs_c_domain_integer);
	     
        _CopyValue(&_Width_of(proto), &_Width_of(window), hs_c_domain_integer);

        _CopyValue(&_Height_of(proto), &_Height_of(window),
	    hs_c_domain_integer);

    }

    return;
    }
    

_Void  EnvOpWindowFree(window)
_Window window;

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
    _Window	wnd;
    _Window	prev_window;

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_XPosition_of(window), hs_c_domain_integer);
    _DeleteValue(&_YPosition_of(window), hs_c_domain_integer);
    _DeleteValue(&_Width_of(window), hs_c_domain_integer);
    _DeleteValue(&_Height_of(window), hs_c_domain_integer);
    _DeleteValue(&_DisplayData_of(window), hs_c_domain_any_ptr);

    /*
    ** Remove the window from the list of lb windows
    */

    _BeginCriticalSection

    wnd = WindowList;
    prev_window = wnd;

    /*
    ** Find the window to delete in the list
    */

    while (wnd != (_Window) _NullObject) {
    
	if (wnd == window)
	    break;

	prev_window = wnd;
	wnd = _NextWindow_of(wnd);
    }

    if (prev_window == wnd) 
	WindowList = (_Window) _NullObject;
    else
	_NextWindow_of(prev_window) = _NextWindow_of(wnd);

    _EndCriticalSection	    

    /*
    **  Ask our supertype to free its properties
    */
    
    _Free_S(window, MyType);

    return;
    }
    

_Void  EnvOpWindowGetValue(window, property, domain, value)
_Window window;
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
        _GetValue_S(window, property, domain, value, MyType);
    }
    else {
        /*
        **  Answer depends on the property
        */

        switch (index) {
	
            case _XPositionIndex :
	    
                if (_IsDomain(hs_c_domain_integer, domain))
                    _CopyValue(&_XPosition_of(window), value,
                        hs_c_domain_integer);
                else
                    _Raise(inv_domain);

                break;

            case _YPositionIndex :
	    
                if (_IsDomain(hs_c_domain_integer, domain))
                    _CopyValue(&_YPosition_of(window), value,
                        hs_c_domain_integer);
                else
                    _Raise(inv_domain);

                break;

            case _WidthIndex :
	    
                if (_IsDomain(hs_c_domain_integer, domain))
                    _CopyValue(&_Width_of(window), value,
                        hs_c_domain_integer);
                else
                    _Raise(inv_domain);

                break;

            case _HeightIndex :
	    
                if (_IsDomain(hs_c_domain_integer, domain))
                    _CopyValue(&_Height_of(window), value,
                        hs_c_domain_integer);
                else
                    _Raise(inv_domain);

                break;

            case _DisplayDataIndex:
	    
                if (_IsDomain(hs_c_domain_any_ptr, domain))
                    _CopyValue(&_DisplayData_of(window), value,
                        hs_c_domain_any_ptr);
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
    

_Void  EnvOpWindowSetValue(window, property, domain, value, flag)
_Window window;
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
        _SetValue_S(window, property, domain, value, flag, MyType);
    }
    else {
        /*
        **  Answer depends on the property
        */

        switch (index) {
	
            case _XPositionIndex:
	    
                _SetSingleValuedProperty(&_XPosition_of(window),
                    hs_c_domain_integer, domain, value, flag, _False);
                break;
		
            case _YPositionIndex:
	    
                _SetSingleValuedProperty(&_YPosition_of(window),
                    hs_c_domain_integer, domain, value, flag, _False);
                break;
		
            case _WidthIndex:
	    
                _SetSingleValuedProperty(&_Width_of(window),
                    hs_c_domain_integer, domain, value, flag, _False);
                break;
		
            case _HeightIndex:
	    
                _SetSingleValuedProperty(&_Height_of(window),
                    hs_c_domain_integer, domain, value, flag, _False);
                break;
		
            case _DisplayDataIndex:
	    
                _SetSingleValuedProperty(&_DisplayData_of(window),
                    hs_c_domain_any_ptr, domain, value, flag, _False);
                break;
		
            default :
                _Raise(no_such_property);
                break;
        }
    }

    return;
    }
          

_Void  EnvOpWindowClose(window)
_Window window;

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
    

_Void  EnvOpWindowHighlight(window, hs_object)
_Window window;
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
    **  Let the Svn dependent routine do the work
    */
        
    EnvSvnWindowHighlight(_DisplayData_of(window), hs_object);
    
    return;
    }
    

_Void  EnvOpWindowSelect(window, hs_object)
_Window window;
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
    **  Let the Svn dependent routine do the work
    */
        
    EnvSvnWindowSelect(_DisplayData_of(window), hs_object);
    
    return;
    }
    

_Boolean  EnvOpWindowSetWindowState(window, state, operation)
_Window window;
 _WindowState state;

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
    _StateFlags	    flag;
    _Boolean	    old_state;

    /*
    **  Turn the given State into a Flag
    */

    flag = _StateToFlag(state);

    /*
    **  Get the old State
    */

    old_state = ((_StateFlags_of(window) & flag) != 0);

    /*
    **  Set the State as requested
    */

    switch (operation) {

        case _StateSet :
	
            _StateFlags_of(window) |= flag;
	    
	    if (state == _StateHighlightingOn)
		EnvSvnWindowSetHighlight(_DisplayData_of(window), _True);
		
            break;

        case _StateClear :
	
            _StateFlags_of(window) &= ~flag;
	    
	    if (state == _StateHighlightingOn)
		EnvSvnWindowSetHighlight(_DisplayData_of(window), _False);
		
            break;

        case _StateComplement :
	
            _StateFlags_of(window) ^= flag;
	    
	    if (state == _StateHighlightingOn)
		EnvSvnWindowSetHighlight(_DisplayData_of(window), _True);
		
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

_Boolean  EnvOpWindowGetSelection(window, operation, user_data)
_Window window;
 _GetSelectOperation operation;

    _AnyPtr user_data;

/*
**++
**  Functional Description:
**	Gets the selected objects in the window.
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
**	_True when getselectionoperation can be performed.
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Boolean get_selection = _False;
    _WindowPrivate private;

    _GetValue(window, _P_DisplayData, hs_c_domain_any_ptr,
	&private);
    

    get_selection = EnvSvnWindowGetSelection(private, operation,
		    user_data);
     
    return get_selection;
    }


_Void  EnvOpWindowDisplayMessage(window, status, count)
_Window window;
 _Status *status;
 _Integer count;

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
    **  Let the message module handle the message window
    */

    EnvDWMessageBox(_DisplayData_of(window), status, count);
    
    return;
    }


_Void  EnvOpWindowRaiseWindow(window)
_Window window;

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
    **  Let the DECwindows module handle the request
    */

    EnvDWWindowRaiseWindow(_DisplayData_of(window));

    return;
    }
    

_Void  EnvOpWindowSetCursor(window, cursor_type)
_Window window;
 _CursorType cursor_type;

/*
**++
**  Functional Description:
**	Tell the active Linkbase and Environment windows
**	to set the given cursor type on themselves and
**	their associated windows.
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
    static int SetCursorCount = 0;

    _Window	wnd;

    _BeginCriticalSection

    if (cursor_type == _DefaultCursor)
	SetCursorCount--;
    else
	SetCursorCount++;

    if ( (cursor_type == _DefaultCursor && (SetCursorCount == 0)) ||
	 (cursor_type != _DefaultCursor) ) {

	/*
	**  Run down the list of windows
	*/

	wnd = WindowList;

	while (wnd != (_Window) _NullObject) {
	
	    if (!(_Boolean) _SetWindowState(wnd, _StateReuse, _StateGet)) {

		if (_IsType(wnd, _TypeEnvWindow)) 
		    EnvDwEnvWindowSetCursor(_DisplayData_of(wnd), cursor_type);
		else
		    EnvDwLbWindowSetCursor(_DisplayData_of(wnd), cursor_type);
	    }

	    wnd = _NextWindow_of(wnd);
	}
    }
    
    _EndCriticalSection	    

    return;
    }
    

_Void  EnvOpWindowPositionWindow(window)
_Window window;

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
    _EnvWindow envwin;
    Widget envwidget;
    _Window	wnd;
    _Integer	count = 0;
    _WindowPrivate private;
    _WindowPrivate envprivate;

    /*
    **  Run down the list of windows - we need a count
    */
    
    wnd = WindowList;

    while (wnd != (_Window) _NullObject) {
	count++;
	wnd = _NextWindow_of(wnd);
    }
    count--;

    if (count < 0) count = 0;
    
    /*
    **  Get the environment window
    */

    envwin = EnvOpWindowGetEnvWindow(window);

    _GetValue(envwin, _P_DisplayData, hs_c_domain_any_ptr,
	&envprivate);
    
    /*
    **  Position the window with respect to the environment window
    */

    EnvDWPositionWindow((_WindowPrivate)(_DisplayData_of(window)),
	envprivate, count);

    return;
    }

_EnvWindow  EnvOpWindowGetEnvWindow(window)
_Window window;

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
    _Window	wnd;

    _BeginCriticalSection

    wnd = WindowList;

    while (wnd != _NullObject) {

	if (_IsType(wnd, _TypeEnvWindow))
	    return ((_EnvWindow) (wnd));

	wnd = _NextWindow_of(wnd);
    }

    _EndCriticalSection

    return ((_EnvWindow) (wnd));
    }
    
