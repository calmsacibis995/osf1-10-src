
/*
**++
**  COPYRIGHT (c) 1987, 1988, 1989, 1990, 1991, 1992 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
** 
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
** 
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
** 
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
*/


/*
**++
**  MODULE NAME:
**	prdw_c.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	Standard C Bindings, entry into PrintScreen DECwindows
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson, Mark Antonelli
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:     
**	6-APR-1987
**
**  MODIFICATION HISTORY:
**
**	16-Aug-1992	Edward P Luwish
**		Fold in stuff from getcolor.c and creatisl.c, add calls
**		to convert spectral classes
**
**	 7-Feb-1992	Edward P Luwish
**		Many changes made before eliminating entire module.
**
**	 3-Jun-1991	Edward P Luwish
**		Precede sleep with XFlush to make more reliable delay times
**
**	14-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
**	13-JUN-1990	KMR
**		clean up condition handling
**		
**	18-MAY-1988
**		Close Display
**		Removed obselete PrintBitmap code
**	9-MAY-1988
**		Change device to dpy for consistency
**		removed flush calls
**      7-APR-1988
**		Remove restrictions on color
**		DECwDwtWidgetProg now included in iprdw.h
**	30-MAR-1988
**		Added printwidget support
**	21-MAR-1988
**		Switch to options list and new facility code
**	11-JAN-1988
**		Add options to fiximage call
**		add call to creatdd and play games with file open
**		Translate coords wrt root in PrintScreenRect
**		Exit if 4 plane system
**	6-JAN-1988
**		Removed window parameter form dxPrscSreenRect
**		Round requested width UP to nearest multple of 4
**		Calls color routines (module getcolor)
**		Calls XFlush
**
**	Dec	11,	1987, KMR
**		Color stuff works now.
**	Oct	26,	1987, KMR
**		Works under BL 4
**	July	6,	1987, MFA
**		Color support.
**	May	18,	1987, MFA
**	    	Device is supposed to be a pointer.
**	May	14, 	1987, MFA
**		Change X11 call to X11 alpha update format.
**	April 	6, 	1987, MFA
**
**--
*/

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/prdw_c.c,v 1.2.1.1 92/04/16 12:31:08 Ed_Luwish Exp Locker: Ed_Luwish $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*

/*
**	dxPrscPrintScreenRect( dpy, command, x, y, width, height )
**
**		dpy	- (R0) (*Display) display dpy
**		command - (RO) (*Command)
**		x	- (R0) (long) x coordinate of screen image
**		y	- (RO) (long) y coordinate of screen image
**		height	- (RO) (long) height of screen image
**		width	- (RO) (long) width of screen image
**
**	dwPrscPrintScreenWindow( dpy, window, command)
**
**		window	- (R0) (*Window) window ID
**
**	dxPrscPrintScreenBitmap( length, address, command)
**		
**		length	- (RO) (long) length of bitmap buffer
**		address - (RO) (long) address of the first byte of buffer
**
**	requires:
** 		valopt.c, convert.c, queuejob.c, getfile.c
**	returns:
**		status code as defined in prdw.h
**	notes:
**		none
**
*/

#ifndef VMS
#include <unistd.h>
#endif
#include <stdio.h>
#ifdef VMS
#include <descrip.h>
#endif
#include "iprdw.h"
#if defined(VMS)
#endif
#include "smshare.h"

#ifdef VMS
#include "desc.h"
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <img/IdsImage.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#include <Mrm/MrmAppl.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw_entry.h"

extern char equiv_string[];

static unsigned long capture_it PROTOTYPE((
    Display		*dpy,
    int			dfs,
    int			x,
    int			y,
    unsigned int	width,
    unsigned int	height,
    XImage		**ximage,
    Colormap		*cmap,
    Visual		**vis
));

static unsigned long external_hooks PROTOTYPE((
    Display		*dpy,
    int			dfs,
    Command		*command,
    XImage		*ximage,
    Colormap		cmap,
    Visual		*vis
));

static unsigned long convert_out PROTOTYPE((
    Display		*dpy,
    int			dfs,
    Command		*command,
    XImage		*ximage,
    Visual		*vis,
    Colormap		cmap,
    Widget		top
));

/*
 * dxPrscPrintScreenWindow
 */
unsigned long dxPrscPrintScreenWindow
#if _PRDW_PROTO_
(
    Display	*dpy,
    int		dfs,
    Window	window,
    Command	*command,
    Widget	top
)
#else
(dpy, dfs, window, command, top)
    Display	*dpy;
    int		dfs;
    Window 	window;
    Command	*command;
    Widget	top;
#endif
{
    int			win_x;		/* upper left x of window */
    int			win_y;		/* upper left y of window */
    unsigned int	win_height;	/* height of window */
    unsigned int	win_width;	/* width of window */
    int			src_x,src_y;	
    int			dst_x,dst_y;	
    Window		ret_win, root;
    XWindowAttributes	winattr;	/* window attr structure */
    unsigned long	status;		/* return status code */
	
    /*
    ** get coordinates of window relative to Root window
    */
    if (BAD(status = XGetWindowAttributes (dpy, window, &winattr)))
	return (status);

    root = winattr.root;
    src_x = winattr.x;
    src_y = winattr.y;

    if (!XTranslateCoordinates
	(dpy, window, root, 0, 0, &dst_x,&dst_y, &ret_win))
	return (status = XERROR);

    win_x = dst_x;
    win_y = dst_y;
    win_height = winattr.height - winattr.border_width;
    win_width = winattr.width - winattr.border_width;

    /*
    ** Now call PrintScreenRect
    */
    status = dxPrscPrintScreenRect
	(dpy, dfs, command, win_x, win_y, win_width, win_height, top);
    return (status);
}

/*
 * dxPrscPrintScreenRect
 */
unsigned long dxPrscPrintScreenRect
#if _PRDW_PROTO_
(
    Display		*dpy,
    int			dfs,
    Command		*command,
    int			x,
    int			y,
    unsigned int	width,
    unsigned int	height,
    Widget		top
)
#else
(dpy, dfs, command, x, y, width, height, top)
    Display		*dpy;
    int			dfs;
    Command		*command;
    int			x;
    int			y;
    unsigned int	width;
    unsigned int	height;
    Widget		top;
#endif
{
    XImage		*ximage;
    unsigned long	status = Normal;
    Visual		*vis;
    Colormap		cmap;

    command->options.dpy = dpy;

    XFlush(dpy);
/*
    sleep(command->options.delay);
*/

    /*
    ** check parameters, fix if necessary,  and get an image 
    */
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (width < 0) width = 8;
    if (height < 0) height = 8;

    /*
    ** Actually do the capture.  There is a grab screen in here so be careful.
    */
    status = capture_it
	(dpy, dfs, x, y, width, height, &ximage, &cmap, &vis);

    if (BAD(status)) return (status);

    if (ximage == NULL) return (status = dxPrscNoImage);

    /*
    ** Attempt to use external format converters.  If returns true then it
    ** did so and we are done.
    */
    if (external_hooks (dpy, dfs, command, ximage, cmap, vis))
	return (Normal);

    /*
    ** Use various ISL things to convert to the right image type and file
    ** format.
    */
    status = convert_out (dpy, dfs, command, ximage, vis, cmap, top);
    if (BAD(status)) return (status);

    /*
    ** It must have worked
    */
    return (status);
}

static unsigned long capture_it
#if _PRDW_PROTO_
(
    Display		*dpy,
    int			dfs,
    int			x,
    int			y,
    unsigned int	width,
    unsigned int	height,
    XImage		**ximage,
    Colormap		*cmap,
    Visual		**vis
)
#else
(dpy, dfs, x, y, width, height, ximage, cmap, vis)
    Display		*dpy;
    int			dfs;
    int			x;
    int			y;
    unsigned int	width;
    unsigned int	height;
    XImage		**ximage;
    Colormap		*cmap;
    Visual		**vis;
#endif
{
    XWindowAttributes	winattr;
    int			format = ZPixmap;
    int			revert;
    Window		root_window;
    Window		fwindow;
    int			cmap_count;
    int			cmap_index;
    Colormap		*cmap_list;
    unsigned long	status;
    /*
    ** select correct image format based on device type; and get image 
    ** and (if needed) color info while screen is frozen; then unfreeze
    */
    switch (DisplayPlanes (dpy, dfs))
    {
    case 1:
    case 4:
    case 8:
    case 24:
	format = ZPixmap;
	break;
    default:
	*ximage = NULL;
	return (status = dxPrscNoImage);
	break;
    }

    root_window = RootWindow(dpy,dfs);

    *ximage = XGetImage
	(dpy, root_window, x, y, width, height, AllPlanes, format);

    /*
    ** We're going to get the Colormap from the window with focus.
    */
    XGetInputFocus (dpy, &fwindow, &revert);

    /*
    ** If necessary get the root window.
    */
    if (fwindow == None || fwindow == PointerRoot)
	fwindow = root_window;

    /*
    ** Get the Colormap and Visual from the window.
    */
    XGetWindowAttributes (dpy, fwindow, &winattr);
    *cmap = winattr.colormap;
    *vis = winattr.visual;
 
    /*
    ** Make sure we are using an installed Colormap.  Note that the cmap
    ** list must come from the correct screen, so we use root_window.
    */
    cmap_list = XListInstalledColormaps (dpy, root_window, &cmap_count);

    /*
    ** Determine whether the one used by the window with focus is in the
    ** installed set.  If so, then we are all set.
    */
    for (cmap_index = 0; cmap_index < cmap_count; cmap_index++)
    {
	if (cmap_list[cmap_index] == *cmap) break;
    }

    /*
    ** If one used by the window is not in the installed set and if there is
    ** an installed set, then use the first one in the installed set.
    ** Otherwise, it will remain set to the colormap for the window with
    ** focus.
    */
    if (cmap_index == cmap_count && cmap_count != 0) *cmap = cmap_list[0];

#ifdef R5_XLIB
    {
#include <X11/Xcms.h>
	/*
	** We can only get from the colormap to the visual on R5.
	*/
	XcmsCCC	ccc;
	ccc = XcmsCCCOfColormap (dpy, *cmap);
	*vis = VisualOfCCC (ccc);
    }
#endif

    XUngrabServer(dpy);
    XFlush(dpy);

/*
**
** END OF DANGER ZONE.
**
*/
    return (Normal);
}

static unsigned long external_hooks
#if _PRDW_PROTO_
(
    Display		*dpy,
    int			dfs,
    Command		*command,
    XImage		*ximage,
    Colormap		cmap,
    Visual		*vis
)
#else
(dpy, dfs, command, ximage, cmap, vis)
    Display		*dpy;
    int			dfs;
    Command		*command;
    XImage		*ximage;
    Colormap		cmap;
    Visual		*vis;
#endif
{
    char		*shell_command;
    XColor		*colors;
    int			numcolors;
    unsigned long	status;
    /*
    ** if DXPRINT_XWD exists, write xwd file and return
    */
    shell_command = equiv_string;

    if (getEquiv ("DXPRINT_XWD", shell_command))
    {
	numcolors = getColors (dpy, cmap, vis, &colors);

	xwdWrite
	(
	    command->print_dest,	/* file name */
	    ximage,			/* image struct */
	    numcolors,			/* color count */
	    vis,
	    "Print Screen",		/* window name */
	    &colors,			/* XColor struct */
	    dpy,			/* display */
	    dfs				/* screen */
	);
	if (numcolors != 0) XtFree ((char *)colors);
	XDestroyImage(ximage);
	return (True);
    }
    /*
    ** if DXPRINT_PS exists, write xwd file, fork the command in DXPRINT_PS
    ** and return
    **
    ** if DXPRINT_SX exists, write xwd file, fork the command in DXPRINT_SX
    ** and return
    **
    ** if DXPRINT_DD exists, write xwd file, fork the command in DXPRINT_DD
    ** and return
    */
    switch (command->options.storage_format)
    {
    case dxPrscPostscript:
	if (getEquiv ("DXPRINT_PS", shell_command))
	{
	    numcolors = getColors (dpy, cmap, vis, &colors);

	    xwdWrite
	    (
		command->print_dest,	/* file name */
		ximage,			/* image struct */
		numcolors,		/* color count */
		vis,
		"Print Screen",		/* window name */
		&colors,		/* XColor struct */
		dpy,			/* display */
		dfs			/* screen */
	    );
	    execShellCmd (shell_command);

	    if (numcolors != 0) XtFree ((char *)colors);
	    XDestroyImage (ximage);
	    return (True);
	}
	break;
    case dxPrscSixel:
	if (getEquiv ("DXPRINT_SX", shell_command))
	{
	    numcolors = getColors (dpy, cmap, vis, &colors);

	    xwdWrite
	    (
		command->print_dest,	/* file name */
		ximage,			/* image struct */
		numcolors,		/* color count */
		vis,
		"Print Screen",		/* window name */
		&colors,		/* XColor struct */
		dpy,			/* display */
		dfs			/* screen */
	    );
	    execShellCmd (shell_command);

	    if (numcolors != 0) XtFree ((char *)colors);
	    XDestroyImage (ximage);
	    return (True);
	}
	break;

    case dxPrscDDIF:
	if (getEquiv ("DXPRINT_DD", shell_command))
	{
	    numcolors = getColors (dpy, cmap, vis, &colors);

	    xwdWrite
	    (
		command->print_dest,	/* file name */
		ximage,			/* image struct */
		numcolors,		/* color count */
		vis,
		"Print Screen",		/* window name */
		&colors,		/* XColor struct */
		dpy,			/* display */
		dfs			/* screen */
	    );
	    execShellCmd (shell_command);

	    if (numcolors != 0) XtFree ((char *)colors);
	    XDestroyImage (ximage);
	    return (True);
	}
	break;
    }
    return (False);
}

static unsigned long convert_out
#if _PRDW_PROTO_
(
    Display		*dpy,
    int			dfs,
    Command		*command,
    XImage		*ximage,
    Visual		*vis,
    Colormap		cmap,
    Widget		top
)
#else
(dpy, dfs, command, ximage, vis, cmap, top)
    Display		*dpy;
    int			dfs;
    Command		*command;
    XImage		*ximage;
    Visual		*vis;
    Colormap		cmap;
    Widget		top;
#endif
{
    int			image_height;
    int			image_width;
    unsigned long	fid;
    FILE		*file = NULL;
    unsigned long	status = 0;
    unsigned long	newfid;
    unsigned int	conv = False;
    int			system_color;
    unsigned long	output_class;

    /*
    ** Set up a condition handler in case something goes wrong in ISL or 
    ** print widget                                                     
    */
    status = Normal;
#ifndef GRAB
    ChfEstablish( img_signal_handler );
#endif

    /*
    ** Convert Ximage to FID
    */
    if (ximage->depth != 1)
	output_class = ImgK_ClassMultispect;
    else
	output_class = ImgK_ClassBitonal;

    fid = XimageToFid
    (
	dpy,
	ScreenOfDisplay(dpy,dfs),
	ximage,
	vis,
	cmap,
	output_class,
	command->options.reverse_image
    );

    image_height = ximage->height;
    image_width = ximage->width;

    /*
    ** Destroy ximage
    */
    XDestroyImage(ximage);

    /* 
    ** open output file
    */
    if ((file = getfile (NULL))  == NULL)
    {
	ImgDeleteFrame (fid);
	return (status = dxPrscBadFileIO);
    }

    /*
    ** convert spectral classes, if necessary
    */
    newfid = fid;
    system_color = determine_system_color (dpy, dfs);

    if (system_color == color_system)
    {
	if (command->options.print_color == dxPrscPrinterBW)
	{
	    newfid = CvtDataClass
	    (
		(unsigned long)fid,
		(int)dxPrscPrinterColor,
		(int)dxPrscPrinterBW
	    );
	    conv = True;
	}
	if (command->options.print_color == dxPrscPrinterGrey)
	{
	    newfid = CvtDataClass
	    (
		(unsigned long)fid,
		(int)dxPrscPrinterColor,
		(int)dxPrscPrinterGrey
	    );
	    conv = True;
	}
    }
    if ((system_color == gray_system) &&
	(command->options.print_color == dxPrscPrinterBW ))
    {
	newfid = CvtDataClass
	(
	    (unsigned long)fid,
	    (int)dxPrscPrinterGrey,
	    (int)dxPrscPrinterBW
	);
	conv = True;
    }

    if (newfid == 0)
    {
	return (status = dxPrscNoMemory);
    }

    /*
     * convert image to appropriate output form format
     */

    switch (command->options.storage_format)
    {
    case dxPrscPostscript:
	status = creatps
	    (NULL, image_height, image_width, file, newfid, conv, dpy, dfs);
	break;
    case dxPrscSixel:
	status = creatsx
	    (NULL, image_height, image_width, file, newfid, conv, dpy, dfs);
	break;
    case dxPrscDDIF:
	status = creatdd
	    (NULL, image_height, image_width, ximage, newfid, conv, dpy, dfs);
	break;

    default:
	return (status = dxPrscIntChkFail);
    }
    if (BAD(status)) return (status);

    /*
    ** all done, close up shop
    */
    if (command->options.storage_format != dxPrscDDIF)
    {
	/* DDIF file are special*/
	if (fclose (file) == EOF) return (status = dxPrscBadFileIO);
    }

    /*
    ** print resulting image
    */
    if ((command->options.send_to_printer != dxPrscFile) &&
	(command->options.command_mode = dxPrscXmode))
    {
	send_print_job ();
    }

    return (status);
	
}
