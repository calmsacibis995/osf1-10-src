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
 *-------------------------------------------------------------
 *|         RESTRICTED RIGHTS LEGEND                          |
 *| Use, duplication, or disclosure by the Government is      |
 *| subject to restrictions as set forth in subparagraph      |
 *| (c)(1)(ii) of the Rights in Technical Data and Computer   |
 *| Software Clause at DFARS 252.227-7013.                    |
 *|         MIPS Computer Systems, Inc.                       |
 *|         928 Arques Avenue                                 |
 *|         Sunnyvale, CA 94086                               |
 *-------------------------------------------------------------
 */
/* --------------------------------------------------------- */
/* | Copyright (c) 1986, 1989 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                                  | */
/* --------------------------------------------------------- */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./kernel/sysV/scnhdr.h,v 4.3.2.5 1992/12/09 15:02:04 Jay_Estabrook Exp $ */
#ifndef __SCNHDR_H
#define __SCNHDR_H

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#if mips || __alpha
/*
 * The entries that refer to line numbers are not used for line numbers on
 * "mips" machines.  See symhdr.h for the entries to get to the line number
 * table.  The entries that were for line numbers are used for gp tables on
 * "mips" machines.  That is s_lnnoptr is the file ptr to the gp table and
 * s_nlnno is the number of table entries.  See the end of this file for the
 * structure.
 */
#endif

#ifdef __LANGUAGE_C__
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
/* START ALPHA */
	int		s_flags;	/* flags */
/* END ALPHA */
	};

#endif /* __LANGUAGE_C__ */
#ifdef LANGUAGE_PASCAL
type
  scnhdr = packed record
      s_name : packed array[1..8] of char; /* section name		     */
/* START ALPHA */
      s_paddr : quad;			/* physical address		     */
      s_vaddr : quad;			/* virtual address		     */
      s_size : quad;			/* section size 		     */
      s_scnptr : quad;			/* file ptr to raw data for section  */
      s_relptr : quad;			/* file ptr to relocation	     */
      s_lnnoptr : quad; 		/* file ptr to gp histogram	     */
/* END ALPHA */
      s_nreloc : ushort;		/* number of relocation entries      */
      s_nlnno : ushort; 		/* number of gp histogram entries    */
      s_flags : long;			/* flags			     */
    end {record};
#endif /* LANGUAGE_PASCAL */

#if defined(mips) || defined(__alpha)
/* SCNROUND is the size that sections are rounded off to */
#ifdef __LANGUAGE_C__
/* START ALPHA */
#define SCNROUND 16
/* END ALPHA */
#endif /* __LANGUAGE_C__ */
#ifdef LANGUAGE_PASCAL
#define SCNROUND (16)
#endif /* LANGUAGE_PASCAL */
#endif /* mips or alpha */

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

#ifdef __LANGUAGE_C__
#define	_TEXT	".text"
#define	_DATA	".data"
#define	_BSS	".bss"
#define	_TV	".tv"
#define _INIT   ".init"
#define _FINI   ".fini"
#define _LIB    ".lib"

#if mips || __alpha
/* of dso related sections */
#define _GOT      ".got"
#define _DYNAMIC  ".dynamic"
#define _DYNSYM   ".dynsym"
#define _REL_DYN  ".rel.dyn"
#define _DYNSTR   ".dynstr"
#define _HASH     ".hash"

/* Mips specific dso sections */
#define _DSOLIST  ".dsolist"
#define _MSYM     ".msym"
#define _CONFLICT ".conflict"
#define _REGINFO  ".reginfo"
#define _PACKAGE  ".package"
#define	_PACKSYM  ".packsym"
#endif

#endif /* __LANGUAGE_C__ */
#ifdef LANGUAGE_PASCAL
#ifdef PASTEL
#define	_TEXT	('.text'||chr(0))
#define	_DATA	('.data'||chr(0))
#define	_BSS	('.bss'||chr(0))
#define	_TV	('.tv'||chr(0))
#define	_INIT	('.init'||chr(0))
#define	_FINI	('.fini'||chr(0))
#define	_LIB	('.lib'||chr(0))

#if mips || __alpha
/* of dso related sections */
#define _GOT      ('.got||'chr(0))
#define _DYNAMIC  ('.dynamic'||chr(0))
#define _DYNSYM   ('.dynsym'||chr(0))
#define _REL_DYN  ('.rel.dyn'||chr(0))
#define _DYNSTR   ('.dynstr'||chr(0))
#define _HASH     ('.hash'||chr(0))

/* Mips specific dso sections */
#define _DSOLIST  ('.dsolist'||chr(0))
#define _MSYM     ('.msym'||chr(0))
#define _CONFLICT ('.conflict'||chr(0))
#define _REGINFO  ('.reginfo'||chr(0))
#define _PACKAGE  ('.package'||chr(0))
#define _PACKSYM  ('.packsym'||chr(0))
#endif

#else 
#define	_TEXT	".text\0"
#define	_DATA	".data\0"
#define	_BSS	".bss\0"
#define	_TV	".tv\0"
#define	_INIT	".init\0"
#define	_FINI	".fini\0"
#define	_LIB	".lib\0"

#if mips || __alpha
/* of dso related sections */
#define _GOT      ".got\0"
#define _DYNAMIC  ".dynamic\0"
#define _DYNSYM   ".dynsym\0"
#define _REL_DYN  ".rel.dyn\0"
#define _DYNSTR   ".dynstr\0"
#define _HASH     ".hash\0"

/* Mips specific dso sections */
#define _DSOLIST  ".dsolist\0"
#define _MSYM     ".msym\0"
#define _CONFLICT ".conflict\0"
#define _REGINFO  ".reginfo\0"
#define _PACKAGE  ".package\0"
#define _PACKSYM  ".packsym\0"
#endif

#endif /* PASTEL */
#endif /* LANGUAGE_PASCAL */

#if mips || __alpha
/*
 * Mips names for read only data (.rdata), small data (.sdata) and small bss
 * (.bss).  Small sections are used for global pointer relative data items.
 */
#ifdef __LANGUAGE_C__
#define	_RDATA	".rdata"
#define	_SDATA	".sdata"
#define	_SBSS	".sbss"
#define _UCODE	".ucode"
#define _LIT8	".lit8"
#define _LIT4	".lit4"
#endif /* __LANGUAGE_C__ */
#ifdef LANGUAGE_PASCAL
#ifdef PASTEL
#define	_RDATA	('.rdata'||chr(0))
#define	_SDATA	('.sdata'||chr(0))
#define	_SBSS	('.sbss'||chr(0))
#define	_UCODE	('.ucode'||chr(0))
#define	_LIT8	('.lit8'||chr(0))
#define	_LIT4	('.lit4'||chr(0))
#else 
#define	_RDATA	".rdata\0"
#define	_SDATA	".sdata\0"
#define	_SBSS	".sbss\0"
#define	_UCODE	".ucode\0"
#define	_LIT8	".lit8\0"
#define	_LIT4	".lit4\0"
#endif /* PASTEL */
#endif /* LANGUAGE_PASCAL */
#endif


/*
 * The low 4 bits of s_flags is used as a section "type"
 */

#ifdef __LANGUAGE_C__
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
#if mips || __alpha
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
#define STYP_FINI	0x01000000      /* insts for .fini */
#define STYP_COMMENT	0x02000000	/* */


/* of dso related sections types */
#define STYP_GOT        0x00001000
#define STYP_DYNAMIC    0x00002000
#define STYP_DYNSYM     0x00004000
#define STYP_REL_DYN    0x00008000
#define STYP_DYNSTR     0x00010000
#define STYP_HASH       0x00020000

/* Mips specific dso sections */
#define STYP_DSOLIST    0x00040000
#define STYP_MSYM       0x00080000
#define STYP_CONFLICT   0x00100000
#define STYP_REGINFO    0x00200000
#define STYP_PACKAGE    0x00400000
#define STYP_PACKSYM	0x00800000

#else
#define STYP_INFO	0x200		/* comment section : not allocated
						not relocated, not loaded */
#define STYP_LIB	0x800		/* for .lib section : same as INFO */
#define STYP_OVER	0x400		/* overlay section : relocated
						not allocated or loaded */
#endif /* mips */
#endif /* __LANGUAGE_C__ */
#ifdef LANGUAGE_PASCAL
#define STYP_REG	16#00		/* "regular" section:
						allocated, relocated, loaded */
#define STYP_DSECT	16#01		/* "dummy" section:
						not allocated, relocated,
						not loaded */
#define STYP_NOLOAD	16#02		/* "noload" section:
						allocated, relocated,
						 not loaded */
#define STYP_GROUP	16#04		/* "grouped" section:
						formed of input sections */
#define STYP_PAD	16#08		/* "padding" section:
						not allocated, not relocated,
						 loaded */
#define STYP_COPY	16#10		/* "copy" section:
						for decision function used
						by field update;  not
						allocated, not relocated,
						loaded;  reloc & lineno
						entries processed normally */
#define	STYP_TEXT	16#20		/* section contains text only */
#define STYP_DATA	16#40		/* section contains data only */
#define STYP_BSS	16#80		/* section contains bss only */
#if mips || __alpha
#define STYP_RDATA	16#100		/* section contains read only data */
#define STYP_SDATA	16#200		/* section contains small data only */
#define STYP_SBSS	16#400		/* section contains small bss only */
#define STYP_UCODE	16#800		/* section only contains ucodes */
#define STYP_LIT8	16#08000000	/* literal pool for 8 byte literals */
#define STYP_LIT4	16#10000000	/* literal pool for 4 byte literals */
#define S_NRELOC_OVFL	16#20000000	/* s_nreloc overflowed, the value is in
					   v_addr of the first entry */
#define STYP_LIB	16#40000000	/* section is a .lib section */
#define STYP_INIT	16#80000000	/* section only contains the text
					   instructions for the .init sec. */
#define STYP_FINI	16#01000000      /* insts for .fini */
#define STYP_COMMENT	16#02000000	/* */

/* of dso related sections types */
#define STYP_GOT        16#00001000
#define STYP_DYNAMIC    16#00002000
#define STYP_DYNSYM     16#00004000
#define STYP_REL_DYN    16#00008000
#define STYP_DYNSTR     16#00010000
#define STYP_HASH       16#00020000

/* Mips specific dso sections */
#define STYP_DSOLIST    16#00080000
#define STYP_CONFLICT   16#00100000
#define STYP_REGINFO    16#00200000
#define STYP_PACKAGE    16#00400000
#define STYP_PACKSYM    16#00800000

#else
#define STYP_INFO	16#200		/* comment section : not allocated
						not relocated, not loaded */
#define STYP_LIB	16#800		/* for .lib section : same as INFO */
#define STYP_OVER	16#400		/* overlay section : relocated
						not allocated or loaded */
#endif /* mips */
#endif /* LANGUAGE_PASCAL */

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

#if mips || __alpha
/*
 * This table gives the section size corresponding to each applicable
 * Gnum (always including 0), sorted by smallest size first. It is pointed to
 * by the s_lnnoptr field in the section header and its number of entries
 * (including the header) is in the s_nlnno field in the section header.
 * This table only needs to exist for the .sdata and .sbss sections
 * sections.  If there is no "small" section then the gp table for it is
 * attached to the coresponding "large" section so the information still
 * gets to the loader.
 */
/* START ALPHA */
#ifdef __LANGUAGE_C__
union gp_table {
  struct {
    int current_g_value; /* actual value */
    int unused;
  } header;
  struct {
    int g_value; /* hypothetical value */
    int bytes;	/* section size corresponding to hypothetical value */
  } entry;
}; 
#define GPTAB	union gp_table
#define GPTABSZ	sizeof(GPTAB)

#endif /* __LANGUAGE_C__ */

#ifdef LANGUAGE_PASCAL
type
  gp_table = record
    case boolean of
      false: (current_g_value: integer; unused: integer);
      true: (g_value: integer; bytes: integer);
    end;
  gpt_ptr = ^gp_table;
#endif /* LANGUAGE_PASCAL */
/* END ALPHA */

#endif /* mips */

#if defined(mips) || defined(__alpha)
/* START ALPHA */
/*
 * This is the definition of a mips .lib section entry.  Note the size and
 * offset are in sizeof(int)'s not bytes.
 */
#ifdef __LANGUAGE_C__
struct libscn {
	int	size;		/* size of this entry (including target name) */
	int	offset;		/* offset from start of entry to target name  */
	long	tsize;		/* text size in bytes, padded to DW boundary  */
	long	dsize;		/* initialized data "  	  "    "  "   "       */
	long	bsize;		/* uninitialized data "   "    "  "   "       */
	long	text_start;	/* base of text used for this library	      */
	long	data_start;	/* base of data used for this library	      */
	long	bss_start;	/* base of bss used for this library	      */
/* END ALPHA */
	/* pathname of target shared library */
};

#endif /* __LANGUAGE_C__ */

#define	LIBSCN	struct libscn
#define	LSCNSZ	sizeof(LIBSCN)

#endif /* mips */
#endif
