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
#ifndef _Xp_h_
#define _Xp_h_
#include <X11/Xp/COPY>

/*
* SCCS_data: @(#) Xp.h	1.2 92/03/18 15:16:51
*
*	This module contains declarations useful to clients of the
*	Xp library.
*******************************************************************************
*/

#include <X11/Wc/WcCreate.h>	/* for _() macro */

/* These are identical: multiple names for backward compatibility
*/
void XpRegisterAthena	_(( XtAppContext ));
void XpRegisterAll	_(( XtAppContext ));
void AriRegisterAthena	_(( XtAppContext ));

/* These are identical: multiple names for backward compatibility
*/
Widget XpCreateSimpleMenu _(( Widget, String, Arg*, Cardinal ));
Widget WcCreateSimpleMenu _(( Widget, String, Arg*, Cardinal ));

#endif /* _Xp_h_ */
