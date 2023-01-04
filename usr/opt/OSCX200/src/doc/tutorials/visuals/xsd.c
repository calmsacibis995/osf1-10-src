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
#include <X11/Xlib.h>
#include <stdio.h>
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))

Display	*dpy;		/* X server we're talking to */
int dumpw, dumph;	/* Size of the "specified rectangle" */

/*
 *  There will be one of these for each Colormap we find
 */
struct cmp {
    struct cmp		*next;		/* Link in chain */
    Colormap		cmap;		/* The Colormap ID */
    unsigned long	allocated, used;/* Size & usage of inUse */
    XColor		*inUse;		/* Array of in-use colors */
};
struct cmp *inUse;	/* The list of Colormaps we found */
int numColor = 0;	/* Upper bound on number of distinct RGB values */

/*
 *  There will be one of these for each pixel in the rectangle
 */
struct pxl {
    struct cmp		*pCmap;		/* The colormap for this pixel */
    unsigned long	pixel;		/* The pixel value at this pixel */
};
struct pxl *Map;	/* The map representing the rectangle */
#define FindPixel(x,y) ((Map+((y)*dumpw))+(x))

/*
 *  Find (and create if necessary) a struct cmp for this Colormap
 */
struct cmp *
FindColormap(cmap)
    Colormap	cmap;
{
    register struct cmp *pCmap;

    /*  If we've seen this Colormap before,  return its struct cmp */
    for (pCmap = inUse;  pCmap; pCmap = pCmap->next)
	if (cmap == pCmap->cmap)
	    return (pCmap);
    /* First time, so create a new struct cmp, link it and return it */
    pCmap = (struct cmp *) calloc(sizeof (struct cmp), 1);
    pCmap->next = inUse;
    pCmap->cmap  = cmap;
    inUse = pCmap;
    return (pCmap);
}

/*
 *  Record this pixel value as being in use in its Colormap
 */
void
RegisterPixel(pixel, pCmap)
    unsigned long	pixel;
    struct cmp		*pCmap;
{
    register unsigned long i = pCmap->used;

    /*  If the pixel value is already known,  do nothing */
    while (i)
	if (pixel == pCmap->inUse[--i].pixel)
	    return;
    /*  This is the first time we've seen this pixel value */
    if (pCmap->used >= pCmap->allocated) {
	/*  Need to expand or create the inUse array */
	pCmap->allocated = (pCmap->allocated * 2) + 10;
	pCmap->inUse = (XColor *)(pCmap->inUse ?
			realloc(pCmap->inUse,
				pCmap->allocated * sizeof (XColor)) :
			malloc(pCmap->allocated * sizeof (XColor)));
    }
    /*  Now we have space to store the XColor, use QueryColor to get RGB */
    pCmap->inUse[pCmap->used].pixel = pixel;
    XQueryColor(dpy, pCmap->cmap, &pCmap->inUse[pCmap->used]);
    numColor++;
    pCmap->used++;
}

/*
 *  This gets called once for each window we find as we walk down the tree
 */
DoWindow(w, xo, yo, x, y, wi, hi)
    Window	w;
    int		xo, yo;	/* Parent's origin in root space */
    int		x, y;	/* Top-left of rectangle in root space */
    int		wi, hi;	/* Size of rectangle */
{
    XWindowAttributes	xwa;	/* Place to return the window's attributes */
    XImage		*xim;	/* Image to store window's pixels */
    int			width, height, x1, y1, xi, yi;
    Window		root, parent, *children;
    int			nchild, n;
    struct cmp		*pCmap;

    /*  Get the attributes of this window,  and locate its struct cmp */
    if (!XGetWindowAttributes(dpy, w, &xwa) || xwa.map_state != IsViewable)
	return;
    pCmap = FindColormap(xwa.colormap);
    /* Compute top-left of image in root space */
    x1 = max(x, xwa.x+xo);
    y1 = max(y, xwa.y+yo);
    width = min(x+wi, xwa.x + xwa.width + 2*xwa.border_width + xo) - x1;
    height = min(y+hi, xwa.y + xwa.height + 2*xwa.border_width + yo) - y1;
    if (width <= 0 || height <= 0)
	return;
    /*  Use GetImage to get the pixel values for the rectangle */
    if (!(xim = XGetImage(dpy, w, x1-xwa.border_width-xwa.x-xo,
			  y1-xwa.border_width-xwa.y-yo, width, height,
			  (~0), ZPixmap)))
	return;
    /*  For each pixel in the returned image */
    for (yi = 0; yi < height; yi++)
	for (xi = 0; xi < width; xi++) {
	    register struct pxl *pPxl = FindPixel(xi+x1, yi+y1);

	    /*  Label the pixel in the map  with this window's Colormap */
	    pPxl->pCmap = pCmap;
	    /*  And with its pixel value */
	    pPxl->pixel = XGetPixel(xim, xi, yi);
	    RegisterPixel(pPxl->pixel, pCmap);
	}
    /*  Free the space for the image */
    XDestroyImage(xim);
    /*  Find the children of this window, in back-to-front order */
    if (XQueryTree(dpy, w, &root, &parent, &children, &nchild)) {
	for (n = 0; n < nchild; n++) {
	    /*  Process each of the child windows recursively */
	    DoWindow(children[n], xo + xwa.x + xwa.border_width,
		     yo + xwa.y + xwa.border_width, x1, y1, width, height);
	}
	/*  Free the list of children */
	if (nchild > 0)
	    XFree(children);
    }
    return;
}

/*
 *  Return the XColor structure for the pixel at [x,y] in the map
 */
XColor *
FindColor(x, y)
    int		x, y;
{
    struct pxl	*pPxl = FindPixel(x, y);	/* Find the struct pxl */
    struct cmp	*pCmp = pPxl->pCmap;		/* And the struct cmp */
    int		i;

    /*  Scan the in-use array for this colormap for the pixel value */
    for (i = 0; i < pCmp->used; i++)
	if (pPxl->pixel == pCmp->inUse[i].pixel)
	    return (&(pCmp->inUse[i]));
    return (NULL);
}

/*
 *  Write the representation of the rectangle to stdout
 */
DoOutput(x, y, w, h)
    int		x, y, w, h;
{
    int		xi, yi;

    /*  Write the width,  height,  and number of colors */
    fwrite(&w, sizeof (int), 1, stdout);
    fwrite(&h, sizeof (int), 1, stdout);
    fwrite(&numColor, sizeof (int), 1, stdout);
    /*  For each pixel in the image */
    for (yi = 0; yi < h; yi++)
	for (xi = 0; xi < w; xi++) {
	    XColor	*color = FindColor(x + xi, y + yi);

	    /*  Write the R, G & B values for this pixel */
	    fwrite(&(color->red), sizeof (unsigned short), 1, stdout);
	    fwrite(&(color->green), sizeof (unsigned short), 1, stdout);
	    fwrite(&(color->blue), sizeof (unsigned short), 1, stdout);
	}
}

main(argc, argv)
    int		argc;
    char	**argv;
{
    int		scrn;

    /*  Try to connect to the server */
    if ((dpy = XOpenDisplay(NULL)) == (Display *) 0) {
	fprintf(stderr, "Can't open display\n");
	exit(1);
    }
    /*  Dump the specified part of the default screen */
    scrn = DefaultScreen(dpy);
    if (argc > 1)
	dumpw = atoi(argv[1]);
    else
        dumpw = DisplayWidth(dpy, scrn);
    if (argc > 2)
	dumph = atoi(argv[2]);
    else
        dumph = DisplayHeight(dpy, scrn);
    /*  Create the map with one struct pxl per pixel */
    Map = (struct pxl *) calloc(sizeof (struct pxl), dumpw * dumph);
    /*  Grab the server so things don't change under our feet */
    XGrabServer(dpy);
    /*  Recursively build the map */
    DoWindow(RootWindow(dpy, scrn), 0, 0, 0, 0, dumpw, dumph);
    /*  Finished reading things from the server - let it go */
    XUngrabServer(dpy);
    /*  Write the RGB representation of the rectangle to stdout */
    DoOutput(0, 0, dumpw, dumph);
    exit(0);
}
