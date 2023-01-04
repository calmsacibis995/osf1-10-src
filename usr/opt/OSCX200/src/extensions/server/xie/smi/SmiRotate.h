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
**      This is the header file for the ROTATE pipeline element module.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.1
**	RISC ULTRIX V3.1
**
**  AUTHOR(S):
**
**      Gary Grebus
**
**  CREATION DATE:
**
**      Fri Mar  2 11:34:55 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMIROTATE
#define _SMIROTATE

/*
**  Include files
*/
#include <SmiPipe.h>

    /*	 
    **  Function reference
    */	 
int     SmiRotate();
int     SmiCreateRotate();

/*
**  Structure definitions and Typedefs
*/

/*
**  Rotate pipeline context block.
*/
typedef struct _RotatePipeCtx {
    PipeElementCommonPart   common;
    struct _RotatePart {
	PipeSinkPtr	     src;
	PipeSinkPtr	     dst;
	double		     angle;
	PipeDrainPtr	     srcdrn;
	unsigned long fill[XieK_MaxComponents]; /* Fill values */
    } RotatePart;
} RotatePipeCtx, *RotatePipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Rotate part, access MACROs
*/
#define RotSrcDrn_(ctx)	  (((RotatePipeCtxPtr)(ctx))->RotatePart.srcdrn)
#define RotAngle_(ctx)	  (((RotatePipeCtxPtr)(ctx))->RotatePart.angle)
#define RotFill_(ctx,i)	  (((RotatePipeCtxPtr)(ctx))->RotatePart.fill[i])

/*
**  Rotate Parameters
*/
#define RotSrcSnk_(ctx)   (((RotatePipeCtxPtr)(ctx))->RotatePart.src)
#define RotDstSnk_(ctx)   (((RotatePipeCtxPtr)(ctx))->RotatePart.dst)

#endif
/* _SMIROTATE */
