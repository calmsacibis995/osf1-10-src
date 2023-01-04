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
/* $XConsortium: XGeom.c,v 1.8 91/01/06 11:45:51 rws Exp $ */
/* Copyright Massachusetts Institute of Technology 1985 */

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

/*
 * This routine given a user supplied positional argument and a default
 * argument (fully qualified) will return the position the window should take
 * returns 0 if there was some problem, else the position bitmask.
 */

#if NeedFunctionPrototypes
int XGeometry (
     Display *dpy,			/* user's display connection */
     int screen,			/* screen on which to do computation */
     _Xconst char *pos,			/* user provided geometry spec */
     _Xconst char *def,			/* default geometry spec for window */
     unsigned int bwidth,		/* border width */
     unsigned int fwidth,		/* size of position units */
     unsigned int fheight,
     int xadd,				/* any additional interior space */
     int yadd,
     register int *x,			/* always set on successful RETURN */
     register int *y,			/* always set on successful RETURN */
     register int *width,		/* always set on successful RETURN */
     register int *height)		/* always set on successful RETURN */
#else
int XGeometry (dpy, screen, pos, def, bwidth, fwidth, fheight, xadd, yadd, x, y, width, height)
     Display *dpy;			/* user's display connection */
     int screen;			/* screen on which to do computation */
     char *pos;				/* user provided geometry spec */
     char *def;				/* default geometry spec for window */
     unsigned int bwidth;		/* border width */
     unsigned int fwidth, fheight;	/* size of position units */
     int xadd, yadd;			/* any additional interior space */
     register int *x, *y, *width, *height;/* always set on successful RETURN */
#endif
{
	int px, py;			/* returned values from parse */
	unsigned int pwidth, pheight;	/* returned values from parse */
	int dx, dy;			/* default values from parse */
	unsigned int dwidth, dheight;	/* default values from parse */
	int pmask, dmask;		/* values back from parse */

	pmask = XParseGeometry(pos, &px, &py, &pwidth, &pheight);
	dmask = XParseGeometry(def, &dx, &dy, &dwidth, &dheight);

	/* set default values */
	*x = (dmask & XNegative) ? 
	    DisplayWidth(dpy, screen)  + dx - dwidth * fwidth - 
	        2 * bwidth - xadd : dx;
	*y = (dmask & YNegative) ? 
	    DisplayHeight(dpy, screen) + dy - dheight * fheight - 
	        2 * bwidth - yadd : dy;
	*width  = dwidth;
	*height = dheight;

	if (pmask & WidthValue)  *width  = pwidth;
	if (pmask & HeightValue) *height = pheight;

	if (pmask & XValue)
	    *x = (pmask & XNegative) ?
	      DisplayWidth(dpy, screen) + px - *width * fwidth - 
		  2 * bwidth - xadd : px;
	if (pmask & YValue)
	    *y = (pmask & YNegative) ?
	      DisplayHeight(dpy, screen) + py - *height * fheight - 
		  2 * bwidth - yadd : py;
	return (pmask);
}
