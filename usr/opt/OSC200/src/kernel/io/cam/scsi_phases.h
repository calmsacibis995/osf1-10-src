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
 * @(#)$RCSfile: scsi_phases.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/12/11 15:01:20 $
 */
#ifndef _SCSIPHASES_
#define _SCSIPHASES_

/* ---------------------------------------------------------------------- */

/* scsi_phases.h	Version 1.00			Mar. 26, 1991 */

/*  This file contains the definitions as described in the SCSI-II spec,
    chapter five.

Modification History

	Version	  Date		Who	Reason

	1.00	11/09/90	janet	Created this file.
	1.01	03/26/91	janet	Changed name of file from 
					scsi_chap5.h
*/

/* ---------------------------------------------------------------------- */
/*
 * SCSI phase defines.  These phase defines are used along with
 * the SCSI State machine as indices.  That is why there definitions
 * are what they are.
 */
#define SCSI_BUS_FREE			0
#define SCSI_ARBITRATION		1
#define SCSI_SELECTION			2
#define SCSI_RESELECTION		3
#define SCSI_COMMAND			4
#define SCSI_DATAIN			5
#define SCSI_DATAOUT			6
#define SCSI_STATUS			7
#define SCSI_MSGIN			8
#define SCSI_MSGOUT			9
#define SCSI_NSTATES			10
#define SCSI_PHASEBIT(phase)		(1 << (phase))

/*
 * SCSI Message defines:
 */
#define SCSI_ABORT				0x06
#define SCSI_ABORT_TAG				0x0d
#define SCSI_BUS_DEVICE_RESET			0x0c
#define SCSI_CLEAR_QUEUE			0x0e
#define SCSI_COMMAND_COMPLETE			0x00
#define SCSI_DISCONNECT				0x04
#define SCSI_IDENTIFY				0x80
#define SCSI_INITIATE_RECOVERY			0x0f
#define SCSI_INITIATOR_DETECTED_ERROR		0x05
#define SCSI_LINKED_COMMAND_COMPLETE		0x0a
#define SCSI_LINKED_COMMAND_COMPLETE_WFLAG	0x0b
#define SCSI_MESSAGE_PARITY_ERROR		0x09
#define SCSI_MESSAGE_REJECT			0x07
#define SCSI_NO_OPERATION			0x08
#define SCSI_RELEASE_RECOVERY			0x10
#define SCSI_RESTORE_POINTERS			0x03
#define SCSI_SAVE_DATA_POINTER			0x02
#define SCSI_TERMINATE_IO_PROCESS		0x11

#define SCSI_ID_RES_BITS                        0x18  /* Reserved bits in ID msgs */

/*
 * Two byte Messages
 */
#define SCSI_IGNORE_WIDE_RESIDUE		0x23
#define SCSI_HEAD_OF_QUEUE_TAG			0x21
#define SCSI_ORDERED_QUEUE_TAG			0x22
#define SCSI_SIMPLE_QUEUE_TAG			0x20
#define SCSI_IS_MSG_TWO_BYTE(msg) 	 	(((msg) >= 0x20) && ((msg) <= 0x2f))
#define SCSI_IS_MSG_RESERVED(msg)               ((((msg) >= 0x12) && ((msg) <= 0x1f)) \
                                        || (((msg) >= 0x24) && ((msg) <= 0x2f))       \
                                        || (((msg) >= 0x30) && ((msg) <= 0x7f)))


/*
 * Extended Messages
 */
#define SCSI_EXTENDED_MESSAGE			0x01
#define SCSI_MODIFY_DATA_POINTER		0x00
#define SCSI_SYNCHRONOUS_XFER			0x01
#define SCSI_WIDE_XFER				0x03

#endif /* _SCSIPHASES_ */
