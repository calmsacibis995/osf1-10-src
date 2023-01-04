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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmResNames.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:17:13 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

/*
 * Value definitions:
 */



/******************************<->*************************************
 *
 *  Window manager resource names ...
 *
 *
 *  Description:
 *  -----------
 * 
 ******************************<->***********************************/

/* mwm specific appearance and behavior resources: */

#define WmNautoKeyFocus			"autoKeyFocus"
#define WmNautoRaiseDelay		"autoRaiseDelay"
#define WmNbitmapDirectory		"bitmapDirectory"
#define WmNbuttonBindings		"buttonBindings"
#define WmNcleanText			"cleanText"
#define WmNclientAutoPlace		"clientAutoPlace"
#define WmNcolormapFocusPolicy		"colormapFocusPolicy"
#define WmNconfigFile			"configFile"
#define WmNdeiconifyKeyFocus		"deiconifyKeyFocus"
#define WmNdoubleClickTime		"doubleClickTime"
#define WmNenableWarp			"enableWarp"
#define WmNenforceKeyFocus		"enforceKeyFocus"
#define WmNfadeNormalIcon		"fadeNormalIcon"
#define WmNfeedbackGeometry		"feedbackGeometry"
#define WmNframeBorderWidth		"frameBorderWidth"
#define WmNfreezeOnConfig		"freezeOnConfig"
#define WmNiconAutoPlace		"iconAutoPlace"
#define WmNiconBoxGeometry		"iconBoxGeometry"
#define WmNiconBoxLayout		"iconBoxLayout"
#define WmNiconBoxName			"iconBoxName"
#define WmNiconBoxSBDisplayPolicy	"iconBoxSBDisplayPolicy"
#define WmNiconBoxScheme		"iconBoxScheme"
#define WmNiconBoxTitle			"iconBoxTitle"
#define WmNiconClick			"iconClick"
#define WmNiconDecoration		"iconDecoration"
#define WmNiconImageMaximum		"iconImageMaximum"
#define WmNiconImageMinimum		"iconImageMinimum"
#define WmNiconPlacement		"iconPlacement"
#define WmNiconPlacementMargin		"iconPlacementMargin"
#define WmNinteractivePlacement		"interactivePlacement"
#define WmNkeyBindings			"keyBindings"
#define WmNkeyboardFocusPolicy		"keyboardFocusPolicy"
#define WmNlimitResize			"limitResize"
#define WmNlowerOnIconify		"lowerOnIconify"
#define WmNmaximumMaximumSize		"maximumMaximumSize"
#define WmNmoveThreshold		"moveThreshold"
#define WmNmultiScreen			"multiScreen"
#define WmNpassButtons			"passButtons"
#define WmNpassSelectButton		"passSelectButton"
#define WmNpositionIsFrame		"positionIsFrame"
#define WmNpositionOnScreen		"positionOnScreen"
#define WmNquitTimeout			"quitTimeout"
#define WmNraiseKeyFocus		"raiseKeyFocus"
#define WmNresizeBorderWidth		"resizeBorderWidth"
#define WmNresizeCursors		"resizeCursors"
#define WmNshowFeedback			"showFeedback"
#define WmNstartupKeyFocus		"startupKeyFocus"
#define WmNsystemButtonClick		"wMenuButtonClick"
#define WmNsystemButtonClick2		"wMenuButtonClick2"
#define WmNtransientDecoration		"transientDecoration"
#define WmNtransientFunctions		"transientFunctions"
#define WmNuseIconBox			"useIconBox"
#define WmNmoveOpaque                   "moveOpaque"

/* conponent appearance resources: */

#define WmNactiveBackground		"activeBackground"
#define WmNactiveBackgroundPixmap	"activeBackgroundPixmap"
#define WmNactiveBottomShadowColor 	"activeBottomShadowColor"
#define WmNactiveBottomShadowPixmap	"activeBottomShadowPixmap"
#define WmNactiveForeground		"activeForeground"
#define WmNactiveTopShadowColor		"activeTopShadowColor"
#define WmNactiveTopShadowPixmap	"activeTopShadowPixmap"
#define WmNbackground			"background"
#define WmNbackgroundPixmap		"backgroundPixmap"
#define WmNbottomShadowColor 		"bottomShadowColor"
#define WmNbottomShadowPixmap		"bottomShadowPixmap"
#define WmNfont				"font"
#define WmNforeground			"foreground"
#define WmNsaveUnder			"saveUnder"
#define WmNtopShadowColor		"topShadowColor"
#define WmNtopShadowPixmap		"topShadowPixmap"
#ifdef DEC_MOTIF_EXTENSION
/* Menu Colors */
#define WmNmenuBackground		"menu*background"
#define WmNmenuBottomShadowColor 	"menu*bottomShadowColor"
#define WmNmenuForeground		"menu*foreground"
#define WmNmenuTopShadowColor		"menu*topShadowColor"
#define WmNmenuBackgroundPixmap		"menu*backgroundPixmap"
#define WmNmenuBottomShadowPixmap 	"menu*bottomShadowPixmap"
#define WmNmenuForegroundPixmap	 	"menu*foregroundPixmap"       
#define WmNmenuTopShadowPixmap 		"menu*topShadowColorPixmap"
#define WmNiconBoxVBackground "iconbox.IBframe.IBsWindow.vScrollBar.TroughColor"
#define WmNiconBoxHBackground "iconbox.IBframe.IBsWindow.hScrollBar.TroughColor"
#define WmNiconBoxBackground  "iconbox*background"
#define WmNiconBoxBackgroundPixmap "iconbox*backgroundPixmap"
#define k_mwm_unspec_pixmap_str "unspecified_pixmap"
#define k_mwm_50_foreground_str "50_foreground"
#endif


/* mwm - client specific resources: */

#define WmNclientDecoration		"clientDecoration"
#define WmNclientFunctions		"clientFunctions"
#define WmNfocusAutoRaise		"focusAutoRaise"
#define WmNiconImage			"iconImage"
#define WmNiconImageBackground		"iconImageBackground"
#define WmNiconImageBottomShadowColor	"iconImageBottomShadowColor"
#define WmNiconImageBottomShadowPixmap	"iconImageBottomShadowPixmap"
#define WmNiconImageForeground		"iconImageForeground"
#define WmNiconImageTopShadowColor	"iconImageTopShadowColor"
#define WmNiconImageTopShadowPixmap	"iconImageTopShadowPixmap"
#define WmNmatteBackground		"matteBackground"
#define WmNmatteBottomShadowColor	"matteBottomShadowColor"
#define WmNmatteBottomShadowPixmap	"matteBottomShadowPixmap"
#define WmNmatteForeground		"matteForeground"
#define WmNmatteTopShadowColor		"matteTopShadowColor"
#define WmNmatteTopShadowPixmap		"matteTopShadowPixmap"
#define WmNmatteWidth			"matteWidth"
#define WmNmaximumClientSize		"maximumClientSize"
#define WmNscreenList			"screenList"
#define WmNscreens			"screens"
#define WmNsystemMenu			"windowMenu"
#define WmNuseClientIcon		"useClientIcon"
#define WmNusePPosition			"usePPosition"
#ifdef DEC_MOTIF_EXTENSION
#define WmNinterPlaceDelay              "interPlaceDelay"
#define WmNinterPlaceRetries            "interPlaceRetries"
#define WmNuseDECMode                   "useDECMode"
#define WmNforceAltSpace                "forceAltSpace"
#define WmNICCCMCompliant               "ICCCMCompliant"
#define WmNmatchMenuColors              "matchMenuColors"
#define WmNignoreModKeys                "ignoreModKeys"
#define WmNignoreAllModKeys             "ignoreAllModKeys"
#define WmNiconFullDepth                "iconFullDepth"
#endif /* DEC_MOTIF_EXTENSION */

/* window manager part resource names: */

#define WmNclient			"client"
#define WmNfeedback			"feedback"
#define WmNicon				"icon"
#define WmNmenu				"menu"
#define WmNtitle			"title"
#define WmNdefaults			"defaults"

/* window manager client resource names: */

#define WmNiconBox			"iconbox"
#define WmNconfirmbox			"confirmbox"



/*************************************<->*************************************
 *
 *  Window manager resource classes ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/

/* mwm specific appearance and behavior resources: */

#define WmCAutoKeyFocus			"AutoKeyFocus"
#define WmCAutoRaiseDelay		"AutoRaiseDelay"
#define WmCBitmapDirectory		"BitmapDirectory"
#define WmCButtonBindings		"ButtonBindings"
#define WmCCleanText			"CleanText"
#define WmCClientAutoPlace		"ClientAutoPlace"
#define WmCColormapFocusPolicy		"ColormapFocusPolicy"
#define WmCConfigFile			"ConfigFile"
#define WmCDeiconifyKeyFocus		"DeiconifyKeyFocus"
#define WmCDoubleClickTime		"DoubleClickTime"
#define WmCEnableWarp			"EnableWarp"
#define WmCEnforceKeyFocus		"EnforceKeyFocus"
#define WmCFadeNormalIcon		"FadeNormalIcon"
#define WmCFeedbackGeometry		"FeedbackGeometry"
#define WmCFrameBorderWidth		"FrameBorderWidth"
#define WmCFreezeOnConfig		"FreezeOnConfig"
#define WmCIconAutoPlace		"IconAutoPlace"
#define WmCIconBoxGeometry		"IconBoxGeometry"
#define WmCIconBoxLayout		"IconBoxLayout"
#define WmCIconBoxName			"IconBoxName"
#define WmCIconBoxSBDisplayPolicy	"IconBoxSBDisplayPolicy"
#define WmCIconBoxScheme		"IconBoxScheme"
#define WmCIconBoxTitle			"IconBoxTitle"
#define WmCIconClick			"IconClick"
#define WmCIconDecoration		"IconDecoration"
#define WmCIconImageMaximum		"IconImageMaximum"
#define WmCIconImageMinimum		"IconImageMinimum"
#define WmCIconPlacement		"IconPlacement"
#define WmCIconPlacementMargin		"IconPlacementMargin"
#define WmCInteractivePlacement		"InteractivePlacement"
#define WmCKeyBindings			"KeyBindings"
#define WmCKeyboardFocusPolicy		"KeyboardFocusPolicy"
#define WmCLimitResize			"LimitResize"
#define WmCLowerOnIconify		"LowerOnIconify"
#define WmCMaximumMaximumSize		"MaximumMaximumSize"
#define WmCMoveThreshold		"MoveThreshold"
#define WmCMultiScreen			"MultiScreen"
#define WmCPassButtons			"PassButtons"
#define WmCPassSelectButton		"PassSelectButton"
#define WmCPositionIsFrame		"PositionIsFrame"
#define WmCPositionOnScreen		"PositionOnScreen"
#define WmCQuitTimeout			"QuitTimeout"
#define WmCRaiseKeyFocus		"RaiseKeyFocus"
#define WmCResizeBorderWidth		"ResizeBorderWidth"
#define WmCResizeCursors		"ResizeCursors"
#define WmCScreenList			"ScreenList"
#define WmCScreens			"Screens"
#define WmCShowFeedback			"ShowFeedback"
#define WmCStartupKeyFocus		"StartupKeyFocus"
#define WmCSystemButtonClick		"WMenuButtonClick"
#define WmCSystemButtonClick2		"WMenuButtonClick2"
#define WmCTransientDecoration		"TransientDecoration"
#define WmCTransientFunctions		"TransientFunctions"
#define WmCUseIconBox			"UseIconBox"
#define WmCMoveOpaque                   "MoveOpaque"

/* component appearance resources: */

#define WmCActiveBackground		"ActiveBackground"
#define WmCActiveBackgroundPixmap	"ActiveBackgroundPixmap"
#define WmCActiveBottomShadowColor 	"ActiveBottomShadowColor"
#define WmCActiveBottomShadowPixmap	"ActiveBottomShadowPixmap"
#define WmCActiveForeground		"ActiveForeground"
#define WmCActiveTopShadowColor		"ActiveTopShadowColor"
#define WmCActiveTopShadowPixmap	"ActiveTopShadowPixmap"
#define WmCBackground			"Background"
#define WmCBackgroundPixmap		"BackgroundPixmap"
#define WmCBottomShadowColor 		"BottomShadowColor"
#define WmCBottomShadowPixmap		"BottomShadowPixmap"
#define WmCFont				"Font"
#define WmCForeground			"Foreground"
#define WmCSaveUnder			"SaveUnder"
#define WmCTopShadowColor		"TopShadowColor"
#define WmCTopShadowPixmap		"TopShadowPixmap"

/* mwm - client specific resources: */

#define WmCClientDecoration		"ClientDecoration"
#define WmCClientFunctions		"ClientFunctions"
#define WmCFocusAutoRaise		"FocusAutoRaise"
#define WmCIconImage			"IconImage"
#define WmCIconImageBackground		"IconImageBackground"
#define WmCIconImageBottomShadowColor	"IconImageBottomShadowColor"
#define WmCIconImageBottomShadowPixmap	"IconImageBottomShadowPixmap"
#define WmCIconImageForeground		"IconImageForeground"
#define WmCIconImageTopShadowColor	"IconImageTopShadowColor"
#define WmCIconImageTopShadowPixmap	"IconImageTopShadowPixmap"
#define WmCMatteBackground		"MatteBackground"
#define WmCMatteBottomShadowColor	"MatteBottomShadowColor"
#define WmCMatteBottomShadowPixmap	"MatteBottomShadowPixmap"
#define WmCMatteForeground		"MatteForeground"
#define WmCMatteTopShadowColor		"MatteTopShadowColor"
#define WmCMatteTopShadowPixmap		"MatteTopShadowPixmap"
#define WmCMatteWidth			"MatteWidth"
#define WmCMaximumClientSize		"MaximumClientSize"
#define WmCSystemMenu			"WindowMenu"
#define WmCUseClientIcon		"UseClientIcon"
#define WmCUsePPosition			"UsePPosition"
#ifdef DEC_MOTIF_EXTENSION
#define WmCinterPlaceDelay              "interPlaceDelay"
#define WmCinterPlaceRetries            "interPlaceRetries"
#define WmCUseDECMode                   "UseDECMode"
#define WmCForceAltSpace                "ForceAltSpace"
#define WmCICCCMCompliant               "ICCCMCompliant"
#define WmCMatchMenuColors              "MatchMenuColors"
#define WmCIgnoreModKeys                "IgnoreModKeys"
#define WmCIgnoreAllModKeys             "IgnoreAllModKeys"
#define WmCIconFullDepth                "IconFullDepth"
#endif /* DEC_MOTIF_EXTENSION */

/* window manager part resource names: */

#define WmCClient			"Client"
#define WmCFeedback			"Feedback"
#define WmCIcon				"Icon"
#define WmCMenu				"Menu"
#define WmCTitle			"Title"
#define WmCDefaults			"Defaults"

/* window manager client resource names: */

#define WmCIconBox			"Iconbox"
#define WmCConfirmbox			"Confirmbox"



/*************************************<->*************************************
 *
 *  Window manager resource converter names ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/

#define WmRCFocusPolicy			"WmCFocus"
#define WmRClientDecor			"WmCDecor"
#define WmRClientFunction		"WmCFunc"
#define WmRIconBoxLayout		"WmIBLayout"
#define WmRIconDecor			"WmIDecor"
#define WmRIconPlacement		"WmIPlace"
#define WmRKFocusPolicy			"WmKFocus"
#define WmRSize				"WmSize"
#define WmRShowFeedback			"WmShowFeedback"
#define WmRUsePPosition			"WmUsePPosition"



/*************************************<->*************************************
 *
 *  Window manager resource set definitions and default resource values ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/




/*************************************<->*************************************
 *
 *  Mwm resource description file definitions ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/

/* Configuration resource types: */

#define CRS_BUTTON		(1L << 0)
#define CRS_KEY			(1L << 1)
#define CRS_MENU		(1L << 2)
#define CRS_ACCEL		(1L << 3)
#define CRS_ANY			(CRS_BUTTON | CRS_KEY | CRS_MENU | CRS_ACCEL)

#ifdef DEC_MOTIF_EXTENSION
externalref String mwm_user_def_res_file;
externalref String mwm_user_bw_res_file;
externalref String mwm_user_gray_res_file;
externalref String mwm_sys_def_res_file;
externalref String mwm_sys_bw_res_file;
externalref String mwm_sys_gray_res_file;
#endif
#ifdef VMS
#define mwm_debug_file_name "decw$user_defaults:decw$mwm.log"
#define mwm_def_res_name "decw$mwm"
#define mwm_bw_res_name "decw$mwm_bw"
#define mwm_gray_res_name "decw$mwm_gray"
#define mwm_rc_res_user_name "decw$mwm_rc"
#define mwm_rc_res_sys_name "decw$mwm_rc"
#else
#define mwm_debug_file_name "mwm.log"
#define mwm_def_res_name "Mwm"
#define mwm_bw_res_name "Mwm_bw"
#define mwm_gray_res_name "Mwm_gray"
#define mwm_rc_res_user_name ".mwmrc"
#define mwm_rc_res_sys_name "system.mwmrc"
#endif
