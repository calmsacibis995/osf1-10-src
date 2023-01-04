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
**      This is the header file for the LOGICAL pipeline element module.
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
**      Wed Apr 25 11:02:18 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMILOGICAL
#define _SMILOGICAL

/*
**  Include files
*/
#include <SmiPipe.h>

    /*	 
    **  Function reference
    */	 
int             SmiLogical();
int             SmiCreateLogical();

/*
**  Structure definitions and Typedefs
*/

/*
**  Logical pipeline context block.
*/
typedef struct _LogicalPipeCtx {
    PipeElementCommonPart   common;
    struct _LogicalPart {
	PipeSinkPtr		srcsnk1;
	PipeSinkPtr		srcsnk2;
	PipeSinkPtr		dstsnk;
	PipeDrainPtr		srcdrn1;
	PipeDrainPtr		srcdrn2;
	PipeDataPtr             srcdata1;
	PipeDataPtr             srcdata2;
	unsigned long int       constant[XieK_MaxComponents];
	UdpPtr			cpp;
	long int		x1;
	long int		x2;
	long int		y1;
	long int		y2;
	void    		(*function)();
    } LogicalPart;
} LogicalPipeCtx, *LogicalPipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Logical part, access MACROs
*/
#define LogSrcSnk1_(ctx) \
		(((LogicalPipeCtxPtr)(ctx))->LogicalPart.srcsnk1)
#define LogSrcSnk2_(ctx) \
		(((LogicalPipeCtxPtr)(ctx))->LogicalPart.srcsnk2)
#define LogDstSnk_(ctx) (((LogicalPipeCtxPtr)(ctx))->LogicalPart.dstsnk)

#define LogSrcDrn1_(ctx) \
		(((LogicalPipeCtxPtr)(ctx))->LogicalPart.srcdrn1)
#define LogSrcDrn2_(ctx) \
		(((LogicalPipeCtxPtr)(ctx))->LogicalPart.srcdrn2)
#define LogSrcData1_(ctx) \
                (((LogicalPipeCtxPtr)(ctx))->LogicalPart.srcdata1)
#define LogSrcData2_(ctx) \
                (((LogicalPipeCtxPtr)(ctx))->LogicalPart.srcdata2)
#define LogConst_(ctx,comp) \
		(((LogicalPipeCtxPtr)(ctx))->LogicalPart.constant[comp])
#define LogCpp_(ctx)	  (((LogicalPipeCtxPtr)(ctx))->LogicalPart.cpp)
#define LogRoiX1_(ctx)	  (((LogicalPipeCtxPtr)(ctx))->LogicalPart.x1)
#define LogRoiX2_(ctx)	  (((LogicalPipeCtxPtr)(ctx))->LogicalPart.x2)
#define LogRoiY1_(ctx)	  (((LogicalPipeCtxPtr)(ctx))->LogicalPart.y1)
#define LogRoiY2_(ctx)	  (((LogicalPipeCtxPtr)(ctx))->LogicalPart.y2)
#define LogFunc_(ctx) \
		(((LogicalPipeCtxPtr)(ctx))->LogicalPart.function)

#endif
/* _SMILOGICAL */
