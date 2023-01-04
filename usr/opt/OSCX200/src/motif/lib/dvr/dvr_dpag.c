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
#define Module DVR_DPAG
#define Ident  "V02-027"

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
**	dvr_dpag.c
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**      Displays page structure.
**
**
** AUTHORS:
**      Marc A. Carignan,  27-Dec-1988
**
**
** MODIFIED BY:
**
**	V02-027		RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>
**
**	V02-026		KLM001		Kevin McBride		12-Jun-1992
**		use engine user private fields as CDAuserparam
**
**	V02-025		DAM000		Dennis McEvoy		03-Jun-1992
**		use engine user private fields as CDAuserparam
**
**	V02-024		DAM0000		Dennis McEvoy		02-jun-1992
**		type cleanups for alpha/osf1
**
**      V02-023		RKN0000		Ram Kumar Nori		02-Jun-1992
**		Make a call to dvr_reset_gc routine from dvr_display_fragment_
**		object routine.
**
**	V02-022		ECR0000		Elizabeth C. Rust	30-Mar-1992
**		merged in audio code
**
**	V02-021		DAM0000		Dennis McEvoy		05-aug-1991
**		renamed header files, removed dollar signs
**
**	V02-020		SJM0000		Stephen Munyan		10-Jul-1991
**		Fixed child processing logic such that it only skips processing
**		children if the non-image proto flag is set for object types
**		galley and text.
**
**	V02-019		RAM0000		Ralph A. Mack		31-May-1991
**		Add support for MS-Windows fonts
**
**      V02-018	        DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**
**	V02-017		RAM0000		Ralph A. Mack		24-Apr-1991
**		Add #ifdefs for MS-Windows
**
**	V02-016		CJR0000		Chris Ralto		04-Apr-1991
**		Add two args to dvr_stroke_path calls, for draw pattern.
**
**	V02-015		DAM0000		Dennis McEvoy		03-apr-1991
**		cleanup typedefs
**
**	V02-014		SJM0000		Stephen Munyan		28-Mar-1991
**		Fix clipping bug that occurs on in-line frames if the
**		frame is empty.  This bug causes all other objects until
**		we reach the end of the line to be clipped.
**
**	V02-013		SJM0000		Stephen Munyan		26-Mar-1991
**		Added code to ignore obj_tab in the Layout Engine
**		structures.
**
**	V02-012		DAM0012		Dennis McEvoy		21-mar-1991
**		add support for changebar display
**
**	V02-011		MHB0000		Mark Bramhall		 5-Mar-1991
**		Remove temporary flag definition.
**
**	V02-010		RTG0001		Dick Gumbel		05-Mar-1991
**		Cleanup #include's
**
**	V02-009		DAM0009		Dennis McEvoy		21-dec-1990
**		fix compile warning for os/2
**
**	V02-008		DAM0008		Dennis McEvoy		21-dec-1990
**		fix default font handling for os/2
**
**	V02-007		BFB0000		Barbara F. Bazemore	14-Nov-1990
**		Make sure ImageFail status gets out to the Diagnostic
**		Info box.
**
**	V02-006		SJM0000		Stephen Munyan		27-Jun-1990
**		Conversion to Motif
**
**	V02-005		DAM0005		Dennis McEvoy		16-jul-1990
**		use y dpi for non 1:1 pixel displays
**
**	V02-004		DAM0004		Dennis McEvoy		05-mar-1990
**		changes for OS/2 port
**
**	V02-003		DAM0003		Dennis McEvoy		25-Jul-1989
**		check for NULL private object before trying to display
**
**	V02-002		PMJ0001		Patricia M. Justus	15-Jun-1989
**		Implement new flag for imaging galleys.
**
**	V02-001		DAM0001		Dennis A. McEvoy	27-Apr-1989
**		Remove call to dvr_get_font_info(); this is now done
**		in format phase so we can recalculate x-stretch based
**		on X display font metrics;
**--
*/


/*
 *  include files
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

#include "dvrwdef.h"			/* Public Defns w/dvr_include */
#include "dvrwint.h"			/* Viewer Constants and structures  */
#include "dvrwptp.h"			/* dvr windowing prototypes */


/* local routine prototypes */

PROTO( CDAstatus dvr_display_page_object,
		(DvrViewerWidget,
		 PAG) );

PROTO( CDAstatus dvr_display_page_frame,
		(DvrViewerWidget,
		 FRM) );

PROTO( CDAstatus  dvr_display_objects,
		(DvrViewerWidget,
		 OBJ,
		 BBOX_REF) );

PROTO( CDAstatus dvr_display_frame_object,
		(DvrViewerWidget,
		 FRM,
		 CDAmeasurement,
		 CDAmeasurement) );

PROTO( CDAstatus dvr_display_galley_object,
		(DvrViewerWidget,
		 GLY,
		 CDAmeasurement,
		 CDAmeasurement) );

PROTO( CDAstatus dvr_display_fragment_object,
		(DvrViewerWidget,
		 FRG,
		 CDAmeasurement,
		 CDAmeasurement) );

PROTO( CDAstatus dvr_display_text_object,
		(DvrViewerWidget,
		 TXT,
		 CDAmeasurement,
		 CDAmeasurement) );

PROTO( boolean rectangular_path,
                (PTH,
                 BBOX_REF) );

PROTO( boolean clip_display_intersect,
                (BBOX_REF,
                 BBOX_REF) );




/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_page
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Display specified portion of current page.  If current page not yet
 *	formatted, calls format page routine before displaying page.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *      bbox_ul_x	- Bounding box upper left X coordinate
 *      bbox_ul_y	- Bounding box upper left Y coordinate
 *      bbox_lr_x	- Bounding box lower right X coordinate
 *      bbox_lr_y	- Bounding box lower right Y coordinate
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *	DVR_PAGENOTFOUND  Specified page not found
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_page (
    vw,
    bbox_ul_x,
    bbox_ul_y,
    bbox_lr_x,
    bbox_lr_y )

DvrViewerWidget vw;		/* Viewer widget context pointer */
CDAmeasurement bbox_ul_x;			/* Bounding box upper left X coordinate */
CDAmeasurement bbox_ul_y;			/* Bounding box upper left Y coordinate */
CDAmeasurement bbox_lr_x;			/* Bounding box lower right X coordinate */
CDAmeasurement bbox_lr_y;			/* Bounding box lower right Y coordinate */

{

    /* Local variables */
    DvrStruct     *dvr_struct;		/* DVR struct pointer */
    PAG           page;			/* Page structure */
    CDAmeasurement x_window_top;         /* X window position top */
    CDAmeasurement y_window_top;		/* Y window position top */
    BBOX_UL_LR	  window_bbox;
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;


    /*
     *	Set up display parameters necessary in DVR structure.
     */

#ifdef DEBUG
    /* Synchronize X output */
    (void) XSynchronize ( XtDisplay (vw->dvr_viewer.work_window), 1 );
#endif

#ifdef DEBUG_SYNCHRONIZE
    /* Synchronize X output */
    (void) XSynchronize ( XtDisplay (vw->dvr_viewer.work_window), 1 );
#endif

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    page = dvr_struct->current_page;

    /* Check if current page exists; if not, exit with page not found status */
    if ( page == NULL ) {
	/* No current page found; empty document? */
	return DVR_PAGENOTFOUND;
    }


    /*
     *	Current page found; process page.
     */


    /* Desired display bounding box area, offset by current X,Y window
       top position value, for proper comparison with the object bbox
       values (which are already set to display with this offset) */

    x_window_top = dvr_struct->Xtop;
    y_window_top = dvr_struct->Ytop;

    dvr_struct->display_bbox.bbox_ul_x = bbox_ul_x + x_window_top;
    dvr_struct->display_bbox.bbox_lr_x = bbox_lr_x + x_window_top;

#ifdef OS2 /* upper left 0,0 */
    dvr_struct->display_bbox.bbox_ul_y = bbox_lr_y + y_window_top;
    dvr_struct->display_bbox.bbox_lr_y = bbox_ul_y + y_window_top;
#else /* lower left 0,0 */
    dvr_struct->display_bbox.bbox_ul_y = bbox_ul_y + y_window_top;
    dvr_struct->display_bbox.bbox_lr_y = bbox_lr_y + y_window_top;
#endif

    /*
     *	Check to make sure current page has been formatted; if not, format
     *	now before proceeding to display page.
     */

    if ( page->pag_user_private == NULL ) {
	/* Page private structure not yet defined; indicates that page
	   has not yet been processed by the format page routines */

#ifdef DEBUG
printf ( "\nCurrent page to be displayed has not been formatted yet;" );
printf ( "\n  formatting page now before continuing with display...\n" );
#endif

	status = dvr_format_page ( vw );
	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error during page processing */
	    return status;
	}

#ifdef DEBUG
printf ( "Page formatting prior to page display completed\n");
#endif
    }


    /* set tile stipple */
    dvr_set_ts_origin(vw);

    /*
     *	Display portion of current page specified by the bounding box,
     *	described in screen pixel X coordinates.
     */

#ifdef DEBUG
printf ( "\nDisplaying page structure...\n" );
#endif

    status = dvr_display_page_object (
	vw,
	page );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /*
    **  Reset clip rectangle to entire window so XCopyArea will work.
    **	Note that dvr_set_clip adjusts for Xtop, Ytop.
    */

    window_bbox.bbox_ul_x = x_window_top;
    window_bbox.bbox_lr_x = WORK_WIDTH(vw)  + x_window_top;

#ifdef OS2
    /* OS/2 has a lower left 0,0 */
    window_bbox.bbox_lr_y = y_window_top;
    window_bbox.bbox_ul_y = WORK_HEIGHT(vw) + y_window_top;
#else
    window_bbox.bbox_ul_y = y_window_top;
    window_bbox.bbox_lr_y = WORK_HEIGHT(vw) + y_window_top;
#endif

    dvr_set_clip(&window_bbox, vw);

/*BEGIN AUDIO STUFF */
#ifdef CDA_AUDIO_SUPPORT
    dvr_display_audio_buttons(vw,page);
#endif
/*END AUDIO STUFF*/

#ifdef DEBUG
printf ( "\nEnd page displaying\n" );
#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_page_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays data on current page.  Limits the display operations to
 *	the specified display bounding box, if indicated by flag.  Bounding
 *	box can be NULL if flag is false.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	page		- Page structure
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_page_object (
    vw,
    page )

DvrViewerWidget vw;    		/* Viewer widget context pointer */
PAG page;     			/* Page structure */

{

    /* Local variables */
    PAG           proto_page;		/* Prototype page */
    FRM           frame;		/* Frame */
    CDAstatus status;
    BBOX_REF	  display_bbox;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
    **  Initialize clip rectangle to entire display bbox (exposed region).
    */
    display_bbox = &(vw->dvr_viewer.Dvr.display_bbox);
    dvr_set_clip(display_bbox, vw);

    /*
     *	Display prototype pages, if any.
     */


    if (( proto_page = page->pag_proto_page ) != NULL ) {
	/* Prototype page reference found; call routine recursively
	   to display prototype page */

#ifdef DEBUG
printf ( "\nDisplaying prototype page...\n" );
#endif


	status = dvr_display_page_object (
	    vw,
	    proto_page );

	/* reset clip rectangle */
	dvr_set_clip(display_bbox, vw);

 	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
       	    return status;
	}

#ifdef DEBUG
printf ( "End prototype page\n" );
#endif
    }


    /*
     *  First, display prototype page frame, if it exists, followed by
     *	the primary page frame, if it exists.  Order is arbitrary during
     *	the display processing phase.
     */

    /* Display prototype page frame first, if it exists */
    if (( frame = page->pag_proto_frame ) != NULL ) {

	/* Prototype page frame found */

#ifdef DEBUG
printf ( "\nDisplaying prototype page frame...\n" );
#endif

	status = dvr_display_page_frame (
	    vw,
	    frame );

	/* reset clip rectangle */
	dvr_set_clip(display_bbox, vw);

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
	    return status;
	}

#ifdef DEBUG
printf ( "End prototype page frame\n" );
#endif
    }


    /* Display primary page frame second, if it exists */
    if (( frame = page->pag_page_frame ) != NULL) {

	/* Primary page frame found */

#ifdef DEBUG
printf ( "\nDisplaying primary page frame...\n" );
#endif

	status = dvr_display_page_frame (
	    vw,
	    frame );

	/* reset clip rectangle */
	dvr_set_clip(display_bbox, vw);

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
	    return status;
	}

#ifdef DEBUG
printf ( "End primary page frame\n" );
#endif
    }


    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_page_frame
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays page frame graphics, if specified.  Displays page frame
 *	objects.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	page		- Page structure
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_page_frame (
    vw,
    frame )

DvrViewerWidget vw;		/* Viewer widget context pointer */
FRM frame;			/* Frame structure */

{

    /* Local variables */
    DvrStruct *dvr_struct;		/* DVR struct pointer */
    OBJ_PRIVATE object_private;		/* Object private data */
    BBOX_REF display_bbox;		/* Display bbox pointer */
    BBOX_UL_LR	clip_bbox;		/* clipping region */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;


    /*
     *	Attempt to display page frame unless its frame bounding box is
     *	not defined.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    object_private = (OBJ_PRIVATE) frame->frm_obj.obj_user_private;


    if ( !OBJECT_BBOX_DEFINED (object_private) ) {

	/* Page frame object bounding box undefined; frame includes
	   no displayable objects; exit */

#ifdef DEBUG
printf ( " Skipping page frame; no displayble objects found...\n" );
#endif

        return DVR_NORMAL;
    }

    /* Otherwise, frame bounding box included in specified display bounding
       box region */

#ifdef DEBUG
printf ( " Displaying page frame...\n" );
#endif

    /*
     *	Display any indicated outline paths, the only displayable portion
     *	of the frame object.
     */

    /* Check if object displayable is marked 'no display', and should be
       skipped, but not its children; it is assumed that this flag was set
       due to either the object being marked 'transparent', or an error,
       graphical in nature, has already been reported, and the fact that
       the displayable portion of this frame is bad, does not imply
       that its children are also in error */

    /* Check for object 'no display' flag */
    if ( ~object_private->flags & NO_DISPLAY_FLAG ) {

	/* Object 'no display' flag not set; OK to display drawable portion */
	if ( object_private->real_path != NULL ) {
	    /* Translated path available */

	    /* Call fill routine first, then stroke */

	    if ( frame->frm_obj.obj_flags & ddif_m_frame_background_fill ) {

		/* Frame background fill specified; fill area bounded by path */
		status = dvr_fill_path (
		    object_private->real_path,
		    &frame->frm_attributes,
		    vw,
		    0L,	/* No flags */
		    (STR) object_private->private_info );   /* Object private */

		if ( DVR_FAILURE ( status )) {
		    /* Graphics routine reports errors; associated object will
		       not be properly displayable; mark as 'no display' to
		       suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

	    }


	    if ( frame->frm_obj.obj_flags & ddif_m_frame_border ) {

		/* Frame border specified; draw border */

		status = dvr_stroke_path (
		    object_private->real_path,
		    &frame->frm_attributes,
		    vw,
		    0L,	/* No flags */
		    (STR) object_private->private_info,   /* Object private */
		    0L,  /* No draw pattern size */
		    0L); /* No draw pattern */

		if ( DVR_FAILURE ( status )) {
		    /* Graphics routine reports errors; associated object will
		       not be properly displayable; mark as 'no display' to
		       suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}
	    }

	/* Else - no path, no border or background fill possible or necessary */
	}

    /*
    **	Determine clip rectangle. frame->frm_obj.obj_path (transformed into
    **	object_private->clip_path) is the clipping path.  If there is no
    **	clipping path (or it's not rectangular), we clip to the page frame
    **	bounding box, but only because this is a page frame.
    */
    if (!rectangular_path(object_private->clip_path, &clip_bbox))
	{
	clip_bbox.bbox_ul_x = object_private->bbox_ul_x;
	clip_bbox.bbox_lr_y = object_private->bbox_lr_y;
	clip_bbox.bbox_lr_x = object_private->bbox_lr_x;
	clip_bbox.bbox_ul_y = object_private->bbox_ul_y;
	}
    display_bbox = &(vw->dvr_viewer.Dvr.display_bbox);
    clip_display_intersect(&clip_bbox, display_bbox);
    dvr_set_clip(&clip_bbox, vw);

    }	/* End if object has valid displayable portion */


    /*
     *	Display all objects in page frame.
     */

    status = dvr_display_objects (
	vw,
	(OBJ) frame,
	(BBOX_REF) &clip_bbox );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_objects
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays all objects in specified object queue.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	root_object	- Current root object structure
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *	DVR_UNKOBJTYPE Unknown object type
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_objects (
    vw,
    root_object,
    clip_bbox )

DvrViewerWidget vw;		/* Viewer widget context pointer */
OBJ root_object;		/* Current root object structure */
BBOX_REF    clip_bbox;		/* clip region from parent */

{
    /* Local variables */
    DvrStruct *dvr_struct;		/* DVR struct pointer */
    OBJ_PRIVATE object_private;		/* Object private data */
    BBOX_REF display_bbox,		/* Display bbox pointer */
	     current_clip_bbox;
    OBJ object;				/* Object pointer */
    PTH clip_path;			/* frame clipping path */
    CDAmeasurement x_window_top;	/* X window position top */
    CDAmeasurement y_window_top;	/* Y window position top */
    CDAstatus status;
    BBOX_UL_LR	local_clip_bbox;	/* current clip region */


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;


    /*
     *	Display all objects in queue, calling this routine recursively for
     *	any child objects found.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */


#ifdef DEBUG
printf ( " Displaying object queue...\n" );
#endif


    /* Get screen top position for proper offset into page (local defs) */
    x_window_top = dvr_struct->Xtop;
    y_window_top = dvr_struct->Ytop;


    /*
     *	Algorithm: Traverse, recursively through page structures (except for
     *  page objects and page frames, which are handled specially), checking
     *	the object private data bounding box information against the requested
     *	bounding box display area, traversing down a branch of the data tree
     *  structure only if the object bounding box intersects the requested
     *	display bounding box.  (The assumption is that parent object bounding
     *	boxes include the display area required for ALL their children.)
     */

    FOREACH_INLIST ( root_object->obj_queue, object, OBJ ) {	/* Loop */

#ifdef DEBUG
printf ( " Object type found: ");
switch (object->obj_type) {
    case obj_frm:  printf("obj_frm\n");  break;
    case obj_trn:  printf("obj_trn\n");  break;
    case obj_gly:  printf("obj_gly\n");  break;
    case obj_txt:  printf("obj_txt\n");  break;
    case obj_frg:  printf("obj_frg\n");  break;
    case obj_tab:  printf("obj_tab\n");  break;
    case obj_lin:  printf("obj_lin\n");  break;
    case obj_crv:  printf("obj_crv\n");  break;
    case obj_arc:  printf("obj_arc\n");  break;
    case obj_fil:  printf("obj_fil\n");  break;
    case obj_img:  printf("obj_img\n");  break;
/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    case obj_aud:  printf("obj_aud\n");  break;
#endif
/*END AUDIO STUFF*/
    default:  printf("UNKNOWN [Value is %08X (Hex)]\n", object->obj_type );
}
#endif

        /* Check if object displayable is marked 'bad', and should be skipped,
	   but not its children; it is assumed that this error, graphical
	   in nature, has already been reported, and that the fact that
           the displayable portion of this frame is bad, does not imply
           that its children are also in error */

	object_private = (OBJ_PRIVATE) ((OBJ) object)->obj_user_private;

	if (object_private == 0)
	    /*  something is pretty screwed up, do not attempt to go
	     *  any further, return
	     */
	    return(DVR_INTERNALERROR);

	current_clip_bbox = clip_bbox;

	/* Check for object 'no display' flag */
	if ( ~object_private->flags & NO_DISPLAY_FLAG ) {

	    /* Object 'no display' flag not set; OK to display drawable
	       portion */

	    /*
	     *	If the object bbox not included in specified clip bbox area,
	     *	skip to next element in loop, ignoring all of this object's
	     *	children (assumes that parent object's bounding	boxes include
	     *	the area required to image their child objects).  If the
	     *	object bbox is included in the display
	     *	region, display object and its children.
	     */

	    display_bbox = &( dvr_struct->display_bbox );

	    if ( !OBJECT_BBOX_INCLUDED (object_private, clip_bbox) ) {

		/* Skip display of object and its children */

#ifdef DEBUG
printf ( " Skipping object; object not included in display bbox...\n" );
#endif
		continue;	/* At end of 'For Each' while loop */
	    }


	    /* Otherwise, either display bounding box check flag is off, or
	       flag is on and object bounding box included in specified
	       display bounding box region */
#ifdef DEBUG
printf ( " Displaying object...\n" );
#endif


	    /*
	     *  Branch on object type displaying specific objects.
	     */

	    switch ( object->obj_type ) {

	    /* Note: 'Page' is not a valid object structure */

	    case obj_frm :

		/* Frame object */

    		status = dvr_display_frame_object (
		    vw,
		    (FRM) object,
		    x_window_top,
		    y_window_top );

		if ( DVR_FAILURE ( status ))
		    /* Frame will not be properly displayable, if so marked;
		       mark as 'no display' to suppress further attempts to
		       display */
		    object_private->flags |= NO_DISPLAY_FLAG;

		/*
		**	Now set clip rectangle for frame, but only if the frame
		**	has a clipping path.
		*/
		if ((clip_path = object_private->clip_path) != NULL)
		    {
		    if (!rectangular_path(clip_path,
					  &local_clip_bbox))
			{
			/*
			**  path is not rectangular, fall back to bbox for
			**  clipping region.
			*/
			local_clip_bbox.bbox_ul_x = object_private->bbox_ul_x;
			local_clip_bbox.bbox_lr_y = object_private->bbox_lr_y;
			local_clip_bbox.bbox_lr_x = object_private->bbox_lr_x;
			local_clip_bbox.bbox_ul_y = object_private->bbox_ul_y;
			}

		    clip_display_intersect(&local_clip_bbox, clip_bbox);
		    dvr_set_clip(&local_clip_bbox, vw);
		    current_clip_bbox = &local_clip_bbox;
		    }

		break;


	    case obj_trn :

		/* Translation object */

		/* Object cannot be displayed */

		break;


	    case obj_gly :

		/* Galley object */

		/*
		* Check to see if this galley object should be imaged or not.
		*/

                if ((object->obj_flags & ddif_m_gly_nonimaged_proto) != 0)
                    break;

		status = dvr_display_galley_object (
		    vw,
		    (GLY) object,
		    x_window_top,
		    y_window_top );

		if ( DVR_FAILURE ( status )) {
		    /* Galley will not be properly displayable, if so marked;
		       mark as 'no display' to suppress further attempts to
		       display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

		break;


	    case obj_txt :

		/* Text line object */

		/* check for changebars */


		status = dvr_display_text_object (
		    vw,
		    (TXT) object,
		    x_window_top,
		    y_window_top );

		if ( DVR_FAILURE ( status )) {
		    /* text will not be properly displayable; mark as
		       'no display' to suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

		break;

	    case obj_frg :

		/* Text fragment object */

		status = dvr_display_fragment_object (
		    vw,
		    (FRG) object,
		    x_window_top,
		    y_window_top );

		if ( DVR_FAILURE ( status )) {
		    /* Fragment will not be properly displayable; mark as
		       'no display' to suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

		break;


	    case obj_tab :

		/* Tab object */

		/* Object cannot be displayed */

		break;


	    case obj_lin :

		/* Polyline object */

		status = dvr_display_polyline_object (
		    vw,
		    (LIN) object);

		if ( DVR_FAILURE ( status )) {
		    /* Polyline will not be properly displayable; mark as
		       'no display' to suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

		break;


	    case obj_crv :

		/* Bezier curve object */

		status = dvr_display_curve_object (
		    vw,
		    (CRV) object);

		if ( DVR_FAILURE ( status )) {
		    /* Curve will not be properly displayable; mark as
		       'no display' to suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

		break;


	    case obj_arc :

		/* Arc object */

		status = dvr_display_arc_object (
		    vw,
		    (ARC) object);

		if ( DVR_FAILURE ( status )) {
		    /* Arc will not be properly displayable; mark as
		       'no display' to suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

		break;


	    case obj_fil :

		/* Fill object */

		status = dvr_display_fill_object (
		    vw,
		    (LIN) object);

		if ( DVR_FAILURE ( status )) {
		    /* Fill will not be properly displayable; mark as
		       'no display' to suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;
		}

		break;


	    case obj_img :

		/* Image object */

		status = dvr_display_image_object (
		    vw,
		    (IMG) object,
		    x_window_top,
		    y_window_top );

		if ( status != DVR_NORMAL ) {
		    /* Image will not be properly displayable; mark as
		       'no display' to suppress further attempts at display */
		    object_private->flags |= NO_DISPLAY_FLAG;

		    /* Report that we' can't display the image */
		    (void) dvr_error_callback(vw, 0L, status, 0, 0L);
		}

		break;



/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
	    case obj_aud :
		/* Audio object */
                status = dvr_display_audio_object(vw,
						  (AUD)object,
						  x_window_top,
						  y_window_top);

		if ( status != DVR_NORMAL ) {
		    /* Audio button will not be properly displayable; mark as
		     * 'no display' to suppress further attempts at displaying
		     * it.
		     */
		    object_private->flags |= NO_DISPLAY_FLAG;

		    /* Report that we' can't display it. */
		    (void) dvr_error_callback(vw, 0L, status, 0, 0L);
		}

		break;
#endif
/*END AUDIO STUFF*/

	    default :

		/* Unknown object type; do nothing, already reported in
	           format phase */

		;	    /* Continue */

	    }  /* End Switch */

	}   /* End if object has valid displayable portion */


	/*
	 *  Display child objects of current object, if any.
	 * Don't display any children of objects that are marked as
	 * non-imageable.
	 */

	/*
	 * Only process the children of objects that do not have the
	 * non-imaged proto flag turned on.
	 *
	 * The non-imaged proto flag can only be used for objects of
	 * type galley and text.
	*/

	if (
	    ((object->obj_type != obj_gly) && (object->obj_type != obj_txt)) ||
	     ((object->obj_flags & ddif_m_gly_nonimaged_proto) == 0)
	   )
       {
	  if ( !(QUE_EMPTY (object->obj_queue)) ) {

#ifdef DEBUG
printf ( "\n Child object queue found; recurse...\n" );
#endif

	    /* Display child object tree */

	    status = dvr_display_objects (
		vw,
		object,
		current_clip_bbox );

	    /*
	    **  Reset to parent's clip rectangle
	    */
	    dvr_set_clip(clip_bbox, vw);

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
		return status;
	    }
	  }
	  else
	    {
	     /*
	      * If we're processing a frame object we need to
	      * check if we should reset the clip.  The reason
	      * being that if the frame is empty we would have
	      * bypassed the reset to parents clip rectangle
	      * above.
	     */


	     if (object->obj_type == obj_frm)
		{
		 /*
		  * Reset to parent's clip rectangle
		 */

	         dvr_set_clip(clip_bbox, vw);
		}
	    }

#ifdef DEBUG
printf ( " End of child object queue\n\n" );
#endif

      }	/* End If not processing nonimaged_proto */

    }  /* End For-Each While Loop */

#ifdef DEBUG
printf ( " End displaying object queue elements\n" );
#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_frame_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays frame object graphics, except for page frames, if specified.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	frame		- Frame structure
 *	x_window_top	- X window position top, for positioning offset
 *	y_window_top	- Y window position top, for positioning offset
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_frame_object (
    vw,
    frame,
    x_window_top,
    y_window_top )

DvrViewerWidget vw;		/* Viewer widget context pointer */
FRM frame;			/* Frame structure */
CDAmeasurement x_window_top;	/* X window position top */
CDAmeasurement y_window_top;	/* Y window position top */

{

    /* Local variables */
    OBJ_PRIVATE object_private;	/* Object private data */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Display any indicated outline paths, the only displayable portion
     *	of the frame object.
     */

    object_private = (OBJ_PRIVATE) frame->frm_obj.obj_user_private;

    if ( object_private->real_path != NULL ) {
	/* Translated path available */

	/* Call fill routine first, then stroke */

	if ( frame->frm_obj.obj_flags & ddif_m_frame_background_fill ) {

	    /* Frame background fill specified; fill area bounded by path */
	    status = dvr_fill_path (
		object_private->real_path,
		&frame->frm_attributes,
		vw,
		0L,	/* No flags */
		object_private->private_info );   /* Object private */

	    if ( DVR_FAILURE ( status )) {
		/* Graphics routine reports errors; associated object will not
	           be properly displayable; mark as 'no display' to suppress
		   further attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }
	}

	if ( frame->frm_obj.obj_flags & ddif_m_frame_border ) {

	    /* Frame border specified; draw border */

	    status = dvr_stroke_path (
		object_private->real_path,
		&frame->frm_attributes,
		vw,
		0L,	/* No flags */
		object_private->private_info,   /* Object private */
		0L,  /* No draw pattern size */
		0L); /* No draw pattern */

	    if ( DVR_FAILURE ( status )) {
		/* Graphics routine reports errors; associated object will not
	           be properly displayable; mark as 'no display' to suppress
		   further attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }
	}

    /* Else - no path, no border or background fill possible or necessary */
    }

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_galley_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays galley object graphics, if specified.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	galley		- Galley structure
 *	x_window_top	- X window position top, for positioning offset
 *	y_window_top	- Y window position top, for positioning offset
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_galley_object (
    vw,
    galley,
    x_window_top,
    y_window_top )

DvrViewerWidget vw;		/* Viewer widget context pointer */
GLY galley;			/* Galley structure */
CDAmeasurement x_window_top;	/* X window position top */
CDAmeasurement y_window_top;	/* Y window position top */

{

    /* Local variables */
    OBJ_PRIVATE object_private;	/* Object private data */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;


    /*
     *	Display any indicated outline paths, the only displayable portion
     *	of the galley object.
     */

    object_private = (OBJ_PRIVATE) galley->gly_obj.obj_user_private;

    if ( object_private->real_path != NULL ) {
	/* Translated path available */

	/* Call fill routine first, then stroke */

	if ( galley->gly_obj.obj_flags & ddif_m_gly_background_fill ) {

	    /* Galley background fill specified; fill area bounded by path */
	    status = dvr_fill_path (
		object_private->real_path,
		&galley->gly_attributes,
		vw,
		0L,	/* No flags */
		(STR) object_private->private_info );   /* Object private */

	    if ( DVR_FAILURE ( status )) {
		/* Graphics routine reports errors; associated object will not
	           be properly displayable; mark as 'no display' to suppress
		   further attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }
	}

	if ( galley->gly_obj.obj_flags & ddif_m_gly_border ) {

	    /* Galley border specified; draw border */
	    status = dvr_stroke_path (
		object_private->real_path,
		&galley->gly_attributes,
		vw,
		0L,	/* No flags */
		(STR) object_private->private_info,   /* Object private */
		0L,  /* No draw pattern size */
		0L); /* No draw pattern */

	    if ( DVR_FAILURE ( status )) {
		/* Graphics routine reports errors; associated object will not
	           be properly displayable; mark as 'no display' to suppress
		   further attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }
	}

    /* Else - no path, no border or background fill possible or necessary */
    }

    /* Successful completion */
    return DVR_NORMAL;
}




/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_text_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays text object; for now, that just means displaying a
 *	changebar if flag is set for this TXT
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	txt_obj		- Text object structure
 *	x_window_top	- X window position top, for positioning offset
 *	y_window_top	- Y window position top, for positioning offset
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_text_object (
    vw,
    txt_obj,
    x_window_top,
    y_window_top )

DvrViewerWidget vw;		/* Viewer widget context pointer */
TXT  txt_obj;			/* Text fragment structure */
CDAmeasurement x_window_top;	/* X window position top */
CDAmeasurement y_window_top;	/* Y window position top */

{
    if (txt_obj->txt_do_changebar)
      {

	TXT_PRIVATE text_private = (TXT_PRIVATE) (((OBJ_PRIVATE)txt_obj->txt_obj.obj_user_private)->private_info);
	CDAstatus   status;
	boolean	    visible;

	/* set foreground to solid black (ala decwrite) */

#ifdef CDA_DECWINDOWS
	int save_fore = vw->dvr_viewer.Dvr.gcv.foreground;

	vw->dvr_viewer.Dvr.gcv.foreground = vw->manager.foreground;
	XSetForeground (XtDisplay(vw),
			vw->dvr_viewer.Dvr.GcId,
			vw->dvr_viewer.Dvr.gcv.foreground );

#else
	struct pat  black_path;

	black_path.pat_type = pat_solid;
	black_path.pat__specific.pat__solid.pat_red   =
	black_path.pat__specific.pat__solid.pat_green =
	black_path.pat__specific.pat__solid.pat_blue  = 0;

        status = dvr_output_pattern ( vw, &black_path, &visible );

        if (( !visible ) || ( DVR_FAILURE ( status ))) {
	    /* somethings amiss */
	    return DVR_NORMAL;
        }
#endif

        /*  draw changebar at the edge of
	 *  the page frame
	 */

	(void) dvr_draw_line (vw,
	  text_private->cb_x1 - x_window_top,
	  text_private->cb_y1 - y_window_top,
	  text_private->cb_x2 - x_window_top,
	  text_private->cb_y2 - y_window_top,
	  text_private->cb_wid);

#ifdef CDA_DECWINDOWS
	/* reset foreground to original value */
	vw->dvr_viewer.Dvr.gcv.foreground = save_fore;
	XSetForeground (XtDisplay(vw),
			vw->dvr_viewer.Dvr.GcId,
			vw->dvr_viewer.Dvr.gcv.foreground );
#endif

	/* need to reset for os/2, others... */

      }

    return (DVR_NORMAL);

}



/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_fragment_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays text fragment objects.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	fragment	- Text fragment structure
 *	x_window_top	- X window position top, for positioning offset
 *	y_window_top	- Y window position top, for positioning offset
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_display_fragment_object (
    vw,
    fragment,
    x_window_top,
    y_window_top )

DvrViewerWidget vw;		/* Viewer widget context pointer */
FRG fragment;			/* Text fragment structure */
CDAmeasurement x_window_top;	/* X window position top */
CDAmeasurement y_window_top;	/* Y window position top */

{

    /* Local variables */
    DvrViewerPtr dvr_window;	    /* Viewer widget */
    DvrStruct *dvr_struct;	    /* DVR struct pointer */
    OBJ_PRIVATE object_private;	    /* Object private data */
    FRG_PRIVATE fragment_private;   /* Fragment private */
    FRG_PRIVATE last_frag_private;  /* Last fragment private */
    PAT pat;			    /* Text pattern pointer */
    FIN finc;			    /* Font incarnation structure */
    unsigned long *font_struct;	    /* X font structure */
    int font_index;		    /* Font index */
    CDAmeasurement x_pos, y_pos;	/* X,Y positions */
    CDAmeasurement baseline_x1, baseline_x2;	/* Baseline, X start, end coordinates */
    CDAmeasurement baseline_y, y;		    /* Baseline, Y position; working Y pos */
    CDAmeasurement y_offset;		    /* Y offset rendition centipoint pos */
    CDAmeasurement prev_y;			    /* Previous Y position */
    CDAmeasurement width;			    /* String pixel width; line width */
    CDAmeasurement total_pixel_len;	    /* Total X pixel length */
    boolean visible;		    /* Flag true if object is 'visible' */
    CDAstatus status;


    /* Check parameter validity */
    if (( vw == NULL ) || ( fragment == NULL )) return DVR_BADPARAM;


    /*
     *	Display text fragment.
     */

    dvr_window = (DvrViewerPtr) &( vw->dvr_viewer );	/* Init */
    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );
    object_private = (OBJ_PRIVATE) fragment->frg_obj.obj_user_private;

    /* Get specified font and determine X font ID;
       use default font if specified font not loadable or usable */

    font_index = (int) fragment->frg_finc->fin_xfont_index;

#if defined(OS2)
    if (font_index == DEFAULT_FONT_INDEX)
	font_index = (int) dvr_window->default_font_id;
#endif

#ifdef MSWINDOWS
    if (dvr_window->current_font_index != font_index)
    {
    	if (font_index <= 0)
    	    (void)dvr_error_callback (vw, 0L, DVR_NOFONT, 0, 0L);
    	else
    	{
    	    dvr_window->current_font_index = font_index;
    	    dvr_set_font (vw, font_index);
    	}
    }
    font_struct = (unsigned long *) &DvrHps(vw);
    total_pixel_len = 0;

#else
    /* Font already loaded in format phase; check if font already set */
    if ( (dvr_window->num_fonts != 0) &&
	 ( dvr_window->current_font_index != font_index) ) {

	unsigned long font_id;

	if (font_index == DEFAULT_FONT_INDEX)
	    font_id = dvr_window->default_font_id;

	else
	    font_id = dvr_window->font_table[font_index].x_font_id;

	if (font_id <= 0)
	    /* unexpected error; continue processing with already loaded font */
	    (void) dvr_error_callback(vw, 0L, DVR_NOFONT, 0, 0L);
	else
          {
#if defined(OS2)
            dvr_window->current_font_index = font_index;
#endif
	    dvr_set_font(vw, font_id);
          }

	/* Save font ID as new current font ID */
	dvr_window->current_font_index = font_index;

	/* Else - font already set in GC */
	}

    total_pixel_len = 0;    /* Init */

#ifdef CDA_DECWINDOWS
    /* Get active font struct */

    if ( font_index == DEFAULT_FONT_INDEX ) {
	/* Default font in use */
	font_struct = dvr_window->default_font_struct;
    }
    else {
	/* Regular loaded font in use; get font struct from array */
	font_struct = dvr_window->font_table[font_index].x_font_struct;
    }
#endif

#endif /* End if #ifndef MSWINDOWS */


    /*
     *	Check for fragment private structures, created to handle stretch
     *	and rendition attributes.  Display appropriately.  First, set up
     *	necessary values and environment before displaying text.
     */

    /* Load specified pattern (color), if effect for entire fragment object */

    if (( pat = fragment->frg_text_msk_patt_ptr ) == NULL ) {
	/* No pattern specified */
	(void) dvr_error_callback ( vw, 0L, DVR_FORMATINFO, 0, 0L );
    }

#if defined(OS2)
    /* Visible flag returns TRUE if pattern should be visible (displayed) */
    status = dvr_output_text_pattern ( vw, pat, &visible );
#else
    /* Visible flag returns TRUE if pattern should be visible (displayed) */
    status = dvr_output_pattern ( vw, pat, &visible );
#endif

    if (( !visible ) || ( DVR_FAILURE ( status ))) {
	/* Object marked as invisible (transparent) or bad error status
	   returned; specific errors are reported via error callback in
	   output pattern routine, which has more information as to the
	   cause of the error; any special bad status versus transparent
	   processing may occur here; in either case, do not attempt to
	   redisplay this associated object in the future */

	object_private->flags |= NO_DISPLAY_FLAG;

        /*
	 * Call reset context routine for X-Windows 
	*/ 
#ifdef CDA_DECWINDOWS
	dvr_reset_gc (vw);
#endif

	return DVR_NORMAL;
    }


    /*
     *	Display text, checking for fragment private information.
     */

    if (( fragment_private = (FRG_PRIVATE) object_private->private_info )
	!= NULL ) {

	/* Special processing in effect; handle X stretch ; Note: Y stretch
	   is not supported */

	/* Determine initial screen location coordinates and draw string */
	x_pos = object_private->x_pos - x_window_top;
	y_pos = object_private->y_pos - y_window_top;

	while ( fragment_private != NULL ) {

	    /* Display substring segment */

	    /* Draw string to window */

	    dvr_draw_string(vw,
			    x_pos,
			    y_pos,
			    fragment_private->string,
			    (unsigned long) fragment_private->str_len );

	    /* No status code returned to check */

	    /* Need to get the pixel length of the string just drawn */

	    if ( fragment_private->pixel_len == 0 ) {

		/* Pixel length not yet calculated */
		width = dvr_text_width (
		    vw,
		    font_struct,
		    fragment_private->string,
		    (unsigned long) fragment_private->str_len );

		/* Store width */
		fragment_private->pixel_len = (int) width;

	    /* Else - Pixel length already calculated previously */
            }

	    /* Apply stretch in order to position next fragment substring */
	    x_pos += fragment_private->pixel_len + fragment_private->x_stretch;

	    /* Move to next fragment private structure; save current pointer
	       as previous, or last, fragment private reference before moving */
	    last_frag_private = fragment_private;
	    fragment_private = fragment_private->next;

	}   /* End While */

	/* Calculate string total pixel length; subtract final X stretch
	   value from the next X position value, and subtract this number
	   from the starting X position to determine the string length */
	total_pixel_len = x_pos - last_frag_private->x_stretch -
	    ( object_private->x_pos - x_window_top );
    }

    else {

	/* No substrings to process; process full string */

	/* Determine screen location coordinates and draw string */
	x_pos = object_private->x_pos - x_window_top;
	y_pos = object_private->y_pos - y_window_top;


	/* Draw string to window */
	dvr_draw_string(vw,
			x_pos,
			y_pos,
	    		(char *) fragment->frg_ptr,
	    		(unsigned long) fragment->frg_len );

	/* No status code returned to check */

	/* Defer calculation of string total pixel length unless rendition
	   attributes are to be applied */
    }


    /*
     *	Display selected text rendition attributes, such as underlining,
     *	if specified.
     */

    /* Skip rest of routine if no renditions specified */
    if ( !IF_RND_MASK
	(fragment->frg_renditions,RND_UNDERLINING_0,RND_UNDERLINING_1) ) {

	/* Rendition mask empty */

        /*
	 * Call reset context routine for X-Windows 
	*/ 
#ifdef CDA_DECWINDOWS
	dvr_reset_gc (vw);
#endif

	return DVR_NORMAL;
    }


    /* Check if total pixel length has yet been calculated; if not, calculate
       as a single string now (single fragment output, with X stretch, is
       deferred unless needed here) */

    if ( total_pixel_len == 0 ) {
	/* Pixel length not yet calculated */

	total_pixel_len = dvr_text_width (
	    vw,
	    font_struct,
	    (char *) fragment->frg_ptr,
	    fragment->frg_len );

    }


    /* Set up line end coordinates for lining operations; baseline with
       X,Y window offsets */

    baseline_y = object_private->y_pos - y_window_top;	/* Y1,Y2 same */

    baseline_x1 = object_private->x_pos - x_window_top;
    baseline_x2 = baseline_x1 + total_pixel_len;


    /* Init line width, font information reference */
    width = 1;	    /* Width now used as line width for rest of routine */
    finc = fragment->frg_finc;

    /* Note: All the font metric information available is described in
       centipoint units that are lower left oriented, rather than upper
       left oriented pixel value; therefore, all positional values are
       negated to 'switch' from a lower to upper box coordinate system;
       coordinate system is still left relative, although this position
       is not necessary since the line renditions all occur along the X
       path and constant Y positions.  Widths are not negated since these
       actually represent magnitude only values */


    if (IF_RND (fragment->frg_renditions,DDIF_K_RND_2_UNDERLINE)) {

	/* Double underline: upper line is 1/3 the underline width above the
	   underline position; lower line is 2/3 the underline width below
	   the underline position; width of each line is 2/3 the underline
	   width */

	/* Prepare upper line */
	y_offset = -finc->fin_font_metrics.fom_ulp -
	    ( finc->fin_font_metrics.fom_ulw / 3 );
	y = baseline_y + CONVERT_CP_TO_PIXEL_ROUNDED (y_offset,
						      vw->dvr_viewer.y_dpi);
	prev_y = y;

	/* Prepare lower line */
	y_offset = -finc->fin_font_metrics.fom_ulp +
	    (( finc->fin_font_metrics.fom_ulw * 2 ) / 3 );
	y = baseline_y + CONVERT_CP_TO_PIXEL_ROUNDED (y_offset,
						      vw->dvr_viewer.y_dpi);

	/* Make sure lines have at least one blank pixel between */
	if ( y == prev_y ) {
	    /* Double underline won't work on same line; image lower line
	       down an additional two lines */
            y += 2;	/* Move down two pixels */
	}

	if ( ( y - 1 ) == prev_y ) {
	    /* Double underline won't work on adjacent lines; image lower
	       line down an additional pixel */
	    y++;	/* Move down a pixel */
	}

	/* Draw upper and lower lines */
	width = CONVERT_CP_TO_PIXEL_ROUNDED
	    ((( finc->fin_font_metrics.fom_ulw * 2 ) / 3),
	     vw->dvr_viewer.x_dpi);  /* For both lines */

	(void) dvr_draw_line (vw, baseline_x1, prev_y, baseline_x2,
	    prev_y, width);
	(void) dvr_draw_line (vw, baseline_x1, y, baseline_x2, y, width);
    }


    if (IF_RND (fragment->frg_renditions,DDIF_K_RND_UNDERLINE)) {

	/* Single underline: line is at the underline position and width
	   is the underline width */

	y_offset = -finc->fin_font_metrics.fom_ulp;
	y = baseline_y + CONVERT_CP_TO_PIXEL_ROUNDED (y_offset,
						      vw->dvr_viewer.y_dpi);
	width = CONVERT_CP_TO_PIXEL_ROUNDED (finc->fin_font_metrics.fom_ulw,
					     vw->dvr_viewer.x_dpi);

	(void) dvr_draw_line (vw, baseline_x1, y, baseline_x2, y, width);
    }


    if (IF_RND (fragment->frg_renditions,DDIF_K_RND_OVERLINE)) {

	/* Single overline: overline is 2/3 the underline width above the
	   height of a capital letter; width is 2/3 the underline width */

	y_offset = -finc->fin_font_metrics.fom_capheight -
	    (( finc->fin_font_metrics.fom_ulw * 2 ) / 3 );
	y = baseline_y + CONVERT_CP_TO_PIXEL_ROUNDED (y_offset,
						      vw->dvr_viewer.y_dpi);

	width = CONVERT_CP_TO_PIXEL_ROUNDED
	    ((( finc->fin_font_metrics.fom_ulw * 2 ) / 3),
	     vw->dvr_viewer.x_dpi);

	(void) dvr_draw_line (vw, baseline_x1, y, baseline_x2, y, width);
    }


    if (IF_RND (fragment->frg_renditions,DDIF_K_RND_CROSS_OUT)) {

	/* Cross-out line (redline): line is at 1/2 the height of a
	   lowercase 'x'; width is 2/3 the underline width; [move line
	   up an extra pixel to compensate for numerical error] */

	y_offset = -finc->fin_font_metrics.fom_xheight / 2;
	y = baseline_y +
	    CONVERT_CP_TO_PIXEL_ROUNDED (y_offset, vw->dvr_viewer.y_dpi) - 1;

	width = CONVERT_CP_TO_PIXEL_ROUNDED
	    ((( finc->fin_font_metrics.fom_ulw * 2 ) / 3),
	     vw->dvr_viewer.y_dpi);

	(void) dvr_draw_line (vw, baseline_x1, y, baseline_x2, y, width);
    }

/*
 * Call reset context routine for X-Windows 
 */ 
#ifdef CDA_DECWINDOWS
    dvr_reset_gc (vw);
#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
** ++
**  Function: rectangular_path
**
**  Functional Description:
**
**	Determine if the path is rectangular.  If it isn't, return false. If
**	it is, return true and return the extent in the clip_bbox parameter.
**	(by PBD)
**
**
**  Arguments:
**
**	path : pointer to pth structure
**
**	clip_bbox : pointer to struct containing extent of rectangle
**
**  Result:
**
**	TRUE : Path is rectangular, clip_bbox values reflect path values
**
**	FALSE : Path is not rectangular, clip_bbox values are undefined
**
**  Exceptions:
**	None
**--
*/

boolean rectangular_path(path, clip_bbox)
     PTH	path;
     BBOX_REF    clip_bbox;
{
  CDAmeasurement	left, bottom, right, top;
  CDAcount		num_points, point_counter;
  CDAmeasurement	*coord_ptr;
  CDAmeasurement	first_x, first_y, x, y;

  if (path == NULL)
    return(FALSE);

  /*
   **  If this is a compound path (queue is not empty), we can't deal with it.
   */
  if ((path->pth_str.str_queue.que_flink != (QUE)path) &&
      (path->pth_str.str_queue.que_flink != NULL))
    return (FALSE);

  if (path->pth_type != pth_lin)
    return (FALSE);

  num_points = path->pth__specific.pth__xy.pth_pairs;
  /*
   **  A rectangular path must have 4 or 5 points.
   */
  if ((num_points != 4) && (num_points != 5))
    return (FALSE);

  coord_ptr = &(path->pth__specific.pth__xy.pth_first_x);
  first_x = *coord_ptr;
  coord_ptr++;
  first_y = *coord_ptr;
  coord_ptr++;
  left = first_x;
  right = first_x;
  top = first_y;
  bottom = first_y;

  for (point_counter = 2;  point_counter <= num_points;  point_counter++)
    {
      x = *coord_ptr;
      coord_ptr++;
      y = *coord_ptr;
      coord_ptr++;
      if (x < left)
	left = x;
      if (x > right)
	right = x;
      if (y > bottom)
	bottom = y;
      if (y < top)
	top = y;
    }
  if (num_points == 5)
    {
      /*
       **  if there are 5 points, the first and last must coincide, or it's not
       **  a rectangle.
       */
      if ((x != first_x) || (y != first_y))
	return(FALSE);
    }
  else if ((x != first_x) && (y != first_y))
    /*
     **  if there are 4 points, either the x or y coordinates must be the
     **  same as the first, or it's not a rectangle.
     */
    return (FALSE);
  
  clip_bbox->bbox_ul_x = left;
  clip_bbox->bbox_ul_y = top;
  clip_bbox->bbox_lr_x = right;
  clip_bbox->bbox_lr_y = bottom;
  return (TRUE);
}




/*
**++
**  clip_display_intersect
**
**  Functional Description:
**
**	Clip the first bbox against the second, altering the values of the
**	first bbox as required.  If there is no intersection, the values of the
**	first bbox are all set to zero. Typically (but not necessarily) used to
**	intersect a frame clip bbox with the display (expose region) bbox.
**	(by PBD)
**
**  Arguments:
**	clip_bbox : the bbox to be clipped
**
**	display_bbox : the bbox that the clip_bbox is clipped against
**
**
**  Result:
**	TRUE : there is an intersection
**
**	FALSE : there is no intersection
**
**  Exceptions:
**	None
**--
*/
boolean clip_display_intersect(clip_bbox, display_bbox)
BBOX_REF	clip_bbox, display_bbox;
    {
#if defined(OS2) || defined(MSWINDOWS)
    if (!dvr_object_bbox_included_b(clip_bbox, display_bbox))
#else
    if (!OBJECT_BBOX_INCLUDED(clip_bbox, display_bbox))
#endif
	{
	clip_bbox->bbox_ul_x = 0;
	clip_bbox->bbox_ul_y = 0;
	clip_bbox->bbox_lr_x = 0;
	clip_bbox->bbox_lr_y = 0;
	return(FALSE);
	}

    if (clip_bbox->bbox_ul_x < display_bbox->bbox_ul_x)
	clip_bbox->bbox_ul_x = display_bbox->bbox_ul_x;

    if (clip_bbox->bbox_lr_x > display_bbox->bbox_lr_x)
	clip_bbox->bbox_lr_x = display_bbox->bbox_lr_x;

    if (clip_bbox->bbox_ul_y < display_bbox->bbox_ul_y)
	clip_bbox->bbox_ul_y = display_bbox->bbox_ul_y;

    if (clip_bbox->bbox_lr_y > display_bbox->bbox_lr_y)
	clip_bbox->bbox_lr_y = display_bbox->bbox_lr_y;

    return (TRUE);
    }
