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

/*******************************************************************************
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**  
**                          All Rights Reserved
**  
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**  
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**  
*******************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**	X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This is the header file for the DMI POINT pipeline element module.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.0
**	RISC ULTRIX V3.0
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Fri Apr  6 15:05:04 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _DMIPOINT
#define _DMIPOINT

/*
**  Include files
*/
#include <SmiPipe.h>

    /*	 
    **  Function reference
    */	 
int             DmiPoint();
int             DmiCreatePoint();

/*
**  Structure definitions and Typedefs
*/

/*
**  Point pipeline context block.
*/
typedef struct _PointPipeCtx {
    PipeElementCommonPart   common;
    struct _PointPart {
	PipeSinkPtr		srcsnk;
	PipeSinkPtr		dstsnk;
	PipeDrainPtr		srcdrn;
	UdpPtr			cpp;
	long int		x1;	/* ROI or CPP position and extent */
	long int                x2;
	long int		y1;
	long int		y2;
	int                     cmpin;
	int                     cmpout;
	UdpPtr			table[XieK_MaxComponents];
	UdpPtr			intable[XieK_MaxComponents];
    } PointPart;
} PointPipeCtx, *PointPipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Point part, access MACROs
*/
#define PntSrcSnk_(ctx) (((PointPipeCtxPtr)(ctx))->PointPart.srcsnk)
#define PntDstSnk_(ctx) (((PointPipeCtxPtr)(ctx))->PointPart.dstsnk)

#define PntSrcDrn_(ctx)	    (((PointPipeCtxPtr)(ctx))->PointPart.srcdrn)
#define PntCpp_(ctx)	    (((PointPipeCtxPtr)(ctx))->PointPart.cpp)
#define PntRoiX1_(ctx)	    (((PointPipeCtxPtr)(ctx))->PointPart.x1)
#define PntRoiX2_(ctx)	    (((PointPipeCtxPtr)(ctx))->PointPart.x2)
#define PntRoiY1_(ctx)	    (((PointPipeCtxPtr)(ctx))->PointPart.y1)
#define PntRoiY2_(ctx)	    (((PointPipeCtxPtr)(ctx))->PointPart.y2)
#define PntCmpIn_(ctx)       (((PointPipeCtxPtr)(ctx))->PointPart.cmpin)
#define PntCmpOut_(ctx)       (((PointPipeCtxPtr)(ctx))->PointPart.cmpout)
#define PntInTable_(ctx,comp) \
			(((PointPipeCtxPtr)(ctx))->PointPart.intable[comp])
#define PntTable_(ctx,comp)  (((PointPipeCtxPtr)(ctx))->PointPart.table[comp])

#endif
/* _DMIPOINT */
