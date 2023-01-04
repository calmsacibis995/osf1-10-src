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
 * @(#)$RCSfile: scsi_special.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/26 16:25:10 $
 */
#if !defined(SCSI_SPECIAL_INCLUDE)
#define SCSI_SPECIAL_INCLUDE 1

/************************************************************************
 *									*
 * File:	scsi_special.h						*
 * Date:	May 7, 1991						*
 * Author:	Robin T. Miller						*
 *									*
 * Description:								*
 *	Definitions for CAM/SCSI special I/O control interface.		*
 *									*
 * Modification History:						*
 *									*
 * August 3, 1991 by Robin Miller.					*
 *	Added field sp_sense_resid for sense data resdiual count and	*
 *	changed field sp_sense_length from int to u_char.		*
 *									*
 * July 20, 1991 by Robin Miller.					*
 *	Added fields to scsi_special structure to allow device major/	*
 * minor number, logical unit number, and bus/target/lun information	*
 * to be specified.  This would permit this I/O control command to be	*
 * used with the User Agent interface if desired.			*
 *									*
 ************************************************************************/

/*
 * Define Special Argument Control Flags (Also See 'h/cam_special.h'):
 */
#define SA_NO_ERROR_RECOVERY	0x01	/* Don't perform error recovery	*/
#define SA_NO_ERROR_LOGGING	0x02	/* Don't log error messages.	*/
#define SA_NO_SLEEP_INTR	0x04	/* Don't allow sleep interrupts	*/

/*
 * Structure for Processing Special I/O Control Commands.
 */
typedef struct scsi_special {
	U32	sp_flags;		/* The special command flags.	*/
	dev_t	sp_dev;			/* Device major/minor number.	*/
	u_char	sp_unit;		/* Device logical unit number.	*/
	u_char	sp_bus;			/* SCSI host adapter bus number	*/
	u_char	sp_target;		/* SCSI device target number.	*/
	u_char	sp_lun;			/* SCSI logical unit number.	*/
	u_int	sp_sub_command;		/* The sub-command (see below).	*/
	U32	sp_cmd_parameter;	/* Command parameter (if any).	*/
	int	sp_iop_length;		/* Parameters buffer length.	*/
	caddr_t	sp_iop_buffer;		/* Parameters buffer address.	*/
	u_char	sp_sense_length;	/* Sense data buffer length.	*/
	u_char	sp_sense_resid;		/* Sense data residual count.	*/
	caddr_t	sp_sense_buffer;	/* Sense data buffer address.	*/
	int	sp_user_length;		/* User data buffer length.	*/
	caddr_t	sp_user_buffer;		/* User data buffer address.	*/
	int	sp_timeout;		/* Timeout for this command.	*/
	u_char	sp_retry_count;		/* Retrys performed on command.	*/
	u_char	sp_retry_limit;		/* Times to retry this command.	*/
	int	sp_xfer_resid;		/* Transfer residual count.	*/
} T_SCSI_SPECIAL;

#define SCSI_SPECIAL		_IOWR('p', 100, struct scsi_special)

/*
 * Structure for data transfer commands (I/O parameters):
 *
 * NOTE:  The command specific flags are expected to be defined with
 *	  the actual SCSI structure in the appropriate include file.
 */
typedef struct data_transfer_params {
	u_char	dt_cmd_flags;		/* The command specific flags.	*/
	/*
	 * Inquiry Data (may be necessary to implement commands):
	 */
	u_char	dt_inq_dtype	: 5,	/* The peripheral device type.	*/
		dt_inq_pqual	: 3;	/* The peripheral qualifier.	*/
	u_char	dt_inq_rdf	: 4,	/* The response data format.	*/
				: 4;	/* Reserved.			*/
	u_char			: 8;	/* Reserved.			*/
	/*
	 * Common Device Fields:
	 */
	u_int	dt_block_size;		/* The device block size.	*/
	/*
	 * Direct-Access Devices:
	 */
	u_int	dt_lba;			/* The logical block address.	*/
	/*
	 * Sequential-Access Devices:
	 */
	u_char	dt_density_code;	/* The density code.		*/
	u_char	dt_compress_code;	/* The data compression code.	*/
	u_char	dt_speed_setting;	/* The tape speed setting.	*/
	u_char	dt_buffered_setting;	/* The buffer control setting.	*/
	/*
	 * Additional Parameters (if any):
	 */
	u_int	dt_device_depend[10];	/* Device dependent parameters.	*/
} DATA_TRANSFER_PARAMS;

/*
 * Defines to Clarify Field Usage:
 */
#define dt_record_size	dt_block_size	/* For sequential-access device	*/

/*
 * Special I/O Control Sub-Commands.
 */
#define SCMD_CHANGE_DEFINITION			1
#define SCMD_COMPARE				2
#define SCMD_COPY				3
#define SCMD_COPY_VERIFY			4
#define SCMD_ERASE				5
#define SCMD_ERASE_10				6
#define SCMD_ERASE_12				7
#define SCMD_EXCHANGE_MEDIUM			8
#define SCMD_FORMAT				9
#define SCMD_FORMAT_UNIT			10
#define SCMD_GET_DATA_BUFFER_STATUS		11
#define SCMD_GET_MESSAGE_10			12
#define SCMD_GET_MESSAGE_12			13
#define SCMD_GET_MESSAGE_6			14
#define SCMD_GET_WINDOW				15
#define SCMD_INITIALIZE_ELEMENT_STATUS		16
#define SCMD_INQUIRY				17
#define SCMD_LOAD_UNLOAD			18
#define SCMD_LOCATE				19
#define SCMD_LOCK_UNLOCK_CACHE			20
#define SCMD_LOG_SELECT				21
#define SCMD_LOG_SENSE				22
#define SCMD_MEDIUM_SCAN			23
#define SCMD_MODE_SELECT_10			24
#define SCMD_MODE_SELECT_6			25
#define SCMD_MODE_SENSE_10			26
#define SCMD_MODE_SENSE_6			27
#define SCMD_MOVE_MEDIUM			28
#define SCMD_OBJECT_POSITION			29
#define SCMD_PAUSE_RESUME			30
#define SCMD_PLAY_AUDIO_10			31
#define SCMD_PLAY_AUDIO_12			32
#define SCMD_PLAY_AUDIO_MSF			33
#define SCMD_PLAY_AUDIO_TRACK_INDEX		34
#define SCMD_PLAY_TRACK_RELATIVE_10		35
#define SCMD_PLAY_TRACK_RELATIVE_12		36
#define SCMD_POSITION_TO_ELEMENT		37
#define SCMD_PREFETCH				38
#define SCMD_PREVENT_ALLOW_REMOVAL		39
#define SCMD_PRINT				40
#define SCMD_READ				41
#define SCMD_READ_SCANNER			42
#define SCMD_READ_10				43
#define SCMD_READ_12				44
#define SCMD_READ_6				45
#define SCMD_READ_BLOCK_LIMITS			46
#define SCMD_READ_BUFFER			47
#define SCMD_READ_CAPACITY			48
#define SCMD_READ_DEFECT_DATA			49
#define SCMD_READ_DEFECT_DATA_10		50
#define SCMD_READ_DEFECT_DATA_12		51
#define SCMD_READ_ELEMENT_STATUS		52
#define SCMD_READ_GENERATION			53
#define SCMD_READ_HEADER			54
#define SCMD_READ_LONG				55
#define SCMD_READ_POSITION			56
#define SCMD_READ_REVERSE			57
#define SCMD_READ_SUBCHANNEL			58
#define SCMD_READ_TOC				59
#define SCMD_READ_UPDATED_BLOCK			60
#define SCMD_REASSIGN_BLOCKS			61
#define SCMD_RECEIVE				62
#define SCMD_RECEIVE_DIAGNOSTIC			63
#define SCMD_RECOVER_BUFFERED_DATA		64
#define SCMD_RELEASE				65
#define SCMD_RELEASE_UNIT			66
#define SCMD_REQUEST_SENSE			67
#define SCMD_REQUEST_VOLUME_ELEMENT_ADDRESS	68
#define SCMD_RESERVE				69
#define SCMD_RESERVE_UNIT			70
#define SCMD_REWIND				71
#define SCMD_REZERO_UNIT			72
#define SCMD_SCAN				73
#define SCMD_SEARCH_DATA_EQUAL			74
#define SCMD_SEARCH_DATA_EQUAL_10		75
#define SCMD_SEARCH_DATA_EQUAL_12		76
#define SCMD_SEARCH_DATA_HIGH			77
#define SCMD_SEARCH_DATA_HIGH_10		78
#define SCMD_SEARCH_DATA_HIGH_12		79
#define SCMD_SEARCH_DATA_LOW			80
#define SCMD_SEARCH_DATA_LOW_10			81
#define SCMD_SEARCH_DATA_LOW_12			82
#define SCMD_SEEK_10				83
#define SCMD_SEEK_6				84
#define SCMD_SEND				85
#define SCMD_SEND_SCANNER			86
#define SCMD_SEND_DIAGNOSTIC			87
#define SCMD_SEND_MESSAGE_10			88
#define SCMD_SEND_MESSAGE_12			89
#define SCMD_SEND_MESSAGE_6			90
#define SCMD_SEND_VOLUME_TAG			91
#define SCMD_SET_LIMITS				92
#define SCMD_SET_LIMITS_10			93
#define SCMD_SET_LIMITS_12			94
#define SCMD_SET_WINDOW				95
#define SCMD_SLEW_PRINT				96
#define SCMD_SPACE				97
#define SCMD_START_STOP_UNIT			98
#define SCMD_STOP_PRINT				99
#define SCMD_SYNCHRONIZE_BUFFER			100
#define SCMD_SYNCHRONIZE_CACHE			101
#define SCMD_TEST_UNIT_READY			102
#define SCMD_UPDATE_BLOCK			103
#define SCMD_VERIFY				104
#define SCMD_VERIFY_TAPE			105
#define SCMD_VERIFY_10				106
#define SCMD_VERIFY_12				107
#define SCMD_WRITE				108
#define SCMD_WRITE_10				109
#define SCMD_WRITE_12				110
#define SCMD_WRITE_6				111
#define SCMD_WRITE_BUFFER			112
#define SCMD_WRITE_FILEMARKS			113
#define SCMD_WRITE_LONG				114
#define SCMD_WRITE_SAME				115
#define SCMD_WRITE_VERIFY			116
#define SCMD_WRITE_VERIFY_10			117
#define SCMD_WRITE_VERIFY_12			118

#endif /* !defined(SCSI_SPECIAL_INCLUDE) */
