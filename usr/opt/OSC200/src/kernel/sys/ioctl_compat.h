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
 *	@(#)$RCSfile: ioctl_compat.h,v $ $Revision: 4.5.12.3 $ (DEC) $Date: 1993/07/27 18:04:47 $	
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Change log
 *
 */
#ifndef _SYS_IOCTL_COMPAT_H_
#define _SYS_IOCTL_COMPAT_H_

#include <sys/ttychars.h>
#include <sys/ttydev.h>
#include <io/common/devio.h>


struct tchars {
	char	t_intrc;	/* interrupt */
	char	t_quitc;	/* quit */
	char	t_startc;	/* start output */
	char	t_stopc;	/* stop output */
	char	t_eofc;		/* end-of-file */
	char	t_brkc;		/* input delimiter (like nl) */
};

struct ltchars {
	char	t_suspc;	/* stop process signal */
	char	t_dsuspc;	/* delayed stop process signal */
	char	t_rprntc;	/* reprint line */
	char	t_flushc;	/* flush output (toggles) */
	char	t_werasc;	/* word erase */
	char	t_lnextc;	/* literal next character */
};

/*
 * Structure for TIOCGETP and TIOCSETP ioctls.
 */
#ifndef _SGTTYB_
#define	_SGTTYB_
struct sgttyb {
	char	sg_ispeed;		/* input speed */
	char	sg_ospeed;		/* output speed */
	char	sg_erase;		/* erase character */
	char	sg_kill;		/* kill character */
	short	sg_flags;		/* mode flags */
};
#endif

#ifdef _USE_OLD_TTY
# undef  TIOCGETD
# define TIOCGETD	_IOR('t', 0, int)	/* get line discipline */
# undef  TIOCSETD
# define TIOCSETD	_IOW('t', 1, int)	/* set line discipline */
#else
# define OTIOCGETD	_IOR('t', 0, int)	/* get line discipline */
# define OTIOCSETD	_IOW('t', 1, int)	/* set line discipline */
#endif /* _USE_OLD_TTY */

#define	TIOCHPCL	_IO('t', 2)		/* hang up on last close */
#define	TIOCGETP	_IOR('t', 8,struct sgttyb)/* get parameters -- gtty */
#define	TIOCSETP	_IOW('t', 9,struct sgttyb)/* set parameters -- stty */
#define	TIOCSETN	_IOW('t',10,struct sgttyb)/* as above, but no flushtty*/
#define	TIOCSETC	_IOW('t',17,struct tchars)/* set special characters */
#define	TIOCGETC	_IOR('t',18,struct tchars)/* get special characters */

/* Terminal modem control ioctl */
#define TIOCSMLB        _IO('t', 101)           /* Turn on loopback mode*/
#define TIOCCMLB        _IO('t', 100)           /* Turn off loop. mode  */
#define TIOCNMODEM      _IOW('t', 80, int)      /* Ignore modem status  */
#define TIOCMODEM       _IOW('t', 79, int)      /* Look at modem status */

/* new for scsi_disk.c */
/* Generic device information i/o controls */
#define DEVIOCGET	_IOR('v', 1, struct devget)	/* Get dev.info.*/
#define DEVGETGEOM	_IOR('v', 2, DEVGEOMST )	/* Get geometry */
#define SRVC_REQUEST    _IOWR('v', 3, SRVC_REQ) 	/*Service Req SCSI */

/* Disk partition table i/o controls */
#define DIOCGETPT	_IOR('p', 1, struct pt)	/* Get disk paritition	*/
#define DIOCSETPT	_IOW('p', 2, struct pt)	/* Set disk paritition	*/
#define DIOCDGTPT	_IOR('p', 3, struct pt)	/* Get default disk par.*/

/* MSCP disks */
/* Disk i/o controls */
#define DKIOCHDR	_IO('d', 1)			/* Header r/w   */
#define DKIOCDOP	_IOW('d', 2, struct dkop)	/* Do a disk op.*/
#define DKIOCGET	_IOR('d', 3, struct dkget)	/* Get status   */
#define DKIOCACC	_IOWR('d', 4, struct dkacc)	/* Disk access  */
#define DKIOCEXCL	_IOWR('d', 5, int)		/* Exclusive use*/


/* tty.h */
#define MODEM_CD   0x01
#define MODEM_DSR  0x02
#define MODEM_CTS  0x04
#define MODEM_DSR_START  0x08
#define MODEM_BADCALL 0x10

/* massive */
#define O_BLKINUSE	2

#define	DELAY_FLAG	15
#define	TS_ISUSP	16

#define		TANDEM		0x00000001	/* send stopc on out q full */
#define		CBREAK		0x00000002	/* half-cooked mode */
#define		LCASE		0x00000004	/* simulate lower case */
#define		ECHO		0x00000008	/* echo input */
#define		CRMOD		0x00000010	/* map \r to \r\n on output */
#define		RAW		0x00000020	/* no i/o processing */
#define		ODDP		0x00000040	/* get/send odd parity */
#define		EVENP		0x00000080	/* get/send even parity */
#define		ANYP		0x000000c0	/* get any parity/send none */
#define		NLDELAY		0x00000300	/* \n delay */
#define		TBDELAY		0x00000c00	/* horizontal tab delay */
#define		XTABS		0x00000c00	/* expand tabs on output */
#define		CRDELAY		0x00003000	/* \r delay */
#define		VTDELAY		0x00004000	/* vertical tab delay */
#define		BSDELAY		0x00008000	/* \b delay */
#define		ALLDELAY	(NLDELAY|TBDELAY|CRDELAY|VTDELAY|BSDELAY)

#ifndef NL0		/* Compatability , these are in termios.h */
#define			NL0	0x00000000
#define			NL1	0x00000100	/* tty 37 */
#define			NL2	0x00000200	/* vt05 */
#define			NL3	0x00000300
#define			TAB0	0x00000000
#define			TAB1	0x00000400	/* tty 37 */
#define			TAB2	0x00000800
#define			TAB3	0x00000C00	/* expand tabs on output */
#define			CR0	0x00000000
#define			CR1	0x00001000	/* tn 300 */
#define			CR2	0x00002000	/* tty 37 */
#define			CR3	0x00003000	/* concept 100 */
#define			FF0	0x00000000
#define			FF1	0x00004000
#define			BS0	0x00000000
#define			BS1	0x00008000
#define			VT0	0x00000000
#define			VT1	0x00010000	/* tty 37 */
#endif	/* NL0 */

#define		CRTBS		0x00010000	/* do backspacing for crt */
#define		PRTERA		0x00020000	/* \ ... / erase */
#define		CRTERA		0x00040000	/* " \b " to wipe out char */
#define		TILDE		0x00080000	/* hazeltine tilde kludge */
#define		MDMBUF		0x00100000	/*start/stop output on carrier*/
#define		LITOUT		0x00200000	/* literal output */
#define		TOSTOP		0x00400000	/*SIGSTOP on background output*/
#define		FLUSHO		0x00800000	/* flush output to terminal */
#define		NOHANG		0x01000000	/* no SIGHUP on carrier drop */
#define		L001000		0x02000000
#define		CRTKIL		0x04000000	/* kill line with " \b " */
#define		PASS8		0x08000000
#define		CTLECH		0x10000000	/* echo control chars as ^X */
#define		PENDIN		0x20000000	/* tp->t_rawq needs reread */
#define		DECCTQ		0x40000000	/* only ^Q starts after ^S */
#define		NOFLSH		0x80000000	/* no output flush on signal */
#define	TIOCLBIS	_IOW('t', 127, int)	/* bis local mode bits */
#define	TIOCLBIC	_IOW('t', 126, int)	/* bic local mode bits */
#define	TIOCLSET	_IOW('t', 125, int)	/* set entire local mode word */
#define	TIOCLGET	_IOR('t', 124, int)	/* get local modes */
#define		LCRTBS		(CRTBS>>16)
#define		LPRTERA		(PRTERA>>16)
#define		LCRTERA		(CRTERA>>16)
#define		LTILDE		(TILDE>>16)
#define		LMDMBUF		(MDMBUF>>16)
#define		LLITOUT		(LITOUT>>16)
#define		LTOSTOP		(TOSTOP>>16)
#define		LFLUSHO		(FLUSHO>>16)
#define		LNOHANG		(NOHANG>>16)
#define		LCRTKIL		(CRTKIL>>16)
#define		LPASS8		(PASS8>>16)
#define		LCTLECH		(CTLECH>>16)
#define		LPENDIN		(PENDIN>>16)
#define		LDECCTQ		(DECCTQ>>16)
#define		LNOFLSH		(NOFLSH>>16)
#define	TIOCSLTC	_IOW('t',117,struct ltchars)/* set local special chars*/
#define	TIOCGLTC	_IOR('t',116,struct ltchars)/* get local special chars*/

/* ULTRIX compatible line discipline */
#define OTTYDISC        0x00       	/* Old, v7 std tty driver */
#define NETLDISC        0x01       	/* Line discipline for berk net */
#define NTTYDISC        0x02       	/* New tty discipline */

#if	!defined(TABLDISC) || (TABLDISC != 3)
#define TABLDISC        0x03       	/* Hitachi tablet discipline */
#endif	/* TABLDISC */

#define NTABLDISC       0x04       	/* Gtco tablet discipline */
#define HCLDISC         0x05       	/* Half cooked discipline */
#define TERMIODISC      0x06       	/* termio line discipline */ 
#define SLPDISC         0x07       	/* BSD Serial Line IP 	*/
#define PCMDISC         0x08       	/* Peripheral Control Module for 
					   dial and button boxex */
                                        /* Line disc #'s 16-23 are 
					   reserved for local extension.*/

#endif
