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
/* $XConsortium: PropAlloc.c,v 1.5 91/02/01 16:33:26 gildea Exp $ */
/* Copyright 1989 Massachusetts Institute of Technology */

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#include "Xlibint.h"
#include "Xutil.h"
#include <stdio.h>


/*
 * Routines for allocating space for structures that are expected to get 
 * longer at some point.
 */

XSizeHints *XAllocSizeHints ()
{
    return ((XSizeHints *) Xcalloc (1, (unsigned) sizeof (XSizeHints)));
}


XStandardColormap *XAllocStandardColormap ()
{
    return ((XStandardColormap *)
	    Xcalloc (1, (unsigned) sizeof (XStandardColormap)));
}


XWMHints *XAllocWMHints ()
{
    return ((XWMHints *) Xcalloc (1, (unsigned) sizeof (XWMHints)));
}


XClassHint *XAllocClassHint ()
{
    register XClassHint *h;

    if (h = (XClassHint *) Xcalloc (1, (unsigned) sizeof (XClassHint))) 
      h->res_name = h->res_class = NULL;

    return h;
}


XIconSize *XAllocIconSize ()
{
    return ((XIconSize *) Xcalloc (1, (unsigned) sizeof (XIconSize)));
}


