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
static char	*sccsid = "@(#)$RCSfile: display_xpr.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/08/15 07:46:35 $";
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
#include <mach/boolean.h>
#include <kern/xpr.h>
#include <mach/machine/vm_types.h>


static int xpr_inited = FALSE;
int     nxprbufs;
unsigned long xprflags;
struct xprbuf *xprbase;		/* Pointer to circular buffer
				 * nxprbufs*sizeof(xprbuf) */
struct xprbuf *xprptr;		/* Currently allocated xprbuf */
struct xprbuf *xprlast;		/* Pointer to end of circular buffer */

char   *xpr_bit_names[] = {
			   "Syscalls",
			   "Traps",
			   "Sched",
			   "Nptcp",
			   "Np",
			   "Tcp",
			   "Fs",
			   "VM-Object",
			   "VM-Object-Cache",
			   "VM-Page",
			   "VM-Pageout",
			   "Memory-Object",
			   "VM-Fault",
			   "Inode-Pager",
			   "Inode-Pager-Data"
};

#define	XPRBITS		(sizeof(xpr_bit_names)/(sizeof(char *)))


display_xprbuf_brief(vxp)
struct xprbuf *vxp;
{
	int     result;
	char   *msg;
	struct xprbuf *pxp;

	result = phys(vxp, &pxp, ptb0);
	if (result != SUCCESS) {
		printf("mda: Could not translate xpr buf adr %#x\n", vxp);
		return (FAILED);
	}
	pxp = (struct xprbuf *) MAPPED(pxp);

	msg = pxp->msg;
	result = phys(msg, &msg, ptb0);
	if (result != SUCCESS) {
		printf("mda: Could not translate xpr buf msg %#x\n", msg);
		return (FAILED);
	}
	msg = (char *) MAPPED(msg);
	printf("%3.3d: %10.10u ", pxp->cpuinfo, pxp->timestamp);
	printf(msg, pxp->arg1, pxp->arg2, pxp->arg3, pxp->arg4, pxp->arg5);
	if (((int) msg < (int) 0x4000) || (*msg == (char) 0))
		printf("\n");
}



display_xpr_buffer()
{
	int     result, i;
	struct xprbuf *xp;

	if (!xpr_inited) {
		xpr_inited = TRUE;
		if(get_address("_xprbase", &xprbase) != SUCCESS) {
			printf("mda: Could not get address of xprbase\n");
			return (FAILED);
		}
		if(get_address("_xprlast", &xprlast) != SUCCESS) {
			printf("mda: Could not get address of xprlast\n");
			return (FAILED);
		}
		if(get_address("_xprptr", &xprptr) != SUCCESS) {
			printf("mda: Could not get address of xprptr\n");
			return (FAILED);
		}
		if(get_address("_nxprbufs", &nxprbufs) != SUCCESS) {
			printf("mda: Could not get address of nxprbufs\n");
			return (FAILED);
		}
		PHYS(nxprbufs, nxprbufs);
		nxprbufs = (int) MAP(nxprbufs);

		result = get_address("_xprflags", &xprflags);
		if (result != SUCCESS) {
			printf("mda: Could not get address of xprflags\n");
			return (FAILED);
		}
		PHYS(xprflags, xprflags);
		xprflags = (int) MAP(xprflags);
	}
	if (nxprbufs == 0) {
		printf("mda: XPR facility not enabled\n");
		return (SUCCESS);
	}
	printf("XPR Flags enabled: ");
	diagram(xprflags, xpr_bit_names, XPRBITS);
	printf("\nCPU   Timestamp Message\n");

	xp = (struct xprbuf *) MAP(xprptr);
	for (i = 0; i < nxprbufs; i++) {
		--xp;
		display_xprbuf_brief(xp);
		if (xp == (struct xprbuf *) MAP(xprbase))
			xp = (struct xprbuf *) MAP(xprlast);
	}
}
