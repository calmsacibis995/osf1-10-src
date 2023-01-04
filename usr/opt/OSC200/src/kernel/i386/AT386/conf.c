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
static char	*sccsid = "@(#)$RCSfile: conf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:07 $";
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
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>

#if	SER_COMPAT
#define	DEV_FUNNEL_NULL	,FUNNEL_NULL
#else
#define	DEV_FUNNEL_NULL
#endif

int	nulldev();
int	nodev();

int	timeopen();
int	timeclose();
int	timemap();

int	ioplopen(), ioplclose();
int	ioplmmap();

#include "hd.h"
#if	NHD > 0
int	hdopen(), hdstrategy(), hdread(), hdwrite(), hddump(), hdioctl(), hdsize();
#else
#define	hdopen		nulldev
#define	hdstrategy	nulldev
#define	hdread		nulldev
#define	hdwrite		nulldev
#define	hddump		nulldev
#define	hdioctl		nulldev
#define	hdsize		nulldev
#endif

#include "fd.h"
#if	NFD > 0
int	fdopen(), fdstrategy(), fdread(), fdwrite(), fddump(), fdioctl(), fdsize();
#else
#define	fdopen		nulldev
#define	fdstrategy	nulldev
#define	fdread		nulldev
#define	fdwrite		nulldev
#define	fddump		nulldev
#define	fdioctl		nulldev
#define	fdsize		nulldev
#endif

#include "wt.h"
#if	NWT > 0
int	wtopen(), wtstrategy(), wtread(), wtwrite(), wtdump(), wtioctl(), wtsize(), wtclose();
#else
#define	wtopen		nulldev
#define	wtclose		nulldev
#define	wtread		nulldev
#define	wtwrite		nulldev
#define	wtioctl		nulldev
#define	wtstrategy	nulldev
#define	wtdump		nulldev
#define	wtsize		nulldev
#endif

#include "ln.h"
#if	NLN > 0
int lnopen(), lnclose(), lnioctl(), lnselect();
#else
#define lnopen    nodev
#define lnclose   nodev
#define lnioctl   nodev
#define lnselect  nodev
#endif

#include "ec.h"
#if	NEC > 0
int ecopen(), ecclose(), ecioctl();
#else
#define ecopen    nodev
#define ecclose   nodev
#define ecioctl   nodev
#endif

/* Logical Volume Manager pseudo-device */
#include "lv.h"
#if NLV > 0
int lv_nlv = NLV;
#include <lvm/lvmd.h>
int	lv_open(), lv_close();
void	lv_strategy();
int	lv_read(), lv_ioctl(), lv_write();
struct volgrp lv_volgrp[NLV];
#endif

#if NLV > 0
#define	lv_open0	lv_open
#define	lv_close0	lv_close
#define	lv_strategy0	(int (*)())lv_strategy
#define	lv_read0	lv_read
#define	lv_write0	lv_write
#define	lv_ioctl0	lv_ioctl
#define	lv_volgrp0	((struct tty *)&lv_volgrp[0])
#else
#define	lv_open0	nodev
#define	lv_close0	nodev
#define	lv_strategy0	nodev
#define	lv_read0	nodev
#define	lv_write0	nodev
#define	lv_ioctl0	nodev
#define lv_volgrp0	0
#endif

#if NLV > 1
#define	lv_open1	lv_open
#define	lv_close1	lv_close
#define	lv_strategy1	(int (*)())lv_strategy
#define	lv_read1	lv_read
#define	lv_write1	lv_write
#define	lv_ioctl1	lv_ioctl
#define	lv_volgrp1	((struct tty *)&lv_volgrp[1])
#else
#define	lv_open1	nodev
#define	lv_close1	nodev
#define	lv_strategy1	nodev
#define	lv_read1	nodev
#define	lv_write1	nodev
#define	lv_ioctl1	nodev
#define lv_volgrp1	0
#endif

#if NLV > 2
#error Need to add more declarations to conf.c to configure this many LVMs.
#endif

#define MAX_BDEVSW      32
dswlock_t       bdevlock[MAX_BDEVSW];           /* bdevsw lock structure */
struct bdevsw	bdevsw[MAX_BDEVSW] =
{
	{ hdopen,	nulldev,	hdstrategy,	hddump,		/*0*/
	  hdsize,	0,		hdioctl		DEV_FUNNEL_NULL },
	{ fdopen,	nulldev,	fdstrategy,	fddump,		/*1*/
	  fdsize,	0,		fdioctl		DEV_FUNNEL_NULL},
	{ wtopen,	nulldev,	wtstrategy,	wtdump,		/*2*/
	  wtsize,	0,		wtioctl		DEV_FUNNEL_NULL},
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*3*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*4*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*5*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*6*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*7*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*8*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*9*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*10*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*11*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*12*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*13*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*14*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*15*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*16*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
	/* Logical Volume Manager devices - cmajor and bmajor must match */
	{ lv_open0,	lv_close0,	lv_strategy0,	nulldev,	/*17*/
	  0,		0,		lv_ioctl0	DEV_FUNNEL_NULL },
	{ lv_open1,	lv_close1,	lv_strategy1,	nulldev,	/*18*/
	  0,		0,		lv_ioctl1	DEV_FUNNEL_NULL }
};
int	nblkdev = sizeof (bdevsw) / sizeof (bdevsw[0]);

int	kdopen(), kdclose(), kdread(), kdwrite(), kdioctl();
extern struct tty kd_tty[];

int	kbdinit(), kbdopen(), kbdclose(), kbdioctl(), kbdselect(), kbdread();
int	kdmmap();

#include "com.h"
#if	NCOM > 0
int	comopen(), comclose(), comread(), comwrite(), comioctl(), comstop();
extern	struct	tty	com_tty[];
#else
#define comopen		nodev
#define comclose	nodev
#define comread		nodev
#define comwrite	nodev
#define comioctl	nodev
#define comstop		nodev
#define com_tty		0
#endif

#include "qd.h"
#if	NQD > 0
int	qdopen(), qdclose(), qdread(), qdwrite(), qdioctl();
extern	struct	tty	qd_tty[];
#else
#define qdopen		nodev
#define qdclose		nodev
#define qdread		nodev
#define qdwrite		nodev
#define qdioctl		nodev
#define qd_tty		0
#endif

int	cnopen(),cnclose(),cnread(),cnwrite(),cnioctl();
struct tty cons;

int	syopen(),syread(),sywrite(),syioctl(),syselect();

int 	mmread(),mmwrite();
#define mmselect	seltrue

#include <pty.h>
#if	NPTY > 0
int	ptsopen(),ptsclose(),ptsread(),ptswrite(),ptsstop();
int	ptcopen(),ptcclose(),ptcread(),ptcwrite(),ptcselect();
int	ptyioctl();
extern struct	tty pt_tty[];
#else
#define ptsopen		nodev
#define ptsclose	nodev
#define ptsread		nodev
#define ptswrite	nodev
#define ptcopen		nodev
#define ptcclose	nodev
#define ptcread		nodev
#define ptcwrite	nodev
#define ptyioctl	nodev
#define pt_tty		0
#define ptcselect	nodev
#define ptsstop		nulldev
#endif

#include "blit.h"
#if	NBLIT > 0
int	blitopen(), blitclose(), blitioctl(), blitmmap();
#else
#define	blitopen	nodev
#define	blitclose	nodev
#define	blitioctl	nodev
#define	blitmmap	nodev
#endif 

#include "mouse.h"
#if	NMOUSE > 0
int	mouseinit(), mouseopen(), mouseclose(), mouseioctl(), mouseselect();
int	mouseread();
#else
#define	mouseinit	nodev
#define	mouseopen	nodev
#define	mouseclose	nodev
#define	mouseioctl	nodev
#define	mouseselect	nodev
#define	mouseread	nodev
#endif

#include "streams.h"
#if	STREAMS
int	clone_open();
#else
#define clone_open	nodev
#endif

int	logopen(),logclose(),logread(),logioctl(),logselect();

int	ttselect(), seltrue();

#define MAX_CDEVSW      64
dswlock_t       cdevlock[MAX_CDEVSW];           /* cdevsw lock structure */
struct cdevsw	cdevsw[MAX_CDEVSW] =
{
	{comopen,	comclose,	comread,	comwrite,	/*0*/
	comioctl,	comstop,	nulldev,	com_tty,
	ttselect,	nodev 		DEV_FUNNEL_NULL},

	{kdopen,	kdclose,	kdread,		kdwrite,	/*1*/
	kdioctl,	nulldev,	nulldev,	kd_tty,
	ttselect,	kdmmap	 	DEV_FUNNEL_NULL},

	{syopen,	nulldev,	syread,		sywrite,	/*2*/
	syioctl,	nulldev,	nulldev,	0,
	syselect,	nodev		DEV_FUNNEL_NULL},

	{nulldev,	nulldev,	mmread,		mmwrite,	/*3*/
	nodev,		nulldev,	nulldev,	0,
	mmselect,	nodev 		DEV_FUNNEL_NULL},

	{hdopen,	nulldev,	hdread,		hdwrite,	/*4*/
	hdioctl,	nodev,		nulldev,	0,
	seltrue,	nodev 		DEV_FUNNEL_NULL},

	{fdopen,	nulldev,	fdread,		fdwrite,	/*5*/
	fdioctl,	nodev,		nulldev,	0,
	seltrue,	nodev 		DEV_FUNNEL_NULL},

	{wtopen,	wtclose,	wtread,		wtwrite,	/*6*/
	wtioctl,	nodev,		nulldev,	0,
	seltrue,	nodev 		DEV_FUNNEL_NULL},

 	{blitopen,	blitclose,	nodev,		nodev,		/*7*/
 	blitioctl,	nodev,		nulldev,	0,
 	nodev,		blitmmap 	DEV_FUNNEL_NULL},

 	{lnopen,	lnclose,	nodev,		nodev,		/*8*/
 	lnioctl,	nodev,		nodev,		0,
 	lnselect,	nodev 		DEV_FUNNEL_NULL},

	{ptsopen,	ptsclose,	ptsread,	ptswrite,	/*9*/
	ptyioctl,	ptsstop,	nodev,		pt_tty,
	ttselect,	nodev 		DEV_FUNNEL_NULL},

	{ptcopen,	ptcclose,	ptcread,	ptcwrite,	/*10*/
	ptyioctl,	nulldev,	nodev,		pt_tty,
	ptcselect,	nodev 		DEV_FUNNEL_NULL},
	
	{kbdopen,	kbdclose,	kbdread,	nodev,		/*11*/
	kbdioctl,	nodev,		nodev,		0,
	kbdselect,	nodev 		DEV_FUNNEL_NULL},

	{mouseopen,	mouseclose,	mouseread,	nodev,		/*12*/
	mouseioctl,	nodev,		nodev,		0,
	mouseselect,	nodev 		DEV_FUNNEL_NULL},

	{qdopen,	qdclose,	qdread,		qdwrite,	/*13*/
	qdioctl,	nulldev,	nulldev,	qd_tty,
	ttselect,	nodev 		DEV_FUNNEL_NULL},

 	{timeopen,	timeclose,	nodev,		nodev,		/*14*/
 	nodev,		nodev,		nulldev,	0,
 	nodev,		timemap 	DEV_FUNNEL_NULL},

	{ecopen,        ecclose,        nodev,          nodev,          /*15*/
	ecioctl,        nodev,          nodev,          0,
	nodev,          nodev 		DEV_FUNNEL_NULL},

	{ioplopen,      ioplclose,      nodev,          nodev,          /*16*/
	nodev,          nodev,          nulldev,        0,
	nodev,          ioplmmap	DEV_FUNNEL_NULL },

	/* Logical Volume Manager devices - cmajor and bmajor must match */
	{ lv_open0,	lv_close0,	lv_read0,	lv_write0,	/*17*/
	  lv_ioctl0,	nodev,		nodev,		lv_volgrp0,
	  nodev,	nodev		DEV_FUNNEL_NULL },

	{ lv_open1,	lv_close1,	lv_read1,	lv_write1,	/*18*/
	  lv_ioctl1,	nodev,		nodev,		lv_volgrp1,
	  nodev,	nodev		DEV_FUNNEL_NULL },

	{logopen,	logclose,	logread,	nodev,		/*19*/
        logioctl,	nodev,		nulldev,	0,
	logselect,	nodev		DEV_FUNNEL_NULL },

	{nodev,         nodev,          nodev,          nodev,          /*20*/
	nodev,          nodev,          nodev,          0,
	nodev,          nodev		DEV_FUNNEL_NULL },

	{nodev,         nodev,          nodev,          nodev,          /*21*/
	nodev,          nodev,          nodev,          0,
	nodev,          nodev		DEV_FUNNEL_NULL },

	{nodev,         nodev,          nodev,          nodev,          /*22*/
	nodev,          nodev,          nodev,          0,
	nodev,          nodev		DEV_FUNNEL_NULL },

	{nodev,         nodev,          nodev,          nodev,          /*23*/
	nodev,          nodev,          nodev,          0,
	nodev,          nodev		DEV_FUNNEL_NULL },

	/* STREAMS clone device needs N empty slots following
	 * for configuring N static STREAMS drivers. N ~= 10.
	 * If all STREAMS drivers are dynamic, N == 0. If not
	 * available, it's not fatal but str_init will complain. */
	{clone_open,    nodev,          nodev,          nodev,          /*24*/
	nodev,          nodev,          nodev,          0,
	nodev,          nodev		DEV_FUNNEL_NULL }

};
int	nchrdev = sizeof (cdevsw) / sizeof (cdevsw[0]);

int	mem_no = 3; 	/* major device number of memory special file */

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(0, 4);

