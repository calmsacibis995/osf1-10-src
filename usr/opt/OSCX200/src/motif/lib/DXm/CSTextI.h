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

              Copyright (c) Digital Equipment Corporation, 1990
              All Rights Reserved.  Unpublished rights reserved
              under the copyright laws of the United States.

              The software contained on this media is proprietary
              to and embodies the confidential technology of
              Digital Equipment Corporation.  Possession, use,
              duplication or dissemination of the software and
              media is authorized only pursuant to a valid written
              license from Digital Equipment Corporation.

              RESTRICTED RIGHTS LEGEND   Use, duplication, or
              disclosure by the U.S. Government is subject to
              restrictions as set forth in Subparagraph (c)(1)(ii)
              of DFARS 252.227-7013, or in FAR 52.227-19, as
              applicable.

*****************************************************************************
**++
**  FACILITY:
**
**      DECwindows Toolkit
**
**  ABSTRACT:
**
**      International Text Widget String Source Data Handling Code
**
**
**  MODIFICATION HISTORY:
**
**      12 APR 1990  Begin rework of old CSText widget code.
**
**	5 JAN 1993  Removed fix_charset macro.  Removed #ifdef in include
**		    of DXmCSTextP.h.  Removed definition of MIN and MAXINT.
**		    For MAXINT, include values.h for U*x platforms, define for
**		    VMS.  For MIN, include sys/param.h for U*x platforms, 
**		    define for VMS.
**
**	15 Mar 1992 Added access method CSTextVerifyBell()
**
**--
**/

#ifndef _CSTextI_h
#define _CSTextI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#if ! defined(VMS) && ! defined(WIN32)
#include <values.h>	/* For MAXINT */
#include <sys/param.h>	/* For MIN    */
#else
#ifndef MAXINT
#define MAXINT 0x7fffffff
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#endif

#include <DXm/DXmCSTextP.h>

#include "CSTextSrc.h"
#include "CSTextOutP.h"
#include "CSTextInP.h"
#include "CSTextStr.h"

#define ISOLATIN1_CHARSET "ISO8859-1"

#define ALLOC_DEFAULT_CHARSET( charset_ptr )  \
		  { charset_ptr = XtMalloc(strlen(ISOLATIN1_CHARSET)); \
	strncpy ( charset_ptr, ISOLATIN1_CHARSET, strlen(ISOLATIN1_CHARSET));}

#define I_STR_EditDone  DXmCSTextStatusEditDone
#define I_STR_EditError DXmCSTextStatusEditError

/*
    This is a private definition of the CSText widget structure. It is
    intended to serve as a temporary solution for the problem of recognition of
    the new elements (textpath, editingpath) in the Input/Output
    modules.
*/

#define CastIT(w)              ((DXmCSTextWidget)(w))

#define Output(w)              (CastIT (w)->cstext.output)
#define Input(w)               (CastIT (w)->cstext.input)

#define TopPos(w)              (CastIT (w)->cstext.top_position)
#define BotPos(w)              (CastIT (w)->cstext.bottom_position)
#define CursorPos(w)           (CastIT (w)->cstext.cursor_position)

#define NoFontCB(w)            (CastIT (w)->cstext.nofont_callback)
#define ActivateCB(w)          (CastIT (w)->cstext.activate_callback)
#define FocusCB(w)             (CastIT (w)->cstext.focus_callback)
#define LoseFocusCB(w)         (CastIT (w)->cstext.losing_focus_callback)
#define ValueCB(w)             (CastIT (w)->cstext.value_changed_callback)
#define MotionCB(w)            (CastIT (w)->cstext.motion_verify_callback)
#define ModifyCB(w)            (CastIT (w)->cstext.modify_verify_callback)

#define NeedRedisplay(w)       (CastIT (w)->cstext.needs_redisplay)
#define SetNeedRedisplay(w)    (NeedRedisplay (w) = TRUE)
#define ResetNeedRedisplay(w)  (NeedRedisplay (w) = FALSE)

#define PendingVScroll(w)      (CastIT (w)->cstext.pending_vertical_scroll)
#define PendingHScroll(w)      (CastIT (w)->cstext.pending_horiz_scroll)

#define CSTextLines(w)          (CastIT (w)->cstext.lines)

#define CSTextMaxLength(w)      (CastIT (w)->cstext.max_length)
#define CSTextLength(w)         (CastIT (w)->cstext.length)
#define CSTextHasSelection(w)   (CastIT (w)->cstext.has_selection)
#define CSTextSelLeft(w)        (CastIT (w)->cstext.sel_left)
#define CSTextSelRight(w)       (CastIT (w)->cstext.sel_right)

#define CSTextRows(w)	       (CastIT (w)->cstext.rows)
#define CSTextCols(w)	       (CastIT (w)->cstext.columns)
#define CSTextWidth(w)	       (CastIT (w)->cstext.width)
#define CSTextHeight(w)	       (CastIT (w)->cstext.height)
#define CSTextMarginWidth(w)    (CastIT (w)->cstext.margin_width)
#define CSTextMarginHeight(w)   (CastIT (w)->cstext.margin_height)
#define CSTextHasFocus(w)       (CastIT (w)->cstext.has_focus)

#define CSTextOutputCreate(w)   (CastIT (w)->cstext.output_create)
#define CSTextInputCreate(w)    (CastIT (w)->cstext.input_create)
#define CSTextDisableDepth(w)   (CastIT (w)->cstext.disable_depth)
#define CSTextInRedisplay(w)    (CastIT (w)->cstext.in_redisplay)
#define CSTextTraversed(w)      (CastIT (w)->cstext.traversed)
#define CSTextAddMode(w)        (CastIT (w)->cstext.add_mode)
#define CSTextEditMode(w)       (CastIT (w)->cstext.edit_mode)
#define CSTextEditable(w)       (CastIT (w)->cstext.editable)
#define CSTextEditingPath(w)    (CastIT (w)->cstext.editing_path)
#define CSTextPath(w)           (CastIT (w)->cstext.text_path)
#define CSTextValue(w)	       (CastIT (w)->cstext.value)
#define CSTextNewTop(w)	       (CastIT (w)->cstext.new_top)
#define CSTextAutoShowCursorPos(w) (CastIT (w)->cstext.auto_show_cursor_position)
#define CSTextDefaultCharSet(w) (CastIT (w)->cstext.def_charset)

#define CSTextDirection(w)      (CastIT (w)->primitive.dxm_layout_direction)
#define CSTextVerifyBell(w)     (CastIT (w)->cstext.verify_bell)

/*
** FORWARD DECLARATIONS
*/

/* CSTextIn.c
 */

#ifdef _NO_PROTO
#ifdef I18N_IO_SHR
extern void _DXmCSTextInputCreate ( );
#else
extern void DXmCSTextInputCreate ( );
#endif /* I18N_IO_SHR */

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#ifdef I18N_IO_SHR
extern void _DXmCSTextInputCreate ( DXmCSTextWidget widget , ArgList args , Cardinal num_args );
#else
extern void DXmCSTextInputCreate ( DXmCSTextWidget widget , ArgList args , Cardinal num_args );
#endif /* I18N_IO_SHR */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

/* CSTextOut.c
 */

#ifdef _NO_PROTO

#ifdef I18N_IO_SHR
extern void _DXmCSTextOutputCreate ( );
#else
extern void DXmCSTextOutputCreate ( );
#endif /* I18N_IO_SHR */

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#ifdef I18N_IO_SHR
extern void _DXmCSTextOutputCreate ( DXmCSTextWidget widget , ArgList args , Cardinal num_args );
#else
extern void DXmCSTextOutputCreate ( DXmCSTextWidget widget , ArgList args , Cardinal num_args );
#endif /*I18N_IO_SHR */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

/* CSTextStr.c
 */

#ifdef _NO_PROTO

extern DXmCSTextPosition _DXmCSTextSourceGetPosition ( );
extern void _DXmCSTextSourceLocate ( );
extern DXmCSTextStatus _DXmCSTextSourceInsertChar ( );
extern DXmCSTextStatus _DXmCSTextSourceInsertString ( );
extern Boolean _DXmCSTextSrcConvert ( );
extern void _DXmCSTextSetSelection ( );
extern DXmCSTextStatus _DXmCSTextSourceReadString ( );
extern DXmCSTextStatus _DXmCSTextSourceReplaceString ( );
extern void _DXmCSTextScanPositions ( );
extern Boolean _DXmCSTextIsScanBreak ( );
extern Boolean _DXmCSTextIsWhiteSpace ( );
extern void _DXmCSTextScanPreviousParagraph ( );
extern void _DXmCSTextScanNextParagraph ( );
extern void _DXmCSTextScanWord ( );
extern Boolean _DXmCSTextScanWordLimits ( );
extern void _DXmCSTextScanEndOfLine ( );
extern void _DXmCSTextScanStartOfLine ( );
extern void _DXmCSTextScanNextLine ( );
extern void _DXmCSTextScanPrevLine ( );
extern void _DXmCSTextScanAll ( );
extern Boolean _DXmCSTextGetSelection ( );
extern void _DXmCSTextSourceDestroy ( );
extern XmString _DXmCSTextSourceGetValue ( );
extern DXmCSTextStatus _DXmCSTextSourceSetValue ( );
extern Boolean _DXmCSTextNavNextChar ( );
extern Boolean _DXmCSTextNavPrevChar ( );
extern Boolean NavNextSegment ( );
extern Boolean NavNextSegmentInLine ( );
extern Boolean NavEndPrevSegment ( );
extern Boolean NavPrevLine ( );
extern Boolean NavEndLine ( );
extern Boolean NavStartLine ( );
extern Boolean NavEndPrevLine ( );
extern Boolean NavNextLine ( );
extern Boolean _DXmCSTextEqualLocation ( );
extern Boolean _DXmCSTextEqualSegment ( );
extern TextLine _DXmCSTextGetFirstLine ( );
extern TextLine _DXmCSTextGetLastLine ( );
extern TextLine _DXmCSTextGetNextLine ( );
extern TextLine _DXmCSTextGetPrevLine ( );
extern TextSegment _DXmCSTextGetFirstSegment ( );
extern TextSegment _DXmCSTextGetLastSegment ( );
extern TextSegment _DXmCSTextGetNextSegment ( );
extern TextSegment _DXmCSTextGetPrevSegment ( );

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern DXmCSTextPosition _DXmCSTextSourceGetPosition ( DXmCSTextWidget widget , TextLine line , TextSegment segment , DXmCSTextPosition offset );
extern void _DXmCSTextSourceLocate ( DXmCSTextWidget widget , DXmCSTextPosition position , TextLocation location );
extern DXmCSTextStatus _DXmCSTextSourceInsertChar ( DXmCSTextWidget widget , DXmCSTextPosition start , XmString string );
extern DXmCSTextStatus _DXmCSTextSourceInsertString ( DXmCSTextWidget widget , DXmCSTextPosition start , XmString string , int alloc );
extern Boolean _DXmCSTextSrcConvert ( Widget w , Atom *seltype , Atom *desiredtype , Atom *type , XtPointer *value , unsigned long *length , int *format );
extern void _DXmCSTextSetSelection ( DXmCSTextWidget widget , DXmCSTextPosition left , DXmCSTextPosition right , Time time );
extern DXmCSTextStatus _DXmCSTextSourceReadString ( DXmCSTextWidget widget , DXmCSTextPosition start , DXmCSTextPosition end , XmString *string );
extern DXmCSTextStatus _DXmCSTextSourceReplaceString ( DXmCSTextWidget widget , DXmCSTextPosition start , DXmCSTextPosition end , XmString string );
extern void _DXmCSTextScanPositions ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextScanDirection direction , int count , DXmCSTextPosition *new_position );
extern Boolean _DXmCSTextIsScanBreak ( Widget widget , TextLocation location , int scan_direction , I18nScanType scan_type );
extern Boolean _DXmCSTextIsWhiteSpace ( Widget widget , TextLocation location );
extern void _DXmCSTextScanPreviousParagraph ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextPosition *new_position );
extern void _DXmCSTextScanNextParagraph ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextPosition *new_position );
extern void _DXmCSTextScanWord ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextScanDirection direction , DXmCSTextPosition *new_position );
extern Boolean _DXmCSTextScanWordLimits ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextPosition *first_position , DXmCSTextPosition *last_position );
extern void _DXmCSTextScanEndOfLine ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextPosition *new_position );
extern void _DXmCSTextScanStartOfLine ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextPosition *new_position );
extern void _DXmCSTextScanNextLine ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextPosition *new_position );
extern void _DXmCSTextScanPrevLine ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextPosition *new_position );
extern void _DXmCSTextScanAll ( DXmCSTextWidget widget , DXmCSTextPosition position , DXmCSTextScanDirection direction , DXmCSTextPosition *new_position );
extern Boolean _DXmCSTextGetSelection ( DXmCSTextWidget widget , DXmCSTextPosition *left , DXmCSTextPosition *right );
extern void _DXmCSTextSourceDestroy ( DXmCSTextWidget widget );
extern XmString _DXmCSTextSourceGetValue ( DXmCSTextWidget widget );
extern DXmCSTextStatus _DXmCSTextSourceSetValue ( DXmCSTextWidget widget , XmString value );
extern Boolean _DXmCSTextNavNextChar ( TextLocation location );
extern Boolean _DXmCSTextNavPrevChar ( TextLocation location );
extern Boolean NavNextSegment ( TextLocation location );
extern Boolean NavNextSegmentInLine ( TextLocation location );
extern Boolean NavEndPrevSegment ( TextLocation location );
extern Boolean NavPrevLine ( TextLocation location );
extern Boolean NavEndLine ( TextLocation location );
extern Boolean NavStartLine ( TextLocation location );
extern Boolean NavEndPrevLine ( TextLocation location );
extern Boolean NavNextLine ( TextLocation location );
extern Boolean _DXmCSTextEqualLocation ( TextLocation a , TextLocation b );
extern Boolean _DXmCSTextEqualSegment ( TextSegment a , TextSegment b );
extern TextLine _DXmCSTextGetFirstLine ( DXmCSTextWidget widget );
extern TextLine _DXmCSTextGetLastLine ( DXmCSTextWidget widget );
extern TextLine _DXmCSTextGetNextLine ( TextLine line );
extern TextLine _DXmCSTextGetPrevLine ( TextLine line );
extern TextSegment _DXmCSTextGetFirstSegment ( TextLine line );
extern TextSegment _DXmCSTextGetLastSegment ( TextLine line );
extern TextSegment _DXmCSTextGetNextSegment ( TextSegment segment );
extern TextSegment _DXmCSTextGetPrevSegment ( TextSegment segment );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _CSTextI_h */
