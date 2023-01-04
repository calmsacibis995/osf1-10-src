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
                                                          
#ifndef _DXmColorMixP_h
#define _DXmColorMixP_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:DXmColor.h>
#include <DECW$INCLUDE:XmP.h>
#else
#include <DXm/DXmColor.h>
#include <Xm/XmP.h>
#endif


/* Record type used by browser */

typedef struct _BrowserColorRec
{
    char	    *name;	/* X11 color name */
    XmString	    string;	/* String to display in button */
    unsigned short  red;
    unsigned short  green;	/* Color values returned by server */
    unsigned short  blue;
    Boolean	    dark_fg;	/* Use black fg if TRUE, white if not */
}
    BrowserColorRec, *BrowserColor;

/* Record used to hold HLS values of interpolator tiles */

typedef struct _HLSValuesRec
{
    double	    hue;
    double	    lightness;
    double	    saturation;
}
    HLSValuesRec, *HLSValues;

typedef struct _ColDNDContextRec
{
    Widget w;
    Position x;
    Position y;
    Widget dragContext;
}
    ColDNDContextRec, *ColDNDContext;


typedef Widget	*WidgetPtr;
typedef void    (*DXmColorSetProc) (
#ifndef _NO_PROTO
    DXmColorMixWidget,
    unsigned short, unsigned short, unsigned short 
#endif
	);


/*  fields for the ColorMix widget class record  */

typedef struct
{
    XtPointer		extension;      /* Pointer to extension record */
} DXmColorMixClassPart;


/* Full class record declaration */

typedef struct _DXmColorMixClassRec
{
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    XmBulletinBoardClassPart	bulletin_board_class;
    DXmColorMixClassPart	color_mix_class;
} DXmColorMixClassRec;

externalref DXmColorMixClassRec dxmColorMixClassRec;

/* fields for the ColorMix widget record */

typedef struct
{
    XmString     	mainlabel,	/* comp strings for widget labels */
			displabel,
			mixlabel,
			redlabel,
			grnlabel,
			blulabel,
			sldlabel,
			vallabel,
			applabel,
			oklabel,
			reslabel,
			canlabel,
			huelabel,
			lightlabel,
			satlabel,
			blklabel,
			whtlabel,
			grylabel,
			fullabel,
			optlabel,
			rgblabel,
			hlslabel,
			pickerlabel,
			undolabel,
			smearlabel,
			helplabel,
#ifndef NO_WARM
			warmerlabel,
			coolerlabel,
			darkerlabel,
			lighterlabel,
#endif
			ptitlelabel,
			spectrumlabel,
			pastellabel,
			metalliclabel,
			earthtonelabel,
			userpalettelabel,
			ititlelabel,
			clearlabel,
			spinfolabel,
			splabel,
			browserlabel,
			greyscalelabel;
    Widget		mainlabelwid,	/* label widget for 'main' label */
                        displabelwid,
			mixlabelwid, 	
			sldlabelwid,
			vallabelwid,
			redlabelwid,
			grnlabelwid,
			blulabelwid,
			huelabelwid,  	
			lightlabelwid,  	
			satlabelwid,  	
			blklabelwid,  	
			whtlabelwid,  	
			grylabelwid,  	
			fullabelwid,  	
			optlabelwid,  	
    	 		apppbwid,  	/* apply pushbutton widget id */
			okpbwid,   	/* ok pushbutton widget id */
			respbwid,   	/* reset pushbutton widget id */
			canpbwid,       /* cancel pushbutton widget id */
			helppbwid,	/* help pushbutton widget id */
			pdhlswid,  	/* "HLS" pull down entry widget id */
			pdrgbwid,  	/* "RGB" pull down entry widget id */
			pdpickerwid,	/* "Picker" pull down entry widget id */
    			redsclwid,      /* default color mixer red scale */
			grnsclwid,      /* default color mixer green scale */
			blusclwid,      /* default color mixer blue scale */
			huesclwid,      /* default color mixer hue scale */
			lightsclwid,    /* default color mixer lightness scale */
			satsclwid,      /* default color mixer saturation scale */
    			redtextwid,     /* red RGB value text entry field */
			grntextwid,     /* green RGB value text entry field */
			blutextwid,     /* blue RGB value text entry field */
			pickerdawid,	/* picker drawing area */
			interpdawid,	/* interpolater drawing area */
			pickerframewid, /* frame around picker area */
			interpframewid, /* frame around interpolator area */
			undopbwid,	/* picker undo button */
			smearpbwid,	/* picker smear button */
			lbucketpbwid,	/* left paint bucket button */
			rbucketpbwid,	/* right paint bucket button */
			ptitlelabelwid, /* picker title label widget */    
			ititlelabelwid, /* interpolator title label widget */    
			warmerlabelwid, /* label for warmth button */
			coolerlabelwid, /* label for coolness button */
			darkerlabelwid, /* label for darkness button */
			lighterlabelwid,/* label for lightness button */ 
			warmerpbwid,	/* warmth button */
			coolerpbwid,	/* coolness button */
			darkerpbwid,	/* darkness button */
			lighterpbwid,	/* lightness button */
			spectrumpbwid,	  /*    buttons   */
			pastelpbwid,	  /*     for      */
			metallicpbwid,	  /*    picker    */
			earthtonepbwid,	  /*    option    */
			userpalettepbwid, /*     menu     */
			pickeroptmenwid,  /* picker option menu */
			pickerpdwid,	  /* picker pulldown menu */
			sppbwid,	/* scratch pad button */
			sppopupwid,	/* scratch pad popup window */
			spinfolabelwid, /* scratch pad informative label */
			spclearpbwid,   /* scratch pad clear button */
			spcancelpbwid,	/* scratch pad cancel button */
			spswwid,	/* scratch pad scrolled window */
			spsbwid,	/* scratch pad scroll bar */
			spframewid,	/* scratch pad frame widget */
			spdawid,	/* scratch pad drawing area widget */
			spbucketpbwid,	/* scratch pad bucket button */
    			origwid,        /* original color view window widget */
    	 		newwid,      	/* new color view window widget id */
    	 		dispwid,   	/* color display widget id */
			mixerwid,     	/* color mixer widget id */
			hlsmixerwid,   	/* hls mixer widget id */
			rgbmixerwid,    /* rgb mixer widget id */
			pickermixerwid, /* picker mixer widget id */
    			curmixerwid,    /* color mixing/specification widget */
			curdispwid,     /* color display widget */
			workwid,     	/* optional application work area */
    			optmenwid,
			pdmenwid,
			pdbrowserwid,
			browsermixerwid,
			browserswwid,
			browserbbwid,
			browsersbwid,
			pdgreyscalewid,
			greyscalemixerwid,
			greyscaleindlabelwid,
			greyscalescalewid,
			helpwid;        /* help widget id */
    WidgetPtr		browserpbwid;
    XtCallbackList	okcallback,	/* OK callback */
			applycallback,  /* Cancel callback */
			cancelcallback; /* Apply callback */
    XColor  		backcolor,      /* background color of display window */
			origcolor,      /* original color specified by appl */
			newcolor,       /* color mixed by user */
			currentspcolor, /* currently visible scratch pad color */
			*grabcolor,	/* color grabbed with eyedropper */
			*pxcolors,	/* colors used by picker */ 
			*ixcolors,	/* colors used by interpolator */
			*undocolors,	/* last saved interpolator colors */
			*scratchcolors, /* colors used by scratch pad */
			*bxcolors,	/* colors used by the browser */
			*hlcolor;	/* currently highlighted color */
    HLSValues		ihlsvalues;	/* HLS interpolator tile values */
    DXmColorSetProc	setnewcolproc,	/* file search routine */
			setmixcolproc;  /* reset colors routine */
    double		hue;
    double		light;
    double		sat;
    short		disptype;       /* either b&w, gray scale, color, etc */
    short		textcols;	/* text columns in def RGB mixer */
    Dimension		vieww;		/* color display sub-window width */
    Dimension		viewh;		/* color display sub-window height */
    Dimension		viewm;		/* color display window margin */
    Boolean		matchcolors;    /* if true, orig & new colors matched */
    Boolean		defdisp; 	/* true if default display widget */
    Boolean		defmixer;	/* true if default mixer widget */
    Boolean		allocorigcolor;	/* true if allocated original color*/
    Boolean		allocnewcolor;	/* true if allocated new color */
    Boolean		allocbackcolor;	/* true if allocated back grnd color */
    Boolean		allocpickercolors; /* true if allocated picker colors */
    Boolean		allocinterpcolors; /* true if allocated interpolator colors */
    Boolean		allocscratchcolor; /* true if allocated scratch pad color */
    Boolean		hold;		  /* true if color is 'frozen' */
    Boolean		nolayout;         /* if true, do not layout widgets */
    Boolean		isgrabbed;        /* if true, pointer is grabbed by picker */
    Boolean		ispickerselected; /* true if picker tile selected */
    Boolean		isinterpselected; /* true if interpolator tile selected */
    Boolean		isorigselected;	  /* true if orig tile is selected */
    Boolean		isnewselected;	  /* true if new tile is selected */
    Boolean		isspselected;	  /* true if scratch pad tile is selected */
    Boolean		greyongrey;	/* true if gs is default on gs systems */
    Boolean		isinterpsens;	/* true if interpolator is sensitive */
    Boolean		isspmanaged;	/* true if scratch pad should be managed */
    BrowserColor	namedcolor;	/* set when new color is from browser */
    int			pselecttile;	/* selected picker tile */
    int			iselecttile;	/* selected interpolator tile */
    int			scratchcount;	/* number of scratch pad colors */
    unsigned char	colormodel;	/* what colormodel is specified */
    short		ptileheight;	/* height of picker tiles */
    short		ptilewidth;	/* width of picker tiles */
    unsigned short     *pcolors;	/* picker color array */
    short		pcolorcount;	/* number of picker colors */
    XtIntervalId	timer;		/* id of current timeout */
    int			timereventx;	/* x of mouse event connected to timeout */
    unsigned long    	timerinterval;	/* interval for MB1 hold on tiles */
    Cursor		eyedropper;	/* eyedropper cursor used by picker */
    Pixmap		gs_pixmap;	/* background for greyscale scale */
#ifndef NO_WARM
    unsigned short	warmthinc;	/* color increment for warmth buttons */
    short		lightnessinc;	/* color increment for lightness buttons */
#endif
    short		itileheight;	/* height of interpolator tiles */
    short		itilewidth;	/* width of interpolator tiles */
    short		itilecount;	/* number of actual interpolator tiles */
    short		irealtilecount;	/* number of requested interpolator tiles */
    Boolean		allocbrowsercolors; /* true if alloced browser colors */
    Boolean		initbrowsercolors; /* true if colors are initialized */
    BrowserColor	bcolors;	/* X11 browser colors */
    short		bcolorcount;	/* total number of browser colors */
    short		bitemcount;	/* number of visible browser items */
    Boolean		ispopup;        /* true if popup widget */
    Widget		dragIcon;       /* eyedropper drag icon widget */
    int			lastinterpside; /* side of the interpolator last */
					/* highlighted during a drag     */
    Widget		drophelpdialog; /* drop site help dialog */
    XmString     	drophelptilelabel;   /* Strings displayed in dialog box when */
    XmString	        drophelpinterplabel; /* drop site help is requested	     */
} DXmColorMixPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _DXmColorMixRec
{
    CorePart			core;
    CompositePart		composite;
    ConstraintPart		constraint;
    XmManagerPart		manager;
    XmBulletinBoardPart		bulletin_board;
    DXmColorMixPart 		colormix;
} DXmColorMixRec;


#ifndef DXmIsColorMix
#define DXmIsColorMix(w)  (XtIsSubclass (w, dxmColorMixWidgetClass))
#endif


/* private constants */

#define BlackAndWhite		0
#define StatGray		1
#define GryScale		2
#define StatColor		3
#define DynColor		4

#define RedChanged		1
#define GrnChanged		2
#define BluChanged		3

#define	ARG_LIST_CNT		(25)
#define MAXCOLORVALUE		65535
#define HALFCOLORVALUE		32767
#define NOCOLORVALUE 		70000
#define COLLAB			1
#define COLPB			2


/* computation macros */

#define Double(d)	((d) << 1)
#define TotalWidth(w)   (XtWidth  (w) + Double (XtBorderWidth (w)))
#define TotalHeight(w)  (XtHeight (w) + Double (XtBorderWidth (w)))


/*  access macros  */

#define ColDirection(cmw) 	((cmw)->manager.string_direction)
#define ColLayoutDirection(cmw) ((cmw)->manager.dxm_layout_direction)
#define ColButtonFontList(cmw) 	((cmw)->bulletin_board.button_font_list)
#define ColLabelFontList(cmw) 	((cmw)->bulletin_board.label_font_list)
#define ColTextFontList(cmw) 	((cmw)->bulletin_board.text_font_list)
#define ColNoLayout(cmw)	((cmw)->colormix.nolayout)
#define ColDefDisp(cmw)		((cmw)->colormix.defdisp)
#define ColDefMixer(cmw)	((cmw)->colormix.defmixer)
#define ColTextCols(cmw)	((cmw)->colormix.textcols)
#define ColDispViewW(cmw)	((cmw)->colormix.vieww)
#define ColDispViewH(cmw)	((cmw)->colormix.viewh)
#define ColDispViewM(cmw)	((cmw)->colormix.viewm)
#define ColBackColor(cmw)	((cmw)->colormix.backcolor)
#define ColAllocBackColor(cmw)	((cmw)->colormix.allocbackcolor)
#define ColBackColorPix(cmw)	((cmw)->colormix.backcolor.pixel)
#define ColBackColorFlg(cmw)	((cmw)->colormix.backcolor.flags)
#define ColBackColorRed(cmw)	((cmw)->colormix.backcolor.red)
#define ColBackColorGrn(cmw)	((cmw)->colormix.backcolor.green)
#define ColBackColorBlu(cmw)	((cmw)->colormix.backcolor.blue)
#define ColOrigColor(cmw)	((cmw)->colormix.origcolor)
#define ColAllocOrigColor(cmw)	((cmw)->colormix.allocorigcolor)
#define ColOrigColorPix(cmw)	((cmw)->colormix.origcolor.pixel)
#define ColOrigColorFlg(cmw)	((cmw)->colormix.origcolor.flags)
#define ColOrigColorRed(cmw)	((cmw)->colormix.origcolor.red)
#define ColOrigColorGrn(cmw)	((cmw)->colormix.origcolor.green)
#define ColOrigColorBlu(cmw)	((cmw)->colormix.origcolor.blue)
#define ColNewColor(cmw)	((cmw)->colormix.newcolor)
#define ColAllocNewColor(cmw)	((cmw)->colormix.allocnewcolor)
#define ColNewColorPix(cmw)	((cmw)->colormix.newcolor.pixel)
#define ColNewColorFlg(cmw)	((cmw)->colormix.newcolor.flags)
#define ColNewColorRed(cmw)	((cmw)->colormix.newcolor.red)
#define ColNewColorGrn(cmw)	((cmw)->colormix.newcolor.green)
#define ColNewColorBlu(cmw)	((cmw)->colormix.newcolor.blue)
#define ColNewColorHue(cmw)	((cmw)->colormix.hue)
#define ColNewColorLight(cmw)	((cmw)->colormix.light)
#define ColNewColorSat(cmw)	((cmw)->colormix.sat)
#define ColDispType(cmw)	((cmw)->colormix.disptype)
#define ColMatchColors(cmw)	((cmw)->colormix.matchcolors)
#define ColHold(cmw)		((cmw)->colormix.hold)
#define ColModel(cmw)		((cmw)->colormix.colormodel)
#define ColPickerTileHeight(cmw)    ((cmw)->colormix.ptileheight)
#define ColPickerTileWidth(cmw)	    ((cmw)->colormix.ptilewidth)
#define ColPickerColors(cmw)	    ((cmw)->colormix.pcolors)
#define ColPickerColorCount(cmw)    ((cmw)->colormix.pcolorcount)
#define ColAllocPickerXColors(cmw)  ((cmw)->colormix.allocpickercolors)
#define ColPickerXColors(cmw)	    ((cmw)->colormix.pxcolors)
#define ColGrabColor(cmw)	    ((cmw)->colormix.grabcolor)
#define ColTimerInterval(cmw)	    ((cmw)->colormix.timerinterval)
#define ColTimer(cmw)		    ((cmw)->colormix.timer)
#define ColTimerEventX(cmw)	    ((cmw)->colormix.timereventx)
#define ColPickerCursor(cmw)	    ((cmw)->colormix.eyedropper)
#define ColInterpTileHeight(cmw)    ((cmw)->colormix.itileheight)
#define ColInterpTileWidth(cmw)	    ((cmw)->colormix.itilewidth)
#define ColInterpTileCount(cmw)	    ((cmw)->colormix.itilecount)
#define ColRealInterpTileCount(cmw) ((cmw)->colormix.irealtilecount)
#define ColAllocInterpXColors(cmw)  ((cmw)->colormix.allocinterpcolors)
#define ColInterpXColors(cmw)	    ((cmw)->colormix.ixcolors)
#define ColUndoXColors(cmw)	    ((cmw)->colormix.undocolors)
#define ColInterpHLSValues(cmw)	    ((cmw)->colormix.ihlsvalues)
#define ColIsPointerGrabbed(cmw)    ((cmw)->colormix.isgrabbed)
#define ColWarmthIncrement(cmw)	    ((cmw)->colormix.warmthinc)
#define ColLightnessIncrement(cmw)  ((cmw)->colormix.lightnessinc)
#define ColBrowserXColors(cmw)	    ((cmw)->colormix.bxcolors)
#define ColBrowserColors(cmw)	    ((cmw)->colormix.bcolors)
#define ColAllocBrowserXColors(cmw)  ((cmw)->colormix.allocbrowsercolors)
#define ColInitBrowserColors(cmw)   ((cmw)->colormix.initbrowsercolors)
#define ColBrowserColorCount(cmw)   ((cmw)->colormix.bcolorcount)
#define ColBrowserItemCount(cmw)    ((cmw)->colormix.bitemcount)
#define ColGreyscalePixmap(cmw)     ((cmw)->colormix.gs_pixmap)
#define ColHighlightedColor(cmw)    ((cmw)->colormix.hlcolor)
#define ColIsPickerSelected(cmw)    ((cmw)->colormix.ispickerselected)
#define ColIsInterpSelected(cmw)    ((cmw)->colormix.isinterpselected)
#define ColIsOrigSelected(cmw)      ((cmw)->colormix.isorigselected)
#define ColIsNewSelected(cmw)       ((cmw)->colormix.isnewselected)
#define ColIsScratchPadSelected(cmw) ((cmw)->colormix.isspselected)
#define ColPickerSelectTile(cmw)    ((cmw)->colormix.pselecttile)
#define ColInterpSelectTile(cmw)    ((cmw)->colormix.iselecttile)
#define ColGreyOnGrey(cmw)          ((cmw)->colormix.greyongrey)
#define ColIsInterpSensitive(cmw)   ((cmw)->colormix.isinterpsens)
#define ColNamedColor(cmw)	    ((cmw)->colormix.namedcolor)
#define ColCurrentSPColor(cmw)	    ((cmw)->colormix.currentspcolor)
#define ColCurrentSPPix(cmw)	    ((cmw)->colormix.currentspcolor.pixel)
#define ColScratchCount(cmw)	    ((cmw)->colormix.scratchcount)
#define ColScratchColors(cmw)	    ((cmw)->colormix.scratchcolors)
#define ColIsScratchPadManaged(cmw) ((cmw)->colormix.isspmanaged)
#define ColAllocScratchColor(cmw)   ((cmw)->colormix.allocscratchcolor)
#define ColIsPopup(cmw) 	    ((cmw)->colormix.ispopup)

#define ColMainLab(cmw)		((cmw)->colormix.mainlabel)
#define ColDispLab(cmw)		((cmw)->colormix.displabel)
#define ColMixLab(cmw)		((cmw)->colormix.mixlabel)
#define ColRedLab(cmw)		((cmw)->colormix.redlabel)
#define ColGrnLab(cmw)		((cmw)->colormix.grnlabel)
#define ColBluLab(cmw)		((cmw)->colormix.blulabel)
#define ColSldLab(cmw)		((cmw)->colormix.sldlabel)
#define ColValLab(cmw)		((cmw)->colormix.vallabel)
#define ColOkLab(cmw)		((cmw)->colormix.oklabel)
#define ColAppLab(cmw)		((cmw)->colormix.applabel)
#define ColResLab(cmw)		((cmw)->colormix.reslabel)
#define ColCanLab(cmw)		((cmw)->colormix.canlabel)
#define ColHueLab(cmw)		((cmw)->colormix.huelabel)
#define ColLightLab(cmw)	((cmw)->colormix.lightlabel)
#define ColSatLab(cmw)		((cmw)->colormix.satlabel)
#define ColBlkLab(cmw)		((cmw)->colormix.blklabel)
#define ColWhtLab(cmw)		((cmw)->colormix.whtlabel)
#define ColGryLab(cmw)		((cmw)->colormix.grylabel)
#define ColFulLab(cmw)		((cmw)->colormix.fullabel)
#define ColPDLab(cmw)		((cmw)->colormix.optlabel)
#define ColPDHLSLab(cmw)	((cmw)->colormix.hlslabel)
#define ColPDRGBLab(cmw)	((cmw)->colormix.rgblabel)
#define ColPDPickerLab(cmw)	((cmw)->colormix.pickerlabel)
#define ColUndoLab(cmw)		((cmw)->colormix.undolabel)
#define ColSmearLab(cmw)	((cmw)->colormix.smearlabel)
#define ColHelpLab(cmw)		((cmw)->colormix.helplabel)
#define ColPTitleLab(cmw)	((cmw)->colormix.ptitlelabel)
#define ColSpectrumLab(cmw)	((cmw)->colormix.spectrumlabel)
#define ColPastelLab(cmw)	((cmw)->colormix.pastellabel)
#define ColMetallicLab(cmw)	((cmw)->colormix.metalliclabel)
#define ColEarthtoneLab(cmw)	((cmw)->colormix.earthtonelabel)
#define ColUserPaletteLab(cmw)	((cmw)->colormix.userpalettelabel)
#define ColITitleLab(cmw)	((cmw)->colormix.ititlelabel)
#define ColWarmerLab(cmw)	((cmw)->colormix.warmerlabel)
#define ColCoolerLab(cmw)	((cmw)->colormix.coolerlabel)
#define ColDarkerLab(cmw)	((cmw)->colormix.darkerlabel)
#define ColLighterLab(cmw)	((cmw)->colormix.lighterlabel)
#define ColClearLab(cmw)	((cmw)->colormix.clearlabel)
#define ColSPInfoLab(cmw)	((cmw)->colormix.spinfolabel)
#define ColScratchPadLab(cmw)	((cmw)->colormix.splabel)
#define ColPDBrowserLab(cmw)	((cmw)->colormix.browserlabel)
#define ColPDGreyscaleLab(cmw)	((cmw)->colormix.greyscalelabel)


#define ColMainLabWid(cmw)	((cmw)->colormix.mainlabelwid)
#define ColDispLabWid(cmw)	((cmw)->colormix.displabelwid)
#define ColMixLabWid(cmw)	((cmw)->colormix.mixlabelwid)
#define ColBlkLabWid(cmw)	((cmw)->colormix.blklabelwid)
#define ColWhtLabWid(cmw)	((cmw)->colormix.whtlabelwid)
#define ColGryLabWid(cmw)	((cmw)->colormix.grylabelwid)
#define ColFulLabWid(cmw)	((cmw)->colormix.fullabelwid)
#define ColRedLabWid(cmw)	((cmw)->colormix.redlabelwid)
#define ColGrnLabWid(cmw)	((cmw)->colormix.grnlabelwid)
#define ColBluLabWid(cmw)	((cmw)->colormix.blulabelwid)
#define ColHueLabWid(cmw)	((cmw)->colormix.huelabelwid)
#define ColLightLabWid(cmw)	((cmw)->colormix.lightlabelwid)
#define ColSatLabWid(cmw)	((cmw)->colormix.satlabelwid)
#define ColOptLabWid(cmw)	((cmw)->colormix.optlabelwid)
#define ColRedSclWid(cmw)	((cmw)->colormix.redsclwid)
#define ColGrnSclWid(cmw)	((cmw)->colormix.grnsclwid)
#define ColBluSclWid(cmw)	((cmw)->colormix.blusclwid)
#define ColHueSclWid(cmw)	((cmw)->colormix.huesclwid)
#define ColLightSclWid(cmw)	((cmw)->colormix.lightsclwid)
#define ColSatSclWid(cmw)	((cmw)->colormix.satsclwid)
#define ColRedTextWid(cmw)	((cmw)->colormix.redtextwid)
#define ColGrnTextWid(cmw)	((cmw)->colormix.grntextwid)
#define ColBluTextWid(cmw)	((cmw)->colormix.blutextwid)
#define ColSldLabWid(cmw)	((cmw)->colormix.sldlabelwid)
#define ColValLabWid(cmw)	((cmw)->colormix.vallabelwid)
#define ColOkPbWid(cmw)		((cmw)->colormix.okpbwid)
#define ColAppPbWid(cmw)	((cmw)->colormix.apppbwid)
#define ColResPbWid(cmw)	((cmw)->colormix.respbwid)
#define ColCanPbWid(cmw)	((cmw)->colormix.canpbwid)
#define ColDispWid(cmw)		((cmw)->colormix.dispwid)
#define ColCurDispWid(cmw)	((cmw)->colormix.curdispwid)
#define ColOrigWid(cmw)		((cmw)->colormix.origwid)
#define ColNewWid(cmw)		((cmw)->colormix.newwid)
#define ColMixerWid(cmw)	((cmw)->colormix.mixerwid)
#define ColHLSMixerWid(cmw)	((cmw)->colormix.hlsmixerwid)
#define ColRGBMixerWid(cmw)	((cmw)->colormix.rgbmixerwid)
#define ColPickerMixerWid(cmw)	((cmw)->colormix.pickermixerwid)
#define ColCurMixerWid(cmw)	((cmw)->colormix.curmixerwid)
#define ColWorkWid(cmw)		((cmw)->colormix.workwid)
#define ColPDMenWid(cmw)	((cmw)->colormix.pdmenwid)
#define ColPickerDAWid(cmw)	((cmw)->colormix.pickerdawid)
#define ColInterpDAWid(cmw)	((cmw)->colormix.interpdawid)
#define ColPickerFrameWid(cmw)	((cmw)->colormix.pickerframewid)
#define ColInterpFrameWid(cmw)	((cmw)->colormix.interpframewid)
#define ColPickerPDWid(cmw)	((cmw)->colormix.pickerpdwid)
#define ColPickerOptMenWid(cmw)	((cmw)->colormix.pickeroptmenwid)
#define ColUndoPbWid(cmw)	((cmw)->colormix.undopbwid)
#define ColSmearPbWid(cmw)	((cmw)->colormix.smearpbwid)
#define ColHelpPbWid(cmw)	((cmw)->colormix.helppbwid)
#define ColLeftBucketPbWid(cmw)	((cmw)->colormix.lbucketpbwid)
#define ColRightBucketPbWid(cmw) ((cmw)->colormix.rbucketpbwid)
#define ColPTitleLabWid(cmw)	((cmw)->colormix.ptitlelabelwid)
#define ColSpectrumPbWid(cmw)	 ((cmw)->colormix.spectrumpbwid)
#define ColMetallicPbWid(cmw)	 ((cmw)->colormix.metallicpbwid)
#define ColPastelPbWid(cmw)	 ((cmw)->colormix.pastelpbwid)
#define ColEarthtonePbWid(cmw)	 ((cmw)->colormix.earthtonepbwid)
#define ColUserPalettePbWid(cmw) ((cmw)->colormix.userpalettepbwid)
#define ColITitleLabWid(cmw)	((cmw)->colormix.ititlelabelwid)
#define ColOptMenWid(cmw)	((cmw)->colormix.optmenwid)
#define ColPDHLSWid(cmw)	((cmw)->colormix.pdhlswid)
#define ColPDRGBWid(cmw)	((cmw)->colormix.pdrgbwid)
#define ColPDPickerWid(cmw)	((cmw)->colormix.pdpickerwid)
#define ColWarmerLabWid(cmw)	((cmw)->colormix.warmerlabelwid)
#define ColCoolerLabWid(cmw)	((cmw)->colormix.coolerlabelwid)
#define ColDarkerLabWid(cmw)	((cmw)->colormix.darkerlabelwid)
#define ColLighterLabWid(cmw)	((cmw)->colormix.lighterlabelwid)
#define ColWarmerPbWid(cmw)	((cmw)->colormix.warmerpbwid)
#define ColCoolerPbWid(cmw)	((cmw)->colormix.coolerpbwid)
#define ColDarkerPbWid(cmw)	((cmw)->colormix.darkerpbwid)
#define ColLighterPbWid(cmw)	((cmw)->colormix.lighterpbwid)
#define ColScratchPadPbWid(cmw)	((cmw)->colormix.sppbwid)
#define ColSPPopupWid(cmw)	((cmw)->colormix.sppopupwid)
#define ColSPInfoLabWid(cmw)	((cmw)->colormix.spinfolabelwid)
#define ColSPBucketPbWid(cmw)	((cmw)->colormix.spbucketpbwid)
#define ColSPClearPbWid(cmw)	((cmw)->colormix.spclearpbwid)
#define ColSPCancelPbWid(cmw)	((cmw)->colormix.spcancelpbwid)
#define ColSPScrolledWWid(cmw)	((cmw)->colormix.spswwid)
#define ColSPScrollBarWid(cmw)	((cmw)->colormix.spsbwid)
#define ColSPDrawingAreaWid(cmw) ((cmw)->colormix.spdawid)
#define ColSPFrameWid(cmw)	((cmw)->colormix.spframewid)
#define ColPDBrowserWid(cmw)	((cmw)->colormix.pdbrowserwid)
#define ColBrowserMixerWid(cmw)	((cmw)->colormix.browsermixerwid)
#define ColBrowserSWWid(cmw)	((cmw)->colormix.browserswwid)
#define ColBrowserBBWid(cmw)	((cmw)->colormix.browserbbwid)
#define ColBrowserFrameWid(cmw)	((cmw)->colormix.browserframewid)
#define ColBrowserSBWid(cmw)	((cmw)->colormix.browsersbwid)
#define ColBrowserPbWid(cmw)	((cmw)->colormix.browserpbwid)
#define ColPDGreyscaleWid(cmw)	((cmw)->colormix.pdgreyscalewid)
#define ColGreyscaleMixerWid(cmw)   ((cmw)->colormix.greyscalemixerwid)
#define ColGreyscaleIndLabWid(cmw)  ((cmw)->colormix.greyscaleindlabelwid)
#define ColGreyscaleSclWid(cmw)	    ((cmw)->colormix.greyscalescalewid)
#define ColHelpWid(cmw)	    	((cmw)->colormix.helpwid)
#define ColDragIcon(cmw)	((cmw)->colormix.dragIcon)
#define ColLastInterpSide(cmw)	((cmw)->colormix.lastinterpside)
#define ColDropHelpDialog(cmw)	((cmw)->colormix.drophelpdialog)
#define ColDropHelpTileLab(cmw)	((cmw)->colormix.drophelptilelabel)
#define ColDropHelpInterpLab(cmw)	((cmw)->colormix.drophelpinterplabel)

#define IsColMainLab(cmw)	((cmw)->colormix.mainlabel	!= NULL)
#define IsColDispLab(cmw)	((cmw)->colormix.displabel	!= NULL)
#define IsColMixLab(cmw)	((cmw)->colormix.mixlabel 	!= NULL)
#define IsColRedLab(cmw)	((cmw)->colormix.redlabel  	!= NULL)
#define IsColGrnLab(cmw)	((cmw)->colormix.grnlabel  	!= NULL)
#define IsColBluLab(cmw)	((cmw)->colormix.blulabel  	!= NULL)
#define IsColSldLab(cmw)	((cmw)->colormix.sldlabel  	!= NULL)
#define IsColValLab(cmw)	((cmw)->colormix.vallabel  	!= NULL)
#define IsColAppLab(cmw)	((cmw)->colormix.applabel  	!= NULL)
#define IsColOkLab(cmw)		((cmw)->colormix.oklabel   	!= NULL)
#define IsColResLab(cmw)	((cmw)->colormix.reslabel   	!= NULL)
#define IsColCanLab(cmw)	((cmw)->colormix.canlabel  	!= NULL)
#define IsColHueLab(cmw)	((cmw)->colormix.huelabel  	!= NULL)
#define IsColLightLab(cmw)	((cmw)->colormix.lightlabel  	!= NULL)
#define IsColSatLab(cmw)	((cmw)->colormix.satlabel  	!= NULL)
#define IsColBlkLab(cmw)	((cmw)->colormix.blklabel  	!= NULL)
#define IsColWhtLab(cmw)	((cmw)->colormix.whtlabel  	!= NULL)
#define IsColGryLab(cmw)	((cmw)->colormix.grylabel  	!= NULL)
#define IsColFulLab(cmw)	((cmw)->colormix.fullabel  	!= NULL)
#define IsColPDLab(cmw)		((cmw)->colormix.optlabel	!= NULL)
#define IsColPDHLSLab(cmw)	((cmw)->colormix.hlslabel	!= NULL)
#define IsColPDRGBLab(cmw)	((cmw)->colormix.rgblabel	!= NULL)
#define IsColPDPickerLab(cmw)	((cmw)->colormix.pickerlabel	!= NULL)
#define IsColUndoLab(cmw)	((cmw)->colormix.undolabel	!= NULL)
#define IsColSmearLab(cmw)	((cmw)->colormix.smearlabel	!= NULL)
#define IsColHelpLab(cmw)	((cmw)->colormix.helplabel	!= NULL)
#define IsColPTitleLab(cmw)	((cmw)->colormix.ptitlelabel	!= NULL)
#define IsColSpectrumLab(cmw)	((cmw)->colormix.spectrumlabel	!= NULL)
#define IsColPastelLab(cmw)	((cmw)->colormix.pastellabel	!= NULL)
#define IsColMetallicLab(cmw)	((cmw)->colormix.metalliclabel	!= NULL)
#define IsColEarthtoneLab(cmw)	((cmw)->colormix.earthtonelabel	!= NULL)
#define IsColUserPaletteLab(cmw) ((cmw)->colormix.userpalettelabel != NULL)
#define IsColITitleLab(cmw)	((cmw)->colormix.ititlelabel	!= NULL)
#define IsColWarmerLab(cmw)	((cmw)->colormix.warmerlabel	!= NULL)
#define IsColCoolerLab(cmw)	((cmw)->colormix.coolerlabel	!= NULL)
#define IsColDarkerLab(cmw)	((cmw)->colormix.darkerlabel	!= NULL)
#define IsColLighterLab(cmw)	((cmw)->colormix.lighterlabel	!= NULL)
#define IsColClearLab(cmw)	((cmw)->colormix.clearlabel	!= NULL)
#define IsColSPInfoLab(cmw)	((cmw)->colormix.spinfolabel	!= NULL)
#define IsColScratchPadLab(cmw)	((cmw)->colormix.splabel	!= NULL)
#define IsColPDBrowserLab(cmw)	 ((cmw)->colormix.browserlabel	 != NULL)
#define IsColPDGreyscaleLab(cmw) ((cmw)->colormix.greyscalelabel != NULL)

#define IsColMainLabWid(cmw)	((cmw)->colormix.mainlabelwid	!= (Widget) NULL)
#define IsColDispLabWid(cmw)	((cmw)->colormix.displabelwid	!= (Widget) NULL)
#define IsColMixLabWid(cmw)	((cmw)->colormix.mixlabelwid 	!= (Widget) NULL)
#define IsColRedLabWid(cmw)	((cmw)->colormix.redlabelwid  	!= (Widget) NULL)
#define IsColGrnLabWid(cmw)	((cmw)->colormix.grnlabelwid  	!= (Widget) NULL)
#define IsColBluLabWid(cmw)	((cmw)->colormix.blulabelwid  	!= (Widget) NULL)
#define IsColHueLabWid(cmw)	((cmw)->colormix.huelabelwid  	!= (Widget) NULL)
#define IsColLightLabWid(cmw)	((cmw)->colormix.lightlabelwid 	!= (Widget) NULL)
#define IsColSatLabWid(cmw)	((cmw)->colormix.satlabelwid  	!= (Widget) NULL)
#define IsColOptLabWid(cmw)	((cmw)->colormix.optlabelwid  	!= (Widget) NULL)
#define IsColBlkLabWid(cmw)	((cmw)->colormix.blklabelwid  	!= (Widget) NULL)
#define IsColWhtLabWid(cmw)	((cmw)->colormix.whtlabelwid  	!= (Widget) NULL)
#define IsColGryLabWid(cmw)	((cmw)->colormix.grylabelwid  	!= (Widget) NULL)
#define IsColFulLabWid(cmw)	((cmw)->colormix.fullabelwid  	!= (Widget) NULL)
#define IsColRedSclWid(cmw)	((cmw)->colormix.redsclwid  	!= (Widget) NULL)
#define IsColGrnSclWid(cmw)	((cmw)->colormix.grnsclwid  	!= (Widget) NULL)
#define IsColBluSclWid(cmw)	((cmw)->colormix.blusclwid  	!= (Widget) NULL)
#define IsColHueSclWid(cmw)	((cmw)->colormix.huesclwid  	!= (Widget) NULL)
#define IsColLightSclWid(cmw)	((cmw)->colormix.lightsclwid  	!= (Widget) NULL)
#define IsColSatSclWid(cmw)	((cmw)->colormix.satsclwid  	!= (Widget) NULL)
#define IsColRedTextWid(cmw)	((cmw)->colormix.redtextwid  	!= (Widget) NULL)
#define IsColGrnTextWid(cmw)	((cmw)->colormix.grntextwid  	!= (Widget) NULL)
#define IsColBluTextWid(cmw)	((cmw)->colormix.blutextwid  	!= (Widget) NULL)
#define IsColSldLabWid(cmw)	((cmw)->colormix.sldlabelwid  	!= (Widget) NULL)
#define IsColValLabWid(cmw)	((cmw)->colormix.vallabelwid  	!= (Widget) NULL)
#define IsColAppPbWid(cmw)	((cmw)->colormix.apppbwid  	!= (Widget) NULL)
#define IsColOkPbWid(cmw)	((cmw)->colormix.okpbwid   	!= (Widget) NULL)
#define IsColResPbWid(cmw)	((cmw)->colormix.respbwid   	!= (Widget) NULL)
#define IsColCanPbWid(cmw)	((cmw)->colormix.canpbwid  	!= (Widget) NULL)
#define IsColDispWid(cmw)	((cmw)->colormix.dispwid   	!= (Widget) NULL)
#define IsColCurDispWid(cmw)	((cmw)->colormix.curdispwid   	!= (Widget) NULL)
#define IsColOrigWid(cmw)	((cmw)->colormix.origwid      	!= (Widget) NULL)
#define IsColNewWid(cmw)	((cmw)->colormix.newwid       	!= (Widget) NULL)
#define IsColMixerWid(cmw)	((cmw)->colormix.mixerwid     	!= (Widget) NULL)
#define IsColRGBMixerWid(cmw)	((cmw)->colormix.rgbmixerwid   	!= (Widget) NULL)
#define IsColHLSMixerWid(cmw)	((cmw)->colormix.hlsmixerwid   	!= (Widget) NULL)
#define IsColPickerMixerWid(cmw) ((cmw)->colormix.pickermixerwid != (Widget) NULL)
#define IsColCurMixerWid(cmw)	((cmw)->colormix.curmixerwid   	!= (Widget) NULL)
#define IsColWorkWid(cmw)	((cmw)->colormix.workwid     	!= (Widget) NULL)
#define IsColPDMenWid(cmw)	((cmw)->colormix.pdmenwid       != (Widget) NULL)
#define IsColOptMenWid(cmw)	((cmw)->colormix.optmenwid	!= (Widget) NULL)
#define IsColPDHLSWid(cmw)	((cmw)->colormix.pdhlswid	!= (Widget) NULL)
#define IsColPDRGBWid(cmw)	((cmw)->colormix.pdrgbwid	!= (Widget) NULL)
#define IsColPDPickerWid(cmw)	((cmw)->colormix.pdpickerwid	!= (Widget) NULL)
#define IsColPickerDAWid(cmw)	((cmw)->colormix.pickerdawid    != (Widget) NULL)
#define IsColInterpDAWid(cmw)	((cmw)->colormix.interpdawid    != (Widget) NULL)
#define IsColPickerFrameWid(cmw) ((cmw)->colormix.pickerframewid    != (Widget) NULL)
#define IsColInterpFrameWid(cmw) ((cmw)->colormix.interpframewid    != (Widget) NULL)
#define IsColPickerPDWid(cmw)	((cmw)->colormix.pickerpdwid != (Widget) NULL)
#define IsColPickerOptMenWid(cmw) ((cmw)->colormix.pickeroptmenwid != (Widget) NULL)
#define IsColSpectrumPbWid(cmw)	((cmw)->colormix.spectrumpbwid  != (Widget) NULL)
#define IsColPastelPbWid(cmw)	((cmw)->colormix.pastelpbwid  != (Widget) NULL)
#define IsColMetallicPbWid(cmw)	((cmw)->colormix.metallicpbwid  != (Widget) NULL)
#define IsColEarthtonePbWid(cmw) ((cmw)->colormix.earthtonepbwid  != (Widget) NULL)
#define IsColUserPalettePbWid(cmw) ((cmw)->colormix.userpalettepbwid  != (Widget) NULL)
#define IsColUndoPbWid(cmw)	((cmw)->colormix.undopbwid     	!= (Widget) NULL)
#define IsColSmearPbWid(cmw)	((cmw)->colormix.smearpbwid     != (Widget) NULL)
#define IsColHelpPbWid(cmw)	((cmw)->colormix.helppbwid     != (Widget) NULL)
#define IsColLeftBucketPbWid(cmw)  ((cmw)->colormix.lbucketpbwid     != (Widget) NULL)
#define IsColRightBucketPbWid(cmw) ((cmw)->colormix.rbucketpbwid     != (Widget) NULL)
#define IsColPTitleLabWid(cmw)	((cmw)->colormix.ptitlelabelwid != (Widget) NULL)
#define IsColITitleLabWid(cmw)	((cmw)->colormix.ititlelabelwid != (Widget) NULL)
#define IsColWarmerLabWid(cmw)	((cmw)->colormix.warmerlabelwid != (Widget) NULL)
#define IsColCoolerLabWid(cmw)	((cmw)->colormix.coolerlabelwid != (Widget) NULL)
#define IsColLighterLabWid(cmw)	((cmw)->colormix.lighterlabelwid != (Widget) NULL)
#define IsColDarkerLabWid(cmw)	((cmw)->colormix.darkerlabelwid != (Widget) NULL)
#define IsColWarmerPbWid(cmw)	((cmw)->colormix.warmerpbwid != (Widget) NULL)
#define IsColCoolerPbWid(cmw)	((cmw)->colormix.coolerpbwid != (Widget) NULL)
#define IsColLighterPbWid(cmw)	((cmw)->colormix.lighterpbwid != (Widget) NULL)
#define IsColDarkerPbWid(cmw)	((cmw)->colormix.darkerpbwid != (Widget) NULL)
#define IsColScratchPadPbWid(cmw)   ((cmw)->colormix.sppbwid != (Widget) NULL)
#define IsColSPPopupWid(cmw)	    ((cmw)->colormix.sppopupwid != (Widget) NULL)
#define IsColSPInfoLabWid(cmw)	    ((cmw)->colormix.spinfolabelwid != (Widget) NULL)
#define IsColSPBucketPbWid(cmw)	    ((cmw)->colormix.spbucketpbwid != (Widget) NULL)
#define IsColSPClearPbWid(cmw)	    ((cmw)->colormix.spclearpbwid != (Widget) NULL)
#define IsColSPCancelPbWid(cmw)	    ((cmw)->colormix.spcancelpbwid != (Widget) NULL)
#define IsColSPScrolledWWid(cmw)    ((cmw)->colormix.spswwid != (Widget) NULL)
#define IsColSPScrollBarWid(cmw)    ((cmw)->colormix.spsbwid != (Widget) NULL)
#define IsColSPDrawingAreaWid(cmw)  ((cmw)->colormix.spdawid != (Widget) NULL)
#define IsColSPFrameWid(cmw)	    ((cmw)->colormix.spframewid != (Widget) NULL)
#define IsColPDBrowserWid(cmw)	     ((cmw)->colormix.pdbrowserwid != (Widget) NULL)
#define IsColBrowserMixerWid(cmw)    ((cmw)->colormix.browsermixerwid != (Widget) NULL)
#define IsColBrowserSWWid(cmw)	     ((cmw)->colormix.browserswwid != (Widget) NULL)
#define IsColBrowserBBWid(cmw)	     ((cmw)->colormix.browserbbwid != (Widget) NULL)
#define IsColBrowserFrameWid(cmw)    ((cmw)->colormix.browserframewid != (Widget) NULL)
#define IsColBrowserSBWid(cmw)	     ((cmw)->colormix.browsersbwid != (Widget) NULL)
#define IsColBrowserPBWid(cmw)	     ((cmw)->colormix.browserpbwid != (Widget *) NULL)
#define IsColPDGreyscaleWid(cmw)     ((cmw)->colormix.pdgreyscalewid != (Widget) NULL)
#define IsColGreyscaleMixerWid(cmw)  ((cmw)->colormix.greyscalemixerwid != (Widget) NULL)
#define IsColGreyscaleIndLabWid(cmw) ((cmw)->colormix.greyscaleindlabelwid != (Widget) NULL)
#define IsColGreyscaleSclWid(cmw)    ((cmw)->colormix.greyscalescalewid != (Widget) NULL)
#define IsColHelpWid(cmw)	    ((cmw)->colormix.helpwid != (Widget) NULL)

/* GetValues hook routines */

#ifdef _NO_PROTO
static void _DXmColorMixGetMainLabelStr();
static void _DXmColorMixGetDispLabelStr();
static void _DXmColorMixGetMixLabelStr();
static void _DXmColorMixGetSldLabelStr();
static void _DXmColorMixGetValLabelStr();
static void _DXmColorMixGetRedLabelStr();
static void _DXmColorMixGetGrnLabelStr();
static void _DXmColorMixGetBluLabelStr();
static void _DXmColorMixGetHueLabelStr();
static void _DXmColorMixGetLightLabelStr();
static void _DXmColorMixGetSatLabelStr();
static void _DXmColorMixGetBlkLabelStr();
static void _DXmColorMixGetWhtLabelStr();
static void _DXmColorMixGetGryLabelStr();
static void _DXmColorMixGetFulLabelStr();
static void _DXmColorMixGetOptLabelStr();
static void _DXmColorMixGetHLSLabelStr();
static void _DXmColorMixGetRGBLabelStr();
static void _DXmColorMixGetPickerLabelStr();
static void _DXmColorMixGetPTitleLabelStr();
static void _DXmColorMixGetSpectrumLabelStr();
static void _DXmColorMixGetPastelLabelStr();
static void _DXmColorMixGetMetallicLabelStr();
static void _DXmColorMixGetEarthLabelStr();
static void _DXmColorMixGetUserPalLabelStr();
static void _DXmColorMixGetITitleLabelStr();
static void _DXmColorMixGetUndoLabelStr();
static void _DXmColorMixGetSmearLabelStr();
static void _DXmColorMixGetHelpLabelStr();
static void _DXmColorMixGetClearLabelStr();
static void _DXmColorMixGetSPInfoLabelStr();
static void _DXmColorMixGetSPLabelStr();
static void _DXmColorMixGetOkLabelStr();
static void _DXmColorMixGetApplyLabelStr();
static void _DXmColorMixGetCancelLabelStr();
static void _DXmColorMixGetResetLabelStr();
static void _DXmColorMixGetWarmerLabelStr();
static void _DXmColorMixGetCoolerLabelStr();
static void _DXmColorMixGetLighterLabelStr();
static void _DXmColorMixGetDarkerLabelStr();
static void _DXmColorMixGetBrowserLabelStr();
static void _DXmColorMixGetGreysclLabelStr();

#else /* _NO_PROTO undefined */

static void _DXmColorMixGetMainLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetDispLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetMixLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetSldLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetValLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetRedLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetGrnLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetBluLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetHueLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetLightLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetSatLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetBlkLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetWhtLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetGryLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetFulLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetOptLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetHLSLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetRGBLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetPickerLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetOkLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetApplyLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetCancelLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetResetLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetUndoLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetSmearLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetHelpLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetPTitleLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetSpectrumLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetPastelLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetMetallicLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetEarthLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetUserPalLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetITitleLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetWarmerLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetCoolerLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetLighterLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetDarkerLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetClearLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetSPInfoLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetSPLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetBrowserLabelStr ( Widget cmw , int resource , XtArgVal *value );
static void _DXmColorMixGetGreysclLabelStr ( Widget cmw , int resource , XtArgVal *value );

#endif /* _NO_PROTO undefined */

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DXmColorMixP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
