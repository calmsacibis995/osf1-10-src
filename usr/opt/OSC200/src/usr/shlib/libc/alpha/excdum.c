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
 * $Header: /usr/sde/osf1/rcs/os/src/usr/shlib/libc/alpha/excdum.c,v 1.1.7.2 1993/06/08 01:15:43 Thomas_Woodburn Exp $
 */

/* this file contains dummy routines callable from init and fini sections.
 *	If the user includes libexc, libexc_init.a will call the libexc
 *	routines due to linker and loader ordering and pre-emption rules.
 *	If libexc is not included, we need these routines so all of the
 *	init sections in shared objects have something to call.
 */

/* define these routines in name space pollution free manner using weak pragmas
 * in ./usr/ccs/lib/libc/alpha/find_rtfunc.c.  Leave them here in case we
 * want to turn off _NAME_SPACE_WEAK_STRONG
 */

#ifndef _NAME_SPACE_WEAK_STRONG
#include <excpt.h>

/* these remain in libc because of old binaries and shared objects which
 *	might still reference them. They should be phased out.
 *	libexc_init.a now calls the __exc routines.
 */

/* call __ versions of routines found in libc */

extern void
exc_add_pc_range_table(
	PRUNTIME_FUNCTION	pbase,		/* base of function table */
	pdsc_count		count)		/* how many */
{
    __exc_add_pc_range_table(pbase, count);
} /* exc_add_pc_range_table */


extern void
exc_remove_pc_range_table(
	PRUNTIME_FUNCTION	pbase)		/* base of function table */
{
    __exc_remove_pc_range_table(pbase);
} /* exc_remove_pc_range_table */

extern void
exc_add_gp_range(
	exc_address		first_addr,	/* base of function table */
	pdsc_count		length,		/* in bytes */
	exc_address		gp)		/* gp for this range */
{
    __exc_add_gp_range(first_addr, length, gp);
} /* exc_add_gp_range */

extern void
exc_remove_gp_range(
	exc_address		first_addr)	/* base of function table */
{
    __exc_remove_gp_range(first_addr);
} /* exc_remove_gp_range */
#endif /* _NAME_SPACE_WEAK_STRONG */
