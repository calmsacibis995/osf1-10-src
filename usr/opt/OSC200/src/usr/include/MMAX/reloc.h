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
 *	@(#)$RCSfile: reloc.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:05:52 $
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
/*	@(#)reloc.h	3.1	*/
/*		*/
/*	2/26/91	*/
/*	5.2 SID reloc.h	2.1	*/
struct reloc {
	long	r_vaddr;	/* (virtual) address of reference */
	long	r_symndx;	/* index into symbol table */
	unsigned short	r_type;		/* relocation type */
	};


/*
 *   relocation types for all products and generics
 */

/*
 * All generics
 *	reloc. already performed to symbol in the same section
 */

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
 * XL generics
 *	10-bit direct reference
 *	10-bit "relative" reference
 *	32-bit "relative" reference
 */
#define	R_DIR10		025
#define R_REL10		026
#define R_REL32		027

/*
 * 3B and M32 generics
 *	32-bit direct reference
 */
#define  R_DIR32	06

/*
 * M32 generic
 *	32-bit direct reference with bytes swapped
 */
#define  R_DIR32S	012

/*
 * DEC Processors  VAX 11/780 and VAX 11/750
 *
 */

#if (vax || m68) && !n16
#define R_RELBYTE	017
#define R_RELWORD	020
#define R_RELLONG	021
#define R_PCRBYTE	022
#define R_PCRWORD	023
#define R_PCRLONG	024
#endif

#if n16
/*
 * National Semiconductor Processor NS16000
 *
 */

#define R_ADDRTYPE	0x000f	/* type of relocation item */
#define R_NOTHING	0x0000	/* ignore */
#define R_ADDRESS	0x0001	/* take address if symbol */
#define R_LINKENTRY	0x0002	/* take symbols's link table entry index */

#define R_RELTO		0x00f0	/* action to take when relocating */
#define R_ABS		0x0000	/* keep symbol's address as such */
#define R_PCREL		0x0010	/* subtract the pc address of hole */
#define R_SBREL		0x0020	/* subtract the static base start of this
					section's module */

#define R_FORMAT	0x0f00	/* relocation item data format */
#define R_NUMBER	0x0000	/* retain as two's complement value */
#define R_DISPL		0x0100	/* convert to NS32000 displacement */
#define R_PROCDES	0x0200	/* convert to NS32000 procedure descriptor */
#define R_IMMED		0x0300	/* convert to NS32000 immediate operand */

#define R_SIZESP	0xf000	/* relocation item size */
#define R_S_ONE		0x0000	/* relocate 1 byte,  RS_S_08 */
#define R_S_TWO		0x1000	/* relocate 2 bytes, RS_S_16 */
#define R_S_FOUR	0x2000	/* relocate 4 bytes, RS_S_32 */

#define R_RELBYTE	R_ADDRESS | R_ABS | R_NUMBER | R_S_ONE
#define R_RELWORD	R_ADDRESS | R_ABS | R_NUMBER | R_S_TWO
#define R_RELLONG	R_ADDRESS | R_ABS | R_NUMBER | R_S_FOUR
#define R_PCRBYTE	R_ADDRESS | R_PCREL | R_NUMBER | R_S_ONE
#define R_PCRWORD	R_ADDRESS | R_PCREL | R_NUMBER | R_S_TWO
#define R_PCRLONG	R_ADDRESS | R_PCREL | R_NUMBER | R_S_FOUR
#define R_RRELBYTE	R_ADDRESS | R_ABS | R_DISPL | R_S_ONE
#define R_RRELWORD	R_ADDRESS | R_ABS | R_DISPL | R_S_TWO
#define R_RRELLONG	R_ADDRESS | R_ABS | R_DISPL | R_S_FOUR
#define R_RPCRBYTE	R_ADDRESS | R_PCREL | R_DISPL | R_S_ONE
#define R_RPCRWORD	R_ADDRESS | R_PCREL | R_DISPL | R_S_TWO
#define R_RPCRLONG	R_ADDRESS | R_PCREL | R_DISPL | R_S_FOUR
#endif

#define	RELOC	struct reloc
#define	RELSZ	10	/* sizeof(RELOC) */

	/* Definition of a "TV" relocation type */

#if N3B || U3B
#define ISTVRELOC(x)	((x==R_OPT16)||(x==R_IND24)||(x==R_IND32))
#endif
#if B16 || X86
#define ISTVRELOC(x)	(x==R_IND16)
#endif
#if M32
#define ISTVRELOC(x)	(x!=x)	/* never the case */
#endif
