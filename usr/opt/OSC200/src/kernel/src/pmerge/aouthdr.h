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
 *	files obtained from mips.  I was not able to simply include
 *	them as they are sprinkled with ifdef mips.  To do so would
 *	have produced a simulator that only worked when compiled on
 *	a mips machine.
 *
 * 	3/19/90	rjl
 */
struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	long		f_timdat;	/* time & date stamp */
	long		f_symptr;	/* file pointer to symbolic header */
	long		f_nsyms;	/* sizeof(symbolic hdr) */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
};
/*
 *   Bits for f_flags:
 *
 *	F_RELFLG	relocation info stripped from file
 *	F_EXEC		file is executable  (i.e. no unresolved
 *				externel references)
 *	F_LNNO		line nunbers stripped from file
 *	F_LSYMS		local symbols stripped from file
 *	F_MINMAL	this is a minimal object file (".m") output of fextract
 *	F_UPDATE	this is a fully bound update file, output of ogen
 *	F_SWABD		this file has had its bytes swabbed (in names)
 *	F_AR16WR	this file has the byte ordering of an AR16WR (e.g. 11/70) machine
 *				(it was created there, or was produced by conv)
 *	F_AR32WR	this file has the byte ordering of an AR32WR machine(e.g. vax)
 *	F_AR32W		this file has the byte ordering of an AR32W machine (e.g. 3b,maxi,MC68000)
 *	F_PATCH		file contains "patch" list in optional header
 *	F_NODF		(minimal file only) no decision functions for
 *				replaced functions
 */

#define  F_RELFLG	0000001
#define  F_EXEC		0000002
#define  F_LNNO		0000004
#define  F_LSYMS	0000010
#define  F_MINMAL	0000020
#define  F_UPDATE	0000040
#define  F_SWABD	0000100
#define  F_AR16WR	0000200
#define  F_AR32WR	0000400
#define  F_AR32W	0001000
#define  F_PATCH	0002000
#define  F_NODF		0002000
/*
 *   Magic Numbers
 */
	/* mips */
#define  MIPSEBMAGIC	0x0160
#define  MIPSELMAGIC	0x0162
#define  SMIPSEBMAGIC	0x6001
#define  SMIPSELMAGIC	0x6201
#define  MIPSEBUMAGIC	0x0180
#define  MIPSELUMAGIC	0x0182
#define	 LIBMAGIC	0443

#define	FILHDR	struct filehdr
#define	FILHSZ	sizeof(FILHDR)

#define ISCOFF(x) \
		(((x)==MIPSEBMAGIC) || ((x)==MIPSELMAGIC) \
		|| ((x)==SMIPSEBMAGIC) || ((x)==SMIPSELMAGIC) \
		|| ((x)==MIPSEBUMAGIC) || ((x)==MIPSELUMAGIC))

#define IS_BADMAGIC(x) \
 (((x).magic)!=OMAGIC && ((x).magic)!=NMAGIC && ((x).magic)!=ZMAGIC)

struct aouthdr {
	short	magic;		/* see above				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to DW bdry*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
	long	bss_start;	/* base of bss used for this file	*/
	long	gprmask;	/* general purpose register mask	*/
	long	cprmask[4];	/* co-processor register masks		*/
	long	gp_value;	/* the gp value used for this object    */
};

#define AOUTHDR struct aouthdr
#define AOUTHSZ sizeof(AOUTHDR)

struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address, aliased s_nlib */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to gp histogram */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of gp histogram entries */
	long		s_flags;	/* flags */
};
/* SCNROUND is the size that sections are rounded off to */
#define SCNROUND ((long)16)

/* the number of shared libraries in a .lib section in an absolute output file
 * is put in the s_paddr field of the .lib section header, the following define
 * allows it to be referenced as s_nlib
 */

#define s_nlib	s_paddr
#define	SCNHDR	struct scnhdr
#define	SCNHSZ	sizeof(SCNHDR)


/*
 * Define constants for names of "special" sections
 */

#define	_TEXT	".text"
#define	_DATA	".data"
#define	_BSS	".bss"
#define	_TV	".tv"
#define _INIT ".init"
#define _FINI ".fini"
#define _LIB ".lib"
/*
 * Mips names for read only data (.rdata), small data (.sdata) and small bss
 * (.bss).  Small sections are used for global pointer relative data items.
 */
#define	_RDATA	".rdata"
#define	_SDATA	".sdata"
#define	_SBSS	".sbss"
#define _UCODE	".ucode"
#define _LIT8	".lit8"
#define _LIT4	".lit4"

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
#define STYP_RDATA	0x100		/* section contains read only data */
#define STYP_SDATA	0x200		/* section contains small data only */
#define STYP_SBSS	0x400		/* section contains small bss only */
#define STYP_UCODE	0x800		/* section only contains ucodes */
#define STYP_LIT8	0x08000000	/* literal pool for 8 byte literals */
#define STYP_LIT4	0x10000000	/* literal pool for 4 byte literals */
#define S_NRELOC_OVFL	0x20000000	/* s_nreloc overflowed, the value is in
					   v_addr of the first entry */
#define STYP_LIB	0x40000000	/* section is a .lib section */
#define STYP_INIT	0x80000000	/* section only contains the text
					   instructions for the .init sec. */
/*
 *  In a minimal file or an update file, a new function
 *  (as compared with a replaced function) is indicated by S_NEWFCN
 */

#define S_NEWFCN  0x100

/*
 * In 3b Update Files (output of ogen), sections which appear in SHARED
 * segments of the Pfile will have the S_SHRSEG flag set by ogen, to inform
 * dufr that updating 1 copy of the proc. will update all process invocations.
 */

#define S_SHRSEG	0x20
/*
 * This is the definition of a mips .lib section entry.  Note the size and
 * offset are in sizeof(long)'s not bytes.
 */
struct libscn {
	long	size;		/* size of this entry (including target name) */
	long	offset;		/* offset from start of entry to target name  */
	long	tsize;		/* text size in bytes, padded to DW boundary  */
	long	dsize;		/* initialized data "  	  "    "  "   "       */
	long	bsize;		/* uninitialized data "   "    "  "   "       */
	long	text_start;	/* base of text used for this library	      */
	long	data_start;	/* base of data used for this library	      */
	long	bss_start;	/* base of bss used for this library	      */
	/* pathname of target shared library */
};


#define	LIBSCN	struct libscn
#define	LSCNSZ	sizeof(LIBSCN)

#define C_TXTOFF(f,a) \
 ((a).magic == ZMAGIC || (a).magic == LIBMAGIC ? 0 : \
  ((a).vstamp < 23 ? \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + 7) & 0xfffffff8) : \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + SCNROUND-1) & ~(SCNROUND-1)) ) )
