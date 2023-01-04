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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
** File: 
**
**   rop.h --- RasterOps device interface header
**
** Author: 
**
**   Joel Gringorten ???
**
** Revisions:
**
**   29.08.91 Carver
**     - added RopRec and RopPtr types; this structure is now used by rop.c
**       and libpip.c uniformly as a handle to the board.
**
*/
#ifndef ROP_H
#define ROP_H

#include <servermd.h>

typedef struct {
	unsigned char *board;
	unsigned char *bit8;
	unsigned char *bit24;
	unsigned char *SelectionPlane;
	unsigned char *VideoEnablePlane;
	unsigned char *mapRegister;
	unsigned char *MappedArea;  /*3max only */
	unsigned char *pipRegisters;
	unsigned char *dutyCycle;
#if LONG_BIT == 64 && defined(__alpha)
	unsigned char *dense_board;
	unsigned char *dense_bit8;
	unsigned char *dense_bit24;
	unsigned char *sparse_board;
	unsigned char *sparse_bit8;
	unsigned char *sparse_bit24;
#undef TCO_DUTY_CYCLE
#define TCO_DUTY_CYCLE    0x080040  /* Duty Cycle */
#endif

} ropStruct, RopRec, *RopPtr;

#ifndef TCO_DUTY_CYCLE    0x040020  /* Duty Cycle */
#define TCO_DUTY_CYCLE    0x040020  /* Duty Cycle */
#endif

#endif ROP_H
