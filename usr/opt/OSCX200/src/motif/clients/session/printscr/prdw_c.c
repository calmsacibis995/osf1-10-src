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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/prdw_c.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
**++
**  COPYRIGHT (c) 1987, 1988, 1989, 1991 BY
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
**/


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
**	10-MAR-1991 Edward P Luwish
**		Separate memory-intensive portion of code and fork
**		as separate process.  This also solves Zombie process
**		problem by leaving condition handler alone.
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
 *	Dec	11,	1987, KMR
 *		Color stuff works now.
 *	Oct	26,	1987, KMR
 *		Works under BL 4
 *	July	6,	1987, MFA
 *		Color support.
 *	May	18,	1987, MFA
 *	    	Device is supposed to be a pointer.
 *	May	14, 	1987, MFA
 *		Change X11 call to X11 alpha update format.
 *	April 	6, 	1987, MFA

**--
**/
#include <sys/wait.h>
#include <signal.h>
#include "smdata.h"

extern void reaper();

/*
 *	dxPrscPrintScreenRect( dpy, optionslist, x, y, width, height )
 *
 *		dpy	- (R0) (*Display) display dpy
 *		optionslist - (RO) (*dxPrscOptions) optionslist, all are used
 *		x	- (R0) (long) x coordinate of screen image
 *		y	- (RO) (long) y coordinate of screen image
 *		height	- (RO) (long) height of screen image
 *		width	- (RO) (long) width of screen image
 *
 *	dwPrscPrintScreenWindow( dpy, window, optionslist )
 *
 *		window	- (R0) (*Window) window ID
 *
 *	requires:
 * 		valopt.c, convert.c, queuejob.c, getfile.c
 *	returns:
 *		status code as defined in prdw.h
 *	notes:
 *		none
 *
 */
#include	<stdio.h>
#include	<iprdw.h>
#include	<ChfDef.h>

#ifdef VMS
#include <decw$include/DECwDwtApplProg.h>
#endif


XImage	*XGetImage();
FILE	*getfile();



/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *      This routine is called when an exception or condition occurs which
 *      would ordinarily cause the Viewer, and anyone who invoked the Viewer,
 *      to exit.  For example, if the Viewer tries to divide by zero, an
 *      exception is raised and this routine will be invoked, provided it has
 *      been set up by a VAXC$ESTABLISH call.
 *
 *      All calls to this general handler are assumed to be fatal.  The
 *      Viewer code should never cause an exception, but we must clean up
 *      return gracefully if we get one.
 *
 *  FORMAL PARAMETERS:
 *
 *      signal  - signal vector
 *      mechanism - error mechanism arguments
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  FUNCTION VALUE:
 *
 *      DVR$_FATALERR   Viewer bugcheck
 *
 *  SIDE EFFECTS:
 *      Exits from the Viewer.
 *
 *--
 */
int condition_handler(signal, mechanism)
 struct chf$signal_array *signal;
 struct chf$mech_array  *mechanism;
{
 int    status;

#ifdef VMS
  signal->chf$l_sig_name = DECW$_PRSC_FATERRLIB;  /* Hide real error status with fatal error */
  LIB$SIG_TO_RET (signal, mechanism) ;
#endif
#ifdef ULTRIX
  SYS$SIGTORET( signal, mechanism );
#endif
}





unsigned long innerDxPrscPrintScreenRect(dfs, x, y, width, height)
int  dfs;
long	x;
long	y;
long	height;
long	width;
{
	XImage	*ximage;	/* pointer to X image	     		*/
	FILE	*file;		/* temporary output file		*/
	unsigned long status;	/* return status code			*/
	int	format;		/* image format				*/
	XWindowAttributes	winattr;
	dxPrscOptions   options;
	Visual		*vis;
	int isl;
	Display		*dpy;

	dpy = XOpenDisplay(smdata.display_name);
	/* Freeze screen while you take snapshot */
#ifndef GRAB
	XGrabServer(dpy);  
#endif


	/*	
	 * Convert options list into options structure. Acessing parts of a 
	 * structure is easier than rummaging through an item list.
	 */

	if( BAD( status = innerValopt( smdata.optionslist, &options, 1) ) ) 
		return( status ); 
	options.dpy = dpy;

	/*
	 * check parameters, fix if necessary,  and get an image 
	 */
	if( x < 0 )
		x = 0;
	if( y < 0 )
		y = 0;
	if( width < 0 )
		width = 8;
	if( height < 0 )
		height = 8;
	
	/*
	 * select correct image format based on device type; and get image 
	 * and (if needed) color info while screen is frozen; then unfreeze
	 */
	switch( DisplayPlanes( dpy, 0 ) )
	{
	case 1:
		format = ZPixmap;
		break;
	case 4:
	case 8:
	case 24:
		format = ZPixmap;
		break;
	default:
		return( status = dxPrscNoImage );
		break;
	}

	ximage = XGetImage( dpy, XRootWindow(dpy,dfs),
		x, y, width, height, AllPlanes, format );

	if( ximage == NULL )
		{
		XUngrabServer(dpy);
		XFlush(dpy);
		XCloseDisplay(dpy);
		return( status = dxPrscNoImage );
		}

        XGetWindowAttributes(dpy, RootWindow(dpy, dfs), &winattr);
	vis = winattr.visual;
	if (DisplayPlanes( dpy, dfs) != 1) {
		getcolor( dpy, dfs, RootWindow(dpy, dfs), vis);   
		}

#ifndef GRAB
	XUngrabServer(dpy);   
#endif
	XFlush(dpy);

/*
 * Set up a condition handler in case something goes wrong in ISL or 
 * print widget                                                     
 */
        status = Normal;
#ifndef GRAB
#ifdef VMS
        VAXC$ESTABLISH( condition_handler );
#else
     	ChfEstablish( condition_handler );
#endif
#endif


/*
 * Convert a multi-plane ximage to a 1 plane ximage
 */ 
	if (DisplayPlanes( dpy, dfs) != 1) {
		status = fiximage (&ximage, dpy, dfs, vis, &options); 
		}
	if( BAD (status) )
		{
		CleanXImage( dpy, dfs, vis, ximage);
		XCloseDisplay(dpy);
		return( status );
		}

	if (DisplayPlanes( dpy, dfs) != 1) {
		creat_color_islframe(&options, ximage, vis, &isl);
		}
	else    {
		creat_mono_islframe(&options, ximage, &isl);
		}

	CleanXImage( dpy, dfs, vis, ximage);

/* 
 * open output file
 */
	if( (file = getfile( &options ))  == NULL ) 
		{
		XCloseDisplay(dpy);
		return( status = DECW$_PRSC_BADFILEIO ); 
		}

/*
 * convert image to appropriate output form format
 */
	switch( options.storage_format )
	{
	case dxPrscPostscript:
		status = creatps(&options, ximage, file, isl);
		break;
	case dxPrscSixel:
		status = creatsx(&options, ximage, file, isl);
		break;
	case dxPrscDDIF:
		status = creatdd(&options, ximage, isl);
		break;
	default:
		XCloseDisplay(dpy);
		return(status = dxPrscIntChkFail);
	}

	XCloseDisplay(dpy);

	if (BAD(status))
		return(status);

	/*
	 * all done, close up shop
	 */
        if  (options.storage_format != dxPrscDDIF)  /* DDIF file are special*/
	        if( fclose( file ) == EOF )
		    return(status = dxPrscBadFileIO);
	return( status );
	
}

CleanXImage( dpy, dfs, vis, ximage)
Display		*dpy;
int		dfs;
Visual		*vis;
XImage		*ximage;
{
	if ( (DisplayPlanes( dpy, dfs) != 1) && (vis->class == PseudoColor) )
		{
		free(ximage->data);
                XFree((char *)ximage);
		}
	else
		XDestroyImage (ximage);
}

/*
 * dxPrscPrintScreenRect
 */

unsigned long dxPrscPrintScreenRect( dpy, dfs, optionslist, x, y,
		width, height, top)
Display	*dpy;
int	dfs;
long	*optionslist;
long	x;
long	y;
long	width;
long	height;
Widget	top;
{
	unsigned long status;
	dxPrscOptions options;
	int forkpid, waitpid;

	strcpy(smdata.display_name,DisplayString(XtDisplay(top)));
	signal(SIGCHLD,SIG_DFL);
	forkpid = fork();
	if (forkpid == 0)
	    {
	    status = innerDxPrscPrintScreenRect(dfs, x, y, width, height);
	    exit(status);
	    }
	if (forkpid == -1)
	    {
	    return;
	    }
	else
	    {
	    do
		{
		waitpid = wait(&status);
		} while (waitpid != forkpid);
	    }
	signal(SIGCHLD,reaper);

	if( BAD( status = innerValopt( smdata.optionslist, &options, 1) ) ) 
		return( status ); 
/*
 * print resulting image
 */


#ifdef PRINTWID
	if( BAD( status = getqueue( &options, top ) ) ) 
		return( status ); 
#endif

	/*
	 * it must have worked
	 */
	return( status );
}

/*
 * dxPrscPrintScreenWindow
 */
unsigned long dxPrscPrintScreenWindow(dpy, dfs, window, optionslist, top )
long 	window;
Display	*dpy;
int dfs;
long	*optionslist;
Widget	top;			/* Widget id of toplevel widget       */
{
	long	win_x;		/* upper left x coor of window		*/
	long	win_y;		/* upper left y coor of window		*/
	long	win_height;	/* height of window			*/
	long	win_width;	/* width of window			*/
	int	src_x,src_y;	
	int	dst_x,dst_y;	
	Window  ret_win, root;
	XWindowAttributes attr;	/* window attributes structure		*/
	unsigned long status;	/* return status code			*/

	
/*
 * get coordinates of window relative to Root window
 */
	if( BAD( status = XGetWindowAttributes( dpy, window, &attr ) ) )
		return( status );

        root = attr.root;
	src_x = attr.x;
	src_y = attr.y;
	if ( !XTranslateCoordinates(dpy, window, root,
				0, 0, &dst_x,&dst_y, &ret_win))
		return( status = XERROR );

	win_x = (long)dst_x;
	win_y = (long)dst_y;
	win_height = (long)attr.height - (long)attr.border_width;
	win_width = (long)attr.width - (long)attr.border_width;

/*
 * Now call PrintScreenRect
 */
	status = dxPrscPrintScreenRect( dpy, dfs, optionslist, win_x, win_y, 
		win_width, win_height, top);
	return( status );
}
