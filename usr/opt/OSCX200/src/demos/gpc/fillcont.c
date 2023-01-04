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
/* $XConsortium: fillcont.c,v 5.2 91/04/03 09:40:25 rws Exp $ */
/***********************************************************
Copyright(c) 1989,1990, 1991 by Sun Microsystems, Inc. and the X Consortium at M.I.T.

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
|
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
| Author        :	jmz / SimGraphics Engineering Corportation
|
| File          :	fillcont.c
| Date          :	Thu Feb  8 15:13:51 PST 1990
| Project       :	PLB
| Description	:	Contour buffer -to- array  copy routines
| Status	:	Version 1.0
|
| Revisions     :	
|
|      12/90            MFC Tektronix, Inc.: PEX-SI PEX5R1 Release.
|
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Table of Contents
|
|	int fillVecSet(int, Real_int_union *, char *, int, vector **)
|		:	Assign the malloc'd space to the array and
|	int fillIntSet(int, Real_int_union *, char *, int, int **
|		:	Assign the malloc'd space to the array and
|
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Include files
\*--------------------------------------------------------------------*/
#include <stdio.h>
#include <X11/Xosdefs.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
char *malloc();
#endif
#if defined(macII) && !defined(__STDC__)  /* stdlib.h fails to define these */
char *malloc();
#endif /* macII */
#include "bifbuild.h"
#include "biftypes.h"
#include "bifparse.h"
#include "new_ents.h"
#include "doentity.h"
#include "db_tools.h"
#include "bifmacro.h"
#include "ph_map.h"
#include "globals.h"

/*--------------------------------------------------------------------*\
| Procedure     :	int fillVecSet(int, Real_int_union *,
|			char *, int, int)
|---------------------------------------------------------------------
| Description   :	Assign the malloc'd space to the array and
|			fill it.  If the supplied contour list is too
|			short, the final entry is duplicated to the end
|			of the array.
|---------------------------------------------------------------------
| Return        :	The number of bytes used by the filled array.
\*--------------------------------------------------------------------*/
void fillVecSet(numConts, contList, ptr, reqSize, adSize)
int		numConts;
Real_int_union	*contList;
char		*ptr;
int		reqSize;
int		adSize;

{
	int i, j, actSize, contSize;
	Pvec3 *vecs, *dupeMe;

	/* Establish the base of the array */
	vecs = (Pvec3 *)ptr;

	/* Fill It */
	actSize = 0;
	if ( numConts  > 0 )
	{
		/* Copy the data */
		for ( i = 0; i < numConts; i++ )
		{
			/* The contour size list */
			contSize = (contList++)->Int;
			actSize += contSize;

			/* The vector arrays */
			for ( j = 0; j < contSize ; j++ )
			{
				vecs->delta_x = (contList++)->Float;
				vecs->delta_y = (contList++)->Float;
				vecs->delta_z = (contList++)->Float;
				vecs = (Pvec3 *)(((char *)vecs) + adSize);
			}
		}

		/* Dupe the last entry ( if needed ) */
		dupeMe = (Pvec3 *)(((char *)vecs) - adSize);
		for ( i = actSize ; i < reqSize ; i++ )
		{
			vecs->delta_x = dupeMe->delta_x;
			vecs->delta_y = dupeMe->delta_y;
			vecs->delta_z = dupeMe->delta_z;
			vecs = (Pvec3 *)(((char *)vecs) + adSize);
		}
	}
	else
	{
	/* Default Empty List has one zero entry */
	/* To possible reference from bombing    */
	    vecs->delta_x = 0.0;
	    vecs->delta_y = 0.0;
	    vecs->delta_z = 0.0;
	}

} /* End fillVecSet() */

/*--------------------------------------------------------------------*\
| Procedure     :	int fillIntSet(int, Real_int_union *,
|			char *, int, int, int)
|---------------------------------------------------------------------
| Description   :	Assign the malloc'd space to the array and
|			fill it.  If the supplied contour list is too
|			short, the final entry is duplicated to the end
|			of the array.
|---------------------------------------------------------------------
| Return        :	The number of bytes used by the filled array.
\*--------------------------------------------------------------------*/
int fillIntSet(numConts, contList, ptr, reqSize, adSize, colorInd)
int		numConts;
Real_int_union	*contList;
char		*ptr;
int		reqSize;
int		adSize;
int		colorInd;

{
	int i, j, actSize, contSize;
	int *vals, *dupeMe;

	/* Establish the base of the array */
	vals = (int *)ptr;

	/* Fill It */
	actSize = 0;
	if ( numConts  > 0 )
	{
	    /* Copy the data */
	    for ( i = 0 ; i < numConts ; i++ )
	    {	
		/* The contour size list */
		contSize = (contList++)->Int;
		actSize += contSize;

#ifdef EXTERNALNOTE
		/* index is incremented by one so that color '0' does
		   not step on the default BG color. */
#endif
		for ( j = 0; j < contSize ; j++ )
		{
		    *vals = (contList++)->Int + colorInd;
		    vals = (int *)(((char *)vals) + adSize);
		}
	    }

	    /* Dupe the last entry ( if needed ) */
	    dupeMe = (int *)(((char *)vals) - adSize);
	    for ( i = actSize ; i < reqSize ; i++ )
	    {
		*vals = *dupeMe;
		vals = (int *)(((char *)vals) + adSize);
	    }
	}
	else
	{
	/* Default Empty List has one zero entry */
	/* To keep possible reference from bombing    */
		*vals = 0;
	}

} /* End fillIntSet() */

