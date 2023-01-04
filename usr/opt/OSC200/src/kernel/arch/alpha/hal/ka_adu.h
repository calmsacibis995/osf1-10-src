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
 *	"@(#)ka_adu.h	9.1	(ULTRIX/OSF)	10/21/91"
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: machine/alpha/ka_adu.h
 *
 * 08-Aug-91 -- afd
 *	Added name field to tv_slot struct for tvbus autoconfig code.
 *
 * 12-Oct-90 -- Tim Burke
 *	Created this file for Alpha support.
 */

#ifndef _KA_ADU_H_
#define _KA_ADU_H_

/*
 * Format of the machine check logout area of the adu.
 *
 * tim fix - the defines related to the logout area should really be in a
 * separate file so that errlog.c can include a generic alpha logout spec??
 */
/* tim fix - this structure is not clearly labeled in the adu spec */
struct mchk_logout {
	long	retry;
	long	pt[32];
	long	exc_addr;
	long	exc_sum;
	long	msk;
	long	pal_base;
	long	hirr;
	long	hier;
	long	mm_csr;
	long	va;
	long	bui_addr;
	long	bui_stat;
	long	dc_addr;
	long	fill_addr;
	long	dc_stat;
	long	fill_syndrome;
	long	bc_tag;
};

/*
 * Format of the logout area for a System Corrected Error 
 */
/* tim fix - this structure is not yet defined in the adu spec */
struct sce_logout {
	long	retry;
	long 	xx;
	long	yy;
};

/*
 * Format of the logout area for a Processor Corrected Error 
 */
/* tim fix - this structure is not yet defined in the adu spec */
struct pce_logout {
	long	retry;
	long 	xx;
	long	yy;
};

/*
 * Defines that refer to specific areas of the logout area.
 * For all of the logout formats the retry bit is bit number 63.
 * The low 4 bytes of the first quadword specify the number of bytes in the
 * loguot area.
 */
#define RETRY_BIT	0x1000000000000000
#define BCOUNT_MASK	0xFFFF

/*
 * tvbus definitions
 *
 * If there were going to be future tvbus implimentations this
 * should probably be in a seperate file. For now this is adu
 * specific.
 */
#define TVIOMAPSIZE 8		/* a single i/o module register map 8kpages */
#define TVIOREGBASE 0x300000000 /* TV I/O module base registers		    */

/*
 * tv bus configuration module types
 */
#define TV_TYPE_EMPTY	0	/* empty slot	*/
#define TV_TYPE_IO	1	/* i/o modeule	*/
#define TV_TYPE_MEM64	2	/* memory	*/
#define TV_TYPE_CPU3	3	/* ev3 cpu	*/
#define TV_TYPE_CPU4	4	/* ev4 cpu	*/

/*
 * tv bus configuration module flags
 */
#define TV_BROKEN	1	/* broken module		*/
#define TV_PRIM_CPU	2	/* primary cpu			*/
#define TV_CONSOLE	4	/* I/O module with console 	*/
#define TVSTATUSBITS "\20\3console\2primary\1broken"

/*
 * tv bus configuration data structures
 */
struct tv_io_config {
	char tv_ether_addr[8];	/* actually only the first 6 bytes	*/
	int tv_scsiid;		/* scsi id				*/
	int tv_fill0;		/* rest is unused			*/
};

struct tv_mem_config {
	unsigned long tv_interl_base_addr;	/* base addr of interleave */
	long tv_interl_position;		/* interleave position	   */
};

/*
 * Configuration is a 512 byte sized structure
 */
struct tv_config {
	unsigned int tv_type;	/* module type				*/
	unsigned int tv_flag;	/* module flags				*/
	union {
		struct tv_io_config tv_io_config;
		struct tv_mem_config tv_mem_config;
		char tv_fill[504];
	} tv_config_spec;
};

#define TV_SLOTS 16

struct tv_slot {
	int module_type;	/* the module id */
	char *virtaddr;		/* the base virtual addr of the I/O module */
	char name[15];		/* the device name (aducn, adusz, aduln) */
};

extern struct tv_slot tv_slot[];	/* table with IO device info */

#endif
