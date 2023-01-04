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
static char	*sccsid = "@(#)$RCSfile: tty_def.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/10/12 20:28:24 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
/*
 *	@(#)tty_common.c		1/21/91
 *	
 *	1-21-91	Create file	Kuo-Hsiung Hsieh
 */

/*
 *  tty common default setting. 
 */
/*
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/mount.h>	*/		/* funnelling through mnt struct */
/*
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/ioctl.h>
*/
#include <sys/tty.h>
#include <sys/ttydefaults.h>
#include <kern/xpr.h>
/*
#include <sys/proc.h>
#include <sys/uio.h>
#include <kern/parallel.h>
*/

/* default settings for ULTRIX/BSD 
 * The settings have to be consistent with ttcompatsetflags() COOKED
 * mode.
 */
/* ultrix strip char to seven bits as default ??? ISTRIP
 * what parity ultrix used ??? */
/* ultrix does not care about parity.  No parity check is done 
   INPCK is clear in iflag */
/*
#define BSD_IFLAG (BRKINT|IGNPAR|ISTRIP|IXON|IXANY)
*/
#define BSD_IFLAG (BRKINT|IGNPAR|ISTRIP|IXON|IXANY|IMAXBEL)
#define BSD_OFLAG (OPOST)
#define BSD_CFLAG (PARENB|CREAD|CS7)
/*
#define BSD_LFLAG (ISIG|ICANON)
*/
#define BSD_LFLAG (ISIG|ICANON|IEXTEN)
#define TERMIO_ONLY_IFLAG 	\
	(IGNBRK|IGNPAR|PARMRK|INLCR|IGNCR|BRKINT|INPCK|ISTRIP |IXON)
#define TERMIO_ONLY_OFLAG  (ONOCR|ONLRET|OFILL|OFDEL|OCRNL)
/*
#define TERMIO_ONLY_CFLAG  (CREAD|PARENB|LOBLK)
*/
#define TERMIO_ONLY_CFLAG  (CREAD|PARENB)
#define TERMIO_ONLY_LFLAG  (ISIG|ECHOK|ECHONL)

#ifdef _ANSI_C_SOURCE
tty_def_open(struct tty *tp, dev_t dev, int flag, int clocal)
#else
tty_def_open(tp, dev, flag, clocal)
    struct tty *tp;
    dev_t dev;
    int flag;
    int clocal;
#endif
{
	/* What is the default line displine */
	/* How do we know the program's intention for any line displine
	*/	
	/*
	 * Ultrix defaults to a "COOKED" mode on the first
	 * open, while termio defaults to a "RAW" style.
         * Base this decision by a flag set in the termio
         * emulation routine for open, or set by an explicit
         * ioctl call.
         */

	XPR(XPR_TTY, ("tty_def_open: tp = 0x%x, t_state = 0x%x", tp, tp->t_state, 0, 0));
	ttychars(tp);

	switch (tp->t_line) {
	/* termios line discipline */
	case TTYDISC  :	
		tp->t_iflag = TTYDEF_IFLAG;
		tp->t_oflag = TTYDEF_OFLAG;
		tp->t_lflag = TTYDEF_LFLAG;
		tp->t_cflag |= CS8|CREAD;
		tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
		tp->t_dev = dev;
		break;
	/* case OTTYDISC :*/   /* OTTYDISC conflicts with TTYDISC */
	case NTTYDISC :
	/* Provide a backward compatible ULTRIX
         * environment.  "COOKED" style.
         */
		/* only one version of the flag reflect the true state */
		/*
              		tp->t_flags = IFLAGS;  
	 	*/	
              	tp->t_iflag = BSD_IFLAG;
              	tp->t_oflag = BSD_OFLAG;
              	tp->t_lflag = BSD_LFLAG;
              	tp->t_cflag |= BSD_CFLAG;
		break;
         case TERMIODISC : 
	/* SYSTEM V default settings. 
	 * Provide a termio style environment.
         * "RAW" style by default.
	 */
	/*
              	tp->t_flags = RAW;
         */
              	tp->t_iflag = 0;
              	tp->t_oflag = 0;
              	tp->t_cflag |= CS8|CREAD|HUPCL;
	      	tp->t_lflag = 0;
		break;
	defaults : 	/* OSF default settings */	
	      	tp->t_iflag = TTYDEF_IFLAG;
	 	tp->t_oflag = TTYDEF_OFLAG;
	 	tp->t_lflag = TTYDEF_LFLAG;
	 	tp->t_cflag = CS8|CREAD;
		/*
	 	tp->t_line = 0;
		*/
	 	tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
	}
        ttsetwater(tp);

        if (clocal)
        	tp->t_cflag |= CLOCAL;
	else 
		tp->t_cflag |= HUPCL;
	
}


tty_def_close(tp)
register struct tty *tp;
{

	switch (tp->t_line) {
	/* termios line discipline */
	case TTYDISC  :	
		break;
	/* case OTTYDISC :*/   /* OTTYDISC conflicts with TTYDISC */
	case NTTYDISC :
	/* Provide a backward compatible ULTRIX
         * environment.  "COOKED" style.
         */
		break;
	defaults : 	/* OSF default settings */	
    		tp->t_iflag &= ~TERMIO_ONLY_IFLAG;
    		tp->t_oflag &= ~TERMIO_ONLY_OFLAG;
    		tp->t_cflag &= ~TERMIO_ONLY_CFLAG;
    		tp->t_lflag &= ~TERMIO_ONLY_LFLAG;
	}
}
