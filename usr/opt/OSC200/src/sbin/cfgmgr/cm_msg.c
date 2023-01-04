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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: cm_msg.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:32:55 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 */

#include "cfgmgr.h"
#include "cm.h"
#include "cm_msgstr.h"

#ifdef MSG
#define MSGSTR(Num, Str) 	catgets(catd, MS_CFGMGR, Num, Str)
#else
#define MSGSTR(Num, Str) 	Str
#endif

/*
 *	CMGR:	MSG conversion
 */

char *
cm_msg( int msg_id )
{
#ifdef MSG
	static nl_catd	catd;
#endif
	extern char *	cmgr_msgstr[];
	static int	maxerr;
	static int	initialized = FALSE;

	if (initialized == FALSE) {
		initialized = TRUE;
		for ( maxerr=0; cmgr_msgstr[maxerr] != NULL; maxerr++)
		;
#ifdef MSG
        	catd = catopen(MF_CFGMGR, MS_CFGMGR);
#endif
	}

	if (msg_id < 0 || msg_id >= maxerr)
		msg_id = MSG_MAX;

	return(MSGSTR(msg_id, cmgr_msgstr[msg_id]));
}

