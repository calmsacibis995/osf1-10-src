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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxmail/dvr_decw_def.h,v 1.1.2.2 92/06/26 11:43:49 Dave_Hill Exp $ */
/*
**++
**  COPYRIGHT (c) 1987, 1991 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
**  ABSTRACT:
**  	this file will include all the public
**  	definitions for appications calling the
**  	windowing CDA Viewer interface. Note, public function
**	prototypes are in dvr$decw_ptp.h on vms; dvr_decw_ptp.h on
**	ultrix; dvr$wptp.h os/2.
**
**--
**/
#ifdef vms
#define CDA_DECWINDOWS

#ifndef _cdatyp_
#include <cda$typ.h>
#endif

#endif
/* to avoid confusing makedepend */
#ifdef unix
# undef __unix__
# define __unix__
#endif

#ifdef __unix__
#define CDA_DECWINDOWS

#ifndef _cdatyp_
#include <cda_typ.h>
#endif

#endif

#ifndef CDA_DECWINDOWS
#ifndef _cdatyp_
#include <cda$typ.h>
#endif
#endif

/*
 * typedef for DVR callback structure
 */

typedef struct
  {
    int		    reason;

#ifdef CDA_DECWINDOWS
    XEvent	    *event;
#endif

    CDAstatus 	    status;
    CDAenvirontext  CDA_FAR *string_ptr;
  } DvrCallbackStruct;

#define DvrCRactivate	 		0
#define DvrCRendDocument  		1
#define DvrCRhelpRequested		2
#define DvrCRcdaError			3
#define DvrCRpsOK			4
#define DvrCRexpose			5
#define DvrCRmouseMotion		6
#define DvrCRbuttonEvent		7
#define DvrCRscrollBarEvent		8

/*
 * SELECT options flags (Make sure don't conflict w/ CC Viewer flag position
 * when adding more.)  These flags must match the correponding CC Viewer values.
 */
#define DvrSoftDirectives 		(1<<0)
#define DvrWordWrap			(1<<1)
#define DvrText				(1<<4)	/* output text		    */
#define DvrImages			(1<<5)	/* output images	    */
#define DvrGraphics			(1<<6)	/* output graphics	    */
#define DvrLayout			(1<<8)	/* Do layout		    */
#define DvrSpecificLayout		(1<<9)	/* Do specific layout	    */

/* Values for item list codes used in Converter Selection Widget. */

#define DvrOptionFlags			201
#define DvrFileSelectionOverride	202
#define DvrFormatSelectionList		203
#define DvrInitialFormatSelection	204
#define DvrOptionalLabelButton		205
#define DvrOptionalIconButton		206
#define DvrDirectoryMask		207
#define DvrWindowX			208
#define DvrWindowY			209
#define DvrWindowCaption		210
#define DvrServerNode			211
#define DvrUsername			212
#define DvrPassword			213
#define DvrUserParameter		214

/* Mask values for option flags in Converter Selection Widget. */

#define DvrMinputFile		  	  1
#define DvrMoutputFile		  	  2
#define DvrMlistDDIFformats		  4
#define DvrMlistDTIFformats		  8
#define DvrMlistCDAformats		 16
#define DvrMlistAllFormats		252
#define DvrMnoMaskModification		256
#define DvrMomitOptionsFile		512
#define DvrMomitOptionsButton		512
#define DvrMomitNetworkButton	       1024


#ifdef CDA_DECWINDOWS

/* literals to be used for CDA X viewer (specific to Xwindows) */

#define DvrViewerClassStr 		"DVRviewer"

#define DvrNscrollHorizontal 		"DVRscrollHorizontal"
#define DvrNscrollVertical 		"DVRscrollVertical"
#define DvrNprocessingOptions		"DVRprocessingOptions"
#define DvrNpaperWidth			"DVRpaperWidth"
#define DvrNpaperHeight			"DVRpaperHeight"

#define DvrNbuttonBox			"DVRbuttonBox"
#define DvrNpageNumber			"DVRpageNumber"

#define DvrNuseComments			"DVRuseComments"
#define DvrNuseBitmaps			"DVRuseBitmaps"
#define DvrNuseTrays			"DVRuseTrays"
#define DvrNwatchProgress		"DVRwatchProgress"
#define DvrNorientation			"DVRorientation"
#define DvrNscaleValue			"DVRscaleValue"
#define DvrNheaderRequired		"DVRheaderRequired"

#define DvrNscrollBarCallback		"DVRscrollBarCallback"
#define DvrNbuttonsCallback		"DVRbuttonsCallback"
#define DvrNmouseMotionCallback		"DVRmouseMotionCallback"


/*
 * typedef for DVR scroll callback structure
 * (so far, only applies to XWindows)
 */

typedef struct
  {
    int		    reason;   		 /* scroll bar callback reason */
    XEvent	    *event;              /* X event structure */
    int		    Xtop;                /* current X top position within page */
    int		    Ytop;                /* current Y top position within page */
    int		    page_width;		 /* page width */
    int		    page_height;	 /* page height*/
    int		    win_width;		 /* window width */
    int		    win_height;		 /* window height */
  } DvrScrollBarEvent;

#else

/*  item codes for low level viewer create DvrViewerCreate() when
 *  not on DECwindows (OS/2,...)
 */

#define DVR$_SCROLL_HORIZONTAL		101
#define DVR$_SCROLL_VERTICAL		102
#define DVR$_PROCESSING_OPTIONS		103
#define DVR$_PAPER_WIDTH		104
#define DVR$_PAPER_HEIGHT		105
#define DVR$_BUTTON_BOX			106
#define DVR$_PAGE_NUMBER		107
#define DVR$_CALLBACK 			108
#define DVR$_HELP_CALLBACK		109
#define DVR$_X				110
#define DVR$_Y				111
#define DVR$_WIDTH			112
#define DVR$_HEIGHT			113

/* Type definitions */

typedef struct {
    CDAuserparam   user_param;
    CDAenvirontext CDA_FAR *server;
    CDAenvirontext CDA_FAR *username;
    CDAenvirontext CDA_FAR *password;
    CDAenvirontext CDA_FAR *dir_mask;
    CDAenvirontext CDA_FAR *file_spec;
    CDAenvirontext CDA_FAR *file_format;
    CDAsize        num_options;
    CDAenvirontext CDA_FAR * CDA_FAR * options;
} DvrConvCallbackData;


#endif
