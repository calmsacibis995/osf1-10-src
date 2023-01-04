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
 * @(#)$RCSfile: cam_tape.h,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/07/14 18:16:38 $
 */

/* ---------------------------------------------------------------------- */

/* cam_tape.h	Version 1.00			Feb. 22, 1991

This file contains all the defines that the ctz_tape.c driver needs. The
defines are only for the tape driver (SCSI CAM) and apply to no other
CAM SCSI module.

Modification History

	Version		Date		Who 	Reason
	1.00		02/22/91	dallas	Created this module
	1.01		06/19/91	dallas	Added new defines.
	1.02		07/31/91	dallas  Addded structure Version
						numbers for the error 
						logger. Add new macro's
						for wrapper code.
	1.03		08/30/91	dallas	Added new state bit for
						loaders. Changed member
						names of ACTION struct
						to be more consistent.

	1.04		09/13/91	dallas	Added Tried sync state.
						modeule name change
						from cam_tape_define.h 
						to cam_tape.h

	1.05		09/13/91	dallas	Fixed wrapper macros.
	1.06		11/19/91	dallas  Added new state bit
						CTAPE_ABORT_IO_STATE.
						This allows us to report
						aborted ccb's not done
						by tape driver.
	1.07		12/03/91	dallas  Got rid of CTAPE_BTOL
						macro and now use 
						CTAPE_INFO for sense info
						bytes..
	1.08		01/27/91	dallas	Added error contol defines
						and expanded the defines
						for check conditions.
						


*/

#ifndef _CAM_TAPE_H_
#define _CAM_TAPE_H_
/* ---------------------------------------------------------------------- */

/* Include Files	*/
/*	None		*/






/* ---------------------------------------------------------------------- */


/* Defines		*/


/*

The following flags are used in the TAPE_SPECIFIC structure in member
ts_state_flags. The state flags are used to determine and indicate
certain states the driver and scsi tape unit is in.

*/

#define CTAPE_NOT_READY_STATE			0x00000001
	/* Indicates that the unit was opened with the FNDELAY flag and
	 * the unit had some sort of failure during open but was seen
	 */
#define CTAPE_UNIT_ATTEN_STATE			0x00000002
	/* Indicates that a check condition occurred and the sense key
	 * was UNIT ATTENTION. This usually indicates that a media change
	 * has occurred but could be power up, reset. Either way we must
	 * notice that current tape position is lost.
	 */
#define CTAPE_RESET_STATE			0x00000004
	/* Indicates that we have been notified of a reset set condition 
	 * on the device or bus.
	 */
#define CTAPE_RESET_PENDING_STATE		0x00000008
	/*
	 * A reset is pending will get notified shortly
	 */
#define	CTAPE_OPENED_STATE			0x00000010
	/*
	 * The unit is opened
	 */
#define CTAPE_DISEOT_STATE			0x00000020
	/*
	 * The user has requested no notification of end of 
	 * media.
	 */
#define CTAPE_ABORT_TPPEND_STATE		0x00000040
	/*
	 * For fixed block operations. When a tape mark is detected
	 * with nbuf I/O. We abort the queues and this is just and
	 * indication that a tp mark is being handled.
	 */
#define CTAPE_AUTO_DENSITY_VALID_STATE		0x00000080
	/* This flag used when a unit atten is noticed during the open
	 * sequence. The CTAPE_UNIT_ATTEN_STATE flag directs the open
	 * routine to call ctz_auto_density when at beginning of media
	 * the auto density valid flag thell us that tape density was
	 * determined and for all reads - read at that density.
	 */
#define CTAPE_ORPHAN_CMD_STATE			0x00000100
	/* 
	 * This flag is set when a  command is orphaned. The 
	 * process does not wait for completion. Currently the 
	 * only place where we orphan a command is the rewind
	 * on close condition. The next open of the device is 
	 * blocked until the bit is cleared.
	 */
#define CTAPE_POSITION_LOST_STATE		0x00000200
	/*
	 * POSITION is lost due to command failure.. We can't
	 * determine where we are on tape anymore. The conditons
	 * that clear the state is UNIT_ATTEN and RESET. Also the
	 * mt op's of MTREW,  MTOFFL, MTRETEN.
	 */
#define CTAPE_TRIED_ONLINE_STATE		0x00000400
	/*
	 * We found the unit to be in the NOT READY state. We have
	 * tried to bring the unit online once. 
	 */
#define CTAPE_SYNC_STATE			0x00000800
	/*
	 * Tried to do synchronous negoiation
	 */
#define CTAPE_IO_ABORT_STATE			0x00001000
	/*
	 * We are aborting the the I/O queues
	 */


/* ----------------------------------------------------------------------- */


/*

The following flags are used in the TAPE_SPECIFIC structure in member
ts_flags. The flags are used to determine and indicate certain
conditions of the tape media.

*/

#define CTAPE_BOM				0x00000001
	/* Tape is at Beginning of Media same as BOT Beginning of Tape
	 */
#define CTAPE_EOM				0x00000002
	/* At End of Tape
	 */
#define CTAPE_OFFLINE				0x00000004
	/* Indicates the the device is returning a DEVICE not READY
	 * in response to a check condition..Basically means the tape
	 * media is not loaded or is being loaded.
	 */
#define CTAPE_WRT_PROT				0x00000008
	/* Tape is either Hardware write protectioned or opened read only
	 */
#define CTAPE_BLANK				0x00000010
	/* Tape is blank (blank check)
	 */
#define CTAPE_WRITTEN				0x00000020
	/* The tape has been written this time around.
	 */
#define CTAPE_CSE				0x00000040
	/* This is a short lived flag and will probably go away
	 * CHECK_IMP
	 */
#define CTAPE_SOFTERR				0x00000080
	/* Indicates that a soft error has been reported by the tape
	 * unit.
	 */
#define CTAPE_HARDERR				0x00000100
	/* Hard error has occurred this flag can be reported back to 
	 * the user either thru and ioctl or the user gets notified 
	 * by the buf struct being marked as EIO.
	 */
#define CTAPE_DONE				0x00000200
	/* Indicates done
	 */
#define CTAPE_RETRY				0x00000400
	/* Indicates retry
	 */
#define CTAPE_ERASED				0x00000800
	/* The tape has or is erased.
	 */
#define CTAPE_TPMARK				0x00001000
	/* A tape mark has been detected on a read (can't on writes).
	 */
#define CTAPE_SHRTREC				0x00002000
	/* The user asked for 10k bytes but the record on tape is only
	 * 5k, post the short record flag. next operation clears it.
	 * Indication is retrieved by the user thru an ioctl.
	 */
#define CTAPE_RDOPP				0x00004000
	/* We are reading in the reverse direction.. Not implemented
	 * at this time.
	 */
#define CTAPE_REWINDING				0x00008000
	/* the unit is rewinding
	 */
#define CTAPE_TPMARK_PENDING			0x00010000
	/* Due to fixed block type tapes a tape mark can be reported
	 * by the unit after reading valid data blocks. We don't want
	 * to post the tpmark back to the user now but on the next read 
	 * sequencial read.


/* ----------------------------------------------------------------------- */

/*

Tape Structures Declarations - Please note all are typedef'd

*/

/* TAPE_SPECIFIC	*/

typedef struct tape_specific {
	U32	ts_flags;	/* Tape flags - BOM,EOT			*/	
	U32	ts_state_flags;	/* STATE - UNIT_ATTEN, RESET etc.	*/
	U32	ts_resid;	/* Last operation residual count	*/
	U32	ts_block_size;	/* See below for a complete desc.	*/
	U32	ts_density;	/* What density are we running at 	*/
	U32	ts_records;	/* How many records in since last tpmark*/
	U32	ts_num_filemarks; /* number of file mark into tape	*/
}TAPE_SPECIFIC;

#define	TAPE_SPECIFIC_VERS	0x01	/* Please remember to increment 
					 * the version number if you 
					 * change the structure...
					 */

/*
 *	The ts_block_size member is used for fixed block tapes like
 *	QIC. Since 9 trk units (variable) commands deal in bytes and
 *	Fixed block units deal in blocks (multiples of 512) we must
 *	know if we are dealing in bytes or blocks.. so we record 
 *	what we are running at in the member so we can do the right
 *	thing...I HOPE.
*/

/*
 * The ctape_action struct is passed down to the tape action
 * routines to be filled in based on success or failure of the
 * command.
 */
typedef struct ctape_action {
	CCB_SCSIIO 	*act_ccb;	/* The ccb that is returned 	*/
	I32		act_ret_error;	/* Error code if any		*/
	U32		act_fatal;	/* Is this considered fatal	*/
	U32		act_ccb_status;	/* The ccb status code		*/
	U32		act_scsi_status;/* The scsi error code		*/
	U32		act_chkcond_error;/* The check condition error	*/
}CTAPE_ACTION; 

/*
 * CTAPE_ACTION defines
 * action.fatal flags;
 */
#define	ACT_FAILED		0x00000001	/* This action has failed */
#define	ACT_RESOURCE		0x00000002	/* Resource problem (memory)*/
#define ACT_PARAMETER		0x00000004	/* Invalid parameter	*/
#define	ACT_RETRY_EXCEDED 	0x00000008	/* Retry the operation count
						 * count has been excede
						 */
#define ACT_INTERRUPTED		0x00000010	/* Signal interruption	*/

 
/* The dens_tbl_entry struct */

typedef struct dens_tbl_entry {
        U32             dens_code;   /* The density code             */
        U32             den_blocking;   /* The blocking factor          */
}DENS_TBL_ENTRY;



/*
 * Retry limit for scsi status of busy
 */
#define CTAPE_RETRY_LIMIT	100


/*
 * Defines for the ctape_online routine.
 */
#define OPEN_UNIT	0x00000001	/* Go thru full open 		*/
#define NO_TIME		0x00000002	/* DO wait for full ready time 
					 * of unit 
					 */

#define CTAPE_MIN_SENSE	0x00000007	/*
					 * Minimun size of sense
					 * data we need.
					 */
/*
 * CTAPE_REL_MEM will examine an CCB_SCSIIO ccb to see if
 * the data buffer pointer is non NULL. If so the marco will
 * call ccmn_rel_dbuf with the size to release the memory 
 * back to the pools.
 */
#define CTAPE_REL_MEM(ccb); { \
    if(((CCB_SCSIIO *)(ccb))->cam_data_ptr != (u_char *)NULL ) { \
	ccmn_rel_dbuf(((CCB_SCSIIO *)(ccb))->cam_data_ptr, \
				((CCB_SCSIIO *)(ccb))->cam_dxfer_len ); \
	((CCB_SCSIIO *)(ccb))->cam_data_ptr = (u_char *)NULL; \
	((CCB_SCSIIO *)(ccb))->cam_dxfer_len = (U32)NULL; \
    } \
}

/*
 * Tapes will report Null's for density if the tape
 * is blank or if it has not determined tape
 * density..
 */
#define CTAPE_DENS_DEFAULT	0x00

/*
 * Index into density table
 */
#define DEV_TO_DENS_IDX(dev)	(((dev) >> 1) & 0x7)

/*
 * Maximum I/O size.
 */
#define CTAPE_MAXPHYS	(16 * (1024 * 1024))	/* 16 meg		*/

/*
 * Our default time out value for NON read/writes (rewind,space)
 */
#define CTAPE_DEF_TIMEO	2400 	/* 40 MINIUTES */

/*
 * 5 second time
 */
#define CTAPE_TIME_5	5

/*
 * Whether we sleep in the work routines 
 * ctape_move_tape(), ctape_load(), etc.
 */
#define CTAPE_SLEEP	0x00000000
#define CTAPE_NOSLEEP	0x00000001

/*
 * Success and failure defines
 */
#define CTAPE_SUCCESS	00
#define CTAPE_FAIL	-1

/*
 * ERROR reports defines
 */
#define CTAPE_HARD_ERRS		1
#define CTAPE_ALL_ERRORS	2

/*
 * Rewind on close bit define
 */
#define CTAPE_NOREWIND_DEV	0x1

#define CTAPE_READ		1
#define CTAPE_WRITE 		2  


/*
 * Flags defines for the routines that do tape action.
 * Please NOTE THAT CMD_IMED is used all over.. Do not
 * define any command flags as 0x01
 */
#define CMD_IMED	0x1		/* Command immediate modifier	*/


/* 
 * LOAD/UNLOAD
 */
#define LOAD_CMD_UNLOAD	0x02
#define LOAD_CMD_LOAD	0x04
#define LOAD_CMD_RET	0x08
#define LOAD_CMD_EOT	0x10



/*
 * Defines for what ctape_ccb_chkcond returns
 */
#define CHK_SENSE_NOT_VALID	0x0001	/* Valid bit is not set in sense */
#define CHK_EOM			0x0002	/* End of media			*/
#define CHK_FILEMARK		0x0003	/* File mark detected		*/
#define CHK_ILI			0x0004	/* Incorrect length		*/
#define CHK_NOSENSE_BITS	0x0005	/* NOSENSE key and no bits	*/
#define CHK_RECOVER		0x0006	/* soft error reported		*/
#define CHK_NOT_READY		0x0007	/* Device is not ready		*/
#define CHK_MEDIUM_ERR		0x0008	/* Device reported hard tape error*/
#define CHK_HARDWARE		0x0009  /* Hardware error		*/
#define CHK_ILL_REQ		0x000a	/* Illegal request maynot support cmd*/
#define CHK_UNIT_ATTEN		0x000b	/* Unit attention (ready???)	*/
#define CHK_DATA_PROT		0x000c	/* write protected		*/
#define CHK_BLANK_CHK		0x000d	/* Blank check erased tape	*/
#define CHK_VENDOR_SPEC		0x000e	/* Vendor specific (no idea)	*/
#define CHK_COPY_ABORT		0x000f	/* Copy cmd aborted		*/
#define CHK_CMD_ABORTED		0x0010	/* Command has been aborted	*/
#define CHK_EQUAL		0x0011	/* Search cmd equal		*/
#define CHK_VOL_OFLOW		0x0012	/* Phys end of media, data in buf */
#define CHK_MIS_CMP		0x0013	/* Miscompare src != media	*/
#define CHK_UNKNOWN_KEY		0x0014	/* Bogus sense key		*/
#define CHK_CHK_NOSENSE		0x0015	/* Sense Auto sense valid bit 0	*/
#define CHK_INFORMATIONAL	0x0016  /* Informational message...	*/

/* 
 * The following flags are the Check conditions that the action 
 * routines are passed to say what conditions are NOT reported to the
 * error log. An example is auto density determination at unit atten
 * or bom we forward space 1 record on certain tapes if the tape is 
 * blank it reports medium error. Well it's not an error so we
 * should not report it.
 */

#define ERR_EOM			0x00000001	/* EOM			*/
#define ERR_FILEMARK		0x00000002	/* Fielmark		*/
#define ERR_ILI			0x00000004	/* Illegal lenght	*/
#define ERR_RECOV		0x00000008	/* Recovered error	*/
#define ERR_NOT_RDY		0x00000010	/* Not ready		*/
#define ERR_MEDIUM		0x00000020	/* Medium Error	(hard)	*/
#define ERR_HARDWARE		0x00000040 	/* Hardware error (device)*/
#define ERR_ILL_REQ		0x00000080	/* Illegal request	*/
#define ERR_UNIT_ATTEN		0x00000100	/* UNIT Attention	*/
#define	ERR_DATA_PROT		0x00000200	/* Write protect	*/
#define ERR_BLANK_CHK		0x00000400	/* Blank check 		*/
#define ERR_VENDOR_SPEC		0x00000800	/* We don't support	*/
#define ERR_COPY_ABORT		0x00001000	/* Copy aborted 	*/
#define ERR_CMD_ABORTED		0x00002000	/* Target aborted command*/
#define ERR_EQUAL		0x00004000	/* Searched data equal	*/
#define ERR_VOL_OFLOW		0x00008000	/* Volume overflow	*/
#define ERR_MIS_CMP		0x00010000	/* Miscompare error	*/


/*
 * Clear the fields in the CCB which will be filled in on a retry
 * of the CCB.
 */
#define CTAPE_CLEAR_CCB(ccb)					\
	(ccb)->cam_ch.cam_status = 0;			\
	(ccb)->cam_scsi_status = 0;			\
	(ccb)->cam_resid = 0;				\



#define CTAPE_INFO(sns, long_val) {		\
	union 	{				\
		unsigned char	c[4];		\
		U32		l;		\
	}tmp;					\
	if( (sns)->valid != NULL){		\
	    tmp.c[3] = (sns)->info_byte3;	\
	    tmp.c[2] = (sns)->info_byte2;	\
	    tmp.c[1] = (sns)->info_byte1;	\
	    tmp.c[0] = (sns)->info_byte0;	\
	    (long_val) = tmp.l;			\
	}					\
	else{					\
	    (long_val) = NULL;			\
	}					\
}

#define CTAPE_LOCK_OR_STATE(pd, ts, flags) {			\
	int	ipl;						\
	PDRV_IPLSMP_LOCK( (pd), LK_RETRY, ipl );		\
	(ts)->ts_state_flags |= (flags);			\
	PDRV_IPLSMP_UNLOCK( (pd), ipl );			\
}

#define CTAPE_LOCK_OR_FLAGS(pd, ts, flags) {			\
	int	ipl;						\
	PDRV_IPLSMP_LOCK( (pd), LK_RETRY, ipl );		\
	(ts)->ts_flags |= (flags);				\
	PDRV_IPLSMP_UNLOCK( (pd), ipl );			\
}

#define CTAPE_BERROR(buf , count, error ) {			\
	(buf)->b_resid = (count);				\
	(buf)->b_error = (error);				\
	(buf)->b_flags |= B_ERROR;				\
}

/*
 * If active queue contains more then the ccb in error	
 * then set state flag and abort the queues
 */
#ifdef OSF
#define CTAPE_RAWASYNC_ABORT( buf, pd )
#else
#define CTAPE_RAWASYNC_ABORT( buf, pd ) {			\
	if(((pd)->pd_active_ccb > 1) && 			\
		((BOP_IS_RAWASYNC_SET((buf)->b_flags)) != NULL)) {	\
	    ((TAPE_SPECIFIC *)(pd)->pd_specific)->ts_state_flags  |=    \
				CTAPE_IO_ABORT_STATE;		\
	    ccmn_abort_que( (pdrv_dev) );			\
	}							\
}
#endif /* OSF */


#define CTAPE_NULLCCB_ERR( act_ptr, pd, mod ) {			\
    int	ipl;							\
	PDRV_IPLSMP_LOCK( (pd), LK_RETRY, ipl );		\
	CAM_ERROR((mod), "NULL CCB returned", CAM_SOFTWARE,	\
		(CCB_HEADER *)NULL, (pd)->pd_dev, 		\
		(u_char *)NULL);				\
	PDRV_IPLSMP_UNLOCK((pd), ipl);				\
	(act_ptr)->act_fatal |= (ACT_RESOURCE | ACT_FAILED);	\
	(act_ptr)->act_ret_error = ENOMEM;			\
}

#endif
