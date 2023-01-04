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
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmResource.c,v $ $Revision: 1.1.4.4 $ $Date: 1993/10/18 17:17:11 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmIBitmap.h"

#ifndef VMS
#include <memory.h>
#endif

#ifdef DEC_MOTIF_EXTENSION
/* Needed for Wmresparse.h */
#include <stdio.h>
/* For exit */
#include <stdlib.h>
/* For _XmGetMoveOpaqueScreen */
#include <Xm/ScreenP.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>	/* needed for IsModifier macro */
#endif                                                                   

#include <Xm/XmP.h>
#include <Xm/RowColumn.h>

/*
 * include extern functions
 */
#include "WmResource.h"
#include "WmError.h"
#include "WmGraphics.h"
#include "WmMenu.h"
#include "WmResParse.h"
#ifdef DEC_MOTIF_EXTENSION
#include "mwm_util.h"
#endif

/*
 * Function Declarations:
 */
XmColorData *_WmGetDefaultColors ();

#ifdef _NO_PROTO
void _WmABackgroundDefault ();
void _WmABackgroundPixmapDefault ();
void _WmABottomShadowColorDefault ();
void _WmAForegroundDefault ();
void _WmATopShadowColorDefault ();
void _WmATopShadowPixmapDefault ();
void _WmBackgroundDefault ();
void _WmBackgroundPixmapDefault ();
void _WmBottomShadowColorDefault ();
void _WmFocusAutoRaiseDefault ();
void _WmForegroundDefault ();
void _WmGetDynamicDefault ();
void _WmIconImageBDefault ();
void _WmIconImageBSCDefault ();
void _WmIconImageBSPDefault ();
void _WmIconImageFDefault ();
void _WmIconImageTSCDefault ();
void _WmIconImageTSPDefault ();
void _WmMatteBDefault ();
void _WmMatteBSCDefault ();
void _WmMatteBSPDefault ();
void _WmMatteFDefault ();
void _WmMatteTSCDefault ();
void _WmMatteTSPDefault ();
void _WmMultiClickTimeDefault ();
void _WmTopShadowColorDefault ();
void _WmTopShadowPixmapDefault ();

#else /* _NO_PROTO */
void _WmTopShadowPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageFDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageBDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageBSCDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageBSPDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageTSCDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageTSPDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteFDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteBDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteBSCDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteBSPDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteTSCDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteTSPDefault (Widget widget, int offset, XrmValue *value);
void _WmBackgroundDefault (Widget widget, int offset, XrmValue *value);
void _WmForegroundDefault (Widget widget, int offset, XrmValue *value);
void _WmBackgroundPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmBottomShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmTopShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmABackgroundDefault (Widget widget, int offset, XrmValue *value);
void _WmAForegroundDefault (Widget widget, int offset, XrmValue *value);
void _WmABackgroundPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmABottomShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmATopShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmATopShadowPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmFocusAutoRaiseDefault (Widget widget, int offset, XrmValue *value);
void _WmMultiClickTimeDefault (Widget widget, int offset, XrmValue *value);
void ProcessWmResources (void);
void ProcessGlobalScreenResources (void);
void SetStdGlobalResourceValues (void);
void ProcessScreenListResource (void);
void ProcessAppearanceResources (WmScreenData *pSD);
void MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources);
void GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, XFontStruct *font, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC);
void ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName);
void ProcessWorkspaceResources (WmWorkspaceData *pWS);
void ProcessClientResources (ClientData *pCD);
void SetStdClientResourceValues (ClientData *pCD);
void SetStdScreenResourceValues (WmScreenData *pSD);
GC GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap);
void _WmGetDynamicDefault (Widget widget, unsigned char type, String defaultColor, Pixel newBackground, XrmValue *value);
Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2);

#endif /* _NO_PROTO */


/*
 * Global Variables:
 */

/* builtin window menu specification */

char defaultSystemMenuName[] = "DefaultWindowMenu";
char builtinSystemMenuName[] = "_MwmWindowMenu_";
#ifndef MCCABE
#ifdef DEC_MOTIF_EXTENSION
/* DEC buildit menu */
#define BUILTINSYSTEMMENU "_MwmWindowMenu_\n\
{\n\
	 Restore 	_R	Alt<Key>F5	f.normalize\n\
	 Move 		_M	Alt<Key>F7	f.move\n\
	 Size 	  	_S	Alt<Key>F8	f.resize\n\
	 Minimize 	_n	Alt<Key>F9	f.minimize\n\
	 Maximize 	_x	Alt<Key>F10	f.maximize\n\
	 Lower 	   	_L	Alt<Key>F3	f.lower\n\
        no-label				f.separator\n\
         Workspace      _W                      f.menu WorkspaceMenu\n\
	 Close 		_C	Alt<Key>F4	f.kill\n\
         Help           _H                      f.menu MwmHelpMenu\n\
}";
#else
#define BUILTINSYSTEMMENU "_MwmWindowMenu_\n\
{\n\
	Restore		_R	Alt<Key>F5	f.restore\n\
	Move		_M	Alt<Key>F7	f.move\n\
	Size		_S	Alt<Key>F8	f.resize\n\
	Minimize	_n	Alt<Key>F9	f.minimize\n\
	Maximize	_x	Alt<Key>F10	f.maximize\n\
	Lower		_L	Alt<Key>F3	f.lower\n\
	no-label				f.separator\n\
	Close		_C	Alt<Key>F4	f.kill\n\
}";
#endif /* DEC_MOTIF_EXTENSION */
char builtinSystemMenu[] = BUILTINSYSTEMMENU
#else /* MCCABE */
char builtinSystemMenu[];
#endif /* MCCABE */

char defaultRootMenuName[] = "DefaultRootMenu";
char builtinRootMenuName[] = "_MwmRootMenu_";
#ifndef MCCABE
#define BUILTINROOTMENU "DefaultRootMenu\n\
{\n\
	\"Root Menu\"		f.title\n\
	\"New Window\"		f.exec \"xterm &\"\n\
	\"Shuffle Up\"		f.circle_up\n\
	\"Shuffle Down\"	f.circle_down\n\
	\"Refresh\"		f.refresh\n\
	\"Pack Icons\"		f.pack_icons\n\
	 no-label		f.separator\n\
	\"Restart...\"		f.restart\n\
}";
char builtinRootMenu[] = BUILTINROOTMENU
#else /* MCCABE */
char builtinRootMenu[];
#endif /* MCCABE */


/* builtin key bindings specification */

char defaultKeyBindingsName[] = "DefaultKeyBindings";
char builtinKeyBindingsName[] = "_MwmKeyBindings_";
#ifndef MCCABE
#ifdef DEC_MOTIF_EXTENSION
/* Alt Ctrl ! is needed to re-toggle back to user's settings.
   Default DEC Motif bindings */
#define BUILTINKEYBINDINGS "_MwmKeyBindings_\n\
{\n\
	Shift<Key>Escape	window|icon		f.post_wmenu\n\
	Meta<Key>space		window|icon		f.post_wmenu\n\
	Meta<Key>Tab		root|icon|window	f.next_key\n\
	Meta Shift<Key>Tab	root|icon|window	f.prev_key\n\
	Meta<Key>Escape		root|icon|window	f.next_key\n\
	Meta Shift<Key>Escape	root|icon|window	f.prev_key\n\
	Meta Shift Ctrl<Key>exclam root|icon|window	f.set_behavior\n\
	Meta Ctrl<Key>exclam    root|icon|window	f.set_behavior\n\
	Meta<Key>F6		window			f.next_key transient\n\
	Meta Shift<Key>F6	window			f.prev_key transient\n\
	    <Key>F4		icon			f.post_wmenu\n\
}";
#else
#define BUILTINKEYBINDINGS "_MwmKeyBindings_\n\
{\n\
	Shift<Key>Escape	window|icon		f.post_wmenu\n\
	Alt<Key>space		window|icon		f.post_wmenu\n\
	Alt<Key>Tab		root|icon|window	f.next_key\n\
	Alt Shift<Key>Tab	root|icon|window	f.prev_key\n\
	Alt<Key>Escape		root|icon|window	f.circle_down\n\
	Alt Shift<Key>Escape	root|icon|window	f.circle_up\n\
	Alt Shift Ctrl<Key>exclam root|icon|window	f.set_behavior\n\
	Alt Ctrl<Key>1		  root|icon|window	f.set_behavior\n\
	Alt<Key>F6		window			f.next_key transient\n\
	Alt Shift<Key>F6	window			f.prev_key transient\n\
	Shift<Key>F10		icon			f.post_wmenu\n\
}";
#endif /* DEC_MOTIF_EXTENSION */
char builtinKeyBindings[] = BUILTINKEYBINDINGS

#else
char builtinKeyBindings[];
#endif

/*
 * NOTE: Default Toggle Behavior key bindings.  There are TWO key bindings as
 * of 1.1.4 and 1.2.  Make sure you make any modify builtinKeyBindings (above)
 * whenever modifying behaviorKeyBindings.
 */

char behaviorKeyBindingName[] = "_MwmBehaviorKey_";
#ifndef MCCABE
#define BEHAVIORKEYBINDINGS "_MwmBehaviorKey_\n\
{\n\
	Alt Shift Ctrl<Key>exclam root|icon|window	f.set_behavior\n\
	Alt Ctrl<Key>1		  root|icon|window	f.set_behavior\n\
}";
char behaviorKeyBindings[] = BEHAVIORKEYBINDINGS

#else
char behaviorKeyBindings[];
#endif


/* default button bindings specification */

char defaultButtonBindingsName[] = "DefaultButtonBindings";
char builtinButtonBindingsName[] = "_MwmButtonBindings_";
#ifndef MCCABE
#ifdef DEC_MOTIF_EXTENSION
/* DEC Built in bindings */
#define BUILTINBUTTONBINDINGS "_MwmButtonBindings_\n\
{\n\
	<Btn1Down>	        icon|frame	f.raise\n\
	<Btn3Down>	        icon|frame	f.post_wmenu\n\
	<Btn1Down>	        root		f.menu	RootMenu\n\
	<Btn3Down>	        root		f.menu	RootMenu\n\
        <Btn1Click2>            title           f.minimize\n\
        Shift <Btn1Click2>      icon            f.minimize\n\
        Shift <Btn1Click>       icon|frame      f.lower\n\
        Ctrl <Btn1Click>        root|icon|frame f.next_key\n\
        Ctrl Shift <Btn1Click>  root|icon|frame f.prev_key\n\
        Meta <Btn1Click>        root|icon|frame f.next_key transient\n\
        Meta Shift <Btn1Click>  root|icon|frame f.prev_key transient\n\
}";
#else
#define BUILTINBUTTONBINDINGS "_MwmButtonBindings_\n\
{\n\
	<Btn1Down>	icon|frame	f.raise\n\
	<Btn3Down>	icon|frame	f.post_wmenu\n\
	<Btn3Down>	root		f.menu DefaultRootMenu\n\
}";
#endif /* DEC_MOTIF_EXTENSION */
char builtinButtonBindings[] = BUILTINBUTTONBINDINGS

#else
char builtinButtonBindings[];
#endif


static ClientData *_pCD;
static String _defaultBackground;
static String _defaultActiveBackground;
static AppearanceData *_pAppearanceData;

static char _defaultColor1HEX[] = "#A8A8A8A8A8A8";
static char _defaultColor2HEX[] = "#5F5F92929E9E";

static char _defaultColor1[] = "LightGrey";
static char _defaultColor2[] = "CadetBlue";
#define DEFAULT_COLOR_NONE	NULL

static char _foreground[]    = "foreground";
static char _50_foreground[] = "50_foreground";
static char _25_foreground[] = "25_foreground";

#define WmBGC          XmBACKGROUND
#define WmFGC          XmFOREGROUND
#define WmTSC          XmTOP_SHADOW
#define WmBSC          XmBOTTOM_SHADOW

#define MAX_SHORT	0xffff

#ifndef BITMAPDIR
#ifdef VMS
#define BITMAPDIR "decw$user_defaults"
#else               
#define BITMAPDIR "/usr/include/X11/bitmaps/"
#endif
#endif


/*************************************<->*************************************
 *
 *  wmGlobalResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm general
 *  appearance and behavior resources.  These resources are specified
 *  with the following syntax:
 *
 *      "Mwm*<resource_identifier>".
 *
 *************************************<->***********************************/


XtResource wmGlobalResources[] =
{

    {
	WmNautoKeyFocus,
	WmCAutoKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, autoKeyFocus),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNautoRaiseDelay,
	WmCAutoRaiseDelay,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, autoRaiseDelay),
	XtRImmediate,
	(caddr_t)500
    },

    {
	WmNbitmapDirectory,
	WmCBitmapDirectory,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, bitmapDirectory),
	XtRString,
	(caddr_t)BITMAPDIR
    },

    {
	WmNclientAutoPlace,
	WmCClientAutoPlace,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, clientAutoPlace),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNcolormapFocusPolicy,
	WmCColormapFocusPolicy,
	WmRCFocusPolicy,
	sizeof (int),
        XtOffsetOf(WmGlobalData, colormapFocusPolicy),
	XtRImmediate,
	(caddr_t)CMAP_FOCUS_KEYBOARD
    },

    {
	WmNconfigFile,
	WmCConfigFile,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, configFile),
	XtRImmediate,
	(caddr_t)NULL
    },

    {
	WmNdeiconifyKeyFocus,
	WmCDeiconifyKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, deiconifyKeyFocus),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNdoubleClickTime,
	WmCDoubleClickTime,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, doubleClickTime),
	XtRCallProc,
	(caddr_t)_WmMultiClickTimeDefault
    },

    {
	WmNenableWarp,
	WmCEnableWarp,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, enableWarp),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNenforceKeyFocus,
	WmCEnforceKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, enforceKeyFocus),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNfreezeOnConfig,
	WmCFreezeOnConfig,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, freezeOnConfig),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNiconAutoPlace,
	WmCIconAutoPlace,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, iconAutoPlace),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNiconClick,
	WmCIconClick,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, iconClick),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNinteractivePlacement,
        WmCInteractivePlacement,
        XtRBoolean,
        sizeof (Boolean),
        XtOffsetOf(WmGlobalData, interactivePlacement),
        XtRImmediate,
        (caddr_t)False
    },

    {
	WmNkeyboardFocusPolicy,
	WmCKeyboardFocusPolicy,
	WmRKFocusPolicy,
	sizeof (int),
        XtOffsetOf(WmGlobalData, keyboardFocusPolicy),
	XtRImmediate,
	(caddr_t)KEYBOARD_FOCUS_EXPLICIT
    },

    {
	WmNlowerOnIconify,
	WmCLowerOnIconify,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, lowerOnIconify),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNmoveThreshold,
	WmCMoveThreshold,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, moveThreshold),
	XtRImmediate,
	(caddr_t)4
    },

    {
	WmNpassButtons,
	WmCPassButtons,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, passButtons),
	XtRImmediate,
	(caddr_t)False
    },

    {
	WmNpassSelectButton,
	WmCPassSelectButton,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, passSelectButton),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNpositionIsFrame,
	WmCPositionIsFrame,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionIsFrame),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNpositionOnScreen,
	WmCPositionOnScreen,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionOnScreen),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNquitTimeout,
	WmCQuitTimeout,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, quitTimeout),
	XtRImmediate,
	(caddr_t)1000
    },

    {
	WmNraiseKeyFocus,
	WmCRaiseKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, raiseKeyFocus),
	XtRImmediate,
	(caddr_t)False
    },

    {
	WmNshowFeedback,
	WmCShowFeedback,
	WmRShowFeedback,
	sizeof (int),
        XtOffsetOf(WmGlobalData, showFeedback),
	XtRImmediate,
	(caddr_t)(WM_SHOW_FB_DEFAULT)
    },

    {
	WmNstartupKeyFocus,
	WmCStartupKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, startupKeyFocus),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNsystemButtonClick,
	WmCSystemButtonClick,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, systemButtonClick),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNsystemButtonClick2,
	WmCSystemButtonClick2,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, systemButtonClick2),
	XtRImmediate,
	(caddr_t)True
    },
#ifdef DEC_MOTIF_BUG_FIX
    {
	WmNinterPlaceDelay,
	WmCinterPlaceDelay,               
	XtRInt,
	sizeof (int),
        XtOffset(WmGlobalData *, interPlaceDelay),
	XtRImmediate,
	(caddr_t)1000
    },

    {
	WmNinterPlaceRetries,
	WmCinterPlaceRetries,
	XtRInt,
	sizeof (int),
        XtOffset(WmGlobalData *, interPlaceRetries),
	XtRImmediate,                           
	(caddr_t)4                 
    },
    {
        WmNuseDECMode,
        WmCUseDECMode,
        XtRBoolean,
        sizeof (Boolean),
	XtOffsetOf (WmGlobalData, useDECMode),
        XtRImmediate,
        (caddr_t)True
    },
    {
        WmNforceAltSpace,
        WmCForceAltSpace,
        XtRBoolean,
        sizeof (Boolean),
	XtOffsetOf (WmGlobalData, forceAltSpace),
        XtRImmediate,
        (caddr_t)False
    },
    {
        WmNICCCMCompliant,
        WmCICCCMCompliant,
        XtRBoolean,
        sizeof (Boolean),
	XtOffsetOf (WmGlobalData, ICCCMCompliant), 
        XtRImmediate,
        (caddr_t)False                            
    },
    {
        WmNignoreModKeys,
        WmCIgnoreModKeys,
        XtRBoolean,
        sizeof (Boolean),
	XtOffsetOf (WmGlobalData, ignoreModKeys), 
        XtRImmediate,
        (caddr_t)False                            
    },
    {
        WmNignoreAllModKeys,
        WmCIgnoreAllModKeys,
        XtRBoolean,
        sizeof (Boolean),
	XtOffsetOf (WmGlobalData, ignoreAllModKeys), 
        XtRImmediate,
        (caddr_t)False                            
    },
    {
        WmNiconFullDepth,                 
        WmCIconFullDepth,
        XtRBoolean,
        sizeof (Boolean),
	XtOffsetOf (WmGlobalData, iconFullDepth), 
        XtRImmediate,
        (caddr_t)False                            
    },
#endif /* DEC_MOTIF_BUG_FIX */                     

}; /* END OF wmGlobalResources[] */


/*
 * These determine the screens to manage at startup.
 * These are broken out to enhance startup performance.
 */
XtResource wmGlobalScreenResources[] =
{
    {
	WmNmultiScreen,
	WmCMultiScreen,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, multiScreen),
	XtRImmediate,
	(caddr_t)False
    },

    {
	WmNscreens,
	WmCScreens,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, screenList),
	XtRImmediate,
	(caddr_t)NULL
    },
};



/******************************<->*************************************
 *
 *  wmStdGlobalResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm general appearance
 *  and behavior resources that are not automatically set for the standard
 *  (default) behavior.  These resources are specified with the following
 *  syntax:
 *
 *      "Mwm*<resource_identifier>".
 *
 ******************************<->***********************************/

XtResource wmStdGlobalResources[] =
{

    {
	WmNbitmapDirectory,
	WmCBitmapDirectory,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, bitmapDirectory),
	XtRString,
	(caddr_t)BITMAPDIR
    },

    {
	WmNconfigFile,
	WmCConfigFile,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, configFile),
	XtRImmediate,
	(caddr_t)NULL
    },

    {
	WmNiconAutoPlace,
	WmCIconAutoPlace,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, iconAutoPlace),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNmoveThreshold,
	WmCMoveThreshold,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, moveThreshold),
	XtRImmediate,
	(caddr_t)4
    },

    {
	WmNpositionIsFrame,
	WmCPositionIsFrame,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionIsFrame),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNpositionOnScreen,
	WmCPositionOnScreen,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionOnScreen),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNquitTimeout,
	WmCQuitTimeout,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, quitTimeout),
	XtRImmediate,
	(caddr_t)1000
    },

    {
	WmNshowFeedback,
	WmCShowFeedback,
	WmRShowFeedback,
	sizeof (int),
        XtOffsetOf(WmGlobalData, showFeedback),
	XtRImmediate,
	(caddr_t)(WM_SHOW_FB_DEFAULT)
    },

};


/******************************<->*************************************
 *
 *  wmScreenResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm screen specific
 *  appearance and behavior resources.  These resources are specified
 *  with the following syntax:
 *
 *      "Mwm*screen<#>*<resource_identifier>".
 *
 ******************************<->***********************************/

#ifdef DEC_MOTIF_EXTENSION
XtResource *mwm_screen_res = NULL;
#endif
XtResource wmScreenResources[] =
{
    {
	WmNbuttonBindings,
	WmCButtonBindings,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, buttonBindings),
	XtRString,
	(caddr_t)defaultButtonBindingsName
    },

    {
	WmNcleanText,
	WmCCleanText,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, cleanText),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNfeedbackGeometry,
	WmCFeedbackGeometry,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, feedbackGeometry),
	XtRString,
	(caddr_t)NULL
    },

    {
	WmNfadeNormalIcon,
	WmCFadeNormalIcon,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, fadeNormalIcon),
	XtRImmediate,
	(caddr_t)False
    },

    {
	WmNiconDecoration,
	WmCIconDecoration,
	WmRIconDecor,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconDecoration),
	XtRImmediate,
	(caddr_t)USE_ICON_DEFAULT_APPEARANCE
    },

    {
	WmNiconImageMaximum,
	WmCIconImageMaximum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMaximum),
	XtRString,
	"50x50"
    },

    {
	WmNiconImageMinimum,
	WmCIconImageMinimum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMinimum),
	XtRString,
	"16x16"
    },

    {
	WmNiconPlacement,
	WmCIconPlacement,
	WmRIconPlacement,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconPlacement),
	XtRImmediate,
	(caddr_t)(ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_BOTTOM_SECONDARY)
    },

    {
	WmNiconPlacementMargin,
	WmCIconPlacementMargin,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconPlacementMargin),
	XtRImmediate,
	(caddr_t)-1
    },

    {
	WmNkeyBindings,
	WmCKeyBindings,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, keyBindings),
	XtRString,
	(caddr_t)defaultKeyBindingsName
    },

    {
	WmNframeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, frameBorderWidth),
	XtRImmediate,
	(caddr_t) BIGSIZE
    },

    {
	WmNiconBoxGeometry,
	WmCIconBoxGeometry,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, iconBoxGeometry),
	XtRString,
	(caddr_t)NULL
    },

    {
	WmNiconBoxName,
	WmCIconBoxName,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, iconBoxName),
	XtRString,
	(caddr_t)"iconbox"
    },

    {
	WmNiconBoxSBDisplayPolicy,
	WmCIconBoxSBDisplayPolicy,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, iconBoxSBDisplayPolicy),
	XtRString,
	(caddr_t)"all"
    },

    {
	WmNiconBoxScheme,
	WmCIconBoxScheme,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconBoxScheme),
	XtRImmediate,
	(caddr_t)0
    },

    {
	WmNiconBoxTitle,
	WmCIconBoxTitle,
	XmRXmString,
	sizeof (XmString),
	XtOffsetOf (WmScreenData, iconBoxTitle),
	XmRXmString,
	(caddr_t)NULL
    },

    {
	WmNlimitResize,
	WmCLimitResize,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, limitResize),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNmaximumMaximumSize,
	WmCMaximumMaximumSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, maximumMaximumSize),
	XtRString,
	"0x0"
    },

    {
	WmNresizeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, resizeBorderWidth),
	XtRImmediate,
	(caddr_t) BIGSIZE
    },

    {
	WmNresizeCursors,
	WmCResizeCursors,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, resizeCursors),
	XtRImmediate,
	(caddr_t)True
    },

    {
	WmNtransientDecoration,
	WmCTransientDecoration,
	WmRClientDecor,
	sizeof (int),
	XtOffsetOf (WmScreenData, transientDecoration),
	XtRImmediate,
	(caddr_t)(WM_DECOR_SYSTEM | WM_DECOR_RESIZEH)
    },

    {
	WmNtransientFunctions,
	WmCTransientFunctions,
	WmRClientFunction,
	sizeof (int),
	XtOffsetOf (WmScreenData, transientFunctions),
	XtRImmediate,
	(caddr_t)(WM_FUNC_ALL & ~(MWM_FUNC_MAXIMIZE | MWM_FUNC_MINIMIZE))
    },


    {
	WmNuseIconBox,
	WmCUseIconBox,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, useIconBox),
	XtRImmediate,
	(caddr_t)False
    },

    {
	WmNmoveOpaque,
	WmCMoveOpaque,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, moveOpaque),
	XtRImmediate,
	(caddr_t)False

    }

};


/******************************<->*************************************
 *
 *  wmStdScreenResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm screen specific
 *  appearance and behavior resources that are not automatically set for 
 *  the standard (default) behavior.  These resources are specified with 
 *  the following syntax:
 *
 *      "Mwm*screen<#>*<resource_identifier>".
 *
 ******************************<->***********************************/

XtResource wmStdScreenResources[] =
{
    {
	WmNframeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, frameBorderWidth),
	XtRImmediate,
	(caddr_t) BIGSIZE
    },

    {
	WmNiconImageMaximum,
	WmCIconImageMaximum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMaximum),
	XtRString,
	"50x50"
    },

    {
	WmNiconImageMinimum,
	WmCIconImageMinimum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMinimum),
	XtRString,
	"32x32"
    },

    {
	WmNiconPlacementMargin,
	WmCIconPlacementMargin,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconPlacementMargin),
	XtRImmediate,
	(caddr_t)-1
    },

    {
	WmNmaximumMaximumSize,
	WmCMaximumMaximumSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, maximumMaximumSize),
	XtRString,
	"0x0"
    },

    {
	WmNresizeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, resizeBorderWidth),
	XtRImmediate,
	(caddr_t) BIGSIZE
    }
};



/******************************<->*************************************
 *
 *  wmWorkspaceResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm workspace 
 *  specific appearance and behavior resources.  These resources are 
 *  specified with the following syntax:
 *
 *      "Mwm*[screen<#>*]<workspace>*<resource_identifier>".
 *
 ******************************<->***********************************/
XtResource *wmWorkspaceResources = NULL;



/******************************<->*************************************
 *
 *  wmStdWorkspaceResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm workspace specific 
 *  appearance and behavior resources that are not automatically set for 
 *  the standard (default) behavior.  These resources are specified with 
 *  the following syntax:
 *
 *      "Mwm*[screen<#>*]<workspace>*<resource_identifier>".
 *
 *************************************<->***********************************/

XtResource *wmStdWorkspaceResources = NULL;



/*************************************<->*************************************
 *
 *  wmClientResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of client specific 
 *  window manager resources.  These resources are specified with the
 *  following syntax:
 *
 *      "Mwm*<client_name_or_class>*<resource_identifier>"
 *
 *************************************<->***********************************/

#ifdef DEC_MOTIF_EXTENSION
XtResource *mwm_client_res = NULL;
#endif
XtResource wmClientResources[] =
{

    {
	WmNclientDecoration,
	WmCClientDecoration,
	WmRClientDecor,
	sizeof (int),
	XtOffsetOf (ClientData, clientDecoration),
	XtRImmediate,
	(caddr_t)(WM_DECOR_DEFAULT)
    },

    {
	WmNclientFunctions,
	WmCClientFunctions,
	WmRClientFunction,
	sizeof (int),
	XtOffsetOf (ClientData, clientFunctions),
	XtRImmediate,
	(caddr_t)(WM_FUNC_DEFAULT)
    },

    {
	WmNfocusAutoRaise,
	WmCFocusAutoRaise,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, focusAutoRaise),
	XtRCallProc,
	(caddr_t)_WmFocusAutoRaiseDefault
    },

    {
	WmNiconImage,
	WmCIconImage,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImage),
	XtRString,
	(caddr_t)NULL
    },

    {
	WmNiconImageBackground,
	WmCIconImageBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageBackground),
	XtRCallProc,
	(caddr_t)_WmIconImageBDefault
    },

    {
	WmNiconImageForeground,
	WmCIconImageForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageForeground),
	XtRCallProc,
	(caddr_t)_WmIconImageFDefault
    },

    {
	WmNiconImageBottomShadowColor,
	WmCIconImageBottomShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageBottomShadowColor),
	XtRCallProc,
	(caddr_t)_WmIconImageBSCDefault
    },

    {
	WmNiconImageBottomShadowPixmap,
	WmCIconImageBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (ClientData, iconImageBottomShadowPStr),
	XtRCallProc,
	(caddr_t)_WmIconImageBSPDefault
    },

    {
	WmNiconImageTopShadowColor,
	WmCIconImageTopShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageTopShadowColor),
	XtRCallProc,
	(caddr_t)_WmIconImageTSCDefault
    },

    {
	WmNiconImageTopShadowPixmap,
	WmCIconImageTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImageTopShadowPStr),
	XtRCallProc,
	(caddr_t)_WmIconImageTSPDefault
    },

    {
	WmNmatteWidth,
	WmCMatteWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (ClientData, matteWidth),
	XtRImmediate,
	(caddr_t)0
    },

    {
	WmNmaximumClientSize,
	WmCMaximumClientSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (ClientData, maximumClientSize),
	XtRString,
	"0x0"
    },

    {
	WmNsystemMenu,
	WmCSystemMenu,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, systemMenu),
	XtRString,
	(caddr_t)defaultSystemMenuName
    },

    {
	WmNuseClientIcon,
	WmCUseClientIcon,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, useClientIcon),
	XtRImmediate,
	(caddr_t)False
    },

    {
	WmNusePPosition,
	WmCUsePPosition,
	WmRUsePPosition,
	sizeof (int),
	XtOffsetOf (ClientData, usePPosition),
	XtRImmediate,
	(caddr_t)(USE_PPOSITION_NONZERO)
    }

}; /* END OF STRUCTURE wmClientResources */



/*************************************<->*************************************
 *
 *  wmStdClientResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of client specific 
 *  window manager resources that are not automatically set for the standard
 *  (default) behavior.  These resources are specified with the
 *  following syntax:
 *
 *      "Mwm*<client_name_or_class>*<resource_identifier>"
 *
 *************************************<->***********************************/

XtResource wmStdClientResources[] =
{

    {
	WmNiconImage,
	WmCIconImage,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImage),
	XtRString,
	(caddr_t)NULL
    },

    {
	WmNiconImageBackground,
	WmCIconImageBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageBackground),
	XtRCallProc,
	(caddr_t)_WmIconImageBDefault
    },

    {
	WmNiconImageForeground,
	WmCIconImageForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageForeground),
	XtRCallProc,
	(caddr_t)_WmIconImageFDefault
    },

    {
	WmNiconImageBottomShadowColor,
	WmCIconImageBottomShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageBottomShadowColor),
	XtRCallProc,
	(caddr_t)_WmIconImageBSCDefault
    },

    {
	WmNiconImageBottomShadowPixmap,
	WmCIconImageBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (ClientData, iconImageBottomShadowPStr),
	XtRCallProc,
	(caddr_t)_WmIconImageBSPDefault
    },

    {
	WmNiconImageTopShadowColor,
	WmCIconImageTopShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageTopShadowColor),
	XtRCallProc,
	(caddr_t)_WmIconImageTSCDefault
    },

    {
	WmNiconImageTopShadowPixmap,
	WmCIconImageTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImageTopShadowPStr),
	XtRCallProc,
	(caddr_t)_WmIconImageTSPDefault
    },

    {
	WmNmatteWidth,
	WmCMatteWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (ClientData, matteWidth),
	XtRImmediate,
	(caddr_t)0
    },

    {
	WmNmaximumClientSize,
	WmCMaximumClientSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (ClientData, maximumClientSize),
	XtRString,
	"0x0"
    },

    {
	WmNuseClientIcon,
	WmCUseClientIcon,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, useClientIcon),
	XtRImmediate,
	(caddr_t)False
    }

};



/*************************************<->*************************************
 *
 *  wmClientResourcesM
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of client specific 
 *  window manager resources that affect the appearance of the client
 *  matte.  These resources are specified with the following syntax:
 *
 *      "Mwm*<client_name_or_class>*<resource_identifier>"
 *
 *************************************<->***********************************/

#ifdef DEC_MOTIF_EXTENSION
XtResource *mwm_client_resm = NULL;
#endif
XtResource wmClientResourcesM[] =
{
    {
	WmNmatteBackground,
	WmCMatteBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, matteBackground),
	XtRCallProc,
	(caddr_t)_WmMatteBDefault
    },

    {
	WmNmatteForeground,
	WmCMatteForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, matteForeground),
	XtRCallProc,
	(caddr_t)_WmMatteFDefault
    },

    {
	WmNmatteBottomShadowColor,
	WmCMatteBottomShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, matteBottomShadowColor),
	XtRCallProc,
	(caddr_t)_WmMatteBSCDefault
    },

    {
	WmNmatteBottomShadowPixmap,
	WmCMatteBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (ClientData, matteBottomShadowPStr),
	XtRCallProc,
	(caddr_t)_WmMatteBSPDefault
    },

    {
	WmNmatteTopShadowColor,
	WmCMatteTopShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, matteTopShadowColor),
	XtRCallProc,
	(caddr_t)_WmMatteTSCDefault
    },

    {
	WmNmatteTopShadowPixmap,
	WmCMatteTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, matteTopShadowPStr),
	XtRCallProc,
	(caddr_t)_WmMatteTSPDefault
    }
};



/*************************************<->*************************************
 *
 *  wmAppearanceResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of component appearance
 *  resources.  These resources are specified with the following syntax:
 *
 *      "Mwm*<resource_identifier>"
 *      "Mwm*client*<resource_identifier>"
 *      "Mwm*icon*<resource_identifier>"
 *      "Mwm*feedback*<resource_identifier>"
 *
 *************************************<->***********************************/

#ifdef DEC_MOTIF_EXTENSION
XtResource *mwm_appear_res = NULL;
#endif
XtResource wmAppearanceResources[] =
{

    {
	XmNfontList,
	XmCFontList,
	XmRFontList,
	sizeof (XmFontList),
	XtOffsetOf (AppearanceData, fontList),
	XtRString,
	"fixed"
    },

    {      
	WmNsaveUnder,
	WmCSaveUnder,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (AppearanceData, saveUnder),
	XtRImmediate,
	(caddr_t)False
    },

    {
	XtNbackground,
	XtCBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, background),
	XtRCallProc,
	(caddr_t)_WmBackgroundDefault
    },

    {
	XtNforeground,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (AppearanceData, foreground),
	XtRCallProc,
	(caddr_t)_WmForegroundDefault
    },

    {
	XmNbottomShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (AppearanceData, bottomShadowColor),
	XtRCallProc,
	(caddr_t)_WmBottomShadowColorDefault
    },

    {
	XmNbottomShadowPixmap,
	XmCBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, bottomShadowPStr),
	XtRString,
	(caddr_t)NULL
    },

    {
	XmNtopShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (AppearanceData, topShadowColor),
	XtRCallProc,
	(caddr_t)_WmTopShadowColorDefault
    },

    {
	XmNbackgroundPixmap,
	XmCBackgroundPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, backgroundPStr),
	XtRCallProc,
	(caddr_t)_WmBackgroundPixmapDefault
    },

    {
	XmNtopShadowPixmap,
	XmCTopShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, topShadowPStr),
	XtRCallProc,
	(caddr_t)_WmTopShadowPixmapDefault
    },

    {
	WmNactiveBackground,
	XtCBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeBackground),
	XtRCallProc,
	(caddr_t)_WmABackgroundDefault
    },

    {
	WmNactiveForeground,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeForeground),
	XtRCallProc,
	(caddr_t)_WmAForegroundDefault
    },

    {
	WmNactiveBottomShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeBottomShadowColor),
	XtRCallProc,
	(caddr_t)_WmABottomShadowColorDefault
    },

    {
	WmNactiveBottomShadowPixmap,
	XmCBottomShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (AppearanceData, activeBottomShadowPStr),
	XtRString,
	(caddr_t)NULL
    },

    {
	WmNactiveTopShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeTopShadowColor),
	XtRCallProc,
	(caddr_t)_WmATopShadowColorDefault
    },

    {
	WmNactiveBackgroundPixmap,
	XmCBackgroundPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, activeBackgroundPStr),
	XtRCallProc,
	(caddr_t)_WmABackgroundPixmapDefault
    },

    {
	WmNactiveTopShadowPixmap,
	XmCTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (AppearanceData, activeTopShadowPStr),
	XtRCallProc,
	(caddr_t)_WmATopShadowPixmapDefault
    }

};



/*************************************<->*************************************
 *
 *  _WmIconImageFDefault (widget, offset, value)
 *  _WmIconImageBDefault (widget, offset, value)
 *  _WmIconImageBSCDefault (widget, offset, value)
 *  _WmIconImageBSPDefault (widget, offset, value)
 *  _WmIconImageTSCDefault (widget, offset, value)
 *  _WmIconImageTSPDefault (widget, offset, value)
 *  _WmMatteFDefault (widget, offset, value)
 *  _WmMatteBDefault (widget, offset, value)
 *  _WmMatteBSCDefault (widget, offset, value)
 *  _WmMatteBSPDefault (widget, offset, value)
 *  _WmMatteTSCDefault (widget, offset, value)
 *  _WmMatteTSPDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  These functions are used to generate dynamic defaults for various
 *  client-specific appearance related resources.
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  _pCD = (static global) pointer to client data associated with resources
 *
 * 
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
_WmIconImageFDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmIconImageFDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmFGC, 0,
	_pCD->iconImageBackground, value);

} /* END OF FUNCTION _WmIconImageFDefault */

#ifdef _NO_PROTO
void 
_WmIconImageBDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmIconImageBDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    value->addr = (char *)&(_pCD->pSD->iconAppearance.background);
    value->size = sizeof (Pixel);

} /* END OF FUNCTION _WmIconImageBDefault */


#ifdef _NO_PROTO
void 
_WmIconImageBSCDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmIconImageBSCDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmBSC, 0,
	_pCD->iconImageBackground, value);

} /* END OF FUNCTION _WmIconImageBSCDefault */


#ifdef _NO_PROTO
void 
_WmIconImageBSPDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmIconImageBSPDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{

    value->addr = (char *)_pCD->pSD->iconAppearance.bottomShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmIconImageBSCDefault */


#ifdef _NO_PROTO
void 
_WmIconImageTSCDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmIconImageTSCDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmTSC, 0,
	_pCD->iconImageBackground, value);

} /* END OF FUNCTION _WmIconImageTSCDefault */


#ifdef _NO_PROTO
void 
_WmIconImageTSPDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmIconImageTSPDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{

    value->addr = (char *)_pCD->pSD->iconAppearance.topShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmIconImageTSPDefault */


#ifdef _NO_PROTO
void 
_WmMatteFDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmMatteFDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmFGC, 0,
	_pCD->matteBackground, value);

} /* END OF FUNCTION _WmMatteFDefault */


#ifdef _NO_PROTO
void 
_WmMatteBDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmMatteBDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    value->addr = (char *)&(_pCD->pSD->clientAppearance.background);
    value->size = sizeof (Pixel);

} /* END OF FUNCTION _WmMatteBDefault */


#ifdef _NO_PROTO
void 
_WmMatteBSCDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmMatteBSCDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmBSC, 0,
	_pCD->matteBackground, value);

} /* END OF FUNCTION _WmMatteBSCDefault */


#ifdef _NO_PROTO
void 
_WmMatteBSPDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmMatteBSPDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{

    value->addr = (char *)_pCD->pSD->clientAppearance.bottomShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmMatteBSCDefault */


#ifdef _NO_PROTO
void 
_WmMatteTSCDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmMatteTSCDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmTSC, 0,
	_pCD->matteBackground, value);

} /* END OF FUNCTION _WmMatteTSCDefault */


#ifdef _NO_PROTO
void 
_WmMatteTSPDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmMatteTSPDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{

    value->addr = (char *)_pCD->pSD->clientAppearance.topShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmMatteTSCDefault */



/*************************************<->*************************************
 *
 *  _WmBackgroundDefault (widget, offset, value)
 *  _WmForegroundDefault (widget, offset, value)
 *  _WmBackgroundPixmapDefault (widget, offset, value)
 *  _WmBottomShadowColorDefault (widget, offset, value)
 *  _WmTopShadowColorDefault (widget, offset, value)
 *  _WmTopShadowPixmapDefault (widget, offset, value)
 *  _WmABackgroundDefault (widget, offset, value)
 *  _WmAForegroundDefault (widget, offset, value)
 *  _WmABackgroundPixmapDefault (widget, offset, value)
 *  _WmABottomShadowColorDefault (widget, offset, value)
 *  _WmRFBackgroundDefault (widget, offset, value)
 *  _WmRFForegroundDefault (widget, offset, value)
 *  _WmATopShadowColorDefault (widget, offset, value)
 *  _WmATopShadowPixmapDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  These functions are used to generate dynamic defaults for various
 *  component appearance related resources (not client-specific).
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  _defaultBackground = (static global) default background color (inactive)
 *
 *  _defaultActiveBackground = (static global) default bg color (active)
 *
 *  _pAppearanceData = (static global) pointer to resouce set structure
 *
 * 
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
_WmBackgroundDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmBackgroundDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmBGC, _defaultBackground, 0, value);

} /* END OF FUNCTION _WmBackgroundDefault */


#ifdef _NO_PROTO
void 
_WmForegroundDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmForegroundDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmFGC, 0, _pAppearanceData->background,
	value);

} /* END OF FUNCTION _WmForegroundDefault */


#ifdef _NO_PROTO
void 
_WmBackgroundPixmapDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmBackgroundPixmapDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    static String string;


    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->topShadowColor == _pAppearanceData->background))
    {
	string = _25_foreground;
    }
    else
    {
	string = NULL;
    }

    value->addr = (char *)string;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmBackgroundPixmapDefault */


#ifdef _NO_PROTO
void 
_WmBottomShadowColorDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmBottomShadowColorDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmBSC, 0, _pAppearanceData->background,
	value);

} /* END OF FUNCTION _WmBottomShadowColorDefault */


#ifdef _NO_PROTO
void 
_WmTopShadowColorDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmTopShadowColorDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmTSC, 0, _pAppearanceData->background,
	value);

} /* END OF FUNCTION _WmTopShadowColorDefault */


#ifdef _NO_PROTO
void 
_WmTopShadowPixmapDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmTopShadowPixmapDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    static String string;


    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->topShadowColor == _pAppearanceData->background))
    {
	/* Fix monochrome 3D appearance */
	string = _50_foreground;
	if (_pAppearanceData->backgroundPStr != NULL)
	    if (!strcmp(_pAppearanceData->backgroundPStr, _25_foreground) ||
		!strcmp(_pAppearanceData->backgroundPStr, _50_foreground))
	    {
		string = _foreground;
	    }
    }
    else
    {
	string = NULL;
    }

    value->addr = (char *)string;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmTopShadowPixmapDefault */


#ifdef _NO_PROTO
void 
_WmABackgroundDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmABackgroundDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmBGC, _defaultActiveBackground, 0, value);

} /* END OF FUNCTION _WmABackgroundDefault */


#ifdef _NO_PROTO
void 
_WmAForegroundDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmAForegroundDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmFGC, 0, _pAppearanceData->activeBackground,
	value);

} /* END OF FUNCTION _WmAForegroundDefault */

#ifdef _NO_PROTO
void 
_WmABackgroundPixmapDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmABackgroundPixmapDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    static String string;


    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->activeTopShadowColor ==
	 				_pAppearanceData->activeBackground))
    {
	string = _50_foreground;
    }
    else
    {
	string = NULL;
    }

    value->addr = (char *)string;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmABackgroundPixmapDefault */

#ifdef _NO_PROTO
void 
_WmABottomShadowColorDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmABottomShadowColorDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmBSC, 0, _pAppearanceData->activeBackground,
	value);

} /* END OF FUNCTION _WmABottomShadowColorDefault */


#ifdef _NO_PROTO
void 
_WmATopShadowColorDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmATopShadowColorDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    _WmGetDynamicDefault (widget, WmTSC, 0, _pAppearanceData->activeBackground,
	value);

} /* END OF FUNCTION _WmATopShadowColorDefault */


#ifdef _NO_PROTO
void 
_WmATopShadowPixmapDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmATopShadowPixmapDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    static String string;

    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->activeTopShadowColor ==
	                             _pAppearanceData->activeBackground))
    {
	/* Fix monochrome 3D appearance */
	string = _50_foreground;
	if (_pAppearanceData->activeBackgroundPStr != NULL)
	    if (!strcmp
		    (_pAppearanceData->activeBackgroundPStr, _25_foreground) ||
		!strcmp
		    (_pAppearanceData->activeBackgroundPStr, _50_foreground))
	    {
		string = _foreground;
	    }
    }
    else
    {
	string = NULL;
    }
    
    value->addr = (char *)string;
    value->size = sizeof (String);
    
} /* END OF FUNCTION _WmATopShadowPixmapDefault */




/*************************************<->*************************************
 *
 *  _WmFocusAutoRaiseDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a default value for the focusAutoRaise resource.
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 * 
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
_WmFocusAutoRaiseDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmFocusAutoRaiseDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    static Boolean focusAutoRaise;

    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	focusAutoRaise = True;
    }
    else
    {
	focusAutoRaise = False;
    }

    value->addr = (char *)&focusAutoRaise;
    value->size = sizeof (Boolean);

} /* END OF FUNCTION _WmFocusAutoRaiseDefault */


/*************************************<->*************************************
 *
 *  _WmMultiClickTimeDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a default value for the doubleClickTime resource.
 *  We dynamically default to the XtR4 multiClickTime value.
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
_WmMultiClickTimeDefault (widget, offset, value)
	Widget	 widget;
	int	 offset;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmMultiClickTimeDefault (Widget widget, int offset, XrmValue *value)
#endif /* _NO_PROTO */
{
    static int multiClickTime;

    multiClickTime = XtGetMultiClickTime(XtDisplay(widget));

    value->addr = (char *)&multiClickTime;
    value->size = sizeof (int);

} /* END OF FUNCTION _WmMultiClickTimeDefault */





/******************************<->*************************************
 *
 *  ProcessWmResources ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve and process window manager resources
 *  that are not client-specific.
 *
 *
 *  Inputs:
 *  ------
 *  wmGlobalResources = pointer to wm resource list
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (global data filled out with resource values)
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
ProcessWmResources ()

#else /* _NO_PROTO */
void 
ProcessWmResources (void)
#endif /* _NO_PROTO */
{

    /*
     * Process the mwm general appearance and behavior resources.  Retrieve
     * a limited set of resource values if the window manager is starting
     * up with the standard behavior.
     */

    if (wmGD.useStandardBehavior)
    {                                             
	XtGetApplicationResources (wmGD.topLevelW, (XtPointer) &wmGD,
	    wmStdGlobalResources, XtNumber (wmStdGlobalResources), NULL, 0);

	/*
	 * Fill in the standard resource values.
	 */

	SetStdGlobalResourceValues ();
    }
    else
    {                                    
	XtGetApplicationResources (wmGD.topLevelW, (XtPointer) &wmGD,
	    wmGlobalResources, XtNumber (wmGlobalResources), NULL, 0);
    }

    wmGD.ignoreLockMod = True;		/* !!! use a resource? !!! */

#ifdef DEC_MOTIF_EXTENSION
    {
	KeySym numlock = XStringToKeysym("Num_Lock");
	if (( numlock != NoSymbol ) && ( IsModifierKey(numlock))) {
	    wmGD.ignoreNumLockMod = True;
	}
    }
#endif
} /* END OF FUNCTION ProcessWmResources */



/******************************<->*************************************
 *
 *  ProcessGlobalScreenResources ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve window manager resources to 
 *  determine the screens to manage.
 *
 *
 *  Inputs:
 *  ------
 *  wmGlobalScreenResources = pointer to wm resource list
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (global data filled out with resource values)
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
ProcessGlobalScreenResources ()

#else /* _NO_PROTO */
void 
ProcessGlobalScreenResources (void)
#endif /* _NO_PROTO */
{
    XtGetApplicationResources (wmGD.topLevelW, (XtPointer)&wmGD,
	wmGlobalScreenResources, 
	XtNumber (wmGlobalScreenResources), NULL, 0);

    if (wmGD.multiScreen)
    {
        wmGD.numScreens = ScreenCount(DISPLAY);
    }
    else
    {
	wmGD.numScreens = 1;
    }

    if (wmGD.screenList != NULL)
    {
	ProcessScreenListResource();
    }
}



/*************************************<->*************************************
 *
 *  SetStdGlobalResourceValues ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets resource data to standard values.  This setting
 *  is done in place of getting the values from the user settings in
 *  the resource database.
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (global data filled out with resource values)
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
SetStdGlobalResourceValues ()

#else /* _NO_PROTO */
void 
SetStdGlobalResourceValues (void)
#endif /* _NO_PROTO */
{
    wmGD.autoKeyFocus = True;
#ifdef DEC_MOTIF_EXTENSION
    wmGD.clientAutoPlace = False;
    wmGD.positionIsFrame = False;
#else
    wmGD.clientAutoPlace = True;
#endif
    wmGD.colormapFocusPolicy = CMAP_FOCUS_KEYBOARD;
    wmGD.deiconifyKeyFocus = True;
    wmGD.doubleClickTime = 500;
    wmGD.freezeOnConfig = True;
    wmGD.iconAutoPlace = True;
    wmGD.iconClick = True;
    wmGD.interactivePlacement = False;
    wmGD.keyboardFocusPolicy = KEYBOARD_FOCUS_EXPLICIT;
    wmGD.lowerOnIconify = True;
    wmGD.passSelectButton = True;
    wmGD.startupKeyFocus = True;
    wmGD.systemButtonClick = True;
#ifdef DEC_MOTIF_EXTENSION
    wmGD.raiseKeyFocus = True;
    wmGD.systemButtonClick2 = False;
#else
    wmGD.systemButtonClick2 = True;
#endif

} /* END OF FUNCTION SetStdGlobalResourceValues */



/*************************************<->*************************************
 *
 *  ProcessScreenListResource ()
 *
 *
 *  Description:
 *  -----------
 *  This processes the names in the screenList resource.
 *
 *
 *  Inputs:
 *  ------
 *  wmGlobalResources = pointer to wmGD.screenList 
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.screenNames
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
ProcessScreenListResource ()

#else /* _NO_PROTO */
void 
ProcessScreenListResource (void)
#endif /* _NO_PROTO */
{
    unsigned char *lineP;
    unsigned char *string;
    int sNum = 0;
    int nameCount = 0;

    lineP = (unsigned char *)wmGD.screenList;

    /*
     *  Parse screenList. 
     */
    while (((string = GetString(&lineP)) != NULL) && 
	   (sNum < ScreenCount(DISPLAY)))
    {
	if (!(wmGD.screenNames[sNum] = (unsigned char *) 
	    WmRealloc ((char*)wmGD.screenNames[sNum], strlen((char*)string)+1)))
	{
	    exit(WM_ERROR_EXIT_VALUE);
	}
	else 
	{
	    strcpy((char *)wmGD.screenNames[sNum], (char *)string);
	    nameCount++;
	    sNum++;
	}
    }

    /*
     * If the number of listed screens (sNum) is < screen count, fill in the 
     * remaining screen names with the name of the first screen specified,
     * if such exists.
     */
    if (nameCount > 0)
    {
	string = wmGD.screenNames[0];    /* name of the first screen */
	while (sNum < ScreenCount(DISPLAY))
	{
	    if (!(wmGD.screenNames[sNum] = (unsigned char *) 
		WmRealloc ((char*)wmGD.screenNames[sNum], 
				strlen((char *)string)+1)))
	    {
		exit(WM_ERROR_EXIT_VALUE);
	    }
	    else 
	    {
		strcpy((char *)wmGD.screenNames[sNum], (char *)string);
		sNum++;
	    }
	}
    }

	
} /* END OF FUNCTION ProcessScreenListResource */




/******************************<->*************************************
 *
 *  ProcessAppearanceResources (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Retrieve and process the general appearance resources for the mwm
 *  subparts: "client", "icon", and "feedback"
 *
 *
 *  Inputs:
 *  ------
 *  pSD   = pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  modifies parts of global data wmGD.
 *
 *  Comments:
 *  --------
 *  o Changeable GCs are created with XCreateGC. The base GCs used for
 *    text output will have clip_masks defined for them later.
 *  
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void                         
ProcessAppearanceResources (pSD)

	WmScreenData *pSD;

#else /* _NO_PROTO */
void 
ProcessAppearanceResources (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    Widget clientW;		/* dummy widget for resource fetching */
    int i;
    Arg args[10];


    /*
     * Get the client subpart resources:
     */
                                    
    /* save info in static globals for dynamic default processing */
    _defaultBackground = _defaultColor1;
    _defaultActiveBackground = _defaultColor2;
    _pAppearanceData = &(pSD->clientAppearance);

#ifdef DEC_MOTIF_EXTENSION
    /* Copy the resource list since it may be modified by XtGetSubResources. */
    if (( mwm_appear_res == NULL ) && !ONE_SCREEN )
      {
        mwm_alloc( &mwm_appear_res, sizeof( wmAppearanceResources ), "Allocating resources " );
        memcpy( mwm_appear_res, wmAppearanceResources, sizeof( wmAppearanceResources ) );
      }
#endif /* DEC_MOTIF_EXTENSION */
    (void)XtGetSubresources (pSD->screenTopLevelW, 
	      (caddr_t) &(pSD->clientAppearance),
	      WmNclient, WmCClient, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef DEC_MOTIF_EXTENSION
    /* Reset all the subresources */
    if ( !ONE_SCREEN )
        mwm_subres_get( pSD, mwm_appear_res, XtNumber( wmAppearanceResources ), 
                        (caddr_t)&(pSD->clientAppearance) );
#endif
    /*
     * Process the client resource values:
     */

    /* make background, top and bottom shadow pixmaps */

    MakeAppearanceResources (pSD, &(pSD->clientAppearance), True);


    /*
     * Get the client.title subpart resources:
     */

	/* insert "client" widget in hierarchy */

    i = 0;
    clientW = XtCreateWidget (WmNclient, xmRowColumnWidgetClass, 
			pSD->screenTopLevelW, (ArgList) args, i);


	/* fetch "client.title" subpart appearance resources */

    _pAppearanceData = &(pSD->clientTitleAppearance);

    (void)XtGetSubresources (clientW, (caddr_t) &(pSD->clientTitleAppearance),
	      WmNtitle, WmCTitle, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef DEC_MOTIF_EXTENSION
    /* Reset all the subresources */
    if ( !ONE_SCREEN )
        mwm_subres_get( pSD, mwm_appear_res, XtNumber( wmAppearanceResources ), 
                        (caddr_t)&(pSD->clientTitleAppearance) );
#endif


    /*
     * Process the client.title resource values:
     */


    /* 
     * check if client title appearance is different from the rest of frame.
     */
    if (SimilarAppearanceData (&(pSD->clientAppearance), 
			       &(pSD->clientTitleAppearance)))
    {
        /* title bar doesn't need special graphic processing */
	pSD->decoupleTitleAppearance = False;
    }
    else 
    {
	/* make background, top and bottom shadow pixmaps */
	MakeAppearanceResources (pSD, &(pSD->clientTitleAppearance), True);
	pSD->decoupleTitleAppearance = True;
    }

    XtDestroyWidget (clientW);	/* all done with dummy widget */


    /*
     * Get the icon subpart resources:
     */                             

    _pAppearanceData = &(pSD->iconAppearance);

    (void)XtGetSubresources (pSD->screenTopLevelW, 
	      (caddr_t) &(pSD->iconAppearance),
	      WmNicon, WmCIcon, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef DEC_MOTIF_EXTENSION
    /* Reset all the subresources */
    if ( !ONE_SCREEN )
        mwm_subres_get( pSD, mwm_appear_res, XtNumber( wmAppearanceResources ), 
                        (caddr_t)&(pSD->iconAppearance) );
#endif


    /*         
     * Process the icon resource values:
     */

    /* make background, top and bottom shadow pixmaps */

    MakeAppearanceResources (pSD, &(pSD->iconAppearance), True);

                             
    /*
     * Get the feedback subpart resources:
     * !!! only get "inactive" resources !!!
     */

    _defaultBackground = _defaultColor2;
    _defaultActiveBackground = _defaultColor2;
    _pAppearanceData = &(pSD->feedbackAppearance);

    (void)XtGetSubresources (pSD->screenTopLevelW, 
	      (caddr_t) &(pSD->feedbackAppearance),
	      WmNfeedback, WmCFeedback, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef DEC_MOTIF_EXTENSION
    /* Reset all the subresources */
    if ( !ONE_SCREEN )
        mwm_subres_get( pSD, mwm_appear_res, XtNumber( wmAppearanceResources ), 
                        (caddr_t)&(pSD->feedbackAppearance) );
#endif

    /*
     * Process the feedback resource values:
     */

    /* make background, top and bottom shadow pixmaps */

    MakeAppearanceResources (pSD, &(pSD->feedbackAppearance), False);


} /* END OF FUNCTION ProcessAppearanceResources */

#ifndef NO_MULTIBYTE
#ifdef _NO_PROTO
static unsigned int
WmFontListFontHeight(fontlist, findex)
     XmFontList fontlist;
     short   findex;
#else
static unsigned int
WmFontListFontHeight(XmFontList fontlist, short findex)
#endif /* _NO_PROTO */
{

#define FontSetAscent(xfontsetextents) \
    ( -((xfontsetextents)->max_logical_extent.y))
#define FontSetDescent(xfontsetextents) \
      (((xfontsetextents)->max_logical_extent.height) \
       + ((xfontsetextents)->max_logical_extent.y))

    XmFontListEntry	fontEntry;
    XmFontContext	fontContext;
    XmFontType		fontType;
    XtPointer		fontPointer;
    Boolean		contextIsValid;
    int			i;

    contextIsValid = XmFontListInitFontContext(&fontContext, fontlist);

    for (i=0; i<=findex; i++)
    {
	fontEntry = XmFontListNextEntry(fontContext);
    }

    fontPointer = XmFontListEntryGetFont(fontEntry, &fontType);

    if (contextIsValid)
      XmFontListFreeFontContext(fontContext);

    
    if (fontType == XmFONT_IS_FONTSET)
    {
	XFontSetExtents *extents;

	extents = XExtentsOfFontSet((XFontSet)fontPointer);

	if (extents != NULL)
	{
	    return(FontSetAscent(extents) + FontSetDescent(extents));
	}
	else return(0);
    }
    else if (fontType == XmFONT_IS_FONT)
    {
	return(((XFontStruct *)fontPointer)->ascent +
	       ((XFontStruct *)fontPointer)->descent);
    }
    else return(0);
}
#endif


/*************************************<->*************************************
 *
 *  MakeAppearanceResources (pSD, pAData, makeActiveResources)
 *
 *
 *  Description:
 *  -----------
 *  This function makes top, bottom and background pixmaps for a window
 *  manager component.  Inactive and active (if specified) GC's are
 *  also made.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *
 *  pAData = pointer to appearance data structure containing resource info
 *
 *  makeActiveResources = if True then make active resources
 * 
 *  Outputs:
 *  -------
 *  *pAData = pixmap and GC fields filled out
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
MakeAppearanceResources (pSD, pAData, makeActiveResources)
	WmScreenData *pSD;
	AppearanceData *pAData;
	Boolean makeActiveResources;

#else /* _NO_PROTO */
void 
MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources)
#endif /* _NO_PROTO */
{
    short findex;
    Pixel foreground;


    /*
     * Extract a font from the font list.
     */

    _XmFontListSearch (pAData->fontList, XmFONTLIST_DEFAULT_TAG, &findex,
	&(pAData->font));

    if (!pAData->font) 
    {
	char msg[130];

	sprintf(msg, "failed to load font: %.100s\0", pAData->fontList);
	Warning(msg);
	exit(WM_ERROR_EXIT_VALUE);
    }

#ifndef NO_MULTIBYTE
    /*
     *  Calculate title bar's height and store it in pAData.
     */
    if (findex < 0 )
    {
	char msg[130];
	sprintf(msg, "cannot find an appropriate font: %.100s\0", pAData->fontList);
	Warning(msg);
        pAData->font = XLoadQueryFont(DISPLAY,"fixed");
        if (pAData->font)
                pAData->titleHeight = (pAData->font)->ascent
                                        + (pAData->font)->descent
                                        + WM_TITLE_BAR_PADDING;
        else pAData->titleHeight = 16;
    }
    else
    {
        pAData->titleHeight = WmFontListFontHeight(pAData->fontList, findex)
                                        + WM_TITLE_BAR_PADDING;
    }
#endif


    /*
     * Make standard (inactive) appearance resources.
     */

    /* background pixmap */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pAData->backgroundPStr ))
#else
    if (pAData->backgroundPStr) 
#endif /* DEC_MOTIF_EXTENSION */
    {
	pAData->backgroundPixmap = XmGetPixmap (
			               ScreenOfDisplay (DISPLAY, 
					   pSD->screen),
				       pAData->backgroundPStr,
				       pAData->foreground,
				       pAData->background);

	if (pAData->backgroundPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->backgroundPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->backgroundPixmap = (Pixmap)NULL;
    }

    /* top shadow pixmap */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pAData->topShadowPStr ))
#else
    if (pAData->topShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
    {
	/*
	 * Make sure top shadow color is not the same as background
	 * otherwise the wrong pixmap will be generated.
	 */
	if (pAData->topShadowColor != pAData->background)
	    foreground = pAData->topShadowColor;
	else
	    foreground = pAData->foreground;
	pAData->topShadowPixmap = XmGetPixmap (
			               ScreenOfDisplay (DISPLAY,
					   pSD->screen),
				       pAData->topShadowPStr,
				       foreground,
				       pAData->background);

	if (pAData->topShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->topShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->topShadowPixmap = (Pixmap)NULL;
    }


    /* bottom shadow pixmap */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pAData->bottomShadowPStr ))
#else
    if (pAData->bottomShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
    {
	/*
	 * Make sure bottom shadow color is not the same as background
	 * otherwise the wrong pixmap will be generated.
	 */
	if (pAData->bottomShadowColor != pAData->background)
	    foreground = pAData->bottomShadowColor;
	else
	    foreground = pAData->foreground;
	pAData->bottomShadowPixmap = XmGetPixmap (
			               ScreenOfDisplay (DISPLAY,
					   pSD->screen),
				       pAData->bottomShadowPStr,
				       foreground,
				       pAData->background);

	if (pAData->bottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->bottomShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->bottomShadowPixmap = (Pixmap)NULL;
    }

    /* inactive appearance GC */

    GetAppearanceGCs (pSD,
		      pAData->foreground,
		      pAData->background,
		      pAData->font,
		      pAData->backgroundPixmap,
		      pAData->topShadowColor,
		      pAData->topShadowPixmap,
		      pAData->bottomShadowColor,
		      pAData->bottomShadowPixmap,
		      &(pAData->inactiveGC),
		      &(pAData->inactiveTopShadowGC),
		      &(pAData->inactiveBottomShadowGC));



    /*
     * Make active apppearance resources if specified.
     */

    if (!makeActiveResources)
    {
	return;
    }

    /* active background pixmap */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pAData->activeBackgroundPStr ))
#else
    if (pAData->activeBackgroundPStr)
#endif /* DEC_MOTIF_EXTENSION */
    {
	pAData->activeBackgroundPixmap = XmGetPixmap (
			                     ScreenOfDisplay (DISPLAY,
						 pSD->screen),
				             pAData->activeBackgroundPStr,
				             pAData->activeForeground,
				             pAData->activeBackground);

	if (pAData->activeBackgroundPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->activeBackgroundPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->activeBackgroundPixmap = (Pixmap)NULL;
    }

    /* active top shadow pixmap */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pAData->activeTopShadowPStr ))
#else
    if (pAData->activeTopShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
    {
	pAData->activeTopShadowPixmap = XmGetPixmap (
			                    ScreenOfDisplay (DISPLAY,
						pSD->screen),
				            pAData->activeTopShadowPStr,
				            pAData->activeTopShadowColor,
				            pAData->activeBackground);

	if (pAData->activeTopShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->activeTopShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->activeTopShadowPixmap = (Pixmap)NULL;
    }


    /* active bottom shadow pixmap */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pAData->activeBottomShadowPStr ))
#else
    if (pAData->activeBottomShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
    {
	pAData->activeBottomShadowPixmap = XmGetPixmap (
			                       ScreenOfDisplay (DISPLAY,
						   pSD->screen),
				               pAData->activeBottomShadowPStr,
				               pAData->activeBottomShadowColor,
				               pAData->activeBackground);

	if (pAData->activeBottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->activeBottomShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->activeBottomShadowPixmap = (Pixmap)NULL;
    }

    /* inactive appearance GC */

    GetAppearanceGCs (pSD,
		      pAData->activeForeground,
		      pAData->activeBackground,
		      pAData->font,
		      pAData->activeBackgroundPixmap,
		      pAData->activeTopShadowColor,
		      pAData->activeTopShadowPixmap,
		      pAData->activeBottomShadowColor,
		      pAData->activeBottomShadowPixmap,
		      &(pAData->activeGC),
		      &(pAData->activeTopShadowGC),
		      &(pAData->activeBottomShadowGC));


} /* END OF FUNCTION MakeAppearanceResources */



/*************************************<->*************************************
 *
 *  GetAppearanceGCs (pSD, fg, bg, font, bg_pixmap, ts_color, 
 *                    ts_pixmap, bs_color, bs_pixmap, pGC, ptsGC, pbsGC)
 *
 *
 *  Description:
 *  -----------
 *  Creates the appearance GCs for any of the icon, client, or feedback 
 *  resources.
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  fg		- base foreground color
 *  bg		- base background color
 *  font	- font
 *  bg_pixmap	- background pixmap
 *  ts_color	- top shadow color
 *  ts_pixmap	- top shadow pixmap
 *  bs_color	- bottom shadow color
 *  bs_pixmap	- bottom shadow pixmap
 *  pGC		- pointer to location to receive base GC
 *  ptsGC	- pointer to location to receive top shadow GC
 *  pbsGC	- pointer to location to receive bottom shadow GC
 * 
 *  Outputs:
 *  -------
 *  *pGC	- base GC
 *  *ptsGC	- top shadow GC
 *  *pbsGC	- bottom shadow GC
 *  
 *
 *  Comments:
 *  --------
 * 
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
GetAppearanceGCs (pSD, fg, bg, font, bg_pixmap, ts_color, ts_pixmap, bs_color, bs_pixmap, pGC, ptsGC, pbsGC)
	WmScreenData *pSD;
	Pixel fg, bg, ts_color, bs_color;
	Pixmap bg_pixmap, ts_pixmap, bs_pixmap;
	XFontStruct *font;
	GC *pGC, *ptsGC, *pbsGC;

#else /* _NO_PROTO */
void 
GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, XFontStruct *font, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC)
#endif /* _NO_PROTO */
{
    XGCValues gcv;
    XtGCMask  mask;


    /*
     * Get base GC
     */

    mask = GCForeground | GCBackground | GCFont;
    gcv.foreground = fg;
    gcv.background = bg;
    gcv.font = font->fid;

    if (bg_pixmap)
    {
	mask |= GCTile;
	gcv.tile = bg_pixmap;
    }

    *pGC = XCreateGC (DISPLAY, pSD->rootWindow, mask, &gcv);

    /*
     * !!! Need GC error detection !!!
     */

    *ptsGC = GetHighlightGC (pSD, ts_color, bg, ts_pixmap);

    *pbsGC = GetHighlightGC (pSD, bs_color, bg, bs_pixmap);

} /* END OF FUNCTION GetAppearanceGCs */




/*************************************<->*************************************
 *
 *  ProcessScreenResources (pSD, screenName)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves resources that are screen specific.  If the
 *  window manager is providing standard behavior then retrieve the limited
 *  set of resources that don't affect standard behavior and set the
 *  values of the other resources to the standard values.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  screenName = name of screen
 *
 * 
 *  Outputs:
 *  -------
 *  pSD = resource data for screen is set
 *
 *
 *  Comments:
 *  --------
 *  o Gets subresources based on workspace name
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void         
ProcessScreenResources (pSD, screenName)
	WmScreenData *pSD;
	unsigned char *screenName;

#else /* _NO_PROTO */
void 
ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName)
#endif /* _NO_PROTO */
{          
#ifdef DEC_MOTIF_EXTENSION
char *image_bits;
int image_size;
#endif
    /*
     * Retrieve screen specific resources.
     */

    if (wmGD.useStandardBehavior)
    {
	XtGetSubresources (wmGD.topLevelW, (caddr_t) pSD, 
	    (String) screenName, (String)screenName, wmStdScreenResources, 
	    XtNumber (wmStdScreenResources), NULL, 0);

	/*
	 * Fill in the standard resource values.
	 */                        

	SetStdScreenResourceValues (pSD);
    }
    else
    {
#ifdef DEC_MOTIF_EXTENSION
        if (( mwm_screen_res == NULL ) && !ONE_SCREEN )
          {
            /* Copy the resource list since it may be modified by XtGetSubResources. */
            mwm_alloc( &mwm_screen_res, sizeof( wmScreenResources ), "Allocating resources " );
            memcpy( mwm_screen_res, wmScreenResources, sizeof( wmScreenResources ) );
          }
#endif /* DEC_MOTIF_EXTENSION */
	XtGetSubresources (wmGD.topLevelW, (caddr_t) pSD, 
	    (String)screenName, (String)screenName, wmScreenResources, 
	    XtNumber (wmScreenResources), NULL, 0);

#ifndef MOTIF_ONE_DOT_ONE
	pSD->moveOpaque = _XmGetMoveOpaqueByScreen(ScreenOfDisplay(DISPLAY,pSD->screen));
#endif
#ifdef DEC_MOTIF_EXTENSION
        /* Reset all the subresources */
        if ( !ONE_SCREEN )
            mwm_subres_get( pSD, mwm_screen_res, XtNumber( wmScreenResources ), 
                            (caddr_t)pSD );
#endif
    }

    /*
     * Do some additional processing on the window manager resource values.
     */


    if (pSD->iconImageMinimum.width < ICON_IMAGE_MIN_WIDTH)
    {
	pSD->iconImageMinimum.width = ICON_IMAGE_MIN_WIDTH;
    }
    else if (pSD->iconImageMinimum.width > ICON_IMAGE_MAX_WIDTH)
    {
	pSD->iconImageMinimum.width = ICON_IMAGE_MAX_WIDTH;
    }

    if (pSD->iconImageMinimum.height < ICON_IMAGE_MIN_HEIGHT)
    {
	pSD->iconImageMinimum.height = ICON_IMAGE_MIN_HEIGHT;
    }
    else if (pSD->iconImageMinimum.height > ICON_IMAGE_MAX_HEIGHT)
    {
	pSD->iconImageMinimum.height = ICON_IMAGE_MAX_HEIGHT;
    }

    if (pSD->iconImageMaximum.width < pSD->iconImageMinimum.width)
    {
	pSD->iconImageMaximum.width = pSD->iconImageMinimum.width;
    }
    else if (pSD->iconImageMaximum.width > ICON_IMAGE_MAX_WIDTH)
    {
	pSD->iconImageMaximum.width = ICON_IMAGE_MAX_WIDTH;
    }

    if (pSD->iconImageMaximum.height < pSD->iconImageMinimum.height)
    {
	pSD->iconImageMaximum.height = pSD->iconImageMinimum.height;
    }
    else if (pSD->iconImageMaximum.height > ICON_IMAGE_MAX_HEIGHT)
    {
	pSD->iconImageMaximum.height = ICON_IMAGE_MAX_HEIGHT;
    }

    if (pSD->iconPlacementMargin > MAXIMUM_ICON_MARGIN)
    {
	pSD->iconPlacementMargin = MAXIMUM_ICON_MARGIN;
    }

    if (pSD->maximumMaximumSize.width <= 0)
    {
	pSD->maximumMaximumSize.width =
			2 * DisplayWidth (DISPLAY, pSD->screen);
    }

    if (pSD->maximumMaximumSize.height <= 0)
    {
	pSD->maximumMaximumSize.height =
			2 * DisplayHeight (DISPLAY, pSD->screen);
    }

    /*
     * Set the icon appearance default based on whether or not the icon box
     * is being used.
     */

    if (pSD->iconDecoration & USE_ICON_DEFAULT_APPEARANCE)
    {
	if (pSD->useIconBox)
	{
	    pSD->iconDecoration = ICON_APPEARANCE_ICONBOX;
	}
	else
	{
	    pSD->iconDecoration = ICON_APPEARANCE_STANDALONE;
	}
    }

    /*
     * If resizeBorderWidth or frameBorderWidth is unset then initialize
     * to dynamic defaults.
     */

    if ((pSD->resizeBorderWidth == (Dimension)BIGSIZE) ||
	(pSD->frameBorderWidth == (Dimension)BIGSIZE))
    {
	double xres, yres, avg_res;

	xres = (((double) DisplayWidth(DISPLAY, pSD->screen)) / 
		((double) DisplayWidthMM(DISPLAY, pSD->screen)));
	yres = (((double) DisplayHeight(DISPLAY, pSD->screen)) / 
		((double) DisplayHeightMM(DISPLAY, pSD->screen)));

	avg_res = (xres + yres) / 2.0;

	/* Multiply times width in mm (avg. 7-8 pixels) */
	if (pSD->resizeBorderWidth == (Dimension)BIGSIZE)
	{
	    pSD->resizeBorderWidth = (int) (avg_res * 2.2);

	    /* limit size because big borders look ugly */
	    if (pSD->resizeBorderWidth > 7) pSD->resizeBorderWidth = 7;
	}

	/* Multiply times width in mm (avg. 5-6 pixels) */
	if (pSD->frameBorderWidth == (Dimension)BIGSIZE)
	{
	    pSD->frameBorderWidth = (int) (avg_res * 1.7);

	    /* limit size because big borders look ugly */
	    if (pSD->frameBorderWidth > 5) pSD->frameBorderWidth = 5;
	}
    }


    pSD->externalBevel = FRAME_EXTERNAL_SHADOW_WIDTH;
    pSD->joinBevel = FRAME_INTERNAL_SHADOW_WIDTH;
    if (pSD->frameBorderWidth < 
	   (pSD->externalBevel + MIN_INTERNAL_BEVEL))
    {
	pSD->frameBorderWidth = 
	    pSD->externalBevel + MIN_INTERNAL_BEVEL;
    }
    else if (pSD->frameBorderWidth > MAXIMUM_FRAME_BORDER_WIDTH)
    {
	pSD->frameBorderWidth = MAXIMUM_FRAME_BORDER_WIDTH;
    }

    if (pSD->resizeBorderWidth < 
	   (pSD->externalBevel + MIN_INTERNAL_BEVEL))
    {
	pSD->resizeBorderWidth = 
	    (pSD->externalBevel + MIN_INTERNAL_BEVEL);
    }
    else if (pSD->resizeBorderWidth > MAXIMUM_FRAME_BORDER_WIDTH)
    {
	pSD->resizeBorderWidth = MAXIMUM_FRAME_BORDER_WIDTH;
    }

    /*
     * Process the component appearance resources for client, 
     * icon and feedback parts of mwm.
     */

     ProcessAppearanceResources (pSD);
                                
    /*
     * Save the default icon pixmap in global data. We'll use it only
     * as a last resort.
     */

#ifdef DEC_MOTIF_EXTENSION
         /* Large icon ? */
         if (( pSD->iconImageMaximum.width >= iImage_large_width ) &&
             ( pSD->iconImageMaximum.height >= iImage_large_height ))
           {                  
             image_bits = iImage_large_bits;
             image_size = iImage_large_width;
           }
         /* medium icon ? */
         else if (( pSD->iconImageMaximum.width >= iImage_width ) &&
                  ( pSD->iconImageMaximum.height >= iImage_height ))   
           {                  
             image_bits = iImage_bits;
             image_size = iImage_width;
           }
         /* Small icon ! */
         else 
           {                  
             image_bits = iImage_small_bits;    
             image_size = iImage_small_width;
           }
    pSD->builtinIconPixmap = 
	XCreateBitmapFromData (DISPLAY, pSD->rootWindow, image_bits, 
				       image_size, image_size );
#else
    pSD->builtinIconPixmap = 
	XCreateBitmapFromData (DISPLAY, pSD->rootWindow, iImage_bits, 
				       iImage_width, iImage_height);
#endif /* DEC_MOTIF_EXTENSION */

} /* END OF FUNCTION ProcessScreenResources */



/******************************<->*************************************
 *
 *  ProcessWorkspaceResources (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves resources that are workspace specific.  If the
 *  window manager is providing standard behavior then retrieve the limited
 *  set of resources that don't affect standard behavior and set the
 *  values of the other resources to the standard values.
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 * 
 *  Outputs:
 *  -------
 *  pWS = resource data for workspace is set
 *
 *
 *  Comments:
 *  --------
 *  o Gets subresources based on workspace name
 * 
 ******************************<->***********************************/

#ifdef _NO_PROTO
void 
ProcessWorkspaceResources (pWS)
	WmWorkspaceData *pWS;

#else /* _NO_PROTO */
void 
ProcessWorkspaceResources (WmWorkspaceData *pWS)
#endif /* _NO_PROTO */
{

    /*
     * Retrieve workspace specific resources.
     */

    if (wmGD.useStandardBehavior)
    {
	XtGetSubresources (pWS->pSD->screenTopLevelW, (caddr_t) pWS, 
	    pWS->name, pWS->name, wmStdWorkspaceResources, 
	    XtNumber (wmStdWorkspaceResources), NULL, 0);

	/*
	 * Fill in the standard resource values.
	 *
	 * (no code for this right now)
	 */

    }
    else
    {
	XtGetSubresources (pWS->pSD->screenTopLevelW, (caddr_t) pWS, 
	    pWS->name, pWS->name, wmWorkspaceResources, 
	    XtNumber (wmWorkspaceResources), NULL, 0);
    }


} /* END OF FUNCTION ProcessWorkspaceResources */



/*************************************<->*************************************
 *
 *  ProcessClientResources (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves resources that are client specific.  If the
 *  window manager is providing standard behavior then retrieve the limited
 *  set of resources that don't affect standard behavior and set the
 *  values of the other resources to the standard values.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = resource data for client is set
 *
 *
 *  Comments:
 *  --------
 *  o Gets subresources based on client name and class.
 *  o Creates GC for the client Matte, if there is one.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
ProcessClientResources (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
void 
ProcessClientResources (ClientData *pCD)
#endif /* _NO_PROTO */
{
    String clientName;
    String clientClass;
    WmScreenData *pSD = pCD->pSD;

    /*
     * Retrieve basic client specific resources.
     */

    _pCD = pCD;	/* save in static global for dynamic default processing */
    clientName = (pCD->clientName) ? pCD->clientName : WmNdefaults;
    clientClass = (pCD->clientClass) ? pCD->clientClass : WmNdefaults;

    if (wmGD.useStandardBehavior)
    {     
	XtGetSubresources (pSD->screenTopLevelW, (caddr_t) pCD, clientName,
	    clientClass, wmStdClientResources, XtNumber (wmStdClientResources),
	    NULL, 0);

	/*
	 * Fill in the standard resource values.
	 */

	SetStdClientResourceValues (pCD);
    }
    else
    {
#ifdef DEC_MOTIF_EXTENSION
        /* Copy the resource list since it may be modified by XtGetSubResources. */
        if (( mwm_client_res == NULL ) && !ONE_SCREEN )
          {
            mwm_alloc( &mwm_client_res, sizeof( wmClientResources ), "Allocating resources " );
            memcpy( mwm_client_res, wmClientResources, sizeof( wmClientResources ) );
          }
#endif /* DEC_MOTIF_EXTENSION */
	XtGetSubresources (pSD->screenTopLevelW, (caddr_t) pCD, clientName,
	    clientClass, wmClientResources, XtNumber (wmClientResources), NULL,
	    0);
#ifdef DEC_MOTIF_EXTENSION
        /* Reset all the subresources */
        if ( !ONE_SCREEN )
            mwm_subres_get( pSD, mwm_client_res, XtNumber( wmClientResources ), 
                            (caddr_t)pCD );
#endif
    }

    /*
     * If (using DefaultWindowMenu but not defined) then use the builtin
     * system menu.
     */

    if ((pCD->systemMenu == defaultSystemMenuName) &&
	(pSD->defaultSystemMenuUseBuiltin == TRUE))
    {
	pCD->systemMenu = builtinSystemMenuName;
    }

    /*
     * If the client decorations or client functions have been defaulted
     * fix up the fields in the ProcessMwmHints function.
     */


    /* make top and bottom shadow pixmaps */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pCD->iconImageBottomShadowPStr ))
#else
    if (pCD->iconImageBottomShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
    {
	if ((pCD->iconImageBottomShadowPStr ==
		    pSD->iconAppearance.bottomShadowPStr) &&
	    (pCD->iconImageBottomShadowColor ==
		    pSD->iconAppearance.bottomShadowColor) &&
	    (pCD->iconImageBackground == 
		    pSD->iconAppearance.background))
	{
	    pCD->iconImageBottomShadowPixmap =
		    pSD->iconAppearance.bottomShadowPixmap;
	}
	else
	{
	    pCD->iconImageBottomShadowPixmap =
			    XmGetPixmap ( ScreenOfDisplay (DISPLAY,
				              pSD->screen),
				          pCD->iconImageBottomShadowPStr,
				          pCD->iconImageBottomShadowColor,
				          pCD->iconImageBackground);

	    if (pCD->iconImageBottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	    {
	        pCD->iconImageBottomShadowPixmap = (Pixmap)NULL;
	    }
	}
    }
    else
    {
	pCD->iconImageBottomShadowPixmap = (Pixmap)NULL;
    }

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pCD->iconImageTopShadowPStr ))
#else
    if (pCD->iconImageTopShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
    {
	if ((pCD->iconImageTopShadowPStr ==
				pSD->iconAppearance.topShadowPStr) &&
	    (pCD->iconImageTopShadowColor ==
				pSD->iconAppearance.topShadowColor) &&
	    (pCD->iconImageBackground == pSD->iconAppearance.background))
	{
	    pCD->iconImageTopShadowPixmap =
					pSD->iconAppearance.topShadowPixmap;
	}
	else
	{
	    pCD->iconImageTopShadowPixmap =
			    XmGetPixmap ( ScreenOfDisplay (DISPLAY,
				              pSD->screen),
				          pCD->iconImageTopShadowPStr,
				          pCD->iconImageTopShadowColor,
				          pCD->iconImageBackground);

	    if (pCD->iconImageTopShadowPixmap == XmUNSPECIFIED_PIXMAP)
	    {
	        pCD->iconImageTopShadowPixmap = (Pixmap)NULL;
	    }
	}
    }
    else
    {
	pCD->iconImageTopShadowPixmap = (Pixmap)NULL;
    }

    if ((pCD->internalBevel < MIN_INTERNAL_BEVEL)  || 
	(pCD->internalBevel > MAX_INTERNAL_BEVEL))
    {
	pCD->internalBevel = MAX_INTERNAL_BEVEL;
    }


    /*
     * Retrieve matte resources and make internal matte resources.
     */

    if (pCD->matteWidth > 0)
    {
#ifdef DEC_MOTIF_EXTENSION
        /* Copy the resource list since it may be modified by XtGetSubResources. */
        if (( mwm_client_resm == NULL ) && !ONE_SCREEN )
          {
            mwm_alloc( &mwm_client_resm, sizeof( wmClientResourcesM ), "Allocating resources " );
            memcpy( mwm_client_resm, wmClientResourcesM, sizeof( wmClientResourcesM ) );
          }
#endif /* DEC_MOTIF_EXTENSION */
	XtGetSubresources (pSD->screenTopLevelW, (caddr_t) pCD, clientName,
	    clientClass, wmClientResourcesM, XtNumber (wmClientResourcesM),
	    NULL, 0);

#ifdef DEC_MOTIF_EXTENSION
        /* Reset all the subresources */
        if ( !ONE_SCREEN )
            mwm_subres_get( pSD, mwm_client_resm, XtNumber( wmClientResourcesM ), 
                            (caddr_t)pCD );
#endif
        /* make top and bottom shadow pixmaps */

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pCD->matteBottomShadowPStr ))
#else
        if (pCD->matteBottomShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
        {
	    if ((pCD->matteBottomShadowPStr ==
				    pSD->clientAppearance.bottomShadowPStr) &&
	        (pCD->matteBottomShadowColor ==
				    pSD->clientAppearance.bottomShadowColor) &&
	        (pCD->matteBackground == pSD->clientAppearance.background))
	    {
	        pCD->matteBottomShadowPixmap =
				pSD->clientAppearance.bottomShadowPixmap;
	    }
	    else
	    {
	        pCD->matteBottomShadowPixmap =
			        XmGetPixmap (ScreenOfDisplay (DISPLAY,
				                 pSD->screen),
				             pCD->matteBottomShadowPStr,
				             pCD->matteBottomShadowColor,
				             pCD->matteBackground);

	        if (pCD->matteBottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	        {
	            pCD->matteBottomShadowPixmap = (Pixmap)NULL;
	        }
	    }
        }
        else
        {
	    pCD->matteBottomShadowPixmap = (Pixmap)NULL;
        }

#ifdef DEC_MOTIF_EXTENSION
/* Avoid the performance hit of getting the unspecified pixmap. */
    if ( !MWM_UNSPEC_PIXMAP( pCD->matteTopShadowPStr ))
#else
        if (pCD->matteTopShadowPStr)
#endif /* DEC_MOTIF_EXTENSION */
        {
	    if ((pCD->matteTopShadowPStr ==
				    pSD->clientAppearance.topShadowPStr) &&
	        (pCD->matteTopShadowColor ==
				    pSD->clientAppearance.topShadowColor) &&
	        (pCD->matteBackground == pSD->clientAppearance.background))
	    {
	        pCD->matteTopShadowPixmap =
					pSD->clientAppearance.topShadowPixmap;
	    }
	    else
	    {
	        pCD->matteTopShadowPixmap =
			        XmGetPixmap (ScreenOfDisplay (DISPLAY,
					         pSD->screen),
				             pCD->matteTopShadowPStr,
				             pCD->matteTopShadowColor,
				             pCD->matteBackground);

	        if (pCD->matteTopShadowPixmap == XmUNSPECIFIED_PIXMAP)
	        {
	            pCD->matteTopShadowPixmap = (Pixmap)NULL;
	        }
	    }
        }
        else
        {
	    pCD->matteTopShadowPixmap = (Pixmap)NULL;
        }


       	/* make top and bottom shadow GC's */

	pCD->clientMatteTopShadowGC = GetHighlightGC (pCD->pSD,
				      	  pCD->matteTopShadowColor,
				      	  pCD->matteBackground,
				      	  pCD->matteTopShadowPixmap);

	pCD->clientMatteBottomShadowGC = GetHighlightGC (pCD->pSD,
				      	  pCD->matteBottomShadowColor,
				      	  pCD->matteBackground,
				      	  pCD->matteBottomShadowPixmap);
    }

} /* END OF FUNCTION ProcessClientResources */



/*************************************<->*************************************
 *
 *  SetStdClientResourceValues (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets client resource data to standard values.  This setting
 *  is done in place of getting the values from the user settings in
 *  the resource database.
 * 
 *  Input:
 *  -----
 *  pCD = pointer to the client data
 *
 * 
 *  Output:
 *  ------
 *  pCD = (client data filled out with resource values)
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
SetStdClientResourceValues (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
void 
SetStdClientResourceValues (ClientData *pCD)
#endif /* _NO_PROTO */
{
    pCD->clientDecoration = WM_DECOR_DEFAULT;
    pCD->clientFunctions = WM_FUNC_DEFAULT;
    pCD->focusAutoRaise = True;
    pCD->systemMenu = builtinSystemMenuName;
    pCD->usePPosition = USE_PPOSITION_NONZERO;

} /* END OF FUNCTION SetStdClientResourceValues */



/******************************<->*************************************
 *
 *  SetStdScreenResourceValues (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets screen resource data to standard values.  This setting
 *  is done in place of getting the values from the user settings in
 *  the resource database.
 *
 *  Input:
 *  -----
 *  pSD = pointer to the screen data
 *
 * 
 *  Output:
 *  ------
 *  pSD = (screen data filled out with resource values)
 * 
 ******************************<->***********************************/

#ifdef _NO_PROTO
void 
SetStdScreenResourceValues (pSD)
	WmScreenData *pSD;

#else /* _NO_PROTO */
void 
SetStdScreenResourceValues (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    pSD->buttonBindings = builtinButtonBindingsName;
    pSD->cleanText = True;
    pSD->iconDecoration =
		(ICON_LABEL_PART | ICON_IMAGE_PART | ICON_ACTIVE_LABEL_PART);
#ifdef DEC_MOTIF_EXTENSION
    pSD->iconPlacement =
		(ICON_PLACE_BOTTOM_PRIMARY | ICON_PLACE_RIGHT_SECONDARY);
#else
    pSD->iconPlacement =
		(ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_BOTTOM_SECONDARY);
#endif /* DEC_MOTIF_EXTENSION */
    pSD->keyBindings = builtinKeyBindingsName;
    pSD->limitResize = True;
    pSD->resizeCursors = True;
    pSD->transientDecoration = (WM_DECOR_SYSTEM | WM_DECOR_RESIZEH);
    pSD->transientFunctions =
		(WM_FUNC_ALL & ~(MWM_FUNC_MAXIMIZE | MWM_FUNC_MINIMIZE |
				 MWM_FUNC_RESIZE));
    pSD->useIconBox = False;

    pSD->feedbackGeometry = NULL;
    pSD->moveOpaque = False;

} /* END OF FUNCTION SetStdScreenResourceValues */


/*************************************<->*************************************
 *
 *  GetHighlightGC (pSD, fg, bg, pixmap)
 *
 *
 *  Description:
 *  -----------
 *  Get a graphic context for either drawing top- or bottom-shadow 
 *  highlights.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  fg = foreground color
 *  bg = background color
 *  pixmap = pixmap for highlight
 * 
 *  Outputs:
 *  -------
 *  RETRUN = GC with the input parameters incorporated.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
GC GetHighlightGC (pSD, fg, bg, pixmap)
	WmScreenData *pSD;
	Pixel fg;
	Pixel bg;
	Pixmap pixmap;

#else /* _NO_PROTO */
GC GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap)
#endif /* _NO_PROTO */
{
    XGCValues gcv;
    XtGCMask  mask;


    mask = GCForeground | GCBackground | GCLineWidth | GCFillStyle;
    gcv.background = bg;
    gcv.foreground = fg;
    gcv.line_width = 1;

    if (pixmap)
    {
	mask |= GCFillStyle | GCTile;
	gcv.fill_style = FillTiled;
	gcv.tile = pixmap;
    }
    else
    {
	gcv.fill_style = FillSolid;
    }


    return (XtGetGC (pSD->screenTopLevelW, mask, &gcv));

} /* END OF FUNCTION GetHighlightGC */



/*************************************<->*************************************
 *
 *  _WmGetDynamicDefault (widget, type, defaultColor, newBackground, value)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to generate a default color of the requested 
 *  type.  Default colors are generated for a 3-D appearance.
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the widget that is associated with the resource or
 *           that is the reference widget for the wm subpart.
 *
 *  type = this is the type of color resource (e.g., top shadow color).
 *
 *  defaultColor = pointer to default color name/specification.
 *
 *  newBackground = background pixel for generating 3-D colors.
 *
 * 
 *  Outputs:
 *  -------
 *  value = pointer to the XrmValue in which to store the color
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void 
_WmGetDynamicDefault (widget, type, defaultColor, newBackground, value)
	Widget widget;
	unsigned char type;
	String defaultColor;
	Pixel newBackground;
	XrmValue *value;

#else /* _NO_PROTO */
void 
_WmGetDynamicDefault (Widget widget, unsigned char type, String defaultColor, Pixel newBackground, XrmValue *value)
#endif /* _NO_PROTO */
{
    static Screen *oldScreen = NULL;
    static Screen *newScreen;
    static Colormap oldColormap;
    static Colormap newColormap;
    static Pixel newValue;
    static Pixel background;
    static String oldDefaultColor = DEFAULT_COLOR_NONE;
    static XmColorData colorData;

    /* initialize the return value */

    value->size = sizeof (newValue);
    value->addr = (char *)&newValue;


    /*
     * Process monochrome defaults first.
     */

    newScreen = XtScreen (widget);

    if (Monochrome (newScreen))
    {
	if (type == WmFGC)
	{
	    newValue = BlackPixelOfScreen (newScreen);
	}
	else if (type == WmBGC)
	{
	    newValue = WhitePixelOfScreen (newScreen);
	}
	else if (type == WmTSC)
	{
	    newValue = WhitePixelOfScreen (newScreen);		
	}
	else if (type == WmBSC)
	{
	    newValue = BlackPixelOfScreen (newScreen);
	}

	return;
    }


    /*
     * Check to see if appropriate colors are available from the
     * previous request; if the color is a background color then get
     * default colors.  Generate 3-D colors if necessary.  Maintain
     * new colors in static variables for later reuse.
     */

    newColormap = widget->core.colormap;

    if ((oldScreen != NULL) && (oldScreen == newScreen) &&
	(oldColormap == newColormap) && (type != WmBGC) &&
	(background == newBackground))
    {
    }
    else if ((oldScreen == newScreen) && (oldColormap == newColormap) &&
	     (type == WmBGC) && (oldDefaultColor == defaultColor))
    {
    }
    else if (type == WmBGC)
    {
	/*
	 * Find or generate a background color and associated 3-D colors.
	 */

	oldDefaultColor = defaultColor;
/*
 * Fix for CR 5152 - Due to the use of Realloc in the color caches,
 *                   a static pointer is not acceptable.  Change it
 *                   to a static structure to maintain the data
 */
	colorData = *_WmGetDefaultColors (newScreen, newColormap, defaultColor);
    }
    else
    {
	/*
	 * Find or generate a color based on the associated background color.
	 */

	oldDefaultColor = DEFAULT_COLOR_NONE;
	background = newBackground;

	colorData = *_XmGetColors (newScreen, newColormap, background);
	
    }

    oldScreen = newScreen;
    oldColormap = newColormap;


    /*
     * Set up the return value.
     */

    newValue =  _XmAccessColorData(&colorData, type);

} /* END OF FUNCTION _WmGetDynamicDefault */



/*************************************<->*************************************
 *
 *  _WmGetDefaultColors (screen, colormap, defaultColor)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to find or generate default 3-D colors based on a
 *  default background color.
 *
 *
 *  Inputs:
 *  ------
 *  screen = screen for which colors are to be generated.
 *
 *  colormap = colormap that is to be used to make colors.
 *
 *  defaultColor = pointer to a default color name/specification.
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = pointer to WmColorData structure containing 3-D colors.
 * 
 *************************************<->***********************************/

XmColorData * _WmGetDefaultColors (screen, colormap, defaultColor)
	Screen *screen;
	Colormap colormap;
	String defaultColor;

{
    static XmColorData *defaultSet[2] = {NULL, NULL};
    static int defaultCount[2] = {0, 0};
    static int defaultSize[2] = {0, 0};
    int setId;
    register XmColorData *set;
    register int count;
    register int size;
    register int i;
    Display *display = DisplayOfScreen (screen);
    XColor colorDef;

/*
 * Fix for CR 5152 - Due to the use of Realloc with _XmGetColors, it is
 *                   necessary to maintain a separate cache of color
 *                   data.  The Realloc may cause the data to be moved,
 *                   and the cache would contain pointers into the heap.
 */

    /*
     * Look through the cache to see if the defaults are already in the
     * cache.  There is a list of cached defaults for each default color.
     */

    if (defaultColor == _defaultColor2)
    {
	setId = 1;
    }
    else
    {
	setId = 0;
    }

    set = defaultSet[setId];
    count = defaultCount[setId];
    size = defaultSize[setId];
    
    for (i = 0; i < count; i++)
    {
	if (((set + i)->screen == screen) && ((set + i)->color_map == colormap))
	{
	    return (set + i);
	}
    }

    /* 
     * No match in the cache, make a new entry and generate the colors.
     */

    if (count == size)
    {
	size = (defaultSize[setId] += 10);
	set = defaultSet[setId] =
		(XmColorData *)WmRealloc ((char *) defaultSet[setId],
			            sizeof (XmColorData) * size);
    }

    /*
     * Make the default background color for the resource set.
     */

    if(!XParseColor (display, colormap, defaultColor, &colorDef))
    {
        if(!(strcmp(defaultColor, _defaultColor1)))
        {
            XParseColor (display, colormap, _defaultColor1HEX, &colorDef);
        }
        else
        {
            XParseColor (display, colormap, _defaultColor2HEX, &colorDef);
        }
    }

    XAllocColor (display, colormap, &colorDef);


    /*
     * Generate the 3-D colors and save them in the defaults cache.
     */

    set[count] = *_XmGetColors (screen, colormap, colorDef.pixel);
    (defaultCount[setId])++;

    return (set + count);


} /* END OF FUNCTION _WmGetDefaultColors */



/*************************************<->*************************************
 *
 *  WmRealloc (ptr, size)
 *
 *
 *  Description:
 *  -----------
 *  This function is used reallocate a block of storage that has been
 *  malloc'ed.
 *
 *
 *  Inputs:
 *  ------
 *  ptr = pointer to storage that is to be realloc'ed; if NULL malloc an
 *        initial block of storage.
 *
 *  size = size of new storage
 * 
 *  Outputs:
 *  -------
 *  RETURN = pointer to realloc'ed block of storage
 * 
 *************************************<->***********************************/

char * WmRealloc (ptr, size)
	char *ptr;
	unsigned size;

{
    if (ptr)
    {
	ptr = (char *)XtRealloc (ptr, size);
    }
    else
    {
	ptr = (char *)XtMalloc (size);
    }

    if (ptr == NULL)
    {
	Warning ("Insufficient memory for window manager data");
    }

    return (ptr);

} /* END OF FUNCTION WmRealloc */



/*************************************<->*************************************
 *
 *  WmMalloc (ptr, size)
 *
 *
 *  Description:
 *  -----------
 *  This function is used malloc a block of storage.  If a previous block
 *  of storage is being replace the old block is free'd.
 *
 *
 *  Inputs:
 *  ------
 *  ptr = pointer to storage that is to be replaced (free'd).
 *
 *  size = size of new storage
 * 
 *  Outputs:
 *  -------
 *  RETURN = pointer to malloc'ed block of storage
 * 
 *************************************<->***********************************/

char * WmMalloc (ptr, size)
	char *ptr;
	unsigned size;

{
    if (ptr)
    {
	XtFree (ptr);
    }

    ptr = (char *)XtMalloc (size);

    if (ptr == NULL)
    {
	Warning ("Insufficient memory for window manager data");
    }

    return (ptr);

} /* END OF FUNCTION WmMalloc */



/*************************************<->*************************************
 *
 *  SetupDefaultResources (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to setup default (builtin) resources for the
 *  key bindings.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  wmGD = (defaultKeyBindingsString, ...)
 *
 *  builtinKeyBindingsName = name of default key bindings set
 *
 * 
 *  Outputs:
 *  -------
 *   None 
 *
 *************************************<->***********************************/

void
SetupDefaultResources (pSD)

WmScreenData *pSD;

{
    KeySpec *nextKeySpec;
    String keyBindings;
    MenuSpec *menuSpec;


/*
 * If (using DefaultBindings mechanism and bindings are not found in .mwmrc)
 *	then use the builtin bindings.
 */
    if (!pSD->keySpecs && !wmGD.useStandardBehavior)
    {
	/*
	 * Print warning if user is NOT using "DefaultKeyBindings".
	 */
	if (strcmp (pSD->keyBindings, defaultKeyBindingsName))
	{
	    MWarning("Key bindings %s not found, using builtin key bindings\n",
		     pSD->keyBindings);
	}
	pSD->keyBindings = builtinKeyBindingsName;
    }

    if (!pSD->buttonSpecs && !wmGD.useStandardBehavior)
    {
	/*
	 * Print warning if user is NOT using "DefaultButtonBindings".
	 */
	if (strcmp (pSD->buttonBindings, defaultButtonBindingsName))
	{
	    MWarning("Button bindings %s not found, using builtin button bindings\n",
		     pSD->buttonBindings);
	}
	pSD->buttonBindings = builtinButtonBindingsName;
    }

    if (pSD->keyBindings == builtinKeyBindingsName)
    {
	/*
	 * Default key specifications are to be used and no default
	 * set has been provided by the user.  Make the built-in default
	 * set.
	 */

	ParseKeyStr (pSD, (unsigned char *)builtinKeyBindings);
#ifdef DEC_MOTIF_BUG_FIX
/* Allow the user to toggle back to their behavior */
    }
    else
    {
#endif /* DEC_MOTIF_BUG_FIX */
	/*
	 * Add the switch behavior key binding to the front of the list
	 * of user specified key bindings that have been parsed.
	 */

	nextKeySpec = pSD->keySpecs;
	keyBindings = pSD->keyBindings;
	pSD->keyBindings = behaviorKeyBindingName;
	pSD->keySpecs = NULL;

	ParseKeyStr (pSD, (unsigned char *)behaviorKeyBindings);
	if (pSD->keySpecs)
	{
	    /* Skip past the TWO key definitions (1.2 & 1.1.4) */
	    pSD->keySpecs->nextKeySpec->nextKeySpec = nextKeySpec;
	}
	else
	{
	    pSD->keySpecs = nextKeySpec;
	}
	pSD->keyBindings = keyBindings;
    }

    if (pSD->buttonBindings == builtinButtonBindingsName)
    {
	/*
	 * Default button specifications are to be used and no default
	 * set has been provided by the user.  Make the built-in default
	 * set.
	 */

	ParseButtonStr (pSD, (unsigned char *)builtinButtonBindings);
    }

    /*
     * Set defaultSystemMenuUseBuiltin to FALSE if DefaultWindowMenu spec
     * is found.
     */

    menuSpec = pSD->menuSpecs;
    while ( menuSpec )
    {
	if (!strcmp(menuSpec->name, defaultSystemMenuName))
	{
		pSD->defaultSystemMenuUseBuiltin = FALSE;
		break;
	}
	menuSpec = menuSpec->nextMenuSpec;
    }

} /* END OF FUNCTION SetupDefaultResources */



/*************************************<->*************************************
 *
 *  SimilarAppearanceData (pAD1, pAD2)
 *
 *
 *  Description:
 *  -----------
 *  This function returns True if the two passed sets of AppearanceData
 *  are similar. This is designed to compare appearance data before
 *  creation of the GCs.
 *
 *
 *  Inputs:
 *  ------
 *  pAD1	pointer to AppearanceData 1
 *  pAD2	pointer to AppearanceData 2
 *
 * 
 *  Outputs:
 *  -------
 *  Function returns True if similar, False otherwise.
 * 
 *  Comments:
 *  ---------
 *  This function is only used to compare the client
 *  and client*title appearance data.
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean SimilarAppearanceData (pAD1, pAD2)

	AppearanceData *pAD1, *pAD2;
#else /* _NO_PROTO */
Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2)
#endif /* _NO_PROTO */
{
    Boolean rval;

    /*
     * !!! Should find out why all the Pixmap resources are unset !!!
     */

    if ((pAD1->fontList == pAD2->fontList) &&
	(pAD1->background == pAD2->background) &&
	(pAD1->foreground == pAD2->foreground) &&
	(pAD1->backgroundPStr == pAD2->backgroundPStr) &&
	(pAD1->bottomShadowColor == pAD2->bottomShadowColor) &&
	(pAD1->bottomShadowPStr == pAD2->bottomShadowPStr) &&
	(pAD1->topShadowColor == pAD2->topShadowColor) &&
	(pAD1->topShadowPStr == pAD2->topShadowPStr) &&
	(pAD1->activeBackground == pAD2->activeBackground) &&
	(pAD1->activeForeground == pAD2->activeForeground) &&
	(pAD1->activeBackgroundPStr == pAD2->activeBackgroundPStr) &&
	(pAD1->activeBottomShadowColor == pAD2->activeBottomShadowColor) &&
	(pAD1->activeBottomShadowPStr == pAD2->activeBottomShadowPStr) &&
	(pAD1->activeTopShadowColor == pAD2->activeTopShadowColor) &&
	(pAD1->activeTopShadowPStr == pAD2->activeTopShadowPStr) )
    {
	rval = True;
    }
    else 
    {
	rval = False;
    }

    return (rval);

} /* END OF FUNCTION SimilarAppearanceData */

