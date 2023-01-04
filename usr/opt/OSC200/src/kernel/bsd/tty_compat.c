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
static char *rcsid = "@(#)$RCSfile: tty_compat.c,v $ $Revision: 4.4.12.2 $ (DEC) $Date: 1993/05/12 15:21:17 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * tty_compat.c
 *
 * Modification History:
 *
 *  8-Oct-91	Philip Cameron
 *	Added conversion functions to convert between Ultrix 4.2 and 
 *	OSF/1 versions of the termio and termios structs. Also changed
 *	tt_sysv_compat() to pass the POSIX TOSTOP flag in an unused
 *	bit.
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

/* 
 * mapping routines for old line discipline (yuck)
 */

#include "ult_bin_compat.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/termios.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/dk.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/syslog.h>
#include <sys/termio.h>

#include <machine/reg.h>

int ttydebug = 0;


#ifdef COMPAT_43
extern int ttcompatspcodes[];
extern struct speedtab ttcompatspeeds[];

/*ARGSUSED*/
ttcompat(tp, com, data, flag)
	register struct tty *tp;
	unsigned int com;
	caddr_t data;
	long flag;
{
	int ret;

	switch(com) {
	case TIOCGETP: {
		register struct sgttyb *sg = (struct sgttyb *)data;
		register u_char *cc = tp->t_cc;
		register speed;

		speed = ttspeedtab(tp->t_ospeed, ttcompatspeeds);
		sg->sg_ospeed = (speed == -1) ? 15 : speed;
		if (tp->t_ispeed == 0)
			sg->sg_ispeed = sg->sg_ospeed;
		else {
			speed = ttspeedtab(tp->t_ispeed, ttcompatspeeds);
			sg->sg_ispeed = (speed == -1) ? 15 : speed;
		}
		sg->sg_erase = cc[VERASE];
		sg->sg_kill = cc[VKILL];
		sg->sg_flags = ttcompatgetflags(tp->t_iflag, tp->t_lflag,
						tp->t_oflag, tp->t_cflag);
		break;
	}

	case TIOCSETP:
	case TIOCSETN: {
		register struct sgttyb *sg = (struct sgttyb *)data;
		struct termios term;
		int speed;

		term = tp->t_termios;
		if ((speed = sg->sg_ispeed) > 15 || speed < 0)
			term.c_ispeed = speed;
		else
			term.c_ispeed = ttcompatspcodes[speed];
		if ((speed = sg->sg_ospeed) > 15 || speed < 0)
			term.c_ospeed = speed;
		else
			term.c_ospeed = ttcompatspcodes[speed];
		term.c_cc[VERASE] = sg->sg_erase;
		term.c_cc[VKILL] = sg->sg_kill;
		tp->t_flags = (tp->t_flags&0xffff0000) | sg->sg_flags;
		ttcompatsetsgflags(tp->t_flags, &term.c_iflag, &term.c_oflag,
				   &term.c_lflag, &term.c_cflag,
				   &term.c_cc[VMIN], &term.c_cc[VTIME]);
		return (ttioctl(tp, com == TIOCSETP ? TIOCSETAF : TIOCSETA, 
			(caddr_t)&term, flag));
	}

	case TIOCGETC: {
		struct tchars *tc = (struct tchars *)data;
		register u_char *cc = tp->t_cc;

		tc->t_intrc = cc[VINTR];
		tc->t_quitc = cc[VQUIT];
		tc->t_startc = cc[VSTART];
		tc->t_stopc = cc[VSTOP];
		tc->t_eofc = cc[VEOF];
		tc->t_brkc = cc[VEOL];
		break;
	}
	case TIOCSETC: {
		struct tchars *tc = (struct tchars *)data;
		register u_char *cc = tp->t_cc;

		cc[VINTR] = tc->t_intrc;
		cc[VQUIT] = tc->t_quitc;
		cc[VSTART] = tc->t_startc;
		cc[VSTOP] = tc->t_stopc;
		cc[VEOF] = tc->t_eofc;
		cc[VEOL] = tc->t_brkc;
		if (tc->t_brkc == -1)
			cc[VEOL2] = _POSIX_VDISABLE;
		break;
	}
	case TIOCSLTC: {
		struct ltchars *ltc = (struct ltchars *)data;
		register u_char *cc = tp->t_cc;

		cc[VSUSP] = ltc->t_suspc;
		cc[VDSUSP] = ltc->t_dsuspc;
		cc[VREPRINT] = ltc->t_rprntc;
		cc[VDISCARD] = ltc->t_flushc;
		cc[VWERASE] = ltc->t_werasc;
		cc[VLNEXT] = ltc->t_lnextc;
		break;
	}
	case TIOCGLTC: {
		struct ltchars *ltc = (struct ltchars *)data;
		register u_char *cc = tp->t_cc;

		ltc->t_suspc = cc[VSUSP];
		ltc->t_dsuspc = cc[VDSUSP];
		ltc->t_rprntc = cc[VREPRINT];
		ltc->t_flushc = cc[VDISCARD];
		ltc->t_werasc = cc[VWERASE];
		ltc->t_lnextc = cc[VLNEXT];
		break;
	}
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET: {
		struct termios term;

		term = tp->t_termios;
		if (com == TIOCLSET)
			tp->t_flags = (tp->t_flags&0xffff) | *(int *)data<<16;
		else {
			ret = ttcompatgetflags(tp->t_iflag, tp->t_lflag,
					       tp->t_oflag, tp->t_cflag);
			tp->t_flags = (ret&0xffff0000)|(tp->t_flags&0xffff);
			if (com == TIOCLBIS)
				tp->t_flags |= *(int *)data<<16;
			else
				tp->t_flags &= ~(*(int *)data<<16);
		}
		ttcompatsetlflags(tp->t_flags, &term.c_iflag, &term.c_oflag,
				  &term.c_lflag, &term.c_cflag);
		return (ttioctl(tp, TIOCSETA, &term, flag));
	}
	case TIOCLGET:
		ret = ttcompatgetflags(tp->t_iflag, tp->t_lflag,
				       tp->t_oflag, tp->t_cflag);
		*(int *)data = (ret >> 16);
		break;

	case OTIOCGETD:
		*(int *)data = tp->t_line ? tp->t_line : 2;
		break;

	case OTIOCSETD: {
		int ldisczero = 0;

		return(ttioctl(tp, TIOCSETD, 
			*(int *)data == 2 ? (caddr_t)&ldisczero : data, flag));
	}

	default:
		return (-1);
	}
	return(0);
}


#endif	/* COMPAT_43 */

/* Mapping routines for SVID tty ioctls */
tt_sysv_compat(tp, com, data, flag)
	register struct tty *tp;
	unsigned int com;
	caddr_t data;
	long flag;
{
	struct termios term;
	struct termio *termio_ptr;
	int cmd;
	int speed;
	int line;
	int i;
        TSPLVAR(s)
	
	termio_ptr = (struct termio *) data;
	
	switch(com) {
	case TCXONC: {
		/* Start/stop control */
                
                switch (*(int *)data) {
                case TCOOFF:
                        return(ttioctl(tp, TIOCSTOP, data, flag));
                        
                case TCOON:
                        return(ttioctl(tp, TIOCSTART, data, flag));

                case TCIOFF:
                        if (tp->t_cc[VSTOP] != _POSIX_VDISABLE &&
                            putc(tp->t_cc[VSTOP], &tp->t_outq) == 0) {
                                TSPLTTY(s);
                                tp->t_state |= TS_TBLOCK;
                                TSPLX(s);
                                ttstart(tp);
                        }
                        return(0);
                                
                case TCION:
                        if (tp->t_cc[VSTART] != _POSIX_VDISABLE &&
                            putc(tp->t_cc[VSTART], &tp->t_outq) == 0) {
                                TSPLTTY(s);
                                tp->t_state &= ~TS_TBLOCK;
                                TSPLX(s);
                                ttstart(tp);
                        }
                        return(0);
                                
                default:
                        return(EINVAL);
                }
                break;
        }
                        
	case TCFLSH: {
		/* Flush queues */
                int flags;
                
                switch (*(int *)data) {
                case TCIFLUSH:
                        flags = FREAD;
                        break;
				
                case TCOFLUSH:
                        flags = FWRITE;
                        break;
 
                case TCIOFLUSH:
                        flags = FREAD | FWRITE;
                        break;
 
		default:
                        return(EINVAL);
                }
						
                ttyflush(tp, flags);
                break;
        }
		
		
	case TCGETA:
                /* Get tty attributes */
		termio_ptr->c_iflag = tp->t_iflag;
		termio_ptr->c_oflag = tp->t_oflag;
		speed = ttspeedtab(tp->t_ospeed, ttcompatspeeds);
		speed = speed == -1 ? B0 : speed;
		termio_ptr->c_cflag = tp->t_cflag | speed;
		termio_ptr->c_lflag = tp->t_lflag;
		if (tp->t_lflag & NOFLSH) 
			termio_ptr->c_lflag |= VNOFLSH;
		termio_ptr->c_line = tp->t_line;
		termio_ptr->c_cc[VVINTR] = tp->t_cc[VINTR];
		termio_ptr->c_cc[VVQUIT] = tp->t_cc[VQUIT];
		termio_ptr->c_cc[VVERASE] = tp->t_cc[VERASE];
		termio_ptr->c_cc[VVKILL] = tp->t_cc[VKILL];
		termio_ptr->c_cc[VVEOL2] = tp->t_cc[VEOL2];
		termio_ptr->c_cc[VVSWTCH] = tp->t_cc[VSUSP];
		if (tp->t_lflag & ICANON) {
			termio_ptr->c_cc[VVEOL] = tp->t_cc[VEOL];
			termio_ptr->c_cc[VVEOF] = tp->t_cc[VEOF];
		}
		else {
			termio_ptr->c_cc[VVMIN] = tp->t_cc[VMIN];
			termio_ptr->c_cc[VVTIME] = tp->t_cc[VTIME];
		}
		break;
		
	case TCSETAW:
	case TCSETAF:
	case TCSETA:
		/* Note that since the termio structure uses shorts and */
		/* the termios structure uses longs any flag set in the */
		/* upper half of the word will be cleared. However, this */
		/* is the correct behavior. Note that we have to move */
		/* around the NOFLSH flag. */
		term.c_iflag = termio_ptr->c_iflag;
		term.c_oflag = termio_ptr->c_oflag;
		term.c_cflag = termio_ptr->c_cflag & ~0xf;
		term.c_ispeed = term.c_ospeed =
		    ttcompatspcodes[termio_ptr->c_cflag & 0xf];
		term.c_lflag = termio_ptr->c_lflag & ~VNOFLSH;
		if (termio_ptr->c_lflag & VNOFLSH)
			term.c_lflag |= NOFLSH;

		/* Initialize the whole termios cc array since its bigger */
		for (i = 0; i < NCCS; i++) {
			term.c_cc[i] = tp->t_cc[i];
		}
		
		term.c_cc[VINTR] = termio_ptr->c_cc[VVINTR];
		term.c_cc[VQUIT] = termio_ptr->c_cc[VVQUIT];
		term.c_cc[VERASE] = termio_ptr->c_cc[VVERASE];
		term.c_cc[VKILL] = termio_ptr->c_cc[VVKILL];
		term.c_cc[VEOL2] = termio_ptr->c_cc[VVEOL2];
		term.c_cc[VSUSP] = termio_ptr->c_cc[VVSWTCH];
		if (termio_ptr->c_lflag & ICANON) {
			term.c_cc[VEOL] = termio_ptr->c_cc[VVEOL];
			term.c_cc[VEOF] = termio_ptr->c_cc[VVEOF];
		}
		else {
			term.c_cc[VMIN] = termio_ptr->c_cc[VVMIN];
			term.c_cc[VTIME] = termio_ptr->c_cc[VVTIME];
		}
 
		switch(com) {
		case TCSETAW:
			cmd = TIOCSETAW;
			break;
			
		case TCSETAF:
			cmd = TIOCSETAF;
			break;
			
		case TCSETA:
			cmd = TIOCSETA;
			break;
		}
		
                /*
                 * Check to see if the line displine is
                 * changing. If so change it then to the
                 * ioctl.
                 */
		if (termio_ptr->c_line != tp->t_line) {
			int retval;
			line = termio_ptr->c_line;
			if (retval = ttioctl(tp, TIOCSETD, &line, flag))
				return(retval);
		}

		return(ttioctl(tp, cmd, &term, flag));
		break;
 
	default:
		return(EINVAL);
	}
 
	return(0);
}
