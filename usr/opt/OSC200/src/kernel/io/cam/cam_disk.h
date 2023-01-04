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
 * @(#)$RCSfile: cam_disk.h,v $ $Revision: 1.1.11.2 $ (DEC) $Date: 1993/07/27 17:38:43 $
 */
#ifndef _CAM_DISK_
#define _CAM_DISK_

/************************************************************************
 *
 *  cam_disk.h		Version 1.00 			April 18, 1991
 *
 *  This file contains the definitions and data structures for the 
 *  CAM disk driver.
 *
 *  MODIFICATION HISTORY:
 *
 *  VERSION  DATE	WHO	REASON
 *
 *  1.00     01/26/91	maria	Created from Disk Driver Func. Spec.
 *
 *  1.01     07/29/91	maria	Added rz wrapper mapping macros.
 *				Added ds_dkn for logging statistical info.
 *				Removed soft/hard error counters.
 *
 *  1.02     07/31/91   dallas  Added Version numbers to the structures
 *                              for the error logger
 *
 *  1.03     08/13/91   maria	Incrmented the CD_RETRY_RECOVERY define.
 *
 *  1.04     09/03/91   maria	Added OSF specific fields to disk
 *				specific structure.
 *				Added per partition open counter.
 *
 *  1.05     09/05/91   maria	Removed OSF specific fields from disk
 *				specific structure.
 *				Name changes for defines.
 *
 *  1.06     09/16/91   maria	Fixed wrapper macros for 4th
 *				controller.
 *
 *  1.07     11/19/91   maria	Moved peripheral flags and BBR defines
 *				from cam_disk.c.
 *
 *  1.08     12/16/91   maria	Added start unit retry count define.
 *
 ************************************************************************/

#define CD_NUM_PARTITIONS 	8

#define CD_RETRY_RECOVERY	10	/* Retry count for CCBs during the */
					/* recovery process. */
#define CD_RETRY_MODE_SEL 	5	/* Retry count for each mode */
					/* select page */
#define CD_RETRY_START_UNIT 	3	/* Retry count for start unit cmd */
#define CD_RETRY_MODE_SEN 	2	/* Retry count for mode sense cmd */
#define CD_RETRY_READ_CAP	2	/* Retry count for read capacity cmd */
#define CD_RETRY_IO		5	/* Retry for read and write cmds */

/*
 * Disk Specific Structure Definition:
 */
typedef struct disk_specific  {
	struct buf *ds_bufhd;		/* Anchor for requests which come */
					/* into strategy that cannot be */
					/* started due to error recovery */
					/* in progresss. */
	int	   ds_dkn;		/* Used for system statistics */
	U32	   ds_bbr_state;	/* Used indicate the current */
					/* BBR state if active */
	U32	   ds_bbr_retry;	/* BBR retries for reassignment */
	u_char	   *ds_bbr_buf;		/* Points to read/write and */
					/* reassign data buffer */
	CCB_SCSIIO *ds_bbr_rwccb;	/* R/W ccb used for BBR */
	CCB_SCSIIO *ds_bbr_reasccb;	/* Reassign ccb used for BBR */
	CCB_SCSIIO *ds_bbr_origccb;	/* Ccb which encountered bad block */
	CCB_SCSIIO *ds_tur_ccb;		/* SCSI I/O CCB for tur cmd */
					/* during recovery */
	CCB_SCSIIO *ds_start_ccb;	/* SCSI I/O CCB for start unit */
					/* cmd during recovery */
	CCB_SCSIIO *ds_mdsel_ccb;	/* SCSI I/O CCB for mode select */
					/* cmd during recovery */
	CCB_SCSIIO *ds_rdcp_ccb;	/* SCSI I/O CCB for read capacity */
					/* cmd during recovery */
	CCB_SCSIIO *ds_read_ccb;	/* SCSI I/O CCB for Read cmd */
					/* during recovery */
	CCB_SCSIIO *ds_prev_ccb;	/* SCSI I/O CCB for Prevent */
					/* Media Removal cmd during recovery */
	U32	   ds_block_size;	/* This units block size */
	U32	   ds_tot_size;		/* Total disk size in blocks */
	U32	   ds_media_changes;	/* Number of times media was */
					/* changed - removables */
	struct pt  ds_pt;		/* Partition structure */
	U32	   ds_openpart;		/* Bit mask of open parts */
	U32	   ds_bopenpart;	/* No of block opens */
	U32	   ds_copenpart;	/* No of char opens */
	U32	   ds_wlabel;		/* Write enable label */
	struct disklabel ds_label;
	PDRV_WS	   *ds_bbr_wait_queue;	
	u_char	   ds_raid_level;	/* RAID level (-1 can't deteminer) */
}DISK_SPECIFIC;

#define DISK_SPECIFIC_VERS      0x02	/* Please remember to increment the
					 * version number when you change
					 * the structure
					 */

/*
 * Peripheral device structure pd_flags bit definitions used by this
 * driver.
 */
#define PD_NO_DEVICE	0x01	/* Indicates device was opened with */
				/* F_NODELAY and the open failed */
#define PD_ONLINE	0x02	/* Indicates device ready and online */
 				/* ie partition table has been read */
#define PD_REC_INPROG	0x04	/* Indicates that the device has */
				/* encountered an error condition & */
				/* error recovery is in progress. */
#define PD_REC_START	0x08	/* Indicate recovery has started ie */
				/* first recovery ccb has been sent */
#define PD_REC_PEND	0x10	/* Indicates that recovery must be */
				/* restarted during the recovery */
				/* process. ie an error condition has */
				/* occurred during recovery processing */
				/* which requires recovery to restart */
#define PD_REC_TIMEOUT	0x20	/* Indicates that a timeout was issued */
				/* to retry a ccb during the recovery */
				/* process */
#define PD_OPENRAW	0x40	/* No part info, therefore open raw 
				 * (part a) */
#define PD_SYNCH	0x80	/* Indicates that we have sent a cmd */
				/* to initiate synchronous - set on */
				/* very first open */
#define PD_OFFLINE	0x100	/* Indicates device ready but not online */
				/* ie no floppy inserted into drive */
/*
 * The following defines are used to indicate the recovery states seen.
 */
#define PD_REC_TUR		0x01000
#define PD_REC_ST_UNIT		0x02000
#define PD_REC_MODE_SEL		0x04000
#define PD_REC_READ_CAP		0x08000
#define PD_REC_READ		0x10000
#define PD_REC_PREV_ALLOW	0x20000
/*
 * Device is in maint. mode (RAID)
 */
#define PD_MAINT 		0x100000	/* RAID LUN in maint mode. */

/*
 * The following are the defines used for Bad Block Recovery.
 */
#define CD_BBR_READ		0x01	/* BBR read state */
#define CD_BBR_REASSIGN		0x02	/* BBR reassigning state */
#define CD_BBR_REASSIGN_NO_READ	0x04	/* BBR reassigning state */
#define CD_BBR_WRITE		0x08	/* BBR write */
#define CD_BBR_DONE		0x10	/* BBR completed state */
#define CD_BBR_RETRY		2	/* Number of BBR retries */
#define CD_BBR_NO_ERROR		0	/* BBR completed sucessfully */
#define CD_BBR_ERROR		1	/* BBR has failed */

#define CD_ECC_CORRECTABLE	0x18	/* sense code for ECC */
					/* correctable recovered errors */
#define CD_NO_MEDIA		0x3a	/* sense code for no media  present */
/*
 * Retrieve the partition number from the major/minor number
 */
#define CD_GET_PARTITION(dev)	((GETDEVS(dev)) & 0x03F)

#define RAID_MINOR	0xfffff		/* Special for raid */

#endif /* _CAM_DISK_ */
