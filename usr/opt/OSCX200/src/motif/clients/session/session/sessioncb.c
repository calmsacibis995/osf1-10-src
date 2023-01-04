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
#include "smdata.h"

int	session_menu_cb(widgetID, tag, reason)
Widget	*widgetID;
caddr_t	tag;
XmRowColumnCallbackStruct	*reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	If the function selected was pause session, then call the
**	pause routine, otherwise call the end session code
**
**  FORMAL PARAMETERS:
**
**	widgetID - The menu widget id
**	tag - A pointer not used
**	reason - A structure which includes the menu item selected.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    if (reason->widget == smdata.pause_button)
	pause_session (widgetID, tag, reason);

    else if (reason->widget == smdata.quit_button)
	end_session (widgetID, tag, reason);
}
