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
/*
* $XConsortium: tsourceP.h,v 1.1 89/09/01 17:36:46 kit Exp $
*/


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/* 
 * tocSourceP.h - Private definitions for tocSource widget
 * 
 */

#ifndef _XawtocSourceP_h
#define _XawtocSourceP_h

/***********************************************************************
 *
 * tocSource Widget Private Data
 *
 ***********************************************************************/

#include <X11/ObjectP.h>
#include <X11/Xaw/TextSrcP.h>
#include "tsource.h"
#include "tocintrnl.h"

/************************************************************
 *
 * New fields for the TocSource widget class record.
 *
 ************************************************************/

typedef struct _TocSourceClassPart {
  char foo;			/* keep compiler happy. */
} TocSourceClassPart;

/* Full class record declaration */
typedef struct _TocSourceClassRec {
    ObjectClassPart     object_class;
    TextSrcClassPart	text_source_class;
    TocSourceClassPart	toc_source_class;
} TocSourceClassRec;

extern TocSourceClassRec tocSourceClassRec;

/* New fields for the TextSrc widget record */
typedef struct {
    /* resources */
  Toc toc;
} TocSourcePart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _TocSourceRec {
  ObjectPart    object;
  TextSrcPart	text_source;
  TocSourcePart	toc_source;
} TocSourceRec;

#endif /* _XawTextSrcP_h */
