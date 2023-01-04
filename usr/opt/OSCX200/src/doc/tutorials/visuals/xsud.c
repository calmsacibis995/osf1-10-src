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
#include	<X11/Xlib.h>
#include	<X11/Xutil.h>
#include	<stdio.h>
#include	<sys/file.h>

Display			*dpy;		/*  X server we're talking to */
Window			win;		/*  Window to paint the image in */
GC			gc;		/*  GC to use for painting */
Visual			*visual;	/*  Visual to use for the window */
Colormap		cmap;		/*  Colormap to use for painting */
XEvent			ev;		/*  Event received from the server */
XImage			*image;		/*  To hold the image to be painted */
int			width, height;	/*  Size of the window */
int         	        best_size[6];	/*  Largest colormap per Visual class */
XVisualInfo             *best_vis[6];	/*  Best Visual per Visual class */
int                     best_class[] = { TrueColor, DirectColor, PseudoColor,
                                         StaticColor, GrayScale, StaticGray };

/*
 *  Return the most suitable Visual for representing numcolors colors
 */
XVisualInfo *
FindVisual(numcolors)
    int		numcolors;
{
    int			num_vis, i, big_map = 0;
    XVisualInfo 	vinfo_template, *vlist, *v, *big_vis = NULL;

    /*  Get descriptions of all the visuals */
    vlist = XGetVisualInfo(dpy, VisualNoMask, &vinfo_template, &num_vis);
    /* Scan the list examining the colormap size */
    for (v = vlist; v < vlist + num_vis; v++) {
        /*  Remember the biggest colormap among all classes */
	if (v->colormap_size > big_map) {
	    big_map = v->colormap_size;
	    big_vis = v;
	}
	/*  If the colormap is big enough,  remember it */
        if (v->colormap_size > numcolors) {
            if ((best_size[v->class] == 0)
              || (v->colormap_size < best_size[v->class])) {
	        /*  Smallest so far that'll do */
                best_size[v->class] = v->colormap_size;
                best_vis[v->class] = v;
            }
	}
    }
    /*  In decreasing order of usability,  look at each class */
    for (i = 0; i < 6; i++)
        if (best_size[best_class[i]] > numcolors)
            /*  This class can represent enough colors */
            return (best_vis[best_class[i]]);
    /*  Sigh!  We'll have to make do with a Visual that's too small */
    return (big_vis);
}

main(argc, argv)
int	argc;
char	**argv;
{
    int         numcolors, num_pxls, num_vis, x, y;
    unsigned short *rgbvalues;
    XVisualInfo *vis;
    XSetWindowAttributes values;
    XColor      color;


    /* Read the size information from stdin */
    fread((char *) &width, sizeof(int), 1, stdin);
    fread((char *) &height, sizeof(int), 1, stdin);
    fread((char *) &numcolors, sizeof(int), 1, stdin);
    /* Allocate space to hold the RGB data,  and read it in */
    num_pxls = width * height;
    rgbvalues = (unsigned short *) malloc(sizeof(unsigned short)
					  * 3 * num_pxls);
    fread((char *) rgbvalues, sizeof(unsigned short), 3 * num_pxls, stdin);
    /* Connect to the server */
    if ((dpy = XOpenDisplay(NULL)) == (Display *) 0) {
	fprintf(stderr, "can't open display\n");
	exit(1);
    }
    /* Find a suitable Visual for numcolors */
    vis = FindVisual(numcolors);
    /* Create a Colormap in the Visual we found */
    cmap = XCreateColormap(dpy, RootWindow(dpy, DefaultScreen(dpy)),
			   vis->visual, AllocNone);
    /* Create an image the right size */
    image = XCreateImage(dpy, vis->visual, vis->depth, ZPixmap, 0,
	      (unsigned long *) malloc(num_pxls * sizeof(unsigned long)),
			 width, height, 32, 0);
    /* For each pixel in the image */
    for (y = 0; y < height; y++)
	for (x = 0; x < width; x++) {
	    /* Fill out the RGB fields of the XColor struct */
	    color.red = *rgbvalues++;
	    color.green = *rgbvalues++;
	    color.blue = *rgbvalues++;
	    color.flags = DoRed | DoGreen | DoBlue;
	    /* Get the server to convert from RGB to pixel value */
	    if (!XAllocColor(dpy, cmap, &color)) {
		/* The colormap filled up - give up */
		fprintf(stderr, "Colormap full\n");
		exit(1);
	    }
	    /* Put the pixel value into the image */
	    (void) XPutPixel(image, x, y, color.pixel);
	}
    /* Create a suitable window using the Colormap, background White */
    values.colormap = cmap;
    /* get White from our colormap */
    XAllocNamedColor(dpy, cmap, "white", &color, &color);
    values.background_pixel = color.pixel;
    /* Listen for Expose,  Enter and Leave events */
    values.event_mask = ExposureMask | EnterWindowMask | LeaveWindowMask;
    win = XCreateWindow(dpy, RootWindow(dpy, DefaultScreen(dpy)),
			0, 0, width, height, 0, vis->depth, InputOutput,
		     vis->visual, CWColormap | CWEventMask | CWBackPixel,
			&values);
    /* Create a GC to use for repainting the window */
    gc = XCreateGC(dpy, win, 0, NULL);
    /* Map the window,  wait for the Expose events,  and paint */
    XMapWindow(dpy, win);
    while (True) {
	XExposeEvent *e;

	XNextEvent(dpy, &ev);
	switch (ev.type) {
	    case Expose:
		e = (XExposeEvent *) & ev;
		/* Copy the image to the exposed part of the window */
		XPutImage(dpy, win, gc, image, e->x, e->y, e->x, e->y,
		      (e->x + e->width > width ? width - e->x : e->width),
		      (e->y + e->height > height ? height - e->y : e->height));
		break;
	    case EnterNotify:
		/* Mouse is in the window,  install its Colormap */
		XInstallColormap(dpy, cmap);
		break;
	    case LeaveNotify:
		/* Mouse has left the window,  uninstall its Colormap */
		XUninstallColormap(dpy, cmap);
		break;
	}
    }
}
