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
***********************************************************
**                                                        *
**  Copyright (c) Digital Equipment Corporation, 1990  	  *
**  All Rights Reserved.  Unpublished rights reserved	  *
**  under the copyright laws of the United States.	  *
**                                                        *
**  The software contained on this media is proprietary	  *
**  to and embodies the confidential technology of 	  *
**  Digital Equipment Corporation.  Possession, use,	  *
**  duplication or dissemination of the software and	  *
**  media is authorized only pursuant to a valid written  *
**  license from Digital Equipment Corporation.	    	  *
**  							  *
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	  *
**  disclosure by the U.S. Government is subject to	  *
**  restrictions as set forth in Subparagraph (c)(1)(ii)  *
**  of DFARS 252.227-7013, or in FAR 52.227-19, as	  *
**  applicable.	    					  *
**  		                                          *
***********************************************************
**++
**  FACILITY:
**
**	< to be supplied >
**
**  ABSTRACT:
**
**	< to be supplied >
**
**  ENVIRONMENT:
**
**	< to be supplied >
**
**  MODIFICATION HISTORY:
**
**	< to be supplied >
**
**--
**/


/*
 * Author:  Jay Bolgatz  1990
 */
                                                          
#define COLORMIX

#ifdef VAXC
#else
#define const
#endif

/*---------------------------------------------------*/
/* include files                                     */
/*---------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>

#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/Xutil.h>
#include <X11/Xcms.h>
#include <X11/DECwI18n.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/ListP.h>
#include <Xm/Scale.h>
#include <Xm/ScaleP.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>
#include <Xm/RowColumnP.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/BulletinBP.h>
#include <Xm/DrawnB.h>
#include <Xm/SeparatoG.h>
#include <Xm/ScrolledWP.h>
#include <Xm/DragDrop.h>

#include "DXmMessI.h"
#include "DXmPrivate.h"
#ifndef WIN32
#include <DXm/DXmHelpB.h>
#endif
#include <DXm/DXmColor.h>
#include <DXm/DXmColorP.h>
#include <DXm/DECspecific.h>

/*---------------------------------------------------*/
/* Message definitions for character arrays	     */
/*---------------------------------------------------*/
#define COLORALLOCERR	 _DXmMsgColorMix_0000


/*---------------------------------------------------*/
/* forward declarations (in order of appearance)     */
/*---------------------------------------------------*/

/* core and composite class routines */

static void 	ClassInitialize();
static void 	Initialize();
static void 	Realize();
static void 	Destroy();
static Boolean	SetValues();
static XtGeometryResult GeometryManager();
static void 	ChangeManaged();
static void	RemoveChild();

/* initialization routines */

static void 	InitFields();
static void 	InitDefaultLabels();
static void 	InitPickerColorArray();
static void 	InitPickerCursor();
static void 	InitInterpColorArray();
static void 	InitSPColorArray();
static void 	InitBrowserColorArray();
static void 	InitDefaultBrowserStrings();
static void 	CopyPickerColorList();
static void 	ResetLabelFields();
static void 	CheckBrowserInitialization();
static void 	DetermineDisplayType();
static void 	GetDialogTitleString();

/* create routines */

static void 	CreateColLabelGadget();
static void 	CreateColTextWidget();
static void 	CreateColPBGadget();
static void 	CreateColPB();
static void 	CreateColABGadget();
static void 	CreateColPDMenu();
static void 	CreateColOptMenu();
static void 	CreateColShell();
static void 	CreateColDisplay();
static void 	CreateColHLSMixer();
static void 	CreateColRGBMixer();
static void 	CreateColPickerMixer();
static void 	CreateColScratchPad();
static void 	CreateColBrowserMixer();
static void 	CreateColGrayscaleMixer();
static void 	PopulateColorMixOptionMenu();

/* SetValues routines */

static Boolean  CheckMargins();
static Boolean  CheckAndUpdateDispMargins();
static Boolean  CheckAndUpdatePickerColors();
static Boolean  CheckAndUpdateDispWid();
static Boolean  CheckAndUpdateMixerWid();
static Boolean  CheckAndUpdateNewColors();
static Boolean  CheckAndUpdateOrigColors();
static Boolean  CheckAndUpdateBackColors();
static Boolean  CheckAndUpdateColors();
static Boolean  CheckAndUpdateColorModel();
static Boolean  CheckAndUpdateLayoutDir();
static Boolean  CheckAndUpdateTileSizes();
static void	UpdateMixer();
static void     UpdateMenu();
static void     UpdateText();

/* layout geometry handing routines */

static void	LayoutDisplay();
static void 	LayoutHLSMixer();
static void 	LayoutRGBMixer();
static void 	LayoutRGBMixerRToL();
static void 	LayoutGreyscaleMixerWindow();
static void 	LayoutPickerMixer();
static void 	LayoutBrowserMixerWindow();
static void 	LayoutScratchPad();
static void	LayoutCurrentMixer();
static void	LayoutColorMixWidget();
static void	GetBrowserButtonMaxSize();
static void 	GetHLSLabelWidth();
static void	GetColorMixSize();
static void 	GetPickerWidgetWidth();
static void 	ColorMixSetPBWidth();
static void	SetColorMixDimensions();
static void	ColorMixShrink();

/* subwidget action routines */

static void	ColorMixFocusInDisplay();
static void	ColorMixFocusOutDisplay();
static void	ColorMixLeftPicker();
static void	ColorMixRightPicker();
static void	ColorMixFocusInPicker();
static void	ColorMixFocusOutPicker();
static void	ColorMixLeftInterp();
static void	ColorMixRightInterp();
static void	ColorMixFocusInInterp();
static void	ColorMixFocusOutInterp();
static void	ColorMixUpSP();
static void	ColorMixDownSP();
static void	ColorMixTopSP();
static void	ColorMixBottomSP();
static void	ColorMixFocusInSP();
static void	ColorMixFocusOutSP();

/* subwidget callback routines */

static void	ColorMixDispMap();
static void	ColorMixDispUnmap();
static void 	ColorMixDisplayInput();
static void 	ColorMixPickerMenuCallback();
static void 	ColorMixPickerExpose();
static void 	ColorMixPickerInput();
static void 	ColorMixInterpExpose();
static void 	ColorMixInterpInput();
static void	ColorMixInterpSmear();
static void	ColorMixInterpUndo();
static void	ColorMixInterpFill();
static void	ColorMixMakeWarmer();
static void	ColorMixMakeCooler();
static void	ColorMixMakeLighter();
static void	ColorMixMakeDarker();
static void	ColorMixScratchPadExpose();
static void	ColorMixScratchPadInput();
static void	ColorMixScratchPadScroll();
static void	ColorMixScratchPadClear();
static void	ColorMixScratchPadCancel();
static void	ColorMixScratchPadAddColor();
static void 	ColorMixHLSSCAct();
static void 	ColorMixRGBSCAct();
static void	ColorMixBrowserScroll();
static void	ColorMixBrowserSelect();
static void	ColorMixGreyscaleUpdate();
static void 	ColorMixMenuCallback();
static void 	ColorMixPBAct();
static void 	ColorMixHelp();
static void 	ColorMixHelpError();

/* mouse handling routines */

static void	DisplayMB1Press ();
static void	DisplayMB1Release ();
static void	PickerMB1Press();
static void	PickerMB1Release();
static void	InterpMB1Press ();
static void	InterpMB1Release ();
static void	ScratchPadMB1Press ();
static void	ScratchPadMB1Release ();
static void	ChangeAndGrabPointer();

/* color handling routines */

static void	SetUpColors();
static void	SetHLSMixerNewColor();
static void	SetRGBMixerNewColor();
static void	SetDisplayNewColor();
static void	SetMixerNewColor();
static void	SetOrigColor();
static void 	SetAGrayColor();
static void 	SetAColor();
static void	TextSetColor();
static Boolean	AllocStaticColorCell();
static Boolean	AllocDynColorCells();
static void	AllocDynColors();
static void	ColorMixInterpSmear();
static void	ColorMixInterpFill();
static void	ColorMixRGBToHLS();
static void	ColorMixHLSToRGB();
static void 	MatchNewToOrig();
static void	ResetColors();
static void	SetGreyscaleColors();
static int	ColCvtColorToScaleValue();
static unsigned short	CvtScaleToColorValue();
static void	AddColorToScratchPad ();

/* draw routines */

static void	DrawPickerTileBorder();
static void	DrawInterpTileBorder();
static void	DrawButtonBorder();
static void	ColMoveObject();
static void	PickerSelect();
static void	InterpSelect();
static void	DisplaySelect();
static void	ScratchPadSelect();
static void	UnhighlightCurrentColor();

/* drag and drop routines */

static void HandleColorDrop();
static void DoDropSiteTransfer();
static void DoDropSiteHelp();
static void ReceiveColorTransfer();
static void StartColorDrag();
static Boolean DragColorConvertProc();
static void GenericColorDragProc();
static void InterpColorDragProc();
static void CreateDropHelpDialog();
static XtCallbackProc DropHelpOK();
static XtCallbackProc DropHelpCancel();

/* misc work routines */

static void     UpdateRGBText();
static void	ManageScratchPad();
static void	ReconfigureInterpolator();
static void	SetPickerOptionSensitivity();
static void	SetInterpolatorSensitivity();
static void	SetInterpolatorEndTile();
static void	SetEndTile ();
static void	SaveUndoState();
static void	SaveInterpHLSValues();
static void	CopyPickerColorList();
static void	RemoveColDisplay();
static void	RemoveScratchPad();
static void	FreePickerXColors();
static void	FreeInterpXColors();
static void 	ColorMixProvideHelp();
static void 	ColorMixCallback();
static void 	color_range();
static double   value();

/* static data declarations */

/* Default picker colors (there must be 10 to match default count */

static const unsigned short default_spectrum_colors[30] = 
					{65535, 65535, 65535,	/* White	*/
					 65535,	 1310,	   0,	/* Red		*/
					 65535, 28180,  1310,	/* Orange	*/
					 62913, 60816,	   0,	/* Yellow	*/
					     0,	34078,	 199,	/* Green	*/
					     0,	    0, 55049,	/* Blue		*/
					 15217, 17377, 41142,	/* Blue violet	*/
					 23912, 14548, 33947,   /* Violet	*/
					 33422, 15728,	 553,   /* Brown	*/
					     0,	    0,	   0};	/* Black	*/

static const unsigned short default_pastel_colors[30] = 
					{65535, 33422, 46539,
					 65535, 46528, 41287,
					 65535, 55704, 14417,
					 65535, 65535, 29130,
					 30146, 61602, 48386,
					 41559, 57267, 65049,
					 35388, 51117, 51117,
					 50967, 32620, 57597,
					 65535, 41942, 58326,
					 51263, 43399, 36655};

/*static const unsigned short default_metallic_colors[30] = 
					{65535, 65535, 65535,
					 40659,  6525,  8358,
					 60292,  2009,     0,
					 65535, 31229,  6553,
					 65535, 52931,     0,
					  3604, 29163,  3753,
					     0, 46167, 43965,
					  7536,  7536, 57998,
					  8960,  8960, 36352,
					 29478, 10852, 49439};
*/

static const unsigned short default_metallic_colors[30] = 
					{65535,     0, 40000,
					 65535,     0,     0,
					 65535, 37027,  6553,
					 65535, 65535,     0,
					     0, 60292,     0,
					     0, 60292, 60292,
					     0, 32512, 65535,
					     0,     0, 65535,
					 50967,  7228, 65535,
					 42597, 10481, 65535};


static const unsigned short default_earthtone_colors[30] = 
					{42405, 10794, 10794,
					 45460, 22281,     0,
					 41405, 24344,  9712,
					 56360, 28069,     0,
					 56360, 43253,     0,
					 27000, 26482,   523,
					 22281, 32112,     0,
					 33029, 22019, 25506,
					 28330, 14921, 20508,
					 27076, 12741,   448};

static BrowserColorRec default_browser_colors[] = 
       {
	{"Midnight Blue", NULL, 0, 0, 0, FALSE},
	{"Dark Slate Blue", NULL, 0, 0, 0, FALSE},
	{"Slate Blue", NULL, 0, 0, 0, FALSE},
	{"Medium Slate Blue", NULL, 0, 0, 0, FALSE},
	{"Light Slate Blue", NULL, 0, 0, 0, FALSE},
	{"Light Steel Blue", NULL, 0, 0, 0, FALSE},
	{"Navy", NULL, 0, 0, 0, FALSE},
	{"Cadet Blue", NULL, 0, 0, 0, FALSE},
	{"Medium Blue", NULL, 0, 0, 0, FALSE},
	{"Blue", NULL, 0, 0, 0, FALSE},
	{"Light Blue", NULL, 0, 0, 0, FALSE},
	{"Dark Turquoise", NULL, 0, 0, 0, FALSE},
	{"Medium Turquoise", NULL, 0, 0, 0, FALSE},
	{"Turquoise", NULL, 0, 0, 0, FALSE},
	{"Pale Turquoise", NULL, 0, 0, 0, FALSE},
	{"Deep Sky Blue", NULL, 0, 0, 0, FALSE},
	{"Sky Blue", NULL, 0, 0, 0, FALSE},
	{"Light Sky Blue", NULL, 0, 0, 0, FALSE},
	{"Cyan", NULL, 0, 0, 0, FALSE},
	{"Light Cyan", NULL, 0, 0, 0, FALSE},
	{"Powder Blue", NULL, 0, 0, 0, FALSE},
	{"Alice Blue", NULL, 0, 0, 0, FALSE},
	{"Azure", NULL, 0, 0, 0, FALSE},
	{"Dodger Blue", NULL, 0, 0, 0, FALSE},
	{"Royal Blue", NULL, 0, 0, 0, FALSE},
	{"DECWblue", NULL, 0, 0, 0, FALSE},
	{"Cornflower Blue", NULL, 0, 0, 0, FALSE},
	{"Blue Violet", NULL, 0, 0, 0, FALSE},
	{"Dark Violet", NULL, 0, 0, 0, FALSE},
	{"Violet", NULL, 0, 0, 0, FALSE},
	{"Medium Violet Red", NULL, 0, 0, 0, FALSE},
	{"Violet Red", NULL, 0, 0, 0, FALSE},
	{"Pale Violet Red", NULL, 0, 0, 0, FALSE},
	{"Plum", NULL, 0, 0, 0, FALSE},
	{"Purple", NULL, 0, 0, 0, FALSE},
	{"Medium Purple", NULL, 0, 0, 0, FALSE},
	{"Dark Orchid", NULL, 0, 0, 0, FALSE},
	{"Medium Orchid", NULL, 0, 0, 0, FALSE},
	{"Thistle", NULL, 0, 0, 0, FALSE},
	{"Orchid", NULL, 0, 0, 0, FALSE},
	{"Lavender", NULL, 0, 0, 0, FALSE},
	{"Lavender Blush", NULL, 0, 0, 0, FALSE},
	{"Magenta", NULL, 0, 0, 0, FALSE},
	{"Hot Pink", NULL, 0, 0, 0, FALSE},
	{"Deep Pink", NULL, 0, 0, 0, FALSE},
	{"Light Pink", NULL, 0, 0, 0, FALSE},
	{"Pink", NULL, 0, 0, 0, FALSE},
	{"Medium Aquamarine", NULL, 0, 0, 0, FALSE},
	{"Aquamarine", NULL, 0, 0, 0, FALSE},
	{"Dark Sea Green", NULL, 0, 0, 0, FALSE},
	{"Sea Green", NULL, 0, 0, 0, FALSE},
	{"Light Sea Green", NULL, 0, 0, 0, FALSE},
	{"Forest Green", NULL, 0, 0, 0, FALSE},
	{"Medium Forest Green", NULL, 0, 0, 0, FALSE},
	{"Dark Olive Green", NULL, 0, 0, 0, FALSE},
	{"Olive Drab", NULL, 0, 0, 0, FALSE},
	{"Dark Khaki", NULL, 0, 0, 0, FALSE},
	{"Khaki", NULL, 0, 0, 0, FALSE},
	{"Dark Green", NULL, 0, 0, 0, FALSE},
	{"Green", NULL, 0, 0, 0, FALSE},
	{"Pale Green", NULL, 0, 0, 0, FALSE},
	{"Lawn Green", NULL, 0, 0, 0, FALSE},
	{"Lime Green", NULL, 0, 0, 0, FALSE},
	{"Medium Spring Green", NULL, 0, 0, 0, FALSE},
	{"Spring Green", NULL, 0, 0, 0, FALSE},
	{"Green Yellow", NULL, 0, 0, 0, FALSE},
	{"Yellow Green", NULL, 0, 0, 0, FALSE},
	{"Chartreuse", NULL, 0, 0, 0, FALSE},
	{"Yellow", NULL, 0, 0, 0, FALSE},
	{"Light Yellow", NULL, 0, 0, 0, FALSE},
	{"Cornsilk", NULL, 0, 0, 0, FALSE},
	{"Lemon Chiffon", NULL, 0, 0, 0, FALSE},
	{"Honeydew", NULL, 0, 0, 0, FALSE},
	{"Dark Goldenrod", NULL, 0, 0, 0, FALSE},
	{"Goldenrod", NULL, 0, 0, 0, FALSE},
	{"Medium Goldenrod", NULL, 0, 0, 0, FALSE},
	{"Light Goldenrod", NULL, 0, 0, 0, FALSE},
	{"Pale Goldenrod", NULL, 0, 0, 0, FALSE},
	{"Light Goldenrod Yellow", NULL, 0, 0, 0, FALSE},
	{"Gold", NULL, 0, 0, 0, FALSE},
	{"Blanched Almond", NULL, 0, 0, 0, FALSE},
	{"Beige", NULL, 0, 0, 0, FALSE},
	{"Brown", NULL, 0, 0, 0, FALSE},
	{"Saddle Brown", NULL, 0, 0, 0, FALSE},
	{"Chocolate", NULL, 0, 0, 0, FALSE},
	{"Peru", NULL, 0, 0, 0, FALSE},
	{"Rosy Brown", NULL, 0, 0, 0, FALSE},
	{"Burlywood", NULL, 0, 0, 0, FALSE},
	{"Tan", NULL, 0, 0, 0, FALSE},
	{"Moccasin", NULL, 0, 0, 0, FALSE},
	{"SandyBrown", NULL, 0, 0, 0, FALSE},
	{"Coral", NULL, 0, 0, 0, FALSE},
	{"Light Coral", NULL, 0, 0, 0, FALSE},
	{"Dark Salmon", NULL, 0, 0, 0, FALSE},
	{"Salmon", NULL, 0, 0, 0, FALSE},
	{"Light Salmon", NULL, 0, 0, 0, FALSE},
	{"Peach Puff", NULL, 0, 0, 0, FALSE},
	{"Bisque", NULL, 0, 0, 0, FALSE},
	{"Papaya Whip", NULL, 0, 0, 0, FALSE},
	{"Dark Orange", NULL, 0, 0, 0, FALSE},
	{"Orange", NULL, 0, 0, 0, FALSE},
	{"Orange Red", NULL, 0, 0, 0, FALSE},
	{"Red", NULL, 0, 0, 0, FALSE},
	{"Tomato", NULL, 0, 0, 0, FALSE},
	{"Indian Red", NULL, 0, 0, 0, FALSE},
	{"Firebrick", NULL, 0, 0, 0, FALSE},
	{"Sienna", NULL, 0, 0, 0, FALSE},
	{"Maroon", NULL, 0, 0, 0, FALSE},
	{"Misty Rose", NULL, 0, 0, 0, FALSE},
	{"White", NULL, 0, 0, 0, FALSE},
	{"Ivory", NULL, 0, 0, 0, FALSE},
	{"Snow", NULL, 0, 0, 0, FALSE},
	{"Mint Cream", NULL, 0, 0, 0, FALSE},
	{"Floral White", NULL, 0, 0, 0, FALSE},
	{"Ghost White", NULL, 0, 0, 0, FALSE},
	{"Seashell", NULL, 0, 0, 0, FALSE},
	{"Old Lace", NULL, 0, 0, 0, FALSE},
	{"White Smoke", NULL, 0, 0, 0, FALSE},
	{"Linen", NULL, 0, 0, 0, FALSE},
	{"Antique White", NULL, 0, 0, 0, FALSE},
	{"Navajo White", NULL, 0, 0, 0, FALSE},
	{"Wheat", NULL, 0, 0, 0, FALSE},
	{"Gainsboro", NULL, 0, 0, 0, FALSE},
	{"Light Gray", NULL, 0, 0, 0, FALSE},
	{"Gray", NULL, 0, 0, 0, FALSE},
	{"Dim Gray", NULL, 0, 0, 0, FALSE},
	{"Light Slate Gray", NULL, 0, 0, 0, FALSE},
	{"Slate Gray", NULL, 0, 0, 0, FALSE},
	{"Dark Slate Gray", NULL, 0, 0, 0, FALSE},
	{"Black", NULL, 0, 0, 0, FALSE},
	{"Screen Background", NULL, 0, 0, 0, FALSE},
	{"Border Topshadow", NULL, 0, 0, 0, FALSE},
	{"Border Background", NULL, 0, 0, 0, FALSE},
	{"Border Bottomshadow", NULL, 0, 0, 0, FALSE},
	{"Window Topshadow", NULL, 0, 0, 0, FALSE},
	{"Window Background", NULL, 0, 0, 0, FALSE},
	{"Window Bottomshadow", NULL, 0, 0, 0, FALSE}};

/* Bitmap data for eyedropper cursor */

#define eyedropper_width 16
#define eyedropper_height 16
#define eyedropper_x_hot 0
#define eyedropper_y_hot 15
static const unsigned char eyedropper_bits[] = {
   0x00, 0x70, 0x00, 0xf8, 0x00, 0xfc, 0x00, 0xff, 0x00, 0x7f, 0x80, 0x3e,
   0x40, 0x1c, 0x20, 0x18, 0x10, 0x04, 0x08, 0x02, 0x04, 0x01, 0x82, 0x00,
   0x42, 0x00, 0x21, 0x00, 0x19, 0x00, 0x06, 0x00};

#define eyedropper_mask_width 16
#define eyedropper_mask_height 16
static const unsigned char eyedropper_mask_bits[] = {
   0x00, 0x70, 0x00, 0xf8, 0x00, 0xfc, 0x00, 0xff, 0x00, 0x7f, 0x80, 0x3f,
   0xc0, 0x1f, 0xe0, 0x1f, 0xf0, 0x07, 0xf8, 0x03, 0xfc, 0x01, 0xfe, 0x00,
   0x7e, 0x00, 0x3f, 0x00, 0x1f, 0x00, 0x06, 0x00};

#define eyedropper_32_width 32
#define eyedropper_32_height 32
#define eyedropper_32_x_hot 0
#define eyedropper_32_y_hot 32
static const unsigned char eyedropper_32_bits[] = {
   0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xc0, 0xff,
   0x00, 0x00, 0xc0, 0xff, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0xf0, 0xff,
   0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x3f,
   0x00, 0x00, 0xff, 0x3f, 0x00, 0x80, 0xfc, 0x0f, 0x00, 0xc0, 0xfc, 0x0f,
   0x00, 0x20, 0xf0, 0x03, 0x00, 0x30, 0xf0, 0x03, 0x00, 0x48, 0xc0, 0x03,
   0x00, 0x8c, 0xc0, 0x03, 0x00, 0x02, 0x30, 0x00, 0x00, 0x03, 0x10, 0x00,
   0x80, 0x04, 0x0c, 0x00, 0xc0, 0x08, 0x04, 0x00, 0x20, 0x00, 0x03, 0x00,
   0x30, 0x00, 0x01, 0x00, 0x48, 0xc0, 0x00, 0x00, 0x8c, 0x40, 0x00, 0x00,
   0x04, 0x30, 0x00, 0x00, 0x04, 0x10, 0x00, 0x00, 0x02, 0x0c, 0x00, 0x00,
   0x02, 0x04, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0xc2, 0x01, 0x00, 0x00,
   0x3d, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00};

#define eyedropper_32_mask_width 32
#define eyedropper_32_mask_height 32
static const unsigned char eyedropper_32_mask_bits[] = {
   0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xc0, 0xff,
   0x00, 0x00, 0xc0, 0xff, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0xf0, 0xff,
   0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x3f,
   0x00, 0x00, 0xff, 0x3f, 0x00, 0x80, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f,
   0x00, 0xe0, 0xff, 0x03, 0x00, 0xf0, 0xff, 0x03, 0x00, 0xf8, 0xff, 0x03,
   0x00, 0xfc, 0xff, 0x03, 0x00, 0xfe, 0x3f, 0x00, 0x00, 0xff, 0x1f, 0x00,
   0x80, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x07, 0x00, 0xe0, 0xff, 0x03, 0x00,
   0xf0, 0xff, 0x01, 0x00, 0xf8, 0xff, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00,
   0xfc, 0x3f, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0xfe, 0x0f, 0x00, 0x00,
   0xfe, 0x07, 0x00, 0x00, 0xfe, 0x03, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00,
   0x3f, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00};

/* Bitmap data for greyscale pixmap label */

#define gslabel_width 325
#define gslabel_height 30
static const unsigned char gslabel_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
   0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x1f};

#define gslabel_rtol_width 325
#define gslabel_rtol_height 30
static const unsigned char gslabel_rtol_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
   0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf8, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


/* Bitmaps for paint buckets */

#define lbucket_width 24
#define lbucket_height 24
static const unsigned char lbucket_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x38, 0x00,
   0x00, 0x6c, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x83, 0x01, 0x80, 0x01, 0x3f,
   0xc0, 0x18, 0x46, 0x60, 0xe4, 0x3f, 0x30, 0x24, 0x18, 0x18, 0x18, 0x30,
   0x38, 0x00, 0x60, 0x68, 0x00, 0xc0, 0xc8, 0x00, 0x60, 0x88, 0x01, 0x30,
   0x08, 0x03, 0x18, 0x08, 0x06, 0x0c, 0x08, 0x0c, 0x06, 0x08, 0x18, 0x03,
   0x08, 0xb0, 0x01, 0x1c, 0xe0, 0x00, 0x3e, 0x40, 0x00, 0x7f, 0x00, 0x00};

#define rbucket_width 24
#define rbucket_height 24
static const unsigned char rbucket_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00,
   0x00, 0x36, 0x00, 0x00, 0x63, 0x00, 0x80, 0xc1, 0x00, 0xfc, 0x80, 0x01,
   0x62, 0x18, 0x03, 0xfc, 0x27, 0x06, 0x18, 0x24, 0x0c, 0x0c, 0x18, 0x18,
   0x06, 0x00, 0x1c, 0x03, 0x00, 0x16, 0x06, 0x00, 0x13, 0x0c, 0x80, 0x11,
   0x18, 0xc0, 0x10, 0x30, 0x60, 0x10, 0x60, 0x30, 0x10, 0xc0, 0x18, 0x10,
   0x80, 0x0d, 0x10, 0x00, 0x07, 0x38, 0x00, 0x02, 0x7c, 0x00, 0x00, 0xfe};

/* colormix help library declaration */

#ifdef VMS
#define HELP_LIBRARY	"decw$dxmcolor_help" /* VMS help library */
#else
#define HELP_LIBRARY	"DXmColor_Help"      /* ULTRIX help library  */
#endif

static Opaque help_context = (Opaque) NULL;

/* Mask bits for chorded cancel */

#define CANCELMASK	(ShiftMask|ControlMask|Mod1Mask|Mod2Mask| \
                         Mod3Mask|Mod4Mask|Mod5Mask|Button2Mask|Button3Mask)


/*---------------------------------------------------*/
/* colormix translations for subwidgets		     */
/*---------------------------------------------------*/

static XtTranslations cmwtext_translations_parsed;
static XtTranslations cmwdisp_translations_parsed;
static XtTranslations cmwpick_translations_parsed;
static XtTranslations cmwinterp_translations_parsed;
static XtTranslations cmwsp_translations_parsed;

/*
 *  0xff0d = Return  
 *  0xff8d = Enter  
 */

static const char *cmwtext_translation_table = 
    "~Shift<KeyPress>0xff0d:	DXMCMTEXTSETCOL()\n\
     <KeyPress>0xff8d:		DXMCMTEXTSETCOL()";


static const char *cmwdisp_translation_table = 
    "<FocusIn>:         PrimitiveFocusIn()\
			DXmColorMixFocusInDisplay()\n\
     <FocusOut>:        PrimitiveFocusOut()\
			DXmColorMixFocusOutDisplay()\n\
     <Btn2Down>:	DXmColorMixStartColorDrag()";

static const char *cmwpick_translation_table = 
    "<Key>osfLeft:	PrimitiveTraverseLeft()\
			DXmColorMixLeftPicker()\n\
     <Key>osfRight:	PrimitiveTraverseRight()\
			DXmColorMixRightPicker()\n\
     <Key>osfUp:	PrimitiveTraverseUp()\
			DXmColorMixLeftPicker()\n\
     <Key>osfDown:	PrimitiveTraverseDown()\
			DXmColorMixRightPicker()\n\
     <FocusIn>:         PrimitiveFocusIn()\
			DXmColorMixFocusInPicker()\n\
     <FocusOut>:        PrimitiveFocusOut()\
			DXmColorMixFocusOutPicker()\n\
     <Btn2Down>:	DXmColorMixStartColorDrag()";

static const char *cmwinterp_translation_table = 
    "<Key>osfLeft:	PrimitiveTraverseLeft()\
			DXmColorMixLeftInterp()\n\
     <Key>osfRight:	PrimitiveTraverseRight()\
			DXmColorMixRightInterp()\n\
     <Key>osfUp:	PrimitiveTraverseUp()\
			DXmColorMixLeftInterp()\n\
     <Key>osfDown:	PrimitiveTraverseDown()\
			DXmColorMixRightInterp()\n\
     <FocusIn>:         PrimitiveFocusIn()\
			DXmColorMixFocusInInterp()\n\
     <FocusOut>:        PrimitiveFocusOut()\
			DXmColorMixFocusOutInterp()\n\
     <Btn2Down>:	DXmColorMixStartColorDrag()";

static const char *cmwsp_translation_table = 
    "<Key>osfLeft:	PrimitiveTraverseLeft()\
			DXmColorMixUpSP()\n\
     <Key>osfRight:	PrimitiveTraverseRight()\
			DXmColorMixDownSP()\n\
     <Key>osfUp:	PrimitiveTraverseUp()\
			DXmColorMixUpSP()\n\
     <Key>osfDown:	PrimitiveTraverseDown()\
			DXmColorMixDownSP()\n\
     <Key>osfPageUp:	DXmColorMixUpSP(0)\n\
     <Key>osfPageDown:	DXmColorMixDownSP(0)\n\
     <Key>osfBeginLine:	PrimitiveTraverseHome()\
     			DXmColorMixTopSP()\n\
     <Key>osfEndLine:	DXmColorMixBottomSP()\n\
     <FocusIn>:         PrimitiveFocusIn()\
			DXmColorMixFocusInSP()\n\
     <FocusOut>:        PrimitiveFocusOut()\
			DXmColorMixFocusOutSP()\n\
     <Btn2Down>:	DXmColorMixStartColorDrag()";


static XtActionsRec ActionsTable[] = 
    {
	{"DXMCMTEXTSETCOL", (XtActionProc)TextSetColor},
	{"DXmColorMixFocusInDisplay", (XtActionProc)ColorMixFocusInDisplay},
	{"DXmColorMixFocusOutDisplay", (XtActionProc)ColorMixFocusOutDisplay},
	{"DXmColorMixLeftPicker", (XtActionProc)ColorMixLeftPicker},
	{"DXmColorMixRightPicker", (XtActionProc)ColorMixRightPicker},
	{"DXmColorMixFocusInPicker", (XtActionProc)ColorMixFocusInPicker},
	{"DXmColorMixFocusOutPicker", (XtActionProc)ColorMixFocusOutPicker},
	{"DXmColorMixLeftInterp", (XtActionProc)ColorMixLeftInterp},
	{"DXmColorMixRightInterp", (XtActionProc)ColorMixRightInterp},
	{"DXmColorMixFocusInInterp", (XtActionProc)ColorMixFocusInInterp},
	{"DXmColorMixFocusOutInterp", (XtActionProc)ColorMixFocusOutInterp},
	{"DXmColorMixUpSP", (XtActionProc)ColorMixUpSP},
	{"DXmColorMixDownSP", (XtActionProc)ColorMixDownSP},
	{"DXmColorMixTopSP", (XtActionProc)ColorMixTopSP},
	{"DXmColorMixBottomSP", (XtActionProc)ColorMixBottomSP},
	{"DXmColorMixFocusInSP", (XtActionProc)ColorMixFocusInSP},
	{"DXmColorMixFocusOutSP", (XtActionProc)ColorMixFocusOutSP},
	{"DXmColorMixStartColorDrag", (XtActionProc)StartColorDrag},
        {NULL, NULL}
    };

/* Atoms used by drag and drop */

static Atom COMPOUND_TEXT;
static Atom DXM_COLOR;


/*---------------------------------------------------*/
/* colormix callback declarations		     */
/*---------------------------------------------------*/

static XtCallbackRec cmw_help_cb[]  = {{(XtCallbackProc) ColorMixHelp, NULL}, NULL};
static XtCallbackRec cmw_map_cb[]   = {{(XtCallbackProc) ColorMixDispMap, NULL}, NULL};
static XtCallbackRec cmw_unmap_cb[] = {{(XtCallbackProc) ColorMixDispUnmap, NULL}, NULL};
static XtCallbackRec cmw_pbact_cb[] = {{(XtCallbackProc) ColorMixPBAct, NULL}, NULL};
static XtCallbackRec cmw_hlsscact_cb[] = {{(XtCallbackProc) ColorMixHLSSCAct, NULL}, NULL};
static XtCallbackRec cmw_scact_cb[] = {{(XtCallbackProc) ColorMixRGBSCAct, NULL}, NULL};
static XtCallbackRec cmw_menu_cb[]  = {{(XtCallbackProc) ColorMixMenuCallback, NULL}, NULL, NULL};
static XtCallbackRec cmw_pmenu_cb[]  = {{(XtCallbackProc) ColorMixPickerMenuCallback, NULL}, NULL, NULL};
static XtCallbackRec cmw_picker_expose_cb[]  = {{(XtCallbackProc) ColorMixPickerExpose, NULL}, NULL};
static XtCallbackRec cmw_interp_expose_cb[]  = {{(XtCallbackProc) ColorMixInterpExpose, NULL}, NULL};
static XtCallbackRec cmw_sp_expose_cb[]  = {{(XtCallbackProc) ColorMixScratchPadExpose, NULL}, NULL};
static XtCallbackRec cmw_picker_input_cb[]  = {{(XtCallbackProc) ColorMixPickerInput, NULL}, NULL};
static XtCallbackRec cmw_interp_input_cb[]  = {{(XtCallbackProc) ColorMixInterpInput, NULL}, NULL};
static XtCallbackRec cmw_display_input_cb[]  = {{(XtCallbackProc) ColorMixDisplayInput, NULL}, NULL};
static XtCallbackRec cmw_sp_scroll_cb[] = {{(XtCallbackProc) ColorMixScratchPadScroll, NULL}, NULL};
static XtCallbackRec cmw_smear_cb[] = {{(XtCallbackProc) ColorMixInterpSmear, NULL}, NULL};
static XtCallbackRec cmw_bucket_cb[] = {{(XtCallbackProc) ColorMixInterpFill, NULL}, NULL};
static XtCallbackRec cmw_undo_cb[] = {{(XtCallbackProc) ColorMixInterpUndo, NULL}, NULL};
static XtCallbackRec cmw_warmer_cb[] = {{(XtCallbackProc) ColorMixMakeWarmer, NULL}, NULL};
static XtCallbackRec cmw_cooler_cb[] = {{(XtCallbackProc) ColorMixMakeCooler, NULL}, NULL};
static XtCallbackRec cmw_lighter_cb[] = {{(XtCallbackProc) ColorMixMakeLighter, NULL}, NULL};
static XtCallbackRec cmw_darker_cb[] = {{(XtCallbackProc) ColorMixMakeDarker, NULL}, NULL};
static XtCallbackRec cmw_clear_cb[] = {{(XtCallbackProc) ColorMixScratchPadClear, NULL}, NULL};
static XtCallbackRec cmw_sp_cancel_cb[] = {{(XtCallbackProc) ColorMixScratchPadCancel, NULL}, NULL};
static XtCallbackRec cmw_sp_bucket_cb[] = {{(XtCallbackProc) ColorMixScratchPadAddColor, NULL}, NULL};
static XtCallbackRec cmw_sp_input_cb[]  = {{(XtCallbackProc) ColorMixScratchPadInput, NULL}, NULL};
static XtCallbackRec cmw_browser_cb[] = {{(XtCallbackProc) ColorMixBrowserSelect, NULL}, NULL};
static XtCallbackRec cmw_bsbact_cb[] = {{(XtCallbackProc) ColorMixBrowserScroll, NULL}, NULL};
static XtCallbackRec cmw_gsscl_cb[] = {{(XtCallbackProc) ColorMixGreyscaleUpdate, NULL}, NULL};


/*-----------------------------------------------------*/
/* This procedure is called by the resource manager    */
/* in order to initialize the XmNdialogTitle resource. */
/* We couldn't initialize this with a static value     */
/* because we need to call DXmGetLocaleString to get   */
/* this value					       */
/*-----------------------------------------------------*/
static void GetDialogTitleString (widget, offset, value)
Widget   widget;
int      offset;
XrmValue * value;
{
    static XmString title = NULL;

    /* This routine can be called multiple times, so free the last */
    /* title string before allocating the next one.  Freeing it is */
    /* legal because the widget makes a copy of the title string   */
    /* after this procedure returns.				   */

    if (title != NULL)
	XmStringFree (title);

    title = DXmGetLocaleString((I18nContext)NULL, "Color Mixing", I18NADJECTIVE | I18NLABEL );

    value->size = sizeof (title);
    value->addr = (char *) &title;
}


/*---------------------------------------------------*/
/* "synthetic" resources for GetValues		     */
/*---------------------------------------------------*/

static XmSyntheticResource syn_resources[] = 
{
	{	DXmNmainLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.mainlabel), 
		_DXmColorMixGetMainLabelStr,
		NULL
	}, 

	{	DXmNdisplayLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.displabel), 
		_DXmColorMixGetDispLabelStr,
		NULL
	}, 

	{	DXmNmixerLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.mixlabel), 
		_DXmColorMixGetMixLabelStr,
		NULL
	}, 

	{	DXmNsliderLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.sldlabel), 
		_DXmColorMixGetSldLabelStr,
		NULL
	}, 

	{	DXmNvalueLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.vallabel), 
		_DXmColorMixGetValLabelStr,
		NULL
	}, 

	{	DXmNredLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.redlabel), 
		_DXmColorMixGetRedLabelStr,
		NULL
	}, 

	{	DXmNgreenLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.grnlabel), 
		_DXmColorMixGetGrnLabelStr,
		NULL	
	}, 

	{	DXmNblueLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.blulabel), 
		_DXmColorMixGetBluLabelStr,
		NULL
	}, 

	{	DXmNhueLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.huelabel), 
		_DXmColorMixGetHueLabelStr,
		NULL
	}, 

	{	DXmNlightLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.lightlabel), 
		_DXmColorMixGetLightLabelStr,
		NULL
	}, 

	{	DXmNsatLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.satlabel), 
		_DXmColorMixGetSatLabelStr,
		NULL
	}, 

	{	DXmNblackLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.blklabel), 
		_DXmColorMixGetBlkLabelStr,
		NULL
	}, 

	{	DXmNwhiteLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.whtlabel), 
		_DXmColorMixGetWhtLabelStr,
		NULL
	}, 

	{	DXmNgrayLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.grylabel), 
		_DXmColorMixGetGryLabelStr,
		NULL
	}, 

	{	DXmNfullLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.fullabel), 
		_DXmColorMixGetFulLabelStr,
		NULL
	}, 

	{	DXmNoptionLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.optlabel), 
		_DXmColorMixGetOptLabelStr,
		NULL
	}, 

	{	DXmNhlsLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.hlslabel), 
		_DXmColorMixGetHLSLabelStr,
		NULL
	}, 

	{	DXmNrgbLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.rgblabel), 
		_DXmColorMixGetRGBLabelStr,
		NULL
	}, 

	{	XmNokLabelString, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.oklabel), 
		_DXmColorMixGetOkLabelStr,
		NULL
	}, 

	{	XmNapplyLabelString, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.applabel), 
		_DXmColorMixGetApplyLabelStr,
		NULL
	}, 

	{	XmNcancelLabelString, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.canlabel), 
		_DXmColorMixGetCancelLabelStr,
		NULL
	}, 

	{	DXmNresetLabelString, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.reslabel), 
		_DXmColorMixGetResetLabelStr,
		NULL
	}, 

	{	DXmNpickerLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.pickerlabel), 
		_DXmColorMixGetPickerLabelStr,
		NULL
	}, 

	{	DXmNundoLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.undolabel), 
		_DXmColorMixGetUndoLabelStr,
		NULL
	}, 

	{	DXmNsmearLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.smearlabel), 
		_DXmColorMixGetSmearLabelStr,
		NULL
	}, 

#ifndef WIN32
	{	DXmNhelpLabel, 
#else
	{	"helpLabel", 
#endif
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.helplabel), 
		_DXmColorMixGetHelpLabelStr,
		NULL
	}, 

	{	DXmNpickerTitleLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.ptitlelabel), 
		_DXmColorMixGetPTitleLabelStr,
		NULL
	}, 

	{	DXmNspectrumLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.spectrumlabel), 
		_DXmColorMixGetSpectrumLabelStr,
		NULL
	}, 

	{	DXmNpastelLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.pastellabel), 
		_DXmColorMixGetPastelLabelStr,
		NULL
	}, 


	{	DXmNvividLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.metalliclabel), 
		_DXmColorMixGetMetallicLabelStr,
		NULL
	}, 


	{	DXmNearthtoneLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.earthtonelabel), 
		_DXmColorMixGetEarthLabelStr,
		NULL
	}, 


	{	DXmNuserPaletteLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.userpalettelabel), 
		_DXmColorMixGetUserPalLabelStr,
		NULL
	}, 


	{	DXmNinterpTitleLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.ititlelabel), 
		_DXmColorMixGetITitleLabelStr,
		NULL
	}, 

	{	DXmNwarmerLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.warmerlabel), 
		_DXmColorMixGetWarmerLabelStr,
		NULL
	}, 

	{	DXmNcoolerLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.coolerlabel), 
		_DXmColorMixGetCoolerLabelStr,
		NULL
	}, 

	{	DXmNlighterLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.lighterlabel), 
		_DXmColorMixGetLighterLabelStr,
		NULL
	}, 

	{	DXmNdarkerLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.darkerlabel), 
		_DXmColorMixGetDarkerLabelStr,
		NULL
	}, 

	{	DXmNclearLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.clearlabel), 
		_DXmColorMixGetClearLabelStr,
		NULL
	}, 

	{	DXmNscratchPadInfoLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.spinfolabel), 
		_DXmColorMixGetSPInfoLabelStr,
		NULL
	}, 

	{	DXmNscratchPadLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.splabel), 
		_DXmColorMixGetSPLabelStr,
		NULL
	}, 

	{	DXmNbrowserLabel, 
		sizeof (XmString), 
		XtOffset (DXmColorMixWidget, colormix.browserlabel), 
		_DXmColorMixGetBrowserLabelStr,
		NULL
	}, 

        {       DXmNgreyscaleLabel,
                sizeof (XmString),
                XtOffset (DXmColorMixWidget, colormix.greyscalelabel),
                _DXmColorMixGetGreysclLabelStr,
                NULL
        },

};


/*---------------------------------------------------*/
/* color mixing widget resource defaults 	     */
/*---------------------------------------------------*/

static const int 	resources_1		= 1;
static const int 	resources_0   		= 0;
static const unsigned short resources_5000   	= 5000;
static const Boolean 	resources_1_boolean   	= 1;
static const short 	resources_10_dimension	= 10;
static const short 	resources_20_dimension	= 20;
static const short 	resources_30_dimension	= 30;
static const short 	resources_80_dimension	= 80;
static const int 	resources_gray		= HALFCOLORVALUE;
static XtWidgetProc 	default_setdisplay_proc = (XtWidgetProc) SetDisplayNewColor;
static XtWidgetProc 	default_setmixer_proc   = (XtWidgetProc) SetMixerNewColor;
static const unsigned char resource_model	= DXmColorModelPicker;
static const short	resources_0_short	= 0;
static const short	resources_5_short	= 5;
static const short	resources_10_short	= 10;

/*---------------------------------------------------*/
/* color mixing widget resources                     */
/*---------------------------------------------------*/

static XtResource resources[] = 
{
    /* set superclass resources having different defaults */

    {XmNnavigationType, XmCNavigationType, XmRNavigationType, sizeof (unsigned char),
     	XtOffset(XmManagerWidget, manager.navigation_type),
     		XmRImmediate, (XtPointer) XmSTICKY_TAB_GROUP},

    {XmNmarginWidth, XmCMarginWidth, XmRShort, sizeof (short),
	XtOffset(XmBulletinBoardWidget, bulletin_board.margin_width), 
		XmRShort, (XtPointer)&resources_10_dimension},

    {XmNmarginHeight, XmCMarginHeight, XmRShort, sizeof (short),
	XtOffset(XmBulletinBoardWidget, bulletin_board.margin_height), 
		XmRShort, (XtPointer)&resources_10_dimension},

    {XmNautoUnmanage, XmCAutoUnmanage, XmRBoolean, sizeof(Boolean),
	XtOffset(XmBulletinBoardWidget, bulletin_board.auto_unmanage), 
		XmRImmediate, (XtPointer) FALSE},

    {XmNdialogTitle, XmCDialogTitle, XmRXmString, sizeof (XmString), 
	XtOffset (XmBulletinBoardWidget, bulletin_board.dialog_title), 
		XmRCallProc, (XtPointer) GetDialogTitleString},

    {XmNnoResize, XmCNoResize, XmRBoolean, sizeof (Boolean),
	XtOffset (XmBulletinBoardWidget, bulletin_board.no_resize),
		XmRImmediate, (XtPointer) TRUE},

    /* colormix specific resources */

    {DXmNmainLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.mainlabel), 
		XmRString, NULL},
 
    {DXmNdisplayLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.displabel), 
		XmRString, NULL},
 
    {DXmNmixerLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.mixlabel), 
		XmRString, NULL},
 
    {DXmNsliderLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.sldlabel), 
		XmRString, NULL},
 
    {DXmNvalueLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.vallabel), 
		XmRString, NULL},
 
    {DXmNredLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.redlabel), 
		XmRString, NULL},
 
    {DXmNgreenLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.grnlabel), 
		XmRString, NULL},
 
    {DXmNblueLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.blulabel), 
		XmRString, NULL},
 
    {DXmNhueLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.huelabel), 
		XmRString, NULL},
 
    {DXmNlightLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.lightlabel), 
		XmRString, NULL},
 
    {DXmNsatLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.satlabel), 
		XmRString, NULL},
 
    {DXmNblackLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.blklabel), 
		XmRString, NULL},
 
    {DXmNwhiteLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.whtlabel), 
		XmRString, NULL},
 
    {DXmNgrayLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.grylabel), 
		XmRString, NULL},
 
    {DXmNfullLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.fullabel), 
		XmRString, NULL},
 
    {DXmNoptionLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.optlabel), 
		XmRString, NULL},
 
    {DXmNhlsLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.hlslabel), 
		XmRString, NULL},
 
    {DXmNrgbLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.rgblabel), 
		XmRString, NULL},

    {DXmNpickerLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.pickerlabel), 
		XmRString, NULL},

    {XmNokLabelString, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.oklabel), 
		XmRString, NULL},
 
    {XmNapplyLabelString, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.applabel), 
		XmRString, NULL},
 
    {DXmNresetLabelString, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.reslabel), 
		XmRString, NULL},
 
    {XmNcancelLabelString, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.canlabel), 
		XmRString, NULL},
 
    {DXmNorigRedValue, DXmCOrigRedValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.origcolor.red), 
		DXmRColorValue, (XtPointer)&resources_0},

    {DXmNorigGreenValue, DXmCOrigGreenValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.origcolor.green), 
		DXmRColorValue, (XtPointer)&resources_0},

    {DXmNorigBlueValue, DXmCOrigBlueValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.origcolor.blue), 
		DXmRColorValue, (XtPointer)&resources_0},

    {DXmNnewRedValue, DXmCNewRedValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.newcolor.red), 
		DXmRColorValue, (XtPointer)&resources_0},

    {DXmNnewGreenValue, DXmCNewGreenValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.newcolor.green), 
		DXmRColorValue, (XtPointer)&resources_0},

    {DXmNnewBlueValue, DXmCNewBlueValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.newcolor.blue), 
		DXmRColorValue, (XtPointer)&resources_0},

    {DXmNbackRedValue, DXmCBackRedValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.backcolor.red), 
		DXmRColorValue, (XtPointer)&resources_gray},

    {DXmNbackGreenValue, DXmCBackGreenValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.backcolor.green), 
		DXmRColorValue, (XtPointer)&resources_gray},

    {DXmNbackBlueValue, DXmCBackBlueValue, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.backcolor.blue), 
		DXmRColorValue, (XtPointer)&resources_gray},

    {DXmNsetNewColorProc, DXmCSetNewColorProc, XmRProc, sizeof(char *),
        XtOffset (DXmColorMixWidget, colormix.setnewcolproc), 
		XmRProc, (XtPointer)&default_setdisplay_proc},

    {DXmNsetMixerColorProc, DXmCSetMixerColorProc, XmRProc, sizeof(char *),
        XtOffset (DXmColorMixWidget, colormix.setmixcolproc), 
		XmRProc, (XtPointer)&default_setmixer_proc},

    {DXmNmatchColors, DXmCMatchColors, XmRBoolean, sizeof (Boolean),
	  XtOffset(DXmColorMixWidget, colormix.matchcolors), 
		XmRBoolean, (XtPointer)&resources_1_boolean},

    {DXmNgreyscaleOnGreyscale, DXmCGreyscaleOnGreyscale, XmRBoolean, sizeof (Boolean),
	  XtOffset(DXmColorMixWidget, colormix.greyongrey), 
		XmRBoolean, (XtPointer)&resources_1_boolean},

    {XmNokCallback, XmCCallback, XmRCallback, sizeof (XtCallbackList),
	 XtOffset (DXmColorMixWidget, colormix.okcallback), 
		XmRImmediate, (XtPointer) NULL},

    {XmNapplyCallback, XmCCallback, XmRCallback, sizeof (XtCallbackList),
	 XtOffset (DXmColorMixWidget, colormix.applycallback), 
		XmRImmediate, (XtPointer) NULL},

    {XmNcancelCallback, XmCCallback, XmRCallback, sizeof (XtCallbackList),
	 XtOffset (DXmColorMixWidget, colormix.cancelcallback), 
		XmRImmediate, (XtPointer) NULL},

    {DXmNdisplayColWinWidth, DXmCDisplayColWinWidth, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.vieww), 
		XmRShort, (XtPointer)&resources_80_dimension},

    {DXmNdisplayColWinHeight, DXmCDisplayColWinHeight, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.viewh), 
		XmRShort, (XtPointer)&resources_80_dimension},

    {DXmNdispWinMargin, DXmCDispWinMargin, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.viewm), 
		XmRShort, (XtPointer)&resources_20_dimension},

    {DXmNdisplayWindow, XmCWindow, XmRWindow, sizeof (Widget),
	  XtOffset (DXmColorMixWidget, colormix.curdispwid),
		XmRWindow, (XtPointer) &resources_1},

    {DXmNorigDispWindow, XmCWindow, XmRWindow, sizeof (Widget),
	  XtOffset (DXmColorMixWidget, colormix.origwid),
		XmRWindow, (XtPointer) NULL},

    {DXmNnewDispWindow, XmCWindow, XmRWindow, sizeof (Widget),
	  XtOffset (DXmColorMixWidget, colormix.newwid),
		XmRWindow, (XtPointer) NULL},

    {DXmNmixerWindow, XmCWindow, XmRWindow, sizeof (Widget),
	  XtOffset (DXmColorMixWidget, colormix.curmixerwid),
		XmRWindow, (XtPointer) &resources_1},

    {XmNworkWindow, XmCWindow, XmRWindow, sizeof (Widget),
	  XtOffset (DXmColorMixWidget, colormix.workwid),
		XmRWindow, (XtPointer) NULL},

    {DXmNcolorModel, DXmCColorModel, DXmRColorModel, sizeof (unsigned char),
	XtOffset (DXmColorMixWidget, colormix.colormodel),
		DXmRColorModel, (XtPointer) &resource_model},

    {DXmNundoLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.undolabel), 
		XmRString, NULL},

    {DXmNsmearLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.smearlabel), 
		XmRString, NULL},

#ifndef WIN32
    {DXmNhelpLabel, XmCXmString, XmRXmString, sizeof(XmString),
#else
    {"helpLabel", XmCXmString, XmRXmString, sizeof(XmString),
#endif
         XtOffset (DXmColorMixWidget, colormix.helplabel), 
		XmRString, NULL},

    {DXmNpickerTitleLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.ptitlelabel), 
		XmRString, NULL},

    {DXmNspectrumLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.spectrumlabel), 
		XmRString, NULL},

    {DXmNpastelLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.pastellabel), 
		XmRString, NULL},

    {DXmNvividLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.metalliclabel), 
		XmRString, NULL},

    {DXmNearthtoneLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.earthtonelabel), 
		XmRString, NULL},

    {DXmNuserPaletteLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.userpalettelabel), 
		XmRString, NULL},

    {DXmNclearLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.clearlabel), 
		XmRString, NULL},

    {DXmNscratchPadInfoLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.spinfolabel), 
		XmRString, NULL},

    {DXmNscratchPadLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.splabel), 
		XmRString, NULL},

    {DXmNpickerTileHeight, DXmCPickerTileHeight, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.ptileheight), 
		XmRShort, (XtPointer)&resources_30_dimension},

    {DXmNpickerTileWidth, DXmCPickerTileWidth, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.ptilewidth), 
		XmRShort, (XtPointer)&resources_30_dimension},

    {DXmNpickerColors, DXmCPickerColors, DXmRPickerColors, sizeof(XtPointer),
	  XtOffset(DXmColorMixWidget, colormix.pcolors), 
		DXmRPickerColors, (XtPointer)&resources_0},

    {DXmNpickerColorCount, DXmCPickerColorCount, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.pcolorcount), 
		XmRShort, (XtPointer)&resources_0_short},

    {DXmNinterpTitleLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.ititlelabel), 
		XmRString, NULL},

    {DXmNinterpTileHeight, DXmCInterpTileHeight, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.itileheight), 
		XmRShort, (XtPointer)&resources_30_dimension},

    {DXmNinterpTileWidth, DXmCInterpTileWidth, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.itilewidth), 
		XmRShort, (XtPointer)&resources_30_dimension},

    {DXmNinterpTileCount, DXmCInterpTileCount, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.itilecount), 
		XmRShort, (XtPointer)&resources_10_short},

    {DXmNwarmerLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.warmerlabel), 
		XmRString, NULL},

    {DXmNcoolerLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.coolerlabel), 
		XmRString, NULL},

    {DXmNlighterLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.lighterlabel), 
		XmRString, NULL},

    {DXmNdarkerLabel, XmCXmString, XmRXmString, sizeof(XmString),
         XtOffset (DXmColorMixWidget, colormix.darkerlabel), 
		XmRString, NULL},

    {DXmNwarmthIncrement, DXmCWarmthIncrement, DXmRColorValue, sizeof (unsigned short),
	  XtOffset(DXmColorMixWidget, colormix.warmthinc), 
		DXmRColorValue, (XtPointer)&resources_5000},

    {DXmNlightnessIncrement, DXmCLightnessIncrement, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.lightnessinc), 
		XmRShort, (XtPointer)&resources_5_short},

    {DXmNbrowserItemCount, DXmCBrowserItemCount, XmRShort, sizeof (short),
	  XtOffset(DXmColorMixWidget, colormix.bitemcount), 
		XmRShort, (XtPointer)&resources_5_short},

    {DXmNbrowserLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.browserlabel), 
		XmRString, NULL},

    {DXmNgreyscaleLabel, XmCXmString, XmRXmString, sizeof (XmString),
         XtOffset (DXmColorMixWidget, colormix.greyscalelabel),
		XmRString, NULL},

};


/*---------------------------------------------------*/
/* class record declaration			     */
/*---------------------------------------------------*/

externaldef(dxmcolormixclassrec) DXmColorMixClassRec dxmColorMixClassRec = 
{
    {
    	/* superclass	      */	(WidgetClass) &xmBulletinBoardClassRec, 
    	/* class_name	      */	"DXmColorMix",
    	/* widget_size	      */	sizeof(DXmColorMixRec),
    	/* class_initialize   */    	(XtProc) ClassInitialize,
    	/* chained class init */	NULL,
    	/* class_inited       */	FALSE,
    	/* initialize	      */	(XtInitProc) Initialize,
    	/* initialize hook    */	NULL,
    	/* realize	      */	(XtRealizeProc) Realize,
    	/* actions	      */	ActionsTable,
    	/* num_actions	      */	XtNumber(ActionsTable),
    	/* resources	      */	resources,
    	/* num_resources      */	XtNumber(resources),
    	/* xrm_class	      */	NULLQUARK,
    	/* compress_motion    */	TRUE,
    	/* compress_exposure  */	TRUE,
    	/* compress enter/exit*/	FALSE,
    	/* visible_interest   */	FALSE,
    	/* destroy	      */	(XtWidgetProc) Destroy,
    	/* resize	      */	XtInheritResize,
    	/* expose	      */	XtInheritExpose,
    	/* set_values	      */	(XtSetValuesFunc) SetValues,
	/* set values hook    */	NULL,                    
	/* set values almost  */	XtInheritSetValuesAlmost,
	/* get values hook    */	NULL,                    
    	/* accept_focus	      */	XtInheritAcceptFocus,
	/* version	      */	XtVersion,
        /* callback offset    */        NULL,
        /* defalut translation*/        XtInheritTranslations,
    	/* query geo proc     */	XtInheritQueryGeometry,
    	/* disp accelerator   */	NULL,
	/* extension	      */	NULL,
    },

    {	/* composite class record */    

	/* childrens geo mgr proc   */  (XtGeometryHandler) GeometryManager,
	/* set changed proc	    */	(XtWidgetProc) ChangeManaged,
	/* add a child		    */	XtInheritInsertChild,
	/* remove a child	    */	(XtWidgetProc) RemoveChild,
	/* extension		    */	NULL,
    },

    {	/* constraint class record */

	/* no additional resources  */	NULL,
	/* num additional resources */	0,
	/* size of constraint rec   */	0,
	/* constraint_initialize    */	NULL,
	/* constraint_destroy	    */	NULL,
	/* constraint_setvalue	    */	NULL,
	/* extension		    */	NULL,
    },

    {	/* manager class record */
      XtInheritTranslations,			/* default translations   */
      syn_resources, 				/* get resources      	  */
      XtNumber (syn_resources), 		/* num get_resources 	  */
      NULL, 					/* get_cont_resources     */
      0, 					/* num_get_cont_resources */
      XmInheritParentProcess,			/* parent_process	  */
      NULL, 					/* extension		  */
    }, 

    {	/* bulletin board class record */     
	FALSE,
        XmInheritGeoMatrixCreate,                /* geo_matrix_create */
	NULL, 					/* extension */
    }, 	

    {	/* colormix class record */

	/* extension		    */	NULL,
    }
};

externaldef(dxmcolormixwidgetclass) WidgetClass dxmColorMixWidgetClass = (WidgetClass)& dxmColorMixClassRec;



/*---------------------------------------------------*/
/* core and composite class routines 		     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/* initialize the colormix class 		     */
/*---------------------------------------------------*/

static void ClassInitialize()
{
    /* parse subwidget translation tables */

    cmwtext_translations_parsed = 
			XtParseTranslationTable (cmwtext_translation_table);
    cmwdisp_translations_parsed = 	  
		XtParseTranslationTable (cmwdisp_translation_table);
    cmwpick_translations_parsed = 
			XtParseTranslationTable (cmwpick_translation_table);
    cmwinterp_translations_parsed = 
			XtParseTranslationTable (cmwinterp_translation_table);
    cmwsp_translations_parsed = 
			XtParseTranslationTable (cmwsp_translation_table);
}


                           
/*---------------------------------------------------*/
/* initialize an instance of the colormix widget     */
/*---------------------------------------------------*/

static void Initialize(request,cmw)
    DXmColorMixWidget request,cmw;
{
    Arg	al[1];

    InitFields(cmw);

    if ((ColLayoutDirection(cmw) == DXmLAYOUT_LEFT_DOWN))
       ColDirection(cmw) = XmSTRING_DIRECTION_R_TO_L;
 
    CreateColShell(cmw);

    if (ColCurDispWid(cmw) == (Widget) 1)
    {
    	CreateColDisplay(cmw);
	ColCurDispWid(cmw) = (Widget) ColDispWid(cmw);
	ColDefDisp(cmw) = TRUE;
    }

    XtManageChild(ColCurDispWid(cmw));

    CreateColHLSMixer(cmw);
    CreateColRGBMixer(cmw);
    CreateColPickerMixer(cmw);
    CreateColBrowserMixer(cmw);
    CreateColGrayscaleMixer(cmw);
    CreateColScratchPad(cmw);

    PopulateColorMixOptionMenu(cmw);

    if (ColCurMixerWid(cmw) == (Widget) 1)
    {
        switch (ColModel(cmw))
        {
            case DXmColorModelHLS:
		ColMixerWid(cmw) = (Widget) ColHLSMixerWid(cmw);
		break;

            case DXmColorModelRGB:
		ColMixerWid(cmw) = (Widget) ColRGBMixerWid(cmw);
		break;		

            case DXmColorModelBrowser:
		ColMixerWid(cmw) = (Widget) ColBrowserMixerWid(cmw);
		break;		

            case DXmColorModelGreyscale:
		ColMixerWid(cmw) = (Widget) ColGreyscaleMixerWid(cmw);
		break;		

	    default:
		ColMixerWid(cmw) = (Widget) ColPickerMixerWid(cmw);
		break;		
	}

	ColCurMixerWid(cmw) = (Widget) ColMixerWid(cmw);
	ColDefMixer(cmw) = TRUE;

        XtManageChild(ColOptMenWid(cmw));
    }

    XtManageChild(ColCurMixerWid(cmw));

    XtAddCallbacks((Widget)cmw,
		   XmNmapCallback,
		   cmw_map_cb);
 
    XtAddCallbacks((Widget)cmw,
		   XmNunmapCallback,
		   cmw_unmap_cb);

    XmAddTabGroup(ColDispWid(cmw));
    XmAddTabGroup(ColOptMenWid(cmw));
    XmAddTabGroup(ColPickerOptMenWid(cmw)); 
    XmAddTabGroup(ColPickerFrameWid(cmw));
    XmAddTabGroup(ColInterpFrameWid(cmw));
    XmAddTabGroup(ColPickerMixerWid(cmw));	
    XmAddTabGroup(ColHLSMixerWid(cmw));
    XmAddTabGroup(ColRGBMixerWid(cmw));	
    XmAddTabGroup(ColBrowserMixerWid(cmw));	
    XmAddTabGroup(ColGreyscaleMixerWid(cmw));

    ResetLabelFields(cmw);
}

                           
/*---------------------------------------------------*/
/* realize the color mix widget	     		     */
/*---------------------------------------------------*/
 
static void Realize(cmw,window_mask,window_attributes)
DXmColorMixWidget        cmw;
Mask                 *window_mask;           
XSetWindowAttributes *window_attributes;
{
    (*xmBulletinBoardWidgetClass->core_class.realize) 
					((Widget)cmw,window_mask,window_attributes);
    DetermineDisplayType(cmw);

    if (ColDispType(cmw) == GryScale) 
	SetPickerOptionSensitivity (cmw, FALSE);

    if ((ColDispType(cmw) == GryScale) && (ColGreyOnGrey(cmw)))
    {
	ColModel(cmw) = DXmColorModelGreyscale;
	UpdateMixer(cmw);
	UpdateMenu(cmw);
    }

    if (ColDefDisp(cmw))
    {
    	if (ColMatchColors(cmw))
	    MatchNewToOrig(cmw, FALSE);

	if (ColDispType(cmw) == BlackAndWhite || ColDispType(cmw) == StatGray)
	{
	    RemoveColDisplay(cmw);
	    RemoveScratchPad(cmw);
	}
	else 
	{
	    if (ColDispType(cmw) == DynColor || ColDispType(cmw) == GryScale)
		AllocDynColors(cmw);

	    SetUpColors(cmw);
	}
    }

    /*
     * if not a popup, then Map callback will not be called.  Therefore,
     * call map routine (sizing and color alloc code) here...
     */
    if (!ColIsPopup(cmw))
	ColorMixDispMap(cmw);

}
 

/*---------------------------------------------------*/
/* deallocate the widget specific resources 	     */
/*---------------------------------------------------*/

static void Destroy (cmw)
    DXmColorMixWidget cmw;
{
    XtRemoveAllCallbacks ((Widget)cmw, XmNokCallback);
    XtRemoveAllCallbacks ((Widget)cmw, XmNapplyCallback);
    XtRemoveAllCallbacks ((Widget)cmw, XmNcancelCallback);

    if (ColPickerColors(cmw) != NULL)
	XtFree ((char *)ColPickerColors(cmw));

    if (ColPickerXColors(cmw) != NULL)
	XtFree ((char *)ColPickerXColors(cmw));

    if (ColInterpXColors(cmw) != NULL)
	XtFree ((char *)ColInterpXColors(cmw));

    if (ColInterpHLSValues(cmw) != NULL)
	XtFree ((char *)ColInterpHLSValues(cmw));

    if (ColScratchColors(cmw) != NULL)
	XtFree ((char *)ColScratchColors(cmw));

    if (ColBrowserColors(cmw) != NULL)
	XtFree ((char *)ColBrowserColors(cmw));

    if (ColBrowserXColors(cmw) != NULL)
	XtFree ((char *)ColBrowserXColors(cmw));

    if (ColBrowserPbWid(cmw) != NULL)
	XtFree ((char *)ColBrowserPbWid(cmw));

}


/*---------------------------------------------------*/
/* colormix setvalues				     */
/*---------------------------------------------------*/

static Boolean SetValues(old, request, new)
Widget   old,request,new;
{      
    DXmColorMixWidget oldcmw = (DXmColorMixWidget) old;
    DXmColorMixWidget newcmw = (DXmColorMixWidget) new;

    Boolean NewMargins, NewDispWid, NewMixWid, NewColors, NewColModel, 
            NewDispMargins, NewLabel, NewOptLab, NewTileSize, 
	    NewLayoutDirection; 
    
    /* insure that dialog child overlap is always TRUE */

    if (newcmw->bulletin_board.allow_overlap != TRUE)
	newcmw->bulletin_board.allow_overlap = TRUE;

    NewMargins = CheckMargins(oldcmw,newcmw);

    NewLabel = IsColMainLab(newcmw)	    || IsColDispLab(newcmw)	    ||
	       IsColMixLab(newcmw)	    || IsColOkLab(newcmw)	    ||
	       IsColAppLab(newcmw)	    || IsColResLab(newcmw)	    ||
	       IsColCanLab(newcmw)	    || IsColPDLab(newcmw)	    ||
	       IsColPDHLSLab(newcmw)	    || IsColPDRGBLab(newcmw)	    ||
	       IsColRedLab(newcmw)	    || IsColGrnLab(newcmw)	    ||
	       IsColBluLab(newcmw)	    || IsColValLab(newcmw)	    ||
	       IsColSldLab(newcmw)	    || IsColHueLab(newcmw)	    ||
	       IsColLightLab(newcmw)	    || IsColSatLab(newcmw)	    ||
	       IsColBlkLab(newcmw)	    || IsColWhtLab(newcmw)	    ||
	       IsColGryLab(newcmw)	    || IsColFulLab(newcmw)	    ||
	       IsColUndoLab(newcmw)	    || IsColUserPaletteLab(newcmw)  ||
	       IsColSpectrumLab(newcmw)	    || IsColPastelLab(newcmw)	    ||
	       IsColMetallicLab(newcmw)	    || IsColEarthtoneLab(newcmw)    ||
	       IsColSmearLab(newcmw)	    || IsColPDPickerLab(newcmw)	    ||
	       IsColPDBrowserLab(newcmw)    || IsColPDGreyscaleLab(newcmw)  ||
	       IsColITitleLab(newcmw)	    || IsColHelpLab(newcmw)	    ||
	       IsColClearLab(newcmw)	    || IsColSPInfoLab(newcmw)	    ||
	       IsColScratchPadLab(newcmw);


    NewOptLab = (IsColPDHLSLab(newcmw) || IsColPDRGBLab(newcmw) || 
		 IsColPDPickerLab(newcmw) || IsColPDBrowserLab(newcmw) ||
		 IsColPDGreyscaleLab(newcmw));

    {
	int save_x, save_y, save_w, save_h, save_bw;

	/*
	 *  Do any setvalues now for sub-widgets
	 *
	 *  NOTE:  HACK ALERT!
	 *  These set values requests may come back through
 	 *  this widget as geometry requests, which can
	 *  also result in this widget doing a geometry request.
	 *  If left to itself, this will interfere with the
	 *  "automatic" geometry request done at the end of
	 *  XtSetValues.  To get around this, we do the following:
	 * 
	 *	1. Save the geometry fields of the new widget
	 *	   and put back in the "real" values from the
	 *	   current widget.
	 *	2. Do the XtSetValues on the children.  This
	 *         may result in the geometry fields of the
	 *	   parent widget being modified.
	 *	3. Copy the geometry fields from the new widget
	 *	   back to the "current" widget so that they
	 *	   reflect reality.
	 *	4. Copy the saved x, y, and borderwidth fields 
	 *         back into the new widget so that the "automatic"
	 *	   geometry request will handle these.
	 *	5. If the requested width or height is different
	 *	   than the old width or height, copy it back into 
	 *	   the new widget so that the "automatic" geometry request
	 *         will handle it.
	 */         

	/* 1 */
	DXMGETWIDGETPARAMS(newcmw, save_x, save_y, save_w, save_h, save_bw);
	DXMCOPYWIDGETGEOMETRY(oldcmw, newcmw);

	/* 2 */
    	if (IsColMainLab(newcmw))
            UpdateText(newcmw, ColMainLabWid(newcmw), &ColMainLab(newcmw));

    	if (IsColDispLab(newcmw))
            UpdateText(newcmw, ColDispLabWid(newcmw), &ColDispLab(newcmw));

    	if (IsColMixLab(newcmw))
            UpdateText(newcmw, ColMixLabWid(newcmw), &ColMixLab(newcmw));

    	if (IsColOkLab(newcmw))
            UpdateText(newcmw, ColOkPbWid(newcmw), &ColOkLab(newcmw));

    	if (IsColAppLab(newcmw))
            UpdateText(newcmw, ColAppPbWid(newcmw), &ColAppLab(newcmw));

    	if (IsColResLab(newcmw))
            UpdateText(newcmw, ColResPbWid(newcmw), &ColResLab(newcmw));

    	if (IsColCanLab(newcmw))
            UpdateText(newcmw, ColCanPbWid(newcmw), &ColCanLab(newcmw));

    	if (IsColPDLab(newcmw))
            UpdateText(newcmw, ColOptLabWid(newcmw), &ColPDLab(newcmw));

    	if (IsColPDHLSLab(newcmw))
            UpdateText(newcmw, ColPDHLSWid(newcmw), &ColPDHLSLab(newcmw));

    	if (IsColPDRGBLab(newcmw))
            UpdateText(newcmw, ColPDRGBWid(newcmw), &ColPDRGBLab(newcmw));

    	if (IsColPDPickerLab(newcmw))
            UpdateText(newcmw, ColPDPickerWid(newcmw), &ColPDPickerLab(newcmw));

    	if (IsColValLab(newcmw))
            UpdateText(newcmw, ColValLabWid(newcmw), &ColValLab(newcmw));

    	if (IsColSldLab(newcmw))
            UpdateText(newcmw, ColSldLabWid(newcmw), &ColSldLab(newcmw));

    	if (IsColRedLab(newcmw))
            UpdateText(newcmw, ColRedLabWid(newcmw), &ColRedLab(newcmw));

    	if (IsColGrnLab(newcmw))
            UpdateText(newcmw, ColGrnLabWid(newcmw), &ColGrnLab(newcmw));

    	if (IsColBluLab(newcmw))
            UpdateText(newcmw, ColBluLabWid(newcmw), &ColBluLab(newcmw));

    	if (IsColHueLab(newcmw))
            UpdateText(newcmw, ColHueLabWid(newcmw), &ColHueLab(newcmw));

    	if (IsColLightLab(newcmw))
            UpdateText(newcmw, ColLightLabWid(newcmw), &ColLightLab(newcmw));

    	if (IsColSatLab(newcmw))
            UpdateText(newcmw, ColSatLabWid(newcmw), &ColSatLab(newcmw));

    	if (IsColBlkLab(newcmw))
            UpdateText(newcmw, ColBlkLabWid(newcmw), &ColBlkLab(newcmw));

    	if (IsColWhtLab(newcmw))
            UpdateText(newcmw, ColWhtLabWid(newcmw), &ColWhtLab(newcmw));

    	if (IsColGryLab(newcmw))
            UpdateText(newcmw, ColGryLabWid(newcmw), &ColGryLab(newcmw));

    	if (IsColFulLab(newcmw))
            UpdateText(newcmw, ColFulLabWid(newcmw), &ColFulLab(newcmw));

    	if (IsColUndoLab(newcmw))
            UpdateText(newcmw, ColUndoPbWid(newcmw), &ColUndoLab(newcmw));

    	if (IsColSmearLab(newcmw))
            UpdateText(newcmw, ColSmearPbWid(newcmw), &ColSmearLab(newcmw));

    	if (IsColHelpLab(newcmw))
            UpdateText(newcmw, ColHelpPbWid(newcmw), &ColHelpLab(newcmw));

    	if (IsColSpectrumLab(newcmw))
            UpdateText(newcmw, ColSpectrumPbWid(newcmw), &ColSpectrumLab(newcmw));

    	if (IsColPastelLab(newcmw))
            UpdateText(newcmw, ColPastelPbWid(newcmw), &ColPastelLab(newcmw));

    	if (IsColMetallicLab(newcmw))
            UpdateText(newcmw, ColMetallicPbWid(newcmw), &ColMetallicLab(newcmw));

    	if (IsColEarthtoneLab(newcmw))
            UpdateText(newcmw, ColEarthtonePbWid(newcmw), &ColEarthtoneLab(newcmw));

    	if (IsColUserPaletteLab(newcmw))
            UpdateText(newcmw, ColUserPalettePbWid(newcmw), &ColUserPaletteLab(newcmw));

    	if (IsColITitleLab(newcmw))
            UpdateText(newcmw, ColITitleLabWid(newcmw), &ColITitleLab(newcmw));

    	if (IsColWarmerLab(newcmw))
            UpdateText(newcmw, ColWarmerPbWid(newcmw), &ColWarmerLab(newcmw));

    	if (IsColCoolerLab(newcmw))
            UpdateText(newcmw, ColCoolerPbWid(newcmw), &ColCoolerLab(newcmw));

    	if (IsColLighterLab(newcmw))
            UpdateText(newcmw, ColLighterPbWid(newcmw), &ColLighterLab(newcmw));

    	if (IsColDarkerLab(newcmw))
            UpdateText(newcmw, ColDarkerPbWid(newcmw), &ColDarkerLab(newcmw));

    	if (IsColClearLab(newcmw))
            UpdateText(newcmw, ColSPClearPbWid(newcmw), &ColClearLab(newcmw));

    	if (IsColSPInfoLab(newcmw))
            UpdateText(newcmw, ColSPInfoLabWid(newcmw), &ColSPInfoLab(newcmw));

    	if (IsColScratchPadLab(newcmw))
            UpdateText(newcmw, ColScratchPadPbWid(newcmw), &ColScratchPadLab(newcmw));

    	if (IsColPDBrowserLab(newcmw))
            UpdateText(newcmw, ColPDBrowserWid(newcmw), &ColPDBrowserLab(newcmw));

    	if (IsColPDGreyscaleLab(newcmw))
            UpdateText(newcmw, ColPDGreyscaleWid(newcmw), &ColPDGreyscaleLab(newcmw));

    	NewDispWid = CheckAndUpdateDispWid(oldcmw,newcmw);

    	NewMixWid = CheckAndUpdateMixerWid(oldcmw,newcmw);

    	NewColors = CheckAndUpdateColors(oldcmw,newcmw);

    	NewColModel = CheckAndUpdateColorModel(oldcmw,newcmw);

        NewDispMargins = CheckAndUpdateDispMargins(oldcmw,newcmw);

	NewTileSize = CheckAndUpdateTileSizes(oldcmw,newcmw);

	NewLayoutDirection = CheckAndUpdateLayoutDir(oldcmw,newcmw);

	if (NewOptLab)
	{
	    if (!NewColModel)
		UpdateMenu(newcmw);
        }

	if (NewMargins || NewDispWid || NewMixWid || NewColModel || 
	    NewDispMargins || NewLabel || NewTileSize || NewLayoutDirection)

	    LayoutColorMixWidget(newcmw);

	CheckAndUpdatePickerColors(oldcmw, newcmw);

	/* 3 */
	DXMCOPYWIDGETGEOMETRY(newcmw, oldcmw);

	/* 4 */
	newcmw->core.x = save_x;
	newcmw->core.y = save_y;
	newcmw->core.border_width = save_bw;

	/* 5 */
	if (save_h != oldcmw->core.height)
	    newcmw->core.height = save_h;
	if (save_w != oldcmw->core.width)
	    newcmw->core.width = save_w;
    }

    return (Boolean) (FALSE);
}


/*-----------------------------------------------------*/
/* colormix widget geometry manager - calls superclass */
/* geometry and then checks if colormix kid changed    */
/* size						       */
/*-----------------------------------------------------*/

static XtGeometryResult GeometryManager (w, desired, allowed)
    Widget  w;
    XtWidgetGeometry *desired, *allowed;
{
    XmBulletinBoardWidgetClass superclass;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent (w);
    XtGeometryResult  a = XtGeometryNo;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    a = (*superclass->composite_class.geometry_manager) (w,desired,allowed);

    /* 
     * if colormix kid and GeometryYes - then re-layout widgets
     */
    if (w == (Widget) ColDispWid(cmw) || w == (Widget) ColMixerWid(cmw) 
			|| w == (Widget) ColWorkWid(cmw))
        if (a == XtGeometryYes)
	    LayoutColorMixWidget(cmw);

    return (a);
}


/*---------------------------------------------------*/
/* called when widget first becomes visible - this   */
/* routine calls LayoutColorMixWidget before calling dialog */
/* managedSetChanged.  It is also called whenever    */
/* the number of managed children changes.	     */
/*---------------------------------------------------*/
static void ChangeManaged(cmw)
    DXmColorMixWidget cmw;
{

    if (ColNoLayout(cmw))
        return;

    LayoutColorMixWidget(cmw);
}


/*---------------------------------------------------*/
/* remove child from colormix widget - insures that  */
/* widget private fields remain current.	     */
/*---------------------------------------------------*/

static void RemoveChild (child)
    Widget    child;
{
    XmBulletinBoardWidgetClass superclass;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent (child);
    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    /*                                           
     * use the dialog class remove proc to do all the dirty work
     */                    

    (*superclass->composite_class.delete_child) (child);

    /*
     * check if removing colormix sub-widget & reset fields 
     * if necessary
     */

    if (child == (Widget) ColDispWid(cmw))
	ColDispWid(cmw) = NULL;
    else if (child == (Widget) ColMixerWid(cmw))
	ColMixerWid(cmw) = NULL;
    else if (child == (Widget) ColWorkWid(cmw))
	ColWorkWid(cmw) = NULL;
}


                           
/*---------------------------------------------------*/
/* initialization routines 			     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/* initializes private and public widget fields      */
/*---------------------------------------------------*/

static void InitFields(cmw)
    DXmColorMixWidget cmw;
{
    /* 
     * set dialog box fields 
     */

    cmw->manager.unit_type = XmPIXELS;
    cmw->bulletin_board.allow_overlap = TRUE;
    cmw->bulletin_board.resize_policy = XmRESIZE_ANY;

    if (XtHasCallbacks ((Widget)cmw, XmNhelpCallback) != XtCallbackHasSome)
	XtAddCallback ((Widget)cmw, XmNhelpCallback, (XtCallbackProc) ColorMixHelp, (XtPointer) NULL);

    if (cmw->bulletin_board.dialog_style != XmDIALOG_APPLICATION_MODAL
 				&& 
	cmw->bulletin_board.dialog_style != XmDIALOG_MODELESS)
    {
	cmw->bulletin_board.dialog_style = XmDIALOG_MODELESS;
    }

    /* 
     * set color mixing widget fields 
     */

    InitDefaultLabels(cmw);

    ColTextCols(cmw) = 5;

    ColDefDisp(cmw)		= FALSE;
    ColDefMixer(cmw)		= FALSE;
    ColAllocNewColor(cmw)	= FALSE;
    ColAllocOrigColor(cmw)	= FALSE;
    ColAllocBackColor(cmw)	= FALSE;
    ColHold(cmw)		= FALSE;
    ColNoLayout(cmw)		= FALSE;
    ColIsPointerGrabbed(cmw)	= FALSE;
    ColAllocPickerXColors(cmw)	= FALSE;
    ColAllocInterpXColors(cmw)	= FALSE;
    ColAllocBrowserXColors(cmw) = FALSE;
    ColAllocScratchColor(cmw)   = FALSE;
    ColIsScratchPadManaged(cmw) = FALSE;
    ColInitBrowserColors(cmw)	= FALSE;
    ColIsPickerSelected(cmw)	= FALSE;
    ColIsInterpSelected(cmw)	= FALSE;
    ColIsNewSelected(cmw)	= FALSE;
    ColIsOrigSelected(cmw)	= FALSE;
    ColIsScratchPadSelected(cmw) = FALSE;
    ColIsInterpSensitive(cmw)	= TRUE;

    ColMainLabWid(cmw) 		= NULL;
    ColDispLabWid(cmw) 		= NULL;
    ColMixLabWid(cmw) 		= NULL;
    ColRedLabWid(cmw)  		= NULL;
    ColGrnLabWid(cmw)  		= NULL;
    ColBluLabWid(cmw)  		= NULL;
    ColHueLabWid(cmw)  		= NULL;
    ColLightLabWid(cmw) 	= NULL;
    ColSatLabWid(cmw)  		= NULL;
    ColRedSclWid(cmw)  		= NULL;
    ColGrnSclWid(cmw)  		= NULL;
    ColBluSclWid(cmw)  		= NULL;
    ColRedTextWid(cmw)  	= NULL;
    ColGrnTextWid(cmw)  	= NULL;
    ColBluTextWid(cmw)  	= NULL;
    ColSldLabWid(cmw)   	= NULL;
    ColValLabWid(cmw)   	= NULL;
    ColOkPbWid(cmw)  		= NULL;
    ColAppPbWid(cmw)  		= NULL;
    ColResPbWid(cmw)  		= NULL;
    ColCanPbWid(cmw)  		= NULL;
    ColDispWid(cmw)  		= NULL;
    ColMixerWid(cmw)  		= NULL;
    ColRGBMixerWid(cmw) 	= NULL;
    ColHLSMixerWid(cmw) 	= NULL;
    ColPickerMixerWid(cmw) 	= NULL;
    ColHueSclWid(cmw)  		= NULL;
    ColLightSclWid(cmw) 	= NULL;
    ColSatSclWid(cmw)  		= NULL;
    ColPDMenWid(cmw)  		= NULL;
    ColOptMenWid(cmw)  		= NULL;
    ColPDHLSWid(cmw)  		= NULL;
    ColPDRGBWid(cmw)  		= NULL;
    ColPDPickerWid(cmw) 	= NULL;
    ColPickerDAWid(cmw) 	= NULL;
    ColInterpDAWid(cmw) 	= NULL;
    ColPickerFrameWid(cmw) 	= NULL;
    ColInterpFrameWid(cmw) 	= NULL;
    ColPickerPDWid(cmw) 	= NULL;
    ColPickerOptMenWid(cmw) 	= NULL;
    ColUndoPbWid(cmw)  		= NULL;
    ColSmearPbWid(cmw)  	= NULL;
    ColHelpPbWid(cmw)  		= NULL;
    ColLeftBucketPbWid(cmw)  	= NULL;
    ColRightBucketPbWid(cmw)  	= NULL;
    ColITitleLabWid(cmw) 	= NULL;
    ColSpectrumPbWid(cmw) 	= NULL;
    ColPastelPbWid(cmw) 	= NULL;
    ColMetallicPbWid(cmw) 	= NULL;
    ColEarthtonePbWid(cmw) 	= NULL;
    ColUserPalettePbWid(cmw) 	= NULL;
    ColWarmerLabWid(cmw) 	= NULL;
    ColCoolerLabWid(cmw) 	= NULL;
    ColLighterLabWid(cmw) 	= NULL;
    ColDarkerLabWid(cmw) 	= NULL;
    ColWarmerPbWid(cmw) 	= NULL;
    ColCoolerPbWid(cmw) 	= NULL;
    ColLighterPbWid(cmw) 	= NULL;
    ColDarkerPbWid(cmw) 	= NULL;
    ColScratchPadPbWid(cmw) 	= NULL;
    ColSPPopupWid(cmw) 		= NULL;
    ColSPInfoLabWid(cmw) 	= NULL;
    ColSPClearPbWid(cmw) 	= NULL;
    ColSPCancelPbWid(cmw) 	= NULL;
    ColSPScrolledWWid(cmw) 	= NULL;
    ColSPScrollBarWid(cmw) 	= NULL;
    ColSPDrawingAreaWid(cmw) 	= NULL;
    ColSPFrameWid(cmw) 		= NULL;
    ColSPBucketPbWid(cmw) 	= NULL;
    ColPDBrowserWid(cmw)	= NULL;
    ColBrowserMixerWid(cmw)	= NULL;
    ColBrowserSWWid(cmw)	= NULL;
    ColBrowserBBWid(cmw)	= NULL;
    ColBrowserSBWid(cmw)	= NULL;
    ColBrowserPbWid(cmw)	= NULL;
    ColPDGreyscaleWid(cmw)	= NULL;
    ColGreyscaleMixerWid(cmw)	= NULL;
    ColGreyscaleIndLabWid(cmw)	= NULL;
    ColGreyscaleSclWid(cmw)	= NULL;
    ColHelpWid(cmw)		= NULL;

    ColPickerSelectTile(cmw)    = 0;
    ColInterpSelectTile(cmw)   	= 0;

    ColNamedColor(cmw) 		= NULL;
    ColHighlightedColor(cmw) 	= NULL;
    ColGrabColor(cmw) 		= NULL;
  
    if (ColMatchColors(cmw))
	MatchNewToOrig(cmw, FALSE);

    ColTimerInterval(cmw) = (unsigned long) 250;
    ColTimer(cmw) = (XtIntervalId) 0;

    ColPickerColorCount(cmw) = 10;

    if (ColInterpTileCount(cmw) <  3)
	ColInterpTileCount(cmw) = 3;

    ColRealInterpTileCount(cmw) = ColInterpTileCount(cmw);

    ColBrowserColorCount(cmw) = XtNumber (default_browser_colors);
    ColBrowserColors(cmw) = (BrowserColor) default_browser_colors;
	
    ColLastInterpSide(cmw) = -1;
    ColDropHelpDialog(cmw) = (Widget) NULL;

    InitDefaultBrowserStrings (cmw);

    if (ColBrowserItemCount(cmw) <  1)
	ColBrowserItemCount(cmw) = 1;
	
    if (ColPickerColors(cmw) != NULL)
	CopyPickerColorList (cmw);

    InitPickerColorArray (cmw);
    InitPickerCursor (cmw);
    InitInterpColorArray (cmw);
    InitSPColorArray (cmw);
    InitBrowserColorArray (cmw);
}

                           
/*---------------------------------------------------*/
/* initializes colormix default labels		     */
/*---------------------------------------------------*/

static void InitDefaultLabels(cmw)
    DXmColorMixWidget cmw;
{
    if (ColSldLab(cmw) == NULL)
	ColSldLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Percentage", I18NNOUN | I18NLABEL );
 
    if (ColValLab(cmw) == NULL)
	ColValLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Value", I18NNOUN | I18NLABEL );
 
    if (ColRedLab(cmw) == NULL)
	ColRedLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Red", I18NNOUN | I18NLABEL );
 
    if (ColGrnLab(cmw) == NULL)
	ColGrnLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Green", I18NNOUN | I18NLABEL );
 
    if (ColBluLab(cmw) == NULL)
	ColBluLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Blue", I18NNOUN | I18NLABEL );
 
    if (ColHueLab(cmw) == NULL)
	ColHueLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Hue", I18NNOUN | I18NLABEL );
 
    if (ColLightLab(cmw) == NULL)
	ColLightLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Lightness", I18NNOUN | I18NLABEL );
 
    if (ColSatLab(cmw) == NULL)
	ColSatLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Saturation", I18NNOUN | I18NLABEL );
 
    if (ColBlkLab(cmw) == NULL)
	ColBlkLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Black", I18NNOUN | I18NLABEL );
 
    if (ColWhtLab(cmw) == NULL)
	ColWhtLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"White", I18NNOUN | I18NLABEL );
 
    if (ColGryLab(cmw) == NULL)
	ColGryLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Gray", I18NNOUN | I18NLABEL );
 
    if (ColFulLab(cmw) == NULL)
	ColFulLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Full", I18NADJECTIVE | I18NLABEL );
 
    if (ColPDLab(cmw) == NULL)
	ColPDLab(cmw) = (XmString)
			  DXmGetLocaleString((I18nContext)NULL,"Color Model ", I18NNOUN | I18NLABEL );
 
    if (ColPDHLSLab(cmw) == NULL)
	ColPDHLSLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"HLS", I18NNOUN | I18NLIST );
 
    if (ColPDRGBLab(cmw) == NULL)
	ColPDRGBLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"RGB", I18NNOUN | I18NLIST );
 
    if (ColPDPickerLab(cmw) == NULL)
	ColPDPickerLab(cmw) = (XmString)
			        DXmGetLocaleString((I18nContext)NULL,"Picker", I18NNOUN | I18NLIST );
 
    if (ColOkLab(cmw) == NULL)
	ColOkLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"OK", I18NVERB | I18NBUTTON );
 
    if (ColAppLab(cmw) == NULL)
	ColAppLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Apply", I18NVERB | I18NBUTTON );
 
    if (ColResLab(cmw) == NULL)
	ColResLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Reset", I18NVERB | I18NBUTTON );
 
    if (ColCanLab(cmw) == NULL)
	ColCanLab(cmw) = (XmString)
			   DXmGetLocaleString((I18nContext)NULL,"Cancel", I18NVERB | I18NBUTTON );
 
    if (ColUndoLab(cmw) == NULL)
	ColUndoLab(cmw) = (XmString)
			      DXmGetLocaleString((I18nContext)NULL,"Undo", I18NVERB | I18NBUTTON );
 
    if (ColSmearLab(cmw) == NULL)
	ColSmearLab(cmw) = (XmString)
			      DXmGetLocaleString((I18nContext)NULL,"Smear", I18NVERB | I18NBUTTON );
 
    if (ColHelpLab(cmw) == NULL)
	ColHelpLab(cmw) = (XmString)
			      DXmGetLocaleString((I18nContext)NULL,"Help", I18NVERB | I18NBUTTON );

    if (ColPTitleLab(cmw) == NULL)
	ColPTitleLab(cmw) = (XmString)
			      DXmGetLocaleString((I18nContext)NULL,"Spectrum", I18NNOUN | I18NLABEL );
 
    if (ColSpectrumLab(cmw) == NULL)
	ColSpectrumLab(cmw) = (XmString)
			     DXmGetLocaleString((I18nContext)NULL,"Spectrum", I18NNOUN | I18NLABEL );
 
    if (ColPastelLab(cmw) == NULL)
	ColPastelLab(cmw) = (XmString)
			     DXmGetLocaleString((I18nContext)NULL,"Pastels", I18NNOUN | I18NLABEL );
 
    if (ColMetallicLab(cmw) == NULL)
	ColMetallicLab(cmw) = (XmString)
				DXmGetLocaleString((I18nContext)NULL,"Vivids", I18NNOUN | I18NLABEL );
 
    if (ColEarthtoneLab(cmw) == NULL)
	ColEarthtoneLab(cmw) = (XmString)
			    DXmGetLocaleString((I18nContext)NULL,"Earthtones", I18NNOUN | I18NLABEL );
 
    if (ColUserPaletteLab(cmw) == NULL)
	ColUserPaletteLab(cmw) = (XmString)
			  DXmGetLocaleString((I18nContext)NULL,"User palette", I18NNOUN | I18NLABEL );
 
    if (ColClearLab(cmw) == NULL)
	ColClearLab(cmw) = (XmString)
			      DXmGetLocaleString((I18nContext)NULL,"Clear", I18NVERB | I18NLABEL );
 
    if (ColSPInfoLab(cmw) == NULL)
	ColSPInfoLab(cmw) = (XmString)
		DXmGetLocaleString((I18nContext)NULL,"Save colors here.", I18NVERB | I18NBUTTON );
 
    if (ColScratchPadLab(cmw) == NULL)
	ColScratchPadLab(cmw) = (XmString)
		DXmGetLocaleString((I18nContext)NULL,"Scratch Pad...", I18NNOUN | I18NLABEL );
 
    if (ColITitleLab(cmw) == NULL)
	ColITitleLab(cmw) = (XmString)
		DXmGetLocaleString((I18nContext)NULL,"Interpolator", I18NNOUN | I18NLABEL );
 
    if (ColWarmerLab(cmw) == NULL)
	ColWarmerLab(cmw) = (XmString)
				DXmGetLocaleString((I18nContext)NULL,"Warmer", I18NADJECTIVE | I18NLABEL );
 
    if (ColCoolerLab(cmw) == NULL)
	ColCoolerLab(cmw) = (XmString)
				DXmGetLocaleString((I18nContext)NULL,"Cooler", I18NADJECTIVE | I18NLABEL );
 
    if (ColLighterLab(cmw) == NULL)
	ColLighterLab(cmw) = (XmString)
				DXmGetLocaleString((I18nContext)NULL,"Lighter", I18NADJECTIVE | I18NLABEL );
 
    if (ColDarkerLab(cmw) == NULL)
	ColDarkerLab(cmw) = (XmString)
				DXmGetLocaleString((I18nContext)NULL,"Darker", I18NADJECTIVE | I18NLABEL );
 
    if (ColPDBrowserLab(cmw) == NULL)
	ColPDBrowserLab(cmw) = (XmString)
				DXmGetLocaleString((I18nContext)NULL,"Browser", I18NNOUN | I18NLIST );
 
    if (ColPDGreyscaleLab(cmw) == NULL)
	ColPDGreyscaleLab(cmw) = (XmString)
			     DXmGetLocaleString((I18nContext)NULL,"Grayscale", I18NNOUN | I18NLIST );


    if (ColDropHelpTileLab(cmw) == NULL)
    {
	XmString line1,line2,line3,line4,sep,temp1,temp2;

	line1 = DXmGetLocaleString((I18nContext)NULL,"This drop will attempt to change the color of the color tile", I18NNOUN | I18NLIST );
	line2 = DXmGetLocaleString((I18nContext)NULL,"dropped upon.  The item dropped must be either a color or a", I18NNOUN | I18NLIST );
	line3 = DXmGetLocaleString((I18nContext)NULL,"valid X11 color representation string.  Press OK to accept ", I18NNOUN | I18NLIST );
	line4 = DXmGetLocaleString((I18nContext)NULL,"this drop or Cancel to cancel it.", I18NNOUN | I18NLIST );
	sep = XmStringSeparatorCreate();

	temp1 = XmStringConcat (line1, sep);
	temp2 = XmStringConcat (temp1, line2);
	XmStringFree (temp1);
	temp1 = XmStringConcat (temp2,sep);
	XmStringFree (temp2);
	temp2 = XmStringConcat (temp1, line3);
	XmStringFree(temp1);
	temp1 = XmStringConcat (temp2, sep);
	XmStringFree (temp2);
	ColDropHelpTileLab(cmw) = XmStringConcat (temp1, line4);
	
	XmStringFree(temp1);
	XmStringFree(line1);
	XmStringFree(line2);
	XmStringFree(line3);
	XmStringFree(line4);
	XmStringFree(sep);
    }

    if (ColDropHelpInterpLab(cmw) == NULL)
    {
	XmString line1,line2,line3,line4,sep,temp1,temp2;

	line1 = DXmGetLocaleString((I18nContext)NULL,"This drop will attempt to change the color of the closest", I18NNOUN | I18NLIST );
	line2 = DXmGetLocaleString((I18nContext)NULL,"interpolator end tile.  The item dropped must be either a", I18NNOUN | I18NLIST );
	line3 = DXmGetLocaleString((I18nContext)NULL,"color or a valid X11 color representation string.  Press OK", I18NNOUN | I18NLIST );
	line4 = DXmGetLocaleString((I18nContext)NULL,"to accept this drop or Cancel to cancel it.", I18NNOUN | I18NLIST );
	sep = XmStringSeparatorCreate();

	temp1 = XmStringConcat (line1, sep);
	temp2 = XmStringConcat (temp1, line2);
	XmStringFree (temp1);
	temp1 = XmStringConcat (temp2,sep);
	XmStringFree (temp2);
	temp2 = XmStringConcat (temp1, line3);
	XmStringFree(temp1);
	temp1 = XmStringConcat (temp2, sep);
	XmStringFree (temp2);
	ColDropHelpInterpLab(cmw) = XmStringConcat (temp1, line4);
	
	XmStringFree(temp1);
	XmStringFree(line1);
	XmStringFree(line2);
	XmStringFree(line3);
	XmStringFree(line4);
	XmStringFree(sep);
    }
}


/*---------------------------------------------------*/
/* allocates an array of XColor structures for the   */
/* picker and initializes the RGB values for each    */
/* structure in the array.  The pixel field of each  */
/* structure is filled in at color mix map time,     */
/* when color cell allocation is attempted.	     */
/*---------------------------------------------------*/

static void InitPickerColorArray (cmw)
    DXmColorMixWidget cmw;
{
    XColor *xcolor;
    unsigned short *color;
    int i;

    /* Allocate the array of X color structures used at map time for */
    /* color cell allocation					     */

    ColPickerXColors(cmw) = (XColor *) XtMalloc (sizeof(XColor) * 
						 ColPickerColorCount(cmw));

    if (ColPickerColors(cmw) != NULL)
	color = ColPickerColors (cmw); 
    else
	color = (unsigned short *) default_spectrum_colors;

    for (i=0, xcolor=ColPickerXColors(cmw); 
	 i<ColPickerColorCount(cmw); 
	 i++, xcolor++)                                                                           
    {
	xcolor->red =   (unsigned short) (*color);  color++;
	xcolor->green = (unsigned short) (*color);  color++;
	xcolor->blue =  (unsigned short) (*color);  color++;
    }
}


/*---------------------------------------------------*/
/* creates an eyedropper cursor with a black border  */
/* and white center for use by the color picker      */
/*---------------------------------------------------*/

static void InitPickerCursor (cmw)
    DXmColorMixWidget cmw;
{
    Pixmap mask, source;
    XColor fg, bg, dummy;
    Display *dpy = (Display *) XtDisplay (cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);
    unsigned int best_width, best_height;
    unsigned int source_width, source_height, source_x_hot, source_y_hot;
    unsigned int mask_width, mask_height;	    
    unsigned char *source_bits, *mask_bits;
    Arg al[15];
    int ac;

    XLookupColor (dpy, cmw->core.colormap, "black",
	&dummy, &fg);

    XLookupColor (dpy, cmw->core.colormap, "white",
	&dummy, &bg);

    XQueryBestCursor (dpy, RootWindowOfScreen(XtScreen(cmw)),
		      32, 32, &best_width, &best_height);

    if ((best_width >= 32) && (best_height >= 32))
    {
	source_width = eyedropper_32_width;
	source_height = eyedropper_32_height;
	source_x_hot = eyedropper_32_x_hot;
	source_y_hot = eyedropper_32_y_hot;
	source_bits = eyedropper_32_bits;
	mask_width = eyedropper_32_mask_width;
	mask_height = eyedropper_32_mask_height;
	mask_bits = eyedropper_32_mask_bits;
    }
    else
    {
	source_width = eyedropper_width;
	source_height = eyedropper_height;
	source_x_hot = eyedropper_x_hot;
	source_y_hot = eyedropper_y_hot;
	source_bits = eyedropper_bits;
	mask_width = eyedropper_mask_width;
	mask_height = eyedropper_mask_height;
	mask_bits = eyedropper_mask_bits;
    }

    source = XCreatePixmapFromBitmapData (dpy, XDefaultRootWindow (dpy), 
	(char *)source_bits, source_width, source_height, 1, 0, 1);
    
    mask = XCreatePixmapFromBitmapData (dpy, XDefaultRootWindow (dpy), 
	(char *)mask_bits, mask_width, mask_height, 
	1, 0, 1);

    ColPickerCursor(cmw) = XCreatePixmapCursor (dpy, source, mask, 
	&fg, &bg, source_x_hot, source_y_hot);

    XFreePixmap (dpy, source);
    XFreePixmap (dpy, mask); 

    /* Create the eyedropper icon used during dragging */

    source = XCreatePixmapFromBitmapData (dpy, 
					  RootWindowOfScreen(XtScreen(cmw)), 
					  (char *)source_bits, source_width, 
					  source_height, 1, 0, 1);
    
    mask = XCreatePixmapFromBitmapData (dpy, 
					RootWindowOfScreen(XtScreen(cmw)), 
					(char *)mask_bits, mask_width, 
					mask_height, 1, 0, 1);

    ac = 0;
    XtSetArg(al[ac], XmNhotX, source_x_hot); ac++;
    XtSetArg(al[ac], XmNhotY, source_y_hot); ac++;
    XtSetArg(al[ac], XmNwidth, source_width); ac++;
    XtSetArg(al[ac], XmNheight, source_height); ac++;
    XtSetArg(al[ac], XmNpixmap, source); ac++;
    XtSetArg(al[ac], XmNmask, mask); ac++;
    ColDragIcon(cmw) = XmCreateDragIcon((Widget) cmw, "dragIcon", al, ac);
}


/*---------------------------------------------------*/
/* allocates an array of XColor structures for the   */
/* interpolator and initializes the RGB values for   */
/* structure in the array.  The pixel field of each  */
/* structure is filled in at color mix map time,     */
/* when color cell allocation is attempted.  Also    */
/* allocates and undo and hls arrays which are used  */
/* for restoration of previous interpolator states.  */
/*---------------------------------------------------*/

static void InitInterpColorArray (cmw)
    DXmColorMixWidget cmw;
{
    XColor *xcolor, *ucolor;
    HLSValues hls;
    int i;

    /* Allocate the arrays of X color structures used at map time for */
    /* color cell allocation	     				      */

    ColInterpXColors(cmw) = (XColor *) XtMalloc (sizeof(XColor) * 
						 ColInterpTileCount(cmw));

    ColUndoXColors(cmw) =   (XColor *) XtMalloc (sizeof(XColor) * 
						 ColInterpTileCount(cmw));


    ColInterpHLSValues(cmw) = (HLSValues) XtMalloc (sizeof(HLSValuesRec) * 
						 ColInterpTileCount(cmw));
    
    /* Initialize all tile colors to white */

    for (i=0, xcolor=ColInterpXColors(cmw), ucolor = ColUndoXColors(cmw),
	 hls = ColInterpHLSValues(cmw); 
	 i<ColInterpTileCount(cmw); 
	 i++, xcolor++, ucolor++, hls++)
    {
	xcolor->red   = ucolor->red   = (unsigned short) MAXCOLORVALUE; 
	xcolor->green = ucolor->green = (unsigned short) MAXCOLORVALUE; 
	xcolor->blue  = ucolor->blue  = (unsigned short) MAXCOLORVALUE; 	

	hls->hue = 0.0;
	hls->lightness = 100.0;
	hls->saturation = 0.0;
    }
}


/*---------------------------------------------------*/
/* allocates XColor structures for the scratch       */
/* pad current color and the first item of the       */
/* scratch pad list of colors and initializes the    */
/* RGB values of these structures to white.  The     */
/* pixel field for the current color structure is    */
/* filled in at color mix map time, when color cell  */
/* allocation is attempted.			     */
/*---------------------------------------------------*/

static void InitSPColorArray (cmw)
    DXmColorMixWidget cmw;
{
    XColor *xcolor;

    xcolor = (XColor *) &(ColCurrentSPColor(cmw));

    xcolor->red   = (unsigned short) MAXCOLORVALUE; 
    xcolor->green = (unsigned short) MAXCOLORVALUE; 
    xcolor->blue  = (unsigned short) MAXCOLORVALUE; 	

    ColScratchCount(cmw) = 1;

    xcolor = (XColor *) XtMalloc (sizeof(XColor) * ColScratchCount(cmw));

    xcolor->red   = (unsigned short) MAXCOLORVALUE; 
    xcolor->green = (unsigned short) MAXCOLORVALUE; 
    xcolor->blue  = (unsigned short) MAXCOLORVALUE; 	

    ColScratchColors(cmw) = xcolor;
}


/*-----------------------------------------------------*/
/* allocates an array of XColor structures for the     */
/* browser.  The actual color cells will be allocated  */
/* at color mix map time, and the proper RGB values    */
/* will be stored in them when the browser is selected */
/* for the first time.	                               */
/*-----------------------------------------------------*/

static void InitBrowserColorArray (cmw)
    DXmColorMixWidget cmw;
{
    int i;
    BrowserColor color;
    XColor exact, screen;

    ColBrowserXColors(cmw) = 
	(XColor *) XtMalloc (sizeof(XColor) * ColBrowserItemCount(cmw));

    CheckBrowserInitialization(cmw);
}


/*---------------------------------------------------*/
/* Creates compound strings for the browser buttons  */
/* from the standard X11 color names.		     */
/*---------------------------------------------------*/

static void InitDefaultBrowserStrings (cmw)
    DXmColorMixWidget cmw;
{
    int i;
    BrowserColor color;

    for (i=0, color = ColBrowserColors(cmw);
	 i < ColBrowserColorCount(cmw);
	 i++, color++)
    {
	color->string = (XmString)
			    DXmGetLocaleString((I18nContext)NULL,color->name, I18NNOUN | I18NLIST );
    }
}



/*---------------------------------------------------*/
/* allocates a copy of the picker color array that   */
/* the user passed to the widget.		     */
/*---------------------------------------------------*/

static void CopyPickerColorList (cmw)
    DXmColorMixWidget cmw;
{
    unsigned short *temp, *users, *widgets;
    int i;

    temp = (unsigned short *) XtMalloc (sizeof(unsigned short) 
		     * (ColPickerColorCount(cmw) * 3));

    for (i=0, users=ColPickerColors(cmw), widgets=temp;
	 i < (ColPickerColorCount(cmw) * 3);
	 i++, users++, widgets++)
    {
	*widgets = *users;
    }	

    ColPickerColors(cmw) = temp;
}


                           
/*---------------------------------------------------*/
/* clears label fields since actual value kept with  */
/* subwidgets displaying the strings.  Necessary for */
/* setvalues...					     */
/*---------------------------------------------------*/

static void ResetLabelFields(cmw)
    DXmColorMixWidget cmw;
{
    ColMainLab(cmw) 	= NULL;
    ColDispLab(cmw) 	= NULL;
    ColMixLab(cmw) 	= NULL;
    ColRedLab(cmw) 	= NULL;
    ColGrnLab(cmw) 	= NULL;
    ColBluLab(cmw) 	= NULL;
    ColHueLab(cmw) 	= NULL;
    ColLightLab(cmw) 	= NULL;
    ColSatLab(cmw) 	= NULL;
    ColSldLab(cmw) 	= NULL;
    ColValLab(cmw) 	= NULL;
    ColHueLab(cmw) 	= NULL;
    ColLightLab(cmw) 	= NULL;
    ColSatLab(cmw) 	= NULL;
    ColBlkLab(cmw) 	= NULL;
    ColWhtLab(cmw) 	= NULL;
    ColGryLab(cmw) 	= NULL;
    ColFulLab(cmw) 	= NULL;
    ColPDLab(cmw) 	= NULL;
    ColPDHLSLab(cmw) 	= NULL;
    ColPDRGBLab(cmw) 	= NULL;
    ColPDPickerLab(cmw)	= NULL;
    ColOkLab(cmw) 	= NULL;
    ColAppLab(cmw) 	= NULL;
    ColResLab(cmw) 	= NULL;
    ColCanLab(cmw) 	= NULL;
    ColUndoLab(cmw) 	= NULL;
    ColSmearLab(cmw) 	= NULL;
    ColHelpLab(cmw) 	= NULL;
    ColITitleLab(cmw) 	= NULL;
    ColSpectrumLab(cmw)	    = NULL;
    ColPastelLab(cmw)	    = NULL;
    ColMetallicLab(cmw)	    = NULL;
    ColEarthtoneLab(cmw)    = NULL;
    ColUserPaletteLab(cmw)  = NULL;
    ColWarmerLab(cmw)	    = NULL;
    ColCoolerLab(cmw)	    = NULL;
    ColLighterLab(cmw)	    = NULL;
    ColDarkerLab(cmw)	    = NULL;
    ColClearLab(cmw)	    = NULL;
    ColSPInfoLab(cmw)	    = NULL;
    ColScratchPadLab(cmw)   = NULL;
    ColPDBrowserLab(cmw)    = NULL;
    ColPDGreyscaleLab(cmw)  = NULL;
}


/*-----------------------------------------------------*/
/* checks whether or not the RGB values for each name  */
/* in the browser color array have been fetched yet,   */
/* and fetches them if they have not.  The calling of  */
/* this procedure is deferred until the browser is     */
/* actually selected for the first time to avoid the   */
/* possibly unnecessary overhead of the 150+ server    */
/* calls that take place here when the default colors  */
/* are used.					       */
/*-----------------------------------------------------*/

static void CheckBrowserInitialization (cmw)
    DXmColorMixWidget cmw;
{
    int i;
    int total = 0;
    BrowserColor color, new, temp;
    XColor exact, screen;
    int intensity, luminosity, brightness;
    if (ColInitBrowserColors(cmw))
	return;

    new = temp = (BrowserColor) XtMalloc (sizeof(BrowserColorRec) * ColBrowserColorCount(cmw));

    for (i=0, color = ColBrowserColors(cmw); 
	 i < ColBrowserColorCount(cmw); 
	 i++, color++)
    {
	if (XLookupColor (XtDisplay(cmw),
			  cmw->core.colormap,
		          color->name,
		          &exact, &screen))
	{
	    *temp = *color;
	    temp->red   = exact.red;
	    temp->green = exact.green;
	    temp->blue  = exact.blue;

	    /* Determine whether black or white text will show up better */
	    /* on this background.  This algorithm comes from visual.c   */

	    intensity = (int) (temp->red + temp->green + temp->blue) / 3;

	    luminosity = (int) ((.30 * (float) temp->red) 
			      + (.59 * (float) temp->green)
			      + (.11 * (float) temp->blue));

	    brightness = (int) ((intensity * .25) + (luminosity * .75));

	    if (brightness > HALFCOLORVALUE)
		temp->dark_fg = TRUE;
	    else
		temp->dark_fg = FALSE;

	    temp++;
	    total++;
	}
    }

    ColBrowserColorCount(cmw) = total;
    ColBrowserColors(cmw) = (BrowserColor) new;
    ColInitBrowserColors(cmw) = TRUE;
}


/*---------------------------------------------------*/
/* Determines type of display (ie color, grayscale,  */
/* etc)						     */
/*---------------------------------------------------*/

static void DetermineDisplayType(cmw)
    DXmColorMixWidget cmw;
{
    Visual  *type;
    int      depth;
    Screen  *screen;

    screen = (Screen *) XtScreen(cmw);
    type   = XDefaultVisualOfScreen(screen);

    switch (type->class)
    {
    	case StaticGray:
	{
            if (XDefaultDepthOfScreen(screen) == 1)
		ColDispType(cmw) = BlackAndWhite;
	    else
		ColDispType(cmw) = StatGray;
	    break;
	}

    	case GrayScale:
	{
   	    ColDispType(cmw) = GryScale;
   	    break;
	}

	case    StaticColor:
	case    TrueColor:
	{
	    ColDispType(cmw) = StatColor;
	    break;
	}

    	case    PseudoColor:
    	case    DirectColor:
	{
	    ColDispType(cmw) = DynColor;
	    break;
	}

	default:
	{
	    ColDispType(cmw) = BlackAndWhite;
	}

    }  /* switch */
}


/*---------------------------------------------------*/
/* create routines				     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/* creates a label gadget         		     */
/*---------------------------------------------------*/

static void CreateColLabelGadget(cmw, parent, label, w)
    DXmColorMixWidget cmw;
    Widget parent;
    XmString  label;
    Widget *w;
{
    Arg al[5];
    int ac = 0;
    XmFontList fonts; 

    if (ColLabelFontList(cmw))
	fonts = ColLabelFontList(cmw);
    else
	fonts = ColTextFontList(cmw);

    if (fonts)
    {
    	XtSetArg(al[ac], XmNfontList, fonts);  ac++;
    }

    XtSetArg(al[ac], XmNlabelString, label);  ac++;
    XtSetArg(al[ac], XmNstringDirection, ColDirection(cmw)); ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); ac++;

    *w = (Widget) XmCreateLabelGadget( parent, "colormixlabelgadget", al, ac);
}


/*---------------------------------------------------*/
/* creates a text entry field for the RGB mixer      */
/*---------------------------------------------------*/

static void CreateColTextWidget(cmw, w, color_value)
    XmBulletinBoardWidget   cmw;
    Widget  *w;
    int  color_value;
{
    Arg  	al[7];
    int  	ac = 0;
    XmString 	temp_cs;
    long	size,status;

    temp_cs = (XmString) _DXmCvtItoCS(color_value, &size, &status);

    if (ColTextFontList(cmw))
    {
    	XtSetArg(al[ac], XmNfontList, ColTextFontList(cmw));  ac++;
    }

    XtSetArg(al[ac], XmNrows, 1);  		    ac++;
    XtSetArg(al[ac], XmNcolumns, 5); 		    ac++;
    XtSetArg(al[ac], XmNmaxLength, 5); 		    ac++;
    XtSetArg(al[ac], XmNresizeWidth, FALSE); 	    ac++;
    XtSetArg(al[ac], XmNvalue, temp_cs);  	    ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); ac++;

    *w = (Widget) DXmCreateCSText( cmw, "colormixTextWidget", al, ac);

    XtOverrideTranslations (*w, cmwtext_translations_parsed);

    XmStringFree(temp_cs);
}


/*---------------------------------------------------*/
/* creates a pushbutton gadget		     	     */
/*---------------------------------------------------*/

static void CreateColPBGadget(cmw, parent, label, w)
    DXmColorMixWidget cmw;
    Widget parent;
    XmString  label;
    Widget *w;
{
    Arg al[6];
    int ac = 0;
    XmFontList fonts; 

    if (ColLabelFontList(cmw))
	fonts = ColButtonFontList(cmw);
    else
	fonts = ColTextFontList(cmw);

    if (fonts)
    {
    	XtSetArg(al[ac], XmNfontList, fonts);  ac++;
    }

    XtSetArg(al[ac], XmNlabelString, label);  			ac++;
    XtSetArg(al[ac], XmNactivateCallback, cmw_pbact_cb);  	ac++;
    XtSetArg(al[ac], XmNstringDirection, ColDirection(cmw));  	ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);  		ac++;

    *w = (Widget) XmCreatePushButtonGadget( parent, "colormix PBGadget", al, ac);
}


/*---------------------------------------------------*/
/* creates a pushbutton widget		     	     */
/*---------------------------------------------------*/

static void CreateColPB (cmw, parent, label, w)
    DXmColorMixWidget cmw;
    Widget parent;
    XmString  label;
    Widget *w;
{
    Arg al[6];
    int ac = 0;
    XmFontList fonts; 

    if (ColLabelFontList(cmw))
	fonts = ColButtonFontList(cmw);
    else
	fonts = ColTextFontList(cmw);

    if (fonts)
    {
    	XtSetArg(al[ac], XmNfontList, fonts);  ac++;
    }

    XtSetArg(al[ac], XmNlabelString, label);  			ac++;
    XtSetArg(al[ac], XmNactivateCallback, cmw_pbact_cb);  	ac++;
    XtSetArg(al[ac], XmNstringDirection, ColDirection(cmw));  	ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); 		ac++;

    *w = (Widget) XmCreatePushButton ( parent, "colormix PBWidget", al, ac);
}


/*---------------------------------------------------*/
/* creates an arrowbutton gadget	     	     */
/*---------------------------------------------------*/

static void CreateColABGadget(cmw, parent, direction, w, cb)
    DXmColorMixWidget cmw;
    Widget parent;
    int direction;
    Widget *w;
    XtCallbackList cb;
{
    Arg al[10];
    int ac = 0;

    XtSetArg(al[ac], XmNarrowDirection, direction);	ac++;
    XtSetArg(al[ac], XmNactivateCallback, cb);          ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);     ac++;

    *w = (Widget) XmCreateArrowButtonGadget( parent, "colormix ABWidget", al, ac);
}


/*---------------------------------------------------*/
/* creates a pulldown menu - used to present multi-  */
/* color models available			     */
/*---------------------------------------------------*/

static void CreateColPDMenu(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[6];
    int ac = 0;
    XmFontList fonts; 

    if (ColLabelFontList(cmw))
	fonts = ColLabelFontList(cmw);
    else
	fonts = ColTextFontList(cmw);

    if (fonts)
    {
    	XtSetArg(al[ac], XmNfontList, fonts); ac++;
    }

    XtSetArg(al[ac], XmNstringDirection, ColDirection(cmw));  		ac++;
    XtSetArg(al[ac], XmNentryCallback, cmw_menu_cb);          		ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);           		ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw));  	ac++;

    ColPDMenWid(cmw) = (Widget) XmCreatePulldownMenu( (Widget) cmw,
				       	     	      "colormix pulldown menu",
				             	      al, 
				             	      ac);
}


/*---------------------------------------------------*/
/* creates an option menu - draws color model pull-  */
/* down						     */
/*---------------------------------------------------*/

static void CreateColOptMenu(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[20];
    int ac = 0;

    XmFontList fonts; 
    XmString blank_string;

    if (ColLabelFontList(cmw))
	fonts = ColLabelFontList(cmw);
    else
	fonts = ColTextFontList(cmw);

    if (fonts)
    {
    	XtSetArg(al[ac], XmNfontList, fonts);  ac++;
    }

    XtSetArg(al[ac], XmNmarginWidth, 0);			ac++;
    XtSetArg(al[ac], XmNsubMenuId, ColPDMenWid(cmw));           ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);             ac++;
    XtSetArg(al[ac], XmNstringDirection, ColDirection(cmw));    ac++;

    blank_string = XmStringCreateSimple ("");
    XtSetArg(al[ac], XmNlabelString, blank_string);		ac++;

    switch (ColModel(cmw))
    {
	case DXmColorModelHLS:
	    XtSetArg(al[ac], XmNmenuHistory, ColPDHLSWid(cmw));
	    break;		

	case DXmColorModelRGB:
	    XtSetArg(al[ac], XmNmenuHistory, ColPDRGBWid(cmw));
	    break;		

	case DXmColorModelBrowser:
	    XtSetArg(al[ac], XmNmenuHistory, ColPDBrowserWid(cmw));
	    break;		

	case DXmColorModelGreyscale:
	    XtSetArg(al[ac], XmNmenuHistory, ColPDGreyscaleWid(cmw));
	    break;		

	default:
	    XtSetArg(al[ac], XmNmenuHistory, ColPDPickerWid(cmw));
	    break;		
    }
	    
    ac++;

    ColOptMenWid(cmw) = (Widget) XmCreateOptionMenu( (Widget) cmw,
				       		     "colormix option ",
				       		     al, 
				       		     ac);


    XmStringFree (blank_string);

    /* This is the option menu title.  We can't use the labelString */
    /* resource because it doesn't support right-to-left layouts    */

    CreateColLabelGadget(cmw, cmw, ColPDLab(cmw), &ColOptLabWid(cmw));
    ac = 0;
    XtSetArg(al[ac], XmNmarginWidth, 0); ac++;
    XtSetValues(ColOptLabWid(cmw), al, ac);
    XtManageChild (ColOptLabWid(cmw)); 
}

                           
/*---------------------------------------------------*/
/* creates colormix label and pushbutton widgets     */
/*---------------------------------------------------*/

static void CreateColShell(cmw)
    DXmColorMixWidget cmw;
{

    if (IsColMainLab(cmw))
    {
	if (!XmStringEmpty(ColMainLab(cmw)))
	    CreateColLabelGadget(cmw, cmw, ColMainLab(cmw), &ColMainLabWid(cmw)); 
    }

    if (IsColDispLab(cmw))
    {
	if (!XmStringEmpty(ColDispLab(cmw)))
	    CreateColLabelGadget(cmw, cmw, ColDispLab(cmw), &ColDispLabWid(cmw)); 
    }

    if (IsColMixLab(cmw))
    {
	if (!XmStringEmpty(ColMixLab(cmw)))
	    CreateColLabelGadget(cmw, cmw, ColMixLab(cmw), &ColMixLabWid(cmw)); 
    }

    if (IsColScratchPadLab(cmw))
    {
	if (!XmStringEmpty(ColScratchPadLab(cmw)))
	{    
	    CreateColPB(cmw, cmw, ColScratchPadLab(cmw), &ColScratchPadPbWid(cmw)); 
	}
    }

    if (IsColOkLab(cmw))
    {
	if (!XmStringEmpty(ColOkLab(cmw)))
	{    
	    CreateColPB(cmw, cmw, ColOkLab(cmw), &ColOkPbWid(cmw)); 
	}
    }

    if (IsColAppLab(cmw))
    {
	if (!XmStringEmpty(ColAppLab(cmw)))
	    CreateColPB(cmw, cmw, ColAppLab(cmw), &ColAppPbWid(cmw)); 
    }

    if (IsColResLab(cmw))
    {
	if (!XmStringEmpty(ColResLab(cmw)))
	    CreateColPB(cmw, cmw, ColResLab(cmw), &ColResPbWid(cmw)); 
    }

    if (IsColCanLab(cmw))
    {
	if (!XmStringEmpty(ColCanLab(cmw)))
        {
	    CreateColPB(cmw, cmw, ColCanLab(cmw), &ColCanPbWid(cmw)); 
            if (IsColCanPbWid(cmw))
            {   /* Add CancelButton to Billboard for osfCancel Translation */
                cmw->bulletin_board.cancel_button = ColCanPbWid(cmw);
            }
        }
    }

    if (IsColHelpLab(cmw))
    {
	if (!XmStringEmpty(ColHelpLab(cmw)))
	    CreateColPB(cmw, cmw, ColHelpLab(cmw), &ColHelpPbWid(cmw)); 
    }

    ColorMixSetPBWidth(cmw);

    XtManageChildren (XtChildren(cmw), XtNumChildren(cmw));	
}

                           
/*---------------------------------------------------*/
/* creates the colormix default color display widget */
/*---------------------------------------------------*/

static void CreateColDisplay(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[20];
    int ac = 0;
    Display *dpy = XtDisplay(cmw);
    Atom import_list[2];

    XtSetArg(al[ac], XmNmarginWidth, ColDispViewM(cmw));    ac++;
    XtSetArg(al[ac], XmNmarginHeight, ColDispViewM(cmw));   ac++;
    XtSetArg(al[ac], XmNresize, XmRESIZE_ANY);              ac++;
    XtSetArg(al[ac], XmNmapCallback, cmw_map_cb);           ac++;
    XtSetArg(al[ac], XmNunmapCallback, cmw_unmap_cb);       ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);         ac++;
    XtSetArg(al[ac], XmNnavigationType, XmSTICKY_TAB_GROUP); ac++;
    XtSetArg(al[ac], XmNtraversalOn, TRUE);                  ac++;
 
    ColDispWid(cmw) = (Widget) XmCreateBulletinBoard((Widget) cmw, 
					  "color mixing widget display",
					  al, ac);
					
    /* 
     * create 'original' and 'new' color display windows
     */

    ac = 0;
    XtSetArg(al[ac], XmNwidth, ColDispViewW(cmw));        	ac++;
    XtSetArg(al[ac], XmNheight, ColDispViewH(cmw));       	ac++;
    XtSetArg(al[ac], XmNshadowThickness, 0);                    ac++;
    XtSetArg(al[ac], XmNhighlightThickness, 0);                 ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);       	ac++;
    XtSetArg(al[ac], XmNarmCallback, cmw_display_input_cb);	ac++;
    XtSetArg(al[ac], XmNactivateCallback, cmw_display_input_cb); ac++;
    XtSetArg(al[ac], XmNdisarmCallback, cmw_display_input_cb);	ac++;

    ColOrigWid(cmw) = (Widget) XmCreateDrawnButton(ColDispWid(cmw), 
				      	"color mixing widget orig window",
				      	al, ac);

    ColNewWid(cmw)  = (Widget) XmCreateDrawnButton(ColDispWid(cmw), 
				      	"color mixing widget new window",
				      	al, ac);

    XtOverrideTranslations (ColOrigWid(cmw), cmwdisp_translations_parsed);
    XtOverrideTranslations (ColNewWid(cmw), cmwdisp_translations_parsed);

    XtManageChildren (XtChildren(ColDispWid(cmw)), XtNumChildren(ColDispWid(cmw)));	

    /* Register new color tile as a drop site */

    COMPOUND_TEXT = XmInternAtom (dpy, "COMPOUND_TEXT", False);
    
    import_list[0] = COMPOUND_TEXT;

    ac = 0;
    XtSetArg (al[ac], XmNimportTargets, import_list); ac++;
    XtSetArg (al[ac], XmNnumImportTargets, 1); ac++;
    XtSetArg (al[ac], XmNdropSiteOperations, XmDROP_COPY); ac++;
    XtSetArg (al[ac], XmNdropProc, HandleColorDrop); ac++;
    XtSetArg (al[ac], XmNdragProc, GenericColorDragProc); ac++;
    XtSetArg (al[ac], XmNanimationStyle, XmDRAG_UNDER_NONE); ac++;
    XmDropSiteRegister (ColNewWid(cmw), al, ac);
}

                           
/*---------------------------------------------------*/
/* creates colormix default color HLS mixing widget  */
/*---------------------------------------------------*/

static void CreateColHLSMixer(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[20];
    int ac = 0;
    char c[6];
    Widget lb1, lb2;

    ColorMixRGBToHLS((double)ColNewColorRed(cmw), 
		     (double)ColNewColorGrn(cmw), 
		     (double)ColNewColorBlu(cmw),
	     	     &ColNewColorHue(cmw), 
		     &ColNewColorLight(cmw), 
		     &ColNewColorSat(cmw));

    XtSetArg(al[ac], XmNmarginWidth, 5);  		ac++;
    XtSetArg(al[ac], XmNmarginHeight, 5);               ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);     ac++;
    XtSetArg(al[ac], XmNborderWidth, 0);                ac++;

    ColHLSMixerWid(cmw) = (Widget) XmCreateBulletinBoard((Widget) cmw, 
					  "color mixing widget mixer",
					  al, ac);

    CreateColLabelGadget(cmw, 
		   ColHLSMixerWid(cmw), 
		   ColHueLab(cmw), 
		   &ColHueLabWid(cmw)); 

    CreateColLabelGadget(cmw, 
		   ColHLSMixerWid(cmw), 
		   ColLightLab(cmw), 
		   &ColLightLabWid(cmw)); 

    CreateColLabelGadget(cmw, 
		   ColHLSMixerWid(cmw), 
		   ColSatLab(cmw), 
		   &ColSatLabWid(cmw)); 

    ac=0;
    XtSetArg(al[ac], XmNwidth, 180);                                    ac++;
    XtSetArg(al[ac], XmNscaleWidth, 180);                        	ac++;
    XtSetArg(al[ac], XmNminimum, 0);					ac++;
    XtSetArg(al[ac], XmNmaximum, 359);                                  ac++;
    XtSetArg(al[ac], XmNvalue, (int)ColNewColorHue(cmw));               ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL);                     ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw));     ac++;
    XtSetArg(al[ac], XmNshowValue, TRUE);                               ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_hlsscact_cb);                 ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_hlsscact_cb);         ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);                     ac++;

    ColHueSclWid(cmw) = (Widget) XmCreateScale(ColHLSMixerWid(cmw), 
					    "color mixing widget mixer hue scale",
					    al, ac);
					
    CreateColLabelGadget(cmw, 
		    ColHLSMixerWid(cmw), 
		    ColBlkLab(cmw), 
		    &ColBlkLabWid(cmw)); 
					
    CreateColLabelGadget(cmw, 
		    ColHLSMixerWid(cmw), 
		    ColWhtLab(cmw), 
		    &ColWhtLabWid(cmw)); 

    ac=0;
    XtSetArg(al[ac], XmNwidth, 180);                                    ac++;
    XtSetArg(al[ac], XmNscaleWidth, 180);                        	ac++;
    XtSetArg(al[ac], XmNminimum, 0);					ac++;
    XtSetArg(al[ac], XmNmaximum, 100);                                  ac++;
    XtSetArg(al[ac], XmNvalue, (int)(ColNewColorLight(cmw)*100.0));     ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL);                     ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw));     ac++;
    XtSetArg(al[ac], XmNshowValue, TRUE);                               ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_hlsscact_cb);                 ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_hlsscact_cb);         ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);                     ac++;

    ColLightSclWid(cmw) = (Widget) XmCreateScale(ColHLSMixerWid(cmw), 
					    "color mixing widget mixer light scale",
					    al, ac);
					
					
    CreateColLabelGadget(cmw, 
		    ColHLSMixerWid(cmw), 
		    ColGryLab(cmw), 
		    &ColGryLabWid(cmw)); 
					
    CreateColLabelGadget(cmw, 
		    ColHLSMixerWid(cmw), 
		    ColFulLab(cmw), 
		    &ColFulLabWid(cmw)); 


    ac=0;
    XtSetArg(al[ac], XmNwidth, 180);                                    ac++;
    XtSetArg(al[ac], XmNscaleWidth, 180);                        	ac++;
    XtSetArg(al[ac], XmNminimum, 0);					ac++;
    XtSetArg(al[ac], XmNmaximum, 100);                                  ac++;
    XtSetArg(al[ac], XmNvalue, (int)(ColNewColorSat(cmw)*100.0));       ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL);                     ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw));     ac++;
    XtSetArg(al[ac], XmNshowValue, TRUE);                               ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_hlsscact_cb);                 ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_hlsscact_cb);         ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);                     ac++;

    ColSatSclWid(cmw) = (Widget) XmCreateScale(ColHLSMixerWid(cmw), 
					    "color mixing widget mixer sat scale",
					    al, ac);

    XtManageChildren (XtChildren(ColHLSMixerWid(cmw)), 
		      XtNumChildren(ColHLSMixerWid(cmw)));	
}

                           
/*---------------------------------------------------*/
/* creates colormix default color RGB mixing widget  */
/*---------------------------------------------------*/

static void CreateColRGBMixer(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[10];
    int ac = 0;
    Widget dummy;

    XtSetArg(al[ac], XmNmarginWidth, 5);  		ac++;
    XtSetArg(al[ac], XmNmarginHeight, 5); 		ac++;
    XtSetArg(al[ac], XmNborderWidth, 0);  		ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); 	ac++;

    ColRGBMixerWid(cmw) = (Widget) XmCreateBulletinBoard((Widget) cmw, 
					  "color mixing widget mixer",
					  al, ac);

    /* 
     * create labels, RGB mixing scales and value text entry fields.
     */

    if (IsColSldLab(cmw))
	if (!XmStringEmpty(ColSldLab(cmw)))
	    CreateColLabelGadget(cmw, ColRGBMixerWid(cmw), ColSldLab(cmw), &ColSldLabWid(cmw)); 

    if (IsColValLab(cmw))
	if (!XmStringEmpty(ColValLab(cmw)))
	    CreateColLabelGadget(cmw, ColRGBMixerWid(cmw), ColValLab(cmw), &ColValLabWid(cmw)); 

    ac = 0;
    XtSetArg(al[ac], XmNminimum, 0);					ac++;
    XtSetArg(al[ac], XmNmaximum, 100);                                  ac++;
    XtSetArg(al[ac], XmNwidth, 175);                                    ac++;
    XtSetArg(al[ac], XmNscaleWidth, 175);                        	ac++;
    XtSetArg(al[ac], XmNvalue, ColCvtColorToScaleValue(ColNewColorRed(cmw)));
    ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL);                     ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw));     ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_scact_cb);                    ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_scact_cb);            ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);                     ac++;

    ColRedSclWid(cmw) = (Widget) XmCreateScale(ColRGBMixerWid(cmw), 
					    "color mixing widget mixer red scale",
					    al, ac);

    CreateColLabelGadget(cmw, ColRedSclWid(cmw), ColRedLab(cmw), &ColRedLabWid(cmw)); 
    XtManageChild(ColRedLabWid(cmw));

    ac = 0;
    XtSetArg(al[ac], XmNminimum, 0);					ac++;
    XtSetArg(al[ac], XmNmaximum, 100);                                  ac++;
    XtSetArg(al[ac], XmNwidth, 175);                                    ac++;
    XtSetArg(al[ac], XmNscaleWidth, 175);                               ac++;
    XtSetArg(al[ac], XmNvalue, ColCvtColorToScaleValue(ColNewColorGrn(cmw)));
    ac++;                                                               
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL);			ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw));     ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_scact_cb);                    ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_scact_cb);            ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);                     ac++;

    ColGrnSclWid(cmw) = (Widget) XmCreateScale(ColRGBMixerWid(cmw), 
					    "color mixing widget mixer green scale",
					    al, ac);

    CreateColLabelGadget(cmw, ColGrnSclWid(cmw), ColGrnLab(cmw), &ColGrnLabWid(cmw)); 
    XtManageChild(ColGrnLabWid(cmw));

					
    ac = 0;
    XtSetArg(al[ac], XmNwidth, 175);					ac++;
    XtSetArg(al[ac], XmNscaleWidth, 175);                               ac++;
    XtSetArg(al[ac], XmNminimum, 0);                                    ac++;
    XtSetArg(al[ac], XmNmaximum, 100);                                  ac++;
    XtSetArg(al[ac], XmNvalue, ColCvtColorToScaleValue(ColNewColorBlu(cmw)));
    ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL);                     ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw));     ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_scact_cb);                    ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_scact_cb);            ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);                     ac++;

    ColBluSclWid(cmw) = (Widget) XmCreateScale(ColRGBMixerWid(cmw), 
					    "color mixing widget mixer blue scale",
					    al, ac);

    CreateColLabelGadget(cmw, ColBluSclWid(cmw), ColBluLab(cmw), &ColBluLabWid(cmw)); 
    XtManageChild(ColBluLabWid(cmw));


    CreateColTextWidget(ColRGBMixerWid(cmw), &ColRedTextWid(cmw), ColNewColorRed(cmw));
    CreateColTextWidget(ColRGBMixerWid(cmw), &ColGrnTextWid(cmw), ColNewColorGrn(cmw));
    CreateColTextWidget(ColRGBMixerWid(cmw), &ColBluTextWid(cmw), ColNewColorBlu(cmw));

    XtManageChildren (XtChildren(ColRGBMixerWid(cmw)), 
		      XtNumChildren(ColRGBMixerWid(cmw)));	
}

                           
/*---------------------------------------------------*/
/* creates colormix default color picking widget     */
/*---------------------------------------------------*/

static void CreateColPickerMixer(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[20];
    Widget dummy;
    Pixel armcolor;
    int ac = 0, picker_width, interp_width;
    Pixmap lbucketpix, rbucketpix, lbucketarmpix, rbucketarmpix;
    Display *dpy = (Display *) XtDisplay (cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);
    XmFontList fonts; 
    XmString blank_string;
    Atom import_list[2];

    if (ColLabelFontList(cmw))
	fonts = ColLabelFontList(cmw);
    else
	fonts = ColTextFontList(cmw);

    XtSetArg(al[ac], XmNmarginWidth, 5);		ac++;
    XtSetArg(al[ac], XmNmarginHeight, 5);               ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);     ac++;

    ColPickerMixerWid(cmw) = (Widget) XmCreateBulletinBoard((Widget) cmw, 
					  "color mixing widget mixer",
					  al, ac);
    ac = 0;

    if (fonts)
    {
    	XtSetArg(al[ac], XmNfontList, fonts);  ac++;
    }

    XtSetArg(al[ac], XmNstringDirection, ColDirection(cmw));  		ac++;
    XtSetArg(al[ac], XmNentryCallback, cmw_pmenu_cb);         		ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);           		ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw)); 	ac++;

    ColPickerPDWid(cmw) = (Widget) XmCreatePulldownMenu(ColPickerMixerWid(cmw),
				       	     	      "picker pulldown menu",
				             	      al, 
				             	      ac);

    CreateColPBGadget(cmw, ColPickerPDWid(cmw), ColUserPaletteLab(cmw), &ColUserPalettePbWid(cmw));
    CreateColPBGadget(cmw, ColPickerPDWid(cmw), ColSpectrumLab(cmw), &ColSpectrumPbWid(cmw));
    CreateColPBGadget(cmw, ColPickerPDWid(cmw), ColPastelLab(cmw), &ColPastelPbWid(cmw));
    CreateColPBGadget(cmw, ColPickerPDWid(cmw), ColMetallicLab(cmw), &ColMetallicPbWid(cmw));
    CreateColPBGadget(cmw, ColPickerPDWid(cmw), ColEarthtoneLab(cmw), &ColEarthtonePbWid(cmw));

    XtManageChildren(XtChildren(ColPickerPDWid(cmw)), 
		     XtNumChildren(ColPickerPDWid(cmw)));	

    if (ColPickerColors(cmw) == NULL)
	XtUnmanageChild(ColUserPalettePbWid(cmw));

    ac = 0;

    if (fonts)
    {
    	XtSetArg(al[ac], XmNfontList, fonts);  ac++;
    }

    XtSetArg(al[ac], XmNmarginWidth, 0);                      ac++;
    XtSetArg(al[ac], XmNsubMenuId, ColPickerPDWid(cmw));      ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb);           ac++;
    XtSetArg(al[ac], XmNstringDirection, ColDirection(cmw));  ac++;

    blank_string = XmStringCreate ("","");
    XtSetArg(al[ac], XmNlabelString, blank_string);	      ac++;

    ColPickerOptMenWid(cmw) = (Widget) XmCreateOptionMenu(ColPickerMixerWid(cmw),
				       		     "picker option menu",
				       		     al, 
				       		     ac);

    XmStringFree (blank_string);

    ac = 0;

    XtSetArg(al[ac], XmNshadowThickness, 2);       		ac++;
    XtSetArg(al[ac], XmNshadowType, XmSHADOW_IN);  		ac++;
    XtSetArg(al[ac], XmNnavigationType, XmSTICKY_TAB_GROUP);  	ac++;

    ColPickerFrameWid(cmw) = (Widget) XmCreateFrame(ColPickerMixerWid(cmw), 
					  "picker frame",
					  al, ac);

    picker_width = ColPickerTileWidth(cmw) * ColPickerColorCount(cmw);

    ac = 0;
    XtSetArg(al[ac], XmNshadowThickness, 0);       		ac++;
    XtSetArg(al[ac], XmNhighlightThickness, 0);                 ac++;
    XtSetArg(al[ac], XmNwidth, picker_width);  			ac++;
    XtSetArg(al[ac], XmNheight, ColPickerTileHeight(cmw));      ac++;
    XtSetArg(al[ac], XmNarmCallback, cmw_picker_input_cb);    	ac++;
    XtSetArg(al[ac], XmNactivateCallback, cmw_picker_input_cb); ac++;
    XtSetArg(al[ac], XmNdisarmCallback, cmw_picker_input_cb);   ac++;
    XtSetArg(al[ac], XmNexposeCallback, cmw_picker_expose_cb);  ac++;

    ColPickerDAWid(cmw) = (Widget) XmCreateDrawnButton(ColPickerFrameWid(cmw), 
					  "picker drawing area",
					  al, ac);


    ac = 0;
    XtSetArg(al[ac], XmNshadowThickness, 2);       		ac++;
    XtSetArg(al[ac], XmNshadowType, XmSHADOW_IN);  		ac++;
    XtSetArg(al[ac], XmNnavigationType, XmSTICKY_TAB_GROUP);  	ac++;

    ColInterpFrameWid(cmw) = (Widget) XmCreateFrame(ColPickerMixerWid(cmw), 
					  "interp frame",
					  al, ac);


    interp_width = ColInterpTileWidth(cmw) * ColInterpTileCount(cmw);

    ac = 0;
    XtSetArg(al[ac], XmNshadowThickness, 0);       		ac++;
    XtSetArg(al[ac], XmNhighlightThickness, 0);                 ac++;
    XtSetArg(al[ac], XmNwidth, interp_width);                   ac++;
    XtSetArg(al[ac], XmNheight, ColInterpTileHeight(cmw));      ac++;
    XtSetArg(al[ac], XmNexposeCallback, cmw_interp_expose_cb);  ac++;
    XtSetArg(al[ac], XmNarmCallback, cmw_interp_input_cb);    	ac++;
    XtSetArg(al[ac], XmNactivateCallback, cmw_interp_input_cb); ac++;
    XtSetArg(al[ac], XmNdisarmCallback, cmw_interp_input_cb);   ac++;

    ColInterpDAWid(cmw) = (Widget) XmCreateDrawnButton(ColInterpFrameWid(cmw), 
					  "interpolator drawing area",
					  al, ac);

    XtOverrideTranslations (ColPickerDAWid(cmw), cmwpick_translations_parsed);
    XtOverrideTranslations (ColInterpDAWid(cmw), cmwinterp_translations_parsed);

    CreateColPBGadget(cmw, 
		ColPickerMixerWid(cmw),
		ColSmearLab(cmw),
		&dummy);

    /* Create pixmaps for paint bucket buttons */

    lbucketpix = XCreatePixmapFromBitmapData (dpy, 
					    DefaultRootWindow(dpy),
					    (char *)lbucket_bits,
					    lbucket_width, 
					    lbucket_height, 
					    cmw->manager.foreground, 
					    cmw->core.background_pixel,
					    cmw->core.depth);

    rbucketpix = XCreatePixmapFromBitmapData (dpy, 
					    DefaultRootWindow(dpy),
					    (char *)rbucket_bits,
					    rbucket_width, 
					    rbucket_height, 
					    cmw->manager.foreground, 
					    cmw->core.background_pixel,
					    cmw->core.depth);

    XtSetArg (al[0], XmNarmColor, &armcolor);
    XtGetValues (dummy, al, 1);

    lbucketarmpix = XCreatePixmapFromBitmapData (dpy, 
					    DefaultRootWindow(dpy),
					    (char *)lbucket_bits,
					    lbucket_width, 
					    lbucket_height, 
					    cmw->manager.foreground, 
					    armcolor,	
					    cmw->core.depth);

    rbucketarmpix = XCreatePixmapFromBitmapData (dpy, 
					    DefaultRootWindow(dpy),
					    (char *)rbucket_bits,
					    rbucket_width, 
					    rbucket_height, 
					    cmw->manager.foreground, 
					    armcolor,
					    cmw->core.depth);

    CreateColPB(cmw, 
		ColPickerMixerWid(cmw),
		NULL,
		&ColLeftBucketPbWid(cmw));

    XtSetArg(al[0], XmNactivateCallback, cmw_bucket_cb);
    XtSetArg(al[1], XmNlabelType, XmPIXMAP);
    XtSetArg(al[2], XmNlabelPixmap, lbucketpix);
    XtSetArg(al[3], XmNarmPixmap, lbucketarmpix);
    XtSetValues (ColLeftBucketPbWid(cmw), al, 4);

    CreateColPB(cmw, 
		ColPickerMixerWid(cmw),
		NULL,
		&ColRightBucketPbWid(cmw));

    XtSetArg(al[0], XmNactivateCallback, cmw_bucket_cb);
    XtSetArg(al[1], XmNlabelType, XmPIXMAP);
    XtSetArg(al[2], XmNlabelPixmap, rbucketpix);
    XtSetArg(al[3], XmNarmPixmap, rbucketarmpix);
    XtSetValues (ColRightBucketPbWid(cmw), al, 4);


    CreateColABGadget(cmw, 
		ColPickerMixerWid(cmw),
		XmARROW_UP,
		&ColLighterPbWid(cmw),
		cmw_lighter_cb);

    CreateColABGadget(cmw, 
		ColPickerMixerWid(cmw),
		XmARROW_DOWN,
		&ColDarkerPbWid(cmw),
		cmw_darker_cb);


    CreateColLabelGadget(cmw, 
		   ColPickerMixerWid(cmw), 
		   ColLighterLab(cmw), 
		   &ColLighterLabWid(cmw)); 

    CreateColLabelGadget(cmw, 
		   ColPickerMixerWid(cmw), 
		   ColDarkerLab(cmw), 
		   &ColDarkerLabWid(cmw)); 


    CreateColABGadget(cmw, 
		ColPickerMixerWid(cmw),
		XmARROW_UP,
		&ColWarmerPbWid(cmw),
		cmw_warmer_cb);

    CreateColABGadget(cmw, 
		ColPickerMixerWid(cmw),
		XmARROW_DOWN,
		&ColCoolerPbWid(cmw),
		cmw_cooler_cb);

    CreateColLabelGadget(cmw, 
		   ColPickerMixerWid(cmw), 
		   ColWarmerLab(cmw), 
		   &ColWarmerLabWid(cmw)); 

    CreateColLabelGadget(cmw, 
		   ColPickerMixerWid(cmw), 
		   ColCoolerLab(cmw), 
		   &ColCoolerLabWid(cmw)); 

    CreateColPBGadget(cmw, 
		ColPickerMixerWid(cmw),
		ColSmearLab(cmw),
		&ColSmearPbWid(cmw));

    XtSetArg(al[0], XmNactivateCallback, cmw_smear_cb);
    XtSetValues (ColSmearPbWid(cmw), al, 1);

    CreateColPBGadget(cmw, 
		ColPickerMixerWid(cmw),
		ColUndoLab(cmw),
		&ColUndoPbWid(cmw));

    XtSetArg(al[0], XmNactivateCallback, cmw_undo_cb);
    XtSetValues (ColUndoPbWid(cmw), al, 1);

    if (XtWidth(ColSmearPbWid(cmw)) != XtWidth(ColUndoPbWid(cmw)))
    {
    	if (XtWidth(ColSmearPbWid(cmw)) > XtWidth(ColUndoPbWid(cmw)))
    	{
	    XtSetArg(al[0], XmNwidth, XtWidth(ColSmearPbWid(cmw)));
    	    XtSetValues(ColUndoPbWid(cmw), al, 1);
    	}
    	else
    	{
	    XtSetArg(al[0], XmNwidth, XtWidth(ColUndoPbWid(cmw)));
    	    XtSetValues(ColSmearPbWid(cmw), al, 1);
    	}
    }

    CreateColLabelGadget(cmw, 
		   ColPickerMixerWid(cmw), 
		   ColITitleLab(cmw), 
		   &ColITitleLabWid(cmw)); 


    XtManageChildren (XtChildren(ColPickerFrameWid(cmw)), 
		      XtNumChildren(ColPickerFrameWid(cmw)));	

    XtManageChildren (XtChildren(ColInterpFrameWid(cmw)), 
		      XtNumChildren(ColInterpFrameWid(cmw)));	

    XtManageChildren (XtChildren(ColPickerMixerWid(cmw)), 
		      XtNumChildren(ColPickerMixerWid(cmw)));	

    XtDestroyWidget(dummy);

    /* Register interpolator as a drop site */

    COMPOUND_TEXT = XmInternAtom (dpy, "COMPOUND_TEXT", False);
    
    import_list[0] = COMPOUND_TEXT;

    ac = 0;
    XtSetArg (al[ac], XmNimportTargets, import_list); ac++;
    XtSetArg (al[ac], XmNnumImportTargets, 1); ac++;
    XtSetArg (al[ac], XmNdropSiteOperations, XmDROP_COPY); ac++;
    XtSetArg (al[ac], XmNdropProc, HandleColorDrop); ac++;
    XtSetArg (al[ac], XmNdragProc, InterpColorDragProc); ac++;
    XtSetArg (al[ac], XmNanimationStyle, XmDRAG_UNDER_NONE); ac++;
    XmDropSiteRegister (ColInterpDAWid(cmw), al, ac);

}

                           
/*---------------------------------------------------*/
/* creates colormix scratch pad popup 		     */
/*---------------------------------------------------*/

static void CreateColScratchPad(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[20];
    Widget dummy;
    Pixel armcolor;
    int ac = 0;
    Pixmap rbucketpix, rbucketarmpix;
    Display *dpy = (Display *) XtDisplay (cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);
    Atom import_list[2];
    
    /* Create dummy gadget to obtain default armcolor pixel */

    CreateColPBGadget(cmw, 
		      cmw,
		      ColScratchPadLab(cmw),
		      &dummy);

    XtSetArg (al[0], XmNarmColor, &armcolor);
    XtGetValues (dummy, al, 1);


    /* Create pixmaps for paint bucket buttons */

    rbucketpix = XCreatePixmapFromBitmapData (dpy, 
					    DefaultRootWindow(dpy),
					    (char *)rbucket_bits,
					    rbucket_width, 
					    rbucket_height, 
					    cmw->manager.foreground, 
					    cmw->core.background_pixel,
					    cmw->core.depth);


    rbucketarmpix = XCreatePixmapFromBitmapData (dpy, 
					    DefaultRootWindow(dpy),
					    (char *)rbucket_bits,
					    rbucket_width, 
					    rbucket_height, 
					    cmw->manager.foreground, 
					    armcolor,
					    cmw->core.depth);

    /* Create scratch pad popup */

    ac = 0;
    XtSetArg(al[ac], XmNmarginWidth, 5); 		 	ac++;
    XtSetArg(al[ac], XmNmarginHeight, 5); 		 	ac++;
    XtSetArg(al[ac], XmNautoUnmanage, FALSE); 		 	ac++;
    XtSetArg(al[ac], XmNdialogStyle, XmDIALOG_MODELESS); 	ac++;
    XtSetArg(al[ac], XmNdialogTitle, ColScratchPadLab(cmw)); 	ac++;
    XtSetArg(al[ac], XmNnoResize, TRUE); 			ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); 		ac++;

    ColSPPopupWid(cmw) = (Widget) XmCreateBulletinBoardDialog((Widget) cmw, 
					  "scratch pad popup",
					   al, ac);


    CreateColLabelGadget(cmw, 
			 ColSPPopupWid(cmw), 
			 ColSPInfoLab(cmw), 
			 &ColSPInfoLabWid(cmw)); 

    CreateColPB(cmw, 
		ColSPPopupWid(cmw),
		NULL,
		&ColSPBucketPbWid(cmw));

    XtSetArg(al[0], XmNactivateCallback, cmw_sp_bucket_cb);
    XtSetArg(al[1], XmNlabelType, XmPIXMAP);
    XtSetArg(al[2], XmNlabelPixmap, rbucketpix);
    XtSetArg(al[3], XmNarmPixmap, rbucketarmpix);
    XtSetValues (ColSPBucketPbWid(cmw), al, 4);

    ac = 0;
    XtSetArg(al[ac], XmNspacing, 0); 	  	  ac++;
    XtSetArg(al[ac], XmNborderWidth, 0);  	  ac++;

    if ((ColLayoutDirection(cmw) == DXmLAYOUT_LEFT_DOWN) ||
	(ColLayoutDirection(cmw) == DXmLAYOUT_LEFT_UP))
    {
	XtSetArg (al[ac], XmNscrollBarPlacement, XmBOTTOM_LEFT); ac++;
    }
	
    ColSPScrolledWWid(cmw) = (Widget) XmCreateScrolledWindow(ColSPPopupWid(cmw), 
					"scratch pad scrolled window",
					 al, ac);

    ac = 0;
    XtSetArg(al[ac], XmNshadowThickness, 2);       ac++;
    XtSetArg(al[ac], XmNshadowType, XmSHADOW_IN);  ac++;

    ColSPFrameWid(cmw) = (Widget) XmCreateFrame(ColSPScrolledWWid(cmw), 
					  "scratch pad frame",
					  al, ac);

    ac = 0;
    XtSetArg(al[ac], XmNshadowThickness, 0);                    ac++;
    XtSetArg(al[ac], XmNhighlightThickness, 0);                 ac++;
    XtSetArg(al[ac], XmNwidth, ColPickerTileWidth(cmw) * 3);    ac++;
    XtSetArg(al[ac], XmNheight, ColPickerTileHeight(cmw) * 2);  ac++;
    XtSetArg(al[ac], XmNtraversalOn, TRUE); 	  		ac++;
    XtSetArg(al[ac], XmNnavigationType, XmSTICKY_TAB_GROUP);    ac++;
    XtSetArg(al[ac], XmNarmCallback, cmw_sp_input_cb);        	ac++;
    XtSetArg(al[ac], XmNactivateCallback, cmw_sp_input_cb);     ac++;
    XtSetArg(al[ac], XmNdisarmCallback, cmw_sp_input_cb);       ac++;
    XtSetArg(al[ac], XmNexposeCallback, cmw_sp_expose_cb);      ac++;

    ColSPDrawingAreaWid(cmw) = (Widget) XmCreateDrawnButton(ColSPFrameWid(cmw), 
					  "scratch pad drawing area",
					  al, ac);

    XtOverrideTranslations(ColSPDrawingAreaWid(cmw), cmwsp_translations_parsed);

    ac = 0;
    XtSetArg(al[ac], XmNtraversalOn, FALSE); 			 ac++;
    XtSetArg(al[ac], XmNhighlightThickness, 2);                  ac++;
    XtSetArg(al[ac], XmNpageIncrement, 1);			 ac++;
    XtSetArg(al[ac], XmNprocessingDirection, XmMAX_ON_TOP);      ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_sp_scroll_cb); ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_sp_scroll_cb); 	 ac++;

    ColSPScrollBarWid(cmw) = (Widget) XmCreateScrollBar(ColSPScrolledWWid(cmw), 
					  "scratch pad scroll bar",
					   al, ac);


    XmScrolledWindowSetAreas (ColSPScrolledWWid(cmw),	    /* Scrolled window */
			      NULL,			    /* Horizontal scroll bar */
			      ColSPScrollBarWid(cmw),	    /* Vertical scroll bar */
			      ColSPFrameWid(cmw));   	    /* Work region */

    CreateColPBGadget(cmw, 
		ColSPPopupWid(cmw),
		ColClearLab(cmw),
		&ColSPClearPbWid(cmw));

    XtSetArg(al[0], XmNactivateCallback, cmw_clear_cb);
    XtSetValues (ColSPClearPbWid(cmw), al, 1);

    CreateColPBGadget(cmw, 
		ColSPPopupWid(cmw),
		ColCanLab(cmw),
		&ColSPCancelPbWid(cmw));

    XtSetArg(al[0], XmNactivateCallback, cmw_sp_cancel_cb);
    XtSetValues (ColSPCancelPbWid(cmw), al, 1);

    XtSetArg(al[0], XmNcancelButton, ColSPCancelPbWid(cmw));
    XtSetValues (ColSPPopupWid(cmw), al, 1);

    XtManageChild (ColSPDrawingAreaWid(cmw));
    XtManageChild (ColSPFrameWid(cmw)); 

    XtManageChildren (XtChildren(ColSPPopupWid(cmw)), 
		      XtNumChildren(ColSPPopupWid(cmw)));	

    XtDestroyWidget(dummy);

    XtSetArg(al[0], XmNnavigationType, XmSTICKY_TAB_GROUP);
    XtSetValues(ColSPPopupWid(cmw), al, 1);

    XmAddTabGroup(ColSPBucketPbWid(cmw));
    XmAddTabGroup(ColSPScrolledWWid(cmw));

    /* Register scratch pad color tile as a drop site */

    COMPOUND_TEXT = XmInternAtom (dpy, "COMPOUND_TEXT", False);
    
    import_list[0] = COMPOUND_TEXT;

    ac = 0;
    XtSetArg (al[ac], XmNimportTargets, import_list); ac++;
    XtSetArg (al[ac], XmNnumImportTargets, 1); ac++;
    XtSetArg (al[ac], XmNdropSiteOperations, XmDROP_COPY); ac++;
    XtSetArg (al[ac], XmNdropProc, HandleColorDrop); ac++;
    XtSetArg (al[ac], XmNdragProc, GenericColorDragProc); ac++;
    XtSetArg (al[ac], XmNanimationStyle, XmDRAG_UNDER_NONE); ac++;
    XmDropSiteRegister (ColSPDrawingAreaWid(cmw), al, ac);
}

                           
/*---------------------------------------------------*/
/* creates colormix default color browsing widget    */
/*---------------------------------------------------*/

static void CreateColBrowserMixer(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[20];
    int ac = 0, i, formheight=0, maxh=0, maxw=0;
    WidgetPtr pb;
    BrowserColor color;
    unsigned long fg_pixel;
    Display *dpy = XtDisplay(cmw);
			     
    XtSetArg(al[ac], XmNmarginWidth, 5); 		ac++;
    XtSetArg(al[ac], XmNmarginHeight, 5); 		ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); 	ac++;

    ColBrowserMixerWid(cmw) = (Widget) XmCreateBulletinBoard((Widget) cmw, 
					  "color mixing widget mixer",
					  al, ac);

    ac = 0;
    XtSetArg(al[ac], XmNspacing, 0); 	 ac++;
    XtSetArg(al[ac], XmNborderWidth, 0); ac++;
    if ((ColLayoutDirection(cmw) == DXmLAYOUT_LEFT_DOWN) ||
	(ColLayoutDirection(cmw) == DXmLAYOUT_LEFT_UP))
    {
	XtSetArg (al[ac], XmNscrollBarPlacement, XmBOTTOM_LEFT); ac++;
    }
	
    ColBrowserSWWid(cmw) = (Widget) XmCreateScrolledWindow(ColBrowserMixerWid(cmw), 
					"browser scrolled window",
					 al, ac);

    XmAddTabGroup(ColBrowserSWWid(cmw));

    GetBrowserButtonMaxSize (cmw, &maxw, &maxh);

    if (maxw < 250) 
	maxw = 250;

    ac = 0;
    XtSetArg(al[ac], XmNresizePolicy, XmRESIZE_GROW); 		ac++;
    XtSetArg(al[ac], XmNshadowThickness, 2); 			ac++;
    XtSetArg(al[ac], XmNshadowType, XmSHADOW_IN); 		ac++;
    XtSetArg(al[ac], XmNwidth, maxw+4); 			ac++;
    XtSetArg(al[ac], XmNmarginWidth, 0); 			ac++;
    XtSetArg(al[ac], XmNmarginHeight, 0); 			ac++;
    XtSetArg(al[ac], XmNdialogStyle, XmDIALOG_WORK_AREA); 	ac++;

    ColBrowserBBWid(cmw) = (Widget) XmCreateBulletinBoard(ColBrowserSWWid(cmw), 
					  "work area bb widget",
					  al, ac);

    ac = 0;
    XtSetArg(al[ac], XmNtraversalOn, TRUE); 				ac++;
    XtSetArg(al[ac], XmNhighlightThickness, 2);                  	ac++;
    XtSetArg(al[ac], XmNmaximum, ColBrowserColorCount(cmw)); 		ac++;
    XtSetArg(al[ac], XmNpageIncrement, ColBrowserItemCount(cmw)-1); 	ac++;
    XtSetArg(al[ac], XmNsliderSize, ColBrowserItemCount(cmw)); 		ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_bsbact_cb); 		ac++;
    XtSetArg(al[ac], XmNpageIncrementCallback, cmw_bsbact_cb); 		ac++;
    XtSetArg(al[ac], XmNpageDecrementCallback, cmw_bsbact_cb); 		ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_bsbact_cb); 			ac++;

    ColBrowserSBWid(cmw) = (Widget) XmCreateScrollBar(ColBrowserSWWid(cmw), 
					  "browser scroll bar",
					   al, ac);


    XmScrolledWindowSetAreas (ColBrowserSWWid(cmw),  /* Scrolled window */
			      NULL,		     /* Horizontal scroll bar */
			      ColBrowserSBWid(cmw),  /* Vertical scroll bar */
			      ColBrowserBBWid(cmw)); /* Work region */

    XmAddTabGroup(ColBrowserSBWid(cmw));
    XmAddTabGroup(ColBrowserBBWid(cmw));

    ColBrowserPbWid(cmw) = 
	(WidgetPtr) (XtMalloc (sizeof(Widget) * ColBrowserItemCount(cmw)));


    for (i=0, pb = ColBrowserPbWid(cmw), color = ColBrowserColors(cmw);
	 i < ColBrowserItemCount(cmw);
	 i++, pb++, color++)
    {
	CreateColPB(cmw, 
		    ColBrowserBBWid(cmw),
		    color->string, 
		    pb);

	cmw_browser_cb[0].closure = (XtPointer) color;
 
	ac = 0;
	XtSetArg(al[ac], XmNwidth, maxw); 			ac++;
	XtSetArg(al[ac], XmNheight, maxh+12); 			ac++;
	XtSetArg(al[ac], XmNborderWidth, 0); 			ac++;
	XtSetArg(al[ac], XmNmarginHeight, 0); 			ac++;
	XtSetArg(al[ac], XmNmarginWidth, 0); 			ac++;
	XtSetArg(al[ac], XmNmarginTop, 0); 			ac++;
	XtSetArg(al[ac], XmNmarginBottom, 0); 			ac++;
	XtSetArg(al[ac], XmNmarginLeft, 0); 			ac++;
	XtSetArg(al[ac], XmNmarginRight, 0); 			ac++;
	XtSetArg(al[ac], XmNshadowThickness, 0); 		ac++;
	XtSetArg(al[ac], XmNactivateCallback, cmw_browser_cb); 	ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); 	ac++;
	XtSetArg(al[ac], XmNrecomputeSize, FALSE); 		ac++;
	XtSetValues (*pb, al, ac);

	XtManageChild (*pb);
    }

    XtManageChildren (XtChildren(ColBrowserSWWid(cmw)), 
		      XtNumChildren(ColBrowserSWWid(cmw)));	

    XtManageChildren (XtChildren(ColBrowserMixerWid(cmw)), 
		      XtNumChildren(ColBrowserMixerWid(cmw)));	

}

                           
/*---------------------------------------------------*/
/* creates colormix default color picking widget     */
/*---------------------------------------------------*/

static void CreateColGrayscaleMixer(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[20];
    int ac = 0;
    double grey;
    Pixmap pixlabel;
    Display *dpy = (Display *) XtDisplay (cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);

    XtSetArg(al[ac], XmNmarginWidth, 5); 		ac++;
    XtSetArg(al[ac], XmNmarginHeight, 5); 		ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); 	ac++;
    XtSetArg(al[ac], XmNborderWidth, 0); 		ac++;

    ColGreyscaleMixerWid(cmw) = (Widget) XmCreateBulletinBoard((Widget) cmw, 
					  "color mixing widget mixer",
					  al, ac);


    /* Create the pixmap used to set the scale background at realization */

    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
	(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
	pixlabel = XCreatePixmapFromBitmapData (dpy, 
						DefaultRootWindow(dpy),
						(char *)gslabel_bits,
						gslabel_width, 
						gslabel_height, 
						BlackPixelOfScreen(screen), 
						WhitePixelOfScreen(screen), 
						DefaultDepthOfScreen(screen));
    else
	pixlabel = XCreatePixmapFromBitmapData (dpy, 
						DefaultRootWindow(dpy),
						(char *)gslabel_rtol_bits,
						gslabel_rtol_width, 
						gslabel_rtol_height, 
						BlackPixelOfScreen(screen), 
						WhitePixelOfScreen(screen), 
						DefaultDepthOfScreen(screen));
	

    ColGreyscalePixmap(cmw) = pixlabel; 

    grey = (ColNewColorRed(cmw) + ColNewColorGrn(cmw) + ColNewColorBlu(cmw))/3;
				
    grey = ((double)(grey/65535) * 100);			
					
    ac = 0;
    XtSetArg(al[ac], XmNshowValue, TRUE); 				ac++;
    XtSetArg(al[ac], XmNminimum, 0); 					ac++;
    XtSetArg(al[ac], XmNmaximum, 100); 					ac++;
    XtSetArg(al[ac], XmNscaleWidth, gslabel_width); 			ac++;
    XtSetArg(al[ac], XmNscaleHeight, gslabel_height); 			ac++;
    XtSetArg(al[ac], XmNwidth, gslabel_width); 				ac++;
    XtSetArg(al[ac], XmNvalue, (int) grey);  				ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); 			ac++;
    XtSetArg(al[ac], DXmNlayoutDirection, ColLayoutDirection(cmw)); 	ac++;
    XtSetArg(al[ac], XmNdragCallback, cmw_gsscl_cb); 			ac++;
    XtSetArg(al[ac], XmNvalueChangedCallback, cmw_gsscl_cb); 		ac++;
    XtSetArg(al[ac], XmNhelpCallback, cmw_help_cb); 			ac++;

    ColGreyscaleSclWid(cmw) = (Widget) XmCreateScale(ColGreyscaleMixerWid(cmw), 
					    "greyscale scale widget",
					    al, ac);


    XtManageChildren (XtChildren(ColGreyscaleMixerWid(cmw)), 
		      XtNumChildren(ColGreyscaleMixerWid(cmw)));	

}


/*---------------------------------------------------*/
/* sets up multi-color model option menu and pulldown*/
/*---------------------------------------------------*/

static void PopulateColorMixOptionMenu(cmw)
    DXmColorMixWidget cmw;
{
    CreateColPDMenu(cmw);

    CreateColPBGadget(cmw, ColPDMenWid(cmw), ColPDPickerLab(cmw), &ColPDPickerWid(cmw));
    CreateColPBGadget(cmw, ColPDMenWid(cmw), ColPDHLSLab(cmw), &ColPDHLSWid(cmw));
    CreateColPBGadget(cmw, ColPDMenWid(cmw), ColPDRGBLab(cmw), &ColPDRGBWid(cmw));
    CreateColPBGadget(cmw, ColPDMenWid(cmw), ColPDBrowserLab(cmw), &ColPDBrowserWid(cmw));
    CreateColPBGadget(cmw, ColPDMenWid(cmw), ColPDGreyscaleLab(cmw), &ColPDGreyscaleWid(cmw));

    XtManageChildren(XtChildren(ColPDMenWid(cmw)), 
		     XtNumChildren(ColPDMenWid(cmw)));	

    CreateColOptMenu(cmw);
}


/*---------------------------------------------------*/
/* GetValues hook routines			     */
/*						     */
/* colormix relies on subwidgets to store cs strings */
/* to save memory				     */
/*---------------------------------------------------*/

#ifdef _NO_PROTO

static void _DXmColorMixGetMainLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetMainLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;
    XmString	data;
    Arg		al[1];

    if (IsColMainLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColMainLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetDispLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetDispLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColDispLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColDispLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetMixLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetMixLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColMixLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColMixLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetSldLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetSldLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColSldLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColSldLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetValLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetValLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColValLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColValLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetRedLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetRedLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColRedLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColRedLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetGrnLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetGrnLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColGrnLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColGrnLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetBluLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetBluLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColBluLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColBluLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetHueLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetHueLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColHueLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColHueLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetLightLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetLightLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColLightLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColLightLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetSatLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetSatLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColSatLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColSatLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetBlkLabelStr (w, resource, value)
    Widget	w;	
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetBlkLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColBlkLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColBlkLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetWhtLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetWhtLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColWhtLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColWhtLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetGryLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetGryLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColGryLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColGryLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}


#ifdef _NO_PROTO

static void _DXmColorMixGetFulLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetFulLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColFulLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColFulLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetOptLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetOptLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColOptLabWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColOptLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
   	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetHLSLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetHLSLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColPDHLSWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColPDHLSWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
   	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetRGBLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetRGBLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColPDRGBWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColPDRGBWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
   	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetPickerLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetPickerLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColPDPickerWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColPDPickerWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
   	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetOkLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetOkLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColOkPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColOkPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetApplyLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetApplyLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColAppPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColAppPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetCancelLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetCancelLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColCanPbWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColCanPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetResetLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetResetLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColResPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColResPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetUndoLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetUndoLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColUndoPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColUndoPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetSmearLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetSmearLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColSmearPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColSmearPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetHelpLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetHelpLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColHelpPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColHelpPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetPTitleLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetPTitleLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColPTitleLabWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColPTitleLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetSpectrumLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetSpectrumLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColSpectrumPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColSpectrumPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetPastelLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetPastelLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColPastelPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColPastelPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetMetallicLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetMetallicLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColMetallicPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColMetallicPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetEarthLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetEarthLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColEarthtonePbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColEarthtonePbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetUserPalLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetUserPalLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColUserPalettePbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColUserPalettePbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetITitleLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetITitleLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColITitleLabWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColITitleLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetWarmerLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetWarmerLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColWarmerLabWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColWarmerLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetCoolerLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetCoolerLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColCoolerLabWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColCoolerLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetLighterLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetLighterLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColLighterLabWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColLighterLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetDarkerLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetDarkerLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColDarkerLabWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColDarkerLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetClearLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetClearLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColSPClearPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColSPClearPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetSPInfoLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetSPInfoLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColSPInfoLabWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColSPInfoLabWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetSPLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetSPLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColScratchPadPbWid(cmw))
    {
	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColScratchPadPbWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetBrowserLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetBrowserLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColPDBrowserWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColPDBrowserWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
   	*value = (XtArgVal) NULL;
}

#ifdef _NO_PROTO

static void _DXmColorMixGetGreysclLabelStr (w, resource, value)
    Widget	w;
    int		resource;
    XtArgVal		*value;

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetGreysclLabelStr ( Widget w , int resource , XtArgVal *value )

#endif /* _NO_PROTO undefined */
{
    XmString	data;
    Arg		al[1];
    DXmColorMixWidget cmw = (DXmColorMixWidget) w;

    if (IsColPDGreyscaleWid(cmw))
    {
    	XtSetArg (al[0], XmNlabelString, &data);
	XtGetValues (ColPDGreyscaleWid(cmw), al, 1);
	*value = (XtArgVal) data;
    }
    else
   	*value = (XtArgVal) NULL;
}



/*---------------------------------------------------*/
/* SetValues routines 				     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/* checks if colormix margins have been modified     */
/*---------------------------------------------------*/

static Boolean CheckMargins(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    if (newcmw->bulletin_board.margin_width != oldcmw->bulletin_board.margin_width ||
    	newcmw->bulletin_board.margin_height != oldcmw->bulletin_board.margin_height)
    	return (Boolean) (TRUE);
    else
    	return (Boolean) (FALSE);
}


                           
/*---------------------------------------------------*/
/* updates the colormix default color display widget */
/* margins and dimensions of the display windows -   */
/* called from setvalues			     */
/*						     */
/* (note that display window is updated regardless   */
/*  of whether or not it is managed)                 */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateDispMargins(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    Arg al[2];
    Boolean change = FALSE;

    if (!IsColDispWid(newcmw))
	return (Boolean) (change);

    if (ColDispViewM(newcmw) != ColDispViewM(oldcmw))
    {
    	XtSetArg(al[0], XmNmarginWidth,  ColDispViewM(newcmw));
    	XtSetArg(al[1], XmNmarginHeight, ColDispViewM(newcmw));
    	XtSetValues(ColDispWid(newcmw), al, 2);
	change = TRUE;
    }

    if (ColDispViewW(newcmw) != ColDispViewW(oldcmw)
			    ||
        ColDispViewH(newcmw) != ColDispViewH(oldcmw))
    {
	XtSetArg(al[0], XmNwidth,  ColDispViewW(newcmw));
	XtSetArg(al[1], XmNheight, ColDispViewH(newcmw));
    	XtSetValues(ColOrigWid(newcmw), al, 2);
    	XtSetValues(ColNewWid(newcmw),  al, 2);
	change = TRUE;
    }

    return (Boolean) (change);
}

                           
/*---------------------------------------------------*/
/* checks and updates picker color array if a new    */
/* user palette is set - called from setvalues.      */
/*---------------------------------------------------*/

static Boolean CheckAndUpdatePickerColors(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{

/* not supported */

}

                           
/*---------------------------------------------------*/
/* checks and updates color display widget - called  */
/* from setvalues				     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateDispWid(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    if (ColCurDispWid(newcmw) == ColCurDispWid(oldcmw))
    	return (Boolean) (FALSE);

    /*
     * Caller has 'changed' display widget field.  If the caller has set the 
     * display widget id to NULL, use default display widget.  Otherwise, 
     * unmanage default display if present & managed.
     */
    if (!IsColCurDispWid(newcmw))
    {
	if (!IsColDispWid(newcmw))
	    CreateColDisplay(newcmw);

	ColCurDispWid(newcmw) = (Widget) ColDispWid(newcmw);
	ColDefDisp(newcmw) = TRUE;
	newcmw->colormix.setnewcolproc = (DXmColorSetProc) default_setdisplay_proc;
	newcmw->colormix.setmixcolproc = (DXmColorSetProc) default_setmixer_proc;

	if (!XtIsRealized(newcmw))
	    XtManageChild(ColCurDispWid(newcmw));
        else
	{
	    if (ColDispType(newcmw) == BlackAndWhite || ColDispType(newcmw) == StatGray)
	    	RemoveColDisplay(newcmw);
	    else
	    	XtManageChild(ColCurDispWid(newcmw));
	}
    }
    else
	if (ColDefDisp(newcmw))
    	{
	    ColDefDisp(newcmw) = FALSE;

/*
   this should be done by the user -- just add check in the procs
   changed 22-aug-89...

	    if (newcmw->colormix.setnewcolproc == default_setdisplay_proc)
	    	newcmw->colormix.setnewcolproc = NULL;
*/

	    if (IsColDispWid(newcmw))
	    	if (XtIsManaged(ColDispWid(newcmw)))
		    XtUnmanageChild(ColDispWid(newcmw));
    	}

    return (Boolean) (TRUE);
}

                           
/*---------------------------------------------------*/
/* checks and updates color mixer widget - called    */
/* from setvalues				     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateMixerWid(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    if (ColCurMixerWid(newcmw) == ColCurMixerWid(oldcmw))
    	return (Boolean) (FALSE);

    /*
     * Caller has 'changed' mixer widget field.  If the caller has set the 
     * mixer widget id to NULL, use default mixer.  Otherwise, unmanage
     * default mixer (if present & managed).
     */
    if (!IsColCurMixerWid(newcmw))
    {
	if (!IsColMixerWid(newcmw))
	{
	    switch (ColModel(newcmw))
	    {
		case DXmColorModelHLS:
		    ColMixerWid(newcmw) = (Widget) ColHLSMixerWid(newcmw);
		    break;		

		case DXmColorModelRGB:
		    ColMixerWid(newcmw) = (Widget) ColRGBMixerWid(newcmw);
		    break;		

		case DXmColorModelBrowser:
		    ColMixerWid(newcmw) = (Widget) ColBrowserMixerWid(newcmw);
		    break;		

		case DXmColorModelGreyscale:
		    ColMixerWid(newcmw) = (Widget) ColGreyscaleMixerWid(newcmw);
		    break;		

		default:
		    ColMixerWid(newcmw) = (Widget) ColPickerMixerWid(newcmw);
		    break;		
	    }	
        }

	ColCurMixerWid(newcmw) = (Widget) ColMixerWid(newcmw);
	ColDefMixer(newcmw) = TRUE;

	XtManageChild(ColCurMixerWid(newcmw));
    }
    else
	if (ColDefMixer(newcmw))
	{
	    ColDefMixer(newcmw) = FALSE;

/*
   this should be done by the user -- just add check in the procs
   changed 22-aug-89...

	    if (newcmw->colormix.setmixcolproc == default_setmixer_proc)
	    	newcmw->colormix.setmixcolproc = NULL;
*/

	    if (IsColMixerWid(newcmw))
	    	if (XtIsManaged(ColMixerWid(newcmw)))
		    XtUnmanageChild(ColMixerWid(newcmw));
    	}

   if (IsColOptMenWid(newcmw))
   {
	if (ColDefMixer(newcmw))
        {
	    if (!XtIsManaged(ColOptMenWid(newcmw)))
		XtManageChild(ColOptMenWid(newcmw));
	}
	else
	{
	    if (XtIsManaged(ColOptMenWid(newcmw)))
		XtUnmanageChild(ColOptMenWid(newcmw));
	}
    }

    return (Boolean) (TRUE);
}

                           
/*---------------------------------------------------*/
/* checks and updates new color value - called from  */
/* SetValues				  	     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateNewColors(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    if (ColNewColorRed(newcmw) == ColNewColorRed(oldcmw)
				 &&
	ColNewColorGrn(newcmw) == ColNewColorGrn(oldcmw)
				 &&
	ColNewColorBlu(newcmw) == ColNewColorBlu(oldcmw))
        return (Boolean) (FALSE);
    else
    {
	DXmColorMixSetNewColor(newcmw,
			       ColNewColorRed(newcmw), 
			       ColNewColorGrn(newcmw), 
		               ColNewColorBlu(newcmw));

        return (Boolean) (TRUE);
    }
}

                           
/*---------------------------------------------------*/
/* checks and updates orig color value - called from */
/* SetValues				  	     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateOrigColors(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    if (ColOrigColorRed(newcmw) == ColOrigColorRed(oldcmw)
				 &&
	ColOrigColorGrn(newcmw) == ColOrigColorGrn(oldcmw)
				 &&
	ColOrigColorBlu(newcmw) == ColOrigColorBlu(oldcmw))
        return (Boolean) (FALSE);
    else
    {
	SetOrigColor(newcmw, 
		    ColOrigColorRed(newcmw), 
		    ColOrigColorGrn(newcmw), 
		    ColOrigColorBlu(newcmw));
        return (Boolean) (TRUE);
    }
}

                           
/*---------------------------------------------------*/
/* checks and updates background color value of color*/
/* display widget - called from SetValues	     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateBackColors(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    if (ColBackColorRed(newcmw) == ColBackColorRed(oldcmw)
				 &&
	ColBackColorGrn(newcmw) == ColBackColorGrn(oldcmw)
				 &&
	ColBackColorBlu(newcmw) == ColBackColorBlu(oldcmw))
        return (int) (FALSE);
    else
    {
	SetUpColors(newcmw);  /* when changing back, must set all 3 */
        return (Boolean) (TRUE);
    }
}

                           
/*---------------------------------------------------*/
/* checks and updates default display widget color   */
/* values.  Note that 'TRUE' is returned only when   */
/* color values fields changed AND the actual colors */
/* currently displayed change as well - called from  */
/* SetValues.                                        */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateColors(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    int change = 0;

    if (!ColDefDisp(newcmw))
	return (Boolean) (FALSE);

    change = CheckAndUpdateNewColors(oldcmw, newcmw);
    change = CheckAndUpdateOrigColors(oldcmw, newcmw) || change;
    change = CheckAndUpdateBackColors(oldcmw, newcmw) || change;

    return (Boolean) (change);
}

                           
/*---------------------------------------------------*/
/* called from SetValues, this routines updates the  */
/* colormix mixing tool and option menu to specified */
/* color model					     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateColorModel(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    unsigned char resize_mode;

    if (ColModel(newcmw) == ColModel(oldcmw))
	return (Boolean) (FALSE);

    if (!ColDefMixer(newcmw))
	return (Boolean) (FALSE);

    UpdateMixer(newcmw);
    UpdateMenu(newcmw);

    if (XtIsManaged(newcmw) && XtIsRealized(newcmw))
    	return (Boolean) (TRUE);
    else
	return (Boolean) (FALSE);
}


/*---------------------------------------------------*/
/* Checks and updates layout if RToL direction has   */
/* changed.  Called from SetValues.		     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateLayoutDir( oldcmw,newcmw )
     DXmColorMixWidget oldcmw,newcmw;
{

    Arg  arglist[1];
    Widget *kid ;
    int i;

    if (ColLayoutDirection(newcmw) != ColLayoutDirection(oldcmw))
    {
       XtSetArg(arglist[0], DXmNlayoutDirection, ColLayoutDirection(newcmw));

       /* Propagate layout direction to direct children of the Colormix widget */

       for (i = 0, kid = newcmw->composite.children;
            i < newcmw->composite.num_children;
            i++, kid++)

               if (!(*kid)->core.being_destroyed)
                  XtSetValues(*kid, arglist, 1);  

       /* Propagate layout direction to the bulletin boards children widgets */
 
       if (IsColRedSclWid(newcmw))
	  if (XtIsManaged(ColRedSclWid(newcmw)))
	     XtSetValues(ColRedSclWid(newcmw), arglist, 1);

       if (IsColGrnSclWid(newcmw))
	  if (XtIsManaged(ColGrnSclWid(newcmw)))
	     XtSetValues(ColGrnSclWid(newcmw), arglist, 1);

       if (IsColBluSclWid(newcmw))
	  if (XtIsManaged(ColBluSclWid(newcmw)))
	     XtSetValues(ColBluSclWid(newcmw), arglist, 1);

       if (IsColHueSclWid(newcmw))
	  if (XtIsManaged(ColHueSclWid(newcmw)))
	     XtSetValues(ColHueSclWid(newcmw), arglist, 1);

       if (IsColLightSclWid(newcmw))
	  if (XtIsManaged(ColLightSclWid(newcmw)))
	     XtSetValues(ColLightSclWid(newcmw), arglist, 1);

       if (IsColSatSclWid(newcmw))
	  if (XtIsManaged(ColSatSclWid(newcmw)))
	     XtSetValues(ColSatSclWid(newcmw), arglist, 1);

       if (IsColPDGreyscaleWid(newcmw))
	  if (XtIsManaged(ColPDGreyscaleWid(newcmw)))
	     XtSetValues(ColPDGreyscaleWid(newcmw), arglist, 1);

       if (IsColGreyscaleMixerWid(newcmw))
	  if (XtIsManaged(ColGreyscaleMixerWid(newcmw)))
	     XtSetValues(ColGreyscaleMixerWid(newcmw), arglist, 1);

       if (IsColGreyscaleSclWid(newcmw))
	  if (XtIsManaged(ColGreyscaleSclWid(newcmw)))
	     XtSetValues(ColGreyscaleSclWid(newcmw), arglist, 1);

       if (IsColBrowserSWWid(newcmw))
	  if (XtIsManaged(ColBrowserSWWid(newcmw)))
	     XtSetValues(ColBrowserSWWid(newcmw), arglist, 1);

       return (Boolean) (TRUE);
    }
    return (Boolean) (FALSE);       
}
                         
                           
/*---------------------------------------------------*/
/* checks and updates the picker and interpolator    */
/* drawing area widget sizes if the tile size has    */
/* changed.					     */
/*---------------------------------------------------*/

static Boolean CheckAndUpdateTileSizes(oldcmw,newcmw)
    DXmColorMixWidget oldcmw;
    DXmColorMixWidget newcmw;
{
    Arg al[5];
    int ac=0;
    Boolean change = FALSE, managed;

    if (ColPickerTileWidth(oldcmw) != ColPickerTileWidth(newcmw))
    {
	change = TRUE;
	if (ColPickerTileWidth(newcmw) < 1)
	    ColPickerTileWidth(newcmw) = 1;
	XtSetArg (al[ac], XmNwidth, ColPickerTileWidth(newcmw) 
				    * ColPickerColorCount(newcmw));
	ac++;
    }

    if (ColPickerTileHeight(oldcmw) != ColPickerTileHeight(newcmw))
    {
	change = TRUE;
	if (ColPickerTileHeight(newcmw) < 1)
	    ColPickerTileHeight(newcmw) = 1;
	XtSetArg (al[ac], XmNheight, ColPickerTileHeight(newcmw));
	ac++;
    }

    if (ac>0)
    {
	if (XtIsManaged(newcmw))
	    XtUnmanageChild (ColPickerDAWid(newcmw));

	XtSetValues (ColPickerDAWid(newcmw), al, ac);

	if (XtIsManaged(newcmw))
	    XtManageChild (ColPickerDAWid(newcmw));
    }

    ac=0;

    if (ColInterpTileWidth(oldcmw) != ColInterpTileWidth(newcmw))
    {
	change = TRUE;
	if (ColInterpTileWidth(newcmw) < 1)
	    ColInterpTileWidth(newcmw) = 1;
	XtSetArg (al[ac], XmNwidth, ColInterpTileWidth(newcmw) 
				    * ColInterpTileCount(newcmw));
	ac++;
    }

    if (ColInterpTileHeight(oldcmw) != ColInterpTileHeight(newcmw))
    {
	change = TRUE;
	if (ColInterpTileHeight(newcmw) < 1)
	    ColInterpTileHeight(newcmw) = 1;
	XtSetArg (al[ac], XmNheight, ColInterpTileHeight(newcmw));
	ac++;
    }

    if (ac>0)
    {
	if (XtIsManaged(newcmw))
	    XtUnmanageChild (ColInterpDAWid(newcmw));

	XtSetValues (ColInterpDAWid(newcmw), al, ac);

	if (XtIsManaged(newcmw))
	    XtManageChild (ColInterpDAWid(newcmw));
    }

    return change;
}

                           
/*---------------------------------------------------*/
/* update colormix mixer tool to specified color     */
/* model - called whenever the colormodel changes    */
/* (either through setvalues of option menu)         */
/*---------------------------------------------------*/

static void UpdateMixer(cmw)
    DXmColorMixWidget cmw;
{
    ColNoLayout(cmw) = TRUE;

    if (XtIsManaged(ColCurMixerWid(cmw)))
	XtUnmanageChild(ColCurMixerWid(cmw));

    /* update to current default mixing tool to the latest 'new' color */

    if (cmw->colormix.setmixcolproc != NULL)
    	(* cmw->colormix.setmixcolproc) (cmw,ColNewColorRed(cmw), 
		        		     ColNewColorGrn(cmw), 
		        		     ColNewColorBlu(cmw)); 

    if ((ColCurMixerWid(cmw) == ColPickerMixerWid(cmw)) &&
	(ColIsOrigSelected(cmw) || ColIsNewSelected(cmw)))
	    UnhighlightCurrentColor(cmw);

    switch (ColModel(cmw))
    {
	case DXmColorModelHLS:
	    ColMixerWid(cmw) = (Widget) ColHLSMixerWid(cmw);
	    break;		

	case DXmColorModelRGB:
	    ColMixerWid(cmw) = (Widget) ColRGBMixerWid(cmw);
	    break;		

	case DXmColorModelBrowser:
	    ColMixerWid(cmw) = (Widget) ColBrowserMixerWid(cmw);
	    break;		

	case DXmColorModelGreyscale:
	    ColMixerWid(cmw) = (Widget) ColGreyscaleMixerWid(cmw);
	    break;		

	default:
	    ColMixerWid(cmw) = (Widget) ColPickerMixerWid(cmw);
	    break;		
    }

    if (ColModel(cmw) == DXmColorModelBrowser)
	CheckBrowserInitialization(cmw);

    if (ColModel(cmw) == DXmColorModelGreyscale)
	SetGreyscaleColors(cmw);

    ColCurMixerWid(cmw) = (Widget) ColMixerWid(cmw);

    ColNoLayout(cmw) = FALSE;

    XtManageChild(ColCurMixerWid(cmw));
}

                           
/*---------------------------------------------------*/
/* update colormix option menu history to reflect    */
/* current color model used in color mixer widget -  */
/* called whenever model is changed w/o option menu  */
/*---------------------------------------------------*/

static void UpdateMenu(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[1];

    switch (ColModel(cmw))
    {
	case DXmColorModelHLS:
	    XtSetArg(al[0], XmNmenuHistory, ColPDHLSWid(cmw));
	    break;		

	case DXmColorModelRGB:
	    XtSetArg(al[0], XmNmenuHistory, ColPDRGBWid(cmw));
	    break;		

	case DXmColorModelBrowser:
	    XtSetArg(al[0], XmNmenuHistory, ColPDBrowserWid(cmw));
	    break;		

	case DXmColorModelGreyscale:
	    XtSetArg(al[0], XmNmenuHistory, ColPDGreyscaleWid(cmw));
	    break;		

	default:
	    XtSetArg(al[0], XmNmenuHistory, ColPDPickerWid(cmw));
	    break;		
    }

    XtSetValues(ColOptMenWid(cmw),al,1);
}

                           
/*---------------------------------------------------*/
/* updates colormix child widget label text string   */
/* to change specified via SetValues		     */
/*---------------------------------------------------*/

static void UpdateText(cmw,w,label)
    DXmColorMixWidget cmw;
    Widget w;
    XmString *label;
{
    Arg al[2];

    XtSetArg(al[0], XmNlabelString, *label);
    XtSetArg(al[1], XmNstringDirection, ColDirection(cmw)); 
    XtSetValues(w, al, 2);

    *label = NULL;
}


/*---------------------------------------------------*/
/* layout geometry handing routines 		     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/* places colormix default display sub-widgets in    */
/* correct positions relative to each other	     */
/*---------------------------------------------------*/

static void LayoutDisplay(cmw)
    DXmColorMixWidget cmw;
{
    int x,y;
    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColDispWid(cmw);
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    x = w->bulletin_board.margin_width;
    y = w->bulletin_board.margin_height;
    
    if (IsColOrigWid(cmw))
	if (XtIsManaged(ColOrigWid(cmw))) 
	{
	    ColMoveObject(ColOrigWid(cmw), x, y);
	    x = XtX(ColOrigWid(cmw)) + XtWidth(ColOrigWid(cmw));
	}

    if (IsColNewWid(cmw))
	if (XtIsManaged(ColNewWid(cmw))) 
	    ColMoveObject(ColNewWid(cmw), x, y);

    (*superclass->composite_class.change_managed) ((Widget)w);
}


/*---------------------------------------------------*/
/* places colormix default HLS mixer sub-widgets in  */
/* correct positions relative to each other	     */
/*---------------------------------------------------*/

static void LayoutHLSMixer(cmw)
    DXmColorMixWidget cmw;
{
    int mw,mh,x,y,temp, label_width = 0;
    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColHLSMixerWid(cmw);
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    mw = w->bulletin_board.margin_width;
    mh = w->bulletin_board.margin_height;

    x = mw;
    y = mh;
    
    GetHLSLabelWidth(cmw, &label_width);

    if (XtHeight(ColHueSclWid(cmw)) < XtHeight(ColHueLabWid(cmw)))
	y += XtHeight(ColHueLabWid(cmw)) - XtHeight(ColHueSclWid(cmw));

    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
	(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
    {
	if (IsColBlkLabWid(cmw))
	{
	    temp = 0;
	    if (IsColLightLabWid(cmw))
		temp = XtWidth (ColLightLabWid(cmw));

	    if (XtWidth(ColBlkLabWid(cmw)) > temp+mw+XtWidth(ColLightSclWid(cmw))/2)
		x = XtWidth (ColBlkLabWid(cmw)) - XtWidth(ColLightSclWid(cmw))/2-mw;
	}

	if (IsColGryLabWid(cmw))
	{
	    temp = 0;
	    if (IsColSatLabWid(cmw))
		temp = XtWidth (ColSatLabWid(cmw));

	    if (XtWidth(ColGryLabWid(cmw)) > temp+mw+XtWidth(ColSatSclWid(cmw))/2)
	    {
		temp = XtWidth (ColGryLabWid(cmw)) - XtWidth(ColSatSclWid(cmw))/2-mw;
		if (temp > x)
		    x = temp;
	    }
	}
    }
    else    /* Right to left */
    {
	if (IsColWhtLabWid(cmw))
	{
	    if (XtWidth(ColWhtLabWid(cmw)) > XtWidth(ColLightSclWid(cmw))/2)
		x = XtWidth (ColWhtLabWid(cmw)) - XtWidth(ColLightSclWid(cmw))/2;
	}

	if (IsColFulLabWid(cmw))
	{
	    temp = 0;
	    if (XtWidth(ColFulLabWid(cmw)) > XtWidth(ColSatSclWid(cmw))/2)
		temp = XtWidth (ColFulLabWid(cmw)) - XtWidth(ColSatSclWid(cmw))/2;
	    if (temp > x)
		x = temp;
	}
    }

    if (IsColHueSclWid(cmw))
	if (XtIsManaged(ColHueSclWid(cmw)))
	{ 
	    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
	    (ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
	    {	
		if (IsColHueLabWid(cmw))
		{ 
		    if (XtIsManaged(ColHueLabWid(cmw)))
			ColMoveObject(ColHueLabWid(cmw), 
				 x+label_width-XtWidth(ColHueLabWid(cmw)), 
				 y + XtHeight(ColHueSclWid(cmw)) - XtHeight (ColHueLabWid(cmw)));
		}

		ColMoveObject(ColHueSclWid(cmw), x+label_width+mw, y);
	    }

	    else /* Layout right to left */
	    {	
		ColMoveObject(ColHueSclWid(cmw), x, y);

		if (IsColHueLabWid(cmw))
		{ 
		    if (XtIsManaged(ColHueLabWid(cmw)))
			ColMoveObject(ColHueLabWid(cmw), 
				 x + XtWidth(ColHueSclWid(cmw)) + mw, 
				 y + XtHeight(ColHueSclWid(cmw)) - XtHeight (ColHueLabWid(cmw)));
		}

	    }

    	    y = y + XtHeight(ColHueSclWid(cmw)) + (5*mh);
	}

    if (IsColLightSclWid(cmw))
	if (XtIsManaged(ColLightSclWid(cmw)))
	{ 
	    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
	    (ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
	    {
		if (IsColBlkLabWid(cmw))
		{
		    if (XtIsManaged(ColBlkLabWid(cmw)))
		    {
			if (XtWidth(ColBlkLabWid(cmw)) > XtWidth(ColLightSclWid(cmw))/2)
			    ColMoveObject(ColBlkLabWid(cmw), 
				x+label_width+mw+XtWidth(ColLightSclWid(cmw))/2
				- XtWidth (ColBlkLabWid(cmw)), y);
			else
			    ColMoveObject(ColBlkLabWid(cmw), 
				x+label_width+mw, y);
		    }
		}

		if (IsColWhtLabWid(cmw))
		{
		    if (XtIsManaged(ColWhtLabWid(cmw)))
		    {
			if (XtWidth(ColWhtLabWid(cmw)) > XtWidth(ColLightSclWid(cmw))/2)
			    ColMoveObject(ColWhtLabWid(cmw), 
				x+label_width+mw+XtWidth(ColLightSclWid(cmw))/2,
				y);
			else
			    ColMoveObject(ColWhtLabWid(cmw), 
				x+label_width+mw+XtWidth(ColLightSclWid(cmw))
				- XtWidth(ColWhtLabWid(cmw)), y);

		    y = y + XtHeight (ColWhtLabWid(cmw));
		    }
		}

		if (IsColLightLabWid(cmw))
		{ 
		    if (XtIsManaged(ColLightLabWid(cmw)))
			ColMoveObject(ColLightLabWid(cmw), 
				 x+label_width-XtWidth(ColLightLabWid(cmw)), 
				 y + XtHeight(ColLightSclWid(cmw)) - XtHeight (ColLightLabWid(cmw)));
		}

		ColMoveObject(ColLightSclWid(cmw), x+label_width+mw, y);
	    }

	    else /* Layout right to left */
	    {	
		if (IsColBlkLabWid(cmw))
		{
		    if (XtIsManaged(ColBlkLabWid(cmw)))
		    {
			if (XtWidth(ColBlkLabWid(cmw)) > XtWidth(ColLightSclWid(cmw))/2)
			    ColMoveObject(ColBlkLabWid(cmw), 
				x+XtWidth(ColLightSclWid(cmw))/2,
				y);
			else
			    ColMoveObject(ColBlkLabWid(cmw), 
				x+XtWidth(ColLightSclWid(cmw)) 
				- XtWidth(ColBlkLabWid(cmw)), y);
		    }
		}

		if (IsColWhtLabWid(cmw))
		{
		    if (XtIsManaged(ColWhtLabWid(cmw)))
		    {
			if (XtWidth(ColWhtLabWid(cmw)) > XtWidth(ColLightSclWid(cmw))/2)
			    ColMoveObject(ColWhtLabWid(cmw), 
				x+XtWidth(ColLightSclWid(cmw))/2
				- XtWidth(ColWhtLabWid(cmw)), y);
			else
			    ColMoveObject(ColWhtLabWid(cmw), 
				x, y);

		    y = y + XtHeight (ColWhtLabWid(cmw));
		    }
		}

		ColMoveObject(ColLightSclWid(cmw), x, y);

		if (IsColLightLabWid(cmw))
		{ 
		    if (XtIsManaged(ColLightLabWid(cmw)))
			ColMoveObject(ColLightLabWid(cmw), 
				 x + XtWidth(ColLightSclWid(cmw)) + mw, 
				 y + XtHeight(ColLightSclWid(cmw)) - XtHeight (ColLightLabWid(cmw)));
		}
	    }

    	    y = y + XtHeight(ColLightSclWid(cmw)) + (5*mh);

	}

    if (IsColSatSclWid(cmw))
	if (XtIsManaged(ColSatSclWid(cmw)))
	{ 
	    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
		(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
	    {	
		if (IsColGryLabWid(cmw))
		{
		    if (XtIsManaged(ColGryLabWid(cmw)))
		    {
			if (XtWidth(ColGryLabWid(cmw)) > XtWidth(ColSatSclWid(cmw))/2)
			    ColMoveObject(ColGryLabWid(cmw), 
				x+label_width+mw+XtWidth(ColSatSclWid(cmw))/2
				- XtWidth (ColGryLabWid(cmw)), y);
			else
			    ColMoveObject(ColGryLabWid(cmw), 
				x+label_width+mw, y);
		    }
		}

		if (IsColFulLabWid(cmw))
		{
		    if (XtIsManaged(ColFulLabWid(cmw)))
		    {
			if (XtWidth(ColFulLabWid(cmw)) > XtWidth(ColSatSclWid(cmw))/2)
			    ColMoveObject(ColFulLabWid(cmw), 
				x+label_width+mw+XtWidth(ColSatSclWid(cmw))/2,
				y);
			else
			    ColMoveObject(ColFulLabWid(cmw), 
				x+label_width+mw+XtWidth(ColSatSclWid(cmw))
				- XtWidth(ColFulLabWid(cmw)), y);

		    y = y + XtHeight (ColFulLabWid(cmw));
		    }
		}

		if (IsColSatLabWid(cmw))
		{ 
		    if (XtIsManaged(ColSatLabWid(cmw)))
			ColMoveObject(ColSatLabWid(cmw),
				 x+label_width-XtWidth(ColSatLabWid(cmw)), 
				 y + XtHeight(ColHueSclWid(cmw)) - XtHeight (ColSatLabWid(cmw)));
		}

		ColMoveObject(ColSatSclWid(cmw), x+label_width+mw, y);
	    }

	    else /* Layout right to left */
	    {	
		if (IsColGryLabWid(cmw))
		{
		    if (XtIsManaged(ColGryLabWid(cmw)))
		    {
			if (XtWidth(ColGryLabWid(cmw)) > XtWidth(ColSatSclWid(cmw))/2)
			    ColMoveObject(ColGryLabWid(cmw), 
				x+XtWidth(ColSatSclWid(cmw))/2,
				y);
			else
			    ColMoveObject(ColGryLabWid(cmw), 
				x+XtWidth(ColSatSclWid(cmw)) 
				- XtWidth(ColGryLabWid(cmw)), y);
		    }
		}

		if (IsColFulLabWid(cmw))
		{
		    if (XtIsManaged(ColFulLabWid(cmw)))
		    {
			if (XtWidth(ColFulLabWid(cmw)) > XtWidth(ColSatSclWid(cmw))/2)
			    ColMoveObject(ColFulLabWid(cmw), 
				x+XtWidth(ColSatSclWid(cmw))/2
				- XtWidth(ColFulLabWid(cmw)), y);
			else
			    ColMoveObject(ColFulLabWid(cmw), 
				x, y);

		    y = y + XtHeight (ColFulLabWid(cmw));
		    }
		}

		ColMoveObject(ColSatSclWid(cmw), x, y);

		if (IsColSatLabWid(cmw))
		{ 
		    if (XtIsManaged(ColSatLabWid(cmw)))
			ColMoveObject(ColSatLabWid(cmw), 
				 x + XtWidth(ColSatSclWid(cmw)) + mw, 
				 y + XtHeight(ColSatSclWid(cmw)) - XtHeight (ColSatLabWid(cmw)));
		}
	    }
	}

    (*superclass->composite_class.change_managed) ((Widget)w);
}


/*---------------------------------------------------*/
/* places colormix default RGB mixer sub-widgets in  */
/* correct positions relative to each other	     */
/*---------------------------------------------------*/

static void LayoutRGBMixer(cmw)
    DXmColorMixWidget cmw;
{
    int mw,mh,x,y;
    int textx = 0;
    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColRGBMixerWid(cmw);
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    mw = w->bulletin_board.margin_width;
    mh = w->bulletin_board.margin_height;
    
    x = mw;
    y = mh;

    if (IsColSldLabWid(cmw))
	if (XtIsManaged(ColSldLabWid(cmw)))
	{ 
	    ColMoveObject(ColSldLabWid(cmw), x, y);
    	    y = XtY(ColSldLabWid(cmw)) + XtHeight(ColSldLabWid(cmw)) + mh;
	}

    if (IsColRedSclWid(cmw))
	if (XtIsManaged(ColRedSclWid(cmw)))
	{ 
	    ColMoveObject(ColRedSclWid(cmw), x, y);

    	    if (IsColRedTextWid(cmw))
	    	if (XtIsManaged(ColRedTextWid(cmw)))
		{
	    	    ColMoveObject( ColRedTextWid(cmw), 
			     	  x+XtWidth(ColRedSclWid(cmw))+2*mw, 
				  y+(XtHeight(ColRedTextWid(cmw))/2)+2);
                    textx = XtX(ColRedTextWid(cmw));
		}
    	    y = XtY(ColRedSclWid(cmw)) + XtHeight(ColRedSclWid(cmw)) + 2*mh;
	}

    if (IsColGrnSclWid(cmw))
	if (XtIsManaged(ColGrnSclWid(cmw))) 
	{
	    ColMoveObject(ColGrnSclWid(cmw), x, y);

    	    if (IsColGrnTextWid(cmw))
	    	if (XtIsManaged(ColGrnTextWid(cmw))) 
		{
	    	    ColMoveObject( ColGrnTextWid(cmw), 
			     	  x+XtWidth(ColGrnSclWid(cmw))+2*mw,
				  y+(XtHeight(ColGrnTextWid(cmw))/2)+2);
                    if (textx == 0)
			textx = XtX(ColGrnTextWid(cmw));
		}

	    y = XtY(ColGrnSclWid(cmw)) + XtHeight(ColGrnSclWid(cmw)) + 2*mh;
    	}

    if (IsColBluSclWid(cmw))
	if (XtIsManaged(ColBluSclWid(cmw)))
	{
	    ColMoveObject(ColBluSclWid(cmw), x, y);

    	    if (IsColBluTextWid(cmw))
	    	if (XtIsManaged(ColBluTextWid(cmw))) 
		{
	    	    ColMoveObject( ColBluTextWid(cmw), 
			     	  x+XtWidth(ColBluSclWid(cmw))+2*mw,
				  y+(XtHeight(ColBluTextWid(cmw))/2)+2);
                    if (textx == 0)
			textx = XtX(ColBluTextWid(cmw));
		}
	}

    if (IsColValLabWid(cmw))
	if (XtIsManaged(ColValLabWid(cmw)))
	    if (textx != 0)
	    	ColMoveObject(ColValLabWid(cmw), textx, XtY(ColSldLabWid(cmw)));
	    else
		XtUnmanageChild(ColValLabWid(cmw));

    (*superclass->composite_class.change_managed) ((Widget)w);
}


/*---------------------------------------------------*/
/* places colormix default RGB mixer sub-widgets in  */
/* correct positions relative to each other when     */
/* layout direction is right to left.		     */
/*---------------------------------------------------*/

static void LayoutRGBMixerRToL(cmw)
    DXmColorMixWidget cmw;
{
    int mw,mh,x,y;
    int textx = 0;
    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColRGBMixerWid(cmw);
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    mw = w->bulletin_board.margin_width;
    mh = w->bulletin_board.margin_height;
    
    x = mw;
    y = mh;

    if (IsColValLabWid(cmw))
	if (XtIsManaged(ColValLabWid(cmw)))
	    if (XtWidth(ColRedTextWid(cmw)) >= XtWidth(ColValLabWid(cmw)))
	    	ColMoveObject(ColValLabWid(cmw), 
		   x + XtWidth(ColRedTextWid(cmw)) - XtWidth(ColValLabWid(cmw)),
		   y);
	    else
	    {
	    	ColMoveObject(ColValLabWid(cmw), x, y);
		x = x + XtWidth(ColValLabWid(cmw))-XtWidth(ColRedTextWid(cmw)); 
	    }

    y = XtY(ColValLabWid(cmw)) + XtHeight(ColValLabWid(cmw)) + mh;

    y = y + ((XtHeight(ColRedTextWid(cmw))/2)+2);

    if (IsColRedTextWid(cmw))
	if (XtIsManaged(ColRedTextWid(cmw)))
	{ 
	    ColMoveObject(ColRedTextWid(cmw), x, y);

    	    if (IsColRedSclWid(cmw))
	    	if (XtIsManaged(ColRedSclWid(cmw)))
		{
	    	    ColMoveObject( ColRedSclWid(cmw), 
			     	  x+XtWidth(ColRedTextWid(cmw))+2*mw, 
				  y-(XtHeight(ColRedTextWid(cmw))/2)-2);
		}
    	    y = XtY(ColRedSclWid(cmw)) + XtHeight(ColRedSclWid(cmw)) + 2*mh;
	}

    y = y + ((XtHeight(ColGrnTextWid(cmw))/2)+2);

    if (IsColGrnTextWid(cmw))
	if (XtIsManaged(ColGrnTextWid(cmw))) 
	{
	    ColMoveObject(ColGrnTextWid(cmw), x, y);

    	    if (IsColGrnSclWid(cmw))
	    	if (XtIsManaged(ColGrnSclWid(cmw))) 
		{
	    	    ColMoveObject( ColGrnSclWid(cmw), 
			     	  x+XtWidth(ColGrnTextWid(cmw))+2*mw,
				  y-(XtHeight(ColGrnTextWid(cmw))/2)-2);
		}

	    y = XtY(ColGrnSclWid(cmw)) + XtHeight(ColGrnSclWid(cmw)) + 2*mh;
    	}

    y = y + ((XtHeight(ColBluTextWid(cmw))/2)+2);

    if (IsColBluTextWid(cmw))
	if (XtIsManaged(ColBluTextWid(cmw)))
	{
	    ColMoveObject(ColBluTextWid(cmw), x, y);

    	    if (IsColBluSclWid(cmw))
	    	if (XtIsManaged(ColBluSclWid(cmw))) 
		{
	    	    ColMoveObject( ColBluSclWid(cmw), 
			     	  x+XtWidth(ColBluTextWid(cmw))+2*mw,
				  y-(XtHeight(ColBluTextWid(cmw))/2)-2);
		}
	}

    if (IsColSldLabWid(cmw))
	if (XtIsManaged(ColSldLabWid(cmw)))
	{ 
	    ColMoveObject(ColSldLabWid(cmw), 
		XtX(ColRedSclWid(cmw)) + XtWidth(ColRedSclWid(cmw)) -
		XtWidth(ColSldLabWid(cmw)), XtY(ColValLabWid(cmw)));
	}

    (*superclass->composite_class.change_managed) ((Widget)w);
}


/*---------------------------------------------------*/
/* places colormix default greyscale mixer           */
/* sub-widgets in correct positions relative to each */
/* other.					     */
/*---------------------------------------------------*/

static void LayoutGreyscaleMixerWindow(cmw)
    DXmColorMixWidget cmw;
{
    int mw, mh;
    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColGreyscaleMixerWid(cmw);
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    mw = w->bulletin_board.margin_width;
    mh = w->bulletin_board.margin_height;

    if (IsColGreyscaleSclWid(cmw))
    { 
	if (XtIsManaged(ColGreyscaleSclWid(cmw)))
	    ColMoveObject(ColGreyscaleSclWid(cmw), mw, mh);
    }

    (*superclass->composite_class.change_managed) ((Widget)w);
}


/*---------------------------------------------------*/
/* places colormix default picker sub-widgets in     */
/* correct positions relative to each other	     */
/*---------------------------------------------------*/

static void LayoutPickerMixer(cmw)
    DXmColorMixWidget cmw;
{
    int mw, mh, x, y, total_width, pb_width, iframe_y;
    Boolean ititle_fits = TRUE;
    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColPickerMixerWid(cmw);
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    mw = w->bulletin_board.margin_width;
    mh = w->bulletin_board.margin_height;

    y = mh;

    GetPickerWidgetWidth(cmw, mw, &total_width, &pb_width);

    if (IsColPickerOptMenWid(cmw))
    { 
	if (XtIsManaged(ColPickerOptMenWid(cmw)))
	    ColMoveObject(ColPickerOptMenWid(cmw), 
		((total_width/2) - (XtWidth(ColPickerOptMenWid(cmw))/2)), y);

	y = y + XtHeight(ColPickerOptMenWid(cmw)) + mh;
    }

    if (IsColPickerFrameWid(cmw))
    { 
	if (XtIsManaged(ColPickerFrameWid(cmw)))
	    ColMoveObject(ColPickerFrameWid(cmw), 
		((total_width/2) - (XtWidth(ColPickerFrameWid(cmw))/2)), y);

	y = y + XtHeight(ColPickerFrameWid(cmw)) + (2*mh);
    }

    if ((TotalWidth(ColITitleLabWid(cmw)) +
	 TotalWidth(ColLeftBucketPbWid(cmw)) +
	 TotalWidth(ColRightBucketPbWid(cmw)) + (2*mw)) >
	 TotalWidth(ColInterpFrameWid(cmw)))
    {
	ititle_fits = FALSE;
	iframe_y = y + XtHeight (ColITitleLabWid(cmw)) + mh 
		     + XtHeight (ColLeftBucketPbWid(cmw)) + mh;
    }
    else
    {
	if (XtHeight(ColITitleLabWid(cmw)) > XtHeight(ColLeftBucketPbWid(cmw)))
	    iframe_y = y + XtHeight(ColITitleLabWid(cmw)) + mh;
	else
	    iframe_y = y + XtHeight(ColLeftBucketPbWid(cmw)) + mh;
    }

    x = ((total_width/2) - (XtWidth(ColInterpFrameWid(cmw))/2));

    if (IsColITitleLabWid(cmw))
    { 
	if (XtIsManaged(ColITitleLabWid(cmw)))
	{
	    if (ititle_fits)
	    {
		ColMoveObject(ColITitleLabWid(cmw), 
		    ((total_width/2) - (XtWidth(ColITitleLabWid(cmw))/2)),
		    iframe_y - mh - XtHeight(ColITitleLabWid(cmw)));
	    }
	    else
	    {
		ColMoveObject(ColITitleLabWid(cmw), 
		    ((total_width/2) - (XtWidth(ColITitleLabWid(cmw))/2)), y);
		y = y + XtHeight(ColITitleLabWid(cmw)) + mh;
	    }
	}
    }

    if (IsColLeftBucketPbWid(cmw))
    { 
	if (XtIsManaged(ColLeftBucketPbWid(cmw)))
	{
	    if (ititle_fits)
	    {
		ColMoveObject(ColLeftBucketPbWid(cmw), 
		    x, 		    
		    iframe_y - mh - XtHeight(ColLeftBucketPbWid(cmw)));
	    }
	    else 
	    {
		ColMoveObject(ColLeftBucketPbWid(cmw), x, y);
	    }
	}
    }

    if (IsColRightBucketPbWid(cmw))
    { 
	if (XtIsManaged(ColRightBucketPbWid(cmw)))
	{
	    if (ititle_fits)
	    {
		ColMoveObject(ColRightBucketPbWid(cmw), 
		 x + TotalWidth(ColInterpFrameWid(cmw)) - TotalWidth(ColRightBucketPbWid(cmw)),
		 iframe_y - mh - XtHeight(ColRightBucketPbWid(cmw)));
	    }
	    else 
	    {
		ColMoveObject(ColRightBucketPbWid(cmw),
		 x + TotalWidth(ColInterpFrameWid(cmw)) - TotalWidth(ColRightBucketPbWid(cmw)),
		 y);
	    }
	}
    }

    if (IsColInterpFrameWid(cmw))
    { 
	if (XtIsManaged(ColInterpFrameWid(cmw)))
	{
	    ColMoveObject(ColInterpFrameWid(cmw), x, iframe_y);
	}
    }

    y = iframe_y + (XtHeight(ColInterpFrameWid(cmw))/2);
		 
    if (IsColLighterPbWid(cmw))
    { 
	if (XtIsManaged(ColLighterPbWid(cmw)))
	    ColMoveObject(ColLighterPbWid(cmw), 
			 x - 4*mw - XtWidth(ColLighterPbWid(cmw)),
			 y - XtHeight(ColLighterPbWid(cmw)));

    }

    if (IsColLighterLabWid(cmw))
    { 
	if (XtIsManaged(ColLighterLabWid(cmw)))
	    ColMoveObject(ColLighterLabWid(cmw), 
			 x - mw - XtWidth(ColLighterLabWid(cmw)),
			 y - XtHeight(ColLighterPbWid(cmw)) 
			   - XtHeight(ColLighterLabWid(cmw)));
    }

    if (IsColDarkerPbWid(cmw))
    { 
	if (XtIsManaged(ColDarkerPbWid(cmw)))
	    ColMoveObject(ColDarkerPbWid(cmw), 
			 x - 4*mw - XtWidth(ColDarkerPbWid(cmw)),
			 y);
    }

    if (IsColDarkerLabWid(cmw))
    { 
	if (XtIsManaged(ColDarkerLabWid(cmw)))
	    ColMoveObject(ColDarkerLabWid(cmw), 
			 x - mw - XtWidth(ColDarkerLabWid(cmw)),
			 y + XtHeight(ColDarkerPbWid(cmw)));
    }

    if (IsColWarmerPbWid(cmw))
    { 
	if (XtIsManaged(ColWarmerPbWid(cmw)))
	    ColMoveObject(ColWarmerPbWid(cmw), 
			 x + XtWidth(ColInterpFrameWid(cmw)) + 4*mw,
			 y - XtHeight(ColWarmerPbWid(cmw)));
    }

    if (IsColWarmerLabWid(cmw))
    { 
	if (XtIsManaged(ColWarmerLabWid(cmw)))
	    ColMoveObject(ColWarmerLabWid(cmw), 
			 x + XtWidth(ColInterpFrameWid(cmw)) + mw,
			 y - XtHeight(ColWarmerPbWid(cmw)) 
			   - XtHeight(ColWarmerLabWid(cmw)));
    }

    if (IsColCoolerPbWid(cmw))
    { 
	if (XtIsManaged(ColCoolerPbWid(cmw)))
	    ColMoveObject(ColCoolerPbWid(cmw), 
			 x + XtWidth(ColInterpFrameWid(cmw)) + 4*mw,
			 y);
    }

    if (IsColCoolerLabWid(cmw))
    { 
	if (XtIsManaged(ColCoolerLabWid(cmw)))
	    ColMoveObject(ColCoolerLabWid(cmw), 
			 x + XtWidth(ColInterpFrameWid(cmw)) + mw,
			 y + XtHeight(ColCoolerPbWid(cmw)));

    }

    
    if (pb_width > XtWidth(ColInterpFrameWid(cmw)))
	y = y + XtHeight(ColCoolerPbWid(cmw))+XtHeight(ColCoolerLabWid(cmw))+mh;
    else
	y = y + (XtHeight(ColInterpFrameWid(cmw))/2) + (4*mh);


    x = (total_width - pb_width) / 2;  /* Calculate initial x so that the   */
    if (x < mw)			       /* row of buttons is centered within */
	x = mw;			       /* the color picking subwidget	    */

    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
	(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
    {
	if (IsColSmearPbWid(cmw))
	{ 
	    if (XtIsManaged(ColSmearPbWid(cmw)))
		ColMoveObject(ColSmearPbWid(cmw), 
		    x, y);

	    x = x + XtWidth(ColSmearPbWid(cmw)) + mw;
	}

	if (IsColUndoPbWid(cmw))
	{ 
	    if (XtIsManaged(ColUndoPbWid(cmw)))
		ColMoveObject(ColUndoPbWid(cmw), 
		    x, y);
	}
    }

    else  /* Layout right to left */
    {
	if (IsColUndoPbWid(cmw))
	{ 
	    if (XtIsManaged(ColUndoPbWid(cmw)))
		ColMoveObject(ColUndoPbWid(cmw), 
		    x, y);

	    x = x + XtWidth(ColUndoPbWid(cmw)) + mw;
	}

	if (IsColSmearPbWid(cmw))
	{ 
	    if (XtIsManaged(ColSmearPbWid(cmw)))
		ColMoveObject(ColSmearPbWid(cmw), 
		    x, y);

	    x = x + XtWidth(ColSmearPbWid(cmw)) + mw;
	}
    }

    (*superclass->composite_class.change_managed) ((Widget)w);
}


/*---------------------------------------------------*/
/* places colormix default browser sub-widgets in    */
/* correct positions relative to each other	     */
/*---------------------------------------------------*/

static void LayoutBrowserMixerWindow(cmw)
    DXmColorMixWidget cmw;
{
    int i, y=2;
    WidgetPtr pb;
    Arg al[5];

    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColBrowserBBWid(cmw);
    XmBulletinBoardWidgetClass superclass;
    XmScrolledWindowWidget sw = (XmScrolledWindowWidget) ColBrowserSWWid(cmw);
    XmScrolledWindowWidgetClass sw_class;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;
    sw_class = (XmScrolledWindowWidgetClass) xmScrolledWindowWidgetClass;

    for (i=0, pb=ColBrowserPbWid(cmw);
	 i<ColBrowserItemCount(cmw);
	 i++, pb++)
    {
	if (XtIsManaged(*pb))
	{
	    ColMoveObject(*pb, 2, y);
	    y = y + XtHeight (*pb) - 2;  /* No space between buttons */
	}
	
    }

    (*superclass->composite_class.change_managed) ((Widget)w);

    /* If the parent and grandparent of this bulletin board haven't been   */
    /* realized, then call their change_managed procs explicitly, since    */
    /* the preceding call will only change the size of the bulletin board  */
    /* and not the its parent or grandparent.  This situation is only      */
    /* encountered with non-popup colormix widgets, where the layout must  */
    /* be done at realize time rather than at map time.			   */
    
    if (!XtIsRealized (ColBrowserSWWid (cmw))) {
	(*sw_class->composite_class.change_managed) ((Widget)ColBrowserSWWid(cmw));
    }

    if (!XtIsRealized (ColBrowserMixerWid (cmw))) {
	(*superclass->composite_class.change_managed) ((Widget)ColBrowserMixerWid(cmw));
    }
}


/*---------------------------------------------------*/
/* places scratch pad subwidgets in proper positions */
/* relative to each other.			     */
/*---------------------------------------------------*/

static void LayoutScratchPad(cmw)
    DXmColorMixWidget cmw;
{
    int mw, mh, x, y, total_width, pb_width;
    XmBulletinBoardWidget w = (XmBulletinBoardWidget) ColSPPopupWid(cmw);
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;

    mw = w->bulletin_board.margin_width;
    mh = w->bulletin_board.margin_height;

    y = mh;
    total_width = mw;

    if (XtWidth(ColSPInfoLabWid(cmw)) > total_width)
	total_width = XtWidth(ColSPInfoLabWid(cmw));

    if (XtWidth(ColSPScrolledWWid(cmw)) > total_width)
	total_width = XtWidth(ColSPScrolledWWid(cmw));

    pb_width = XtWidth(ColSPClearPbWid(cmw)) + mw + XtWidth(ColSPCancelPbWid(cmw));

    if (pb_width > total_width)
	total_width = pb_width;

    if (IsColSPInfoLabWid(cmw))
    { 
	if (XtIsManaged(ColSPInfoLabWid(cmw)))
	    ColMoveObject(ColSPInfoLabWid(cmw), 
		((total_width/2) - (XtWidth(ColSPInfoLabWid(cmw))/2)), y);

	y = y + XtHeight(ColSPInfoLabWid(cmw)) + mh;
    }

    if (IsColSPBucketPbWid(cmw))
    { 
	if (XtIsManaged(ColSPBucketPbWid(cmw)))
	    ColMoveObject(ColSPBucketPbWid(cmw), 
		((total_width/2) - (XtWidth(ColSPBucketPbWid(cmw))/2)), y);

	y = y + XtHeight(ColSPBucketPbWid(cmw)) + mh;
    }

    if (IsColSPScrolledWWid(cmw))
    { 
	if (XtIsManaged(ColSPScrolledWWid(cmw)))
	    ColMoveObject(ColSPScrolledWWid(cmw), 
		((total_width/2) - (XtWidth(ColSPScrolledWWid(cmw))/2)), y);

	y = y + XtHeight(ColSPScrolledWWid(cmw)) + mh;
    }

    x = (total_width - pb_width) / 2;  /* Calculate initial x so that the   */
    if (x < mw)			       /* row of buttons is centered within */
	x = mw;			       /* scratch pad popup		    */

    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
	(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
    {
	if (IsColSPClearPbWid(cmw))
	{ 
	    if (XtIsManaged(ColSPClearPbWid(cmw)))
		ColMoveObject(ColSPClearPbWid(cmw), 
		    x, y);

	    x = x + XtWidth(ColSPClearPbWid(cmw)) + mw;
	}

	if (IsColSPCancelPbWid(cmw))
	{ 
	    if (XtIsManaged(ColSPCancelPbWid(cmw)))
		ColMoveObject(ColSPCancelPbWid(cmw), 
		    x, y);
	}
    }

    else  /* Layout right to left */
    {
	if (IsColSPCancelPbWid(cmw))
	{ 
	    if (XtIsManaged(ColSPCancelPbWid(cmw)))
		ColMoveObject(ColSPCancelPbWid(cmw), 
		    x, y);

	    x = x + XtWidth(ColSPCancelPbWid(cmw)) + mw;
	}

	if (IsColSPClearPbWid(cmw))
	{ 
	    if (XtIsManaged(ColSPClearPbWid(cmw)))
		ColMoveObject(ColSPClearPbWid(cmw), 
		    x, y);
	}
    }

    (*superclass->composite_class.change_managed) ((Widget)w);
}


/*---------------------------------------------------*/
/* calls appropriate mixer layout routine depending  */
/* on which color model is 'current' (which mixer is */
/* visible)
/*---------------------------------------------------*/

static void LayoutCurrentMixer(cmw)
    DXmColorMixWidget cmw;
{
    switch (ColModel(cmw))
    {
	case DXmColorModelHLS:
	    LayoutHLSMixer(cmw);
	    break;		

	case DXmColorModelRGB:
	    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
		(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
		LayoutRGBMixer(cmw);
	    else
		LayoutRGBMixerRToL(cmw);
	    break;		

	case DXmColorModelBrowser:
	    LayoutBrowserMixerWindow(cmw);
	    break;		

	case DXmColorModelGreyscale:
	    LayoutGreyscaleMixerWindow(cmw);
	    break;		

	default:
	    LayoutPickerMixer(cmw);
	    break;		
    }
}


/*---------------------------------------------------*/
/* places colormix sub-widgets in correct positions  */
/* relative to each other			     */
/*---------------------------------------------------*/

static void LayoutColorMixWidget(cmw)
    DXmColorMixWidget cmw;
{
    int width = 0, height = 0, pbwidth = 0;
    int mw = cmw->bulletin_board.margin_width;
    int mh = cmw->bulletin_board.margin_height;
    int num_managed_buttons = 0, width_per_button = 0, extra_space, spacing;
    int x = mw, y = mh;
    XmBulletinBoardWidgetClass superclass;

    superclass = (XmBulletinBoardWidgetClass) xmBulletinBoardWidgetClass;
    
    if (IsColDispWid(cmw) && ColDefDisp(cmw))
	if (XtIsManaged(ColDispWid(cmw)))
	    LayoutDisplay(cmw);

    if (IsColMixerWid(cmw) && ColDefMixer(cmw))
	if (XtIsManaged(ColMixerWid(cmw)))
	    LayoutCurrentMixer(cmw);

    GetColorMixSize(cmw, &width, &height, &pbwidth);

    if (IsColMainLabWid(cmw))
	if (XtIsManaged(ColMainLabWid(cmw))) 
	{
	    x = (width - (int) XtWidth(ColMainLabWid(cmw))) / 2;
	    if (x < mw)
		x = mw;

	    ColMoveObject(ColMainLabWid(cmw), x, y);
	    y = XtY(ColMainLabWid(cmw)) + XtHeight(ColMainLabWid(cmw)) + mh;
    	}

    if (IsColDispLabWid(cmw))
	if (XtIsManaged(ColDispLabWid(cmw))) 
	{
	    x = (width - (int) XtWidth(ColDispLabWid(cmw))) / 2;
	    if (x < mw)
		x = mw;

	    ColMoveObject(ColDispLabWid(cmw), x, y);
	    y = XtY(ColDispLabWid(cmw)) + XtHeight(ColDispLabWid(cmw)) + mh;
    	}

    if (IsColCurDispWid(cmw))
	if (XtIsManaged(ColCurDispWid(cmw))) 
	{
	    x = (width - (int) XtWidth(ColCurDispWid(cmw))) / 2;
	    if (x < mw)
		x = mw;

	    ColMoveObject(ColCurDispWid(cmw), x, y);
	    y = XtY(ColCurDispWid(cmw)) + XtHeight(ColCurDispWid(cmw)) + (2*mh);
	}

    x = (width - 
	(XtWidth(ColOptMenWid(cmw)) + mw/2 + XtWidth(ColOptLabWid(cmw))))/2;

    if (IsColOptMenWid(cmw) && IsColMixerWid(cmw) && ColDefMixer(cmw))
	if (XtIsManaged(ColOptMenWid(cmw)))
	{
	    if (x < mw)
		x = mw;

	    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
		(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
	    {
		ColMoveObject(ColOptLabWid(cmw), x,
			y + XtHeight(ColOptMenWid(cmw))/2 
			  - XtHeight(ColOptLabWid(cmw))/2);
		x = x + XtWidth(ColOptLabWid(cmw)) + mw/2;
		ColMoveObject(ColOptMenWid(cmw), x, y);
	    }
	    
	    else /* Layout menu from right to left */
	    {
		ColMoveObject(ColOptMenWid(cmw), x, y);
		x = x + XtWidth(ColOptMenWid(cmw)) + mw/2;
		ColMoveObject(ColOptLabWid(cmw), x,
			y + XtHeight(ColOptMenWid(cmw))/2 
			  - XtHeight(ColOptLabWid(cmw))/2);
	    }

	    y = XtY(ColOptMenWid(cmw)) + XtHeight(ColOptMenWid(cmw)) + mh;
    	}


    if (IsColMixLabWid(cmw))
	if (XtIsManaged(ColMixLabWid(cmw))) 
	{
	    x = (width - (int) XtWidth(ColMixLabWid(cmw))) / 2;
	    if (x < mw)
		x = mw;

	    ColMoveObject(ColMixLabWid(cmw), x, y);
	    y = XtY(ColMixLabWid(cmw)) + XtHeight(ColMixLabWid(cmw)) + mh;
    	}

    if (IsColCurMixerWid(cmw))
	if (XtIsManaged(ColCurMixerWid(cmw))) 
	{
	    x = (width - (int) XtWidth(ColCurMixerWid(cmw))) / 2;
	    if (x < mw)
		x = mw;

	    ColMoveObject(ColCurMixerWid(cmw), x, y);
	    y = XtY(ColCurMixerWid(cmw)) + XtHeight(ColCurMixerWid(cmw)) + (2*mh);
    	}

    if (IsColWorkWid(cmw))
	if (XtIsManaged(ColWorkWid(cmw))) 
    	{
	    x = (width - (int) XtWidth(ColWorkWid(cmw))) / 2;
	    if (x < mw)
		x = mw;

	    ColMoveObject(ColWorkWid(cmw), x, y);
	    y = XtY(ColWorkWid(cmw)) + XtHeight(ColWorkWid(cmw)) + (2*mh);
    	}


    if (IsColScratchPadPbWid(cmw))
	if (XtIsManaged(ColScratchPadPbWid(cmw))) 
    	{
    	    LayoutScratchPad(cmw);

	    x = (width - (int) XtWidth(ColScratchPadPbWid(cmw))) / 2;
	    if (x < mw)
		x = mw;

	    ColMoveObject(ColScratchPadPbWid(cmw), x, y);
	    y = XtY(ColScratchPadPbWid(cmw)) 
			+ XtHeight(ColScratchPadPbWid(cmw)) + (2*mh);
    	}

    if (IsColOkPbWid(cmw))
	if XtIsManaged(ColOkPbWid(cmw)) {
	    num_managed_buttons++; 
	    width_per_button = XtWidth (ColOkPbWid(cmw));
	}

    if (IsColAppPbWid(cmw))
	if XtIsManaged(ColAppPbWid(cmw)) { 
	    num_managed_buttons++; 
	    width_per_button = XtWidth (ColAppPbWid(cmw));
	}

    if (IsColResPbWid(cmw))
	if XtIsManaged(ColResPbWid(cmw)) {
	    num_managed_buttons++; 
	    width_per_button = XtWidth (ColResPbWid(cmw));
	}

    if (IsColCanPbWid(cmw))
	if XtIsManaged(ColCanPbWid(cmw)) {
	    num_managed_buttons++; 
	    width_per_button = XtWidth (ColCanPbWid(cmw));
	}

    if (IsColHelpPbWid(cmw))
	if XtIsManaged(ColHelpPbWid(cmw)) {
	    num_managed_buttons++; 
	    width_per_button = XtWidth (ColHelpPbWid(cmw));
	}

    /* Space the buttons evenly across the width of the colormix widget */

    extra_space = width - (num_managed_buttons * width_per_button);
    spacing = extra_space / (num_managed_buttons + 1);
    
    x = spacing;

    /*
     *  now layout the buttons
     */

    if ((ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_DOWN) ||
	(ColLayoutDirection(cmw) == DXmLAYOUT_RIGHT_UP))
    {
	if (IsColOkPbWid(cmw))
	    if (XtIsManaged(ColOkPbWid(cmw))) 
	    {
		ColMoveObject(ColOkPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }

	if (IsColAppPbWid(cmw))
	    if (XtIsManaged(ColAppPbWid(cmw))) 
	    {
		ColMoveObject(ColAppPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }

	if (IsColResPbWid(cmw))
	    if (XtIsManaged(ColResPbWid(cmw))) 
	    {
		ColMoveObject(ColResPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }

	if (IsColCanPbWid(cmw))
	    if (XtIsManaged(ColCanPbWid(cmw))) 
	    {
		ColMoveObject(ColCanPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }

	if (IsColHelpPbWid(cmw))
	    if (XtIsManaged(ColHelpPbWid(cmw))) 
		ColMoveObject(ColHelpPbWid(cmw), x, y);
    }

    else  /* Layout buttons from right to left */
    {
	if (IsColHelpPbWid(cmw))
	    if (XtIsManaged(ColHelpPbWid(cmw))) 
	    {
		ColMoveObject(ColHelpPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }
		
	if (IsColCanPbWid(cmw))
	    if (XtIsManaged(ColCanPbWid(cmw))) 
	    {
		ColMoveObject(ColCanPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }

	if (IsColResPbWid(cmw))
	    if (XtIsManaged(ColResPbWid(cmw))) 
	    {
		ColMoveObject(ColResPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }


	if (IsColAppPbWid(cmw))
	    if (XtIsManaged(ColAppPbWid(cmw))) 
	    {
		ColMoveObject(ColAppPbWid(cmw), x, y);
		x = x + width_per_button + spacing;
	    }

	if (IsColOkPbWid(cmw))
	    if (XtIsManaged(ColOkPbWid(cmw))) 
		ColMoveObject(ColOkPbWid(cmw), x, y);
    }

    (*superclass->composite_class.change_managed) ((Widget)cmw);

}


/****************************************************/
/* determines the maximum size a browser button may */
/* be by creating one, setting its label to each of */
/* the compound strings in the list of browser      */
/* color strings, and recording the maximum height  */
/* and width.					    */
/****************************************************/

static void GetBrowserButtonMaxSize (cmw, maxw, maxh)
    DXmColorMixWidget cmw;
    int *maxw;
    int *maxh;
{
    Arg al[20];
    Widget button;
    int ac=0, i;
    BrowserColor color;
 
    CreateColPB(cmw, 
	       ColBrowserMixerWid(cmw),
	       DXmGetLocaleString((I18nContext)NULL," ", I18NNOUN | I18NBUTTON ),
	       &button);

    XtSetArg(al[ac], XmNborderWidth, 0); 		ac++;
    XtSetArg(al[ac], XmNmarginHeight, 0); 		ac++;
    XtSetArg(al[ac], XmNmarginWidth, 0); 		ac++;
    XtSetArg(al[ac], XmNmarginTop, 0); 			ac++;
    XtSetArg(al[ac], XmNmarginBottom, 0); 		ac++;
    XtSetArg(al[ac], XmNmarginLeft, 0); 		ac++;
    XtSetArg(al[ac], XmNmarginRight, 0); 		ac++;
    XtSetArg(al[ac], XmNshadowThickness, 0); 		ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;

    XtSetValues (button, al, ac);

    *maxw = XtWidth(button); 
    *maxh = XtHeight(button); 

    for (i=0, color = ColBrowserColors(cmw);
	 i < ColBrowserColorCount(cmw);
	 i++, color++)
    {
	XtSetArg (al[0], XmNlabelString, color->string);
	XtSetValues (button, al, 1);

	if (XtWidth(button) > *maxw)
	    *maxw = XtWidth(button);

	if (XtHeight(button) > *maxh)
	    *maxh = XtHeight(button);
    }

    XtDestroyWidget (button);
}


/*---------------------------------------------------*/
/* obtains width of widest HLS scale label	     */
/*---------------------------------------------------*/

static void GetHLSLabelWidth(cmw, label_width)
    DXmColorMixWidget cmw;
    int *label_width;
{
    *label_width = 0;

    if (IsColHueLabWid(cmw))
   	*label_width = XtWidth(ColHueLabWid(cmw));

    if (IsColLightLabWid(cmw))
    {
	if (*label_width < XtWidth(ColLightLabWid(cmw)))
	    *label_width = XtWidth(ColLightLabWid(cmw));
    }

    if (IsColSatLabWid(cmw))
    {
	if (*label_width < XtWidth(ColSatLabWid(cmw)))
	    *label_width = XtWidth(ColSatLabWid(cmw));
    }
}


/*---------------------------------------------------*/
/* returns colormix widget optimal size		     */
/*---------------------------------------------------*/
         
static void GetColorMixSize (cmw, w, h, pbw)
    DXmColorMixWidget cmw;
    int *w;
    int *h;
    int *pbw;
{
    int twidth, mixerheight = 0, pbheight = 0;
    int mw = cmw->bulletin_board.margin_width;
    int mh = cmw->bulletin_board.margin_height;

    *w = 2*cmw->bulletin_board.margin_width;
    *h = mh;

    if (IsColMainLabWid(cmw))
	if (XtIsManaged(ColMainLabWid(cmw)))
    	{   
    	    twidth = TotalWidth(ColMainLabWid(cmw)) + (2*mw);
    	    if (twidth > *w)
            	*w = twidth;
	    *h = *h + XtHeight(ColMainLabWid(cmw)) + mh;
    	}

    if (IsColDispLabWid(cmw))
	if (XtIsManaged(ColDispLabWid(cmw)))
    	{
    	    twidth = TotalWidth(ColDispLabWid(cmw)) + (2*mw);
    	    if (twidth > *w)
            	*w = twidth;
	    *h = *h + XtHeight(ColDispLabWid(cmw)) + mh;
    	}

    if (IsColCurDispWid(cmw))
	if (XtIsManaged(ColCurDispWid(cmw)))
    	{
    	    twidth = TotalWidth(ColCurDispWid(cmw)) + (2*mw);
    	    if (twidth > *w)
            	*w = twidth;
	    *h = *h + XtHeight(ColCurDispWid(cmw)) + (2*mh);
    	}

    if (IsColOptMenWid(cmw))
	if (XtIsManaged(ColOptMenWid(cmw)))
    	{
    	    twidth = TotalWidth(ColOptMenWid(cmw)) + (2*mw);
    	    if (twidth > *w)
            	*w = twidth;
	    *h = *h + XtHeight(ColOptMenWid(cmw)) + mh;
    	}

    if (IsColMixLabWid(cmw))
	if (XtIsManaged(ColMixLabWid(cmw)))
    	{
    	    twidth = TotalWidth(ColMixLabWid(cmw)) + (2*mw);
    	    if (twidth > *w)
            	*w = twidth;
	    *h = *h + XtHeight(ColMixLabWid(cmw)) + mh;
    	}

    if (IsColCurMixerWid(cmw))
     {
	twidth = TotalWidth(ColCurMixerWid(cmw)) + (2*mw);
	if (twidth > *w)
	    *w = twidth;
	if (XtHeight(ColCurMixerWid(cmw)) > mixerheight)
	    mixerheight = XtHeight(ColCurMixerWid(cmw)); 	
    }

    if (ColDefMixer(cmw))   {
	if (IsColPickerMixerWid(cmw) && XtIsSensitive(ColPDPickerWid(cmw)))
	{
	    twidth = TotalWidth(ColPickerMixerWid(cmw)) + (2*mw);
	    if (twidth > *w)
		*w = twidth;
	    if (XtHeight(ColPickerMixerWid(cmw)) > mixerheight)
		mixerheight = XtHeight(ColPickerMixerWid(cmw)); 	
	}

	if (IsColRGBMixerWid(cmw))
	{
	    twidth = TotalWidth(ColRGBMixerWid(cmw)) + (2*mw);
	    if (twidth > *w)
		*w = twidth;
	    if (XtHeight(ColRGBMixerWid(cmw)) > mixerheight)
		mixerheight = XtHeight(ColRGBMixerWid(cmw)); 	
	}

	if (IsColHLSMixerWid(cmw))
	{
	    twidth = TotalWidth(ColHLSMixerWid(cmw)) + (2*mw);
	    if (twidth > *w)
		*w = twidth;
	    if (XtHeight(ColHLSMixerWid(cmw)) > mixerheight)
		mixerheight = XtHeight(ColHLSMixerWid(cmw)); 	
	}

	if (IsColBrowserMixerWid(cmw))
	{
	    twidth = TotalWidth(ColBrowserMixerWid(cmw)) + (2*mw);
	    if (twidth > *w)
		*w = twidth;
	    if (XtHeight(ColBrowserMixerWid(cmw)) > mixerheight)
		mixerheight = XtHeight(ColBrowserMixerWid(cmw)); 	
	}

	if (IsColGreyscaleMixerWid(cmw))
	{
	    twidth = TotalWidth(ColGreyscaleMixerWid(cmw)) + (2*mw);
	    if (twidth > *w)
		*w = twidth;
	    if (XtHeight(ColGreyscaleMixerWid(cmw)) > mixerheight)
		mixerheight = XtHeight(ColGreyscaleMixerWid(cmw)); 	
	}
    }

    *h = *h + mixerheight + (2*mw);

    if (IsColWorkWid(cmw))
	if (XtIsManaged(ColWorkWid(cmw)))
    	{
    	    twidth = TotalWidth(ColWorkWid(cmw)) + (2*mw);
    	    if (twidth > *w)
            	*w = twidth;
	    *h = *h + XtHeight(ColWorkWid(cmw)) + (2*mh);
    	}

    if (IsColScratchPadPbWid(cmw))
	if (XtIsManaged(ColScratchPadPbWid(cmw)))
    	{
    	    twidth = TotalWidth(ColScratchPadPbWid(cmw)) + (2*mw);
    	    if (twidth > *w)
            	*w = twidth;
	    *h = *h + XtHeight(ColScratchPadPbWid(cmw)) + (2*mh);
    	}

    *pbw = 2*cmw->bulletin_board.margin_width;
    
    if (IsColOkPbWid(cmw))
	if (XtIsManaged(ColOkPbWid(cmw))) {
    	    *pbw = *pbw + TotalWidth(ColOkPbWid(cmw)) + mw;
	    if (XtHeight(ColOkPbWid(cmw)) > pbheight)
		pbheight = XtHeight(ColOkPbWid(cmw));
	}

    if (IsColAppPbWid(cmw))
	if (XtIsManaged(ColAppPbWid(cmw))) {
	    *pbw = *pbw + TotalWidth(ColAppPbWid(cmw)) + mw;
	    if (XtHeight(ColAppPbWid(cmw)) > pbheight)
		pbheight = XtHeight(ColAppPbWid(cmw));
	}

    if (IsColResPbWid(cmw))
	if (XtIsManaged(ColResPbWid(cmw))) {
    	    *pbw = *pbw + TotalWidth(ColResPbWid(cmw)) + mw;
	    if (XtHeight(ColResPbWid(cmw)) > pbheight)
		pbheight = XtHeight(ColResPbWid(cmw));
	}

    if (IsColCanPbWid(cmw))
	if (XtIsManaged(ColCanPbWid(cmw))) {
    	    *pbw = *pbw + TotalWidth(ColCanPbWid(cmw)) + mw;
	    if (XtHeight(ColCanPbWid(cmw)) > pbheight)
		pbheight = XtHeight(ColCanPbWid(cmw));
	}

    if (IsColHelpPbWid(cmw))
	if (XtIsManaged(ColHelpPbWid(cmw))) {
    	    *pbw = *pbw + TotalWidth(ColHelpPbWid(cmw));
	    if (XtHeight(ColHelpPbWid(cmw)) > pbheight)
		pbheight = XtHeight(ColHelpPbWid(cmw));
	}

    *h = *h + pbheight + mh;
   
    if (*pbw > *w)
	*w = *pbw;
}



/*---------------------------------------------------*/
/* returns color picker widget optimal width	     */
/*---------------------------------------------------*/
         
static void GetPickerWidgetWidth(cmw, mw, width, pbw)
    DXmColorMixWidget cmw;
    int mw;
    int *width;
    int *pbw;
{
    int twidth;
    int interpw;	/* Width of widgets in the interpolator window's row */
    int adjustorw;
    *width = 2*mw;

    if (IsColPickerOptMenWid(cmw))
	if (XtIsManaged(ColPickerOptMenWid(cmw)))
    	{   
    	    twidth = TotalWidth(ColPickerOptMenWid(cmw)) + (2*mw);
    	    if (twidth > *width)
            	*width = twidth;
    	}


    if (IsColPickerFrameWid(cmw))
	if (XtIsManaged(ColPickerFrameWid(cmw)))
    	{
    	    twidth = TotalWidth(ColPickerFrameWid(cmw)) + (2*mw);
    	    if (twidth > *width)
            	*width = twidth;
    	}

    if (IsColITitleLabWid(cmw))
	if (XtIsManaged(ColITitleLabWid(cmw)))
    	{   
    	    twidth = TotalWidth(ColITitleLabWid(cmw)) + (2*mw);
    	    if (twidth > *width)
            	*width = twidth;
    	}


    if (IsColInterpFrameWid(cmw))
	if (XtIsManaged(ColInterpFrameWid(cmw)))
    	{
    	    interpw = TotalWidth(ColInterpFrameWid(cmw)) + (2*mw);
	    if (interpw > *width)
		*width = interpw;
    	}

    adjustorw = 2*mw;

    if (IsColWarmerPbWid(cmw) && XtIsManaged(ColWarmerPbWid(cmw)))
    {
	twidth = (4*mw) + TotalWidth(ColWarmerPbWid(cmw));
	if (twidth > adjustorw)
	    adjustorw = twidth;
    }
    else if (IsColCoolerPbWid(cmw) && XtIsManaged(ColCoolerPbWid(cmw)))
    {
    	twidth = (4*mw) + TotalWidth(ColCoolerPbWid(cmw));
	if (twidth > adjustorw)
	    adjustorw = twidth;
    }

    if (IsColLighterPbWid(cmw) && XtIsManaged(ColLighterPbWid(cmw)))
    {
	twidth = (4*mw) + TotalWidth(ColLighterPbWid(cmw));
	if (twidth > adjustorw)
	    adjustorw = twidth;
    }
    else if (IsColDarkerPbWid(cmw) && XtIsManaged(ColDarkerPbWid(cmw)))
    {
    	twidth = (4*mw) + TotalWidth(ColDarkerPbWid(cmw));
	if (twidth > adjustorw)
	    adjustorw = twidth;
    }

    if (IsColWarmerLabWid(cmw) && XtIsManaged(ColWarmerLabWid(cmw)))
    {   
	twidth = TotalWidth(ColWarmerLabWid(cmw)) + (2*mw);
	if (twidth > adjustorw)
            	adjustorw = twidth;
    }

    if (IsColCoolerLabWid(cmw) && XtIsManaged(ColCoolerLabWid(cmw)))
    {   
	twidth = TotalWidth(ColCoolerLabWid(cmw)) + (2*mw);
	if (twidth > adjustorw)
            	adjustorw = twidth;
    }


    if (IsColLighterLabWid(cmw) && XtIsManaged(ColLighterLabWid(cmw)))
    {   
	twidth = TotalWidth(ColLighterLabWid(cmw)) + (2*mw);
	if (twidth > adjustorw)
            	adjustorw = twidth;
    }

    if (IsColDarkerLabWid(cmw) && XtIsManaged(ColDarkerLabWid(cmw)))
    {   
	twidth = TotalWidth(ColDarkerLabWid(cmw)) + (2*mw);
	if (twidth > adjustorw)
            	adjustorw = twidth;
    }

    interpw = interpw + (2 * adjustorw);
    if (interpw > *width)
	*width = interpw;

    *pbw = 2*mw;

    if (IsColSmearPbWid(cmw))
	if (XtIsManaged(ColSmearPbWid(cmw)))
    	    *pbw = *pbw + TotalWidth(ColSmearPbWid(cmw)) + mw;

    if (IsColUndoPbWid(cmw))
	if (XtIsManaged(ColUndoPbWid(cmw)))
	    *pbw = *pbw + TotalWidth(ColUndoPbWid(cmw)) + mw;

    if (*pbw > *width)
	*width = *pbw;
}


/*---------------------------------------------------*/
/* sets colormix pushbutton widgets to be same width */
/*---------------------------------------------------*/

static void ColorMixSetPBWidth(cmw)
    DXmColorMixWidget cmw;
{
    Dimension largest_width = 0;
    Arg	al[1];

    if (IsColOkPbWid(cmw))
	largest_width = XtWidth(ColOkPbWid(cmw)); 

    if (IsColAppPbWid(cmw))
	if (largest_width < XtWidth(ColAppPbWid(cmw)))
	    largest_width = XtWidth(ColAppPbWid(cmw)); 

    if (IsColResPbWid(cmw))
	if (largest_width < XtWidth(ColResPbWid(cmw)))
	    largest_width = XtWidth(ColResPbWid(cmw)); 

    if (IsColCanPbWid(cmw))
	if (largest_width < XtWidth(ColCanPbWid(cmw)))
	    largest_width = XtWidth(ColCanPbWid(cmw)); 

    if (IsColHelpPbWid(cmw))
	if (largest_width < XtWidth(ColHelpPbWid(cmw)))
	    largest_width = XtWidth(ColHelpPbWid(cmw)); 

    if (largest_width > 0)
    {
    	XtSetArg (al[0], XmNwidth, largest_width);

	if (IsColOkPbWid(cmw) && largest_width != XtWidth(ColOkPbWid(cmw)))
    	    XtSetValues (ColOkPbWid(cmw), al, 1);

	if (IsColAppPbWid(cmw) && largest_width != XtWidth(ColAppPbWid(cmw)))
	    XtSetValues (ColAppPbWid(cmw), al, 1);

	if (IsColResPbWid(cmw) && largest_width != XtWidth(ColResPbWid(cmw)))
	    XtSetValues (ColResPbWid(cmw), al, 1);

	if (IsColCanPbWid(cmw) && largest_width != XtWidth(ColCanPbWid(cmw)))
    	    XtSetValues (ColCanPbWid(cmw), al, 1);

	if (IsColHelpPbWid(cmw) && largest_width != XtWidth(ColHelpPbWid(cmw)))
    	    XtSetValues (ColHelpPbWid(cmw), al, 1);
    }
}


/*---------------------------------------------------*/
/* Figures out the optimal width and height for the  */
/* color mix widget at map time and sets them        */
/*---------------------------------------------------*/
static void SetColorMixDimensions (cmw)
    DXmColorMixWidget cmw;
{
    Arg al[5];
    int w=0, h=0, pbwidth, ac=0;
    Dimension height_clipped=0, width_clipped=0;

    LayoutColorMixWidget(cmw);

    /* Make sure all mixers are layed out */

    if (ColModel(cmw) == DXmColorModelPicker)
	ColPickerMixerWid(cmw)->core.managed = FALSE;	

    LayoutPickerMixer(cmw);

    if (ColModel(cmw) == DXmColorModelPicker)
	ColPickerMixerWid(cmw)->core.managed = TRUE;	

    if (ColModel(cmw) == DXmColorModelRGB)
	ColRGBMixerWid(cmw)->core.managed = FALSE;

    LayoutRGBMixer(cmw);

    if (ColModel(cmw) == DXmColorModelRGB)
	ColRGBMixerWid(cmw)->core.managed = TRUE;

    if (ColModel(cmw) == DXmColorModelHLS)
	ColHLSMixerWid(cmw)->core.managed = FALSE;

    LayoutHLSMixer(cmw);

    if (ColModel(cmw) == DXmColorModelHLS)
	ColHLSMixerWid(cmw)->core.managed = TRUE;

    if (ColModel(cmw) == DXmColorModelBrowser)
	ColBrowserBBWid(cmw)->core.managed = FALSE;

    LayoutBrowserMixerWindow(cmw);

    if (ColModel(cmw) == DXmColorModelBrowser)
	ColBrowserBBWid(cmw)->core.managed = TRUE;

    if (ColModel(cmw) == DXmColorModelGreyscale)
	ColGreyscaleMixerWid(cmw)->core.managed = FALSE;

    LayoutGreyscaleMixerWindow(cmw);

    if (ColModel(cmw) == DXmColorModelGreyscale)
	ColGreyscaleMixerWid(cmw)->core.managed = TRUE;

    /* Get optimal size */

    GetColorMixSize (cmw, &w, &h, &pbwidth);

    /* Update the size of the colormix if necessary */

    XtSetArg(al[ac], XmNresizePolicy, XmRESIZE_NONE); ac++;
	
    if (w != XtWidth(cmw))
    {
	XtSetArg (al[ac], XmNwidth, w); ac++;
    }

    if (h != XtHeight (cmw))
    {
	XtSetArg (al[ac], XmNheight, h); ac++;
    }

    /*
     * Set core.managed to FALSE so that a sick window manager can't
     * thwart our attempts to grow to the correct size.  This is the 
     * same hack currently used in the Xm/DialogS.c module. 
     */    

    cmw->core.managed = FALSE;
    XtSetValues ((Widget)cmw, al, ac);
    cmw->core.managed = TRUE;

    if (!_DXmCheckFit(cmw, &height_clipped, &width_clipped))
	ColorMixShrink(cmw, height_clipped, width_clipped);
}           

   
/*---------------------------------------------------*/
/* this is a patch routine until automatic shell     */
/* scroll bars become available... 		     */
/*---------------------------------------------------*/

static void ColorMixShrink(cmw, height_clipped, width_clipped)
    DXmColorMixWidget	  cmw;
    Dimension 		  height_clipped;
    Dimension		  width_clipped;
{
    Arg al[11];
    int ac = 0;

    if (height_clipped > 0 || width_clipped > 0)
    {
    	XtSetArg (al[ac], XmNmarginHeight, 5); 			ac++;
    	XtSetArg (al[ac], XmNmarginWidth, 5); 			ac++;
    	XtSetArg (al[ac], DXmNdisplayColWinHeight, 40); 	ac++;
    	XtSetArg (al[ac], DXmNdisplayColWinWidth, 40); 		ac++;
    	XtSetArg (al[ac], DXmNdispWinMargin, 10); 		ac++;
    	XtSetArg (al[ac], DXmNpickerTileHeight, 15); 		ac++;
    	XtSetArg (al[ac], DXmNpickerTileWidth, 15); 		ac++;
    	XtSetArg (al[ac], DXmNinterpTileHeight, 15); 		ac++;
    	XtSetArg (al[ac], DXmNinterpTileWidth, 15); 		ac++;
    	XtSetArg (al[ac], XmNresizePolicy, XmRESIZE_ANY); 	ac++;
    	XtSetValues ((Widget)cmw, al, ac);
    }
}

   
/*---------------------------------------------------*/
/* these are the colormix subwidget action routines  */
/* called by the translation manager.  Their purpose */
/* is to support keyboard traversal.		     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/* called by the translation manager when focus moves*/
/* to default colormix color display widget.  Unhigh-*/
/* light current color and highlight display widget  */
/* with focus.  				     */
/*---------------------------------------------------*/

static void ColorMixFocusInDisplay(w, event)
    XmDrawnButtonWidget	 w;
    XEvent      *event;
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);

    DisplaySelect(cmw, w);
}


/*---------------------------------------------------*/
/* called by the translation manager when focus out  */
/* out of the display widget.  Unhighlight display   */
/* widget loosing focus.			     */
/*---------------------------------------------------*/

static void ColorMixFocusOutDisplay(w, event)
    Widget	 w;
    XEvent      *event;
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb left    */
/* in the picker widget.  Move current selected color*/
/* "left" one tile.				     */
/*---------------------------------------------------*/

static void ColorMixLeftPicker(w, event)
    Widget	 w;
    XEvent      *event;
{
    int i;
    XColor *xc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);

    if (ColPickerSelectTile(cmw) == 0)
	ColPickerSelectTile(cmw) = ColPickerColorCount(cmw)-1;
    else
	ColPickerSelectTile(cmw) = ColPickerSelectTile(cmw)-1;

    for (i=0, xc = ColPickerXColors(cmw); i<ColPickerSelectTile(cmw); i++, xc++)
	;	/* Find color record for selected tile */

    ColIsPickerSelected(cmw) = TRUE;
    ColHighlightedColor(cmw) = xc;

    DrawPickerTileBorder (cmw, ColPickerSelectTile(cmw), 4, NULL);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb right   */
/* in the picker widget.  Move current selected color*/
/* "right" one tile.				     */
/*---------------------------------------------------*/

static void ColorMixRightPicker(w, event)
    Widget	 w;
    XEvent      *event;
{
    int i;
    XColor *xc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);

    if (ColPickerSelectTile(cmw) == (ColPickerColorCount(cmw)-1))
	ColPickerSelectTile(cmw) = 0;
    else
	ColPickerSelectTile(cmw) = ColPickerSelectTile(cmw)+1;


    for (i=0, xc = ColPickerXColors(cmw); i<ColPickerSelectTile(cmw); i++, xc++)
	;	/* Find color record for selected tile */

    ColIsPickerSelected(cmw) = TRUE;
    ColHighlightedColor(cmw) = xc;

    DrawPickerTileBorder (cmw, ColPickerSelectTile(cmw), 4, NULL);
}


/*---------------------------------------------------*/
/* called by the translation manager when focus moves*/
/* to picker widget.  Unhighlight current color and  */
/* highlight last selected picker tile. 	     */
/*---------------------------------------------------*/

static void ColorMixFocusInPicker(w, event)
    Widget	 w;
    XEvent      *event;
{
    int i;
    XColor *xc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);

    for (i=0, xc = ColPickerXColors(cmw); i<ColPickerSelectTile(cmw); i++, xc++)
	;	/* Find color record for selected tile */

    ColIsPickerSelected(cmw) = TRUE;
    ColHighlightedColor(cmw) = xc;

    DrawPickerTileBorder (cmw, ColPickerSelectTile(cmw), 4, NULL);
}


/*---------------------------------------------------*/
/* called by the translation manager when focus moves*/
/* out of the picker widget.  Unhighlight last       */
/* selected picker tile. 	     		     */
/*---------------------------------------------------*/

static void ColorMixFocusOutPicker(w, event)
    Widget	 w;
    XEvent      *event;
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb left    */
/* in the interpolator widget.  Move current selected*/
/* color "left" one tile.			     */
/*---------------------------------------------------*/

static void ColorMixLeftInterp(w, event)
    Widget	 w;
    XEvent      *event;
{
    int i;
    XColor *xc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);

    if (ColInterpSelectTile(cmw) == 0)
	ColInterpSelectTile(cmw) = ColInterpTileCount(cmw)-1;
    else
	ColInterpSelectTile(cmw) = ColInterpSelectTile(cmw)-1;


    for (i=0, xc = ColInterpXColors(cmw); i<ColInterpSelectTile(cmw); i++, xc++)
	;	/* Find color record for selected tile */

    ColIsInterpSelected(cmw) = TRUE;
    ColHighlightedColor(cmw) = xc;

    DrawInterpTileBorder (cmw, ColInterpSelectTile(cmw), 4, NULL);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb right   */
/* in the interpolator widget.  Move current selected*/
/* color "right" one tile.			     */
/*---------------------------------------------------*/

static void ColorMixRightInterp(w, event)
    Widget	 w;
    XEvent      *event;
{
    int i;
    XColor *xc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);

    if (ColInterpSelectTile(cmw) == (ColInterpTileCount(cmw)-1))
	ColInterpSelectTile(cmw) = 0;
    else
	ColInterpSelectTile(cmw) = ColInterpSelectTile(cmw)+1;


    for (i=0, xc = ColInterpXColors(cmw); i<ColInterpSelectTile(cmw); i++, xc++)
	;	/* Find color record for selected tile */

    ColIsInterpSelected(cmw) = TRUE;
    ColHighlightedColor(cmw) = xc;

    DrawInterpTileBorder (cmw, ColInterpSelectTile(cmw), 4, NULL);
}


/*---------------------------------------------------*/
/* called by the translation manager when focus moves*/
/* to the interpolator widget.  Highlight last       */
/* selected picker tile. 	     		     */
/*---------------------------------------------------*/

static void ColorMixFocusInInterp(w, event)
    Widget	 w;
    XEvent      *event;
{
    int i;
    XColor *xc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);

    for (i=0, xc = ColInterpXColors(cmw); i<ColInterpSelectTile(cmw); i++, xc++)
	;	/* Find color record for selected tile */

    ColIsInterpSelected(cmw) = TRUE;
    ColHighlightedColor(cmw) = xc;

    DrawInterpTileBorder (cmw, ColInterpSelectTile(cmw), 4, NULL);

}


/*---------------------------------------------------*/
/* called by the translation manager when focus moves*/
/* out of the interpolator widget.  Unhighlight last */
/* selected interplator tile. 	     		     */
/*---------------------------------------------------*/

static void ColorMixFocusOutInterp(w, event)
    Widget	 w;
    XEvent      *event;
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    UnhighlightCurrentColor(cmw);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb up in   */
/* scratch pad list box.  Scroll "down" one color.   */
/*---------------------------------------------------*/

static void ColorMixUpSP (w, event)
    Widget	w;
    XEvent      *event;
{   
    int i, value, slider, incr, page;
    XColor *xc, *current;

    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));

    XmScrollBarGetValues(ColSPScrollBarWid(cmw), &value, &slider, &incr, &page);

    if (value == (ColScratchCount(cmw)-1))
	return;
    else
	value = value + 1;

    XmScrollBarSetValues(ColSPScrollBarWid(cmw), value, slider, 0, 0, TRUE);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb down in */
/* scratch pad list box.  Scroll "up" one color.     */
/*---------------------------------------------------*/

static void ColorMixDownSP (w, event)
    Widget	w;
    XEvent      *event;
{   
    int i, value, slider, incr, page;
    XColor *xc, *current;

    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));

    XmScrollBarGetValues(ColSPScrollBarWid(cmw), &value, &slider, &incr, &page);

    if (value == 0)
	return;
    else
	value = value - 1;

    XmScrollBarSetValues(ColSPScrollBarWid(cmw), value, slider, 0, 0, TRUE);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb top in  */
/* scratch pad list box.  Scroll to "first" color.   */
/*---------------------------------------------------*/

static void ColorMixTopSP (w, event)
    Widget	w;
    XEvent      *event;
{   
    int i, value, slider, incr, page;
    XColor *xc, *current;

    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));

    XmScrollBarGetValues(ColSPScrollBarWid(cmw), &value, &slider, &incr, &page);

    if (value == 0)
	return;
    else
	value = 0;

    XmScrollBarSetValues(ColSPScrollBarWid(cmw), value, slider, 0, 0, TRUE);
}


/*---------------------------------------------------*/
/* called by the translation manager upon kb bottom in*/
/* scratch pad list box.  Scroll to "last" color.    */
/*---------------------------------------------------*/

static void ColorMixBottomSP (w, event)
    Widget	w;
    XEvent      *event;
{   
    int i, value, slider, incr, page;
    XColor *xc, *current;

    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));

    XmScrollBarGetValues(ColSPScrollBarWid(cmw), &value, &slider, &incr, &page);

    if (value == (ColScratchCount(cmw)-1))
	return;
    else
	value = (ColScratchCount(cmw)-1);

    XmScrollBarSetValues(ColSPScrollBarWid(cmw), value, slider, 0, 0, TRUE);
}


/*---------------------------------------------------*/
/* called by the translation manager when focus moves*/
/* to the scratch pad list box.  Highlight last      */
/* selected color.	 	     		     */
/*---------------------------------------------------*/

static void ColorMixFocusInSP(w, event)
    XmDrawnButtonWidget	 w;
    XEvent      *event;
{
    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));

    UnhighlightCurrentColor(cmw);

    ScratchPadSelect(cmw);
}


/*---------------------------------------------------*/
/* called by the translation manager when focus moves*/
/* out of the scratch pad list box.  Unhighlight last*/
/* selected color.	 	     		     */
/*---------------------------------------------------*/

static void ColorMixFocusOutSP(w, event)
    Widget	 w;
    XEvent      *event;
{
    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));

    UnhighlightCurrentColor(cmw);
}


/*---------------------------------------------------*/
/* sets the sensitivity of the picker option in the  */
/* colormix option menu.  This option is set to      */
/* insensitive when the color mix widget is unable   */
/* to allocate sufficient color resources for the    */
/* picker tiles.  If this happens, and the picker is */
/* the current color model, the color model is	     */
/* switched to RGB.				     */
/*---------------------------------------------------*/

static void SetPickerOptionSensitivity(cmw, is_sensitive)
    DXmColorMixWidget cmw;
    Boolean is_sensitive;
{
    Arg	al[1];
    int ac = 0;

    if (IsColPDPickerWid(cmw))
    {
	XtSetArg (al[ac], XmNsensitive, is_sensitive);  ac++;
	XtSetValues (ColPDPickerWid(cmw), al, ac);
    }

    /* Switch to RGB if we failed to allocate enough colors */

    if ((!is_sensitive) && (ColModel(cmw) == DXmColorModelPicker))
    {
	ColModel(cmw) = DXmColorModelRGB;
	UpdateMixer(cmw);
	UpdateMenu(cmw);
    }
}


/*---------------------------------------------------*/
/* misc work routines	 			     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/* this routine updates the appropriate text widget  */
/* when one of the default RGB mixer's scale widgets */
/* are used to change either the Red, Green or Blue  */
/* color value  				     */
/*---------------------------------------------------*/

static void UpdateRGBText(cmw, color_value, mask)
    DXmColorMixWidget  	cmw;
    unsigned short  	color_value;
    unsigned char   	mask;
{   
    Widget 	w;
    XmString 	temp_cs;
    long	size,status;

    temp_cs = (XmString) _DXmCvtItoCS(color_value, &size, &status);

    if (mask == RedChanged)
	w = ColRedTextWid(cmw);
    else if (mask == GrnChanged)
	w = ColGrnTextWid(cmw);
    else if (mask == BluChanged)
	w = ColBluTextWid(cmw);

    DXmCSTextSetString(w, temp_cs);

    XmStringFree(temp_cs);
}

   
/*---------------------------------------------------*/
/* conversion routines.  There is round-off error;   */
/* however, it should be negligible.		     */
/*---------------------------------------------------*/

static int ColCvtColorToScaleValue(value)
    int value;
{
    return (int) ((value * 100) / MAXCOLORVALUE);
}

static unsigned short CvtScaleToColorValue(value)
    int value;
{
    return (int) ((value * MAXCOLORVALUE) / 100);
}


   
/*---------------------------------------------------*/
/* sets default HLS color mixing tool to specified   */
/* color value	     		     		     */
/*---------------------------------------------------*/

static void SetHLSMixerNewColor(cmw,red,green,blue)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;
{   
    ColorMixRGBToHLS((double)red, 
		     (double)green, 
		     (double)blue,
	     	     &ColNewColorHue(cmw), 
		     &ColNewColorLight(cmw), 
		     &ColNewColorSat(cmw));

    XmScaleSetValue(ColHueSclWid(cmw),   (int) (ColNewColorHue(cmw)));

    XmScaleSetValue(ColLightSclWid(cmw), (int) (ColNewColorLight(cmw)*100));

    XmScaleSetValue(ColSatSclWid(cmw),   (int) (ColNewColorSat(cmw)*100));
}

   
/*---------------------------------------------------*/
/* sets default RGB color mixing tool to specified   */
/* color value			     		     */
/*---------------------------------------------------*/

static void SetRGBMixerNewColor(cmw,red,green,blue)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;
{   
    XmString 	temp_cs;
    long	size,status;

    XmScaleSetValue(ColRedSclWid(cmw), ColCvtColorToScaleValue(red));
    XmScaleSetValue(ColGrnSclWid(cmw), ColCvtColorToScaleValue(green));
    XmScaleSetValue(ColBluSclWid(cmw), ColCvtColorToScaleValue(blue));

    temp_cs = (XmString) _DXmCvtItoCS(red, &size, &status);
    DXmCSTextSetString(ColRedTextWid(cmw), temp_cs);
    XmStringFree(temp_cs);

    temp_cs = (XmString) _DXmCvtItoCS(green, &size, &status);
    DXmCSTextSetString(ColGrnTextWid(cmw), temp_cs);
    XmStringFree(temp_cs);

    temp_cs = (XmString) _DXmCvtItoCS(blue, &size, &status);
    DXmCSTextSetString(ColBluTextWid(cmw), temp_cs);
    XmStringFree(temp_cs);
}

   
/*---------------------------------------------------*/
/* sets default color mixing tool color value -      */
/* whenever the 'new' color changes w/o using the    */
/* mixer tool(s) themselves			     */
/*---------------------------------------------------*/

static void SetMixerNewColor(cmw,red,green,blue)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;
{   
    double grey;

    ColNewColorRed(cmw) = red;
    ColNewColorGrn(cmw) = green;
    ColNewColorBlu(cmw) = blue;

    if (ColDefMixer(cmw))
    {
	if (ColModel(cmw) == DXmColorModelHLS)
	    SetHLSMixerNewColor(cmw, 
				ColNewColorRed(cmw), 
				ColNewColorGrn(cmw), 
				ColNewColorBlu(cmw));

	if (ColModel(cmw) == DXmColorModelRGB)
	    SetRGBMixerNewColor(cmw, 
				ColNewColorRed(cmw), 
				ColNewColorGrn(cmw), 
				ColNewColorBlu(cmw));

    }
}

   
/*---------------------------------------------------*/
/* this routine changes the default RGB mixer's 'orig'*/
/* color window	background			     */
/*---------------------------------------------------*/

static void SetOrigColor(cmw, red, green, blue)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;
{   
    Arg al[1];

    ColOrigColorRed(cmw) = red;
    ColOrigColorGrn(cmw) = green;
    ColOrigColorBlu(cmw) = blue;

    if (!ColDefDisp(cmw))
	return;

    if (XtIsRealized(cmw) && XtIsManaged(ColDispWid(cmw)))
    {
        ColOrigColorFlg(cmw) = DoRed | DoGreen | DoBlue;

    	if (ColDispType(cmw) == DynColor)
    	{
	    if (ColAllocOrigColor(cmw))
	    	XStoreColor(XtDisplay(cmw),
		    	    cmw->core.colormap,
		    	    &(ColOrigColor(cmw)));
    	}
    	else if (ColDispType(cmw) == StatColor)
    	    ColAllocOrigColor(cmw) = AllocStaticColorCell(cmw,&(ColOrigColor(cmw)));
        else if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw,&ColOrigColor(cmw));

    	if (ColAllocOrigColor(cmw))
    	{
    	    XtSetArg(al[0], XmNbackground, ColOrigColorPix(cmw));
    	    XtSetValues(ColOrigWid(cmw), al, 1);
    	}
    }
}

   
/*---------------------------------------------------*/
/* 'generic' routine to set a gray color	     */
/*---------------------------------------------------*/

static void SetAGrayColor(cmw, col, alloced)
    DXmColorMixWidget cmw;
    XColor	   *col;
    Boolean	   alloced;
{   
    XColor temp;
    int ave;

    if (XtIsRealized(cmw) && alloced)
    {
    	ave = col->red + col->green + col->blue;
    	ave = ave/3;

	temp.red = temp.green = temp.blue = (unsigned short) ave;
	temp.flags = DoRed | DoGreen | DoBlue;
        temp.pixel = col->pixel;
        temp.pad = col->pad;

	XStoreColor(XtDisplay(cmw),
		    cmw->core.colormap,
		    &temp);

	col->pixel = temp.pixel;
    }
}

   
/*---------------------------------------------------*/
/* 'generic' routine to set a color		     */
/*---------------------------------------------------*/

static void SetAColor(cmw, col, alloced)
    DXmColorMixWidget cmw;
    XColor	   *col;
    Boolean	   *alloced;
{   
    if (XtIsRealized(cmw))
    {
        col->flags = DoRed | DoGreen | DoBlue;

    	if (ColDispType(cmw) == DynColor)
    	{
	    if (*alloced)
	    	XStoreColor(XtDisplay(cmw),
		    	    cmw->core.colormap,
		    	    col);
    	}
    	else if (ColDispType(cmw) == StatColor)
    	    *alloced = (Boolean) AllocStaticColorCell(cmw,col);
    }
}

   
/*---------------------------------------------------*/
/* this routine sets the colors the default color    */
/* display widget		     		     */
/*---------------------------------------------------*/

static void SetUpColors(cmw)
    DXmColorMixWidget cmw;
{   
    Arg al[2];
    XColor *xcolors, *ucolors;
    int i;
    WidgetPtr pb;
    Boolean picker_alloced = TRUE, interp_alloced = TRUE;
    Boolean browser_alloced = TRUE;
    BrowserColor colors;
    Screen  *screen = (Screen *) XtScreen(cmw);

    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw,&(ColBackColor(cmw)),ColAllocBackColor(cmw));
    else
	SetAColor(cmw,&(ColBackColor(cmw)),&(ColAllocBackColor(cmw)));

  
    if (ColAllocBackColor(cmw))
    {
	XtSetArg(al[0], XmNbackground, ColBackColorPix(cmw));
	XtSetValues(ColDispWid(cmw), al, 1);
    }


    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw,&(ColOrigColor(cmw)),ColAllocOrigColor(cmw));
    else
	SetAColor(cmw,&(ColOrigColor(cmw)),&(ColAllocOrigColor(cmw)));

    if (ColAllocOrigColor(cmw))
    {
	XtSetArg(al[0], XmNbackground, ColOrigColorPix(cmw));
	XtSetValues(ColOrigWid(cmw), al, 1);
    }


    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw,&(ColNewColor(cmw)),ColAllocNewColor(cmw));
    else
	SetAColor(cmw,&(ColNewColor(cmw)),&(ColAllocNewColor(cmw)));

    if (ColAllocNewColor(cmw))
    {
	XtSetArg(al[0], XmNbackground, ColNewColorPix(cmw));
	XtSetValues(ColNewWid(cmw), al, 1);
    }


    for (i=0, xcolors = ColPickerXColors(cmw); 
	 i < ColPickerColorCount(cmw); 
	 i++, xcolors++)
    {
        if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xcolors, ColAllocPickerXColors(cmw));
	else
	{    
	    SetAColor(cmw, xcolors, &(ColAllocPickerXColors(cmw)));
	    picker_alloced = (picker_alloced && ColAllocPickerXColors(cmw)); 
	}
    }

    ColAllocPickerXColors(cmw) = picker_alloced;

    for (i=0, xcolors = ColInterpXColors(cmw), ucolors = ColUndoXColors(cmw); 
	 i < ColInterpTileCount(cmw); 
	 i++, xcolors++, ucolors++)
    {
	if (ColIsInterpSensitive(cmw))
	{
	    xcolors->red   = ucolors->red   = (unsigned short) MAXCOLORVALUE; /* Initialize */
	    xcolors->green = ucolors->green = (unsigned short) MAXCOLORVALUE; /*    to      */
	    xcolors->blue  = ucolors->blue  = (unsigned short) MAXCOLORVALUE; /*   white    */	

	    if (ColDispType(cmw) == GryScale)
		SetAGrayColor(cmw, xcolors, ColAllocInterpXColors(cmw));
	    else
	    {    
		SetAColor(cmw, xcolors, &(ColAllocInterpXColors(cmw)));
		interp_alloced = (interp_alloced && ColAllocInterpXColors(cmw)); 
	    }
	}

	ucolors->pixel = xcolors->pixel;
    }

    if (ColIsInterpSensitive(cmw))
	ColAllocInterpXColors(cmw) = interp_alloced;

    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw,&(ColCurrentSPColor(cmw)), ColAllocScratchColor(cmw));
    else
	SetAColor(cmw,&(ColCurrentSPColor(cmw)),&(ColAllocScratchColor(cmw)));

    if (ColAllocScratchColor(cmw))
    {
	XtSetArg(al[0], XmNbackground, ColCurrentSPPix(cmw));
	XtSetValues(ColSPDrawingAreaWid(cmw), al, 1);
    }

    for (i=0, xcolors = ColBrowserXColors(cmw), colors = ColBrowserColors(cmw),
	 pb = ColBrowserPbWid(cmw); 
	 i < ColBrowserItemCount(cmw); 
	 i++, xcolors++, colors++, pb++)
    {
	xcolors->red   = colors->red;
	xcolors->green = colors->green;
	xcolors->blue  = colors->blue;

        if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xcolors, ColAllocBrowserXColors(cmw));
	else
	{    
	    SetAColor(cmw, xcolors, &(ColAllocBrowserXColors(cmw)));
	    browser_alloced = (browser_alloced && ColAllocBrowserXColors(cmw)); 
	}

	if (ColAllocBrowserXColors(cmw))
	{
	    if (colors->dark_fg)
		XtSetArg(al[0], XmNforeground, BlackPixelOfScreen(screen));
	    else
		XtSetArg(al[0], XmNforeground, WhitePixelOfScreen(screen));
	
	    XtSetArg(al[1], XmNbackground, xcolors->pixel);
	    XtSetValues(*pb, al, 2);
	}

    }

    XmScrollBarSetValues (ColBrowserSBWid(cmw), 0, ColBrowserItemCount(cmw),
			  0, 0, TRUE);

    SetPickerOptionSensitivity (cmw, picker_alloced);
}

   
/*---------------------------------------------------*/
/* sets the 'new' color tile in the default color    */
/* display window				     */
/*---------------------------------------------------*/

static void SetDisplayNewColor(cmw, red, green, blue)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;
{   
    Arg al[1];

    if (ColModel(cmw) != DXmColorModelBrowser)
	ColNamedColor(cmw) = NULL;

    ColNewColorRed(cmw) = red;
    ColNewColorGrn(cmw) = green;
    ColNewColorBlu(cmw) = blue;

    if (!ColDefDisp(cmw))
	return;

    if (XtIsRealized(cmw) && XtIsManaged(ColDispWid(cmw)))
    {
        ColNewColorFlg(cmw) = DoRed | DoGreen | DoBlue;

    	if (ColDispType(cmw) == DynColor)
    	{
	    if (ColAllocNewColor(cmw))
	    	XStoreColor(XtDisplay(cmw),
		    	    cmw->core.colormap,
		    	    &(ColNewColor(cmw)));
    	}
    	else if (ColDispType(cmw) == StatColor)
    	    ColAllocNewColor(cmw) = AllocStaticColorCell(cmw,&(ColNewColor(cmw)));
        else if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw,&ColNewColor(cmw));

    	if (ColAllocNewColor(cmw))
    	{
    	    XtSetArg(al[0], XmNbackground, ColNewColorPix(cmw));
    	    XtSetValues(ColNewWid(cmw), al, 1);
    	}
    }
}

                           
/*---------------------------------------------------*/
/* allocates read or 'static' color cell	     */
/*---------------------------------------------------*/
 
static Boolean AllocStaticColorCell(cmw, color)
    DXmColorMixWidget cmw;
    XColor *color;
{
    int status;

    status = XAllocColor( XtDisplay(cmw),
			  cmw->core.colormap,
			  color);
    if (!status)
    {
	DXMWARNING("DXmColorMixWidget",COLORALLOCERR);
        return (Boolean) (FALSE);
    }

    return (Boolean) (TRUE);
}
 
                           
/*---------------------------------------------------*/
/* allocates read/write or 'dynamic' color cell	     */
/*---------------------------------------------------*/
 
static Boolean AllocDynColorCells(cmw,pix,num_pixels)
    DXmColorMixWidget cmw;
    unsigned long *pix;
    int num_pixels;
{
    int status;

    status = XAllocColorCells( XtDisplay(cmw),
			       cmw->core.colormap,
			       0, 0, 0, pix, num_pixels);
    if (!status)
    {
	DXMWARNING("DXmColorMixWidget", COLORALLOCERR);
        return (Boolean) (FALSE);
    }

    return (Boolean) (TRUE);
}
 
                           
/*---------------------------------------------------*/
/* allocates color cells required by default color   */
/* display widget when running on dynamic color screen*/
/*---------------------------------------------------*/
 
static void AllocDynColors(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[5];
    int i, alloced=0;
    unsigned long onepix, *pixels, *pix;
    Widget *pb;
    XColor *xcolors;
    Screen  *screen = (Screen *) XtScreen(cmw);

    /* 
     * first allocate cells to display new and original colors 
     * against background color
     */

    if (!ColAllocNewColor(cmw))
        ColAllocNewColor(cmw)  = (Boolean) AllocDynColorCells(cmw, 
						    &ColNewColorPix(cmw), 1);
    if (!ColAllocOrigColor(cmw))
    	ColAllocOrigColor(cmw) = (Boolean) AllocDynColorCells(cmw, 
						    &ColOrigColorPix(cmw), 1);
    if (!ColAllocBackColor(cmw))
    	ColAllocBackColor(cmw) = (Boolean) AllocDynColorCells(cmw, 
						    &ColBackColorPix(cmw), 1);


    /* 
     * next allocate scratch pad color (since it can be used in 
     * any color model)
     */

    if (!ColAllocScratchColor(cmw))
    {
    	ColAllocScratchColor(cmw) = (Boolean) AllocDynColorCells(cmw, 
						   &ColCurrentSPPix(cmw), 1);
	if (ColAllocScratchColor(cmw))
	    XtSetArg (al[0], XmNsensitive, TRUE);
	else
	    XtSetArg (al[0], XmNsensitive, FALSE);

	XtSetValues (ColScratchPadPbWid(cmw), al, 1);
    }


    /* allocate cells for picker model static colors */

    if (!ColAllocPickerXColors(cmw))
    {
	pixels = (unsigned long *) XtMalloc (sizeof(unsigned long) *
					     ColPickerColorCount(cmw));

	ColAllocPickerXColors(cmw) = (Boolean) AllocDynColorCells(cmw,
						    pixels,
						    ColPickerColorCount(cmw));
	
	if (ColAllocPickerXColors(cmw))
	{
	    for (i=0, pix = pixels, xcolors = ColPickerXColors(cmw);
		 i < ColPickerColorCount (cmw);
		 i++, pix++, xcolors++)
		    xcolors->pixel = (unsigned long) *pix;
	    SetPickerOptionSensitivity (cmw, TRUE);
	}
	else
	    SetPickerOptionSensitivity(cmw, FALSE);
	
	XtFree ((char *)pixels);
    }


    /* allocate cells for picker interpolator colors */

    if (!ColAllocInterpXColors(cmw))
    {
	if (ColInterpTileCount(cmw) != ColRealInterpTileCount(cmw))
	    ColInterpTileCount(cmw) = ColRealInterpTileCount(cmw);

	for (i=0, xcolors = ColInterpXColors(cmw); 
	     i<ColInterpTileCount(cmw); 
	     i++, xcolors++)
	{
	    ColAllocInterpXColors(cmw) = 
		(Boolean) AllocDynColorCells(cmw, &onepix, 1);
	
	    if (ColAllocInterpXColors(cmw))
	    {	 
		xcolors->pixel = onepix;
		alloced++;
	    }
	    else	
		xcolors->pixel = BlackPixelOfScreen(screen);
	}

	if (alloced < 3)
	{
	    for (i=0, xcolors=ColInterpXColors(cmw);
		 i<alloced;
		 i++,xcolors++)
	    {
		    XFreeColors(XtDisplay(cmw),
			    cmw->core.colormap,
			    &xcolors->pixel, 1, 0);

		    xcolors->pixel = BlackPixelOfScreen(screen);
	    }
	    ColAllocInterpXColors(cmw) = FALSE;
	    SetInterpolatorSensitivity(cmw, FALSE);
	}
	else
	{
	    SetInterpolatorSensitivity(cmw, TRUE);
	    ReconfigureInterpolator(cmw, alloced);
	}
    }


    /* lastly, allocate cells for X11 browser colors */

    if (!ColAllocBrowserXColors(cmw))
    {
	pixels = (unsigned long *) XtMalloc (sizeof(unsigned long) *
					     ColBrowserItemCount(cmw));

	if (ColDispType(cmw) != GryScale)
	    ColAllocBrowserXColors(cmw) = (Boolean) AllocDynColorCells(cmw,
						    pixels,
						    ColBrowserItemCount(cmw));

	if (ColAllocBrowserXColors(cmw))
	{
	    for (i=0, pix = pixels, xcolors = ColBrowserXColors(cmw);
		 i < ColBrowserItemCount(cmw);
		 i++, pix++, xcolors++)
		    xcolors->pixel = (unsigned long) *pix;
	}
	else
	{
	    XtSetArg (al[0], XmNforeground, cmw->manager.foreground);
	    XtSetArg (al[1], XmNbackground, cmw->core.background_pixel);
	    for (i=0, pb = ColBrowserPbWid(cmw);
		 i < ColBrowserItemCount(cmw);
		 i++, pb++)
		    XtSetValues (*pb, al, 2);
	}

	XtFree ((char *)pixels);
    }

}	    


/*---------------------------------------------------*/
/* sets the size of the interpolator drawing area    */
/* if there are enough color cells for its use.	     */
/*---------------------------------------------------*/

static void ReconfigureInterpolator(cmw, alloced)
    DXmColorMixWidget cmw;
    int alloced;
{
    Arg al[5];
    int width;

    ColAllocInterpXColors(cmw) = TRUE;
    ColInterpTileCount(cmw) = alloced;

    width = ColInterpTileWidth(cmw) * ColInterpTileCount(cmw);

    XtSetArg (al[0], XmNwidth, width);
    XtSetValues(ColInterpDAWid(cmw), al, 1);

    LayoutPickerMixer (cmw);
}


/*---------------------------------------------------*/
/* sets the sensitivity of the various interpolator  */
/* controls.					     */
/*---------------------------------------------------*/

static void SetInterpolatorSensitivity(cmw, is_sensitive)
    DXmColorMixWidget cmw;
    Boolean is_sensitive;
{
    Arg al[5];

    if (ColIsInterpSensitive(cmw) == is_sensitive)
	return;

    if (is_sensitive == TRUE)
    {
	XtManageChild (ColLeftBucketPbWid(cmw));
	XtManageChild (ColRightBucketPbWid(cmw));
    }
    else
    {
	XtUnmanageChild (ColLeftBucketPbWid(cmw));
	XtUnmanageChild (ColRightBucketPbWid(cmw));
    }

    ColIsInterpSensitive(cmw) = is_sensitive;

    XtSetArg (al[0], XmNsensitive, is_sensitive);

    XtSetValues (ColInterpDAWid(cmw), al, 1);
    XtSetValues (ColITitleLabWid(cmw), al, 1);
    XtSetValues (ColLeftBucketPbWid(cmw), al, 1);
    XtSetValues (ColRightBucketPbWid(cmw), al, 1);
    XtSetValues (ColLighterLabWid(cmw), al, 1);
    XtSetValues (ColLighterPbWid(cmw), al, 1);
    XtSetValues (ColDarkerLabWid(cmw), al, 1);
    XtSetValues (ColDarkerPbWid(cmw), al, 1);
    XtSetValues (ColWarmerLabWid(cmw), al, 1);
    XtSetValues (ColWarmerPbWid(cmw), al, 1);
    XtSetValues (ColCoolerLabWid(cmw), al, 1);
    XtSetValues (ColCoolerPbWid(cmw), al, 1);
    XtSetValues (ColSmearPbWid(cmw), al, 1);
    XtSetValues (ColUndoPbWid(cmw), al, 1);
}
 

/*---------------------------------------------------*/
/* converts RGB color values to HLS		     */
/*---------------------------------------------------*/

static void ColorMixRGBToHLS(Red, Green, Blue, Hue, Light, Sat)
    double Red, Green, Blue, *Hue, *Light, *Sat;
{
    /*
     * algorithm from Foley and VanDam
     */

    double min,max;
    double rc,gc,bc;

    /* convert from X11 to standard RGB */

    Red   = Red/MAXCOLORVALUE;
    Green = Green/MAXCOLORVALUE;
    Blue  = Blue/MAXCOLORVALUE;

    color_range(Red,Green,Blue,&min,&max);

    /* lightness */
    *Light = (max + min)/2;

    /* calculate hue and saturation */
    if (max == min)
    {
	/* r=g=b -- achromatic case */
	*Sat = 0;
	*Hue = 0;  /* undefined? */
    }
    else
    {  /* chromatic case */
	if (*Light <= 0.5)
	   *Sat = (max-min)/(max+min);
	else
	   *Sat = (max-min)/(2-max-min);

	/* calculate hue */   
	rc = (max-Red)   / (max-min);
	gc = (max-Green) / (max-min);
	bc = (max-Blue)  / (max-min);

	if (Red == max)
	    *Hue = bc-gc;
	else if (Green == max)
	    *Hue = 2+rc-bc;
	else if (Blue == max)
	    *Hue = 4+gc-rc;

	*Hue = *Hue*60.0;

	if (*Hue < 0.0)
	    *Hue = *Hue+360;

    }  /* chromatic case */
}

static void color_range(r,g,b,min,max)
double r,g,b,*min,*max;
{

    *min = *max = r;

    if (g < *min)
	*min = g;
    else if (g > *max)
	*max = g;

    if (b < *min)
	*min = b;
    else if (b > *max)
	*max = b;

}  /* function color_range */


/*---------------------------------------------------*/
/* converts HLS color values to RGB		     */
/*                                                   */
/* from Carson Hovey's colormixhls.c...              */
/*---------------------------------------------------*/

static void ColorMixHLSToRGB(dHue, dLight, dSat, Red, Green, Blue)
    double dHue, dLight, dSat;
    unsigned short *Red, *Green, *Blue;
{

    /*
     * HLS algorithm from Foley and VanDam.  C implementation thanks to
     * Martin Brunecky at Auto-trol, Denver. 
     */

    double dRed, dGreen, dBlue;
    double m1,m2;

    if (dLight < 0.5) 
	m2 = (dLight)*(1+dSat);
    else 
        m2 = dLight + dSat- (dLight)*(dSat);

    m1 = (2.0*dLight) - m2;

    if ( dSat == 0 )
    {  /* gray shade (ignore hue?) */ 
	(dRed)=(dGreen)=(dBlue)=(dLight); 
    }
    else
    {  /* calculate RGB values */
	dRed   =value(m1,m2,(double)(dHue+120.0));
	dGreen =value(m1,m2,dHue);
	dBlue  =value(m1,m2,(double)(dHue-120.0));
    }

    /* rgb in range of 0.0 to 1.0 - convert to X11 0 to MAXCOLORVALUE */

    dRed   = dRed   * MAXCOLORVALUE;
    dGreen = dGreen * MAXCOLORVALUE;
    dBlue  = dBlue  * MAXCOLORVALUE;

    *Red   = (unsigned short) dRed;
    *Green = (unsigned short) dGreen;
    *Blue  = (unsigned short) dBlue;

    return;
}

static double value (n1,n2,hue)
double n1,n2,hue;
{
      double val;

      if (hue > 360.0)  
	hue = hue - 360.0;

      if (hue < 0.0)  
	hue = hue + 360.0;

      if (hue < 60.0)
	val = n1+(n2-n1)*hue/60.0;
      else if (hue < 180.0)
	val = n2;
      else if (hue < 240.0)
	val = n1+(n2-n1)*(240.0-hue)/60.0;
      else
	val = n1;
      return (val);
}  /* function VALUE */

                           
/*---------------------------------------------------*/
/* sets new color values to original color values and*/
/* updates mixer value (note that mixer proc may be  */
/* applications supplied and need not be associated  */
/* with default mixer)				     */
/*---------------------------------------------------*/

static void MatchNewToOrig(cmw,update_mixer)
    DXmColorMixWidget cmw;
    Boolean update_mixer;
{
    ColNewColorRed(cmw) = ColOrigColorRed(cmw);
    ColNewColorGrn(cmw) = ColOrigColorGrn(cmw);
    ColNewColorBlu(cmw) = ColOrigColorBlu(cmw);

    if (cmw->colormix.setmixcolproc != NULL)
    	(* cmw->colormix.setmixcolproc) (cmw,ColNewColorRed(cmw), 
		        		     ColNewColorGrn(cmw), 
		        		     ColNewColorBlu(cmw)); 

    if (ColModel(cmw) == DXmColorModelGreyscale)
	SetGreyscaleColors(cmw);

}


/*---------------------------------------------------*/
/* changes the origin of a widget or gadget	     */
/*---------------------------------------------------*/

static void ColMoveObject (g, x, y)
    RectObj g;
    int x;
    int y;
{
   if (XtIsWidget (g))
	XtMoveWidget ((Widget)g, x, y);
   else
	_XmConfigureObject((Widget) g, x, y, g->rectangle.width, g->rectangle.height, 0);
}


   
/*---------------------------------------------------*/
/* this the generic call back routine that will call */
/* the application with the appropriate reason(s)    */
/*---------------------------------------------------*/

static void ColorMixCallback(cmw, reason, event)
    DXmColorMixWidget 	cmw;
    unsigned int     	reason;
    XEvent             *event;
{
    DXmColorMixCallbackStruct temp;
    BrowserColor bc = ColNamedColor(cmw);

    temp.reason   = reason;
    temp.event    = event;
    temp.origred  = ColOrigColorRed(cmw);
    temp.origgrn  = ColOrigColorGrn(cmw);
    temp.origblu  = ColOrigColorBlu(cmw);
    temp.newred   = ColNewColorRed(cmw);
    temp.newgrn   = ColNewColorGrn(cmw);
    temp.newblu   = ColNewColorBlu(cmw);
    if (bc != NULL)
	temp.newname = bc->name;
    else
	temp.newname = (char *) NULL;

    switch (reason) 
      {
        case XmCR_ACTIVATE :
	    XtCallCallbacks ((Widget)cmw, XmNokCallback, &temp);
            break;

        case XmCR_APPLY:
	    XtCallCallbacks ((Widget)cmw, XmNapplyCallback, &temp);
            break;

        case XmCR_CANCEL :
	    XtCallCallbacks ((Widget)cmw, XmNcancelCallback, &temp);
            break;

        case XmCR_HELP     : 
	    XtCallCallbacks ((Widget)cmw, XmNhelpCallback, &temp);
            break;
       }
}


/*----------------------------------------------------*/
/* formats a valid X color specification string based */
/* on the RGB values and color format choice passed   */
/* in.						      */
/*----------------------------------------------------*/

static char *FormatColorString (red, green, blue, format)
    unsigned short red, green, blue;
    XcmsColorFormat format;
{
    char temp_string[200], *return_str;
    Boolean success = FALSE;
    int i;

    switch (format)
    {
	case XcmsRGBFormat :
	    sprintf (temp_string, "rgb:%4.4x/%4.4x/%4.4x", red, green, blue);

	    /* Some CRTLs right justify with spaces, we need to substitute */
	    /* zeros in order for the RGB string to be valid		   */

	    for (i=0; i<strlen(temp_string); i++)
		if (isspace(temp_string[i]))
		    temp_string[i] = '0';

	    success = TRUE;
	    break;
    }

    if (success)
    {
	return_str = XtMalloc (strlen(temp_string) + 1);
	strcpy (return_str, temp_string);
	return (return_str);
    }
    else
	return (NULL);
}


/*---------------------------------------------------*/
/* returns the first colormix ancestor of a widget   */
/*---------------------------------------------------*/

static DXmColorMixWidget GetColorMixAncestor (widget)
    Widget widget;
{
    Widget temp=widget;

    for (;((temp != NULL) && !DXmIsColorMix(temp)); temp=XtParent(temp))
    {};

    return ((DXmColorMixWidget)temp);
}    


/*---------------------------------------------------*/
/* converts a compound text string to a normal text  */
/* string using the same algorithms as the XmText    */
/* widget.					     */
/*---------------------------------------------------*/

static char *ColCvtCTToString (type, value, length, w)
    Atom *type;
    XtPointer value;
    unsigned long *length;
    Widget w;
{
    XTextProperty tmp_prop;
    char ** tmp_value;
    char * total_tmp_value = (char *) NULL;
    int i, num_vals, status, malloc_size = 0;

    tmp_prop.value = (unsigned char *) value;
    tmp_prop.encoding = *type;
    tmp_prop.format = 8;
    tmp_prop.nitems = *length;
    num_vals = 0;

    status = XmbTextPropertyToTextList(XtDisplay(w), &tmp_prop, &tmp_value,
				       &num_vals);

    /* if no conversion, num_vals is not changed */
    if (num_vals && (status == Success || status > 0)) {
	for (i = 0; i < num_vals ; i++)
	    malloc_size += strlen(*tmp_value + i);
	total_tmp_value = XtMalloc ((unsigned) malloc_size + 1);
	total_tmp_value[0] = '\0';
	for (i = 0; i < num_vals ; i++)
	    strcat(total_tmp_value, *tmp_value + i);
	XFreeStringList(tmp_value);
    } 

    return (total_tmp_value);
} 


/*---------------------------------------------------*/
/* checks target compatibility and then initiates a  */
/* drop transfer by calling XmDropTransferStart.     */
/*						     */
/* NOTE: The XmDropProcCallbackStruct passed into    */
/*       this routine via the call_data parameter    */
/*       is sometimes generated synthetically by     */
/*       the DropHelpOK callback, which only fills   */
/*       in the x, y, and drag context fields.       */
/*       The other fields in this structure are      */
/*       bogus and should not be referenced in this  */
/*       in this routine.			     */
/*---------------------------------------------------*/

static void DoDropSiteTransfer (w, call_data)
    Widget w;
    XtPointer call_data;
{
    XmDropProcCallback DropData;
    XmDropTransferEntryRec transferEntries[4];
    XmDropTransferEntry	transferList;
    Arg al[10];
    ColDNDContext drop_ctx;
    Widget drag_cont, initiator;
    Atom *exportTargets;
    Cardinal numExportTargets, i, n=0;
    Boolean target_found = FALSE;

    DropData = (XmDropProcCallback) call_data;

    drag_cont = DropData->dragContext;

    n = 0;
    XtSetArg(al[n], XmNsourceWidget, &initiator); n++;
    XtSetArg(al[n], XmNexportTargets, &exportTargets); n++;
    XtSetArg(al[n], XmNnumExportTargets, &numExportTargets); n++;
    XtGetValues((Widget) drag_cont, al, n);

    COMPOUND_TEXT = XmInternAtom (XtDisplay(w), "COMPOUND_TEXT", False);

    for (i = 0; i < numExportTargets; i++) 
	if (exportTargets[i] == COMPOUND_TEXT)
	    target_found = TRUE;

    n = 0;

    if (target_found)
    {
	drop_ctx = (ColDNDContext) XtMalloc(sizeof(ColDNDContextRec));
	drop_ctx->w = w;
	drop_ctx->x = DropData->x;
	drop_ctx->y = DropData->y;

	transferEntries[0].target = COMPOUND_TEXT;
	transferEntries[0].client_data = (XtPointer) drop_ctx;
	transferList = transferEntries;
	XtSetArg (al[n], XmNdropTransfers, transferList); n++;
	XtSetArg (al[n], XmNnumDropTransfers, 1); n++;
	XtSetArg (al[n], XmNtransferProc, ReceiveColorTransfer); n++;
    }
    else
    {
	XtSetArg(al[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
	XtSetArg(al[n], XmNnumDropTransfers, 0); n++;
    }

    XmDropTransferStart (DropData->dragContext, al, n);
}


/*---------------------------------------------------*/
/* brings up a help dialog if drop site help is      */
/* requested on one of the colormix subwidgets.      */
/*---------------------------------------------------*/

static void DoDropSiteHelp (w, call_data)
    Widget w;
    XtPointer call_data;
{
    XmDropProcCallback DropData = (XmDropProcCallback) call_data;
    static ColDNDContextRec helpData;
    Arg al[10];
    DXmColorMixWidget cmw = GetColorMixAncestor(w);
 
    /* Help requested, post help dialog */

    if (ColDropHelpDialog(cmw) == (Widget) NULL)
	CreateDropHelpDialog(cmw);

    if (w == ColInterpDAWid(cmw))
	XtSetArg (al[0], XmNmessageString, ColDropHelpInterpLab(cmw));
    else
	XtSetArg (al[0], XmNmessageString, ColDropHelpTileLab(cmw));

    XtSetValues (ColDropHelpDialog(cmw), al, 1);

    XtRemoveAllCallbacks (ColDropHelpDialog(cmw), XmNokCallback);	 
    XtRemoveAllCallbacks (ColDropHelpDialog(cmw), XmNcancelCallback);	 

    XtAddCallback ((Widget)ColDropHelpDialog(cmw), XmNokCallback, (XtCallbackProc) DropHelpOK, (XtPointer) &helpData);
    XtAddCallback ((Widget)ColDropHelpDialog(cmw), XmNcancelCallback, (XtCallbackProc) DropHelpCancel, (XtPointer) &helpData);

    helpData.w = w;
    helpData.x = DropData->x;
    helpData.y = DropData->y;
    helpData.dragContext = DropData->dragContext;

    XtManageChild (ColDropHelpDialog(cmw));
}


/*---------------------------------------------------*/
/* called whenever the user attempts to drop onto    */
/* one of the subwidgets capable of receiving color  */
/* data.  Initiates the drop transfer process.       */
/*---------------------------------------------------*/

static void HandleColorDrop (w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XmDropProcCallback DropData;

    DropData = (XmDropProcCallback) call_data;

    if (DropData->dropAction == XmDROP_HELP)
    {
	/* Bring up help dialog */

	DoDropSiteHelp (w, call_data);
    }
    else
    {
	DoDropSiteTransfer (w, call_data);
    }
}


/*---------------------------------------------------*/
/* Transfers a dropped color to the widget dropped   */
/* upon.					     */
/*---------------------------------------------------*/

static void ReceiveColorTransfer (w, closure, seltype, type, 
				  value, length, format)
    Widget w;
    XtPointer closure;
    Atom *seltype;
    Atom *type;
    XtPointer value;
    unsigned long *length;
    int format;
{
    char *color_string;
    XColor xc, screen;
    Status status;
    DXmColorMixWidget cmw;
    ColDNDContext drop_ctx = (ColDNDContext) closure;
    Widget widget = drop_ctx->w;

    cmw = GetColorMixAncestor(drop_ctx->w);

    DXM_COLOR = XmInternAtom (XtDisplay(w), "DXM_COLOR", False);
    COMPOUND_TEXT = XmInternAtom (XtDisplay(w), "COMPOUND_TEXT", False);

    if ((*type == COMPOUND_TEXT) || (*type == DXM_COLOR)) {
	color_string = ColCvtCTToString (type, value, length, widget);
	if (color_string)
	{
	    status = XLookupColor (XtDisplay(widget), widget->core.colormap,
				  color_string, &xc, &screen); 		
	    if (status)
	    {
		if (widget == ColNewWid(cmw))
		{
		    ColNamedColor(cmw) = NULL;
		    DXmColorMixSetNewColor(cmw, xc.red, xc.green, xc.blue);
		}
		else if (widget == ColSPDrawingAreaWid(cmw))
		{
		    AddColorToScratchPad (cmw, xc.red, xc.green, xc.blue);
		}
		else if (widget == ColInterpDAWid(cmw))
		{
		    SetEndTile (cmw, xc.red, xc.green, xc.blue,
				drop_ctx->x, drop_ctx->y);
		}

		XtFree ((char *)drop_ctx);
	    }
	    XtFree (color_string);
	}
    }
}


/*---------------------------------------------------*/
/* starts drag processing when MB2 is pressed over a */
/* colored area.				     */
/*---------------------------------------------------*/

static void StartColorDrag (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg al[20];
    Cardinal n=0;
    Atom exportList[2];
    DXmColorMixWidget cmw = GetColorMixAncestor(w);
    ColDNDContext drop_ctx;
    XColor *xc;
    int i, tile;

    drop_ctx = (ColDNDContext) XtMalloc(sizeof(ColDNDContextRec));
    drop_ctx->w = w;
    drop_ctx->x = event->xbutton.x;
    drop_ctx->y = event->xbutton.y;

    if (w == ColPickerDAWid(cmw))
    {
	tile = drop_ctx->x / ColPickerTileWidth(cmw);
	for (i=0, xc = ColPickerXColors(cmw); i<tile; i++, xc++)
	    ;	    /* Find color record for tile clicked upon */
	XtSetArg (al[n], XmNcursorBackground, xc->pixel); n++;
    }
    else if (w == ColInterpDAWid(cmw))
    {
	tile = drop_ctx->x / ColInterpTileWidth(cmw);
	for (i=0, xc = ColInterpXColors(cmw); i<tile; i++, xc++)
	    ;	    /* Find color record for tile clicked upon */
	XtSetArg (al[n], XmNcursorBackground, xc->pixel); n++;
    }
    else
    {
	XtSetArg (al[n], XmNcursorBackground, w->core.background_pixel); n++;
    }

    DXM_COLOR = XmInternAtom (XtDisplay(w), "DXM_COLOR", False);
    COMPOUND_TEXT = XmInternAtom (XtDisplay(w), "COMPOUND_TEXT", False);

    exportList[0] = DXM_COLOR;
    exportList[1] = COMPOUND_TEXT;

    XtSetArg (al[n], XmNsourceCursorIcon, ColDragIcon(cmw)); n++;
    XtSetArg (al[n], XmNblendModel, XmBLEND_JUST_SOURCE); n++;
    XtSetArg (al[n], XmNexportTargets, exportList); n++;
    XtSetArg (al[n], XmNnumExportTargets, 2); n++;
    XtSetArg (al[n], XmNdragOperations, XmDROP_COPY); n++;
    XtSetArg (al[n], XmNconvertProc, DragColorConvertProc); n++;
    XtSetArg (al[n], XmNclientData, (XtPointer) drop_ctx);  n++;
    XmDragStart (w, event, al, n);    
}


/*---------------------------------------------------*/
/* converts the dragged color value to the desired   */
/* output format and passes it on to the drop widget */
/*---------------------------------------------------*/

static Boolean DragColorConvertProc ( w, selection, target, type,
				      value, length, format )
    Widget w ;
    Atom *selection, *target, *type ;
    XtPointer *value ;
    unsigned long *length ;
    int *format ;
{
    Arg al[10];
    DXmColorMixWidget cmw;
    char *temp_str, *passtext;
    XmString temp_cs;
    BrowserColor bc;
    ColDNDContext drop_ctx;
    XColor *xc;
    int i, tile;				       

    DXM_COLOR = XmInternAtom (XtDisplay(w), "DXM_COLOR", False);
    COMPOUND_TEXT = XmInternAtom (XtDisplay(w), "COMPOUND_TEXT", False);

    if ((*target != DXM_COLOR) && (*target != COMPOUND_TEXT))
	return (FALSE);

    XtSetArg (al[0], XmNclientData, &drop_ctx);
    XtGetValues (w, al, 1);
    cmw = GetColorMixAncestor(drop_ctx->w);
    
    if (drop_ctx->w == ColOrigWid(cmw))
    {
	temp_str =  FormatColorString (ColOrigColorRed(cmw),
				       ColOrigColorGrn(cmw),   
				       ColOrigColorBlu(cmw),
				       XcmsRGBFormat);
    } 
    else if (drop_ctx->w == ColNewWid(cmw))
    {
	bc = ColNamedColor(cmw);

	if (bc != NULL)
	    temp_str = strcpy (XtMalloc(strlen(bc->name)+1), bc->name);
	else
	    temp_str =  FormatColorString (ColNewColorRed(cmw),
					   ColNewColorGrn(cmw),   
					   ColNewColorBlu(cmw),
					   XcmsRGBFormat);
    }
    else if (drop_ctx->w == ColSPDrawingAreaWid(cmw))
    {
	xc = (XColor *) &(ColCurrentSPColor(cmw));
	temp_str =  FormatColorString (xc->red, xc->green, xc->blue,
				       XcmsRGBFormat);
    }
    else if (drop_ctx->w == ColPickerDAWid(cmw))
    {
	tile = drop_ctx->x / ColPickerTileWidth(cmw);

	for (i=0, xc = ColPickerXColors(cmw); i<tile; i++, xc++)
	    ;	    /* Find color record for tile released upon */

	temp_str =  FormatColorString (xc->red, xc->green, xc->blue,
				       XcmsRGBFormat);
    }
    else if (drop_ctx->w == ColInterpDAWid(cmw))
    {
	tile = drop_ctx->x / ColInterpTileWidth(cmw);

	for (i=0, xc = ColInterpXColors(cmw); i<tile; i++, xc++)
	    ;	    /* Find color record for tile released upon */

	temp_str =  FormatColorString (xc->red, xc->green, xc->blue,
				       XcmsRGBFormat);
    }

    temp_cs = XmStringCreateLocalized (temp_str);
    passtext = XmCvtXmStringToCT (temp_cs);

    *type = *target;
    *value = (XtPointer) passtext;
    *length = strlen (passtext);
    *format = 8;

    XtFree (temp_str);
    XmStringFree (temp_cs);

    return (TRUE);
}


/*---------------------------------------------------*/
/* Called whenever a drag enters, leaves, or moves   */
/* within the scratch pad tile or new color tile.    */
/* Highlights or unhighlights the tile as necessary. */
/* The interpolator doesn't use this drag proc, it   */
/* has it's own because it needs to do extra work to */
/* handle the two end tiles correctly.		     */
/*---------------------------------------------------*/

static void GenericColorDragProc(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Atom *exportTargets;
    Cardinal numExportTargets;
    Boolean valid_target = FALSE;
    XmDragProcCallbackStruct *cb = (XmDragProcCallbackStruct *) call_data;
    DXmColorMixWidget cmw;
    Arg al[10];
    int i;

    cb->operations = XmDROP_COPY;

    if (cb->reason != XmCR_DROP_SITE_MOTION_MESSAGE)
    {
	COMPOUND_TEXT = XmInternAtom (XtDisplay(w), "COMPOUND_TEXT", False);
	cmw = GetColorMixAncestor(w);

	/* Get list of transfer targets */

	XtSetArg(al[0], XmNexportTargets, &exportTargets);
	XtSetArg(al[1], XmNnumExportTargets, &numExportTargets);
	XtGetValues(cb->dragContext, al, 2);

	for (i=0; i < numExportTargets; i++)
	    if (exportTargets[i] == COMPOUND_TEXT)
		valid_target = TRUE;

	if (valid_target)
	    switch(cb->reason) {
		case XmCR_DROP_SITE_ENTER_MESSAGE:
		    cb->animate = FALSE;
		    DrawButtonBorder (cmw, w, 4, NULL);
		    break;
		case XmCR_DROP_SITE_LEAVE_MESSAGE:
		    cb->animate = FALSE;
		    if (w == ColNewWid(cmw))
			DrawButtonBorder (cmw, w, 4, &(ColNewColor(cmw)));
		    else if (w == ColSPDrawingAreaWid(cmw))
			DrawButtonBorder (cmw, w, 4, &(ColCurrentSPColor(cmw)));
		    break;
	    }
    }
}



/*---------------------------------------------------*/
/* Called whenever a drag enters, leaves, or moves   */
/* within the interpolator.  Highlights or           */
/* unhighlights the end tiles as necessary.	     */
/*---------------------------------------------------*/

static void InterpColorDragProc(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Atom *exportTargets;
    Cardinal numExportTargets;
    Boolean valid_target = FALSE;
    XmDragProcCallbackStruct *cb = (XmDragProcCallbackStruct *) call_data;
    DXmColorMixWidget cmw;
    XColor *lc, *rc;
    Arg al[10];
    int i, tile;

    cb->operations = XmDROP_COPY;

    COMPOUND_TEXT = XmInternAtom (XtDisplay(w), "COMPOUND_TEXT", False);
    cmw = GetColorMixAncestor(w);

    /* Get list of transfer targets */

    XtSetArg(al[0], XmNexportTargets, &exportTargets);
    XtSetArg(al[1], XmNnumExportTargets, &numExportTargets);
    XtGetValues(cb->dragContext, al, 2);

    for (i=0; i < numExportTargets; i++)
	if (exportTargets[i] == COMPOUND_TEXT)
	    valid_target = TRUE;

    if (valid_target) {
	if (cb->x > (XtWidth(w) / 2))
	    tile = ColInterpTileCount(cmw) - 1;
	else 
	    tile = 0;

	lc = ColInterpXColors(cmw);
	for (i=1, rc = lc; i<ColInterpTileCount(cmw); i++, rc++)
	    ;	/* Find color record for right hand tile */

	switch(cb->reason) {
	    case XmCR_DROP_SITE_ENTER_MESSAGE:
		ColLastInterpSide(cmw) = tile;
		cb->animate = FALSE;
		DrawInterpTileBorder (cmw, tile, 4, NULL);
		break;
	    case XmCR_DROP_SITE_LEAVE_MESSAGE:
		ColLastInterpSide(cmw) = -1;
		cb->animate = FALSE;
		DrawInterpTileBorder (cmw, 0, 4, lc);
		DrawInterpTileBorder (cmw, 0, 2, NULL);
		DrawInterpTileBorder (cmw, ColInterpTileCount(cmw)-1, 4, rc);
		DrawInterpTileBorder (cmw, ColInterpTileCount(cmw)-1, 2, NULL);
		break;
	    case XmCR_DROP_SITE_MOTION_MESSAGE:
		if (ColLastInterpSide(cmw) != tile)
		{
		    ColLastInterpSide(cmw) = tile;
		    cb->animate = FALSE;
		    DrawInterpTileBorder (cmw, tile, 4, NULL);
		    if (tile == 0) {
			DrawInterpTileBorder (cmw, ColInterpTileCount(cmw)-1, 4, rc);
			DrawInterpTileBorder (cmw, ColInterpTileCount(cmw)-1, 2, NULL);
		    } else {
			DrawInterpTileBorder (cmw, 0, 4, lc);
			DrawInterpTileBorder (cmw, 0, 2, NULL);
		    }
		}
		break;
	}
    }
}


/*---------------------------------------------------*/
/* Creates the message box displayed whenever the    */
/* the user drags an object over a color tile drop   */
/* site and presses the help key.		     */
/*---------------------------------------------------*/

static void CreateDropHelpDialog(cmw)
    DXmColorMixWidget	  cmw;
{
    Arg al[20];
    int ac=0;


    XtSetArg (al[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ac++;

    ColDropHelpDialog(cmw) = (Widget) XmCreateMessageDialog (cmw, "DropHelpDialog", al, ac);

    XtUnmanageChild ((Widget) XmMessageBoxGetChild( ColDropHelpDialog(cmw),
						    XmDIALOG_HELP_BUTTON));
}


/*---------------------------------------------------*/
/* called when user clicks OK button on drop site    */
/* help dialog.  Carries out the transfer.	     */
/*---------------------------------------------------*/

static XtCallbackProc DropHelpOK (w, client, callback)
    Widget	  	  w;
    XtPointer		  client;
    XtPointer		  callback;
{
    ColDNDContext helpData = (ColDNDContext) client;
    static XmDropProcCallbackStruct drop_proc_struct;

    drop_proc_struct.x = helpData->x;
    drop_proc_struct.y = helpData->y;
    drop_proc_struct.dragContext = helpData->dragContext;

    DoDropSiteTransfer (helpData->w, (XtPointer) &drop_proc_struct);
}


/*---------------------------------------------------*/
/* Called when user clicks Cancel button on drop     */
/* site help dialog.  Cancels the transfer in        */
/* progress.					     */
/*---------------------------------------------------*/

static XtCallbackProc DropHelpCancel (w, client, callback)
    Widget	  	  w;
    XtPointer		  client;
    XtPointer		  callback;
{
    ColDNDContext helpData = (ColDNDContext) client;
    Arg al[2];

    XtSetArg(al[0], XmNtransferStatus, XmTRANSFER_FAILURE);
    XtSetArg(al[1], XmNnumDropTransfers, 0);

    XmDropTransferStart (helpData->dragContext, al, 2);
}



/*---------------------------------------------------*/
/* default color display widget's map routine -      */
/* called whenever the display widget is mapped      */
/* (becomes visible)				     */
/*---------------------------------------------------*/

static void ColorMixDispMap(cmw, tag, callback)
    DXmColorMixWidget	  cmw;
    XtPointer		  tag;
    XmAnyCallbackStruct *callback;
{
    Arg al[5];
    int i;
    Widget *kid;

    if (!XtIsRealized(cmw))
	return;

    /* if not a popup, then 'map' routine called at realize and scale widget
     * will not have a window - therefore, cannot add pixmap
     */
    if (ColIsPopup(cmw))
    {
    	/* Set the background pixmap of the greyscale scale */
    	/* widget's scroll bar window		        */

    	for (i=0, kid=XtChildren(ColGreyscaleSclWid(cmw));
	     i<XtNumChildren(ColGreyscaleSclWid(cmw));
	     i++, kid++)
    	{
	

	    if (XtIsSubclass(*kid, xmScrollBarWidgetClass))
	    	XSetWindowBackgroundPixmap (XtDisplay(*kid),
				            XtWindow(*kid),
					    ColGreyscalePixmap(cmw));
    	}
    }

    if (ColAllocNewColor(cmw) || ColAllocOrigColor(cmw) || ColAllocBackColor(cmw))
    {
	SetColorMixDimensions (cmw);
	return;
    }

    if (ColDefDisp(cmw))
    {
    	if (ColMatchColors(cmw))
	    MatchNewToOrig(cmw, TRUE);

	if (ColDispType(cmw) == BlackAndWhite || ColDispType(cmw) == StatGray)
	{    
	    RemoveColDisplay(cmw);
	    SetPickerOptionSensitivity (cmw, FALSE);
	}
	else 
	{
	    if (ColDispType(cmw) == DynColor || ColDispType(cmw) == GryScale)
		AllocDynColors(cmw);

	    SetUpColors(cmw);
	}
    }

    SetColorMixDimensions (cmw);
}

   
/*---------------------------------------------------*/
/* frees the color cells allocated by the picker     */
/* drawing area widget.				     */
/*---------------------------------------------------*/

static void FreePickerXColors(cmw)
    DXmColorMixWidget	  cmw;
{
    int i;
    XColor *xc;

    for (i=0, xc=ColPickerXColors(cmw);    
	 i < ColPickerColorCount(cmw);
	 i++, xc++)    
	    XFreeColors(XtDisplay(cmw),
			cmw->core.colormap,
			&xc->pixel,
			(int) 1,
			(unsigned long) 0);
		     
    ColAllocPickerXColors(cmw) = FALSE;
}

   
/*---------------------------------------------------*/
/* frees the color cells allocated by the	     */
/* interpolator drawing area widget.		     */
/*---------------------------------------------------*/

static void FreeInterpXColors(cmw)
    DXmColorMixWidget	  cmw;
{
    int i;
    XColor *xc;

    for (i=0, xc=ColInterpXColors(cmw);    
	 i < ColInterpTileCount(cmw);
	 i++, xc++)    
	    XFreeColors(XtDisplay(cmw),
			cmw->core.colormap,
			&xc->pixel,
			(int) 1,
			(unsigned long) 0);
		     
    ColAllocInterpXColors(cmw) = FALSE;
}

   
/*---------------------------------------------------*/
/* frees the color cells allocated by the	     */
/* browser color model.				     */
/*---------------------------------------------------*/

static void FreeBrowserXColors(cmw)
    DXmColorMixWidget	  cmw;
{
    int i;
    XColor *xc;

    for (i=0, xc=ColBrowserXColors(cmw);    
	 i < ColBrowserItemCount(cmw);
	 i++, xc++)    
	    XFreeColors(XtDisplay(cmw),
			cmw->core.colormap,
			&xc->pixel,
			(int) 1,
			(unsigned long) 0);
		     
    ColAllocBrowserXColors(cmw) = FALSE;
}

   
/*---------------------------------------------------*/
/* default color display widget's unmap routine -    */
/* called whenever the display widget is unmapped    */
/* (no longer visible)				     */
/*---------------------------------------------------*/

static void ColorMixDispUnmap(cmw, tag, callback)
    DXmColorMixWidget	  cmw;
    XtPointer		  tag;
    XmAnyCallbackStruct *callback;
{
    XColor *xcolors;
    int i;
		     
    if (!XtIsRealized(cmw))
	return;

    if (XtIsManaged (ColSPPopupWid(cmw)))
	XtUnmanageChild (ColSPPopupWid(cmw));

    if (ColDefDisp(cmw))
    {
    	if (ColDispType(cmw) == GryScale || ColDispType(cmw) == DynColor)
    	{
    	    if (ColAllocNewColor(cmw))
    	    {
	    	XFreeColors(XtDisplay(cmw),
		    	cmw->core.colormap,
		    	&ColNewColorPix(cmw),
		    	(int) 1,
		    	(unsigned long) 0);

	    	ColAllocNewColor(cmw) = FALSE;
    	    }

    	    if (ColAllocOrigColor(cmw))
    	    {
	    	XFreeColors(XtDisplay(cmw),
		    	cmw->core.colormap,
		    	&ColOrigColorPix(cmw),
		    	(int) 1,
		    	(unsigned long) 0);

	    	ColAllocOrigColor(cmw) = FALSE;
    	    }

    	    if (ColAllocBackColor(cmw))
    	    {
	    	XFreeColors(XtDisplay(cmw),
		    	cmw->core.colormap,
		    	&ColBackColorPix(cmw),
		    	(int) 1,
		    	(unsigned long) 0);

	    	ColAllocBackColor(cmw) = FALSE;
    	    }

    	    if (ColAllocScratchColor(cmw))
    	    {
	    	XFreeColors(XtDisplay(cmw),
		    	cmw->core.colormap,
		    	&ColCurrentSPPix(cmw),
		    	(int) 1,
		    	(unsigned long) 0);

	    	ColAllocScratchColor(cmw) = FALSE;
    	    }


    	    if (ColAllocPickerXColors(cmw))
		FreePickerXColors(cmw);

    	    if (ColAllocInterpXColors(cmw))
		FreeInterpXColors(cmw);


    	    if (ColAllocBrowserXColors(cmw))
		FreeBrowserXColors(cmw);

	}
    }  /* if default display widget */
}


/*-----------------------------------------------------*/
/* Deselects the current selected object.	       */
/*-----------------------------------------------------*/

static void UnhighlightCurrentColor (cmw)
    DXmColorMixWidget	cmw;
{
    Arg al[5];

    if (ColIsPickerSelected(cmw))
    {
	ColIsPickerSelected(cmw) = FALSE;
	DrawPickerTileBorder (cmw, ColPickerSelectTile(cmw), 
			      4, ColHighlightedColor(cmw));
	return;
    }

    if (ColIsInterpSelected(cmw))
    {
	ColIsInterpSelected(cmw) = FALSE;
	DrawInterpTileBorder (cmw, ColInterpSelectTile(cmw), 
			      4, ColHighlightedColor(cmw));

	/* Draw new black border if this is an end tile */
	if ((ColInterpSelectTile(cmw) == 0) || 
	    (ColInterpSelectTile(cmw) == (ColInterpTileCount(cmw)-1)))
		DrawInterpTileBorder (cmw, ColInterpSelectTile(cmw), 2, NULL);

	return;
    }

    if (ColIsOrigSelected(cmw))
    {
	ColIsOrigSelected(cmw) = FALSE;

	DrawButtonBorder 
		(cmw, ColOrigWid(cmw), 4, &(ColOrigColor(cmw)));
	return;
    }

    if (ColIsNewSelected(cmw))
    {
	ColIsNewSelected(cmw) = FALSE;

	DrawButtonBorder 
		(cmw, ColNewWid(cmw), 4, &(ColNewColor(cmw)));
	return;
    }

    if (ColIsScratchPadSelected(cmw))
    {
	ColIsScratchPadSelected(cmw) = FALSE;
	DrawButtonBorder 
		(cmw, ColSPDrawingAreaWid(cmw), 4, ColHighlightedColor(cmw));
	return;
    }
}


/*---------------------------------------------------*/
/* repaints the highlight border around the scratch  */
/* pad tile if required.			     */
/*---------------------------------------------------*/
static void ColorMixScratchPadExpose (w, tag, callback)
    XmDrawnButtonWidget	  w;
    XtPointer		  tag;
    XmDrawnButtonCallbackStruct *callback;
{
    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));
    XColor *xc;

    if (ColIsScratchPadSelected(cmw))
	    DrawButtonBorder (cmw, ColSPDrawingAreaWid(cmw), 4, NULL);

}


/*---------------------------------------------------*/
/* repaints exposed regions of the color picker      */
/* drawing area widget with the proper colored tiles */
/*---------------------------------------------------*/

static void ColorMixPickerRedraw (cmw, left, right)
    DXmColorMixWidget cmw;
    int left, right;
{
    int i;
    XColor *xc;
    XGCValues gc_vals;
    unsigned long gc_mask;
    GC gc;
    Window window = XtWindow(ColPickerDAWid(cmw));
    Display *dpy = XtDisplay(cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);

    /* Don't let the tile painting generate additional exposure events */
    gc_vals.graphics_exposures = FALSE;    
    gc_vals.line_width = 1;
    gc_mask = GCGraphicsExposures | GCLineWidth;
    gc = XCreateGC (dpy, window, gc_mask, &gc_vals);

    for (i=0, xc = ColPickerXColors(cmw); i<left; i++, xc++)
	;			   /* Find color record for first exposed tile */

    for (i=left; i<=right; i++)    /* For all exposed picker tiles */
    {
	XSetForeground (dpy, gc, xc->pixel);

	XFillRectangle (dpy,				/* Display */
			window,				/* Drawing area window */
			gc,				/* Graphics context */
			i * ColPickerTileWidth(cmw),	/* x, left edge of tile */
			0,				/* y, from expose event */
			ColPickerTileWidth(cmw),	/* width, one tile wide */
			XtHeight(ColPickerDAWid(cmw)) );/* height, from expose event */
			
	XSetForeground (dpy, gc, BlackPixelOfScreen(screen));
	
	if (i < ColPickerColorCount(cmw) - 1)  /* Tile separator line */
	    XDrawLine (dpy, window, gc, 
			(i+1) * ColPickerTileWidth(cmw) - 1,
			0,
			(i+1) * ColPickerTileWidth(cmw) - 1,
			ColPickerTileHeight(cmw));

	if (ColIsPickerSelected(cmw) && (i==ColPickerSelectTile(cmw)))
	    DrawPickerTileBorder (cmw, i, 4, NULL);

	xc++;
    }

    XFreeGC (dpy, gc);
}


/*---------------------------------------------------*/
/* repaints exposed regions of the color picker      */
/* drawing area widget with the proper colored tiles */
/*---------------------------------------------------*/

static void ColorMixPickerExpose (w, tag, callback)
    XmDrawnButtonWidget	  w;
    XtPointer		  tag;
    XmDrawnButtonCallbackStruct *callback;
{
    int left, right;
    XEvent *event = callback->event;
    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    /* Compute the indices of the two tiles on the edges of the expose region */
    left = event->xexpose.x / ColPickerTileWidth(cmw);
    right = (event->xexpose.x + event->xexpose.width - 1) / ColPickerTileWidth(cmw);

    ColorMixPickerRedraw (cmw, left, right);
}


/*----------------------------------------------------*/
/* repaints exposed regions of the color interpolator */
/* drawing area widget with the proper colored tiles  */
/* and draws borders around end tiles if needed.      */
/*----------------------------------------------------*/

static void ColorMixInterpRedraw (cmw, left, right)
    DXmColorMixWidget cmw;
    int left,right;
{
    int i;
    XColor *xc;
    XGCValues gc_vals;
    unsigned long gc_mask;
    GC gc;
    Window window = XtWindow(ColInterpDAWid(cmw));
    Display *dpy = XtDisplay(cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);
			
    /* Don't let the tile painting generate additional exposure events */
    gc_vals.graphics_exposures = FALSE;    
    gc_mask = GCGraphicsExposures;
    gc = XCreateGC (dpy, window, gc_mask, &gc_vals);

    for (i=0, xc = ColInterpXColors(cmw); i<left; i++, xc++)
	;			   /* Find color record for first exposed tile */

    for (i=left; i<=right; i++)    /* For all exposed picker tiles */
    {
	XSetForeground (dpy, gc, xc->pixel);

	XFillRectangle (dpy,				/* Display */
			window,				/* Drawing area window */
			gc,				/* Graphics context */
			i * ColInterpTileWidth(cmw),	/* x, left edge of tile */
			0,				/* y, from expose event */
			ColInterpTileWidth(cmw),	/* width, one tile wide */
			XtHeight(ColInterpDAWid(cmw)) );/* height, from expose event */

	XSetForeground (dpy, gc, BlackPixelOfScreen(screen));
	
	if (i < ColInterpTileCount(cmw) - 1)  /* Tile separator line */
	    XDrawLine (dpy, window, gc, 
			(i+1) * ColInterpTileWidth(cmw) - 1,
			0,
			(i+1) * ColInterpTileWidth(cmw) - 1,
			ColInterpTileHeight(cmw));

	if (ColIsInterpSelected(cmw) && (i==ColInterpSelectTile(cmw)))
	    DrawInterpTileBorder (cmw, i, 4, NULL);

	xc++;
    }

    if (left == 0)				/* Leftmost tile */
	DrawInterpTileBorder (cmw, left, 2, NULL);

    if (right == (ColInterpTileCount(cmw)-1))	/* Rightmost tile */
	DrawInterpTileBorder (cmw, right, 2, NULL);

    XFreeGC (dpy, gc);
}


/*----------------------------------------------------*/
/* repaints exposed regions of the color interpolator */
/* drawing area widget with the proper colored tiles  */
/* and draws borders around end tiles if needed.      */
/*----------------------------------------------------*/

static void ColorMixInterpExpose (w, tag, callback)
    XmDrawnButtonWidget	  w;
    XtPointer		  tag;
    XmDrawnButtonCallbackStruct *callback;
{
    int left, right;
    XEvent *event = callback->event;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));
			
    /* Compute the indices of the two tiles on the edges of the expose region */
    left = event->xexpose.x / ColInterpTileWidth(cmw);
    right = (event->xexpose.x + event->xexpose.width - 1) / ColInterpTileWidth(cmw);

    ColorMixInterpRedraw (cmw, left, right);

}


/*----------------------------------------------------*/
/* draws a black border of the given width around the */
/* specified interpolator tile.			      */
/*----------------------------------------------------*/

static void DrawInterpTileBorder (cmw, tile, line_width, color)
    DXmColorMixWidget	cmw;
    int			tile;
    int			line_width;
    XColor		*color;
{
    unsigned long pixel;
    XGCValues gc_vals;
    unsigned long gc_mask;
    GC gc;
    int x, y, width, height;
    Screen  *screen = (Screen *) XtScreen(cmw);

    /* Don't let the tile painting generate additional exposure events */
    gc_vals.graphics_exposures = FALSE;    
    gc_vals.line_width = line_width;
    if (color == NULL)
	gc_vals.foreground = BlackPixelOfScreen(screen);
    else
	gc_vals.foreground = color->pixel;

    gc_mask = (GCGraphicsExposures | GCForeground | GCLineWidth);
    gc = XCreateGC (XtDisplay(cmw), XtWindow(cmw), gc_mask, &gc_vals);

    x = tile * ColInterpTileWidth(cmw) + (line_width/2);
    y = line_width/2;
    width = ColInterpTileWidth(cmw) - line_width - 1;
    height = ColInterpTileHeight(cmw) - line_width;

    /* Adjust width for righthand tile because it already has a one */
    /* pixel border on its right side				    */

    if (tile == (ColInterpTileCount(cmw) - 1))
	width++;

    XDrawRectangle (XtDisplay(cmw),		    /* Display */
		    XtWindow(ColInterpDAWid(cmw)),  /* Window */
		    gc,				    /* Graphics context */
		    x,
		    y,
		    width, 
		    height);

    XFreeGC (XtDisplay(cmw), gc);
}


/*----------------------------------------------------*/
/* draws a border of the given width around the       */
/* specified picker tile.			      */	
/*----------------------------------------------------*/

static void DrawPickerTileBorder (cmw, tile, line_width, color)
    DXmColorMixWidget	cmw;
    int			tile;
    int			line_width;
    XColor		*color;
{
    unsigned long pixel;
    XGCValues gc_vals;
    unsigned long gc_mask;
    GC gc;
    int x, y, width, height;
    Screen  *screen = (Screen *) XtScreen(cmw);
	
    /* Don't let the tile painting generate additional exposure events */
    gc_vals.graphics_exposures = FALSE;    
    gc_vals.line_width = line_width;
    if (color == NULL)
	gc_vals.foreground = BlackPixelOfScreen(screen);
    else
	gc_vals.foreground = color->pixel;

    gc_mask = (GCGraphicsExposures | GCForeground | GCLineWidth);
    gc = XCreateGC (XtDisplay(cmw), XtWindow(cmw), gc_mask, &gc_vals);

    x = tile * ColPickerTileWidth(cmw) + (line_width/2);
    y = line_width/2;
    width = ColPickerTileWidth(cmw) - line_width - 1;
    height = ColPickerTileHeight(cmw) - line_width;

    /* Adjust width for righthand tile because it already has a one */
    /* pixel border on its right side				    */

    if (tile == (ColPickerColorCount(cmw) - 1))
	width++;

    XDrawRectangle (XtDisplay(cmw),		    /* Display */
		    XtWindow(ColPickerDAWid(cmw)),  /* Window */
		    gc,				    /* Graphics context */
		    x, 
		    y, 
		    width,
		    height);

    XFreeGC (XtDisplay(cmw), gc);
}


/*----------------------------------------------------*/
/* draws a border of the given width around the       */
/* scratch pad color tile			      */
/*----------------------------------------------------*/

static void DrawButtonBorder (cmw, w, line_width, color)
    DXmColorMixWidget	cmw;
    Widget 		w;
    int			line_width;
    XColor		*color;
{
    unsigned long pixel;
    XGCValues gc_vals;
    unsigned long gc_mask;
    GC gc;
    int x, y, width, height;
    Screen  *screen = (Screen *) XtScreen(cmw);
	
    /* Don't let the tile painting generate additional exposure events */
    gc_vals.graphics_exposures = FALSE;    
    gc_vals.line_width = line_width;
    if (color == NULL)
	gc_vals.foreground = BlackPixelOfScreen(screen);
    else
	gc_vals.foreground = color->pixel;

    gc_mask = (GCGraphicsExposures | GCForeground | GCLineWidth);
    gc = XCreateGC (XtDisplay(cmw), XtWindow(cmw), gc_mask, &gc_vals);

    x = line_width/2;
    y = line_width/2;
    width  = XtWidth(w)  - line_width;
    height = XtHeight(w) - line_width;
/*
    width = ColPickerTileWidth(cmw) * 3 - line_width - 1;
    height = ColPickerTileHeight(cmw) * 2 - line_width;
*/

    XDrawRectangle (XtDisplay(cmw),		    /* Display */
		    XtWindow(w),  		    /* Window */
		    gc,				    /* Graphics context */
		    x, 
		    y, 
		    width,
		    height);

    XFreeGC (XtDisplay(cmw), gc);
}



/*---------------------------------------------------*/
/* Causes a selection box to be drawn around the     */
/* picker tile clicked on.			     */
/*---------------------------------------------------*/

static void PickerSelect (cmw, event)
    DXmColorMixWidget cmw;
    XEvent *event;
{
    XColor *xc;
    int i, tile;

    if (!ColIsInterpSensitive(cmw))
	return;

    tile = event->xbutton.x / ColPickerTileWidth(cmw);
    for (i=0, xc = ColPickerXColors(cmw); i<tile; i++, xc++)
	;	/* Find color record for selected tile */

    ColIsPickerSelected(cmw) = TRUE;
    ColPickerSelectTile(cmw) = tile;
    ColHighlightedColor(cmw) = xc;

    DrawPickerTileBorder (cmw, tile, 4, NULL);
}


/*---------------------------------------------------*/
/* changes the cursor to an eyedropper filled with   */
/* the specified color and then performs a pointer   */
/* grab on behalf of the interpolator drawing area   */
/* widget window.				     */
/*---------------------------------------------------*/

static void ChangeAndGrabPointer (cmw, xc)
    DXmColorMixWidget cmw;
    XColor *xc;
{
    long    emask;
    XColor  fg, dummy;
    Display *dpy = (Display *) XtDisplay (cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);

    /* Change the eyedropper cursor color */

    XLookupColor (dpy, cmw->core.colormap, "black",
	&dummy, &fg);

    XRecolorCursor (dpy, ColPickerCursor(cmw), &fg, xc);

    /* Have the interpolator window grab this cursor */

    ColIsPointerGrabbed(cmw) = TRUE;
    emask = (ButtonReleaseMask | ButtonPressMask);
    XGrabPointer (dpy, XtWindow(ColInterpDAWid(cmw)), FALSE, emask, 
	GrabModeAsync, GrabModeAsync, None, ColPickerCursor(cmw), CurrentTime); 

}


/*---------------------------------------------------*/
/* handles an MB1 press on a picker tile.	     */
/*---------------------------------------------------*/

static void PickerMB1Press (cmw, event)
    DXmColorMixWidget cmw;
    XEvent *event;
{
    UnhighlightCurrentColor(cmw);
    PickerSelect(cmw, event);
}



/*---------------------------------------------------*/
/* handles an MB1 release on a picker tile.  This    */
/* consists of determining the tile's color and	     */
/* setting the color mix display widget to match it. */
/*---------------------------------------------------*/

static void PickerMB1Release (cmw, event)
    DXmColorMixWidget cmw;
    XEvent *event;
{
    int tile, i;
    XColor *xc;

    /* See if button release is within button press window */
    if ((event->xbutton.x < 0) || (event->xbutton.y < 0) || 
	(event->xbutton.x > XtWidth(ColPickerDAWid(cmw))) || 
	(event->xbutton.y > XtHeight(ColPickerDAWid(cmw))))
	    return;

    /* Find and set new color */

    tile = event->xbutton.x / ColPickerTileWidth(cmw);

    for (i=0, xc = ColPickerXColors(cmw); i<tile; i++, xc++)
	    ;	    /* Find color record for tile released upon */

    DXmColorMixSetNewColor(cmw, xc->red, xc->green, xc->blue);
}


/*---------------------------------------------------*/
/* dispatches mouse button events received by the    */
/* color picker drawing area widget		     */
/*---------------------------------------------------*/

static void ColorMixPickerInput (w, tag, callback)
    XmDrawnButtonWidget	  w;
    XtPointer		  tag;
    XmDrawnButtonCallbackStruct *callback;
{
    int i;
    XColor *xc;
    XEvent *event = callback->event;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    if (event == 0)
	return;

    if ((event->type == ButtonPress) && (event->xbutton.button == Button1))
	PickerMB1Press (cmw, event);

    if ((event->type == ButtonRelease) && (event->xbutton.button == Button1))
	PickerMB1Release (cmw, event);

    if ((event->type == KeyPress) && (callback->reason == XmCR_ARM))
    	DXmColorMixSetNewColor(cmw, 
			ColHighlightedColor(cmw)->red, 
			ColHighlightedColor(cmw)->green, 
			ColHighlightedColor(cmw)->blue);
}


/*----------------------------------------------------*/
/* causes the either the new or original tile to look */
/* selected by giving it a four pixel borderwidth     */
/*----------------------------------------------------*/

static void DisplaySelect (cmw, w)
    DXmColorMixWidget cmw;
    Widget w;
{
    Arg al[5];

    if (!ColIsInterpSensitive(cmw))
	return;

    if (w == ColOrigWid(cmw))
    {
	ColIsOrigSelected(cmw) = TRUE;
	ColHighlightedColor(cmw) = (XColor *) &(ColOrigColor(cmw));
    }
    else
    {
	ColIsNewSelected(cmw) = TRUE;
	ColHighlightedColor(cmw) = (XColor *) &(ColNewColor(cmw));
    }

    DrawButtonBorder (cmw, w, 4, NULL);
}


/*---------------------------------------------------*/
/* handles an MB1 press on the new or original color */
/* display tile.				     */
/*---------------------------------------------------*/

static void DisplayMB1Press (w, event)
    Widget w;
    XEvent *event;
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);
    DisplaySelect(cmw, w);
}


/*---------------------------------------------------*/
/* handles an MB1 release on the new or original     */
/* color display tiles.				     */
/*---------------------------------------------------*/

static void DisplayMB1Release (w, event)
    Widget w;
    XEvent *event;
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));
    XColor *xc;

    /* See if button release is within button press window */
    if ((event->xbutton.x < 0) || (event->xbutton.y < 0) || 
	(event->xbutton.x > XtWidth(w)) || 
	(event->xbutton.y > XtHeight(w)))
	    return;

    if (w == ColOrigWid(cmw))
    {
	ColNamedColor(cmw) = NULL;
	xc = (XColor *) &(ColOrigColor(cmw));
	DXmColorMixSetNewColor(cmw, xc->red, xc->green, xc->blue);
    }
}



/*---------------------------------------------------*/
/* dispatches mouse button events received by the    */
/* color mix new and original color display drawing  */
/* area widgets					     */
/*---------------------------------------------------*/

static void ColorMixDisplayInput (w, tag, callback)
    XmDrawnButtonWidget	  w;
    XtPointer		  tag;
    XmDrawnButtonCallbackStruct *callback;
{
    XColor *xc;
    XEvent *event = callback->event;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    if (event == 0)
	return;

    if ((event->type == ButtonRelease) && (event->xbutton.button == Button1))
	DisplayMB1Release (w, event);

    if ((event->type == ButtonPress) && (event->xbutton.button == Button1))
	DisplayMB1Press (w, event);	    

    if ((event->type == KeyPress) && (callback->reason == XmCR_ARM))
    	if (w == (XmDrawnButtonWidget) ColOrigWid(cmw))
    	{
	    ColNamedColor(cmw) = NULL;
	    xc = (XColor *) &(ColOrigColor(cmw));
	    DXmColorMixSetNewColor(cmw, xc->red, xc->green, xc->blue);
	}
}



/*----------------------------------------------------*/
/* called whenever interpolator tiles chane color.    */
/* Converts the RGB values of each tile to HLS and    */
/* stores these values.  These HLS values are used    */
/* for color manipulation via the lighter and darker  */
/* buttons.					      */
/*----------------------------------------------------*/

static void SaveInterpHLSValues(cmw)
    DXmColorMixWidget cmw;
{
    int i;
    HLSValues hls;
    XColor *xc;

    for (i=0, xc=ColInterpXColors(cmw), hls=ColInterpHLSValues(cmw);
	 i < ColInterpTileCount(cmw); 
	 i++, xc++, hls++)
    {
	ColorMixRGBToHLS ((double) xc->red, 
			  (double) xc->green, 
			  (double) xc->blue, 
			  &hls->hue, 
			  &hls->lightness, 
			  &hls->saturation);
    }
}


/*----------------------------------------------------*/
/* called when the warmer arrow button is activated.  */
/* Increments the red value each interpolator tile.   */
/*----------------------------------------------------*/

static void ColorMixMakeWarmer (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    int i;
    XColor *xc;
    Boolean alloced = TRUE;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);

    SaveUndoState (cmw);

    for (i=0, xc=ColInterpXColors(cmw);
	 i < ColInterpTileCount(cmw); 
	 i++, xc++)
    {
	if ((int) (xc->red + ColWarmthIncrement(cmw)) > MAXCOLORVALUE)
	    xc->red = MAXCOLORVALUE;
	else
	    xc->red = xc->red + ColWarmthIncrement(cmw);

        if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xc, alloced);
	else
	    SetAColor(cmw, xc, &alloced);
    }

    SaveInterpHLSValues(cmw);

    if (ColDispType(cmw) == StatColor)  
	ColorMixInterpRedraw (cmw, 0, ColInterpTileCount(cmw)-1);
}


/*-----------------------------------------------------*/
/* called when the cooler arrow button is activated.   */
/* Increments the blue value of each interpolator tile */
/*-----------------------------------------------------*/

static void ColorMixMakeCooler (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    int i;
    XColor *xc;
    Boolean alloced = TRUE;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);

    SaveUndoState (cmw);

    for (i=0, xc=ColInterpXColors(cmw);
	 i < ColInterpTileCount(cmw); 
	 i++, xc++)
    {
	if ((int) (xc->blue + ColWarmthIncrement(cmw)) > MAXCOLORVALUE)
	    xc->blue = MAXCOLORVALUE;
	else
	    xc->blue = xc->blue + ColWarmthIncrement(cmw);

        if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xc, alloced);
	else
	    SetAColor(cmw, xc, &alloced);
    }

    SaveInterpHLSValues(cmw);

    if (ColDispType(cmw) == StatColor)  
	ColorMixInterpRedraw (cmw, 0, ColInterpTileCount(cmw)-1);
}


/*-----------------------------------------------------*/
/* called when the lighter arrow button is activated.  */
/* Increments the the lightness value for each         */
/* interpolator tile				       */
/*-----------------------------------------------------*/

static void ColorMixMakeLighter (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    int i;
    XColor *xc;
    HLSValues hls;
    double linc;
    Boolean alloced = TRUE;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);

    SaveUndoState (cmw);

    linc = (double) ((double) ColLightnessIncrement(cmw) / (double) 100.0);
 
    for (i=0, xc=ColInterpXColors(cmw), hls=ColInterpHLSValues(cmw);
	 i < ColInterpTileCount(cmw); 
	 i++, xc++, hls++)
    {
	if (hls->lightness + linc > 1.0)
	    hls->lightness = 1.0;
	else 
	    hls->lightness = hls->lightness + linc;

	ColorMixHLSToRGB(hls->hue, hls->lightness, hls->saturation,
			 &xc->red, &xc->green, &xc->blue);

        if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xc, alloced);
	else
	    SetAColor(cmw, xc, &alloced);
     
    }

    if (ColDispType(cmw) == StatColor)  
	ColorMixInterpRedraw (cmw, 0, ColInterpTileCount(cmw)-1);
}



/*-----------------------------------------------------*/
/* called when the darker arrow button is activated.   */
/* Decrements the lightness value for each             */
/* interpolator tile				       */
/*-----------------------------------------------------*/

static void ColorMixMakeDarker (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    int i;
    XColor *xc;
    HLSValues hls;
    double linc;
    Boolean alloced = TRUE;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);

    SaveUndoState (cmw);

    linc = (double) ((double) ColLightnessIncrement(cmw) / (double) 100.0);
 
    for (i=0, xc=ColInterpXColors(cmw), hls=ColInterpHLSValues(cmw);
	 i < ColInterpTileCount(cmw); 
	 i++, xc++, hls++)
    {
	if (hls->lightness - linc < 0.0)
	    hls->lightness = 0.0;
	else 
	    hls->lightness = hls->lightness - linc;

	ColorMixHLSToRGB(hls->hue, hls->lightness, hls->saturation,
			 &xc->red, &xc->green, &xc->blue);

        if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xc, alloced);
	else
	    SetAColor(cmw, xc, &alloced);
     
    }

    if (ColDispType(cmw) == StatColor)  
	ColorMixInterpRedraw (cmw, 0, ColInterpTileCount(cmw)-1);
}


/*-----------------------------------------------------*/
/* Called when a browser named color button is pushed, */
/* sets the new color tile to the color of the button  */
/*-----------------------------------------------------*/

static void ColorMixBrowserSelect (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(w))));
    BrowserColor color = (BrowserColor) tag;

    ColNamedColor(cmw) = color;

    DXmColorMixSetNewColor(cmw, color->red, color->green, color->blue);
}


/*-------------------------------------------------------*/
/* Called when the value of the browser scroll bar is    */
/* changed.  Updates the labels, foreground colors,      */
/* background colors, and callback closures of the       */
/* browser buttons to reflect the scrolling action.	 */
/*-------------------------------------------------------*/

static void ColorMixBrowserScroll (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmScrollBarCallbackStruct *callback;
{   
    int i, ac=0;
    Arg al[20];
    WidgetPtr pb;
    XColor *xcolors;
    BrowserColor colors;
    unsigned long fg_pixel;
    int value = callback->value;
    Boolean browser_alloced = TRUE;
    XmString pb_label = NULL;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));
    Display *dpy = XtDisplay(cmw);
    Screen  *screen = (Screen *) XtScreen(cmw);

    if (value > (ColBrowserColorCount(cmw) - ColBrowserItemCount(cmw) + 1))
    {
	value = ColBrowserColorCount(cmw) - ColBrowserItemCount(cmw) + 1;
	XmScrollBarSetValues(ColBrowserSBWid(cmw), value, ColBrowserItemCount(cmw), 0, 0, FALSE);
    }

    for (i=0, colors = ColBrowserColors(cmw);
	 i < value; 
	 i++, colors++)
	    ;		    /* Find new top color record */

    for (i=0, xcolors = ColBrowserXColors(cmw), pb = ColBrowserPbWid(cmw); 
	 i < ColBrowserItemCount(cmw); 
	 i++, xcolors++, colors++, pb++)
    {
	ac = 0;

	xcolors->red   = colors->red;
	xcolors->green = colors->green;
	xcolors->blue  = colors->blue;

	if (ColAllocBrowserXColors(cmw))
	{
	    if (ColDispType(cmw) == GryScale)
		SetAGrayColor(cmw, xcolors, ColAllocBrowserXColors(cmw));
	    else
	    {    
		SetAColor(cmw, xcolors, &(ColAllocBrowserXColors(cmw)));
		browser_alloced = (browser_alloced && ColAllocBrowserXColors(cmw)); 
	    }

	    XtSetArg(al[ac], XmNbackground, xcolors->pixel); ac++;

	    if (colors->dark_fg)
		fg_pixel = BlackPixelOfScreen(screen);
	    else
		fg_pixel = WhitePixelOfScreen(screen);
	
	    XtSetArg(al[ac], XmNforeground, fg_pixel); ac++;
	}

	XtSetArg(al[ac], XmNlabelString, colors->string);   ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;

	cmw_browser_cb[0].closure = (XtPointer) colors;
	XtSetArg(al[ac], XmNactivateCallback, cmw_browser_cb); ac++;

	XtSetValues(*pb, al, ac);
    }

    if (pb_label != NULL) 
	XmStringFree (pb_label);
    
}


/*----------------------------------------------------*/
/* called whenever the greyscale mixer is about to    */
/* be selected.  Converts the color display widget to */
/* a matching greyscale shade.			      */
/*----------------------------------------------------*/

static void SetGreyscaleColors(cmw)
    DXmColorMixWidget cmw;
{
    double grey;

    grey = (ColNewColorRed(cmw) + ColNewColorGrn(cmw) + ColNewColorBlu(cmw))/3;

    ColNewColorRed(cmw) = (unsigned short) grey;
    ColNewColorGrn(cmw) = (unsigned short) grey;
    ColNewColorBlu(cmw) = (unsigned short) grey;

    /* set the 'new' color in the color display window */

    if (cmw->colormix.setnewcolproc != NULL)
        (* cmw->colormix.setnewcolproc)
	(cmw, ColNewColorRed(cmw), ColNewColorGrn(cmw), ColNewColorBlu(cmw));

    grey = grey/655.35;  /* Update the scale slider */

    XmScaleSetValue(ColGreyscaleSclWid(cmw), (int) grey);
}


/*----------------------------------------------------*/
/* called whenever the greyscale mixer's scale widget */
/* is changed by the user.  Changes the color display */
/* widget to match the scale's new value	      */
/*----------------------------------------------------*/

static void ColorMixGreyscaleUpdate (w, tag, callback)
    Widget	  	  w;
    XtPointer		  tag;
    XmScaleCallbackStruct *callback;
{   
    double grey;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    grey = (double) callback->value * 655.35;

    ColNewColorRed(cmw) = (unsigned short) grey;
    ColNewColorGrn(cmw) = (unsigned short) grey;
    ColNewColorBlu(cmw) = (unsigned short) grey;

    /* set the 'new' color in the color display window */

    if (cmw->colormix.setnewcolproc != NULL)
        (* cmw->colormix.setnewcolproc)
	(cmw, ColNewColorRed(cmw), ColNewColorGrn(cmw), ColNewColorBlu(cmw));
}



/*-----------------------------------------------------*/
/* called whenever a change is about to be made to     */
/* the interpolator window.  Saves the interpolator's  */
/* current color state so that restoration is possible */
/* via the undo button.				       */
/*----------------------------------------------------*/

static void SaveUndoState (cmw)
    DXmColorMixWidget cmw;
{
    int i;
    XColor *xc, *uc;

    for (i=0, xc = ColInterpXColors(cmw), uc = ColUndoXColors(cmw);
	 i<ColInterpTileCount(cmw);
	 i++, xc++, uc++)
    {
	*uc = *xc;
    }
}


/*----------------------------------------------------*/
/* called when the undo button is activated, this     */
/* procedure restores the interpolator state saved    */
/* in the undo list.
/*----------------------------------------------------*/

static void ColorMixInterpUndo (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));
    int i;
    XColor *xc;

    UnhighlightCurrentColor(cmw);

    /* Swap current and undo color arrays */

    xc = ColInterpXColors(cmw);
    ColInterpXColors(cmw) = ColUndoXColors(cmw);
    ColUndoXColors(cmw) = xc;

    /* Apply the new colors */

    for (i=0, xc = ColInterpXColors(cmw);
	 i<ColInterpTileCount(cmw);
	 i++, xc++)
    {
	if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xc, ColAllocInterpXColors(cmw));
	else
	    SetAColor(cmw, xc, &(ColAllocInterpXColors(cmw)));
    }

    if (ColDispType(cmw) == StatColor)  
	ColorMixInterpRedraw (cmw, 0, ColInterpTileCount(cmw)-1);

    SaveInterpHLSValues(cmw);
}


/*----------------------------------------------------*/
/* called when a picker paint bucket is activated,    */
/* dumps the new color into the appropriate end of    */
/* the interpolator bar.			      */
/*----------------------------------------------------*/

static void ColorMixInterpFill (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    int i,tile=0;
    Boolean alloced = TRUE;
    XColor *ic;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));

    UnhighlightCurrentColor(cmw);

    SaveUndoState(cmw);

    ic = ColInterpXColors(cmw);

    if (w == ColRightBucketPbWid(cmw))
 	    for (i=1 ; i<ColInterpTileCount(cmw); i++)
		ic++;	    /* Find right edge tile */

    /* Set the tile color */

    ic->flags = DoRed | DoGreen | DoBlue;
    ic->red = ColNewColorRed(cmw);
    ic->green = ColNewColorGrn(cmw);
    ic->blue = ColNewColorBlu(cmw);

    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw, ic, alloced);
    else
	SetAColor(cmw, ic, &alloced);

    if (ColDispType(cmw) == StatColor)  
    {
   	if (w == ColRightBucketPbWid(cmw))
	    tile = ColInterpTileCount(cmw)-1;
 
	ColorMixInterpRedraw (cmw, tile, tile);
    }

    SaveInterpHLSValues(cmw);
}


/*----------------------------------------------------*/
/* called when the smear button is activated, this    */
/* procedure determines new colors for the central    */
/* interpolator tiles by interpolating in a linear    */
/* fashion between the RGB values of the two current  */
/* end tiles.					      */
/*----------------------------------------------------*/

static void ColorMixInterpSmear (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(w));
    int central_tiles, i;
    unsigned short lred, lgreen, lblue, rred, rgreen, rblue;
    int rinc, ginc, binc;
    XColor *xc = ColInterpXColors(cmw);

    UnhighlightCurrentColor(cmw);

    central_tiles = ColInterpTileCount(cmw) - 2;

    if (central_tiles < 1)
	return;

    SaveUndoState (cmw);

    lred = xc->red; lgreen = xc->green; lblue = xc->blue;

    for (i=0; i < (central_tiles+1); i++)
	    xc++;   /* Find color record of last tile */

    rred = xc->red; rgreen = xc->green; rblue = xc->blue;
    
    /* Compute per tile color increments */

    if ((rred - lred) == 0)
	rinc = 0;
    else
    {
	rinc = rred - lred;
	rinc = rinc / (central_tiles + 1);
    }

    if ((rgreen - lgreen) == 0)
	ginc = 0;
    else
    {
	ginc = rgreen - lgreen;
	ginc = ginc / (central_tiles + 1);
    }

    if ((rblue - lblue) == 0)
	binc = 0;
    else
    {
	binc = rblue - lblue;
	binc = binc / (central_tiles + 1);
    }

    xc = ColInterpXColors (cmw); xc++; /* Start with 2nd tile */

    for (i=0; i<central_tiles; i++) /* Compute new color */
    {
	xc->red =   (unsigned short) (lred + (rinc * (i+1)));
	xc->green = (unsigned short) (lgreen + (ginc * (i+1)));
	xc->blue =  (unsigned short) (lblue + (binc * (i+1)));
	xc->flags = DoRed | DoGreen | DoBlue;

	if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xc, ColAllocInterpXColors(cmw));
	else
	    SetAColor(cmw, xc, &(ColAllocInterpXColors(cmw)));

	xc++;
    }

    if (ColDispType(cmw) == StatColor)  
	ColorMixInterpRedraw (cmw, 0, ColInterpTileCount(cmw)-1);

    SaveInterpHLSValues(cmw);
}


/*---------------------------------------------------*/
/* Causes a selection box to be drawn around the     */
/* interpolator tile clicked on.		     */
/*---------------------------------------------------*/

static void InterpSelect(cmw, event)
    DXmColorMixWidget cmw;
    XEvent *event;
{
    XColor *xc;
    int i, tile;

    if (!ColIsInterpSensitive(cmw))
	return;

    tile = event->xbutton.x / ColInterpTileWidth(cmw);
    for (i=0, xc = ColInterpXColors(cmw); i<tile; i++, xc++)
	;	/* Find color record for selected tile */

    ColIsInterpSelected(cmw) = TRUE;
    ColInterpSelectTile(cmw) = tile;
    ColHighlightedColor(cmw) = xc;
    DrawInterpTileBorder (cmw, tile, 4, NULL);
}


/*---------------------------------------------------*/
/* handles an MB1 press on an interpolator tile.     */
/*---------------------------------------------------*/

static void InterpMB1Press (cmw, event)
    DXmColorMixWidget cmw;
    XEvent *event;
{
    UnhighlightCurrentColor(cmw);
    InterpSelect(cmw, event);
}


/*---------------------------------------------------*/
/* handles an MB1 release on an interpolator tile.   */
/* The color of the interpolator tile is used to set */
/* the new color display widget.		     */
/*---------------------------------------------------*/

static void InterpMB1Release (cmw, event)
    DXmColorMixWidget cmw;
    XEvent *event;
{
    int tile, i;
    XColor *xc;

    /* See if button release is within button press window */
    if ((event->xbutton.x < 0) || (event->xbutton.y < 0) || 
	(event->xbutton.x > XtWidth(ColInterpDAWid(cmw))) || 
	(event->xbutton.y > XtHeight(ColInterpDAWid(cmw))))
	    return;

    /* Find and set new color */

    tile = event->xbutton.x / ColInterpTileWidth(cmw);

    for (i=0, xc = ColInterpXColors(cmw); i<tile; i++, xc++)
	    ;	    /* Find color record for tile released upon */

    DXmColorMixSetNewColor(cmw, xc->red, xc->green, xc->blue);
}


/*---------------------------------------------------*/
/* sets the interpolator end tile color		     */
/*---------------------------------------------------*/

static void SetEndTile (cmw, red, green, blue, x, y)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;
    Position x, y;
{
    int tile = 0, i;
    XColor *ic;
    Widget da = ColInterpDAWid(cmw);

    if ((x<0) || (y<0) || (x>XtWidth(da)) || (y>XtHeight(da)))
	    return;

    SaveUndoState(cmw);

    /* Determine whether to set left or right edge tile */

    ic = ColInterpXColors(cmw);

    if (x > (XtWidth(da) / 2))
	for (i=1 ; i<ColInterpTileCount(cmw); i++)
	    ic++;	    /* Find right edge tile */

    /* Set the tile color */

    ic->flags = DoRed | DoGreen | DoBlue;
    ic->red = red;
    ic->green = green;
    ic->blue = blue;

    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw, ic, ColAllocInterpXColors(cmw));
    else
	SetAColor(cmw, ic, &(ColAllocInterpXColors(cmw)));

    if (ColDispType(cmw) == StatColor)  
    {
	if (x > (XtWidth(da) / 2))
	    tile = ColInterpTileCount(cmw)-1;

	ColorMixInterpRedraw (cmw, tile, tile);
    }

    SaveInterpHLSValues(cmw);
}


/*---------------------------------------------------*/
/* sets the color of the closest interpolator end    */
/* tile when the user moves the eyedropper cursor    */
/* and releases MB1.
/*---------------------------------------------------*/

static void SetInterpolatorEndTile (w, event)
    XmDrawnButtonWidget	  w;
    XEvent		  *event;
{
    int x, y;
    XColor *pc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    XUngrabPointer (XtDisplay(cmw), CurrentTime);
    ColIsPointerGrabbed(cmw) = FALSE;

    /* Check if eyedropper cursor is actually in interpolator window */

    x = event->xbutton.x;
    y = event->xbutton.y;

    if (!event->xbutton.same_screen)
	    return;

    pc = ColGrabColor(cmw);

    SetEndTile (cmw, pc->red, pc->green, pc->blue, x, y);

}


/*---------------------------------------------------*/
/* dispatches mouse button events received by the    */
/* color interpolator drawing area widget	     */
/*---------------------------------------------------*/

static void ColorMixInterpInput (w, tag, callback)
    XmDrawnButtonWidget	  w;
    XtPointer		  tag;
    XmDrawnButtonCallbackStruct *callback;
{
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));
    XEvent *event = callback->event;

    if (event == 0)
	return;

    /* Hendle MB1 down */

    if ((event->type == ButtonPress) && (event->xbutton.button == Button1))
	InterpMB1Press (cmw, event);

    /* Handle button release */

    if ((event->type == ButtonRelease) && (event->xbutton.button == Button1))
	InterpMB1Release (cmw, event);

    if ((event->type == KeyPress) && (callback->reason == XmCR_ARM))
    	DXmColorMixSetNewColor(cmw, 
			ColHighlightedColor(cmw)->red, 
			ColHighlightedColor(cmw)->green, 
			ColHighlightedColor(cmw)->blue);
}



/*----------------------------------------------------*/
/* Sets the scratch pad to its initial state of a     */
/* single white tile.				      */
/*----------------------------------------------------*/

static void ColorMixScratchPadClear (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    XColor *xcolor;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));
    
    if (ColScratchCount(cmw) == 1)
	return;

    xcolor = (XColor *) &ColCurrentSPColor(cmw);

    xcolor->red   = (unsigned short) MAXCOLORVALUE; 
    xcolor->green = (unsigned short) MAXCOLORVALUE; 
    xcolor->blue  = (unsigned short) MAXCOLORVALUE; 	

    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw, xcolor, ColAllocScratchColor(cmw));
    else
	SetAColor(cmw, xcolor, &(ColAllocScratchColor(cmw)));

    if (ColDispType(cmw) == StatColor)  
    {
    	Arg al[1];
    	XtSetArg(al[0], XmNbackground, xcolor->pixel);
    	XtSetValues(ColSPDrawingAreaWid(cmw), al, 1);
    }

    if (ColDispType(cmw) == StatColor)  
	DrawButtonBorder (cmw, ColSPDrawingAreaWid(cmw), 4, NULL);

    if (ColScratchColors(cmw) != NULL)
	XtFree ((char *)ColScratchColors(cmw));

    ColScratchCount(cmw) = 1;

    xcolor = (XColor *) XtMalloc (sizeof(XColor) * ColScratchCount(cmw));

    xcolor->red   = (unsigned short) MAXCOLORVALUE; 
    xcolor->green = (unsigned short) MAXCOLORVALUE; 
    xcolor->blue  = (unsigned short) MAXCOLORVALUE; 	

    ColScratchColors(cmw) = xcolor;

    XtUnmanageChild (ColSPScrollBarWid(cmw));
}


/*----------------------------------------------------*/
/* Unmanages the scratch pad popup		      */
/*----------------------------------------------------*/

static void ColorMixScratchPadCancel (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));

    XtUnmanageChild (ColSPPopupWid(cmw));
}


/*----------------------------------------------------*/
/* Adds a color to the scratch pad.		      */
/*----------------------------------------------------*/

static void AddColorToScratchPad (cmw, red, green, blue)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;
{
    Arg al[10];
    int i;
    XColor *spc;
    XmScrollBarCallbackStruct sbcallback;

    ColScratchColors(cmw) = (XColor *) XtRealloc ((char *)ColScratchColors(cmw), 
				sizeof(XColor) * (ColScratchCount(cmw) + 1));

    for (i=0, spc=ColScratchColors(cmw); i<ColScratchCount(cmw); i++, spc++)
	;   /* Point to new item at end of list */
	    
    spc->red =  red;
    spc->green = green;
    spc->blue = blue;

    if (ColScratchCount(cmw) == 1)
	XtManageChild(ColSPScrollBarWid(cmw));

    ColScratchCount(cmw)++;

    XtSetArg (al[0], XmNminimum, 0);
    XtSetArg (al[1], XmNmaximum, ColScratchCount(cmw));
    XtSetArg (al[2], XmNvalue, ColScratchCount(cmw) - 1);
    XtSetArg (al[3], XmNsliderSize, 1);
    XtSetValues (ColSPScrollBarWid(cmw), al, 4);

    sbcallback.value = ColScratchCount(cmw) - 1;
    XtCallCallbacks (ColSPScrollBarWid(cmw), XmNvalueChangedCallback, &sbcallback);
}


/*----------------------------------------------------*/
/* Adds the currently selected color to the scratch   */
/* pad, called when the scratch pad paint bucket is   */
/* clicked on.					      */
/*----------------------------------------------------*/

static void ColorMixScratchPadAddColor (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    Arg al[10];
    int i;
    XColor *spc;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(w)));
    XmScrollBarCallbackStruct sbcallback;

    UnhighlightCurrentColor(cmw);

    AddColorToScratchPad (cmw,
			  ColNewColorRed(cmw),
			  ColNewColorGrn(cmw),
			  ColNewColorBlu(cmw));
}


/*----------------------------------------------------*/
/* causes the scratch pad drawing area to look        */
/* selected by giving it a four pixel borderwidth     */
/*----------------------------------------------------*/

static void ScratchPadSelect (cmw)
    DXmColorMixWidget cmw;
{
    ColIsScratchPadSelected(cmw) = TRUE;
    ColHighlightedColor(cmw) = (XColor *) &(ColCurrentSPColor(cmw));

    DrawButtonBorder (cmw, ColSPDrawingAreaWid(cmw), 4, NULL);
}


/*---------------------------------------------------*/
/* Handles an MB1 press on the scratch pad color     */
/* area.					     */
/*---------------------------------------------------*/

static void ScratchPadMB1Press (cmw, w, event)
    DXmColorMixWidget cmw;
    Widget w;
    XEvent *event;
{
    UnhighlightCurrentColor(cmw);
    ScratchPadSelect(cmw);
}


/*---------------------------------------------------*/
/* handles an MB1 release on the scratch pad tile by */
/* setting the new color tile equal to the current   */
/* visible scratch pad color			     */
/*---------------------------------------------------*/

static void ScratchPadMB1Release (cmw, event)
    DXmColorMixWidget cmw;
    XEvent *event;
{
    XColor *xc;

    /* See if button release is within button press window */
    if ((event->xbutton.x < 0) || (event->xbutton.y < 0) || 
	(event->xbutton.x > XtWidth(ColSPDrawingAreaWid(cmw))) || 
	(event->xbutton.y > XtHeight(ColSPDrawingAreaWid(cmw))))
	    return;

    ColNamedColor(cmw) = NULL;
    xc = (XColor *) &(ColCurrentSPColor(cmw));
    DXmColorMixSetNewColor(cmw, xc->red, xc->green, xc->blue);
}



/*---------------------------------------------------*/
/* Handles mouse actions on the scratch pad.	     */
/*---------------------------------------------------*/

static void ColorMixScratchPadInput (w, tag, callback)
    XmDrawnButtonWidget	  w;
    XtPointer		  tag;
    XmDrawnButtonCallbackStruct *callback;
{
    XColor *xc;
    XEvent *event = callback->event;
    DXmColorMixWidget cmw = 
	(DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(XtParent(w)))));

    if (event == 0)
	return;

    if ((event->type == ButtonPress) && (event->xbutton.button == Button1))
	ScratchPadMB1Press (cmw, w, event);	    

    if ((event->type == ButtonRelease) && (event->xbutton.button == Button1))
	ScratchPadMB1Release (cmw, event);

    if ((event->type == KeyPress) && (callback->reason == XmCR_ARM))
    {
	ColNamedColor(cmw) = NULL;
    	xc = (XColor *) &(ColCurrentSPColor(cmw));
    	DXmColorMixSetNewColor(cmw, xc->red, xc->green, xc->blue);
    }
}


/*---------------------------------------------------*/
/* Handles scratch pad scrolling actions	     */
/*---------------------------------------------------*/

static void ColorMixScratchPadScroll (w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmScrollBarCallbackStruct *callback;
{   
    int i;
    XColor *xc, *current;
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(XtParent(XtParent(XtParent(w))));

    current = (XColor *) &(ColCurrentSPColor(cmw));

    for (i=0, xc = ColScratchColors(cmw); i<callback->value; i++, xc++)
	;   /* Find new top color record */

    current->flags = DoRed | DoGreen | DoBlue;
    current->red   = xc->red;
    current->blue  = xc->blue;
    current->green = xc->green;

    if (ColDispType(cmw) == GryScale)
	SetAGrayColor(cmw, current, ColAllocScratchColor(cmw));
    else
	SetAColor(cmw, current, &(ColAllocScratchColor(cmw)));

    if (ColDispType(cmw) == StatColor)  
    {
    	Arg al[1];
    	XtSetArg(al[0], XmNbackground, current->pixel);
    	XtSetValues(ColSPDrawingAreaWid(cmw), al, 1);
    }
}

   
/*---------------------------------------------------*/
/* resets 'new' color to 'original' color value -    */
/* called whenever the user activates the 'RESET' pb */
/*---------------------------------------------------*/

static void ResetColors(cmw)
    DXmColorMixWidget cmw;
{   
    XColor *xcolors, *ucolors;
    HLSValues hls;
    int i;
    double grey;
    Boolean interp_alloced = TRUE;

    ColNamedColor(cmw) = NULL;

    /* reset the 'new' color in the color display area */

    if (cmw->colormix.setnewcolproc != NULL)
    	(* cmw->colormix.setnewcolproc)
	(cmw, ColOrigColorRed(cmw), ColOrigColorGrn(cmw), ColOrigColorBlu(cmw));


    /* reset the 'new' color in the color mixing tool */

    if (cmw->colormix.setmixcolproc != NULL)
    	(* cmw->colormix.setmixcolproc)
	(cmw, ColOrigColorRed(cmw), ColOrigColorGrn(cmw), ColOrigColorBlu(cmw));

    if (ColDefMixer(cmw))
    {
	/* reset the interpolator and undo colors to white in picker model */

	if ((ColModel(cmw) == DXmColorModelPicker) && 
	    (ColAllocInterpXColors(cmw)) &&
	    (ColIsInterpSensitive(cmw)))
	{
	    for (i=0, xcolors = ColInterpXColors(cmw), ucolors = ColUndoXColors(cmw), 
		 hls = ColInterpHLSValues(cmw);
		 i < ColInterpTileCount(cmw); 
		 i++, xcolors++, ucolors++, hls++)
	    {
		xcolors->red   = ucolors->red   = (unsigned short) MAXCOLORVALUE; /* Initialize */
		xcolors->green = ucolors->green = (unsigned short) MAXCOLORVALUE; /*    to      */
		xcolors->blue  = ucolors->blue  = (unsigned short) MAXCOLORVALUE; /*   white    */	

		hls->hue = 0.0;
		hls->lightness = 100.0;
		hls->saturation = 0.0;

		if (ColDispType(cmw) == GryScale)
		    SetAGrayColor(cmw, xcolors, ColAllocInterpXColors(cmw));
		else
		{    
		    SetAColor(cmw, xcolors, &(ColAllocInterpXColors(cmw)));
		    interp_alloced = (interp_alloced && ColAllocInterpXColors(cmw)); 
		}

		ucolors->pixel = xcolors->pixel;
	    }
	    ColAllocInterpXColors(cmw) = interp_alloced;
	}

	/* Reset scroll window to top of browser list */

	if (ColModel(cmw) == DXmColorModelBrowser)
	{
	    XmScrollBarSetValues 
		(ColBrowserSBWid(cmw), 0, ColBrowserItemCount(cmw), 0, 0, TRUE);
	}

	/* Reset greyscale mixer */

	if (ColModel(cmw) == DXmColorModelGreyscale)
	{
	    SetGreyscaleColors(cmw);
	}
    }
}

                           
/*---------------------------------------------------*/
/* unmanages color display widget and label (if they */
/* exists and are managed).  Called if the display   */
/* device does not have color capabilities or the    */
/* application replaces the default display widget.  */
/*---------------------------------------------------*/
 
static void RemoveColDisplay(cmw)
    DXmColorMixWidget cmw;
{
    if (!ColDefDisp(cmw))
	return;

    if (XtIsManaged(ColCurDispWid(cmw)))
	XtUnmanageChild(ColCurDispWid(cmw));

    if (IsColDispLabWid(cmw))
	if (XtIsManaged(ColDispLabWid(cmw)))
	    XtUnmanageChild(ColDispLabWid(cmw));
}
 
                           
/*---------------------------------------------------*/
/* unmanages scratch pad pushbutton widget from      */
/* main colormix widget.  Called if the display      */
/* device does not have color capabilities.          */
/*---------------------------------------------------*/
 
static void RemoveScratchPad(cmw)
    DXmColorMixWidget cmw;
{
    if (IsColScratchPadPbWid(cmw))
	if (XtIsManaged(ColScratchPadPbWid(cmw)))
	    XtUnmanageChild(ColScratchPadPbWid(cmw));
}
 
   
/*---------------------------------------------------*/
/* this routine is called when any of the colormix   */
/* pushbuttons are activated, indicating that an     */
/* action is requested by the user	     	     */
/*---------------------------------------------------*/

static void ColorMixPBAct(w, tag, callback)
    Widget	  	w;
    XtPointer		tag;
    XmAnyCallbackStruct *callback;
{   
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(w);
	
    if (ColModel(cmw) == DXmColorModelPicker)
	UnhighlightCurrentColor(cmw);
		 
    if (w == ColScratchPadPbWid(cmw))
	XtManageChild (ColSPPopupWid(cmw));
    if (w == ColOkPbWid(cmw))
    	ColorMixCallback(cmw, XmCR_ACTIVATE, callback->event);
    else if (w == ColAppPbWid(cmw))
    {
	/* if RGB model, make sure text fields match curr color */
	if (ColModel(cmw) == DXmColorModelRGB)
	{
	    TextSetColor(ColRedTextWid(cmw), callback->event);
	    TextSetColor(ColGrnTextWid(cmw), callback->event);
	    TextSetColor(ColBluTextWid(cmw), callback->event);
        }
    	ColorMixCallback(cmw, XmCR_APPLY, callback->event);
    }
    else if (w == ColCanPbWid(cmw))
    	ColorMixCallback(cmw, XmCR_CANCEL, callback->event);
    else if (w == ColResPbWid(cmw))
	ResetColors(cmw);
    else if (w == ColHelpPbWid(cmw))
	ColorMixProvideHelp(cmw);
}

   
/*---------------------------------------------------*/
/* this routine is called the user hits the return   */
/* or enter keys while on the default RGB mixer's    */
/* text widgets have focus, indicating that the user */
/* wishes to change either the Red, Green or Blue    */
/* by typing the actual color value  		     */
/*---------------------------------------------------*/

static void TextSetColor(w, event)
    Widget	 w;
    XEvent      *event;
{   
    XmBulletinBoardWidget d = (XmBulletinBoardWidget) XtParent(w);
    DXmColorMixWidget 	cmw = (DXmColorMixWidget) XtParent(d);
    Widget 		s;
    int 		scale_value, newcolor;
    XmString 		value;			 
    long		size,status;

    value = (XmString) DXmCSTextGetString(w);
    newcolor = _DXmCvtCStoI(value, &size, &status);

    if (newcolor < 0 || newcolor > MAXCOLORVALUE)
	return;

    if (w == ColRedTextWid(cmw))
    {
	ColNewColorRed(cmw) = (unsigned short) newcolor;
	s = ColRedSclWid(cmw);
    }
    else if (w == ColGrnTextWid(cmw))
    {
	ColNewColorGrn(cmw) = (unsigned short) newcolor;
	s = ColGrnSclWid(cmw);
    }
    else if (w == ColBluTextWid(cmw))
    {
	ColNewColorBlu(cmw) = (unsigned short) newcolor;
	s = ColBluSclWid(cmw);
    }

    if (cmw->colormix.setnewcolproc != NULL)
        (* cmw->colormix.setnewcolproc)
	(cmw, ColNewColorRed(cmw), ColNewColorGrn(cmw), ColNewColorBlu(cmw));

    scale_value = (unsigned short) ColCvtColorToScaleValue(newcolor);
    ColHold(cmw) = TRUE;
    XmScaleSetValue(s, scale_value);
    ColHold(cmw) = FALSE;

    XmStringFree(value);
}

   
/*---------------------------------------------------*/
/* this routine is called by the default HLS mixer's */
/* scale widgets indicating that the user wishes to  */
/* change either the Red, Green or Blue color value  */
/*---------------------------------------------------*/

static void ColorMixHLSSCAct(w, tag, callback)
    Widget	  	  w;
    XtPointer		  tag;
    XmScaleCallbackStruct *callback;
{   
    XmBulletinBoardWidget d = (XmBulletinBoardWidget) XtParent(w);
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(d);
    int scale_value;

    if (ColHold(cmw))
	return;

    /* scale_value = callback->value; */
    XmScaleGetValue(w, &scale_value);

    if (w == ColHueSclWid(cmw))
    {
	ColNewColorHue(cmw) = (double) scale_value;
    }
    else if (w == ColLightSclWid(cmw))
    {
	ColNewColorLight(cmw) = (double) scale_value/100;
    }
    else if (w == ColSatSclWid(cmw))
    {
	ColNewColorSat(cmw) = (double) scale_value/100;
    }

    ColorMixHLSToRGB(ColNewColorHue(cmw), 
		     ColNewColorLight(cmw), 
		     ColNewColorSat(cmw),
	     	     &ColNewColorRed(cmw), 
		     &ColNewColorGrn(cmw), 
		     &ColNewColorBlu(cmw));

    if (cmw->colormix.setnewcolproc != NULL)
        (* cmw->colormix.setnewcolproc)
	(cmw, ColNewColorRed(cmw), ColNewColorGrn(cmw), ColNewColorBlu(cmw));
}

   
/*---------------------------------------------------*/
/* this routine is called by the default RGB mixer's */
/* scale widgets indicating that the user wishes to  */
/* change either the Red, Green or Blue color value  */
/*---------------------------------------------------*/

static void ColorMixRGBSCAct(w, tag, callback)
    Widget	  	  w;
    XtPointer		  tag;
    XmScaleCallbackStruct *callback;
{   
    XmBulletinBoardWidget d = (XmBulletinBoardWidget) XtParent(w);
    DXmColorMixWidget cmw = (DXmColorMixWidget) XtParent(d);
    int scale_value;
    unsigned short newcolor;
    unsigned char mask;

    if (ColHold(cmw))
	return;

    /* scale_value = callback->value; */
    XmScaleGetValue(w, &scale_value);

    newcolor = (unsigned short) CvtScaleToColorValue(scale_value);

    if (w == ColRedSclWid(cmw))
    {
	ColNewColorRed(cmw) = newcolor;
	mask = RedChanged;
    }
    else if (w == ColGrnSclWid(cmw))
    {
	ColNewColorGrn(cmw) = newcolor;
	mask = GrnChanged;
    }
    else if (w == ColBluSclWid(cmw))
    {
	ColNewColorBlu(cmw) = newcolor;
	mask = BluChanged;
    }

    if (cmw->colormix.setnewcolproc != NULL)
        (* cmw->colormix.setnewcolproc)
	(cmw, ColNewColorRed(cmw), ColNewColorGrn(cmw), ColNewColorBlu(cmw));

/* Motif SCALE bug (yes BUG) - no value change callback
   if user upclicks after moving thumb...

    if (callback->reason == XmCR_VALUE_CHANGED)  */

	UpdateRGBText(cmw, newcolor, mask);
}


/*---------------------------------------------------*/
/* this routine will be called from picker option    */
/* menu when a new palette is selected.  It will     */
/* update the spectrum with the selected palette's   */
/* colors.					     */
/*---------------------------------------------------*/

static void ColorMixPickerMenuCallback(m, tag, callback)
    XmRowColumnWidget             m;
    int                    tag;
    XmRowColumnCallbackStruct *callback;
{
    XColor *xc;
    unsigned short *color;
    Widget pb = (Widget) callback->widget;
    Widget shell = (Widget) XtParent (m);
    DXmColorMixWidget cmw;
    int i;

    cmw = (DXmColorMixWidget) XtParent(XtParent (shell));

    UnhighlightCurrentColor(cmw);

    if (pb == ColUserPalettePbWid(cmw))
	color = (unsigned short *) ColPickerColors(cmw);
    else if (pb == ColSpectrumPbWid(cmw))
	color = (unsigned short *) default_spectrum_colors;
    else if (pb == ColPastelPbWid(cmw))
	color = (unsigned short *) default_pastel_colors;
    else if (pb == ColMetallicPbWid(cmw))
	color = (unsigned short *) default_metallic_colors;
    else if (pb == ColEarthtonePbWid(cmw))
	color = (unsigned short *) default_earthtone_colors;

    xc = ColPickerXColors (cmw);

    for (i=0; i<ColPickerColorCount(cmw); i++) /* Compute new color */
    {
	xc->red =   (unsigned short) (*color); color++;
	xc->green = (unsigned short) (*color); color++;
	xc->blue =  (unsigned short) (*color); color++;
	xc->flags = DoRed | DoGreen | DoBlue;

	if (ColDispType(cmw) == GryScale)
	    SetAGrayColor(cmw, xc, ColAllocPickerXColors(cmw));
	else
	    SetAColor(cmw, xc, &(ColAllocPickerXColors(cmw)));

	xc++;
    }

    if (ColDispType(cmw) == StatColor)  
	ColorMixPickerRedraw (cmw, 0, ColPickerColorCount(cmw));
}


/*---------------------------------------------------*/
/* this routine will be called from the listbox menu */
/* when a gadget is activated (selected) by the user */
/*---------------------------------------------------*/

static void ColorMixMenuCallback(m, tag, callback)
    XmRowColumnWidget             m;
    int                    tag;
    XmRowColumnCallbackStruct *callback;
{
    Widget pb;
    Widget shell;
    DXmColorMixWidget cmw;
    Boolean change = FALSE;

    pb    = (Widget) callback->widget;
    shell = (Widget) XtParent (m);
    cmw   = (DXmColorMixWidget) XtParent (shell);

    if (ColModel(cmw) == DXmColorModelPicker)
	UnhighlightCurrentColor(cmw);

    if (pb == ColPDHLSWid(cmw))
    {
	if (ColModel(cmw) != DXmColorModelHLS)
	{
    	    ColModel(cmw) = DXmColorModelHLS;
	    change = TRUE;
	}
    }

    else if (pb == ColPDRGBWid(cmw))
    {
	if (ColModel(cmw) != DXmColorModelRGB)
	{
    	    ColModel(cmw) = DXmColorModelRGB;
	    change = TRUE;
	}
    }

    else if (pb == ColPDBrowserWid(cmw))
    {
	if (ColModel(cmw) != DXmColorModelBrowser)
	{
    	    ColModel(cmw) = DXmColorModelBrowser;
	    change = TRUE;
	}
    }

    else if (pb == ColPDGreyscaleWid(cmw))
    {
	if (ColModel(cmw) != DXmColorModelGreyscale)
	{
    	    ColModel(cmw) = DXmColorModelGreyscale;
	    change = TRUE;
	}
    }

    else
    {
	if (ColModel(cmw) != DXmColorModelPicker)
	{
    	    ColModel(cmw) = DXmColorModelPicker;
	    change = TRUE;
	}
    }

    if (change)
	UpdateMixer(cmw);
}


#ifndef WIN32
/* Remove the above conditional when HyperHelp makes it to Unix */

/*---------------------------------------------------*/
/* this routine is called when the user requests help*/
/* on the colormix widget either by activating the   */
/* "help" pushbutton or context-based help anywhere  */
/* inside the colormix widget or its children.       */
/*---------------------------------------------------*/

static void ColorMixProvideHelp(cmw)
    DXmColorMixWidget cmw;
{
    if (help_context == (Opaque) NULL)
	DXmHelpSystemOpen (&help_context, cmw, 
	     HELP_LIBRARY, ColorMixHelpError, (Opaque) NULL);    

    DXmHelpSystemDisplay (help_context, HELP_LIBRARY,"topic", 
			  "overview", ColorMixHelpError, (Opaque) NULL);
     
}

#else
static void ColorMixProvideHelp(cmw)
    DXmColorMixWidget cmw;
{
    Arg al[5];
    int ac = 0;
    XmString lib, topic;
    Widget shell;
#ifndef WIN32
/* Turned of until help widget is ported to NT */
    /* 
     * if help widget already created, just set topic and make visible;
     * else create and set library...
     */
    if (IsColHelpWid(cmw))
    {
	topic = XmStringLtoRCreate("overview", "ISO8859-1");
    	shell = XtParent(ColHelpWid(cmw));

	XtSetArg(al[ac], DXmNfirstTopic, topic); ac++;
	XtSetValues(ColHelpWid(cmw), al, ac);

	XmStringFree(topic);

	if (XtIsManaged(ColHelpWid(cmw)))
    	    XRaiseWindow(XtDisplay(shell), XtWindow(shell));
	else
	    XtManageChild(ColHelpWid(cmw));
    }
    else
    {
    	lib = XmStringLtoRCreate(HELP_LIBRARY, "ISO8859-1");

    	XtSetArg (al[ac], DXmNlibrarySpec, lib); 		ac++;
    	XtSetArg (al[ac], DXmNlibraryType, DXmTextLibrary); 	ac++;
    	XtSetArg (al[ac], DXmNapplicationName, cmw->bulletin_board.dialog_title); 
    	ac++;

    	ColHelpWid(cmw) = DXmCreateHelp ((Widget) cmw, "colormix help", al, ac);

	XmStringFree(lib);
		
    	XtManageChild(ColHelpWid(cmw));
    }
#endif
}
#endif



/* 
 * These will probably become part of a public include file eventually,
 * remove these definitions when they do.
 */

#define Bkr_Success                 0
#define Bkr_Busy                    1
#define Bkr_Send_Event_Failure      2
#define Bkr_Startup_Failure         3
#define Bkr_Create_Client_Failure   4
#define Bkr_Invalid_Object          5
#define Bkr_Get_Data_Failure        6
#define Bkr_Bad_Filename            7

/* 
 * This routine is called whenever one of the HyperHelp routines returns
 * a failure status.
 */

static void ColorMixHelpError (problem_string, status)
    char    *problem_string;               
    int     status;

{
        switch (status)
        {
            case Bkr_Send_Event_Failure:
		printf("HyperHelp error: Failure sending event to bookreader, status code %d\n", status);
		break;

            case Bkr_Startup_Failure:
		printf("HyperHelp error: Failed to start bookreader, status code %d\n", status);
		break;

            case Bkr_Create_Client_Failure:
		printf("HyperHelp error: Can't create client context, status code %d\n", status);
		break;

            case Bkr_Invalid_Object:
		printf("HyperHelp error: Invalid object passed to help routines, status code %d\n", status);
		break;

            case Bkr_Get_Data_Failure:
		printf("HyperHelp error: Can't retrieve information from help context, status code %d\n", status);
		break;

            case Bkr_Bad_Filename:
		printf("HyperHelp error: Bad filename passed to DXmHelpSystemOpen, status code %d\n", status);
		break;
	}
}

   
/*---------------------------------------------------*/
/* this routine is called when help is requested on  */
/* a subwidget of the colormix widget.               */
/*---------------------------------------------------*/

static void ColorMixHelp(w, tag, callback)
    Widget		  w;
    XtPointer		  tag;
    XmAnyCallbackStruct *callback;
{
    Boolean found_cmw = FALSE;
    Widget pw = w;

    while (!found_cmw && pw != (Widget) NULL)
    {
	if (XtIsSubclass(pw, dxmColorMixWidgetClass))
	    found_cmw = TRUE;
	else
	    pw = XtParent(pw);
    }

   if (found_cmw)
	ColorMixProvideHelp(pw);
}



/*---------------------------------------------------*/
/*  ColorMix public entry points		     */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/*  DXmColorMixSetNewColor			     */
/*						     */
/*  sets current new color to RGB values	     */
/*---------------------------------------------------*/

#ifdef _NO_PROTO

void DXmColorMixSetNewColor(cmw, red, green, blue)
    DXmColorMixWidget cmw;
    unsigned short red, green, blue;

#else /* _NO_PROTO undefined */

extern void DXmColorMixSetNewColor ( DXmColorMixWidget cmw , unsigned short red , unsigned short green , unsigned short blue )

#endif /* _NO_PROTO undefined */
{   
    ColNewColorRed(cmw) = red;
    ColNewColorGrn(cmw) = green;
    ColNewColorBlu(cmw) = blue;

    /* set the 'new' color in the color display window */

    if (cmw->colormix.setnewcolproc != NULL)
        (* cmw->colormix.setnewcolproc)
	(cmw, ColNewColorRed(cmw), ColNewColorGrn(cmw), ColNewColorBlu(cmw));

    /* set the 'new' color in the color mixing tool */

    if (cmw->colormix.setmixcolproc != NULL)
        (* cmw->colormix.setmixcolproc)
	(cmw, ColNewColorRed(cmw), ColNewColorGrn(cmw), ColNewColorBlu(cmw));

    if (ColModel(cmw) == DXmColorModelGreyscale)
	SetGreyscaleColors(cmw);

}

/*---------------------------------------------------*/
/*  DXmColorMixGetNewColor			     */
/*						     */
/*  returns current new color to RGB values	     */
/*---------------------------------------------------*/

#ifdef _NO_PROTO

void DXmColorMixGetNewColor(cmw, red, green, blue)
    DXmColorMixWidget cmw;
    unsigned short *red, *green, *blue;

#else /* _NO_PROTO undefined */

extern void DXmColorMixGetNewColor ( DXmColorMixWidget cmw , unsigned short *red , unsigned short *green , unsigned short *blue )

#endif /* _NO_PROTO undefined */
{   
    *red   = ColNewColorRed(cmw);
    *green = ColNewColorGrn(cmw);
    *blue  = ColNewColorBlu(cmw);
}



/*---------------------------------------------------*/
/*  DXmCreateColorMix				     */
/*						     */
/*  creates and returns a ColorMix widget	     */
/*---------------------------------------------------*/

#ifdef _NO_PROTO

Widget DXmCreateColorMix (p, name, args, ac)
    Widget	p;		/*  parent widgetb */
    String	name;		/*  widget name	   */
    ArgList	args;		/*  arg list	   */
    Cardinal	ac;	        /*  arg count	   */

#else /* _NO_PROTO undefined */

extern Widget DXmCreateColorMix ( Widget p , String name , ArgList args , Cardinal ac )

#endif /* _NO_PROTO undefined */
{
    DXmColorMixWidget cmw;

    cmw = (DXmColorMixWidget) XtCreateWidget 
				    (name, dxmColorMixWidgetClass, p, args, ac);
    ColIsPopup(cmw) = FALSE;

    return (Widget) (cmw);
}


/*---------------------------------------------------*/
/* DXmCreateColorMixDialog                           */
/*						     */
/* creates a DialogShell and a ColorMix child of the */
/* shell; returns the ColorMix widget                */
/*---------------------------------------------------*/

#ifdef _NO_PROTO

Widget DXmCreateColorMixDialog (ds_p, name, args, ac)
    Widget	ds_p;		/*  parent for shell	*/
    String	name;		/*  widget name		*/
    ArgList	args;		/*  al for colormix     */
    Cardinal	ac;		/*  ac for sb	        */

#else /* _NO_PROTO undefined */

extern Widget DXmCreateColorMixDialog ( Widget ds_p , String name , ArgList args , Cardinal ac )

#endif /* _NO_PROTO undefined */
{
	Widget		ds;		/*  DialogShell		*/
	char           *ds_name;        /*  DialogShell name    */
    	Arg		_args[1];	/*  local al */
	DXmColorMixWidget cmw;		/*  new colormix widget	*/

	/* create DialogShell parent */

	ds_name = XtCalloc(strlen(name)+XmDIALOG_SUFFIX_SIZE+1,sizeof(char));
	strcpy(ds_name,name);
	strcat(ds_name,XmDIALOG_SUFFIX);

	XtSetArg (_args[0], XmNallowShellResize, True);
	ds = XmCreateDialogShell (ds_p, ds_name, _args, 1);

	XtFree(ds_name);

	/* create ColorMix */

	cmw = (DXmColorMixWidget) XtCreateWidget 
				 (name, dxmColorMixWidgetClass, ds, args, ac);

	XtAddCallback ((Widget)cmw, XmNdestroyCallback, _XmDestroyParentCallback, NULL);

        ColIsPopup(cmw) = TRUE;

	return (Widget) (cmw);
}
