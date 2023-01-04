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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: print.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/06/10 17:04:12 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: p60ths psecs p2dig displaychar draino flush plist
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.11  com/cmd/csh/print.c, cmdcsh, bos320, 9140320 9/25/91 09:33:02	
 */

#include <sys/ioctl.h>
#include "sh.h"


void
p60ths(long l)
{
	l += 3;
	csh_printf("%d.%d", (int) (l / 60), (int) ((l % 60) / 6));
}

void
psecs(long l)
{
	register int i;

	i = l / 3600;
	if (i) {
		csh_printf("%d:", i);
		i = l % 3600;
		p2dig(i / 60);
		goto minsec;
	}
	i = l;
	csh_printf("%d", i / 60);
minsec:
	i %= 60;
	display_char(':');
	p2dig(i);
}

void
p2dig(register int i)
{
	csh_printf("%d%d", i / 10, i % 10);
}

uchar_t linbuf[128];
uchar_t *linp = linbuf;
static int quoted = 0;
bool	print_control_codes = 0; /* Don't print control codes, display
				    them as ^[, etc. */

#ifndef _SBCS
/*
 * If c is NLQUOTE, then the next "character" must be reconstructed
 * to determine how many bytes are actually quoted.  The only way
 * to solve this is 1) follow NLQUOTE with the number of quoted bytes
 * or 2) convert everything to process codes.
 *
 * If the last few characters are preceeded by an invalid character,
 * they will be lost when draino() is called because there is no way
 * to determine whether a character is invalid vs. insufficient text
 * to complete. The multibyte version of flush() will empty the
 * remaining ilsbuf[] bytes.
 */

static uchar_t ilsbuf[MB_LEN_MAX];
static uchar_t *pils = NULL;

void
display_char(register int c)
{
	if (!quoted) {
		if (c != NLQUOTE)
			put_one_char(c);
		else {
			quoted++;
			pils = ilsbuf;
		}
	} else if (mb_cur_max == 1) {
		put_one_char(c);
		quoted = 0;
	} else {
		register int n;

		*pils++ = c;
		n = mblen((char *)ilsbuf, pils - ilsbuf);
		if (n > 0) {
			pils = ilsbuf;
			do
				put_one_char(*pils++);
			while (--n > 0);
			quoted = 0;
		} else if (pils - ilsbuf >= mb_cur_max) {
			uchar_t *ptmp;

			ptmp = ilsbuf;
			put_one_char(*ptmp++);
			n = pils - ptmp;
			quoted = 0;
			while (n-- > 0)
				display_char (*ptmp++);
		}
	}
	return;
}

void
put_one_char(register int c)
{
	if (!print_control_codes && !quoted &&
	    (c < ' ' && c != '\t' && c != '\n' && c != '\b')) {	
		*linp++ = '^';
		c |= 'A' - 1;
	}
	*linp++ = c;
	if (c == '\n' || linp > &linbuf[sizeof linbuf - MB_LEN_MAX])
		flush_now();
}
#else

/*
 *  if c is NLQUOTE, don't output it, but TRIM the next character.
 *  if previous character was not NLQUOTE, don't TRIM this character.
 */
void
display_char(register int c)
{
	if(quoted) {
		quoted = 0;
		c &= TRIM;
	}
	else {
		if (c == NLQUOTE) {
			quoted++;
			return;
		}
		if (!print_control_codes &&
		    ((c < 034 && c != '\t' && c != '\n' && c != '\b') || 
		     c == 0177)) {	
			*linp++ = '^';
			if (c == 0177)
				c = '?';
			else
				c |= 'A' - 1;
		}
	}
	*linp++ = c;
	if (c == '\n' || linp > &linbuf[sizeof linbuf - 2])
		flush();
}
#endif

void
draino(void)
{
	quoted = 0;
	linp = linbuf;
}

#ifndef _SBCS
/*
 * Empty ilsbuf[] before output - avoid infinite recursion loop by
 * always eliminating one byte in ilsbuf[] each time flush() is called
 *
 * Assume first ilsbuf[] byte is invalid multibyte character and process
 * remaining ilsbuf[] through display_char() again
 */
void
flush(void)
{
	if (quoted && pils > ilsbuf) {
		register int n;
		register uchar_t *ptmp;
 
		ptmp = ilsbuf;
		put_one_char(*ptmp++);
		n = pils - ptmp;
		quoted = 0;
		while (n-- > 0)
			display_char (*ptmp++);
		flush();
	}
	flush_now();
	quoted = 0;
}
#endif

void
#ifndef _SBCS
flush_now()
#else
flush(void)
#endif
{
	register int unit;
	int ret, count;
	int lmode = 0;

	if (linp == linbuf)
		return;
	if (haderr)
		unit = didfds ? 2 : SHDIAG;
	else
		unit = didfds ? 1 : SHOUT;

	if (didfds==0 && ioctl(unit, TIOCLGET, &lmode)==0 && lmode & LFLUSHO) {	
			lmode = LFLUSHO;
			IOCTL(unit, TIOCLBIC, &lmode, "43");
			write(unit, "\n", 1);
	}
	count = linp - linbuf;
	ret = write(unit, linbuf, count);
	linp = linbuf;
	/*
	 * This seems like a good idea, but many things don't expect this.
	 * 
	 * if (ret != count) {
	 * 	Perror("write");
	 * }
	 */
}

void
plist(register struct varent *vp)
{

	if (setintr)
		sigrelse(SIGINT);
	for (vp = vp->link; vp != 0; vp = vp->link) {
		int len = blklen(vp->vec);

		csh_printf((char *)vp->name);
		csh_printf("\t");
		if (len != 1)
			display_char('(');
		blkpr(vp->vec);
		if (len != 1)
			display_char(')');
		csh_printf("\n");
	}
	if (setintr)
		sigrelse(SIGINT);
}
