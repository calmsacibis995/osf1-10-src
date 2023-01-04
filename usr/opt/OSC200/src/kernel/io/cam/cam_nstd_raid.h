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
 * @(#)$RCSfile: cam_nstd_raid.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/18 19:17:49 $
 */


/*************************************************************************
 *
 *	cam_nstd_raid.h		Version 1.00		July 2, 1993
 *
 *	This file contains the definitions and data structures for the
 *	Non-standard RAID services which are part of the CAM Disk driver.
 *
 *	MODIFICATION HISTORY:
 *
 *	VERSION	DATE	WHO	REASON
 *
 *	1.00	07/02/93 DeLeo	Created from the Services Design Spec.
 *
 *************************************************************************/

#ifndef _CAM_NSTD_RAID_H_
#define _CAM_NSTD_RAID_H_


/* 
 * Defines
 */

#define NSTD_MAX_CMD		16	/* MAX SCSI CMD LENGTH	 	*/
#define NSTD_DATA_MASK		0x0000000000000003
#define NSTD_ERR_MASK		0xe000000000000000

/* ----------------------------------------------------------------------- */
/* nstd_flags definitions */
#define NSTD_DATA_RESV		0x0000000000000000	/* Reserved code	*/
#define NSTD_DATA_IN		0x0000000000000001	/* target to host	*/
#define NSTD_DATA_OUT		0x0000000000000002	/* host to target	*/
#define NSTD_DATA_NONE		0x0000000000000003	/* No data		*/
#define NSTD_DEV_ERR		0x8000000000000000	/* Non good scsi status	*/
#define NSTD_REF_ERR		0x4000000000000000	/* Not allowed 		*/
#define NSTD_CAM_ERR		0x2000000000000000	/* Some sort of cam err */


/*
 * Non Standard RAID Structures Declarations - Please note all are typedef'd
 */

typedef struct nstd_raid {
    U64 nstd_opcode;		/* Command (config,maint,etc)		*/
    U64 nstd_subop;		/* Configure Logical Vol etc.		*/
    U64 nstd_directed_lun;	/* Lun to which cmd applies to		*/
    U64 nstd_flags;		/* Flags that apply (errors)		*/
    U64 nstd_bus;		/* Bus number of target			*/
    U64 nstd_target;		/* Target number of controller		*/
    U64 nstd_lun;		/* Lun number that command is sent to	*/
    U64 nstd_scsi_stat;		/* Scsi command status			*/
    U64 nstd_error;		/* Error status (EIO ENXIO etc)		*/
    U64 nstd_cmd_timeo;		/* Time out value for scsi cmd in secs.	*/
    U64 nstd_cmd_len;		/* SCSI Command length			*/
    U64 nstd_total_data_area;	/* Combine length of data & sense areas	*/
    U64 nstd_data_len;		/* Length of data area if it applies	*/
    U64 nstd_data_resid;	/* SCSI command residual count request 
				 * - actual				*/
    U64 nstd_sns_offset;	/* Offset into data area for sns data	*/
    U64 nstd_sns_len;		/* Sense area length			*/
    U64 nstd_sense_resid;	/* Residual sense count requested - 
				 * actual				*/
    U8  nstd_cmd[NSTD_MAX_CMD];	/* Actual scsi command			*/
    U8  nstd_data_area[1];	/* cmd data area and sense data area 
				 * actually Longer depending on what the 
				 * operation is				*/

}NSTD_RAID;

#define	NSTD_RAID_VERS		0x01	/* Please remember to increment 
					 * the version number if you 
					 * change the structure...
					 */

/* 
 * Struct returned for NSTD_GET_CTRL_LIST operation
 */

typedef struct raid_ctrl {
    U64 ctrl_bus;		/* Bus number which target resides		*/
    U64 ctrl_target;		/* Target number which this controller is at	*/
    U8  ctrl_vid[8];		/* Vendor id from inquiry		*/
    U8  ctrl_pid[16];		/* Product id from inquiry 		*/
}RAID_CTRL;

/* 
 * Opcode and subopcode defines
 */
/* Opcodes	*/
#define NSTD_GET_CTRL_NUM	0x000001	/* Get Num RAID controllers	*/
#define NSTD_GET_CTRL_LIST	0x000002	/* Get RAID controllers list	*/
#define NSTD_CONFIG		0x000003	/* RAID Configuration Cmd	*/
#define NSTD_MAINT		0x000004	/* RAID Maintenance Cmd		*/

/* Subopcode that go with NSTD_CONFIG	(Configuration Cmds) */
#define SUB_GET_PHYS		0x000001	/* Get physical devices on box	*/
#define SUB_SET_PHYS		0x000002	/* Set physical devices on box	*/
#define SUB_CREATE_LV		0x000003	/* Create a logical vol (no lun)*/
#define SUB_CREATE_LV_LUN	0x000004	/* Create a logical vol /lun	*/
#define SUB_CREATE_LUN		0x000005	/* Create a LUN 		*/
#define SUB_DELETE_LV		0x000006	/* Delete a logical vol (no lun)*/
#define SUB_DELETE_LV_LUN	0x000007	/* Delete a logical vol/lun	*/
#define SUB_DELETE_LUN		0x000008	/* Delete a LUN			*/
#define SUB_MODIFY_LV		0x000009	/* Modify a logical vol (no lun)*/
#define SUB_MODIFY_LUN		0x000010	/* Modify LUN & LV		*/
#define SUB_GET_LUN_STAT	0x000011	/* Get status of a lun		*/
#define SUB_GET_LV_STAT		0x000012	/* Get status of a Logical Vol	*/
#define SUB_RECON_LV		0x000013	/* Reconstruct a Logical Vol.	*/
#define SUB_CREATE_SPARE_SET	0x000014	/* Create a spare set for LV's	*/
#define SUB_DELETE_SPARE_SET	0x000015	/* Delete a spare set for LV's  */
#define SUB_MODIFY_SPARE_SET	0x000016	/* Modify a existing spare set	*/
#define SUB_GET_SPARE_STAT	0x000017	/* Get a spare set's status	*/

/*  Subopcode that go with NSTD_MAINT (Maintenance Cmds) */
#define SUB_GET_DEVICE_STAT	0x000001	/* Get a phys device(s) status	*/
#define SUB_SET_DEVICE_STAT	0x000002	/* Set a phys device(s) status  */
#define SUB_FORMAT_LV		0x000003	/* Format a Logical vol.	*/
#define SUB_GET_CTRL_STAT	0x000004	/* Get a controllers status	*/
#define SUB_SET_CTRL_STAT	0x000005	/* Set a controllers status 	*/
#define SUB_MARK_CTRL_FAIL	0x000006	/* Mark a controller down	*/
#define SUB_MARK_CTRL_ALIVE	0x000007	/* Mark a controller alive 	*/
#define SUB_SET_PHYS_DEV_MAINT	0x000008	/* Marks dev/lun as maint ops	*/
#define SUB_CLEAR_PHYS_DEV_MAINT 0x000009	/* Clear dev/lun as maint ops   */
#define SUB_MAINT_PASS		0x000010	/* Cmd is a maint cmd		*/
#define SUB_GET_REDUND_CTRL	0x000011	/* Gets redundant cntrl info	*/
#define SUB_SET_REDUND_CTRL	0x000012	/* Set up redundant ctrl info	*/


#endif
