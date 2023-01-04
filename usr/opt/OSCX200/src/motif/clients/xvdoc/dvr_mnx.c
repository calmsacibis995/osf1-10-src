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
#define Module DVR_MNX
#define Ident  "V02-078"

/*
**++
**   COPYRIGHT (c) 1988, 1992 BY
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
**	This is the driver module for the ddif viewer application.
**	It creates a main window with a menu bar and a ddif viewer
**	work window using UIL/DRM.
**
** AUTHORS:
**      Dennis McEvoy,  15-Feb-1987
**
**
** MODIFIED BY:
**
**	V02-001		DAM0001		Dennis A. McEvoy	10-Apr-1989
**		Add code for radio box with size units use one
**		create_proc() instead of multiple
**			DAM0001		Dennis McEvoy		9-May-1989
**		use enviornment variable to find UID file on ultrix;
**		change class-name to usable value for applcation specific
**		    defaults file;
**		for DECWINDOWS V2 and after, open DRM before calling
**		    XtInitialize() so we can get class and application
**		    name from UIL (and allow them to be translated)
**		do not use message facility for our own status values per
**		    request from I18N group;
**			DAM0001		Dennis McEvoy		17-may-89
**		do not let default size grow larger than screen
**			DAM0001		Dennis McEvoy		18-may-89
**		    do not override Xdefaults values for x,y
**	V02-002		DAM0002		Dennis A. McEvoy	15-jun-89
**		clear title bar on unsupported format
**	V02-003		DAM0003		Dennis A. McEvoy	25-jul-89
**		put open file name in icon
**	V02-004		DAM0004		Dennis A. McEvoy	09-aug-89
**		use screen number when creating icons
**	V02-005		DAM0005		Dennis A. McEvoy	10-aug-89
**		add smarts to have file filter mask match format in
**		open dialog box
**	V02-006		DAM0006		Dennis A. McEvoy	19-oct-89
**		add smarts for ps viewing
**	V02-007		DAM0007		Dennis A. McEvoy	06-apr-90
**		take out free() of doc info buffer; now handled in widget
**	V02-008		DAM0008		Dennis A. McEvoy	05-jul-90
**		check for new cda status: CDA_ICVNOTFND
**	V02-009		DAM0009		Dennis A. McEvoy	16-jul-90
**		use y dpi for vertical calculations
**	V02-010		SJM0000		Stephen Munyan		27-Jun-1990
**		Conversion to Motif
**	V02-011		SJM0000		Stephen Munyan		16-Aug-1990
**		Removed the hack in the file selection widget that prevented
**		the filter string from being updated.  Note the hack was removed
**		as the result of a bug in the file selection widget being corrected
**		in BL5.
**	V02-012		SJM0000		Stephen Munyan		 9-Oct-1990
**		Merge in CBR changes from Charlie Chan
**	V02-013		SJM0000		Stephen Munyan		 2-Nov-1990
**
**		Made a notation that XtAddActions should be converted to
**		XtAppAddActions in the future when the CDA Viewer switches
**		over to the new X11R4 Xt routines.
**	V02-014		DAM0010		Dennis A. McEvoy	16-nov-90
**		add x error handler
**	V02-015		DAM0011		Dennis A. McEvoy	19-nov-90
**		fix watch cursor fallback
**	V02-016		SJM0000		Stephen Munyan		26-Nov-1990
**		changed from using XQueryExtension to XListExtensions
**		in order to prevent the XDPS extension from being loaded
**		until it's needed.
**	V02-017		DAM0012		Dennis A. McEvoy	27-Nov-90
**		fixed x error handler logic to use server routines
**		to look up error messages.
**	V02-018		SJM0000		Stephen J. Munyan	30-Dec-1990
**		fixed 10 byte memory leak on every XmGetNextSegment call
**		since the character set is now being returned as an ASCIZ
**		string.
**	V02-019		RKN0001		Ram K. Nori		13-Dec-90
**		Added a routine to strip the file spec into dir & filename
**		w/o file extension. Fixed the file filter bug. i.e on
**		selecting a format, the currently selected format is appened
**		as a file extension.
**
**	V02-020		SJM		Stephen Munyan		 4-Feb-1991
**
**		Add new compound strings for help topic lookup so that
**		when the help topics in the menu bar are translated, we
**		can still look up the topic names independently.  This
**		fix was required for I18N support.
**
**	V02-021		SJM		Stephen Munyan		13-Feb-1991
**
**		Change to Motif resource names for Icon resources in order
**		to get rid of some old Xt references.
**
**		Also swapped the strcpy operations for DVR_GRAPHICFAIL
**		and DVR_IMAGEFAIL since the message strings that were
**		being copied were backwards.
**	V02-022		DAM0022		Dennis A. McEvoy	19-feb-91
**		fix for ultrix
**
**	V02-023		SJM0000		Stephen Munyan		20-Feb-1991
**		Change to using "On Version" and "On Window" for help
**		pull downs to be Motif compliant.
**
**	V02-024		KMRK		Kathy Robinson		23-Feb-1991
**		Free compound strings used for SetARgs calls
**
**	V02-025		DAM		Dennis McEvoy		04-mar-1991
**		remove call to XtInitailize(); call routines ourselves
**		to maintain compatability with v2 .dat files
**	V02-026		DAM             Dennis McEvoy           19-mar-1991
**		modify call to XtInitialize to exit properly if display
**		is not set
**	V02-027		DAM             Dennis McEvoy           29-mar-1991
**		remove duplicate message fetching code
**	V02-028		DAM             Dennis McEvoy           03-apr-1991
**		cleanup typedefs
**	V02-029		DAM             Dennis McEvoy           04-apr-1991
**		fix vms error handling
**	V02-030		DAM             Dennis McEvoy           05-apr-1991
**		add new icon support
**	V02-031		SJM		Stephen Munyan		09-Apr-1991
**		Added On Help Menu pull down menu item.
**	V02-032		DAM		Dennis McEvoy		26-Apr-1991
**		fix for icon code from J Ferguson
**	V02-033		DAM		Dennis McEvoy		28-Apr-1991
**		fix compound string accvio on ultrix
**	V02-034		DAM		Dennis McEvoy		28-Apr-1991
**		fix format/filter matching for ultrix
**      V02-035	        DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**      V02-036	        DAM0001		Dennis McEvoy		07-may-1991
**		add lookup code for ultrix new style converters
**      V02-037	        DAM0001		Dennis McEvoy		07-may-1991
**		remove condition handler for ultrix; no longer needed
**		since we no longer use the ids widget and we do not call
**		ids/img from the application
**      V02-038	        DAM0001		Dennis McEvoy		15-may-1991
**		change readdir parameter for osf/1 cvt lookup
**      V02-039	        DAM0001		Dennis McEvoy		16-may-1991
**		close Mrm Hierarchy on exit
**      V02-040	        DAM0001		Dennis McEvoy		22-may-1991
**		do not destroy ps options dialog box if xdps is
**		not present; instead just leave insensitive; we cannot
**		delete the ps options box because it is now part of
**		a form dialog box with attachment dependencies
**	V02-041		DAM0001		Dennis McEvoy		24-may-1991
**		fix bug in icon fetch
**	V02-042		DAM0001		Dennis McEvoy		28-may-1991
**		use latest motif calls for ultrix
**	V02-043		DAM0001		Dennis McEvoy		11-jun-1991
**		cleanup new style converter lookup (fix alignment error)
**	V02-044		DAM0001		Dennis McEvoy		12-jun-1991
**		fix default filter for ultrix
**	V02-045		SJM0000		Stephen Munyan		17-Jul-1991
**		DEC C Cleanups (mumble became mumble_STATE) to clean up
**		message code severity code.
**	V02-046		DAM0001		Dennis McEvoy		03-jul-1991
**		make sure ddif and dtif are listed for ultrix format
**		list box
**	V02-047		DAM0001		Dennis McEvoy		22-jul-1991
**		remove out of date testing #ifs (LOW_LEVEL, VMS_BINDING, etc)
**	V02-048		DAM0001		Dennis McEvoy		25-jul-1991
**		update icon code to same as checked in for ssb decw v3
**	V02-049		SJM0000		Stephen Munyan		29-Jul-1991
**		DEC C Cleanups
**	V02-050		SJM0000		Stephen Munyan		 2-Aug-1991
**		Merge in DXIsXUIWMRunning() which was originally
**		added to the DECwindows V3.0 SSB stream by Jackie Ferguson
**		in the DECWIN sources on 18-Jun-1991.
**	V02-051		DAM0001		Dennis McEvoy		05-aug-1991
**		renamed headers, removed dollar signs
**	V02-052		DAM0001		Dennis McEvoy		12-aug-1991
**		Merge in DXIsXUIWMRunning() for un*x as well
**	V02-053		DAM0001		Dennis McEvoy		13-aug-1991
**		remove LOCAL_UID conditional
**	V02-054		DAM0001		Dennis McEvoy		04-sep-1991
**		remove obsolete #ifdefs
**      V02-055         JJT0001         Jeff Tancill            16-sep-1991
**              remove local XtIsRealized definition, not needed
**              anymore since NULL is defined as 0 now.
**	V02-056		DAM000		Dennis McEvoy		01-oct-1991
**		fix strip_file_spec for ultrix
**	V02-057		DAM000		Dennis McEvoy		03-oct-1991
**		cleanup X calls to match protos
**	V02-058		DAM000		Dennis McEvoy		03-oct-1991
**		add protos for un*x system calls w/o protos
**	V02-059		DAM000		Dennis McEvoy		21-oct-1991
**		cleanup fileCompare to match proto for unix qsort
**	V02-060		DAM000		Dennis McEvoy		22-oct-1991
**		change fileCompare back for ultrix only
**      V02-061         DAM000          Dennis McEvoy		28-oct-1991
**		change fileCompare for sun as well
**      V02-062         DAM000          Dennis McEvoy		28-oct-1991
**		conditionalize ps viewing based on DVR_PSVIEW
**		(defined in dvrwint.h)
**	V02-063		SJM000		Stephen Munyan		 9-Jan-1992
**		Merge in changes from DEC Japan
**	V02-064		RKN000		Ram Kumar Nori		20-Jan-1992
**		Implemented Hyper help with context sensitive help
**	V02-065		SJM000		Stephen Munyan		 1-Feb-1992
**		Fixed error in error message processing for arrow heads
**      V02-066         RKN000          Ram Kumar Nori          17-Mar-1992
**              Open Hyperhelp only if requested. Do not exit if hyperhlep
**              returns a status 1 (Bookreader Busy).
**      V02-067         RKN000          Ram Kumar Nori          01-Apr-1992
**              Set help callback on Audio button.
**      V02-068         ECR000          Elizabeth C. Rust	06-Apr-1992
**              Add XtFree(context) to close memory leaks of StringContext.
**      V02-069         DAM000          Dennis McEvoy		15-Apr-1992
**              fix memory leak with diag info text buffer
**      V02-070         ECR000          Elizabeth C. Rust	28-Apr-1992
**              Add check for null pointer after XtCreateApplicationContext.
**      V02-071		CJR000		Chris Ralto		18-Jun-1992
**              Change symbol __osf1__ to __osf__ for OSF/ALPHA.
**      V02-072		DAM000		Dennis McEvoy		22-Jun-1992
**              make cbr ref param a CDAuserparam to match proto
**      V02-073		CJR000		Chris Ralto		02-Jul-1992
**              Replace call to XtAppErrorMsg (which either hangs or stack
**		dumps) with fprintf to stderr and then exit, if XtOpenDisplay
**		fails.
**      V02-074		RDH001		Don Haney		08-Jul-1992
**		Add casts required for strict checking in OSF1/ALPHA
**
**	V02-075		RDH003		Don Haney		15-Jul-1992
**		Change class name to dxvdoc, uid filenames to dxvdoc.uid, and
**		use sizeof to compute lengths of uid filenames.
**
**	V02-076		RDH006		Don Haney		24-Aug-1992
**		Re-sequence includes to eliminate MAX redefined warning
**
**      V02-077         RJD000          Ronan Duke	      30-Aug-1993
**              Include Xm/MAnagerP.h if linking against Motif V1.2
**
**      V02-078         RJD001          Ronan Duke	       6-Sep-1993
**              Cast XmCR_* enum values to be int to avoid compiler warnings
**
**--
*/

/*
**
**  INCLUDE FILES
**
**/
#include <cdatrans.h>

#ifdef __vms__

#include <dvrint.h>				/* DVR internal definitions */
#include <fscndef.h>
#include <descrip.h>
#include <ssdef.h>
#include <stsdef.h>
#include <rmsdef.h>
#include <decw$cursor.h>
#include <chfdef.h>
#include <decw$xlibmsg.h>

#else

#include <DPS/dpsXclient.h>
#include <X11/decwcursor.h>
#include <dvrint.h>				/* DVR internal definitions */
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/ioctl.h>
#endif

#ifdef __vms__

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */
#endif

#include <Xm/Xm.h>				/* Motif definitions */
#include <Xm/List.h>				/* Scrolled List definitions */
#include <Xm/MessageB.h>			/* Message widget definitions */
#include <Xm/Scale.h>				/* Scale widget definitions */
#include <Xm/SelectioB.h>			/* Selection Box definitions */
#include <Xm/Text.h>				/* Text Widget definitions */
#include <Xm/ToggleB.h>				/* Toggle button definitions */
#include <Xm/AtomMgr.h>

/* RD:
 * if linking against V1.2 of Motif then need to explicitly include 
 * Xm/ManagerP.h to get XmManagerPart definition
 */
#ifdef MOTIF_V12
#include <Xm/ManagerP.h>	
#endif

#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */
#include <Mrm/MrmDecls.h>			/* Motif Resource Manager definitions */

#include <DXm/DECspecific.h>
#include <DXm/DXmHelpB.h>			/* DxmNfirstTopic, etc. */
#include <X11/ShellP.h>				/* Required by DXmHelpSP.h */
#include <DXm/DXmHelpSP.h>			/* helpshell widgetclass */
#include <X11/Vendor.h>
#include <X11/cursorfont.h>

#ifdef __vms__

#pragma standard				/* turn /stand=port back on */

#endif

#include <dvrwdef.h>			/* DECwindows Viewer Public Definitions		*/
#include <dvrwint.h>			/* DECwindows Viewer Internal Definitions	*/
#include <dvrwptp.h>			/* DECwindows Viewer Public Prototypes		*/



/*
** Local PROTO statements
*/

static 	void 	set_icons_on_shell();
	void 	set_icons ();
	Pixmap 	fetch_icon_literal();
static 	char 	*get_icon_index_name();
static 	void 	set_iconify_pixmap();
static 	Boolean dvr_app_check_for_xdps();
	void		dir_file_scan();
	void		upper_case();
	void		change_title();
	void		viewer_new_file();
	CDAstatus	get_uil_strings();
	void		strip_directory();
	void		free_doc_info_buffer();
	void		set_watch_cursor();
	void		free_uil_strings();
	void		set_ps_initial_values();
	void		change_filter();
	CDAstatus	fetch_compound_string();
	CDAstatus	fetch_simple_string();
	CDAstatus	get_message_text();
	void		reset_cursor();
	CDAstatus	fdone();
	void		_fsquish();
	void		MainLoop();
static	void		tracking_help ();
static	void		help_error ();
static  void		if_error_report ();
static	void		set_viewer_widget_callbacks();


PROTO(Boolean DXIsXUIWMRunning, (Widget, Boolean) );

#ifdef __unix__
#if defined(ultrix) || defined(sun)
PROTO( int fileCompare, (char **, char **) );
#else
/* ANSI C, match proto for qsort (does not compile on mips ultrix)  */
PROTO( int fileCompare, (const void *, const void *) );
#endif
#endif

/*
 * Prototypes for DEC C since we can't use lib$routines.h or starlet.h
 * yet since they don't yet work with DEC C.  Hopefully this limitation
 * will be raised by the time we ship so we can remove this kludge.
*/

#ifdef __vms__

PROTO(struct FCT * _ImgCreateFrameFromSegAggr, (CDArootagghandle, CDAagghandle));
PROTO(void * LIB$ESTABLISH, ());
PROTO(CDAstatus LIB$SIG_TO_RET, ());
PROTO(CDAstatus LIB$FIND_FILE, ());
PROTO(CDAstatus LIB$FIND_FILE_END, ());
PROTO(CDAstatus SYS$GETMSG, ());
PROTO(CDAstatus SYS$FILESCAN, ());

#endif


/*
**
**  EXTERNAL and GLOBAL SPECIFIERS
**
**/

/*
 *  DEFINES
 */

/*
 *  size to increment diagnostic info buffer
 *  (50 columns by 10 rows)
 */
#define DIAG_BUFF_INCREMENT 	10000


/*
 *  strings to pass to XtInitialize
 */

#ifdef __vms__
#define dvr_class_str		"DDIF$VIEW"
#else
#define dvr_class_str		"dxvdoc"
#endif

#define dvr_xdefaults_name_str  "Viewer"

/*
 *  strings to blast on screen; these are errors that
 *  might occur before/after UIL is available; all other strings
 *  are fetched from UIL;
 */
#define dvr_drm_reg_fail_str    "\nDVR could not register widget, aborting\n"
#define dvr_drm_ctx_fail_str	"\nDVR could not create application context, aborting\n"
#define dvr_drm_dpy_fail_str    "\nDVR could not open display, aborting\n"
#define dvr_drm_open_hier_str	"\nDVR could not open DRM hierarchy, aborting\n"
#define dvr_drm_main_ffail_str	"\nDVR could not fetch main window, aborting\n"
#define dvr_drm_str_ffail_str	"\nDVR could not fetch strings, aborting\n"
#define dvr_drm_popup_ffail_str	"\nDVR could not fetch pop-up window\n"
#define dvr_memdeallofail_str 	"\nDVR memory deallocation error\n"


/*
 *  MACROS
 */

#define SEVERITY(code)		\
    (code & 0x00000007)

/*
 * macro to change cursor to watch
*/

#define StartMainWait(widget)\
	{\
	if ( XtIsRealized (widget) )\
	    {\
	    XDefineCursor ( XtDisplay(widget), XtWindow(widget), \
            WatchCursor );\
	    XFlush(XtDisplay(widget));\
	    }\
	}

/*
 * macro to switch cursor back to default
*/

#define StopMainWait(widget)\
	{\
	if ( XtIsRealized (widget) )\
	    {\
	    XUndefineCursor ( XtDisplay(widget), XtWindow(widget) );\
	    }\
	}

#define POSTSCRIPT_STR	format_filter_table[14].format

#define ARG(n, v) {n, (XtArgVal) v}



/*
 *  ICON DATA
 */

#define viewer_width 32
#define viewer_height 32
static char viewer_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0x07, 0x40, 0x00, 0x00, 0x04,
   0xa0, 0xff, 0xf8, 0x05, 0xa0, 0x80, 0x00, 0x04, 0xa0, 0xb0, 0xfe, 0x05,
   0xa0, 0x84, 0x00, 0x04, 0xa0, 0x9a, 0xfe, 0x05, 0xa0, 0xa1, 0x00, 0x04,
   0xa0, 0xff, 0x7e, 0x04, 0x20, 0x00, 0x00, 0x04, 0x20, 0x00, 0x00, 0x04,
   0xa0, 0xfd, 0xfe, 0x05, 0x20, 0x00, 0x00, 0x04, 0xa0, 0xff, 0xfe, 0x05,
   0x20, 0x00, 0x00, 0x04, 0xa0, 0x01, 0x98, 0x05, 0xa0, 0x61, 0x18, 0x05,
   0xa0, 0xe1, 0x19, 0x05, 0xa0, 0x61, 0x99, 0x05, 0xa0, 0x61, 0x19, 0x05,
   0xa0, 0x67, 0x79, 0x05, 0xa0, 0x65, 0x59, 0x05, 0xa0, 0x65, 0xd9, 0x05,
   0xa0, 0x65, 0x59, 0x05, 0xa0, 0x65, 0x59, 0x05, 0xa0, 0x65, 0x59, 0x05,
   0xa0, 0xe7, 0xf9, 0x05, 0x20, 0x00, 0x00, 0x04, 0x20, 0x00, 0x00, 0x04,
   0xe0, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00};

#define sm_viewer_width 17
#define sm_viewer_height 17
static char sm_viewer_bits[] = {
   0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00, 0x08, 0x20, 0x00, 0xe8, 0x2d, 0x00,
   0x28, 0x21, 0x00, 0xe8, 0x2d, 0x00, 0x28, 0x21, 0x00, 0xe8, 0x2d, 0x00,
   0x08, 0x20, 0x00, 0xe8, 0x2e, 0x00, 0x08, 0x20, 0x00, 0x28, 0x22, 0x00,
   0xa8, 0x22, 0x00, 0xa8, 0x2a, 0x00, 0xa8, 0x2a, 0x00, 0x08, 0x20, 0x00,
   0xf8, 0x3f, 0x00};



/*
 *  constants for paper sizes
 */
#define DVR_CHAR_UNITS	1
#define DVR_INCH_UNITS  2
#define DVR_MM_UNITS	3

#define MM_PER_INCH	25.4
#define CPNTS_PER_INCH 	7200
#define TWELVE_PT_WID	720
#define TWELVE_PT_HEI   1200

static float INCH_PER_CHAR_W = (float) TWELVE_PT_WID/ (float) CPNTS_PER_INCH;
static float INCH_PER_CHAR_H = (float) TWELVE_PT_HEI/ (float) CPNTS_PER_INCH;

static float MM_PER_CHAR_W;
static float MM_PER_CHAR_H;

#define A0_MM_WIDTH	841
#define A0_MM_HEIGHT	1191
#define A0_DEFAULT_UNIT DVR_MM_UNITS

#define A1_MM_WIDTH	594
#define A1_MM_HEIGHT	841
#define A1_DEFAULT_UNIT DVR_MM_UNITS

#define A2_MM_WIDTH	420
#define A2_MM_HEIGHT	594
#define A2_DEFAULT_UNIT DVR_MM_UNITS

#define A3_MM_WIDTH	297
#define A3_MM_HEIGHT	420
#define A3_DEFAULT_UNIT DVR_MM_UNITS

#define A4_MM_WIDTH	210
#define A4_MM_HEIGHT	297
#define A4_DEFAULT_UNIT DVR_MM_UNITS


#define A_INCH_WIDTH	8.50
#define A_INCH_HEIGHT	11.00
#define A_DEFAULT_UNIT  DVR_INCH_UNITS

#define B_INCH_WIDTH	11.00
#define B_INCH_HEIGHT 	17.00
#define B_DEFAULT_UNIT  DVR_INCH_UNITS

#define C_INCH_WIDTH	17.00
#define C_INCH_HEIGHT	22.00
#define C_DEFAULT_UNIT  DVR_INCH_UNITS

#define D_INCH_WIDTH	22.00
#define D_INCH_HEIGHT	34.00
#define D_DEFAULT_UNIT  DVR_INCH_UNITS

#define E_INCH_WIDTH	34.00
#define E_INCH_HEIGHT	44.00
#define E_DEFAULT_UNIT  DVR_INCH_UNITS

#define LEGAL_INCH_WIDTH    8.50
#define LEGAL_INCH_HEIGHT   14.00
#define LEGAL_DEFAULT_UNIT  DVR_INCH_UNITS

#define LP_INCH_WIDTH	13.70
#define LP_INCH_HEIGHT	11.00
#define LP_DEFAULT_UNIT DVR_INCH_UNITS

#define VT_INCH_WIDTH	8.00
#define VT_INCH_HEIGHT	5.00
#define VT_DEFAULT_UNIT DVR_INCH_UNITS

#define EXEC_INCH_WIDTH	  7.50
#define EXEC_INCH_HEIGHT  10.50
#define EXEC_DEFAULT_UNIT DVR_INCH_UNITS

#define B4_MM_WIDTH	257
#define B4_MM_HEIGHT	364
#define B4_DEFAULT_UNIT DVR_MM_UNITS

#define B5_MM_WIDTH	182
#define B5_MM_HEIGHT	257
#define B5_DEFAULT_UNIT DVR_MM_UNITS

#define DVR_NUM_PAPER_SIZES 16


typedef struct
  {
    XmString  cstring;
    float	   pap_width;
    float	   pap_height;
    unsigned short default_unit;
  } paper_struct;

static paper_struct paper_list[DVR_NUM_PAPER_SIZES];


/*
 *  constants/structs for format/filter matching
 */

/*  number of formats we support filter/format name matching for; currently
 *  this includes all those on the base system, and all those in the converter
 *  library kit  + postscript (ps)
 */
#define NUM_FORMATS  15

typedef struct
  {
    char *filter;
    char *format;
  } format_filter_pair;

static format_filter_pair format_filter_table[NUM_FORMATS];


/*
 *  CONSTANTS TO IDENTIFY WIDGETS TO create_proc() [from UIL]
 */

#define	k_close_pb		 	1
#define	k_char_tb			2
#define	k_viewer			3
#define	k_diag_info_pb			4
#define	k_diag_info_st			5
#define	k_doc_info_pb			6
#define	k_doc_info_st			7
#define	k_format_lb			8
#define	k_format_st			9
#define	k_inch_tb			10
#define	k_menu			 	11
#define	k_mm_tb				12
#define	k_reformat_tb			13
#define	k_size_height_st		14
#define	k_size_lb			15
#define	k_size_width_st			16
#define	k_ps_comments			17
#define k_ps_bwidths			18
#define k_ps_fake_trays			19
#define k_ps_draw_mode			20
#define k_ps_scale			21
#define k_ps_op_dbox			22
#define k_pap_ok_but			23
#define	k_pap_can_but			24
#define k_ps_header_req			25
#define k_0_deg_tb			26
#define k_90_deg_tb			27
#define k_180_deg_tb			28
#define k_270_deg_tb			29

/* Define the buttons in the help pulldown menu */
#define k_help_context                  30
#define k_help_window                   31
#define k_help_help			32
#define k_help_version                  33

/* cbr stuff */

#define k_new_file_pb			103

static Boolean  cbr_mode = FALSE;
static CDAstatus (*cbr_new_file_cb)() = NULL;
static CDAstatus (*cbr_frg_ref_cb) () = NULL;
static CDAuserparam  	cbr_frg_ref_parm = NULL;
static CDAuint32   	cbr_ref_buttons = 0;
static CDAaddress	cbr_viewwgt_ptr = NULL;

/* end cbr stuff */

/* constants for orient buttons */
#define DEGREES_0 	0
#define DEGREES_90 	1
#define DEGREES_180	2
#define DEGREES_270	3


/*
 * Define Book file help system
 */

#ifdef __osf__
#define viewer_help "dxvdoc"
#else
#define viewer_help "ddif$view"
#endif


/*
 *  MISC STATICS
 */

static int	viewer_options;		    /* options for the viewer	      */
static int 	x_default_position  = 100;  /* default window X,Y screen pos  */
static int 	y_default_position  = 100;

static int	current_size_index    = 0;  /* index of selected paper size   */
static char	*doc_info_text_buffer = 0;  /* buffer for doc info 	      */
static int	save_size_index       = 0;
static int	diag_info_text_length = 0;
static char	*paper_height_str     = 0;
static char 	*paper_width_str      = 0;
static char	*save_height_str      = 0;
static char 	*save_width_str       = 0;

static Widget	toplevel;		    /* top level returned from xtinit */
static char	*diag_info_text_buffer;
static char 	*save_file_format_str;

static unsigned char  current_file   [256]; /* currently loaded ddif file     */
static unsigned char  current_format [256]; /* current format 		      */
static unsigned char  current_op_file[256]; /* current options file	      */
static unsigned char  current_dir_mask[256]; /* current dir mask for open box */
static unsigned long  current_paper_width;  /* current paper width            */
static unsigned long  current_paper_height; /* current paper height           */
static unsigned short file_loaded = 0;	    /* boolean to tell if a file is
					       currently loaded 	      */

static unsigned short current_size_unit = 0;
static unsigned short save_size_unit;

static Boolean  save_use_comments,
		save_use_bwidths,
		save_use_trays,
	  	save_draw_mode;

static Widget   save_orient_widget;
static int	save_scale;

/*  if the user has not changed their filter mask in the open file
 *  selection box, then we will change it for them if they click on
 *  a new format; If the user has changed their mask, then we do not
 *  change it when they select a new format in case they might have
 *  specified the filter before the format. This flag indicates if the
 *  user has changed their filter mask on their own.
 */

static Boolean  filter_change_flag = FALSE;
static Boolean  hyper_help_open    = FALSE;
/*
 *  define cursor var so we can put up watch while loading
 */
static Cursor WatchCursor = 0;

/*  initially a copyright message appears in the title bar;
 *  this stays up until a user action.
 */
static Boolean 	done_with_copyright = FALSE;

/*
 *  Boolean used to signal fatal error to message cb routine
 */
static int 	fatal_flag;

/*
 * The translation table needs to be accesible to format_text_create_proc.
 * Since I create it in the decw$viewr proc (maybe it shouldn't be there??
 * it needs to be global.
 */
static XtTranslations cr_trans_parsed;

static unsigned char current_op_file_dir[256]; /* directory for opt file      */
static unsigned char current_file_dir   [256]; /* directory for loaded file   */
static Boolean	diag_info_button_on = FALSE;   /* diag info button state      */


/*  this flag is used to determine whether or not to list Postscript
 *  as a valid format; If the display server has display postscript, then
 *  we permit PS viewing.
 */
static Boolean dps_exists;


/*  this flag specifies if a PS file was specified via
 *  the command line; if so, then we have to call DvrDocumentInfo()
 *  from the callback proc
 */
static Boolean ps_on_command_line = FALSE;

/*  Global help system context
 */

Opaque	help_context;

/*  Book file for help system
 */


/*
 *  non uil non integer static procs
 */
static void	stext_cr_proc  ();
static void 	set_sensitivity ();
static void	display_error ();
static Pixmap 	MakePixmap();
static void 	ScrBringToFront();
static void	open_drm_and_get_strings();
static void     strip_file_spec ();
int		main_condition_handler();
int		viewer_x_error_rtn();
static void	set_hyper_help_cbs ();


/*
 *  Action table associating the string SOMETHING with the routine
 *  stext_cr_proc()
 */
static XtActionsRec actions[] =
        {
            {"SOMETHING",       (XtActionProc)stext_cr_proc},
            NULL
         };

/*  these are the necessary widget id's
 *  to get from the .UID file; they are
 *  the widgets that the application is
 *  responsible for mapping/manipulating;
 */

static Widget 	main_window;		/* main window child of top level   */
static Widget 	viewer = 0;		/* viewer widget child of main win  */
static Widget  	help_widget = 0;	/* help widget			    */
static Widget 	file_sel_box = 0;	/* file selection widget	    */
static Widget 	viewer_message = 0;	/* message box			    */
static Widget	options_file_sel_box=0; /* options file selection  pop-up   */
static Widget   format_dialog_box;
static Widget   close_button_id;	/* close file push button ID        */
static Widget   main_menu_id;		/* main menu for application        */
static Widget   format_list_id;		/* id for format list widget	    */
static Widget   format_text_id;		/* id for format text widget	    */
static Widget 	diag_info_button;	/* id of diag info button	    */
static Widget	diag_info_dialog_box=0; /* id of diag info dialog box	    */
static Widget	diag_info_text_widget;	/* id of text widget for diag info  */
static Widget 	doc_info_button;	/* id of doc info button	    */
static Widget	doc_info_dialog_box=0;  /* id of doc info dialog box	    */
static Widget	doc_info_text_widget=0;	/* id of text widget for doc info   */
static Widget	paper_size_dialog_box = 0; /* dialog box for paper size options */
static Widget   reform_toggle_id = 0;
static Widget 	size_width_text_id = 0;
static Widget   size_height_text_id = 0;
static Widget 	size_list_id = 0;
static Widget   char_toggle_id;
static Widget   inch_toggle_id;
static Widget   mm_toggle_id;
static Widget	ps_comments_id;
static Widget	ps_bwidths_id;
static Widget	ps_fake_trays_id;
static Widget	ps_draw_mode_id;
static Widget	ps_popup_id = 0;
static Widget   ps_scale_id;
static Widget   orientGroup[4];
static Widget	viewer_caution_box = 0;

static Widget   ps_op_id;
static Widget   pap_ok_id;
static Widget   pap_can_id;

static Display	    *vw_dpy;
static XtAppContext vw_context;


/*
 *  DRM STATICS
 */

/*
 *  DRM database hierarch id
 */
static MrmHierarchy dvr_DRMHierarchy;

/*
 *  names of uid files for drm
 */
static char 	*db_filename_vec [1];

#ifdef __vms__

/*  in post-DECWINDOWS-V1, do not specify path name to DRM for
 *  language switching;
 */

#define DEFAULT_UID_FULL_NAME 	"DDIF$VIEW"
#define DEFAULT_UID_FULL_LEN	10 /* add one for \0 */

#else  /* ultrix */

/*  in post-DECWINDOWS-V1, do not specify path name to DRM for
 *  language switching;
 */

#define DEFAULT_UID_FULL_NAME	"dxvdoc.uid"
#define DEFAULT_UID_FULL_LEN    (sizeof DEFAULT_UID_FULL_NAME)

#define DEFAULT_UID_NAME	"/dxvdoc.uid"
#define DEFAULT_UID_LEN		(sizeof DEFAULT_UID_NAME)

#endif

/*
 *  dummy needed to call drm routines
 */
static MrmType	dummy_class;

/*  these are the callback routines
 *  specified in the .uil file;
 */
static void	about_proc();
static void	activate_proc();
static void 	close_proc ();
static void	create_proc ();
static void	caution_cb_proc ();
static void	diag_info_dismiss_proc();
static void	diag_info_proc();
static void	diag_info_update_proc();
static void	doc_info_dismiss_proc();
static void	doc_info_proc();
static void	exit_proc ();
static void	file_sel_proc  ();
static void	format_cancel_proc ();
static void	format_ok_proc ();
static void	format_list_click_proc ();
static void	format_list_confirm_proc ();
static void	help_proc ();
static void	message_proc ();
static void	new_file_proc ();
static void	options_file_activate_proc ();
static void 	options_file_sel_proc ();
static void	paper_size_cancel_proc ();
static void	paper_size_ok_proc ();
static void	paper_size_proc ();
static void	size_list_click_proc ();
static void	unit_toggle_change_proc();
static void	on_help_proc ();
static void	help_system_proc ();
static void	help_activate_proc ();


/*  The names and addresses of callback routines.  Note: we keep the
 *  names in lexicographic order so that the resource manager can do
 *  a quick lookup at initialization time.
 *  Remember to change regnum if you change regvec
 */
static MrmCount		regnum = 24;
static MrmRegisterArg  	regvec[] = {
		{"activate_proc", 		(caddr_t) activate_proc},
		{"close_proc", 			(caddr_t) close_proc},
                {"create_proc", 		(caddr_t) create_proc},
                {"caution_cb_proc", 		(caddr_t) caution_cb_proc},
                {"diag_info_dismiss_proc", 	(caddr_t) diag_info_dismiss_proc},
                {"diag_info_proc", 		(caddr_t) diag_info_proc},
                {"doc_info_dismiss_proc", 	(caddr_t) doc_info_dismiss_proc},
                {"doc_info_proc", 		(caddr_t) doc_info_proc},
    		{"exit_proc", 			(caddr_t) exit_proc},
		{"file_sel_proc", 		(caddr_t) file_sel_proc},
		{"format_list_click_proc", 	(caddr_t) format_list_click_proc},
		{"format_list_confirm_proc", 	(caddr_t) format_list_confirm_proc},
    		{"help_proc", 			(caddr_t) help_proc},
    		{"message_proc", 		(caddr_t) message_proc},
    		{"new_file_proc", 		(caddr_t) new_file_proc},
		{"help_activate_proc",		(caddr_t) help_activate_proc},
		{"help_system_proc",		(caddr_t) help_system_proc},
    		{"options_file_activate_proc", 	(caddr_t) options_file_activate_proc},
		{"options_file_sel_proc", 	(caddr_t) options_file_sel_proc},
    		{"paper_size_cancel_proc", 	(caddr_t) paper_size_cancel_proc},
    		{"paper_size_ok_proc", 		(caddr_t) paper_size_ok_proc},
    		{"paper_size_proc", 		(caddr_t) paper_size_proc},
		{"size_list_click_proc", 	(caddr_t) size_list_click_proc},
                {"unit_toggle_change_proc", 	(caddr_t) unit_toggle_change_proc}
    		};

/*
 *  callback lists which include DRM procs
 */

static XtCallbackRec mess_cb[] =
		{ {message_proc,  NULL},
		  {NULL,	  NULL} };
/*
 *  callback lists for file selction widget context sensitive help
 */

static XtCallbackRec hyper_help_cb[] =
		{ {help_system_proc,  NULL},
		  {NULL,	  NULL} };



/*
 *  CONSTANT STRINGS FETCHED FROM UIL
 */
static char 	*dvr_copyright_str;
static char 	*dvr_colon_str;
static char 	*dvr_title_str = 0;
static char 	*dvr_ddif_file_ext;
static char 	*dvr_default_format_str;
static char 	*dvr_options_file_ext;
static char	*dvr_cursor_font_file;
static char	*dvr_cursor_fg_color;
static char	*dvr_cursor_bg_color;
static char 	*dvr_cda_wildcard;
static char 	*dvr_ddif_wildcard;
static char 	*dvr_dtif_wildcard;
static char 	*dvr_dtif_string;
static char 	*dvr_ddif_string;
static char	*dvr_on_window_label;
static char 	*dvr_on_version_label;
static char	*dvr_ps_nohead_str;

#ifdef __vms__
static char	*dvr_message_title;

#endif

#ifdef __unix__
static char 	*dvr_stdin_str;
static char 	*dvr_ult_help_library;
#endif


/*
**++
**  ROUTINE NAME:
**	Dvr__DECW_Application(file_name_str, format_str, op_file_str,
**		    select_options)
**
**  FUNCTIONAL DESCRIPTION:
**      This is the main routine for the ddif viewer application;
**	It starts up the toolkit and DRM, fetches the main widget
**	and processes events.
**
**  FORMAL PARAMETERS:
**    	char	*file_name_str;
**    	char	*format_str;
**    	char	*op_file_str;
**    	int	*select_options;
**	int	argc;
**	char    **argv;
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

Dvr__DECW_Application ( file_name_str, format_str, op_file_str,
			select_options, dvr_argc, dvr_argv,
			paper_height, paper_width)
    char	*file_name_str;
    char	*format_str;
    char	*op_file_str;
    int		*select_options;
    int  	dvr_argc;
    char 	**dvr_argv;
    unsigned long paper_height;
    unsigned long paper_width;
{

    int 	i;
    int		stat;
    Arg 	arglist[10];
    int		argcount;
    Pixmap 	icon_pixmap;
    Pixmap 	sm_icon_pixmap;
    float	dpi_float;
    int		dpi_int;
    int		default_width, default_height;
    int		opcode, event, error;
    WidgetList	child_widgets;
    Widget	menu_bar;
    Widget	help_menu;
    String	help_pd_str;

/*
 * ASCII translation table associating the routine identified by the string
 * SOMETHING with the event of a press of the return key.
 */
    String cr_trans_table =
            "<KeyPress>0xff0d:  SOMETHING()";

    /* set statics passed in from command line */
    viewer_options  = *select_options;
    current_paper_width  = paper_width;
    current_paper_height = paper_height;
    current_file_dir[0] = '\0';

    /* initialize conversion ratio literals */
    MM_PER_CHAR_W   = (float) INCH_PER_CHAR_W * (float) MM_PER_INCH;
    MM_PER_CHAR_H   = (float) INCH_PER_CHAR_H * (float) MM_PER_INCH;

    MrmInitialize ();	/* initialize Motif */

    DXmInitialize ();	/* initialize the DEC extensions */

    /*  register the class for our "user defined" viewer widget so
     *  DRM/UIL can understand it.
     */

    if (!(stat = DvrRegisterClass()))
      {
	fprintf(stderr, dvr_drm_reg_fail_str);

#ifdef __vms__
	exit(DVR_DRMREGISTERFAIL);
#else
	exit(1);
#endif
      }

#ifdef __vms__
   /*  set the arg count to 0, since VMS XtInitiialize will
    *  not parse parameters.
    */

    dvr_argc = 0;
#endif

   /* set X error handler */
   XSetErrorHandler(viewer_x_error_rtn);

   /*  initialize the toolkit; among other things,
    *  this opens the display, and parses the command line
    *  for any parameters of interest to X.
    */
    XtToolkitInitialize();
    vw_context = XtCreateApplicationContext();
    if (vw_context == 0)
	{
	fprintf(stderr, dvr_drm_ctx_fail_str);
#ifdef __vms__
	exit(DVR_DRMCTXFAIL);
#else
	exit(1);
#endif
	}

    vw_dpy = XtOpenDisplay(
	/* context */			vw_context,
	/* display name: use set
	   dispay or -d argument */     NULL,
	/* Application name */		dvr_xdefaults_name_str,
	/* Application Class */		dvr_class_str,
	/* Options */			NULL, 0,
	/* Command line parameters */	&dvr_argc, dvr_argv
	);

    /*
    * If the display connection could not be opened, issue an error
    * message and immediately exit.  As of this writing, XtAppErrorMsg
    * (which handles fatal errors) either hangs or stack dumps, so for
    * now instead explicitly write the error message to stderr and exit,
    * which is how it's done in the "X Window System Toolkit" book anyway.
    */

    if (vw_dpy == 0)
      {
#ifdef __vms__
	exit(DVR_DRMDPYFAIL);
#else
	fprintf(stderr, dvr_drm_dpy_fail_str);
	exit(1);
#endif
      }

    toplevel = XtAppCreateShell(
	/* app name, use same as dpy */	dvr_xdefaults_name_str,
	/* application class name */	dvr_class_str,
	/* application class */		applicationShellWidgetClass,
	/* display */			vw_dpy,
	/* options */			NULL, 0);

#ifdef DVR_PSVIEW
    /*  find out if server has display postscript extension;
     *  allow MIT registered string or previously used DECwindows
     *  string
     */

    dps_exists = dvr_app_check_for_xdps(toplevel);
#else
    dps_exists = FALSE;
#endif

#ifndef __vms__

    /* subtract one from the argument count; for the program name */
    dvr_argc = dvr_argc -1;

    /*  for ultrix, argc and argv are passed to XtInitialize so
     *  X can parse them for any relavent parameters; if there are
     *  parameters left, use the last one for the file name; if not
     *  start up viewer with no file name;
     */

    if (dvr_argc == 0)
        file_name_str[0] = '\0';
    else
	strcpy(file_name_str, dvr_argv[dvr_argc]);

#endif

/*
 *  Initialize the binary translation table which is used by
 *  format_text_create_proc and put our action table into the general
 *  action table so the toolkit can see it.
 */
    cr_trans_parsed = XtParseTranslationTable(cr_trans_table);

    XtAppAddActions(vw_context,
		    actions,
		    2);

#ifdef DEBUG
    XSynchronize(XtDisplay(toplevel), 1);
#endif

    (void) open_drm_and_get_strings();

    /* remember directory of op file if specified */
    strcpy((char *) current_op_file, (char *) op_file_str);
    current_op_file_dir[0] =  '\0';

    dir_file_scan(current_op_file, current_op_file_dir, dvr_options_file_ext);

    /*  if a file name has been sent in, use it as
     *  the opening file spec;
     */

    if (strlen(file_name_str) != 0)
      {

        dir_file_scan(file_name_str, current_file_dir, dvr_ddif_file_ext);

#ifdef __unix__
	/* if this is ultrix, use "-" to mean stdin */
	if ( (strcmp(file_name_str, "-") == 0) &&
	     (strcmp(format_str, (char *) POSTSCRIPT_STR) != 0) ) /* ps widget does not handle stdin */
	   strcpy((char *) current_file, dvr_stdin_str);
	else
#endif
           strcpy((char *) current_file, (char *) file_name_str);
      }
    else
      {
	current_file[0] = '\0';
#ifdef __unix__
	/* for ultrix, default to *.ps if -f ps is specified */
	if (strcmp((char *) format_str, (char *) POSTSCRIPT_STR) == 0)
	  {
	    strcpy((char *) current_file_dir, "*.");
	    strcat((char *) current_file_dir, (char *) POSTSCRIPT_STR);
	  }
#endif
      }

    if(strlen(format_str) != 0)
      {
#ifdef __vms__
	upper_case(format_str);
#endif
        strcpy((char *) current_format, (char *) format_str);
      }
    else
	strcpy((char *) current_format, (char *) dvr_ddif_string);


    change_title();  		/* change title bar			*/

    /* fetch the main window as a child of the top level */

    if (MrmFetchWidget (
		dvr_DRMHierarchy,
		"dvr_main",
		toplevel,
		& main_window,
		& dummy_class) != MrmSUCCESS)
      {
	fprintf(stderr, dvr_drm_main_ffail_str);

#ifdef __vms__
	exit(DVR_DRMMAINFETCHFAIL);
#else
	exit(1);
#endif
      }


    if ( (toplevel->core.width == 0) &&
	 (toplevel->core.height == 0) )
      {

        /* if no width and height have been specified via Xdefaults,
         * try to make the viewer 8 1/2 by 9 inches so we can display writer
         * files reasonably
         */
	int max_width, max_height;

    	/* compute the x dpi of the screen; first get the screen width in mm */
    	dpi_float = (float)
		XWidthMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(toplevel)));

    	/* now convert to inches */
    	dpi_float = dpi_float / 25.4;

    	/* now divide by number of pixels by inches to get the DPI */
    	dpi_float = (float)
		XWidthOfScreen(XDefaultScreenOfDisplay(XtDisplay(toplevel))) /
		dpi_float;

    	/* round off and store as an int */
    	dpi_int = (int) (dpi_float + 0.5) ;

    	/* get and set default sizes */
    	default_width = dpi_int * 8.5  + 	/* 8 1/2 inches worth of dots 	*/
		    		17 +		/* 17 dots for scroll bar     	*/
		    		4;		/* 4 for borders	      	*/

	max_width = XWidthOfScreen(XDefaultScreenOfDisplay(XtDisplay(toplevel)))
		    - x_default_position;

	/* dont let default size go off screen; if screen can't hold 8 1/2
         * inches, use maximum that it will hold
	 */
        if (default_width > max_width)
	    default_width = max_width;

    	/* compute the y dpi of the screen; first get the screen height in mm */
    	dpi_float = (float)
		XHeightMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(toplevel)));

    	/* now convert to inches */
    	dpi_float = dpi_float / 25.4;

    	/* now divide by number of pixels by inches to get the DPI */
    	dpi_float = (float)
		XHeightOfScreen(XDefaultScreenOfDisplay(XtDisplay(toplevel))) /
		dpi_float;

    	/* round off and store as an int */
    	dpi_int = (int) (dpi_float + 0.5) ;

    	default_height = dpi_int * 9 +		/* 9 inches worth of dots	*/
		     		17 +		/* 17 dots for scroll bar	*/
		     		4 +		/* 4 for borders		*/
				20;		/* 20 for menu bar */

	max_height = XHeightOfScreen(XDefaultScreenOfDisplay(XtDisplay(toplevel)))
		    - y_default_position;

	/* dont let default size go off screen; if screen can't hold 9
         * inches, use maximum that it will hold
	 */
        if (default_height > max_height)
	    default_height = max_height;

    	argcount = 0;
    	XtSetArg(arglist[argcount], XmNwidth, default_width);
    	argcount++;
    	XtSetArg(arglist[argcount], XmNheight, default_height);
    	argcount++;
    	XtSetValues(main_window, arglist, argcount);

	/* center application */
	x_default_position = (int) (x_default_position / 2);
	y_default_position = (int) (y_default_position / 2);

      }

    /*
     *  if an x or y position has been specified via an Xdefaults file,
     *  remember it here;
     */
    if (toplevel->core.x != 0)
	x_default_position = toplevel->core.x;
    if (toplevel->core.y != 0)
	y_default_position = toplevel->core.y;

    /* set up the icons for the viewer */

#ifdef old
    icon_pixmap = MakePixmap(XtDisplay(toplevel),
		XDefaultRootWindow(XtDisplay(toplevel)),
		viewer_bits, viewer_width, viewer_height);

    sm_icon_pixmap = MakePixmap(XtDisplay(toplevel),
		XDefaultRootWindow(XtDisplay(toplevel)),
		sm_viewer_bits, sm_viewer_width, sm_viewer_height);

#endif

/*
 *   leave in XUI icons for support running to DW V2 workstations
 */

    argcount = 0;

#ifdef old
    XtSetArg(arglist[argcount], XmNiconPixmap, icon_pixmap);
    argcount++;

    XtSetArg(arglist[argcount], "iconifyPixmap", sm_icon_pixmap);
    argcount++;
#endif


    XtSetArg(arglist[argcount], XmNx, x_default_position);
    argcount++;
    XtSetArg(arglist[argcount], XmNy, y_default_position);
    argcount++;
    XtSetValues(toplevel, arglist, argcount);

    /*  Make "main_window" and all it's children
     *  appear on the screen, ready to interact with.
     *  By managing the viewer before we call DvrViewerFile,
     *  we avoid an extra expose.
     */

    XtManageChild (main_window);

    /*
    ** If there is Hyper help, set up helpcallbacks on viewer widget children
    ** for context sensitive help
    */
#ifdef CDA_HYPER_HELP
       set_viewer_widget_callbacks(viewer);
#endif
#ifdef __unix__ 	
    /*MOTIF 1.2 TS*/
    if (dps_exists)
	XDPSSetEventDelivery (vw_dpy, dps_event_pass_through);
#endif
    /* try to open file if specified */
    if (strlen(file_name_str) != 0)
      {
        /*  due to a restriction in PS widget, we have to realize
         *  our widget before setting a file in the PS widget, this should
         *  be fixed in the PS widget, and removed from here
         */

    	if ( dps_exists &&
	    (strcmp((char *) current_format, (char *) POSTSCRIPT_STR) == 0) )
    	    ps_on_command_line = TRUE;


#ifdef __unix__
	/* if this is ultrix, allow "-" for stdin. pass an empty
	 * string to cda, and it assumes stdin.
	 */
	if ( (strcmp(file_name_str, "-") == 0) &&
	     (strcmp(format_str, (char *) POSTSCRIPT_STR) != 0) ) /* ps widget does not handle stdin */
	    file_name_str[0] = '\0';
#endif
	/* save current format */
	strcpy((char *) current_format, (char *) format_str);

	viewer_new_file(file_name_str, format_str, op_file_str);
      }

#ifdef __unix__
    else
      {
	/*  if this is ultrix, check to see if stdin is not empty
	 *  if it isn't, use stdin for the file.
	 */
	int num_chars_waiting = 0;
	int stat;

	stat =  ioctl(0, FIONREAD, &num_chars_waiting);
	if ( (num_chars_waiting != 0) &&
	     (stat == 0) )
	  {
	    /* read the data from stdin, put "stdin" in the title bar */

	    /* save current format */
	    strcpy((char *) current_format, format_str);

	    viewer_new_file(file_name_str, format_str, op_file_str);
	    strcpy((char *) current_file, dvr_stdin_str);
	    change_title();
	  }
      }
#endif

    /*  Add an event handler to track Reparent notify events.
     */
    XtAddEventHandler( toplevel, StructureNotifyMask, False,
    	    	       set_icons_on_shell, None );

    if (!XtIsRealized(toplevel))
	XtRealizeWidget(toplevel);

    /*
    ** Currently main_window controls dvr_menu_bar, so get the widget Id of
    ** dvr_menu_bar so that, help_pd menu can be fetched
    */

    child_widgets = DXmChildren ((CompositeWidget )main_window);
    menu_bar = child_widgets[4];

#ifdef CDA_HYPER_HELP
       help_pd_str = "hyper_help_pd";
#else
       help_pd_str = "help_pd";
#endif
    if (MrmFetchWidget (
		dvr_DRMHierarchy,
		help_pd_str,
		menu_bar,
		& help_menu,
		& dummy_class) != MrmSUCCESS)
      {
	fprintf(stderr, dvr_drm_main_ffail_str);

#ifdef __vms__
	exit(DVR_DRMMAINFETCHFAIL);
#else
	exit(1);
#endif
      }

    argcount = 0;
    XtSetArg(arglist[argcount], XmNmenuHelpWidget, help_menu);
    argcount++;
    XtSetValues(menu_bar, arglist, argcount);

    if (!XtIsRealized(help_menu))
	XtRealizeWidget(help_menu);

    XtManageChild(help_menu);

    /*  Set the icon pixmap after we have realized the shell.
     */
    set_icons( toplevel, dvr_DRMHierarchy );

    /* if a pop-up has already been generated, manage it */
    if (viewer_message != 0)
	XtManageChild(viewer_message);

    else if (strlen((char *) current_file) == 0)
      {
	/*  put up file selection box on startup if no file
	 *  specified
	 */
	new_file_proc(0, 0, 0);
      }

    else if (viewer_caution_box != 0)
        /*  caution box is created if user tries to view a ps
	 *  file wtihout standard header (%!)
	 */
	XtManageChild(viewer_caution_box);

    /*  loop in our own main loop which is wrapped with a
     *  condition handler to catch ISL errors
     */
    while (1) MainLoop();    /* loop til we puke */

#ifdef __vms__
    exit(DVR_NORMAL);
#endif

}



/*
**++
**  ROUTINE NAME:
**	MainLoop()
**
**  FUNCTIONAL DESCRIPTION:
** 	establish condition handler and then loop getting x events
**	forever; if a condition is signaled (probably through ISL,
**	the condition is caught, and the loop is re-entered from
**	Dvr__DECW_Application()
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

void MainLoop()

{
    XEvent event;

    /* establish condition handler to catch ISL signals */
#ifdef __vms__
    LIB$ESTABLISH( main_condition_handler );
#endif

    if (done_with_copyright)
	/*  will only get here in case this is not our first time
	 *  into MainLoop; this will only happen in the case where
	 *  our global signal handler was called for some reason
	 */

#ifdef __unix__
	/*MOTIF 1.2 TS*/
	/* The following is used in lieu of XtAppMainLoop so that
	 * DPS events will be processed before getting into the
	 * Xt event processing routine.  Otherwise, the DPS event
	 * may be "lost".
	 */

	for (;;) {
	    XtAppNextEvent(vw_context, &event);
	    if(!(dps_exists && XDPSDispatchEvent(&event)))
		XtDispatchEvent(&event);
	}
#else
   	XtAppMainLoop(vw_context);
#endif


    /*  note: XtAppMainLoop() is not used initially so we can put up
     *  our copyright notice and take it down upon the first event;
     *  all XtAppMainLoop() is is a loop getting and dispatching events.
     *  after the copyright is down, go into XtAppMainLoop() (note, program
     *  exits in exit_proc()
     */

    /* get events forever.  We get out by the exit button */
    while (1)
      {
	XEvent event;

    	XtAppNextEvent (vw_context, &event);

	/* leave up copyright until user does anything */

#ifdef __unix__
	/*MOTIF 1.2 TS*/
    	if ((!done_with_copyright) && ((event.type == ButtonPress) ||
	    (event.type == KeyPress)))
	  {
	     done_with_copyright = TRUE;
	     change_title ();
    	     if (!(dps_exists && XDPSDispatchEvent(&event)))
		 XtDispatchEvent (&event);

	     /* Another in-lieu-of XtAppMainLoop (see above) */

	     for (;;) {
		 XtAppNextEvent(vw_context, &event);
		 if(!(dps_exists && XDPSDispatchEvent(&event)))
		     XtDispatchEvent(&event);
	     }

	  }
	else
          if (!(dps_exists && XDPSDispatchEvent(&event)))
	      XtDispatchEvent (&event);

#else
    	if ((!done_with_copyright) && ((event.type == ButtonPress) ||
	    (event.type == KeyPress)))
	  {
	     done_with_copyright = TRUE;
	     change_title ();
	     XtDispatchEvent (&event);

   	     /*  now we can go into mail loop without checking for
   	      *  copyright flag
   	      */
   	     XtAppMainLoop(vw_context);
	  }
	else
   	  XtDispatchEvent (&event);
#endif

      }

}


/*
**++
**  ROUTINE NAME:
**	viewer_x_error_rtn()
**
**  FUNCTIONAL DESCRIPTION:
** 	process errors from X server:
**	we cannot generate an X protocal  request (any message from the
**	client to the server that would modify the display) from an X
**	error handler, so we have to blast the messages to stderr.
**	(XGetErrorDatabaseText() code based on decw calendar code.)
**
**  FORMAL PARAMETERS:
**    Display 		*dpy;
**    XErrorEvent 	*error_event;
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
**	prints message to stderr
**--
**/

viewer_x_error_rtn (dpy, error_event)
    Display 	*dpy;
    XErrorEvent *error_event;

{
    char 		cda_error_str[512];
    char		xlib_error_str[512];
    char		number_str[20];
    char 		*mtype = "XlibMessage";

    /*  first generate prefix, cda string should have already been fetched
     *  from UID file
     */
    if (dvr_title_str != 0)
      {
        strcpy(cda_error_str, dvr_title_str);
	strcat(cda_error_str, ": ");
      }
    else
      /* default in case something went wrong getting strings out of uid */
      strcpy(cda_error_str, "CDA Viewer : ");

    /* now append default error string and carriage return and print */
    XGetErrorDatabaseText (dpy, mtype, "XError", "X Error from server",
			   xlib_error_str, 512);
    strcat(cda_error_str, xlib_error_str);
    strcat(cda_error_str, "\n  ");
    fprintf(stderr, cda_error_str);

    /* fetch and print out X error for this code */
    XGetErrorText(dpy, error_event->error_code,
		xlib_error_str, 512);
    strcat(xlib_error_str, "\n  ");
    fprintf(stderr, xlib_error_str);

    /* fetch and print out major code info */
    XGetErrorDatabaseText (dpy, mtype, "MajorCode", "Request Major code %d",
			   xlib_error_str, 512);
    fprintf (stderr, xlib_error_str, error_event->request_code);
    sprintf (number_str, "%d", error_event->request_code);
    XGetErrorDatabaseText (dpy, "XRequest", number_str, "",
			   xlib_error_str, 512);
    fprintf (stderr, " (%s)", xlib_error_str);
    fputs ("\n  ", stderr);

    /* fetch and print out minor code info */
    XGetErrorDatabaseText (dpy, mtype, "MinorCode", "Request Minor code",
			   xlib_error_str, 512);
    fprintf (stderr, xlib_error_str, error_event->minor_code);
    fputs ("\n  ", stderr);

    /* fetch and print resource id info */
    XGetErrorDatabaseText (dpy, mtype, "ResourceID", "ResourceID 0x%x",
			   xlib_error_str, 512);
    fprintf (stderr, xlib_error_str, error_event->resourceid);
    fputs ("\n  ", stderr);

    /* fetch and print error serial info */
    XGetErrorDatabaseText (dpy, mtype, "ErrorSerial", "Error Serial #%d",
			   xlib_error_str, 512);
    fprintf (stderr, xlib_error_str, error_event->serial);
    fputs ("\n  ", stderr);

    /* fetch and print current serial info */
    XGetErrorDatabaseText (dpy, mtype, "CurrentSerial", "Current Serial #%d",
			   xlib_error_str, 512);
    fprintf (stderr, xlib_error_str, NextRequest(dpy) - 1);
    fputs ("\n", stderr);

    return (DVR_NORMAL);
}


/*
**++
**  ROUTINE NAME:
**	open_drm_and_get_strings();
**
**  FUNCTIONAL DESCRIPTION:
** 	this routine opens the DRM hierarchy, registers all routines,
**	and fetches misc. strings; if there is a problem, a message is
**	printed, and the program exits;
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
**	exits on problem;
**--
**/

static void open_drm_and_get_strings()

{
    int stat;

#ifdef __unix__

    /*  on ultrix, check for UIDDIR enviornment var;
     *  if set, use to look for UID file; else look in
     *  default area
     */

    char *uiddir;

    if ((uiddir= (char *) getenv("UIDDIR")) != NULL)
      {
	/* UIDDIR is defined, look here for UID file */
	db_filename_vec[0] = XtMalloc( DEFAULT_UID_LEN +
				       strlen(uiddir)  );
	strcpy(db_filename_vec[0], uiddir);
	strcat(db_filename_vec[0], DEFAULT_UID_NAME);

      }

    else
        /* UIDDIR not defined, use the default */
#endif

      {
	db_filename_vec[0] = XtMalloc(DEFAULT_UID_FULL_LEN);
	strcpy(db_filename_vec[0], DEFAULT_UID_FULL_NAME);
      }

    /* Define the DRM "hierarchy" (flat for FT1) */

    if (MrmOpenHierarchy (1,			    /* number of files	    */
			db_filename_vec, 	    /* files     	    */
			NULL,			    /* os_ext_list (null)   */
			&dvr_DRMHierarchy)	    /* ptr to returned id   */
			!= MrmSUCCESS)
      {
	fprintf(stderr, dvr_drm_open_hier_str);

#ifdef __vms__
	exit(DVR_DRMHIERARCHYFAIL);
#else
	exit(1);
#endif

      }

    /*  Register our callback routines so that the resource manager can
     *  resolve them at widget-creation time.
     */

    MrmRegisterNames(regvec, regnum);	/* they are ordered */

    /* get strings from uil */
    stat = get_uil_strings();
    if (stat != DVR_NORMAL)
      {
	fprintf(stderr, dvr_drm_str_ffail_str);

#ifdef __vms__
	exit(DVR_DRMSTRINGFETCHFAIL);
#else
	exit(1);
#endif
      }

}


/*
 *  the following icon routines were provided by the DECwindows
 *  build folks to help all of the OOTB's switch to Motif icons
 */

/*
 *  Callback routine which sets the icon pixmaps for Reparenting
 *  window managers.
 */
static void
set_icons_on_shell( shell, user_data, event )
    Widget   	    shell;
    caddr_t 	    user_data;	/* unused */
    XEvent   	    *event;
{
    XIconSize       *size_list;
    int	    	    num_sizes;
    Display 	    *dpy = XtDisplay( shell );
    Window  	    root_window = XDefaultRootWindow( dpy );
    XReparentEvent  *reparent = (XReparentEvent *) &event->xreparent;

    if ( event->type != ReparentNotify )
    	return;

    /* Ignore reparents back to the root window.
     */
    if ( reparent->parent == root_window )
    	return;

    /*  Set the icons for this shell.
     */
    if ( ! XGetIconSizes( dpy, root_window, &size_list, &num_sizes ) )
        return;
    else
    {
    	XFree( (char *) size_list );
    	set_icons( shell, dvr_DRMHierarchy );
    }

}  /* end of set_icons_on_shell */


/*
 *  Sets the icon and iconify pixmaps for the given shell widget.
 */
void
set_icons( shell, hierarchy_id )
    Widget  	    shell;
    MrmHierarchy    hierarchy_id;
{
    Display 	    	*dpy = XtDisplay( shell );
    Screen  	    	*scr = XtScreen( shell );
    unsigned int    	icon_size;
    char	    	*icon_name;
    static char     	*shell_icon_sizes[] = { "75", "50", "32", "17" };
    static int	    	num_sizes = XtNumber( shell_icon_sizes );
    static unsigned int	current_icon_size = 0;
    static Pixmap   	icon_pixmap = 0;
    static Pixmap   	iconify_pixmap = 0;

    /* Determine the icon pixmap name and size to fetch.
     */
    icon_name = get_icon_index_name( dpy, "ICON_PIXMAP", &icon_size,
    	    	    	    	     shell_icon_sizes, num_sizes );

    if ( icon_name == NULL)
      {
	/* default on xui systems */
	icon_size = 32;
    	icon_name = (char *) XtMalloc(18);
    	strcpy( icon_name, "ICON_PIXMAP_32X32" );
      }

    if ( icon_name != NULL )
    {
    	/*  If the icon sizes are different we need to free the current
    	 *  ones, and re-fetch new icons.  We assume that re-fetching
    	 *  new icons is an infrequent operation, so we don't cache the
    	 *  old icons.
    	 */
    	if ( ( current_icon_size != 0 )	    	    	/* Icon exists.     */
    	     && ( current_icon_size != icon_size ) )	/* New icon needed. */
    	{
    	    if ( icon_pixmap )
    	    	XFreePixmap( dpy, icon_pixmap );
    	    if ( ( iconify_pixmap ) && ( iconify_pixmap != icon_pixmap ) )
    	    	XFreePixmap( dpy, iconify_pixmap );
    	    icon_pixmap = 0;
    	    iconify_pixmap = 0;
    	    current_icon_size = 0;
    	}
    	if ( current_icon_size == 0 )
    	{
    	    current_icon_size = icon_size;
    	    icon_pixmap = fetch_icon_literal( hierarchy_id, dpy, scr, icon_name );
    	}
    	XtFree( icon_name );
    	icon_name = NULL;
    }

    else    /* Can't get icon sizes for some reason */
    	return;

    /* Fetch the iconify pixmap for compatibility with the XUI window manager.
     */

#if 1
    /*
     * Note that we're using the private DX routine which was not shipped as
     * part of the DECW$DXMLIBSHR sharable image since it was added so late
     * in the development cycle.
     *
     * The code that follows in the else should be deleted once it is decided that
     * we won't be returning to the old Xm routine.
    */

    if ( ! DXIsXUIWMRunning( shell, 0 ) )
#else
    /*
     * Old code.  Kept in case we need to go back to it at some future date.
    */

    if ( ! XmIsMotifWMRunning( shell ) )
#endif
    {
    	if ( icon_size == 17 )  	    /* Don't fetch icon twice */
    	    iconify_pixmap = icon_pixmap;
    	else if ( icon_size > 17 )
    	    iconify_pixmap = fetch_icon_literal( hierarchy_id, dpy, scr,
    	    	    	     "ICON_PIXMAP_17X17" );
    }

    /* Set the icon pixmap on the shell.
     */
    if ( icon_pixmap )
    {
    	if ( XtWindow(shell) != 0 )
    	{
    	    /* HACK: Under Motif 1.1 changing iconPixmap will cause the window
    	    *  	 to go to its intial state.  This appears to be a side-effect
    	    *  	 of ICCCM-compliant behavior, and doing XtSetValues in the
    	    *  	 X toolkit, so we need to call Xlib directly instead of
    	    *  	 setting XtNiconPixmap.
    	    */
    	    XWMHints    *wmhints = NULL;

    	    wmhints = XGetWMHints( dpy, XtWindow( shell ) );
    	    if ( wmhints != NULL )
    	    {
	    	wmhints->flags &= ~StateHint;
    	    	wmhints->flags |= IconPixmapHint;
    	    	wmhints->icon_pixmap = icon_pixmap;
    	    	XSetWMHints( dpy, XtWindow( shell ), wmhints );
    	    	XFree( (char *) wmhints );
    	    }
    	    else
    	    {
	    	wmhints = (XWMHints *)XtCalloc(1, sizeof(XWMHints));
	    	wmhints->flags &= ~StateHint;
    	    	wmhints->flags |= IconPixmapHint;
    	    	wmhints->icon_pixmap = icon_pixmap;
    	    	XSetWMHints( dpy, XtWindow( shell ), wmhints );
	    	XtFree( (char *) wmhints );
    	    }
    	}
    	else
    	{
    	    Arg	arglist[1];
    	    XtSetArg(arglist[0], XmNiconPixmap, icon_pixmap);
    	    XtSetValues(shell, arglist, 1);
    	}
    }

    /* Set the iconify pixmap for the XUI window manager
     */
    if ( iconify_pixmap )
    	set_iconify_pixmap( shell, iconify_pixmap );

}  /* end of set_icons */


/*
 *  Fetches a bitmap
 *  to be used as an icon for the Motif window manager.
 */
Pixmap
fetch_icon_literal( hierarchy_id, dpy, scr, index_string )
    MrmHierarchy    hierarchy_id;
    Display 	    *dpy;
    Screen  	    *scr;
    String  	    index_string;
{
    CDAstatus  	    status;
    Pixmap  	    tmp_pixmap;
    Dimension	    bm_width, bm_height;

    status = MrmFetchBitmapLiteral( hierarchy_id, index_string, scr, dpy,
    	    	    	    	  &tmp_pixmap, &bm_width, &bm_height );

    if ( status == MrmSUCCESS )
	return  tmp_pixmap;
    else
    	return  (Pixmap) 0;


}   /* end of fetch_icon_literal */


/*
 *  Finds the largest icon supported by the window manager and returns
 *  a string which represents that icon in UIL.
 */
static char *
get_icon_index_name( dpy, root_index_name, icon_size_rtn,
    	    	supported_icon_sizes, num_supported_sizes )
    Display 	    *dpy;
    char    	    *root_index_name;
    unsigned int    *icon_size_rtn;
    char    	    **supported_icon_sizes;
    int	    	    num_supported_sizes;
{
    XIconSize	*size_list;
    int	    	num_sizes;
    int	    	cursize;
    int	    	i;
    char    	*icon_index = NULL;
    int	    	icon_size;
    char    	*icon_size_ptr;
    Boolean 	found_icon_size = False;

    *icon_size_rtn = 0;	    /* Initial value */

    if ( ! XGetIconSizes( dpy, XDefaultRootWindow( dpy ),
    	    	    	  &size_list, &num_sizes ) )
    	return ( NULL );

    /* Find the largest icon supported by the window manager.
     */
    cursize = 0;
    for ( i = 1; i < num_sizes; i++ )
    {
    	if ( ( size_list[i].max_width >= size_list[cursize].max_width )
    	      && ( size_list[i].max_height >= size_list[cursize].max_height ) )
    	    cursize = i;
    }
    if ( ( size_list[cursize].max_width <= 0 )
    	 || ( size_list[cursize].max_height <= 0 ) )
    {
    	XFree( (char *) size_list );
    	return ( NULL );
    }

    /* Find the largest supported icon.
     */
    for ( i = 0; i < num_supported_sizes; i++ )
    {
    	icon_size = atoi( supported_icon_sizes[i] );
    	if ( ( icon_size <= size_list[cursize].max_width )
    	      && ( icon_size <= size_list[cursize].max_height ) )
    	{
    	    icon_size_ptr = supported_icon_sizes[i];
    	    found_icon_size = TRUE;
    	    break;
    	}
    }
    XFree( (char *) size_list );

    /*  Build the icon index name
     *
     *     format: root_index_name + "_" + icon_size_ptr + "X" + icon_size_ptr
     */
    if ( found_icon_size )
    {
    	icon_index = (char *) XtMalloc( strlen( root_index_name ) 	+
    	    	    	    	    	sizeof( "_" )	    	    	+
    	    	    	    	        ( 2 * sizeof( icon_size_ptr ) ) +
    	    	    	    	    	1 );    /* for \0 char */
    	strcpy( icon_index, root_index_name );
    	strcat( icon_index, "_" );
    	strcat( icon_index, icon_size_ptr );
    	strcat( icon_index, "X" );
    	strcat( icon_index, icon_size_ptr );

    	*icon_size_rtn = (unsigned int) icon_size;
    }

    return( icon_index );

}  /* end of get_icon_index_name */


/*
 *  Sets the iconify pixmap in the DEC_WM_HINTS property for the
 *  given shell widget.
 */
static void
set_iconify_pixmap( shell, iconify_pixmap )
    Widget shell;
    Pixmap iconify_pixmap;
{
typedef unsigned long int   INT32;
typedef struct {
        INT32 value_mask;
        INT32 iconify_pixmap;
        INT32 icon_box_x;
        INT32 icon_box_y;
        INT32 tiled;
        INT32 sticky;
        INT32 no_iconify_button;
        INT32 no_lower_button;
        INT32 no_resize_button;
} internalDECWmHintsRec, *internalDECWmHints;

#define WmNumDECWmHintsElements ( sizeof( internalDECWmHintsRec ) / sizeof( INT32 ) )

#ifndef DECWmIconifyPixmapMask
#define DECWmIconifyPixmapMask	( 1L << 0 )
#endif

    internalDECWmHints	  prop = 0;
    internalDECWmHintsRec prop_rec;
    Atom    	    	  decwmhints;
    Atom    	    	  actual_type;
    int     	    	  actual_format;
    long    	    	  leftover;
    unsigned long   	  nitems;
    Boolean 	    	  free_prop = True;

    decwmhints = XmInternAtom( XtDisplay( shell ), "DEC_WM_HINTS", False );

    if ( XGetWindowProperty( XtDisplay( shell ), XtWindow( shell ), decwmhints,
    	    	0L, (long)WmNumDECWmHintsElements, False, decwmhints,
    	    	&actual_type, &actual_format, &nitems,
    	    	(unsigned long *) &leftover, (unsigned char **)&prop )
    	    != Success )
    	return;
    if ( ( actual_type != decwmhints )
    	 || ( nitems < WmNumDECWmHintsElements ) || ( actual_format != 32 ) )
    {
    	if ( prop != 0 ) XFree ( (char *)prop );

    	/*  An empty "value" was returned, so create a new one.
    	 */
    	free_prop = False;
    	prop = (internalDECWmHints) &prop_rec;
    	prop->value_mask    	= 0;
    	prop->icon_box_x 	= -1;
    	prop->icon_box_y 	= -1;
    	prop->tiled 	    	= False;
    	prop->sticky     	= False;
    	prop->no_iconify_button	= False;
    	prop->no_lower_button	= False;
    	prop->no_resize_button	= False;
    }

    prop->value_mask	 |= DECWmIconifyPixmapMask;
    prop->iconify_pixmap  = iconify_pixmap;

    XChangeProperty( XtDisplay( shell ), XtWindow( shell ), decwmhints,
    	    	     decwmhints, 32, PropModeReplace,
    	    	     (unsigned char *) prop, WmNumDECWmHintsElements );

    if ( free_prop )
    	if ( prop != 0 ) XFree ( (char *)prop );

}  /* end of set_iconify_pixmap */



/*
**++
**  ROUTINE NAME:
**	MakePixmap(dpy, root, data, width, height)
**
**  FUNCTIONAL DESCRIPTION:
** 	routine to make a pixmap stolen from session mgr
**
**  FORMAL PARAMETERS:
**	Display *dpy;
**	Drawable root;
**	short *data;
**	Dimension width, height;
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

static Pixmap MakePixmap(dpy, root, data, width, height)
Display *dpy;
Drawable root;
short *data;
Dimension width, height;
{
    Pixmap pid;
    unsigned long ScreenNumber;

    ScreenNumber = XDefaultScreen(dpy);

    pid = XCreatePixmapFromBitmapData (dpy, root, (char *) data,
		(Dimension) width, (Dimension) height,
		(unsigned long) BlackPixel (dpy, ScreenNumber),
		(unsigned long) WhitePixel (dpy, ScreenNumber),
		1);							/* Changed to one for Motif - who knows why */
/*		(unsigned int) DefaultDepth (dpy, ScreenNumber));	*/
    return(pid);
}



/*
**++
**  ROUTINE NAME:
**	ScrBringToFront(w)
**
**  FUNCTIONAL DESCRIPTION:
**	Force the specified widget to appear at the top of the window stack.
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
**	If perfomance is a problem, this could be a macro that either
**	manages or pops the widget.
**--
**/

static void ScrBringToFront(w)
Widget w;

{
    XtWidgetGeometry request, reply;
    XtGeometryResult result;

    /* If it is already visible - raise it and give it focus 	*/

    if (XtIsManaged(w))
      {
	/* it is visible, pop it to the top of the screen 	*/
	XRaiseWindow(XtDisplay(w),
		     XtWindow(XtParent(w)) );
      }

}






/*
**++
**  ROUTINE NAME:
**	change_title()
**
**  FUNCTIONAL DESCRIPTION:
**	call XtSetValues on the top level widget to change the title bar and
**	the icon to contain the name of the currently loaded file.
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

void change_title()
{
    char new_title[300];
    char new_icon[300];
    Arg  arg_list[10];

    /*  have the new title contain "CDA Viewer: followed
     *  by the name of the file. Have the icon labeled with
     *  the name of the application.
     */
    strcpy(new_title, dvr_title_str);

    /* if the copyright is still up, don't change title yet */

    if (done_with_copyright)
      {
	if (file_loaded)
	  {
	    strcat(new_title, dvr_colon_str);
            strcat((char *) new_title, (char *) current_file);
	  }

	/* else, viewer empty,  just leave 'CDA Viewer' in title bar */

      }
    else
      {
	strcat(new_title, dvr_colon_str);
	strcat(new_title, dvr_copyright_str);
      }

    /* now adjust icon text */
    if (file_loaded)
      {
   	/*  if a file is loaded, place the name of the file in the
	 *  icon; just the file name is used, the directory is stripped
	 */
	strip_directory(current_file, new_icon);
      }
    else
	/*  else, no file is loaded, just use application name  */
	strcpy(new_icon, dvr_title_str);


    XtSetArg(arg_list[0], XmNtitle, new_title);
    XtSetArg(arg_list[1], XmNiconName, new_icon);

    XtSetValues(toplevel, arg_list, 2);
}





/*
**++
**  ROUTINE NAME:
**	viewer_new_file(file_name, file_format, options_file)
**
**  FUNCTIONAL DESCRIPTION:
**      Call the viewer widget's file associate procedure for the
**      passed-in-file. Then call the change_title procedure so that the
**	new file name appears on the title bar.
**
**  FORMAL PARAMETERS:
**   	char *file_name;
**	char *file_format;
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

void viewer_new_file(file_name, file_format, options_file)
    char *file_name;
    char *file_format;
    char *options_file;
{
   int stat;
   int p_options=0;
   Arg arg_list[10];
   FILE	*read_stat;	/* test to make sure file can be read */

    /* clear any text out of the diagnostic info buffer
     * and grey out the diagnostic info selection
     */
    if ((diag_info_text_buffer != NULL)
	&& (diag_info_text_widget != NULL))  {

	diag_info_text_buffer[0] = '\0';
	XmTextSetString( diag_info_text_widget
			, diag_info_text_buffer		/* null text	    */
			);

        set_sensitivity (diag_info_button, FALSE);
	diag_info_button_on = FALSE;
	}

    /* clear any text out of the diagnostic info buffer */
    if (diag_info_text_buffer != NULL)  {

	diag_info_text_buffer[0] = '\0';
	XmTextSetString( diag_info_text_widget
			 , diag_info_text_buffer		/* null text	    */
			 );
	}

    free_doc_info_buffer();
    set_sensitivity (doc_info_button, FALSE);

    /* set cursor to a watch */
    set_watch_cursor(toplevel);

    /* put up new file name */
    strcpy((char *) current_file, (char *) file_name);
    change_title();

    /* try to load the file */
    stat = DvrViewerFile((DvrViewerWidget) viewer, file_name, file_format,
			 options_file, NULL, NULL);

    /* switch cursor back */
    reset_cursor(toplevel);

    /*  if not successful, clear the window (lory will eventually
     *	handle this), and blast up an error message box. If we are
     *  seccessfull, copy the new file name into the global var.
     */
    if ( stat != DVR_NORMAL)
      {
	file_loaded = 0;

	/* disable close and document info */
        set_sensitivity (close_button_id, FALSE);
        set_sensitivity (doc_info_button, FALSE);

	/* free the document info buffer if allocated */
	free_doc_info_buffer();
      }
    else
      {

	file_loaded = 1;

#ifdef __unix__
	/* for ultrix, assume stdin for empty strings */
    	if (strlen(file_name) == 0)
	    strcpy((char *) current_file, dvr_stdin_str);
	else
#endif
	strcpy((char *) current_file, (char *) file_name);

        /* Make buttons sensitive to input */
        set_sensitivity (close_button_id, TRUE);

/* cbr stuff */

	if (cbr_mode == TRUE)
	    set_sensitivity (close_button_id, FALSE);

/* end cbr stuff */

	stat = DvrDocumentInfo((DvrViewerWidget) viewer, &doc_info_text_buffer);
 	if (stat == DVR_NORMAL)
	  {
            set_sensitivity (doc_info_button, TRUE);
 	    if (doc_info_text_widget != 0)
	        XmTextSetString(doc_info_text_widget, doc_info_text_buffer);
	  }
	else
	  doc_info_text_buffer = 0;

      }

    change_title();	    /* change the title bar */

}




/*
**++
**  ROUTINE NAME:
**	exit_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is called when the user selects
**	exit from the file menu. It exits the application.
**
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void exit_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;
{

    close_proc(w, tag, reason);

    free_uil_strings();

    if (paper_height_str != 0)
        XtFree(paper_height_str);
    if (paper_width_str != 0)
        XtFree(paper_width_str);
    if (paper_height_str != 0)
        XtFree(save_height_str);
    if (paper_width_str != 0)
        XtFree(save_width_str);

    if (WatchCursor != (Cursor) NULL)
	XFreeCursor(XtDisplay(toplevel), WatchCursor);

    XtDestroyWidget(viewer);
    XtDestroyWidget(main_window);
    XtDestroyWidget(toplevel);

    /* free uid file name */
    if (db_filename_vec[0] != 0)
    	XtFree(db_filename_vec[0]);

    /* close Mrm Hierarchy */
    MrmCloseHierarchy(dvr_DRMHierarchy);

    /* Close the help System on exit */
#ifdef CDA_HYPER_HELP
    if (hyper_help_open)
       {
       DXmHelpSystemClose (help_context, help_error, "Help System Error");
       }
#endif
#ifdef __vms__
    exit(DVR_NORMAL);
#else
    exit(0);
#endif

}


/*
**++
**  ROUTINE NAME:
**	fetch_help ()
**
**  FUNCTIONAL DESCRIPTION:
**      This routine fetches the help widget if it has not
**      yet been fetched. It changes the library spec if this
**	is ultrix.
**
**  FORMAL PARAMETERS:
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

fetch_help ()

{
XmString cstr;

   if (help_widget == 0) {
   	if (MrmFetchWidget(
	    dvr_DRMHierarchy,
	    "help_widget_box",
	    viewer,
	    & help_widget,
	    & dummy_class) != MrmSUCCESS)
      {
	fprintf(stderr, dvr_drm_popup_ffail_str);
	return(DVR_DRMPOPUPFETCHFAIL);
      }


#ifdef __unix__
    	{
	  Arg arglist[2];
	  cstr = XmStringLtoRCreate(dvr_ult_help_library , "ISO8859-1");
	  XtSetArg(arglist[0], DXmNlibrarySpec, cstr);
	  XtSetValues(help_widget,
		      arglist,
		      1);
	  XtFree((char *)cstr);

    	}
#endif


    }

    return(DVR_NORMAL);
}



/*
**++
**  ROUTINE NAME:
**	help_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called when help is requested
**      from the menu bar. Map the help widget with Overview as the
**	first topic.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void help_proc (w, tag, reason)
    	Widget		w;
    	caddr_t		*tag;
    	caddr_t		*reason;

{
    Arg arglist[2];
    int stat;
    MrmCode		dummy_type;		/* Dummy parameter passed to the fetch literal routine */

#ifndef CDA_HYPER_HELP
       {
       XmString		Help_topic;		/* Help Topic - Compound String */
       stat = fetch_help();
       if (stat == DVR_NORMAL)
          {
	  /* First fetch the name of the topic to be looked up */

	  stat = MrmFetchLiteral(
			dvr_DRMHierarchy,
			"dvr$help_topic_on_window",	/* Topic we want to start with in the help widget */
			XtDisplay(toplevel),
			(caddr_t *) &Help_topic,
			&dummy_type);

	  if (stat != MrmSUCCESS)
	     {
	     Help_topic = XmStringLtoRCreate("Overview", "ISO8859-1");
	     }

    	  XtSetArg(arglist[0], DXmNfirstTopic, Help_topic);
	  XtSetValues(help_widget, arglist,  1);

    	  XtManageChild(help_widget);
    	  ScrBringToFront(help_widget);

	  /* Free up the compound string */

	  XtFree((char *)Help_topic);
          }
       }
#else
       {
       CDAstatus	    ret_status;
       String		    h_help_topic;


       /*  Set up Help System environment */
       if (!hyper_help_open)
          {
          DXmHelpSystemOpen (&help_context, toplevel, viewer_help, help_error,
                                                "Help System Error");
          hyper_help_open = TRUE;
          }
       ret_status = fetch_simple_string("dvr$help_overview", &h_help_topic);

       if (ret_status != DVR_NORMAL)
          if_error_report (ret_status);
       else
          DXmHelpSystemDisplay (help_context,viewer_help,"topic",h_help_topic,
					    help_error,"Help System Error");
       }
#endif
}


/*
**++
**  ROUTINE NAME:
**	about_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called when about is requested
**      from the menu bar. Map the help widget with about as the
**	first topic.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void about_proc (w, tag, reason)
    	Widget		w;
    	caddr_t		*tag;
    	caddr_t		*reason;

{
    Arg arglist[2];
    int stat;
    XmString		Help_topic;		/* Help Topic - Compound String */
    MrmCode		dummy_type;		/* Dummy parameter passed to the fetch literal routine */

    stat = fetch_help();
    if (stat == DVR_NORMAL)
      {
	/* First fetch the name of the topic to be looked up */

	/* Note that we used to fetch the string ABOUT but since */
	/* we converted to Motif we want "On Version".  The name */
	/* stored in the help library is still ABOUT but we want */
	/* the names to match as closely as possible in UIL.	 */

	stat = MrmFetchLiteral(
			dvr_DRMHierarchy,
			"dvr$help_topic_on_version",	/* Topic we want to start with in the help widget */
			XtDisplay(toplevel),
			(caddr_t *) &Help_topic,
			&dummy_type);

	if (stat != MrmSUCCESS)
	    {
	     /*
	      * Name in the library is still "About" even though we're "On Version"
	      * in the UIL file and on the pull down menu.
	     */

	     Help_topic = XmStringLtoRCreate("About", "ISO8859-1");
	    }

    	XtSetArg(arglist[0], DXmNfirstTopic, Help_topic);
	XtSetValues(help_widget,
		    arglist,
		    1);

    	XtManageChild(help_widget);
    	ScrBringToFront(help_widget);
	/* Free up the compound string */

	XtFree((char *)Help_topic);
      }

}


/*
**++
**  ROUTINE NAME:
**	on_help_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called when On Help is requested
**      from the menu bar. Map the help widget with onhelp_item as the
**	first topic.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void on_help_proc (w, tag, reason)
    	Widget		w;
    	caddr_t		*tag;
    	caddr_t		*reason;

{
    Arg arglist[2];
    int stat;
    XmString		Help_topic;		/* Help Topic - Compound String */
    MrmCode		dummy_type;		/* Dummy parameter passed to the fetch literal routine */

    stat = fetch_help();
    if (stat == DVR_NORMAL)
      {
	/* Fetch the name of the item that we're suppose to be  */
	/* displaying.  In the event that we can't find it fall */
	/* back to a default string stored below.		*/

	stat = MrmFetchLiteral(
			dvr_DRMHierarchy,
			"dvr$help_topic_on_help",	/* Topic we want to start with in the help widget */
			XtDisplay(toplevel),
			(caddr_t *) &Help_topic,
			&dummy_type);

	if (stat != MrmSUCCESS)
	    {
	     /*
	      * Name in the library on_help_item.
	     */

	     Help_topic = XmStringLtoRCreate("onhelp_item", "ISO8859-1");
	    }

    	XtSetArg(arglist[0], DXmNfirstTopic, Help_topic);
	XtSetValues(help_widget,
		    arglist,
		    1);

    	XtManageChild(help_widget);
    	ScrBringToFront(help_widget);
	/* Free up the compound string */

	XtFree((char *)Help_topic);
      }

}


/*
**++
**  ROUTINE NAME:
**	unit_toggle_change_proc (w, tag, cb_data)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called after the user has clicked one
**      of the toggle buttons in the size unit radio box;
**	it updates the current paper size units.
**
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
**    	XmToggleButtonCallbackStruct *cb_data;
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

static void unit_toggle_change_proc (w, tag, cb_data)
    Widget		w;
    caddr_t		*tag;
    XmToggleButtonCallbackStruct *cb_data;

{
    if (cb_data->set)
      {
	/*  if button is being turned on, update statics to reflect
	 *  current paper size unit; update current values to reflect
	 *  current unit;
	 */
	unsigned short 	new_size_unit;
    	char 		*new_height_str;
    	char 		*new_width_str;
    	float  		new_paper_height,
			new_paper_width;

	new_height_str = (char *) XtMalloc(20);
	new_width_str  = (char *) XtMalloc(20);

	/* get current height string, and set number value */
    	new_height_str = (char *) XmTextGetString( size_height_text_id );

    	if (strlen(new_height_str) != 0)
	    sscanf(new_height_str, "%f", &new_paper_height);
    	else
	    new_paper_height = 0;

	/* get current width string, and set number value */
   	new_width_str = (char *) XmTextGetString( size_width_text_id );

    	if (strlen(new_width_str) != 0)
    	    sscanf(new_width_str, "%f", &new_paper_width);
    	else
	    new_paper_width = 0;

	/* set new unit */
    	if (w == char_toggle_id)
	    new_size_unit = DVR_CHAR_UNITS;
	else if (w == inch_toggle_id)
	    new_size_unit = DVR_INCH_UNITS;
	else if (w == mm_toggle_id)
	    new_size_unit = DVR_MM_UNITS;

	/* convert width to new units */
        if (new_paper_width != 0)
	  {
	    if ( (current_size_unit == DVR_MM_UNITS) &&
		 (new_size_unit     == DVR_INCH_UNITS) )
		new_paper_width = new_paper_width / (float) MM_PER_INCH;

	    else if ( (current_size_unit == DVR_MM_UNITS) &&
		      (new_size_unit     == DVR_CHAR_UNITS) )
		new_paper_width = new_paper_width / (float) MM_PER_CHAR_W;

	    else if ( (current_size_unit == DVR_INCH_UNITS) &&
		      (new_size_unit     == DVR_MM_UNITS) )
		new_paper_width = new_paper_width * (float) MM_PER_INCH;

	    else if ( (current_size_unit == DVR_INCH_UNITS) &&
		      (new_size_unit     == DVR_CHAR_UNITS) )
		new_paper_width = new_paper_width / (float) INCH_PER_CHAR_W;

	    else if ( (current_size_unit == DVR_CHAR_UNITS) &&
		      (new_size_unit     == DVR_INCH_UNITS) )
		new_paper_width = new_paper_width * (float) INCH_PER_CHAR_W;

	    else if ( (current_size_unit == DVR_CHAR_UNITS) &&
		      (new_size_unit     == DVR_MM_UNITS) )
		new_paper_width = new_paper_width * (float) MM_PER_CHAR_W;

	  }

	/* convert height to new units */
        if (new_paper_height != 0)
	  {
	    if ( (current_size_unit == DVR_MM_UNITS) &&
		 (new_size_unit     == DVR_INCH_UNITS) )
		new_paper_height = new_paper_height / (float) MM_PER_INCH;

	    else if ( (current_size_unit == DVR_MM_UNITS) &&
		      (new_size_unit     == DVR_CHAR_UNITS) )
		new_paper_height = new_paper_height / (float) MM_PER_CHAR_H;

	    else if ( (current_size_unit == DVR_INCH_UNITS) &&
		      (new_size_unit     == DVR_MM_UNITS) )
		new_paper_height = new_paper_height * (float) MM_PER_INCH;

	    else if ( (current_size_unit == DVR_INCH_UNITS) &&
		      (new_size_unit     == DVR_CHAR_UNITS) )
		new_paper_height = new_paper_height / (float) INCH_PER_CHAR_H;

	    else if ( (current_size_unit == DVR_CHAR_UNITS) &&
		      (new_size_unit     == DVR_INCH_UNITS) )
		new_paper_height = new_paper_height * (float) INCH_PER_CHAR_H;

	    else if ( (current_size_unit == DVR_CHAR_UNITS) &&
		      (new_size_unit     == DVR_MM_UNITS) )
		new_paper_height = new_paper_height * (float) MM_PER_CHAR_H;

	  }

	/* set new units */
	current_size_unit = new_size_unit;

	/* store new width string */
	if (strlen(new_width_str) != 0)
	  {
	    sprintf(new_width_str, "%g", new_paper_width);
	  }

	/* store new height string */
	if (strlen(new_height_str) != 0)
	  {
	    sprintf(new_height_str, "%g", new_paper_height);
	  }

	/* set text widgets to new (converted) values */
    	XmTextSetString(size_height_text_id, new_height_str);
    	XmTextSetString(size_width_text_id, new_width_str);

	/* free memory from strings */
	XtFree(new_height_str);
	XtFree(new_width_str);

      }


}


/*
**++
**  ROUTINE NAME:
**	file_sel_proc (w, tag, cb_data)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called after the user has clicked one
**      of the buttons in the file selection widget. If the reason
**	is activate, the new file is loaded.
**
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
**    	XmFileSelectionBoxCallbackStruct *cb_data;
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
static void file_sel_proc (w, tag, cb_data)
    Widget		w;
    caddr_t		*tag;
    XmFileSelectionBoxCallbackStruct *cb_data;


{
   int reason = cb_data->reason;
   int stat;
   XmString 		cstr;

   XtUnmanageChild(file_sel_box);

   if ((reason == (int)XmCR_ACTIVATE) || (reason == (int)XmCR_OK))
     {
	char 	*file_format_str;
	char	*op_file_str;
	char	*char_set;
	int	dont_care;
        char    *file_name;
	Arg	arg_list[3];

	XmStringContext 	context;
	XmString 		file_sel_value;

/*
 * If the Paper size or Options file dialog boxes are
 * still up unmanage them.
*/

	if ((options_file_sel_box != (Widget) NULL) && XtIsRealized(options_file_sel_box))
	    XtUnmanageChild(options_file_sel_box);

	if ((paper_size_dialog_box != (Widget) NULL) && XtIsRealized(paper_size_dialog_box))
	    XtUnmanageChild(paper_size_dialog_box);

/*
 * The following code segment has been commented out since it doesn't
 * work with the current base level of Motif.  In order to correct this
 * problem, I'm going to use the value of XmNdirSpec that is returned in
 * the call back structure.
 *
 *	XtSetArg(arg_list[0], XmNdirSpec, &file_sel_value);
 *	XtGetValues(file_sel_box, arg_list, 1);
 *
 *	XmStringInitContext(&context, file_sel_value);
*/
	XmStringInitContext(&context, cb_data->value);			/* Get XmNdirSpec from the callback structure */

	XmStringGetNextSegment(context, &file_name, &char_set,		/* Note context is now by value in Motif */
			       (XmStringDirection *) &dont_care, (Boolean *) &dont_care);

	file_format_str = (char *) XmTextGetString(format_text_id);

	strcpy((char *) current_format, (char *) file_format_str);

#ifdef __vms__
	upper_case(current_format);
#endif

        if (strlen(file_name) == 0)
#ifdef __vms__
	    display_error(RMS$_FNF);
#else
	    display_error(CDA_OPENFAIL);
#endif
	else
            viewer_new_file(file_name, file_format_str, current_op_file);

        XtFree(file_name);
	XtFree(file_format_str);
	XtFree(char_set);						/* Character set is now an ASCIZ string in Motif */
	XtFree((char *)context);
     }

   else
     {
     	/* cancelled operation, reset text */
   	cstr =  XmStringLtoRCreate(save_file_format_str , "ISO8859-1"),
        (void) XmTextSetString(format_text_id, save_file_format_str);
        (void) XmListSelectItem(format_list_id, cstr,FALSE);
	XtFree((char *)cstr);

	/*
	 * If the Paper size or Options file dialog boxes are
	 * still up unmanage them.
	 *
	 * Note that we don't want to do this if someone pressed help or some
	 * other key we're not aware of.
	*/

	if (reason == (int)XmCR_CANCEL)
	    {
		if ((options_file_sel_box != (Widget) NULL) && XtIsRealized(options_file_sel_box))
		    XtUnmanageChild(options_file_sel_box);

		if ((paper_size_dialog_box != (Widget) NULL) && XtIsRealized(paper_size_dialog_box))
		    XtUnmanageChild(paper_size_dialog_box);
	    };
     }

    XtFree(save_file_format_str);

}


/*
**++
**  ROUTINE NAME:
**	options_file_sel_proc (w, tag, cb_data)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called after the user has clicked one
**      of the buttons in the options file selection widget. If the reason
**	is activate, then save the new options file from the widget;
**
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
**    	XmFileSelectionBoxCallbackStruct *cb_data;
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
static void options_file_sel_proc (w, tag, cb_data)
    Widget		w;
    caddr_t		*tag;
    XmFileSelectionBoxCallbackStruct *cb_data;


{
   int reason = cb_data->reason;
   int stat;

   XtUnmanageChild(options_file_sel_box);

   if ((reason == (int)XmCR_ACTIVATE) || (reason == (int)XmCR_OK))
     {
	char	*op_file_str;
   	char 	*file_format_str;
	char	*char_set;
	int	dont_care;
        char    *file_name;
	Arg 	arg_list[3];

	XmStringContext 	context;

	XmStringInitContext(&context, cb_data->value);

	XmStringGetNextSegment(context, &op_file_str, &char_set,		/* Note context is now by value in Motif */
			       (XmStringDirection *) &dont_care, (Boolean *) &dont_care);

	/* save the current op file  */
        strcpy((char *) current_op_file, (char *) op_file_str);

	/* free memory */
	XtFree(op_file_str);
	XtFree(char_set);						/* Character set is now an ASCIZ string in Motif */
	XtFree((char *)context);
     }

else  /* cancelled operation */
     {
     }
}


/*
**++
**  ROUTINE NAME:
**	new_file_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is called when the user selects
**	"OPEN" from the file menu. It maps a file selection
**	box to the screen.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void new_file_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{

    Arg arglist[10];
    int argcount = 0;
    XmString cstr;

/* cbr stuff */

    if (cbr_mode == TRUE) {
        if (cbr_new_file_cb != 0) {
            if (*cbr_new_file_cb != 0) {
                DvrCbrSetCtxt((DvrViewerWidget) viewer, cbr_frg_ref_cb, cbr_frg_ref_parm, cbr_ref_buttons);
                if ( (* cbr_new_file_cb) ())  return;
                DvrCbrClearCtxt((DvrViewerWidget) viewer);
            }
        }
/*
        DvrCbrClearCtxt(viewer);
        Dvr__DECW_Cbr_Clear_Mode ();
*/
    }

/* end cbr stuff */

    /*  if the file selection box has not yet
     *  been fetched (created), do so, and then map it
     */

   if (file_sel_box == 0)
      {
      if (MrmFetchWidget (
	  dvr_DRMHierarchy,
	  "file_sel_box",
	  viewer,
	  & file_sel_box,
      	  & dummy_class) != MrmSUCCESS)
         {
	 fprintf(stderr, dvr_drm_popup_ffail_str);
	 return;
         }
      else
         {
#ifdef CDA_HYPER_HELP
            {
	    Widget		file_sel_child;
	    CDAstatus	ret_status;
	    String		help_topic;
	    /*
	    ** File selction widget has only one help callback. For implemnting
	    ** context sensitive help on APPLY, FILTER and Cancel buttons, help
	    ** callback resource along with the call back routine has to be added
	    ** for those buttons.
	    */
	    /* Set helpcallback on cancel button */
	    file_sel_child = (Widget) XmFileSelectionBoxGetChild(file_sel_box,
 				     (unsigned char) XmDIALOG_CANCEL_BUTTON);
	    ret_status = fetch_simple_string("dvr$help_op_cancel_but",&help_topic);
	    if_error_report (ret_status);
	    set_hyper_help_cbs (file_sel_child, help_topic);

	    /* Set helpcallback on ok button */
	    file_sel_child = (Widget) XmFileSelectionBoxGetChild(file_sel_box,
				     (unsigned char) XmDIALOG_OK_BUTTON);
	    ret_status = fetch_simple_string("dvr$help_op_ok_but",&help_topic);
	    if_error_report (ret_status);
	    set_hyper_help_cbs (file_sel_child, help_topic);

	    /* Set helpcallback on filter button */
	    file_sel_child = (Widget) XmFileSelectionBoxGetChild(file_sel_box,
				     (unsigned char) XmDIALOG_APPLY_BUTTON);
	    ret_status = fetch_simple_string("dvr$help_op_filter_but",&help_topic);
	    if_error_report (ret_status);
	    set_hyper_help_cbs (file_sel_child, help_topic);

	    /* Set helpcallback on filter label*/
	    file_sel_child = (Widget) XmFileSelectionBoxGetChild(file_sel_box,
				     (unsigned char) XmDIALOG_FILTER_LABEL);
	    ret_status = fetch_simple_string("dvr$help_open_file_filt",&help_topic);
	    if_error_report (ret_status);
	    set_hyper_help_cbs (file_sel_child, help_topic);

	    /* Set helpcallback on filter text*/
	    file_sel_child = (Widget) XmFileSelectionBoxGetChild(file_sel_box,
				     (unsigned char) XmDIALOG_FILTER_TEXT);
	    ret_status = fetch_simple_string("dvr$help_open_file_filt",&help_topic);
	    if_error_report (ret_status);
	    set_hyper_help_cbs (file_sel_child, help_topic);

	    /* Set helpcallback on file selection label*/
	    file_sel_child = (Widget) XmFileSelectionBoxGetChild(file_sel_box,
				     (unsigned char) XmDIALOG_SELECTION_LABEL);
	    ret_status = fetch_simple_string("dvr$help_open_file_box",&help_topic);
	    if_error_report (ret_status);
	    set_hyper_help_cbs (file_sel_child, help_topic);

	    /* Set helpcallback on file selection Text */
	    file_sel_child = (Widget) XmFileSelectionBoxGetChild(file_sel_box,
				     (unsigned char) XmDIALOG_TEXT);
	    ret_status = fetch_simple_string("dvr$help_open_file_box",&help_topic);
	    if_error_report (ret_status);
	    set_hyper_help_cbs (file_sel_child, help_topic);
            }
#endif
         if (strlen((char *) current_file_dir) != 0)
     	    {

	    argcount = 0;
	    cstr = XmStringLtoRCreate((char *) current_file_dir , "ISO8859-1");
       	    XtSetArg(arglist[argcount], XmNdirMask, cstr);
       	    argcount++;
            XtSetValues(file_sel_box, arglist, argcount);
	    XtFree((char *)cstr);

	    strcpy((char *) current_dir_mask, (char *) current_file_dir);

#ifdef __vms__
	    upper_case(current_dir_mask);
#endif
     	    }
         }
      }

   /* remember text in case of cancel */
   save_file_format_str = (char *) XmTextGetString(format_text_id);

   XtManageChild(file_sel_box);

   argcount = 0;
   cstr = XmStringLtoRCreate((char *) current_file , "ISO8859-1");
   XtSetArg(arglist[argcount], XmNdirSpec, cstr);	/* was XmNvalue in XUI */
   argcount++;
   XtSetValues(file_sel_box, arglist, argcount);
   XtFree((char *)cstr);

   ScrBringToFront(file_sel_box);
}

/*
**++
**  ROUTINE NAME:
**	set_hyper_help_cbs (child_widget, value)
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is called to set the help callback resource for the
**	input widget.
**
**  FORMAL PARAMETERS:
**    	Widget		child_widget;
**    	String		value;
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
**	Set the XmNhelpCallback resource for the File Selection Widget .
**--
**/

static void set_hyper_help_cbs (child_widget, value)
    Widget		child_widget;
    String		value;

{
    unsigned long	next_arg;
    Arg			arglist[1];
    int			argcount = 0;

    hyper_help_cb[0].closure = (XtPointer)value;
    next_arg = (unsigned long) hyper_help_cb;
    XtSetArg(arglist[argcount], XmNhelpCallback, next_arg);
    argcount++;
    XtSetValues(child_widget, arglist, argcount);
}

/*
**++
**  ROUTINE NAME:
**	close_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is called when the user selects
**	"CLOSE" from the file menu. It closes the current open
**	file.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void close_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{
    unsigned long stat;

    /* set cursor to a watch */
    set_watch_cursor(toplevel);

    stat = DvrCloseFile((DvrViewerWidget) viewer);

    /* reset cursor */
    reset_cursor(toplevel);

    file_loaded = 0;
    change_title();	    /* change the title bar */

    /* Make buttons insensitive to input; dim/grey push buttons */
    set_sensitivity (close_button_id, FALSE);
    set_sensitivity (doc_info_button, FALSE);
    set_sensitivity (diag_info_button, FALSE);
    diag_info_button_on = FALSE;

    /* clear any text out of the diagnostic info buffer */
    if (diag_info_text_buffer != NULL)  {

	diag_info_text_buffer[0] = '\0';
	XmTextSetString( diag_info_text_widget
			 , diag_info_text_buffer		/* null text	    */
			 );
	}

    free_doc_info_buffer();
}

/*
**++
**  ROUTINE NAME:
**	free_doc_info_buffer()
**
**  FUNCTIONAL DESCRIPTION:
**	clear the document info text buffer
**
**  FORMAL PARAMETERS:
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

void free_doc_info_buffer ()
{
    /* memory get's freed by widget */
    doc_info_text_buffer = 0;
}


/*
**++
**  ROUTINE NAME:
**	set_sensitivity (w, true_false)
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure can be called with any widget ID and either TRUE or
**	FALSE to change the sensitivity of the corresponding widget; TRUE
**	is enable sensitivity; FALSE is to make widget insensitive (dimmed/
**	greyed)
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**	Boolean		true_false;
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

static void set_sensitivity (w, true_false)
    Widget		w;
    Boolean		true_false;

{
    /* Local variables */
    caddr_t		*tag;
    caddr_t		*reason;
    Arg 	        arglist[2];

    /* Set sensitivity value as indicated */
    if (w == NULL) return;	/* Error; can't do this; just return */
    XtSetArg(arglist[0], XmNsensitive, true_false);
    XtSetValues(w, arglist, 1);

}



/*
**++
**  ROUTINE NAME:
**	format_list_click_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	routine called when a user clicks in the format list
**	box; set the format text widget to the new value;
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void format_list_click_proc (w, tag, reason)
    Widget		     	w;
    caddr_t 			*tag;
    XmListCallbackStruct	*reason;

{

    char		*char_set;
    int			dont_care;
    char    		*format_name;
    XmStringContext 	context;

    /* get null terminated string from compound string */
    XmStringInitContext(&context, reason->item);

    XmStringGetNextSegment(context, &format_name, &char_set,		/* Note context is now by value in Motif */
		           (XmStringDirection *) &dont_care, (Boolean *) &dont_care);

    /* set text widget to new value */
    (void) XmTextSetString(format_text_id, format_name);

    /* free memory */
    XtFree(format_name);
    XtFree(char_set);							/* Character set is now an ASCIZ string in Motif */
    XtFree((char *)context);

    /* update dir mask */
    (void) stext_cr_proc(w);
}

/*
**++
**  ROUTINE NAME:
**	format_list_confirm_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	routine called when user double clicks in format list
**	box; currently does nothing;
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void format_list_confirm_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{
/*
    stext_cr_proc(w);
*/
}


/*
**++
**  ROUTINE NAME:
**	size_list_click_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	routine called when a user clicks in the size list
**	box; set the size text widgets to the corresponding new values;
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void size_list_click_proc (w, tag, reason)
    Widget		     	w;
    caddr_t 			*tag;
    XmListCallbackStruct	*reason;

{
    Widget unit_widget_id;

    /* entry clicked on; set text widget values */
    current_size_index = reason->item_position;		/* was item_number in XUI */

    if (current_size_unit != paper_list[current_size_index-1].default_unit)
      {
      	/* turn off current toggle before setting to new value */
	Widget old_unit_id;
    	switch (current_size_unit)
      	  {
	    case DVR_CHAR_UNITS :
		old_unit_id = char_toggle_id;
	    	break;
	    case DVR_INCH_UNITS :
		old_unit_id = inch_toggle_id;
	    	break;
	    case DVR_MM_UNITS :
		old_unit_id = mm_toggle_id;
	    	break;
      	  }

    	(void) XmToggleButtonSetState(old_unit_id, FALSE, FALSE);

      }


    current_size_unit = paper_list[current_size_index-1].default_unit;

    switch (current_size_unit)
      {
	case DVR_CHAR_UNITS :
		unit_widget_id = char_toggle_id;
	    	break;
	case DVR_INCH_UNITS :
		unit_widget_id = inch_toggle_id;
	    	break;
	case DVR_MM_UNITS :
		unit_widget_id = mm_toggle_id;
	    	break;
      }

    (void) XmToggleButtonSetState(unit_widget_id, TRUE, FALSE);

    sprintf(paper_height_str,
	    "%g",
	    paper_list[current_size_index-1].pap_height);
    sprintf(paper_width_str,
	    "%g",
	    paper_list[current_size_index-1].pap_width);

    XmTextSetString(size_height_text_id, paper_height_str);
    XmTextSetString(size_width_text_id, paper_width_str);


}


/*
**++
**  ROUTINE NAME:
**	diag_info_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	display the diagnostic information dialog box
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void diag_info_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{

    /*  if the diagnostic information dialog box has not yet
     *  been fetched (created), do so, and then map it
     */

   if (diag_info_dialog_box == 0) {
    if (MrmFetchWidget (
	dvr_DRMHierarchy,
	"diag_info_dialog_box",
	viewer,
	& diag_info_dialog_box,
	& dummy_class) != MrmSUCCESS)
        {
	fprintf(stderr, dvr_drm_popup_ffail_str);

	/* don't try to manage a widget we don't have */
	return;
	}
    }

   XtManageChild(diag_info_dialog_box);
   ScrBringToFront(diag_info_dialog_box);
}

/*
**++
**  ROUTINE NAME:
**	doc_info_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	display the document information dialog box
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void doc_info_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{

    /*  if the document information dialog box has not yet
     *  been fetched (created), do so, and then map it
     */

   if (doc_info_dialog_box == 0) {
    if (MrmFetchWidget (
	dvr_DRMHierarchy,
	"doc_info_dialog_box",
	viewer,
	& doc_info_dialog_box,
	& dummy_class) != MrmSUCCESS)
        {
	fprintf(stderr, dvr_drm_popup_ffail_str);
	return;
	}
      else
        XmTextSetString(doc_info_text_widget, doc_info_text_buffer);

    }

   XtManageChild(doc_info_dialog_box);
   ScrBringToFront( doc_info_dialog_box);
}



/*
**++
**  ROUTINE NAME:
**	diag_info_dismiss_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	unmap the diagnostic information dialog box.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void diag_info_dismiss_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{

   XtUnmanageChild(diag_info_dialog_box);

}


/*
**++
**  ROUTINE NAME:
**	caution_cb_proc (w, tag, cb_struct)
**
**  FUNCTIONAL DESCRIPTION:
**	called when user clicks in one of the caution box's buttons;
**	unmap the box, and allow user to view if they replied yes; else
**	close up file and continue.
**
**  FORMAL PARAMETERS:
**    	Widget			w;
**    	caddr_t			*tag;
**    	XmAnyCallbackStruct 	*cb_struct;
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

static void caution_cb_proc (w, tag, cb_struct)
    Widget		 w;
    caddr_t		 *tag;
    XmAnyCallbackStruct *cb_struct;

{

   XtUnmanageChild(viewer_caution_box);

   if (cb_struct->reason == (int)XmCR_OK)
     {
	/*  user wants to try to view file even though it does
	 *  not start with standard postscript header; set flag and
	 *  reopen file.
         */
	Arg 	arg_list[3];

	XtSetArg(arg_list[0], DvrNheaderRequired, FALSE);
	XtSetValues(viewer, arg_list, 1);

	viewer_new_file(current_file, current_format, NULL);

	/* reset flag to previous state */

	XtSetArg(arg_list[0], DvrNheaderRequired, TRUE);
	XtSetValues(viewer, arg_list, 1);

     }
   else
     {
	/* user does not want to view; close file and continue */
	close_proc(w, tag, cb_struct);
      }

}


/*
**++
**  ROUTINE NAME:
**	doc_info_dismiss_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	unmap the document information dialog box.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void doc_info_dismiss_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{

   XtUnmanageChild(doc_info_dialog_box);

}


/*
**++
**  ROUTINE NAME:
**	paper_size_ok_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	unmap the paper size options dialog box; set new values for widget;
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void paper_size_ok_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{
   Arg arglist[12];
   int argcount = 0;
   Boolean reformat;
   float new_paper_height;
   float new_paper_width;

   int 	orientation_selected,
	orientation_index,
	scale_value,
	i;

   Boolean use_comments,
	   use_bitmaps,
	   use_trays,
	   watch_progress;

   XtUnmanageChild(paper_size_dialog_box);

   reformat = XmToggleButtonGetState(reform_toggle_id);
   if (reformat)
     {
	viewer_options = viewer_options 	&
			 ~DvrSoftDirectives 	&
			 ~DvrLayout     	&
			 ~DvrSpecificLayout;
     }
   else
     {
	viewer_options = viewer_options 	|
			 DvrSoftDirectives	|
			 DvrLayout     		|
			 DvrSpecificLayout;
     }

    XtSetArg(arglist[argcount], DvrNprocessingOptions, viewer_options);
    argcount++;

    paper_height_str = (char *) XmTextGetString( size_height_text_id );

    if (strlen(paper_height_str) != 0)
      {
	/* convert new height to millemeters before setting in widget */
	sscanf(paper_height_str, "%f", &new_paper_height);
	if (current_size_unit == DVR_INCH_UNITS)
	    new_paper_height = new_paper_height * MM_PER_INCH;
	else if (current_size_unit == DVR_CHAR_UNITS)
	    new_paper_height = new_paper_height * MM_PER_CHAR_H;
	current_paper_height = (int) (new_paper_height + 0.5);
      }
    else
	current_paper_height = 0;

    XtSetArg(arglist[argcount], DvrNpaperHeight, current_paper_height);
    argcount++;

    paper_width_str = (char *) XmTextGetString( size_width_text_id );

    if (strlen(paper_width_str) != 0)
      {
	/* convert new width to millemeters before setting in widget */
	sscanf(paper_width_str, "%f", &new_paper_width);
	if (current_size_unit == DVR_INCH_UNITS)
	    new_paper_width = new_paper_width * MM_PER_INCH;
	else if (current_size_unit == DVR_CHAR_UNITS)
	    new_paper_width = new_paper_width * MM_PER_CHAR_W;
	current_paper_width = (int) (new_paper_width + 0.5);
      }
    else
	current_paper_width = 0;

    XtSetArg(arglist[argcount], DvrNpaperWidth, current_paper_width);
    argcount++;

    if (dps_exists)
      {
	/*  if server has display-postscript, get values from
	 *  additional option widgets
	 */

    	use_comments    = XmToggleButtonGetState(ps_comments_id);
    	use_bitmaps     = XmToggleButtonGetState(ps_bwidths_id);
    	use_trays       = XmToggleButtonGetState(ps_fake_trays_id);
    	watch_progress  = XmToggleButtonGetState(ps_draw_mode_id);

    	XtSetArg(arglist[argcount], DvrNuseComments,    use_comments);   argcount++;
    	XtSetArg(arglist[argcount], DvrNuseBitmaps,     use_bitmaps);    argcount++;
    	XtSetArg(arglist[argcount], DvrNuseTrays,       use_trays);      argcount++;
    	XtSetArg(arglist[argcount], DvrNwatchProgress,  watch_progress); argcount++;

    	for (i=0; i<4; i++)
      	  {
	    if (XmToggleButtonGetState(orientGroup[i]))
	        orientation_index = i;
      	  }

        switch (orientation_index)
          {
	    /* ps widget seems to have 90 and 270 backwards */

	    case DEGREES_0	: orientation_selected = 0; 	break;
	    case DEGREES_90	: orientation_selected = 270; 	break;
	    case DEGREES_180	: orientation_selected = 180;   break;
	    case DEGREES_270	: orientation_selected = 90;    break;
          }

    	XtSetArg(arglist[argcount], DvrNorientation, orientation_selected); argcount++;

    	(void) XmScaleGetValue(ps_scale_id, &scale_value);

    	XtSetArg(arglist[argcount], DvrNscaleValue, scale_value); argcount++;
      }

    XtSetValues(viewer, arglist, argcount);

}



/*
**++
**  ROUTINE NAME:
**	paper_size_cancel_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	unmap the paper_size dialog box; reset values;
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void paper_size_cancel_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{
   int 		reformat_value;
   Widget       unit_widget_id;

   XtUnmanageChild(paper_size_dialog_box);

   /* get and reset current reformat state */
   if ( !(viewer_options & DvrLayout) )
      	reformat_value = TRUE;
   else
	reformat_value = FALSE;

   (void) XmToggleButtonSetState(reform_toggle_id, reformat_value, FALSE);

   /* reset paper size s-texts */
   XmTextSetString(size_height_text_id, save_height_str);
   XmTextSetString(size_width_text_id,  save_width_str);


   /* if the selection in the list box has changed, reset it */
   if (current_size_index != save_size_index)
      {
	current_size_index = save_size_index;

    	/* highlight current size in list-box */
	if (current_size_index != 0)
    	    (void) XmListSelectItem(size_list_id,
				paper_list[current_size_index-1].cstring,
				FALSE);
	else
	  {
	    /* 	nothing was previously selected,
	     * 	turn off any cancelled selection
             */
	    Arg arg_list[3];

	    XtSetArg(arg_list[0], XmNselectedItemCount, 0);
	    XtSetValues(size_list_id, arg_list, 1);
	  }
      }

    /* reset paper size units */
    if (current_size_unit != save_size_unit)
      {
    	switch (current_size_unit)
          {
	    case DVR_CHAR_UNITS:
			unit_widget_id = char_toggle_id;
			break;
	    case DVR_INCH_UNITS:
			unit_widget_id = inch_toggle_id;
			break;
	    case DVR_MM_UNITS:
		 	unit_widget_id = mm_toggle_id;
			break;
          }
        (void) XmToggleButtonSetState(unit_widget_id, FALSE, FALSE);
      }

    current_size_unit = save_size_unit;
    switch (current_size_unit)
      {
	case DVR_CHAR_UNITS:
		unit_widget_id = char_toggle_id;
		break;
	case DVR_INCH_UNITS:
		unit_widget_id = inch_toggle_id;
		break;
	case DVR_MM_UNITS:
		unit_widget_id = mm_toggle_id;
		break;
      }
    (void) XmToggleButtonSetState(unit_widget_id, TRUE, FALSE);

   if (dps_exists)
     {
        (void) XmToggleButtonSetState(ps_comments_id,   save_use_comments, FALSE);
        (void) XmToggleButtonSetState(ps_bwidths_id,    save_use_bwidths,  FALSE);
        (void) XmToggleButtonSetState(ps_fake_trays_id, save_use_trays,    FALSE);
        (void) XmToggleButtonSetState(ps_draw_mode_id,  save_draw_mode,    FALSE);

        (void) XmToggleButtonSetState(save_orient_widget, TRUE,  TRUE);


	(void) XmScaleSetValue(ps_scale_id, save_scale);
      }



}



/*
**++
**  ROUTINE NAME:
**	message_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called after the user acknowledges
**      an error message box. If this is a fatal error, we exit
**
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;	  - used to signal a fatal error
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
**	exits on a fatal error
**--
**/

static void message_proc (w, tag, reason)
    Widget		w;
    int			*tag;
    int			*reason;

{

    XtUnmanageChild(viewer_message);

    /*  the tag is used to signal fatal errors; if set
     *  we have a fatal-error, exit the application.
     */
    if (fatal_flag)
	exit(DVR_FATALERROR);

}


/*
**++
**  ROUTINE NAME:
**      map_caution_box()
**
**  FUNCTIONAL DESCRIPTION:
**	map the viewer caution window; this procedure is called when
**	a user tries to view a ps file that does not start with the
**	standard ps header; set the text to include the current file name.
**
**
**  FORMAL PARAMETERS:
**      none
**
**  IMPLICIT INPUTS:
**      none
**
**  FUNCTION VALUE:
**      none
**
**  SIDE EFFECTS:
**      none
**--
**/

map_caution_box()

  {
    Arg  arg_list[3];
    char message_str[356];
    XmString cstr;

    /* fetch caution box if it has not yet been done */

    if (viewer_caution_box == 0)
      {
	if (MrmFetchWidget (
	    dvr_DRMHierarchy,
	    "viewer_caution_box",
	    toplevel,					/* was viewer in XUI */	/* tried main_window */
	    & viewer_caution_box,
	    & dummy_class) != MrmSUCCESS)
	  {
	    fprintf(stderr, dvr_drm_popup_ffail_str);

	    /* don't try to manipulate a widget we don't have */
	    return;

	  }
      } /* end viewer caution create */

    /* create label with current file name */

    /* comment out for now; multiple line fetches screwey */

    /*
    sprintf(message_str, dvr_ps_nohead_str, current_file);
    cstr =  XmStringLtoRCreate(message_str , "ISO8859-1");
    XtSetArg(arg_list[0], XmNmessageString, cstr);
    XtSetValues(viewer_caution_box, arg_list, 1);
    XtFree(cstr);
    */

    if (XtIsManaged(viewer))
        XtManageChild(viewer_caution_box);
    return;
  }






/*
**++
**  ROUTINE NAME:
**      activate_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called when there is an error
**      while reading the document; call the callback procedure
**      with an error reading the document; call the callback procedure
**      with an error reason.
**
**
**  FORMAL PARAMETERS:
**      Widget          w;
**      caddr_t         *tag;
**      caddr_t         *reason;
**
**  IMPLICIT INPUTS:
**      none
**
**  FUNCTION VALUE:
**      none
**
**  SIDE EFFECTS:
**      none
**--
**/

static void activate_proc (w, tag, dvr_reason)
    Widget              w;
    caddr_t             *tag;
    DvrCallbackStruct   *dvr_reason;

{
#define MAX_MESSAGE_LENGTH  256

int	message_length;
char	message_buffer[MAX_MESSAGE_LENGTH];/* DVS diagnostic message	*/
char	*old_buffer;			/* current diag text contents	*/
int	old_buffer_length;
CDAstatus ret_status;		/* return status of this routine    */

    if ( (dvr_reason->reason == DvrCRpsOK)   &&
	 (ps_on_command_line) 		     &&
	 (dvr_reason->status == DVR_NORMAL) )
      {
	/*  PS file opened from command line, we need to call
	 *  DvrDocumentInfo() now that the page has been displayed
	 *  and the PS doc info has been read in; The call to DvrDocumentInfo()
	 *  when the widget has not yet been realized does not work because
   	 *  the ps widget delays work until it is realized
	 */

	ps_on_command_line = FALSE; /* clear flag for all of eternity */
 	if (DvrDocumentInfo((DvrViewerWidget) viewer, &doc_info_text_buffer) == DVR_NORMAL)
	  {
            set_sensitivity (doc_info_button, TRUE);
 	    if (doc_info_text_widget != 0)
	        XmTextSetString(doc_info_text_widget, doc_info_text_buffer);
	  }
	else
	  doc_info_text_buffer = 0;
      }


    /*
     * If this is an error callback with a DVS engine error,
     * update the Diagnostic Info widget with the error text
     */
    else if (dvr_reason->reason == DvrCRcdaError)
      {

      if (dvr_reason->status == DVR_NOPSHEAD)
	{
	  /*  if this file does not contain standard ps header,
	   *  map caution box to make sure user want to try and view it
	   */
	  map_caution_box();
	  return;
	}

      message_buffer[0] = '\0';

      /* If a message text has been supplied, stick it in the
       * message buffer for the diag info widget
       */

      if (dvr_reason->string_ptr != NULL)
        {
	message_length = strlen( dvr_reason->string_ptr );
	if ((message_length + 2) >= MAX_MESSAGE_LENGTH)
	    message_length = MAX_MESSAGE_LENGTH - 2;

	strcpy(message_buffer, dvr_reason->string_ptr);

	if (strcmp((char *) current_format, (char *) POSTSCRIPT_STR) != 0)
	  /*  for layout engine messages, need to append CR
	   *  postscript messages append their own CR
	   */
	  message_buffer[message_length] = '\n';

	message_length++;
	message_buffer[message_length] = '\0';

	} /* end if text provided   */

     else  {
     if ((dvr_reason->status != 0)
	&& (  (SEVERITY(dvr_reason->status) == INFO_STATE)
	   || (SEVERITY(dvr_reason->status) == WARNING_STATE)
	 ) )	{
	 /* This is an informational or warning message.  Stick
	  * the appropriate text in the Diagnostic Info widget
	  */
	  ret_status = get_message_text( dvr_reason->status,
					 message_buffer,
					 TRUE,
#ifdef __vms__
					 TRUE );
#else
					 FALSE );
#endif
	  if (SEVERITY(ret_status) != SUCCESS_STATE ) return;
	  message_length = strlen(message_buffer);

	 }}

     /*
      * If there is a message to put in the diagnostic info widget,
      * update the widget text.
      */
     if (message_buffer[0] != 0)
	{
	/*  if the diagnostic information dialog box has not yet
	 *  been fetched (created), do so, do not map it here.
	 */

        if (diag_info_dialog_box == 0) {
	if (MrmFetchWidget (
	    dvr_DRMHierarchy,
	    "diag_info_dialog_box",
	    viewer,
	    & diag_info_dialog_box,
	    & dummy_class) != MrmSUCCESS)
	    {
	    fprintf(stderr, dvr_drm_popup_ffail_str);

	    /* don't try to manipulate a widget we don't have */
	    return;

	    }
	 } /* end diag info widget create */

	/* Get the current diagnostic text that is out in the widget */

	old_buffer = (char *) XmTextGetString( diag_info_text_widget );
	old_buffer_length = strlen( old_buffer );

	/* Extend the working text buffer size if needed.   */
	if ( (old_buffer_length + message_length)
	     > diag_info_text_length )    {

	    diag_info_text_length += DIAG_BUFF_INCREMENT;

	    if (diag_info_text_buffer != NULL)
		XtFree( diag_info_text_buffer );

	    diag_info_text_buffer = XtMalloc( diag_info_text_length );
	    }

	/* If this message is the same as the last message,
	 * don't bother repeating ourselves
	 */
	if(   (old_buffer_length != 0)
	   && (strcmp( &(old_buffer[old_buffer_length-message_length])
	              , message_buffer)
	       == 0)
	  )
	    {
	      /* Clean up the memory we were handed from the GetString call */
	      XtFree( old_buffer );
	      return;
            }

	/* Copy the current diag text into our buffer,
	 * then tack on the new message
	 */
	strcpy( diag_info_text_buffer, old_buffer );
	strcat( diag_info_text_buffer, message_buffer );

	/* Stuff the new text buffer out to the widget */

	XmTextSetString( diag_info_text_widget
			,  diag_info_text_buffer	/* initial text	    */
			);

	/* Clean up the memory we were handed from the GetString call */
	XtFree( old_buffer );

	/*
	 * Ungray the diagnostic info item on the menu if it
	 * hasn't been already.
	 */
	if (diag_info_button_on == FALSE)
	    {
	    set_sensitivity (diag_info_button, TRUE);
	    diag_info_button_on = TRUE;
	    }
	} /* end new text to write to the widget */
      else
	{
	  /*  currently, since the ps code does not pass back
	   *  a status directly, we have to catch it via a callback;
	   *  call the close routine, and switch the status to the
	   *  same status that a user would see if CDA could not open
	   *  a file for consistancy
	   */

	   /* pop up an error box if the status is not informational */
	   display_error(dvr_reason->status);

	}
      }  /* end CdaError case	*/

} /* end activate_proc */


/*
**++
**  ROUTINE NAME:
**	options_file_activate_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	display the op file selection box;
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void options_file_activate_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{
    Arg arg_list[5];
    int argcount;
    XmString cstr;

    if (options_file_sel_box == 0)
       {
       if (MrmFetchWidget(
	   dvr_DRMHierarchy,
	   "options_file_sel_box",
	   viewer,
	   & options_file_sel_box,
	   & dummy_class) != MrmSUCCESS)
      	  {
	  fprintf(stderr, dvr_drm_popup_ffail_str);
	  return;
      	  }
       else
          {
#ifdef CDA_HYPER_HELP
	     {
	     Widget		file_sel_child;
	     CDAstatus		ret_status;
	     String		help_topic;
	     /*
	     ** File selction widget has only one help callback. For implemnting
	     ** context sensitive help on APPLY, FILTER and Cancel buttons, help
	     ** callback resource along with the call back routine has to be added
	     ** added for those buttons.
	     */
             /* Set helpcallback on file cancel button */
	     file_sel_child = (Widget) XmFileSelectionBoxGetChild(options_file_sel_box,
				     (unsigned char) XmDIALOG_CANCEL_BUTTON);
	     ret_status = fetch_simple_string("dvr$help_opt_filter_but",&help_topic);
             if_error_report (ret_status);
	     set_hyper_help_cbs (file_sel_child, help_topic);

             /* Set helpcallback on file ok button */
	     file_sel_child = (Widget) XmFileSelectionBoxGetChild(options_file_sel_box,
				     (unsigned char) XmDIALOG_OK_BUTTON);
             ret_status = fetch_simple_string("dvr$help_opt_ok_but",&help_topic);
             if_error_report (ret_status);
	     set_hyper_help_cbs (file_sel_child, help_topic);

             /* Set helpcallback on file filter button */
	     file_sel_child = (Widget) XmFileSelectionBoxGetChild(options_file_sel_box,
				     (unsigned char) XmDIALOG_APPLY_BUTTON);
             ret_status = fetch_simple_string("dvr$help_opt_filter_but",&help_topic);
             if_error_report (ret_status);
	     set_hyper_help_cbs (file_sel_child, help_topic);

             /* Set helpcallback on filter label*/
	     file_sel_child = (Widget) XmFileSelectionBoxGetChild(options_file_sel_box,
				     (unsigned char) XmDIALOG_FILTER_LABEL);
             ret_status = fetch_simple_string("dvr$help_opt_file_filt",&help_topic);
             if_error_report (ret_status);
	     set_hyper_help_cbs (file_sel_child, help_topic);

             /* Set helpcallback on filter text*/
	     file_sel_child = (Widget) XmFileSelectionBoxGetChild(options_file_sel_box,
				     (unsigned char) XmDIALOG_FILTER_TEXT);
             ret_status = fetch_simple_string("dvr$help_opt_file_filt",&help_topic);
             if_error_report (ret_status);
	     set_hyper_help_cbs (file_sel_child, help_topic);

             /* Set helpcallback on file selection label*/
	     file_sel_child = (Widget) XmFileSelectionBoxGetChild(options_file_sel_box,
				     (unsigned char) XmDIALOG_SELECTION_LABEL);
             ret_status = fetch_simple_string("dvr$help_opt_file_box",&help_topic);
             if_error_report (ret_status);
	     set_hyper_help_cbs (file_sel_child, help_topic);

             /* Set helpcallback on file selection Text */
	     file_sel_child = (Widget) XmFileSelectionBoxGetChild(options_file_sel_box,
				     (unsigned char) XmDIALOG_TEXT);
             ret_status = fetch_simple_string("dvr$help_opt_file_box",&help_topic);
             if_error_report (ret_status);
	     set_hyper_help_cbs (file_sel_child, help_topic);
	     }
#endif
   	  if (strlen((char *) current_op_file_dir) != 0)
     	     {
	     argcount = 0;
	     cstr = XmStringLtoRCreate((char *) current_op_file_dir , "ISO8859-1");
       	     XtSetArg(arg_list[argcount], XmNdirMask, cstr);
       	     argcount++;
	     XtSetValues(options_file_sel_box, arg_list, argcount);
	     XtFree((char *)cstr);
     	     }

	  }
    }

    XtManageChild(options_file_sel_box);

    /* set value of widget */
    argcount = 0;
    cstr = XmStringLtoRCreate((char *) current_op_file , "ISO8859-1");
    XtSetArg(arg_list[argcount], XmNdirSpec, cstr);		/* was XmNvalue in XUI */
    argcount++;
    XtSetValues(options_file_sel_box, arg_list, argcount);
    XtFree ((char *)cstr);

    ScrBringToFront(options_file_sel_box);
}


/*
**++
**  ROUTINE NAME:
**	paper_size_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**	display the paper-size options dialog box
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**    	caddr_t		*tag;
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

static void paper_size_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{
    Arg arg_list[10];
    int arg_count = 0;

    /*  if the paper size dialog box has not yet
     *  been fetched (created), do so, and then map it
     */

   if (paper_size_dialog_box == 0) {

    /* allocate space for paper size strings */
    if (paper_height_str == 0)
	paper_height_str = XtMalloc(20);

    if (paper_width_str == 0)
	paper_width_str = XtMalloc(20);

    if (save_height_str == 0)
	save_height_str = XtMalloc(20);

    if (save_width_str == 0)
	save_width_str = XtMalloc(20);

    if (MrmFetchWidget (
	dvr_DRMHierarchy,
	"paper_size_dialog_box",
	viewer,
	& paper_size_dialog_box,
	& dummy_class) != MrmSUCCESS)
        {

	fprintf(stderr, dvr_drm_popup_ffail_str);
	return;
	}

    else
        {
	  if (dps_exists)
	      set_ps_initial_values();
	  else
	    {
	      /* no dps in server, disable all extra ps specific options */
	      int i;
	      XtSetSensitive(ps_op_id, FALSE);
	      XtSetSensitive(ps_scale_id, FALSE);

	      for (i=0; i<4; i++)
	          XtSetSensitive(orientGroup[i], FALSE);
	    }

        }

    }

   /* save selected item in list box in case of cancel */
   save_size_index = current_size_index;
   save_size_unit  = current_size_unit;
   save_height_str = (char *) XmTextGetString( size_height_text_id );
   save_width_str  = (char *) XmTextGetString( size_width_text_id );

   if (dps_exists)
     {
	int 	i;
	char 	*file_format_str;
	Boolean viewing_ps = FALSE;
	Arg 	arg_list[3];

   	save_use_comments = XmToggleButtonGetState(ps_comments_id);
 	save_use_bwidths  = XmToggleButtonGetState(ps_bwidths_id);
   	save_use_trays    = XmToggleButtonGetState(ps_fake_trays_id);
   	save_draw_mode    = XmToggleButtonGetState(ps_draw_mode_id);

	for (i=0; i<4; i++)
 	  {
	    if (XmToggleButtonGetState(orientGroup[i]))
		save_orient_widget = orientGroup[i];
	  }

	(void) XmScaleGetValue(ps_scale_id, &save_scale);

	/*  if the current format is PS, enable ps-specific options;
	 *  else enable reformat-toggle (specific to cda viewing)
	 */
	file_format_str = (char *) XmTextGetString(format_text_id);
#ifdef __vms__
	upper_case(file_format_str);
#endif

	if (strcmp(file_format_str, POSTSCRIPT_STR) == 0)
	    viewing_ps = TRUE;

	XtSetArg(arg_list[0], XmNsensitive, viewing_ps);

	/*  postscript specific options include the scale widget, and
 	 *  the orientation radio box; set them all insensitive
	 */

	XtSetSensitive(ps_op_id, viewing_ps);			/* Note XtSetSensitive wants sensitive = True */
	XtSetSensitive(ps_scale_id, viewing_ps);		/* Note XtSetSensitive wants sensitive = True */

	for (i=0; i<4; i++)
	    XtSetSensitive(orientGroup[i], viewing_ps);		/* Note XtSetSensitive wants sensitive = True */

	XtSetSensitive(reform_toggle_id, !viewing_ps);		/* Note XtSetSensitive wants sensitive = True */
	XtFree(file_format_str);

      }



   XtManageChild(paper_size_dialog_box);
   ScrBringToFront( paper_size_dialog_box);
}



/*
**++
**  ROUTINE NAME:
**	display_error(status)
**
**  FUNCTIONAL DESCRIPTION:
**      put up a message box for the status passed in. On VMS, the
**	message utility is used; On ultrix, compound strings are used.
**
**
**  FORMAL PARAMETERS:
**    	unsigned int status;
**
**  IMPLICIT INPUTS:
**      none
**
**  FUNCTION VALUE:
**      none
**
**  SIDE EFFECTS:
**      none
**--
**/

static void display_error(status)
    CDAstatus status;


{
   Arg		arg_list[3];
   int		argcount 	= 0;
   CDAstatus	ret_status;

#ifdef __vms__
   int     message_struct[2];
   Boolean our_message 	= TRUE; /* tells if this is a DVR message */
#endif

   char message_drm_str[100];
   XmString cstr;

   ret_status = get_message_text( status,
				  message_drm_str,
				  FALSE,
				  FALSE);

   if (ret_status != DVR_NORMAL)
        return;

#ifdef __vms__
   if (!strcmp(message_drm_str, "dvr$default_error_str"))
      {
        /*  if this is not our message, use VMS message
	 *  routine to display;
	 */
    	message_struct[0] = 1;
    	message_struct[1] = status;
	our_message = FALSE;
      }
#endif

    /* if we have a fatal error, set flag to exit appliccation. */

    if (status == DVR_FATALERROR)
	fatal_flag = 1;
    else
	fatal_flag = 0;

    /* display the message */
/*
 * The following code has been removed and place into the non-VMS section
 * since there seems to be a bug in the DXmDisplayVmsMessage routine such
 * that it doesn't properly handle widgets that are already setup.
 *
 * HACK HACK HACK HACK
 *
 *
 *   if (viewer_message == 0) {
 *
 *    if (MrmFetchWidget (
 *	dvr_DRMHierarchy,
 *	"viewer_message",
 *	viewer,
 *	& viewer_message,
 *	& dummy_class) != MrmSUCCESS)
 *        {
 *
 *	fprintf(stderr, dvr_drm_popup_ffail_str);
 *	return;
 *	}
 *
 *    }
 *
 * End of the code that had to be blocked out.
*/

#ifdef __vms__
    if (!our_message)
	{
/*
 * The following unmanage was added due to a problem with DXmDisplayVmsMessage
 * not properly handling widgets that are already in existance.
 *
*/

	if (viewer_message != 0)			/* Unmanage the existing viewer message to avoid	*/
	    XtUnmanageChild(viewer_message);		/* problems with the VMS message routine.		*/

	viewer_message = 0;				/* DXmDisplayVmsMessage likes this parameter to be zero	*/

	/*
	 * Display the message and parameters associated with the
	 * message vector.
	*/

        DXmDisplayVmsMessage(viewer, 			/* Widget parent  */
			     dvr_message_title, 	/* Name		  */
			     1, 			/* Pos		  */
			     0, 			/* X		  */
			     0,				/* Y		  */
			     XmDIALOG_MODELESS,		/* Style	  */
			     message_struct, 		/* Message Vector */
			     &viewer_message, 		/* Widget id	  */
			     NULL,			/* User Routine   */
			     mess_cb, 			/* OK Callback    */
			     NULL);			/* Help Callback  */

	/*
	 * Fix the title string since the DXmDisplayVmsMessage routine
	 * insists on putting _popup on the end of the name.
	*/

	    {
	     Arg arg_list[3];

	     cstr = XmStringLtoRCreate(dvr_message_title, "ISO8859-1");
	     XtSetArg(arg_list[0], XmNdialogTitle, cstr);
	     XtFree(cstr);

	     XtSetValues(viewer_message, arg_list, 1);
	    }
	}
    else
#endif

      {
	Arg arg_list[3];
	int arg_count = 0;

       /*
	* The following code fragment was moved from the above section since
	* it is only used if we don't have a VMS message vector.
       */

       if (viewer_message == 0)
	{
	 Widget cancel_button;			/* Cancel button widget */

	 if (MrmFetchWidget (dvr_DRMHierarchy, "viewer_message", viewer, &viewer_message,&dummy_class) != MrmSUCCESS)
	     {
	     fprintf(stderr, dvr_drm_popup_ffail_str);
	     return;
	     }

	 /*
	  * If we were able to fetch the widget get rid of the cancel
	  * button since we don't need it.
	 */

	 cancel_button = (Widget) XmMessageBoxGetChild(viewer_message, XmDIALOG_CANCEL_BUTTON);
	 XtUnmanageChild(cancel_button);

	}		 /* End of the moved code segment */

       XtSetArg(arg_list[arg_count], XmNmessageString, message_drm_str);
       arg_count++;

       MrmFetchSetValues(dvr_DRMHierarchy,		/* Hierarchy to be searched 		*/
			 viewer_message,		/* Widget to receive the value found 	*/
			 arg_list,			/* List of items to be searched for	*/
			 arg_count);			/* Number of items to search for	*/
       }

    if (XtIsRealized(viewer))
        XtManageChild(viewer_message);

}



/*
**++
**  ROUTINE NAME:
**	stext_cr_proc (w)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure is called after the user hits return in one of the
**	simple text widgets within one of the pop-up dialog boxes;
**      If the routine was called for the format text box, try to
**	match the new format with a matching directory mask filter.
**
**	if the routine is called via one of the other widgets (e.g.
**	paper size width and height stexts), do nothing, we just want
**	to trap the <CR> so the stext does not grow.
**
**
**  FORMAL PARAMETERS:
**    	Widget		w;
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
static void stext_cr_proc (w)
    Widget		w;


{

    if ( (w == format_text_id) || (w == format_list_id) )
      {
	char			*new_format;
	Arg 			arg_list[2];
	XmString	 	cstring_dir_mask;
	XmString	 	cstring_file_select;
#ifdef CDA_TWOBYTE
	XmString                cstring_file_select_2;
#endif
	char			*char_set;
    	int			dont_care;
    	char    		*filter_str;
    	char    		*file_spec;
    	XmStringContext 	context;
    	char    		dir_string[256];
    	char    		file_string[80];
	XmString	 	cstr;

	new_format = (char *) XmTextGetString(format_text_id);

        dir_string[0] = '\0';
        file_string[0] = '\0';
#ifdef __vms__
	if ( w == format_text_id )
	    upper_case( new_format);
#endif
	/* get current value of dir mask and dir sepc */

 	/*  if the dir mask filter has not yet changed, then
	*  see if the newly entered format has a matching dir
	*  mask filter, if so, change it in open file sel box
	*/
  	XtSetArg( arg_list[0], XmNdirMask, & cstring_dir_mask );
	XtGetValues( file_sel_box, arg_list, 1);

    	/* get null terminated string from compound string */
    	XmStringInitContext( & context, cstring_dir_mask );

    	XmStringGetNextSegment ( context,
				 & filter_str,
				 & char_set,
				 (XmStringDirection *) & dont_care,
				 (Boolean *) & dont_care
			       );

	XtFree(char_set);
        /* get the name of the current file selected, this is work around to
        *  get the file name as there is a bug in the file seletction widget
        */
#ifdef CDA_TWOBYTE
	/* The child of Selection is CSText in Japanese Xm. */
	XtSetArg (arg_list[0], XmNtextString, &cstring_file_select);
	XtGetValues (file_sel_box, arg_list, 1);
#else
        file_spec = (char *) XmTextGetString(XmSelectionBoxGetChild (file_sel_box,
							   XmDIALOG_TEXT));
#endif
#ifdef __vms__
	upper_case(filter_str);
#endif
        strip_file_spec (filter_str, dir_string, file_string);

        /* Change The Filter and retain the same directory specification */
	change_filter(new_format, dir_string, file_string);

        /* Set the name of the current file selected, this is work around to
        *  set the file name as there is a bug in the file seletction widget
        */
#ifdef CDA_TWOBYTE
	/* Need XmStringCopy(), because SelectionBox will not update if
	   the address of XmString is the same. */
	cstring_file_select_2 = XmStringCopy (cstring_file_select);

	/* The child of Selection is CSText in Japanese Xm. */
	XtSetArg (arg_list[0], XmNtextString, cstring_file_select_2);
	XtSetValues (file_sel_box, arg_list, 1);
	XmStringFree (cstring_file_select);
	XmStringFree (cstring_file_select_2);
#else
        XmTextSetString(XmSelectionBoxGetChild (file_sel_box, XmDIALOG_TEXT),
 			file_spec);
#endif

        /* Free Memory Allocated For the Strings */
	XtFree(filter_str);
	XtFree((char *)context);

	/* select list box if the format was entered in the stext */

	if (w == format_text_id)
	  {
            cstr = XmStringLtoRCreate(new_format , "ISO8859-1"),
            (void) XmListSelectItem(format_list_id, cstr, FALSE);
	    XtFree((char *)cstr);
	  }

	XtFree(new_format);

      } /* end w == x or y */

}


#ifdef __vms__
/*
**++
**  ROUTINE NAME:
**	upper_case(input_str)
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure converts a lower case string to upper case;
**	note, this is only used on VMS to match up format names to
**      their corresponding dir mask filter names; everything is
**	case sensitive on ultrix, so this is not used there.
**
**
**  FORMAL PARAMETERS:
**    	char	*input_str;
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

void upper_case(input_str)
    char *input_str;

{
    int i;

    for (i=0; i<strlen(input_str); i++)

	/*  for each character, if it is lower case (ascii 97-122),
	 *  then convert it to upper case (subtract 32)
	 */

	if ( (input_str[i] > 96) &&
	     (input_str[i] < 123) )
	    input_str[i] = input_str[i] - 32;

}

#endif



/*
**++
**  ROUTINE NAME:
**  change_filter(new_format, dir_string, file_string)
**
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure attempts to match a new format to a dir mask
**	filter based on the table generated from UIL; if a match is found,
**	then the filter is updated to correspond to the new format;
**      If a match is not found, then this must be a format not currently
**	on the base kit or the converter kit, use *.ddif (default)
**
**
**  FORMAL PARAMETERS:
**    	char	*new_format;
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

void change_filter(new_format, dir_string, file_string)
    char * new_format;
    char * dir_string;
    char * file_string;

{
    int index;
    Arg arg_list[2];
    char format_str[31];
    char * dot_pos;
    XmString cstr;

    /* Initialize the format string */
    format_str[0] = '\0';
    for (index=0; index<NUM_FORMATS; index++)
      {
	if (strcmp(format_filter_table[index].format, new_format) == 0)
    	  {
	    /* match is found! set dir mask in open file sel box */

            /* if there is no file name then have * as a file name */
            if ( strlen (file_string) == 0)
                strncat(dir_string, format_filter_table[index].filter,
                        strlen( format_filter_table[index].filter ));
            else
               {
               /* conacatenate the directory string and the file string */
    	       strncat(dir_string, file_string, strlen(file_string));
               strncat(dir_string, ".", 1);
               dot_pos = strrchr(format_filter_table[index].filter, '.');
               strcpy(format_str, dot_pos+1);
               strncat(dir_string, format_str, strlen( format_str ));
               }
	    cstr = XmStringLtoRCreate(dir_string , "ISO8859-1");
	    XtSetArg(arg_list[0], XmNdirMask, cstr);

	    XtSetValues(file_sel_box, arg_list, 1);
	    XtFree ((char *)cstr);

	    strcpy((char *) current_dir_mask, (char *) dir_string);

#ifdef __vms__
	    /*  on vms, always store dir mask in upper case for
	     *  string compares
	     */
	    upper_case(current_dir_mask);
#endif

	    return;
	  }
      }

    /*  if we get here, then no match was found, use "ddif" (default)
     *  note, "ddif" is the 5th entry in the table
     */
    if ( strlen (file_string) == 0)
       {
       /* There is no file name hence have a default vlaue of "*.ddif"  */
       cstr = XmStringLtoRCreate(format_filter_table[5].filter , "ISO8859-1");
       XtSetArg(arg_list[0], XmNdirMask, cstr);
       strcpy((char *) current_dir_mask, (char *) format_filter_table[5].filter);
       XtFree ((char *)cstr);
       }
    else
       {
       /* conacatenate the directory string and the file string */
       strncat(dir_string, file_string, strlen(file_string));
       strncat(dir_string, ".", 1);
       /*  The file name has a string length > 0 i.e file name is given but
        *  file format is not a valid format, hence extract the defualt file
        *  fromat from format_filter_table.
 	*/
       dot_pos = '\0';
       dot_pos = strrchr(format_filter_table[5].filter, '.');
       strcpy(format_str, dot_pos+1);
       strncat(dir_string, format_str, strlen( format_str ));

       cstr = XmStringLtoRCreate(dir_string , "ISO8859-1");
       XtSetArg(arg_list[0], XmNdirMask, cstr);
       strcpy((char *) current_dir_mask, (char *) dir_string);
       XtFree((char *)cstr);
       }

    XtSetValues(file_sel_box, arg_list, 1);

    XtFree(file_string);

#ifdef __vms__
    /*  on vms, always store dir mask in upper case for
     *  string compares
     */
    upper_case(current_dir_mask);
#endif
}


/*
**++
**  ROUTINE NAME:
**	build_format_filter_table()
**
**  FUNCTIONAL DESCRIPTION:
**      a static table is filled in by fetching matching format/filter
**	string pairs from UIL; these pairs represent all of the formats
**      supported on the base kit and on the converter lib kit.
**
**	NOTE: this table relies on the fact that "*.ddif", the default
**	      is the 5th entry in the table; if the table is updated, and
**	      "*.ddif" is no longer the 5th entry, then this needs to
**	      be updated in a few spots in this module.
**
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
build_format_filter_table()

{

    CDAstatus	ret_status;

    /*  first fetch all of the filters; these are all the same for
     *  vms and ultrix except for the calcgrd filter string;
     *  then fetch all of the format strings; these are all case sensitive
     *  so different strings are fetched for VMS and ultrix
     */

    ret_status = fetch_compound_string("dvr$afs_filter_str",
			&format_filter_table[0].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$tab_filter_str",
			&format_filter_table[1].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

#ifdef __vms__
    ret_status = fetch_compound_string("dvr$vms_calc_filter_str",
			&format_filter_table[2].filter);
#else
    ret_status = fetch_compound_string("dvr$ult_calc_filter_str",
			&format_filter_table[2].filter);
#endif
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$cgm_filter_str",
			&format_filter_table[3].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$dca_filter_str",
			&format_filter_table[4].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ddif_filter_str",
			&format_filter_table[5].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$dif_filter_str",
			&format_filter_table[6].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$dtif_filter_str",
			&format_filter_table[7].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$dx_filter_str",
			&format_filter_table[8].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$macp_filter_str",
			&format_filter_table[9].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$sgml_filter_str",
			&format_filter_table[10].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$text_filter_str",
			&format_filter_table[11].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$tiff_filter_str",
			&format_filter_table[12].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$wk1_filter_str",
			&format_filter_table[13].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ps_filter_str",
			&format_filter_table[14].filter);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

#ifdef __vms__
    ret_status = fetch_compound_string("dvr$vms_afs_format_str",
			&format_filter_table[0].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_tab_format_str",
			&format_filter_table[1].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_calc_format_str",
			&format_filter_table[2].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_cgm_format_str",
			&format_filter_table[3].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_dca_format_str",
			&format_filter_table[4].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_ddif_format_str",
			&format_filter_table[5].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_dif_format_str",
			&format_filter_table[6].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_dtif_format_str",
			&format_filter_table[7].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_dx_format_str",
			&format_filter_table[8].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_macp_format_str",
			&format_filter_table[9].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_sgml_format_str",
			&format_filter_table[10].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_text_format_str",
			&format_filter_table[11].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_tiff_format_str",
			&format_filter_table[12].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_wk1_format_str",
			&format_filter_table[13].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_ps_format_str",
			&format_filter_table[14].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);
#else
    ret_status = fetch_compound_string("dvr$ult_afs_format_str",
			&format_filter_table[0].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_tab_format_str",
			&format_filter_table[1].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_calc_format_str",
			&format_filter_table[2].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_cgm_format_str",
			&format_filter_table[3].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_dca_format_str",
			&format_filter_table[4].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_ddif_format_str",
			&format_filter_table[5].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_dif_format_str",
			&format_filter_table[6].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_dtif_format_str",
			&format_filter_table[7].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_dx_format_str",
			&format_filter_table[8].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_macp_format_str",
			&format_filter_table[9].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_sgml_format_str",
			&format_filter_table[10].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_text_format_str",
			&format_filter_table[11].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_tiff_format_str",
			&format_filter_table[12].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_wk1_format_str",
			&format_filter_table[13].format);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    	ret_status = fetch_compound_string("dvr$ult_ps_format_str",
			&format_filter_table[14].format);
    	if (ret_status != DVR_NORMAL)
	    return(ret_status);
#endif


    /* use DDIF (index 5) as default */
    strcpy((char *) current_dir_mask, (char *) format_filter_table[5].filter);

#ifdef __vms__
    /*  on vms, always store dir mask in upper case for
     *  string compares
     */
    upper_case(current_dir_mask);
#endif

    return(DVR_NORMAL);
}


/*
**++
**  ROUTINE NAME:
**	free_format_filter_table (w)
**
**  FUNCTIONAL DESCRIPTION:
**      free the format filter matching table; all of these strings
**	were fetched from UIL
**
**
**  FORMAL PARAMETERS:
**    	none;
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
free_format_filter_table()

{
    int i;

    for (i=0; i<NUM_FORMATS; i++)
      {
	XtFree(format_filter_table[i].format);
	XtFree(format_filter_table[i].filter);
      }
}


/*
**++
**  ROUTINE NAME:
**	fetch_compound_string(drm_str, null_term_str)
**
**  FUNCTIONAL DESCRIPTION:
**      given a DRM string : drm_str, fetch the corresponding
**	compound string from UIL; extract the null terminated
**	string from the compound string, and return it as : null_term_str;
**
**  FORMAL PARAMETERS:
**    	char *drm_str;
**    	char **null_term_str;
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

CDAstatus fetch_compound_string(drm_str, null_term_str)
    char *drm_str;
    char **null_term_str;
{
    XmString		  cstring;
    char		  *char_set;
    int			  dont_care;
    XmStringContext  	  cs_context;
    CDAstatus		  ret_status;
    char 		  *ret_string;
    MrmCode		  drm_code;

    /* fetch the string */

    ret_status = MrmFetchLiteral(
		   dvr_DRMHierarchy,
		   drm_str,
		   XtDisplay(toplevel),
		   (caddr_t *) &cstring,
		   &drm_code);

    if (ret_status != MrmSUCCESS)
	return(ret_status);

    /* retrieve the null terminated string */
    XmStringInitContext(&cs_context, cstring);

    XmStringGetNextSegment(cs_context, &ret_string, &char_set,		/* Note context is now by value in Motif */
		      	   (XmStringDirection *) &dont_care, (Boolean *) &dont_care);

    *null_term_str = ret_string;

    /* free memory */

    XtFree(char_set);							/* Character set is now an ASCIZ string in Motif */

    /* free context */
    XtFree((char *)cs_context);

    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	fetch_simple_string(drm_str, sstring)
**
**  FUNCTIONAL DESCRIPTION:
**      given a DRM string : drm_str, fetch the corresponding
**	simple string from UIL; extract the null terminated
**	string from the simple string, and return it as : null_term_str;
**
**  FORMAL PARAMETERS:
**    	char *drm_str;
**    	String sstring
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

CDAstatus fetch_simple_string(drm_str, sstring)
    char    * drm_str;
    String  * sstring;
{
    CDAstatus		  ret_status;
    char 		  *ret_string;
    MrmCode		  drm_code;

    /* fetch the string */

    ret_status = MrmFetchLiteral(
		   dvr_DRMHierarchy,
		   drm_str,
		   XtDisplay(toplevel),
		   (caddr_t) sstring,
		   &drm_code);

    if (ret_status != MrmSUCCESS)
	return(ret_status);



    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	get_message_text(status, message_buffer, get_vms_msg, return_text)
**
**  FUNCTIONAL DESCRIPTION:
**	given a status code, return the associated ASCIZ text
**	message in the message buffer.
**
**  FORMAL PARAMETERS:
**    status		: status to get string for
**    message_buffer    : buffer to return string that matches status
**    return_text       : if true, will return text corresponding to status
**			  else, will return drm_string corresponding to
**			  status
**    get_vms_msg       : if true, call VMS MSG facility to retrieve message
**			  text iff we cannot find a match for the status in
**			  our list.
**
**  IMPLICIT INPUTS:
**      none
**
**  FUNCTION VALUE:
**      none
**
**  SIDE EFFECTS:
**      none
**--
**/
CDAstatus get_message_text( status, message_buffer, return_text, get_vms_msg )

    CDAstatus	 status;
    char	 message_buffer[MAX_MESSAGE_LENGTH];/* diagnostic message	*/
    Boolean 	 return_text;
    Boolean      get_vms_msg;
{
    short int	message_length;
    CDAstatus	ret_status;
    char	drm_string[100];
    char	*ret_string;

    /*
     *  find the matching drm string for this status
     */
    switch (status)
	{
	case DVR_ALREADYWIDGET:
	    strcpy(drm_string, "dvr$alreadywidget_str");
	    break;
	case DVR_ARRNOTSUP:
	    strcpy(drm_string, "dvr$arrnotsup_str");
	    break;
	case DVR_BADCOMMENTS:
	    strcpy(drm_string, "dvr$badcomments_str");
	    break;
	case DVR_BADFRAMETYPE:
	    strcpy(drm_string, "dvr$badframetype_str");
	    break;
	case DVR_DDIFERR:
	    strcpy(drm_string, "dvr$ddiferr_str");
	    break;
	case DVR_DEFAULTFONT:
	    strcpy(drm_string, "dvr$defaultfont_str");
	    break;
	case DVR_EOC:
	    strcpy(drm_string, "dvr$eoc_str");
	    break;
	case DVR_EOD:
	    strcpy(drm_string, "dvr$eod_str");
	    break;
	case DVR_FATALERROR:
	    strcpy(drm_string, "dvr$fatalerror_str");
	    break;
#ifdef __vms__
	case RMS$_DNF:
	    strcpy(drm_string, "dvr$dirfail_str");
	    break;

	case RMS$_FNF:
#endif
	case DVR_FILENOTFOUND:
	    strcpy(drm_string, "dvr$filenotfound_str");
	    break;
	case DVR_FILENOTOPEN:
	    strcpy(drm_string, "dvr$filenotopen_str");
	    break;
	case DVR_FORMATERROR:
	    strcpy(drm_string, "dvr$formaterr_str");
	    break;
	case DVR_FORMATINFO:
	    strcpy(drm_string, "dvr$formatinfo_str");
	    break;
	case DVR_FORMATWARN:
	    strcpy(drm_string, "dvr$formatwarn_str");
	    break;
	case DVR_GRAPHICFAIL:
	    strcpy(drm_string, "dvr$graphicfail_str");
	    break;
	case DVR_IMAGEFAIL:
	    strcpy(drm_string, "dvr$imagefail_str");
	    break;
	case DVR_INTERNALERROR:
	    strcpy(drm_string, "dvr$internalerror_str");
	    break;
	case DVR_INVALREQ:
	    strcpy(drm_string, "dvr$invalreq_str");
	    break;
	case DVR_INVFILETYPE:
	    strcpy(drm_string, "dvr$invfiletype_str");
	    break;
        case DVR_MEMALLOFAIL:
	    strcpy(drm_string, "dvr$memallofail_str");
	    break;
	case DVR_MEMDEALLOFAIL:
	    strcpy(drm_string, "dvr$memdeallofail_str");
	    break;
	case DVR_NOCONVERTER:
	    strcpy(drm_string, "dvr$noconverter_str");
	    break;
	case DVR_NODISPCONT:
	    strcpy(drm_string, "dvr$nodispcont_str");
	    break;
	case DVR_NODPSEXT:
	    strcpy(drm_string, "dvr$nodpsext_str");
	    break;
	case DVR_NOFONT:
	    strcpy(drm_string, "dvr$nofont_str");
	    break;
	case DVR_NOPAGE:
	    strcpy(drm_string, "dvr$nopage_str");
	    break;
	case DVR_NOTBITONAL:
	    strcpy(drm_string, "dvr$notbitonal_str");
	    break;
	case DVR_NOTDDIFDOC:
	    strcpy(drm_string, "dvr$notddifdoc_str");
	    break;
	case DVR_OPENFAIL:
	    strcpy(drm_string, "dvr$openfail_str");
	    break;
	case DVR_PAGENOTFOUND:
	    strcpy(drm_string, "dvr$pagenotfound_str");
	    break;
	case DVR_SCRFULL:
	    strcpy(drm_string, "dvr$scrfull_str");
	    break;
	case DVR_TEXTFAIL:
	    strcpy(drm_string, "dvr$textfail_str");
	    break;
	case DVR_TOPOFDOC:
	    strcpy(drm_string, "dvr$topofdoc_str");
	    break;
	case DVR_UNKOBJTYPE:
	    strcpy(drm_string, "dvr$unkobjtype_str");
	    break;
	case DVR_UNKSTRTYPE:
	    strcpy(drm_string, "dvr$unkstrtype_str");
	    break;
	case CDA_OPENFAIL:
	    strcpy(drm_string, "dvr$cda_openfail_str");
	    break;
	case CDA_INVDOC:
	    strcpy(drm_string, "dvr$cda_invdoc_str");
	    break;
	case CDA_INVITMLST:
	    strcpy(drm_string, "dvr$cda_invitmlst_str");
	    break;
	case CDA_UNSUPFMT:
	case CDA_ICVNOTFND:
	    strcpy(drm_string, "dvr$cda_unsupfmt_str");
	    break;
	case CDA_READFAIL:
	    strcpy(drm_string, "dvr$cda_readfail_str");
	    break;

	default:

#ifdef __vms__
	    if (return_text && get_vms_msg)
	      {
		struct	dsc$descriptor_s messg_buf_descrip;

    		messg_buf_descrip.dsc$w_length = MAX_MESSAGE_LENGTH;
    		messg_buf_descrip.dsc$b_dtype = DSC$K_DTYPE_T;
    		messg_buf_descrip.dsc$b_class = DSC$K_CLASS_S;
    		messg_buf_descrip.dsc$a_pointer = message_buffer;

    		/* Have VMS fetch the message text for us	*/
    		ret_status = SYS$GETMSG( status
			   , &message_length
			   , &messg_buf_descrip
			   , 0x0000000F		/* facility, sever, &text*/
			   , 0			/* no optional info back */
			   );

    		/*  null terminate the message string    */
    		if (SEVERITY(ret_status) == SUCCESS_STATE)
		  {
		    message_buffer[message_length++] = '\n';
		    message_buffer[message_length] = '\0';
		  }
    		return(ret_status);
	      }
	    else
#endif
	        strcpy(drm_string, "dvr$default_error_str");

	    break;

	} /* end translate switch   */

    if (return_text)
      {
	/* return the actual string that corresponds to the drm string */
    	ret_status = fetch_compound_string(drm_string, &ret_string);

        if (ret_status != DVR_NORMAL)
	    return(ret_status);

        strcpy(message_buffer, ret_string);
        XtFree(ret_string);
      }
    else
	/* just return the drm string */
	strcpy(message_buffer, drm_string);

    return (DVR_NORMAL);
} /* end get_message_text   */




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	set_watch_cursor(vw)
**	 - If widget has not yet had a watch cursor created for it;
**	   make it;
**	 - call macro to set cursor to watch
**
**  FORMAL PARAMETERS:
**
**  vw  - viewer widget
**
**--
*/
void set_watch_cursor(vw)
    Widget vw;


{
    /* pointer to the viewer part of Widget */

    if ( WatchCursor == (Cursor) NULL) {

	/* create watch cursor and store in widget */
	unsigned int cursorfont;
	XColor close_white, close_black, black, white;
	int stat;
	XFontStruct *cursor_font_struct;

	/* load the cursor font */
	cursorfont = XLoadFont(XtDisplay(vw),
	             dvr_cursor_font_file);

	/* verify the font */
	cursor_font_struct = XQueryFont(XtDisplay(vw), cursorfont);

	/* get black and white color maps from server for watch */

	stat = XLookupColor(XtDisplay(vw),
	 XDefaultColormap(XtDisplay(vw), 0),
	  dvr_cursor_bg_color, &close_black, &black);

	stat = XLookupColor(XtDisplay(vw),
	       XDefaultColormap(XtDisplay(vw), 0),
	       dvr_cursor_fg_color, &close_white, &white);

	/* if we could not open font, use X watch, else use decw watch */

	if (cursor_font_struct == NULL)

	    WatchCursor =
	      XCreateFontCursor(XtDisplay(vw),
		   XC_watch);
	else
          {
	    WatchCursor =
	      XCreateGlyphCursor(XtDisplay(vw),
		 cursorfont, cursorfont,
	        decw$c_wait_cursor, decw$c_wait_cursor + 1, &close_white,
	      &close_black);

	    XFreeFont(XtDisplay(vw), cursor_font_struct);
	  }

    }

    /* call macro to change cursor to watch */
    StartMainWait(vw);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	reset_cursor(vw)
**	 - calls macro to reset cursor to original value
**
**  FORMAL PARAMETERS:
**
**  vw - widget
**
**--
*/
void reset_cursor(vw)

    Widget vw;		/* pointer to the viewer Widget */

{
    StopMainWait(vw);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	dir_file_scan(in_string, dir_string, file_ext)
**	 - return the directory specified in in_string in dir_string
**
**
**--
*/
void dir_file_scan(in_string, dir_string, file_ext)
    char 	*in_string;
    char	*dir_string;
    char	*file_ext;

{
#ifdef __vms__
    if (strlen(in_string) != 0)
	{
	  /* get the directory specified to pass to
	   * the file selection widget
	   */
	  unsigned long retstat;
	  unsigned long fldflags;
	  struct dsc$descriptor file$dsc;
	  struct
	    {
	      unsigned short int component_length;
	      unsigned short int item_code;
	      unsigned int	 component_address;
	    } item_list[4];

	  file$dsc.dsc$a_pointer = in_string;
	  file$dsc.dsc$w_length  = strlen(in_string);
	  file$dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	  file$dsc.dsc$b_class   = DSC$K_CLASS_S;

	  /* ask VMS for the file's device and directory */
	  item_list[0].item_code = FSCN$_DEVICE;
	  item_list[1].item_code = FSCN$_DIRECTORY;
	  item_list[2].item_code = FSCN$_TYPE;

	  item_list[3].component_length = 0;
	  item_list[3].item_code = 0;
	  item_list[3].component_address = 0;
          retstat = SYS$FILESCAN(&file$dsc, item_list, &fldflags);

	  if (retstat == SS$_NORMAL)
	    {
	      /* make the directory out of the device and the
	       * directory returned
	       */
	      strncat(dir_string, (char *) item_list[0].component_address,
			item_list[0].component_length);
	      strncat(dir_string, (char *) item_list[1].component_address,
			item_list[1].component_length);
	      strcat(dir_string, "*");
	      if (item_list[2].component_length != 0)
		strncat(dir_string, (char *) item_list[2].component_address,
			item_list[2].component_length);
	      else
		strcat(dir_string, file_ext);

	    }

	}
#endif

#ifdef __unix__
    char *char_pos;
    int  dir_len;
    char new_file_ext[256];

    /* get default file ext */
    strcpy(new_file_ext, file_ext);

    dir_string[0] = '\0';

    /* find final backslash, this is the end of the directory */
    char_pos = strrchr(in_string, '/');

    /* if there is a final backslash, get directory */
    if (char_pos != 0)
      {
	char *dot_pos;

	/* generate default filter based on extention provided */
	dot_pos = strrchr(char_pos, '.');
	if (dot_pos)
	    strcpy(new_file_ext, dot_pos);

    	dir_len = char_pos - in_string + 1;
	strncpy(dir_string, in_string, dir_len);
      }
    else
      {
	char *dot_pos;

	/* generate default filter based on extention provided */
	dot_pos = strrchr(in_string, '.');
	if (dot_pos)
	    strcpy(new_file_ext, dot_pos);
      }

    /* cat with *.whatever */
    strcat(dir_string, "*");
    strcat(dir_string, new_file_ext);

#endif  /* ultrix */

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	strip_directory(in_string, out_string)
**	 - take a string representing a file spec and return
**	   a string representing just the file; strip the directory
**	   and extention.
**
**
**--
*/
void strip_directory(in_string, out_string)
    char 	*in_string;
    char	*out_string;

{
#ifdef __vms__
    if (strlen(in_string) != 0)
	{
	  /* get the file name specifid (without directory)
	   */
	  unsigned long retstat;
	  unsigned long fldflags;
	  struct dsc$descriptor file$dsc;
	  struct
	    {
	      unsigned short int component_length;
	      unsigned short int item_code;
	      unsigned int	 component_address;
	    } item_list[3];

	  /* initially, the out string is null */
	  out_string[0] = '\0';

	  file$dsc.dsc$a_pointer = in_string;
	  file$dsc.dsc$w_length  = strlen(in_string);
	  file$dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	  file$dsc.dsc$b_class   = DSC$K_CLASS_S;

	  /* ask VMS for the file's name */
	  item_list[0].item_code = FSCN$_NAME;
	  item_list[1].item_code = FSCN$_TYPE;

	  item_list[2].component_length = 0;
	  item_list[2].item_code = 0;
	  item_list[2].component_address = 0;
          retstat = SYS$FILESCAN(&file$dsc, item_list, &fldflags);

	  if (retstat == SS$_NORMAL)
	    {
	      /* make the directory out of the device and the
	       * directory returned
	       */
	      strncat(out_string, (char *) item_list[0].component_address,
			item_list[0].component_length);
	      strncat(out_string, (char *) item_list[1].component_address,
			item_list[1].component_length);

	    }

	}
#endif

#ifdef __unix__
    char *char_pos;
    int  dir_len;

    /* find final backslash, this is the start of the filename */
    char_pos = strrchr(in_string, '/');

    /* if there is a final backslash, strip directory */
    if (char_pos != 0)
      {
	/* add one to get past final backslash */
	char_pos = char_pos + 1;

	strcpy(out_string, char_pos);
      }
    else
	/*  else the string contains no directory,
	 *  use full string as file name
	 */
	strcpy(out_string, in_string);

#endif  /* ultrix */

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	strip_directory(in_string, dir_string, file_string)
**	 - take a string representing a file spec (full path) and splits the
**	   the input file spec into a directory string ( with device name ),
**	   file string and extention string.
**
**
**--
*/
static void
strip_file_spec (in_string, dir_string, file_string)
    char 	*in_string;
    char	*dir_string;
    char	*file_string;

{
#ifdef __vms__
    if (strlen(in_string) != 0)
	{
	  /* get the file name specifid (without directory)
	   */
	  unsigned long retstat;
	  unsigned long fldflags;
	  struct dsc$descriptor file$dsc;
	  struct
	    {
	      unsigned short int component_length;
	      unsigned short int item_code;
	      unsigned int	 component_address;
	    } item_list[4];

	  file$dsc.dsc$a_pointer = in_string;
	  file$dsc.dsc$w_length  = strlen(in_string);
	  file$dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	  file$dsc.dsc$b_class   = DSC$K_CLASS_S;

	  /* ask VMS for the file's name */
	  item_list[0].item_code = FSCN$_DEVICE;
	  item_list[1].item_code = FSCN$_DIRECTORY;
	  item_list[2].item_code = FSCN$_NAME;

	  item_list[3].component_length = 0;
	  item_list[3].item_code = 0;
	  item_list[3].component_address = 0;
          retstat = SYS$FILESCAN(&file$dsc, item_list, &fldflags);

	  if (retstat == SS$_NORMAL)
	    {
	      /* make the directory out of the device and the
	       * directory returned
	       */
	      strncat(dir_string, (char *) item_list[0].component_address,
			item_list[0].component_length);
	      strncat(dir_string, (char *) item_list[1].component_address,
			item_list[1].component_length);
	      strncat(file_string, (char *) item_list[2].component_address,
			item_list[2].component_length);

	    }

	}
#endif

#ifdef __unix__
    char *char_pos;
    char *ext_pos;
    int  str_len;

    /* find final backslash, this is the start of the filename */
    char_pos = strrchr(in_string, '/');

    if (char_pos != 0)
      {
        /* if there is a final backslash, copy the directory */
 	str_len = char_pos - in_string + 1;
	strncpy(dir_string, in_string, str_len);
	dir_string[str_len] = '\0';

   	/* add one to get past final backslash. get the file name without file
           extension,
        */
	char_pos = char_pos + 1;
        ext_pos = strrchr(in_string, '.');
        str_len = ext_pos - char_pos;

	if (str_len < 1)
	    /* filter does not contain a '.' */
	    strcpy(file_string, char_pos);
	else
	  {
	    strncpy(file_string, char_pos, str_len);
	    file_string[str_len] = '\0';
	  }
      }
    else
      {
	/*  else the string contains no directory, hence only get the file
	 *  name without file extension
	 */

        ext_pos = strrchr(in_string, '.');
        str_len = ext_pos - in_string;

	if (str_len < 1)
	    /* filter does not contain a '.' */
	    strcpy(file_string, in_string);
	else
	  {
	    strncpy(file_string, in_string, str_len);
	    file_string[str_len] = '\0';
	  }
      }
#endif  /* ultrix */

}


#ifdef __vms__
/*  routines borrowed from CALC to find out which converters
 *  are on a VMS system.
 */

/*
 *		WC.C		Michael Zarlenga	18-Mar-1987
 *		====		================	===========
 *
 *
 *	fwild(filespec,filename)
 *
 *	char	*filespec, *filename;
 *		Used to start a wildcard search.
 *		filespec and filename should be char[256]
 *		Returns RMS$_NORMAL if successful.
 *
 *
 *
 *	fnext(filename)
 *	char	*filename;
 *
 *		Used to continue a wildcard search.
 *		filename should be char[256]
 *		Returns RMS$_NORMAL if successful.
 *
 *
 *
 *	fdone()
 *		Used to end a wildcard search.
 *		Returns RMS$_NORMAL if successful.
 *
 */


static	struct	dsc$descriptor_s
			_last, _spec, _name;
static	unsigned int	_context;
static	CDAstatus	_status;
static	char		_saved_filename[256];

/* ========================================================================= */
fwild(filespec,filename)
char	*filespec, *filename;
{
   _spec.dsc$b_class   = DSC$K_CLASS_S;
   _spec.dsc$b_dtype   = DSC$K_DTYPE_T;
   _spec.dsc$w_length  = strlen(filespec);
   _spec.dsc$a_pointer = filespec;

   _name.dsc$b_class   = DSC$K_CLASS_S;
   _name.dsc$b_dtype   = DSC$K_DTYPE_T;
   _name.dsc$w_length  = 255;
   _name.dsc$a_pointer = filename;

   _last.dsc$b_class   = DSC$K_CLASS_S;
   _last.dsc$b_dtype   = DSC$K_DTYPE_T;
   _last.dsc$w_length  = 0;
   _last.dsc$a_pointer = 0;

   _context            = 0;

   _status = LIB$FIND_FILE(&_spec,&_name,&_context,0,&_last,0,0);

   _fsquish(filename);
   strcpy(_saved_filename,filename);

   return _status;
}
/* ========================================================================= */
fnext(filename)
char	*filename;
{
   _name.dsc$w_length  = 255;
   _name.dsc$a_pointer = filename;

   _last.dsc$w_length  = strlen(_saved_filename);
   _last.dsc$a_pointer = _saved_filename;

   _status = LIB$FIND_FILE(&_spec,&_name,&_context,0,&_last,0,0);

   _fsquish(filename);
   strcpy(_saved_filename,filename);

   return _status;
}
/* ========================================================================= */

CDAstatus fdone()
{
   CDAstatus	_status;

   _status = LIB$FIND_FILE_END(&_context);
   return _status;
}
/* ========================================================================= */

void _fsquish(string)
char	*string;
{
   for (string+=254; (*string == ' '); string--) ;
   *(++string) = 0;
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	get_converter_list(wild_card_str, num_found, new_style_name)
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	given a wildcarded string representing front-end converters
 *	on a system; return an array of strings which are the names
 *	of the converters on the system;
 *
 *  FORMAL PARAMETERS:
 *   	char *wild_card_str;
 *   	int  *num_found;
 *	Boolean new_style_name (TRUE only if we're looking for new
 * 			        style named converters on ultrix)
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  FUNCTION VALUE:
 *      returns an array of null terminated strings
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */
char **get_converter_list(wild_card_str, num_found, new_style_name)
    char    *wild_card_str;
    int     *num_found;
    Boolean new_style_name;

{
    char		next_converter[256];
    int			length,
			search_stat;

    char		*position_pointer;

    char		name_buffer[256],
			temp_string[256],
			final_string[256];

    char		**ret_list = 0;
    int			list_size = 0;
    int			stat;

    /* 3 wildcarding routines are fwild, fnext, and fdone */
    search_stat = fwild(wild_card_str, name_buffer);
    *num_found = 0;

    while ( (search_stat != RMS$_NMF) && (search_stat != RMS$_FNF) )
      {
	position_pointer = strchr (name_buffer, '_');
	position_pointer = position_pointer + 1;

	/* strip off characters before the underscore */
	strcpy (temp_string, position_pointer);
	position_pointer = strchr (temp_string, '.');
	length = position_pointer - temp_string;

	/* strip off characters after the dot extenstion name */
	strncpy (final_string, temp_string, length);
	final_string [length] = '\0';

	/* get more space for the new string */
	list_size = list_size + sizeof(char *);
	ret_list = (char **) XtRealloc(ret_list, list_size);

	/* put new string in array */
	ret_list[*num_found] = XtMalloc( (strlen(final_string)+1) );
	strcpy(ret_list[*num_found], final_string);

	*num_found = *num_found + 1;

	/* find next match */
	search_stat = fnext(name_buffer);
      }

    fdone();

    return(ret_list);
}
#endif


#ifdef __unix__
/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	get_converter_list(directory, num_found, new_style_name)
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	given a wildcarded string representing front-end converters
 *	on a system; return an array of strings which are the names
 *	of the converters on the system; The bulk of this routine was
 *	copied from the DECtoolkit FileSelection Widget; under the condition
 *	that the DECtoolkit people will not be supporting it in any way.
 * 	The routine was originally called  buildFileList()
 *
 *  FORMAL PARAMETERS:
 *   	char *wild_card_str;
 *   	int  *num_found;
 *	Boolean new_style_name (TRUE only if we're looking for new
 * 			        style named converters on ultrix)
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  FUNCTION VALUE:
 *      returns an array of null terminated strings
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */

/*
 * Returns an array of pointers to text strings. Each string is a file in the
 * given directory that matches the suffix globbing pattern.
 *
 * Tilde notation is handled.  This returns full path names even if a relative
 * one is given.
 */

char **get_converter_list(directory, num_found, new_style_name)
char	*directory;
int 	*num_found;
Boolean new_style_name;
{
	struct passwd	*pw, *getpwuid(), *getpwnam();
	char		*str, *ncp, *ocp, namebuf[128], dirbuf[1024];
	char		*rePart, *dirEnd, *name, **fileList;
	char		*XtMalloc(), *XtRealloc();
	int		reFound, listLen, matched;
#ifndef __osf__
	extern		bcopy(), re_exec();
#endif
	char		cvtBuf[256], *re_comp();
	int		firstDot;
	DIR		*dirp;

#ifdef osf1
	struct dirent	*dp;
#else
	struct direct	*dp;
#endif

 	int		file_count;
	char		temp_string[256];
	char		*char_pos;
        int		i;

	/*
	 * First, handle tilde notation.
	 */

	if ( *directory == '~' ) {
		if (directory[1] == 0 || directory[1] == '/' ) {
			if((pw=getpwuid(getuid())) == NULL)
				return NULL;
			str = XtMalloc(strlen(pw->pw_dir)+1+strlen(directory));
			strcpy(str, pw->pw_dir);
			strcat(str, directory+1);
		} else {
			ncp = namebuf;
			ocp = directory+1;
			while(*ocp && *ocp != '/')
				*ncp++ = *ocp++;
			*ncp = 0;
			if((pw=getpwnam(namebuf)) == NULL)
				return NULL;
			str = XtMalloc(strlen(pw->pw_dir)+1+strlen(ocp));
			strcpy(str, pw->pw_dir);
			strcat(str, ocp);
		}
	} else if (*directory != '/') {
#ifndef __osf__
		extern char getwd();
#endif

		ncp = (char *)getwd(dirbuf);
		str = XtMalloc(strlen(ncp)+1+strlen(directory)+1);
		strcpy(str, ncp);
		strcat(str, "/");
		strcat(str, directory);
	} else {
		str = XtMalloc(strlen(directory)+1);
		strcpy(str, directory);
	}

	/*
	 * Split out the globbing part.  Look from the right until a slash is
	 * found; if there are no special chars, assume null re.
	 * Leave the directory name proper, with a trailing /, in dirbuf.
	 */

	reFound = 0;
	for (rePart = &str[strlen(str)]; *rePart != '/'; rePart--) {
		switch(*rePart) {
		case '[':
		case '*':
		case '?':	reFound = 1;
		}
	}
	dirEnd = rePart;
	if (reFound) {
		ncp = cvtBuf;
		ocp = ++rePart;
		*ncp++ = '^';
		while (*ocp) {
			switch(*ocp) {
			case '.':
				*ncp++ = '\\';
				*ncp++ = '.';
				break;
			case '?':
				*ncp++ = '.';
				break;
			case '*':
				*ncp++ = '.';
				*ncp++ = '*';
				break;
			default:
				*ncp++ = *ocp;
			}
			ocp++;
		}
		*ncp++ = '$';
		*ncp = '\0';
		if ((ncp = re_comp(cvtBuf)) == 0)
			firstDot = (*rePart == '.');
		else {
			reFound = 0;
		}
		bcopy(str, dirbuf, dirEnd - str);
		if (dirbuf[dirEnd - str - 1] != '/') {
			dirbuf[dirEnd - str] = '/';
			dirEnd++;
		}
		dirbuf[dirEnd - str] = '\0';
	} else {
		strcpy(dirbuf, str);
		dirEnd = &dirbuf[strlen(dirbuf)-1];
		if (*dirEnd != '/')
			*++dirEnd = '/';
		*++dirEnd = '\0';
	}
	XtFree(str);

	/*
	 * Scan the directory.
	 */

	if ((dirp = opendir(dirbuf)) == NULL) {
		return NULL;
	}

	listLen = 0;
	fileList = NULL;
	name = NULL;
	while (dp = readdir(dirp)) {
		matched = 0;
		if (name)
			XtFree(name);
		name = XtMalloc(strlen(dp->d_name)+1);
		strcpy(name, dp->d_name);
		if (reFound) {
			/*
			 * The file pattern * must not match dot.  If the
			 * pattern begins with dot and the file doesn't, OR the
			 * pattern doesn't begin with dot and the file does,
			 * skip it.
			 */
			if (firstDot && (*name != '.') ||
			    !firstDot && (*name == '.'))
				continue;
			matched = re_exec(name);
		} else
			matched = 1;
		if (!matched)
			continue;
		if (fileList)
			fileList = (char **)XtRealloc(fileList,
						(listLen+2)*sizeof(char *));
		else
			fileList = (char **)XtMalloc(2*sizeof(char *));
		fileList[listLen] = XtMalloc(strlen(name) + strlen(dirbuf)+1);
		strcpy(fileList[listLen], dirbuf);
		strcat(fileList[listLen], name);
		listLen++;
	}

	if (listLen > 0)
	{
	    qsort(fileList, listLen, sizeof(char *), fileCompare);
	    fileList[listLen] = '\0';
	}

	closedir(dirp);

	if (fileList == NULL)
	   file_count = 0;
	else
	   for (file_count = 0; fileList[file_count]; file_count++);

	*num_found = file_count;

	/*  strip the string down so that it is only the
	 *  converter name
	 */

	if (new_style_name)
	  {
	    /* new style name, e.g. (textddif, wk1dtif, etc) */
	    for (i=0; i<file_count; i++)
	      {
                CDAsize name_len;

		/* first remove anthing before last slash */
		if (strrchr(fileList[i], '/'))
		  {
		    char *ns_tmp_str;
		    ns_tmp_str = strrchr(fileList[i], '/') + 1;
		    strcpy(fileList[i], ns_tmp_str);
		  }

		name_len = strlen(fileList[i]);

		/* now, just strip off the end four chars */
	    	fileList[i][name_len-4] = '\0';
	      }
	  }
	else
	/* old style name (e.g. ddif_read_*) */
	for (i=0; i<file_count; i++)
	  {
	    int j;

	    /* find right-most '/' and strip chars before */
	    char_pos = strrchr(fileList[i], '/');
	    char_pos = char_pos + 1;
	    strcpy(temp_string, char_pos);

	    /*  find first two '_' s and strip off before them;
    	     *  what's left is the converter name
	     */

	    for (j=0; j<2; j++)
	      {
	    	char_pos = strchr(temp_string, '_');
	    	char_pos = char_pos + 1;
	    	strcpy(temp_string, char_pos);
	      }

	    strcpy(fileList[i], temp_string);
	  }

	return fileList;
}


#if defined(ultrix) || defined(sun)

int fileCompare(s1, s2)
char **s1, **s2;
{

#else

/* ANSI C, match proto for qsort (does not compile on mips ultrix) */

int fileCompare(s1_param, s2_param)
const void *s1_param, *s2_param;
{
    char **s1 = (char **) s1_param;
    char **s2 = (char **) s2_param;

#endif

    return strcmp(*s1, *s2);
}


/* constants used in searching path */
#define PATHSEP         ':'
#define DIRSEP          '/'

/*  list of wildcarded converter names for unix ; first two are new style;
 *  final three are old style names
 */
static char *unix_cvt_list[] =
	       {"*ddif",
		"*dtif",
		"cda_read_*",
		"ddif_read_*",
		"dtif_read_*"};

/* routine to return the next directory in a path string;
 * path_ptr is a pointer to a path string that is modified
 * to advance past the next path; next_dir will return the next
 * path in the path string.
 *
 * (copied from cda_pth.c)
 */

get_next_path(path_ptr, next_dir)
    char **path_ptr;
    char *next_dir;

   {
        register char 	*nam;
        static char	*home = 0;
        char		*path;

        path = *path_ptr;

	/* copy the current directory into next_dir */
	nam = next_dir;
	/* copy till PATHSEP */
	if (*path == '~') {
	    char           *p = nam;
	    path++;
	    while (*path && *path != PATHSEP && *path != DIRSEP)
		*p++ = *path++;
	    *p = 0;
	    if (*nam == 0) {
		if (home == 0) {
		    if (!(home = getenv("HOME")))
			home = ".";
		}
		strcpy(next_dir, home);
	    } else {
		struct passwd  *pw = getpwnam(next_dir);
		if (pw)
		    strcpy(next_dir, pw->pw_dir);
		else
		    next_dir[0] = '\0';
	    }
	    nam = next_dir + strlen(next_dir);
	}
	while (*path != PATHSEP && *path)
	    *nam++ = *path++;
	*nam = 0;
	if (nam == next_dir)
	    *nam++ = '.';	/* null component is current dir */

	*nam = '\0';

        *path_ptr = path;
}

#endif  /* ultrix */



/*  function to free an array of strings;
 *  first the strings are deallocated;
 *  then the array of addresses is deallocated
 */

void free_list(list, count)
    char **list;
    int  count;
{
    int i;

    if (list == 0)
	return;

    for (i=0; i< count; i++)
	XtFree(list[i]);

    XtFree((char *)list);
}


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	new_string(str_list, new_string, str_count)
 *
 *  FUNCTIONAL DESCRIPTION:
 *      this routine takes an array of strings: str_list;
 *      a null terminated string to be alphebetically inserted in to
 *	the list: new_string; and a count for the array: str_count;
 *	(str count is the count without the new item; it is incremented after
 * 	the new string is inserted.)
 *
 *	it finds the correct alphebetical position for the string within
 *	the array, and inserts it into the list
 *
 *  FORMAL PARAMETERS:
 *   	char	 	**str_list;
 *   	char	  	*new_string;
 *   	int		*str_count;
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  FUNCTION VALUE:
 *
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */
new_string(str_list, new_string, str_count)
    char 	  **str_list;
    char	  *new_string;
    int		  *str_count;

{
    int i,j;
    unsigned short found = 0;

    i = 0;

    while ( (i < *str_count) &&
	    (!found)          )

      {
	if (strcmp(str_list[i], new_string) > 0)
	  {
	    /*  string position found; move all strings below it down
	     *  one position in the array;
             */
	    found = 1;
	    for (j = *str_count; i < j; j--)
	      	str_list[j] = str_list[j-1];
	  }
	else
	    i++;
      }

    /* insert new string */
    str_list[i] = new_string;
    *str_count = *str_count + 1;
}




/*
 *++
 *
 *  FUNCTION NAME:
 *
 *      build_converter_list()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      routine to build a list of available front end converters
 *	on a system, and call routine to fill list box with built list;
 *	DDIF and DTIF are assumed to be on all systems;
 *	the rest of the converters are found by calling system routines.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  FUNCTION VALUE:
 *
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */

build_converter_list()

{
    XmString		*item_list;
    int			item_count;
    int			item_size;
    int			list_size;
    int			num_strings[3];
    Arg			arg_list[10];
    int			arg_count;
    unsigned long 	stat;
    int			cs_count;		/* # compound strings to pass
					           to toolkit list box */

    XmString 		cstr;			/* Current-format string. */

    char		*conv_strings[3];	/* array of wildcarded converter strings */
    char		**conv_list;            /* array of converter names */

    char		**sub_conv_list[3];     /* these 3 sub-lists contain
						   the list of converters for
						   each of the 3 wildcarded
						   converter strings */

#ifdef __unix__
    /*  extra variables used on ultrix to lookup new style
     *  converter names as well
     */
    char		***new_style_conv_list = 0;
    int			**num_matches_array = 0;
    int			path_count = 0;
    char		*path;
    char 		next_wildcard[256];
    char		next_dir[256];
#endif

    int 		i, j;

    /*  fill in the wildcarded converter strings */
    conv_strings[0] = XtMalloc( (strlen(dvr_cda_wildcard)+1) );
    strcpy(conv_strings[0], dvr_cda_wildcard);
    conv_strings[1] = XtMalloc( (strlen(dvr_ddif_wildcard)+1) );
    strcpy(conv_strings[1], dvr_ddif_wildcard);
    conv_strings[2] = XtMalloc( (strlen(dvr_dtif_wildcard)+1) );
    strcpy(conv_strings[2], dvr_dtif_wildcard);


    /*  initially, item count is 2 (ddif and dtif), and item list is empty;
     *  item_count applies to both the null-terminated string array; and the
     *  compound string array that we pass to the toolkit; list_size is the
     *  size of the null-terminated string array; item_size is the size of
     *	the compound string array;
     */
    item_count = 2;

    item_size = 0;
    list_size = 0;

    item_list = 0;
    conv_list = 0;

    /* allocate memory for first two strings */

    list_size = list_size + (2 * sizeof(char *));
    conv_list = (char **) XtRealloc((char *)conv_list, list_size);

    conv_list[0] = XtMalloc((strlen(dvr_ddif_string)+1));
    conv_list[1] = XtMalloc((strlen(dvr_dtif_string)+1));

    /* put first two strings in list */
    strcpy(conv_list[0], dvr_ddif_string);
    strcpy(conv_list[1], dvr_dtif_string);

    if (dps_exists)
      /* add postscript if display has display postscript */
      {
      	list_size = list_size + sizeof(char *);
      	conv_list = (char **) XtRealloc((char *)conv_list, list_size);

      	conv_list[2] = XtMalloc((strlen(POSTSCRIPT_STR)+1));

      	strcpy(conv_list[2], POSTSCRIPT_STR);

      	item_count++;
      }

    for (i=0; i<3; i++)
      {
	char **current_conv_list;

	/*  for each of the three wildcarded converter strings,
	 *  find all available converter names, and add to list by
	 *  calling new_string()
	 */

	sub_conv_list[i] = get_converter_list(conv_strings[i],
					   &num_strings[i],
					   FALSE);

	current_conv_list = sub_conv_list[i];

	if (num_strings[i] != 0)
	  {
	    list_size = list_size + (num_strings[i] * sizeof(char *));
	    conv_list = (char **) XtRealloc((char *)conv_list, list_size);

	    for (j=0; j<num_strings[i]; j++)
	        new_string(conv_list, current_conv_list[j], &item_count);
	  }
      }

#ifdef __unix__

    /*  for each dir in the path, look up new style converters
     *  for both *ddif and *dtif; new style converters can exist
     *  anywhere in PATH
     */
    path = getenv("PATH");

    if (path)
      	do
	  {
	    char **current_conv_list;
	    int  next_dir_len;
	    int  k;

            get_next_path(&path, next_dir);

	    /*  for each of the new style wildcarded converter strings,
	     *  in this path dir ("*ddif", "*dtif") find all available
	     *  converter names, and add to list by calling new_string()
	     */

	    /* allocate memory for two more lists of possible
	     * converters from this dir
	     */
	    new_style_conv_list = (char ***) XtRealloc((char *)new_style_conv_list,
						(sizeof(char **) *
						(path_count+2)));

	    num_matches_array   = (int **) XtRealloc((char *)num_matches_array,
						(sizeof(int *) *
						(path_count+2)));

	    for (k=0; k<2; k++)
	      {
	       	char *next_filter = unix_cvt_list[k];

	        strcpy(next_wildcard, next_dir);

	    	/* make sure path ends with '/' */
	    	if (next_wildcard[strlen(next_dir)-1] != '/')
	            strcat(next_wildcard, "/");

		/* append filter ("*ddif" or "*dtif") */
	        strcat(next_wildcard, next_filter);

		/* allocate memory for another count of cvtrs found */
		num_matches_array[path_count] = (int *) XtMalloc(sizeof(int));
		*num_matches_array[path_count] = 0;

		/* get list of matching converters */
		new_style_conv_list[path_count] = get_converter_list(next_wildcard,
					   num_matches_array[path_count],
					   TRUE);

		current_conv_list = new_style_conv_list[path_count];

		if (*num_matches_array[path_count] != 0)
		  {
	    	    list_size = list_size + (*num_matches_array[path_count] * sizeof(char *));
	    	    conv_list = (char **) XtRealloc((char *)conv_list, list_size);

	    	    for (j=0; j< *num_matches_array[path_count]; j++)
	        	new_string(conv_list, current_conv_list[j], &item_count);
	      	  }

	    	path_count++;
       	      }

	  /* skip over PATHSEP and try again */
	  }
	while (*(path++)); /* do look in next dir in path string until
			    * end of path string
			    */

    /* see if CDAPATH environment var is set; converters can also
     * exist in CDAPATH (both new style and old); note, CDAPATH is
     * not a list of dirs; it points to one single dir
     */
    path = getenv("CDAPATH");

    if (path)
      {
	int k;

	/* allocate space for 5 more converter lists
	 * see unix_cvt_list; these will be deallocated along with the
	 * rest of the list from PATH
	 */
	new_style_conv_list = (char ***) XtRealloc((char *)new_style_conv_list,
						(sizeof(char **) *
						(path_count+5)));

	num_matches_array   = (int **) XtRealloc((char *)num_matches_array,
						(sizeof(int *) *
						(path_count+5)));

	for (k=0; k<5; k++)
	  {
	    /* lookup matching converters in CDAPATH for each of
	     * the wildcards in unix_cvt_list
	     */

	    char *next_filter = unix_cvt_list[k];
            char **current_conv_list;

	    strcpy(next_wildcard, path);

	    /* make sure path ends with '/' */
	    if (next_wildcard[strlen(path)-1] != '/')
	        strcat(next_wildcard, "/");
	    strcat(next_wildcard, next_filter);

	    /* allocate space for another list count */
	    num_matches_array[path_count] = (int *) XtMalloc(sizeof(int));
	    *num_matches_array[path_count] = 0;

	    /* get next matching list */
	    new_style_conv_list[path_count] = get_converter_list(next_wildcard,
					   num_matches_array[path_count],
					   (k<2)); /* only first 2 converter
						    * wildcards in unix_cvt_list
                                                    * are new style
						    */
	    current_conv_list = new_style_conv_list[path_count];

	    if (*num_matches_array[path_count] != 0)
	      {
	    	list_size = list_size + (*num_matches_array[path_count] * sizeof(char *));
	    	conv_list = (char **) XtRealloc((char *)conv_list, list_size);

	    	for (j=0; j< *num_matches_array[path_count]; j++)
	          new_string(conv_list, current_conv_list[j], &item_count);
	      }

	    path_count++;
	  }

      }

#endif

    /* create compound string list to pass to toolkit */
    item_size = item_count * sizeof(XmString);
    item_list = (XmString *) XtRealloc((char *)item_list, item_size);

    cs_count = 0;

    for (i = 0; i<item_count; i++)
      {
#ifdef __unix__
	/*  on ultrix, ignore cda_read_cda and oldcddif (internal only);
	 *  ignore any strings with dots in them (like defstyle.);
	 *  also ignore any names starting with "cda" , "ddif" or "dtif";
	 *  this may be the case if looking for new style names and finding
	 *  old style name instead
	 */
	if ( (strcmp(conv_list[i], "ddif") == 0) ||      /* allow ddif */
	     (strcmp(conv_list[i], "dtif") == 0) ||      /* allow dtif */
                                                         /* disallow these */
	   ( (strncmp(conv_list[i], "cda", 3)  != 0) &&
	     (strcmp (conv_list[i], "oldc")    != 0) &&
	     (strncmp(conv_list[i], "ddif", 4) != 0) &&
	     (strncmp(conv_list[i], "dtif", 4) != 0) &&
	     !(strchr(conv_list[i], '.')) ) )
	  {
	    /* if necessary, switch to old names */

	    if (i != 0)
	      {
		if (strcmp(conv_list[i], conv_list[i-1]))
		  {
            	    item_list[cs_count] = XmStringLtoRCreate(conv_list[i], "ISO8859-1");
	    	    cs_count = cs_count + 1;
		  }
	      }
	    else
#else
	  {
#endif
	      {
            	item_list[cs_count] = XmStringLtoRCreate(conv_list[i], "ISO8859-1");
	    	cs_count = cs_count + 1;
	      }
	  }
      }

    /* call set values on list-box to fill with converter list */

    arg_count = 0;
    XtSetArg(arg_list[arg_count], XmNitemCount, cs_count);
    arg_count++;
    XtSetArg(arg_list[arg_count], XmNitems, item_list);
    arg_count++;

    XtSetValues(format_list_id, arg_list, arg_count);

    /* highlight current format in list-box */

    cstr = XmStringLtoRCreate((char *) current_format , "ISO8859-1");
    (void) XmListSelectItem(format_list_id,
				cstr,
				FALSE);
    XtFree((char *)cstr);

    /* free string arrays that have been allocated locally */
    for (i=0; i<3; i++)
      {
	if (num_strings[i] != 0)
	    free_list(sub_conv_list[i], num_strings[i]);
      }

#ifdef __unix__
    /* free any lists allocated looking for new style converters or
     * CDAPATH converters
     */
    for (i=0; i<path_count; i++)
      {
	if (*num_matches_array[i] != 0)
	    free_list(new_style_conv_list[i], *num_matches_array[i]);
	XtFree((char *)num_matches_array[i]);
      }

    XtFree((char *)num_matches_array);
#endif

    for (i = 0; i<cs_count; i++)
      {
	XtFree((char *)item_list[i]);
      }

    XtFree((char *)item_list);

    XtFree((char *)conv_list);
    free_list(conv_strings, 3);
}


/*
**++
**  ROUTINE NAME:
**	get_paper_list(paper_list)
**
**  FUNCTIONAL DESCRIPTION:
**      fill in the static table: paper_list, with the values
**	for all the paper sizes to be displayed in the list box
**	within the options pop-up box; get the list of paper
**	strings from the DECtoolkit; the strings are set in UIL.
**
**  FORMAL PARAMETERS:
**	paper_struct *paper_list;
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

get_paper_list(paper_list)
    paper_struct *paper_list;

{
    Arg 	arglist[3];
    XmString 	*cs_paper_list;

    /* get the list of paper size strings */
    XtSetArg(arglist[0], XmNitems, &cs_paper_list);
    XtGetValues(size_list_id, arglist, 1);

    /* fill in the table */

    paper_list[0].cstring 	= XmStringCopy(cs_paper_list[0]);
    paper_list[0].pap_width   	= A0_MM_WIDTH;
    paper_list[0].pap_height  	= A0_MM_HEIGHT;
    paper_list[0].default_unit  = A0_DEFAULT_UNIT;

    paper_list[1].cstring 	= XmStringCopy(cs_paper_list[1]);
    paper_list[1].pap_width   	= A1_MM_WIDTH;
    paper_list[1].pap_height  	= A1_MM_HEIGHT;
    paper_list[1].default_unit  = A1_DEFAULT_UNIT;

    paper_list[2].cstring 	= XmStringCopy(cs_paper_list[2]);
    paper_list[2].pap_width   	= A2_MM_WIDTH;
    paper_list[2].pap_height  	= A2_MM_HEIGHT;
    paper_list[2].default_unit  = A2_DEFAULT_UNIT;

    paper_list[3].cstring 	= XmStringCopy(cs_paper_list[3]);
    paper_list[3].pap_width   	= A3_MM_WIDTH;
    paper_list[3].pap_height  	= A3_MM_HEIGHT;
    paper_list[3].default_unit  = A3_DEFAULT_UNIT;

    paper_list[4].cstring 	= XmStringCopy(cs_paper_list[4]);
    paper_list[4].pap_width   	= A4_MM_WIDTH;
    paper_list[4].pap_height  	= A4_MM_HEIGHT;
    paper_list[4].default_unit  = A4_DEFAULT_UNIT;

    paper_list[5].cstring 	= XmStringCopy(cs_paper_list[5]);
    paper_list[5].pap_width   	= A_INCH_WIDTH;
    paper_list[5].pap_height  	= A_INCH_HEIGHT;
    paper_list[5].default_unit  = A_DEFAULT_UNIT;

    paper_list[6].cstring 	= XmStringCopy(cs_paper_list[6]);
    paper_list[6].pap_width   	= B_INCH_WIDTH;
    paper_list[6].pap_height  	= B_INCH_HEIGHT;
    paper_list[6].default_unit  = B_DEFAULT_UNIT;

    paper_list[7].cstring 	= XmStringCopy(cs_paper_list[7]);
    paper_list[7].pap_width   	= C_INCH_WIDTH;
    paper_list[7].pap_height  	= C_INCH_HEIGHT;
    paper_list[7].default_unit  = C_DEFAULT_UNIT;

    paper_list[8].cstring 	= XmStringCopy(cs_paper_list[8]);
    paper_list[8].pap_width     = D_INCH_WIDTH;
    paper_list[8].pap_height  	= D_INCH_HEIGHT;
    paper_list[8].default_unit  = D_DEFAULT_UNIT;

    paper_list[9].cstring 	= XmStringCopy(cs_paper_list[9]);
    paper_list[9].pap_width   	= E_INCH_WIDTH;
    paper_list[9].pap_height  	= E_INCH_HEIGHT;
    paper_list[9].default_unit  = E_DEFAULT_UNIT;

    paper_list[10].cstring 	= XmStringCopy(cs_paper_list[10]);
    paper_list[10].pap_width    = LEGAL_INCH_WIDTH;
    paper_list[10].pap_height   = LEGAL_INCH_HEIGHT;
    paper_list[10].default_unit = LEGAL_DEFAULT_UNIT;

    paper_list[11].cstring 	= XmStringCopy(cs_paper_list[11]);
    paper_list[11].pap_width    = LP_INCH_WIDTH;
    paper_list[11].pap_height   = LP_INCH_HEIGHT;
    paper_list[11].default_unit = LP_DEFAULT_UNIT;

    paper_list[12].cstring 	= XmStringCopy(cs_paper_list[12]);
    paper_list[12].pap_width    = VT_INCH_WIDTH;
    paper_list[12].pap_height   = VT_INCH_HEIGHT;
    paper_list[12].default_unit = VT_DEFAULT_UNIT;

    paper_list[13].cstring 	= XmStringCopy(cs_paper_list[13]);
    paper_list[13].pap_width    = EXEC_INCH_WIDTH;
    paper_list[13].pap_height   = EXEC_INCH_HEIGHT;
    paper_list[13].default_unit = EXEC_DEFAULT_UNIT;

    paper_list[14].cstring 	= XmStringCopy(cs_paper_list[14]);
    paper_list[14].pap_width    = B4_MM_WIDTH;
    paper_list[14].pap_height   = B4_MM_HEIGHT;
    paper_list[14].default_unit = B4_DEFAULT_UNIT;

    paper_list[15].cstring 	= XmStringCopy(cs_paper_list[15]);
    paper_list[15].pap_width    = B5_MM_WIDTH;
    paper_list[15].pap_height   = B5_MM_HEIGHT;
    paper_list[15].default_unit = B5_DEFAULT_UNIT;

}


/*
**++
**  ROUTINE NAME:
**	get_uil_strings()
**
**  FUNCTIONAL DESCRIPTION:
**      fetch all necessary compound strings from UIL; this
**	module should not contain any text that will be displayed
**	in an application window; any string fetched here should
**	also be freed in free_uil_strings().
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

CDAstatus get_uil_strings()

{
    CDAstatus	ret_status;

    ret_status = fetch_compound_string("dvr$copyright_str", &dvr_copyright_str);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$colon_str", &dvr_colon_str);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$title_str", &dvr_title_str);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$cursor_font_file",
					&dvr_cursor_font_file);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$cursor_fg_color",
					&dvr_cursor_fg_color);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$cursor_bg_color",
					&dvr_cursor_bg_color);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$on_window_label",
					&dvr_on_window_label);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$on_version_label",
					&dvr_on_version_label);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ps_nohead_str",
					&dvr_ps_nohead_str);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

#ifdef __vms__
    ret_status = fetch_compound_string("dvr$vms_ddif_file_ext",
					&dvr_ddif_file_ext);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_def_format_str",
					&dvr_default_format_str);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_op_file_ext",
					&dvr_options_file_ext);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_cda_wildcard",
					&dvr_cda_wildcard);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_ddif_wildcard",
					&dvr_ddif_wildcard);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_dtif_wildcard",
					&dvr_dtif_wildcard);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_ddif_string",
					&dvr_ddif_string);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$vms_dtif_string",
					&dvr_dtif_string);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$message_title",
					&dvr_message_title);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

#else
    ret_status = fetch_compound_string("dvr$ult_ddif_file_ext",
					&dvr_ddif_file_ext);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_def_format_str",
					&dvr_default_format_str);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_op_file_ext",
					&dvr_options_file_ext);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_stdin_str",
					&dvr_stdin_str);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_cda_wildcard",
					&dvr_cda_wildcard);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_ddif_wildcard",
					&dvr_ddif_wildcard);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_dtif_wildcard",
					&dvr_dtif_wildcard);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_ddif_string",
					&dvr_ddif_string);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_dtif_string",
					&dvr_dtif_string);
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    ret_status = fetch_compound_string("dvr$ult_help_library",
					&dvr_ult_help_library);

    if (ret_status != DVR_NORMAL)
	return(ret_status);

#endif   /* ultrix specific */

    ret_status = build_format_filter_table();
    if (ret_status != DVR_NORMAL)
	return(ret_status);

    return(DVR_NORMAL);

}


/*
**++
**  ROUTINE NAME:
**	free_uil_strings()
**
**  FUNCTIONAL DESCRIPTION:
**      free all strings initially fetchede from UIL;
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

void free_uil_strings()

{
    XtFree(dvr_copyright_str);
    XtFree(dvr_colon_str);
    XtFree(dvr_title_str);
    XtFree(dvr_cursor_font_file);
    XtFree(dvr_cursor_fg_color);
    XtFree(dvr_cursor_bg_color);
    XtFree(dvr_ddif_file_ext);
    XtFree(dvr_default_format_str);
    XtFree(dvr_options_file_ext);
    XtFree(dvr_dtif_string);
    XtFree(dvr_ddif_string);
    XtFree(dvr_cda_wildcard);
    XtFree(dvr_ddif_wildcard);
    XtFree(dvr_dtif_wildcard);
    XtFree(dvr_on_window_label);
    XtFree(dvr_on_version_label);
    XtFree(dvr_ps_nohead_str);

#ifdef __vms__
    XtFree(dvr_message_title);
#endif

#ifdef __unix__
    XtFree(dvr_stdin_str);
    XtFree(dvr_ult_help_library);
#endif

    free_format_filter_table();

}


/*
**++
**  ROUTINE NAME:
**	setup_viewer (w)
**
**  FUNCTIONAL DESCRIPTION:
**      save viewer widget id; set any values specified on command line.
**
**  FORMAL PARAMETERS:
**	Widget		w;
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

setup_viewer (w)
    Widget		w;

{
    /*  get the Main Window's work area widget id,
     *  this is the id of the viewer widget.
     */
    Arg arglist[10];
    int		argcount = 0;

    viewer = w;

    if (dps_exists)

    /*  if the user has requested processing options other than the
     *  defaults via the command line, set them here. If the options
     *  are not different, then do not set them here. This allows the user
     *  to use Xdefaults to set the processing options iff they have not
     *  specified any on the command line. (i.e. the command line takes
     *  precedence over the xdefaults file.)
     */

    if ( !( (viewer_options & DvrSoftDirectives) &&
	    (viewer_options & DvrWordWrap) 	 &&
	    (viewer_options & DvrLayout)	 &&
	    (viewer_options & DvrSpecificLayout) ) )
      {
    	XtSetArg(arglist[argcount], DvrNprocessingOptions, viewer_options);
	argcount++;
      }

    if (current_paper_width != 0)
      {
	float 	float_mm_paper_width;
	int 	int_mm_paper_width;

	current_size_unit = DVR_CHAR_UNITS;

	float_mm_paper_width = current_paper_width * MM_PER_CHAR_W;
	int_mm_paper_width = (int) (float_mm_paper_width + 0.5);

	XtSetArg(arglist[argcount], DvrNpaperWidth, int_mm_paper_width);
	argcount++;
      }

    if (current_paper_height != 0)
      {
	float 	float_mm_paper_height;
	int 	int_mm_paper_height;

	current_size_unit = DVR_CHAR_UNITS;

	float_mm_paper_height = current_paper_height * MM_PER_CHAR_H;
	int_mm_paper_height = (int) (float_mm_paper_height + 0.5);

	XtSetArg(arglist[argcount], DvrNpaperHeight, int_mm_paper_height);
	argcount++;
      }

    XtSetValues(viewer, arglist, argcount);

}


/*
**++
**  ROUTINE NAME:
**	size_list_create_proc (w)
**
**  FUNCTIONAL DESCRIPTION:
**      save size list box id; build paper list and highlight a list
**	box entry if the user has entered a paper width and height that
**	match one of the entries in the list;
**
**  FORMAL PARAMETERS:
**	Widget		w;
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

static void size_list_create_proc (w)
    Widget		w;

{
    XmString 	item_list[DVR_NUM_PAPER_SIZES];
    int 	i;
    Arg 	arg_list[3];
    int 	arg_count;

    size_list_id = w;

    get_paper_list(paper_list);

    if ( (current_paper_height != 0) &&
	 (current_paper_width  != 0) )
      {

    	for (i = 0; i<DVR_NUM_PAPER_SIZES; i++)
      	  {
	    /*  if this paper-size matches the one specified on
	     *  the command line, remember which one it is, so we
	     *  can highlight it in the list box
	     */
	    float cur_pap_height, cur_pap_width;
	    int   char_pap_height, char_pap_width;

	    cur_pap_height = paper_list[i].pap_height;
	    cur_pap_width  = paper_list[i].pap_width;
	    if (paper_list[i].default_unit == DVR_INCH_UNITS)
	      {
		if (current_size_unit == DVR_CHAR_UNITS)
		  {
	    	    cur_pap_height = cur_pap_height/INCH_PER_CHAR_H;
	    	    cur_pap_width  = cur_pap_width /INCH_PER_CHAR_W;
		  }
		else if (current_size_unit == DVR_MM_UNITS)
		  {
	    	    cur_pap_height = cur_pap_height * MM_PER_INCH;
	    	    cur_pap_width  = cur_pap_width  * MM_PER_INCH;
		  }

	      }
	    else if (paper_list[i].default_unit == DVR_MM_UNITS)
	      {
		if (current_size_unit == DVR_CHAR_UNITS)
		  {
	     	    cur_pap_height = cur_pap_height/MM_PER_CHAR_H;
	    	    cur_pap_width  = cur_pap_width /MM_PER_CHAR_W;
		  }
		else if (current_size_unit == DVR_INCH_UNITS)
		  {
	     	    cur_pap_height = cur_pap_height/MM_PER_INCH;
	    	    cur_pap_width  = cur_pap_width /MM_PER_INCH;
		  }
	      }

	    char_pap_height = (int) (cur_pap_height+0.5);
	    char_pap_width  = (int) (cur_pap_width +0.5);

	    if ( (char_pap_height == current_paper_height) &&
	         (char_pap_width  == current_paper_width) )
	    	current_size_index = i+1;

          }

        if (current_size_index != 0)
          {
    	    /* highlight current size in list-box */
    	    (void) XmListSelectItem(size_list_id,
				paper_list[current_size_index-1].cstring,
				FALSE);
          }

      }
}




/*
**++
**  ROUTINE NAME:
**	create_proc (w, tag, reason)
**
**  FUNCTIONAL DESCRIPTION:
**      this procedure is called upon creation of a widget via UIL if
**	a widget has a "create" callback procedure associated with it.
**
**	This is the case whenever we need to alter or query a widget's values
**	after it has been created - in which case we need the widget's id;
**
**	the tag is passed in identifying which widget has just been created
**	(via a UIL fetch); the tag literals have corresponding entries in
**	this module and in the UIL file; so if any widgets are added, they
**	need literals added in both places.
**
**  FORMAL PARAMETERS:
**    	Widget		w;
**	caddr_t		*tag;
**	caddr_t		*reason;
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

static void create_proc (w, tag, reason)
    Widget		w;
    caddr_t		*tag;
    caddr_t		*reason;

{
    unsigned short 	widget_num = (int) *tag;
    float		temp_float;
    Arg			arglist[3];

    switch (widget_num)
      {
	case k_menu:
	    	main_menu_id = w;
		break;

	case k_diag_info_pb:
		diag_info_button = w;
		break;

	case k_diag_info_st:
    		diag_info_text_widget = w;
		break;

	case k_doc_info_pb:
		doc_info_button = w;
		break;

	case k_doc_info_st:
    		doc_info_text_widget = w;
		break;

	case k_close_pb:
		close_button_id = w;
		break;

	case k_char_tb:
    		char_toggle_id = w;
		if (current_size_unit == DVR_CHAR_UNITS)
     		    (void) XmToggleButtonSetState(char_toggle_id,
						   TRUE, FALSE);
		break;

	case k_inch_tb:
		inch_toggle_id = w;
		if (current_size_unit == DVR_INCH_UNITS)
     		    (void) XmToggleButtonSetState(inch_toggle_id,
						   TRUE, FALSE);
		break;

	case k_mm_tb:
		mm_toggle_id = w;
		if (current_size_unit == DVR_MM_UNITS)
     		    (void) XmToggleButtonSetState(mm_toggle_id,
						   TRUE, FALSE);
		break;

	case k_viewer:
	        if (w == 0)
		    /* viewer not created, probably a UID failure, exit */
		    exit(DVR_DRMMAINFETCHFAIL);
		else
		    setup_viewer(w);
/* cbr stuff */

		if (cbr_mode == TRUE)
		    {
                     if (cbr_viewwgt_ptr != 0)
		         {
                          cbr_viewwgt_ptr = w;
                          DvrCbrSetCtxt((DvrViewerWidget) w, cbr_frg_ref_cb, cbr_frg_ref_parm, cbr_ref_buttons);
                         }
		    }
		else
		    DvrCbrClearCtxt((DvrViewerWidget) w);

		break;

	case k_new_file_pb:
/*
		if (cbr_mode == TRUE)
		    set_sensitivity (w, FALSE);
*/
		break;

/* end cbr stuff */

	case k_size_lb:
		size_list_create_proc(w);
		break;

	case k_format_lb:
    		format_list_id = w;

    		/* fill list box with converters on system */
    		build_converter_list();
		break;

	case k_format_st:
    		format_text_id = w;
    		if (strlen((char *) current_format) != 0)
		    /* if set on command line, set in text widget */
		    XmTextSetString(format_text_id, (char *) current_format);

    		XtOverrideTranslations(w, cr_trans_parsed);
		break;

	case k_reformat_tb:
		reform_toggle_id = w;

		/* get current processing options from widget */
		XtSetArg(arglist[0], DvrNprocessingOptions, &viewer_options);
		XtGetValues(viewer, arglist, 1);

    		/*  if layout is not on, then reformat was set
		 *  on command line; or via Xdefaults
		 */

    		if ( !(viewer_options & DvrLayout) )
     		    (void) XmToggleButtonSetState(reform_toggle_id,
						   TRUE, FALSE);
		break;

	case k_size_width_st:
    		size_width_text_id = w;

		/* ignore carriage returns */
    		XtOverrideTranslations(w, cr_trans_parsed);

		/* get current paper width from widget (in mm) */
		XtSetArg(arglist[0], DvrNpaperWidth, &current_paper_width);
		XtGetValues(viewer, arglist, 1);

		if ( (current_paper_width != 0) &&
		     (current_size_unit   != DVR_CHAR_UNITS) )

                    /*  size must have been via Xdefaults,
		     *  use mm (default unit from widget)
		     */
		    current_size_unit = DVR_MM_UNITS;

		else
		  {
		    /*  convert to characters (default unit from command line)
		     *  for text box
		     */
		    current_size_unit = DVR_CHAR_UNITS; /* default */
		    temp_float = current_paper_width/MM_PER_CHAR_W;
		    current_paper_width = (int) (temp_float + 0.5);
                  }

    		if (current_paper_width != 0)
      		  {
		    /* if set on command line or Xdefaults, set in text widget */
		    sprintf(paper_width_str, "%d", current_paper_width);
		    XmTextSetString(size_width_text_id, paper_width_str);
      		  }
		break;

	case k_size_height_st:
		size_height_text_id = w;

		/* ignore carriage returns */
    		XtOverrideTranslations(w, cr_trans_parsed);

		/* get current paper height from widget (in mm) */
		XtSetArg(arglist[0], DvrNpaperHeight, &current_paper_height);
		XtGetValues(viewer, arglist, 1);

		if ( (current_paper_height != 0) &&
		     (current_size_unit   != DVR_CHAR_UNITS) )

                    /*  size must have been via Xdefaults,
		     *  use mm (default unit from widget)
		     */
		    current_size_unit = DVR_MM_UNITS;

		else
		  {
		    /*  convert to characters (default unit from command line)
		     *  for text box
		     */
		    current_size_unit = DVR_CHAR_UNITS; /* default */
		    temp_float = current_paper_height/MM_PER_CHAR_H;
		    current_paper_height = (int) (temp_float + 0.5);
		  }

    		if (current_paper_height != 0)
      		  {
		    /* if set on command line or Xdefaults, set in text widget */
	  	    sprintf(paper_height_str, "%d", current_paper_height);
	  	    XmTextSetString(size_height_text_id, paper_height_str);
		  }
		break;

	case k_ps_comments:
		ps_comments_id = w;
		break;

	case k_ps_bwidths:
		ps_bwidths_id = w;
		break;

	case k_ps_fake_trays:
		ps_fake_trays_id = w;
		break;

	case k_ps_draw_mode:
		ps_draw_mode_id = w;
		break;

        case k_ps_scale:
		ps_scale_id = w;
		break;

	case k_ps_op_dbox:
		ps_op_id = w;
		break;

	case k_pap_ok_but:
		pap_ok_id = w;
		break;

	case k_pap_can_but:
		pap_can_id = w;
		break;

	case k_0_deg_tb:
		orientGroup[DEGREES_0] = w;
		break;

	case k_90_deg_tb:
		orientGroup[DEGREES_90] = w;
		break;

	case k_180_deg_tb:
		orientGroup[DEGREES_180] = w;
		break;

	case k_270_deg_tb:
		orientGroup[DEGREES_270] = w;
		break;




      }
}


/*++
 *
 *  FUNCTION NAME:
 *
 *      main_condition_handler()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      This routine is called when an exception or condition occurs which
 *      would ordinarily cause the Viewer, and anyone who invoked the Viewer,
 *      to exit.  This routine is specifically to take care of conditions
 *      raised by the ISL, and IDS support routines.  Some of these
 *      are "bad" in that they call LIB$STOP instead of just returning a
 *      bad status.  This routine catches the bad status and returns it as
 *      the value of the shell routine surrounding each support routine call.
 *
 *
 *  FORMAL PARAMETERS:
 *
 *      signal  - signal vector
 *      mechanism - error mechanism arguments
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      resignals
 *
 *  FUNCTION VALUE:
 *
 *      Whatever error status caused the condition
 *
 *  SIDE EFFECTS:
 *      None
 *
 *--
 */
#ifdef __vms__
int main_condition_handler(signal, mechanism)
 struct chf$signal_array    *signal;
 struct chf$mech_array      *mechanism;
{

   if (signal->chf$l_sig_name == X$_ERROREVENT)
       /* on vms, x errors are signalled up as well; do not
	* sig-to-ret on these; error is handled by X,
	* processing should continue
	*/
       return(DVR_NORMAL);

    if (signal->chf$l_sig_name != SS$_UNWIND)

      {
    	/*  if we're not just unwinding,
	 *  call our callback proc with error reason and status
	 */

    	DvrCallbackStruct   dvr_reason;

    	dvr_reason.reason = DvrCRcdaError;

    	dvr_reason.status = signal->chf$l_sig_name;

    	dvr_reason.string_ptr = NULL;

    	activate_proc(viewer, &(signal->chf$l_sig_name), &dvr_reason);
      }

   /*
    * if severe, return from the procedure which established this exception
    * handler; if not severe, processing will be continued immediately
    * after problem which signalled handler.
    */

    if ( (SEVERITY(signal->chf$l_sig_name) != INFO_STATE) &&
	 (SEVERITY(signal->chf$l_sig_name) != WARNING_STATE) )
      {

        /* reset cursor; (I'm not sure if this is still necessary
	 * since we no longer call IDS)
         */
    	reset_cursor(viewer);

        LIB$SIG_TO_RET( signal, mechanism );

      }

}
#endif


/*
**++
**  ROUTINE NAME:
**      set_ps_initial_values()
**
**  FUNCTIONAL DESCRIPTION:
**      this routine is called after the paper size dialog box has been
**      created; It queries the viewer widget to find out if any of the
**      ps specific resources have been set via defaults files; if so,
**	we set the applications UI to reflect such.
**
**  FORMAL PARAMETERS:
**      none
**
**  SIDE EFFECTS:
**   none
**--
**/

void set_ps_initial_values()

{
    Arg arg_list[10];
    int arg_count = 0;

    Boolean use_comments,
	    use_bwidths,
	    use_trays,
	    watch_progress;

    int	    orientation,
	    scale_factor;

    XtSetArg(arg_list[arg_count], DvrNuseBitmaps, &use_bwidths);
    arg_count++;
    XtSetArg(arg_list[arg_count], DvrNuseComments, &use_comments);
    arg_count++;
    XtSetArg(arg_list[arg_count], DvrNuseTrays, &use_trays);
    arg_count++;
    XtSetArg(arg_list[arg_count], DvrNwatchProgress, &watch_progress);
    arg_count++;
    XtSetArg(arg_list[arg_count], DvrNorientation, &orientation);
    arg_count++;
    XtSetArg(arg_list[arg_count], DvrNscaleValue, &scale_factor);
    arg_count++;

    /* get the widget's values */
    XtGetValues(viewer, arg_list, arg_count);

    /* set our UI to refect widget state */

    (void) XmToggleButtonSetState(ps_comments_id, use_comments, FALSE);
    (void) XmToggleButtonSetState(ps_bwidths_id,  use_bwidths, FALSE);
    (void) XmToggleButtonSetState(ps_fake_trays_id, use_trays, FALSE);
    (void) XmToggleButtonSetState(ps_draw_mode_id, watch_progress, FALSE);

    switch (orientation)
      {
	case 0:
	  (void) XmToggleButtonSetState(orientGroup[DEGREES_0], TRUE, TRUE);
	  break;

	case 90:
	  /* note 90 and 270 are backwards in ps widget */
	  (void) XmToggleButtonSetState(orientGroup[DEGREES_270], TRUE, TRUE);
	  break;

	case 180:
	  (void) XmToggleButtonSetState(orientGroup[DEGREES_180], TRUE, TRUE);
	  break;

	case 270:
	  (void) XmToggleButtonSetState(orientGroup[DEGREES_90], TRUE, TRUE);
	  break;

	default:
	  break;
      }

    XmScaleSetValue(ps_scale_id, scale_factor);
}


/* cbr stuff */

/*
**++
**  ROUTINE NAME:
**	Dvr__DECW_Cbr_Clear_Mode()
**
**  FUNCTIONAL DESCRIPTION:
**      This clears the CBR mode returning Viewer operation to normal.
**
**  FORMAL PARAMETERS:
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

CDAstatus Dvr__DECW_Cbr_Clear_Mode ()
{
    cbr_mode         = 0;
    cbr_frg_ref_cb   = NULL;
    cbr_frg_ref_parm = NULL;
    cbr_ref_buttons  = 0;

    return(DVR_NORMAL);
}



/*
**++
**  ROUTINE NAME:
**	Dvr__DECW_Cbr_Set_Mode(mode, frg_ref_cb, frg_ref_parm, button_flag )
**
**  FUNCTIONAL DESCRIPTION:
**      This sets the CBR mode, allowing for highlighting and navigation
**
**  FORMAL PARAMETERS:
**	unsigned long mode;			Sets the static variable cbr_mode to be on/off
**	unsigned long (*frg_ref_cb) ();		CBR Callback routine
**	unsigned long frg_ref_parm;		CBR Callback routine user parameter
**	unsigned long button_flag;		Button Flag
**	unsigned long *viewwgt_ptr;		Viewer Widget to be returned to the application (widget?)
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

CDAstatus Dvr__DECW_Cbr_Set_Mode (mode, new_file_cb, frg_ref_cb, frg_ref_parm, button_flag, viewwgt_ptr)
    CDAuint32 mode;
    CDAstatus (*new_file_cb) ();
    CDAstatus (*frg_ref_cb) ();
    CDAuserparam frg_ref_parm;
    CDAuint32 button_flag;
    CDAaddress viewwgt_ptr;
{
    if (mode > 0) {
        cbr_mode         = mode;
        cbr_new_file_cb  = new_file_cb;
        cbr_frg_ref_cb   = frg_ref_cb;
        cbr_frg_ref_parm = frg_ref_parm;
        cbr_ref_buttons  = button_flag;
        cbr_viewwgt_ptr  = viewwgt_ptr;

        if (cbr_frg_ref_cb == 0) {
            printf("Error in Reference Callback -- Clearing CBR mode\n");
            Dvr__DECW_Cbr_Clear_Mode ();
        } else {
            if (*cbr_frg_ref_cb == 0) {
                printf("Error in Reference Callback -- Clearing CBR mode\n");
                Dvr__DECW_Cbr_Clear_Mode ();
            } else {
                if (cbr_frg_ref_parm == 0) {
                    printf("Error in Reference Callback -- Clearing CBR mode\n");
                    Dvr__DECW_Cbr_Clear_Mode ();
                }
            }
        }
    } else {
        cbr_mode         = 0;
        cbr_frg_ref_cb   = NULL;
        cbr_frg_ref_parm = NULL;
        cbr_ref_buttons  = 0;
    }
    return(DVR_NORMAL);
}

/* end cbr stuff */


/*
**++
**  ROUTINE NAME:
**	dvr_app_check_for_xdps (vw)
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
**	A boolean that indicates if XDPS is available
**
**  SIDE EFFECTS:
**	none
**--
**/

static Boolean dvr_app_check_for_xdps (vw)
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
**  Help System errors also could be fatal
*/

static void help_error (problem_string, status)
    char	* problem_string;
    int		status;

{
    if (status != 1)
       {
       printf("%s, %x\n", problem_string, status);
       exit(0);
       }
}

/*
**++
**  ROUTINE NAME:
**
**  static void help_system_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  Help System Callback. Creates a help system session
**
**  FORMAL PARAMETERS:
**
**	Widget			w;
**	char			* tag;
**	XmAnyCallbackStruct	* reason;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	none
**--
**/

static void help_system_proc ( w, tag, reason)
    Widget		w;
    char		* tag;
    XmAnyCallbackStruct	* reason;
{

    int			stat;
    MrmCode		dummy_type;	/* Dummy parameter passed to the fetch literal routine */

#ifdef CDA_HYPER_HELP
       /*  Set up Help System environment */
       if (!hyper_help_open)
          {
          DXmHelpSystemOpen (&help_context, toplevel, viewer_help, help_error,
                                                "Help System Error");
          hyper_help_open = TRUE;
          }

       DXmHelpSystemDisplay (help_context, viewer_help, "topic", tag,
				help_error, "Help System Error");
#else
       {
       if ( (w == file_sel_box) || (w == options_file_sel_box))
          (void) on_help_proc (w, k_help_help, reason);
       }
#endif
}

/*
**++
**  ROUTINE NAME:
**
**  static void help_activate_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  This is a call back routine for the entries under Help pull down menu.
**
**
**  FORMAL PARAMETERS:
**
**	Widget			w;
**	int			* tag;
**	XmAnyCallbackStruct	* reason;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	none
**--
**/

static void help_activate_proc (w, tag, reason)
    Widget		w;
    int			* tag;
    XmAnyCallbackStruct	* reason;
{
    int		widget_num = *tag;
    XmString	topic;
    String	help_topic;
    CDAstatus	ret_status;

#ifdef CDA_HYPER_HELP
    /*  Set up Help System environment */
    if (!hyper_help_open)
       {
       DXmHelpSystemOpen (&help_context, toplevel, viewer_help, help_error,
                                                "Help System Error");
       hyper_help_open = TRUE;
       }
#endif
 
    switch ( widget_num)
      {
      case k_help_help :
#ifdef CDA_HYPER_HELP
             {
	     ret_status = fetch_simple_string("dvr$help_overview",&help_topic);
     	     if_error_report (ret_status);
             DXmHelpSystemDisplay (help_context, viewer_help, "topic", help_topic,
					    help_error, "Help System Error");
	     }
#else
 	     (void) on_help_proc (w, tag, reason);
#endif
	   break;

      case k_help_window :
#ifdef CDA_HYPER_HELP
	     {
	     ret_status = fetch_simple_string("dvr$help_decbasics",&help_topic);
     	     if_error_report (ret_status);
             DXmHelpSystemDisplay (help_context, viewer_help, "topic", help_topic,
					    help_error, "Help System Error");
	     }
#else
             (void) help_proc (w, tag, reason);
#endif
	   break;

      case k_help_version :

#ifdef CDA_HYPER_HELP
	     {
	     ret_status = fetch_simple_string("dvr$help_about",&help_topic);
     	     if_error_report (ret_status);
             DXmHelpSystemDisplay (help_context, viewer_help, "topic", help_topic,
					    help_error, "Help System Error");
	     }
#else
             (void) about_proc (w, tag, reason);
#endif
	   break;

      case k_help_context :
	   tracking_help ();
	   break;

      }

}

/*
**++
**  ROUTINE NAME:
**
**  static void tracking_help
**
**  FUNCTIONAL DESCRIPTION:
**
**  Switches Viewer into context-sensitive mode and calls the selected widget's
**  context-sensitive help callback
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
**
**  SIDE EFFECTS:
**	none
**--
**/
static void tracking_help ()
{
#ifdef CDA_HYPER_HELP
       DXmHelpOnContext (toplevel, FALSE);
#endif
}

/*
**++
**  ROUTINE NAME:
**	set_viewer_widget_callbacks(vw)
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function set the help callbacks on Viwer Widget Children such
**	as arrow and page buttons.
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
**
**  SIDE EFFECTS:
**	none
**--
**/

static void  set_viewer_widget_callbacks(vw)
    Widget	vw;				/* widget associated with a window	*/

{
    Widget		widget_id;
    DvrViewerWidget	viewer_widget;
    CDAstatus		ret_status;
    String		help_topic;

    if (vw != NULL)
       {

       viewer_widget = (DvrViewerWidget)vw;

       /* Top or first page of the document button */
       widget_id = TopBut(viewer_widget);
       ret_status = fetch_simple_string("dvr$help_left_arrow_vbar",&help_topic);
       if_error_report (ret_status);
       set_hyper_help_cbs (widget_id, help_topic);

       /* Previous page of the document button */
       widget_id = PrevBut(viewer_widget);
       ret_status = fetch_simple_string("dvr$help_left_arrow",&help_topic);
       if_error_report (ret_status);
       set_hyper_help_cbs (widget_id, help_topic);

       /* Last page of the document button */
       widget_id = BotBut(viewer_widget);
       ret_status = fetch_simple_string("dvr$help_right_arrow_vbar",&help_topic);
       if_error_report (ret_status);
       set_hyper_help_cbs (widget_id, help_topic);

       /* Next page of the document button */
       widget_id = NextBut(viewer_widget);
       ret_status = fetch_simple_string("dvr$help_right_arrow",&help_topic);
       if_error_report (ret_status);
       set_hyper_help_cbs (widget_id, help_topic);

       /* Goto page button */
       widget_id = GotoBut(viewer_widget);
       ret_status = fetch_simple_string("dvr$help_page_but",&help_topic);
       if_error_report (ret_status);
       set_hyper_help_cbs (widget_id, help_topic);

       /* Vertical slide bar button */
       widget_id = Vscr(viewer_widget);
       ret_status = fetch_simple_string("dvr$help_slide_rs",&help_topic);
       if_error_report (ret_status);
       set_hyper_help_cbs (widget_id, help_topic);

       /* Horizontal slide bar button */
       widget_id = Hscr(viewer_widget);
       ret_status = fetch_simple_string("dvr$help_slide_rs",&help_topic);
       if_error_report (ret_status);
       set_hyper_help_cbs (widget_id, help_topic);

#ifdef CDA_AUDIO_SUPPORT
       /* Audio button */
       widget_id = AudioTog(viewer_widget);
       if ( widget_id != NULL)
          {
          ret_status = fetch_simple_string("dvr$help_audio_button",&help_topic);
          if_error_report (ret_status);
          set_hyper_help_cbs (widget_id, help_topic);
          }
#endif
       }

}

static void if_error_report (status)

    CDAstatus	status;
{
   if (status != DVR_NORMAL)
      {
      fprintf(stderr, dvr_drm_str_ffail_str);

#ifdef __vms__
	exit(DVR_DRMSTRINGFETCHFAIL);
#else
	exit(1);
#endif
      }
}
