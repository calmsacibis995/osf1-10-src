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
 * @(#)$RCSfile: scsi_sequential.h,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/05/24 16:47:15 $
 */
#ifndef _SCSI_SEQ_
#define _SCSI_SEQ_

/* ---------------------------------------------------------------------- */

/* scsi_chap9.h		Version 1.00			Mar. 01, 1991 

   Contains all the structure and defines for the SCSI II spec
   chapter 9. Sequential access devices.


Modification History

	Version	Date		Who	Reason

	1.00	03/01/91	dallas	Created this module. 
	1.01	11/19/91	dallas  Added new scsi density codes.

/* ---------------------------------------------------------------------- */
/* Include files. */


/* ---------------------------------------------------------------------- */

/* 
 * Erase command cdb 6 bytes
 */
typedef struct seq_erase_cdb6 {
	u_char	opcode;		/* 0x19					*/
	u_char	extend	:1,	/* Erase from position to end (long)	*/
		immed	:1,	/* Return immediate			*/
			:3,	/* 3 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char	control;	/* Control byte				*/
}SEQ_ERASE_CDB6;

/*
 * Erase opcode define
 */
#define SEQ_ERASE_OP	0x19


/* ---------------------------------------------------------------------- */

/* 
 * Load/unload command cdb 6 byte
 */
typedef struct seq_load_cdb6 {
	u_char	opcode;		/* 0x1b					*/
	u_char	immed	:1,	/* Return immediate			*/
			:4,	/* 3 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char	load	:1,	/* Load tape/unload 1 = load		*/
		reten	:1,	/* Retension tape			*/
		eot	:1,	/* Unload at eot			*/
			:5;	/* 5 Bits reserved			*/
	u_char	control;	/* Control byte				*/
}SEQ_LOAD_CDB6;


/*
 * LOAD/UNLOAD opcode define
 */
#define SEQ_LOAD_OP	0x1b


/* ---------------------------------------------------------------------- */

/* 
 * Locate
 */

/* ---------------------------------------------------------------------- */

/* 
 * Read command cdb 6 byte (tapes only support 6 byte read cdb's)
 */
typedef struct seq_read_cdb6 {
	u_char	opcode;		/* 0x08					*/
	u_char	fixed	:1,	/* Fixed block or variable length	*/
		sili	:1,	/* Suppress illegal length indicator	*/
			:3,	/* 3 Bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char	trans_len2;	/* MSB of transfer length		*/
	u_char	trans_len1;	/* MID of transfer length		*/
	u_char	trans_len0;	/* LSB of transfer length		*/
	u_char	control;	/* Control byte				*/
}SEQ_READ_CDB6;

/* 
 * Read opcode define
 */
#define SEQ_READ_OP	0x8


/*
 * Take an transfer count and place it into a 6 byte read cdb SEQTRANS_TO_READ6
 */

#define SEQTRANS_TO_READ6(trans,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (trans); \
	((SEQ_READ_CDB6 *)(cdb))->trans_len0 = tmp.c[0]; \
	((SEQ_READ_CDB6 *)(cdb))->trans_len1 = tmp.c[1]; \
	((SEQ_READ_CDB6 *)(cdb))->trans_len2 = tmp.c[2]; \
}





/* ---------------------------------------------------------------------- */

/* 
 * Read block limits command cdb 6 byte 
 */
typedef struct seq_block_limits_cdb6 {
	u_char	opcode;		/* 0x05					*/
	u_char		:5,	/* 5 Bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char	control;	/* Control byte				*/
}SEQ_BLOCK_LIMITS_CDB6;

/* 
 * Read Block limits opcode define
 */
#define SEQ_BLOCK_LIMITS_OP	0x5


/* 
 * The data format that comes back for the command.
 */
typedef struct seq_block_limits_data {
	u_char			:8;	/* Reserved			*/
	u_char	max_blk_len2;		/* MSB maximum block length	*/
	u_char	max_blk_len1;		/* MID maximum block length	*/
	u_char	max_blk_len0;		/* LSB maximum block length	*/
	u_char	min_blk_len2;		/* MSB minimun block length	*/
	u_char	min_blk_len0;		/* LSB minimun block length	*/
}SEQ_BLOCK_LIMITS_DATA;


/* ---------------------------------------------------------------------- */

/*
 * Read position
 */


/* ---------------------------------------------------------------------- */

/* 
 * Read reverse
 */


/* ---------------------------------------------------------------------- */

/*
 * Recover buffered data
 */


/* ---------------------------------------------------------------------- */

/*
 * Release unit command cdb 6 byte
 */
typedef struct seq_release_cdb6 {
	u_char	opcode;		/* 0x17					*/
	u_char			:1,	/* 1 bit reserved		*/
		third_dev_id	:3,	/* Third party dev id		*/
		third_pat	:1,	/* Third party bit		*/
		lun		:3;	/* Logical unit number		*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char	control;		/* Control byte			*/
}SEQ_RELEASE_CDB6;

/*
 * Release opcode
 */
#define	SEQ_RELEASE_OP		0x17

/* ---------------------------------------------------------------------- */

/* 
 * Reserve command
 */

/* 
 * Reserve commnad cdb 6 byte 
 */
typedef struct seq_reserve_cdb6 {
	u_char	opcode;		/* 0x16					*/
	u_char		:1,		/* 1 bit reserved		*/
		third_dev_id	:3,	/* Third party dev id		*/
		third_pat	:1,	/* Third party bit		*/
		lun		:3;	/* Logical unit number		*/
	u_char	control;		/* Control byte			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
}SEQ_RESERVE_CDB6;

/*
 * Reserve opcode
 */
#define	SEQ_RESERVE_OP		0x16


/* ---------------------------------------------------------------------- */

/* 
 * Rewind command cdb 6 byte
 */
typedef struct seq_rewind_cdb6 {
	u_char	opcode;		/* 0x01					*/
	u_char	immed	:1,	/* Immediate bit */
	u_char		:4,	/* 4 Bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char	control;	/* Control byte				*/
}SEQ_REWIND_CDB6;

/* 
 * Rewind opcode define
 */
#define SEQ_REWIND_OP	0x1

/* ---------------------------------------------------------------------- */

/*
 * Space command cdb 6 byte
 */
typedef struct seq_space_cdb6 {
	u_char	opcode;		/* 0x11					*/
	u_char	code	:3,	/* Code record filemark ,etc		*/
	u_char		:2,	/* 2 Bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char	count2;		/* MSB of count (howmany) and direction */
	u_char	count1;		/* MID of count (howmany) and direction */
	u_char	count0;		/* LSB of count (howmany) and direction */
	u_char	control;	/* Control byte				*/
}SEQ_SPACE_CDB6;

/* 
 * SPACE opcode define
 */
#define SEQ_SPACE_OP	0x11

/* 
 * Defines for the code field (octal)
 */
#define SEQ_SPACE_BLOCKS	00
#define SEQ_SPACE_FILEMARKS	01
#define SEQ_SPACE_ENDDATA	03

/*
 * Take an space count and place it into a 6 byte space  cdb 
 * SEQ_COUNT_TO_SPACECDB6
 */

#define SEQ_COUNT_TO_SPACECDB6(count,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (count); \
	((SEQ_SPACE_CDB6 *)(cdb))->count0 = tmp.c[0]; \
	((SEQ_SPACE_CDB6 *)(cdb))->count1 = tmp.c[1]; \
	((SEQ_SPACE_CDB6 *)(cdb))->count2 = tmp.c[2]; \
}

/* ---------------------------------------------------------------------- */

/* 
 * Verify
 */


/* ---------------------------------------------------------------------- */

/* 
 * Write command cdb 6 byte (tapes only support 6 byte Write cdb's)
 */
typedef struct seq_write_cdb6 {
	u_char	opcode;		/* 0x0a					*/
	u_char	fixed	:1,	/* Fixed block or variable length	*/
			:4,	/* 4 Bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char	trans_len2;	/* MSB of transfer length		*/
	u_char	trans_len1;	/* MID of transfer length		*/
	u_char	trans_len0;	/* LSB of transfer length		*/
	u_char	control;	/* Control byte				*/
}SEQ_WRITE_CDB6;

/* 
 * WRITE opcode define
 */
#define SEQ_WRITE_OP	0xa




/*
 * Take an lbn and place it into a 6 byte write  cdb SEQLBN_TO_WRITE6
 */

#define SEQTRANS_TO_WRITE6(trans,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (trans); \
	((SEQ_WRITE_CDB6 *)(cdb))->trans_len0 = tmp.c[0]; \
	((SEQ_WRITE_CDB6 *)(cdb))->trans_len1 = tmp.c[1]; \
	((SEQ_WRITE_CDB6 *)(cdb))->trans_len2 = tmp.c[2]; \
}




/* ---------------------------------------------------------------------- */

/* 
 * Write filemarks command cdb 6 byte
 */
typedef struct seq_writemarks_cdb6 {
	u_char	opcode;		/* 0x10					*/
	u_char	immed	:1,	/* Immediate bit			*/
		wsmk	:1,	/* Write setmarks 			*/
			:3,	/* 3 bits reserved			*/
		lun	:1;	/* Logical unit number			*/
	u_char	trans_len2;	/* MSB of number of marks		*/
	u_char	trans_len1;	/* MID of number of marks		*/
	u_char	trans_len0;	/* LSB of number of marks		*/
	u_char	control;	/* Control byte				*/
}SEQ_WRITEMARKS_CDB6;

/* 
 * Opcode define
 */
#define SEQ_WRITEMARKS_OP	0x10

/*
 * Take an count and place it into a 6 byte writemarks cdb 
 * SEQ_COUNT_TO_WFM_CDB6
 */

#define SEQ_COUNT_TO_WFM_CDB6(count,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (count); \
	((SEQ_WRITEMARKS_CDB6 *)(cdb))->trans_len0 = tmp.c[0]; \
	((SEQ_WRITEMARKS_CDB6 *)(cdb))->trans_len1 = tmp.c[1]; \
	((SEQ_WRITEMARKS_CDB6 *)(cdb))->trans_len2 = tmp.c[2]; \
}

/* ---------------------------------------------------------------------- */

/*
 * Verify Command
 */
typedef struct seq_verify_cdb6 {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char	fixed	: 1,		/* Fixed Block/Variable Length.	[1] */
		bytcmp	: 1,		/* Byte Compare or CRC/ECC only.    */
		immed	: 1,		/* Complete Immediate (1 = Yes).    */
			: 2,		/* Reserved.			    */
		lun	: 3;		/* Logical Unit Number.		    */
	u_char	verflen2;		/* Verification Length.		[2] */
	u_char	verflen1;		/* Verification Length.		[3] */
	u_char	verflen0;		/* Verification Length.		[4] */
	u_char	link	: 1,		/* Link.			[5] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
} SEQ_VERIFY_CDB6;

#define SEQ_VERIFY_OP	0x13

/* ---------------------------------------------------------------------- */

/*
 * Parameters for sequential access devices - These are the pages
 */

/* ---------------------------------------------------------------------- */


/* 
 * The diagnostic  parameters  used with direct access devices.
 */
/* 
 * Translate Address page - Send Diagnostic
 */

/* 
 * Translate Address page - Receive Diagnostic
 */
/* ---------------------------------------------------------------------- */

/* 
 * Log Parameters
 */

/* ---------------------------------------------------------------------- */

/*
 * The mode parmater pages for sequential access devices
 */

/*
 * Page code field defines
 */
#define SEQ_NO_PAGE		0x00	/* No page			*/
#define SEQ_PGM_ERR_RECOV	0x01	/* The error recovery page	*/
#define SEQ_PGM_DEV_CONF	0x10	/* Device configuration page	*/
#define SEQ_PGM_PART1		0x11	/* Partition page 1		*/
#define SEQ_PGM_PART2		0x12	/* Partition page 2		*/
#define SEQ_PGM_PART3		0x13	/* Partition page 3		*/
#define SEQ_PGM_PART4		0x14	/* Partition page 4		*/

/* 
 * The mode parameter header, 6 byte cdb
 */
typedef struct seq_mode_head6 {
	u_char	mode_len;		/* The length of the mode head	*/
	u_char	medium_type;		/* The medium type		*/
	u_char	speed		:4,	/* Tape speed			*/
		buf_mode	:3,	/* Buffered mode setting	*/
		wp		:1;	/* Write protected		*/
	u_char	blk_desc_len;		/* Len of the descriptor block	*/
}SEQ_MODE_HEAD6;

/* 
 * The mode parameter header, 10 byte cdb
 */
typedef struct seq_mode_head10 {
	u_char	mode_len1;		/* The length of mode head MSB	*/
	u_char	mode_len0;		/* The length of mode head LSB	*/
	u_char	medium_type;		/* The medium type		*/
	u_char	speed		:4,	/* Tape speed			*/
		buf_mode	:3,	/* Buffered mode setting	*/
		wp		:1;	/* Write protected		*/
	u_char		:8;		/* Byte reserved		*/	
	u_char		:8;		/* Byte reserved		*/	
	u_char	blk_desc_len1;		/* Len of the desc block MSB	*/
	u_char	blk_desc_len0;		/* Len of the desc block LSB	*/
}SEQ_MODE_HEAD10;



/* 
 * The Block Descriptor for both 6 and 10 byte MODE PARAMETERS.
 */

typedef struct seq_mode_desc {
	u_char	density_code;	/* The density code Tapes		*/
	u_char	num_blocks2;	/* MSB of number of blocks 		*/
	u_char	num_blocks1;	/* Middle of number of blocks		*/
	u_char	num_blocks0;	/* LSB of number of blocks		*/
	u_char	reserved;	/* reserved				*/
	u_char	block_len2;	/* MSB of block length			*/
	u_char	block_len1;	/* Middle of block length		*/
	u_char	block_len0;	/* LSB of block length			*/
}SEQ_MODE_DESC;

/*
 * Take an block size and place it into a block desc SEQBLK_SIZE_TO_DESC
 */

#define SEQBLK_SIZE_TO_DESC(blk_size,desc); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (blk_size); \
	((SEQ_MODE_DESC *)(desc))->block_len0 = tmp.c[0]; \
	((SEQ_MODE_DESC *)(desc))->block_len1 = tmp.c[1]; \
	((SEQ_MODE_DESC *)(desc))->block_len2 = tmp.c[2]; \
}

/*
 * Block size to u_long
 */

#define SEQBLK_SIZE_TO_LONG(x,desc); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = 0; \
	tmp.c[0] = ((SEQ_MODE_DESC *)(desc))->block_len0; \
	tmp.c[1] = ((SEQ_MODE_DESC *)(desc))->block_len1; \
	tmp.c[2] = ((SEQ_MODE_DESC *)(desc))->block_len2; \
	(x) = tmp.l; \
}


/* 
 * The actual data structure for SEQ the mode select data.
 */

/*
 * For a mode parameter list 6 byte cdb Non scatter/gather
 */

typedef struct seq_mode_data6 {
	SEQ_MODE_HEAD6		sel_head;	/* The header		*/
	SEQ_MODE_DESC		sel_desc;	/* The descriptor piece	*/
	u_char	page_data[ALL_MAX_PAGE_SIZE];	/* Page data area	*/
}SEQ_MODE_DATA6;

/*
 * For a mode parameter list 6 byte cdb scatter/gather
 */

typedef struct seq_mode_data6_gather {
	SEQ_MODE_HEAD6		*sel_head;	/* The header		*/
	SEQ_MODE_DESC		*sel_desc;	/* The descriptor piece	*/
	u_char			*page_data;	/* Page data area	*/
}SEQ_MODE_DATA6_GATHER;

/*
 * For a mode parameter list 10 byte cdb NON scatter/gather
 */
typedef struct seq_mode_data10{
	SEQ_MODE_HEAD10		sel_head;	/* The header		*/
	SEQ_MODE_DESC		sel_desc;	/* The descriptor piece	*/
	u_char	page_data[ALL_MAX_PAGE_SIZE];	/* Page data area	*/
}SEQ_MODE_DATA10;

/*
 * For a mode parameter list 10 byte cdb scatter/gather
 */
typedef struct seq_mode_data10_gather {
	SEQ_MODE_HEAD10		*sel_head;	/* The header		*/
	SEQ_MODE_DESC		*sel_desc;	/* The descriptor piece	*/
	u_char			*page_data;	/* Page data area	*/
}SEQ_MODE_DATA10_GATHER;


/*
 * The mode page header
 */
typedef struct seq_mode_pg_hd {
	u_char	page_code	: 6;	/* define page function */
	u_char	reserved	: 1;
	u_char	ps		: 1;	/* Page savable. */
	u_char	page_length;		/* length of current page */
}SEQ_MODE_PG_HD;

/*
 * Tape density codes....
 * R = Reel to Reel, C = Cartridge, CS = Cassette
 */
#define	SEQ_DEF_BPI	0x00		/* 0x00 - Default density.	*/
#define	SEQ_800R_BPI	0x01		/* 0x01 - 800 BPI   (NRZI, R)	*/
#define	SEQ_1600R_BPI	0x02		/* 0x02 - 1600 BPI  (PE, R)	*/
#define	SEQ_6250R_BPI	0x03		/* 0x03 - 6250 BPI  (GCR, R)	*/
#define	SEQ_8000C_BPI	0x04		/* 0x04 - 8000 BPI  (GCR, C)	*/
#define	SEQ_8000R_BPI	0x05		/* 0x05 - 8000 BPI  (GCR, C)	*/
#define	SEQ_3200R_BPI	0x06		/* 0x06 - 3200 BPI  (PE, R)	*/
#define	SEQ_6400C_BPI	0x07		/* 0x07 - 6400 BPI  (IMFM, C)	*/
#define	SEQ_8000CS_BPI	0x08		/* 0x08 - 8000 BPI  (GCR, CS)	*/
#define	SEQ_38000C_BPI	0x09		/* 0x09 - 37871 BPI (GCR, C)	*/
#define	SEQ_6666C_BPI	0x0a		/* 0x0A - 6667 BPI  (MFM, C)	*/
#define	SEQ_1600C_BPI	0x0b		/* 0x0B - 1600 BPI  (PE, C)	*/
#define	SEQ_12690C_BPI	0x0c		/* 0x0C - 12690 BPI (GCR, C)	*/
#define	SEQ_QIC120_ECC	0x0d		/* 0x0D - QIC-120 with ECC.	*/
#define	SEQ_QIC150_ECC	0x0e		/* 0x0E - QIC-150 with ECC.	*/
#define	SEQ_QIC120	0x0f		/* 0x0F - QIC-120   (GCR, C)	*/
#define	SEQ_QIC150	0x10		/* 0x10 - QIC-150   (GCR, C)	*/
#define	SEQ_QIC320	0x11		/* 0x11 - QIC-320   (GCR, C)	*/
#define	SEQ_QIC1350	0x12		/* 0x12 - QIC-1350  (RLL, C)	*/
#define	SEQ_61000_BPI	0x13		/* 0x13 - 4mm Tape  (DDS, CS)	*/
#define	SEQ_54000_BPI	0x14		/* 0x14 - 8mm Tape  (???, CS)	*/
#define	SEQ_45434_BPI	0x15		/* 0x15 - TKZ09 8mm Tape        */
#define SEQ_10000_BPI	0x16		/* 0x16 - TK70 tape mfm		*/
#define SEQ_42500_BPI	0x17		/* 0x17 - TZ85 tape mfm		*/
#define SEQ_62500_BPI	0x19		/* 0x19 - TZ87 tape mfm		*/
#define SEQ_36000_BPI	0x1E		/* 0x1E - QIC-1GB TZK11 	*/
#define SEQ_40640_BPI	0x22		/* 0x22 - QIC-2GB TZK11		*/

/* ---------------------------------------------------------------------- */

/* 
 * Device configuration page
 */
typedef struct seq_dev_conf_pg {
	SEQ_MODE_PG_HD	pg_head;	/* Page header			*/
	u_char	act_format	:5,	/* Active format in use		*/
		caf		:1,	/* Change active format		*/
		cap		:1,	/* Change active partition	*/
				:1;	/* 1 bit reserved		*/
	u_char	act_part;		/* Active partition		*/
	u_char	wrt_buf_ratio;		/* Write buffer full ratio	*/
	u_char	rd_buf_ratio;		/* Read buffer empty ratio	*/
	u_char	wrt_delay1;		/* MSB Write delay time		*/
	u_char	wrt_delay0;		/* LSB Write delay time		*/
	u_char	rew		:1,	/* Report early warning of eom	*/
		rbo		:1,	/* recover buffer order		*/
		socf		:2,	/* stop on consecutive filemark	*/
		avc		:1,	/* automatic velocity control	*/
		rsmk		:1,	/* report set marks		*/
		bis		:1,	/* block identifiers supported	*/
		dbr		:1;	/* data buffer recovery		*/
	u_char	gap_size;		/* Gap size			*/
	u_char			:3,	/* 3 bits reserved		*/
		sew		:1,	/* Sync at early waring		*/
		eeg		:1,	/* Enable EOD generation	*/
		eod		:3;	/* End of data define		*/
	u_char	early_buf2;		/* MSB of reduce buffer at early*/
	u_char	early_buf1;		/* MID of reduce buffer at early*/
	u_char	early_buf0;		/* LSB of reduce buffer at early*/
	u_char	compr_algo;		/* Compression algorithm	*/
	u_char			:8;	/* Reserved			*/
}SEQ_DEV_CONF_PG;

/*
 * Defines for socf field
 */
#define SEQ_SOCF_PRE_RD		00	/* Continue reading donot stop	*/
#define SEQ_SOCF_STOP_RD1	01	/* Stop prefetch 1 mark		*/
#define SEQ_SOCF_STOP_RD2	02	/* Stop prefetch 2 marks	*/
#define SEQ_SOCF_STOP_RD3	03	/* Stop prefetch 3 marks	*/

/* 
 * defines for eod field (end of data define) (octal)
 */
#define SEQ_EOD_DEF		00	/* Default			*/
#define SEQ_EOD_ERASED		01	/* Erased data is EOD		*/
#define SEQ_EOD_SOCF		02	/* AS defined in socf field	*/
#define SEQ_EOD_NOTSUP		03	/* Not supported in this unit	*/

/* ---------------------------------------------------------------------- */

/* 
 * Medium Partition Pages
 */

/*
 * Partition size descriptors
 */
typedef struct seq_part_desc {
	u_char	part_size1;		/* MSB of partition size	*/
	u_char	part_size0;		/* LSB of partition size	*/
}SEQ_PART_DESC;


/* 
 * Medium Partition page 1 
 */
typedef struct seq_part1_pg {
	SEQ_MODE_PG_HD	pg_head;	/* Page header			*/
	u_char	max_add_parts;		/* Max additional partitions	*/
	u_char	add_parts;		/* Additional partitions	*/
	u_char			:3,	/* 3 bits reserved		*/
		psum		:2,	/* Partition size unit of measure*/
		idp		:1,	/* Initiator defined partition	*/
		sdp		:1,	/* select data partitions	*/
		fdp		:1;	/* Fixed data partitions	*/
	u_char	med_format_rec;		/* Media format recognition	*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
	SEQ_PART_DESC	part_desc[64];	/* Maximum of 64 partition desc	*/
}SEQ_PART1_PG;

/*
 * PSUM defines (octal)
 */
#define SEQ_PSUM_BYTES	00	/* Measure is in bytes			*/
#define SEQ_PSUM_KBYTES	01	/* Measure is in kilo bytes		*/
#define SEQ_PSUM_MBYTES	02	/* Measure is in mega bytes		*/

/*
 * Medium format recognition defines (octal)
 */
#define SEQ_MFR_NONE	00	/* Unit can't recognize format and parts*/
#define SEQ_MFR_FMT	01	/* Unit can recognize format		*/
#define SEQ_MFR_PART	02	/* Unit can recognize partitions	*/
#define SEQ_MFR_BOTH	03	/* Unit can recognize BOTH		*/

/* 
 * Medium Partition pages 2, 3, and 4.. All have the same format
 * Just the page code is different. 
 */
typedef struct seq_part234_pg {
	SEQ_MODE_PG_HD	pg_head;		/* Page header			*/
	SEQ_PART_DESC	part_desc[64]; /* Maximum of 64 partition desc	*/
}SEQ_PART234_PG;




/* ---------------------------------------------------------------------- */


/*
 * Read Write error recovery page
 */
typedef struct seq_err_recov_pg {
	SEQ_MODE_PG_HD	pg_head;	/* Page header			*/
	u_char	dcr		:1,	/* Disable corrections		*/
		dte		:1,	/* Disable transfers on error   */
		per		:1,	/* Post error recovery		*/
		err		:1,	/* Enable error recovery	*/
				:1,	/* 1 bit reserved		*/
		tb		:1,	/* Transfer block on error	*/
				:2;	/* 2 Bits reserved		*/
	u_char	rd_retry_cnt;		/* Read retry count		*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char	wrt_retry_cnt;		/* Write retry count		*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
}SEQ_ERR_RECOV_PG;

/* ---------------------------------------------------------------------- */

#endif /* _SCSI_SEQ_ */
