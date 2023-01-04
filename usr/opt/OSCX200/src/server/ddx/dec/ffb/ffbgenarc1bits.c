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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: ffbgenarc1bits.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:10:13 $";
#endif
/*
 */
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "ffbparams.h"

Pixmap pix;
Display *display;
GC gc, bggc;
XGCValues gcv;

/*
 * Open_Display: Routine to open a display with correct error handling.
 */

Display *Open_Display(display_name)
    char *display_name;
{
    Display *d;

    d = XOpenDisplay(display_name);
    if (d == (Display *)NULL) {
	fprintf (stderr, "unable to open display '%s'\n",
		 XDisplayName (display_name));
	exit (1);
	/* doesn't return */
    }

    return(d);
}

main() {
    int		    i, j;
    XImage	    *image;

    display = Open_Display(getenv("DISPLAY"));
    (void)XSynchronize(display, 1);
    pix = XCreatePixmap(display, DefaultRootWindow(display),
	FFBBUSBITS, FFBBUSBITS, 1);
    gcv.foreground = 1;
    gcv.background = 0;
    gcv.graphics_exposures = 0;
    gcv.line_width = 1;
    gc = XCreateGC(display, pix, 
	GCForeground | GCBackground | GCGraphicsExposures | GCLineWidth, &gcv);
    gcv.foreground = 0;
    bggc = XCreateGC(display, pix, 
	GCForeground | GCBackground | GCGraphicsExposures | GCLineWidth, &gcv);

    
    for (i = 0; i < FFBBUSBITS; i++) {
	XFillRectangle(display, pix, bggc, 0, 0, FFBBUSBITS, FFBBUSBITS);
	XDrawArc(display, pix, gc, 0, 0, i, i, 0, 64*360);
	image = XGetImage(display, pix, 0, 0, FFBBUSBITS, FFBBUSBITS, 1,
		    ZPixmap);
	printf("static CommandWord width1Circle%d[%d] = {", i, i/2+1);
	for (j = 0; j <= i/2; j++) {
	    if ((j&3) == 0) printf("\n");
	    printf("    0x%08lx", ((unsigned *)image->data)[j]);
	    if (j == i/2) printf("\n"); else printf(",");
	}
	printf("};\n\n");
    }
    printf("\n");
    printf("static CommandWord *width1Circles[FFBBUSBITS] = {");
    for (i = 0; i < FFBBUSBITS; i++) {
	if ((i&3) == 0) printf("\n");
	printf("    width1Circle%d", i);
	if (i == FFBBUSBITS-1) printf("\n"); else printf(",");
    }
    printf("};\n\n");
}

/*
 * HISTORY
 */
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/ffbgenarc1bits.c,v 1.1.2.2 1993/11/19 21:10:13 Robert_Lembree Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
