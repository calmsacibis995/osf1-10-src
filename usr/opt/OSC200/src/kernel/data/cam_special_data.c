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

/************************************************************************
 *									*
 * File:	cam_special_data.c (Special Command Tables)		*
 * Date:	April 1, 1991						*
 * Author:	Robin Miller						*
 *									*
 * Description:								*
 *	These tables control processing of I/O control commands.  A	*
 * command MUST exist in one of these tables or it won't get processed.	*
 * These tables allow common processing, buffer allocation, and error	*
 * reporting for all I/O control commands.				*
 *									*
 * Modification History:						*
 *									*
 * January 16, 1992 by Robin Miller.					*
 *	Changed magtape ERASE command timeout to infinite.  This was	*
 *	done since the TZ85 takes approximately an hour to erase since	*
 *	it does	not support a bulk erase (does one track at a time).	*
 *									*
 * December 23, 1991 by Robin Miller.					*
 *	Increased timeouts for nearly all SCSI commands.  It appears	*
 *	my initial worst case timeout estimates were too low for some	*
 *	devices, especially during the error recovery process.		*
 *									*
 * December 2, 1991 by Robin Miller.					*
 *	Increased magtape command timeouts from 3 minutes to 5 minutes.	*
 *	The original command timeouts were too short for the Exabyte.	*
 *									*
 * November 27, 1991 by Robin Miller.					*
 *	Add entries for magtape load/unload & online/offline commands.	*
 *									*
 * August 24, 1991 by Robin Miller.					*
 *	Add entries for direct read & write commands.			*
 *									*
 * August 13, 1991 by Robin Miller.					*
 *	Modify #include directives to permit user applications to use	*
 *	this data file (changed "../h/..." format to <sys/...>).	*
 *									*
 * August 3, 1991 by Robin Miller.					*
 *	Rearrange fields spc_device_type and spc_cmd_code so data is	*
 *	 aligned on longwords.						*
 *	Convert cam_SpecialCmdsHdr to true doubly linked list.		*
 *	Correctly setup the spc_device_type fields for each entry.	*
 *	Changed DTYPE_ALL_DEVICES to ALL_DEVICE_TYPES definition.	*
 *									*
 * July 30, 1991 by Robin Miller.					*
 *	Added read & write buffer commands.				*
 *									*
 * June 27, 1991 by Robin Miller.					*
 *	Change SCSI structures & operation codes to new definitions.	*
 *									*
 * June 22, 1991 by Robin Miller.					*
 *	Remove setup function for Reassign Blocks command.  This isn't	*
 * needed since the rzdisk utility only reassigns one block at a time.	*
 *									*
 ************************************************************************/

#include <io/common/iotypes.h>
#include <sys/param.h>
#include <io/common/devio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>

#include <io/cam/cdrom.h>
#include <sys/mtio.h>
#include <io/cam/rzdisk.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#include <io/cam/scsi_rodirect.h>
#include <io/cam/scsi_sequential.h>
#include <io/cam/scsi_special.h>

/*
 * External Declarations:
 */
extern int scmn_DoGetSense();
extern int scmn_DoCdTocHeader(), scmn_DoCdSetAddressFormat();

extern int scmn_MakeInquiry(), scmn_MakeModeSelect(), scmn_MakeModeSense();
extern int scmn_MakeReceiveDiagnostic(), scmn_MakeSendDiagnostic();
extern int scmn_MakeReadBuffer(), scmn_MakeWriteBuffer(), scmn_SetupWriteBuffer();

extern int scmn_MakeReadDirect(), scmn_MakeWriteDirect();
extern int scmn_MakeReadSequential(), scmn_MakeWriteSequential();
extern int scmn_MakeFormatUnit(), scmn_MakeReadDefectData();
extern int scmn_MakeSeekPosition(), scmn_MakeVerifyDirect();

extern int scmn_MakeCdPlayAudioLBA();
extern int scmn_MakeCdPlayAudioMSF(), scmn_MakeCdPlayAudioTI();
extern int scmn_MakeCdPlayAudioTR(), scmn_MakeCdReadTOC();
extern int scmn_MakeCdReadSubChannel(), scmn_MakeCdReadHeader();
extern int scmn_MakeCdPlayTrack(), scmn_MakeCdPlayback();
extern int scmn_MakeCdSetAddressFormat();

extern int scmn_MakeEraseOptical();

extern int scmn_MakeMtSpace(), scmn_MakeMtWriteFileMark();
extern int scmn_MakeVerifySequential();

extern int scmn_SetupFormatUnit(), scmn_SetupReadDefectData();
extern int scmn_SetupModeSense(), scmn_SetupModeSelect();
extern int scmn_SetupInquiry(), scmn_SetupSeekPosition();
extern int scmn_SetupReadLong(), scmn_SetupWriteLong();
extern int scmn_SetupSendDiagnostic(), scmn_SetupReceiveDiagnostic();
extern int scmn_SetupCdTocEntrys(), scmn_SetupCdReadSubChannel();
extern int scmn_SetupCdReadHeader();
extern int scmn_SetupCdPlayback();

/*
 * Local Definitions:
 */
#define ON		1		/* Turn a bit ON. */
#define OFF		0		/* Turn a bit OFF. */
#define ONE_MINUTE	60		/* Seconds in 1 minute. */
#define SPC_DEFAULT_TIMEOUT ONE_MINUTE	/* Default command timeout. */

/*
 * Prototype CDB's for Simple Direct Access I/O Commands.
 */
struct dir_prevent_cdb6 AllowRemoval_cdb = {
    /* opcode = */	DIR_PREVENT_OP,			/* lun = */	0,
    /* prevent = */	OFF
};

struct dir_prevent_cdb6 PreventRemoval_cdb = {
    /* opcode = */	DIR_PREVENT_OP,			/* lun = */	0,
    /* prevent = */	ON
};

struct dir_release_cdb6 ReleaseUnit_cdb = {
    /* opcode = */	DIR_RELEASE_OP,		/* extent = */	0,
    /* third_dev_id = */ 0,			/* third_pat = */ 0,
    /* lun = */		0,			/* reserve_id = */ 0,
};

struct dir_reserve_cdb6 ReserveUnit_cdb = {
    /* opcode = */	DIR_RESERVE_OP,		/* extent = */	0,
    /* third_dev_id = */ 0,			/* third_pat = */ 0,
    /* lun = */		0,			/* reserve_id = */ 0,
};

struct dir_start_cdb6 StartUnit_cdb = {
    /* opcode = */	DIR_START_OP,		/* immed = */	OFF,
    /* lun = */		0,			/* start = */	ON
};

struct dir_start_cdb6 StopUnit_cdb = {
    /* opcode = */	DIR_START_OP,		/* immed = */	OFF,
    /* lun = */	0,				/* start = */	OFF
};

struct dir_start_cdb6 EjectCaddy_cdb = {
    /* opcode = */	DIR_START_OP,		/* immed = */	ON,
    /* lun = */		0,			/* start = */	OFF,
    /* loej = */	ON
};

/*
 * Prototype CDB's for Simple Sequential Access I/O Commands:
 */
struct seq_erase_cdb6 EraseTape_cdb = {
    /* opcode = */	SEQ_ERASE_OP,	/* longe = */	ON /* <- QIC */
};

struct seq_load_cdb6 LoadTape_cdb = {
    /* opcode = */	SEQ_LOAD_OP,	/* immed = */	ON,
    /* lun = */		0,		/* load = */	ON,
    /* reten = */	OFF,		/* eot = */	OFF
};

struct seq_load_cdb6 OnlineTape_cdb = {
    /* opcode = */	SEQ_LOAD_OP,	/* immed = */	OFF,
    /* lun = */		0,		/* load = */	ON,
    /* reten = */	OFF,		/* eot = */	OFF
};

struct seq_load_cdb6 OfflineTape_cdb = {
    /* opcode = */	SEQ_LOAD_OP,	/* immed = */	OFF,
    /* lun = */		0,		/* load = */	OFF,
    /* reten = */	OFF,		/* eot = */	OFF
};

struct seq_load_cdb6 UnloadTape_cdb = {
    /* opcode = */	SEQ_LOAD_OP,	/* immed = */	ON,
    /* lun = */		0,		/* load = */	OFF,
    /* reten = */	OFF,		/* eot = */	OFF
};

struct seq_load_cdb6 RetentionTape_cdb = {
    /* opcode = */	SEQ_LOAD_OP,	/* immed = */	OFF,
    /* lun = */		0,		/* load = */	ON,
    /* reten = */	ON,		/* eot = */	OFF
};

struct seq_space_cdb6 SpaceEORM_cdb = {
    /* opcode = */	SEQ_SPACE_OP,	/* code = */	SEQ_SPACE_ENDDATA
};

/*
 * Prototype CDB's for Simple CD-ROM Audio Commands.
 */
struct CdPauseResume_CDB CdPausePlay_cdb = {
    /* opcode = */	RODIR_PAUSE_RESUME,	/* lun = */	0,
    /* resume = */	OFF
};

struct CdPauseResume_CDB CdResumePlay_cdb = {
    /* opcode = */	RODIR_PAUSE_RESUME,	/* lun = */	0,
    /* resume = */	ON
};

/************************************************************************
 *									*
 *			Special Generic Command Table			*
 *									*
 ************************************************************************/
struct special_cmd cam_GenericCmds[] = {
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_TEST_UNIT_READY,			/* spc_sub_command */
	SPC_SUB_COMMAND,			/* spc_cmd_flags */
	ALL_TUR_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"test unit ready"			/* spc_cmdp */
    },
    {	SCSI_GET_INQUIRY_DATA,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	ALL_INQ_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeInquiry,			/* spc_mkcdb */
	scmn_SetupInquiry,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"inquiry"				/* spc_cmdp */
    },
    {	SCSI_MODE_SELECT,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYIN | SPC_DATA_OUT),		/* spc_cmd_flags */
	ALL_MODE_SEL6_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeModeSelect,			/* spc_mkcdb */
	scmn_SetupModeSelect,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"mode select"				/* spc_cmdp */
    },
    {	SCSI_MODE_SENSE,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	ALL_MODE_SENSE6_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeModeSense,			/* spc_mkcdb */
	scmn_SetupModeSense,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"mode sense"				/* spc_cmdp */
    },
    {	SCSI_GET_SENSE,				/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	0,					/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	scmn_DoGetSense,			/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"get sense"				/* spc_cmdp */
    },
    {	SCSI_SEND_DIAGNOSTIC,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYIN | SPC_DATA_OUT),		/* spc_cmd_flags */
	ALL_SEND_DIAGNOSTIC_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeSendDiagnostic,		/* spc_mkcdb */
	scmn_SetupSendDiagnostic,		/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"send diagnostic"			/* spc_cmdp */
    },
    {	SCSI_RECEIVE_DIAGNOSTIC,		/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	ALL_RECEIVE_DIAGNOSTIC_OP,		/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeReceiveDiagnostic,		/* spc_mkcdb */
	scmn_SetupReceiveDiagnostic,		/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"receive diagnostic"			/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_READ_BUFFER,			/* spc_sub_command */
	(SPC_DATA_IN | SPC_SUB_COMMAND),	/* spc_cmd_flags */
	ALL_READ_BUFFER_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeReadBuffer,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read buffer"				/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_WRITE_BUFFER,			/* spc_sub_command */
	(SPC_DATA_OUT | SPC_SUB_COMMAND),	/* spc_cmd_flags */
	ALL_WRITE_BUFFER_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	(3 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeWriteBuffer,			/* spc_mkcdb */
	scmn_SetupWriteBuffer,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"write buffer"				/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_RELEASE,				/* spc_sub_command */
	SPC_SUB_COMMAND,			/* spc_cmd_flags */
	DIR_RELEASE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &ReleaseUnit_cdb,		/* spc_cdbp */
	"release unit"				/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_RESERVE,				/* spc_sub_command */
	SPC_SUB_COMMAND,			/* spc_cmd_flags */
	DIR_RESERVE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &ReserveUnit_cdb,		/* spc_cdbp */
	"reserve unit"				/* spc_cmdp */
    },
    { END_OF_CMD_TABLE }	/* End of cam_GenericCmds[] Table. */
};

/************************************************************************
 *									*
 *		    Special Direct Access Command Table			*
 *									*
 ************************************************************************/
struct special_cmd cam_DirectCmds[] = {
    {	SCSI_FORMAT_UNIT,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYIN | SPC_DATA_OUT),		/* spc_cmd_flags */
	DIR_FORMAT_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* spc_device_type */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	-1,					/* spc_data_length */
	(240 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeFormatUnit,			/* spc_mkcdb */
	scmn_SetupFormatUnit,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"format unit"				/* spc_cmdp */
    },
    {	SCSI_REASSIGN_BLOCK,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_OUT,				/* spc_cmd_flags */
	DIR_REASSIGN_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* spc_device_type */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"reassign block"			/* spc_cmdp */
    },
    {	SCSI_READ_DEFECT_DATA,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	DIR_READ_DEFECT_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* spc_device_type */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeReadDefectData,		/* spc_mkcdb */
	scmn_SetupReadDefectData,		/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read defect data"			/* spc_cmdp */
    },
    {	SCSI_VERIFY_DATA,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_VERIFY_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(15 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeVerifyDirect,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"verify data"				/* spc_cmdp */
    },
    {	SCSI_READ_LONG,				/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	DIR_READ_LONG_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* spc_device_type */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	scmn_SetupReadLong,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read long data"			/* spc_cmdp */
    },
    {	SCSI_WRITE_LONG,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYIN | SPC_DATA_OUT),		/* spc_cmd_flags */
	DIR_WRITE_LONG_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* spc_device_type */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	-1,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	scmn_SetupWriteLong,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"write long data"			/* spc_cmdp */
    },
    {	SCSI_READ_CAPACITY,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_IN,				/* spc_cmd_flags */
	DIR_READCAP_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read capacity"				/* spc_cmdp */
    },
    {	SCSI_STOP_UNIT,				/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_START_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	DIR_STOP_UNIT,				/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	(25 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &StopUnit_cdb,		/* spc_cdbp */
	"stop unit"				/* spc_cmdp */
    },
    {	SCSI_START_UNIT,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_START_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	DIR_START_UNIT,				/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(3 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &StartUnit_cdb,		/* spc_cdbp */
	"start unit"				/* spc_cmdp */
    },
    {	CDROM_EJECT_CADDY,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_START_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &EjectCaddy_cdb,		/* spc_cdbp */
	"eject caddy/media"			/* spc_cmdp */
    },
    {	SCSI_ALLOW_REMOVAL,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_PREVENT_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	DIR_ALLOW_REMOVAL,			/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &AllowRemoval_cdb,		/* spc_cdbp */
	"allow removal"				/* spc_cmdp */
    },
    {	SCSI_PREVENT_REMOVAL,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_PREVENT_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	DIR_PREVENT_REMOVAL,			/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &PreventRemoval_cdb,		/* spc_cdbp */
	"prevent removal"			/* spc_cmdp */
    },
    {	SCSI_SEEK_POSITION,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_SEEK10_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeSeekPosition,			/* spc_mkcdb */
	scmn_SetupSeekPosition,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"seek position"				/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_READ,				/* spc_sub_command */
	(SPC_DATA_IN | SPC_SUB_COMMAND),	/* spc_cmd_flags */
	0,					/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeReadDirect,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"direct read"				/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_WRITE,				/* spc_sub_command */
	(SPC_DATA_OUT | SPC_SUB_COMMAND),	/* spc_cmd_flags */
	0,					/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeWriteDirect,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"direct write"				/* spc_cmdp */
    },
    { END_OF_CMD_TABLE }	/* End of cam_DirectCmds[] Table. */
};

/************************************************************************
 *									*
 *		  Special CD-ROM Audio Access Command Table		*
 *									*
 ************************************************************************/
struct special_cmd cam_AudioCmds[] = {
    {	CDROM_PAUSE_PLAY,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PAUSE_RESUME,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	RODIR_AUDIO_PAUSE,			/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &CdPausePlay_cdb,		/* spc_cdbp */
	"pause play"				/* spc_cmdp */
    },
    {	CDROM_RESUME_PLAY,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PAUSE_RESUME,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	RODIR_AUDIO_RESUME,			/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &CdResumePlay_cdb,		/* spc_cdbp */
	"resume play"				/* spc_cmdp */
    },
    {	CDROM_PLAY_AUDIO,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PLAY_AUDIO_10,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayAudioLBA,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"play audio LBA"			/* spc_cmdp */
    },
    {	CDROM_PLAY_AUDIO_MSF,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PLAY_AUDIO_MSF,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayAudioMSF,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"play audio msf"			/* spc_cmdp */
    },
    {	CDROM_PLAY_AUDIO_TI,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PLAY_AUDIO_TRACK_INDEX,		/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayAudioTI,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"play audio track/index"		/* spc_cmdp */
    },
    {	CDROM_PLAY_AUDIO_TR,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PLAY_TRACK_RELATIVE_10,		/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayAudioTR,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"play track relative"			/* spc_cmdp */
    },
    {	CDROM_TOC_HEADER,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_IN,				/* spc_cmd_flags */
	RODIR_READ_TOC,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	sizeof(struct cd_toc_header),		/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	scmn_DoCdTocHeader,			/* spc_docmd */
	scmn_MakeCdReadTOC,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read TOC"				/* spc_cmdp */
    },
    {	CDROM_TOC_ENTRYS,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	RODIR_READ_TOC,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdReadTOC,			/* spc_mkcdb */
	scmn_SetupCdTocEntrys,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read TOC entry"			/* spc_cmdp */
    },
    {	CDROM_READ_SUBCHANNEL,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	RODIR_READ_SUBCHANNEL,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdReadSubChannel,		/* spc_mkcdb */
	scmn_SetupCdReadSubChannel,		/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read sub-channel"			/* spc_cmdp */
    },
    {	CDROM_READ_HEADER,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	RODIR_READ_HEADER,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdReadHeader,			/* spc_mkcdb */
	scmn_SetupCdReadHeader,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"read header"				/* spc_cmdp */
    },
    {	CDROM_PLAY_VAUDIO,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PLAY_VAUDIO,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayAudioLBA,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"vendor play audio"			/* spc_cmdp */
    },
    {	CDROM_PLAY_MSF,				/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PLAY_MSF,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayAudioMSF,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"vendor play msf"			/* spc_cmdp */
    },
    {	CDROM_PLAY_TRACK,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_PLAY_TRACK,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayTrack,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"play track"				/* spc_cmdp */
    },
    {	CDROM_PLAYBACK_CONTROL,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYIN | SPC_DATA_OUT),		/* spc_cmd_flags */
	RODIR_PLAYBACK_CONTROL,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	-1,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayback,			/* spc_mkcdb */
	scmn_SetupCdPlayback,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"playback control"			/* spc_cmdp */
    },
    {	CDROM_PLAYBACK_STATUS,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	(SPC_COPYOUT | SPC_DATA_IN),		/* spc_cmd_flags */
	RODIR_PLAYBACK_STATUS,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	-1,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeCdPlayback,			/* spc_mkcdb */
	scmn_SetupCdPlayback,			/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"playback status"			/* spc_cmdp */
    },
    {	CDROM_SET_ADDRESS_FORMAT,		/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	RODIR_SET_ADDRESS_FORMAT,		/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	RODIR_DEFAULT_TIMEOUT,			/* spc_timeout */
	scmn_DoCdSetAddressFormat,		/* spc_docmd */
	scmn_MakeCdSetAddressFormat,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"set address format"			/* spc_cmdp */
    },
    { END_OF_CMD_TABLE }	/* End of cam_AudioCmds[] Table. */
};

/************************************************************************
 *									*
 *		Special Sequential Access Command Table			*
 *									*
 ************************************************************************/
struct special_cmd cam_SequentialCmds[] = {
    {	SCSI_ALLOW_REMOVAL,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_PREVENT_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	DIR_ALLOW_REMOVAL,			/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &AllowRemoval_cdb,		/* spc_cdbp */
	"allow removal"				/* spc_cmdp */
    },
    {	SCSI_PREVENT_REMOVAL,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	DIR_PREVENT_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	DIR_PREVENT_REMOVAL,			/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &PreventRemoval_cdb,		/* spc_cdbp */
	"prevent removal"			/* spc_cmdp */
    },
    {	SCSI_LOAD_UNIT,				/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_LOAD_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"load unit"				/* spc_cmdp */
    },
    {	SCSI_UNLOAD_UNIT,			/* spc_ioctl_cmd */
	0,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_LOAD_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	SPC_DEFAULT_TIMEOUT,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"unload unit"				/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_VERIFY_TAPE,			/* spc_sub_command */
	(SPC_DATA_NONE | SPC_SUB_COMMAND),	/* spc_cmd_flags */
	SEQ_VERIFY_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	CAM_TIME_INFINITY,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeVerifySequential,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"verify tape"				/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_READ,				/* spc_sub_command */
	(SPC_DATA_IN | SPC_SUB_COMMAND),	/* spc_cmd_flags */
	SEQ_READ_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_IN,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(60 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeReadSequential,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"sequential read"			/* spc_cmdp */
    },
    {	SCSI_SPECIAL,				/* spc_ioctl_cmd */
	SCMD_WRITE,				/* spc_sub_command */
	(SPC_DATA_OUT | SPC_SUB_COMMAND),	/* spc_cmd_flags */
	SEQ_WRITE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_OUT,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	(60 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeWriteSequential,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"sequential write"			/* spc_cmdp */
    },
    { END_OF_CMD_TABLE }	/* End of cam_Sequential[] Table. */
};

/************************************************************************
 *									*
 *			Special 'mt' Command Table			*
 *									*
 ************************************************************************/
struct special_cmd cam_MtCmds[] = {
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTERASE,				/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_ERASE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	CAM_TIME_INFINITY,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &EraseTape_cdb,		/* spc_cdbp */
	"erase tape"				/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTLOAD,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_LOAD_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(5 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &LoadTape_cdb,		/* spc_cdbp */
	"load tape"				/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTONLINE,				/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_LOAD_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(5 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &OnlineTape_cdb,		/* spc_cdbp */
	"online (load) tape"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTOFFL,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_LOAD_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(5 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &OfflineTape_cdb,		/* spc_cdbp */
	"offline (unload) tape"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTUNLOAD,				/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_LOAD_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(5 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &UnloadTape_cdb,		/* spc_cdbp */
	"unload tape"				/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTREW,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_REWIND_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(5 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"rewind tape"				/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTRETEN,				/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_LOAD_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	(5 * ONE_MINUTE),			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &RetentionTape_cdb,		/* spc_cdbp */
	"retention tape"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTSEOD,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_SPACE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	CAM_TIME_INFINITY,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	(int (*)()) 0,				/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) &SpaceEORM_cdb,		/* spc_cdbp */
	"space to end of media"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTBSF,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_SPACE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	CAM_TIME_INFINITY,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeMtSpace,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"backward space file"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTBSR,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_SPACE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	CAM_TIME_INFINITY,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeMtSpace,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"backward space record"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTFSF,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_SPACE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	CAM_TIME_INFINITY,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeMtSpace,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"forward space file"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTFSR,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_SPACE_OP,				/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FREAD,					/* spc_file_flags */
	0,					/* spc_data_length */
	CAM_TIME_INFINITY,			/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeMtSpace,			/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"forward space record"			/* spc_cmdp */
    },
    {	MTIOCTOP,				/* spc_ioctl_cmd */
	MTWEOF,					/* spc_sub_command */
	SPC_DATA_NONE,				/* spc_cmd_flags */
	SEQ_WRITEMARKS_OP,			/* spc_cmd_code */
	0,					/* RESERVED */
	0,					/* spc_device_type */
	0,					/* spc_cmd_parameter */
	CAM_DIR_NONE,				/* spc_cam_flags */
	FWRITE,					/* spc_file_flags */
	0,					/* spc_data_length */
	ONE_MINUTE,				/* spc_timeout */
	(int (*)()) 0,				/* spc_docmd */
	scmn_MakeMtWriteFileMark,		/* spc_mkcdb */
	(int (*)()) 0,				/* spc_setup */
	(caddr_t) 0,				/* spc_cdbp */
	"write file mark"			/* spc_cmdp */
    },
    { END_OF_CMD_TABLE }	/* End of cam_MtCmds[] Table. */
};

/************************************************************************
 *									*
 *			Special Command Table Headers			*
 *									*
 ************************************************************************/
struct special_header cam_SpecialCmdsHdr = {
	&cam_SpecialCmdsHdr,			/* sph_flink */
	&cam_SpecialCmdsHdr,			/* sph_blink */
	(struct special_cmd *) 0,		/* sph_cmd_table */
	0,					/* sph_device_type */
	0,					/* sph_table_flags */
	"Head of Special Commands Table"	/* sph_queue_name */
};

struct special_header cam_GenericCmdsHdr = {
	(struct special_header *) 0,		/* sph_flink */
	(struct special_header *) 0,		/* sph_blink */
	cam_GenericCmds,			/* sph_cmd_table */
	ALL_DEVICE_TYPES,			/* sph_device_type */
	0,					/* sph_table_flags */
	"Generic Commands"			/* sph_table_name */
};

struct special_header cam_DirectCmdsHdr = {
	(struct special_header *) 0,		/* sph_flink */
	(struct special_header *) 0,		/* sph_blink */
	cam_DirectCmds,				/* sph_cmd_table */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* sph_device_type */
	 BITMASK(ALL_DTYPE_OPTICAL) |
	 BITMASK(ALL_DTYPE_RODIRECT)),
	0,					/* sph_table_flags */
	"Direct Access Commands"		/* sph_table_name */
};

struct special_header cam_AudioCmdsHdr = {
	(struct special_header *) 0,		/* sph_flink */
	(struct special_header *) 0,		/* sph_blink */
	cam_AudioCmds,				/* sph_cmd_table */
	BITMASK(ALL_DTYPE_RODIRECT),		/* sph_device_type */
	0,					/* sph_table_flags */
	"CD-ROM Audio Commands"			/* sph_table_name */
};

struct special_header cam_SequentialCmdsHdr = {
	(struct special_header *) 0,		/* sph_flink */
	(struct special_header *) 0,		/* sph_blink */
	cam_SequentialCmds,			/* sph_cmd_table */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* sph_device_type */
	0,					/* sph_table_flags */
	"Sequential Access Commands"		/* sph_table_name */
};

struct special_header cam_MtCmdsHdr = {
	(struct special_header *) 0,		/* sph_flink */
	(struct special_header *) 0,		/* sph_blink */
	cam_MtCmds,				/* sph_cmd_table */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* sph_device_type */
	SPH_SUB_COMMAND,			/* sph_table_flags */
	"'mt' Commands"				/* sph_table_name */
};

/*
 * Define Special Commands Header & Table for Initialization Routine.
 */
struct special_header *cam_SpecialCmds = &cam_SpecialCmdsHdr;

struct special_header *cam_SpecialHdrs[] =
	{ &cam_GenericCmdsHdr, &cam_DirectCmdsHdr, &cam_AudioCmdsHdr,
	  &cam_SequentialCmdsHdr, &cam_MtCmdsHdr, 0 };
