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
 *   24 May 1990 - Jeffrey Mogul/DECWRL
 *      created file
 *	Copyright (c) 1990 by Digital Equipment Corporation
 *
*/

#include "gwscreen.h"
#include <sys/kernel.h>
#include <sys/errno.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/gw_screen.h>

#if	NGWSCREEN == 0

/* global data */
int gw_screen_on = SCREENMODE_OFF;		/* forever */

void
gw_forwardscreen(pkt, srcrt, af, fwdproc, errorproc)
	struct mbuf *pkt;
	int srcrt;
	int af;
	void (*fwdproc)();
	void (*errorproc)();
{
	/* always forward */
	(*fwdproc)(pkt, srcrt);
}

void
gw_screen_init()
{
}

int
screen_control(so, cmd, data) 
	struct socket *so;
	int cmd;
	caddr_t data;
{
	return(EOPNOTSUPP);
}

#else

/* SCREEN_MAXPEND can be changed if necessary */
#define SCREEN_MAXPEND 32	/* default max # of packets pending */
int screen_maxpend = SCREEN_MAXPEND;

#endif	/* NGWSCREEN */
