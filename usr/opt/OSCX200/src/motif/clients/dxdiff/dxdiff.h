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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/dxdiff.h,v 1.1.2.3 93/01/05 17:26:12 Don_Haney Exp $ */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *	dxdiff
 *
 *	dxdiff.h - application header file for dxdiff
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 3rd March 1988
 *
 *
 *
 *	Modification History
 *	------------ -------
 *	
 */

#ifndef	DXDIFF_H
#define	DXDIFF_H

#define	DXDIFFNAME	"dxdiff"	/* application name */
#define	DXDIFFCLASS	"DxDiff"	/* application class */

#define	DXDIFFSHELLVARL	"$DxDiffLeftFile"	/* MUST have $ at start */
#define	DXDIFFSHELLVARR	"$DxDiffRightFile"	/* ditto */

/* note that any shell variable specified MUST be prefixed by a $ */

typedef void (*VoidProc)();

/********************************
 *
 *      EnvVariables
 *
 ********************************/

typedef	struct	_envvariables	{
		char	*shellvariable,
			*value;
} EnvVariables;

#ifdef HYPERHELP
#include        <DXm/bkr_api.h>
#define         dxdiff_help  "dxdiff"
#endif

#endif	DXDIFF_H /* do not include anything after this line */

/********************************
 * 
 * application resources
 *
 ********************************/

typedef struct _application_options {
    Boolean	slaveverticalscrolling;
    Boolean	slavehorizontalscrolling;

    Boolean	displaylinenumbers;
    Boolean	drawdiffsaslines;

    Boolean	Debug;
    XColor	linenumberforeground;

} *application_options_ptr;

extern struct _application_options app_options;
