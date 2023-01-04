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
**	HyperSession HyperInformation support routinmes
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Andr Pavanello
**
**  Creation Date: 21-Nov-89
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

/*
**  Type Definitions
*/

/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_Void  EnvHis(window_prv)
_WindowPrivate window_prv;

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


lwk_dxm_ui  EnvHisCreateDwUi(window_prv, title)
_WindowPrivate window_prv;
 _String title;

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
    lwk_string	    string;
    lwk_integer	    integer;
    lwk_callback    callback;
    lwk_dxm_ui	    dwui_object;
    lwk_boolean	    env_manager;
    XmString	    cs_name;

    /*
    **  Create a His DwUi object associated with a given window
    */

    cs_name = _StringToXmString((char *) title);

    /*
    ** Without the Memex menu
    */
    status = lwk_create_dxm_ui((lwk_any_pointer) cs_name, lwk_c_false,
	lwk_c_false, window_prv->main_widget, window_prv->connection_menu,
	&dwui_object);
    
/*
**    status = lwk_create_dxm_ui((lwk_any_pointer) cs_name, lwk_c_true,
**        lwk_c_false, window_prv->main_widget, window_prv->connection_menu,
**        &dwui_object);
*/

    XmStringFree(cs_name);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(dwui_creation_failed);
    }

    /*
    **  Set the environment manager attribute to True
    */

    env_manager = lwk_c_true;

    status = lwk_set_value(dwui_object, lwk_c_p_environment_manager,
	lwk_c_domain_boolean, (lwk_any_pointer) &env_manager, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }

    /*
    **  Set the $SupportedSurrogateTypes property of the DwUi
    */


    /*
    **  Set the $SupportedOperations property of the DwUi (View, Edit)
    */


    /*
    **  Set the $UserData property of the DwUi (the window object)
    */


    /*
    **  Set the callback properties of the DwUi
    */


    /*
    **  Set the $CurrentHighlighting property of the DwUi to be the
    **  $DefaultHighlighting of the DwUi.
    */

    return dwui_object;
    }

