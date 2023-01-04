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
static char *rcsid = "@(#)$RCSfile: ult_cvt.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:06:22 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)ult_cvt.c	3.1	(ULTRIX/OSF)	2/26/91";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * ult_cvt.c
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
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

static int compatspcodes[16] = { 
	0, 50, 75, 110, 134, 150, 200, 300, 600, 1200,
	1800, 2400, 4800, 9600, 19200, 38400,
};



/*
 * compatability with Ultrix 4.2
 *	The defines and struct are from h/termio.h and h/termios.h
 */
#define	U_NCCS	19
struct u_termios {
        tcflag_t        c_iflag;                /* Input Modes          */
        tcflag_t        c_oflag;                /* Output Modes         */
        tcflag_t        c_cflag;                /* Control Modes        */
        tcflag_t        c_lflag;                /* Local Modes          */
        cc_t            c_cc[U_NCCS];           /* Control Characters   */
        cc_t            c_line;                 /* line disc. -local ext*/
};

#define	U_NCC	10
struct u_termio {
	unsigned short	c_iflag;	/* input modes	*/
	unsigned short	c_oflag;	/* output modes	*/
	unsigned short	c_cflag;	/* control modes	*/
	unsigned short	c_lflag;	/* line discipline modes	*/
	char	c_line;			/* line discipline	*/
	unsigned char	c_cc[U_NCC];	/* control chars	*/
};


static unsigned int cvtiOtoU();
static unsigned int cvtoOtoU();
static unsigned int cvtcOtoU();
static unsigned int cvtlOtoU();
static unsigned int cvtiUtoO();
static unsigned int cvtoUtoO();
static unsigned int cvtcUtoO();
static unsigned int cvtlUtoO();

/* Ultrix 4.2 termios control character indicies */
#define U_VINTR   0
#define U_VQUIT   1
#define U_VERASE  2
#define U_VKILL   3
#define U_VEOF    4
#define U_VEOL    5
#define U_VEOL2   6
#define U_VSWTCH  7
#define U_VMIN    8
#define U_VTIME   9
#define U_VSTART  10
#define U_VSTOP   11
#define U_VSUSP   12
#define U_VDSUSP  13
#define U_VRPRNT  14
#define U_VFLUSH  15
#define U_VWERASE 16
#define U_VLNEXT  17
#define U_VQUOTE  18

/* input modes */
#define U_IGNBRK  0000001
#define U_BRKINT  0000002
#define U_IGNPAR  0000004
#define U_PARMRK  0000010
#define U_INPCK   0000020
#define U_ISTRIP  0000040
#define U_INLCR   0000100
#define U_IGNCR   0000200
#define U_ICRNL   0000400
#define U_IUCLC   0001000
#define U_IXON    0002000
#define U_IXANY   0004000
#define U_IXOFF   0010000
#define U_TPENDIN 0x10000      /* Retype pending input at next read or input */
#define U_TCBREAK 0x20000      /* Limited canonical processing */

/* output modes */
#define U_OPOST   0000001
#define U_OLCUC   0000002
#define U_ONLCR   0000004
#define U_OCRNL   0000010
#define U_ONOCR   0000020
#define U_ONLRET  0000040
#define U_OFILL   0000100
#define U_OFDEL   0000200
#define U_NLDLY   0000400
#define U_CRDLY   0030000
#define U_CR1     0010000
#define U_CR2     0020000
#define U_TABDLY  0006000
#define U_TAB1    0002000
#define U_TAB2    0004000
#define U_BSDLY   0100000
#define U_VTDLY   0001000
#define U_FFDLY   0040000
#define U_PTILDE 0x10000          /* Convert ~ to ` on output */
    /* These are for local translation only.  Not intended for general use! */
#define U_PFLUSHO 0x20000        /* Actually a state flag.Output being flushed */
#define U_PLITOUT 0x40000         /* Supress output translations */
    /* Note: if PNL2  definition changes, also change the cases in tty.c */
#define U_PNL2    0x80000         /* Newline delay type 2 */

/* control modes */
/* input speed is encrypted in 0xf0000 output speed is in 0xf */
#define U_CSIZE   0000060
#define U_CS6     0000020
#define U_CS7     0000040
#define U_CS8     0000060
#define U_CSTOPB  0000100
#define U_CREAD   0000200
#define U_PARENB  0000400
#define U_PARODD  0001000
#define U_HUPCL   0002000
#define U_CLOCAL  0004000
#define U_LOBLK   0010000
#define U_CBAUD   0000017
#define U_B50     0000001
#define U_PAUTOFLOW 0x100000      /* Hardware controled flow control */

/* "local" flags */
/* line discipline 0 modes */
#define U_ISIG    0000001
#define U_ICANON  0000002
#define U_XCASE   0000004
#define U_ECHO    0000010
#define U_ECHOE   0000020
#define U_ECHOK   0000040
#define U_ECHONL  0000100
#define U_NOFLSH  0000200
#define U_PIEXTEN 0x800000     /* Enable local special characters */
#define U_PCTLECH 0x10000      /* Echo input control chars as ^X */
#define U_PPRTERA 0x20000      /* Hardcopy terminal erase mode using \c */
#define U_PCRTBS  0x40000      /* Backspace on erase */
#define U_PCRTERA 0x80000      /* Printing terminal erase mode */
#define U_PCRTKIL 0x100000     /* BS-space-BS erase entire line on kill */
#define U_PRAW    0x200000     /* Berkeley non-canonical I/O */
#define U_PTOSTOP 0x400000     /* Send SIGTTOU for bg output - must be 0x40 !*/


/* 
 * These conversion routines are called from ioctl when servicing a 
 * 't' ioctl from an Ultrix 4.2 executable. The functions convert
 * termio and termios structs between the definitions for the 
 * two systems. The conversion involves moving the bits to the 
 * proper location and re-ordering the control char vector.
 *
 * The flag conversions are done based on ints for both termio and 
 * termios since the bits are the same for the termio defined bits
 */

/*
 * cvtOtoU	- OSF/1 termio to Ultrix 4.2 termio
 * 	This is called before copyout.
 */
cvtOtoU(osf)
struct termio	*osf;
{
struct u_termio ult;	/* convert into here */

	bzero(&ult, sizeof (ult));

	/* Convert the control character vector */
	ult.c_cc[U_VINTR]	= osf->c_cc[VVINTR];
	ult.c_cc[U_VQUIT]	= osf->c_cc[VVQUIT];
	ult.c_cc[U_VERASE]	= osf->c_cc[VVERASE];
	ult.c_cc[U_VKILL]	= osf->c_cc[VVKILL];
	ult.c_cc[U_VEOL2]	= osf->c_cc[VVEOL2];
	ult.c_cc[U_VSWTCH]	= osf->c_cc[VVSWTCH];
	
	if(osf->c_lflag & ICANON) {
	    ult.c_cc[U_VEOF]	= osf->c_cc[VVEOF];
	    ult.c_cc[U_VEOL]	= osf->c_cc[VVEOL];
	    ult.c_cc[U_VMIN]	= 1;	/* wait for 1 char */
 	    ult.c_cc[U_VTIME]	= 0;    /* wait a long time */
	} else {
	    ult.c_cc[U_VEOF]	= 4;	/* ^d */
	    ult.c_cc[U_VEOL]	= 0xff; /* not defined */
	    ult.c_cc[U_VMIN]	= osf->c_cc[VVMIN];
 	    ult.c_cc[U_VTIME]	= osf->c_cc[VVTIME];
	}

	/* Convert the input mode bits */
	ult.c_iflag = cvtiOtoU((int)osf->c_iflag);

	/* Convert the output mode bits */
	ult.c_oflag = cvtoOtoU((int)osf->c_oflag);

	/* Convert the control bits */
	ult.c_cflag = cvtcOtoU((int)osf->c_cflag);

	/* Convert the "local" flags */
	ult.c_lflag = cvtlOtoU((int)osf->c_lflag, 0 /*termio*/);

	/* get the speed */
	ult.c_cflag |= (osf->c_cflag & 0x0f);

	/* set the line disc. */
	ult.c_line = osf->c_line;
	cvtLineOtoU(&ult.c_line);

	bcopy(&ult, osf, sizeof(ult));
}


/*
 * cvtUtoO
 *	Converts a supplied Ultrix 4.2 termio struct into an OSF/1
 *	termio struct. 
 *
 * NOTE: This assumes that the function is passed a buffer that is 
 *	long enough to hold the converted struct. Since this is expected
 *	to be called by ioctl() with a pointer to the 128 byte data
 *	buffer, this will be O.K.
 */
cvtUtoO(ult)
struct u_termio *ult;
{
struct termio	osf;
int	error = 0;

	/* Convert the control character vector */
	osf.c_cc[VVINTR]	= ult->c_cc[U_VINTR];
	osf.c_cc[VVQUIT]	= ult->c_cc[U_VQUIT];
	osf.c_cc[VVERASE]	= ult->c_cc[U_VERASE];
	osf.c_cc[VVKILL]	= ult->c_cc[U_VKILL];
	osf.c_cc[VVEOL2]	= ult->c_cc[U_VEOL2];
	osf.c_cc[VVSWTCH]	= ult->c_cc[U_VSWTCH];

	if(ult->c_lflag & U_ICANON) {
	    osf.c_cc[VVEOF]	= ult->c_cc[U_VEOF];
	    osf.c_cc[VVEOL]	= ult->c_cc[U_VEOL];
	} else {
	    osf.c_cc[VVEOF]	= ult->c_cc[U_VMIN];
 	    osf.c_cc[VVEOL]	= ult->c_cc[U_VTIME];
	}


	/* Convert the input mode bits */
	osf.c_iflag = cvtiUtoO((int)ult->c_iflag, &error);

	/* Convert the output mode bits */
	osf.c_oflag = cvtoUtoO((int)ult->c_oflag, &error);

	/* Convert the control bits */
	osf.c_cflag = cvtcUtoO((int)ult->c_cflag, &error);

	/* "local" flags */
	/* line discipline 0 modes */
	osf.c_lflag = cvtlUtoO((int)ult->c_lflag, 0 /*termio*/, &error);

	/* set speed */
	osf.c_cflag |= (ult->c_cflag & U_CBAUD);

	/* c_line is *ALWAYS* TERMIODISC in Ultrix */
	osf.c_line = ult->c_line;
	cvtLineUtoO(&osf.c_line);

	bcopy(&osf, ult, sizeof(osf));
	return(0);
}



/*
 * cvtsOtoU	OSF/1 termios to Ultrix 4.2 termios conversion
 */
cvtsOtoU(osf)
struct termios	*osf;
{
struct u_termios ult;	/* work space */
int		 i;

	bzero(&ult, sizeof (struct u_termios));

	/* Convert the control character vector */
	ult.c_cc[U_VINTR]	= osf->c_cc[VINTR];
	ult.c_cc[U_VQUIT]	= osf->c_cc[VQUIT];
	ult.c_cc[U_VERASE]	= osf->c_cc[VERASE];
	ult.c_cc[U_VKILL]	= osf->c_cc[VKILL];
	ult.c_cc[U_VEOF]	= osf->c_cc[VEOF];
	ult.c_cc[U_VEOL]	= osf->c_cc[VEOL];
	ult.c_cc[U_VEOL2]	= osf->c_cc[VEOL2];
	ult.c_cc[U_VSWTCH]	= osf->c_cc[7 /* spare 1 */];
	ult.c_cc[U_VMIN]	= osf->c_cc[VMIN];
	ult.c_cc[U_VTIME]	= osf->c_cc[VTIME];
	ult.c_cc[U_VSTART]	= osf->c_cc[VSTART];
	ult.c_cc[U_VSTOP]	= osf->c_cc[VSTOP];
	ult.c_cc[U_VSUSP]	= osf->c_cc[VSUSP];
	ult.c_cc[U_VDSUSP]	= osf->c_cc[VDSUSP];
	ult.c_cc[U_VRPRNT]	= osf->c_cc[VREPRINT];
	ult.c_cc[U_VFLUSH]	= osf->c_cc[VDISCARD];
	ult.c_cc[U_VWERASE]	= osf->c_cc[VWERASE];
	ult.c_cc[U_VLNEXT]	= osf->c_cc[VLNEXT];
	ult.c_cc[U_VQUOTE]	= osf->c_cc[19 /* spare 2 */];
	
	/* Convert the input mode bits */
	ult.c_iflag = cvtiOtoU(osf->c_iflag);

	/* Convert the output mode bits */
	ult.c_oflag = cvtoOtoU(osf->c_oflag);

	/* Convert the control bits */
	ult.c_cflag = cvtcOtoU(osf->c_cflag);

	/* Convert the "local" flags */
	/* line discipline 0 modes */
        ult.c_lflag = cvtlOtoU(osf->c_lflag, 1 /*termios*/);

	/* get the input speed */
	for(i = 0; i < 16; i++){
		if (osf->c_ispeed == compatspcodes[i])
			break;
	}
	/* When the speed is not found, B50 is selected. When
	 * setting the speed on osf, B50 results in the speed not
	 * being changed */
	if (i == 16)
		i = U_B50;	/* ignore speed setting */
	ult.c_cflag |= (i << 16);

	/* get the output speed */
	for(i = 0; i < 16; i++){
		if (osf->c_ospeed == compatspcodes[i])
			break;
	}
	if (i == 16)
		i = U_B50;	/* ignore speed setting */
	ult.c_cflag |= i;

	/* set the line disc. */
	ult.c_line = 0;

	bcopy(&ult, osf, sizeof(ult));
}


/*
 * cvtsUtoO
 *	Converts a supplied Ultrix 4.2 termios struct into an OSF/1
 *	termios struct. Look at the existing struct when setting speed.
 *	This is needed because the OSF/1 struct may have an unsupported 
 *	speed.
 *
 * Un-mapped bits:
 *	When an Ultrix bit is set that dowsn't have an OSF/1 equivalent,
 *	the conversion function returns EINVAL.
 *
 * NOTE: This assumes that the function is passed a buffer that is 
 *	long enough to hold the converted struct. Since this is expected
 *	to be called by ioctl() with a pointer to the 128 byte data
 *	buffer, this will be O.K.
 */
cvtsUtoO(ult, osf_pt)
struct u_termios *ult;
struct termios *osf_pt;
{
int		error = 0;
struct termios	osf;

	/* clear work space */
	bzero(&osf, sizeof(osf));

	/* Convert the control character vector */
	/* The 2 spare locations are used to hold values that
	 * are in Ultrix 4.2 but not in OSF/1 */
	osf.c_cc[VEOF]		= ult->c_cc[U_VEOF];
	osf.c_cc[VEOL]		= ult->c_cc[U_VEOL];
	osf.c_cc[VEOL2]		= ult->c_cc[U_VEOL2];
	osf.c_cc[VERASE]	= ult->c_cc[U_VERASE];
	osf.c_cc[VWERASE]	= ult->c_cc[U_VWERASE];
	osf.c_cc[VKILL]		= ult->c_cc[U_VKILL];
	osf.c_cc[VREPRINT]	= ult->c_cc[U_VRPRNT];
	osf.c_cc[ 7 /*spare1*/]	= ult->c_cc[U_VSWTCH];
	osf.c_cc[VINTR]		= ult->c_cc[U_VINTR];
	osf.c_cc[VQUIT]		= ult->c_cc[U_VQUIT];
	osf.c_cc[VSUSP]		= ult->c_cc[U_VSUSP];
	osf.c_cc[VDSUSP]	= ult->c_cc[U_VDSUSP];
	osf.c_cc[VSTART]	= ult->c_cc[U_VSTART];
	osf.c_cc[VSTOP]		= ult->c_cc[U_VSTOP];
	osf.c_cc[VLNEXT]	= ult->c_cc[U_VLNEXT];
	osf.c_cc[VDISCARD]	= ult->c_cc[U_VFLUSH];
	osf.c_cc[VMIN]		= ult->c_cc[U_VMIN];
	osf.c_cc[VTIME]		= ult->c_cc[U_VTIME];
/*	osf.c_cc[VSTATUS]	= ult->c_cc[????];		*/
	osf.c_cc[19 /*spare2*/]	= ult->c_cc[U_VQUOTE];

	/* Convert the input mode bits */
	osf.c_iflag = cvtiUtoO(ult->c_iflag, &error);

	/* Convert the output mode bits */
	osf.c_oflag = cvtoUtoO(ult->c_oflag, &error);

	/* Convert the control bits */
	osf.c_cflag = cvtcUtoO(ult->c_cflag, &error);

	/* "local" flags */
	/* line discipline 0 modes */
	osf.c_lflag = cvtlUtoO(ult->c_lflag, 1 /*termios*/, &error);

	/* When the speed is U_B50, don't change the speed */
	if ((ult->c_cflag & U_CBAUD) != U_B50)
		osf.c_ospeed = compatspcodes[ult->c_cflag & U_CBAUD];
	else
		osf.c_ospeed = osf_pt->c_ospeed;

	/* set input speed */
	if (((ult->c_cflag >> 16) & U_CBAUD) != U_B50)
		osf.c_ispeed = compatspcodes[((ult->c_cflag >> 16) & U_CBAUD)];
	else
		osf.c_ispeed = osf_pt->c_ispeed;

	/* line discipline */

	bcopy(&osf, ult, sizeof(osf));
	return(0);
}



/*
 * NOTE:
 *	The position of the flag bits in each of the control fields is
 *	different in OSF/1 and Ultrix 4.2. These functions "move"
 *	control flags within the flag field.
 *
 *	Since the termio bits are in the same position as the termios
 *	bits of the same name on each system, there is only one conversion
 *	function for each flag that is used by both termio and termios.
 *
 * NOTE:
 *	There are several errors in the OSF/1 flag bit definitions that
 *	cause bits not to be representable in the u_shorts that the 
 *	termio struct specifies. These problems are resolved in the 
 *	calling functions. These functions use 32-bit objects.
 */


/*
 * cvtiOtoU
 *	Convert the c_iflag field from OSF/1 to Ultrix 4.2 bit positions
 */
static unsigned int
cvtiOtoU(iflag)
unsigned int	iflag;
{
unsigned int	uu = 0;

	uu |= (iflag & IGNBRK) ? U_IGNBRK  : 0;
	uu |= (iflag & BRKINT) ? U_BRKINT  : 0;
	uu |= (iflag & IGNPAR) ? U_IGNPAR  : 0;
	uu |= (iflag & PARMRK) ? U_PARMRK  : 0;
	uu |= (iflag & INPCK)  ? U_INPCK   : 0;
	uu |= (iflag & ISTRIP) ? U_ISTRIP  : 0;
	uu |= (iflag & INLCR)  ? U_INLCR   : 0;
	uu |= (iflag & IGNCR)  ? U_IGNCR   : 0;
	uu |= (iflag & ICRNL)  ? U_ICRNL   : 0;
	uu |= (iflag & IUCLC)  ? U_IUCLC   : 0;
	uu |= (iflag & IXON)   ? U_IXON    : 0;
	uu |= (iflag & IXANY)  ? U_IXANY   : 0;
	uu |= (iflag & IXOFF)  ? U_IXOFF   : 0;
/* Bit mappings for these bits are not defined */
/*	uu |= (iflag & IMAXBEL)  ? ????? : 0;	*/
/*	uu |= (iflag & ?????)  ? U_TPENDIN : 0;	*/
/*	uu |= (iflag & ?????)  ? U_TCBREAK : 0;	*/

	return (uu);
}


/*
 * cvtoOtoU
 *	Convert the c_oflag field from OSF/1 to Ultrix 4.2 bit positions
 */
static unsigned int
cvtoOtoU(oflag)
unsigned int	oflag;
{
unsigned int	uu = 0;
	
	uu |= (oflag & OPOST)  ? U_OPOST   : 0;
	uu |= (oflag & OLCUC)  ? U_OLCUC   : 0;
	uu |= (oflag & ONLCR)  ? U_ONLCR   : 0;
	uu |= (oflag & OCRNL)  ? U_OCRNL   : 0;
	uu |= (oflag & ONOCR)  ? U_ONOCR   : 0;
	uu |= (oflag & ONLRET) ? U_ONLRET  : 0;
	uu |= (oflag & OFILL)  ? U_OFILL   : 0;
	uu |= (oflag & OFDEL)  ? U_OFDEL   : 0;
	uu |= (oflag & NLDLY)  ? U_NLDLY   : 0;
	uu |= (oflag & CR1)    ? U_CR1     : 0;
	uu |= (oflag & CR2)    ? U_CR2     : 0;
	uu |= (oflag & TAB1)   ? U_TAB1    : 0;
	uu |= (oflag & TAB2)   ? U_TAB2    : 0;
	uu |= (oflag & BSDLY)  ? U_BSDLY   : 0;
	uu |= (oflag & VTDLY)  ? U_VTDLY   : 0; /* VTDLY is 0x10000 !!!*/
	uu |= (oflag & FFDLY)  ? U_FFDLY   : 0;
/* Bit mappings for these bits are not defined */
/*	uu |= (oflag & ????)   ? U_PTILDE  : 0;	*/
/*	uu |= (oflag & ????)   ? U_PFLUSHO : 0;	*/
/*	uu |= (oflag & ????)   ? U_PLITOUT : 0;	*/
/*	uu |= (oflag & ????)   ? U_PNL2    : 0;	*/
/*	uu |= (oflag & OXTABS) ? ????      : 0;	*/
/*	uu |= (oflag & ONOEOT) ? ????      : 0;	*/

	return (uu);
}



/*
 * cvtcOtoU
 *	Convert the c_cflag field from OSF/1 to Ultrix 4.2 bit positions
 */
static unsigned int
cvtcOtoU(cflag)
unsigned int cflag;
{
unsigned int	uu = 0;

	uu |= (cflag & CS6)     ? U_CS6      : 0;
	uu |= (cflag & CS7)     ? U_CS7      : 0;
	uu |= (cflag & CSTOPB)  ? U_CSTOPB   : 0;
	uu |= (cflag & CREAD)   ? U_CREAD    : 0;
	uu |= (cflag & PARENB)  ? U_PARENB   : 0;
	uu |= (cflag & PARODD)  ? U_PARODD   : 0;
	uu |= (cflag & HUPCL)   ? U_HUPCL    : 0;
	uu |= (cflag & CLOCAL)  ? U_CLOCAL   : 0;
	uu |= (cflag & CRTSCTS) ? U_PAUTOFLOW : 0;
	uu |= (cflag & 0x80 )    ? U_LOBLK    : 0;

	return (uu);
}



/*
 * cvtlOtoU
 *	Convert the c_lflag field from OSF/1 to Ultrix 4.2 bit positions
 */
static unsigned int
cvtlOtoU(lflag, sflag)
unsigned int lflag;
int sflag;	/* 1==termios, 0==termio */
{
unsigned int	uu = 0;

        uu |= (lflag & ISIG)    ? U_ISIG    : 0;
        uu |= (lflag & ICANON)  ? U_ICANON  : 0;
        uu |= (lflag & XCASE)   ? U_XCASE   : 0;
        uu |= (lflag & ECHO)    ? U_ECHO    : 0;
        uu |= (lflag & ECHOE)   ? U_ECHOE   : 0;
        uu |= (lflag & ECHOK)   ? U_ECHOK   : 0;
        uu |= (lflag & ECHONL)  ? U_ECHONL  : 0;
	if(sflag)
        	uu |= (lflag & NOFLSH)  ? U_NOFLSH  : 0;
	else
        	uu |= (lflag & VNOFLSH)  ? U_NOFLSH  : 0;
        uu |= (lflag & IEXTEN)  ? U_PIEXTEN : 0;
        uu |= (lflag & TOSTOP)  ? U_PTOSTOP : 0;
        uu |= (lflag & ECHOCTL) ? U_PCTLECH : 0;
/* Bit mappings for these bits are not defined */
/*      uu |= (lflag & ECHOKE)     ? ????   : 0;	*/
/*      uu |= (lflag & ECHOPRT)    ? ????   : 0;	*/
/*      uu |= (lflag & ALTWERASE)  ? ????   : 0;	*/
/*      uu |= (lflag & MDMBUF)     ? ????   : 0;	*/
/*      uu |= (lflag & FLUSHO)     ? ????   : 0;	*/
/*      uu |= (lflag & NOHANG)     ? ????   : 0;	*/
/*      uu |= (lflag & PENDIN)     ? ????   : 0;	*/
/*      uu |= (lflag & NOKERNINFO) ? ????   : 0;	*/
/*      uu |= (lflag & ????)       ? U_PRAW    : 0;	*/
/*      uu |= (lflag & ????)       ? U_PPRTERA : 0;	*/
/*      uu |= (lflag & ????)       ? U_PCRTBS  : 0;	*/
/*      uu |= (lflag & ????)       ? U_PCRTERA : 0;	*/
/*      uu |= (lflag & ????)       ? U_PCRTKIL : 0;	*/

	return (uu);
}



/*
 * cvtiUtoO
 *	Convert the c_iflag field from Ultrix 4.2 to OSF/1 bit positions
 */
static unsigned int
cvtiUtoO(iflag, error)
unsigned int	iflag;
int	*error;		/* Ultrix 4.2 flag bit that is not defined for OSF/1*/
{
unsigned int	uu = 0;

	uu |= (iflag & U_IGNBRK) ? IGNBRK : 0;
	uu |= (iflag & U_BRKINT) ? BRKINT : 0;
	uu |= (iflag & U_IGNPAR) ? IGNPAR : 0;
	uu |= (iflag & U_PARMRK) ? PARMRK : 0;
	uu |= (iflag & U_INPCK)  ? INPCK  : 0;
	uu |= (iflag & U_ISTRIP) ? ISTRIP : 0;
	uu |= (iflag & U_INLCR)  ? INLCR  : 0;
	uu |= (iflag & U_IGNCR)  ? IGNCR  : 0;
	uu |= (iflag & U_ICRNL)  ? ICRNL  : 0;
	uu |= (iflag & U_IXON)   ? IXON   : 0;
	uu |= (iflag & U_IXOFF)  ? IXOFF  : 0;
	uu |= (iflag & U_IXANY)  ? IXANY  : 0;
	uu |= (iflag & U_IUCLC)  ? IUCLC  : 0;
/* Bit mappings for these bits are not defined */
/*	uu |= (iflag & U_TPENDIN  ? ????? : 0;	*/
/*	uu |= (iflag & U_TCBREAK  ? ????? : 0;	*/
	if(iflag &= ~(U_IGNBRK | U_BRKINT | U_IGNPAR | U_PARMRK | U_INPCK | 
		U_ISTRIP | U_INLCR | U_IGNCR | U_ICRNL | U_IXON | U_IXOFF | 
		U_IXANY | U_IUCLC))
		*error = EINVAL;

	return(uu);
}



/*
 * cvtoUtoO
 *	Convert the c_oflag field from Ultrix 4.2 to OSF/1 bit positions
 */
static unsigned int
cvtoUtoO(oflag, error)
unsigned int	oflag;
int	*error;		/* Ultrix 4.2 flag bit that is not defined for OSF/1*/
{
unsigned int	uu = 0;

	uu |= (oflag & U_OPOST)  ? OPOST  : 0;
	uu |= (oflag & U_OLCUC)  ? OLCUC  : 0;
	uu |= (oflag & U_ONLCR)  ? ONLCR  : 0;
	uu |= (oflag & U_OCRNL)  ? OCRNL  : 0;
	uu |= (oflag & U_ONOCR)  ? ONOCR  : 0;
	uu |= (oflag & U_ONLRET) ? ONLRET : 0;
	uu |= (oflag & U_OFILL)  ? OFILL  : 0;
	uu |= (oflag & U_OFDEL)  ? OFDEL  : 0;
	uu |= (oflag & U_NLDLY)  ? NLDLY  : 0;
	uu |= (oflag & U_CR1)    ? CR1    : 0;
	uu |= (oflag & U_CR2)    ? CR2    : 0;
	uu |= (oflag & U_TAB1)   ? TAB1   : 0;
	uu |= (oflag & U_TAB2)   ? TAB2   : 0;
        uu |= (oflag & U_BSDLY)  ? BSDLY  : 0;
	uu |= (oflag & U_VTDLY)  ? VTDLY  : 0; /* VTDLY is 0x10000 !!!*/
	uu |= (oflag & U_FFDLY)  ? FFDLY  : 0;
/* Bit mappings for these bits are not defined */
/*	uu |= (oflag & U_PTILDE  ? PTILDE : 0;	*/
/*	uu |= (oflag & U_PFLUSHO ? ????   : 0;	*/
/*	uu |= (oflag & U_PLITOUT ? ????   : 0;	*/
/*	uu |= (oflag & U_PNL2    ? ????   : 0;	*/
/*	uu |= (oflag & ????)     ? OXTABS : 0;	*/
/*	uu |= (oflag & ????)     ? ONOEOT : 0;	*/
	if(oflag &= ~(U_OPOST | U_OLCUC | U_ONLCR | U_OCRNL | U_ONOCR | 
		U_ONLRET | U_OFILL | U_OFDEL | U_NLDLY | U_CR1 | U_CR2 | 
		U_TAB1 | U_TAB2 | U_BSDLY | U_VTDLY | U_FFDLY))
		*error = EINVAL;

	return(uu);
}



/*
 * cvtcUtoO
 *	Convert the c_cflag field from Ultrix 4.2 to OSF/1 bit positions
 */
static unsigned int
cvtcUtoO(cflag, error)
unsigned int	cflag;
int	*error;		/* Ultrix 4.2 flag bit that is not defined for OSF/1*/
{
unsigned int	uu = 0;

	uu |= (cflag & U_CS6)     ? CS6      : 0;
	uu |= (cflag & U_CS7)     ? CS7      : 0;
	uu |= (cflag & U_CSTOPB)  ? CSTOPB   : 0;
	uu |= (cflag & U_CREAD)   ? CREAD    : 0;
	uu |= (cflag & U_PARENB)  ? PARENB   : 0;
	uu |= (cflag & U_PARODD)  ? PARODD   : 0;
	uu |= (cflag & U_HUPCL)   ? HUPCL    : 0;
	uu |= (cflag & U_CLOCAL)  ? CLOCAL   : 0;
	uu |= (cflag & U_PAUTOFLOW) ? CRTSCTS  : 0;
	uu |= (cflag & U_LOBLK)   ? 0x80     : 0;
	if(cflag &= ~(U_CS6 | U_CS7 | U_CSTOPB | U_CREAD | U_PARENB | U_PARODD |
		U_HUPCL | U_CLOCAL | U_PAUTOFLOW | 0x80 | 0xf000f))
		*error = EINVAL;

	return(uu);
}



/*
 * cvtlUtoO
 *	Convert the c_lflag field from Ultrix 4.2 to OSF/1 bit positions
 */
static unsigned int
cvtlUtoO(lflag, sflag, error)
unsigned int	lflag;
int	sflag;	/* 1==termios, 0==termio */
int	*error;	/* Ultrix 4.2 flag bit that is not defined for OSF/1*/
{
unsigned int	uu = 0;

        uu |= (lflag & U_ISIG)    ? ISIG       : 0;
        uu |= (lflag & U_ICANON)  ? ICANON     : 0;
        uu |= (lflag & U_XCASE)   ? XCASE      : 0;
        uu |= (lflag & U_ECHO)    ? ECHO       : 0;
        uu |= (lflag & U_ECHOE)   ? ECHOE      : 0;
        uu |= (lflag & U_ECHOK)   ? ECHOK      : 0;
        uu |= (lflag & U_ECHONL)  ? ECHONL     : 0;
	if(sflag)
        	uu |= (lflag & U_NOFLSH)  ? NOFLSH     : 0;
	else
        	uu |= (lflag & U_NOFLSH)  ? VNOFLSH     : 0;
        uu |= (lflag & U_PIEXTEN) ? IEXTEN     : 0;
        uu |= (lflag & U_PTOSTOP) ? TOSTOP     : 0;
        uu |= (lflag & U_PCTLECH) ? ECHOCTL    : 0;
/* Bit mappings for these bits are not defined */
/*      uu |= (lflag & ????)      ? ECHOKE     : 0;	*/
/*      uu |= (lflag & ????)      ? ECHOPRT    : 0;	*/
/*      uu |= (lflag & ????)      ? ALTWERASE  : 0;	*/
/*      uu |= (lflag & ????)      ? MDMBUF     : 0;	*/
/*      uu |= (lflag & ????)      ? FLUSHO     : 0;	*/
/*      uu |= (lflag & ????)      ? NOHANG     : 0;	*/
/*      uu |= (lflag & ????)      ? PENDIN     : 0;	*/
/*      uu |= (lflag & ????)      ? NOKERNINFO : 0;	*/
/*      uu |= (lflag & U_PRAW)    ? ????       : 0;	*/
/*      uu |= (lflag & U_PPRTERA) ? ????       : 0;	*/
/*      uu |= (lflag & U_PCRTBS)  ? ????       : 0;	*/
/*      uu |= (lflag & U_PCRTERA) ? ????       : 0;	*/
/*      uu |= (lflag & U_PCRTKIL) ? ????       : 0;	*/
	if(lflag &= ~(U_ISIG | U_ICANON | U_XCASE | U_ECHO | U_ECHOE | 
		U_ECHOK | U_ECHONL | U_NOFLSH | U_PIEXTEN | U_PTOSTOP | 
		U_PCTLECH))
		*error = EINVAL;

	return(uu);
}


/*
 * cvtLineOtoU
 *	Convert line discipline from OSF/1 semantics to Ultrix 4.2
 *	If discipline is not supported in Ultrix, "or" 0x40.
 */
cvtLineOtoU(line)
char *line;
{
	if (*line == 0 /*termios*/)
		*line = 2;
	else if (*line == 2 /*Ultrix termios*/)
		*line = 2;
	else if (*line == 4 /*slipdisc*/)
		*line = 7;
	else if (*line == 3 /*tabldisc*/)
		*line = 3;
	else 
		*line |= 0x40;	/* just save OSF value */
}



/*
 * cvtLineUtoO
 *	Convert line discipline from Ultrix 4.2 to OSF/1 format.
 *	If 0x40 bit is set no conversion is needed.
 */
cvtLineUtoO(line)
char *line;
{
	if (*line & 0x40)
		*line &= ~0x40;	/* just turn off bit */
	else if (*line == 2)
		*line = 0;
	else if (*line == 7 /* slipdisc */)
		*line == 4;
	else if (*line == 3 /* tabldisc */)
		*line = 3;
}


/* -------------  open() and fcntl() conversions -------------------- */

/* 
 * Ultrix 4.2 open flags and fcntl flags
 */
#define UO_WRONLY	     001
#define UO_RDWR		     002
#define UO_NDELAY	     004
#define UO_APPEND	     010
#define UO_CREAT	   01000
#define UO_TRUNC	   02000
#define UO_EXCL		   04000
#define UO_FSYNC	 0100000
#define UO_FNOCTTY	02000000
/* Ultrix 4.2 only (not supported by OSF/1) */
#define UO_BLKINUSE	  010000
#define UO_BLKANDSET	  030000
#define UO_TERMIO	01000000
/* Ultrix 4.2 fcntl flags */
#define U_FASYNC	    0100
#define UO_NONBLOCK	 0400000

/* 
 * Function to convert Ultrix 4.2 fcntl F_SETFL flags to OSF bit positions.
 * Input is an int containing the Ultrix 4.2 bits. The output OSF bits
 * are returned by the function.
 */
cvtFcntlUtoO(ult)
int ult;	/* Ultrix 4.2 fcntl flags */
{
int osf = 0;    /* OSF/1 fcntl flags */

	osf |= (ult & UO_NDELAY) ? O_NDELAY : 0;
	osf |= (ult & UO_APPEND) ? O_APPEND : 0;
	osf |= (ult & U_FASYNC) ? FASYNC : 0;
	osf |= (ult & UO_FSYNC) ? O_SYNC : 0;
	osf |= (ult & UO_NONBLOCK) ? O_NONBLOCK : 0;
	osf |= (ult & 3);

	return(osf);
}



/* 
 * Function to convert OSF/1 fcntl F_GETFL flags to  Ultrix 4.2 bit positions.
 * Input is an int containing the OSF/1 bits. The output Ultrix 4.2 bits
 * are returned by the function.
 */
cvtFcntlOtoU(osf)
int osf;	/* OSF/1 fcntl flags */
{
int ult = 0;	/* Ultrix 4.2 fcntl flags */

	ult |= (osf & O_NDELAY) ? UO_NDELAY : 0;
	ult |= (osf & O_APPEND) ? UO_APPEND : 0;
	ult |= (osf & FASYNC) ? U_FASYNC : 0;
	ult |= (osf & O_SYNC) ? UO_FSYNC : 0;
	ult |= (osf & O_NONBLOCK) ? UO_NONBLOCK : 0;
	ult |= (osf & 3);

	return(ult);
}




/* 
 * Function to convert Ultrix 4.2 open flags to OSF bit positions.
 * Input is an int containing the Ultrix 4.2 bits. The output OSF bits
 * are returned by the function.
 */
cvtOpenUtoO(ult)
int ult;	/* Ultrix 4.2 open flags */
{
int osf = 0;    /* OSF/1 open flags */

	osf |= (ult & UO_WRONLY) ? O_WRONLY : 0;
	osf |= (ult & UO_RDWR) ? O_RDWR : 0;
	osf |= (ult & UO_NDELAY) ? O_NDELAY : 0;
	osf |= (ult & UO_APPEND) ? O_APPEND : 0;
	osf |= (ult & U_FASYNC) ? FASYNC : 0;
	osf |= (ult & UO_CREAT) ? O_CREAT : 0;
	osf |= (ult & UO_TRUNC) ? O_TRUNC : 0;
	osf |= (ult & UO_EXCL) ? O_EXCL : 0;
	osf |= (ult & UO_FSYNC) ? O_SYNC : 0;
	osf |= (ult & UO_NONBLOCK) ? O_NONBLOCK : 0;
	osf |= (ult & UO_FNOCTTY) ? O_NOCTTY : 0;

	return(osf);
}


