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
 *	@(#)$RCSfile: blitvar.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:07:51 $
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
 
/* **********************************************************************
 File:         blitvar.h
 Description:  Definitions used by Blit driver other than h/w definition.


 Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989.
 All rights reserved.
********************************************************************** */

#include <i386/AT386/blitreg.h>
#include <sys/types.h>
#include <mach/boolean.h>


/* 
 * This is how we use the Blit's graphics memory.  The frame buffer 
 * goes at the front, and the rest is used for miscellaneous 
 * allocations.  Users can use the "spare" memory, but they should do 
 * an ioctl to find out which part of the memory is really free.
 */

struct blitmem {
	union blitfb {
		u_char mono_fb[BLIT_MONOFBSIZE];
		u_char color_fb[1];	/* place-holder */
	} fb;
	u_char spare[BLIT_MEMSIZE - sizeof(union blitfb)];
};


/*
 * Macro to get from blitdev pointer to monochrome framebuffer.
 */
#define       BLIT_MONOFB(blt, fbptr) \
	{ struct blitmem *mymem = (struct blitmem *)((blt)->graphmem); \
	fbptr = mymem->fb.mono_fb; \
	}


/* 
 * Single-tile description that can be used to describe the entire 
 * screen. 
 */

struct screen_descrip {
	STRIPHEADER strip;
	TILEDESC tile;
};


/* 
 * Number of microseconds we're willing to wait for display processor 
 * to load its command block.
 */

#define DP_RDYTIMEOUT			1000000


/* 
 * Conversion macros.
 */

#define VM_TO_ADDR786(vmaddr, blit_base) \
	((int)(vmaddr) - (int)(blit_base))


extern boolean_t blit_present();
extern void blit_init();
