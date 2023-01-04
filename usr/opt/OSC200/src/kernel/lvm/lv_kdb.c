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
static char	*sccsid = "@(#)$RCSfile: lv_kdb.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:19:29 $";
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
#include <mach_kdb.h>
#include <mach_assert.h>
#if     MACH_KDB

#include <lvm/lvmd.h>

#define printf	kdbprintf

#if     MACH_SLOCKS
void print_simple_lock();
#endif
void buffer_print();
void print_rw_lock();

lv_print(c, addr)
char c;
caddr_t addr;
{
	switch(c) {
	case 'g': lv_print_volgrp(addr); break;
	case 'l': lv_print_lvol(addr); break;
	case 'p': lv_print_pbuf(addr); break;
	case 'v': lv_print_pvol(addr); break;
	case '?':	/* help list */
	default:
		printf("$V commands are:\n\n");
		printf("\t$Vg\tprint struct volgrp\n");
		printf("\t$Vl\tprint struct lvol\n");
		printf("\t$Vp\tprint struct pbuf\n");
		printf("\t$Vv\tprint struct pvol\n");
		printf("\n\t$V?\tprint this list\n");
	}
	return;
}

lv_print_volgrp(vg)
struct volgrp *vg;
{
int lvnum, pvnum;

	print_rw_lock(&(vg->vg_lock));
	printf("num_lvols = %d\n", vg->num_lvols);
	for (lvnum = 0; lvnum < vg->num_lvols; lvnum++) {
		if (vg->lvols[lvnum]) {
			printf("[%d] %X ", lvnum, vg->lvols[lvnum]);
		}
	}
	printf("\n");
	printf("size_pvols =  %d num_pvols: = %d\n",
		vg->size_pvols, vg->num_pvols);
	for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
		if (vg->pvols[pvnum]) {
			printf("[%d] %X ", pvnum, vg->pvols[pvnum]);
		}
	}
	printf("\n");
	printf("major_num = %X\n", vg->major_num);
	printf("vg_maxlvs = %X vg_maxpvs = %X vg_maxpxs = %X\n",
		vg->vg_maxlvs, vg->vg_maxpvs, vg->vg_maxpxs);
	printf("vg_flags = %X\n", vg->vg_flags);
#if     MACH_SLOCKS
	printf("vg_intlock interlock:\n");
	print_simple_lock(&vg->vg_intlock.lvc_lock);
#endif
	/* Status Area Info */
	printf("sa_header = %X\n", vg->vg_sa_ptr.sa_header);
	if (vg->vg_sa_ptr.sa_header) {
		printf("VGSA H timestamp %X %X\n",
			SA_H_TIMESTAMP(vg).tv_sec,
			SA_H_TIMESTAMP(vg).tv_usec);
		printf("VGSA maxpvs %X maxpxs %X\n",
			SA_MAXPVS(vg), SA_MAXPXS(vg));
	}
	if (vg->vg_sa_ptr.sa_trailer) {
		printf("VGSA T timestamp %X %X\n",
			SA_T_TIMESTAMP(vg).tv_sec,
			SA_T_TIMESTAMP(vg).tv_usec);
	}
	printf("vg_sa_wheel %X\n", vg->vg_sa_wheel);
	return;
}

lv_print_lvol(lv)
struct lvol *lv;
{
int i;
	if (lv->work_Q) {
		printf("work_Q anchors: ");
		for (i = 0; i < WORKQ_SIZE; i++) {
			if ((i%4) == 0)
				printf("\n");
			printf("%X%16t", lv->work_Q[i]);
		}
		printf("\n");
	} else {
		printf("work_Q NULL\n");
	}
	printf("lv_lext = %X,", lv->lv_lext);
	printf("lv_exts[0] = %X, lv_exts[1] = %X, lv_exts[2] = %X\n",
		lv->lv_exts[0], lv->lv_exts[1], lv->lv_exts[2]);
	printf("lv_schedule = %X\n", lv->lv_schedule);
	printf("lv_lock = %X\n", &lv->lv_lock);
	printf("lv_complcnt = %X, lv_totalcount = %X, lv_requestcount = %X\n",
		lv->lv_complcnt, lv->lv_totalcount, lv->lv_requestcount);
#if     MACH_SLOCKS
	printf("lv_intlock interlock:\n");
	print_simple_lock(&lv->lv_intlock.lvc_lock);
#endif
	
	return;
}

lv_print_pbuf(pb)
struct pbuf *pb;
{
	return;
}

lv_print_pvol(pv)
struct pvol *pv;
{
	printf("pvum %X\n", pv->pv_num);
	printf("pv_vgra_psn = %X\n", pv->pv_vgra_psn);
	printf("pv_sa_psn %X %X\n",
				pv->pv_sa_psn[0],
				pv->pv_sa_psn[1]);
	printf("VGSA 0 Timestamp %X %X\n",
				pv->pv_vgsats[0].tv_sec,
				pv->pv_vgsats[0].tv_usec);
	printf("VGSA 1 Timestamp %X %X\n",
				pv->pv_vgsats[1].tv_sec,
				pv->pv_vgsats[1].tv_usec);
	return;
}

#endif
