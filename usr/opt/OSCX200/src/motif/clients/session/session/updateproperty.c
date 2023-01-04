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
#include <stdio.h>
#define SS_NORMAL 1
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"

/*
 * prototype
 */
char **add_app_commands();


static	unsigned    int	first = 0;

int	sm_change_property(display)
Display		*display;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Puts the property on the root window
**
**  FORMAL PARAMETERS:
**
**	display - The display connection number
**
**  IMPLICIT INPUTS:
**
**	xrmdb	- The database which we are going to use to
**			   create the property
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	If we fail to store the property the session manage will
**	exit.
**
**--
**/
{
unsigned    int	status;

/* Put the property on the root window. */
if (first == 0)
    {
    removelist = get_remove_list(&removecount);
    removelist = add_app_commands(display_id, 
		xrmdb.xrmlist[system_color_type][rdb_merge],
		removelist, &removecount);
    }
first = 1;
status = store_properties(display, xrmdb,removelist,removecount);
if (status != SS_NORMAL)
    {
    put_error(status, k_sm_putproperty_msg);
    exit(1);
    }
}
