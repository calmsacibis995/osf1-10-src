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
 * @(#)$RCSfile: uagt.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/06/25 09:11:32 $
 */
#ifndef _UAGT_INCL_
#define _UAGT_INCL_

/************************************************************************
 *
 *  uagt.h		Version 1.00 			June 20, 1990
 *
 *  This file contains the definitions and data structures for the User
 *  Agent Device Driver (UAGT) that controls access to the CAM Subsystem.
 *
 *  MODIFICATION HISTORY:
 *
 *  VERSION  DATE	WHO	REASON
 *
 *  1.00     06/20/90	jag	Created from User Agent Func. Spec.
 *
 *  1.01     10/18/90	maria	Added the count fields to the
 *				UAGT_CAM_CCB structure.
 *
 *  1.03     11/19/91	maria	Added new ioctl to scan a bus or nexus
 *				and a new data structure for these ioctls.
 *				Added a flags field to the UAGT_CAM_CCB
 *				for indicating non-interruptable sleep.
 *
 ************************************************************************/


/* 
 * This data structure is used for the communication process between the
 * User Agent Device Driver and the user level programs needing to have access
 * to the CAM SCSI subsystem.  It will be copied into kernel space as part of
 * of the ioctl(2) call from user space.  The user program must fill in the
 * various pointers in this structure and the user agent driver will correctly
 * fill in the corisponding pointers in the CCB.  From the CAM SCSI subsystem's
 * point of view the request will have come from the UAgt driver.
 */

typedef struct uagt_cam_ccb
{
    CCB_HEADER *uagt_ccb;		/* pointer to the users CCB */
    U32 uagt_ccblen;			/* length of the users CCB */
    u_char *uagt_buffer;		/* pointer for the data buffer */
    U32 uagt_buflen;			/* length of user request */
    u_char *uagt_snsbuf;		/* pointer for the sense buffer */
    U32 uagt_snslen;			/* length of user's sense buffer */
    CDB_UN *uagt_cdb;			/* ptr for a CDB if not in CCB */
    U32 uagt_cdblen;			/* CDB length if appropriate */
    U32 uagt_flags;			/* See below */
} UAGT_CAM_CCB;

typedef struct uagt_cam_scan  {
	u_char	ucs_bus;		/* Bus id for scan */
	u_char	ucs_target;		/* Target id for scan */
	u_char	ucs_lun;		/* LUN for scan */
} UAGT_CAM_SCAN;

/*
 * Uagt_flags field defines.
 */
#define UAGT_NO_INT_SLEEP	0x01	/* Indicates that the user agent */
					/* driver should not sleep at an */
					/* interruptable priority. */
					/* The default is to sleep at */
					/* an interruptable priority. */

/*
 * Ioctl() define for the calls to the UAgt driver. 
 */
#define	UAGT_CAM_IO		_IOWR('s', 00, struct uagt_cam_ccb)
#define	UAGT_CAM_SINGLE_SCAN	_IOWR('s', 01, struct uagt_cam_scan)
#define	UAGT_CAM_FULL_SCAN	_IOWR('s', 02, struct uagt_cam_scan)

#endif /* _UAGT_INCL_ */
