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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: WmGlobal.h,v $ $Revision: 1.1.4.5 $ $Date: 1993/10/18 17:19:35 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifndef NO_SHAPE
#ifdef VMS
#include <X11/shape.h>
#else
#include <X11/extensions/shape.h>
#endif /* VMS */
#endif /* NO_SHAPE  */
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/MwmUtil.h>
#ifdef AUTOMATION
#include <X11/Xfuncs.h>
#include <AutoMwm.h>
#endif /* AUTOMATION */
#include <Xm/Xm.h>

/*
 * Value definitions and macros:
 */

#ifdef MOTIF_ONE_DOT_ONE
#define XmFONTLIST_DEFAULT_TAG	"XmSTRING_DEFAULT_CHARSET"
#endif

/* window manager name and class used to get resources: */
#define WM_RESOURCE_NAME	"mwm"
#define	WM_RESOURCE_CLASS	"Mwm"
/* ICCC atom names: */

#define _XA_WM_STATE		"WM_STATE"
#define _XA_WM_PROTOCOLS	"WM_PROTOCOLS"
#define _XA_WM_CHANGE_STATE	"WM_CHANGE_STATE"
#define _XA_WM_SAVE_YOURSELF	"WM_SAVE_YOURSELF"
#define _XA_WM_DELETE_WINDOW	"WM_DELETE_WINDOW"
#define _XA_WM_TAKE_FOCUS	"WM_TAKE_FOCUS"
#define _XA_WM_COLORMAP_WINDOWS	"WM_COLORMAP_WINDOWS"

/* window manager exit value on fatal errors: */
#define WM_ERROR_EXIT_VALUE	1

/* built-in button bindings for window manager actions: */
#define SELECT_BUTTON			Button1
#define SELECT_BUTTON_MASK		Button1Mask
#define SELECT_BUTTON_MOTION_MASK	Button1MotionMask

#define FOCUS_SELECT_BUTTON	SELECT_BUTTON
#define FOCUS_SELECT_MODIFIERS	0

/* direct manipulation button */
#define DMANIP_BUTTON			Button2
#define DMANIP_BUTTON_MASK		Button2Mask
#define DMANIP_BUTTON_MOTION_MASK	Button2MotionMask

/* menu button */
#define BMENU_BUTTON			Button3
#define BMENU_BUTTON_MASK		Button3Mask

/* Needed by PostMenu() to specify key post: */
#define NoButton		0

/* manage window flags: */
#define MANAGEW_WM_STARTUP	(1L << 0)
#define MANAGEW_WM_RESTART	(1L << 1)
#define MANAGEW_NORMAL		(1L << 2)
#define MANAGEW_ICON_BOX	(1L << 3)
#define MANAGEW_CONFIRM_BOX	(1L << 4)
#define MANAGEW_WM_RESTART_ICON	(1L << 5)
#ifdef DEC_MOTIF_BUG_FIX
/* Remanage a window after interplace failed */
#define MANAGEW_RETRY           (1L << 15)
#endif

#define MANAGEW_WM_CLIENTS	(MANAGEW_ICON_BOX | MANAGEW_CONFIRM_BOX)

/* keyboard input focus flag values (for calls to SetKeyboardFocus) */
#define ALWAYS_SET_FOCUS	(1L << 0)
#define REFRESH_LAST_FOCUS	(1L << 1)
#define CLIENT_AREA_FOCUS	(1L << 2)
#define SCREEN_SWITCH_FOCUS	(1L << 3)
/* special value for use for Do_Focus_Key to set to internal window */
#define WORKSPACE_IF_NULL	(1L << 4)
#ifdef DEC_MOTIF_BUG_FIX
/* used to ignore colormap set on a FocusOut event */
#define IGNORE_COLORMAP_FOCUS   (1L << 5)
#endif

/* Menu posting flag values (for calls to PostMenu) */
#define POST_AT_XY		(1L << 0)
#define POST_TRAVERSAL_ON	(1L << 1)
#define POST_STICKY		(1L << 2)

/* feedback box styles */
#define FB_OFF			(0)
#define FB_SIZE			(1L << 0)
#define FB_POSITION		(1L << 1)

/* confirmbox and waitbox indexes */
#define DEFAULT_BEHAVIOR_ACTION		0
#define CUSTOM_BEHAVIOR_ACTION		1
#define RESTART_ACTION		2
#define QUIT_MWM_ACTION		3

/* extract text height in pixels from a (XFontStruct *) */
#define TEXT_HEIGHT(pfs) (((pfs)->ascent)+((pfs)->descent))

/* icon frame shadow widths */
#define ICON_EXTERNAL_SHADOW_WIDTH	2
#define ICON_INTERNAL_SHADOW_WIDTH	1

/* padding widths */
#define ICON_IMAGE_TOP_PAD	2
#define ICON_IMAGE_BOTTOM_PAD	2
#define ICON_IMAGE_LEFT_PAD	2
#define ICON_IMAGE_RIGHT_PAD	2

/* image offsets */
#define ICON_INNER_X_OFFSET	\
	    (ICON_IMAGE_LEFT_PAD+ICON_EXTERNAL_SHADOW_WIDTH)
#define ICON_INNER_Y_OFFSET	\
	    (ICON_IMAGE_TOP_PAD+ICON_EXTERNAL_SHADOW_WIDTH)


#define ICON_IMAGE_X_OFFSET ICON_INNER_X_OFFSET+ICON_INTERNAL_SHADOW_WIDTH
#define ICON_IMAGE_Y_OFFSET ICON_INNER_Y_OFFSET+ICON_INTERNAL_SHADOW_WIDTH



/* number of rectangles to allocate */
#define NUM_IMAGE_TOP_RECTS	\
	    ((2*ICON_EXTERNAL_SHADOW_WIDTH)+(2*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_IMAGE_BOTTOM_RECTS	\
	    ((2*ICON_EXTERNAL_SHADOW_WIDTH)+(2*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_LABEL_TOP_RECTS	(2*ICON_EXTERNAL_SHADOW_WIDTH)
#define NUM_LABEL_BOTTOM_RECTS	(2*ICON_EXTERNAL_SHADOW_WIDTH)

#define NUM_BOTH_TOP_RECTS	\
	    ((3*ICON_EXTERNAL_SHADOW_WIDTH)+(3*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_BOTH_BOTTOM_RECTS	\
	    ((3*ICON_EXTERNAL_SHADOW_WIDTH)+(3*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_STATIC_TOP_RECTS	(2*ICON_INTERNAL_SHADOW_WIDTH)
#define NUM_STATIC_BOTTOM_RECTS	(2*ICON_INTERNAL_SHADOW_WIDTH)


/* client frame shadow widths */
#define FRAME_EXTERNAL_SHADOW_WIDTH	2
#define FRAME_INTERNAL_SHADOW_WIDTH	1
#define FRAME_CLIENT_SHADOW_WIDTH	1
#define FRAME_MATTE_SHADOW_WIDTH	1

/* padding around text in title bar */
#define WM_TOP_TITLE_PADDING	1
#define WM_BOTTOM_TITLE_PADDING	1
#define WM_TOP_TITLE_SHADOW	FRAME_INTERNAL_SHADOW_WIDTH
#define WM_BOTTOM_TITLE_SHADOW	FRAME_INTERNAL_SHADOW_WIDTH

#define WM_TITLE_BAR_PADDING	(WM_TOP_TITLE_PADDING \
				 +WM_BOTTOM_TITLE_PADDING \
				 +WM_TOP_TITLE_SHADOW \
				 +WM_BOTTOM_TITLE_SHADOW)

/* stretch directions  - (starts at NW and goes clockwise) */
#define STRETCH_NO_DIRECTION	-1
#define STRETCH_NORTH_WEST	0
#define STRETCH_NORTH		1
#define STRETCH_NORTH_EAST	2
#define STRETCH_EAST		3
#define STRETCH_SOUTH_EAST	4 
#define STRETCH_SOUTH 		5
#define STRETCH_SOUTH_WEST	6
#define STRETCH_WEST		7

#define STRETCH_COUNT		8


/* function flag masks */
#define WM_FUNC_DEFAULT		MWM_FUNC_ALL
#define WM_FUNC_NONE		0
#define WM_FUNC_ALL		(MWM_FUNC_RESIZE | MWM_FUNC_MOVE |\
				 MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE |\
				 MWM_FUNC_CLOSE)

/* decorations flag masks */
#define WM_DECOR_DEFAULT	MWM_DECOR_ALL
#define WM_DECOR_NONE		0
#define WM_DECOR_BORDER		(MWM_DECOR_BORDER)
#define WM_DECOR_TITLE		(MWM_DECOR_TITLE)
#define WM_DECOR_SYSTEM		(WM_DECOR_TITLE | MWM_DECOR_MENU)
#define WM_DECOR_MINIMIZE	(WM_DECOR_TITLE | MWM_DECOR_MINIMIZE)
#define WM_DECOR_MAXIMIZE	(WM_DECOR_TITLE | MWM_DECOR_MAXIMIZE)
#define WM_DECOR_TITLEBAR	(WM_DECOR_SYSTEM | WM_DECOR_MINIMIZE |\
				 WM_DECOR_MAXIMIZE)  
#define WM_DECOR_RESIZEH	(WM_DECOR_BORDER | MWM_DECOR_RESIZEH)
#define WM_DECOR_RESIZE		(WM_DECOR_RESIZEH)
#define WM_DECOR_ALL		(WM_DECOR_TITLEBAR | WM_DECOR_RESIZEH)


/* icon box definitions */
#define ICON_BOX_FUNCTIONS	(MWM_FUNC_RESIZE | MWM_FUNC_MOVE |\
				 MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE)


/* show feedback definitions */
#define WM_SHOW_FB_BEHAVIOR	(1L << 0)
#define WM_SHOW_FB_MOVE		(1L << 1)
#define WM_SHOW_FB_PLACEMENT	(1L << 2)
#define WM_SHOW_FB_RESIZE	(1L << 3)
#define WM_SHOW_FB_RESTART	(1L << 4)
#define WM_SHOW_FB_QUIT         (1L << 5)
#define WM_SHOW_FB_KILL         (1L << 6)

#define WM_SHOW_FB_ALL		(WM_SHOW_FB_BEHAVIOR  | WM_SHOW_FB_MOVE    |\
				 WM_SHOW_FB_PLACEMENT | WM_SHOW_FB_RESIZE  |\
				 WM_SHOW_FB_RESTART   | WM_SHOW_FB_QUIT    |\
				 WM_SHOW_FB_KILL)

#define WM_SHOW_FB_NONE		0

#define WM_SHOW_FB_DEFAULT	WM_SHOW_FB_ALL



/*************************************<->*************************************
 *
 *  Miscellaneous utility window manager data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are used in resource processing, ...
 *
 *************************************<->***********************************/

typedef struct _WHSize
{
    int		width;
    int		height;

} WHSize;


typedef struct _AspectRatio
{
    int		x;
    int		y;

} AspectRatio;


typedef struct _WmColorData
{
    Screen *screen;
    Colormap colormap;
    Pixel background;
    Pixel foreground;
    Pixel top_shadow;
    Pixel bottom_shadow;

} WmColorData;



/*************************************<->*************************************
 *
 *  Event processing data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are used in saving button and key
 *  specifications that are used in processing events that are used to do 
 *  window manager functions (e.g., set the colormap focus).
 *
 *************************************<->***********************************/

typedef unsigned long Context;
typedef unsigned long Behavior;
typedef unsigned long GroupArg;

typedef Boolean (*WmFunction) ();

#define NO_MODIFIER	0		/* value for state field */

typedef struct _KeySpec
{
    unsigned int state;
    KeyCode      keycode;
    Context	 context;
    Context	 subContext;
    WmFunction	 wmFunction;
    String	 wmFuncArgs;
    struct _KeySpec *nextKeySpec;

} KeySpec;

typedef struct _ButtonSpec
{
    unsigned int state;
    unsigned int button;
    unsigned int eventType;
    Boolean	click;
    Context	context;
    Context	subContext;
    WmFunction	wmFunction;
    String	wmFuncArgs;
    struct _ButtonSpec *nextButtonSpec;

} ButtonSpec;


/*
 * Context field values:
 */

#define F_CONTEXT_NONE		0
#define F_CONTEXT_ROOT		(1L << 0)
#define F_CONTEXT_ICON		(1L << 1)
#define F_CONTEXT_NORMAL	(1L << 2)
#define F_CONTEXT_MAXIMIZE	(1L << 3)
#define F_CONTEXT_ICONBOX	(1L << 4)
#define F_CONTEXT_WINDOW	(F_CONTEXT_NORMAL|F_CONTEXT_MAXIMIZE)
#define F_CONTEXT_ALL		(F_CONTEXT_ROOT|F_CONTEXT_ICON|F_CONTEXT_WINDOW)


/*
 * context field mark for catching menu recursion 
 *   (tied to F_CONTEXT_... values):
 */

#define CR_MENU_MARK		(1L << 5)


/*
 * Part context defines for event processing.  The part context is used
 * to make a subcontext mask.
 */

/* window (frame and client) part contexts */
#define WINDOW_PART_NONE	0
#define FRAME_NONE		WINDOW_PART_NONE
#define FRAME_CLIENT		1
#define FRAME_SYSTEM		2
#define FRAME_TITLE		3
#define FRAME_MINIMIZE		4
#define FRAME_MAXIMIZE		5
#define FRAME_RESIZE_NW		6
#define FRAME_RESIZE_N		7
#define FRAME_RESIZE_NE		8
#define FRAME_RESIZE_E		9
#define FRAME_RESIZE_SE		10
#define FRAME_RESIZE_S 		11
#define FRAME_RESIZE_SW		12
#define FRAME_RESIZE_W		13
#define FRAME_NBORDER		14
#define FRAME_MATTE		15
#define FRAME_MISC		FRAME_MATTE

/* icon part contexts */
#define ICON_PART_NONE		0
#define ICON_PART_ALL		16

/* root part contexts */
#define ROOT_PART_NONE		0
#define ROOT_PART_ALL		17

/* iconbox part contexts */
#define ICONBOX_PART_NONE	0
#define ICONBOX_PART_IBOX	18
#define ICONBOX_PART_IICON	19
#define ICONBOX_PART_WICON	20


/*
 * Subcontext field values:
 */

#define F_SUBCONTEXT_NONE		(1L << WINDOW_PART_NONE)

#define F_SUBCONTEXT_I_ALL		(1L << ICON_PART_ALL)

#define F_SUBCONTEXT_R_ALL		(1L << ROOT_PART_ALL)


#define F_SUBCONTEXT_IB_IBOX		(1L << ICONBOX_PART_IBOX)
#define F_SUBCONTEXT_IB_IICON		(1L << ICONBOX_PART_IICON)
#define F_SUBCONTEXT_IB_WICON		(1L << ICONBOX_PART_WICON)

#define F_SUBCONTEXT_IB_ICONS		(F_SUBCONTEXT_IB_IICON |\
					 F_SUBCONTEXT_IB_WICON)

#define F_SUBCONTEXT_IB_ALL		(F_SUBCONTEXT_IB_IBOX |\
					 F_SUBCONTEXT_IB_IICON |\
					 F_SUBCONTEXT_IB_WICON)


#define F_SUBCONTEXT_W_CLIENT		(1L << FRAME_CLIENT)
#define F_SUBCONTEXT_W_APP		F_SUBCONTEXT_W_CLIENT
#define F_SUBCONTEXT_W_SYSTEM		(1L << FRAME_SYSTEM)
#define F_SUBCONTEXT_W_TITLE		(1L << FRAME_TITLE)
#define F_SUBCONTEXT_W_MINIMIZE		(1L << FRAME_MINIMIZE)
#define F_SUBCONTEXT_W_MAXIMIZE		(1L << FRAME_MAXIMIZE)
#define F_SUBCONTEXT_W_RESIZE_NW	(1L << FRAME_RESIZE_NW)
#define F_SUBCONTEXT_W_RESIZE_N		(1L << FRAME_RESIZE_N)
#define F_SUBCONTEXT_W_RESIZE_NE	(1L << FRAME_RESIZE_NE)
#define F_SUBCONTEXT_W_RESIZE_E		(1L << FRAME_RESIZE_E)
#define F_SUBCONTEXT_W_RESIZE_SE	(1L << FRAME_RESIZE_SE)
#define F_SUBCONTEXT_W_RESIZE_S		(1L << FRAME_RESIZE_S)
#define F_SUBCONTEXT_W_RESIZE_SW	(1L << FRAME_RESIZE_SW)
#define F_SUBCONTEXT_W_RESIZE_W		(1L << FRAME_RESIZE_W)
#define F_SUBCONTEXT_W_NBORDER		(1L << FRAME_NBORDER)
#define F_SUBCONTEXT_W_MATTE		(1L << FRAME_MATTE)
#define F_SUBCONTEXT_W_MISC		F_SUBCONTEXT_W_MATTE


#define F_SUBCONTEXT_W_RBORDER		(F_SUBCONTEXT_W_RESIZE_NW |\
					 F_SUBCONTEXT_W_RESIZE_N |\
					 F_SUBCONTEXT_W_RESIZE_NE |\
					 F_SUBCONTEXT_W_RESIZE_E |\
					 F_SUBCONTEXT_W_RESIZE_SE |\
					 F_SUBCONTEXT_W_RESIZE_S |\
					 F_SUBCONTEXT_W_RESIZE_SW |\
					 F_SUBCONTEXT_W_RESIZE_W)

#define F_SUBCONTEXT_W_BORDER		(F_SUBCONTEXT_W_RBORDER |\
					 F_SUBCONTEXT_W_NBORDER)

#define F_SUBCONTEXT_W_TITLEBAR		(F_SUBCONTEXT_W_SYSTEM |\
					 F_SUBCONTEXT_W_TITLE |\
					 F_SUBCONTEXT_W_MINIMIZE |\
					 F_SUBCONTEXT_W_MAXIMIZE)

#define F_SUBCONTEXT_W_FRAME		(F_SUBCONTEXT_W_BORDER |\
					 F_SUBCONTEXT_W_TITLEBAR)

#define F_SUBCONTEXT_W_ALL		(F_SUBCONTEXT_W_FRAME |\
					 F_SUBCONTEXT_W_MATTE |\
					 F_SUBCONTEXT_W_CLIENT)


/*
 * Click / double-click processing data:
 */

typedef struct _ClickData
{
    Boolean	clickPending;
    Boolean	doubleClickPending;
    unsigned int button;
    unsigned int state;
    unsigned int releaseState;
    struct _ClientData *pCD;
    Context	context;
    Context	subContext;
    Context	clickContext;
    Context	doubleClickContext;
    Time	time;

} ClickData;


/*
 * Frame part identification aids:
 */

typedef struct _Gadget_Rectangle
{
    short	id;
    XRectangle  rect;

} GadgetRectangle;


/*
 * Behavior function argument field values:
 */

#define F_BEHAVIOR_DEFAULT	(1L << 0)
#define F_BEHAVIOR_CUSTOM	(1L << 1)
#define F_BEHAVIOR_SWITCH	(1L << 2)


/*
 * Window/icon group function argument field values:
 */

#define F_GROUP_WINDOW		(1L << 0)
#define F_GROUP_ICON		(1L << 1)
#define F_GROUP_DEFAULT		(F_GROUP_WINDOW | F_GROUP_ICON)
#define F_GROUP_TRANSIENT	(1L << 2)
#define F_GROUP_ALL		(F_GROUP_DEFAULT | F_GROUP_TRANSIENT)
#define F_GROUP_GROUP		(1L << 3)



/*************************************<->*************************************
 *
 *  Menu specification data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are used in creating window manager menus that
 *  are specified using resource files.  A list of menu specifications
 *  (MenuSpec) is made when the resource files are parsed.  The desktop
 *  menu and system menus are created as needed using the menu specification
 *  list.
 *
 *************************************<->***********************************/

typedef struct _MenuItem
{
    int		 labelType;
    String	 label;
    int          labelBitmapIndex;
    KeySym	 mnemonic;
    unsigned int accelState;
    KeyCode      accelKeyCode;
    String	 accelText;
    WmFunction	 wmFunction;
    String	 wmFuncArgs;
    Context	 greyedContext;
    long         mgtMask;
    struct _MenuItem *nextMenuItem;

} MenuItem;


typedef struct _MenuButton
{
    MenuItem	*menuItem;
    Widget	buttonWidget;
    Boolean     managed;

} MenuButton;


typedef struct _MenuSpec
{
    String	  name;
    Context	  currentContext;
    Widget	  menuWidget;      /* RowColumn widget */
    unsigned int  whichButton;    /* tracks whichButton resource for top menu */
    unsigned int  height;          /* height of top menu */
    MenuItem	 *menuItems;       /* linked list of MenuItem structures */
    MenuButton   *menuButtons;     /* array of MenuButton structures */
    unsigned int  menuButtonSize;  /* size of menuButtons array */
    unsigned int  menuButtonCount; /* number of menuButtons elements in use */
    Context	  accelContext;    /* accelerator context */
    KeySpec	 *accelKeySpecs;   /* list of accelerator KeySpecs */
    Boolean       menuFilled;
    struct _MenuSpec *nextMenuSpec;

} MenuSpec;


/*************************************<->*************************************
 *
 *  Window and function specification data structures ...
 *
 *
 *  Description:
 *  -----------
 *
 *************************************<->***********************************/

typedef struct _WindowItem
{
    String              window;
    struct _WindowItem *nextWindowItem;

} WindowItem;

typedef struct _WindowSet
{
    String             name;
    WindowItem        *windowItems;
    struct _WindowSet *nextWindowSet;

} WindowSet;

typedef struct _FunctionItem
{
    WmFunction		  wmFunction;
    String                wmFuncArgs;
    struct _FunctionItem *nextFunctionItem;

} FunctionItem;

typedef struct _FunctionSet
{
    String               name;
    FunctionItem        *functionItems;
    struct _FunctionSet *nextFunctionSet;

} FunctionSet;


/*************************************<->*************************************
 *
 *  Window manager timer data structure ...
 *
 *
 *  Description:
 *  -----------
 *  This data stucture is used to keep track of window manager timers.  Each
 *  active timer has an instance of the data structure.
 *
 *************************************<->***********************************/

typedef struct _WmTimer
{
    XtIntervalId	timerId;
    struct _ClientData	*timerCD;
    unsigned int	timerType;
    struct _WmTimer	*nextWmTimer;

} WmTimer;

/* Timer types: */
#define TIMER_NONE		0
#define TIMER_QUIT		1
#define TIMER_RAISE		2
#ifdef DEC_MOTIF_BUG_FIX
/* Timer for retry on interactive placement */ 
#define TIMER_INTERPLACE         100
#endif /* DEC_MOTIF_BUG_FIX */


/*************************************<->*************************************
 *
 *  Window manager frame component data structures
 *
 *
 *  Description:
 *  -----------
 *  This data stucture is used for drawing frame component graphics.
 *
 *************************************<->***********************************/

typedef struct _RList
{
    int		allocated;		/* number of allocated XRectangles */
    int		used;			/* number currently in use */
    XRectangle	*prect;			/* array of XRectangles */
} RList;




/*************************************<->*************************************
 *
 *  Window manager component appearance data structure ...
 *
 *
 *  Description:
 *  -----------
 *  This structure is used to hold component appearance data for client,
 *  icon, and feedback subparts. 
 * 
 *************************************<->***********************************/


typedef struct _AppearanceData
{
    XmFontList	fontList;			/* resource */
    XFontStruct	*font;
#ifndef NO_MULTIBYTE
    unsigned int	titleHeight;		/* title bar's height */
#endif
    Boolean	saveUnder;			/* resource */
    Pixel	background;			/* resource */
    Pixel	foreground;			/* resource */
    String	backgroundPStr;			/* resource */
    Pixmap	backgroundPixmap;
    Pixel	bottomShadowColor;		/* resource */
    String	bottomShadowPStr;		/* resource */
    Pixmap	bottomShadowPixmap;
    Pixel	topShadowColor;			/* resource */
    String	topShadowPStr;			/* resource */
    Pixmap	topShadowPixmap;
    Pixel	activeBackground;		/* resource */
    String	activeBackgroundPStr;		/* resource */
    Pixmap	activeBackgroundPixmap;
    Pixel	activeBottomShadowColor;	/* resource */
    String	activeBottomShadowPStr;		/* resource */
    Pixmap	activeBottomShadowPixmap;
    Pixel	activeForeground;		/* resource */
    Pixel	activeTopShadowColor;		/* resource */
    String	activeTopShadowPStr;		/* resource */
    Pixmap	activeTopShadowPixmap;

    GC		inactiveGC;
    GC		inactiveTopShadowGC;
    GC		inactiveBottomShadowGC;
    GC		activeGC;
    GC		activeTopShadowGC;
    GC		activeBottomShadowGC;

} AppearanceData;


typedef struct _AppearanceData *PtrAppearanceData;


/*************************************<->*************************************
 *
 *  IconInfo
 *
 *
 *  Description:
 *  -----------
 *
 *************************************<->***********************************/

typedef struct _IconInfo
{
        Widget theWidget;
        struct _ClientData *pCD;
} IconInfo;

typedef struct _IconInfo *PtrIconInfo;



/*************************************<->*************************************
 *
 *  IconPlacement
 *
 *
 *  Description:
 *  -----------
 *
 *************************************<->***********************************/

typedef struct _IconPlacementData
{
    Boolean	onRootWindow;		/* true if icons on root window */
    unsigned 	iconPlacement;		/* style of placement */
    int		placementRows;		/* number of rows in placement space */
    int		placementCols;		/* number of cols in placement space */
    int		totalPlaces;		/* total number of placment slots */
    int		iPlaceW;		/* width increment (to next place) */
    int		iPlaceH;		/* height increment (to next place) */
    IconInfo	*placeList;		/* list of icon info */
    int		placeIconX;
    int		placeIconY;
    int		*placementRowY;
    int		*placementColX;
} IconPlacementData;



/*************************************<->*************************************
 *
 *  IconBoxData
 *
 *
 *  Description:
 *  -----------
 *  This structure is used to hold window and widget information for
 *  each icon box.
 *
 *************************************<->***********************************/

typedef struct _IconBoxData
{
    Widget	shellWidget;
    Widget	frameWidget;
    Widget      scrolledWidget;
    Widget	vScrollBar;
    Widget	hScrollBar;
    Widget      bBoardWidget;
    Widget	clipWidget; 
    int		numberOfIcons;
    int		currentRow;
    int		currentCol;
    int		lastRow;
    int		lastCol;
    struct _ClientData	*pCD_iconBox;	/* ptr to its own clientdata */
    struct _IconBoxData *pNextIconBox;	/* ptr to next icon box */
    struct _IconPlacementData IPD;	/* icon placement data */
} IconBoxData;

typedef struct _IconBoxData *PtrIconBoxData;

#define IB_SPACING		0
#define IB_MARGIN_HEIGHT	3 
#define IB_MARGIN_WIDTH		3 
#define IB_HIGHLIGHT_BORDER	3


/*************************************<->*************************************
 *
 *  Bitmap/Pixmap cache data
 *
 *
 *  Description:
 *  -----------
 *  These structures are used for the bitmap and pixmap caches held
 *  on a per-screen basis. (This is per-screen because you can't do
 *  XCopyPlane from one pixmap to another when they have different
 *  roots.)
 *
 *************************************<->***********************************/

#define NO_PIXMAP    0
#define LABEL_PIXMAP 1
#define ICON_PIXMAP  2

typedef struct _PixmapCache
{
   unsigned int  pixmapType;   /* icon or label? */
   Pixel         foreground;
   Pixel         background;
   Pixmap        pixmap;
   struct _PixmapCache *next;

} PixmapCache;

typedef struct _BitmapCache
{
   char         *path;
   Pixmap        bitmap;
   unsigned int  width;
   unsigned int  height;
   PixmapCache  *pixmapCache;

} BitmapCache;





/******************************<->*************************************
 *
 *  Client window list entry data structure ...
 *
 *
 *  Description:
 *  -----------
 *  This structure provides the data for an entry in the client window list.
 *  The client window list has an entry for each non-transient client
 *  window and each non-iconbox icon.
 * 
 ******************************<->***********************************/

typedef struct _ClientListEntry
{
    struct _ClientListEntry *nextSibling;
    struct _ClientListEntry *prevSibling;
    int		type;
    struct _ClientData *pCD;

} ClientListEntry;



/*************************************<->*************************************
 *
 *  Frame information
 *
 *
 *  Description:
 *  -----------
 *  This structure contains geometry information for the window manager 
 *  frame.
 * 
 *************************************<->***********************************/

typedef struct _FrameInfo
{
    int			x;
    int			y;
    unsigned int	width;
    unsigned int	height;
    unsigned int	upperBorderWidth;
    unsigned int	lowerBorderWidth;
    unsigned int	cornerWidth;
    unsigned int	cornerHeight;
    unsigned int	titleBarHeight;

} FrameInfo;



/*************************************<->*************************************
 *
 *  WmScreenData
 *
 *
 *  Description:
 *  -----------
 *  This is the data structure for holding the window manager's
 *  screen data. There is one instantiation of the structure for
 *  each screen.
 *
 *************************************<->***********************************/

typedef struct _WmScreenData
{
    int		dataType;
    int		screen;			/* number for this screen */
    Boolean	managed;
    Window	rootWindow;
    Widget	screenTopLevelW;
    Widget	screenTopLevelW1;       /* for internal WM components */
    Widget      confirmboxW[4];
    Window	wmWorkspaceWin;		/* holds wm properties */
    Window	feedbackWin;
    Window	activeIconTextWin;
    Window	activeLabelParent;
    String	displayString;		/* used for putenv in F_Exec */

    /* wm state info: */

    unsigned long clientCounter;
    long	focusPriority;
    Window	inputScreenWindow;
    struct _ClientData	*colormapFocus;
    Colormap	workspaceColormap;
    Colormap	lastInstalledColormap;
    struct _WmWorkspaceData	*pActiveWS;	/* for this screen */

    /* per screen caches */
    BitmapCache *bitmapCache;
    unsigned int bitmapCacheSize;
    unsigned int bitmapCacheCount;

    /* per screen icon info */
    Boolean	fadeNormalIcon;			/* resource */
    long	iconPlacement;			/* resource */
    int		iconPlacementMargin;		/* resource */
    long	iconDecoration;			/* resource */
    WHSize	iconImageMaximum;		/* resource */
    WHSize	iconImageMinimum;		/* resource */
    Pixmap	builtinIconPixmap;
    int		iconWidth;
    int		iconHeight;
    int		iconImageHeight;
    int		iconLabelHeight;
    GC		shrinkWrapGC;
    GC		fadeIconGC;
    GC		fadeIconTextGC;


    /* per screen feedback data */
    unsigned long fbStyle;
    unsigned int fbWinWidth;
    unsigned int fbWinHeight;
    char fbLocation[20];
    char fbSize[20];
    int fbLocX;
    int fbLocY;
    int fbSizeX;
    int fbSizeY;
    int fbLastX;
    int fbLastY;
    unsigned int fbLastWidth;
    unsigned int fbLastHeight;
    RList *fbTop;
    RList *fbBottom;
    int     actionNbr;

    /* resource description file data: */

    String	buttonBindings;			/* resource */
    ButtonSpec	*buttonSpecs;
    String	keyBindings;			/* resource */
    KeySpec	*keySpecs;
    MenuSpec   **acceleratorMenuSpecs;
    unsigned int acceleratorMenuCount;
    MenuSpec	*menuSpecs;

    Boolean	defaultSystemMenuUseBuiltin;

    Pixmap	defaultPixmap;
    GC		xorGC;

    /* per screen appearance resources */

    Boolean	cleanText;			/* resource */
    Boolean	decoupleTitleAppearance;	/* see clientTitleAppearance */
    int		frameBorderWidth;		/* resource */
    String	feedbackGeometry;		/* resource */
    String	iconBoxGeometry;		/* resource */
    String	iconBoxName;			/* resource */
    String      iconBoxSBDisplayPolicy;         /* resource */
    int		iconBoxScheme;			/* resource - testing */
    XmString    iconBoxTitle;			/* resource */
    int		externalBevel;
    int		joinBevel;
    Boolean	limitResize;			/* resource */
    WHSize	maximumMaximumSize;		/* resource */
    int		resizeBorderWidth;		/* resource */
    Boolean	resizeCursors;			/* resource */
    int		transientDecoration;		/* resource */
    int		transientFunctions;		/* resource */
    Boolean	useIconBox;			/* resource */
    Boolean     moveOpaque;                     /* move window not outlines */


    /* client frame component appearance resources and data: */

    AppearanceData clientAppearance;		/* resources ... */
    AppearanceData clientTitleAppearance;	/* resources ... */
    XPoint	transientOffset;
    int		Num_Resize_Ts_Elements;
    int		Num_Resize_Bs_Elements;
    int		Num_Title_Ts_Elements;
    int		Num_Title_Bs_Elements;

    /* icon component appearance resources and data: */

    AppearanceData iconAppearance;		/* resources ... */

    /* feedback component appearance resources and data: */

    AppearanceData feedbackAppearance;		/* resources ... */

    /* client list pointers: */

    ClientListEntry 	*clientList;
    ClientListEntry 	*lastClient;

    /* workspace for this screen */
              
    struct _WmWorkspaceData	*pWS;
#ifdef DEC_MOTIF_EXTENSION
/* Resource info for each screen. */
    Boolean     reset_resources;
    XrmDatabase user_database;
    int         monitor;
    XrmDatabase sys_database;
    Boolean     user_database_set;
    Boolean     sys_database_set;
#define k_mwm_color_type 1
#define k_mwm_bw_type 2   
#define k_mwm_gray_type 3
#endif /* DEC_MOTIF_EXTENSION */

} WmScreenData;

typedef struct _WmScreenData *PtrScreenData;


/* 
 * Convenience macros for data access
 */
#define ROOT_FOR_CLIENT(pcd) ((pcd)->pSD->rootWindow)
#define SCREEN_FOR_CLIENT(pcd) ((pcd)->pSD->screen)
#define PSD_FOR_CLIENT(pcd) ((pcd)->pSD)
#define BUTTON_SPECS(pcd) ((pcd)->pSD->buttonSpecs)
#define KEY_SPECS(pcd) ((pcd)->pSD->keySpecs)
#define ACCELERATOR_MENU_COUNT(pcd) ((pcd)->pSD->acceleratorMenuCount)
#define ACCELERATOR_MENU_SPECS(pcd) ((pcd)->pSD->acceleratorMenuSpecs)
#define WORKSPACE_COLORMAP(pcd) ((pcd)->pSD->workspaceColormap)
#define FADE_NORMAL_ICON(pcd) ((pcd)->pSD->fadeNormalIcon)
/*
#define ICON_DEFAULT_TITLE(pcd) ((pcd)->iconDefaultTitle)
*/
#define ICON_DECORATION(pcd) ((pcd)->pSD->iconDecoration)
#define ICON_HEIGHT(pcd) ((pcd)->pSD->iconHeight)
#define ICON_WIDTH(pcd) ((pcd)->pSD->iconWidth)
#define ICON_IMAGE_HEIGHT(pcd) ((pcd)->pSD->iconImageHeight)
#define ICON_LABEL_HEIGHT(pcd) ((pcd)->pSD->iconLabelHeight)
#define ICON_IMAGE_MAXIMUM(pcd) ((pcd)->pSD->iconImageMaximum)
#define ICON_IMAGE_MINIMUM(pcd) ((pcd)->pSD->iconImageMinimum)
#define SHRINK_WRAP_GC(pcd) ((pcd)->pSD->shrinkWrapGC)
#define FADE_ICON_GC(pcd) ((pcd)->pSD->fadeIconGC)
#define FADE_ICON_TEXT_GC(pcd) ((pcd)->pSD->fadeIconTextGC)
#define DEFAULT_PIXMAP(pcd) ((pcd)->pSD->defaultPixmap)
#define ICON_PLACE(pcd) ((pcd)->iconPlace)
#define ICON_X(pcd) ((pcd)->iconX)
#define ICON_Y(pcd) ((pcd)->iconY)
#define P_ICON_BOX(pcd) ((pcd)->pIconBox)
#define ICON_FRAME_WIN(pcd) ((pcd)->iconFrameWin)



/*************************************<->*************************************
 *
 *  WmWorkspaceData
 *
 *
 *  Description:
 *  -----------
 *  This is the structure for holding the workspace specific data. 
 *  (This has been broken out in anticipation of possible future 
 *  enhancements.)
 * 
 *************************************<->***********************************/

typedef struct _WmWorkspaceData
{
    int			dataType;

    String		name;		/* workspace name */

    WmScreenData	*pSD;		/* screen data for this workspace */
    IconBoxData 	*pIconBox;	/* icon box data for this workspace */
    IconPlacementData 	IPData;

    Widget		workspaceTopLevelW;

    /* workspace state information */

    struct _ClientData	*keyboardFocus;	/* ptr to client with the key focus */
    struct _ClientData	*nextKeyboardFocus; /* next client to get focus */

} WmWorkspaceData;

typedef struct _WmWorkspaceData *PtrWorkspaceData;


/*
 * Convenience macros for data access
 */
#define CLIENT_APPEARANCE(pcd) ((pcd)->pSD->clientAppearance)
#define CLIENT_TITLE_APPEARANCE(pcd) ((pcd)->pSD->clientTitleAppearance)
#define DECOUPLE_TITLE_APPEARANCE(pcd) ((pcd)->pSD->decoupleTitleAppearance)
/*
#define CLIENT_DEFAULT_TITLE(pcd) ((pcd)->pSD->clientDefaultTitle)
*/
#define MAX_MAX_SIZE(pcd) ((pcd)->pSD->maximumMaximumSize)
#define SHOW_RESIZE_CURSORS(pcd) ((pcd)->pSD->resizeCursors)
#define JOIN_BEVEL(pcd) ((pcd)->pSD->joinBevel)
#define EXTERNAL_BEVEL(pcd) ((pcd)->pSD->externalBevel)
#define FRAME_BORDER_WIDTH(pcd) ((pcd)->pSD->frameBorderWidth)
#define RESIZE_BORDER_WIDTH(pcd) ((pcd)->pSD->resizeBorderWidth)
#define NUM_TITLE_TS_ELEMENTS(pcd) ((pcd)->pSD->Num_Title_Ts_Elements)
#define NUM_TITLE_BS_ELEMENTS(pcd) ((pcd)->pSD->Num_Title_Bs_Elements)
#define NUM_RESIZE_TS_ELEMENTS(pcd) ((pcd)->pSD->Num_Resize_Ts_Elements)
#define NUM_RESIZE_BS_ELEMENTS(pcd) ((pcd)->pSD->Num_Resize_Bs_Elements)
#define ICON_APPEARANCE(pcd) ((pcd)->pSD->iconAppearance)

#define ACTIVE_LABEL_PARENT(pcd) ((pcd)->pSD->activeLabelParent)

#define ICON_BOX_GEOMETRY(pcd) ((pcd)->pSD->iconBoxGeometry)
#define ICON_BOX_TITLE(pcd) ((pcd)->pSD->iconBoxTitle)

#define TRANSIENT_DECORATION(pcd) ((pcd)->pSD->transientDecoration)
#define TRANSIENT_FUNCTIONS(pcd) ((pcd)->pSD->transientFunctions)


/*************************************<->*************************************
 *
 *  ClientData
 *
 *
 *  Description:
 *  -----------
 *  This data structure is instantiated for every client window that is
 *  managed by the window manager.  The information that is saved in the
 *  data structure is specific to the associated client window.
 *
 *  ClientData is instantiated at the time a client window is intially
 *  managed and is destroyed when the client window is withdrawn from
 *  window management (the ClientData may not be destroyed when a 
 *  client window is withdrawn if frame/icons are cached).
 *
 *************************************<->***********************************/

typedef struct _ClientData
{
    int		dataType;			/* client data type */

    Window	client;
    long	clientFlags;
    int		icccVersion;
    int		clientState;			/* WM_HINTS field */
    int		inputFocusModel;		/* WM_HINTS field */
    int		inputMode;
    long	focusPriority;
    unsigned long clientID;
    int		wmUnmapCount;

    /* client supported protocols: */

    Atom	*clientProtocols;		/* WM_PROTOCOLS */
    int		clientProtocolCount;
    long	protocolFlags;
    long	*mwmMessages;			/* _MWM_MESSAGES */
    int		mwmMessagesCount;

    /* client colormap data: */

    Colormap	clientColormap;			/* selected client colormap */
    Window	*cmapWindows;			/* from WM_COLORMAP_WINDOWS */
    Colormap	*clientCmapList;
    int		clientCmapCount;		/* len of clientCmapList */
    int		clientCmapIndex;		/* current cmap in list */
#ifndef OLD_COLORMAP /* colormap */
    int		*clientCmapFlags;		/* installed, uninstalled */
    Bool	clientCmapFlagsInitialized;	/* Are clientCmapFlags valid? */
#endif

    /* associated window data: */

    ClientListEntry clientEntry;
    ClientListEntry iconEntry;
    XID		windowGroup;			/* WM_HINTS field */
    IconBoxData *pIconBox;			/* icon box for this win */
    IconBoxData *thisIconBox;			/* icon box data for self */
    						/*   if this is an icon box */
    Context    grabContext;                     /* used to support icon box */
                                                /* icon key, button, menus */
    Window	transientFor;			/* transient for another win */
    struct _ClientData *transientLeader;	/* trans leader of this win */
    struct _ClientData *transientChildren;	/* transients of this win */
    struct _ClientData *transientSiblings;	/* related transient win's */
    int		primaryModalCount;		/* primary modal win count */
    int		fullModalCount;			/* full modal win count */

    /* client resource data */

    String	clientClass;			/* WM_CLASS field */
    String	clientName;			/* WM_CLASS filed */
    long	clientDecoration;		/* resource */
    long	clientFunctions;		/* resource */
    Boolean	focusAutoRaise;			/* resource */
    String	iconImage;			/* resource */
    Pixel	iconImageBackground;		/* resource */
    Pixel	iconImageBottomShadowColor;	/* resource */
    String	iconImageBottomShadowPStr;	/* resource */
    Pixmap	iconImageBottomShadowPixmap;
    Pixel	iconImageForeground;		/* resource */
    Pixel	iconImageTopShadowColor;	/* resource */
    String	iconImageTopShadowPStr;		/* resource */
    Pixmap	iconImageTopShadowPixmap;
    int		internalBevel;			/* resource */
    Pixel	matteBackground;		/* resource */
    Pixel	matteBottomShadowColor;		/* resource */
    String	matteBottomShadowPStr;		/* resource */
    Pixmap	matteBottomShadowPixmap;
    Pixel	matteForeground;		/* resource */
    Pixel	matteTopShadowColor;		/* resource */
    String	matteTopShadowPStr;		/* resource */
    Pixmap	matteTopShadowPixmap;
    int		matteWidth;			/* resource */
    WHSize	maximumClientSize;		/* resource */
    String	systemMenu;			/* resource */
    MenuItem    *mwmMenuItems;			/* custom menu items or NULL */
    MenuSpec	*systemMenuSpec;
    Boolean	useClientIcon;			/* resource */

    /* client frame data: */

    long	sizeFlags;			/* WM_NORMAL_HINTS field */
    long	decor;				/* client decoration*/
    long	decorFlags;			/* depressed gadgets flags */
    int		minWidth;			/* WM_NORMAL_HINTS field */
    int		minHeight;			/* WM_NORMAL_HINTS field */
    Boolean	maxConfig;			/* True => use max config */
    int		maxX;				/* maximized window X loc */
    int		maxY;				/* maximized window Y loc */
    int		maxWidthLimit;
    int		maxWidth;			/* WM_NORMAL_HINTS field */
    int		maxHeightLimit;
    int		maxHeight;			/* WM_NORMAL_HINTS field */
    int		widthInc;			/* WM_NORMAL_HINTS field */
    int		heightInc;			/* WM_NORMAL_HINTS field */
    AspectRatio	minAspect;			/* WM_NORMAL_HINTS field */
    AspectRatio	maxAspect;			/* WM_NORMAL_HINTS field */
    int		baseWidth;			/* WM_NORMAL_HINTS field */
    int		baseHeight;			/* WM_NORMAL_HINTS field */
    int		windowGravity;			/* WM_NORMAL_HINTS field */
    int		clientX;			/* normal window X loc */
    int		clientY;			/* normal window Y loc */
    int		clientWidth;			/* normal window width */
    int		clientHeight;			/* normal window height */
    XPoint	clientOffset;			/* frame to client window */
    XmString	clientTitle;			/* WM_NAME field */
    Window	clientFrameWin;			/* top-level, frame window */
    Window	clientStretchWin[STRETCH_COUNT];/* for resizing border */
    Window	clientTitleWin;			/* for title bar */
    Window	clientBaseWin;			/* for matte & reparenting */
    int		xBorderWidth;			/* original X border width */
    FrameInfo	frameInfo;			/* frame geometry data */

    /* client window frame graphic data: */

    RList	*pclientTopShadows;		/* top shadow areas */
    RList	*pclientBottomShadows;		/* bottom shadow areas */

    RList	*pclientTitleTopShadows;	/* top shadow areas */
    RList	*pclientTitleBottomShadows;	/* bottom shadow areas */

    RList	*pclientMatteTopShadows;	/* matte top shadows */
    RList	*pclientMatteBottomShadows;	/* matte bottom shadows */

    /* rectangles for client frame gadgets */

    XRectangle		titleRectangle;		/* title bar area */
    GadgetRectangle	*pTitleGadgets;		/* title bar gadgets */
    int			cTitleGadgets;		/* count of title rects */
    GadgetRectangle	*pResizeGadgets;	/* resize areas */
    XRectangle		matteRectangle;		/* matte / base window area */

    /* client appearance data: */

    GC		clientMatteTopShadowGC;
    GC		clientMatteBottomShadowGC;
    WmScreenData	*pSD;			/* where visuals come from */

    /* icon data: */

    long	iconFlags;
    XmString	iconTitle;			/* WM_ICON_NAME field */
    int		iconX;				/* WM_HINTS field */
    int		iconY;				/* WM_HINTS field */
    int		iconPlace;
    Window	iconFrameWin;
    Pixmap	iconPixmap;			/* WM_HINTS field */
    Pixmap	iconMask;			/* WM_HINTS field */
    Window	iconWindow;			/* WM_HINTS field */

    RList	*piconTopShadows;		/* these change to 	*/
    						/* to reflect the 	*/
    RList	*piconBottomShadows;		/* depressed icon image */

#ifndef NO_SHAPE
    short       wShaped;                /* this window has a bounding shape */
#endif /* NO_SHAPE  */

    int		usePPosition;		/* indicate whether to use PPosition */

    long	window_status;			/* used for Tear-off Menus */

#ifdef DEC_MOTIF_BUG_FIX
    /* Used for replay of manage window due to interplace failure */
    long manageFlags;
    /* Current number of retries */
    int interPlaceRetries;
    /*  List of transient dialog boxes with no parents */
    /* This has been fixed in Xm and Mwm 1.2 (See WmCEvent.C
       HandleCPropertyNotify).  However, this code fix in 1.1 
       for losttransients needs  to remain for backward compatibility 
       for XUI and Motif 1.1 applications. */
    struct _ClientData *transientLost;
#endif /* DEC_MOTIF_BUG_FIX */
} ClientData;

typedef struct _ClientData *PtrClientData;

/* client data convenience macros */

#define IS_APP_MODALIZED(pcd) \
    (((pcd)->primaryModalCount)||((pcd)->fullModalCount))

#define IS_MAXIMIZE_VERTICAL(pcd) \
  ((pcd->maximumClientSize.height == BIGSIZE) && \
   (pcd->maximumClientSize.width == 0))

#define IS_MAXIMIZE_HORIZONTAL(pcd) \
  ((pcd->maximumClientSize.width == BIGSIZE) && \
   (pcd->maximumClientSize.height == 0))

/* window management state of client windows (clientState): */
#define WITHDRAWN_STATE		0
#define NORMAL_STATE		1
#define MINIMIZED_STATE		2
#define MAXIMIZED_STATE		3

/* clientFlags field values: */
#define CLIENT_HINTS_TITLE		(1L << 0)
#define CLIENT_REPARENTED		(1L << 1)
#define CLIENT_TRANSIENT		(1L << 2)
#define CLIENT_CONTEXT_SAVED		(1L << 3)
#define CLIENT_IN_SAVE_SET		(1L << 4)
#define USERS_MAX_POSITION		(1L << 5)
#define WM_INITIALIZATION		(1L << 6)
#define CLIENT_DESTROYED		(1L << 7)
#define ICON_REPARENTED			(1L << 8)
#define ICON_IN_SAVE_SET		(1L << 9)
#define CLIENT_TERMINATING		(1L << 10)
#define ICON_BOX                        (1L << 11)  /* one of our icon boxes */
#define CONFIRM_BOX                     (1L << 12)  /* a confirmation box */


#define CLIENT_WM_CLIENTS		(ICON_BOX | CONFIRM_BOX)

/* decorFlags bit fields */
#define SYSTEM_DEPRESSED		(1L << 0)
#define TITLE_DEPRESSED			(1L << 1)
#define MINIMIZE_DEPRESSED		(1L << 2)
#define MAXIMIZE_DEPRESSED		(1L << 3)

/* iconFlags field values: */
#define ICON_HINTS_POSITION		(1L << 0)
#define ICON_HINTS_TITLE		(1L << 1)
#define ICON_HINTS_PIXMAP		(1L << 2)

/* client protocol flags and sizes: */
#define PROTOCOL_WM_SAVE_YOURSELF	(1L << 0)
#define PROTOCOL_WM_DELETE_WINDOW	(1L << 1)
#define PROTOCOL_WM_TAKE_FOCUS		(1L << 2)
#define PROTOCOL_MWM_MESSAGES		(1L << 3)
#define PROTOCOL_MWM_OFFSET		(1L << 4)
#ifdef DEC_MOTIF_EXTENSION
#define PROTOCOL_MWM_FRAME    	       	(1L << 5)                   
#define PROTOCOL_DEC_WM_OLD_ICCCM       (1L << 6)
#endif

#define MAX_CLIENT_PROTOCOL_COUNT	40
#define MAX_COLORMAP_WINDOWS_COUNT	40
#define MAX_MWM_MESSAGES_COUNT		40

/* bevel width limits between window manager frame and client window */
#define MIN_INTERNAL_BEVEL		1
#define MAX_INTERNAL_BEVEL		2



/*************************************<->*************************************
 *
 *  WmGlobalData
 *
 *
 *  Description:
 *  -----------
 *  This is the main data structure for holding the window manager's
 *  global data. There is one instantiation of the structure for
 *  the window manager.
 * 
 *************************************<->***********************************/

typedef struct _WmGlobalData
{               
    int		dataType;
    char	**argv;			/* command line argument vector */
    char	**environ;		/* environment vector */	
    char	*mwmName;		/* name of our executable */
    Widget	topLevelW;
    Boolean     confirmDialogMapped;    /* confirm dialog is mapped */
    XtAppContext	mwmAppContext;	/* application context for mwm */
    XContext	windowContextType;	/* window context for XSaveContext */
    XContext	screenContextType;	/* screen context for XSaveContext */

    /* presentation resource id's: */

    Display	*display;		/* display we are running to */
    int		numScreens;		/* number of screens */
    unsigned char	**screenNames;	/* default names for screens */
    WmScreenData	*Screens;	/* array of screen info */

    Cursor	workspaceCursor;		/* basic arrow cursor */
    Cursor	stretchCursors[STRETCH_COUNT];
    Cursor	configCursor;
    Cursor	movePlacementCursor;
    Cursor	sizePlacementCursor;



    /* wm state info: */

    WmScreenData *pActiveSD;		/* with keyfocus window */
    Boolean	useStandardBehavior;	/* behavior flag */
    Boolean	wmRestarted;		/* restart flag */
    Boolean	errorFlag;		/* handle on async errors */

		    /* The following are global because pointer is grabbed */
    MenuSpec	*menuActive;		/* ptr to currently active menu */
    KeySpec	*menuUnpostKeySpec;	/* key to upost current menu */
    ClientData	*menuClient;		/* client for which menu is posted */
    KeySpec     *F_NextKeySpec;         /* used when travering to windows */
    KeySpec     *F_PrevKeySpec;         /* used when travering to windows */

    Context     grabContext;            /* used in GrabWin when no event */

    ClickData	clickData;		/* double-click detection data */
    int		configAction;		/* none, resize, move */
    unsigned int configButton;		/* button used for config */
    unsigned int configPart;		/* resize frame part */
    Boolean 	configSet;		/* True if configPart set */
    Boolean	movingIcon;		/* True if icon being moved */
    Boolean	preMove;		/* move threshold support */
    int		preMoveX;
    int		preMoveY;
    ClientData	*gadgetClient;		/* last client with */
    int		gadgetDepressed;	/* depressed gadget */
    Boolean	checkHotspot;		/* event hotspot flag */
    XRectangle	hotspotRectangle;	/* event hotspot area */
    WmTimer	*wmTimers;		/* timer data */
    Boolean	passKeysActive;		/* flag for pass keys function */
    KeySpec	*passKeysKeySpec;	/* key for pass keys function */
    Boolean	activeIconTextDisplayed;	/* True if active label up */
    unsigned int currentEventState;	/* key/button event modifier state */
    Boolean	queryScreen;		/* True if not sure of active screen */

    /* keyboard focus data: */

    ClientData	*keyboardFocus;		/* ptr to client with the key focus */
    ClientData	*nextKeyboardFocus;	/* next client to get focus */
    Boolean	systemModalActive;
    ClientData	*systemModalClient;
    Window	systemModalWindow;

    /* atoms used in inter-client communication: */

    Atom	xa_WM_STATE;
    Atom	xa_WM_PROTOCOLS;
    Atom	xa_WM_CHANGE_STATE;
    Atom	xa_WM_SAVE_YOURSELF;
    Atom	xa_WM_DELETE_WINDOW;
    Atom	xa_WM_TAKE_FOCUS;
    Atom	xa_WM_COLORMAP_WINDOWS;
    Atom	xa_MWM_HINTS;
    Atom	xa_MWM_MESSAGES;
    Atom	xa_MWM_MENU;
    Atom	xa_MWM_INFO;
    Atom	xa_MWM_OFFSET;
#ifdef DEC_MOTIF_EXTENSION
    Atom	xa_MWM_FRAME;
#endif
#ifdef AUTOMATION
	Atom    xa_MWM_FRAME_ICON_INFO;
#endif
    Atom	xa_MOTIF_BINDINGS;
    Atom        xa_COMPOUND_TEXT;
#ifdef VMS
    Atom        xa_TKM_REQUESTS;
    int         TKM_VERSION;
#endif
    /* mwm specific appearance and behavior resources and data: */

    Boolean	autoKeyFocus;			/* resource */
    int		autoRaiseDelay;			/* resource */
    String	bitmapDirectory;		/* resource */
    Boolean	clientAutoPlace;		/* resource */
    int		colormapFocusPolicy;		/* resource */
    String	configFile;			/* resource */
    Boolean	deiconifyKeyFocus;		/* resource */
    int		doubleClickTime;		/* resource */
    Boolean	enableWarp;			/* resource */
    Boolean	enforceKeyFocus;		/* resource */
    Boolean	freezeOnConfig;			/* resource - testing */
    Boolean	iconAutoPlace;			/* resource */
    Boolean	iconClick;			/* resource */
    Boolean	ignoreLockMod;			/* resource */
    Boolean	interactivePlacement;		/* resource */
    int		keyboardFocusPolicy;		/* resource */
    Boolean	lowerOnIconify;			/* resource */
    int		moveThreshold;			/* resource */
    Boolean     passButtonsCheck; /* used to override passButtons */
    Boolean	passButtons;			/* resource */
    Boolean	passSelectButton;		/* resource */
    Boolean	positionIsFrame;		/* resource */
    Boolean	positionOnScreen;		/* resource */
    int		quitTimeout;			/* resource */
    Boolean     raiseKeyFocus;                  /* resource */
    Boolean     multiScreen;                  	/* resource */
    String	screenList;			/* resource */
    long	showFeedback;			/* resource */
    Boolean	startupKeyFocus;		/* resource */
    Boolean	systemButtonClick;		/* resource */
    Boolean	systemButtonClick2;		/* resource */
    Boolean	useLargeCursors;
    Boolean	waitForClicks;			/* resource */

    XmString	clientDefaultTitle;
    XmString	iconDefaultTitle;

    Window	attributesWindow;
    XWindowAttributes	windowAttributes;

#ifndef NO_SHAPE
    Boolean     hasShape;                /* server supports Shape extension */
    int         shapeEventBase, shapeErrorBase;
#endif /* SHAPE */
#ifdef DEC_MOTIF_EXTENSION
    Cursor      waitCursor;
    /* Display for dialog boxes */
    Display     *dialog_display;
    /* Dual heads of different colors */
    Boolean     multicolor;
    /* Dual heads, what's the main color, color or gray */
    int         main_monitor;
    /* debug flag */
    Boolean     debug;
    /* current mode switch from display for internationalization */
    unsigned int mode_switch;
    /* debug file */
    FILE *debug_file;
    Boolean        useDECMode;  /* Use DEC wm style */
    /* Override button bindings and always set Alt (or Compose) Space
       to post window menu.  This is useful for keyboard with both
       Alt and Compose. */
    Boolean        forceAltSpace; 
    /* Do not display warnings about icons or other ICCCM features */
    Boolean        ICCCMCompliant;  
    /* Ignore current mod keys for internationalization */
    Boolean        ignoreModKeys;
    /* Ignore all MOD keys for internationalization */
    Boolean        ignoreAllModKeys;                          
#ifndef NumLockMask
#define NumLockMask Mod4Mask
#endif
    Boolean	   ignoreNumLockMod;	/* Ignore NumLock modifier */
    /* Show icons full depth */
    Boolean        iconFullDepth;
#endif
#ifdef DEC_MOTIF_BUG_FIX
    /* Used if interplace fails */
    int		interPlaceDelay;			/* resource */
    int		interPlaceRetries;               /* resource */
    /*  List of transient dialog boxes with no parents */
    /* This has been fixed in Xm and Mwm 1.2 (See WmCEvent.C
       HandleCPropertyNotify).  However, this code fix in 1.1 
       for losttransients needs  to remain for backward compatibility 
       for XUI and Motif 1.1 applications. */
    ClientData *transientLost;
    /* Need to replay enter notify events on windows with the
       pointer that used to be modalized.  This is for pointer focus. */
    int         replayEnterEvent;
    XEnterWindowEvent savedEnterEvent;
#endif /* DEC_MOTIF_BUG_FIX */
} WmGlobalData;

/* quick references to global data: */
#define DISPLAY		wmGD.display
#define ACTIVE_PSD	(wmGD.pActiveSD)
#define ACTIVE_SCREEN	(wmGD.pActiveSD->screen)
#define ACTIVE_WS	(wmGD.pActiveSD->pActiveWS)
#define ACTIVE_ROOT	(wmGD.pActiveSD->rootWindow)
#define ACTIVE_ICON_TEXT_WIN (wmGD.pActiveSD->activeIconTextWin)
#ifdef DEC_MOTIF_EXTENSION
#define ONE_SCREEN      (wmGD.numScreens == 1)
#define WID_SCREEN_NUM ( XScreenNumberOfScreen( XtScreen( wid )))
#endif /* DEC_MOTIF_EXTENSION */

/* colormap focus policy values (colormapFocus): */
#define CMAP_FOCUS_EXPLICIT	0
#define CMAP_FOCUS_POINTER	1
#define CMAP_FOCUS_KEYBOARD	2

/* keyboard input focus policy values (keyboardFocus): */
#define KEYBOARD_FOCUS_EXPLICIT	0
#define KEYBOARD_FOCUS_POINTER	1

/* icon appearance values (iconAppearance): */
#define ICON_LABEL_PART			(1L << 0)
#define ICON_IMAGE_PART			(1L << 1)
#define ICON_ACTIVE_LABEL_PART		(1L << 2)
#define USE_ICON_DEFAULT_APPEARANCE	(1L << 3)
#define ICON_APPEARANCE_STANDALONE	(ICON_LABEL_PART | ICON_IMAGE_PART |\
					 ICON_ACTIVE_LABEL_PART)
#define ICON_APPEARANCE_ICONBOX		(ICON_LABEL_PART | ICON_IMAGE_PART)

/* icon placement values (iconPlacement, ...): */
#define ICON_PLACE_LEFT_PRIMARY		(1L << 0)
#define ICON_PLACE_RIGHT_PRIMARY	(1L << 1)
#define ICON_PLACE_TOP_PRIMARY		(1L << 2)
#define ICON_PLACE_BOTTOM_PRIMARY	(1L << 3)
#define ICON_PLACE_LEFT_SECONDARY	(1L << 4)
#define ICON_PLACE_RIGHT_SECONDARY	(1L << 5)
#define ICON_PLACE_TOP_SECONDARY	(1L << 6)
#define ICON_PLACE_BOTTOM_SECONDARY	(1L << 7)
#define ICON_PLACE_EDGE			(1L << 8)
#define ICON_PLACE_TIGHT		(1L << 9)
#define ICON_PLACE_RESERVE		(1L << 10)

#define NO_ICON_PLACE			-1
#define MINIMUM_ICON_SPACING		4
#define MAXIMUM_ICON_MARGIN		128
#define ICON_IMAGE_MAX_WIDTH		128
#define ICON_IMAGE_MAX_HEIGHT		128
#define ICON_IMAGE_MIN_WIDTH		16
#define ICON_IMAGE_MIN_HEIGHT		16

/*default client window title: */
#define DEFAULT_CLIENT_TITLE	"*****"
#define DEFAULT_ICON_TITLE	DEFAULT_CLIENT_TITLE

/* client decoration parameters */
#define MAXIMUM_FRAME_BORDER_WIDTH	64

/* configuration action (configAction): */
#define NO_ACTION			0
#define MOVE_CLIENT			1
#define RESIZE_CLIENT			2
#define PLACE_CLIENT			3
             
/* Motif input bindings file name */
#define MOTIF_BINDINGS_FILE		".motifbind"

/* Data type definitions */
#define GLOBAL_DATA_TYPE		1001
#define CLIENT_DATA_TYPE		1002
#define SCREEN_DATA_TYPE		1003
#define WORKSPACE_DATA_TYPE		1004

/* Stacking functions */
#define STACK_NORMAL			0
#define STACK_WITHIN_FAMILY		1
#define STACK_FREE_FAMILY		2

/* UsePPosition values */
#define USE_PPOSITION_OFF		0
#define USE_PPOSITION_ON		1
#define USE_PPOSITION_NONZERO		2

/* Largest dimension for special casing */
#define BIGSIZE 32767

#ifdef DEC_MOTIF_EXTENSION
/* Define the LANG environment variable */
#ifdef VMS                                
#define MWM_LANG "xnl$lang"
#else      
#define MWM_LANG "LANG"
#endif /* VMS */
#endif /* DEC_MOTIF_EXTENSION */

/*
 * External references for global data:
 */

extern WmGlobalData	wmGD;

#ifdef DEC_MOTIF_EXTENSION
/* Customize arguments */
#define k_mwm_cust_ws_arg "workspace"
#define k_mwm_cust_border_arg "border"
#define k_mwm_cust_icons_arg "icons"
#define k_mwm_cust_border_col_arg "bordercolor"
#define k_mwm_cust_icon_col_arg "iconcolor"     
#define k_mwm_cust_matte_arg "matte"
#define k_mwm_cust_apply_arg "apply"
#define k_mwm_cust_reset_arg "reset"
#define k_mwm_cust_default_arg "default"
#define k_mwm_help_context_arg "context"
#define k_mwm_help_wm_arg "mwm"
#define k_mwm_help_version_arg "version"
#define k_mwm_help_terms_arg "terms"
#define k_mwm_help_shortcuts_arg "shortcuts"
#define k_mwm_help_arg "help"
/* Check for unspecified pixmap */
#define MWM_UNSPEC_PIXMAP( arg ) ( !arg || mwm_str_eq( arg, "unspecified_pixmap" ))
#endif

#ifdef DEC_MOTIF_EXTENSION
/* The following is needed due to a change in the R5 xlib by MIT to 
   reduce the access of certain elements in the display structure that
   were considered private. The ModeSwitchOfDisplay() is a DEC extension
   into the R5 Xlib until such a time as a routine or Macro is provided 
   that returns the same data. */
/* When the ULTRIX and other Xlibs support this extension,
   remove the ifdefs */
#ifdef VMS
extern unsigned int ModeSwitchOfDisplay(Display *);
#else
#ifdef __alpha
extern unsigned int ModeSwitchOfDisplay(Display *);
#else  /* ULTRIX or Sun of OSF/1 */
#define ModeSwitchOfDisplay(Display) ( 0 )
#endif /* __alpha */
#endif /* VMS */
#endif /* DEC_MOTIF_EXTENSION */
