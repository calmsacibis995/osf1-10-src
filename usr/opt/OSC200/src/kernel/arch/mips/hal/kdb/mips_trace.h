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
 *	@(#)$RCSfile: mips_trace.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:22 $
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
 * derived from mips_trace.h	2.1	(ULTRIX/OSF)	12/3/90
 */

#define SP_REGISTER 29

typedef struct frame_info {
	int narg;		/* number of arguments, if known */
	int nloc;		/* number local variables */
	int framesize;		/* size of stack frame */
	int saved_pc_off;	/* offset of return pc from top of frame */
	int saved_fp_off;	/* offset of saved fp from top of frame */
	unsigned int isleaf:1,	/* leaf procedure */
		     isvector:1,/* exception vector */
		     mod_sp:1,	/* affects the sp */
		     at_entry:1,/* sp has not been modified yet */
		     xxx:28;	/* unused */
	int framereg;		/* frame register */
} *frame_info_t;
