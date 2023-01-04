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
**      This is the header file for the AREA pipeline element module.
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
**      Fri Jun  1 14:09:11 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMIAREA
#define _SMIAREA

/*
**  Include files
*/
#include <SmiPipe.h>

/*	 
**  Function reference
*/	 
int	SmiArea();
int	SmiCreateArea();


/*
**  Structure definitions and Typedefs
*/

/*
**  Area pipeline context block.
*/
typedef struct _AreaPipeCtx {
    PipeElementCommonPart   common;
    struct _AreaPart {
	PipeSinkPtr		srcsnk;
	PipeSinkPtr		dstsnk;
	PipeDrainPtr		srcdrn;
	TmpDatPtr               kernel;
	UdpPtr                  *window[XieK_MaxComponents];
	PipeDataPtr             *dwindow[XieK_MaxComponents];
	int                     window_next[XieK_MaxComponents];
	int                     window_center[XieK_MaxComponents];
	UdpPtr			idc;
	UdpPtr                  cpp;	/* Converted CPP */
	long int                X1;
	long int                X2;
	long int                Y1;
	long int                Y2;
	int     		opidx;
    } AreaPart;
} AreaPipeCtx, *AreaPipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Area part, access MACROs
*/
#define AreaSrcSnk_(ctx) \
		(((AreaPipeCtxPtr)(ctx))->AreaPart.srcsnk)

#define AreaDstSnk_(ctx) (((AreaPipeCtxPtr)(ctx))->AreaPart.dstsnk)

#define AreaSrcDrn_(ctx) \
		(((AreaPipeCtxPtr)(ctx))->AreaPart.srcdrn)
#define AreaKernel_(ctx) \
                (((AreaPipeCtxPtr)(ctx))->AreaPart.kernel)
#define AreaWindow_(ctx,comp) \
		(((AreaPipeCtxPtr)(ctx))->AreaPart.window[comp])
#define AreaDWindow_(ctx,comp) \
		(((AreaPipeCtxPtr)(ctx))->AreaPart.dwindow[comp])
#define AreaWindowNxt_(ctx,comp) \
		(((AreaPipeCtxPtr)(ctx))->AreaPart.window_next[comp])
#define AreaWindowCtr_(ctx,comp) \
		(((AreaPipeCtxPtr)(ctx))->AreaPart.window_center[comp])
#define AreaIdc_(ctx)	  (((AreaPipeCtxPtr)(ctx))->AreaPart.idc)
#define AreaCpp_(ctx)	  (((AreaPipeCtxPtr)(ctx))->AreaPart.cpp)
#define AreaRoiX1_(ctx)	  (((AreaPipeCtxPtr)(ctx))->AreaPart.X1)
#define AreaRoiX2_(ctx)	  (((AreaPipeCtxPtr)(ctx))->AreaPart.X2)
#define AreaRoiY1_(ctx)	  (((AreaPipeCtxPtr)(ctx))->AreaPart.Y1)
#define AreaRoiY2_(ctx)	  (((AreaPipeCtxPtr)(ctx))->AreaPart.Y2)
#define AreaOpIdx_(ctx) \
		(((AreaPipeCtxPtr)(ctx))->AreaPart.opidx)

#endif
/* _SMIAREA */
