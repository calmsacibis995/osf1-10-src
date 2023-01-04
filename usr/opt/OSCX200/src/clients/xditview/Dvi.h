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
* $XConsortium: Dvi.h,v 1.5 91/07/25 21:33:53 keith Exp $
*/

#ifndef _XtDvi_h
#define _XtDvi_h

/***********************************************************************
 *
 * Dvi Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 foreground	     Foreground		Pixel		Black
 fontMap	     FontMap		char *		...
 pageNumber	     PageNumber		int		1
*/

#define XtNfontMap	"fontMap"
#define XtNpageNumber	"pageNumber"
#define XtNlastPageNumber   "lastPageNumber"
#define XtNnoPolyText	"noPolyText"
#define XtNseek		"seek"
#define XtNscreenResolution "screenResolution"
#define XtNpageWidth	"pageWidth"
#define XtNpageHeight	"pageHeight"

#define XtCFontMap	"FontMap"
#define XtCPageNumber	"PageNumber"
#define XtCLastPageNumber   "LastPageNumber"
#define XtCNoPolyText	"NoPolyText"
#define XtCSeek		"Seek"
#define XtCScreenResolution "ScreenResolution"
#define XtCPageWidth	"PageWidth"
#define XtCPageHeight	"PageHeight"

typedef struct _DviRec *DviWidget;  /* completely defined in DviPrivate.h */
typedef struct _DviClassRec *DviWidgetClass;    /* completely defined in DviPrivate.h */

extern WidgetClass dviWidgetClass;

#endif /* _XtDvi_h */
/* DON'T ADD STUFF AFTER THIS #endif */
