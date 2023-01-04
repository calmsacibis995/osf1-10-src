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
#define Module DVR_CREX
#define Ident  "V02-064"

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
**	This is the entry level module for creating DDIF viewer widgets.
**	It contains all the routines required for a widget.
**
** ENVIORMNET:
**	VMS DECwindows and ultrix uws
**
** AUTHORS:
**      Dennis McEvoy, 15-Nov-1987
**
**
** MODIFIED BY:
**
**	V02-001		DAM0001		Dennis A. McEvoy	29-Mar-1989
**		Add code for widget to use UIL
**			DAM0002		Dennis McEvoy		9-May-1989
**		change NOWIDGET_UIL to DECWINDOWS_V1;
**		use DECW V2 feature: insensitive pixmap pushbuttons;
**		lookup ultrix UID file using enviornment variable if defined;
**			DAM0003		Dennis McEvoy		17-may-89
**		do not call dvr__get_dpi() for decw v2 (fixed);
**		make sure pixmap pushbuttons are same size as text
**		pushbuttons (for v2)
**			DAM0004		Dennis McEvoy		18-may-89
**		use font units for goto-dialog box (DECW V1)
**	V02-002		DAM0002		Dennis A. McEvoy	26-jul-1989
**		check status back from button callbacks
**	V02-003		DAM0003		Dennis A. McEvoy	29-sep-1989
**		return status from dvr_goto_page_proc()
**	V02-004		DAM0004		Dennis A. McEvoy	19-oct-1989
**		add smarts for viewing postscript
**	V02-005		DAM0005		Dennis A. McEvoy	22-feb-1990
**		add smarts for dealing with button-row, page number resources
**	V02-006		DAM0006		Dennis A. McEvoy	05-mar-1990
**		changes for OS/2 port
**	V02-007		DAM0007		Dennis A. McEvoy	06-apr-1990
**		handle doc info buffer within the widget; widget
**	        now allocates and frees doc info buffer. (previous
**		version had widget allocate it, and application free it)
**	V02-008		DAM0008		Dennis A. McEvoy	25-apr-1990
**		take out unnecessary use of DwtCallbackStructPtr
**	V02-009		DAM0009		Dennis A. McEvoy	27-apr-1990
**		fix parameter def for DvrViewerFile()
**	V02-010		DAM0010		Dennis A. McEvoy	29-may-1990
**		check for existing doc info before redoing
**	V02-011		DAM0011		Dennis A. McEvoy	12-jul-1990
**		merge in micky's changes for new callback support
**	       		MB00001		Micky Balladelli        19-Apr-1990
**		add expose events callback
**	       		MB00002		Micky Balladelli        19-Apr-1990
**		fill dvr_reason.event with the passed event.
**	       		MB00003		Micky Balladelli        25-Apr-1990
**		add mouse motion events callback
**	       		MB00004		Micky Balladelli        09-May-1990
**		add button events callback
**	       		MB00005		Micky Balladelli        15-Jun-1990
**		add scroll bar events callback
**	V02-012		DAM0012		Dennis A. McEvoy	16-jul-1990
**		add support for non 1:1 dpi pixel displays
**	V02-013		SJM0000		Stephen Munyan		21-Jun-1990
**		conversion to Motif
**	V02-014		SJM0000		Stephen Munyan		 9-Oct-1990
**		merge in CBR changes from Charlie Chan
**	V02-015		SJM0000		Stephen Munyan		 2-Nov-1990
**		change from XtAddActions to XtAppAddActions
**	V02-016		SJM0000		Stephen Munyan		26-Nov-1990
**		changed from using XQueryExtension to XListExtensions
**		in order to prevent the XDPS extension from being loaded
**		until it's needed.
**
**	V02-017		SJM0000		Stephen Munyan		28-Nov-1990
**		Merge in proto fixes from XUI
**
**		V02-013		JJT0001		Jeff Tancill		27-Nov-1990
**			fix protos
**		V02-014		DAM0001		Dennis McEvoy		28-Nov-1990
**						Steve Munyan
**			fix protos
**	V02-018		DAM0001		Dennis McEvoy		03-dec-1990
**		remove extern for dvr_scroll_action_proc (use proto)
**	V02-019		DAM0001		Dennis McEvoy		19-dec-1990
**		add font fallback/dpi check for case where running to
**		(non dec) servers with font resolution different from the
**		screen resolution
**	V02-020		DAM0001		Dennis McEvoy		03-jan-1991
**		create/destroy toolkit message handle
**	V02-021		DAM0001		Dennis McEvoy		14-jan-1991
**		modify toolkit message handle param
**	V02-022		DAM0001		Dennis McEvoy		23-jan-1991
**		fix ps ok callback
***	V02-023		DAM0001		Dennis McEvoy		05-feb-1991
**		fix help callback
**	V02-024		DAM0001		Dennis McEvoy		06-feb-1991
**		fix end of doc callback
**	V02-025		DAM0001		Dennis McEvoy		15-feb-1991
**		add actions for help/activate callbacks for
**		bulletin board children
**	V02-026		SJM000		Stephen Munyan		18-Feb-1991
**		Modified compress expose parameter in the widget to
**		have the intrinsics compress expose events to the maximum
**		value allowed in X11R4.
**	V02-027		KMRK		Kathy Robinson		23-Feb-1991
**		Free comp. strings used to set args
**	V02-028		DAM0001		Dennis McEvoy		01-mar-1991
**		convert to new typedefs
**	V02-029		DAM0001		Dennis McEvoy		04-mar-1991
**		cleanup new typedefs
**	V02-030		RTG0001		Dick Gumbel		05-mar-1991
**		cleanup #include's
**	V02-031		DAM0001		Dennis McEvoy		08-mar-1991
**		fix problem with font lookup
**	V02-032		DAM0001		Dennis McEvoy		13-mar-1991
**		fix destroy bug
**	V02-033		DAM0001		Dennis McEvoy		20-mar-1991
**		improve font lookup performance
**	V02-034		DAM0001		Dennis McEvoy		28-mar-1991
**		changes to start using Xm scroll window widget
**		for ps viewing
**	V02-035		DAM0001		Dennis McEvoy		03-apr-1991
**		cleanup typedefs
**	V02-036		DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**	V02-037		SJM0000		Stephen Munyan		08-May-1991
**		Fixed problem with widget initilization related to
**		uninitialized variables in the CBR structure which
**		hangs off the viewer widget.
**	V02-038		DAM0001		Dennis McEvoy		10-may-1991
**		remove update-callbacks from set-values proc; not needed
**		in R4.
**	V02-039		DAM0001		Dennis McEvoy		16-may-1991
**		close Mrm Hierarchy on destroy
**	V02-040		DAM0001		Dennis McEvoy		17-may-1991
**		make sure gc is created before using
**
**	V02-041		DAM0001		Dennis McEvoy		24-jun-1991
**		make sure gc is created before using (in close-file)
**
**	V02-042		SJM0000		Stephen Munyan		24-Jun-1991
**		Use new prototype-style functions defs if CDA_EXPAND_PROTO == 1
**		on cda__tag_descriptor.
**
**		Add a fake PROTO for lib$find_file to fix a compilation
**		warning from DEC C.
**	V02-043		SJM0000		Stephen Munyan		 1-Jul-1991
**		Got rid of the params and num_params parameters on the
**		dve_call_expose_callbacks since they're never used and
**		were causing problems with DEC C.
**
**	V02-044		DAM0001		Dennis McEvoy		23-jul-1991
**		remove obsolete testing #ifs
**	V02-045		DAM0001		Dennis McEvoy		05-aug-1991
**		rename headers, remove dollar signs
**	V02-046		DAM0001		Dennis McEvoy		12-aug-1991
**		remove unnecessary static, rename default statics
**	V02-047		DAM0001		Dennis McEvoy		11-sep-1991
**		fix page callback to work on sun (tag has to match uil type)
**      V02-048		JJT0001         Jeff Tancill            16-sep-1991
**              remove local XtIsRealized definition, not needed
**              anymore since NULL is defined as 0 now.
**	V02-049		DAM0001		Dennis McEvoy		01-oct-1991
**		remove extraneous scroll bar routine refs
**		clean up X routine calls to match protos
**	V02-050		DAM0001		Dennis McEvoy		18-oct-1991
**		more cleanups to match protos
**	V02-051		DAM0001		Dennis McEvoy		28-oct-1991
**		add #ifdef DVR_PSVIEW around ps viewer related calls
**		DVR_PSVIEW will be defined in dvrwint.h for vms and ultrix
**		and osf/1 but not defined for sun
**
**	V02-052		DAM0001		Dennis McEvoy		09-jan-1992
**		use exact dpi for ps to make positioning exact on servers with
**		non standard dpi
**
**	V02-053		SJM000		Stephen Munyan		 9-Jan-1992
**		Merge in changes from DEC Japan
**
**	V02-054		CJR0001		Chris Ralto		19-Feb-1992
**		Move V1 entry point routines from this module
**		into new module dvr_olde.
**
**	V02-055		ECR0001		Elizabeth C. Rust	30-Mar-1992
**		Merged in audio code.
**
**	V02-056		DAM0001		Dennis McEvoy		02-Jun-1992
**		cleanups for alpha/osf1
**
**	V02-057		KLM0001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**
**	V02-058		RDH001		Don Haney		08-Jul-1992
**		Add casts required for strict checking in OSF1/ALPHA
**
**	V02-059		RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>
**
**	V02-060		SJM0000		Stephen Munyan		22-Jul-1992
**		Changed all occurances of sts to CDAstatus
**
**	V02-061		ECR0000		Elizabeth C. Rust	17-Aug-1992
**		Implemented 'name' override parameter for DvrViewerCreate.
**
**	V02-062		JJT0002		Jeff Tancill		18-Sep-1992
**		Unconditionalize conversion to lowercase of PS format name
**		in DvrViewerFile.
**
**      V02-063		SAK001          Sarah Keating           1-Jul-1993
**	    The dvr_viewer.num_fonts field should have been declared int,
+*	    but since it's long, initializing it to zero works around the 
**	    problem.
+*	    Since XListFonts() expects a pointer to int, the top half of 
**	    num_fonts is not written and may contain random garbage.
+*	    The storage passed as the 'actual_count_return' is cleared 
**          before each call to XListFonts().
**
**	V02-064		RJD001		Ronan Duke      30-Aug-1992
**		Include Xm/MAnagerP.h if linking against Motif V1.2
*--
*/

/*
**
**  INCLUDE FILES
**
**/
#include <cdatrans.h>

#ifdef __vms__

#include <dvrint.h>				/* DVR internal definitions */
#include <descrip.h>

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */
#endif

#include <Xm/Xm.h>				/* Motif definitions */
#include <Xm/Text.h>				/* Text Widget definitions */

/* RD:
 * if linking against V1.2 of Motif then need to explicitly include 
 * Xm/ManagerP.h to get XmManagerPart definition
 */
#ifdef MOTIF_V12
#include <Xm/ManagerP.h>	
#endif

#include <DXm/DECspecific.h>			/* DXm Specific definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */
#include <Mrm/MrmPublic.h>			/* Motif Resource Manager public defintions */

#ifdef __vms__
#pragma standard				/* turn /stand=port back on */

#include <rms.h>
#include <stat.h>

#endif

#ifdef __unix__
#include <dvrint.h>				/* DVR internal definitions */

#include <sys/stat.h>

#endif

#include "dvrwdef.h"			/* Public Defns w/dvr_include */
#include "dvrwint.h"			/* Viewer Constants and structures  */
#include "dvrwptp.h"			/* prototypes for dvr */

#ifdef DVR_PSVIEW
#include <psviewwg.h>
#endif

/*
 * Prototypes for DEC C since we can't use lib$routines.h or starlet.h
 * yet since they don't yet work with DEC C.  Hopefully this limitation
 * will be raised by the time we ship so we can remove this kludge.
*/

#ifdef __vms__

PROTO(CDAstatus LIB$FIND_FILE, ());

#endif



/*
 * The following prototype is being added to work around a bug in
 * INTRINSIC.H.  Once the problem has been corrected, this code should
 * be removed.
*/

PROTO(void XtResizeWindow, (Widget));



/*
**
**  EXTERNAL and GLOBAL SPECIFIERS
**
**/

#ifdef DVR_PSVIEW
/* routine for registering classes of PS viewer widgets */

extern void 		PSViewInitializeForDRM();
#endif


/*  macros for computing layout of children  */

#define Double(x)	((x) << 1)
#define Half(x)		((x) >> 1)

#define TotalWidth(w)   (XtWidth  (w) + Double (XtBorderWidth (w)))
#define TotalHeight(w)  (XtHeight (w) + Double (XtBorderWidth (w)))

/*  macro for determining if geometry mode has changed  */

#define GeoMode(r) ((r)->request_mode)



/*
 *  CONSTANTS FOR BUTTON CALLBACKS
 */

#define k_next_pg	1
#define k_prev_pg	2
#define k_top_doc	3
#define k_bot_doc	4
#define k_goto_pg	5

/* cbr stuff */

#define k_next_ref	101
#define k_prev_ref	102

/* end cbr stuff */

/*
 *  CONSTANTS FOR UIL
 */

#define k_work_area 	1
#define k_h_scroll      2
#define k_v_scroll	3
#define k_button_box	4
#define k_top_button	5
#define k_prev_button	6
#define k_goto_button	7
#define k_next_button	8
#define k_bot_button	9
#define k_page_label	10
#define k_goto_text	11
#define k_goto_ok	12
#define k_goto_cancel	13
#define k_top_grey	14
#define k_prev_grey	15
#define k_next_grey	16
#define k_bot_grey	17
#define k_ps_window	18
#define k_ps_scroll_w	19
/*AUDIO STUFF*/
#define k_audio_toggle  20
/*END AUDIO STUFF*/

/* cbr stuff */

#define k_next_ref_button	101
#define k_prev_ref_button	102

/* end cbr stuff */


/*---------------------------------------------------*/
/* forward declarations                              */
/*---------------------------------------------------*/

/*    Public routines.    */

/*    Private routines.    */

PROTO( CDAstatus dvr_free_doc_info,
    	(DvrViewerWidget) );


PROTO( void dvr_select_dpi_from_fonts,
	(DvrViewerWidget,
    	 char **,
    	 int) );

PROTO( void dvr_get_dpi_and_fonts,
	(DvrViewerWidget) );

PROTO( CDAstatus dvr_goto_page_proc,
	(DvrViewerWidget) );

static Boolean 		dvr_widget_check_for_xdps();
static void 		dvr_managed_set_changed();
static void		dvr_call_regular_callbacks();
static void		dvr_call_motion_callbacks();
static void		dvr_call_buttons_callbacks();
       void		dvr_call_scroll_bar_callbacks();
static void		dvr_call_help_callbacks();
static void		dvr_expose_in_window();
static void		dvr_initialize();
static void		dvr_realize ();
static void		dvr_destroy();
static Boolean		dvr_set_values();
static Boolean 		dvr_accept_focus();
static XtGeometryResult	dvr_geometry_manager();
static void		dvr_change_window_geometry();
       void		dvr_add_table();
static void		dvr_remove_table();
DvrViewerWidget 	dvr_find_viewer_widget ();

/* callback procs for buttons within widget */

static void		dvr_page_cb_proc();
static void		dvr_goto_entered_proc();
static void		dvr_message_cb_proc();

static void		dvr_call_help_proc ();
static void		dvr_create_proc ();
static void		dvr_cancel_view_proc ();

Widget 			DvrWindow();
DvrViewerWidget 	DvrViewerCreateDRM();


/*---------------------------------------------------*/
/* default event bindings                            */
/*---------------------------------------------------*/

/*  translations: call help on help-mb1;
 *	       	  call regualr callbacks on mb1 down or end
 *		      of document;
 */
static char  dvr_translations [] =
    	"@Help<Btn1Down>:       DVR_HELP()\n\
	 <Btn1Up>:	 DVR_ACTIVATE()";

/*  action routines for translations  */
static XtActionsRec dvr_actions [] =
    {
    	{"DVR_HELP",	(XtActionProc)dvr_call_help_callbacks},
	{"DVR_ACTIVATE",(XtActionProc)dvr_call_regular_callbacks},
	{"DVR_BUTTONS", (XtActionProc)dvr_call_buttons_callbacks},
	{"DVR_MOTION",  (XtActionProc)dvr_call_motion_callbacks},
        {NULL, 		NULL}
    };

static Boolean 	dvr_default_v_scroll = TRUE;
static Boolean 	dvr_default_h_scroll = TRUE;
static int     	dvr_default_proc_ops =
		    DvrWordWrap | DvrSoftDirectives | DvrLayout | DvrSpecificLayout;
static Dimension dvr_default_width    = 684;
static Dimension dvr_default_height   = 723;
static int	dvr_default_x        = 0;
static int 	dvr_default_y	 = 0;
static int	dvr_default_paper_width  = 0;
static int	dvr_default_paper_height = 0;
static Boolean  dvr_default_button_box = TRUE;
static int	dvr_default_page_number = 0;

/* new postscript resources */
static Boolean 	dvr_default_use_comments = TRUE;
static Boolean 	dvr_default_use_bitmaps  = FALSE;
static Boolean  dvr_default_use_trays    = TRUE;
static Boolean  dvr_default_watch_progress = FALSE;
static Boolean	dvr_default_header_required = TRUE;

static int	dvr_default_orientation = 0;
static int	dvr_default_scale_value = 10;

/*---------------------------------------------------*/
/* widget resources                                  */
/*                                                   */
/* these are the resources (attributes) that the     */
/* widget supports                                   */
/*---------------------------------------------------*/

static XtResource dvr_resources[] =
    {

    /* viewer widget specific resources */

    /* activate callback */
    {XmNactivateCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
	 XtOffset (DvrViewerWidget, dvr_viewer.callback),
	 XtRCallback, NULL},

    /* scroll bar callback */
    {DvrNscrollBarCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
	 XtOffset (DvrViewerWidget, dvr_viewer.scroll_bar_callback),
	 XtRCallback, NULL},

    /* expose callback */
    {XmNexposeCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
    	 XtOffset (DvrViewerWidget, dvr_viewer.expose_callback),
	 XtRCallback, NULL},

    /* mouse motion callback */
    {DvrNmouseMotionCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
    	 XtOffset (DvrViewerWidget, dvr_viewer.mouse_motion_callback),
	 XtRCallback, NULL},

    /* buttons callback */
    {DvrNbuttonsCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
    	 XtOffset (DvrViewerWidget, dvr_viewer.buttons_callback),
	 XtRCallback, NULL},

    /* horizontal scroll boolean */
    {DvrNscrollHorizontal, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.h_scroll_flag),
	 XtRBoolean, (caddr_t) &dvr_default_h_scroll},

    /* vertical scroll boolean */
    {DvrNscrollVertical, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.v_scroll_flag),
	 XtRBoolean, (caddr_t) &dvr_default_v_scroll},

    /* processing Options bitmast */
    {DvrNprocessingOptions, XtCValue, XtRInt, sizeof(int),
	 XtOffset (DvrViewerWidget, dvr_viewer.processing_options),
	 XtRInt, (caddr_t) &dvr_default_proc_ops},

    /* width */
    {XmNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	 XtOffset (DvrViewerWidget, core.width),
	 XtRDimension, (caddr_t) &dvr_default_width},

    /* height */
    {XmNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	 XtOffset (DvrViewerWidget, core.height),
	 XtRDimension, (caddr_t) &dvr_default_height},

    /* paper width */
    {DvrNpaperWidth, XtCValue, XtRInt, sizeof(int),
     	 XtOffset (DvrViewerWidget, dvr_viewer.paper_width),
     	 XtRInt, (caddr_t) &dvr_default_paper_width},

    /* paper height */
    {DvrNpaperHeight, XtCValue, XtRInt, sizeof(int),
     	 XtOffset (DvrViewerWidget, dvr_viewer.paper_height),
     	 XtRInt, (caddr_t) &dvr_default_paper_height},

    /* orientation (postscript) */
    {DvrNorientation, XtCValue, XtRInt, sizeof(int),
     	 XtOffset (DvrViewerWidget, dvr_viewer.Ps.orientation),
     	 XtRInt, (caddr_t) &dvr_default_orientation},

    /* scale value (postscript) */
    {DvrNscaleValue, XtCValue, XtRInt, sizeof(int),
     	 XtOffset (DvrViewerWidget, dvr_viewer.Ps.scale_value),
     	 XtRInt, (caddr_t) &dvr_default_scale_value},

    /* use comments boolean (postscript) */
    {DvrNuseComments, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.Ps.use_comments),
	 XtRBoolean, (caddr_t) &dvr_default_use_comments},

    /* use bitmaps boolean (postscript) */
    {DvrNuseBitmaps, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.Ps.use_bitmaps),
	 XtRBoolean, (caddr_t) &dvr_default_use_bitmaps},

    /* use "fake" trays boolean (postscript) */
    {DvrNuseTrays, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.Ps.use_trays),
	 XtRBoolean, (caddr_t) &dvr_default_use_trays},

    /* watch progress boolean (postscript) */
    {DvrNwatchProgress, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.Ps.watch_progress),
	 XtRBoolean, (caddr_t) &dvr_default_watch_progress},

    /* header required boolean (postscript) */
    {DvrNheaderRequired, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.Ps.header_required),
	 XtRBoolean, (caddr_t) &dvr_default_header_required},


    /* button box boolean */
    {DvrNbuttonBox, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset (DvrViewerWidget, dvr_viewer.button_box_flag),
	 XtRBoolean, (caddr_t) &dvr_default_button_box},

    /* page number */
    {DvrNpageNumber, XtCValue, XtRInt, sizeof(int),
     	 XtOffset (DvrViewerWidget, dvr_viewer.page_number),
     	 XtRInt, (caddr_t) &dvr_default_page_number},


 /*
  * X and Y commented out for now; 0 is automatically put in for default;
  * not working if we set it here;
  *
    {XmNx, XtCX, XtRPosition, sizeof(Position),
	 XtOffset (DvrViewerWidget, core.x),
	 XtRPosition, (caddr_t) &dvr_default_x},

    {XmNy, XtCY, XtRPosition, sizeof(Position),
	 XtOffset (DvrViewerWidget, core.y),
	 XtRPosition, (caddr_t) &dvr_default_y}
*/

    };



static CompositeClassExtensionRec compositeClassExtRec = {
    NULL,
    NULLQUARK,
    XtCompositeExtensionVersion,
    sizeof(CompositeClassExtensionRec),
    TRUE,
};


/* initialize the widget class rec */

#ifdef __vms__
#pragma nostandard
#endif

externaldef(dvr_viewer_class_rec) DvrViewerClassRec dvr_viewer_class_rec =

#ifdef __vms__
#pragma standard
#endif

{
    {	/* core			   */
	/* superclass		   */	(WidgetClass) & xmManagerClassRec,
	/* class_name		   */	DvrViewerClassStr,
	/* widget_size		   */	sizeof(DvrViewerWidgetRec),
	/* class_initialize	   */	NULL,
	/* chained class part init */	NULL,
	/* class_inited		   */	FALSE,

	/* initialize		   */	dvr_initialize,
	/* init hook proc	   */	NULL,

	/* realize		   */	dvr_realize,
	/* actions		   */	dvr_actions,
	/* num_actions		   */	XtNumber(dvr_actions),

	/* resources		   */	dvr_resources,
	/* num_resources	   */	XtNumber(dvr_resources),
	/* xrm_class		   */	NULLQUARK,

	/* compress_motion	   */	TRUE,
	/* compress_exposure	   */	(XtExposeCompressMultiple | XtExposeGraphicsExposeMerged), /* Was TRUE prior to X11R4 */
	/* compress enter-leave	   */	TRUE,
	/* visible_interest	   */	TRUE,

	/* destroy		   */	dvr_destroy,
	/* resize		   */	(XtWidgetProc) dvr_resize,
	/* expose		   */	NULL,

	/* set_values		   */	dvr_set_values,
	/* set values hook proc	   */	NULL,
	/* set values almost proc  */	XtInheritSetValuesAlmost,

	/* get values hook proc	   */	NULL,

	/* accept_focus		   */	dvr_accept_focus,

	/* current version	   */	XtVersion,

	/* callback offset	   */	NULL,
	/* default trans	   */	dvr_translations,

	/* Query geom proc	   */	NULL,
	/* Display Accelerator	   */	NULL,
	/* extension		   */	NULL,
    },

    {	/* composite class record  */
	/* childrens geo mgr proc  */	dvr_geometry_manager,		/* was XtInheritGeometryManager,	*/

	/* set changed proc	   */	XtInheritChangeManaged,

	/* add a child		   */	XtInheritInsertChild,
	/* remove a child	   */	XtInheritDeleteChild,
	/* extension		   */	(XtPointer)&compositeClassExtRec,
    },

    {	/* constraint class record */
	/* no additional resources */	NULL,
	/* num additional resources*/	0,
	/* size of constraint rec  */	0,
	/* constraint initialize   */	NULL,
	/* constraint destroy	   */	NULL,
	/* constraint setvalue	   */	NULL,
	/* constraint extension	   */	NULL,
    },

    {   /* XmManager class record  */
	/* default translations	   */	XtInheritTranslations,
	/* get resources	   */	NULL,				/* I'm not sure what there are	*/
	/* num get resources	   */	0,				/* but Jim VanGilder thought we	*/
	/* get_cont_resources	   */	NULL,				/* could get away with making	*/
	/* num get_const_resources */	0,				/* them NULL or 0.		*/
	/* parent process	   */	XmInheritParentProcess,
	/* extension		   */	NULL,
    },

    {	/* viewer class record	   */
	/* just a dummy field	   */	0,
    }
};

#ifdef __vms__
#pragma nostandard
#endif

externaldef(dvr_viewer_class_rec_addr) DvrViewerWidgetClass dvr_viewer_class_rec_addr =
	    (DvrViewerWidgetClass) &dvr_viewer_class_rec;

#ifdef __vms__
#pragma standard
#endif


/*
**++
**  ROUTINE NAME:
**	dvr_handle_nonmaskable_event(w, client_data, eventP)
**
**  FUNCTIONAL DESCRIPTION:
**	event handler for nonmaskable events.  In particular,
**	we are interested in GraphicsExpose events.  These are events which
**	do not generate a normal expose event, but still generate an area
**	to be repainted. In our case, this happens when the user scrolls the
**	window, and we move the area up using XCopyArea and there is a window
**	partially obscuring our work window. When the area is copied up, we
**	have to do a manual repaint of the area that was obscured by the
**	the other window.
**
**  FORMAL PARAMETERS:
**    	Widget 	w;
**    	int 	client_data;
**    	XEvent 	*eventP;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_handle_nonmaskable_event(w, client_data, eventP)
    Widget 	w;
    int 	client_data;
    XEvent 	*eventP;
{
    if (eventP->xany.type == GraphicsExpose)
      {
	/*  need to pass viewer widget to display routine, w is the
	 *  work area child of the viewer widget; the client_data is
	 *  the address of the actual viewer widget.
         */
	DvrViewerWidget vw = (DvrViewerWidget) client_data;

	dvr_display_page(vw,
	    eventP->xgraphicsexpose.x,
	    eventP->xgraphicsexpose.y,
	    eventP->xgraphicsexpose.x + eventP->xgraphicsexpose.width,
	    eventP->xgraphicsexpose.y + eventP->xgraphicsexpose.height);

        /*
        **  Invoke the application specific callback routine for the expose
        **  event
        */
	if (vw->dvr_viewer.expose_callback != NULL)				/* was .ecallback in XUI */
	    dvr_call_expose_callbacks(vw, (XExposeEvent *)eventP);
      }

}




/*
**++
**  ROUTINE NAME:
**	dvr_initialize (request, new)
**
**  FUNCTIONAL DESCRIPTION:
** 	The children for the viewer widget are created here.
** 	This includes the scroll bars (if requested), the work area,
**	the button box and all of it's children.
**
**  FORMAL PARAMETERS:
**	Widget   request;	 as build from arglist
**	Widget   new;		 after superclasses init
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_initialize (request, new)
    Widget   request;				/* as build from arglist */
    Widget   new;				/* after superclasses init */
{
    DvrViewerWidget vw = (DvrViewerWidget) new;

    int 	    argcount = 0;
    Arg 	    arg_list[20];
    Boolean   	    h_scroll_flag;
    Boolean   	    v_scroll_flag;
    int		    p_options;
    Widget	    manage_list[10];
    int 	    manage_count = 0;
    char	    dvr_init_page_str[100];
    XtCallbackRec     cb_list[2];
    float	    dpi_float;
    char	    *name;
    int		    opcode, event, error;
    DvrCdaItemList  item_list[8];
    CDAconstant	    item_count = 0L;

    /*
     *  clear out the fields not yet used in the Viewer Part
     *  of the widget
     */

    Work(vw) 	= NULL;
    Hscr(vw) 	= NULL;
    Vscr(vw) 	= NULL;
    Bbox(vw) 	= NULL;
    TopBut(vw) 	= NULL;
    PrevBut(vw) = NULL;
    GotoBut(vw) = NULL;
    NextBut(vw) = NULL;
    BotBut(vw) 	= NULL;
    PageLabel(vw) = NULL;

    GotoDialog(vw) = NULL;
    Message(vw) = NULL;

/* BEGIN AUDIO */
#ifdef CDA_AUDIO_SUPPORT
    AudioTog(vw) = NULL;
#endif
/* END AUDIO */

/* cbr stuff */

    PrevRef(vw) = NULL;
    NextRef(vw) = NULL;

/* end cbr stuff */

    PSscroll(vw) = NULL;
    PSvbar(vw) = NULL;
    PShbar(vw) = NULL;

    PSwin(vw) = NULL;
    CancelBut(vw) = NULL;
    ScaleLbl(vw) = NULL;


    vw->dvr_viewer.ViewerCursor = (Cursor) NULL;
    vw->dvr_viewer.default_font_id = (unsigned long) NULL;
					    /* initialize so we won't try   */
					    /* to unload font if it never   */
					    /* gets loaded.		    */

    vw->dvr_viewer.cursor_ref_count = 0;
    vw->dvr_viewer.doc_info_text_buffer = 0;

    /* initialize font index to undefined */
    vw->dvr_viewer.current_font_index = UNDEFINED_FONT_INDEX;

    /* set destroy flag to false */
    vw->dvr_viewer.destroy_flag = FALSE;

    memset ( &(vw->dvr_viewer.Dvr),
	     (int) NULL,
	     sizeof(DvrStruct) );

/* cbr stuff */

    memset (&(vw->dvr_viewer.Cbr),
    	    (int) NULL,
	    sizeof(CbrStruct) );

/* end cbr stuff */

    /*  ask server is it has display postscript extension; this will
     *  determine whether or not we allow ps viewing;
     *  allow MIT registered string or previously used DECwindows
     *  string
     */
    vw->dvr_viewer.Ps.dps_exists = dvr_widget_check_for_xdps(vw);

    /* create cda toolkit message log handle */
    MAKE_ITEM(item_list, item_count, CDA_MESSAGE_OUTPUT_PROCEDURE,
	      0, dvr_toolkit_message_routine);
    item_count++;

    MAKE_ITEM(item_list, item_count, CDA_MESSAGE_OUTPUT_PROC_PARM,
	sizeof(DvrViewerWidget), &vw);
    item_count++;

    MAKE_ITEM(item_list, item_count, CDA_MESSAGE_OUTPUT_PROC_BUFFER,
	      512, vw->dvr_viewer.message_buffer);
    item_count++;

    MAKE_ITEM(item_list, item_count, 0, 0, 0);
    item_count++;

    vw->dvr_viewer.Dvr.WidgetStatus = CdaCreateMessageLog
		((CDAitemlist *) item_list, &(vw->dvr_viewer.message_handle));

    if (vw->dvr_viewer.Dvr.WidgetStatus == CDA_NORMAL)
        vw->dvr_viewer.Dvr.WidgetStatus = DVR_NORMAL;

}


/*
**++
**  ROUTINE NAME:
**	dvr_widget_check_for_xdps (vw)
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function returns a boolean value which indicates
**	if the XDPS extension is available on the server.
**
**  FORMAL PARAMETERS:
**	Widget	vw		Widget associated with the display
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	Boolean			A boolean that indicates if XDPS is available
**
**  SIDE EFFECTS:
**	none
**--
**/

static Boolean dvr_widget_check_for_xdps (vw)
    Widget   vw;				/* widget associated with a window	*/

{
    char	**list_of_extensions;		/* array of extension name strings	*/
    char	**extension_pointer;		/* pointer into the list of extensions	*/
    int		nextensions_return;		/* number of extensions returned	*/
    int		i;				/* loop index				*/
    Boolean	return_status;			/* return status			*/

    /*
     * Initially assume that we weren't able to locate the XDPS extension
    */

    return_status = FALSE;

    /*
     * Ask the server for the list of known extensions available.
    */

    list_of_extensions = XListExtensions(XtDisplay(vw), 			/* Display associated with the server	*/
					 &nextensions_return);			/* Number of extensions returned	*/

    extension_pointer = list_of_extensions;

    for ( i = 0; i < nextensions_return; i++)
	{
	    /* Check for both the standard name and the name that was
	     * used back when DPS was shipped as a seperate layered
	     * product on VMS V5.3.
	    */

	    if ((strcmp(*extension_pointer, "Adobe-DPS-Extension") == 0) || 	/* Standard XDPS extension name  */
		(strcmp(*extension_pointer, "DPSExtension") == 0))		/* VMS V5.3 Layered product name */
		{
		    /*
		     * We found a match.  As a result we indicate success and
		     * terminate the scan.
		    */

		    return_status = TRUE;
		    break;
		};

	    extension_pointer++;
	};

    /*
     * Once we've finished scanning the list of extensions return
     * the memory allocated by XListExtensions.
    */

    XFreeExtensionList(list_of_extensions);

    return (return_status);
}


/*
**++
**  ROUTINE NAME:
**	dvr_managed_set_changed (w)
**
**  FUNCTIONAL DESCRIPTION:
** 	every widget requires a managed_set_changed proc;
** 	ours does not do anything; once the children of the
**	viewer widget are managed, they do not change their
**	managed state.
**
**  FORMAL PARAMETERS:
**	Widget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_managed_set_changed(w)
    Widget w;
{

/*
 *  dont think we need one
 */

}



/*
**++
**  ROUTINE NAME:
**	dvr_accept_focus (vw)
**
**  FUNCTIONAL DESCRIPTION:
** 	this procedure is called when an applicaiton tries to give
** 	focus to our widget; an attempt is made to get input focus
**	for the work area of the viewer widget;
**
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static Boolean dvr_accept_focus (vw, time)
    DvrViewerWidget vw;
    Time *time;

{

    if ((Work (vw) != NULL) && XtIsManaged (Work (vw)) &&
	((Work (vw))->core.widget_class->core_class.accept_focus != NULL))
	(* (Work (vw))->core.widget_class->core_class.accept_focus) ((Work (vw)), time);

}



/*
**++
**  ROUTINE NAME:
** 	dvr_set_values(old,new)
**
**  FUNCTIONAL DESCRIPTION:
** 	this routine detects differences in two versions
** 	of a widget, when a difference is found the
** 	appropriate action is taken.  It is called when the
**	user calls the public routine XtSetvalues on the widget;
**
**  FORMAL PARAMETERS:
**	Widget old, new;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
static Boolean dvr_set_values (current, request, new_one)
    Widget   current;				/* original widget */
    Widget   request;				/* as modif by arglist */
    Widget   new_one;				/* as modif by superclasses */
{
    DvrViewerWidget old = (DvrViewerWidget) current;
    DvrViewerWidget new = (DvrViewerWidget) new_one;
    Widget 	    unmanage_list[5];
    Widget          manage_list[5];
    int 	    unmanage_count = 0;
    int	   	    manage_count = 0;
    Arg		    ps_arg_list[3];
    int		    ps_arg_count = 0;

    /*  need to handle our setable resources;
     *	the callbacks and processing options.
     */

    /*
    **  Check the callbacks lists and invoke dvr_add_table
    **  with the appropriate translation table
    */
    if (new->dvr_viewer.mouse_motion_callback != NULL &&			/* was .ecallback in XUI */
        new->dvr_viewer.buttons_callback != NULL)				/* was .ecallback in XUI */
    {
	dvr_add_table( Work(new)
		     , dvr_buttons_translations_with_motion
		     );

    } else if ( new->dvr_viewer.mouse_motion_callback != NULL &&		/* was .ecallback in XUI */
		new->dvr_viewer.buttons_callback == NULL)			/* was .ecallback in XUI */
    {
	dvr_add_table( Work(new)
		     , dvr_translations_with_motion
		     );

    } else if ( new->dvr_viewer.mouse_motion_callback == NULL &&		/* was .ecallback in XUI */
	        new->dvr_viewer.buttons_callback != NULL)			/* was .ecallback in XUI */
    {
	dvr_add_table( Work(new)
		     , dvr_buttons_translations
		     );

    }
    /*
    **  Restore the default translation table if no callback list
    **  have been defined for motion and buttons events
    */
    else if ( new->dvr_viewer.mouse_motion_callback == NULL &&			/* was .ecallback in XUI */
	        new->dvr_viewer.buttons_callback == NULL &&			/* was .ecallback in XUI */
		old->dvr_viewer.mouse_motion_callback != NULL &&		/* was .ecallback in XUI */
	        old->dvr_viewer.buttons_callback != NULL )			/* was .ecallback in XUI */
    {
	dvr_remove_table( Work(new) );
    }

    /* 	if user tried to changed scroll bar options,
     *	ignore for now;
     */
    if (old->dvr_viewer.h_scroll_flag != new->dvr_viewer.h_scroll_flag)
      {
        new->dvr_viewer.h_scroll_flag = old->dvr_viewer.h_scroll_flag;
      }

    if (old->dvr_viewer.v_scroll_flag != new->dvr_viewer.v_scroll_flag)
      {
        new->dvr_viewer.v_scroll_flag = old->dvr_viewer.v_scroll_flag;
      }

    /* button box is create time only */
    if (old->dvr_viewer.button_box_flag != new->dvr_viewer.button_box_flag)
      {
        new->dvr_viewer.button_box_flag = old->dvr_viewer.button_box_flag;
      }

    if (old->dvr_viewer.processing_options !=
	new->dvr_viewer.processing_options)
      {
	/* do we need to do anything special?  */
      }

    if (new->dvr_viewer.page_number != old->dvr_viewer.page_number)
	DvrGotoPage(new, new->dvr_viewer.page_number);

    if (new->dvr_viewer.Ps.dps_exists)
      /* set ps specific options iff server has postscript extension */
      {
    	/*  the paper_width fields in the viewer part of the widget are
    	 *  used for both normal cda processing and postscript processing;
    	 *  for ps processing, we need to set our ps specific window sizes
    	 *  whenever paper size changes; the actual set values on the ps
	 *  window gets done when a new file is opened.
    	 */

    	if (old->dvr_viewer.paper_width !=
	    new->dvr_viewer.paper_width)

	  {
	    /* default for ps width */
	    double pap_width = 215.90;

    	    double real_x_dpmm;

    	    /*  have to get the actual dpmm (dots per mm) from the server; the
    	     *  dpi in the dvr struct is adjusted to match the X fonts used on a
    	     *  server; for ps viewing, we need to use the exact dpi to
    	     *  position things correctly
    	     */

    	    real_x_dpmm = (double) XWidthOfScreen(XDefaultScreenOfDisplay(XtDisplay(new))) /
		 (double) XWidthMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(new)));

	    if (new->dvr_viewer.paper_width != 0)
	       pap_width = (double) new->dvr_viewer.paper_width;

	    new->dvr_viewer.Ps.window_width =
	 			   (int) (pap_width * real_x_dpmm);

	  }

    	if (old->dvr_viewer.paper_height !=
	    new->dvr_viewer.paper_height)

	  {
	    /* default for ps height */
	    double pap_height =  279.40;

    	    double real_y_dpmm;

    	    real_y_dpmm = (double) XHeightOfScreen(XDefaultScreenOfDisplay(XtDisplay(new))) /
		 (double) XHeightMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(new)));

	    if (new->dvr_viewer.paper_height != 0)
	       pap_height = (double) new->dvr_viewer.paper_height;

	    new->dvr_viewer.Ps.window_height =
				   (int)    (pap_height * real_y_dpmm);
	  }

      }
    return (TRUE);
}



/*
**++
**  ROUTINE NAME:
**	dvr_call_callbacks(w, r, *e)
**
**  FUNCTIONAL DESCRIPTION:
**      call the callbacks supplied for this widget instance using
**	the x intrinsics; supply the integer r as the reason.
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget	w;
**      int    		r;
**      XEvent 		*e;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_call_callbacks(vw, r, e)
    DvrViewerWidget	vw;
    int			r;
    XEvent		*e;
{
    DvrCallbackStruct dvr_reason;

    dvr_reason.reason = r;
    dvr_reason.status = vw->dvr_viewer.Dvr.WidgetStatus;
    dvr_reason.string_ptr = 0;

    switch( r )
    {
	case( DvrCRbuttonEvent ):
	    dvr_reason.event = e;
	    XtCallCallbacks((Widget)vw, DvrNbuttonsCallback, &dvr_reason);
	    break;
	case( DvrCRactivate ) :
	case( DvrCRendDocument ) :
	case( DvrCRcdaError ) :
	case( DvrCRpsOK ) :
	    dvr_reason.event = e;
	    XtCallCallbacks((Widget)vw, XmNactivateCallback, &dvr_reason);
	    break;
	case( DvrCRexpose ) :
	    dvr_reason.event = e;
	    XtCallCallbacks((Widget)vw, XmNexposeCallback, &dvr_reason);
	    break;
	case( DvrCRmouseMotion ) :
	    dvr_reason.event = e;
	    XtCallCallbacks((Widget)vw, DvrNmouseMotionCallback, &dvr_reason);
	    break;
	case( DvrCRhelpRequested ) :
	    dvr_reason.event = e;
    	    XtCallCallbacks((Widget)vw, XmNhelpCallback, &dvr_reason);
	    break;
    }
}


/*
**++
**  ROUTINE NAME:
**	dvr_call_regular_callbacks(w, event, params, num_params)
**
**  FUNCTIONAL DESCRIPTION:
**      call the callbacks supplied for this widget instance;
**	give DvrCRactivate as the reason; this means that
**	a mouse button 1 down event occurred in the widget;
**
**  FORMAL PARAMETERS:
**	Widget	w;
**	XButtonPressedEvent  *event;
**	char	**params;
**	int	num_params;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_call_regular_callbacks(w, event, params, num_params)
    Widget   w;
    XButtonPressedEvent  *event;
    char   **params;
    int	     num_params;
{

    DvrCallbackStruct dvr_reason;
    DvrViewerWidget   vw;

    /* get the id of the parent viewer */
    vw = dvr_find_viewer_widget (w);

    if ( !(event->state & Mod5Mask) )

	/*  if this was button up event without the help modifier,
	 *  call the application's activate callback proc;
	 */
        (void) dvr_call_callbacks(vw, DvrCRactivate, (XEvent *) event);
}


/*
**++
**  ROUTINE NAME:
**	dvr_call_motion_callbacks(w, event, params, num_params)
**
**  FUNCTIONAL DESCRIPTION:
**      call the callbacks supplied for this widget instance;
**	give DvrCRmouseMotion as the reason; this means a mouse motion
**	occured.
**
**  FORMAL PARAMETERS:
**	Widget	w;
**	XPointerEvent  *event;
**	char	**params;
**	int	num_params;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_call_motion_callbacks(w, event, params, num_params)
    Widget   w;
    XPointerMovedEvent  *event;
    char   **params;
    int	     num_params;
{
    DvrViewerWidget	vw;

    /* get the id of the parent viewer */
    vw = dvr_find_viewer_widget (w);

    /*
    **  Check the Widget id. If it is an image window widget, we
    **  need to compute the position of the event related to the
    **  work area.
    */
    if( w != Work(vw) )
    {
	event->x = XtX(w) + event->x;
	event->y = XtY(w) + event->y;
    }

    (void) dvr_call_callbacks(vw, DvrCRmouseMotion, (XEvent *) event);

}


/*
**++
**  ROUTINE NAME:
**	dvr_call_buttons_callbacks(w, event, params, num_params)
**
**  FUNCTIONAL DESCRIPTION:
**      call the callbacks supplied for this widget instance;
**	give DvrCRbuttonEvent as the reason; this means a button event
**	occured.
**
**  FORMAL PARAMETERS:
**	Widget	w;
**	XPointerEvent  *event;
**	char	**params;
**	int	num_params;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_call_buttons_callbacks(w, event, params, num_params)
    Widget   w;
    XButtonPressedEvent  *event;
    char   **params;
    int	     num_params;
{
    DvrViewerWidget	vw;

    /* get the id of the parent viewer */
    vw = dvr_find_viewer_widget (w);

    /*
    **  Check the Widget id. If it is an image window widget, we
    **  need to compute the position of the event related to the
    **  work area.
    */
    if( w != Work(vw) )
    {
	event->x = XtX(w) + event->x;
	event->y = XtY(w) + event->y;
    }
    (void) dvr_call_callbacks(vw, DvrCRbuttonEvent, (XEvent *) event);

}


/*
**++
**  ROUTINE NAME:
**	dvr_call_expose_callbacks(vw, event)
**
**  FUNCTIONAL DESCRIPTION:
**      call the callbacks supplied for this widget instance;
**	give DvrCRexpose as the reason; this means that
**	an expose event occurred in the widget;
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget		vw;
**	XExposeEvent 		*event;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_call_expose_callbacks(vw, event)
    DvrViewerWidget	vw;
    XExposeEvent  	*event;
{
    (void) dvr_call_callbacks(vw, DvrCRexpose, (XEvent *) event);

}


/*
**++
**  ROUTINE NAME:
**	dvr_call_scroll_bar_callbacks(vw, Xtop, Ytop, page_width, page_height
**				     ,win_width,win_height)
**
**  FUNCTIONAL DESCRIPTION:
**      call the callbacks supplied for this widget instance;
**	give DvrCRscrollBarEvent as the reason; this means that
**	an expose event occurred in the widget;
**
**  FORMAL PARAMETERS:
**	Widget	vw;
**	int     Xtop;
**	int     Ytop;
**      int	page_width;
**      int	page_height;
**      int	win_width;
**      int	win_height;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_call_scroll_bar_callbacks(vw, Xtop, Ytop, page_width, page_height
				  ,win_width,win_height)
    DvrViewerWidget	vw;
    int			Xtop;
    int			Ytop;
    int			page_width;
    int		        page_height;
    int			win_width;
    int			win_height;
{
    DvrScrollBarEvent event;
    XEvent	      xevent;

    event.reason      = DvrCRscrollBarEvent;
    event.Xtop	      = Xtop;
    event.Ytop	      = Ytop;
    event.page_width  = page_width;
    event.page_height = page_height;
    event.win_width   = win_width;
    event.win_height  = win_height;

    /*  fill in an X event in the structure; NOTE: it is
     *  not really a valid X event; but, this field is required
     *  for all X callback structs;
     * 	we could fill this in with the real scroll event from the
     *  DWT scrollbar widget if we restructured dvr_actions.c;
     *  for now, doesn't seem to be an issue since it is not needed
     *  by the MEMEX project
     */
    event.event	      = &xevent;

    XtCallCallbacks((Widget)vw, DvrNscrollBarCallback, &event);

}


/*
**++
**  ROUTINE NAME:
**	dvr_call_help_callbacks(w, r)
**
**  FUNCTIONAL DESCRIPTION:
**      call the help callbacks supplied for this widget instance;
**
**  FORMAL PARAMETERS:
**	Widget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_call_help_callbacks(w, event, params, num_params)
    Widget   w;
    XButtonPressedEvent  *event;
    char   **params;
    int	     num_params;
{
    DvrCallbackStruct dvr_reason;
    DvrViewerWidget   vw;

    /* get the id of the parent viewer */
    vw = dvr_find_viewer_widget (w);

    dvr_reason.reason = DvrCRhelpRequested;
    dvr_reason.status = DVR_NORMAL;
    dvr_reason.event  = (XEvent *) event;

    XtCallCallbacks((Widget)vw, XmNhelpCallback, &dvr_reason);

}




/*
**++
**  ROUTINE NAME:
**	dvr_destroy (w)
**
**  FUNCTIONAL DESCRIPTION:
** 	Destroy the widget specific data structs
**
**  FORMAL PARAMETERS:
**	Widget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_destroy (w)
    Widget w;
{
    DvrViewerWidget vw = (DvrViewerWidget) w;
    int function_code = CDA_STOP;
    CDAstatus stat;

    vw->dvr_viewer.destroy_flag = TRUE;

    /*  call cda_convert to close out widget iff a file is open */
    if (vw->dvr_viewer.Dvr.FileOpen)
      {
    	DvrCloseFile(vw);
      }

    /*
     * free available fonts list
     */
    XFreeFontNames(vw->dvr_viewer.available_fonts);

    /*
     * unload fonts and free font table
     */

    (void) dvr_unload_fonts ( vw );
    (void) dvr_dealloc_font_table ( vw );

    /*
     * free the callback lists
     */

    XtRemoveAllCallbacks ((Widget)vw, XmNactivateCallback);
    XtRemoveAllCallbacks ((Widget)vw, XmNhelpCallback);
    XtRemoveAllCallbacks ((Widget)vw, XmNexposeCallback);
    XtRemoveAllCallbacks ((Widget)vw, DvrNmouseMotionCallback);
    XtRemoveAllCallbacks ((Widget)vw, DvrNbuttonsCallback);

    /*
     * free cursor if defined
     */

    if (vw->dvr_viewer.ViewerCursor != (Cursor) NULL)
	XFreeCursor(XtDisplay(vw), vw->dvr_viewer.ViewerCursor);

    /* free allocated strings */
    XtFree(vw->dvr_viewer.page_label_str);
    XtFree(vw->dvr_viewer.of_label_str);
    XtFree(vw->dvr_viewer.default_font_str);
    XtFree(vw->dvr_viewer.Ps.ps_str);

    /* free toolkit message handle */
    if (vw->dvr_viewer.message_handle)
        vw->dvr_viewer.Dvr.WidgetStatus =
	    CdaCloseMessageLog(vw->dvr_viewer.message_handle);
    else
	vw->dvr_viewer.Dvr.WidgetStatus = CDA_NORMAL;

    /* close Mrm hierarchy */
    MrmCloseHierarchy(vw->dvr_viewer.drm_h);
}


/*
**++
**  ROUTINE NAME:
**	dvr_realize (w, window_mask, window_atts)
**
**  FUNCTIONAL DESCRIPTION:
**      This routine is called when the widget is realized;
**	The window is created for the widget here.
**
**  FORMAL PARAMETERS:
**  	Widget			w;
**    	Mask			*window_mask;
**  	XSetWindowAttributes	* window_atts;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_realize (w, window_mask, window_atts)
    Widget			w;
    Mask			*window_mask;
    XSetWindowAttributes	* window_atts;

{
    DvrViewerWidget vw = (DvrViewerWidget) w;

    if (XtIsRealized (vw))
	return;

    /* check if viewer is requested to be created without scrollbars
     * or button box; if so, get rid of them here; by default, they are
     * created in the UIL fetch
     */

    if (!vw->dvr_viewer.h_scroll_flag)
      {
	if (Hscr(vw))
	  {
	    XtUnmanageChild(Hscr(vw));
	    XtDestroyWidget(Hscr(vw));
	    Hscr(vw) = 0;
	  }
      }

    if (!vw->dvr_viewer.v_scroll_flag)
      {
	if (Vscr(vw))
	  {
	    XtUnmanageChild(Vscr(vw));
	    XtDestroyWidget(Vscr(vw));
	    Vscr(vw) = 0;
	  }
      }

    if (!vw->dvr_viewer.button_box_flag)
      {
	if (Bbox(vw))
	  {
	    XtUnmanageChild(Bbox(vw));
	    XtDestroyWidget(Bbox(vw));
	    Bbox(vw) = 0;
	  }
      }


    XtCreateWindow ((Widget)vw,
		    InputOutput,
		    CopyFromParent,
		    *window_mask,
		    window_atts);

    /* set all the areas up correctly */
    dvr_resize(vw);

}



/*
**++
**  ROUTINE NAME:
**	dvr_geometry_manager (w, request, reply)
**
**  FUNCTIONAL DESCRIPTION:
** 	Geometry manager for our children.  This routine is necessary
** 	in Motif since XmManager widgets and XmComposite widgets must
**	provide geometry managers.  Note that in XUI this wasn't necessary
**	since we were in DwtCommon which provided this function by default
**	unless we explicitly over rode the default (which we didn't).
**
**  FORMAL PARAMETERS:
**	Widget			w		Widget to be manipulated
**	XtWidgetGeometry	*request	Request to be made
**	XtWidgetGeometry	*reply		Our answer to the request
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**
**	XtGeometryResult
**
**		Returns status to indicate if the request was
**		granted.
**
**  SIDE EFFECTS:
**	The geometry of the specified widget may change.
**--
**/

static XtGeometryResult dvr_geometry_manager (w, request, reply)
    Widget		w;			/* Widget to be manipulated		*/
    XtWidgetGeometry	*request;		/* Geometry request from the widget	*/
    XtWidgetGeometry	*reply;			/* Our reply to the geometry request	*/
{

    /*
     *  At this time our geometry manager will always be 100% agreeable.
     *  We might want to change this at some future date to reflect a better
     *  model.  An example of this is the routine geometry_manager in the LAYOUT.C
     *  module in the CALENDAR application.
     */

    return (XtGeometryYes);

}


/*
**++
**  ROUTINE NAME:
**	dvr_change_window_geometry (w, size)
**
**  FUNCTIONAL DESCRIPTION:
** 	change the subwidget's window geometry; called by
**	change_widget to do the actual moving and resizing of
**	children; (originally DwtChangeWindowGeometry from
**	DECWMISC.)
**
**  FORMAL PARAMETERS:
**	Widget w;
**	XtWidgetGeometry *size;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_change_window_geometry (w, size)
    Widget w;
    XtWidgetGeometry *size;
{
    /*
     * split the changes into the two modes the intrinsic's understand,
     */

    if (size->request_mode & (CWX | CWY))
    {
	Position newx, newy;
	if (size->request_mode & CWX)
	    newx = size->x;
	else
	    newx = w->core.x;
	if (size->request_mode & CWY)
	    newy = size->y;
	else
	    newy = w->core.y;
	XtMoveWidget (w, newx, newy);
    }

    if (size->request_mode & (CWWidth | CWHeight | CWBorderWidth))
    {
	if (size->request_mode & CWWidth)
	    w->core.width = size->width;
	if (size->request_mode & CWHeight)
	    w->core.height = size->height;
	if (size->request_mode & CWBorderWidth)
	    w->core.border_width = size->border_width;
	XtResizeWindow (w);
    }
}



/*
**++
**  ROUTINE NAME:
**	change_widget(p, x, y, w, h)
**
**  FUNCTIONAL DESCRIPTION:
**      change the widget p to have the location
**	x,y; and the size w by h; called by dvr_resize
**	when one of the widgets children has changed
**	its dimensions;
**
**  FORMAL PARAMETERS:
**	Widget p;
**    	int x,y,w,h;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void change_widget(p, x, y, w, h)
    Widget 	p;
    Position 	x,y;
    Dimension  	w,h;

{
    XtWidgetGeometry g, *gp;

    gp = &g;
    GeoMode(gp) = 0;

    if ( XtX(p) != x) { GeoMode(gp) |= CWX;  gp->x = x; }
    if ( XtY(p) != y) { GeoMode(gp) |= CWY;  gp->y = y; }
    if ( XtWidth(p) != w) { GeoMode(gp) |= CWWidth;  gp->width = w; }
    if ( XtHeight(p) != h) { GeoMode(gp) |= CWHeight;  gp->height= h; }

    if (GeoMode(gp))
      {
	dvr_change_window_geometry(p, gp);
	(* (XtCoreProc (p, resize))) (p);
      }
}


/*
**++
**  ROUTINE NAME:
**	dvr_position_buttons(vw, b_box_height)
**
**  FUNCTIONAL DESCRIPTION:
**      change the positions of the buttons within the viewer's
**	button box based on how big the viewer is.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**	int 		*b_box_height;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_position_buttons(vw, b_box_height)
    DvrViewerWidget 	vw;
    int			*b_box_height;

{
    int x = 10;
    int y = 4;

/*AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT

    int audio_in_doc;

    /* Only display the audio toggle button when we've found some audio.
     * This has to be done before positioning the buttons because managing
     * and unmanaging the toggle affects the Y coordinate of all the other
     * buttons!!
     */

     /*
      * Check to see if there is audio in the document only if we've
      * already called the layout engine.  Note that we set a flag to
      * indicate if audio has been found.  In the event that audio is
      * found we need to setup the audio button in the button box.
     */

    if (audio_in_doc=DVS_AUDIO_IN_DOC(vw->dvr_viewer.Dvr.engine_context))
	{
	 /*
	  * Turn on the Audio Toggle Button which allows Audio Icons to be
	  * made visible/invisible.
	 */

	  if (AudioTog(vw) != NULL)
	      {
	       XtManageChild(AudioTog(vw));
	      }
	 }
     else
	{
	 /*
	  * Turn off the Audio Toggle Button which allows Audio Icons to be
	  * made visible/invisible.  Note that this may be occuring because there
	  * is not audio in the document.
	  *
	  * Note that we first check to see if the widget has been fetched yet.
	  * The reason we have to do this is that the Audio code is currently
	  * a bit on the sloppy side.
	 */

	 if (AudioTog(vw) != NULL)
	     {
	      XtUnmanageChild(AudioTog(vw));	/* Do we need to check if we're actually managed??? */
	     }
	}

    if (vw->dvr_viewer.Dvr.engine_context == NULL)
	{
	 /*
	  * Can't be any audio in the document yet since we don't have
	  * a document.  If we don't set this flag up properly we'll cause
	  * an ACCVIO in the code below when it tries to manipulate the
	  * widgets.
	 */

	 audio_in_doc = FALSE;

	 /*
	  * Check to see if the Audio Toggle Button is still turned on
	  * from a previous file.  If so we need to turn it off.
	 */

	 if (AudioTog(vw) != NULL)
	     {
	      XtUnmanageChild(AudioTog(vw));			/* Do we need to check if we're actually managed??? */
	     }
	}
#endif
    /*
     * NOTE 	I've changed the position of the code that increments the X
     *		coordinate below so that after each button is positioned the
     *		X coord is left 10 pixels to its right (ie ready for the next
     *		button).    Before I made the change, the code to position a
     *		button had to know what buttons were already visible.  This
     *		gets tricky now that there are more optional buttons.
     */
/*END AUDIO STUFF*/    


    /*  put the 'top document' button at the far left of the top row
     *  of the button box.
     */
    XtMoveWidget(TopBut(vw), x, y);
    x = x + XtWidth(TopBut(vw)) + 10;

    /*  If there's room, place each button to the right of
     *  the one before it; if there's not enough room, place
     *  the button on a new row at the far left
     */

    if ( (x + XtWidth(PrevBut(vw))) > XtWidth(Bbox(vw)) )
      {
	/* place the button on the next row at the far left */
	y = y + XtHeight(TopBut(vw)) + 7;
	x = 10;
      }

    XtMoveWidget(PrevBut(vw), x, y);
    x = x + XtWidth(PrevBut(vw)) + 10;

    if ( (x + XtWidth(GotoBut(vw))) > XtWidth(Bbox(vw)) )
      {
	/* place the button on the next row at the far left */
	y = y + XtHeight(PrevBut(vw)) + 7;
	x = 10;
      }

    XtMoveWidget(GotoBut(vw), x, y);
    x = x + XtWidth(GotoBut(vw)) + 10;

    if ( (x + XtWidth(NextBut(vw))) > XtWidth(Bbox(vw)) )
      {
	/* place the button on the next row at the far left */
	y = y + XtHeight(GotoBut(vw)) + 7;
	x = 10;
      }

    XtMoveWidget(NextBut(vw), x, y);
    x = x + XtWidth(NextBut(vw)) + 10;

    if ( (x + XtWidth(BotBut(vw))) > XtWidth(Bbox(vw)) )
      {
	/* place the button on the next row at the far left */
	y = y + XtHeight(NextBut(vw)) + 7;
	x = 10;
      }

    XtMoveWidget(BotBut(vw), x, y);
    x = x + XtWidth(BotBut(vw)) + 10;

/* cbr stuff */

    if (vw->dvr_viewer.Cbr.mode != FALSE && vw->dvr_viewer.Cbr.EnableRefButs)
      {
        x = x + 10;
        if ( (x + XtWidth(PrevRef(vw))) > XtWidth(Bbox(vw)) )
          {
    	/* place the button on the next row at the far left */
            y = y + XtHeight(BotBut(vw)) + 7;
            x = 10;
          }

        XtMoveWidget(PrevRef(vw), x, y);

        x = x + XtWidth(PrevRef(vw)) + 10;
        if ( (x + XtWidth(NextRef(vw))) > XtWidth(Bbox(vw)) )
          {
	/* place the button on the next row at the far left */
            y = y + XtHeight(PrevRef(vw)) + 7;
            x = 10;
          }

        XtMoveWidget(NextRef(vw), x, y);

        x = x + XtWidth(NextRef(vw)) + 10;
      }

/* end cbr changes */


/*AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    if (audio_in_doc)
    {
        x = x + 10;
        if ( (x + XtWidth(AudioTog(vw))) > XtWidth(Bbox(vw)) )
        {
	    /* place the button on the next row at the far left */
	    y = y + XtHeight(PrevBut(vw)) + 7;
	    x = 10;
        }

        XtMoveWidget(AudioTog(vw), x, y);
        x = x + XtWidth(AudioTog(vw)) + 10;
    }
#endif
/*END AUDIO STUFF*/
    if ( (CancelBut(vw) != NULL) && (XtIsManaged(CancelBut(vw))) )
      {
	/*  ps has two additional buttons; place them a little more
	 *  to the right
	 */

    	x = x + 10;
    	if ( (x + XtWidth(CancelBut(vw))) > XtWidth(Bbox(vw)) )
          {
	    /* place the button on the next row at the far left */
	    y = y + XtHeight(BotBut(vw)) + 7;
            x = 10;
          }

    	XtMoveWidget(CancelBut(vw), x, y);

    	x = x + XtWidth(CancelBut(vw)) + 10;
      }
    else
	/*  else, not viewing ps, last button before page label is bottom
	 *  document button
	 */
        x = x + XtWidth(BotBut(vw)) + 10; /* right of 'bottom document' button */


    /*  if the button box is wide enough to hold all of the
     *  buttons and labels, then put the label at the far right
     *  of the box; else, put it on the next row at the far right;
     */

    if ( (x + XtWidth(PageLabel(vw))) > XtWidth(Bbox(vw)) )
      {
	/*  place the label on the next row at the far right
	 *  if there's room; else but it at the far left;
	 */
	y = y + XtHeight(TopBut(vw)) + 7;
	x = XtWidth(Bbox(vw)) - XtWidth(PageLabel(vw)) - 10;
	x = MAX(x,10);
      }
    else
      {
	/* y pos has to be lower for label because it does not have shadow */
	y = y + 3;
	x = MAX(x, ( XtWidth(Bbox(vw)) - XtWidth(PageLabel(vw)) - 10 ) );
      }


    XtMoveWidget(PageLabel(vw), x, y);

    if ( XtY(PageLabel(vw)) != XtY(TopBut(vw)) )
    /* if the button box is larger than one row */
      {
	/*  then the button box should try to grow; all of the buttons
         *  are not on the same row.
	 */
	int h_scr_height = 0;
	int b_box_y;

	if (Hscr(vw) != NULL)
	    h_scr_height = XtHeight(Hscr(vw));

	/*  make the button box tall enough to hold all of the buttons
	 *  if there's room. If the viewer widget is too small, make the
	 *  button box smaller (children will be clipped). The viewer's
	 *  work area is never allowed to be shorter than DVR_MIN_WORK_HEIGHT.
	 */

	*b_box_height = MIN(
	  (XtY(PageLabel(vw)) + XtHeight(PageLabel(vw)) + 11),
	  (XtHeight(vw) - h_scr_height - DVR_MIN_WORK_WIDTH) );

	b_box_y = XtHeight(vw) - *b_box_height;

	change_widget(Bbox(vw),
		XtX(Bbox(vw)),
		b_box_y,
		XtWidth(Bbox(vw)),
		*b_box_height);
      }


}


/*
**++
**  ROUTINE NAME:
**	dvr_move_pg_label(vw)
**
**  FUNCTIONAL DESCRIPTION:
**      move the widget's page label so that it is at the far
**	right of the widget's button box.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_move_pg_label(vw)
    DvrViewerWidget vw;

{
    if (vw->dvr_viewer.button_box_flag) {

    int		new_x, bot_but_x;

    /*  put the page label at the far right of the button box if
     *  there's room.
     */
    new_x = XtWidth(Bbox(vw)) - XtWidth(PageLabel(vw)) - 10;		/* was -6 in XUI */

    bot_but_x = XtX(BotBut(vw));

    /*  if the page label and the bottom document button are not on
     *  the same line, then the page label is on it's own line and
     *  is treated differently
     */
    if ( XtY(PageLabel(vw)) == XtY(BotBut(vw)) )

	/*  if they're on the same line, then put the label either
	 *  at the far right of the button box; or next to the bottom
	 *  document button (whichever is farther out)
	 */
        new_x = MAX(new_x, (bot_but_x + XtWidth(BotBut(vw)) + 10) );	/* was + 6 in XUI */

    else
	/*  if the page label is on a separate line, then put the label
	 *  to the far right if the button box is large enough to hold it;
	 *  else, put it at the far left.
	 */
        new_x = MAX(10, new_x);						/* was 4 */

    XtMoveWidget(PageLabel(vw), new_x, XtY(PageLabel(vw)) );

    }
}



/*
**++
**  ROUTINE NAME:
**	dvr_set_page_number(vw)
**
**  FUNCTIONAL DESCRIPTION:
**      change the label in the viewer's button box to reflect
**	the current position within the document.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_set_page_number(vw)
    DvrViewerWidget vw;

{
    if (vw->dvr_viewer.button_box_flag) {

    char 	page_label[256];
    Arg		arg_list[5];
    struct pag  *current_page 	= vw->dvr_viewer.Dvr.current_page;
    int		page_num;
    Boolean	move_back, move_forward;
    XmString	cstr;

#ifdef DVR_PSVIEW
    if (VIEWING_PS(vw)) /* viewing ps ? */
      {
	/* for ps, we have current page number */
	page_num = vw->dvr_viewer.Ps.current_page;
      }

    else /* normal CDA viewing */
#endif
      {
    	/* if current page is empty, doc must be empty, set number to 1 */
    	if (current_page == NULL)
	    page_num = 1;
    	else
	    page_num = current_page->pag_page_number;
      }

    sprintf(page_label, "%s %d", vw->dvr_viewer.page_label_str, page_num);

    if (page_num == 1)
	move_back = FALSE;
    else
	move_back = TRUE;

    /* if this is the first page; disable backward page movement buttons */
    XtSetArg(arg_list[0], XmNsensitive, move_back);

    XtSetValues(PrevBut(vw), arg_list, 1);
    XtSetValues(TopBut(vw),  arg_list, 1);

/* cbr stuff */

    if (vw->dvr_viewer.Cbr.mode != FALSE && vw->dvr_viewer.Cbr.EnableRefButs)
        XtSetValues(PrevRef(vw), arg_list, 1);

/* end cbr stuff */

    if (vw->dvr_viewer.Dvr.DocRead)
      {
 	int  	last_page_num;

#ifdef DVR_PSVIEW
	if (VIEWING_PS(vw)) /* ps viewing ? */
	  {
             last_page_num = vw->dvr_viewer.Ps.last_page;
	  }

	else /* normal cda viewing */
#endif
	  {
	    struct 	pag *last_page =
		  	(struct pag *) vw->dvr_viewer.Dvr.page_list.que_blink;

	    /* check for null last page */
	    if ( (last_page == NULL) || (current_page == NULL) )
	        last_page_num = 1;
  	    else
	        last_page_num = last_page->pag_page_number;
           }

   	sprintf(page_label, "%s %s %d", page_label, vw->dvr_viewer.of_label_str,
		last_page_num);

	if (last_page_num == page_num)
	    move_forward = FALSE;
	else
	    move_forward = TRUE;

    	XtSetArg(arg_list[0], XmNsensitive, move_forward);

    	XtSetValues(NextBut(vw), arg_list, 1);
    	XtSetValues(BotBut(vw),  arg_list, 1);

/* cbr stuff */

	if (vw->dvr_viewer.Cbr.mode != FALSE && vw->dvr_viewer.Cbr.EnableRefButs)
            XtSetValues(NextRef(vw), arg_list, 1);

/* end cbr stuff */

	if (last_page_num == 1)
    	    XtSetValues(GotoBut(vw), arg_list, 1);
      }

#ifdef CDA_TWOBYTE
    {
      int len, stat;
      cstr = DXmCvtFCtoCS (page_label, &len, &stat);
    }
#else
    cstr = XmStringLtoRCreate(page_label , "ISO8859-1");
#endif

    XtSetArg(arg_list[0], XmNlabelString, cstr);
    XtSetValues(PageLabel(vw), arg_list, 1);
    XtFree ((char *)cstr);

    dvr_move_pg_label(vw);
    }
}



/*
**++
**  ROUTINE NAME:
**	dvr_update_page_number(vw)
**
**  FUNCTIONAL DESCRIPTION:
**      update the page number field in the viewer widget structure
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_update_page_number(vw)
    DvrViewerWidget vw;

{
    struct pag  *current_page 	= vw->dvr_viewer.Dvr.current_page;

#ifdef DVR_PSVIEW
    if (VIEWING_PS(vw)) /* viewing ps ? */
      {
	/* for ps, we have current page number */
	vw->dvr_viewer.page_number  = vw->dvr_viewer.Ps.current_page;
      }

    else /* normal CDA viewing */
#endif

      {
    	/* if current page is empty, doc must be empty, set number to 1 */
    	if (current_page == NULL)
	    vw->dvr_viewer.page_number = 0;
    	else
	    vw->dvr_viewer.page_number = current_page->pag_page_number;
      }

    return(DVR_NORMAL);
}




/*
**++
**  ROUTINE NAME:
**	dvr_resize (vw)
**
**  FUNCTIONAL DESCRIPTION:
**      rearrage the different regions of a viewer
**	widget based on the new size of the widget;
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
void dvr_resize (vw)
    DvrViewerWidget vw;
{
    Arg			arglist [2];
    XtWidgetGeometry	wg;
    int 		x, y;
    int			h_scr_height = (int) NULL;
    int 		w, h, a;
    int 		top    = 0;
    int 		bottom = XtHeight (vw);
    int 		right  = XtWidth (vw);

/*
 * layout each of the children which are still managed according the
 * current size we now are. Never allow the work area to become smaller
 * than DVR_MIN_WORK_WIDTH by DVR_MIN_WORK_HEIGHT. If the viewer widget
 * is smaller than this, then allow the other children to be clipped.
 *
 */

    if ( ! XtIsRealized(vw) ) return;

    if ((Vscr (vw) != NULL) && XtIsManaged (Vscr (vw)))

	/*  put the vertical scroll bar at the far right of the
	 *  widget UNLESS the widget is smaller than the minimum
	 *  width;  In this case, the vertical scroll bar will be
	 *  clipped off the edge of the viewer widget.
	 */
      {
	right = right - TotalWidth (Vscr (vw));

	if (right < DVR_MIN_WORK_WIDTH )
	    right = DVR_MIN_WORK_WIDTH;
      }

    x = 0;

    if (Hscr(vw) != NULL)
	h_scr_height = XtHeight(Hscr(vw));

    if (vw->dvr_viewer.button_box_flag)
      {
    	/* put the button box at the bottom of the widget */

    	/* make the box tall enogh to hold the buttons */
        a = TotalHeight(NextBut(vw)) + 7;

    	y = bottom -a;
    	if (y < (DVR_MIN_WORK_HEIGHT + h_scr_height) )
	  y = DVR_MIN_WORK_HEIGHT + h_scr_height;

        w = XtWidth(vw) - Double (XtBorderWidth (Bbox (vw)));
        if (w < DVR_MIN_WORK_WIDTH)
	  w = DVR_MIN_WORK_WIDTH;

        change_widget (Bbox (vw), x, y, w, a);

    	/*  position the buttons within the button box;
    	 *  if the width of button box is not large enough to
    	 *  hold all of the buttons, then the height of the button box (a)
    	 *  may change to accomadate more rows of buttons; imagine that!
    	 */
        dvr_position_buttons(vw, &a);

    	if (bottom > a)
	    bottom -= a;
      }

    /*
     * put the horizontal scroll bar just above the button box running
     * from the left to the left edge of the vertical scroll bar
     */

    if ((Hscr (vw) != NULL) && XtIsManaged (Hscr (vw)))
    {
	a = TotalHeight (Hscr (vw));

	x = 0;

	y = bottom - a;
	if ( y < DVR_MIN_WORK_HEIGHT )
	    y = DVR_MIN_WORK_HEIGHT;

	w = right - Double(XtBorderWidth(Hscr(vw)));

	if (w < DVR_MIN_WORK_WIDTH)
	    w = DVR_MIN_WORK_WIDTH;

	change_widget (Hscr (vw), x, y, w, XtHeight (Hscr (vw)));

	if (bottom > a)
	    bottom -= a;
    }

    /*
     * put the vertical scroll bar on the right edge of the main window
     * running from the bottom of the menubar to the top of the horizontal
     * scroll bar
     */

    if ((Vscr (vw) != NULL) && XtIsManaged (Vscr (vw)))
    {
	x = right;

	y = top;

	h = bottom - top - Double(XtBorderWidth(Vscr(vw)));
	if (h < DVR_MIN_WORK_HEIGHT)
	    h = DVR_MIN_WORK_HEIGHT;

	change_widget (Vscr (vw), x, y, XtWidth (Vscr (vw)), h);
    }

    /*
     * put the work area below the menubar, to the left of the vertical
     * scroll, above the horizontal scroll
     */

    if ((Work (vw) != NULL) && XtIsManaged (Work (vw)))
    {
	x = 0;

	y = top;

	w = right - Double (XtBorderWidth (Work (vw)));
	if (w < DVR_MIN_WORK_WIDTH)
	    w = DVR_MIN_WORK_WIDTH;

	h = bottom - top - Double (XtBorderWidth (Work (vw)));
        if (h < DVR_MIN_WORK_HEIGHT)
	    h = DVR_MIN_WORK_HEIGHT;

	change_widget (Work (vw), x, y, w, h);
    }

#ifdef DVR_PSVIEW
    if ( (PSscroll(vw) != NULL) && XtIsManaged(PSscroll(vw)) )
    {
	/*  if the ps scroll window is managed, then make it the width
	 *  of the viewer widget, and the height minus the size of the button
	 *  box
	 */

	Arg sw_arg_list[5];

	dvr_resize_ps_widget(vw);

	/*  because the Motif scrolled window widget automatically
	 *  sets the increment value for the scrollbar on resizes; we
	 *  need to override it here to get the same behavior as in normal
	 *  cda viewing
	 */
 	if (PSvbar(vw) != 0)
	  {
 	    /* set increment value to same as normal viewer*/
	    XtSetArg(sw_arg_list[0], XmNincrement, 10);
	    XtSetValues(PSvbar(vw), sw_arg_list, 1);
	  }
	if (PShbar(vw) != 0)
	  {
	    /* set increment value to same as normal viewer*/
	    XtSetArg(sw_arg_list[0], XmNincrement, 10);
	    XtSetValues(PShbar(vw), sw_arg_list, 1);
          }

    }
#endif

}



/*
**++
**  ROUTINE NAME:
**	dvr_resize_ps_widget(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	size the ps scroll widget to be the size of the viewer minus the
**	row of buttons at the bottom. The ps widget is a child of the scroll
**	widget and gets automatically resized upon resizing the ps scroll
**	widget.
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_resize_ps_widget(vw)
    DvrViewerWidget vw;

{
#ifdef DVR_PSVIEW
    short bbox_height = 0;

    if (vw->dvr_viewer.button_box_flag)
	bbox_height = XtHeight(Bbox(vw));

    change_widget (PSscroll(vw),
		   0,
		   0,
	           XtWidth(vw) - (2 * XtBorderWidth(vw)),
		   XtHeight(vw) - (2 * XtBorderWidth(vw)) - bbox_height);
#endif
}


/*
**++
**  ROUTINE NAME:
**	dvr_create_gc(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	if the widget does not have a gc associated with it, create
**	it specifying the values that we care about.
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_create_gc(vw)
    DvrViewerWidget vw;
{
    DvrStructPtr	dvr_ptr 	= &(vw->dvr_viewer.Dvr);

    /* create a GC specifying values that we care about iff it has
     * not yet been created.
     */
    if ( (dvr_ptr->GcId == NULL)  && (XtIsRealized(vw)) )
      {
        Arg 		arg_list[5];
        int 		arg_count = 0;
        Pixel 		fg_pixel, bg_pixel;
	XGCValues	temp_gcv;

        /* get the current foreground and background */
        XtSetArg(arg_list[arg_count], XmNforeground, &fg_pixel);
        arg_count++;
        XtSetArg(arg_list[arg_count], XmNbackground, &bg_pixel);
        arg_count++;

        XtGetValues((Widget)vw, arg_list, arg_count);

        /* create the GC
         */
        dvr_ptr->gcv.foreground 	= fg_pixel;
        dvr_ptr->gcv.background 	= bg_pixel;
        dvr_ptr->gcv.function 		= GXcopy;
	dvr_ptr->gcv.line_width 	= 0;
	dvr_ptr->gcv.line_style 	= LineSolid;
	dvr_ptr->gcv.cap_style 		= CapButt;
	dvr_ptr->gcv.join_style 	= JoinMiter;
	dvr_ptr->gcv.fill_style 	= FillSolid;
	dvr_ptr->gcv.arc_mode 		= ArcPieSlice;
        /* gcv.subwindow_mode = IncludeInferiors; */

        dvr_ptr->GcId = XCreateGC(XtDisplay(vw),
		Win(vw),
		GCForeground | GCBackground | GCFunction | GCLineWidth |
		GCCapStyle | GCJoinStyle | GCFillStyle | GCArcMode ,
                /* | GCSubwindowMode, */
	        &dvr_ptr->gcv);

	/*  create an erase GC which has the foreground and
	 *  background inverted
	 */

        temp_gcv.foreground = bg_pixel;
        temp_gcv.background = fg_pixel;
        dvr_ptr->erase_gc_id = XCreateGC(XtDisplay(vw),
	        Win(vw),
                GCForeground | GCBackground,
	        &temp_gcv);
      }

    return (DVR_NORMAL);
}


/*
**++
**  ROUTINE NAME:
**	dvr_expose_in_window(w, event, params, num_params)
**
**  FUNCTIONAL DESCRIPTION:
**  	this is the action routine for the expose event in a DvrWindow;
**  	all it  does is compress the exposures because DwtDialogBox does
**  	not do this automatically; and calls the display-page routine for
**  	the exposed area.
**
**
**  FORMAL PARAMETERS:
**	Widget 		w;
**	XExposeEvent  	*event;
**	char   		**params;
**	int	     	num_params;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_expose_in_window(w, event, params, num_params)
    Widget 		w;
    XExposeEvent  	*event;
    char   		**params;
    int	     		num_params;
{
    /* the viewer is the work area's parent */
    DvrViewerWidget vw = (DvrViewerWidget) w->core.parent;

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
        if (XtIsManaged(vw))
	  {
	    int ul_x = event->x;
	    int ul_y = event->y;
	    int lr_x = ul_x + event->width;
 	    int lr_y = ul_y + event->height;
	    CDAstatus stat;

	    /* put up watch; may not be needed if we're fast enough */
	    dvr_set_watch_cursor(vw);

	    /*  repaint this part of the screen, we shouldn't have
	     *  to clear the area becuase we are just drawing over
	     *  what should already be there
	     */

	    dvr_create_gc(vw);
	    stat = dvr_display_page(vw, ul_x, ul_y, lr_x, lr_y);
	    if (DVRFailure(stat))
	      {
		vw->dvr_viewer.Dvr.WidgetStatus = stat;
	        dvr_reset_cursor(vw);
		return;
	      }

	    vw->dvr_viewer.Dvr.WidgetStatus = dvr_adjust_sliders(vw);
	    dvr_reset_cursor(vw);
	  }
      }

      /*
      **  Invoke the application specific callback routine for the expose
      **  event
      */
      if (vw->dvr_viewer.expose_callback != NULL)				/* was .ecallback in XUI */
	  dvr_call_expose_callbacks(vw, event);

}



/*
**++
**  ROUTINE NAME:
**	dvr_set_status (w, cda_stat);
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called after every call to cda_convert. It checks
**	to make sure that the status returned from cda (cda_stat) is
**	successfull. If so, the routine exits, and the calling routine should
**	then check to WidgetStatus to make sure it is normal.
** 	If the cda_stat is not successfull, we then check the widget's
**	status here to see if it is success. If so, then we put the cda status
**	in the widget's status field so that an error will get flagged.
**	If the cda_stat is bad, and the widget's status is bad, use the
**	widget's status to return to the caller.
**
**	This routine is necessary in case we call something(e.g. isl, gks) that
**	signals our condition handler - which would then unwind to cda, cda
**	would then return to us with a bad status, but the widget status might
**	still be normal. (so we can't trust it)
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	unsigned int 	cda_stat;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_set_status (w, cda_stat)
    Widget		w;
    unsigned int 	cda_stat;

{
    DvrViewerWidget vw = (DvrViewerWidget) w;

    if (DVRSuccess(cda_stat))
	return;

    else if (DVRSuccess(vw->dvr_viewer.Dvr.WidgetStatus) ||
	     (vw->dvr_viewer.Dvr.WidgetStatus == 0) )
	vw->dvr_viewer.Dvr.WidgetStatus = cda_stat;


}


/*
**++
**  ROUTINE NAME:
**	dvr_initialize_window(vw)
**
**  FUNCTIONAL DESCRIPTION:
** 	this routine initializes all the varibles within the
**	widget having to do with displaying the current page
**	within the current window. It also displays the first
**	window of the page if we're managed.
**
**  FORMAL PARAMETERS:
**  	DvrViewerWidget       vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_initialize_window(vw)
    DvrViewerWidget 	vw;

{
    CDAstatus	stat;

    DvrStructPtr	dvr_ptr 	= &(vw->dvr_viewer.Dvr);

    dvr_ptr->Xtop 	= 0;
    dvr_ptr->Ytop	= 0;
    dvr_ptr->VertValue 	= 0;
    dvr_ptr->HorizValue	= 0;

    /* update page number label */
    dvr_set_page_number(vw);

    /* if we're managed, display first page */

    if (XtIsRealized(vw))
	  {
	    int lr_x = XtWidth(Work(vw));
	    int lr_y = XtHeight(Work(vw));

	    if (!vw->dvr_viewer.Dvr.erase_gc_id)
		dvr_create_gc(vw);

	    /* clear window */
	    XFillRectangle(XtDisplay(vw), Win(vw),
			vw->dvr_viewer.Dvr.erase_gc_id, 0, 0,
			XtWidth(Work(vw)),
			XtHeight(Work(vw)) );

            stat = dvr_display_page(vw, 0, 0, lr_x, lr_y);
	    if (DVRFailure(stat))
	      {
		dvr_ptr->WidgetStatus = stat;
		return(stat);
	      }

	    dvr_ptr->WidgetStatus = dvr_adjust_sliders(vw);
	    return(dvr_ptr->WidgetStatus);
	  }

    dvr_ptr->WidgetStatus =  DVR_NORMAL;;

    return (DVR_NORMAL);
}


/*
**++
**  ROUTINE NAME:
**	dvr_page_cb_proc(w, t, r)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called whenever the user hits one of the
**	page-movement buttons within the widget;
**
**  FORMAL PARAMETERS:
**    	Widget 			w;
**    	unsigned short	     	*t;
**    	XmAnyCallbackStruct 	*r;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_page_cb_proc(w, t, r)
    Widget 			w;
    XtPointer    		t;
    XmAnyCallbackStruct 	*r;

{
    Widget 		button_box;
    DvrViewerWidget 	vw;

    /* tag type has to match type specified in UIL file (integer)
     * to work on sun
     */
    int 		*button_type = (int *) t;

    int 		stat;

    /* the viewer widget id is the grandfather of the button */

    button_box = w->core.parent;
    vw = (DvrViewerWidget) button_box->core.parent;

    /* call the procedure corresponding to the button hit */

    switch (*button_type)
      {
	case k_next_pg:
		stat = DvrNextPage(vw);
		break;
	case k_prev_pg:
		stat = DvrPreviousPage(vw);
		break;
	case k_top_doc:
		stat = DvrTopDocument(vw);
		break;
	case k_bot_doc:
		stat = DvrBottomDocument(vw);
		break;
	case k_goto_pg:
		stat = dvr_goto_page_proc(vw);
		break;

/* cbr stuff */

	case k_next_ref:
            {
                int return_page;
		if (vw->dvr_viewer.Cbr.mode != 0) {
                    stat = DvrCbrNextRef(vw, 0, &return_page);
                    if (!DVRSuccess(stat)) {
			 /* error messange */
                        printf("No more references\n");
                    }
                }
            }
		break;

	case k_prev_ref:
            {
                int return_page;
		if (vw->dvr_viewer.Cbr.mode != 0) {
                    stat = DvrCbrPrevRef(vw, 0, &return_page);
                    if (!DVRSuccess(stat)) {
			 /* error messange */
                        printf("No previous references\n");
                    }
                }
            }
 		break;

/* end cbr stuff */

      }

    if (DVRFailure(stat))
	dvr_error_callback(vw, 0, stat, 0, 0);
}


/*
**++
**  ROUTINE NAME:
**	dvr_goto_page_proc (vw)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called when the user clicks on the GOTO PAGE
**	push button.  It Maps the Goto Dialog box to the screen.  If
**	the Goto Dialog box has not yet been created,
**	it is done here (along with all of its children).
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget		vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_goto_page_proc (vw)
    DvrViewerWidget	vw;

{

    /*  if the goto dialog box has not yet
     *  created, do so, and then map it
     */

    if (GotoDialog(vw) == 0)
      {
    	MrmType	dummy_class;

	if (MrmFetchWidget(
	    vw->dvr_viewer.drm_h,
	    "goto_dialog",
	    (Widget) vw,
	    & GotoDialog(vw),
	    & dummy_class) != MrmSUCCESS)
	  {
	    /* call error callback, bag */
	    dvr_error_callback(vw, 0, DVR_DRMPOPUPFETCHFAIL, 0, 0);
	    return(DVR_DRMPOPUPFETCHFAIL);
	  }
	else
	  {
	    /* pass the widget address back to the unmap proc */
	    XtCallbackRec cb_list[2];
	    int arg_count = 0;
	    Arg arg_list[3];

	    cb_list[0].callback = dvr_goto_entered_proc;
	    cb_list[0].closure  = (XtPointer) vw;
	    cb_list[1].callback = NULL;
	    cb_list[1].closure  = NULL;

	    XtSetArg(arg_list[arg_count], XmNactivateCallback, cb_list);
	    arg_count++;

	    XtSetValues(GotoOK(vw), arg_list, arg_count);
	    XtSetValues(GotoCancel(vw), arg_list, arg_count);
	  }
     }


    /* save the value of the text box incase of cancel */
    vw->dvr_viewer.save_goto_text = (char *) XmTextGetString(GotoText(vw));

   /* map the dialog box */

   if (XtIsManaged(GotoDialog(vw)))
	XRaiseWindow(XtDisplay(vw), XtWindow(XtParent(GotoDialog(vw))));
   else
        XtManageChild(GotoDialog(vw));

   return(DVR_NORMAL);
}


/*
**++
**  ROUTINE NAME:
**	dvr_goto_entered_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	unmap the goto page dialog box. If the OK button was hit,
**	get the page number entered, and try to goto that page.
**	The viewer id is passed in as the tag.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	XtPointer	tag;
**    	caddr_t		*reason;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_goto_entered_proc (w, tag, reason)
    Widget		w;
    XtPointer		tag;
    caddr_t		*reason;

{
   CDAcount     page_num;
   char		*num_entered;
   CDAstatus	stat = 0;

   DvrViewerWidget vw = (DvrViewerWidget) tag;

   XtUnmanageChild(GotoDialog(vw));

   /*  if the user hit the OK button, get the number entered,
    *  and call the GOTO proc
    */
   if (w == GotoOK(vw))
     {
        struct pag      *current_page 	= vw->dvr_viewer.Dvr.current_page;
        int		cur_page_num;
	char		*valid_digits 	= "0123456789 ";
	char	        current_char;
        int		i;
	Boolean		valid_number	   = TRUE;
	boolean		call_eod_callbacks = FALSE;

#ifdef DVR_PSVIEW
	if (VIEWING_PS(vw)) /* ps viewing ? */
	  {
	    cur_page_num = vw->dvr_viewer.Ps.current_page;
	  }
	else
#endif
	  {
	    /* normal CDA Viewing */

	    if (current_page == NULL)
	        cur_page_num = 1;
	    else
                cur_page_num = current_page->pag_page_number;
	  }

	/* get value from text widget and call DvrGotoPage */
   	num_entered = (char *) XmTextGetString(GotoText(vw));

	/* strip off spaces at the end */
	i = strlen(num_entered) -1;
	while( (num_entered[i] == ' ') && (i <= 0) )
	  {
	    num_entered[i] = '\0';
            i = strlen(num_entered) - 1;
	  }

	/*  if i is less than 0, then the user has entered an
	 *  empty string, slap their wrist!
	 */
	if (i < 0)
	    valid_number = FALSE;

	else for(i=0; i<strlen(num_entered); i++)
	    if (strchr(valid_digits, num_entered[i]) == 0)
		valid_number = FALSE;

	if (valid_number)
   	    sscanf(num_entered, "%d", &page_num);
	else
	    page_num = 0;

   	XtFree(num_entered);

	/* page 0 is invalid */
        if (page_num <= 0)
	    stat = 0;

	/* if we're already on page_num, then no work is required */
	else if (page_num == cur_page_num)
	    stat = 1;
	else
	  {
#ifdef DVR_PSVIEW
	    if (VIEWING_PS(vw)) /* ps viewing ? */
	      {
		if (vw->dvr_viewer.Ps.last_page != 0)
		  {
		    if (page_num > vw->dvr_viewer.Ps.last_page)
			page_num = vw->dvr_viewer.Ps.last_page;
		  }
	      }

	    else
#endif
	      {
              /* normal CDA Viewing */

	      if (vw->dvr_viewer.Dvr.DocRead)
		{
		/* if the document is read, dont go if we don't have to */
	        struct 	pag *last_page =
		  	(struct pag *) vw->dvr_viewer.Dvr.page_list.que_blink;
 	        int  	last_page_num;

		/* check for empty last page */
	 	if ( (last_page == NULL) || (current_page == NULL) )
		    last_page_num = 1;
		else
		    last_page_num  = last_page->pag_page_number;

		/* not that many pages? set to valid number */
	        if (page_num > last_page_num)
		  {
		    page_num = last_page_num;
		    if (cur_page_num == page_num)
		        stat = 1;
		    else
		      {
			if (vw->dvr_viewer.Dvr.DocRead)
			   call_eod_callbacks = TRUE;
		      }
		  }
	        }
	      }

	    if (stat == 0)
   	    	stat = DvrGotoPage(vw, page_num);
	  }

	/* if not successfull, put up a message box */
   	if ( DVRFailure(stat) || (stat == DVR_PAGENOTFOUND) )
     	  {

	    if (Message(vw) == NULL)
	      {
        	MrmType	dummy_class;
   		if (MrmFetchWidget(
		    vw->dvr_viewer.drm_h,
		    "dvr_message",
		    (Widget) vw,
		    & Message(vw),
	    	    & dummy_class) != MrmSUCCESS)
		      {
			/* call error callback, bag */
			dvr_error_callback(vw, 0, DVR_DRMPOPUPFETCHFAIL, 0, 0);
			return;
		      }
	      }

	    XtManageChild(Message(vw));
          }
	else
	  {
	    if (call_eod_callbacks)
		dvr_call_eod_callbacks(vw);
	  }
     }
   else /* user hit cancel */
	/* reset text */
	XmTextSetString(GotoText(vw), vw->dvr_viewer.save_goto_text);


   /* free storage */
   XtFree(vw->dvr_viewer.save_goto_text);
}



/*
**++
**  ROUTINE NAME:
**	dvr_message_cb_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	unmap the message box; the parent of the button
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	int		tag;
**    	caddr_t		*reason;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_message_cb_proc (w, tag, reason)
    Widget		w;
    int			tag;
    caddr_t		*reason;

{
   Widget mw = w->core.parent;

   XtUnmanageChild(mw);

}


/*
**++
**  ROUTINE NAME:
**	dvr_set_buttons_sensitive(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	set all of the buttons sensitivity according to whether or
**	not a file is open. If a file is open, enable the buttons.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget 	vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_set_buttons_sensitive(vw)
    DvrViewerWidget vw;

{
    if (vw->dvr_viewer.button_box_flag) {

    int sensitive = vw->dvr_viewer.Dvr.FileOpen;
    Arg arg_list[3];

    XtSetArg(arg_list[0], XmNsensitive, sensitive);

    XtSetValues(NextBut(vw), arg_list, 1);
    XtSetValues(PrevBut(vw), arg_list, 1);
    XtSetValues(TopBut(vw),  arg_list, 1);
    XtSetValues(BotBut(vw),  arg_list, 1);
    XtSetValues(GotoBut(vw), arg_list, 1);

/* cbr stuff */

    if (vw->dvr_viewer.Cbr.mode != FALSE && vw->dvr_viewer.Cbr.EnableRefButs)
      {
        XtSetValues(PrevRef(vw), arg_list, 1);
        XtSetValues(NextRef(vw), arg_list, 1);
      }

/* end cbr stuff */
    }
}



/*
**++
**  ROUTINE NAME:
**	dvr_init_for_drm()
**
**  FUNCTIONAL DESCRIPTION:
**      this routine needs to be called before our widget code tries
**	to fetch anything from it's UIL file; it initializes DRM, and
**	registers our user-defined class;
**
**  FORMAL PARAMETERS:
**	none
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_init_for_drm()

{
    /* start up MRM */
    MrmInitialize();

    /* start up DXm */
    DXmInitialize();

    /* register viewer widget class */
    if (MrmRegisterClass(MrmwcUnknown,
			 DvrViewerClassStr,
			 "DvrViewerCreateDRM",
			 (Widget (*)()) DvrViewerCreateDRM,
			 (WidgetClass) dvr_viewer_class_rec_addr)
			 != MrmSUCCESS)
	return(DVR_INTERNALERROR);

#ifdef DVR_PSVIEW
    /* register classes for ps widgets */
    (void) PSViewInitializeForDRM ();
#endif

    return(DVR_NORMAL);
}




/*
**++
**  ROUTINE NAME:
**	dvr_find_viewer_widget (w)
**
**  FUNCTIONAL DESCRIPTION:
**	given a widget: w, find it's closest ancestor that is a viewer
**	widget; this routine is called by the uil-creation routine, it
**	get's called when a child of the viewer whose ID will be needed
**	later is created, and it needs to know the viewer id.
**
**  FORMAL PARAMETERS:
**	Widget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

DvrViewerWidget dvr_find_viewer_widget (w)
    Widget w;
{
    Widget		p;

    p = w;
    while (!XtIsSubclass (p, (WidgetClass)dvr_viewer_class_rec_addr)) p = XtParent (p);
    return ((DvrViewerWidget) p);
}


/*
**++
**  ROUTINE NAME:
**	dvr_add_table(w,table)
**
**  FUNCTIONAL DESCRIPTION:
**      Overrides the widget instance with the new parsed translation.
**
**  FORMAL PARAMETERS:
**	Widget	     w;
**	char	     *table;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_add_table(w, table, actions)
    Widget	 w;
    char	 *table;
{
    Arg arglist[2];
    int	ac = 0;
    XtTranslations parsed_t_table = XtParseTranslationTable
				       ( table );
    if ( w->core.tm.translations != NULL )
	XtOverrideTranslations( w, parsed_t_table );
    else
    {
	XtSetArg( arglist[ac], XmNtranslations, parsed_t_table);
	ac++;
	XtSetValues( w, arglist, ac);
    }
}


/*
**++
**  ROUTINE NAME:
**	dvr_remove_table(w)
**
**  FUNCTIONAL DESCRIPTION:
**      removes the current translation table. Overrides the widget
**	instance with the default parsed translation table.
**
**  FORMAL PARAMETERS:
**	Widget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_remove_table(w)
    Widget   w;
{
    XtTranslations parsed_t_table = XtParseTranslationTable
				       ( dvr_translations );

    XtOverrideTranslations( w, parsed_t_table );
}




/*
**++
**  ROUTINE NAME:
**	dvr_cancel_view_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      procedure to be called when user hits cancel button, call proc
**	to abort viewing
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**	caddr_t		*tag;
**	unsigned int	*reason;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_cancel_view_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
{

#ifdef DVR_PSVIEW

    DvrViewerWidget	vw;

    /* get the id of the parent viewer */
    vw = dvr_find_viewer_widget (w);

    dvr_abort_ps_view(vw);

#endif

}




/*
**++
**  ROUTINE NAME:
**	dvr_create_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      procedure to be called upon creation of the children of the
**	viewer widget via UIL; we store the id's of the children in the
**	parent viewer so we can manipulate them later;
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**	caddr_t		*tag;
**	unsigned int	*reason;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_create_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
{

    DvrViewerWidget	vw;
    int			widget_num = (int) *tag;

    /* get the id of the parent viewer */
    vw = dvr_find_viewer_widget (w);

    switch (widget_num) {
	case k_work_area : {

    		/* new actions for dialog box */
    		static XtActionsRec dvr_win_actions [] =
    		  {
   		    {"DVR_WIN_EXPOSE",  (XtActionProc) dvr_expose_in_window},
    		    {"DVR_HELP",	(XtActionProc) dvr_call_help_callbacks},
		    {"DVR_ACTIVATE",	(XtActionProc) dvr_call_regular_callbacks},
    		    {NULL, 		NULL}
      		  };

		Work (vw) = w;

		/* add new actions for dialog box so we get expose events */

    		XtAppAddActions(XtWidgetToApplicationContext(w), 		/* Application context for translation	*/
			        dvr_win_actions, 				/* Actions to be added			*/
				3);						/* Number of actions to be added	*/

		/* add event handler so we get non-maskable events */
		XtAddEventHandler(Work(vw), 0, 1,
				  (XtEventHandler) dvr_handle_nonmaskable_event, vw);

		/*
		**  Check the callbacks lists and invoke dvr_add_table
		**  with the appropriate translation table
		*/
		if (vw->dvr_viewer.mouse_motion_callback != NULL &&		/* was .ecallback in XUI */
		    vw->dvr_viewer.buttons_callback != NULL)			/* was .ecallback in XUI */
		{
		    dvr_add_table( Work(vw)
				 , dvr_buttons_translations_with_motion
				 );

		} else if ( vw->dvr_viewer.mouse_motion_callback != NULL &&	/* was .ecallback in XUI */
			    vw->dvr_viewer.buttons_callback == NULL)		/* was .ecallback in XUI */
		{
		    dvr_add_table( Work(vw)
				 , dvr_translations_with_motion
				 );

		} else if ( vw->dvr_viewer.mouse_motion_callback == NULL &&	/* was .ecallback in XUI */
			    vw->dvr_viewer.buttons_callback != NULL)		/* was .ecallback in XUI */
		{
		    dvr_add_table( Work(vw)
				 , dvr_buttons_translations
				 );

		}
		break;
	}

	/*  if the viewer is created without one of it's scrollbars or
	 *  it's button box, we must unmanage them/set to NULL in the
	 *  viewer's realize proc so that it really takes effect
	 */

	case k_h_scroll : {
		Hscr(vw) = w;
		break;
	}

	case k_v_scroll : {
		Vscr (vw) = w;
		break;
	}

	case k_button_box : {
    		/* new actions for dialog box */
    		static XtActionsRec dvr_bbox_actions [] =
    		  {
    		    {"DVR_HELP",	(XtActionProc) dvr_call_help_callbacks},
    		    {NULL, 		NULL}
      		  };

		/*  add new actions for dialog box so we get events for
		 *  callbacks
		 */

    		XtAppAddActions(XtWidgetToApplicationContext(w), 		/* Application context for translation	*/
			        dvr_bbox_actions, 				/* Actions to be added			*/
				1);						/* Number of actions to be added	*/
		Bbox (vw) = w;
		break;
	}


/*AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
	case k_audio_toggle : {
		if (vw->dvr_viewer.button_box_flag)
		  {
		    Arg arglist[3];
		    int argcount = 0;

                    /* Don't put the audio toggle on the screen until
                     * we know that there is some audio data.
                     */
		    XtUnmanageChild(w);

		    AudioTog(vw) = w;
                    AudioState(vw) = 0;

		    /*  make sure pixmap buttons are same height as
	 	     *  goto text button
		     */
		    argcount = 0;
		    XtSetArg(arglist[argcount], XmNheight,
			 (GotoBut(vw))->core.height);
		    argcount++;

	            XtSetValues(w, arglist, argcount);
                  }
		else
		  {
		    /*  if the viewer is created without a button box
		     *  requested, unmap it, leave null in widget
		     */
		    XtUnmanageChild(w);
		  }
		break;
        }
#else
	case k_audio_toggle : {         /*Still create the button even if no */
                XtDestroyWidget(w);     /*audio support, because there's no  */
		break;			/*conditional compilation for UIL.   */
        }
#endif
/*END AUDIO STUFF*/

	case k_top_button : {
		if (vw->dvr_viewer.button_box_flag)
		  {
		    Arg arglist[3];
		    int argcount = 0;

		    TopBut (vw) = w;

		    /*  make sure pixmap buttons are same height as
	 	     *  goto text button
		     */
		    argcount = 0;
		    XtSetArg(arglist[argcount], XmNheight,
			 (GotoBut(vw))->core.height);
		    argcount++;

	            XtSetValues(TopBut(vw), arglist, argcount);
                  }
		else
		  {
		    /*  if the viewer is created without a button box
		     *  requested, unmap it, leave null in widget
		     */
		    XtUnmanageChild(w);
		  }
		break;
	}

	case k_prev_button : {
		if (vw->dvr_viewer.button_box_flag)
		  {
		    Arg arglist[3];
		    int argcount = 0;

		    PrevBut (vw) = w;

		    /*  make sure pixmap buttons are same height as
	 	     *  goto text button
		     */
		    argcount = 0;
		    XtSetArg(arglist[argcount], XmNheight,
			 (GotoBut(vw))->core.height);
		    argcount++;

	            XtSetValues(PrevBut(vw), arglist, argcount);
                  }
		else
		  {
		    /*  if the viewer is created without a button box
		     *  requested, unmap it, leave null in widget
		     */
		    XtUnmanageChild(w);
		  }

		break;
	}

	case k_goto_button : {
		if (vw->dvr_viewer.button_box_flag)
		    GotoBut (vw) = w;
		else
		  {
		    /*  if the viewer is created without a button box
		     *  requested, unmap it, leave null in widget
		     */
		    XtUnmanageChild(w);
		  }
		break;
	}

	case k_next_button : {
		if (vw->dvr_viewer.button_box_flag)
		  {
		    Arg arglist[3];
		    int argcount = 0;

		    NextBut (vw) = w;

		    /*  make sure pixmap buttons are same height as
	 	     *  goto text button
		     */
		    argcount = 0;
		    XtSetArg(arglist[argcount], XmNheight,
			 (GotoBut(vw))->core.height);
		    argcount++;

	            XtSetValues(NextBut(vw), arglist, argcount);
                  }
		else
		  {
		    /*  if the viewer is created without a button box
		     *  requested, unmap it, leave null in widget
		     */
		    XtUnmanageChild(w);
		  }

		break;
	}

	case k_bot_button : {
		if (vw->dvr_viewer.button_box_flag)
		  {
		    Arg arglist[3];
		    int argcount = 0;

		    BotBut (vw) = w;

		    /*  make sure pixmap buttons are same height as
	 	     *  goto text button
		     */
		    argcount = 0;
		    XtSetArg(arglist[argcount], XmNheight,
			 (GotoBut(vw))->core.height);
		    argcount++;

	            XtSetValues(BotBut(vw), arglist, argcount);
                  }
		else
		  {
		    /*  if the viewer is created without a button box
		     *  requested, unmap it, leave null in widget
		     */
		    XtUnmanageChild(w);
		  }

		break;
	}


	case k_page_label : {
		if (vw->dvr_viewer.button_box_flag)
		    PageLabel (vw) = w;
		else
		  {
		    /*  if the viewer is created without a button box
		     *  requested, unmap it, leave null in widget
		     */
		    XtUnmanageChild(w);
		  }
		break;
	}

	case k_goto_text : {
		GotoText (vw) = w;
		break;
	}

	case k_goto_ok : {
		GotoOK (vw) = w;
		break;
	}

	case k_goto_cancel : {
		GotoCancel (vw) = w;
		break;
	}

#ifdef DVR_PSVIEW
	case k_ps_scroll_w: {
		if (vw->dvr_viewer.Ps.dps_exists)
		  {
		    PSscroll(vw) = w;
		  }
		break;
	}

	case k_ps_window: {
		if (vw->dvr_viewer.Ps.dps_exists)
		  {
		    PSwin(vw) = w;
	   	    (void) dvr_init_ps(vw);

#ifdef PC_NEW_CALLBACKS
/*  still waiting on ps; micky had some of the code in place; we need
 *  to wait on all of the ps code before putting this in for real
 */
      		    /*
		    **  Check the callbacks lists and invoke dvr_add_table
		    **  with the appropriate translation table
		    */
		    if (vw->dvr_viewer.mouse_motion_callback != NULL &&		/* was .ecallback in XUI */
			vw->dvr_viewer.buttons_callback != NULL)		/* was .ecallback in XUI */
		    {
			dvr_add_table( PSwin(vw)
				     , dvr_buttons_translations_with_motion
				     );

		    } else if ( vw->dvr_viewer.mouse_motion_callback != NULL &&	/* was .ecallback in XUI */
				vw->dvr_viewer.buttons_callback == NULL)	/* was .ecallback in XUI */
		    {
			dvr_add_table( PSwin(vw)
				     , dvr_translations_with_motion
				     );

		    } else if ( vw->dvr_viewer.mouse_motion_callback == NULL &&	/* was .ecallback in XUI */
				vw->dvr_viewer.buttons_callback != NULL)	/* was .ecallback in XUI */
		    {
			dvr_add_table( PSwin(vw)
				     , dvr_buttons_translations
				     );

		    }
#endif
		  }
		break;
	}

#endif /* DVR_PSVIEW */

    }

}


/*
**++
**  ROUTINE NAME:
**	dvr_call_help_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      procedure to be called when help is requested in any of the
**	various children of the viewer widget (work area, scrollbars,
**	button box, message box, etc)
**
**	call the viewer's help callback list (note, not context sensitive)
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**	caddr_t		*tag;
**	unsigned int	*reason;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

static void dvr_call_help_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
{

    DvrViewerWidget	vw;
    int			widget_num = (int) *tag;
    XEvent 		ev;

    /* get the id of the parent viewer */
    vw = dvr_find_viewer_widget (w);

    /* issue callback to calling application */
    vw->dvr_viewer.Dvr.WidgetStatus = DVR_NORMAL;

    dvr_call_callbacks(vw, DvrCRhelpRequested, &ev);
}




/*  PUBLIC ENTRY POINTS FOR C BINDINGS BEGIN HERE  */

/*
**++
**  ROUTINE NAME:
**	DvrViewer (parent, name, x, y, width, height,
**		   h_scroll_flag, v_scroll_flag, processing_options,
**		   callback, help_callback);
**
**  FUNCTIONAL DESCRIPTION:
**      high level creation routine for a viewer widget;
**
**  FORMAL PARAMETERS:
**    	Widget        	parent;
**    	char	        *name;
**    	Position       	x;
**    	Position      	y;
**    	Dimension      	width;
**    	Dimension     	height;
**	Boolean		h_scroll_flag;
**	Boolean		v_scroll_flag;
**	unsigned long	processing_options;
**    	XtCallbackList	callback;
**	XtCallbackList  help_callback;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

#if CDA_EXPAND_PROTO == 1

DvrViewerWidget DvrViewer (Widget        	parent,
			   CDAenvirontext	*name,
			   Position           	x,
			   Position           	y,
			   Dimension          	width,
			   Dimension          	height,
			   Boolean		h_scroll_flag,
			   Boolean		v_scroll_flag,
			   CDAflags		processing_options,
			   XtCallbackList	callback,
			   XtCallbackList	help_callback)

#else

DvrViewerWidget DvrViewer (parent, name, x, y, width, height,
		  h_scroll_flag, v_scroll_flag, processing_options,
		  callback, help_callback)
    Widget        	parent;
    CDAenvirontext	*name;
    Position           	x;
    Position           	y;
    Dimension          	width;
    Dimension          	height;
    Boolean		h_scroll_flag;
    Boolean		v_scroll_flag;
    CDAflags		processing_options;
    XtCallbackList	callback;
    XtCallbackList	help_callback;

#endif

{
    DvrViewerWidget 	vw;
    Widget	w;
    Arg 	arglist[20];
    unsigned short 	argCount = 0;

    /*  set up the argument list for call to XtCreateWidget  */

    if (x != (Position) NULL)
      {
    	XtSetArg(arglist[argCount], XmNx, x);
	argCount++;
      }

    if (y != (Position) NULL)
      {
    	XtSetArg(arglist[argCount], XmNy, y);
	argCount++;
      }

    if (height != (Dimension) NULL)
      {
   	XtSetArg(arglist[argCount], XmNheight, height);
	argCount++;
      }

    if (width != (Dimension) NULL)
      {
    	XtSetArg(arglist[argCount], XmNwidth, width);
	argCount++;
      }


    if (processing_options != (CDAflags) NULL)
      {
	XtSetArg(arglist[argCount], DvrNprocessingOptions, processing_options);
	argCount++;
      }

    XtSetArg(arglist[argCount], DvrNscrollHorizontal, h_scroll_flag);
    argCount++;
    XtSetArg(arglist[argCount], DvrNscrollVertical, v_scroll_flag);
    argCount++;
    XtSetArg(arglist[argCount], XmNactivateCallback, callback);
    argCount++;
    XtSetArg(arglist[argCount], XmNhelpCallback, help_callback);
    argCount++;

    return ( DvrViewerCreate(parent,
			     name,
			     arglist,
			     argCount) );

}



/*
**++
**  ROUTINE NAME:
**	DvrViewerCreate (parent, name, args, argCount)
**
**  FUNCTIONAL DESCRIPTION:
**      low level creation routine for a viewer widget;
**
**  FORMAL PARAMETERS:
**	Widget   parent;
**	char    *name;
**	ArgList  arglist;
**	int      argCount;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

DvrViewerWidget DvrViewerCreate (parent, name, arglist, argCount)
    Widget   	   parent;
    CDAenvirontext *name;
    ArgList  	   arglist;
    CDAcardinal    argCount;
{
    int j;
    DvrViewerWidget vw;
    Widget w;

    MrmType	          dummy_class;
    MrmCode		  dummy_type;		/*  Dummy parameter passed to the fetch literal routine */
    MrmHierarchy	  drm_h;
    int			  ret_status;

    static MrmRegisterArg	regvec[] = {
/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
	  	{"dvr_audio_button_cb",   (caddr_t) dvr_audio_button_cb},
	  	{"dvr_audio_toggle_cb",   (caddr_t) dvr_audio_toggle_cb},
#else
	  	{"dvr_audio_button_cb",   (caddr_t) NULL},  /*Have to put something */
	  	{"dvr_audio_toggle_cb",   (caddr_t) NULL},  /*- no UIL cond.compilation.*/
#endif
/*END AUDIO STUFF*/
	  	{"dvr_cancel_view_proc",  (caddr_t) dvr_cancel_view_proc},
		{"dvr_call_help_proc", 	  (caddr_t) dvr_call_help_proc},
		{"dvr_create_proc", 	  (caddr_t) dvr_create_proc},
		{"dvr_goto_entered_proc", (caddr_t) dvr_goto_entered_proc},
		{"dvr_message_cb_proc",   (caddr_t) dvr_message_cb_proc},
		{"dvr_page_cb_proc", 	  (caddr_t) dvr_page_cb_proc},
		{"dvr_scroll_action_proc",(caddr_t) dvr_scroll_action_proc}
	};

    char *db_filename_vec[1];
    char *uiddir;

/*AUDIO STUFF - I changed the next line to keep regnum in step with regvec
 *automatically.                                            
 */
    int			  regnum = (sizeof regvec / sizeof regvec[0]);


/*  move this call to DvrRegisterClass() for DECwindows V1;
 *  this should only work in DECwindows V2 when the restriction
 *  on calling RegisterClass before XtInitialize() goes away;
 */

    if (dvr_init_for_drm() != DVR_NORMAL)
      {
	return ((DvrViewerWidget) NULL);
      };

#ifdef __unix__

    /*  on ultrix, check for UIDDIR enviornment var;
     *  if set, use to look for UID file; else look in
     *  default area
     */

    if ((uiddir= (char *) getenv("UIDDIR")) != NULL)
      {
	/* UIDDIR is defined, look here for UID file */
	db_filename_vec[0] = XtMalloc( DEFAULT_DVR_UID_LEN +
				       strlen(uiddir)  );
	strcpy(db_filename_vec[0], uiddir);
	strcat(db_filename_vec[0], DEFAULT_DVR_UID_NAME);

	XtFree(uiddir);
      }

    else
        /* UIDDIR not defined, use the default */
#endif

      {
	db_filename_vec[0] = XtMalloc(DEFAULT_DVR_UID_FULL_LEN);
	strcpy(db_filename_vec[0], DEFAULT_DVR_UID_FULL_NAME);
      }

    /*  Define the DRM "hierarchy" 					      */
    /*                                                                        */
    if (MrmOpenHierarchy (1,		    	/* number of files	      */
			  db_filename_vec,      /* files     	              */
			  NULL,	       		/* os_ext_list (null)         */
			  &drm_h)		/* ptr to returned id         */
			  != MrmSUCCESS)
      {
	/* return a NULL widget pointer; caller should know
	 * something's wrong.
	 */
	return ((DvrViewerWidget) NULL);
      };


    /* Register our callback routines so that the resource manager can            */
    /* resolve them at widget-creation time.                                      */
    /*                                                                            */
    MrmRegisterNames(regvec, regnum);

    if (MrmFetchWidgetOverride (drm_h,
			     	"viewer_widget",
		             	parent,
				name,
			     	arglist,
			     	argCount,
		             	&w,
		             	&dummy_class) != MrmSUCCESS)
      {
	return ((DvrViewerWidget) NULL);
      };

    vw = (DvrViewerWidget) w;

    vw->dvr_viewer.drm_h = drm_h;

    /* set up the context, and fetch uil strings */

    /*
     * Fetch the page label string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$page_label_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&vw->dvr_viewer.page_label_str,  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch the "of" label string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$of_label_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&vw->dvr_viewer.of_label_str,	  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch the default font string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$default_font_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&vw->dvr_viewer.default_font_str,  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch the postscript string.
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$postscript_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&vw->dvr_viewer.Ps.ps_str,	  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /* if something went wrong, deallocate the widget, and return 0 */
    if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
      {
	XtDestroyWidget((Widget)vw);
	vw = 0;
      }

    else
      {
	/* set file-open flag to false */
	vw->dvr_viewer.Dvr.FileOpen = 0;

        dvr_get_dpi_and_fonts(vw);

    	/* init font table;
     	   MUST be called AFTER setting up available fonts (that is, vw->
       	   dvr_viewer.num_fonts must already be set) */

    	vw->dvr_viewer.Dvr.WidgetStatus = dvr_init_font_table ( vw );
        if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
          {
	    XtDestroyWidget((Widget)vw);
	    vw = 0;
          }
      }

    return( (DvrViewerWidget) vw );

}



/*
**++
**  ROUTINE NAME:
**	DvrViewerCreateDRM (parent, name, arglist, argCount)
**
**  FUNCTIONAL DESCRIPTION:
** 	call XtCreateWidget() to allocate memory for a viewer widget
**
**  FORMAL PARAMETERS:
**    	Widget   parent;
**   	char    *name;
**    	ArgList  arglist;
**    	int      argCount;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

DvrViewerWidget DvrViewerCreateDRM (parent, name, arglist, argCount)
    Widget   parent;
    char    *name;
    ArgList  arglist;
    int      argCount;
{
    int j;
    DvrViewerWidget vw;
    Widget w;

    w = XtCreateWidget( name,
		        (WidgetClass) dvr_viewer_class_rec_addr,
		        parent,
		        arglist,
		        argCount );

    vw = (DvrViewerWidget) w;

    return( (DvrViewerWidget) vw );

}



/*
**++
**  ROUTINE NAME:
**	DvrViewerFile(w,filename, format_type, options_file, get_rtn, get_param)
**
**  FUNCTIONAL DESCRIPTION:
**      associate the supplied file with the
**	supplied viewer widget;
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget w;
**    	char   *filename;
**    	char   *format_type;
**	char   *options_file
**	PROTO(unsigned long (CDA_CALLBACK *get_rtn), (void*, unsigned long *, unsigned char **));
**    	void	*get_param;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrViewerFile(w, filename, format_type, options_file, get_rtn, get_param)
    DvrViewerWidget 	w;
    CDAenvirontext   	*filename;
    CDAenvirontext   	*format_type;
    CDAenvirontext 	*options_file;
    PROTO(CDAstatus (CDA_CALLBACK *get_rtn), (CDAuserparam, CDAsize *, CDAbufaddr *));
    CDAuserparam	get_param;

{
    DvrViewerWidget vw = (DvrViewerWidget) w;

    CDAstatus ret_stat;

    int  		function_code;
    int  		format_len = 0;
    int  		name_len = 0;
    int  		ddif_len = 0;
    char		*ddif_name = "DDIF";
    int			op_file_len = 0;
    unsigned long 	output_file;
    int			item_count = 0;
    DvrCdaItemList	item_list[12];
    char		format_type_str[256];


    /* return invalid address if widget is zero */
    if (vw == 0) return(DVR_INVADDR);

    if (filename != NULL)
    	name_len      	= strlen(filename);
    if (format_type != NULL)
      {
	int i;
    	format_len 	= strlen(format_type);

	/* convert to lower case */
        for (i=0; i<format_len; i++)
	    format_type_str[i] = tolower(format_type[i]);

	format_type_str[format_len] = '\0';
      }

    if (options_file != NULL)
    	op_file_len	= strlen(options_file);
    ddif_len 	= strlen(ddif_name);

    /* close out file if one already open */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
#ifdef DVR_PSVIEW
	/* check for special PS case */
	if (VIEWING_PS(vw))
          /*  if we're viewing ps, do not call DvrCloseFile() to
	   *  avoid having to swtich between managed windows
	   */

	  {
	    CDAstatus free_stat;

	    /* check for case where PS is still churning; if so, abort! */
	    if (vw->dvr_viewer.Ps.waiting)
		PSViewAbort(PSwin(vw));

	    vw->dvr_viewer.Dvr.DocRead = FALSE;
	    vw->dvr_viewer.Dvr.FileOpen = FALSE;
	    dvr_view_ps_file(vw, NULL);

	    vw->dvr_viewer.Ps.current_page = 0;
	    vw->dvr_viewer.Ps.last_page = 0;

	    /* get rid of doc info memory */
	    free_stat = dvr_free_doc_info(vw);
    	    if (DVRFailure(free_stat))
	        dvr_error_callback(vw, 0, free_stat, 0, 0);
	  }

	else
#endif
	    DvrCloseFile(vw);
      }

    /* initially page number is zero */
    vw->dvr_viewer.page_number = 0;

    dvr_set_watch_cursor(vw);

#ifdef DVR_PSVIEW
    /* check for postscript special case */
    if ( strcmp(format_type_str, vw->dvr_viewer.Ps.ps_str) == 0)
      {
	struct stat 	 	stbuf;
#ifdef __vms__
	unsigned long    	lib_stat;
	struct dsc$descriptor_s file_spec, result_spec;
    	char		 	result_str[255];
	unsigned long 		context;
#endif

	if (!vw->dvr_viewer.Ps.dps_exists)
	  {
	    /* server does not have display ps; return error */
	    dvr_reset_cursor(vw);
	    dvr_error_callback(vw, 0, DVR_NODPSEXT, 0, 0);
	    return(DVR_NODPSEXT);
	  }

	/*  before trying to view ps file, make sure file exists,
	 *  if it doesn't, return file-not-found to provide the behavior
	 *  of normal CDA processing for consistancy
         */
#ifdef __vms__
	/* for VMS, use the system library routine to verify the file
	 * exists; this allows remote file access
	 */
   	file_spec.dsc$b_class   = DSC$K_CLASS_S;
   	file_spec.dsc$b_dtype   = DSC$K_DTYPE_T;
   	file_spec.dsc$w_length  = strlen(filename);
  	file_spec.dsc$a_pointer = filename;

   	result_spec.dsc$b_class   = DSC$K_CLASS_S;
   	result_spec.dsc$b_dtype   = DSC$K_DTYPE_T;
   	result_spec.dsc$w_length  = 255;
  	result_spec.dsc$a_pointer = result_str;

	context = 0;

        if ( (lib_stat =
	      LIB$FIND_FILE(&file_spec, &result_spec, &context, 0, 0, 0, 0))
	      != RMS$_NORMAL)
#else
	if (stat(filename, &stbuf) == -1)
#endif
	  {
	    dvr_reset_cursor(vw);

	    if (VIEWING_PS(vw))
	      {
    		char	dvr_init_page_str[100];
		Arg   	arg_list[3];
		XmString cstr;

	  	dvr_set_viewer_normal(vw);

		if (vw->dvr_viewer.button_box_flag)
		  {

        	    /* grey buttons */
		    dvr_set_buttons_sensitive(vw);

		    /* set page label to 'Page 0 of 0' */
    		    sprintf(dvr_init_page_str,
	        	"%s %d %s %d",
	        	vw->dvr_viewer.page_label_str,
	        	0,
	        	vw->dvr_viewer.of_label_str,
	        	0);

#ifdef CDA_TWOBYTE
		    {
		      int len, stat;
		      cstr = DXmCvtFCtoCS (dvr_init_page_str, &len, &stat);
		    }
#else
		    cstr = XmStringLtoRCreate(dvr_init_page_str , "ISO8859-1");
#endif

		    XtSetArg(arg_list[0], XmNlabelString, cstr);
		    XtSetValues(PageLabel(vw), arg_list, 1);
		    XtFree((char*)cstr);

        	    dvr_move_pg_label(vw);
		  }
	      }

	    dvr_error_callback(vw, 0, CDA_OPENFAIL, 0, 0);
	    return(CDA_OPENFAIL);
	  }

	else
	  {
	    if (!VIEWING_PS(vw))
	      {
	    	/*  switching to ps viewing; take down normal cda viewing
	     	*  window and scroll bars; replace with postscript scroll
	     	*  window
	     	*/
	    	XtUnmanageChild(Work(vw));

	    	if ( (Vscr(vw) != NULL) && XtIsManaged(Vscr(vw)) )
	            XtUnmanageChild(Vscr(vw));

	    	if ( (Hscr(vw) != NULL) && XtIsManaged(Hscr(vw)) )
	            XtUnmanageChild(Hscr(vw));

            	/* fetch ps widget if we have not yet done so */

	    	if (PSscroll(vw) == NULL)
	      	  {
        	    MrmType	dummy_class;

		    Arg sw_arg_list[5];
		    int sw_arg_count = 0;

		    /* if both scrollbars off, turn off auto scrollbars */
		    if ( (!vw->dvr_viewer.h_scroll_flag) &&
			 (!vw->dvr_viewer.v_scroll_flag) )
		      {
                        XtSetArg(sw_arg_list[sw_arg_count],
                                 XmNscrollingPolicy,
				 XmAPPLICATION_DEFINED);
		    	sw_arg_count++;
                      }

   		    if (MrmFetchWidgetOverride(
		      vw->dvr_viewer.drm_h,
		      "ps_scroll_w",
		      (Widget) vw,
		      NULL,
		      sw_arg_list,
		      sw_arg_count,
		      & PSscroll(vw),
	    	      & dummy_class) != MrmSUCCESS)
		      	{
			  /* call error callback, bag */
			  dvr_error_callback(vw, 0, DVR_DRMMAINFETCHFAIL, 0, 0);
			  return;
		      	}
		    else
		      {
			/* get the v and h scrollbars for the ps window */

		        XtSetArg(sw_arg_list[0], XmNhorizontalScrollBar,
				 &PShbar(vw));
		        XtSetArg(sw_arg_list[1], XmNverticalScrollBar,
				 &PSvbar(vw));
		        XtGetValues(PSscroll(vw), sw_arg_list, 2);

			if (PSvbar(vw) != 0)
			  {
			    /* turn off if flag cleared */
			    if ( (!vw->dvr_viewer.v_scroll_flag) &&
			         (vw->dvr_viewer.h_scroll_flag) )
			      {
			    	XtUnmanageChild(PSvbar(vw));
			    	PSvbar(vw) = 0;
			      }
			  }
			if (PShbar(vw) != 0)
			  {
			    if ( (!vw->dvr_viewer.h_scroll_flag) &&
			         (vw->dvr_viewer.v_scroll_flag) )
			      {
			    	/* turn off if flag cleared */
			    	XtUnmanageChild(PShbar(vw));
			        PShbar(vw) = 0;
			      }
                          }
		      }

		    if (vw->dvr_viewer.button_box_flag)
		      {
	 	    	/* fetch cancel button */
   		    	if (MrmFetchWidget(
		   	   vw->dvr_viewer.drm_h,
		   	   "ps_cancel_but",
		   	   Bbox(vw),
		   	   & CancelBut(vw),
	    	   	   & dummy_class) != MrmSUCCESS)
		      	      {
			  	/* call error callback, bag */
			  	dvr_error_callback(vw, 0, DVR_DRMMAINFETCHFAIL, 0, 0);
			  	return;
		      	      }
		    	else
			    XtManageChild(CancelBut(vw));
		      }

		    /*  manage and resize to fit current viewer
		     */
	    	    XtManageChild(PSscroll(vw));

	    	    dvr_resize(vw);

	      	  }

		else
		  {
		    /* ps widget already created, manage and resize */
		    if (vw->dvr_viewer.button_box_flag)
		        XtManageChild(CancelBut(vw));
	    	    XtManageChild(PSscroll(vw));

	    	    dvr_resize(vw);
		  }
	      }

	    (void) dvr_view_ps_file(vw, filename);

	    vw->dvr_viewer.Dvr.FileOpen = TRUE;
	    dvr_set_buttons_sensitive(vw);

	    dvr_reset_cursor(vw);

	    dvr_update_page_number(vw);

	    return(DVR_NORMAL);
	  }
      }

    else /* normal CDA viewing */

#endif /* DVR_PSVIEW */

      {
	/*  if the PS window is managed, take it down and bring back normal
	 *  windows
	 */

#ifdef DVR_PSVIEW
	if (VIEWING_PS(vw))
	  {
	    if (vw->dvr_viewer.button_box_flag)
	        XtUnmanageChild(CancelBut(vw));
	    XtUnmanageChild(PSscroll(vw));
	    XtManageChild(Work(vw));

	    if (Vscr(vw) != NULL)
	        XtManageChild(Vscr(vw));

	    if (Hscr(vw) != NULL)
	        XtManageChild(Hscr(vw));

	    dvr_resize(vw);

	  }
#endif

/* cbr stuff */

   	{
   	MrmType	dummy_class;

   	  if (vw->dvr_viewer.Cbr.mode != FALSE && vw->dvr_viewer.Cbr.EnableRefButs) {

            if (vw->dvr_viewer.button_box_flag) {

              if (PrevRef(vw) == NULL) {
                if (MrmFetchWidget(
                  vw->dvr_viewer.drm_h,
                  "prev_ref_but",
                  Bbox(vw),
                  & PrevRef(vw),
                  & dummy_class) != MrmSUCCESS)
                  {
                    /* call error callback, bag */
		    dvr_error_callback(vw, 0, DVR_DRMMAINFETCHFAIL, 0, 0);
                    return;
                  }
                else
                    XtManageChild(PrevRef(vw));
              } else {
                if ( !(XtIsManaged(PrevRef(vw))) ) {
                    XtManageChild(PrevRef(vw));
                }
              }

              if (NextRef(vw) == NULL) {
                if (MrmFetchWidget(
                  vw->dvr_viewer.drm_h,
                  "next_ref_but",
                  Bbox(vw),
                  & NextRef(vw),
                  & dummy_class) != MrmSUCCESS)
                  {
                    /* call error callback, bag */
                    dvr_error_callback(vw, 0, DVR_DRMMAINFETCHFAIL, 0, 0);
                    return;
                  }
                else
                    XtManageChild(NextRef(vw));
              } else {
                if ( !(XtIsManaged(NextRef(vw))) ) {
                    XtManageChild(NextRef(vw));
                }
              }

   	    }
          } else {
             DvrCbrUnmanageRefs(vw);
          }
        }

/* end cbr stuff */

        function_code = CDA_START;

        /*  set up the item list list for cda_convert; specify
         *  Dvr_Viewer_Backend as the back end procedure
         */
        MAKE_ITEM(item_list, item_count, CDA_INPUT_FORMAT,
	      format_len, format_type_str);
        item_count++;

        MAKE_ITEM(item_list, item_count, CDA_OUTPUT_BACK_END_DOMAIN,
	      ddif_len, ddif_name);
        item_count++;

        MAKE_ITEM(item_list, item_count, CDA_OUTPUT_BACK_END_PROCEDURE,
	      0, Dvr_Viewer_Backend);
        item_count++;

        MAKE_ITEM(item_list, item_count, CDA_OUTPUT_FILE, 0, &output_file);
        item_count++;

  	MAKE_ITEM(item_list, item_count, CDA_MESSAGE_HANDLE, 4,
		  vw->dvr_viewer.message_handle);
	item_count++;

        if (op_file_len != 0)
          {
	    MAKE_ITEM(item_list, item_count, CDA_OPTIONS_FILE,
		op_file_len, options_file);
	    item_count++;
          }

        if (get_rtn != NULL)
          {
	    MAKE_ITEM(item_list, item_count, CDA_INPUT_PROCEDURE,
		0, get_rtn);
	    item_count++;

	    /* get parameter only makes sense if we have a get-routine */

           if (get_param != NULL)
             {
	        MAKE_ITEM(item_list, item_count, CDA_INPUT_PROCEDURE_PARM,
		    4, get_param);
	        item_count++;
             }
          }

        /* else, we have no get get routine, use the input file supplied */

        else
          {
            MAKE_ITEM(item_list, item_count, CDA_INPUT_FILE,
	        name_len, filename);
            item_count++;
          }

        MAKE_ITEM(item_list, item_count, 0, 0, 0);

        vw->dvr_viewer.Dvr.WidgetStatus = 0;

#ifdef DEBUG
printf("\ncall cda_convert\n");
printf("\nfilename is %s\n", filename);
printf("\nformat is  %s\n", format_type);
#endif

        ret_stat = cda_convert ((CDAconstant *) &function_code,
			    (CDAitemlist *) item_list,
			    &vw,
			    (CDAconverterhandle *) &(vw->dvr_viewer.Dvr.ConverterContext));

#ifdef DEBUG
printf("\nstat from cda is %d\n", ret_stat);
#endif

        dvr_set_status(vw, ret_stat);
        if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
          {
	    dvr_error_callback(vw, 0, vw->dvr_viewer.Dvr.WidgetStatus, 0, 0);
            vw->dvr_viewer.Dvr.FileOpen = 0;
	    dvr_reset_cursor(vw);
            return(vw->dvr_viewer.Dvr.WidgetStatus);
          }
        else
          {
	    DvrStructPtr dvr_ptr = &(vw->dvr_viewer.Dvr);

	    /* converter has started correctly, set flag, enable buttons, create gc,
	     * get the first page,

	     * first window if we're managed
             */
     	    dvr_ptr->FileOpen = TRUE;
	    dvr_set_buttons_sensitive(vw);
      	    dvr_ptr->DocRead  = FALSE;
 	    QUE_INIT(dvr_ptr->page_list);
	    QUE_INIT(dvr_ptr->private_structs);

	    ret_stat = dvr_goto_next_page(vw, TRUE);

/* cbr stuff */

            if (vw->dvr_viewer.Cbr.mode == TRUE)
		DvrCbrFirstRef(vw);

/* end cbr stuff */

	    if (DVRFailure(ret_stat))
	        dvr_error_callback(vw, 0, ret_stat, 0, 0);

	    else
	      {
                dvr_create_gc(vw);
	        ret_stat = dvr_initialize_window(vw);

	      }

            dvr_reset_cursor(vw);
	    dvr_update_page_number(vw);

	    return(ret_stat);
          }
      }
}



/*
**++
**  ROUTINE NAME:
**	DvrRegisterClass()
**
**  FUNCTIONAL DESCRIPTION:
**      register the viewer's class so that it can be created
**	through DRM/UIL.
**
**  FORMAL PARAMETERS:
**    	none
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrRegisterClass()

{

    if (MrmRegisterClass(MrmwcUnknown,		/* use for all user-defineds */
			 DvrViewerClassStr, 	/* our class string	     */
			 "DvrViewerCreate", 	/* low level entry point str */
			 (Widget (*)()) DvrViewerCreate,   	/* low level entry point adr */
			 (WidgetClass) dvr_viewer_class_rec_addr) /* pointer to class record   */
			 != MrmSUCCESS)
	return(DVR_INTERNALERROR);

    return(DVR_NORMAL);

}


/*
**++
**  ROUTINE NAME:
**	DvrCloseFile()
**
**  FUNCTIONAL DESCRIPTION:
**      Close the file and cleare the screen.
**
**	NOTE: internally, all document closes go through this routine
**	      (DvrViewerFile() calls DvrCloseFile() ), except in the
**	      the special case where a PostScript file is open when
**	      DvrViewerFile() is called; in which case the document is
**	      closed in DvrViewerFile(), see DvrViewerFile()
**
**  FORMAL PARAMETERS:
**    	none
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrCloseFile(vw)
	DvrViewerWidget vw;
{
    XmString cstr;

    /* return invalid address if widget is zero */
    if (vw == 0) return(DVR_INVADDR);

    /* set page number to zero */
    vw->dvr_viewer.page_number = 0;

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
	Arg arg_list[5];
    	char dvr_init_page_str[100];
   	CDAstatus free_stat;

        dvr_set_watch_cursor(vw);

	vw->dvr_viewer.Dvr.FileOpen = FALSE;

	/* get rid of doc info memory */
	free_stat = dvr_free_doc_info(vw);
    	if (DVRFailure(free_stat))
	    dvr_error_callback(vw, 0, free_stat, 0, 0);

#ifdef DVR_PSVIEW
	/* check for special PS case */
	if (VIEWING_PS(vw))
	  {
	    Arg arg_list[5];
	    int arg_count=0;

	    /* check for case where PS is still churning; if so, abort! */
	    if (vw->dvr_viewer.Ps.waiting)
		PSViewAbort(PSwin(vw));

	    dvr_view_ps_file(vw, NULL);
	    vw->dvr_viewer.Ps.current_page = 0;
	    vw->dvr_viewer.Ps.last_page = 0;

	    if (!vw->dvr_viewer.destroy_flag)
	        /* manage normal cda windows */
	    	dvr_set_viewer_normal(vw);
	  }

	else /* normal CDA viewing */
#endif
	  {
    	    int function_code = CDA_STOP;
    	    CDAstatus stat;

	    if (!vw->dvr_viewer.destroy_flag)
              {
	        /* set scroll bars to full (empty) */
	    	XtSetArg(arg_list[0], XmNvalue, 0);
	     	XtSetArg(arg_list[1], XmNsliderSize, DVR_MAX_SCROLL_VALUE);
	    	if (Hscr(vw) != NULL)
	            XtSetValues(Hscr(vw), arg_list, 2);
	    	if (Vscr(vw) != NULL)
	            XtSetValues(Vscr(vw), arg_list, 2);

		/* if we're managed, clear window */
		if (XtIsManaged(vw))
		  {
		    if (vw->dvr_viewer.Dvr.erase_gc_id != NULL)
			XFillRectangle(XtDisplay(vw), Win(vw),
			    vw->dvr_viewer.Dvr.erase_gc_id, 0, 0,
			    XtWidth(Work(vw)),
			    XtHeight(Work(vw)) );
		  }

	    	/* get rid of colors, etc */
	        dvr_cleanup_page(vw);

	    	/* free GC's if there are some */
	    	if (vw->dvr_viewer.Dvr.GcId != NULL)
	          {
	            XFreeGC(XtDisplay(vw), vw->dvr_viewer.Dvr.GcId);
	            vw->dvr_viewer.Dvr.GcId = NULL;

	            vw->dvr_viewer.current_font_index = UNDEFINED_FONT_INDEX;

	            XFreeGC(XtDisplay(vw), vw->dvr_viewer.Dvr.erase_gc_id);
	            vw->dvr_viewer.Dvr.erase_gc_id = NULL;
	          }
              }

	    /* unload fonts used for current file; don't dealloc table */
	    (void) dvr_unload_fonts ( vw );

	    /* free memory associated with reading in file before stopping
	       converter */
	    (void) dvr_free_file_memory ( vw );

            vw->dvr_viewer.Dvr.WidgetStatus = 0;
    	    stat = cda_convert ((CDAconstant *) &function_code,
				0,
				&vw,
		       		(CDAconverterhandle *) &(vw->dvr_viewer.Dvr.ConverterContext));

	    dvr_set_status(vw, stat);
    	    if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
	      {
	        dvr_error_callback(vw, 0, vw->dvr_viewer.Dvr.WidgetStatus, 0, 0);
	      }

	    /* Note: DVR structure queues are reinitialized in new file routine */

          }

	/* following operations needed for both ps and normal cda viewing */

	/* clear out all of the DVR structure now that we're done deallocating
	   the memory used for this file, including stopping the converter */
    	memset ( &(vw->dvr_viewer.Dvr),
	         (int) NULL,
	         sizeof(DvrStruct) );

        if ( (vw->dvr_viewer.button_box_flag) &&
	     (!vw->dvr_viewer.destroy_flag) )
	  {
            /* grey buttons */
	    dvr_set_buttons_sensitive(vw);

	    /* set page label to 'Page 0 of 0' */
    	    sprintf(dvr_init_page_str,
	        "%s %d %s %d",
	        vw->dvr_viewer.page_label_str,
	        0,
	        vw->dvr_viewer.of_label_str,
	        0);

#ifdef CDA_TWOBYTE
	    {
	      int len, stat;
	      cstr = DXmCvtFCtoCS (dvr_init_page_str, &len, &stat);
	    }
#else
	    cstr = XmStringLtoRCreate(dvr_init_page_str , "ISO8859-1");
#endif

	    XtSetArg(arg_list[0], XmNlabelString, cstr);
	    XtSetValues(PageLabel(vw), arg_list, 1);
	    XtFree ((char *)cstr);

            dvr_move_pg_label(vw);
	  }

	dvr_reset_cursor(vw);
        return(DVR_NORMAL);
      }

    else
	return(DVR_FILENOTOPEN);


}


/*
**++
**  ROUTINE NAME:
**	dvr_free_doc_info(vw)
**
**  FUNCTIONAL DESCRIPTION:
**
**      free doc info buffer
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget	vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
CDAstatus dvr_free_doc_info(vw)
    DvrViewerWidget vw;

  {
    CDAstatus stat;

    if (vw->dvr_viewer.doc_info_text_buffer)
      {
	CDAsize len;

	len = (CDAsize) strlen(vw->dvr_viewer.doc_info_text_buffer)+1;
	stat = DVR_DEALLOCATE_MEMORY(len,
				     &vw->dvr_viewer.doc_info_text_buffer
				     );
      }
    else
      	stat = DVR_NORMAL;

    vw->dvr_viewer.doc_info_text_buffer = 0;
    return (stat);
  }



/*  ultility routine used by dvr__get_dpi_from_font() to locate the
 *  rightmost ocurrence of a character in a string
 */
int right_index(string, chr)
    char *string;
    char chr;

{
    int index;
    unsigned short found = 0;

    index = strlen(string) - 1;

    while( (index >= 0) && (found == 0) )
      {
	if (string[index] == chr)
	    found = 1;
	else
	    index = index - 1;
      }

    if (found == 0)
    	return (-1);
    else
	return (index);
}


/*
**++
**  ROUTINE NAME:
**	dvr__get_dpi_from_font(font_name, font_x_dpi, font_y_dpi)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine will parse an XLFD font-string looking for the dpi fields,
**	and will then, return the dpi numbers from the XLFD string passed in;
**	if a non valid font string is passed in, defaults are filled in, and
**	the routine returns;
**
**  FORMAL PARAMETERS:
**     	char *font_name;
**    	int  *font_x_dpi;
**    	int  *font_y_dpi;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr__get_dpi_from_font(font_name, font_x_dpi, font_y_dpi)
    char *font_name;
    int  *font_x_dpi;
    int  *font_y_dpi;

{
    unsigned long 	num_fonts = 0;
    char 		*valid_digits = "0123456789";
    int 		i, pos, length;
    char 		dpi_str[20];
    char 		font_string[256];
    int 		dpi_num = 0;

    /* set defaults in case we fail */
    *font_x_dpi = 75;
    *font_y_dpi = 75;

    /* copy the font returned */
    strcpy(font_string, font_name);

    for (i=1; i<5; i++)
      {
	/* strip the characters following the final
	 * four dashes; that should leave us with the
	 * dpi on the end of the string following a dash
	 */

	pos = right_index(font_string, '-');
   	if (pos != -1)
	    font_string[pos] = '\0';
      }

    /*  if there were not 4 dashes on the end, return,
     *  we can't go any further
     */
    if (pos == -1)
   	return;

    /* get the position of the dash in front of the y dpi */
    pos = right_index(font_string, '-');

    /* if we didn't get that dash, return */
    if (pos == -1)
	return;

    /* go to the character following the dash */
    pos = pos+1;

    /* the y dpi string is from pos to the end of the string */
    length = strlen(font_string) - pos;

    /* paranoid */
    if (length < 1)
	return;

    /* get the y dpi string */
    for (i=0; i < length; i++)
      {
	dpi_str[i] = font_string[pos];

	/* anything other than digits, return */
   	if (strchr(valid_digits, dpi_str[i])==0)
	    return;

	pos++;
      }

    /* null terminate */
    dpi_str[length] = '\0';

    /* get number from string */
    sscanf(dpi_str, "%d", &dpi_num);

    if (dpi_num > 0)
	/* dpi is valid, stuff it in return var */
	*font_y_dpi = dpi_num;


    /* now get the x dpi, first strip off the y dpi */
    pos = right_index(font_string, '-');
    if (pos != -1)
	font_string[pos] = '\0';

    pos = right_index(font_string, '-');

    /*  if there was not another dash before the y dpi return,
     *  we can't go any further
     */
    if (pos == -1)
   	return;


    /* go to the character following the dash */
    pos = pos+1;

    /* the x dpi string is now from pos to the end of the string */
    length = strlen(font_string) - pos;

    /* paranoid */
    if (length < 1)
	return;

    /* get the x dpi string */
    for (i=0; i < length; i++)
      {
	dpi_str[i] = font_string[pos];

	/* anything other than digits, return */
   	if (strchr(valid_digits, dpi_str[i])==0)
	    return;

	pos++;
      }

    /* null terminate */
    dpi_str[length] = '\0';

    /* get number from string */
    sscanf(dpi_str, "%d", &dpi_num);

    if (dpi_num > 0)
	/* dpi is valid, stuff it in return var */
	*font_x_dpi = dpi_num;

    /* now we should be all set, rejoice! return! */
}


/*
**++
**  ROUTINE NAME:
**	dvr__select_dpi_from_fonts
**
**  FUNCTIONAL DESCRIPTION:
**	this routine will fill in the viewer widget's dpi fields
**	choosing the closest matching font from font_list
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget vw
**	char		**font_list;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
void dvr_select_dpi_from_fonts(vw, font_list, num_fonts)
    DvrViewerWidget vw;
    char	    **font_list;
    int		    num_fonts;
{
    int next_x_dpi, next_y_dpi;
    int new_x_dpi, new_y_dpi;
    int	i;

    for (i=0; i<num_fonts; i++)
      {
        dvr__get_dpi_from_font(font_list[i],
			     &next_x_dpi,
			     &next_y_dpi);
	if ( (next_x_dpi == vw->dvr_viewer.x_dpi) &&
	     (next_y_dpi == vw->dvr_viewer.y_dpi) )
	    /* screen dpi matches font dpi, return */
	    return;

	if (i == 0)
	  {
	    new_x_dpi = next_x_dpi;
	    new_y_dpi = next_y_dpi;
	  }
	else
	  {
	    if (abs(vw->dvr_viewer.x_dpi - next_x_dpi) <
		abs(vw->dvr_viewer.x_dpi - new_x_dpi) )
		new_x_dpi = next_x_dpi;
	    if (abs(vw->dvr_viewer.y_dpi - next_y_dpi) <
		abs(vw->dvr_viewer.y_dpi - new_y_dpi) )
		new_y_dpi = next_y_dpi;
	  }

      }

    vw->dvr_viewer.x_dpi = new_x_dpi;
    vw->dvr_viewer.y_dpi = new_y_dpi;

}



/*
**++
**  ROUTINE NAME:
**	dvr_get_dpi_and_fonts(vw)
**
**  FUNCTIONAL DESCRIPTION:
**
**      query server for dpi and font information; fill in these fields
**	in viewer widget: x_dpi, y_dpi, available_fonts
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget	vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
void dvr_get_dpi_and_fonts(vw)
    DvrViewerWidget	vw;

{
    float	    dpi_float;
    char	    font_list_string[100];
    unsigned long   num_matching_fonts = 0;
    char	    **matching_font_list;
    int	            font_x_dpi, font_y_dpi, i;

    /* compute the x dpi of the screen; first get the screen width in mm */
    dpi_float = (float)
	XWidthMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw)));

    /* now convert to inches */
    dpi_float = dpi_float / 25.4;

    /* now divide by number of pixels by inches to get the DPI */
    dpi_float = (float)
	XWidthOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw))) /
	dpi_float;

    /* round off and store in the widget */
    vw->dvr_viewer.x_dpi = (int) (dpi_float + 0.5) ;

    /* compute the y dpi of the screen; first get the screen height in mm */
    dpi_float = (float)
	XHeightMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw)));

    /* now convert to inches */
    dpi_float = dpi_float / 25.4;

    /* now divide by number of pixels by inches to get the DPI */
    dpi_float = (float)
	XHeightOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw))) /
	dpi_float;

    /* round off and store in the widget */
    vw->dvr_viewer.y_dpi = (int) (dpi_float + 0.5) ;

    /* first find all courier 12 fonts */
    matching_font_list = XListFonts(XtDisplay(vw),
					vw->dvr_viewer.default_font_str,
					10000,	/* max # of fonts to list! */
					(int *) &num_matching_fonts );

    if (num_matching_fonts != 0)
      {
        /* use closest matching font for dpi */
	(void) dvr_select_dpi_from_fonts(vw, matching_font_list,
					    num_matching_fonts);

        /* free X storage */
        XFreeFontNames(matching_font_list);
      }

    /*  match all fonts with correct dpi for server
     *	"-*-*-*-*-*-*-*-*-xres-yres-*-*-*-*"
     */
    sprintf(font_list_string,
	    "%s%d%s%d%s",
	    "-*-*-*-*-*-*-*-*-",
	    vw->dvr_viewer.x_dpi,
	    "-",
	    vw->dvr_viewer.y_dpi,
	    "-*-*-*-*");

    /* find out all the available fonts on the server to
     * pass to engine
     */

     /* This num_fonts field should have been declared int, but since it's
      * long, initializing it to zero works around the problem.  Since
      * XListFonts expects a pointer to int, the top half of num_fonts is
      * not written and may contain random garbage.
      */

    vw->dvr_viewer.num_fonts = 0;
    vw->dvr_viewer.available_fonts = XListFonts(XtDisplay(vw),
					font_list_string,
					10000,	/* max # of fonts to list! */
					(int *) &(vw->dvr_viewer.num_fonts) );

    if (vw->dvr_viewer.num_fonts)
	return;

    /*  no fonts matching our default font on the system, and no fonts
     *  on the system matching the dpi of the screen; next
     *  ask for any fonts matching the XLFD naming format
     */
    num_matching_fonts = 0;
    matching_font_list = XListFonts(XtDisplay(vw),
				    "-*-*-*-*-*-*-*-*-*-*-*-*-*-*",
				    10000,	/* max # of fonts to list! */
				    (int *) &num_matching_fonts );
    if (num_matching_fonts == 0)
      {
	/*  if we still have no matches at this point,
	 *  return any fonts that are on the server
	 */
        vw->dvr_viewer.num_fonts = 0;
      	vw->dvr_viewer.available_fonts = XListFonts(XtDisplay(vw),
					"*",
					10000,	/* max # of fonts to list! */
					(int *) &(vw->dvr_viewer.num_fonts) );

	if (!vw->dvr_viewer.num_fonts)
	    /* if still no fonts, issue warning, viewer will return 0 */
	    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)vw),
		      "noFonts",
		      "xListFonts",
		      "XError",
		      "No fonts available on server",
		      (String *) NULL,
		      (Cardinal *) NULL);


	/*  return, using the dpi returned from the server and any fonts that
	 *  it can find
	 */
	return;
      }

      /*  at this point, we have a matching fonts list, for simplicity,
       *  we choose the first matching font and parse it for it's resolution
       */
      dvr__get_dpi_from_font(matching_font_list[0],
			     &font_x_dpi,
			     &font_y_dpi);

      /* free X storage */
      XFreeFontNames(matching_font_list);

      /*  if the font dpi is larger than the screen dpi, then substitute
       *  the font dpi for our internal calculations; this "lying" to our
       *  formatting engine will provide a more WYSIWYG viewer
       */

      /* use font dpi for all of our calculations */
      vw->dvr_viewer.x_dpi = font_x_dpi;
      vw->dvr_viewer.y_dpi = font_y_dpi;

      /* now redo our font lookup */

      /*  match all fonts with this dpi for server
       *	"-*-*-*-*-*-*-*-*-xres-yres-*-*-*-*"
       */
      sprintf(font_list_string,
	    "%s%d%s%d%s",
	    "-*-*-*-*-*-*-*-*-",
	    vw->dvr_viewer.x_dpi,
	    "-",
	    vw->dvr_viewer.y_dpi,
	    "-*-*-*-*");

      /* find out all the available fonts on the server to
       * pass to engine
       */

      vw->dvr_viewer.num_fonts = 0;
      vw->dvr_viewer.available_fonts = XListFonts(XtDisplay(vw),
					font_list_string,
					10000,	/* max # of fonts to list! */
					(int *) &(vw->dvr_viewer.num_fonts) );

      /* that's all folks! */
}




/*
**++
**  ROUTINE NAME:
**	dvr_fill_char_buffer(vw, item_type, tab_str, new_buffer_addr, byte_size_addr)
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine takes a buffer and appends the information
**	specified by item_type from the document loaded in vw.
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget	vw;
**      CDAconstant	item_type;
**	char		*tab_str;
**	char		**new_buffer_addr;
**      CDAsize		*byte_size_addr;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_fill_char_buffer(vw, item_type, tab_str, new_buffer_addr, byte_size_addr)
    DvrViewerWidget	vw;
    CDAconstant		item_type;
    char		*tab_str;
    char		**new_buffer_addr;
    CDAsize		*byte_size_addr;
{
    char 		*new_buffer = *new_buffer_addr;
    CDAsize		array_size;
    CDAaddress		*item_address;
    CDAsize		item_length;
    CDAsize		agg_index;
    CDAconstant		add_info;
    CDAstatus		stat;
    CDAsize		increment_size;
    CDAsize		byte_size = *byte_size_addr;
    CDAagghandle	agg_handle;

    /*  all of the info is extracted from the header aggregate
     *  except the product-name which is extracted from the descriptor
     */
    if (item_type == DDIF_DSC_PRODUCT_NAME)
	agg_handle = vw->dvr_viewer.Dvr.dsc_agg_handle;
    else
	agg_handle = vw->dvr_viewer.Dvr.dhd_agg_handle;
     /* find out how big the array is (date is not an array, set to empty) */
    if (item_type == DDIF_DHD_DATE)
	stat = CDA_EMPTY;
    else
        stat = cda_get_array_size(&agg_handle,
			          &item_type,
				  &array_size);

    if ( DVRSuccess(stat) || (stat == CDA_EMPTY) )
      {
	/*  if stat is empty, then this is not an array, just get
	 *  the one item.
	 */
	if (stat == CDA_EMPTY)
	    array_size = 1;

	for(agg_index = 0; agg_index < array_size; agg_index++)
          {
	    /*  locate the item and add it to our buffer */
    	    stat = cda_locate_item (&vw->dvr_viewer.Dvr.root_agg_handle,
				    (CDAagghandle *) &agg_handle,
				    &item_type,
				    (CDAaddress *) &item_address,
				    &item_length,
				    (CDAindex *) &agg_index,
				    &add_info);

	    if ( DVRSuccess(stat) && (stat != CDA_EMPTY) )
	      {
		increment_size = item_length;

		/*  if we're in an array, increment size may be larger
		 *  becuase we're dealing with multiple lines with extra
		 *  spaces and carriage returns
		 */
		if( (array_size != 1) && (agg_index != (array_size-1)) )
		    increment_size = increment_size+1;

		if (agg_index != 0)
		    increment_size = increment_size+strlen(tab_str);

		stat = realloc_memory(	(void **) &new_buffer,
					&byte_size,
					increment_size );
		if (DVRSuccess(stat))
		  {
		    /*  if we're in an array, and this is not the first
		     *  element; then the element has no header; pad it with
		     *  the tab string;
		     */
		    if (agg_index != 0)
			strcat(new_buffer, tab_str);

		    /* put in the data returned from cda */
		    strncat(new_buffer, (CDAenvirontext *) item_address, item_length);

		    /*  if we're in an array, and this is not the last
		     *  element; then we need to add a carriage return;
		     */
		    if( (array_size != 1) && (agg_index != (array_size-1)) )
			strcat(new_buffer, "\n");
		  }
	      }


	  }
      }

    /* return the updated buffer and byte size */
    *new_buffer_addr = new_buffer;
    *byte_size_addr  = byte_size;
}


/*
**++
**  ROUTINE NAME:
**	dvr_fill_char_buffer_ps(info_str, tab_str, new_buffer_addr, byte_size_addr)
**
**  FUNCTIONAL DESCRIPTION:
**      this routine performs the same function that dvr_fill_char_buffer()
**	does; it behaves differently for ps; since we already have our info
**	string from ps, we just need to stuff it into the buffer; no calls
**	to cda are necessary;
**
**  FORMAL PARAMETERS:
**    	char		*info_str;
**	char		*tab_str;
**	char		**new_buffer_addr;
**      CDAsize		*byte_size_addr;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

dvr_fill_char_buffer_ps(info_str, tab_str, new_buffer_addr, byte_size_addr)
    char		*info_str;
    char		*tab_str;
    char		**new_buffer_addr;
    CDAsize		*byte_size_addr;
{
    char 		*new_buffer = *new_buffer_addr;
    CDAstatus		stat;
    CDAsize		increment_size;
    CDAsize		byte_size = *byte_size_addr;


    /*  bump up buffer size based on how big new string is;
     *  (plus one for \n)
     */
    increment_size = strlen(info_str) +1;

    stat = realloc_memory((void **) &new_buffer,
			  &byte_size,
			  increment_size );
    if (DVRSuccess(stat))
      {
	/* put in the data passed in */
	strcat(new_buffer, info_str);

      }

    /* return the updated buffer and byte size */
    *new_buffer_addr = new_buffer;
    *byte_size_addr  = byte_size;
}



/*
**++
**  ROUTINE NAME:
**	DvrDocumentInfo(vw, char_buffer)
**
**  FUNCTIONAL DESCRIPTION:
**      return information from the currently open document's
**	header aggregate: title, author, version, date; buffer will
**	be deallocated when file is closed;
**
**  FORMAL PARAMETERS:
**    	none
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrDocumentInfo (vw, char_buffer)
	DvrViewerWidget vw;
	CDAenvirontext	**char_buffer;

{
    CDAconstant		item_type;
    CDAstatus		stat;
    CDAsize		increment_size;
    CDAsize		byte_size;
    char		*new_buffer;
    Boolean		info_found = FALSE;
    CDAsize		save_byte_size;

    int 		ps_doc_info_found = 0;
    char		**ps_info_ptr;

    int			ret_status;
    MrmCode		dummy_type;		/*  Dummy parameter passed to the fetch literal routine */

    /*
     *  define strings for UIL fetches;
     */

    char *dvr_product_name_str;
    char *dvr_title_str	      ;
    char *dvr_author_str      ;
    char *dvr_version_str     ;
    char *dvr_date_str	      ;
    char *dvr_info_tab_str    ;

    /* check for bugus parameters */
    if ( (vw == NULL) || (char_buffer == NULL) )
	return(DVR_BADPARAM);

    /* make sure a file is open */
    if (!vw->dvr_viewer.Dvr.FileOpen)
	return(DVR_FILENOTOPEN);

    if (vw->dvr_viewer.doc_info_text_buffer)
      {
    	/* return the allocated buffer */
    	*char_buffer = vw->dvr_viewer.doc_info_text_buffer;
	return(DVR_NORMAL);
      }

#ifdef DVR_PSVIEW
    if (VIEWING_PS(vw))
      {
	/*  for ps, call entry point to get doc info; if there is none,
	 *  go no further, return error
	 */
	ps_doc_info_found = PSViewGetDocumentInfo(PSwin(vw), &ps_info_ptr);
	if (ps_doc_info_found == 0)
	    return(DVR_NODISPCONT);
	else
	  {
	    info_found = TRUE;
	  }
      }
#endif

    /* fetch all constant strings from UIL */

    /*  for each string, get the literal from MRM. */

    /*
     * Fetch the product name string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$product_name_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&dvr_product_name_str,		  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch title string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$title_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&dvr_title_str,			  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch the author string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$author_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&dvr_author_str,			  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch the version string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$version_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&dvr_version_str,		  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch the date string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$date_str",				/* index		*/
			XtDisplay(vw),				/* display		*/
			&dvr_date_str,			  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /*
     * Fetch the info table string
    */

    ret_status = MrmFetchLiteral(
			vw->dvr_viewer.drm_h,			/* UID hierarchy id	*/
			"dvr$info_tab_str",			/* index		*/
			XtDisplay(vw),				/* display		*/
			&dvr_info_tab_str,		  	/* value returned	*/
			&dummy_type);				/* type of data returned*/

    if (ret_status != MrmSUCCESS)
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_DRMSTRINGFETCHFAIL;

    /* allocate the memory and put the title header in the text buffer */

    new_buffer = NULL;
    byte_size = 0;
    increment_size = strlen(dvr_product_name_str)+3;
    stat = realloc_memory((void **) &new_buffer,
			  &byte_size,
			  increment_size );
    strcpy(new_buffer, dvr_product_name_str);

#ifdef DVR_PSVIEW

    if (VIEWING_PS(vw))
      {
	/*  for ps, check to see if we we have any data for this
	 *  field; if so call routine to fill it in
	 */
	if (ps_info_ptr[PSVWGCreator] != NULL)
	    dvr_fill_char_buffer_ps(ps_info_ptr[PSVWGCreator],
				    dvr_info_tab_str,
				    &new_buffer,
				    &byte_size);
      }

    else /* normal CDA Viewing */

#endif

      {

    	/* get the product name aggregate from the document and fill it in the buffer */
    	item_type = DDIF_DSC_PRODUCT_NAME;
    	save_byte_size = byte_size;
    	dvr_fill_char_buffer(vw, item_type, dvr_info_tab_str, &new_buffer, &byte_size);

    	/* if info has been found, set flag */
    	if (save_byte_size != byte_size)
      	    info_found = TRUE;
      }

    /* add carriage returns for white space */
    strcat(new_buffer, "\n\n");

    /* allocate the memory and put the title header in the text buffer */
    increment_size = strlen(dvr_title_str)+2;
    stat = realloc_memory((void **) &new_buffer,
			  &byte_size,
			  increment_size );
    strcat(new_buffer, dvr_title_str);

#ifdef DVR_PSVIEW

    if (VIEWING_PS(vw))
      {
	if (ps_info_ptr[PSVWGTitle] != NULL)
	    dvr_fill_char_buffer_ps(ps_info_ptr[PSVWGTitle],
				    dvr_info_tab_str,
				    &new_buffer,
				    &byte_size);
      }

    else /* normal CDA Viewing */
#endif

      {
    	/* get the author aggregate from the document, and fill it in the buffer */
    	item_type = DDIF_DHD_TITLE;
    	save_byte_size = byte_size;
    	dvr_fill_char_buffer(vw, item_type, dvr_info_tab_str, &new_buffer, &byte_size);

    	/* if info has been found, set flag */
    	if (save_byte_size != byte_size)
	    info_found = TRUE;
      }


    /* add carriage returns for white space */
    strcat(new_buffer, "\n\n");

    /* allocate the memory and put the author header in the text buffer */
    increment_size = strlen(dvr_author_str)+2;
    stat = realloc_memory((void **) &new_buffer,
			  &byte_size,
			  increment_size );
    strcat(new_buffer, dvr_author_str);

#ifdef DVR_PSVIEW

    if (VIEWING_PS(vw))
      {
	if (ps_info_ptr[PSVWGFor] != NULL)
	    dvr_fill_char_buffer_ps(ps_info_ptr[PSVWGFor],
				    dvr_info_tab_str,
				    &new_buffer,
				    &byte_size);
      }

    else /* normal CDA Viewing */

#endif

      {
    	/* get the author aggregate from the document, and fill it in the buffer */
    	item_type = DDIF_DHD_AUTHOR;
    	save_byte_size = byte_size;
    	dvr_fill_char_buffer(vw, item_type, dvr_info_tab_str, &new_buffer, &byte_size);

    	/* if info has been found, set flag */
    	if (save_byte_size != byte_size)
	    info_found = TRUE;
      }

    /* add carriage returns for white space */
    strcat(new_buffer, "\n\n");

    /* allocate the memory and put the version header in the text buffer */
    increment_size = strlen(dvr_version_str)+2;
    stat = realloc_memory((void **) &new_buffer,
			  &byte_size,
			  increment_size );
    strcat(new_buffer, dvr_version_str);

#ifdef DVR_PSVIEW
    if (VIEWING_PS(vw))
      {
	/* no ps version str */
      }

    else /* normal CDA Viewing */
#endif

      {
    	/* get the version aggregate from the document, and fill it in the buffer */
    	item_type = DDIF_DHD_VERSION;
    	save_byte_size = byte_size;
    	dvr_fill_char_buffer(vw, item_type, dvr_info_tab_str, &new_buffer, &byte_size);

    	/* if info has been found, set flag */
    	if (save_byte_size != byte_size)
	    info_found = TRUE;
      }

    /* add carriage returns for white space */
    strcat(new_buffer, "\n\n");

    /* allocate the memory and put the date header in the text buffer */
    increment_size = strlen(dvr_date_str);
    stat = realloc_memory((void **) &new_buffer,
			  &byte_size,
			  increment_size );
    strcat(new_buffer, dvr_date_str);

#ifdef DVR_PSVIEW
    if (VIEWING_PS(vw))
      {
	if (ps_info_ptr[PSVWGCreationDate] != NULL)
	    dvr_fill_char_buffer_ps(ps_info_ptr[PSVWGCreationDate],
				    dvr_info_tab_str,
				    &new_buffer,
				    &byte_size);
      }

    else /* normal CDA Viewing */
#endif
      {
    	/* get the version aggregate from the document, and fill it in the buffer */
    	item_type = DDIF_DHD_DATE;
    	save_byte_size = byte_size;
    	dvr_fill_char_buffer(vw, item_type, dvr_info_tab_str, &new_buffer, &byte_size);

    	/* if info has been found, set flag */
    	if (save_byte_size != byte_size)
	    info_found = TRUE;
      }


    XtFree(dvr_product_name_str);
    XtFree(dvr_title_str);
    XtFree(dvr_author_str);
    XtFree(dvr_version_str);
    XtFree(dvr_date_str);
    XtFree(dvr_info_tab_str);

    if (info_found)
      {
    	/* return the allocated buffer */
    	*char_buffer = new_buffer;
	vw->dvr_viewer.doc_info_text_buffer = new_buffer;
	return(DVR_NORMAL);
      }

    else
      {
        /* no info found, release memory, and return empty */

	int context = 0;

	stat = DVR_DEALLOCATE_MEMORY(byte_size, &new_buffer);
	if (DVRFailure(stat))
	    return(stat);
	else
	    return(DVR_NODISPCONT);
      }

}
