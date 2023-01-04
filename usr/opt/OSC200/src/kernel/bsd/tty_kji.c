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
static char	*sccsid = "@(#)$RCSfile: tty_kji.c,v $ $Revision: 4.4.10.2 $ (DEC) $Date: 1993/08/02 20:41:03 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * tty_kji
 *
 * Modification History:
 *
 * 7-SEP-91 Brian Harrigan
 *      Removed 30-sep changes as definitions were changed in the .h files.
 *
 * 30-AUG-91    Brian Harrigan
 *      Changed FUNNEL defs for RT_PREEMPT for the RT MPK
 *	(EFT) This Hack should be fixed in the .h files ...
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
 *	@(#)tty_kji.c	3.1 (Berkeley) 2/26/91
 */

#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/dk.h>
#include <sys/buf.h>
#include <sys/dk.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/poll.h>
#include <machine/reg.h>
#include <kern/parallel.h>
#include <kern/assert.h>

#include <cputypes.h>
#include <sys/termio.h>

extern ttin_timeout();

#define TTY_TYPEMASK	037		/* kji_partab type mask */
#define SHIF_JIS 0040	/* bit 6 is SHIFT-JIS shift byte indicator */
#define ISSHIFT(x) (kji_partab[(x) & TTY_CHARMASK] & SHIF_JIS)

/*
 * Table giving parity for characters and indicating
 * character classes to tty driver. The 8th bit
 * indicates parity, the 7th bit indicates the character
 * is an alphameric or underscore (for ALTWERASE), the
 * 6th bit tell if the character is a shift character (ISSHIFT)
 * The low 5 bits indicate delay type.  If the low 5 bits are 0
 * then the character needs no special processing on output.
 */

char kji_partab[] = {
	0001,0201,0201,0001,0201,0001,0001,0201,	/* nul - bel */
	0202,0004,0003,0201,0005,0206,0201,0001,	/* bs - si */
	0201,0001,0001,0201,0001,0201,0201,0001,	/* dle - etb */
	0001,0201,0201,0001,0201,0001,0001,0201,	/* can - us */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* sp - ' */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* ( - / */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* 0 - 7 */
	0300,0100,0000,0200,0000,0200,0200,0000,	/* 8 - ? */
	0200,0100,0100,0300,0100,0300,0300,0100,	/* @ - G */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* H - O */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* P - W */
	0300,0100,0100,0200,0000,0200,0200,0100,	/* X - _ */
	0000,0300,0300,0100,0300,0100,0100,0300,	/* ` - g */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* h - o */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* p - w */
	0100,0300,0300,0000,0200,0000,0000,0201,	/* x - del */
	/*
	 * meta chars
	 */
	0200,0040,0040,0240,0040,0200,0200,0000,	/* 0x80 - 0x87 */
	0040,0240,0240,0040,0240,0040,0040,0240,	/* 0x88 - 0x8f */
	0040,0240,0240,0040,0240,0040,0040,0240,	/* 0x90 - 0x97 */
	0240,0040,0040,0240,0040,0240,0240,0040,	/* 0x98 - 0x9f */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* 0xa0 - 0xa7 */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* 0xa8 - 0xaf */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* 0xb0 - 0xb7 */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* 0xb8 - 0xbf */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* 0xc0 - 0xc7 */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* 0xc8 - 0xcf */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* 0xd0 - 0xd7 */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* 0xd8 - 0xdf */
	0240,0040,0040,0240,0040,0240,0240,0040,	/* 0xe0 - 0xe7 */
	0040,0240,0240,0000,0200,0000,0000,0200,	/* 0xe8 - 0xef */
	0040,0240,0240,0040,0240,0040,0040,0240,	/* 0xf0 - 0xf7 */
	0240,0040,0040,0240,0040,0200,0200,0000,	/* 0xf8 - 0xff */
};

extern char kji_partab[], maptab[];

/*
 * Is 'c' a line delimiter ("break" character)?
 */
#define ttbreakc(c) (c == '\n' || CCEQ(cc[VEOF], c) || \
		CCEQ(cc[VEOL], c) || CCEQ(cc[VEOL2], c))

/*
 * reinput pending characters after state switch
 * call at spltty().
 */
kji_ttypend(tp)
	register struct tty *tp;
{
	struct clist tq;
	register c;

	LASSERT(TTY_LOCK_HOLDER(tp));
	tp->t_lflag &= ~PENDIN;
	tp->t_state |= TS_TYPEN;
	tq = tp->t_rawq;
	tp->t_rawq.c_cc = 0;
	tp->t_rawq.c_cf = tp->t_rawq.c_cl = 0;
	while ((c = getc(&tq)) >= 0)
		kji_ttyinput(c, tp);
	tp->t_state &= ~TS_TYPEN;
}

/*
 *
 * Place a character on raw TTY input queue,
 * putting in delimiters and waking up top
 * half as needed.  Also echo if required.
 * The arguments are the character and the
 * appropriate tty structure.
 */
kji_ttyinput(c, tp)
	register long c;
	register struct tty *tp;
{
	register int iflag = tp->t_iflag;
	register int lflag = tp->t_lflag;
	register u_char *cc = tp->t_cc;
	int i, err;

	LASSERT(TTY_LOCK_HOLDER(tp));
	/*
	 * If input is pending take it first.
	 */
	if (lflag&PENDIN)
		kji_ttypend(tp);
	/*
	 * Gather stats.
	 */
	tk_nin++;
	if (lflag&ICANON) {
		tk_cancc++;
		tp->t_cancc++;
	} else {
		tk_rawcc++;
		tp->t_rawcc++;
	}
	/*
	 * Handle exceptional conditions (break, parity, framing).
	 */
	if (err = (c&TTY_ERRORMASK)) {
		c &= ~TTY_ERRORMASK;
		if (err&TTY_FE && !c) {		/* break */
			if (iflag&IGNBRK)
				goto endcase;
			else if (iflag&BRKINT && lflag&ISIG &&
				(cc[VINTR] != _POSIX_VDISABLE))
				c = cc[VINTR];
			else {
				c = 0;
				if (iflag&PARMRK)
					goto parmrk;
			}
		} else if ((err&TTY_PE && iflag&INPCK) || err&TTY_FE) {
			if (iflag&IGNPAR)
				goto endcase;
			else if (iflag&PARMRK) {
parmrk:
				putc(0377|TTY_QUOTE, &tp->t_rawq);
				putc(0|TTY_QUOTE, &tp->t_rawq);
				putc(c|TTY_QUOTE, &tp->t_rawq);
				goto endcase;
			} else
				c = 0;
		}
	}
	/*
	 * In tandem mode, check high water mark.
	 */
	if (iflag&IXOFF)
		ttyblock(tp);
	if ((tp->t_state&TS_TYPEN) == 0 && (iflag&ISTRIP))
		c &= 0177;
	/*
	 * Check for literal nexting very first
	 */
	if (tp->t_state&TS_LNCH) {
		c |= TTY_QUOTE;
		tp->t_state &= ~TS_LNCH;
	}
	/*
	 * Scan for special characters.  This code
	 * is really just a big case statement with
	 * non-constant cases.  The bottom of the
	 * case statement is labeled ``endcase'', so goto
	 * it after a case match, or similar.
	 */
	/*
	 * Control chars which aren't controlled
	 * by ICANON, ISIG, or IXON.
	 */
	if (lflag&IEXTEN) {
		if (CCEQ(cc[VLNEXT],c)) {
			if (lflag&ECHO) {
				if (lflag&ECHOE)
					ttyoutstr("^\b", tp);
				else
					ttyecho(c, tp);
			}
			tp->t_state |= TS_LNCH;
			goto endcase;
		}
		if (CCEQ(cc[VDISCARD],c)) {
			if (lflag&FLUSHO)
				tp->t_lflag &= ~FLUSHO;
			else {
				ttyflush(tp, FWRITE);
				ttyecho(c, tp);
				if (tp->t_rawq.c_cc + tp->t_canq.c_cc)
					ttyretype(tp);
				tp->t_lflag |= FLUSHO;
			}
			goto startoutput;
		}
	}
	/*
	 * Signals.
	 */
	if (lflag&ISIG) {
		if (CCEQ(cc[VINTR], c) || CCEQ(cc[VQUIT], c)) {
			if ((lflag&NOFLSH) == 0)
				ttyflush(tp, FREAD|FWRITE);
			ttyecho(c, tp);
			unix_master();
			pgsignal(tp->t_pgrp,
				CCEQ(cc[VINTR],c) ? SIGINT : SIGQUIT, 1);
			unix_release();
			goto endcase;
		}
		if (CCEQ(cc[VSUSP],c)) {
			if ((lflag&NOFLSH) == 0)
				ttyflush(tp, FREAD);
			ttyecho(c, tp);
			unix_master();
			pgsignal(tp->t_pgrp, SIGTSTP, 1);
			unix_release();
			goto endcase;
		}
		if (CCEQ(cc[VSTATUS],c)) {
			ttyinfo(tp);
			goto endcase;
		}
	}
	/*
	 * Handle start/stop characters.
	 */
	if (iflag&IXON) {
		if (CCEQ(cc[VSTOP],c)) {
			if ((tp->t_state&TS_TTSTOP) == 0) {
				tp->t_state |= TS_TTSTOP;
				CDEVSW_STOP(major(tp->t_dev), tp, 0, err);
				return;
			}
			if (!CCEQ(cc[VSTART], c))
				return;
			/*
			 * if VSTART == VSTOP then toggle
			 */
			goto endcase;
		}
		if (CCEQ(cc[VSTART], c))
			goto restartoutput;
	}
	/*
	 * IGNCR, ICRNL, & INLCR
	 */
	if (c == '\r') {
		if (iflag&IGNCR)
			goto endcase;
		else if (iflag&ICRNL)
			c = '\n';
	}
	else if (c == '\n' && iflag&INLCR)
		c = '\r';

				/* Map Upper case to lower case */
	if (iflag&IUCLC && 'A' <= c && c <= 'Z')
	    c += 'a' - 'A';

	/*
	 * Non canonical mode; don't process line editing
	 * characters; check high water mark for wakeup.
	 *
	 */
	if (!(lflag&ICANON)) {
		if (tp->t_rawq.c_cc > TTYHOG) {
			if (iflag&IMAXBEL) {
				if (tp->t_outq.c_cc < tp->t_hiwat)
					(void) ttyoutput(CTRL('g'), tp);
			} else
				ttyflush(tp, FREAD | FWRITE);
		} else {
			if (putc(c, &tp->t_rawq) >= 0) {
				ttyecho(c, tp);
				if (tp->t_rawq.c_cc >= tp->t_cc[VMIN])
				{
				    ttwakeup(tp);
			        }
				else if (tp->t_shad_time > 0)
				{
					if (tp->t_state & TS_INTIMEOUT)
					{
						untimeout(ttin_timeout, tp);
					}
					timeout(ttin_timeout, (caddr_t) tp,
						tp->t_shad_time);
					tp->t_state |= TS_INTIMEOUT;
				}
				
			}
		}
		goto endcase;
	}
	/*
	 * From here on down canonical mode character
	 * processing takes place.
	 */
	/*
	 * erase (^H / ^?)
	 */
	if (CCEQ(cc[VERASE], c)) {
		if (tp->t_rawq.c_cc)
			kji_ttyrub(unputc(&tp->t_rawq), tp);
		goto endcase;
	}
	/*
	 * kill (^U)
	 */
	if (CCEQ(cc[VKILL], c)) {
		if (lflag&ECHOKE && tp->t_rawq.c_cc == tp->t_rocount &&
		    !(lflag&ECHOPRT)) {
			while (tp->t_rawq.c_cc)
				kji_ttyrub(unputc(&tp->t_rawq), tp);
		} else {
			ttyecho(c, tp);
			if (lflag&ECHOK || lflag&ECHOKE)
				ttyecho('\n', tp);
			while (getc(&tp->t_rawq) > 0)
				;
			tp->t_rocount = 0;
		}
		tp->t_state &= ~TS_LOCAL;
		goto endcase;
	}
	/*
	 * word erase (^W)
	 */
	if (CCEQ(cc[VWERASE], c)) {	
		int ctype;

#define CTYPE(c) ((lflag&ALTWERASE) ? (kji_partab[(c)&TTY_CHARMASK]&0100) : 0)
		/*
		 * erase whitespace
		 */
		while ((c = unputc(&tp->t_rawq)) == ' ' || c == '\t')
			kji_ttyrub(c, tp);
		if (c == -1)
			goto endcase;
		/*
		 * special case last char of token
		 */
		kji_ttyrub(c, tp);
		c = unputc(&tp->t_rawq);
		if (c == -1 || c == ' ' || c == '\t') {
			if (c != -1)
				(void) putc(c, &tp->t_rawq);
			goto endcase;
		}
		/*
		 * erase rest of token
		 */
		ctype = CTYPE(c);
		do {
			kji_ttyrub(c, tp);
			c = unputc(&tp->t_rawq);
			if (c == -1)
				goto endcase;
		} while (c != ' ' && c != '\t' && CTYPE(c) == ctype);
		(void) putc(c, &tp->t_rawq);
		goto endcase;
#undef CTYPE
	}
	/*
	 * reprint line (^R)
	 */
	if (CCEQ(cc[VREPRINT], c)) {
		ttyretype(tp);
		goto endcase;
	}
	/*
	 * Check for input buffer overflow
	 */
	if (tp->t_rawq.c_cc+tp->t_canq.c_cc >= TTYHOG) {
		if (iflag&IMAXBEL) {
			if (tp->t_outq.c_cc < tp->t_hiwat)
				(void) ttyoutput(CTRL('g'), tp);
		} else
			ttyflush(tp, FREAD | FWRITE);
		goto endcase;
	}

	/*
	 * Put data char in q for user and
	 * wakeup on seeing a line delimiter.
	 */
	if (putc(c, &tp->t_rawq) >= 0) {
		if (ttbreakc(c)) {
			tp->t_rocount = 0;
			catq(&tp->t_rawq, &tp->t_canq);
			ttwakeup(tp);
		} else if (tp->t_rocount++ == 0)
			tp->t_rocol = tp->t_col;
		if (tp->t_state&TS_ERASE) {
			/*
			 * end of prterase \.../
			 */
			tp->t_state &= ~TS_ERASE;
			(void) ttyoutput('/', tp);
		}
		i = tp->t_col;
		ttyecho(c, tp);
		if (CCEQ(cc[VEOF], c) && lflag&ECHO) {
			/*
			 * Place the cursor over the '^' of the ^D.
			 */
			i = MIN(2, tp->t_col - i);
			while (i > 0) {
				(void) ttyoutput('\b', tp);
				i--;
			}
		}
	}
endcase:
	/*
	 * IXANY means allow any character to restart output.
	 */
	if ((tp->t_state&TS_TTSTOP) && !(iflag&IXANY)
	    && cc[VSTART] != cc[VSTOP])
		return;
restartoutput:
	tp->t_state &= ~TS_TTSTOP;
	tp->t_lflag &= ~FLUSHO;
startoutput:
	ttstart(tp);
}

/* I don't think this is right, but that's what he gave us...
 * Suspect a null map is more appropriate - tmt */
extern struct {
	char from;
	char to;
	} xcase_map[];

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
kji_ttread(tp, uio, flag)
	register struct tty *tp;
	struct uio *uio;
	long flag;
{
	register struct clist *qp;
	register int c;
	register long lflag = tp->t_lflag;
	register u_char *cc = tp->t_cc;
	int first, error = 0;
	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	first = 1;
	
loop:
	TSPLTTY(s);
	/*
	 * take pending input first
	 */
	if (lflag&PENDIN)
		kji_ttypend(tp);
	/*
	 * Handle carrier.
	 */
	if (!(tp->t_state&TS_CARR_ON) && !(tp->t_cflag&CLOCAL)) {
		if (tp->t_state&TS_ISOPEN) {
			TSPLX(s);
			return (0);	/* EOF */
		} else if (flag & (IO_NDELAY|IO_NONBLOCK)) {
			TSPLX(s);
			return (EWOULDBLOCK);
		} else {
			/*
			 * sleep awaiting carrier
			 */
			error = ttysleep(tp, (caddr_t)&tp->t_rawq, 
					 TTIPRI | PCATCH, ttyin);
			TSPLX(s);
			if (error)
				return (error);
			goto loop;
		}
	}
	TSPLX(s);
	/*
	 * Hang process if it's in the background.
	 */
	unix_master();
	if (isbackground(u.u_procp, tp)) {
		if ((u.u_procp->p_sigignore & sigmask(SIGTTIN)) ||
		   (u.u_procp->p_sigmask & sigmask(SIGTTIN)) ||
		    u.u_procp->p_flag&SVFORK ||
		    u.u_procp->p_pgrp->pg_jobc == 0)
			return (EIO);
		pgsignal(u.u_procp->p_pgrp, SIGTTIN, 1);
		unix_release();
		if (error = ttysleep(tp, (caddr_t)&lbolt, TTIPRI | PCATCH,
				     ttybg))
			return (error);
		goto loop;
	}
	unix_release();
	/*
	 * If canonical, use the canonical queue,
	 * else use the raw queue.
	 *
	 * XXX - should get rid of canonical queue.
	 * (actually, should get rid of clists...)
	 */
	if (lflag&ICANON)
	{
		qp = &tp->t_canq;
		
		TSPLTTY(s);
		if (qp->c_cc <= 0) {
			/** XXX ??? ask mike why TS_CARR_ON was (once) necessary here
			  if ((tp->t_state&TS_CARR_ON) == 0 ||
			  (tp->t_state&TS_NBIO)) {
			  TSPLX(s);
			  return (EWOULDBLOCK);
			  }
			  **/
			if (flag & (IO_NDELAY|IO_NONBLOCK)) {
				TSPLX(s);
				return (EWOULDBLOCK);
			}
			error = ttysleep(tp, (caddr_t)&tp->t_rawq,
				TTIPRI | PCATCH, ttyin);
			TSPLX(s);
			if (error)
				return (error);
			goto loop;
		}
		TSPLX(s);
	}
	else
	{
		qp = &tp->t_rawq;
		TSPLTTY(s);
		if (qp->c_cc < tp->t_cc[VMIN] || qp->c_cc == 0) {
			/** XXX ??? ask mike why TS_CARR_ON was (once) necessary here
			  if ((tp->t_state&TS_CARR_ON) == 0 ||
			  (tp->t_state&TS_NBIO)) {
			  TSPLX(s);
			  return (EWOULDBLOCK);
			  }
			  **/
			if (flag & (IO_NDELAY|IO_NONBLOCK)) {
				TSPLX(s);
				return (EWOULDBLOCK);
			}
			if (tp->t_cc[VMIN] == 0 &&
			    (tp->t_shad_time == 0 || !first))
			{
				/*
				 * If there are no chars and the caller
				 * doesn't want to wait or has waited once
				 * then return
				 */
				return(error);
			}
			if (tp->t_shad_time > 0 && !(tp->t_state&TS_INTIMEOUT) 
			    && (qp->c_cc > 0 || tp->t_cc[VMIN] == 0))
			{
				first = 0;
				timeout(ttin_timeout, tp, tp->t_shad_time);
				tp->t_state |= TS_INTIMEOUT;
			}

			if (error = ttysleep (tp, (caddr_t)&tp->t_rawq,
					      TTIPRI | PCATCH, ttyin)) {
				TSPLX(s);
				return (error);
			}
			if (qp->c_cc <= 0)
			{
				/* If there are no characters wait again. */
				TSPLX(s);
				goto loop;
			}
		}

		TSPLX(s);
	}		
	/*
	 * Input present, check for input mapping and processing.
	 */
	first = 1;
	while ((c = getc(qp)) >= 0) {
		/*
		 * delayed suspend (^Y)
		 */
		if (CCEQ(cc[VDSUSP], c) && lflag&ISIG) {
			unix_master();
			pgsignal(tp->t_pgrp, SIGTSTP, 1);
			unix_release();
			if (first) {
				if (error = ttysleep(tp, (caddr_t)&lbolt,
					TTIPRI | PCATCH, ttybg))
					break;
				goto loop;
			}
			break;
		}
		/*
		 * Interpret EOF only in canonical mode.
		 */
		if (CCEQ(cc[VEOF], c) && lflag&ICANON)
			break;
			
		/*
		 * Connical upper/lower presentation
		 */
		if((lflag&ICANON) && (lflag&XCASE))
		{
			int next_char;	
			/*
			 * If this character is a quote and
			 * if the next character is one we want to
			 * remap then discard the \\ and send new
			 * remap character otherwise send the \\
			 */
			if ((c == '\\') && (qp->c_cc > 0))
			{
				next_char = *qp->c_cf;
				if ( next_char >= 'a' && next_char <= 'z')
				{
					(void) getc(qp);
					c = next_char - 'a' + 'A';
				}
				else
				{
					int cnt;
					cnt = 0;
					while(xcase_map[cnt].from)
					{
						if (xcase_map[cnt].from == next_char)
						{
							(void) getc(qp);
							c = xcase_map[cnt].to;
							break;
						}
						cnt++;
					}
				}
			}
		}
		
				   
		
		/*
		 * Give user character.
		 */
 		error = ureadc(c , uio);
		if (error)
			break;
 		if (uio->uio_resid == 0)
			break;
		/*
		 * In canonical mode check for a "break character"
		 * marking the end of a "line of input".
		 */
		if (lflag&ICANON && ttbreakc(c)) {
			break;
		}
		first = 0;
	}
	/*
	 * Look to unblock output now that (presumably)
	 * the input queue has gone down.
	 */
	if (tp->t_state&TS_TBLOCK && tp->t_rawq.c_cc < TTYHOG/5) {
		if (cc[VSTART] != _POSIX_VDISABLE
		   && putc(cc[VSTART], &tp->t_outq) == 0) {
			TSPLTTY(s);
			tp->t_state &= ~TS_TBLOCK;
			TSPLX(s);
			ttstart(tp);
		}
	}
	return (error);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
kji_ttwrite(tp, uio, flag)
	register struct tty *tp;
	register struct uio *uio;
	long flag;
{
	register char *cp;
	register int cc = 0, ce;
	int i, hiwat, cnt, error, s;
	char obuf[OBUFSIZ];

	LASSERT(TTY_LOCK_HOLDER(tp));
	hiwat = tp->t_hiwat;
	cnt = uio->uio_resid;
	error = 0;
loop:
	TSPLTTY(s);
	if (!(tp->t_state&TS_CARR_ON) && !(tp->t_cflag&CLOCAL)) {
		if (tp->t_state&TS_ISOPEN) {
			TSPLX(s);
			return (EIO);
		} else if (flag & (IO_NDELAY|IO_NONBLOCK)) {
			TSPLX(s);
			error = EWOULDBLOCK;
			goto out;
		} else {
			/*
			 * sleep awaiting carrier
			 */
			error = ttysleep(tp, (caddr_t)&tp->t_rawq,
					 TTIPRI | PCATCH, ttopen);
			TSPLX(s);
			if (error)
				goto out;
			goto loop;
		}
	}
	TSPLX(s);
	/*
	 * Hang the process if it's in the background.
	 */
	unix_master();
	if (isbackground(u.u_procp, tp) &&
	    (tp->t_lflag&TOSTOP) && (u.u_procp->p_flag&SVFORK)==0 &&
	    !(u.u_procp->p_sigignore & sigmask(SIGTTOU)) &&
	    !(u.u_procp->p_sigmask & sigmask(SIGTTOU)) &&
	     u.u_procp->p_pgrp->pg_jobc) {
		pgsignal(u.u_procp->p_pgrp, SIGTTOU, 1);
		unix_release();
		if (error = ttysleep(tp, (caddr_t)&lbolt, TTIPRI | PCATCH,
				     ttybg))
			goto out;
		goto loop;
	}
	unix_release();
	/*
	 * Process the user's data in at most OBUFSIZ
	 * chunks.  Perform any output translation.
	 * Keep track of high water mark, sleep on overflow
	 * awaiting device aid in acquiring new space.
	 */
	while (uio->uio_resid > 0 || cc > 0) {
		if (tp->t_lflag&FLUSHO) {
			uio->uio_resid = 0;
			return (0);
		}
		if (tp->t_outq.c_cc > hiwat)
			goto ovhiwat;
		/*
		 * Grab a hunk of data from the user.
		 * unless we have some left over from last time.
		 */
		if (cc == 0) {
			cc = min(uio->uio_resid, OBUFSIZ);
			cp = obuf;
			error = uiomove(cp, cc, uio);
			if (error) {
				cc = 0;
				break;
			}
		}
		/*
		 * If nothing fancy need be done, grab those characters we
		 * can handle without any of ttyoutput's processing and
		 * just transfer them to the output q.  For those chars
		 * which require special processing (as indicated by the
		 * bits in kji_partab), call ttyoutput.  After processing
		 * a hunk of data, look for FLUSHO so ^O's will take effect
		 * immediately.
		 */
		while (cc > 0) {
			if (!(tp->t_oflag&OPOST))
				ce = cc;
			else {
				if ((tp->t_oflag & OLCUC) ||
				    (tp->t_lflag & XCASE))
				/* Process all the characters one by one */
					ce = 0;
				else 
					ce = cc - scanc((unsigned)cc,
						(u_char *)cp,
						(u_char *)kji_partab, 077);
				
				/*
				 * If ce is zero, then we're processing
				 * a special character through ttyoutput.
				 */
				if (ce == 0) {
					tp->t_rocount = 0;
					if (ttyoutput(*cp, tp) >= 0) {
					    /* no c-lists, wait a bit */
					    ttstart(tp);
					    if (error = ttysleep(tp,
						(caddr_t)&lbolt,
						TTOPRI | PCATCH, ttybuf))
							break;
					    goto loop;
					}
					cp++, cc--;
					if ((tp->t_lflag&FLUSHO) ||
					    tp->t_outq.c_cc > hiwat)
						goto ovhiwat;
					continue;
				}
			}
			/*
			 * A bunch of normal characters have been found,
			 * transfer them en masse to the output queue and
			 * continue processing at the top of the loop.
			 * If there are any further characters in this
			 * <= OBUFSIZ chunk, the first should be a character
			 * requiring special handling by ttyoutput.
			 */
			tp->t_rocount = 0;
			i = b_to_q(cp, ce, &tp->t_outq);
			ce -= i;
			tp->t_col += ce;
			cp += ce, cc -= ce, tk_nout += ce;
			tp->t_outcc += ce;
			if (i > 0) {
				/* out of c-lists, wait a bit */
				ttstart(tp);
				if (error = ttysleep(tp, (caddr_t)&lbolt,
					    TTOPRI | PCATCH, ttybuf))
					break;
				goto loop;
			}
			if (tp->t_lflag&FLUSHO || tp->t_outq.c_cc > hiwat)
				break;
		}
		ttstart(tp);
	}
out:
	/*
	 * If cc is nonzero, we leave the uio structure inconsistent,
	 * as the offset and iov pointers have moved forward,
	 * but it doesn't matter (the call will either return short
	 * or restart with a new uio).
	 */
	uio->uio_resid += cc;
	return (error);
ovhiwat:
	ttstart(tp);
	TSPLTTY(s);
	/*
	 * This can only occur if FLUSHO is set in t_lflag,
	 * or if ttstart/oproc is synchronous (or very fast).
	 */
	if (tp->t_outq.c_cc <= hiwat) {
		TSPLX(s);
		goto loop;
	}
	if (flag & (IO_NDELAY|IO_NONBLOCK)) {
		TSPLX(s);
		if (uio->uio_resid == cnt)
			return (EWOULDBLOCK);
		return (0);
	}
	tp->t_state |= TS_ASLEEP;
	error =  ttysleep(tp, (caddr_t)&tp->t_outq, TTOPRI | PCATCH, ttyout);
	TSPLX(s);
	if (error)
		goto out;
	goto loop;
}

/* Findchar is called when the most recent character has
 * already been ungetc'd.
 * Find the position of the 1st byte of the last char still
 * in the queue.  If it is a shift character, return TRUE (1)
 * Else return FALSE (0).
 */
static int
findchar(tp)
register struct tty *tp;
{
	register char *cp;
	register char *startch;	/* start of last char */
	register shift;		/* true if on shift char */
	int c;
	int s;
	char *nextc();

	s = spltty();
	shift = 0;
	startch = cp = tp->t_rawq.c_cf;
	if (cp)
		c = *cp;		/* XXX FIX NEXTC */
	for (; cp; cp = nextc(&tp->t_rawq, cp, &c)) {
		c &= TTY_CHARMASK;	/* ditch quoting, etc. */
		if (shift) {		/* if on 2nd byte of char */
			shift = 0;
		} else if (ISSHIFT(c)) { /* if on 1st byte of char */
			startch = cp;
			shift = 1;
		}
	}
	splx(s);
	return shift;
}

/*
 * Rubout one character from the rawq of tp
 * as cleanly as possible.
 */
kji_ttyrub(c, tp)
	register long c;
	register struct tty *tp;
{
	register char *cp;
	register int savecol;
	char *nextc();
	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	if ((tp->t_lflag&ECHO) == 0)
		return;
	tp->t_lflag &= ~FLUSHO;	
	if (tp->t_lflag&ECHOE) {
		if (tp->t_rocount == 0) {
			/*
			 * Screwed by kji_ttwrite; retype
			 */
			ttyretype(tp);
			return;
		}
		/* sjis code begin */
		if (findchar(tp)) {
			ttyrubo(tp, 2);
			c = unputc(&tp->t_rawq);	/* delete an extra */
		} else
		/* sjis code end */
			if (c == ('\t'|TTY_QUOTE) || c == ('\n'|TTY_QUOTE))
			ttyrubo(tp, 2);
		else switch (kji_partab[c&=0377]&077) {

		case ORDINARY:
			ttyrubo(tp, 1);
			break;

		case VTAB:
		case BACKSPACE:
		case CONTROL:
		case RETURN:
		case NEWLINE:
		case FF:
			if (tp->t_lflag&ECHOCTL)
				ttyrubo(tp, 2);
			break;

		case TAB: {
			int c;

			if (tp->t_rocount < tp->t_rawq.c_cc) {
				ttyretype(tp);
				return;
			}
			TSPLTTY(s);
			savecol = tp->t_col;
			tp->t_state |= TS_CNTTB;
			tp->t_lflag |= FLUSHO;
			tp->t_col = tp->t_rocol;
			cp = tp->t_rawq.c_cf;
			if (cp)
				c = *cp;	/* XXX FIX NEXTC */
			for (; cp; cp = nextc(&tp->t_rawq, cp, &c))
				ttyecho(c, tp);
			tp->t_lflag &= ~FLUSHO;
			tp->t_state &= ~TS_CNTTB;
			TSPLX(s);
			/*
			 * savecol will now be length of the tab
			 */
			savecol -= tp->t_col;
			tp->t_col += savecol;
			if (savecol > 8)
				savecol = 8;		/* overflow screw */
			while (--savecol >= 0)
				(void) ttyoutput('\b', tp);
			break;
		}

		default:
			/* XXX */
			printf("kji_ttyrub: would panic c = %d, val = %d\n",
				c, kji_partab[c&=0377]&077);
			/*panic("kji_ttyrub");*/
		}
	} else if (tp->t_lflag&ECHOPRT) {
		if ((tp->t_state&TS_ERASE) == 0) {
			(void) ttyoutput('\\', tp);
			tp->t_state |= TS_ERASE;
		}
		ttyecho(c, tp);
	} else
		ttyecho(tp->t_cc[VERASE], tp);
	tp->t_rocount--;
}
