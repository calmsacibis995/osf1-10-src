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
 *	@(#)$RCSfile: addrconf.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:07:30 $
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
/* 
 * derived from addrconf.h	2.1	(ULTRIX/OSF)	12/3/90
 */

struct addressconf addressconf[AC_N_AREAS] = {

	{ (caddr_t)0x400000, AC_UPWARD|AC_FIXED }, /* AC_TEXT */
	{ (caddr_t)0x10000000, AC_UPWARD|AC_FIXED }, /* AC_DATA */
	{ (caddr_t)0x10000000, AC_UPWARD|AC_FLOAT }, /* AC_BSS */
	{ (caddr_t)0x7ffff000, AC_DOWNWARD|AC_FIXED }, /* AC_STACK */
	{ (caddr_t)0x70000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_TEXT */
	{ (caddr_t)0x70000000, AC_UPWARD|AC_FLOAT }, /* AC_LDR_DATA */
	{ (caddr_t)0x70000000, AC_UPWARD|AC_FLOAT }, /* AC_LDR_BSS */
	{ (caddr_t)0x71000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_PRIV */
	{ (caddr_t)0x72000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_GLB */
	{ (caddr_t)0x73000000, AC_UPWARD|AC_FIXED }, /* AC_LDR_PRELOAD */
	{ (caddr_t)0x400000, AC_UPWARD|AC_FLOAT }, /* AC_MMAP_TEXT */
	{ (caddr_t)0x40000000, AC_UPWARD|AC_FLOAT }, /* AC_MMAP_DATA */
	{ (caddr_t)0x40000000, AC_UPWARD|AC_FLOAT }, /* AC_MMAP_BSS */
};
