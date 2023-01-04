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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1990 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
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
**	6 Jan 1993  Created enumerated data type DXmCSTextDirection - Russ
**
**--
**/

#ifndef _DXmCSText_h
#define _DXmCSText_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:IntrinsicP.h>
#else
#include <X11/IntrinsicP.h>
#endif

typedef long 			DXmCSTextPosition;

typedef enum {DXmCSTextStatusEditDone,DXmCSTextStatusEditError} DXmCSTextStatus;

/* Class record constants */

#ifndef NO_CSTEXT_EXTERNALREF
externalref 	WidgetClass	dxmCSTextWidgetClass;
#endif

typedef struct _DXmCSTextRec	  *DXmCSTextWidget;

/*
 * forward declatations
 */

#ifdef _NO_PROTO


/* cstext.c */
extern Boolean DXmCSTextRemove ( );
extern Boolean DXmCSTextCopy ( );
extern Boolean DXmCSTextCut ( );
extern Boolean DXmCSTextPaste ( );
extern Widget DXmCreateCSText ( );
extern Widget DXmCreateScrolledCSText ( );
extern XmString DXmCSTextGetString ( );
extern void DXmCSTextSetString ( );
extern DXmCSTextStatus DXmCSTextReplace ( );
extern void DXmCSTextRead ( );
extern DXmCSTextStatus DXmCSTextInsert ( );
extern DXmCSTextStatus DXmCSTextInsertChar ( );
extern Boolean DXmCSTextHasSelection ( );
extern Boolean DXmCSTextGetEditable ( );
extern void DXmCSTextSetEditable ( );
extern int DXmCSTextGetMaxLength ( );
extern void DXmCSTextSetMaxLength ( );
extern XmStringDirection DXmCSTextGetTextPath ( );
extern void DXmCSTextSetTextPath ( );
extern XmString DXmCSTextGetSelection ( );
extern Boolean DXmCSTextGetSelectionInfo ( );
extern void DXmCSTextSetSelection ( );
extern void DXmCSTextClearSelection ( );
extern void DXmCSTextShowPosition ( );
extern void DXmCSTextVerticalScroll ( );
extern void DXmCSTextHorizontalScroll ( );
extern void DXmCSTextDisableRedisplay ( );
extern void DXmCSTextEnableRedisplay ( );
extern void DXmCSTextMarkRedraw ( );
extern void DXmCSTextSetHighlight ( );
extern DXmCSTextPosition DXmCSTextGetTopPosition ( );
extern void DXmCSTextSetTopPosition ( );
extern DXmCSTextPosition DXmCSTextGetLastPosition ( );
extern DXmCSTextPosition DXmCSTextGetCursorPosition ( );
extern DXmCSTextPosition DXmCSTextGetInsertionPosition ( );
extern void DXmCSTextSetInsertionPosition ( );
extern void DXmCSTextSetCursorPosition ( );
extern int DXmCSTextPosToLine ( );
extern int DXmCSTextNumLines ( );
extern void DXmCSTextInvalidate ( );
extern DXmCSTextPosition DXmCSTextXYToPos ( );
extern Boolean DXmCSTextPosToXY ( );
extern XmString DXmCSTextGetStringWrapped ( );

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* cstext.c */
extern Boolean DXmCSTextRemove ( DXmCSTextWidget widget );
extern Boolean DXmCSTextCopy ( Widget widget , Time time );
extern Boolean DXmCSTextCut ( Widget widget , Time time );
extern Boolean DXmCSTextPaste ( DXmCSTextWidget widget );
extern Widget DXmCreateCSText ( Widget parent , char *name , Arg *args , Cardinal num_args );
extern Widget DXmCreateScrolledCSText ( Widget parent , char *name , ArgList arglist , Cardinal argcount );
extern XmString DXmCSTextGetString ( Widget widget );
extern void DXmCSTextSetString ( DXmCSTextWidget widget , XmString value );
extern DXmCSTextStatus DXmCSTextReplace ( Widget widget , DXmCSTextPosition frompos , DXmCSTextPosition topos , XmString value );
extern void DXmCSTextRead ( Widget widget , DXmCSTextPosition frompos , DXmCSTextPosition topos , XmString *value );
extern DXmCSTextStatus DXmCSTextInsert ( Widget widget , DXmCSTextPosition pos , XmString value );
extern DXmCSTextStatus DXmCSTextInsertChar ( Widget widget , DXmCSTextPosition pos , XmString value );
extern Boolean DXmCSTextHasSelection ( DXmCSTextWidget widget );
extern Boolean DXmCSTextGetEditable ( DXmCSTextWidget widget );
extern void DXmCSTextSetEditable ( DXmCSTextWidget widget , Boolean editable );
extern int DXmCSTextGetMaxLength ( DXmCSTextWidget widget );
extern void DXmCSTextSetMaxLength ( DXmCSTextWidget widget , int max );
extern XmStringDirection DXmCSTextGetTextPath ( DXmCSTextWidget widget );
extern void DXmCSTextSetTextPath ( DXmCSTextWidget widget , XmStringDirection path );
extern XmString DXmCSTextGetSelection ( Widget widget );
extern Boolean DXmCSTextGetSelectionInfo ( Widget widget , DXmCSTextPosition *left , DXmCSTextPosition *right );
extern void DXmCSTextSetSelection ( Widget widget , DXmCSTextPosition first , DXmCSTextPosition last , Time time );
extern void DXmCSTextClearSelection ( Widget widget , Time time );
extern void DXmCSTextShowPosition ( DXmCSTextWidget widget , DXmCSTextPosition position );
extern void DXmCSTextVerticalScroll ( DXmCSTextWidget widget , int n );
extern void DXmCSTextHorizontalScroll ( DXmCSTextWidget widget , int n );
extern void DXmCSTextDisableRedisplay ( DXmCSTextWidget widget , Boolean losesbackingstore );
extern void DXmCSTextEnableRedisplay ( DXmCSTextWidget widget );
extern void DXmCSTextMarkRedraw ( DXmCSTextWidget widget , DXmCSTextPosition left , DXmCSTextPosition right );
extern void DXmCSTextSetHighlight ( DXmCSTextWidget widget , DXmCSTextPosition left , DXmCSTextPosition right , XmHighlightMode mode );
extern DXmCSTextPosition DXmCSTextGetTopPosition ( DXmCSTextWidget widget );
extern void DXmCSTextSetTopPosition ( DXmCSTextWidget widget , DXmCSTextPosition top_position );
extern DXmCSTextPosition DXmCSTextGetLastPosition ( DXmCSTextWidget widget );
extern DXmCSTextPosition DXmCSTextGetCursorPosition ( DXmCSTextWidget widget );
extern DXmCSTextPosition DXmCSTextGetInsertionPosition ( DXmCSTextWidget widget );
extern void DXmCSTextSetInsertionPosition ( DXmCSTextWidget widget , DXmCSTextPosition position );
extern void DXmCSTextSetCursorPosition ( DXmCSTextWidget widget , DXmCSTextPosition position );
extern int DXmCSTextPosToLine ( DXmCSTextWidget widget , DXmCSTextPosition position );
extern int DXmCSTextNumLines ( DXmCSTextWidget widget );
extern void DXmCSTextInvalidate ( DXmCSTextWidget widget , DXmCSTextPosition left , DXmCSTextPosition right );
extern DXmCSTextPosition DXmCSTextXYToPos( Widget widget, Position x, Position y );
extern Boolean DXmCSTextPosToXY ( Widget widget, DXmCSTextPosition position, Position *x, Position *y );
extern XmString DXmCSTextGetStringWrapped( Widget widget, DXmCSTextPosition start, DXmCSTextPosition end );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif  /* _NO_PROTO */



/*
 * compound string text widget (sub-class of text)
 */

#define DXmNnoFontCallback	"noFontCallback"
#define DXmNtextPath		"textPath"
#define DXmCTextPath	 	"TextPath"
#define DXmNeditingPath		"editingPath"
#define DXmCEditingPath		"EditingPath"
#define DXmCCompString          "CompString"
#define DXmNbidirectionalCursor	"bidirectionalCursor"
#define DXmCBidirectionalCursor	"BidirectionalCursor"
/* sub-resource for changing input method, for Asian use only
 */
#define DXmNinputMethod	        "inputMethod"
#define DXmCInputMethod	        "InputMethod"
#define DXmNinputMethodType	"inputMethodType"
#define DXmCInputMethodType	"InputMethodType"

#define DXmIM_NONE	        "IM_NONE"
#define DXmIM_DEFAULT           "IM_DEFAULT"

#define DXmIM_DEFAULT_TYPE	0
#define DXmIM_NONE_TYPE		1
#define DXmIM_STRING_TYPE	2

#define DXmCURSOR_MODE_ADD   1

#define DXmHIGHLIGHT_SELECTED              1
#define DXmHIGHLIGHT_SECONDARY_SELECTED    2

#define DXmCR_NOFONT   41

/*
 * CSText direction constants
 */
typedef enum {
    DXmDIRECTION_RIGHT_DOWN=0,
    DXmDIRECTION_LEFT_DOWN,
    DXmDIRECTION_RIGHT_UP,
    DXmDIRECTION_LEFT_UP
} DXmCSTextDirection;

typedef struct _DXmCSTextClassRec *DXmCSTextWidgetClass;

typedef struct
{
    int     		reason;
    XEvent  		*event;
    char    		*charset;
    unsigned int  	charset_len;
} DXmCSTextCallbackStruct;

typedef struct
{
    int                 reason;
    XEvent              *event;
    Boolean             doit;
    DXmCSTextPosition   currInsert,
                        newInsert;
    DXmCSTextPosition   startPos,
	                endPos;
    XmString            text;

} DXmCSTextVerifyCallbackStruct, *DXmCSTextVerifyPtr;
  


#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif

