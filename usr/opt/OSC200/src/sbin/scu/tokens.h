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
 * @(#)$RCSfile: tokens.h,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/11/23 23:11:37 $
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
 * File:	scu_tokens.h
 * Author:	Robin T. Miller
 * Date:	August 9, 1991
 *
 * Description:
 *	Command/Keyword token definitions used by SCSI Utility Program.
 *
 * Modification History:
 *
 */

/*
 * Define the Parser Tokens:
 */
enum parser_tokens {

	/****************************************************************
	 *								*
	 *	     Internal Parser Ciontrol Token Definitions		*
	 * 	These tokens MUST be defined first (see 'parser.h').	*
	 *								*
	 ****************************************************************/

	PC_END_OF_TABLE,		/* End of parser table value.	*/
	PC_LAMBDA,			/* Always match (optional).	*/
	PC_SHARED_TABLE,		/* Parse shared keyword table.	*/

	/****************************************************************
	 *								*
	 *		    Command Token Definitions			*
	 *								*
	 ****************************************************************/

	C_END_OF_TABLE,			/* End of parser table value.	*/
	C_ALLOW_REMOVAL,		/* Allow medium removal.	*/
	C_CHANGE,			/* Change mode page fields.	*/
	C_DOWNLOAD,			/* Download software to device.	*/
	C_EJECT_CADDY,			/* Eject the caddy.		*/
	C_ERASE,			/* Optical erase command.	*/
	C_EVALUATE,			/* Evaluate & print expression.	*/
	C_EXIT,				/* Exit from the program.	*/
	C_FORMAT_UNIT,			/* Format a disk or diskette.	*/
	C_HELP,				/* Display help information.	*/
	C_MT,				/* Perfrom an 'mt' command.	*/
	C_PAUSE_PLAY,			/* Pause audio playing.		*/
	C_PLAY,				/* Play an audio command.	*/
	C_PREVENT_REMOVAL,		/* Prevent medium removal.	*/
	C_READ,				/* Read data blocks.		*/
	C_REASSIGN_BLOCK,		/* Reassign defective block.	*/
	C_RELEASE,			/* Release device or SIM Queue.	*/
	C_RESERVE,			/* Reserve device on a bus.	*/
	C_RESET,			/* Reset bus or device.		*/
	C_RESUME_PLAY,			/* Resume audio playing.	*/
	C_SCAN,				/* Scan SCSI devices or media.	*/
	C_SET,				/* Set parameter.		*/
	C_SHOW,				/* Show parameter.		*/
	C_SLEEP,			/* Sleep for number of seconds.	*/
	C_SOURCE,			/* Source commands from file.	*/
	C_START_UNIT,			/* Start the drive.		*/
	C_STOP_UNIT,			/* Stop the drive.		*/
	C_SKIP,				/* Skip forward/backward tracks.*/
	C_SWITCH,			/* Switch to another device.	*/
	C_TEST,				/* Perform diagnostic tests.	*/
	C_TUR,				/* Perform test unit ready.	*/
	C_UPLOAD,			/* Upload software from device.	*/
	C_VERIFY,			/* Verify (Read) device blocks.	*/
	C_WRITE,			/* Write data blocks.		*/
	C_XZA,				/* XZA-specific commands	*/
	C_LAST_TOKEN,			/* Last command token value.	*/

	/****************************************************************
	 *								*
	 *		    Keyword Token Definitions			*
	 *								*
	 ****************************************************************/

	/*
	 * CAM/SCSI Specific Keyword Tokens:
	 */
	K_EDT,				/* Equipment device table.	*/
	K_SIMQ,				/* SIM Queue Operation.		*/
	K_NEXUS,			/* Nexus information specified.	*/
	K_BUS,				/* Bus # field specified.	*/
	K_TARGET,			/* Target # field specified.	*/
	K_LUN,				/* Logical unit # specified.	*/
	K_SUBLUN,			/* Sub-LUN number specified.	*/

	/*
	 * Data I/O Parameter Keyword Tokens:
	 */
	K_ALIGN_OFFSET,			/* Data buffer alignment offset	*/
	K_BLOCK_SIZE,			/* Data block size (in bytes).	*/
	K_COMPARE_FLAG,			/* Compare data control flag.	*/
	K_DATA_LIMIT,			/* The data transfer limit.	*/
	K_DATA_PATTERN,			/* The data pattern to use.	*/
	K_DELAY_VALUE,			/* Delay between I/O's value.	*/
	K_ENDING_LBA,			/* Ending logical block address	*/
	K_ERROR_LIMIT,			/* Error limit to tolerate.	*/
	K_INCREMENT_COUNT,		/* The record increment count.	*/
	K_PASS_LIMIT,			/* The pass limit.		*/
	K_RANDOM_FLAG,			/* Do random disk I/O testing.	*/
	K_RANDOMIZING_KEY,		/* The randomizing key to use.	*/
	K_RECORD_LIMIT,			/* The # of records to process.	*/
	K_RECOVERY_FLAG,		/* Error recovery control flag.	*/
	K_RETRY_LIMIT,			/* Times to retry the command.	*/
	K_SEEK_COUNT,			/* The seek record count (disk)	*/
	K_SKIP_COUNT,			/* The skip record count (tape)	*/
	K_STARTING_LBA,			/* Starting logical block.	*/
	K_STEP_VALUE,			/* Step value for disk seeks.	*/
	K_VERIFY_FLAG,			/* Verify data written flag.	*/
	K_VERIFY_LENGTH,		/* Verify data length value.	*/
	K_WATCH_PROGRESS,		/* Watch the I/O progress flag.	*/

	/*
	 * Magtape Keyword Tokens:
	 */
	K_BSF,				/* Backward space file marks.	*/
	K_BSR,				/* Backward space file records.	*/
	K_ERASE,			/* Erase the tape.		*/
	K_FSF,				/* Forward space file marks.	*/
	K_FSR,				/* Forward space file records.	*/
	K_LOAD,				/* Load the tape.		*/
	K_OFFLINE,			/* Take tape offline (unload).	*/
	K_ONLINE,			/* Bring tape online (load).	*/
	K_REWIND,			/* Rewind the tape.		*/
	K_RETENSION,			/* Retension the tape.		*/
	K_SEOD,				/* Space to end of data.	*/
	K_UNLOAD,			/* Unload the tape.		*/
	K_WEOF,				/* Write tape file marks.	*/

	/*
	 * Show Keyword Tokens:
	 */
	K_AUDIO,			/* Show/Set audio parameters.	*/
	K_CAPACITY,			/* Show disk capacity.		*/
	K_DEFECT_LIST,			/* Show the disk defect list.	*/
	K_DEVICE_TYPE,			/* Show/Set CAM device type.	*/
	K_FLAG,				/* Show a particular flag.	*/
	K_INQUIRY,			/* Show inquiry information.	*/
	K_LIMITS,			/* Show/Set device limits.	*/
	K_MEMORY_SIZE,			/* Show controller memory size.	*/
	K_PAGES,			/* Show/Set all mode pages.	*/
	K_PATH_INQUIRY,			/* Show CAM path inquiry info.	*/
	K_SENSE,			/* Show request sense data.	*/
	K_STATUS,			/* Show current device status.	*/
	K_TAPE,				/* Show/Set tape parameters.	*/
	K_TOC,				/* Show the table of contents.	*/

	/*
	 * Show/Set Audio Keyword Tokens:
	 */
	K_CATALOG,			/* Show media catalog number.	*/
	K_CHANNEL,			/* Show sub-Q channel data.	*/
	K_DEFAULTS,			/* Set/show test defaults.	*/
	K_ISRC,				/* Show the ISRC number.	*/
	K_LBA_HEADER,			/* Show logical block header.	*/
	K_PLAY_STATUS,			/* Show the playback status.	*/
	K_PLAY_POSITION,		/* Show current play position.	*/
	K_SEEK_POSITION,		/* Seek to specified position.	*/ 
	K_VOLUME,			/* Volume control levels.	*/

	K_ADDRESS,			/* Address format (lba or msf).	*/
	K_LBA,				/* Logical Block Address format	*/
	K_MSF,				/* Miniute/Second/Frame format.	*/
	K_LENGTH,			/* Transfer length (in blocks).	*/

	/*
	 * Device Type Keyword Tokens:
	 */
	K_DTYPE_DIRECT,			/* Direct access.		*/
	K_DTYPE_SEQUENTIAL,		/* Sequential access.		*/
	K_DTYPE_PRINTER,		/* Printer.			*/
	K_DTYPE_PROCESSOR,		/* Processor.			*/
	K_DTYPE_WORM,			/* Write-Once/Read Many.	*/
	K_DTYPE_RODIRECT,		/* Read-Only direct access.	*/
	K_DTYPE_SCANNER,		/* Scanner.			*/
	K_DTYPE_OPTICAL,		/* Optical.			*/
	K_DTYPE_CHANGER,		/* Changer.			*/
	K_DTYPE_COMM,			/* Communications Device	*/

	/*
	 * Miscellaneous Keyword Tokens:
	 */
	K_BYPASS,			/* Bypass special checks.	*/
	K_CAM,				/* CAM specific parameters.	*/
	K_CAM_DEBUG,			/* CAM specific debug flags.	*/
	K_CAM_RETRYS,			/* CAM special I/O retries.	*/
	K_CCB_FLAGS,			/* CAM control block flags.	*/
	K_DEBUG,			/* Program debug mode flag.	*/
	K_DEVICE,			/* Perform operation on device.	*/
	K_DUMP,				/* Dump buffer control flag.	*/
	K_DUMP_LIMIT,			/* Dump data buffer limit.	*/
	K_LONG,				/* Perform read/write long.	*/
	K_MEDIA,			/* Scan the device media.	*/
	K_OUTPUT,			/* Output display format.	*/
	K_OUTPUT_LOG,			/* Output text to a log file.	*/
	K_OUTPUT_PAGER,			/* User defined output pager.	*/
	K_OVERPRINT_MODE,		/* Overprint in watch progress.	*/
	K_PAGING_FLAG,			/* The screen paging flag.	*/
	K_PARTITION,			/* Disk partition information.	*/
	K_REPORT_FULL,			/* Do full display format.	*/
	K_REPORT_BRIEF,			/* Do brief reporting format.	*/
	K_REPORT_SUMMARY,		/* Do summary display format.	*/
	K_STATISTICS,			/* Performance statistics flag.	*/
	K_TIMING,			/* Command timing control flag.	*/
	K_TIMEOUT,			/* Override default timeout.	*/
	K_VERBOSE,			/* The program verbosity mode.	*/

	/*
	 * XZA Keyword Tokens
	 */
	K_XZA_SHOW,			/* XZA-specific show commands	*/
	K_XZA_SET,			/* XZA-specific set commands	*/
	K_XZA_COUNTERS,			/* XZA Read Counters commnand	*/
	K_XZA_ERRORS,			/* XZA Show Errors command	*/
	K_XZA_SERR,			/* XZA Show Soft Errors		*/
	K_XZA_HERR,			/* XZA Show Hard Errors		*/
	K_XZA_PARAMS,			/* XZA Show Params command	*/
	K_XZA_SCSIID,			/* XZA SCSI id parameter	*/
	K_XZA_READ,			/* XZA Read 			*/
	K_XZA_WRITE,			/* XZA Write 			*/
	K_XZA_IMAGE,			/* XZA Read/Write Image		*/
	K_XZA_NCR,			/* XZA Show NCR_regs		*/

	/*
	 * Floppy Diskette Format Keyword Tokens:
	 */
	K_DENSITY,			/* Density specified flag.	*/
	K_FORMAT_RX50,			/* Single density RX50	 400KB	*/
	K_FORMAT_LD,			/* Low (single) density	 360KB	*/
	K_FORMAT_DD,			/* Double density	 720KB	*/
	K_FORMAT_HD,			/* High density     3.5" 1.2MB	*/
	K_FORMAT_HD5,			/* High density	  5.25" 1.44MB	*/
	K_FORMAT_ED,			/* Extra density	2.88MB	*/
	K_FORMAT_OTHER,			/* Prompt for density params.	*/

	/*
	 * Units Keyword Tokens:
	 */
	K_STARTING_UNITS,		/* Starting units specified.	*/
	K_STARTING_M_UNITS,		/* Starting minute units value.	*/
	K_STARTING_S_UNITS,		/* Starting second units value.	*/
	K_STARTING_F_UNITS,		/* Starting frame units value.	*/

	K_ENDING_UNITS,			/* Ending units specified.	*/
	K_ENDING_M_UNITS,		/* Ending minute units value.	*/
	K_ENDING_S_UNITS,		/* Ending second units value.	*/
	K_ENDING_F_UNITS,		/* Ending frame units value.	*/

	/*
	 * Play Keyword Tokens:
	 */
	K_AUDIO_LBA,			/* Play audio (lba format).	*/
	K_AUDIO_MSF,			/* Play audio (msf format).	*/
	K_STARTING_TRACK,		/* Starting track number.	*/
	K_ENDING_TRACK,			/* Ending track number.		*/
	K_RELATIVE,			/* Play relative command.	*/
	K_TRACK,			/* Single track number.		*/

	/*
	 * Audio Keyword Tokens:
	 */
	K_ADDRESS_FORMAT,		/* Set default address format.	*/
	K_PLAY_INDEXES,			/* Number of indexes to play.	*/
	K_VENDOR_UNIQUE,		/* Vendor unique play command.	*/

	/*
	 * Table Of Contents Tokens:
	 */
	K_TOC_ENTRY,			/* Table of contents entry.	*/
	K_TOC_HEADER,			/* Table of contents header.	*/
	K_TOC_FULL,			/* Full table of contents.	*/
	K_TOC_SUMMARY,			/* Summary of table of contents.*/

	/*
	 * Tape formats and density codes.
	 */
	K_FIXED_LENGTH,			/* Fixed length tape blocks.	*/
	K_VARIABLE_LENGTH,		/* Variable length tape blocks.	*/
	K_TAPE_DENSITY,			/* Magtape density specified.	*/
	K_BLOCKING_MODE,		/* The fixed length block size.	*/
	K_BUFFERED_MODE,		/* The tape buffered mode.	*/

	/*
	 * Defect List Tokens:
	 */
	K_BLOCK_FORMAT,			/* Block defect list format.	*/
	K_BFI_FORMAT,			/* Bytes from index format.	*/
	K_PHYSICAL_FORMAT,		/* Physical sector format.	*/

	K_ALL_DEFECTS,			/* Vendor & Grown defect lists.	*/
	K_VENDOR_DEFECTS,		/* Vendor defect list only.	*/
	K_GROWN_DEFECTS,		/* Grown defect list only.	*/
	K_NO_DEFECTS,			/* Format without defect lists.	*/

	/*
	 * Read/Write Buffer Mode Field Tokens:
	 */
	K_BUFFER_ID,			/* Read/Write buffer ID.	*/
	K_BUFFER_MODE,			/* Read/Write buffer mode.	*/
	K_BUFFER_OFFSET,		/* Read/Write buffer offset.	*/
	K_SEGMENT_SIZE,			/* Download segment size.	*/

	/*
	 * Mode Page Field Tokens:
	 */
	K_AUDIO_CONTROL,		/* Audio control page.		*/
	K_CDROM_PARAMS,			/* CD-ROM parameters page.	*/
	K_ERROR_RECOVERY,		/* Error control recovery page.	*/
	K_DISCO_RECO,			/* Disconnect/reconnect page.	*/
	K_DIRECT_ACCESS,		/* Direct-access format page.	*/
	K_DISK_GEOMETRY,		/* Disk geometry page.		*/
	K_FLEXIBLE_DISK,		/* Flexible disk page.		*/
	K_CACHE_CONTROL,		/* Cache control page.		*/
	K_VERIFY_RECOVERY,		/* Verify recovery page.	*/
	K_DEC_SPECIFIC,			/* DEC specific page.		*/
	K_READAHEAD_CONTROL,		/* Read-ahead control page.	*/
	K_MODE_PARAMETERS,		/* Mode parameter headers.	*/
	K_DEVICE_CONFIG,		/* Device configuration page.	*/
	K_MEDIUM_PARTITION_1,		/* Medium partition page 1.	*/
	K_MODE_PAGE,			/* Mode page value specified.	*/

	/*
	 * Page Control Field Tokens:
	 */
	K_PCF,				/* Page control field.		*/
	K_PCF_CURRENT,			/* Current mode select params.	*/
	K_PCF_CHANGEABLE,		/* Changeable     ""    ""	*/
	K_PCF_DEFAULT,			/* Default        ""    ""	*/
	K_PCF_SAVED,			/* Saved          ""    ""	*/
	K_SAVE_MODE_PARAMETERS,		/* Save mode parameters flag.	*/

	/*
	 * Reset Keyword Tokens:
	 */
	K_RESET_BUS,			/* Reset the SCSI bus.		*/
	K_RESET_DEVICE,			/* Reset a SCSI device.		*/

	/*
	 * Skip Keyword Tokens:
	 */
	K_FORWARD_TRACK,		/* Forward track.		*/
	K_BACKWARD_TRACK,		/* Backward track.		*/

	/*
	 * Test Keyword Tokens:
	 */
	K_SELF_TESTS,			/* Controller default self-test	*/
	K_CONTROLLER_TESTS,		/* Do the controller tests.	*/
	K_DRIVE_TESTS,			/* Do the drive control tests.	*/
	K_MEMORY_TESTS,			/* Test the controllers memory.	*/

	/*
	 * Test Memory / Download Microcode Keywords:
	 */
	K_SAVE_DATA,			/* Save downloaded image data.	*/

	/*
	 * Volume Keyword Tokens:
	 */
	K_CHANNEL_0,			/* Channel 0 parameters.	*/
	K_CHANNEL_1,			/* Channel 1 parameters.	*/
	K_CHANNEL_2,			/* Channel 2 parameters.	*/
	K_CHANNEL_3,			/* Channel 3 parameters.	*/
	K_CHAN0_SELECT,			/* Channel 0 selection code.	*/
	K_CHAN0_VOLUME,			/* Channel 0 volume level.	*/
	K_CHAN1_SELECT,			/* Channel 1 selection code.	*/
	K_CHAN1_VOLUME,			/* Channel 1 volume level.	*/
	K_CHAN2_SELECT,			/* Channel 2 selection code.	*/
	K_CHAN2_VOLUME,			/* Channel 2 volume level.	*/
	K_CHAN3_SELECT,			/* Channel 3 selection code.	*/
	K_CHAN3_VOLUME,			/* Channel 3 volume level.	*/
	K_VOLUME_LEVEL,			/* Volume level (all channels).	*/

	K_DONT_CARE,			/* General use keyword token.	*/
	K_LAST_TOKEN			/* Last keyword token value.	*/
};
