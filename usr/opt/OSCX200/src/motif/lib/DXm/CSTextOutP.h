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
**                         ALL RIGHTS RESERVED                              *
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
**      < to be supplied >
**
**  ABSTRACT:
**
**      < to be supplied >
**
**  ENVIRONMENT:
**
**      < to be supplied >
**
**  MODIFICATION HISTORY:
**
**	8 Jan 1993  Created enumerated data type DXmCSTextDataOpCode, used
**		     in prototypes.
**
**
**--
**/
#ifndef _CSTextOutP_h
#define _CSTextOutP_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include "CSTextOut.h"

/* BOGUS
 * what is this about????
 */

#define DXmCURSOR_TYPE_ADD_MODE  1


/*
 *
 * Definitions for modules implementing and using text output routines.
 *
 */

typedef struct _cursor_rec
{
  Widget             widget;           /* widget we're attached to */
  Pixmap             pixmap;           /* pixmap to use to erase the cursor */
  DXmCSTextPosition   position;         /* source position of cursor */
  Position           x, y;             /* screen position */
  Dimension          width, height;

  Boolean            active;           /* we should be blinking */

  int                blink_rate;       /* ms between blink transitions */
  XtIntervalId       timer_id;         /* Xt's timer ID when blinking */

  XtWidgetProc       proc;             /* draw or undraw proc, to do next */

  Boolean            on_display;       /* do we need to be undrawn */
}
    CSTextCursorRec, *CSTextCursor;






/*
 * this is all obsolete, right
 */

typedef struct
{

    int 		vshown;		/* How big the thumb is in the vbar. */

    int 		hoffset;	/* How much we've scrolled off the 
					   start. */
    Boolean 		exposevscroll;	/* Non-zero if we expect expose events 
					   to be off vertically. */
    Boolean 		exposehscroll;	/* Non-zero if we expect expose events 
					   to be off horizontally. */
}
    _DXmCSTextScrollRec;

/* Rendition Manager */

typedef struct _RendMgrRec *RendMgr;

typedef struct _DXmCSTextOutputDataRec 
{
    DXmCSTextWidget 	widget;		/* Back-pointer to the widget */
    Pixel 		foreground;	/* Color to use for text. */

    Boolean 		wordwrap,	/* Whether to wordwrap. */
     		        hasfocus,       /* do we have focus now */
     	                doing_expose,
    		        ip_visible,     /* is insertion point visible */
     		        auto_show_ip,   /* keep insertion point visible */
     		        half_border,
                        bidirectional,  /* normal or bi-dir insert mode */
      		        resize_width,   /* resize policy */
			resize_height;

    Dimension           max_descent,    /* font data */
                        max_ascent,

     		        min_width, 
			min_height;

    XmFontList          font_list;

    Dimension           typical_char_width,  /* found after perusing the */
                        typical_char_height; /* font list */

    int                 blink_rate;

    CSTextCursor        cursor_list;         /* all active cursor requests */

    Pixmap              normal_pix,	     /* regular insertion cursor */
                        l_to_r_pix,	     /* bi-dir l to r insertion */
     	                r_to_l_pix,	     /* bi_dir r to l insertion */
     	                inactive_pix;        /* when we don't have focus */


                           /* scroll bar stuff */

    Boolean 		scroll_vertical,     /* doing vertical scrolling ? */
			scroll_horizontal,   /* doing horizontal scrolling ? */
                        scroll_leftside,     /* is vert on the left */
			scroll_topside;      /* is horiz on top */

    Widget 		vertical_scrollbar, 
			horizontal_scrollbar;

    int                 vertical_offset,     /* how to get line x,y changed */
                        horizontal_offset;   /* to window x,y */

    int                 prev_top_edge;       /* to detect margin changes */
    int                 prev_left_edge;      /* during setvalues */

    int                 line_count;          /* number of output lines */

    Boolean 		ignore_vbar;	/* do we ignore callbacks from vbar.*/
    Boolean 		ignore_hbar;	/* do we ignore callbacks from hbar.*/

    GC 			normal_gc, 
			inv_gc, 
			xor_gc, 
			image_gc;

    Pixmap 		error_char;	/* Pixmap for painting error chars */
    Opaque		aim_rec;        /* pointer to Asian IM specific stuff*/
    char		*input_method; /* specify the start up Asian */
                                       /* IM name used		     */
    int			input_method_type; /* specify how to specify the IM */
    Boolean		redisplay_all; /* specify all text need redisplay. */
    Boolean		need_vbar_update; /* specify vbar need recompute. */
    Boolean		need_hbar_update; /* specify hbar need recompute. */
    RendMgr		rendition_mgr;		/* Rendition Manager */
} 
    DXmCSTextOutputDataRec, *DXmCSTextOutputData;

#define OutputData(w)  ((DXmCSTextOutputData)   \
                           ( ((DXmCSTextOutput)(Output(w)) )->data))
 
#define IsTwoByteFont(f)   (((f)->min_byte1 != 0 || (f)->max_byte1 != 0))
#define AutoShowCursor(w)  (OutputData (w)->auto_show_ip)

#define InsertX(w)         (OutputData (w)->insert_x)
#define InsertY(w)         (OutputData (w)->insert_y)

#define NormalGC(w)        (OutputData (w)->normal_gc)
#define InverseGC(w)       (OutputData (w)->inv_gc)
#define XorGC(w)           (OutputData (w)->xor_gc)
#define ImageGC(w)         (OutputData (w)->image_gc)

#define HalfBorder(w)      (OutputData (w)->half_border)

#define TypicalWidth(w)    (OutputData (w)->typical_char_width)
#define TypicalHeight(w)   (OutputData (w)->typical_char_height)

#define MaxDescent(w)      (OutputData (w)->max_descent)
#define MaxAscent(w)       (OutputData (w)->max_ascent)

#define DoingExpose(w)     (OutputData (w)->doing_expose)

#define IsLtoRTextPath(w)  (CSTextPath (w))
#define IsRtoLTextPath(w)  ( ! IsLtoRTextPath (w))

#define CursorId(w)        (OutputData (w)->cursor_id)
#define CursorList(w)      (OutputData (w)->cursor_list)
#define CursorEnabled(w)   (OutputData (w)->ip_visible)
#define FontList(w)        (OutputData (w)->font_list)
#define NormalCursor(w)    (OutputData (w)->normal_pix)
#define LtoRCursor(w)      (OutputData (w)->l_to_r_pix)
#define RtoLCursor(w)      (OutputData (w)->r_to_l_pix)
#define InactiveCursor(w)  (OutputData (w)->inactive_pix)
#define BlinkRate(w)       (OutputData (w)->blink_rate)

#define BothMargin(w)      (CSTextLeftMargin (w) + CSTextRightMargin (w))

#define PrevTopEdge(w)     (OutputData (w)->prev_top_edge)
#define PrevLeftEdge(w)    (OutputData (w)->prev_left_edge)
#define InnerWidget(w)     (((DXmCSTextWidget)(w))->cstext.inner_widget)
#define InnerWindow(w)     (XtWindow (InnerWidget (w)))
#define InnerWidth(w)      (XtWidth (InnerWidget (w)))
#define InnerHeight(w)     (XtHeight (InnerWidget (w)))
#define InnerX(w)          (XtX (InnerWidget (w)))
#define InnerY(w)          (XtY (InnerWidget (w)))

#define IsLToRSegment(s)   (s->direction == XmSTRING_DIRECTION_L_TO_R)

#define VBar(w)             (OutputData (w)->vertical_scrollbar)
#define HBar(w)             (OutputData (w)->horizontal_scrollbar)
#define VerticalOffset(w)   (OutputData (w)->vertical_offset)
#define HorizontalOffset(w) (OutputData (w)->horizontal_offset)
#define CanResizeWidth(w)   (OutputData (w)->resize_width)
#define CanResizeHeight(w)  (OutputData (w)->resize_height)
#define IgnoreVBar(w)       (OutputData (w)->ignore_vbar)
#define IgnoreHBar(w)       (OutputData (w)->ignore_hbar)
#define DoVScroll(w)        (OutputData (w)->scroll_vertical)
#define DoHScroll(w)        (OutputData (w)->scroll_horizontal)
#define VSCrollOnLeft(w)    (OutputData (w)->scroll_leftside)
#define HSCrollOnTop(w)     (OutputData (w)->scroll_topside)

#define LineCount(w)        (OutputData (w)->line_count)
#define AIMRec(w)	    (OutputData (w)->aim_rec)
#define InputMethod(w) (OutputData (w)->input_method)
#define InputMethodType(w)  (OutputData (w)->input_method_type)

#define RedisplayAll(w)	    (OutputData(w)->redisplay_all)
#define NeedVBarUpdate(w)   (OutputData(w)->need_vbar_update)
#define NeedHBarUpdate(w)   (OutputData(w)->need_hbar_update)
#define RenditionMgr(w)     (OutputData(w)->rendition_mgr)

/*
 * various op codes that the output module HandleData method must
 * deal with
 */

typedef enum {
    DXmCSTextOutputDelete = 0,		/* dump output data for everyth'g */

    DXmCSTextOutputSegmentDelete,	/* dump output data for this seg */
    DXmCSTextOutputSegmentAdd,		/* new seg added to source */
    DXmCSTextOutputSegmentModified,	/* source changed this seg */

    DXmCSTextOutputLineDelete,		/* dump output data for this line */
					/* this implies all segs of line */
    DXmCSTextOutputLineAdd,		/* new line added to source */

    DXmCSTextSegmentValidateData,	/* make sure output data for seg */
					/* is current */
    DXmCSTextLineValidateData,		/* make sure output data for line */
					/* is current, implies all segs */
    DXmCSTextValidateData,		/* make  output data for whole */
					/* source current */

    DXmCSTextOutputLineModified,	/* source line changed something */

    DXmCSTextOutputSingleLineDelete,	/* only that single line is deleted. */
					/* don't do any validation now */

    DXmCSTextOutputSingleSegmentDelete,	/* only that single segment is */
					/* deleted.  don't do any */
					/* validation now */

    DXmCSTextOutputRestLineValidate,	/* now, do validateion starting */
					/* from this line */

    DXmCSTextOutputSingleLineInvalidate, /* invalidate this line */

    DXmCSTextOutputTextEmpty 		/* Text becomes empty */
} DXmCSTextDataOpCode;

/*
 * sample headers for the output methods
 
void
SetInsertionPoint (widget, position, state)
    DXmCSTextWidget widget;
    DXMCSTextPosition position;
    Boolean state;
    {}

void
Draw (widget)
    DXmCSTextWidget widget;
    {}

void
RedrawHalfBorder (widget)
    DXmCSTextWidget widget;
    {}

void
RedisplayHBar (widget)
    DXmCSTextWidget widget;
    {}

Boolean                                  indicates if redraw is needed
MakePositionVisible (widget, position)
    DXmCSTextWidget widget;
    DXMCSTextPosition position;
    {}

Boolean                                  indicates if redraw is needed
VerticalScroll (widget)                  pending_vertical_scroll tells how much
    DXmCSTextWidget widget;
    {}

Boolean                                  indicates if redraw is needed
HorizontalScroll (widget)                pending_horiz_scroll tells how much
    DXmCSTextWidget widget;
    {}

void
SetCharRedraw (widget, location)
    DXmCSTextWidget widget;
    TextLocation location;
    {}

void
CharHighlight (widget, location, mode)
    DXmCSTextWidget widget;
    TextLocation location;
    XmHighlightMode mode;
    {}

void
HandleData (widget, line, segment, op_code)   method that handles output data
    DXmCSTextWidget widget;                   structure manipulations on a
    TextLine line;                            segment, line or across the whole
    TextSegment segment                       source depending on op_code
    DXmCSTextDataOpCode op_code
    {}

 *
 */

typedef (*DXmProc)();	
	/* there really should be one for each type so that
	   proper type checking can be done .... */

#ifdef _NO_PROTO
typedef (*DXmDrawProc)();	
typedef (*DXmSetInsPointProc)();	
typedef (*DXmSetCurProc)();	
typedef (*DXmMakePosVisProc)();	
typedef (*DXmHandleDataProc)();	
typedef (*DXmInvalidateProc)();	
typedef (*DXmGetValuesProc)();	
typedef (*DXmSetValuesProc)();	
typedef (*DXmRealizeProc)();	
typedef (*DXmDestroyProc)();	
typedef (*DXmResizeProc)();	
typedef (*DXmRedisplayProc)();	
typedef (*DXmVertScrollProc)();	
typedef (*DXmHorScrollProc)();	
typedef (*DXmSetCharRedrawProc)();	
typedef (*DXmSetCharDrawModeProc)();	
typedef (*DXmComputeLineIndexProc)();	
typedef (*DXmSearchFontListProc)();	
typedef (*DXmNumLinesOnTextProc)();	
typedef (*DXmChangeInputMethodProc)();	
typedef (*DXmScanNextLineProc)();	
typedef (*DXmScanPrevLineProc)();	
typedef (*DXmScanStartOfLineProc)();	
typedef (*DXmScanEndOfLineProc)();	
typedef (*DXmRedisplayHBarProc)();	
typedef (*DXmRedrawHalfBorderProc)();	
#else
typedef (*DXmDrawProc)(DXmCSTextWidget widget);
typedef (*DXmSetInsPointProc)(DXmCSTextWidget widget, DXmCSTextPosition position);	
typedef (*DXmSetCurProc)(DXmCSTextWidget widget, int cursor_type);	
typedef (*DXmMakePosVisProc)(DXmCSTextWidget widget, DXmCSTextPosition position);	
typedef (*DXmHandleDataProc)(DXmCSTextWidget widget, TextLine line, TextSegment segment, DXmCSTextDataOpCode op_code);	
typedef (*DXmInvalidateProc)(DXmCSTextWidget widget, DXmCSTextPosition left, DXmCSTextPosition right);	
typedef (*DXmGetValuesProc)(DXmCSTextWidget widget, ArgList args, Cardinal num_args);	
typedef (*DXmSetValuesProc)( DXmCSTextWidget widget, ArgList args, Cardinal num_args);	
typedef (*DXmRealizeProc)(DXmCSTextWidget widget, Mask *valueMask, XSetWindowAttributes *attributes);	
typedef (*DXmDestroyProc)(DXmCSTextWidget widget);	
typedef (*DXmResizeProc)(DXmCSTextWidget widget);	
typedef (*DXmRedisplayProc)(DXmCSTextWidget widget, XEvent *event);	
typedef (*DXmVertScrollProc)(DXmCSTextWidget widget);	
typedef (*DXmHorScrollProc)(DXmCSTextWidget widget);	
typedef (*DXmSetCharRedrawProc)(DXmCSTextWidget widget, TextLocation location);	
typedef (*DXmSetCharDrawModeProc)(DXmCSTextWidget widget, TextLocation location,unsigned int length, XmHighlightMode mode);	
typedef (*DXmComputeLineIndexProc)(DXmCSTextWidget widget, TextLocation location,int *return_value);	
typedef (*DXmSearchFontListProc)(DXmCSTextWidget widget, XmStringCharSet     charset, XFontStruct **font);	
typedef (*DXmNumLinesOnTextProc)(DXmCSTextWidget widget, int *num_lines);	
typedef (*DXmChangeInputMethodProc)(DXmCSTextWidget widget, XEvent *event);	
typedef (*DXmScanNextLineProc)(DXmCSTextWidget   widget, DXmCSTextPosition position, TextLocation      location, DXmCSTextPosition *out_position);	
typedef (*DXmScanPrevLineProc)(DXmCSTextWidget   widget, DXmCSTextPosition position, TextLocation      location, DXmCSTextPosition *out_position);	
typedef (*DXmScanStartOfLineProc)(DXmCSTextWidget   widget, TextLocation      location, DXmCSTextPosition *out_position);	
typedef (*DXmScanEndOfLineProc)(DXmCSTextWidget   widget, TextLocation      location, DXmCSTextPosition *out_position);	
typedef (*DXmRedisplayHBarProc)(DXmCSTextWidget widget);	
typedef (*DXmRedrawHalfBorderProc)(DXmCSTextWidget widget);	
#endif

typedef struct _DXmCSTextOutputRec 
{
    DXmCSTextOutputData	data;

    /* known to be useful */
    DXmDrawProc		Draw;
    DXmSetInsPointProc		SetInsertionPoint;
    DXmSetCurProc             SetCursor;
    DXmMakePosVisProc		MakePositionVisible;
    DXmHandleDataProc             HandleData;
    DXmCSTextXYToPosProc	XYToPos;
    BooleanProc		PosToXY;
    DXmInvalidateProc		Invalidate;
    DXmGetValuesProc		GetValues;
    DXmSetValuesProc		SetValues;
    DXmRealizeProc		Realize;
    DXmDestroyProc		Destroy;
    DXmResizeProc		Resize;
    DXmRedisplayProc	        Redisplay;
    DXmVertScrollProc             VerticalScroll;
    DXmHorScrollProc             HorizontalScroll;
    DXmSetCharRedrawProc             SetCharRedraw;
    DXmSetCharDrawModeProc             SetCharDrawMode;
    DXmComputeLineIndexProc             ComputeLineIndex;
    DXmSearchFontListProc             SearchFontList;
    DXmNumLinesOnTextProc		NumLinesOnText;

    DXmChangeInputMethodProc		ChangeInputMethod;  /* don't conditionalize by   */
					    /* I18N_MULTIBYTE.  More safe*/
/* Scanning routines dealing with logical lines
 */
    DXmScanNextLineProc		ScanNextLine;       
    DXmScanPrevLineProc		ScanPrevLine;
    DXmScanStartOfLineProc		ScanStartOfLine;
    DXmScanEndOfLineProc		ScanEndOfLine;

/* Read String routine dealing with logical lines.  Note that '\n' is added
 * to the end of each out_line's string
 */
    DXmCSTextStatusProc	ReadString;

    /* historical, may be useless */

    DXmRedisplayHBarProc		RedisplayHBar;
    DXmRedrawHalfBorderProc        	RedrawHalfBorder;

    /*
    BooleanProc		MeasureLine;
    BooleanProc		MoveLines;
    */
} 
    DXmCSTextOutputRec;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _TextOutP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
