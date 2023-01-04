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
 *	@(#)$RCSfile: scnhdr.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:05:54 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*	@(#)scnhdr.h	3.1	*/
/*		*/
/*	2/26/91	*/
/*	5.2 SID scnhdr.h	2.2 12/15/82	*/



#ifndef _SCNHDR_H_
#define _SCNHDR_H_

struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	long		s_flags;	/* flags */
#ifdef ns32000
	long		s_symptr;	/* file pointer to symbol entries */
	unsigned short	s_modno;	/* Module number of module associated */
					/*   with this section */
	short		s_pad;		/* padding to 4 byte multiple */
#endif
	};

#define	SCNHDR	struct scnhdr
#define	SCNHSZ	sizeof(SCNHDR)




/*
 * Define constants for names of "special" sections
 */

#define _TEXT ".text"
#define _DATA ".data"
#define _BSS  ".bss"
#define _TV  ".tv"




/*
 * The low 4 bits of s_flags is used as a section "type"
 */

#define STYP_REG	0x00		/* "regular" section:
						allocated, relocated, loaded */
#define STYP_DSECT	0x01		/* "dummy" section:
						not allocated, relocated,
						not loaded */
#define STYP_NOLOAD	0x02		/* "noload" section:
						allocated, relocated,
						 not loaded */
#define STYP_GROUP	0x04		/* "grouped" section:
						formed of input sections */
#define STYP_PAD	0x08		/* "padding" section:
						not allocated, not relocated,
						 loaded */
#define STYP_COPY	0x10		/* "copy" section:
						for decision function used
						by field update;  not
						allocated, not relocated,
						loaded;  reloc & lineno
						entries processed normally */
#define	STYP_TEXT	0x20		/* section contains text only */
#define STYP_DATA	0x40		/* section contains data only */
#define STYP_BSS	0x80		/* section contains bss only */

/*
 *  In a minimal file or an update file, a new function
 *  (as compared with a replaced function) is indicated by S_NEWFCN
 */

#define S_NEWFCN  0x10

/*
 * In 3b Update Files (output of ogen), sections which appear in SHARED
 * segments of the Pfile will have the S_SHRSEG flag set by ogen, to inform
 * dufr that updating 1 copy of the proc. will update all process invocations.
 */

#define S_SHRSEG	0x20

#endif /* _SCNHDR_H_ */
