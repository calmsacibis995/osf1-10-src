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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: hunt.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/09/07 15:38:07 $";
#endif
/*
hunt.c	1.5  com/cmd/tip,3.1,9013 12/21/89 16:47:41";
 */
/* 
 * COMPONENT_NAME: UUCP hunt.c
 * 
 * FUNCTIONS: MSGSTR, dead, hunt 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "hunt.c	5.1 (Berkeley) 4/30/85"; */

#include "tip.h"

extern char *getremote();
extern char *rindex();

static	jmp_buf deadline;
static	int deadfl;

void
dead()
{

	deadfl = 1;
	longjmp(deadline, 1);
}

hunt(name)
	char *name;
{
	register char *cp;
	sig_t f;

	f = signal(SIGALRM, dead);
	while (cp = getremote(name)) {
		deadfl = 0;
		uucplock = rindex(cp, '/')+1;
		if (uu_lock(uucplock) < 0) {
			continue;
		}
		/*
		 * Straight through call units, such as the BIZCOMP,
		 * VADIC and the DF, must indicate they're hardwired in
		 *  order to get an open file descriptor placed in FD.
		 * Otherwise, as for a DN-11, the open will have to
		 *  be done in the "open" routine.
		 */
		if (!HW)
			break;
		if (setjmp(deadline) == 0) {
			alarm(10);
			FD = open(cp, O_RDWR);
		}
		alarm(0);
		if (FD < 0) {
			perror(cp);
			deadfl = 1;
		}
		if (!deadfl) {
			ioctl(FD, TIOCEXCL, 0);
			ioctl(FD, TIOCHPCL, 0);
			signal(SIGALRM, SIG_DFL);
			return ((int)cp);
		}
		(void)uu_unlock(uucplock);
	}
	signal(SIGALRM, f);
	return (deadfl ? -1 : (int)cp);
}
