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
 * @(#)$RCSfile: scsi_opcodes.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/06/25 08:57:36 $
 */
#if !defined(SCSI_OPCODES_INCLUDE)
#define SCSI_OPCODES_INCLUDE 1

/************************************************************************
 *									*
 * File:	scsi_opcodes.h						*
 * Date:	May 7, 1991						*
 * Author:	Robin T. Miller						*
 *									*
 * Description:								*
 *	SCSI Operation Codes.  The operation codes defined in this file	*
 * were taken directly from the ANSI SCSI-2 Specification.		*
 *									*
 * Modification History:						*
 *									*
 ************************************************************************/

/*
 * MACRO for converting a longword to a byte
 *	args - the longword
 *	     - which byte in the longword you want
 */
#define	LTOB(a,b)	((a>>(b*8))&0xff)

/*
 * Define Masks for SCSI Group Codes.
 */
#define	SCSI_GROUP_0		0x00	/* SCSI Group Code 0.		*/
#define SCSI_GROUP_1		0x20	/* SCSI Group Code 1.		*/
#define SCSI_GROUP_2		0x40	/* SCSI Group Code 2.		*/
#define SCSI_GROUP_3		0x60	/* SCSI Group Code 3.		*/
#define SCSI_GROUP_4		0x80	/* SCSI Group Code 4.		*/
#define SCSI_GROUP_5		0xA0	/* SCSI Group Code 5.		*/
#define SCSI_GROUP_6		0xC0	/* SCSI Group Code 6.		*/
#define SCSI_GROUP_7		0xE0	/* SCSI Group Code 7.		*/
#define SCSI_GROUP_MASK		0xE0	/* SCSI Group Code mask.	*/

/*
 * Define Maximum Logical Block Address (LBA) for a 6-byte CDB.
 *
 *	0x1fffff = 2097151 512 byte blocks = 1,073,741,312 bytes
 *		or 1048575.50 Kbytes (K = 1024.)
 *		or 1023.99 Mbytes    (M = 1024 * 1024 = 1,048,576.)
 *		or 0.99 Gbytes       (G = 1048576 * 1024 = 1,073,741,824)
 */
#define SCSI_MAX_LBA		0x1fffff /* Maximum 6-byte CDB LBA.	*/

/*
 * Pause/Resume Audio Playback Parameters:
 */
#define SP_AUDIO_PAUSE		0	/* Pause audio playback.	*/
#define SP_AUDIO_RESUME		1	/* Resume audio playback.	*/

/*
 * Allow/Prevent Medium Removal Parameters:
 */
#define SP_ALLOW_REMOVAL	0	/* Allow medium removal.	*/
#define SP_PREVENT_REMOVAL	1	/* Prevent medium removal.	*/

/*
 * Start/Stop Unit Parameters:
 */
#define SP_STOP_UNIT		0	/* Stop the unit.		*/
#define SP_START_UNIT		1	/* Start the unit.		*/

/*
 * SCSI Operation Codes for All Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_INQUIRY				0x12
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_READ_BUFFER			0x3C
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_WRITE_BUFFER			0x3B

/*
 * SCSI Operation Codes for Direct-Access Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_FORMAT_UNIT			0x04
#define SOPC_INQUIRY				0x12
#define SOPC_LOCK_UNLOCK_CACHE			0x36
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_PREFETCH				0x34
#define SOPC_PREVENT_ALLOW_REMOVAL		0x1E
#define SOPC_READ_6				0x08
#define SOPC_READ_10				0x28
#define SOPC_READ_BUFFER			0x3C
#define SOPC_READ_CAPACITY			0x25
#define SOPC_READ_DEFECT_DATA			0x37
#define SOPC_READ_LONG				0x3E
#define SOPC_REASSIGN_BLOCKS			0x07
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RELEASE				0x17
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE				0x16
#define SOPC_REZERO_UNIT			0x01
#define SOPC_SEARCH_DATA_EQUAL			0x31
#define SOPC_SEARCH_DATA_HIGH			0x30
#define SOPC_SEARCH_DATA_LOW			0x32
#define SOPC_SEEK_6				0x0B
#define SOPC_SEEK_10				0x2B
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_SET_LIMITS				0x33
#define SOPC_START_STOP_UNIT			0x1B
#define SOPC_SYNCHRONIZE_CACHE			0x35
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_VERIFY				0x2F
#define SOPC_WRITE_6				0x0A
#define SOPC_WRITE_10				0x2A
#define SOPC_WRITE_VERIFY			0x2E
#define SOPC_WRITE_BUFFER			0x3B
#define SOPC_WRITE_LONG				0x3F
#define SOPC_WRITE_SAME				0x41

/*
 * SCSI Operation Codes for Sequential-Access Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_ERASE				0x19
#define SOPC_INQUIRY				0x12
#define SOPC_LOAD_UNLOAD			0x1B
#define SOPC_LOCATE				0x2B
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_PREVENT_ALLOW_REMOVAL		0x1E
#define SOPC_READ				0x08
#define SOPC_READ_BLOCK_LIMITS			0x05
#define SOPC_READ_BUFFER			0x3C
#define SOPC_READ_POSITION			0x34
#define SOPC_READ_REVERSE			0x0F
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RECOVER_BUFFERED_DATA		0x14
#define SOPC_RELEASE_UNIT			0x17
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE_UNIT			0x16
#define SOPC_REWIND				0x01
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_SPACE				0x11
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_VERIFY_TAPE			0x13
#define SOPC_WRITE				0x0A
#define SOPC_WRITE_BUFFER			0x3B
#define SOPC_WRITE_FILEMARKS			0x10

/*
 * SCSI Operation Codes for Printer Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_FORMAT				0x04
#define SOPC_INQUIRY				0x12
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_PRINT				0x0A
#define SOPC_READ_BUFFER			0x3C
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RECOVER_BUFFERED_DATA		0x14
#define SOPC_RELEASE_UNIT			0x17
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE_UNIT			0x16
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_SLEW_PRINT				0x0B
#define SOPC_STOP_PRINT				0x1B
#define SOPC_SYNCHRONIZE_BUFFER			0x10
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_WRITE_BUFFER			0x3B

/*
 * SCSI Operation Codes for Processor Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_INQUIRY				0x12
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_READ_BUFFER			0x3C
#define SOPC_RECEIVE				0x08
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_SEND				0x0A
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_WRITE_BUFFER			0x3B

/*
 * SCSI Operation Codes for Write-Once Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_INQUIRY				0x12
#define SOPC_LOCK_UNLOCK_CACHE			0x36
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MEDIUM_SCAN			0x38
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_PREFETCH				0x34
#define SOPC_PREVENT_ALLOW_REMOVAL		0x1E
#define SOPC_READ_6				0x08
#define SOPC_READ_10				0x28
#define SOPC_READ_12				0xA8
#define SOPC_READ_BUFFER			0x3C
#define SOPC_READ_CAPACITY			0x25
#define SOPC_READ_LONG				0x3E
#define SOPC_REASSIGN_BLOCKS			0x07
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RELEASE				0x17
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE				0x16
#define SOPC_REZERO_UNIT			0x01

/*
 * SCSI Operation Codes for CD-ROM Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_INQUIRY				0x12
#define SOPC_LOCK_UNLOCK_CACHE			0x36
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_PAUSE_RESUME			0x4B
#define SOPC_PLAY_AUDIO_10			0x45
#define SOPC_PLAY_AUDIO_12			0xA5
#define SOPC_PLAY_AUDIO_MSF			0x47
#define SOPC_PLAY_AUDIO_TRACK_INDEX		0x48
#define SOPC_PLAY_TRACK_RELATIVE_10		0x49
#define SOPC_PLAY_TRACK_RELATIVE_12		0xA9
#define SOPC_PREFETCH				0x34
#define SOPC_PREVENT_ALLOW_REMOVAL		0x1E
#define SOPC_READ_6				0x08
#define SOPC_READ_10				0x28
#define SOPC_READ_12				0xA8
#define SOPC_READ_BUFFER			0x3C
#define SOPC_READ_CAPACITY			0x25
#define SOPC_READ_HEADER			0x44
#define SOPC_READ_LONG				0x3E
#define SOPC_READ_SUBCHANNEL			0x42
#define SOPC_READ_TOC				0x43
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RELEASE				0x17
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE				0x16
#define SOPC_REZERO_UNIT			0x01
#define SOPC_SEARCH_DATA_EQUAL_10		0x31
#define SOPC_SEARCH_DATA_EQUAL_12		0xB1
#define SOPC_SEARCH_DATA_HIGH_10		0x30
#define SOPC_SEARCH_DATA_HIGH_12		0xB0
#define SOPC_SEARCH_DATA_LOW_10			0x32
#define SOPC_SEARCH_DATA_LOW_12			0xB2
#define SOPC_SEEK_6				0x0B
#define SOPC_SEEK_10				0x2B
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_SET_LIMITS_10			0x33
#define SOPC_SET_LIMITS_12			0xB3
#define SOPC_START_STOP_UNIT			0x1B
#define SOPC_SYNCHRONIZE_CACHE			0x35
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_VERIFY_10				0x2F
#define SOPC_VERIFY_12				0xAF
#define SOPC_WRITE_BUFFER			0x3B

/*
 * Sony CDU-541 Vendor Unique Commands (Group 6).
 */
#define SOPC_SET_ADDRESS_FORMAT			0xC0
#define SOPC_PLAYBACK_STATUS			0xC4
#define SOPC_PLAY_TRACK				0xC6
#define SOPC_PLAY_MSF				0xC7
#define SOPC_PLAY_VAUDIO			0xC8
#define SOPC_PLAYBACK_CONTROL			0xC9

/*
 * SCSI Operation Codes for Scanner Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_GET_DATA_BUFFER_STATUS		0x34
#define SOPC_GET_WINDOW				0x25
#define SOPC_INQUIRY				0x12
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_OBJECT_POSITION			0x31
#define SOPC_READ_SCANNER			0x28
#define SOPC_READ_BUFFER			0x3C
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RELEASE_UNIT			0x17
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE_UNIT			0x16
#define SOPC_SCAN				0x1B
#define SOPC_SET_WINDOW				0x24
#define SOPC_SEND_SCANNER			0x2A
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_WRITE_BUFFER			0x3B

/*
 * SCSI Operation Codes for Optical Memory Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_COMPARE				0x39
#define SOPC_COPY				0x18
#define SOPC_COPY_VERIFY			0x3A
#define SOPC_ERASE_10				0x2C
#define SOPC_ERASE_12				0xAC
#define SOPC_FORMAT_UNIT			0x04
#define SOPC_INQUIRY				0x12
#define SOPC_LOCK_UNLOCK_CACHE			0x36
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MEDIUM_SCAN			0x38
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_PREFETCH				0x34
#define SOPC_PREVENT_ALLOW_REMOVAL		0x1E
#define SOPC_READ_6				0x08
#define SOPC_READ_10				0x28
#define SOPC_READ_12				0xA8
#define SOPC_READ_BUFFER			0x3C
#define SOPC_READ_CAPACITY			0x25
#define SOPC_READ_DEFECT_DATA_10		0x37
#define SOPC_READ_DEFECT_DATA_12		0xB7
#define SOPC_READ_GENERATION			0x29
#define SOPC_READ_LONG				0x3E
#define SOPC_READ_UPDATED_BLOCK			0x2D
#define SOPC_REASSIGN_BLOCKS			0x07
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RELEASE				0x17
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE				0x16
#define SOPC_REZERO_UNIT			0x01
#define SOPC_SEARCH_DATA_EQUAL_10		0x31
#define SOPC_SEARCH_DATA_EQUAL_12		0xB1
#define SOPC_SEARCH_DATA_HIGH_10		0x30
#define SOPC_SEARCH_DATA_HIGH_12		0xB0
#define SOPC_SEARCH_DATA_LOW_10			0x32
#define SOPC_SEARCH_DATA_LOW_12			0xB2
#define SOPC_SEEK_6				0x0B
#define SOPC_SEEK_10				0x2B
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_SET_LIMITS_10			0x33
#define SOPC_SET_LIMITS_12			0xB3
#define SOPC_START_STOP_UNIT			0x1B
#define SOPC_SYNCHRONIZE_CACHE			0x35
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_UPDATE_BLOCK			0x3D
#define SOPC_VERIFY_10				0x2F
#define SOPC_VERIFY_12				0xAF
#define SOPC_WRITE_6				0x0A
#define SOPC_WRITE_10				0x2A
#define SOPC_WRITE_12				0xAA
#define SOPC_WRITE_VERIFY_10			0x2E
#define SOPC_WRITE_VERIFY_12			0xAE
#define SOPC_WRITE_BUFFER			0x3B
#define SOPC_WRITE_LONG				0x3F

/*
 * SCSI Operation Codes for Medium-Changer Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_EXCHANGE_MEDIUM			0xA6
#define SOPC_INITIALIZE_ELEMENT_STATUS		0x07
#define SOPC_INQUIRY				0x12
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_MOVE_MEDIUM			0xA5
#define SOPC_POSITION_TO_ELEMENT		0x2B
#define SOPC_PREVENT_ALLOW_REMOVAL		0x1E
#define SOPC_READ_BUFFER			0x3C
#define SOPC_READ_ELEMENT_STATUS		0xB8
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_RELEASE				0x17
#define SOPC_REQUEST_VOLUME_ELEMENT_ADDRESS	0xB5
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_RESERVE				0x16
#define SOPC_REZERO_UNIT			0x01
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_SEND_VOLUME_TAG			0xB6
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_WRITE_BUFFER			0x3B

/*
 * SCSI Operation Codes for Communication Devices.
 */
#define SOPC_CHANGE_DEFINITION			0x40
#define SOPC_GET_MESSAGE_6			0x08
#define SOPC_GET_MESSAGE_10			0x28
#define SOPC_GET_MESSAGE_12			0xA8
#define SOPC_INQUIRY				0x12
#define SOPC_LOG_SELECT				0x4C
#define SOPC_LOG_SENSE				0x4D
#define SOPC_MODE_SELECT_6			0x15
#define SOPC_MODE_SELECT_10			0x55
#define SOPC_MODE_SENSE_6			0x1A
#define SOPC_MODE_SENSE_10			0x5A
#define SOPC_READ_BUFFER			0x3C
#define SOPC_RECEIVE_DIAGNOSTIC			0x1C
#define SOPC_REQUEST_SENSE			0x03
#define SOPC_SEND_DIAGNOSTIC			0x1D
#define SOPC_SEND_MESSAGE_6			0x0A
#define SOPC_SEND_MESSAGE_10			0x2A
#define SOPC_SEND_MESSAGE_12			0xAA
#define SOPC_TEST_UNIT_READY			0x00
#define SOPC_WRITE_BUFFER			0x3B

#endif /* !defined(SCSI_OPCODES_INCLUDE) */
