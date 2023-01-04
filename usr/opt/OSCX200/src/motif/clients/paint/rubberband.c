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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/rubberband.c,v 1.1.2.2 92/12/11 08:35:56 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module RUBBERBAND "V1-000"
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
**   RUBBERBAND contains routines that do rubberbanding for all actions
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

#define HORIZONTAL 0
#define VERTICAL 1
#define QUADRANT_1 2
#define QUADRANT_2 3
#define QUADRANT_3 4
#define QUADRANT_4 5

static XPoint rbpts[10];
static int rbnum;
static int vertical_concave;
static int zoom_mult;
static int rb_brush_x1, rb_brush_y1;
static int rb_brush_x2, rb_brush_y2;
static int rb_brush_wd;
static int rb_brush_half_wd;


/*
 *
 * ROUTINE:  Complement_Theta
 *      
 * ABSTRACT: 
 *
 *  Find the complement of the given angle in the first quadrant
 *
 */
double Complement_Theta( theta )
double theta;
{
double mod_theta;
	if( theta <= 90.0 )
		mod_theta = 90 - theta;
	else
	if( theta <= 180.0 )
		mod_theta = 90 - (180.0 - theta);
	else
	if( theta < 270.0 )
		mod_theta = 270 - theta;
	else
		mod_theta = 90 - (360.0 - theta);

	return mod_theta;
}

/*
 *
 * ROUTINE:  Rband_Miter_Line
 *
 * ABSTRACT: 
 *
 *  Determine the outline of a mitered line of the current width
 *
 */        
void Rband_Miter_Line()  
{
    double Angle();
    double theta, mod_theta;               
    int x1, y1;

/* If width is small, just rubberband a single line */
    if (rb_brush_wd < 2) {
	rbpts[0].x = anchor_x;
	rbpts[0].y = anchor_y;
	rbpts[1].x = rband_x;
	rbpts[1].y = rband_y;                                  
	rbnum = 2;              
    }
    else {
/* Otherwise simulate the line by creating a polygon whose ends change
 * as the rubberbanded angle changes.
 */
	theta = Angle (anchor_x, anchor_y, rband_x, rband_y);
	mod_theta = Complement_Theta (theta);
	x1 = (cos (mod_theta/RADIANS_PER_DEGREE) * (float)rb_brush_half_wd)
	     + 0.5;
	y1 = (sin (mod_theta/RADIANS_PER_DEGREE) * (float)rb_brush_half_wd)
	     + 0.5;
/* First and third quadrant */
	if (((theta >= 0) && (theta <= 90)) || 
	    ((theta >= 180) && (theta <= 270))) {
	    rbpts[0].x = anchor_x + x1;
	    rbpts[0].y = anchor_y + y1;
	    rbpts[1].x = rband_x + x1;
	    rbpts[1].y = rband_y + y1;
	    rbpts[2].x = rband_x - x1;
	    rbpts[2].y = rband_y - y1;
	    rbpts[3].x = anchor_x - x1;
	    rbpts[3].y = anchor_y - y1;
	    rbpts[4].x = rbpts[0].x;
	    rbpts[4].y = rbpts[0].y;
	    rbnum = 5;
	}         
	else {
	    rbpts[0].x = anchor_x + x1;
	    rbpts[0].y = anchor_y - y1;
	    rbpts[1].x = rband_x + x1;
	    rbpts[1].y = rband_y - y1;
	    rbpts[2].x = rband_x - x1;
	    rbpts[2].y = rband_y + y1;
	    rbpts[3].x = anchor_x - x1;
	    rbpts[3].y = anchor_y + y1;
	    rbpts[4].x = rbpts[0].x;
	    rbpts[4].y = rbpts[0].y;
	    rbnum = 5;
	}
    }
/* jj-1/20/89 make sure not a degenerate rectangle */
    if (((rbpts[0].x != rbpts[1].x) || (rbpts[0].y != rbpts[1].y)) &&
	((rbpts[1].x != rbpts[2].x) || (rbpts[1].y != rbpts[2].y)))
	XDrawLines (disp, XtWindow(ifocus), gc_rband, rbpts, rbnum,
		    CoordModeOrigin);
}

/*
 *
 * ROUTINE:  Rband_Miter_Arc
 *
 * ABSTRACT: 
 *
 *  Rubber band the outline of mitered arc
 *
 */
void Rband_Miter_Arc()
{
int wd, ht;
int tmp_wd, tmp_ht;
int x1, y1, x2, y2;

/* If thin line, rubberband single arc */
	if( rb_brush_wd < 2 )
		XDrawArc( disp, XtWindow(ifocus), gc_rband, arc_x, arc_y,
		   arc_wd, arc_ht, arc_beg_angle, arc_end_angle);
	else
	if( ( abs(anchor_x - rband_x) <= rb_brush_half_wd ) ||
	    ( abs(anchor_y - rband_y) <= rb_brush_half_wd ) )
		Rband_Miter_Line();
	else{
/* Draw first arc */
			XDrawArc( disp, XtWindow(ifocus), gc_rband, arc_x-rb_brush_half_wd, 
		   arc_y-rb_brush_half_wd, arc_wd+rb_brush_wd, arc_ht+rb_brush_wd, 
			 arc_beg_angle, arc_end_angle);

/* Draw second arc */
			XDrawArc( disp, XtWindow(ifocus), gc_rband, arc_x+rb_brush_half_wd, 
		   arc_y+rb_brush_half_wd, arc_wd-rb_brush_wd, arc_ht-rb_brush_wd, 
			 arc_beg_angle, arc_end_angle);

/* ends */
			if( vertical_concave ){
				XDrawLine( disp, XtWindow(ifocus), gc_rband, anchor_x-rb_brush_half_wd,
					anchor_y, anchor_x+rb_brush_half_wd, anchor_y );
				XDrawLine( disp, XtWindow(ifocus), gc_rband, rband_x,
					 rband_y-rb_brush_half_wd, rband_x, rband_y+rb_brush_half_wd );
				}
			else{
				XDrawLine( disp, XtWindow(ifocus), gc_rband, anchor_x,
					anchor_y-rb_brush_half_wd, anchor_x, anchor_y+rb_brush_half_wd );
				XDrawLine( disp, XtWindow(ifocus), gc_rband, rband_x-rb_brush_half_wd,
					 rband_y, rband_x+rb_brush_half_wd, rband_y );

				}
			}              
}


/*
 *
 * ROUTINE:  Init_Rband
 *
 * ABSTRACT: Initializes variables used to rubberband shapes
 *
 */
void Init_Rband()
{                                                                        
	if( !rbanding ){
		shape_xmin = 32767;
		shape_xmax = -1;
		shape_ymin = 32767;
		shape_ymax = -1; 
		numpts = 0;
		Save_Point( cur_x, cur_y );
		anchor_x = cur_x;
		anchor_y = cur_y;
		rbanding = TRUE;
		}
	first_time = TRUE;

	rband_x = anchor_x;
	rband_y = anchor_y;

	if( current_action == ARC ){
		arc_wd = 0;
		arc_ht = 0;
		arc_x = 0;
		arc_y = 0;
		arc_beg_angle = 0;
		arc_end_angle = 0;
		}

 		if( (current_action == RECTANGLE) || (current_action == ELLIPSE) ||
		    (current_action == SELECT_RECT) ){
			rect_x = anchor_x;
			rect_y = anchor_y;
			rect_wd = 0;
			rect_ht = 0;
			}
     
		if( ifocus == zoom_widget )
			zoom_mult = zoom_pixsize;
		else
			zoom_mult = 1;     

		if( (current_action == BRUSH) ){
		   	brush_x1 = brushes[cur_brush ][ cur_brush_index].x1;
			brush_y1 = brushes[cur_brush ][ cur_brush_index].y1;
			brush_x2 = brushes[cur_brush ][ cur_brush_index].x2;
			brush_y2 = brushes[cur_brush ][ cur_brush_index].y2;
			brush_wd = brushes[cur_brush ][ cur_brush_index].wd;
			rb_brush_x1 = brush_x1 * zoom_mult;
			rb_brush_y1 = brush_y1 * zoom_mult;
			rb_brush_x2 = brush_x2 * zoom_mult;
			rb_brush_y2 = brush_y2 * zoom_mult;
			}
		else
		if( current_action == ERASE ){
			if( ifocus == zoom_widget )
				brush_wd = 2;
			else
				brush_wd = 16;
			}
		else
			brush_wd = cur_line_wd;


		brush_half_wd = brush_wd/2;
		rb_brush_wd = brush_wd * zoom_mult;
		rb_brush_half_wd = rb_brush_wd / 2;

/* WYSIWYG rubberbanding */
		if( current_action == BRUSH ){
			if( cur_brush == ROUND_BRUSH )
				cur_gc = Get_GC( GC_PD_ROUND_BRUSH );
			else
				cur_gc = Get_GC( GC_PD_SQUARE_BRUSH );
			}
		else
		if( current_action == ERASE )
			cur_gc = Get_GC( GC_PD_ERASER );
		else
		if( current_action == STROKE )
			cur_gc = Get_GC( GC_SD_STROKE );
		else
		if( current_action == POLYGON )
			cur_gc = Get_GC( GC_SD_OUTLINE );


}

/*
 *
 * ROUTINE:  Find_Quadrant
 *
 * ABSTRACT: 
 *
 * Find which quadrant a given point is in relative to the anchor point
 * anchor_x, anchor_y
 *
 */             
int Find_Quadrant( x, y )
int x, y;
{            
double angle;
extern double Angle();

	angle = Angle( anchor_x, anchor_y, x, y );
	if( (angle >= 0) && (angle < 90))                                   
		return( QUADRANT_1 );
	else
	if( (angle >=90) && (angle < 180))
		return( QUADRANT_2 );
	else                                                      
	if( (angle >=180) && (angle < 270))
		return( QUADRANT_3 );
	else
	if( (angle >=270) && (angle < 360))
		return( QUADRANT_4 );
}

/*
 *
 * ROUTINE:  Find_Direction
 *
 * ABSTRACT: 
 *
 * Given the angle, return which direction the user wants to draw the line.
 *
 */
int Find_Direction( angle )
double angle;
{             
	if( ((angle >= 0.0) && (angle <= 25.0)) ||
			((angle >= 335.0) && (angle <= 360.0)) ||
			((angle >= 155.0) && (angle <= 205.0)) )
		return(HORIZONTAL); /* horizontal line */
	else       
	if( ((angle >= 65.0) && (angle <= 115.0)) ||
			((angle >= 245.0) && (angle <= 295.0)) )
		return(VERTICAL); /* vertical line */
	else
	if( (angle > 25.0) && (angle < 65.0))
		return(QUADRANT_1); /* 45 degrees */
	else
	if( (angle > 115.0) && (angle < 155.0))
		return(QUADRANT_2); /* 135 degrees */
	else
	if( (angle > 205.0) && (angle < 245.0))
		return(QUADRANT_3); /* 225 degrees */
	else
	if( (angle > 295.0) && (angle < 335.0))
		return(QUADRANT_4); /* 315 degrees */
}

/*
 *
 * ROUTINE:  Find_Chord
 *
 * ABSTRACT: 
 *
 * Given the point xpt, ypt, and quadrant find the chord that will close
 * the arc.
 *
 */
void Find_Chord( qd, xpt, ypt )
int qd, xpt, ypt;
{
	chord_x1 = points[0].x;
	chord_y1 = points[0].y;
	if( (qd == QUADRANT_1) || (qd == QUADRANT_4) ){
		chord_x2 = points[0].x + arc_wd/2;
		chord_y2 = ypt;
		}
	else{ /* (qd == QUADRANT_2) or (qd == QUADRANT_3) */
		chord_x2 = points[0].x - arc_wd/2;
		chord_y2 = ypt;
		}
}
/*
 *
 * ROUTINE:  Find_Arc
 *
 * ABSTRACT: 
 *
 * Given the point xpt, ypt, return the dimensions to rubberband an
 * arc to that point.  x3 and y3 are the unconstrained mouse coordinates - use
 * them to determine the concavity of the arc.
 */
void Find_Arc( x1, y1, x2, y2, x3, y3 )
int x1, y1, x2, y2, x3, y3;
{
int quadrant; /* jj-port */
extern double Distance();
extern double Angle();
int prv_x, prv_y;
int direction;
double angle;
double dist;
int mult;
double change_dist;  /* distance from anchor in which concavity can change */

/* An arc can be drawn between points in two ways:
 *       +*             *+
 *        *           *  
 *       *      or   * 
 *		+*            *+
 *
 * Decide which way to draw the arc by finding the sub-quadrant
 * where the pointer is located.  After a given distance
 * don't make the decision anymore, use the current direction.
 * A vertical concave arc is one which is concave against the y-axis.
 * A horizontal concave arc is one which is concave against the x-axis.
 *
 *                     vertical
 *                    .         .
 *                      .  |  .
 *                        .|.
 *       horizontal--------+--------concave
 *                        .|.
 *                      .  |  .
 *                    .    |    .
 *                      concave
 */

/* if rubberbanding in zoomed mode, multiply the distance by the size
 * of a magnified pixel.
 */          
	if( (ifocus == zoom_widget) && rbanding )
		mult = zoom_pixsize;
	else
		mult = 1;
	if (grid_on)
	    change_dist = 1.5 * grid_size;
	else 
	    change_dist = 10.0;
	dist = Distance( x1, y1, x2, y2);
	if( dist <= (change_dist * mult)){
		angle = Angle( x1, y1, x3, y3 );
		if( ((angle > 0) && (angle <= 45)) ||
				((angle > 135) && (angle <= 225)) ||
				((angle > 315) && (angle <= 360)) )
			vertical_concave = FALSE;
		else
			vertical_concave = TRUE;
		}

	quadrant = Find_Quadrant( x2, y2 ); /* jj-port */

	arc_wd = 2 * abs( x2 - x1 );
	arc_ht = 2 * abs( y2 - y1 );
	if( vertical_concave ){
		if( quadrant == QUADRANT_1 ){  /* jj-port */
			arc_x = x1;
			arc_y = y2;
			arc_beg_angle = 64*90;
			arc_end_angle = 64*90;
			}
		else
		if( quadrant == QUADRANT_2 ){  /* jj-port */
			arc_x = x2 - ( x1 - x2);
			arc_y = y2;
			arc_beg_angle = 64*0;
			arc_end_angle = 64*90;
			}
		else
		if( quadrant == QUADRANT_3 ){  /* jj-port */
			arc_x = x2 - ( x1 - x2);
			arc_y = y1 - (y2 - y1 );
			arc_beg_angle = 64*270;
			arc_end_angle = 64*90;
		 	}
		else
		if( quadrant == QUADRANT_4 ){  /* jj-port */
			arc_x = x1;
			arc_y = y1 - (y2 - y1 );
			arc_beg_angle = 64*180;
			arc_end_angle = 64*90;
		 	}
		}
	else{	/* horizontal concave */
		if( quadrant == QUADRANT_1 ){  /* jj-port */
			arc_x = x1 - ( x2 - x1 ); 
			arc_y = y2 - (y1 - y2);
			arc_beg_angle = 64*270;
			arc_end_angle = 64*90;
			}                    
		else
		if( quadrant == QUADRANT_2 ){  /* jj-port */
			arc_x = x1 - ( x1 - x2 ); 
			arc_y = y2 - (y1 - y2);
			arc_beg_angle = 64*180;
			arc_end_angle = 64*90;
			}
		else
		if( quadrant == QUADRANT_3 ){  /* jj-port */
			arc_x = x2;
			arc_y = y1;
			arc_beg_angle = 64*90;
			arc_end_angle = 64*90;
			}
		else
		if( quadrant == QUADRANT_4 ){  /* jj-port */
			arc_x = x1 - ( x2 - x1 );
			arc_y = y1;
			arc_beg_angle = 64*0;
			arc_end_angle = 64*90;
			}
		}
/* jj-port
	Find_Chord( quadrant, x2, y2 );
*/
}


/*
 *
 * ROUTINE:  Constrain_HV
 *
 * ABSTRACT: 
 *
 * Constrains the current x,y cooridinate to either only horizontal or
 * vertical.  This decision is made depending on the location of the
 * second point in the points buffer.
 *
 */
void Constrain_HV()
{
double angle;
static int direction;
extern double Angle();

	if( numpts == 1 ){
		angle = Angle( anchor_x, anchor_y, cur_x, cur_y );
		direction = Find_Direction( angle );
		}
	if( numpts >= 1 ){
		if( direction == HORIZONTAL )/* horizontal */
			cur_y = anchor_y;
		else /* vertical */
			cur_x = anchor_x;
		}
}

/*
 *
 * ROUTINE:  Constrain_Line
 *
 * ABSTRACT: 
 *
 * Returns the variables rband_x, and rband_y.  The rubberband point
 * is constrained to draw a vertical, horizontal, 45, 135, 225, or 315
 * angle line.
 *
 */
void Constrain_Line()
{
double angle;
static int direction;
double tmp;
extern double Distance();
extern double Angle();

	angle = Angle( anchor_x, anchor_y, cur_x, cur_y );
	direction = Find_Direction( angle );

	if( direction == HORIZONTAL ){/* horizontal */
		rband_x = cur_x;                      
		rband_y = anchor_y;
		}
	else
	if( direction == VERTICAL ){/* vertical */
		rband_x = anchor_x;
		rband_y = cur_y;
		}
	else{ /* 45 degrees */
		tmp = MAX( abs(cur_x -anchor_x), abs(cur_y-anchor_y));
		if( direction == QUADRANT_1 ){ /* 45 degrees */
			rband_y = anchor_y - tmp; 
			rband_x = anchor_x + tmp;
			}
		if( direction == QUADRANT_2 ){ /* 135 degrees */
			rband_y = anchor_y - tmp; 
			rband_x = anchor_x - tmp;
			}
		if( direction == QUADRANT_3 ){ /* 225 degrees */
			rband_y = anchor_y + tmp; 
			rband_x = anchor_x - tmp;
			}
		if( direction == QUADRANT_4 ){ /* 315 degrees */
			rband_y = anchor_y + tmp; 
			rband_x = anchor_x + tmp;
			}
		}
}


/*
 *
 * ROUTINE:  Rband_Line
 *
 * ABSTRACT: 
 *
 *  Rubber band a line
 *
 */
void Rband_Line()
{
             
/* Erase old line */ 
	if( !first_time ){
/*
		if( use_brush )
			Rband_Brush_Line();
		else
*/
			Rband_Miter_Line();
		}

		if( alt_shape )
			Constrain_Line();
		else{
			rband_x = cur_x;
			rband_y = cur_y;
			}

/*
		if( use_brush )
			Rband_Brush_Line();
		else
*/
			Rband_Miter_Line();

		first_time = FALSE;
}


/*
 *
 * ROUTINE:  Rband_Polygon
 *
 * ABSTRACT: 
 *
 *  Rubber band a polygon
 *
 */
void Rband_Polygon()
{
	Rband_Line();
}

/*
 *
 * ROUTINE:  Rband_Miter_Rectangle
 *
 * ABSTRACT: 
 *
 *  Determine the outline of a mitered rectangle of the current width
 *                        
 */
static void Rband_Miter_Rectangle()  
{
/* Draw outer rectangle */
	XDrawRectangle( disp, XtWindow(ifocus), gc_rband, rect_x, rect_y, rect_wd,
		 rect_ht );

/* Draw inner rectangle */
	if( (rect_wd > 2*rb_brush_wd) && (rect_ht > 2*rb_brush_wd) &&
	    (rb_brush_wd > 1) )
		XDrawRectangle( disp, XtWindow(ifocus), gc_rband, rect_x+rb_brush_wd, 
		 rect_y + rb_brush_wd, rect_wd - 2*rb_brush_wd, rect_ht - 2*rb_brush_wd);
}


/*
 *
 * ROUTINE:  Rband_Rectangle
 *
 * ABSTRACT: 
 *
 *  Rubber band a rectangle
 *
 */
void Rband_Rectangle()
{    
   
/* Erase old rectangle */

	if( !first_time ){
/*
		if( use_brush )
			Rband_Brush_Rectangle();
		else
*/
			Rband_Miter_Rectangle();
		}        

		rband_x = cur_x;
		rband_y = cur_y;
                    
/* Draw new rectangle */
		Find_Rectangle( anchor_x, anchor_y, rband_x, rband_y );

/*
		if( use_brush )
			Rband_Brush_Rectangle();
		else
*/
			Rband_Miter_Rectangle();

		first_time = FALSE;
}

/*
 *
 * ROUTINE:  Rband_Box
 *
 * ABSTRACT: 
 *
 *  Rubber band a thin line rectangle
 *
 */
void Rband_Box()
{    
   
/* Erase old rectangle */

	if( !first_time ){
		XDrawRectangle( disp, XtWindow(ifocus), gc_rband, rect_x, rect_y, rect_wd,
		 rect_ht );
		}        

		rband_x = cur_x;
		rband_y = cur_y;
                    
/* Draw new rectangle */
	Find_Rectangle( anchor_x, anchor_y, rband_x, rband_y );
	XDrawRectangle( disp, XtWindow(ifocus), gc_rband, rect_x, rect_y, rect_wd,
	 rect_ht );

	first_time = FALSE;
}

/*
 *
 * ROUTINE:  Rband_Miter_Ellipse
 *
 * ABSTRACT: 
 *
 *  Determine the outline of a mitered ellipse of the current width
 *                        
 */
static void Rband_Miter_Ellipse()
{
/* Draw outer ellipse */
	XDrawArc( disp, XtWindow(ifocus), gc_rband, rect_x, rect_y, rect_wd,
		rect_ht, 0, 64*360);

/* Draw inner ellipse */
	if( (rect_wd > 2*rb_brush_wd) && (rect_ht > 2*rb_brush_wd) &&
	    (rb_brush_wd > 1) )
		XDrawArc( disp, XtWindow(ifocus), gc_rband, rect_x+rb_brush_wd, 
		 rect_y + rb_brush_wd, rect_wd - 2*rb_brush_wd, rect_ht - 2*rb_brush_wd,
			0, 64*360);

}

/*
 *
 * ROUTINE:  Rband_Ellipse
 *
 * ABSTRACT: 
 *
 *  Rubber band an ellipse
 *
 */
void Rband_Ellipse()
{

/*
printf( "x= %d, y= %d\n", x, y );
*/  
	if( !first_time ){
/*
		if( use_brush )
			Rband_Brush_Ellipse();
		else
*/
			Rband_Miter_Ellipse();
		}

		rband_x = cur_x;
		rband_y = cur_y;
                    
/* Draw new circle */
		Find_Rectangle( anchor_x, anchor_y, rband_x, rband_y );
/*
		if( use_brush )
			Rband_Brush_Ellipse();
		else
*/
			Rband_Miter_Ellipse();

		first_time = FALSE;
}

/*
 *
 * ROUTINE:  Rband_Arc
 *
 * ABSTRACT: 
 *
 *  Rubber band an arc
 *
 */
void Rband_Arc()
{
    int a, b, dist;

/* Draw old arc */
    if( !first_time ){
/*
	if( use_brush )
	    Rband_Brush_Arc();
	else
*/
	    Rband_Miter_Arc();
    }

    rband_x = cur_x;
    rband_y = cur_y;
    if (alt_shape) {
	a = abs (anchor_x - rband_x);   
	b = abs (anchor_y - rband_y);
	dist = MAX (a,b);

	if (rband_x < anchor_x)
	    rband_x = anchor_x - dist;
	else
	    rband_x = anchor_x + dist;

	if (rband_y < anchor_y)
	    rband_y = anchor_y - dist;
	else
	    rband_y = anchor_y + dist;
    }

/* Draw new arc */
    Find_Arc( anchor_x, anchor_y, rband_x, rband_y, true_ptr_x, true_ptr_y);
/*
    if( use_brush )
    	Rband_Brush_Arc();
    else
*/
	Rband_Miter_Arc();

    first_time = FALSE;

}


/*
 *
 * ROUTINE:  Rband_Move
 *
 * ABSTRACT: 
 *
 *  Rubberband the area that the user has selected
 *
 */
void Rband_Move()
{    
int i;
int x, y, width, height;
int prv_x, prv_y;
Pixmap tmpmap;

/* Rubber band the selected area by restoring the part of the picture
 * that gets uncovered while the user is moving the selected piece around.  
 * For example, if the user has moved a selected rectangle in this way:
 *
 *			 x......... new
 *			 x.       . 
 *			--------  .
 *			|x.     | .
 *			|x.     | .
 *	 		|x......| .
 *	 		|xxxxxxx| 
 *	old	--------
 *
 * Only the space marked with x's gets copied from the backup picture.
 */
    if( ifocus == zoom_widget ){
	/* x,y Distance moved - converted to unzoomed coordinates */
	xdist = (cur_x - rband_x)/zoom_pixsize;
	ydist = (cur_y - rband_y)/zoom_pixsize;
    }
    else{
/* x,y Distance moved */
	xdist = cur_x - rband_x;
	ydist = cur_y - rband_y;
    }

/* Transfer copymap to the picture window */
    Copy_Select_To_Screen (select_x+xdist, select_y+ydist);

/* Now refresh the areas of the picture that have been uncovered */
    if( (abs(ydist) > select_height) || (abs(xdist) > select_width) )
	Refresh_Picture( select_x, select_y, select_width, select_height);
    else{			
	if( ydist < 0 )
	    Refresh_Picture (select_x, select_y+select_height + ydist, 
			     select_width, abs(ydist) );

	if( ydist > 0 )                       
	    Refresh_Picture (select_x, select_y, select_width, ydist );

	if( xdist < 0 )
	    Refresh_Picture (select_x + select_width + xdist, select_y, 
			     abs(xdist), select_height );
     
	if( xdist > 0 )
	    Refresh_Picture( select_x, select_y, xdist, select_height );
    }      
/* Only refresh zoom window if events are occuring in zoom window */

    if (zoomed) { 
	Refresh_Magnifier (MIN ((select_x + xdist), select_x),
			   MIN ((select_y + ydist), select_y),
			   select_width + abs (xdist),
			   select_height + abs (ydist));
	if (ifocus == zoom_widget) {
	    refresh_zoom_from = FROM_COPYMAP;
	    Refresh_Zoom_View (select_x + xdist, select_y + ydist, select_width,
			       select_height);
	    refresh_zoom_from = FROM_PICTURE;
	}
    }

/* Update variables for next time around */
    rband_x = cur_x;
    rband_y = cur_y;
    select_x += xdist;
    select_y += ydist;
    moved_xdist += xdist;
    moved_ydist += ydist;

}
                         
/*
 *
 * ROUTINE:  Rband_Brush
 *
 * ABSTRACT: 
 *
 * Brush stroke in the picture
 *
 */
void Rband_Brush()
{
XPoint pts[2];
int x, y;
int wd, ht;
int xmin, xmax, ymin, ymax;


    if (first_time) {
	Simulate_Brush (picture, points, 1);
	first_time = FALSE;

/* necessary for refresh */
	x = points[0].x - brush_half_wd;
	y = points[0].y - brush_half_wd;
	wd = brush_wd;
	ht = brush_wd;
    }

    else{
	if( alt_shape ) {
	    Constrain_HV();
	}
	Save_Point (cur_x, cur_y);

	xmin = MIN (points[numpts-2].x, points[numpts-1].x);
	xmax = MAX (points[numpts-2].x, points[numpts-1].x);
	ymin = MIN (points[numpts-2].y, points[numpts-1].y);
	ymax = MAX (points[numpts-2].y, points[numpts-1].y);
	x = xmin - brush_half_wd;
	y = ymin - brush_half_wd;
	wd = xmax - xmin + brush_wd;
	ht = ymax - ymin + brush_wd;
	Continue_Save_Picture_State (x, y, wd, ht);

/* Draw brush into picture */
	Simulate_Brush (picture, &(points[numpts-2]), 2);
    }        

    Refresh_Picture( x, y, wd, ht );

/* Refresh Zoom window if necessary */
    if( ifocus == zoom_widget )
	Refresh_Zoom_View( x, y, wd, ht );

    rband_x = cur_x;
    rband_y = cur_y;
}

/*
 *
 * ROUTINE:  Rband_Stroke
 *
 * ABSTRACT: 
 *
 *  Rubber band a stroke
 *
 */
void Rband_Stroke()
{
extern double Distance();
int x, y;
int wd, ht;
int xmin, xmax, ymin, ymax;
int mult;

	if (ifocus == zoom_widget)
	    mult = 8;
	else
	    mult = 1;

	if (Distance( cur_x, cur_y, rband_x, rband_y ) > (2.0 * mult)) {
		Save_Point( cur_x, cur_y );
		Draw_Polyline();
		rband_x = cur_x;
		rband_y = cur_y;
	}
}

/*
 *
 * ROUTINE:  Rband_Pencil
 *
 * ABSTRACT: 
 *
 *  Rubber band a pencile stroke
 *
 */
void Rband_Pencil()
{
	int xpt, ypt;
	int wd, ht;
	int x, y;
	int xmin, xmax, ymin, ymax;
	static GC rb_gc_pencil;
	GC gc_pencil;
	XPoint pts[2];

    if( first_time ){
	rb_gc_pencil = (GC) Get_Rband_Pencil_GC(); /* jj-port */
	XDrawPoint (disp, pwindow, rb_gc_pencil, IX_TO_PWX(points[0].x), 
		    IY_TO_PWY( points[0].y) );

	gc_pencil = (GC) Get_Pencil_GC();
        XDrawPoint( disp, picture, gc_pencil, points[0].x, points[0].y );
        if (zoomed) {
            Refresh_Zoom_View (points[0].x, points[0].y, 1, 1);
            Refresh_Magnifier (points[0].x, points[0].y, 1, 1);
        }

	first_time = FALSE;
    }
    else{
        if( alt_shape )
	    Constrain_HV();

	Save_Point (cur_x, cur_y);

	XSetForeground (disp, rb_gc_pencil, pencil_value);
	XDrawLine (disp, pwindow, rb_gc_pencil, 
		   IX_TO_PWX (points[numpts-2].x), 
		   IY_TO_PWY (points[numpts-2].y), 
		   IX_TO_PWX (points[numpts-1].x), 
		   IY_TO_PWY (points[numpts-1].y));

	x = MIN (points[numpts-2].x, points[numpts-1].x);
	xmax = MAX (points[numpts-2].x, points[numpts-1].x);
	y = MIN (points[numpts-2].y, points[numpts-1].y);
	ymax = MAX (points[numpts-2].y, points[numpts-1].y);
	wd = xmax - x + 1;
	ht = ymax - y + 1;

	Continue_Save_Picture_State (x, y, wd, ht);

	gc_pencil = (GC) Get_Pencil_GC();
	XDrawLine (disp, picture, gc_pencil, points[numpts-2].x, 
		   points[numpts-2].y, points[numpts-1].x, points[numpts-1].y);

	if (zoomed) {
            Refresh_Zoom_View (x, y, wd, ht);
            Refresh_Magnifier (x, y, wd, ht);
        }
    }
}

/*
 *
 * ROUTINE:  Rband_Area
 *
 * ABSTRACT: 
 *
 *  Rubber band a select area action
 *
 */
void Rband_Area()
{
extern double Distance();

	if( Distance( rband_x, rband_y, cur_x, cur_y ) > 2.0 ){
		Save_Point( cur_x, cur_y );
		XDrawLine( disp, XtWindow(ifocus), gc_rband, rband_x, rband_y, cur_x, cur_y );
		rband_x = cur_x;
		rband_y = cur_y;
		}
}


/*
 *
 * ROUTINE:  Random
 *
 * ABSTRACT: 
 *
 *  Produces a random number between 0 and SPRAY_RADIUS
 *
 */
int Random()
{
int num;

/* if the random number is not in the allowable range, divide by two
 * until an acceptable number is found 
 */
	num = rand();
	while(1){
		if( num <= SPRAY_DIAMETER )
			break;
		else
			num = num >> 1;
		}
return( num );
}			

/*
 *
 * ROUTINE:  Rband_Zoom_Move
 *
 * ABSTRACT: 
 *
 *  Rubberbands the zoom region
 *
 */
void Rband_Zoom_Move()
{                             

/* Erase old box */
	if( !first_time )
	  XDrawRectangle( disp, pwindow, gc_highlight, 
	   IX_TO_PWX(zoom_xorigin) + zoom_move_xdist -1, 
	   IY_TO_PWY(zoom_yorigin) + zoom_move_ydist -1, 
	   zoom_width+1, zoom_height+1 );

  zoom_move_xdist = cur_x - rband_x;
  zoom_move_ydist = cur_y - rband_y;

/* Draw new rectangle */
  XDrawRectangle( disp, pwindow, gc_highlight, 
   IX_TO_PWX(zoom_xorigin) + zoom_move_xdist -1,
   IY_TO_PWY(zoom_yorigin) + zoom_move_ydist -1,
   zoom_width+1, zoom_height+1 );

	first_time = FALSE;

}
/*
 *           
 * ROUTINE:  Echo_Action
 *
 * ABSTRACT: 
 *
 *  Depending on the current action, call appropriate routine to rubberband.
 *
 */          
void Echo_Action()
{

	switch ( current_action ) {
	case LINE : {
			Rband_Line();		/* rubber band a line */
		 	break;                      
			}

	case STROKE :{
			Rband_Stroke();		/* rubber band a stroke */
			break;
			}
             
	case CROP :
	case SELECT_RECT : {
			Rband_Box();
			break;
			}

	case SQUARE :
	case RECTANGLE : {   
			Rband_Rectangle();	/* rubber band a rectangle */
			break;
			}

	case CIRCLE :
	case ELLIPSE : {          
			Rband_Ellipse();	/* rubber band an ellipse */
			break;
	 		}

	case ARC : {          
			Rband_Arc();	/* rubber band an arc */
			break;
			}

	case POLYGON : {          
			Rband_Polygon();	/* rubber band a polygon */
			break;
		 	}
                                  
	case ERASE :
	case BRUSH : {
			Rband_Brush();	/* Brush */
			break;
			}
      
	case SELECT_AREA : {
			Rband_Area();
			break;
			}

	case PENCIL : {
			Rband_Pencil();	/* Pencil */
			break;
			}
      
	case MOVE : {
			Rband_Move();		/* Rubberband the move */
			break;
			}

	case SPRAYCAN : {
			Rband_Spray();
			break;
			}

  case ZOOM_MOVE : {
      Rband_Zoom_Move();
      break;
      }
	}
	
}
/*
 *    
 * ROUTINE:  Erase_Rband
 *
 * ABSTRACT: 
 *  Depending on the current action, stop rubberbanding and
 *  return the picture to its previous state if necessary
 *
 */
void Erase_Rband()
{
int i;
int x, y, width, height;
int beg_angle, end_angle;

	switch ( current_action ) {
		case POLYGON :
		case LINE : {
/*			if( use_brush )
				Rband_Brush_Line();
			else
*/
				Rband_Miter_Line();
			rbanding = FALSE;
		 	break;                   
			}

		case SQUARE :
		case RECTANGLE : {
/*
			if( use_brush )
				Rband_Brush_Rectangle();
			else
*/
				Rband_Miter_Rectangle();
			rbanding = FALSE;
		 	break;                   
			}

		case CROP : {
			XDrawRectangle( disp, XtWindow(ifocus), gc_rband, rect_x, rect_y,
				rect_wd, rect_ht );
			crop_rectangle = FALSE;
			break;
			}

		case TEXT :
		case SELECT_RECT : {
			XDrawRectangle( disp, XtWindow(ifocus), gc_rband,
					rect_x, rect_y, rect_wd, rect_ht );
			rbanding = FALSE;
			break;
			}

		case SELECT_AREA : {
		    for( i = 1; i < numpts; ++i )
			if (ifocus == zoom_widget)
			    XDrawLine (disp, XtWindow(ifocus), gc_rband,
				       IX_TO_ZX (points[i-1].x), 
				       IY_TO_ZY (points[i-1].y),
				       IX_TO_ZX (points[i].x),
				       IY_TO_ZY (points[i].y));
			else
			    XDrawLine (disp, XtWindow(ifocus), gc_rband,
				       IX_TO_PWX (points[i-1].x), 
				       IY_TO_PWY (points[i-1].y),
				       IX_TO_PWX (points[i].x),
				       IY_TO_PWY (points[i].y));
			rbanding = FALSE;
			break;
			}      

		case CIRCLE :
		case ELLIPSE : {          
/*
			if( use_brush )
				Rband_Brush_Ellipse();
			else
*/
				Rband_Miter_Ellipse();
			rbanding = FALSE;
			break;
		 	}

		case ZOOM_MOVE : {
			XDrawRectangle( disp, pwindow, gc_highlight, 
			   IX_TO_PWX(zoom_xorigin) + (cur_x-anchor_x)-1, 
			   IY_TO_PWY(zoom_yorigin) + (cur_y-anchor_y)-1, 
			   zoom_width+1, zoom_height+1 );
			break;
			}

		case ARC : {          
/*
			if( use_brush )
				Rband_Brush_Arc();
			else
*/
				Rband_Miter_Arc();
			rbanding = FALSE;
			break;
		 	}

	}
}
