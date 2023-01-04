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
 *	@(#)$RCSfile: termio.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/12 19:04:14 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 


/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _SYS_TERMIO_H_
#define _SYS_TERMIO_H_

#include <standards.h>

#include <sys/ioctl.h>
#include <sys/ttmap.h>

/* termios defines all the bits, baudrates, etc. */
#include <sys/termios.h>

/*
 * Ioctl control packet
 */

#define NCC 8

struct termio {
        unsigned short c_iflag;         /* input modes */
        unsigned short c_oflag;         /* output modes */
        unsigned short c_cflag;         /* control modes */
        unsigned short c_lflag;         /* line discipline modes */
        char c_line;                    /* line discipline */
        unsigned char c_cc[NCC];        /* control chars */
};

/* Unfortunately System V have and POSIX have different values for some of
 * the defines.  So we have to perform the following kluge which redefines
 * the V___ values for termio.  This means you cannot do ioctls using termios
 * and termio in the same source file.
 */
#ifdef _KERNEL
#define VVINTR	0
#define VVQUIT	1
#define	VVERASE	2
#define VVKILL	3
#define	VVEOF	4
#define VVMIN	4
#define	VVEOL	5
#define VVTIME	5
#define VVEOL2  6
#define VVSWTCH 7

#define VNOFLSH  0x8000		/*
				 * This is the system V value the BSD
				 * equalivent is 0x80000000 which won't fit
				 * in a termio structure.
				 */
#else
 
/* Redefine the these values for SVID */
#undef	VEOF
#undef	VEOL
#undef	VERASE
#undef VKILL
#undef VINTR
#undef VQUIT
#undef VMIN	
#undef VTIME	
#undef VEOL2  
#undef VSWTCH 
#undef NOFLSH

#define VINTR	0
#define VQUIT	1
#define	VERASE	2
#define VKILL	3
#define	VEOF	4
#define VMIN	4
#define	VEOL	5
#define VTIME	5
#define VEOL2  6
#define VSWTCH 7

#define NOFLSH 0x8000
#endif /* _KERNEL */

#define TIOC            _IO('t', 0)      /* Specifies ioctl group */

#define LDIOC           ('D'<<8)
#define LDOPEN          (LDIOC|0)
#define LDCLOSE         (LDIOC|1)
#define LDCHG           (LDIOC|2)
#define LDGETT          (LDIOC|8)
#define LDSETT          (LDIOC|9)

/*
 * Terminal types
 */
#define TERM_NONE       0       /* tty */
#define TERM_TEC        1       /* TEC Scope */
#define TERM_V61        2       /* DEC VT61 */
#define TERM_V10        3       /* DEC VT100 */
#define TERM_TEX        4       /* Tektronix 4023 */
#define TERM_D40        5       /* TTY Mod 40/1 */
#define TERM_H45        6       /* Hewlitt-Packard 45 */
#define TERM_D42        7       /* TTY Mod 40/2B */

/*
 * Terminal flags
 */
#define TM_NONE         0000    /* use default flags */
#define TM_SNL          0001    /* special newline flag */
#define TM_ANL          0002    /* auto newline on column 80 */
#define TM_LCF          0004    /* last col of last row special */
#define TM_CECHO        0010    /* echo terminal cursor control */
#define TM_CINVIS       0020    /* do not send esc seq to user */
#define TM_SET          0200    /* must be on to set/res flags */

/*
 * structure of ioctl arg for LDGETT and LDSETT
 */
struct  termcb  {
        char    st_flgs;        /* term flags */
        char    st_termt;       /* term type */
        char    st_crow;        /* gtty only - current row */
        char    st_ccol;        /* gtty only - current col */
        char    st_vrow;        /* variable row */
        char    st_lrow;        /* last row */
};

#endif /* _SYS_TERMIO_H_ */
