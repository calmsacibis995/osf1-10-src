/* #module xwdio.c "v1.0"
 *
 *  Copyright (c) Digital Equipment Corporation, 1990
 *  All Rights Reserved.  Unpublished rights reserved
 *  under the copyright laws of the United States.
 *  
 *  The software contained on this media is proprietary
 *  to and embodies the confidential technology of 
 *  Digital Equipment Corporation.  Possession, use,
 *  duplication or dissemination of the software and
 *  media is authorized only pursuant to a valid written
 *  license from Digital Equipment Corporation.
 *
 *  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 *  disclosure by the U.S. Government is subject to
 *  restrictions as set forth in Subparagraph (c)(1)(ii)
 *  of DFARS 252.227-7013, or in FAR 52.227-19, as
 *  applicable.
 *
 *
 * FACILITY:
 *	DECwindows Print Screen
 *
 * ABSTRACT:
 *	
 *
 * NOTES:
 *	
 *
 * REVISION HISTORY:
 */

/*
** Includes
*/
#include "iprdw.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <X11/Xmd.h>
#ifdef VMS
#include "xwdfile.h"
#else
#include <X11/XWDFile.h>
#endif
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
#include "prdw_entry.h"

#ifdef STANDALONE_XWD
char	filename_buffer[80];
char	display_name[2] = {'\000', '\000'};
char	name[] = "xwdump";

int main
#if _PRDW_PROTO_
(
    int argc,
    char** argv
)
#else
(argc, argv)
    int		argc;
    char	**argv;
#endif
{
    int			mode;
    int			screen;
    Display		*display;
    Window		window;
    XWindowAttributes	winattr;
    int			status;
    XImage		*ximage;
    XColor		*colors;
    int			numcolors;

    /* ask for record vs playback */

    printf("Record = 0, Playback = 1: ");
    scanf("%d",&mode);
    printf("Screen number: ");
    scanf("%d",&screen);
    printf("File name: ");
    scanf("%s",filename_buffer);

    if (mode == 0)
    {
	printf("Record screen %d in %s\n",screen,filename_buffer);

	/* capture screen */

	if ((display = XOpenDisplay(display_name)) == NULL )
	{
	    printf("Can't open display\n");
	    exit(0);
	}
	window = RootWindow (display, screen);
	XGetWindowAttributes (display, window, &winattr);

	ximage = XGetImage
	(
	    display, window,
	    0, 0,
	    winattr.width, winattr.height,
	    AllPlanes,ZPixmap
	);

	numcolors = getColors
	    (display, winattr.colormap, winattr.visual, &colors);

	/* call xwdWrite */

	status = xwdWrite
	(
	    filename_buffer,
	    ximage,
	    numcolors,
	    winattr.visual,
	    name,
	    &colors,
	    display,
	    screen
	);

	XtFree (colors);

	XDestroyImage (ximage);
	XCloseDisplay (display);
    }
    else
    {
	/* call xwdRead */

	printf("Playback screen %d from %s\n",screen,filename_buffer);
	/* display on screen */
    } /* if */
} /* main */
#endif /* STANDALONE_XWD */

int xwdWrite
#if _PRDW_PROTO_
(
    char		*xwd_filename,
    XImage		*ximage,
    int			colorct,
    Visual		*vis,
    char		*wname,
    XColor		**colors,
    Display		*display,
    int			screen
)
#else
(xwd_filename,ximage,colorct,vis,wname,colors,display,screen)
    char		*xwd_filename;
    XImage		*ximage;
    int			colorct;
    Visual		*vis;
    char		*wname;
    XColor		**colors;
    Display		*display;
    int			screen;
#endif
{
    FILE		*xwd_file;
    int			status;
    int			header_size;
    int			image_size;
    XWDFileHeader	header;
    int			format = ZPixmap;

    /* create file */

    xwd_file = fopen(xwd_filename,"w");

    header_size = sizeof(header) + strlen(wname) + 1;	/* incl wname's NUL */

    image_size = ximage->bytes_per_line * ximage->height;

    /* make header */

    header.header_size = (CARD32) header_size;
    header.file_version = (CARD32) XWD_FILE_VERSION;
    header.pixmap_format = (CARD32) format;
    header.pixmap_depth = (CARD32) ximage->depth;
    header.pixmap_width = (CARD32) ximage->width;
    header.pixmap_height = (CARD32) ximage->height;
    header.xoffset = (CARD32) ximage->xoffset;
    header.byte_order = (CARD32) ximage->byte_order;
    header.bitmap_unit = (CARD32) ximage->bitmap_unit;
    header.bitmap_bit_order = (CARD32) ximage->bitmap_bit_order;
    header.bitmap_pad = (CARD32) ximage->bitmap_pad;
    header.bits_per_pixel = (CARD32) ximage->bits_per_pixel;
    header.bytes_per_line = (CARD32) ximage->bytes_per_line;

    header.visual_class = (CARD32) vis->class;
    header.red_mask = (CARD32) vis->red_mask;
    header.green_mask = (CARD32) vis->green_mask;
    header.blue_mask = (CARD32) vis->blue_mask;
    header.bits_per_rgb = (CARD32) vis->bits_per_rgb;
    header.colormap_entries = (CARD32) vis->map_entries;

    header.ncolors = colorct;

    /*
    ** This should be from the window being captured.  At current this will
    ** always be RootWindow, but it could be expanded to capture a specific
    ** window.  In that case, we will need to make sure we distinguish the
    ** window attributes for the specific window.
    */
    header.window_width = (CARD32) DisplayWidth(display,screen);
    header.window_height = (CARD32) DisplayHeight(display,screen);
    header.window_x = 0;
    header.window_y = 0;
    header.window_bdrwidth = 0;

    /* swap bits/bytes of header if needed */


    /* write header */

    fwrite( (char *) &header, sizeof(header), 1, xwd_file);
    fwrite( wname, strlen(wname) + 1, 1, xwd_file);

    /*
    ** write color maps, if any.
    */
    fwrite( (char *)*colors, sizeof(XColor), colorct, xwd_file);

    /* write image data */

    fwrite( ximage->data, image_size, 1, xwd_file);

    /* wrap things up */

    fclose(xwd_file);

} /* xwdWrite */

int getColors
#if _PRDW_PROTO_
(
    Display		*display,
    Colormap		cmap,
    Visual		*visual,
    XColor		**colors
)
#else
(display,cmap,visual,colors)
Display			*display;
Colormap		cmap;
Visual			*visual;
XColor			**colors;
#endif
{
    int		i;
    Pixel	red,green,blue;

    XColor		*color_table;
    int			color_count;
    int			bit_shift;

    /*
    ** We don't get anywhere without a colormap.
    */
    if (cmap == (Colormap)0) return (0);

    /*
    ** This gives the number of entries in each map (or the whole map for
    ** single LUT visuals).
    */
    color_count = visual->map_entries;

    /*
    ** The color table for TrueColor and DirectColor need only be as
    ** big as one of the component LUTs in the Colormap.
    **
    ** The color values that we put into our table are those for the
    ** pixels with the red, green and blue components of the pixel equal
    ** to each other and to the color table index.
    **
    ** When we identify the rgb values for given pixels, we decompose
    ** the pixel and use each component to separately index the color
    ** table.
    **
    ** For other visuals, the color table size is the same as the size
    ** of the colormap.
    */
    color_table = (XColor *)XtCalloc (color_count, sizeof(XColor));

    /*
    ** We don't get anywhere without a color_table.
    */
    if (color_table == NULL) exit(0);

    /*
    ** Predefine a table for pixel conversion.  This way we can do
    ** all of the pixel identification from a single protocol request.
    ** We also do the conversion from 16 bit per component to 8 bit
    ** per component here.
    */
    if ((visual->class == TrueColor) ||
	(visual->class == DirectColor))
    {
	Pixel		rpix = 0, gpix = 0, bpix = 0;
	Pixel		rpix_incr, gpix_incr, bpix_incr;

/*
** This macro defines the least significant pixel in a mask.  This can then
** be used as an incrementor for the component.
*/
#define lowbit(x) ((x) & (~(x) + 1))

	/*
	** We now have the increment values for each component.
	*/
	rpix_incr = lowbit(visual->red_mask);
	gpix_incr = lowbit(visual->green_mask);
	bpix_incr = lowbit(visual->blue_mask);

	for (i = 0; i < (color_count); i++)
	{
	    /*
	    ** Color table with each of the components.
	    */
	    color_table[i].pixel = rpix | gpix | bpix;

	    /*
	    ** Increment each component.
	    */
	    rpix += rpix_incr;
	    gpix += gpix_incr;
	    bpix += bpix_incr;

	    /*
	    ** Wrap back on the components if necessary.
	    */
	    if (rpix > visual->red_mask) rpix = 0;
	    if (gpix > visual->green_mask) gpix = 0;
	    if (bpix > visual->blue_mask) bpix = 0;
	}
    }
    else
    {
	for (i = 0; i < (color_count); i++)
	{
	    color_table[i].pixel = i;
	}
    }

    /*
    ** Actually retreive the color values from the server.
    */
    XQueryColors (display, cmap, color_table, color_count);

    *colors = color_table;

    return (color_count);
} /* getColors */
