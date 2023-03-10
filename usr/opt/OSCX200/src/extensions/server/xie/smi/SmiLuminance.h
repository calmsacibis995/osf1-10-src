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
**      This is the header file for the LUMINANCE pipeline element module.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.0
**	RISC ULTRIX V3.0
**
**  AUTHOR(S):
**
**      John Weber
**
**  CREATION DATE:
**
**      February 12, 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMILUMINANCE
#define _SMILUMINANCE

/*
**  Include files
*/
#include <SmiPipe.h>

    /*	 
    **  Function reference
    */	 
int             SmiLuminance();
int             SmiCreateLuminance();

/*
**  Structure definitions and Typedefs
*/

/*
**  Luminance pipeline context block.
*/
typedef struct _LuminancePipeCtx {
    PipeElementCommonPart   common;
    struct _LuminancePart {
	PipeSinkPtr	     srcsnk;
	PipeSinkPtr	     dstsnk;
	PipeDrainPtr	     reddrn;
	PipeDrainPtr	     grndrn;
	PipeDrainPtr	     bludrn;
	PipeDataPtr	     reddat;
	PipeDataPtr	     grndat;
	PipeDataPtr	     bludat;
	float                weight[XieK_MaxComponents];
    } LuminancePart;
} LuminancePipeCtx, *LuminancePipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Luminance part, access MACROs
*/
#define LumRedDrn_(ctx)	(((LuminancePipeCtxPtr)(ctx))->LuminancePart.reddrn)
#define LumGrnDrn_(ctx)	(((LuminancePipeCtxPtr)(ctx))->LuminancePart.grndrn)
#define LumBluDrn_(ctx)	(((LuminancePipeCtxPtr)(ctx))->LuminancePart.bludrn)
#define LumRedDat_(ctx)	(((LuminancePipeCtxPtr)(ctx))->LuminancePart.reddat)
#define LumGrnDat_(ctx)	(((LuminancePipeCtxPtr)(ctx))->LuminancePart.grndat)
#define LumBluDat_(ctx)	(((LuminancePipeCtxPtr)(ctx))->LuminancePart.bludat)
#define LumWeight_(ctx,comp) \
	(((LuminancePipeCtxPtr)(ctx))->LuminancePart.weight[comp])
#define LumWeightPtr_(ctx) \
	(&((LuminancePipeCtxPtr)(ctx))->LuminancePart.weight[0])
/*
**  Luminance Parameters
*/
#define LumSrcSnk_(ctx) (((LuminancePipeCtxPtr)(ctx))->LuminancePart.srcsnk)
#define LumDstSnk_(ctx) (((LuminancePipeCtxPtr)(ctx))->LuminancePart.dstsnk)

#endif
/* _SMILUMINANCE */
