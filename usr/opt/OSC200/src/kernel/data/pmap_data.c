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
 *	@(#)$RCSfile: pmap_data.c,v $ $Revision: 1.2.2.17 $ (DEC) $Date: 1993/01/19 15:51:06 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * OSF/1 Release 1.0
 */
/*
 * Modification History: ...msp/src/kernel/dec/data/pmap_data.c
 *
 * 03-Oct-91 -- ald
 *      Added FLAMINGO TurboChannel bus mapping area
 *
 * 25-Sep-91 -- prm
 *	added RUBY Gbus mapping area
 */

#include <machine/pmap.h>

#ifdef	ALPHAADU
#include <hal/ka_adu.h>
extern vm_offset_t tv_cnfg_base;	/* lie */
extern vm_offset_t adu_tv_cons_base;	/* lie */
#endif	/* ALPHAADU */

#include <memd.h>
#if NMEMD > 0
extern vm_offset_t MD_mach_buf;
#endif

struct sys_space sys_space[] = {
/* ex:	{size, &myaddr, cputype}, */
#ifdef	ALPHAADU
	{ 8192, &tv_cnfg_base, 0 },
	{ TVIOMAPSIZE * 8192, &adu_tv_cons_base, 0},
#if NMEMD > 0
	{ NMEMD * 512, &MD_mach_buf, 0},
#endif	/* NMEMD */
#endif	/* ALPHAADU */
	{0, 0, 0 }          /* must be last element */
};

int nsys_space_elems = sizeof(sys_space)/sizeof(struct sys_space) - 1;


