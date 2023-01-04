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
 *	@(#)$RCSfile: usercopy.s,v $ $Revision: 1.2.3.3 $ (DEC) $Date: 1992/03/18 16:02:48 $
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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */


/*	Code within this file moved to individual modules based on */
/*	procedure name.  */

/*	arch/mips/usercopy.d/addupc.s		standard Binary */
/*	arch/mips/usercopy.d/bcmp.s		standard Binary */
/*	arch/mips/usercopy.d/bcopy.s		standard Binary */
/*	arch/mips/usercopy.d/bzero.s		standard Binary */
/*	arch/mips/usercopy.d/clearseg.s		standard Binary */
/*	arch/mips/usercopy.d/copyin.s		standard Binary */
/*	arch/mips/usercopy.d/copyinstr.s	standard Binary */
/*	arch/mips/usercopy.d/copyout.s		standard Binary */
/*	arch/mips/usercopy.d/copyoutstr.s	standard Binary */
/*	arch/mips/usercopy.d/copystr.s		standard Binary */
/*	arch/mips/usercopy.d/fubyte.s		standard Binary */
/*	arch/mips/usercopy.d/fuword.s		standard Binary */
/*	arch/mips/usercopy.d/hwcp.s		standard Binary */
/*	arch/mips/usercopy.d/strlen.s		standard Binary */
/*	arch/mips/usercopy.d/subyte.s		standard Binary */
/*	arch/mips/usercopy.d/suword.s		standard Binary */
/*	arch/mips/usercopy.d/uload_half.s	standard Binary */
/*	arch/mips/usercopy.d/uload_uhalf.s	standard Binary */
/*	arch/mips/usercopy.d/uload_word.s	standard Binary */
/*	arch/mips/usercopy.d/ustore_half.s	standard Binary */
/*	arch/mips/usercopy.d/ustore_word.s	standard Binary */
/*	 */
/*	arch/mips/hal/kn01_clean_dcache.s	standard Binary */
/*	arch/mips/hal/kn01_clean_icache.s	standard Binary */
/*	arch/mips/hal/kn01_page_dflush.s	standard Binary */
/*	arch/mips/hal/kn01_page_iflush.s	standard Binary */
/*	arch/mips/hal/kn01flush_cache.s		standard Binary */
/*	arch/mips/hal/kn5800_cln_dcache.s	standard Binary */
/*	arch/mips/hal/kn5800_cln_icache.s	standard Binary */
/*	arch/mips/hal/kn5800_flsh_cache.s	standard Binary */
/*	arch/mips/hal/kn5800_pg_dflush.s	standard Binary */
/*	arch/mips/hal/kn5800_pg_iflush.s	standard Binary */
/*	 */
/*	arch/mips/cstrerror.s		standard Binary */
/*	arch/mips/fixade_error.s	standard Binary */
/*	arch/mips/uerror.s		standard Binary */
/*	arch/mips/cerror.s		standard Binary */
/*	 */
/*	arch/mips/config_cache.s	standard Binary */
/*	arch/mips/size_cache.s		standard Binary */
