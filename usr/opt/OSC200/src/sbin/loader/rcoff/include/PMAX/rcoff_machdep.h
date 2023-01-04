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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* rcoff_machdep.h
 * Machine-dependent definitions for PMAX relocatable COFF files
 *
 * This file contains the definitions for the machine-dependent macros
 * used by the relocatable COFF format-dependent manager.
 *
 * OSF/1 Release 1.0
 */

#include <a.out.h>

/* The RCOFF_MAGICOK macro returns true iff this is a valid COFF file
 * for this machine type.  Argument is pointer to the file header
 * of the file.
 */

#define RCOFF_MAGICOK(fhdr) (((fhdr)->f_magic == MIPSELMAGIC) || \
			    ((fhdr)->f_magic == SMIPSELMAGIC) || \
			    ((fhdr)->f_magic == MIPSELUMAGIC) )

/* These macros return the file offset, base virtual address, and
 * size in bytes of each region in the COFF file.  They are machine-
 * dependent because different manufacturers represent this information
 * differently in the COFF file.  On PMAX, we can simply
 * use the standard N_TXTOFF macro.  Arguments are pointers to the file
 * header and a.out header of the file.
 */

#define RCOFF_SECTALIGN	4096
#define RCOFF_TXTOFF(fhdr, ohdr)	N_TXTOFF((*fhdr), (*ohdr))
#define RCOFF_TXTSTART(fhdr, ohdr) (trunc((ohdr)->text_start, ldr_getpagesize()))
#define RCOFF_TXTEND(fhdr, ohdr) (round((ohdr)->text_start + (ohdr)->tsize, \
				       ldr_getpagesize()))

#define RCOFF_DATAOFF(fhdr, ohdr) (RCOFF_TXTOFF((fhdr), (ohdr)) + (ohdr)->tsize)
#define RCOFF_DATASTART(fhdr, ohdr) (trunc((ohdr)->data_start, ldr_getpagesize()))
#define RCOFF_DATAEND(fhdr, ohdr) (round((ohdr)->data_start + (ohdr)->dsize, \
					ldr_getpagesize()))

#define RCOFF_BSSSTART(fhdr, ohdr) (trunc((ohdr)->bss_start, ldr_getpagesize()))
#define RCOFF_BSSEND(fhdr, ohdr) (round((ohdr)->bss_start + (ohdr)->bsize, \
				       ldr_getpagesize()))

/* RCOFF_ENTRYPT returns the entry point address from the COFF module */

#define RCOFF_ENTRYPT(fhdr, ohdr) ((ohdr)->entry)

/* Transfer vector declarations */

typedef union {
	struct {
		unsigned immed : 16;
		unsigned rt : 5;
		unsigned rs : 5;
		unsigned op : 6;
	} I;
	struct {
		unsigned funct : 6;
		unsigned re : 5;
		unsigned rd : 5;
		unsigned rt : 5;
		unsigned rs : 5;
		unsigned op : 6;
	} R;
	unsigned	word;
} MIPS_inst_t;

typedef MIPS_inst_t vector_t[4];

#define LUI(rt, immed) ((017 << 26) | (0 << 21) | (rt << 16) | immed)
#define ORI(rt, rs, immed) ((015 << 26) | (rs << 21) | (rt << 16) | immed)
#define JR(rs) ((0 << 26) | (rs << 21) | (0 << 16) | (0 << 11) | (0 << 6) | 010)
#define NOP ((0 << 26) | (0 << 21) | (0 << 16) | (0 << 11) | (0 << 6) | 0)
#define AT_REG 1
