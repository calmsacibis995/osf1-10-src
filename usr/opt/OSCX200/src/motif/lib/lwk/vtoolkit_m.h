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
/* vtoolkit.h - General "V" toolkit include file

*****************************************************************************
*									    *
*  COPYRIGHT (c) 1990  BY               				    *
*  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		    *
*  ALL RIGHTS RESERVED.						    	    *
* 									    *
*  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED    *
*  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE    *
*  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER    *
*  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY    *
*  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY    *
*  TRANSFERRED.							            *
* 									    *
*  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE    *
*  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT    *
*  CORPORATION.							            *
* 									    *
*  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS    *
*  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		    *
* 									    *
*****************************************************************************

*****************************************************************************
*									    *
*		       DIGITAL INTERNAL USE ONLY			    *
*									    *
*      THIS SOFTWARE IS NOT FORMALLY SUPPORTED AND IS PROVIDED AS IS.       *
*  Do not distribute outside Digital without first contacting the author.   *
*									    *
*		Steve Klein, DECWIN::KLEIN, Digital ZKO			    *
*									    *
*****************************************************************************

This module assists in the portability of the "V" widgets across toolkits.
It currently supports the DWT toolkit (the default) and the MOTIF toolkit.

MODIFICATION HISTORY:

20-Feb-1990 (sjk) Fix capitalization to get ULTRIX version to build.
31-Jan-1990 (sjk) Added MOTIF synonym for DwtOrientationHorizontal,
    DwtOrientationVertical, and scrollwidgetclass.
24-Jan-1990 (sjk) Added MOTIF synonym for DwtLatin1String.
23-Jan-1990 (sjk) Added MOTIF synonym for DwtNlabel.
19-Jan-1990 (sjk) Initial entry.
 8-Mar-1990 (lg)  Build Motif regardless, remove XUI includes
 22-Aug-90  (ap)  Normalize include file path
*/


#include <Mrm/MrmAppl.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <Xm/XmP.h>
#include <Xm/ScrollBar.h>

#define DRMHierarchy		MrmHierarchy
#define DRMRegisterArg		MrmRegisterArg
#define DRMSuccess		MrmSUCCESS
#define DRMwcUnknown		MRMwcUnknown
#define DwtAnyCallbackStruct	XmAnyCallbackStruct
#define DwtAttachNone		XmATTACH_NONE
#define DwtCFontList		XmCFontList
#define DwtFetchWidget		MrmFetchWidget
#define DwtFontRec		XmFontListRec
#define DwtInitializeDRM	MrmInitialize
#define DwtLatin1String(t)	XmStringCreate (t, XmSTRING_DEFAULT_CHARSET)
#define DwtMenuPosition		XmMenuPosition
#define DwtNactivateCallback	XmNactivateCallback
#define DwtNadbRightAttachment	XmNrightAttachment
#define DwtNarmCallback		XmNarmCallback
#define DwtNdisarmCallback	XmNdisarmCallback
#define DwtNdragCallback	XmNdragCallback
#define DwtNinc			XmNincrement
#define DwtNlabel		XmNlabelString
#define DwtNmaxValue		XmNmaximum
#define DwtNminValue		XmNminimum
#define DwtNorientation		XmNorientation
#define DwtNpageDecCallback	XmNpageDecrementCallback
#define DwtNpageIncCallback	XmNpageIncrementCallback
#define DwtNpageInc		XmNpageIncrement
#define DwtNshown		XmNsliderSize
#define DwtNvalue		XmNvalue
#define DwtNvalueChangedCallback XmNvalueChangedCallback
#define DwtOpenHierarchy	MrmOpenHierarchy
#define DwtOrientationHorizontal XmHORIZONTAL
#define DwtOrientationVertical	XmVERTICAL
#define DwtRFontList		XmRFontList
#define DwtRegisterClass	MrmRegisterClass
#define DwtRegisterDRMNames	MrmRegisterNames
#define DwtSFontDefault		XmDefaultFont
#define DwtSTextClearSelection	XmTextClearSelection
#define DwtSTextGetString	XmTextGetString
#define DwtSTextSetString	XmTextSetString
#define DwtScrollBarCallbackStruct XmScrollBarCallbackStruct
#define DwtTextDisableRedisplay	XmTextDisableRedisplay
#define DwtTextEnableRedisplay	XmTextEnableRedisplay
#define DwtTextSetTopPosition	XmTextSetTopPosition
#define DwtTogglebuttonCallbackStruct XmToggleButtonCallbackStruct
#define scrollwidgetclass	xmScrollBarWidgetClass

#if defined(VAXC)
#define external globalref
#else
#define external extern
#endif /* VAXC */
