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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SGX.C*/
/* *4    19-JUN-1992 20:19:53 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *3     9-JUN-1992 09:58:47 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *2     3-MAR-1992 17:04:20 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:40:36 PARMENTER "Extensions stipple pixmap"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SGX.C*/
#ifndef VMS
   /*
#else
# module BKR_SGX "V03-0000"
#endif
#ifndef VMS
   */
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Display routines for SGX datatype.
**
**  AUTHORS:
**
**      Joseph M. Kraetsch
**
**  CREATION DATE:     19-Jul-1990
**
**  MODIFICATION HISTORY:
**
**	V03-0000    JMK0000	Joseph M. Kraetsch	19-Jul-1990
**  	    	    New module.
**
**--
**/


/*
 * INCLUDE FILES
 */

#include   <X11/Xlib.h>
#include   <string.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_sgx.h"         /* function prototypes for .c module */

#define    bkr_screen	    XDefaultScreen( bkr_display )
#define    root_window	    XDefaultRootWindow( bkr_display )



/*
 * FORWARD ROUTINES
 */

static void		    set_line_width();


/*
 * FORWARD DEFINITIONS 
 */

static BR_HANDLE 	    sgx_init = NULL;
static GC                   sgx_gc = NULL;	/*  Private GC for sgx  */
static unsigned		    gc_line_width = 1;	/*  current setting  */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_sgx_init
**
** 	Initializes the SGX display interface.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_sgx_init(VOID_PARAM)
{
    unsigned int    value_mask;
    XGCValues       xgcvalues;

    if ( sgx_init == NULL)
    {
        /*  create a private gc for sgx use  */	

        xgcvalues.foreground = bkr_window_foreground;
        xgcvalues.background = bkr_window_background;
        xgcvalues.line_width = 0;
        value_mask = GCForeground | GCBackground | GCLineWidth;
        sgx_gc = (GC)XCreateGC( bkr_display, root_window, value_mask, &xgcvalues );

        gc_line_width = 0;    
        sgx_init = (BR_HANDLE)1;
    }
}   /*  end of bkr_sgx_init  */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_sgx_exit
**
** 	Terminates use of the SGX graphics display interface.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_sgx_exit(VOID_PARAM)
{
    if ( ( sgx_init == NULL) || ( sgx_init == NODISPLAY ) )
	return;

    /*  free any sgx resources  */

    XFreeGC( bkr_display, sgx_gc );
    sgx_gc = NULL;
    sgx_init = NULL;
    return;

}   /*  end of bkr_sgx_exit  */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_sgx_display
**
** 	Display routine for handling the CHUNK_SGX data type.
**
**  FORMAL PARAMETERS:
**
**  	window - id of the window to draw in.
**  	chunk  - pointer to the chunk to display.
**  	xoff   - x offset of origin in window.
**  	yoff   - y offset of origin in window.
**  	xclip  - x value of clip rectangle.
**  	yclip  - y value of clip rectangle.
**  	wclip  - width of clip rectangle.
**  	hclip  - height of clip rectangle.
**  	    	    	    NOTE: clip rect. is wrt window
**			    	  origin, NOT wrt (xoff, yoff)
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    handle for graphic being displayed, or
**  	    	    NODISPLAY value if an error occurred.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BR_HANDLE
bkr_sgx_display PARAM_NAMES((window,chunk,xoff,yoff,xclip,yclip,wclip,hclip))
    Window		window PARAM_SEP
    BMD_CHUNK		*chunk PARAM_SEP
    int 		xoff PARAM_SEP		/*  x offset of origin in window    */
    int 		yoff PARAM_SEP		/*  y offset of origin in window    */
    int 		xclip PARAM_SEP		/*  x value of clip rectangle	    */
    int 		yclip PARAM_SEP		/*  y value of clip rectangle	    */
    int 		wclip PARAM_SEP		/*  width of clip rectangle	    */
    int 		hclip PARAM_END		/*  height of clip rectangle	    */
					/*  NOTE: clip rect. is wrt window  */
					/*  origin, NOT wrt (xoff, yoff)    */
{
    BR_HANDLE		handle = 0;	/*  sgx display handle		    */
    BMD_SGX_PKT		*packet;
    BR_UINT_8		*data_end;
    BMD_SGX_SET_PKT	*set;
    unsigned		line_width = 0;	/*  current line width (default 0)  */
    BMD_SGX_LINE_PKT    *line;
    BMD_SGX_RECT_PKT	*rectangle;
    BMD_SGX_ARC_PKT 	*arc;
    BMD_SGX_POLY_PKT	*poly;
    XPoint		points[BMD_SGX_MAX_PTS + 1]; /*  for drawing polygons */
    int			point;

    if ( sgx_init == NULL)	/*  first call for sgx  */
	bkr_sgx_init();

    if ( sgx_init == NODISPLAY 	    	    /*  sgx not available  */
    	  || chunk->handle == NODISPLAY )   /*  couldn't display chunk last time */
    	return NODISPLAY;

    if ( chunk->data_addr == 0 )
	return NULL;

    packet = (BMD_SGX_PKT *) chunk->data_addr;
    data_end = chunk->data_addr + chunk->data_len;

    while ( packet < (BMD_SGX_PKT *) data_end )
    {
	switch ( packet->tag )
	{
	    case BMD_SGX_SET:	/*  set drawing attributes           */
		set = (BMD_SGX_SET_PKT *) packet->value;
		if ( set->attribute != GCLineWidth )
		    break;	/*  only line width can be set  */
		line_width = SCALE_VALUE400( set->value );
		/*  don't change gc until we have to  */
		break;

	    case BMD_SGX_LINE:	/* draw a rule                      */
		line = (BMD_SGX_LINE_PKT *) packet->value;
		if ( line_width != gc_line_width )
		    set_line_width( line_width );

		XDrawLine ( bkr_display, window, sgx_gc,
		    xoff + SCALE_VALUE400( line->x1 ),
		    yoff + SCALE_VALUE400( line->y1 ),
		    xoff + SCALE_VALUE400( line->x2 ),
		    yoff + SCALE_VALUE400( line->y2 ) );
		break;

	    case BMD_SGX_RECT:	/* draw a rectangle                 */
		rectangle = (BMD_SGX_RECT_PKT *) packet->value;
		if ( line_width != gc_line_width )
		    set_line_width( line_width );
		XDrawRectangle ( bkr_display, window, sgx_gc,
                    xoff + SCALE_VALUE400( rectangle->x ),
                    yoff + SCALE_VALUE400( rectangle->y ),
                    SCALE_VALUE400( rectangle->width ),
                    SCALE_VALUE400( rectangle->height ) );
		break;

	    case BMD_SGX_RECT_FILL:	/* draw a filled rectangle          */
		rectangle = (BMD_SGX_RECT_PKT *) packet->value;
		XFillRectangle ( bkr_display, window, sgx_gc,
                    xoff + SCALE_VALUE400( rectangle->x ),
                    yoff + SCALE_VALUE400( rectangle->y ),
                    SCALE_VALUE400( rectangle->width ),
                    SCALE_VALUE400( rectangle->height ) );
		break;

	    case BMD_SGX_ARC:	/* draw an arc                      */
		arc = (BMD_SGX_ARC_PKT *) packet->value;
		if ( line_width != gc_line_width )
		    set_line_width( line_width );
		XDrawArc ( bkr_display, window, sgx_gc,
                    xoff + SCALE_VALUE400( arc->x ),
                    yoff + SCALE_VALUE400( arc->y ),
                    SCALE_VALUE400( arc->width ),
                    SCALE_VALUE400( arc->height ),
                    arc->start_angle,
                    arc->extent );
		break;

	    case BMD_SGX_ARC_FILL:	/* draw a filled arc                */
		arc = (BMD_SGX_ARC_PKT *) packet->value;
		XFillArc ( bkr_display, window, sgx_gc,
                    xoff + SCALE_VALUE400( arc->x ),
                    yoff + SCALE_VALUE400( arc->y ),
                    SCALE_VALUE400( arc->width ),
                    SCALE_VALUE400( arc->height ),
                    arc->start_angle,
                    arc->extent );
		break;

	    case BMD_SGX_POLY:	/* draw a polygon		    */
		poly = (BMD_SGX_POLY_PKT *) packet->value;
		if ( poly->num_points > BMD_SGX_MAX_PTS )
		    break;
		for (point = 0; point < poly->num_points; point++)
		{
		    points[point].x = xoff 
			+ SCALE_VALUE400( poly->points[point].x );
		    points[point].y = yoff 
			+ SCALE_VALUE400( poly->points[point].y );
		}

	       XDrawLines(
		    bkr_display,		/*  display	*/
		    window,			/*  window      */
		    sgx_gc,			/*  GC          */
		    points,			/*  points      */
		    poly->num_points,		/*  num_points  */
		    CoordModeOrigin );          /*  mode        */
		break;

	    case BMD_SGX_POLY_FILL:	/* draw a filled polygon	    */
		poly = (BMD_SGX_POLY_PKT *) packet->value;
		if ( poly->num_points > BMD_SGX_MAX_PTS )
		    break;
		for (point = 0; point < poly->num_points; point++)
		{
		    points[point].x = xoff 
			+ SCALE_VALUE400( poly->points[point].x );
		    points[point].y = yoff 
			+ SCALE_VALUE400( poly->points[point].y );
		}

		XFillPolygon(
		    bkr_display,		/*  display	*/
		    window,			/*  window      */
		    sgx_gc,			/*  GC          */
		    points,			/*  points      */
		    poly->num_points,		/*  num_points  */
		    Complex,                    /*  shape	*/
		    CoordModeOrigin );          /*  mode        */
		break;

	    default:
		/*  ignore unknown tags  */
		break;

	}
	packet = (BMD_SGX_PKT *) ( (char *) packet + packet->len );
	if ( packet->len == 0 )
	    break;	
    }

    return handle;

}   /*  end of bkr_sgx_display  */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_sgx_close
**
** 	Closes an SGX graphic given its handle.
**
**  FORMAL PARAMETERS:
**
**	handle - id of the displayed graphic.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_sgx_close PARAM_NAMES((handle))
    BR_HANDLE handle PARAM_END
{
    if ( ( sgx_init == NULL ) || ( sgx_init == NODISPLAY )
	  || ( handle == NULL ) || ( handle == NODISPLAY ) )
	return;

    /*  free any sgx resources associated with handle  */

    return;

}   /*  end of bkr_sgx_close  */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	set_line_width
**
** 	Modifies the line width of the SGX private GC.
**
**  FORMAL PARAMETERS:
**
**	line_width - new value for the line width.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static void
set_line_width ( line_width )
    unsigned		line_width;	/*  new line width  */
{
    unsigned int    value_mask;
    XGCValues       xgcvalues;

    /*  set the line width in the gc  */	

    xgcvalues.line_width = line_width;
    value_mask = GCLineWidth;
    XChangeGC( bkr_display, sgx_gc, value_mask, &xgcvalues );
    gc_line_width = line_width;

}

