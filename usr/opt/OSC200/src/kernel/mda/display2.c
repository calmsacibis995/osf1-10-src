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
static char	*sccsid = "@(#)$RCSfile: display2.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:44 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include "mda.h"
#include <mmax_apc.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <kern/processor.h>


char   *processor_state_names[] = {
				   "Offline",
				   "Running",
				   "Idle",
				   "Dispatching",
				   "Assignment Changing",
				   "Shutting Down"
};

#define	PROC_STATE_SIZE	       (sizeof(processor_state_names)/(sizeof(char *)))


display_processor(vpr)
processor_t vpr;
{
	int     result;
	processor_t ppr;

	result = phys(vpr, &ppr, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	ppr = (processor_t) MAPPED(ppr);

	printthree("xxx", "proc_queue", "next_thread", "idle_thread",
		   ppr->processor_queue, ppr->next_thread, ppr->idle_thread);
	printthree("s  ", "State", "", "",
		   processor_state_names[ppr->state]);
	printthree("xxx", "quantum", "first_quantum", "last_quantum",
		   ppr->quantum, ppr->first_quantum, ppr->last_quantum);
	printthree("xxx", "proc_set", "proc_set_next", "processors",
		   ppr->processor_set, ppr->processor_set_next,
		   ppr->processors.next);
	printthree("xxd", "lock", "processor_self", "slot_num",
		   ppr->lock, ppr->processor_self, ppr->slot_num);
}




display_clist(vcl)
struct clist *vcl;
{
	int     result;
	struct clist *pcl;

	result = phys(vcl, &pcl, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	pcl = (struct clist *) MAPPED(pcl);

	printthree("dxx", "c_cc", "c_cf", "c_cl",
		   pcl->c_cc, pcl->c_cf, pcl->c_cl);
}


/* removed because t_chars no longer exists
 */
#ifdef notdef
display_ttychars(vttc)
struct ttychars *vttc;
{
	int     result;
	struct ttychars *pttc;
	char    ch1[5], ch2[5], ch3[5];

	result = phys(vttc, &pttc, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	pttc = (struct ttychars *) MAPPED(pttc);

	sprintf(ch1, "%s", pttc->tc_erase);
	sprintf(ch2, "%s", pttc->tc_kill);
	sprintf(ch3, "%s", pttc->tc_intrc);

	printthree("sss", "tc_erase", "tc_kill", "tc_intrc",
		   ch1, ch2, ch3);
	printthree("xxx", "tc_quitc", "tc_startc", "tc_stopc",
		   pttc->tc_quitc, pttc->tc_startc, pttc->tc_stopc);
	printthree("xxx", "tc_eofc", "tc_brkc", "tc_suspc",
		   pttc->tc_eofc, pttc->tc_brkc, pttc->tc_suspc);
	printthree("xxx", "tc_dsuspc", "tc_rprntc", "tc_flushc",
		   pttc->tc_dsuspc, pttc->tc_rprntc, pttc->tc_flushc);
	printthree("xx ", "tc_werasc", "tc_lnextc", "",
		   pttc->tc_werasc, pttc->tc_lnextc, 0);
}
#endif /* notdef */


display_winsiz(vws)
struct winsize *vws;
{
	int     result;
	struct winsize *pws;

	result = phys(vws, &pws, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	pws = (struct winsize *) MAPPED(pws);

	printthree("xx ", "ws_row", "ws_col", "",
		   pws->ws_row, pws->ws_col, 0);
	printthree("xx ", "ws_xpixel", "ws_ypixel",
		   pws->ws_xpixel, pws->ws_ypixel, 0);
}


display_queue_entry(vqe, field_name)
struct queue_entry *vqe;
char   *field_name;
{
	int     result;
	struct queue_entry *pqe;

	result = phys(vqe, &pqe, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	pqe = (struct queue_entry *) MAPPED(pqe);

	printthree("sxx", field_name, "next", "prev",
		   "", pqe->next, pqe->prev);
}


display_tty(vtty)
struct tty *vtty;
{
	int     result;
	struct tty *ptty;

	result = phys(vtty, &ptty, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	ptty = (struct tty *) MAPPED(ptty);

	printf("tty structure at %#x:\n", vtty);

	printf("   T_rawq:\n");
	display_clist(&vtty->t_rawq);
	printf("   T_canq:\n");
	display_clist(&vtty->t_canq);
	printf("   t_outq:\n");
	display_clist(&vtty->t_outq);
#if MMAX_MP
	display_queue_entry(&vtty->t_selq, "t_selq");
#else MMAX_MP
	printthree("xx ", "t_rsel", "t_wsel", "",
		   ptty->t_rsel, ptty->t_wsel, 0);
#endif MMAX_MP
	printthree("xxx", "t_oproc", "t_addr", "t_dev",
		   ptty->t_oproc, ptty->t_addr, ptty->t_dev);
	printthree("xdx", "t_flags", "t_state", "t_pgrp",
		   ptty->t_flags, ptty->t_state, ptty->t_pgrp);
	printthree("xxx", "t_line", "t_col", "t_rocol",
		   ptty->t_line, ptty->t_col, ptty->t_rocol);
	printthree("xxx", "t_ispeed", "t_ospeed", "t_rocount",
		   ptty->t_ispeed, ptty->t_ospeed, ptty->t_rocount);
/* removed because t_chars no longer exists
	display_ttychars(&vtty->t_chars);
 */
	display_winsiz(&vtty->t_winsize);
#if CS_TTYLOCK
	printthree("sxx", "t_ttyloc", "tlc_hostid", "tlc_ttyid",
		   "", vtty->t_ttylock.tlc_hostid, vtty->t_ttyloc.ttyid);
#endif CS_TTYLOCK

#if MMAX_MP
	printf("   Lock @ %#x\n", &vtty->t_lock);
	display_lock(&ptty->t_lock);
#endif MMAX_MP
}

display_processor_set(p)
processor_set_t p;
{
	printf("Processor set at %#x:\n", p);
}

display_z256()
{
	int     i;
	struct z256info {
		int     thread;
		int     alloc;
		int     dealloc;
	}      *z256info;

	if (get_address("_z256info", &z256info) != SUCCESS) {
		printf("mda: Could not get address of z256info\n");
		return(FAILED);
	}
	PHYS(z256info, z256info);
	z256info = (struct z256info *) MAPPED(z256info);

	for(i = 0; i < 320; i++) {
		printthree("xxx", "thread", "alloc", "dealloc",
		       z256info->thread, z256info->alloc, z256info->dealloc);
		z256info++;
	}
}
