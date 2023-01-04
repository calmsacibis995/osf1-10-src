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
/* $XConsortium: bifconfig.c,v 5.1 91/02/16 10:06:56 rws Exp $ */
/***********************************************************
Copyright (c) 1989,1990, 1991 by Sun Microsystems, Inc. and the X Consortium at M.I.T.

						All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*--------------------------------------------------------------------*\
|  Copyright (C) 1989,1990, 1991, National Computer Graphics Association
|
|  Permission is granted to any individual or institution to use, copy, or
|  redistribute this software so long as it is not sold for profit, provided
|  this copyright notice is retained.
|
|                         Developed for the
|                National Computer Graphics Association
|                         2722 Merrilee Drive
|                         Fairfax, VA  22031
|                           (703) 698-9600
|
|                                by
|                 SimGraphics Engineering Corporation
|                    1137 Huntington Drive  Unit A
|                      South Pasadena, CA  91030
|                           (213) 255-0900
|---------------------------------------------------------------------
|
| Author        :	John M. Zulauf / SimGraphics Engineering Corp.
|
| File          :	bifconfig.c
| Date          :	Sat Jun 24 15:10:09 PDT 1989
| Project       :	PLB
| Description   :	System Configuration and Initialization Calls
| Status        :	Version 1.0
|
| Revisions     :	
|	2/89		MFR SimGEC: Added full configuration capabilities
|
|       5/90            MFC Tektronix, Inc.: PEX-SI API Binding change.
|
|      12/90            MFC Tektronix, Inc.: PEX-SI PEX5R1 Release.
|
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Table of Contents
|
|	int bif_defaultconfig()
|		:	set the BIF defaultconfig configuration value
|	int bif_colormodel(BIF_INT)
|		:	set the BIF color_model configuration value
|	int bif_colormode(BIF_INT)
|		:	set the BIF color_mode configuration value
|	int bif_buffermode(BIF_INT)
|		:	set the BIF buffer_mode configuration value
|
\*--------------------------------------------------------------------*/

/*---------------------------------------------------------------------*\
|	Include files 
\*--------------------------------------------------------------------- */
#include <stdio.h>
#include "biftypes.h"
#include "bifparse.h"
#include "bifmacro.h"
#include "globals.h"
#include "ph_map.h"
#include "brfexption.h"
#define EXCEPTION_HANDLER 1

/*----------------------------------------------------------------------*\
| Procedure	:	int bif_defaultconfig()
|------------------------------------------------------------------------|
| Description	:	 set the BIF defaultconfig configuration value
|
|		This call is a configuration parameter setting routine.
|		The system dependant calls are made in bif_openwk();
|------------------------------------------------------------------------|
| Return	:	Error Code
\*----------------------------------------------------------------------*/
int bif_defaultconfig()
{

#ifdef TEST_PRINT
	printf("DEFAULT CONFIGURATION : \n");
#endif /* TEST_PRINT */
	bif_colormodel(RGB);
	bif_colormode(TRUE_COLOR);
	bif_buffermode(DOUBLE_BUFFER);

} /* End procedure bif_defaultconfig */

/*----------------------------------------------------------------------*\
| Procedure	:	int bif_colormodel(BIF_INT)
|------------------------------------------------------------------------|
| Description	:	set the BIF color_model configuration value
|		color_model is one of: < RGB | CIE | HSV | HLS >
|
|		This call is a configuration parameter setting routine.
|		The system dependant calls are made in bif_openwk();
|------------------------------------------------------------------------|
| Return	:	Error Code
\*----------------------------------------------------------------------*/
int bif_colormodel(color_model)
BIF_INT color_model;
{

#ifdef TEST_PRINT
	printf("COLOR_MODEL : Set to %d\n",color_model);
#endif /* TEST_PRINT */

	wk_info.color_model = REMAP_CMODEL(color_model);
#ifdef USING_PHIGS
	/* this should work correctly, but on our box does not seem to
	work. */
/* PEX Note:
        This does work, the only problem exists when the first default
        configuration is defined, PHIGS is not open. The correct thing
        to do is test if the system is initialized before makeing calls
        to PHIGS functions.
#ifdef EXCEPTION_HANDLER
	PLB_EXCEPTION(BIF_EX_NOCOLORMODL);
#endif
*/

	if (wk_info.phigs_open) 
	    pset_colr_model((Pint)bench_setup.workid,
			    (Pint)wk_info.color_model );
#endif
} /* End procedure bif_colormodel */

/*----------------------------------------------------------------------*\
| Procedure	:	int bif_colormode(BIF_INT)
|------------------------------------------------------------------------|
| Description	:	set the BIF color_mode configuration value
|		color_mode is one of: < PSEUDO_COLOR | TRUE_COLOR >
|
|		This call is a configuration parameter setting routine.
|		The system dependant calls are made in bif_openwk();
|------------------------------------------------------------------------|
| Return	:	Error Code
\*----------------------------------------------------------------------*/
int bif_colormode(color_mode)
BIF_INT color_mode;
{

#ifdef TEST_PRINT
	printf("COLOR_MODE : Set to %d\n",color_mode);
#endif /* TEST_PRINT */
	wk_info.color_mode = color_mode;
} /* End procedure bif_colormode */

/*----------------------------------------------------------------------*\
| Procedure	:	int bif_buffermode(BIF_INT)
|------------------------------------------------------------------------|
| Description	:	set the BIF buffer_mode configuration value
|		buffer_mode is one of: < SINGLE_BUFFER | DOUBLE_BUFFER >
|
|		This call is a configuration parameter setting routine.
|		The system dependant calls are made in bif_openwk();
|------------------------------------------------------------------------|
| Return	:	Error Code
\*----------------------------------------------------------------------*/
int bif_buffermode(buffer_mode)
BIF_INT buffer_mode;
{

#ifdef TEST_PRINT
	printf("BUFFER_MODE : Set to %d\n",buffer_mode);
#endif /* TEST_PRINT */
	wk_info.buffer_mode = buffer_mode;
} /* End procedure bif_buffermode */

int bif_window(x, y)
BIF_INT x, y;
{
#ifdef TEST_PRINT
    printf("WINDOW_SIZE : Set to %d %d\n",x,y);
#endif /* TEST_PRINT */
    	wk_info.x = x;
    	wk_info.y = y;
}
