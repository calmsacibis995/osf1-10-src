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
/*=======================================================================
*
*                    COPYRIGHT (c) 1988, 1989, 1992 BY
*              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
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
/*	This include file contains SVN specific information to be used by     */
/*	source module and composite widget developers.                        */
/*									      */
/*   AUTHORS:								      */
/*									      */
/*									      */
/*   CREATION DATE:							      */
/*									      */
/*   MODIFICATION HISTORY:						      */
/*									      */
/*	025	AN			20-Oct-1992			      */
/*		Add new field to SvnEventInfo structure to contain XEvent     */
/*	024	CS			13-Apr-1992			      */
/*		Change path_entries field from array to pointer.	      */
/*	023	CS			 2-Jan-1992			      */
/*		Added widget_list and widget_list_number fields		      */
/*	022	AN			11-Nov-1991			      */
/*		Add ptr_clips rectangle to handle exposes in PTR window.      */
/*	021	AN			25-Oct-1991			      */
/*		Added fields for new implementation of PTR window.  Now	      */
/*		SVN contains Form widgets and 2 window widgets for each side. */
/*		Also added misc fields for live scrolling rewrite.	      */
/*	020	AN			19-Mar-1991			      */
/*		Added DXmSvnIndex for subclassing off of SVN .		      */
/*	019	AN			26-Feb-1991			      */
/*		Add fields for live scrolling of path-to-root entries.	      */
/*      018	AN			25-Feb-1991			      */
/*		Add fields to support live scrolling in SVN.		      */
/*	017	AN			11-Dec-1990			      */
/*		Add highlight_gc and inverse_highlight_gc to svn_part.  Remove*/
/*		location_cursor_inverse_gc, now its same as location_cursor_gc.*/
/*	016	AN			20-Nov-1990			      */
/*		Add fields to instance struct. for using timeouts for extended*/
/*		selection code.						      */
/*	015	AN			20-Sep-1990			      */
/*		Add resource for new callback EntryTransfer, and more fields  */
/*		new Motif mouse semantics and location cursor		      */
/*	014	AN			23-Aug-1990			      */
/*		Add more fields in instance struct. for keyboard traversal and */
/*		location cursor.					      */
/*	013	AN			20-Aug-1990			      */
/*		Add new fields in instance struct. for keyboard traversal and */
/*		location cursor.					      */
/*	012	AN			11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	011	AN			05-Jul-1990			      */
/*		Prefix svnclass and svnrec defines with 'DXm'		      */
/*	010	AN			28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	009	AN			25-Jun-1990			      */
/*		Changes made so that only compound strings are supported      */
/*		instead of text strings and compound strings.		      */
/*	008	AN			13-Mar-1990			      */
/*		Add structure SvnPoint					      */
/*      007	WW			26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	006	WW			25-Jan-1990			      */
/*		Change DECwWsSvn*.h to DECwMSvn.h.			      */
/*	005	SL	 		23-Jan-1990			      */
/*		Change naming scheme of all constants from Svn* to SvnK*.     */
/*	004	WW			23-Jan-1990			      */
/*		Change SvnWidget to svn_widget.				      */
/*      003     SDL + WDW		18-Jan-1990                           */
/*		Change extern to externalref on widgetclass declarations.     */
/*	002	WW			15-Jan-1990			      */
/*		Perform post-motif-port modifications.			      */
/*	001	SL	 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

#ifndef _DXmSvnP_h
#define _DXmSvnP_h

#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:ManagerP.h>
#include <DECW$INCLUDE:DXmSvn.h>
#else
#include <Xm/ManagerP.h>
#include <DXm/DXmSvn.h>
#endif


/*----------------------------------*/
/* SVN widget part of class record  */
/*----------------------------------*/

typedef struct
    {                        
    XtPointer dummy;
    } DXmSvnClassPart;


/*----------------------------------*/
/* full SVN widget class record     */
/*----------------------------------*/

typedef struct _DXmSvnClassRec
    {
    CoreClassPart             core_class;
    CompositeClassPart	      composite_class;
    ConstraintClassPart	      constraint_class;
    XmManagerClassPart	      manager_class;
    DXmSvnClassPart	      Svn_class;
    } DXmSvnClassRec, *DXmSvnClass;


/*----------------------------------------*/
/* SVN widget private component structure */
/*----------------------------------------*/

typedef struct
    {
    union
    {
      struct {
        XmString	text;		/* Compound string 	    */
        XmFontList	font_list;	/* font list  */
        } is_text;

      struct {
        Pixmap          pixmap;          /* Pixmap structure             */
        } is_pixmap;

      struct {
        Widget          readwrite_text;  /* text widget identifier       */
        } is_widget;
    } var;
  
    Position        orig_x;          /* original relative to entry   */
    Position        orig_y;          /* original relative to entry   */
    Position        x;               /* relative to entry            */
    Position        y;               /* relative to entry            */

    Dimension       orig_width;      /* original width               */
    Dimension       orig_height;     /* original height              */
    Dimension       height;          /* height of component          */
    Dimension       width;           /* width of component           */

    short           orig_rj;         /* originally right justified   */
    short           type;            /* type of component            */
    short           hidden_mode;     /* displaymode to be hidden     */
    } DXmSvnCompStruct, *DXmSvnCompPtr;



/*-----------------------------------------*/
/* literal defining the max display length */
/*-----------------------------------------*/

#define max_comps    30
#define max_display  150
#define max_clips    25


/*-----------------------------------------*/
/* Pointer to the level array for SVN tree */
/* and default size of the array.          */
/*-----------------------------------------*/

#define DEFAULT_MAX_LEVEL 5
typedef int (*LevelPtr)[];


/*-----------------------------------------*/
/* Define component type literals.  We'll  */
/*  use the Dwt ones so that we can use    */
/*  some of their support routines.  We'll */
/*  define the widget one as 99 so that we */
/*  don't conflict with them.              */
/*-----------------------------------------*/

#define DXmSvnKcompNotSet   0
#define DXmSvnKcompText     1
#define DXmSvnKcompPixmap   2
#define DXmSvnKcompWidget   3


typedef struct
    {
    long int x, y;
    } DXmSvnPoint;

typedef struct
    {
    int		    type;			/* type of event */
    Position	    x;				/* window x coord. */
    Position	    y;				/* window y coord. */
    unsigned int    state;			/* button state */
    int		    entry_number;		/* Entry number event occured on */
    Time	    time;
    int		    compnm;			/* component number event occurred on */
    XEvent	    *event;			/* Pointer to XEvent structure */
    } SvnEventInfo;

/*----------------------------------------*/
/* SVN widget private entry structure     */
/*----------------------------------------*/

typedef struct entrynode
    {
    struct entrynode   * next;               /* pointer to next entry      */
    struct entrynode   * prev;               /* pointer to previous entry  */
    DXmSvnCompStruct      * entrycompPtr;       /* pointer to component       */
    XtPointer            entry_tag;          /* application controlled tag */
    int                  x;                  /* x position of entry        */
    int                  px;                 /* connection x pos to parent */
    int                  py;                 /* connection y pos to parent */
    Dimension            width;              /* the width of this entry    */
    Dimension            height;             /* the height of this entry   */
    Dimension            orig_height;        /* height specified in set    */
    short                num_allocated;      /* number of allocated comps  */
    short                num_components;     /* the number of components   */
    short                level;              /* the entries level number   */
    short                selected_comp;      /* component selected         */
    Boolean              valid;              /* in synch with the source   */
    Boolean              height_adjusted;    /* added top and bottom       */
    Boolean              selected;           /* the entry is selected      */
    Boolean              highlighted;        /* the entry is highlighted   */
    Boolean              grayed;             /* the entry is unselectable  */
    Boolean              index_window;       /* show in index window       */
    } DXmSvnEntryStruct, *DXmSvnEntryPtr;


/*----------------------------------------*/
/* SVN widget part of instance record     */
/*----------------------------------------*/

/*
**  In order to run on MIPS with minimal fixup for natural boundary access, the
**  fields are ordered according to size.  The biggest ones are first, followed
**  by smaller and smaller fields.
*/

typedef struct
    {
/*
**  The following are structures
*/
     int		default_font;		      /* default font id */
     XmFontList		default_fontlist;             /* XtExtDefaultFont    */
     XmFontList		level0_fontlist;              /* DXmSvnNfontListLevel0  */
     XmFontList		level1_fontlist;              /* DXmSvnNfontListLevel1  */
     XmFontList		level2_fontlist;              /* DXmSvnNfontListLevel2  */
     XmFontList		level3_fontlist;              /* DXmSvnNfontListLevel3  */
     XmFontList		level4_fontlist;              /* DXmSvnNfontListLevel4  */
     XtCallbackList   	SelectAndConfirm_callback;    /* DXmSvnNselectAndConfir.*/
     XtCallbackList   	ExtendConfirm_callback;       /* DXmSvnNextendConfirmCa.*/
     XtCallbackList   	EntrySelected_callback;       /* DXmSvnNentrySelectedCa.*/
     XtCallbackList   	EntryUnselected_callback;     /* DXmSvnNentryUnselected */
     XtCallbackList   	TransitionsDone_callback;     /* DXmSvnNtransitionsDone */
     XtCallbackList   	DisplayChanged_callback;      /* DXmSvnNdisplayChangedC */
     XtCallbackList   	AttachToSource_callback;      /* DXmSvnNattachToSourceC.*/
     XtCallbackList   	DetachFromSource_callback;    /* DXmSvnNdetachFromSourc.*/
     XtCallbackList   	SelectionsDragged_callback;   /* DXmSvnNselectionsDragg.*/
     XtCallbackList   	GetEntry_callback;            /* DXmSvnNgetEntryCallback*/
     XtCallbackList   	Dragging_callback;            /* DXmSvnNdraggingCallback*/
     XtCallbackList   	DraggingEnd_callback;         /* DXmSvnNdraggingEndCall.*/
     XtCallbackList   	unused_callback;	      /* not currently used */
     XtCallbackList   	Help_callback;                /* DwtNhelpRequested   */
     XtCallbackList	PopupMenu_callback;	      /* DXmSvnNpopupMenuCallback */
     XtCallbackList	EntryTransfer_callback;	      /* DXmSvnNentryTransferCallback */
     XRectangle         clips [max_clips];            /* Clip rectangles     */

/*
**  The following are declared as an XID which is an unsigned long - 4 bytes
*/
     Window              clips_window;                 /* window for clips    */
     Cursor              watch_cursor;                 /* Created once        */
     Cursor              nav_box_cursor;               /* Created once        */
     Pixmap              grey_pixmap;                  /* graphic contexts    */
     Pixmap              top_pixmap;                   /*                     */
     Pixmap              bot_pixmap;                   /*                     */
     Pixmap              ghost;                        /*                     */
     Pixmap              tree_nav_open_pixmap;         /* nav button pixmap   */
     Pixmap              tree_nav_close_pixmap;        /* nav button pixmap   */
     Pixmap              outline_nav_pixmap;           /* nav button pixmap   */
     Pixmap              tree_highlight_pixmap;        /* nav button pixmap   */
/*
**  These are declared as unsigned long's which are 4 bytes
*/
     Pixel               foreground_pixel;             /* XmNforeground      */
     XtIntervalId        timerid;                      /* drag/double click   */
     XtIntervalId        button_timerid;               /* id for timer        */
     Time                button_up_event_time;
/*
**  pointers are 4 bytes
*/
     GC                  inverse_gc;                   /* graphic contexts    */
     GC                  background_gc;                /* graphic contexts    */
     GC                  foreground_gc;                /* graphic contexts    */
     GC                  grey_gc;                      /* graphic contexts    */
     GC                  drag_gc;                      /* graphic contexts    */
     GC                  tree_highlight_gc;            /* GC for wide lines   */
     GC                  nav_box_gc;                   /* GC for nav box      */
     GC			 copyarea_gc;		       /* GC used for  XCopyArea's */
     Widget              index_window;                 /* widget identifier   */
     Widget              vscroll;                      /* DXmSvn scroll bars     */
     Widget              hscroll;                      /* DXmSvnNoutlineHScrollWi*/
     Widget              secondary_hscroll;            /*                     */
     Widget              top_button;                   /*                     */
     Widget              bot_button;                   /*                     */
     Widget              nav_button;                   /* push button for nav */
     Widget		 nav_window_popup;	       /* nav window popup    */
     Widget		 nav_window;		       /* nav window widget   */
     Widget              pane_widget;                  /* DXmSvnNpaneWidget      */
     Widget              primary_window_widget;            /* DXmSvnNprimaryWindowWidget */
     Widget              secondary_window_widget;            /* DXmSvnNsecondaryWindowWidget */
     Widget		 primary_ptr_widget;
     Widget		 secondary_ptr_widget;
     Widget		 primary_form;
     Widget		 secondary_form;
     Widget		 primary_separator_widget;
     Widget		 secondary_separator_widget;
     XmString		 nav_window_title;	       /* DXmSvnNnavWindowTitle  */
     DXmSvnEntryPtr	 current_entry;		       /* entry to keep place */
     DXmSvnEntryPtr         entryPtr;                     /* pointer to entry    */
     DXmSvnEntryPtr         cache_pointer;                /* pointer cache       */
     LevelPtr		 levelx;		       /* ptr max x values    */
     LevelPtr		 levely;		       /* ptr y of each level */
     char               *user_data;                    /* XmNuserData        */
/*
**  int's are 4 bytes
*/
     int                 num_entries;                  /* DXmSvnNnumberOfEntries */
     int                 num_selections;               /* number of selections*/
     int                 num_highlighted;              /* number of hi-lighted*/
     int                 cache_number;                 /* entry in cache      */
     int                 range_hook;                   /* entry in range      */
     int                 button_top;                   /* top or bottom       */
     int                 button_waitms;                /* ms before start     */
     int                 button_repeatms;              /* ms before repeat    */
     int		 mapx, mapy;		       /* world coord to window convert */
     int		 tree_width;		       /* width of tree */
     int		 tree_height;		       /* height of tree */
     int		 prevx, prevy;		       /* previous location of current_entry */
     int		 range_hook_x;		       /* start point of range select */
     int		 range_hook_y;		       /* start point of range select */
     int		 nav_window_box_x;	       /* position of current */
     int		 nav_window_box_y;	       /* display box in nav window */
     int		 nav_window_box_height;
     int		 nav_window_box_width;
     int		 current_entry_number;	       /* entry number for current entry */
     int		 vscroll_in_progress;	       
     int		 hscroll_in_progress;	       
     int		 map_level;
     int		 primary_percentage;	       /* save/restore column */
     XtPointer		 column_tags [max_comps];

/*
**   Dimension's are ShortCard's which are unsigned ints which are 2 bytes
*/
     Dimension           column_widths [max_comps];   
     Dimension           margin_width;                 /* XmNmarginWidth     */
     Dimension           margin_height;                /* XmNmarginHeight    */
     Dimension           indent_margin;                /* DXmSvnNindentMargin    */
     Dimension           default_spacing;              /* DXmSvnNdefaultSpacing  */
     Dimension           level_spacing;		       /* DXmSvnNtreeLevelSpacing*/
     Dimension           sibling_spacing;	       /* DXmSvnNtreeSiblingSpac */
     Dimension           arc_width;		       /* DXmSvnNtreeArcWidth    */
     Dimension           max_width;                    /* needed for horiz scr*/
     Dimension           secondary_max_width;          /* needed for horiz scr*/
     Dimension           ghost_width;                  /*                     */
     Dimension           ghost_height;                 /*                     */
     Dimension           display_invalid[max_display]; /* zero or old_height  */

/*
**   Position's are short's which are 2 bytes
*/
     Position            window_basex;                 /* Horizontal scrolling*/
     Position            box_base_x;                   /* range box           */
     Position            box_base_y;                   /*                     */
     Position            box_other_x;                  /*                     */
     Position            box_other_y;                  /*                     */
     Position            ghost_x;                      /*                     */
     Position            ghost_y;                      /*                     */
     Position            ghost_basex;                  /*                     */
     Position            ghost_basey;                  /*                     */
     Position            display_line_y;               /* y position of line  */
     Position            secondary_base_x;	       /* DXmSvnNsecondaryBaseX        */

/*
**   short's are 2 bytes
*/
     short               display_mode;                 /* DXmSvnNdisplayMode     */
     short               tree_style;	               /* DXmSvnNtreeStyle	      */
     short               start_column_component;       /* DXmSvnNstartColumnCompo*/
     short               selection_mode;               /* DXmSvnNselectionMode   */
     short               disabled_count;               /* counter             */
     short               index_window_shown;           /* entry number        */
     short               button_mode;                  /* used in DXmSvn module  */
     short               clip_count;                   /* used rectangles     */
     short               timer_entry;                  /*                     */
     short               num_path;                     /* entries in path     */
     short               entries[max_display];         /*                     */
     short               display_count;                /* count of displayed  */
     short               max_level;                    /* number of levels in level array */

/*
**   Boolean's are char's which are 1 byte
*/
     Boolean             column_width_set [max_comps]; /* Auto adjust column  */
     Boolean             column_lines;  	       /* DXmSvnNcolumnLines     */
     Boolean		 fixed_width;                  /* DXmSvnNfixedWidthEntrie*/
     Boolean             multiple_selections;          /* DXmSvnNmultipleSelecti.*/
     Boolean             use_scroll_buttons;           /* DXmSvnNuseScrollButtons*/
     Boolean             expect_highlighting;          /* DXmSvnNexpectHighlighti*/
     Boolean             force_seq_get_entry;          /* DXmSvnNforceSeqGetEntry*/
     Boolean             show_path_to_root;            /* DXmSvnNshowPathToRoot  */
     Boolean		 centered_components;	       /* DXmSvnNtreeCenteredComp*/
     Boolean		 perpendicular_lines;	       /* DXmSvnNtreePerpendicula*/
     Boolean		 index_all;		       /* DXmSvnNtreeIndexAll    */
     Boolean		 entry_shadows;		       /* DXmSvnNtreeEntryShadows*/
     Boolean		 tree_entry_outlines;	       /* DXmSvnNtreeEntryOutline*/
     Boolean		 truncate_strings;	       /* DXmSvnNtruncateStrings */
     Boolean             secondary_components_unmapped;/* DXmSvnNsecondaryComponentsUnm*/
     Boolean             index_window_needed;          /* true or false       */
     Boolean             sub_widgets_used;             /* sub widgets are used*/
     Boolean             show_selections;              /* For global selection*/
     Boolean             show_highlighting;            /*                     */
     Boolean             ghosting;                     /* ghosting            */
     Boolean             display_changed;              /* accuracy of screen  */
     Boolean             transitions_made;
     Boolean	         refresh_all;		       /* display needs to be redrawn */
     Boolean             button_down;                  /* button autorepeat   */
     Boolean		 update_in_progress;	       /* flag that screen update is in progress */
     Boolean		 tree_connections;	       /* draw connections? */
     Boolean		 update_nav_window;	       /* Nav window needs to be redrawn */
     Boolean		 unused_boolean;	       /* unused for now */
/*
** Fields needed for new mouse and keyboard semantics...
*/
     DXmSvnEntryPtr	 location_cursor_entry;		/* Entry that contains the location cursor */
     GC			 location_cursor_gc;		/* GC for location cursor */
     short		 location_cursor;		/* current entry number that has location number */
     short		 anchor_entry;			/* Entry that has the anchor ... for a extended click */
     SvnEventInfo	 last_event;			/* Info on last event that occurred... */
     short		 last_selected_entry;		/* Last entry that was selected during extended selection drag ...*/
     short		 loc_cur_prev_selected;		/* Flag whether loc. cursor was selected previously */
     short		 drag_state;			/* Signals whether we are doing selection or a toggle drag */
     int		 leave_direction;
     XtIntervalId	 drag_id;	
     GC			 inverse_highlight_gc;		/* GC for selected - highlighted entry */
     GC			 highlight_gc;			/* GC for selected - highlighted entry */

/* Live scrolling fields */

     Boolean	         grop_pending;			/* Graphics exposure operation pending */
     int		 internal_x;
     int		 internal_y;
     int		 scroll_x;
     int		 scroll_y;
     int		 basex;				
     int		 basey;
     int		 internal_value;		/* New scrollbar callback value */
     int		 scroll_value;			/* Current scrollbar     callback value */
     int		 current_value;
     short		 up_entries[max_display];
     short		 *path_entries;
     short		 ls_num_path;
     Boolean		 live_scrolling;		/* DXmSvnNliveScrolling   */
     XRectangle          ptr_clips [max_clips];		/* Clip rectangles     */
     short               ptr_clip_count;		/* used PTR rectangles     */
     Window              ptr_clips_window;              /* PTR window for clips    */
     WidgetList		 widget_list;			/* Used for manipulating widgets */
     int		 widget_list_number;		/* Contains the number of widget list elements */
    } SvnPart;


typedef struct _DXmSvnRec                  /* SVN full instance record */
    {
    CorePart              core;
    CompositePart         composite;
    ConstraintPart	  constraint;
    XmManagerPart	  manager;
    SvnPart               svn;
    } DXmSvnRec, *svn_widget;

externalref DXmSvnClassRec       dxmSvnClassRec;
externalref DXmSvnClass          dxmSvnWidgetClass;

#define DXmSvnIndex (XmManagerIndex+1)

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _DXmSvnP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
