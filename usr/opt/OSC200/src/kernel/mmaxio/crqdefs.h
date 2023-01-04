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
 *	@(#)$RCSfile: crqdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:05 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

#ifndef	_CRQDEFS_
#define _CRQDEFS_

#ifdef	KERNEL
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax/inline_lock.h>
#endif

#include <kern/lock.h>
#include <kern/queue.h>
#include <sys/types.h>

#include <kern/assert.h>

/*
 *	The following two types are used internally in crq definitions
 *	and must be defined here.
 */

/*
 *	dbl_link_t is the standard type for doubly linked queues in crqs.
 */

typedef struct dbl_link {
	struct dbl_link *dbl_fwd;
	struct dbl_link *dbl_bwd;
} dbl_link_t;

/*
 * The system header data structure is defined in UMAX V only for
 * compatibility with UMAX 4.2. It is not really used as anything
 * but a placeholder.
 */

typedef struct syshdr {
	unsigned int	hdr_size:13;	/* Structure size in bytes */
	                                /* Note: differs from Umax */
	unsigned int    hdr_bdl_alloc:1;/* ptr to bdl was stored   */
	unsigned int	hdr_type:11;	/* Structure type */
	unsigned int	hdr_version:6;	/* Structure version */
	unsigned int	hdr_alloc:1;	/* Allocator flag */
} syshdr_t;

/*1
 * crq.h
 *
 *	This file defines a crq structure, the basic communication block used
 *	for master/slave interactions in the Umax system.  CRQs are used to
 *	communicate between requester boards on system start-up and are the
 *	basic data structure in the I/O system.  Also defined are the CRQ
 *	statistics extension block and the message header structure that
 *	defines the standard header for all messages queued to crqs, and
 *	the set of standard messages.
/**/

/*
 *   bd_t	- (DMA) Buffer Descriptor Type 
 *
 *	Buffer descriptors are a hardware supported data structure
 *	for accomplishing dma type i/o back and forth to a MULTIMAX
 *	EMC board.  They point to 'physical addresses' in memory
 *	and may either be chained together or 'arrayed' together
 *	to allow scatter/gather (physical memory) to (or from)
 *	contiguous disk blocks or EMC memory.  They are also used
 *	as a communication structure to low level disk/tape drivers
 *	for nonmultimax systems.
 *
 *	HARDWARE DOCUMENTATION:	The following description is a guess
 *			based on what seems to work.  The hardware manual
 *			is ambiguous and wrong and can not be trusted.
 *
 *	IMPORTANT:	A bd_t must be double longword (64 bit)
 *			aligned (e.g. .align 8).
 *
 *	HARDWARE BUGS:  The following hardware bugs are present:
 *
 *		Chaining:  Chaining does not work.  The Chain bit (bd_chain)
 *			must be zero.  This corresponds to the way we are
 *			using them anyway.
 *
 *		Address First:  Contrary to EMC hardware manual, the bd_addr
 *			field preceeds the long word of flags and sizes.
 *
 *	FIELDS in the Buffer Descriptors are interpreted as follows:
 *
 *	bd_valid (1 bit) - Valid Bit, a 1 indicates the contents for
 *			the pointed at buffer are valid.  In essence,
 *			a 0 implies the end of an array or list of bd_t's.
 *
 *	bd_chain (1 bit) - Chain Bit, a 0 indicates that the address
 *			(bd_addr) is a data buffer, whereas a 1 indicates
 *			the address is a chain link to another buffer
 *			descriptor.  (not supported yet as of EMC II)
 *
 *	bd_fff (3 bits) - Offset to the first valid byte in the first 64
 *			bit aligned chunk of the buffer at bd_addr.
 *
 *	bd_lll (3 bits) - Offset to the last valid byte in the last 64 
 *			bit chunk in the 64 bit aligned buffer at bd_addr.
 *
 *	bd_size (24 bits) - The number of bytes spanned by the quadwords
 *			including the fff and lll bytes, less 4.  Thus,
 *			the bottom three bits are always '100'.
 *
 *	bd_addr (32 bits) - The physical address of the buffer 
 *			(when bd_chain == 0) or the physical address of the
 *			next bd_t in a chain (when bd_chain == 1).  This
 *			value should be quadword aligned (low 3 bits zero).
 *
 *	In normal useage, a buffer will be 64 bit aligned (.align 8)
 *	and data will start at the beginning of the buffer.  For a 1k
 *	buffer, this would result in:
 *		
 *		bd_fff = 0;  bd_lll = 7; bd_size = 1020;
 *
 *	For cases of doing nonaligned i/o of a weird size, things get
 *	really tough.  
 *
 */

typedef struct bd {
	u_long	bd_addr;
	union {
		u_long bits;
		struct {
			int	bd_size: 24;	/* bottom 2 bits zero */
			u_int	bd_lll:   3;
			u_int	bd_fff:   3;
			u_int	bd_chain: 1;	/* Must be Zero */
			u_int	bd_valid: 1;
		} bm;
	} bun;
} bd_t;

/*
 *  bd_t MACROS
 *
 *	BDL's (bd_t's) are never directly referenced in code.  Instead
 *	use the following macros for future portability, hardware changes,
 *	etc.	The following macros support setting up buffer descriptors.
 *	
 *	BD_TAIL takes a pointer to a bd_t and makes it a valid
 *	end of the chain or array of bd_t's.
 *
 *	BD_NONALIGN sets up a single buffer descriptor to point at
 *	'bsize' (physically) contiguous bytes of memory that starts
 *	at (physical) address 'baddr'.
 *
 *	BD_ALIGN is a shorter version of BD_NONALIGN which assumes
 *	that the physical address 'baddr' is aligned on a 64 bit bdry.
 *
 *	BD_SIMPLE is even shorter, and makes the additional assumption
 *	that the length of the buffer is a multiple of 8 bytes.
 *
 *	The following macros extract info from buffer descriptors
 *
 *	BD_ADDR returns the actual (physical) start of i/o.
 *
 *	BD_SIZE returns the actual byte size of the i/o transfer.
 *
 *	BD_NEXT updates bd_p to point to the next data containing
 *	bd_t in the current array/chain.  For now, lets assume no
 *	chained bdl's (just arrays).  If its chained, we must use
 *	interpret the pointer as a physical address, which may not
 *	always ==virtual.  If we know phys==virt, then we could 
 *	add the line:
 *
 *	if ((bd_p)->chain) {(bd_t *) (bd_p) = (bd_t *) (bd_p)->bd_addr;} 
 *
 *	BDL_MAX_BYTES is the maximum number of bytes in a single bd_t
 *	entry.  Although bd_t's can support 24 bit sizes, particular
 *	disk controllers may support smaller transfers in a single i/o.
 *	Although the EMC or a particular driver could do several 
 *	operations to perform a given bd_t's worth of I/O, we do
 *	not guarantee the transfer.  BD_MAX_BYTES is thus the largest
 *	guaranteed transfer.
 */

#define BDL_MAX_BYTES	0x800000

#define BD_SZ_MSK 0x00fffffcL
#define BD_LLL_SHFT 24
#define BD_FFF_SHFT 27

#define BD_TAIL(bd_p) { \
	(bd_p)->bd_addr = 0; \
	(bd_p)->bun.bits = 0; \
	}

/* valid=1, chain=0, fff=0, lll=7, size=?  ===> 0x87xxxxxx */

#define BD_SIMPLE(bd_p, baddr, bsize) { \
	(bd_p)->bd_addr =  (u_long) (baddr); \
	(bd_p)->bun.bits = 0x87000000L | ((u_long)bsize - 4); \
	}

/* valid=1, chain=0, fff=0, lll=?, size=? */

#define BD_ALIGN(bd_p, baddr, bsize) { \
	(bd_p)->bd_addr =  (u_long) (baddr); \
	(bd_p)->bun.bits = 0x80000000L | \
			((((u_long)(bsize)-1) & 7) << BD_LLL_SHFT) | \
			((((u_long)(bsize)+7) & ~7) - 4); \
	}


/* valid=1, chain=0, fff=?, lll=?, size=? */

#define BD_NONALIGN(bd_p, baddr, bsize) { \
	register long unsize; \
	(bd_p)->bd_addr =  ((u_long) (baddr)) & ~7; \
	(bd_p)->bun.bits = 0x80000000L | \
		((((u_long) (baddr)) & 7) << BD_FFF_SHFT) | \
		((((u_long)(baddr)+(u_long)(bsize)-1) & 7) << BD_LLL_SHFT) | \
		(((((u_long)(baddr) & 7)+(u_long)(bsize)+7) & ~7) - 4); \
	}

#define BD_ADDR(bd_p) ( \
		(char *) (((u_long) (bd_p)->bd_addr) + (bd_p)->bun.bm.bd_fff) \
	)

#define BD_SIZE(bd_p) (\
		((bd_p)->bun.bits & BD_SZ_MSK) + 4 \
		    + ((bd_p)->bun.bm.bd_lll - 7) \
		    - (bd_p)->bun.bm.bd_fff \
	) 
		
#define BD_NEXT(bd_p) { \
	(bd_p)++; \
	assert((bd_p)->bun.bm.bd_chain == 0); \
	}


/*
 *	On the MULTIMAX all dma i/o is done using buffer descriptor lists as
 *	defined for the hydra EMC hardware.  It is basically a scatter/gather
 *	map to physical memory, and is more or less equivalent of the 
 *	page table entries used in code for physio.
 *
 */
/*
 *	This file was created by extracting from UMAX 4.2 includes the
 *	following files:
 *		crq.h
 *		crq_msgs.h
 *		crq_opcodes.h
 *		crq_status.h
 *		crq_attns.h
 *		iodefs.h
 */

/* 
 * Types that can appear in the hdr_type field.
 */

#define TYPE_MDMUD	3		/* Masstore disk unit data */
#define TYPE_MTMUD	4		/* Masstore tape unti data */
#define TYPE_CRQ	5		/* I/O controller cmd/response Q */
#define TYPE_CRQMSG	6		/* CRQ message packet */
#define TYPE_BDL	7		/* Buffer descriptor list */
#define TYPE_MUT	8		/* Masstore unit table */
#define TYPE_BUF	9		/* Buffer header */
#define TYPE_MSCRQ	10		/* Masstore CRQ data area */
#define TYPE_TEXT	11		/* Shared text structure */
#define TYPE_INODE	12		/* In-memory inode structure */
#define TYPE_SELDESCT	13		/* Select descriptor table */
#define TYPE_FILE	14		/* File structure */
#define TYPE_ERRLOG_MSG	15		/* Error log message */
#define TYPE_SLTYPAHD	16		/* Serial line typahead structure */
#define TYPE_SCC_RDPKT	17		/* Read cmd for SCC serial lines */
#define TYPE_SL_ATTN	18		/* SCC serial line attention */
#define TYPE_SL_WRITE_MSG 19		/* SCC serial line write message */
#define TYPE_CRQ_ABORT_MSG 20		/* CRQ abort operation message */
#define TYPE_SL_PAUSE_MSG 21		/* Pause a serial line message */
#define TYPE_SL_RESUME_MSG 22		/* Resume a serial line message */
#define TYPE_SL_MODE_MSG 23		/* Set/get line operational mode */
#define TYPE_CREATE_CHAN_MSG	24	/* Create channel message */
#define TYPE_DELETE_CHAN_MSG	25	/* Delete channel message */
#define TYPE_WARM_RESTART_MSG	26	/* Warm restart message */
#define TYPE_ENCRQ	27		/* Ethernet CRQ data area */
#define TYPE_ENCOPY 	28		/* Ethernet send data copy area */
#define TYPE_CRQ_STATS	29		/* CRQ statistics block	*/

/*
 * The extended statistics data structure contains information used in
 * performance monitoring of the crq interface.  It is referenced through the
 * crq_stats field if and only if CRQOPT_EXTSTATS is set in crq_opts.
 */

typedef	struct crq_stats {
	dbl_link_t	crs_links;	/* Standard header links */
	syshdr_t	crs_hdr;	/* Standard header size/type */
	short		crs_maxcmd;	/* Max size of cmd queue */
	short		crs_curcmd;	/* Current size of cmd queue */
	short		crs_maxrsp;	/* Max size of response queue */
	short		crs_currsp;	/* Current size of response queue */
	short		crs_maxattn;	/* Max size of attention queue */
	short		crs_curattn;	/* Current size of attention queue */
	short		crs_maxfree;	/* Max size of free list queue */
	short		crs_curfree;	/* Current size of free list queue */
	long		crs_vectattempts;/* # attempts to send a vector */
} crq_stats_t;

#define CRQ_STATS_NULL	((crq_stats_t *) 0)

/*
 *	The CRQ is the basic structure used for master/slave communication.
 */

typedef struct clock {
	int	lock_data;		/* low byte is the one that counts */
} crq_lock_data_t, *crq_lock_t;

typedef	struct crq {
	dbl_link_t	crq_links;	/* Standard header links */
	syshdr_t	crq_hdr;	/* Standard header size/type */
	crq_lock_data_t	crq_slock;	/* Spin lock */
	char		crq_padding[4];	/* Pad to match UMAX 4.2 size */
	char		crq_mode;	/* Polled vs interrupt value */
	char		crq_opt;	/* Option bit field */
	short		crq_unitid;	/* Unitid associated with crq */
	long		crq_slave_vect;	/* Vector to interrupt device/slave */
	long		crq_master_vect;/* Vector to interrupt host/master */
	dbl_link_t	crq_cmd;	/* Queue of outstanding commands */
	dbl_link_t	crq_immedcmd;	/* Queue of high-priority commands */
	dbl_link_t	crq_rsp;	/* Queue of completed responses */
	dbl_link_t	crq_attn;	/* Queue of attentions to master */
	dbl_link_t	crq_free;	/* Queue of free cmd/attn packets */
	long		crq_totcmds;	/* Total # commands issued (master) */
	long		crq_totrsps;	/* Tot. # responses returned (slave) */
	long		crq_totattns;	/* Total # attentions sent (slave) */
	long		crq_totvects;	/* Total # vectors sent (both) */
	crq_stats_t	*crq_stats;	/* Ptr to extended sts blk or NULL */
} crq_t;

/*
 * Mode definitions
 */
#define CRQMODE_POLL	0x00
#define CRQMODE_INTR	0x01

/*
 * Option definitions
 */
#define CRQOPT_EXTSTATS	0x01
#define CRQOPT_NULL	0x00

/* Useful constant */

#define CRQ_NULL ((crq_t *) 0)

/* All CRQ commands begin with a crq_msg structure, followed by a set of
 * variables specific to the particular message.  These variables provide for
 * the arguments and return status values for the particular command.
 *
 * Justification for this division is that some of the processing of
 * any CRQ command does not depend on the format of the particular
 * command.  All of this command independent information is collected into
 * the crq_msg structure.
 */

typedef struct crq_msg {
	dbl_link_t 	crq_msg_links;	/* Queue links */
	syshdr_t	crq_msg_hdr;	/* Standard system header */
	crq_t		*crq_msg_crq;	/* Back ptr to msg's CRQ */
	unsigned short	crq_msg_code;	/* Message type code */
	unsigned short	crq_msg_unitid;	/* Unitid of message target */
	long		crq_msg_refnum;	/* Device-specific field */
	long		crq_msg_timestamp[2];/* Time message was queued */
	long		crq_msg_status;	/* Overall message status */
} crq_msg_t;

#define CRQ_MSG_NULL	((crq_msg_t *) 0)

/*
 * The following compound structures define the message formats for
 * common crq commands.
 */

typedef struct crq_load_msg {		/* LOAD ON-BOARD MEMORY */
	crq_msg_t	load_hdr;
	char		*load_src_addr;	/* Hydra memory start address */
	char		*load_dst_addr;	/* Local memory load address */
	long		load_byte_cnt;	/* # bytes to load */
	long		load_status;	/* Result of load */
} crq_load_msg_t;

typedef struct crq_dump_msg {		/* DUMP ON-BOARD MEMORY */
	crq_msg_t	dump_hdr;
	char		*dump_src_addr;	/* Local memory start address */
	char		*dump_dst_addr;	/* Hydra memory dump address */
	long		dump_byte_cnt;	/* # bytes to dump */
	long		dump_status;	/* Result of dump */
} crq_dump_msg_t;

typedef struct crq_exec_msg {		/* START PROGRAM */
	crq_msg_t	exec_hdr;
	unsigned int	exec_start_addr;/* Program start address */
	long		exec_status;	/* Result of exec */
} crq_exec_msg_t;

typedef	struct crq_warm_msg {		/* WARM RESTART */
	crq_msg_t	warm_hdr;
	long		warm_status;	/* Detailed result of warm restart */
} crq_warm_msg_t;

typedef	struct crq_cold_msg {		/* COLD RESTART */
	crq_msg_t	cold_hdr;
	long		cold_status;	/* Detailed result of cold restart */
} crq_cold_msg_t;

typedef struct crq_create_chan_msg {	/* CREATE CHANNEL */
	crq_msg_t	creat_chan_hdr;
	crq_t		*creat_chan_crq;/* CRQ address */
	long		creat_chan_status;/* Result of create */
} crq_create_chan_msg_t;

typedef struct crq_delete_chan_msg {	/* DELETE CHANNEL */
	crq_msg_t	del_chan_hdr;
	crq_t		*del_chan_crq;	/* CRQ address */
	long		del_chan_status;/* Result of delete */
} crq_delete_chan_msg_t;

typedef struct crq_abort_req_msg {	/* ABORT REQUEST */
	crq_msg_t	abort_hdr;
	long		abort_refnum;	/* Refnum of command to abort */
	long		abort_status;	/* Result of abort */
} crq_abort_msg_t;

typedef struct crq_abort_all_msg {	/* ABORT ALL */
	crq_msg_t	abort_all_hdr;
	long		abort_all_status;/* Result of abort */
} crq_abort_all_msg_t;

typedef struct crq_nop_msg {		/* NOP */
	crq_msg_t	nop_hdr;
	long		nop_status;	/* Result of nop */
} crq_nop_msg_t;

/* All CRQ attentions begin with a crq_msg structure, followed by a set of
 * variables specific to the particular attention.
 *
 * Justification for this division is that some of the processing of
 * any CRQ attention does not depend on the format of the particular
 * attention.  All of this message-independent information is collected into
 * the crq_msg structure.
 */

typedef struct crq_locktmo_msg {	/* LOCK TIMEOUT */
	crq_msg_t	locktmo_hdr;
	crq_t		*locktmo_crq;	/* Pointer to applicable crq */
	short		locktmo_status;	/* Auxilliary status */
} crq_locktmo_msg_t;

typedef struct crq_vectnot_xmt_msg {	/* VECTOR NOT TRANSMITTED */
	crq_msg_t	vectnot_xmt_hdr;
	crq_t		*vectnot_xmt_crq;/* Pointer to applicable crq */
	short		vectnot_xmt_status;/* Auxilliary status */
} crq_vectnot_xmt_msg_t;

typedef struct crq_vectnot_rcv_msg {	/* VECTOR NOT RECEIVED */
	crq_msg_t	vectnot_rcv_hdr;
	crq_t		*vectnot_rcv_crq;/* Pointer to applicable crq */
	short		vectnot_rcv_status;/* Auxilliary status */
} crq_vectnot_rcv_msg_t;

typedef struct crq_nofree_msg {		/* NO FREE LIST PACKETS */
	crq_msg_t	nofree_hdr;
	crq_t		*nofree_crq;	/* Pointer to applicable crq */
	short		nofree_status;	/* Auxilliary status */
} crq_nofree_msg_t;

typedef struct crq_invcrq_msg {		/* BOGUS-LOOKING CRQ */
	crq_msg_t	invcrq_hdr;
	crq_t		*invcrq_crq;	/* Pointer to applicable crq */
	short		invcrq_status;	/* Auxilliary status */
} crq_invcrq_msg_t;

typedef struct crq_maxsize_msg {		/* MAX_ATTN_SIZE CRQ */
	crq_msg_t	invcrq_hdr;
	char		data[32];
} crq_maxsize_msg_t;

/*
 * The maximum size of any attention message in the system
 */
#define MAX_ATTN_SIZE	sizeof(crq_msg_t) + 24

/*
 * The following codes define the standard commands recognized by all crq
 * systems.
 */

#define CRQOP_CREATE_CHANNEL		1
#define CRQOP_DELETE_CHANNEL		2
#define CRQOP_ABORT_REQ			3
#define CRQOP_ABORT_ALL			4
#define CRQOP_LOAD			5
#define CRQOP_DUMP			6
#define CRQOP_EXEC_PROG			7
#define CRQOP_WARM			8
#define CRQOP_COLD			9
#define CRQOP_NOP			10

/*
 * The following commands are specific to general masstore operations.
 */

#define CRQOP_MS_BASE			64
#define CRQOP_MS_READ			64
#define CRQOP_MS_WRITE			65
#define CRQOP_MS_READ_CONFIG		66
#define CRQOP_MS_READ_STAT		67
#define CRQOP_MS_READ_RESET_STAT 	68
#define CRQOP_MS_DISABLE_WRITE		69
#define CRQOP_MS_ENABLE_WRITE		70
#define CRQOP_MS_TEST_UNIT_READY	71

/*
 * Commands for masstore disks
 */

#define CRQOP_MD_RECALIBRATE		72
#define CRQOP_MD_FORMAT_DRIVE		73
#define CRQOP_MD_REPLACE_TRACK		74
#define CRQOP_MD_READ_GEOMETRY		75
#define CRQOP_MD_READ_BAD_BLOCK		76
#define CRQOP_MD_SEEK			77
#define CRQOP_MD_READ_LONG		78
#define CRQOP_MD_WRITE_LONG		79
#define CRQOP_MD_CHECK_TRACK		80
#define CRQOP_MD_READ_DT_PROM		81
#define CRQOP_MD_READ_SECT_ID		82

/*
 * Commands for masstore tapes
 */

#define CRQOP_MT_WRITE_FILE_MARK 	83
#define CRQOP_MT_REWIND			84
#define CRQOP_MT_SPACE_BLOCK		85
#define CRQOP_MT_SPACE_FMARK		86
#define CRQOP_MT_SPACE_SQN_FMARK 	87

/*
 * EMC Ethernet commands
 */

#define CRQOP_EN_XMIT_FRAME		256
#define CRQOP_EN_RCV_FRAME		257
#define CRQOP_EN_READ_STAT		258
#define CRQOP_EN_READ_RST_STAT		259
#define CRQOP_EN_CONTROL		260
#define CRQOP_EN_XMIT_FRAME_STS	    	261

/*
 * SCC control commands
 */

#define CRQOP_SCC_SYS_STATE		320
#define CRQOP_SCC_SET_TOY		321
#define CRQOP_SCC_GET_TOY		322
#define CRQOP_SCC_SET_DCT		323
#define CRQOP_SCC_GET_DCT		324
#define CRQOP_SCC_GET_BBRAM_AVAIL	325
#define CRQOP_SCC_REBOOT		326
#define CRQOP_SCC_START_PROFILE		327
#define CRQOP_SCC_STOP_PROFILE		328

/*
 * SCC serial line commands
 */

#define CRQOP_SL_SET_MODE		336
#define CRQOP_SL_GET_MODE		337
#define CRQOP_SL_INIT			338
#define CRQOP_SL_PAUSE			339
#define CRQOP_SL_RESUME			340
#define CRQOP_SL_READ			341
#define CRQOP_SL_WRITE			342

/* SL attentions for the SCC */

#define CRQATTN_SL_READ			360
#define CRQATTN_SL_WRITE		361

/* Other SCC attentions */

#define CRQATTN_ENV			362
#define CRQATTN_MEM_CSR			363
#define CRQATTN_PS_FAIL			364
#define CRQATTN_SOFTERR			365
#define CRQATTN_ALIVE			366

/* Standard and device-specific attention message code */

#define CRQATN_BASE			0x8000
#define CRQATN_LOCK_TMO			CRQATN_BASE+1
#define CRQATN_VECT_NOT_XMIT		CRQATN_BASE+2
#define CRQATN_VECT_NOT_RCV		CRQATN_BASE+3
#define CRQATN_NO_FREE_MSG		CRQATN_BASE+4
#define CRQATN_INVLD_CRQ		CRQATN_BASE+5

#define CRQATN_EMC_ATN			CRQATN_BASE+64
#define CRQATN_MSC_ATN			CRQATN_BASE+65

/*
 * Standard crq message status codes that appear in the message-independent
 * status field...
 */

#define STS_SUCCESS	0	/* Operation successful */
#define STS_WARNING	1	/* Operation successful: status to report */
#define STS_PENDING	2	/* Used when an I/O command is in progress */
#define STS_QUEUED	3	/* Command queued, waiting for device */
#define STS_FREE	4	/* Message packet on free list */
#define STS_ABORTED	5	/* Operation aborted */
#define STS_BADARG	6	/* Invalid argument */
#define STS_INVOPCODE	7	/* Unrecognized opcode field contents */
#define STS_ERROR	8	/* Operation unsuccessful */
#define STS_TIMEOUT	9	/* Operation timed out */

/*
 * Definitions needed for dealing with appropriate sub-fields within
 * the CRQ fields.
 */

/*
 * Define an ultrmax device type of the lamp,slot,dev,lun variety
 */
typedef long	umdev_t;

#define MAKEUNITID(lamp, slot, dev, lun)	\
	(((lamp & 0xf) << 12)			\
	 | ((slot & 0xf) << 8)			\
	 | ((dev & 0x3) << 6)			\
	 | (lun & 0x3f))
#define GETLAMP(unit)		(((int)(unit) >> 12) & 0xf)
#define GETSLOT(unit)		(((int)(unit) >> 8) & 0xf)
#define GETDEV(unit)		(((int)(unit) >> 6) & 0x3)
#define GETLUN(unit)		((int)(unit) & 0x3f)

#if	MMAX_XPC
#define GETVECTORNUM(vec) (((xpcvbxmit_t *)(vec))->f.v_vector)
#define MAKEVECTOR(vec,CLASS,NUM) { \
		((xpcvbxmit_t *) (vec))->l = 0; \
		((xpcvbxmit_t *) (vec))->f.v_class = CLASS; \
		((xpcvbxmit_t *) (vec))->f.v_vector = NUM; \
		}
#endif	MMAX_XPC

#if	MMAX_APC
#define GETVECTORNUM(vec) (((apcvbxmit_t *)(vec))->f.v_vector)
#define MAKEVECTOR(vec,CLASS,NUM) { \
		((apcvbxmit_t *) (vec))->l = 0; \
		((apcvbxmit_t *) (vec))->f.v_class = CLASS; \
		((apcvbxmit_t *) (vec))->f.v_vector = NUM; \
		}
#endif	MMAX_APC

#if	MMAX_DPC
#define GETVECTORNUM(vec) (((dpcsendvec_t *)(vec))->f.v_vector)
#define MAKEVECTOR(vec,CLASS,NUM) { \
		((dpcsendvec_t *) (vec))->l = 0; \
		((dpcsendvec_t *) (vec))->f.v_class = CLASS; \
		((dpcsendvec_t *) (vec))->f.v_vector = NUM; \
		}
#endif	MMAX_DPC

#define GETMAJOR(dev)		(((int)(dev) >> 24) & 0xff)
#define GETUNITID(dev)		(((int)(dev) >> 8) & 0xffff)
#define GETPAR(dev)		(minor(dev) % 16)

#define MAKEDEV(major, lamp, slot, dev, lun, par)	\
	(((major & 0xff) << 24)				\
	| (MAKEUNITID(lamp, slot, dev, lun) << 8)	\
	| (par & 0xff))
#define MAKEDEVUNIT(major, unit, par)			\
	(((major & 0xff) << 24)				\
	| ((unit & 0xffff) << 8)			\
	| (par & 0xff))

/*
 * Dummy field for invalid vectors
 */
#define VEC_INVALID	0xffffffff

/*
 * Logical unit numbers known by default on the MULTIMAX
 */
#define REQ_LUN		0
#define SLOT_LUN	1

/*
 * The following structure is used by procs that decide they need
 * a synchronization means between setting up an I/O and having
 * the interrupt service routine cause the requestor to resume
 * execution. This is done by executing get_isr_queue() from the
 * I/O requestor and put_isr_queue() from the interrupt service
 * routine.
 */

typedef mpqueue_head_t isr_queue_t;

/*
 * The following macro is used to initialize an isr_queue
 */

#define initisrqueue mpqueue_init

#if	_KERNEL&&__GNUC__
/* crq is just another form of simple lock */
extern __inline__ crq_unlock(l)
        register crq_lock_data_t *l;
{
        l->lock_data = 0;
}

extern __inline__ crq_lock(l)
        register crq_lock_data_t *l;
{
        _simple_lock(&l->lock_data);
}

#define	crq_lock_init crq_unlock

#endif	/* _KERNEL&&__GNUC__ */

#endif	_CRQDEFS_
