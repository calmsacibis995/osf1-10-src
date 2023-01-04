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
 *	@(#)$RCSfile: mop.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:38 $
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
#ifndef MOP_H
#define MOP_H
/*
 * Network MOP support
 */
#define REQ_PROG_CODE 8
#define REQ_LOAD_CODE 10
#define REQ_DUMP_SERVICE 12
#define REQ_MEMORY_DUMP 4

#define NET_QNA 5

#define MOP_VERSION 1

#define PGMTYP_SECONDARY 0
#define PGMTYP_TERTIARY 1
#define PGMTYP_OPSYS 2

#define SYSTEMPROC 0

#define XTRA_BUFSZ 0x191
#define RCV_BUF_SZ 1400
			
#define MEMLD_XFR_CODE 0
#define MEMLD_CODE 2
#define VOLASS_CODE 3
#define MEMORY_DUMP_DATA 14
#define PARAM_CODE 20
#define NETLOAD_REQUEST 255		/* local definition */

#ifdef mips
#define NETBLK_LDADDR	0x8001fc00	/* 1k under the kernel */
#endif /* mips */

#define ENDMRK	0
#define TRGNAME	1
#define TRGADDR	2
#define HSTNAME	3
#define HSTADDR	4
#define HSTTIME	5

#ifndef LOCORE
struct netblk 	{
    char	srvname[32];		/* server hostname (boot server)*/
    unsigned long srvipadr;		/* server IP address (boot server)*/
    char	cliname[32];		/* client hostname	*/
    unsigned long cliipadr;		/* client IP address	*/
    unsigned long brdcst;		/* broadcast address	*/
    unsigned long netmsk;		/* network mask address */
    short 	swapfs;			/* swap file system type*/
    short	rootfs;			/* root file system type*/
    short 	swapsz;			/* swap size in 1/2 Meg units */
    short	dmpflg;			/* dump flag 0 - disabled */
				        /*           1 - enabled  */
    char	rootdesc[80];		/* root filesys descriptor */
    char	swapdesc[80];		/* swap file descriptor	*/
    char	reserved[20];   	/* for later use	*/
};

/*
 * MOP structures
 */

union mop_packets {
	struct {
		unsigned char	code;
		unsigned char	devtype;
		unsigned char	mopver;
		unsigned char	pgmtyp;
		unsigned char	swid_form;
		unsigned char	proc;
		unsigned short	rbufsz_param;
		unsigned char	sz_field;
		unsigned char	rcvbufsz[2];
	} req_pgm;

	struct {
		unsigned char code;
		unsigned char loadnum;
		unsigned char error;
	} req_memload;

	struct {
		unsigned char code;
		unsigned char loadnum;
		unsigned char loadaddr[4];
		unsigned char data[RCV_BUF_SZ];
	} memload;

	struct {
		unsigned char code;
		unsigned char	loadnum;
		unsigned char	loadaddr[4];
		union {
			unsigned char	data[RCV_BUF_SZ];
			unsigned char	xfr_addr[4];
		} type;
	} memload_xfr;

	struct {
		unsigned char	code;
		unsigned char	devtype;
		unsigned char	mopver;
		unsigned char	memsiz[4];
		unsigned char	bits;
		unsigned char	rbufsz_param[2];
		unsigned char	sz_field;
		unsigned char	rcvbufsz[2];
	} req_dump_srvc;

	struct {
		unsigned char code;
		unsigned char	memaddr[4];
		unsigned char	data[RCV_BUF_SZ];
	} dump_data;

	struct {
		unsigned char code;
		unsigned char	memaddr[4];
		unsigned char	count[2];
	} req_mem_dump;
};

#define MOPCODE req_pgm.code	/* the offset it the same for all packets */
#endif /* LOCORE */
#endif /* MOP_H */
