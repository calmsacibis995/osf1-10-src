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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/draw.c,v 1.1.2.2 92/12/11 08:34:10 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module DRAW "V1-000"
  #endif
*/
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
**   DRAW contains routines that draw shapes into the picture
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**	dl	10/6/88
**	Don't draw brush stroke if fill pattern is none
**--
**/
                             
#include "paintrefs.h"

/*
 * This should draw a shape or copy a pixmap to the given point depending
 * on what the user has specified as the current pen.
 */
void Draw_With_Brush( w, x, y)
Drawable w;
int x, y;
{            
int tmp_x, tmp_y;
int x1, y1, x2, y2;

if( outline_stipple || (current_action == ERASE) ){	/* jj - 3/27/89 */
/* if( fill_stipple || (current_action == ERASE) ){ */
	if( cur_brush == ROUND_BRUSH ){
		tmp_x = x - brush_wd/2;
		tmp_y = y - brush_wd/2;
		XFillArc( disp, w, cur_gc, tmp_x, tmp_y, brush_wd, brush_wd, 0, 64*360);
		}
	else
	if( cur_brush==SQUARE_BRUSH ){
		tmp_x = x - brush_wd/2;
		tmp_y = y - brush_wd/2;
		XFillRectangle( disp, w, cur_gc, tmp_x, tmp_y, brush_wd, brush_wd );
		}
	else{                        
		x1 = x + brush_x1;
		y1 = y + brush_y1;
		x2 = x + brush_x2;
		y2 = y + brush_y2;
	 	XDrawLine( disp, w, cur_gc, x1, y1, x2, y2 );
	 	}
	}
}
     
/*
 *
 * ROUTINE:  Draw_Square_Brush
 *
 * ABSTRACT: Simulate shape as drawn with a square brush
 *
 */
void Draw_Square_Brush( w, pts, npts )
Drawable w;
XPoint *pts;                
int npts;
{
static int num = 7;
XPoint polypts[7];
    int i, j, k;
    int wd;

    wd = brush_wd/2;
    for( i = 1; i < npts; ++i ) {
/*
	if( (pts[i].x == pts[i-1].x) && (pts[i].y == pts[i-1].y) )
	    Draw_With_Brush( w, pts[i-1].x, pts[i-1].y );
	else
*/
	if (((pts[i].x >= pts[i-1].x) && (pts[i].y <= pts[i-1].y)) ||
	    ((pts[i].x <= pts[i-1].x) && (pts[i].y >= pts[i-1].y))) {
	    if ((pts[i].x >= pts[i-1].x) && (pts[i].y <= pts[i-1].y)) {
		j = i-1;
		k = i;
	    }
	    else {
		j = i;
		k = i-1;
	    }
	    polypts[0].x = pts[j].x - wd;
	    polypts[0].y = pts[j].y - wd;
	    polypts[1].x = pts[k].x - wd;
	    polypts[1].y = pts[k].y - wd;
	    polypts[2].x = pts[k].x + wd;
	    polypts[2].y = pts[k].y - wd;
	    polypts[3].x = pts[k].x + wd;            
	    polypts[3].y = pts[k].y + wd;
	    polypts[4].x = pts[j].x + wd;
	    polypts[4].y = pts[j].y + wd;
	    polypts[5].x = pts[j].x - wd;
	    polypts[5].y = pts[j].y + wd;
	    polypts[6].x = polypts[0].x;
	    polypts[6].y = polypts[0].y;
	    XFillPolygon( disp, w, cur_gc, polypts, num, Complex, CoordModeOrigin);
/*
		Draw_With_Brush( w, pts[i-1].x, pts[i-1].y );
		Draw_With_Brush( w, pts[i].x, pts[i].y );
*/
	}
	else {
	    if ((pts[i].x >= pts[i-1].x) && (pts[i].y >= pts[i-1].y)) {
		j = i-1;
		k = i;
	    }
	    else {
		j = i;
		k = i-1;
	    }
	    polypts[0].x = pts[j].x + wd;
	    polypts[0].y = pts[j].y - wd;
	    polypts[1].x = pts[k].x + wd;
	    polypts[1].y = pts[k].y - wd;
	    polypts[2].x = pts[k].x + wd;
	    polypts[2].y = pts[k].y + wd;
	    polypts[3].x = pts[k].x - wd;            
	    polypts[3].y = pts[k].y + wd;
	    polypts[4].x = pts[j].x - wd;
	    polypts[4].y = pts[j].y + wd;
	    polypts[5].x = pts[j].x - wd;
	    polypts[5].y = pts[j].y - wd;
	    polypts[6].x = polypts[0].x;
	    polypts[6].y = polypts[0].y;
	    XFillPolygon( disp, w, cur_gc, polypts, num, Complex, CoordModeOrigin);
/*
	    Draw_With_Brush( w, pts[i-1].x, pts[i-1].y );
	    Draw_With_Brush( w, pts[i].x, pts[i].y );
*/
	}
    }
}
/*
 *
 * ROUTINE:  Simulate_Brush
 *
 * ABSTRACT: Simulate a real bitmap brush by constructing polygons to
 *           connect the points.
 *
 */
void Simulate_Brush( w, pts, npts )
Drawable w;
XPoint *pts;                
int npts;
{
static int num = 5;
XPoint polypts[5];
int i, j;
int x1, y1, x2, y2;

		
/* don't draw anything if there is no stipple dl - 10/6/88 */
    if( outline_stipple || (current_action == ERASE) ){	/* jj - 3/27/89 */
/*  if( fill_stipple || (current_action == ERASE) ){ */
	if( npts == 1 ){
		Draw_With_Brush( w, pts[0].x, pts[0].y );
		return;
		}

/* Use square brush when drawing eraser too */
	if( (cur_brush == SQUARE_BRUSH) )
		Draw_Square_Brush( w, pts, npts );
	else
	if( cur_brush == ROUND_BRUSH )
		XDrawLines( disp, w, cur_gc, pts, npts, CoordModeOrigin);
	else{
		x1 = brush_x1;
		y1 = brush_y1;
		x2 = brush_x2;
		y2 = brush_y2;

/* use point pairs to construct polygon */
		for( i = 1; i < npts; ++i ){
			if( (pts[i].x == pts[i-1].x) && (pts[i].y == pts[i-1].y) )
				Draw_With_Brush( w, pts[0].x, pts[0].y );
			else{                        
				polypts[0].x = pts[i-1].x + x1;
				polypts[0].y = pts[i-1].y + y1;
				polypts[1].x = pts[i].x + x1;
				polypts[1].y = pts[i].y + y1;
				polypts[2].x = pts[i].x + x2;            
				polypts[2].y = pts[i].y + y2;
				polypts[3].x = pts[i-1].x + x2;
				polypts[3].y = pts[i-1].y + y2;
				polypts[4].x = polypts[0].x;
				polypts[4].y = polypts[0].y;
				XFillPolygon( disp, w, cur_gc, polypts, num, Complex, CoordModeOrigin);
				XDrawLines( disp, w, cur_gc, polypts, num, CoordModeOrigin);
				}
			}
		}
/*
		}
*/
    }
}                                 
/*
 *
 * ROUTINE:  Parametric Arc
 *
 * ABSTRACT: 
 *
 * Parametric Ellipse Algorithm taken from: "Mathematical Elements for 
 * Computer Graphics" by David F. Rogers and J. Alan Adams. pg. 216
 *      
 */
void Parametric_Arc( w, x, y, semi_major, semi_minor, beg_angle, sweep_angle )
Drawable w;
int x, y;
int semi_major, semi_minor;
int beg_angle, sweep_angle;
{
int center_x, center_y;
double c1, c2, c3;            
double s1, s2, s3;
double x1, y1;
double p, i1, t1;
double beg_sin, end_sin;
int new_x, new_y;
int old_x, old_y;
int m;
int n;
XPoint pts[2];

		center_x = x + semi_major;
		center_y = y + semi_minor;
		n = abs( sweep_angle )/6;
		p = 2*PI/( 360/6 -1);		/* Calc. the increment in the parameter */
		i1 = 0; /* 0/RADIANS_PER_DEGREE degrees of incline */
		c1 = cos(i1);			/* Cosine and sine of inclination angle */
		s1 = sin(i1);
		c2 = cos(p);			/* Cosine and sine of parameter increment */
		s2 = sin(p);
/*
		c3 = 1.0;
		s3 = 0.0;
*/
		c3 = cos( beg_angle/RADIANS_PER_DEGREE );
		s3 = sin( beg_angle/RADIANS_PER_DEGREE );
		x1 = (double)(semi_major)*c3;
		y1 = (double)(semi_minor)*s3;
		pts[0].x = (int)( (double)center_x + x1*c1 - y1*s1);
		pts[0].y = (int)( (double)center_y - x1*s1 - y1*c1);
		for( m = 0; m < n; ++m ){                        
			x1 = (double)(semi_major)*c3;
			y1 = (double)(semi_minor)*s3;

				pts[1].x = (int)( (double)center_x + x1*c1 - y1*s1);
				pts[1].y = (int)( (double)center_y - x1*s1 - y1*c1);
				Simulate_Brush( w, pts, 2 );
				pts[0].x = pts[1].x;
				pts[0].y = pts[1].y;

			t1 = c3*c2 - s3*s2;
			s3 = s3*c2 + c3*s2;
			c3 = t1;
			}
}

/*
 *
 * ROUTINE:  Update_Views
 *
 * ABSTRACT: Update the windows that contain views of the picture
 *
 */
void Update_Views()
{
int x, y, width, height;
int i, i_array[4];		/* dummy variables */
/* Update display */
/*
		Find_Affected_Area( &x, &y, &width, &height, &i, i_array);
		Refresh_Picture( x, y, width, height );
*/
    Refresh_Picture (undo_x, undo_y, undo_width, undo_height);
}

/*
 *
 * ROUTINE:  Update_Zoom
 *
 * ABSTRACT: Update the zoom highlight after wysiwig drawing
 *
 */
void Update_Zoom()
{
int x, y, wd, ht;
int i, i_array[4];		/* dummy variables */

/* Refresh only what is necessary */
/*
	Find_Affected_Area( &x, &y, &wd, &ht, &i, i_array);
	if( ifocus != zoom_widget )
		Refresh_Zoom_View( x, y, wd, ht );
*/
    if (ifocus != zoom_widget)
	Refresh_Zoom_View (undo_x, undo_y, undo_width, undo_height);
    Refresh_Magnifier (undo_x, undo_y, undo_width, undo_height);
}
             
/*
 *
 * ROUTINE:  Draw_Line
 *
 * ABSTRACT: Draw a line into the picture
 *
 */
void Draw_Line()
{
    int i;
    Pixmap tmp_pix;    

    if( outline_stipple ){
	Save_Picture_State();

    	XDrawLine (disp, picture, Get_GC (GC_PD_OUTLINE), points[0].x, 
		   points[0].y, points[1].x, points[1].y);
	if (UG_num_used > 1) {
	    Set_Cursor_Watch (pwindow);
	    for (i = 1; i <= 4; i++) { 
		if (UG_image[UG_last + i] != 0) {
		    XSetTSOrigin (disp, Get_GC(GC_PD_OUTLINE), -UG_extra[i].x,
				  -UG_extra[i].y);
		    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
					     UG_extra[i].wd, UG_extra[i].ht,
					     pdepth);
		    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY),
			       UG_image[UG_last + i], 0, 0, 0, 0,
			       UG_extra[i].wd, UG_extra[i].ht);

		    XDrawLine (disp, tmp_pix, Get_GC (GC_PD_OUTLINE), 
			       points[0].x + (picture_x - UG_extra[i].x),
			       points[0].y + (picture_y - UG_extra[i].y),
			       points[1].x + (picture_x - UG_extra[i].x),
			       points[1].y + (picture_y - UG_extra[i].y));

		    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
				     UG_extra[i].ht, bit_plane, img_format,
				     picture_image,  UG_extra[i].x, 
				     UG_extra[i].y, ImgK_Src);
				
		    XFreePixmap (disp, tmp_pix);
		}
	    }
	    XSetTSOrigin (disp, Get_GC(GC_PD_OUTLINE), -picture_x, -picture_y);
	    Set_Cursor (pwindow, current_action);
	}

	Update_Views();
    }			
}    

/*
 *
 * ROUTINE:  Draw_Polyline
 *
 * ABSTRACT: Draws the intermediate polylines into the picture
 *
 */
void Draw_Polyline()
{
XPoint pts[3];
int npts;
int xmin, ymin, xmax, ymax;
int x, y, wd, ht;
int i;

    if( outline_stipple && (numpts > 1) ){
             
/* Connect the last two rubberband points */
	pts[0].x = points[numpts-1].x;
	pts[0].y = points[numpts-1].y;
	pts[1].x = points[numpts-2].x;
	pts[1].y = points[numpts-2].y;
	npts = 2;

/* If a brush is not used, plot last three points in order to get mitering*/
	if( !use_brush && (numpts > 2) ){
	    pts[2].x = points[numpts-3].x;
	    pts[2].y = points[numpts-3].y;
	    ++npts;
	}
			
/* cur_gc set in Init_Rband() */
/* if Drawing a Polygon, draw into pixmap as well */

	xmin = MIN( pts[0].x, pts[1].x );
	if (npts > 2)
	    xmin = MIN (xmin, pts[2].x);
	ymin = MIN( pts[0].y, pts[1].y );
	if (npts > 2)
            ymin = MIN (ymin, pts[2].y);
	xmax = MAX( pts[0].x, pts[1].x );
	if (npts > 2)
            xmax = MAX (xmax, pts[2].x);
	ymax = MAX( pts[0].y, pts[1].y );
	if (npts > 2)
            ymax = MAX (ymax, pts[2].y);
/* give polygon a clearance of 5 brush widths on either side */
	if (current_action == POLYGON) {
	    x = xmin - (brush_wd * 5);
	    y = ymin - (brush_wd * 5);
	    wd = xmax - xmin + (brush_wd * 10);
	    ht = ymax - ymin + (brush_wd * 10); 
	}
	else {
	    x = xmin - brush_half_wd;
	    y = ymin - brush_half_wd;
	    wd = xmax - xmin + brush_wd;
	    ht = ymax - ymin + brush_wd;
	}

	Continue_Save_Picture_State (x, y, wd, ht);
	if (current_action == POLYGON) {
	    XDrawLines (disp, picture, Get_GC (GC_PD_OUTLINE), pts, npts, 
			CoordModeOrigin);
	    if (zoomed)
		Refresh_Zoom_View( x, y, wd, ht );
	}
	else
            XDrawLines (disp, picture, Get_GC (GC_PD_STROKE), pts, npts,
                        CoordModeOrigin);

	for (i=0; i < npts; i++) {
	    pts[i].x = IX_TO_PWX (pts[i].x);
	    pts[i].y = IY_TO_PWY (pts[i].y);
	}

	if (current_action == POLYGON) {
            XDrawLines (disp, pwindow, cur_gc, pts, npts, CoordModeOrigin);
            if (zoomed)
		Refresh_Magnifier( x, y, wd, ht );
        }
        if (current_action == STROKE) {
            Refresh_Picture (x, y, wd, ht);
        }
    }
}    

/*
 *           
 * ROUTINE:  Draw_Polygon
 *
 * ABSTRACT: Draw a polyline/polygon into the picture
 *
 */
void Draw_Polygon()    
{
    int x, y, wd, ht, xmax, ymax;
    int i, j;
    int xoff, yoff;
    XPoint *pts;
    Pixmap tmp_pix;
    GC gc_fill, gc_outline;

    if (!fill_stipple && !outline_stipple)
	return;

    gc_fill = Get_GC (GC_PD_FILL);
    if (current_action == POLYGON)
	gc_outline = Get_GC(GC_PD_OUTLINE);
    else
	gc_outline = Get_GC(GC_PD_STROKE);

/* Draw into offscreen picture */
    if (fill_stipple) {
	XFillPolygon (disp, picture, gc_fill, points, numpts, Complex,
		      CoordModeOrigin);
    }
    if (outline_stipple) {
	XDrawLines (disp, picture, gc_outline, points, numpts, CoordModeOrigin);
    }

/* Refresh windows */

    if (current_action == POLYGON) {
	xmax = shape_xmax + brush_wd * 5;
	ymax = shape_ymax + brush_wd * 5;
	x = MAX (-picture_x, shape_xmin - brush_wd * 5);
	y = MAX (-picture_y, shape_ymin - brush_wd * 5);
    }
    else {
	xmax = shape_xmax + brush_wd - brush_half_wd;
	ymax = shape_ymax + brush_wd - brush_half_wd;
	x = MAX (-picture_x, shape_xmin - brush_half_wd);
	y = MAX (-picture_y, shape_ymin - brush_half_wd);
    }

    wd = MIN (xmax, IX_TO_PMX (pimage_wd)) - x;
    ht = MIN (ymax, IY_TO_PMY (pimage_ht)) - y;

    Refresh_Picture (x, y, wd, ht);    

    i = UG_num_used;
    Find_More_Affected_Rect (x, y, wd, ht, &UG_num_used, UG_used);

    if (i == UG_num_used)
	return;

    Set_Cursor_Watch (pwindow);
	    
    while (i < UG_num_used) {
	j = UG_used[i];
	UG_image[j] = XSubImage (picture_image, ui_x (j), ui_y (j),
                                 ui_wd (j), ui_ht (j));
	i++;
    } 

    pts = (XPoint *) XtMalloc (sizeof (XPoint) * numpts);

    for (i = 1; i <= 4; i++) { 
	if (UG_image[UG_last + i] != 0) {
	    XSetTSOrigin (disp, gc_fill, -UG_extra[i].x, -UG_extra[i].y);
	    XSetTSOrigin (disp, gc_outline, -UG_extra[i].x, -UG_extra[i].y);
	    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
				     UG_extra[i].wd, UG_extra[i].ht, pdepth);
	    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY), 
		       UG_image[UG_last + i], 0, 0, 0, 0,
		       UG_extra[i].wd, UG_extra[i].ht);

	    xoff = picture_x - UG_extra[i].x;
	    yoff = picture_y - UG_extra[i].y;

/* convert polygon into proper coordinates */
	    for (j = 0; j < numpts; j++) {
		pts[j].x = points[j].x + xoff;
		pts[j].y = points[j].y + yoff;
	    }

	    if (fill_stipple) {
		XFillPolygon (disp, tmp_pix, gc_fill, pts, numpts, Complex,
			      CoordModeOrigin);
	    }
	    if (outline_stipple) {
		XDrawLines (disp, tmp_pix, gc_outline, pts, numpts,
			    CoordModeOrigin);
	    }


	    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
			     UG_extra[i].ht, bit_plane, img_format,
			     picture_image,  UG_extra[i].x, 
			     UG_extra[i].y, ImgK_Src);
				
	    XFreePixmap (disp, tmp_pix);
	}
    }

    XSetTSOrigin (disp, gc_fill, -picture_x, -picture_y);
    XSetTSOrigin (disp, gc_outline, -picture_x, -picture_y);

    XtFree ((char *)pts);
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Draw_Stroke
 *
 * ABSTRACT: Draw stroke into the picture
 *
 */
void Draw_Stroke()
{
    Draw_Polygon();
}

/*
 *
 * ROUTINE:  Draw_Brush
 *
 * ABSTRACT: Draw a brush stroke into the picture
 *
 */
void Draw_Brush()
{
    int x, y, wd, ht, xmax, ymax;
    int i, j;
    int xoff, yoff;
    XPoint pts[2];
    Pixmap tmp_pix;

    xmax = shape_xmax + brush_wd - brush_half_wd;
    ymax = shape_ymax + brush_wd - brush_half_wd;
    x = MAX (-picture_x, shape_xmin - brush_half_wd);
    y = MAX (-picture_y, shape_ymin - brush_half_wd);
    wd = MIN (xmax, IX_TO_PMX (pimage_wd)) - x;
    ht = MIN (ymax, IY_TO_PMY (pimage_ht)) - y;

    i = UG_num_used;
    Find_More_Affected_Rect (x, y, wd, ht, &UG_num_used, UG_used);

    if (i == UG_num_used)
	return;

    Set_Cursor_Watch (pwindow);
	    
    while (i < UG_num_used) {
	j = UG_used[i];
	UG_image[j] = XSubImage (picture_image, ui_x (j), ui_y (j),
                                 ui_wd (j), ui_ht (j));
	i++;
    } 

    for (i = 1; i <= 4; i++) { 
	if (UG_image[UG_last + i] != 0) {
	    XSetTSOrigin (disp, cur_gc, -UG_extra[i].x, -UG_extra[i].y);
	    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
				     UG_extra[i].wd, UG_extra[i].ht, pdepth);
	    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY), 
		       UG_image[UG_last + i], 0, 0, 0, 0,
		       UG_extra[i].wd, UG_extra[i].ht);

	    xoff = picture_x - UG_extra[i].x;
	    yoff = picture_y - UG_extra[i].y;

/* initial click */
	    pts[0].x = points[0].x + xoff;
	    pts[0].y = points[0].y + yoff;
	    Simulate_Brush (tmp_pix, pts, 1);

/* brush stroke */
	    for (j = 1; j < numpts; j++) {
		x = MIN(points[j].x, points[j-1].x) - brush_half_wd + xoff;
		y = MIN(points[j].y, points[j-1].y) - brush_half_wd + yoff;
		wd = abs (points[j].x - points[j-1].x) + brush_wd;
		ht = abs (points[j].y - points[j-1].y) + brush_wd;
		if (Intersecting_Rectangles (0, 0, UG_extra[i].wd,
					     UG_extra[i].ht, x, y, wd, ht)) {
		    pts[0].x = points[j-1].x + xoff;
		    pts[0].y = points[j-1].y + yoff;
		    pts[1].x = points[j].x + xoff;
		    pts[1].y = points[j].y + yoff;
		    Simulate_Brush (tmp_pix, pts, 2);
		}
	    }
	    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
			     UG_extra[i].ht, bit_plane, img_format,
			     picture_image,  UG_extra[i].x, 
			     UG_extra[i].y, ImgK_Src);
				
	    XFreePixmap (disp, tmp_pix);
	}
    }
    XSetTSOrigin (disp, cur_gc, -picture_x, -picture_y);
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Draw_Erase
 *
 * ABSTRACT: Draw the erase stroke into the picture
 *
 */
void Draw_Erase()
{                  
    Draw_Brush();
}

/*
 *
 * ROUTINE:  Draw_Pencil
 *
 * ABSTRACT: Draw a pencil stroke into the picture
 *
 */
void Draw_Pencil ()
{     
    GC gc_pencil;
    int x, y, wd, ht, xmax, ymax;
    int i, j;
    int xoff, yoff;
    Pixmap tmp_pix;

    xmax = shape_xmax + 1;
    ymax = shape_ymax + 1;
    x = MAX (-picture_x, shape_xmin);
    y = MAX (-picture_y, shape_ymin);
    wd = MIN (xmax, IX_TO_PMX (pimage_wd)) - x;
    ht = MIN (ymax, IY_TO_PMY (pimage_ht)) - y;

    i = UG_num_used;
    Find_More_Affected_Rect (x, y, wd, ht, &UG_num_used, UG_used);

    if (i == UG_num_used)
	return;

    Set_Cursor_Watch (pwindow);
    gc_pencil = (GC) Get_Pencil_GC();

    while (i < UG_num_used) {
	j = UG_used[i];
	UG_image[j] = XSubImage (picture_image, ui_x (j), ui_y (j), ui_wd (j),
				 ui_ht (j));
	i++;
    } 

    for (i = 1; i <= 4; i++) { 
	if (UG_image[UG_last + i] != 0) {
	    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
				     UG_extra[i].wd, UG_extra[i].ht, pdepth);
	    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY),
		       UG_image[UG_last + i], 0, 0, 0, 0,
		       UG_extra[i].wd, UG_extra[i].ht);

	    xoff = picture_x - UG_extra[i].x;
	    yoff = picture_y - UG_extra[i].y;

/* initial click can't be off pixmap don't even attempt to draw it*/
	    
/* pencil stroke */
	    for (j = 1; j < numpts; j++) {
		    x = MIN(points[j].x, points[j-1].x) + xoff;
		    y = MIN(points[j].y, points[j-1].y) + yoff;
		    wd = abs (points[j].x - points[j-1].x) + 1;
		    ht = abs (points[j].y - points[j-1].y) + 1;
		    if (Intersecting_Rectangles (0, 0, UG_extra[i].wd,
						 UG_extra[i].ht,
						 x, y, wd, ht)) {
			XDrawLine (disp, tmp_pix, gc_pencil, 
				   points[j-1].x + xoff, points[j-1].y + yoff, 
				   points[j].x + xoff, points[j].y + yoff);
		    }
	    }
	    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
			     UG_extra[i].ht, bit_plane, img_format,
			     picture_image,  UG_extra[i].x, 
			     UG_extra[i].y, ImgK_Src);
				
	    XFreePixmap (disp, tmp_pix);
	}
    }
    Set_Cursor (pwindow, current_action);
}


/*
 *
 * ROUTINE:  Draw_Rectangle
 *
 * ABSTRACT: Draw a rectangle/square into the picture
 *
 */
void Draw_Rectangle()
{
    XPoint pts[5];
    int i, rx, ry;
    Pixmap tmp_pix;    

/*  Find_Rectangle( points[0].x, points[0].y, points[1].x, points[1].y ); */

    if ((rect_wd > 0) && (rect_ht > 0)) {
	Save_Picture_State();

/* Draw the rectangle into the offscreen picture */
	if( fill_stipple ){
	    if (outline_stipple) {
		XFillRectangle( disp, picture, Get_GC( GC_PD_FILL ), 
		                rect_x, rect_y, rect_wd, rect_ht);
	    }
	    else {
		if ((rect_wd > 2 * brush_wd) && (rect_ht > 2 * brush_wd))
		    XFillRectangle (disp, picture, Get_GC( GC_PD_FILL ),
				    rect_x + brush_wd, rect_y + brush_wd,
				    rect_wd - 2 * brush_wd, 
				    rect_ht - 2 * brush_wd);
	    }
	}
	if( outline_stipple ){
	    if ((rect_wd > 2 * cur_line_wd) && (rect_ht > 2 * cur_line_wd)) 
		XDrawRectangle (disp, picture, Get_GC (GC_PD_OUTLINE), 
		                rect_x + cur_line_wd / 2,
		                rect_y + cur_line_wd / 2,
                                abs(rect_wd - cur_line_wd),
		                abs(rect_ht - cur_line_wd));
	    else 
		XFillRectangle (disp, picture, Get_GC (GC_PD_OUTLINE),
				rect_x, rect_y, rect_wd, rect_ht);
	}

	if (UG_num_used > 1) {
	    Set_Cursor_Watch (pwindow);
	    for (i = 1; i <= 4; i++) { 
		if (UG_image[UG_last + i] != 0) {
		    XSetTSOrigin (disp, Get_GC (GC_PD_OUTLINE), 
				  -UG_extra[i].x, -UG_extra[i].y);
		    XSetTSOrigin (disp, Get_GC (GC_PD_FILL), 
				  -UG_extra[i].x, -UG_extra[i].y);
		    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
					     UG_extra[i].wd, UG_extra[i].ht,
					     pdepth);
		    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY),
			       UG_image[UG_last + i], 0, 0, 0, 0,
			       UG_extra[i].wd, UG_extra[i].ht);
		    rx = rect_x + (picture_x - UG_extra[i].x);
		    ry = rect_y + (picture_y - UG_extra[i].y);

		    if( fill_stipple ){
			if (outline_stipple) {
			    XFillRectangle (disp, tmp_pix, Get_GC (GC_PD_FILL), 
					    rx, ry, rect_wd, rect_ht);
			}
			else {
			    if ((rect_wd > 2 * brush_wd) && 
				(rect_ht > 2 * brush_wd))
				XFillRectangle (disp, tmp_pix, 
						Get_GC (GC_PD_FILL),
						rx + brush_wd, ry + brush_wd,
						rect_wd - 2 * brush_wd, 
						rect_ht - 2 * brush_wd);
			}
		    }
		    if( outline_stipple ){
			if ((rect_wd > 2 * cur_line_wd) && 
			    (rect_ht > 2 * cur_line_wd)) 
			    XDrawRectangle (disp, tmp_pix, 
					    Get_GC (GC_PD_OUTLINE), 
					    rx + cur_line_wd / 2,
					    ry + cur_line_wd / 2,
					    abs(rect_wd - cur_line_wd),
					    abs(rect_ht - cur_line_wd));
			else 
			    XFillRectangle (disp, tmp_pix, 
					    Get_GC (GC_PD_OUTLINE),
					    rx, ry, rect_wd, rect_ht);
		    }
		
		    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
				     UG_extra[i].ht, bit_plane, img_format,
				     picture_image,  UG_extra[i].x, 
				     UG_extra[i].y, ImgK_Src);
				
		    XFreePixmap (disp, tmp_pix);
		}
	    }
	    XSetTSOrigin (disp, Get_GC (GC_PD_OUTLINE), -picture_x, -picture_y);
	    XSetTSOrigin (disp, Get_GC (GC_PD_FILL), -picture_x, -picture_y);
	    Set_Cursor (pwindow, current_action);
	}
/* Refresh picture and zoom windows if necessary */
		Update_Views();

    }
}    

/*
 *
 * ROUTINE:  Draw_Ellipse
 *
 * ABSTRACT: Draw an ellipse/circle into the picture
 *
 */
void Draw_Ellipse()
{
    int i, rx, ry;
    Pixmap tmp_pix;    

    if((rect_wd > 0) && (rect_ht > 0)){
	Save_Picture_State();

/* Draw into offscreen picture */
/*	Find_Rectangle( points[0].x, points[0].y, points[1].x, points[1].y); */
	if (fill_stipple) {
	    if (outline_stipple) {
		if ((rect_wd > brush_wd) && (rect_ht > brush_wd)) {
		    XFillArc (disp, picture, Get_GC( GC_PD_FILL ),
			      rect_x + brush_half_wd, rect_y + brush_half_wd,
			      rect_wd - brush_wd, rect_ht - brush_wd,
			      0, 64*360);
 		}
	    }
	    else {
		if ((rect_wd > (2 * brush_wd)) && (rect_ht > (2 * brush_wd))) {
		    XFillArc (disp, picture, Get_GC (GC_PD_FILL), 
			      rect_x + brush_wd, rect_y + brush_wd, 
			      rect_wd - 2 * brush_wd, rect_ht - 2 * brush_wd,
			      0, 64*360);
		}
	    }
	}

	if( outline_stipple ){
	    if ((rect_wd > (2 * brush_wd)) && (rect_ht > (2 * brush_wd))) {
		XDrawArc (disp, picture, Get_GC (GC_PD_OUTLINE), 
			  rect_x + brush_half_wd, rect_y + brush_half_wd,
			  rect_wd - brush_wd, rect_ht - brush_wd, 0, 64*360);
	    }
	    else {
/* should be using routines to change these values but there is no routine to
   get gc values */
		XFillArc (disp, picture, Get_GC (GC_PD_OUTLINE), rect_x, rect_y,
			  rect_wd, rect_ht, 0, 64*360);
	    }
	}	

	if (UG_num_used > 1) {
	    Set_Cursor_Watch (pwindow);
	    for (i = 1; i <= 4; i++) { 
		if (UG_image[UG_last + i] != 0) {
		    XSetTSOrigin (disp, Get_GC (GC_PD_OUTLINE), 
				  -UG_extra[i].x, -UG_extra[i].y);
		    XSetTSOrigin (disp, Get_GC (GC_PD_FILL), 
				  -UG_extra[i].x, -UG_extra[i].y);
		    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
					     UG_extra[i].wd, UG_extra[i].ht,
					     pdepth);
		    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY),
			       UG_image[UG_last + i], 0, 0, 0, 0,
			       UG_extra[i].wd, UG_extra[i].ht);
		    rx = rect_x + (picture_x - UG_extra[i].x);
		    ry = rect_y + (picture_y - UG_extra[i].y);

		    if (fill_stipple) {
			if (outline_stipple) {
			    if ((rect_wd > brush_wd) && (rect_ht > brush_wd)) {
				XFillArc (disp, tmp_pix, Get_GC (GC_PD_FILL),
					  rx + brush_half_wd,
					  ry + brush_half_wd,
					  rect_wd - brush_wd, 
					  rect_ht - brush_wd, 0, 64*360);
			    }
			}
			else {
			    if ((rect_wd > (2 * brush_wd)) && 
				(rect_ht > (2 * brush_wd))) {
				XFillArc (disp, tmp_pix, Get_GC (GC_PD_FILL), 
					  rx + brush_wd, ry + brush_wd, 
					  rect_wd - 2 * brush_wd,
					  rect_ht - 2 * brush_wd, 0, 64*360);
			    }
			}
		    }

		    if( outline_stipple ){
			if ((rect_wd > (2 * brush_wd)) &&
			    (rect_ht > (2 * brush_wd))) {
			    XDrawArc (disp, tmp_pix, Get_GC (GC_PD_OUTLINE), 
				      rx + brush_half_wd, ry + brush_half_wd,
				      rect_wd - brush_wd, rect_ht - brush_wd,
				      0, 64*360);
			}
			else {
/* should be using routines to change these values but there is no routine to
   get gc values */
			    XFillArc (disp, tmp_pix, Get_GC (GC_PD_OUTLINE),
				      rx, ry, rect_wd, rect_ht, 0, 64*360);
			}
		    }	
		    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
				     UG_extra[i].ht, bit_plane, img_format,
				     picture_image,  UG_extra[i].x, 
				     UG_extra[i].y, ImgK_Src);
				
		    XFreePixmap (disp, tmp_pix);
		}
	    }
	    XSetTSOrigin (disp, Get_GC (GC_PD_OUTLINE), -picture_x, -picture_y);
	    XSetTSOrigin (disp, Get_GC (GC_PD_FILL), -picture_x, -picture_y);
	    Set_Cursor (pwindow, current_action);
	}
/* Refresh the windows */
	Update_Views();
    }

}

/*
 *
 * ROUTINE:  Draw_Arc
 *
 * ABSTRACT: Draw an arc into the picture           
 *
 */
void Draw_Arc()
{
    int i, rx, ry;
    Pixmap tmp_pix;    

    if ((arc_wd >> 0) && (arc_ht >> 0)) {
	Save_Picture_State();

/* Convert the points used for rubberbanding */
	if( ifocus == zoom_widget ) {
	    arc_x = ZX_TO_IX (arc_x);
	    arc_y = ZY_TO_IY (arc_y);
	    arc_wd = arc_wd / zoom_pixsize;
	    arc_ht = arc_ht / zoom_pixsize;
	}
	else{
	    arc_x = PWX_TO_IX (arc_x);
	    arc_y = PWY_TO_IY (arc_y);
	}

/* Draw into image */
	if (fill_stipple) {
	    XFillArc (disp, picture, Get_GC (GC_PD_FILL), arc_x, arc_y, 
		      arc_wd, arc_ht, arc_beg_angle, arc_end_angle);
	}
	if (outline_stipple) {
	    XDrawArc (disp, picture, Get_GC (GC_PD_OUTLINE), 
		      arc_x, arc_y, arc_wd, arc_ht, arc_beg_angle,
		      arc_end_angle);
	} 

	if (UG_num_used > 1) {
	    Set_Cursor_Watch (pwindow);
	    for (i = 1; i <= 4; i++) { 
		if (UG_image[UG_last + i] != 0) {
		    XSetTSOrigin (disp, Get_GC (GC_PD_OUTLINE), 
				  -UG_extra[i].x, -UG_extra[i].y);
		    XSetTSOrigin (disp, Get_GC (GC_PD_FILL), 
				  -UG_extra[i].x, -UG_extra[i].y);
		    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
					     UG_extra[i].wd, UG_extra[i].ht,
					     pdepth);
		    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY),
			       UG_image[UG_last + i], 0, 0, 0, 0,
			       UG_extra[i].wd, UG_extra[i].ht);

		    rx = arc_x + (picture_x - UG_extra[i].x);
		    ry = arc_y + (picture_y - UG_extra[i].y);

		    if (fill_stipple) {
			XFillArc (disp, tmp_pix, Get_GC (GC_PD_FILL), rx, ry, 
				  arc_wd, arc_ht, arc_beg_angle, arc_end_angle);
		    }
		    if (outline_stipple) {
			XDrawArc (disp, tmp_pix, Get_GC (GC_PD_OUTLINE), 
				  rx, ry, arc_wd, arc_ht, arc_beg_angle,
				  arc_end_angle);
		    } 

		    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
				     UG_extra[i].ht, bit_plane, img_format,
				     picture_image,  UG_extra[i].x, 
				     UG_extra[i].y, ImgK_Src);
				
		    XFreePixmap (disp, tmp_pix);
		}
	    }
	    XSetTSOrigin (disp, Get_GC (GC_PD_OUTLINE), -picture_x, -picture_y);
	    XSetTSOrigin (disp, Get_GC (GC_PD_FILL), -picture_x, -picture_y);
	    Set_Cursor (pwindow, current_action);
	}

/* Refresh the zoom and picture window if necessary */
	Update_Views();
    }
}

