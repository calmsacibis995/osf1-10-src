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
 *	@(#)$RCSfile: boot.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:42 $
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
 *     Copyright (C) 1984 Encore Computer Inc.
 *
 * Include file description:
 *	This include file contains the definition of the system boot parameters
 */

#include <sys/param.h>
#include <sys/time.h>

#include <mmaxio/crqdefs.h>
#include <mmax/sccdefs.h>


/* Panic Save area, where registers go on a crash:
 *        usp, r0-r7, sp, fp, pc, psl/mod
 */
#define PSA_USP 0
#define PSA_R7 4
#define PSA_R6 8
#define PSA_R5 12
#define PSA_R4 16
#define PSA_R3 20
#define PSA_R2 24
#define PSA_R1 28
#define PSA_R0 32
#define PSA_FP 36
#define PSA_PC 40
#define PSA_PSR 44
#define PSA_SSP 48
#define PSA_ICU_ISRV 52
#define PSA_ICU_IPND 54
#define PSA_SIZE 56

typedef union	boot {
/*
 * The buffer is an attempt to define a maximum size of the boot structure so
 * that sysboot will never have to be rebuilt just because the boot structure
 * changed.
 */
	char		boot_max[1024];
/*
 * This is the real boot structure.
 */
struct {
	/*
	 * General system definitions.
	 */
	unsigned int	b_unitid;	/* Boot device-unit id */
	struct timeval	b_time;		/* Boot time */
	struct	timezone b_timezone;	/* Boot time zone */
	unsigned int	b_flags;	/* Boot flags */
	unsigned int	b_cpuid;	/* Boot processor id */
	unsigned int	b_slotid[NUM_SLOT]; /* Per slot identifiers */
	char	*b_os_base;	/* OS base address */
	char	b_image[PATH_LEN]; /* Boot image file name */
	char	b_param[PATH_LEN]; /* Boot parameter file name */

/*
 * Various devices
 */
	umdev_t	b_rootdev;	/* Root device */
	umdev_t	not_used1;	/* Paging device */
	umdev_t	b_console_dev;	/* Console device */
	umdev_t	b_dumpdev;	/* Dump device */

/*
 * Processor management.
 */
	int	b_maxcpus;	/* Number of cpus to start */

/*
 * Process management.
 */
	int	b_tseval;	/* time-slice end counter value */
	int	not_used3;	/* Maximum number of users */
	int	not_used4;	/* Maximum user process */
	int	not_used5;	/* Physical time slice in 600usec ticks */

/*
 * Virtual memory management.
 */
	int	b_physmem;	/* Number of physical pages present */
	int	b_kernvaddr;	/* Maximum kernel virtual address space */
	int	b_kmap_entries;	/* Number of kernel map entries */
	int	not_used9;	/* Maximum number of wired pages. */
/*
 * Network subsystem
 */
	int	b_nthreads;	/* Number of receive threads	*/
	char	b_tcpchksum;	/* TCP checksum flag */
	char	b_udpchksum;	/* UDP checksum flag */
	char	b_ipchksum;	/* IP  checksum flag */
	int	b_tcpsendspace;	/* TCP send space (bytes) */
	int	b_tcprecvspace;	/* TCP recv space (bytes) */
	int	b_udpsendspace;	/* UDP send space (bytes) */
	int	b_udprecvspace;	/* UDP recv space (bytes) */
	char	b_ipforwarding;	/* IP Forwarding flag	  */
	char	not_used18a;	/* Not used (was an int)  */
	char	not_used18b;	/* Not used (was an int)  */
	char	not_used18c;	/* Not used (was an int)  */

	int	b_nmbclusters;	/* Number of mbuf clusters */
	int	not_used20;	/* Small receive cmd pkts */
	int	not_used21;	/* Send command packets */

/*
 * File system
 */
	int	not_used22;	/* Initial number of FSDs */
	int	not_used23;	/* Maximum number of FSDs */
	int	not_used24;	/* Initial # of buffers */
	int	b_buf_percent;	/* Amount of buffer space   */
	int	not_used26;	/* Buf cache hash frame size*/
	int	not_used27;	/* Initial # of file struct */
	int	not_used28;	/* Maximum # of file struct */
	int	not_used29;	/* Initial # of inode struct*/
	int	not_used30;	/* Maximum # of inode struct*/
	int	not_used31;	/* Inode cache hash frm size*/
	int	not_used32;	/* Initial # of cylgr struct*/
	int	not_used33;	/* Maximum # of cylgr struct*/
	int	not_used34;	/* Cyl group hash frm size */

/*
 * Allocator definitions.
 */
	int	not_used35;	/* Size of pool in bytes */

/*
 * Terminal subsystem
 */
	int	not_used36;	/* Size of scc rcv buffers */
	int	not_used37;	/* No. receive msgs per line */
	int	not_used38;	/* Input wait time (msec) */
	int	not_used39;	/* # cblocks in pool	*/
	int	not_used40;	/* Size of cblks (bytes) */
	int	not_used41;	/* cblock address mask	*/
	int	not_used42;	/* No. pseudo terminals */

/*
 * Mass storage subsystem
 */
	int	b_cmdreqblks;	/* No. CRQ command request blocks */
	char	not_used44;	/* Seek optimization flag */
	int	not_used45;	/* No. msg pkts per unit */

/*
 * Profiling
 */
	int	not_used46;	/* Number of functions */
	int	not_used47;	/* Rate of pc profiling in msecs */
	int	not_used48;	/* Fraction of text for PC profiling */
/*
 * Miscellaneous tuning parameters.
 */
	int	not_used49;	/* Break-even point for MOVSU/MOVUS vs. */
				/*	 mapped copyin/copyout */
/* 
 * Quota system flag
 */
	int	not_used50;	/* Are you ever intending to turn quotas on?*/
/*
 * System V IPC parameters
 */
	int	not_used51;	/* Number of semaphore allocations	*/
	int	not_used52;	/* Number of semaphore IDs		*/
	int	not_used53;	/* Number of semaphores in system	*/
	int	not_used54;	/* Number of undo structures		*/
	int	not_used55;	/* Max # undos per process		*/

	int	not_used56;	/* # entries in msg map			*/
	int	not_used57;	/* maximum message size			*/
	int	not_used58;	/* maximum # bytes on queue		*/
	int	not_used59;	/* # of msg queue identifiers		*/
	int	not_used60;	/* msg segment size, sizeof(int) multiple */
	int	not_used61;	/* # of message headers			*/
	int	not_used62;	/* # of msg segments (MUST BE < 32768 )	*/

	int	not_used63;	/* max shared memory size		*/
	int	not_used64;	/* min  "     "       "			*/
	int	not_used65;	/* # shared memory ids in system	*/
	int	not_used66;	/* # shared memories per process	*/
	int	not_used67;	/* max # clicks of shared memory	*/

	} boot_used;
} BOOT;

#define boot_unitid		boot_used.b_unitid
#define boot_time		boot_used.b_time
#define boot_timezone		boot_used.b_timezone
#define boot_flags		boot_used.b_flags
#define boot_cpuid		boot_used.b_cpuid
#define boot_slotid		boot_used.b_slotid
#define boot_os_base		boot_used.b_os_base
#define boot_image		boot_used.b_image
#define boot_param		boot_used.b_param
#define boot_rootdev		boot_used.b_rootdev
#define boot_console_dev	boot_used.b_console_dev
#define boot_dumpdev		boot_used.b_dumpdev
#define boot_maxcpus		boot_used.b_maxcpus
#define boot_physmem		boot_used.b_physmem
#define boot_kernvaddr		boot_used.b_kernvaddr
#define boot_kmap_entries	boot_used.b_kmap_entries
#define boot_tseval		boot_used.b_tseval
#define boot_nthreads		boot_used.b_nthreads
#define boot_ipchksum		boot_used.b_ipchksum
#define boot_tcpchksum		boot_used.b_tcpchksum
#define boot_udpchksum		boot_used.b_udpchksum
#define boot_tcpsendspace	boot_used.b_tcpsendspace
#define boot_tcprecvspace	boot_used.b_tcprecvspace
#define boot_udpsendspace	boot_used.b_udpsendspace
#define boot_udprecvspace	boot_used.b_udprecvspace
#define boot_ipforwarding	boot_used.b_ipforwarding
#define boot_nmbclusters	boot_used.b_nmbclusters
#define boot_bufpercent		boot_used.b_buf_percent
#define boot_max_buffers	boot_used.b_max_buffers
#define boot_cmdreqblks		boot_used.b_cmdreqblks

/* 
 * This file describes the id fields contained in the id proms located on
 * each board in the MULTIMAX system.
 */

#define ID_PROM_TYPE		1
#define REV_PROM_TYPE		2
#define ETHERNET_PROM_TYPE	3

#define UNDEF_BD	0
#define EMC		1
#define DPC		2
#define SCC		3
#define DLA		4
#define EMCII		5
#define DPCII		6
#define SMC		7
#define SMCII		8
#define SCCII		9
#define APC		10
#define DLA_16_ASYNC	11
#define DLA_8_SYNC	12
#define ANNEX_UX_ASYNC	13			/* Unix Only Annex	    */
#define MSC		14
#define SMC_16		15
#define ANNEXII_MX_ASYNC	16	/* UNIX only Annex II */
#define ANNEXII_UX_ASYNC	17
#define ANNEXII_SYNC		18
#define TIU_UX_ASYNC		19	/* Proteon TIU */
#define MSC_TRACE		20	/* Special for DPC address trace */
#define UIC			21
#define XPC			22
#define ANNEX_X25		23	/* X.25 capable Annex II */
#define EMCDIF			24	/* Differential EMC */
#define UNKNOWN_BD		25	/* Should be 1 more than last one.  */
