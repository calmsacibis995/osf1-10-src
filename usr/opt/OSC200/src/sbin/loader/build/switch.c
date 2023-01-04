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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: switch.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:14:14 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* switch.c
 * 
 * the loader switch
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

#ifdef	LDR_PRELOAD_MGR
extern int ldr_preload_entry();
#endif /* LDR_PRELOAD_MGR */

#ifdef	LDR_MACHO_MGR
extern int ldr_macho_entry();
#endif /* LDR_MACHO_MGR */

#ifdef	LDR_COFF_MGR
extern int ldr_coff_entry();
#endif /* LDR_COFF_MGR */

#ifdef	LDR_RCOFF_MGR
extern int ldr_rcoff_entry();
#endif /* LDR_RCOFF_MGR */

#ifdef	LDR_AOUT_MGR
extern int ldr_aout_entry();
#endif /* LDR_AOUT_MGR */

#ifdef	LDR_DUMMY_MGR
extern int ldr_dummy_entry();
#endif /* LDR_DUMMY_MGR */

/* NOTE that entries in this table should be in the order
 * that the recognizers should be called.  In particular,
 * the preload manager should always be the first manager
 * in the table.
 */

ldr_entry_pt_t ldr_manager_entries[] = {

#ifdef	LDR_PRELOAD_MGR
	ldr_preload_entry,
#endif /* LDR_PRELOAD_MGR */

#ifdef	LDR_MACHO_MGR
	ldr_macho_entry,
#endif /* LDR_MACHO_MGR */

#ifdef	LDR_COFF_MGR
	ldr_coff_entry,
#endif /* LDR_COFF_MGR */

#ifdef	LDR_RCOFF_MGR
	ldr_rcoff_entry,
#endif /* LDR_RCOFF_MGR */

#ifdef	LDR_AOUT_MGR
	ldr_aout_entry,
#endif /* LDR_AOUT_MGR */

#ifdef	LDR_DUMMY_MGR
	ldr_dummy_entry,
#endif /* LDR_DUMMY_MGR */

};

int n_ldr_mgr_entries = sizeof(ldr_manager_entries) / sizeof(ldr_entry_pt_t);
