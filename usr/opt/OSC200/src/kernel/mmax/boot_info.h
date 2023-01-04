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
 *	@(#)$RCSfile: boot_info.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:45 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *        Copyright 1985, 1986 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Include file description:
 *	Data information corresponding to boot.h, for sysboot and sysparam
 *
 * Original Author: Sharon Krause	Created on: May 5, 1986
 */



/* definitions for the boot_info.h structure */
/*
#define MINDYNSIZE	524288

#define BI_CHAR		1001
#define BI_DEV		1002
#define BI_HDR		1003
#define BI_INFO		1004
#define BI_INT		1005
#define BI_INV		1006
#define BI_HEX		1007

#define AUTO_SIZE	-5000

#define SV_NONE		0

#define SPECIAL_CASE	-6000
#define CALCULATED_VAL	-6000

#define SV_MAXCPUS	-6001
#define SV_MAXUSERS	-6002
#define SV_MAX_NBUFFERS	-6003
#define SV_WSSRATE	-6004
#define SV_MAX_NFSDESC	-6005
#define SV_MAX_NFILE	-6006
#define SV_INIT_NINODE	-6007
#define SV_MAX_NINODE	-6008
#define SV_MAX_NCYLGR	-6009
#define SV_BUF_HASHFSZ	-6010
#define SV_INODE_HASHFSZ	-6011
#define SV_CG_HASHFSZ	-6012
#define SV_INIT_NFILE	-6013
#define SV_NUM_CBLKS	-6014
#define SV_NUM_MSGS	-6015
#define SV_INIT_NBUFFERS	-6016
#define SV_DYNSIZE	-6017
#define SV_CBLK_RNDUP	-6018
#define SV_DAYLIGHT	-6019
#define SV_DEVICE	-6020
#define SV_MINUTES	-6021
*/


/* REALLY START HERE */

#ifndef	STANDALONE

/* this number MUST match the actual size of the following table */

#ifdef	ENCORE
#define BOOT_ENTRIES 91
#else
#ifdef MACH
#define BOOT_ENTRIES 91
#else
#define BOOT_ENTRIES 86
#endif
#endif

	 int	boot_entries = BOOT_ENTRIES;

	 BOOT_INFO boot_info[ BOOT_ENTRIES ] = {

/* General system definitons */

{ "GENERAL SYSTEM DEFINITIONS",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Auto_set",
   BI_INV,
   0,
   0,
   0,
   12,
   SV_NONE},

{ "Timezone -- inside USA: AST, EST, CST, MST, PST, HST, or YST",
  "Info_line",
   BI_INFO,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "  	 -- outside USA: number of minutes west of Greenwich",
  "Info_line",
   BI_INFO,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ " ",
  "Timezone",
   BI_INT,
   -1200,
   1200,
   300,
   AUTO_SIZE,
   SV_MINUTES},

{ "Use of Daylight savings time during year",
  "Day_sav_t",
   BI_INT,
   0,
   1,
   1,
   AUTO_SIZE,
   SV_DAYLIGHT},

{ "Parameters filled in by sysboot -- not settable",
  "Not_set",
   BI_INV,
   0,
   0,
   0,
   588,
   SV_NONE},

/* various devices */

{ "DEVICE DEFINITONS",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Devices are entered in the lamp slot dev lun partition format",
  "Info_line",
   BI_INFO,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "",
  "Info_line",
   BI_INFO,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Root device",
  "Root_dev",
   BI_DEV,
   -1,
   -1,
   -1,
   AUTO_SIZE,
   SV_DEVICE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used1",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Console_dev",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Dump device",
  "Dump_dev",
   BI_DEV,
   -1,
   -1,
   -1,
   AUTO_SIZE,
   SV_DEVICE},

/* Processor management */

{ "PROCESSOR INFORMATION",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

#ifdef	MACH
{ "",
  "Info_line",
   BI_INFO,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},
#endif

{ "Maximum number of CPUs configured",
  "Max_cpus",
   BI_INT,
   2,
   20,
   10,
   AUTO_SIZE,
   SV_MAXCPUS,},

#ifdef	MACH
{ "THREAD MANAGEMENT",
  "Header",
  BI_HDR,
  0,
  0,
  0,
  AUTO_SIZE,
  SV_NONE},
#else	MACH
/* Process management */

{ "PROCESS MANAGEMENT",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

#endif

{ "Time-Slice End Counter Value, .9216 Mhz ticks",
  "Tseval",
   BI_INT,
   10000,
   99999,
   92160,
   AUTO_SIZE,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used3",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used4",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used5",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

/* Virtual memory */

{ "VIRTUAL MEMORY",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Physmem",
   BI_INV,
   0,
   0,
   0,
   4,
   SV_NONE},

{ "Maximum Kernel Virtual Address space (maximum is ffffffff)",
  "Max_kernvaddr",
   BI_HEX,
   0x00fc0000,
   0xffffffff,
   0x0fffe000,
   AUTO_SIZE,
   SV_NONE},

{ "Kernel Map Entries",
  "Kmap_entries",
   BI_INT,
   1000,
   10000,
   2000,
   AUTO_SIZE,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used9",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

/* Network subsystem */

{ "NETWORK SUBSYSTEM",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Number of network input threads?",
  "Nthreads",
   BI_INT,
   1,
   20,
   1,
   AUTO_SIZE,
   SV_NONE},

{ "Check incoming TCP Checksums?",
  "Tcpchksum",
   BI_CHAR,
   0,
   1,
   1,
   AUTO_SIZE,
   SV_NONE},

{ "Check incoming UDP Checksums?",
  "Udpchksum",
   BI_CHAR,
   0,
   1,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Check incoming IP Checksums?",
  "Ipchksum",
   BI_CHAR,
   0,
   1,
   1,
   AUTO_SIZE,
   SV_NONE},

{ "Maximum send space (in bytes) for TCP Socket",
  "Tcp_sendspace",
   BI_INT,
   2048,
   8192,
   4096,
   AUTO_SIZE,
   SV_NONE},

{ "Maximum receive space (in bytes) for TCP Socket",
  "Tcp_recvspace",
   BI_INT,
   2048,
   8192,
   4096,
   AUTO_SIZE,
   SV_NONE},

{ "Maximum send space (in bytes) for UDP Socket",
  "Udp_sendspace",
   BI_INT,
   2048,
   65536,
   2048,
   AUTO_SIZE,
   SV_NONE},

{ "Maximum send space (in bytes) for UDP Socket",
  "Udp_recvspace",
   BI_INT,
   2048,
   65536,
   2048,
   AUTO_SIZE,
   SV_NONE},

{ "Enable IP packet forwarding?",
  "ip_forwarding",
   BI_CHAR,
   0,
   1,
   0,
   AUTO_SIZE,
   SV_NONE},

#ifdef	MACH
{ "Parameters filled in by sysboot -- not settable",
  "Not_used18a",
   BI_INV,
   -1,
   -1,
   -1,
   1,
   SV_NONE},
{ "Parameters filled in by sysboot -- not settable",
  "Not_used18b",
   BI_INV,
   -1,
   -1,
   -1,
   1,
   SV_NONE},
{ "Parameters filled in by sysboot -- not settable",
  "Not_used18c",
   BI_INV,
   -1,
   -1,
   -1,
   1,
   SV_NONE},
#endif	MACH

{ "Number of mbuf clusters",
  "Nmbclusters",
   BI_INT,
   512,
   8192,
   1024,
   AUTO_SIZE,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used20",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used21",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

/* File system */

{ "FILE SYSTEM",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used22",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used23",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used24",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Percentage of memory to be used for the buffer cache",
  "Buffer space Percentage",
   BI_INT,
   2,
   70,
   10,
   AUTO_SIZE,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used26",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used27",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used28",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used29",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used30",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used31",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used32",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used33",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used34",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

#ifndef	MACH
/* Allocator definitions */

{ "ALLOCATOR DEFINITIONS",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

#endif

{ "Parameters filled in by sysboot -- not settable",
  "Not_used35",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

#ifndef	MACH
/* Terminal subsystem */

{ "TERMINAL SUBSYSTEM",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

#endif

{ "Parameters filled in by sysboot -- not settable",
  "Not_used36",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used37",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used38",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used39",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used40",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used41",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used42",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

/* Masstore subsystem */

#ifdef notdef
{ "MASSTORE SUBSYSTEM",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

{ "Number of CRQ Command Requestor Blocks (80 per physical device)",
  "Cmdreqblks",
   BI_INT,
   80,
   1600,
   240,
   AUTO_SIZE,
   SV_NONE},
#else

{ "Parameters filled in by sysboot -- not settable",
  "Header",
   BI_INV,
   -1,
   -1,
   -1,
   1,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Cmdreqblks",
   BI_INV,
   -1,
   -1,
   -1,
   1,
   SV_NONE},
#endif

{ "Parameters filled in by sysboot -- not settable",
  "Not_used44",
   BI_INV,
   -1,
   -1,
   -1,
   1,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used45",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

#ifndef	MACH

/* Profiling */

{ "PROFILING SUBSYSTEM",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   4,
   SV_NONE},

#endif

{ "Parameters filled in by sysboot -- not settable",
  "Not_used46",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used47",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used48",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

#ifndef	MACH
/* Miscellaneous */

{ "MISCELLANEOUS",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

#endif

{ "Parameters filled in by sysboot -- not settable",
  "Not_used49",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used50",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

#ifndef	MACH
/* System V IPC subsystem */

{ "System V IPC SUBSYSTEM",
  "Header",
   BI_HDR,
   0,
   0,
   0,
   AUTO_SIZE,
   SV_NONE},

#endif

{ "Parameters filled in by sysboot -- not settable",
  "Not_used51",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used52",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used53",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used54",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used55",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used56",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used57",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used58",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used59",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used60",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used61",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used62",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used63",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used64",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used65",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used66",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE},

{ "Parameters filled in by sysboot -- not settable",
  "Not_used67",
   BI_INV,
   -1,
   -1,
   -1,
   4,
   SV_NONE}

};

#endif	STANDALONE
