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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/alpha/reloc.h,v 1.2.2.7 1992/09/11 13:37:27 Mike_Rickabaugh Exp $ */
#ifndef _RELOC_H
#define _RELOC_H

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#if defined(__mips) || defined(__mips64) || defined(__alpha)

#ifdef __LANGUAGE_C__
struct reloc {
    long	r_vaddr;	/* (virtual) address of reference */
#if defined(__mips64) || defined(__alpha)
    unsigned	r_symndx;	/* index into symbol table */
    unsigned	r_type 	: 8;	/* relocation type */
    unsigned	r_extern: 1;	/* if 1 symndx is an index into */
				/* the external symbol table, else */
				/* symndx is a section # */
    unsigned	r_offset:6;	/* for R_OP_STORE, quad based LE bit offset */
    unsigned	r_reserved:11;	/* Must be zero */
    unsigned	r_size:6;	/* R_OP_STORE, bit size */
#else
    unsigned	r_symndx:24,	/* index into symbol table */
		r_reserved:2,
		r_type:5,	/* relocation type */
		r_extern:1;	/* if 1 symndx is an index into the external
				   symbol table, else symndx is a section # */
#endif
    };
#endif /* __LANGUAGE_C__ */

#ifdef __LANGUAGE_PASCAL__
type
  reloc = packed record
#if defined(__mips64) || defined(__alpha)
      r_vaddr : integer64;		/* (virtual) address of reference    */
      r_symndx : integer;		/* index into symbol table	     */
      r_type : 0..255;			/* relocation type		     */
      r_extern : 0..1;			/* if 1, symndx is an index into the */
					/* external symbol table, else	     */
					/* symndx is a section		   # */
      r_offset : 0..63;			/* for R_OP_STORE, quad based LE bit offset */
      r_reserved : 0..lshift(1, 11)-1;
      r_size : 0..63;			/* R_OP_STORE, bit size */
#else
      r_vaddr : long;			/* (virtual) address of reference    */
      r_symndx : 0..lshift(1, 24)-1;	/* index into symbol table	     */
      r_reserved : 0..3;
      r_type : 0..31;			/* relocation type		     */
      r_extern : 0..1;			/* if 1, symndx is an index into the */
					/* external symbol table, else	     */
					/* symndx is a section		   # */
#endif
      end {record};
#endif /* __LANGUAGE_PASCAL__ */

/*
 * Section numbers for symndex for local relocation entries (r_extern == 0).
 * For these entries the starting address for the section referenced by the
 * section number is used in place of an external symbol table entry's value.
 */
#define	R_SN_NULL	0
#define	R_SN_TEXT	1
#define	R_SN_RDATA	2
#define	R_SN_DATA	3
#define	R_SN_SDATA	4
#define	R_SN_SBSS	5
#define	R_SN_BSS	6
#define	R_SN_INIT	7
#define	R_SN_LIT8	8
#define	R_SN_LIT4	9
#define	R_SN_XDATA	10
#define	R_SN_PDATA	11
#define R_SN_FINI       12

#if defined(__mips64) || defined(__alpha)
#define	R_SN_LITA	13
#define R_SN_ABS	14		/* constant relocation r_vaddr's */
#define MAX_R_SN	14
#else
#define MAX_R_SN        12
#endif

#else /* !defined(__mips) */

struct reloc {
	long	r_vaddr;	/* (virtual) address of reference */
	long	r_symndx;	/* index into symbol table */
	unsigned short	r_type;	/* relocation type */
	};
#endif /* __mips */

/*
 *   relocation types for all products and generics
 */

/*
 * All generics
 *	reloc. already performed to symbol in the same section
 */
#define  R_ABS		0

#if __mips && !defined(__mips64)
/*
 * Mips machines
 *
 *	16-bit reference
 *	32-bit reference
 *	26-bit jump reference
 *	reference to high 16-bits
 *	reference to low 16-bits
 *	reference to global pointer reletive data item
 *	reference to global pointer reletive literal pool item
 */
#define	R_REFHALF	1
#define	R_REFWORD	2
#define	R_JMPADDR	3
#define	R_REFHI		4
#define	R_REFLO		5
#define	R_GPREL		6
#define	R_LITERAL	7
#define R_REL32         8
#define R_REFHI_64      9
#define R_REFLO_64      10
#define R_REFWORD_64    11
#define R_PC16	        12
#if 0 /* __osf__ */
#define	R_RELHI		13
#define	R_RELLO		14
#endif /* __osf__ */
#define R_REFSHFT       15
#define	R_REFHI_ADDEND	16	/* lo value is in immed of inst */
#define MAX_R_TYPE      31
#endif

#if defined(__mips64) || defined(__alpha)
/*
 * ALPHA machines
 *
 *	32-bit reference
 *	64-bit reference
 *	32-bit displacement from gp
 *	reference to global pointer relative literal pool item
 *	identifies useage of a literal address previously loaded
 *	lda/ldah instruction pair to initialize gp.
 *	21-bit branch reference
 *	14-bit jsr hint reference
 */

#define R_REFLONG	1
#define R_REFQUAD	2
#define R_GPREL32	3
#define R_LITERAL	4
#define R_LITUSE	5
#define R_GPDISP	6
#define R_BRADDR	7
#define R_HINT		8
/*
 *	self relative relocations mean that the memory location at
 *	r_vaddr contains an offset to the destination. If the relocation
 *	is r_extern==1, then the value at the memory location is ignored
 *	(maybe we should allow offsets?). If r_extern==0, then the value
 *	at the memory location is the actual offset. 
 *
 *	The linker uses the relocated target and a relocated r_vaddr to
 *	determine the offset. Offsets are considered signed.
 */
#define R_SREL16	9		/* self relative 16 bit offset */
#define R_SREL32	10		/* self relative 32 bit offset */
#define R_SREL64	11		/* self relative 64 bit offset */
/* 
 *	stack relocations provide a primitive expression evaluator for
 *	relocatable and constant values at link time. It also provides
 *	a way to store a value into a bit field (the R_OP_STORE has a
 *	bit size and offset field (from a quadword aligned qaudword)).
 *
 *	The operations specify what they relocate and what happens to
 *	the linktime stack. It is an error to cause a stack underflow
 *	or have values left on the stack when the relocation for a section
 *	is complete.
 *
 *	terms:
 *		tos		top of stack
 *		stack		qaudword array representing the stack
 *		vaddr		address field in reloc record or 
 *					extern symbol address
 *		relocate(X)	relocate address X
 *		X(o:s)		address X, bitoffset o, bit size s
 *		r_offset	offset field in reloc record
 *		r_size		bitsize field in reloc record
 *
 *	Note: use R_SN_ABS as the section for constants (like in shifts).
 *		
 */
#define R_OP_PUSH	12	/* stack[++tos] = relocate(vaddr) */
#define R_OP_STORE	13	/* vaddr(r_offset:r_size) = stack[tos--] */
#define R_OP_PSUB	14	/* stack[tos] = stack[tos] - relocate(vaddr) */
#define R_OP_PRSHIFT	15	/* stack[tos] = stack[tos] >> relocate(vaddr) */

/*
 *	The GPVALUE relocation provides a mechanism to designate a range
 *	of section addresses to have a particular gp value.  This is not
 *	represented in the optional header when there are multiple gp
 *	ranges within an object file.
 *
 *	To compute the value of gp, add the r_symndx field to the value
 *	of gp in the optional header.  This gives the gp value used for
 *	the range from vaddr to the address pointed by the next GPVALUE
 *	field or object's last relocation record.
 *
 *	vaddr		points to the address where the gp range starts.
 *	symndx		the offset added to optional header's value.
 *	type		R_GPVALUE
 *	extern		0
 */

#define R_GPVALUE	16

#define MAX_R_TYPE	17

/*
 * Literal usage types for R_LITUSE
 *
 *	literal address in register of a memory format instruction
 *	literal address in byte offset register of byte-manipulation instruction
 *	literal address is in target register of a jsr instruction.
 */

#define R_LU_BASE	1
#define R_LU_BYTOFF	2
#define R_LU_JSR	3




#endif

/*
 * X86 generic
 *	8-bit offset reference in 8-bits
 *	8-bit offset reference in 16-bits 
 *	12-bit segment reference
 *	auxiliary relocation entry
 */
#define	R_OFF8		07
#define R_OFF16		010
#define	R_SEG12		011
#define	R_AUX		013

/*
 * B16 and X86 generics
 *	16-bit direct reference
 *	16-bit "relative" reference
 *	16-bit "indirect" (TV) reference
 */
#define  R_DIR16	01
#define  R_REL16	02
#define  R_IND16	03

/*
 * 3B generic
 *	24-bit direct reference
 *	24-bit "relative" reference
 *	16-bit optimized "indirect" TV reference
 *	24-bit "indirect" TV reference
 *	32-bit "indirect" TV reference
 */
#define  R_DIR24	04
#define  R_REL24	05
#define  R_OPT16	014
#define  R_IND24	015
#define  R_IND32	016

/*
 * 3B and M32 || u3b15 || u3b5 || u3b2 generics
 *	32-bit direct reference
 */
#define  R_DIR32	06

/*
 * M32 || u3b15 || u3b5 || u3b2 generic
 *	32-bit direct reference with bytes swapped
 */
#define  R_DIR32S	012

/*
 * DEC Processors  VAX 11/780 and VAX 11/750
 *
 */

#define R_RELBYTE	017
#define R_RELWORD	020
#define R_RELLONG	021
#define R_PCRBYTE	022
#define R_PCRWORD	023
#define R_PCRLONG	024

/*
 * Motorola 68000
 *
 * ... uses R_RELBYTE, R_RELWORD, R_RELLONG, R_PCRBYTE and R_PCRWORD as for
 * DEC machines above.
 */

#define	RELOC	struct reloc
#define	RELSZ	sizeof(RELOC)

	/* Definition of a "TV" relocation type */

#if _N3B
#define ISTVRELOC(x)	((x==R_OPT16)||(x==R_IND24)||(x==R_IND32))
#endif
#if _B16 || _X86
#define ISTVRELOC(x)	(x==R_IND16)
#endif
#if _M32 || __u3b15 || __u3b5 || __u3b2
#define ISTVRELOC(x)	(x!=x)	/* never the case */
#endif

#endif
