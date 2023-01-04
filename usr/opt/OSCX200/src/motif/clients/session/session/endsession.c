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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/session/session/endsession.c,v 1.1.2.2 92/12/07 11:32:39 Don_Haney Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include "X11/Intrinsic.h"

#include "smdata.h"
#include "smconstants.h"

extern OptionsRec options;
extern char smdb_file[];

#ifdef HYPERHELP
extern void help_error();
#endif


int	end_session(widgetID, tag, reason)
Widget	*widgetID;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Depending on customization settings, prompt the user with a
**	confirmation box, or simply end the session.
**
**  FORMAL PARAMETERS:
**
**	widgetID - the end session menu button id
**	tag - nothing for now
**	reason - a button pressed reason
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
    int	result;

    /* get the current position of the control panel */
    move_event();

    /* if any of the setup features have been changed, but not saved, then
       alert the user. */

    if (smdata.resource_changed && ! options.nocautions) {
        caution_display(resource_changed_number);

        if (smdata.fcaution == cancel) return;

        if (smdata.fcaution == yes) sm_save_database();
    } else {
        if (smsetup.end_confirm != 0) {
            caution_display(end_session_number);
            if (smdata.fcaution == cancel) return;
        }
    }
#ifdef HYPERHELP
    DXmHelpSystemClose(help_context, help_error, "Help System Error");
#endif
    result = unlink(smdb_file);
    unixrundown(widgetID);
}
