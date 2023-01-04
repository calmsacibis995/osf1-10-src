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
** COPYRIGHT (c) 1991 BY
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
**  Facility:
**
**	LinkWorks Services
**
**  Abstract:
**
**	Public LinkWorks Services that are specific to DECwindows Motif
**
**  Author:
**	W. Ward Clark, MEMEX Project
**
**  Creation Date:  26-Feb-91
**
**  Modification History:
**  X0.16   WWC  26-Feb-91  DXm routines extracted from HIS_PUBLIC_INTERFACE.C
**--
*/

/*
**  Include Files
*/
#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_dwui_decwindows_m.h"

lwk_status lwk_entry  lwk_create_dxm_ui(appl_name, create_menu, default_accelerators, main_window, connection_menu, dwui)
XmString appl_name;

    lwk_boolean create_menu;
 lwk_boolean default_accelerators;

    Widget main_window;
 Widget connection_menu;
 lwk_dxm_ui_ptr dwui;

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
    **  Check the validity of the arguments.
    */

    if (dwui == (lwk_dxm_ui_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_DXmUiPtr) dwui) = (_DXmUi) _CreateDXmUi(_TypeDXmUi,
	(_CompoundString) appl_name, (_Boolean) create_menu,
	(_Boolean) default_accelerators, (_Widget) main_window,
	(_Widget) connection_menu);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }

lwk_status lwk_entry  lwk_do_dxm_menu_action(object, menu)
lwk_object object;

    lwk_dxm_menu_action menu;

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
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _SelectMenu((_Object) object, (_Menu) menu, (_Closure) 0);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }
