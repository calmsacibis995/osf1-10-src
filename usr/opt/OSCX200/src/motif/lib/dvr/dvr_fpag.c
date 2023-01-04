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
#define Module DVR_FPAG
#define Ident  "V02-043"

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
** MODULE_NAME:
**	dvr_format_page.c (vms)
**	dvr_format_page.c (ultrix)
**	dvr_fpag.c	  (OS/2)
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**	Formats page structure.
**
** ENVIORNMENT:
**	vms, ultrix, os/2
**
**
** AUTHORS:
**	Marc A. Carignan,  28-Nov-1988
**
**
** MODIFIED BY:
**
**	V02-043		RJD000		Ronan Duke		30-Aug-1992
**		Include Xm/MAnagerP.h if linking against Motif V1.2
**
**	V02-042		DAM000		Dennis McEvoy		03-Sep-1992
**		account for text alignment on MSWindows; we need to 
**		set the y coordinate for all text fragment to be the upper
**		left because SetTextAlign() seems to have no effect on
**		HP PCL printers.
**	    
**	V02-041		DAM000		Dennis McEvoy		02-Sep-1992
**		account for printing offset on MSWindows; we need to call
**		dvr_adjust_print_coords() whereever we call dvr_change_y_orient
**		to account for the printing offset.
**	    
**	V02-040		ECR000		Elizabeth C. Rust	30-Jul-1992
**		Fix typo in calc_arc_endpoints (return(status) was inside an ifdef).
**
**	V02-039		DAM000		Dennis McEvoy		29-Jul-1992
**		fix negative extent arcs on mswindows 
**	    
**	V02-038		RKN000		Ram Kumar Nori		23-Jul-1992
**		Modified the Offsets & Realtives and Bounding Box coordinate
**		calculations.
**	    
**      V02-037         DAM000          Dennis McEvoy           21-Jul-1992
**              modify mswindows format fragment code to use printer
**              hdc if present
**
**      V02-036         RDH003          Don Haney               15-Jul-1992
**              Specify same-directory includes with "", not <>
**
**	V02-035		KLM001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**
**	V02-034		DAM000		Dennis McEvoy		03-Jun-1992
**		use engine user private fields as CDAuserparam
**
**	V02-033		RAM000		Ralph A. Mack		28-Apr-1992
**		Fix problem in MS-Windows-only code: New Windows 3.1 include
**		file requires that we remove #define NOUSER in order to have
**		definitions of GetDC and ReleaseDC included.
**
**	V02-032		ECR000		Elizabeth C. Rust	30-Mar-1992
**		Merge in audio code.
**
**	V02-031		SJM000		Stephen Munyan		 9-Jan-1992
**		Merge in changes from DEC Japan
**
**      V02-030	        RKN0001		Ram Kumar Nori		29-Oct-1991
**		Modified dvr_transform_path routine to handle arrow heads
**		rotated arcs and rounded/smoothed polylines etc.
**
**      V02-029	        RKN0001		Ram Kumar Nori		18-Oct-1991
**		Added path type pth_rotated to the case stateemnt in
**		dvr_transform_path rotuine
**
**      V02-028	        RKN0001		Ram Kumar Nori		17-Sep-1991
**		Changed The Condition To Call calc_arc_endpoints routine
**
**      V02-027	        DAM0001		Dennis McEvoy		05-Aug-1991
**		rename header files, remove dollar signs
**
**	V02-026		SJM0000		Stephen Munyan		10-Jul-1991
**		Fixed child processing logic such that it only skips processing
**		children if the non-image proto flag is set for object types
**		galley and text.
**
**	V02-025		RAM002		Ralph A. Mack		31-May-1991
**		Add support for MS-Windows fonts.
**
**      V02-024	        RKN0001		Ram Kumar Nori		30-May-1991
**		Compute The Bounding Box For Rotated Ellipses
**
**      V02-023	        DAM0001		Dennis McEvoy		14-May-1991
**		make sure real path is NULL before calling transform_path
**		because we now call transform_path from
**		dvr_format_polyline_object and dvr_format_arc_object
**		in the case pf objects with arrowheads so that we can
**		account for the arrow heads in the bounding box
**		NOTE: it would be cleaner to account for arrow heads
**		      directly in transform_path; we should consider doing
**		      this in the future; for now, it would involve
**		      changing too many data structures at this late date
**
**      V02-022	        DAM0001		Dennis McEvoy		06-May-1991
**		move dvr_int include to cleanup ultrix build
**
**	V02-021		RAM001		Ralph A. Mack		24-Apr-91
**		Add #ifdefs for MS-Windows
**
**	V02-020		DAM020		Dennis McEvoy		08-Apr-91
**		take out extra changebar spaceafter check (taken
**		care of in engine.)
**
**	V02-019		DAM019		Dennis McEvoy		03-Apr-91
**		cleanup typedefs
**
**	V02-018		SJM0000		Stephen Munyan		26-Mar-1991
**		Ignore obj_tab objects produced by the Layout Engine
**
**	V02-017		DAM017		Dennis McEvoy		21-Mar-91
**		add support for changebars in text objs
**
**	V02-016		MHB0000		Mark Bramhall		05-Mar-1991
**		Remove temporary flag definition.
**
**	V02-015		RTG001		Dick Gumbel		05-Mar-91
**		Cleanup #include's
**
**	V02-014		DAM012		Dennis McEvoy		21-Dec-90
**		fix os/2 compiler warnings
**
**	V02-013		DAM001		Dennis McEvoy		21-Dec-90
**		use default font if needed for os/2
**
**	V02-012		DAM001		Dennis McEvoy		21-Dec-90
**		remove old os/2 font adjust code
**
**	V02-011		SJM0000		Stephen Munyan		29-Nov-1990
**		Merge in PROTO fixes from XUI
**
**	V02-013		DAM013		Dennis McEvoy		28-Nov-90
**		fix protos
**
**	V02-010		SJM0000		Stephen Munyan		30-Aug-1990
**		Add hyphen/minus fix such that 2D is imaged as AD for
**		ISO Latin-1 fonts.  Note that the fragment widths have
**		already been calculated assuming that this conversion
**		will be done here in the Viewer (and in the PSBE).
**
**		The reason the change is not done in the Layout Engine
**		is that the conversion does not want to be done for
**		the TEXT Back End and the Graphics Hard Copy Back End.
**
**	V02-009		BFB0001		Barbara Bazemore	21-Aug-1990
**		merge in Kenji Yamada-san's scaled image support changes
**
**	V02-008		SJM0000		Stephen Munyan		27-Jun-1990
**		Conversion to Motif
**
**	V02-007		DAM0001		Dennis McEvoy		26-Jul-90
**		move text adjustment code into engine for os/2
**
**	V02-006		DAM0001		Dennis McEvoy		18-Jul-90
**		fix text adjustment for OS/2 EGA fonts
**
**	V02-005		DAM0001		Dennis McEvoy		16-Jul-90
**		use y dpi for non 1:1 pixel displays
**
**	V02-004		DAM0001		Dennis McEvoy		05-Mar-90
**		changes for OS/2 port
**
**	V02-003		PMJ0001		Patricia M. Justus	15-Jun-89
**		Implement new flag for imaging galleys.
**
**	V02-002		PBD0001		Peter B. Derr		13-Jun-89
**		Always calculate arc endpoints regardless of arc extent.
**
**	V02-001		MHB0001		Mark H. Bramhall	1-May-89
**		Put color info queue root into page.
**--
*/


/*
 *  Include files
 */
#include <cdatrans.h>

#ifdef __vms__
#include <dvrint.h>				/* DVR internal definitions */

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */

#include <Xm/Xm.h>				/* Motif definitions */

/* RD:
 * if linking against V1.2 of Motif then need to explicitly include 
 * Xm/ManagerP.h to get XmManagerPart definition
 */
#ifdef MOTIF_V12
#include <Xm/ManagerP.h>	
#endif

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
#define NOMETAFILE				/* to save memory on compile. */
#include <windows.h>				/* MS-Windows definitions. */
#undef ERROR
#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef __unix__

#include <Xm/Xm.h>				/* Motif definitions */
#include <Xm/ManagerP.h>			/* rd: 23/7: need XmManagerPart def  */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#include "dvrwdef.h"			/* Public Defns w/dvr_include */
#include "dvrwint.h"			/* Viewer Constants and structures  */
#include "dvrwptp.h"			/* dvr windowing prototypes */





/* local routine prototypes */

PROTO( CDAstatus dvr_format_page_object,
		(DvrViewerWidget,
		 PAG,
		 STATEREF) );

PROTO( CDAstatus dvr_format_page_frame,
		(DvrViewerWidget,
		 FRM,
		 STATEREF) );

PROTO( CDAstatus dvr_format_objects,
		(DvrViewerWidget,
		 OBJ,
		 STATEREF) );

PROTO( CDAstatus dvr_format_frame_object,
		(DvrViewerWidget,
		 FRM,
		 STATEREF) );

PROTO( CDAstatus dvr_format_transform_object,
		(DvrViewerWidget,
		 TRN,
		 STATEREF) );

PROTO( CDAstatus dvr_format_galley_object,
		(DvrViewerWidget,
		 GLY,
		 STATEREF) );

PROTO( CDAstatus dvr_format_text_object,
		(DvrViewerWidget,
		 TXT,
		 STATEREF) );

PROTO( CDAstatus dvr_format_fragment_object,
		(DvrViewerWidget,
		 FRG,
		 STATEREF) );

PROTO( CDAstatus dvr_adjust_state_bbox,
		(DvrViewerWidget,
		 STATEREF,
		 OBJ) );

PROTO( CDAstatus dvr_bbox_to_path,
		(DvrViewerWidget,
		 CDAmeasurement, CDAmeasurement, CDAmeasurement, CDAmeasurement,
		 PTH *) );

PROTO( CDAstatus dvr_transform_path,
		(DvrViewerWidget,
		 STATEREF,
		 longword,
		 PTH,
		 PTH *,
		 PTH,
		 PTH,
		 void *) );

PROTO( CDAstatus calc_arc_endpoints,
		(DvrViewerWidget,
		 PTH,
		 ARC_PRIVATE *) );

PROTO( CDAstatus dvr_record_alloc_path,
		(DvrViewerWidget,
		 PTH) );

PROTO( CDAmeasurement dvr_change_y_orient,
		(CDAmeasurement,
		 CDAmeasurement) );

#ifdef MSWINDOWS
PROTO( void dvr_adjust_print_coords,
		(DvrViewerWidget,
		 CDAmeasurement *,
		 CDAmeasurement *) );
#endif

#ifdef DEBUG_DUMP

PROTO(void dump_page,
		(DvrViewerWidget,
		 PAG,
		 unsigned char) );

PROTO(void dump_page_element,
		(DvrViewerWidget,
		 PAG,
		 unsigned char,
		 FILE *,
		 unsigned char) );

PROTO(void dump_elements,
		(DvrViewerWidget,
		 OBJ,
		 long,
		 FILE *,
		 unsigned char) );

PROTO(void print_element_info,
		(DvrViewerWidget,
		 OBJ,
		 long,
		 FILE *,
		 unsigned char) );

#endif




/*
**
**  MACRO DEFINITIONS
**
**/

#define DVR_ABS(n)	(((n) < 0) ? -(n) : (n))	/* absolute value */
#define PI      3.141592653589793	/* probably precise enough :-) */


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_format_page
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Format all data on current page.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
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



CDAstatus dvr_format_page (
    vw )

DvrViewerWidget vw;		/* Viewer widget context pointer */

{
    /* Local variables */
    DvrStruct *dvr_struct;		/* DVR struct pointer */
    STATE state_basis;			/* State basis data */
    PAG_PRIVATE page_private;		/* Page private structure */
    PAG page;				/* Page structure */
    CDAstatus status;

    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Format current page.
     */

#ifdef DEBUG
printf ( "\nFormatting page structure...\n" );
#endif


/*AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    /*
     * If audio is encountered we need to put the audio toggle button on 
     * the screen.  If there's no audio take the button off the screen.
     * Maybe we should always have the audio toggle button on the screen
     * to make the user interested in audio???
     */
    {  int audio,managed;
       if ((audio= DVS_AUDIO_IN_DOC(vw->dvr_viewer.Dvr.engine_context)) 
	   && !(managed=XtIsManaged(AudioTog(vw))))
           dvr_resize(vw);
       else if (managed && !audio)
	   dvr_resize(vw);
    }
#endif
/*END AUDIO STUFF*/

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    page = dvr_struct->current_page;

    /* Initialize state basis data */
    state_basis.parent_state = NULL;	/* No parent initially */
    state_basis.x_offset    = 0;
    state_basis.y_offset    = 0;
    state_basis.x_relative  = 0;
    state_basis.y_relative  = 0;
    state_basis.bbox_ll_x   = POSITIVE_UNDEFINED;
    state_basis.bbox_ll_y   = POSITIVE_UNDEFINED;
    state_basis.bbox_ur_x   = NEGATIVE_UNDEFINED;
    state_basis.bbox_ur_y   = NEGATIVE_UNDEFINED;
    state_basis.x_scale	    = (float) 1.0;
    state_basis.y_scale	    = (float) 1.0;


    /* Store current (primary) page height for use throughout the page
       formatting phase */

    dvr_struct->working_page_height = page->pag_height[0];

    /*
     *	Format page object, the root of the page tree.  Routine calls
     *	itself recursively for referenced prototype pages, and traverses
     *	the tree structure for each page frame hierarchy.  Routine will also
     *	update the state basis information to reflect the smallest rectangular
     *	bounding box necessary to display the content of all the page's child
     *	objects.
     */

    status = dvr_format_page_object (
	vw,
	page,
	&state_basis );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }


    /*
     *  Store page paper size height and width in primary page private
     *	structure, converting values from centipoints to pixels, using
     *	the server's current DPI (dots per inch) measurement.  Note:
     *  Prototype page private structures do not utilize height and width
     *	data fields; these only apply to primary page displays.
     */

    page_private = (PAG_PRIVATE) page->pag_user_private;

    page_private->height = CONVERT_CP_TO_PIXEL ( page->pag_height[0],
						 vw->dvr_viewer.y_dpi );
    page_private->width = CONVERT_CP_TO_PIXEL ( page->pag_width[0],
						vw->dvr_viewer.x_dpi );

#ifdef DEBUG_DUMP

/* Dump pixels if requested */
#ifdef DUMP_PIXELS
    /* Print out page object information in pixels (flag false) */
    (void) dump_page ( vw, page, 0 );
#endif

/* Dump centipoints if requested */
#ifdef DUMP_CENTIPOINTS
    /* Print out page object information in centipoints (flag true) */
    (void) dump_page ( vw, page, 1 );
#endif

/* Dump both if neither DUMP_PIXELS nor DUMP_CENITPOINTS defined,
   but DEBUG_DUMP has been selected */
#ifndef DUMP_PIXELS
#ifndef DUMP_CENTIPOINTS
    /* Print out page object information in pixels, then centipoints (changes
       height and width params, so must be invoked after pixels) */
    (void) dump_page ( vw, page, 0 );
    (void) dump_page ( vw, page, 1 );
#endif
#endif

#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_format_page_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats page objects.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw  		- Viewer widget context structure
 *	page		- Page structure
 *	state		- State data
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



CDAstatus dvr_format_page_object (
    vw,
    page,
    state )

DvrViewerWidget vw;    		/* Viewer widget context pointer */
PAG page;     			/* Page structure */
STATEREF state;			/* State data */

{
    /* Local variables */
    DvrStruct *dvr_struct;	/* DVR struct pointer */
    STATE new_state;		/* New state data */
    PAG_PRIVATE page_private;	/* Page private data */
    PAG_PRIVATE proto_page_private;  /* Prototype page private data */
    PAG proto_page;		/* Prototype page */
    FRM frame;			/* Frame */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Format prototype pages, if any.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    COPY_INIT_STATE_INFO (state,&new_state);

    /*
     *	Allocate page private data structure.
     */

    status = dvr_alloc_struct (
	vw,
        pag_private,
	(void **) &page_private );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /* Initialize special image queue and bounding box */
    QUE_INIT (page_private->img_list);
    page_private->bbox_ll_x = POSITIVE_UNDEFINED;
    page_private->bbox_ll_y = POSITIVE_UNDEFINED;
    page_private->bbox_ur_x = NEGATIVE_UNDEFINED;
    page_private->bbox_ur_y = NEGATIVE_UNDEFINED;
    QUE_INIT (page_private->color_info_list);

/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    /* Initialise the list of audio buttons associated with this page. */
    page_private->audio_on_page = (AUD_PRIVATE)NULL;
#endif
/*END AUDIO STUFF*/

    /* Store in page structure */
    page->pag_user_private = page_private;


    if (( proto_page = page->pag_proto_page ) != NULL ) {
	/* Prototype page reference found */

#ifdef DEBUG
printf ( "\nFormatting prototype page...\n" );
#endif

	/* Check if prototype page has already been formatted; if so,
	   simply use return bounding box information in further
	   page calculations; that is, compare the page frame bounding
	   box information, stored as lower left and upper right centipoint
	   coordinates (saved specifically for this processing step) in
	   the page private structure.  (Note: If this prototype page has
	   been formatted, then so have any other prototype pages that this
	   page references; this prototype page is therefore referenced
	   by more than one page.) */

	if (( proto_page_private = (PAG_PRIVATE) proto_page->pag_user_private )
            != NULL ) {
	    /* Page already formatted; use information stored here */

#ifdef DEBUG
printf ( "\nPrototype page already formatted; skipping page...\n" );
#endif

	    /* Update bounding box info with stored page bbox */
#if defined(OS2) || defined(MSWINDOWS)
	    dvr_calculate_max_bbox_p(state, proto_page_private);
#else
	    CALCULATE_MAX_BBOX (state,proto_page_private);
#endif
	}

	else {
	    /* Referenced prototype page not yet formatted; call routine
	       recursively to format referenced prototype page */

#ifdef DEBUG
printf ( "\nPrototype page not yet formatted; formatting now...\n" );
#endif

	    status = dvr_format_page_object (
		vw,
		proto_page,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
		return status;
	    }

#ifdef DEBUG
printf ( "End prototype page\n" );
#endif

	    /* Update bounding box info */
	    CALCULATE_MAX_BBOX (state,&new_state);
	}
    }


    /*
     *  Format page frames, either primary or prototype page frames.
     *	First, format prototype page frame, if it exists, followed by
     *	the primary page frame, if it exists.
     */

    /* Format prototype page frame first, if it exists */
    if (( frame = page->pag_proto_frame ) != NULL ) {

#ifdef DEBUG
printf ( "\nFormatting prototype page frame...\n" );
#endif

	/* Prototype page frame found; check if prototype page frame has
	   already been processed; process only if frame and its child
	   objects have not yet been formatted.  Note: Prototype page frames
	   may be reference from more than one primary or prototype page */

	if ( frame->frm_obj.obj_user_private == NULL ) {
	    /* Prototype page frame not yet processed; process now */

#ifdef DEBUG
printf ( "\nPrototype page frame not yet processed; processing now...\n" );
#endif

	    status = dvr_format_page_frame (
		vw,
		frame,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
		return status;
	    }

	    /* Check return page frame bounding box size information for
	       accurate tracking of actual bounding box sizes */

	    CALCULATE_MAX_BBOX (state,&new_state);

	}

	else {
	    /* Prototype page frame already processed; skip frame processing */

#ifdef DEBUG
printf ( "\nPrototype page frame already processed; skipping frame...\n" );
#endif
	    ;	/* No op */

        }

#ifdef DEBUG
printf ( "End prototype page frame\n" );
#endif
    }

    /* Format primary page frame second, if it exists */
    if (( frame = page->pag_page_frame ) != NULL) {

#ifdef DEBUG
printf ( "\nFormatting primary page frame...\n" );
#endif

	/* Primary page frame found */
	status = dvr_format_page_frame (
	    vw,
	    frame,
	    &new_state );

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
	    return status;
	}

	/* Check return page frame bounding box size information for
	   accurate tracking of actual bounding box sizes */

	CALCULATE_MAX_BBOX (state,&new_state);

#ifdef DEBUG
printf ( "End primary page frame\n" );
#endif
    }

    /*
     *	Largest page frame size will now be returned to calling routine,
     *	in order to properly accomodate the display of all non-clipped
     *	objects described on the page; store information in page private
     *	structure; information here is represented in centipoint frame
     *	coordinate values for potential future formatting (i.e., later
     *	referenced prototype page processing).
     */

    page_private->bbox_ll_x = state->bbox_ll_x;
    page_private->bbox_ll_y = state->bbox_ll_y;
    page_private->bbox_ur_x = state->bbox_ur_x;
    page_private->bbox_ur_y = state->bbox_ur_y;

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_format_page_frame
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats page frames.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	frame		- Frame structure
 *	state		- State data
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



CDAstatus dvr_format_page_frame (
    vw,
    frame,
    state )

DvrViewerWidget vw;		/* Viewer widget context pointer */
FRM frame;			/* Frame structure */
STATEREF state;			/* State data */

{
    /* Local variables */
    DvrStruct		*dvr_struct;	/* DVR struct pointer */
    STATE		new_state;		/* New state data */
    STATE		temp_new_state;	/* temp copy of state */
    OBJ_PRIVATE		object_private;	/* Object private data */
    PTH			path;	       	/* Path pointer */
    CDAmeasurement	x_value;	/* Working X value */
    CDAmeasurement	y_value;	/* Working Y value */
    CDAmeasurement	bbox_ul_x;	/* Working lower left and upper right */
    CDAmeasurement	bbox_ul_y;	/* X,Y bounding box coordinate values */
    CDAmeasurement	bbox_lr_x;
    CDAmeasurement	bbox_lr_y;
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Format page frames.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    COPY_INIT_STATE_INFO (state,&new_state);

    /* Compute the offsets (frame position) in terms of page coordinates
     * as the origin of the parent PLUS the position of the frame. 
     * NOTE: The frame position is always specified in terms of it's parent 
     * Coordinate system. Relative values specify the origin
     */ 
    new_state.x_offset = state->x_relative + frame->frm_position_x; 
    new_state.y_offset = state->y_relative + frame->frm_position_y; 

    /* Compute the raltives values, which actually give the orgin of the
     * frame in page coordinate system. Origin is computed as offset MINUS
     * frame bounding box lower left coordinates. NOTE: Frame bounding box  
     * coordinates are frame's coordinate system not parent's coordinate
     * system
     */
    new_state.x_relative = new_state.x_offset - frame->frm_bbox_ll_x;
    new_state.y_relative = new_state.x_offset - frame->frm_bbox_ll_y;

    /* Compute the frame bounding box in page coordinate system. Bounding box
     * lower left coordinates in page coordinates system is computed as frame 
     * origin (relative) PLUS frame bounding box lower left coordinates in 
     * frame coordinate system. Bounding box upper right coordinates in page 
     * coordinates system is computed as Bounding box lower left coordinates
     * PLUS frame's height/width 
    */
    new_state.bbox_ll_x = new_state.x_relative + frame->frm_bbox_ll_x;
    new_state.bbox_ll_y	= new_state.y_relative + frame->frm_bbox_ll_y;
    new_state.bbox_ur_x = frame->frm_bbox_ur_x - frame->frm_bbox_ll_x
					+ new_state.bbox_ll_x;
    new_state.bbox_ur_y = frame->frm_bbox_ur_y - frame->frm_bbox_ll_y
						+ new_state.bbox_ll_y;

    /*
     *	Check if path defined, checking first if the object flags for
     *	border or background fill are indicated, and then frame outline for
     *	relative path information.  Store information in object private data
     *	structure.
     */

    /* Allocate object private data structure */
    status = dvr_alloc_struct (
	vw,
	obj_private,
	(void **) &object_private );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /* Initialize bounding box */
    object_private->bbox_ul_x = POSITIVE_UNDEFINED;
    object_private->bbox_ul_y = POSITIVE_UNDEFINED;
    object_private->bbox_lr_x = NEGATIVE_UNDEFINED;
    object_private->bbox_lr_y = NEGATIVE_UNDEFINED;

    /* Store in frame object header */
    frame->frm_obj.obj_user_private = object_private;


    /* Check for graphic background or border fill flags */
    if ( frame->frm_obj.obj_flags &
	( ddif_m_frame_border | ddif_m_frame_background_fill )) {

	/* Check if path defined */
        if (( path = frame->frm_outline ) == NULL ) {

	    /* Path not defined; use bounding box associated with frame,
	       converting it to a path representation */
	    status = dvr_bbox_to_path (
		vw,
		frame->frm_bbox_ll_x,
		frame->frm_bbox_ll_y,
		frame->frm_bbox_ur_x,
		frame->frm_bbox_ur_y,
		&path );

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error; internal processing error in bounding
		   box to path conversion */
		(void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );

		/* Object will not be properly displayable; mark as 'no
		   display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }
	}


	/* Transform path relative to new state, whether path is actually
	   specified original path, or recently calculated bounding box path */

	status = dvr_transform_path (
	    vw,
	    &new_state,
	    0,
	    path,
	    &object_private->real_path,
	    NULL,
	    NULL,
	    &object_private->private_info );

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error; internal processing error in path transform */
            (void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );

	    /* Object will not be properly displayable; mark as 'no display'
	       to suppress attempts to display */
	    object_private->flags |= NO_DISPLAY_FLAG;
	}


	/* Finally, account for the bounding box line width, so that the
	   state bounding box properly takes the line width into account;
	   this updated bounding box should include the entire line width,
	   not just to the center of the line */

	status = dvr_adjust_state_bbox (
	    vw,
	    &new_state,
	    (OBJ) frame );

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error; internal processing error in path transform */
            (void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );
	}
    }
    /*
    **  If frame has a clipping path, transform it.
    */
    if (frame->frm_obj.obj_path != NULL)
	{
	status = dvr_transform_path(vw, &new_state,0,
				    frame->frm_obj.obj_path,
				    &object_private->clip_path,
				    NULL,
				    NULL,
				    (void *)NULL);
	if ( DVR_FAILURE ( status ))
	    {
	    /* Unexpected error; internal processing error in path transform */
            (void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );
	    /* Object will not be properly displayable; mark as 'no display'
	       to suppress attempts to display */
	    object_private->flags |= NO_DISPLAY_FLAG;
	    }
	}


    /*
     **	Format all objects in page frame.  Use a temporary copy of the state
     **	because we don't want to expand the bbox because we clip to the page
     **	frame.
     */
    temp_new_state.parent_state = new_state.parent_state;
    temp_new_state.x_offset = new_state.x_offset;
    temp_new_state.y_offset = new_state.y_offset;
    temp_new_state.x_relative = new_state.x_relative;
    temp_new_state.y_relative = new_state.y_relative;
    temp_new_state.bbox_ll_x = new_state.bbox_ll_x;
    temp_new_state.bbox_ll_y = new_state.bbox_ll_y;
    temp_new_state.bbox_ur_x = new_state.bbox_ur_x;
    temp_new_state.bbox_ur_y = new_state.bbox_ur_y;
    temp_new_state.x_scale   = new_state.x_scale;
    temp_new_state.y_scale   = new_state.y_scale;

    status = dvr_format_objects (
	vw,
	(OBJ) frame,
	&temp_new_state );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }


    /* Check return bounding box information for accurate tracking of
     * actual bounding box sizes 
     */

    CALCULATE_MAX_BBOX (state,&new_state);


    /*  Convert position information in page coordinate system from frame 
     *  oriented lower left centipoint values to window coordinate upper left 
     *  corner pixel values storing this information in the object private 
     *  data structure.
     */
    x_value = new_state.x_offset;
    y_value = dvr_change_y_orient(dvr_struct->working_page_height,
				  new_state.y_offset );

    object_private->x_pos = CONVERT_CP_TO_PIXEL (x_value, vw->dvr_viewer.x_dpi);
    object_private->y_pos = CONVERT_CP_TO_PIXEL (y_value, vw->dvr_viewer.y_dpi);

#ifdef MSWINDOWS
    dvr_adjust_print_coords(vw,
			    &object_private->x_pos,
			    &object_private->y_pos);
#endif

    /* Convert new bounding box information, if defined */

    if ( BBOX_DEFINED (&new_state) ) {

       /* Bounding box defined for this object */

       /* Convert the bounding box cooridnate in page coordinate system from 
        * frame oriented lower left centipoint values to window coordinate 
	* upper left corner pixel values storing this information in the object
        * private data structure.
        */
	bbox_ul_x = new_state.bbox_ll_x;

#ifdef OS2
	/*  OS/2 has a lower left 0,0
	 *  Note, on OS/2 bbox_lr_y is really the upper right y value, and
	 *  bbox_ul_y is really the lower left value; the names were left
	 *  unchanged to reduce the amount of code change necessary to port
	 *  the code to OS/2; the viewer already had the varibles named
	 *  *lr_y and *ul_y, so it was easier to use the same var names.
	 *  The engine pages have the same 0,0 location as PM, and the engine
	 *  stores the values as *ur_y and *ll_y, so it was easier to just
	 *  grab the values directly from the engine, rather than converting
	 *  them. This also makes the bbox_included() logic the same on all
	 *  platforms.
	 */
	bbox_lr_y =
#else
	bbox_ul_y =
#endif
		    dvr_change_y_orient(dvr_struct->working_page_height,
	    				new_state.bbox_ur_y );
	bbox_lr_x = new_state.bbox_ur_x;

#ifdef OS2
	bbox_ul_y =
#else
	bbox_lr_y =
#endif
		    dvr_change_y_orient(dvr_struct->working_page_height,
	    				new_state.bbox_ll_y );

	object_private->bbox_ul_x = CONVERT_CP_TO_PIXEL (bbox_ul_x,
							 vw->dvr_viewer.x_dpi);
	object_private->bbox_ul_y = CONVERT_CP_TO_PIXEL (bbox_ul_y,
							 vw->dvr_viewer.y_dpi);
	object_private->bbox_lr_x = CONVERT_CP_TO_PIXEL (bbox_lr_x,
							 vw->dvr_viewer.x_dpi);
	object_private->bbox_lr_y = CONVERT_CP_TO_PIXEL (bbox_lr_y,
							 vw->dvr_viewer.y_dpi);

#ifdef MSWINDOWS
    dvr_adjust_print_coords(vw,
			    &object_private->bbox_ul_x,
			    &object_private->bbox_ul_y);
    dvr_adjust_print_coords(vw, 
			    &object_private->bbox_lr_x,
			    &object_private->bbox_lr_y);
#endif



    /* Else - object private bounding box remains undefined; that is,
       specific object is not imaged */
    }


    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_format_objects
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats all objects in specified object queue.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	root_object	- Current root object structure
 *	state		- State data
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



CDAstatus dvr_format_objects (
    vw,
    root_object,
    state )

DvrViewerWidget	    vw;			/* Viewer widget context pointer */
OBJ		    root_object;	/* Current root object structure */
STATEREF	    state;		/* State data */

{
    /* Local variables */
    DvrStruct	    * dvr_struct;	/* DVR struct pointer */
    STATE	    new_state;		/* New state data */
    STATE	    temp_new_state;	/* temp copy of state */
    OBJ_PRIVATE	    object_private;	/* Object private data */
    OBJ		    object;			/* Object pointer */
    CDAmeasurement	    x_value;		/* Working X value */
    CDAmeasurement	    y_value;		/* Working Y value */
    CDAmeasurement	    bbox_ul_x;		/* Working lower left and upper right */
    CDAmeasurement	    bbox_ul_y;		/* X,Y bounding box coordinate values */
    CDAmeasurement	    bbox_lr_x;
    CDAmeasurement	    bbox_lr_y;
    CDAstatus	    status;
    PTH		    rota_round_path;	/* Rounded/Smoothed Or Rounded Path */
    longword	    flags;		/* Flags set on specific object */

    rota_round_path = NULL;

    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Format all elements in object queue.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */

#ifdef DEBUG
printf ( " Formatting object queue...\n" );
#endif


    /*
     *	Traverse through the object queue formatting each element based
     *	upon the object type, and perform any required formatting tasks
     *	specific to that particular object type.  Then, performs any
     *	remaining generic objects tasks before moving on to the next
     *	element in the list, including formatting child object queues
     *	by recursively calling this routine.
     */

    FOREACH_INLIST ( root_object->obj_queue, object, OBJ ) {	/* Loop */

	/*
	 *  Allocate object private data
	 */

	status = dvr_alloc_struct (
	    vw,
	    obj_private,
	    (void **) &object_private );

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
	    return status;
	}

	/* Initialize bounding box */
	object_private->bbox_ul_x = POSITIVE_UNDEFINED;
	object_private->bbox_ul_y = POSITIVE_UNDEFINED;
	object_private->bbox_lr_x = NEGATIVE_UNDEFINED;
	object_private->bbox_lr_y = NEGATIVE_UNDEFINED;

	/* Store in object header */
	object->obj_user_private = object_private;


	/*
	 *  Reinit current state info each time, verifying that each child
	 *  object formatted in this loop will base its new state on that
	 *  of its parent, rather than the state of a previous sibling;
	 *  also, initialize the new bounding box information, which will be
	 *  incorporated into the parent bounding box if it exceeds the
	 *  parent's bounds for each object in the queue.  Subsequent
	 *  iterations will continue to maximize this bounding box until all
	 *  the objects at one level have been formatted, and this composite
	 *  maximum bounding box size can then be returned to the parent
	 *  object for additional comparison and formatting.
	 */

	 COPY_INIT_STATE_INFO (state,&new_state);

#ifdef DEBUG
printf ( " Object type found: ");
switch ( object->obj_type ) {
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

	/*
	 *  Branch on object type and perform any required specific
	 *  object formatting tasks.
	 */

	/* Note: It is expected that the individual object formatting
	   routines will probably report, in detail, any errors encountered
	   in formatting, and return an error status in order to mark
	   the element as having a bad displayable, if object has
	   a displayable portion */


	flags = object->obj_flags;
	switch ( object->obj_type ) {

	/* Note: 'Page' is not a valid object structure */

	case obj_frm :

	    /* Frame object */
	    status = dvr_format_frame_object (
		vw,
		(FRM) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Frame will not be properly displayable, if so marked;
		   mark as 'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


        case obj_trn :

	    /* Transform object */

	    status = dvr_format_transform_object (
		vw,
		(TRN) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Transform is not a displayable object; report error */
		(void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );
	    }

	    break;


        case obj_gly :

	    /* Galley object */

	    /*
	    * Check to see if this galley object should be imaged or not.
	    */

	    if ((object->obj_flags & ddif_m_gly_nonimaged_proto) != 0)
		break;

	    status = dvr_format_galley_object (
		vw,
		(GLY) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Galley will not be properly displayable, if so marked;
		   mark as 'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


	case obj_txt :

	    /* Text line object */

	    /*
	    * Check to see if this text object should be imaged or not. The
	    * galley flags are copied over to the text object flags because
	    * the parent of a text object is a frame and not a galley. If the
	    * flag is set, the object is not to be imaged.
	    */

	    if ((object->obj_flags & ddif_m_gly_nonimaged_proto) != 0)
		break;



	    status = dvr_format_text_object (
		vw,
		(TXT) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Text is not a displayable object; report error */
		(void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );
	    }

	    break;


	case obj_frg :

	    /* Text fragment object */

	    status = dvr_format_fragment_object (
		vw,
		(FRG) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Fragment will not be properly displayable; mark as
		   'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


	case obj_tab :

	    /* Tab object */

	    /* Ignored by the Viewer */

	    break;


        case obj_lin :

	    /* Polyline object */

	    status = dvr_format_polyline_object (
		vw,
		(LIN) object,
    		& rota_round_path,
		& new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Polyline will not be properly displayable; mark as
		   'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


	case obj_crv :

	    /* Bezier curve object */

	    status = dvr_format_curve_object (
		vw,
		(CRV) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Curve will not be properly displayable; mark as
		   'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


	case obj_arc :

	    /* Arc object */

	    status = dvr_format_arc_object (
		vw,
		(ARC) object,
    		& rota_round_path,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Arc will not be properly displayable; mark as
		   'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


	case obj_fil :

	    /* Fill object */

	    status = dvr_format_fill_object (
		vw,
		(CRV) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Fill will not be properly displayable; mark as
		   'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


	case obj_img :

	    /* Image object */

	    status = dvr_format_image_object (
		vw,
		(IMG) object,
		&new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Image will not be properly displayable; mark as
		   'no display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    break;


/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
	case obj_aud :
	    /* Audio object */
	    status = dvr_format_audio_object (vw,
					      (AUD) object,
					      &new_state );
	    if ( DVR_FAILURE ( status ))
	    {	/* Audio button will not be properly displayable; mark as
		 *  'no display' to suppress attempts to display it.
		 */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }
	    break;
#endif
/*END AUDIO STUFF*/

	default :

	    /* Unknown object type; report error, and continue */
	    (void) dvr_error_callback ( vw, 0L, DVR_UNKOBJTYPE, 0, 0L );

	}  /* End Switch */


	/*
	 *  Format child objects of current object if they exist.
	 * If the object had the non-image flag set don't format the
	 * children of the object.
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

	    /*
	     *  Child object queue found; format objects, with
	     *	potentially updated state information.
	     */

#ifdef DEBUG
printf ( "\n Processing child objects: " );
#endif
	    /* Update maximum bounding boxes before processing children;
	       More detailed macro description can be found one page below */

	    CALCULATE_MAX_BBOX (state,&new_state);


	    /*
	    **  Call format objects routine to process children.
	    **	But, if this is a frame with a clip path, don't expand the
	    **	bounding box with its children -- use a temporary copy of the
	    **	state info.
	    */
	    if ((object->obj_type == obj_frm) && (object->obj_path != NULL))
		{
		temp_new_state.parent_state = new_state.parent_state;
		temp_new_state.x_offset = new_state.x_offset;
		temp_new_state.y_offset = new_state.y_offset;
		temp_new_state.x_relative = new_state.x_relative;
		temp_new_state.y_relative = new_state.y_relative;
		temp_new_state.bbox_ll_x = new_state.bbox_ll_x;
		temp_new_state.bbox_ll_y = new_state.bbox_ll_y;
		temp_new_state.bbox_ur_x = new_state.bbox_ur_x;
		temp_new_state.bbox_ur_y = new_state.bbox_ur_y;
		temp_new_state.x_scale = new_state.x_scale;
		temp_new_state.y_scale = new_state.y_scale;

		status = dvr_format_objects (
		    vw,
		    object,
		    &temp_new_state );
		}
	    else
		status = dvr_format_objects (
		    vw,
		    object,
		    &new_state );

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
		return status;
	    }

#ifdef DEBUG
printf ( " End of child object queue\n\n" );
#endif
	  }
	}

	/*
	 *  Transform paths necessary for current object.
	 */

	if ( ( object_private->orig_path ) &&
  	     ( !object_private->real_path ) ) {

	    /* Path found; transform into X pixel coordinates, and store
	       value in real path */

	    status = dvr_transform_path (
		vw,
		&new_state,
		object->obj_flags,
		object_private->orig_path,
		&object_private->real_path,
		object->obj_path,
    		rota_round_path,
		&object_private->private_info );

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error; internal processing error in path
		   transform */
		(void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );

		/* Object will not be properly displayable; mark as 'no
		   display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	    /*
	     *  Account for the bounding box line width, so that the
	     *  state bounding box properly takes the line width into account;
	     *  this updated bounding box should include the entire line width,
	     *  not just to the center of the line; this only applies to certain
	     *  object types, that is, those types that may require graphical
	     *  line drawing.
	     */

	    /* Routine only valid for the following object types, already
	       accounted for by checking for the object private original
	       path; if this path does not exist, then even if the object
	       is one of the following types, it is not 'imaged'; if it
	       does have an original path, then it is to be 'imaged', and
	       must be one of the following specified object types */

	    /* Check not necessary: */
	    /* if (( object->obj_type == obj_frm ) ||
		( object->obj_type == obj_gly ) ||
		( object->obj_type == obj_lin ) ||
		( object->obj_type == obj_crv ) ||
		( object->obj_type == obj_arc )) { */


	    (void) dvr_adjust_state_bbox (
		vw,
		&new_state,
		object );

	    /* Ignore errors as inconsequential */
	}

	/*
	**  If this is a frame, and it has a clipping path, transform it.
	*/
	if ( (object->obj_type == obj_frm) &&
	     (object->obj_path != NULL) )
	    {
	    status = dvr_transform_path (
		vw,
		&new_state,
		0,
		object->obj_path,
		&object_private->clip_path,
		NULL,
		NULL,
		(void *)NULL);
	    if ( DVR_FAILURE ( status ))
		{
		/* Unexpected error; internal processing error in path
		   transform */
		(void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );

		/* Object will not be properly displayable; mark as 'no
		   display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
		}
	    }


	/*
	 *  Update state bounding box information to reflect changes affected
	 *  at this level; that is, incorporate the child bounding box size
	 *  (at this level) to the parent object's bounding box size, new_state
	 *  and state, respectively.  This allows parent bounding box infor-
	 *  mation to properly reflect bigger bounding box sizes than stated
	 *  in the DDIF document.  (Note: Clipping boundaries are not directly
	 *  related to this bounding box correction, and if supported, must be
	 *  treated differently and separately.)
	 */

	/* Routine appears here for objects without children, after any
	   possible path transform operations.  If object has children,
	   they will already have had their bounding boxes processed by
	   this macro, but may benefit for this additional call which will
	   catch children 'larger' than the space currently calculated for
	   the parent object, which should include the space required to
	   display all its children */

	CALCULATE_MAX_BBOX (state,&new_state);


       /*  Convert position information in page coordinate system from frame 
        *  oriented lower left centipoint values to window coordinate upper 
        *  left corner pixel values storing this information in the object 
        *  private data structure.
        */
	x_value = new_state.x_offset;
	y_value = dvr_change_y_orient(dvr_struct->working_page_height,
				      new_state.y_offset );
	object_private->x_pos = CONVERT_CP_TO_PIXEL (x_value,
						     vw->dvr_viewer.x_dpi);
	object_private->y_pos = CONVERT_CP_TO_PIXEL (y_value,
						     vw->dvr_viewer.y_dpi);
#ifdef MSWINDOWS
    	dvr_adjust_print_coords(vw,
				&object_private->x_pos,
			        &object_private->y_pos);
#endif

	/* Convert new bounding box information, if defined */

	if ( BBOX_DEFINED (&new_state) ) {

	   /* Bounding box defined for this object */
           /* Convert the bounding box cooridnate in page coordinate system 
	    * from frame oriented lower left centipoint values to window 
	    * coordinate upper left corner pixel values storing this 
	    * information in the object private data structure.
            */
	    bbox_ul_x = new_state.bbox_ll_x ;

#ifdef OS2
	/*  OS/2 has a lower left 0,0
	 *  Note, on OS/2 bbox_lr_y is really the upper right y value, and
	 *  bbox_ul_y is really the lower left value; the names were left
	 *  unchanged to reduce the amount of code change necessary to port
	 *  the code to OS/2; the viewer already had the varibles named
	 *  *lr_y and *ul_y, so it was easier to use the same var names.
	 *  The engine pages have the same 0,0 location as PM, and the engine
	 *  stores the values as *ur_y and *ll_y, so it was easier to just
	 *  grab the values directly from the engine, rather than converting
	 *  them. This also makes the bbox_included() logic the same on all
	 *  platforms.
	 */
	    bbox_lr_y =
#else
	    bbox_ul_y =
#endif
			dvr_change_y_orient(dvr_struct->working_page_height,
					    new_state.bbox_ur_y );
	    bbox_lr_x = new_state.bbox_ur_x;
#ifdef OS2
	    bbox_ul_y =
#else
	    bbox_lr_y =
#endif
			dvr_change_y_orient(dvr_struct->working_page_height,
					    new_state.bbox_ll_y);

	    object_private->bbox_ul_x = CONVERT_CP_TO_PIXEL (bbox_ul_x,
						vw->dvr_viewer.x_dpi);
	    object_private->bbox_ul_y = CONVERT_CP_TO_PIXEL (bbox_ul_y,
						vw->dvr_viewer.y_dpi);
	    object_private->bbox_lr_x = CONVERT_CP_TO_PIXEL (bbox_lr_x,
						vw->dvr_viewer.x_dpi);
	    object_private->bbox_lr_y = CONVERT_CP_TO_PIXEL (bbox_lr_y,
						vw->dvr_viewer.y_dpi);

#ifdef MSWINDOWS
    	    dvr_adjust_print_coords(vw,
				    &object_private->bbox_ul_x,
			            &object_private->bbox_ul_y);
    	    dvr_adjust_print_coords(vw,
				    &object_private->bbox_lr_x,
			            &object_private->bbox_lr_y);
#endif
	/* Else - object private bounding box remains undefined; that is,
	   specific object is not imaged */
	}

    }  /* End For-Each While Loop */

#ifdef DEBUG
printf ( " End formatting object queue\n" );
#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_format_frame_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats frame objects, except for page frames.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	frame		- Frame structure
 *	state		- State data
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



CDAstatus dvr_format_frame_object (
    vw,
    frame,
    state )

DvrViewerWidget vw;		/* Viewer widget context pointer */
FRM frame;			/* Frame structure */
STATEREF state;			/* State data */

{
    /* Local variables */
    DvrStruct *dvr_struct;	/* DVR struct pointer */
    OBJ_PRIVATE object_private;	/* Object private pointer */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Format frames, except for page frames.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    object_private = (OBJ_PRIVATE) frame->frm_obj.obj_user_private;

#ifdef DEBUG
printf ( "\n Frame object formatting...\n" );
#endif

    /* Compute the offsets (postions) of the frame object in page coordinate 
     * system as the origin of it's parent PLUS the frame position scaled 
     * according to any transforms in effect
     */
    state->x_offset = state->x_relative + 
		    (CDAmeasurement) (frame->frm_position_x * state->x_scale);

    state->y_offset = state->y_relative + 
		    (CDAmeasurement) (frame->frm_position_y * state->y_scale);

    /* Compute the relatives (origin) of the frame object in page coordinate
     * system as the position of frame object MINUS frame position scaled 
     * according to any transforms in effect
     */
    state->x_relative = state->x_offset - 
                  (CDAmeasurement) ( frame->frm_bbox_ll_x * state->x_scale );
    state->y_relative = state->y_offset -
		  (CDAmeasurement) ( frame->frm_bbox_ll_y * state->y_scale );

    /* Convert the bounding box cooridnates in frame coordinate system to 
     * page coordinate system and scale it according to any transforms in 
     * effect
     */

    state->bbox_ll_x = state->x_relative +
		       (CDAmeasurement) (frame->frm_bbox_ll_x * state->x_scale);
    state->bbox_ll_y = state->x_relative +
		       (CDAmeasurement) (frame->frm_bbox_ll_y * state->y_scale);

    state->bbox_ur_x = state->bbox_ll_x + 
            (CDAmeasurement) ( (frame->frm_bbox_ur_x - frame->frm_bbox_ll_x)
				* state->x_scale );

    state->bbox_ur_y =  state->bbox_ll_y +
	    (CDAmeasurement) ( (frame->frm_bbox_ur_y - frame->frm_bbox_ll_y)
				* state->y_scale );


    /* Verify that object private data has been allocated */
    if ( frame->frm_obj.obj_user_private == NULL ) {
	/* Not allocated; error */
	return DVR_INTERNALERROR;
    }


    /*
     *	Format graphic path, if specified.
     */

    /* Check for graphic background or border fill flags */
    if ( frame->frm_obj.obj_flags &
	( ddif_m_frame_border | ddif_m_frame_background_fill )) {

	/* Check if path defined */
        if (( object_private->orig_path = frame->frm_outline ) == NULL ) {

	    /* Path not defined; use bounding box associated with frame,
	       converting it to a path representation */
	    status = dvr_bbox_to_path (
		vw,
		frame->frm_bbox_ll_x,
		frame->frm_bbox_ll_y,
		frame->frm_bbox_ur_x,
		frame->frm_bbox_ur_y,
		&object_private->orig_path );	/* Stored in orig_path
						   for further processing
						   in format objects routine */

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error; internal processing error in bounding
		   box to path conversion */
		(void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );

		/* Object will not be properly displayable; mark as 'no
		   display' to suppress attempts to display */
		object_private->flags |= NO_DISPLAY_FLAG;
	    }

	/* Else - Use frame outline path specified */
	}
    }

#ifdef DEBUG
printf ( " End frame object\n" );
#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_format_transform_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats transform objects.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	transform	- Transform structure
 *	state		- State data
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



CDAstatus dvr_format_transform_object (
    vw,
    transform,
    state )

DvrViewerWidget vw;		/* Viewer widget context pointer */
TRN transform;			/* Transform structure */
STATEREF state;			/* State data */

{
    /* Local variables */
    DvrStruct *dvr_struct;	/* DVR struct pointer */

    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Format transforms.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */

#ifdef DEBUG
printf ( "\n Transform object formatting...\n" );
#endif

    /* Modify state to correspond to new orientation, which directly applies
       to child object orientations; currently, only X,Y coordinate
       translations are applied; other transform properties to be added in
       the future, such as rotation, skew, etc. */


    /* Transform 3x3 matrix available from the layout engine is actually
       stored in row-major (versus standard
       column-major) order as a 9-element array;	0  1  2
       therefore, the elements 0-1-2-3-4-5-		3  4  5
       6-7-8 are represented by the matrix:		6  7  8	    */

#ifdef DEBUG
printf ( "\nTransformation matrix (row-major order):\n");
printf ( "    %8f %8f %8f\n", transform->trn_transform[0],
    transform->trn_transform[1], transform->trn_transform[2] );
printf ( "    %8f %8f %8f\n", transform->trn_transform[3],
    transform->trn_transform[4], transform->trn_transform[5] );
printf ( "    %8f %8f %8f\n", transform->trn_transform[6],
    transform->trn_transform[7], transform->trn_transform[8] );
#endif


    /* Transform state X,Y offset values, in current scale coordinates */

    /* X translation value */
    state->x_offset += (CDAmeasurement) (transform->trn_transform[6] * state->x_scale);

    /* Y translation value */
    state->y_offset += (CDAmeasurement) (transform->trn_transform[7] * state->y_scale);


    /* Transform objects are never imaged, but update the X,Y relative
       positions to better store this object's, and its children's, starting
       location */

    /* X translation value */
    state->x_relative += (CDAmeasurement) (transform->trn_transform[6] * state->x_scale);

    /* Y translation value */
    state->y_relative += (CDAmeasurement) (transform->trn_transform[7] * state->y_scale);

    /* Update the scale transform information in the current state. */

    state->x_scale *= transform->trn_transform[0];   /* X scale value */
    state->y_scale *= transform->trn_transform[4];   /* Y scale value */

    /* No bounding box specified; child bbox values will propagate back
       up to this element so that the bbox stored in this transform object
       private data will reflect the composite bbox of its child objects */


    /* Any additional calculations necessary to format a transform
       object should be performed here, or where appropriate within this
       routine */


#ifdef DEBUG
printf ( " End transform object\n" );
#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_format_galley_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats galley objects.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	galley		- Galley structure
 *	state		- State data
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



CDAstatus dvr_format_galley_object (
    vw,
    galley,
    state )

DvrViewerWidget vw;		/* Viewer widget context pointer */
GLY galley;			/* Galley structure */
STATEREF state;			/* State data */

{
    /* Local variables */
    DvrStruct *dvr_struct;	/* DVR struct pointer */
    OBJ_PRIVATE object_private;	/* Object private pointer */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Format galleys.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    object_private = (OBJ_PRIVATE) galley->gly_obj.obj_user_private;

#ifdef DEBUG
printf ( "\n Galley object formatting...\n" );
#endif

    /* Verify that object private data has been allocated */
    if ( galley->gly_obj.obj_user_private == NULL ) {
	/* Not allocated; error */
	return DVR_INTERNALERROR;
    }


    /*
     *	Format graphic path, if specified.
     */

    /* Check for graphic background or border fill flags */
    if ( galley->gly_obj.obj_flags &
	( ddif_m_gly_border | ddif_m_gly_background_fill )) {

	/* Galleys do not have specific paths defined; if graphic
	   flags on, then user bounding box associated with galley,
	   converting it to a path representation */

        status = dvr_bbox_to_path (
	    vw,
	    galley->gly_bbox_ll_x,
	    galley->gly_bbox_ll_y,
	    galley->gly_bbox_ur_x,
	    galley->gly_bbox_ur_y,
	    &object_private->orig_path );	/* Stored in orig_path
						   for further processing
						   in format objects routine */

        if ( DVR_FAILURE ( status )) {
	    /* Unexpected error; internal processing error in bounding
	       box to path conversion */
	    (void) dvr_error_callback ( vw, 0L, DVR_INTERNALERROR, 0, 0L );

	    /* Object will not be properly displayable; mark as 'no display'
	       to suppress attempts to display */
	    object_private->flags |= NO_DISPLAY_FLAG;
        }
    }

#ifdef DEBUG
printf ( " End galley object\n" );
#endif

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *  	dvr_format_text_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats text line object.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	text		- Image structure
 *	state		- State data
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



CDAstatus dvr_format_text_object (
    vw,
    text,
    state )

DvrViewerWidget vw;		/* Viewer widget context pointer */
TXT text;			/* Text line object */
STATEREF state;			/* State data */

{
    /* Local variables */
    CDAstatus status;


    /* Check parameter validity */
    if (( vw == NULL ) || ( text == NULL ) || ( state == NULL ))
	return DVR_BADPARAM;

    /*
     *	Format text line.
     */

    /* Modify state to reflect the increase current offset by the text start
     * position. The offset values are in page coordinate system
     */
    state->x_offset = state->x_relative + text->txt_start_x;
    state->y_offset = state->y_relative + text->txt_start_y;

    /* Text line objects are never imaged, but update the X,Y relative
     * positions which gives the origin, in case of text object the 
     * origin is same as the offset/starting position.
     */

    state->x_relative = state->x_offset;
    state->y_relative = state->y_offset;

    /* Use X,Y stored sizes for bounding box determination relative
       to current position (rather than the new position described by
       this object for its child objects); actual bounding box values for
       this 'non-imaging' object will be determined during TXT child
       object formatting, since accurate determination of the bounding
       box for all children at this point is not known */

    /* Note: Text assumed to begin at this position (left text alignment)
       and move to the right horizontally */

    state->bbox_ll_x = state->x_offset;

    /* Descender value is signed, and is probably negative */
    state->bbox_ll_y = state->y_offset + text->txt_descender;
                                                                                
    state->bbox_ur_x = state->bbox_ll_x + text->txt_length;

    /* Ascender value is signed, and is probably positive */
    state->bbox_ur_y = state->y_offset + text->txt_ascender; 
  
    /* Any additional calculations necessary to format a text line
       object should be performed here */

    /* fill in changebar info if present */
    if (text->txt_do_changebar)
      {
        TXT_PRIVATE 	text_private;
	signlong	space_after_value;

	status = dvr_alloc_struct (
		vw,
		txt_private,
		(void **) &text_private );

	if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
		return status;
	    }

        /*
	 *  check for direction of page, and place changebar x coordinates
	 *  accordingly
	 */
	if (vw->dvr_viewer.Dvr.current_page->pag_hand == DDIF_K_RIGHT_PAGE)
	  {
	    /*  right-handed page, changebar goes out from the rightmost
	     *  galley edge
	     */
	    text_private->cb_x1  =
		vw->dvr_viewer.Dvr.current_page->pag_rightmost_gly_edge+
				text->txt_changebar_offset;

	    /* expand bounding box right side x */
    	    state->bbox_ur_x = MAX(state->bbox_ur_x, text_private->cb_x1);

	    /* convert to pixels */
	    text_private->cb_x2  =
	    text_private->cb_x1  =
    	    	CONVERT_CP_TO_PIXEL_ROUNDED (text_private->cb_x1,
					     vw->dvr_viewer.x_dpi);

	  }

	else
	  {
	    /*  left hand page, changebar goes in from the leftmost
	     *  galley edge
	     */
	    text_private->cb_x1  =
			vw->dvr_viewer.Dvr.current_page->pag_leftmost_gly_edge-
				text->txt_changebar_offset;

	    /* expand bounding box left side x */
    	    state->bbox_ll_x =	MIN(state->bbox_ll_x, text_private->cb_x1);

	    /* convert to pixels */
	    text_private->cb_x2  =
	    text_private->cb_x1  =
    	    	CONVERT_CP_TO_PIXEL_ROUNDED (text_private->cb_x1,
					     vw->dvr_viewer.x_dpi);


	  }

	/*  compute y coords for changebars; same for both left and
	 *  right pages, only vertical changebars are supported;
	 *
	 *  following decwrite's lead, the changebar will extend from
	 *  the top of the line's bounding box to the bottom of its bounding
	 *  box plus the space after for this line plus the leading for
	 *  this line.
	 */

 	space_after_value = text->txt_space_after;


	text_private->cb_y1  =
	    CONVERT_CP_TO_PIXEL_ROUNDED
		(
                dvr_change_y_orient(vw->dvr_viewer.Dvr.working_page_height,
				    text->txt_start_y)  -
		text->txt_descender   +
		space_after_value +
		text->txt_leading_size,

		vw->dvr_viewer.y_dpi);

	text_private->cb_y2  =

	    CONVERT_CP_TO_PIXEL_ROUNDED
		(
		dvr_change_y_orient(vw->dvr_viewer.Dvr.working_page_height,
				text->txt_start_y) -
                text->txt_ascender,

		vw->dvr_viewer.y_dpi);

#ifdef MSWINDOWS
    	dvr_adjust_print_coords(vw,
				&text_private->cb_x1,
			        &text_private->cb_y1);
    	dvr_adjust_print_coords(vw,
				&text_private->cb_x2,
				&text_private->cb_y2);
#endif

     	/* account for changebar including space after and leading */
    	state->bbox_ll_y = state->bbox_ll_y -
			   text->txt_space_after -
			   text->txt_leading_size;

	text_private->cb_wid = CONVERT_CP_TO_PIXEL_ROUNDED
					(text->txt_changebar_width,
					 vw->dvr_viewer.x_dpi );


	((OBJ_PRIVATE)text->txt_obj.obj_user_private)->private_info = (void *)text_private;

      }

    /* Successful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *  	dvr_format_fragment_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Formats text fragment object.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	fragment	- Text fragment structure
 *	state		- State data
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



#ifdef CDA_TWOBYTE
#ifndef CS16BIT
#define CS16BIT(csid)           ((csid == CDA_K_DEC_KANJI))
#endif
#endif

CDAstatus dvr_format_fragment_object (
    vw,
    fragment,
    state )


DvrViewerWidget vw;		/* Viewer widget context pointer */
FRG fragment;			/* Text fragment structure */
STATEREF state;			/* State data */

{
    /* Local variables */
    OBJ_PRIVATE object_private;	    /* Object private structure */
    FRG_PRIVATE fragment_private;   /* Fragment private structure */
    FRG_PRIVATE prev_frag_private;  /* Previous fragment private structure */
    char *string;		    /* Substring */
    CDAmeasurement str_pos;	    	    /* String character positions */
    CDAsize frag_pos;
    CDAmeasurement x_stretch;		    /* X stretch in pixels */
    CDAmeasurement x_stretch_remainder;	    /* X stretch calculation remainder in
				       centipoints */
    CDAmeasurement next_stretch;		    /* Stretch to apply to next text fragment */
    CDAstatus status;
    DvrViewerPtr dvr_window;

    char *font_name;		    /* Font name */
    int font_index;		    /* Font index */
    unsigned long *font_struct;     /* font struct to pass to X */

    CDAmeasurement x_pixel_len,       /* vars for computing X font metrics */
		 x_cp_len,
		 total_cp_xlen;

    CDAmeasurement cp_diff,
	x_stretch_pad_int;

    float x_stretch_pad;

    unsigned short int one_word = 0; /* tells if fragment is just one line */

#ifndef MSWINDOWS
    CDAsize font_name_len;
#endif
#if defined(OS2)
    unsigned long save_font_index;
#endif
#if defined(MSWINDOWS)
    HDC hDC;
#endif


    /* Check parameter validity */
    if (( vw == NULL ) || ( fragment == NULL ) || ( state == NULL ))
	return DVR_BADPARAM;

#if defined(OS2)
    save_font_index = vw->dvr_viewer.current_font_index;
#endif


    /*
     * Before we format the fragment check to see if we need to
     * convert any minus characters to hyphens (eg: 2D -> AD).
     * Note that this conversion is done only for ISO Latin-1.
     *
     * This check must be done before we proceed since the correct
     * information must be past to the X mesasurement routines in
     * order for them to match up properly with the Layout Engine
     * measurements.
     */
#ifdef MSWINDOWS
     if (fragment->frg_finc->fin_csid == CDA_K_ISO_LATIN1)
#else
     if (fragment->frg_finc->fin_fenc->fec_csid == CDA_K_ISO_LATIN1)
#endif
	{
	    /*
	     * If we're dealing with ISO Latin-1 check the string.
	     *
	     * Note that we should most likely optimize this so we
	     * don't perform this check all th time by adding a flag
	     * to the FRG struture.
	     */

	     utext	*f_ptr;			/* text fragment pointer    */
	     signlong	f_len;			/* text fragment length	    */

	     f_ptr = fragment->frg_ptr;		/* text in the fragment   */
	     f_len = fragment->frg_len;		/* length of the fragment */

	     /*
	      * Scan the line looking for 2D's (minus) that need to be converted to AD's (hyphen).
	      */

	     while (f_len != 0)
	         {
		     if (*f_ptr == (utext)0x2D)
		         *f_ptr = (utext)0xAD;

		     f_ptr++;
		     f_len--;
		 };
	};

    /*
     *	Format text fragment.
     */

    dvr_window = (DvrViewerPtr)  &(vw->dvr_viewer);

#ifdef MSWINDOWS
    hDC = 0;
    font_index = (int)fragment->frg_finc->fin_xfont_index;
    font_name = (char *) fragment->frg_finc->fin_xfont;
    if (fragment->frg_spaces == 0)
        one_word = 1;

    if (Work(vw) != 0)
       hDC = GetDC (Work(vw));
    else
       /* off screen device (e.g. printer), hdc should already be present */
       hDC = DvrHps(vw);

    SelectObject (hDC, font_index);
    font_struct = (unsigned long *) &hDC;
#else
    if (vw->dvr_viewer.num_fonts == 0) {
    /*  if we have no fonts available, use one and only font for
     *  calculating font metrics
     */
    font_index = 0;

    font_name_len = 256L;
    DVR_ALLOCATE_MEMORY(font_name_len, &font_name);

    strcpy(font_name,
	   "-ADOBE-Courier-Medium-R-Normal--12-120-75-75-M-70-ISO8859-1");
    }

    else {

    font_index = (int) fragment->frg_finc->fin_xfont_index;
    font_name = (char *) fragment->frg_finc->fin_xfont;
    }

    /*  get font info for this fragment from X; this is done here in the
     *  format phase so we can query X for the length of each fragment,
     *  & compare it to the length that the fragment should be (from the
     *  engine).  If the X font metrics create a string of different length
     *  then the engine, the X stretch is adjusted so that the text will
     *  allign on the X display.
     */

#if defined(OS2)
    if (font_index == DEFAULT_FONT_INDEX)
      {
	vw->dvr_viewer.current_font_index = vw->dvr_viewer.default_font_id;
	if (vw->dvr_viewer.default_font_id)
	    font_name = (char *) vw->dvr_viewer.available_fonts
					[vw->dvr_viewer.default_font_id];
      }
    else
	vw->dvr_viewer.current_font_index = font_index;
#endif

#ifdef DEBUG
    printf("\nget font info\n");
#endif

    status = dvr_get_font_info ( vw, (long) font_index, font_name );

#ifdef DEBUG
    printf("\ngot font info\n");
#endif

    if ( fragment->frg_spaces == 0 )
	/*  if the engine thinks that no x-stretch is necessary,
	 *  then set flag; in this case, we have to stretch out the
	 *  single word to make the X word the same length that the engine
	 *  thought it should be; in effect, this means that every fragment
	 *  will have some x_stretch unless the X font matches the print
	 *  font metrics exactly;
	 */

#ifdef CDA_TWOBYTE
      {
	one_word = 1;
	/* DVS set frg_x_stretch as a stretch between Kanji character when
	   frg_spaces == 0 */
	fragment->frg_x_stretch = 0;
      }
#else
	one_word = 1;
#endif

#ifdef DEBUG
    printf("\nnum fonts is %ld\n", vw->dvr_viewer.num_fonts);
#endif

    if (vw->dvr_viewer.num_fonts != 0) {

    /* get X font struct from widget */
    if (font_index == DEFAULT_FONT_INDEX)
	font_struct = dvr_window->default_font_struct;
    else
	font_struct = dvr_window->font_table[font_index].x_font_struct;
    }
#endif /* End of ifndef MSWINDOWS */
    /*
     *  RECALCULATE X-STRETCH FOR FRAGMENT BASED ON X FONT METRICS
     */

    if (one_word) {

        /*  if there's only one word, then we need only call
	 *  XTextWidth() once
	 */

#ifdef DEBUG
printf("\ncall text width\n");
#endif

	x_pixel_len = dvr_text_width(vw,
				 (unsigned long *) font_struct,
				 (char *) fragment->frg_ptr,
				 fragment->frg_len);

#ifdef DEBUG
	printf("\nwidth is %ld\n", x_pixel_len);
#endif

	total_cp_xlen = CONVERT_PIXEL_TO_CP(x_pixel_len, vw->dvr_viewer.x_dpi);

    }

    else {

        /*  else, more than one word, call XTextWidth() for each
	 *  word in fragment; (separated by spaces in engine)
	 */

        frag_pos = 0;	    /* Position within fragment string */
        str_pos  = 0;	    /* Position within substring */
        string   = (char *) fragment->frg_ptr;
        total_cp_xlen 	= 0;

	while ( frag_pos < fragment->frg_len ) {

	    /* More string to calculate length */

	    while (( frag_pos < fragment->frg_len ) &&
		   ( string[str_pos] != ' ' )) {

		++frag_pos; ++str_pos; /* find space */
            }

	    if ( frag_pos < fragment->frg_len ) {
		/* Not at end of fragment string; increment to include space */
		++frag_pos; ++str_pos;
            }

#ifdef DEBUG
	    printf("\nget text width2\n");
#endif

	    /* space found, get pixel length of word */
	    x_pixel_len = dvr_text_width(vw,
				     (unsigned long *) font_struct,
				     string,
				     str_pos);

#ifdef DEBUG
	    printf("\ntext width is %ld\n", x_pixel_len);
#endif

	    /* convert to centipoints */
	    x_cp_len = CONVERT_PIXEL_TO_CP(x_pixel_len, vw->dvr_viewer.x_dpi);

	    /* add to total */
	    total_cp_xlen = total_cp_xlen + x_cp_len;

	    /* advance string */
	    string = (char *) &string[str_pos];
	    str_pos = 0;
        }

    }

    /* add in the stretch that the engine has calculated to get total
     * length that the fragment would be on an X-display (in centipoints)
     */
    total_cp_xlen = total_cp_xlen +
			    (fragment->frg_spaces * fragment->frg_x_stretch);

    /* get difference from engine total x length */
    cp_diff = fragment->frg_x_size - total_cp_xlen;

    /*  calculate the number of centipoints that X should add to the
     *  X-stretch; this is eqaul to the total difference divided by the
     *  total number of spaces in the line. (or characters in the word in
     *  the one word case.)
     */
    if (one_word)
      {
	if (fragment->frg_len != 0)
#ifdef CDA_TWOBYTE
	    if (CS16BIT (fragment->frg_finc->fin_csid))
	      /* 16bit text will be broken by 2-byte. */
	      if (fragment->frg_len > 2)
		x_stretch_pad = (float) cp_diff / (float) ((fragment->frg_len/2)-1);
	      else
		x_stretch_pad = (float) cp_diff / (float) (fragment->frg_len/2);
	    else
	      x_stretch_pad = (float) cp_diff / (float) fragment->frg_len;
#else
	    /* Non-CDA_TWOBYTE case */

	    x_stretch_pad = (float) cp_diff / (float) fragment->frg_len;
#endif
	else
	    x_stretch_pad = (float) cp_diff;
      }
    else
	x_stretch_pad = (float) cp_diff / (float) fragment->frg_spaces;

    /* convert to integer */
    x_stretch_pad_int = (int) (x_stretch_pad + 0.5);

    /* add to stretch */
    fragment->frg_x_stretch = fragment->frg_x_stretch +
			      x_stretch_pad_int;

    /*
     * SET X-STRETCH FOR EACH WORD WITHIN FRAGMENT
     */

    /*
     *	Additional text fragment positioning work is accomplished
     *	right here.  Specifically, support relating to stretch and rendition
     *	attributes is performed here.  Stretched elements should be
     *	broken out and 'stretched' here once (rather then each time in
     *	the display phase), and represented by pointers into the main
     *	fragment string using private data elements.
     */

    /* Initilaize before positioning words within fragment */

    object_private = (OBJ_PRIVATE) fragment->frg_obj.obj_user_private;
    frag_pos = 0;	    /* Position within fragment string */
    str_pos = 0;	    /* Position within substring */
    string = (char *) fragment->frg_ptr;
    prev_frag_private = NULL;
    fragment_private = NULL;

    x_stretch = CVT_CP_TO_PIXEL_TRUNC_WITH_REM
    		    (fragment->frg_x_stretch,x_stretch_remainder,
		     vw->dvr_viewer.x_dpi);

    /* break string into words, space separated;
       Note: Y stretch not supported */

    while ( frag_pos < fragment->frg_len ) {

	    /* More string to format */

	    if (one_word) {

		/*  if there is only one word, stretch out the
		 *  characters of the word so it is the same length that
		 *  the engine thinks it is;
                 */
	 	str_pos = 1;
		frag_pos++;
#ifdef CDA_TWOBYTE
		if (CS16BIT (fragment->frg_finc->fin_csid)) {
		    /* 16 bit text cannot be broken into 1 byte. */
		    str_pos = 2;
		    frag_pos++;
		}
#endif
	    }

	    else {

	    /* else find the next word in the fragment */

	    while (( frag_pos < fragment->frg_len ) &&
		( string[str_pos] != ' ' )) {

		++frag_pos; ++str_pos;
            }

	    if ( frag_pos < fragment->frg_len ) {
		/* Not at end of fragment string; increment to include space */
		++frag_pos; ++str_pos;
            }

	    }

	    /* Save substring information; allocate memory, then fill
	       in correct information */

	    status = dvr_alloc_struct (
		vw,
		frg_private,
		(void **) &fragment_private );

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
#ifdef MSWINDOWS
                if (Work(vw) != 0)
		   ReleaseDC (Work(vw), hDC);
#endif
		return status;
	    }

	    /* Allocation successful; fill in data */

	    fragment_private->string = string;
	    fragment_private->str_len = (int) str_pos;
	    fragment_private->x_stretch = (int) x_stretch;
	    fragment_private->next = NULL;	/* Init */

	    /* Hook up to previous element, or to fragment object private
	       structure if first substring */

	    if ( prev_frag_private == NULL ) {
		/* First substring; hook up to object private structure */
		object_private->private_info = (void *)fragment_private;
	    }
	    else {
		/* Hook up to previous fragment private object */
		prev_frag_private->next = fragment_private;
	    }


	    /* Set up for next substring; loop */

	    string = (char *) &string[str_pos];
	    str_pos = 0;	/* Reinit */
	    prev_frag_private = fragment_private;
	    next_stretch = fragment->frg_x_stretch + x_stretch_remainder;

	    /* Calculate next stretch value and new corresponding remainder */
	    x_stretch = CVT_CP_TO_PIXEL_TRUNC_WITH_REM
		(next_stretch,x_stretch_remainder, vw->dvr_viewer.x_dpi);
	}

    /* Store X,Y offsets (positions in state for object positioning
     * in page coordinate system
     */
    state->x_offset = state->x_offset + fragment->frg_x;
    state->y_offset = state->y_offset + fragment->frg_y;

    /* Use X,Y stored sizes for bounding box determination relative
       to current position (not necessarily relative to X,Y positions
       specified in fragment object */

    state->bbox_ll_x = state->x_offset;
    /* Parent TXT descender value is signed, and is probably negative.
       (Note: fragment->frg_y not used in this context) */
    state->bbox_ll_y = state->y_offset +
			((TXT) fragment->frg_obj.obj_parent)->txt_descender;

    state->bbox_ur_x = state->x_offset + fragment->frg_x_size;
    /* Parent TXT ascender value is signed, and is probably positive.
       (Note: fragment->frg_y_size not used in this context) */
    state->bbox_ur_y = state->y_offset + 
		       ((TXT) fragment->frg_obj.obj_parent)->txt_ascender;

#ifdef MSWINDOWS
    /*  if we're printing (no work window if we're printing), 
     *  then we have to offset the y coordinate because SetTextAlign()
     *  seems to have no effect on HP PCL printers
     */
    if (Work(vw) == 0)
        state->y_offset += fragment->frg_finc->fin_font_metrics.fom_bbox_ury;
#endif

#ifdef old_OS2
    vw->dvr_viewer.current_font_index = save_font_index;
#endif
#ifdef MSWINDOWS
    if (Work(vw) != 0)
       ReleaseDC (Work(vw), hDC);
#endif

    /* Successful completion */
    return DVR_NORMAL;
}



/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_adjust_state_bbox
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Adjusts state bounding box size to account for graphic line widths.
 *	That is, bounding boxes may represent or include line drawings, and
 *	the actual bbox coordinates designate the line centers, rather than
 *	the 'outside' of the line, relative to its box representation.  This
 *	routine simply adds half the line width to the upper left and lower
 *	right state bbox coordinates to account for this possibility.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	state		- State structure
 *	object		- Object structure
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_adjust_state_bbox (
    vw,
    state,
    object )

DvrViewerWidget vw;		/* Viewer widget context pointer */
STATEREF state;			/* State structure */
OBJ object;			/* Object structure */

{
    /* Local variables */
    FRM frame;			/* Frame structure */
    GLY galley;			/* Galley structure */
    LIN polyline;		/* Polyline structure */
    CRV curve;			/* Curve structure */
    ARC arc;			/* Arc structure */
    GAT attributes;		/* Graphic attributes */
    CDAmeasurement line_width;		/* Graphic line thickness */

    /* Check parameter validity */
    if (( vw == NULL ) || ( state == NULL ) || ( object == NULL ))
	return DVR_BADPARAM;

    /*
     *	Add half of line thickness to bounding box coordinates.  Get
     *	graphics attributes from object structure, and calculate.
     */

    switch ( object->obj_type ) {

    case obj_frm:

	/* Frame object */
	frame = (FRM) object;
	attributes = (GAT) &frame->frm_attributes;

	break;

    case obj_gly:

	/* Galley object */
	galley = (GLY) object;
	attributes = (GAT) &galley->gly_attributes;

	break;

    case obj_lin:

	/* Polyline object */
	polyline = (LIN) object;
	attributes = (GAT) &polyline->lin_attributes;

	break;

    case obj_crv:

	/* Curve object */
	curve = (CRV) object;
	attributes = (GAT) &curve->crv_attributes;

	break;

    case obj_arc:

	/* Arc object */
	arc = (ARC) object;
	attributes = (GAT) &arc->arc_attributes;

	break;

    default:

	/* Non-supported type; can't add line thickness */

	/* Note: Although fill (FIL) object type includes a graphics
	   attribute structure, it logically does not specify a line
	   thickness to apply, rather a pattern to apply within the
	   confines of a specified outline or bounding box */

	return DVR_NORMAL;

    }


    /* Found attribute structure; get line thickness */
    line_width = attributes->gat_line_width;


    /* Add half of width to all sides of state bounding box; subtract
       from upper left coordinates (minimize) and add to lower right
       coordinates (maximize) */

    line_width /= 2;	/* Divide width by 2 */

    state->bbox_ll_x -= line_width;
    state->bbox_ll_y -= line_width;

    state->bbox_ur_x += line_width;
    state->bbox_ur_y += line_width;


    /* Succesful completion */
    return DVR_NORMAL;
}


/*
**++
**  FUNCTIONAL DESCRIPTION: dvr_bbox_to_path
**
**      Create a path from bounding box coordinates.  by PBD
**
**  FORMAL PARAMETERS:
**
**      ll_x, ll_y, ur_x, ur_y : the bounding box (In by Value)
**	path : the resulting path (Out by Ref)
**
**  IMPLICIT OUTPUTS:
**
**      ptr to new pth allocated is stored in context
**
**  COMPLETION CODES:
**
**      [@description_or_none@]
**
**  SIDE EFFECTS:
**
**      array of ptrs to allocate pth structures may be reallocated larger
**
**--
**/

CDAstatus dvr_bbox_to_path(vw, ll_x, ll_y, ur_x, ur_y, path)
DvrViewerWidget	vw;
CDAmeasurement	ll_x, ll_y, ur_x, ur_y;
PTH	*path;
{
    CDAstatus	status;
    CDAsize	alloc_size;
    PTH			new_path;
    CDAmeasurement	*coord_ptr;

    alloc_size = sizeof(struct pth) + 3 * 2 * sizeof(CDAmeasurement);
	/* 3 more points, 2 longs each.  pth struct has space for two points*/

    status = dvr_alloc_memory (
	vw,
	alloc_size,
	(void **) &new_path );

    if (!DVRSuccess(status))
	return(status);

    new_path->pth_str.str_length = alloc_size;
    QUE_INIT(new_path->pth_str.str_queue);
    new_path->pth_type = pth_lin;
    new_path->pth__specific.pth__xy.pth_pairs = 5;
    coord_ptr = &(new_path->pth__specific.pth__xy.pth_first_x);

    *coord_ptr = ll_x;		    /* ll */
    *(coord_ptr + 1) = ll_y;
    coord_ptr += 2;

    *coord_ptr = ur_x;		    /* lr */
    *(coord_ptr + 1) = ll_y;
    coord_ptr += 2;

    *coord_ptr = ur_x;		    /* ur */
    *(coord_ptr + 1) = ur_y;
    coord_ptr += 2;

    *coord_ptr = ll_x;		    /* ul */
    *(coord_ptr + 1) = ur_y;
    coord_ptr += 2;

    *coord_ptr = ll_x;		    /* ll again to close path */
    *(coord_ptr + 1) = ll_y;

    *path = new_path;

    status = dvr_record_alloc_path(vw, new_path);

    return(status);

}



/*
**++
**  FUNCTIONAL DESCRIPTION: dvr_transform_path
**
**	Transform a path according to the current transformation, update the
**	bounding box, and then transform the path to pixel coordinates. by PBD
**
**  FORMAL PARAMETERS:
**
**      vw : viewer context pointer (by value)
**	state : ptr to state structure that contains the transform and the
**		    bounding box    (by value)
**	in_path : input path structure ptr (by value)
**	out_path : output path structure ptr (by reference)
**
**  IMPLICIT INPUTS:
**
**      working_page_height in context structure
**
**  IMPLICIT OUTPUTS:
**
**      bounding box in state structure updated
**
**  COMPLETION CODES:
**
**      DVR_NORMAL or DVR_GRAPHICFAIL (not a line, curve or arc)
**
**  SIDE EFFECTS:
**
**      Coordinates of in_path are NOT transformed in place.  out_path
**	points to a newly allocated pth structure.
**
**--
**/

CDAstatus
dvr_transform_path( vw, state, flags, in_path, out_path, engine_path,
		    rota_round_path, private_info)
DvrViewerWidget	vw;
STATEREF	state;
longword	flags;
PTH		in_path;
PTH		* out_path;
PTH		engine_path;
PTH		rota_round_path;
void		* private_info;
{
    CDAstatus	status = DVR_NORMAL;
    PTH			next_path, out_next_path;
    CDAmeasurement	*coord_ptr, *out_coord_ptr;
    longword		counter;
    CDAmeasurement		x_max, x_min, y_max, y_min;
    ARR_INFO		arrow_struct = NULL;
    PTH			transformed_path;

    status = dvr_alloc_memory(vw, in_path->pth_str.str_length, (void **) out_path);
    if (!DVRSuccess(status))
	return(status);

    status = dvr_record_alloc_path(vw, *out_path);
    if (!DVRSuccess(status))
	return(status);

    QUE_INIT((*out_path)->pth_str.str_queue);
    QUE_INIT((*out_path)->pth__specific.pth__xy.pth_qjoin);
    (*out_path)->pth_str.str_length = in_path->pth_str.str_length;
    (*out_path)->pth_type = in_path->pth_type;

    switch (in_path->pth_type)
    {

    case pth_lin:
    case pth_crv:
    case pth_rotated:
        {
	LIN_PRIVATE * private;
	CDAmeasurement	    radius_dpi;

        private = (LIN_PRIVATE *) private_info;

        /*
	** If private is NULL then there can not be arrow heads
	*/
        if ( private != NULL)
           if ( * private != NULL)
              arrow_struct = (* private)->arrow_info;

	(*out_path)->pth__specific.pth__xy.pth_pairs =
				    in_path->pth__specific.pth__xy.pth_pairs;

	/* first point - update bounding box */

	if ((in_path->pth__specific.pth__xy.pth_first_x + state->x_relative) < state->bbox_ll_x)
	    state->bbox_ll_x = in_path->pth__specific.pth__xy.pth_first_x + state->x_relative;
	if ((in_path->pth__specific.pth__xy.pth_first_y + state->y_relative) < state->bbox_ll_y)
	    state->bbox_ll_y = in_path->pth__specific.pth__xy.pth_first_y + state->y_relative;
	if ((in_path->pth__specific.pth__xy.pth_first_x + state->x_relative) > state->bbox_ur_x)
	    state->bbox_ur_x = in_path->pth__specific.pth__xy.pth_first_x + state->x_relative;
	if ((in_path->pth__specific.pth__xy.pth_first_y + state->y_relative) > state->bbox_ur_y)
	    state->bbox_ur_y = in_path->pth__specific.pth__xy.pth_first_y + state->y_relative;

	/* add offset */
	(*out_path)->pth__specific.pth__xy.pth_first_x =
		in_path->pth__specific.pth__xy.pth_first_x + state->x_relative; 
	(*out_path)->pth__specific.pth__xy.pth_first_y =
		in_path->pth__specific.pth__xy.pth_first_y + state->y_relative;

	/* change y coord to ul-lr */
	(*out_path)->pth__specific.pth__xy.pth_first_y =
		dvr_change_y_orient(vw->dvr_viewer.Dvr.working_page_height,
			       (*out_path)->pth__specific.pth__xy.pth_first_y);

	/* convert to pixel */
	(*out_path)->pth__specific.pth__xy.pth_first_x =
	    CONVERT_CP_TO_PIXEL((*out_path)->pth__specific.pth__xy.pth_first_x,
				vw->dvr_viewer.x_dpi);
	(*out_path)->pth__specific.pth__xy.pth_first_y =
	    CONVERT_CP_TO_PIXEL((*out_path)->pth__specific.pth__xy.pth_first_y,
				vw->dvr_viewer.y_dpi);

#ifdef MSWINDOWS
    	dvr_adjust_print_coords
		(vw,
		 &(*out_path)->pth__specific.pth__xy.pth_first_x,
		 &(*out_path)->pth__specific.pth__xy.pth_first_y);
#endif

	/* now transform the rest of the points */
	coord_ptr = &(in_path->pth__specific.pth__xy.pth_second_x);
	out_coord_ptr = &((*out_path)->pth__specific.pth__xy.pth_second_x);
	counter = 1;
	while (counter < in_path->pth__specific.pth__xy.pth_pairs)
	    {
		/* Note: state bbox coordinates should be maximized/minimized
		   as appropriate, rather than overwritten with information
		   obtained from this path transform */
		CDAmeasurement *x_val;

		/* x coordinate */
		if ((*coord_ptr + state->x_relative) < state->bbox_ll_x )
		    state->bbox_ll_x = *coord_ptr + state->x_relative;
		if ((*coord_ptr + state->x_relative) > state->bbox_ur_x )
		    state->bbox_ur_x = *coord_ptr + state->x_relative;
		*out_coord_ptr = *coord_ptr + state->x_relative; 
		*out_coord_ptr = CONVERT_CP_TO_PIXEL(*out_coord_ptr,
					vw->dvr_viewer.x_dpi);	/* x coord */
#ifdef MSWINDOWS
		x_val = out_coord_ptr;
#endif
		coord_ptr++;
		out_coord_ptr++;

		/* y coordinate */
		if ((*coord_ptr + state->y_relative) < state->bbox_ll_y )
		    state->bbox_ll_y = *coord_ptr + state->y_relative;
		if ((*coord_ptr + state->y_relative)> state->bbox_ur_y )
		    state->bbox_ur_y = *coord_ptr + state->y_relative;
		*out_coord_ptr = *coord_ptr + state->y_relative; 
		/* change y coord to ul-lr */
		*out_coord_ptr =
			dvr_change_y_orient(
				vw->dvr_viewer.Dvr.working_page_height,
				*out_coord_ptr);
		*out_coord_ptr = CONVERT_CP_TO_PIXEL(*out_coord_ptr,
					vw->dvr_viewer.y_dpi);

#ifdef MSWINDOWS
    		dvr_adjust_print_coords(vw,
					x_val,
					out_coord_ptr);
#endif

		coord_ptr++;
		out_coord_ptr++;
		counter ++;
	    }
            /*
	    ** If the path is rounded and there are markers on the path
	    ** then engine defined path has to be transformed to pixel
	    ** and stored in the LIN_PRIVATE
	    */
            if ( rota_round_path && (flags & ddif_m_lin_draw_markers) )
	       {
	       status = dvr_transform_path( vw,
					    state,
					    0,
					    engine_path,
					    & (* private)->eng_path,
					    NULL,
					    NULL,
					    NULL);

	       if (!DVRSuccess(status))
		    return(status);
	       }
        }
	break;

    case pth_arc:
	{

	ARC_PRIVATE *a_private;
	CDAmeasurement	    radius_dpi;

        a_private = (ARC_PRIVATE *) private_info;

        if ( a_private != NULL)
           if ( * a_private != NULL)
              arrow_struct = (* a_private)->arrow_info;
	/* Angle is not considered when calulating the bounding box.  The   */
	/* bounding box is calculated as if the arc were a full circle (or  */
	/* ellipse).							    */

        if ( in_path->pth__specific.pth__arc.pth_rotation == 0)
	   {
	   x_max = in_path->pth__specific.pth__arc.pth_center_x +
			DVR_ABS(in_path->pth__specific.pth__arc.pth_radius_x);
	   x_min = in_path->pth__specific.pth__arc.pth_center_x -
			DVR_ABS(in_path->pth__specific.pth__arc.pth_radius_x);
	   y_max = in_path->pth__specific.pth__arc.pth_center_y
		    + DVR_ABS(in_path->pth__specific.pth__arc.pth_radius_x +
			      in_path->pth__specific.pth__arc.pth_radius_y);
	   y_min = in_path->pth__specific.pth__arc.pth_center_y
		    - DVR_ABS(in_path->pth__specific.pth__arc.pth_radius_x +
			      in_path->pth__specific.pth__arc.pth_radius_y);
           }
        else
           {
	   /* To simplify the bounding box computation for the rotated ellipses
           ** the maximun of the x_radius and y_radius is added/subtracted to
	   ** determine the the bounding box coordinates
	   */

           CDAmeasurement		max_radius, y_radius;

           y_radius = DVR_ABS(in_path->pth__specific.pth__arc.pth_radius_x +
                              in_path->pth__specific.pth__arc.pth_radius_y);
           max_radius = DVR_ABS(in_path->pth__specific.pth__arc.pth_radius_x);

           if ( y_radius > max_radius)
              max_radius = y_radius;
	   x_max = in_path->pth__specific.pth__arc.pth_center_x + max_radius;
	   x_min = in_path->pth__specific.pth__arc.pth_center_x - max_radius;
	   y_max = in_path->pth__specific.pth__arc.pth_center_y + max_radius;
	   y_min = in_path->pth__specific.pth__arc.pth_center_y - max_radius;

           }

	if (x_min + state->x_relative < state->bbox_ll_x)
	    state->bbox_ll_x = x_min + state->x_relative;
	if (x_max + state->x_relative > state->bbox_ur_x)
	    state->bbox_ur_x = x_max + state->x_relative;
	if (y_min + state->y_relative < state->bbox_ll_y)
	    state->bbox_ll_y = y_min + state->y_relative;
	if (y_max + state->y_relative > state->bbox_ur_y)
	    state->bbox_ur_y = y_max + state->y_relative;

	/* add offset to center */
	(*out_path)->pth__specific.pth__arc.pth_center_x =
               in_path->pth__specific.pth__arc.pth_center_x + state->x_relative;
	(*out_path)->pth__specific.pth__arc.pth_center_y =
	       in_path->pth__specific.pth__arc.pth_center_y + state->y_relative;

	/* change y coord to ul-lr */
	(*out_path)->pth__specific.pth__arc.pth_center_y =
		dvr_change_y_orient(vw->dvr_viewer.Dvr.working_page_height,
			     (*out_path)->pth__specific.pth__arc.pth_center_y);
	/* convert center coords to pixels */
	(*out_path)->pth__specific.pth__arc.pth_center_x =
	  CONVERT_CP_TO_PIXEL((*out_path)->pth__specific.pth__arc.pth_center_x,
			      vw->dvr_viewer.x_dpi);
	(*out_path)->pth__specific.pth__arc.pth_center_y =
	  CONVERT_CP_TO_PIXEL((*out_path)->pth__specific.pth__arc.pth_center_y,
			      vw->dvr_viewer.y_dpi);

#ifdef MSWINDOWS
	dvr_adjust_print_coords(vw,
		&(*out_path)->pth__specific.pth__arc.pth_center_x,
		&(*out_path)->pth__specific.pth__arc.pth_center_y);
#endif

	/* copy arc start, extent and rotation */
	(*out_path)->pth__specific.pth__arc.pth_start =
				     in_path->pth__specific.pth__arc.pth_start;
	(*out_path)->pth__specific.pth__arc.pth_extent =
				     in_path->pth__specific.pth__arc.pth_extent;
	(*out_path)->pth__specific.pth__arc.pth_rotation =
				   in_path->pth__specific.pth__arc.pth_rotation;

	/*  NOTE: path radius should be converted to pixel down in
 	 *  calc_arc_endpoints(); here we just put in the cp values
	 */
	(*out_path)->pth__specific.pth__arc.pth_radius_x =
	          		in_path->pth__specific.pth__arc.pth_radius_x;
	(*out_path)->pth__specific.pth__arc.pth_radius_y =
	          		in_path->pth__specific.pth__arc.pth_radius_y;

	/* for now, let's assume that a 'full circle' implies that no radial
	   lines be drawn; a 'full circle' is assumed to be an arc that extends
	   360 degrees or more; values are stored in hundreths of degrees */
	if (a_private != NULL)
        /* calc arc endpoints */
	  {
	    status = calc_arc_endpoints(vw, *out_path, a_private);
	    if (!DVRSuccess(status))
		return(status);
	  }
	else
	  {
            /* don't think this should ever happen; but in the case that
	     * it does, fill in radius x and y just in case
	     */

	    /*  radius is only converted to pixels, no other transformation;
	     *  for the radius dpi, take an average of x and y as our best guess
	     */
	    radius_dpi = (vw->dvr_viewer.x_dpi + vw->dvr_viewer.y_dpi) / 2;

	    (*out_path)->pth__specific.pth__arc.pth_radius_x =
	          CONVERT_CP_TO_PIXEL(in_path->pth__specific.pth__arc.pth_radius_x,
			      	      radius_dpi);
	    (*out_path)->pth__specific.pth__arc.pth_radius_y =
	          CONVERT_CP_TO_PIXEL(in_path->pth__specific.pth__arc.pth_radius_y,
				      radius_dpi);
	  }

        /*
	** If the the path is rotated then transform the rotated path to pixel
	** coordinates. After the original rotated path is transformed it can
	** be deallocated.
	*/
        if ( in_path->pth__specific.pth__arc.pth_rotation != 0)
	   {
	   status = dvr_transform_path( vw,
					state,
					0,
					(* a_private)->rotated_path,
					&transformed_path,
					NULL,
					NULL,
					(void *)NULL
					);

	   if (!DVRSuccess(status))
              return(status);

	   status = DVR_DEALLOCATE_MEMORY((* a_private)->rotated_path->pth_str.str_length,
					  & (* a_private)->rotated_path);
	   if (!DVRSuccess(status))
              return(status);

           (* a_private)->rotated_path = transformed_path;

	   }

	}
	break;

    default:	/* unknown path type */
	status = DVR_GRAPHICFAIL;
	break;
    }

    /*
    ** Check to see if there are arrow heads on the Path. If there are then
    ** transform the arrow_path (i.e the path that is shortend for arrow heads)
    ** and also transform the start arrow head and end arrow head to pixel
    ** Coordinates. After the original arrow head path and the arrow heads are
    ** transformed to pixel coordinates they are deallocated.
    */

    if ( arrow_struct )
       {
       status = dvr_transform_path( vw,
				    state,
				    0,
				    arrow_struct->arrow_path,
				    &transformed_path,
				    NULL,
				    NULL,
				    (void *)NULL
				  );

	if (!DVRSuccess(status))
           return(status);

	status = DVR_DEALLOCATE_MEMORY( arrow_struct->arrow_path->pth_str.str_length,
					&arrow_struct->arrow_path);
	if (!DVRSuccess(status))
           return(status);

        arrow_struct->arrow_path = transformed_path;

        if (arrow_struct->start_arrow)
	   {
           status = dvr_transform_path( vw,
				        state,
				        0,
				        arrow_struct->start_arrow,
				        & transformed_path,
				        NULL,
				        NULL,
				        (void *)NULL
				      );

	  if (!DVRSuccess(status))
           return(status);

	status = DVR_DEALLOCATE_MEMORY( arrow_struct->start_arrow->pth_str.str_length,
					&arrow_struct->start_arrow);
	if (!DVRSuccess(status))
           return(status);

        arrow_struct->start_arrow = transformed_path;
        }

       if (arrow_struct->end_arrow)
	  {
          status = dvr_transform_path( vw,
				       state,
				       0,
				       arrow_struct->end_arrow,
				       &transformed_path,
				       NULL,
				       NULL,
				       (void *)NULL
				     );

	  if (!DVRSuccess(status))
             return(status);

	  status = DVR_DEALLOCATE_MEMORY( arrow_struct->end_arrow->pth_str.str_length,
					&arrow_struct->end_arrow);
	  if (!DVRSuccess(status))
             return(status);

          arrow_struct->end_arrow = transformed_path;
          }

       }
    FOREACH_INLIST(in_path->pth_str.str_queue, next_path, PTH)
	{
	    status = dvr_transform_path (vw,
					 state,
					 0,
					 next_path,
					 &out_next_path,
					 NULL,
					 NULL,
					 (void *)NULL);

	    /* Since no longer updating in place, do */
	    QUE_APPEND((*out_path)->pth_str.str_queue, out_next_path)

	    if (!DVRSuccess(status))
		return(status);
	}

    return(status);
}



/*
**++
**  FUNCTIONAL DESCRIPTION: calc_arc_endpoints
**
**	Allocate an arc_private structure, calculate the arc endpoints, and
**	store them in the structure.  This is intended to be called after the
**	arc has been transformed to the ul pixel coordinate system.  If it is
**	called before the transformation, the angles will effectively be
**	negated (clockwise instead of ccw).
**
**  FORMAL PARAMETERS:
**
**      path : PTH pointer to pth path structure (In, by value)
**	arc_data : ARC_PRIVATE ptr to arc_private structure (Out, by reference)
**
**  COMPLETION CODES:
**
**      DVR_NORMAL or DVR_MEMALLOCFAIL
**
**  SIDE EFFECTS:
**
**      arc_private structure is allocated
**
**--
**/

CDAstatus	calc_arc_endpoints(vw, path, arc_data)
DvrViewerWidget	vw;
PTH		path;
ARC_PRIVATE	*arc_data;
{

    double	start_angle, end_angle, middle_angle, x, y;
    CDAstatus	status = DVR_NORMAL;
    CDAmeasurement	x_radius, y_radius;

   if (* arc_data == NULL)
      status = dvr_alloc_struct(vw, arc_private, (void **) arc_data);
    if (!DVRSuccess(status))
	return(status);

    /*  convert x and y radius to pixel values; NOTE: y radius stored in
     *  path is just an offset from the x radius
     */
    x_radius =
	CONVERT_CP_TO_PIXEL(path->pth__specific.pth__arc.pth_radius_x,
			    vw->dvr_viewer.x_dpi);
    y_radius =
	CONVERT_CP_TO_PIXEL(path->pth__specific.pth__arc.pth_radius_x +
			    path->pth__specific.pth__arc.pth_radius_y,
			    vw->dvr_viewer.y_dpi);

    /* store radius pixel values back in path */
    path->pth__specific.pth__arc.pth_radius_x = x_radius;

    path->pth__specific.pth__arc.pth_radius_y =
	CONVERT_CP_TO_PIXEL(path->pth__specific.pth__arc.pth_radius_y,
			    vw->dvr_viewer.y_dpi);

    start_angle = (double)path->pth__specific.pth__arc.pth_start
							* PI / (100.0 * 180.0);
    end_angle = (double)(path->pth__specific.pth__arc.pth_start
			    + path->pth__specific.pth__arc.pth_extent)
			* PI / (100.0 * 180.0);	 /* centidegrees to radians */

    middle_angle = (double)(path->pth__specific.pth__arc.pth_start
			    + ( (long) path->pth__specific.pth__arc.pth_extent/2 ) )
			* PI / (100.0 * 180.0);	 /* centidegrees to radians */

    x = (double)x_radius * cos(start_angle);
    y = (double)(y_radius) * -sin(start_angle);

    (*arc_data)->first_endpoint_x = (long)floor((double)0.5 + x)   /* round */
				    + path->pth__specific.pth__arc.pth_center_x;

#ifdef OS2 /* lower left 0,0 */
    (*arc_data)->first_endpoint_y = path->pth__specific.pth__arc.pth_center_y -
				    (long)floor((double)0.5 + y);   /* round */
#else      /* upper left 0,0 */

    (*arc_data)->first_endpoint_y = (long)floor((double)0.5 + y)   /* round */
				    + path->pth__specific.pth__arc.pth_center_y;
#endif

    x = (double)x_radius * cos(end_angle);
    y = (double)(y_radius)  * -sin(end_angle);

    (*arc_data)->second_endpoint_x = (long)floor((double)0.5 + x)   /* round */
				    + path->pth__specific.pth__arc.pth_center_x;

#ifdef OS2 /* lower left 0,0 */
    (*arc_data)->second_endpoint_y = path->pth__specific.pth__arc.pth_center_y -
                                     (long)floor((double)0.5 + y);   /* round */
#else      /* upper left 0,0 */
    (*arc_data)->second_endpoint_y = (long)floor((double)0.5 + y)   /* round */
				    + path->pth__specific.pth__arc.pth_center_y;
#endif

    x = (double)x_radius * cos(middle_angle);
    y = (double)(y_radius) * -sin(middle_angle);

    (*arc_data)->middle_endpoint_x = (long)floor((double)0.5 + x)   /* round */
				    + path->pth__specific.pth__arc.pth_center_x;

#ifdef OS2 /* lower left 0,0 */
    (*arc_data)->middle_endpoint_y = path->pth__specific.pth__arc.pth_center_y -
                                     (long)floor((double)0.5 + y);   /* round */
#else      /* upper left 0,0 */
    (*arc_data)->middle_endpoint_y = (long)floor((double)0.5 + y)   /* round */
				    + path->pth__specific.pth__arc.pth_center_y;
#endif

#ifdef MSWINDOWS
    /*  on ms-windows, arcs are always drawn counter clockwise; 
     *  if we have a negative extent arc, then we need to swap the first
     *  and last endpoints so the arc will be displayed correctly
     */
    if (path->pth__specific.pth__arc.pth_extent< 0)
      {
	CDAmeasurement tmpx, tmpy;
	tmpx = (*arc_data)->second_endpoint_x;
	tmpy = (*arc_data)->second_endpoint_y;
        (*arc_data)->second_endpoint_x = (*arc_data)->first_endpoint_x;
        (*arc_data)->second_endpoint_y = (*arc_data)->first_endpoint_y;
        (*arc_data)->first_endpoint_x = tmpx;
        (*arc_data)->first_endpoint_y = tmpy;
      }

#endif

    return(status);
}


/*
**++
**  FUNCTIONAL DESCRIPTION: dvr_record_alloc_path
**
**      Record the pointer to the allocated path in the viewer context structure
**	so that it can be deallocated later. by PBD
**
**  FORMAL PARAMETERS:
**
**      vw : viewer context pointer
**	path : pth structure pointer
**
**  IMPLICIT INPUTS:
**
**	allocated_pth_structs, alloc_pth_struct_array_size, and
**	alloc_pth_struct_count fields of the context structure (vw)
**
**  IMPLICIT OUTPUTS:
**
**      above fields of context structure modified
**
**  COMPLETION CODES:
**
**      DVR_NORMAL (success) or DVR_MEMALLOCFAIL or DVR_MEMDEALLOCFAIL
**
**  SIDE EFFECTS:
**
**      allocated_pth_structs array may be reallocated larger
**
**--
**/

CDAstatus	dvr_record_alloc_path(vw, path)
DvrViewerWidget	vw;
PTH		path;
{
    PTH	*list_place;
    CDAstatus		status = DVR_NORMAL;
    CDAsize		byte_size, byte_increment, counter = 0;
    CDAsize		list_size = 100;

    list_place = vw->dvr_viewer.Dvr.allocated_pth_structs;
    if (list_place == NULL)
	{
	    byte_size = 0;
	    byte_increment = list_size * sizeof(PTH *);
	    status = realloc_memory((void **) &list_place, &byte_size, byte_increment);
	    if (!DVRSuccess(status))
		return(status);

	    vw->dvr_viewer.Dvr.allocated_pth_structs = list_place;
	    list_size = byte_size / sizeof(PTH *);
	    vw->dvr_viewer.Dvr.alloc_pth_struct_array_size = list_size;
	}

    vw->dvr_viewer.Dvr.alloc_pth_struct_count++;

    if (vw->dvr_viewer.Dvr.alloc_pth_struct_count >=
		(unsigned long)	vw->dvr_viewer.Dvr.alloc_pth_struct_array_size)
	{
	    list_place == vw->dvr_viewer.Dvr.allocated_pth_structs;

	    byte_size =
		      vw->dvr_viewer.Dvr.alloc_pth_struct_count * sizeof(PTH *);
	    byte_increment = byte_size / 5; /* add 20% */
	    status = realloc_memory((void **) &list_place, &byte_size, byte_increment);
	    if (!DVRSuccess(status))
		return(status);

	    vw->dvr_viewer.Dvr.allocated_pth_structs = list_place;
	    list_size = byte_size / sizeof(PTH *);
	    vw->dvr_viewer.Dvr.alloc_pth_struct_array_size = list_size;
	}

    list_place += vw->dvr_viewer.Dvr.alloc_pth_struct_count - 1;

    *list_place = path;

    return(status);
}



/*
 * dvr_change_y_orient()
 *	utility routine to change a Y coordinate value from
 *	a lower left (0,0) based coordinate system to a upper
 *	left (0,0) based coordinate system
 */

CDAmeasurement dvr_change_y_orient(page_height, old_y)
    CDAmeasurement page_height;
    CDAmeasurement old_y;

{
    return
      (
#ifndef OS2
	/*  for OS2, do not change coordinate, PM has the same
	 *  lower left based (0,0) that is used by the engine
	 */
	page_height -
#endif
	old_y
      );
}



#ifdef MSWINDOWS

/*
 * dvr_adjust_print_coords()
 *	utility routine to adjust a coordinate pair to account for
 *	any offset that might be present in a printer; we need to 
 *	call this routine on MSWindows whereever we call dvr_change_y_orient
 *	to account for the printer offset. This is only necessary when
 *	printing; No work is required if we're displaying to the screen.
 *
 *      Originally, we were just calling SetViewportOrg() to set the
 * 	origin; that worked fine for PS printers, but seemed to have no
 *	effect when printing to HP (PCL) printers. If all of the Windows
 *      Print Drivers were corrected to support SetViewportOrg(), then
 *	we could switch back to that.
 */

void dvr_adjust_print_coords(vw, x, y)
    DvrViewerWidget vw;
    CDAmeasurement  *x;
    CDAmeasurement  *y;

{
  /*  if there is no work window to display to, then we must be printing;
   *  need to account for printer offset
   */	
  if (Work(vw) == 0)
    {
      *x -= (CDAmeasurement) vw->dvr_viewer.print_origin.x;
      *y -= (CDAmeasurement) vw->dvr_viewer.print_origin.y;
    }		

}

#endif



#ifdef DEBUG_DUMP

/*
 *  UTILITY ROUTINES (DEBUG only)
 */


/* Routine dump_page dumps the entire page structure, including all prototype
   pages, page frames, and objects on those pages */

void dump_page (
    vw,
    page,
    centipoint_flag )

DvrViewerWidget		vw;    		/* Viewer widget context pointer */
PAG			page;		/* Primary page */
unsigned char		centipoint_flag;/* True to use centipoints, else pixels */

{
    /* Local variables */
    PAG_PRIVATE		page_private;	/* Page private structure */
    FILE		*fp;		/* Local file pointer */
    char		filename[64];	/* Filename */
    char		temp_str[32];	/* Temp string */
    int			i, n;		/* Counter; count */
    CDAmeasurement	page_height;	/* Width of Page */
    CDAmeasurement	page_width;	/* Height of Page */


    /*
     *  Information may be dumped in PIXELS or CENTIPOINTS, determined at
     *  compile and run time.
     */

    if ( centipoint_flag == 1 ) {
	/* Dump lower left centipoint values; that is, convert upper left
	   pixel values to their corresponding lower left centipoint values */
	strcpy ( filename, "PAGE_DUMP.CP" );
    }
    else {
	/* Dump pixel values */
	strcpy ( filename, "PAGE_DUMP.PIX" );
    }


    /*
     *	Open file, traverse pages calling dump_elements for each page
     *	frame, then close file upon return.
     */

    fp = fopen ( filename, "w" );
    if ( fp == NULL ) return;

    fprintf ( fp, "\n  DDIF Page Structure Dump" );
    fprintf ( fp, "\n  ------------------------\n\n" );

    if ( centipoint_flag == 1 ) {
	/* Dump centipoints */

        fprintf ( fp, "  All values are expressed in CENTIPOINT units, with" );
        fprintf ( fp, " bounding box values\n" );
        fprintf ( fp, "  representing LOWER LEFT and UPPER RIGHT X,Y" );
        fprintf ( fp, " coordinate positions.  Other\n" );
        fprintf ( fp, "  information supplied includes the X,Y object" );
        fprintf ( fp, " position coordinates and the\n" );
        fprintf ( fp, "  indication of original (O) and/or real (R) path" );
        fprintf ( fp, " definitions and of the\n" );
	fprintf ( fp, "  existence of object private information.\n\n" );

	fprintf ( fp, "  [Note: Object values are actually stored as pixel" );
	fprintf ( fp, " values, but are converted\n" );
	fprintf ( fp, "  here to their equivalent centipoint values, and" );
	fprintf ( fp, " adjusted from upper left\n" );
	fprintf ( fp, "  to lower left based coordinates.  Precision will" );
	fprintf ( fp, " be lost due to conversion\n" );
	fprintf ( fp, "  from centipoints to pixels, and then back to" );
	fprintf ( fp, " centipoint approximations.]\n\n" );
    }
    else {
	/* Dump pixels */

        fprintf ( fp, "  All values are expressed in PIXEL units, with" );
        fprintf ( fp, " bounding box values\n" );
        fprintf ( fp, "  representing UPPER LEFT and LOWER RIGHT X,Y" );
        fprintf ( fp, " coordinate positions.  Other\n" );
        fprintf ( fp, "  information supplied includes the X,Y object" );
        fprintf ( fp, " position coordinates and the\n" );
	fprintf ( fp, "  indication of original (O) and/or real (R) path" );
        fprintf ( fp, " definitions and of the\n" );
	fprintf ( fp, "  existence of object private information.\n\n" );
    }

    page_private = (PAG_PRIVATE) page->pag_user_private;
    
    if ( page_private == NULL)
       return;

    if ( centipoint_flag == 1 ) 
       {
	/* Dump centipoints */
	page_height = CONVERT_PIXEL_TO_CP (page_private->height,
						    vw->dvr_viewer.y_dpi);
	page_width = CONVERT_PIXEL_TO_CP (page_private->width,
						   vw->dvr_viewer.x_dpi);

    }
    /* Else - dump pixels (default storage) */
    else
       {
       page_height = page_private->height;
       page_width  = page_private->width;
       }

    /* Calculate start to print */
    n = 53;	/* Init */
    sprintf ( temp_str, "%d", page->pag_page_number );
    n += strlen ( temp_str );
    sprintf ( temp_str, "%d", page_height );
    n += strlen ( temp_str );
    sprintf ( temp_str, "%d", page_width );
    n += strlen ( temp_str );

    /* Print out stars */
    fprintf ( fp, "  " );
    for ( i = 0; i < n; ++i ) {
	fprintf ( fp, "*" );
    }
    fprintf ( fp, "\n" );

    fprintf ( fp, "  ** Primary page number: %d  \
[Page Height: %d, Width: %d] **\n",
	page->pag_page_number, page_height, page_width );

    /* Print out stars */
    fprintf ( fp, "  " );
    for ( i = 0; i < n; ++i ) {
	fprintf ( fp, "*" );
    }
    fprintf ( fp, "\n" );


    /* Call dump_page_element routine for page */
    dump_page_element ( vw, page, 1, fp, centipoint_flag );

    /* End of page dump */
    fprintf ( fp, "  End of page dump\n" );

    /* Close file */
    (void) fclose ( fp );
}


/* Routine dump_page_element dumps out a page, and calls itself recursively
   with any prototype pages found.  [Note: Page frames are treated always
   as at level 0, whether prototype or primary page frames, with all their
   object elements beginning at level 1.] */

void dump_page_element (
    vw,
    page,
    primary_page_flag,
    fp,
    centipoint_flag )

DvrViewerWidget vw;    		/* Viewer widget context pointer */
PAG page;			/* Page to dump */
unsigned char primary_page_flag;		/* True if primary page */
FILE *fp;			/* File pointer */
unsigned char centipoint_flag;		/* True to use centipoints, else pixels */

{

    /* Local variables */
    PAG proto_page;		/* Prototype page */
    OBJ object;			/* Object structure */
    int level;			/* Object tree level */

    /*
     *	Call routine recursively if prototype page found.
     */

    if ( proto_page = page->pag_proto_page ) {
	/* Prototype page found; call routine recursively until page
           to process no longer has a prototype page reference */
	dump_page_element ( vw, proto_page, 0, fp, centipoint_flag );
    }


    /*
     *	Call display elements to dump page frame objects, of both the
     *	prototype and primary page frames.
     */

    /* Print message if primary page */
    if ( primary_page_flag == 1 ) {
	/* Primary page found */
	fprintf ( fp, "\n  ** Primary page **\n\n" );
    }
    else {
	/* Processing prototype page */
	fprintf ( fp, "\n  ** Prototype page **\n\n" );
    }

    /* Process page frames, print it out before calling dump_elements for
       its children */

    if ( object = (OBJ) page->pag_proto_frame ) {
	/* Prototype page frame found */
	fprintf ( fp, "  Start prototype page frame\n" );

	/* Print out root info first */
	print_element_info ( vw, object, 0, fp, centipoint_flag );

	dump_elements ( vw, object, 1, fp, centipoint_flag );
	fprintf ( fp, "  End prototype page frame\n\n" );
    }

    if ( object = (OBJ) page->pag_page_frame ) {
	/* Primary page frame found */
	fprintf ( fp, "  Start primary page frame\n" );

	/* Print out root info first */
	print_element_info ( vw, object, 0, fp, centipoint_flag );

	dump_elements ( vw, object, 1, fp, centipoint_flag );
	fprintf ( fp, "  End primary page frame\n\n" );
    }
}


/* Routine dump_elements is called recursively to traverse the tree in order
   to print out the relevant object information.  It is called with the root
   object to traverse, the tree level for indenting and level tracking, and
   the file pointer of the file to which to send output */

void dump_elements (
    vw,
    root_object,
    level,
    fp,
    centipoint_flag )

DvrViewerWidget vw;    		/* Viewer widget context pointer */
OBJ root_object;		/* Root of object tree to traverse */
long level;			/* Object tree level */
FILE *fp;			/* File pointer */
unsigned char centipoint_flag;		/* True to use centipoints, else pixels */

{
    /* Local variables */
    OBJ object;			/* Object structure */
    int next_level;		/* Next tree level */


    /*
     *	Traverse tree by calling routine recursively, printing out the
     *	relevant object information at each level;  perform a breadth-
     *	first search, visiting the current element before its children.
     */

    FOREACH_INLIST ( root_object->obj_queue, object, OBJ ) {

	/* Loop processing each object in queue */
	print_element_info ( vw, object, level, fp, centipoint_flag );

	if ( ! QUE_EMPTY (object->obj_queue) ) {

	    /* Child elements found */
	    next_level = level + 1;
	    dump_elements ( vw, object, next_level, fp, centipoint_flag );
	}
    }
}


/* Routine print_element_info prints out the relevant information within
   the element specified, including level marking and indentation based
   on tree level.  Information is sent out to the file corresponding to
   the specified file pointer */

void print_element_info (
    vw,
    object,
    level,
    fp,
    centipoint_flag )

DvrViewerWidget vw;    		/* Viewer widget context pointer */
OBJ object;			/* Specified object */
long level;			/* Object tree level */
FILE *fp;			/* File pointer */
unsigned char centipoint_flag;		/* True to use centipoints, else pixels */

{
    /* Local variables */
    OBJ_PRIVATE object_private;		/* Object private structure */
    int page_height;			/* Working page height */
    int i;				/* Local loop counter */


    /*
     *	Print out object info.
     */

    page_height = vw->dvr_viewer.Dvr.working_page_height;

    /* Print out level number and loop with indents */
    fprintf ( fp, "%2d  ", level );    /* Print in four columns */

    for ( i = 0; i < level; ++i )
	fprintf ( fp, "  " );	    /* Skip more space for each level */


    /* Print out relevant information */
    /* fprintf ( fp, "Type: " ); */
    switch ( object->obj_type ) {
    case obj_frm:  fprintf( fp, "FRM" );  break;
    case obj_trn:  fprintf( fp, "TRN" );  break;
    case obj_gly:  fprintf( fp, "GLY" );  break;
    case obj_txt:  fprintf( fp, "TXT" );  break;
    case obj_frg:  fprintf( fp, "FRG" );  break;
    case obj_tab:  fprintf( fp, "TAB" );  break;
    case obj_lin:  fprintf( fp, "LIN" );  break;
    case obj_crv:  fprintf( fp, "CRV" );  break;
    case obj_arc:  fprintf( fp, "ARC" );  break;
    case obj_fil:  fprintf( fp, "FIL" );  break;
    case obj_img:  fprintf( fp, "IMG" );  break;
/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    case obj_aud:  fprintf( fp, "AUD" );  break;
#endif
/*END AUDIO STUFF*/
    default:  fprintf( fp, "%3d", object->obj_type );
    }
    fprintf ( fp, " " );


    /* Get object private info */
    object_private = (OBJ_PRIVATE) object->obj_user_private;

    /* Insert a Check to see if all the objects have a OBJ_PRIVATE */
    if (object_private == NULL)
       return;

    /* Print out X,Y object positions */
    if ( centipoint_flag == 1 ) {
	/* Dump centipoints */
	fprintf ( fp, "(%d,%d), ",
	    CONVERT_PIXEL_TO_CP (object_private->x_pos, vw->dvr_viewer.x_dpi),
	    page_height -
	    CONVERT_PIXEL_TO_CP (object_private->y_pos, vw->dvr_viewer.y_dpi));
    }
    else {
	/* Dump pixels */
	fprintf ( fp, "(%d,%d), ",
	    object_private->x_pos, object_private->y_pos );
    }

    /* Print out bounding box coordinates */
    fprintf ( fp, "BBox" );
    if ( OBJECT_BBOX_DEFINED (object_private)) {
	/* Bounding box specified */

	if ( centipoint_flag == 1 ) {
	    /* Dump centipoints */
	    fprintf ( fp, ": (%d,%d),(%d,%d)",
		CONVERT_PIXEL_TO_CP (object_private->bbox_ul_x,
				     vw->dvr_viewer.x_dpi),
		page_height - CONVERT_PIXEL_TO_CP (object_private->bbox_ul_y,
				 		   vw->dvr_viewer.y_dpi),
		CONVERT_PIXEL_TO_CP (object_private->bbox_lr_x,
				     vw->dvr_viewer.x_dpi),
		page_height - CONVERT_PIXEL_TO_CP (object_private->bbox_lr_y,
						   vw->dvr_viewer.y_dpi));
	}
	else {
	    /* Dump pixels */
	    fprintf ( fp, ": (%d,%d),(%d,%d)",
		object_private->bbox_ul_x, object_private->bbox_ul_y,
		object_private->bbox_lr_x, object_private->bbox_lr_y );
	}
    }
    else {
	/* No bbox specified */
	fprintf ( fp, " Undefined" );
    }

    /* Print bbox information delimeter following information if either
       path or private info exists */
    if (( object_private->orig_path != NULL ) ||
	( object_private->real_path != NULL ) ||
	( object_private->private_info != NULL )) {

	/* Paths or private info exist; output delimeter */
	fprintf ( fp, ", " );
    }

    /* Print out whether or not paths are defined */
    if (( object_private->orig_path == NULL ) &&
	( object_private->real_path == NULL )) {

	/* No paths defined */
	;   /* Output nothing */
    }
    else {
	/* One or both paths defined */
	fprintf ( fp, "Paths: " );
	if ( object_private->orig_path != NULL )
	    fprintf ( fp, "O" );
	if (( object_private->orig_path != NULL ) &&
	    ( object_private->real_path != NULL )) {
	    /* Both paths found; separate them */
	    fprintf ( fp, "/" );
	}
	if ( object_private->real_path != NULL )
	    fprintf ( fp, "R" );
    }

    /* Print path and private info delimeter if both exist */
    if ((( object_private->orig_path != NULL ) ||
	( object_private->real_path != NULL )) &&
	( object_private->private_info != NULL )) {

	/* Path and private info both exist; output delimeter */
	fprintf ( fp, ", " );
    }

    /* Print out whether or not object private info is defined */
    if ( object_private->private_info == NULL ) {
	/* No private info */
	;   /* Output nothing */
    }
    else {
	/* Private info reference exists */
	fprintf ( fp, "PInfo" );
    }

    fprintf ( fp, "\n" );
}
#endif	    /* End DEBUG only routines */
