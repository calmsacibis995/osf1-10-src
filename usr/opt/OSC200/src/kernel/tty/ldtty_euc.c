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
static char *rcsid = "@(#)$RCSfile: ldtty_euc.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/07 22:54:57 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * multi-byte EUC specific processing
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/file.h>

#include <sys/stropts.h>
#include <sys/stream.h>
#include <tty/stream_tty.h>
#include <tty/ldtty.h>

#define SS2	0x8e
#define SS3	0x8f

/*
 * return the number of chars at the end of rawbuf
 * which does not make up a complete mb character
 */
PRIVATE_STATIC int
euctty_state(register struct ldtty *tp)
{
	register unsigned char *cp;
	register unsigned int c;
	register int eucleft = 0;
	register int euctype;
	register mblk_t *mp;

	if (!tp->t_rawcc || !tp->t_rawbuf)
		return(0);
	mp = tp->t_rawbuf;
	while (mp) {
		cp = mp->b_rptr;
		while (cp < mp->b_wptr) {
			c = *cp;
			if (eucleft) {
				eucleft--;
			}
			else if (c == SS2) {
				eucleft = tp->t_cswidth.eucw[2];
				euctype = 2;
			}
			else if (c == SS3) {
				eucleft = tp->t_cswidth.eucw[3];
				euctype = 3;
			}
			else if ((0x00 <= c) && (c < 0xa0))
				/*
				 * C0, ascii, C1
				 */
				eucleft = 0;
			else {
				eucleft = tp->t_cswidth.eucw[1] - 1;
				euctype = 1;
			}
			cp++;
		}
		mp = mp->b_cont;
	}
	if (eucleft)
		if (euctype == 1)
			return(tp->t_cswidth.eucw[euctype] - eucleft);
		else /* euctype == 2 or euctype == 3 */
			return(tp->t_cswidth.eucw[euctype] - eucleft + 1);
	else
		return(0);
}

/*
 * calculate the rocount for the euc string in rawbuf
 */
int
euctty_rocount(register struct ldtty *tp)
{
	register unsigned char *cp;
	register int ct = 0;
	register unsigned int c;
	register int eucleft = 0;
	register mblk_t *mp;

	if (!tp->t_rawcc || !tp->t_rawbuf)
		return(0);
	mp = tp->t_rawbuf;
	while (mp) {
		cp = mp->b_rptr;
		while (cp < mp->b_wptr) {
			c = *cp;
			if (eucleft)
				eucleft--;
			else if (c == SS2) {
				eucleft = tp->t_cswidth.eucw[2];
				ct += tp->t_cswidth.scrw[2];
			}
			else if (c == SS3) {
				eucleft = tp->t_cswidth.eucw[3];
				ct += tp->t_cswidth.scrw[3];
			}
			else if ((0x00 <= c) && (c < 0xa0))
				ct++;
			else {
				eucleft = tp->t_cswidth.eucw[1] - 1;
				ct += tp->t_cswidth.scrw[1];
			}
			cp++;
		}
		mp = mp->b_cont;
	}
	return(ct);
}

/*
 * echo a character, which might be part of a multi-byte EUC char
 * should only be called by ldtty_echo()
 */
void
euctty_echo(register struct ldtty *tp, register int c)
{
	register int i;

	/*
	 * we are waiting for more chars to make up a multi-byte EUC
	 */
	if (tp->t_eucleft) {
		tp->t_rocount--;	/* cuz ldtty_input incremented it */
		tp->t_eucbytes[tp->t_eucind++] = c & TTY_CHARMASK;
		tp->t_eucleft--;
	}
	/*
	 * could be start of new multi-byte EUC
	 */
	else if (c == SS2) {
		tp->t_rocount--;	/* cuz ldtty_input incremented it */
		tp->t_codeset = 2;
		tp->t_eucleft = tp->t_cswidth.eucw[2];
		tp->t_eucbytes[0] = c;
		tp->t_eucind = 1;
	}
	else if (c == SS3) {
		tp->t_rocount--;	/* cuz ldtty_input incremented it */
		tp->t_codeset = 3;
		tp->t_eucleft = tp->t_cswidth.eucw[3];
		tp->t_eucbytes[0] = c;
		tp->t_eucind = 1;
	}
	else if (c < 0xa0) {
		/*
		 * regular ascii characters and control characters
		 * should be mostly the same as ldtty_echo()
		 */
		if (tp->t_lflag & ECHOCTL) {
			if (((c & TTY_CHARMASK) < 0x20) && (c != '\t') &&
			    (c != '\n') || (c == 0x7f)) {
				ldtty_output(tp, '^');
				c &= TTY_CHARMASK;
				if (c == 0x7f)
					c = '?';
				else
					c += 'A' - 1;
			}
			/*
			 * we won't echo C1 characters as M-^something.
			 * a lower converter module should have converted
			 * the C1 to its C0 equivalent.
			 */
		}
		if ((tp->t_lflag & XCASE) && (c == '\\')) {
			ldtty_output(tp, c | TTY_QUOTE);
			return;
		}
		ldtty_output(tp, c);
		return;
	}
	else {
		/*
		 * first byte of codeset 1
		 */
		tp->t_rocount--;	/* cuz ldtty_input incremented it */
		tp->t_codeset = 1;
		tp->t_eucleft = tp->t_cswidth.eucw[1] - 1;
		tp->t_eucbytes[0] = c & TTY_CHARMASK;
		tp->t_eucind = 1;
	}
	/*
	 * echo the multi-byte EUC if it is complete
	 */
	if (!tp->t_eucleft) {
		register int col_old = tp->t_col;

		for (i = 0; i < tp->t_eucind; i++)
			ldtty_output(tp, tp->t_eucbytes[i]);
		tp->t_rocount += tp->t_cswidth.scrw[tp->t_codeset];
		tp->t_col = col_old + tp->t_cswidth.scrw[tp->t_codeset];
	}
}

/*
 * erase one EUC character
 */
void
euctty_erase(register struct ldtty *tp)
{
	unsigned char rub_c;
	register int eucleft;
	unsigned char eucbytes[8];
	register int i;

	/*
	 * if partial characters were being stored, just throw them away
	 * they were never echoed, so don't have to rub out
	 */
	if (tp->t_eucleft) {
		tp->t_eucleft = 0;
		return;
	}
	if (!tp->t_rawcc)
		return;
	/*
	 * remove the last EUC character from t_rawbuf
	 */
	rub_c = ldtty_unstuffc(tp);
	eucleft = euctty_state(tp);
	if (eucleft == 0) {
		/*
		 * single byte character
		 */
		ldtty_rub(tp, rub_c);
	}
	else {
		/*
		 * multi-byte character
		 */
		eucbytes[eucleft] = rub_c;
		for (i = eucleft - 1; i >= 0; i--)
			eucbytes[i] = ldtty_unstuffc(tp);
		eucbytes[eucleft + 1] = 0;
		euctty_rub(tp, eucbytes);
	}
}

/*
 * kill a line containing EUC characters
 */
void
euctty_kill(register struct ldtty *tp)
{
	tp->t_eucleft = 0;	/* partial EUC char */
	if ((tp->t_lflag & ECHOE) &&
	    (euctty_rocount(tp) == tp->t_rocount) &&
	    !(tp->t_lflag & ECHOPRT)) {
		/*
		 * it's possible to optimize this..
		 */
		while (tp->t_rawcc) {
			euctty_erase(tp);
		}
	}
	else {
		ldtty_echo(tp, tp->t_cc[VKILL]);
		if ((tp->t_lflag & ECHOK) || (tp->t_lflag & ECHOKE))
			ldtty_echo(tp, '\n');
		if (tp->t_rawbuf)
			ldtty_bufreset(tp);
		tp->t_rocount = 0;
	}
	tp->t_state &= ~TS_LOCAL;
}

/*
 * rub out a mb EUC string as cleanly as possible
 */
PRIVATE_STATIC void
euctty_rub(register struct ldtty *tp, register unsigned char *eucstring)
{
	if ((tp->t_lflag & ECHO) == 0)
		return;
	tp->t_lflag &= ~FLUSHO;
	if (tp->t_lflag & ECHOE) {
		if (tp->t_rocount == 0) {
			/*
			 * screwed by write, retype the line
			 */
			ldtty_retype(tp);
			return;
		}
		/*
		 * rubout the EUC string
		 */
		ldtty_rubo(tp, euctty_scrwidth(tp, eucstring));
	}
	else if (tp->t_lflag & ECHOPRT) {
		if ((tp->t_state & TS_ERASE) == 0) {
			ldtty_output(tp, '\\');
			tp->t_state |= TS_ERASE;
		}
		while (*eucstring)
			ldtty_echo(tp, *eucstring++);
	}
	else
		ldtty_echo(tp, tp->t_cc[VERASE]);
	tp->t_rocount -= euctty_scrwidth(tp, eucstring);
}

/*
 * return the screen character width of the mb EUC string
 */
PRIVATE_STATIC int
euctty_scrwidth(register struct ldtty *tp, register unsigned char *eucstring)
{
	if (eucstring[0] == SS2)
		return(tp->t_cswidth.scrw[2]);
	else if (eucstring[0] == SS3)
		return(tp->t_cswidth.scrw[3]);
	else
		return(tp->t_cswidth.scrw[1]);
}

/*
 * erase one EUC word
 *
 * there is no processing here for multi-byte spaces
 */
void
euctty_werase(register struct ldtty *tp)
{
	int rub_c;

	tp->t_eucleft = 0;	/* partial EUC char */
	/*
	 * erase white space
	 */
	rub_c = ldtty_unstuffc(tp);
	while ((euctty_state(tp) == 0) && ((rub_c == ' ') || (rub_c == '\t'))) {
		ldtty_rub(tp, rub_c);
		rub_c = ldtty_unstuffc(tp);
	}
	if (rub_c == -1)
		return;
	/*
	 * special case last char of token
	 */
	ldtty_stuffc(rub_c, tp);
	euctty_erase(tp);
	rub_c = ldtty_unstuffc(tp);
	if (rub_c == -1)
		return;
	if ((euctty_state(tp) == 0) && ((rub_c == ' ') || (rub_c == '\t'))) {
		ldtty_stuffc(rub_c, tp);
		return;
	}
	/*
	 * erase rest of token
	 * which can be a mix of multi-byte and single byte characters
	 */
	do {
		ldtty_stuffc(rub_c, tp);
		euctty_erase(tp);
		rub_c = ldtty_unstuffc(tp);
		if (rub_c == -1)
			return;
	} while (euctty_state(tp) || ((rub_c != ' ') && (rub_c != '\t')));
	ldtty_stuffc(rub_c, tp);
}

/*
 * multi-byte EUC write processing
 *
 * make sure that a full multi-byte EUC character gets put out
 * if there is a partial multi-byte EUC, it will be saved in
 *  tp->t_out_eucbytes[], tp->t_out_eucind,
 *  tp->t_out_eucleft, tp->t_out_codeset
 * euctty_writechar() does the real work
 */
int
euctty_write(register struct ldtty *tp, register mblk_t *mp)
{
	register queue_t *q = WR(tp->t_queue);
	unsigned char c;
	register mblk_t *mp1;
	int newc;

	while (mp) {
		while (mp->b_rptr < mp->b_wptr) {
			if (!tp->t_outbuf && !ldtty_getoutbuf(tp))
				goto flow;
			tp->t_rocount = 0;
			c = *mp->b_rptr++;
			newc = -1;
			while (euctty_writechar(tp, c, &newc)) {
				if (newc >= 0)
					*(--mp->b_rptr) = newc;
				if ((tp->t_state & TS_WAITEUC)
				    || ldtty_start(tp))
					goto flow;
				if (newc >= 0) {
					c = *mp->b_rptr++;
					newc = -1;
				}
			}
		}
		mp1 = mp;
		mp = mp->b_cont;
		freeb(mp1);
	}
	if (ldtty_start(tp) == 0)
		return 0;
flow:
	/* flow controlled, return */
	if (mp)
		putbq(q, mp);
	return (-1);
}

/*
 * try to write one char, which may be part of a multi-byte EUC char
 * should only be called by euctty_write()
 */
PRIVATE_STATIC int
euctty_writechar(
	register struct ldtty *tp,
	unsigned char c,
	int *newcp
	)
{

	/*
	 * we are waiting for more chars in a multi-byte EUC
	 */
	if (tp->t_out_eucleft) {
		tp->t_out_eucbytes[tp->t_out_eucind++] = c;
		tp->t_out_eucleft--;
	}
	/*
	 * start of multi-byte (char set 2 or 3)
	 */
	else if (c == SS2) {
		tp->t_out_codeset = 2;
		tp->t_out_eucleft = tp->t_cswidth.eucw[2];
		tp->t_out_eucind = 1;
		tp->t_out_eucbytes[0] = c;
	}
	else if (c == SS3) {
		tp->t_out_codeset = 3;
		tp->t_out_eucleft = tp->t_cswidth.eucw[3];
		tp->t_out_eucind = 1;
		tp->t_out_eucbytes[0] = c;
	}
	/*
	 * could be C0, ascii, C1
	 */
	else if (c < 0xa0) {
		int	newc;

		if ((newc = ldtty_output(tp, c)) >= 0) {
			*newcp = newc;
			return(-1);
		}
		return(0);
	}
	/*
	 * start of multi-byte (char set 1)
	 */
	else {
		tp->t_out_codeset = 1;
		tp->t_out_eucleft = tp->t_cswidth.eucw[1] - 1;
		tp->t_out_eucind = 1;
		tp->t_out_eucbytes[0] = c;
	}
	/*
	 * echo the multi-byte EUC if it is complete
	 */
	if (!tp->t_out_eucleft)
		if (euctty_out(tp) != 0)
			return(-1);

	return(0);
}

int
euctty_out(register struct ldtty *tp)
{
	register mblk_t 	*op;
	int			i;

	if (!tp->t_outbuf)
		if (!ldtty_getoutbuf(tp))
			return(-1);

	op = tp->t_outbuf;

	if (op->b_wptr + tp->t_out_eucind >= op->b_datap->db_lim) {
		if (ldtty_start(tp) || (ldtty_getoutbuf(tp) == 0)) {
			tp->t_state |= TS_WAITEUC;
			return(-1);
		}
		op = tp->t_outbuf;
	}
	/*
	 * put the EUC char into the output buffer
	 * update tp->t_col
	 */
	for (i = 0; i < tp->t_out_eucind; i++)
		*op->b_wptr++ = tp->t_out_eucbytes[i];
	tp->t_col += tp->t_cswidth.scrw[tp->t_out_codeset];
	tp->t_state &= ~TS_WAITEUC;
	return(0);
}
