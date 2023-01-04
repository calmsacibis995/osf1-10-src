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
 * @(#)$RCSfile: scsi_direct.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/07/27 17:50:51 $
 */
#ifndef _SCSI_DIR_
#define _SCSI_DIR_

/* ---------------------------------------------------------------------- */

/* scsi_direct.h	Version 1.00			Mar. 01, 1991 

   Contains all the structure and defines for the SCSI II spec
   chapter 8. Direct access devices.


Modification History

	Version	Date		Who	Reason

	1.00	03/01/91	dallas	Created this module. 
	1.01	06/27/91	robin	Add verify structure & command.
	1.02	09/14/91	robin	Added 2 missing reserved fields
					to the Read Defect List CDB.

/* ---------------------------------------------------------------------- */
/*
 * Define Maximum Logical Block Address (LBA) for a 6-byte CDB.
 *
 *	0x1fffff = 2097151 512 byte blocks = 1,073,741,312 bytes
 *		or 1048575.50 Kbytes (K = 1024.)
 *		or 1023.99 Mbytes    (M = 1024 * 1024 = 1,048,576.)
 *		or 0.99 Gbytes       (G = 1048576 * 1024 = 1,073,741,824)
 */
#define DIR_MAX_LBA_CDB6      0x1fffff	/* Maximum 6-byte CDB LBA.	*/
#define DIR_MAX_LEN_CDB6	0xff	/* Maximum 6-byte CDB length.	*/

/* ---------------------------------------------------------------------- */

/* 
 * Format Unit defines.
 */

/* 
 * The format cdb 6 bytes
 */
typedef struct dir_format_cdb6 {
	u_char	opcode;			/* 0x04				*/
	u_char	defect_list_fmt	:3,	/* Defect list format	 	*/
		cmp_list	:1,	/* Supplied defect list complete*/
		fmt_data	:1,	/* format data list		*/
		lun		:3;	/* Logical unit number		*/
	u_char	vendor_specific;	/* vendor specific information	*/
	u_char	interleave1;		/* MSB interleave factor	*/
	u_char	interleave0;		/* LSB interleave factor	*/
	u_char	control;		/* control byte			*/
}DIR_FORMAT_CDB6;

/*
 * The opcode for format
 */
#define DIR_FORMAT_OP	0x04

/* 
 * The defect list format codes. PLEASE NOTE OCTAL values
 */
#define DIR_BLOCK_FMT	00	/* BLOCK FORMAT				*/
#define DIR_INDEX_FMT	04	/* BYTES FROM INDEX FORMAT		*/
#define DIR_SECTOR_FMT	05	/* SECTOR FORMAT			*/


/* 
 * The data that is supplied with the format command
 */

/* 
 * The defect header list
 */
typedef struct dir_defect_header {
	u_char		:8;	/* 8 bits reserved			*/
	u_char	vs	:1,	/* CHANGE				*/
		immed	:1,	/* Immediate				*/
		dsp	:1,	/* Disable saving parameters		*/
		ip	:1,	/* Initialization pattern		*/
		stpf	:1,	/* stop format				*/
		dcrt	:1,	/* disable certification		*/
		dpry	:1,	/* disable primary			*/
		fov	:1;	/* format options valid			*/
	u_char	def_list_len1;	/* MSB of defect list lenght		*/
	u_char	def_list_len0;	/* LSB of defect list lenght		*/
}DIR_DEFECT_HEADER;


/*
 * Defect descriptor block format
 */
typedef struct dir_defect_block {
	u_char	db_block_addr3;	/* MSB of block address			*/
	u_char	db_block_addr2;	/* Middle high of block address		*/
	u_char	db_block_addr1;	/* Middle low of block address		*/
	u_char	db_block_addr0;	/* LSB of block address			*/
}DIR_DEFECT_BLOCK;

/*
 * Defect descriptor index format
 */
typedef struct dir_defect_index {
	u_char	di_cylin_num2;	/* MSB of cylinder number of defect	*/
	u_char	di_cylin_num1;	/* MID of cylinder number of defect	*/
	u_char	di_cylin_num0;	/* LSB of cylinder number of defect	*/
	u_char	di_head_num;	/* Head number of defect		*/
	u_char	di_bytes_index3;/* MSB defect bytes from index		*/
	u_char	di_bytes_index2;/* MID HIGH defect bytes from index	*/
	u_char	di_bytes_index1;/* MID LOW defect bytes from index	*/
	u_char	di_bytes_index0;/* LSB defect bytes from index		*/
}DIR_DEFECT_INDEX;

/*
 * Defect descriptor sector format
 */
typedef struct dir_defect_sector {
	u_char	ds_cylin_num2;	/* MSB of cylinder number of defect	*/
	u_char	ds_cylin_num1;	/* MID of cylinder number of defect	*/
	u_char	ds_cylin_num0;	/* LSB of cylinder number of defect	*/
	u_char	ds_head_num;	/* Head number of defect		*/
	u_char	ds_sector_num3;	/* MSB defect sector number		*/
	u_char	ds_sector_num2;	/* MID HIGH defect sector number	*/
	u_char	ds_sector_num1;	/* MID LOW defect sector number		*/
	u_char	ds_sector_num0;	/* LSB defect sector number		*/
}DIR_DEFECT_SECTOR;

/*
 * Initialization pattern descriptor
 */
typedef struct dir_pattern_desc {
	u_char			:6,	/* reserved			*/
		ip_mod		:2;	/* Init pattern modifier	*/
	u_char	pattern_len1;		/* MSB of pattern length	*/
	u_char	pattern_len0;		/* LSB of pattern length	*/
	u_char	data_pattern[512];	/* CHANGE			*/
}DIR_PATTERN_DESC;

/* 
 * Initialization Pattern modifiers defines PLEASE NOTE octal
 */
#define DIR_IPM_NOHEAD		00	/* Donot modifier header	*/
#define DIR_IPM_HEAD_LOG	01	/* Write head logical		*/
#define DIR_IPM_HEAD_PHY	02	/* Write head physical		*/

/* 
 * Initialization pattern types
 */
#define DIR_PATTERN_DEF		0x00	/* Use the default pattern	*/
#define DIR_PATTERN_REPEAT	0x01	/* Repeat init pattern as require */




/*
 * Due to the hugh variations in the sizes of the format parameter list
 * USE the SCATTER gatter lists for each structure.
 */

/*
 * Format parameter list with a pattern descriptor 
 */
typedef struct dir_fmt_para_list_pat {
	DIR_DEFECT_HEADER	*defect_header;	/* ptr to defect header	*/
	DIR_PATTERN_DESC	*pattern_desc;	/* to pattern descriptor */
		/* 
		 *The following are pointer to arrays od defect 
		 * defect descriptors
		 */
	union {
		DIR_DEFECT_BLOCK	**defect_block;
		DIR_DEFECT_INDEX	**defect_index;
		DIR_DEFECT_SECTOR	**defect_sector;
	}defect_desc;
}DIR_FMT_PARA_LIST_PAT;

/*
 * Format parameter list without a pattern descriptor 
 */
typedef struct dir_fmt_para_list {
	DIR_DEFECT_HEADER	*defect_header;	/* ptr to defect header	*/
		/* 
		 *The following are pointer to arrays od defect 
		 * defect descriptors
		 */
	union {
		DIR_DEFECT_BLOCK	**defect_block;
		DIR_DEFECT_INDEX	**defect_index;
		DIR_DEFECT_SECTOR	**defect_sector;
	}defect_desc;
}DIR_FMT_PARA_LIST;

	

/* ---------------------------------------------------------------------- */

/* 
 * Lock/unlock cache cdb 10 byte
 */
typedef struct dir_cache_lock_cdb10 {
	u_char	opcode;		/* 0x36					*/
	u_char	reladr	:1,	/* Relative address bit.		*/
		lock	:1,	/* lock or unlock			*/
			:3,	/* 3 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn1;		/* MSB of logical block number		*/
	u_char	lbn0;		/* LSB of logical block number		*/
	u_char		:8;	/* reserved				*/
	u_char	num_blocks1;	/* MSB 0f number of blocks		*/
	u_char	num_blocks0;	/* LSB 0f number of blocks		*/
	u_char	control;	/* control byte				*/
}DIR_CACHE_LOCK_CDB10;

/* 
 * Lock/unlock cache opcode
 */
#define DIR_CACHE_LOCK_OP	0x36

/* ---------------------------------------------------------------------- */

/* 
 * Prevent allow medium removal 6 byte cdb
 */
typedef struct dir_prevent_cdb6 {
	u_char	opcode;		/* 0x1e					*/
	u_char		:5,	/* 5 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char	prevent	:1,	/* Prevent/allow			*/
			:7;	/* 7 bits reserved			*/
	u_char	control;	/* the control byte			*/
}DIR_PREVENT_CDB6;

/* 
 * Prevent allow opcode
 */
#define DIR_PREVENT_OP		0x1e

/*
 * Allow/Prevent Medium Removal Parameters:
 */
#define DIR_ALLOW_REMOVAL	0	/* Allow medium removal.	*/
#define DIR_PREVENT_REMOVAL	1	/* Prevent medium removal.	*/


/* ---------------------------------------------------------------------- */

/*
 * Read commands
 */

/* 
 * The read 6 byte cdb
 */
typedef struct dir_read_cdb6{
	u_char	opcode;		/* 0x08					*/
	u_char	lbn2	:5,	/* The MSB of the locical block number	*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn1;		/* MID of logical block number		*/
	u_char	lbn0;		/* LSB of logical block number		*/
	u_char	trans_len;	/* Transfer length in blocks		*/
	u_char	control;	/* the control byte			*/
}DIR_READ_CDB6;

/* 
 * Read 6 byte opcode
 */
#define DIR_READ6_OP	0x08

/*
 * Take an lbn and place it into a 6 byte read  cdb DIRLBN_TO_READ6
 */

#define DIRLBN_TO_READ6(lbn,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (lbn); \
	((DIR_READ_CDB6 *)(cdb))->lbn0 = (tmp).c[0]; \
	((DIR_READ_CDB6 *)(cdb))->lbn1 = (tmp).c[1]; \
	((DIR_READ_CDB6 *)(cdb))->lbn2 = ((tmp).c[2] & 0x1f); \
}

/*
 * Take a transfer count and place it into a 6 byte cdb DIRTRANS_TO_READ6
 */

#define DIRTRANS_TO_READ6(count,cdb); \
	((DIR_READ_CDB6 *)(cdb))->trans_len = ((count) & 0xff)  



/* 
 * The read 10 byte cdb
 */
typedef struct dir_read_cdb10 {
	u_char	opcode;		/* 0x28					*/
	u_char	reladr	:1,	/* Relative address bit			*/
			:2,	/* 2 bits reserved			*/
		fua	:1,	/* force unit access			*/
		dpo	:1,	/* disable page out (unit's cache)	*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn3;		/* MSB of logical block number		*/
	u_char	lbn2;		/* MID HIGH of logical block number	*/
	u_char	lbn1;		/* MID LOW of logical block number	*/
	u_char	lbn0;		/* LSB of logical block number		*/
	u_char		:8;	/* Reserved				*/
	u_char	tran_len1;	/* MSB Transfer length in blocks	*/
	u_char	tran_len0;	/* LSB Transfer length in blocks	*/
	u_char	control;	/* the control byte			*/
}DIR_READ_CDB10;

/* 
 * Read 10 byte opcode
 */
#define DIR_READ10_OP	0x28
/*
 * Take an lbn and place it into a 10 byte cdb LBN_TO_READ10
 */

#define DIRLBN_TO_READ10(lbn,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (lbn); \
	((DIR_READ_CDB10 *)(cdb))->lbn0 = tmp.c[0]; \
	((DIR_READ_CDB10 *)(cdb))->lbn1 = (tmp).c[1]; \
	((DIR_READ_CDB10 *)(cdb))->lbn2 = (tmp).c[2]; \
	((DIR_READ_CDB10 *)(cdb))->lbn3 = (tmp).c[3]; \
}

/*
 * Take a transfer count and place it into a 10 byte cdb DIRTRANS_TO_READ10
 */

#define DIRTRANS_TO_READ10(count,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (count); \
	((DIR_READ_CDB10 *)(cdb))->tran_len0 = tmp.c[0]; \
	((DIR_READ_CDB10 *)(cdb))->tran_len1 = tmp.c[1]; \
}


/* ---------------------------------------------------------------------- */

/* 
 * Read Capacity defines
 */

/*
 * Read capacity cdb 10 bytes
 */
typedef struct dir_read_cap_cdb10 {
	u_char	opcode;		/* 0x25					*/
	u_char	reladr	:1,	/* Relative address bit			*/
			:4,	/* 4 bits reserved			*/
		lun	:3;	/* logical unit number			*/
	u_char	lbn3;		/* MSB of logical block address		*/
	u_char	lbn2;		/* MID HIGH of logical block address	*/
	u_char	lbn1;		/* MID LOW of logical block address	*/
	u_char	lbn0;		/* LSB of logical block address		*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char		:1,	/* Partial medium indicator		*/
			:7;	/* 7 bits reserved			*/
	u_char	control;	/* the control byte			*/
}DIR_READ_CAP_CDB10;

/* 
 * The read capacity opcode define
 */
#define DIR_READCAP_OP	0x25


/* 
 * Read capacity data returned
 */
typedef struct dir_read_cap_data {
	u_char	lbn3;		/* MSB of number of logical blocks	*/
	u_char	lbn2;		/* MID HIGH of number of logical blocks	*/
	u_char	lbn1;		/* MID LOW of number of logical blocks	*/
	u_char	lbn0;		/* LSB of number of logical blocks	*/
	u_char	block_len3;	/* MSB of block length in bytes		*/
	u_char	block_len2;	/* MID HIGH of block length in bytes	*/
	u_char	block_len1;	/* MID LOW of block length in bytes	*/
	u_char	block_len0;	/* MSB of block length in bytes		*/
}DIR_READ_CAP_DATA;



/* ---------------------------------------------------------------------- */

/* 
 * Read defect
 */

/* 
 * Read defect cdb 10 bytes
 */
typedef struct dir_read_defect_cdb10 {
	u_char	opcode;			/* Operation Code.		[0] */
	u_char			: 5,	/* Reserved.			[1] */
		lun		: 3;	/* Logical Unit Number.		    */
	u_char	defect_list_fmt	: 3,	/* Defect List Format.		[2] */
		glist		: 1,	/* Grown Defect List.		    */
		plist		: 1,	/* primary defect list		    */
				: 3;	/* Reserved.			    */
	u_char			: 8;	/* Reserved.			[3] */
	u_char			: 8;	/* Reserved.			[4] */
	u_char			: 8;	/* Reserved.			[5] */
	u_char			: 8;	/* Reserved.			[6] */
	u_char	alloc_len1;		/* Allocation Length (MSB).	[7] */
	u_char	alloc_len0;		/* Allocation Length (LSB).	[8] */
	u_char	link	: 1,		/* Link.			[9] */
		flag	: 1,		/* Flag.			    */
			: 4,		/* Reserved.			    */
		vendor	: 2;		/* Vendor Unique.		    */
}DIR_READ_DEFECT_CDB10;

/*
 * Read defect opcode
 */
#define DIR_READ_DEFECT_OP	0x37

/* 
 * The read defect data (returned from read defect cmd)
 * PLEASE NOTE YOU DON'T Have to use SCATTER/GATHER list for this
 * struct. See below for one that you do have to.
 */
typedef struct dir_defect_data {
	u_char			:8;	/* reserved			*/
	u_char	defect_list_fmt	:3,	/* Defect list format		*/
		glist		:1,	/* Grown defect list		*/
		plist		:1,	/* primary defect list		*/
				:3;	/* 3 bits reserved		*/
	u_char	list_len1;		/* MSB of list length		*/
	u_char	list_len0;		/* MSB of list length		*/
		/* 
		 * YES I know I hard coded the numbers..... The index
		 * and sector structs are 8 bytes and the block is
		 * 4 bytes.. With 2 bytes for length (512) max bytes
		 * for all lists.
		 */
	union {
		DIR_DEFECT_BLOCK	defect_block[128];
		DIR_DEFECT_INDEX	defect_index[64];
		DIR_DEFECT_SECTOR	defect_sector[64];
	}defect_desc;
}DIR_DEFECT_DATA;

/* 
 * The read defect data (returned from read defect cmd)
 * PLEASE NOTE MUST USE SCATTER/GATHER LIST 
 */
typedef struct dir_defect_data_gather {
	u_char			:8;	/* reserved			*/
	u_char	defect_list_fmt	:3,	/* Defect list format		*/
		glist		:1,	/* Grown defect list		*/
		plist		:1,	/* primary defect list		*/
				:3;	/* 3 bits reserved		*/
	u_char	list_len1;		/* MSB of list length		*/
	u_char	list_len0;		/* MSB of list length		*/
	union {
		DIR_DEFECT_BLOCK	**defect_block;
		DIR_DEFECT_INDEX	**defect_index;
		DIR_DEFECT_SECTOR	**defect_sector;
	}defect_desc;
}DIR_DEFECT_DATA_GATHER;


/* ---------------------------------------------------------------------- */

/* 
 * Read long cdb 10 bytes
 */
typedef struct dir_read_long_cdb10 {
	u_char	opcode;		/* 0x3e					*/
	u_char	reladr	:1,	/* Relative address bit			*/
		correct	:1,	/* Read without correction		*/
			:3,	/* 3 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn3;		/* MSB of logical block address		*/
	u_char	lbn2;		/* MID HIGH of logical block address	*/
	u_char	lbn1;		/* MID LOW of logical block address	*/
	u_char	lbn0;		/* LSB of logical block address		*/
	u_char		:8;	/* Reserved				*/
	u_char	trans_len1;	/* MSB of transfer length in BYTES	*/
	u_char	trans_len0;	/* LSB of transfer length in BYTES	*/
	u_char	control;	/* Control byte				*/
}DIR_READ_LONG_CDB10;

/*
 * Read long opcode
 */
#define DIR_READ_LONG_OP	0x3e



/* ---------------------------------------------------------------------- */

/* 
 * Reassign blocks
 */

/* 
 * Reassign block cdb 6 bytes
 */
typedef struct dir_reassign_cdb6 {
	u_char	opcode;		/* 0x07					*/
	u_char		:5,	/* 5 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char	control;	/* control byte				*/
}DIR_REASSIGN_CDB6;

/* 
 * REASSIGN opcode define
 */
#define	DIR_REASSIGN_OP	0x07


/*
 * Defect descriptors
 */
typedef struct dir_defect_desc {
	u_char	lbn3;	/* MSB of logical block address			*/
	u_char	lbn2;	/* MID HIGH of logical block address		*/
	u_char	lbn1;	/* MID LOW of logical block address		*/
	u_char	lbn0;	/* LSB of logical block address			*/
}DIR_DEFECT_DESC;

/* 
 * Reassign block defect list header 
 */
typedef struct dir_reas_defect_header {
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char	list_len1;	/* MSB of length of defect list (BYTES)	*/
	u_char	list_len0;	/* LSB of length of defect list (BYTES)	*/
}DIR_REAS_DEFECT_HEADER;

/* 
 * Reassign block defect list
 * PLEASE NOTE NON SCATTER/GATHER
 */
typedef struct dir_defect_list {
	DIR_REAS_DEFECT_HEADER	defect_header;	/* The defect header	*/
	DIR_DEFECT_DESC	defect_desc[128]; 	/* MAXIMUN number	*/
}DIR_DEFECT_LIST;

/* 
 * Reassign block defect list
 * PLEASE NOTE SCATTER/GATHER
 */
typedef struct dir_defect_list_gather {
	DIR_REAS_DEFECT_HEADER	defect_header;	/* The defect header	*/
	DIR_DEFECT_DESC		**defect_desc; 	/* pointer to list of them*/
}DIR_DEFECT_LIST_GATHER;



/* ---------------------------------------------------------------------- */

/*
 * Release command 
 */

/* 
 * Release command cdb 6 byte NOT Extented
 */
typedef struct dir_release_cdb6 {
	u_char	opcode;		/* 0x17					*/
	u_char	extent		:1,	/* Extended release (optional)	*/
		third_dev_id	:3,	/* Third party dev id		*/
		third_pat	:1,	/* Third party bit		*/
		lun		:3;	/* Logical unit number		*/
	u_char	resrv_id;		/* reservation ID		*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char	control;		/* Control byte			*/
}DIR_RELEASE_CDB6;

/*
 * Release opcode
 */
#define	DIR_RELEASE_OP		0x17

/* ---------------------------------------------------------------------- */

/* 
 * Reserve command
 */

/* 
 * Reserve commnad cdb 6 byte 
 */
typedef struct dir_reserve_cdb6 {
	u_char	opcode;		/* 0x16					*/
	u_char	extent		:1,	/* Extended release (optional)	*/
		third_dev_id	:3,	/* Third party dev id		*/
		third_pat	:1,	/* Third party bit		*/
		lun		:3;	/* Logical unit number		*/
	u_char	resrv_id;		/* reservation ID		*/
	u_char	extent_len1;		/* MSB of Extented list length	*/
	u_char	extent_len0;		/* LSB of Extented list length	*/
	u_char	control;		/* Control byte			*/
}DIR_RESERVE_CDB6;


/*
 * Reserve opcode
 */
#define	DIR_RESERVE_OP		0x16



/* 
 * The Extended list (format of)
 */
typedef struct dir_extend_format {
	u_char	resrv_type	:2,	/* reservation type		*/
		reladr		:1,	/* Relative address bit		*/
				:5;	/* 5 bits reserved		*/
	u_char	num_blocks2;		/* MSB of number of blocks	*/
	u_char	num_blocks1;		/* MID of number of blocks	*/
	u_char	num_blocks0;		/* LOW of number of blocks	*/
	u_char	lbn3;			/* MSB of logical block address	*/
	u_char	lbn2;			/* MID HIGH of logical block addr*/
	u_char	lbn1;			/* MID LOW of logical block addr*/
	u_char	lbn0;			/* LSB of logical block addr	*/
}DIR_EXTEND_FORMAT;

/*
 * Defines for reservation type PLEASE NOTE OCTAL
 */
#define	DIR_RESRV_RD_SHARE	00
#define DIR_RESRV_WRT_EXCLUS	01
#define DIR_RESRV_RD_EXCLUS	02
#define DIR_RESRV_EXCLUS	03

/*
 * The non scatter/gather reserve list struct.
 */
typedef struct dir_extend_data {
	DIR_EXTEND_FORMAT	extend_data[64];	/* Maximun number*/
}DIR_EXTEND_DATA;


/* 
 * The scatter/gather reserve list struct
 */
typedef struct dir_extend_data_gather {
	DIR_EXTEND_FORMAT	**extend_data;		/* list of them	*/
}DIR_EXTEND_DATA_GATHER;



/* ---------------------------------------------------------------------- */

/*
 * REZERO unit
 */

/* ---------------------------------------------------------------------- */

/* 
 * SEARCH data cmds
 */

/* ---------------------------------------------------------------------- */

/* 
 * SEEK cmds
 */

/* 
 * The seek command cdb 6 byte
 */
typedef struct dir_seek_cdb6 {
	u_char	opcode;		/* 0x0b					*/
	u_char	lbn2	:5,	/* MSB of logical block address		*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn1;		/* MID of logical block address		*/
	u_char	lbn0;		/* LSB of logical block address		*/
	u_char		:8;	/* reserved				*/
	u_char	control;	/* The control byte			*/
}DIR_SEEK_CDB6;

/* 
 * SEEK opcode define 6 byte cdb
 */
#define DIR_SEEK6_OP	0x0b

/* 
 * The seek command cdb 10 byte
 */
typedef struct dir_seek_cdb10{
	u_char	opcode;		/* 0x2b					*/
	u_char		:5,	/* 5 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn3;		/* MSB of logical block address		*/
	u_char	lbn2;		/* MID HIGH of logical block address	*/
	u_char	lbn1;		/* MID LOW of logical block address	*/
	u_char	lbn0;		/* LSB of logical block address		*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char		:8;	/* reserved				*/
	u_char	control;	/* The control byte			*/
}DIR_SEEK_CDB10;

/* 
 * SEEK opcode define 10 byte cdb
 */
#define DIR_SEEK10_OP	0x2b

/* ---------------------------------------------------------------------- */

/* 
 * SET LIMITS
 */

/* ---------------------------------------------------------------------- */

/* 
 * START/STOP unit 
 */

/* 
 * START/STOP unit command  cdb 6 byte
 */
typedef struct dir_start_cdb6 {
	u_char	opcode;		/* 0x1b					*/
	u_char	immed	:1,	/* Immediate bit			*/
			:4,	/* 4 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
	u_char	start	:1,	/* Start unit bit			*/
		loej	:1,	/* The load eject bit			*/
			:6;	/* 6 bits reserved			*/
	u_char	control;	/* The control byte			*/
}DIR_START_CDB6;

/* 
 * START/STOP unit opcode define
 */
#define DIR_START_OP	0x1b

/*
 * Start/Stop Unit Parameters:
 */
#define DIR_STOP_UNIT		0	/* Stop the unit.		*/
#define DIR_START_UNIT		1	/* Start the unit.		*/


/* ---------------------------------------------------------------------- */

/*
 * SYNCHRONIZE CACHE command
 */


/* ---------------------------------------------------------------------- */

/*
 * VERIFY Command
 */
typedef struct dir_verify_cdb10 {
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
} DIR_VERIFY_CDB10;

#define DIR_VERIFY_OP	0x2F

/* ---------------------------------------------------------------------- */

/* 
 * WRITE commands
 */

/*
 * Write command cdb 6 byte
 */
typedef struct dir_write_cdb6 {
	u_char	opcode;		/* 0x0a					*/
	u_char	lbn2	:5,	/* MSB of logical block address		*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn1;		/* MID of logical block address		*/
	u_char	lbn0;		/* LSB of logical block address		*/
	u_char	trans_len;	/* transfer length in blocks		*/
	u_char	control;	/* The control byte			*/
}DIR_WRITE_CDB6;

/*
 * Write command opcode
 */
#define	DIR_WRITE6_OP	0x0a


/*
 * Take an lbn and place it into a 6 byte write  cdb DIRLBN_TO_WRITE6
 */

#define DIRLBN_TO_WRITE6(lbn,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (lbn); \
	((DIR_WRITE_CDB6 *)(cdb))->lbn0 = tmp.c[0]; \
	((DIR_WRITE_CDB6 *)(cdb))->lbn1 = (tmp).c[1]; \
	((DIR_WRITE_CDB6 *)(cdb))->lbn2 = ((tmp).c[2] & 0x1f); \
}

/*
 * Take a transfer count and place it into a 6 byte cdb DIRTRANS_TO_WRITE6
 */

#define DIRTRANS_TO_WRITE6(count,cdb); \
	((DIR_WRITE_CDB6 *)(cdb))->trans_len = ((count) & 0xff)  


/*
 * Write command cdb 10 byte
 */
typedef struct dir_write_cdb10 {
	u_char	opcode;		/* 0x2a					*/
	u_char	reladr	:1,	/* Relative address bit			*/
			:2,	/* 2 bits reserved			*/
		fua	:1,	/* force unit access			*/
		dpo	:1,	/* Disable page out (disk cache)	*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn3;		/* MSB of logical block number		*/
	u_char	lbn2;		/* MID HIGH of logical block number	*/
	u_char	lbn1;		/* MID LOW of logical block number	*/
	u_char	lbn0;		/* LSB of logical block number		*/
	u_char		:8;	/* Reserved				*/
	u_char	tran_len1;	/* MSB Transfer length in blocks	*/
	u_char	tran_len0;	/* LSB Transfer length in blocks	*/
	u_char	control;	/* the control byte			*/
}DIR_WRITE_CDB10;

/* 
 * Write 10 byte opcode
 */
#define DIR_WRITE10_OP	0x2a
/*
 * Take an lbn and place it into a 10 byte cdb DIRLBN_TO_WRITE10
 */

#define DIRLBN_TO_WRITE10(lbn,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (lbn); \
	((DIR_WRITE_CDB10 *)(cdb))->lbn0 = tmp.c[0]; \
	((DIR_WRITE_CDB10 *)(cdb))->lbn1 = (tmp).c[1]; \
	((DIR_WRITE_CDB10 *)(cdb))->lbn2 = (tmp).c[2]; \
	((DIR_WRITE_CDB10 *)(cdb))->lbn3 = (tmp).c[3]; \
}

/*
 * Take a transfer count and place it into a 10 byte cdb DIRTRANS_TO_WRITE10
 */

#define DIRTRANS_TO_WRITE10(count,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (count); \
	((DIR_WRITE_CDB10 *)(cdb))->tran_len0 = tmp.c[0]; \
	((DIR_WRITE_CDB10 *)(cdb))->tran_len1 = tmp.c[1]; \
}



/* ---------------------------------------------------------------------- */

/*
 * WRITE and VERIFY command
 */
typedef struct dir_write_vrfy_cdb10 {
	u_char	opcode;		/* 0x2e					*/
	u_char	reladr	:1,	/* Relative address bit			*/
		bytchk  :1,	/* Byte check				*/
		resv1	:1,	/*  bit reserved			*/
		resv2	:1,	/*  bit reserved			*/
		dpo	:1,	/* Disable page out (disk cache)	*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn3;		/* MSB of logical block number		*/
	u_char	lbn2;		/* MID HIGH of logical block number	*/
	u_char	lbn1;		/* MID LOW of logical block number	*/
	u_char	lbn0;		/* LSB of logical block number		*/
	u_char		:8;	/* Reserved				*/
	u_char	tran_len1;	/* MSB Transfer length in blocks	*/
	u_char	tran_len0;	/* LSB Transfer length in blocks	*/
	u_char	control;	/* the control byte			*/
}DIR_WRITE_VRFY_CDB10;

/* 
 * Write 10 byte opcode
 */
#define DIR_WRITE_VRFY_10_OP	0x2e

/*
 * Take an lbn and place it into a 10 byte cdb DIRLBN_TO_WRITE10
 */

#define DIRLBN_TO_WRITE_VRFY10(lbn,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (lbn); \
	((DIR_WRITE_CDB10 *)(cdb))->lbn0 = tmp.c[0]; \
	((DIR_WRITE_CDB10 *)(cdb))->lbn1 = (tmp).c[1]; \
	((DIR_WRITE_CDB10 *)(cdb))->lbn2 = (tmp).c[2]; \
	((DIR_WRITE_CDB10 *)(cdb))->lbn3 = (tmp).c[3]; \
}
/*
 * Take a transfer count and place it into a 10 byte cdb DIRTRANS_TO_WRITE10
 */

#define DIRTRANS_TO_WRITE_VRFY10(count,cdb); { \
	union { \
		unsigned char c[4]; \
		U32		 l; \
	}tmp; \
	tmp.l = (count); \
	((DIR_WRITE_CDB10 *)(cdb))->tran_len0 = tmp.c[0]; \
	((DIR_WRITE_CDB10 *)(cdb))->tran_len1 = tmp.c[1]; \
}

/* ---------------------------------------------------------------------- */

/* 
 * Write long cdb 10 bytes
 */
typedef struct dir_write_long_cdb10 {
	u_char	opcode;		/* 0x3f					*/
	u_char	reladr	:1,	/* Relative address bit			*/
			:4,	/* 4 bits reserved			*/
		lun	:3;	/* Logical unit number			*/
	u_char	lbn3;		/* MSB of logical block address		*/
	u_char	lbn2;		/* MID HIGH of logical block address	*/
	u_char	lbn1;		/* MID LOW of logical block address	*/
	u_char	lbn0;		/* LSB of logical block address		*/
	u_char		:8;	/* Reserved				*/
	u_char	trans_len1;	/* MSB of transfer length in BYTES	*/
	u_char	trans_len0;	/* LSB of transfer length in BYTES	*/
	u_char	control;	/* Control byte				*/
}DIR_WRITE_LONG_CDB10;

/*
 * Write long opcode
 */
#define DIR_WRITE_LONG_OP	0x3f

/* ---------------------------------------------------------------------- */

/* 
 * WRITE SAME 
 */


/* ---------------------------------------------------------------------- */

/*
 * Parameters for direct access devices - These are the pages
 */

/* ---------------------------------------------------------------------- */


/* 
 * The diagnostic parameters  used with direct access devices.
 */
/* 
 * Translate Address page - Send Diagnostic
 */

/* 
 * Translate Address page - Receive Diagnostic
 */
/* ---------------------------------------------------------------------- */

/* 
 * Log Parameters Page are defined in scsi_all.h 
 */



/* ---------------------------------------------------------------------- */

/*
 * The mode parmater pages for direct access devices
 */

/* 
 * The mode parameter header, 6 byte cdb
 */
typedef struct dir_mode_head6 {
	u_char	mode_len;		/* The length of the mode head	*/
	u_char	medium_type;		/* The medium type		*/
	u_char			:4,	/* 4 bits reserved		*/
		dpofua		:1,	/* Does unit have cache (sense)	*/
				:2,	/* 2 bits reserved		*/
		wp		:1;	/* Write protected		*/
	u_char	blk_desc_len;		/* Len of the descriptor block	*/
}DIR_MODE_HEAD6;

/* 
 * The mode parameter header, 10 byte cdb
 */
typedef struct dir_mode_head10 {
	u_char	mode_len1;		/* The length of mode head MSB	*/
	u_char	mode_len0;		/* The length of mode head LSB	*/
	u_char	medium_type;		/* The medium type		*/
	u_char			:4,	/* 4 bits reserved		*/
		dpofua		:1,	/* Does unit have cache (sense)	*/
				:2,	/* 2 bits reserved		*/
		wp		:1;	/* Write protected		*/
	u_char		:8;		/* Byte reserved		*/	
	u_char		:8;		/* Byte reserved		*/	
	u_char	blk_desc_len1;		/* Len of the desc block MSB	*/
	u_char	blk_desc_len0;		/* Len of the desc block LSB	*/
}DIR_MODE_HEAD10;



/* 
 * The Block Descriptor for both 6 and 10 byte MODE PARAMETERS.
 */

typedef struct dir_mode_desc {
	u_char	density_code;	/* The density code Tapes		*/
	u_char	num_blocks2;	/* MSB of number of blocks 		*/
	u_char	num_blocks1;	/* Middle of number of blocks		*/
	u_char	num_blocks0;	/* LSB of number of blocks		*/
	u_char		:8;	/* reserved				*/
	u_char	block_len2;	/* MSB of block length			*/
	u_char	block_len1;	/* Middle of block length		*/
	u_char	block_len0;	/* LSB of block length			*/
}DIR_MODE_DESC;


/* 
 * The actual data structure for DIR the mode select data.
 */

/*
 * For a mode parameter list 6 byte cdb Non scatter/gather
 */

typedef struct dir_mode_data6 {
	DIR_MODE_HEAD6		sel_head;	/* The header		*/
	DIR_MODE_DESC		sel_desc;	/* The descriptor piece	*/
	u_char	page_data[ALL_MAX_PAGE_SIZE];	/* Page data area	*/
}DIR_MODE_DATA6;

/*
 * For a mode parameter list 6 byte cdb scatter/gather
 */

typedef struct dir_mode_data6_gather{
	DIR_MODE_HEAD6		*sel_head;	/* The header		*/
	DIR_MODE_DESC		*sel_desc;	/* The descriptor piece	*/
	u_char			*page_data;	/* Page data area	*/
}DIR_MODE_DATA6_GATHER;

/*
 * For a mode parameter list 10 byte cdb NON scatter/gather
 */
typedef struct dir_mode_data10 {
	DIR_MODE_HEAD10		sel_head;	/* The header		*/
	DIR_MODE_DESC		sel_desc;	/* The descriptor piece	*/
	u_char	page_data[ALL_MAX_PAGE_SIZE];	/* Page data area	*/
}DIR_MODE_DATA10;

/*
 * For a mode parameter list 10 byte cdb scatter/gather
 */
typedef struct dir_mode_data10_gather {
	DIR_MODE_HEAD10		*sel_head;	/* The header		*/
	DIR_MODE_DESC		*sel_desc;	/* The descriptor piece	*/
	u_char			*page_data;	/* Page data area	*/
}DIR_MODE_DATA10_GATHER;


/*
 * The mode page header
 */
typedef struct dir_mode_pg_hd {
	u_char	page_code	: 6;	/* define page function */
	u_char	reserved	: 1;
	u_char	ps		: 1;	/* Page savable. */
	u_char	page_length;		/* length of current page */
}DIR_MODE_PG_HD;



/* ---------------------------------------------------------------------- */

/*
 * PAGE CODE field defines for this chapter
 */
#define DIR_PG_ERR_RECOV	0x01	/* Error recovery page		*/
#define DIR_PG_FORMAT		0x03	/* Format device page		*/
#define DIR_PG_GEOM		0x04	/* Geometry page rigid disk	*/
#define DIR_PG_FLEXI		0x05	/* Flexible disk page		*/
#define DIR_PG_VERIFY_ERR	0x07	/* Verify error recovery	*/
#define DIR_PG_CACHE		0x08	/* Cache control page		*/
#define DIR_PG_MEDIUM		0x0b	/* Medium types supported	*/
#define DIR_PG_NOTCH		0x0c	/* Notch/Partition page		*/



/* ---------------------------------------------------------------------- */



/*
 * Caching Page
 */
typedef struct dir_cache_pg {
	DIR_MODE_PG_HD	pg_head;	/* Page Head			*/
	u_char		rcd	:1,	/* read cache disable		*/
			mf	:1,	/* multiplication factor	*/
			wce	:1,	/* write cache enable		*/
				:5;	/* 5 bits reserved		*/
	u_char		wrp	:4,	/* Wrt retent priority		*/
			drrp	:4;	/* Demand read ret prior	*/
	u_char		dis_pre_trans1;		/* MSB of disable prefetch
						   tranfer length	*/
	u_char		dis_pre_trans0;		/* LSB of disable prefetch
						   tranfer length	*/
	u_char		min_prefetch1;		/* MSB minimum prefetch	*/
	u_char		min_prefetch0;		/* LSB minimum prefetch	*/
	u_char		max_prefetch1;		/* MSB maximun prefetch	*/
	u_char		max_prefetch0;		/* LSB maximun prefetch	*/
	u_char		max_ceiling1;		/* MSB of maximum ceiling*/
	u_char		max_ceiling0;		/* LSB of maximum ceiling*/
}DIR_CACHE_PG;


/* ---------------------------------------------------------------------- */


/*
 * Flexible disk page
 */
typedef struct dir_flexi_pg{
	DIR_MODE_PG_HD	pg_head;/* Page head				*/
	u_char	trans_rate1;	/* Tranfer rate MSB			*/
	u_char	trans_rate0;	/* Tranfer rate LSB			*/
	u_char	num_heads;	/* Number of heads			*/
	u_char	sec_trk;	/* Sectors per track			*/
	u_char	bytes_sec1;	/* Bytes per sector MSB		 	*/
	u_char	bytes_sec0;	/* Bytes per sector LSB		 	*/
	u_char	num_cyl1;	/* MSB of number of cylinders		*/
	u_char	num_cyl0;	/* LSB of number of cylinders		*/
	u_char	wrt_precomp1;	/* start cly precomp MSB		*/
	u_char	wrt_precomp0;	/* start cly precomp LSB		*/
	u_char	wrt_current1;	/* start reduced wrt MSB		*/
	u_char	wrt_current0;	/* start reduced wrt LSB		*/
	u_char	step_rate1;	/* Drive step rate MSB			*/
	u_char	step_rate0;	/* Drive step rate LSB			*/
	u_char	step_pulse;	/* Step pulse width 			*/
	u_char	hd_settle1;	/* Head settle delay MSB		*/
	u_char	hd_settle0;	/* Head settle delay LSB		*/
	u_char	motor_on;	/* Motor on delay			*/
	u_char	motor_off;	/* Motor off delay			*/
	u_char		:5,	/* 5 bits reserved			*/
		mo	:1,	/* motor on bit				*/
		ssn	:1,	/* Start sector number			*/
		trdy	:1;	/* True ready				*/
	u_char	spc	:4,	/* step pulse cylinder			*/
			:4;	/* 4 bits reserved			*/
	u_char	wrt_comp;	/* write compensation			*/
	u_char	hd_load;	/* Head load delay			*/
	u_char	hd_unload;	/* head unload delay			*/
	u_char	pin2;		/* pin 2 of interface			*/
	u_char	pin34;		/* pin 34 of interface			*/
	u_char	pin1;		/* pin 1 of interface			*/
	u_char	pin4;		/* pin 4 of interface			*/
	u_char	rpm1;		/* MSB Rotations per minute		*/
	u_char	rpm0;		/* LSB Rotations per minute		*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
}DIR_FLEXI_PG;





/* ---------------------------------------------------------------------- */

/*
 * Format Device Page
 */
typedef struct dir_format_pg {
	DIR_MODE_PG_HD	pg_head;	/* page header			*/
	u_char	trks_zone1;		/* MSB of tracks per zone	*/
	u_char	trks_zone0;		/* LSB of tracks per zone	*/
	u_char	alt_secs_zone1;		/* MSB of alternate sector/zone	*/
	u_char	alt_secs_zone0;		/* LSB of alternate sector/zone	*/
	u_char	alt_trks_zone1;		/* MSB of alternate tracks/zone	*/ 
	u_char	alt_trks_zone0;		/* LSB of alternate tracks/zone	*/ 
	u_char	alt_trks_unit1;		/* MSB of alternate tracks/unit	*/ 
	u_char	alt_trks_unit0;		/* LSB of alternate tracks/unit	*/ 
	u_char	secs_track1;		/* MSB of sectors per track	*/
	u_char	secs_track0;		/* LSB of sectors per track	*/
	u_char	bytes_sector1;		/* MSB of bytes per sector	*/
	u_char	bytes_sector0;		/* LSB of bytes per sector	*/
	u_char	interleave1;		/* MSB of interleave factor	*/
	u_char	interleave0;		/* LSB of interleave factor	*/
	u_char	trk_skew1;		/* MSB of track skew factor	*/
	u_char	trk_skew0;		/* LSB of track skew factor	*/
	u_char	cyl_skew1;		/* MSB of cylinder skew factor	*/
	u_char	cyl_skew0;		/* LSB of cylinder skew factor	*/
	u_char			:4,	/* 4 Bits reserved		*/
		surf		:1,	/* Surface addressing bit	*/
		rmb		:1,	/* Removable media bit		*/
		hsec		:1,	/* Hard sector bit		*/
		ssec		:1;	/* Soft sector bit		*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
}DIR_FORMAT_PG;


/* ---------------------------------------------------------------------- */

/* 
 * Medium types supported
 */
typedef struct dir_medium_pg {
	DIR_MODE_PG_HD	pg_head;	/* Page header			*/
	u_char			:8;	/* Reserved			*/
	u_char			:8;	/* Reserved			*/
	u_char	medium_type1;		/* Medium type one supported	*/
	u_char	medium_type2;		/* Medium type two supported	*/
	u_char	medium_type3;		/* Medium type three supported	*/
	u_char	medium_type4;		/* Medium type four supported	*/
}DIR_MEDIUM_PG;

/* What are the media types defined for disk.... CHANGE
*/


/* ---------------------------------------------------------------------- */

/* 
 * Notch page	(ZBR drives)
 */
typedef struct dir_notch_pg {
	DIR_MODE_PG_HD	pg_head;	/* Page Header 			*/
	u_char			:6,	/* 6 bits reserved		*/
		lpn		:1,	/* Logical or physical notch	*/
		nd		:1;	/* Notched drive		*/
	u_char	max_notches1;		/* MSB Maximum number of notches*/
	u_char	max_notches0;		/* LSB Maximum number of notches*/
	u_char	act_notch1;		/* MSB of active notch		*/
	u_char	act_notch0;		/* LSB of active notch		*/
	u_char	start_bound3;		/* MSB of starting boundary	*/
	u_char	start_bound2;		/* MID HIGH of starting boundary*/
	u_char	start_bound1;		/* MID LOW of starting boundary	*/
	u_char	start_bound0;		/* LSB of starting boundary	*/
	u_char	end_bound3;		/* MSB of ending boundary	*/
	u_char	end_bound2;		/* MID HIGH of ending boundary	*/
	u_char	end_bound1;		/* MID LOW of ending boundary	*/
	u_char	end_bound0;		/* LSB of ending boundary	*/
	u_char	pg_notch7;		/* MSB of pages notched 	*/
	u_char	pg_notch6;		/* XXX of pages notched 	*/
	u_char	pg_notch5;		/* XXX of pages notched 	*/
	u_char	pg_notch4;		/* XXX of pages notched 	*/
	u_char	pg_notch3;		/* XXX of pages notched 	*/
	u_char	pg_notch2;		/* XXX of pages notched 	*/
	u_char	pg_notch1;		/* XXX of pages notched 	*/
	u_char	pg_notch0;		/* LSB of pages notched 	*/
}DIR_NOTCH_PG;



/* ---------------------------------------------------------------------- */

/* 
 * Read/write error recovery page
 */
typedef struct dir_err_recov_pg {
	DIR_MODE_PG_HD	pg_head;	/* Page header			*/
	u_char	dcr		:1,	/* Disable corrections		*/
		dte		:1,	/* Disable transfers on error	*/
		per		:1,	/* Post error recovery		*/
		eer		:1,	/* Enable error recocvery	*/
		rc		:1,	/* Read continuous		*/
		tb		:1, 	/* Transfer block on error	*/
		arre		:1,	/* Auto read reallocation	*/
		awre		:1;	/* Auto write reallocation	*/
	u_char	rd_retry_cnt;		/* Read retry count		*/
	u_char	corr_span;		/* Correction span		*/
	u_char	hd_offset;		/* Head offset count		*/
	u_char	strobe_offset;		/* Data strobe offset count	*/
	u_char			:8;	/* Reserved			*/
	u_char	wrt_retry_cnt;		/* Write retry count		*/
	u_char			:8;	/* Reserved			*/
	u_char	recov_time1;		/* MSB of Recovery time limit	*/
	u_char	recov_time0;		/* LSB of Recovery time limit	*/
}DIR_ERR_RECOV_PG;



	 
/* ---------------------------------------------------------------------- */

/* 
 * Rigid disk drive geometry page
 */
typedef struct dir_geom_pg {
	DIR_MODE_PG_HD	pg_head;	/* Page header			*/
	u_char	num_cyl2;		/* MSB of number of cylinders	*/
	u_char	num_cyl1;		/* MID of number of cylinders	*/
	u_char	num_cyl0;		/* LSB of number of cylinders	*/
	u_char	num_heads;		/* Number of heads		*/
	u_char	wrt_precomp2;		/* start cly precomp MSB	*/
	u_char	wrt_precomp1;		/* start cly precomp MID	*/
	u_char	wrt_precomp0;		/* start cly precomp LSB	*/
	u_char	wrt_current2;		/* start reduced wrt MSB	*/
	u_char	wrt_current1;		/* start reduced wrt MID	*/
	u_char	wrt_current0;		/* start reduced wrt LSB	*/
	u_char	step_rate1;		/* Drive step rate MSB		*/
	u_char	step_rate0;		/* Drive step rate LSB		*/
	u_char	land_cyl2;		/* MSB Landing zone cylinder	*/
	u_char	land_cyl1;		/* MID Landing zone cylinder	*/
	u_char	land_cyl0;		/* LSB Landing zone cylinder	*/
	u_char	rpl		:2,	/* rotational position locking	*/
				:6;	/* 6 Bits reserved		*/
	u_char	rot_offset;		/* Rotational offset		*/
	u_char			:8;	/* Reserved			*/
	u_char	rpm1;		/* MSB Rotations per minute		*/
	u_char	rpm0;		/* LSB Rotations per minute		*/
	u_char		:8;	/* Reserved				*/
	u_char		:8;	/* Reserved				*/
}DIR_GEOM_PG;

/* 
 * Defines for the rpl field. (octal)
 */
#define DIR_RPL_NONE		00
#define DIR_RPL_SLAVE		01
#define DIR_RPL_MASTER		02
#define DIR_RPL_MASTER_CNTL	03



/* ---------------------------------------------------------------------- */

/* 
 * Verify error recovery page.
 */
typedef struct dir_verify_pg {
	DIR_MODE_PG_HD	pg_head;	/* Page header			*/
	u_char	dcr		:1,	/* Disable corrections		*/
		dte		:1,	/* Disable transfer on error	*/
		per		:1,	/* Post error reporting		*/
		eer		:1,	/* Enable error recovery	*/
				:4;	/* 4 bits reserved		*/
	u_char	retry_cnt;		/* Verify retry count		*/
	u_char	corr_span;		/* Verify correction span	*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char			:8;	/* reserved			*/
	u_char	recov_time1;		/* MSB of Recovery time limit	*/
	u_char	recov_time0;		/* LSB of Recovery time limit	*/
}DIR_VERIFY_PG;



/* ---------------------------------------------------------------------- */

#endif	/* _SCSI_DIR_ */

