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
static char	*sccsid = "@(#)$RCSfile: conf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:02 $";
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
 *        Copyright 1985 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use.
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 * 	This module defines the entry points of device drivers.
 *
 * Original Author: Who knows?	Created on: Who cares?
 */


/*
 *****************************************************************************
 *                                                                           *
 *		Include Files and External Definitions                       *
 *                                                                           *
 *****************************************************************************
 */

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/socket.h>
#include <kern/lock.h>

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

int 	mmread(),mmwrite();
#define mmselect	seltrue

extern	msdblkopen(),
	msdopen(),
	msdblkclose(),
	msdclose(),
	msdread(),
	msdwrite(),
	msdioctl(),
	msddump(),
	msdpsize(),
	msdstrategy();

extern	mstblkopen(),
	mstopen(),
	mstblkclose(),
	mstclose(),
	mstread(),
	mstwrite(),
	mstioctl(),
	mststrategy();

extern  bbread(),
	bbwrite();

extern	int	logopen(),
		logclose(),
		logread(),
		logioctl(),
		logselect();

extern	int	slcopen(),
		slcclose(),
		slcread(),
		slcwrite(),
		slcioctl(),
		slcselect(),
		slcstop();
extern	int	slccharin(),
		slccharout(),
		slcprintf();
extern	struct	tty slc_tty[];
extern	int	slc_cnt;
#define slcntty	(&slc_cnt)

extern	int	syopen(),
		syread(),
		sywrite(),
		syioctl(),
		syselect(),
		syunselect(),
		sydup();

extern	int	null_read(),
		null_write();

extern	int	ptsopen(),
		ptsclose(),
		ptsread(),
		ptswrite(),
		ptsstop(),
		ptsselect(),
		ptsunselect(),
		ptsdup(),
		ptsgsignal(),
		ptsprintf();
extern	int	ptcopen(),
		ptcclose(),
		ptcread(),
		ptcwrite(),
		ptcselect(),
		ptcdup(),
		ptcunselect(),
		ptyioctl();
extern	struct	tty pt_tty[];
extern	int	npty;
#define ptsntty	(&npty)

#ifdef	RDP
extern	int	rdpopen(),
		rdpclose(),
		rdpread(),
		rdpwrite(),
		rdpioctl(),
		rdpselect(),
		rdpunselect(),
		rdpdup(),
		rdpgsignal(),
		rdpprintf();
#else
#define rdpopen		nodev
#define rdpclose	nodev
#define rdpread		nodev
#define rdpwrite	nodev
#define rdpioctl	nodev
#define rdpselect	nodev
#define rdpunselect	nodev
#define rdpdup		nodev
#endif

extern	int	ttselect(),
		ttunselect(),
		seltrue();

/* Logical Volume Manager pseudo-device */
#include <lv.h>
int lv_nlv = NLV;
#if NLV > 0
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
#define	lv_open2	lv_open
#define	lv_close2	lv_close
#define	lv_strategy2	(int (*)())lv_strategy
#define	lv_read2	lv_read
#define	lv_write2	lv_write
#define	lv_ioctl2	lv_ioctl
#define	lv_volgrp2	((struct tty *)&lv_volgrp[2])
#else
#define	lv_open2	nodev
#define	lv_close2	nodev
#define	lv_strategy2	nodev
#define	lv_read2	nodev
#define	lv_write2	nodev
#define	lv_ioctl2	nodev
#define lv_volgrp2	0
#endif

#if NLV > 3
#error Need to add more declarations to conf.c to configure this many LVMs.
#endif

/*
 *****************************************************************************
 *                                                                           *
 *		Local Macros                                                 *
 *                                                                           *
 *****************************************************************************
 */
#define cnopen		slcopen
#define cnclose		slcclose
#define cnread		slcread
#define cnwrite		slcwrite
#define cnioctl		slcioctl
#define cncharin	slccharin
#define cncharout	slccharout
#define cnprintf	slcprintf
#define cnunselect	ttunselect
#define cons		(slc_tty[0])
#define cnntty		(&slc_cnt)


int	msd_bmajor  = 0;

/*
 *      Block Device Switch static declaration and initialization
 */

#define MAX_BDEVSW      32
dswlock_t       bdevlock[MAX_BDEVSW];           /* bdevsw lock structure */
struct bdevsw	bdevsw[MAX_BDEVSW] =
{
	{ msdblkopen,	msdblkclose,	msdstrategy,	msddump,	/*0*/
	  msdpsize,	0,		nodev		DEV_FUNNEL_NULL },
	{ mstblkopen,	mstblkclose,	mststrategy,	nulldev,	/*1*/
	  0,		1,		nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*2*/
	  0,		0,		nodev		DEV_FUNNEL_NULL },
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
	/* Logical Volume Manager devices - cmajor and bmajor must match */
	{ lv_open0,	lv_close0,	lv_strategy0,	nulldev,	/*10*/
	  0,		0,		lv_ioctl0	DEV_FUNNEL_NULL },
	{ lv_open1,	lv_close1,	lv_strategy1,	nulldev,	/*11*/
	  0,		0,		lv_ioctl1	DEV_FUNNEL_NULL },
	{ lv_open2,	lv_close2,	lv_strategy2,	nulldev,	/*12*/
	  0,		0,		lv_ioctl2	DEV_FUNNEL_NULL }
};

int	nblkdev = ( sizeof (bdevsw) / sizeof (struct bdevsw) );

/*
 * Declare the block device major numbers
 */
char	Rootmajor = 0;
char	Pagemajor = 0;
char	Dumpmajor = 0;

char	Bdev_disk_major = 0;
char	Bdev_tape_major = 1;


#include <aud.h>
#if     NAUD > 0
int     audopen(),audclose(),audread(),audwrite(),audioctl();
#else
#define audopen         nodev
#define audclose        nodev
#define audread         nodev
#define audwrite        nodev
#define audioctl        nodev
#endif

#include <spd.h>
#if     NSPD > 0
int     spdopen(),spdclose(),spdread(),spdwrite(),spdioctl();
#else
#define spdopen         nodev
#define spdclose        nodev
#define spdread         nodev
#define spdwrite        nodev
#define spdioctl        nodev
#endif

#include "streams.h"
#if	STREAMS
int	clone_open();
#else
#define clone_open	nodev
#endif

/*
 *      Character Device Switch static declaration and initialization
 */
#define MAX_CDEVSW      64
dswlock_t       cdevlock[MAX_CDEVSW];           /* cdevsw lock structure */
struct cdevsw	cdevsw[MAX_CDEVSW] =
{
	{ cnopen,	cnclose,	cnread,		cnwrite,	/*0*/
	  cnioctl,	nulldev,	nulldev,	&cons,
	  ttselect,	nodev		DEV_FUNNEL_NULL },
	{ slcopen,	slcclose,	slcread,	slcwrite,	/*1*/
	  slcioctl,	slcstop,	nulldev,	slc_tty,
	  slcselect,	nodev		DEV_FUNNEL_NULL },
	{ syopen,	nulldev,	syread,		sywrite,	/*2*/
	  syioctl,	nulldev,	nulldev,	0,
	  syselect,	nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	mmread,		mmwrite,	/*3*/
	  nodev,	nulldev,	nulldev,	0,
	  mmselect,	nodev		DEV_FUNNEL_NULL },
	{ msdopen,	msdclose,	msdread,	msdwrite,	/*4*/
	  msdioctl,	nodev,		nodev,		0,
	  seltrue,	nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	nulldev,	nulldev,	/*5*/
	  nodev,	nodev,		nulldev,	0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ ptsopen,	ptsclose,	ptsread,	ptswrite,	/*6*/
	  ptyioctl,	ptsstop,	nodev,		pt_tty,
	  ttselect,	nodev		DEV_FUNNEL_NULL },
	{ ptcopen,	ptcclose,	ptcread,	ptcwrite,	/*7*/
	  ptyioctl,	nulldev,	nodev,		pt_tty,
	  ptcselect,	nodev		DEV_FUNNEL_NULL },
	{ mstopen,	mstclose,	mstread,	mstwrite,	/*8*/
	  mstioctl,	nodev,		nodev,		0,
	  seltrue,	nodev		DEV_FUNNEL_NULL },
	{ rdpopen,	rdpclose,	rdpread,	rdpwrite,	/*9*/
	  rdpioctl,	nodev,		nodev,		0,
	  rdpselect,	nodev		DEV_FUNNEL_NULL },
	/* Logical Volume Manager devices - cmajor and bmajor must match */
	{ lv_open0,	lv_close0,	lv_read0,	lv_write0,	/*10*/
	  lv_ioctl0,	nodev,		nodev,		lv_volgrp0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ lv_open1,	lv_close1,	lv_read1,	lv_write1,	/*11*/
	  lv_ioctl1,	nodev,		nodev,		lv_volgrp1,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ lv_open2,	lv_close2,	lv_read2,	lv_write2,	/*12*/
	  lv_ioctl2,	nodev,		nodev,		lv_volgrp2,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ nodev,	nodev,		nodev,		nodev,		/*13*/
	  nodev,	nodev,		nodev,		0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ nulldev,	nulldev,	bbread,		bbwrite,	/*14*/
	  nodev,	nulldev,	nulldev,	0,
	  seltrue,	nodev		DEV_FUNNEL_NULL },
	{ logopen,	logclose,	logread,	nodev,		/*15*/
	  logioctl,	nodev,		nulldev,	0,
	  logselect,	nodev		DEV_FUNNEL_NULL },
	{ timeopen,	timeclose,	nodev,		nodev,		/*16*/
	  nodev,	nodev,		nulldev,	0,
	  nodev,	timemap		DEV_FUNNEL_NULL },
	{ audopen,	audclose,	audread,	audwrite,       /*17*/
	  audioctl,	nodev,		nulldev,	0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ spdopen,	spdclose,	spdread,	spdwrite,       /*18*/
	  spdioctl,	nodev,		nulldev,	0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ nodev,	nodev,		nodev,		nodev,		/*19*/
	  nodev,	nodev,		nodev,		0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ nodev,	nodev,		nodev,		nodev,		/*20*/
	  nodev,	nodev,		nodev,		0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ nodev,	nodev,		nodev,		nodev,		/*21*/
	  nodev,	nodev,		nodev,		0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ nodev,	nodev,		nodev,		nodev,		/*22*/
	  nodev,	nodev,		nodev,		0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	{ nodev,	nodev,		nodev,		nodev,		/*23*/
	  nodev,	nodev,		nodev,		0,
	  nodev,	nodev		DEV_FUNNEL_NULL },
	/* STREAMS clone device needs N empty slots following
	 * for configuring N static STREAMS drivers. N ~= 10.
	 * If all STREAMS drivers are dynamic, N == 0. If not
	 * available, it's not fatal but str_init will complain. */
	{clone_open,    nodev,          nodev,          nodev,          /*24*/
	nodev,          nodev,          nodev,          0,
	nodev,          nodev		DEV_FUNNEL_NULL }
};

int	nchrdev = ( sizeof (cdevsw) / sizeof (struct cdevsw) );

/*
 * Declare the console device major number
 */
char	Consolemajor = 0;

char	Cdev_console_major = 	0;
char	Cdev_sl_major = 	1;
char	Cdev_disk_major = 	4;
char	Cdev_swap_major = 	5;
char	Cdev_slave_pty_major = 	6;
char	Cdev_master_pty_major = 7;
char	Cdev_tape_major = 	8;
char	Cdev_rdp_major = 	9;

dev_t	sydev = makedev(2,0);	/* dev number for indirect tty */
int	mem_no = 3; 	/* major device number of memory special file */
