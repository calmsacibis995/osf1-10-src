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
#define Module DVR_GRAX
#define Ident  "V02-069"

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
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**	This module contains all of the Viewer work routines for displaying
**	graphics.  These work routines are mostly called from the routines
**	in DVR_DISPLAY_GRAPHICS.C.
**
**	NOTE: This module should contain all device dependent (X) calls
**	      for the DVR facility not present in DVR_MAIN.C (X application)
**	      or DVR_CREATE.C/DVR_ACTIONS.C (X widget modules); these are
**	      the only four routines which should contain X calls.
**
**
** ENVIORNMENT:
**	vms decwindows, ultrix uws
**
** AUTHORS:
**	Kathy Robinson,  15-Dec-1988
**
**
** MODIFIED BY:
**
**	V02-001		PBD0001		Peter B. Derr		1-Mar-1989
**		Use new flattened include files
**			DAM0002		Dennis McEvoy		12-may-89
**		add cast to fix ultrix warning
**			DAM0003		Dennis McEvoy		16-may-89
**		correct calculation of pie arc's y endpoints
**
**	V02-002		DAM0002		Dennis McEvoy		31-may-89
**		correct foreground/background in standard
**		patterns on color displays
**
**	V02-003		PBD0003		Peter Derr		13-Jun-89
**		Don't close arc outline unless pie arc or close arc flag is set.
**		(in dvr_stroke_path)
**
**	V02-004		DAM004		Dennis McEvoy		12-Jul-89
**		use new ISL parameters
**
**	V02-005		KMR004		Kathy Robinson		8-Aug-89
**		only use ISL when we have to.  Faster through X.
**
**	V02-005		SJM0001		Stephen Munyan		18-Dec-1989
**		Change a static to a define to reduce the number of non-sharable
**		code psects.
**
**	V02-006		DAM006		McEvoy			5-mar-90
**		changes for OS/2 port
**
**	V02-006		KMR		Kathy Robinson		26-Mar-90
**		Added marker drawing support
**
**	V02-007		SJM		Stephen Munyan		4-Apr-90
**		Removed extern reference to XSetFont since it was
**		breaking the DECWIN build.
**
**	V02-008		DAM		Dennis McEvoy		12-Jul-90
**		merge in micky's changes for new callbacks
**	                MB0001		Micky Balladelli	25-Jun-1990
**		Add events callbacks to Image widgets:
**		events are : Mouse/Buttons/Expose
**
**	V02-009		DAM		Dennis McEvoy		16-jul-90
**		use y dpi for non 1:1 pixel displays
**
**	V02-010		SJM		Stephen J. Munyan	30-Jul-1990
**		Conversion to Motif
**
**	V02-011		BFB		Barbara Bazemore	21-Aug-1990
**		Merge in Kenji Yamada-san's image scaling support changes.
**
**	V02-012		BFB		Barbara Bazemore	30-Aug-1990
**		Call IDS routine that has MOTIF support.  Old one uses XUI.
**
**	V02-013		SJM		Stephen J. Munyan	25-Oct-1990
**              Fixed font id query code not to reference a pointer before
**              checking to see if the pointer was null.
**
**	V02-014		SJM0000		Stephen Munyan		2-Nov-1990
**		Change from XtAddActions to XtAppAddActions
**
**	V02-015		BFB		Barbara Bazemore	9-Nov-1990
**		Change from IDS widgets to IDS bitmap rendering for images
**
**	V02-016		DAM		Dennis McEvoy	 	21-Nov-1990
**		Fix wait cursor fallback
**
**	V02-017		SJM		Stephen Munyan		28-Nov-1990
**		Incorporate changes from the XUI stream.
**
**          V02-013     JJT             Jeff Tancill            27-nov-90
**              Fix call to dvr_stroke_path, delete extra argument.
**
**	V02-018		DAM		Dennis McEvoy		28-Nov-1990
**		Fix protos
**
**	V02-019		DAM		Dennis McEvoy		29-Nov-1990
**		Application_callback should take void * param
**
**	V02-020		SJM		Stephen Munyan		6-Dec-1990
**		Incorporate changes from the XUI stream.
**
**	    V02-016	    SJM		    Stephen Munyan	6-Dec-1990
**		 Fix ACCVIO in bezier code if a null path is
**		    specified.
**
**	V02-021		BFB		Barbara Bazemore	7-Dec-1990
**		Memory leak - delete IDS presentation surface when done
**
**	V02-022		KMRK		Kathy Robinson		9-Jan-1991
**		Use agreed upon definition of pie-arc flags - finally!
**
**	V02-023		SJM		Stephen Munyan		19-Feb-1991
**		Moved IdsDeletePresentSurface to DVR_IMAGE.C page cleanup
**		since we don't delete the rendering until that point.
**		The reason for this is that if we delete the presentation
**		surface first Image Services may corrupt their internal
**		data structures.
**
**	V02-024		BFB		Barbara Bazemore	20-Feb-1991
**		Use graphics context returned by Image Services for XPutImage,
**		this will cause reverse polarity to be handled correctly.
**
**	V02-025		KMRK		Kathy Robinson		23-Feb-1991
**		Use our own graphics context, but reverse the fore and back
**		values if the image polarity is negative.
**
**	V02-026		RTG		Dick Gumbel		05-Mar-1991
**		Cleanup #include's
**
**	V02-027		DAM		Dennis McEvoy		06-Mar-1991
**		Fix set_clip bug
**
**	V02-028		RKN		Ram Kumar Nori		12-Mar-1991
**		Fix Marker Bug
**
**	V02-029		CJR		Chris Ralto		03-Apr-1991
**		Add Viewer support for DDIF Line Draw Patterns.
**
**	V02-030		DAM		Dennis McEvoy		04-apr-1991
**		cleanup typedefs
**
**	V02-031		CJR		Chris Ralto		05-Apr-1991
**		Fix handling of superfluous line draw patterns.
**
**	V02-032		CJR		Chris Ralto		08-Apr-1991
**		Fix problem whereby an X Server error results from using
**		a small line pattern size for dash and/or dot line styles.
**
**      V02-033         RKN             Ram Kumar Nori          12-Apr-1991
**              Fix Marker display bug when scrolling.
**
**      V02-034         SJM             Stephen Munyan          17-Apr-1991
**              Fix markers such that they get drawn after the fill and
**              the outline to make sure that they don't get occluded.
**
**      V02-035         RKN             Ram Kumar Nori          20-Apr-1991
**              Added Code To Draw Arrow Heads.
**
**      V02-035         RKN             Ram Kumar Nori          23-Apr-1991
**              Added DVS_ARR.H include file.
**
**	V02-036		SJM		Stephen Munyan		26-Apr-1991
**		Fix roundoff error in allocate color routine
**		such that colors are allocated correctly on 24 plane
**		systems.
**
**      V02-037		RKN             Ram Kumar Nori          27-Apr-1991
**		Added a routine to rotate ellipses and elliptical
**		arcs.
**              Added a Call in dvr_display_arc_object to draw
**		arrow heads on arcs.
**
**	V02-038		DAM		Dennis McEvoy		13-May-1991
**		fix ultrix warnings
**
**	V02-039		DAM		Dennis McEvoy		15-May-1991
**		switch to ImgDef.h
**
**	V02-040		DAM		Dennis McEvoy		15-May-1991
**		disallow arrows on closed polylines
**
**	V02-041		DAM		Dennis McEvoy		17-May-1991
**		fix accvio in delete_page_colors
**
**	V02-042		DAM		Dennis McEvoy		20-May-1991
**		allow arrows on smoothed/rounded lines
**
**      V02-043		RKN             Ram Kumar Nori          29-May-1991
**		Change the direction of rotation of elliptical arcs.
**
**      V02-044		RKN             Ram Kumar Nori          29-May-1991
**		Rotate the radial lines of the elliptical arcs.
**
**	V02-045		DAM		Dennis McEvoy		07-Jun-1991
**		use setjmp for chf on unix
**
**	V02-046		SJM		Stephen Munyan		27-Jun-1991
**		DEC C Cleanups
**
**	V02-047		DAM		Dennis McEvoy		05-Aug-1991
**		renamed header files, removed dollar signs
**
**	V02-048		DAM		Dennis McEvoy		12-Aug-1991
**		rename setjmp vars
**
**	V02-049		SJM		Stephen Munyan		21-Aug-1991
**		Change to use ids_alloc_colors.h such that we work
**		for the Open Systems stuff.
**
**	V02-050		DAM		Dennis McEvoy		04-Sep-1991
**		remove obsolete #ifdefs
**
**	V02-051		DAM		Dennis McEvoy		11-Sep-1991
**		ifdef line draw pattern for big endien machines
**
**	V02-052		RKN		Ram Kumar Nori		17-Sep-1991
**		Fixed Arrow Head Bugs On Filled Arcs and Filled Rounded
**		Polylines With Markers.
**
**	V02-053		DAM		Dennis McEvoy		03-Oct-1991
**		cleanup X calls to match protos
**
**	V02-054		RKN		Ram Kumar Nori		04-Oct-1991
**		Modified dvr_output_markers Routine To Use XSegment Strucutre
**		Instead Of XPoint Structure
**
**	V02-055		DAM		Dennis McEvoy		07-Oct-1991
**		cleanup #params to XDrawSegments
**
**	V02-056		RKN		Ram Nori		18-Oct-1991
**		Moved dvr_rotated_ellipse routine to dvr_dgra module.
**		Added pth_rotated to case staements in dvr_stroke path
**		and dvr_fill_path routines.
**
**	V02-057		RKN		Ram Kumar Nori		29-Oct-1991
**		Changed the head1 & head2 type to PTH in dvr_drawarrowheads
**
**	V02-058		RKN		Ram Kumar Nori		20-Nov-1991
**		Remove arrow head support on closed polylines.
**
**	V02-059		SJM000		Stephen Munyan		 9-Jan-1992
**		Merge in changes from DEC Japan
**
**	V02-060		DAM000		Dennis McEvoy		26-mar-1992
**		use portable das bits for sun
**
**	V02-061		DAM000		Dennis McEvoy		12-apr-1992
**		fix nonstandard line drawing on big endian machines
**
**	V02-062		RKN000		Ram Kumar Nori		02-Jun-1992
**		Added a routine dvr_reset_gc, to reset the background,
**		foreground and the fill.
**
**	V02-063		DAM000		Dennis McEvoy		02-jun-1992
**		type cleanups for alpha/osf1
**
**	V02-064		KLM001		Kevin McBride		12-jun-1992
**		ALPHA OSF/1 port
**
**	V02-065		DAM001		Dennis McEvoy		17-jun-1992
**		ids cleanups for alpha
**
**	V02-066		KLM002		Kevin McBride		22-jun-1992
**		Cleanup for prototype change of dvr_draw_line
**
**	V02-067		RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>; and the
**		converse for non-same
**
**	V02-068		JJT0000		Jeff Tancill		13-Oct-1992
**		Use CDAlocal_DAS symbol to determine which DAS to use.
**      V02-069         RJD000          Ronan Duke		30-Aug-1992
**              Include Xm/MAnagerP.h if linking against Motif V1.2
**
**--
*/

#include <cdatrans.h>

#ifdef __vms__

#include <dvrint.h>				/* DVR internal definitions */

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */

#include <X11/cursorfont.h>
#include <decw$cursor.h>

#include <Xm/Xm.h>				/* Motif definitions */
/* RD:
 * if linking against V1.2 of Motif then need to explicitly include 
 * Xm/ManagerP.h to get XmManagerPart definition
 */
#ifdef MOTIF_V12
#include <Xm/ManagerP.h>	
#endif

#include <Mrm/MrmAppl.h>			/* Motif Resource Manager definitions */

#pragma standard				/* turn /stand=port back on */


#include <chfdef.h>                             /* condition handler structs */
#include <descrip.h>				/* __vms__ descriptor defs  */
#include <ImgDef.h>				/* DECimage ISL defs	*/


#include <ids_alloc_colors.h>			/* constants for ids color */

#else	/* UNIX - and for all others, this is the best default */

#include <X11/cursorfont.h>
#include <X11/decwcursor.h>

#include <Xm/Xm.h>				/* Motif definitions */
#include <Xm/ManagerP.h>			/* rd: 23/7: need XmManagerPart def  */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#ifdef CDAlocal_DAS
#include <chfdef.h>
#else
#include <ChfDef.h>                             /* condition handler structs */
#endif

#include <dvrint.h>				/* DVR internal definitions */

#ifdef CDAlocal_DAS
#include <imgconst.h>
#include <imgtyp.h>
#include <imgptp.h>
#include <idscolor.h>
#else
#include <ImgDef.h>
#include <ids_alloc_colors.h>			/* constants for ids color */
#endif

#include <setjmp.h>

extern jmp_buf   dvr_env;
extern CDAstatus dvr_chf_status;

#endif

#ifdef __vms__
#pragma nostandard				/* /stand=port problem in
						   IdsImage.h */
#endif

#ifdef CDAlocal_DAS
#include <idsconst.h>
#include <idsptp.h>
#else
#include <IdsImage.h>
#endif

#ifdef __vms__
#pragma standard
#endif

#include <dvsarr.h>				/* DVS_ARR definitions */

#include "dvrwfill.h"

#include "dvrwdef.h"				/* Public Defns w/dvr_include */
#include "dvrwint.h"				/* Viewer Constants and structures  */



/*
 * The following PROTO definition was added as a workaround until
 * DEC C properly handles LIB$ESTABLISH in a future version of the
 * compiler.
*/

#ifdef __vms__

PROTO(void * LIB$ESTABLISH, ());

#endif


#define CENTI	0.01
#define INVSQRT2  0.7071

PROTO (void dvr_drawarrowheads,
	    (DvrViewerWidget,
	     GAT,
	     int,
	     PTH,
	     PTH,
	     int) );


/*****************************************************************************
 * dvr_output_markers (vw, path, mark_type, size)
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *  Given a pattern structure such as the layout engine produces, set the
 *  serve's GC to an approriate state and update the widget's gcv.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw	RW	Viewer widget pointer.  Used to get parameters for
 *			X calls, and to pass to dvr_error_callback.
 *
 *	path	R	This structure contains the actual path
 * 			which is used for the drawing.  This path given in
 *			a PIXEL coord system, and is always a line.
 *
 *	mark_t	R	Shape of marker to be drawn.
 *
 *	size	R	Size of marker to be drawn.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *      none
 *
 *  IMPLICIT OUTPUTS:
 *
 *      Markers drawm on screen
 *
 *  FUNCTION VALUE:
 *
 *      DVR_NORMAL if no errors signaled
 *	DVR_GRAPHICFAIL if any errors occured
 *
 *  SIDE EFFECTS:
 *
 *      Signals any errors that occur during processing.
 *
 ******************************************************************************/
unsigned long
dvr_output_markers(vw, path, mark_type, size)
    DvrViewerWidget	vw;
    PTH 		path;
    longword		mark_type;
    signlong		size;
{
Window		win;
Display		* dpy;
GC		gc;
XGCValues	gcv;		/* shadows what's in gc */
DvrStructPtr	dvr_ptr = &(vw->dvr_viewer.Dvr);
XPoint		* xpoints;	/* parameter to XDrawLine */
XSegment	* xsegs;	/* parameter to XDrawLine */
signlong	* lpoints;	/* pointer to layout engine's list of points */
int		i, count;	/* always useful to have around */
int		Xtop, Ytop;     /* Difference between origin of paper & of display */
signlong	x_factor, y_factor;
signlong	x_size, y_size;
signlong	x_coord, y_coord; /* Top Left Corner of a bounding rectangle */

gcv = dvr_ptr->gcv;
win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gc =  dvr_ptr->GcId;

Xtop = dvr_ptr->Xtop;
Ytop = dvr_ptr->Ytop;

switch (mark_type)
    {
    case DDIF_K_PLUS_MARKER:
    default:
	x_size  =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.x_dpi);
	y_size  =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.y_dpi);
	count   =  2 * path->pth__specific.pth__xy.pth_pairs;
	xsegs   =  (XSegment *)malloc(sizeof(XSegment)*(count));
	lpoints =  &path->pth__specific.pth__xy.pth_first_x;
	for (i=0; i < path->pth__specific.pth__xy.pth_pairs; i++)
		{
		xsegs[2*i].x1   = *lpoints - x_size - Xtop;
		xsegs[2*i].x2   = *lpoints + x_size - Xtop;
		xsegs[2*i+1].x1 = *lpoints - Xtop;
		xsegs[2*i+1].x2 = *lpoints - Xtop;
		lpoints++;
		xsegs[2*i].y1   = *lpoints - Ytop;
		xsegs[2*i].y2   = *lpoints - Ytop;
		xsegs[2*i+1].y1 = *lpoints - y_size - Ytop;
		xsegs[2*i+1].y2 = *lpoints + y_size - Ytop;
		lpoints++;
		}
	XDrawSegments( dpy, win, gc, xsegs, count);
	free(xsegs);
	break;

    case DDIF_K_ASTERISK_MARKER:

	x_factor =  CONVERT_CP_TO_PIXEL ((size * INVSQRT2), vw->dvr_viewer.x_dpi);
	y_factor =  CONVERT_CP_TO_PIXEL ((size * INVSQRT2), vw->dvr_viewer.y_dpi);
	x_size   =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.x_dpi);
	y_size   =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.y_dpi);
	count    =  4 * path->pth__specific.pth__xy.pth_pairs;
	xsegs    =  (XSegment *)malloc(sizeof(XSegment)*(count));
	lpoints  =  & path->pth__specific.pth__xy.pth_first_x;
	for (i=0; i < path->pth__specific.pth__xy.pth_pairs; i++)
		{
		xsegs[4*i].x1   = *lpoints - x_factor - Xtop;
		xsegs[4*i].x2   = *lpoints + x_factor - Xtop;
		xsegs[4*i+1].x1 = *lpoints - x_factor - Xtop;
		xsegs[4*i+1].x2 = *lpoints + x_factor - Xtop;
		xsegs[4*i+2].x1 = *lpoints - x_size - Xtop;
		xsegs[4*i+2].x2 = *lpoints + x_size - Xtop;
		xsegs[4*i+3].x1 = *lpoints - Xtop;
		xsegs[4*i+3].x2 = *lpoints - Xtop;
		lpoints++;
		xsegs[4*i].y1   = *lpoints - y_factor - Ytop;
		xsegs[4*i].y2   = *lpoints + y_factor - Ytop;
		xsegs[4*i+1].y1 = *lpoints + y_factor - Ytop;
		xsegs[4*i+1].y2 = *lpoints - y_factor - Ytop;
		xsegs[4*i+2].y1 = *lpoints - Ytop;
		xsegs[4*i+2].y2 = *lpoints - Ytop;
		xsegs[4*i+3].y1 = *lpoints - y_size - Ytop;
		xsegs[4*i+3].y2 = *lpoints + y_size - Ytop;
		lpoints++;

		}
	XDrawSegments( dpy, win, gc, xsegs, count);
	free(xsegs);
	break;

    case DDIF_K_CROSS_MARKER:
	x_factor =  CONVERT_CP_TO_PIXEL ((size * INVSQRT2), vw->dvr_viewer.x_dpi);
	y_factor =  CONVERT_CP_TO_PIXEL ((size * INVSQRT2), vw->dvr_viewer.y_dpi);
	x_size   =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.x_dpi);
	y_size   =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.y_dpi);
	count    =  2 * path->pth__specific.pth__xy.pth_pairs;
	xsegs    =  (XSegment *)malloc(sizeof(XSegment)*(count));
	lpoints  =  & path->pth__specific.pth__xy.pth_first_x;
	for (i=0; i < path->pth__specific.pth__xy.pth_pairs; i++)
		{
		xsegs[2*i].x1   = *lpoints - x_factor - Xtop;
		xsegs[2*i].x2   = *lpoints + x_factor - Xtop;
		xsegs[2*i+1].x1 = *lpoints - x_factor - Xtop;
		xsegs[2*i+1].x2 = *lpoints + x_factor - Xtop;
		lpoints++;
		xsegs[2*i].y1   = *lpoints - y_factor - Ytop;
		xsegs[2*i].y2   = *lpoints + y_factor - Ytop;
		xsegs[2*i+1].y1 = *lpoints + y_factor - Ytop;
		xsegs[2*i+1].y2 = *lpoints - y_factor - Ytop;
		lpoints++;
		}
	XDrawSegments( dpy, win, gc, xsegs, count);
	free(xsegs);
	break;

    case DDIF_K_CIRCLE_MARKER:
    case DDIF_K_DOT_MARKER:

	x_factor =  CONVERT_CP_TO_PIXEL ((size * INVSQRT2), vw->dvr_viewer.x_dpi);
	y_factor =  CONVERT_CP_TO_PIXEL ((size * INVSQRT2), vw->dvr_viewer.y_dpi);
	x_size   =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.x_dpi);
	y_size   =  CONVERT_CP_TO_PIXEL (size, vw->dvr_viewer.y_dpi);
	lpoints  = &path->pth__specific.pth__xy.pth_first_x;
	for (i=0; i <path->pth__specific.pth__xy.pth_pairs; i++)
		{
                x_coord = * lpoints++ - x_size/2 - Xtop;
                y_coord = * lpoints++ - y_size/2 - Ytop;
		if (mark_type == DDIF_K_DOT_MARKER)
		   XFillArc( dpy, win, gc, x_coord, y_coord, x_size, y_size,
				0, 360*64);
		else
		   XDrawArc( dpy, win, gc, x_coord, y_coord, x_size, y_size,
				0, 360*64);
		}
	break;
    }
}

/*****************************************************************************
 * dvr_output_pattern (vw, patt, flag)
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *  Given a pattern structure such as the layout engine produces, set the
 *  serve's GC to an approriate state and update the widget's gcv.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw	RW	Viewer widget pointer.  Used to get parameters for
 *			X calls, and to pass to dvr_error_callback.
 *
 *	patt	R  	The pattern stucture used to set graphics context.
 *
 *	flag	W	TRUE if pattern is visible,  FALSE if pattern is
 *			completely transparent.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *      none
 *
 *  IMPLICIT OUTPUTS:
 *
 *      Updated gc in server.  This sets the state for future X Drawing calls.
 *
 *  FUNCTION VALUE:
 *
 *      DVR_NORMAL if no errors signaled
 *	DVR_GRAPHICFAIL if any errors occured
 *
 *  SIDE EFFECTS:
 *
 *      Signals any errors that occur during processing.
 *
 ******************************************************************************/

/*  Given a pointer to a solid color pattern, returns an integer between
 *  0 and 1 which is the gray level of the color
 */
#define YIQ(patt) \
   ( (float) ( (patt)->pat__specific.pat__solid.pat_red * 0.30 * CENTI  +  \
      (patt)->pat__specific.pat__solid.pat_green * 0.59 * CENTI + \
      (patt)->pat__specific.pat__solid.pat_blue * 0.11 * CENTI) )


/*  Given a DDIF standard pattern index and fore and back ground pixels,
 *  sets the gc for that pattern in the fore and background colors specified
 *  and updates gcv to reflect the change.
 *
 *  N.B.:  All the DDIF patterns are specified in  dvr_decw_fill_patterns.h
 *  along with the array ddif_patterns, which contains all the pattern
 *  pointers.  This .h file is taken from DDIF_STD_PATTERNS_32.H in
 *  DDIF_PUBLIC:  and then the definition of ddif_patterns is added to the
 *  end.
 */


void
tile(index, dpy, win, gc, gcv, fore, back)
longword index;         /* Which DDIF standard pattern to use */
Display *dpy;		/* Standard X stuff */
Window win;		/* Standard X stuff */
GC gc;			/* Where the new X state is set */
XGCValues *gcv;		/* shadows what's in gc */
unsigned long fore, back;/*Fore and background pixel to use */
{
Pixmap pixmap; 	/* X pixmap with pattern in it */
int dfs;    	   	/* Standard X stuff */

dfs = XDefaultScreen(dpy);

pixmap =
XCreatePixmapFromBitmapData( dpy,win, (char *) ddif_patterns[index],
			     ddif_pattern_3_width, ddif_pattern_3_height,
			     fore, back, XDefaultDepth(dpy, dfs) );
if (pixmap)
	XSetTile(dpy, gc, pixmap);
else
	printf("No more pixmap memory");
/*XFreePixmap(pixmap);   */
gcv->fill_style = FillTiled;
XSetFillStyle(dpy, gc, FillTiled);  }


int alloc_color (vw, patt)
    DvrViewerWidget	vw;
    PAT			patt;
{
DvrStructPtr		dvr_ptr = &(vw->dvr_viewer.Dvr);
PAG_PRIVATE		page;
COLOR_INFO		node;
Display			*dpy;
Window			win;
XWindowAttributes	winattr;
int			screen;
int			cmap;
XColor			color;

    /*
     * See if this pattern has already been used
     * and if so, then we already know what pixel to use.
     */
    page = (PAG_PRIVATE)dvr_ptr->current_page->pag_user_private;
    FOREACH_INLIST (page->color_info_list, node, COLOR_INFO)
	{
	if ((node->color_info_red   == patt->pat__specific.pat__solid.pat_red)
	 && (node->color_info_green == patt->pat__specific.pat__solid.pat_green)
	 && (node->color_info_blue  == patt->pat__specific.pat__solid.pat_blue))
	    return node->color_info_pixel;
	}

    /*
     * New pattern, need to ascertain closest color
     * using the ids routines.
     */

    /* Really, the colormap should be found once per widget */
    dpy = XtDisplay (vw);
    win = XtWindow (vw->dvr_viewer.work_window);
    XGetWindowAttributes (dpy, win, &winattr);
    screen = (int) winattr.screen;
    cmap = winattr.colormap;

    /* Call IDS to get a color close to what was asked for */

    /*
     * Note that the pattern values are stored as values from 0-100
     * inclusive.  Some day we should increase these values since 100
     * values per color isn't really enough for some systems.
     *
     * The following algorithm multiplies the (0-100) value by 65535 then
     * divides the result by 100.  We must do it this way to prevent the
     * lower two digits from being truncated.  In the future I would recommend
     * converting these values back to floating point.
    */

    color.red   = (patt->pat__specific.pat__solid.pat_red   * 65535) / 100;
    color.green = (patt->pat__specific.pat__solid.pat_green * 65535) / 100;
    color.blue  = (patt->pat__specific.pat__solid.pat_blue  * 65535) / 100;

    if (!XAllocColor(dpy, cmap, &color))
	{
    	    IdsAllocColors (screen, cmap, &color, 1, Ids_RGBSpace, 0.0, 0.0 );
	}
    /*
     * add pattern/pixel info to master list.
     */
    node = (COLOR_INFO)XtMalloc (sizeof (struct color_info));
    node->color_info_pixel = color.pixel;
    node->color_info_red   = patt->pat__specific.pat__solid.pat_red;
    node->color_info_green = patt->pat__specific.pat__solid.pat_green;
    node->color_info_blue  = patt->pat__specific.pat__solid.pat_blue;
    QUE_INSERT (page->color_info_list, &node->color_info_queue);

    return node->color_info_pixel;
}

/*
 *  N.B.:  There is a clash between the X version of the world with its
 *  user-settable fore and background colors, and the explcit color setting
 *  in these patterns.  For now, solid_black and solid_white are translated to
 *  the user's fore and background, respectively.
 *  Standard patterns will also be rendered in fore and back.
 *  When a solid color is specified in RGB, true black, true white or the
 *  nearest patterned gray will be used.
 */
CDAstatus
dvr_output_pattern(vw, patt, flag)
DvrViewerWidget vw;
PAT 		patt;
boolean 	*flag;
{
DvrStructPtr 	dvr_ptr = &(vw->dvr_viewer.Dvr);
Display 	*dpy;		/* Standard X stuff */
Window 		win;  		/* Standard X stuff */
GC 		gc;				/* Where the new X state is set */
XGCValues 	gcv;		/* shadows what's in gc */
int 		dfs;       	/* Standard X stuff */
float 		gray; 		/* If RGB is given this is the closest gray (range 0 - 1.0)*/
int 		i, count;   	/* An extra int is alsways handy. */
int 		fore, back;
longword 	patt_index; 	/* The standard DDIF pattern closest to the gray wanted */



gcv = dvr_ptr->gcv;
win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gc =  dvr_ptr->GcId;
dfs = XDefaultScreen(dpy);

/* for temporary solution, let gcv.fore,back always stay same.
 * always force a reset.  We'll work a better strategy out later.
 */

gcv.foreground =  vw->manager.foreground;
gcv.background =  vw->core.background_pixel;
gcv.fill_style = FillSolid;

XSetFillStyle(dpy, gc, FillSolid);
XSetForeground (dpy, gc,  gcv.foreground );
XSetBackground (dpy, gc,  gcv.background );

/* Assume invisible until we know other wise */
*flag = FALSE;

if (patt == 0) /* transparent, no more work to be done */
	return (DVR_NORMAL);

switch (patt ->pat_type)
	{
	case (pat_transparent):    /* transparent, no more work to be done */
		{
		return (DVR_NORMAL);
		break;
		}
	case (pat_solid_black):
		{
		if (gcv.fill_style != FillSolid)
			{
			gcv.fill_style = FillSolid;
			XSetFillStyle(dpy, gc, FillSolid);
			}
		break;
		}
	case (pat_solid_white):
		{
		if (gcv.fill_style != FillSolid)
			{
			gcv.fill_style = FillSolid;
			XSetFillStyle(dpy, gc, FillSolid);
			}
		XSetForeground (dpy, gc,  gcv.background );
		break;
		}

	case (pat_solid):
		{
		if  (DisplayPlanes (dpy, dfs) == 1)
		    {
		    /*
		     * On a monochrome sytem, we will substitute a gray pattern
		     * for a solid color.  There are 16 levels of gray in DDIF,
		     * in addition, of course, to Black and White.
		     */
		    gray = YIQ(patt);
		    if ( (gray <= 1.0/17.0)  &&
				    (gcv.foreground != BlackPixel (dpy, dfs)) )
			  {  /* BLACK */
			  if (gcv.fill_style != FillSolid)
				{
				gcv.fill_style = FillSolid;
				XSetFillStyle(dpy, gc, FillSolid);
				}
		          gcv.foreground = BlackPixel (dpy,dfs);
			  XSetForeground (dpy, gc,  BlackPixel (dpy, dfs) );
			  }
		    else if ((gray >= 16.0/17.0)  &&
		          	    (gcv.foreground != WhitePixel (dpy, dfs) ))
			  {  /* WHITE */
			  if (gcv.fill_style != FillSolid)
				{
				gcv.fill_style = FillSolid;
				XSetFillStyle(dpy, gc, FillSolid);
				}
		          gcv.foreground = WhitePixel (dpy, dfs);
			  XSetForeground (dpy, gc, WhitePixel (dpy, dfs));
			  }
		    else
			/*
			 * Currently, all the DDIF gray patterns are numbered
			 * consecutivly.  If this changes, you could create
			 * an array containing the pattern numbers and
			 * dereference off of that.
			 */
			{
			patt_index = 62;
		    	for (i=15; gray < (float)(i/17.0); i--)
				patt_index--;
		    	tile( patt_index, dpy, win, gc, &gcv,
				   WhitePixel(dpy,dfs), BlackPixel(dpy,dfs) );
                         }
		    }
		else
		    {
 		    fore = alloc_color (vw, patt);
		    XSetForeground (dpy, gc, fore);
		    XSetFillStyle(dpy, gc, FillSolid);
		    }
                break;
		}
	case (pat_standard):
		{
		patt_index = patt->pat__specific.pat__standard.pat_index;
		if  (DisplayPlanes (dpy, dfs) == 1)
		    {
		    if (patt_index == 0)
			  return (FALSE);
		    if (patt_index == 1)   /* White */
			  {
			  XSetForeground (dpy, gc,  gcv.background );
			  gcv.fill_style = FillSolid;
			  XSetFillStyle(dpy, gc, FillSolid);
			  }
		    if (patt_index == 2)  /* Black */
			  {
			  gcv.fill_style = FillSolid;
			  XSetFillStyle(dpy, gc, FillSolid);
			  }
		    if (patt_index > 2 && patt_index < 63)
		    tile(patt_index, dpy, win, gc, &gcv,
	      	         gcv.foreground, gcv.background);
		    }
		else
		    {
 		    back = alloc_color (vw, patt->pat__specific.pat__standard.pat_colors[0]);
 		    fore = alloc_color (vw, patt->pat__specific.pat__standard.pat_colors[1]);

		    if (patt->pat__specific.pat__standard.pat_colors[0]->pat_type == pat_solid_black )
			back = gcv.foreground;
		    if (patt->pat__specific.pat__standard.pat_colors[0]->pat_type == pat_solid_white )
			back = gcv.background;
		    if (patt->pat__specific.pat__standard.pat_colors[1]->pat_type == pat_solid_black )
			fore = gcv.foreground;
		    if (patt->pat__specific.pat__standard.pat_colors[1]->pat_type == pat_solid_white )
			fore = gcv.background;

		    tile(patt_index, dpy, win, gc, &gcv, fore, back);
		    }
		break;
		}
	case (pat_raster):
		{
		dvr_error_callback (vw, 0, DVR_GRAPHICFAIL, 0, 0);
		return (DVR_GRAPHICFAIL);
		break;
		}
	case (pat_unknown):
		{
		dvr_error_callback (vw, 0, DVR_GRAPHICFAIL, 0, 0);
		return (DVR_GRAPHICFAIL);
                break;
		}
	}
	/* If we got here, pattern is visible */
	*flag = TRUE;

	return (DVR_NORMAL);
}

/*****************************************************************************
 * dvr_fill_path (path, gattr, vw, flags, private)
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Draws to the widget work area the filled shape given by the path,
 *	using the attributes to control the appearance of the fill.
 *
 *  FORMAL PARAMETERS:
 *
 *	path	R	This structure contains the actual path
 * 			which is used for the drawing.  This path given in
 *			a PIXEL coord system, and is always a line or arc.
 *			Beziers are broken into polylines during the
 *			format phase.
 *
 *	gattr	R	Graphics attributes of figure.  Used to determine
 *			the fill pattern.
 *
 *	vw	RW	Viewer widget pointer.  Used to get parameters for
 *			X drawing calls, and to pass to dvr_error_callback
 *
 *	flags 	R	Used to signal "close figure", and "pie arc"
 *
 *	private	R	Object-type specific data set up during format phase.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *      none
 *
 *  IMPLICIT OUTPUTS:
 *
 *      Updated gc in server.  This sets the state for future X Drawing calls
 *	as well as the Drawing calls made by this routine.
 *	Updated screen display.
 *
 *  FUNCTION VALUE:
 *
 *      DVR_NORMAL if no errors signaled
 *	DVR_GRAPHICFAIL if any errors occured
 *
 *  SIDE EFFECTS:
 *
 *      Signals any errors that occur during processing.
 *
 ******************************************************************************/

/*
 * N.B.:  If an object is both filled and stroked, fill_path ought to be
 * called AFTER stroke_path.  Otherwise, the filled area may cover part
 * of the outlined line.
 */

CDAstatus
dvr_fill_path(path, gattr, vw, flags, private)
PTH path;		/* points of figure to be drawn */
GAT gattr;		/* graphics attributes of figure */
longword flags;		/* pie arc, close figure */
DvrViewerWidget  vw;
STR private;    	/* object specific data */

{
Window win;
Display *dpy;
GC gc;
XGCValues gcv;		/* shadows what's in gc */
DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);

XPoint *xpoints;	/* parameter to XDrawLine */
signlong *lpoints;	/* pointer to layout engine's list of points */
PTH subpath;		/* children of path (for recursive path definitions */
int arc_x,arc_y;	/* parameter to XDrawArc */
unsigned int arc_width, arc_height;	/* parameter to XDrawArc */
int arc_begin_angle,arc_rotate_angle;	/* parameter to XDrawArc */
int i, count;		/* always useful to have around */
int Xtop, Ytop;         /* Difference between origin of paper & of display */
boolean  visible;       /* If object is not visible, don't draw it */
/*
 *   Actual drawing goes here
 */

gcv = dvr_ptr->gcv;
win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gc =  dvr_ptr->GcId;
Xtop = dvr_ptr->Xtop;
Ytop = dvr_ptr->Ytop;

dvr_output_pattern (vw, gattr->gat_line_int_patt_ptr,  &visible);

if (! visible)   /* No need to draw anything */
	return (DVR_NORMAL);


switch (path->pth_type)
    {
    case (pth_crv):
#	ifdef DEBUG
     	printf("Drawing filled curve\n");
#	endif
    case (pth_lin):
    case (pth_rotated):
	{
#	ifdef DEBUG
	printf("Drawing filled polyline\n");
#	endif

	count = path->pth__specific.pth__xy.pth_pairs;
	xpoints = (XPoint *)malloc(sizeof(XPoint)* count);
	lpoints = &path->pth__specific.pth__xy.pth_first_x;
	for (i=0; i < count; i++)
		{
		xpoints[i].x = *lpoints - Xtop;
		lpoints++;
		xpoints[i].y = *lpoints - Ytop;
		lpoints++;
		}
	XFillPolygon( dpy, win, gc, xpoints, count, Complex, CoordModeOrigin);
	free(xpoints);
	break;
	}
    case (pth_arc):
	{
	long arc_temp_y_radius;

#	ifdef DEBUG
	printf("Drawing filled arc\n");
#	endif
	/*
	 *  Arc drawing.  Ignores rotation angle, since rotating
	 *  ellipses is non trivial
	 */
	if (flags & ddif_m_arc_pie_arc)
		XSetArcMode(dpy,gc,ArcPieSlice);
	else
		XSetArcMode(dpy,gc,ArcChord);

	if (vw->dvr_viewer.x_dpi != vw->dvr_viewer.y_dpi)
	  {
  	    /*  radius x is calculated in format phase using x dpi; to get actual
	     *  arc height, we have to use y dpi, and then add y radius which is
	     *  actually just an offset from the x offset; this should really be
	     *  done in format phase for more optimization; done here for now to
	     *  reduce amount of code change late in project. (only need to do
	     *  this if x dpi differs from y dpi; else use width calculated in
	     *  in format phase.)
	     */
	    arc_temp_y_radius = CONVERT_PIXEL_TO_CP
			(path->pth__specific.pth__arc.pth_radius_x,
			 vw->dvr_viewer.x_dpi);
	    arc_temp_y_radius = CONVERT_CP_TO_PIXEL(arc_temp_y_radius,
					vw->dvr_viewer.y_dpi);
	  }
	else
	    arc_temp_y_radius = path->pth__specific.pth__arc.pth_radius_x;

	arc_x = (int)(path->pth__specific.pth__arc.pth_center_x
			- path->pth__specific.pth__arc.pth_radius_x  - Xtop);

	arc_y = (int)(path->pth__specific.pth__arc.pth_center_y
		      - (arc_temp_y_radius
		         + path->pth__specific.pth__arc.pth_radius_y) - Ytop);
	arc_width =
		(unsigned int)path->pth__specific.pth__arc.pth_radius_x*2 ;

	arc_height =
		(unsigned int)(path->pth__specific.pth__arc.pth_radius_y*2
			            +	arc_temp_y_radius*2);
	arc_begin_angle = (int)path->pth__specific.pth__arc.pth_start
				* 64*CENTI;
	arc_rotate_angle = (int)path->pth__specific.pth__arc.pth_extent
				* 64*CENTI;

        if ( path->pth__specific.pth__arc.pth_rotation == 0)
	   XFillArc( dpy, win, gc, arc_x, arc_y, arc_width,
		    arc_height, arc_begin_angle, arc_rotate_angle);
	break;
	}
    default:
	{
	dvr_error_callback (vw, 0, DVR_GRAPHICFAIL, 0, 0);
	return(DVR_GRAPHICFAIL);
	}
    }

FOREACH_INLIST(path->pth_str.str_queue, subpath, PTH)
	dvr_fill_path(subpath, gattr, vw, flags, private);

/* for temporary solution, let gcv.fore,back always stay same.
 * always force a reset.  We'll work a better staregy out later.
 */

gcv.foreground =  vw->manager.foreground;
gcv.background =  vw->core.background_pixel;
gcv.fill_style = FillSolid;

XSetFillStyle(dpy, gc, FillSolid);
XSetForeground (dpy, gc,  gcv.foreground );
XSetBackground (dpy, gc,  gcv.background );

return ( DVR_NORMAL);
}


/*****************************************************************************
 * dvr_stroke_path (path, gattr, vw, flags, private,
 *		    draw_pattern_size, draw_pattern)
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Draws to the widget work area an outline of the given path, using the
 *	given attributes to control the appearance of the line.
 *
 *  FORMAL PARAMETERS:
 *
 *	path	R	This structure contains the actual path
 * 			which is used for the drawing.  This path given in
 *			a PIXEL coord system, and is always a line or arc.
 *			Beziers are broken into polylines during the
 *			format phase.
 *
 *	gatt	R	Graphics attributes of figure.  Used to determine
 *			line join, dashes, etc.
 *
 *	vw    	RW	Viewer widget pointer.  Used to get parameters for
 *			X drawing calls, and to pass to dvr_error_callback
 *
 *	flags	R	Used to signal "close figure", and "pie arc"
 *
 *	private	R	Object specific data set up during format phase.
 *			Contains arc end points, for example.
 *
 *	draw_pattern_size	R	Number of bits in the line draw
 *					pattern; if no line draw pattern was
 *					specified, size will be zero.
 *
 *	draw_pattern		R	Bit string containing the line
 *					draw pattern.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *      none
 *
 *  IMPLICIT OUTPUTS:
 *
 *      Updated gc in server.  This sets the state for future X Drawing calls
 *	as well as the Drawing calls made by this routine.
 *	Updated screen display.
 *
 *  FUNCTION VALUE:
 *
 *      DVR_NORMAL if no errors signaled
 *	DVR_GRAPHICFAIL if any errors occured
 *
 *  SIDE EFFECTS:
 *
 *      Signals any errors that occur during processing.
 *
 ******************************************************************************/


/*
 * N.B.:  If an object is both filled and stroked, fill_path ought to be
 * called AFTER stroke_path.  Otherwise, the filled area may cover part
 * of the outlined line.
 */


CDAstatus
dvr_stroke_path(path, gattr, vw, flags, private,
		draw_pattern_size, draw_pattern)
PTH		path;
GAT		gattr;
longword	flags;
DvrViewerWidget vw;
STR		private;
longword	draw_pattern_size;
byte		* draw_pattern;


{
Window		win;
Display		* dpy;
GC		gc;
XGCValues	gcv;		 /* shadows what's in gc */
DvrStructPtr	dvr_ptr = &(vw->dvr_viewer.Dvr);
unsigned int	width;		 /* parameter to XSetLineAttributes */
int		join,cap,style;	 /* parameter to XSetLineAttributes */
char		* xdashes;  	 /* parameter to XSetLineAttributes */
signlong	* ldashes;	 /* pointer to layout engine's dash list */
int		dash_count;	 /* parameter to XSetLineAttributes */
boolean		change_gc;	 /* true if gattr and gc disagree */
XPoint		* xpoints;	 /* parameter to XDrawLine */
signlong	* lpoints;	 /* pointer to layout engine's list of points */
PTH		subpath;	 /* children of path (for recursive path definitions */
int		arc_x,arc_y;	 /* parameter to XDrawArc */
unsigned int	arc_width;
unsigned int	arc_height;	 /* parameter to XDrawArc */
int		arc_begin_angle;
int		arc_rotate_angle;/* parameter to XDrawArc */
int		i, count;	 /* always useful to have around */
int		Xtop, Ytop;      /* Difference between origin of paper & of display */
boolean		visible;         /* If object is not visible, don't draw it */

gcv = dvr_ptr->gcv;
win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gc =  dvr_ptr->GcId;

Xtop = dvr_ptr->Xtop;
Ytop = dvr_ptr->Ytop;


/*
 *  Set width of line in pixels
 */
width =  CONVERT_CP_TO_PIXEL (gattr->gat_line_width, vw->dvr_viewer.x_dpi);
if (width == 0)
	width = 1;
if (width != gcv.line_width)
	{
	change_gc = TRUE;
	gcv.line_width = width;
	}

/*
 *  Set style of line ends (caps)  Note that we only look at the starting
 *  cap for now.  If the end cap is different from the start, you won't
 *  see it.  Also note no arrowheads!
 */
switch (gattr->gat_line_start_end)
	{
	case (DDIF_K_BUTT_LINE_END):
		{
		cap = CapButt;
		break;
		}
	case (DDIF_K_ROUND_LINE_END):
		{
		cap = CapRound;
		break;
		}
	case (DDIF_K_SQUARE_LINE_END):
		{
		cap = CapProjecting;
		break;
		}
	case (DDIF_K_ARROW_LINE_END):
		{
		cap = CapProjecting;
		break;
		}
       case (DDIF_K_UNFILLED_ARROW_LINE_END):
                {
                cap = CapButt;
                break;
                }
	default:
		{
		dvr_error_callback (vw, 0, DVR_GRAPHICFAIL, 0, 0);
		cap = CapRound;
		break;
		}
	}

if ((gattr->gat_line_end_end == DDIF_K_ARROW_LINE_END)||
    (gattr->gat_line_end_end == DDIF_K_UNFILLED_ARROW_LINE_END))
   cap = CapButt;

if ((gattr->gat_line_start_end == DDIF_K_ARROW_LINE_END)||
    (gattr->gat_line_start_end == DDIF_K_UNFILLED_ARROW_LINE_END))
   cap = CapButt;

if (cap != gcv.cap_style)
	{
	change_gc = TRUE;
	gcv.cap_style = cap;
	}

/*
 *  Set style of line joins.  Note that X has no settable mitre limit
 */
switch (gattr->gat_line_join)
	{
	case (DDIF_K_MITERED_LINE_JOIN):
		{
		join = JoinMiter;
		break;
		}
	case (DDIF_K_ROUNDED_LINE_JOIN):
		{
		join = JoinRound;
		break;
		}
	case (DDIF_K_BEVELED_LINE_JOIN):
		{
		join = JoinBevel;
		break;
		}
	default:
		{
		dvr_error_callback (vw, 0, DVR_GRAPHICFAIL, 0, 0);
		join = JoinRound;
		break;
		}
	}
if (join != gcv.join_style)
	{
	change_gc = TRUE;
	gcv.join_style = join;
	}



/*
 *  Set style of line dashes.  For now, any time we see a dashed line, we
 *  reset the dashes, since comparing dash lists is messy.
 */
if ( (gattr->gat_line_style != 0) &&
     (gattr->gat_line_style_ptr->lst_num_segments !=0) )
	{
#	ifdef DEBUG
	printf("Dashed line\n");
#	endif

	change_gc = TRUE;
	style = LineOnOffDash;
	gcv.line_style = style;
	cap = CapButt;		/* The other line caps look REALLY bad */
	if (cap != gcv.cap_style)
		{
		change_gc = TRUE;
		gcv.cap_style = cap;
		}

	dash_count = gattr->gat_line_style_ptr->lst_num_segments;
	xdashes = (char *)malloc ((size_t) dash_count);
	ldashes = &gattr->gat_line_style_ptr->lst_first_segment;
	for (i=0; i < dash_count; i++, ldashes++)
		{
		/* Topeleski's const.= 1.33 */
		/*
		* Convert the dash length from centipoints to pixels;
		* if the dash length in pixels is zero (as a result of
		* specifying a very small line pattern size), force the
		* dash length in pixels to be one pixel long, because
		* XSetDashes requires all of the dash lengths to be nonzero;
		* put the dash length in pixels into the xdashes list.
		*/
		xdashes[i] =
		    (char) MAX (CONVERT_CP_TO_PIXEL
				    ((*ldashes * gattr->gat_line_patt_size),
				     vw->dvr_viewer.x_dpi),
			        1L);
		}
	XSetDashes (dpy, gc, 0, xdashes, dash_count);
	free(xdashes);
	}
else
	{
	style = LineSolid;
	if (style != gcv.line_style)
		{
		change_gc = TRUE;
		gcv.line_style = style;
		}
	}

/*
 * Tell server about the graphics context changes, but only if necessary.
 * GC validation is expensive!
 */

dvr_output_pattern (vw, gattr->gat_line_msk_patt_ptr,  &visible);

if (change_gc)
	XSetLineAttributes (dpy, gc, width, style, cap, join);


if (! visible)    /* No need to draw anything */
	return (DVR_NORMAL);

/*
 *   Actual drawing goes here
 */

switch (path->pth_type)
    {
    case (pth_crv):
#	ifdef DEBUG
	printf("Drawing outlined curve\n");
#	endif
    case (pth_lin):
    case (pth_rotated):
	{
#	ifdef DEBUG
	printf("Drawing outlined polyline\n");
#	endif

	count = path->pth__specific.pth__xy.pth_pairs;
        /*
        ** In the format phase one set or two sets of coordinates are added
	** for the rotated ellipses depending on the arc mode. Here in the
	** dvr_stroke_path routine these additional coordinates have to be
	** considered only if the close_arc bit is set. Those two coordinates
	** have to be condsidered regardless of the state of the close_arc bit
	** in dvr_fill_path routine
	*/
        if ( path->pth_type == pth_rotated)
           {
	   if (flags & ddif_m_arc_pie_arc)
              {
               if ((flags & ddif_m_arc_close_arc) && private )
	          count = path->pth__specific.pth__xy.pth_pairs;
               else
                  count = count - 2;
              }

	   if ( !(flags & ddif_m_arc_pie_arc))
              {
              if ((flags & ddif_m_arc_close_arc) && private )
	         count = path->pth__specific.pth__xy.pth_pairs;
              else
                 count = count - 1;
	      }
          }
	xpoints = (XPoint *)malloc(sizeof(XPoint)*(count+1));
	lpoints = &path->pth__specific.pth__xy.pth_first_x;
	for (i=0; i < count; i++)
		{
		xpoints[i].x = *lpoints - Xtop;
		lpoints++;
		xpoints[i].y = *lpoints - Ytop;
		lpoints++;
		}
	if (flags & ddif_m_lin_close_polyline)
		{
		xpoints[count] = xpoints[0];
		count++;
		}
	dvr_stroke_polyline (vw, xpoints, count,
			     draw_pattern_size, draw_pattern);
	free(xpoints);
	break;
	}
    case (pth_arc):
	{
	long arc_temp_y_radius;
	/*
	 *  Arc drawing.  Ignores rotation angle, since rotating
	 *  ellipses is non trivial
	 */
#	ifdef DEBUG
	printf("Drawing outlined arc\n");
#	endif

	if (vw->dvr_viewer.x_dpi != vw->dvr_viewer.y_dpi)
	  {
  	    /*  radius x is calculated in format phase using x dpi; to get actual
	     *  arc height, we have to use y dpi, and then add y radius which is
	     *  actually just an offset from the x offset; this should really be
	     *  done in format phase for more optimization; done here for now to
	     *  reduce amount of code change late in project. (only need to do
	     *  this if x dpi differs from y dpi; else use width calculated in
	     *  in format phase.)
	     */
	    arc_temp_y_radius = CONVERT_PIXEL_TO_CP
				(path->pth__specific.pth__arc.pth_radius_x,
				 vw->dvr_viewer.x_dpi);
	    arc_temp_y_radius = CONVERT_CP_TO_PIXEL(arc_temp_y_radius,
				 	vw->dvr_viewer.y_dpi);
	  }
	else
	    arc_temp_y_radius = path->pth__specific.pth__arc.pth_radius_x;

	arc_x = (int)(path->pth__specific.pth__arc.pth_center_x
			- path->pth__specific.pth__arc.pth_radius_x  - Xtop);
	arc_y = (int)(path->pth__specific.pth__arc.pth_center_y
		      - (arc_temp_y_radius
		         + path->pth__specific.pth__arc.pth_radius_y) - Ytop);
	arc_width =
		(unsigned int)path->pth__specific.pth__arc.pth_radius_x*2;

	arc_height =
		(unsigned int)(path->pth__specific.pth__arc.pth_radius_y*2
			            +	arc_temp_y_radius*2);
	arc_begin_angle = (int)path->pth__specific.pth__arc.pth_start
				* 64*CENTI;
	arc_rotate_angle = (int)path->pth__specific.pth__arc.pth_extent
				* 64*CENTI;

	XDrawArc( dpy, win, gc, arc_x, arc_y, arc_width,
		    arc_height, arc_begin_angle, arc_rotate_angle);

	if ((flags & ddif_m_arc_pie_arc)  && (flags & ddif_m_arc_close_arc)
								&& private )
	   {
	   XPoint	    arcpoints[3];
	   ARC_PRIVATE arc_pvt;
	   int	    npnts;
	   arc_pvt =   (ARC_PRIVATE)private;

	   npnts = 3;
	   if ( path->pth__specific.pth__arc.pth_rotation == 0)
              {
	      arcpoints[0].x = (int)arc_pvt->first_endpoint_x - Xtop;
	      arcpoints[0].y = (int)arc_pvt->first_endpoint_y - Ytop;
	      arcpoints[1].x = (int)path->pth__specific.pth__arc.pth_center_x - Xtop;
	      arcpoints[1].y = (int)path->pth__specific.pth__arc.pth_center_y - Ytop;
	      arcpoints[2].x = (int)arc_pvt->second_endpoint_x - Xtop;
	      arcpoints[2].y = (int)arc_pvt->second_endpoint_y - Ytop;
	      XDrawLines( dpy, win, gc, arcpoints, npnts, CoordModeOrigin);
              }
	   }
	if ( !(flags & ddif_m_arc_pie_arc) && (flags & ddif_m_arc_close_arc)
								&& private )
	   {
	   XPoint	    arcpoints[2];
	   ARC_PRIVATE arc_pvt;
	   int	    npnts;
	   arc_pvt =   (ARC_PRIVATE)private;

	   npnts = 2;
           if ( path->pth__specific.pth__arc.pth_rotation == 0)
              {
	      arcpoints[0].x = (int)arc_pvt->first_endpoint_x - Xtop;
	      arcpoints[0].y = (int)arc_pvt->first_endpoint_y - Ytop;
	      arcpoints[1].x = (int)arc_pvt->second_endpoint_x - Xtop;
	      arcpoints[1].y = (int)arc_pvt->second_endpoint_y - Ytop;
	      XDrawLines( dpy, win, gc, arcpoints, npnts, CoordModeOrigin);
	      }
	   }
	break;
	}
    default:
	{
	dvr_error_callback (vw, 0, DVR_GRAPHICFAIL, 0, 0);
	return ( DVR_GRAPHICFAIL);
	}
    }

FOREACH_INLIST(path->pth_str.str_queue, subpath, PTH)
	dvr_stroke_path(subpath, gattr, vw, flags, private,
			draw_pattern_size, draw_pattern);

/* for temporary solution, let gcv.fore,back always stay same.
 * always force a reset.  We'll work a better strategy out later.
 */

gcv.foreground =  vw->manager.foreground;
gcv.background =  vw->core.background_pixel;
gcv.fill_style = FillSolid;

XSetFillStyle(dpy, gc, FillSolid);
XSetForeground (dpy, gc,  gcv.foreground );
XSetBackground (dpy, gc,  gcv.background );

return ( DVR_NORMAL );
}


/*****************************************************************************
 * dvr_stroke_polyline (vw, xpoints, count, draw_pattern_size, draw_pattern)
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Draws to the widget work area an outline of the polyline whose
 *	vertices are contained in the xpoints array, using the line
 *	draw pattern if one is specified.  The use of XDrawLines (as
 *	opposed to XDrawSegments or multiple calls to XDrawLine) ensures
 *	that line joins will be drawn properly.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw    			RW	Viewer widget pointer.  Used to get
 *					parameters for X drawing calls.
 *
 *	xpoints			R	Array of XPoint structures containing
 *					the polyline coordinate pairs.
 *
 *	count			R	Number of points in the xpoints array;
 *					if the polyline is closed, the initial
 *					point is duplicated at the end.
 *
 *	draw_pattern_size	R	Number of bits in the line draw
 *					pattern; if no line draw pattern was
 *					specified, size will be zero.
 *
 *	draw_pattern		R	Bit string containing the line
 *					draw pattern.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *      none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Updated screen display.
 *
 *  FUNCTION VALUE:
 *
 *      DVR_NORMAL
 *
 *  SIDE EFFECTS:
 *
 *      Signals any errors that occur during processing.
 *
 ******************************************************************************/


unsigned long
dvr_stroke_polyline (vw, xpoints, count, draw_pattern_size, draw_pattern)
DvrViewerWidget vw;
XPoint *xpoints;
int count;
longword draw_pattern_size;
byte *draw_pattern;

{

Window win;
Display *dpy;
GC gc;
XGCValues gcv;
DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);
longword *draw_pattern_bits;
byte *current_draw_pattern_byte;
byte draw_pattern_mask;
longword bit_index;
XPoint *segment_group_start;
XPoint *segment_group_end;
XPoint *end_xpoints;
int segment_points_count;

/*
* Obtain X items.
*/
gcv = dvr_ptr->gcv;
win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gc =  dvr_ptr->GcId;

/*
* If no line draw pattern was specified at all (or, to be generous to
* various potential CDA applications which might explicitly specify the
* default value, if a one-length '1' was specified), just use the existing
* efficient XDrawLines function to draw the polyline; otherwise, draw the
* polyline one segment at a time according to the line draw pattern,
* using XDrawLine.
*/
if ((draw_pattern_size == 0) ||
    (draw_pattern_size == 1) &&
#ifdef CDAbig_endian
    /* bit order reversed on sun, mac, etc */
    (*draw_pattern & 0x80))
#else
    (*draw_pattern & 0x01))
#endif
    XDrawLines (dpy,
		win,
		gc,
		xpoints,
		count,
		CoordModeOrigin);
else

    /*
    * Draw nothing if a one-length '0' pattern was specified (so proceed
    * only if the pattern size is greater than 1; we've already handled
    * the other possible cases above).
    */

    if (draw_pattern_size > 1)
	{

	/*
	* First, copy the line draw pattern into a longword array,
	* to accelerate pattern processing on RISC-based machines.
	* Bit operations are reportedly extremely slow on RISC,
	* so we'll avoid using them here to speed up performance.
	*/

	/*
	* Allocate memory for the pattern array.
	*/
	draw_pattern_bits =
	    (longword *) malloc(sizeof(longword) * draw_pattern_size);

	/*
	* Set the current pattern byte to point to the first byte of
	* the draw pattern, and set the pattern mask to use the first
	* bit of the current pattern byte.
	*/
	current_draw_pattern_byte = draw_pattern;

#ifdef CDAbig_endian
	draw_pattern_mask = 0x80;
#else
	draw_pattern_mask = 0x01;
#endif

	/*
	* Loop through the bits of the pattern mask, copying them
	* one by one into the pattern array.  The CDA Toolkit makes
	* the big/little endian issue for bit strings transparent to
	* CDA applications, so we need not be concerned about that here.
	*/
	for (bit_index = 0; bit_index < draw_pattern_size; bit_index++)
	    {

	    /*
	    * Set or clear the current element "bit" of the pattern array.
	    */
	    if (*current_draw_pattern_byte & draw_pattern_mask)
		draw_pattern_bits[bit_index] = 1L;
	    else
		draw_pattern_bits[bit_index] = 0L;

	    /*
	    * Left-shift the set bit in the pattern mask to use the next bit
	    * in the current pattern byte; if we've used all the bits in the
	    * current pattern byte (by left-shifting the set bit off the left
	    * end of the mask so the mask is now 0), move on to the next byte
	    * in the draw pattern, and reset the pattern mask to use the first
	    * bit of this next pattern byte.
	    */
#ifdef CDAbig_endian
	    if ((draw_pattern_mask >>= 1) == 0x00)
#else
	    if ((draw_pattern_mask <<= 1) == 0x00)
#endif
		{
		current_draw_pattern_byte++;
#ifdef CDAbig_endian
		draw_pattern_mask = 0x80;
#else
		draw_pattern_mask = 0x01;
#endif
		}

	    }   /* End bit-index for loop */

	/*
	* Done copying the line draw pattern into a longword array,
	* Now use the pattern array to draw the polyline.
	*/

        /*
	* Reset the bit index to point to the beginning of the
	* pattern array.
	*/
	bit_index = 0;

        /*
	* Initialize the segment group pointers into xpoints, to point
	* to the start of the xpoints array; set the end pointer to
	* point to the last element of the xpoints array.
	*/
	segment_group_start = xpoints;
	segment_group_end = xpoints;
	end_xpoints = xpoints + count - 1;

	/*
	* Loop through the line segments, using "sliding pointers"
	* to traverse the xpoints array.  The strategy is to move the
	* segment group pointers along to point to the start of a group
	* of xpoints segments with a corresponding group of contiguous
	* 1's in the line draw pattern array, and then drawing that
	* segment group with a single call to XDrawLines (which draws
	* line joins and is much more efficient than multiple calls to
	* XDrawline).  Then the segment group pointers are moved along
	* to the next segment group, skipping over those segments with
	* pattern 0's, and this is repeated for all points in the polyline.
	* The final segment group pointer should point to the next-to-last
	* element of xpoints, because that is the first xpoint in the
	* pair of xpoints of the final segment in the polyline.
	* The polyline closure segment, if any, has previously been put
	* into the xpoints array, and so the closure segment will be
	* correctly drawn (or not) according to the pattern array.
	*/
	while (segment_group_end < end_xpoints)
	    {

	    /*
	    * Count the number of segments in this group of pattern 1's.
	    */
	    while ((draw_pattern_bits[bit_index] == 1L) &&
		   (segment_group_end < end_xpoints))
		{

		/*
		* Increment the segment end pointer; do NOT move the
		* segment start pointer... it must remain fixed at the
		* start of this segment group, and later it will be
		* passed to XDrawLines.  By pointing directly into
		* xpoints, we will avoid making time-wasting copies
		* of partial xpoints data to temporary structures.
		*/
		segment_group_end++;

		/*
		* If we're at the end of the pattern array, reset the array
		* index to re-use the pattern from the beginning.
		*/
		if (++bit_index == draw_pattern_size)
		    bit_index = 0;

		}   /* End pattern 1's while loop */

	    /*
	    * If there are any segments in this group (there may not be,
	    * especially if the pattern starts with one or more zeroes,
	    * or other such cases), draw the segments.
	    */
	    segment_points_count =
		(segment_group_end - segment_group_start) + 1;
	    if (segment_points_count > 1)
		{

		/*
		* Draw the segments of this group with a single call to
		* XDrawLines, specifying the pointer to the start of the
		* segment group within xpoints and the number of segments
		* that are in this group of pattern 1's.
		*/
		XDrawLines (dpy,
			    win,
			    gc,
			    segment_group_start,
			    segment_points_count,
			    CoordModeOrigin);

		/*
		* Move the segment group start pointer to just beyond
		* this segment group within the xpoints array.
		*/
		segment_group_start = segment_group_end;

		}   /* End if draw segment group */

	    /*
	    * Move the segment group pointers to the start of the next
	    * segment group, skipping over the xpoints elements which
	    * correspond to pattern 0's.
	    */
	    while ((draw_pattern_bits[bit_index] == 0L) &&
		   (segment_group_end < end_xpoints))
		{

		/*
		* Skip over this xpoint element with pattern 0.
		*/
		segment_group_start++;
		segment_group_end++;

		/*
		* If we're at the end of the pattern array, reset the array
		* index to re-use the pattern from the beginning.
		*/
		if (++bit_index == draw_pattern_size)
		    bit_index = 0;

		}   /* End pattern 0's while loop */

	    }   /* End line-segment while loop */

	/*
	* Deallocate memory for the pattern array.
	*/
	free (draw_pattern_bits);

	}   /* End if draw_pattern_size */

return (DVR_NORMAL);

}


void
DvrSetClip(left, bottom, right, top, vw, flags)
int left, bottom;		/* lower left x and y */
int right, top;		/* upper right x and y */
longword flags;		/* pie arc, close figure */
DvrViewerWidget  vw;


{
Window win;
Display *dpy;
GC gc;
XGCValues gcv;		/* shadows what's in gc */
DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);

XRectangle rect;
int i, count;		/* always useful to have around */
int Xtop, Ytop;

/*
 *   Actual drawing goes here
 */

gcv = dvr_ptr->gcv;
win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gc =  dvr_ptr->GcId;
Xtop = dvr_ptr->Xtop;
Ytop = dvr_ptr->Ytop;

        rect.x = left;
        rect.y = bottom;
        rect.width = right - left;
        rect.height = top - bottom;
	rect.x += Xtop;
	rect.y += Ytop;

XSetClipRectangles (dpy, gc, 0, 0, &rect, 1, YXBanded);
}

void
dvr_draw_line(vw, x1, y1, x2, y2, wid)
DvrViewerWidget  vw;
CDAmeasurement x1, y1, x2, y2, wid;
{
Display *dpy;
Window  win;
GC   gc;
XGCValues gcv;
int Xtop, Ytop;
DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);


win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gcv = dvr_ptr->gcv;
gc =  dvr_ptr->GcId;

gcv.line_width = wid;
gcv.line_style = LineSolid;
gcv.cap_style = CapButt;
XSetLineAttributes(dpy, gc, wid, LineSolid, CapButt, gcv.join_style);

XDrawLine(dpy, win, gc, x1, y1, x2, y2);
}


/*  Here to be called from the debugger. Remember to deposit dpy in d! */

Synch()
      { Display *d;

	d = (Display *)5;
	XSync (d, FALSE); }

/*
 *  ADDITIONAL ROUTINES TO PERFORM DEVICE DEPENDENT WINDOW OPERATIONS
 */


CDAstatus
dvr_display_polyline_object (vw, poly)
DvrViewerWidget	    vw;
LIN		    poly;

{
GC		gc;
XGCValues	gcv;
Display		* dpy;
DvrStructPtr	dvr_ptr = &(vw->dvr_viewer.Dvr);
longword	flags;
OBJ_PRIVATE	private;
LIN_PRIVATE     line_private;
ARR_INFO        arrow_struct;
PTH		path;
unsigned long	fore;
boolean		visible;   /* If object is not visible, don't draw it */
boolean		round_smooth = FALSE;
boolean		arrow_flag = FALSE;

dpy = XtDisplay(vw);
gcv = dvr_ptr->gcv;
gc =  dvr_ptr->GcId;
fore = gcv.foreground;

flags = poly->lin_obj.obj_flags;

private = (OBJ_PRIVATE)(poly->lin_obj.obj_user_private);
path = poly->lin_obj.obj_path;

/*
** Check to see if the poly line is rounded or smoothed
*/
if ( (flags & ddif_m_lin_rounded_polyline ) ||
     (QUE_GET(path->pth__specific.pth__xy.pth_qjoin)) )
   round_smooth = TRUE;

/*
** Ckeck to see if there are any arrow heads on the poly line
*/
if ((poly->lin_attributes.gat_line_start_end == DDIF_K_ARROW_LINE_END)||
    (poly->lin_attributes.gat_line_end_end == DDIF_K_UNFILLED_ARROW_LINE_END)||
    (poly->lin_attributes.gat_line_start_end == DDIF_K_UNFILLED_ARROW_LINE_END) ||
    (poly->lin_attributes.gat_line_end_end == DDIF_K_ARROW_LINE_END) )
   arrow_flag = TRUE;

/*
** Don't draw Arrow heads If the polyline is closed
*/
if (flags & ddif_m_lin_close_polyline)
   {
   if (arrow_flag)
      dvr_error_callback (vw, 0, DVR_ARRNOTSUP, 0, 0);
   arrow_flag = FALSE;
   }

line_private = private->private_info;

if ( line_private != NULL && arrow_flag )
   arrow_struct = line_private->arrow_info;

/*
 * Draw the fill first.
*/

if (flags & ddif_m_lin_fill_polyline)
   dvr_fill_path(private->real_path, &poly->lin_attributes,
		      vw, flags, private->private_info);

/*
 * Draw the line after the fill to make sure it doesn't get
 * over-written by the fill operation.
*/

if (flags & ddif_m_lin_draw_polyline)
   {
   if (arrow_flag)
      dvr_stroke_path(arrow_struct->arrow_path, &poly->lin_attributes,
			vw, flags, private->private_info,
			poly->lin_draw_patt_size, &poly->lin_draw_patt);
   else
      dvr_stroke_path(private->real_path, &poly->lin_attributes,
                        vw, flags, private->private_info,
                        poly->lin_draw_patt_size, &poly->lin_draw_patt);
   }
/*
 * Draw the markers last to make sure that they don't
 * get over-written by the fill or the line outline.
*/

if (flags & ddif_m_lin_draw_markers )
   {
   dvr_output_pattern ( vw,
			poly->lin_attributes.gat_marker_msk_patt_ptr,
			&visible);

   if (round_smooth)
      dvr_output_markers ( vw, line_private->eng_path,
			poly->lin_attributes.gat_marker_style,
			poly->lin_attributes.gat_marker_size
		         );
   else
      dvr_output_markers ( vw, private->real_path,
			poly->lin_attributes.gat_marker_style,
			poly->lin_attributes.gat_marker_size
		         );
   }

/*
** Check to if see polyline line has arrow head. Draw arrow heads only if the
** polyline is not closed
*/
if ( !(flags & ddif_m_lin_close_polyline) && arrow_flag)
   {
   if (line_private != NULL)
      {
      dvr_drawarrowheads(vw,
		     &poly->lin_attributes,
		     arrow_struct->arrow_type,
		     arrow_struct->start_arrow,
		     arrow_struct->end_arrow,
		     arrow_struct->arrow_width);
      }
   }

if (gcv.foreground != fore)
	{
	gcv.foreground = fore;
	XSetForeground(dpy, gc, fore);
        }
return(DVR_NORMAL);
}


CDAstatus
dvr_display_arc_object (vw, arc)
DvrViewerWidget	    vw;
ARC		    arc;

{

Display		    * dpy;
GC		    gc;			/* Graphics Context Struct       */
XGCValues	    gcv;		/* Graphics Context variable	 */
ARC_PRIVATE	    arc_pri;		/* Pointer To ARC_PRIVATE Struct */
ARR_INFO	    arrow_struct;	/* Pointer To ARR_INFO strucutre */
PTH		    path;		/* Pointer To A Path Structure	 */
longword	    flags;		/* Flags For the Object		 */
OBJ_PRIVATE	    private;		/* Pointer To OBJ_PRIVATE Struct */
boolean		    arrow_flag = FALSE; /* A Boolean flag for arrows	 */
unsigned long	    fore;
signlong            * coord_ptr;	/* Ptr to The Coordinates	 */
int                 count;		/* No. Of X,Y pairs		 */
DvrStructPtr	    dvr_ptr = &(vw->dvr_viewer.Dvr);
PTH		    rotated_path;

gcv = dvr_ptr->gcv;
gc =  dvr_ptr->GcId;
fore = gcv.foreground;

flags = arc->arc_obj.obj_flags;
private = (OBJ_PRIVATE)(arc->arc_obj.obj_user_private);

if ((arc->arc_attributes.gat_line_start_end == DDIF_K_ARROW_LINE_END) ||
    (arc->arc_attributes.gat_line_end_end == DDIF_K_UNFILLED_ARROW_LINE_END) ||
    (arc->arc_attributes.gat_line_start_end == DDIF_K_UNFILLED_ARROW_LINE_END) ||
    (arc->arc_attributes.gat_line_end_end == DDIF_K_ARROW_LINE_END) )
   arrow_flag = TRUE;

arc_pri = private->private_info;

/*
** Get the Original path
*/
path = private->real_path;

if ( arc_pri != NULL && arrow_flag )
   arrow_struct = arc_pri->arrow_info;

if (flags & ddif_m_lin_fill_polyline)
   {
   if ( (path->pth__specific.pth__arc.pth_rotation != 0) && arc_pri )
      dvr_fill_path(arc_pri->rotated_path, &arc->arc_attributes,
			vw, flags, private->private_info);
   else
      dvr_fill_path(private->real_path, &arc->arc_attributes,
			vw, flags, private->private_info);
   }
if (flags & ddif_m_lin_draw_polyline)
   {
   if (arrow_flag)
      /*
      ** Draw the Shortend Arrow Path
      */
      dvr_stroke_path(arrow_struct->arrow_path, &arc->arc_attributes,
		      vw, flags, private->private_info, 0L, 0L);
   else
      {
      if ( (path->pth__specific.pth__arc.pth_rotation != 0) && arc_pri )
         dvr_stroke_path( arc_pri->rotated_path, &arc->arc_attributes,
	      	          vw, flags, private->private_info, 0L, 0L);
      else
         dvr_stroke_path( private->real_path, &arc->arc_attributes,
	      	          vw, flags, private->private_info, 0L, 0L);
      }
   }

/*
** Check to see if arc has arrow head.
*/
if (arrow_flag)
   {
   if (arc_pri != NULL)
      {
      dvr_drawarrowheads(vw,
		     &arc->arc_attributes,
		     arrow_struct->arrow_type,
		     arrow_struct->start_arrow,
		     arrow_struct->end_arrow,
		     arrow_struct->arrow_width);
      }
   }
if (gcv.foreground != fore)
	{
	gcv.foreground = fore;
	XSetForeground(dpy, gc, fore);
        }
return(DVR_NORMAL);
}


CDAstatus
dvr_display_curve_object (vw, curve)
DvrViewerWidget  vw;
CRV curve;

{
GC   gc;
XGCValues gcv;
Display *dpy;

DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);
longword flags;
OBJ_PRIVATE private;

unsigned long fore;

dpy = XtDisplay(vw);
gcv = dvr_ptr->gcv;
gc =  dvr_ptr->GcId;
fore = gcv.foreground;

/*
 * Check for a null path.  If we have a null path
 * return immediately since the Layout Engine
 * encountered some sort of error.
*/

if (curve->crv_obj.obj_path == 0)
    return;

/*
 * We have a valid set of points so we can proceed.
*/

flags = curve->crv_obj.obj_flags;
private = (OBJ_PRIVATE)(curve->crv_obj.obj_user_private);

if (flags & ddif_m_lin_fill_polyline)
	dvr_fill_path(private->real_path, &curve->crv_attributes,
		      vw, flags, private->private_info);

if (flags & ddif_m_lin_draw_polyline)
	dvr_stroke_path(private->real_path, &curve->crv_attributes,
		      vw, flags, private->private_info, 0L, 0L);

if (gcv.foreground != fore)
	{
	gcv.foreground = fore;
	XSetForeground(dpy, gc, fore);
        }
return(DVR_NORMAL);
}

CDAstatus
dvr_display_fill_object (vw, poly)
DvrViewerWidget  vw;
LIN poly;
{
GC   gc;
XGCValues gcv;
Display *dpy;

DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);
longword flags;
OBJ_PRIVATE private;
unsigned long fore;

dpy = XtDisplay(vw);
gcv = dvr_ptr->gcv;
gc =  dvr_ptr->GcId;
fore = gcv.foreground;

flags = poly->lin_obj.obj_flags;
private = (OBJ_PRIVATE)(poly->lin_obj.obj_user_private);

if (flags & ddif_m_lin_fill_polyline)
	dvr_fill_path(private->real_path, &poly->lin_attributes,
		      vw, flags, private->private_info);

if (flags & ddif_m_lin_draw_polyline)
	dvr_stroke_path(private->real_path, &poly->lin_attributes,
			vw, flags, private->private_info,
			poly->lin_draw_patt_size, &poly->lin_draw_patt);

if (gcv.foreground != fore)
	{
	gcv.foreground = fore;
	XSetForeground(dpy, gc, fore);
        }
return(DVR_NORMAL);
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_display_image_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Displays an image by invoking IDS to create a rendering with
 *	the picture in it.  Put the image up on the screen.  Keep track
 *      of outstanding renderings by putting them on an image list.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	image		- layout engine image structure
 *	x_window_top	- X window position top, for positioning offset
 *	y_window_top	- Y window position top, for positioning offset
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_IMGFAIL	Call to IDS failed for some reason
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      The IDS routines do not return status, they signal
 *	errors.  If IDS signals an error our condition handler
 *	dvr_image_condition_handler will get control and cause
 *	this routine to return to its caller with DVR_IMGFAIL
 *	status.
 *
 *--
 */


CDAstatus dvr_display_image_object (vw, image, x_window_top, y_window_top )
DvrViewerWidget	vw;			/* Viewer widget context pointer */
IMG		image;			/* image object */
CDAmeasurement	x_window_top;		/* X window position top */
CDAmeasurement	y_window_top;		/* Y window position top */
{
  IdsItmlst2		presentation_itmlst[9];
#ifdef CDAlocal_DAS
  IdsRenderContext    	presentation_id;
#else
  unsigned long int	presentation_id;
#endif
  DvrStruct		*dvr_struct;
  OBJ_PRIVATE		object_private;	    /* x,y positions in pixels  */
  IMG_PRIVATE		image_private;	    /* image widget info	*/
  int			image_height;
  int			image_width;
  IMG_LIST_STRUCT	image_window_entry;
  PAG_PRIVATE		page_private;	    /* Current page Dvr info	*/
  int			status;
  Display		*x_display;
  Window		x_window;

  /* Sanity check */

  if ( (vw == NULL) || (image == NULL) )
    return DVR_BADPARAM;

  /*
   * Check if image frame is available.  If it isn't an
   * info message will already have been issued by DVS.
   */
  if ( image->img_fid == 0 )
    return DVR_NORMAL;

  /* Get generic private data pointer off of the DVS object structure */
  object_private = ((OBJ_PRIVATE) ((OBJ) image)->obj_user_private);

  image_private = (IMG_PRIVATE)(object_private->private_info);

  if (image_private == NULL)
    /* bad image error reported during formatting */
    return DVR_NORMAL;

  if (image_private->img_rendering == (struct IdsRendering *) TRUE)
    /* previous error creating the IDS rendering, give up */
    return DVR_NORMAL;
  
  /*
   * If there is already a rendering with this picture then we only need
   * to pop the picture onto the screen, don't make another one.
   */
  if (image_private->img_rendering !=   NULL)
    {
      status = dvr_put_image_to_screen(vw, image
				       , x_window_top, y_window_top);

      return status;
    }

  /*
   * Set the creation flag at this point.  Either the widget will
   * be created successfully and kept track of in a img_list entry
   * off the current page, or it will fail at some point and
   * we don't want to repeatedly try and fail so leave the creation
   * flag set on failure.
   */
  image_private->img_rendering = (struct IdsRendering *) TRUE;
  
  /*
   * Keep a close handle on any image services errors that
   * get signalled.
   */
#ifdef __vms__
  LIB$ESTABLISH( dvr_image_condition_handler );
#else
  ChfEstablish (dvr_image_condition_handler);
  if (setjmp(dvr_env) != 0)
    {
      /* longjmp has been called indicating we have returned from
       * ISl call with an error, either ChfSignal or ChfStop was called.
       */
      dvr_error_callback(vw, 0L, DVR_IMAGEFAIL, NULL, 0L);
      
      dvr_error_callback(vw, 0L, dvr_chf_status, NULL, 0L);
      
      return DVR_IMAGEFAIL;
    }
#endif

  /*
   *  Create a presentation surface that matches our current display.
   *  The surface is used to describe the current device to IDS so it
   *  can render the image appropriately.
   */
  
  x_window = XtWindow (vw->dvr_viewer.work_window);
  x_display = XtDisplay (vw->dvr_viewer.work_window);
  
  image_height = object_private->bbox_lr_y - object_private->bbox_ul_y;
  image_width  = object_private->bbox_lr_x - object_private->bbox_ul_x;
  
  presentation_itmlst[0].item_name = (char *) IdsNprotocol;
  /*    presentation_itmlst[0].value = (unsigned long int) Ids_Pixmap; */
  presentation_itmlst[0].value = (unsigned long int) Ids_XImage;
  presentation_itmlst[1].item_name = (char *) IdsNworkstation;
  presentation_itmlst[1].value = (unsigned long int) x_display;
  presentation_itmlst[2].item_name = (char *) IdsNwsWindow;
  presentation_itmlst[2].value = (unsigned long int) x_window;
  
  /* We don't have the whole viewer display window to play with.  */
  /* Make the target window as big as the image frame.	    */
  
  presentation_itmlst[3].item_name = (char *) IdsNwindowHeight;
  presentation_itmlst[3].value = (unsigned long int) image_height;
  presentation_itmlst[4].item_name = (char *) IdsNwindowWidth;
  presentation_itmlst[4].value = (unsigned long int) image_width;
  
  /* Terminate the item list */
  presentation_itmlst[5].item_name = (char *) NULL;
  presentation_itmlst[5].value = (unsigned long int) NULL;

  presentation_id = IdsCreatePresentSurface(presentation_itmlst);
  
  /* Save the presentation id so we can free it later.	    */
  
  image_private->img_presentation = presentation_id;
  
  /*
   * Render the image into a bitmap that can be used by the current
   * display.  This can take a boatload of time if the image differs
   * significantly from the display, or if there are a lot of colors
   * to negotiate.
   */
  
  /* Make sure the resulting image bitmap is the size and	    */
  /* resolution we want */
  
  presentation_itmlst[0].item_name = (char *) IdsNscaleMode;
  presentation_itmlst[0].value = (unsigned long int) Ids_Flood;
  presentation_itmlst[1].item_name = (char *) IdsNfitWidth;
  presentation_itmlst[1].value = (unsigned long int) image_width;
  presentation_itmlst[2].item_name = (char *) IdsNfitHeight;
  presentation_itmlst[2].value = (unsigned long int) image_height;
  
  /* Terminate the item list					    */
  presentation_itmlst[3].item_name = (char *) NULL;
  presentation_itmlst[3].value = (unsigned long int) NULL;
  
  image_private->img_rendering = (struct IdsRendering *) IdsCreateRendering
    (image->img_fid,presentation_id
     ,presentation_itmlst);

#ifdef __unix__
  ChfRevert();
#endif

  /* Now we have the image bitmap, pop it to the screen!	    */
  
  status = dvr_put_image_to_screen(vw, image
				   , x_window_top, y_window_top);
  
  /*
   * Note that we delete the rendering and the presentation surface
   * in DVR_IMAGE since we must delete the rendering before the presentation
   * surface can be deleted.  This change was made such that Image Services
   * wouldn't corrupt internal data structures.
   */
  
  /*
   * Now we have a rendering that takes up a good-size chunk of memory,
   * keep track of it at the page level.
   * The rendering will need to be deleted when we are no longer
   * on this page.
   */
  status = dvr_alloc_memory (vw, sizeof(struct img_list_struct),
			     (void **) &image_window_entry);
  
  if ( DVR_FAILURE ( status ))
    /* Unexpected error allocating memory */
    return status;

  image_window_entry->object_data = object_private;
  
  dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );
  page_private = (PAG_PRIVATE)(dvr_struct->current_page->pag_user_private);
  
  QUE_APPEND( page_private->img_list, image_window_entry );
  
  return DVR_NORMAL;
}




/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_get_font_info
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get the font info associated with the specified font table index.
 *	Routine loads font if not yet loaded.  If font unloadable, or if
 *	a default font is specified by an invalid font index (-1), default
 *	font information is returned.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	font_index	- Specified font index
 *	font_name	- Specified font name (corresponding to index)
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


CDAstatus dvr_get_font_info (
    vw,
    font_index,
    font_name )

DvrViewerWidget vw;		/* Viewer widget context pointer */
int font_index;			/* Specified font index */
char *font_name;		/* Specified font name */

{
    /* Forward function declarations */
    extern int dvr_get_default_font ();

    extern XFontStruct *XLoadQueryFont ();

    /* Local variables */

    XFontStruct *font_struct;	/* X font struct */
    DvrViewerPtr dvr_window;	/* Viewer widget */
    DvrStruct *dvr_struct;	/* DVR struct pointer */
    char **font_list;		/* X font list (array of char strings) */


    unsigned int font_id;	/* X font ID */
    int status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Check if default font (-1), or invalid font index specified.
     */

    dvr_window = (DvrViewerPtr) &( vw->dvr_viewer );	/* Init */
    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );

    /* Check font index for validity */
    if (( font_index == DEFAULT_FONT_INDEX ) || ( font_index >= dvr_window->num_fonts )) {

	/* Invalid font index; use default */
	status = dvr_get_default_font (
	    vw );

	/* Error callback; using default font */
	dvr_error_callback ( vw, 0, DVR_DEFAULTFONT, 0, 0 );

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
	    return status;
	}

	return DVR_NORMAL;

    /* Else - valid font index specified; continue with processing below */

    }


    /*
     *	Valid font index specified; load font if necessary; return default
     *	font if errors occur during specified font processing.
     */

    /* Check if specified font is already loaded */
    if (( font_id = dvr_window->font_table[font_index].x_font_id ) == 0 ) {

	/* Font not loaded: no corresponding X font ID present; load font */
	font_struct = XLoadQueryFont (
	    XtDisplay (dvr_window->work_window),
	    font_name );

	/* Check for error */
	if ( font_struct == NULL ) {

	    /* Font not loaded successfully; use default font */
	    status = dvr_get_default_font (
		vw );

	    /* Error callback; using default font */
	    dvr_error_callback ( vw, 0, DVR_DEFAULTFONT, 0, 0 );

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
		return status;
	    }
	}

	else {

	    /* Font loaded successfully; save X font ID and X font struct */

	    font_id = (unsigned long) font_struct->fid;		/* Font ID */

	    dvr_window->font_table[font_index].x_font_id =
                (unsigned long) font_id;
	    dvr_window->font_table[font_index].x_font_struct =
                (unsigned long *) font_struct;

	    /* Font will be set in the GC in the display phase */

	}

    }	/* End of X font not yet loaded code */

    else {

    }

    /* Succesful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_get_default_font
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get the default font, load in memory if necessary, and set as the
 *	current font if not already in use.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
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


int dvr_get_default_font (
    vw )

DvrViewerWidget vw;		/* Viewer widget context pointer */

{
    /* Forward function declarations */

    extern XFontStruct *XLoadQueryFont ();

    /* Local variables */
    XFontStruct *font_struct;	/* X font struct */
    DvrViewerPtr dvr_window;	/* Viewer widget */
    DvrStruct *dvr_struct;	/* DVR struct pointer */
    unsigned int font_id;	/* X font ID */
    int status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Set up default font as current font.
     */

    dvr_window = (DvrViewerPtr) &( vw->dvr_viewer );	/* Init */
    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );


    /* Check if default font was not found in font list, or other unloadable
       error */
    if ( dvr_window->default_font_id == DEFAULT_FONT_INDEX )
	return DVR_NORMAL;	/* Just use font available in GC */


    /* Check if default font is loaded */
    if ( dvr_window->default_font_id == 0 ) {

	/* Default font MAY already be loaded as a specifically requested
	   font in the font list; however, the choice is to use a general
	   pattern matching algorithm through the entire list, or to risk
	   reloading this font.  We will risk reloading this font is we
	   MUST try to use the default (otherwise, we should use a default
           font INDEX rather than a default font ID; such changes should
	   probably be limited to use by this routine) */

	/* Load default font */
	font_struct = XLoadQueryFont (
	    XtDisplay (dvr_window->work_window),
	    dvr_window->default_font_str );

	/* Check for error */
	if ( font_struct == NULL ) {
            /*  could not get normal default, try to use first
	     *  font from out list
	     */

	    /* Error callback; couldn't load (default) font */
	    dvr_error_callback ( vw, 0, DVR_NOFONT, 0, 0 );

	    /* reinit default font */
	    XtFree(dvr_window->default_font_str);
	    dvr_window->default_font_str = 0;

	    dvr_window->default_font_str =
		XtMalloc(strlen(dvr_window->available_fonts[0]) + 1);

	    strcpy(dvr_window->default_font_str,
		   dvr_window->available_fonts[0]);

	    font_struct = XLoadQueryFont (
	        XtDisplay (dvr_window->work_window),
	        dvr_window->default_font_str );

            };

	/* Check for error */
	if ( font_struct == NULL ) {
	    /* still Could not load default font; just use whatever font
	       is available in GC currently; report problem */

	    /* Could not load font; don't try again by saving invalid ID */
	    dvr_window->default_font_id = DEFAULT_FONT_INDEX;

	    /* Error callback; couldn't load (default) font */
	    dvr_error_callback ( vw, 0, DVR_NOFONT, 0, 0 );

	    /*  viewer is in trouble in this case; I do not think all
	     *  error condtions are checked for -dam
	     */

	}
	else {

	    /* Default font loaded successfully; save as default and
	       current font ID and font struct; set this font in GC */
	    font_id = (unsigned long) font_struct->fid;	/* Font ID */
            dvr_window->default_font_id = font_id;
            dvr_window->default_font_struct = (unsigned long *)font_struct;
	    dvr_window->current_font_index = DEFAULT_FONT_INDEX;    /* Use default font */

	    /* Font just loaded, so not currently in use;
	       set font in GC in display phase */

	}
    }

    else {

	/* Default font already loaded; check if same as current font */
	if ( dvr_window->current_font_index != DEFAULT_FONT_INDEX ) {

	/* Font not currently in use;
	   set the font in the GC in display phase */

	/* Save font ID as new current font ID */
	dvr_window->current_font_index = DEFAULT_FONT_INDEX;	/* Default font 'index' */

	/* Else - font already set in GC */
	}
    }


    /* Succesful completion */
    return DVR_NORMAL;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_unload_fonts
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Unloads fonts loaded during Viewer session.  Routine clears
 *	font table entry, allowing this structure to be reused with lses
 *	fonts loaded for another file.  Font table is not deallocated.
 *	Also, X font structures are deallocated using the X free font routine.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
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


CDAstatus dvr_unload_fonts (vw)

DvrViewerWidget vw;		/* Viewer widget context pointer */
{
    /* Forward function declarations */
    /* extern int XUnloadFont (); */

    extern int XFreeFont ();

    /* Local variables */
    DvrViewerPtr dvr_window;	/* Viewer widget */
    int i;			/* Working index */
    int status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Unload all fonts marked as loaded in font table.
     */

    dvr_window = (DvrViewerPtr) &( vw->dvr_viewer );	/* Init */

    for ( i = 0; i < dvr_window->num_fonts; ++i ) {

	/* Loop through font table unloaded fonts that were loaded,
	   including default font, if loaded */

	if ( dvr_window->font_table[i].x_font_id ) {
	    /* ID exists, font was loaded */

	    /* Free font structure memory allocated by X, and close
	       (unload) font */

	    /**************************************
	     Note: XLoadFont requires XUnload Font;
	     XLoadQueryFont requires XFreeFont
	     **************************************/

	    (void) XFreeFont (
		XtDisplay (dvr_window->work_window),
		dvr_window->font_table[i].x_font_struct );

	    /* (void) XUnloadFont (
		XtDisplay (dvr_window->work_window),
		dvr_window->font_table[i].x_font_id ); */

	    /* Clear entry field(s) for reuse */
	    dvr_window->font_table[i].x_font_id = 0;
	    dvr_window->font_table[i].x_font_struct = 0;

	/* Else, no ID implies that font was not loaded; loop */
	}
    }


    /*
     *	Unload default font (code may eventually disappear if a default font
     *	index is used rather than a default font ID in future code mods);
     *  worst case is trying to unload an already unloaded font.
     */

    if ( dvr_window->default_font_id > 0 ) {

	/* Real font ID value stored; not 0 or -1; unload font */
	(void) XUnloadFont (
	    XtDisplay (dvr_window->work_window),
	    dvr_window->default_font_id );

	/* Clear default font ID */
	dvr_window->default_font_id = 0;

    /* Else - default font was not specifically or successfully loaded */
    }


    /* Succesful completion */
    return DVR_NORMAL;
}


/*
**++
**  dvr_set_clip
**
**  Functional Description:
**	Using X, set the clip rectangle to the area defined.
**
**
**  Arguments:
**	clip_bbox : clip rectangle in ul-lr pixel coordinates
**
**	vw : Viewer widget (context structure)
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
void dvr_set_clip(clip_bbox, vw)
BBOX_UL_LR	*clip_bbox;
DvrViewerWidget	vw;
    {
    Window  win;
    Display *dpy;
    GC	    gc;
    XGCValues gcv;		/* shadows what's in gc */
    DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);

    XRectangle rect;
    int Xtop, Ytop;


    gcv = dvr_ptr->gcv;
    win = XtWindow(vw->dvr_viewer.work_window);
    dpy = XtDisplay(vw);
    gc =  dvr_ptr->GcId;
    if ((clip_bbox->bbox_lr_x <= clip_bbox->bbox_ul_x)
	 || (clip_bbox->bbox_lr_y <= clip_bbox->bbox_ul_y))
	XSetClipRectangles (dpy, gc, 0, 0, &rect, 0, YXBanded);
    else
	{
	Xtop = dvr_ptr->Xtop;
	Ytop = dvr_ptr->Ytop;
	rect.x = clip_bbox->bbox_ul_x - Xtop;
	rect.y = clip_bbox->bbox_ul_y - Ytop;
	rect.width = clip_bbox->bbox_lr_x - clip_bbox->bbox_ul_x + 1;
	rect.height = clip_bbox->bbox_lr_y - clip_bbox->bbox_ul_y + 1;
							/* y increases down */

	XSetClipRectangles (dpy, gc, 0, 0, &rect, 1, YXBanded);
	}
    }


/*
**++
**  dvr_load_cursor
**
**  Functional Description:
**	call X routines to load watch cursor
**
**++
*/

CDAstatus dvr_load_cursor(vw)
     DvrViewerWidget vw;
{

  DvrViewerPtr pDvrWindow = &(vw->dvr_viewer);

  if ( pDvrWindow->ViewerCursor == (Cursor) NULL)
    {

      /* create watch cursor and store in widget */
      unsigned int cursorfont;
      XColor close_white, close_black, black, white;
      int stat;
      XFontStruct *cursor_font_struct;

      /* load the cursor font */
      cursorfont = XLoadFont(XtDisplay(vw),
			     "decw$cursor");

      /* verify the font */
      cursor_font_struct = XQueryFont(XtDisplay(vw), cursorfont);

      /* get black and white color maps from server for watch */

      stat = XLookupColor(XtDisplay(vw),
			  XDefaultColormap(XtDisplay(vw), 0),
			  "BLACK", &close_black, &black);

      stat = XLookupColor(XtDisplay(vw),
			  XDefaultColormap(XtDisplay(vw), 0),
			  "WHITE", &close_white, &white);

      /* if we could not open font, use X watch, else use decw watch */

      if (cursor_font_struct == NULL)

    	pDvrWindow->ViewerCursor =
	  XCreateFontCursor(XtDisplay(vw), XC_watch);
      else
	{
	  pDvrWindow->ViewerCursor =
	    XCreateGlyphCursor(XtDisplay(vw),
			       cursorfont, cursorfont,
			       decw$c_wait_cursor, decw$c_wait_cursor + 1, &close_white,
			       &close_black);
	  XFreeFont(XtDisplay(vw), cursor_font_struct);
	}
    }
}



/*
**++
**  dvr_set_ts_origin
**
**  Functional Description:
**	call X routines to set Tile Stipple Origin
**
**++
*/

CDAstatus dvr_set_ts_origin(vw)
     DvrViewerWidget vw;		/* Viewer widget context pointer */
{
  DvrStruct *dvr_struct;		/* DVR struct pointer */
  PAG page;				/* Page structure */
  Display *dpy;		    /* Display device */
  GC gc;			    /* X graphics context */

  PAG_PRIVATE page_priv;
  int x_window_top, y_window_top;

  dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
  page = dvr_struct->current_page;

  /*  Set the Tile/Stipple Origin (used by X to draw patterns) to
   *  be the lower right hand corner of the page.  Otherwise, when we
   *  scroll, the patterns will shift relative to the objects.
   */

  page_priv = (PAG_PRIVATE) page->pag_user_private;

  dpy = XtDisplay(vw);
  gc  = dvr_struct->GcId;

  x_window_top = dvr_struct->Xtop;
  y_window_top = dvr_struct->Ytop;

  XSetTSOrigin(dpy, gc,
	       page_priv->width - x_window_top ,
	       page_priv->height - y_window_top);
}


/*
**++
**  dvr_delete_page_colors
**
**  Functional Description:
**	Call X routines to deallocate all the colors used for graphics
**	or text on this page
**
**++
*/

CDAstatus dvr_delete_page_colors(vw)
     DvrViewerWidget vw;
{
  Display		*dpy;
  Window		win;
  XWindowAttributes	winattr;
  int			cmap;
  COLOR_INFO		node;

  PAG_PRIVATE 	cur_page_private;
  PAG 		current_page;

  /*
   *	cruise through the page structure down to the image widget list
   */

  current_page = vw->dvr_viewer.Dvr.current_page;
  if ( current_page == NULL ) return DVR_NORMAL;	/* No current page
							   yet; initial page */
  cur_page_private = (PAG_PRIVATE) current_page->pag_user_private;
  if ( cur_page_private == NULL ) return DVR_NORMAL;	/* Page not yet
							   formatted */

  /* Really, the colormap should be found once per widget*/
  dpy = XtDisplay (vw);
  win = XtWindow (vw->dvr_viewer.work_window);

  if (win)
    {
      XGetWindowAttributes (dpy, win, &winattr);
      cmap = winattr.colormap;

      FOREACH_POPLIST (cur_page_private->color_info_list, node, COLOR_INFO)
	{
	  XFreeColors (dpy, cmap, (unsigned long *) &node->color_info_pixel, 1, 0);
	  XtFree ((char *) node);
	}
    }
}


/*
**++
**  dvr_image_expose_callback
**
**  Functional Description:
**	Expose event routine for image widgets
**
**++
*/
static void dvr_image_expose_callback( w, event, params, num_params )
    Widget	    w;
    XExposeEvent    *event;
    char	    **params;
    int		    num_params;
{

    DvrViewerWidget vw = (DvrViewerWidget) XtParent( XtParent (w));

    event->x = event->x + XtX( w );
    event->y = event->y + XtY( w );
    /*
    **  Invoke the application specific callback routine for the expose
    **  event
    */
    if (vw->dvr_viewer.expose_callback != NULL)					/* was .ecallback in XUI */
	dvr_call_expose_callbacks( vw, event);


}



/*
 *  THE FOLLOWING ARE JUST BLANKETS AROUND CALLS TO X ROUTINES
 */

CDAstatus dvr_set_font(vw, font_id)
     DvrViewerWidget vw;
     int font_id;
{
  /* Font not currently in use; set the font in the GC */
  XSetFont( XtDisplay (vw), vw->dvr_viewer.Dvr.GcId, font_id );
}

#ifdef CDA_TWOBYTE
#ifndef FONT2BYTE
#define FONT2BYTE(fs) (((XFontStruct *)(fs))->min_byte1 != 0 || \
		       ((XFontStruct *)(fs))->max_byte1 != 0)
#endif
#endif

CDAstatus dvr_draw_string (vw, x_pos, y_pos, string, str_len)
     DvrViewerWidget	vw;
     CDAmeasurement	x_pos;
     CDAmeasurement	y_pos;
     char		*string;
     CDAsize		str_len;
{
  extern int XDrawString ();

  Display *dpy;		    /* Display device */
  Window win;			    /* X window */
  GC gc;			    /* X graphics context */

  win = XtWindow (vw->dvr_viewer.work_window);
  dpy = XtDisplay (vw->dvr_viewer.work_window);
  gc  = vw->dvr_viewer.Dvr.GcId;

#ifdef CDA_TWOBYTE
  if (FONT2BYTE (vw->dvr_viewer.current_font_index == DEFAULT_FONT_INDEX ?
		 vw->dvr_viewer.default_font_struct :
		 vw->dvr_viewer.font_table[vw->dvr_viewer.current_font_index].x_font_struct))
    (void) XDrawString16(dpy,win,gc,x_pos,y_pos,string,str_len/2);
  else
    (void) XDrawString(dpy,win,gc,x_pos,y_pos,string,str_len);
#else
  (void) XDrawString (dpy, win, gc, x_pos, y_pos, string, str_len);
#endif

}

CDAmeasurement dvr_text_width (vw, font_struct, string, length)
    DvrViewerWidget	vw;
    unsigned long 	*font_struct;
    char		*string;
    CDAsize		length;

{
#ifdef CDA_TWOBYTE
    if (FONT2BYTE(font_struct))
        return (XTextWidth16(font_struct,string,length/2));
    else
        return (XTextWidth(font_struct,string,length));
#else
    return ( XTextWidth((XFontStruct *) font_struct,
	     		string,
			length) );
#endif
}

CDAstatus dvr_application_callback (vw, reason_val)
     DvrViewerWidget 	vw;
     void		*reason_val;

{
  XtCallCallbacks((Widget) vw, XmNactivateCallback, reason_val);
}

CDAstatus dvr_kill_widget(widget_id)
     Widget widget_id;
{
  /* unmap the widget */
  XtUnmanageChild(widget_id);

  /* Get rid of this widget */
  XtDestroyWidget(widget_id);
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_put_image_to_screen
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Put an IDS image rendering up on the screen (redraw an image)
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	image		- image object structure
 *	x_window_top	- X window position top, for positioning offset
 *	y_window_top	- Y window position top, for positioning offset
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *	May cause X errors for whatever reason.
 *
 *--
 */

unsigned long dvr_put_image_to_screen (
    vw,
    image,
    x_window_top,
    y_window_top )

DvrViewerWidget vw;		/* Viewer widget context pointer */
IMG	image;			/* image object */
int	x_window_top;		/* X window position top */
int	y_window_top;		/* Y window position top */


{

    OBJ_PRIVATE	object_private;	    /* x,y positions in pixels  */
    IMG_PRIVATE	image_private;	    /* image widget info	*/
    IdsRendering *rendering;	    /* IDS bitmap data structure */
    Display	*x_display;	    /* Display device */
    Window	x_window;	    /* X window */
    int		x_coord, y_coord;   /* Coordinates of image in the window */
    GC		x_graphics_ctx,	    /* X graphics context */
		gc;		    /* Our own GC */
    XGCValues	gcv;
    XImage	*x_image;

    /* Get generic private data pointer off of the DVS object structure */

    object_private = ((OBJ_PRIVATE) ((OBJ) image)->obj_user_private);

    image_private = (IMG_PRIVATE)(object_private->private_info);

    rendering = (IdsRendering *) image_private->img_rendering;

    /* Get the current window and display handles	*/

    x_window = XtWindow (vw->dvr_viewer.work_window);
    x_display = XtDisplay (vw->dvr_viewer.work_window);
    x_graphics_ctx  = rendering->type_spec_data.xlib.image_gc; /* was vw->dvr_viewer.Dvr.GcId; */
    gc =  vw->dvr_viewer.Dvr.GcId;
    gcv = vw->dvr_viewer.Dvr.gcv;

    /*
    **	Pull the image pixmap out of our hat
    */
    x_image = (XImage *) rendering->type_spec_data.xlib.ximage;

    /*
     * Get coordinates of upper left corner of the image relative
     * to the upper left corner of the viewer window, in pixels.
     */
    x_coord = object_private->x_pos - x_window_top;
    y_coord = object_private->y_pos - y_window_top;

    if ( image_private->img_polarity == ImgK_ZeroMinIntensity)
	{
    	XSetForeground (x_display, gc,  gcv.background );
    	XSetBackground (x_display, gc,  gcv.foreground );
	}

    /*
    **	Copy the image into the window.  Place it at the correct coordinates
    **  so it fits in with the rest of the document page.
    */
    if( rendering->type == Ids_XImage)
     {
     XPutImage(x_display,		    /* display id	    */
	      x_window,			    /* window id	    */
	      gc,			    /* display graphics context */
	      x_image,				    /* X image structure    */
	      0,				    /* Src X coordinate	    */
	      0,				    /* Src Y coordinate	    */
	      x_coord,				    /* Dst X coordinate	    */
	      y_coord,				    /* Dst Y coordinate	    */
	      x_image->width,			    /* Width of image	    */
	      x_image->height);			    /* Height of image	    */
     }
    else if(  rendering->type == Ids_Pixmap
	    && rendering->type_spec_data.xlib.pixmap != 0 )
     {
       if( x_image->format == ZPixmap && rendering->type_spec_data.xlib.pixmap != 0)
        XCopyArea( x_display,
                    rendering->type_spec_data.xlib.pixmap,
                    x_window,
                    x_graphics_ctx,
                    0,0,		/* source image origin - use whole thing */
                    x_image->width,
                    x_image->height,
		    x_coord, y_coord);	/* draw it at the appropriate place in */
					/* the current window */
       else
	/* Black and white */
        XCopyPlane( x_display,
                    rendering->type_spec_data.xlib.pixmap,
                    x_window,
                    x_graphics_ctx,
                    0,0,	    /* Source image origin */
		    x_image->width,
                    x_image->height,
                    x_coord, y_coord, /* Destination image origin */
		    1);		    /* Copy plane 1 */
     }

    if ( image_private->img_polarity == ImgK_ZeroMinIntensity)
	{
    	XSetForeground (x_display, gc,  gcv.foreground );
    	XSetBackground (x_display, gc,  gcv.background );
	}

  return (DVR_NORMAL);

} /* end dvr_put_image_to_screen */

/*
 * FUNCTION:
 *      Draw the arrowheads.
 * INPUTS:
 *      vw = pointer to vw stuct
 *      arrow = specifies what type of arrowhead(s) to draw
 *      head1 = struct containing points for first arrowhead
 *      head2 = struct containing points for second arrowhead
 *      arrow_width = specifies width of arrowhead(s)
 * RESULTS:
 *      Updates vals off of gcv as needed
 *      Draws arrrows in the window
 * NOTES:
*/
void
dvr_drawarrowheads(vw, gattr, arrow, head1, head2, arrow_width)
DvrViewerWidget  vw;
GAT              gattr;
int              arrow;
PTH		 head1;
PTH		 head2;
int              arrow_width;

{
Window          win;
Display         *dpy;
GC              gc;
XGCValues       gcv;          /* shadows what's in gc */
DvrStructPtr    dvr_ptr = &(vw->dvr_viewer.Dvr);
int             width;
boolean         visible;      /* If object is not visible, don't draw it */
int		i;
XPoint          arrow_head1[3];
XPoint          arrow_head2[3];
signlong	* head1_ptr;
signlong	* head2_ptr;

gcv = dvr_ptr->gcv;
win = XtWindow(vw->dvr_viewer.work_window);
dpy = XtDisplay(vw);
gc =  dvr_ptr->GcId;

if ( head1 != NULL)
   head1_ptr = &(head1->pth__specific.pth__xy.pth_first_x );

if ( head2 != NULL)
   head2_ptr = &(head2->pth__specific.pth__xy.pth_first_x );

for ( i = 0; i < 3; i++)
   {
   if ( head1 != NULL)
      {
      arrow_head1[i].x = * head1_ptr++ - dvr_ptr->Xtop;
      arrow_head1[i].y = * head1_ptr++ - dvr_ptr->Ytop;
      }
   if ( head2 != NULL)
      {
      arrow_head2[i].x = * head2_ptr++ - dvr_ptr->Xtop;
      arrow_head2[i].y = * head2_ptr++ - dvr_ptr->Ytop;
      }
   }

switch (arrow)
   {
   case START_UNFILLED_ARROW:
   case END_UNFILLED_ARROW:
       XSetLineAttributes (dpy, gc, arrow_width, gcv.line_style,gcv.cap_style,
                                            gcv.join_style);
       XDrawLines(dpy,win,gc,arrow_head1,3,CoordModeOrigin);
       break;
   case START_FILLED_ARROW:
   case END_FILLED_ARROW:
       dvr_output_pattern (vw, gattr->gat_line_msk_patt_ptr,&visible);
       XFillPolygon(dpy,win,gc,arrow_head1,3,Complex,CoordModeOrigin);
        break;
   case BOTH_UNFILLED_ARROWS:
       XSetLineAttributes (dpy, gc, arrow_width, gcv.line_style,gcv.cap_style,
                                            gcv.join_style);
       XDrawLines(dpy,win,gc,arrow_head1,3,CoordModeOrigin);
       XDrawLines(dpy,win,gc,arrow_head2,3,CoordModeOrigin);
       break;
   case BOTH_FILLED_ARROWS:
       dvr_output_pattern (vw, gattr->gat_line_msk_patt_ptr,&visible);
       XFillPolygon(dpy,win,gc,arrow_head1,3,Complex,CoordModeOrigin);
       XFillPolygon(dpy,win,gc,arrow_head2,3,Complex,CoordModeOrigin);
       break;
   }
   width = CONVERT_CP_TO_PIXEL(gattr->gat_line_width,vw->dvr_viewer.x_dpi);
   XSetLineAttributes (dpy, gc, width, gcv.line_style,gcv.cap_style,
                                            gcv.join_style);

}                                /* end routine */


/*****************************************************************************
 * FUNCTION:
 *      reset gc
 * INPUTS:
 *      vw = Pointer to vw stuct
 * RESULTS:
 *      Resets only  Foreground, Background and Fill Values
 * NOTES:
 *****************************************************************************
*/
void
dvr_reset_gc (vw)
DvrViewerWidget  vw;

{
DvrStructPtr	dvr_ptr = &(vw->dvr_viewer.Dvr);
Display		* dpy;		/* Standard X stuff */
Window		win;  		/* Standard X stuff */
GC		gc;		/* Where the new X state is set */
XGCValues	gcv;		/* shadows what's in gc */

    gcv = dvr_ptr->gcv;
    win = XtWindow(vw->dvr_viewer.work_window);
    dpy = XtDisplay(vw);
    gc =  dvr_ptr->GcId;

    /* for temporary solution, let gcv.fore,back always stay same.
    * always force a reset.  We'll work a better strategy out later.
    */

    gcv.foreground =  vw->manager.foreground;
    gcv.background =  vw->core.background_pixel;
    gcv.fill_style = FillSolid;

    XSetFillStyle(dpy, gc, FillSolid);
    XSetForeground (dpy, gc,  gcv.foreground );
    XSetBackground (dpy, gc,  gcv.background );
}
