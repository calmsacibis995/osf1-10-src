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
static char	*sccsid = "@(#)$RCSfile: sysexits.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:29:31 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint

#endif 
*/
#include <sysexits.h>

/*
 *  SYSEXITS.C -- error messages corresponding to sysexits.h
 */
char *SysExMsg[] = {
	/* 64 USAGE */		"500 Bad usage",
	/* 65 DATAERR */	"501 Data format error",
	/* 66 NOINPUT */	"550 Cannot open input",
	/* 67 NOUSER */		"550 User unknown",
	/* 68 NOHOST */		"550 Host unknown",
	/* 69 UNAVAILABLE */	"554 Service unavailable",
	/* 70 SOFTWARE */	"554 Internal error",
	/* 71 OSERR */		"451 Operating system error",
	/* 72 OSFILE */		"554 System file missing",
	/* 73 CANTCREAT */	"550 Can't create output",
	/* 74 IOERR */		"451 I/O error",
	/* 75 TEMPFAIL */	"250 Deferred",
	/* 76 PROTOCOL */	"554 Remote protocol error",
	/* 77 NOPERM */		"550 Insufficient permission",
	/* 78 CONFIG */		"554 Local configuration error",
};

int N_SysEx = sizeof(SysExMsg) / sizeof(SysExMsg[0]);

/*
 *  STATSTRING -- return string corresponding to an error status
 *
 *	Parameters:
 *		stat -- the status to decode.
 *
 *	Returns:
 *		The string corresponding to that status
 *
 *	Side Effects:
 *		none.
 */
char *
statstring(stat)
	int stat;
{
	static char ebuf[50];

	stat -= EX__BASE;
	if (stat < 0 || stat >= N_SysEx) {
		(void)sprintf(ebuf, "554 Unknown status %d", stat + EX__BASE);
		return(ebuf);
	}
	return(SysExMsg[stat]);
}
