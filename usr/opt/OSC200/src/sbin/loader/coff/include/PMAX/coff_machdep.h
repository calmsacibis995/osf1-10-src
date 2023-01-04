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
 *	@(#)$RCSfile: coff_machdep.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:35:45 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* coff_machdep.h
 * Machine-dependent definitions for PMAX COFF files
 *
 * This file contains the definitions for the machine-dependent macros
 * used by the COFF format-dependent manager.
 *
 * OSF/1 Release 1.0
 */

#include <a.out.h>
#include <elf_abi.h>
#include <elf_mips.h>

/* The COFF_MAGICOK macro returns true iff this is a valid COFF file
 * for this machine type.  Argument is pointer to the file header
 * of the file.
 */

#define COFF_MAGICOK(fhdr) (((fhdr)->f_magic == MIPSELMAGIC) || \
			    ((fhdr)->f_magic == SMIPSELMAGIC) || \
			    ((fhdr)->f_magic == MIPSELUMAGIC) )

/* These macros return the file offset, base virtual address, and
 * size in bytes of each region in the COFF file.  They are machine-
 * dependent because different manufacturers represent this information
 * differently in the COFF file.  On PMAX, we can simply
 * use the standard N_TXTOFF macro.  Arguments are pointers to the file
 * header and a.out header of the file.
 */

#define COFF_SECTALIGN	4096
#define COFF_TXTOFF(fhdr, ohdr)	N_TXTOFF((*fhdr), (*ohdr))
#define COFF_TXTSTART(fhdr, ohdr) (trunc((ohdr)->text_start, ldr_getpagesize()))
#define COFF_TXTEND(fhdr, ohdr) (round((ohdr)->text_start + (ohdr)->tsize, \
				       ldr_getpagesize()))

#define COFF_DATAOFF(fhdr, ohdr) (COFF_TXTOFF((fhdr), (ohdr)) + (ohdr)->tsize)
#define COFF_DATASTART(fhdr, ohdr) (trunc((ohdr)->data_start, ldr_getpagesize()))
#define COFF_DATAEND(fhdr, ohdr) (round((ohdr)->data_start + (ohdr)->dsize, \
					ldr_getpagesize()))

#define COFF_BSSSTART(fhdr, ohdr) (trunc((ohdr)->bss_start, ldr_getpagesize()))
#define COFF_BSSEND(fhdr, ohdr) (round((ohdr)->bss_start + (ohdr)->bsize, \
				       ldr_getpagesize()))

/* COFF_ENTRYPT returns the entry point address from the COFF module */

#define COFF_ENTRYPT(fhdr, ohdr) ((ohdr)->entry)
