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
static char	*sccsid = "@(#)$RCSfile: ltnprintf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:45:03 $";
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

#include <stdio.h>

extern int ltpid, ltvflag, ltqflag;	/* see ltgetopt() */
extern char *ltprogramname;

/*VARARGS10*/
ltnprintf(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
	char *fmt;
	int arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10;
{
	if (!ltqflag) {
		(void)printf("STATUS(%s:%d): ", ltprogramname, ltpid);
		(void)printf(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7,
			arg8, arg9, arg10);
		fflush(stdout);	/* we're testing - don't keep us in suspense */
	}
}
