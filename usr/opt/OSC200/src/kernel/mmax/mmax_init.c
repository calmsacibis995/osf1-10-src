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
static char	*sccsid = "@(#)$RCSfile: mmax_init.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:01 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 *	File:	mmax/mmax_init.c
 *
 *	Basic initialization for MMAX.
 *
 */

#include <mach_nbc.h>
#include <xpr_debug.h>

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax_debug.h>
#include <mmax_etlb.h>
#include <mmax_kdb.h>

#include <sys/param.h>
#include <mmax/pcb.h>
#include <sys/kernel.h>
#include <sys/time.h>
#include <sys/systm.h>
#include <mmaxio/crqdefs.h>
#include <mmax/cpudefs.h>
#include <mmax/sccdefs.h>
#include <mmax/machparam.h>
#include <mmax/boot.h>
#include <mmax/cpu.h>

#include <mach/vm_param.h>
#include <mach/vm_prot.h>

#define PHYS(addr)	(addr)

BOOT	Boot;
vm_size_t	mem_size;
vm_offset_t	first_addr;
vm_offset_t	last_addr;

vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
vm_offset_t	vm_max_kernel_address;
unsigned int	Bufpercentage;

extern int	vm_object_cached_max;
extern int	kentry_count;
int		start_nmbclusters;
extern int	nmbclusters;		/* from param.c */
extern int	start_tseval;

extern	int	tcp_sendspace, tcp_recvspace;
extern	int	udp_sendspace, udp_recvspace;
extern	u_char	ipcksum;
extern	int	tcpcksum;
extern	int	udpcksum;
extern	int	ipforwarding;

int	Maxnetisrthreads;

#if	MACH_NBC
extern vm_size_t	mfs_map_size;
#endif

#if	XPR_DEBUG
extern int nxprbufs;
#endif

mmax_init(bootp)
BOOT	*bootp;
{
	extern	char	edata, end;
	int	cpuid;
	decl_simple_lock_data(extern,printf_lock)

	cpuid = getcpuid();
	blkclr(PHYS(&edata), &end - &edata);

	simple_lock_init(&printf_lock);
	master_cpu = cpuid;	/* Set the master cpu number.	*/
	Boot = *bootp;		/* Copy the boot data structure */

	mem_size = Boot.boot_physmem;

#if	MMAX_DPC && !MMAX_ETLB
	/*
	 *	Without ETLB, limit physical memory to 15.5 Meg
	 */

#define MEG	(1024*1024)
#define PHYS_LIMIT	(15*MEG + 512*1024)
	if (mem_size > PHYS_LIMIT) mem_size = PHYS_LIMIT;
#endif

	boothowto  = Boot.boot_flags;		/* Boot flags */
	bootdev = Boot.boot_rootdev;		/* Root device */

	start_tseval = Boot.boot_tseval;
	kentry_count = Boot.boot_kmap_entries;

	/*
	 * Get Timezone information.
	 *
	 * Note: We don't lock this structure, even with the
	 *	MMAX_MP code.  The reason is that a) the
	 *	Time locks aren't initialized yet and b) this
	 *	routine is called so early in the initialization
	 *	sequence that no-one is touching this structure.
	 */

	tz = Boot.boot_timezone;

	/*
	 * Get network configuration parameters.
	 */
	/* let's not and say we did -- gmf */

#if	0	/* TMT */
	Maxnetisrthreads = Boot.boot_nthreads;
	ipcksum  = Boot.boot_ipchksum;
	tcpcksum = Boot.boot_tcpchksum;
	udpcksum = Boot.boot_udpchksum;
	ipforwarding = Boot.boot_ipforwarding;

	tcp_sendspace = Boot.boot_tcpsendspace;
	tcp_recvspace = Boot.boot_tcprecvspace;

	udp_sendspace = Boot.boot_udpsendspace;
	udp_recvspace = Boot.boot_udprecvspace;
#endif
	start_nmbclusters = Boot.boot_nmbclusters;
	nmbclusters = start_nmbclusters;
#if	0	/* TMT */
	if (Maxnetisrthreads < 1) {
		printf("WARNING:  overrode maxnetisrthreads:\n");
		printf("\told value %d, new value %d\n",
		       Maxnetisrthreads, 1);
		Maxnetisrthreads = 1;
	} else if (Maxnetisrthreads > 20) {
		printf("WARNING:  maxnetisrthreads is large (%d)\n",
		       Maxnetisrthreads);
	}
#if	MMAX_DEBUG
	if(tcp_sendspace < 1024) {
		printf("Tcp_sendspace = %u\n", tcp_sendspace);
		tcp_sendspace = 2048;
	}
	if(tcp_recvspace < 1024) {
		printf("Tcp_recvspace = %u\n", tcp_recvspace);
		tcp_recvspace = 2048;
	}
	if(udp_sendspace < 1024) {
		printf("udp_sendspace = %u\n", udp_sendspace);
		udp_sendspace = 2048;
	}
	if(udp_recvspace < 1024) {
		printf("udp_recvspace = %u\n", udp_recvspace);
		udp_recvspace = 2048;
	}
#endif
#endif

	/*
	 * Get file system configuration parameters.
	 */

	Bufpercentage = Boot.boot_bufpercent;
#if	MMAX_DEBUG
	if(Bufpercentage > 100 || Bufpercentage < 2) {
		printf("bufpercentage = %u\n", Bufpercentage);
		Bufpercentage = 10;
	}
#endif

	/*
	 * Set maximum kernel virtual address space based on boot parameter.
	 */
	
	vm_max_kernel_address = VM_MAX_KERNEL_SPACE;
	if (Boot.boot_kernvaddr < vm_max_kernel_address) {
		if (Boot.boot_kernvaddr < VM_MIN_KERNEL_SPACE)
			vm_max_kernel_address = VM_MIN_KERNEL_SPACE;
		else
			vm_max_kernel_address = Boot.boot_kernvaddr;
	}

	virtual_end = vm_max_kernel_address;

	startCPU(cpuid);		/* take care of various inits */

	/*
	 *	Set our machine-independent page size to be 8k.
	 */

	page_size = 8192;
	vm_set_page_size();

	/*
	 *	Bump object cache size to 500 entries (due to having
	 *	lots of memory on most Multimaxen).
	 */
	vm_object_cached_max = 500;

#if	MACH_NBC
	/*
	 *	Set size of mfs_map to 4 Meg
	 */

	mfs_map_size = 4*1024*1024;
#endif

	/*
	 *	Initialize the interrupt table here so pmap can allocate
	 *	a vector for tlb shootdown.
	 */
	itbl_init();

#if	XPR_DEBUG
/*	nxprbufs = 32*1024; */
#endif
	/*
	 *	Set up the physical memory maps.  Note that the values of
	 *	avail_start and avail_end are changed by pmap_bootstrap.
	 *	The return to locore turns on memory management and the
	 *	Extended Translation Buffer.
	 */
	first_addr  = round_page(PHYS(&end));
	last_addr   = trunc_page(mem_size);
	avail_start = first_addr;
	avail_end   = last_addr;
	pmap_bootstrap(&avail_start, &avail_end, &virtual_avail,
		&virtual_end);
}

print_reg(type, code, sp, r7, r6, r5, r4, r3, r2, r1, r0, fp, pc, psl)
{

	printf("print_reg: type = %d code = %x sp = %x\n", type, code, sp);
	printf("r7= %x r6= %x r5= %x r4= %x r3= %x r2= %x r1= %x r0= %x\n",
		r7, r6, r5, r4, r3, r2, r1, r0);
	printf("fp= %x pc= %x psl = %x\n", fp, pc, psl);
}
