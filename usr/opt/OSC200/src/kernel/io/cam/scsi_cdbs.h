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
 * @(#)$RCSfile: scsi_cdbs.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/06/25 08:53:05 $
 */
#if !defined(SCSI_CDBS_INCLUDE)
#define SCSI_CDBS_INCLUDE 1

/************************************************************************
 *									*
 * File:	scsi_cdbs.h						*
 * Date:	April 9, 1991						*
 * Author:	Robin T. Miller						*
 *									*
 * Description:								*
 *	SCSI Command Block Descriptor definitions.			*
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

/************************************************************************
 *									*
 *			  Generic SCSI Commands				*
 *									*
 ************************************************************************/

/* 
 * Test Unit Ready Command Descriptor Block:
 */
struct TestUnitReady_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Inquiry Data Structure:
 */
struct InquiryData {
	u_char	inq_dtype	: 5,	/* Peripheral device type.	[0] */
		inq_pqual	: 3;	/* Peripheral qualifier.	    */
	u_char	inq_dqual	: 7,	/* Device type qualifier.	[1] */
		inq_rmb		: 1;	/* Removable media.		    */
	u_char	inq_ansi	: 3,	/* ANSI version.		[2] */
		inq_ecma	: 3,	/* ECMA version.		    */
		inq_iso		: 2;	/* ISO version.			    */
	u_char	inq_rdf		: 4,	/* Response data format.	[3] */
				: 2,	/* Reserved.			    */
		inq_trmiop	: 1,	/* Terminate I/O process.	    */
		inq_aenc	: 1;	/* Asynchronous notification.	    */
	u_char	inq_addlen;		/* Additional length.		[4] */
	u_char			: 8;	/* Reserved.			[5] */
	u_char			: 8;	/* Reserved.			[6] */
	u_char	inq_sftre	: 1,	/* Soft reset support.		[7] */
		inq_cmdque	: 1,	/* Command queuing support.	    */
				: 1,	/* Reserved bit.		    */
		inq_linked	: 1,	/* Linked command support.	    */
		inq_sync	: 1,	/* Synchronous data transfers.	    */
		inq_wbus16	: 1, 	/* Support for 16 bit transfers.    */
		inq_wbus32	: 1,	/* Support for 32 bit transfers.    */
		inq_reladdr	: 1;	/* Relative addressing support.	    */
	u_char	inq_vid[8];		/* Vendor ID.		     [8-15] */
	u_char	inq_pid[16];		/* Product ID.		    [16-31] */
	u_char	inq_revlevel[4];	/* Revision level.	    [32-35] */
	u_char	inq_revdata[8];		/* Revision data.	    [36-43] */
};

/*
 * Defined Peripheral Quailifiers:
 */
#define PQUAL_CONNECTED		0x0	/* Device is connected.		*/
#define PQUAL_NOT_CONNECTED	0x1	/* Device is NOT connected.	*/
#define PQUAL_NO_PHYSICAL	0x3	/* No physical device support.	*/
#define PQUAL_VENDOR_SPECIFIC	0x4	/* Vendor specific peripheral.	*/

/*
 * Defined Device Types:
 */
#define	DTYPE_DIRECT		0x00	/* Direct access.		*/
#define	DTYPE_SEQUENTIAL	0x01	/* Sequential access.		*/
#define	DTYPE_PRINTER		0x02	/* Printer.			*/
#define	DTYPE_PROCESSOR		0x03	/* Processor.			*/
#define	DTYPE_WORM		0x04	/* Write-Once/Read Many.	*/
#define	DTYPE_RODIRECT		0x05	/* Read-Only direct access.	*/
#define	DTYPE_SCANNER		0x06	/* Scanner.			*/
#define	DTYPE_OPTICAL		0x07	/* Optical.			*/
#define	DTYPE_CHANGER		0x08	/* Changer.			*/
#define	DTYPE_NOTPRESENT	0x1F	/* Unknown or no device type.	*/
#define DTYPE_ALL_DEVICES	0xff	/* Supported for all devices.	*/

/*
 * Defined Response Data Formats:
 */
#define	RDF_LEVEL0		0x00	/* May or maynot comply to ANSI	*/
#define	RDF_CCS			0x01	/* SCSI-1 compliant.		*/
#define	RDF_SCSI2		0x02	/* SCSI-2 compliant.		*/

/*
 * Inquiry Command Descriptor Block:
 */
struct Inquiry_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	evpd	: 1,		/* Enable Vital Product Data.	[1] */
			: 4,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	pgcode	: 8;		/* EVPD Page Code.		[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	alclen;			/* Allocation Length.		[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Mode Sense Command Descriptor Block:
 */
struct ModeSense_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 3,		/* Reserved.			[1] */
		dbd	: 1,		/* Disable Block Descriptors.	    */
			: 1,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	pgcode	: 6,		/* Page code.			[2] */
		pcf	: 2;		/* Page Control Field.		    */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	alclen;			/* Allocation Length.		[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Mode Select Command Descriptor Block:
 */
struct ModeSelect_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	sp	: 1,		/* Save Parameters.		[1] */
			: 3,		/* Reserved.			    */
		pf	: 1,		/* Page Format.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	pll;			/* Parameter List Length.	[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Send Diagnostic Command Descriptor Block:
 */
struct SendDiagnostic_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	control	: 5,		/* Diagnostic control bits.	[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char	param_len1;		/* Allocation length (MSB).	[3] */
	u_char	param_len0;		/* Allocation length (LSB).	[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Receive Diagnostic Result Command Descriptor Block:
 */
struct ReceiveDiagnostic_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char	alloc_len1;		/* Allocation length (MSB).	[3] */
	u_char	alloc_len0;		/* Allocation length (LSB).	[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Sense Key Codes:
 */
#define SKV_NOSENSE		0x0	/* No error or no sense info.	*/
#define SKV_RECOVERED		0x1	/* Recovered error (success).	*/
#define SKV_NOT_READY		0x2	/* Unit is not ready.		*/
#define SKV_MEDIUM_ERROR	0x3	/* Nonrecoverable error.	*/
#define SKV_HARDWARE_ERROR	0x4	/* Nonrecoverable hardware err.	*/
#define SKV_ILLEGAL_REQUEST	0x5	/* Illegal CDB parameter.	*/
#define SKV_UNIT_ATTENTION	0x6	/* Target has been reset.	*/
#define SKV_DATA_PROTECT	0x7	/* Unit is write protected.	*/
#define SKV_BLANK_CHECK		0x8	/* A no-data condition occured.	*/
#define SKV_COPY_ABORTED	0xA	/* Copy command aborted.	*/
#define SKV_ABORTED_CMD		0xB	/* Target aborted cmd, retry.	*/
#define SKV_EQUAL		0xC	/* Vendor unique, not used.	*/
#define SKV_VOLUME_OVERFLOW	0xD	/* Physical end of media detect	*/
#define SKV_MISCOMPARE		0xE	/* Source & medium data differ.	*/
#define SKV_RESERVED		0xF	/* This sense key is reserved.	*/

/*
 * Request Sense Data Format.
 */
struct RequestSenseData {
	u_char	error_code	: 4,	/* Error code.			[0] */
		error_class	: 3,	/* Error class.			    */
		valid		: 1;	/* Information fields valid.	    */
	u_char	segment_number;		/* Segment number.		[1] */
	u_char	sense_key	: 4,	/* Sense key.			[2] */
				: 1,	/* Reserved.			    */
		illegal_length	: 1,	/* Illegal length indicator.	    */
		end_of_medium	: 1,	/* End of medium.		    */
		file_mark	: 1;	/* Tape filemark detected.	    */
	u_char	info_byte3;		/* Information byte (MSB).	[3] */
	u_char	info_byte2;		/*      "       "		[4] */
	u_char	info_byte1;		/*      "       "		[5] */
	u_char	info_byte0;		/* Information byte (LSB).	[6] */
	u_char	addl_sense_length;	/* Additional sense length.	[7] */
	u_char	cmd_spec_info3;		/* Command specific info (MSB).	[8] */
	u_char	cmd_spec_info2;		/*    "       "      "		[9] */
	u_char	cmd_spec_info1;		/*    "       "      "		[10]*/
	u_char	cmd_spec_info0;		/* Command specific info (LSB).	[11]*/
	u_char	addl_sense_code;	/* Additional sense code.	[12]*/
	u_char	addl_sense_qual;	/* Additional sense qualifier.	[13]*/
	u_char	fru_code;		/* Field replaceable unit.	[14]*/
	u_char	addl_sense_bytes[10];	/* Additional sense bytes.	[15]*/
};

/*
 * Additional Sense Bytes Format for "ILLEGAL REQUEST" Sense Key:
 */
struct sense_field_pointer {
	u_char	bit_ptr		: 3,	/* Bit pointer.			[15]*/
		bpv		: 1,	/* Bit pointer valid.		    */
				: 2,	/* Reserved.			    */
		cmd_data	: 1,	/* Command/data (1=CDB, 0=Data)	    */
		sksv		: 1;	/* Sense key specific valid.	    */
	u_char	field_ptr1;		/* Field pointer (MSB byte).	[16]*/
	u_char	field_ptr0;		/* Field pointer (LSB byte).	[17]*/
};

/*
 * Additional Sense Bytes Format for "RECOVERED ERROR", "HARDWARE ERROR",
 * or "MEDIUM ERROR" Sense Keys:
 */
struct sense_retry_count {
	u_char			: 7,	/* Reserved.			[15]*/
		sksv		: 1;	/* Sense key specific valid.	    */
	u_char	retry_count1;		/* Retry count (MSB byte).	[16]*/
	u_char	retry_count0;		/* Retry count (LSB byte).	[17]*/
};

/*
 * Additional Sense Bytes Format for "NOT READY" Sense Key:
 */
struct sense_format_progress {
	u_char			: 7,	/* Reserved.			[15]*/
		sksv		: 1;	/* Sense key specific valid.	    */
	u_char	progress_ind1;		/* Progress indicator (MSB byte)[16]*/
	u_char	progress_ind0;		/* Progress indicator (LSB byte)[17]*/
};


/************************************************************************
 *									*
 *			     Direct I/O Commands			*
 *									*
 ************************************************************************/

/*
 * Format Unit Command Descriptor Block:
 */
struct FormatUnit_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	dlf	: 3,		/* Defect List Format.		[1] */
		cmplst	: 1,		/* Complete List.		    */
		fmtdat	: 1,		/* Format Data.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	pattern;		/* Format Data Pattern.		[2] */
	u_char	interleave1;		/* Interleave Factor.		[3] */
	u_char	interleave0;		/* Interleave Factor.		[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Prevent/Allow Medium Removal Command Descriptor Block:
 */
struct PreventAllow_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	prevent	: 1,		/* Prevent = 1, Allow = 0.	[4] */
			: 7;		/* Reserved.			    */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Read Capacity Command Descriptor Block:
 */
struct ReadCapacity_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	reladr 	: 1,		/* Relative Address.		[1] */
			: 4,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	lba_3;			/* Logical Block Address 	[2] */
	u_char	lba_2;			/*    "      "      "		[3] */
	u_char	lba_1;			/*    "      "      "		[4] */
	u_char	lba_0;			/*    "      "      "		[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char		: 8;		/* Reserved.			[7] */
	u_char	pmi	: 1,		/* Partial Media Indicator.	[8] */
			: 7;		/* Reserved.			    */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Reassign Blocks Command Descriptor Block:
 */
struct ReassignBlocks_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Disk Read / Write / Seek 10-byte CDB.
 */
struct DirectRW10_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	reladr	: 1,		/* Relative Address.		[1] */
			: 4,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	lbaddr3;		/* Logical Block Address.	[2] */
	u_char	lbaddr2;		/* Logical Block Address.	[3] */
	u_char	lbaddr1;		/* Logical Block Address.	[4] */
	u_char	lbaddr0;		/* Logical Block Address.	[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char	xferlen1;		/* Transfer Length.    		[7] */
	u_char	xferlen0;		/* Transfer Length.    		[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Read Defect Data Command Descriptor Block:
 */
struct ReadDefectData_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	dlf	: 3,		/* Defect List Format.		[2] */
		grown	: 1,		/* Grown Defect List.		    */
		manuf	: 1,		/* Manufacturers Defect List.	    */
			: 3;		/* Reserved.			    */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char		: 8;		/* Reserved.			[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char	alclen1;		/* Allocation Length.		[7] */
	u_char	alclen0;		/* Allocation Length.		[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Start/Stop Unit Command Descriptor Block:
 */
struct StartStopUnit_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	immed	: 1,		/* Immediate.			[1] */
			: 4,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	start	: 1;		/* Start = 1, Stop = 0.		[4] */
	u_char	loej	: 1;		/* Load/Eject = 1, 0 = No Affect.   */
	u_char		: 6;		/* Reserved.			    */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/* 
 * Verify Data Command Descriptor Block:
 */
struct VerifyDirect_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	reladr 	: 1,		/* Relative Address.		[1] */
		bytchk	: 1,		/* Byte Check.			    */
			: 3,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	lbaddr3;		/* Logical Block Address 	[2] */
	u_char	lbaddr2;		/*    "      "      "		[3] */
	u_char	lbaddr1;		/*    "      "      "		[4] */
	u_char	lbaddr0;		/*    "      "      "		[5] */
	u_char		: 8;		/* Reserved.			[6] */
	u_char	verflen1;		/* Verification Length.		[7] */
	u_char	verflen0;		/* Verification Length.		[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/************************************************************************
 *									*
 *			 Sequential I/O Commands			*
 *									*
 ************************************************************************/

/*
 * Erase Tape Command Descriptor Block:
 */
struct EraseTape_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	longe	: 1,		/* Long Erase (1 = Entire Tape)	[1] */
			: 4,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Load / Unload / Retention Tape Command Descriptor Block:
 */
struct LoadUnload_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	immed	: 1,		/* Immediate.			[1] */
			: 4,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char	load	: 1,		/* Load.			[4] */
		reten	: 1,		/* Retention.			    */
		eot	: 1,		/* End Of Tape.			    */
			: 5;		/* Reserved.			    */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Rewind Tape Command Descriptor Block:
 */
struct RewindTape_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	immed	: 1,		/* Immediate.			[1] */
			: 4,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char		: 8;		/* Reserved.			[3] */
	u_char		: 8;		/* Reserved.			[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
};

/*
 * Space Operation Codes.
 */
#define SPACE_BLOCKS		0	/* Space blocks (or records).	*/
#define SPACE_FILE_MARKS	1	/* Space file marks.		*/
#define SPACE_SEQ_FILE_MARKS	2	/* Space sequential file marks.	*/
#define SPACE_END_OF_DATA	3	/* Space to end of media.	*/
#define SPACE_SETMARKS		4	/* Space setmarks.		*/
#define SPACE_SEQ_SET_MARKS	5	/* Space sequential setmarks.	*/

/*
 * Space Tape Command Descriptor Block:
 */
struct SpaceTape_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	code	: 3,		/* Space Blocks/Filemarks/EOM.	[1] */
			: 2,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	count2;			/* Count (MSB).			[2] */
	u_char	count1;			/* Count.			[3] */
	u_char	count0;			/* Count (LSB).			[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 1,		/* Vendor Unique.		    */
		fast	: 1;		/* Fast Space Algorithm.	    */
};

/*
 * Write Filemarks Command Descriptor Block:
 */
struct WriteFileMark_CDB {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char		: 5,		/* Reserved.			[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	fmcount2;		/* Number of Filemarks (MSB).	[2] */
	u_char	fmcount1;		/* Number of Filemarks.		[3] */
	u_char	fmcount0;		/* Number of Filemarks (LSB).	[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 1,		/* Vendor Unique.		    */
		fast	: 1;		/* Fast Space Algorithm.	    */
};

/************************************************************************
 *									*
 *			  CD-ROM Audio Commands				*
 *									*
 ************************************************************************/

/*
 * CD-ROM Pause/Resume Command Descriptor Block:
 */
struct CdPauseResume_CDB {
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
};

/*
 * CD-ROM Play Audio LBA Command Descriptor Block:
 */
struct CdPlayAudioLBA_CDB {
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
};

/*
 * CD-ROM Play Audio MSF Command Descriptor Block:
 */
struct CdPlayAudioMSF_CDB {
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
};

/*
 * CD-ROM Play Audio Track/Index Command Descriptor Block:
 */
struct CdPlayAudioTI_CDB {
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
};

/*
 * CD-ROM Play Audio Track Relative Command Descriptor Block:
 */
struct CdPlayAudioTR_CDB {
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
};

/*
 * CD-ROM Read TOC Command Descriptor Block:
 */
struct CdReadTOC_CDB {
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
};

/*
 * CD-ROM Read Sub-Channel Command Descriptor Block:
 */
struct CdReadSubChannel_CDB {
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
};

/*
 * CD-ROM Read Header Command Descriptor Block:
 */
struct CdReadHeader_CDB {
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
};

/*
 * CD-ROM Play Track Command Descriptor Block:
 */
struct CdPlayTrack_CDB {
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
};

/*
 * CD-ROM Playback Control/Status Command Descriptor Block:
 */
struct CdPlayback_CDB {
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
};

/*
 * CD-ROM Set Address Format Command Descriptor Block:
 */
struct CdSetAddressFormat_CDB {
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
};

#endif /* !defined(SCSI_CDBS_INCLUDE) */
