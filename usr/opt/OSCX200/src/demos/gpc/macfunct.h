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
/* $XConsortium: macfunct.h,v 5.2 91/02/16 10:07:41 rws Exp $ */

/*
 */
/***********************************************************
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

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

/*
 * Copyright (c) 1989, 1990, 1991 by M.I.T. and Sun Microsystems, Inc.
 */

/*--------------------------------------------------------------------*\
|  Copyright (C) 1989, 1990, 1991, National Computer Graphics Association
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
| Author        :	John M. Zulauf
|
| File          :	macfunct.h
| Date          :	Sun Jun 25 22:41:36 PDT 1989
| Project       :	PLB
| Description   :	Macro-Functions... macros the generate
|			functions.  For those who don't like to
|			type, or yank-put (ad infinitum).
| Status        :	Version 1.0
|
| Revisions     :
|
|       2/90            MFC Tektronix, Inc.: PEX-SI API implementation.
|
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Table of Contents
|
|	define MF_TRUE_COLOR(MV_routine,MV_token,MV_handle,MV_phigs)
|	define MF_MAP_INDEX(MV_routine,MV_token,MV_handle,MV_phigs)
|	define MF_BIF_SIZE(MV_routine,MV_token,MV_handle,MV_phigs)
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Use a Macro to define a whole class of function:
|
|	the bif_<prim>color functions.
\*--------------------------------------------------------------------*/
#define MF_TRUE_COLOR(MV_routine,MV_token,MV_handle,MV_phigs) \
	/*----------------------------------------------------------*\
	| Procedure: int MV_routine(c1, c2, c3)			     \
	|----------------------------------------------------------- \
	| Description: Receive a MV_TOKEN entity from the parser     \
	|----------------------------------------------------------- \
	| Return: Error Code (Not Implemented)			     \
	\*---------------------------------------------------------*/\
	int MV_routine(c1, c2,c3)				     \
	BIF_REAL c1, c2,c3;					     \
	{							     \
		static int entSize        = sizeof(BIF_True_color)   \
		                            + sizeof(Pgcolr);        \
								     \
		/*--------------------------------------------------*\
		|	Since true color entities have the same	     \
		|	structure we can use a common routine.	     \
		\*-------------------------------------------------*/\
		bif_truecolor((float)c1,(float)c2,(float)c3,	     \
			entSize, (int)MV_token, MV_handle, MV_phigs);\
								     \
	} /* End MV_routine() */


/*--------------------------------------------------------------------*\
|	Use a Macro to define a whole class of function:
|
|	the bif_<prim>colorindex functions.
\*--------------------------------------------------------------------*/

#define MF_MAP_INDEX(MV_routine,MV_token,MV_handle,MV_phigs)	     \
	/*----------------------------------------------------------*\
	| Procedure: int MV_routine(index)			     \
	|----------------------------------------------------------- \
	| Description: Receive a MV_token entity from the parser     \
	|----------------------------------------------------------- \
	| Return: Error Code (Not Implemented)			     \
	\*---------------------------------------------------------*/\
	int MV_routine(index)					     \
	BIF_INT index;						     \
	{							     \
		static int entSize        = sizeof(BIF_Index);   \
								     \
		/*--------------------------------------------------*\
		|	Since true color entities have the same	     \
		|	structure we can use a common routine.	     \
		\*-------------------------------------------------*/\
		bif_colorindex((int)index, entSize,		     \
			(int)MV_token, MV_handle, MV_phigs);	     \
								     \
	} /* End MV_routine() */


#define MF_BIF_SIZE(MV_routine,MV_token,MV_handle,MV_phigs) \
	/*----------------------------------------------------------*\
	| Procedure: int MV_routine(size)			     \
	|----------------------------------------------------------- \
	| Description: Receive a MV_token entity from the parser     \
	| 		Many _size and _scalefactor routines	     \
	|		use this as a shorthand form.		     \
	|----------------------------------------------------------- \
	| Return: Error Code (Not Implemented)			     \
	\*---------------------------------------------------------*/\
	int MV_routine(size)					     \
	BIF_REAL size;						     \
	{							     \
		static int entSize = sizeof(BIF_Size);		     \
		bif_size((float)size,entSize,MV_token,		     \
			MV_handle,MV_phigs);			     \
								     \
	} /* End procedure MV_routine */

