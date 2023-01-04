
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
static char rcsid[] = "@(#)$RCSfile: line.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:06:26 $";
#endif
/* 
 * COMPONENT_NAME: UUCP line.c
 * 
 * FUNCTIONS: fixline, genbrk, restline, savline, sethup, setline, 
 *            sytfix2line, sytfixline 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
line.c	1.4  com/cmd/uucp,3.1,9013 1/9/90 16:30:21";
*/
/*	uucp:line.c	1.4
*/
#include "uucp.h"
/* VERSION( line.c	5.2 -  -  ); */

static struct sg_spds {
	int	sp_val,
		sp_name;
} spds[] = {
	{ 300,  B300},
	{ 600,  B600},
	{1200, B1200},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
#ifdef EXTA
	{19200,	EXTA},
#endif
#ifdef B19200
	{19200,	B19200},
#endif
#ifdef B38400
	{38400,	B38400},
#endif
	{0,    0}
};

#define PACKSIZE	64
#define HEADERSIZE	6
#define SNDFILE	'S'
#define RCVFILE 'R'
#define RESET	'X'

int	linebaudrate = 0;	/* for speedup hook in pk (unused in ATTSV) */

#ifdef ATTSV

static struct termio Savettyb;
/*
 * set speed/echo/mode...
 *	tty 	-> terminal name
 *	spwant 	-> speed
 *	type	-> type
 *
 *	if spwant == 0, speed is untouched
 *	type is unused, but needed for compatibility
 *
 * return:  
 *	none
 */
fixline(tty, spwant, type)
int	tty, spwant, type;
{
	register struct sg_spds	*ps;
	struct termio		ttbuf;
	int			speed = -1;

	DEBUG(6, "fixline(%d, ", tty);
	DEBUG(6, "%d)\n", spwant);
	if (ioctl(tty, TCGETA, &ttbuf) != 0)
		return;
	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		ASSERT(speed >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", speed);
		ttbuf.c_cflag = speed;
	} else
		ttbuf.c_cflag &= CBAUD;
	ttbuf.c_iflag = ttbuf.c_oflag = ttbuf.c_lflag = (ushort)0;

#ifdef NO_MODEM_CTRL
	/*   CLOCAL may cause problems on pdp11s with DHs */
	if (type == D_DIRECT) {
		DEBUG(4, "fixline - direct\n", "");
		ttbuf.c_cflag |= CLOCAL;
	} else
#endif

		ttbuf.c_cflag &= ~CLOCAL;
	ttbuf.c_cflag |= (CS8 | CREAD | (speed ? HUPCL : 0));
	ttbuf.c_cc[VMIN] = 1;
	ttbuf.c_cc[VTIME] = 1;
	
	ASSERT(ioctl(tty, TCSETA, &ttbuf) >= 0,
	    MSGSTR(MSG_LINEA2,"RETURN FROM fixline ioctl"), "", errno);
	return;
}

sethup(dcf)
int	dcf;
{
	struct termio ttbuf;

	if (ioctl(dcf, TCGETA, &ttbuf) != 0)
		return;
	if (!(ttbuf.c_cflag & HUPCL)) {
		ttbuf.c_cflag |= HUPCL;
		(void) ioctl(dcf, TCSETA, &ttbuf);
	}
}

genbrk(fn)
register int	fn;
{
	if (isatty(fn)) 
		(void) ioctl(fn, TCSBRK, 0);
}


/*
 * optimize line setting for sending or receiving files
 * return:
 *	none
 */
setline(type)
register char	type;
{
	static struct termio tbuf;
	
	if (ioctl(Ifn, TCGETA, &tbuf) != 0)
		return;
	DEBUG(2, "setline - %c\n", type);
	switch (type) {
	case RCVFILE:
		if (tbuf.c_cc[VMIN] != 1) {
		    tbuf.c_cc[VMIN] = 1;
		    (void) ioctl(Ifn, TCSETAW, &tbuf);
		}
		break;

	case SNDFILE:
	case RESET:
		if (tbuf.c_cc[VMIN] != 1) {
		    tbuf.c_cc[VMIN] = 1;
		    (void) ioctl(Ifn, TCSETAW, &tbuf);
		}
		break;
	}
}

savline()
{
	int ret;

	ret = ioctl(0, TCGETA, &Savettyb);
	Savettyb.c_cflag = (Savettyb.c_cflag & ~CS8) | CS7;
	Savettyb.c_oflag |= OPOST;
	Savettyb.c_lflag |= (ISIG|ICANON|ECHO);
	return(ret);
}

/***
 *	sytfixline(tty, spwant)	set speed/echo/mode...
 *	int tty, spwant;
 *
 *	return codes:  none
 */

sytfixline(tty, spwant)
int tty, spwant;
{
	struct termio ttbuf;
	struct sg_spds *ps;
	int speed = -1;
	int ret;

	for (ps = spds; ps->sp_val >= 0; ps++)
		if (ps->sp_val == spwant)
			speed = ps->sp_name;
	DEBUG(4, "sytfixline - speed= %d\n", speed);
	ASSERT(speed >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", speed);
	(void) ioctl(tty, TCGETA, &ttbuf);
	ttbuf.c_iflag = (ushort)0;
	ttbuf.c_oflag = (ushort)0;
	ttbuf.c_lflag = (ushort)0;
	ttbuf.c_cflag = speed;
	ttbuf.c_cflag |= (CS8|CLOCAL);
	ttbuf.c_cc[VMIN] = 1;
	ttbuf.c_cc[VTIME] = 1;
	ret = ioctl(tty, TCSETAW, &ttbuf);
	ASSERT(ret >= 0, MSGSTR(MSG_LINEA4,"RETURN FROM sytfixline"), "", ret);
	return;
}

sytfix2line(tty)
int tty;
{
	struct termio ttbuf;
	int ret;

	(void) ioctl(tty, TCGETA, &ttbuf);
	ttbuf.c_cflag &= ~CLOCAL;
	ttbuf.c_cflag |= CREAD|HUPCL;
	ret = ioctl(tty, TCSETAW, &ttbuf);
	ASSERT(ret >= 0, MSGSTR(MSG_LINEA5,"RETURN FROM sytfix2line"), "", ret);
	return;
}


restline()
{
	return(ioctl(0, TCSETA, &Savettyb));
}

#else /* !ATTSV */

static struct sgttyb Savettyb;

/***
 *	fixline(tty, spwant, type)	set speed/echo/mode...
 *	int tty, spwant;
 *
 *	if spwant == 0, speed is untouched
 *	type is unused, but needed for compatibility
 *
 *	return codes:  none
 */

/*ARGSUSED*/
fixline(tty, spwant, type)
int tty, spwant, type;
{
	struct sgttyb	ttbuf;
	struct sg_spds	*ps;
	int		 speed = -1;

	DEBUG(6, "fixline(%d, ", tty);
	DEBUG(6, "%d)\n", spwant);

	if (ioctl(tty, TIOCGETP, &ttbuf) != 0)
		return;
	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		ASSERT(speed >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", speed);
		ttbuf.sg_ispeed = ttbuf.sg_ospeed = speed;
	} else {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_name == ttbuf.sg_ispeed) {
				spwant = ps->sp_val;
				break;
			}
		ASSERT(spwant >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", spwant);
	}
	ttbuf.sg_flags = (ANYP | RAW);
	(void) ioctl(tty, TIOCSETP, &ttbuf);
	(void) ioctl(tty, TIOCHPCL, STBNULL);
	(void) ioctl(tty, TIOCEXCL, STBNULL);
	linebaudrate = spwant;		/* for hacks in pk driver */
	return;
}

sethup(dcf)
int	dcf;
{
	if (isatty(dcf)) 
		(void) ioctl(dcf, TIOCHPCL, STBNULL);
}

/***
 *	genbrk		send a break
 *
 *	return codes;  none
 */

genbrk(fn)
{
	if (isatty(fn)) {
		(void) ioctl(fn, TIOCSBRK, 0);
		nap(HZ/10);				/* 0.1 second break */
		(void) ioctl(fn, TIOCCBRK, 0);
	}
	return;
}

/*
 * V7 and RT aren't smart enough for this -- linebaudrate is the best
 * they can do.
 */
/*ARGSUSED*/
setline(dummy) { }

savline()
{
	int	ret;

	ret = ioctl(0, TIOCGETP, &Savettyb);
	Savettyb.sg_flags |= ECHO;
	Savettyb.sg_flags &= ~RAW;
	return(ret);
}

restline()
{
	return(ioctl(0, TIOCSETP, &Savettyb));
}
#endif
