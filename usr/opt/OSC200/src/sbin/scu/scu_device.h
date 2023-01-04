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
 * @(#)$RCSfile: scu_device.h,v $ $Revision: 1.1.11.2 $ (DEC) $Date: 1993/11/23 23:09:49 $
 */
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_device.h
 * Author:	Robin T. Miller
 * Date:	August 9, 1991
 *
 * Description:
 *	Per device structure for SCSI Utility Program.
 *
 * Modification History:
 *
 */
#include <io/common/devio.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_special.h>

/*
 * External Declarations:
 */
extern struct scu_device *ScuDevice;
extern struct scu_device *ScuDeviceList;
extern struct scu_device *ScuPrevious;

/*
 * User Settable CAM CCB Flags Mask (See h/cam.h for definitions).
 */
#define CAM_CCB_FLAGS_MASK	\
  (CAM_DIS_DISCONNECT | CAM_INITIATE_SYNC | CAM_DIS_SYNC | CAM_SIM_QHEAD | \
   CAM_SIM_QFREEZE | CAM_SIM_QFRZDIS | CAM_ENG_SYNC )

/*
 * Test Control Flag Definitions:
 */
#define TC_NO_BUFFER		0x01	/* No I/O data buffer required.	*/
#define TC_NO_DEFAULT		0x02	/* Do not default I/O length.	*/
#define TC_NO_STATISTICS	0x04	/* Do not display statistics.	*/
#define TC_NO_TIMING		0x08	/* Do not perform timing.	*/
#define TC_NO_SETUP		0x10	/* No special setup necessary.	*/

/*
 * Device Unit Structure:
 */
typedef struct scu_device {
	struct scu_device *scu_flink;	/* Forward link to next entry.	*/
	struct scu_device *scu_blink;	/* Backward link to prev entry.	*/
	int	scu_fd;			/* The device file descriptor.	*/
	int	scu_flags;		/* The device control flags.	*/
	u_char	scu_unit;		/* Device logical unit number.	*/
	u_char	scu_bus;		/* SCSI host adapter bus number	*/
	u_char	scu_target;		/* SCSI device target number.	*/
	u_char	scu_lun;		/* SCSI logical unit number.	*/
	u_long	scu_cam_flags;		/* The CAM CCB flags field.	*/
	caddr_t	scu_device_entry;	/* The device entry name.	*/
	caddr_t scu_device_name;	/* The Digital device name.	*/
	u_long	scu_device_capacity;	/* The device capacity.		*/
	u_long	scu_device_size;	/* The device block size.	*/
	u_char	scu_device_type;	/* The SCSI device type.	*/
	u_char	scu_error_control;	/* The saved error control.	*/
	u_char	scu_read_retrys;	/* The saved error read retrys.	*/
	u_char	scu_write_retrys;	/* The saved error write retrys	*/
	int	scu_file_flags;		/* The file control flags.	*/
	struct all_inq_data *scu_inquiry; /* Pointer to inquiry data.	*/
	caddr_t	scu_mode_pages[MAX_PCF_TYPES]; /* Mode pages buffers.	*/
	caddr_t	scu_default_pages[MAX_MODE_PAGES]; /* Default pages.	*/
	int	scu_data_length;	/* Command data buffer length.	*/
	caddr_t	scu_data_buffer;	/* Command data buffer address.	*/
	int	scu_iop_length;		/* Parameters buffer length.	*/
	caddr_t	scu_iop_buffer;		/* Parameters buffer address.	*/
	/*
	 * The temporary buffer is used by common support functions.
	 */
	int	scu_tmp_length;		/* Temporary buffer length.	*/
	caddr_t	scu_tmp_buffer;		/* Temporary buffer address.	*/
	u_char	scu_retry_count;	/* Retrys performed on command.	*/
	u_char	scu_retry_limit;	/* Times to retry the command.	*/
	u_char	scu_sense_length;	/* Sense data buffer length.	*/
	u_char	scu_sense_resid;	/* Sense data residual count.	*/
	caddr_t	scu_sense_buffer;	/* Sense data buffer address.	*/
	int	scu_user_length;	/* User data buffer length.	*/
	caddr_t	scu_user_buffer;	/* User data buffer address.	*/
	int	scu_timeout;		/* Timeout for this command.	*/
	int	scu_xfer_resid;		/* Transfer residual count.	*/
	struct devget scu_devget;	/* Get device information.	*/
	struct special_args *scu_sap;	/* Special argument pointer.	*/
	/*
	 * Statistics/Test Allocation:
	 */
	u_long	scu_test_control;	/* The test control flags.	*/
	u_long	scu_align_offset;	/* Align buffer at this offset.	*/
	caddr_t	scu_aligned_bptr;	/* Aligned data buffer pointer.	*/
	u_long	scu_block_count;	/* Blocks transferred count.	*/
	u_long	scu_block_limit;	/* Data transfer block limit.	*/
	u_long	scu_block_size;		/* Data block size (in bytes).	*/
	u_long	scu_buffer_mode;	/* The Read/Write Buffer mode.	*/
	u_long	scu_compare_flag;	/* Compare data control flag.	*/
	u_long	scu_data_bytes;		/* Total data bytes per pass.	*/
	u_long	scu_data_limit;		/* Total data limit per pass.	*/
	u_long	scu_data_pattern;	/* The default data pattern.	*/
	u_long	scu_delay_value;	/* Delay between I/O's value.	*/
	u_long	scu_ending_lba;		/* Ending logical block address	*/
	u_long	scu_error_code;		/* Last error code returned.	*/
	u_long	scu_error_count;	/* Number of errors detected.	*/
	u_long	scu_error_limit;	/* Number of errors tolerated.	*/
	u_long	scu_seek_position;	/* The initial seek position.	*/
	u_long	scu_last_position;	/* The last seek position.	*/
	u_long	scu_incr_count;		/* # of records to increment.	*/
	u_long	scu_logical_block;	/* The logical block number.	*/
	u_long	scu_pass_count;		/* Number of passes completed.	*/
	u_long	scu_pass_limit;		/* Default number of passes.	*/
	u_long	scu_random_flag;	/* Do random I/O disk testing.	*/
	u_long	scu_randomizing_key;	/* The randomizing key to use.	*/
	u_long	scu_record_count;	/* The # of records processed.	*/
	u_long	scu_record_limit;	/* The # of records to process.	*/
	u_long	scu_recovery_flag;	/* Error recovery control flag.	*/
	u_long	scu_partial_records;	/* # of partial records proc'ed	*/
	u_long	scu_seek_count;		/* The seek count for disks.	*/
	u_long	scu_segment_size;	/* The download segment size.	*/
	u_long	scu_skip_count;		/* The skip count for tapes.	*/
	u_long	scu_starting_lba;	/* Starting logical block.	*/
	u_long	scu_step_value;		/* Step value for disk seeks.	*/
	u_long	scu_total_bytes;	/* Total bytes transferred.	*/
	u_long	scu_total_errors;	/* Total errors (all passes).	*/
	u_long	scu_total_records;	/* Total records (all passes).	*/
	u_long	scu_warning_errors;	/* Total non-fatal error count.	*/
	u_long	scu_verify_flag;	/* Verify data written flag.	*/
	u_long	scu_verify_length;	/* The default verify length.	*/
	u_long	scu_xfer_length;	/* The data transfer length.	*/
	/*
	 * Process Storage Allocation:
	 */
	pid_t	scu_pid;		/* The child process ID.	*/
#ifdef OSF
	int	scu_wait_stat;		/* For child exit status.	*/
#else
	union wait scu_wait_stat;	/* For child exit status.	*/
#endif
	/*
	 * Elapsed Time Allocation:
	 */
	long	scu_after_time;		/* Per pass elapsed time.	*/
	long	scu_before_time;	/*  "   "      "      "		*/
	struct tms scu_after;		/*  "   "      "      "		*/
	struct tms scu_before;		/*  "   "      "      "		*/
	struct timeb scu_stime;		/* For elapsed times.		*/
	struct timeb scu_ptime;		/*  "     "      "		*/
	struct timeb scu_etime;		/*  "     "      "		*/
} SCU_DEVICE;

/*
 * Diskette Density Parameters Structure:
 */
typedef struct density_params {
	u_char	dp_heads;		/* The number of heads.		*/
	u_char	dp_sectors_track;	/* Sectors per track.		*/
	u_short	dp_data_sector;		/* Data bytes per sector.	*/
	u_short	dp_cylinders;		/* The number of cylinders.	*/
	u_char	dp_step_pulse_cyl;	/* Addl' step pulses per cyl.	*/
	u_short	dp_transfer_rate;	/* Transfer rate.		*/
} DENSITY_PARAMS;

/*
 * Floppy Diskette Format Table Values:
 */
enum density_values {
	V_FORMAT_RX50,			/* Single density RX50	 400KB	*/
	V_FORMAT_LD,			/* Low (single) density	 360KB	*/
	V_FORMAT_DD,			/* Double density	 720KB	*/
	V_FORMAT_HD,			/* High density	    3.5" 1.2MB	*/
	V_FORMAT_HD5,			/* High density   5.25" 1.44MB	*/
	V_FORMAT_ED,			/* Extra density	2.88MB	*/
	V_FORMAT_OTHER			/* Prompt for density params.	*/
};

/*
 * Sequential Access (Tape) Density Definitions:
 */
enum tape_densitys {
	T_DEFAULT_DENSITY,		/* Default density.		*/
	T_DENSITY_800_BPI,		/* Density 800 BPI   (NRZI, R)	*/
	T_DENSITY_1600_BPI,		/* Density 1600 BPI  (PE, R)	*/
	T_DENSITY_6250_BPI,		/* Density 6250 BPI  (GCR, R)	*/
	T_DENSITY_8000_BPI,		/* Density 8000 BPI  (GCR, C)	*/
	T_DENSITY_3200_BPI,		/* Density 3200 BPI  (PE, R)	*/
	T_DENSITY_6400_BPI,		/* Density 6400 BPI  (IMFM, C)	*/
	T_DENSITY_38000_BPI,		/* Density 37871 BPI (GCR, C)	*/
	T_DENSITY_6666_BPI,		/* Density 6667 BPI  (MFM, C)	*/
	T_DENSITY_12690_BPI,		/* Density 12690 BPI (GCR, C)	*/
	T_DENSITY_10000_BPI,		/* Density QIC-120/QIC-150.	*/
	T_DENSITY_16000_BPI,		/* Density QIC-320   (GCR, C)	*/
	T_DENSITY_42500_BPI,		/* Density TZ85 42500bpi mfm	*/
	T_DENSITY_61000_BPI,		/* Density 4mm Tape  (DDS, CS)	*/
	T_DENSITY_54000_BPI,		/* Density 8mm Tape  (???, CS)	*/
	T_DENSITY_QIC_24,		/* Density QIC-24    (GCR, C)	*/
	T_DENSITY_QIC_120_ECC,		/* Density QIC-120 with ECC.	*/
	T_DENSITY_QIC_150_ECC,		/* Density QIC-150 with ECC.	*/
	T_DENSITY_QIC_120,		/* Density QIC-120   (GCR, C)	*/
	T_DENSITY_QIC_150,		/* Density QIC-150   (GCR, C)	*/
	T_DENSITY_QIC_320,		/* Density QIC-320   (GCR, C)	*/
	T_DENSITY_QIC_1350,		/* Density QIC-1350  (RLL, C)	*/
	T_DENSITY_45434_BPI,    	/* 8mm Tape 8500 mode           */
	T_DENSITY_62500_BPI,    	/* 62500 BPI (MFM, CS)          */
	T_DENSITY_QIC_1G,    		/* 36000 BPI QIC-1G (GCR, C)    */
	T_DENSITY_QIC_2G,    		/* 40640 BPI QIC-2G (GCR, C)    */
	T_DENSITY_UNKNOWN		/* Unknown Tape Density.	*/
};

#define T_DENSITY_36000_BPI	T_DENSITY_QIC_1G
#define T_DENSITY_40640_BPI	T_DENSITY_QIC_2G

typedef struct tape_density_entry {
	enum tape_densitys td_density;	/* The tape density value.	*/
	char	*td_name;		/* The tape density name.	*/
} TAPE_DENSITY_ENTRY;
