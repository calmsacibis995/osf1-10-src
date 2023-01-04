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
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define STRING	"Hello, world"
#define BORDER	1
#define FONT	"fixed"
#define	ARG_FONT		"font"
#define	ARG_BORDER_COLOR	"bordercolor"
#define	ARG_FOREGROUND		"foreground"
#define	ARG_BACKGROUND		"background"
#define ARG_BORDER		"border"
#define	ARG_GEOMETRY		"geometry"
#define DEFAULT_GEOMETRY	""

/*
 * This structure forms the WM_HINTS property of the window,
 * letting the window manager know how to handle this window.
 * See Section 9.1 of the Xlib manual.
 */
XWMHints	xwmh = {
    (InputHint|StateHint),	/* flags */
    False,			/* input */
    NormalState,		/* initial_state */
    0,				/* icon pixmap */
    0,				/* icon window */
    0, 0,			/* icon location */
    0,				/* icon mask */
    0,				/* Window group */
};

main(argc,argv)
    int argc;
    char **argv;
{
    Display    *dpy;		/* X server connection */
    Window      win;		/* Window ID */
    GC          gc;		/* GC to draw with */
    char       *fontName;	/* Name of font for string */
    XFontStruct *fontstruct;	/* Font descriptor */
    unsigned long ftw, fth, pad;/* Font size parameters */
    unsigned long fg, bg, bd;	/* Pixel values */
    unsigned long bw;		/* Border width */
    char       *tempstr;	/* Temporary string */
    XColor      color;		/* Temporary color */
    Colormap    cmap;		/* Color map to use */
    XGCValues   gcv;		/* Struct for creating GC */
    XEvent      event;		/* Event received */
    XSizeHints  xsh;		/* Size hints for window manager */
    char       *geomSpec;	/* Window geometry string */
    XSetWindowAttributes xswa;	/* Temporary Set Window Attribute struct */

    /*
     * Open the display using the $DISPLAY environment variable to locate
     * the X server.  See Section 2.1.
     */
    if ((dpy = XOpenDisplay(NULL)) == NULL) {
	fprintf(stderr, "%s: can't open %s\n", argv[0], XDisplayName(NULL));
	exit(1);
    }

    /*
     * Load the font to use.  See Sections 10.2 & 6.5.1
     */
    if ((fontName = XGetDefault(dpy, argv[0], ARG_FONT)) == NULL) {
	fontName = FONT;
    }
    if ((fontstruct = XLoadQueryFont(dpy, fontName)) == NULL) {
	fprintf(stderr, "%s: display %s doesn't know font %s\n",
		argv[0], DisplayString(dpy), fontName);
	exit(1);
    }
    fth = fontstruct->max_bounds.ascent + fontstruct->max_bounds.descent;
    ftw = fontstruct->max_bounds.width;

    /*
     * Select colors for the border,  the window background,  and the
     * foreground.  We use the default colormap to allocate the colors in.
     * See Sections 2.2.1, 5.1.2, & 10.4.
     */
    cmap = DefaultColormap(dpy, DefaultScreen(dpy));
    if ((tempstr = XGetDefault(dpy, argv[0], ARG_BORDER_COLOR)) == NULL ||
	XParseColor(dpy, cmap, tempstr, &color) == 0 ||
	XAllocColor(dpy, cmap, &color) == 0) {
	bd = WhitePixel(dpy, DefaultScreen(dpy));
    }
    else {
	bd = color.pixel;
    }
    if ((tempstr = XGetDefault(dpy, argv[0], ARG_BACKGROUND)) == NULL ||
	XParseColor(dpy, cmap, tempstr, &color) == 0 ||
	XAllocColor(dpy, cmap, &color) == 0) {
	bg = BlackPixel(dpy, DefaultScreen(dpy));
    }
    else {
	bg = color.pixel;
    }
    if ((tempstr = XGetDefault(dpy, argv[0], ARG_FOREGROUND)) == NULL ||
	XParseColor(dpy, cmap, tempstr, &color) == 0 ||
	XAllocColor(dpy, cmap, &color) == 0) {
	fg = WhitePixel(dpy, DefaultScreen(dpy));
    }
    else {
	fg = color.pixel;
    }
    /*
     * Set the border width of the window,  and the gap between the text
     * and the edge of the window, "pad".
     */
    pad = BORDER;
    if ((tempstr = XGetDefault(dpy, argv[0], ARG_BORDER)) == NULL)
	bw = 1;
    else
	bw = atoi(tempstr);

    /*
     * Deal with providing the window with an initial position & size.
     * Fill out the XSizeHints struct to inform the window manager. See
     * Sections 9.1.6 & 10.3.
     */
    geomSpec = XGetDefault(dpy, argv[0], ARG_GEOMETRY);
    if (geomSpec == NULL) {
	/*
	 * The defaults database doesn't contain a specification of the
	 * initial size & position - fit the window to the text and locate
	 * it in the center of the screen.
	 */
	xsh.flags = (PPosition | PSize);
	xsh.height = fth + pad * 2;
	xsh.width = XTextWidth(fontstruct, STRING, strlen(STRING)) + pad * 2;
	xsh.x = (DisplayWidth(dpy, DefaultScreen(dpy)) - xsh.width) / 2;
	xsh.y = (DisplayHeight(dpy, DefaultScreen(dpy)) - xsh.height) / 2;
    }
    else {
	int         bitmask;

	bzero(&xsh, sizeof(xsh));
	bitmask = XGeometry(dpy, DefaultScreen(dpy), geomSpec, DEFAULT_GEOMETRY,
			    bw, ftw, fth, pad, pad, &(xsh.x), &(xsh.y),
			    &(xsh.width), &(xsh.height));
	if (bitmask & (XValue | YValue)) {
	    xsh.flags |= USPosition;
	}
	if (bitmask & (WidthValue | HeightValue)) {
	    xsh.flags |= USSize;
	}
    }

    /*
     * Create the Window with the information in the XSizeHints, the
     * border width,  and the border & background pixels. See Section 3.3.
     */
    win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
			      xsh.x, xsh.y, xsh.width, xsh.height,
			      bw, bd, bg);

    /*
     * Set the standard properties for the window managers. See Section
     * 9.1.
     */
    XSetStandardProperties(dpy, win, STRING, STRING, None, argv, argc, &xsh);
    XSetWMHints(dpy, win, &xwmh);

    /*
     * Ensure that the window's colormap field points to the default
     * colormap,  so that the window manager knows the correct colormap to
     * use for the window.  See Section 3.2.9. Also,  set the window's Bit
     * Gravity to reduce Expose events.
     */
    xswa.colormap = DefaultColormap(dpy, DefaultScreen(dpy));
    xswa.bit_gravity = CenterGravity;
    XChangeWindowAttributes(dpy, win, (CWColormap | CWBitGravity), &xswa);

    /*
     * Create the GC for writing the text.  See Section 5.3.
     */
    gcv.font = fontstruct->fid;
    gcv.foreground = fg;
    gcv.background = bg;
    gc = XCreateGC(dpy, win, (GCFont | GCForeground | GCBackground), &gcv);

    /*
     * Specify the event types we're interested in - only Exposures.  See
     * Sections 8.5 & 8.4.5.1
     */
    XSelectInput(dpy, win, ExposureMask);

    /*
     * Map the window to make it visible.  See Section 3.5.
     */
    XMapWindow(dpy, win);

    /*
     * Loop forever,  examining each event.
     */
    while (1) {
	/*
	 * Get the next event
	 */
	XNextEvent(dpy, &event);

	/*
	 * On the last of each group of Expose events,  repaint the entire
	 * window.  See Section 8.4.5.1.
	 */
	if (event.type == Expose && event.xexpose.count == 0) {
	    XWindowAttributes xwa;	/* Temp Get Window Attribute struct */
	    int         x, y;

	    /*
	     * Remove any other pending Expose events from the queue to
	     * avoid multiple repaints. See Section 8.7.
	     */
	    while (XCheckTypedEvent(dpy, Expose, &event));

	    /*
	     * Find out how big the window is now,  so that we can center
	     * the text in it.
	     */
	    if (XGetWindowAttributes(dpy, win, &xwa) == 0)
		break;
	    x = (xwa.width - XTextWidth(fontstruct, STRING, strlen(STRING))) / 2;
	    y = (xwa.height + fontstruct->max_bounds.ascent
		 - fontstruct->max_bounds.descent) / 2;

	    /*
	     * Fill the window with the background color,  and then paint
	     * the centered string.
	     */
	    XClearWindow(dpy, win);
	    XDrawString(dpy, win, gc, x, y, STRING, strlen(STRING));
	}
    }

    exit(1);
}

