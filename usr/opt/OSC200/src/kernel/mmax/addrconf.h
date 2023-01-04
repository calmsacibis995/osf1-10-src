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
 *	@(#)$RCSfile: addrconf.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:28 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* addrconf.h -- Machine-dependent address config record initialization
 * Note that this file contains a data structure initialization.  Thus,
 * it must ONLY be included by a single C file: bsd/kern_mman.c.
 * NO ONE ELSE SHOULD INCLUDE THIS FILE!
 * Depends on <sys/types.h> and <sys/addrconf.h>
 *
 * OSF/1 Release 1.0
 */

/*
 * Where should the loader live on the Multimax?  In general, we'd
 * like for it to live as high and out of the way as possible.
 * However, on the Multimax there some restrictions.  For example,
 * crt0 initialize the global variable, _environ, as follows.
 *
 *                      movd r0,@_environ
 *
 * This instruction uses the absolute addressing mode.  When packed
 * into an instruction, the absolute displacement (e.g. the value
 * @_environ) must fit into a signed 30-bit field.  However, it is
 * invalid for the value to be negative.  So for all practical
 * purposes, the value of the symbol _environ must not be greater than
 * 29 bits in size.  Therefore all loader text, data and BSS must
 * reside below 0x1fffffff.  We currently select 0x1f000000, which
 * gives the loader 16 megabytes.
 */

/*
 * The above reasoning was used to find a location for the loader.
 * Likewise, shared libraries will suffer similar problems.  Therefore
 * this address space configuration record effectively limits the
 * amount of usable address space on the Multimax, excluding the stack,
 * to 512 MB.
 */

struct addressconf addressconf[AC_N_AREAS] = {

	{ (caddr_t)0x4000, AC_UPWARD|AC_FIXED }, /* AC_TEXT */
	{ (caddr_t)0x4000, AC_UPWARD|AC_FLOAT }, /* AC_DATA */
	{ (caddr_t)0x4000, AC_UPWARD|AC_FLOAT }, /* AC_BSS */
	{ (caddr_t)0x7fffe000, AC_DOWNWARD|AC_FIXED }, /* AC_STACK */
	{ (caddr_t)0x1f000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_TEXT */
	{ (caddr_t)0x1f000000, AC_UPWARD|AC_FLOAT }, /* AC_LDR_DATA */
	{ (caddr_t)0x1f000000, AC_UPWARD|AC_FLOAT }, /* AC_LDR_BSS */
	{ (caddr_t)0x1e000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_PRIV */
	{ (caddr_t)0x1d000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_GLB */
	{ (caddr_t)0x1c000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_PRELOAD */
	{ (caddr_t)0x4000000, AC_UPWARD|AC_FLOAT }, /* AC_MMAP_TEXT */
	{ (caddr_t)0x4000000, AC_UPWARD|AC_FLOAT }, /* AC_MMAP_DATA */
	{ (caddr_t)0x4000000, AC_UPWARD|AC_FLOAT }, /* AC_MMAP_BSS */
};
