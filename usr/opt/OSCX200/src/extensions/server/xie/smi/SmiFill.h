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
**      This is the header file for the FILL pipeline element module.
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
**      Wed Jul 11 10:19:08 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMIFILL
#define _SMIFILL

/*
**  Include files
*/
#include <SmiPipe.h>

    /*	 
    **  Function reference
    */	 
int		SmiCreateFill();
int             SmiFill();
int             SmiFillRegion();

/*
**  Structure definitions and Typedefs
*/

/*
**  Fill pipeline context block.
*/
typedef struct _FillPipeCtx {
    PipeElementCommonPart   common;
    struct _FillPart {
	PipeSinkPtr	     src;
	PipeSinkPtr	     dst;
	long int             x1;
	long int             x2;
	long int             y1;
	long int             y2;
	UdpPtr               cpp;
	unsigned long        constant[XieK_MaxComponents];
	PipeDrainPtr	     srcdrn;
    } FillPart;
} FillPipeCtx, *FillPipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Fill part, access MACROs
*/
#define FilSrcDrn_(ctx)	    (((FillPipeCtxPtr)(ctx))->FillPart.srcdrn)
/*
**  Fill Parameters
*/
#define FilSrcSnk_(ctx)	    (((FillPipeCtxPtr)(ctx))->FillPart.src)
#define FilDstSnk_(ctx)	    (((FillPipeCtxPtr)(ctx))->FillPart.dst)
#define FilRoiX1_(ctx)      (((FillPipeCtxPtr)(ctx))->FillPart.x1)
#define FilRoiX2_(ctx)      (((FillPipeCtxPtr)(ctx))->FillPart.x2)
#define FilRoiY1_(ctx)      (((FillPipeCtxPtr)(ctx))->FillPart.y1)
#define FilRoiY2_(ctx)      (((FillPipeCtxPtr)(ctx))->FillPart.y2)
#define FilCpp_(ctx)        (((FillPipeCtxPtr)(ctx))->FillPart.cpp)
#define FilConst_(ctx,comp) (((FillPipeCtxPtr)(ctx))->FillPart.constant[comp])

#endif
/* _SMIFILL */
