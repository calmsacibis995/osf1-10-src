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
 * @(#)$RCSfile: scsi_processor.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/03 15:23:33 $
 */
/*
 */
#ifndef _SCSI_PROC_
#define _SCSI_PROC_

/* ---------------------------------------------------------------------- */

/* scsi_processor.h

   Contains all the structures and defines for the SCSI II spec,
   chapter 11, processor devices.
*/

/* ---------------------------------------------------------------------- */

/* 
 * Receive defines.
 */

/* 
 * The receive cdb 6 bytes
 */
typedef struct proc_receive_cdb {
	u_char	opcode;		/* 0x08					*/
	u_char		:5,	/* 5 bits reservered			*/
		lun	:3;	/* Logical unit number			*/
	u_char	alloc_len2;	/* Parameter length (MSB).		*/
	u_char	alloc_len1;	/* Parameter length (MID).		*/
	u_char	alloc_len0;	/* Parameter length (LSB).		*/
	u_char	control;	/* Control byte				*/
}PROC_RECEIVE_CDB;

/*
 * The opcode for receive
 */
#define PROC_RECEIVE_OP	0x08

/* ---------------------------------------------------------------------- */

/* 
 * Send defines.
 */

/* 
 * The send cdb 6 bytes
 */
typedef struct proc_send_cdb {
	u_char	opcode;		/* 0x0a					*/
	u_char	aen	:1,	/* Indicates normal data format		*/
			:4,	/* 4 bits reserveed			*/
		lun	:3;	/* Logical unit number			*/
	u_char	alloc_len2;	/* Parameter length (MSB).		*/
	u_char	alloc_len1;	/* Parameter length (MID).		*/
	u_char	alloc_len0;	/* Parameter length (LSB).		*/
	u_char	control;	/* Control byte				*/
}PROC_SEND_CDB;

/*
 * The opcode for send
 */
#define PROC_SEND_OP	0x0a

#endif /* _SCSI_PROC_ */
