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
 *	@(#)$RCSfile: emcdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:13 $
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


/*1
 * emcdefs.h
 *
 *	Definitions dealing with the EMC that are of general
 *	interest.
/*/

/*
 * This file was created by extracting the UMAX 4.2 files
 *	emcdefs.h
 *	emclun.h
 *	emcstat.h
 *	em_msg_hdr.h
 *	emcattn.h
 */

/*
 * Number of attention messages to allocate for slot level CRQ
 */

#define NUM_EMC_ATTNS	2

/*
 * Logical unit number definitions on the EMC.
 */

#define EMCLUN_MASSTORE	2
#define EMCLUN_ENET	3


#define EMCLUN_MT0	8

#define EMCLUN_EN_CMD	61
#define EMCLUN_EN_RCVS	62
#define EMCLUN_EN_RCVL	63

/* Status code for Ethernet and Masstore messages */

#define EMCER_NO_STATUS		0   /* No status code */

#define EMCER_ILL_OPCODE    	1   /* Illegal message code */
#define EMCER_BAD_UNITID	2   /* Bad unitid in CRQ or message */
#define EMCER_ILL_LBN		3   /* Illegal disk logical block address */
#define EMCER_FRAME_TOOLG	4   /* Ethernet frame too large */

#define EMCER_CHAN_EXIST	5   /* Channel already exists */
#define EMCER_NO_MORE_CHAN	6   /* No more channel available */
#define EMCER_UNIT_OFFLINE	7   /* Unit offline or inoperative */
#define EMCER_CMD_PENDING	8   /* Command still pending on channel */

#define EMCER_RECOVERED_ERROR	9   /* Some recovery action performed */
#define EMCER_DATA_ECCC		10  /* Data error ECC corrected */
#define EMCER_DATA_ECCU		11  /* Data error ECC uncorrectable */

#define EMCER_INVLD_BUFF_DESC	12  /* Invalid buffer descriptor */
#define EMCER_BAD_BUFF_ADDR	13  /* Accessing non-existent memory */
#define EMCER_HOST_MEM_PE	14  /* Main memory parity error */

#define EMCER_IOBUS_PE		15  /* I/O bus parity error */
#define EMCER_CTLR_NOT_SELECT	16  /* Controller not selected */
#define EMCER_CTLR_TIMEOUT	17  /* Controller timed out */
#define EMCER_CTLR_HW_ERROR	18  /* Controller hardware error */
#define EMCER_ILLEGAL_REQUEST	19  /* Illegal request to controller */
#define EMCER_UNIT_ATTN		20  /* Controller reset or removable medium
				       may have been changed since last
				       command */
#define EMCER_CMD_ABORTED	21  /* Command aborted by the controller */
#define EMCER_VOLUME_OVFL	22  /* A buffered peripheral device has reached
				       the End-of-Medium and data remains in
				       the buffer which has not been written
				       to the medium. */

#define EMCER_DRIVE_NOT_READY	23  /* Drive not ready */
#define EMCER_DATA_PROTECT	24  /* Data protected from operation */

#define EMCER_NO_INDEX_SIGNAL	25  /* No index signal during disk format */
#define EMCER_DRIVE_FAULT	26  /* Permanent drive fault condition */
#define EMCER_DRIVE_NOT_SELECT	27  /* Drive not selected */
#define EMCER_MULTI_DR_SELECT	28  /* More than one drive selected */

#define EMCER_FILE_MARK		29  /* File mark encountered */
#define EMCER_END_MEDIUM	30  /* End of medium reached */
#define EMCER_INCORRECT_LENGTH	31  /* Incorrect logical block length */
#define EMCER_NO_DATA		32  /* No data detected */
#define EMCER_BOT_NOT_INDICATED	33  /* BOT not indicated after Rewind, Load */

#define EMCER_SCSI_SENSE_KEY	34  /* SCSI sense key code */
#define EMCER_DTC801C_ERRCODE	35  /* DTC-801C specific error code */
#define EMCER_NCR_CTLR_ERRCODE	36  /* NCR tape controller error code */
#define EMCER_BLANK_CHECK	37  /* Encountered a blank block while
				       reading tape */
#define EMCER_TIMEOUT		40  /* Timed out by EMC */

#define EMCER_SEEK_INCOMPLETE	41  /* Seek complete signal missing */
#define EMCER_WRITE_FAULT	42  /* Drive detected failure which
				       disallows writes */
#define EMCER_ID_CRC_ERROR	43  /* ID field could not be recovered by
				       retry */
#define EMCER_ID_AM_NOT_FOUND	44  /* Missing sector pulse */
#define EMCER_RECORD_NOT_FOUND	45  /* Could not locate the desired sector */
#define EMCER_SEEK_ERROR	46  /* Could not seek to track with
				       correct ID */
#define EMCER_BAD_FORMAT	47  /* Format failed or no valid format on
				       drive */
#define EMCER_ADP48_ERRCODE	50  /* NCR ADP-48 specific error code */

#define EMCER_CTLR_MALFUNC	51  /* Controller malfunctions */
#define EMCER_8031_MALFUNC	52  /* 8031 malfunctions */
#define EMCER_SCSI_CHIP_MALFUNC  53  /* SCSI controller chip malfunctions */
#define EMCER_BAD_DR_TYPE	54  /* Bad disk drive type */

#define EMCER_DATA_REWRITTEN	55  /* IO controller re-wrote data to correct */
				    /* error */
#define EMCER_BLOCK_REMAPPED	56  /* The IO controller issued a */
				    /* reassign block command */


/* emc_msg_t:  format of first part of all EMC messages/commands */

typedef struct emc_msg {			/* Standard header for Ethernet
						and Masstore messages */
	crq_msg_t   em_msg_hdr;		/* Standard crq message header */
	short	em_status_code;		/* Command status code */
	short	em_dev_errcode;		/* Device specific error code */
	long	em_compltn_cnt;		/* Completion count (bytes or else)*/
	long	em_xtnd_status;		/* Extended status */
} emc_msg_t;

/* emc_atn_msg_t: format of all EMC attention messages */

typedef struct emc_atn_msg {
	crq_msg_t	emc_atn_hdr;
	long		emc_atn_status;	    /* Attention status */
	long		emc_atn_xtnd_stat;  /* Extended attention status */
} emc_atn_msg_t;

/* Most Attention Messages will be sent to the Requestor Channel with the
   following exceptions, which will be sent to the Unit Channel:

	EMCATN_SCSI_ILLMSG - if the unit channel exists
	EMCATN_SCSI_SENSE_ERROR
*/

#define EMCATN_POWER_FAIL	1   /* NBI Powerfail signal asserted */
#define EMCATN_CPU_RAM_PE	2   /* EMC CPU RAM Read parity error */
#define EMCATN_CPU_LOCAL_PE	3   /* EMC CPU Local Bus Read parity error */
#define EMCATN_CPU_NBI_PE	4   /* EMC CPU NBI Read parity error */
#define EMCATN_CPU_BAD_ADDR	5   /* EMC CPU bad Nanobus Memory Address */
#define EMCATN_CPU_LAN_PE	6   /* EMC CPU LAN Memory Read parity error */
#define EMCATN_LAN_CTLR_PE	7   /* 82586 LAN Memory Read parity error */
#define EMCATN_SCSI_FIFO_PE	8   /* SCSI Data FIFO Output parity error */
#define EMCATN_LOCAL_BUS_PE	9   /* EMC Local Bus parity error with Abort */
#define EMCATN_NBI_NO_GRANT	10  /* NBI No Grant with Timeout */
#define EMCATN_NBI_NOT_ACCEPTED	11  /* NBI Address not Accepted or
				       Stallwrite with Timeout */
#define EMCATN_NBI_RW_DATA	12  /* NBI Write Data parity error or no
				       Read Acknowledge with Timeout */
#define EMCATN_NBI_PROTOCOL	13  /* NBI Protocol or Logic error with
				       Timeout */
#define EMCATN_NBI_DLW_READ	14  /* NBI Double Long Word error */
				    /* xtnd_stat = NBI Master for all above */

#define EMCATN_CASCADE_INT	21  /* Cascade interrupt in non-cascade mode */
#define EMCATN_ILL_VECT_INT	22  /* Illegal Vector interrupt from ICU,
				       xtnd_stat = ICU IR No. */
#define EMCATN_ILL_TRAP		23  /* Illegal Trap, xtnd_stat = Trap Vector */
#define EMCATN_ILL_DEV_VECT	24  /* Illegal slave vector received,
				       xtnd_stat = vector */

#define EMCATN_SCSI_ILLMSG	31  /* Illegal SCSI message, xtnd_stat =
				       1st four message bytes */
#define EMCATN_SCSI_SENSE_ERROR	32  /* Error on a SCSI Request Sense command,
				       xtnd_stat = EMC Unit Number */
#define EMCATN_SCSI_BUS_PE	33  /* SCSI Bus parity error */
#define EMCATN_SCSI_CPU_TIMEOUT	34  /* SCSI CPU Handshaking Timeout */
#define EMCATN_SCSI_CPU_FAIL	35  /* SCSI CPU failed self test */

#define EMCATN_82586_SCB_TMO	41  /* 82586 SCB interrupt acknowledgement
				       timeout */
#define EMCATN_82586_CMD_TMO	42  /* 82586 Action Command completion
				       timeout */
#define EMCATN_82586_CMD_ERR	43  /* 82586 Action Command completed with
				       error */
