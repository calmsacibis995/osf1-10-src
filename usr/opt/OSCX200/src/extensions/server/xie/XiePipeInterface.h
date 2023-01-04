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
Copyright 1989,1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This module contains the macros which are used to access the
**      the DDX routines needed to create and run pipelines.
**	
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Thu Feb  7 17:05:40 1991
**
************************************************************************/
#ifdef DWV3

/* For DECWindows V3, interpose the layer to support yielding */
extern int 	XieAbortPipeline();
extern Pipe 	XieCreatePipeline();
extern int	XieDestroyPipeline();
extern int	XieFlushPipeline();
extern int      XieInitPipelineInterface();
extern int	XieResumePipeline();

#define AbortPipeline_(flo)		(XieAbortPipeline( flo ))
#define CreatePipeline_(client, flo)	(XieCreatePipeline( client, flo ))
#define DestroyPipeline_(flo)		(XieDestroyPipeline( flo ))
#define FlushPipeline_(flo)		(XieFlushPipeline( flo ))
#define InitiatePipeline_(flo)		(DdxInitiatePipeline_( Pipe_(flo) ))
#define InitPipelineInterface_()        (XieInitPipelineInterface())
#define	OptimizePipeline_(flo)		(DdxOptimizePipe_(Pipe_(flo)))
#define ResumePipeline_(flo) 		(XieResumePipeline( flo ))
#else

/* Everywhere else, call the DDX routines directly */
#define AbortPipeline_(flo)		(DdxAbortPipeline_(Pipe_(flo)))
#define CreatePipeline_(client, flo)	(DdxCreatePipeline_( 0 ))
#define DestroyPipeline_(flo)		(DdxDestroyPipeline_( Pipe_(flo) ))
#define FlushPipeline_(flo)		(DdxFlushPipeline_(Pipe_(flo)))
#define InitiatePipeline_(flo)		(DdxInitiatePipeline_(Pipe_(flo)))
#define InitPipelineInterface_()        (Success) /* Dummy */
#define	OptimizePipeline_(flo)		(DdxOptimizePipe_(Pipe_(flo)))
#define ResumePipeline_(flo) 		(DdxResumePipeline_(Pipe_(flo)))

#endif /* DWV3 */
