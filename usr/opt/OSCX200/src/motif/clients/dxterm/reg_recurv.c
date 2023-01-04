/*****	RECURV ( ENVIRONMENT(EIS), IDENT= '1.006')  *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	recurv.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
*	RECURV.BLI	Regis Curve Routines 
*
*	FACILITY: Common Regis Parser
*
*	ABSTRACT: Routines related to drawing curves.
*	
*	ENVIRONMENT: Uses routines MULTI_LINE and DRAW_LINE
*
*	AUTHOR: Cindy Cyr	CREATION DATE: 20-Nov-1981
*	MODIFIED BY:  Baldwin Cheung
*		      Andy Vesper
*
*	MODIFICATION HISTORY:
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
* Tom Porcher		26-Apr-1988	X0.4-12
*	- Remove & on array reference for Ultrix.
*
*	Edit	006	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h.
*
*	Edit	005	??-???-??	/RFD
* Translated the origional Bliss code into C.
*
*	Edit	004	11-Aug-83	/AFV
*  Change cdraw to store the interpolation points on the 
*  stack (in LOCAL VECTORs) and  call DRAW_MULTI_LINE
*  instead of DRAW_LINE
*
*	Edit	003	19-Apr-83
* Change from GIDIS facility to REGIS -- drop support for Arcs and circles
*
*	Edit	002	28-Jan-82
* Fix relative addressing for open and closed curves
*
*	Edit	001	13-Jan-82
* Make this part of base level X1 release add documentation
*
*	Edit	000	17-Dec-81
* Enter curve routine code
**/

#include "gidcalls.h"
#include "regstruct.h"

#define	 NOS_NUMBER_OF_SEGMENTS	9

/*****			E X T E R N A L    F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      FILE   */
/*	--------		-----------			      ----   */
extern
    draw_line(),		/*				      REDRAW */
    draw_multi_line();		/*				    MULTILINE*/
#if SEPARATE_CURRENT_POSITION 
extern
    rpg_request_gidi_pos();		/* Request and read current	      REPORT */
#endif				/*	GIDIS position			     */

/******		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/*									     */
/*	FUNCTION			DESCRIPTION		       VALUE */
/*	--------			-----------		       ----- */
/**
*	cdraw(v,v,v)							no
*	crv_begin(v)							no
*	crv_continue(v,v)						no
*	crv_end()							no
*	draw_arc(v,v,v,v,v)						no
**/

    /*		TRIGONOMETRIC interpolation table	*/
    static	int	blending_table[9] [4] =
    {
	{-9, 125, 13, -1},
	{-14, 117, 29, -4},
 	{-16, 104, 48, -8},
 	{-15, 87, 68, -12},
 	{-12, 68, 87, -15},
 	{-8, 48, 104, -16},
 	{-4, 29, 117, -14},
 	{-1, 13, 125, -9},
 	{0, 0, 128, 0}
    };

/*		 CUBIC interpolation table		*/
/**
*    static	int	blending_table[] [] =		\
*    {							\
*	{-5, 124, 10, -1},				\
*	{-9, 114, 25, -1},				\
* 	{-9, 99, 43, -5},				\
*	{-9, 82, 62, -7},				\
* 	{-7, 62, 82, -9},				\
*	{-5, 43, 99, -9},				\
*	{-2, 25, 114, -9},				\
*	{-1, 10, 124, -5},				\
* 	{0, 0, 128, 0}					\
*    };
**/	                	                        

cdraw(segs, remain, save)
int	segs;
int	remain;
int	save;
{
    /**
    *	Functional Description:
    *	
    *
    *	Formal Parameters:
    *	segs		number of segments to draw
    *	remain		number of degrees to draw in last segment
    *	save		flag -- if TRUE  save & restore 
    *			gid_x and gid_y
    *
    *	Implicit Parameters: 
    *	BLENDING_TABLE:	values used for interpolating curves
    *	cv_q: values on curve also used for the curve interpolation
    *
    *	Called Routines:
    *	DRAW_LINE - used to draw each segment on curve (nine segments drawn 
    *	between 2 points on curve. (That's 36 vectors for a circle)
    *
    * note:	Gidis's c.p. is not the same as Regis's c.p. (gid_x, gid_y)
    *	while drawing curves.  Gidis's c.p. is left at the end of the 
    *	last segment drawn, while [gid_x,gid_y] is the last specifier 
    *	in the input data stream.
    *	This is included to make relative coordinates work correctly
    *	while minimizing the number of G56_SET_POSITION's.
    *	cdraw saves the values of gid_x and gid_y, plugs cv_qx[1] and
    *	cv_qy[1] into them and draws the appropriate lines.  Before
    *	returning, cdraw restores gid_x/y to the saved values.  This
    *	is only included for SHADEd curves, but is done always because
    *	it is cheap enough.  
    *	Note that cdraw does not do a G56_SET_POSITION; it is the caller's
    *	responsibility to make sure that Gidis's c.p. is at cv_qx[1],cv_qy[1].
    *	fix -- above action takes place only when SAVE is TRUE
    **/

    int		i;
    int		j;
    int		x;
    int		y;
    int		tol;
    long	int	xtmp;
    long	int	ytmp;
    int		x_image;
    int		y_image;
    int		x_table[9];
    int		y_table[9];
    int		x_last;
    int		y_last;
    int		tmp1;
    int		tmp2;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* save gid_x and gid_y and load with Gidis's c.p.(see note above) */

    if (IS_TRUE(save))
    {
	x_image = rs->gid_x;
	y_image = rs->gid_y;
	rs->gid_x = rs->cv_qx[1];
	rs->gid_y = rs->cv_qy[1];
    }

/* the following code was meant to speed up drawing, but it makes small
   circles look flat - RDM 7/27/88 */

#if 0
    tol = rs->sa_x_extent >> 7;

    tmp1 = ((rs->cv_qx[1] - rs->cv_qx[2]) < 0 ? (-(rs->cv_qx[1] - rs->cv_qx[2])) :
		(rs->cv_qx[1] - rs->cv_qx[2]));
    tmp2 = ((rs->cv_qy[1] - rs->cv_qy[2]) < 0 ? (-(rs->cv_qy[1] - rs->cv_qy[2])) :
		(rs->cv_qy[1] - rs->cv_qy[2]));
    if (( tmp1 <= tol ) && ( tmp2 <= tol ))
    {
	draw_line(rs->cv_qx[2], rs->cv_qy[2]);
	if (IS_TRUE(save))
	{
	    rs->gid_x = x_image;	/* restore last coord pair to gid_x,gid_y */
	    rs->gid_y = y_image;	/* before returning */
	}
	return;
    }

#endif

    x_last = rs->gid_x;
    y_last = rs->gid_y;

    for (i = 0; i < segs; i++)
    {
	xtmp = 0;
	ytmp = 0;
	
	for (j = 0; j <= 3; j++) 
	{
	    xtmp += (blending_table[i][j] * rs->cv_qx[j]);
	    ytmp += (blending_table[i][j] * rs->cv_qy[j]);
	} 
	x = xtmp / 128;
	y = ytmp / 128;

	if ((i == (segs - 1)) && (remain != 10))
	{
	    int		x_delta;
	    int		y_delta;

	    x_delta = x - x_last;
	    y_delta = y - y_last;

	    x_delta = (x_delta * remain)/10;
	    y_delta = (y_delta * remain)/10;

	    x = x_last + x_delta;
	    y = y_last + y_delta;
	}
	x_last = (x_table[i] = x);
	y_last = (y_table[i] = y);
    }			/* of INCR i FROM 0 TO segs - 1 */

    /* now draw the whole thing */

    draw_multi_line(segs, x_table, y_table);

    if (IS_TRUE(save))
    {
	rs->gid_x = x_image;		/* restore last coord pair to gid_x,gid_y */
	rs->gid_y = y_image;
    }
}	/* End of ROUTINE cdraw */

crv_begin(closed)
int	closed;
{
    /**
    *	Functional Description: 
    *	This routine flags the fact that I am {ning a curve sequence. I
    *	will save my current location as the first point of the curve(although
    *	for an open curve this will only be used to determine direction)
    *	This routine is reached through GIDIS and an op-code 23.
    *
    *	Formal Parameters:
    *	closed		TRUE if a closed curve wanted,
    *			FALSE if an open curve wanted
    *
    *	Implicit Parameters:
    *	gid_x,gid_y:	current software location
    *
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
	rs->cv_closed = closed;
	rs->cv_check_pt = 1;		/*check for p1 next */
	rs->cv_in_progress = TRUE;		/*note that I have a curve in progress */

#if SEPARATE_CURRENT_POSITION 
	if (IS_FALSE(rs->gid_xy_valid))
	{
	    rpg_request_gidi_pos();
	}
#endif
	rs->cv_px[0] = rs->gid_x;		/*P0 set to current postiion */
	rs->cv_py[0] = rs->gid_y;

	rs->cv_qx[0] = rs->cv_px[0];
	rs->cv_qy[0] = rs->cv_py[0];
}

crv_continue(x_coor, y_coor)
int	x_coor;
int	y_coor;
{
    /**
    *	Functional Description: 
    *	This routine will continue a closed or open curve. It will save
    *	up points until it has a total of four,  it will begin drawing
    *	the curve. 
    *
    *	Formal Parameters:
    *	X-COOR: the x-value of the point on the curve.
    *	Y-COOR: the y-value of the point on the curve.
    *
    *	Implicit Parameters:
    *	cv_in_progress: see if we really have a curve in progress, if not don't
    *	do this 
    *	cv_check_pt: tells which point on the curve to include this point as
    *
    * WARNING:  gid_x and gid_y are deliberately not the same as Gidis's
    *	c.p. -- this is to allow relative position specifiers to be
    *	relative to the latest coordinate pair.  Gidis's c.p. is left at
    *	the previous coordinate pair which is how far we have actually
    *	drawn.
    *	cdraw saves gid_x and gid_y (latest pair) and sets them back to 
    *	cv_qx[1] and cv_qy[1] (previous pair, Gidis's c.p.) before
    *	drawing.  This is only really needed when shading is in effect.
    *	cdraw restores gid_x and gid_y before returning.
    *
    *	Routines Called:
    *	G56_SET_POSITION - to position on second point and draw from there
    *	cdraw - draw curve
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE(rs->cv_in_progress))
    {
	switch (rs->cv_check_pt)
	{
	    case  1 :
	    	rs->cv_qx[1] = x_coor;
	    	rs->cv_qy[1] = y_coor;
	    	rs->cv_px[1] = x_coor;
	    	rs->cv_py[1] = y_coor;
	    	rs->cv_check_pt = 2;
		G56_SET_POSITION(x_coor, y_coor);
		break;
	    case  2 :
	    	rs->cv_qx[2] = x_coor;
	    	rs->cv_qy[2] = y_coor;
	    	rs->cv_px[2] = x_coor;
	    	rs->cv_py[2] = y_coor;
	    	rs->cv_check_pt = 3;
		break;
	    case  3 :
	    	rs->cv_qx[3] = x_coor;
	    	rs->cv_qy[3] = y_coor;
	    	rs->cv_check_pt = 4;

		/*draw curve from q[1] to q[2] */

	    	cdraw(NOS_NUMBER_OF_SEGMENTS, 10, TRUE);
		break;
	    default :
	    	rs->cv_qx[0] = rs->cv_qx[1];
	    	rs->cv_qy[0] = rs->cv_qy[1];
	    	rs->cv_qx[1] = rs->cv_qx[2];
	    	rs->cv_qy[1] = rs->cv_qy[2];
	    	rs->cv_qx[2] = rs->cv_qx[3];
	    	rs->cv_qy[2] = rs->cv_qy[3];
	    	rs->cv_qx[3] = x_coor;
	    	rs->cv_qy[3] = y_coor;

		/*draw curve from q[1] to q[2] */

		cdraw (NOS_NUMBER_OF_SEGMENTS, 10, TRUE);
	}
	rs->gid_x = x_coor;		/* fake out routine COORDINATES */
	rs->gid_y = y_coor;		/* for relative coordinates */
#if SEPARATE_CURRENT_POSITION 
	rs->gid_xy_valid = TRUE;
#endif
    }
}		/* of routine crv_continue */

crv_end()
{
    /**
    *	Functional Description:
    *	This routine is used to finish up an open or closed curve. For an open
    *	curve it simply clears flags to let us know that we are done with the 
    *	curve. For a closed curve, it takes the first three points for the 
    *	curve(one was the current position when the curve began), and uses 
    *	those to close the curve.
    *	
    *	Formal Parameters:
    *	none
    *
    *	Implicit Parameters:
    *	cv_in_progress: see if we really have a curve in progress, if not don't
    *	do this 
    *	cv_check_pt: tells which point on the curve to include this point as
    *	P: first three points of closed curve
    *
    *	Routines Called:
    *	G56_SET_POSITION - to position on second point and draw from there
    *	cdraw - draw curve
    **/

    int		count;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE(rs->cv_in_progress))
    {

	if (IS_TRUE(rs->cv_closed))
	{

	    switch (rs->cv_check_pt)
	    {
		case  1 :	/* one point in table -- P[0] C(B) (E) */
		    break;	/* c.p. remains at point [0] */
		case  2 :	/* two points in table -- P[0] C(B) [1] (E) */
	    /* set c.p. to position 0. */

		    rs->gid_x = rs->cv_px[0];
		    rs->gid_y = rs->cv_py[0];
		    G56_SET_POSITION(rs->gid_x, rs->gid_y);
#if SEPARATE_CURRENT_POSITION 
		    rs->gid_xy_valid = TRUE;
#endif
		    break;		/* of 2 points */
		case  3 :	/* 3 points: P[0] C(B) [1] [2] (E) */
		    rs->cv_qx[3] = rs->cv_px[0];
		    rs->cv_qy[3] = rs->cv_py[0];
				/* from [1] to [2] */
		    cdraw(NOS_NUMBER_OF_SEGMENTS, 10, TRUE);
		    for (count = 1; count <= 2; count++)
		    {
			rs->cv_qx[0] = rs->cv_qx[1];
			rs->cv_qy[0] = rs->cv_qy[1];
			rs->cv_qx[1] = rs->cv_qx[2];
			rs->cv_qy[1] = rs->cv_qy[2];
			rs->cv_qx[2] = rs->cv_qx[3];
			rs->cv_qy[2] = rs->cv_qy[3];
			rs->cv_qx[3] = rs->cv_px[count]; /*retrieve first three points */
			rs->cv_qy[3] = rs->cv_py[count]; /*to finish closed curve */

			cdraw (NOS_NUMBER_OF_SEGMENTS, 10, TRUE);	/*  */
		    }
	    /* set c.p. to position 0. */

		    rs->gid_x = rs->cv_px[0];
		    rs->gid_y = rs->cv_py[0];
		    G56_SET_POSITION(rs->gid_x, rs->gid_y);
#if SEPARATE_CURRENT_POSITION 
		    rs->gid_xy_valid = TRUE;
#endif
		    break;		/* of 3 points */
		default :  /* four or more points -- P[0] C(B) [1] [2] [3] ... (E) */
		    for (count = 0; count <= 2; count++)
		    {
			rs->cv_qx[0] = rs->cv_qx[1];
			rs->cv_qy[0] = rs->cv_qy[1];
			rs->cv_qx[1] = rs->cv_qx[2];
			rs->cv_qy[1] = rs->cv_qy[2];
			rs->cv_qx[2] = rs->cv_qx[3];
			rs->cv_qy[2] = rs->cv_qy[3];
			rs->cv_qx[3] = rs->cv_px[count];	/*retrieve first three points */
			rs->cv_qy[3] = rs->cv_py[count];	/*to finish closed curve */
		
			cdraw (NOS_NUMBER_OF_SEGMENTS, 10, TRUE);		/*draw curve, all of it */
		    }
	    /* set c.p. to position 0. */

		    rs->gid_x = rs->cv_px[0];
		    rs->gid_y = rs->cv_py[0];
		    G56_SET_POSITION(rs->gid_x, rs->gid_y);
#if SEPARATE_CURRENT_POSITION 
		    rs->gid_xy_valid = TRUE;
#endif
	    }
	}		/* End of if IS_TRUE(rs->cv_closed) */
	else		/* must be open curve */
	{
	    rs->gid_x = rs->cv_qx[rs->cv_check_pt - 1];
	    rs->gid_y = rs->cv_qy[rs->cv_check_pt - 1];
	    G56_SET_POSITION(rs->gid_x, rs->gid_y);
#if SEPARATE_CURRENT_POSITION 
	    rs->gid_xy_valid = TRUE;
#endif
	}
	rs->cv_in_progress = FALSE;
	rs->cv_closed = FALSE;
    }		/* End of if IS_TRUE(cv_in_progress) */
}			/* End of routine crv_end */

crv_abort()
{
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE(rs->cv_in_progress))
    {
	G56_SET_POSITION(rs->gid_x, rs->gid_y);
#if SEPARATE_CURRENT_POSITION 
	rs->gid_xy_valid = TRUE;
#endif
    }
}

draw_arc(x_center, y_center, x_circum, y_circum, degrees) 
int	x_center;
int	y_center;
int	x_circum;
int	y_circum;
int	degrees;
{
    /**
    *	EXPLICIT INPUTS:
    *	    x_center,y_center	center of circle
    *	    x_circum,y_circum	point on circumference at which to start
    *	    degrees		number of degrees to start
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:	    none
    *
    *	IMPLICIT OUTPUTS:
    *
    *	SIDE EFFECTS:
    *	    a circle is drawn, using cdraw to do the work
    **/

    int		count;
    int		x_rad;
    int		y_rad;
    int		quadrants;
    int		segments;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (degrees == 0)
    {
	return;
    }
    if ((rs->gid_x != x_circum) || (rs->gid_y != y_circum))
    {
	rs->gid_x = x_circum;
	rs->gid_y = y_circum;
	G56_SET_POSITION(rs->gid_x, rs->gid_y);
#if SEPARATE_CURRENT_POSITION 
	rs->gid_xy_valid = TRUE;
#endif
    }
    x_rad = x_circum - x_center;
    y_rad = y_circum - y_center;

    rs->cv_qx[1] = x_circum;		/* point */
    rs->cv_qy[1] = y_circum;

    rs->cv_qx[3] = x_center - x_rad;	/* 180 degrees away */
    rs->cv_qy[3] = y_center - y_rad;

    if (degrees < 0)
    {
	rs->cv_qx[2] = x_center - y_rad;	/* +90 degrees */
	rs->cv_qy[2] = y_center + x_rad;

	rs->cv_qx[0] = x_center + y_rad;	/* -90 degrees */
	rs->cv_qy[0] = y_center - x_rad;

	degrees = (-degrees);
    }
    else
    {
	rs->cv_qx[2] = x_center + y_rad;	/* +90 degrees */
	rs->cv_qy[2] = y_center - x_rad;

	rs->cv_qx[0] = x_center - y_rad;	/* -90 degrees */
	rs->cv_qy[0] = y_center + x_rad;
    }
    if (degrees > 360)
    {
	degrees = 360;
    }
    rs->cv_in_progress = TRUE;
    {

	/* get number of full 90 degree quadrants */

	quadrants = degrees/90;
	degrees %= 90;			/* degrees = degrees % 90; */

	/* determine how many segments are needed in the last quadrant */
	/* and how many degrees are needed in the last segment */

	segments = degrees/10;
	degrees %= 10;			/* degrees = degrees % 10; */

	/* add one to segments if there is a partial */
	/* if no partial, make sure we output 10 degrees */

	if (degrees > 0)
	{
	    segments++;
	}
	else
	{
	    degrees = 10;
	}
    }
    for (count = 1; count <= quadrants; count++)
    {
	int    x_temp;
	int    y_temp;

	cdraw (9, 10, FALSE);		/* 9 segments, all of 10 degrees */

	x_temp = rs->cv_qx[0];
	y_temp = rs->cv_qy[0];

	rs->cv_qx[0] = rs->cv_qx[1];
	rs->cv_qy[0] = rs->cv_qy[1];
	rs->cv_qx[1] = rs->cv_qx[2];
	rs->cv_qy[1] = rs->cv_qy[2];
	rs->cv_qx[2] = rs->cv_qx[3];
	rs->cv_qy[2] = rs->cv_qy[3];
	rs->cv_qx[3] = x_temp;
	rs->cv_qy[3] = y_temp;
    }
    if (segments > 0) 	/* must output a partial quadrant */
    {
	cdraw (segments, degrees, FALSE);	/* with the last segment containing */
    }					/* only 'degrees' degrees. */
    rs->cv_in_progress = FALSE;
    rs->cv_closed = FALSE;
}
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:01:13 ""
**/
