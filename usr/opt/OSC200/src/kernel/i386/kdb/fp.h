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
 *	@(#)$RCSfile: fp.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:10:57 $
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

struct frame {
	int *f_handler;
	struct f_fm {
		unsigned int : 5;
		unsigned int fm_psw : 11;
		unsigned int fm_mask : 12;
		unsigned int : 1;
		unsigned int fm_s : 1;
		unsigned int fm_spa : 2;
	} f_msk ;
	int *f_ap ;
	struct frame *f_fp ;
	unsigned char *f_pc ;
	int f_r0[12] ;
} ;

#define f_psw f_msk.f_fmask
#define f_mask f_msk.fm_mask
#define f_s f_msk.fm_s
#define f_spa f_msk.fm_spa
