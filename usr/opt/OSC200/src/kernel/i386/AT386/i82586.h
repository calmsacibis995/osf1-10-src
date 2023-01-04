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
 *	@(#)$RCSfile: i82586.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:36 $
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
 * Defines for managing the status word of the 82586 cpu.  For details see
 * the Intel LAN Component User's Manual starting at p. 2-14.
 *
 */

#define SCB_SW_INT	0xf000
#define SCB_SW_CX	0x8000
#define SCB_SW_FR	0x4000
#define SCB_SW_CNA	0x2000
#define SCB_SW_RNR	0x1000

/* 
 * Defines for managing the Command Unit Status portion of the 82586
 * System Control Block.
 *
 */

#define SCB_CUS_IDLE	0x0000
#define SCB_CUS_SUSPND	0x0100
#define SCB_CUS_ACTV	0x0200

/* 
 * Defines for managing the Receive Unit Status portion of the System
 * Control Block.
 *
 */

#define SCB_RUS_IDLE	0x0000
#define SCB_RUS_SUSPND	0x0010
#define SCB_RUS_NORESRC 0x0020
#define SCB_RUS_READY	0x0040

/*
 * Defines that manage portions of the Command Word in the System Control
 * Block of the 82586.  Below are the Interrupt Acknowledge Bits and their
 * appropriate masks.
 *
 */

#define SCB_ACK_CX	0x8000
#define SCB_ACK_FR	0x4000
#define SCB_ACK_CNA	0x2000
#define SCB_ACK_RNR	0x1000

/* 
 * Defines for managing the Command Unit Control word, and the Receive
 * Unit Control word.  The software RESET bit is also defined.
 *
 */

#define SCB_CU_STRT	0x0100
#define SCB_CU_RSUM	0x0200
#define SCB_CU_SUSPND	0x0300
#define SCB_CU_ABRT	0x0400

#define SCB_RESET	0x0080

#define SCB_RU_STRT	0x0010
#define SCB_RU_RSUM	0x0020
#define SCB_RU_SUSPND	0x0030
#define SCB_RU_ABRT	0x0040


/*
 * The following define Action Commands for the 82586 chip.
 *
 */

#define	AC_NOP		0x00
#define AC_IASETUP	0x01
#define AC_CONFIGURE	0x02
#define AC_MCSETUP	0x03
#define AC_TRANSMIT	0x04
#define AC_TDR		0x05
#define AC_DUMP		0x06
#define AC_DIAGNOSE	0x07


/*
 * Defines for General Format for Action Commands, both Status Words, and
 * Command Words.
 *
 */

#define AC_SW_C		0x8000
#define AC_SW_B		0x4000
#define AC_SW_OK	0x2000
#define AC_SW_A		0x1000

#define	AC_CW_EL	0x8000
#define AC_CW_S		0x4000
#define AC_CW_I		0x2000

/*
 * Specific defines for the transmit action command.
 *
 */

#define TBD_SW_EOF	0x8000
#define TBD_SW_COUNT	0x3fff

/*
 * Specific defines for the receive frame actions.
 *
 */

#define RBD_SW_EOF	0x8000
#define RBD_SW_COUNT	0x3fff


/*
 * 82586 chip specific structure definitions.  For details, see the Intel
 * LAN Components manual.
 *
 */


typedef	struct	{
	ushort	scp_sysbus;
	ushort	scp_unused[2];
	ushort	scp_iscp;
	ushort	scp_iscp_base;
} scp_t;


typedef	struct	{
	ushort	iscp_busy;
	ushort	iscp_scb_offset;
	ushort	iscp_scb;
	ushort	iscp_scb_base;
} iscp_t;


typedef struct	{
	ushort	scb_status;
	ushort	scb_command;
	ushort	scb_cbl_offset;
	ushort	scb_rfa_offset;
	ushort	scb_crcerrs;
	ushort	scb_alnerrs;
	ushort	scb_rscerrs;
	ushort	scb_ovrnerrs;
} scb_t;


typedef	struct	{
	ushort	tbd_offset;
	uchar	dest_addr[6];
	ushort	length;
} transmit_t;


typedef	struct	{
	ushort	fifolim_bytecnt;
	ushort	addrlen_mode;
	ushort	linprio_interframe;
	ushort	slot_time;
	ushort	hardware;
	ushort	min_frame_len;
} configure_t;


typedef	struct	{
	ushort	ac_status;
	ushort	ac_command;
	ushort	ac_link_offset;
	union	{
		transmit_t	transmit;
		configure_t	configure;
		uchar		iasetup[6];
	} cmd;
} ac_t;
	

typedef	struct	{
	ushort	act_count;
	ushort	next_tbd_offset;
	ushort	buffer_addr;
	ushort	buffer_base;
} tbd_t;


typedef	struct	{
	ushort	status;
	ushort	command;
	ushort	link_offset;
	ushort	rbd_offset;
	uchar	destination[6];
	uchar	source[6];
	ushort	length;
} fd_t;


typedef	struct	{
	ushort	status;
	ushort	next_rbd_offset;
	ushort	buffer_addr;
	ushort	buffer_base;
	ushort	size;
} rbd_t;
