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
**      This is the header file for the ChromeCom pipeline element module.
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
**      Wed Jul 11 12:02:22 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMICHROMECOM
#define _SMICHROMECOM

/*
**  Include files
*/
#include <SmiPipe.h>

/*	 
**  Function reference
*/	 
int             SmiCreateChromeCom();

/*
**  Structure definitions and Typedefs
*/

/*
**  ChromeCom pipeline context block.
*/
typedef struct _ChromeComPipeCtx {
    PipeElementCommonPart   common;
    struct _ChromeComPart {
	PipeSinkPtr	     srcsnk[XieK_MaxComponents];
	PipeSinkPtr	     dstsnk;
	PipeDrainPtr	     drn[XieK_MaxComponents];
    } ChromeComPart;
} ChromeComPipeCtx, *ChromeComPipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, ChromeCom part, access MACROs
*/
#define ChcDrn_(ctx,comp) \
	(((ChromeComPipeCtxPtr)(ctx))->ChromeComPart.drn[comp])
/*
**  ChromeCom Parameters
*/
#define ChcSrcSnk_(ctx,comp) \
	(((ChromeComPipeCtxPtr)(ctx))->ChromeComPart.srcsnk[comp])
#define ChcDstSnk_(ctx) (((ChromeComPipeCtxPtr)(ctx))->ChromeComPart.dstsnk)

#endif
/* _SMICHROMECOM */
