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
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/zoom.c,v 1.1.2.2 92/12/11 08:36:20 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   Handles creation and update of zoom widget
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**
**--
**/           
#include "paintrefs.h" 

int zoom_highlight;
static int zoom_hi_x, zoom_hi_y;
static int zoom_hi_wd, zoom_hi_ht;
static int max_zoom_wd, max_zoom_ht;

/*
 *
 * ROUTINE:  Highlight_Zoom
 *
 * ABSTRACT: 
 *
 *  Highlight the zoom area in the picture
 *
 */
void Highlight_Zoom()
{    
GC gc_solid;

    if ((zoom_highlight) || refresh_picture_pending) {
/* changed size of zoom magnifier - jj 10/11/88 */  
	gc_solid = Get_GC( GC_SD_SOLID );
	gc_values.foreground = colormap[WHITE].pixel;
	gc_values.line_width = 9;
	XChangeGC( disp, gc_solid, GCLineWidth|GCForeground, &gc_values );
	XDrawRectangle( disp, pwindow, gc_solid, IX_TO_PWX(zoom_xorigin-6), 
	 IY_TO_PWY(zoom_yorigin-6), zoom_width+11, zoom_height+11 );

	gc_values.line_width = 2;
	XChangeGC( disp, gc_solid, GCLineWidth, &gc_values );

	XSetForeground( disp, gc_solid, colormap[BLACK].pixel);
	XDrawRectangle( disp, pwindow, gc_solid, IX_TO_PWX(zoom_xorigin-11),
	 IY_TO_PWY(zoom_yorigin-11), zoom_width+22, zoom_height+22 );

	gc_values.line_width = 0;
	XChangeGC( disp, gc_solid, GCLineWidth, &gc_values );

	XDrawRectangle( disp, pwindow, gc_solid, IX_TO_PWX(zoom_xorigin-1), 
	 IY_TO_PWY(zoom_yorigin-1), zoom_width+1, zoom_height+1 );

 	XDrawLine( disp, pwindow, gc_solid,
	  IX_TO_PWX(zoom_xorigin-1), 
	  IY_TO_PWY(zoom_yorigin-1),
	  IX_TO_PWX(zoom_xorigin-11), 
	  IY_TO_PWY(zoom_yorigin-11) );
 	XDrawLine( disp, pwindow, gc_solid,
	  IX_TO_PWX(zoom_xorigin+zoom_width), 
	  IY_TO_PWY(zoom_yorigin-1),
	  IX_TO_PWX(zoom_xorigin+zoom_width+10), 
	  IY_TO_PWY(zoom_yorigin-11) );
 	XDrawLine( disp, pwindow, gc_solid,
	  IX_TO_PWX(zoom_xorigin+zoom_width), 
	  IY_TO_PWY(zoom_yorigin+zoom_height), 
	  IX_TO_PWX(zoom_xorigin+zoom_width+10), 
	  IY_TO_PWY(zoom_yorigin+zoom_height+10) );
 	XDrawLine( disp, pwindow, gc_solid,
	  IX_TO_PWX(zoom_xorigin-1), 
	  IY_TO_PWY(zoom_yorigin+zoom_height), 
	  IX_TO_PWX(zoom_xorigin-11), 
	  IY_TO_PWY(zoom_yorigin+zoom_height+10) );
    }
}   

/*
 *
 * ROUTINE:  UnHighlight_Zoom
 *
 * ABSTRACT: 
 *
 *  UnHighlight the zoom area in the picture
 *
 */
void UnHighlight_Zoom()
{
    if ((zoom_highlight) || refresh_picture_pending) {
	Copy_Bitmap( picture, pwindow, PWX_TO_IX(zoom_hi_x), 
	 PWY_TO_IY(zoom_hi_y), zoom_hi_wd, zoom_hi_ht, zoom_hi_x, zoom_hi_y );
	if( grid_on )
		Display_Grid( PWX_TO_IX(zoom_hi_x), PWY_TO_IY(zoom_hi_y),
			      zoom_hi_wd, zoom_hi_ht);
/*
	if( select_on )
		Refresh_Select_Highlight( zoom_xorigin, zoom_yorigin, zoom_width, zoom_height );
*/
    }
}   

/*
 *
 * ROUTINE:  Intersecting_Rectangles
 *
 * ABSTRACT: 
 *
 * Check to see if the two rectangle passed intersect
 *
 */                        
int Intersecting_Rectangles( rx1, ry1, rwd, rht, zx1, zy1, zwd, zht )
int rx1, ry1;
int rwd, rht;   
int zx1, zy1;
int zwd, zht;
{
int zx2, zy2;
int rx2, ry2;
              
	zx2 = zx1 + zwd;
	zy2 = zy1 + zht;

	rx2 = rx1 + rwd;
	ry2 = ry1 + rht;

/* rectangle contains zoom area */
	if( (rx1 <= zx1) && (rx2 >= zx2) && (ry1 <= zy1) && (ry2 >= zy2))
		return(TRUE);

/* zoom area contains rectangle */
	if( (rx1 >= zx1) && (rx2 <= zx2) && (ry1 >= zy1) && (ry2 <= zy2))
		return(TRUE);

/* top side intersects left edge of zoomed area */
	if( ((rx1 <= zx1) && (rx2 > zx1)) && ((ry1 >=zy1) && (ry1 < zy2)) )
		return(TRUE);

/* bottom side intersects left edge of zoomed area */
	if( ((rx1 <= zx1) && (rx2 > zx1)) && ((ry2 >=zy1) && (ry2 < zy2)) )
		return(TRUE);

/* top side intersects right edge of zoomed area */
	if( ((rx1 <= zx2) && (rx2 > zx2)) && ((ry1 >=zy1) && (ry1 < zy2)) )
		return(TRUE);

/* bottom side intersects right edge of zoomed area */
	if( ((rx1 <= zx2) && (rx2 > zx2)) && ((ry2 >=zy1) && (ry2 < zy2)) )
		return(TRUE);

/* left side intersects top edge of zoomed area */
	if( ((rx1 >= zx1) && (rx1 < zx2)) && ((ry1 <= zy1) && (ry2 > zy1)) )
		return(TRUE);                          

/* left side intersects bottom edge of zoomed area */
	if( ((rx1 >= zx1) && (rx1 < zx2)) && ((ry1 <= zy2) && (ry2 > zy2)) )
		return(TRUE);

/* right side intersects top edge of zoomed area */
	if( ((rx2 >= zx1) && (rx2 < zx2)) && ((ry1 <= zy1) && (ry2 > zy1)) )
		return(TRUE);

/* right side intersects bottom edge of zoomed area */
	if( ((rx2 >= zx1) && (rx2 < zx2)) && ((ry1 <= zy2) && (ry2 > zy2)) )
		return(TRUE);

	return(FALSE);
}


/*
long Pixel_Value(
    int x,
    int y)
{
    if (flood_image->data[((y)*bytes_per_line)+((x)>>3)] & (1<<((x)&7))) {
	return (1L);
    } else {
	return (0L);
    }
}
*/

/*
 *
 * ROUTINE:  Refresh_Magnifier
 *
 * ABSTRACT:  
 *
 * Refresh the zoom highlight rectangle if necessary.
 */                                                       
void Refresh_Magnifier( x1, y1, width, height )
int x1, y1, width, height; 
{
/* Check to refresh Zoom highlight */
    if (zoom_highlight){
	if( Intersecting_Rectangles( IX_TO_PWX(x1), IY_TO_PWY(y1), width, height, 
	  zoom_hi_x, zoom_hi_y, zoom_hi_wd, zoom_hi_ht ))
		Highlight_Zoom();
    }
}
/*
 *
 * ROUTINE:  Refresh_Zoom_View
 *
 * ABSTRACT:  
 *
 * Draw the pixels from the backup picture as squares in the window.
 *
 */                                                       
void Refresh_Zoom_View( x1, y1, width, height )
int x1, y1, width, height; 
{
XImage *image, *clip_image, *image_2;
int i,j,k;
long value;
int xpt, ypt;
int wd, ht;                                                        
int xtmp, ytmp;
int zx, zy;
int zwd, zht;
GC gc_solid;
long *bp;
int cells;
int from_2;
 
    if (Intersecting_Rectangles (x1, y1, width, height, 
	zoom_xorigin, zoom_yorigin, zoom_width, zoom_height)) {

/***********************************
 * Do some boundary checking       *
 **********************************/
/* Make sure left edge is no less than zoom_xorigin */
	if( x1 < zoom_xorigin )
	    zx = zoom_xorigin;
	else
	    zx = x1;

/* Make sure top edge is no less than zoom_yorigin */
	if( y1 < zoom_yorigin )
	    zy = zoom_yorigin;
	else
	    zy = y1;        

/* Make sure right edge is no greater than zoom_xorigin+zoom_width */

	if( (x1+width) > (zoom_xorigin+zoom_width) )
	    zwd = zoom_width - ( zx - zoom_xorigin );
	else {
	    if( x1 < zoom_xorigin )
	    	zwd = x1 + width - zoom_xorigin;
	    else
		zwd = width;
	}

/* Make sure bottom edge is no greater than zoom_yorigin+zoom_height */
	if( (y1+height) > (zoom_yorigin+zoom_height) )
	    zht = zoom_height - ( zy - zoom_yorigin );
	else {
	    if( y1 < zoom_yorigin )
		zht = y1 + height - zoom_yorigin;
	    else
		zht = height;
	}


/*******************************************************************
 * Get the image and draw squares to represent an individual pixel *
 ******************************************************************/

	clip_image = 0;
	image_2 = 0;
	switch (refresh_zoom_from) {
	    case FROM_PICTURE :
		image = XGetImage (disp, picture, zx, zy, zwd, zht, bit_plane,
				   img_format);
		break;
	    case FROM_COPYMAP :
		image = XGetImage (disp, copymap,  zx - x1, zy - y1, zwd, zht,
				   bit_plane, img_format);
		if ((clip_mask != 0) && (!opaque_fill || !select_rectangle)) {
		    clip_image = XGetImage (disp, clip_mask, zx - x1, zy - y1,
					    zwd, zht, 1, XYPixmap);
		    image_2 = XGetImage (disp, picture, zx, zy, zwd, zht,
					 bit_plane, img_format);
		}
	    	break;
	}

	if (image) {
	    gc_solid = Get_GC( GC_SD_SOLID );
	    wd = zoom_pixsize -zoom_pixborder /* *2 */;
	    ht = wd;

	    for (i = 0; i < zht; ++i)
		for (j = 0; j < zwd; ++j) {
		    from_2 = FALSE;
		    if ((clip_image != 0) && (image_2 != 0))
			if (XGetPixel (clip_image, j, i) == 0)
			    from_2 = TRUE;

		    if (from_2)
			value = XGetPixel (image_2, j, i);
		    else
			value = XGetPixel (image, j, i);

		    xtmp = j + zx - zoom_xorigin;
		    ytmp = i + zy - zoom_yorigin;
		    bp = bitvals + ytmp*zoom_width+xtmp;
		    if (value != *bp) {
			if (pdepth == 1) {
			    if (value == picture_fg)
				XSetForeground (disp, gc_solid,
						colormap[BLACK].pixel);
			    else
				XSetForeground (disp, gc_solid,
						colormap[WHITE].pixel);
			}
			else {
			    XSetForeground (disp, gc_solid, value);
			}
			xpt = xtmp*zoom_pixsize + zoom_pixborder;
			ypt = ytmp*zoom_pixsize + zoom_pixborder;
			XFillRectangle (disp, zoom, gc_solid, xpt, ypt, wd, ht);
			*bp = value;
		    } /* if (value != *bp) */
		} /* for (j = 0; j < zwd; ++j) */

	    if (clip_image != NULL)
	    {
#if 0
		XtFree (clip_image->data);
#endif
		XDestroyImage (clip_image);
	    }
	    if (image_2 != NULL)
	    {
#if 0
		XtFree (image_2->data);
#endif
		XDestroyImage (image_2);
	    }
	    if (image != NULL)
	    {
#if 0
		XtFree (image->data);
#endif
		XDestroyImage (image);
	    }
	    if (select_on) {
	    }
	    if (grid_on) {
		Draw_Zoom_Grid (zx, zy, zwd, zht);
	    }
	} /* if (image) */
    } /* if (Intersecting_Rectangles) */

}				


/*
 *
 * ROUTINE:  Initialize_Magnifier
 *
 * ABSTRACT: 
 *
 *  Initialize highlighting in the picture window
 *
 */
void Initialize_Magnifier()
{
XPoint pts[5];

/* Highlight the zoom area if necessary */
	if( zoom_region != 0 ) {
	  XDestroyRegion( zoom_region );
	  zoom_region = 0;
	}  
/* changed size of zoom magnifier - jj 10/11/88 */
	if( ! ((zoom_width == pwindow_wd) && (zoom_height == pwindow_ht)) ){
		zoom_highlight = TRUE;
		zoom_hi_x = IX_TO_PWX(zoom_xorigin-12);
		zoom_hi_y = IY_TO_PWY(zoom_yorigin-12);
		zoom_hi_wd = zoom_width + 24;
		zoom_hi_ht = zoom_height + 24;

		pts[0].x = zoom_xorigin - 12;
		pts[0].y = zoom_yorigin - 12;
		pts[1].x = zoom_xorigin + zoom_width + 11;
		pts[1].y = pts[0].y;
		pts[2].x = pts[1].x;
		pts[2].y = zoom_yorigin + zoom_height + 11;
		pts[3].x = pts[0].x;
		pts[3].y = pts[2].y;
		pts[4].x = pts[0].x;
		pts[4].y = pts[0].y;

		zoom_region = XPolygonRegion( pts, 5, EvenOddRule );
		Refresh_Magnifier( zoom_xorigin, zoom_yorigin, zoom_width, zoom_height );
	}  
	else { 
	    zoom_highlight = FALSE;
	    Set_Cursor (pwindow, current_action);
	}
}

/*
 *
 * ROUTINE:  Within_Zoom_Region
 *
 * ABSTRACT: 
 *
 *  Check to see if the given point is in the zoom region
 *  The given point is in picture window coordinates
 */
int Within_Zoom_Region( w, x, y)
Widget w;
int x, y;
{
	if( zoom_highlight ) 
	  return( (w == picture_widget) && 
          XPointInRegion( zoom_region, PWX_TO_IX(x), PWY_TO_IY(y)) );
	else
		return( FALSE );
}

/*
 *
 * ROUTINE:  Zoom_Dialog_Position
 *
 * ABSTRACT: 
 *
 *  Finds reasonable position for the zoom dialog box
 *  Returns an x, y position
 */
void Zoom_Dialog_Position( x, y )
Position *x, *y;
{
    Position px, py;
    int box_wd, box_ht;
    Arg args[5];
    int argcnt;

/* The zoom dialog box should be placed to the right or the left of the
 * current zoom area
 */
  argcnt = 0;
/* Since this doesn't work right now...
  Get_Attribute( toplevel, XmNx, &px );
  Get_Attribute( toplevel, XmNy, &py );
  px = px + IX_TO_PWX( zoom_xorigin );
  py = py + IY_TO_PWY( zoom_yorigin );
  box_wd = zoom_width * zoom_pixsize;
  box_ht = zoom_height * zoom_pixsize;
  if( (zoom_xorigin - box_wd) < 0 )
    *x = px + zoom_width + 5;
  else
    *x = px - box_wd - 5;

  *y = py - box_ht/2;
*/
    *x = 500;
    *y = DEFAULT_Y;
}


/*
 *
 * ROUTINE:  Find_Zoom_Rectangle
 *
 * ABSTRACT: 
 *
 *  Find the rectangle in the picture that should be zoomed.
 *  x,y are coordinates in the picture that the user wants zoomed.
 *  Outputs are: zoom_xorigin, zoom_yorigin, zoom_width, zoom_height.
 *  all are in picture coordinates.
 *
 */
void Find_Zoom_Rectangle (x, y, wd, ht)
    int x, y;
    int wd, ht;
{

/* Check to make sure zoom window is not larger than the picture */
    if (wd >  pwindow_wd)
	zoom_width = pwindow_wd;
    else
	zoom_width = wd;

    if (ht > pwindow)
	zoom_height = pwindow_ht;
    else
	zoom_height = ht;

/* Make sure zoom windows x and y origin are in bounds */
    zoom_xorigin = x - zoom_width/2;
    if (zoom_xorigin < pic_xorigin)
        zoom_xorigin = pic_xorigin;
    else {
/* changed positioning of zoom window - jj 10/11/88 */
        if((IX_TO_PWX(zoom_xorigin) + zoom_width) >= pwindow_wd) {
	    if (zoom_width < pwindow_wd)
		zoom_xorigin = pic_xorigin + pwindow_wd - zoom_width;
	    else
		zoom_xorigin = pic_xorigin;
	}
    }

    zoom_yorigin = y - zoom_height/2;
    if( zoom_yorigin < pic_yorigin )
	zoom_yorigin = pic_yorigin;
    else {
	if((IY_TO_PWY(zoom_yorigin) + zoom_height) >= pwindow_ht) {
	    if (zoom_height < pwindow_ht)
		zoom_yorigin = pic_yorigin + pwindow_ht - zoom_height;
	    else
		zoom_yorigin = pic_yorigin;
	}
    }
}


/*
 *
 * ROUTINE:  Resize_Zoom_Window
 *
 * ABSTRACT: 
 *
 *  Resize the zoom widget
 *
 */
void Resize_Zoom_Window (new_wd, new_ht)
    int new_wd, new_ht;
{
    int wd, ht;
    Arg args[3];
    int argcnt = 0;
    int i;
    int cells;
    long *bp;

/* free current bit values */
    if (bitvals != NULL)
    {
	XtFree ((char *)bitvals);
	bitvals = NULL;
    }

/* erase the current magnifier */
    UnHighlight_Zoom();

    wd = new_wd / zoom_pixsize + ((new_wd % zoom_pixsize) ? 1 : 0);
    ht = new_ht / zoom_pixsize + ((new_ht % zoom_pixsize) ? 1 : 0);

    Find_Zoom_Rectangle (zoom_xorigin + wd / 2, zoom_yorigin + ht/2, wd, ht);
    
/* re-initialize array containing current bit values */
    cells = zoom_width * zoom_height;
    bitvals = (long *) XtMalloc ((unsigned)(sizeof(long)*cells)); /* jj-port */

    bp = bitvals;
    for( i = cells; i > 0; --i )
	*bp++ = picture_bg;

/* re-initialize the magnifier */
    Initialize_Magnifier();
}


Move_Or_Resize_Zoom_Maybe ()
{
    int x, y, wd, ht;

    x = zoom_xorigin;
    y = zoom_yorigin;
    wd = zoom_width;
    ht = zoom_height;

    Find_Zoom_Rectangle (zoom_xorigin + wd / 2, zoom_yorigin + ht/2, wd, ht);

    if ((wd != zoom_width) || (ht != zoom_height)) {
	Resize_Widget (zoom_dialog, zoom_width * zoom_pixsize,
                       zoom_height * zoom_pixsize);
    }
    else {
	if ((x != zoom_xorigin) || (y != zoom_yorigin)) {
	    UnHighlight_Zoom();
	    Refresh_Zoom_View (zoom_xorigin, zoom_yorigin, zoom_width,
			       zoom_height);
	    Initialize_Magnifier();
	}
    }
}

/*
 *
 * ROUTINE:  End_Zoom
 *
 * ABSTRACT: 
 *
 *  Make the zoom widget invisible
 *
 */
void End_Zoom()
{
	if( zoom_highlight ){
		UnHighlight_Zoom();
		XDestroyRegion( zoom_region );
		zoom_highlight = FALSE;
		zoom_region = 0;
		}
/*
	XtDestroyWidget(zoom_dialog);
	zoom_dialog = 0;
*/
	XtUnmanageChild (zoom_dialog);
	if (bitvals != NULL)
	{
	    XtFree ((char *)bitvals);
	    bitvals = NULL;
	}
	zoomed = FALSE;                 
	Stop_Motion_Events( picture_widget );
}   

/*
void Set_Zoom_Size_Hints ()
{
    XSizeHints size_hints;

    if (XGetNormalHints (disp, XtWindow (zoom_dialog), &size_hints) != 0) {
	size_hints.min_width = MIN_PICTURE_WD * zoom_pixsize;
	size_hints.min_height = MIN_PICTURE_HT * zoom_pixsize;
	size_hints.max_width = MIN ((pwindow_wd * zoom_pixsize), max_zoom_wd);
	size_hints.max_height = MIN ((pwindow_ht * zoom_pixsize), max_zoom_ht);
	XSetNormalHints (disp, XtWindow (zoom_dialog), &size_hints);
    }		
}
*/

/*
 *                                                  
 * ROUTINE:  Begin_Zoom
 *
 * ABSTRACT: 
 *
 *  Create a separate window(if necessary) and give a zoomed view of the
 *  surrounding area
 *
 */
void Begin_Zoom (x, y)
int x, y;
{
    int i, j;
    Position px, py;
    Dimension wd, ht;
    Widget p;
    Arg args[15];
    int argcnt = 0;
    int cells;
    long *bp;
                 
/* Draw the pixels contained in the rectangle.
 * The zoom_xorigin,zoom_yorigin correspond to the picture coordinate
 * that the zoom window is mapped to. 
 */

    Set_Cursor_Watch (pwindow);
/* Create Zoom dialog if it does not already exist */
    if (!zoom_dialog) {
        if (Fetch_Widget ("zoom_dialog_box", main_widget,
		          &zoom_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch line dialog");

	zoom_widget = widget_ids[ZOOM_WINDOW];

/* treg -> */
	if (visual_info->visual != XDefaultVisual (disp, screen)) {
 
	    Set_Attribute (zoom_widget, XmNdepth, pdepth);
	    Set_Attribute (zoom_widget, XmNcolormap, paint_colormap);
	    Set_Attribute (zoom_widget, XmNbackground, colormap[WHITE].pixel);
	    Set_Attribute (zoom_widget, XmNborder, colormap[BLACK].pixel);

	}
/* <- treg */

	XtInstallAllAccelerators (zoom_widget, main_widget);

	max_zoom_wd = screen_wd - (screen_wd % zoom_pixsize);
	max_zoom_ht = screen_ht - (screen_ht % zoom_pixsize);

	Zoom_Dialog_Position (&px, &py);

	XtSetArg (args[2], XmNx, px);
	argcnt++;
	XtSetArg (args[3], XmNy, py);
	argcnt++;
    }

/* Set zoom width and height */
    if( pimage_wd < 60 )
	wd = pwindow_wd;
    else
	wd = 40;

    if( pimage_ht < 60 )
	ht = pwindow_ht;
    else
	ht = 40;
	
    Find_Zoom_Rectangle (x, y, wd, ht);

    old_zoom_wd = zoom_width * zoom_pixsize;
    old_zoom_ht = zoom_height * zoom_pixsize;

/* set zoom dialog attributes */
    XtSetArg (args[0], XmNwidth, (Dimension)old_zoom_wd); 
    argcnt++;
    XtSetArg (args[1], XmNheight, (Dimension)old_zoom_ht); 
    argcnt++;

    XtSetValues (zoom_dialog, args, argcnt);

    XtManageChild(zoom_dialog);

/* initialize array containing current bit values */
    cells = zoom_width*zoom_height;
    bitvals = (long *) XtMalloc( (unsigned)(sizeof(long)*cells)); /* jj-port */
    bp = bitvals;
    for( i = cells; i > 0; --i )
	*bp++ = picture_bg;

    zoom = XtWindow( zoom_widget );
    zoom_region = 0;

    Initialize_Magnifier();
    zoomed = TRUE;
    Set_Cursor (pwindow, current_action);
/*    Set_Cursor(zoom, current_action); */
    Start_Motion_Events( picture_widget );
}
