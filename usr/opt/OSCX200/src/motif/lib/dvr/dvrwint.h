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
#ifndef DVR_VIEW_TYPES
#define DVR_VIEW_TYPES

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
**  MODULE NAME
**	dvrwint.h
**
**  FACILITY:
**
**      DVR_ CDA DECwindows Viewer
**
**  ABSTRACT:
**
**      Internal definitions for DECwindows Viewer modules.  Created by merging
**	the former dvr_constants.h, dvr_structs.h, and dvr_create.h.
**
** ENVIORNMENT:
**	vms, ultrix, os/2
**
**  AUTHORS:
**
**      Peter Derr, Dennis McEvoy, Marc Carignan and others
**
**
**  CREATION DATE:     1-Mar-89
**
**  MODIFICATION HISTORY:
**
**	V02-001		PD0001		Peter Derr		3-May-1989
**		Initial creation
**			DM0002		Dennis McEvoy		9-May-1989
**		change NOWIDGET_UIL flag to DECWINDOWS_V1;
**		move UID file names into this module;
**
**	V02-002		DM0002		Dennis McEvoy		19-Oct-89
**		add postscript widget children
**
**	V02-003		DM0003		Dennis McEvoy
**		add new structure fields for new resources 	22-Feb-90
**
**	V02-004		DM0004		Dennis McEvoy
**		changes for OS/2 port 				05-Mar-90
**
**	V02-005		DM0005		Dennis McEvoy
**		use cda_malloc/cda_fee on all platforms 	05-Apr-90
**
**	V02-006		DM0006		Dennis McEvoy
**		add multithead vars for os/2
**		switch to new cda toolkit typedef names (cda_) 	13-Apr-90
**
**	V02-007		DM0007		Dennis McEvoy
**		clean up #ifdefs
**		move in private os/2 client window procs
**					 from ptp h file 	28-Jun-90
**	V02-008		DM0008		Dennis McEvoy
**		merge in Micky's callback changes		29-Jun-90
**	       		MB0001		Micky Balladelli
**		add expose event callback as dvr_viewer.expose_callback
**	       		MB0002		Micky Balladelli
**		add motion event callback as dvr_viewer.mouse_motion_callback
**	       		MB0003		Micky Balladelli
**		add buttons events callback as dvr_viewer.buttons_callback
**	       		MB0004		Micky Balladelli
**		add scroll bar events callback as dvr_viewer.scroll_bar_callback
**
**	V02-009		DM0009		Dennis McEvoy		16-Jul-90
**		add vertical dpi to support non 1:1 pix displays
**
**	V02-010		SJM007		Stephen Munyan		19-Jun-1990
**		convert to Motif
**
**	V02-011		BFB001		Barbara Bazemore	21-Aug-1990
**		merge in Kenji Yamada-san's changes for scaled image support
**
**	V02-012		SJM000		Stephen Munyan		09-Oct-1990
**		merge in the CBR changes from Charlie Chan
**
**	V02-013		SJM000		Stephen Munyan		05-Nov-1990
**		add PROTO statements for CBR.
**
**	V02-014		BFB002		Barbara Bazemore	09-Nov-1990
**		modify image support to use X bitmap instead of IDS widget
**
**	V02-015		DAM015		Dennis McEvoy		26-Nov-1990
**		correct changes in 014 to work on os/2
**
**	V02-016		SJM000		Stephen Munyan		28-Nov-1990
**		merge in proto fixes from XUI version
**
**	V02-012		JJT0001		Jeff Tancill		27-Nov-90
**		correct function protos to match defintion in source code.
**
**	V02-013		DAM0001		dam			28-Nov-90
**		fix protos
**
**	V02-017		DAM017		dam			03-Dec-1990
**		cleanup/clarify platform specific protos
**
**	V02-018		DAM002		dam	  		08-Jan-1991
**		add message log
**
**	V02-019		SJM000		Stephen Munyan		19-Feb-1991
**		added presentation surface to image private area
**		since it must be deleted after the image rendering
**		to avoid problems with Image Services.
**
**	V02-020		KMRK		Kathy Robbinson		23-Feb-1991
**		added two new fields to img_private
**
**	V02-021		DAM002		dam	  		01-Mar-1991
**		convert to new typedefs
**
**	V02-022		DAM002		dam			04-Mar-1991
**		clean up typedefs
**
**	V02-023		DAM002		dam	  		13-Mar-1991
**		add destroy flag
**
**	V02-024		DAM002		dam	  		21-Mar-1991
**		add txt private fields for changebars
**
**	V02-025		DAM002		dam	  		28-Mar-1991
**		add new vars for ps scrolling
**
**	V02-026		DAM002		dam	  		03-Apr-1991
**		cleanup typedefs
**
**	V02-027		CJR000		Chris Ralto		04-Apr-1991
**		Add two new args to dvr_stroke_path prototype;
**		add new prototype for new function dvr_stroke_polyline.
**
**	V02-028		RKN000		Ram Kumar Nori		16-Apr-1991
**		Add new fields in Private structure for processing
**		arrow heads.
**
**	V02-029		RAM000		Ralph A Mack		24-Apr-1991
**		Add initial #ifdefs and #defines for MS-Windows
**
**	V02-030		RKN000		Ram Kumar Nori		06-May-1991
**		Add new field in Arc Private structure for processing
**		arrow heads On Arcs.
**
**	V02-031		RAM000		Ralph A. Mack		14-May-1991
**		Add protos for new MS-Windows-only routines
**
**	V02-032		DAM000		Dennis McEvoy		17-May-1991
**		remove extraneous typedefs from enums
**
**	V02-033		SJM000		Stephen Munyan		17-Jun-1991
**		Get rid of extraneous status code defintions for DEC C/Alpha
**		cleanup.
**
**	V02-034		SJM000		Stephen Munyan		18-Jun-1991
**		Fixed PROTO for dvr_draw_lines to match the routine
**		definition in DVR_GRAPHICS.  This involved changing long
**		references to int references.
**
**	V02-035		SJM000		Stephen Munyan		24-Jun-1991
**		Added proto's to make DEC C work.
**
**	V02-036		DAM000		Dennis McEvoy		22-Jul-1991
**		fix proto for image condition handler on ultrix
**
**	V02-037		DAM000		Dennis McEvoy		05-Aug-1991
**		rename, remove dollar signs
**
**	V02-038		JJT000		Jeff Tancill		22-Aug-1991
**		Change $$ to __ in Cbr rtn PROTOs.
**
**	V02-039		RKN000		Ram Kumar Nori		17-Sep-1991
**		Added new fields to LIN_PRIVATE & ARR_INFO structures.
**
**	V02-040		RKN000		Ram Kumar Nori		18-Oct-1991
**		Added a new field to ARC_PRIVATE structure.
**
**	V02-041		DAM000		Dennis McEvoy		28-oct-1991
**		add DVR_PSVIEW define for vms, ultrix and osf1,
**		not for sun
**
**	V02-042		RKN000		Ram Kumar Nori		29-Oct-1991
**		Changed the type of start_arrow and end_arrow field to PTH in
**		ARR_INFO structure. Changed the number of parmeters passed to
**		dvr_format_polyline_object & dvr_format_arc_object
**
**	V02-043		RKN000		Ram Kumar Nori		2o-Jan-1992
**		Define CDA_HYPER_HELP for VMS
**
**	V02-044		DAM000		Dennis McEvoy		26-mar-1992
**		use portable das bits for sun
**
**	V02-045		ECR000		Elizabeth C. Rust	30-Mar-1992
**		merge in audio code
**
**	V02-046		RAM000		Ralph A. Mack		16-Apr-1992
**		Use HANDLE rather than HCURSOR for now with Windows 3.1
**		includes. May perhaps revisit this in the future.
**
**	V02-047		RKN000		Ram Kumar Nori		02-Jun-1992
**		Added prototype for dvr_reset_gc routine.
**
**	V02-048		DAM000		Dennis McEvoy		02-Jun-1992
**		type cleanups for alpha/osf1
**
**	V02-049		KLM001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**
**	V02-050		DAM000		Dennis McEvoy		12-Jun-1992
**		turn off hyper help on osf/1 for now 
**
**	V02-051		KLM001		Kevin McBride		12-Jun-1992
**		fix up compiler complaints
**
**	V02-052		DAM001		Dennis McEvoy		17-Jun-1992
**		change img_presentation to be a long
**
**	V02-053		DAM001		Dennis McEvoy		22-Jun-1992
**		make cbr param a CDAuserparam to match dvs
**
**	V02-054		KLM001		Kevin McBride		22-Jun-1992
**		change a proto to cleanup DOS warnings
**
**	V02-055         RDH003          Don Haney               15-Jul-1992
**              Change uid filenames to ddif_wgt.uid and use sizeof to compute
**              lengths of uid filenames.
**
**	V02-056		RKN000		Ram Kumar Nori		23-Jul-1992
**		Modified the CALCUALATE_MAX_BBOX macro not to use PARENT_X_
**		OFFSET and PARENT_Y_OFFSET macros. 
**
**	V02-057		DAM001		Dennis McEvoy		04-Aug-1992
**		add cancel button for Windows
**
**	V02-058		DAM001		Dennis McEvoy		18-Aug-1992
**		add filename, opfilename, format strings to dvr struct for
**		Windows to allow DvrViewerPrint to restart convert
**
**	V02-059		DAM001		Dennis McEvoy		02-Sep-1992
**		add origin point to windows dvr structure to account
**		for printing offset
**
**	V02-060		DAM001		Dennis McEvoy		29-Sep-1992
**		add image routine addresses for windows
**
**	V02-061		DAM001		Dennis McEvoy		30-Sep-1992
**		add pallette handle to img_private on windows
**
**	V02-062		PBD000		Peter Derr		 7-Oct-1992
**		#undef DVRFailure and MAKE_ITEM before defining them to avoid
**		DECC compiler warnings.
**	V02-063		JJT0000		Jeff Tancill		13-Oct-1992
**		Replace use of #elif to accomodate some older compilers,
**		use CDAlocal_DAS symbol to determine which DAS to use.
**
**	V02-064		RJD0000		Ronan Duke		 6-Sep-1993
**		Conditionalize name of viewer widget depending on version of Motif 
**		being linked against.
**
**--
**/


/*
**
**  Include files
**
*/

#include <cdaident.h>
static char *library_version = CDA_VERSION;

/* define DVR_PSVIEW for systems where client-side xdps is
 * available; currently, that includes vms, ultrix, osf/1 (not sun)
 */

#ifdef __vms__
#define DVR_PSVIEW
#endif

#ifdef __unix__

#define DVR_PSVIEW

#ifdef sun
#undef DVR_PSVIEW
#endif

#ifdef NO_PS_YET
/* temporarily turn off ps viewing on alpha/osf1 til we get the dps libs */
#undef DVR_PSVIEW
#endif

#include <Xm/ManagerP.h> /* to define XmManagerPart */
#endif	/* __unix__ */





#if defined(OS2) || defined(MSWINDOWS)
#define MAX_FONT_ID		254		/* presentation manager allows */
#endif                                          /* 254 different fonts */

#define FNT_TABLE_SIZE      50			/* size of font table  */
#define DSP_ARRAY_SIZE      200                 /* display array element */
#define TABLE_INCREMENT     100
/* literals for callbacks */

#define	DVR_VERT_UP_UNIT	0
#define	DVR_VERT_DN_UNIT	1
#define	DVR_VERT_UP_PAGE	2
#define	DVR_VERT_DN_PAGE	3
#define	DVR_VERT_TO_TOP		4
#define	DVR_VERT_TO_BOTTOM	5
#define	DVR_VERT_TO_POSIT	6
#define	DVR_VERT_DRAG		7

#define	DVR_HORIZ_UP_UNIT	8
#define	DVR_HORIZ_DN_UNIT	9
#define	DVR_HORIZ_UP_PAGE	10
#define	DVR_HORIZ_DN_PAGE	11
#define	DVR_HORIZ_TO_TOP	12
#define	DVR_HORIZ_TO_BOTTOM	13
#define	DVR_HORIZ_TO_POSIT	14
#define	DVR_HORIZ_DRAG		15

#define	DVR_EXPOSE		16

#define DVR_CDA_START		17
#define DVR_CDA_CLOSE		18

#define DVR_VERT_PERCENT	19
#define DVR_HORIZ_PERCENT	20

/*
 *  special font index literals
 */

#define DEFAULT_FONT_INDEX	-1
#define UNDEFINED_FONT_INDEX	-2


/*
 * Error message stuff
 */
#ifdef __vms__
#include <stsdef>
#define WARNING_STATE	STS$K_WARNING
#define SUCCESS_STATE	STS$K_SUCCESS
#define ERROR_STATE	STS$K_ERROR
#define INFO_STATE	STS$K_INFO
#define SEVERE_STATE	STS$K_SEVERE
#define M_SEVERITY	STS$M_SEVERITY

#else

#ifdef sun
#include <idstyp.h>
#endif

#define WARNING_STATE	0
#define SUCCESS_STATE	1
#define ERROR_STATE   	2
#define INFO_STATE	3
#define SEVERE_STATE	4
#define M_SEVERITY	0x00000007

#endif


/*
 * Activate HyperHelp only on specific platforms.
 */

#if defined(__vms__) 
#define CDA_HYPER_HELP	1
#endif


#define DVR_MIN_WORK_WIDTH 	20
#define DVR_MIN_WORK_HEIGHT 	20

/*
 * Low bit of status code
 */

#ifdef __vms__
#ifdef DVRSuccess
#undef DVRSuccess
#endif
#define DVRSuccess(status)	((status & STS$M_SUCCESS)!=0)
#ifdef DVRFailure
#undef DVRFailure
#endif
#define DVRFailure(status)	((status & STS$M_SUCCESS)==0)
/* Alternate failure macro below */
#define DVR_FAILURE(status)	((status & STS$M_SUCCESS)==0)
#define DVR_FATAL_ERROR(status)	(status & STS$K_SEVERE)

#else
#define DVRSuccess(status)	(((status) & 1) != 0)
#define DVRFailure(status)	(((status) & 1) == 0)
/* Alternate failure macro below */
#define DVR_FAILURE(status)	(((status) & 1) == 0)
#define DVR_FATAL_ERROR(status)	(status & 0x100)

#endif

/* literals for scroll bars */
#if defined(OS2) || defined(MSWINDOWS)
#define DVR_MAX_SCROLL_VALUE     100
#else
#define DVR_MAX_SCROLL_VALUE	 1000000
#endif

#define DVR_INC_SIZE		 10

/* Additional V2 constants */
#define ERROR_LOGFILE VIEWER_ERRORS.LOG

#define DEFAULT_DPI 75
#define CENTIPOINTS_PER_INCH 7200
#define POSITIVE_UNDEFINED 0x7FFFFFFF	/* Infinity? - Decimal:  2147483647 */
#define NEGATIVE_UNDEFINED 0x80000000	/* Infinity? - Decimal: -2147483648 */

/* Per object flags longword values (bit mask) */
#define NO_DISPLAY_FLAG 1   /* Object 'no display' flag; suppress displayable
			       portion of object if set */

/* ... Add additional per object bit flags as necessary ... */


/* UID FILE NAMES */

#ifdef __vms__

/*   rd: viewer widget is named DDIF$VIEWWGT12 if we're linking against V1.2 of
    Motif	*/
#ifdef MOTIF_V12

#define DEFAULT_DVR_UID_FULL_NAME	"DDIF$VIEWWGT12"
#define DEFAULT_DVR_UID_FULL_LEN	15 /* add one for \0 */

#else

#define DEFAULT_DVR_UID_FULL_NAME	"DDIF$VIEWWGT"
#define DEFAULT_DVR_UID_FULL_LEN	13 /* add one for \0 */

#endif        

#endif

#ifdef __unix__

#define DEFAULT_DVR_UID_FULL_NAME	"ddif_wgt_1_2.uid"
#define DEFAULT_DVR_UID_FULL_LEN	(sizeof DEFAULT_DVR_UID_FULL_NAME)

#define DEFAULT_DVR_UID_NAME		"/ddif_wgt_1_2.uid"
#define DEFAULT_DVR_UID_LEN		(sizeof DEFAULT_DVR_UID_NAME)

#endif


/*  helpful macros for accessing ddif viewer
 *  widget fields; (formerly in dvr_create.c)
 */
#if defined(OS2) || defined(MSWINDOWS)
/* OS/2 specific macros (defined differently on vms, ultrix) */

#define Work(w)     ((w)->dvr_viewer.work_window_hwnd)
#define Vscr(w)     ((w)->dvr_viewer.v_scroll_hwnd)
#define Hscr(w)     ((w)->dvr_viewer.h_scroll_hwnd)
#define Bbox(w)	    ((w)->dvr_viewer.bbox_hwnd)
#define NextBut(w)  ((w)->dvr_viewer.next_pg_hwnd)
#define PrevBut(w)  ((w)->dvr_viewer.prev_pg_hwnd)
#define TopBut(w)   ((w)->dvr_viewer.top_doc_hwnd)
#define BotBut(w)   ((w)->dvr_viewer.bot_doc_hwnd)
#define GotoBut(w)  ((w)->dvr_viewer.goto_pg_hwnd)
#define PageLabel(w)  ((w)->dvr_viewer.pg_of_label_hwnd)
#define GotoDialog(w) ((w)->dvr_viewer.goto_dialog_hwnd)
#define Message(w)    ((w)->dvr_viewer.message_box_hwnd)
#define CancelBut(w)  ((w)->dvr_viewer.cancel_view_hwnd)

#define DvrHps(w)   ((w)->dvr_viewer.output_hps)
#define DvrHab(w)   ((w)->dvr_viewer.output_hab)

/* cbr stuff */

#define PrevRef(w)   ((w)->dvr_viewer.Cbr.PrevRefhwnd)
#define NextRef(w)   ((w)->dvr_viewer.Cbr.NextRefhwnd)

/* end cbr stuff */

#else /* X/DECwindows (VMS, ultrix) specific macros */

#define Work(w)     ((w)->dvr_viewer.work_window)
#define Vscr(w)     ((w)->dvr_viewer.v_scroll)
#define Hscr(w)     ((w)->dvr_viewer.h_scroll)
#define GCid(w)     ((w)->dvr_viewer.Dvr.GcId)
#define Win(w)      (XtWindow((w)->dvr_viewer.work_window))
#define Bbox(w)	    ((w)->dvr_viewer.button_box)
#define NextBut(w)  ((w)->dvr_viewer.next_pg_but)
#define PrevBut(w)  ((w)->dvr_viewer.prev_pg_but)
#define TopBut(w)   ((w)->dvr_viewer.top_doc_but)
#define BotBut(w)   ((w)->dvr_viewer.bot_doc_but)
#define GotoBut(w)  ((w)->dvr_viewer.goto_pg_but)
#define PageLabel(w)  ((w)->dvr_viewer.pg_of_label)
#define GotoDialog(w) ((w)->dvr_viewer.goto_dialog_box)
#define GotoText(w)   ((w)->dvr_viewer.goto_text_box)
#define GotoLabel(w)  ((w)->dvr_viewer.goto_text_label)
#define GotoOK(w)     ((w)->dvr_viewer.goto_ok_but)
#define GotoCancel(w) ((w)->dvr_viewer.goto_cancel_but)
#define Message(w)    ((w)->dvr_viewer.message_box)
#define PSwin(w)      ((w)->dvr_viewer.ps_window)
#define PSscroll(w)   ((w)->dvr_viewer.ps_scroll_window)
#define PSvbar(w)     ((w)->dvr_viewer.ps_scroll_h)
#define PShbar(w)     ((w)->dvr_viewer.ps_scroll_v)
#define CancelBut(w)  ((w)->dvr_viewer.cancel_but)
#define ScaleLbl(w)   ((w)->dvr_viewer.scale_lbl)

/*AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
#define AudioTog(w)   ((w)->dvr_viewer.audiotog)
#define AudioState(w)   ((w)->dvr_viewer.audio_state)
#endif
/*END AUDIO STUFF*/

/* cbr stuff */

#define PrevRef(w)   ((w)->dvr_viewer.Cbr.PrevRefBut)
#define NextRef(w)   ((w)->dvr_viewer.Cbr.NextRefBut)

/* end cbr stuff */


#endif

#if defined(OS2) || defined(MSWINDOWS)
#define WORK_WIDTH(w) 	( dvr_work_width(w) )
#define WORK_HEIGHT(w) 	( dvr_work_height(w) )
#else
#define WORK_WIDTH(w)  	(XtWidth(w->dvr_viewer.work_window))
#define WORK_HEIGHT(w) 	(XtHeight(w->dvr_viewer.work_window))
#endif


/*
 * Enumerated type definitions
 */

enum phase_type
    {
    format_phase,
    display_phase
    } ;

enum str_type
    {
    pag_private,		/* Per page 	  private structure */
    obj_private,		/* Per object 	  private structure */
    frm_private,		/* Frame 	  private structure */
    trn_private,		/* Transform 	  private structure */
    gly_private,		/* Galley 	  private structure */
    txt_private,		/* Text line	  private structure */
    frg_private,		/* Text fragment  private structure */
    lin_private,	      	/* Polyline	  private structure */
    crv_private,		/* Bezier curve	  private structure */
    arc_private,		/* Arc		  private structure */
    fil_private,		/* Fill		  private structure */
/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    aud_private,		/* Audio	  private structure */
#endif
/*END AUDIO STUFF*/
    img_private,		/* Image	  private structure */
    img_list_struct		/* Image list structure		    */
    } ;

/*
 * Macros
 */

/* macro for setting up item list for
 * calls to cda_convert
 */
#ifdef MAKE_ITEM
#undef MAKE_ITEM
#endif
#define MAKE_ITEM(item, i, code, length, address) \
{item[i].item_code    = (unsigned short int) code;			  \
 item[i].item_length  = (unsigned short int) length;			  \
 item[i].item_address = (void *) address;}

#ifndef MAX	/* Define MAX if not already defined */
#if defined(OS2) || defined(MSWINDOWS)      /* macro does not work on os/2 */
#define MAX(MaxA, MaxB)	   (dvr_max((MaxA), (MaxB)))
#else
#define MAX(MaxA, MaxB)    ((MaxA) >  (MaxB) ? (MaxA) : (MaxB))
#endif
#endif

#ifndef MIN	/* Define MIN if not already defined */
#if defined(OS2) || defined(MSWINDOWS)
#define MIN(MinA, MinB)	   (dvr_min((MinA), (MinB)))
#else
#define MIN(MinA, MinB)    ((MinA) < (MinB) ? (MinA) : (MinB))
#endif
#endif

/* macro to change cursor to watch */
#ifdef OS2
/* macros defined differently on VMS,ultrix */

#define StartWait(widget)\
        {\
            if ((widget)->dvr_viewer.wait_pointer)\
                WinSetPointer(HWND_DESKTOP, (widget)->dvr_viewer.wait_pointer);\
        }
#define StopWait(widget)\
        {\
            if ((widget)->dvr_viewer.default_pointer)\
                WinSetPointer(HWND_DESKTOP, (widget)->dvr_viewer.default_pointer);\
        }
#else
#if defined(MSWINDOWS)
#define StartWait(widget)\
	{\
	    if ((widget)->dvr_viewer.wait_pointer)\
		SetCursor ((widget)->dvr_viewer.wait_pointer);\
	}
#define StopWait(widget)\
	{\
	    if ((widget)->dvr_viewer.default_pointer)\
		SetCursor ((widget)->dvr_viewer.default_pointer);\
	}
#else /* VMS,ultrix definitions of same macros */


#define StartWait(widget)\
	{\
	if ( XtIsRealized (widget) )\
	    {\
	    XDefineCursor ( XtDisplay(widget), XtWindow(widget), \
            pDvrWindow->ViewerCursor );\
	    XFlush(XtDisplay(widget));\
	    }\
	}

/* macro to switch cursor back to default */

#define StopWait(widget)\
	{\
	if ( XtIsRealized (widget) )\
	    {\
	    XUndefineCursor ( XtDisplay(widget), XtWindow(widget) );\
	    }\
	}

#endif
#endif

/*****************/
/* New V2 macros */
/*****************/

/* CONVERT_CP_TO_PIXEL: Given a centipoint value, returns the equivalent
   pixel value using the device pixel DPI (dots per inch) value, treated
   as a linear measurement; parameter and return values are integers;
   centipoint values are truncated; the alternate routine, CONVERT_CP_TO_
   PIXEL_ROUNDED rounds to the nearest integer value, rather than truncate
   (currently used only for improved rendition attribute support and display) */
/* [Assumption: 'vw' is defined and available as a viewer widget pointer] */
/* dpi should be the dpi value x_dpi for horizontal; y_dpi for vertical */

#define CONVERT_CP_TO_PIXEL( centipoints, dpi ) \
    ((CDAmeasurement) (( (centipoints) * dpi ) / CENTIPOINTS_PER_INCH ))

#define CONVERT_CP_TO_PIXEL_ROUNDED( centipoints, dpi ) \
    ((CDAmeasurement) (((float) ( (centipoints) * dpi ) / \
	(float) CENTIPOINTS_PER_INCH ) + 0.50 ))


/* CVT_CP_TO_PIXEL_TRUNC_WITH_REM: Given a centipoint and a return remainder
   value param, returns the equivalent pixel value using the device DPI,
   similar to the CONVERT_CP_TO_PIXEL macro; however, this routine does
   not round the conversion; rather it truncates the calculated value,
   and returns the associated centipoint remainder value in param 2 */
/* [Assumption: 'vw' is defined and available as a viewer widget pointer] */
/* dpi should be the dpi value x_dpi for horizontal; y_dpi for vertical */


#define CVT_CP_TO_PIXEL_TRUNC_WITH_REM( centipoints, remainder, dpi ) \
    ((CDAmeasurement) \
    /* Calculate conversion remainder */ \
    ( remainder = ((( (centipoints) * dpi ) % \
	CENTIPOINTS_PER_INCH ) / dpi ), \
    /* Integer divide (truncate) and return pixel equivalent */ \
    (( (centipoints) * dpi ) / CENTIPOINTS_PER_INCH )))


/* CONVERT_PIXEL_TO_CP: Given a pixel value, returns the equivalent
   centipoint value using the device pixel DPI (dots per inch) value, treated
   as a linear measurement; parameter is an integer value */
/* [Assumption: 'vw' is defined and available as a viewer widget pointer] */
/* dpi should be the dpi value x_dpi for horizontal; y_dpi for vertical */

#define CONVERT_PIXEL_TO_CP( pixels, dpi ) \
    ((CDAmeasurement) (CENTIPOINTS_PER_INCH * ((float)(pixels) / dpi ) ))



/* COPY_INIT_STATE_INFO: Copies state information required for child objects
   from the parent to child state structure, clearing the values to be
   returned from the children, thereby initializing the child state; child
   parent reference is also set to point to the specified parent status; input
   parameters state and new_state must both be accessible as type STATEREF
   (that is, pointers to type STATE); no return value */

#define COPY_INIT_STATE_INFO( state, child_state ) \
    /* Point child to parent state structure */ \
    (child_state)->parent_state = state; \
    /* Copy params */ \
    (child_state)->x_offset  = state->x_offset; \
    (child_state)->y_offset  = state->y_offset; \
    (child_state)->x_relative = state->x_relative; \
    (child_state)->y_relative = state->y_relative; \
    (child_state)->x_scale = state->x_scale; \
    (child_state)->y_scale = state->y_scale; \
    /* Init params */ \
    (child_state)->bbox_ll_x = POSITIVE_UNDEFINED; \
    (child_state)->bbox_ll_y = POSITIVE_UNDEFINED; \
    (child_state)->bbox_ur_x = NEGATIVE_UNDEFINED; \
    (child_state)->bbox_ur_y = NEGATIVE_UNDEFINED;


/* BBOX_DEFINED: Returns true if bounding box has been defined.  Parameter
   should be a pointer to a state structure (type STATEREF); Note: Routine
   will also work with a pointer to a page private structure (type
   PAG_PRIVATE) */

#define BBOX_DEFINED( state ) \
    ((((state)->bbox_ll_x == POSITIVE_UNDEFINED) && \
    ((state)->bbox_ll_y == POSITIVE_UNDEFINED) && \
    ((state)->bbox_ur_x == NEGATIVE_UNDEFINED) && \
    ((state)->bbox_ur_y == NEGATIVE_UNDEFINED)) ? FALSE : TRUE )


/* OBJECT_BBOX_DEFINED: Returns true if bounding box has been defined.
   Parameter should be a pointer to a object private structure (type
   OBJ_PRIVATE); Note: Routine will NOT work with PAG_PRIVATE structure */

#define OBJECT_BBOX_DEFINED(object_private) \
    (((object_private->bbox_ul_x == POSITIVE_UNDEFINED) && \
      (object_private->bbox_ul_y == POSITIVE_UNDEFINED) && \
      (object_private->bbox_lr_x == NEGATIVE_UNDEFINED) && \
      (object_private->bbox_lr_y == NEGATIVE_UNDEFINED)) ? FALSE : TRUE )


/* CALCULATE_MAX_BBOX: Maximize the size of the bounding box for lower left
 * and upper right X,Y coordinates.  Routines expects the parent bounding
 * box reference followed by the child bounding box; the parent information
 * is compared to the child's,and the maximun and minimum bounding box values 
 * are replaced in the parent state structure. Parameters are expected to be 
 * pointers to the state information structures (type STATEREF) corresponding 
 * to the parent and child levels, respectively 
 */

/* Local macros (intended for use by CALCULATE_MAX_BBOX only) which is not
 * used anymore
 */
#define PARENT_X_OFFSET( state ) \
    (((state)->parent_state == NULL ) ? (state)->x_offset : \
      ((state)->x_offset - (state)->parent_state->x_offset))
#define PARENT_Y_OFFSET( state ) \
    (((state)->parent_state == NULL ) ? (state)->y_offset : \
      ((state)->y_offset - (state)->parent_state->y_offset))

/* 'External' macro */
#if defined(OS2) || defined(MSWINDOWS)

/* macro is too long for OS2 */
#define CALCULATE_MAX_BBOX( state, child_state ) \
    ( dvr_calculate_max_bbox(state, child_state) )

#else
#define CALCULATE_MAX_BBOX( state, child_state ) \
    if ( BBOX_DEFINED (child_state) ) { \
	(state)->bbox_ll_x = \
	    MIN ((state)->bbox_ll_x , \
		((child_state)->bbox_ll_x)); \
	(state)->bbox_ll_y = \
	    MIN ((state)->bbox_ll_y , \
		((child_state)->bbox_ll_y)); \
	(state)->bbox_ur_x = \
	    MAX ((state)->bbox_ur_x , \
		((child_state)->bbox_ur_x )); \
	(state)->bbox_ur_y = \
	    MAX ((state)->bbox_ur_y , \
		((child_state)->bbox_ur_y)); \
    }

#endif


/* OBJECT_BBOX_INCLUDED: Returns true if bounding box in object private
   structure falls within the display bounding box specified as the current
   page display region.  Param 1 should be object private pointer, type
   OBJ_PRIVATE; param 2 should be display bbox pointer, type BBOX_REF,
   which should ALREADY be X,Y coordinate translated based on the window
   X,Y top values for proper 'page' coordinate object bbox comparison */

/* Algorithm: All possible cases have been reduced to seven tests, any of
   which are true indicate bbox inclusion;  first, test for the upper left
   object bbox to be within the display bbox, second, test for the lower
   right object bbox to be within this display region, third, check for
   the object bbox to include the display bbox as a subset of its own
   display area, that is, where the display bbox does not intersect the object,
   but where the display bbox is completely within the display area of the
   object, fourth, check if the object X coordinate values fall out of the
   display bbox, but where the Y coordinates fall within these coordinates,
   and fifth, use test four again switching the X and Y coordinate tests;
   sixth, check if the upper right corner of the object bbox is within the
   display bbox, and seventh, check if the lower left corner is within;
   as soon as one of the individual tests fail, C jumps to the next test,
   without necessarily completing the test (that is, if any of the ANDed
   values are false the whole expression is false); also, when one of the
   tests is successful, C exits with success, since testing the other ORed
   conditions would not produce a TRUEer value */

/* Old version:
#define OBJECT_BBOX_INCLUDED( object_private, display_bbox )		\
  ( ((( display_bbox->bbox_ul_x <= object_private->bbox_ul_x ) &&	\
        ( object_private->bbox_ul_x <= display_bbox->bbox_lr_x )) &&	\
        (( display_bbox->bbox_ul_y <= object_private->bbox_ul_y ) &&	\
	( object_private->bbox_ul_y <= display_bbox->bbox_lr_y ))) ||	\
    ((( display_bbox->bbox_ul_x <= object_private->bbox_lr_x ) &&	\
        ( object_private->bbox_lr_x <= display_bbox->bbox_lr_x )) &&	\
	(( display_bbox->bbox_ul_y <= object_private->bbox_lr_y ) &&	\
	( object_private->bbox_lr_y <= display_bbox->bbox_lr_y ))) ||	\
    ((( object_private->bbox_ul_x <= display_bbox->bbox_ul_x ) &&	\
        ( object_private->bbox_ul_y <= display_bbox->bbox_ul_y )) &&	\
	(( object_private->bbox_lr_x >= display_bbox->bbox_lr_x ) &&	\
	( object_private->bbox_lr_y >= display_bbox->bbox_lr_y ))) ||	\
    ((( object_private->bbox_ul_x <= display_bbox->bbox_ul_x ) &&	\
        ( object_private->bbox_lr_x >= display_bbox->bbox_lr_x )) &&	\
	(( object_private->bbox_ul_y >=  display_bbox->bbox_ul_y ) &&	\
	( object_private->bbox_lr_y <= display_bbox->bbox_lr_y ))) ||	\
    ((( object_private->bbox_ul_y <= display_bbox->bbox_ul_y ) &&	\
        ( object_private->bbox_lr_y >= display_bbox->bbox_lr_y )) &&	\
	(( object_private->bbox_ul_x >= display_bbox->bbox_ul_x ) &&	\
	( object_private->bbox_lr_x <= display_bbox->bbox_lr_x ))) ||	\
    ( ( object_private->bbox_ul_x <= display_bbox->bbox_ul_x ) &&	\
	(( display_bbox->bbox_ul_y <= object_private->bbox_ul_y ) &&	\
	( object_private->bbox_ul_y <= display_bbox->bbox_lr_y ))) ||	\
    ( ( object_private->bbox_lr_x >= display_bbox->bbox_lr_x ) &&	\
	(( display_bbox->bbox_ul_y <= object_private->bbox_lr_y ) &&	\
	( object_private->bbox_lr_y <= display_bbox->bbox_lr_y ))) )
*/

#if defined(OS2) || defined(MSWINDOWS)

/* macro is too big for OS2 */
#define OBJECT_BBOX_INCLUDED( o, d ) \
    ( dvr_object_bbox_included(o,d) )

#else

#define OBJECT_BBOX_INCLUDED( o, d ) \
    ((( d->bbox_ul_x <= o->bbox_ul_x ) && ( d->bbox_ul_y <= o->bbox_ul_y ) && \
      ( o->bbox_ul_x <= d->bbox_lr_x ) && ( o->bbox_ul_y <= d->bbox_lr_y )) || \
    (( d->bbox_ul_x <= o->bbox_lr_x ) && ( d->bbox_ul_y <= o->bbox_lr_y ) && \
      ( o->bbox_lr_x <= d->bbox_lr_x ) && ( o->bbox_lr_y <= d->bbox_lr_y )) || \
    (( d->bbox_ul_x <= o->bbox_ul_x ) && ( o->bbox_ul_x <= d->bbox_lr_x ) && \
      ( o->bbox_ul_y <= d->bbox_ul_y ) && ( d->bbox_ul_y <= o->bbox_lr_y )) || \
    (( d->bbox_ul_y <= o->bbox_ul_y ) && ( o->bbox_ul_y <= d->bbox_lr_y ) && \
      ( o->bbox_ul_x <= d->bbox_ul_x ) && ( d->bbox_ul_x <= o->bbox_lr_x )) || \
    (( d->bbox_ul_x <= o->bbox_lr_x ) && ( o->bbox_lr_x <= d->bbox_lr_x ) && \
      ( o->bbox_ul_y <= d->bbox_lr_y ) && ( d->bbox_lr_y <= o->bbox_lr_y )) || \
    (( d->bbox_ul_y <= o->bbox_lr_y ) && ( o->bbox_lr_y <= d->bbox_lr_y ) && \
      ( o->bbox_ul_x <= d->bbox_lr_x ) && ( d->bbox_lr_x <= o->bbox_lr_x )) || \
    (( o->bbox_ul_x <= d->bbox_ul_x ) && ( o->bbox_ul_y <= d->bbox_ul_y ) && \
      ( d->bbox_lr_x <= o->bbox_lr_x ) && ( d->bbox_lr_y <= o->bbox_lr_y )))

#endif

/* QUE_EMPTY: Returns true if queue is empty; queue parameter is treated as
   an object of type struct que */

#define QUE_EMPTY(queue) \
    (( (queue).que_flink == &(queue) ) && \
     ( (queue).que_blink == &(queue) ))



/*
 *  Default memory allocation and deallocation macros.
 */

/* Size and context passed by value; address for allocation should be the
   address of a pointer value (pointer by reference), address for deallocation
   should be the actual pointer value (pointer by value) */

#define DVR_ALLOCATE_MEMORY(size, address) \
    (DVRSuccess(cda_malloc( (CDAsize	*) &(size), (CDAaddress *) address, 0L)) ? DVR_NORMAL : DVR_MEMALLOFAIL)

#define DVR_DEALLOCATE_MEMORY(size, address) \
    (DVRSuccess(cda_free( (CDAsize	*) &(size), (CDAaddress *) address, 0L)) ? DVR_NORMAL : DVR_MEMDEALLOFAIL)



#if defined(OS2) || defined(MSWINDOWS)  /* PS viewing not available on OS/2 */

#define VIEWING_PS(vw) 0

#else

#define VIEWING_PS(vw) \
( vw->dvr_viewer.Ps.dps_exists && (PSscroll(vw) != 0) && (XtIsManaged(PSscroll(vw))) )

#endif

#ifdef OS2

/* strings for buttons and labels within widget */
#define dvr_goto_pg_str		"Page..."
#define dvr_page_label_str	"Page"
#define dvr_of_label_str	"of"
#define dvr_goto_page_str  	"Go To Page"
#define dvr_ok_str		"  OK  "
#define dvr_cancel_str		"Cancel"
#define dvr_invalid_page_str	"Invalid Page Number"

#define dvr_product_name_str	"Product Name : "
#define dvr_title_str		"Title : "
#define dvr_author_str		"Author : "
#define dvr_version_str		"Version : "
#define	dvr_date_str		"Date : "
#define dvr_info_tab_str 	"        "

/* specify a default font */
#define DEFAULT_FONT "-ADOBE-COURIER-MEDIUM-R-NORMAL--*-120-*-*-M-*-ISO8859-1"

#endif

#ifdef MSWINDOWS
#define DEFAULT_FONT "-ADOBE-COURIER-MEDIUM-R-NORMAL--*-120-*-*-M-*-ISO8859-1"
#endif

/* literals for converting page size to centipoints */
#define DVR_DECW_CPNTS_VERT	1200
#define DVR_DECW_CPNTS_HORIZ	720

#define DVR_MM_PER_INCH		25.4
#define DVR_CPNTS_PER_INCH 	7200


/* literals for classifying buttons */
#define DVR_TOP_BUTTONS 0
#define DVR_BOT_BUTTONS 1
#define DVR_ALL_BUTTONS	2



/*
 * FOR DEBUGGING ONLY
 * COMMENT OUT WHEN BUILDING FT OR SDC KITS
 */
/* #define Debug   TRUE */


/*
**++
**  FORMER module name:	dvr_structs.h
**
**
**  ABSTRACT:	    Viewer data structure definitions.
**
**
**  MODIFICATION HISTORY:
**
**  05  06-feb-89   dam   add descriptor aggregate to Dvr struct
**  04  17-jan-89   mac   modify font structure
**  03  16-jan-89   dam   add agregate handles to Dvr struct
**  02  07-Dec-88+  mac   new structures required for V2
**  01  23-nov-88   dam   update for V2; add/remove fields for viewer
**			  engine;
**
**  V1 complete
**
**  05  21-Jun-88   Svh   Changed Xtop and Ytop from unsigned short to int
**  04  06-Jun-88   Svh   Modified font structure
**  03  02-Jun-88   Ll    Added DisplayTable & DisplayBuffer
**  02  12-Mar-88   Lv    Added variables to handle "frames"
**  01  12-Mar-88   Lv    Added variables to handle previous page.
**--
*/

#if defined(OS2) || defined(MSWINDOWS)
#define Boolean BOOL
#endif

/* Note: Layout Engine (DVS) definitions must already be defined; many of
   these following structures rely upon the definitions within this file */

/* State information, including transform and other contextual info */
typedef struct state_str
    {

    struct state_str *parent_state;  /* Pointer to parent state structure,
				        if it exists */

    CDAmeasurement x_offset;	/* Current coordinate system offset from original */
    CDAmeasurement y_offset;	/* X,Y origin, referring to lower left bounding */
			/* box position, with values stored in centipoints */

    CDAmeasurement x_relative;	/* X,Y positions relative to current lower left */
    CDAmeasurement y_relative;	/* position in centipoints; values entered by */
			/* specific object format routines and translated */
			/* to pixel upper left values upon return; default */
			/* relative value is 0,0 (that is, at position */
			/* indicated by parent object) */

    CDAmeasurement bbox_ll_x;	/* Bounding box information; can be used to pass */
    CDAmeasurement bbox_ll_y;	/* back information to a parent calling level with */
    CDAmeasurement bbox_ur_x;	/* the actual largest bounding box information to */
    CDAmeasurement bbox_ur_y;	/* allow parent to modify their sizes to accomodate */
			/* larger display areas, if required; lower left */
			/* and upper right bounding box coordinates are */
			/* retained; user-specified bounding boxes may */
			/* have been incorrect; should probably increase */
			/* parent bounding box size unless a smaller */
			/* clipping region is specifically defined */
    float x_scale;      /* accumlated value of trn_transform[0] */
    float y_scale;      /* accumlated value of trn_transform[4] */

    } STATE, *STATEREF ;

/* Bounding box structure; upper left and lower right X,Y coordinates are
   defined in this structure, used as a parameter in display routine calls */
typedef struct bbox_ul_lr
    {

    CDAmeasurement bbox_ul_x;	/* Specified display bounding box, in pixels */
    CDAmeasurement bbox_ul_y;
    CDAmeasurement bbox_lr_x;
    CDAmeasurement bbox_lr_y;

    } BBOX_UL_LR, *BBOX_REF;


/* Per object structure (obj) private structure (except page object) */
typedef struct obj_private
    {

    struct str private_str;	/* Header */

    CDAmeasurement x_pos;		/* Object starting X position, in pixels */
    CDAmeasurement y_pos;		/* Object starting Y position, in pixels */

    CDAmeasurement bbox_ul_x;	/* Bounding box upper left X, in pixels */
    CDAmeasurement bbox_ul_y;	/* Bounding box upper left Y, in pixels */
    CDAmeasurement bbox_lr_x;	/* Bounding box lower right X, in pixels */
    CDAmeasurement bbox_lr_y;	/* Bounding box lower right Y, in pixels */

    CDAflags flags;	/* Flags longword */

    PTH orig_path;	/* Original relative path, to be transformed, if any;
			   format phase transforms this path into final pixel
			   upper left coordinates; this path is expected to be
			   lower left relative and stored in centipoints */
    PTH real_path;	/* Transformed path in pixels, if necessary (pixels);
			   this information is available to be used by object
			   specific display routines during the display phase;
			   this information is upper left relative and stored
			   in current pixel coordinates */

    PTH clip_path;	/* Clip path transformed in pixels - only used in
			   frames */

    void *private_info;	/* Specific object private info structure, if any */

    } *OBJ_PRIVATE;


/* image list structure - this keeps track of any/all IDS renderings that  */
/* are allocated by the Viewer.  Delete rendering when the Viewer moves    */
/* more than one page away from the page on which the rendering appears	   */

typedef struct img_list_struct
    {

    struct str 	img_str;		/* Header */

#if defined(OS2) || defined(MSWINDOWS)
    HWND	hwnd_id;
#endif

    OBJ_PRIVATE object_data;		/* image object data, including rendering  */

    } *IMG_LIST_STRUCT;


/* color allocations queue */

typedef struct color_info
    {
    struct que	color_info_queue;
    int		color_info_pixel;
    long	color_info_red;
    long	color_info_green;
    long	color_info_blue;
    } *COLOR_INFO;

/* Per page structure (pag) private structure */
typedef struct pag_private
    {

    struct str private_str;	/* Header */

    CDAmeasurement       height;		/* Page paper height, in pixels */
    CDAmeasurement       width;		/* Page paper width, in pixels */

    CDAmeasurement       bbox_ll_x;	/* Aggregate page bounding box; data stored */
    CDAmeasurement       bbox_ll_y;	/* in centipoint frame coordinates; to be */
    CDAmeasurement       bbox_ur_x;	/* used only for prototype page reference */
    CDAmeasurement       bbox_ur_y;	/* optimization for pages that reference it; */
                                /* that is, pages that reference this page */
                                /* need not reprocess this page to calculate */
                                /* bounding box info on the referencing page */

    struct que img_list;	/* Allocated image widgets list; queue of */
				/* img_list_struct elements */

    struct que	color_info_list; /* Color allocations list queue root */
/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    struct aud_private *audio_on_page; /*List of audio buttons associated with the page.*/
				       /*USE que HERE???*/
#endif
/*END AUDIO STUFF*/
    } *PAG_PRIVATE;

/*
 * Specific object private structures; not all objects will required
 * private information structures
 */

/* Frame (FRM) private info */
typedef struct frm_private
    {

    struct str private_str;	/* Header */

    PTH        trn_path;	/* Transformed outline path, in pixels */

    } *FRM_PRIVATE;


/* Transform (TRN) private info */
typedef struct trn_private
    {

    struct str private_str;	/* Header */

    } *TRN_PRIVATE;


/* Galley (GLY) private info */
typedef struct gly_private
    {

    struct str private_str;	/* Header */

    PTH        trn_path;	/* Transformed outline path, in pixels */

    } *GLY_PRIVATE;


/* Text line (TXT) private info */
typedef struct txt_private
    {

    struct str private_str;	/* Header */

    CDAmeasurement cb_x1;			/* changebar first x coord (in pixels) */
    CDAmeasurement cb_y1;			/* changebar first y coord (in pixels) */
    CDAmeasurement cb_x2;			/* changebar second x coord (in pixels) */
    CDAmeasurement cb_y2;			/* changebar second y coord (in pixels) */
    CDAmeasurement cb_wid; 		/* changebar width length (in pixels) */

    } *TXT_PRIVATE;


/* Text fragment (FRG) private info */

typedef struct frg_private
    {
    struct str		private_str;	/* Header */

    char		*string;		/* Substring */
    CDAsize		str_len;		/* String length, in characters */
    CDAmeasurement	x_stretch;		/* Additional space stretch to apply after
						   string, in pixels (may be negative) */
    CDAmeasurement	pixel_len;		/* Display pixel length */

    struct frg_private	*next;			/* Pointer to next element, if any */

    } *FRG_PRIVATE;

typedef struct
    {
    short	x,y;
    } Coord, *COORDS;

typedef struct
    {
    CDAmeasurement	    arrow_width;     /* Unfilled Arrow Width in Pixels*/
    int		    arrow_type;	     /* Type Of the Arrow Heads */
    PTH		    start_arrow;     /* Starting Arrow Points In Pixels */
    PTH		    end_arrow;	     /* End Arrow Points In Pixels*/
    PTH             arrow_path;      /* The path for Arrow heads */
    } Arr_info, *ARR_INFO;

/* Polyline (LIN) private info */

typedef struct lin_private
    {
    struct str	private_str;	  /* Header */
    ARR_INFO    arrow_info;       /* Pointer To arrow Head info Struct */
    PTH		eng_path;         /* Engine defined path tranformed in pixels */
    } *LIN_PRIVATE;


/* Bezier curve (CRV) private info */

typedef struct crv_private
    {

    struct str private_str;	/* Header */

    } *CRV_PRIVATE;


/* Arc (ARC) private info */

typedef struct arc_private
    {

    struct str private_str;	/* Header */

    CDAmeasurement    first_endpoint_x;	/* arc endpoints, transformed to pixel	    */
    CDAmeasurement    first_endpoint_y;	/* coordinates				    */
    CDAmeasurement    second_endpoint_x;
    CDAmeasurement    second_endpoint_y;

    CDAmeasurement    middle_endpoint_x;
    CDAmeasurement    middle_endpoint_y;
    PTH	    rotated_path;	 /* Rotated Arc Path in Pixel Coordinates  */

    ARR_INFO    arrow_info;       /* Pointer To arrow Head info Struct */
    } *ARC_PRIVATE;


/* Fill (FIL) private info */

typedef struct fil_private
    {

    struct str private_str;	/* Header */

    } *FIL_PRIVATE;


/* Image (IMG) private info */

typedef struct img_private
    {
    struct str		private_str;	    /* Header */

#if defined(OS2) || defined(MSWINDOWS)
    boolean	img_widget_created;	    /* TRUE if image widget exists */
    HBITMAP 	img_ddb;		    /* device dependent bitmap */
    HANDLE	img_hpallette;		    /* pallette handle */
#else
    struct IdsRendering *img_rendering;	    /* Image rendering for current display */
#ifdef sun
    IdsRenderContext  img_presentation;
#else
    unsigned long int img_presentation;	    /* Image presentation id for current display */
#endif
#endif
    int img_polarity;
    float img_aspect;

    } *IMG_PRIVATE;


/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
/*
 * Audio private info. 
 */
typedef struct aud_private
    {
    struct str          private_str;	/* Header */
    Widget              audio_button;    
    CDAmeasurement		audio_button_no;/*Zero rel. no of the button on page.*/
    CDAmeasurement		audio_button_x; /*Coords of the button in pixels */
    CDAmeasurement		audio_button_y; /*relative to upper left of page.*/
    CDAmeasurement		audio_object_x; /*Coords of the audio in pixels  */
    CDAmeasurement		audio_object_y; /*relative to upper left of page.*/
    struct aud_private 		*next_audio_on_page;
    CDAaddress			aud_backlink;   /*Pointer back to dvs AUD structure*/
    } *AUD_PRIVATE;
#endif
/*END AUDIO STUFF*/


/*
 *  Font information structure
 */

typedef struct FontStruct
    {
    unsigned long x_font_id;		    /* X font ID */
    unsigned long *x_font_struct;	    /* X font structure */
#if defined(OS2) || defined(MSWINDOWS)
CDAsize	fm_index; 		    /* index into font metrics array */
#endif
    } FontStruct, *FontStructPtr;

/*
 * Structure to maintain Dvr Viewer Context;
 * file specific portion of viewer widget;
 */

typedef struct DvrStruct
    {
    struct que	    page_list; 			/* list of pages [pag's] */
    struct que	    private_structs;            /* list of all private data
						   allocated */
    PTH		    *allocated_pth_structs;	/* array of PTH ptrs to	    */
    CDAsize	    alloc_pth_struct_array_size; /* allocated pth	    */
						 /* structures		    */
    CDAcount	    alloc_pth_struct_count;	/* num of PTH structs	    */
						/* allocated		    */

    PAG             current_page;		/* pointer to current page */
    CDAmeasurement	    working_page_height;	/* Page height of primary page
						   currently/last formatted;
						   value only significant while
						   formatting a page */

    BBOX_UL_LR	    display_bbox;		/* Specified display bounding
						   box region; exposed portion
						   of current page */

    ENG             engine_context;             /* context to pass to viewer
						   engine */

#if defined(OS2) || defined(MSWINDOWS)

#else /* X/DECwindows (vms, ultrix) specific fields for this struct */

    GC		    GcId;                       /* Graphics context for
						   drawing */
    XGCValues	    gcv;			/* gc values struct -
						   corresponds to GcId */
    GC		    erase_gc_id;		/* GC for erasing */
    XFontStruct     *pFont;			/* current font structure */
#endif

    FontStructPtr   FontTable;                  /* font table */
    unsigned int    FontId;                     /* current font ident */

    CDAstatus		WidgetStatus;		/* widget status */
    CDAconverterhandle *ConverterContext;	/* cda converter context */
    CDAmeasurement	    Ytop;                       /* current Y top position
						   within page */
    CDAmeasurement	    Xtop;                       /* current X top position
						   within page */
    unsigned long   VertPercentTotal;		/* the percent of the page
						   that has been read so far
					   	   vertically */
    unsigned long   HorizPercentTotal;		/* the percent of the page
						   that has been read so far
					   	   vertically */
    unsigned long   VertPercentCurrent;		/* the percent of the page
						   displayed in the current
						   window (vertical) */
    unsigned long   HorizPercentCurrent;	/* the percent of the page
						   displayed in the current
						   window (horizontal) */
    unsigned long   VertValue;			/* value of vert. sb slider */
    unsigned long   HorizValue;			/* value of horiz. sb slider */
    CDAsize	    FontTableSize;
    Boolean	    FileOpen;			/* tells if a file is open */
    Boolean         DocRead;                    /* Document comp read in */
    int		    *error_log;			/* Error log file (FILE *) */
    CDAagghandle dhd_agg_handle;	/* document header agg handle */
    CDArootagghandle root_agg_handle;	/* doc's root agg handle */
    CDAagghandle dsc_agg_handle;	/* document descriptor agg handle */
    } DvrStruct, *DvrStructPtr;


typedef struct PsStruct {
    Boolean	dps_exists;
    char	*ps_str;
    CDAcount	current_page;
    CDAcount	last_page;
    Boolean	waiting;
    Boolean	use_comments;
    Boolean	use_bitmaps;
    Boolean	use_trays;
    Boolean	watch_progress;
    int		scale_value;
    int		orientation;
    int		window_width;
    int		window_height;
    Boolean	header_required;
    } PsStruct, *PsStructPtr;
/* cbr stuff */

#if defined(OS2) || defined(MSWINDOWS)
typedef struct CbrStruct {
    Boolean	  mode;
    PROTO(CDAstatus (*FrgRefCb),  (CDAuserparam,
	  CDAconstant, CDAint32, CDAuint32 *, CDAuint32 **));	
    CDAuserparam  FrgRefParm;
    HWND          NextRefhwnd;
    HWND          PrevRefhwnd;
    Boolean	  EnableRefButs;
    } CbrStruct, *CbrStructPtr;
#else
typedef struct CbrStruct {
    Boolean	  mode;
    PROTO(CDAstatus (*FrgRefCb),  (CDAuserparam,
	  CDAconstant, CDAint32, CDAuint32 *, CDAuint32 **));	
    CDAuserparam  FrgRefParm;
    Widget	  NextRefBut;
    Widget	  PrevRefBut;
    Boolean	  EnableRefButs;
    } CbrStruct, *CbrStructPtr;
#endif

/* end cbr stuff */

/*
 * item list structure for calls to cda_convert
 */

typedef struct DvrCdaItemList
    {
    unsigned short int item_length;		/* length of item */
    unsigned short int item_code;		/* cda code for item */
    		   void *item_address;		/* address of item */
    } DvrCdaItemList, *pDvrCdaItemList;



/*
**++
**  FORMER MODULE NAME:		dvr_create.h
**
**
**  ABSTRACT:
**
**	type definitions for the ddif viewer widget
**
**
**  MODIFICATION HISTORY:
**
**   09  dam  13-feb-1989   add paper width and height
**   08	 bfb   9-feb-1989   add cursor reference count to widget
**   07  mac  17-jan-1989   add default and current font IDs
**   06  dam  16-jan-1989   add save goto text
**   05  dam  13-jan-1989   add pixmaps to widget
**   04  mac  13-Jan-1989   add font struct table to widget
**   03  dam  11-Jan-1989   add dpi and default font to widget
**   02	 dam  05-Jan-1989   add new children to viewer: button box with
**			    buttons and label children
**   01  dam  23-nov-1988   update for V2; add/remove fields to use
**			    viewer engine;
**
**   V1 complete
**
**   06  dam  31-aug-1988   become a subclass of common
**   05  dam  11-jul-1988   added ViewerCursor to structure
**   04  Svh  06-Jun-1988   Changed pFontTable to FontTable
**   03  Ll   02-Jun-1988   Added DisplayBuffer & DisplayTable
**   02  lv   11-Mar-1988   Remove the QEntry structure definitions
**   01  dam  03-Mar-1988   Removed unneccessary fields, defines
**
**--
**/

/**************************************************************************
 *
 * ddif viewer widget data and class definitions;
 *
 * all this info will normally stay the same for the life
 * of the widget; the fields within the Dvr
 * structure which will change for each file loaded;
 *
 **************************************************************************/
#ifdef MSWINDOWS	/* Translate OS/2 PM terminology into MS-Windows terms*/
#define HMODULE 	HANDLE	/* Library handle */
#define HPOINTER 	HANDLE  /* Cursor handle */
#define HAB 		HANDLE	/* Instance handle */
#define HPS 		HDC
#define FONTMETRICS 	TEXTMETRIC
#endif

typedef	struct
    {
#if defined(OS2) || defined(MSWINDOWS) /* OS/2 specfic fields for this struct */
    HWND                    viewer_hwnd;
    HWND		    work_window_hwnd;
    HWND		    h_scroll_hwnd;
    HWND		    v_scroll_hwnd;
    HWND		    bbox_hwnd;
    HWND		    next_pg_hwnd;
    HWND		    prev_pg_hwnd;
    HWND		    top_doc_hwnd;
    HWND		    bot_doc_hwnd;
    HWND		    goto_pg_hwnd;
    HWND		    pg_of_label_hwnd;
    HWND		    goto_dialog_hwnd;
    HWND		    message_box_hwnd;
    HWND		    cancel_view_hwnd;	
    HPS			    output_hps;
    HAB			    output_hab;
    unsigned short          font_count;
    FONTMETRICS		    *fm_array;
    HPOINTER                wait_pointer;
    HPOINTER                default_pointer;
    PROTO( void (CDA_CALLBACK *callback_rtn), (HWND, DvrCallbackStruct *));
    PROTO( void (CDA_CALLBACK *help_rtn),     (HWND, DvrCallbackStruct *));

    HMODULE 		    hResourceFile;  /* handle to Resource File */

    LONG		    current_pattern_id;
    HBITMAP		    pattern_array[65];

    /*  the following booleans are used with the routines DvrLoadFile()
     *  and DvrDisplayFirstPage() on OS/2; These routines can be used together
     *  instead of a call to DvrViewerFile(). This allows applications to
     *  execute the DvrLoadFile() call in a second thread of execution;
     *  DvrloadFile() gets the first page from the engine which can be a
     *  lengthy process on OS/2; Applications cannot call DvrViewerFile()
     *  in a second thread of execution because it manipulates viewer windows
     *  in the main thread which is not allowed in OS/2.
     *
     *  the boolean: load_in_progress is set while a call to DvrLoadFile is
     * 	in progress; another load call cannot be called until the first is
     *  complete.
     *
     *  the boolean: allow_display_call is set to indicate that the viewer
     *  is waiting for DvrDisplayFirstPage() to be called. DvrDisplayFirstPage()
     *  should be called only after a call to DvrLoadFile is complete. This
     *  flag is set after at the end of DvrLoadFile().
     */
    Boolean		    load_in_progress;
    Boolean		    allow_display_call;

    FARPROC		    yield_rtn_ptr;	

    CDAenvirontext	    *cur_filename;
    CDAenvirontext	    *cur_format;
    CDAenvirontext	    *cur_opfilename;

    POINT 		    print_origin;

    HBITMAP		    (CDA_APIENTRY *img_fid_to_ddb_rtn)
			    (CDAaddress, HDC, short, short, HANDLE);
    void		    (CDA_APIENTRY *img_free_ddb_rtn)
			    (HBITMAP);

#else /* X/DECwindows (vms,ultrix) specific fields for this struct */

    Widget		    work_window;
    Widget		    h_scroll;
    Widget		    v_scroll;
    Widget		    ps_scroll_window;
    Widget		    ps_window;
    Widget		    ps_scroll_h;
    Widget		    ps_scroll_v;
    Widget		    button_box;
    Widget		    next_pg_but;
    Widget		    prev_pg_but;
    Widget		    top_doc_but;
    Widget		    bot_doc_but;
    Widget		    goto_pg_but;
    Widget		    cancel_but;
    Widget		    scale_lbl;
    Widget		    pg_of_label;
    Widget 		    goto_dialog_box;
    Widget		    goto_text_label;
    Widget		    goto_text_box;
    Widget		    goto_ok_but;
    Widget		    goto_cancel_but;
    Widget 		    message_box;
/*AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    Widget                  audiotog;
    CDAflags		    audio_state;
#endif
/*END AUDIO STUFF*/
    XtCallbackList	    callback;
    XtCallbackList	    expose_callback;
    XtCallbackList	    mouse_motion_callback;
    XtCallbackList	    buttons_callback;
    XtCallbackList	    scroll_bar_callback;
    Cursor 		    ViewerCursor;
    MrmHierarchy	    drm_h;
#endif

    /* the rest of the fields in this struct are platform independent;
     * there should be no more platform specific fields beyond this point
     * in the struct
     */

    PsStruct		    Ps;
    FontStruct		    *font_table;
    Boolean		    h_scroll_flag;
    Boolean		    v_scroll_flag;
    long		    processing_options;
    DvrStruct		    Dvr;
    char		    **available_fonts;
    unsigned long	    num_fonts;
    float		    cent_to_pix_height;
    float		    cent_to_pix_width;
    CDAmeasurement	    x_dpi;
    CDAmeasurement	    y_dpi;
    CDAsize		    default_font_id;	/* Default font ID, not index */
    unsigned long	    *default_font_struct;  /* Default font struct */
    CDAsize		    current_font_index;	/* Index of current font */
    char		    *save_goto_text;
    int			    cursor_ref_count;	/* cursor reference count   */
    int			    paper_width;
    int			    paper_height;
    char		    *page_label_str;
    char		    *of_label_str;
    char		    *default_font_str;
    Boolean		    button_box_flag;
    long		    page_number;
    char		    *doc_info_text_buffer;

/* cbr stuff */

    CbrStruct		    Cbr;

/* end cbr stuff */

    CDAmessagehandle message_handle;
    CDAenvirontext	    message_buffer[512];

    Boolean		    destroy_flag; /* set when widget is being destroyed */

    } DvrViewerPart, *DvrViewerPtr;

/*
 * now define the actual viewer window widget data structure
 */

/*  note, for OS/2, the viewer widget structure contains only the
 *  viewer part of the widget; all references to X widgets are #ifdef'ed
 *  out. OS/2 presentation manager uses handles to windows instead (hwnd's)
 */

typedef struct dvrviewerwidget
    {
#if defined(OS2) || defined(MSWINDOWS)

#else /* X/DECwindows (vms,ultrix) specific fields for this struct */
    CorePart	    	core;			/* basic widget */
    CompositePart   	composite;		/* composite specific data */
    ConstraintPart	constraint;		/* constraint specific data */
    XmManagerPart	manager;		/* manager specific data */
#endif

    DvrViewerPart   dvr_viewer;			/* main specific */
    } DvrViewerWidgetRec, * DvrViewerWidget;

typedef struct
    {
    int		    mumble;		/* nothing special */
    } DvrViewerClassPart;

typedef struct _VwrWindowClassRec
    {
#if defined(OS2) || defined(MSWINDOWS)

#else /* X/DECwindows (vms,ultrix) specific fields for this struct */
    CoreClassPart	  core_class;
    CompositeClassPart	  composite_class;
    ConstraintClassPart	  constraint_class;
    XmManagerClassPart	  manager_class;
#endif

    DvrViewerClassPart	dvr_viewer_class;
    } DvrViewerClassRec, * DvrViewerWidgetClass;


#if defined(OS2) ||defined(MSWINDOWS)

#else /* X/DECwindows (vms,ultrix) specific structs */

/*
**  Translation table including button events.
*/
static char  dvr_buttons_translations [] =
    	"@Help<Btn1Down>:       DVR_HELP()\n\
	 <Btn1Down>:		DVR_BUTTONS()\n\
	 <Btn1Up>:		DVR_BUTTONS()\n\
	 <Btn2Down>:		DVR_BUTTONS()\n\
	 <Btn2Up>:		DVR_BUTTONS()\n\
	 <Btn3Down>:		DVR_BUTTONS()\n\
	 <Btn3Up>:		DVR_BUTTONS()\n\
	 <Btn4Down>:		DVR_BUTTONS()\n\
	 <Btn4Up>:		DVR_BUTTONS()\n\
	 <Btn5Down>:		DVR_BUTTONS()\n\
	 <Btn5Up>:		DVR_BUTTONS()";

/*
**  Translation table including mouse motion events.
*/
static char  dvr_translations_with_motion [] =
    	"@Help<Btn1Down>:       DVR_HELP()\n\
         <Btn1Up>:		DVR_ACTIVATE()\n\
	 <MotionNotify>:	DVR_MOTION()";

/*
**  Translation table including button events and mouse events.
*/
static char  dvr_buttons_translations_with_motion [] =
    	"@Help<Btn1Down>:       DVR_HELP()\n\
	 <Btn1Down>:		DVR_BUTTONS()\n\
	 <Btn1Up>:		DVR_BUTTONS()\n\
	 <Btn2Down>:		DVR_BUTTONS()\n\
	 <Btn2Up>:		DVR_BUTTONS()\n\
	 <Btn3Down>:		DVR_BUTTONS()\n\
	 <Btn3Up>:		DVR_BUTTONS()\n\
	 <Btn4Down>:		DVR_BUTTONS()\n\
	 <Btn4Up>:		DVR_BUTTONS()\n\
	 <Btn5Down>:		DVR_BUTTONS()\n\
	 <Btn5Up>:		DVR_BUTTONS()\n\
	 <MotionNotify>:	DVR_MOTION()";

#endif


/* internal undocumented inter-module entry points */

/*  Dvr_Viewer_Backend is defined in DVR_BACKEND.C
 */
PROTO(CDAstatus CDA_APIENTRY Dvr_Viewer_Backend,
		(CDAconstant *,
		 CDAitemlist *,
		 DvrViewerWidget *,
		 CDAfrontendhandle  *,
		 CDAuserparam * ) );

/*  dvr_format_polyline_object is defined in DVR_DISPLAY_GRAPHICS.C
 */
PROTO( CDAstatus dvr_format_polyline_object,
		(DvrViewerWidget,
		 LIN,
		 PTH *,
		 STATEREF) );

/*  dvr_format_arc_object is defined in DVR_DISPLAY_GRAPHICS.C
 */
PROTO( CDAstatus dvr_format_arc_object,
		(DvrViewerWidget,
		 ARC,
		 PTH *,
		 STATEREF) );

/*  dvr_format_curve_object is defined in DVR_DISPLAY_GRAPHICS.C
 */
PROTO( CDAstatus dvr_format_curve_object,
		(DvrViewerWidget,
		 CRV,
		 STATEREF) );

/*  dvr_format_fill_object is defined in DVR_DISPLAY_GRAPHICS.C
 */
PROTO( CDAstatus dvr_format_fill_object,
		(DvrViewerWidget,
		 CRV,
		 STATEREF) );

/*  dvr_display_page.c is defined in DVR_DISPLAY_PAGE.C
 */
PROTO( CDAstatus dvr_display_page,
		(DvrViewerWidget,
		 CDAmeasurement,
		 CDAmeasurement,
		 CDAmeasurement,
		 CDAmeasurement) );

/*  dvr_format_page is defined in DVR_FORMAT_PAGE.C
 */
PROTO( CDAstatus dvr_format_page,
		(DvrViewerWidget) );

/*  dvr_format_image_object is defined in DVR_IMAGE.C
 */
PROTO( CDAstatus dvr_format_image_object,
		(DvrViewerWidget,
		 IMG,
		 STATEREF) );

/*  dvr_cleanup_page is defined in DVR_IMAGE.C
 */
PROTO( CDAstatus dvr_cleanup_page,
		(DvrViewerWidget) );

/*  dvr_goto_next_page is defined in DVR_PAGE_MGMT.C
 */
PROTO( CDAstatus dvr_goto_next_page,
		(DvrViewerWidget,
		 boolean) );

/*  dvr_goto_page_number is defined in DVR_PAGE_MGMT.C
 */
PROTO( CDAstatus dvr_goto_page_number,
		(DvrViewerWidget,
		 boolean,
		 CDAconstant *) );

/*  dvr_goto_previous_page is defined in DVR_PAGE_MGMT.C
 */
PROTO( CDAstatus dvr_goto_previous_page,
		(DvrViewerWidget,
		 boolean) );

/*  dvr_alloc_memory is defined in DVR_STRUCT_MGMT.C
 */
PROTO( CDAstatus dvr_alloc_memory,
		(DvrViewerWidget,
		 CDAsize,
		 CDAaddress *) );

/*  dvr_alloc_struct is defined in DVR_STRUCT_MGMT.C
 */
PROTO( CDAstatus dvr_alloc_struct,
		(DvrViewerWidget,
		 enum str_type,
		 CDAaddress *) );

/*  dvr_dealloc_memory is defined in DVR_STRUCT_MGMT.C
 */
PROTO( CDAstatus dvr_dealloc_memory,
		(DvrViewerWidget,
		 CDAsize,
		 CDAaddress *) );

/*  dvr_free_file_memory is defined in DVR_STRUCT_MGMT.C
 */
PROTO( CDAstatus dvr_free_file_memory,
		(DvrViewerWidget) );

/*  realloc_memory is defined in DVR_STRUCT_MGMT.C
 */
PROTO( CDAstatus realloc_memory,
		(CDAaddress *,
		 CDAsize *,
		 CDAsize) );

/*  dvr_toolkit_message_routine is define in DVR_UTIL.C
 */
PROTO( CDAstatus CDA_CALLBACK dvr_toolkit_message_routine,
			(CDAaddress,
			 CDAstatus *,
			 CDAsize *,
			 unsigned char *,
			 CDAsize *,
			 unsigned char * *) );

/*  dvr_error_callback is defined in DVR_UTIL.C
 */
PROTO( CDAstatus dvr_error_callback,
		(DvrViewerWidget,
		 CDAconstant,
		 CDAstatus,
		 CDAenvirontext [512],
		 CDAuint32) );

/*  dvr_set_watch_cursor is defined in DVR_UTIL.C
 */
PROTO( CDAstatus dvr_set_watch_cursor,
		(DvrViewerWidget) );

/*  dvr_reset_cursor is defined in DVR_UTIL.C
 */
PROTO( CDAstatus dvr_reset_cursor,
		(DvrViewerWidget) );

/*  dvr_init_font_table is defined in DVR_UTIL.C
 */
PROTO( CDAstatus dvr_init_font_table,
		(DvrViewerWidget) );

/*  dvr_dealloc_font_table is defined in DVR_UTIL.C
 */
PROTO( CDAstatus dvr_dealloc_font_table,
		(DvrViewerWidget) );

#ifdef __vms__

/*  Dvr__DECW_Command is defined in DVR_DCMD.C
 */
PROTO( CDAstatus Dvr__DECW_Command,
		(int, char **) );
#endif


/* cbr changes */

PROTO(CDAstatus DvrCbrPrevRef,
		(DvrViewerWidget, boolean, int *) );

PROTO(CDAstatus DvrCbrNextRef,
		(DvrViewerWidget, boolean, int *) );

PROTO(CDAstatus DvrCbrFindPage,
		(DvrViewerWidget, int *, int) );

PROTO(CDAstatus DvrCbrFirstRef,
		( DvrViewerWidget ) );

PROTO(void DvrCbrSetCtxt,
		( DvrViewerWidget, 
		  PROTO(CDAstatus (*),  (CDAuserparam,
	  		CDAconstant, CDAint32, CDAuint32 *, CDAuint32 **)), 
		  CDAuserparam, CDAuint32 ) );

PROTO(void DvrCbrClearCtxt,
		( DvrViewerWidget ) );

PROTO(void DvrCbrManageRefs,
		( DvrViewerWidget ) );

PROTO(void DvrCbrUnmanageRefs,
		( DvrViewerWidget ) );

PROTO(CDAstatus Dvr__DECW_Cbr_Clear_Mode,
		( void ) );					/* no parameters */

PROTO(CDAstatus Dvr__DECW_Cbr_Set_Mode,
		( CDAuint32, CDAstatus (*)(), CDAstatus (*)(), CDAuserparam, CDAuint32, CDAaddress) );

/* end cbr changes */



/*  get_page_data is defined in both DVR_ACTIONS.C (vms, ultrix)
 *  and in DVR_ACT.C (OS/2)
 */
PROTO( CDAstatus get_page_data,
		(void *,
		 unsigned long *,
		 unsigned long *) );

/*  dvr_adjust_sliders is defined in both DVR_ACTIONS.C (vms, ultrix)
 *  and in DVR_ACT.C (OS/2)
 */
PROTO( CDAstatus dvr_adjust_sliders,
		(DvrViewerWidget) );

/*  dvr_initialize_window is defined in both DVR_CREATE.C (vms, ultrix)
 *  and in DVR_CRE.C (OS/2)
 */
PROTO( CDAstatus dvr_initialize_window,
		(DvrViewerWidget) );

/*  dvr_set_page_number is defined in both DVR_CREATE.C (vms, ultrix)
 *  and in DVR_CRE.C (OS/2)
 */
PROTO( CDAstatus dvr_set_page_number,
                (DvrViewerWidget) );

/*  dvr_update_page_number is defined in both DVR_CREATE.C (vms, ultrix)
 *  and in DVR_CRE.C (OS/2)
 */
PROTO( CDAstatus dvr_update_page_number,
                (DvrViewerWidget) );

/*  dvr_call_scroll_bar_callbacks is defined in DVR_CREATE.C (vms, ultrix)
 * and in DVR_CRE.C (OS/2 - Not yet written)
 */
PROTO( void dvr_call_scroll_bar_callbacks,
		(DvrViewerWidget, int, int, int, int, int, int) );

/*  dvr_output_pattern is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_output_pattern,
		(DvrViewerWidget,
		 PAT,
		 boolean *) );

/*  dvr_fill_path is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_fill_path,
		(PTH,
		 GAT,
                 DvrViewerWidget,
		 longword,
		 STR) );

/*  dvr_stroke_path is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_stroke_path,
		(PTH,
		 GAT,
                 DvrViewerWidget,
		 longword,
		 STR,
		 longword,
		 byte *) );

/*  dvr_draw_line is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( void dvr_draw_line,
		(DvrViewerWidget,
		 CDAmeasurement, CDAmeasurement,
		 CDAmeasurement, CDAmeasurement,
		 CDAmeasurement) );

/*  dvr_display_polyline_object is defined in DVR_GRAPHICS.C (vms, ultrix),
 *  in DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_display_polyline_object,
		(DvrViewerWidget,
		 LIN) );

/*  dvr_display_arc_object is defined in DVR_GRAPHICS.C (vms, ultrix),
 *  in DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_display_arc_object,
		(DvrViewerWidget,
		 ARC) );

/*  dvr_display_curve_object is defined in DVR_GRAPHICS.C (vms, ultrix),
 *  in DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_display_curve_object,
		(DvrViewerWidget,
		 CRV) );

/*  dvr_display_fill_object is defined in DVR_GRAPHICS.C (vms, ultrix),
 *  in DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_display_fill_object,
		(DvrViewerWidget,
		 LIN) );

/*  dvr_text_width is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAmeasurement dvr_text_width,
		(DvrViewerWidget,
		 unsigned long *,
		 char *,
		 CDAsize) );

/*  dvr_application_callback is defined in DVR_GRAPHICS.C (vms, ultrix),
 *  in DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_application_callback,
		(DvrViewerWidget,
		 void *) );

/*  dvr_load_cursor is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_load_cursor,
		(DvrViewerWidget) );

/*  dvr_get_font_info is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_get_font_info,
		(DvrViewerWidget,
		 int,
		 char *) );

/*  dvr_unload_fonts is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_unload_fonts,
		(DvrViewerWidget) );

/*  dvr_set_font is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_set_font,
		(DvrViewerWidget,
		 int) );

/*  dvr_draw_string is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_draw_string,
		(DvrViewerWidget,
		 CDAmeasurement,
		 CDAmeasurement,
		 char *,
		 CDAsize) );

/*  dvr_set_clip is defined in DVR_GRAPHICS.C (vms, ultrix), in DVR_GRA.C (OS/2)
 *  and in DVR_GRAD.C (MS-Windows)
 */
PROTO( void dvr_set_clip,
		(BBOX_UL_LR *,
		 DvrViewerWidget) );

/*  dvr_set_ts_origin is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus  dvr_set_ts_origin,
		(DvrViewerWidget) );

/*  dvr_display_image_object is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_display_image_object,
		(DvrViewerWidget,
		 IMG,
		 CDAmeasurement,
		 CDAmeasurement) );

/*  dvr_delete_page_colors is defined in DVR_GRAPHICS.C (vms, ultrix), in
 *  DVR_GRA.C (OS/2), and in DVR_GRAD.C (MS-Windows)
 */
PROTO( CDAstatus dvr_delete_page_colors,
		(DvrViewerWidget) );




/*  the following procedures are defined on os/2, on MS-Windows, and
 *  on vms & ultrix BUT they expect different parameters
 *  on various platforms
 */

#if defined(OS2) || defined(MSWINDOWS)

/*  the OS/2 version of dvr_kill_widget is defined in DVR_GRA.C
 *  the MS-Windows version of dvr_kill_widget is defined in DVR_GRAD.C
 */
PROTO( unsigned long dvr_kill_widget,
		(HWND) );

/*  the OS/2 version of dvr_image_condition_handler is defined in DVR_IMAGE.C
 */
PROTO( void CDA_APIENTRY dvr_image_condition_handler,
		(unsigned short,
		 unsigned short) );

/*  the OS/2 version of dvr_scroll_action_proc is defined in DVR_ACT.C
 */
#ifdef OS2
PROTO( void dvr_scroll_action_proc,
		(DvrViewerWidget,
		 HWND,
		 int,
		 short) );
#endif
/*  the MS-Windows version of dvr_scroll_action_proc is defined in DVR_ACTD.C
 */
#ifdef MSWINDOWS
PROTO( void dvr_scroll_action_proc,
		(DvrViewerWidget,
		 HWND,
		 int,
		 short,
		 BOOL) );
#endif
/*  the OS/2 version of dvr_stroke_polyline is defined in DVR_GRA.C
 *  the MS-Windows version of dvr_stroke_polyline is defined in DVR_GRAD.C
 */
PROTO( unsigned long dvr_stroke_polyline,
		(DvrViewerWidget,
		 POINT *,
		 int,
		 longword,
		 byte *) );

#else /* X/DECwindows (vms, ultrix) protos for the same routines */

/*  the vms & ultrix version of dvr_kill_widget is defined in DVR_GRAPHICS.C
 */
PROTO( CDAstatus dvr_kill_widget,
		(Widget) );


/*  the vms & ultrix version of dvr_image_condition_handler is defined in DVR_IMAGE.C
 */

#ifdef __vms__

PROTO( unsigned long dvr_image_condition_handler,
		(struct chf$signal_array *,
		 struct chf$mech_array *) );

#else /* ultrix */

PROTO( unsigned long dvr_image_condition_handler,
		(unsigned int *,
		 unsigned int *) );

#endif


/*  the vms & ultrix version of dvr_scroll_action_proc is defined in DVR_ACTIONS.C
 */
PROTO( void dvr_scroll_action_proc,
		(Widget,
		 char *,
		 XmScrollBarCallbackStruct *) );

/*  the vms & ultrix version of dvr_stroke_polyline is defined in DVR_GRAPHICS.C
 */
PROTO( unsigned long dvr_stroke_polyline,
		(DvrViewerWidget,
		 XPoint *,
		 int,
		 longword,
		 byte *) );
#endif


/*  OS2 specific routines */

#if defined(OS2) || defined(MSWINDOWS)

/*  dvr_work_width is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( long dvr_work_width,
		(DvrViewerWidget) );

/*  dvr_work_height is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( long dvr_work_height,
		(DvrViewerWidget) );

/*  dvr_object_bbox_included is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( Boolean dvr_object_bbox_included,
		(OBJ_PRIVATE,
		 BBOX_REF) );

/*  dvr_max is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( long dvr_max,
		(long,
		 long) );

/*  dvr_min is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( long dvr_min,
		(long,
		 long) );

/*  dvr_output_text_pattern is defined in DVR_GRA.C (OS/2)
 *  dvr_output_text_pattern is defined in DVR_GRAD.C (MS-Windows)
 */
PROTO( unsigned long dvr_output_text_pattern,
		(DvrViewerWidget,
		 PAT,
		 boolean *) );

/*  dvr_calculate_max_bbox is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( unsigned long dvr_calculate_max_bbox,
		(STATEREF,
		 STATEREF) );

/*  dvr_object_bbox_included_b is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( Boolean dvr_object_bbox_included_b,
		(BBOX_REF,
		 BBOX_REF) );

/*  dvr_calculate_max_bbox_p is defined in DVR_UTIL.C (OS/2 and MS-Windows)
 */
PROTO( unsigned long dvr_calculate_max_bbox_p,
		(STATEREF,
		 PAG_PRIVATE) );

#ifdef OS2
/*  dvr_get_font_list is defined in DVR_DWF.C (OS/2 only)
 */
PROTO( unsigned long dvr_get_font_list,
		(DvrViewerWidget) );

/*  dvr_load_font_dlls is defined in DVR_DWF.C (OS/2 only)
 */
PROTO( unsigned long dvr_load_font_dlls,
		(DvrViewerWidget,
		 Boolean) );

/*  dvr_load_os2_font is defined in DVR_DWF.C (OS/2 only)
 */
PROTO( unsigned long dvr_load_os2_font,
    		(DvrViewerWidget,
    		 HPS,
    		 char *) );

/*  dvr_pm_text_width is defined in DVR_DWF.C (OS/2 only)
 */
PROTO( unsigned long dvr_pm_text_width,
    		(DvrViewerWidget,
		 HPS,
    		 char *) );


/*  dvr_move_image_widgets is defined in DVR_ACT.C (OS/2 only)
 */
PROTO( void dvr_move_image_widgets,
                (DvrViewerWidget) );

/*  DvrClientWndProc is defined in DVR_CRE.C (OS/2 only)
 */
PROTO( MRESULT EXPENTRY DvrClientWndProc,
		(HWND,
		 USHORT,
		 MPARAM,
		 MPARAM) );

/*  DvrWorkWndProc is defined in DVR_CRE.C (OS/2 only)
 */
PROTO( MRESULT EXPENTRY DvrWorkWndProc,
		(HWND,
		 USHORT,
		 MPARAM,
		 MPARAM) );

#else  /* For MS-Windows... */

/*  DvrClientWndProc is defined in DVR_INID.C (MS-Windows only)
 */
PROTO( LONG FAR PASCAL _export DvrViewerWndProc,
		(HWND,
		 UINT,
		 UINT,
		 LONG) );

/*  DvrWorkWndProc is defined in DVR_INID.C (MS-Windows only)
 */
PROTO( LONG FAR PASCAL _export DvrWorkWndProc,
		(HWND,
		 UINT,
		 UINT,
		 LONG) );

/* dvr_establish_font_data is defined in DVR_GRAD.C (MS-Windows only)
 */
PROTO( CDAstatus dvr_establish_font_data,
	(DvrViewerWidget));

/* dvr_destroy_font_data is defined in DVR_GRAD.C (MS-Windows only)
 */
PROTO( CDAstatus dvr_destroy_font_data,
	(DvrViewerWidget));

/* dvr_force_redraw is defined in DVR_GRAD.C (MS-Windows only)
 */
PROTO( CDAstatus dvr_force_redraw,
	(DvrViewerWidget,
	 long,
	 long));

/* dvr_reset_scroll_ranges is defined in DVR_GRAD.C (MS-Windows only)
 */
PROTO( CDAstatus dvr_reset_scroll_ranges,
	(DvrViewerWidget,
	 int,
	 int));
#endif


#else /* X/DECwindows (vms, ultrix) specific routines */

/*  dvr_put_image_to_screen is defined in DVR_GRAPHICS.C (vms & ultrix only)
 */
PROTO( unsigned long dvr_put_image_to_screen,
		(DvrViewerWidget,
		 IMG,
		 int,
		 int) );

/*  dvr_call_callbacks is defined in DVR_CREATE.C (vms & ultrix only)
 */
PROTO( void dvr_call_callbacks,
	(DvrViewerWidget, int, XEvent *));

/*  dvr_call_expose_callbacks is defined in DVR_CREATE.C (vms & ultrix only)
 */
PROTO( void dvr_call_expose_callbacks,
	(DvrViewerWidget, XExposeEvent *));

/*  dvr_call_eod_callbacks is defined in DVR_ACTIONS.C (vms & ultrix only)
 */

PROTO( void dvr_call_eod_callbacks,
	(DvrViewerWidget) );

#ifdef DVR_PSVIEW

/*  dvr_init_ps is defined in DVR_PS.C (vms & ultrix only)
 */

PROTO( void dvr_init_ps,
	(DvrViewerWidget) );

/*  dvr_view_ps_file is defined in DVR_PS.C (vms & ultrix only)
 */

PROTO( void dvr_view_ps_file,
	(DvrViewerWidget, char *) );

/*  dvr_ps_error_proc is defined in DVR_PS.C (vms & ultrix only)
 */

PROTO( void dvr_ps_error_proc,
	(Widget, char *, int) );

/*  dvr_ps_text_proc is defined in DVR_PS.C (vms & ultrix only)
 */

PROTO( void dvr_ps_text_proc,
	(Widget, char *, int) );

/*  dvr_set_viewer_normal is defined in DVR_PS.C (vms & ultrix only)
 */

PROTO( CDAstatus dvr_set_viewer_normal,
	(DvrViewerWidget) );

/*  EnableAbort is defined in DVR_PS.C (vms & ultrix only)
 */

PROTO( void EnableAbort,
	(DvrViewerWidget, Boolean) );

/*  dvr_abort_ps_view is defined in DVR_PS.C (vms & ultrix only)
 */

PROTO( void dvr_abort_ps_view,
	(DvrViewerWidget) );

/* dvr_resize_ps_widget is defined in DVR_CREATE.c (vms & ultrix only)
 */

PROTO( void dvr_resize_ps_widget,
	(DvrViewerWidget) );

#endif /* DVR_PSVIEW */

/* dvr_resize is defined in DVR_CREATE.c (vms & ultrix only)
 */

PROTO( void dvr_resize,
	(DvrViewerWidget) );

#endif


/*AUDIO STUFF*/

/* The following routines are defined in DVR_AUDIO.C */

#ifdef CDA_AUDIO_SUPPORT
PROTO (unsigned long dvr_format_audio_object,
			(DvrViewerWidget vw,
			 AUD audio,
			 STATEREF state));

PROTO (unsigned long dvr_display_audio_object,
			(DvrViewerWidget vw,
			 AUD   audio,
			 long  x_window_top,
			 long  y_window_top));

PROTO (void dvr_display_audio_buttons,
			(DvrViewerWidget vw,
			 PAG page));

PROTO (void dvr_delete_audio_buttons,
			(PAG page));

PROTO (void dvr_unmanage_audio_buttons,
			(PAG page));

PROTO (void dvr_audio_button_cb,
			(Widget		      w,
			 int		     *tag,
			 XmAnyCallbackStruct *reason));

PROTO (void dvr_audio_toggle_cb,
			(Widget		      w,
			 int		     *tag,
			 XmAnyCallbackStruct *reason));

PROTO (void dvr_audio_free_channel,());



#endif
/*END AUDIO STUFF*/

/*
 ** For Ultrix and VMS call dvr_reset_gc to reset the GC
 */

#ifdef CDA_DECWINDOWS

PROTO( void dvr_reset_gc, (DvrViewerWidget) );

#endif

#endif /* ifndef DVR_VIEW_TYPES */
