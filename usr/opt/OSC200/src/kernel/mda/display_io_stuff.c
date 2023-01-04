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
static char	*sccsid = "@(#)$RCSfile: display_io_stuff.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:50 $";
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
#include <mmax_mp.h>

#include "mda.h"
#include <sys/types.h>
#include <mmaxio/crqdefs.h>
#include <mmaxio/emcdefs.h>
#include <mmaxio/msdefs.h>
#include <mmaxio/elog.h>
#include <mmaxio/ms_dev.h>

display_crq(crqp)
struct crq *crqp;
{
	printthree("xxx", "link.fwd", "link.bwd", "hdr",
		   crqp->crq_links.dbl_fwd,
		   crqp->crq_links.dbl_bwd, crqp->crq_hdr);
	printthree("xxx", "lock", "mode", "opt",
		   crqp->crq_slock, crqp->crq_mode, crqp->crq_opt);
	printthree("xxx", "unitid", "slave-vec", "master vec",
		   crqp->crq_unitid,
		   crqp->crq_slave_vect,
		   crqp->crq_master_vect);
	printthree("xxx", "cmd", "immedcmd", "rsp",
		   crqp->crq_cmd.dbl_fwd,
		   crqp->crq_immedcmd.dbl_bwd,
		   crqp->crq_rsp.dbl_fwd);
	printthree("xxx", "", "", "",
		   crqp->crq_cmd.dbl_bwd,
		   crqp->crq_immedcmd.dbl_bwd,
		   crqp->crq_rsp.dbl_bwd);
	printthree("xxd", "attn", "free", "totcmds",
		   crqp->crq_attn.dbl_fwd,
		   crqp->crq_free.dbl_fwd,
		   crqp->crq_totcmds);
	printthree("xx ", "", "", "",
		   crqp->crq_attn.dbl_bwd,
		   crqp->crq_free.dbl_bwd,
		   0);
	printthree("ddd", "totrsps", "totattns", "totvects",
		   crqp->crq_totrsps, crqp->crq_totattns, crqp->crq_totvects);
	printthree("x  ", "crq_stats@", "", "",
		  &crqp->crq_stats, 0, 0);
}

display_msg(msgp)
struct crq_msg *msgp;
{

}



display_ms(msp)
struct ms_struct *msp;
{
	printf("Size of ms_struct = %u (%#x)\n",
	       sizeof(struct ms_struct), sizeof(struct ms_struct));
	display_crq(&msp->ms_crq);
	display_msg(&msp->ms_attn_msg[0]);
	display_msg(&msp->ms_attn_msg[1]);
	printthree("xxx", "state", "class", "subclass",
		   msp->ms_state, msp->ms_class, msp->ms_subclass);
	printthree("xxx", "flags", "starttime", "dev",
		   msp->ms_flags, msp->ms_starttime, msp->ms_dev);
	printthree("xxx", "multidev", "retries", "devinfo@",
		   msp->ms_multidev, msp->ms_retries, &msp->ms_devinfo);
	printthree("xxx", "iostart", "totbytes[0]", "totbytes[1]",
		   msp->ms_iostart,
		   msp->ms_tot_bytes[0],
		   msp->ms_tot_bytes[1]);
	printthree("xxx", "ecc_hard", "ecc_soft", "errors",
		   msp->ms_ecc_hard, msp->ms_ecc_soft, msp->ms_errors);
	if (msp->ms_class == CLASS_DISK) {
		printthree("xxx", "blk_parts", "chr_parts", "layout",
			   msp->ms_un.msdisk.md_block_parts,
			   msp->ms_un.msdisk.md_char_parts,
			   msp->ms_un.msdisk.md_layout);
	} else {
		printthree("xxx", "currec", "eofrec", "eotcnt",
			   msp->ms_un.mstape.mt_currec,
			   msp->ms_un.mstape.mt_eofrec,
			   msp->ms_un.mstape.mt_eotcnt);
		printthree("x  ", "owner pid", "", "",
			   msp->ms_un.mstape.mt_owner_pid, 0, 0);
	}
	printf("openlock:\n");
	display_lock(&msp->ms_openlock);
}
