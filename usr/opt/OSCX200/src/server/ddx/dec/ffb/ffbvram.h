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
 * @(#)$RCSfile: ffbvram.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:18:56 $
 */
/*
 */
#ifndef _FFBVRAM_H_
#define _FFBVRAM_H_

/* 16 MB start of fb */
#if FFBPIXELBITS==8
#define FFBMODEL_VRAMBASE	0x800000
#elif FFBPIXELBITS==32
#define FFBMODEL_VRAMBASE	0x1000000
#endif

/* models notion of beginning of vram */
extern unsigned char *vram;
extern unsigned vram_bytes;

extern int	      screen_width;
extern int	      phys_screen_width;
extern int	      screen_height;

struct model_colormap{
	unsigned short palette[3][512];
};

extern unsigned char *sfbVRAM;

#include <sys/types.h>
#include <sys/workstation.h>

#define N10_PRESENT
#define PQ_DEBUG
#ifdef __alpha
#  include <io/dec/ws/bt459.h>
#  include <io/dec/ws/px.h>
#  include <io/dec/ws/pq.h>
#  include <io/dec/ws/stamp.h>
#  define _PQO_RAM(P) ((int *)((pq_mapd *)(P)->dev.pq.pxod)->ram.reqbuf)
#else
#  include <io/ws/bt459.h>
#  include <io/ws/px.h>
#  include <io/ws/pq.h>
#  include <io/ws/stamp.h>
#  define _PQO_RAM(P) ((int *)PQO_RAM(P))
#endif

extern pxInfo *pxInfoPtr;

#endif /* ifndef _FFBVRAM_H_ */

/*
 * HISTORY
 */
