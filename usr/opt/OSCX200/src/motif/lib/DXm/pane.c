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
#ifdef VMS
/*
 * Note: Using #module in Unix will get interpreted by the preprocessor as a 
 *	 control line and blow up. Do not uncomment the following line.
 *
 * module PANE "3.2"
 */
#else
#ifndef lint
static char rcsid[] = "$Header: /usr/sde/osf1/rcs/x11/src/motif/lib/DXm/pane.c,v 1.1.6.4 1993/12/17 21:18:20 Richard_June Exp $";
#endif /* lint */
#endif /* VMS  */
/*
 * Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/*
 * V1.0a    12-Jan-1988	MRR
 *	    Add our logicals to #includes.
 * V1.0b    26-Feb-1988	RDB
 *	    Fix bugs with allowresizing & geometry manager
 * V1.0c    01-Mar-1988	PMB
 *	    Merge with latest copy of pane widget.
 * V1.0d    17-Mar-1988	RDB
 *	    fix major bugs and enhancements
 * V1.1	    1-Apr-1988	MR
 *	    Make work with BL7.2 - add class_part_initialize,
 *	    add time argument to AcceptFocus.
 * V1.2	    13-Apr-1988	JV
 *	    Make work with BL7.6 - changed DwtNcallback to DwtNmapCallback
 * V1.3	    20-Apr-1988	MRR
 *	    Upward compatibility support.
 * V1.4	    13-Jun-1988	MRR
 *	    Add help support.
 * V1.5	    30-Jun-1988	MRR
 *	    Support for Help-MBx.
 * V1.6	    19-Jul-1988	MRR
 *	    Help modifier changed to @Help.
 * V1.7	    20-Jul-1988	RDB
 *	    Initialization Bug
 * V1.8	    01-Sep-1988	RDB
 *	    Resize on DwtPaneSetMax error
 *	    Knob draging problem
 * V1.9	    07-Sep-1988	RDB
 *	    Color changes on 8 plane GPX on Knob drag
 * V1.10    08-Sep-1988	RDB
 *	    Extra knob appearing when managing and unmanaging widgets
 * V1.11    06-Oct-1988 RDB
 *	    During Resize check if widget height also changed.
 * V1.12    19-Oct-1988	RDB
 *	    change cursor to vertical or horizontial cursor and 
 *	    MB2 & MB3 pushes fixed
 * V1.13    31-Oct-1988 RDB
 *	    Add DwtPaneAddChild calls
 * V1.14    01-Nov-1988	RDB
 *	    Change resize to resizable and allowresizing to resize
 * V1.15    07-Nov-1988	RDB
 *	    Allow shared panes
 * V1.16    29-Nov-1988 RDB
 *	    Add Destory routine
 *	    Add SetValues routine
 *	    Fix SetConstraintValues routine
 *	    Add DeleteChild routine
 *	    Add FocusCallbacks
 *	    Add MapCallbacks
 *	    Fix FocusNext, FocusPrev routines
 *	    Fix compiledtextbindings
 *	    Add VMS routines for all the public routines.
 * V1.17    06-Jan-1989	JG
 *	    Make work on PMAX
 * V2.0     15-Feb-1989 MRR
 *	    AcceptFocus should return what child's accept_focus returns.
 * V2.1     22-Feb-1989 RDB
 *	    Fix set min and max resize but when pane is shared
 * V2.2	    1-Mar-1989	MRR
 *	    GC's gotten with XtGetGC should be removed with XtDestroyGC,
 *	    not XFreeGC.
 * V2.3	    7-Mar-1989	RDB
 *	    Orientation (w) defined incorrectly
 * V2.4	    9-Mar-1989	MRR
 *	    CvtStringToOrientation and DwtPrivate no longer needed.
 * V2.5	    9-Mar-1989	MRR
 *	    Copy DwtUpdateCallback here and rename as LclUpdateCallback.
 * V2.6	    14-Mar-1989	RDB
 *	    Change the default width and height to be 200 instead of 100
 * V2.7	    23-Mar-1989	RDB
 *	    change resource default for Foreground
 * V2.8	    24-mar-1989	RDB
 *	    Make knobs mullions
 * V2.9	    30-Mar-1989	RDB
 *	    Redo definitions for panewidgetclass, panewidgetclassrec, mullions
 *	    Style guide compliancy on vertical & horizontal cursor
 * V2.10    04-Apr-1989	RDB
 *	    Change MullionSize to be something like borderwidth &
 *	    add MullionLength to be the total height of a mullion
 * V2.11    05-Apr-1989	RDb
 *	    Make LclUpdateCallback static
 * V2.12    03-May-1989	RDB
 *	    MB2 & MB3 cursor changes
 * V2.13    12-May-1989	RDB
 *	    make widget_size the PanePart not PaneRec
 * V2.14    15-May-1989	RDB
 *	    Geometry Manager doesn't allow widgets to resize
 *	    DwtPaneMakeViewable error in selecting right subwidget
 * V3.0	    22-Aug-1989 MRR
 *	    Copied to V3 library.
 * V3.1	    29-Aug-1989	RDB
 *	    Fix DwtPaneMakeViewable error in setting the widget position
 * V3.2	    30-AUG-1989 RDB
 *	    Remove Const from Joel
 * V3.3	    23-Oct-1989	RDB
 *	    New version of pane widget
 * 	    09-Jan-1990	SDL
 *	    Run this module through the Motif converter DXM_PORT.COM
 * 	    09-Jan-1990	WDW
 *	    Make post-motif-port-adjustments.
 *	    10-Jan-1990 WDW + SDL
 *	    Add VMSDescToNull which was supplied by J. Vangilder.
 *	    Make sure shorts, Dimensions, and Positions were not
 *	    intermixed.
 *	    11-Jan-1990 WDW + SDL
 *	    Change level of indirection for callback lists in
 *	    LclUpdateCallback and calls to this routine.
 *	    Change resource name and class for resize to resizePolicy.
 *	    15-Jan-1990 WDW
 *	    Change level of indirection for callback lists in
 *	    LclUpdateCallback and calls to this routine.
 *          26-Jan-1990 WDW
 *	    Ultrix compatibility.
 *          31-Jan-1990 WDW
 *	    Modify headers so private part is in panep.h
 * 	    01-Feb-1990	SDL
 *	    To compile cleanly with /STANDARD=PORT, add #pragma standard 
 *	    lines around #include and externaldefs, fix up #endifs.  Change 
 *	    various lines using VPaneCursor, HPaneCursor, OrigLoc(pane) and
 *	    visable_child to assign to and test for 0 instead of NULL.
 * 	    07-Feb-1990	SDL
 *	    Fix an accvio that occurs if an application attempts to change
 *	    callbacks using SetValues.  Motif does the memory deallocation
 *	    and therefore we don't need to XtRemoveAllCallbacks the old
 *	    callbacks and XtAddCallbacks the new ones (SetValues no longer
 *	    needs to call the LclUpdateCallback routine).
 *  
 *	    24-Apr-1990	  AN
 *	    Fix a bug that Roger Brinkley found with columm separator...
 *	    08-May-1990	  AN
 *	    Take out panefield macros and place them in PaneP.h
 *
 *	    17-May-1990	    AN
 *	    Change bcopy to memcpy
 *	    20-Nov-1990	    AN
 *	    Made Pane widget subclass off of MOTIF manager class.
 *	    21-Nov-1990	    RDB
 *	    Remove a bunch of old code for focus and add traversal stuff
 *	    28-Nov-1990	    AN
 *	    Make sure pane class structure inherits translations
 *	    10-May-1991	    AN
 *	    Change mullion class to XmPrimitive so that TAB groups work
 *	    correctly. 
 *	    01-Feb-1993	    AN
 *	    Change messages from static strings to character arrays , to save
 *	    space.
 */
#define PANE

#ifdef VMS
/*#pragma nostandard*/
#include "XmP.h"
#include "MrmPublic.h"
#include "StringDefs.h"
#include "Text.h"
#include "cursorfont.h"
#include "descrip.h"
#include "keysym.h"
#ifdef _NO_PROTO
extern char *VMSDescToNull();  /* jmg */
#else
extern char *VMSDescToNull ( struct dsc$descriptor_s *desc );
#endif
/*#pragma standard*/
#else
#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <X11/StringDefs.h>
#include <Xm/Text.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#endif

#include "PaneP.h"	/* includes Pane.h */
#include "DXmPrivate.h"
#include "DXmMessI.h"

/*---------------------------------------------------*/
/* Message definitions for character arrays	     */
/*---------------------------------------------------*/
#define PANENOSUBWID	 _DXmMsgPane_0000
#define PANEBADWID	 _DXmMsgPane_0001

#ifdef _NO_PROTO
static void MullionInitialize ( );
static Dimension LengthUsed ( );
static void RefigureDlengths ( );
static void TryToFillPane ( );
static void CommitNewSizes ( );
static void InvertIt ( );
static void MoveMagicBorders ( );
static void DrawMagicBorders ( );
static void EraseMagicBorders ( );
static Dimension FindWherePointing ( );
static Widget FindWhichAdjust ( );
static void HandleMullionEvents ( );
static void HandleCancelMullionEvents ( );
static void ClassInitialize ( );
static void Initialize ( );
static void Realize ( );
static void Destroy ( );
static void CheckHeightChanges ( );
static void Resize ( );
static Boolean SetValues ( );
static void LclUpdateCallback ( );
static Boolean SetConstraintValues ( );
static XtGeometryResult GeometryManager ( );
static void ChangeManaged ( );
static void InsertChild ( );
static void DeleteChild ( );
static void Help ( );
static void PaneProcessTab ( );
#else
static void MullionInitialize ( Widget unused_request , Widget w );
static Dimension LengthUsed ( pane_widget pane );
static void RefigureDlengths ( pane_widget pane , Widget sub , int delta );
static void TryToFillPane ( pane_widget pane );
static void CommitNewSizes ( pane_widget pane , Widget dontnotify );
static void InvertIt ( pane_widget pane , Dimension loc , Dimension mullionlength );
static void MoveMagicBorders ( pane_widget pane );
static void DrawMagicBorders ( pane_widget pane );
static void EraseMagicBorders ( pane_widget pane );
static Dimension FindWherePointing ( pane_widget pane , Widget widget , XEvent *event );
static Widget FindWhichAdjust ( pane_widget pane , Widget widget , XEvent *event );
static void HandleMullionEvents ( Widget widget , XEvent *event );
static void HandleCancelMullionEvents ( Widget widget );
static void ClassInitialize ( Widget unused_w );
static void Initialize ( Widget unused_request , Widget w );
static void Realize ( Widget w , Mask *vMask , XSetWindowAttributes *attributes );
static void Destroy ( Widget w );
static void CheckHeightChanges ( pane_widget pane );
static void Resize ( Widget w );
static Boolean SetValues ( Widget old , Widget unused_request , Widget new );
static void LclUpdateCallback ( XtCallbackList *rstruct , Widget s , XtCallbackList *sstruct , char *argname );
static Boolean SetConstraintValues ( Widget current , Widget unused_widget , Widget newWidget );
static XtGeometryResult GeometryManager ( Widget subwidget , XtWidgetGeometry *request , XtWidgetGeometry *reply );
static void ChangeManaged ( Widget w );
static void InsertChild ( Widget subwidget );
static void DeleteChild ( Widget subwidget );
static void Help ( Widget w , XEvent *event );
static void PaneProcessTab ( Widget w , XEvent *event );
#endif /* _NO_PROTO undefined */

#define NoMask                     0L
 
/*
 * 'Length' is defined as height for vpanes, and width for hpanes.  'Breadth'
 * is the other dimension.
 */
 
 
 
#define FindLength(width, height) \
    ((Orientation(pane) == XmVERTICAL) ? (height) : (width))
#define FindBreadth(width, height) \
    ((Orientation(pane) == XmVERTICAL) ? (width) : (height))
 
#define FindWidth(length, breadth) \
    ((Orientation(pane) == XmVERTICAL) ? (breadth) : (length))
#define FindHeight(length, breadth) \
    ((Orientation(pane) == XmVERTICAL) ? (length) : (breadth))
 
#define FindStartLoc(x, y) \
    ((Orientation(pane) == XmVERTICAL) ? (y) : (x))
 
/* Use direct method for Length, safe because core fields guaranteed
 * to be a same offset */
#define Length(widget)	FindLength(widget->core.width, widget->core.height)
#define Breadth(widget)	FindBreadth(widget->core.width, widget->core.height)
#define StartLoc(widget)FindStartLoc(widget->core.x, widget->core.y)
#define LengthMullion(subdata) (subdata->resizable ? MullionLength(pane) : InterSub(pane))

#define EventToTime(time, event)					  \
    switch ((event)->type) {						  \
        case KeyPress: 							  \
        case KeyRelease: 						  \
		(time) = (event)->xkey.time; break;			  \
        case ButtonPress: 						  \
        case ButtonRelease: 						  \
 		(time) = (event)->xbutton.time; break;			  \
	case MotionNotify: 						  \
		(time) = (event)->xmotion.time; break;			  \
	case EnterNotify: 						  \
	case LeaveNotify: 						  \
		(time) = (event)->xcrossing.time; break;		  \
	case PropertyNotify:						  \
		(time) = (event)->xproperty.time; break;		  \
	case SelectionClear: 						  \
		(time) = (event)->xselectionclear.time; break; 		  \
	case SelectionRequest: 						  \
		(time) = (event)->xselectionrequest.time; break;	  \
	case SelectionNotify: 						  \
		(time) = (event)->xselection.time; break;		  \
	default: 							  \
		(time) = CurrentTime; break;				  \
    }
 
static const Dimension	resource_min	    = 1;
static const Dimension	resource_max	    = 10000;
static const Dimension	resource_mullion    = 2;
static const Dimension	resource_spacing = 1;
static const unsigned char	resource_vert = XmVERTICAL;
static const Position 	resource_0_position 	= 0;
 
static XmPartResource resources[] = {
 
    {	XmNorientation, 			/* fixed -- jg */
	XmCOrientation,
	XmROrientation, 
	sizeof(unsigned char),
	XmPartOffset(Pane,orientation), 
	XmROrientation, 
	(XtPointer) &resource_vert},
 
    {	PaneNmullionSize, 			/* fixed -- jg */
	PaneCMullionSize, 
	XmRDimension, 
	sizeof(Dimension),
	XmPartOffset(Pane,mullionsize), 
	XmRDimension, 
	(XtPointer) &resource_mullion},
 
    {	XmNforeground,  			/* fixed -- jg */
	XmCForeground, 
	XmRPixel, 
	sizeof(Pixel),
	XmPartOffset(Pane,foreground), 
	XmRString, 
	XtExtdefaultforeground},
 
    {	XmNspacing,  			/* fixed -- jg */
	XmCSpacing, 
	XmRShort, 
	sizeof(Dimension),
	XmPartOffset(Pane,intersub), 
	XmRShort, 
	(XtPointer) &resource_spacing},
 
    {	XmNresizePolicy, 			/* fixed -- jg */
	XmCResizePolicy, 
	XmRResizePolicy, 
	sizeof(int),
	XmPartOffset(Pane,resize_mode),
	XmRImmediate, 
	(XtPointer) XmRESIZE_GROW},
 
    {	PaneNoverrideText,		/* fixed -- jg */
	PaneCOverrideText, 
	XmRBoolean, 
	sizeof(Boolean),
	XmPartOffset(Pane,overridetext), 
	XmRImmediate, 
	(XtPointer) FALSE},
 
    {   XmNmapCallback,
	XmCCallback, 
	XmRCallback, 
	sizeof(XtCallbackList),
	XmPartOffset(Pane,map_callback), 
	XmRCallback, 
	(XtPointer) NULL},	/* fixed -- jg */
 
    {   XmNunmapCallback,
	XmCCallback, 
	XmRCallback, 
	sizeof(XtCallbackList),
	XmPartOffset(Pane,unmap_callback), 
	XmRCallback, 
	(XtPointer) NULL},
 
    {	XmNfocusCallback,	/* fixed -- jg */
	XmCCallback, 
	XmRCallback, 
	sizeof(XtCallbackList),
	XmPartOffset(Pane,focus_callback), 
	XmRCallback, 
	(XtPointer) NULL},
 
    {	XmNhelpCallback,	/* fixed -- jg */
	XmCCallback, 
	XmRCallback, 
	sizeof(XtCallbackList),
	XmPartOffset(Pane,helpcallback), 
	XmRCallback, 
	(XtPointer) NULL},
};
 
static XtResource constraints[] = {
    {	XmNminimum, 
 	XmCMinimum,
	XmRDimension, 
	sizeof(Dimension),
	XtOffset(Constraints,min), 
	XmRDimension, 
	(XtPointer) &resource_min},
 
    {	XmNmaximum, 
	XmCMaximum, 
	XmRDimension, 
	sizeof(Dimension),
	XtOffset(Constraints,max), 
	XmRDimension, 
	(XtPointer) &resource_max},
 
    {	PaneNposition,
	PaneCPosition,
	XmRPosition, 
	sizeof(Position),
	XtOffset(Constraints,position), 
	XmRPosition,
	(XtPointer) &resource_0_position},
 
    {	XmNresizable,
	XmCBoolean, 
	XmRBoolean, 
	sizeof(Boolean),
	XtOffset(Constraints,resizable), 
	XmRImmediate, 
	(XtPointer) TRUE},
 
    {	PaneNsharedFlag,
	PaneCSharedFlag, 
	XmRInt, 
	sizeof(int),
	XtOffset(Constraints,sharedflag), 
	XmRImmediate, 
	(XtPointer) PaneMpbNotShared},
 
};

 
static const char DefaultMullionTranslation[] =
       "<EnterWindow>:			                mullion-enter()\n\
        <LeaveWindow>:					mullion-exit()\n\
        ~Shift ~Ctrl ~Mod1 ~@Help<Btn1Down>:            mullion-start()\n\
        ~Shift ~Ctrl ~Mod1 ~@Help Button1<PtrMoved>:    mullion-adjust()\n\
        ~Shift ~Ctrl ~Mod1 ~@Help <Btn1Up>:             mullion-end()\n\
        ~Shift ~Ctrl ~Mod1 ~@Help<Btn2Down>:            mullion-cancel()\n\
        ~Shift ~Ctrl ~Mod1 ~@Help<Btn3Down>:            mullion-cancel()\n\
        @Help<Btn1Up>:					Help()";
 
static const char DefaultTranslation[] =
    "@Help<BtnDown>:    				Help()";
 
static XtActionsRec PaneActionsTable[] = 
    {
	{"mullion-enter",		(XtActionProc) HandleMullionEvents},
	{"mullion-exit",		(XtActionProc) HandleMullionEvents},
	{"mullion-start",		(XtActionProc) HandleMullionEvents},
	{"mullion-adjust",		(XtActionProc) HandleMullionEvents},
	{"mullion-end",		(XtActionProc) HandleMullionEvents},
	{"mullion-cancel",		(XtActionProc) HandleCancelMullionEvents},
    	{"Help",                	(XtActionProc) Help},
        {NULL, NULL}
    };

static XtActionsRec MullionActionsTable[] = 
    {
	{"mullion-enter",		(XtActionProc) HandleMullionEvents},
	{"mullion-exit",		(XtActionProc) HandleMullionEvents},
	{"mullion-start",		(XtActionProc) HandleMullionEvents},
	{"mullion-adjust",		(XtActionProc) HandleMullionEvents},
	{"mullion-end",		(XtActionProc) HandleMullionEvents},
	{"mullion-cancel",		(XtActionProc) HandleCancelMullionEvents},
    	{"Help",                	(XtActionProc) Help},
        {NULL, NULL}
    };
 
static Cursor VPaneCursor=0, HPaneCursor=0;

externaldef(panewidgetclassrec) PaneClassRec panewidgetclassrec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &xmManagerClassRec,
    /* class_name         */    "Pane",
    /* widget_size        */	sizeof(PanePart),
    /* class_initialize   */    (XtProc) ClassInitialize,
    /* class_part_initial */	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    (XtInitProc) Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    Realize,
    /* actions            */    PaneActionsTable,
    /* num_actions	  */	XtNumber(PaneActionsTable),
    /* resources          */    (XtResource *) resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    Resize,
    /* expose             */    NULL,
    /* set_values         */    (XtSetValuesFunc) SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    NULL,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version		  */	XtVersionDontCheck,
    /* callbacks          */    NULL,
    /* tm_table           */    DefaultTranslation,
    /* disp accelerator   */    NULL,
    /* extension	  */	NULL
  },{
/* composite_class fields */
    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child	  */	InsertChild,
    /* delete_child	  */	DeleteChild,
    /* extension	  */	NULL
  },{
/* constraint_class fields */
    /* constraint resource list            */ (XtResourceList) constraints,
    /* number of constraints in list       */ XtNumber(constraints),
    /* size of constraint record           */ sizeof(ConstraintsRec),
    /* constraint initialization           */ NULL,
    /* constraint destroy proc             */ NULL,
    /* constraint set_values proc          */ (XtSetValuesFunc) SetConstraintValues,
    /* extension			   */ NULL
  },
  {		
    /* MANAGER CLASS			    */
    /* translations			    */   XtInheritTranslations,
    /* get resources			    */   NULL,
    /* num get_resources		    */   0,			
    /* get_cont_resources		    */   NULL,
    /* num_get_cont_resources		    */	 0,					
    /* parent_process			    */   XmInheritParentProcess,
    /* extension			    */   NULL					
  },
  {
    /* offsets		  */	NULL,
    /* mumble		  */	0	/* Make C compiler happy   */
  }
};

externaldef(panewidgetclass) PaneClass panewidgetclass = &panewidgetclassrec;


externaldef (mullionclassrec) MullionClassRec mullionclassrec = {
    {
    /* superclass         */  (WidgetClass) &xmPrimitiveClassRec,
    /* class name         */   "Mullion",
    /* size               */   sizeof(MullionRec),
    /* class initialize   */   NULL,
    /* class_part_initial */   NULL,
    /* class_inited       */   FALSE,
    /* initialize         */   (XtInitProc) MullionInitialize,
    /* initialize_hook    */   NULL,
    /* realize            */   XtInheritRealize,
    /* actions            */   MullionActionsTable,
    /* num_actions        */   XtNumber(MullionActionsTable),
    /* resources	  */   NULL,
    /* num_resources      */   0,
    /* xrm_class          */   NULLQUARK,
    /* compress_motion    */   TRUE,
    /* compress_exposure  */   TRUE,
    /* compress enterleave*/   TRUE,
    /* visible_interest   */   FALSE,
    /* destroy            */   NULL,
    /* resize             */   NULL,
    /* expose             */   NULL,
    /* set_values         */   NULL,
    /* set_values_hook    */   NULL,
    /* set_values_almost  */   NULL,
    /* get_values_hook    */   NULL,
    /* accept_focus       */   NULL,
    /* version		  */   XtVersionDontCheck,
    /* callbacks          */   NULL,
    /* tm_table           */   DefaultMullionTranslation,
    /* query_geometry	  */   XtInheritQueryGeometry,
    /* display_accelerator*/   NULL,
    /* extension	  */   NULL
  },
  {				        /* Xmprimitive        */
      (XtWidgetProc)_XtInherit,         /* border_highlight   */
      (XtWidgetProc)_XtInherit,         /* border_unhighlight */
      NULL,				/* translations       */
      NULL,                             /* arm_and_activate   */
      NULL,	  	    		/* resources      */
      0,				/* num resources  */
      NULL                             /* extension          */
  }, 
  {
      NULL			    /* mullion class part */
  }
};
 
externaldef( mullionwidgetclass) WidgetClass mullionWidgetClass =  
				(WidgetClass) &mullionclassrec;
 
#define DataFromWidget(subwidget) (Constraints)subwidget->core.constraints

#ifdef _NO_PROTO 
static void MullionInitialize(unused_request, w)
Widget unused_request, w;
#else
static void MullionInitialize(Widget unused_request, Widget w)
#endif
{
    pane_widget pane = (pane_widget) w->core.parent;
    w->core.border_pixel = Foreground(pane);
    w->core.border_width = MullionSize(pane);
    w->core.width = FindWidth(InterSub(pane),Breadth(pane));
    w->core.height = FindHeight(InterSub(pane),Breadth(pane));
    w->core.x = w->core.y = -99;
}
 

/*
 * Figure out what length of the window we're actually using.  Ideally, this
 * will be the same as the window's length, but it will often be different
 * (for example, when we don't have enough windows to take up the entire
 * window's length.)
 */
 
#ifdef _NO_PROTO 
static Dimension LengthUsed(pane)
pane_widget pane;
#else
static Dimension LengthUsed(pane_widget pane)
#endif
{
    Dimension result;
    int i, cnt;
    Constraints subdata, nextsubdata;
    Boolean allfixed;

    if (PaneNumChildren(pane) == 0) return 0;
    result = 0;
    for (i=0 ; i<PaneNumChildren(pane); i++)
	{
	result += Length(PaneChildren(pane)[i]);
	/* The size of the mullion will be the spacing size if:
	 * - the subwidget is a fixed size
	 * - the next subwidgets are all a fixed size 
	 * - the subwidget is the last child (no spacing at all)
	 */
	subdata = DataFromWidget(PaneChildren(pane)[i]);
	if (i < PaneNumChildren(pane) - 1) 
	    { 
	    if (!subdata->resizable)
		{
		result += InterSub(pane);
		}
	    else
		{
		allfixed = TRUE;
		for (cnt = i + 1; cnt < PaneNumChildren(pane); cnt++)
		    {
		    nextsubdata = DataFromWidget(PaneChildren(pane)[cnt]);
		    if (nextsubdata->resizable)
			{
			allfixed = FALSE;
			break;
			}
		    }
		if (allfixed)
		    {
		    result += InterSub(pane);
		    }
		 else
		    {
		    result += MullionLength(pane);
		    }
		}
	    } 
	}
    return result;
}
 
 

/*
 * Set the dheight field of each subwidget to be the appropriate height if we
 * want to move the border below the given subwidget by the given number of
 * pixels.
 */
 
#ifdef _NO_PROTO 
static void RefigureDlengths(pane, sub, delta)
pane_widget pane;
Widget sub;
int delta;
#else
static void RefigureDlengths(pane_widget pane, Widget sub, int delta)
#endif
{
    int cur_above, min_above, max_above;
    int cur_below, min_below, max_below;
    int used;
    Boolean currentlyabove;
    Widget subwidget;
    Constraints subdata;
    int i, which, temp, newlength;
    
    /*
     * if we allow resizing of the pane we will just reset the child 
     * between the min and the max if it isn't already.
     */
    if (ResizeMode(pane) != XmRESIZE_NONE) {
	for (i=0 ; i<PaneNumChildren(pane) ; i++) {
	    subwidget = PaneChildren(pane)[i];
	    subdata = DataFromWidget(subwidget);
	    /*
	     * This only looks tough. If the ResizeMode(pane) == GrowOnly and
	     * the length is less than 0 then DONT reset the dlength to be the
	     * new length. Otherwise make the newlength the dlength.
	     */
	    if (ResizeMode(pane) != XmRESIZE_GROW &&
		    Length(subwidget) > subdata->dlength)
			subdata->dlength = Length(subwidget);
	    /* if the subwidget cannot be resized set the min and max to
	     * the created length of the widget.
	     */ 
	    if (subdata->resizable)
		{
		if (subdata->dlength < subdata->min)
		    subdata->dlength = subdata->min;
		if (subdata->dlength >subdata->max)
		    subdata->dlength = subdata->max;
		}
	    else
		{
		subdata->min = subdata->max = subdata->dlength;
		}
	}/* end for */
	return;
    }
    /* Ok no resizing allowed so we have to be a little more particular
     */
    cur_above = min_above = max_above = 0;
    cur_below = min_below = max_below = 0;
    currentlyabove = TRUE;
    /* Find the length above & including the changed widget & the length
     * below this widget. Total above & below on current, maximium and
     * minimium
     */
    for (i=0 ; i<PaneNumChildren(pane) ; i++) {
	subwidget = PaneChildren(pane)[i];
	subdata = DataFromWidget(subwidget);
	if (currentlyabove) {
	    cur_above += Length(subwidget);
	    min_above += subdata->min;
	    max_above += subdata->max;
	} else {
	    cur_below += Length(subwidget);
	    min_below += subdata->min;
	    max_below += subdata->max;
	}
	if (subwidget == sub) {
	    which = i;
	    currentlyabove = FALSE;
	}
    } /* end for loop */
    if (currentlyabove)
	XtWarning(PANENOSUBWID);
    used = LengthUsed(pane);
    if (used < Length(pane)) min_below -= Length(pane) - used;
    else max_below += used - Length(pane);
    if (delta < min_above - cur_above) delta = min_above - cur_above;
    if (delta < cur_below - max_below) delta = cur_below - max_below;
    if (delta > max_above - cur_above) delta = max_above - cur_above;
    if (delta > cur_below - min_below) delta = cur_below - min_below;
    temp = delta;
    /* a for loop is easier to read here. Also why continue if temp = 0?
     * And further more there is no good reason to reset temp.
     */
    for (i = which; i >= 0; i--)
	{
	subwidget = PaneChildren(pane)[i];
	subdata = DataFromWidget(subwidget);
	/* only make changes if we can resize this widget
	 */
	if (subdata->resizable)
	    {
	    newlength = Length(subwidget) + temp;
	    if (newlength < (int) subdata->min) newlength = subdata->min;
	    if (newlength > (int) subdata->max) newlength = subdata->max;
	    subdata->dlength = newlength;
	    }
	else
	    {
	    subdata->min = subdata->max = subdata->dlength = Length(subwidget);
	    }
	temp -= (subdata->dlength - Length(subwidget));
	}
    /* take what ever was gained on the top and put it on the bottom
     */
    temp = -delta;
    for (i = which+1; i < PaneNumChildren(pane); i++)
	{
	subwidget = PaneChildren(pane)[i];
	subdata = DataFromWidget(subwidget);
	/* only make changes if we can resize this widget
	 */
	if (subdata->resizable)
	    {
	    newlength = Length(subwidget) + temp;
	    if (newlength < (int) subdata->min) newlength = subdata->min;
	    if (newlength > (int) subdata->max) newlength = subdata->max;
	    subdata->dlength = newlength;
	    }
	else
	    {
	    subdata->min = subdata->max = subdata->dlength = Length(subwidget);
	    }
	temp -= (subdata->dlength - Length(subwidget));
	}
}
 
 

#ifdef _NO_PROTO 
static void TryToFillPane(pane)
pane_widget pane;
#else
static void TryToFillPane(pane_widget pane)
#endif
{
    int used, length;
    length = Length(pane);
    used = LengthUsed(pane);
    if (PaneNumChildren(pane) > 0)
	RefigureDlengths(pane, PaneChildren(pane)[PaneNumChildren(pane)-1],
			 length - used);
}
 

#ifdef _NO_PROTO 
static void CommitNewSizes(pane, dontnotify)
pane_widget pane;
Widget dontnotify;
#else
static void CommitNewSizes(pane_widget pane, Widget dontnotify)
#endif
{
    XWindowChanges changes;
    Dimension loc;
    int i, cnt; 
    Widget subwidget, nextsubwidget, mullion;
    Constraints subdata, nextsubdata;
    Boolean allfixed;
    loc = 0;
    for (i=0 ; i<PaneNumChildren(pane) ; i++) 
	{
	/* Do the subwidget first
	 */
	subwidget = PaneChildren(pane)[i];
	subdata = DataFromWidget(subwidget);
	/* depending on the orientation we'll change the subwidgets
	 * x, width or y, height.
	 */
	changes.x = FindWidth(loc, 0);
	changes.y = FindHeight(loc, 0);
	changes.width = FindWidth(subdata->dlength, Breadth(pane));
	changes.height = FindHeight(subdata->dlength, Breadth(pane));
	changes.border_width = 0;
	if (changes.x != subwidget->core.x || changes.y != subwidget->core.y ||
	      changes.width != subwidget->core.width ||
	      changes.height != subwidget->core.height ||
	      changes.border_width != subwidget->core.border_width) 
	    {
	    subwidget->core.x = changes.x;
	    subwidget->core.y = changes.y;
	    subwidget->core.width = changes.width;
	    subwidget->core.height = changes.height;
	    subwidget->core.border_width = changes.border_width;
	    if (XtIsRealized(subwidget))    
		{
		XConfigureWindow(XtDisplay(subwidget), XtWindow(subwidget),
		    (unsigned) CWX |CWY | CWWidth | CWHeight | CWBorderWidth,
		    &changes);
		}
	    if (subwidget != dontnotify &&
		 XtClass(subwidget)->core_class.resize != (XtWidgetProc) NULL)
		{
		(*(subwidget->core.widget_class->core_class.resize))(subwidget);
		}
	    }
	loc += Length(subwidget);

	/* Now position the mullion 
	 */
	mullion = subdata->mullion;
 
	/* Get any new width and height
	 */
	changes.width = FindWidth(InterSub(pane),Breadth(pane));
	changes.height = FindHeight(InterSub(pane),Breadth(pane));
	changes.border_width = MullionSize(pane);
 
	/* Move this mullion where they can't get to it if:
	 * - the subwidget is a fixed size
	 * - the next subwidgets are all a fixed size 
	 * - the subwidget is the last child
	 */
	if (i < PaneNumChildren(pane) - 1) 
	    { 
	    if (!subdata->resizable)
		{
		changes.x = changes.y = -99;
		loc += InterSub(pane);
		}
	    else
		{
		allfixed = TRUE;
		for (cnt = i + 1; cnt < PaneNumChildren(pane); cnt++)
		    {
		    nextsubwidget = PaneChildren(pane)[cnt];
		    nextsubdata = DataFromWidget(nextsubwidget);
		    if (nextsubdata->resizable)
			{
			allfixed = FALSE;
			break;
			}
		    }
		if (allfixed)
		    {
		    changes.x = changes.y = -99;
		    loc += InterSub(pane);
		    }
		 else
		    {
		    changes.x = FindWidth(loc, 0);
		    changes.y = FindHeight(loc, 0);
		    loc += MullionLength(pane);
		    }
		}
 
	    } 
	else 
	    {
    	    changes.x = changes.y = -99;
	    loc += InterSub(pane);
	    }
	if (changes.x != mullion->core.x || 
	    changes.y != mullion->core.y ||
	    changes.width != mullion->core.width || 
	    changes.height != mullion->core.height ||
	    changes.border_width != mullion->core.border_width) 
	    {
	    mullion->core.x = changes.x;
	    mullion->core.y = changes.y;
	    mullion->core.width = changes.width;
	    mullion->core.height = changes.height;
	    mullion->core.border_width = changes.border_width;
	    if (XtIsRealized(mullion))
		{
		XConfigureWindow(XtDisplay(mullion), XtWindow(mullion),
		    (unsigned) CWX | CWY | CWWidth | CWHeight | CWBorderWidth,
		    &changes);
		XRaiseWindow(XtDisplay(mullion), XtWindow(mullion));
		}
	    }
	else
	    {
	    if (XtIsRealized(mullion))
		XRaiseWindow(XtDisplay(mullion), XtWindow(mullion));
	    }
    }
}
 
 

/*
 * Invert the given border.
 */
 
#ifdef _NO_PROTO 
static void InvertIt(pane, loc, mullionlength)
pane_widget pane;
Dimension loc;
Dimension mullionlength;
#else
static void InvertIt(pane_widget pane, Dimension loc, Dimension mullionlength)
#endif
{
    loc += mullionlength / 2 + 1;   /* Deal with X's definition that wide */
				    /* lines are centered around the */
				    /* specified points.  */
 
    XDrawLine(XtDisplay(pane), XtWindow(pane), InvGC(pane),
	      FindWidth(loc, 0), FindHeight(loc, 0),
	      FindWidth(loc, Breadth(pane)), FindHeight(loc, Breadth(pane)));
}
 
 

/*
 * Move the magic borders to the location specified by the current dlength
 * values. 
 */
 
#ifdef _NO_PROTO 
static void MoveMagicBorders(pane)
pane_widget pane;
#else
static void MoveMagicBorders(pane_widget pane)
#endif
{
    Dimension loc;
    int i, cnt;
    Constraints subdata, nextsubdata;
    Boolean allfixed;

    loc = 0;
    for (i=0 ; i<PaneNumChildren(pane) ; i++)
	{
	subdata = DataFromWidget(PaneChildren(pane)[i]);
	loc += subdata->dlength;
	if (loc != subdata->magicborder) 
	    {
	    if (PaneChildren(pane)[i] == WhichAdjust(pane))
		{
		if (subdata->magicborder != OrigLoc(pane)) 
		    InvertIt(pane, (Dimension) subdata->magicborder, (Dimension) LengthMullion(subdata));
		if (loc != OrigLoc(pane)) 
		    InvertIt(pane, (Dimension) loc, (Dimension) LengthMullion(subdata)); 
		}
	    else
		{
		if (subdata->magicborder != 
		   (StartLoc(PaneChildren(pane)[i]) + Length(PaneChildren(pane)[i])))
		    InvertIt(pane, (Dimension) subdata->magicborder, (Dimension) LengthMullion(subdata));
		if (loc != 
		   (StartLoc(PaneChildren(pane)[i]) + Length(PaneChildren(pane)[i])))
		    InvertIt(pane, (Dimension) loc, (Dimension) LengthMullion(subdata));
		}
	    subdata->magicborder = loc;
	    }
	if (i < PaneNumChildren(pane) - 1) 
	    { 
	    if (!subdata->resizable)
		{
		loc += InterSub(pane);
		}
	    else
		{
		allfixed = TRUE;
		for (cnt = i + 1; cnt < PaneNumChildren(pane); cnt++)
		    {
		    nextsubdata = DataFromWidget(PaneChildren(pane)[cnt]);
		    if (nextsubdata->resizable)
			{
			allfixed = FALSE;
			break;
			}
		    }
		if (allfixed)
		    {
		    loc += InterSub(pane);
		    }
		 else
		    {
		    loc += MullionLength(pane);
		    }
		}
	    } 
	else 
	    {
	    loc += InterSub(pane);
	    }
	}
}
 

/*
 * Turn on the magic borders.
 */
 
#ifdef _NO_PROTO 
static void DrawMagicBorders(pane)
pane_widget pane;
#else
static void DrawMagicBorders(pane_widget pane)
#endif
{
    Dimension loc;
    int i, cnt;
    Constraints subdata, nextsubdata;
    Boolean allfixed;

    loc = 0;
    for (i=0 ; i<PaneNumChildren(pane) ; i++) {
	subdata = DataFromWidget(PaneChildren(pane)[i]);
	loc += subdata->dlength;
	subdata->magicborder = loc;
	if (i < PaneNumChildren(pane) - 1) 
	    { 
	    if (!subdata->resizable)
		{
		loc += InterSub(pane);
		}
	    else
		{
		allfixed = TRUE;
		for (cnt = i + 1; cnt < PaneNumChildren(pane); cnt++)
		    {
		    nextsubdata = DataFromWidget(PaneChildren(pane)[cnt]);
		    if (nextsubdata->resizable)
			{
			allfixed = FALSE;
			break;
			}
		    }
		if (allfixed)
		    {
		    loc += InterSub(pane);
		    }
		 else
		    {
		    loc += MullionLength(pane);
		    }
		}
	    } 
	else 
	    {
	    loc += InterSub(pane);
	    }
    }
}
 

/*
 * Turn off the magic borders.
 */
 
#ifdef _NO_PROTO 
static void EraseMagicBorders(pane)
pane_widget pane;
#else
static void EraseMagicBorders(pane_widget pane)
#endif
{
    int		i;
    Constraints subdata;
    for (i=0 ; i<PaneNumChildren(pane) ; i++) 
	{
	if (PaneChildren(pane)[i] == WhichAdjust(pane))
	    {
	    subdata = DataFromWidget(PaneChildren(pane)[i]);
	    if (subdata->magicborder != OrigLoc(pane))
		InvertIt(pane, (Dimension) subdata->magicborder, (Dimension) LengthMullion(subdata));
	    }
	else
	    {
	    subdata = DataFromWidget(PaneChildren(pane)[i]);
	    if (subdata->magicborder != 
	       (StartLoc(PaneChildren(pane)[i]) + Length(PaneChildren(pane)[i])))
		InvertIt(pane, (Dimension) subdata->magicborder, (Dimension) LengthMullion(subdata));
	    }
	}
    XClearWindow(XtDisplay(pane), XtWindow(pane));
}
 
 

/*
 * Find where (lengthwise) in the pane window the given event is pointing.
 * (The event was delivered to the given widget.)
 */
 
#ifdef _NO_PROTO 
static Dimension FindWherePointing(pane, widget, event)
pane_widget pane;
Widget widget;
XEvent *event;
#else
static Dimension FindWherePointing(pane_widget pane, Widget widget, XEvent *event)
#endif
{
    Position x, y;
    x = event->xbutton.x;
    y = event->xbutton.y;
    while (widget != (Widget) pane) {
	x += widget->core.x;
	y += widget->core.y;
	widget = widget->core.parent;
    }
    return FindLength(x, y);
}
 
 

/*
 * Given that the button down event was delivered to the mullion,
 * figure out which window is above the border that the user is trying to
 * adjust.  
 */
 
#ifdef _NO_PROTO 
static Widget FindWhichAdjust(pane, widget, event)
pane_widget pane;
Widget widget;
XEvent *event;
#else
static Widget FindWhichAdjust(pane_widget pane, Widget widget, XEvent *event)
#endif
{
    Widget result;
    Dimension where, loc;
    int i;
    Constraints subdata;

    where = FindWherePointing(pane, widget, event);
    loc = MullionLength(pane)/2 + 1;
    result = PaneChildren(pane)[0];
    for (i=0 ; i<PaneNumChildren(pane) ; i++) {
	widget = PaneChildren(pane)[i];
	if (loc < where) result = widget;
	    else break;
	subdata = DataFromWidget(widget);
	loc += Length(widget) + LengthMullion(subdata);
    }
    return result;
}
 

/*
 * Handle button events in the mullion window.
 */
 
#ifdef _NO_PROTO 
static void HandleMullionEvents(widget, event)
Widget widget;
XEvent *event;
#else
static void HandleMullionEvents(Widget widget, XEvent *event)
#endif
{
    pane_widget pane = (pane_widget) widget->core.parent;
    if (PaneNumChildren(pane) == 0) return;
    switch (event->type) {
      case EnterNotify:
	if (Orientation(pane) == XmVERTICAL)
	    {
	    XDefineCursor (XtDisplay(widget->core.parent), XtWindow(widget->core.parent), VPaneCursor);
	    }
	else
	    {
	    XDefineCursor (XtDisplay(widget->core.parent), XtWindow(widget->core.parent), HPaneCursor);
	    }
	LeftMullion(pane) = False;
	break;
      case LeaveNotify:
	if (WhichAdjust(pane) == NULL)
	    {
	    XUndefineCursor (XtDisplay(widget->core.parent), XtWindow(widget->core.parent));
	    }
	else
	    {
	    LeftMullion(pane) = True;
	    }
	break;
      case ButtonPress:
	WhichAdjust(pane) = FindWhichAdjust(pane, widget, event);
/*
 * The OrigLoc is the Origicnal location of the dividing line between two panes. It is
 * calculated by adding the starting location (x or y depending on orientation) to
 * the length (width or height depending on orientation) 
 */
	OrigLoc(pane) = StartLoc(WhichAdjust(pane)) + Length(WhichAdjust(pane));
	RefigureDlengths(pane, WhichAdjust(pane), 0);
	DrawMagicBorders(pane);
	break;
      case MotionNotify:
	if (WhichAdjust(pane) == NULL) break;
	RefigureDlengths(pane, WhichAdjust(pane),
			 FindWherePointing(pane, widget, event) -
			     OrigLoc(pane));
	MoveMagicBorders(pane);
	break;
      case ButtonRelease:
/* 
 * make sure the operation wasn't canceled 
*/
	if (WhichAdjust(pane) != NULL) 
	    {
	    RefigureDlengths(pane, WhichAdjust(pane),
			     FindWherePointing(pane, widget, event) -
				 OrigLoc(pane));
	    MoveMagicBorders(pane);
	    EraseMagicBorders(pane);
	    CommitNewSizes(pane, (Widget) NULL);
	    WhichAdjust(pane) = NULL;
	    }
	if (LeftMullion(pane))
	    {
	    XUndefineCursor (XtDisplay(widget->core.parent), XtWindow(widget->core.parent));
	    LeftMullion(pane) = False;
	    }
	break;
    }
}
 
/*
 * Handle cancel (MB2 & MB3 press) button events in the mullion window.
 */
 
#ifdef _NO_PROTO 
static void HandleCancelMullionEvents(widget)
Widget widget;
#else
static void HandleCancelMullionEvents(Widget widget)
#endif
{
    pane_widget pane = (pane_widget) widget->core.parent;
    if (PaneNumChildren(pane) == 0) return;
    if (LeftMullion(pane))
	{
	EraseMagicBorders(pane);
	XUndefineCursor (XtDisplay(widget->core.parent), XtWindow(widget->core.parent));
	LeftMullion(pane) = False;
	}
    WhichAdjust(pane) = NULL;
}

#ifdef _NO_PROTO 
static void ClassInitialize(unused_w)
Widget unused_w;
#else
static void ClassInitialize(Widget unused_w)
#endif
{
    XmResolvePartOffsets((WidgetClass) panewidgetclass,
		 &panewidgetclassrec.pane_class.paneoffsets);
}
    
 
 
 

/*
 * Initialize this instance of pane.
 */
 
#ifdef _NO_PROTO 
static void Initialize(unused_request, w)
Widget unused_request, w;
#else
static void Initialize(Widget unused_request, Widget w)
#endif
{
    pane_widget pane = (pane_widget) w;
    XGCValues values;
 
/*
**  Create the pane cursors...
*/
    if (VPaneCursor == 0)
	{
    /*
     *  Local data for the pane cursor
     *	Note: Normally one would do this sort of thing in ClassInitialize but,
     *	setting up cursors requires a live widget which ClassInit doesn't have.
     */
	XColor cursor_fore, cursor_back;
	Font   cursor_font;
	int    cursor_wait;
 
    /*
    **  Set up the colors
    */
	cursor_fore.pixel = 0;
	cursor_fore.red   = 65535;
	cursor_fore.green = 65535;
	cursor_fore.blue  = 65535;
 
	cursor_back.pixel = 0;
	cursor_back.red   = 0;
	cursor_back.green = 0;
	cursor_back.blue  = 0;
#if 0
	cursor_font = XLoadFont (XtDisplay(w), "decw$cursor");
	cursor_wait = decw$c_vpane_cursor;
	VPaneCursor = XCreateGlyphCursor (XtDisplay(w), cursor_font, cursor_font, cursor_wait, 
						     cursor_wait + 1, &cursor_fore, &cursor_back);
	cursor_wait = decw$c_hpane_cursor;
	HPaneCursor = XCreateGlyphCursor (XtDisplay(w), cursor_font, cursor_font, cursor_wait, 
						     cursor_wait + 1, &cursor_fore, &cursor_back);
	XUnloadFont (XtDisplay(w), cursor_font);
#endif
        VPaneCursor = XCreateFontCursor(XtDisplay(w),XC_sb_v_double_arrow);
        HPaneCursor = XCreateFontCursor(XtDisplay(w),XC_sb_h_double_arrow);
	}
 
    if (Width(pane) == 0) Width(pane) = 200; /* Better default?%%% */
    if (Height(pane) == 0) Height(pane) = 200;
    PaneNumChildren(pane) = 0;
    PaneChildren(pane) = NULL;
    WhichAdjust(pane) = NULL;
    OrigLoc(pane) = 0;	/* Don't use NULL because of /STAND=PORT */
    LastHadFocus(pane) = NULL;
    MullionLength(pane) = (MullionSize(pane) * 2) + InterSub(pane);
    values.function = GXinvert;
    values.foreground = BackgroundPixel(pane) ^ Foreground(pane);
    if (values.foreground == 0) values.foreground = 1;
    values.line_width = InterSub(pane);
    if (values.line_width == 1)
	values.line_width = 0;	/* Take advantage of fast server lines. */
    values.plane_mask = BackgroundPixel(pane) ^ Foreground(pane);
    values.fill_style = FillSolid;
    values.subwindow_mode = IncludeInferiors;
    InvGC(pane) = XtGetGC(w,
			       (XtGCMask) GCFunction | GCForeground
			       | GCLineWidth | GCPlaneMask | GCFillStyle
			       | GCSubwindowMode,
			       &values);
}
 

/*
 * Realize the pane widget.
 */
 
#ifdef _NO_PROTO 
static void Realize(w, vMask, attributes)
Widget w;
Mask *vMask;
XSetWindowAttributes *attributes;
#else
static void Realize(Widget w, Mask *vMask, XSetWindowAttributes *attributes)
#endif
{
    pane_widget pane = (pane_widget) w;
    Mask valueMask = *vMask;
 
    if (ResizeMode(pane) != XmRESIZE_NONE) {
/*
 * For now the ResizeMode is forced to XmRESIZE_NONE so that you can move the
 * mullion once you get this beast up. If the user wants to change the pane widget
 * back he should call PaneAllowResizing, do the work in adjusting the widget
 * and then reset the value back.
 * 
 * This needs to be changed and modeled after the DIALOG widget. See the 
 * adapt_to_kids routine.
 */
	ResizeMode(pane) = XmRESIZE_NONE;
	TryToFillPane(pane);
	CommitNewSizes(pane, (Widget) NULL);
    }
    
    attributes->bit_gravity = NorthWestGravity;
    attributes->background_pixel = Foreground(pane);
    valueMask |= CWBitGravity | CWBackPixel;
    
    XtCreateWindow(w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
		   valueMask, attributes);
}
 

/*
 * Destroy the pane widget.
 */
 
#ifdef _NO_PROTO 
static void Destroy(w)
Widget w;
#else
static void Destroy(Widget w)
#endif
{
    pane_widget pane = (pane_widget) w;
 
/* Free the memory allocated for the children that are displayed and those in 
 * the composite field. We created and allocated space for them it so we
 * must Free it.
 */
    XtFree((char *)PaneChildren(pane));
 
/*
 * Remove all callbacks that were created
 */
    XtRemoveAllCallbacks ((Widget)pane, XmNhelpCallback);
    XtRemoveAllCallbacks ((Widget)pane, XmNmapCallback);
    XtRemoveAllCallbacks ((Widget)pane, XmNunmapCallback);
/*
 *  Free the InvGC that was created
 */
    XtDestroyGC (InvGC(pane));
}
 

/*
 * The purpose of this routine is to check and see in the Vertical 
 * orientation if a change in width will also change a subwidgets height
 */
 
#ifdef _NO_PROTO 
static void CheckHeightChanges(pane)
    pane_widget pane;
#else
static void CheckHeightChanges(pane_widget pane)
#endif
{
    XtWidgetGeometry intended, reply;
    Widget subwidget;
    Constraints subdata;
    int i;
 
    if (Orientation(pane) != XmVERTICAL) return; 
 
    intended.request_mode = CWWidth;		/* ask about this width */
 
    for (i=0 ; i<PaneNumChildren(pane); i++) 
	{
	subwidget = PaneChildren(pane)[i];
	subdata = DataFromWidget(subwidget);
	if (!subdata->resizable)
	    {
	    intended.width = Width(pane);   /* if it affects the size */
					    /* he'll tell us */
 
	    switch (XtQueryGeometry (subwidget, &intended, &reply))
		{
		case XtGeometryAlmost:		/* he wants to compromise */
 
		    if ((reply.request_mode & CWHeight) &&
			    subwidget->core.height != reply.height)
			{
			/* take height he suggests */
			subwidget->core.height = reply.height;	
			RefigureDlengths(pane,subwidget,subdata->dlength -
					    reply.height);
			}
		    break;
 
		case XtGeometryYes:
		case XtGeometryNo:
			/* he agrees, no problem w/ */
			/* current height */
		    break;
		} /* end case */
	    } /* end if */
	} /* end for loop */
}
 

/*
 * The pane widget has been resized; handle everything.
 */
 
#ifdef _NO_PROTO 
static void Resize(w)
Widget w;
#else
static void Resize(Widget w)
#endif
{
    pane_widget pane = (pane_widget) w;
    CheckHeightChanges(pane);
    TryToFillPane(pane);
    CommitNewSizes(pane, (Widget) NULL);
}
 

/*
 * Resources in the pane widget have changed; handle it.
 */
 
#ifdef _NO_PROTO 
static Boolean SetValues (old, unused_request, new)
    Widget old, unused_request, new;
#else
static Boolean SetValues (Widget old, Widget unused_request, Widget new)
#endif
{
    pane_widget	oldpane = (pane_widget) old;
    pane_widget	newpane = (pane_widget) new;
    Boolean	redisplay = FALSE;
    XGCValues	values;
 
 
/*
 * For now ignore orientation changes
 */
    if (Orientation(newpane) != Orientation(oldpane))
	{
	Orientation(newpane) = Orientation(oldpane);
	}
 
/*
 * For now ignore resize mode change
 */
    if (ResizeMode(newpane) != ResizeMode(oldpane))
	{
	ResizeMode(newpane) = ResizeMode(oldpane);
	}
/*
 * Something should be done with foreground here but I don't think that works
 * anyway so wait till you fix it latter
 */
 
/*
 * Check the mullion and spacing for changes. If so set redisplay
 */
    if ((MullionSize(newpane) != MullionSize(oldpane)) ||
	(InterSub(newpane) != InterSub(oldpane)))
	    {
/*
 * Change the inverted lines width to InterSub new length
 */
	    if (InterSub(newpane) != InterSub(oldpane))
		{
		values.line_width = InterSub(newpane);
		if (values.line_width == 1)
		    values.line_width = 0;	/* Take advantage of fast server lines. */
		XChangeGC (XtDisplay(newpane),InvGC(newpane), GCLineWidth, &values);
		}
#ifdef bugfixed
/*
 * Currently there is a bug in the XConfirgureWindow which prevents reseting of
 * the border width. In the mean time don't allow any change in MullionSize.
 */
	    MullionLength(newpane) = (MullionSize(newpane) * 2) + InterSub(newpane);
#else
	    MullionSize(newpane) = MullionSize(oldpane);
#endif
/* 
 * if the mullions need to be changed in width and or height they will be changed
 * in the CommitNewSizes routine. 
 */
	    redisplay = TRUE;
	    TryToFillPane(newpane);
	    CommitNewSizes(newpane, (Widget) NULL);
	    }
 
    return redisplay;
}	

/*
 * SetValue on the callback is interpreted as replacing
 * all callbacks
 */

#ifdef _NO_PROTO 
static void
LclUpdateCallback (rstruct, s, sstruct, argname)
    
    XtCallbackList	*rstruct;		/* the real callback list */
    Widget 	  	s;			/* the scratch widget*/
    XtCallbackList	*sstruct;		/* the scratch callback list */
    char          	*argname;
#else
static void
LclUpdateCallback (XtCallbackList *rstruct, Widget s, XtCallbackList *sstruct, char *argname)
#endif
{
#if 0

/* [[ This is no longer needed in Motif. ]] */

    XtCallbackList list;

    /*
     * if a new callback has been specified in the scratch widget,
     * remove and deallocate old callback and init new 
     */
    if (*rstruct != *sstruct)
    {
	list = *sstruct;
	/*
	 *  Copy the old callback list into the new widget, since
	 *  XtRemoveCallbacks needs the "real" widget
    	 */
        *sstruct = *rstruct;
	XtRemoveAllCallbacks(s, argname);
	*sstruct = NULL;
	XtAddCallbacks(s, argname, list);
    }
#endif
}
 
 

#ifdef _NO_PROTO 
static Boolean SetConstraintValues(current, unused_widget, newWidget)
    Widget   current, unused_widget, newWidget;
#else
static Boolean SetConstraintValues(Widget   current, Widget unused_widget, Widget newWidget)
#endif
{
    Constraints cur, new;
    Cardinal	position;
    Dimension	min, max;
    Boolean	resizable;
    Cardinal	sharedflag;
    Boolean	do_add_widget = FALSE;
 
    cur = DataFromWidget(current);
    new = DataFromWidget(newWidget);
    position = new->position;
    min = new->min;
    max = new->max;
    resizable = new->resizable;
    sharedflag = new->sharedflag;
 
    if (max == 0)
	max = resource_max;
    if (min == 0)
	min = resource_min;
/*
 * We have to reset all of these because the address of newWidget what is 
 * acutally stored in the pane widget. If you use the current widget you'll
 * stack dump.
 */
    if (position != cur->position)
	{
	do_add_widget = TRUE;
	new->position = cur->position;
	}
    if (min != cur->min)
	{
	do_add_widget = TRUE;
	new->min = cur->min;
	}
    if (max != cur->max)
	{
	do_add_widget = TRUE;
	new->max = cur->max;
	}
    if (resizable != cur->resizable)
	{
	do_add_widget = TRUE;
	new->resizable = cur->resizable;
	}
    if (sharedflag != cur->sharedflag)
	{
	do_add_widget = TRUE;
	new->sharedflag = cur->sharedflag;
	}
 
/* 
 * This is just like PaneAddWidget so I'll just call it and 
 * let it take care of everything.
 */
    if (do_add_widget)
	{
	PaneAddWidget (newWidget, position, min, max, resizable, sharedflag);
	/* We must reset all of these because it is possible they have changed
	 * and if we don't change them they will just get changed back to what
	 * they were when we leave this routine.
	 */
	current->core.x = newWidget->core.x;
	current->core.y = newWidget->core.y;
	current->core.width = newWidget->core.width;
	current->core.height = newWidget->core.height;
	}
    return TRUE;
}
 

/*
 * One of our subwidgets is trying to resize itself. For now we will only
 * deal with XmRESIZE_NONE. Anything else won't work right.
 */
 
#ifdef _NO_PROTO 
static XtGeometryResult GeometryManager(subwidget, request, reply)
Widget subwidget;
XtWidgetGeometry *request, *reply;
#else
static XtGeometryResult GeometryManager(Widget subwidget, XtWidgetGeometry *request, XtWidgetGeometry *reply)
#endif
{
    XtGeometryResult result;
    pane_widget pane = (pane_widget) subwidget->core.parent;
    Constraints subdata;
    Dimension rlength, rbreadth;

    /* If we don't want the pane to grow on a request from a child, then just return */

    if (ResizeMode(pane) == XmRESIZE_NONE) 
	return XtGeometryNo;

    if ((request->request_mode & CWWidth) == 0)
	request->width = subwidget->core.width;
    if ((request->request_mode & CWHeight) == 0)
	request->height = subwidget->core.height;
    rlength = FindLength(request->width, request->height);
    rbreadth = FindBreadth(request->width, request->height);
    subdata = DataFromWidget(subwidget);
    result = XtGeometryAlmost;
    if (rlength < subdata->min) 
	{
	rlength = subdata->min;
	} 
    else if (rlength > subdata->max) 
	{
	rlength = subdata->max;
	};
    if (rbreadth == Breadth(pane)) 
	{
	if (subdata->dlength == rlength)
	    {
	    /*
	     * I know this seams bizare since everthing looks like it is 
	     * already set. However the Text widget does things in a funny
	     * order and it resets the height back so we must redo the 
	     * CommitNewSizes to get the widget back in sinc with the
	     * constraint.
	     */
	    CommitNewSizes(pane, (Widget) NULL); 
	    result = XtGeometryDone;
	    }
	else
	    {
	    RefigureDlengths(pane, subwidget, rlength - subdata->dlength);
	    CommitNewSizes(pane, (Widget) NULL); 
	    if (subdata->dlength == FindLength(request->width, request->height)) 
		result = XtGeometryDone;
	    };
	}
    if (result == XtGeometryAlmost) 
	{
	*reply = *request;
	rbreadth = Breadth(pane);
        reply->width = FindWidth(subdata->dlength, rbreadth);
	reply->height = FindHeight(subdata->dlength, rbreadth);
	}
    return result;
}
 
 
 

/*
 * One of our subwidgets has changed its managed status.
 */
 
#ifdef _NO_PROTO 
static void ChangeManaged(w)
Widget w;
#else
static void ChangeManaged(Widget w)
#endif
{
    XWindowChanges changes;
    pane_widget pane = (pane_widget) w;
    Widget subwidget, cursubwidget;
    Constraints subdata, cursubdata;
    int position, i;
 
    PaneNumChildren(pane) = 0;
    for (i=(NumChildren(pane)+1)/2 ; i<NumChildren(pane); i++) 
	{
	subwidget = Children(pane)[i];
	if (XtClass (subwidget) == mullionWidgetClass)
	    {
	    continue;
	    }
	subdata = DataFromWidget(subwidget);
 
/* 
 * Recreate the PaneChildren list by only adding those subwidgets which are
 * managed and if shared, are in view. If a subwidget fails the test
 * move his mullion into oblivion. Like the rest of the mullions that don't need 
 * to be seen.
 */
	if (subwidget->core.managed && 
		(subdata->sharedflag == PaneMpbNotShared ||
		subdata->sharedflag == (PaneMpbShared | PaneMpbViewableInPane)))
	    {
	    if (subdata->sharedflag & PaneMpbShared)
		{
		XtSetMappedWhenManaged (subwidget, TRUE);
		}
	    /*
	     * NOTE:The only reason we do the realizing here is because if
	     *	    it isn't realized the mullions don't show up on top. Once
	     *	    mullions are removed you can get rid of this if statement
	     */
	    if (XtIsRealized(pane) && !(XtIsRealized(subwidget)))
		{
		XtRealizeWidget(subwidget);
		};
	    PaneChildren(pane)[(PaneNumChildren(pane))++] = subwidget;
	    }
	else if (subwidget->core.being_destroyed)
	    {
	/*
	 * This subwidget is in the process of being destroyed.
	 * If this widget is shared then give the pane to first subwidget sharing 
	 * the same position.
	 */
	    for (position=(NumChildren(pane)/2)+subdata->position; 
		 position <= NumChildren(pane); 
		 position++)
		{
		cursubwidget = Children(pane) [position-1];
		cursubdata = DataFromWidget (cursubwidget);
		if (cursubdata->position != subdata->position ||
		    cursubwidget == subwidget)
		    continue;
		PaneMakeViewable (cursubwidget);
	    /*
	     * It is necessary to add the subwidget to the PaneChildren list if
	     * we have already gone passed it. It will be in the correct order
	     * since PaneChildren are based on position.
	     */
		if (position < i)
		    PaneChildren(pane)[(PaneNumChildren(pane))++] = subwidget;
		break;
		}
	    }
	else
	    {
	    /*
	     * UnMap any shared panes that aren't used
	     */
	    if (subdata->sharedflag & PaneMpbShared)
		{
		XtSetMappedWhenManaged (subwidget, FALSE);
		XtSetMappedWhenManaged (subdata->mullion, FALSE);
		}
	    else
		{
		/*
		 * Move the mullion widget into oblivion
		 */
		changes.x = changes.y = -99;
		if (changes.x != subdata->mullion->core.x || 
			changes.y != subdata->mullion->core.y)
		    {
		    subdata->mullion->core.x = changes.x;
		    subdata->mullion->core.y = changes.y;
		    if (XtIsRealized(subdata->mullion))
			XConfigureWindow(XtDisplay(subdata->mullion), 
				XtWindow(subdata->mullion), CWX | CWY, &changes);
		    }
		}
	    }
    }
/*
 * Ok now you only have those children that are managed so fill the pane.
 */
    TryToFillPane(pane);
    CommitNewSizes(pane, (Widget) NULL);
}
 

/*
 * A new subwidget has been added.
 */
 
#ifdef _NO_PROTO 
static void InsertChild(subwidget)
Widget subwidget;
#else
static void InsertChild(Widget subwidget)
#endif
{
    pane_widget pane = (pane_widget) subwidget->core.parent;
    Widget cursubwidget;
    Constraints subdata, cursubdata;
    int i, position, n;
    Arg args[2];
 
 
    /* check for mullion class here since when we create the mullion this code
     * will get called again. Get the desired length from the actual length
     * and then create a mullion for this widget. Finally position the widget
     * where the user wants it
     */
    if (XtClass(subwidget) != mullionWidgetClass) 
	{
	n = 0;
#if 0
	XtSetArg(args[n], XmNnavigationType, (XtArgVal) XmTAB_GROUP); n++; 
	XtSetValues(subwidget, args, n); 
#endif
	subdata = (Constraints)subwidget->core.constraints;
	subdata->dlength = Length(subwidget);
	if (subdata->max == 0)
	    subdata->max = resource_max;
	if (subdata->min == 0)
	    subdata->min = resource_min;
	if (subdata->max < subdata->min) 
	    subdata->max = subdata->min;
 
	n = 0;
	XtSetArg(args[n], XmNtraversalOn, (XtArgVal) False); n++;
	XtSetArg(args[n], XmNnavigationType, (XtArgVal) XmNONE); n++;
	subdata->mullion = XtCreateWidget("mullion", (WidgetClass) mullionWidgetClass, (Widget) pane,
				       args, n);
	XtManageChild(subdata->mullion);
	if (subdata->position)
	    {
	    position = subdata->position + ((NumChildren(pane)+1)/2);
	    if (position > (NumChildren(pane)+1))
		{
		position = (NumChildren(pane)+1);
		}
	    }
	else
	    {
	    position = (NumChildren(pane)+1); 
	    }
	subdata->position = position - ((NumChildren(pane)+1)/2); 
	position--;
	} 
    else 
	{
	position = (NumChildren(pane)/2);
	}
    Children(pane) = 
        (WidgetList) XtRealloc((XtPointer) Children(pane),
    	(unsigned) (NumChildren(pane) + 1) * sizeof(Widget));
 
    /* Ripple children up one space from "position" */
    for (i = NumChildren(pane); i > position; i--)
	{
	if (XtClass (Children(pane)[i-1]) != mullionWidgetClass &&
	    XtClass (subwidget) != mullionWidgetClass)
	    {
	    cursubwidget = Children(pane)[i-1];
	    cursubdata = (Constraints)cursubwidget->core.constraints;
	    /* 
	     * Check to see if the position of the current subwidget is less
	     * than that of the subwidget we are creating. If it is then that
	     * means there are other subwidgets below this one which are also
	     * shared and I should exit at this point to store this new 
	     * subwidget in the list of children.
	     */
	    if (cursubdata->position < subdata->position)
		{
		break;
		}
	    else 
		{
		/*
		 * Check to see if the position of the current subwidget and
		 * the created subwidget are the same. If they are and 
		 * the new one is shared, then set the old one to
		 * shared as well. Also set the current subwidgets inview to
		 * false if the subwidget will be in view.
		 *
		 * Otherwise, if the subwidget isn't shared the new one just 
		 * wants the currents ones position so bump it up by one.
		 */
		if ((cursubdata->position == subdata->position) && 
			(subdata->sharedflag & PaneMpbShared))
		    {
		    if (subdata->sharedflag & PaneMpbViewableInPane) 
			{
			cursubdata->sharedflag = PaneMpbShared;
			}
		    else
			{
			/* 
			 * Only set those that aren't shared. Those that are 
			 * shared are already set and since the new one does
			 * not want to be viewable we will just keep what we
			 * have.
			 */
			if (cursubdata->sharedflag == PaneMpbNotShared)
			    {
			    cursubdata->sharedflag = PaneMpbShared | PaneMpbViewableInPane;
			    }
			}
		    }
		else if (subdata->sharedflag == PaneMpbNotShared)
		    {
		    cursubdata->position++;
		    }
		} /* end else */
	    } /* end if */
        Children(pane)[i] = Children(pane)[i-1];
	}
    Children(pane)[i] = subwidget;
    NumChildren(pane)++;
    PaneChildren(pane) = (WidgetList)
	XtRealloc((char *)PaneChildren(pane),
		  (Cardinal) NumChildren(pane) * sizeof(Widget));
 
}
    
 

/*
 * A subwidget has been removed.
 */
 
#ifdef _NO_PROTO 
static void DeleteChild(subwidget) 
Widget subwidget;
#else
static void DeleteChild(Widget subwidget)
#endif
{
    pane_widget pane = (pane_widget) subwidget->core.parent;
    Constraints subdata;
    int i, position;
 
/*
 * Destroy will destroy mullion and the child. If it is a mullion being 
 * deleted skip the stuff about subdata and shared position.
 */
    if (XtClass(subwidget) != mullionWidgetClass) 
	{
	subdata = DataFromWidget(subwidget);
    /* 
     * destory the mullion 
     */
	XtDestroyWidget (subdata->mullion);
	}
/*
 * Ok now we need to remove this widget from the composite list 
 */
    for (position = 0; position < NumChildren(pane); position++) 
	{
        if (Children(pane) [position] == subwidget) 
	    {
            break;
	    }
	}
 
    if (position == NumChildren(pane)) return;
 
    /* Ripple children down one space from "position" */
    NumChildren(pane)--;
    for (i = position; i < NumChildren(pane); i++) 
	{
        Children(pane) [i] = Children(pane) [i+1];
	}
} 
 
 

/*---------------------------------------------------*/
/* this routine will be called from the widget's     */
/* main event handler via the translation manager.   */
/*---------------------------------------------------*/
 
#ifdef _NO_PROTO 
static void Help(w, event)
Widget w;
XEvent *event;
#else
static void Help(Widget w, XEvent *event)
#endif
{
    XmAnyCallbackStruct temp;
 
    temp.reason = XmCR_HELP;
    temp.event = event;
 
    if (XtClass(w) == (WidgetClass) panewidgetclass)
	{
    	XtCallCallbacks(w, XmNhelpCallback, &temp);
	}
    else
	{
	XtCallCallbacks(XtParent(w), XmNhelpCallback, &temp);
	};
}
 
#ifdef _NO_PROTO 
static void PaneProcessTab(w, event)
Widget w;
XEvent *event;
#else
static void PaneProcessTab(Widget w, XEvent *event)
#endif
{
    XmAnyCallbackStruct temp;


} 

/*
 *************************************************************************
 *
 * Public creation entry points
 *
 *************************************************************************
 */
/*
 * low level create entry point
 */
 
#ifdef _NO_PROTO 
Widget PaneCreateWidget (p, name, al, ac)
    Widget  p;				/* parent widget */
    char    *name;			/* pane widget name */
    ArgList al;
    Cardinal ac;    
#else
Widget PaneCreateWidget (Widget p, char *name, ArgList al, Cardinal ac)
#endif
{
    return XtCreateWidget(name, (WidgetClass) panewidgetclass, p, al, ac);
}
 
    
/*
 * high level pane create routine
 */
 
#ifdef _NO_PROTO 
Widget PaneWidget(p, 
	      name, 
	      x, y, 
	      width, height,
	      orientation,
	      mapcallback, helpcallback)
 
    Widget         p;				/* parent widget */
    char          *name;			/* pane widget name */
    Position	   x, y;			/* location of pane widget */
    Dimension	   width, height;		/* size of pane widget */
    int		   orientation;			/* horizontal or vertical */
    XtCallbackList mapcallback;			/* callback for data struct */
    XtCallbackList helpcallback;		/* help requested */
#else
Widget PaneWidget(Widget p, char *name, Position x, Position y, Dimension width, 
Dimension height, int orientation, XtCallbackList mapcallback, XtCallbackList helpcallback)
#endif
{
    Arg al[25];
    Cardinal ac = 0;
 
    /*
     * set up the parameters we are given
     */
 
    XtSetArg (al[ac], XtNname,		name);		ac++;
    XtSetArg (al[ac], XtNx,		x);		ac++;
    XtSetArg (al[ac], XtNy,		y);		ac++;
    XtSetArg (al[ac], XtNwidth,		width);		ac++;
    XtSetArg (al[ac], XtNheight,	height);	ac++;
    XtSetArg (al[ac], XmNorientation,	orientation);	ac++;
 
 
    if (mapcallback != NULL) 
	{
	XtSetArg (al[ac], XmNmapCallback,     mapcallback);
	ac++;
	}
 
    if (helpcallback != NULL) 
	{
	XtSetArg (al[ac], XmNhelpCallback,    helpcallback);
	ac++;
	}
 
    return XtCreateWidget (name, (WidgetClass) panewidgetclass, p, al, ac);
}

    
/*
 * high level pane add widget routine
 */
 
#ifdef _NO_PROTO 
void PaneAddWidget(subwidget, position, min, max, resizable, sharedflag)
    Widget	subwidget;
    Cardinal	position;
    Dimension	min, max;
    Boolean	resizable;
    Cardinal	sharedflag;
#else
void PaneAddWidget(Widget subwidget, Cardinal position, Dimension min, 
    Dimension max, Boolean resizable, Cardinal sharedflag)
#endif
{
    XWindowChanges changes;
    pane_widget	pane = (pane_widget) subwidget->core.parent;
    Widget	cursubwidget;
    Constraints subdata, cursubdata, cursubdata2;
    int		i, curpos, lastsharedposition;
    Boolean	subnotfound=TRUE, subnotplaced=TRUE, cursubnotreset=TRUE;
 
/*
 * Get the position the window is currently in
 */
    subdata = DataFromWidget(subwidget);
    curpos = subdata->position;
 
/*
 * Compare that to the position the user desires and change the windows around
 */
    if (position > (NumChildren(pane)/2))
	position = (NumChildren(pane)/2);
    if (position != curpos && position != 0)
	{
	if (position < curpos)
	    {
	    for (i = NumChildren(pane); i >= position*2; i--)
		{
		cursubwidget = Children(pane)[i-1];
		cursubdata = DataFromWidget(cursubwidget);
		/* 
		 * Check to see if the position of the current subwidget is less
		 * than that of the subwidget. If it is then that
		 * means there are other subwidgets below this one which are also
		 * shared and I should exit at this point to store this new 
		 * subwidget in the list of children.
		 */
		if (cursubdata->position < position)
		    {
		    Children(pane)[i] = subwidget;
		    subdata->position = position;
		    break;
		    }
		/*
		 * if subwidget came from a shared position we need to make 
		 * sure that at least one of the old widgets that shared the 
		 * position will now be displayed
		 */
		if (cursubdata->position == subdata->position && 
			subdata->sharedflag & PaneMpbShared)
		    {
		    if (cursubnotreset)
			{
			cursubdata->sharedflag = PaneMpbShared | PaneMpbViewableInPane;
			cursubnotreset = FALSE;
			}
		    }
		/*
		 * move the subwidgets up one unless 
		 * the current subwidget is subwidget we are trying to change or
		 * the current subs position > the current position or
		 */
		if (cursubwidget == subwidget || subnotfound)
		    {
		    if (cursubwidget == subwidget)
			{
			subnotfound = FALSE;
			continue;
			}
		    }
		else
		    {
		    Children(pane)[i] = Children(pane)[i-1];
		    }
		/*
		 * Reposition the current subwidgets position as appropriate
		 */
		if (subnotfound)
		    {
		    /*
		     * This is a little tricky here. This will before you find
		     * the subwidget. If the subwidget was not formerly 
		     * shared and is not to be shared || the subwidget was formerly
		     * shared and will be shared at its new position we will not
		     * change the position. For the other two conditions we do.
		     * In one case it is necessary to subtract and the other add
		     * to the position
		     */
		    if ((subdata->sharedflag == PaneMpbNotShared) &&
			(sharedflag & PaneMpbShared))
			    cursubdata->position--;
		    else if ((subdata->sharedflag & PaneMpbShared) &&
			(sharedflag == PaneMpbNotShared))
			    cursubdata->position++;
		    }
		else
		    {
		    /*
		     * Only change the position if our subwidget wont be shared.
		     * Otherwise the position will still be occupied.
		     */
		    if (sharedflag == PaneMpbNotShared)
			cursubdata->position++;
		    }
		/*
		 * Check to see if the position of the current subwidget and
		 * the subwidget's new position are the same. If they are and 
		 * the new one is shared, then set the old one to
		 * shared as well. Also set the current subwidgets inview to
		 * false if the subwidget will be in view.
		 *
		 * Otherwise, if the subwidget isn't shared the new one just 
		 * wants the currents ones position so bump it up by one.
		 */
		if(cursubdata->position == position && 
			sharedflag & PaneMpbShared)
		    {
		    if (sharedflag & PaneMpbViewableInPane) 
			{
			cursubdata->sharedflag = PaneMpbShared;
			}
		    else
			{
			/* 
			 * Only set those that aren't shared. Those that are 
			 * shared are already set and since the new one does
			 * not want to be viewable we will just keep what we
			 * have.
			 */
			if (cursubdata->sharedflag == PaneMpbNotShared)
			    {
			    cursubdata->sharedflag = PaneMpbShared | PaneMpbViewableInPane;
			    }
			}
		    }
		}   /* end for loop */
	    }
	else	/* position > curpos */
	    {
	    for (i = (NumChildren(pane)/2)+curpos; i <= NumChildren(pane); i++)
		{
		cursubwidget = Children(pane)[i-1];
		cursubdata = DataFromWidget(cursubwidget);
		/*
		 * Even though you start at the current position it is possible
		 * for the position to be less than the current position due to
		 * shared panes.
		 */
		if (cursubdata->position < curpos || cursubwidget == subwidget
		    || (subnotfound))
		    {
		    if (cursubwidget == subwidget)
			    subnotfound = FALSE;
		    continue;
		    }
		/*
		 * if subwidget came from a shared position we need to make 
		 * sure that at least one of the old widgets that shared the 
		 * position will now be displayed
		 */
		if (cursubdata->position == subdata->position && 
			subdata->sharedflag & PaneMpbShared)
		    {
		    if (cursubnotreset)
			{
			cursubdata->sharedflag = PaneMpbShared | PaneMpbViewableInPane;
			cursubnotreset = FALSE;
			}
		    }
		/*
		 * move the subwidgets up one unless 
		 * the current subwidget is subwidget we are trying to change or
		 * the current subs position > the desired position or
		 * the current subs position = the desired postion and the sub
		 *   we were working with was previously shared.
		 */
		if (cursubdata->position > position ||
		     ((subdata->sharedflag & PaneMpbShared) && 
		       cursubdata->position == position))
		    {
		    /* 
		     * Place subwidget only once after you moved all subs
		     */
		    if (subnotplaced)
			{
			Children(pane)[i-2] = subwidget;
			subdata->position = position;
			subnotplaced=FALSE;
			}
		    }
		else
		    {
		    Children(pane)[i-2] = Children(pane)[i-1];
		    }
		/*
		 * Reposition the current subwidgets position as appropriate
		 */
		if (subnotplaced)
		    {
		    /*
		     * Only change the position if our subwidget wasn't shared.
		     * Otherwise the position will still be occupied.
		     */
		    if (subdata->sharedflag == PaneMpbNotShared)
			cursubdata->position--;
		    }
		else
		    {
		    /*
		     * This is a little tricky here. This will happen after you
		     * placed the subwidget. If the subwidget was not formerly 
		     * shared and is not to be shared || the subwidget was formerly
		     * shared and will be shared at its new position we will not
		     * change the position. For the other two conditions we do.
		     * In one case it is necessary to subtract and the other add
		     * to the position
		     */
		    if ((subdata->sharedflag == PaneMpbNotShared) &&
			(sharedflag & PaneMpbShared))
			    cursubdata->position--;
		    else if ((subdata->sharedflag & PaneMpbShared) &&
			(sharedflag == PaneMpbNotShared))
			    cursubdata->position++;
		    }
		/*
		 * Check to see if the position of the current subwidget and
		 * the subwidget's new position are the same. If they are and 
		 * the new one is shared, then set the old one to
		 * shared as well. Also set the current subwidgets inview to
		 * false if the subwidget will be in view.
		 *
		 * Otherwise, if the subwidget isn't shared the new one just 
		 * wants the currents ones position so bump it up by one.
		 */
		if(cursubdata->position == position && 
			sharedflag & PaneMpbShared)
		    {
		    if (sharedflag & PaneMpbViewableInPane) 
			{
			cursubdata->sharedflag = PaneMpbShared;
			}
		    else
			{
			/* 
			 * Only set those that aren't shared. Those that are 
			 * shared are already set and since the new one does
			 * not want to be viewable we will just keep what we
			 * have.
			 */
			if (cursubdata->sharedflag == PaneMpbNotShared)
			    {
			    cursubdata->sharedflag = PaneMpbShared | PaneMpbViewableInPane;
			    }
			}
		    }
		}   /* end for loop */
	    }			    
	/*
	 * Now that the position has been properly set we can take care of 
	 * the shared flag. 
	 */
	subdata->sharedflag = sharedflag;
	/*
	 * Just for grins lets loop back through and make sure that if a 
	 * subwidget says that it is shared it really is.
	 */
	for (i=NumChildren(pane)/2; i<NumChildren(pane); i++) 
	    {
	    cursubwidget = Children(pane)[i];
	    cursubdata = DataFromWidget(cursubwidget);
	    if (cursubdata->sharedflag & PaneMpbShared)
		{
		if (i+1 != NumChildren(pane))	/* Dont check passed bountry */
		    {
		    cursubdata2 = DataFromWidget(Children(pane)[i+1]);
		    if (cursubdata->position == cursubdata2->position)
			lastsharedposition = cursubdata->position;
		    }
		if (lastsharedposition != cursubdata->position)
		    {
		    cursubdata->sharedflag = PaneMpbNotShared;
		    XtSetMappedWhenManaged (cursubwidget, TRUE);
		    }
		}
	    }
	}
    else
	{
	/*
	 * There was no change in position so if the user wants this subwidget
	 * to be viewable lets do it for him.
	 */
	if (sharedflag & PaneMpbViewableInPane)
	    PaneMakeViewable (subwidget);
	}
 
    subdata->resizable = resizable;
    if (max == 0)
	max = resource_max;
    if (min == 0)
	min = resource_min;
    if (max < min) 
	max = min;
    if (subdata->resizable)
	{
	subdata->min = min;
	subdata->max = max;
	}
    if (subwidget->core.managed)
	{
	PaneNumChildren(pane) = 0;
	for (i=NumChildren(pane)/2; i<NumChildren(pane); i++) 
	    {
	    cursubwidget = Children(pane)[i];
	    cursubdata = DataFromWidget(cursubwidget);
 
    /* 
     * Recreate the PaneChildren list by only adding those subwidgets which are
     * managed and if shared, are in view. If a subwidget fails the test
     * move his mullion into oblivion. Like the rest of the mullions that don't need 
     * to be seen.
     */
	    if (cursubwidget->core.managed && 
		    (cursubdata->sharedflag == PaneMpbNotShared ||
		    cursubdata->sharedflag == (PaneMpbShared | PaneMpbViewableInPane)))
		{
		if (cursubdata->sharedflag & PaneMpbShared)
		    {
		    XtSetMappedWhenManaged (cursubwidget, TRUE);
		    }
		PaneChildren(pane)[(PaneNumChildren(pane))++] = cursubwidget;
		}
	    else
		{
		/*
		 * UnMap any shared panes that aren't used
		 */
		if (cursubdata->sharedflag & PaneMpbShared)
		    {
		    XtSetMappedWhenManaged (cursubwidget, FALSE);
		    XtSetMappedWhenManaged (cursubdata->mullion, FALSE);
		    }
		else
		    {
		    /*
		     * Move the mullion widget into oblivion
		     */
		    changes.x = changes.y = -99;
		    if (changes.x != cursubdata->mullion->core.x || 
			    changes.y != cursubdata->mullion->core.y)
			{
			cursubdata->mullion->core.x = changes.x;
			cursubdata->mullion->core.y = changes.y;
			if (XtIsRealized(cursubdata->mullion))
			    XConfigureWindow(XtDisplay(cursubdata->mullion), 
				    XtWindow(cursubdata->mullion), CWX | CWY, &changes);
			}
		    }
		}
	    } /* end for loop */
	} /* end if subwidget is managed */
    /*
     * Ok now you only have those children that are managed so fill the pane.
     */
/*
 * Check to see if the dlength for the subwidget is greater than the max. If it
 * is then I must call RefigureDlengths instead of TryToFillPane. TryToFillPane
 * will use the current length which hasn't been change so it will pass a delta
 * change to RefigureDlengths of 0 which does zippo. Since I know the amount I
 * need to change I'll just call RefigureDlengths directly.
 */
	if (subdata->dlength > subdata->max)
	    {
	    RefigureDlengths (pane, subwidget, subdata->dlength - subdata->max);
	    }
	else if (subdata->dlength < subdata->min)
	    {
	    RefigureDlengths (pane, subwidget, subdata->dlength - subdata->min);
	    }
	else
	    {
	    TryToFillPane(pane);
	    }
	CommitNewSizes(pane, (Widget) NULL);
}

#ifdef _NO_PROTO 
void PaneGetMinMax(subwidget, min, max)
Widget subwidget;
Dimension *min, *max;
#else
void PaneGetMinMax(Widget subwidget, Dimension *min, Dimension *max)
#endif
{
    Constraints subdata;
    subdata = DataFromWidget(subwidget);
    if (subdata) {
	*min = subdata->min;
	*max = subdata->max;
    } else
	XtWarning(PANEBADWID);
}

 

/*
 * NOTE: In setting the min or max in PaneSetMinMax, PaneSetMin, or
 *	PaneSetMax we donot check for valid min, max values. Values less
 *	than 1 will certianly cause stack dumps and min > max will also 
 *	cause some form of headache, though I haven't tried it yet. Most
 *	DW code doesn't check for valid attributes so I see no reason to
 *	do so here either.
 */
 
#ifdef _NO_PROTO 
void PaneSetMinMax(subwidget, min, max)
Widget subwidget;
Dimension min, max;
#else
void PaneSetMinMax(Widget subwidget, Dimension min, Dimension max)
#endif
{
    pane_widget pane = (pane_widget) subwidget->core.parent;
    Constraints subdata;
 
    if (max == 0)
	max = resource_max;
    if (min == 0)
	min = resource_min;
    if (max < min) 
	max = min;
    subdata = DataFromWidget(subwidget);
    if (subdata->resizable)
	{
	subdata->min = min;
	subdata->max = max;
	}

    if (subdata->sharedflag & PaneMpbShared)
	{
	Widget subwidget2;
	Constraints subdata2;
	int i, visable_child=0;

	for (i=(NumChildren(pane)+1)/2 ; i<NumChildren(pane); i++) 
	    {
	    subwidget2 = Children(pane)[i];
	    if (XtClass (subwidget) == mullionWidgetClass)
		{
		continue;
		}
	    subdata2 = DataFromWidget(subwidget2);
	    if ((subdata2->position != subdata->position) ||
		(subwidget2 == subwidget))
		{
		continue;
		};
	    subdata2->min = subdata->min;
	    subdata2->max = subdata->max;
	    if (subdata2->sharedflag & PaneMpbViewableInPane)
		{
		visable_child = i;
		}
	    }
	if (visable_child != 0)
	    {
	    subwidget = Children(pane)[visable_child];
	    subdata = DataFromWidget(subwidget);
	    }
	}
    /* only try to fill pane and commit sizes if we really need to
     */
    if (subdata->dlength < subdata->min || subdata->dlength > subdata->max)
	{
/*
 * Check to see if the dlength for the subwidget is greater than the max. If it
 * is then I must call RefigureDlengths instead of TryToFillPane. TryToFillPane
 * will use the current length which hasn't been change so it will pass a delta
 * change to RefigureDlengths of 0 which does zippo. Since I know the amount I
 * need to change I'll just call RefigureDlengths directly.
 */
	if ((subdata->dlength > subdata->max) && (PaneNumChildren(pane) > 0))
	    {
	    RefigureDlengths (pane, subwidget, subdata->dlength - subdata->max);
	    }
	else if ((subdata->dlength < subdata->min) && (PaneNumChildren(pane) > 0))
	    {
	    RefigureDlengths (pane, subwidget, subdata->dlength - subdata->min);
	    }
	else
	    {
	    TryToFillPane(pane);
	    }
	CommitNewSizes(pane, (Widget) NULL);
	}
}
 

#ifdef _NO_PROTO 
void PaneSetMin(subwidget, min)
Widget subwidget;
Dimension min;
#else
void PaneSetMin(Widget subwidget, Dimension min)
#endif
{
    pane_widget pane = (pane_widget) subwidget->core.parent;
    Constraints subdata;
    if (min == 0)
	min = resource_min;
    subdata = DataFromWidget(subwidget);
    if (subdata->resizable)
	{
	subdata->min = min;
	if (subdata->max < subdata->min)
	    subdata->max = subdata->min;
	}

    if (subdata->sharedflag & PaneMpbShared)
	{
	Widget subwidget2;
	Constraints subdata2;
	int i, visable_child=0;

	for (i=(NumChildren(pane)+1)/2 ; i<NumChildren(pane); i++) 
	    {
	    subwidget2 = Children(pane)[i];
	    if (XtClass (subwidget) == mullionWidgetClass)
		{
		continue;
		}
	    subdata2 = DataFromWidget(subwidget2);
	    if ((subdata2->position != subdata->position) ||
		(subwidget2 == subwidget))
		{
		continue;
		};
	    subdata2->min = subdata->min;
	    if (subdata2->sharedflag & PaneMpbViewableInPane)
		{
		visable_child = i;
		}
	    }
	if (visable_child != 0)
	    {
	    subwidget = Children(pane)[visable_child];
	    subdata = DataFromWidget(subwidget);
	    }
	}

    if (subdata->dlength < subdata->min)
	{
	RefigureDlengths (pane, subwidget, subdata->dlength - subdata->min);
	CommitNewSizes(pane, (Widget) NULL);
	}
}
 

#ifdef _NO_PROTO 
void PaneSetMax(subwidget, max)
Widget subwidget;
Dimension max;
#else
void PaneSetMax(Widget subwidget, Dimension max)
#endif
{
    pane_widget pane = (pane_widget) subwidget->core.parent;
    Constraints subdata;
    if (max == 0)
	max = resource_max;
    subdata = DataFromWidget(subwidget);
    if (max < subdata->min) 
	max = subdata->min;
    if (subdata->resizable)
	{
	subdata->max = max;
	}
    if (subdata->sharedflag & PaneMpbShared)
	{
	Widget subwidget2;
	Constraints subdata2;
	int i, visable_child=0;

	for (i=(NumChildren(pane)+1)/2 ; i<NumChildren(pane); i++) 
	    {
	    subwidget2 = Children(pane)[i];
	    if (XtClass (subwidget) == mullionWidgetClass)
		{
		continue;
		}
	    subdata2 = DataFromWidget(subwidget2);
	    if ((subdata2->position != subdata->position) ||
		(subwidget2 == subwidget))
		{
		continue;
		};
	    subdata2->max = subdata->max;
	    if (subdata2->sharedflag & PaneMpbViewableInPane)
		{
		visable_child = i;
		}
	    }
	if (visable_child != 0)
	    {
	    subwidget = Children(pane)[visable_child];
	    subdata = DataFromWidget(subwidget);
	    }
	}
    if (subdata->dlength > subdata->max)
	{
	RefigureDlengths (pane, subwidget, subdata->dlength - subdata->max);
	CommitNewSizes(pane, (Widget) NULL);
	}
}
 

/* this needs to be incorporated into set values */
#ifdef _NO_PROTO 
void PaneAllowResizing(pane, allowtype)
pane_widget pane;
int allowtype;
#else
void PaneAllowResizing(pane_widget pane, int allowtype)
#endif
{
    ResizeMode(pane) = allowtype;
    if (!allowtype) TryToFillPane(pane);
}
 
 

#ifdef _NO_PROTO 
void PaneMakeViewable(subwidget)
Widget subwidget;
#else
void PaneMakeViewable(Widget subwidget)
#endif
{
    pane_widget pane = (pane_widget) subwidget->core.parent;
    Constraints cursubdata, subdata;
    Widget cursubwidget, curmullion, mullion;
    XWindowChanges changes;
    int	viewable, notviewable, position;
 
/*
 * The current subwidget is the one occuppying the same position in the 
 * PaneChildren Widget list. If not then we need to go down the list till we
 * find the one that is sharing this position
 */
    subdata = DataFromWidget(subwidget);
    for (position = subdata->position; position > 0; position--)
	{
	cursubwidget = PaneChildren(pane)[position - 1];
	cursubdata = DataFromWidget(cursubwidget);
	if (cursubdata->position == subdata->position)
	    break;
	};
 
/*
 * If this subwidget is the one currently displayed or is not in a shared pane
 * or it is not managed then don't bother doing any of this work.
 */
    if (subwidget == cursubwidget || !subwidget->core.managed || 
	    subdata->sharedflag == PaneMpbNotShared)
	return;
 
    viewable = PaneMpbShared | PaneMpbViewableInPane;
    notviewable = PaneMpbShared;
 
/*
 * Get the subdata of widget currently viewable and set the mullions
 */
    curmullion = cursubdata->mullion;
    mullion = subdata->mullion;
 
 
/*
 * Set the changes based on the current widget attributes
 */
    changes.x = cursubwidget->core.x;
    changes.y = cursubwidget->core.y;
    changes.width = cursubwidget->core.width;
    changes.height = cursubwidget->core.height;
    changes.border_width = 0;
    if (changes.x != subwidget->core.x || changes.y != subwidget->core.y ||
	  changes.width != subwidget->core.width ||
	  changes.height != subwidget->core.height ||
	  changes.border_width != subwidget->core.border_width) 
	{
	subwidget->core.x = changes.x;
	subwidget->core.y = changes.y;
	subwidget->core.width = changes.width;
	subwidget->core.height = changes.height;
	subwidget->core.border_width = changes.border_width;
	if (XtIsRealized(subwidget))    
	    {
	    XConfigureWindow(XtDisplay(subwidget), XtWindow(subwidget),
			     CWX |CWY | CWWidth | CWHeight | CWBorderWidth,
			     &changes);
	    }
	if (XtClass(subwidget)->core_class.resize != (XtWidgetProc) NULL)
	    {
	    (*(subwidget->core.widget_class->core_class.resize))(subwidget);
	    }
	}
 
/*
 * Set the current widgets and subwidget's shared pane attributes
 */
    cursubdata->sharedflag = notviewable;
    subdata->sharedflag = viewable;

/*
 * Set the min and the max values based on the currently visable widget
 */
    subdata->min = cursubdata->min;
    subdata->max = cursubdata->max;
 
/*
 * Unmap the current widget and map the subwidget by setting mapped_when_managed
 */
    XtSetMappedWhenManaged (subwidget, TRUE);
    XtSetMappedWhenManaged (cursubwidget, FALSE);
 
/*
 * If the cursubwidget was the last to have focus change it to the subwidget
 */
    if (LastHadFocus(pane) == cursubwidget)
	LastHadFocus(pane) = subwidget;
 
/*
 * Set the changes based on the current mullions attributes
 */
    changes.x = curmullion->core.x;
    changes.y = curmullion->core.y;
    if (changes.x != mullion->core.x || changes.y != mullion->core.y) 
	{
	mullion->core.x = changes.x;
	mullion->core.y = changes.y;
	if (XtIsRealized(subwidget))    
	    {
	    XConfigureWindow(XtDisplay(mullion), XtWindow(mullion),
			     CWX | CWY, &changes);
	    }
	}
/*
 * Unmap the current widget and map the subwidget by setting mapped_when_managed
 */
    XtSetMappedWhenManaged (mullion, TRUE);
    XtSetMappedWhenManaged (curmullion, FALSE);
 
    PaneChildren(pane)[position -  1] = subwidget;
    
}

#ifdef _NO_PROTO 
unsigned int PaneInitializeForMRM ()
#else
unsigned int PaneInitializeForMRM (void)
#endif
{
int	stat;
 
/*  Initialize MRM                                                            */
/*  This MUST be done before the XtInitialize call...                         */
/*                                                                            */
MrmInitialize ();
 
stat = MrmRegisterClass (MrmwcUnknown, "pane_widget",
	"PaneCreateWidget", PaneCreateWidget,  (WidgetClass) panewidgetclass);
 
if (stat != MrmSUCCESS)
	{
	printf ("pane widget registration failed\n");
	return stat;
	};
 
return stat;
}

#ifdef VMS
#define VMSReadDesc(source_desc,addr,len)	\
    LIB$ANALYZE_SDESC(source_desc,&len,&addr)

/*
**++
**  ROUTINE NAME: 
**	*VMSDescToNull (desc)
**
**  FUNCTIONAL DESCRIPTION:
**  	Converts a string descriptor to a null terminated string 
**
**  FORMAL PARAMETERS:
**	desc - the descriptor
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

char *VMSDescToNull (desc)
     struct dsc$descriptor_s *desc;
{
    char	  *nullterm_string;
    char	  *temp_string;
    unsigned short temp_length;
 
        if (desc->dsc$b_class <= DSC$K_CLASS_D) {
            temp_length = desc->dsc$w_length;
            temp_string = desc->dsc$a_pointer;
        }
        else
            VMSReadDesc(desc,temp_string,temp_length);

        nullterm_string = (char *) XtMalloc (temp_length+1);

        if (temp_length != 0)
            memcpy(temp_string,nullterm_string,temp_length); 

        *(nullterm_string+temp_length) = '\0';  /* Make it null-terminated */

        return (nullterm_string);
}
#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element PANE.C */
/*  *3    30-AUG-1989 15:59:18 BRINKLEY "const removed" */
/*  *2    29-AUG-1989 16:42:50 BRINKLEY "bug fixed" */
/*  *1    22-AUG-1989 16:55:37 RYAN "Initial elements for V3" */
/*  DEC/CMS REPLACEMENT HISTORY, Element PANE.C */
