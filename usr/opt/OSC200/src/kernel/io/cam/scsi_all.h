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
 * @(#)$RCSfile: scsi_all.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/09/07 20:39:06 $
 */
#ifndef _SCSI_ALL_
#define _SCSI_ALL_

/* ---------------------------------------------------------------------- */

/* scsi_all.h		Version 1.00			Feb. 25, 1991 

   General structures and defines for chapter 7 of the SCSI II spec. 

Modification History

	Version	Date		Who	Reason

	1.00	02/25/91	dallas	Creation of the module.  
	1.01	06/27/91	robin	Add send/receive diagnostic cmds.
	1.02	07/29/91	maria	Added ANSI approved version
					field defines for inquiry data.
	1.03	07/30/91	robin	Add read/write buffer commands.
*/

/* ---------------------------------------------------------------------- */
/* Include files. */

/*
 * You will need to include sys/h/types.h for this header file
 */

/* ---------------------------------------------------------------------- */

/*
 * GENERIC DEFINES FOR MODULES
 */

/* 
 * The size of our maximum out going data page for mode selects
 * OUR minimum data buffer is 512
 */
#define	ALL_MAX_PAGE_SIZE	256 

/* ---------------------------------------------------------------------- */

/* 
 *  The CHANGE DEFINITION command defines
 */


/*
 * CHANGE DEFINITION CDB 10 byte
 */

typedef struct all_change_def_cdb10 {
	u_char	opcode;		/* OPcode = 0X40			*/
	u_char		:5,	/* 5 bits reservered			*/
		lun	:3;	/* Logical unit number			*/
	u_char	save	:1,	/* Save control bit			*/
			:7;	/* 7 bits reserved			*/
	u_char	def_parm:7,	/* Definition parameter			*/
	                :1;	/* 1 bit reserved			*/
	u_char		:8;	/* Reserved byte			*/
	u_char		:8;	/* Reserved byte			*/
	u_char		:8;	/* Reserved byte			*/
	u_char		:8;	/* Reserved byte			*/
	u_char	param_ln;	/* Parameter data length		*/
	u_char	control;	/* Control				*/
}ALL_CHANGE_DEF_CDB10;


/*
 * Opcode define for change definition
 */
#define ALL_CHANGE_DEF10_OP	0X40	/* The opcode			*/


/* ---------------------------------------------------------------------- */

/* 
 *  The COMPARE command defines
 */


/*
 * COMPARE CDB 10 byte
 */

typedef struct all_compare_cdb10 {
	u_char	opcode;		/* OPcode = 0X39			*/
	u_char	pad	:1,	/* Padding				*/
	    		:4,	/* 4 bits reservered			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* Reserved byte			*/
	u_char	param_len2;	/* Parameter length (MSB).		*/
	u_char	param_len1;	/* Parameter length (MID).		*/
	u_char	param_len0;	/* Parameter length (LSB).		*/
	u_char		:8;	/* Reserved byte			*/
	u_char		:8;	/* Reserved byte			*/
	u_char		:8;	/* Reserved byte			*/
	u_char	control;	/* Control				*/
}ALL_COMPARE_CDB10;


/*
 * Opcode define for compare
 */
#define ALL_COMPARE10_OP	0X39	/* The opcode			*/


/* ---------------------------------------------------------------------- */

/* 
 *  The COPY command defines
 */


/*
 * COPY CDB 6 byte
 */

typedef struct all_copy_cdb {
	u_char	opcode;		/* OPcode = 0X18			*/
	u_char	pad	:1,	/* Padding				*/
	    		:4,	/* 4 bits reservered			*/
		lun	:3;	/* Logical unit number			*/
	u_char	param_len2;	/* Parameter length (MSB).		*/
	u_char	param_len1;	/* Parameter length (MID).		*/
	u_char	param_len0;	/* Parameter length (LSB).		*/
	u_char	control;	/* Control				*/
}ALL_COPY_CDB;


/*
 * Opcode define for compare
 */
#define ALL_COPY_OP		0X18	/* The opcode			*/


/* ---------------------------------------------------------------------- */

/* 
 *  The COPY AND VERIFY command defines
 */


/*
 * COPY AND VERIFY CDB 10 byte
 */

typedef struct all_copy_and_verify_cdb10 {
	u_char	opcode;		/* OPcode = 0X3a			*/
	u_char	pad	:1,	/* Padding				*/
	    	bytchk	:1,	/* Byte check bit			*/
	    		:3,	/* 3 bits reservered			*/
		lun	:3;	/* Logical unit number			*/
	u_char	param_len2;	/* Parameter length (MSB).		*/
	u_char	param_len1;	/* Parameter length (MID).		*/
	u_char	param_len0;	/* Parameter length (LSB).		*/
	u_char		:8;	/* Reserved byte			*/
	u_char		:8;	/* Reserved byte			*/
	u_char		:8;	/* Reserved byte			*/
	u_char	control;	/* Control				*/
}ALL_COPY_AND_VERIFY_CDB10;


/*
 * Opcode define for compare
 */
#define ALL_COPY_AND_VERIFY10_OP	0X3a	/* The opcode		*/


/* ---------------------------------------------------------------------- */


/* 
 *  The INQUIRY command defines
 */


/*
 * INQUIRY CDB 6 byte
 */

typedef struct all_inq_cdb {
	u_char	opcode;		/* OPcode = 0X12			*/
	u_char	evpd	:1,	/*Enable vital product data bit		*/
			:4,	/* 4 bits resevered			*/
		lun	:3;	/* Logical unit number			*/
	u_char	page;		/* Page code				*/
	u_char		:8;	/* Byte reserved			*/
	u_char  alloc_len;	/* Allocation length			*/
	u_char	control;	/* Control byte				*/
}ALL_INQ_CDB;


/*
 * Opcode define for inquiry
 */
#define ALL_INQ_OP	0X12	/* The opcode				*/



/* 
 * Standard inquiry data format 
 */


typedef struct all_inq_data {
	u_char	dtype	: 5,	/* Peripheral device type.	[0] */
		pqual	: 3;	/* Peripheral qualifier.	    */
	u_char	dmodify	: 7,	/* Device type modifier.	[1] */
		rmb	: 1;	/* Removable media.		    */
	u_char	ansi	: 3,	/* ANSI version.		[2] */
		ecma	: 3,	/* ECMA version.		    */
		iso	: 2;	/* ISO version.			    */
	u_char	rdf	: 4,	/* Response data format.	[3] */
			: 2,	/* Reserved.			    */
		trmiop	: 1,	/* Terminate I/O process	    */
		aenc	: 1;	/* Async Notification of events	    */
	u_char	addlen;		/* Additional length.		[4] */
	u_char		: 8;	/* Reserved.			[5] */
	u_char		: 8;	/* Reserved.			[6] */
	u_char	sftre	: 1,	/* Soft reset 1 = yes		[7] */
		cmdque	: 1,	/* Command queuing		    */
			: 1,	/* Reserved bit				*/
		linked	: 1,	/* Linked command support	    */
		sync	: 1,	/* Synchronous data transfers	    */
		wbus16	: 1, 	/* support of 16 bit transfers	    */
		wbus32	: 1,	/* support of 32 bit transfers	    */
		reladdr : 1;	/* Relative addressing support	    */
	u_char	vid[8];		/* Vendor ID.		     [8-15] */
	u_char	pid[16];	/* Product ID.		    [16-31] */
	u_char	revlevel[4];	/* Revision level.	    [32-35] */
}ALL_INQ_DATA;




/* 
 * Defines for what comes back from the inquiry command.
 */

/*
 * Defined Peripheral Quailifiers: ALL_INQ_DATA.pqual field
 */
	/* Device is connected.					*/
#define ALL_PQUAL_CONN				0x0	

	/* Device is NOT connected			.	*/
#define ALL_PQUAL_NOT_CONN			0x1	

	/* No physical device support.				*/
#define ALL_PQUAL_NO_PHYS			0x3	

	/* Vendor specific peripheral.				*/
#define ALL_PQUAL_VENDOR_SPEC			0x4	


/*
 * Defined Device Types: ALL_INQ_DATA.dtype field
 */
#define	ALL_DTYPE_DIRECT	0x00	/* Direct access.		*/
#define	ALL_DTYPE_SEQUENTIAL	0x01	/* Sequential access.		*/
#define	ALL_DTYPE_PRINTER	0x02	/* Printer.			*/
#define	ALL_DTYPE_PROCESSOR	0x03	/* Processor.			*/
#define	ALL_DTYPE_WORM		0x04	/* Write-Once/Read Many.	*/
#define	ALL_DTYPE_RODIRECT	0x05	/* Read-Only direct access.	*/
#define	ALL_DTYPE_SCANNER	0x06	/* Scanner.			*/
#define	ALL_DTYPE_OPTICAL	0x07	/* Optical.			*/
#define	ALL_DTYPE_CHANGER	0x08	/* Changer.			*/
#define ALL_DTYPE_COMM		0x09	/* Communications Device	*/
#define	ALL_DTYPE_NONE		0x1F	/* Device type is NONE(unknown) */

/* 
 * The ANSI-Approved version field.
 */
#define ALL_SCSI1		0x1
#define ALL_SCSI2		0x2

/* 
 * The removabled media bit ALL_INQ_DATA.rmb field
 */
#define ALL_RMB			0x1	/* Is the device/media removable */


/*
 * Defined Response Data Formats: ALL_INQ_DATA.rdf field
 */
#define	ALL_RDF_LEVEL0		0x00
#define	ALL_RDF_CCS		0x01
#define	ALL_RDF_SCSI2		0x02

	/* Support of TERMINATE of I/O process ALL_INQ_DATA.trmiop	*/
#define ALL_TERMIOP		0x1	

	/* support of async event notification	ALL_INQ_DATA.aenc	*/
#define ALL_ASYNC_NOTICE		0x1


/*
 * The following bit defines tell us what features the unit offers
 * like synchronous data transfers, 32 bit data transfers, and the 
 * in-law bit where if the bit is on their address is YOURS
 */

	/* Soft reset or hard	*/
#define ALL_SOFTRESET		0x1

	/* Command queuing	*/
#define ALL_CMD_QUE		0x1

	/* Linked Commands	*/
#define ALL_LINKED		0x1	

	/* Synchronous data transfers	*/
#define ALL_SYNC		0x1

	/* 16 bit data transfers	*/
#define ALL_WBUS16		0x1

	/* 32 bit transfers		*/
#define ALL_WBUS32		0x1

	/* Relative Addressing		*/
#define ALL_RELATIVE_ADDR	0x1

/* ---------------------------------------------------------------------- */

/*
 * Mode select/sense defines
 */


/*
 * The mode select CDB 6 bytes.
 */



typedef struct all_mode_sel_cdb6 {
	u_char	opcode;		/* 15 hex 				*/
	u_char	sp	: 1,	/* Save parameters			*/
			: 3, 	/* 3 bits reserved			*/
		pf	: 1,	/* Page format-true conforms to scsi 2	*/
		lun	: 3;	/* logical unit number			*/
	u_char		: 8;	/* Reserved byte			*/
	u_char		: 8;	/* Reserved byte			*/
	u_char	param_len;	/* parameter list length		*/
	u_char	control;	/* The control byte			*/
}ALL_MODE_SEL_CDB6;

/* 
 * The 6 byte mode select op code
 */
#define ALL_MODE_SEL6_OP	0x15


/*
 * The mode select CDB 10 bytes.
 */

typedef struct all_mode_sel_cdb10 {
	u_char	opcode;		/* 55 hex 				*/
	u_char	sp	: 1,	/* Save parameters			*/
			: 3, 	/* 3 bits resevred			*/
		pf	: 1,	/* Page format-true conforms to scsi 2	*/
		lun	: 3;	/* logical unit number			*/
	u_char		: 8;	/* Reserved byte			*/
	u_char		: 8;	/* Reserved byte			*/
	u_char		: 8;	/* Reserved byte			*/
	u_char		: 8;	/* Reserved byte			*/
	u_char		: 8;	/* Reserved byte			*/
	u_char	param_len1;	/* MSB parameter list length		*/
	u_char	param_len0;	/* LSB parameter list length		*/
	u_char	control;	/* The control byte			*/
}ALL_MODE_SEL_CDB10;

/* 
 * The 10 byte mode select op code
 */
#define ALL_MODE_SEL10_OP	0x55

/*
 * The mode sense CDB 6 bytes.
 */

typedef struct all_mode_sense_cdb6 {
	u_char	opcode;		/* 1a hex 			[0]	*/
	u_char		: 3,	/* 3 bits reserved		[1]	*/
		dbd	: 1, 	/* Disable block descriptors		*/
			: 1,	/* 1 bit reserved			*/
		lun	: 3;	/* logical unit number			*/
	u_char	page_code : 6,	/* page you want to retrieve	[2]	*/
		pc	: 2;	/* page control field			*/
	u_char		: 8;	/* Reserved byte		[3]	*/
	u_char	alloc_len;	/* Allocation length		[4]	*/
	u_char	control;	/* The control byte		[5]	*/
}ALL_MODE_SENSE_CDB6;

/* 
 * The 6 byte mode sense op code
 */
#define ALL_MODE_SENSE6_OP	0x1A

/*
 * The mode sense CDB 10 bytes.
 */

typedef struct all_mode_sense_cdb10 {
	u_char	opcode;		/* 5a hex 			[0]	*/
	u_char		: 3,	/* 3 bits reserved		[1]	*/
		dbd	: 1, 	/* Disable block descriptors		*/
			: 1,	/* 1 bit reserved			*/
		lun	: 3;	/* logical unit number			*/
	u_char	page_code : 6,	/* page you want to retrieve	[2]	*/
		pc	: 2;	/* page control field			*/
	u_char		: 8;	/* Reserved byte		[3]	*/
	u_char		: 8;	/* Reserved byte		[4]	*/
	u_char		: 8;	/* Reserved byte		[5]	*/
	u_char		: 8;	/* Reserved byte		[6]	*/
	u_char	alloc_len1;	/* MSB Allocation length	[7]	*/
	u_char	alloc_len0;	/* LSB Allocation length	[8]	*/
	u_char	control;	/* The control byte		[9]	*/
}ALL_MODE_SENSE_CDB10;

/* 
 * The 10 byte mode sense op code
 */
#define ALL_MODE_SENSE10_OP	0x5A



/* 
 * Values for Page Control field. (mode pages PCFM)
 */
#define ALL_PCFM_CURRENT	0x00	/* Return current mode select parms */
#define ALL_PCFM_CHANGEABLE	0x01	/*   ""  changeable ""   ""    "" */
#define ALL_PCFM_DEFAULT	0x10	/*   ""   default   ""   ""    "" */
#define ALL_PCFM_SAVED		0x11	/*   ""   saved     ""   ""    "" */

/* ---------------------------------------------------------------------- */

/*
 * RECEIVE DIAGNOSTIC RESULT Command
 */
typedef struct all_receive_diagnostic_cdb {
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
} ALL_RECEIVE_DIAGNOSTIC_CDB;

#define ALL_RECEIVE_DIAGNOSTIC_OP	0x1C

/* ---------------------------------------------------------------------- */

/* 
 * REQUEST SENSE structs and defines
 */

/* 
 * Request sense 6 byte cdb
 */


typedef struct all_req_sense_cdb6 {
	u_char	opcode;		/* 0x03 hex 			[0]	*/
	u_char		: 5,	/* 5 bits reserved		[1]	*/
		lun	: 3;	/* logical unit number			*/
	u_char		: 8;	/* Reserved byte		[2]	*/
	u_char		: 8;	/* Reserved byte		[3]	*/
	u_char	alloc_len;	/* Allocation length		[4]	*/
	u_char	control;	/* The control byte		[5]	*/
}ALL_REQ_SENSE_CDB6;

/* 
 * The 6 byte request sense op code
 */
#define ALL_REQ_SENSE6_OP	0x03

/* 
 * The sense key specific structures for request sense data.
 * sense key specific area has certain formats which are defined
 * for sense sense keys or cmds
 */

/* 
 * The sense specific for illegal request
 */
typedef struct all_sks_ill_req {
	u_char	bit_pointer	:3,	/* bit in error of the byte	*/
		bpv		:1, 	/* bit pointer field valid	*/
				:2,	/* 2 bits reserved		*/
		c_or_d		:1,	/* error is in cmd or data	*/
		sksv		:1;	/* sense key specific valid	*/
	u_char	field_ptr1;		/* MSB of field pointer		*/
	u_char	field_ptr0;		/* LSB of field pointer		*/
}ALL_SKS_ILL_REQ;

/* 
 * The sense specific for Recovered, hardware, medium erros
 */
typedef struct all_sks_retry_cnt {
	u_char			:7,	/* 7 bits reserved		*/
		sksv		:1;	/* sense key specific valid	*/
	u_char	retry_cnt1;		/* MSB retry count 		*/
	u_char	retry_cnt0;		/* LSB retry count		*/
}ALL_SKS_RETRY_CNT;

/* 
 * The sense key of NOT READY and formating unit with the immed bit
 */
typedef struct all_sks_prog_cnt {
	u_char			:7,	/* 7 bits reserved		*/
		sksv		:1;	/* sense key specific valid	*/
	u_char	progress1;		/* MSB of progress count 	*/
	u_char	progress0;		/* LSB of progress count	*/
}ALL_SKS_PROG_CNT;


/* 
 * The request sense data struct (whats returned to us)
 * NON scatter/gather
 */

typedef struct all_req_sns_data {
	u_char	error_code	:7,	/* the error code 70h/71h	*/
		valid		:1;	/* The valid bit		*/
	u_char	segment;		/* the segment number		*/
	u_char	sns_key		:4,	/* sense key			*/
				:1,	/* bit reserved			*/
		ili		:1,	/* illegal length indicator	*/
		eom		:1,	/* End of media			*/
		filemark	:1;	/* file mark			*/
	u_char	info_byte3;		/* MSB of information field	*/
	u_char	info_byte2;		/* byte 2 of information field	*/
	u_char	info_byte1;		/* byte 1 of information field	*/
	u_char	info_byte0;		/* LSB of information field	*/
	u_char	addition_len;		/* additional sense length	*/
	u_char	cmd_specific3;		/* MSB of command specific info	*/
	u_char	cmd_specific2;		/* byte 2 of command specific info*/
	u_char	cmd_specific1;		/* byte 3 of command specific info*/
	u_char	cmd_specific0;		/* LSB pf command specific info	*/
	u_char	asc;			/* additional sense code	*/
	u_char	asq;			/* additional sense code qual	*/
	u_char	fru;			/* field replacement unit	*/
	union	{
		ALL_SKS_ILL_REQ		sks_ill_req;
		ALL_SKS_RETRY_CNT	sks_retry_cnt;
		ALL_SKS_PROG_CNT	sks_prog_cnt;
		}sense_specific;
	union	{
		u_char	auto_sns[DEC_AUTO_SENSE_SIZE];
		u_char	other_sns[ALL_MAX_PAGE_SIZE];
		}additional_sense;
}ALL_REQ_SNS_DATA;

/* 
 * The request sense data struct (whats returned to us)
 * Scatter/gather
 */

typedef struct all_req_sns_data_gather {
	u_char	error_code	:7,	/* the error code 70h/71h	*/
		valid		:1;	/* The valid bit		*/
	u_char	segment;		/* the segment number		*/
	u_char	sns_key		:4,	/* sense key			*/
				:1,	/* bit reserved			*/
		ili		:1,	/* illegal length indicator	*/
		eom		:1,	/* End of media			*/
		filemark	:1;	/* file mark			*/
	u_char	info_byte1;		/* MSB of information field	*/
	u_char	info_byte0;		/* LSB of information field	*/
	u_char	addition_len;		/* additional sense length	*/
	u_char	cmd_specific1;		/* MSB of command specific info	*/
	u_char	cmd_specific0;		/* LSB pf command specific info	*/
	u_char	asc;			/* additional sense code	*/
	u_char	asq;			/* additional sense code qual	*/
	u_char	fru;			/* field replacement unit	*/
	union	{
		ALL_SKS_ILL_REQ		sks_ill_req;
		ALL_SKS_RETRY_CNT	sks_retry_cnt;
		ALL_SKS_PROG_CNT	sks_prog_cnt;
		}sense_specific;
	union	{
		u_char	*auto_sns;
		u_char	*other_sns;
		}additional_sense;
}ALL_REQ_SNS_DATA_GATHER;

/*
 * The error code defines
 */
#define	ALL_IMED_ERR_CODE	0x70	/* Noticed this cmd		*/
#define ALL_DEFER_ERR_CODE	0x71	/* Previous cmd had error (good */

/* 
 * The sense keys
 */
#define ALL_NO_SENSE		0x00	/* No sense key info. check eom */
#define ALL_RECOVER_ERR		0x01	/* Recovered error (soft)	*/
#define ALL_NOT_READY		0x02	/* Device not readay		*/
#define ALL_MEDIUM_ERR		0x03	/* Medium error data bad	*/
#define ALL_HARDWARE_ERR	0x04	/* Device had some so of error	*/
#define ALL_ILLEGAL_REQ		0x05	/* Gave the unit bogus cmd/data	*/
#define ALL_UNIT_ATTEN		0x06	/* Unit Attention		*/
#define ALL_DATA_PROTECT	0x07	/* Write protected unit		*/
#define ALL_BLANK_CHECK		0x08	/* Blank media at this point	*/
#define ALL_VENDOR_SPEC		0x09	/* Vendor specific 		*/
#define ALL_COPY_ABORT		0x0a	/* copy,compare,etc had error	*/
#define ALL_ABORTED_CMD		0x0b	/* Target aborted cmd		*/
#define ALL_EQUAL		0x0c	/* search has found match	*/
#define ALL_VOL_OVERFLOW	0x0d	/* Volume overflow		*/
#define ALL_MISCOMPARE		0x0e	/* source didn't match media	*/




/* 
 * The additional sense codes and qualifiers are defined at the
 * end of this module.
 */




/* ---------------------------------------------------------------------- */

/*
 * LOG SELECT and SENSE
 */

/* 
 * LOG select command cdb 10 bytes
 */
typedef struct all_log_del_cdb10 {
	u_char	opcode;			/* 0x4c				*/
	u_char	sp		:1,	/* Save parameters		*/
		pcr		:1,	/* Parameter code reset		*/
				:3,	/* 3 bits reserved		*/
		lun		:3;	/* Logical unit number		*/
	u_char			:6,	/* 6 bits reserved		*/
		pc		:2;	/* Page control field		*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
	u_char	param_len1;		/* MSB Parameter list length	*/
	u_char	param_len0;		/* LSB Parameter list length	*/
	u_char	control;		/* Control byte			*/
}ALL_LOG_SEL_CDB10;

/* 
 * LOG select opcode define
 */
#define ALL_LOG_SEL_OP		0x4c


/*
 * Page control field defines for LOG command (PCFL) in octal.
 */
#define ALL_PCFL_CURR_THRES	00	/* Current threshold values	*/
#define ALL_PCFL_CURR_CUMU	01	/* Current cumulative values	*/
#define ALL_PCFL_DEF_THRES	02	/* Default threshold values	*/
#define ALL_PCFL_DEF_CUMU	03	/* Default cumulative values	*/


/* 
 * LOG sense command cdb 10 bytes
 */
typedef struct all_log_sns_cdb10 {
	u_char	opcode;			/* 0x4d				*/
	u_char	sp		:1,	/* Save parameters		*/
		ppc		:1,	/* Parameter pointer control	*/
				:3,	/* 3 bits reserved		*/
		lun		:3;	/* Logical unit number		*/
	u_char	pg_code		:6,	/* Page code			*/
		pc		:2;	/* Page control field		*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
	u_char	param_ptr1;		/* MSB Parameter pointer	*/
	u_char	param_ptr0;		/* LSB Parameter pointer	*/
	u_char	alloc_len1;		/* MSB Allocation length	*/
	u_char	alloc_len0;		/* LSB Allocation length	*/
	u_char	control;		/* Control byte			*/
}ALL_LOG_SNS_CDB10;


/* 
 * LOG sense opcode define
 */
#define ALL_LOG_SNS_OP		0x4d




/* ---------------------------------------------------------------------- */

/* 
 * LOG parameter pages
 */

/*
 * Log page header
 */
typedef struct all_log_pg_hd {
	u_char	page_code	:6,	/* The page code		*/
				:2;	/* 2 bits reserved		*/
	u_char			:8;	/* Reserved			*/
	u_char	page_len1;		/* MSB of page length		*/
	u_char	page_len0;		/* LSB of page length		*/
}ALL_LOG_PG_HD;

/* 
 * LOG parameter format  NON scatter gather
 */
typedef struct all_log_fmt {
	u_char	param_code1;		/* MSB of parameter code	*/
	u_char	param_code0;		/* LSB of parameter code	*/
	u_char	lp		:1,	/* List parameters		*/
				:1,	/* 1 bit reserved		*/
		tmc		:2,	/* Threshold met criteria	*/
		etc		:1,	/* Enable threshold criteria	*/
		tsd		:1,	/* Target save disable		*/
		ds		:1,	/* Disable save			*/
		du		:1;	/* Disable update		*/
	u_char	param_len;		/* Parameter length 		*/
	u_char	param_data[ALL_MAX_PAGE_SIZE];
}ALL_LOG_FMT;


/* 
 * LOG PARAMETER PAGE  SCATTER/GATHER
 */
typedef struct all_log_param_pg_gather {
	ALL_LOG_PG_HD	pg_head;	/* Page header			*/
	ALL_LOG_FMT	**log_param;	/* list of pointer to parameters*/
}ALL_LOG_PARAM_PG_GATHER;



/* 
 * LOG PAGE CODE DEFINES 
 */
#define ALL_PGL_SUPPT		0x00	/* Supported Log pages		*/
#define ALL_PGL_RUNS		0x01	/* Over/under runs		*/
#define ALL_PGL_WRT		0x03	/* Error counters write		*/
#define ALL_PGL_READ		0x03	/* Error counters read		*/
#define ALL_PGL_READ_REV	0x04	/* Error counters read reverse	*/
#define ALL_PGL_VERIFY		0x05	/* Error counters verify	*/
#define ALL_PGL_MEDIUM		0x06	/* NON medium errors		*/
#define ALL_PGL_LASTN		0x07	/* Last n errors		*/

/* 
 *  Buffer over-run/under run counters page code 0x01
 */
typedef struct all_log_run_pg {
	u_char			:8;	/* Reserved			*/
	u_char	type		:1,	/* Buffer under or over runs	*/
		cause		:4,	/* Cause for over/under runs	*/
		cnt_basis	:3;	/* Count basis (how counted)	*/
}ALL_LOG_RUN_PG;

/* 
 * Cause field defines
 */
#define ALL_CAUSE_BBSY		0x01	/* SCSI BUS BUSY		*/
#define ALL_CAUSE_TRANS		0x02	/* Transfer rate to slow	*/




/* ---------------------------------------------------------------------- */

/* 
 * MODE Parameter defines
 */
/*
 * Definitions for CCS/SCSI-2 Devices.
 */



/*
 * Since we need to place all the page codes some where for easy 
 * reference they are here. But for each of the device specific 
 * information please refer to the .h file that deal with that 
 * device type. EXAMPLE sequence devices (tapes) see scsi_sequential.h 
 * Follows scsi 2 spec chapters.  
 */

		/* Values for Page Code field. */
/* Defined in chapters 8,9		0x01	   Error recovery page.	*/ 
#define ALL_PGM_DISCO_RECO		0x02	/* Disconnect/reconnect	*/
/* Defined in chapters 8		0x03	   Direct access format */
/* Defined in chapters 8		0x04	   Disk geometry page.	*/
/* Defined in chapters 8		0x05	   Flexible disk page.	*/
/* Defined in chapters			0x06				*/
/* Defined in chapters 8		0x07	/* Verify error recovery*/
/* Defined in chapters 8		0x08	   Cache control page.  */
#define ALL_PGM_PERIPH_DEVICE		0x09	/* Peripheral Dev page  */
#define ALL_PGM_CONTROL_MODE		0x0A	/* Control Mode page	*/
/* Defined in chapters 8		0x0B	   Supported medium	*/
/* Defined in chapters 8		0x0C	   NOTCH and PARTITION	*/
/* Defined in chapters			0x0D	   Device parameters.   */
/* Defined in chapters 13		0x0E	   Audio control page.  */
/* Defined in chapters 9		0x10	   Device Configuration */
/* Defined in chapters 9		0x11	   Medium Partition pg1 */
/* Defined in chapters 9		0x12	   Medium Partition pg2 */
/* Defined in chapters 9		0x13	   Medium Partition pg3	*/
/* Defined in chapters 9		0x14	   Medium Partition pg4 */
/* Defined in chapters ?????		0x25	   DEC specific page.   */
/* Defined in chapters ?????		0x38	   Read-ahead control   */



/* 
 * The mode parameter header, 6 byte cdb
 */
typedef struct all_mode_head6 {
	u_char	mode_len;	/* The length of the mode head 		*/
	u_char	medium_type;	/* The medium type			*/
	u_char	device_spec;	/* The device specific byte		*/
	u_char	blk_desc_len;	/* The length of the descriptor block	*/
}ALL_MODE_HEAD6;

/* 
 * The mode parameter header, 10 byte cdb
 */
typedef struct all_mode_head10 {
	u_char	mode_len1;	/* The length of the mode head MSB	*/
	u_char	mode_len0;	/* The length of the mode head LSB	*/
	u_char	medium_type;	/* The medium type			*/
	u_char	device_spec;	/* The device specific byte		*/
	u_char		:8;	/* Byte reserved			*/	
	u_char		:8;	/* Byte reserved			*/	
	u_char	blk_desc_len1;	/* Length of the descriptor block MSB	*/
	u_char	blk_desc_len0;	/* Length of the descriptor block LSB	*/
}ALL_MODE_HEAD10;


/* 
 * The Block Descriptor for both 6 and 10 byte MODE PARAMETERS.
 */

typedef struct all_mode_desc {
	u_char	density_code;	/* The density code Tapes		*/
	u_char	num_blocks2;	/* MSB of number of blocks 		*/
	u_char	num_blocks1;	/* Middle of number of blocks		*/
	u_char	num_blocks0;	/* LSB of number of blocks		*/
	u_char		:8;	/* reserved				*/
	u_char	block_len2;	/* MSB of block length			*/
	u_char	block_len1;	/* Middle of block length		*/
	u_char	block_len0;	/* LSB of block length			*/
}ALL_MODE_DESC;


/* 
 * The actual data structure for all the mode select data.
 */

/*
 * For a mode parameter list 6 byte cdb NON scatter/gather
 */

typedef struct all_mode_data6 {
	ALL_MODE_HEAD6		sel_head;	/* The header		*/
	ALL_MODE_DESC		sel_desc;	/* The descriptor piece	*/
	u_char	page_data[ALL_MAX_PAGE_SIZE];	/* Page data area	*/
}ALL_MODE_DATA6;


/*
 * For a mode parameter list 6 byte cdb scatter/gather
 */

typedef struct all_mode_data6_gather {
	ALL_MODE_HEAD6		*sel_head;	/* The header		*/
	ALL_MODE_DESC		*sel_desc;	/* The descriptor piece	*/
	u_char			*page_data;	/* Page data area	*/
}ALL_MODE_DATA6_GATHER;

/*
 * For a mode parameter list 10 byte cdb NON scatter/gather
 */
typedef struct all_mode_data10 {
	ALL_MODE_HEAD10		sel_head;	/* The header		*/
	ALL_MODE_DESC		sel_desc;	/* The descriptor piece	*/
	u_char	page_data[ALL_MAX_PAGE_SIZE];	/* Page data area	*/
}ALL_MODE_DATA10;



/*
 * For a mode parameter list 10 byte cdb scatter/gather
 */
typedef struct all_mode_data10_gather{
	ALL_MODE_HEAD10		*sel_head;	/* The header		*/
	ALL_MODE_DESC		*sel_desc;	/* The descriptor piece	*/
	u_char			*page_data;	/* Page data area	*/
}ALL_MODE_DATA10_GATHER;


/*
 * The mode page header
 */
typedef struct all_mode_pg_hd {
	u_char	page_code	: 6;	/* define page function */
	u_char	reserved	: 1;
	u_char	ps		: 1;	/* Page savable. */
	u_char	page_length;		/* length of current page */
}ALL_MODE_PG_HD;


/*
 * The pages defined in chapter 7
 */


/*
 * Page 0x2 - Disconnect / Reconnect Control Parameters:
 */
typedef struct all_disc_reco_pg {
	ALL_MODE_PG_HD	page_header;	/* The page header	*/
	u_char		buf_full_ratio;
	u_char		buf_empty_ratio;
	u_char		bus_inact_limit1;
	u_char		bus_inact_limit0;
	u_char		discon_time_limit1;
	u_char		discon_time_limit0;
	u_char		con_time_limit1;
	u_char		con_time_limit_0;
	u_char		max_burst_size1;
	u_char		max_burst_size0;
	u_char		dtdc	:2,		/* data transfer discon cntl*/ 
	u_char			:6;		/* reserved	*/
	u_char			:8;		/* reserved	*/ 
	u_char			:8;		/* reserved	*/ 
	u_char			:8;		/* reserved	*/ 
}ALL_DISC_RECO_PG;

/*
 * defines for the dtdc field.
 */
#define ALL_DISCON		0x00 /* Disconnects allowed 	  	*/
#define	ALL_NO_DISCON_DATA	0x01 /* Don't disconnect until data done */
#define ALL_NO_DISCON_CMD	0x03 /* Don't disconnect until cmd done	*/


/*
 * Page 0x9 - The device peripheral page
 */
typedef struct all_periph_dev_pg {
	ALL_MODE_PG_HD	page_header;
	u_char	interface_id1;	/* The interface identifier msb		*/
	u_char	interfave_id0;	/* The interface identifier lsb		*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char	vendor_specific[ALL_MAX_PAGE_SIZE - 8]; /* Total max 
						page size is 256	*/
}ALL_PERIPH_DEV_PG;

/* 
 * Page 0xA - The Control mode page 
 */
typedef struct all_control_pg {
	ALL_MODE_PG_HD	page_header;
	u_char	rlec	:1,	/* report log exception			*/
	u_char		:7;	/* reserved				*/
	u_char	dque	:1,	/* Disable tag queuing 1 = true		*/
		qerr	:1,	/* queue management error		*/
			:2,	/* reserved				*/
		que_algo :4;	/* Queue algorithm Modifier		*/
	u_char	eaenp	:1,	/* Async error notification enable	*/
		uaaenp	:1,	/* Async unit atten notification enable	*/
		raenp	:1,	/* Async unit ready notification enable	*/
			:4,	/* reserved				*/
		eeca	:1;	/* Extended contingent allegiance enable*/
	u_char		:8;	/* reserved				*/
	u_char	aen_holdoff1;	/* MSB in millieseconds to hold off async*/
	u_char	aen_holdoff0;	/* LSB in millieseconds to hold off async*/
}ALL_CONTROL_PG;


/* ---------------------------------------------------------------------- */

/* 
 * SEND DIAGNOSTIC Command
 */
typedef struct all_send_diagnostic_cdb {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	control	: 5,		/* Diagnostic control bits.	[1] */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char		: 8;		/* Reserved.			[2] */
	u_char	param_len1;		/* Parameter length (MSB).	[3] */
	u_char	param_len0;		/* Parameter length (LSB).	[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} ALL_SEND_DIAGNOSTIC_CDB;

#define ALL_SEND_DIAGNOSTIC_OP	0x1D

/* ---------------------------------------------------------------------- */

/*
 * Test unit ready
 */
typedef struct all_tur_cdb {
	u_char	opcode;		/* OPcode = 0X00			*/
	u_char		:5,	/* 5 Bits reserved			*/
		lun	:3;	/* LUN					*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char	control;	/* Control byte				*/
}ALL_TUR_CDB;


/*
 * Opcode define for test unit ready
 */
#define ALL_TUR_OP	0X00	/* The opcode				*/

/* ---------------------------------------------------------------------- */


/*
 * The defines and marco for the asc and asq fields
 * in the return data from a request sense
 */

/*
 * This macro takes a pointer to the sense data, and a u_short.
 * With the pointer it make sure that the additional sense
 * length is large enought to contain the asc and asq fields.
 * The the asc field goes to the high byte and asq to the low.
 */

#define ASCQ_TO_USHORT(sns_data,ascq); { \
    union { \
	unsigned char c[2]; \
	unsigned short l; \
    }tmp; \
    if(((ALL_REQ_SNS_DATA *)(sns_data))->addition_len < 4) { \
	(ascq) = 0; \
    } \
    else { \
    	tmp.c[0] = ((ALL_REQ_SNS_DATA *)(sns_data))->asq; \
    	tmp.c[1] = ((ALL_REQ_SNS_DATA *)(sns_data))->asc; \
	(ascq) = tmp.l; \
    } \
}
	

#define	ASCQ_NO_ADDITIONAL	0x0000 
			/* NO Additional Sense Information */

#define	ASCQ_FILEMARK		0x0001 
			/* Filemark detected */

#define ASCQ_EOPM		0x0002 
			/* End of partition/medium detected */

#define	ASCQ_SETMARK	 	0x0003 
			/* Setmark detected */

#define	ASCQ_BOPM	 	0x0004 
			/* Begining of partition/medium detected */

#define	ASCQ_EOD	 	0x0005 
			/* End of data detected */

#define	ASCQ_IO_TERM	 	0x0006 
			/* I/O process terminated */
	
#define	ASCQ_AUDIO_INPROG 	0x0011 
			/* Audio play operation in progress */

#define	ASCQ_AUDIO_PAUSED	 0x0012 
			/* Audio play operation paused */

#define	ASCQ_AUDIO_SUCC		 0x0013 
			/* Audio play operation successfully completed */

#define	ASCQ_AUDIO_ERR		 0x0014 
			/* Audio play operation stopped due to error */

#define	ASCQ_AUDIO_NO_STAT 	0x0015 
			/* No current audio status to return */

#define	ASCQ_NO_IDX_SEC		 0x0100 
			/* No index/sector signal */

#define	ASCQ_NO_SEEK_CMP 	0x0200 
			/* No seek complete */

#define	ASCQ_WRT_FLT		 0x0300 
			/* Device write fault */
	
#define	ASCQ_NO_WRT_CURR 	0x0301 
			/* NO write current */

#define	ASCQ_EXCESS_WRT_ERR	 0x0302 
			/* Excessive write errors */

#define	ASCQ_LUN_NRDY_STAT	 0x0400 
			/* LUN Not ready, cause not reportable */

#define	ASCQ_LUN_NRDY_INPROG	 0x0401 
			/* LUN Is in process of becoming ready */

#define	ASCQ_LUN_NRDY_INIT	 0x0402 
			/* LUN Not ready, init command required */

#define	ASCQ_LUN_NRDY_MAN	 0x0403 
			/* LUN Not ready, manual intervention required */

#define	ASCQ_LUN_NRDY_FMT 	0x0404 
			/* LUN Not ready, format inprogress */

#define	ASCQ_LUN_NO_SEL		 0x0500 
			/* LUN does not respond to selection */

#define	ASCQ_NO_REF_POS		 0x0600 
			/* No reference position found */

#define	ASCQ_MULTI_SEL		 0x0700 
			/* Multiple devices selected */

#define	ASCQ_LUN_COMM_FLT	 0x0800 
			/* LUN communication failure */

#define	ASCQ_LUN_COMM_TIMO		 0x0801 
			/* LUN communication timeout */

#define	ASCQ_LUN_COMM_PARITY	 0x0802 
			/* LUN communication parity error */

#define	ASCQ_TRK_FOL_ERR	 0x0900 
			/* Track following error */

#define	ASCQ_TRK_SERVO		 0x0901 
			/* Tracking servo failure */

#define	ASCQ_FOCUS_SERVO	 0x0902 
			/* Focus servo failure */

#define	ASCQ_SPINDLE_SERVO	 0x0903 
			/* Spindle servo failure */

#define	ASCQ_ERR_LOG_OVF	 0x0a00 
			/* Error log overflow */

#define	ASCQ_WRT_ERR		 0x0c00 
			/* Write error */

#define	ASCQ_WRT_RECV_REALL 	0x0c01 
			/* Write error recovered with auto reallocation */

#define	ASCQ_WRT_REALL_FTL 	0x0c02 
			/* Write error -auto reallocation failed */

#define	ASCQ_ID_CRC_ECC_ERR	 0x1000 
			/* ID, CRC, or ECC error */

#define	ASCQ_UNRECV_RD_ERR	 0x1100 
			/* Unrecoverable read error */

#define	ASCQ_RD_RETRIES_EXH	 0x1101 
			/* Read retries exhausted */
	
#define	ASCQ_ERR_TO_LONG	 0x1102 
			/* Error to long to correct */

#define	ASCQ_MULTI_RD_ERR	 0x1103 
			/* Multiple read errors */

#define	ASCQ_RD_ERR_REALL_FLT	 0x1104 
			/* Unrecovered read error - auto reallocation failed */

#define	ASCQ_L_EC_UNCORR	 0x1105 
			/* L-EC Uncorrectable error */

#define	ASCQ_CIRC_UNCORR 	0x1106 
			/* CIRC Unrecovered error */

#define	ASCQ_DATA_RESYNC_ERR	 0x1107 
			/* Data resynchronization error */

#define	ASCQ_INCOMP_BLK_RD	 0x1108 
			/* Incomplete block read */

#define	ASCQ_NO_GAP		 0x1109 
			/* No gap found */

#define	ASCQ_MISCORR_ERR	 0x110a 
			/* Miscorrected error */

#define	ASCQ_UNRECV_RD_REC_REASS 0x110b 
			/* Unrecovered read error - recommend reassignment */

#define	ASCQ_UNRECV_RD_REC_REWRT 0x110c 
			/* Unrecovered read error - recommend rewrite data */

#define	ASCQ_ADDR_MK_ID		 0x1200 
			/* Address mark not found for ID field */

#define	ASCQ_ADDR_MK_DATA 	0x1300 
			/* Address mark not found for DATA field */

#define	ASCQ_REC_ENTITY_NFND	 0x1400 
			/* Recorded entity not found */

#define	ASCQ_REC_NFND 		0x1401 
			/* Record not found */

#define	ASCQ_FS_MARK_NFND	 0x1402 
			/* Filemark or setmark not found */

#define	ASCQ_EOD_NFND 		0x1403 
			/* End of data not found */

#define	ASCQ_BLK_SEQ_ERR	 0x1404 
			/* Block Sequence error */

#define	ASCQ_RAN_POS_ERR	 0x1500 
			/* Random Positioning error */

#define	ASCQ_MECH_POS_ERR	 0x1501 
			/* Mechanical positioning error */

#define	ASCQ_POS_ERR_RD		 0x1502 
			/* Positioning error detected by read of medium */

#define	ASCQ_DATA_SYNC_MARK	 0x1600 
			/* Data synchronization MARK error */

#define	ASCQ_RECV_NO_ECC	 0x1700 
			/* Recovered data with no error correction */

#define	ASCQ_RECV_RETRIES	 0x1701 
			/* Recovered data with retries */

#define	ASCQ_RECV_POS_OFF	 0x1702 
			/* Recovered data with positive head offset */

#define	ASCQ_RECV_NEG_OFF	 0x1703 
			/* Recovered data with negative head offset */

#define	ASCQ_RECV_CIRC	 	0x1704 
			/* Recovered data with retries and/or CIRC applied */

#define	ASCQ_RECV_PREV_SEC	0x1705 
			/* Recovered data using previous sector */

#define	ASCQ_RECV_REASS		0x1706 
			/* Recovered data without ECC - Data auto reallocated */

#define	ASCQ_RECV_REC_REASS 	0x1707 
		       /* Recovered data without ECC - recommend reassignment */

#define	ASCQ_RECV_ECC	 	0x1800 
			/* Recovered data with error correction applied */

#define	ASCQ_RECV_ECC_RETRIES 	0x1801 
			/* Recovered data with ECC Applied and retries */

#define	ASCQ_RECV_DATA_REASS	0x1802 
			/* Recovered data - Data auto reallocated */

#define	ASCQ_RECV_DATA_CIRC	 0x1803 
			/* Recovered data with CIRC */

#define	ASCQ_RECV_LEC		 0x1804 
			/* Recovered data with LEC */

#define	ASCQ_RECV_DATA_REC_REASS 0x1805 
			/* Recovered data - recommend reassignment */

#define	ASCQ_DEFECT_ERR		 0x1900 
			/* Defect list error */

#define	ASCQ_DEFECT_NAVAIL	 0x1901 
			/* Defect list not available */

#define	ASCQ_DEFECT_PRIM_ERR	 0x1902 
			/* Defect list error in primary list */

#define	ASCQ_DEFECT_GROWN_ERR	 0x1903 
			/* Defect list error in grown list */

#define	ASCQ_PARAM_LEN_ERR	 0x1a00 
			/* Parameter list length error */

#define	ASCQ_SYNC_TRAN_ERR	 0x1b00 
			/* Synchronous data transfer error */

#define	ASCQ_DEFECT_NFND	 0x1c00 
			/* Defect list not found */
	
#define	ASCQ_DEFECT_PRIM_NFND	 0x1c01 
			/* Primary defect list not found */

#define	ASCQ_DEFECT_GROWN_NFND	 0x1c02 
			/* Grown defect list not found */

#define	ASCQ_MISCMP_VERI	 0x1d00 
			/* Miscompare during verify */

#define	ASCQ_RECV_ID_ECC	 0x1e00 
			/* Recovered ID with ECC correction */

#define	ASCQ_INVAL_CMD_OP	 0x2000 
			/* Invaild command operation code */

#define	ASCQ_LBN_RANGE		 0x2100
			/* LBN out of range */

#define	ASCQ_INVAL_ELEM		 0x2101 
			/* Invalid element address */

#define	ASCQ_ILL_FUNC		 0x2200 
			/* Illegal function ( USE 20 00, 24 00 or 26 00) */

#define	ASCQ_INVAL_FLD_CDB	 0x2400 
			/* Invalid field in CDB */

#define	ASCQ_LUN_NSUP	 0x2500 
			/* LUN not supported */

#define	ASCQ_INVAL_FLD_PARAM	 0x2600 
			/* Invalid field in parameter list */

#define	ASCQ_PARAM_NSUP		0x2601 
			/* Parameter not supported */

#define	ASCQ_PARAM_VAL_INV 	0x2602 
			/* Parameter value invalid */

#define	ASCQ_THRES_PARAM_NSUP	0x2603 
			/* Threshold parameters not supported */

#define	ASCQ_WRT_PROT		0x2700 
			/* Write protected */

#define	ASCQ_NRDY_TO_RDY 	0x2800 
		/* Not ready to ready transistion (medium may have changed) */

#define	ASCQ_PON_RESET		0x2900 
			/* Power on, reset, bus device reset occurred */
	
#define	ASCQ_PARAM_CHG		 0x2a00 
			/* Parameters changed */

#define	ASCQ_MODE_PARAM_CHG	 0x2a01 
			/* Mode parameters changed */

#define	ASCQ_LOG_PARAM_CHG	 0x2a02 
			/* Log parameters changed */

#define	ASCQ_CPY_NO_DISC	 0x2b00 
			/* Copy can't execute since host can't disconnect */

#define	ASCQ_CMD_SEQ_ERR	 0x2c00 
			/* Commnad sequence error */

#define	ASCQ_TOO_MANY_WINS	 0x2c01 
			/* Too many window specified */

#define	ASCQ_INVAL_COMP_WINS	 0x2c02 
			/* Invalid combination of windows specified */

#define	ASCQ_OVR_WRT_UPD	 0x2d00 
			/* Overwrite error on update in place */

#define	ASCQ_CMDS_CLEARED	 0x2f00 
			/* Commands cleared by another initiator */

#define	ASCQ_INCOMPAT_MEDIA	 0x3000 
			/* Incompatible medium installed */

#define	ASCQ_UNKNW_FMT	 	0x3001 
			/* Cannot read medium - unknown format */

#define	ASCQ_INCOMPAT_FMT	 0x3002 
			/* Cannot read medium - incompatible format */
	
#define	ASCQ_CLEAN_CART		 0x3003 
			/* Cleaning cartridge installed */

#define	ASCQ_FMT_CORRUPT	 0x3100 
			/* Medium format corrupted */

#define	ASCQ_FMT_FAILED		 0x3101 
			/* Format command failed */

#define	ASCQ_NO_DEFECT_SPARE	 0x3200 
			/* No defect spare location available */

#define	ASCQ_TAPE_LEN_ERR	 0x3300 
			/* Tape length error */

#define	ASCQ_INK_RIB_ERR	 0x3600 
			/* Ribbon, ink or toner failure */

#define	ASCQ_RND_PARAM		 0x3700 
			/* Rounded parameter */

#define	ASCQ_SAVE_PARAM_NSUP	 0x3900 
			/* Saving parameters not supported */

#define	ASCQ_MEDIA_NPRES	 0x3a00 
			/* Medium not present */

#define	ASCQ_SEQ_POS_ERR	 0x3b00 
			/* Sequential postioning error */

#define	ASCQ_POS_BOM_ERR	 0x3b01 
			/* Tape position error at Begining of media */

#define	ASCQ_POS_EOM_ERR	 0x3b02 
			/* Tape position error at End of media */

#define	ASCQ_VERT_FORM_NRDY	 0x3b03 
			/* Tape or electronic vertical forms unit not ready */

#define	ASCQ_SLEW_FLT		 0x3b04 
			/* Slew failure */

#define	ASCQ_PAPER_JAM		 0x3b05 
			/* Paper jAM */

#define	ASCQ_FAIL_TFORM		 0x3b06 
			/* Failed to sense top of form */

#define	ASCQ_FAIL_BFORM		 0x3b07 
			/* Failed to sense bottom of form */

#define	ASCQ_REPOS_ERR		 0x3b08 
			/* Reposition error */

#define	ASCQ_RD_PAST_EOM	 0x3b09 
			/* Read past end of medium */

#define	ASCQ_RD_PAST_BOM	 0x3b0a 
			/* Read past beginning of medium */

#define	ASCQ_POS_PAST_EOM	 0x3b0b 
			/* Position past end of medium */

#define	ASCQ_POS_PAST_BOM	 0x3b0c 
			/* Position past beginning of medium */

#define	ASCQ_MEDIA_DEST_EMP	 0x3b0d 
			/* Medium destination element empty */

#define	ASCQ_MEDIA_SRC_EMP	 0x3b0e 
			/* Medium source element empty */

#define	ASCQ_INVAL_BIT_IDMSG	 0x3d00 
			/* Invalid bits in identify message */

#define	ASCQ_LUN_NSELF_CONF	 0x3e00 
			/* Logical unit has not self configured yet */

#define	ASCQ_TARGET_OP_CHG	 0x3f00 
			/* Target operating conditions have changed */

#define	ASCQ_MICRO_CODE_CHG	 0x3f01 
			/* Micro code has changed */

#define	ASCQ_CHG_OP_DEF		 0x3f02 
			/* Changed operating definition */

#define	ASCQ_INQ_CHG		 0x3f03 
			/* Inquiry data has changed */

#define	ASCQ_RAM_FLT		 0x4000 
			/* Ram failure */

#define	ASCQ_DATA_PATH_FLT	 0x4100 
			/* Data path failure */

#define	ASCQ_PON_SELFT_FLT	 0x4200 
			/* Power on or self test failure */

#define	ASCQ_MSG_ERR		 0x4300 
			/* Message error */

#define	ASCQ_INT_TARGET_FLT	 0x4400 
			/* Internal target failure */

#define	ASCQ_SEL_RSEL_FLT	 0x4500 
			/* Select or reselect failure */

#define	ASCQ_UNSUCC_SRESET	 0x4600 
			/* Unsuccessful soft reset */

#define	ASCQ_SCSI_PARITY_ERR	 0x4700 
			/* SCSI parity error */

#define	ASCQ_INITOR_ERR_MSG	 0x4800 
			/* Initiator detected error message recieved */

#define	ASCQ_INVAL_MSG_ERR	 0x4900 
			/* Invalid message error */

#define	ASCQ_CMD_PHASE_ERR	 0x4a00 
			/* Command phase error */

#define	ASCQ_DATA_PHASE_ERR	 0x4b00 
			/* Data phase error */

#define	ASCQ_LUN_STEST_FLT	 0x4c00 
			/* LUN failed self-configuration */

#define	ASCQ_OVRL_CMDS_AMPT	 0x4e00 
			/* Overlapped Commands attemped */

#define	ASCQ_WRT_APPEND_ERR	 0x5000 
			/* Write append error */

#define	ASCQ_WRT_APPEND_POS_ERR	 0x5001 
			/* Write append position error */

#define	ASCQ_POS_ERR_TIME	 0x5002 
			/* Position error related to timing */

#define	ASCQ_ERASE_FLT		 0x5100 
			/* Erase failure */

#define	ASCQ_CART_FLT		 0x5200 
			/* Cartridge fault */

#define	ASCQ_MEDIA_LD_EJ_FLT	 0x5300 
			/* Media load or eject failure */

#define	ASCQ_TAPE_UNLD_FLT	 0x5301 
			/* Unload tape failure */

#define	ASCQ_MEDIA_PREVNT	 0x5302 
			/* Medium removal prevented */

#define	ASCQ_SCSI_HOST_FLT	 0x5400 
			/* SCSI to host system interface failure */
	
#define	ASCQ_SYS_RESRC_FLT	 0x5500 
			/* System resource failure */

#define	ASCQ_UNRECV_TAB_CONT	 0x5700 
			/* Unable to recover table of contents */

#define	ASCQ_GEN_NEXIST		 0x5800 
			/* Generation does not exist */

#define	ASCQ_UPDT_BLK_RD	 0x5900 
			/* Updated block read */

#define	ASCQ_OPR_REQ_STATE	 0x5a00 
			/* Operator request or state change input */

#define	ASCQ_OPR_MEDIA_RM_REQ	 0x5a01 
			/* Operator medium removal request */

#define	ASCQ_OPR_SEL_WRT_PROT	 0x5a02 
			/* Operator selected write protect */

#define	ASCQ_OPR_SEL_WRT	 0x5a03 
			/* Operator selected write permit */

#define	ASCQ_LOG_EXCEPT		 0x5b00 
			/* Log exception */

#define	ASCQ_THRES_MET		 0x5b01 
			/* Threshold condition met */

#define	ASCQ_LOG_CNT_MAX	 0x5b02 
			/* Log counter at maximum */

#define	ASCQ_LOG_LIST_CODES	 0x5b03 
			/* Log list codes exhausted */

#define	ASCQ_RPL_STAT_CHG	 0x5c00 
			/* RPL status change */

#define	ASCQ_SPINDLES_SYNC	 0x5c01 
			/* Spindles synchronized */

#define	ASCQ_SPINDLES_NSYNC	 0x5c02 
			/* Spindles not synchronized */

#define	ASCQ_LAMP_FLT		 0x6000 
			/* Lamp failure */

#define	ASCQ_VID_ACQ_ERR	 0x6100 
			/* Video acquistion error */

#define	ASCQ_UNA_ACQ_VID	 0x6101 
			/* Unable to aquire video */

#define	ASCQ_OUT_FOCUS		 0x6102 
			/* Out of focus */

#define	ASCQ_SCAN_HD_POS_ERR	 0x6200 
			/* Scan head position error */

#define	ASCQ_END_USER_AREA	 0x6300 
			/* End of user area encountered on this track */

#define	ASCQ_ILL_MODE_TRK	 0x6400 
			/* Illegal mode for this track */
	

/* ---------------------------------------------------------------------- */

/*
 * READ / WRITE BUFFER Buffer Header:
 */
typedef struct all_buffer_header {
	u_char	reserved;		/* Reserved.			[0] */
	u_char	buffer_capacity2;	/* Buffer capacity (MSB).	[1] */
	u_char	buffer_capacity1;	/* Buffer capacity (MID).	[2] */
	u_char	buffer_capacity0;	/* Buffer capacity (LSB).	[3] */
} ALL_BUFFER_HEADER;

/* ---------------------------------------------------------------------- */

/* 
 * READ BUFFER Command
 */
typedef struct all_read_buffer_cdb {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	mode	: 3,		/* Mode field.			[1] */
			: 2,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	id	: 8;		/* Buffer ID.			[2] */
	u_char	offset2;		/* Buffer offset (MSB).		[3] */
	u_char	offset1;		/* Buffer offset (MID).		[4] */
	u_char	offset0;		/* Buffer offset (LSB).		[5] */
	u_char	alloc_len2;		/* Allocation length (MSB).	[6] */
	u_char	alloc_len1;		/* Allocation length (MID).	[7] */
	u_char	alloc_len0;		/* Allocation length (LSB).	[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} ALL_READ_BUFFER_CDB;

#define ALL_READ_BUFFER_OP	0x3C

/*
 * Define READ BUFFER Mode Field Definitions:
 */
#define ALL_RDBUF_COMBINED_HEADER	0x00
#define ALL_RDBUF_VENDOR_SPECIFIC	0x01
#define ALL_RDBUF_READ_DATA		0x02
#define ALL_RDBUF_DESCRIPTOR		0x03

/* ---------------------------------------------------------------------- */

/* 
 * WRITE BUFFER Command
 */
typedef struct all_write_buffer_cdb {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	mode	: 3,		/* Mode field.			[1] */
			: 2,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	id	: 8;		/* Buffer ID.			[2] */
	u_char	offset2;		/* Buffer offset (MSB).		[3] */
	u_char	offset1;		/* Buffer offset (MID).		[4] */
	u_char	offset0;		/* Buffer offset (LSB).		[5] */
	u_char	param_len2;		/* Parameter length (MSB).	[6] */
	u_char	param_len1;		/* Parameter length (MID).	[7] */
	u_char	param_len0;		/* Parameter length (LSB).	[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} ALL_WRITE_BUFFER_CDB;

#define ALL_WRITE_BUFFER_OP	0x3B

/*
 * Define WRITE BUFFER Mode Field Definitions:
 */
#define ALL_WTBUF_COMBINED_HEADER	0x00
#define ALL_WTBUF_VENDOR_SPECIFIC	0x01
#define ALL_WTBUF_WRITE_DATA		0x02
#define ALL_WTBUF_DOWNLOAD_MICROCODE	0x04
#define ALL_WTBUF_DOWNLOAD_SAVE		0x05

/* ---------------------------------------------------------------------- */

#endif /* _SCSI_ALL_ */
