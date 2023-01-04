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
static char *rcsid = "@(#)$RCSfile: psecerror.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:25:08 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	psecerror.c,v $
 * Revision 1.1.1.1  92/03/07  00:44:00  devrcs
 * *** OSF1_1B23 version ***
 * 
 * Revision 1.8  1991/08/16  09:43:59  devrcs
 * 	Fixed an 1.1 i18n bug: removed the usage of NLgetamsg
 * 	[91/08/13  03:04:10  aster]
 *
 * Revision 1.7  91/03/04  17:45:06  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:57:51  lehotsky]
 * 
 * Revision 1.6  91/01/07  15:59:30  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:30:21  dwm]
 * 
 * Revision 1.5  90/10/07  20:08:27  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:16:52  gm]
 * 
 * Revision 1.4  90/08/09  14:24:33  devrcs
 * 	Changes for widened types: make ushorts into longwords.
 * 	[90/08/02  15:37:05  seiden]
 * 
 * Revision 1.3  90/07/27  10:32:21  devrcs
 * 	Fixed the initialization i18n problem
 * 	[90/07/17  08:44:14  staffan]
 * 
 * 	Any function using NLS messages will now open the catalog
 * 	[90/07/16  09:57:09  staffan]
 * 
 * Revision 1.2  90/07/17  12:21:02  devrcs
 * 	Internationalized
 * 	[90/07/05  07:28:55  staffan]
 * 
 * 	Initial version from SecureWare
 * 	[90/06/26  09:37:05  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved.
 *
 * Library routine to print security error message.
 * If security error is not defined, print regular error number instead.
 */

/* #ident "@(#)psecerror.c	1.1 22:32:38 5/28/90 SecureWare, Inc." */

#include "libsecurity.h"

#define MESSAGELENGTH	32

void
psecerror(s)
	char *s;
{
	char unknown_buf[MESSAGELENGTH];
	unsigned n;
	char *c;
	extern int sys_nsecerr;
	extern int sec_errno;
	extern char *sys_secerrlist[];
	extern int sys_secerrnums[];
	
	if (sec_errno == 0) {
		perror(s);
		return;
	}

	if (sec_errno < sys_nsecerr)
		c = MSGSTR(sys_secerrnums[sec_errno],
				sys_secerrlist[sec_errno]);
	else {
		c = unknown_buf;
		sprintf(c, MSGSTR(PSECERROR_1, "Security error %3d occurred."),
			sec_errno);
	}

	if (s && (n = strlen(s))) {
		write(2, s, n);
		write(2, ": ", 2);
	}

	write(2, c, strlen(c));
	write(2, "\n", 1);
}
