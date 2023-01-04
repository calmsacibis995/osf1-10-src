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
static char rcsid[] = "@(#)$RCSfile: print.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/06/10 16:01:34 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.16  com/cmd/sh/sh/print.c, bos320, 9142320i 10/16/91 09:54:07	
 */

#include	"defs.h"
#include	<time.h>
#include	<sys/limits.h>

#define		BUFLEN		256

static uchar_t	buffer[BUFLEN];
static int	index = 0;
uchar_t		numbuf[17];	/*  changed from 12 to 14   */
                                /*  changed form 14 to 17   */

void	prc_buff();
void	prs_buff();
void	prn_buff();
void	prs_cntl();
void	prn_buff();
void	itos();
void	prs();

/*
 * printing and io conversion
 */
void
prp()
{
	if ((flags & prompt) == 0 && cmdadr)
	{
		prs_cntl(cmdadr);
		prs(colon);
	}
}

void
prs(as)
uchar_t	*as;
{
	register uchar_t	*s;

	if (s = as) {
		if (NLSisencoded(s))
			s = NLSndecode(s);
		write(output, s, strlen((char *)s));
	}
}

void
prc(c)
uchar_t	c;
{
	if (c)
		write(output, &c, 1);
}

void
prt(t)
clock_t	t;
{
	register int hr, min, sec;

	t += CLK_TCK / 2;
	t /= CLK_TCK;
	sec = t % CLK_TCK;
	t /= CLK_TCK;
	min = t % CLK_TCK;

	if (hr = t / CLK_TCK)
	{
		prn_buff(hr);
		prc_buff('h');
	}

	prn_buff(min);
	prc_buff('m');
	prn_buff(sec);
	prc_buff('s');
}

void
prn(n)
	int	n;
{
	itos(n);

	prs(numbuf);
}

void
itos(n)
	int n;
{
	register uchar_t *abuf;
	register unsigned a, i;
	int pr, d;

	abuf = numbuf;

	pr = FALSE;
	a = n;
	for (i = 1000000000; i != 1; i /= 10)
	{
		if ((pr |= (d = a / i)))
			*abuf++ = d + '0';
		a %= i;
	}
	*abuf++ = a + '0';
	*abuf++ = 0;
}

int
stoi(icp)
uchar_t	*icp;
{
	register uchar_t	*cp = icp;
	register int	r = 0;
	register uchar_t	c;

	if (NLSisencoded(icp)) 
		NLSdecode(icp);

	while ((c = *cp, digit(c)) && c && r >= 0)
	{
		r = r * 10 + c - '0';
		cp++;
	}
	if (r < 0 || cp == icp)
		failed(icp, MSGSTR(M_BADNUM,(char *)badnum));
	else
		return(r);
}

void
prl(n)
long n;
{
	int i;

	i = 11;
	while (n > 0 && --i >= 0)
	{
		numbuf[i] = n % 10 + '0';
		n /= 10;
	}
	numbuf[11] = '\0';
	prs_buff(&numbuf[i]);
}

void
flushb()
{
	if (index)
	{
		buffer[index] = '\0';
		write(1, buffer, strlen((char *)buffer));
		index = 0;
	}
}

void
prc_buff(c)
	uchar_t c;
{
	if (c)
	{
		if (index + 1 >= BUFLEN)
			flushb();

		buffer[index++] = c;
	}
	else
	{
		flushb();
		write(1, &c, 1);
	}
}

void
prs_buff(s)
	uchar_t *s;
{
	register int len;
	if (NLSisencoded(s))
		s = NLSndecode(s);
	len = strlen((char *)s);

	if (index + len >= BUFLEN)
		flushb();

	if (len >= BUFLEN)
		write(1, s, len);
	else
	{
		movstr(s, &buffer[index]);
		index += len;
	}
}

void
clear_buff()
{
	index = 0;
}


void
prs_cntl(s)
	uchar_t *s;
{
	register uchar_t *ptr = buffer;
	register int c;
	register int mblength;

	if (NLSisencoded(s))
		s++;
/*
 * this line was changed to check the boundry of the buffer so
 * that it doesn't exceed BUFLEN, because when it does it trashes
 * another static variable "cwdname" in pwd.c and causes the 
 * internal pwd command to become broken when the input is 
 * >127 control characters, dewey coffman, 7-30-87
 */

	while((*s != '\0') && ((ptr - buffer) < (BUFLEN - 1)))
	{
		/* translate a control character into a printable sequence */
		c = *s;
# ifdef _SBCS
		if (fontshift(c) && s[1])
		{
			*ptr++ = c;
			c = *++s;
		}
#else
		mblength = mblen((char *)s, MBMAX);
		if (mblength > 1) {
			while (mblength--) {
				*ptr++ = *s++;
			}
			continue;
		}
# endif
#ifdef _SBCS
		if ((c < '\040')
			&& (c != '\n') && (c != '\t'))
#else
		if ((c < '\040') && (!NLSfontshift (c))
			&& (c != '\n') && (c != '\t'))
		/* magic font shifts will be handled in prs() */
#endif
		{	/* assumes ASCII uchar_t */
			*ptr++ = '^';
			*ptr++ = (c + 0100);	/* assumes ASCII uchar_t */
		}
#ifdef _SBCS
		/* keep 0177s in - these are FSH0 uchar_ts handled by prs() */
#else
		else if (c == 0177)
		{	/* '\0177' does not work */
			*ptr++ = '^';
			*ptr++ = '?';
		}
#endif
		else 
		{	/* printable character */
			*ptr++ = c;
		}

		++s;
	}

	*ptr = '\0';
	prs(buffer);
}

void
prn_buff(n)
	int	n;
{
	itos(n);

	prs_buff(numbuf);
}
