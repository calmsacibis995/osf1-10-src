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
#define LS_PRINTF   
#define SYNC_ON_SCROLL
*/
/*=======================================================================
*
*                COPYRIGHT (c) 1988, 1989, 1991, 1992 BY
*	      DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*
* THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
* ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
* INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
* COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
* OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
* TRANSFERRED. 
*
* THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
* AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
* CORPORATION.
*
* DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
* SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*
*=======================================================================
*/

/******************************************************************************/
/*									      */
/*   FACILITY:								      */
/*									      */
/*        SVN -- Structured Visual Navigation Widget 			      */
/*									      */
/*   ABSTRACT:								      */
/*									      */
/*									      */
/*   AUTHORS:								      */
/*									      */
/*									      */
/*   CREATION DATE:							      */
/*									      */
/* MODIFICATION HISTORY:						      */
/*									      */
/*	083	CS			18-Jan-1993			      */
/*		Changed dxm_externaldef to externaldef.  Remove include of    */
/*		XUICompat.h.						      */
/*	082	AN			03-Nov-1992			      */
/*		Put fix in LclKeyScrollDown for ACCVIO when trying to scroll  */
/*		past last entry known to SVN.				      */
/*	081	AN			20-Oct-1992			      */
/*		Add new field to SvnEventInfo structure to contain XEvent     */
/*		structure.						      */
/*	080	AN			12-Aug-1992			      */
/*		Fixed bug in svn_button1_up routine, where we have to make    */
/*		sure to check to see if the location_cursor_entry == NULL     */
/*		before trying to draw the location cursor around it.	      */
/*	079	AN			11-Aug-1992			      */
/*		Fixed bug in svn_button2_down, instead of checking for NULL   */
/*		for the selectionsDraggedCallback field, do a XtHasCallbacks  */
/*	078	CS			 3-Apr-1992			      */
/*		Add initialization of last_event to Initialize.		      */
/*	077	CS			20-Mar-1992			      */
/*		Fix LclManageDragging so it handles multiple level 0 entries  */
/*		when dragging to the top.				      */
/*	076	AN			21-Jan-1992			      */
/*		Fixed bug in DXmSvnDeleteEntries so that if all entries were  */
/*		deleted from SVN, then the margin_width resource would be     */
/*		zeroed out...						      */
/*	075	AN			 8-Jan-1992			      */
/*		Fixed bug where sometimes location cursor would not be erased */
/*		when the SVN window would lose focus.			      */
/*	074	CS			 2-Jan-1992			      */
/*		Changed field referenced for DXmSvnNprimaryWindowWidget and   */
/*		DXmSvnNsecondaryWindowWidget.  Moved some of the definition   */
/*		for the SvnWindow class to svnprivate.h.  Changed return      */
/*		value for DXmSvnGetPrimaryWorkWindow and		      */
/*		DXmSvnGetSecondaryWorkWindow.  Fixed marginHeight resource    */
/*	073	AN			02-Jan-1992			      */
/*		Fix bug in svnhelp action proc, so that if there are no	      */
/*		entries in SVN when the osfHelp key is pressed an ACCVIO  will*/
/*		no occur.						      */
/*	072	AN			31-Dec-1991			      */
/*		Fix a bug in LclUpdateToggleRange where the first entry in    */
/*		the toggle range was being re-toggled as it was dragged over. */
/*	071	CS			16-Dec-1991			      */
/*		Fix several problems refrencing svn.location_cursor_entry     */
/*		when it is NULL.					      */
/*	070	CS			13-Dec-1991			      */
/*		Added a check in svn_graphics_expose for tree mode.	      */
/*	069	AN			 5-Dec-1991			      */
/*		Bug fixes in button1_down, button1_up, motion and BT2 code    */
/*		to check to see if num_entries==0 before doing anything.      */
/*	068	CS			 2-Dec-1991			      */
/*		Added another condition to the JUMP decision in		      */
/*		svn_graphics_expose.  Also changed the jump so now jump to    */
/*		the latest scroll bar value.				      */
/*	067	CS			26-Nov-1991			      */
/*		Made changes for NorthWest gravity.			      */
/*	066	CS			22-Nov-1991			      */
/*		Add a call to LclOutlineScrollAdjust from svn_graphics_expose */
/*	065	AN			21-Nov-1991			      */
/*		Bug fixes with new PTR clip window.			      */
/*	064	AN			14-Nov-1991			      */
/*		More changes for new PTR window				      */
/*	063	AN			25-Oct-1991			      */
/*		Redesigned PTR window, so that it is now a separate window    */
/*		This included changes to the widget creation routines,	      */
/*		initialization and setvalues routines.			      */
/*	062	AN			28-May-1991			      */
/*		Take out XFreeGC of grey_gc.. out of Destroy proc.	      */
/*	061	AN			16-May-1991			      */
/*		Put 3th parameter on DXmGetLocaleString routine.	      */
/*	060	AN			14-May-1991			      */
/*		Fix bug in LclProcessAutoScroll where if the show-path-to-root*/
/*		was set but all entries where at the same level, then auto    */
/*		scrolling out of top of window did not work.		      */
/*	059	AN			13-May-1991			      */
/*		Added focus<in,out> action procs to SVN's translation table,  */
/*		so that when the SVN widget loses focus it erases the loc.    */
/*		cursor too, not only in the window widgets.		      */
/*	058	AN			09-May-1991			      */
/*		Fix bug where F4 was not working properly.. had to move to    */
/*		SVN's translation table instead of work window.	(osfMenu)     */
/*	057	AN		       10-Apr-1991			      */
/*		Change references to svn_widget in prototype def's to Widget. */
/*	056	AN			8-Apr-1991			      */
/*		Misc. bug fixes... Made sure that MB2 drag is disabled if the */
/*		selection dragged callback is not set..			      */
/*      055	P. Rollman		3-apr-1991			      */
/*		Add ANSI and non-ANSI function definitions                    */
/*	054	AN			28-Mar-1991			      */
/*		Add support for widget's in live scrolling. And also fix      */
/*		some misc. bugs with live scrolling.			      */
/*	053	AN			20-Mar-1991			      */
/*		Add changes for bug fix, so that when mode is R->L, and tree  */
/*		display mode, the nav. button does not overwrite the bottom   */
/*		scrollbar.						      */
/*	052	AN			14-Mar-1991			      */
/*		changed svn_graphics_expose routine so that clip_count is     */
/*		incremented after the clip array is filled in.		      */
/*	051	AN			11-Mar-1991			      */
/*	        Fix bug in LclDrawGhostBox so that GC field'graphics_exposures*/
/*		is initialized to 0. This bug caused all sorts of X errors    */
/*		when a MB2 drag was tried.				      */
/*	050	AN			26-Feb-1991			      */
/*		Add support in live scrolling for path-to-root line.	      */
/*      049	AN			25-Feb-1991			      */
/*		Add support for live scrolling in SVN.			      */
/*	048	J. VanGilder		19-Feb-1991			      */
/*		change include of I18N.h to DECwI18n.h			      */
/*	047	A. Chan			18-Feb-1991			      */
/*		Add #include <X11/I18NI.h> and the I18nContext field to	      */
/*		the DXmGetLocaleString() call				      */
/*	046	AN			04-Feb-1991			      */
/*		Fix bug in focus in/out routines, so that it checks to see    */
/*		if a location cursor entry exists before trying to draw it.   */
/*	045	AN			28-Jan-1991			      */
/*		Add code to support Ctrl/<Up arrow, Down arrow> , scroll      */
/*		to next/prev level entry.  Does function of buttons on outside*/
/*		of vertical scrollbar.					      */
/*	044	AN			25-Jan-1991			      */
/*		Add code to handle focus in/Out events... So that location    */
/*		cursor is redrawn and/or erased when necessary.  Also	      */
/*		fixed code that creates grey-pixmap for insensitive entries   */
/*		so that it looks alright on 75 DPI monitiors.		      */
/*	043	AN			04-Jan-1991			      */
/*		Changed windowwidget class to be manager class instead of     */
/*		primitive... this is because we broke functionality that      */
/*		widgets could be components of entries.  If the window widget */
/*		drawing area was primitive, then it could have no children.   */
/*	042	AN			04-Jan-1991			      */
/*		Have svnhelp callback routine zero out no-used fields in      */
/*		returned helpRequested callback structure.		      */
/*	041	AN			20-Dec-1990			      */
/*		Add fix to svn_button2_up routine not to check num_selections */
/*		before it returns the callback.  Also put actual delivering of*/
/*		callback for double click in svn_button1_up code instead of   */
/*		svn_button1_down code.					      */
/*	040	AN			11-Dec-1990			      */
/*		Change look of location cursor so it is now a solid line with */
/*		the color of highlight_color... and the highlight feature of  */
/*		SVN is now a dashed line.				      */
/*	039	AN			06-Dec-1990			      */
/*		Fixed some bugs... dragging entry to scrollbar left lines..   */
/*		help action proc did not return correct entry-number, MB3 on  */
/*		entry after a drag event would cause X errors.		      */
/*	038	AN			28-Nov-1990			      */
/*		Changed window widgets to subclass of PRIMITIVE, added	      */
/*		action procs to return event struct. in callbacks, and	      */
/*		added action procs for TAB traversal.			      */
/*	037	AN			20-Nov-1990			      */
/*		Rework all code button extended selection and drag so that    */
/*		enter-leave and motion events were used instead of XQueryPointer*/
/*		Also added auto-scroll with timer events.  Fixed misc. bugs   */
/*		reported. Mostly EFT release code.			      */
/*      036	AN			18-Oct-1990			      */
/*		Bug fix in DeleteEntries routine to recalculate loc. cursor   */
/*		depending on its new position after the delete.		      */
/*	035	AN			17-Oct-1990			      */
/*		Add more keyboard traversal.				      */
/*	034	Jim VanGilder		10-Oct-1990			      */
/*		Add #include DECspecific.h so DXmLoadQueryFont prototype      */
/*		gets defined in order to compile cleanly on Ultrix	      */
/*	033	AN			10-Oct-1990			      */
/*		Fix bug in LclKeyScrollDown so that if loc. cursor is off     */
/*		screen, if would jump to top of display on down arrow.        */
/*	032	AN			05-Oct-1990			      */
/*		Added some more keyboard traversal action procs.	      */
/*	031	AN			27-Sep-1990			      */
/*		Add code to support tree mode in new mouse semantics and      */
/*		add a few keyboard traversal action procs.		      */
/*	030	AN			20-Sep-1990			      */
/*		Redesigned mouse semantics to adhere to Motif Style guide.    */
/*		Mostly all mouse action procs rewritten, also added support   */
/*		for location cursor.					      */
/*      029     S. Lavigne              24-Aug-1990                           */
/*              Integrate the DEC Israel changes into this module - their     */
/*              code is turned on by the macro LayoutIsRtoL.                  */
/*	028	AN			23-Aug-1990			      */
/*		More changes to make manager class and keyboard traversal work.*/
/*	027	AN			20-Aug-1990			      */
/*		Add changes to make SVN subclass off of Motif's manager class.*/
/*		And also add code for keyboard traversal action procs.        */
/*	026	AN			11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	025	AN			28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	024	AN			25-Jun-1990			      */
/*		Changes made so that only compound strings are supported      */
/*		instead of text strings and compound strings.		      */
/*	023	AN			05-Jun-1990			      */
/*		Change XmDefaultFont to DXmDefaultFont.			      */
/*	022	AN			24-May-1990			      */
/*		Initialize columns array in Initialize proc.		      */
/*	021	AN			22-May-1990			      */
/*		Initialize current_entry_number field in svn structure = 0.   */
/*	020	AN			17-May-1990			      */
/*		Fix problem where scrollbar slider would expand so that       */
/*		columns were not accessible after deleting entries.	      */
/*	019	AN			08-May-1990			      */
/*		Take out code in SetValues proc so that all entries are not   */
/*		refreshed if the selection modes have changed.		      */
/*	018	AN			24-Apr-1990			      */
/*		Add code to Realize proc.. so that do_not_propagate bit       */
/*		is turned off of scrolledwindow widgets, their translation    */
/*		tables are uninstalled etc. All because events are not being  */
/*		propagated to root SVN window as in XUI.		      */
/*	017	S. Lavigne 		20-Apr-1990			      */
/*		Rename routine XtGrayPixmap to LclXtGrayPixmap to fix the     */
/*		Mail group's problem when they link Motif SVN code and the    */
/*		XUI toolkit.  This routine will not become a part of the      */
/*		toolkit.						      */
/*	016	AN			26-Mar-1990			      */
/*		Fix bug where if 100% was set for resource SvnNlhsPercentage  */
/*		X errors would occur.					      */
/*	015	Will Walker		06-Feb-1990			      */
/*		Apparently, Motif now takes care of the memory deallocation   */
/*		when callbacks are changed via set values.  Therefore, the    */
/*		LclUpdateCallback routine is no longer needed (as is	      */
/*		SetValuesCallbacks.					      */
/*	014	Will Walker		05-Feb-1990			      */
/*		Add SvnGet{Lhs|Rhs}WorkWidget.				      */
/*	013	S. Lavigne 		01-Feb-1990			      */
/*		To compile cleanly with /STANDARD=PORT, add #pragma standard  */
/*		lines around #include files and externaldefs.		      */
/*      012	Will Walker		31-Jan-1990			      */
/*		Remove include of PaneP.h				      */
/*      011	Will Walker		26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	010	Will Walker		25-Jan-1990			      */
/*		Change DECwWsSvn*.h to DECwMSvn*.h.			      */
/*	009	Will Walker		23-Jan-1990			      */
/*		Convert Svn* constants to SvnK*.			      */
/*	008	Will Walker		23-Jan-1990			      */
/*		Change SvnSelectEntry to SvnKselectEntry.		      */
/*	007	Will Walker		23-Jan-1990			      */
/*		Convert SvnWidget references to svn_widget.		      */
/*	006	S. Lavigne 		23-Jan-1990			      */
/*		Change all BCSESVN references to SVN.  Change high-level      */
/*		routine BcseSvn to SvnWidget and change low-level routine     */
/*		BcseSvnCreate to SvnCreateWidget.  Also, change routine	      */
/*		BcseSvnInitializeForDRM to SvnInitializeForMRM.		      */
/*	005	Will Walker		19-Jan-1990			      */
/*		Modify svnbuttondown/up/shiftbuttondown routines to not	      */
/*		assume that the widget passed is the svn widget.	      */
/*		Remove translation table from widget class.  Instead, create  */
/*		the ClassInitialize routine and have the translation table    */
/*		parsed there.  The translation table will now be used in      */
/*		the subwidgets (lhs_window_widget,rhs_window_widget).  This   */
/*		is a work-around for some problem we haven't identified yet.  */
/*	004	Will Walker		16-Jan-1990			      */
/*		Add XtGrayPixmap code.  See Leo about why Motif dropped	      */
/*		this.							      */
/*	003	Will Walker		15-Jan-1990			      */
/*		Perform post-motif-port modifications.			      */
/*	002	S. Lavigne 		08-Jan-1990			      */
/*		Comment out the #pragma lines.				      */
/*	001	S. Lavigne 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

#define SVN

#ifdef VMS
/*#pragma nostandard*/
#include "XmP.h"
#include "MrmPublic.h"
#include "StringDefs.h"
#include "Text.h"
#include "cursorfont.h"
#include "descrip.h"
#include "DECwI18n.h"
#ifdef _NO_PROTO
extern char *VMSDescToNull();  /* jmg */
#else
extern char *VMSDescToNull(struct dsc$descriptor_s *desc);  /* jmg */
#endif
/*#pragma standard*/
#else
#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <X11/StringDefs.h>
#include <Xm/Text.h>
#include <X11/cursorfont.h>
#include <X11/DECwI18n.h>
#include "DXmPrivate.h"
#endif

#include "Pane.h"
#include "DXmSvnP.h"
#include "svnprivate.h"
#include "DECspecific.h"


/*
**  Defines for amount of time values (in milliseconds)
*/

#define DELAYDOUBLECLICK  250
#define DELAYDRAGGING     250

/*
** Define event types...
*/
#define BUTTONDOWN  1
#define BUTTON2DOWN 2

/*
** Define window leave directions ...
*/
#define TOPLEAVE    1
#define BOTTOMLEAVE 2
#define LEFTLEAVE   4
#define RIGHTLEAVE  8


/*
**  Define modes
*/

#define MODEIDLE          0
#define MODERANGE         1  
#define MODERANGEEXTEND   2  
#define MODEDRAG          3
#define MODEAWAITDOUBLE   4
#define MODEAWAITDRAG     5
#define MODEAPPLDRAG      6

#define not_first     0
#define first_of_one  1
#define first_of_many 3

#define svn_k_motion_threshold 5

/* Local constants for scrolling routines */

#define SCROLL_ENTRY	0
#define SCROLL_PAGE	1
#define SCROLL_TO_TOP	2
#define SCROLL_TO_BOTTOM 3

#ifdef _NO_PROTO
extern void DisplayEraseLocationCursor ();
#else
extern void DisplayEraseLocationCursor (svn_widget svnw);
#endif



/*
**  Forward routine declarations
*/
    static void LclSelectExtendedRange();
    static void LclManageDragging  ();
    static void LclProcessAutoScroll();
    static void LclDrawGhostBox    ();
    static void LclSetGCs          ();
    static void LclUpdateSelectRange();
    static void LclUpdateToggleRange();
    static void LclAutoScrollRange();
    static void LclActivateEntryProc();
    static void LclSelectEntryProc();
    static void LclExtendKeySelectionUp();
    static void LclExtendKeySelectionDown();
    static void LclKeyScrollDown();
    static void LclKeyScrollUp();
    static void LclExtendButtonSelection();

    static void ClassInitialize	();
    static void Initialize	();
    static void Realize		();
    static void Destroy		();
    static void Resize		();
    static void svn_expose(
#ifdef _NO_PROTO
	Widget , XExposeEvent *, Region
#endif
	);
    static void svn_graphics_expose ();
    static Boolean SetValues	();

/*
**  Action routine declarations
*/
    static void svn_button1_up        ();
    static void svn_button1_down      ();
    static void svn_button1_motion     ();
    static void svn_shiftbutton1_down ();
    static void svn_ctrlbutton1_down  ();
    static void svn_button2_up        ();
    static void svn_button2_motion     ();
    static void svn_button2_down      ();
    static void svn_button3_down      ();
    static void svnhelp            ();
    static void svn_focus_out	    ();
    static void svn_focus_in	    ();
    static void svnworkareafixup   ();
    static void svn_arrow_down	    ();
    static void svn_arrow_up	    ();
    static void svn_page_down	    ();
    static void svn_page_up	    ();
    static void svn_scroll_to_top();
    static void svn_scroll_to_bottom();
    static void svn_arrow_left	    ();
    static void svn_arrow_right    ();
    static void svn_scroll_to_left ();
    static void svn_scroll_to_right();
    static void svn_page_left	    ();
    static void svn_page_right	    ();
    static void svn_f4_menu	    ();
    static void svn_activate_entry ();
    static void svn_select_entry   ();
    static void svn_extend_up();
    static void svn_extend_down();
    static void svn_extend_pageup();
    static void svn_extend_pagedown();
    static void svn_extend_to_top(); 
    static void svn_extend_to_bottom(); 
    static void svn_select_all();
    static void svn_deselect_all();
    static void svn_next_column();
    static void svn_prev_column();
#ifdef _NO_PROTO
    static void svn_enter();
#else
static void svn_enter(Widget w, XCrossingEvent *event, char params, int num_params);
#endif
    static void svn_leave();
    static void svn_cancel();
    static void LclTraversePrevTabGroup();
    static void LclTraverseNextTabGroup();
    static void svn_prev_level();
    static void svn_next_level();

/*
**  Translation Table 
*/

static const char translations[] =
	"~Shift ~Ctrl ~Meta ~Help<Btn1Down>: SVNBUTTON1DOWN()\n\
	  Shift ~Ctrl ~Meta ~Help<Btn1Down>: SVNSHIFTBUTTON1DOWN()\n\
	 ~Shift  Ctrl ~Meta ~Help<Btn1Down>: SVNCTRLBUTTON1DOWN()\n\
			    Button1<Motion>  : SVNBUTTON1MOTION()\n\
	 ~Shift ~Ctrl ~Meta ~Help<Btn1Up>:   SVNBUTTON1UP()\n\
	  Shift ~Ctrl ~Meta ~Help<Btn1Up>:   SVNBUTTON1UP()\n\
	 ~Shift  Ctrl ~Meta ~Help<Btn1Up>:   SVNBUTTON1UP()\n\
	 ~Shift ~Ctrl ~Meta ~Help<Btn2Down>: SVNBUTTON2DOWN()\n\
	 ~Shift  Ctrl  ~Help<Btn2Down>: SVNBUTTON2DOWN()\n\
	 ~Shift ~Ctrl  Alt ~Help<Btn2Down>: SVNBUTTON2DOWN()\n\
			    Button2<Motion>  : SVNBUTTON2MOTION()\n\
	 ~Shift ~Ctrl ~Meta ~Help<Btn2Up>:   SVNBUTTON2UP()\n\
	 ~Shift  Ctrl ~Meta ~Help<Btn2Up>:   SVNBUTTON2UP()\n\
	 ~Shift ~Ctrl ~Meta Alt ~Help<Btn2Up>:   SVNBUTTON2UP()\n\
	 ~Shift ~Ctrl ~Meta ~Help<Btn3Down>: SVNBUTTON3DOWN()\n\
				<Enter>:     SVNENTERWINDOW()\n\
				<Leave>:     SVNLEAVEWINDOW()\n\
	 ~Shift ~Meta ~Alt ~Ctrl <Key>osfMenu:	SVNMENU()\n\
	 <FocusOut>:			     SVNFOCUSOUT()\n\
	 <FocusIn>:			     SVNFOCUSIN()\n\
	  ";


/*
**  Action Table
*/

static XtActionsRec ActionsList[] = {
    {"SVNBUTTON1UP",	    (XtActionProc)svn_button1_up},
    {"SVNBUTTON1DOWN",	    (XtActionProc)svn_button1_down},
    {"SVNSHIFTBUTTON1DOWN", (XtActionProc)svn_shiftbutton1_down},
    {"SVNBUTTON1MOTION",    (XtActionProc)svn_button1_motion},
    {"SVNCTRLBUTTON1DOWN",  (XtActionProc)svn_ctrlbutton1_down},
    {"SVNBUTTON2DOWN",	    (XtActionProc)svn_button2_down},
    {"SVNBUTTON2MOTION",    (XtActionProc)svn_button2_motion},
    {"SVNBUTTON2UP",	    (XtActionProc)svn_button2_up},
    {"SVNBUTTON3DOWN",	    (XtActionProc)svn_button3_down},
    {"SVNHELP",		    (XtActionProc)svnhelp},
    {"SVNWORKAREAFIXUP",    (XtActionProc)svnworkareafixup},
    {"SVNENTERWINDOW",	    (XtActionProc)svn_enter},
    {"SVNLEAVEWINDOW",	    (XtActionProc)svn_leave},
    {"SVNFOCUSOUT",	(XtActionProc)svn_focus_out},
    {"SVNFOCUSIN",	(XtActionProc)svn_focus_in},
    {"SVNMENU",		(XtActionProc)svn_f4_menu}
    };





/*
**  Default Widget Resources  -  this is linked into the Class Record
*/

static XtResource resources[] = {
   {DXmSvnNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
     XtOffset(svn_widget,svn.default_fontlist), XtRString, (XtPointer)NULL},

   {DXmSvnNfontListLevel0, XmCFontList, XmRFontList, sizeof(XmFontList),
     XtOffset(svn_widget,svn.level0_fontlist), XtRString, (XtPointer)NULL},

   {DXmSvnNfontListLevel1, XmCFontList, XmRFontList, sizeof(XmFontList),
     XtOffset(svn_widget,svn.level1_fontlist), XtRString, (XtPointer)NULL},

   {DXmSvnNfontListLevel2, XmCFontList, XmRFontList, sizeof(XmFontList),
     XtOffset(svn_widget,svn.level2_fontlist), XtRString, (XtPointer)NULL},

   {DXmSvnNfontListLevel3, XmCFontList, XmRFontList, sizeof(XmFontList),
     XtOffset(svn_widget,svn.level3_fontlist), XtRString, (XtPointer)NULL},

   {DXmSvnNfontListLevel4, XmCFontList, XmRFontList, sizeof(XmFontList),
     XtOffset(svn_widget,svn.level4_fontlist), XtRString, (XtPointer)NULL},

   {XmNforeground, XmCForeground, XtRPixel, sizeof(Pixel),
     XtOffset(svn_widget,svn.foreground_pixel), XtRString, XtExtdefaultforeground},

   {XmNmarginWidth, XmCMarginWidth, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.margin_width), XtRImmediate, (XtPointer)2},

   {XmNmarginHeight, XmCMarginHeight, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.margin_height), XtRImmediate, (XtPointer)2},

   {XmNuserData, XmCUserData, XmRPointer, sizeof(char *),
     XtOffset(svn_widget,svn.user_data), XmRPointer, (XtPointer)NULL},

   {DXmSvnNindentMargin, XmCMarginWidth, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.indent_margin), XtRImmediate, (XtPointer)16},

   {DXmSvnNfixedWidthEntries, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.fixed_width), XtRImmediate, (XtPointer)TRUE}, 

   {DXmSvnNtruncateText, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.truncate_strings), XtRImmediate, (XtPointer)FALSE}, 

   {DXmSvnNnumberOfEntries, XtCIndex, XtRInt, sizeof(int),
     XtOffset(svn_widget,svn.num_entries), XtRImmediate, (XtPointer)0}, 

   {DXmSvnNdisplayMode, XtCIndex, XtRShort, sizeof(short),
     XtOffset(svn_widget,svn.display_mode), XtRImmediate, (XtPointer)DXmSvnKdisplayOutline},

   {DXmSvnNprimaryPercentage, XtCIndex, XtRInt, sizeof(int),
     XtOffset(svn_widget,svn.primary_percentage), XtRImmediate, (XtPointer)50},

   {DXmSvnNtreeStyle, XtCIndex, XtRShort, sizeof(short),
     XtOffset(svn_widget,svn.tree_style), XtRImmediate, (XtPointer)DXmSvnKoutlineTree},

   {DXmSvnNmultipleSelections, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.multiple_selections), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNghostPixmap, XtCPixmap, XtRPixmap, sizeof(Pixmap),
     XtOffset(svn_widget,svn.ghost), XtRPixmap, (XtPointer)NULL},

   {DXmSvnNghostX, XtCX, XtRPosition, sizeof(Position),
     XtOffset(svn_widget,svn.ghost_basex), XtRImmediate, (XtPointer)0},

   {DXmSvnNghostY, XtCY, XtRPosition, sizeof(Position),
     XtOffset(svn_widget,svn.ghost_basey), XtRImmediate, (XtPointer)0},

   {DXmSvnNghostWidth, XtCWidth, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.ghost_width), XtRImmediate, (XtPointer)0},

   {DXmSvnNghostHeight, XtCHeight, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.ghost_height), XtRImmediate, (XtPointer)0},

   {DXmSvnNdefaultSpacing, XmCMarginWidth, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.default_spacing), XtRImmediate, (XtPointer)12},

   {DXmSvnNuseScrollButtons, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.use_scroll_buttons), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNexpectHighlighting, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.expect_highlighting), XtRImmediate, (XtPointer)FALSE},

   {DXmSvnNforceSeqGetEntry, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.force_seq_get_entry), XtRImmediate, (XtPointer)FALSE},

   {DXmSvnNshowPathToRoot, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.show_path_to_root), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNtreeLevelSpacing, XmCMarginWidth, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.level_spacing), XtRImmediate, (XtPointer)40},

   {DXmSvnNtreeSiblingSpacing, XmCMarginWidth, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.sibling_spacing), XtRImmediate, (XtPointer)5},

   {DXmSvnNtreeArcWidth, XmCMarginWidth, XtRDimension, sizeof(Dimension),
     XtOffset(svn_widget,svn.arc_width), XtRImmediate, (XtPointer)15},

   {DXmSvnNtreeCenteredComponents, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.centered_components), XtRImmediate, (XtPointer)FALSE},

   {DXmSvnNtreePerpendicularLines, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.perpendicular_lines), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNtreeIndexAll, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.index_all), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNtreeEntryShadows, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.entry_shadows), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNtreeEntryOutlines, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.tree_entry_outlines), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNnavWindowTitle, XmCXmString, XmRXmString, sizeof(XmString),
     XtOffset(svn_widget,svn.nav_window_title), XtRString, NULL},

   {DXmSvnNcolumnLines, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.column_lines), XtRImmediate, (XtPointer)FALSE},

   {DXmSvnNstartColumnComponent, XtCIndex, XtRShort, sizeof(short),
     XtOffset(svn_widget,svn.start_column_component), XtRImmediate, (XtPointer)0},

   {DXmSvnNsecondaryComponentsUnmapped, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.secondary_components_unmapped), XtRImmediate, (XtPointer)FALSE},

   {DXmSvnNsecondaryBaseX, XtCX, XtRPosition, sizeof(Position),
     XtOffset(svn_widget,svn.secondary_base_x), XtRImmediate, (XtPointer)0},

   {DXmSvnNselectionMode, XtCIndex, XtRShort, sizeof(short),
     XtOffset(svn_widget,svn.selection_mode), XtRImmediate, (XtPointer)DXmSvnKselectEntry},

   {DXmSvnNpaneWidget, XmCUserData, XmRPointer, sizeof(char *),
     XtOffset(svn_widget,svn.pane_widget), XmRPointer, (XtPointer)NULL},

   {DXmSvnNprimaryWindowWidget, XmCUserData, XmRPointer, sizeof(char *),
     XtOffset(svn_widget,svn.primary_form), XmRPointer, (XtPointer)NULL},

   {DXmSvnNsecondaryWindowWidget, XmCUserData, XmRPointer, sizeof(char *),
     XtOffset(svn_widget,svn.secondary_form), XmRPointer, (XtPointer)NULL},

   {DXmSvnNoutlineHScrollWidget, XmCUserData, XmRPointer, sizeof(char *),
     XtOffset(svn_widget,svn.hscroll), XmRPointer, (XtPointer)NULL},

   {DXmSvnNstartLocationCursor, XtCIndex, XtRShort, sizeof(short),
     XtOffset(svn_widget,svn.location_cursor), XtRImmediate, (XtPointer)1},

   {DXmSvnNliveScrolling, XtCBoolean, XtRBoolean, sizeof(Boolean),
     XtOffset(svn_widget,svn.live_scrolling), XtRImmediate, (XtPointer)TRUE},

   {DXmSvnNselectAndConfirmCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.SelectAndConfirm_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNextendConfirmCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.ExtendConfirm_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNentrySelectedCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.EntrySelected_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNentryUnselectedCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.EntryUnselected_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNtransitionsDoneCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.TransitionsDone_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNdisplayChangedCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.DisplayChanged_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNattachToSourceCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.AttachToSource_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNdetachFromSourceCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.DetachFromSource_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNselectionsDraggedCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.SelectionsDragged_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNgetEntryCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.GetEntry_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNdraggingCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.Dragging_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNdraggingEndCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.DraggingEnd_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNhelpRequestedCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.Help_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNpopupMenuCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.PopupMenu_callback), XtRCallback, (XtPointer)NULL},

   {DXmSvnNentryTransferCallback, XtCCallback, XtRCallback, sizeof (XtCallbackList),
     XtOffset (svn_widget,svn.EntryTransfer_callback), XtRCallback, (XtPointer)NULL}

};

/*
**  This is the class geometry management routine.
*/
#ifdef _NO_PROTO
static XtGeometryResult GeometryManager (w, g, r)

    Widget w;
    XtWidgetGeometry *g, *r;
#else
static XtGeometryResult GeometryManager (Widget w, XtWidgetGeometry *g)
#endif

{
/*
**  Fulfill the request from the subwidget
*/
    if (g->request_mode & CWX          ) w->core.x            = g->x;
    if (g->request_mode & CWY          ) w->core.y            = g->y;
    if (g->request_mode & CWWidth      ) w->core.width        = g->width;
    if (g->request_mode & CWHeight     ) w->core.height       = g->height;
    if (g->request_mode & CWBorderWidth) w->core.border_width = g->border_width;


/*
**  Return confirmation
*/
    return XtGeometryYes;
}

/*
**  This is the class record that gets set at compile/link time this is what 
**  is passed to the widgetcreate routine as the the class.  All fields must 
**  be inited at compile time.
**
**  This definition is for BL8.4 and has been compared against the LISTBOX
*/
static CompositeClassExtensionRec CompositeClassExtRec =
    {
    NULL,
    NULLQUARK,
    XtCompositeExtensionVersion,
    sizeof(CompositeClassExtensionRec),
    TRUE,
    };

externaldef(dxmsvnclassrec) DXmSvnClassRec dxmSvnClassRec = 
 {
      {
         /* CORE CLASS RECORD   */
         /* superclass          */   (WidgetClass) &xmManagerClassRec,
         /* class_name          */   DXmSvnClassName,
         /* widget_size         */   sizeof(DXmSvnRec),
         /* class_initialize    */   (XtProc) ClassInitialize,
         /* class_part_init     */   (XtWidgetClassProc) NULL,
         /* class_inited        */   FALSE,
         /* initialize          */   (XtInitProc) Initialize,
         /* initialize_hook     */   (XtArgsProc) NULL,
         /* realize             */   (XtRealizeProc) Realize,
         /* actions             */   ActionsList,
         /* num_actions         */   XtNumber(ActionsList),
         /* resources           */   resources,
         /* num_resources       */   XtNumber(resources),
         /* xrm_class           */   NULLQUARK,
         /* compress_motion     */   FALSE,
         /* compress_exposure   */   FALSE,
         /* compress_enterleave */   FALSE,
         /* visible_interest    */   FALSE,
         /* destroy             */   (XtWidgetProc) Destroy,
         /* resize              */   (XtWidgetProc) Resize,
         /* expose              */   (XtExposeProc)svn_expose,
         /* set_values          */   (XtSetValuesFunc) SetValues,
         /* set_values_hook     */   (XtArgsFunc) NULL,
         /* set_values_almost   */   XtInheritSetValuesAlmost,
         /* get_values_hook     */   (XtArgsProc) NULL,
         /* accept_focus        */   (XtAcceptFocusProc) NULL,
         /* version (temp off)  */   XtVersion,
         /* callback offsetlst  */   NULL,
         /* tm_table            */   translations,
         /* query geometry      */   (XtGeometryHandler) NULL,
         /* display accelerator */   (XtStringProc) NULL,
         /* extension           */   (XtPointer) NULL,
      },

      {
         /* COMP CLASS RECORD   */
         /* childrens geom mgr  */   (XtGeometryHandler) GeometryManager,
         /* set changed proc    */   XtInheritChangeManaged,
         /* add a child         */   XtInheritInsertChild,
         /* remove a child      */   XtInheritDeleteChild,
         /* extension           */   (XtPointer)NULL, 
      },

      {
	 /* Constraint class record */
				     NULL,
				     0,
				     0,
				     NULL,
				     NULL,
				     NULL,
				     NULL,
      },

      {		
	/* MANAGER CLASS        */
	/* translations		*/   XtInheritTranslations,
	/* get resources	*/   NULL,
	/* num get_resources 	*/   0,			
	/* get_cont_resources   */   NULL,
        /* num_get_cont_resources */ 0,					
        /* parent_process       */   XmInheritParentProcess,
	/* extension		*/   NULL,					
      },

      {                            
         /* extension           */   (XtPointer) NULL,
      }
    };

externaldef(dxmsvnwidgetclass) DXmSvnClass dxmSvnWidgetClass
	= (DXmSvnClass) &dxmSvnClassRec;

/* 
**  This is the widget's class initialize routine.
*/

#ifdef _NO_PROTO
static void ClassInitialize ()
#else
static void ClassInitialize (void)
#endif

{
/*    svn_parsed_translations = XtParseTranslationTable(translations); */
}


/* 
**  This is the widget's instance initialize routine.  It is called once for 
**  each widget.
*/

#ifdef _NO_PROTO
static void Initialize (unused_req, svnw)

  svn_widget unused_req, svnw;
#else
static void Initialize (svn_widget unused_req, svn_widget svnw)
#endif

{
/*
**  Routine data declarations.  
*/
    DXmSvnCallbackStruct temp;


/*
**  Local data for the watch cursor
*/
    XColor   cursor_colors [2];
    Font     cursor_font;
    int      cursor_wait;
    Display *display;
    Screen  *screen;
    int	    i;

/*
**  Force minimum width and height of 75
*/
    if (svnw->core.width  < 75)  svnw->core.width  = 75;
    if (svnw->core.height < 75)  svnw->core.height = 75;


/*
**  Set up the colors for the watch cursor
*/
    display = XtDisplay (svnw);
    screen  = DefaultScreenOfDisplay (display);


    cursor_colors[0].pixel = WhitePixelOfScreen (screen);
    cursor_colors[1].pixel = BlackPixelOfScreen (screen);

    XQueryColors (display, DefaultColormapOfScreen(screen), cursor_colors, 2);


/*
**  Create the watch cursor and NULL out the Nav cursor
*/
    svnw->svn.nav_box_cursor = 0;

    svnw->svn.watch_cursor = _DXmCreateWaitCursor(svnw);

/*
**  Set default values for the non-resource fields of the widget.
*/
    svnw->svn.show_selections      =  TRUE;
    svnw->svn.show_highlighting    =  FALSE; 

    svnw->svn.window_basex         =  0;
    svnw->svn.max_width            =  0;
    svnw->svn.secondary_max_width        =  0;
    svnw->svn.disabled_count       =  0;
    svnw->svn.entryPtr             =  (DXmSvnEntryPtr) NULL;
    svnw->svn.num_entries          =  0;
    svnw->svn.num_selections       =  0;
    svnw->svn.num_highlighted      =  0;

    svnw->svn.index_window         =  0;
    svnw->svn.index_window_needed  =  FALSE;
    svnw->svn.index_window_shown   =  0;

    svnw->svn.vscroll              =  (Widget) NULL;
    svnw->svn.top_button           =  (Widget) NULL;
    svnw->svn.bot_button           =  (Widget) NULL;

    svnw->svn.sub_widgets_used     =  FALSE;
    svnw->svn.num_path             =  0;
    svnw->svn.display_changed      =  FALSE;
    svnw->svn.display_count        =  0;
    svnw->svn.entries[0]           =  0;
    svnw->svn.entries[1]           =  1;

    svnw->svn.transitions_made     =  FALSE;

    svnw->svn.cache_pointer        =  (DXmSvnEntryPtr) NULL;
    svnw->svn.cache_number         =  0;
    svnw->svn.range_hook           =  0;
    svnw->svn.current_entry_number =  0;
    svnw->svn.clip_count           =  0;
    svnw->svn.drag_id		   =  0;

/* I18N START */
    if (svnw->svn.default_fontlist == NULL)
        svnw->svn.default_fontlist = 
		(XmFontList) DXmFontListCreateDefault( (Widget) svnw, NULL );
/* I18N END */
    
    /* Store fid of default font */
    
    {
	XmFontContext context;
	XmStringCharSet charset;
	XFontStruct * font_struct;
	int status;

	status = XmFontListInitFontContext(&context, svnw->svn.default_fontlist);
	if (status)
	{
            status = XmFontListGetNextFont(context, &charset, &font_struct);
            XmFontListFreeFontContext(context);
	    if (status) 
        	XtFree(charset);
        }
        if (status)
	    svnw->svn.default_font = font_struct->fid;
	else
	{       
            XFontStruct  *font = DXmLoadQueryFont(display, DXmDefaultFont);
            svnw->svn.default_font = font->fid;
	}
    }

    /* Initialize the column components internal array....  */

    for (i = 0;  i < max_comps;  i++)
	{
        svnw->svn.column_tags      [i] = 0;
        svnw->svn.column_widths    [i] = 0;
        svnw->svn.column_width_set [i] = 0;
        };

/*
**  Allocate the initial Level arrays 
*/
    svnw->svn.max_level = DEFAULT_MAX_LEVEL;
    svnw->svn.levelx = (LevelPtr) XtMalloc (sizeof(int) * (DEFAULT_MAX_LEVEL + 1));
    svnw->svn.levely = (LevelPtr) XtMalloc (sizeof(int) * (DEFAULT_MAX_LEVEL + 1));

    svnw->svn.path_entries = (short *) XtMalloc (sizeof(short) * (DEFAULT_MAX_LEVEL + 1));

/* Needed for live scrolling */

    svnw->svn.grop_pending = FALSE;
    svnw->svn.scroll_x = 0;
    svnw->svn.scroll_y = 0;
    svnw->svn.internal_x = 0;
    svnw->svn.internal_y = 0;
    svnw->svn.scroll_value = 0;
    svnw->svn.internal_value = 0;
    svnw->svn.current_value = 0;
    svnw->svn.basex = 0;
    svnw->svn.basey = 0;
    svnw->svn.ptr_clip_count = 0;

/*  Initialize widget list */
    svnw->svn.widget_list_number = WIDGET_LIST_INCREMENT;
    svnw->svn.widget_list = (WidgetList) XtMalloc(sizeof(Widget) * svnw->svn.widget_list_number);

/*
**  Initialize scroll bar flags
*/
    svnw->svn.vscroll_in_progress = FALSE;
    svnw->svn.hscroll_in_progress = FALSE;

/*
**  Initialize various tree mode values
*/
    svnw->svn.update_in_progress   =  0;
    svnw->svn.tree_width	   =  0;    
    svnw->svn.tree_height	   =  0;    
    svnw->svn.current_entry	   =  (DXmSvnEntryPtr) NULL;
    svnw->svn.location_cursor_entry = (DXmSvnEntryPtr) NULL;
    svnw->svn.anchor_entry =  svnw->svn.location_cursor;
    svnw->svn.last_selected_entry = 0;
    
    svnw->svn.button_mode          = MODEIDLE;
    svnw->svn.timerid              = 0;

    svnw->svn.button_down          = FALSE;
    svnw->svn.button_timerid       = 0;
    svnw->svn.button_waitms        = 470;
    svnw->svn.button_repeatms      = 30;

    svnw->svn.last_event.type	   = 0;
    svnw->svn.last_event.state	   = 0;
    svnw->svn.last_event.time	   = 0;
    svnw->svn.last_event.event	   = NULL;

/*
**  Initialize Nav Window information
*/
    svnw->svn.nav_window_popup	    = (Widget) NULL;
    svnw->svn.nav_window	    = (Widget) NULL;
    svnw->svn.nav_window_box_width  = 0;
    svnw->svn.nav_window_box_height = 0;


/*
**  Get the navigation window title information set up.
*/
    if (svnw->svn.nav_window_title == NULL) 
	svnw->svn.nav_window_title = (XmString)
		DXmGetLocaleString((I18nContext)NULL, "Navigation Window",I18NNOUN | I18NLABEL);
    else
        svnw->svn.nav_window_title = XmStringCopy(svnw->svn.nav_window_title);


/*
**  Initialize various tree parameters
*/
    if (svnw->svn.tree_style == DXmSvnKuserDefinedTree) {
	svnw->svn.tree_connections = FALSE;
	svnw->svn.mapx = 0;
	svnw->svn.mapy = 0;
	}
    else {
	svnw->svn.tree_connections = TRUE;
	};


/*
**  Initialize the graphic contexts.
*/
    LclSetGCs (svnw);


/*
**  Create the children.  They are created here because they need to use
**  the Graphics Contexts created above.
*/
    DisplayCreate (svnw);

    {
    Arg al[1];

    XmAddTabGroup(svnw->svn.primary_window_widget);
    XmAddTabGroup(svnw->svn.secondary_window_widget);  

    XtSetArg(al[0], XmNnavigationType, XmSTICKY_TAB_GROUP);
    XtSetValues(XtParent(svnw->svn.primary_form), al, 1);

    XtSetArg(al[0], XmNnavigationType, XmSTICKY_TAB_GROUP);
    XtSetValues(XtParent(svnw->svn.secondary_form), al, 1);

    XtSetArg(al[0], XmNnavigationType, XmSTICKY_TAB_GROUP);
    XtSetValues(svnw->svn.primary_window_widget, al, 1);

    XtSetArg(al[0], XmNnavigationType, XmSTICKY_TAB_GROUP);
    XtSetValues(svnw->svn.secondary_window_widget, al, 1);

    XtSetArg(al[0], XmNnavigationType, XmSTICKY_TAB_GROUP);
    XtSetValues((Widget)svnw, al, 1);
    }
/*
**  Attach to the source at the resource specified location.
*/
    DXmSvnDisableDisplay ((Widget) svnw);
    temp.reason = DXmSvnCRAttachToSource;
    temp.entry_number = 0;
    temp.component_number = 0;
    temp.entry_level = 0;
    temp.loc_cursor_entry_number = 0;
    XtCallCallbacks ((Widget)svnw, DXmSvnNattachToSourceCallback, &temp);
    DXmSvnEnableDisplay ((Widget) svnw);
}

/*
**  This routine is called when the widget is being destroyed.  It also is 
**  called once per widget instance.  It must ensure that all of the memory
**  allocated by this widget has been released.
*/

#ifdef _NO_PROTO
static void Destroy (svnw)

  svn_widget svnw;
#else
static void Destroy (svn_widget svnw)
#endif

{
/*
**  Routine data declarations
*/
    DXmSvnCallbackStruct temp;


/*
**  Disable (and never re-enable) the widget
*/
    DXmSvnDisableDisplay ((Widget) svnw);


/*
**  We must deallocate all of our structures off of the SVN instance record.
*/
    if (svnw->svn.num_entries != 0)
       StructSvnDeleteEntries (svnw, 0, svnw->svn.num_entries);


/*
**  Now we must detach from the source module.
*/
    temp.reason = DXmSvnCRDetachFromSource;
    XtCallCallbacks ((Widget)svnw, DXmSvnNdetachFromSourceCallback, &temp);


/*
**  Free all the GCs that were created
*/
    XFreeGC (XtDisplay(svnw), svnw->svn.foreground_gc);
    XFreeGC (XtDisplay(svnw), svnw->svn.background_gc);
    XFreeGC (XtDisplay(svnw), svnw->svn.inverse_gc);
    XFreeGC (XtDisplay(svnw), svnw->svn.location_cursor_gc);
    XFreeGC (XtDisplay(svnw), svnw->svn.highlight_gc);
    XFreeGC (XtDisplay(svnw), svnw->svn.inverse_highlight_gc);
    XFreeGC (XtDisplay(svnw), svnw->svn.copyarea_gc);


/*
**  Free these Pixmaps and GCs only if allocated
*/
    if (svnw->svn.drag_gc          ) XFreeGC (XtDisplay(svnw), svnw->svn.drag_gc);
    if (svnw->svn.tree_highlight_gc) XFreeGC (XtDisplay(svnw), svnw->svn.tree_highlight_gc);

    if (svnw->svn.top_pixmap           ) XFreePixmap(XtDisplay(svnw), svnw->svn.top_pixmap);
    if (svnw->svn.bot_pixmap           ) XFreePixmap(XtDisplay(svnw), svnw->svn.bot_pixmap);
    if (svnw->svn.tree_nav_close_pixmap) XFreePixmap(XtDisplay(svnw), svnw->svn.tree_nav_close_pixmap);
    if (svnw->svn.tree_nav_open_pixmap ) XFreePixmap(XtDisplay(svnw), svnw->svn.tree_nav_open_pixmap);
    if (svnw->svn.tree_highlight_pixmap) XFreePixmap(XtDisplay(svnw), svnw->svn.tree_highlight_pixmap);
    if (svnw->svn.outline_nav_pixmap   ) XFreePixmap(XtDisplay(svnw), svnw->svn.outline_nav_pixmap);


/*
**  Free any cursors we created
*/
    if (svnw->svn.nav_box_cursor) XFreeCursor(XtDisplay(svnw),svnw->svn.nav_box_cursor);


/*
**  Free the navigation window title
*/
    if (svnw->svn.nav_window_title != NULL) XmStringFree (svnw->svn.nav_window_title);
}

/*
**  This routine is called when the parent's geometry manager resizes the 
**  widget.  Make sure that proper flags are turned off.
*/

#ifdef _NO_PROTO
static void Resize (svnw)

  svn_widget svnw;
#else
static void Resize (svn_widget svnw)
#endif

{
/*
**  Resize the scroll bars only if we have already been through Realize
*/
    if (XtIsRealized(svnw)) 
       DisplayResize (svnw);
}

/*
**  This routine will redisplay the widget.  We will leave it separate in case
**  we get smart about exposing partial windows.  The draw routine will always
**  be used to display the whole thing...
*/

#ifdef _NO_PROTO
static void svn_expose(w, event, region)

  Widget w;
  XExposeEvent *event;
  Region region; /* unused */
#else
static void svn_expose(Widget w, XExposeEvent *event, Region region)
#endif

{
/*
**  Routine data declarations
*/
    XEvent new_event;
    svn_widget svnw;



/*
**  Go up through the parent chain looking for the svn class widget.  
*/
    svnw = StructFindSvnWidget (w);


/*
**  If this redisplay was for SVN, then leave.
*/
    if (w == (Widget) svnw) return;

#ifdef LS_EVENT
	printf("%s for %s, x = %d, y = %d, width = %d, height = %d, grop = %s, current value = %d, internal value = %d\n",
	       "Expose",
	       (event->window == XtWindow(svnw->svn.primary_window_widget) ? "Main" : "Path-To-Root"),
	       event->x, event->y, event->width, event->height,
	       (svnw->svn.grop_pending ? "TRUE" : "FALSE"),
	       svnw->svn.current_value, svnw->svn.internal_value);
#endif
/*
**  For outline mode, ignore all previous clips
*/
    if ( (svnw->svn.display_mode == DXmSvnKdisplayOutline || svnw->svn.display_mode == DXmSvnKdisplayColumns) &&
    !svnw->svn.grop_pending )
	{
	svnw->svn.clip_count = 0;
	svnw->svn.ptr_clip_count = 0;
	}

    
    /******************************************************************************/
    /*                                                                            */
    /* Add the clipping rectangle to our array of clipping rectangles	          */
    /* Check what window this exposure is for, then save the clip rectangle in    */
    /* in the appropriate structure.						  */
    /*                                                                            */
    /******************************************************************************/
	/* Is the window the PTR window ? */

	if ((event->window == XtWindow(svnw->svn.primary_ptr_widget)) ||
	    (event->window == XtWindow(svnw->svn.secondary_ptr_widget)))
	    {
	    if (svnw->svn.ptr_clip_count < max_clips) 
		{
		svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].x      = event->x;
		svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].y      = event->y;
	    	svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].width  = event->width;
	    	svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].height = event->height;
	    	svnw->svn.ptr_clip_count++;
		}
	    svnw->svn.ptr_clips_window = event->window;
	    
	    /*
	    **  Stack up the rest of the Expose events for this window.
	    */
	    while (XCheckTypedWindowEvent(event->display, event->window, Expose, &new_event ))
		{
		if (svnw->svn.ptr_clip_count < max_clips)
		    {
		    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].x      = new_event.xexpose.x;
		    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].y      = new_event.xexpose.y;
		    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].width  = new_event.xexpose.width;
		    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].height = new_event.xexpose.height;
		    svnw->svn.ptr_clip_count++;
		    }
		}
	    }

	/* Handle exposures for the main primary window */
	else 	    
	    {
	    if (svnw->svn.clip_count < max_clips) 
		{
		svnw->svn.clips[svnw->svn.clip_count].x      = event->x;
		svnw->svn.clips[svnw->svn.clip_count].y      = event->y;
		svnw->svn.clips[svnw->svn.clip_count].width  = event->width;
		svnw->svn.clips[svnw->svn.clip_count].height = event->height;
		svnw->svn.clip_count++;
		}
	    svnw->svn.clips_window = event->window;

	    /*
	    **  Stack up the rest of the Expose events for this window.
	    */
	    while (XCheckTypedWindowEvent(event->display, event->window, Expose, &new_event ))
		{
		if (svnw->svn.clip_count < max_clips)
		    {
		    svnw->svn.clips[svnw->svn.clip_count].x      = new_event.xexpose.x;
		    svnw->svn.clips[svnw->svn.clip_count].y      = new_event.xexpose.y;
		    svnw->svn.clips[svnw->svn.clip_count].width  = new_event.xexpose.width;
		    svnw->svn.clips[svnw->svn.clip_count].height = new_event.xexpose.height;
		    svnw->svn.clip_count++;
		    }
		}
	    }


/*
**  There aren't anymore, commit to the refresh and then reset things
*/
    if (XtIsRealized(svnw)) 
       {
	/*
	**  If there is no more room for clip rectangles, then just draw everything
	**  and return.
	*/
	if ((svnw->svn.clip_count >= max_clips) ||
	    (svnw->svn.ptr_clip_count >= max_clips))
	    {
	     /*
	     **  Eat up all further expose events, and clear the existing clipping
	     **  rectangles.
	     */
	     while (XCheckTypedWindowEvent(event->display, event->window, Expose, &new_event )){};
	     if (svnw->svn.clip_count >= max_clips)
		svnw->svn.clip_count = 0;
	     if (svnw->svn.ptr_clip_count >= max_clips)
		svnw->svn.ptr_clip_count = 0;
		
	     svnw->svn.refresh_all = TRUE;
	    }


         /*
         **  Merge the clip rectangles...
         */
	 if (svnw->svn.clip_count > 1)
	    DisplayMergeClips (svnw->svn.clips, &(svnw->svn.clip_count));

	 if (svnw->svn.ptr_clip_count > 1)
	    DisplayMergeClips (svnw->svn.ptr_clips, &(svnw->svn.ptr_clip_count));


	 /*
	 **  Call the DisplayDraw procedure.
	 */
	 if (!svnw->svn.grop_pending)
	    {
	    DXmSvnDisableDisplay ((Widget) svnw);
	    DisplayDraw (svnw);
	    DXmSvnEnableDisplay ((Widget) svnw);
	    }
       }
}


/*
**  This routine will handle a graphic expose event for the window widgets.  
*/

#ifdef _NO_PROTO
static void svn_graphics_expose(w, event)

  Widget w;
  XGraphicsExposeEvent *event;
#else
static void svn_graphics_expose(Widget w, XGraphicsExposeEvent *event)
#endif

{
/*
**  Routine data declarations
*/
    svn_widget	    svnw;
    int		    offset = 0;
    XEvent	    new_event;
    DXmSvnEntryPtr  svnentry;
    int		    j;
	
/*
**  Go up through the parent chain looking for the svn class widget.  
*/
    svnw = StructFindSvnWidget (w);

/*
**  check return conditions
*/
    if ((svnw->svn.display_mode == DXmSvnKdisplayTree) ||
	(w == (Widget) svnw) ||
	(event->type != GraphicsExpose && event->type != NoExpose))
	return;

#ifdef LS_EVENT
	printf("%s for %s, x = %d, y = %d, width = %d, height = %d, grop = %s, current value = %d, internal value = %d\n",
	       (event->type == NoExpose ? "NoExpose" : "\nGraphicsExpose"),
	       (event->drawable == XtWindow(svnw->svn.primary_window_widget) ? "Main" : "Path-To-Root"),
	       event->x, event->y, event->width, event->height,
	       (svnw->svn.grop_pending ? "TRUE" : "FALSE"),
	       svnw->svn.current_value, svnw->svn.internal_value);
#endif

    
    /******************************************************************************/
    /*                                                                            */
    /* Handle NoExpose event to sync up CopyArea.				  */
    /*                                                                            */
    /******************************************************************************/
    if (event->type == NoExpose) 
	{
#ifdef LS_PRINTF
/*
	printf("#2.5 - NoExpose event... scroll_value = %d, currentV = %d,internal_value= %d\n",
		svnw->svn.scroll_value, svnw->svn.current_value,svnw->svn.internal_value);
*/
#endif
#ifdef LS_EVENT
	if (((svnw->svn.scroll_x == svnw->svn.internal_x) &&
	    (svnw->svn.scroll_y == svnw->svn.internal_y)) &&
	    (svnw->svn.current_value == svnw->svn.internal_value))
	    printf("NoExpose setting grop = FALSE\n");
	else
	    printf("NoExpose calling LclOutlineAdjustDisplayLS\n");
#endif
	if (((svnw->svn.scroll_x == svnw->svn.internal_x) &&
	    (svnw->svn.scroll_y == svnw->svn.internal_y)) &&
	    (svnw->svn.current_value == svnw->svn.internal_value))
		    svnw->svn.grop_pending = FALSE;
	else
	    LclOutlineAdjustDisplayLS(svnw);

	if (!svnw->svn.grop_pending)
	    LclOutlineScrollAdjust(svnw);
	return;
	}


    /*
    **  Add the clipping rectangle to our array of clipping rectangles
    */
    if (svnw->svn.clip_count < max_clips) 
	{
	svnw->svn.clips[svnw->svn.clip_count].x      = event->x;
	svnw->svn.clips[svnw->svn.clip_count].y      = event->y;
	svnw->svn.clips[svnw->svn.clip_count].width  = event->width;
	svnw->svn.clips[svnw->svn.clip_count].height = event->height;
	svnw->svn.clip_count++;
	}

    svnw->svn.clips_window = (Window)event->drawable;


    /* Stack up the rest of the GraphicsExpose events */

    while (XCheckTypedWindowEvent(event->display, event->drawable, GraphicsExpose, &new_event ))
	{
	if (svnw->svn.clip_count < max_clips)
	    {
	    svnw->svn.clips[svnw->svn.clip_count].x      = new_event.xexpose.x;
	    svnw->svn.clips[svnw->svn.clip_count].y      = new_event.xexpose.y;
	    svnw->svn.clips[svnw->svn.clip_count].width  = new_event.xexpose.width;
	    svnw->svn.clips[svnw->svn.clip_count].height = new_event.xexpose.height;
	    svnw->svn.clip_count++;
	    }
	};

    if (svnw->svn.clip_count >= max_clips)
	    {        
    	     while (XCheckTypedWindowEvent(event->display, event->drawable, GraphicsExpose, &new_event )){};
	     svnw->svn.clip_count = 0;
	     svnw->svn.refresh_all = TRUE;
	    }

    /*
    **  Merge the clip rectangles...
    */
    if (svnw->svn.clip_count != 0)
	DisplayMergeClips (svnw->svn.clips, &(svnw->svn.clip_count));

    /******************************************************************************/
    /*                                                                            */
    /* This section of code will handle the graphics exposure...		  */
    /* First we will find out where in the new display, the specified entry num   */
    /* sits.   If its in the path-to-root, we know that we have to just perform   */
    /* a LclOUtlinePositionJump call, if no path-to-root visible then we have to  */
    /* process this graphic-exposure event.					  */
    /*                                                                            */
    /******************************************************************************/
    /* Find out at what offset in the display array is this entry number */
	
    for (offset = 1; ((svnw->svn.entries[offset] != svnw->svn.current_value) &&
		 (offset <= svnw->svn.display_count)); offset++);

#ifdef LS_PRINTF
printf("offset = %d\n", offset);
#endif

    /******************************************************************************/
    /*                                                                            */
    /* Some special cases to position the specified entry under the PTR window    */
    /* by doing a PositionJump ... :						  */
    /*	    1) If we find this entry in the path-to-root entries		  */
    /*	    2) If we need to go to the topmost entry in the display		  */
    /*	    3) If we need to scroll to the bottommost entry of the display	  */
    /*	    4) If the copy height was higher than the primary window		  */
    /*                                                                            */
    /******************************************************************************/
    if (
	(XtIsManaged(svnw->svn.primary_ptr_widget) && (offset <= svnw->svn.ls_num_path)) ||
	((svnw->svn.current_value < svnw->svn.scroll_value) &&	DRAG_TO_TOP(svnw->svn.current_value)) ||
	((svnw->svn.current_value >= svnw->svn.scroll_value) && DRAG_TO_BOTTOM(svnw->svn.current_value)) ||
	(event->height >= PRIMARY_HEIGHT(svnw))
	)
	{
	offset = svnw->svn.display_count + 1;
	}	

    /******************************************************************************/
    /*                                                                            */
    /* The last scroll request was to jump to some entry off of the display.	  */
    /*                                                                            */
    /******************************************************************************/
    if (offset > svnw->svn.display_count)
    	{					
        int i;

	DXmSvnDisableDisplay((Widget) svnw);

#ifdef LS_PRINTF
	printf("#3 - graphics expose JUMP, currentV = %d, internal_value = %d, scroll_value = %d, svn.ls_num_path = %d\n",
		svnw->svn.current_value, svnw->svn.internal_value, svnw->svn.scroll_value, svnw->svn.ls_num_path);
#endif
	/*
	**  Since we have already commited to a JUMP, might as well jump
	**  to the latest scrollbar value.
	*/
	LclOutlinePositionJump(svnw, svnw->svn.internal_value);
	DisplayDraw(svnw);
	svnw->svn.display_changed = TRUE;
	DXmSvnEnableDisplay((Widget) svnw);

	svnw->svn.current_value = svnw->svn.internal_value;
	svnw->svn.scroll_value = svnw->svn.current_value;
	svnw->svn.scroll_y = svnw->svn.internal_y;

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif
	}
    
    /******************************************************************************/
    /*                                                                            */
    /* We know that the LclOutlineAdjustDisplayLS routine did a partial copy of   */
    /* the current display window.. now we must fill in the entries that should   */
    /* be visible because of the scrolling.					  */
    /*                                                                            */
    /******************************************************************************/
    else
	{
#ifdef LS_PRINTF
	printf("#3 - graphics expose DRAW, currentV = %d, internal_value = %d\n",
	    svnw->svn.current_value,svnw->svn.internal_value);
	XFlush(XtDisplay(svnw));
#endif
	/*
	**  Call the DisplayDraw procedure.
	*/
	 DXmSvnDisableDisplay ((Widget) svnw);
	 DisplayDraw (svnw);

	 /* I'm not sure we need these */
	 svnw->svn.clip_count = 0;
	 svnw->svn.ptr_clip_count = 0;
	 svnw->svn.display_changed = TRUE;
	 DXmSvnEnableDisplay ((Widget) svnw);

	 svnw->svn.scroll_value = svnw->svn.current_value;
	 svnw->svn.scroll_y = svnw->svn.internal_y;

	}  /* End of ... entry number not found in current display array */

	    
} /* End of svn_graphics_expose procedure */

/*
**  This procedure is called when a configure notify event has been registered
**  on a child window.  It simply ignores the event and calls the Display 
**  routine
**
**  There is a second parameter that is passed but not used.  It is an XEvent
**  pointer.
*/

#ifdef _NO_PROTO
static void svnworkareafixup (w)

  Widget w;
#else
static void svnworkareafixup (Widget w)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;


/*
**  Go up through the parent chain looking for the svn class widget.  
*/
    svnw = StructFindSvnWidget (w);


/*
**  If the widget passed in is an SVN class, then exposes mean that one of the 
**  work areas is not covering the whole area.  In that case, we will call the 
**  DisplayWorkAreaFixup procedure to clean it up.
*/
    DisplayWorkAreaFixup (svnw);
}

/*
**  This routine will take care of any changes that have been made to resource values.
*/

#ifdef _NO_PROTO
static Boolean SetValues (oldsvnw, unused_req, newsvnw)

  svn_widget oldsvnw;
  Widget unused_req;
  svn_widget newsvnw;
#else
static Boolean SetValues (svn_widget oldsvnw, Widget unused_req, svn_widget newsvnw)
#endif

{
/*
**  Routine data declarations
*/
    Boolean redisplay_secondary = FALSE;    /*  secondary_base_x...          */
    Boolean redisplay_primary   = FALSE;    /*  indent_margin...             */
    Boolean redisplay_both  = FALSE;    /*  Fonts...                     */
    Boolean redisplay_nav   = FALSE;    /*  layout, lines                */

    Boolean recompute_entry = FALSE;    /*  Things like default spacing  */
    Boolean recompute_tree  = FALSE;    /*  Lines...                     */
    Boolean anchor_tree     = FALSE;    



/*
**  The following resources are always readonly and cannot be changed
*/
    newsvnw->svn.num_entries       = oldsvnw->svn.num_entries;
    newsvnw->svn.pane_widget       = oldsvnw->svn.pane_widget;
    newsvnw->svn.primary_window_widget = oldsvnw->svn.primary_window_widget;
    newsvnw->svn.secondary_window_widget = oldsvnw->svn.secondary_window_widget;
    newsvnw->svn.hscroll           = oldsvnw->svn.hscroll;


/*
**  The following resources cannot be changed after realize time
*/
    if (XtIsRealized(oldsvnw))
       {
          newsvnw->svn.expect_highlighting = oldsvnw->svn.expect_highlighting;
          newsvnw->svn.use_scroll_buttons  = oldsvnw->svn.use_scroll_buttons;
       };


/*
**  If use_scroll_buttons if different, then unmanage only...
*/
    if (newsvnw->svn.use_scroll_buttons == 0)
       if (oldsvnw->svn.use_scroll_buttons)
	  {
            XtUnmanageChild (oldsvnw->svn.top_button); 
            XtUnmanageChild (oldsvnw->svn.bot_button); 
          };

/*
**  These are resources that change where a refresh of both sides will fix 
**  everything up.
*/
    redisplay_both =
	  ((oldsvnw->svn.foreground_pixel   != newsvnw->svn.foreground_pixel ) 
        || (oldsvnw->core.background_pixel  != newsvnw->core.background_pixel)
        || (oldsvnw->svn.default_fontlist   != newsvnw->svn.default_fontlist )
        || (oldsvnw->svn.level0_fontlist    != newsvnw->svn.level0_fontlist  )
        || (oldsvnw->svn.level1_fontlist    != newsvnw->svn.level1_fontlist  )
        || (oldsvnw->svn.level2_fontlist    != newsvnw->svn.level2_fontlist  )
        || (oldsvnw->svn.level3_fontlist    != newsvnw->svn.level3_fontlist  )
        || (oldsvnw->svn.level4_fontlist    != newsvnw->svn.level4_fontlist  )
        || (oldsvnw->svn.margin_height	    != newsvnw->svn.margin_height    )
	|| (oldsvnw->manager.dxm_layout_direction != newsvnw->manager.dxm_layout_direction)
        || (oldsvnw->core.sensitive	    != newsvnw->core.sensitive       ));




/******************************************************************************/
/*                                                                            */
/* See if the selection mode has changed... if so then go thru all entries    */
/* and invalid any entries that are selected.  This way, a redraw will be     */
/* be done on them only.						      */
/*                                                                            */
/******************************************************************************/
    if (oldsvnw->svn.selection_mode != newsvnw->svn.selection_mode)
	{
	DXmSvnEntryPtr svnentry = newsvnw->svn.entryPtr;
	int	    entry_number = 1;
	int	    i;

	for (i = 0; i < newsvnw->svn.num_selections; i++)
	    {
	    while (!svnentry->selected)
		{
		svnentry = svnentry->next;
		entry_number++;
		};

	    DisplaySvnInvalidateEntry(newsvnw, entry_number);
	    svnentry = svnentry->next;
	    entry_number++;
	    };

	};



/******************************************************************************/
/*                                                                            */
/* If the margin_height resource has changed, then we need to do a setvalues  */
/* on the form widget's XmNmarginHeight resource.			      */
/*                                                                            */
/******************************************************************************/
    if (oldsvnw->svn.margin_height != newsvnw->svn.margin_height)
	{
	XtVaSetValues(newsvnw->svn.primary_form,
		      XmNmarginHeight, newsvnw->svn.margin_height,
		      NULL);
	if (newsvnw->svn.display_mode == DXmSvnKdisplayColumns)
	    XtVaSetValues(newsvnw->svn.secondary_form,
			  XmNmarginHeight, newsvnw->svn.margin_height,
			  NULL);
	}

/*
**  These are resource changes where a repaint of the left hand side will fix
**  things up.
*/
    redisplay_primary = 
          ((oldsvnw->svn.indent_margin	   != newsvnw->svn.indent_margin    )
        || (oldsvnw->svn.margin_width	   != newsvnw->svn.margin_width     )
        || (oldsvnw->svn.fixed_width	   != newsvnw->svn.fixed_width      ));


/*
**  The following resource changes affect only the drawing of the right side. 
*/
    redisplay_secondary =
	  ((oldsvnw->svn.column_lines != newsvnw->svn.column_lines)
	|| (oldsvnw->svn.secondary_base_x   != newsvnw->svn.secondary_base_x  ));


/*
**  The following changes will cause all entries to be recomputed.
*/
    recompute_entry =
          ((oldsvnw->svn.truncate_strings        != newsvnw->svn.truncate_strings   )
	|| (oldsvnw->svn.centered_components	 != newsvnw->svn.centered_components) 
	|| (oldsvnw->svn.arc_width		 != newsvnw->svn.arc_width          )
        || (oldsvnw->svn.entry_shadows		 != newsvnw->svn.entry_shadows      )
	|| (oldsvnw->svn.default_spacing         != newsvnw->svn.default_spacing        )
        || (oldsvnw->svn.start_column_component  != newsvnw->svn.start_column_component )
	|| (oldsvnw->manager.dxm_layout_direction != newsvnw->manager.dxm_layout_direction)
        || (oldsvnw->svn.secondary_components_unmapped != newsvnw->svn.secondary_components_unmapped));


/*
**  If display mode changes, we must layout the tree and recalculate the
**  height of each entry.  We know that changing the display mode automatically
**  destroys all of the entry calculations.
*/
    if (oldsvnw->svn.display_mode != newsvnw->svn.display_mode)
       {
         DisplayChangeMode (oldsvnw, newsvnw);
         recompute_entry = FALSE;
       };


/*
**  The following resource changes will cause trees to be recomputed which also
**  means that the navigation window will be in need.
*/
    recompute_tree =
	  ((recompute_entry)
        || (oldsvnw->svn.tree_entry_outlines != newsvnw->svn.tree_entry_outlines)	
	|| (oldsvnw->manager.dxm_layout_direction != newsvnw->manager.dxm_layout_direction)
	|| (oldsvnw->svn.perpendicular_lines != newsvnw->svn.perpendicular_lines));


/*
**  If they are changing the percentage of the left hand side, then change it if it
**  is currently visible.
*/
    if (oldsvnw->svn.primary_percentage != newsvnw->svn.primary_percentage)
       if (newsvnw->svn.display_mode == DXmSvnKdisplayColumns)
	  if (XtIsRealized (oldsvnw))
             {

               /*
               **  The right hand side scroll window is the parent of
               **  the right hand side window widget.
               */
                   Widget secondary_scroll_window = XtParent(oldsvnw->svn.secondary_form);
		   Dimension secondary_width;

	       /*
	       **  Ensure legal values...
	       */
	           if (newsvnw->svn.primary_percentage <   0) newsvnw->svn.primary_percentage = 0;
	           if (newsvnw->svn.primary_percentage > 100) newsvnw->svn.primary_percentage = 100;

               /*
               **  Compute the right and left hand side widths.
               */
		   secondary_width = XtWidth(newsvnw) - button_height - 3;
		   secondary_width = secondary_width - mullion_width;
		   secondary_width = ((100 - newsvnw->svn.primary_percentage) * secondary_width) / 100;

		   /******************************************************************************/
		   /*                                                                            */
		   /* If the with of the secondary comes out to be 0...(meaning that 100 percent was   */
		   /* selected for primary_percentage... then make sure that the window is at least  */
		   /* created with the width of 1.						 */
		   /*                                                                            */
		   /******************************************************************************/
		   if (secondary_width == 0)
			secondary_width = 1;

               /*
	       **  Make the Pane widget readjust
	       */
	           PaneSetMinMax (secondary_scroll_window, secondary_width, secondary_width);
	           PaneSetMinMax (secondary_scroll_window, 0, 0);
	     };


/*
**  We will redisplay the nav window if we either recompute the tree or redisplay
**  the left hand side.
*/
    redisplay_nav = redisplay_primary || redisplay_both || recompute_tree;


/*
**  We need to anchor the tree if we are changing tree styles and are going
**  to be displaying in tree mode or if we have already determined to 
**  recompute the tree.
*/
    anchor_tree =
	  ((recompute_tree)
	|| (oldsvnw->svn.tree_style != newsvnw->svn.tree_style));


/*
**  Do the anchoring of the tree and set the fact that we need to redraw the
**  left hand side.
*/
    if (anchor_tree)
       if (newsvnw->svn.num_entries > 0)
	  {
            newsvnw->svn.display_changed = TRUE;
	    TopTreeSetCurrentEntry(oldsvnw);
	    newsvnw->svn.current_entry = oldsvnw->svn.current_entry;
	    newsvnw->svn.prevx = 0;
	    newsvnw->svn.prevy = 0;
	  };


/*
**  User defined trees get no connections
*/
    newsvnw->svn.tree_connections = TRUE;
    if (newsvnw->svn.tree_style == DXmSvnKuserDefinedTree)
       {
         newsvnw->svn.tree_connections = FALSE;
	 newsvnw->svn.clip_count = 0;
	 newsvnw->svn.ptr_clip_count = 0;
       };

    
    /******************************************************************************/
    /*                                                                            */
    /* See if the location_cursor start position has changed... If so, check to   */
    /* see if the application as not added any entries yet... if not, then place  */
    /* the new value in widget, otherwise, ignore the applications value and place */
    /* the old location cursor entry number back in the widget.		          */
    /*                                                                            */
    /******************************************************************************/
    if (oldsvnw->svn.location_cursor != newsvnw->svn.location_cursor)
	{
	if (oldsvnw->svn.num_entries == 0)  /* No entries added to display yet... */
	    newsvnw->svn.anchor_entry = newsvnw->svn.location_cursor;
	else
	    newsvnw->svn.location_cursor = oldsvnw->svn.location_cursor;
	}

/*
**  If entries need recomputed, then do it...
*/
    if (recompute_entry)
       {
        /*
        **  Local data
	*/
	    int i;
            for (i = 1; i <= newsvnw->svn.num_entries; i++)
		{
	         DXmSvnEntryPtr svnentry = StructGetEntryPtr(newsvnw, i);
	         svnentry->height_adjusted = FALSE;
	        };
            newsvnw->svn.display_changed = TRUE;
	    redisplay_both = TRUE;
       };


/*
**  If we need to update the nav box, then do it.
*/
    if (redisplay_nav)
       {
	 newsvnw->svn.update_nav_window = TRUE;
	 newsvnw->svn.refresh_all = TRUE;
       };


/*
**  If they changed the nav_window_title, then free the old one and make a 
**  copy of the new one...  If the nav window popup has already been 
**  created, then issue a set values on him...
*/
    if (newsvnw->svn.nav_window_title != oldsvnw->svn.nav_window_title) 
       {
         XmStringFree (oldsvnw->svn.nav_window_title);
         newsvnw->svn.nav_window_title = XmStringCopy(newsvnw->svn.nav_window_title);
         if (newsvnw->svn.nav_window_popup != NULL)
	    {
              Arg arguments[2];
	      XtSetArg (arguments[0], XmNdialogTitle, newsvnw->svn.nav_window_title);
              XtSetValues (newsvnw->svn.nav_window_popup, arguments, 1);
            };
       };


/*
**  If we are to redisplay both, then set the individual values.
*/
    if (redisplay_both)
       {
         redisplay_primary = TRUE;
         redisplay_secondary = TRUE;
       };


/*
**  Because the pane widget and it's children are entirely occluding the SVN
**  window, we must generate expose events on the two children.  Return false
**  after that since it does no good.
*/
    if (XtIsRealized (oldsvnw))
       {
	if (redisplay_primary)
	   XClearArea (XtDisplay (newsvnw),
		       XtWindow  (newsvnw->svn.primary_window_widget), 
		       0, 0,
		       XtWidth   (newsvnw->svn.primary_window_widget),
		       XtHeight  (newsvnw->svn.primary_window_widget),
		       TRUE);

	if ((redisplay_secondary) && (XtIsManaged (newsvnw->svn.secondary_window_widget)))
	   XClearArea (XtDisplay (newsvnw),
		       XtWindow  (newsvnw->svn.secondary_window_widget), 
		       0, 0,
		       XtWidth   (newsvnw->svn.secondary_window_widget),
		       XtHeight  (newsvnw->svn.secondary_window_widget),
		       TRUE);
       };

    if (oldsvnw->manager.dxm_layout_direction
                != newsvnw->manager.dxm_layout_direction)
        DisplayResize (newsvnw);

/*
**  Return false in order not to try a clear area.
*/
    return FALSE;
}

/*
**  This routine will realize the widget.  It is responsible for creating the
**  widget window.
*/

#ifdef _NO_PROTO
static void Realize (svnw, valuemask, attributes)

  svn_widget svnw;
  Mask *valuemask;
  XSetWindowAttributes *attributes;
#else
static void Realize (svn_widget svnw, Mask *valuemask, XSetWindowAttributes *attributes)
#endif

{
/*
**  Create the X window
*/
    XtCreateWindow ((Widget)svnw, InputOutput, CopyFromParent, *valuemask, attributes);


/*
**  Realize all of our children because we are going to resize then to fit the
**  current window layout.
*/
    XtRealizeWidget (svnw->svn.pane_widget);
    XtRealizeWidget (svnw->svn.vscroll);
    XtRealizeWidget (svnw->svn.nav_button);
    if (svnw->svn.use_scroll_buttons)
       {
         XtRealizeWidget (svnw->svn.top_button);
         XtRealizeWidget (svnw->svn.bot_button);
       };



    /******************************************************************************/
    /*                                                                            */
    /* Here we want to change a property of the scrolled window widgets that      */
    /* are children of the pane widget.   We need the scrolled window widget to   */
    /* propagate events up to the SVN window.. instead of being relative to the   */
    /* scrolled window.							          */
    /*                                                                            */
    /* Previously before we changed this attribute on the scrolled window widget  */
    /* in Motif, button events (coords returned in called) would be relative to   */
    /* the scrolled window instead of the upper left origin of the SVN window.    */
    /* Therefore if a position of a component field in the right-hand side        */
    /* scrolled window was requested, the coordinates would not be correct.       */
    /*                                                                            */
    /******************************************************************************/
    {
    Widget  secondary_scroll_window = XtParent(svnw->svn.secondary_form);
    Widget  primary_scroll_window = XtParent(svnw->svn.primary_form);
    Mask tempMask = *valuemask;
    

    tempMask |= CWDontPropagate;
    attributes->do_not_propagate_mask = 0;

    XChangeWindowAttributes(
		    XtDisplay(svnw),
		    XtWindow(secondary_scroll_window),
		    tempMask,
		    attributes);
		    	    

    XChangeWindowAttributes(
		    XtDisplay(svnw),
		    XtWindow(primary_scroll_window),
		    tempMask,
		    attributes);

    
    XChangeWindowAttributes(
		    XtDisplay(svnw),
		    XtWindow(svnw->svn.primary_form),
		    tempMask,
		    attributes);

    XChangeWindowAttributes(
		    XtDisplay(svnw),
		    XtWindow(svnw->svn.secondary_form),
		    tempMask,
		    attributes);

    /******************************************************************************/
    /*                                                                            */
    /* Since the scrolled window will except button events, we have to NULL out   */
    /* its translation table and only allow the topmost SVN widget to handle the  */
    /* events.  In XUI, the scrolled window widget DID not except button events   */
    /* so this was not a problem.					          */
    /*                                                                            */
    /******************************************************************************/
    XtUninstallTranslations (primary_scroll_window); 
    XtUninstallTranslations (secondary_scroll_window); 
    XtUninstallTranslations (svnw->svn.primary_form);
    XtUninstallTranslations (svnw->svn.secondary_form);
    
    /******************************************************************************/
    /*                                                                            */
    /* Finally set the do-not-propagate bit on the top SVN parent window.         */
    /*                                                                            */
    /******************************************************************************/
    tempMask |= CWDontPropagate;
    attributes->do_not_propagate_mask =  
         ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

    XChangeWindowAttributes(
		    XtDisplay(svnw),
		    XtWindow(svnw),
		    tempMask,
		    attributes);

    }

    


/*
**  Call the DisplayResize procedure since the scroll bars may have been 
**  created incorrectly.  The height and width fields of the widget are not
**  stable until this phase.
*/
    DisplayResize (svnw);
}

/*
**  Here is the universal routine that registers SVN on behalf of applications.
*/

#ifdef _NO_PROTO
unsigned int DXmSvnInitializeForMRM ()
#else
unsigned int DXmSvnInitializeForMRM (void)
#endif

{
/*
**  Local data declarations
*/
    unsigned int status;
    

/*
**  Initialize MRM... required , just in case user forgot to do this.
*/
    MrmInitialize ();


/*
**  Register the SVN class
*/
    status = MrmRegisterClass (MrmwcUnknown, NULL , "DXmCreateSvn", DXmCreateSvn, (WidgetClass) &dxmSvnClassRec);


/*
**  Register with all caps if that worked
*/
    if (status == MrmSUCCESS)
       {
         status = MrmRegisterClass (MrmwcUnknown, NULL , "DXMCREATESVN", DXmCreateSvn, (WidgetClass) &dxmSvnClassRec);
       };


/*
**  Print a message on failure
*/
    if (status != MrmSUCCESS)
       {
 	 printf ("SVN widget MRM registration failed\n");
       };


/*
**  Return the status
*/
    return status ;
}

/*
**  Here is the SvnWidget public entry point using the C language interface.  The
**  callback parameters may need broken out into separate parameters.  The low
**  level interface should be used in BL1A.
*/

#ifdef _NO_PROTO
Widget DXmSvnWidget (parent, name, x, y, width, height, confirmcallback,
                helpcallback, attachcallback, detachcallback, getentrycallback,
                extendconfirmcallback, entryselectedcallback, 
                entryunselectedcallback, selectionsdraggedcallback, 
                draggingcallback, draggingendcallback)

  Widget          parent; 
  char           *name;
  int             x, y, width, height; 
  XtCallbackList  confirmcallback, helpcallback, attachcallback, detachcallback;    
  XtCallbackList  getentrycallback, extendconfirmcallback, entryselectedcallback;
  XtCallbackList  entryunselectedcallback, selectionsdraggedcallback;
  XtCallbackList  draggingcallback, draggingendcallback;
#else
Widget DXmSvnWidget (
  Widget          parent, 
  char           *name,
  int             x, 
  int             y, 
  int             width,
  int             height,
  XtCallbackList  confirmcallback, 
  XtCallbackList  helpcallback, 
  XtCallbackList  attachcallback,
  XtCallbackList  detachcallback,   
  XtCallbackList  getentrycallback, 
  XtCallbackList  extendconfirmcallback, 
  XtCallbackList  entryselectedcallback,
  XtCallbackList  entryunselectedcallback, 
  XtCallbackList  selectionsdraggedcallback,
  XtCallbackList  draggingcallback, 
  XtCallbackList  draggingendcallback)
#endif

{
/*
**  Routine data declarations
*/
    Arg arglist[20];


/*
**  Pack the supplied arguments for the low level DXmCreateSvn routine.
*/ 
    XtSetArg (arglist[0],  XmNx,                         x                         );
    XtSetArg (arglist[1],  XmNy,                         y                         );
    XtSetArg (arglist[2],  XmNwidth,                     width                     );
    XtSetArg (arglist[3],  XmNheight,                    height                    );
    XtSetArg (arglist[4],  DXmSvnNselectAndConfirmCallback,  confirmcallback           );
    XtSetArg (arglist[5],  DXmSvnNhelpRequestedCallback,     helpcallback              );
    XtSetArg (arglist[6],  DXmSvnNattachToSourceCallback,    attachcallback            );
    XtSetArg (arglist[7],  DXmSvnNdetachFromSourceCallback,  detachcallback            );
    XtSetArg (arglist[8],  DXmSvnNgetEntryCallback,          getentrycallback          );
    XtSetArg (arglist[9],  DXmSvnNextendConfirmCallback,     extendconfirmcallback     );
    XtSetArg (arglist[10], DXmSvnNentrySelectedCallback,     entryselectedcallback     );
    XtSetArg (arglist[11], DXmSvnNentryUnselectedCallback,   entryunselectedcallback   ); 
    XtSetArg (arglist[12], DXmSvnNselectionsDraggedCallback, selectionsdraggedcallback ); 
    XtSetArg (arglist[13], DXmSvnNdraggingCallback,          draggingcallback          );
    XtSetArg (arglist[14], DXmSvnNdraggingEndCallback,       draggingendcallback       );     


/*
**  Call the low level interface routine
*/
    return (DXmCreateSvn (parent, name, arglist, 15));
}

/*
** Here is the DXmCreateSvn public entry point using the C language interface 
*/

#ifdef _NO_PROTO
Widget DXmCreateSvn (parent, name, arglist, argCount)

  Widget   parent;
  char   * name;
  Arg    * arglist;
  int      argCount;
#else
Widget DXmCreateSvn (Widget parent, char *name, Arg *arglist, int argCount)
#endif

{
/*
**  The XtCreateWidget routine will indirectly call the class initialize, and
**  the initialize routine.  In addition, it allocates the class and instance
**  records for the widget.
*/
    return (XtCreateWidget (name, (WidgetClass)dxmSvnWidgetClass, parent, arglist, argCount));
}

/*
** Here is the DXmSvnDisableDisplay public entry point using the C language interface 
*/

#ifdef _NO_PROTO
void DXmSvnDisableDisplay (w)

  Widget w;
  
#else
void DXmSvnDisableDisplay (Widget w)
#endif

  
{
    svn_widget	svnw = StructFindSvnWidget(w);

/*
**  Simply increment the disabled count.
*/
    svnw->svn.disabled_count = svnw->svn.disabled_count + 1;
}

/*
** Here is the DXmSvnEnableDisplay public entry point using the C language interface 
*/

#ifdef _NO_PROTO
void DXmSvnEnableDisplay (w)

  Widget w;
#else
void DXmSvnEnableDisplay (Widget w)
#endif


{
    svn_widget	svnw = StructFindSvnWidget(w);

/*
**  Decrement the disabled count.
*/
    svnw->svn.disabled_count = svnw->svn.disabled_count - 1;


/*
**  Tell the display module to update the screen if this is the last one.
*/
    if (svnw->svn.disabled_count == 0)
       DisplaySvnEnableDisplay (svnw);



/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count < 0)
	XtError(SVN_TOOENABLED);
#endif
}

/*
** Here is the DXmSvnAddEntries public entry point using the C language interface 
*/

#ifdef _NO_PROTO
void DXmSvnAddEntries (w, after_entry, number_of_entries, level, entry_tags, index_window)

  Widget w;
  int after_entry, number_of_entries, level;
  XtPointer *entry_tags;
  int index_window;
#else
void DXmSvnAddEntries (Widget w, int after_entry, int number_of_entries, 
  int level, XtPointer *entry_tags, int index_window)
#endif

{
    svn_widget	svnw = StructFindSvnWidget(w);

/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if (after_entry > svnw->svn.num_entries)
	XtError(SVN_NOSUCHENTNUM);
    if (after_entry < 0)
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Return if they are not adding any...
*/
    if (number_of_entries <= 0) return;


/*
**  Tell the display routine about entries being added.
*/
    DisplaySvnAddEntries (svnw, after_entry, number_of_entries);


/*
**  Add the entries to the structure.
*/
    StructSvnAddEntries (svnw, after_entry, number_of_entries, level, entry_tags, index_window);


/*
**  Bump the number of entries in this SVN instance.
*/
    svnw->svn.num_entries = svnw->svn.num_entries + number_of_entries;


}

/*
** Here is the DXmSvnDeleteEntries public entry point using the C language interface 
*/

#ifdef _NO_PROTO
void DXmSvnDeleteEntries (w, after_entry, number_of_entries)

  Widget w;
  int after_entry, number_of_entries;
#else
void DXmSvnDeleteEntries (Widget w, int after_entry, int number_of_entries)
#endif

{
    int prev_loc_cursor;
    svn_widget	svnw = StructFindSvnWidget(w);


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if (after_entry > svnw->svn.num_entries)
	XtError(SVN_NOSUCHENTNUM);
    if (after_entry < 0)
	XtError(SVN_ILLENTNUM);
    if ((after_entry + number_of_entries) > svnw->svn.num_entries)
	XtError(SVN_DELTOOMANY);
#endif


/*
**  Max out so that we do not delete too many...
*/
    if (svnw->svn.num_entries < after_entry + number_of_entries)
       number_of_entries = svnw->svn.num_entries - after_entry;


/*
**  If we have nothing to delete, then leave
*/
    if (number_of_entries <= 0) return;


/*
**  Ensure that all of the entries being deleted are not selected.
*/
    SelectClearSelections (svnw, after_entry+1, after_entry+number_of_entries);


/*
**  Ensure that all of the entries being deleted are not highlighted.
*/
    SelectClearHighlighting (svnw, after_entry+1, after_entry+number_of_entries);


/*
**  Tell the display routine about entries being deleted.
*/
    DisplaySvnDeleteEntries (svnw, after_entry, number_of_entries);


/*
**  Tell the structure module to delete them.
*/
    StructSvnDeleteEntries (svnw, after_entry, number_of_entries);


/*
**  Decrease the number of entries in this SVN instance.
*/
    svnw->svn.num_entries = svnw->svn.num_entries - number_of_entries;


/*
**  If we now have zero entries, then reset the maximum entry width to zero
**  because some applications reuse SVN widgets.
*/
    if (svnw->svn.num_entries == 0)
       {
        int i;
        svnw->svn.max_width = 0;
        for (i = 0;  i < max_comps;  i++)
          if (svnw->svn.column_width_set [i] == FALSE)
	     {
	     svnw->svn.secondary_max_width = svnw->svn.secondary_max_width - svnw->svn.column_widths[i];
	     svnw->svn.column_widths [i] = 0;
	     }
       };


	
    
    /******************************************************************************/
    /*                                                                            */
    /* Ok..here we update the location cursor:					  */
    /*  1)  If the loc. cursor is before the range deleted, leave as is.	  */
    /*  2)  If there are no more entries after the range is deleted, reset the    */
    /*      location cursor to be 0.						  */
    /*  3) If the loc. cursor was within the range deleted, then reset it to the */
    /*      entry just before the first deleted one.				  */
    /*	4)  If the loc. cursor is after the range deleted, then reset it to be    */
    /*      the old loc. cursor number - the amount deleted.			  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.num_entries == 0)
	{
	svnw->svn.location_cursor_entry = NULL;
	svnw->svn.location_cursor = 1;
	}
    else
	if ((svnw->svn.location_cursor > after_entry)	&&
		(svnw->svn.location_cursor <= (after_entry+number_of_entries)) )
	   {
	   svnw->svn.location_cursor = (after_entry <= 1 ? 1 : after_entry );
	   svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw, svnw->svn.location_cursor);
	   DisplayHighlightEntry(svnw, svnw->svn.location_cursor);
	   }
	else
	    if (svnw->svn.location_cursor > (after_entry + number_of_entries))
		{
		prev_loc_cursor = svnw->svn.location_cursor;
		
		svnw->svn.location_cursor = svnw->svn.location_cursor - number_of_entries;
		svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw, svnw->svn.location_cursor);
		DisplayHighlightEntry(svnw, svnw->svn.location_cursor);
		}

}

/*
** Here is the DXmSvnInvalidateEntry public entry point using the C language interface.
*/

#ifdef _NO_PROTO
void DXmSvnInvalidateEntry (w, entry_number)

  Widget w;
  int entry_number;
#else
void DXmSvnInvalidateEntry (Widget w, int entry_number)
#endif

{
    svn_widget	svnw = StructFindSvnWidget(w);

/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if (entry_number > svnw->svn.num_entries)
	XtError(SVN_NOSUCHENTNUM);
    if (entry_number < 0)
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Tell STRUCTURE to invalidate the entry
*/
    StructSvnInvalidateEntry (svnw, entry_number);


/*
**  Tell display about an entry being invalidated.
*/
    DisplaySvnInvalidateEntry (svnw, entry_number);
}

/*
**  Here is the DXmSvnSetApplDragging public entry point using the C 
**  language interface.  It changes the internal widget mode and calls the
**  display routine to do the insensitizing.
*/

#ifdef _NO_PROTO
void DXmSvnSetApplDragging (w, value)

  Widget w;
  int value;
#else
void DXmSvnSetApplDragging (Widget w, int value)
#endif

{
    svn_widget	svnw = StructFindSvnWidget(w);

/*
**  Tell the display routine about this...
*/
    DisplaySetApplDragging (svnw, value);


/*
**  Turn us into or out of this mode
*/
    if (value) 
         svnw->svn.button_mode = MODEAPPLDRAG;
    else svnw->svn.button_mode = MODEIDLE;
}


/*
**  This is the public entry point for getting the left hand side
**  work widget.  It had been created so people can modify the
**  translation table of SVN in Motif.
*/
#ifdef _NO_PROTO
Widget DXmSvnGetPrimaryWorkWidget (w)

  Widget w;
#else
Widget DXmSvnGetPrimaryWorkWidget (Widget w)
#endif

{
    svn_widget	svnw = StructFindSvnWidget(w);

/*
**  Simply return the widget ID.
*/
    return(svnw->svn.primary_form);
}
/*
**  This is the public entry point for getting the right hand side
**  work widget.  It had been created so people can modify the
**  translation table of SVN in Motif.
*/
#ifdef _NO_PROTO
Widget DXmSvnGetSecondaryWorkWidget (w)

  Widget w;
#else
Widget DXmSvnGetSecondaryWorkWidget (Widget w)
#endif

{
    svn_widget	svnw = StructFindSvnWidget(w);

/*
**  Simply return the widget ID.
*/
    return(svnw->svn.secondary_form);
}


#define inverse_bits  GCFunction   | GCPlaneMask  | GCSubwindowMode | GCFont | GCGraphicsExposures
#define drag_bits     GCFunction   | GCPlaneMask  | GCSubwindowMode | GCFont | GCGraphicsExposures
#define nav_box_bits  GCFunction   | GCPlaneMask  | GCLineWidth | GCFont | GCGraphicsExposures
#define grey_gc_bits  GCTile | GCFillStyle | GCLineWidth | GCFont 


/*
**  This routine will set all the gc's that are used by SVN.  The drawable used
**  to create the GC's are not XtWindow(svnw) but instead we use the routine
**  XDefaultRootWindow(XtDisplay(svnw)).  This is because the window does not
**  exist yet because we are in the Initialize phase and not Realize.
*/

#ifdef _NO_PROTO
static void LclSetGCs (svnw)

  svn_widget svnw;
#else
static void LclSetGCs (svn_widget svnw)
#endif

{
/*
**  Routine data declarations
*/
    XGCValues values;
    static XImage *DashImage;

#ifdef VMS
static const char dashes[32] = {
#else
static unsigned char dashes[32] = {
#endif
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
};
#define DASHES_WIDTH  16
#define DASHES_HEIGHT 16
#define DASHES_OFFSET 0
#define DASHES_FORMAT XYBitmap
#define DASHES_BITMAP_PAD 8
#define DASHES_DEPTH 1
#define DASHES_BYTES_PER_LINE 2

/*
**  Initialize uncreated ones
*/
    svnw->svn.drag_gc = NULL;


/*
**  Get the grey pixmap from the Xt routine, which is now a local routine
**  because it wasn't included in Motif.
*/
    
    values.font		  = svnw->svn.default_font;
    values.line_width     = 0;
    values.fill_style	  = FillTiled;
    values.tile		  = XmGetPixmap (XtScreen((Widget)(svnw)),
					 "50_foreground",
					 svnw->manager.foreground,
					 svnw->core.background_pixel);
					
    svnw->svn.grey_gc       =  XtGetGC((Widget)svnw, grey_gc_bits, &values);

/*
**  Set the common values
*/
    values.function       = GXinvert;
    values.fill_style     = FillTiled;
    values.plane_mask     = svnw->svn.foreground_pixel ^ svnw->core.background_pixel;
    values.foreground     = svnw->svn.foreground_pixel;
    values.background     = svnw->core.background_pixel;
    values.tile           = svnw->svn.grey_pixmap;
    values.line_width     = 0;
    values.dashes	  = 1;
    values.subwindow_mode = IncludeInferiors;
    values.font		  = svnw->svn.default_font;
    values.line_style = LineDoubleDash;
    values.graphics_exposures = FALSE;
    

/*
**  Create the inverted, grey, and foreground 
*/
    svnw->svn.inverse_gc    = XCreateGC (XtDisplay(svnw), 
					 XDefaultRootWindow(XtDisplay(svnw)), 
					 inverse_bits, 
					 &values);

    

    svnw->svn.foreground_gc = XCreateGC (XtDisplay(svnw), 
					 XDefaultRootWindow(XtDisplay(svnw)), 
					 GCForeground | GCBackground | GCFont | GCGraphicsExposures, 
					 &values);

    svnw->svn.copyarea_gc = XCreateGC (XtDisplay(svnw), 
					 XDefaultRootWindow(XtDisplay(svnw)), 
					 GCForeground | GCBackground | GCFont,
					 &values);
/*
**  Create the highlighting GC if highlighting or shadows are enabled
*/
    if (svnw->svn.expect_highlighting || svnw->svn.entry_shadows)
	{
	values.line_width = highlight_width;
	svnw->svn.tree_highlight_gc = XCreateGC (XtDisplay(svnw), 
					         XDefaultRootWindow(XtDisplay(svnw)), 
	                                         GCForeground | GCBackground | GCLineWidth | GCFont | GCGraphicsExposures, 
                                                 &values);
	}

/*
**  Create a special GC for the nav box using the border color if it is
**  different than the foreground color.  Otherwise just use the foregound.
**  Use a 2 pixel wide line.
*/
    values.line_width = 2;
    if (svnw->core.border_pixel != svnw->core.background_pixel)
	values.plane_mask = svnw->core.border_pixel ^ svnw->core.background_pixel;
    else
	values.plane_mask = svnw->svn.foreground_pixel ^ svnw->core.background_pixel;
    svnw->svn.nav_box_gc    = XCreateGC (XtDisplay(svnw), 
                                         XDefaultRootWindow(XtDisplay(svnw)), 
                                         nav_box_bits, 
                                         &values);

/*
**  Create the background
*/
    values.foreground = svnw->core.background_pixel;
    values.background = svnw->svn.foreground_pixel;

    svnw->svn.background_gc = XCreateGC (XtDisplay(svnw), 
                                         XDefaultRootWindow(XtDisplay(svnw)), 
                                         GCForeground | GCBackground | GCFont | GCGraphicsExposures, 
                                         &values);


    /******************************************************************************/
    /*                                                                            */
    /* Create a new GC used for highlight selections... (it used to be the solid  */
    /* 1 pixel wide line that would surround an entry... now it will have to be   */
    /* a dashed line, since the location cursor has to have the attribute of the  */
    /* solid line.								  */
    /*                                                                            */
    /******************************************************************************/
    values.line_width     = 0;
    svnw->svn.highlight_gc = XCreateGC (XtDisplay(svnw),
				        XDefaultRootWindow(XtDisplay(svnw)),
					GCForeground | GCBackground | GCFont | GCLineStyle | GCLineWidth | GCGraphicsExposures,
					&values);

    svnw->svn.inverse_highlight_gc = XCreateGC(XtDisplay(svnw),
					XDefaultRootWindow(XtDisplay(svnw)),
					GCFont | GCLineStyle | GCLineWidth |
					GCPlaneMask | GCSubwindowMode |
					GCForeground | GCBackground | GCGraphicsExposures,
					&values);

    
    {
    DashImage = XCreateImage(XtDisplay(svnw), 
			     DefaultVisualOfScreen(XtScreen(svnw)),
			     DASHES_DEPTH, DASHES_FORMAT, DASHES_OFFSET, 
			     (char *) dashes, DASHES_WIDTH, DASHES_HEIGHT, 
			     DASHES_BITMAP_PAD, DASHES_BYTES_PER_LINE);

    XmInstallImage(DashImage, "highlight_dash");

    values.tile = XmGetPixmap(XtScreen(svnw),
			      "highlight_dash",
			      svnw->manager.highlight_color,
			      svnw->core.background_pixel);
				
    values.foreground = svnw->manager.highlight_color;
    values.fill_style = FillSolid;  /* normal selection mode only for now */
    
    svnw->svn.location_cursor_gc = XCreateGC (XtDisplay(svnw),
					XDefaultRootWindow(XtDisplay(svnw)),
					GCForeground | GCBackground | GCFillStyle | GCTile | GCGraphicsExposures,
					&values);

    }


}

/*
**  This routine draws the box used in dragging.  The x and y positions are
**  assumed to be the base x and base y positions.
*/

#ifdef _NO_PROTO
static void LclDrawGhostBox (svnw, x, y)

  svn_widget svnw;
  int x, y;
#else
static void LclDrawGhostBox (svn_widget svnw, int x, int y)
#endif

{
/*
**  Save the x and y positions
*/
    svnw->svn.ghost_x = x;
    svnw->svn.ghost_y = y;


/*
**  Create the drag GC if we haven't yet
*/
    if (svnw->svn.drag_gc == NULL)
       {
         XGCValues values;
         values.subwindow_mode = IncludeInferiors;
	 values.graphics_exposures = FALSE;
         values.function   = GXxor;
	 values.font = svnw->svn.default_font;
         values.plane_mask = svnw->svn.foreground_pixel ^ svnw->core.background_pixel;
         svnw->svn.drag_gc = XCreateGC (XtDisplay(svnw), XtWindow(svnw), drag_bits, &values);
       };


/*
**  Draw the ghost using the drag graphics context...
*/
    XCopyArea (XtDisplay(svnw), svnw->svn.ghost, XtWindow(svnw), 
	       svnw->svn.drag_gc, 
	       (Position)0, 
	       (Position)0, 
               (Dimension)svnw->svn.ghost_width, 
               (Dimension)svnw->svn.ghost_height,
               (Position)( (LayoutIsRtoL(svnw)) ?
                          svnw->svn.ghost_x + svnw->svn.ghost_basex - svnw->svn.ghost_width
                        : svnw->svn.ghost_x - svnw->svn.ghost_basex),
               (Position)svnw->svn.ghost_y - svnw->svn.ghost_basey);
}

/*
**  This routine is called when the user releases MB3.  This is help.
*/

#ifdef _NO_PROTO
static void svnhelp (w, event)

  Widget w;
  XButtonEvent *event;
#else
static void svnhelp (Widget w, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnCallbackStruct temp;

    svnw = StructFindSvnWidget (w);


/*
**  Pass this directly in the help callback.
*/
    /* If there is no entries in the display just return and ignore this RETURN */

    if (svnw->svn.location_cursor != 0 && svnw->svn.location_cursor_entry != NULL)
	{
	temp.reason      = DXmSvnCRHelpRequested;
	temp.time        = event->time;
	temp.entry_level = svnw->svn.map_level;
	temp.event	     = (XEvent *)event;
	temp.entry_number	= svnw->svn.location_cursor;
	temp.loc_cursor_entry_number = svnw->svn.location_cursor;
	temp.component_number = svnw->svn.location_cursor_entry->selected_comp;
	temp.entry_tag = svnw->svn.location_cursor_entry->entry_tag;
	
	XtCallCallbacks ((Widget)svnw, DXmSvnNhelpRequestedCallback, &temp);
	}
}

/******************************************************************************/
/*                                                                            */
/* svn_focus_out  - This routine is called when SVN window receives a FocusOut*/
/*		    event.  When we get this event we have to erase the	      */
/*		    location cursor from the main window drawing area.        */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_focus_out (w, event)
    Widget		w;
    XFocusChangeEvent	event;
#else
static void svn_focus_out (Widget w, XFocusChangeEvent event)
#endif

{
    svn_widget	svnw = StructFindSvnWidget(w);

    
    /* Check to make sure that there is a a location cursor to erase */
    
    if (svnw->svn.location_cursor_entry != NULL)
	DisplayEraseLocationCursor (svnw);
    
}


/******************************************************************************/
/*                                                                            */
/* svn_focus_in  - This routine is called when SVN window receives a FocusIn  */
/*		    event.  When we get this event we have to draw the	      */
/*		    location cursor on the current entry once again.	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_focus_in (w, event)
    Widget		w;
    XFocusChangeEvent	event;
#else
static void svn_focus_in (Widget w, XFocusChangeEvent event)
#endif

{
    svn_widget	svnw = StructFindSvnWidget(w);

    /******************************************************************************/
    /*                                                                            */
    /* If the location_cursor Pointer is NULL, then we haven't set up the         */
    /* initial location cursor.... do that here.				  */
    /*										  */
    /* Save the entryPtr of the entry that will contain the location cursor.      */
    /* By default, the first entry will contain the location cursor. But the	  */
    /* application can change that by using the external routine		  */
    /* DXmSvnSetLocationCursor...						  */
    /* NOTE - If the application has specified a startLocationCursor entry number */
    /*	      greater than the number of actual entries that exist... it will     */
    /*	      default back to the first entry.					  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.location_cursor_entry == NULL)
	{
	if ((svnw->svn.location_cursor != 1) && (svnw->svn.num_entries >= svnw->svn.location_cursor))
	    {
	    svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw,svnw->svn.location_cursor);
	    svnw->svn.anchor_entry = svnw->svn.location_cursor;
	    }
	else
	    {
	    svnw->svn.location_cursor = 1;
	    svnw->svn.location_cursor_entry = svnw->svn.entryPtr;
	    }
	}

	if (svnw->svn.num_entries != 0)
	    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);

}

/*
**  This routine is called when the user presses MB1.  This routine makes one
**  assumption that may not always be perfectly valid.  This assumption is 
**  that if the doubleclick timer is set, then we own the selections.
*/

#ifdef _NO_PROTO
static void svn_button1_down (svnw, event)

  svn_widget svnw;
  XButtonEvent *event;
#else
static void svn_button1_down (svn_widget svnw, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;
    int prev_loc_cursor;



    /* On a button down... start the traversal back at the primary window widget */
    
    (void) XmProcessTraversal(svnw->svn.primary_window_widget, XmTRAVERSE_CURRENT);
        
    /******************************************************************************/
    /*                                                                            */
    /* If any other button has been depressed with this BTN2 down event, then that*/
    /* would be interpreted as a chorded cancel... kill any timers set, select    */
    /* the location cursor entry and unselect any other entry.			  */
    /*                                                                            */
    /******************************************************************************/
    if ((event->state & Button2Mask) || (event->state & Button3Mask))
	{
	svn_cancel(svnw, event);
	return;
	}

    /******************************************************************************/
    /*                                                                            */
    /* First thing to do is to save state of what this event is...		  */
    /*                                                                            */
    /******************************************************************************/
    svnw->svn.last_event.type = BUTTONDOWN;
    svnw->svn.last_event.state = event->state;
    svnw->svn.last_event.time = event->time;
    svnw->svn.last_event.event = (XEvent *)event;
    svnw->svn.drag_state = 0;	    /* signals a selection event */

/*
**  See if the timer would have fired...  Set the mode idle if we waited too
**  long.
*/
    if (svnw->svn.button_mode == MODEAWAITDOUBLE)
       if (event->time > svnw->svn.button_up_event_time + DELAYDOUBLECLICK)
          svnw->svn.button_mode = MODEIDLE;


/*
**  Lose the timestamp
*/
    svnw->svn.button_up_event_time = (Time) 0;


/*
**  Get the entry that we are positioned on
*/
    DXmSvnMapPosition ((Widget) svnw, event->x, event->y, &temp.entry_number, &temp.component_number, (XtPointer *)&temp.entry_tag);

    if (temp.entry_number == 0)
	{
	svnw->svn.last_event.type = 0;
	return;
	}
    
    svnw->svn.last_event.entry_number = svnw->svn.anchor_entry = temp.entry_number;
    svnw->svn.last_event.compnm = temp.component_number;

    temp.loc_cursor_entry_number = svnw->svn.location_cursor;
    temp.event = (XEvent *)event;

/*
**  Double click
*/
    if (   (svnw->svn.button_mode     == MODEAWAITDOUBLE  ) 
        && (svnw->svn.timer_entry     == temp.entry_number))
       {
	 return;
	 
	 /* Call common routine to activate the entry */
	 
/*	 LclActivateEntryProc(svnw, &temp); */

/*         return; */
       };                 
 

/*
**  Get the entry pointer for this entry.
*/
    svnentry = StructGetEntryPtr (svnw, temp.entry_number);

/*
**  If the entry is insensitive, then ignore it.
*/
    if (svnentry->grayed) return;

    DXmSvnDisableDisplay((Widget) svnw);

/*
**  If we are not showing selections, then unselect all of the entries
*/
    if (svnw->svn.show_selections == FALSE)
       SelectClearSelections (svnw, 1, svnw->svn.num_entries);


/******************************************************************************/
/*                                                                            */
/* Select this entry and deselect all the rest...			      */
/*                                                                            */
/******************************************************************************/
    SelectSelectSet (svnw, temp.entry_number, temp.entry_number, temp.component_number, event->time);

    svnw->svn.last_selected_entry = temp.entry_number;

    DXmSvnEnableDisplay((Widget) svnw);
    
/*
**  Tell the caller that we are done with transitioning selections
*/
    SelectReportTransitions (svnw);


    svnw->svn.button_mode = MODERANGEEXTEND;

}

/******************************************************************************/
/*                                                                            */
/* Routine will handle SHIFT -> MB1 mouse semantics...			      */
/*									      */
/* Here is where the new functionality for the Motif Style Guide mouse	      */
/* semantics is done...  Shifting MB1 (called : BExtend Click) is supposed to */
/* do the following:							      */
/*									      */
/*   If we start with no selections, location cursor on say entry 1, then     */
/*   shift-MB1 it just moves the location cursor, does not move anchor point. */
/*									      */
/*   If say entry 3 is selected, the anchor point and location cursor is on   */
/*   that entry, and say the shift-click is done on an element 5,  entries    */
/*   3,4 and 5 become selected, location cursor moves to entry 5.	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_shiftbutton1_down (w, event)

  Widget w;
  XButtonEvent *event;
#else
static void svn_shiftbutton1_down (Widget w, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnEntryPtr svnentry;
    int entry_number, component_number;
    XtPointer entry_tag;
    int prev_entry;

    svnw = StructFindSvnWidget (w);


/*
**  If we are hiding the selections, then treat this as a non-shift MB1
*/
    if (svnw->svn.show_selections == FALSE)
       {
         svn_button1_down (svnw, event);
         return;
       };


/******************************************************************************/
/*                                                                            */
/* First thing to do is to save state of what this event is...		      */
/*                                                                            */
/******************************************************************************/
    svnw->svn.last_event.type = BUTTONDOWN;
    svnw->svn.last_event.state = event->state;
    svnw->svn.last_event.time = event->time;
    svnw->svn.last_event.event = (XEvent *)event;
    svnw->svn.drag_state = 0;	    /* Signals do selection instead of toggle */

/*
**  Get the entry that we are positioned on
*/
    DXmSvnMapPosition ((Widget) svnw, event->x, event->y, &entry_number, &component_number, &entry_tag);

    if (entry_number == 0)
	{
	svnw->svn.last_event.type = 0;
	return;	/* did click on anything */
	}

    svnw->svn.last_event.entry_number = svnw->svn.last_selected_entry = entry_number;
    
/*
**  Double clicks are handled by the other routine.  Only call it if there is 
**  at least one entry selected and it will pass the double click testing in
**  that other routine.  If the shift double clicked happens on an entry that 
**  is initially selected, then it will not be part of the set.
**
**  If the timer expired, then forget it.
*/
    if (svnw->svn.button_mode == MODEAWAITDOUBLE)
       if (event->time > svnw->svn.button_up_event_time + DELAYDOUBLECLICK)
          svnw->svn.button_mode = MODEIDLE;
     

/*
**  If we are still checking double click and we are still on the same entry,
**  then pass control to the other routine...  Ensure that at least one entry
**  is currently selected...
*/
    if (svnw->svn.button_mode == MODEAWAITDOUBLE) 
       if (svnw->svn.timer_entry == entry_number)
          if (svnw->svn.num_selections > 0)
             {
               svn_button1_down (svnw, event);
               return;
             };


/*
**  Lose the timer and the mode if still awaiting double clicks
*/
    if (svnw->svn.button_mode == MODEAWAITDOUBLE) 
       {
         svnw->svn.button_mode = MODEIDLE;
         svnw->svn.button_up_event_time = (Time) 0;
       };


/*
**  Get the entry pointer for this entry.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);

/*
**  If the entry is insensitive, then ignore it.
*/
    if (svnentry->grayed) return;


/*
**  If multiple selections are not allowed, there is already a selection made,
**  AND it is not this entry, then treat this as a normal MB1 down.
*/
    if ((svnw->svn.multiple_selections == FALSE) && (svnw->svn.num_selections == 1) && (!svnentry->selected))
       {
         svnw->svn.button_mode = MODEIDLE;
         svn_button1_down (svnw, event);
         return;
       };


/* Extend the selection set to contain all entries between anchor point and this selected entry */
    
    LclSelectExtendedRange (svnw, svnw->svn.anchor_entry , entry_number, component_number, event->time );

/*
**  Tell the caller that all transitions are completed.
*/
    SelectReportTransitions (svnw);


/*
**  Set the mode 
*/
    svnw->svn.button_mode = MODEIDLE;


/*
**  If the entry is selected we will if dragging should be turned on
*/
    if (svnentry->selected)
            svnw->svn.button_mode = MODERANGEEXTEND;


}

/******************************************************************************/
/*                                                                            */
/* svn_ctrlbutton1_down    - Handles CRTL -> MB1 mouse semantics.	      */
/*									      */
/* Conditions are as follows :						      */
/*	1) Toggles the selection state of the entry that the MB1 occurred on. */
/*	2) Moves the location cursor and anchor point to that entry.	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_ctrlbutton1_down (w, event)

  Widget w;
  XButtonEvent *event;
#else
static void svn_ctrlbutton1_down (Widget w, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnEntryPtr svnentry;
    int entry_number, component_number;
    XtPointer entry_tag;
    int prev_location_cursor_entry;

    svnw = StructFindSvnWidget (w);

/*
**  Get the entry that we are positioned on
*/
    DXmSvnMapPosition ((Widget) svnw, event->x, event->y, &entry_number, &component_number, &entry_tag);

    if (entry_number == 0) return;
    
/******************************************************************************/
/*                                                                            */
/* First thing to do is to save state of what this event is...		      */
/*                                                                            */
/******************************************************************************/
    svnw->svn.last_event.type = BUTTONDOWN;
    svnw->svn.last_event.state = event->state;
    svnw->svn.last_event.time = event->time;
    svnw->svn.last_event.entry_number = entry_number;
    svnw->svn.last_event.event = (XEvent *)event;
    svnw->svn.drag_state = 1;	    /* Signals a toggle instead of a selection */

    svnw->svn.last_selected_entry = svnw->svn.anchor_entry = entry_number; 

/*
**  Get the entry pointer for this entry.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If multiple selections are not allowed, there is already a selection made,
**  AND it is not this entry, then treat this as a normal MB1 down.
*/
    if ((svnw->svn.multiple_selections == FALSE) && (svnw->svn.num_selections == 1) && (!svnentry->selected))
       {
         svnw->svn.button_mode = MODEIDLE;
         svn_button1_down (svnw, event);
         return;
       };

/*
**  If the entry is insensitive, then ignore it.
*/
    if (svnentry->grayed) return;

    SelectToggleSelections (svnw, entry_number, entry_number, 0, event->time);

/*
**  If we are not showing selections, then unselect all of the entries
*/
    if (svnw->svn.show_selections == FALSE)
       SelectClearSelections (svnw, 1, svnw->svn.num_entries);

/*
**  Tell the caller that we are done with transitioning selections
*/
    SelectReportTransitions (svnw);


/*
**  Set the mode for extend toggle...
**
*/
    svnw->svn.button_mode = MODERANGEEXTEND;


}

/*
**  This routine is called when the mouse button is released.
*/

#ifdef _NO_PROTO
static void svn_button1_up (w, event)

  Widget w;
  XButtonEvent *event;
#else
static void svn_button1_up (Widget w, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;
    int prev_loc_cursor;

    svnw = StructFindSvnWidget (w);


    
    /******************************************************************************/
    /*                                                                            */
    /* Check to see if this button up is to be ignored... meaning that the	  */
    /* previous extended select was canceled by a KCancel (F11) or a chorded cancel */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.last_event.type == 0) return;
    
    /*
    **  Get the entry that we were left on.  We will set the double click timer
    **  only if we were left on a selected entry.
    */
    DXmSvnMapPosition ((Widget) svnw, event->x, event->y, &temp.entry_number, &temp.component_number, (XtPointer *)&temp.entry_tag);

    /* If there are no entries in SVN, then there is nothing to select */
    
    if (svnw->svn.num_entries == 0)
	return;

    svnw->svn.last_event.type = 0;  /* Signal button up to autoscroll code */
    temp.event = (XEvent *)event;

    /*
    **  If we were on whitespace, then we are done.
    */
    if (temp.entry_number == 0)
	{
	/******************************************************************************/
	/*                                                                            */
	/* If the user was in the middle of a range selection, and then released the  */
	/* button, then we just place the location cursor at the last selected entry  */
	/* and return .... otherwise, don't process this upclick.		      */
	/*                                                                            */
	/******************************************************************************/
	if ((svnw->svn.button_mode == MODERANGE) ||
	    (svnw->svn.button_mode == MODERANGEEXTEND))
	    {
	    prev_loc_cursor = svnw->svn.location_cursor;

	    
	    /******************************************************************************/
	    /*                                                                            */
	    /* We have to make sure that the location cursor is always visible on the     */
	    /* display... so if we were in the middle of a autoscroll,then we want to     */
	    /* place the loc. cursor at either the bottom or the top entry in the display.*/
	    /*                                                                            */
	    /******************************************************************************/
	    if (svnw->svn.leave_direction == BOTTOMLEAVE) 
		if (svnw->svn.entries[svnw->svn.display_count] != svnw->svn.last_selected_entry)
		    DXmSvnPositionDisplay((Widget) svnw,svnw->svn.last_selected_entry,DXmSvnKpositionBottom);
		
            if (svnw->svn.leave_direction == TOPLEAVE)
	     	DXmSvnPositionDisplay((Widget) svnw,svnw->svn.last_selected_entry,DXmSvnKpositionTop);
		 
	    if (svnw->svn.drag_id)
		{
		XtRemoveTimeOut(svnw->svn.drag_id);
		svnw->svn.drag_id = 0;
		}

	    svnw->svn.location_cursor = svnw->svn.last_selected_entry;
	    svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw,svnw->svn.location_cursor);
	    DisplayHighlightEntry(svnw, prev_loc_cursor);
	    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);


	    /*
	    **  If we are not showing selections, then unselect all of the entries
	    */
	    if (svnw->svn.show_selections == FALSE)
		SelectClearSelections (svnw, 1, svnw->svn.num_entries);


	    /*
	    **  Tell the caller that we are done with transitioning selections
	    */
	    SelectReportTransitions (svnw);

	    svnw->svn.button_mode = MODEIDLE;
	    
	    return;
	    }
	else return;
	}
	

/*
**  Double click
*/
    if (svnw->svn.button_mode     == MODEAWAITDOUBLE)
       {
	 /* Call common routine to activate the entry */
	 
	 temp.time = event->time;
	 LclActivateEntryProc(svnw, &temp); 
	 svnw->svn.button_mode = MODEIDLE;
         return; 
       };                 
 

    /******************************************************************************/
    /*                                                                            */
    /* If there is a drag timeout set, remove it so we stop scrolling... this	  */
    /* means the user wants the auto scrolling to end.				  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.drag_id)
	{
	XtRemoveTimeOut(svnw->svn.drag_id);
	svnw->svn.drag_id = 0;
	}

    /*
    **  Default the mode to being idle.
    */
    svnw->svn.button_mode = MODEIDLE;


    /*
    **  Get the structure
    */
    svnentry = StructGetEntryPtr (svnw, temp.entry_number);


    /*
    **  If the entry is selected, then set the timer
    */
    if (svnentry->selected)
       {
         svnw->svn.button_mode = MODEAWAITDOUBLE;
         svnw->svn.button_up_event_time = event->time;
         svnw->svn.timer_entry = temp.entry_number;
       };

    /*
    **  If the entry is insensitive, then ignore it.
    */
    if (svnentry->grayed) return;

	/******************************************************************************/
	/*                                                                            */
	/* Make sure that we select or unselect any entries that between the last     */
	/* entry and this current entry that the upclick occurred on... This is	      */
	/* because we don't always get all motion events up to the upclick..?	      */
	/*                                                                            */
	/******************************************************************************/
	if ((temp.entry_number != svnw->svn.last_selected_entry) &&
	    (svnw->svn.multiple_selections == TRUE) ) 
	    {
	    if (svnw->svn.drag_state == 0) 
		LclUpdateSelectRange(svnw, svnentry, temp.entry_number, svnw->svn.last_event.compnm, event->time);
	    else
		LclUpdateToggleRange(svnw, svnentry, temp.entry_number, svnw->svn.last_event.compnm, event->time);
	    }

	/******************************************************************************/
	/*                                                                            */
	/* Before we finish up here, make sure that the location cursor is reset      */
	/* and drawn to it new destination.... The last selected entry should have    */
	/* the location cursor .						      */    
	/*	1) invalidate the entry that currently has the location cursor	      */
	/*	2) Set the location cursor to the new current entry ...		      */
	/*                                                                            */
	/* If the last event was a Ctrl Button1 down, then we don't want to set the   */
	/* location cursor on this Button1Up, since focus does not get transferred    */
	/* on a Ctrl Button1 click, only a Button1.				      */
	/*                                                                            */
	/* NOTE - there might be a case where there is no entries to set the location */
	/*   cursor to.								      */
	/******************************************************************************/
	if ((svnw->svn.last_event.state != (int)ControlMask) &&
	    (svnw->svn.location_cursor_entry != NULL))
	    {
	    prev_loc_cursor = svnw->svn.location_cursor;
	    svnw->svn.location_cursor = svnw->svn.last_selected_entry;
	    svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw,svnw->svn.location_cursor);
	    DisplayHighlightEntry(svnw, prev_loc_cursor);
	    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);
	    }

/*
**  If we are not showing selections, then unselect all of the entries
*/
    if (svnw->svn.show_selections == FALSE)
       SelectClearSelections (svnw, 1, svnw->svn.num_entries);


/*
**  Tell the caller that we are done with transitioning selections
*/
    SelectReportTransitions (svnw);

}

#ifdef _NO_PROTO
static void svn_button1_motion(w, event)

  Widget w;
  XMotionEvent *event;
#else
static void svn_button1_motion(Widget w, XMotionEvent *event)
#endif

{
    /*
    **  Local data declarations
    */
    svn_widget svnw;
    DXmSvnEntryPtr svnentry;
    int entry_number, component_number;
    XtPointer entry_tag;
    int interval, status;
    unsigned char old_leave_direction;
    Widget event_w;
    

    svnw = StructFindSvnWidget (w);

    old_leave_direction = svnw->svn.leave_direction;
    
    /******************************************************************************/
    /*                                                                            */
    /* Check the current button state...					  */
    /*	If the button state is IDLE, we don't care about misc. motion events...   */
    /*  If the button state was awaiting a double click, don't process this motion*/
    /*   event.									  */
    /*                                                                            */
    /******************************************************************************/
    if (!(svnw->svn.last_event.type & BUTTONDOWN)) return;


    /******************************************************************************/
    /*                                                                            */
    /* If any other button has been depressed with this motion event, then that   */
    /* would be interpreted as a chorded cancel... kill any timers set, select    */
    /* the location cursor entry and unselect any other entry.			  */
    /*                                                                            */
    /******************************************************************************/
    if ((event->state & Button2Mask) || (event->state & Button3Mask))
	{
	svn_cancel(svnw, event);
	return;
	}

    /*
    **  If multiple selections are not allowed, there is already a selection made,
    **  then just return, because we don't want to select any more entries.
    */
    if ((svnw->svn.multiple_selections == FALSE) && (svnw->svn.num_selections > 0)) 
	return;



    /* Obtain the entry number */
    
    DXmSvnMapPosition ((Widget) svnw, event->x, event->y, &entry_number, &component_number, &entry_tag);

    /* If there are no entries in SVN, then don't process this motion event */
    
    if (svnw->svn.num_entries == 0)
	return;

    if ((entry_number == 0) &&
	(svnw->svn.leave_direction == 0) &&
	(svnw->svn.display_mode == DXmSvnKdisplayTree)) return;
	
    
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	{
	if (event->x >= svnw->svn.primary_window_widget->core.width)
	    event_w = svnw->svn.secondary_window_widget;
	else
	    event_w = svnw->svn.primary_window_widget;
	}
    else
	event_w = svnw->svn.primary_window_widget;
	

    /******************************************************************************/
    /*                                                                            */
    /* If we are out of the window (signaled by entry_number returned as 0),	  */
    /* and the direction is different then the last leave direction, fake a leave */
    /* window event.  This will allow the application to move the cursor outside  */
    /* the window and SVN will track correctly.				          */
    /*                                                                            */
    /******************************************************************************/
    if (entry_number == 0)
	{
	if (((event->y >= (int)event_w->core.height) &&
	     (svnw->svn.leave_direction & TOPLEAVE)) ||
	    ((event->y <= (int)event_w->core.y) &&
	     (svnw->svn.leave_direction & BOTTOMLEAVE)) || 
	    ((event->x <= (int)svnw->core.x) &&
	     (svnw->svn.leave_direction & LEFTLEAVE)) ||
	    ((event->x >= (int)svnw->core.width) &&
	     (svnw->svn.leave_direction & RIGHTLEAVE)))
	     {
	     if (old_leave_direction != svnw->svn.leave_direction)
		{
		if (svnw->svn.drag_id)
		    XtRemoveTimeOut(svnw->svn.drag_id);
		svnw->svn.drag_id = 0;
		svn_leave(svnw, event);
		return;
		}
	    }
	}

    else    /* The cursor is inside the window */
	{        	
	/* If a time was set remove it since the cursor is back in the window */

	if (svnw->svn.drag_id)
	    XtRemoveTimeOut(svnw->svn.drag_id);
	svnw->svn.drag_id = 0;
	}


		
    svnw->svn.leave_direction = 0;
    
    /******************************************************************************/
    /*                                                                            */
    /* Decide the direction of the pointer leave... and what will be the next     */
    /* entry to select, given the direction of the leave.		          */
    /*                                                                            */
    /******************************************************************************/
    if (event->y >= (int)event_w->core.height)	/* Bottom */
	svnw->svn.leave_direction = BOTTOMLEAVE;

    else
    
    if (event->y <= (int)event_w->core.y)		/* Top */
	svnw->svn.leave_direction = TOPLEAVE;

    else

    if (event->x <= (int)svnw->core.x)		/* Left */
	{
	svnw->svn.leave_direction = LEFTLEAVE;
	DXmSvnMapPosition ((Widget) svnw, 0, event->y, &entry_number, &component_number, &entry_tag);
	}
    else
    	
    if (event->x >= (int)svnw->core.width)	/* Right */
	{
	svnw->svn.leave_direction = RIGHTLEAVE;
	DXmSvnMapPosition ((Widget) svnw, 0, event->y, &entry_number, &component_number, &entry_tag);
	}
	
    
    /******************************************************************************/
    /*                                                                            */
    /* A special case in the above checking is if... this motion event occurred   */
    /* between the left-hand margin of an entry, but was still in the window,     */
    /* we need an entry number anyway.					          */
    /*                                                                            */
    /******************************************************************************/
    if (entry_number == 0)
	DXmSvnMapPosition ((Widget) svnw,
			   0,
			   event->y,
			   &entry_number,
			   &component_number,
			   &entry_tag);
    
    /******************************************************************************/
    /*                                                                            */
    /* If after all that, we still don't have a entry number, then just set the   */
    /* entry number to be the last one in the display. This is to handle the case */
    /* where the motion event occured in the white space between the last entry   */
    /* and the edge of the window.						  */
    /*                                                                            */
    /******************************************************************************/
    if (entry_number == 0)
	entry_number = svnw->svn.entries[svnw->svn.display_count];


    /* Set up a new timeout ... */

    if (svnw->svn.leave_direction)
	{
	if (!svnw->svn.drag_id || (old_leave_direction != svnw->svn.leave_direction)	)
	    {
	    if (svnw->svn.drag_id)
		XtRemoveTimeOut(svnw->svn.drag_id);
		
	    interval = 100;
	    svnw->svn.drag_id = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)svnw),
					       (unsigned long)interval,
					       (XtTimerCallbackProc) LclProcessAutoScroll,
					        (XtPointer) svnw);
	    }						
	}

     
     /******************************************************************************/
     /*                                                                            */
     /* If the leave direction of this motion event was either TOP or BOTTOM,	   */
     /* then let the timer proc handle the selection function so that the scrolling*/
     /* is in sync with the extended selected...				   */
     /*                                                                            */
     /******************************************************************************/
     if ((entry_number == 0)  &&
	 (svnw->svn.display_mode == DXmSvnKdisplayTree)) return;
     
     if ((svnw->svn.leave_direction == TOPLEAVE) || (svnw->svn.leave_direction == BOTTOMLEAVE)) 
	    return;
	    
     /******************************************************************************/
     /*                                                                            */
     /* Only select entries that are off the screen in the timer proc... if the    */
     /* current entry is on the screen then we need to do the extended selection.  */
     /*                                                                            */
     /******************************************************************************/
     if (entry_number != svnw->svn.last_selected_entry)
	    {
	    svnentry = StructGetEntryPtr (svnw, entry_number);

	    
	    /*
	    **  If the entry is insensitive, then ignore it.
	    */
	    if (svnentry->grayed) return;

	
	    /******************************************************************************/
	    /*                                                                            */
	    /* This routine will select or unselect the needed entries from the select    */
	    /* set.									  */
	    /*                                                                            */
	    /******************************************************************************/
	    if (svnw->svn.drag_state == 0)
		LclUpdateSelectRange(svnw, svnentry, entry_number,svnw->svn.last_event.compnm, event->time);
	    else
		LclUpdateToggleRange(svnw, svnentry, entry_number,svnw->svn.last_event.compnm, event->time);
	    
	    svnw->svn.last_selected_entry = entry_number;
	    
	    }	/* End of this entry not already selected */
	    


} /* end of svn_button_motion */   

/******************************************************************************/
/*                                                                            */
/* This routine will manage the auto scrolling if the pointer has left the    */
/* window during a drag or an extended selection.			      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void LclProcessAutoScroll(svnw,id)

  svn_widget	svnw;
  XtIntervalId	id;
    
#else
static void LclProcessAutoScroll(svn_widget svnw, XtIntervalId id)
#endif

{
/*
**  Local data declarations
*/

    int		    interval;
    int		    entry_number;
    XmScrollBarCallbackStruct scroll;

    
    /******************************************************************************/
    /*                                                                            */
    /* If a button went up, then remove the timeout.			          */
    /*                                                                            */
    /******************************************************************************/
    if (!(svnw->svn.last_event.type & BUTTONDOWN))
    {
	XtRemoveTimeOut(svnw->svn.drag_id);
	svnw->svn.drag_id = 0;
	return;		/* Button up code will be called and it will perform the selection  */
    }

    /******************************************************************************/
    /*                                                                            */
    /* Check which direction the pointer went out of the window ... and then      */
    /* perform the appropriate scroll operation.				  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.leave_direction & TOPLEAVE)
    {
	if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	{
	    scroll.reason = XmCR_DECREMENT;
	    scroll.value = 1;
	    TopTreeVScroll(svnw, &scroll);
	}
	else
	{
	    /* if at top of entries, don't change selection */
	    if (!XtIsManaged(svnw->svn.primary_ptr_widget))
	    {
		if (svnw->svn.entries[1] == 1)
		    return;			    /* there is no where to scroll */
		else
		    entry_number = svnw->svn.entries[1] - 1;
	    }
	    else
		/* There is a path to root, this must not be the last entry */
		entry_number = svnw->svn.entries[svnw->svn.num_path+1] - 1;

	    LclExtendButtonSelection(svnw, 1, entry_number);
	}
    }
    else if (svnw->svn.leave_direction & BOTTOMLEAVE)
    {
	/* If at bottom of entries, don't increment */
	/* WARNING ...Don't know how to check for tree mode at bottom */

	if (svnw->svn.display_mode != DXmSvnKdisplayTree)
	    if (LclOutlineAtBottom(svnw)) return;

	if (svnw->svn.last_selected_entry+1 > svnw->svn.num_entries)
	    entry_number = svnw->svn.last_selected_entry;
	else
	    entry_number = svnw->svn.last_selected_entry + 1;

	LclExtendButtonSelection(svnw, 0, entry_number);
    }
    else if (svnw->svn.leave_direction & RIGHTLEAVE)
    {
	scroll.reason = XmCR_INCREMENT;
	scroll.value = 1;
	if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	    TopTreeHScroll(svnw, &scroll);
	else     
	    OutlineHScroll(svnw, 0, &scroll);
    }		
    else if (svnw->svn.leave_direction & LEFTLEAVE)
    {
	scroll.reason = XmCR_PAGE_DECREMENT;
	scroll.value = 1;
	if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	    TopTreeHScroll(svnw, &scroll);
	else
	    OutlineHScroll(svnw, 0, &scroll);
    }

    /* Sync up events for scrolling ... */

    XSync(XtDisplay(svnw), FALSE); 

    
    /******************************************************************************/
    /*                                                                            */
    /* Reset the timeout proc if more auto scrolling is needed.			  */
    /*                                                                            */
    /******************************************************************************/
    interval = 100;
    svnw->svn.drag_id = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)svnw),
					(unsigned long)interval,
					(XtTimerCallbackProc) LclProcessAutoScroll,
					(XtPointer) svnw);


}   /* End of routine LclProcessAutoScroll */

#ifdef _NO_PROTO
static void LclExtendButtonSelection(w, direction, entry)

Widget w;
int	    direction;
int	    entry;		
#else
static void LclExtendButtonSelection(Widget w, int direction, int entry)
#endif

{

    svn_widget svnw = StructFindSvnWidget(w);
    XmScrollBarCallbackStruct scroll;
    DXmSvnEntryPtr svnentry;
    int entry_number = entry;


    svnentry = StructGetEntryPtr (svnw, entry);

    /*
    **  If we are hiding the selections, then treat this as a non-shift arrow up
    */
    if (svnw->svn.show_selections == FALSE)
	{
	XButtonEvent	*event = (XButtonEvent *)&(svnw->svn.last_event);
	svn_button1_down(svnw, event); 
	return;
	};

    /*
    **  If multiple selections are not allowed, there is already a selection made,
    **  AND it is not this entry, then treat this as a normal arrow up.
    */
    if ((svnw->svn.multiple_selections == FALSE) &&
	(svnw->svn.num_selections == 1) &&
	    (!svnw->svn.location_cursor_entry->selected))      
	    {
	    XButtonEvent	*event = (XButtonEvent *)&(svnw->svn.last_event);
	    svn_button1_down(svnw, event); 
	    return;
	    };

    /*
    **  If the entry is insensitive, then ignore it.
    */
    if (svnentry->grayed) return;


    if (direction == 1)	    /* Process Up extend selection */	    
	{
	DXmSvnDisableDisplay((Widget) svnw);
	LclOutlinePositionJump(svnw, entry);
	DXmSvnEnableDisplay((Widget) svnw);
	}
    else 
	{
	scroll.reason = XmCR_INCREMENT; 
	OutlineVScroll(svnw, 0, &scroll); 
	}
	
    if (svnw->svn.drag_state == 0) 
        LclUpdateSelectRange (svnw, svnentry, entry, svnw->svn.last_event.compnm, svnw->svn.last_event.time);
    else
	LclUpdateToggleRange (svnw, svnentry, entry, svnw->svn.last_event.compnm, svnw->svn.last_event.time);
    
    svnw->svn.last_selected_entry = entry_number;
    
/*
**  Tell the caller that all transitions are completed.
*/
    SelectReportTransitions (svnw);

    return;

}   /* End of routine LclExtendButtonSelection */

/******************************************************************************/
/*                                                                            */
/* This routine will handle KCancel events (F11)			      */
/*	This key will either cancel a ongoing drag operation, or an extended  */
/*	selection.  If canceled during an extended selection, the selection   */
/*	will be only on the last entry the location cursor was on.	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_cancel(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_cancel(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);


    if ((!(svnw->svn.last_event.type & BUTTONDOWN)) &&
	(!(svnw->svn.last_event.type & BUTTON2DOWN))) return;
    
    if (svnw->svn.drag_id)
	{
	XtRemoveTimeOut(svnw->svn.drag_id);
	svnw->svn.drag_id = 0;
	}


    /******************************************************************************/
    /*                                                                            */
    /* Handle BTN1 down cancels... (extended selection)... unselect all entries   */
    /* except the location cursor entry.					  */
    /*                                                                            */
    /* Handle BTN2 down cancels... (cancel drag operation ) delete ghost object.  */
    /*										  */
    /******************************************************************************/
    if (svnw->svn.last_event.type & BUTTONDOWN)
	SelectSelectSet(svnw, svnw->svn.location_cursor, svnw->svn.location_cursor,
				svnw->svn.location_cursor_entry->selected_comp, event->time);

    if (svnw->svn.last_event.type & BUTTON2DOWN)
	{
	LclDrawGhostBox(svnw, svnw->svn.ghost_x, svnw->svn.ghost_y);
	DisplayDeleteGhost (svnw);
	svnw->svn.button_mode = MODEIDLE;
	}

    svnw->svn.last_event.type = 0;
}

#ifdef _NO_PROTO
static void LclTraverseNextTabGroup (w, event)
Widget w;
XEvent * event;
#else
static void LclTraverseNextTabGroup (Widget w, XEvent * event)
#endif
{
   XmProcessTraversal (w, XmTRAVERSE_NEXT_TAB_GROUP);
}


#ifdef _NO_PROTO
static void LclTraversePrevTabGroup (w, event)
Widget w;
XEvent * event;
#else
static void LclTraversePrevTabGroup (Widget w, XEvent * event)
#endif
{
   XmProcessTraversal (w, XmTRAVERSE_PREV_TAB_GROUP);
}



#ifdef _NO_PROTO
static void LclUpdateSelectRange(svnw, svnentry, entry_number, component_number, time)

svn_widget svnw;
DXmSvnEntryPtr svnentry;
int entry_number, component_number;
Time time;
#else
static void LclUpdateSelectRange(svn_widget svnw, DXmSvnEntryPtr svnentry, 
                  int entry_number, int component_number, Time time)
#endif
{
    /*
    **  Local data declarations
    */
    XtPointer entry_tag;
    int status, first, last, i;
    int select_them_all = FALSE;


	/******************************************************************************/
	/*                                                                            */
	/* The first thing to do, is to see if all the entries between the last       */
	/* selected entry and this current entry were already selected, if so, then   */
	/* unselect them, because the user might have retracked the pointer over      */
	/* these selections.... signaling a change of mind.			      */
	/*                                                                            */
	/******************************************************************************/

	/* Current entry IS already selected */
	
	if (svnentry->selected)
	    {
	    if (svnw->svn.last_selected_entry > entry_number)
		{
		first = entry_number + 1;
		last = svnw->svn.last_selected_entry;
		}
	    else
		{
		first = svnw->svn.last_selected_entry;
		last = entry_number - 1;
		}
	    }

	/* Current entry is NOT selected */    

	else
	    {
	    if (svnw->svn.last_selected_entry > entry_number)
		{
		first = entry_number;
		last = svnw->svn.last_selected_entry - 1;
		}
	    else
		{
		first = svnw->svn.last_selected_entry + 1;
		last = entry_number;
		}
	    }
	    

	/******************************************************************************/
	/*                                                                            */
	/* If we should be trying to unselect..  then check all entries between       */
	/* the first and the last...						      */
	/*                                                                            */
	/******************************************************************************/
	for (i = first;  i <= last;  i++)
	    {
	    svnentry = StructGetEntryPtr (svnw, i);
	    if (!LclEntrySelected (svnw, svnentry, i, component_number))
		{
		select_them_all = TRUE;
		break;
		}
	    }

	/* If we found entries that were selected,then we will do unselection of all */
	/* instead of selection ...						     */
	    
	if (!select_them_all)
	    {
	    for (i = first;  i <= last;  i++)
		{
		svnentry = StructGetEntryPtr (svnw, i);
		LclUnselectEntry (svnw, svnentry, i);
		}
	    }
	else
	    {		    
	    /******************************************************************************/
	    /*                                                                            */
	    /* Instead of just selecting this entry, we need to make sure that we	  */
	    /* check to see if every entry between this entry and the last		  */
	    /* selected entry is selected. (it is possible to loss motion events and      */
	    /* therefore not have selected entries inbetween)				  */
	    /*                                                                            */
	    /******************************************************************************/
	    LclSelectExtendedRange( svnw,
				    svnw->svn.last_event.entry_number,
				    entry_number,
				    component_number,
				    time);
	    }


}

    
#ifdef _NO_PROTO
static void LclUpdateToggleRange(svnw, svnentry, entry_number, component_number, time )

svn_widget svnw;
DXmSvnEntryPtr svnentry;
int entry_number, component_number;
Time time;
#else
static void LclUpdateToggleRange(svn_widget svnw, DXmSvnEntryPtr svnentry,
                      int entry_number, int component_number, Time time)
#endif

{
    /*
    **  Local data declarations
    */
    XtPointer entry_tag;
    int status, first, last, i;

	
	/* Current entry is lower in entry number than last selected entry */
	
	if (svnw->svn.last_selected_entry > entry_number)
	    {
	    last = svnw->svn.last_selected_entry - 1;
	    first = entry_number;
	    }
	else
	    {
	    last = entry_number;
	    first = svnw->svn.last_selected_entry + 1;		
	    }


	for (i = first;  i <= last;  i++)
	    {
	    svnentry = StructGetEntryPtr (svnw, i);
	    if (LclEntrySelected (svnw, svnentry, i, component_number))
		LclUnselectEntry(svnw, svnentry, i);
	    else
		LclSelectEntry (svnw, svnentry, i, component_number, time, first_of_many);
	    }

}   /* End of routine LclUpdateToggleSelections */

#ifdef _NO_PROTO
static void LclActivateEntryProc(svnw, callback)

svn_widget  svnw;
DXmSvnCallbackStruct	*callback;
#else
static void LclActivateEntryProc(svn_widget svnw, DXmSvnCallbackStruct *callback)
#endif

    {
    
    if (svnw->svn.num_selections == 1) callback->reason = DXmSvnCRSelectAndConfirm;
         else callback->reason = DXmSvnCRExtendConfirm;

    DXmSvnDisableDisplay ((Widget) svnw);
    DisplaySetWatch (svnw, TRUE);

    callback->entry_level = svnw->svn.map_level;

    if (svnw->svn.num_selections == 1)
	XtCallCallbacks ((Widget)svnw, DXmSvnNselectAndConfirmCallback, callback);
    else if (svnw->svn.ExtendConfirm_callback != (XtCallbackList) NULL) 
             XtCallCallbacks ((Widget)svnw, DXmSvnNextendConfirmCallback, callback);
		else {
                    /* 
                    **  We have multiple selections but no ExtendConfirm callback...  
                    **  Go backwards through the list calling the single select
		    **  and confirm for each one.
                    */
                        int num_selected = svnw->svn.num_selections;
                        int * numbers;
			XtPointer * tags;
                        numbers = (int *) XtMalloc (num_selected * sizeof(int));
                        tags    = (XtPointer *) XtMalloc (num_selected * sizeof(XtPointer));
		        callback->reason = DXmSvnCRSelectAndConfirm;
			DXmSvnGetSelections ((Widget) svnw, numbers, NULL, tags, num_selected);
			for (; num_selected > 0;  num_selected--)
                            {
                              callback->entry_number = numbers [num_selected - 1];
                              callback->entry_tag    = tags    [num_selected - 1];
                              XtCallCallbacks ((Widget)svnw, DXmSvnNselectAndConfirmCallback, callback);
                            };
		        XtFree ((char *)numbers);
		        XtFree ((char *)tags);
                   };


	 DisplaySetWatch (svnw, FALSE);
         DXmSvnEnableDisplay ((Widget) svnw);

}   /* end of routine LclActivateEntry */

#ifdef _NO_PROTO
static void LclSelectEntryProc(svnw, time)

svn_widget  svnw;
Time time;
#else
static void LclSelectEntryProc(svn_widget svnw, Time time)
#endif

{

    DXmSvnEntryPtr svnentry;
    int entry_number = svnw->svn.location_cursor;

/*
**  Get the entry pointer for this entry.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If the entry is insensitive, then ignore it.
*/
    if (svnentry->grayed) return;


/*
**  If we are not showing selections, then unselect all of the entries
*/
    if (svnw->svn.show_selections == FALSE)
       SelectClearSelections (svnw, 1, svnw->svn.num_entries);



/******************************************************************************/
/*                                                                            */
/* Select this entry and deselect all the rest...			      */
/*                                                                            */
/******************************************************************************/
    SelectSelectSet (svnw, entry_number, entry_number,
			svnw->svn.location_cursor_entry->selected_comp, time);

/*
**  Tell the caller that we are done with transitioning selections
*/
    SelectReportTransitions (svnw);

    
}

#ifdef _NO_PROTO
static void svn_button2_down (w, event)

  Widget w;
  XButtonEvent *event;
#else
static void svn_button2_down (Widget w, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;
    int has_list;

    svnw = StructFindSvnWidget (w);

    
    /******************************************************************************/
    /*                                                                            */
    /* If any other button has been depressed with this BTN2 down event, then that*/
    /* would be interpreted as a chorded cancel... kill any timers set, select    */
    /* the location cursor entry and unselect any other entry.			  */
    /*                                                                            */
    /******************************************************************************/
    if ((event->state & Button1Mask) || (event->state & Button3Mask))
	{
	svn_cancel((Widget) svnw, (XKeyEvent *)event);
	return;
	}

    /******************************************************************************/
    /*                                                                            */
    /* First thing to do is to save state of what this event is...		  */
    /*                                                                            */
    /******************************************************************************/
    svnw->svn.last_event.type = BUTTON2DOWN;
    svnw->svn.last_event.state = event->state;
    svnw->svn.last_event.x = event->x;
    svnw->svn.last_event.y = event->y;
    svnw->svn.last_event.event = (XEvent *)event;

    
    DXmSvnMapPosition ((Widget) svnw, event->x, event->y, &temp.entry_number, &temp.component_number, (XtPointer *)&temp.entry_tag);
    svnw->svn.last_event.entry_number = temp.entry_number;
    temp.event = (XEvent *)event;
    
    if (temp.entry_number == 0)
	{
	svnw->svn.last_event.type = 0;
	return;
	}
    
    /******************************************************************************/
    /*                                                                            */
    /* Record the current location of the pointer to be the drag holding point    */
    /*                                                                            */
    /******************************************************************************/
    svnw->svn.ghost_x = event->x;
    svnw->svn.ghost_y = event->y;

    /*
    **  Determine if the application is under control of this drag.  We wouldn't
    **  be here if dragging wasn't supported at all.
    */
    if ((svnw->svn.button_mode == MODEAPPLDRAG) &&
	(svnw->svn.Dragging_callback != (XtCallbackList) NULL))
       {
	    
          temp.reason           = DXmSvnCRDragging;
          temp.x                = ( (LayoutIsRtoL(svnw)) ?
                        	     XtWidth(svnw) - event->x : event->x);
          temp.y                = event->y;

          DXmSvnDisableDisplay ((Widget) svnw);
          XtCallCallbacks ((Widget)svnw, DXmSvnNdraggingCallback, &temp);
          DXmSvnEnableDisplay ((Widget) svnw);
          return;
       };

    /******************************************************************************/
    /*                                                                            */
    /* Set up button mode for entry dragging.. if the application has		  */
    /* enabled the event. If the SelectionsDragged callback is == NULL then       */
    /* we will not process this as a MB2 drag.					  */
    /*                                                                            */
    /******************************************************************************/
    has_list = XtHasCallbacks((Widget)svnw, DXmSvnNselectionsDraggedCallback);
    if ((has_list == XtCallbackNoList) || (has_list == XtCallbackHasNone))
	{
	svnw->svn.last_event.type = 0;	/* clear out the state of this event */
	return;
	}
    else 
	svnw->svn.button_mode = MODEAWAITDRAG;

    /******************************************************************************/
    /*                                                                            */
    /* If there is one selection, just drag a ghost of that entry...		  */
    /* If there is multiple selections, drag a ghost of ALL selections...	  */
    /* If there is multiple discontiguous selections, drag a ghost of each	  */
    /*   selection group all together.						  */
    /*									          */
    /******************************************************************************/
    svnentry = StructGetEntryPtr(svnw, temp.entry_number);

    /*
    **  Create and display a ghosting pixmap
    */
    DisplayCreateGhost (svnw, (svnentry->selected ? 0 : svnw->svn.last_event.entry_number) );
    LclDrawGhostBox (svnw, event->x, event->y);


}

#ifdef _NO_PROTO
static void svn_button2_motion(w, event)

  Widget w;
  XMotionEvent *event;
#else
static void svn_button2_motion(Widget w, XMotionEvent *event)
#endif

{
    /*
    **  Local data declarations
    */
    svn_widget svnw;
    int interval;
    unsigned char old_leave_direction;
    Widget event_w;
    

    svnw = StructFindSvnWidget (w);

    old_leave_direction = svnw->svn.leave_direction;
    svnw->svn.last_event.x = event->x;
    svnw->svn.last_event.y = event->y;
    svnw->svn.last_event.event = (XEvent *)event;
    
    /******************************************************************************/
    /*                                                                            */
    /* Check the current button state...					  */
    /*	If the button state is IDLE, we don't care about misc. motion events...   */
    /*                                                                            */
    /******************************************************************************/
    if (!(svnw->svn.last_event.type & BUTTON2DOWN)) return;


    /******************************************************************************/
    /*                                                                            */
    /* If any other button has been depressed with this motion event, then that   */
    /* would be interpreted as a chorded cancel... kill any timers set, select    */
    /* the location cursor entry and unselect any other entry.			  */
    /*                                                                            */
    /******************************************************************************/
    if ((event->state & Button1Mask) || (event->state & Button3Mask))
	{
	svn_cancel((Widget) svnw, (XKeyEvent *)event);
	return;
	}

    event_w = svnw->svn.primary_window_widget;
    
    /******************************************************************************/
    /*                                                                            */
    /* If we are out of the window						  */
    /* and the direction is different then the last leave direction, fake a leave */
    /* window event.  This will allow the application to move the cursor outside  */
    /* the window and SVN will track correctly.				          */
    /*                                                                            */
    /******************************************************************************/
	if (((event->y >= (int)event_w->core.height) &&
	     (svnw->svn.leave_direction & TOPLEAVE)) ||
	    ((event->y <= (int)event_w->core.y) &&
	     (svnw->svn.leave_direction & BOTTOMLEAVE)) || 
	    ((event->x <= (int)svnw->core.x) &&
	     (svnw->svn.leave_direction & LEFTLEAVE)) ||
	    ((event->x >= (int)svnw->core.width) &&
	     (svnw->svn.leave_direction & RIGHTLEAVE)))
	     {
	     if (old_leave_direction != svnw->svn.leave_direction)
		{
		if (svnw->svn.drag_id)
		    XtRemoveTimeOut(svnw->svn.drag_id);
		svnw->svn.drag_id = 0;
		svn_leave(svnw, event);
		return;
		}
	    }
	else	/* motion event occurred inside the window */
	    {
	    /* If a time was set remove it since the cursor is back in the window */

	    if (svnw->svn.drag_id)
		XtRemoveTimeOut(svnw->svn.drag_id);
	    svnw->svn.drag_id = 0;
	    }
		
    svnw->svn.leave_direction = 0;
    
    /******************************************************************************/
    /*                                                                            */
    /* Decide the direction of the pointer leave...this will let the timer proc   */
    /* decide which direction to scroll the display ...				  */
    /*                                                                            */
    /******************************************************************************/
    if (event->y >= (int)event_w->core.height)	/* Bottom */
	svnw->svn.leave_direction = BOTTOMLEAVE;
    else
    
    if (event->y <= (int)event_w->core.y)		/* Top */
	svnw->svn.leave_direction = TOPLEAVE;
    else

    if (event->x <= (int)svnw->core.x)		/* Left */
	svnw->svn.leave_direction = LEFTLEAVE;
    else
    	
    if (event->x >= (int)svnw->core.width)	/* Right */
	svnw->svn.leave_direction = RIGHTLEAVE;
	

    /* Set up a new timeout ... if we are out of the window */

    if (svnw->svn.leave_direction)
	{
	if (!svnw->svn.drag_id || (old_leave_direction != svnw->svn.leave_direction)	)
	    {
	    if (svnw->svn.drag_id)
		XtRemoveTimeOut(svnw->svn.drag_id);
		
	    interval = 100;
	    svnw->svn.drag_id = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)svnw),
					       (unsigned long)interval,
					       (XtTimerCallbackProc) LclManageDragging,
					        (XtPointer) svnw);
	    }						
	}

    
    /******************************************************************************/
    /*                                                                            */
    /* Draw the new ghost box at the location of the motion event... The timer    */
    /* proc will handle scrolling the display.					  */
    /*										  */
    /******************************************************************************/
    LclDrawGhostBox(svnw, svnw->svn.ghost_x, svnw->svn.ghost_y);
    LclDrawGhostBox(svnw, event->x, event->y);


} /* end of svn_button_motion */   

#ifdef _NO_PROTO
static void svn_button2_up (w, event)

  Widget w;
  XButtonEvent *event;
#else
static void svn_button2_up (Widget w, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;

    svnw = StructFindSvnWidget (w);

    /******************************************************************************/
    /*                                                                            */
    /* Check to see if this button up is to be ignored... meaning that the	  */
    /* previous extended select was canceled by a KCancel (F11) or a chorded cancel */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.last_event.type == 0) return;

    /******************************************************************************/
    /*                                                                            */
    /* If there is a drag timeout set, remove it so we stop scrolling... this	  */
    /* means the user wants the auto scrolling to end.				  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.drag_id)
	{
	XtRemoveTimeOut(svnw->svn.drag_id);
	svnw->svn.drag_id = 0;
	}

    DXmSvnMapPosition ((Widget)svnw, event->x, event->y, &temp.entry_number, &temp.component_number, (XtPointer *)&temp.entry_tag);
    temp.event = (XEvent *)event;
    svnw->svn.last_event.type = 0;	/* Signals that dragging is completed */
    svnw->svn.last_event.event = (XEvent *)event;


    /******************************************************************************/
    /*                                                                            */
    /* If we are in application dragging mode, then notify them that we have	  */
    /* completed this dragging in this window.					  */
    /*										  */
    /******************************************************************************/
    if ((svnw->svn.button_mode == MODEAPPLDRAG) &&
	(svnw->svn.DraggingEnd_callback != (XtCallbackList) NULL))
       {
         temp.reason = DXmSvnCRDraggingEnd;
         temp.time   = event->time;
         temp.x      = event->x;
         temp.y      = event->y;
	 temp.loc_cursor_entry_number = svnw->svn.location_cursor;

         DXmSvnDisableDisplay ((Widget) svnw);
         XtCallCallbacks ((Widget)svnw, DXmSvnNdraggingEndCallback, &temp);
         DXmSvnEnableDisplay ((Widget) svnw);
	 svnw->svn.button_mode = MODEIDLE;
         return;
       };


    /* Get entry ptr of current entry */
    
    svnentry = StructGetEntryPtr(svnw, temp.entry_number);

    LclDrawGhostBox (svnw, svnw->svn.ghost_x, svnw->svn.ghost_y);


    /******************************************************************************/
    /*                                                                            */
    /* Report EntryTransfer callback to application, if this entry is the same    */
    /* entry number that the BTN2 down occurred on, and there is at LEAST one     */
    /* primary selection.  Let application know where this BTN2 was modifier      */
    /* by a CTRL (transfer should be a COPY) or a ALT (transfer should be MOVE).  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.EntryTransfer_callback != (XtCallbackList) NULL) &&
	(temp.entry_number == svnw->svn.last_event.entry_number))
	{
	
        temp.reason = DXmSvnCREntryTransfer;
        temp.time   = event->time;
        temp.x      = event->x;
        temp.y      = event->y;
	temp.loc_cursor_entry_number = svnw->svn.location_cursor;
        temp.entry_level  = svnentry->level;

	/* Return the state of the transfer to the application */
	
	if (svnw->svn.last_event.state  == 0)	/* Just Btn2 , no modifiers */
	    temp.transfer_mode  = DXmSvnKtransferUnknown;
	else
		/* Make sure we allow for LOCK and other modifiers */

		if (svnw->svn.last_event.state & ControlMask)   /* Ctrl modifier, has to be a COPY */
		    temp.transfer_mode = DXmSvnKtransferCopy;
		else if (svnw->svn.last_event.state & Mod1Mask)
		    temp.transfer_mode = DXmSvnKtransferMove;	/* Alt modifier, has to be a MOVE */
		
        DXmSvnDisableDisplay ((Widget) svnw);
        XtCallCallbacks ((Widget)svnw, DXmSvnNentryTransferCallback, &temp);
        DXmSvnEnableDisplay ((Widget) svnw);
	}

    else    /* This BTN2 up completes a selections dragged */    
	{ 
	DXmSvnMapPosition ((Widget) svnw, svnw->svn.ghost_x, svnw->svn.ghost_y, &temp.entry_number, 
                                     &temp.component_number, (XtPointer *)&temp.entry_tag);

	temp.dragged_entry_number = svnw->svn.last_event.entry_number;
	temp.reason = DXmSvnCRSelectionsDragged;
	temp.x      = ( (LayoutIsRtoL(svnw)) ?
			  XtWidth(svnw) - svnw->svn.ghost_x : svnw->svn.ghost_x);
	temp.y      = svnw->svn.ghost_y;
	temp.entry_level = svnw->svn.map_level;
	temp.loc_cursor_entry_number = svnw->svn.location_cursor;

	
	/******************************************************************************/
	/*                                                                            */
	/* We need to report to the application if a modifier was pressed with drag.  */
	/*                                                                            */
	/******************************************************************************/
	if (svnw->svn.last_event.state == 0)	/* Just Btn2 , no modifiers */
		temp.transfer_mode = DXmSvnKtransferUnknown;
	else
		/* Make sure we allow for LOCK and other modifiers */

		if (svnw->svn.last_event.state & ControlMask)   /* Ctrl modifier, has to be a COPY */
		    temp.transfer_mode = DXmSvnKtransferCopy;
		else if (svnw->svn.last_event.state & Mod1Mask)
		    temp.transfer_mode = DXmSvnKtransferMove;	/* Alt modifier, has to be a MOVE */

	DXmSvnDisableDisplay ((Widget) svnw);
	XtCallCallbacks ((Widget)svnw, DXmSvnNselectionsDraggedCallback, &temp);
	DXmSvnEnableDisplay ((Widget) svnw);
	}

    DisplayDeleteGhost(svnw);
    svnw->svn.button_mode = MODEIDLE;


}

/*
**  This routine is called on the first motion event while the button is being
**  held down.  This could be in response to a drag operation or to a range
**  selection.  This routine handles the former.
*/

#ifdef _NO_PROTO
static void LclManageDragging (svnw,id)

  svn_widget svnw;
  XtIntervalId id;
#else
static void LclManageDragging (svn_widget svnw, XtIntervalId id)
#endif

{
/*
**  Local data declarations
*/
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;
    int entry_number = 0, interval;
    XmScrollBarCallbackStruct scroll;



    /******************************************************************************/
    /*                                                                            */
    /* If a button went up, then remove the timeout.			          */
    /*                                                                            */
    /******************************************************************************/
    if (!(svnw->svn.last_event.type & BUTTON2DOWN))
    {
	XtRemoveTimeOut(svnw->svn.drag_id);
	svnw->svn.drag_id = 0;
	return;		/* Button2 up code will be called and it will cancel the drag */
    }
    
    
    /******************************************************************************/
    /*                                                                            */
    /* Now make sure we autoscroll the display according to the leave direction   */
    /* Then redraw the ghost object in the same location as it was before the	  */
    /* autoscroll.								  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.leave_direction & TOPLEAVE)
    {
	/* if at top of entries, don't change selection */
	if ((svnw->svn.display_mode == DXmSvnKdisplayOutline) || (svnw->svn.display_mode == DXmSvnKdisplayColumns))
	{
	    if (svnw->svn.entries[1] == 1)
	    {
		if (!svnw->svn.show_path_to_root)
		    return;				    /* at the top, no where to scroll */
		else if (svnw->svn.num_path == 0)
		    return;				    /* at the top, no where to scroll */
	    }

	    /* There is a path to root, the this must not be the last entry */
	    entry_number = svnw->svn.entries[svnw->svn.num_path+1] - 1;
	}
	    
	LclDrawGhostBox (svnw, svnw->svn.ghost_x, svnw->svn.ghost_y);

	if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	{
	    scroll.reason = XmCR_DECREMENT;
	    scroll.value = 1;
	    TopTreeVScroll(svnw, &scroll);
	}
	else 
	{
	    DXmSvnDisableDisplay((Widget) svnw);
	    LclOutlinePositionJump(svnw, entry_number);
	    DXmSvnEnableDisplay((Widget) svnw);
	}
    }
    else if (svnw->svn.leave_direction & BOTTOMLEAVE)
    {
	/* If at bottom of entries, don't increment */
	/* WARNING ...Don't know how to check for tree mode at bottom */

	    if (LclOutlineAtBottom(svnw)) return;

	LclDrawGhostBox (svnw, svnw->svn.ghost_x, svnw->svn.ghost_y);

	scroll.reason = XmCR_INCREMENT; 
	OutlineVScroll(svnw, 0, &scroll); 
    }
    else if (svnw->svn.leave_direction & RIGHTLEAVE)
    {
	LclDrawGhostBox (svnw, svnw->svn.ghost_x, svnw->svn.ghost_y);

	scroll.reason = XmCR_INCREMENT;
	scroll.value = 1;
	if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	    TopTreeHScroll(svnw, &scroll);
	else     
	    OutlineHScroll(svnw, 0, &scroll);
    }
    else if (svnw->svn.leave_direction & LEFTLEAVE)
    {
	LclDrawGhostBox (svnw, svnw->svn.ghost_x, svnw->svn.ghost_y);
    
	scroll.reason = XmCR_PAGE_DECREMENT;
	scroll.value = 1;
	if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	    TopTreeHScroll(svnw, &scroll);
	else
	    OutlineHScroll(svnw, 0, &scroll);
    }

    /* Sync up events for scrolling ... */

    XSync(XtDisplay(svnw), FALSE); 

    
    /******************************************************************************/
    /*                                                                            */
    /* Reset the timeout proc if more auto scrolling is needed.			  */
    /*                                                                            */
    /******************************************************************************/
    interval = 100;
    svnw->svn.drag_id = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)svnw),
					(unsigned long)interval,
					(XtTimerCallbackProc) LclManageDragging,
					(XtPointer) svnw);


    
    /******************************************************************************/
    /*                                                                            */
    /* Redraw the box after performing the scroll...				  */
    /*                                                                            */
    /******************************************************************************/
    LclDrawGhostBox (svnw, svnw->svn.last_event.x, svnw->svn.last_event.y);

}

#ifdef _NO_PROTO
static void svn_button3_down (w, event)

  Widget w;
  XButtonEvent *event;
#else
static void svn_button3_down (Widget w, XButtonEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;

    svnw = StructFindSvnWidget (w);

    if ((svnw->svn.last_event.type & BUTTONDOWN) || (svnw->svn.last_event.type & BUTTON2DOWN))
	{
	svn_cancel ((Widget) svnw, (XKeyEvent *)event);
	return;
	}


    DXmSvnMapPosition ((Widget) svnw, event->x, event->y, &temp.entry_number, &temp.component_number, (XtPointer *)&temp.entry_tag);
    temp.event = (XEvent *)event;

    if (svnw->svn.PopupMenu_callback != (XtCallbackList) NULL)
	{
	temp.reason = DXmSvnCRPopupMenu;
        temp.time   = event->time;
        temp.x      = event->x;
        temp.y      = event->y;
	temp.loc_cursor_entry_number = svnw->svn.location_cursor;
        DXmSvnDisableDisplay ((Widget) svnw);
        XtCallCallbacks ((Widget)svnw, DXmSvnNpopupMenuCallback, &temp);
        DXmSvnEnableDisplay ((Widget) svnw);
	}
	
}

/******************************************************************************/
/*                                                                            */
/* This routine will handle all arrow down events.			      */
/* Move the current location cursor to the next entry... if it is the last    */
/* entry on the display, scroll to it and keep the location cursor on the     */
/* display.								      */
/*									      */
/*	Arrow Down - move location cursor to next entry.		      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_arrow_down (w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_arrow_down (Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

    
    LclKeyScrollDown(svnw, event, SCROLL_ENTRY);

}
    
/******************************************************************************/
/*                                                                            */
/* This routine will handle all arrow up events:			      */
/*    Arrow Up - move location cursor to previous entry ...		      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_arrow_up (w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_arrow_up (Widget w, XKeyEvent *event)
#endif
{
    svn_widget svnw = StructFindSvnWidget (w);

    LclKeyScrollUp(svnw, event, SCROLL_ENTRY);
}


/******************************************************************************/
/*                                                                            */
/* This routine will handle all Next Screen events:			      */
/*    Arrow Up - move location cursor to first entry on next Page...          */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_page_down (w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_page_down (Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

    LclKeyScrollDown(svnw, event, SCROLL_PAGE);
    
}


/******************************************************************************/
/*                                                                            */
/* This routine will handle all Prev Screen events:			      */
/*     - move location cursor to first entry on previous page...	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_page_up (w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_page_up (Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

    LclKeyScrollUp(svnw, event, SCROLL_PAGE);
}

/******************************************************************************/
/*                                                                            */
/*  This routine will handle KBeginData events: (Ctrl/Alt LEFT Arrow)	      */
/*	FUNCTION: Scroll to first entry know to SVN. Set location cursor at   */
/*	          that first entry.					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_scroll_to_top(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_scroll_to_top(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

    LclKeyScrollUp(svnw, event, SCROLL_TO_TOP);

}
    
/******************************************************************************/
/*                                                                            */
/*  This routine will handle KEndData events: (Ctrl/Alt RIGHT Arrow)	      */
/*	FUNCTION: Scroll to first entry know to SVN. Set location cursor at   */
/*	          that first entry.					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_scroll_to_bottom(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_scroll_to_bottom(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

    LclKeyScrollDown(svnw, event, SCROLL_TO_BOTTOM);
}


/******************************************************************************/
/*                                                                            */
/* This routine will do the actual Scrolling Up by keyboard traversal	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void LclKeyScrollUp (svnw, event, navig_flag)

svn_widget  svnw;
XKeyEvent  *event;
int	    navig_flag;	   
#else
static void LclKeyScrollUp (svn_widget svnw, XKeyEvent *event, int navig_flag)
#endif
			   

{

    XmScrollBarCallbackStruct scroll;
    DXmSvnEntryPtr previous_entry, current_entry;
    int previous_entry_number;
    int location_changed;


    if (svnw->svn.num_entries == 0 || svnw->svn.location_cursor == 1 || svnw->svn.display_mode == DXmSvnKdisplayTree)
	return;

    /* Get the entry that we are positioned on...  */
    /* Is this the first entry... if so there is no where to go from here */

    previous_entry = svnw->svn.location_cursor_entry;
    previous_entry_number = svnw->svn.location_cursor;
    
    
    /******************************************************************************/
    /*                                                                            */
    /* If this is a PAGE scroll, wait to setup the location cursor till after we  */
    /* do the scroll....							  */
    /*                                                                            */
    /******************************************************************************/
    if (navig_flag == SCROLL_ENTRY)
	    svnw->svn.location_cursor = svnw->svn.anchor_entry = svnw->svn.location_cursor-1;

    /******************************************************************************/
    /*                                                                            */
    /* Handle the case where we might be at the top of the display but there are  */
    /* entries to display. Scroll the display accordingly.  We are		  */
    /* going to use the existing scrollbar callback routines, and fake a reason   */
    /* and callback structure... Why reinvent the wheel?		          */
    /*                                                                            */
    /* Have to change this to handle tree mode too.				  */
    /*										  */
    /******************************************************************************/
    switch (navig_flag)
	{
	case SCROLL_PAGE :  /* extend selection by a page */
	    {
	    scroll.reason = XmCR_PAGE_DECREMENT;
	    OutlineVScroll(svnw, 0, &scroll);
	    svnw->svn.location_cursor = svnw->svn.anchor_entry = svnw->svn.entries[svnw->svn.num_path+1];
	    break;
	    }
	case SCROLL_ENTRY :
	    {
	    int k, found = FALSE;
	
	    for (k = 1; k <= svnw->svn.display_count; k++)
		if (svnw->svn.location_cursor == svnw->svn.entries[k])
		    {
		    found = TRUE;
		    break;
		    }
	    
	    /******************************************************************************/
	    /*                                                                            */
	    /* If we found the entry in the display array, then we don't need to jump     */
	    /* the display to this entry... But we found this entry in the PTR window...  */
	    /* then we need to PositionJump there because we want to be able to scroll to */
	    /* all other entries in the path-to-root.					  */
	    /*                                                                            */
	    /******************************************************************************/
	    if (!found || (k <= svnw->svn.num_path))
		{
		DXmSvnDisableDisplay((Widget) svnw);
		LclOutlinePositionJump(svnw, svnw->svn.location_cursor);
		DXmSvnEnableDisplay((Widget) svnw);
		}

	    break;
	    } 
   
	case SCROLL_TO_TOP :
	    {
    	    scroll.reason = XmCR_TO_TOP;
	    scroll.pixel = 0;
	    scroll.value = 1;
	    OutlineVScroll(svnw, 0, &scroll);
	    svnw->svn.location_cursor = svnw->svn.anchor_entry = 1;
	    break;
	    }

	};   /* End of switch statement for scrolling */
	
    /*
    **  Get the entry pointer for this entry.	
    */
    svnw->svn.location_cursor_entry = current_entry = StructGetEntryPtr(svnw, svnw->svn.location_cursor);

    /******************************************************************************/
    /*                                                                            */
    /* The function of the arrow keys in NORMAL Mode is to do a KSelect.. which   */
    /* means to function like a MB1 Click.  All other selected entries are        */
    /* deselected while the current entry is selected.				  */
    /*                                                                            */
    /******************************************************************************/
        svnw->svn.last_event.event = (XEvent *)event;
	SelectSelectSet (svnw, svnw->svn.location_cursor, svnw->svn.location_cursor,
			    previous_entry->selected_comp, (event == 0 ? CurrentTime : event->time));

	svnw->svn.last_event.entry_number = svnw->svn.last_selected_entry =
		    svnw->svn.location_cursor;
	svnw->svn.last_event.state = (event == 0 ? 0 : event->state);

	/*
	**  Tell the caller that we are done with transitioning selections
	*/
	SelectReportTransitions (svnw);
		
	DisplayHighlightEntry(svnw, previous_entry_number); /* current entry number */
	DisplayHighlightEntry(svnw, svnw->svn.location_cursor);   /* new entry number */
    
}

/******************************************************************************/
/*                                                                            */
/* This routine will do the actual Scrolling Down by keyboard traversal	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void LclKeyScrollDown(svnw, event, navig_flag)

svn_widget  svnw;
XKeyEvent  *event;
int	    navig_flag;	    /* Flag signalling how many entries to scroll by */
#else
static void LclKeyScrollDown(svn_widget svnw, XKeyEvent *event, int navig_flag)
#endif

{

    XmScrollBarCallbackStruct scroll;
    DXmSvnEntryPtr previous_entry, current_entry;
    int previous_entry_number;



    if (svnw->svn.display_mode == DXmSvnKdisplayTree) return;

    if (svnw->svn.num_entries == 0)  return;

    /* Get the entry that we are positioned on...  */
    /* Is this the first entry... if so there is no where to go from here */

    previous_entry = svnw->svn.location_cursor_entry;
    previous_entry_number = svnw->svn.location_cursor;
    

    /******************************************************************************/
    /*                                                                            */
    /* Is this the last or bottom entry ... If so, just return, no repositioning  */
    /* of the location cursor is needed. This check handles the case where there  */
    /* is more than one entry in the display but all are visible, so the          */
    /* LclOutlineAtBottom routine will not catch it (since it looks that the	  */
    /* scroll bar height.							  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.entries[svnw->svn.display_count] == svnw->svn.location_cursor) &&
	(LclOutlineAtBottom(svnw)) )
	    return;

	
    if (navig_flag == SCROLL_ENTRY)
	svnw->svn.location_cursor = svnw->svn.anchor_entry = svnw->svn.location_cursor+1;

    
    /******************************************************************************/
    /*                                                                            */
    /* If we are trying to scroll pass the last entry... but the last entry is not*/
    /* yet visible on the display, then just jump and scroll to the bottom of the */
    /* display.									  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.location_cursor > svnw->svn.num_entries) &&
	(svnw->svn.entries[svnw->svn.display_count] != svnw->svn.num_entries))
	navig_flag = SCROLL_TO_BOTTOM;

    if ((svnw->svn.location_cursor > svnw->svn.num_entries) &&
	(svnw->svn.entries[svnw->svn.display_count] == svnw->svn.num_entries))
	navig_flag = SCROLL_TO_BOTTOM;

    /******************************************************************************/
    /*                                                                            */
    /* Handle the case where we might be at the bottom of the display but there   */
    /* are more entries that are not visible, in that case we would need to	  */
    /*	scroll the display accordingly.  We are going to use the existing scrollbar */
    /* callback routines, and fake a reason and callback structure...		  */
    /*                                                                            */
    /******************************************************************************/
    switch (navig_flag)
	{
	case SCROLL_PAGE :  /* Scroll by a page */
	    {
	    scroll.reason = XmCR_PAGE_INCREMENT;
	    OutlineVScroll(svnw, 0, &scroll);
	    svnw->svn.location_cursor = svnw->svn.anchor_entry = svnw->svn.entries[svnw->svn.num_path+1];
	    break;
	    }
	case SCROLL_ENTRY : /* Scroll by an entry */
	    {
	    int k, found = FALSE;
	
	    for (k = 1; k <= svnw->svn.display_count; k++)
		if (svnw->svn.location_cursor == svnw->svn.entries[k])
		    {
		    found = TRUE;
		    break;
		    }
	    
	    /*	 
	    **  If we found the entry in the display array and its the last one
	    **  then just scroll up one entry... 
	    */	 
	    if (found)
		{
		if (svnw->svn.location_cursor == svnw->svn.entries[svnw->svn.display_count])
		    {
		    scroll.reason = XmCR_INCREMENT;
		    OutlineVScroll(svnw, 0, &scroll);
		    }
		}
	    /*	 
	    **  Otherwise, we didn't find the entry, position that entry to the top to
	    **  get it visible on the display...
	    */	 
	    else 
		{
		DXmSvnDisableDisplay((Widget) svnw);
		LclOutlinePositionJump(svnw, svnw->svn.location_cursor);
		DXmSvnEnableDisplay((Widget) svnw);
		}

	    break;
	    }   /* End of scroll display by an entry */

	case SCROLL_TO_BOTTOM :
	    {
	    scroll.reason = XmCR_TO_BOTTOM;
	    scroll.pixel = 0;
	    scroll.value = svnw->svn.num_entries;
	    OutlineVScroll(svnw, 0, &scroll);
	    svnw->svn.location_cursor = svnw->svn.anchor_entry = svnw->svn.entries[svnw->svn.display_count];
	    break;
	    }

	}   /* End of switch statement */
    /*
    **  Get the entry pointer for this entry.	
    */
    svnw->svn.location_cursor_entry = current_entry = StructGetEntryPtr(svnw, svnw->svn.location_cursor);


    /******************************************************************************/
    /*                                                                            */
    /* The function of the arrow keys in NORMAL Mode is to do a KSelect.. which   */
    /* means to function like a MB1 Click.  All other selected entries are        */
    /* deselected while the current entry is selected.				  */
    /*                                                                            */
    /******************************************************************************/
        svnw->svn.last_event.event = (XEvent *)event;
    	SelectSelectSet (svnw, svnw->svn.location_cursor, svnw->svn.location_cursor,
				previous_entry->selected_comp, (event == 0 ? CurrentTime : event->time) );

	svnw->svn.last_event.entry_number = svnw->svn.last_selected_entry =
		    svnw->svn.location_cursor;
	svnw->svn.last_event.state = (event == 0 ? 0 : event->state);
		
	/*
	**  Tell the caller that we are done with transitioning selections
	*/
	SelectReportTransitions (svnw);

		
	DisplayHighlightEntry(svnw, previous_entry_number); /* current entry number */
	DisplayHighlightEntry(svnw, svnw->svn.location_cursor);   /* new entry number */
    
}

/******************************************************************************/
/*                                                                            */
/* This routine will handle all arrow left events.			      */
/*	Function : Scroll the display one unit to the LEFT, keeping the	      */
/*	location cursor on the same entry.				      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_arrow_left(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_arrow_left(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);
    XmScrollBarCallbackStruct	scroll;


    if (svnw->svn.num_entries == 0)  return;

    scroll.reason = XmCR_DECREMENT;
    scroll.value = 1;
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	TopTreeHScroll(svnw, &scroll);
    else
	{
	if (w == svnw->svn.primary_window_widget)
	    OutlineHScroll(svnw, 0, &scroll);
	else
	    LclOutlinesecondaryHScroll(svnw, &scroll);
	}
	    
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor); /* current entry number */

}
    
/******************************************************************************/
/*                                                                            */
/* This routine will handle all arrow right events:			      */
/*	Function: Scroll the display one unit to the RIGHT , keeping the      */
/*	location cursor on the same entry.				      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_arrow_right (w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_arrow_right (Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);
    XmScrollBarCallbackStruct	scroll;


    if (svnw->svn.num_entries == 0)  return;

    scroll.reason = XmCR_INCREMENT;
    scroll.value = 1;
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	TopTreeHScroll(svnw, &scroll);
    else
	{
	if (w == svnw->svn.primary_window_widget)
	    OutlineHScroll(svnw, 0, &scroll);
	else
	    LclOutlinesecondaryHScroll(svnw, &scroll);
	}
	
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor); /* current entry number */
}


/******************************************************************************/
/*                                                                            */
/* This routine will handle all Ctrl Prev Screen events:		      */
/*	Function: Scroll the display one page unit to the LEFT, keeping      */
/*	the location cursor on the same entry.				      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_page_left (w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_page_left (Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);
    XmScrollBarCallbackStruct	scroll;


    if (svnw->svn.num_entries == 0)  return;

    scroll.reason = XmCR_PAGE_DECREMENT;
    scroll.value = svnw->svn.display_count;
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	TopTreeHScroll(svnw, &scroll);
    else
	{
	if (w == svnw->svn.primary_window_widget)
	    OutlineHScroll(svnw, 0, &scroll);
	else
	    LclOutlinesecondaryHScroll(svnw, &scroll);
	}
	
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor); /* current entry number */

}



/******************************************************************************/
/*                                                                            */
/* This routine will handle all Ctrl Next Screen events:		      */
/*	Function: Scroll the display one page unit to the RIGHT, keeping      */
/*	the location cursor on the same entry.				      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_page_right(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_page_right(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);
    XmScrollBarCallbackStruct	scroll;


    if (svnw->svn.num_entries == 0)  return;

    scroll.reason = XmCR_PAGE_INCREMENT;
    scroll.value = svnw->svn.display_count;
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	TopTreeHScroll(svnw, &scroll);
    else
	{
	if (w == svnw->svn.primary_window_widget)
	    OutlineHScroll(svnw, 0, &scroll);
	else
	    LclOutlinesecondaryHScroll(svnw, &scroll);
	}
    
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor); /* current entry number */
    
}

/******************************************************************************/
/*                                                                            */
/* This routine will handle all ALT LEFT arrow events.			      */
/*	Function : Scroll the display all the way to the extreme left of the  */
/*	the entries in the display.					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_scroll_to_left(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_scroll_to_left(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);
    XmScrollBarCallbackStruct	scroll;


    if (svnw->svn.num_entries == 0)  return;

    scroll.reason = XmCR_VALUE_CHANGED;
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	{
	scroll.value = svnw->svn.margin_width;
	TopTreeHScroll(svnw, &scroll);
	}
    else
	{
	scroll.value = svnw->svn.margin_width;
	if (w == svnw->svn.primary_window_widget)
	    OutlineHScroll(svnw, 0, &scroll);
	else
	    LclOutlinesecondaryHScroll(svnw, &scroll);
	}
	
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);   /* current entry number */

}

/******************************************************************************/
/*                                                                            */
/* This routine will handle all ALT RIGHT arrow events.			      */
/*	Function : Scroll the display all the way to the extreme right of the  */
/*	the entries in the display.					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_scroll_to_right(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_scroll_to_right(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);
    XmScrollBarCallbackStruct	scroll;


    if (svnw->svn.num_entries == 0)  return;

    scroll.reason = XmCR_VALUE_CHANGED;
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	{
	scroll.value = svnw->svn.tree_width;
	TopTreeHScroll(svnw, &scroll);
	}
    else
	{
	scroll.value = svnw->svn.max_width;
	if (w == svnw->svn.primary_window_widget)
	    OutlineHScroll(svnw, 0, &scroll);
	else
	    LclOutlinesecondaryHScroll(svnw, &scroll);
	}
	
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor); /* current entry number */

}

/******************************************************************************/
/*                                                                            */
/* Handle F4 (KMenu) events....						      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_f4_menu(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_f4_menu(Widget w, XKeyEvent *event)
#endif

{
/*
**  Local data declarations
*/
    svn_widget svnw;
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;

    svnw = StructFindSvnWidget (w);
    temp.event = (XEvent *)event;

    /******************************************************************************/
    /*                                                                            */
    /* If any other button has been depressed with this BTN2 down event, then that*/
    /* would be interpreted as a chorded cancel... kill any timers set, select    */
    /* the location cursor entry and unselect any other entry.			  */
    /*                                                                            */
    /******************************************************************************/
    if ((event->state & Button2Mask) || (event->state & Button3Mask))
	svn_cancel((Widget) svnw, event);

    if (svnw->svn.PopupMenu_callback != (XtCallbackList) NULL)
	{
	temp.reason = DXmSvnCRPopupMenu;
        temp.time   = event->time;
	temp.entry_number = svnw->svn.location_cursor;
	temp.loc_cursor_entry_number = svnw->svn.location_cursor;
	temp.entry_tag = svnw->svn.location_cursor_entry->entry_tag;
        DXmSvnDisableDisplay ((Widget) svnw);
        XtCallCallbacks ((Widget)svnw, DXmSvnNpopupMenuCallback, &temp);
        DXmSvnEnableDisplay ((Widget) svnw);
	}

}	

/******************************************************************************/
/*                                                                            */
/* This routine will handle KActivate events (return, or ctrl/return)	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_activate_entry(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_activate_entry(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;
    DXmSvnCallbackStruct temp;

    svnw = StructFindSvnWidget (w);

    /* If there is no entries in the display just return and ignore this RETURN */

    svnw->svn.last_event.event = (XEvent *)event;

    if (svnw->svn.location_cursor != 0 && svnw->svn.location_cursor_entry != NULL)
    {
	temp.event = (XEvent *)event;
	temp.time = event->time;
	temp.entry_number = svnw->svn.location_cursor;
	temp.loc_cursor_entry_number = svnw->svn.location_cursor;
	temp.entry_tag = svnw->svn.location_cursor_entry->entry_tag;
	temp.component_number = 1;

	/* First select the entry... then activate (confirm) it */

	LclSelectEntryProc(svnw, event->time);

	LclActivateEntryProc(svnw, &temp);
    }
}



/******************************************************************************/
/*                                                                            */
/* This routine will handle KSelect events (spacebar, Ctrl/SpaceBar)	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_select_entry(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_select_entry(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    if (svnw->svn.location_cursor_entry != NULL)
    {
	svnw->svn.last_event.entry_number = svnw->svn.last_selected_entry =
			svnw->svn.location_cursor;
	svnw->svn.last_event.state = event->state;
	svnw->svn.last_event.event = (XEvent *)event;

	LclSelectEntryProc(svnw, event->time);
    }
}    


/******************************************************************************/
/*                                                                            */
/* This routine will handle KSelectAll events (Ctrl Slash)		      */
/*	    FUNCTION: Select all entries known and set location cursor at     */
/*		      first entry.					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_select_all(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_select_all(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;
    int prev_loc_cursor;
    
    svnw = StructFindSvnWidget (w);

    if (svnw->svn.num_entries == 0) return;

	
    svnw->svn.last_event.entry_number = svnw->svn.last_selected_entry = 
		    svnw->svn.location_cursor;
    svnw->svn.last_event.state = event->state;
    svnw->svn.last_event.event = (XEvent *)event;

    LclSelectExtendedRange(svnw, 1, svnw->svn.num_entries, svnw->svn.location_cursor_entry->selected_comp, event->time);

    svnw->svn.anchor_entry = 1;
}



/******************************************************************************/
/*                                                                            */
/* This routine will handle KDeSelectAll events (Ctrl BackSlash)	      */
/*	    FUNCTION: DeSelect all entries known and set location cursor at   */
/*		      first entry.					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_deselect_all(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_deselect_all(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    svnw->svn.last_event.entry_number = svnw->svn.last_selected_entry = 
		    svnw->svn.location_cursor;
    svnw->svn.last_event.state = event->state;
    svnw->svn.last_event.event = (XEvent *)event;

    /* Clear all selections except the location cursor entry */
    
    SelectClearSelections(svnw, 1, svnw->svn.num_entries);

    /* Reselect the location cursor */
    
    LclSelectEntry(svnw, svnw->svn.location_cursor_entry, svnw->svn.location_cursor, 1, event->time, first_of_one);
    
    svnw->svn.anchor_entry = svnw->svn.location_cursor;

}

/******************************************************************************/
/*                                                                            */
/* This routine will handle MShift Up arrow events.  Extend selection to      */
/* include one entry up on display.					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_extend_up(w, event)

Widget w;
XKeyEvent *event;
#else
static void svn_extend_up(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    LclExtendKeySelectionUp(svnw, event, SCROLL_ENTRY);
    
}   /* End of routine svn_extend_up */ 


/******************************************************************************/
/*                                                                            */
/* This routine will handle MShift Down arrow events.  Extend selection to    */
/* include one entry down on display. 					      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_extend_down(w, event)

Widget w;
XKeyEvent *event;
#else
static void svn_extend_down(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    LclExtendKeySelectionDown(svnw, event, SCROLL_ENTRY);
    

}   /* End of routine svn_extend_down */ 



/******************************************************************************/
/*                                                                            */
/* This routine will handle MShift Prev Screen events.  Extend selection to   */
/* include the prev page of entries on the display.			      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_extend_pageup(w, event)

Widget w;
XKeyEvent *event;
#else
static void svn_extend_pageup(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    LclExtendKeySelectionUp(svnw, event, SCROLL_PAGE);

}



/******************************************************************************/
/*                                                                            */
/* This routine will handle MShift Next Screen events.  Extend selection to   */
/* include the Next page of entries on the display.			      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_extend_pagedown(w, event)

Widget w;
XKeyEvent *event;
#else
static void svn_extend_pagedown(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    LclExtendKeySelectionDown(svnw, event, SCROLL_PAGE);
}


/******************************************************************************/
/*                                                                            */
/*  This routine will handle Extend KBeginData events:			      */
/*		(Shift/Ctrl/Alt LEFT Arrow)				      */
/*	FUNCTION: Extend selection from current location cursor to top entry  */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_extend_to_top(w, event)

Widget w;
XKeyEvent *event;
#else
static void svn_extend_to_top(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    LclExtendKeySelectionUp(svnw, event, SCROLL_TO_TOP);
    
}   /* End of routine svn_extend_to_top */ 


/******************************************************************************/
/*                                                                            */
/*  This routine will handle Extend KEndData events:			      */
/*		(Shift/Ctrl/Alt RIGHT Arrow)				      */
/*	FUNCTION: Extend selection from current location cursor to bottom entry  */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_extend_to_bottom(w, event)

Widget w;
XKeyEvent *event;
#else
static void svn_extend_to_bottom(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;

    svnw = StructFindSvnWidget (w);

    LclExtendKeySelectionDown(svnw, event, SCROLL_TO_BOTTOM);
    

}   /* End of routine svn_extend_to_bottom */ 



#ifdef _NO_PROTO
static void LclExtendKeySelectionUp(svnw, event, navig_flag)

svn_widget  svnw;
XKeyEvent  *event;
int	    navig_flag;	    /* Extend by entry or page */
			    /* Entry == 0, Page == 1 */
#else
static void LclExtendKeySelectionUp(svn_widget svnw, XKeyEvent *event, int navig_flag)
#endif

{

    XmScrollBarCallbackStruct scroll;
    DXmSvnEntryPtr svnentry, previous_entry;
    int offset;
    Position y;
    int previous_entry_number, entry_number, comp_number;
    XtPointer tag;


    if (svnw->svn.display_mode == DXmSvnKdisplayTree || svnw->svn.num_entries == 0 || svnw->svn.location_cursor == 1)
	return;

        
    /*
    **  If we are hiding the selections, then treat this as a non-shift arrow up
    */
    if (svnw->svn.show_selections == FALSE)
       {
       if (navig_flag == 0)	    /* entry extend */
	    svn_arrow_up((Widget) svnw, event);
       else
	    svn_page_up((Widget) svnw, event);

       return;
       };

    /*
    **  If multiple selections are not allowed, there is already a selection made,
    **  AND it is not this entry, then treat this as a normal arrow up.
    */
    if ((svnw->svn.multiple_selections == FALSE) &&
	(svnw->svn.num_selections == 1) &&
	    (!svnw->svn.location_cursor_entry->selected))       {
	 if (navig_flag == 0)
	    svn_arrow_up((Widget) svnw, event);
	 else
	    svn_page_up((Widget) svnw, event);
	    
         return;
       };

    /*
    **  If the entry is insensitive, then ignore it.
    */
    if (svnw->svn.location_cursor_entry->grayed) return;



    /* Get the entry that we are position on now.. */

    previous_entry = svnw->svn.location_cursor_entry;
    previous_entry_number = svnw->svn.location_cursor;


    if (navig_flag == SCROLL_ENTRY) 
	entry_number = svnw->svn.location_cursor = svnw->svn.location_cursor - 1;


    if (navig_flag == SCROLL_PAGE)
    	{
	/******************************************************************************/
	/*                                                                            */
	/* If this is a PAGE extend request.. then we want to place the location      */
	/* cursor at approx. the same Y location as it was on the last page... There- */
	/* fore we have to jump some hoops to get the new location cursor entry num.  */
	/*                                                                            */
	/******************************************************************************/
	offset = LclOutlineMapNumberToOffset(svnw, svnw->svn.location_cursor);
	y = LclOutlineMapOffsetToY(svnw, offset);
	}	

    /******************************************************************************/
    /*                                                                            */
    /* If we are extending the selection by a PAGE we automatically will have to  */
    /* scroll the display by a page ... the OutlineVScroll routine will Get any   */
    /* entries that have not been known to SVN yet...  by doing a GetEntry callback. */
    /*										  */
    /* If we are only extended by a entry , then only check to see if we are at   */
    /* the first entry.								  */
    /*                                                                            */
    /******************************************************************************/
    switch (navig_flag)
	{
	case SCROLL_PAGE : 	/* extend selection by a page */
	    {
	    scroll.reason = XmCR_PAGE_DECREMENT;
	    OutlineVScroll(svnw, 0, &scroll);

	    DXmSvnMapPosition((Widget) svnw, previous_entry->x, y, &entry_number, &comp_number, (XtPointer *) &tag);
	    svnw->svn.location_cursor = entry_number;
	    break;
	    }

	case SCROLL_ENTRY :		/* extended selection by an entry */
	    {
	    int k, found = FALSE;
	
	    for (k = 1; k <= svnw->svn.display_count; k++)
		{
		if (svnw->svn.location_cursor == svnw->svn.entries[k])
		    found = TRUE;
		}

	    if (!found)
		{
		DXmSvnDisableDisplay((Widget) svnw);
		LclOutlinePositionJump(svnw, svnw->svn.location_cursor);
		DXmSvnEnableDisplay((Widget) svnw);
		}

	    break;
	    }

	case SCROLL_TO_TOP  :
	    {
	    scroll.reason = XmCR_TO_TOP;
	    scroll.pixel = 0;
	    scroll.value = 1;
	    OutlineVScroll(svnw,0, &scroll);
	    entry_number = svnw->svn.location_cursor = 1;
	    break;
	    }

	};  /* End of switch statement */

	
    svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw, entry_number);

    LclUpdateSelectRange (svnw, svnw->svn.location_cursor_entry, entry_number, previous_entry->selected_comp, event->time);
    
    svnw->svn.last_selected_entry = entry_number;
    
/*
**  Tell the caller that all transitions are completed.
*/
    SelectReportTransitions (svnw);

    DisplayHighlightEntry(svnw, previous_entry_number);
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);

    return;

}   /* End of routine LclExtendKeySelectionUp */


#ifdef _NO_PROTO
static void LclExtendKeySelectionDown(svnw, event, navig_flag)

svn_widget  svnw;
XKeyEvent  *event;
int	    navig_flag;
#else
static void LclExtendKeySelectionDown(svn_widget svnw, XKeyEvent *event, int navig_flag)
#endif

{

    XmScrollBarCallbackStruct scroll;
    DXmSvnEntryPtr svnentry, previous_entry;
    int previous_entry_number,entry_number;
    int offset, comp_number;
    XtPointer tag;
    Position y;

    if (svnw->svn.display_mode == DXmSvnKdisplayTree) return;

    if (svnw->svn.num_entries == 0)  return;

    if ((svnw->svn.entries[svnw->svn.display_count] == svnw->svn.location_cursor) &&
	(LclOutlineAtBottom(svnw)) )
	    return;

    /*
    **  If we are hiding the selections, then treat this as a non-shift arrow down
    */
    if (svnw->svn.show_selections == FALSE)
       {
         svn_arrow_up((Widget) svnw, (XKeyEvent *) event);
         return;
       };


    /*
    **  If multiple selections are not allowed, there is already a selection made,
    **  AND it is not this entry, then treat this as a normal arrow down.
    */
    if ((svnw->svn.multiple_selections == FALSE) && (svnw->svn.num_selections == 1) &&
	(!svnw->svn.location_cursor_entry->selected))
       {
	 if (navig_flag == 0)
	    svn_arrow_down((Widget) svnw, (XKeyEvent *) event);
	 else
	    svn_page_down((Widget) svnw, (XKeyEvent *) event);
         return;
       };

    /*
    **  If the entry is insensitive, then ignore it.
    */
    if (svnw->svn.location_cursor_entry->grayed) return;


    /* Get the entry that we are position on now.. */

    previous_entry = svnw->svn.location_cursor_entry;
    previous_entry_number = svnw->svn.location_cursor;


    if (navig_flag == SCROLL_ENTRY)
	entry_number = svnw->svn.location_cursor = svnw->svn.location_cursor + 1;

    if (navig_flag == SCROLL_PAGE)
	{
	/******************************************************************************/
	/*                                                                            */
	/* If this is a PAGE extend request.. then we want to place the location      */
	/* cursor at approx. the same Y location as it was on the last page... There- */
	/* fore we have to jump some hoops to get the new location cursor entry num.  */
	/*                                                                            */
	/******************************************************************************/
	offset = LclOutlineMapNumberToOffset(svnw, svnw->svn.location_cursor);
	y = LclOutlineMapOffsetToY(svnw, offset);
	}	
    
    /******************************************************************************/
    /*                                                                            */
    /* If we are extending the selection by a PAGE we automatically will have to  */
    /* scroll the display by a page ... the OutlineVScroll routine will Get any   */
    /* entries that have not been known to SVN yet...  by doing a GetEntry callback. */
    /*										  */
    /* If we are only extended by a entry , then only check to see if we are at   */
    /* the first entry.								  */
    /*                                                                            */
    /******************************************************************************/
    switch (navig_flag)
	{
	case SCROLL_PAGE:	/* extend selection by a page */
	    {
	    scroll.reason = XmCR_PAGE_INCREMENT;
	    OutlineVScroll(svnw, 0, &scroll);

	    DXmSvnMapPosition((Widget)svnw, previous_entry->x, y, &entry_number, &comp_number, (XtPointer *)&tag);
	    svnw->svn.location_cursor = entry_number;
	    break;
	    }

	case SCROLL_ENTRY :	/* extended selection by an entry */
	    {	    
	    if (svnw->svn.entries[svnw->svn.display_count] == entry_number)
		{
		scroll.reason = XmCR_INCREMENT;
		OutlineVScroll(svnw, 0, &scroll);
		}
	    break;
	    }

	case SCROLL_TO_BOTTOM :	    /* extend selection from current entry to last entry  */
	    {
	    scroll.reason = XmCR_TO_BOTTOM;
	    scroll.pixel = 0;
	    scroll.value = svnw->svn.num_entries;
	    OutlineVScroll(svnw, 0, &scroll);
	    entry_number = svnw->svn.location_cursor = svnw->svn.entries[svnw->svn.display_count];
	    break;
	    }

	};  /* End of switch statement */

	
   svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw, entry_number);

   LclUpdateSelectRange (svnw, svnw->svn.location_cursor_entry, entry_number, previous_entry->selected_comp, event->time);
    
   svnw->svn.last_selected_entry = entry_number;
    
/*
**  Tell the caller that all transitions are completed.
*/
    SelectReportTransitions (svnw);

    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);
    DisplayHighlightEntry(svnw, previous_entry_number);

    return;
    
}   /* end of routine LclExtendKeySelectionDown */

/******************************************************************************/
/*                                                                            */
/* Extend the selection set to contain all entries between anchor point and   */
/* this selected entry							      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void LclSelectExtendedRange (svnw, low_entry, high_entry, component_number, time)

    svn_widget svnw;
    int low_entry, high_entry, component_number;
    Time time;
#else
static void LclSelectExtendedRange (svn_widget svnw, int low_entry, 
          int high_entry, int component_number, Time time)
#endif

{
    Boolean first_select = first_of_many;
    Boolean traverse_up = FALSE;
    DXmSvnEntryPtr svn_entry;
    int i;

    
    if (low_entry < high_entry) traverse_up = TRUE;

    
    for (i = low_entry ;
	    (traverse_up ? (i <= high_entry) : (i >= high_entry)) ;
		    (traverse_up ? i++ : i--) )
        {
        svn_entry = StructGetEntryPtr (svnw, i);

        if (!LclEntrySelected (svnw, svn_entry, i, 0))
	    {
	    LclSelectEntry (svnw, svn_entry, i, component_number, time, first_select);
	    first_select = not_first;
	    }      
        };  /* for loop thru selctions */
	
}	

/******************************************************************************/
/*                                                                            */
/* This routine will handle KNextColumn events (Ctrl/RIGHT arrow)	      */
/*	    FUNCTION: If selection is not SvnKentry, then position selection  */
/*		      to NEXT component field.				      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_next_column(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_next_column(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;
    int	component_num;

    svnw = StructFindSvnWidget (w);

    if (svnw->svn.num_entries == 0) return;

    
    /******************************************************************************/
    /*                                                                            */
    /* If this entry's selection mode is ENTRY only, OR this entry is grayed,     */
    /* OR this entry is not selected then ignore this event...			  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.selection_mode == DXmSvnKselectEntry) || 
	(svnw->svn.location_cursor_entry->grayed) ||
	(!svnw->svn.location_cursor_entry->selected))
	     return;
    
    /******************************************************************************/
    /*                                                                            */
    /* 1) If the currently selected component is less then the start_component	  */
    /* number, then just set the selection to the first component on the secondary */
    /* side of the entry.							  */
    /*										  */
    /* 2) If the currently selected component is greater than the start_component */
    /* number, then set the selection to the next secondary component field,	  */
    /* unless the current selected component is the last.			  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.location_cursor_entry->selected_comp + 1) >
	    svnw->svn.location_cursor_entry->num_components) return;

    if (svnw->svn.location_cursor_entry->selected_comp < svnw->svn.start_column_component)
	component_num = svnw->svn.start_column_component;
    else
	component_num = svnw->svn.location_cursor_entry->selected_comp + 1;


    DXmSvnDisableDisplay((Widget) svnw);
    DXmSvnSelectComponent((Widget) svnw, svnw->svn.location_cursor, component_num);
    DXmSvnEnableDisplay((Widget) svnw);

}   /* End of svn_next_column routine */



/******************************************************************************/
/*                                                                            */
/* This routine will handle KPrev Column events (Ctrl/Left arrow)	      */
/*	    FUNCTION: If selection is not SvnKentry, then position selection  */
/*		      to NEXT component field.				      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_prev_column(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_prev_column(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;
    int	component_num;

    svnw = StructFindSvnWidget (w);

    if (svnw->svn.num_entries == 0) return;

    
    /******************************************************************************/
    /*                                                                            */
    /* If this entry's selection mode is ENTRY only, OR this entry is grayed,     */
    /* OR this entry is not selected then ignore this event...			  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.selection_mode == DXmSvnKselectEntry) || 
	(svnw->svn.location_cursor_entry->grayed) ||
	(!svnw->svn.location_cursor_entry->selected))
	     return;
    
    /******************************************************************************/
    /*                                                                            */
    /* 1) If the currently selected component is <= start_column_component, then  */
    /*    just return because the selection is already there...			  */
    /*										  */
    /* 2) Otherwise just decrement the currently selected component number ...	  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.location_cursor_entry->selected_comp - 1) == 0) return;

    if (svnw->svn.location_cursor_entry->selected_comp <= svnw->svn.start_column_component)
	component_num = 1;  /* only highlight the primary components ... */
    else
	component_num = svnw->svn.location_cursor_entry->selected_comp -1;


    DXmSvnDisableDisplay((Widget) svnw);
    DXmSvnSelectComponent((Widget) svnw, svnw->svn.location_cursor, component_num);
    DXmSvnEnableDisplay((Widget) svnw);

}   /* End of svn_next_column routine */

/******************************************************************************/
/*                                                                            */
/* This routine will handle ENTER window events.			      */
/*	    FUNCTION:  Remove the dragging timeout if it exists...	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_enter(w, event, params, num_params)

  Widget	    w;
  XCrossingEvent    *event;
  char		    params;
  int		    num_params;
#else
static void svn_enter(Widget w, XCrossingEvent *event, char params, int num_params)
#endif

{
    svn_widget svnw;
    int	component_num;


    /**************************************************************************/
    /*                                                                        */
    /* Enter/Leave events may be delivered with one of three separate modes:  */
    /*									      */
    /* 1. NotifyNormal: The pointer has moved into or out of the window	      */
    /*									      */
    /* 2. NotifyGrab: A grab has been activated.			      */
    /*									      */
    /* 3. NotifyUngrab: A grab has been deactivated.			      */
    /*									      */
    /* Cases 2 and 3 aren't real enter/leave events, and GObE has no interest */
    /* in them.  So we'll just return and ignore the event if it's not	      */
    /* normal.								      */
    /*                                                                        */
    /**************************************************************************/
    if (event->mode != NotifyNormal)
	return;


    svnw = StructFindSvnWidget (w);

    if (svnw->svn.num_entries == 0) return;


    /* If a timeout exists , remove it */
    
    if (svnw->svn.drag_id)
	{
	XtRemoveTimeOut(svnw->svn.drag_id);
	svnw->svn.drag_id = 0;
	}

}   /* End of svn_enter proc */		



/******************************************************************************/
/*                                                                            */
/* This routine will handle LEAVE window events.			      */
/*	    FUNCTION:  
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_leave(w, event)

  Widget w;
  XCrossingEvent *event;
#else
static void svn_leave(Widget w, XCrossingEvent *event)
#endif

{
    svn_widget svnw;
    int	interval;
    Widget event_w;

    svnw = StructFindSvnWidget (w);

    /**************************************************************************/
    /*                                                                        */
    /* Enter/Leave events may be delivered with one of three separate modes:  */
    /*									      */
    /* 1. NotifyNormal: The pointer has moved into or out of the window	      */
    /*									      */
    /* 2. NotifyGrab: A grab has been activated.			      */
    /*									      */
    /* 3. NotifyUngrab: A grab has been deactivated.			      */
    /*									      */
    /* Cases 2 and 3 aren't real enter/leave events, and GObE has no interest */
    /* in them.  So we'll just return and ignore the event if it's not	      */
    /* normal.								      */
    /*                                                                        */
    /**************************************************************************/
    if ((event->mode != NotifyNormal) && (svnw->svn.leave_direction == 0))
	return;

    if (svnw->svn.num_entries == 0) return;

    if ((svnw->svn.last_event.type != BUTTONDOWN) ||
	(svnw->svn.last_event.type != BUTTON2DOWN))
	    return;


    event_w = svnw->svn.primary_window_widget;
    
    /******************************************************************************/
    /*                                                                            */
    /* Determine which direction this leave event occurred on window.		  */
    /*                                                                            */
    /******************************************************************************/
    svnw->svn.leave_direction = 0;  /* initialize field */

    if (event->y >= (int)event_w->core.height)	    /* Bottom */
	svnw->svn.leave_direction |= BOTTOMLEAVE;
    if (event->y <= (int)event_w->core.y)		    /* Top */
	svnw->svn.leave_direction |= TOPLEAVE;
    if (event->x <= (int)svnw->core.x)		    /* Left */
	svnw->svn.leave_direction |= LEFTLEAVE;
    if (event->x >= (int)svnw->core.width)	    /* Right */
	svnw->svn.leave_direction |= RIGHTLEAVE;

    if (svnw->svn.leave_direction == 0)
	{
	svnw->svn.drag_id = 0;
	return;
	}

    interval = 200;

    if (svnw->svn.last_event.type == BUTTONDOWN)
	{
	 svnw->svn.drag_id = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)svnw),
					(unsigned long)interval,
					(XtTimerCallbackProc) LclProcessAutoScroll,
					(XtPointer) svnw);
	}
    else
	{
	 svnw->svn.drag_id = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)svnw),
					(unsigned long)interval,
					(XtTimerCallbackProc) LclManageDragging,
					(XtPointer) svnw);
	}
	

}   /* End of svn_leave proc */

/******************************************************************************/
/*                                                                            */
/* This routine will handle KPrevLevel events (Ctrl/UP arrow)		      */
/*	    FUNCTION: Scroll the window UP to the previous level entry.	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_prev_level(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_prev_level(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;
    int previous_loc_cursor;

    svnw = StructFindSvnWidget (w);

    if (svnw->svn.num_entries == 0) return;

    svnw->svn.button_down = TRUE;
    svnw->svn.button_top = 1;	/* signal top button */

    LclOutlineScrollButton (svnw);

}    


/******************************************************************************/
/*                                                                            */
/* This routine will handle KNextLevel events (Ctrl/DOWN arrow)		      */
/*	    FUNCTION: Scroll the window DOWN to the next level entry.	      */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
static void svn_next_level(w, event)

  Widget w;
  XKeyEvent *event;
#else
static void svn_next_level(Widget w, XKeyEvent *event)
#endif

{
    svn_widget svnw;
    int previous_loc_cursor;
    
    svnw = StructFindSvnWidget (w);

    if (svnw->svn.num_entries == 0) return;

    svnw->svn.button_down = TRUE;
    svnw->svn.button_top  = 3;	/* signal bottom button */

    LclOutlineScrollButton (svnw);

}    



/*
**  The following page is the class record and procedures for a new class
**  called the SvnWindow.  The only procedures used in this class are the
**  Realize procedure (which creates the window) and the geometry_manager
**  routine which will adjust the entry height.
**
**  The procedures are placed first so that forward references are not needed.
*/

/*
**  This routine will realize the SvnWindow widget.  It is responsible for 
**  creating the widget window.
*/

#ifdef _NO_PROTO
static void DXmSvnWindowRealize (svnww, valuemask, attributes)

  SvnWindowWidget svnww;
  Mask *valuemask;
  XSetWindowAttributes *attributes;
#else
static void DXmSvnWindowRealize (SvnWindowWidget svnww, Mask *valuemask, 
           XSetWindowAttributes *attributes)
#endif

{
/*
**  Local data declarations
*/
    Mask lclValueMask = *valuemask;


/*
**  Make it forget gravity
*/
    attributes->bit_gravity = NorthWestGravity;
    lclValueMask |= CWBitGravity;
    

/*
**  Create the X window
*/
    XtCreateWindow ((Widget)svnww, InputOutput, CopyFromParent, lclValueMask, attributes);
}

/*
**  This routine will get called when this window is resized.
*/

#ifdef _NO_PROTO
static void DXmSvnWindowResize (unused_svnww)

  Widget unused_svnww;
#else
static void DXmSvnWindowResize (Widget unused_svnww)
#endif

{
/*
**  Do nothing.  This routine is necessary because the SVN_DISPLAY module calls
**  the resize procedure of the widgets.  I tried XtInheritResize, but could
**  not get it to work.
*/
}

/*
**  This is the class geometry management routine.
*/
#if 0
#ifdef _NO_PROTO
static XtGeometryResult DXmSvnWindowGeometryManager (w, g)

    Widget w;
    XtWidgetGeometry *g;
#else
static XtGeometryResult DXmSvnWindowGeometryManager (Widget w, XtWidgetGeometry *g)
#endif

{
/*
**  Local Variables
*/
    svn_widget svnw;
    DXmSvnEntryPtr svnentry;
    int entry_number, comp_offset;
   

/*
**  Fulfill the request from the subwidget
*/
    if (g->request_mode & CWX          ) w->core.x            = g->x;
    if (g->request_mode & CWY          ) w->core.y            = g->y;
    if (g->request_mode & CWBorderWidth) w->core.border_width = g->border_width;
    if (g->request_mode & CWWidth      ) w->core.width        = g->width;
    if (g->request_mode & CWHeight     ) w->core.height       = g->height;


/*
**  Go up through the parent chain looking for the svn class widget.
*/
    svnw = StructFindSvnWidget (w);


/*
**  Get the first entry structure
*/
    svnentry = svnw->svn.entryPtr;
    entry_number = 1;


/*
**  Spin through the following loop until the entry with this as a subwidget
**  is found or the end of the list is hit.
*/
    while (svnentry != (DXmSvnEntryPtr) NULL)
       {
	 for (comp_offset = 0;  comp_offset <= svnentry->num_components - 1;  comp_offset++)
             if (svnentry->entrycompPtr[comp_offset].var.is_widget.readwrite_text == w)
	        {
		  /*
		  **  Disable the display, and redraw the entry and leave the while loop.
		  */
		      DXmSvnDisableDisplay (svnw);
		      DisplaySvnInvalidateEntry (svnw, entry_number);
		      DXmSvnEnableDisplay (svnw);
		      break;
		};
         entry_number++;
         svnentry = svnentry->next;
        };


/*
**  Return confirmation
*/
    return XtGeometryYes;
}
#endif


/*
**  Translation tables for primary/secondary scroll windows
*/


static const char simple_window_translations [] =
   "<GraphicsExpose>: SVNGRAPHICSEXPOSE()\n\
    <NoExpose>: SVNGRAPHICSEXPOSE()\n\
    <FocusOut>: SVNFOCUSOUT()\n\
    <FocusIn>: SVNFOCUSIN()\n\
    ~Shift ~Meta ~Alt ~Ctrl <Key>osfDown:	SVNARROWDOWN()\n\
    ~Shift ~Meta ~Alt ~Ctrl <Key>osfUp:		SVNARROWUP()\n\
    ~Shift ~Ctrl <Key>osfPageDown:		SVNPAGEDOWN()\n\
    ~Shift ~Ctrl <Key>osfPageUp:		SVNPAGEUP()\n\
    ~Shift ~Meta  Alt  Ctrl <Key>osfLeft:	SVNTOTOP()\n\
    ~Shift ~Meta  Alt  Ctrl <Key>osfRight:	SVNTOBOTTOM()\n\
     Shift ~Meta ~Alt ~Ctrl <Key>osfUp:		SVNEXTENDUP()\n\
     Shift ~Meta ~Alt ~Ctrl <Key>osfDown:	SVNEXTENDDOWN()\n\
     Shift ~Meta ~Alt ~Ctrl <Key>osfPageUp:	SVNEXTENDPAGEUP()\n\
     Shift ~Meta ~Alt ~Ctrl <Key>osfPageDown:	SVNEXTENDPAGEDOWN()\n\
     Shift ~Meta ~Alt  Ctrl <Key>osfLeft:	SVNEXTENDTOTOP()\n\
     Shift ~Meta ~Alt  Ctrl <Key>osfRight:	SVNEXTENDTOBOTTOM()\n\
    ~Shift ~Meta ~Alt ~Ctrl <Key>osfLeft:	SVNARROWLEFT()\n\
    ~Shift ~Meta ~Alt ~Ctrl <Key>osfRight:	SVNARROWRIGHT()\n\
    ~Shift ~Meta  Alt ~Ctrl <Key>osfLeft:	SVNTOLEFT()\n\
    ~Shift ~Meta  Alt ~Ctrl <Key>osfRight:	SVNTORIGHT()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>osfPageUp:	SVNPAGELEFT()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>osfPageDown:	SVNPAGERIGHT()\n\
    ~Shift ~Meta ~Alt ~Ctrl <Key>Return:	SVNACTIVATE()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>Return:	SVNACTIVATE()\n\
    ~Shift ~Meta ~Alt ~Ctrl <Key>space:		SVNSELECT()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>space:		SVNSELECT()\n\
    <Key>osfSelect:				SVNSELECT()\n\
    <Key>osfActivate:			        SVNACTIVATE()\n\
    <Key>osfHelp:				SVNHELP()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>slash:		SVNSELECTALL()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>backslash:	SVNDESELECTALL()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>osfLeft:	SVNPREVCOLUMN()\n\
    ~Shift ~Meta ~Alt  Ctrl <Key>osfRight:	SVNNEXTCOLUMN()\n\
     Shift ~Ctrl ~Meta ~Alt <Key>Tab:		SVNPREVTABGROUP()\n\
    ~Shift ~Ctrl ~Meta ~Alt <Key>Tab:		SVNNEXTTABGROUP()\n\
    ~Shift Ctrl ~Meta ~Alt <Key>Tab:		SVNPREVTABGROUP()\n\
    <Key>osfCancel:			        SVNCANCEL()\n\
    ~Shift ~Meta ~Alt Ctrl <Key>osfUp:		SVNPREVLEVEL()\n\
    ~Shift ~Meta ~Alt Ctrl <Key>osfDown:	SVNNEXTLEVEL()\n\
    ";


static XtActionsRec window_ActionsList[] = {
    {"SVNGRAPHICSEXPOSE",(XtActionProc)svn_graphics_expose},
    {"SVNFOCUSOUT",	(XtActionProc)svn_focus_out},
    {"SVNFOCUSIN",	(XtActionProc)svn_focus_in},
    {"SVNARROWDOWN",	(XtActionProc)svn_arrow_down},
    {"SVNARROWUP",	(XtActionProc)svn_arrow_up},
    {"SVNPAGEDOWN",	(XtActionProc)svn_page_down},
    {"SVNPAGEUP",	(XtActionProc)svn_page_up},
    {"SVNTOTOP",	(XtActionProc)svn_scroll_to_top},
    {"SVNTOBOTTOM",	(XtActionProc)svn_scroll_to_bottom},
    {"SVNARROWLEFT",	(XtActionProc)svn_arrow_left},
    {"SVNARROWRIGHT",	(XtActionProc)svn_arrow_right},
    {"SVNTOLEFT",	(XtActionProc)svn_scroll_to_left},
    {"SVNTORIGHT",	(XtActionProc)svn_scroll_to_right},
    {"SVNPAGELEFT",	(XtActionProc)svn_page_left},
    {"SVNPAGERIGHT",	(XtActionProc)svn_page_right},
    {"SVNEXTENDUP",	(XtActionProc)svn_extend_up},
    {"SVNEXTENDDOWN",	(XtActionProc)svn_extend_down},
    {"SVNEXTENDPAGEUP",	(XtActionProc)svn_extend_pageup},
    {"SVNEXTENDPAGEDOWN",(XtActionProc)svn_extend_pagedown},
    {"SVNEXTENDTOTOP",	  (XtActionProc)svn_extend_to_top},
    {"SVNEXTENDTOBOTTOM", (XtActionProc)svn_extend_to_bottom},
    {"SVNACTIVATE",	(XtActionProc)svn_activate_entry},
    {"SVNSELECT",	(XtActionProc)svn_select_entry},
    {"SVNHELP",		(XtActionProc)svnhelp},
    {"SVNSELECTALL",	(XtActionProc)svn_select_all},
    {"SVNDESELECTALL",  (XtActionProc)svn_deselect_all},
    {"SVNNEXTCOLUMN",	(XtActionProc)svn_next_column},
    {"SVNPREVCOLUMN",	(XtActionProc)svn_prev_column},
    {"SVNCANCEL",	(XtActionProc)svn_cancel},
    {"SVNNEXTTABGROUP",  (XtActionProc) LclTraverseNextTabGroup},
    {"SVNPREVTABGROUP",  (XtActionProc) LclTraversePrevTabGroup},
    {"SVNPREVLEVEL",	(XtActionProc) svn_prev_level},
    {"SVNNEXTLEVEL",	(XtActionProc) svn_next_level} 
    };

/*
**  This is the class record that gets set at compile/link time this is what 
**  is passed to the widgetcreate routine as the class.  All fields must be 
**  inited at compile time.
*/
/*#pragma nostandard*/
externaldef(DXmSvnwindowwidgetclassrec) 
/*#pragma standard*/
SvnWindowClassRec DXmSvnwindowwidgetclassrec 
  = {
      {
         /* CORE CLASS RECORD   */
         /* superclass          */   (WidgetClass) &xmManagerClassRec,
         /* class_name          */   "SvnWindow",
         /* widget_size         */   sizeof(SvnWindowRec),
         /* class_initialize    */   NULL,
         /* class_part_init     */   NULL,
         /* class_inited        */   FALSE,
         /* initialize          */   NULL,
         /* initialize_hook     */   NULL,
         /* realize             */   (XtRealizeProc)DXmSvnWindowRealize,
         /* actions             */   window_ActionsList,
	 /* num_actions         */   XtNumber(window_ActionsList),
         /* resources           */   NULL,
         /* num_resources       */   0,
         /* xrm_class           */   NULLQUARK,
         /* compress_motion     */   TRUE,
         /* compress_exposure   */   FALSE,
         /* compress_enterleave */   FALSE,
         /* visible_interest    */   FALSE,
         /* destroy             */   NULL,
         /* resize              */   DXmSvnWindowResize,
         /* expose              */   (XtExposeProc)svn_expose,
         /* set_values          */   NULL,
         /* set_values_hook     */   NULL,
         /* set_values_almost   */   XtInheritSetValuesAlmost,
         /* get_values_hook     */   NULL,
         /* accept_focus        */   NULL,
         /* version (temp off)  */   XtVersion,
         /* callback offsetlst  */   NULL,
         /* tm_table            */   simple_window_translations,
         /* query geometry      */   NULL,
         /* display accelerator */   NULL,
         /* extension           */   NULL,
      },

      {
         /* COMP CLASS RECORD   */
         /* childrens geom mgr  */   (XtGeometryHandler) GeometryManager,
         /* set changed proc    */   XtInheritChangeManaged,
         /* add a child         */   XtInheritInsertChild,
         /* remove a child      */   XtInheritDeleteChild,
         /* extension           */   (XtPointer)NULL, 
      },
      {
	 /* Constraint class record */
				     NULL,
				     0,
				     0,
				     NULL,
				     NULL,
				     NULL,
				     NULL,
      },

      {		
	/* MANAGER CLASS        */
	/* translations		*/   NULL,
	/* get resources	*/   NULL,
	/* num get_resources 	*/   0,			
	/* get_cont_resources   */   NULL,
        /* num_get_cont_resources */ 0,					
        /* parent_process       */   XmInheritParentProcess,
	/* extension		*/   NULL,					
      },

      {                            
         /* extension           */   (XtPointer) NULL,
      }
    };

/*#pragma nostandard*/
externaldef(dxmSvnWindowWidgetClass) 
/*#pragma standard*/
SvnWindowClass dxmSvnWindowWidgetClass = &DXmSvnwindowwidgetclassrec;

/*
**  This routine creates an SVN window widget...
*/

#ifdef _NO_PROTO
Widget DXmSvnWindow (w, parent, name, width, height, window_num)

  Widget	  w;
  Widget          parent; 
  char           *name;
  int             width, height, window_num; 
#else
Widget DXmSvnWindow (Widget w, Widget parent, char *name, int width, int height, int window_num)
#endif

{
/*
**  Routine data declarations
*/
    Arg arglist[20];
    int argcnt = 0;
    svn_widget svnw = StructFindSvnWidget (w);
    

/******************************************************************************/
/*                                                                            */
/* See which simple window we are trying to create... depending on the window */
/* depends on the attachments to the form.				      */
/*                                                                            */
/******************************************************************************/
    switch (window_num)
	{
	
	case 1:	    /* Main primary window widget */
	    {
	    XtSetArg (arglist[argcnt], XmNbottomAttachment, XmATTACH_FORM); argcnt++;  
	    XtSetArg (arglist[argcnt], XmNtopAttachment, XmATTACH_FORM);    argcnt++;
	    XtSetArg (arglist[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
	    XtSetArg (arglist[argcnt], XmNrightAttachment, XmATTACH_FORM); argcnt++;
	    break;
	    }

	case 2:	    /* Primary path-to-root window widget */
	    {
	    XtSetArg (arglist[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
	    XtSetArg (arglist[argcnt], XmNrightAttachment, XmATTACH_FORM); argcnt++;
	    XtSetArg (arglist[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
	    break;
	    }

	case 3:	    /* Main secondary window widget */
	    {
	    XtSetArg (arglist[argcnt], XmNbottomAttachment, XmATTACH_FORM); argcnt++;  
	    XtSetArg (arglist[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
	    XtSetArg (arglist[argcnt], XmNtopAttachment, XmATTACH_FORM);    argcnt++;
	    XtSetArg (arglist[argcnt], XmNrightAttachment, XmATTACH_FORM); argcnt++;
	    break;
	    }

	case 4:	    /* Secondary path-to-root window widget */
	    {
	    XtSetArg (arglist[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
	    XtSetArg (arglist[argcnt], XmNrightAttachment, XmATTACH_FORM); argcnt++;
	    XtSetArg (arglist[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
	    break;
	    }

	}   /* end of switch */


    /*
    **  Pack the supplied arguments and default the x and y and borders
    */  
    XtSetArg (arglist[argcnt], XmNheight, height); argcnt++;
    XtSetArg (arglist[argcnt], XmNwidth, width); argcnt++;
    XtSetArg (arglist[argcnt], XmNtraversalOn, TRUE);  argcnt++;
    XtSetArg (arglist[argcnt], XmNrecomputeSize, TRUE); argcnt++;


/*
**  The XtCreateWidget routine will allocate the class and instance records for the widget.
*/
    return (XtCreateWidget (name, (WidgetClass)dxmSvnWindowWidgetClass, parent, arglist, argcnt));
}
