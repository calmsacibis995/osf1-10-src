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
 *	@(#)$RCSfile: vbareg.h,v $ $Revision: 1.2.9.2 $ (DEC) $Date: 1993/07/14 18:51:27 $
 */ 
/*
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from vbareg.h	4.2	(ULTRIX)	1/23/9
 */

/*
 * Abstract:
 *	This module contains the definitions for general VMEbus host
 *	modules.
 *
 * Revision History
 *
 *	05-Jun-1991	Mark Parenti
 *		Add definition of vbahd entry in bus structure.
 *
 *	Above this line is OSF/1
 *	--------------------------------------------------------------------
 *
 *	22-Jan-1990	Mark Parenti
 *		Add asc field to vbadata structure.  Selects if DMA PMRs
 *		are mapped to 1st or 2nd GB of VMEbus address space.
 *
 *	14-Nov-1989	Mark Parenti (map)
 *		Original Version
 */

#ifndef _VBAREG_H
#define _VBAREG_H

/*	Definitions for flags parameter to vbasetup()	*/

#define	VME_A16		0x00000000	/* A16 Request			*/
#define	VME_A24		0x00000001	/* A24 Request			*/
#define	VME_A32		0x00000002	/* A32 Request			*/
#define	VME_D08		0x00000000	/* D08 Data Size		*/
#define	VME_D16		0x00000010	/* D16 Data Size		*/
#define	VME_D32		0x00000020	/* D32 Data Size		*/
#define	VME_USER	0x00000040	/* Supervisory Access Mode (if 0)*/
					/* User Access Mode (if 1)	*/
#define	VME_DATA	0x00000004	/* Prog Access Mode (if 0)	*/
					/* Data Access Mode (if 1)	*/
#define	VME_AM		0x0000FF00	/* Addr Modifier bits (if non-zero)*/
					/* Override other bits. */
#define	VME_AM_SHIFT	8		/* Addr Modifier bits shift count */
#define	VME_BS_MASK	0x00030000	/* Byte Swap mask		*/
#define	VME_BS_SHIFT	16		/* Byte Swap shift count	*/
#define	VME_ASPACE_MASK	0x00000003	/* Address space mask		*/
#define	VME_ASIZE_MASK	0x00000030	/* Data space mask		*/
#define	VME_ASIZE_SHIFT	4		/* Shift for array index	*/
#define	VME_ADD_MASK	(VME_A16 | VME_A24 | VME_A32)

#define MAX_A16_ADDR    0xffff
#define MAX_A24_ADDR    0xffffff

#define	vbahd		private[4]

/************************************************************************/
/* The following values are used for the flags parameter of the		*/
/* vbasetup() and vballoc() routines. They may be combined by using the	*/
/* C-language bit-wise or ( | ).					*/
/************************************************************************/
#define	VMEA16D08	(VME_A16 | VME_D08)
#define	VMEA16D16	(VME_A16 | VME_D16)
#define	VMEA16D32	(VME_A16 | VME_D32)

#define	VMEA24D08	(VME_A24 | VME_D08)
#define	VMEA24D16	(VME_A24 | VME_D16)
#define	VMEA24D32	(VME_A24 | VME_D32)

#define	VMEA32D08	(VME_A32 | VME_D08)
#define	VMEA32D16	(VME_A32 | VME_D16)
#define	VMEA32D32	(VME_A32 | VME_D32)

#define	VME_DMA		0x02000000	/* Need DMA registers		*/
#define	VME_RESERV	0x04000000	/* Reserve VME Address space	*/
#define	VME_CANTWAIT	0x01000000	/* Must have it now		*/
#define	VME_BS_NOSWAP	0x00000000	/* No Byte Swap			*/
#define	VME_RMW_ENAB	0x00100000	/* Enable RMW (XVIB ONLY)	*/
#define	VME_BS_BYTE	0x00010000	/* Byte Swap Bytes		*/
#define	VME_BS_WORD	0x00020000	/* Byte Swap Words		*/
/* some adapters may treat Byte Swap Words as a transaction Size Dependent swap instead */
#define	VME_BS_SIZE_DEPENDENT	0x00020000 /* Byte Swap Dependent upon transaction size	*/
#define	VME_BS_LWORD	0x00030000	/* Byte Swap Longwords		*/

#define	NVME_VECS	0x100	/* Number of VME interrupt levels	*/
#ifdef __alpha
#define	VME_VEC_SIZE	sizeof(struct scbentry)	/* Size of a scb entry 	*/
#else /* __alpha */
#define	VME_VEC_SIZE	sizeof(char *)       	/* Size of a scb entry 	*/
#endif /* __alpha */

#define VME_MAX_VEC     (NVME_VECS - 1)
#define VME_MIN_VEC     (0x40)
#define	VME_A24_VALID	0x800000 /* Lowest valid A24 csr		*/
#define	VME_A32_VALID	0x80000000 /* Lowest valid A32 csr		*/

#define VME_MAX_A16_ADD     0x0000FFFF
#define VME_MAX_A24_ADDR    0x00FFFFFF

/* This field selects where in VME address space the DMA Page Map Registers */
/* are mapped. If 2 adapters are used they must use different values */
/* for this field.  Note that if VME_MAP_HIGH is selected only A32 DMA */
/* is supported.							*/
#define	VME_MAP_LOW		0x00	/* DMA mapped to first 1GB 	*/ 
#define	VME_MAP_HIGH		0x01	/* DMA mapped to second 1GB	*/

extern struct gen_bus_adapt *vme_ba_get();
extern void vme_display_addr_type();
extern int vme_ba_ins();
extern int vme_is_ivec_valid();
extern char vme_read_byte();
extern short vme_read_word();
extern int vme_read_long();
extern void vme_write_byte();
extern void vme_write_word();
extern void vme_write_long();

#endif /* _VBAREG_H */
