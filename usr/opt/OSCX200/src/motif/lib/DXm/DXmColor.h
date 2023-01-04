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
                                                          
#ifndef _DXmColorMix_h
#define _DXmColorMix_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:Xm.h>
#else
#include <Xm/Xm.h>
#endif

/* class record constants */

externalref WidgetClass dxmColorMixWidgetClass;

typedef struct _DXmColorMixClassRec * DXmColorMixWidgetClass;
typedef struct _DXmColorMixRec      * DXmColorMixWidget;


/* public entry points  */

#ifdef _NO_PROTO
extern Widget	DXmCreateColorMix();
extern Widget	DXmCreateColorMixDialog();
extern void	DXmColorMixGetNewColor();
extern void	DXmColorMixSetNewColor();

#else /* _NO_PROTO undefined */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern void DXmColorMixSetNewColor ( DXmColorMixWidget cmw , unsigned short red , unsigned short green , unsigned short blue );
extern void DXmColorMixGetNewColor ( DXmColorMixWidget cmw , unsigned short *red , unsigned short *green , unsigned short *blue );
extern Widget DXmCreateColorMix ( Widget p , String name , ArgList args , Cardinal ac );
extern Widget DXmCreateColorMixDialog ( Widget ds_p , String name , ArgList args , Cardinal ac );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO undefined */

/* callback structure */

typedef struct
{
    int 	reason;
    XEvent	*event;
    unsigned short newred;
    unsigned short newgrn;
    unsigned short newblu;
    char * newname;
    unsigned short origred;
    unsigned short origgrn;
    unsigned short origblu;
} DXmColorMixCallbackStruct;


/* color model constants */

#define DXmColorModelRGB	0
#define DXmColorModelHLS	1
#define DXmColorModelPicker	2
#define DXmColorModelBrowser	3
#define DXmColorModelGreyscale	4


/* colormix resources */

#define DXmNorigRedValue		"DXmorigRedValue"
#define DXmCOrigRedValue        	"DXmOrigRedValue"
#define DXmNorigGreenValue		"DXmorigGreenValue"
#define DXmCOrigGreenValue      	"DXmOrigGreenValue"
#define DXmNorigBlueValue		"DXmorigBlueValue"
#define DXmCOrigBlueValue       	"DXmOrigBlueValue"
#define DXmNnewRedValue			"DXmnewRedValue"
#define DXmCNewRedValue        		"DXmNewRedValue"
#define DXmNnewGreenValue		"DXmnewGreenValue"
#define DXmCNewGreenValue      		"DXmNewGreenValue"
#define DXmNnewBlueValue		"DXmnewBlueValue"
#define DXmCNewBlueValue       		"DXmNewBlueValue"
#define DXmNbackRedValue		"DXmbackRedValue"
#define DXmCBackRedValue        	"DXmBackRedValue"
#define DXmNbackGreenValue		"DXmbackGreenValue"
#define DXmCBackGreenValue      	"DXmBackGreenValue"
#define DXmNbackBlueValue		"DXmbackBlueValue"
#define DXmCBackBlueValue       	"DXmBackBlueValue"
#define DXmNsetNewColorProc     	"DXmsetNewColorProc"
#define DXmCSetNewColorProc     	"DXmSetNewColorProc"
#define DXmNmainLabel			"DXmmainLabel"
#define DXmNdisplayLabel		"DXmdisplayLabel"
#define DXmNmixerLabel			"DXmmixerLabel"
#define DXmNredLabel			"DXmredLabel"
#define DXmNgreenLabel			"DXmgreenLabel"
#define DXmNblueLabel			"DXmblueLabel"
#define DXmNsliderLabel			"DXmsliderLabel"
#define DXmNvalueLabel			"DXmvalueLabel"
#define DXmNresetLabelString		"DXmresetLabelString"
#define DXmNoptionLabel			"DXmoptionLabel"
#define DXmNhlsLabel            	"DXmhlsLabel"
#define DXmNrgbLabel            	"DXmrgbLabel"
#define DXmNokCallback			"DXmokCallback"
#define DXmNdisplayColWinWidth		"DXmdisplayColWinWidth"
#define DXmCDisplayColWinWidth  	"DXmDisplayColWinWidth"
#define DXmNdisplayColWinHeight		"DXmdisplayColWinHeight"
#define DXmCDisplayColWinHeight		"DXmDisplayColWinHeight"
#define DXmNdispWinMargin		"DXmdispWinMargin"
#define DXmCDispWinMargin		"DXmDispWinMargin"
#define DXmNdisplayWindow		"DXmdisplayWindow"
#define DXmNorigDispWindow		"DXmorigDispWindow"
#define DXmNnewDispWindow		"DXmnewDispWindow"
#define DXmNmixerWindow			"DXmmixerWindow"
#define DXmNworkWindow			"DXmworkWindow"
#define DXmNmatchColors			"DXmmatchColors"
#define DXmCMatchColors         	"DXmMatchColors"
#define DXmRMatchColors         	"DXmMatchColors"
#define DXmNgreyscaleOnGreyscale	"DXmgreyscaleOnGreyscale"
#define DXmCGreyscaleOnGreyscale	"DXmGreyscaleOnGreyscale"
#define DXmRColorValue          	"DXmColorValue"
#define DXmNsetMixerColorProc   	"DXmsetMixerColorProc"
#define DXmCSetMixerColorProc   	"DXmSetMixerColorProc"
#define DXmNnewHueValue			"DXmnewHueValue"
#define DXmCNewHueValue        		"DXmNewHueValue"
#define DXmNnewLightValue		"DXmnewLightValue"
#define DXmCNewLightValue      		"DXmNewLightValue"
#define DXmNnewSatValue			"DXmnewSatValue"
#define DXmCNewSatValue        		"DXmNewSatValue"
#define DXmNcolorModel			"DXmcolorModel"
#define DXmCColorModel	       		"DXmColorModel"
#define DXmRColorModel			"DXmColorModel"
#define DXmNhueLabel			"DXmhueLabel"
#define DXmNlightLabel			"DXmlightLabel"
#define DXmNsatLabel			"DXmsatLabel"
#define DXmNblackLabel			"DXmblackLabel"
#define DXmNwhiteLabel			"DXmwhiteLabel"
#define DXmNgrayLabel			"DXmgrayLabel"
#define DXmNfullLabel			"DXmfullLabel"
#define DXmNpickerTileHeight		"DXmpickerTileHeight"
#define DXmCPickerTileHeight		"DXmPickerTileHeight"
#define DXmNpickerTileWidth		"DXmpickerTileWidth"
#define DXmCPickerTileWidth		"DXmPickerTileWidth"
#define DXmNpickerColors		"DXmpickerColors"
#define DXmCPickerColors		"DXmPickerColors"
#define DXmRPickerColors		"DXmPickerColors"
#define DXmNpickerColorCount		"DXmpickerColorCount"
#define DXmCPickerColorCount		"DXmPickerColorCount"
#define DXmNpickerLabel			"DXmpickerLabel"
#define DXmNspectrumLabel		"DXmspectrumLabel"
#define DXmNpastelLabel			"DXmpastelLabel"
#define DXmNmetallicLabel		"DXmmetallicLabel"
#define DXmNvividLabel			"DXmvividLabel"
#define DXmNearthtoneLabel		"DXmearthtoneLabel"
#define DXmNuserPaletteLabel		"DXmuserPaletteLabel"
#define DXmNpickerTitleLabel		"DXmpickerTitleLabel"
#define DXmNinterpTileHeight		"DXminterpTileHeight"
#define DXmCInterpTileHeight		"DXmInterpTileHeight"
#define DXmNinterpTileWidth		"DXminterpTileWidth"
#define DXmCInterpTileWidth		"DXmInterpTileWidth"
#define DXmNinterpTileCount		"DXminterpTileCount"
#define DXmCInterpTileCount		"DXmInterpTileCount"
#define DXmNinterpTitleLabel		"DXminterpTitleLabel"
#ifndef NO_WARM
#define DXmNwarmerLabel			"DXmwarmerLabel"
#define DXmNcoolerLabel			"DXmcoolerLabel"
#define DXmNlighterLabel		"DXmlighterLabel"
#define DXmNdarkerLabel			"DXmdarkerLabel"
#define DXmNwarmthIncrement		"DXmwarmthIncrement"
#define DXmCWarmthIncrement		"DXmWarmthIncrement"
#define DXmNlightnessIncrement		"DXmlightnessIncrement"
#define DXmCLightnessIncrement		"DXmLightnessIncrement"
#endif
#define DXmNundoLabel			"DXmundoLabel"
#define DXmNsmearLabel			"DXmsmearLabel"
#define DXmNscratchPadLabel		"DXmscratchPadLabel"
#define DXmNscratchPadInfoLabel		"DXmscratchPadInfoLabel"
#define DXmNclearLabel			"DXmclearLabel"
#define DXmNbrowserLabel		"DXmbrowserLabel"
#define DXmNbrowserItemCount		"DXmbrowserItemCount"
#define DXmCBrowserItemCount		"DXmBrowserItemCount"
#define DXmNgreyscaleLabel		"DXmgreyscaleLabel"

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DXmColorMix_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
