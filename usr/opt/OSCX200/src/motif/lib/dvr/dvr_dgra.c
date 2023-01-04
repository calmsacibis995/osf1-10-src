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
#define Module DVR_DGRA 
#define Ident  "V02-037"

/*
**++
**   COPYRIGHT (c) 1989, 1992 BY
**   DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**   ALL RIGHTS RESERVED.
**
**   THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**   ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**   INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**   COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**   OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**   TRANSFERRED.
**
**   THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**   AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**   CORPORATION.
**
**    DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**    SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
** MODULE NAME:
**	dvr_display_graphics.c (vms)
**	dvr_dgra.c	       (OS/2)
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ENVIORNMENT:
**	vms, ultrix, os/2
**
** ABSTRACT:
**      [@tbs@]
**
**
** AUTHORS:
**      Kathy Robinson,  1-Mar-89
**
**
** MODIFIED BY:
**
**	V02-037	RDH003		Don Haney		15-Jul-1992
**		Specify same-directory ]includes as "", not <>
**
**	V02-036	SJM0000		Stephen Munyan		 2-Jul-1992
**		Add support for closed bezier curves to the bezier
**		curve computation logic.
**
**	V02-035	KLM0002		Kevin McBride		17-Jun-1992
**		Fix type mismatch warnings on DOS build
**
**	V02-034	KLM0001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**
**	V02-033	RKN0000		Ram Nori		03-Jun-1992
**		Cleanup type mis-match in dvr_rounded_polyline routine.
**
**	V02-032	DAM0000		Dennis McEvoy		01-Jun-1992
**		type cleanups for alpha/osf1
**
**	V02-031	RAM0000		Ralph A. Mack		04-Nov-1991
**		Fixed MS-DOS warnings
**
**      V02-030	RKN000          Ram Kumar Nori		29-Oct-1991
**		Removed calls to dvr_transform_path and transformation of
**		arrow heads, rotated ellipses and rounded/smoothed polylines
**		is now done in dvr_transform_path routine.
**
**      V02-029	RKN000          Ram Kumar Nori		18-Oct-1991
**		Moved dvr_rotated_ellipse routine to format phase.
**		Added routine dvr_rotated_ellipse this module
**
**      V02-028		DAM000          Dennis McEvoy	07-Oct-1991
**		cleanup ultrix warnings
**
**      V02-027		DAM000          Dennis McEvoy	02-Oct-1991
**		remove extraneous semicolons
**
**      V02-026		RKN000		Ram Kumar Nori	17-Sep-1991
**		Fixed Arrow Head Bugs On Filled Polylines Which Are Rounded.
**		Added Routines To Shorten Rounded Polylines To Account For
**		Arrow Heads.
**		Fixed Arrow Head Bugs On Filled Arcs.
**
**      V02-025		DAM000          Dennis McEvoy	05-Aug-1991
**		renamed header files, removed dollar signs
**
**      V02-024		RKN000          Ram Nori        12-Jun-1991
**		Fixed MS-DOS Warnings, which were fixed in V02-023 but
**		somehow got lost
**
**      V02-023		RKN000		Ram Nori	10-Jun-1991
**		Fixed The Rounding Bug When Curve Ratio equals to 1 or 0
**		Fixed The Coordinate Computation Bug On Inclined Polylines
**		Fixed MS-DOS Warnings (With Ralph Mack)
**
**      V02-022         CJR0000         Chris Ralto	24-May-1991
**		Disable the broken Fill Area Set code pending
**		a full working implementation.  The current
**		code to display FAS is hanging the Viewer.
**
**      V02-021         DAM0000         Dennis McEvoy	20-May-1991
**		allow arrows on smoothed/rounded polylines
**
**      V02-020         DAM0000         Dennis McEvoy	17-May-1991
**		disallow arrows on closed polylines
**
**      V02-019         DAM0000         Dennis McEvoy	16-May-1991
**		fix arrowhead check for polylines
**
**      V02-018         DAM0000         Dennis McEvoy	15-May-1991
**		change compute_coords proto to match decl
**
**      V02-017         DAM0000         Dennis McEvoy	14-May-1991
**		add bounding box check for objects with arrow heads
**
**      V02-016         DAM0000         Dennis McEvoy	13-May-1991
**		move dvr_int include for ultrix (fix warnings)
**
**      V02-015         RKN0000         Ram Kumar Nori	26-Apr-1991
**		Added code to format arrow heads and call arrow head
**		processing routines for Arcs
**
**	V02-014		RAM0000		Ralph A. Mack	24-Apr-1991
**		Adding #ifdefs for MS-Windows
**
**      V02-013         RKN0000         Ram Kumar Nori	23-Apr-1991
**		Added  DVS_ARR.H inlcude file
**
**      V02-012         RKN0000         Ram Kumar Nori	20-Apr-1991
**		Added code to format arrow heads and call arrow head
**		processing routines
**
**      V02-011         DAM0000         Dennis McEvoy	03-Apr-1991
**		cleanup typedefs
**
**      V02-010         RKN0000         Ram Kumar Nori	22-Mar-1991
**		Initialized Join queues in  rounding & smoothing routines
**
**      V02-009         RKN0000         Ram Kumar Nori	06-Mar-1991
**		Added a Condition before doing rounding or smoothing
**
**      V02-008         RTG0001         Dick Gumbel	05-Mar-1991
**		Cleanup #include's
**
**      V02-007         RKN0000         Ram Kumar Nori	27-Jan-1991
**		Implemented rounded polylines
**
**      V02-006         RKN0000         Ram Kumar Nori	25-Jan-1991
**		Implemented DECwrite Smoothing
**
**      V02-005         SJM0000         Stephen Munyan	6-Dec-1990
**		Merge in fixes from XUI into to the Motif code
**
**	V02-004
**                  Fix ACCVIO in Bezier code logic if an invalid
**                  path is specified.
**
**	V02-004		SJM0000		Stephen Munyan	20-Jun-1990
**		Conversion to Motif
**
**	V02-003		DAM0000		Dennis McEvoy	05-Mar-1990
**		changes for os/2 port
**
**	V02-002		SJM0000		Stephen Munyan	14-Dec-1989
**		Changed static definition to be read only and sharable
**
**	V02-001		PBD0001		Peter B. Derr	1-Mar-1989
**		Cleanup include files
**--
*/

#include <cdatrans.h>

#ifdef __vms__
#include <dvrint.h>				/* DVR internal definitions */

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#pragma standard				/* turn /stand=port back on */

#endif

#ifdef OS2

#define INCL_PM 				/* tell OS/2 to include Presentation Manager defs */
#define PMMLE_INCLUDED                          /* do not include multi-line editing defs */
#include <os2.h>                                /* OS/2 defs */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef MSWINDOWS
#define NOKERNEL				/* Omit unused definitions */
#define NOUSER					/* from parse of windows.h */
#define NOMETAFILE				/* to save memory on compile. */
#include <windows.h>				/* MS-Windows definitions. */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef __unix__

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#include <dvsarr.h>				/* DVS_ARR definitions */

#include "dvrwdef.h"				/* Public Defns w/dvr_include */
#include "dvrwint.h"				/* Viewer Constants and structures  */
#include "dvrwptp.h"				/* dvr windowing prototypes */

/*
 * TYPEDEFS
 */
typedef struct
    {                                               /* Point */
    float f_x, f_y;                                 /*   X and Y coordinates */
    } f_point, *F_POINT;


#define d_step  ((float)0.05)
#define k_tmax  ((float)1.00)
#define k_gen_count 20
#define k_control_pts 4
#define END	0
#define	START	1

/* protos from dvr_format_page */
PROTO( unsigned long dvr_adjust_state_bbox,
		(DvrViewerWidget,
		 STATEREF,
		 OBJ) );

PROTO( unsigned long dvr_transform_path,
		(DvrViewerWidget,
		 STATEREF,
		 PTH, PTH *,
		 void *) );

/* local routine prototypes */

PROTO( static double bblend,
		(long,
    		 double)  );

PROTO( static void bezier ,
		(F_POINT,
   		 double,
    		 signlong  *) );

PROTO( unsigned long dvr_generate_curve,
		(DvrViewerWidget,
    		 PTH,		  /* a path containing a bezier curve */
    		 PTH *,	  	  /* pointer to returned polyline path */
		 boolean) );	  /* flag to indicate we should close the bezier curve */


PROTO( static void dvr_smoothing_path,
            (DvrViewerWidget,
             PTH,
             PTH *,
             signlong) );

PROTO( static void copy_curve_coords,
            (PTH,
             PTH ,
             int) );

PROTO( static void generate_curve_points,
             (PTH,
	      float,
	      PTH *) );

PROTO( static void dvr_rounded_polyline,
	     (DvrViewerWidget,
	      PTH,
	      PTH *,
	      signlong) );

PROTO( double dvr_drawpolylinearrow,
	     (PTH,
	      GAT,
	      int,
	      CDAmeasurement *,
	      PTH *,
	      PTH *,
	      Point **) );

PROTO( double dvr_drawarcarrow,
	     (PTH,
	      GAT,
	      Arc_struct *,
	      int,
	      CDAmeasurement *,
	      PTH *,
	      PTH *) );

PROTO (static signlong compute_coords,
		(signlong,
		 signlong,
		 double,
		 double ) );

PROTO (static long shorten_arrow_path,
	     (DvrViewerWidget,
	      STATEREF,
	      GAT,
	      Arr_info *,
	      PTH,
	      Point *,
	      double,
	      int,
	      boolean ) );

PROTO (static void shorten_round_smooth_path,
	     (PTH,
	      PTH,
	      double,
	      int ) );

PROTO (static int find_the_index,
	     (PTH,
	      double,
	      int ) );

PROTO (static void copy_reduced_path,
	     (PTH,
	      PTH,
	      double,
	      int,
	      int,
	      int ) );

PROTO (unsigned long dvr_rotated_ellipse,
	     (DvrViewerWidget,
	      PTH,
	      PTH * ) );

CDAstatus
dvr_format_polyline_object (vw, poly, new_lin_path, state)
DvrViewerWidget	    vw;
LIN		    poly;
PTH		    * new_lin_path;
STATEREF	    state;

{
OBJ_PRIVATE	    private;
PTH		    tmp_path;
GAT		    gattr;
LIN_PRIVATE	    line_private;
int		    arrow_type;
CDAmeasurement	    arrow_width;
PTH		    head1;
PTH		    head2;
Point		    * v;
double		    head_length;
Arr_info	    * arrow_struct;
unsigned long	    status;
boolean		    arrow_flag = FALSE;
boolean		    draw_markers = FALSE;
boolean		    round_or_smooth = FALSE;

private = (OBJ_PRIVATE)(poly->lin_obj.obj_user_private);

/* set some defaults */
private->private_info = NULL;
private->orig_path = poly->lin_obj.obj_path;

tmp_path = poly->lin_obj.obj_path;
* new_lin_path = NULL;
/*
** Get the Graphics attirbute structure
*/
gattr = &poly->lin_attributes;

/*
** Initilaize the Pointer to LIN_PRIVATE structure to NULL
*/
line_private = NULL;

/*
** Check to see if there are markers on the polyline. Transform the original
** Engine defined path to pixel coordinates
*/
if ( poly->lin_obj.obj_flags & ddif_m_lin_draw_markers )
   draw_markers = TRUE;
/*
** Check to see if the polyline rounded (Smoothing) bit is set. If it is set
** then call the following routine to adjust the coordinates to draw a cubic
** B-Spline btween the vertices.
*/

if ( (poly->lin_obj.obj_flags & ddif_m_lin_rounded_polyline )
     && (tmp_path->pth__specific.pth__xy.pth_pairs >= 3))
   {
   round_or_smooth = TRUE;
   dvr_smoothing_path (vw,
                       poly->lin_obj.obj_path,
                       new_lin_path,
                       poly->lin_obj.obj_flags
                      );
   private->orig_path = * new_lin_path;
   }

else
   if ( (QUE_GET(tmp_path->pth__specific.pth__xy.pth_qjoin))
       && (tmp_path->pth__specific.pth__xy.pth_pairs >= 3))
      {
      /*
      ** Check to see if the polyjoin object. If it is  then call the following
      ** routine to adjust the coordinates to draw a Rational Bezier curve
      ** between the vertices.
      */

      round_or_smooth = TRUE;
      dvr_rounded_polyline ( vw,
			 poly->lin_obj.obj_path,
			 new_lin_path,
			 poly->lin_obj.obj_flags
		       );
      private->orig_path = * new_lin_path;
      }

/*
** If there are Markers on a polyline which has to be rounded then transform
** the engine defined path to pixel coordinates and save in LIN_PRIVATE
*/
if ( round_or_smooth && draw_markers )
   {
   if ( line_private == NULL)
      status = dvr_alloc_struct (vw, lin_private, (void **) &line_private );

   if ( DVR_FAILURE ( status ))
      {
      /*
      ** Unexpected error
      */
      return status;
      }
   line_private->eng_path = NULL;
   }

/*
** check for arrow heads on poly line only if not closed path
*/

if ( !(poly->lin_obj.obj_flags & ddif_m_lin_close_polyline) &&
      ( (gattr->gat_line_start_end == DDIF_K_ARROW_LINE_END)||
        (gattr->gat_line_end_end   == DDIF_K_UNFILLED_ARROW_LINE_END) ||
        (gattr->gat_line_start_end == DDIF_K_UNFILLED_ARROW_LINE_END) ||
        (gattr->gat_line_end_end   == DDIF_K_ARROW_LINE_END) ) )

   {
   head1 = head2 = NULL;
   arrow_type = dvs_get_arrow_head_type (gattr);
   head_length = dvr_drawpolylinearrow ( poly->lin_obj.obj_path, gattr,
			arrow_type, &arrow_width, & head1, & head2, &v);
   if ( head_length != 0.0)
      {
      if ( line_private == NULL)
         {
	 status = dvr_alloc_struct (vw, lin_private, (void **) &line_private);

	 if ( DVR_FAILURE ( status ))
            {
	    /* Unexpected error */
	    return status;
            }
         line_private->eng_path = NULL;
         }
      status = dvr_alloc_memory (vw,sizeof( Arr_info),(void **) &arrow_struct);
      if ( DVR_FAILURE ( status ))
         {
	 /* Unexpected error */
	 return status;
         }
      else
         {
	 arrow_flag = TRUE;
         /*
	 ** Now Shorten the Arrow head path and Format the
	 */
         if ( round_or_smooth)
            /*
            ** If there is rounding or smoothing then reduce the
	    ** modified path other wise reduce the original path
	    ** the reduced path is in Arr_info structure
	    */
            status = shorten_arrow_path ( vw, state, gattr, arrow_struct,
					  * new_lin_path, v, head_length,
					  arrow_type,round_or_smooth);
         else
            status = shorten_arrow_path ( vw, state, gattr, arrow_struct,
					  poly->lin_obj.obj_path, v,
					  head_length, arrow_type,
					  round_or_smooth);
	 if ( DVR_FAILURE ( status ))
	    {
	    /*
	    ** Unexpected error; internal processing error in path transform
	    */
	    (void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0 );

	    /*
	    ** Object will not be properly displayable; mark as 'no display' to
	    ** suppress attempts to display
	    */
	    private->flags |= NO_DISPLAY_FLAG;

	    return status;
	    }
         CdaFree(v);
         }
      }
   else
      {
      /*
      ** head length returned 0.0, invalid, ignore arrow heads
      */
      ((OBJ_PRIVATE)poly->lin_obj.obj_user_private)->private_info = NULL;

      if ( round_or_smooth )
	 private->orig_path = * new_lin_path;
      else
	 private->orig_path = poly->lin_obj.obj_path;
      }
   }

if (line_private != NULL)
   ((OBJ_PRIVATE)poly->lin_obj.obj_user_private)->private_info = (void *)line_private;

if (arrow_flag)
   {
   /* finish formatting polylines with arrowheads */

   /*
   ** Store the arrow width and arrow type
   */
   arrow_struct->arrow_width = CONVERT_CP_TO_PIXEL(arrow_width,
						    vw->dvr_viewer.x_dpi);
   arrow_struct->arrow_type = arrow_type;
   arrow_struct->start_arrow = head1;
   arrow_struct->end_arrow = head2;
   line_private->arrow_info = arrow_struct;

   }

return(DVR_NORMAL);

}



CDAstatus
dvr_format_arc_object (vw, arc, rotated_path, state)
DvrViewerWidget	    vw;
ARC		    arc;
PTH		    * rotated_path;
STATEREF	    state;

{
OBJ_PRIVATE	    private;
PTH		    path;
GAT		    gattr;
ARC_PRIVATE	    arc_priv;
int		    arrow_type;
CDAmeasurement	    arrow_width;
PTH		    head1;
PTH		    head2;
double		    head_length;
Arr_info	    * arrow_struct;
Arc_struct	    arc_info;
PTH		    arc_arr_path;
unsigned long	    status;
boolean		    arrow_flag = FALSE;

private = (OBJ_PRIVATE)(arc->arc_obj.obj_user_private);

path = arc->arc_obj.obj_path;
/*
** Get the Graphics attirbute structure
*/
gattr = &arc->arc_attributes;

/*
** Initilaize the Pointer to ARC_PRIVATE structure to NULL
*/
arc_priv = NULL;

/*
** Draw Arrow heads on poly line only if there is no smoothing, the
** Polylines are not rounded
*/

if ((((gattr->gat_line_start_end == DDIF_K_ARROW_LINE_END)||
       (gattr->gat_line_end_end == DDIF_K_UNFILLED_ARROW_LINE_END)) ||
      ((gattr->gat_line_start_end == DDIF_K_UNFILLED_ARROW_LINE_END)||
       (gattr->gat_line_end_end == DDIF_K_ARROW_LINE_END)))
   )
   {

   head1 = head2 = NULL;
   arrow_type = dvs_get_arrow_head_type (gattr);
   head_length = dvr_drawarcarrow ( arc->arc_obj.obj_path, gattr, &arc_info,
				arrow_type, &arrow_width, & head1, & head2);
   if (head_length != 0.0)
      {
      arrow_flag = TRUE;
      status = dvr_alloc_struct (vw, arc_private, (void **) &arc_priv);

      if ( DVR_FAILURE ( status ))
         {
	 /* Unexpected error */
	 return status;
         }
      status = dvr_alloc_memory (vw, sizeof( Arr_info),(void **) &arrow_struct);
      if ( DVR_FAILURE ( status ))
         {
	 /* Unexpected error */
	 return status;
         }

      /*
      ** Allocate a PTH stucture and copy the contents of the input PTH
      ** strucutre into arc_arr_path and reduce the arc angles to account
      ** for arrow heads
      */
      arc_arr_path = dve__copy_pth_struct( arc->arc_obj.obj_path);

     if (arc->arc_obj.obj_path->pth__specific.pth__arc.pth_rotation == 0 )
        {
        /*
        ** Change the Starting and Extent angle of the arc To Account for arrows
        */
         arc_arr_path->pth__specific.pth__arc.pth_start = arc_info.start_angle * 100;
         arc_arr_path->pth__specific.pth__arc.pth_extent = arc_info.extent * 100;

         arrow_struct->arrow_path = arc_arr_path;
        }
      /*
      ** Store the arrow width and arrow type
      */
      arrow_struct->arrow_width = CONVERT_CP_TO_PIXEL(arrow_width,
						    vw->dvr_viewer.x_dpi);
      arrow_struct->arrow_type = arrow_type;
      arrow_struct->start_arrow = head1;
      arrow_struct->end_arrow = head2;
      arc_priv->arrow_info = arrow_struct;

      ((OBJ_PRIVATE)arc->arc_obj.obj_user_private)->private_info = (void *)arc_priv;

      private->orig_path = arc->arc_obj.obj_path;
      }

   else
      {
      ((OBJ_PRIVATE)arc->arc_obj.obj_user_private)->private_info = NULL;
      private->orig_path = arc->arc_obj.obj_path;
      }
   }
else
   {
   ((OBJ_PRIVATE)arc->arc_obj.obj_user_private)->private_info = NULL;
   private->orig_path = arc->arc_obj.obj_path;
   }

/*
** Check To See If the Arc Is Rotated, If It Is Call The Routine to Rotate
** The Arc and Save the Coordinates transformed to pixels in ARC_PRIVATE
** structure
*/
if (arc->arc_obj.obj_path->pth__specific.pth__arc.pth_rotation != 0 )
   {

   signlong   * coord_ptr;  /* Ptr to The Coordinates        */
   int	      count;

   status = dvr_rotated_ellipse(vw, arc->arc_obj.obj_path, rotated_path);
   if ( DVR_FAILURE ( status ))
       return ( status);

   if (arrow_flag)
      status = shorten_arrow_path(vw, state, gattr, arrow_struct, * rotated_path,
				  NULL, head_length, arrow_type, TRUE);
   if (arc_priv == NULL)
      {
      status = dvr_alloc_struct (vw, arc_private, (void **) &arc_priv);
      if ( DVR_FAILURE ( status ))
	 {
	 /* Unexpected error */
	 return status;
	 }
      ((OBJ_PRIVATE)arc->arc_obj.obj_user_private)->private_info = (void *)arc_priv;
      }


   count =  (int)(* rotated_path)->pth__specific.pth__xy.pth_pairs;
   coord_ptr = & ((* rotated_path)->pth__specific.pth__xy.pth_first_x);

   if (arc->arc_obj.obj_flags & ddif_m_arc_pie_arc)
      {
      /*
      ** It's a pie arc add two more sets of coordinates
      */
      coord_ptr[2*count] = path->pth__specific.pth__arc.pth_center_x ;
      coord_ptr[2*count + 1] = path->pth__specific.pth__arc.pth_center_y ;
      count++;
      coord_ptr[2*count] = coord_ptr[0];
      coord_ptr[2*count + 1] = coord_ptr[1];
      count++;
      (* rotated_path)->pth__specific.pth__xy.pth_pairs = count;
      }

   if ( !(arc->arc_obj.obj_flags  & ddif_m_arc_pie_arc))
      {
      /*
      ** It's a pie arc add two more sets of coordinates
      */
      coord_ptr[2*count] = coord_ptr[0];
      coord_ptr[2*count + 1] = coord_ptr[1];
      count++;
      (* rotated_path)->pth__specific.pth__xy.pth_pairs = count;
      }

   arc_priv->rotated_path = * rotated_path;
   }

return(DVR_NORMAL);
}

CDAstatus
dvr_format_curve_object (vw, curve, state)
DvrViewerWidget  vw;
CRV curve;
STATEREF state;
{
OBJ_PRIVATE private;
PTH crv_path, ret_path;
unsigned long status;

/*
 * Check for a null path.  If we have a null path
 * return immediately since the Layout Engine
 * encountered some sort of error.
*/

if (curve->crv_obj.obj_path == 0)
    return(DVR_FATALERROR);

/*
 * We have a valid set of points so we can proceed.
*/

crv_path = curve->crv_obj.obj_path;

/*
 * Generate the polylines from the bezier curve data stored in the CRV
 * node.  Note that we have to check to see if this is a closed bezier
 * curve since closed bezier curves need to have (x1,x2) added in as the
 * final point on the curve.
*/

status = dvr_generate_curve (vw,
			     crv_path,
			     &ret_path,
			     ((curve->crv_obj.obj_flags & ddif_m_bez_close_curve) ? TRUE : FALSE));

if ( DVR_FAILURE( status) )
	return (status);

private = (OBJ_PRIVATE)(curve->crv_obj.obj_user_private);
private->orig_path = ret_path;
return(DVR_NORMAL);
}



CDAstatus
dvr_format_fill_object (vw, curve, state)
DvrViewerWidget  vw;
CRV curve;
STATEREF state;
{

/*
* Temporarily disable the current attempts to format and display
* Fill Area Sets, which are hanging the Viewer.  By returning a
* failure code here, the "no display" flag will be set in the
* calling function dvr_format_objects, disabling the subsequent
* display in the function dvr_display_objects which checks the flag.
*
* Note that we must return zero (or any even number) to cause DVR_FAILURE
* to recognize a failure; the current value of DVR_GRAPHICFAIL is a
* "success" indicator. (?!)
*/
return (0);

}





/******************************************************************************/
/* bblend   - taken from  GObE display list widget			      */
/*									      */
/*  FUNCTIONAL DESCRIPTION:						      */
/*									      */
/*	Compute cubic Bezier blending function				      */
/*									      */
/*  FORMAL PARAMETERS:							      */
/*									      */
/*	i		index of point being generated			      */
/*									      */
/*	u		the bezier control value 'u' (order)		      */
/*									      */
/*  IMPLICIT INPUTS:							      */
/*									      */
/*	"d_c" is an array of pre-computed values for the combinatorial	      */
/*	equation; the combinations of 4 things taken n at a time (for n from  */
/*	0 to 3).							      */
/*									      */
/*  IMPLICIT OUTPUTS:							      */
/*									      */
/*      none								      */
/*									      */
/*  FUNCTION VALUE:							      */
/*									      */
/*      none								      */
/*									      */
/*  SIDE EFFECTS:							      */
/*									      */
/*      none								      */
/*									      */
/******************************************************************************/
static double bblend (
    l_i,
    d_u
    )
    long		l_i;
    double		d_u;
    {
    static double d_c[4] = {1.0, 3.0, 3.0, 1.0};	/* 4Cn for 0 to 3 */
    long l_j;
    double d_v;

    d_v = d_c [l_i];
    for (l_j = 1; l_j <= l_i; l_j++) d_v *= d_u;
    for (l_j = 1; l_j <= 3 - l_i; l_j++) d_v *= (1 - d_u);
    return d_v;
    }

/******************************************************************************/
/* bezier  - taken from  GObE display list widget			      */
/*									      */
/*  FUNCTIONAL DESCRIPTION:						      */
/*									      */
/*	Generate one point of the cubic Bezier curve			      */
/*									      */
/*  FORMAL PARAMETERS:							      */
/*									      */
/*	sp		Bezier curve point (the point being generated)	      */
/*									      */
/*	u		the bezier control value 'u'			      */
/*									      */
/*	cp		the array of control points			      */
/*									      */
/*  IMPLICIT INPUTS:							      */
/*									      */
/*      none								      */
/*									      */
/*  IMPLICIT OUTPUTS:							      */
/*									      */
/*      none								      */
/*									      */
/*  FUNCTION VALUE:							      */
/*									      */
/*      none								      */
/*									      */
/*  SIDE EFFECTS:							      */
/*									      */
/*      none								      */
/*									      */
/******************************************************************************/
static void bezier (
    ar_sp,
    d_u,
    arr_cp
    )
    F_POINT	ar_sp;
    double	d_u;
    signlong	*arr_cp;
    {
    signlong l_i;
    double d_b;

    ar_sp->f_x = (float) 0.0;
    ar_sp->f_y = (float) 0.0;

    for (l_i = 0; l_i <= 3; l_i++)
	{
	d_b = bblend (l_i, d_u);
	ar_sp->f_x += (float) (arr_cp[l_i * 2] * d_b);
	ar_sp->f_y += (float) (arr_cp[l_i * 2 + 1] * d_b);
	};

    }

/******************************************************************************/
/*                                                                            */
/* dvr_generate_curve  - adapted from  GObE display list widget		      */
/*									      */
/*  FUNCTIONAL DESCRIPTION:						      */
/*									      */
/*	Generate a set of cubic Bezier curves from a list of control points.  */
/*	A cubic Bezier curve contains exactly 4 control points, however this  */
/*	generator supports a list of any number of curves; the first curve    */
/*	uses the first four control points in the array, and succeeding	      */
/*	curves each use the fourth point of the preceeding curve, plus an     */
/*	additional 3 control points.					      */
/*									      */
/*  FORMAL PARAMETERS:							      */
/*									      */
/*	crv_path   	    the path of the control points of a bezier curve  */
/* 	lin_path   	    the path of a polyline approximation of the curve */
/*	closed_bezier_flag  the bezier curve must be closed		      */
/*									      */
/*  IMPLICIT INPUTS:							      */
/*									      */
/*	none								      */
/*									      */
/*  IMPLICIT OUTPUTS:							      */
/*									      */
/*	none								      */
/*									      */
/*  FUNCTION VALUE:							      */
/*									      */
/*	none								      */
/*									      */
/*  SIDE EFFECTS:							      */
/*									      */
/*	allocates memory for a generated polyline			      */
/*                                                                            */
/******************************************************************************/
unsigned long  dvr_generate_curve (
    vw,
    crv_path,
    ret_path,
    closed_bezier_flag
    )
    DvrViewerWidget  vw;
    PTH crv_path;		        /* a path containing a bezier curve */
    PTH *ret_path;			/* pointer to returned polyline path */
    boolean closed_bezier_flag;		/* indicates that we should close the bezier curve */
    {
    PTH lin_path;       	        /* the polyline path being generated */
    double d_i;
    long l_curves;     			/* Number of cubic Bezier sets */
    long l_cp;           	     	/* Control point index */
    long l_ccount;
    long l_gen_points;   	       	/* Number of generated points */
    f_point cp;
    long status;
    long alloc_size;     		/* Size of polyline path */

    signlong *crv_ptr;			/* Pointer to the xy pairs the source CRV node */
    signlong *lin_ptr;			/* Pointer to the xy pairs in the newly created LIN node */

    signlong closed_points[8];		/* 4 points that make up the last curve. */
    int closed_bezier = 0;		/* Closed bezier counter (0=Open, 1=Closed) - Don't make this a boolean */
    int i;				/* Loop index */

    /*
     * Check to see if we're dealing with a closed bezier.
     * If so set the closed_bezier value to 1.  This value
     * is set to 1 instead of TRUE since we will be using
     * the value in arithmetic.  Booleans can't technically
     * be used in math and as a result I don't want to incur
     * the wrath of some optimizing compiler in the future.
     */

    if (closed_bezier_flag)
	{
	 /*
	  * If the bezier curve is closed, we will use the first
	  * point as the last point on the curve.
	 */

	 closed_bezier = 1;
	}

    /*
     * Compute the number of bezier curves that we need to process.
     * Note that if we're dealing with a closed bezier, we need to add
     * one more pair of points since the final point was not specified1
     * in the point list.
     *
     * Note that this calculation is used to determine how many polylines
     * needs to be generated.
    */

    l_curves = (crv_path->pth__specific.pth__xy.pth_pairs + closed_bezier - 1) / 3;
    l_gen_points = (k_gen_count * l_curves) + 1;


    /*
     * Allocate and initialize a pth struct for the polyline, and set the
     * output parameter ret_path to point to it.
     */

    alloc_size = sizeof(*lin_path) + (l_gen_points * 2) * sizeof(long);
    status = dvr_alloc_memory (
	vw,
	alloc_size,
	(void **) &lin_path );

    if (!DVRSuccess(status))
	return(status);

    *ret_path = lin_path;  /* put lin path into returned variable */
    lin_path->pth_str.str_length = alloc_size;
    QUE_INIT(lin_path->pth_str.str_queue);
    QUE_INIT(lin_path->pth__specific.pth__xy.pth_qjoin);
    lin_path->pth_type = pth_lin;
    lin_path->pth__specific.pth__xy.pth_pairs = l_gen_points;

/*  status = dvr_record_alloc_path(vw, lin_path);   */


    /*
     * Set up pointers to x,y pairs, for convinience.  The lin_ptr will be
     * incremented, but the crv_ptr will be treated as a array.
     */
    crv_ptr = &(crv_path->pth__specific.pth__xy.pth_first_x);
    lin_ptr = &(lin_path->pth__specific.pth__xy.pth_first_x);

   /*
    * A Bezier curve is always guaranteed to pass through the first and last
    * control points.  Therefore, it's useless to spend the time generating
    * them... so just copy them from the control point array.
    */
    *lin_ptr = crv_path->pth__specific.pth__xy.pth_first_x;
    lin_ptr++;
    *lin_ptr = crv_path->pth__specific.pth__xy.pth_first_y;
    lin_ptr++;

    l_cp = 0;
    cp.f_x = (float) 0.0;
    cp.f_y = (float) 0.0;

    /*
     * Compute the bezier curves.  Note that we will not try to
     * compute the last curve if we're dealing with a closed
     * bezier since the final point is not part of the array.
    */

    for (l_ccount = 0; l_ccount < (l_curves - closed_bezier); l_ccount++)	/* Generate each curve*/
	{
	/*
	 * Generate the set of polyline points which will represent the cubic
	 * Bezier curve.
	 */
	for (d_i = d_step; d_i < 1.0; d_i += d_step)
	    {
	    bezier (				     	/* Generate a point */
		&cp,
		d_i,
		&crv_ptr[l_cp * 2]
		);

	    *lin_ptr = (signlong) cp.f_x;
	    lin_ptr++;
	    *lin_ptr = (signlong) cp.f_y;
	    lin_ptr++;
	    }

	/* Set the final point of cubic */

	*lin_ptr = crv_ptr[(l_cp + 3) * 2];
	lin_ptr++;
	*lin_ptr = crv_ptr[(l_cp + 3) * 2 + 1];
	lin_ptr++;

	l_cp += 3;		       	  /* Advance to first of next cubic */
	}

    /*
     * If we're dealing with a closed bezier compute the last set of
     * curves before returning to our caller.
    */

    if (closed_bezier)
	{
	 crv_ptr = &(crv_path->pth__specific.pth__xy.pth_first_x);

	 /*
	  * Copy points 0, 1 and 2 directly from the last set of points.
	  * Note that l_cp is still setup from exiting the previous loop.
	 */

	 for (i = 0; i < 6; i++)
	     {
	      closed_points[i] = crv_ptr[(l_cp * 2) + i];
	     }

	 /*
	  * Copy the final points from the head of the bezier point list
	  * since we're building closed bezier.
	 */

	 closed_points[6] = crv_ptr[0];
	 closed_points[7] = crv_ptr[1];

	 /*
	  * Once we've setup a fake array with the final four bezier control
	  * points call the computation logic to generate the points for us.
	 */

	 for (d_i = d_step; d_i < 1.0; d_i += d_step)
	     {
	     bezier (					/* Generate a point */
		 &cp,
		 d_i,
		 &closed_points[0]			/* Point to our altered point structure */
		 );

	     *lin_ptr = (signlong) cp.f_x;
	     lin_ptr++;
	     *lin_ptr = (signlong) cp.f_y;
	     lin_ptr++;
	     }

	 /*
	  * Set the final point of cubic.  Note that the final point of this
	  * curve is the first point specified in the document.
	 */

	 *lin_ptr = crv_ptr[0];
	 lin_ptr++;
	 *lin_ptr = crv_ptr[1];
	 lin_ptr++;
	}

    /*
     * Once we've finished processing the entire set of bezier control
     * points, return to the caller.
    */

    return (DVR_NORMAL);
    }

static void
dvr_smoothing_path (vw, poly_path, new_lin_path, flags)
DvrViewerWidget  vw;
PTH              poly_path;
PTH              * new_lin_path;
signlong         flags;
{

longword            count;
longword            loop;
PTH                 new_path;
PTH                 new_cur_path;
PTH                 cur_path;
signlong            * ctrl_ptr;
signlong            * ptr;
signlong            * new_ptr;
int                 curindex;
int                 startindex;
long                status;
long                alloc_size;         /* Size of polyline path */

/*
** Get the count of number of points in the poly line
*/
count =  poly_path->pth__specific.pth__xy.pth_pairs;
/*
** In the following loop recompute the coordinates of the control points. The
** control point is take to be 1/3 of
*/
alloc_size = sizeof(struct pth);

if ( flags & ddif_m_lin_close_polyline )
   {
   /*
   ** Compute the total number of point in each curve based on four control
   ** points. If the control points are increased or decreased then the
   ** following will not hold good. It's multiplied by 2 because of two
   ** coordinate, and it is also multiplied by count which will give the total
   ** number of curves that are to be fitted in a closed poly line.
   */
   alloc_size = alloc_size + (k_gen_count + 2) * 2 * count * sizeof(long);
   status = dvr_alloc_memory ( vw, alloc_size, (void **) &new_path );
   }
else
   {
   /*
   ** If it's not a closed ploygon the number of xy pairs will be the number of
   ** curves multiplied by number XY pair ( in the poly line ) - 2, multiplied
   ** by 2 and then number of points in each curve, plus 2 ( two end point)
   ** gives the total number of points
   */
   alloc_size = alloc_size + ( (k_gen_count + 1)*(count - 2 ) + 2) * 2
                             * sizeof(long);
   status = dvr_alloc_memory ( vw, alloc_size, (void **) &new_path );
   }

new_path->pth_str.str_length = alloc_size;
QUE_INIT(new_path->pth_str.str_queue);
new_path->pth_type = pth_lin;
new_path->pth__specific.pth__xy.pth_pairs = 0;
QUE_INIT(new_path->pth__specific.pth__xy.pth_qjoin);

/*
** Allocate memory to store the control points
*/
alloc_size = sizeof(struct pth) + (k_control_pts * 2 ) * sizeof(long);
status = dvr_alloc_memory ( vw, alloc_size, (void **) &cur_path );
cur_path->pth_str.str_length = alloc_size;
QUE_INIT(cur_path->pth_str.str_queue);
cur_path->pth_type = pth_lin;
cur_path->pth__specific.pth__xy.pth_pairs = k_control_pts;
QUE_INIT(cur_path->pth__specific.pth__xy.pth_qjoin);

/*
** set pointer to the start of x,y path coords
*/
ptr = &poly_path->pth__specific.pth__xy.pth_first_x;

if (flags & ddif_m_lin_close_polyline )
   curindex = (k_gen_count + 1) * 2;
else
   curindex = 2;

startindex = curindex;
/*
** In the following loop compute the control points btween the 2nd point and
** the last but one point in the ployline. After computing the control points
** call the DVR_GENERATE_CURVE routine to compute the coordinates between the
** control points and then copy these points into the new_path structure
*/
for ( loop = 1; loop < count - 1; loop++)
   {
   ctrl_ptr = &cur_path->pth__specific.pth__xy.pth_first_x;
   * ctrl_ptr++ = ptr [loop*2] + (ptr [loop*2 - 2] - ptr [loop*2])/2;
   * ctrl_ptr++ = ptr [loop*2 + 1] + (ptr [loop*2 - 1] - ptr [loop*2 + 1])/2;
   * ctrl_ptr++ = ptr [loop*2];
   * ctrl_ptr++ = ptr [loop*2 + 1];
   * ctrl_ptr++ = ptr [loop*2];
   * ctrl_ptr++ = ptr [loop*2 + 1];
   * ctrl_ptr++ = ptr [loop*2] + (ptr [loop*2 + 2] - ptr [loop*2])/2;
   * ctrl_ptr++ = ptr [loop*2 + 1] + (ptr [loop*2 + 3] - ptr [loop*2 + 1])/2;

   status = dvr_generate_curve ( vw, cur_path, &new_cur_path, FALSE);
   /*
   ** Copy the curve cooridnates into new_path structure and then increment
   ** the index
   */
   copy_curve_coords (new_path, new_cur_path, curindex);
   curindex = startindex + (int)(new_path->pth__specific.pth__xy.pth_pairs * 2);
   }

/*
** Check to see if it is a closed polyline, if it is then compute the control
** points for the line joining the first point and the last point and also
** the first control point for the line joining first point and the second
** point
*/
if ( flags & ddif_m_lin_close_polyline )
   {
   /*
   ** Compute control point between the first and the second & First and Last
   ** points in poly line
   */
   curindex = 0;
   ctrl_ptr = &(cur_path->pth__specific.pth__xy.pth_first_x);
   * ctrl_ptr++ = ptr [0] + (ptr [loop*2 ] - ptr [0])/2;
   * ctrl_ptr++ = ptr [1] + (ptr [loop*2 + 1] - ptr [1])/2;
   * ctrl_ptr++ = ptr [0];
   * ctrl_ptr++ = ptr [1];
   * ctrl_ptr++ = ptr [0];
   * ctrl_ptr++ = ptr [1];
   * ctrl_ptr++ = ptr [0] + (ptr [2] - ptr [0])/2;
   * ctrl_ptr++ = ptr [1] + (ptr [3] - ptr [1])/2;
   status = dvr_generate_curve ( vw, cur_path, &new_cur_path, FALSE);
   copy_curve_coords (new_path, new_cur_path, curindex);

   /*
   ** Compute control point between the Last and the Last but one & last and
   ** First points in poly line
   */
   curindex = (int)(new_path->pth__specific.pth__xy.pth_pairs * 2);
   ctrl_ptr = &(cur_path->pth__specific.pth__xy.pth_first_x);
   * ctrl_ptr++ = ptr [loop*2] + (ptr [loop*2 - 2] - ptr [loop*2])/2;
   * ctrl_ptr++ = ptr [loop*2 + 1] + (ptr [loop*2 - 1] - ptr [loop*2 + 1])/2;
   * ctrl_ptr++ = ptr [loop*2];
   * ctrl_ptr++ = ptr [loop*2 + 1];
   * ctrl_ptr++ = ptr [loop*2];
   * ctrl_ptr++ = ptr [loop*2 + 1];
   * ctrl_ptr++ = ptr [loop*2] + (ptr [0] - ptr [loop*2])/2;
   * ctrl_ptr++ = ptr [loop*2 + 1] + (ptr [1] - ptr [loop*2 + 1])/2;
   status = dvr_generate_curve ( vw, cur_path, &new_cur_path, FALSE);
   copy_curve_coords (new_path, new_cur_path, curindex);

   /*
   ** Since it's a closed polygon make the last point same as the first point
   */
   new_ptr = &new_path->pth__specific.pth__xy.pth_first_x;
   curindex = (int)(new_path->pth__specific.pth__xy.pth_pairs * 2);
   new_ptr[curindex] = new_ptr[0];
   new_ptr[curindex+1] = new_ptr[1];
   new_path->pth__specific.pth__xy.pth_pairs += 1;
   }
   else
      {
      /*
      ** It's not a closed polygon so, the first point in the new path is same
      ** as the first path in the poly line
      */
      new_ptr = &new_path->pth__specific.pth__xy.pth_first_x;
      * new_ptr++ = ptr[0];
      * new_ptr = ptr[1];
      new_path->pth__specific.pth__xy.pth_pairs += 1;
      /*
      ** It's not a closed polygon so, the last point in the new path is same
      ** as the last path in the poly line
      */
      new_ptr = &new_path->pth__specific.pth__xy.pth_first_x +
                new_path->pth__specific.pth__xy.pth_pairs * 2;
      * new_ptr++ = ptr[2*loop];
      * new_ptr++ = ptr[2*loop+1];
      /*
      ** Increment the number of pairs by one to account for the first and the
      ** last point in the poly line
      */
      new_path->pth__specific.pth__xy.pth_pairs += 1;
      }

      * new_lin_path = new_path;
}

static void copy_curve_coords (new_path, new_cur_path, curindex)
PTH                 new_path;
PTH                 new_cur_path;
int                 curindex;

{
signlong            * cur_ptr; /* Ptr to the first entry in XY path array */
signlong            * path_ptr;
int                 loop;

path_ptr = &new_path->pth__specific.pth__xy.pth_first_x + curindex;
cur_ptr = & new_cur_path->pth__specific.pth__xy.pth_first_x;

/*
** Copy the generated curve coordinates into the new path structrure
*/
for (loop = 0;
     loop < 2 * (int)new_cur_path->pth__specific.pth__xy.pth_pairs;
     loop++)
   * path_ptr ++ = * cur_ptr++;

new_path->pth__specific.pth__xy.pth_pairs +=
                     new_cur_path->pth__specific.pth__xy.pth_pairs;
}

static void
dvr_rounded_polyline (vw, poly_path, new_lin_path, flags)
DvrViewerWidget  vw;
PTH		 poly_path;
PTH              * new_lin_path;
signlong	 flags;

{

longword	    count;
longword	    loop;
PTH		    new_path;
PTH		    new_cur_path;
PTH		    cur_path;
signlong 	    * ctrl_ptr;
signlong	    * ptr;
signlong	    * new_ptr;
int		    curindex;
int		    startindex;
long		    status;
long		    alloc_size;     	/* Size of polyline path */
JOI		    curve_info;
struct que          que_head;
double		    xdiff, ydiff;
double		    distance;

/*
** Get the count of number of points in the poly line
*/
count =  poly_path->pth__specific.pth__xy.pth_pairs;

alloc_size = sizeof(struct pth);

if ( flags & ddif_m_lin_close_polyline )
   {
   /*
   ** Compute the total number of point in each curve based on four control
   ** points. If the control points are increased or decreased then the
   ** following will not hold good. It's multiplied by 2 because of two
   ** coordinate, and it is also multiplied by count which will give the total
   ** number of curves that are to be fitted in a closed poly line.
   */
   alloc_size = alloc_size + (k_gen_count + 1) * 2 * count * sizeof(signlong);
   status = dvr_alloc_memory ( vw, alloc_size, (void **) &new_path );
   }
else
   {
   /*
   ** If it's not a closed ploygon the number of xy pairs will be the number of
   ** curves multiplied by number XY pair ( in the poly line ) - 2, multiplied
   ** by 2 and then number of points in each curve, plus 2 ( two end point)
   ** gives the total number of points
   */
   alloc_size = alloc_size + ( (k_gen_count )*(count - 2 ) + 2) * 2
			     * sizeof(signlong);
   status = cda_malloc ( (CDAsize *) &alloc_size, (CDAaddress *) &new_path, 0L );
   }

new_path->pth_str.str_length = alloc_size;
QUE_INIT(new_path->pth_str.str_queue);
new_path->pth_type = pth_lin;
new_path->pth__specific.pth__xy.pth_pairs = 0;
QUE_INIT(new_path->pth__specific.pth__xy.pth_qjoin);

/*
** Allocate memory to store the control points
*/
alloc_size = sizeof(struct pth) + (k_control_pts * 2 ) * sizeof(signlong);
status = dvr_alloc_memory ( vw, alloc_size, (void **) &cur_path );
cur_path->pth_str.str_length = alloc_size;
QUE_INIT(cur_path->pth_str.str_queue);
cur_path->pth_type = pth_lin;
cur_path->pth__specific.pth__xy.pth_pairs = 0;;
QUE_INIT(cur_path->pth__specific.pth__xy.pth_qjoin);

/*
** Get the pointer to the head of the queue
*/
que_head = poly_path->pth__specific.pth__xy.pth_qjoin;

curve_info = (JOI) QUE_GET(que_head);

/*
** set pointer to the start of x,y path coords
*/
ptr = &poly_path->pth__specific.pth__xy.pth_first_x;

if (flags & ddif_m_lin_close_polyline )
   {
   curve_info = (JOI) QUE_GET_LAST(que_head);
   if (curve_info->joi_S == (float) 0.0 )
      curindex = 4;
   else
      if (curve_info->joi_S == (float) 1.0 )
         curindex = 6;
      else
         curindex = (k_gen_count ) * 2;
   }
else
   curindex = 2;

startindex = curindex;

curve_info = (JOI) QUE_GET(que_head);

/*
** In the following loop compute the control points btween the 2nd point and
** the last but one point in the ployline. After computing the control points
** call the DVR_GENERATE_CURVE routine to compute the coordinates between the
** control points and then copy these points into the new_path structure
*/

for ( loop = 1; loop < count - 1; loop++)
   {
   /*
   ** Get the Displacement values and the curve specification for the current
   ** vertex and advance the que pointer to the next element in the queue
   */
   cur_path->pth__specific.pth__xy.pth_pairs = 0;
   ctrl_ptr = &cur_path->pth__specific.pth__xy.pth_first_x;

   xdiff = ptr[loop*2] - ptr[loop*2 - 2];
   ydiff = ptr[loop*2 + 1] - ptr[loop*2 - 1];
   distance = xdiff*xdiff + ydiff*ydiff;
   distance = sqrt(distance);

   * ctrl_ptr++ = compute_coords( ptr[loop*2],
				  ptr[loop*2 - 2],
				  (double) curve_info->joi_d1,
				  distance
                                 );
   * ctrl_ptr++ = compute_coords( ptr[loop*2 + 1],
				  ptr[loop*2 - 1],
				  (double) curve_info->joi_d1,
				  distance
                                 );

   cur_path->pth__specific.pth__xy.pth_pairs += 1;

   if (curve_info->joi_S != (float) 0.0 )
      {
      * ctrl_ptr++ = ptr [loop*2];
      * ctrl_ptr++ = ptr [loop*2 + 1];
      cur_path->pth__specific.pth__xy.pth_pairs += 1;
      }

   xdiff = ptr[loop*2] - ptr[loop*2 + 2];
   ydiff = ptr[loop*2 + 1] - ptr[loop*2 + 3];
   distance = xdiff*xdiff + ydiff*ydiff;
   distance = sqrt(distance);
   * ctrl_ptr++ = compute_coords( ptr[loop*2],
				  ptr[loop*2 + 2],
				  (double) curve_info->joi_d2,
				  distance
                                 );
   * ctrl_ptr++ = compute_coords( ptr[loop*2 + 1],
				  ptr[loop*2 + 3],
				  (double) curve_info->joi_d2,
				  distance
                                 );

   cur_path->pth__specific.pth__xy.pth_pairs += 1;

   if ( (curve_info->joi_S != (float)0.0 ) && (curve_info->joi_S != (float)1.0))
      generate_curve_points (cur_path, curve_info->joi_S, &new_cur_path);
   /*
   ** Copy the curve cooridnates into new_path structure and then increment
   ** the index
   */

   if ( (curve_info->joi_S != (float)0.0 ) && (curve_info->joi_S != (float)1.0))
      copy_curve_coords (new_path, new_cur_path, curindex);
   else
      copy_curve_coords (new_path, cur_path, curindex);

   curindex = startindex + (int)(new_path->pth__specific.pth__xy.pth_pairs * 2);
   curve_info = ( JOI )curve_info->joi_str.str_queue.que_flink;
   }

/*
** Check to see if it is a closed polyline, if it is then compute the control
** points for the line joining the first point and the last point and also
** the first control point for the line joining first point and the second
** point
*/
if ( flags & ddif_m_lin_close_polyline )
   {
   /*
   ** Compute control point between the first and the second & First and Last
   ** points in poly line
   */
   curindex = 0;
   curve_info = (JOI) QUE_GET_LAST(que_head);
   cur_path->pth__specific.pth__xy.pth_pairs = 0;
   ctrl_ptr = &cur_path->pth__specific.pth__xy.pth_first_x;

   xdiff = ptr[0] - ptr[loop*2];
   ydiff = ptr[1] - ptr[loop*2 +1];
   distance = xdiff*xdiff + ydiff*ydiff;
   distance = sqrt(distance);

   * ctrl_ptr++ = compute_coords( ptr[0],
				  ptr[loop*2],
				  (double) curve_info->joi_d1,
				  distance
                                 );
   * ctrl_ptr++ = compute_coords( ptr[1],
				  ptr[loop*2 + 1],
				  (double) curve_info->joi_d1,
				  distance
                                 );

   cur_path->pth__specific.pth__xy.pth_pairs += 1;

   if (curve_info->joi_S != (float) 0.0 )
      {
      * ctrl_ptr++ = ptr [0];
      * ctrl_ptr++ = ptr [1];
      cur_path->pth__specific.pth__xy.pth_pairs += 1;
      }

   xdiff = ptr[0] - ptr[2];
   ydiff = ptr[1] - ptr[3];
   distance = xdiff*xdiff + ydiff*ydiff;
   distance = sqrt(distance);

   * ctrl_ptr++ = compute_coords( ptr[0],
				  ptr[2],
				  (double) curve_info->joi_d2,
				  distance
                                 );
   * ctrl_ptr++ = compute_coords( ptr[1],
				  ptr[3],
				  (double) curve_info->joi_d2,
				  distance
                                 );
   cur_path->pth__specific.pth__xy.pth_pairs += 1;

   if ( (curve_info->joi_S != (float)0.0 ) && (curve_info->joi_S != (float)1.0))
      generate_curve_points (cur_path, curve_info->joi_S, &new_cur_path);

   if ( (curve_info->joi_S != (float)0.0 ) && (curve_info->joi_S != (float)1.0))
      copy_curve_coords (new_path, new_cur_path, curindex);
   else
      copy_curve_coords (new_path, cur_path, curindex);

   curve_info = ( JOI )curve_info->joi_str.str_queue.que_blink;

   /*
   ** Compute control point between the Last and the Last but one & last and
   ** First points in poly line
   */
   curindex = (int) (new_path->pth__specific.pth__xy.pth_pairs * 2);
   ctrl_ptr = &(cur_path->pth__specific.pth__xy.pth_first_x);
   cur_path->pth__specific.pth__xy.pth_pairs = 0;

   xdiff = ptr[loop*2] - ptr[loop*2 - 2];
   ydiff = ptr[loop*2 + 1] - ptr[loop*2 - 1];
   distance = xdiff*xdiff + ydiff*ydiff;
   distance = sqrt(distance);

   * ctrl_ptr++ = compute_coords( ptr[loop*2],
				  ptr[loop*2 -2],
				  (double) curve_info->joi_d1,
				  distance
                                 );
   * ctrl_ptr++ = compute_coords( ptr[loop*2 +1],
				  ptr[loop*2 - 1],
				  (double) curve_info->joi_d1,
				  distance
                                 );
   cur_path->pth__specific.pth__xy.pth_pairs += 1;

   xdiff = ptr[loop*2] - ptr[0];
   ydiff = ptr[loop*2 + 1] - ptr[1];
   distance = xdiff*xdiff + ydiff*ydiff;
   distance = sqrt(distance);

   if (curve_info->joi_S != (float)0.0 )
      {
      * ctrl_ptr++ = ptr [2*loop];
      * ctrl_ptr++ = ptr [2*loop+1];
      cur_path->pth__specific.pth__xy.pth_pairs += 1;
      }

   * ctrl_ptr++ = compute_coords( ptr[loop*2],
				  ptr[0],
				  (double) curve_info->joi_d2,
				  distance
                                 );

   * ctrl_ptr++ = compute_coords( ptr[loop*2 +1],
				  ptr[1],
				  (double) curve_info->joi_d2,
				  distance
                                 );

   cur_path->pth__specific.pth__xy.pth_pairs += 1;

   if ( (curve_info->joi_S != (float)0.0 ) && (curve_info->joi_S != (float)1.0))
      generate_curve_points (cur_path, curve_info->joi_S, &new_cur_path);

   if ( (curve_info->joi_S != (float)0.0 ) && (curve_info->joi_S != (float)1.0))
      copy_curve_coords (new_path, new_cur_path, curindex);
   else
      copy_curve_coords (new_path, cur_path, curindex);
   /*
   ** Since it's a closed polygon make the last point same as the first point
   */
   new_ptr = &new_path->pth__specific.pth__xy.pth_first_x;
   curindex = (int) (new_path->pth__specific.pth__xy.pth_pairs * 2);
   new_ptr[curindex] = new_ptr[0];
   new_ptr[curindex+1] = new_ptr[1];
   new_path->pth__specific.pth__xy.pth_pairs += 1;
   }
   else
      {
      /*
      ** It's not a closed polygon so, the first point in the new path is same
      ** as the first path in the poly line
      */
      new_ptr = &new_path->pth__specific.pth__xy.pth_first_x;
      * new_ptr++ = ptr[0];
      * new_ptr = ptr[1];
      new_path->pth__specific.pth__xy.pth_pairs += 1;
      /*
      ** It's not a closed polygon so, the last point in the new path is same
      ** as the last path in the poly line
      */
      new_ptr = &new_path->pth__specific.pth__xy.pth_first_x +
                new_path->pth__specific.pth__xy.pth_pairs * 2;
      * new_ptr++ = ptr[2*loop];
      * new_ptr++ = ptr[2*loop+1];
      /*
      ** Increment the number of pairs by one to account for the first and the
      ** last point in the poly line
      */
      new_path->pth__specific.pth__xy.pth_pairs += 1;
      }

      * new_lin_path = new_path;
      alloc_size =  cur_path->pth_str.str_length;
      status = DVR_DEALLOCATE_MEMORY( alloc_size, &cur_path );
}

static signlong
compute_coords (point1, point2, disp, distance)
signlong	    point1;
signlong	    point2;
double		    disp;
double		    distance;

{

signlong	temp_x;

temp_x = (signlong)(abs((int)(point1 - point2))* disp/distance);

if ( point1 > point2)
   return (point1 - temp_x);
else
   if ( point1 < point2)
      return(point1 + temp_x);
   else
      return(point1);
}


#if CDA_EXPAND_PROTO == 1
static void
generate_curve_points (PTH cur_path, float curve_type, PTH *new_cur_path)

#else
static void
generate_curve_points (cur_path, curve_type, new_cur_path)
PTH		 cur_path;
float		 curve_type;
PTH		 * new_cur_path;
#endif

{
long		 alloc_size;     	/* Size of polyline path */
PTH		 curve_pth;
float		 weight;
signlong	 * curve_ptr; /* Ptr to the first entry in XY path array */
signlong	 * cur_ptr;
float		 deno;
float		 nume;
float		 interval;
float		 tee = (float)0.0;
float		 teesquare;
float		 oneminustee;
float		 oneminusteesqu;
long		 status;
int		 loop;

/*
** Allocate memory to store the gnenerated coodinates
*/
alloc_size = sizeof(struct pth) + (k_gen_count ) * 2 * sizeof(signlong);
status = cda_malloc ( (CDAsize *) &alloc_size, (CDAaddress *) &curve_pth, 0L );
curve_pth->pth_str.str_length = alloc_size;
QUE_INIT(curve_pth->pth_str.str_queue);
QUE_INIT(curve_pth->pth__specific.pth__xy.pth_qjoin);
curve_pth->pth_type = pth_lin;
curve_pth->pth__specific.pth__xy.pth_pairs = k_gen_count;
cur_ptr = &cur_path->pth__specific.pth__xy.pth_first_x;
curve_ptr = &curve_pth->pth__specific.pth__xy.pth_first_x;
interval= (float)(k_tmax/(float)k_gen_count);
/*
** Compute the weight of the curve
*/
weight = (float)curve_type/((float)1.0 - (float)curve_type);
/*
** generate the coordinates
*/
for ( loop = 0; loop < k_gen_count; loop++)
   {
   teesquare = tee * tee;
   oneminustee = 1 - tee;
   oneminusteesqu = oneminustee * oneminustee;

   deno = oneminusteesqu  + (2* tee * oneminustee * weight) + teesquare;
   nume = (oneminusteesqu * cur_ptr[0]) +
          ( 2* tee * oneminustee * weight * cur_ptr[2]) +
          ( teesquare * cur_ptr[4]);

   * curve_ptr++ = (signlong) (nume/deno);

   nume = (oneminusteesqu * cur_ptr[1]) +
          ( 2* tee * oneminustee * weight * cur_ptr[3]) +
          ( teesquare * cur_ptr[5]);
   * curve_ptr++ = (signlong) (nume/deno);
   tee = tee + interval;

   }
  * new_cur_path = curve_pth;
}

/*
 * FUNCTION:
 *      Draws a polyline with an arrowhead.
 * INPUTS:
 *      path = polyline path
 *      gattr = graphics attributes for the polyline
 *	arrow_type = Type of the arrow and it's location
 *      arrow_width = Width of an unfilled arrow head
 *      head1 = First arrow head coordinates array
 *      head2 = Second arrow head coordinates array
 *
 * RESULTS:
 *      updates vals off of pwi_gcv as needed
 * NOTES:
 *      The XDrawLines routine requires an array of shorts, not an array of ints
*/
double
dvr_drawpolylinearrow (path, gattr, arrow_type, arrow_width, head1, head2, vtex)
PTH		 path;
GAT              gattr;
int		 arrow_type;
CDAmeasurement	 * arrow_width;
PTH		 * head1;
PTH		 * head2;
Point		 ** vtex;

{
        int             i;
        double          arrow_length;
        CDAmeasurement  line_width;
	CDAsize         npnts;
	CDAmeasurement	* path_ptr;
	Point		* v;

        npnts = path->pth__specific.pth__xy.pth_pairs;
	line_width = gattr->gat_line_width;
        path_ptr = &path->pth__specific.pth__xy.pth_first_x;

        /* Make a new vertex list structure that will be modified, and add in
        * the offset.
        */
        v = (Point *) CdaMalloc(sizeof(Point)*(npnts+1));
        for (i = 0; (CDAsize) i < npnts ; i++)
           {
           v[i].x = * path_ptr++;
           v[i].y = * path_ptr++;
           }
        * vtex = v;
        arrow_length = dvs_calculate_arrow_head_len (v, npnts, arrow_type,
                                line_width, head1, head2, arrow_width);
        return(arrow_length);

}                                /* end routine */

/*
 * FUNCTION:
 *      Draws an Arc with arrowhead.
 * INPUTS:
 *      path = Arc path
 *      gattr = graphics attributes for the polyline
 *	arrow_type = Type of the arrow and it's location
 *      arrow_width = Width of an unfilled arrow head
 *      head1 = First arrow head coordinates array
 *      head2 = Second arrow head coordinates array
 *
 * RESULTS:
 *      updates vals off of pwi_gcv as needed
*/

double
dvr_drawarcarrow( path, gattr, arc_info, arrow, arrow_width, head1, head2)
PTH		 path;
GAT              gattr;
Arc_struct	 * arc_info;
int		 arrow;
CDAmeasurement   * arrow_width;
PTH		 * head1;
PTH		 * head2;

{
double          arrow_length;
CDAmeasurement	line_width;

line_width = gattr->gat_line_width;

/*
** Copy the arc related info into a structure and also compute the height and
** width of the bounding box
*/

arc_info->x_radius = path->pth__specific.pth__arc.pth_radius_x;
arc_info->y_radius_delta = path->pth__specific.pth__arc.pth_radius_y;
arc_info->y_radius = path->pth__specific.pth__arc.pth_radius_y +
			path->pth__specific.pth__arc.pth_radius_x;
arc_info->center_x = path->pth__specific.pth__arc.pth_center_x;
arc_info->center_y = path->pth__specific.pth__arc.pth_center_y;
arc_info->width = path->pth__specific.pth__arc.pth_radius_x * 2;
arc_info->height = arc_info->y_radius * 2;
arc_info->start_angle = path->pth__specific.pth__arc.pth_start/100;
arc_info->extent = path->pth__specific.pth__arc.pth_extent/100;
arc_info->rotation = path->pth__specific.pth__arc.pth_rotation/100;

arrow_length = dvs_calculatearcarrowheads (arc_info, arrow, line_width,
					    arrow_width, head1, head2);
return(arrow_length);
}                                /* end routine */

static long
shorten_arrow_path (vw, state, gattr, arrow_struct, poly_path, v, arrow_length,
				    arrow_type, round_or_smooth)

DvrViewerWidget vw;
STATEREF	state;
GAT		gattr;
Arr_info	* arrow_struct;
PTH		poly_path;
Point		* v;
double		arrow_length;
int		arrow_type;
boolean		round_or_smooth;

{
CDAsize		count;
PTH		arr_path;
signlong	* arr_path_ptr;
double		head_length;
double		apex_angle;
double		base_width;
CDAmeasurement  arrow_width;

/*
** Get the count of number of points in the poly line
*/
count =  poly_path->pth__specific.pth__xy.pth_pairs;

/*
** Allocate a PTH stucture and copy the contents of the  input PTH strucutre
** poly_path
*/
arr_path = dve__copy_pth_struct( poly_path);

head_length = arrow_length;

/*
** Check to see if it is an unfilled arrow, if it is then reduce the
** head length. Compute the apex angle of the arrow head from the base
** width of the arrow head and the arrow head length
*/

if ((arrow_type ==  START_UNFILLED_ARROW) ||(arrow_type ==  END_UNFILLED_ARROW)
    || (arrow_type ==  BOTH_UNFILLED_ARROWS) )
   {
   base_width = dvs_arr_compute_base_width (gattr->gat_line_width, UNFILLED,
							& arrow_width);
   apex_angle = atan (base_width/(2*head_length));
   head_length = arrow_width / (2*sin (apex_angle));
   }

if ( round_or_smooth)
   shorten_round_smooth_path ( arr_path, poly_path, head_length, arrow_type);
else
   {
   /*
   ** Change the Starting and Ending Coordinates Of the Polyline
   */
   arr_path_ptr = &arr_path->pth__specific.pth__xy.pth_first_x;
   * arr_path_ptr = v[0].x;
   * (arr_path_ptr + 1) = v[0].y;
   * (arr_path_ptr + (2*count -2)) = v[count-1].x;
   * (arr_path_ptr + (2*count -1)) = v[count-1].y;
   }

arrow_struct->arrow_path = arr_path;

return(DVR_NORMAL);

}

static void
shorten_round_smooth_path ( arr_path, poly_path, head_length, arrow_type)
PTH		arr_path;
PTH		poly_path;
double		head_length;
int		arrow_type;

{
int		count;
int		end_index;
int		start_index;

count =  (int) poly_path->pth__specific.pth__xy.pth_pairs;
switch (arrow_type)
   {
   case START_FILLED_ARROW :
   case START_UNFILLED_ARROW :
      /*
      ** find the index into the path array from where the poly line path starts
      ** for arrow head
      */
      start_index = find_the_index (poly_path, head_length, START);
      /*
      ** Copy the reduced path into arr_path structure
      */
      copy_reduced_path (arr_path, poly_path, head_length, start_index, count, arrow_type);
      break;

   case END_FILLED_ARROW :
   case END_UNFILLED_ARROW :
      /*
      ** find the index into the path array from where the poly line path starts
      ** for arrow head
      */
      start_index = 0;
      end_index = find_the_index (poly_path,head_length, END);
      copy_reduced_path (arr_path, poly_path, head_length, start_index, end_index, arrow_type);
      break;
   case BOTH_FILLED_ARROWS:
   case BOTH_UNFILLED_ARROWS:
      /*
      ** find the index into the path array from where the poly line path starts
      ** for arrow head
      */
      start_index = find_the_index (poly_path, head_length, START);
      end_index = find_the_index (poly_path, head_length, END);
      /*
      ** Copy the reduced path into arr_path structure
      */
      copy_reduced_path (arr_path, poly_path, head_length, start_index,
						end_index, arrow_type);
      break;

   }
}

static int
find_the_index ( poly_path, head_length, direction)
PTH             poly_path;
double          head_length;
int             direction;

{
signlong	* path_ptr;
int		count;
int		loop;
double		xdiff, ydiff;

count =  (int) poly_path->pth__specific.pth__xy.pth_pairs;
path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x;

if (direction == START)
   {
   for (loop = 1; loop < count - 1; loop++)
      {
      xdiff = path_ptr [0] - path_ptr[2*loop];
      ydiff = path_ptr [1] - path_ptr[2*loop + 1];
      if ((sqrt(xdiff*xdiff + ydiff*ydiff)) > head_length)
        return (loop);
      }
   }
else
   {
   for (loop = count-1; loop >= 1; loop--)

      {
      xdiff = path_ptr [2*count - 2] - path_ptr[2*loop - 2];
      ydiff = path_ptr [2*count - 1] - path_ptr[2*loop - 1];
      if ((sqrt(xdiff*xdiff + ydiff*ydiff)) > head_length)
        return (loop);
      }
   }
}

static void
copy_reduced_path (arr_path, poly_path, head_length, start_index, end_index, arrow_type)
PTH		arr_path;
PTH		poly_path;
double		head_length;
int		start_index;
int		end_index;
int		arrow_type;

{
signlong	* org_path_ptr;
signlong	* arr_path_ptr;
int		count;
int		loop;
double		xdiff, ydiff;
double		distance;

count =  (int) poly_path->pth__specific.pth__xy.pth_pairs;
switch (arrow_type)
   {
   case START_FILLED_ARROW :
   case START_UNFILLED_ARROW :
      arr_path_ptr = & arr_path->pth__specific.pth__xy.pth_first_x + 2;
      org_path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x +
							(start_index * 2);
      for (loop = start_index; loop < end_index; loop++)
         {
         * arr_path_ptr++ = * org_path_ptr++;
         * arr_path_ptr++ = * org_path_ptr++;
	 }
      /*
      */
      org_path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x;
      xdiff = org_path_ptr[0] - org_path_ptr[2*start_index];
      ydiff = org_path_ptr[1] - org_path_ptr[2*start_index + 1];
      distance = sqrt(xdiff*xdiff + ydiff*ydiff);
      arr_path_ptr = & arr_path->pth__specific.pth__xy.pth_first_x ;
      * arr_path_ptr++ = compute_coords ( org_path_ptr[0],
					  org_path_ptr[2*start_index],
					  head_length,
					  distance);
      * arr_path_ptr++ = compute_coords ( org_path_ptr[1],
					  org_path_ptr[2*start_index + 1],
					  head_length,
					  distance);
      /*
      ** reduce the number of XY pair count in the modified path
      */
      arr_path->pth__specific.pth__xy.pth_pairs = count - (start_index -1);


      break;

   case END_FILLED_ARROW :
   case END_UNFILLED_ARROW :
      arr_path_ptr = & arr_path->pth__specific.pth__xy.pth_first_x ;
      org_path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x;

      for (loop = start_index; loop < end_index; loop++)
         {
         * arr_path_ptr++ = * org_path_ptr++;
         * arr_path_ptr++ = * org_path_ptr++;
	 }
      /*
      */
      org_path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x;
      xdiff = org_path_ptr[2*count - 2] - org_path_ptr[2*end_index - 2];
      ydiff = org_path_ptr[2*count - 1] - org_path_ptr[2*end_index - 1];
      distance = sqrt(xdiff*xdiff + ydiff*ydiff);
      arr_path_ptr = & arr_path->pth__specific.pth__xy.pth_first_x +
							    (2*end_index);
      /*
      ** Compute the X & Y coordinate which is one head length away
      */
      * arr_path_ptr++ = compute_coords ( org_path_ptr[2*count - 2],
					  org_path_ptr[2*end_index - 2],
					  head_length,
					  distance);
      * arr_path_ptr++ = compute_coords ( org_path_ptr[2*count - 1],
					  org_path_ptr[2*end_index - 1],
					  head_length,
					  distance);

      /*
      ** Total number of XY pair is end index plus one, for the new
      ** coordinates that is added.
      */
      arr_path->pth__specific.pth__xy.pth_pairs = end_index + 1;
      break;

   case BOTH_FILLED_ARROWS:
   case BOTH_UNFILLED_ARROWS:

      arr_path_ptr = & arr_path->pth__specific.pth__xy.pth_first_x + 2;
      org_path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x +
							(start_index * 2);
      /*
      ** Copy the coordinates form start index to end index
      */
      for (loop = start_index; loop < end_index; loop++)
         {
         * arr_path_ptr++ = * org_path_ptr++;
         * arr_path_ptr++ = * org_path_ptr++;
	 }

      org_path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x;
      xdiff = org_path_ptr[0] - org_path_ptr[2*start_index];
      ydiff = org_path_ptr[1] - org_path_ptr[2*start_index + 1];
      distance = sqrt(xdiff*xdiff + ydiff*ydiff);
      arr_path_ptr = & arr_path->pth__specific.pth__xy.pth_first_x;
      /*
      ** Compute the X & Y coordinate which is one head length away
      */
      * arr_path_ptr++ = compute_coords ( org_path_ptr[0],
					  org_path_ptr[2*start_index],
					  head_length,
					  distance);
      * arr_path_ptr++ = compute_coords ( org_path_ptr[1],
					  org_path_ptr[2*start_index + 1],
					  head_length,
					  distance);

      org_path_ptr = & poly_path->pth__specific.pth__xy.pth_first_x;
      xdiff = org_path_ptr[2*count - 2] - org_path_ptr[2*end_index - 2];
      ydiff = org_path_ptr[2*count - 1] - org_path_ptr[2*end_index - 1];
      distance = sqrt(xdiff*xdiff + ydiff*ydiff);
      arr_path_ptr = & arr_path->pth__specific.pth__xy.pth_first_x +
							    (2*end_index);
      /*
      ** Compute the X & Y coordinate which is one head length away
      */
      * arr_path_ptr++ = compute_coords ( org_path_ptr[2*count - 2],
					  org_path_ptr[2*end_index - 2],
					  head_length,
					  distance);
      * arr_path_ptr++ = compute_coords ( org_path_ptr[2*count - 1],
					  org_path_ptr[2*end_index - 1],
					  head_length,
					  distance);

      /*
      ** Total number of XY pair is differnce in the starting index and end
      ** index plus two
      */
      arr_path->pth__specific.pth__xy.pth_pairs = abs (end_index - start_index)
								     + 2;
      break;
   }

}

/*
** This routine takes in the Center coordiantes for an elliptical arc,
** it's height, width, and the rotation angle and generates coordinates for
** a rotated ellipse
*/
unsigned long
dvr_rotated_ellipse (vw, path, new_path )
    DvrViewerWidget	vw;
    PTH			path;
    PTH			* new_path;

{
signlong		angle;		/* Loop Variable over angle	*/
signlong		end_angle;      /* End angle of the arc		*/
signlong		start_angle;    /* Start angle of the arc	*/
int			count;		/* No. Of X,Y pairs		*/
signlong		xcoord;         /* X coordinate value		*/
signlong		ycoord;         /* Y coordinate value		*/
double			tmp_angle;      /* Angle value in double	*/
double			sinrotation;    /* Sin vlaue for rotation angle */
double			cosrotation;    /* Cos vlaue for rotation angle */
long			alloc_size;	/* Size Of the PTH structure	*/
unsigned int		height;		/* Height Of Outer Rectangle	*/
unsigned int		width;		/* Width Of Outer Rectangle	*/
signlong		* coord_ptr;    /* Ptr to The Coordinates       */
PTH			cur_path;	/* Path strucutre		*/
unsigned long		status;

height = (unsigned int) (path->pth__specific.pth__arc.pth_radius_y +
			 path->pth__specific.pth__arc.pth_radius_x )*2;
width =  (unsigned int) path->pth__specific.pth__arc.pth_radius_x * 2;

/*
** Initialize start and end angles of the arc
*/
end_angle = start_angle = 0;

/*
** Make sure that the extent is never greater than 360
*/
if ( abs((int)(path->pth__specific.pth__arc.pth_extent/100)) >= 360 )
   {
   angle = 360;
   if (path->pth__specific.pth__arc.pth_extent/100 < 0)
     angle = -angle;
   }
else
   angle = path->pth__specific.pth__arc.pth_extent/100;

if ((angle != 0) && ((abs((int)angle)%360) == 0))
   angle = 360;
else
   angle = abs((int)angle)%360;

if ( path->pth__specific.pth__arc.pth_extent/100 < 0)
   angle = -angle;

/*
** Get the the starting and ending angle of the arc
*/
dvs_arr_compute_the_angles ( path->pth__specific.pth__arc.pth_start/100,
			     angle,
			     & start_angle,
			     & end_angle);

/*
** Allocate memory to store the coordinates and Initialize the PTH Structure
** The number of X,Y Cooridnates pair is arc extent+ 1 and add two more pairs
** to take care off if the arc mode is a Pie_arc or a chord
*/
count = ((abs((int)angle)+ 3) * 2) * sizeof(signlong);
alloc_size = sizeof(struct pth) + count;
status = cda_malloc ( (CDAsize *) &alloc_size, (CDAaddress *) &cur_path, 0L );
if ( DVR_FAILURE ( status ))
   {
   /*
   ** Unexpected error
   */
   return (status);
   }
cur_path->pth_str.str_length = alloc_size;
cur_path->pth__specific.pth__xy.pth_pairs = 0;
cur_path->pth_type = pth_rotated;
QUE_INIT(cur_path->pth_str.str_queue);
QUE_INIT(cur_path->pth__specific.pth__xy.pth_qjoin);

/*
** Compute the sin & cos values for the rotation angle
*/
tmp_angle = (path->pth__specific.pth__arc.pth_rotation/100) * RADIANS;
sinrotation =  sin(tmp_angle);
cosrotation =  cos(tmp_angle);

count = 0;
width = width/2;
height = height/2;
coord_ptr = & cur_path->pth__specific.pth__xy.pth_first_x;

for (angle = start_angle; angle <= end_angle; angle++)
   {
   tmp_angle = ((double)angle) * RADIANS;
   xcoord = (signlong) (width * cos(tmp_angle));
   ycoord = (signlong) (height * sin(tmp_angle));
   * coord_ptr++ = (signlong) ( path->pth__specific.pth__arc.pth_center_x +
		                xcoord*cosrotation - ycoord*sinrotation );
   * coord_ptr++ = (signlong) ( path->pth__specific.pth__arc.pth_center_y +
		                (xcoord*sinrotation + ycoord*cosrotation) );
   count++;
   }

cur_path->pth__specific.pth__xy.pth_pairs = count;

* new_path = cur_path;
/*
** Return Success status
*/
return (DVR_NORMAL);
}
