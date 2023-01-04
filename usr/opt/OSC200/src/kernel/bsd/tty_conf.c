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
static char	*sccsid = "@(#)$RCSfile: tty_conf.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/05/26 17:46:38 $";
#endif 
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>

int	nodev();
int	nulldev();

int	ttyopen(),ttylclose(),ttread(),ttwrite(),nullioctl(),ttstart();
int	ttymodem(), nullmodem(), ttyinput();

#ifdef	ibmrt
int	ttselect();
#endif	ibmrt

int	ottyopen(), ottylclose(), ottread(), ottwrite();
int	ottyinput(), ottstart(), ottymodem();

#ifdef	ibmrt
#include <romp_tb.h>
#undef	NTB
#define NTB	NROMP_TB
#else	ibmrt
#include <tb.h>
#endif	ibmrt
#if	NTB > 0 || NROMP_TB > 0
int	tbopen(),tbclose(),tbread(),tbinput(),tbioctl(),tbselect();
#endif
#ifdef	ibmrt
/* This is a kludge; supposed to be machine independent -- sac */
#include <ms.h>
#if	NMS > 0
int	msdopen(),msdclose(),msdread(),msdinput(),msdioctl(),msdselect();
#endif	NMS
#endif	ibmrt

#if	defined(sun3) || defined(sun4)
#include <ms.h>
#if	NMS > 0
int     msopen(), msclose(), msread(), msioctl(), msinput();
#endif

#include <kb.h>
#if	NKB > 0
int     kbdopen(), kbdclose(), kbdread(), kbdioctl(), kbdinput();
#endif
#endif	defined(sun3) || defined(sun4)

#include <sl.h>
#if	NSL > 0
int	slopen(),slclose(),slinput(),sltioctl(),slstart();
#else
slattach(){ }
#endif

#ifdef	vax
#include <du.h>
#if	NDU > 0
int	dutopen(),dutclose(),dutinput(),dutioctl(),dutstart(),dutmodem();
#endif
#endif

#include <kji.h>
#if NKJI > 0
int	kji_ttread(), kji_ttyinput(), kji_ttwrite();
#endif

struct	linesw linesw[] =
{
	ttyopen, ttylclose, ttread, ttwrite, nullioctl,
	ttyinput, nodev, nulldev, ttstart, ttymodem,	/* 0- termios */
#ifdef	ibmrt
	ttselect,
#endif	ibmrt
	nodev, nodev, nodev, nodev, nodev,		/* 1- defunct */
	nodev, nodev, nodev, nodev, nodev,
#ifdef	ibmrt
	nodev,
#endif	ibmrt
	nodev, nodev, nodev, nodev, nodev,		/* 2- defunct */
	nodev, nodev, nodev, nodev, nodev,
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#if	NTB > 0
	tbopen, tbclose, tbread, nodev, tbioctl,
	tbinput, nodev, nulldev, ttstart, nullmodem,	/* 3- TABLDISC */
#ifdef	ibmrt
	tbselect,
#endif	ibmrt
#else
	nodev, nodev, nodev, nodev, nodev,
	nodev, nodev, nodev, nodev, nodev,
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#endif
#if	NSL > 0
	slopen, slclose, nodev, nodev, sltioctl,
	slinput, nodev, nulldev, slstart, nullmodem,	/* 4- SLIPDISC */
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#else
	nodev, nodev, nodev, nodev, nodev,
	nodev, nodev, nodev, nodev, nodev,
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#endif
#if	defined(ibmrt) && NMS > 0
	msdopen, msdclose, msdread, nodev, msdioctl,
	msdinput, nodev, nulldev, ttstart, nulldev,	/* 5- MOUSELDISC */
	msdselect,
#else	defined(ibmrt) && NMS > 0
#if	(defined(sun3) || defined(sun4)) && NMS > 0
	msopen, msclose, msread, nodev, msioctl,
	msinput, nodev, nulldev, nulldev, nulldev,
#else	(defined(sun3) || defined(sun4)) && NMS > 0
	nodev, nodev, nodev, nodev, nodev,
	nodev, nodev, nodev, nodev, nodev,
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#endif	(defined(sun3) || defined(sun4)) && NMS > 0
#endif	defined(ibmrt) && NMS > 0
#if	(defined(sun3) || defined(sun4)) && NKB > 0
        kbdopen, kbdclose, kbdread, ttwrite, kbdioctl,
	kbdinput, nodev, nulldev, ttstart, nulldev,	/* 6- KDBLDISC */
#else	(defined(sun3) || defined(sun4)) && NKB > 0
	nodev, nodev, nodev, nodev, nodev,
	nodev, nodev, nodev, nodev, nodev,
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#endif	(defined(sun3) || defined(sun4)) && NKB > 0
#if	NDU > 0
	dutopen, dutclose, nodev, nodev, dutioctl,
	dutinput, nodev, nulldev, dutstart, dutmodem,	/* 7- DUDISC */
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#else
	nodev, nodev, nodev, nodev, nodev,
	nodev, nodev, nodev, nodev, nodev,
#ifdef	ibmrt
	nodev,
#endif	ibmrt
#endif
#if NKJI > 0		/* kanji shift JIS */
	ttyopen, ttylclose, kji_ttread, kji_ttwrite, nullioctl,
	kji_ttyinput, nodev, nulldev, ttstart, ttymodem, /* 8- KJIDISC */
#endif
};

int	nldisp = sizeof (linesw) / sizeof (linesw[0]);

/*
 * Do nothing specific version of line
 * discipline specific ioctl command.
 */
/*ARGSUSED*/
nullioctl(tp, cmd, data, flags)
	struct tty *tp;
	char *data;
	int flags;
{

#ifdef	lint
	tp = tp; data = data; flags = flags;
#endif
	return (-1);
}
