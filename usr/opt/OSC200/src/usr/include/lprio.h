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
 *	@(#)$RCSfile: lprio.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:16:45 $
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/* lprio.h	5.2 87/03/16 11:23:20 */

#ifndef _LPRIO_H_
#define _LPRIO_H_

#include <sys/termio.h>
#include <sys/ioctl.h>

struct lprio {
	int     ind;            /* indent value */
	int     col;            /* maximum character count */
	int     line;           /* maximum line count */
};

struct lprmode {
	int     modes;          /* optional line printer modes */
};

	/* option flags */
#define PLOT    01
#define NOCL    02      /* no cr/lf */
#define NOFF    0400
#define NONL    01000
#define NOTB    02000
#define NOBS    04000
#define NOCR    010000
#define CAPS    020000
#define WRAP    040000


#define LPR     ('l'<<8)
#define LPRGET  (LPR|01)
#define LPRSET  (LPR|02)
#define LPRGETV (LPR|05)
#define LPRSETV (LPR|06)

/*  IBM additional ioctl's   */

#define LPRVRMG (LPR|10)
#define LPRVRMS (LPR|11)
#define LPRUGES (LPR|12)
#define LPRUFLS (LPR|13)
#define LPRURES (LPR|14)
#define LPRGMOD (LPR|15)
#define LPRSMOD (LPR|16)
#define LPRGETA (LPR|17)
#define LPRSETA (LPR|18)
#define LPRGTOV (LPR|19)
#define LPRSTOV (LPR|20)


/* optional printer modes */
struct oprmode {
	int flags;
};
#define LPRSYNC      0x01
#define LPRALLERR    0x02
#define LPRFONTINIT  0x04
#define SLOWPRNT     0x08


/* error reporting structure */
struct LPRUDE
{       int       status;       /* error reason code */
	int       cresult;      /* current operation result :PSB */
	int       tadapt;       /* adapter type */
	int       npio;         /* number pending IO operations */
};

	/* status values - error reason codes */

#define LPRPOUT  01      /* printer out of forms - intervention req'd */
#define LPRPTIM  0400    /* timeout - intervention required */
#define LPRPERR  01000   /* unspec. internal error - intervention req'd */
#define LPRTERR  02000   /* transmission error */
#define LPRINIT  04000   /* adapter initialization failed */
#define LPRADAP  010000   /* adapter not present */
#define LPRSOFT  020000   /* software error */
#define LPRREAD  040000   /* read error */


	    /* types of adapter */

#define LPRPARALLEL        01
#define LPRSERIAL          02

/* RS232 parameter change structure for LPRGETA and LPRSETA */

struct lpr232 {
	unsigned c_cflag;
};

/* variable timeout value change structure for LPRGTOV and LPRSTOV */
struct lptimer {
	unsigned v_timout;
};

#endif /* _LPRIO_H_ */

