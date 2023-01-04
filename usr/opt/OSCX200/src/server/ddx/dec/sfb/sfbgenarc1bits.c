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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbgenarc1bits.c,v 1.1.3.2 92/01/06 15:36:16 David_Coleman Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "sfbparams.h"

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

    display = Open_Display(":0");
    (void)XSynchronize(display, 1);
    pix = XCreatePixmap(display, DefaultRootWindow(display),
	SFBBUSBITS, SFBBUSBITS, 1);
    gcv.foreground = 1;
    gcv.background = 0;
    gcv.graphics_exposures = 0;
    gcv.line_width = 1;
    gc = XCreateGC(display, pix, 
	GCForeground | GCBackground | GCGraphicsExposures | GCLineWidth, &gcv);
    gcv.foreground = 0;
    bggc = XCreateGC(display, pix, 
	GCForeground | GCBackground | GCGraphicsExposures | GCLineWidth, &gcv);

    
    for (i = 0; i < SFBBUSBITS; i++) {
	XFillRectangle(display, pix, bggc, 0, 0, SFBBUSBITS, SFBBUSBITS);
	XDrawArc(display, pix, gc, 0, 0, i, i, 0, 64*360);
	image = XGetImage(display, pix, 0, 0, SFBBUSBITS, SFBBUSBITS, 1,
		    ZPixmap);
	printf("CommandWord width1Circle%d[%d] = {", i, i/2+1);
	for (j = 0; j <= i/2; j++) {
	    if ((j&3) == 0) printf("\n");
	    printf("    0x%08lx", ((unsigned *)image->data)[j]);
	    if (j == i/2) printf("\n"); else printf(",");
	}
	printf("};\n\n");
    }
    printf("\n");
    printf("CommandWord *width1Circles[SFBBUSBITS] = {");
    for (i = 0; i < SFBBUSBITS; i++) {
	if ((i&3) == 0) printf("\n");
	printf("    width1Circle%d", i);
	if (i == SFBBUSBITS-1) printf("\n"); else printf(",");
    }
    printf("};\n\n");
}
