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
 * @(#)$RCSfile: scsi_rodirect.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/06/25 09:02:58 $
 */
#if !defined(SCSI_RODIRECT_INCLUDE)
#define SCSI_RODIRECT_INCLUDE 1

/************************************************************************
 *									*
 * File:	scsi_rodirect.h						*
 * Date:	June 27, 1991						*
 * Author:	Robin T. Miller						*
 *									*
 * Description:								*
 *	This file contains the read-only direct-access definitions.	*
 *									*
 * Modification History:						*
 *									*
 ************************************************************************/

/*
 * CD-ROM Command Parameter Definitions:
 */
#define RODIR_DEFAULT_TIMEOUT	10	/* Default command timeout.	*/

/*
 * Pause/Resume Audio Playback Parameters:
 */
#define RODIR_AUDIO_PAUSE	0	/* Pause audio playback.	*/
#define RODIR_AUDIO_RESUME	1	/* Resume audio playback.	*/

/*
 * SCSI Operation Codes for CD-ROM Devices.
 */
#define RODIR_CHANGE_DEFINITION			0x40
#define RODIR_COMPARE				0x39
#define RODIR_COPY				0x18
#define RODIR_COPY_VERIFY			0x3A
#define RODIR_INQUIRY				0x12
#define RODIR_LOCK_UNLOCK_CACHE			0x36
#define RODIR_LOG_SELECT			0x4C
#define RODIR_LOG_SENSE				0x4D
#define RODIR_MODE_SELECT_6			0x15
#define RODIR_MODE_SELECT_10			0x55
#define RODIR_MODE_SENSE_6			0x1A
#define RODIR_MODE_SENSE_10			0x5A
#define RODIR_PAUSE_RESUME			0x4B
#define RODIR_PLAY_AUDIO_10			0x45
#define RODIR_PLAY_AUDIO_12			0xA5
#define RODIR_PLAY_AUDIO_MSF			0x47
#define RODIR_PLAY_AUDIO_TRACK_INDEX		0x48
#define RODIR_PLAY_TRACK_RELATIVE_10		0x49
#define RODIR_PLAY_TRACK_RELATIVE_12		0xA9
#define RODIR_PREFETCH				0x34
#define RODIR_PREVENT_ALLOW_REMOVAL		0x1E
#define RODIR_READ_6				0x08
#define RODIR_READ_10				0x28
#define RODIR_READ_12				0xA8
#define RODIR_READ_BUFFER			0x3C
#define RODIR_READ_CAPACITY			0x25
#define RODIR_READ_HEADER			0x44
#define RODIR_READ_LONG				0x3E
#define RODIR_READ_SUBCHANNEL			0x42
#define RODIR_READ_TOC				0x43
#define RODIR_RECEIVE_DIAGNOSTIC		0x1C
#define RODIR_RELEASE				0x17
#define RODIR_REQUEST_SENSE			0x03
#define RODIR_RESERVE				0x16
#define RODIR_REZERO_UNIT			0x01
#define RODIR_SEARCH_DATA_EQUAL_10		0x31
#define RODIR_SEARCH_DATA_EQUAL_12		0xB1
#define RODIR_SEARCH_DATA_HIGH_10		0x30
#define RODIR_SEARCH_DATA_HIGH_12		0xB0
#define RODIR_SEARCH_DATA_LOW_10		0x32
#define RODIR_SEARCH_DATA_LOW_12		0xB2
#define RODIR_SEEK_6				0x0B
#define RODIR_SEEK_10				0x2B
#define RODIR_SEND_DIAGNOSTIC			0x1D
#define RODIR_SET_LIMITS_10			0x33
#define RODIR_SET_LIMITS_12			0xB3
#define RODIR_START_STOP_UNIT			0x1B
#define RODIR_SYNCHRONIZE_CACHE			0x35
#define RODIR_TEST_UNIT_READY			0x00
#define RODIR_VERIFY_10				0x2F
#define RODIR_VERIFY_12				0xAF
#define RODIR_WRITE_BUFFER			0x3B

/*
 * Sony CDU-541 Vendor Unique Commands (Group 6).
 */
#define RODIR_SET_ADDRESS_FORMAT		0xC0
#define RODIR_PLAYBACK_STATUS			0xC4
#define RODIR_PLAY_TRACK			0xC6
#define RODIR_PLAY_MSF				0xC7
#define RODIR_PLAY_VAUDIO			0xC8
#define RODIR_PLAYBACK_CONTROL			0xC9

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Pause/Resume Command Descriptor Block:
 */
typedef struct CdPauseResume_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char		: 8;		/* Reserved.			[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char		: 8;		/* Reserved.			[7] */
	u_char	resume	: 1,		/* Resume = 1, Pause = 0.	[8] */
			: 7;		/* Reserved.			    */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_PAUSE_RESUME_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Play Audio LBA Command Descriptor Block:
 */
typedef struct CdPlayAudioLBA_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	reladr	: 1,		/* Relative Address bit		[1] */
			: 4,		/* Reserved			    */
		lun	: 3;		/* Logical Unit Number		    */
	u_char	lbaddr3;		/* Logical Block Address	[2] */
	u_char	lbaddr2;		/* Logical Block Address	[3] */
	u_char	lbaddr1;		/* Logical Block Address	[4] */
	u_char	lbaddr0;		/* Logical Block Address	[5] */
	u_char		: 8;		/* Reserved			[6] */
	u_char	xferlen1;		/* Transfer Length    		[7] */
	u_char	xferlen0;		/* Transfer Length    		[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_PLAY_AUDIO_LBA_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Play Audio MSF Command Descriptor Block:
 */
typedef struct CdPlayAudioMSF_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char	starting_M_unit;	/* Starting M-unit.		[3] */
	u_char	starting_S_unit;	/* Starting S-unit.		[4] */
	u_char	starting_F_unit;	/* Starting F-unit.		[5] */
	u_char	ending_M_unit;		/* Ending M-unit.		[6] */
	u_char	ending_S_unit;		/* Ending S-unit.		[7] */
	u_char	ending_F_unit;		/* Ending F-unit.		[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_PLAY_AUDIO_MSF_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Play Audio Track/Index Command Descriptor Block:
 */
typedef struct CdPlayAudioTI_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	starting_track;		/* Starting Track.		[4] */
	u_char	starting_index;		/* Starting Index.		[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char	ending_track;		/* Ending Track.		[7] */
	u_char	ending_index;		/* Ending Index			[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_PLAY_AUDIO_TI_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Play Audio Track Relative Command Descriptor Block:
 */
typedef struct CdPlayAudioTR_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	lbaddr3;		/* Logical Block Address	[2] */
	u_char	lbaddr2;		/* Logical Block Address.	[3] */
	u_char	lbaddr1;		/* Logical Block Address.	[4] */
	u_char	lbaddr0;		/* Logical Block Address.	[5] */
	u_char	starting_track;		/* Starting Track.		[6] */
	u_char	xfer_len1;		/* Transfer Length    		[7] */
	u_char	xfer_len0;		/* Transfer Length    		[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_PLAY_AUDIO_TR_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Read TOC Command Descriptor Block:
 */
typedef struct CdReadTOC_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 1,		/* Reserved.			[1] */
		msf	: 1,		/* Report address in MSF format.    */
			: 3,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char		: 8;		/* Reserved.			[5] */
	u_char	starting_track;		/* Reserved.			[6] */
	u_char	alloc_len1;		/* Allocation length (MSB).	[7] */
	u_char	alloc_len0;		/* Allocation length (LSB).	[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_READ_TOC_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Read Sub-Channel Command Descriptor Block:
 */
typedef struct CdReadSubChannel_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 1,		/* Reserved.			[1] */
		msf	: 1,		/* Report address in MSF format.    */
			: 3,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 6,		/* Reserved.			[2] */
		subQ	: 1,		/* Sub-Q Channel Data.		    */
			: 1;		/* Reserved.			    */
	u_char	data_format;		/* Data Format Code.		[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char		: 8;		/* Reserved.			[5] */
	u_char	track_number;		/* Reserved.			[6] */
	u_char	alloc_len1;		/* Allocation length (MSB).	[7] */
	u_char	alloc_len0;		/* Allocation length (LSB).	[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_READ_SUBCHANNEL_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Read Header Command Descriptor Block:
 */
typedef struct CdReadHeader_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 1,		/* Reserved.			[1] */
		msf	: 1,		/* Report address in MSF format.    */
			: 3,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	lbaddr3;		/* Logical Block Address	[2] */
	u_char	lbaddr2;		/* Logical Block Address.	[3] */
	u_char	lbaddr1;		/* Logical Block Address.	[4] */
	u_char	lbaddr0;		/* Logical Block Address.	[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char	alloc_len1;		/* Allocation Length MSB.	[7] */
	u_char	alloc_len0;		/* Allocation Length LSB.	[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_READ_HEADER_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Play Track Command Descriptor Block:
 */
typedef struct CdPlayTrack_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	starting_track;		/* Starting track.		[4] */
	u_char	starting_index;		/* Starting index.		[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char		: 8;		/* Reserved.			[7] */
	u_char	number_indexes;		/* Number of indexes.		[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_PLAY_TRACK_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Playback Control/Status Command Descriptor Block:
 */
typedef struct CdPlayback_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char		: 8;		/* Reserved.			[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char	alloc_len1;		/* Allocation length (MSB).	[7] */
	u_char	alloc_len0;		/* Allocation length (LSB).	[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_PLAYBACK_CDB;

/* ---------------------------------------------------------------------- */

/*
 * CD-ROM Set Address Format Command Descriptor Block:
 */
typedef struct CdSetAddressFormat_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char		: 8;		/* Reserved.			[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char		: 8;		/* Reserved.			[7] */
	u_char	lbamsf	: 1,		/* Address Format 0/1 = LBA/MSF	[8] */
			: 7;		/* Reserved.			    */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} RODIR_SET_ADDRESS_FORMAT_CDB;

#endif /* !defined(SCSI_RODIRECT_INCLUDE) */
