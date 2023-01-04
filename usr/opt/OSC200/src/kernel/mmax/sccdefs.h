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
 *	@(#)$RCSfile: sccdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:58 $
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

/*

 *
 */

#ifndef	_SCCDEFS_
#define _SCCDEFS_

/*1
 * sccdefs.h
 *
 *	This file deals with all the definitions/structures
 *	dealing with the SCC on the MULTIMAX.
/*/


/*
 * This file was created by extracting the UMAX 4.2 files:
 *	sccdefs.h
 *	nmilock.h
 *	scc_msgs.h
 *	scc_status.h
 *	sys_state.h
 *	dct.h
 */


/*2
 * SCC register definitions
/*/

#ifndef	BIT
#define BIT(n)	(1 << n)
#endif

/*
 * Number of slots and things in slots
 */
#define NUM_SLOT	16
#define NUM_MEM		16
#define NUM_SLOT_DEV	4

/*
 * Number of SCC Analog/Digital channels.
 */
#define NUM_AD		10

/*
 * SCC shared memory layout
 */
#define SCC_SLOT		12
#define SCC_CRQS_SLOT		4
#define SCC_CRQS_OFFSET		1
#define SCCMEM_BASE		0xfffc0000
#define SCCMEM_BASE_24		0x00fc0000
#define SCCMEM_TOP		0xfffc8000
#define SCCREG_BASE		(SCCMEM_BASE + 0x10000)
#define SCCREG_TOP		(SCCREG_BASE + 0x300)

/*
 * Console buffer and request flag
 */
#define SCC_CONSOLE_XMTREQ	((char *)(SCCMEM_BASE))
#define SCC_CONSOLE_XMTBUF	((char *)(SCCMEM_BASE + 1))

/* Define Memory CSR addresses.
 */
#define SMCCSR_BASE		(SCCMEM_BASE + 0x20000)
#define MEM_CSR_BASE		SMCCSR_BASE
#define SMCCSR_SIZE		0x040
#define MEM_CSR_SIZE		SMCCSR_SIZE

/* Layout of Memory CSR's.
 */
typedef struct status_mem_csr {
    unsigned set	: 2;	/* Interleaved board number.		    */
    unsigned start_addr : 10;	/* Starting ADDR (except for A3 anomaly).   */
    unsigned interleaved: 1;	/* Board interleave bit.		    */
    unsigned bd_enable	: 1;	/* Board enable.			    */
    unsigned quick_ver	: 1;	/* Initiate quick verify.		    */
    unsigned full_test	: 1;	/* Initiate full test.			    */
    unsigned double_err : 1;	/* Double error flag.			    */
    unsigned single_err : 1;	/* Most recent write data error.	    */
    unsigned write_err	: 1;	/* Most recent write data error.	    */
    unsigned verify_stat: 1;	/* Indicates SA is error code.		    */
    unsigned _4_way_int : 1;	/* If (interleaved) this makes it 4-way.    */
    unsigned unused	: 3;	/* Unused.				    */
    unsigned addid	: 8;	/* ADDID of most recent error.		    */
} status_mem_csr_t;

/* 
 * SCC system interval timer control and count
 */
typedef	union	sccpitctl {
	struct {
		unsigned int	c_bcd:1,
				c_mode:3,
				c_rdwr:2,
				c_select:2;
	} f;
	char c;
} sccpitctl_t;

#define SCCPITCTRL_BCD		0x01
#define SCCPITCTRL_MODE		0x0e
#define SCCPITCTRL_RDWR		0x30
#define SCCPITCTRL_SELECT	0xc0

#define SCCREG_PITCNT0	(char *)(SCCREG_BASE)
#define SCCREG_PITCNT1	(char *)(SCCREG_BASE+0x4)
#define SCCREG_PITCNT2	(char *)(SCCREG_BASE+0x8)
#define SCCREG_PITCTL	(char *)(SCCREG_BASE+0xc)

/*
 * SCC interval timer vector
 */
#define SCCREG_PITVEC		(long *)(SCCREG_BASE + 0x100)

/*
 * SCC free running counter
 */
#define SCCREG_FRCNT	(long *)(SCCREG_BASE + 0x200)
#define FRcounter	*SCCREG_FRCNT

/*
 * Macro to calculate the address of a CRQ in SCC shared memory given the
 * slot and device in slot
 */
#define REQ_CRQ(slot, dev)	\
	(crq_t *) (SCCMEM_BASE +	\
	((((slot + SCC_CRQS_OFFSET) * SCC_CRQS_SLOT) + dev) * sizeof (crq_t)))



/*
 * Number of attention messages to allocate for slot level CRQ
 */
#define NUM_SCC_ATTNS	2

/*
 * Serial line definitions
 */
#define NSL		4

#define SL_READ_LUN	0
#define SL_WRITE_LUN	1

#define MAKESLLUN(line, chan)	((line << 2) | chan | 0x20)
#define GETSLLINE(dev)		((GETUNITID(dev) >> 2) & 0x07)


/* After diagnostic self-tests are completed for any device the device will
 * poll the BOOT_FLAG until it is NOT in STAND_BY.  Report an error if an
 * undefined communication mode is found.  Execute multi-test code if the
 * value of the flag is DIAG_MODE.  Wait for a command on your requestor
 * CRQ if the value of the flag is OPER_MODE.  Please note that the size
 * of the flag was reduced from a long to a short.
 *
 * NOTE:  This flag is to be checked ONLY at the following times:
 *
 *	    1. After self-tests have been completed, but before multi-tests.
 *	    2. After the ENTER_OPERATIONS multi-test command has been given.
 */

#define BOOT_FLAG	(SCCMEM_TOP - sizeof(short))

#define STAND_BY    0	/* SCC communication inactive...WAIT! */
#define OPER_MODE   1	/* SCC communication via requestor CRQ's. */
#define DIAG_MODE   2	/* SCC communication via multi-test commands. */


#define SCC_STATUS_FLAG		(BOOT_FLAG - sizeof(short))
#define SCC_VTERM_ENABLED	0x01
#define SCC_MULTI_ENABLED	0x02
#define SCC_REQCRQ_ENABLED	0x04



/*
 * Define the addresses of the locked array of bytes used to determine when a
 * a system nmi has been requested by software.
 */

#define NMI_LOCK	(SCCMEM_TOP - 0x80)


/*
 *	Virtual Terminal Definitions
 */

/* For each requestor in the system, there are two virtual terminals:
 * TTY and DDT.  Each virtual terminal is the size of two shorts and
 * is located in SCC shared memory below the NMI data area.
 */
#define NUM_VTERM_TYPES		2

#define VTERM_TTY		0
#define VTERM_DDT		1


#define VTERM_BASE 	(NMI_LOCK - \
( 2 * sizeof(short) * NUM_VTERM_TYPES * NUM_SLOT * NUM_SLOT_DEV) )



/* At least in the SCC stdio.c, the distinction between a virtual
 * terminal and real terminal is implied by its file descriptor
 * (an integer).
 *
 * Virtual terminal file descriptors begin at an arbitrarily chosen
 * large number.  The various virtual terminal macros are driven
 * off of this file descriptor.
 */
#define FIRST_VTERM_FP		0x100
#define VGET_FP(slot, dev, type) (FIRST_VTERM_FP +\
	(slot * NUM_SLOT_DEV * NUM_VTERM_TYPES + dev * NUM_VTERM_TYPES + type))
	

/* A virtual terminal is a long word in SCC shared memory uses for 
 * synchronous charatacter data transmition between the SCC and a
 * requestor.  To a requestor, the virtual terminal looks like:
 *
 *	31		15
 *	+---------------+--------------+	R ~ read only
 *	| R Rx short	| R/W Tx short |	R/W ~ read/write
 *	+---------------+--------------+
 *
 * whereas to the SCC, the virtual terminal looks like
 *
 *	31		15
 *	+---------------+--------------+
 *	| R/W Tx short	| R Rx short   |
 *	+---------------+--------------+
 *
 * This cross over is hidden from the virtual  terminal routines themselves
 * by use of the apporpriate macros, which are DPC, EMC and SCC environment
 * dependent.
 *
 *
 *	Tx short:
 *		.[0,7]		Tx data byte
 *		.[8]		'I am connected' bit
 *		.[9,10]		Tx,Rx data available bits
 *		.[11]		Awaiting Rx data (hung in a getc() )
 *		.[12]		Send Break
 *		.[13]		Send I/O error, data error e.g. parity
 *		.[14]		Send I/O error, data lost
 *		.[15]		Reserved
 *
 *	Rx short:
 *		.[0,7]		Rx data byte
 *		.[8]		'He is connected' bit
 *		.[9,10]		Tx,Rx data available bits.
 *		.[11]		'He is waiting to receive data'
 *		.[12]		Break detected
 *		.[13]		I/O error received, data error
 *		.[14]		I/O error, data lost
 *		.[15]		Reserved
 *
 * The Tx, Rx data available bits need some further expalnation:
 * data is present if the EXCLUSIVE OR of the Tx bit in both of the
 * two shorts is 1 or if the Rx bit in both of the two shorts is set.
 * Of course, which bit is the Tx and which is the Rx is reversed
 * between the SCC and the requestors.
 */



#define VTERM_CONNECT_BIT	BIT(8)

 
#if	DPC_TYPE || EMC_TYPE
#define VTERM_TX_BIT		BIT(9)
#define VTERM_RX_BIT		BIT(10)
#endif

#if	SCC_TYPE
#define VTERM_TX_BIT		BIT(10)
#define VTERM_RX_BIT		BIT(9)
#endif

#define VTERM_WAIT_BIT		BIT(11)
#define VTERM_BREAK_BIT		BIT(12)
#define VTERM_DATA_ERROR_BIT	BIT(13)
#define VTERM_DATA_LOST_BIT	BIT(14)


/* Definition of pointers to Tx and Rx shorts */
#define VTERM_ADDR(fp) (VTERM_BASE + 2 * sizeof(short) * (fp - FIRST_VTERM_FP))

#if	DPC_TYPE
#define VTERM_TX(fp)	((short *)VTERM_ADDR(fp))
#define VTERM_RX(fp)	((short *)(VTERM_ADDR(fp) + sizeof(short)))
#endif

#if	EMC_TYPE
#define VTERM_TX(fp)	((short *)SET_W0_ADDR(VTERM_ADDR(fp)))
#define VTERM_RX(fp)	((short *)SET_W1_ADDR(VTERM_ADDR(fp) + sizeof(short)))
#endif

#if	SCC_TYPE
#define VTERM_TX(fp)	(((short *)VTERM_RX(fp)) + 1)
#define VTERM_RX(fp)	((short *)W0_ADDR(VTERM_ADDR(fp)))
#endif

/*
 * Profiling flags, one per device per slot.
 */
#define PROFILING_BASE	(VTERM_BASE - (sizeof(char) * NUM_SLOT * NUM_SLOT_DEV))


/*
 * The nmilock structure consists of a lock byte, lock owner, and an array
 * of flags for each device in the system. The flag is set when a processor
 * takes control of the debugger (dbmon) or when the scc console command
 * 'nmi slot device' is issued.
 */
typedef	struct	nmilock	{
	char	nmi_state;		/* Lock state (and byte) */
	char	nmi_owner;		/* Lock owner (or previous) cpuid */
	char	nmi_flags[NUM_SLOT * NUM_SLOT_DEV];
					/* A byte for each device in the
					 *  system */
} nmilock_t;

#define NMI_PANIC	1	/* Make the system panic */
#define NMI_RESUME	2	/* Merely resume processing */
#define NMI_WAIT	3	/* Non-panic CPU is to merely hang around */
#define NMI_DEBUG	4	/* Go to dbmon for debugging */

/* After diagnostic self-tests are completed for any device the device will
 * poll the BOOT_FLAG until it is NOT in STAND_BY.  Report an error if an
 * undefined communication mode is found.  Execute multi-test code if the
 * value of the flag is DIAG_MODE.  Wait for a command on your requestor
 * CRQ if the value of the flag is OPER_MODE.  Please note that the size
 * of the flag was reduced from a long to a short.
 *
 * NOTE:  This flag is to be checked ONLY at the following times:
 *
 *	    1. After self-tests have been completed, but before multi-tests.
 *	    2. After the ENTER_OPERATIONS multi-test command has been given.
 */

#define BOOT_FLAG	(SCCMEM_TOP - sizeof(short))

#define STAND_BY    0	/* SCC communication inactive...WAIT! */
#define OPER_MODE   1	/* SCC communication via requestor CRQ's. */
#define DIAG_MODE   2	/* SCC communication via multi-test commands. */


/*
 * Number of attention messages to allocate for slot level CRQ
 */
#define NUM_SCC_ATTNS	2

/*
 * Serial line definitions
 */
#define NSL		4

#define SL_READ_LUN	0
#define SL_WRITE_LUN	1

#define MAKESLLUN(line, chan)	((line << 2) | chan | 0x20)
#define GETSLLINE(dev)		((GETUNITID(dev) >> 2) & 0x07)

/*2
 * SCC messages:
 *
 * This file contains the structure definitions for all nonstandard
 * commands and attentions which are possible on the SCC requestor
 * and controller channels.
 *
 * Unit numbers:
 *
 * SCC requestor channel:	0 0 0 0 0 0
 * SCC controller channel: 	0 0 0 0 0 1
 *
 * These two channels support the same set of commands, except that the
 * requestor channel provides a special forwarding mechanism to and from
 * the individual requestors.  It is recomended that the controller channel
 * be created early and that the requestor channel be used only for unusual
 * error conditions.
/*/

/* set/get time of year clock */
typedef struct crq_toy_msg {
	crq_msg_t toy_hdr;
	struct toy *toy_addr;
	short toy_status;
	} crq_toy_msg_t;

/* return system configuration data */
typedef struct crq_sys_state_msg {
	crq_msg_t sys_state_hdr;
	struct sys_state *sys_state_addr;
	short sys_state_status;
	} crq_sys_state_msg_t;

/* set/get DCT (device configuration table) */
typedef struct crq_dct_msg {
	crq_msg_t dct_hdr;
	struct dct *dct_addr;
	long dct_buff_size;
	short dct_status;
	} crq_dct_msg_t;

/* get currently available size of battery backed up RAM in SCC */
typedef struct crq_bbram_msg {
	crq_msg_t bbram_hdr;
	long bbram_size;
	short bbram_status;
	} crq_bbram_msg_t;

/* start/stop system profiling */
typedef struct crq_profile_msg {
	crq_msg_t profile_hdr;
	int profile_rate;
	short profile_status;
	} crq_profile_msg_t;


/*
 * The following are the status codes returned by SCC commands
 * and attention.
 */

#define SCC_CRQ_CHAN_UNDEF		1
#define SCC_CRQ_CHAN_EXITS		2
#define SCC_CRQ_CHAN_CREATED		3
#define SCC_CRQ_CHAN_DELETED		4
#define SCC_CRQ_CHAN_ACTIVE		5

/*
 * The following is the system state structure dealt with by the
 * SCC.
 */

#define MAX_BOOT_NAME_SIZE 		64
#define SYS_STATE_AUTOBOOT_FLAG		1

typedef	struct mem_state {
	long mem_status;
	long mem_addr;		/* filled in after configuration. */
	long mem_sn;		/* serial number */
	long mem_rev;		/* revision number */
	long mem_part;		/* part number */
	long mem_flags;		/* miscellaneous flags. */
			/* Bit 0 when set means do NOT configure the board. */
	} mem_state_t;

typedef	struct req_state {
	long req_status[NUM_SLOT_DEV * 3];  /* devices per board times 3 */
					    /* status of each dev on board */
	long req_board_id;		    /* From id prom, see idprom.h */
	long req_sn;			    /* From id prom, see HIT-0040 */
	unsigned short req_major_revision;
	unsigned short req_minor_revision;
	long req_flags;			    /* miscellaneous flags. */
			/* Bit 0 when set means do NOT configure the board. */
	} req_state_t;

typedef	struct sys_state {
	long sys_state_topmem;  /* Top of memory.			    */
	long sys_state_flags;	/* Boot flags.				    */
	long sys_state_console;	/* Unit ID of console device.		    */
	long sys_state_bootid; 	/* ID of boot device.			    */
	short sys_state_boottype;/* Type of boot device.		*/
	short sys_state_bootpun;/* Physical unit number of boot device.	    */
				/* Pathname of bootdevice.		    */
	char sys_state_bootname[MAX_BOOT_NAME_SIZE];
	long sys_state_cpu;	/* ID of boot CPU.			    */
	long sys_state_config;	/* Bit map of permanently frozen requestors.*/

	struct mem_state sys_state_mem[NUM_MEM];

	struct req_state sys_state_req[NUM_SLOT];
	} sys_state_t;

/*
 *  The Device Configuration Table 
 *  (created by devconfig, then stored in SCC RAM for sysboot)
 */

typedef	struct	dct_fields  	{
	short	dct_name;	/* offset within dct for symbolic name  */
	short	dct_unitid;	/* standard unitid for this device	*/
	short	dct_flags;	/* bootable, autobootable, etc		*/
} dct_fields_t;

typedef struct	dct_header	{
	short	dct_total_bytes;	/* # of bytes for entire dct 	*/
	short	dct_num_entries;	/* # of devices in the dct	*/
	long	dct_reserved;		/* reserved for future use	*/
} dct_header_t;

typedef	struct	dct	{
	dct_header_t	dct_hdr;	/* info about the dct itself	*/
	dct_fields_t	dct_entry[1];	/* the actual device data -- this 
					   `[1]` is to fake out the compiler,
					   since the actual size will depend
					   on which devices are configured */
} dct_t;

/*
 * if (dct_flags & AUTOBOOTABLE) -> device will be included in autoboot process
 */

#define AUTOBOOTABLE	1

#endif	_SCCDEFS_
