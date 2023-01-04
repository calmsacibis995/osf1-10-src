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
 *	@(#)$RCSfile: termios.h,v $ $Revision: 4.3.11.4 $ (DEC) $Date: 1993/08/03 18:28:22 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * Change log
 *
 */
/*
 *  termios structure
 */
#ifndef _SYS_TERMIOS_H_
#define _SYS_TERMIOS_H_

#include <standards.h>

#ifdef _POSIX_SOURCE

typedef unsigned int    tcflag_t;
typedef unsigned char   cc_t;
typedef unsigned int    speed_t;


/* 
 * Special Control Characters 
 *
 * Index into c_cc[] character array.
 *
 *	Name	     Subscript	Enabled by 
 */
#define	VEOF		0	/* ICANON */
#define	VEOL		1	/* ICANON */

#ifdef _OSF_SOURCE
#define	VEOL2		2	/* ICANON */
#endif

#define	VERASE		3	/* ICANON */

#ifdef _OSF_SOURCE
#define VWERASE 	4	/* ICANON */
#endif

#define VKILL		5	/* ICANON */

#ifdef _OSF_SOURCE
#define	VREPRINT 	6	/* ICANON */
#endif

/*			7	   spare 1 */

#define VINTR		8	/* ISIG */
#define VQUIT		9	/* ISIG */
#define VSUSP		10	/* ISIG */

#ifdef _OSF_SOURCE
#define VDSUSP		11	/* ISIG */
#endif

#define VSTART		12	/* IXON, IXOFF */
#define VSTOP		13	/* IXON, IXOFF */

#ifdef _OSF_SOURCE
#define	VLNEXT		14	/* IEXTEN */
#define	VDISCARD	15	/* IEXTEN */
#define	VFLUSH		VDISCARD /* for sun */
#endif

#define VMIN		16	/* !ICANON */
#define VTIME		17	/* !ICANON */

#ifdef _OSF_SOURCE
#define VSTATUS		18	/* ISIG */
#endif

/*			19	   spare 2 */

#define	NCCS		20

/*
 * Ioctl control packet
 */
struct termios {
	tcflag_t	c_iflag;	/* input flags */
	tcflag_t	c_oflag;	/* output flags */
	tcflag_t	c_cflag;	/* control flags */
	tcflag_t	c_lflag;	/* local flags */
	cc_t		c_cc[NCCS];	/* control chars */
	int		c_ispeed;	/* input speed */
	int		c_ospeed;	/* output speed */
};

#ifdef _KERNEL
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE	(0377)
#endif	/* _POSIX_VDISABLE */
#endif	/* _KERNEL */

#ifdef _NO_PROTO
extern speed_t cfgetospeed();
extern int cfsetospeed();
extern speed_t cfgetispeed();
extern int cfsetispeed();
extern int tcgetattr();
extern int tcsetattr();
extern int tcsendbreak();
extern int tcdrain();
extern int tcflush();
extern int tcflow();

#else                           /* use POSIX required prototypes */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern speed_t cfgetospeed(const struct termios *);
extern int cfsetospeed(struct termios *, speed_t);
extern speed_t cfgetispeed(const struct termios *);
extern int cfsetispeed(struct termios *, speed_t);
extern int tcgetattr(int , struct termios *);
extern int tcsetattr(int , int , const struct termios *);
extern int tcsendbreak(int , int );
extern int tcdrain(int );
extern int tcflush(int , int );
extern int tcflow(int , int );
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */

/*
 * Input flags - software input processing
 */
#define	IGNBRK		0x00000001	/* ignore BREAK condition */
#define	BRKINT		0x00000002	/* map BREAK to SIGINTR */
#define	IGNPAR		0x00000004	/* ignore (discard) parity errors */
#define	PARMRK		0x00000008	/* mark parity and framing errors */
#define	INPCK		0x00000010	/* disable checking of parity errors */
#define	ISTRIP		0x00000020	/* strip 8th bit off chars */
#define	INLCR		0x00000040	/* map NL into CR */
#define	IGNCR		0x00000080	/* ignore CR */
#define	ICRNL		0x00000100	/* map CR to NL (ala CRMOD) */
#define	IXON		0x00000200	/* enable output flow control */
#define	IXOFF		0x00000400	/* enable input flow control */
#ifdef _XOPEN_SOURCE
#define	IXANY		0x00000800	/* any char will restart after stop */
#define IUCLC		0x00001000	/* DUMMY VALUE Map upper to lower */
					/* case on input */
#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE
#define	IFLOW		IXON		/* enable output flow control */
#define	ITANDEM		IXOFF		/* enable input flow control */
#define IMAXBEL		0x00002000	/* ring bell on input queue full */
#endif /* _OSF_SOURCE */

/*
 * Output flags - software output processing
 */
#define	OPOST		0x00000001	/* enable following output processing */

#ifdef _XOPEN_SOURCE

#define ONLCR		0x00000002	/* map NL to CR-NL (ala CRMOD) */
#define OLCUC           0x00000004	/* Map lower case to upper on output */
#define OCRNL           0x00000008	/* Map CR to NL on output */
#define ONOCR           0x00000010	/* No CR output at column 0 */
#define ONLRET          0x00000020	/* NL performs CR function */
#define OFILL           0x00000040	/* Use fill characters for delay */
#define OFDEL           0x00000080	/* fill is DEL, else NUL */

#define		NLDLY		0x00000300	/* \n delay */
#define			NL0	0x00000000
#define			NL1	0x00000100	/* tty 37 */
#ifdef _OSF_SOURCE				/* to keep with XPG4 compat. */
#define			NL2	0x00000200	/* vt05 */
#define			NL3	0x00000300
#endif /* _OSF_SOURCE */
#define		TABDLY		0x00000c00	/* horizontal tab delay */
#define			TAB0	0x00000000
#define			TAB1	0x00000400	/* tty 37 */
#define			TAB2	0x00000800
#define			TAB3	0x00000C00	/* expand tabs on output */
#define		CRDLY		0x00003000	/* \r delay */
#define			CR0	0x00000000
#define			CR1	0x00001000	/* tn 300 */
#define			CR2	0x00002000	/* tty 37 */
#define			CR3	0x00003000	/* concept 100 */
#define 	FFDLY           0x00004000	/* Form feed delay */
#define 		FF0	0x00000000
#define 		FF1	0x00004000
#define		BSDLY		0x00008000	/* \b delay */
#define			BS0	0x00000000
#define			BS1	0x00008000
#define		VTDLY		0x00010000	/* vertical tab delay */
#define			VT0	0x00000000
#define			VT1	0x00010000	/* tty 37 */

#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE
#define ONLCRNL		ONLCR
#define OXTABS		0x00040000	/* expand tabs to spaces */
#define ONOEOT		0x00080000	/* discard EOT's (^D) on output) */
#endif /* _OSF_SOURCE */

/*
 * Control flags - hardware control of terminal
 */
#define CSIZE		0x00000300	/* character size mask */
#define     CS5		    0x00000000	    /* 5 bits (pseudo) */
#define     CS6		    0x00000100	    /* 6 bits */
#define     CS7		    0x00000200	    /* 7 bits */
#define     CS8		    0x00000300	    /* 8 bits */
#define CSTOPB		0x00000400	/* send 2 stop bits */
#define CREAD		0x00000800	/* enable receiver */
#define PARENB		0x00001000	/* parity enable */
#define PARODD		0x00002000	/* odd parity, else even */
#define HUPCL		0x00004000	/* hang up on last close */
#define CLOCAL		0x00008000	/* ignore modem status lines */

#ifdef _OSF_SOURCE
#define CRTSCTS		0x00010000	/* RTS/CTS flow control */
#endif /* _OSF_SOURCE */


/* 
 * "Local" flags - dumping ground for other state
 *
 * Warning: some flags in this structure begin with
 * the letter "I" and look like they belong in the
 * input flag.
 */

#define	ECHOE		0x00000002	/* visually erase chars */
#define	ECHOK		0x00000004	/* echo NL after line kill */
#define ECHO		0x00000008	/* enable echoing */
#define	ECHONL		0x00000010	/* echo NL even if ECHO is off */
#define	ISIG		0x00000080	/* enable signals INTR, QUIT, [D]SUSP */
#define	ICANON		0x00000100	/* canonicalize input lines */
#define	IEXTEN		0x00000400	/* enable FLUSHO and LNEXT */
#define	NOFLSH		0x80000000	/* don't flush after interrupt */
#define TOSTOP		0x00400000	/* stop background jobs from output */

#ifdef _XOPEN_SOURCE
#define XCASE		0x00004000	/* Cononical upper/lower presentation*/
#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE
#define	ECHOKE		0x00000001	/* visual erase for line kill */
#define	ECHOPRT		0x00000020	/* visual erase mode for hardcopy */
#define ECHOCTL  	0x00000040	/* echo control chars as ^(Char) */
#define ALTWERASE	0x00000200	/* use alternate WERASE algorithm */
#define	MDMBUF		0x00100000	/* flow control output via Carrier */
#define FLUSHO		0x00800000	/* output being flushed (state) */
#define	NOHANG		0x01000000	/* XXX this should go away */
#define PENDIN		0x20000000	/* retype pending input (state) */
#define NOKERNINFO      0x40000000      /* Disable printing kernel info */
#endif /* _OSF_SOURCE */


/* 
 * Commands passed to tcsetattr() for setting the termios structure.
 */
#define	TCSANOW		0		/* make change immediate */
#define	TCSADRAIN	1		/* drain output, then change */
#define	TCSAFLUSH	2		/* drain output, flush input */

/* values for the queue_selector argument to tcflush() */

#define TCIFLUSH        0
#define TCOFLUSH        1
#define TCIOFLUSH       2

/* values for the action argument to tcflow() */

#define TCOOFF          0
#define TCOON           1
#define TCIOFF          2
#define TCION           3

#if !defined(_SYS_TERMIO_H_)
/*
 * If termios functionality is used and termio is not used then always
 * specify _USE_NEW_TTY. This is a bit of a hack, but ioctl.h defines 
 * _USE_OLD_TTY by default which defines the wrong speed values.
 */
#undef _USE_OLD_TTY
#define _USE_NEW_TTY

#undef B0
#undef B50
#undef B75
#undef B110
#undef B134
#undef B150
#undef B200
#undef B300
#undef B600
#undef B1200
#undef B1800
#undef B2400
#undef B4800
#undef B9600
#undef B19200
#undef B38400
#undef EXTA
#undef EXTB
#endif	/* !defined(_SYS_TERMIO_H_) */

#ifndef	_USE_OLD_TTY
/*
 * Standard speeds
 */
#define B0	0
#define B50	50
#define B75	75
#define B110	110
#define B134	134
#define B150	150
#define B200	200
#define B300	300
#define B600	600
#define B1200	1200
#define	B1800	1800
#define B2400	2400
#define B4800	4800
#define B9600	9600
#define B19200	19200
#define B38400	38400
#define EXTA	19200
#define EXTB	38400
#endif	/* ! _USE_OLD_TTY */
/*
 * END OF PROTECTED INCLUDE.
 */


#endif /* _POSIX_SOURCE */

#endif /* _SYS_TERMIOS_H_ */

#ifdef _OSF_SOURCE
#include <sys/ttydefaults.h>
#endif
