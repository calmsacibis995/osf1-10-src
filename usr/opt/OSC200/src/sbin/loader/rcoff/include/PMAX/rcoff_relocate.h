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
/*
 *
 * rcoff_relocate.h
 *
 * PMAX machine dependent relocatable COFF format dependent loader interfaces.
 *
 * OSF/1 Release 1.0
 */

#ifndef _RCOFF_RELOCATE

#define _RCOFF_RELOCATE

extern int
rcoff_relocate(
	rcoff_module_handle_t module,
	ldr_symbol_rec *imports,
	int nimports,
	ldr_region_rec *regions,
	int nregions,
	struct reloc *reloc,
	univ_t target,
	univ_t vaddr);

/* On a PMAX, the REFHI relocation record is followed by a REFLO.
 * These two relocations are treated as a pair by rcoff_relocate().
 * LDR_RELOC_HI_LO is return as a status code by rcoff_relocate()
 * to indicate to its caller that the following relocation should
 * be skipped.
 */

#define LDR_RELOC_HI_LO 2

#endif /* _RCOFF_RELOCATE */
