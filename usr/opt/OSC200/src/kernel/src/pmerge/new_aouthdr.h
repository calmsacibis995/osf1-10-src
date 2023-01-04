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
/* --------------------------------------------------- */
/* | Copyright (c) 1986 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                            | */
/* --------------------------------------------------- */


/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * 	This file is a collection of various xcoff related header
 *	for alpha.
 *
 * 	29-Oct-1990, Ken Lesniak
 */

struct new_filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	int		f_timdat;	/* time & date stamp */
	long		f_symptr;	/* file pointer to symbolic header */
	int		f_nsyms;	/* sizeof(symbolic hdr) */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
};
#define ALPHAMAGIC 0603

#define	NEW_FILHDR	struct new_filehdr
#define	NEW_FILHSZ	sizeof(NEW_FILHDR)

struct new_aouthdr {
	short	magic;		/* see above				*/
	short	vstamp;		/* version stamp			*/
	int	pad0;
	long	tsize;		/* text size in bytes, padded to DW bdry*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
	long	bss_start;	/* base of bss used for this file	*/
	int	gprmask;	/* general purpose register mask	*/
	int	fprmask;
	long	gp_value;	/* the gp value used for this object    */
};

#define NEW_AOUTHDR struct new_aouthdr
#define NEW_AOUTHSZ sizeof(NEW_AOUTHDR)

struct new_scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address, aliased s_nlib */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to gp histogram */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of gp histogram entries */
	int		s_flags;	/* flags */
};

#define NEW_SCNROUND	16
#define	NEW_SCNHDR	struct new_scnhdr
#define	NEW_SCNHSZ	sizeof(NEW_SCNHDR)

#define NEW_C_TXTOFF(f,a) \
 ((a).magic == ZMAGIC || (a).magic == LIBMAGIC ? 0 : \
  ((a).vstamp < 23 ? \
   ((NEW_FILHSZ + NEW_AOUTHSZ + (f).f_nscns * NEW_SCNHSZ + 7) & 0xfffffff8) : \
   ((NEW_FILHSZ + NEW_AOUTHSZ + (f).f_nscns * NEW_SCNHSZ + NEW_SCNROUND-1) & ~(NEW_SCNROUND-1)) ) )

#define	 LIBMAGIC	0443
#define OMAGIC 0407
#define NMAGIC 0410
#define ZMAGIC 0413
