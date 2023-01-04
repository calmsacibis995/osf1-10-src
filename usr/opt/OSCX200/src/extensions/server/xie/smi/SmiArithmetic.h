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
**      This is the header file for the ARITHMETIC pipeline element module.
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
**      Fri Apr 20 15:08:58 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMIARITHMETIC
#define _SMIARITHMETIC

/*
**  Include files
*/
#include <SmiPipe.h>

/*	 
**  Function reference
*/	 
int             SmiArithmetic();
int             SmiCreateArithmetic();


/*
**  Structure definitions and Typedefs
*/

/*
**  Arithmetic pipeline context block.
*/
typedef struct _ArithmeticPipeCtx {
    PipeElementCommonPart   common;
    struct _ArithmeticPart {
	PipeSinkPtr		srcsnk1;
	PipeSinkPtr		srcsnk2;
	PipeSinkPtr		dstsnk;
	PipeDrainPtr		srcdrn1;
	PipeDrainPtr		srcdrn2;
	PipeDataPtr             srcdata1;
	PipeDataPtr             srcdata2;
	float			fconst[XieK_MaxComponents];
	long int		x1;
	long int		x2;
	long int		y1;
	long int		y2;
	UdpPtr                  cpp;
	int		      (*function)();
    } ArithmeticPart;
} ArithmeticPipeCtx, *ArithmeticPipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Arithmetic part, access MACROs
*/
#define AriSrcSnk1_(ctx) (((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.srcsnk1)
#define AriSrcSnk2_(ctx) (((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.srcsnk2)
#define AriDstSnk_(ctx)  (((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.dstsnk)

#define AriSrcDrn1_(ctx) (((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.srcdrn1)
#define AriSrcDrn2_(ctx) (((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.srcdrn2)
#define AriSrcData1_(ctx) \
			(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.srcdata1)
#define AriSrcData2_(ctx) \
			(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.srcdata2)
#define AriFConst_(ctx,comp) \
		    (((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.fconst[comp])
#define AriRoiX1_(ctx)	(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.x1)
#define AriRoiX2_(ctx)	(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.x2)
#define AriRoiY1_(ctx)	(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.y1)
#define AriRoiY2_(ctx)	(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.y2)
#define AriCpp_(ctx)	(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.cpp)
#define AriFunc_(ctx)	(((ArithmeticPipeCtxPtr)(ctx))->ArithmeticPart.function)

#endif
/* _SMIARITHMETIC */
