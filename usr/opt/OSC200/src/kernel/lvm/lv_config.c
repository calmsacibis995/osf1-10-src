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
static char	*sccsid = "@(#)$RCSfile: lv_config.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:18:54 $";
#endif 
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
 * COMPONENT_NAME: Logical Volume Manager Device Driver - lv_config.c
 *
 * FUNCTIONS: lv_configure, lv_init, lv_uninit
 */
#include <lvm/lvmd.h>

#include <sys/errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/conf.h>

extern	struct	lv_queue	lv_pbuf_pending_Q;

extern struct volgrp lv_volgrp[];
extern struct lv_crit	lv_lasttime_intlock;
static int lv_initialized = 0;
extern int lv_nlv;

int
lvprobe(unit)
int unit;
{
struct volgrp *vg;
int vgnum;

	if (!lv_initialized) {
		LOCK_INTERRUPT_INIT(&lv_pbuf_lock);
		LOCK_INTERRUPT_INIT(&lv_lasttime_intlock);
		LV_QUEUE_INIT(&lv_pbuf_pending_Q);
	}
	
	for (vgnum = 0; vgnum < lv_nlv; vgnum++) {
		vg = &lv_volgrp[vgnum];
		lock_init(&vg->vg_lock, TRUE);

		LV_QUEUE_INIT(&vg->vg_cache_wait);
		LV_PVQUEUE_INIT(&vg->vg_cache_write);

		printf("lvm%d: configured.\n", vgnum);
	}
	return;
}
