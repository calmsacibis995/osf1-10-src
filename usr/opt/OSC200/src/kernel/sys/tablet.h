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
 *	@(#)$RCSfile: tablet.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/06/01 19:11:11 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
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
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1985, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#ifndef	_SYS_TABLET_H_
#define _SYS_TABLET_H_

/*
 * Tablet line discipline.
 */

#ifdef	KERNEL
#ifdef	ibmrt
#include <romp_tbcompat.h>
#endif	/* ibmrt */
#include <sys/ioctl.h>
#endif	/* KERNEL */




/*
 * Reads on the tablet return one of the following
 * structures, depending on the underlying tablet type.
 * The first two are defined such that a read of
 * sizeof (gtcopos) on a non-gtco tablet will return
 * meaningful info.  The in-proximity bit is simulated
 * where the tablet does not directly provide the information.
 */
struct	tbmspos {
	int	xpos, ypos;	/* raw x-y coordinates */
	short	status;		/* buttons/pen down */
#define TBINPROX	0100000		/* pen in proximity of tablet */
	short	scount;		/* sample count */
};

struct	gtcopos {
	int	xpos, ypos;	/* raw x-y coordinates */
	short	status;		/* as above */
	short	scount;		/* sample count */
	short	xtilt, ytilt;	/* raw tilt */
	short	pressure;
	short	pad;		/* pad to longword boundary */
};

struct	polpos {
	short	p_x, p_y, p_z;	/* raw 3-space coordinates */
	short	p_azi, p_pit, p_rol;	/* azimuth, pitch, and roll */
	short	p_stat;		/* status, as above */
	char	p_key;		/* calculator input keyboard */
};

/*
 * Tablet state
 */

#define NTBS		(16)
#define TBMAXREC	(17)	/* max input record size */
#define TBQUEUESIZE	(5)

struct tb {
	int	tbflags;		/* mode & type bits */
	char	cbuf[TBMAXREC];		/* input buffer */
#ifdef	ibmrt
	short	lastindex;
	short	curindex;
#endif	/* ibmrt */
	union {
		struct	tbmspos tbpos;
		struct	gtcopos gtcopos;
		struct	polpos polpos;
#ifdef	ibmrt		
	} rets[TBQUEUESIZE];				/* processed state */
#else	/* ibmrt */
        } rets;
#endif	/* ibmrt	 */
} tb[NTBS];

typedef struct {
        ws_event ws;
        short axis_data[2];
} tablet_event;

#define BIOSMODE	_IOW('b', 1, int)	/* set mode bit(s) */
#define BIOGMODE	_IOR('b', 2, int)	/* get mode bit(s) */
#define TBMODE		0xfff0		/* mode bits: */
#define		TBPOINT		0x0010		/* single point */
#define		TBRUN		0x0000		/* runs contin. */
#define		TBSTOP		0x0020		/* shut-up */
#define		TBGO		0x0000		/* ~TBSTOP */
#define		TBBINDATA	0x0040	/* Mask off high bit of in data? */
#define TBTYPE		0x000f		/* tablet type: */
#if	defined(KERNEL) && defined(ibmrt) && ROMP_TBCOMPAT
#define		HITACHI_DISC	0
#define		GTCO_DISC	1
#define		CALCOMP_DISC	2
#define		PCMS_DISC	3
#define		PLANMS_DISC3	4
#define		PLANMS_DISC2	5
#else	/* defined(KERNEL) && defined(ibmrt) && ROMP_TBCOMPAT */
#define		TBUNUSED	0x0000
#define		TBHITACHI	0x0001		/* hitachi tablet */
#define		TBTIGER		0x0002		/* hitachi tiger */
#define		TBGTCO		0x0003		/* gtco */
#define		TBPOL		0x0004		/* polhemus 3space */
#define		TBHDG		0x0005		/* hdg-1111b, low res */
#define		TBHDGHIRES	0x0006		/* hdg-1111b, high res */
#define 	CALCOMP_DISC	0x0007		
#define 	PCMS_DISC	0x0008
#define 	PLANMS_DISC3	0x0009
#define 	PLANMS_DISC2	0x000a
#ifdef	ibmrt
#define		HITACHI_DISC	TBHITACHI	/* For compatability */
#define		GTCO_DISC	TBGTCO
#endif	/* ibmrt */
#endif	/* defined(KERNEL) && defined(ibmrt) && ROMP_TBCOMPAT */

#define TBIOGETD	_IOR('b',0,int)
#define TBIOSETD	_IOWR('b',1,int)
#define BIOSTYPE	_IOW('b', 3, int)	/* set tablet type */
#define BIOGTYPE	_IOR('b', 4, int)      	/* get tablet type*/
#ifdef	ibmrt
#define TBIOGETC	_IOR('b',5,int)		/* get compat mode flag */
#define TBIOSETC	_IOW('b',6,int)		/* set compat mode flag */
#endif	/* ibmrt */

#endif	/* _SYS_TABLET_H_ */
