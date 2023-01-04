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
 *	@(#)$RCSfile: ioctl.h,v $ $Revision: 4.4.14.10 $ (DEC) $Date: 1993/11/27 13:37:09 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ioctl.h	7.12 (Berkeley) 5/18/90
 */


/*
 * Function prototype for the ioctl system call.
 */

#ifndef _KERNEL
#ifdef _NO_PROTO
extern int	ioctl();
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
extern int 	ioctl(int, int, ...);
}
#endif
#endif
#endif /* _NO_PROTO */
#endif /* _KERNEL */

/*
 * Ioctl definitions
 */

#ifndef	_SYS_IOCTL_H_
#define _SYS_IOCTL_H_

#include <sys/types.h>
#include <sys/secdefines.h>

/*
 * Window/terminal size structure.
 * This information is stored by the kernel
 * in order to provide a consistent interface,
 * but is not used by the kernel.
 */
struct winsize {
	unsigned short	ws_row;			/* rows, in characters */
	unsigned short	ws_col;			/* columns, in characters */
	unsigned short	ws_xpixel;		/* horizontal size, pixels */
	unsigned short	ws_ypixel;		/* vertical size, pixels */
};

/*
 * Pun for SUN.
 */
struct ttysize {
	unsigned short	ts_lines;
	unsigned short	ts_cols;
	unsigned short	ts_xxx;
	unsigned short	ts_yyy;
};
#define	TIOCGSIZE	TIOCGWINSZ
#define	TIOCSSIZE	TIOCSWINSZ

#if	defined(_KERNEL) && defined(sun)
/*
 * The following structure and ioctl's have been added to allow
 * Suntools to run under Mach.
 */

/*
 * Sun version of the winsize struct.
 */
struct swsize {
	int	ts_lines;	/* number of lines on terminal */
	int	ts_cols;	/* number of columns on terminal */
};

/*
 * These ioctl's are used in Suntools.  Although they may seem to conflict
 * with existing 4.3 ioctl's, either the resulting value is different
 * from the other ioctl with the same number or there is code in the
 * appropriate driver to distinguish between them.
 */
#define	_O_TIOCSSIZE	_IOW('t',103,struct swsize)	/* set tty size */
#define	_O_TIOCGSIZE	_IOR('t',102,struct swsize)	/* get tty size */
#define	_N_TIOCSSIZE	_IOW('t',37,struct swsize)	/* set tty size */
#define	_N_TIOCGSIZE	_IOR('t',38,struct swsize)	/* get tty size */

#endif	/* _KERNEL && sun */


#ifndef _IO
/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 3 bits of the upper word are used
 * to encode the in/out status of the parameter.
 */
#define	IOCPARM_MASK	0x1fff		/* parameter length, at most 13 bits */
#define	IOCPARM_LEN(x)	(((x) >> 16) & IOCPARM_MASK)
#define	IOCBASECMD(x)	((x) & ~IOCPARM_MASK)
#define	IOCGROUP(x)	(((x) >> 8) & 0xff)

#define	IOCPARM_MAX	NBPG		/* max size of ioctl, mult. of NBPG */
#define IOC_VOID	0x20000000	/* no parameters */
#define IOC_OUT		0x40000000	/* copy out parameters */
#define IOC_IN		0x80000000	/* copy in parameters */
#define IOC_INOUT	(IOC_IN|IOC_OUT)
#define	IOC_DIRMASK	0xe0000000	/* mask for IN/OUT/VOID */

#define _IOC(inout,group,num,len) \
	(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define	_IO(g,n)	_IOC(IOC_VOID,	(g), (n), 0)
#define	_IOR(g,n,t)	_IOC(IOC_OUT,	(g), (n), sizeof(t))
#define	_IOW(g,n,t)	_IOC(IOC_IN,	(g), (n), sizeof(t))

/* this should be _IORW, but stdio got there first */
#define	_IOWR(g,n,t)	_IOC(IOC_INOUT,	(g), (n), sizeof(t))
#endif

/*
 * tty ioctl commands
 */
						/* 0-2 compat */
#define	TIOCMODG	_IOR('t', 3, int)	/* get modem control state */
#define	TIOCMODS	_IOW('t', 4, int)	/* set modem control state */
#define		TIOCM_LE	0001		/* line enable */
#define		TIOCM_DTR	0002		/* data terminal ready */
#define		TIOCM_RTS	0004		/* request to send */
#define		TIOCM_ST	0010		/* secondary transmit */
#define		TIOCM_SR	0020		/* secondary receive */
#define		TIOCM_CTS	0040		/* clear to send */
#define		TIOCM_CAR	0100		/* carrier detect */
#define		TIOCM_CD	TIOCM_CAR
#define		TIOCM_RNG	0200		/* ring */
#define		TIOCM_RI	TIOCM_RNG
#define		TIOCM_DSR	0400		/* data set ready */
						/* 8-10 compat */
#define	TIOCEXCL	_IO('t', 13)		/* set exclusive use of tty */
#define	TIOCNXCL	_IO('t', 14)		/* reset exclusive use of tty */
						/* 15 unused */
#define	TIOCFLUSH	_IOW('t', 16, int)	/* flush buffers */
						/* 17-18 compat */
#define	TIOCGETA	_IOR('t', 19, struct termios) /* get termios struct */
#define	TIOCSETA	_IOW('t', 20, struct termios) /* set termios struct */
#define	TIOCSETAW	_IOW('t', 21, struct termios) /* drain output, set */
#define	TIOCSETAF	_IOW('t', 22, struct termios) /* drn out, fls in, set */
/* System V tty ioctls */

#define TCGETS          TIOCGETA
#define TCSETS          TIOCSETA
#define TCSETSW         TIOCSETAW
#define TCSETSF         TIOCSETAF
#define TCGETA	        _IOR('t', 23, struct termio) /* get termio struct */
#define TCSETA	        _IOW('t', 24, struct termio) /* set termio struct */
#define TCSETAW	        _IOW('t', 25, struct termio) /* drain output, set */
#define TCSETAF	        _IOW('t', 28, struct termio) /* drn out, flsh, set */
#define TCSBREAK        _IO('t', 29)		/* Send break */
#define TCSBRK          TCSBREAK
#define TCXONC          _IO('t', 30)		/* Set flow control */
#define TCFLSH          _IO('t', 31)		/* Flush queue */

#define	TIOCGETD	_IOR('t', 26, int)	/* get line discipline */
#define	TIOCSETD	_IOW('t', 27, int)	/* set line discipline */


/* locals, from 127 down */
						/* 127-124 compat */
#define	TIOCSBRK	_IO('t', 123)		/* set break bit */
#define	TIOCCBRK	_IO('t', 122)		/* clear break bit */
#define	TIOCSDTR	_IO('t', 121)		/* set data terminal ready */
#define	TIOCCDTR	_IO('t', 120)		/* clear data terminal ready */
#define	TIOCGPGRP	_IOR('t', 119, pid_t)	/* get pgrp of tty */
#define	TIOCSPGRP	_IOW('t', 118, pid_t)	/* set pgrp of tty */
						/* 117-116 compat */
#define	TIOCOUTQ	_IOR('t', 115, int)	/* output queue size */
#define	TIOCSTI		_IOW('t', 114, char)	/* simulate terminal input */
#define	TIOCNOTTY	_IO('t', 113)		/* void tty association */
#define	TIOCPKT		_IOW('t', 112, int)	/* pty: set/clear packet mode */
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define		TIOCPKT_STOP		0x04	/* stop output */
#define		TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */

#define	TIOCSTOP	_IO('t', 111)		/* stop output, like ^S */
#define	TIOCSTART	_IO('t', 110)		/* start output, like ^Q */
#define	TIOCMSET	_IOW('t', 109, int)	/* set all modem bits */
#define	TIOCMBIS	_IOW('t', 108, int)	/* bis modem bits */
#define	TIOCMBIC	_IOW('t', 107, int)	/* bic modem bits */
#define	TIOCMGET	_IOR('t', 106, int)	/* get all modem bits */
#define	TIOCREMOTE	_IOW('t', 105, int)	/* remote input editing */
#define	TIOCGWINSZ	_IOR('t', 104, struct winsize)	/* get window size */
#define	TIOCSWINSZ	_IOW('t', 103, struct winsize)	/* set window size */
#define	TIOCUCNTL	_IOW('t', 102, int)	/* pty: set/clr usr cntl mode */
#define		UIOCCMD(n)	_IO('u', n)		/* usr cntl op "n" */
#define	TIOCCONS	_IOW('t', 98, int)		/* become virtual console */
#define	TIOCSCTTY	_IO('t', 97)		/* become controlling tty */
#define TIOCEXT		_IOW('t', 96, int)	/* pty: external processing */
#define TIOCSIG		_IO('t', 95)		/* pty: generate signal */

/* SLIP (Serial Line IP) ioctl's */
#define	SLIOGUNIT	_IOR('t', 88, int)	/* get slip unit number */
#define	SLIOCSFLAGS	_IOW('t', 89, int)	/* set configuration flags */
#define	SLIOCGFLAGS	_IOR('t', 90, int)	/* get configuration flags */

#define TTYDISC		0		/* termios tty line discipline */
#define TABLDISC	3		/* tablet discipline */
#define SLIPDISC	4		/* serial IP discipline */
#ifdef	sun
#define MOUSELDISC      5               /* mouse discipline */
#define KBDLDISC        6               /* up/down keyboard trans (console) */
#define NTABLDISC	7		/* gtco tablet discipline */
#endif
#define DUDISC		7		/* Dialup IP discipline */
#define KJIDISC         8               /* Kanji Shift JIS discipline */

/* SYS V REL. 4 PTY IOCTL    */
#define ISPTM           _IO('t',71)     	/* get dev_t  */
#define UNLKPT          _IO('t',73)             /* unlock slave pty */
#define ISPTS           _IO('t',74)             /* ret. maj+min of pty master */

#define	TIOCGSID	_IOR('t', 72, int)	/* get sid of tty */

/*
 * Compatability with old terminal driver
 *
 * Source level -> #define _USE_OLD_TTY
 * Kernel level -> options COMPAT_43
 */
#if	!defined(_KERNEL) && !defined(_USE_OLD_TTY) && !defined(_USE_NEW_TTY)
#define _USE_OLD_TTY 1
#endif

#if defined(_USE_OLD_TTY) || defined(COMPAT_43)
#include <sys/ioctl_compat.h>
#endif

#define	FIOCLEX		_IO('f', 1)		/* set close on exec on fd */
#define	FIONCLEX	_IO('f', 2)		/* remove close on exec */
#define	FIONREAD	_IOR('f', 127, int)	/* get number of bytes to read */
#define	FIONBIO		_IOW('f', 126, int)	/* set/clear non-blocking i/o */
#define	FIOASYNC	_IOW('f', 125, int)	/* set/clear async i/o */
#define	FIOSETOWN	_IOW('f', 124, int)	/* set owner */
#define	FIOGETOWN	_IOR('f', 123, int)	/* get owner */
#define FIOPIPESTAT     _IOR('f', 122, struct stat)     /* pipe|fifo stat */
#define FIOFATTACH      _IOW('f', 121, void *)  /* internal: fattach */
#define FIOFDETACH      _IOW('f', 120, void *)  /* internal: fdetach */

/* socket i/o controls */

#define	SIOCSHIWAT	_IOW('s',  0, int)		/* set high watermark */
#define	SIOCGHIWAT	_IOR('s',  1, int)		/* get high watermark */
#define	SIOCSLOWAT	_IOW('s',  2, int)		/* set low watermark */
#define	SIOCGLOWAT	_IOR('s',  3, int)		/* get low watermark */
#define	SIOCATMARK	_IOR('s',  7, int)		/* at oob mark? */
#define	SIOCSPGRP	_IOW('s',  8, pid_t)		/* set process group */
#define	SIOCGPGRP	_IOR('s',  9, pid_t)		/* get process group */
#if SEC_ARCH
#define	SIOCGPEERPRIV	_IOR('s', 10, int)		/* get peer privs */
#endif

#define	SIOCADDRT	_IOW('r', 10, struct ortentry)	/* add route */
#define	SIOCDELRT	_IOW('r', 11, struct ortentry)	/* delete route */

#define	SIOCSIFADDR	_IOW('i', 12, struct ifreq)	/* set ifnet address */
#define	SIOCSIFDSTADDR	_IOW('i', 14, struct ifreq)	/* set p-p address */
#define	SIOCSIFFLAGS	_IOW('i', 16, struct ifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	_IOWR('i',17, struct ifreq)	/* get ifnet flags */
#define	SIOCSIFBRDADDR	_IOW('i', 19, struct ifreq)	/* set broadcast addr */
#define	SIOCSIFNETMASK	_IOW('i', 22, struct ifreq)	/* set net addr mask */
#define	SIOCGIFMETRIC	_IOWR('i',23, struct ifreq)	/* get IF metric */
#define	SIOCSIFMETRIC	_IOW('i', 24, struct ifreq)	/* set IF metric */
#define	SIOCDIFADDR	_IOW('i', 25, struct ifreq)	/* delete IF addr */
#define	SIOCAIFADDR	_IOW('i', 26, struct ifaliasreq)/* add/chg IF alias */
#define SIOCRDZCTRS	_IOWR('i', 27, struct ctrreq)	/* Read counter */
#define SIOCRDCTRS	_IOWR('i', 28, struct ctrreq)	/* Read counter */
#define SIOCPIFADDR	_IOW('i', 29, struct ifaliasreq)/* set primary addr */
#define	SIOCSARP	_IOW('i', 30, struct arpreq)	/* set arp entry */
#define	SIOCDARP	_IOW('i', 32, struct arpreq)	/* delete arp entry */
#define SIOCARPREQ      _IOWR('i',40, struct ifreq)     /* arp request pkt */
#define SIOCMANREQ      _IOWR('i',45, struct ifdata)    /* mgmt request */ 
#define SIOCGETEVENTS   _IOWR('i',46, struct ifdata)    /* event notif */
#define SIOCIFRESET     _IOW('i',47, struct ifreq)      /* Reset interface */
#define SIOCEEUPDATE    _IOW('i',48, struct ifeeprom)   /* Write EEPROM */

#define SIOCADDMULTI	_IOW('i', 49, struct ifreq)	/* add m'cast addr */
#define SIOCDELMULTI	_IOW('i', 50, struct ifreq)	/* del m'cast addr */

/* #define SIOCSCREENON _IOWR('i', 51, int)      screend, net/gw_screen.h */
/* #define SIOCSCREEN   _IOWR('i', 52, struct screen_data)     screend */
/* #define SIOCSCREENSTATS _IOR('i', 53, struct screen_stats)  screend */

/* Added by choosing arbitrary numbers */
#define SIOCENABLBACK	_IOW('i', 60, struct ifreq)	/* Enable loopback */
#define SIOCDISABLBACK	_IOW('i', 61, struct ifreq)	/* Disable loopback */
#define SIOCRPHYSADDR	_IOWR('i', 62, struct ifdevea)	/* Read Phys addr */
#define SIOCSPHYSADDR	_IOWR('i', 63, struct ifdevea)	/* Set addr */
#define SIOCIFSETCHAR	_IOWR('i', 64, struct ifchar)	/* Set characteristic */
#define SIOCSMACSPEED	_IOW('i', 65, struct ifreq)     /* Set MAC  speed */
#define SIOCRMACSPEED	_IOWR('i', 66, struct ifreq)   /* Read MAC  speed */
#define SIOCSIPMTU	_IOW('i', 67, struct ifreq)     /* Set intf. IP MTU */
#define SIOCRIPMTU	_IOWR('i', 68, struct ifreq)   /* Read intf. IP MTU */

/* Source Routing ioctl */
#define SIOCSRREQR	_IOWR('i', 70, struct srreq)	/* SR read request */
#define SIOCSRREQW	_IOWR('i', 71, struct srreq)	/* SR write request */

#if	defined(_SOCKADDR_LEN) || defined(_KERNEL)
/* BSD4.4 sockaddr format accepted and returned - also see sys/socket.h */
#define	OSIOCGIFADDR	_IOWR('i',13, struct ifreq)	/* get ifnet address */
#define	SIOCGIFADDR	_IOWR('i',33, struct ifreq)	/* get ifnet address */
#define	OSIOCGIFDSTADDR	_IOWR('i',15, struct ifreq)	/* get p-p address */
#define	SIOCGIFDSTADDR	_IOWR('i',34, struct ifreq)	/* get p-p address */
#define	OSIOCGIFBRDADDR	_IOWR('i',18, struct ifreq)	/* get broadcast addr */
#define	SIOCGIFBRDADDR	_IOWR('i',35, struct ifreq)	/* get broadcast addr */
#define	OSIOCGIFCONF	_IOWR('i',20, struct ifconf)	/* get ifnet list */
#define	SIOCGIFCONF	_IOWR('i',36, struct ifconf)	/* get ifnet list */
#define	OSIOCGIFNETMASK	_IOWR('i',21, struct ifreq)	/* get net addr mask */
#define	SIOCGIFNETMASK	_IOWR('i',37, struct ifreq)	/* get net addr mask */
#define	OSIOCGARP	_IOWR('i',31, struct arpreq)	/* get arp entry */
#define	SIOCGARP	_IOWR('i',38, struct arpreq)	/* get arp entry */
#else
/* BSD4.3 sockaddr format */
#define	SIOCGIFADDR	_IOWR('i',13, struct ifreq)	/* get ifnet address */
#define	SIOCGIFDSTADDR	_IOWR('i',15, struct ifreq)	/* get p-p address */
#define	SIOCGIFBRDADDR	_IOWR('i',18, struct ifreq)	/* get broadcast addr */
#define	SIOCGIFCONF	_IOWR('i',20, struct ifconf)	/* get ifnet list */
#define	SIOCGIFNETMASK	_IOWR('i',21, struct ifreq)	/* get net addr mask */
#define	SIOCGARP	_IOWR('i',31, struct arpreq)	/* get arp entry */
#endif

/* TODO: added the MTIOC stuff from ULTRIX - does it really belong here */
/* Tape i/o controls */
#define MTIOCTOP	_IOW('m', 1, struct mtop) 	/* Do a tape op.*/
#define MTIOCGET	_IOR('m', 2, struct mtget)	/* Get status	*/

/*
 * STREAMS ioctl commands - group 'S'
 */

#define	I_NREAD		_IO('S', 1) /* return the number of bytes in 1st msg */
#define	I_PUSH		_IO('S', 2) /* push module just below stream head */
#define	I_POP		_IO('S', 3) /* pop module below stream head */
#define	I_LOOK		_IO('S', 4) /* retrieve name of first stream module */
#define	I_FLUSH		_IO('S', 5) /* flush all input and/or output queues */
#define	I_SRDOPT	_IO('S', 6) /* set the read mode */
#define	I_GRDOPT	_IO('S', 7) /* get the current read mode */
#define	I_STR		_IO('S', 8) /* create an internal ioctl message */
#define	I_SETSIG	_IO('S', 9) /* request SIGPOLL signal on events */
#define	I_GETSIG	_IO('S',10) /* query the registered events */
#define	I_FIND		_IO('S',11) /* check for module in stream */
#define	I_LINK		_IO('S',12) /* connect stream under mux fd */
#define	I_UNLINK	_IO('S',13) /* disconnect two streams */
#define I_ISASTREAM     _IO('S',14) /* identifies as stream/pipe/fifo */
#define	I_PEEK		_IO('S',15) /* peek at data on read queue */
#define	I_FDINSERT	_IO('S',16) /* create a message and send downstream */
#define	I_SENDFD	_IO('S',17) /* send an fd to a connected pipe stream */
#define	I_RECVFD	_IO('S',18) /* retrieve a file descriptor */
#define I_FLUSHBAND     _IO('S',19) /* flush a particular input and/or output ba
nd */
#define I_SWROPT        _IO('S',20) /* set the write mode */
#define I_GWROPT        _IO('S',21) /* get the current write mode */
#define I_LIST          _IO('S',22) /* get a list of all modules on a stream */
#define I_ATMARK        _IO('S',23) /* is the next message is "marked"? */
#define I_CKBAND        _IO('S',24) /* check for a message of a particular band
*/
#define I_GETBAND       _IO('S',25) /* get the band of the next message */
#define I_CANPUT        _IO('S',26) /* check to see if a message may be passed o
n a stream */
#define I_SETCLTIME     _IO('S',27) /* set the close timeout wait */
#define I_GETCLTIME     _IO('S',28) /* get the current close timeout wait */
#define I_PLINK         _IO('S',29) /* persistently connect a stream under a mux
 */
#define I_PUNLINK       _IO('S',30) /* disconnect a persistent link */
 /* 31-39 available */

#define	I_GETMSG	_IO('S',40) /* getmsg() system call */
#define	I_PUTMSG	_IO('S',41) /* putmsg() system call */
#define I_GETPMSG       _IO('S',42) /* getpmsg() system call */
#define I_PUTPMSG       _IO('S',43) /* putpmsg() system call */
#define I_PIPE          _IO('S',44) /* connect two streams as a pipe */
#define I_FIFO          _IO('S',45) /* convert a stream into a FIFO */
#if SEC_BASE
#define I_STR_ATTR      _IO('S',60)
#define I_PEEK_ATTR     _IO('S',61)
#define I_FDINSERT_ATTR _IO('S',62)
#define I_SENDFD_ATTR   _IO('S',63)
#define I_RECVFD_ATTR   _IO('S',64)
#define I_GETMSG_ATTR   _IO('S',65)
#define I_PUTMSG_ATTR   _IO('S',66)
#endif  /* SEC_BASE */

/* binary event logger - group B */
#define BINLOG_ENABLE           _IO('B', 1)
#define BINLOG_DISABLE          _IO('B', 2)
#define BINLOG_ASCIIENABLE      _IO('B', 3)
#define BINLOG_ASCIIDISABLE     _IO('B', 4)
#define BINLOG_CLRCNTRS         _IO('B', 5)
#define BINLOG_SETPID           _IOW('B', 6, long)
#define BINLOG_GETSTATUS        _IOR('B', 20, struct binlog_getstatus)

/*
 * AIO ioctl commands - group 'A' -- just here to reserve group 'A'.
 * 	See <sys/sysaio.h> for command definitions.
 */

/*
 * audit ioctl commands - group 'C' -- just here to reserve group 'C'.
 * 	See <sys/audit.h> for command definitions.
 */

#endif	/* _SYS_IOCTL_H_ */
