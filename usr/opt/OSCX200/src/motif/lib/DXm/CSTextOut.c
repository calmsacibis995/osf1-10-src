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
**	DECwindows Toolkit
**
**  ABSTRACT:
**
**	International Text Widget Output Module
**
**
**  MODIFICATION HISTORY:
**
**	12 APR 1990  Begin rework of old CSText widget code.
**
**	8 Jan 1993  Use enumerated data type DXmCSTextDataOpCode with
**		     HandleData.  Removed goto's.  Converted _XmDrawShadow()
**		     to 1.2 _XmDrawShadows()
**	8 Mar 1993  Put fix into routine Draw() so that Ibar cursor is drawn at 
**		    expose time with blinkrate ==0.  Also fixed bug in _create_cursor
**		    so that Ibar cursor is positioned correctly at the lefthand margin. 
**	23 Mar 1993 Put fix into _check_wrapped_segment fo infinite CSText problem 
**		    with autowrap on.
**	        
**
**--
**/


/* File: CSTextOut.c */

/* Private definitions. */

#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include "I18N.h"
#include <Xm/ScrolledWP.h>
#include <Xm/ScrollBarP.h>
#include <Xm/XmP.h>
#include <Xm/XmI.h>
#include <Xm/XmosP.h>
#include <Xm/PushBGP.h>
#include <Xm/LabelP.h>
#include <Xm/PushBP.h>


#include "DXmPrivate.h"
#include "CSTextI.h"
#include "CSTextOutP.h"
#include "CSTextSrc.h"

#ifdef ASIAN_IO_SHR
#include "DXmAIM.h"
#endif /* ASIAN_IO_SHR */

#ifdef DEC_MOTIF_DEBUG
#define DEBUG 1
#endif

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define FontListType(f)	((XmFontList)(f))->type
#define FontListFont(f)	((XmFontList)(f))->font

#define TextRtoL(w)             (CSTextPath (w) == DXmDIRECTION_LEFT_DOWN)
/* Macro to test if x seg is the first in the line */
#define start_of_line(w, seg)   (seg->x == \
                (TextRtoL (w) ? (RightMargin (w) - (seg)->pixel_width) : 0 ))

#ifndef DXmSClassCSTextWidget
#define DXmSClassCSTextWidget "DXmCSText"
#endif 


#define Half(x)   (x >> 1)

#define _COMPUTE_OPTIMAL_SIZE     1
#define _COMPUTE_GROW_SIZE        2

#define _PER_CHAR_NEEDS_REDISPLAY 1
#define _PER_CHAR_DRAW_MODE       2



#define Bound(left, var, right)\
{\
  var = MIN ((var), (right));\
  var = MAX ((var), (left));\
}

/* In RtoL Text Direction - horizontal_offset should be from right side
   so don't include it in calculating x
*/
#define SegToWindowX(w,out_line,out_seg) \
   (TextRtoL (w) ? (out_line->x + out_seg->x - HorizontalOffset (w))\
                 : (HorizontalOffset (w) + out_line->x + out_seg->x))

#define SegToWindowY(w,out_line,out_seg) \
        (VerticalOffset (w) + out_line->y + out_seg->y)



/*
 * these are all unsigned int's so doing arithmetic is a pain...
 */

#define RightEdge(w)   ((int) ((int) XtWidth (widget)) - \
			      ((int) CSTextMarginWidth (widget)))

#define LeftEdge(w)    ((int) ((int) CSTextMarginWidth (widget)))

#define BottomEdge(w)  ((int) ((int) XtHeight (widget)) - \
			      ((int) CSTextMarginHeight (widget)))

#define TopEdge(w)     ((int) ((int) CSTextMarginHeight (widget)))

#define NetWidth(w)    (((int) RightEdge (widget)) - ((int) LeftEdge (widget)))

#define NetHeight(w)   (((int) BottomEdge (widget)) - ((int) TopEdge (widget)))

#define RightMargin(w)   ((int) XtWidth (w)) 


/*
 * as we render a source segment, it may for various reasons need to be
 * broken down into smaller pieces for actual display.  The smaller 
 * pieces are called chunks.
 */
typedef struct _chunk
{
  Boolean skip;                            /* don't bother it's off display */
  Position width, height;
  Position x, y, bottom, right;
  int char_count;
  char * text;
  XmHighlightMode draw_mode;
  struct _chunk * next;
}
    ChunkRec, *Chunk;

#ifdef ASIAN_IO_SHR

#define  ASIAN_LANGUAGE(language) 				\
	 (strcmp( language, "ja_JP" ) || /* Japanese */		\
	  strcmp( language, "zh_TW" ) || /* Chinese-Taiwan */	\
	  strcmp( language, "zh_CN" ) || /* Chinese-PRC */	\
	  strcmp( language, "ko_KR" ) || /* Korean */		\
	  strcmp( language, "th_TH" )    /* Thai */		\
	  )


typedef struct _ICharStruct {
		struct _ICharStruct *next;
		char   *s;
		XmStringCharSet charset;
		int    x;
		int    y;
		int    width;
		int    height;
		} ICharStructRec, *ICharStruct;

typedef struct _AIMStruct {
	        Widget aim_widget;
		ICharStruct	i_char_table;
		Boolean         end_istate_allowed;
		} AIMStructRec, *AIMStruct;

#endif /* ASIAN_IO_SHR */

/* predefine routines defined in CSTextStr.c
 */

extern TextLine _DXmCSTextGetFirstLine();
extern TextLine _DXmCSTextGetLastLine();
extern TextLine _DXmCSTextGetNextLine();
extern TextLine _DXmCSTextGetPrevLine();

extern TextSegment _DXmCSTextGetFirstSegment();
extern TextSegment _DXmCSTextGetLastSegment();
extern TextSegment _DXmCSTextGetNextSegment();
extern TextSegment _DXmCSTextGetPrevSegment();

/* predefine routines found in XmString.c */

/* The following is added temporarily, in DECwindows 1.2, to only gain
   access to XmString/FontListSearch. Currently, in 1.2, this XmString
   routine is static. Allowing access to CSText eliminates unneeded
   duplication across XmString and CSText code.  Once OSF officially
   declares XmString/FontListSearch non static, the following can
   be deleted
*/ 
extern _XmSearchFontList();


/* predefine routines defined in I18n.c
 */

extern void I18nSegmentMeasure();
extern void I18nCreateXlibBuffers();

#define CARET_WIDTH 9
#define CARET_HEIGHT 5

static unsigned char caretBits[] = {
   0x10, 0x00, 0x38, 0x00, 0x6c, 0x00, 0xc6, 0x00, 0x83, 0x01};

static unsigned char fillCaretBits[] = {
   0x10, 0x00, 0x38, 0x00, 0x7c, 0x00, 0xfe, 0x00, 0xff, 0x01};

#define CURSOR_WIDTH 5

static Boolean def_false = FALSE;
static Boolean def_true = TRUE;
static Boolean def_left_side = FALSE;   /* default for XmNscrollLeftSide */

/* BOGUS
 * this should callinto I18n RTL to get the locale defined default direction
 */
static const unsigned char def_dir = XmSTRING_DIRECTION_R_TO_L;

static int def_0 = 0;
static Dimension def_1 = 1;
static Dimension def_2 = 2;
static Dimension def_20 = 20;
static int def_500 = 500;

externaldef(nonvisible) XtResource i_str_output_resources[] = 
{
  {XmNforeground, 
     XmCForeground, 
     XmRPixel, 
     sizeof(Pixel), 
     XtOffset(DXmCSTextOutputData, foreground), 
     XmRString, 
     XtDefaultForeground},
  
  {XmNfontList, 
     XmCFontList, 
     XmRFontList, 
     sizeof(XmFontList), 
     XtOffset(DXmCSTextOutputData, font_list),
     XmRString,
/*     DXmDefaultFont},		*/
     (XtPointer) NULL},		/* I18N */
  
  {XmNwordWrap, 
     XmCWordWrap, 
     XmRBoolean, 
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, wordwrap), 
     XmRBoolean, 
     (XtPointer)&def_false},

  {XmNblinkRate, 
     XmCBlinkRate, 
     XmRInt, 
     sizeof(int),
     XtOffset(DXmCSTextOutputData, blink_rate), 
     XmRInt, 
     (XtPointer)&def_500},

  {XmNresizeWidth, 
     XmCResizeWidth, 
     XmRBoolean, 
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, resize_width),
     XmRBoolean, 
     (XtPointer)&def_false},
  
  {XmNresizeHeight, 
     XmCResizeHeight, 
     XmRBoolean, 
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, resize_height),
     XmRBoolean, 
     (XtPointer)&def_false},
  
  {XmNscrollVertical, 
     XmCScroll, 
     XmRBoolean, 
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, scroll_vertical),
     XmRBoolean,
     (XtPointer)&def_false},
  
  {XmNscrollHorizontal, 
     XmCScroll, 
     XmRBoolean, 
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, scroll_horizontal),
     XmRBoolean, 
     (XtPointer)&def_false},
  
  {XmNscrollLeftSide, 
     XmCScrollSide, 
     XmRBoolean, 
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, scroll_leftside), 
     XmRBoolean,
     (XtPointer)&def_left_side},
  
  {XmNscrollTopSide, 
     XmCScrollSide,
     XmRBoolean, 
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, scroll_topside),
     XmRBoolean, 
     (XtPointer)&def_false},
  
  {XmNcursorPositionVisible,
     XmCCursorPositionVisible,
     XmRBoolean,
     sizeof(Boolean), 
     XtOffset(DXmCSTextOutputData, ip_visible),
     XmRBoolean, 
     (XtPointer)&def_true},

  {XmNautoShowCursorPosition,
     XmCAutoShowCursorPosition,
     XmRBoolean,
     sizeof(Boolean),
     XtOffset(DXmCSTextOutputData, auto_show_ip),
     XmRBoolean, 
     (XtPointer)&def_true},
  
  {DXmNbidirectionalCursor, 
     DXmCBidirectionalCursor, 
     XmRBoolean,
     sizeof(Boolean), 
     XtOffset(DXmCSTextOutputData, bidirectional),
     XmRBoolean, 
     (XtPointer)&def_false},

  {DXmNinputMethod,
     DXmCInputMethod, 
     XmRString,
     sizeof(char *), 
     XtOffset(DXmCSTextOutputData, input_method),
     XmRString, 
     (XtPointer)DXmIM_DEFAULT},

  {DXmNinputMethodType,
     DXmCInputMethodType, 
     XmRInt,
     sizeof(int), 
     XtOffset(DXmCSTextOutputData, input_method_type),
     XmRImmediate,
     (XtPointer)DXmIM_STRING_TYPE},
};

externaldef(nonvisible) int i_str_output_resources_count = 
                            XtNumber(i_str_output_resources);




/*
 * forward declarations
 */
static Boolean _check_wrap();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine  line;		*/
    /*   TextSegment segment;		*/
    /*   int  offset;			*/
    /*   TextSegment *split_segment;	*/

static Boolean _check_wrapped_segment();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine  line;		*/
    /*   TextSegment segment;		*/
    /*   int  offset;			*/
    /*   TextSegment *wrapped_segment;	*/
    /*   Boolean *second_split;		*/


static void SearchFontList ();
    /*   DXmCSTextWidget widget;	*/
    /*   XmStringCharSet charset;       */
    /*   XmFontListEntry **entry;            */

#ifdef _NO_PROTO
static DXmDrawProc Draw ();
    /*   DXmCSTextWidget widget;	*/
#else
static DXmDrawProc Draw (DXmCSTextWidget widget);
#endif

static int _find_start ();
    /*   DXmCSTextWidget widget;	*/
    /*   DXmCSTextOutputLine out_line;	*/
    /*   DXmCSTextOutputSegment out_seg;*/
    /*   int start;			*/


static int _find_end ();
    /*   DXmCSTextWidget widget;	*/
    /*   DXmCSTextOutputLine out_line;	*/
    /*   DXmCSTextOutputSegment out_seg;*/
    /*   int start;			*/

static Dimension _find_width ();
    /*   Dimension   tab_width;		*/
    /*   XFontStruct *font;		*/
    /*   char *buf;			*/
    /*   int count;			*/
    /*   Dimension   accu_width		*/

static Chunk _get_chunks ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine line;			*/
    /*   DXmCSTextOutputLine out_line;	*/
    /*   TextSegment segment;		*/
    /*   DXmCSTextOutputSegment out_seg;*/
    /*   int accu_width			*/

static void _draw_segment ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine line;			*/
    /*   TextSegment segment;		*/
    /*   DXmCSTextOutputLine  out_line; */
    /*   DXmCSTextOutputSegment out_seg;*/
    /*   Boolean complete_cursors;	*/
    /*   int  *min_x;			*/
    /*   int  *max_x;			*/

static void _set_per_char ();
    /*   TextSegment segment;		*/
    /*   int offset;			*/
    /*   int which;			*/
    /*   int value;			*/

static void SetCharRedraw ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLocation location;		*/

static void SetCharDrawMode ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLocation location;		*/
    /*   XmHighlightMode mode;		*/

static void SetDrawMode();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLocation location;		*/
    /*   unsigned int length;		*/
    /*   XmHighlightMode mode;		*/

static void HandleData ();
    /*   DXmCSTextWidget widget;	*/
    /*   Opaque object;			*/
    /*   DXmCSTextDataOpCode op_code;	*/

static void NumLinesOnText ();
    /*   DXmCSTextWidget widget;	*/
    /*	 int	*num_lines;		*/

static void ScanNextLine ();
    /*   DXmCSTextWidget widget;	*/
    /*   DXmCSTextPosition position;    */
    /*   TextLocation location;		*/
    /*   DXmCSTextPosition out_position;*/

static void ScanPrevLine ();
    /*   DXmCSTextWidget widget;	*/
    /*   DXmCSTextPosition position;    */
    /*   TextLocation location;		*/
    /*   DXmCSTextPosition out_position;*/

static void ScanStartOfLine ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLocation    location;	*/
    /*   DXmCSTextPosition out_position;*/

static void ScanEndOfLine ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLocation    location;	*/
    /*   DXmCSTextPosition out_position;*/

static DXmCSTextStatus ReadString();
    /*	 DXmCSTextWidget widget;	*/
    /*	 DXmCSTextPosition start;	*/
    /*	 DXmCSTextPosition end;		*/
    /*	 XmString *string;		*/

static void _set_top_position();
    /*   DXmCSTextWidget widget;	*/

static void _dump_segment ();
    /*   TextSegment segment;		*/

static void _dump_line ();
    /*   TextLine line;			*/

static void _dump_all ();
    /*  DXmCSTextWidget widget;		*/

static void _dump_rest_line ();
    /*  DXmCSTextWidget widget;         */
    /*  TextLine        line;           */
    /*  TextSegment     segment;	*/

static void _dump_rest_segment ();
    /*  DXmCSTextWidget widget;         */
    /*  TextLine        line;           */
    /*  TextSegment     segment;	*/

static void _validate_segment ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextSegment segment;		*/

static void _validate_line ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine line;			*/

static void _validate_rest_line ();
    /*  DXmCSTextWidget widget;         */
    /*  TextLine        line;           */
    /*  TextSegment     segment;	*/

static void _validate_all ();
    /*   DXmCSTextWidget widget;	*/

static void _invalidate_all ();
    /*   DXmCSTextWidget widget;	*/

static void _invalidate_line ();
    /*  DXmCSTextWidget widget;         */
    /*  TextLine        line;           */
    /*  TextSegment     segment;	*/

static void _invalidate_rest_line ();
    /*  DXmCSTextWidget widget;         */
    /*  TextLine        line;           */
    /*  TextSegment     segment;	*/

static void _invalidate_segment ();
    /*  DXmCSTextWidget widget;         */
    /*  TextLine        line;           */
    /*  TextSegment     segment;	*/

static void _compute_output_size ();
    /*   DXmCSTextWidget widget;        */
    /*   Dimension *width, *height;     */
    /*   int mode;                      */

static void _compute_segment ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextSegment segment;		*/

static void _compute_line ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine line;			*/

static void _alloc_segment ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextSegment segment;		*/

static void _alloc_line ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine line;			*/

static void _append_out_segment ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine    line;		*/
    /*   TextSegment segment;		*/
    /*   int         char_count;	*/
    /*   int         offset;		*/

static void _append_out_line();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine    line;		*/
    /*   TextSegment segment;		*/

static char * _flip_string ();
    /*   char *string;			*/
    /*   Boolean text16;		*/

static Boolean _line_intersection ();
    /*   Position x, y;			*/
    /*   DXmCSTextOutputLine out_line;	*/

static Boolean _seg_intersection ();
    /*   Position x, y;			*/
    /*   DXmCSTextOutputLine out_line;	*/
    /*   DXmCSTextOutputSegment out_seg;*/

static DXmCSTextPosition XYToPos ();
    /*   DXmCSTextWidget widget;	*/
    /*   Position x, y;			*/

static void _set_vbar ();
    /*   DXmCSTextWidget widget;	*/

static void _set_hbar ();
    /*   DXmCSTextWidget widget;	*/

static Boolean PosToXY ();
    /*   DXmCSTextWidget widget;	*/
    /*   DXmCSTextPosition position;	*/
    /*   Position *x, *y;		*/

static XtGeometryResult TryResize ();
    /*   DXmCSTextWidget widget;	*/
    /*   Dimension width, height;	*/

static void _complete_cursors ();
    /*   DXmCSTextWidget widget;        */
    /*   Position x, y;                 */
    /*   Dimension width, height;       */

static void _draw_cursor ();
    /*   DXmCSTextWidget widget;        */

static void _undraw_cursor ();
    /*   DXmCSTextWidget widget;        */

static void _cursor_size ();
    /*   DXmCSTextWidget widget;	*/
    /*   DXmCSTextOutputData data;	*/
    /*   Position *x, *y, *w, *h;	*/

static void ClearCursor();
    /*   DXmCSTextWidget widget;	*/

static void SetInsertionPoint();
    /*   DXmCSTextWidget widget;	*/
    /*   DXmCSTextPosition position;	*/
    /*   Boolean state;			*/

static void SetCursor ();
    /*   DXmCSTextWidget widget;	*/
    /*   int cursor_type;		*/

static void CursorTimer ();
    /*  Opaque closure;			*/
    /*  XtIntervalId id;		*/

static void MakePositionVisible ();
    /* DXmCSTextWidget widget;		*/
    /* DXmCSTextPosition position;	*/


static void Invalidate();
    /*  DXmCSTextWidget widget;		*/
    /*  DXmCSTextPosition left, right;	*/

static void _make_stuff_consistent ();
    /*    DXmCSTextWidget widget;	*/

static void LoadFontMetrics ();
    /*  DXmCSTextWidget widget;		*/

static void LoadGCs();
    /*    DXmCSTextWidget widget;	*/

static void MakeCursors();
    /*    DXmCSTextWidget widget;	*/

static void OutputGetValues ();
    /*    DXmCSTextWidget widget;	*/
    /*    ArgList args;			*/
    /*    Cardinal num_args;		*/

static Boolean CKSensitive();
    /*    ArgList args;			*/
    /*    Cardinal num_args;		*/
    /*    DXmCSTextWidget widget;	*/

static Boolean CKBG();
    /*    ArgList args;			*/
    /*    Cardinal num_args;		*/
    /*    Pixel *background;		*/

static void OutputSetValues ();
    /*  DXmCSTextWidget widget;		*/
    /*  ArgList args;			*/
    /*  Cardinal num_args;		*/

static void HandleFocusEvents();
    /*   Widget w;			*/
    /*   Opaque closure;		*/
    /*   XEvent *event;			*/

static void Realize();
    /*   DXmCSTextWidget widget;	*/
    /*   Mask *valueMask;		*/
    /*   XSetWindowAttributes *attributes;*/

static void Destroy();
    /*   DXmCSTextWidget widget;	*/

static void RedrawRegion ();
    /*   DXmCSTextWidget widget;	*/
    /*   int x, y;			*/
    /*    width, height;		*/

static void DoExpose(); 
    /*   DXmCSTextWidget widget;	*/
    /*   XEvent *event;			*/

static void HandleGraphicsExposure();
    /*   Widget w;			*/
    /*   Opaque closure;		*/
    /*   XEvent *event;			*/

static void RedisplayHBar();
    /*   DXmCSTextWidget widget;	*/

static void VerticalScroll ();
    /*   DXmCSTextWidget widget;	*/

static void HorizontalScroll ();
    /*   DXmCSTextWidget widget;	*/

static int _compute_num_output_lines ();
    /*   DXmCSTextWidget widget;	*/

static void ComputeLineIndex ();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLocation  location;	*/
    /*   int *return_value;		*/

static void HandleVBar ();
    /*    Widget w;			*/
    /*    Opaque param, cback;		*/

static void HandleHBar ();
    /*   Widget w;			*/
    /*   Opaque param, cback;		*/

static void Resize ();
    /*   DXmCSTextWidget widget;	*/

/*DON'T conditionalize ChangeInputMethod by I18N_IO_SHR
 */
static void ChangeInputMethod();
    /*   DXmCSTextWidget w;		*/
    /*   int x;				*/
    /*   int y;				*/

static void _do_with_visible_line();
    /*   DXmCSTextWidget w;             */
    /*   void (*proc)();                */

static void _set_redisplay_line ();
    /*	 DXmCSTextWidget widget;	*/
    /*   TextLine line;			*/
    /*   TextSegment segment;		*/

typedef enum {UL_CREATE, UL_DUMP, UL_INVALIDATE, UL_DELETE} UpdateLineType;

static void _update_line();
    /*   DXmCSTextWidget widget;	*/
    /*   TextLine line;			*/
    /*   UpdateLineType type		*/

#ifdef ASIAN_IO_SHR

static DXmAIMWidget _create_aim();
static void get_im_name();

static Boolean font_exist ();
    /*   DXmCSTextWidget w;		*/
    /*   char *charset;			*/

#endif /* ASIAN_IO_SHR */

static void _get_font_entry ();

#ifdef I18N_IO_SHR

#ifdef VMS
void _DXmCSTextOutputCreate();
    /*   DXmCSTextWidget widget;	*/
    /*   ArgList args;			*/
    /*   Cardinal num_args;		*/
#else
void DXmCSTextOutputCreate();
    /*   DXmCSTextWidget widget;	*/
    /*   ArgList args;			*/
    /*   Cardinal num_args;		*/
#endif /* VMS */

#else

void DXmCSTextOutputCreate();
    /*   DXmCSTextWidget widget;	*/
    /*   ArgList args;			*/
    /*   Cardinal num_args;		*/

#endif /* I18N_IO_SHR */

/* Scrollbar manager stuff */

#define BarManagerInit(widget) {\
    NeedVBarUpdate(widget) = True;\
    NeedHBarUpdate(widget) = True;\
}

#define BarManagerNeedVBarUpdate(widget) \
    NeedVBarUpdate(widget) = True;

#define BarManagerNeedHBarUpdate(widget) \
    NeedHBarUpdate(widget) = True;

#define BarManagerSetBars(widget) {\
    if (NeedVBarUpdate(widget)) {\
	_set_vbar(widget);\
	NeedVBarUpdate(widget) = False;\
    }\
    if (NeedHBarUpdate(widget)) {\
	_set_hbar(widget);\
	NeedHBarUpdate(widget) = False;\
    }\
}

/* Rendition Manager */

typedef struct _RendMgrRec {
  RmHighlightMode draw_mode;
  DXmCSTextWidget widget;
} RendMgrRec;

typedef struct {
  int edit_mode;	/* The widget's editing mode */
  int length;
  TextLocationRec current_loc;
  Boolean end_of_text;
} RmNavUnitRec, *RmNavUnit;

typedef struct {
  enum {RmChar, RmSegment, RmLine, RmAll} type;
  TextLine line;
  TextSegment segment;
  int offset;
} RmNavDataRec, *RmNavData;

#ifdef _NO_PROTO
RendMgr RendMgrInit();
XmHighlightMode RendMgrGetRendition();
void RendMgrSetRendition();
XmHighlightMode _RendMgrGetRenditionChar();
RmHighlightMode _RendMgrGetRenditionSegment();
RmHighlightMode _RendMgrGetRenditionLine();
void _RendMgrUpdateSegment();
void _RendMgrUpdateLine();
void _RendMgrUpdateAll();
void _RendMgrSetRenditionChar();
void _RendMgrSetRenditionSegment();
void _RendMgrSetRenditionLine();
void _RendMgrCharSetNotify();
void _RendMgrSegmentSetNotify();
void _RendMgrLineSetNotify();
RmNavUnit RmNavUnitInit();
Boolean RmNavUnitGetNextUnit();
Boolean _RmNavUnitGetChar();
Boolean _RmNavUnitGetSegment();
Boolean _RmNavUnitGetLine();
#else
RendMgr RendMgrInit(
  DXmCSTextWidget widget
);
XmHighlightMode RendMgrGetRendition(
  RendMgr rm,
  DXmCSTextOutputLine out_line,
  DXmCSTextOutputSegment out_seg,
  int offset
);
void RendMgrSetRendition(
  RendMgr rm,
  TextLocation start_loc,
  unsigned int len,
  XmHighlightMode mode
);
XmHighlightMode _RendMgrGetRenditionChar(
  RendMgr rm,
  TextLine line,
  TextSegment segment,
  int offset
);
RmHighlightMode _RendMgrGetRenditionSegment(
  RendMgr rm,
  TextLine line,
  TextSegment segment
);
RmHighlightMode _RendMgrGetRenditionLine(
  RendMgr rm,
  TextLine line
);
void _RendMgrUpdateSegment(
  RendMgr rm,
  TextLine line,
  TextSegment segment
);
void _RendMgrUpdateLine(
  RendMgr rm,
  TextLine line
);
void _RendMgrUpdateAll(
  RendMgr rm
);
void _RendMgrSetRenditionChar(
  RendMgr rm,
  TextLine line,
  TextSegment segment,
  unsigned int offset,
  XmHighlightMode mode
);
void _RendMgrSetRenditionSegment(
  RendMgr rm,
  TextLine line,
  TextSegment segment,
  XmHighlightMode mode
);
void _RendMgrSetRenditionLine(
  RendMgr rm,
  TextLine line,
  XmHighlightMode mode
);
void _RendMgrCharSetNotify(
  RendMgr rm,
  TextLine line,
  TextSegment segment,
  unsigned int offset,
  XmHighlightMode mode
);
void _RendMgrSegmentSetNotify(
  RendMgr rm,
  TextLine line,
  TextSegment segment,
  RmHighlightMode mode
);
void _RendMgrLineSetNotify(
  RendMgr rm,
  TextLine line,
  RmHighlightMode mode
);
RmNavUnit RmNavUnitInit(
  int edit_mode,
  TextLocation start_loc,
  unsigned int len
);
Boolean RmNavUnitGetNextUnit(
  RmNavUnit nu,
  RmNavData nd
);
Boolean _RmNavUnitGetChar(
  RmNavUnit nu,
  TextLine *line,
  TextSegment *segment,
  unsigned int *offset
);
Boolean _RmNavUnitGetSegment(
  RmNavUnit nu,
  TextLine *line,
  TextSegment *segment
);
Boolean _RmNavUnitGetLine(
  RmNavUnit nu,
  TextLine *line
);
#endif /* _NO_PROTO */

/* Rendition Manager Macro defines */

#define RendMgrSetRenditionLine(rm, line, mode) {\
    _RendMgrSetRenditionLine(rm, line, mode);\
    _RendMgrUpdateAll(rm);\
}

/* Rendition Manager functions */

RendMgr RendMgrInit(widget)
DXmCSTextWidget widget;
{
    RendMgr rm = (RendMgr)XtMalloc(sizeof(RendMgrRec));
    rm->draw_mode = RmHIGHLIGHT_NORMAL;
    rm->widget = widget;

    return rm;
}


#define RendMgrFree(rm) XtFree(rm)

#define RendMgrResetRendition(rm) ((rm)->draw_mode = RmHIGHLIGHT_NORMAL)

XmHighlightMode RendMgrGetRendition(rm, out_line, out_seg, offset)
RendMgr rm;
DXmCSTextOutputLine out_line;
DXmCSTextOutputSegment out_seg;
int offset;
{
    if (rm->draw_mode != RmSEE_DETAIL)
        return (XmHighlightMode)rm->draw_mode;

    if (out_line->draw_mode != RmSEE_DETAIL)
        return (XmHighlightMode)out_line->draw_mode;

    if (out_seg->draw_mode != RmSEE_DETAIL)
        return (XmHighlightMode)out_seg->draw_mode;

    return out_seg->per_char[offset].draw_mode;
}


RmNavUnit RmNavUnitInit(edit_mode, start_loc, len)
int edit_mode;
TextLocation start_loc;
unsigned int len;
{
    RmNavUnit nu = (RmNavUnit)XtMalloc(sizeof(RmNavUnitRec));

    nu->edit_mode = edit_mode;
    nu->current_loc = *start_loc;
    nu->length = len;
    nu->end_of_text = False;

    return nu;
}


#define RmNavUnitFree(nu) XtFree(nu)

Boolean _RmNavUnitGetChar(nu, line, segment, offset)
RmNavUnit nu;
TextLine *line;
TextSegment *segment;
unsigned int *offset;
{
    TextLocation loc;

    if (nu->length <= 0 || nu->end_of_text)
	return False;

    loc = &nu->current_loc;
    while (loc->end_of_line) {
	nu->length--;
        if (!_DXmCSTextNavNextChar(loc))
	    nu->end_of_text = True;

	if (nu->length <= 0 || nu->end_of_text)
	    return False;
    }

    *line = loc->line;
    *segment = loc->segment;
    *offset = loc->offset;

    nu->length--;
    if (!_DXmCSTextNavNextChar(loc))
	nu->end_of_text = True;
}


Boolean _RmNavUnitGetSegment(nu, line, segment)
RmNavUnit nu;
TextLine *line;
TextSegment *segment;
{
    TextLocation loc;

    if (nu->length <= 0 || nu->end_of_text)
	return False;
    if (!nu->current_loc.segment)
	return False;

    loc = &nu->current_loc;
    if (loc->offset == 0 && loc->segment->length <= nu->length) {
	*line = loc->line;
	*segment = loc->segment;

	nu->length -= loc->segment->length;

	loc->position += loc->segment->length;
        loc->segment = _DXmCSTextGetNextSegment(loc->segment);
	loc->offset = 0;

	if (!loc->segment) {
	    nu->length--;

	    loc->position++;
	    loc->line = _DXmCSTextGetNextLine(loc->line);
	    loc->line_offset = 0;
	    if (loc->line) {
		loc->segment = _DXmCSTextGetFirstSegment(loc->line);
		while (loc->segment && loc->segment->length == 0)
		    loc->segment = loc->segment->next;
	    } else {
		nu->end_of_text = True;
		loc->segment = NULL;
	    }
	    loc->offset = 0;
	    loc->end_of_line = loc->segment == NULL;
	}
	return True;
    }
    return False;
}


Boolean _RmNavUnitGetLine(nu, line)
RmNavUnit nu;
TextLine *line;
{
    TextLocation loc;
    register int line_length;

    if (nu->length <= 0 || nu->end_of_text)
	return False;
    if (!nu->current_loc.line)
	return False;

    loc = &nu->current_loc;
    line_length = nu->edit_mode == XmMULTI_LINE_EDIT ?
			loc->line->length + 1 :
			loc->line->length;  /* Add 1 for return code */
    if (loc->line_offset == 0 && line_length <= nu->length) {
	*line = loc->line;

	nu->length -= line_length;

	loc->position += line_length;
	loc->line = _DXmCSTextGetNextLine(loc->line);
	loc->line_offset = 0;
	loc->offset = 0;
	if (loc->line) {
            loc->segment = _DXmCSTextGetFirstSegment(loc->line);

	    while (loc->segment && loc->segment->length == 0)
		loc->segment = loc->segment->next;
	} else {
	    loc->segment = NULL;
	    nu->end_of_text = True;
	}
	loc->end_of_line = loc->segment == NULL;

	return True;
    }
    return False;
}


Boolean RmNavUnitGetNextUnit(nu, nd)
RmNavUnit nu;
RmNavData nd;
{
    TextLine line;
    TextSegment segment;
    unsigned int offset;

    if (nu->length == 0 || nu->end_of_text)
	return False;

    if (_RmNavUnitGetLine(nu, &line)) {
        nd->type = RmLine;
	nd->line = line;
	return True;
    }

    if (_RmNavUnitGetSegment(nu, &line, &segment)) {
	nd->type = RmSegment;
	nd->line = line;
	nd->segment = segment;
	return True;
    }

    if (_RmNavUnitGetChar(nu, &line, &segment, &offset)) {
	nd->type = RmChar;
	nd->line = line;
	nd->segment = segment;
	nd->offset = offset;
	return True;
    }

    return False;
}


XmHighlightMode _RendMgrGetRenditionChar(rm, line, segment, offset)
RendMgr rm;
TextLine line;
TextSegment segment;
int offset;
{
    register int i;

    if (rm->draw_mode != RmSEE_DETAIL)
        return (XmHighlightMode)rm->draw_mode;

    if (!line->output_line)
	_validate_line(rm->widget, line, NULL);
    /* NOTE : output lines of a line always have the same highlight mode. */
    if (line->output_line[0].draw_mode != RmSEE_DETAIL)
        return line->output_line[0].draw_mode;

    if (!segment)
	return XmHIGHLIGHT_NORMAL;

    if (!segment->output_segment)
	_validate_segment(rm->widget, line, segment);
    /* NOTE : output segments of a segment always have
	the same highlight mode. */
    if (segment->output_segment[0].draw_mode != RmSEE_DETAIL)
	return segment->output_segment[0].draw_mode;

    for (i = 0; i < segment->num_output_segments; i++) {
	register DXmCSTextOutputSegment out_seg =
			&segment->output_segment[i];

	if (offset >= out_seg->offset &&
	    offset < out_seg->offset + out_seg->char_count) {
	    return out_seg->per_char[offset - out_seg->offset].draw_mode;
	}
    }
    /* Should never be reached here */
}


RmHighlightMode _RendMgrGetRenditionSegment(rm, line, segment)
RendMgr rm;
TextLine line;
TextSegment segment;
{
    if (rm->draw_mode != RmSEE_DETAIL)
        return rm->draw_mode;

    if (!line->output_line)
	_validate_line(rm->widget, line, NULL);
    /* NOTE : output lines of a line always have the same highlight mode. */
    if (line->output_line[0].draw_mode != RmSEE_DETAIL)
        return line->output_line[0].draw_mode;

    if (!segment)
	return XmHIGHLIGHT_NORMAL;

    if (!segment->output_segment)
	_validate_segment(rm->widget, line, segment);
    /* NOTE : output segments of a segment always have
	the same highlight mode. */
    return segment->output_segment[0].draw_mode;
}


RmHighlightMode _RendMgrGetRenditionLine(rm, line)
RendMgr rm;
TextLine line;
{
    if (rm->draw_mode != RmSEE_DETAIL)
        return rm->draw_mode;

    if (!line->output_line)
	_validate_line(rm->widget, line, NULL);
    /* NOTE : output lines of a line always have the same highlight mode. */
    return line->output_line[0].draw_mode;
}


void _RendMgrUpdateSegment(rm, line, segment)
RendMgr rm;
TextLine line;
TextSegment segment;
{
    RmHighlightMode mode;
    DXmCSTextOutputSegment out_seg;
    register int i, j;
    Boolean same_mode;
    Boolean done;

    if (!segment)
	return;

    if (!segment->output_segment)
	_validate_segment(rm->widget, line, segment);

    /* NOTE : output semgents in a segment should always have the same
		highlight mode.   */
    mode = segment->output_segment[0].per_char[0].draw_mode;
    same_mode = True;
    done = False;
    for (i = 0; i < segment->num_output_segments && ! done; i++) {
        out_seg = &segment->output_segment[i];

	for (j = 0; j < out_seg->char_count && ! done; j++) {
	    if (mode != out_seg->per_char[j].draw_mode) {
		same_mode = False;
		done = True;
	    }
	}
    }
    if (!same_mode)
	mode = RmSEE_DETAIL;
    for (i = 0; i < segment->num_output_segments; i++) {
	segment->output_segment[i].draw_mode = mode;
    }
}


void _RendMgrUpdateLine(rm, line)
RendMgr rm;
TextLine line;
{
    TextSegment segment;
    register int i, j;
    RmHighlightMode mode;

    segment = _DXmCSTextGetFirstSegment(line);
    if (!segment)
	return;

    if (!segment->output_segment)
	_validate_segment(rm->widget, line, segment);

    mode = segment->output_segment[0].draw_mode;
    if (mode == RmSEE_DETAIL) {
	if (!line->output_line)
	    _validate_line(rm->widget, line, NULL);
	for (i = 0; i < line->num_output_lines; i++) {
	    line->output_line[i].draw_mode = RmSEE_DETAIL;
	}
	return;
    }

    for (;
	 segment;
	 segment = _DXmCSTextGetNextSegment(segment)) {
	if (!segment->output_segment)
	    _validate_segment(rm->widget, line, segment);
	for (i = 0; i < segment->num_output_segments; i++) {
	    if (mode != segment->output_segment[i].draw_mode) {
		if (!line->output_line)
		    _validate_line(rm->widget, line, NULL);
		for (j = 0; j < line->num_output_lines; j++) {
		    line->output_line[j].draw_mode = RmSEE_DETAIL;
		}
		return;
	    }
	}
    }
    if (!line->output_line)
	_validate_line(rm->widget, line, NULL);
    for (i = 0; i < line->num_output_lines; i++) {
	line->output_line[i].draw_mode = mode;
    }
}


void _RendMgrUpdateAll(rm)
RendMgr rm;
{
    TextLine line;
    RmHighlightMode mode;
    register int i;

    line = _DXmCSTextGetFirstLine(rm->widget);
    if (!line)
	return;

    if (!line->output_line)
	_validate_line(rm->widget, line, NULL);
   
    mode = line->output_line[0].draw_mode;
    if (mode == RmSEE_DETAIL) {
	rm->draw_mode = RmSEE_DETAIL;
	return;
    }
    for (i = 1; i < line->num_output_lines; i++) {
	if (mode != line->output_line[i].draw_mode) {
	    rm->draw_mode = RmSEE_DETAIL;
	    return;
	}
    }
    line = _DXmCSTextGetNextLine(line);
    for (;
	 line;
	 line = _DXmCSTextGetNextLine(line)) {
	if (!line->output_line)
	    _validate_line(rm->widget, line, NULL);
	for (i = 0; i < line->num_output_lines; i++) {
	    if (mode != line->output_line[i].draw_mode) {
		rm->draw_mode = RmSEE_DETAIL;
		return;
	    }
	}
    }
    rm->draw_mode = mode;
    return;
}


void RendMgrSetRendition(rm, start_loc, len, mode)
RendMgr rm;
TextLocation start_loc;
unsigned int len;
XmHighlightMode mode;
{
    RmNavUnit nu;
    RmNavDataRec nd;
    TextLine cur_line = NULL;
    TextSegment cur_segment = NULL;

    if (!start_loc || len == 0)
        return;

    if (start_loc->position == 0 && len >= CSTextLength(rm->widget)) {
	rm->draw_mode = (RmHighlightMode)mode;
	RedisplayAll(rm->widget) = True;
	return;
    }

    nu = RmNavUnitInit(CSTextEditMode(rm->widget), start_loc, len);

    while (RmNavUnitGetNextUnit(nu, &nd)) {
	switch (nd.type) {
	  case RmChar :
	    if (_RendMgrGetRenditionChar(rm, nd.line,
			nd.segment, nd.offset) == mode)
		break;

	    if (cur_segment) {
		if (cur_segment != nd.segment) {
		    _RendMgrUpdateSegment(rm, cur_line, cur_segment);
		    cur_segment = nd.segment;
		}
	    } else {
		cur_segment = nd.segment;
	    }
	    if (cur_line) {
		if (cur_line != nd.line) {
		    _RendMgrUpdateLine(rm, cur_line);
		    cur_line = nd.line;
		}
	    } else {
		cur_line = nd.line;
	    }
	    _RendMgrSetRenditionChar(rm, nd.line, nd.segment, nd.offset, 
					mode);
	    break;
	  case RmSegment :
	    if (_RendMgrGetRenditionSegment(rm, nd.line, nd.segment) == 
		(RmHighlightMode)mode)
		break;

	    if (cur_segment) {
		_RendMgrUpdateSegment(rm, cur_line, cur_segment);
		cur_segment = NULL;
	    }
	    if (cur_line) {
		if (cur_line != nd.line) {
		    _RendMgrUpdateLine(rm, cur_line);
		    cur_line = nd.line;
		}
	    } else {
		cur_line = nd.line;
	    }
	    _RendMgrSetRenditionSegment(rm, nd.line, nd.segment, mode);
	    break;
	  case RmLine :
	    if (_RendMgrGetRenditionLine(rm, nd.line) == 
		(RmHighlightMode)mode)
		    break;

	    if (cur_segment) {
		_RendMgrUpdateSegment(rm, cur_line, cur_segment);
		cur_segment = NULL;
	    }
	    if (cur_line) {
		_RendMgrUpdateLine(rm, cur_line);
		cur_line = NULL;
	    }
	    _RendMgrSetRenditionLine(rm, nd.line, mode);
	    break;
	  default :
	    break;
	}
    }
    if (cur_segment)
	_RendMgrUpdateSegment(rm, cur_line, cur_segment);
    if (cur_line)
	_RendMgrUpdateLine(rm, cur_line);
    _RendMgrUpdateAll(rm);

    RmNavUnitFree((char *)nu);
}


void _RendMgrSetRenditionChar(rm, line, segment, offset, mode)
RendMgr rm;
TextLine line;
TextSegment segment;
unsigned int offset;
XmHighlightMode mode;
{
    register int i, k;

    if (!segment)   /*  It may be an end of line */
	return;

    if (!segment->output_segment)
        _validate_segment(rm->widget, line, segment);

    for (i = 0; i < segment->num_output_segments; i++) {
        register DXmCSTextOutputSegment out_seg
			= &segment->output_segment[i];

        if (offset >= out_seg->offset &&
            offset <  out_seg->offset + out_seg->char_count) {
            register int j = offset - out_seg->offset;

	    out_seg->per_char[j].draw_mode = mode;

	    /* set redisplays */
	    out_seg->per_char[j].need_redisplay = TRUE;
	    out_seg->need_redisplay = TRUE;
	    if (!line->output_line)
		_validate_line(rm->widget, line, NULL);
	    for (k = 0; k < line->num_output_lines; k++) {
		register DXmCSTextOutputLine out_line =
			    &line->output_line[k];
		if (out_line->y != out_seg->y)
		    continue;

		out_line->need_redisplay = TRUE;
		break;
	    }
            break;
        }
    }
    _RendMgrCharSetNotify(rm, line, segment, offset, mode);
}


void _RendMgrSetRenditionSegment(rm, line, segment, mode)
RendMgr rm;
TextLine line;
TextSegment segment;
XmHighlightMode mode;
{
    register int i, j;

    if (!segment)
	return;

    if (!segment->output_segment)
        _validate_segment(rm->widget, line, segment);

    for (i = 0; i < segment->num_output_segments; i++) {
        register DXmCSTextOutputSegment out_seg
			= &segment->output_segment[i];

        out_seg->draw_mode = (RmHighlightMode)mode;

	/* set redisplays */
	for (j = 0; j < out_seg->char_count; j++)
	    out_seg->per_char[j].need_redisplay = TRUE;
        out_seg->need_redisplay = TRUE;
	for (j = 0; j < line->num_output_lines; j++) {
	    register DXmCSTextOutputLine out_line =
	   		&line->output_line[j];
	    if (out_line->y != out_seg->y)
	        continue;

	    out_line->need_redisplay = TRUE;
	    break;
	}
    }        
    _RendMgrSegmentSetNotify(rm, line, segment, (RmHighlightMode)mode);
}


void _RendMgrSetRenditionLine(rm, line, mode)
RendMgr rm;
TextLine line;
XmHighlightMode mode;
{
    register int i, j;
    register TextSegment segment;

    if (!line->output_line)
	_validate_line(rm->widget, line, NULL);

    for (i = 0; i < line->num_output_lines; i++) {
	register DXmCSTextOutputLine out_line =
			&line->output_line[i];

	out_line->draw_mode = (RmHighlightMode)mode;

	/* set redisplays */
	out_line->need_redisplay = TRUE;
    }

    /* set redisplays for all chars/segments in the line */
    for (segment = _DXmCSTextGetFirstSegment(line);
	 segment;
	 segment = _DXmCSTextGetNextSegment(segment)) {
	if (!segment->output_segment)
	    _validate_segment(rm->widget, line, segment);
	for (i = 0; i < segment->num_output_segments; i++) {
	    register DXmCSTextOutputSegment out_seg =
			&segment->output_segment[i];
	    out_seg->need_redisplay = TRUE;
	    for (j = 0; j < out_seg->char_count; j++)
		out_seg->per_char[j].need_redisplay = TRUE;
	}
    }
    _RendMgrLineSetNotify(rm, line, (RmHighlightMode)mode);
}


void _RendMgrCharSetNotify(rm, line, segment, offset, mode)
RendMgr rm;
TextLine line;
TextSegment segment;
unsigned int offset;
XmHighlightMode mode;
{
    register int i, j;
    RmHighlightMode segment_mode =
	_RendMgrGetRenditionSegment(rm, line, segment);

    if (!segment->output_segment)
	_validate_segment(rm->widget, line, segment);

    if (segment_mode == RmSEE_DETAIL) {
	/* In case mode != RmSEE_DETAIL, the whole segment might have the
	   same rendition so that we have to check it.  But it cost so
	   much.  The check will be done later.
	 */
	return;
    } else {
        if (segment_mode == (RmHighlightMode)mode)
	    return;
	else {
	    for (i = 0; i < segment->num_output_segments; i++) {
		register DXmCSTextOutputSegment out_seg =
				&segment->output_segment[i];

		for (j = 0; j < out_seg->char_count; j++) {
		    if (offset == out_seg->offset + j)
			continue;

		    out_seg->per_char[j].draw_mode =
			(XmHighlightMode)segment_mode;
		}
		out_seg->draw_mode = RmSEE_DETAIL;
    	    }
	    _RendMgrSegmentSetNotify(rm, line, segment, RmSEE_DETAIL);
	}
    }
}


void _RendMgrSegmentSetNotify(rm, line, segment, mode)
RendMgr rm;
TextLine line;
TextSegment segment;
RmHighlightMode mode;
{
    register int i;
    register TextSegment cur_segment;
    RmHighlightMode line_mode =
	_RendMgrGetRenditionLine(rm, line);

    if (!line->output_line)
	_validate_line(rm->widget, line, NULL);

    if (line_mode == RmSEE_DETAIL) {
	/* In case mode != RmSEE_DETAIL, the whole line might have the
	   same rendition so that we have to check it.  But it cost so
	   much.  The check will be done later.
	 */
	return;
    } else {
        if (line_mode == mode)
            return;
        else {
	    for (cur_segment = _DXmCSTextGetFirstSegment(line);
		 cur_segment;
		 cur_segment = _DXmCSTextGetNextSegment(cur_segment)) {
		if (cur_segment == segment)
		    continue;

		if (!cur_segment->output_segment)
		    _validate_segment(rm->widget, line, cur_segment);

		for (i = 0; i < cur_segment->num_output_segments; i++) {
		    register DXmCSTextOutputSegment out_seg =
				&cur_segment->output_segment[i];
		    out_seg->draw_mode = line_mode;
		}
	    }
	    for (i = 0; i < line->num_output_lines; i++)
        	line->output_line[i].draw_mode = RmSEE_DETAIL;
	    _RendMgrLineSetNotify(rm, line, RmSEE_DETAIL);
	}
    }
}


void _RendMgrLineSetNotify(rm, line, mode)
RendMgr rm;
TextLine line;
RmHighlightMode mode;
{
    register int i;
    register TextLine cur_line;

    if (rm->draw_mode == RmSEE_DETAIL) {
	/* In case mode != RmSEE_DETAIL, the whole text might have the
	   same rendition so that we have to check it.  But it cost so
	   much.  The check will be done later.
	 */
        return;
    } else {
        if (rm->draw_mode == mode)
            return;
        else {
            for (cur_line = _DXmCSTextGetFirstLine(rm->widget);
		 cur_line;
		 cur_line = _DXmCSTextGetNextLine(cur_line)) {
		if (cur_line == line)
		    continue;

		if (!cur_line->output_line)
		    _validate_line(rm->widget, cur_line, NULL);

		for (i = 0; i < cur_line->num_output_lines; i++) {
		    register DXmCSTextOutputLine out_line =
				&cur_line->output_line[i];
		    out_line->draw_mode = rm->draw_mode;
		}
	    }
            rm->draw_mode = RmSEE_DETAIL;
	}
    }
}


void
_get_xlib (widget, line, segment, out_seg)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
    DXmCSTextOutputSegment out_seg;        
{
  int start_offset, end_offset;

  /* create xlib ONLY when char_count is > 0
   */
  if ( out_seg->char_count > 0 )
  {
    I18nCreateXlibBuffers( (I18nContext) NULL,			/* I18nContext 	*/
			   (I18nFontList *) NULL, 	/* I18nFontList */
			   (char *) segment->charset,	/* charset 	*/
			   (short ) 0,			/* direction 	*/
			   (char *) segment->text,	/* text		*/
			   (short) out_seg->char_count,	/* char count	*/
			   (int	) out_seg->offset,	/* first char pos */
			   (XmVoidProc) NULL,		/* need_more_callback*/
			   (Opaque) NULL,		/* need_more_context*/
			   (XmVoidProc) NULL,		/* no_font_callback*/
			   (Opaque) NULL,		/* no_font_context*/
			   (I18nXlibBuffers*) &(out_seg->xlib_buff),/* xlib buffers */
			   (short *) &(out_seg->xlib_buff_count) );/* xlib buff # */

    /*BOGUS don't know how to use the more than one xlib_buff
     */
  }
}



static void 
_XmTextDrawShadow (w)
   DXmCSTextWidget w;
{
  int a;

  if (XtIsRealized (w)) 
    {
      if (w->primitive.shadow_thickness > 0)
	{
	  a = 2 * w->primitive.highlight_thickness;

	  _XmDrawShadows (XtDisplay (w), 
			 XtWindow (w),
			 w->primitive.top_shadow_GC,
			 w->primitive.bottom_shadow_GC,
			 w->primitive.highlight_thickness,
			 w->primitive.highlight_thickness,
			 w->core.width  - a,
			 w->core.height - a,
			 w->primitive.shadow_thickness,
			 XmSHADOW_IN);
       }

      if (w->primitive.highlighted)
	{
	  _XmHighlightBorder (w);
	}
      else 
	{
	  if (_XmDifferentBackground ((Widget) w, XtParent (w)))
	    _XmUnhighlightBorder (w);
	}
   }
}





static Boolean
_visible_line (widget, out_line)
    DXmCSTextWidget widget;
    DXmCSTextOutputLine out_line;
{
  int z;

  z = VerticalOffset (widget) + out_line->y;

  return ((z >= TopEdge (widget)) && 
	  ((z + ((int) out_line->pixel_height)) <= BottomEdge (widget)));
}


static void
_clear_to_edges (widget, le, re, out_line, min_x, max_x)
    DXmCSTextWidget widget;
    DXmCSTextOutputLine out_line;
    int min_x, max_x;
    int re, le;
{
    int dx, y;

    y  = VerticalOffset (widget) + out_line->y;

    if (le > re)
      {
	/*
	 * have an empty line and le/re are still inverted from min/max'ing
	 */
	XClearArea (XtDisplay (widget),
		    InnerWindow (widget),
		    re,                         /* really left edge */
		    y,
		    le - re,                    /* to right edge */
		    out_line->pixel_height,
		    FALSE);
      }
    else
      {
	dx = min_x - le;
	
	if (dx > 0)
	  XClearArea (XtDisplay (widget), 
		      InnerWindow (widget),
		      le,
		      y, 
		      dx,
		      out_line->pixel_height,
		      FALSE);
	
	dx = re - max_x;
	
	if (dx > 0)
	  XClearArea (XtDisplay (widget), 
		      InnerWindow (widget),
		      max_x, 
		      y,
		      dx,
		      out_line->pixel_height,
		      FALSE);
      }
}



/*
 * a basic output module method.  Draw the stuff in the source.
 *
 * wander across the source looking for something to do, if anything
 * is found then push it out to the display
 */
static void 
_draw (widget, complete_cursors)
    DXmCSTextWidget widget;
    Boolean complete_cursors;
{
  DXmCSTextOutputData data = OutputData (widget);
  TextLine line;
  TextSegment segment;
  DXmCSTextOutputLine out_line;
  DXmCSTextOutputSegment out_seg;
  int i, min_x, max_x, max_y = VerticalOffset (widget);
  int re, le, seg_le, seg_re, l, m;
  XRectangle rect;
  Boolean enter_visible_area = False;
  Boolean done = False;

  le = LeftEdge (widget);
  re = RightEdge (widget);
  
  if ( ! XtIsRealized ((Widget) widget)) return;

  min_x = re;
  max_x = le;

  rect.x      = LeftEdge  (widget);
  rect.y      = TopEdge   (widget);
  rect.width  = NetWidth  (widget);
  rect.height = NetHeight (widget);

  XSetClipRectangles (XtDisplay (widget), 
		      InverseGC (widget),
		      0, 0,                          /* clip origin */
		      &rect,
		      1,
		      Unsorted);

  XSetClipRectangles (XtDisplay (widget), 
		      NormalGC (widget),
		      0, 0,                          /* clip origin */
		      &rect,
		      1,
		      Unsorted);
  
  for (line = _DXmCSTextGetFirstLine (widget);
       line != NULL && ! done;
       line = _DXmCSTextGetNextLine (line))
  {

      _validate_line(widget, line, NULL);

      for (l = 0; l < line->num_output_lines; l++)
      {
	  out_line = &(line->output_line[l]);

	  if ((_visible_line (widget, out_line)))
	  {
	      enter_visible_area = True;

	      if (out_line->need_redisplay || RedisplayAll(widget))
	      {
		  out_line->need_redisplay = FALSE;

		  min_x = re;
		  max_x = le;

		  if (out_line->need_recompute)
		    _compute_line (widget, line, segment);
	  
		  for (segment = _DXmCSTextGetFirstSegment (line);
		       segment != NULL;
		       segment = _DXmCSTextGetNextSegment (segment))
		  {

		      if (segment->num_output_segments == 0)
			_validate_segment (widget, line, segment);

		      for (i = 0; i < segment->num_output_segments; i++)
		      {
			  out_seg = &(segment->output_segment[i]);

			  /* Bug fix - we don't need temp_out_line */
			  if (out_seg->y != out_line->y)
			    continue;

			  /* have to reset max_x for every non-empty out_seg
			   */
			  if (out_seg->char_count > 0)
			    /* The other way around for RTOL */
			    if (TextRtoL(widget))
			      min_x = re ;
			    else
			      max_x = le;

			  /* find the correct out_line
			   */
			  if (out_seg->need_redisplay || RedisplayAll(widget))
			  {
			      if (out_seg->need_recompute)
				_compute_segment (widget, line, segment);

			      _draw_segment (widget, 
					     line, 
					     segment,
					     out_line,
					     out_seg,
					     complete_cursors,
					     &min_x, &max_x);
			  }
			  else
			  {

			      seg_le = SegToWindowX (widget,
						     out_line,
						     out_seg);
			      seg_re = seg_le + out_seg->pixel_width;

			      min_x = MIN (min_x, seg_le);
			      max_x = MAX (max_x, seg_re);
			  }

			  if ( out_seg->need_redisplay  || RedisplayAll(widget))
			    out_seg->need_redisplay = False;
		      }
		  }
		  _clear_to_edges (widget, le, re, out_line,
				   min_x, max_x);
	      }
	  }
	  else
	  {
	      if (enter_visible_area)
	      {
		  done = True;
		  break;
	      }
	  }

	  /* we still need to clear once. case when last char
	   * is deleted from the line
	   */
	  if ( _DXmCSTextGetFirstSegment(line) == (TextSegment) NULL )
	    _clear_to_edges (widget, le, re, out_line, min_x, max_x);

	  max_y += out_line->pixel_height;

      }
  }

  RedisplayAll(widget) = False;

  if (max_y < BottomEdge (widget))
    XClearArea (XtDisplay (widget), 
		InnerWindow (widget),
		LeftEdge (widget), 
		max_y, 
		NetWidth (widget), 
		BottomEdge (widget) - max_y, 
		FALSE);

  _XmTextDrawShadow (widget);
}

static DXmDrawProc 
#ifdef _NO_PROTO
Draw (widget)
    DXmCSTextWidget widget;
#else
Draw (
    DXmCSTextWidget widget)
#endif
{
  CSTextCursor cursor = CursorList (widget);

  _undraw_cursor (widget);
  _draw (widget, FALSE);

  /******************************************************************************/
  /*										*/
  /*   This code fixes a bug where the Ibar cursor was not drawn in the CStext  */
  /* widget if the blinkrate == 0.  The reason it was not being drawn was	*/
  /* because this routine was invoked as one of the last things that were done  */
  /* to reexpose the widget before returning to the application during a focus  */
  /* event.  Therefore the _undraw_cursor function, would erase the cursor, and */
  /* not redraw it.  The only reason it would show up with the blinkrate > 0,   */
  /* was that the timer for the flashing cursor would be invoked and draw the   */
  /* cursor again.							        */
  /*   We have to make sure that we check the status of the Cursor structure    */
  /* to see if the actual Ibar cursor pixmap was created before trying to redraw*/
  /* it, because this routine is called many times before the cursor is valid.  */
  /*										*/
  /******************************************************************************/
  if ((cursor != (CSTextCursor)NULL) && (cursor->active))
     _draw_cursor (widget); 
  BarManagerSetBars(widget);
}




/*
 * hunt for a char that needs redisplay
 */
static int
_find_start (widget, out_line, out_seg, start)
DXmCSTextWidget widget;
DXmCSTextOutputLine out_line;
DXmCSTextOutputSegment out_seg;
int start;
{
/*
  int max = out_seg->char_count;
  DXmCSTextOutputChar buf = out_seg->per_char;

  for ( ; start < max; start++)
    if (buf[start].need_redisplay)
      return (start);

  return (max);
 */
  return start;  
}


/*
 * find the end of the run of chars that need redisplay and all
 * have the same draw mode
 */
static int
_find_end (widget, out_line, out_seg, start)
DXmCSTextWidget widget;
DXmCSTextOutputLine out_line;
DXmCSTextOutputSegment out_seg;
int start;
{
  char *b = out_seg->xlib_buff->text;
  DXmCSTextOutputChar buf = out_seg->per_char;
  int max = out_seg->char_count;
  XmHighlightMode draw_type = RendMgrGetRendition(RenditionMgr(widget),
						  out_line,
						  out_seg,
						  start);
  Boolean need_redisplay = buf[start].need_redisplay;
/*
  for ( ; start < max; start++)
    if (( ! buf[start].need_redisplay) || 
	(buf[start].draw_mode != draw_type))
      return (start);
  */

  for ( ; start < max; start++)
  {
    if (RendMgrGetRendition(RenditionMgr(widget), out_line, out_seg, start)
	  != draw_type || buf[start].need_redisplay != need_redisplay)
      return (start);

    if (b[start] == '\t')
      return (start + 1);
  }

  return (max);
}


static Dimension
_find_width (tab_width, entry, buf, count, accu_width)
    Dimension   tab_width;
    XmFontListEntry entry;
    char *buf;
    int count;
    Dimension	accu_width;
{
  int i;
  Dimension width = 0;
  int t_width;

  for ( i=0; i<count; i++)
  {
     if ( buf[i] == '\t' )
     {
	/* width for tab_width should be less 1/8 of the tab_width
	 * i.e. the tab char itself
	 */
	t_width = (int) accu_width + (int) width;

	width += tab_width - (Dimension) ( t_width % (int) tab_width );

     } else {
          if (FontListType(entry) == XmFONT_IS_FONT)
          {
             XFontStruct *font_struct = (XFontStruct *)FontListFont(entry);
           
	     if (IsTwoByteFont (font_struct))
		width += XTextWidth16 (font_struct, (XChar2b*)&buf[i*2], 1);
	     else
		width += XTextWidth (font_struct, &buf[i], 1);
          } else /* if XmFONT_IS_FONTSET */
	     width += XmbTextEscapement ((XFontSet)FontListFont(entry), 
			&buf[i], 1);
     }               
  }

  return (width);
}




static Chunk
/* Performance /Mak
_init_chunk (segment, out_seg, font, start, end, b, chunk, q, x, y, accu_width)
 */
_init_chunk (widget, out_line, out_seg, start, end, chunk, q, x, y,
	     accu_width, redisplay_all)
  DXmCSTextWidget widget;
  DXmCSTextOutputLine out_line;
  DXmCSTextOutputSegment out_seg;
  int start, end;
  Chunk *chunk, *q;
  int x, y;
  int accu_width;
  Boolean redisplay_all;
{
  char *tmp, *b = out_seg->xlib_buff->text;
  int i;
  Chunk p = (Chunk) XtMalloc (sizeof (ChunkRec));
  int start_byte = 0;
  
  if (*q == NULL)
    *chunk = p;
  else
    (*q)->next = p;
  
  (*q) = p;
  
  p->next        = NULL;
  p->draw_mode   = RendMgrGetRendition(RenditionMgr(widget),
				       out_line, out_seg,
				       start);
  p->char_count  = end - start;
  p->skip        = TRUE;

  for (i=start; i < end; i++)
    {
/* Performance /Mak
      if (out_seg->per_char[i].need_redisplay)
 */
      if (redisplay_all || out_seg->per_char[i].need_redisplay)
	p->skip = FALSE;
      out_seg->per_char[i].need_redisplay = False;
    }

  p->x      = x;
  p->y      = y;
  p->height = out_seg->pixel_height;
  p->bottom = p->y + p->height;

  if (FontListType(*out_seg->font_entry) == XmFONT_IS_FONT) {
     XFontStruct *font = (XFontStruct *)FontListFont(*out_seg->font_entry);
  
     if (IsTwoByteFont (font))
        start_byte = start * 2;
     else
       start_byte = start;
  } else  /* XmFONT_IS_FONTSET */
     /* This is really ugly! Because FontSets can contain single and two
	byte fonts, we cannot assume that all of the characters are the
	same size for a particular segment like we used to. So, in order to
	find "start_byte", we have to loop through all of the characters up to
	"start" and add up their sizes to get the offset of the "start"th byte
	representing the "start"th character.
     */
     for (i=0, tmp=b, start_byte=0;
          i < start;
          i++, start_byte+=mblen(tmp,MB_CUR_MAX), tmp+=start_byte);
  
  p->text   = &(b[start_byte]);
  p->width  = _find_width (out_seg->tab_width, *out_seg->font_entry,
  	      p->text, p->char_count, accu_width);
  p->right  = p->x + p->width;

  return (p);
}









/*
 * break the output segment down into runs of chars (a chunk) 
 * which need displaying and have the same draw_mode.  build linked
 * list of chunks to display
 */

static Chunk 
_get_chunks (widget, line, out_line, segment, out_seg, accu_width)
    DXmCSTextWidget widget;
    TextLine line;
    DXmCSTextOutputLine out_line;
    TextSegment segment;
    DXmCSTextOutputSegment out_seg;
    int accu_width;
{
  Chunk chunk = NULL, p = NULL, q;
  int start = 0;
  int end = 0; 
  int x = SegToWindowX (widget, out_line, out_seg);
  int y = VerticalOffset(widget) + out_line->y;
  int le = LeftEdge (widget);
  int re = RightEdge (widget);

  q = NULL;
    
  do
    {
      start = _find_start (widget, out_line, out_seg, end);

      if (start < out_seg->char_count)
	{
	  end = _find_end (widget, out_line, out_seg, start);

/* Performance /Mak
	  p = _init_chunk (segment, out_seg, font, start, end, 
			   b, &chunk, &q, x, y, accu_width);
 */
	  p = _init_chunk (widget, out_line, out_seg, start, end, 
			   &chunk, &q, x, y, accu_width,
			   RedisplayAll(widget));

	  accu_width += p->width;

          if (!TextRtoL ( widget ))     /* RtoL handled later */

	  if ((p->x >= re) || (p->right <= le))
	    {
	      p->skip = TRUE;
	    }

	  x += p->width;
	}

    } while (start < out_seg->char_count);

    /* 
       RtoL: The x coord. was calculated based on LtoR, adjust it 
       for RtoL. x marks the spot to begin calculations (right end of 
       string)
       Text is flipped for visual right before draw
    */
    if (TextRtoL ( widget ))
       for (end = x,    p = chunk;
                        p != NULL;
            end = p->x, p = p->next )   
       {
            p->x = end - p->width;
            p->right  = end ;
  
  	    if ((p->x >= re) || (p->right <= le))
  	    {
  	      p->skip = TRUE;
  	    }
 
       }

  return (chunk);
}


static void
_draw_segment (widget, line, segment, out_line, 
	       out_seg, complete_cursors, min_x, max_x)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
    DXmCSTextOutputSegment out_seg;
    DXmCSTextOutputLine out_line;
    Boolean complete_cursors;
    int *min_x, *max_x;
{
    DXmCSTextOutputData data = OutputData (widget);
    GC gc, invgc;
    Chunk chunk, p, q;
    int i, j, k, baseline;
    int accu_width;
    int first_char_x ;     /* 	x coord for first character in segment
				(adjusted for tabs) 
			   */

       if (TextRtoL (widget))
          accu_width = RightMargin(widget) - 
                       out_seg->x - out_seg->pixel_width ;
       else

          accu_width = out_seg->x;

       /* BOGUS  _get_chunks() should loop thru all xlib_buff
       */

       if ( out_seg->xlib_buff )
       {
       chunk = _get_chunks ( widget,
			     line, 
                             out_line,
			     segment,
			     out_seg,
			     accu_width );
	} else {
		chunk = NULL;
	}
   
       for (p = chunk;
   	    p != NULL;
   	    p = p->next)
         {
   	   if ( ! p->skip)
   	     {
	       /* BOGUS
		* Ok, so where do I draw the underline is secondary
		* select and I have multiple fonts?
		*/
   	       baseline = p->y + MaxAscent (widget);   /* Fixed by JRD */
   	    
   	       gc = (p->draw_mode == XmHIGHLIGHT_SELECTED) ? 
   	         InverseGC (widget) : NormalGC (widget);
   	    
   	       invgc = (p->draw_mode == XmHIGHLIGHT_SELECTED) ? 
   	         NormalGC (widget) : InverseGC (widget);
   	    
               if (FontListType(*out_seg->font_entry) == XmFONT_IS_FONT) {
                  XFontStruct *font = (XFontStruct *)
				      FontListFont(*out_seg->font_entry);
 
  	          XSetFont (XtDisplay (widget), gc,    font->fid);
   	          XSetFont (XtDisplay (widget), invgc, font->fid);
               };
   	    
   	       XFillRectangle (XtDisplay (widget), InnerWindow (widget), 
   			       invgc,
   			       p->x, 
   			       p->y,
   			       p->width, 
   			       p->height);
   	    
   	       if (complete_cursors)
   	         {
   		   _complete_cursors (widget, p->x, p->y, p->width, p->height);
   	         }

               first_char_x = p->x ;      /* default (LtoR) */

	       /* don't display the tab !!
		*/
	       if ( p->text[p->char_count - 1] == '\t' )
	       {
		    p->char_count -= 1;

                    /* p->width is the display length.
                     * LtoR: Text displayed left-justified within p->width.
                     * RtoL: Text displayed right-justified within p->width.
                     *       first_char_x must be moved over to right.
                     *       re-calculate width without tab and add the
                     *       difference to x
                    */
                    if (TextRtoL (widget))
                       first_char_x += p->width - 
 				      _find_width (out_seg->tab_width, 
      		  				   *out_seg->font_entry,
						   p->text, 
						   p->char_count, 
						   0); /* (no tabs) */
	       }

               if (TextRtoL (widget))
               {
                    /* Flip the text - Since the text is stored in 
                     * visual order, we have to reverse every chunk
                     * before displaying.
                    */
                    p->text = _flip_string (p->text, p->char_count, FALSE);
               }
               if (FontListType(*out_seg->font_entry) == XmFONT_IS_FONT) {
                  XFontStruct *font = (XFontStruct *)
                                      FontListFont(*out_seg->font_entry);
 
	          if (IsTwoByteFont (font))
		     XDrawImageString16 (XtDisplay (widget),
					InnerWindow (widget), 
					gc,
					p->x, 
					baseline,
					(XChar2b*)p->text, 
					p->char_count);
	       else
		   XDrawImageString (XtDisplay (widget),
				     InnerWindow (widget), 
				     gc,
                                     first_char_x,
				     baseline,
				     p->text, 
				     p->char_count);
   	       } else { /* XmFONT_IS_FONTSET */
		     XmbDrawImageString (XtDisplay (widget),
					InnerWindow (widget), 
                                        (XFontSet)
					 FontListFont(*out_seg->font_entry),
					gc,
					p->x, 
					baseline,
					p->text, 
					p->char_count);
               }

   	       if (p->draw_mode == XmHIGHLIGHT_SECONDARY_SELECTED)
   	         {
   		   XDrawLine (XtDisplay (widget), InnerWindow (widget),
   			      gc, 
   			      p->x, 
   			      baseline,
   			      p->right, 
   			      baseline);
   	         }
   	     }
   
   	     *min_x = MIN (*min_x, p->x);
   	     *max_x = MAX (*max_x, p->right);
           }
   
       /*
        * clean up chunk list and XLib char buffer
        */
       for (p = chunk; p != NULL; p = q)
         {
	   q = p->next;
           /* free the flipped text 
           */
           if (!p->skip && TextRtoL (widget))
               XtFree (p->text);

	   XtFree ((char *)p);
         }

}

static void
_set_dirty_line (widget, location)
    DXmCSTextWidget widget;
    TextLocation    location;
{
  DXmCSTextOutputLine    out_line;
  DXmCSTextOutputSegment out_seg;
  TextLine line;
  TextSegment segment, next_segment;
  int i, j;

  if ( location )
  {
    line = location->line;
    segment = location->segment;

    /* now, set all out_segs for the segment to need_redisplay.  For those
     * out_line which has the same y of the out_seg, set need_redisplay also.
     * First, start with the location->segment, then depending on the out_line,
     * set the next segments if necessary
     */
    while ( segment )
    {
	/* save up the segment for comparison
	 */
	next_segment = segment;

	/* for this segment, see which out_seg need to redisplay
	 */
	for ( i = 0; i < segment->num_output_segments; i++ )
	{
	    out_seg = &(segment->output_segment[i]);

	    if ( out_seg == (DXmCSTextOutputSegment) NULL )
		_validate_segment (widget, line, segment);

	    out_seg->need_redisplay = True;

	    /* now find which out_seg need to redisplay
	     */
	    for ( j = 0; j < line->num_output_lines; j++ )
	    {
		out_line = &(line->output_line[j]);

		if ( out_line == (DXmCSTextOutputLine) NULL )
		    _validate_line (widget, line, NULL);

		if ( out_line->y < out_seg->y )
		    continue;

		/* out_line is found
		 */
		if ( out_line->y == out_seg->y )
		{
	            out_line->need_redisplay = True;
		    next_segment = _DXmCSTextGetNextSegment(segment);

		    /* with this out_line, see if next segment is also
		     * having the same y, if so, that segment needed to
		     * redisplay as well
		     */
		    if ( next_segment != (TextSegment) NULL )
		    {
			 if ( next_segment->output_segment == 
			      (DXmCSTextOutputSegment) NULL )
			     _validate_segment (widget, line, next_segment);

			 /* next segment need not redisplay, set next_segment
			  * to NULL to terminate the outter loop
			  */
			 if ( out_line->y !=
			      next_segment->output_segment->y )
			 {
			      next_segment = (TextSegment) NULL;
			 }
		    }
		}

		break;
	    }
	}

	segment = next_segment;
    }
  }
}

static void
_set_dirty_segment (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  DXmCSTextOutputSegment out_seg;
  int i;

  if (segment)
    {
      for ( i = 0; i < segment->num_output_segments; i++ )
      {
	  out_seg = &(segment->output_segment[i]);

	  if (out_seg == NULL)
		_validate_segment (widget, line, segment);

	  out_seg->need_redisplay = TRUE;
      }
    }
}

static void
_set_dirty_out_line (widget, location)
    DXmCSTextWidget widget;
    TextLocation    location;
{
    DXmCSTextOutputSegment out_seg;
    DXmCSTextOutputLine    out_line;
    TextLine line;
    TextSegment segment;
    DXmCSTextPosition offset;
    int i, j, char_count;

    if (!location)
	return;

    line    = location->line;
    segment = location->segment;
    offset  = location->offset;
    out_seg = (DXmCSTextOutputSegment) NULL;
    char_count = 0;

    if ( segment )
    {
	/* first find out which out_seg to set dirty
	 */
	for ( i = 0; i < segment->num_output_segments; i ++ )
	{
	    out_seg = &(segment->output_segment[i]);
	
	    if (out_seg == (DXmCSTextOutputSegment) NULL)
		_validate_segment (widget, line, segment);

	    char_count += out_seg->char_count;

	    if ( char_count > offset )
	    {
		break;
	    }
	}

	if (out_seg)
	    out_seg->need_redisplay = True;
    }

    /* now find which out_line this out_seg is in
     */
    if ( line )
    {
	for ( i = 0; i < line->num_output_lines; i ++ )
	{
	    out_line = &(line->output_line[i]);

	    if (out_line == (DXmCSTextOutputLine) NULL)
	        _validate_line(widget, line, NULL);

	    if ( out_seg == (DXmCSTextOutputSegment) NULL )
	    {
		out_line->need_redisplay = True;
	    }else if (out_line->y == out_seg->y) {
	        out_line->need_redisplay = True;
	        break;
	    }
	}
    }
}


/*
 * output module method to set the need_redisplay bit in the
 * given character position
 */

static void
SetCharRedraw (widget, location)
    DXmCSTextWidget widget;
    TextLocation location;
{
  DXmCSTextOutputSegment seg;
  int i, j;
  TextSegment segment;

  _set_dirty_out_line (widget, location);

  segment = location->segment;

  if ( segment )
    {
      for (i=0; i < segment->num_output_segments; i++)
	{
	  seg = &(segment->output_segment[i]);
	  
	  if ((location->offset >= seg->offset) &&
	      (location->offset < (seg->offset + seg->char_count)))
	    {
	      j = location->offset - seg->offset;
	      seg->per_char[j].need_redisplay = TRUE;
	      return;
	    }
	}
    }
}



/*
 * output module method to set the draw mode of the given
 * character
 */
static void
SetCharDrawMode (widget, location, mode)
    DXmCSTextWidget widget;
    TextLocation location;
    XmHighlightMode mode;
{
  DXmCSTextOutputSegment seg;
  int i, j, dirty_y, saved_idx=0;
  TextSegment segment;

  if (!location)
	return;

  _set_dirty_out_line (widget, location);

  if (location->segment)
    {
      for (i=0; i < location->segment->num_output_segments; i++)
	{
	  seg = &(location->segment->output_segment[i]);

	  dirty_y = seg->y;
	  saved_idx = i;
	  
	  if ((location->offset >= seg->offset) &&
	      (location->offset < (seg->offset + seg->char_count)))
	    {
	      j = location->offset - seg->offset;

	      if (seg->per_char[j].draw_mode != mode)
		{
		  seg->per_char[j].draw_mode = mode;
		  seg->per_char[j].need_redisplay = TRUE;
		}

	      break;
	    }
	}
    }

  /* now set all other out_line's out_segs to redisplay
   */

  i = saved_idx;

  for ( segment  = location->segment;
	segment != (TextSegment) NULL;
	segment  = _DXmCSTextGetNextSegment(segment) )
  {
    for ( ; i < segment->num_output_segments; i++ )
    {
	seg = &(segment->output_segment[i]);

	if ( seg->y == dirty_y )
	{
	    seg->need_redisplay = True;

	     for ( j = 0; j < seg->char_count; j++ )
	     {
		seg->per_char[j].need_redisplay = True;
	     }
	} else {
	     /* too far
	      */
	     return;
	}
    }

    i = 0;
  }

}


static void
SetDrawMode(widget, location, length, mode)
DXmCSTextWidget widget;
TextLocation location;
unsigned int length;
XmHighlightMode mode;
{
    if (!location)
	return;

    RendMgrSetRendition(RenditionMgr(widget), location, length, mode);
}


static void
_think_about_new_size (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  Boolean r_to_l;
  DXmCSTextOutputLine out_line;
  TextSegment first_segment, last_segment;
  DXmCSTextOutputSegment first_out_seg = NULL, last_out_seg = NULL;
  Dimension width = XtWidth (widget), height = XtHeight (widget) ;
  DXmCSTextPosition le, re, x;

  if (( ! CanResizeWidth (widget)) && ( ! CanResizeHeight (widget)))
    return;

  if (line == NULL) return;

     /*
      * we know something happened to this segment, so assume the width 
      * and the height are suspect.
      */

   /* resize width
    */

   if ( CanResizeWidth (widget) )
   {
     first_segment = _DXmCSTextGetFirstSegment (line);
     out_line = line->output_line;

     if (first_segment)
	first_out_seg = first_segment->output_segment;

     last_segment  = _DXmCSTextGetLastSegment (line);
     if (last_segment)
	last_out_seg  = last_segment->output_segment;

     if (out_line && first_out_seg && last_out_seg)
	{
	  /* BOGUS
	   * ltor case only... will have to scan all segs to find left and
	   * rightmost ones
	   */

          /* RtoL... */
          if (TextRtoL ( widget ))  
          {
             x = out_line->x - HorizontalOffset (widget) ;

  	     le = x + last_out_seg->x;

	     re = x + first_out_seg->x + ((int) first_out_seg->pixel_width);
          }
          else
          {
	  x = HorizontalOffset (widget) + out_line->x;

	  le = x + first_out_seg->x;

	  re = x + last_out_seg->x + ((int) last_out_seg->pixel_width);
          }

	  if ((le < LeftEdge (widget)) || (re > RightEdge (widget)))
	    {
	      /*
	       * well, something on this line hangs out too far, so decide
	       * how big we'd like to be
	       */
	      
	      _compute_output_size (widget, 
				    &width, &height, 
				    _COMPUTE_GROW_SIZE);
	    }
	}
     }


     /*
      * if the segment parameter is NULL then something changed inthe
      * line so assume the height is the only issue, so go to the last line
      * and 'think' with his height and the current widget width.
      */

   if (CanResizeHeight (widget)) {

     /* resize height
      */

     line     = _DXmCSTextGetLastLine (widget);
     out_line = &(line->output_line[line->num_output_lines-1]);

     if (out_line && 
	  ((int) (VerticalOffset (widget) +
		  out_line->y + 
		  out_line->pixel_height) > BottomEdge (widget)))
	{
	  _compute_output_size (widget, 
				&width, &height, 
				_COMPUTE_GROW_SIZE);
	}

  }

  /* recompute displayed lines when resizing Rtol */

  r_to_l = widget->primitive.dxm_layout_direction == DXmLAYOUT_LEFT_DOWN;

  if (TryResize (widget, width, height, r_to_l) == XtGeometryYes &&
      TextRtoL(widget) )      
  {
     /* really need to invalidate only displayed lines */
     _invalidate_all (widget) ; 
  }    			

}



static void
NumLinesOnText (widget, num_lines)
    DXmCSTextWidget widget;
    int *num_lines;
{
    *num_lines = (NetHeight (widget) / ((int) TypicalHeight (widget))) - 1;
}

static void
ScanNextLine (widget, position, location, out_position)
    DXmCSTextWidget   widget;
    DXmCSTextPosition position;
    TextLocation      location;
    DXmCSTextPosition *out_position;
{
    TextLine    line, l;
    TextSegment segment, s;
    DXmCSTextOutputSegment out_seg;
    int i, count, saved_y;
    Boolean found;
    DXmCSTextPosition pos;

    if ( !location )
    {
	*out_position = position;
	return;
    }

    segment = location->segment;
    line    = location->line;

    /* if end of the line, use the prev char's segment
     */
    if ( !segment )
    {      
	if (location->end_of_line) 
	{
            if ( !_DXmCSTextNavNextChar (location) ) 
                *out_position = position;
            else 
                *out_position = location->position;
            return;
        }

	segment = location->segment;
	line    = location->line;
	location->position = position;   /* reset to original position */
    }

    /* if still empty, just return
     */
    if ( !segment )
    {
	*out_position = position;
	return;
    }

    count = 0;

    /* figure out which out_seg the location is in
     */
    for ( i = 0; i < segment->num_output_segments; i++ )
    {
	out_seg = &(segment->output_segment[i]);

	count += out_seg->char_count;

	if ( count > location->offset )
		break;
    }

    found = False;
    saved_y = out_seg->y;

    /* now find next segments/out_segs until the out_seg->y is greater than
     * saved_y
     */

    for ( l  = line;
	  l != NULL && !found;
	  l  = _DXmCSTextGetNextLine(l) )
    {
	segment = _DXmCSTextGetFirstSegment(l);

        for ( s = segment;
	      s != NULL && !found;
	      s = _DXmCSTextGetNextSegment(s) )
        {
	    for ( i = 0; i < s->num_output_segments; i++ )
	    {
	        out_seg = &(s->output_segment[i]);

	        if ( out_seg->y > saved_y && out_seg->char_count >= 0 )
	        {
		    found = True;
		    break;
	        }
	    }

	    if ( found )
	        break;
	}

	if ( found )
	    break;
    }

    /* can't find any next line, just return the last position
     */
    if ( !found )
    {
	*out_position = (DXmCSTextPosition) CSTextLength(widget);
	return;
    }

    /* now s, l and out_seg contain our desired location.  We can now figure out
     * the position corresponding to them
     */
    *out_position = _DXmCSTextSourceGetPosition ( widget, l, s,
						  out_seg->offset );
    return;
}


static void
ScanPrevLine (widget, position, location, out_position)
    DXmCSTextWidget   widget;
    DXmCSTextPosition position;
    TextLocation      location;
    DXmCSTextPosition *out_position;
{
    TextLine    line, l;
    TextSegment segment, s;
    DXmCSTextOutputSegment out_seg;
    int i, count, saved_y;
    Boolean found;
    DXmCSTextPosition pos;

    if ( !location )
    {
	*out_position = position;
	return;
    }

    segment = location->segment;
    line    = location->line;

    /* if end of the line, use the previous char's segment
     */
    if ( !segment )
    {
	if (location->end_of_line) 
	{
            if ( !_DXmCSTextNavPrevChar (location) ) 
            {
                *out_position = position;
                return;
            }
            else {
                if (location->end_of_line) 
                { 
                    *out_position = location->position;
                    return;
                }
            }
        }

	segment = location->segment;
	line    = location->line;
	location->position = position;   /* reset to original position */
    }

    /* if still empty, just return
     */
    if ( !segment )
    {
	*out_position = position;
	return;
    }

    count = 0;

    /* figure out which out_seg the location is in
     */
    for ( i = 0; i < segment->num_output_segments; i++ )
    {
	out_seg = &(segment->output_segment[i]);

	count += out_seg->char_count;

	if ( count > location->offset )
		break;
    }

    found = False;
    saved_y = out_seg->y;

    /* now find prev segments/out_segs until the out_seg->y is less than
     * saved_y
     */

    for ( l = line;
	  l != NULL && !found;
	  l = _DXmCSTextGetPrevLine(l) )
    {
	segment = _DXmCSTextGetLastSegment(l);

	for ( s = segment;
	      s != NULL && !found;
	      s = _DXmCSTextGetPrevSegment(s) )
        {
	    for ( i = s->num_output_segments - 1; i >= 0; i-- )
	    {
	        out_seg = &(s->output_segment[i]);

	        if ( out_seg->y < saved_y && out_seg->char_count >= 0 )
	        {
		    found = True;
		    break;
	        }
	    }

	    if ( found )
	        break;
	}

	if ( found )
	    break;
    }

    /* can't find any previous line, just return the 0 position
     */
    if ( !found )
    {
	*out_position = (DXmCSTextPosition) 0;
	return;
    }

    /* now s, l and out_seg contain our desired location.  We can now figure out
     * the position corresponding to them
     */
    *out_position = _DXmCSTextSourceGetPosition ( widget, l, s,
						  out_seg->offset );

    return;
}

static void
ScanStartOfLine (widget, location, out_position)
    DXmCSTextWidget   widget;
    TextLocation      location;
    DXmCSTextPosition *out_position;
{
    int i;
    DXmCSTextOutputSegment out_seg;
    DXmCSTextOutputLine    out_line;
    TextLine	line;
    TextSegment segment;
    Boolean found;
    TextLocationRec temp_location;
    
    if (location->end_of_line) {
	if (!location->line) {
	    *out_position = 0;
	    return;
	}
	if (location->line->length == 0) {
	    *out_position = location->position;
	    return;
	}
	_DXmCSTextSourceLocate(widget, location->position - 1,
			       &temp_location);
	location = &temp_location;
    }

    if (location->end_of_line) {
	if (!location->line) {
	    *out_position = 0;
	    return;
	}
	if (location->line->length == 0) {
	    *out_position = location->position;
	    return;
	}
	_DXmCSTextSourceLocate(widget, location->position - 1,
			       &temp_location);
	location = &temp_location;
    }

    line    = location->line;
    segment = location->segment;

    if (!segment || !line)
    {
	*out_position = 0;
	return;
    }

    /* find out the out_seg corresponding to location->offset
     */
    for ( i = 0;
	  i <= segment->num_output_segments - 1;
	  i++ )
    {
	out_seg = &(segment->output_segment[i]);

	if ( out_seg->offset + out_seg->char_count - 1 >= location->offset )
		break;
    }

    /* out_seg contains the location.  Is it the start of line?  If not, find
     * it.
     */
    found = False;

    if (!start_of_line (widget, out_seg))
    {
	while ( !found )
	{
	    segment = _DXmCSTextGetPrevSegment(segment);

	    if (!segment)
	    {
		*out_position = (DXmCSTextPosition) 0;
		return;
	    }

	    for ( i = segment->num_output_segments - 1;
		  i >= 0;
		  i-- )
	    {
		out_seg = &(segment->output_segment[i]);

                if (start_of_line(widget, out_seg))
		{
		    found = True;
		    break;
		}
	    }
	}
    }

    /* out_seg contain the location we want.  find out the corresponding pos
     */
    *out_position = _DXmCSTextSourceGetPosition ( widget, line, segment,
				  		  out_seg->offset );
	  
    return;
}

static void
ScanEndOfLine (widget, location, out_position)
    DXmCSTextWidget   widget;
    TextLocation      location;
    DXmCSTextPosition *out_position;
{

    int i, saved_y;
    DXmCSTextOutputSegment out_seg, tmp_out_seg;
    DXmCSTextOutputLine    out_line;
    TextLine	line;
    TextSegment segment, s;
    Boolean found;

    if (location->end_of_line) {
	*out_position = location->position;
	return;
    }
    
    line    = location->line;
    segment = location->segment;

    if (!segment || !line)
    {
	*out_position = 0;
	return;
    }

    /* find out the out_seg corresponding to location->offset
     */
    for ( i  = 0;
	  i <= segment->num_output_segments - 1;
	  i++ )
    {
	out_seg = &(segment->output_segment[i]);

	if ( out_seg->offset + out_seg->char_count - 1 >= location->offset )
		break;
    }

    /* out_seg contains the location.  Look next out_seg's to find the one
     * with y > than this out_seg
     */
    found = False;
    saved_y = out_seg->y;
    s = segment;

    while ( !found )
    {
	if (!s)
	{
	    *out_position = _DXmCSTextSourceGetPosition (widget, line,
							 segment,
							 segment->length);
	    return;
	}

	for ( i = 0;
	      i <= s->num_output_segments - 1;
	      i++ )
	{
	    tmp_out_seg = &(s->output_segment[i]);

	    if (tmp_out_seg->y > saved_y)
	    {
		found = True;
		break;
	    }

	    /* save the out_seg and segment
	     */
	    out_seg = tmp_out_seg;
	    segment = s;
	}

	if (!found)
	{
	    s = _DXmCSTextGetNextSegment(s);
	}
    }

    /* out_seg contain the location we want.  find out the corresponding pos
     */
    *out_position = _DXmCSTextSourceGetPosition ( widget, line, segment,
				    out_seg->offset + out_seg->char_count );
	  
    return;
}


static
Boolean _NavNextSegment( location )
TextLocation location;
{
  TextSegment seg;
  Boolean ok = TRUE;

  for (seg = location->segment;
       (seg == location->segment) && ok;
       ok = _DXmCSTextNavNextChar (location))
    ;

  return (ok);
}

typedef struct {
    DXmCSTextCvtContext context;
    int main_path ;    /* widget's main text path */
    XmStringCharSet charset;
    XmStringDirection direction;
    short locale;
    int nest_level;
    char *text;
    int  byte_length;
    int  current_y;
} ConvObjRec, *ConvObj;
    

static
char *_flip_text( text, length) 
char *text ;
int  length ;
{
     /* Flip text in place for efficiency
        (This routine does not support MULTI-BYTE characters)
     */
     char c ;
     int  i ;

     if (length < 2) return text ; /* Nothing to flip */
     length-- ;
     for (i = 0 ; i < length; i++, length--)
     {
         c            = text[length] ;
         text[length] = text[i] ;
         text[i]      = c ; 
     }
     return text ;
}


static void _ConvObjEmitLevel0(co)
ConvObj co;
{
     DXmCSTextCvtContext context = co->context;

	context->text = (char *) NULL ;
	context->byte_length = 0 ;
	context->charset = 0 ;
	context->direction = co->main_path ;
	context->locale = 0 ;
	context->nest_level = 0 ;

	(*context->segment_cb)(context);
}


ConvObj ConvObjInit(context, main_path)
DXmCSTextCvtContext context;
int main_path ;
{
    ConvObj co;

    if (!context)
	return NULL;

    co = (ConvObj)XtMalloc(sizeof(ConvObjRec));
    co->context = context;
    co->main_path = main_path ;
    co->charset = NULL;
    co->direction = 0;
    co->locale = 0;
    co->nest_level = 0;
    co->text = NULL;
    co->byte_length = 0;
    co->current_y = -1;

    return co;
}

void ConvObjFree(co)
ConvObj co;
{
    register DXmCSTextCvtContext context = co->context;

    if (co->text) {
    	if (co->direction != co->main_path) 
	   context->text = _flip_text(co->text, co->byte_length) ;
	else
	   context->text = co->text;
	context->byte_length = co->byte_length;
	context->charset = co->charset;
	context->direction = co->direction;
	context->locale = co->locale;
	context->nest_level = co->nest_level;

	(*context->segment_cb)(context);

	XtFree(co->text);
	co->text = NULL;
	co->byte_length = 0;
    }

    if ( co->current_y != -1 )  /* didn't emit yet */
    {
         /* assure a closing level of 0 */
	 _ConvObjEmitLevel0( co ) ;
    }	  
    if (co->charset)
	XtFree(co->charset);
    XtFree((char *)co);
}


void ConvObjSetInfo(co, charset, direction, locale, nest_level)
ConvObj co;
XmStringCharSet charset;
XmStringDirection direction;
short locale;
int nest_level;
{
    register DXmCSTextCvtContext context = co->context;

    if ((charset && co->charset && strcmp(charset, co->charset) == 0) &&
	direction == co->direction &&
	locale == co->locale &&
	nest_level == co->nest_level)
	return;

    if (co->text) {
    	if (co->direction != co->main_path) 
	   context->text = _flip_text(co->text, co->byte_length) ;
	else
	   context->text = co->text;
	context->byte_length = co->byte_length;
	context->charset = co->charset;
	context->direction = co->direction;
	context->locale = co->locale;
	context->nest_level = co->nest_level;

	(*context->segment_cb)(context);

	XtFree(co->text);
	co->text = NULL;
	co->byte_length = 0;
    } else if ( direction != co->direction && co->nest_level == 0) {
         /* record the nest level */
	 _ConvObjEmitLevel0( co ) ;
    }	  

    if (co->charset)
	XtFree(co->charset);
    co->charset = XtMalloc(strlen(charset) + 1);
    strcpy(co->charset, charset);
    co->direction = direction;
    co->locale = locale;
    co->nest_level = nest_level;
}


void ConvObjForceLineBreak(co)
ConvObj co;
{
    register DXmCSTextCvtContext context = co->context;

    if (co->text) {
    	if (co->direction != co->main_path) 
	   context->text = _flip_text(co->text, co->byte_length) ;
	else
	   context->text = co->text;
	context->byte_length = co->byte_length;
	context->charset = co->charset;
	context->direction = co->direction;
	context->locale = co->locale;
	context->nest_level = co->nest_level;

	(*context->segment_cb)(context);

	XtFree(co->text);
	co->text = NULL;
	co->byte_length = 0;
    }
    _ConvObjEmitLevel0( co ) ;  /* assure a closing level 0 */
    (*context->line_cb)(context);
    co->current_y = -1;
}


void ConvObjSetSegment(co, out_seg, start_offset, end_offset)
ConvObj co;
DXmCSTextOutputSegment out_seg;
int start_offset;
int end_offset;
{
    int start_byte, end_byte, byte_len;

    if (co->current_y != -1 && co->current_y != out_seg->y)
	ConvObjForceLineBreak(co);

    if (co->current_y == -1)
	co->current_y = out_seg->y;
    
    if (end_offset == -1)
	end_offset = out_seg->char_count;

    if (out_seg->char_count == 0) {
	if (!co->text) {
	    co->byte_length = 0;
	    co->text = XtMalloc(1);
	    co->text[0] = '\0';
	}
    } else {
	I18nSegmentMeasure(NULL,
			   co->charset,
			   out_seg->xlib_buff->text,
			   start_offset,
			   end_offset,
			   &start_byte,
			   &end_byte);
	byte_len = end_byte - start_byte;
	co->byte_length += byte_len;
	
	if (co->text) {
	    co->text = XtRealloc(co->text, co->byte_length + 1);
	    strncat(co->text, (char *)out_seg->xlib_buff->text + start_byte, byte_len);
	    co->text[co->byte_length] = '\0';
	} else {
	    co->text = XtMalloc(co->byte_length + 1);
	    strncpy(co->text, (char *)out_seg->xlib_buff->text + start_byte, byte_len);
	    co->text[co->byte_length] = '\0';
	}
    }
}



static
DXmCSTextStatus
ReadString (widget, start, end, string)
    DXmCSTextWidget   widget;
    DXmCSTextPosition start;
    DXmCSTextPosition end;
    XmString	      *string;
{
    DXmCSTextCvtContextRec context;
    TextLocationRec location, end_location, start_location;
    TextLine line;
    TextSegment segment;
    int i, j;
    DXmCSTextOutputLine  out_line;
    ConvObj co;

    *string = (XmString)NULL;

    if( start >= CSTextLength (widget) || start > end )
    {
	return I_STR_EditError;
    }

    _DXmCSTextSourceLocate( widget, 0, &location );
    _DXmCSTextSourceLocate( widget, start, &start_location );
    _DXmCSTextSourceLocate( widget, end, &end_location );

    if( location.line == (TextLine) NULL )
    {
	return I_STR_EditDone;
    }

    context.status = I_STR_EditDone;

    DXmCvtITextToCSInit( &context );

/* this is extraneous  - 
   no need to send all empty segments if we guarantee a level 0 segment
   at start of each line

    ** first send empty segments with direction and nest level until hit **
    ** the start segment
    **
    while( location.segment != start_location.segment )
    {
	int current_level = 0;

	segment = location.segment;

	if( segment && segment->nest_level != current_level )
	{
	    current_level = segment->nest_level;

	    context.text        = (char *)NULL;
	    context.byte_length = 0;
	    context.charset     = location.segment->charset;
	    context.direction   = location.segment->direction;
	    context.locale      = location.segment->locale;
	    context.nest_level  = location.segment->nest_level;

	    (*context.segment_cb) (&context);

	}
	if( !_NavNextSegment( &location ) ) break;
    }
 */

    /* loop thru all lines
     */
    co = ConvObjInit(&context, CSTextPath(widget) ) ;

    for (line = start_location.line;
	 line;
	 line = _DXmCSTextGetNextLine(line)) {
	Boolean segment_start = False, segment_end = False;

        _validate_line(widget, line, NULL);

	for (segment = _DXmCSTextGetFirstSegment(line);
	     segment;
	     segment = _DXmCSTextGetNextSegment(segment)) {
	    if (line == start_location.line) {
		if (segment == start_location.segment)
		    segment_start = True;
		if (!segment_start)
		    continue;
	    }
	    if (line == end_location.line) {
		if (segment_end)
		    break;
		if (segment == end_location.segment)
		    segment_end = True;
	    }
	    ConvObjSetInfo(co, segment->charset, segment->direction, 
			   segment->locale, segment->nest_level);
	    for ( j = 0; j < segment->num_output_segments; j++ ) {
		DXmCSTextOutputSegment out_seg = &(segment->output_segment[j]);
		register int start_offset = 0;
		register int end_offset = -1;

		if (line == start_location.line &&
		    segment == start_location.segment) {
		    if (start_location.offset >=
			  out_seg->offset + out_seg->char_count)
			break;
		    else if (start_location.offset >= out_seg->offset &&
			     start_location.offset <
			       out_seg->offset + out_seg->char_count)
			start_offset = start_location.offset - out_seg->offset;
		}

		if (line == end_location.line &&
		    segment == end_location.segment) {
		    if (end_location.offset < out_seg->offset)
			break;
		    else if (end_location.offset >= out_seg->offset &&
			     end_location.offset <
			       out_seg->offset + out_seg->char_count)
			end_offset = end_location.offset - out_seg->offset;
		}

		ConvObjSetSegment(co, out_seg, start_offset, end_offset);
	    }
	}
	if (line != end_location.line)
	    ConvObjForceLineBreak(co);
    }
    ConvObjFree(co);
    DXmCvtITextToCSEnd( &context );
    *string = context.stream;
    return ( context.status );
}

/*
 * basic output module method to handle the output data which is
 * dangled off the source line and segment data structures.
 */

static void
HandleData (widget, line, segment, op_code)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
    DXmCSTextDataOpCode op_code;         /* depending on op_code */
{
  DXmCSTextOutputData data = OutputData(widget);
  int previous_line_count;
  register int old_line_count;

#ifdef ASIAN_IO_SHR
  AIMStruct aim_ptr = (AIMStruct)AIMRec(widget);
  if (aim_ptr->end_istate_allowed && aim_ptr->aim_widget) {
      DXmAIMCancelIState(aim_ptr->aim_widget);
  }
#endif

  switch (op_code)
    {

      case DXmCSTextOutputLineAdd:       /* new line being built by source */
	_validate_rest_line (widget, line, segment);
	LineCount (widget) = _compute_num_output_lines (widget);
	_think_about_new_size (widget, line, segment);
	_set_vbar (widget);
	old_line_count = LineCount(widget);
	_update_line(widget, line, UL_CREATE);
        if (LineCount(widget) != old_line_count) {
	    _invalidate_rest_line(widget, _DXmCSTextGetNextLine (line), NULL);
	    BarManagerNeedVBarUpdate(widget);
        }
	_think_about_new_size (widget, line, segment);
	break;


      case DXmCSTextOutputSegmentDelete: /* dump output data for this seg */

	_dump_segment (widget, line, segment);

	break;

      case DXmCSTextOutputLineDelete:    /* dump output data for this line */

	_dump_line(widget, line, segment);
	_invalidate_rest_line(widget, line, segment);
	old_line_count = LineCount(widget);
        _update_line(widget, line, UL_DELETE);
        if (LineCount(widget) != old_line_count) {
	    _invalidate_rest_line(widget, _DXmCSTextGetNextLine (line), NULL);
	    BarManagerNeedVBarUpdate(widget);
	}
	break;

      case DXmCSTextOutputDelete:        /* dump output data for everything */
/* Bug fix
        _dump_all (widget);
** Performance /Mak
        _validate_all (widget);
 **
        LineCount(widget) = 0;
 */
	_dump_all(widget);
	_validate_all(widget);
	LineCount(widget) = _compute_num_output_lines(widget);
        break;


      case DXmCSTextOutputSegmentModified:  /* something in seg changed */
	/* have to consider the reverse wrap, so for word wrap, have to dump
	 * the segment
	 */
	if (data->wordwrap)
	{
		_dump_rest_line(widget, line, segment);
		_validate_rest_line (widget, line, segment);
	} else {
		_invalidate_rest_line(widget, line, segment);
		_validate_rest_line (widget, line, segment);
	}

	previous_line_count = LineCount (widget);
	LineCount (widget) = _compute_num_output_lines (widget);
	_think_about_new_size (widget, line, segment);
	_set_hbar (widget);
	if (LineCount (widget) != previous_line_count)
	    _set_vbar (widget);
	if (data->wordwrap)
	{
	    old_line_count = LineCount(widget);
            _update_line(widget, line, UL_DUMP);
	    if (LineCount(widget) != old_line_count) {
	        _invalidate_rest_line(widget, _DXmCSTextGetNextLine (line),
				      NULL);
	        BarManagerNeedVBarUpdate(widget);
	    }
	} else {
            _invalidate_line(widget, line, NULL);
	}

	_think_about_new_size (widget, line, segment);
	BarManagerNeedHBarUpdate(widget);
	_set_top_position (widget);
	break;

      case DXmCSTextOutputLineModified:  /* something in line changed */

	_dump_rest_line (widget, line, segment);
	_validate_all (widget);
	LineCount (widget) = _compute_num_output_lines (widget);
	_think_about_new_size (widget, line, segment);
	_set_vbar (widget);
	if (data->wordwrap) {
	    register int old_line_count = LineCount(widget);
            _update_line(widget, line, UL_DUMP);
	    if (LineCount(widget) != old_line_count) {
	        _invalidate_rest_line(widget, _DXmCSTextGetNextLine (line),
				      NULL);
	        BarManagerNeedVBarUpdate(widget);
	    }
	} else {
            _invalidate_line(widget, line, NULL);
	}
	if (line->length == 0) {
	    RendMgrSetRenditionLine(RenditionMgr(widget), line,
					RmHIGHLIGHT_NORMAL);
	}
	_think_about_new_size (widget, line, segment);
	break;

      case DXmCSTextSegmentValidateData: /* make sure output data for seg */
                                        /* is current */
	_validate_segment (widget, line, segment);
	break;

      case DXmCSTextLineValidateData:    /* make sure output data for line */
                                        /* is current, implies all segs */
	_validate_line (widget, line, segment);
	break;

      case DXmCSTextValidateData:        /* make sure output data for whole */
                                        /* source is current */
	_validate_all (widget);
	break;

      case DXmCSTextOutputSingleLineDelete:  /* dump this segment only */
					     /* don't do any validation yet */

	_dump_line (widget, line, segment);
	old_line_count = LineCount(widget);
        _update_line(widget, line, UL_DELETE);
        if (LineCount(widget) != old_line_count) {
	    _invalidate_rest_line(widget, _DXmCSTextGetNextLine (line), NULL);
	    BarManagerNeedVBarUpdate(widget);
        }

        break;

      case DXmCSTextOutputSingleSegmentDelete: /* dump this line only */
					       /* don't do any validation yet*/
	/* for word wrap, have to dump the whole line
	 */
	if ( data->wordwrap )
	{
	    _dump_rest_line(widget, line, segment);
	    old_line_count = LineCount(widget);
            _update_line(widget, line, UL_DUMP);
	    if (LineCount(widget) != old_line_count)
	        _invalidate_rest_line(widget, _DXmCSTextGetNextLine (line),
				      NULL);
	} else {
/* Bug fix - A deletion of a segment will affect the rest segments of the 
 * line. /Mak
 */
            _invalidate_line(widget, line, NULL);
	    _dump_segment (widget, line, segment);
	}

	if (line->length == 0) {
	    RendMgrSetRenditionLine(RenditionMgr(widget), line,
					RmHIGHLIGHT_NORMAL);
	}

        break;

      case DXmCSTextOutputRestLineValidate:     /* validate starting from
						/* this line */
        _validate_rest_line (widget, line, segment);

        break;

      case DXmCSTextOutputSingleLineInvalidate:  /* invalidate this single
						 */
        _invalidate_line (widget, line, segment);
  
        break;

      case DXmCSTextOutputTextEmpty :
	RendMgrResetRendition(RenditionMgr(widget));
	break;

      default:
#ifdef DEBUG
	printf ("unknown op code in output handle data proc\n");
#endif
	break;
    }
}



static void
_dump_segment (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  int i;
  DXmCSTextOutputSegment out_seg;

  if ( !line )
	return;

  if ( !segment )
	segment = _DXmCSTextGetFirstSegment(line);

  for (i=0; i < segment->num_output_segments; i++)
    {
      out_seg = &(segment->output_segment[i]);

      if (out_seg->per_char)
	{
	  XtFree ((char *)out_seg->per_char);
	  out_seg->per_char = NULL;
	}

      if (out_seg->xlib_buff)  /* free up the xlib_buff */
        {
	  if ((out_seg->xlib_buff)->text)
	  {
              XtFree((out_seg->xlib_buff)->text);
              (out_seg->xlib_buff)->text = NULL;
	  }

	  XtFree((char *)out_seg->xlib_buff);
	  out_seg->xlib_buff = NULL;

	  out_seg->xlib_buff_count = 0;
        }

      if (out_seg->font_entry)       /* invalidate font pointer */
        {
          out_seg->font_entry = NULL;
        }
    }

  if (segment->output_segment)
    XtFree ((char *)segment->output_segment);

  segment->output_segment = NULL;
  segment->num_output_segments = 0;
}

static void
_dump_line (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{

  /* dump segments first
   */
  if (line)
  {
	segment = _DXmCSTextGetFirstSegment(line);

	while (segment != NULL)
	{
	    _dump_segment (widget, line, segment);
	    segment = _DXmCSTextGetNextSegment (segment);
	}
  }

  /* clean up output lines
   */

  if (line->output_line)
	XtFree ((char *) line->output_line );

  line->output_line = NULL;
  line->num_output_lines = 0;
}

static void
_dump_all (widget)
   DXmCSTextWidget widget;
{
   TextLine line = _DXmCSTextGetFirstLine (widget);

   while (line != NULL)
     {
       _dump_line (widget, line, NULL);
       line = _DXmCSTextGetNextLine (line);
     }
}

/* dump the rest of the segment  */

static void
_dump_rest_segment (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{

    if (segment->output_segment)
      {
        while (segment != NULL)
          {
	    _dump_segment (widget, line, segment);
	    segment = _DXmCSTextGetNextSegment (segment);
	  }
      }
}

/* dump the rest of the line */

static void
_dump_rest_line (widget, line, segment)
   DXmCSTextWidget widget;
   TextLine line;
   TextSegment segment;
   
{

   while (line != NULL)
     {
       _dump_line (widget, line, NULL);
       line = _DXmCSTextGetNextLine (line);
     }
}


/*
 * make sure we have up-to-date information inthe output segment list
 */
static void
_validate_segment (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  DXmCSTextOutputSegment out_seg;
  int i, j, ttl_char_count;
  Boolean length_changed, need_validate;
  ttl_char_count = 0;
  length_changed = False;

  need_validate = False;

  if ( ! segment->output_segment)
    {
      segment->num_output_segments = 1;

      _alloc_segment (widget, line, segment);

      need_validate = True;
    }
  else
    {
      for (i=0; i < segment->num_output_segments; i++)
      {
	   out_seg = &(segment->output_segment[i]);
	   ttl_char_count += out_seg->char_count;

	   if ( out_seg->need_recompute )
	   {
		need_validate = True;
	   }
      }

      if ( ttl_char_count != segment->length )
      {
	   length_changed = True;
	   need_validate  = True;
      }

      ttl_char_count = 0;

      for (i=0; i < segment->num_output_segments; i++)
        {
          out_seg = &(segment->output_segment[i]);

	  ttl_char_count += out_seg->char_count;

	  if ( length_changed )
	    {
	      /* set char_count
	       */

	      /* if total char_count so far > total segment length, then
	       * it must be bogus
	       */
	      if ( ttl_char_count > segment->length )
	      {
		    out_seg->char_count = segment->length;

	      } else if ( i == segment->num_output_segments - 1) {

		   /* increment the last out_seg->char_count only
		    */
		    out_seg->char_count += segment->length - ttl_char_count;
	      }

	      XtFree ((char *)out_seg->per_char);

	      out_seg->per_char = (DXmCSTextOutputChar)
		XtMalloc (out_seg->char_count * 
			  sizeof (DXmCSTextOutputCharRec));

	      for (j=0; j < out_seg->char_count; j++)
		{
		  out_seg->per_char[j].need_redisplay = TRUE;
		  out_seg->per_char[j].draw_mode      = XmHIGHLIGHT_NORMAL;
		}
	    }
	}
    }

  if ( need_validate )
      _compute_segment (widget, line, segment);

}

static void
_invalidate_segment (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  int i, j;
  DXmCSTextOutputSegment seg;

  if ( ! segment->output_segment)
    {
      segment->num_output_segments = 1;

      _alloc_segment (widget, line, segment);
    }
  else
    {
      for (i=0; i < segment->num_output_segments; i++)
	{
	  seg = &(segment->output_segment[i]);
	  
	  seg->need_recompute = TRUE;
	  seg->need_redisplay = TRUE;

          if (seg->xlib_buff)
          {
	     if (seg->xlib_buff->text)
	     {
                 XtFree(seg->xlib_buff->text);
                 seg->xlib_buff->text = NULL;
	     }

	     XtFree((char *)seg->xlib_buff);
	     seg->xlib_buff = NULL;

	     seg->xlib_buff_count = 0;
          }

	  for (j=0; j < seg->char_count; j++)
	    {
	      seg->per_char[j].need_redisplay = TRUE;
	    }
	}
    }
  
}


static void
_validate_rest_line (widget, line, segment)
   DXmCSTextWidget widget;
   TextLine line;
   TextSegment segment;
{

   while (line != NULL)
     {
       _validate_line (widget, line, NULL);
       line = _DXmCSTextGetNextLine (line);
     }
}


static void
_validate_line (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  DXmCSTextOutputLine line_out;

  if ( ! line->output_line)
  {
    line->num_output_lines = 1;
    _alloc_line (widget, line, segment);
  }

  for (segment = _DXmCSTextGetFirstSegment (line);
       segment != NULL;
       segment = _DXmCSTextGetNextSegment (segment))
    {
      _validate_segment (widget, line, segment);
    }

  _compute_line (widget, line, segment);

}

static void
_invalidate_rest_line (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  DXmCSTextOutputLine out_line;
  int i;

  while ( line )
  {
    if ( ! line->output_line)
    {
      line->num_output_lines = 1;
      _alloc_line (widget, line, segment);
    }

    for ( i = 0; i < line->num_output_lines; i ++ )
    {
	out_line = &(line->output_line[i]);

	out_line->need_recompute = TRUE;
	out_line->need_redisplay = TRUE;
    }

    for (segment = _DXmCSTextGetFirstSegment(line);
         segment != NULL;
         segment = _DXmCSTextGetNextSegment (segment))
    {
        _invalidate_segment (widget, line, segment);
    }

    line = _DXmCSTextGetNextLine(line);
  }

}


static void
_invalidate_line (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  DXmCSTextOutputLine out_line;
  int i;

  if ( ! line->output_line)
  {
    line->num_output_lines = 1;
    _alloc_line (widget, line, segment);
  }

  for ( i = 0; i < line->num_output_lines; i ++ )
  {
	out_line = &(line->output_line[i]);

	out_line->need_recompute = TRUE;
	out_line->need_redisplay = TRUE;
  }

  for (segment  = _DXmCSTextGetFirstSegment(line);
       segment != (TextSegment) NULL;
       segment  = _DXmCSTextGetNextSegment (segment))
    {
      _invalidate_segment (widget, line, segment);
    }

}


/* Performance /Mak */
static void _update_line(widget, line, type)
DXmCSTextWidget widget;
TextLine line;
UpdateLineType type;
{
    if (type == UL_CREATE) {
        _validate_line(widget, line, NULL);
        LineCount(widget) += line->num_output_lines;
    } else if (type == UL_DUMP) {
        register int old_num_out_lines, delta;

	old_num_out_lines = line->num_output_lines;
        _dump_line(widget, line, NULL);
        _validate_line(widget, line, NULL);
	delta = line->num_output_lines - old_num_out_lines;
        LineCount(widget) += delta;
    } else if (type == UL_INVALIDATE) {
	/* _invalidate_line doesn't change the number of output lines
	 * BY DEFINITION!
         */
        _invalidate_line(widget, line, NULL);
        _validate_line(widget, line, NULL);
    } else if (type == UL_DELETE) {
        LineCount(widget) -= line->num_output_lines;
    }
}


static void
_validate_all (widget)
   DXmCSTextWidget widget;
{
   TextLine line = _DXmCSTextGetFirstLine (widget);

   while (line != NULL)
     {
       _validate_line (widget, line, NULL);
       line = _DXmCSTextGetNextLine (line);
     }
}


static void
_invalidate_all (widget)
   DXmCSTextWidget widget;
{
   TextLine line = _DXmCSTextGetFirstLine (widget);

   while (line != NULL)
     {
       _invalidate_line (widget, line, NULL);
       line = _DXmCSTextGetNextLine (line);
     }
}


/*
 * compute where each segment goes, this should be doable since we know
 * that all source (logical) preceeding segments have been computed.
 *
 * all positions are relative to the line x, y
 */

static void
_compute_segment (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  int i, j;
  TextSegment p;
  int start_offset, end_offset;

  for (i=0; i < segment->num_output_segments; i++)
    {

      if (segment->output_segment[i].font_entry == NULL)
	{
           /* Get the font entry and compute the appropriate tab width */
	  _get_font_entry(widget, segment, &segment->output_segment[i]);
	}

      if (segment->output_segment[i].xlib_buff == (I18nXlibBuffers) NULL)
	{
	  _get_xlib (widget, line, segment, &segment->output_segment[i]);
	  segment->output_segment[i].need_recompute = TRUE;
        }

      if (segment->output_segment[i].need_recompute)
	{
	  int accu_width;

	  /* go to previous source segment for accu_width from previous
	   * segment 
	   */

	  p = _DXmCSTextGetPrevSegment (segment);

	  if (p == (TextSegment) NULL)
	  {
		accu_width = 0;    /* this is the first segment */

	  } else {

		int k;

		  /*
		   * previous source segment is there, so, we get the
		   * last output segment of the previous source segment
		   * and go next to him
		   */

/* Bug fix /Mak */
		if (!p->output_segment)
		    _validate_segment(widget, line, p);

		k = p->num_output_segments - 1;

                if (TextRtoL (widget) )
                     accu_width = RightMargin (widget)
                                            - p->output_segment[k].x;
                else
		accu_width = p->output_segment[k].x +
				((int) p->output_segment[k].pixel_width);
	  }

          /* BOGUS  should loop thru all segment->output_segment[i].xlib_buff 
           */

	  if ( segment->output_segment[i].xlib_buff )
	  {
	     segment->output_segment[i].pixel_width  =
		 _find_width (segment->output_segment[i].tab_width,
			      *segment->output_segment[i].font_entry,
			      segment->output_segment[i].xlib_buff->text,
			      segment->output_segment[i].char_count,
			      accu_width);
	  } else {
	     segment->output_segment[i].pixel_width = 0;
	  }

	  segment->output_segment[i].pixel_height = MaxAscent (widget) +
	      MaxDescent (widget);   /* Fixed by JRD */

	  /* BOGUS
	     for now we ignore all the hard cases like word wrap and
	     directionality, that SHOULD be computable at this point
	     since we are moving in LOGICAL order...
	   */
	  if ( i == 0 )
	    {
	      if (p == NULL)
		{
		  DXmCSTextOutputLine out_line;
		  TextLine            prev_line;
		  int		      num_out_line;

		  /* 
		   * doing first segment on line, this is relative to the
		   * line x, y so it's pretty easy.  Well, dealing with
		   * word wrapping is not that easy.  Should see if previous
 		   * exists first.
		   */
		  prev_line = _DXmCSTextGetPrevLine (line);

		  if ( prev_line )
		  {
/* Bug fix /Mak */
		        _validate_line ( widget, prev_line, NULL);
			num_out_line = prev_line->num_output_lines;
			out_line = &(prev_line->output_line[num_out_line-1]);

                        if (TextRtoL (widget))
                           segment->output_segment[i].x = 
                               RightMargin (widget)  -
				segment->output_segment[i].pixel_width;
                        else 
			segment->output_segment[i].x = 0;
			segment->output_segment[i].y =
			    out_line->y + out_line->pixel_height;
		  } else {
                        if (TextRtoL (widget))
                           segment->output_segment[i].x = 
                               RightMargin(widget) -
				segment->output_segment[i].pixel_width;
                        else 
		  	segment->output_segment[i].x = 0;
		  	segment->output_segment[i].y = 0;
		  }
		}
	      else
		{
		  int k, net_width;
		  DXmCSTextOutputData data = OutputData(widget);

		  net_width  =
			MAX(((int)TypicalWidth(widget)),NetWidth(widget));

		  /*
		   * previous source segment is there, so, we get the
		   * last output segment of the previous source segment
		   * and go next to him
		   */

		  k = p->num_output_segments - 1;

                  if (TextRtoL(widget))
                      segment->output_segment[i].x = p->output_segment[k].x - 
                                        segment->output_segment[i].pixel_width;
                  else
		  segment->output_segment[i].x = p->output_segment[k].x +
				((int) p->output_segment[k].pixel_width);

		  segment->output_segment[i].y = p->output_segment[k].y;

                  if (TextRtoL (widget))
                  {
		     if ( data->wordwrap && 
                          ((RightMargin (widget) - p->output_segment[k].x)
                                      > net_width
		           || segment->output_segment[i].new_line ) )
		     {
			/* start a new line for word wrap
			 */
                        accu_width = 0 ; 

                        /* Recalculate pixel width in case of TABS 
                        */
    	                if ( segment->output_segment[i].xlib_buff )
	                {
  	                   segment->output_segment[i].pixel_width  = 
                             _find_width (segment->output_segment[i].tab_width,
				    *segment->output_segment[i].font_entry,
                                    segment->output_segment[i].xlib_buff->text,
                                    segment->output_segment[i].char_count,
					          accu_width);
	                } 
                        else 
                        {
 	                   segment->output_segment[i].pixel_width = 0;
	                }

                        segment->output_segment[i].x = 
                              RightMargin (widget)  -
				segment->output_segment[i].pixel_width;
			segment->output_segment[i].y =
			      segment->output_segment[i].y +
				p->output_segment[k].pixel_height;
		     }
                  } 
                  else

		  if ( data->wordwrap &&
		       (segment->output_segment[i].x > net_width ||
			segment->output_segment[i].new_line) )
		  {
			/* start a new line for word wrap
			 */
			segment->output_segment[i].x = 0;
			segment->output_segment[i].y =
			    segment->output_segment[i].y +
				p->output_segment[k].pixel_height;
		  }
		}
	    }
	  else
	    {
	      /* start new line for all word wrap out_seg
	       */

              if (TextRtoL(widget))
              {
                 accu_width = 0 ; 
                 /* Recalculate pixel width in case of TABS 
                 */
    	         if ( segment->output_segment[i].xlib_buff )
	         {
  	                   segment->output_segment[i].pixel_width  = 
                             _find_width (segment->output_segment[i].tab_width,
				    *segment->output_segment[i].font_entry,
                                    segment->output_segment[i].xlib_buff->text,
                                    segment->output_segment[i].char_count,
					          accu_width);
	         } 
                 else 
                 {
 	                   segment->output_segment[i].pixel_width = 0;
	         }
                 segment->output_segment[i].x = RightMargin (widget)  -
					segment->output_segment[i].pixel_width;
              }
              else
	      segment->output_segment[i].x = 0;

	      segment->output_segment[i].y =
		  segment->output_segment[i-1].pixel_height +
		  segment->output_segment[i-1].y;
	    }
	}

	if ( segment->output_segment[i].need_recompute )
	{
	    TextSegment split_seg = NULL, s;

	    if (_check_wrap(widget, line, segment,
			    segment->output_segment[i].offset, &split_seg))
	    {
		 s = split_seg;

		 /* validate from the split segment up to the current segment
		  */
		 while ( s != NULL && s != segment )
		 {
		     _invalidate_segment (widget, line, s);
		     _compute_segment (widget, line, s);

		     s = _DXmCSTextGetNextSegment(s);
		 }

		 /* the last one
		  */

		 if ( s == segment && s != NULL )
		 {
			_invalidate_segment(widget, line, s);
			_compute_segment(widget, line, s);
		 }
	    }

	    segment->output_segment[i].need_recompute = False;
	}
    }
}

static void
/* Added widget to parameters - needed  for RtoL check */
_check_second_wrap ( widget, segment, out_seg, location, saved_location, 
                     accu_width,
		     net_width, second_split )
DXmCSTextWidget widget;
TextSegment segment;
DXmCSTextOutputSegment out_seg;
TextLocation location;
TextLocation saved_location;
int accu_width;
int net_width;
Boolean *second_split;
{

    TextSegment q, s;
    DXmCSTextOutputSegment split_seg, temp_out_seg;
    DXmCSTextPosition tmp_offset;
    Boolean ttl_width_formed;
    int count, i, total_width;

    if (!location->end_of_line)
    {
	s = location->segment;
	total_width = 0;
	*second_split = False;
	split_seg = (DXmCSTextOutputSegment) NULL;

	/* first find out_seg containing the break point
	 */

	count = 0;

	for ( i = 0; i < s->num_output_segments; i++ )
	{
	    split_seg = &(s->output_segment[i]);

            if (!split_seg->xlib_buff)
            {
              _validate_segment(widget, location->line, s);
            }

	    if ( count + split_seg->char_count > location->offset )
	    {
		/* sum up the char width from the split point to end of this
		 * out_seg
		 */
		tmp_offset = location->offset - count;

		total_width = _find_width(
				     split_seg->tab_width,
				     *split_seg->font_entry,
				     &(((char *)split_seg->xlib_buff->text)[tmp_offset]),
				     split_seg->char_count - tmp_offset,
				     accu_width );

		break;
	     }

	    count += split_seg->char_count;
	}

	/* now check if this split_seg is started at a new line already.
	 * if so, split the saved_location
	 */
	if ( split_seg &&
	     split_seg->offset == location->offset &&
             start_of_line(widget, split_seg ) )
	    {
		memcpy(location, saved_location, sizeof(TextLocationRec));

	    } else {

	        /* sum up the char width for the rest of this segment
	         * we can go on with the previous for loop
	         */

	        for (i += 1 ;
		     i < s->num_output_segments;
		     i++ )
	        {
		    temp_out_seg = &(s->output_segment[i]);

		    total_width += temp_out_seg->pixel_width;
	        }

	        /* Accumulate all char width from this position to the 'pos',
		 * which is the point where exceeds net_width
	         */

	        /* if s == segment, then we have total_width already
	         */
	        if ( s != segment )
	        {
		    /* accumulate all char width up to and including the
		     * pos' segment
		     */
		    ttl_width_formed = False;

		    for ( q = _DXmCSTextGetNextSegment(s);
		          q != NULL && !ttl_width_formed;
		          q = _DXmCSTextGetNextSegment(q) )
	            {
		        for ( i = 0; i < q->num_output_segments; i++ )
		        {
		            temp_out_seg = &(q->output_segment[i]);

		            total_width += temp_out_seg->pixel_width;

			    if (temp_out_seg == out_seg)
			    {
			        ttl_width_formed = True;
			        break;
			    }
		        }
	            }
	        }

	        /* okay, we need two wraps here.  Set the returned flag for
		 * loop back
	         */
	        if ( total_width > net_width )
		    *second_split = True;
	        else
		    *second_split = False;
	    }
	}
  }

static Boolean _check_wrapped_segment( widget, line, segment, offset,
				       wrapped_segment, second_split)
DXmCSTextWidget widget;
TextLine line;
TextSegment segment;
int  offset;
TextSegment *wrapped_segment;
Boolean *second_split;
{

  int net_width, count;
  int i, j, width, accu_width, total_width, c_width;
  TextLocationRec location, saved_location;
  DXmCSTextPosition pos;
  TextSegment first_segment, p;
  DXmCSTextOutputSegment out_seg;
  Boolean is_white_space = False;

#define SCAN_NEXT_TO_PREV 1

  net_width  = MAX(((int) TypicalWidth  (widget)),NetWidth (widget));

  first_segment = _DXmCSTextGetFirstSegment(line);

  i = segment->num_output_segments;

  if ( i <= 0 )
        return (False);

  *second_split = False;

  out_seg = &(segment->output_segment[i-1]);


  /* This will fix a bug in which the initial size of the widget is
   * too small to hold at least ONE character of this font, and therefore, 
   * the autowrap logic does not know how to handle this.  Consequently it 
   * loops endlessly trying to split this character into two lines.  So 
   * instead here we will make sure that the width of the widget is at 
   * least as large as the largest character in the segments font. 
   */
   {
   XFontStruct *font;
   XmFontType type_return = XmFONT_IS_FONT;
   XtPointer tmp_font;
   XFontSetExtents *fs_extents;
   unsigned long w;

   if (out_seg->font_entry == NULL)
   {
        /* Get the font entry and compute the appropriate tab width */
	_get_font_entry(widget, segment, out_seg);
   }
   tmp_font=
	XmFontListEntryGetFont(*out_seg->font_entry, &type_return);
   if (type_return == XmFONT_IS_FONT) 
       {
       font = (XFontStruct *)tmp_font;
       w = font->max_bounds.width;
       } 
   else 
       { /* XmFONT_IS_FONTSET */
       font = (XFontStruct *)tmp_font;
       fs_extents = XExtentsOfFontSet((XFontSet)font);
       w = (unsigned long)fs_extents->max_ink_extent.width;
       }

    if (w > net_width) net_width = w;
    }
   
  /* not exceeded, just return
   */
  if (TextRtoL(widget) )
  {
      if ((RightMargin(widget) - out_seg->x) <= net_width)
        return (False);
  }
  else

  if (out_seg->x + out_seg->pixel_width <= net_width)
	return (False);


  /* make sure out_seg is validated already
   */
  if (!out_seg->xlib_buff)
  {
     _compute_segment(widget, line, segment);
  }

  /* find out the position that is just exceed net_width
   */
  p = _DXmCSTextGetPrevSegment (segment);

  if ( p == (TextSegment) NULL )
  {
	accu_width = 0;
  } else {
	int k;

	k = p->num_output_segments - 1;

        if (TextRtoL (widget))
           accu_width = RightMargin (widget) - p->output_segment[k].x ;
        else
	accu_width = p->output_segment[k].x +
		     ((int) p->output_segment[k].pixel_width);
  }

  total_width = 0;
  c_width = 0;

  for ( j = 0; j < out_seg->char_count; j++ )
  {
	c_width  = _find_width( out_seg->tab_width,
				*out_seg->font_entry,
				&(((char *)out_seg->xlib_buff->text)[j]),
				1,
				accu_width + total_width );

	total_width += c_width;

	/* if length exceeds here, then j is the position we need
	 */
        if (TextRtoL (widget))
        {
           if (RightMargin (widget) - out_seg->x - out_seg->pixel_width
               + total_width > net_width)
           {   
               break ;
           }
        }
        else

	if (total_width + out_seg->x > net_width)
	{
		break;
	}
  }

  if ( CSTextLength(widget) == 0 )
  {
	/* case when nothing in the widget before this, possibly, paste
	 * Note that if using _DXmCSTextSourceGetPosition(), pos returned
	 * will be 0.
	 */
	pos = j + offset;

	location.end_of_line = 0;
	location.segment     = segment;
	location.line	     = line;
	location.position    = pos;
	location.offset	     = segment->length - 1;
  } else {

	/* pos containing the splitting position
	 */
	/* Note the last argument formerly used offset (the argument to
	** this function). out_seg->offset is the proper value to use -
	** the argument to _DXmCSTextSourceGetPosition is the offset within
	** the segment, and j represents the offset within the output
	** segment, therefore we must add the offset of the output segment
	** within the segment.
	*/
	pos = _DXmCSTextSourceGetPosition ( widget, line, segment, 
					    j + out_seg->offset );

	_DXmCSTextSourceLocate (widget, pos, &location);
  }

  /* force to the very last position for splitting
   */
  if ( location.end_of_line )
  {
	location.end_of_line = 0;
	location.line	     = line;
	location.segment     = segment;
	location.offset      = segment->length - 1;
  }

  /* if this location is a white space and is also the last position of the
   * out_seg, don't consider as wrapping. The wrapping position should be the
   * next position
   */

  if ( _DXmCSTextIsWhiteSpace( (Widget)widget, &location ) )
  {
        
	_DXmCSTextNavNextChar(&location);

	/* force to the very last position for splitting. Case when the very
	 * last position of text is a space, also a candidate for splitting
	 */
	if ( location.end_of_line )
	{
            /* Prevent infinite loop when tab is last character */
            if  (((char *)out_seg->xlib_buff->text)[j] == '\t')
                return(False);

	    location.end_of_line = 0;
	    location.line	 = line;
	    location.segment     = segment;
	    location.offset      = segment->length - 1;
	}

	is_white_space = True;

	memcpy(&saved_location, &location, sizeof(TextLocationRec));
  } else {
	memcpy(&saved_location, &location, sizeof(TextLocationRec));

        while ( !_DXmCSTextIsScanBreak( (Widget)widget, &location, SCAN_NEXT_TO_PREV, I18NTEXTWRAP) )
        {
	    if (!_DXmCSTextNavPrevChar(&location))
		break;

	    /* case when no scan break is found for the entire line, set
	     * the position at 'pos' to be the split point
	     */
	    if (location.end_of_line)
	    {
		memcpy(&location, &saved_location, sizeof(TextLocationRec));
		break;
	    }
        }
  }

  /* Is the break on a secondary path segment?
  */
  if (location.segment != (TextSegment)NULL &&
      location.segment->direction != CSTextPath( widget) )
  {
    	is_white_space = False;
        while ( _DXmCSTextNavPrevChar(&location) )
        {
  	      /* case when no scan break is found for the entire line, set
 	       * the position at 'pos' to be the split point
 	       */
	      if (location.end_of_line)
	      {
		    memcpy(&location, &saved_location, sizeof(TextLocationRec));
  		    break;
	      }

              if (location.segment->direction == CSTextPath( widget))
              {
                    /* if found, position to one character after
                    */
              	    _DXmCSTextNavNextChar(&location);
                    break ;
              }
        }
  }

  /* even though a break point is found, the total length from this
   * point to the pos may still be exceeding the net_width.  This 
   * condition is handled here
   */

  /* make sure location.segment validate already
   */
  _validate_segment(widget, line, location.segment);

  _check_second_wrap ( widget, segment, out_seg, &location, 
                       &saved_location, accu_width,
		       net_width, second_split );

  /* now location contain the splitting point.  find the corresponding segment
   * location.segment is the one.
   */

  p = location.segment;

  if ( p == NULL )
  {
	return (False);
  }

  if ( p == first_segment && location.offset == 0)
  {
       /* no blank in the whole line, split the last segment
	*/
       p = _DXmCSTextGetLastSegment(line);
       location.segment     = p;
       location.offset      = p->length - 1;
       location.end_of_line = p == NULL;
  }

  /* find which out_seg containing the splitting point
   */

  if (p == NULL)
  {
	return (False);
  }

  count = 0;

  for ( i = 0; i < p->num_output_segments; i++ )
  {
	out_seg = &(p->output_segment[i]);

	if ( count + out_seg->char_count > location.offset )
	{
		break;
	}

	count += out_seg->char_count;
  }

  /* now out_seg contain the splitting point.  change it's char_count field
   * and flush it's xlib_buff
   */

  *wrapped_segment = location.segment;

if (p->num_output_segments >  0) {

  if (out_seg->xlib_buff)  /* free up the xlib_buff */
  {
      if ((out_seg->xlib_buff)->text)
      {
          XtFree((out_seg->xlib_buff)->text);
          (out_seg->xlib_buff)->text = NULL;
      }

      XtFree((char *)out_seg->xlib_buff);
      out_seg->xlib_buff = NULL;

      out_seg->xlib_buff_count = 0;
  }

  if ( location.offset > 0 )
  {
    out_seg->char_count = location.offset - count;

    /* out_seg char_count will be changed, validate the per_char pointer
     * as well
     */
    if ( out_seg->per_char )
    {
        XtFree ((char *)out_seg->per_char);

        out_seg->per_char = (DXmCSTextOutputChar)
				XtMalloc (out_seg->char_count * 
			  		    sizeof (DXmCSTextOutputCharRec));

        for (j=0; j < out_seg->char_count; j++)
        {
	    out_seg->per_char[j].need_redisplay = TRUE;
	    out_seg->per_char[j].draw_mode      = XmHIGHLIGHT_NORMAL;
        }
    }

    (location.segment)->num_output_segments += 1;

    _append_out_segment( widget,
		         line,
		         location.segment,
		         (location.segment)->length -
			 		     count  - out_seg->char_count,
		         location.offset);
   } else {

    out_seg->new_line = True;
   }
}

  /* we have to create a new line at this point, although not necessarily
   * creating a new out_seg
   */
  /* for white space split, there is no new out_seg created.  the out_line
   * should not be split too
   */

   (location.line)->num_output_lines += 1;

   _append_out_line( widget,
		     line,
		     segment );

   if (is_white_space)
   {
	_compute_segment(widget, line, location.segment);
   }

   if (!is_white_space || *second_split)
   {
	return (True);
   } else {
	return (False);
   }

}


static Boolean _check_wrap( widget, line, segment, offset, split_segment)
DXmCSTextWidget widget;
TextLine line;
TextSegment segment;
int  offset;
TextSegment *split_segment;
{

  int i;
  TextSegment p, temp_seg, next_seg;
  static Boolean in_check_wrap = False;
  DXmCSTextOutputData data = OutputData (widget);
  Boolean second_split, wrapped, split_again;
  DXmCSTextOutputSegment out_seg;

  /* no word wrap or called from this routin, just return
   */
  if (in_check_wrap || !data->wordwrap)
     return (False);

  *split_segment = (TextSegment) NULL;

  in_check_wrap = True;

  wrapped = _check_wrapped_segment ( widget, line, segment, offset,
				     split_segment, &second_split );

  if (!wrapped)
  {
    in_check_wrap = False;
    return (False);
  }

  if ( second_split && *split_segment)
  {
    _compute_segment(widget, line, *split_segment);

    p = *split_segment;

    next_seg = _DXmCSTextGetNextSegment(segment);

    split_again = False;

    while (p != NULL)
    {
	out_seg = &(segment->output_segment[segment->num_output_segments-1]);

	wrapped = _check_wrapped_segment ( widget, line, p, out_seg->offset,
					   &temp_seg, &split_again );
	if (wrapped || split_again)
	{
            _invalidate_segment (widget, line, p);
	    _compute_segment(widget, line, p);
	    split_again = True;
	} else {
	    p = _DXmCSTextGetNextSegment(p);

	    if (p == next_seg || p == NULL)
		break;

	    _invalidate_segment (widget, line, p);
	    _compute_segment (widget, line, p);
	}
    }
  }

  in_check_wrap = False;

  return (True);
}


static void
_compute_line (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  TextLine prev_line;
  TextSegment saved_segment = NULL;
  DXmCSTextOutputLine p, q;
  DXmCSTextOutputSegment out_seg, next_out_seg;
  int i, k, l, saved_idx, char_count;
  Boolean found;

  for ( l = 0; l < line->num_output_lines; l++ )
  {
    p = &(line->output_line[l]);

    char_count = 0;

    if (p->need_recompute)
    {
      p->need_recompute = FALSE;

      p->x = 0;

      /* first find the y
       */

      /*
       * this line's y is the previous line's y + the previous line's
       * pixel height.  If previous output-line exists, the y of this output
       * line should be derived from it.  Otherwise, if this is the very 
       * first already, just set y to 0
       */

      prev_line = _DXmCSTextGetPrevLine (line);

      if (l > 0)
	q = &(line->output_line[l-1]);
      else
	q = NULL;

      if (q)
      {
	p->y = q->y + ((int) q->pixel_height);

      } else if (prev_line) {

        int num_out_line = prev_line->num_output_lines;

/* Bug fix /Mak */
	_validate_line(widget, prev_line, NULL);
	  
	p->y = prev_line->output_line[num_out_line-1].y + 
	       ((int) prev_line->output_line[num_out_line-1].pixel_height);

      } else {

	p->y = 0;	
      }


      /* with the y, we can locate which out_seg it is corresponding to and
       * then the width and height can be figured out
       */

      found = False;
      out_seg = (DXmCSTextOutputSegment) NULL;

      for ( segment  = _DXmCSTextGetFirstSegment(line);
	    segment != (TextSegment) NULL;
	    segment  = _DXmCSTextGetNextSegment(segment) )
      {
	  for ( i = segment->num_output_segments-1; i >=0; i-- )
	  {
		out_seg = &(segment->output_segment[i]);

		if ( p->y == out_seg->y && out_seg->char_count > 0 )
		{
			saved_idx = i;
			saved_segment = segment;
			found = True;
			break;
		}
	  }

	  if ( found )
	     break;
      }

      /* now out_seg contain the starting out_seg of this out_line.  We have
       * to find the last one having this same y.
       */

      i = saved_idx;
      found = False;

      for ( ;
	    segment != (TextSegment) NULL;
	    segment  = _DXmCSTextGetNextSegment(segment) )
      {
/* Bug fix /Mak */
	  if (!segment->output_segment)
	      _validate_segment(widget, line, segment);

	  for ( ; i >= 0; i-- )
	  {
		next_out_seg = &(segment->output_segment[i]);

		if ( p->y > next_out_seg->y && next_out_seg->char_count > 0 )
		{
		     found = True;
		     break;
		}

		/* save up the out_seg
		 */
		out_seg = next_out_seg;
	  }

	  if ( found )
		break;

	  i = 0;
      }

      if ( out_seg )
      {
	  /*
	   * this line's height is the last output segment's y + it's height
	   */

	  p->pixel_height = out_seg->pixel_height;

          if (TextRtoL(widget))
             p->pixel_width = RightMargin(widget) - out_seg->x;
          else
	  p->pixel_width  = out_seg->x + out_seg->pixel_width;

	  /* now find out_line->char_count
	   */

	  i = saved_idx;
	  char_count = 0;

	  /* starting from saved_segment and saved_idx, sum up all out_seg's
	   * char_count if the y matches
	   */
	  for ( segment  = saved_segment;
		segment != (TextSegment) NULL;
		segment  = _DXmCSTextGetNextSegment(segment) )
	  {
		for ( ; i < segment->num_output_segments; i ++ )
		{
		    out_seg = &(segment->output_segment[i]);

		    if ( p->y != out_seg->y )
			 break;

		    char_count += out_seg->char_count;
		}

		i = 0;
	  }

	  p->char_count   = char_count;
	} else {
	  /*
	   * no segments in this line, height is zero, width = 0;
	   */
	  p->pixel_width  = 0;
	  p->pixel_height = TypicalHeight (widget);
	  p->char_count   = 0;
	}
    }
  }
}


static void
_get_max_line_size (widget, width, height)
    DXmCSTextWidget widget;
    int *width, *height;
{
  TextLine line;
  DXmCSTextOutputLine out_line;
  int i;

  /*
   * now find the longest line and total height
   */
  *width = TypicalWidth (widget);
  *height = TypicalHeight (widget);
  
  for (line = _DXmCSTextGetFirstLine (widget);
       line != NULL;
       line = _DXmCSTextGetNextLine (line))
    {
/* Buf fix /Mak */
      if (!line->output_line)
          _validate_line(widget, line, NULL);

      for ( i = 0; i < line->num_output_lines; i++ )
      {
	  out_line = &(line->output_line[i]);
      
          *width  = MAX (*width,  ((int) out_line->pixel_width));
          *height = MAX (*height, out_line->y + ((int) out_line->pixel_height));
      }
    }
}


static void
_set_from_row_columns (widget, width, height)
    DXmCSTextWidget widget;
    int *width, *height;
{
  /* BOGUS have to make this an output private field which is set to
   * true/false based on creation time row/column being zero
   */
#define RowControls(widget)       (TRUE)
#define ColumnsControls(widget)   (TRUE)
  
  if (RowControls (widget))
    {
      *width = MAX (*width, 
		    ((int) CSTextCols (widget) * TypicalWidth (widget)));
    }
  
  if (ColumnsControls (widget))
    {
      *height = MAX (*height, 
		     ((int) CSTextRows (widget) * TypicalHeight (widget)));
    }
}



static void
_compute_output_size (widget, width, height, mode)
    DXmCSTextWidget widget;
    Dimension *width, *height;
    int mode;
{
  int net_width, net_height, w, h, w_rc = 0, h_rc = 0;
    
  net_width  = MAX (((int) TypicalWidth (widget)),  NetWidth (widget));
  net_height = MAX (((int) TypicalHeight (widget)), NetHeight (widget));

  switch (mode)
    {
      case _COMPUTE_GROW_SIZE:
        _get_max_line_size (widget, &w, &h);
	_set_from_row_columns (widget, &w, &h);
	
	w = MAX (w, net_width);
	h = MAX (h, net_height);

        if ( ! CanResizeWidth (widget))
          w = net_width;

        if ( ! CanResizeHeight (widget))
          h = net_height;
	break;

      case _COMPUTE_OPTIMAL_SIZE:
        _get_max_line_size (widget, &w, &h);
	_set_from_row_columns (widget, &w_rc, &h_rc);

	if (CanResizeWidth (widget))
	  w = MAX (w, w_rc);
	else
	  w = w_rc;

	if ( CanResizeHeight (widget))
	  h = MAX (h, h_rc);
	else
	  h = h_rc;
	break;

      default:
#ifdef DEBUG
	printf ("CSText output has unknown compute_size mode\n");
#endif
	w = net_width;
	h = net_height;
	break;
    }

  w = MAX (w, ((int) TypicalWidth (widget)));
  h = MAX (h, ((int) TypicalHeight (widget)));

  *width = w + (CSTextMarginWidth (widget) << 1);

  *height = h + (CSTextMarginHeight (widget) << 1);
}


static int
_find_first_visible_line (widget)
    DXmCSTextWidget widget;
{
  TextSegment segment;
  DXmCSTextOutputLine out_line;
  TextLine line;
  int i = 0, j;
  Position te = TopEdge (widget);

  for (line = _DXmCSTextGetFirstLine (widget);
       line != NULL;
       line = _DXmCSTextGetNextLine (line))
    {
	for ( j = 0; j < line->num_output_lines; j++ )
	{
	    out_line = &(line->output_line[j]);

	    i++;

	    if ((VerticalOffset (widget) + out_line->y) >= te)
	        return (i);
	}
    }

  return (i);
}

static int
_compute_num_output_lines (widget)
    DXmCSTextWidget widget;
{
  TextSegment segment;
  DXmCSTextOutputLine out_line;
  TextLine line;
  int i = 0, j;

  for (line = _DXmCSTextGetFirstLine (widget);
       line != NULL;
       line = _DXmCSTextGetNextLine (line))
    {
	i += line->num_output_lines;
    }

  return (i);
}


/*
 * allocate our data structs, each source segment may have an arbitrary
 * number of output segments due to word-wrapping, with nested directionality
 * this gets even worse, so in the source segment we keep an array of
 * output segments
 */

static void
_alloc_segment (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  int i, j;
  DXmCSTextOutputSegment seg_out;

  segment->output_segment = (DXmCSTextOutputSegment) 
    XtMalloc (segment->num_output_segments * 
	      sizeof (DXmCSTextOutputSegmentRec));

  for (i=0; i < segment->num_output_segments; i++)
    {
      seg_out = &(segment->output_segment[i]);

      seg_out->need_redisplay = TRUE;
      seg_out->need_recompute = TRUE;

      seg_out->pixel_width    = 0;
      seg_out->pixel_height   = 0;

      if (TextRtoL(widget))
          seg_out->x          = RightMargin (widget) ; 
      else 
      seg_out->x              = 0;
      seg_out->y              = 0;
      
      seg_out->offset         = 0;

      seg_out->xlib_buff      = NULL;   /* malloc inside I18n layer */
      seg_out->xlib_buff_count= 0;

      seg_out->font_entry     = NULL;   /* NULL means SearchFontList() needed */
      seg_out->tab_width      = 0;
      seg_out->new_line       = False;

      seg_out->draw_mode      = RmHIGHLIGHT_NORMAL;

      /* BOGUS, reaching into the segment record for length....
       */
      seg_out->char_count     = segment->length;

      seg_out->per_char = (DXmCSTextOutputChar)
	XtMalloc (seg_out->char_count * sizeof (DXmCSTextOutputCharRec));

      for (j=0; j < seg_out->char_count; j++)
	{
	  seg_out->per_char[j].need_redisplay = TRUE;
	  seg_out->per_char[j].draw_mode      = XmHIGHLIGHT_NORMAL;
	}
    }
}


static void
_append_out_segment (widget, line, segment, char_count, offset)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
    int char_count;
    int offset;
{
  int j;
  DXmCSTextOutputSegment seg_out;

  segment->output_segment = (DXmCSTextOutputSegment) 
    XtRealloc ((char *)segment->output_segment, segment->num_output_segments * 
	      				sizeof (DXmCSTextOutputSegmentRec));

      seg_out = &(segment->output_segment[segment->num_output_segments-1]);

      seg_out->need_redisplay = TRUE;
      seg_out->need_recompute = TRUE;

      seg_out->pixel_width    = 0;
      seg_out->pixel_height   = 0;

      if (TextRtoL(widget))
          seg_out->x          = RightMargin (widget) ; 
      else 
      seg_out->x              = 0;
      seg_out->y              = 0;
      
      seg_out->offset         = offset;

      seg_out->xlib_buff      = NULL;   /* malloc inside I18n layer */
      seg_out->xlib_buff_count= 0;

      seg_out->font_entry     = NULL;   /* NULL means SearchFontList() needed */
      seg_out->tab_width      = 0;
      seg_out->new_line	      = True;   /* new line needed */

      seg_out->draw_mode      = RmHIGHLIGHT_NORMAL;

      /* BOGUS, reaching into the segment record for length....
       */

      seg_out->char_count     = char_count;

      seg_out->per_char = (DXmCSTextOutputChar)
	XtMalloc (seg_out->char_count * sizeof (DXmCSTextOutputCharRec));

      for (j=0; j < seg_out->char_count; j++)
	{
	  seg_out->per_char[j].need_redisplay = TRUE;
	  seg_out->per_char[j].draw_mode      = XmHIGHLIGHT_NORMAL;
	}

}


static void
_alloc_line (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine line;
    TextSegment segment;
{
  DXmCSTextOutputLine line_out;

  line_out =
  line->output_line = (DXmCSTextOutputLine)
    XtMalloc (sizeof (DXmCSTextOutputLineRec));

  line_out->need_redisplay = TRUE;
  line_out->need_recompute = TRUE;
  line_out->pixel_width    = 0;
  line_out->pixel_height   = 0;
  line_out->x              = 0;
  line_out->y              = 0;
  line_out->char_count	   = 0;
  line_out->draw_mode      = RmHIGHLIGHT_NORMAL;
}

static void
_append_out_line (widget, line, segment)
    DXmCSTextWidget widget;
    TextLine  line;
    TextSegment segment;
{
  DXmCSTextOutputLine line_out, prev_out_line;

  line->output_line = (DXmCSTextOutputLine)
	XtRealloc ((char *)line->output_line, line->num_output_lines *
				      sizeof (DXmCSTextOutputLineRec) );

  line_out = &(line->output_line[line->num_output_lines - 1]);

  line_out->need_redisplay = TRUE;
  line_out->need_recompute = TRUE;
  line_out->pixel_width    = 0;
  line_out->pixel_height   = 0;
  line_out->x		   = 0;
  line_out->y		   = 0;
  line_out->char_count     = 0;
  line_out->draw_mode      = RmHIGHLIGHT_NORMAL;

}

/*
 * This routine reverses input string and returns pointer to the reversed one
 */
static char * 
_flip_string (string, len, text16)
    char *string;
    int len;
    Boolean text16;
{
    char *new_string;
    int  i;
    unsigned short *string16 = (unsigned short *) string;
    unsigned short *new_string16;


    if ( !text16)
     {
       new_string = XtMalloc(len);

       for(i=0;i < len; i++)
	 new_string[i] = string[len-1-i];

       return (new_string);
     }
    else
     {
       len = (len + 1) / 2;

       new_string16 = (unsigned short *) XtMalloc (len);

       for(i=0;i < len; i++)
	 new_string16[i] = string16[len-1-i];

       return((char *)new_string16);
     }
}







/* 
 * turn a physical position into a source position
 *
 * not too hard, look for a line that intersects the location,
 * then the source segment, then the output segment then the
 * character
 */

static Boolean
_line_intersection (widget, x, y, out_line)
    DXmCSTextWidget widget;
    Position x, y;
    DXmCSTextOutputLine out_line;
{
  int z = VerticalOffset (widget) + out_line->y;

  return ((y >= z) &&
	  (y < (z + ((int) out_line->pixel_height))));
}

static Boolean
_seg_intersection (widget, x, y, out_line, out_seg)
    DXmCSTextWidget widget;
    Position x, y;
    DXmCSTextOutputLine out_line;
    DXmCSTextOutputSegment out_seg;
{
  int z = SegToWindowX (widget, out_line, out_seg);

  if ( TextRtoL(widget) )
     /* allow intersection exactly on right edge */
     return ((x > z) &&
          (x <= (z + ((int) out_seg->pixel_width))));
  else 
  return ((x >= z) &&
	  (x <  (z + ((int) out_seg->pixel_width))));
}




static int _find_char_offset (widget, x, line, out_line, segment, out_seg)
    DXmCSTextWidget widget;
    Position x;
    TextLine line;
    DXmCSTextOutputLine out_line;
    TextSegment segment;
    DXmCSTextOutputSegment out_seg;
{
  int i, k;
  char c;
  int width = 0, c_width, num_byte, accu_width;
  int accu_char_count;
  DXmCSTextOutputSegment tmp_out_seg;

  /*
   * make segment relative
   */

  if (TextRtoL (widget))
     x = SegToWindowX (widget, out_line, out_seg) + out_seg->pixel_width - x ;
  else
  x = x - SegToWindowX (widget, out_line, out_seg);

  _compute_segment (widget, line, segment);

/*
 * BOGUS have to deal with rtol segments....
 */

  accu_char_count = 0;

  for ( i = 0; i < segment->num_output_segments; i++ )
  {
      tmp_out_seg = &(segment->output_segment[i]);

      if ( tmp_out_seg != out_seg )
      {
	  accu_char_count += tmp_out_seg->char_count;
      } else {
	  break;
      }
  }

  if (TextRtoL (widget))
     accu_width = RightMargin(widget) - out_seg->x - out_seg->pixel_width;
  else 
     accu_width = out_seg->x;

  if (FontListType(*out_seg->font_entry) == XmFONT_IS_FONT)
  {
     num_byte = (IsTwoByteFont ((XFontStruct *)
                                FontListFont(*out_seg->font_entry))) ?  2 : 1;

     width = 0;

     for (i=0, k=0; i<out_seg->char_count; i++, k += num_byte)
     {

	/*BOGUS should handle all xlib_buff */
        c_width = _find_width ( out_seg->tab_width, *out_seg->font_entry,
 			      &((char *)out_seg->xlib_buff->text)[k], 1,
			      width + accu_width );

        if (x < (width + (c_width >> 1)))
	   return (i + accu_char_count);

        width += c_width;
     }
  } else {
     width = 0;

     for (i=0, k=0; 
          i<out_seg->char_count; 
          i++, k += mblen(&((char*)out_seg->xlib_buff->text)[k], MB_CUR_MAX))
     {
	/*BOGUS should handle all xlib_buff */
        c_width = _find_width ( out_seg->tab_width, *out_seg->font_entry,
 			      &((char *)out_seg->xlib_buff->text)[k], 1,
			      width + accu_width );

        if (x < (width + (c_width >> 1)))
	   return (i + accu_char_count);

        width += c_width;
     }

  }; /* XmFONT_IS_FONTSET */
  /*
   * if all else fails return end of the segment
   */
  return (out_seg->char_count + accu_char_count - 1);
}


static DXmCSTextPosition 
XYToPos (widget, x, y)
    DXmCSTextWidget widget;  
    Position x, y;
{
    DXmCSTextOutputData data = OutputData (widget);
    TextLine line;
    TextSegment segment, saved_segment;
    DXmCSTextOutputLine out_line;
    DXmCSTextOutputSegment out_seg;
    int i, offset;
    int j, saved_index;
    Boolean out_seg_found, out_line_found;

    /* first find the out_line which intersects the y
     */
    out_line_found = False;

    for (line = _DXmCSTextGetFirstLine (widget);
	 line != NULL;
	 line = _DXmCSTextGetNextLine (line))
    {
	for ( j = 0; j < line->num_output_lines && !out_line_found; j++ )
	{
	    out_line = &(line->output_line[j]);

	    if (out_line && _line_intersection (widget, x, y, out_line))
	    {
		out_line_found = True;
	    }
	}

	if (out_line_found)
		break;
    }

    /* if out_line not found, x, y is not inside the displayable text
     */

    if (!out_line_found)
    {
	if ( y < TopEdge(widget) )
	    return ( (DXmCSTextPosition) 0 );
	else
	    return ( (DXmCSTextPosition) CSTextLength (widget) );
    }

    /* now we have out_line, find the out_seg
     */

    out_seg_found = False;

    segment = _DXmCSTextGetFirstSegment (line);

    for ( segment = _DXmCSTextGetFirstSegment(line);
	  segment != NULL;
	  segment = _DXmCSTextGetNextSegment(segment) )
    {
	for ( i = 0;
	      i < segment->num_output_segments && !out_seg_found;
	      i ++ )
	{
	    out_seg = &(segment->output_segment[i]);

	    if ( out_seg->y == out_line->y && out_seg->char_count > 0 )
	    {
		out_seg_found = True;
		break;
	    }
	}

	if (out_seg_found)
	    break;
    }

    saved_index = i;

    if (!segment)
    {
        /* case here is possible when the line contains the new line char
	 * only.  Therefore, out_seg->char_count == 0.  In this case, set
	 * position to beginning of the first (and only one) segment of the
	 * line
         */
	segment = _DXmCSTextGetFirstSegment(line);

	/* Note, _DXmCSTextSourceGetPosition() is fine even when segment
	 * == NULL
	 */
	return (_DXmCSTextSourceGetPosition (widget,
					     line,
					     segment,
					     0));
    }

    /* out_seg and out_line is found.  If x is on left of the line, just return
     * the out_seg's beginning position
     */
    if (TextRtoL (widget))
    {
       /* RtoL: If x is on right of the line, just return
        * the out_seg's beginning position
       */
       if (x > (RightMargin (widget) - out_line->x - HorizontalOffset (widget)))
       {
            return (_DXmCSTextSourceGetPosition (widget,
                                                 line,
                                                 segment,
                                                 out_seg->offset));
       }
    }
    else
    if (x < (HorizontalOffset (widget) + out_line->x))
    {
	/* in white to left of line */

	/* BOGUS
	 * do the ltor case
	 */

	if (segment)
	    return (_DXmCSTextSourceGetPosition (widget,
						 line,
						 segment,
						 out_seg->offset));
	else
	   /* BOGUS
	    * so, OK Vick, what is the position of this thing....
	    */
	    return ((DXmCSTextPosition) 0); /* will never reach here */
    }

    /* now we have the out_seg.  But we haven't compare the x intersection
     * yet.  here it is
     */

    out_seg_found = False;

    if (_seg_intersection(widget, x, y, out_line, out_seg))
	out_seg_found = True;
    else
    {
	/* for fall back to the closest position
	 */
	saved_segment = segment;

	for ( ;
	      segment != NULL;
	      segment = _DXmCSTextGetNextSegment(segment) )
	{
	    for ( i = saved_index;
		  i < segment->num_output_segments && !out_seg_found;
		  i ++ )
	    {
		if ( segment->output_segment[i].y > out_line->y )
		{
		    /* we've gone too far.  x must be intersecting with the
		     * white space of the out_seg (previous to the
		     * segment->output_segment[i].  So return the position
		     * of the out_seg's max length
		     */
		    
                    return (_DXmCSTextSourceGetPosition(widget,
							line,
							saved_segment,
							out_seg->offset +
							out_seg->char_count-1));
		}

		out_seg = &(segment->output_segment[i]);
		saved_segment = segment;

		if (_seg_intersection(widget, x, y, out_line, out_seg))
		     out_seg_found = True;
	    }

	    if ( out_seg_found )
		break;

	    /* for next segment, start from 0's out_seg.  So, reset index
	     */
	    saved_index = 0;
	}
    }

    /* if out_seg_found is False.  Closest position should be the max of out_seg
     * .  It can only be the last line.
     */

    if (!out_seg_found)
    {
	return (_DXmCSTextSourceGetPosition(widget,
					    line,
					    saved_segment,
					    out_seg->offset +
					    out_seg->char_count));
    } else {
	/* out_seg contain the position.  Find the exact position by finding
         * the char offset with respect to x
	 */
	offset = _find_char_offset (widget,
				    x,
				    line,
				    out_line,
				    segment,
				    out_seg);

	return (_DXmCSTextSourceGetPosition(widget,
					    line,
					    segment,
					    offset));
    }
}


static int 
_find_char_location (widget, line, out_line, segment, out_seg, offset)
    DXmCSTextWidget widget;
    DXmCSTextOutputLine out_line;
    TextLine line;
    TextSegment segment;
    DXmCSTextOutputSegment out_seg;
    DXmCSTextPosition offset;
{
  Position x;
  int i, k;
  Dimension width = 0;
  int num_byte;
  int accu_width;

  if (segment)
    {
      _compute_segment (widget, line, segment);

      if (TextRtoL (widget))
         accu_width = RightMargin(widget) - out_seg->x - out_seg->pixel_width ;
      else
      accu_width = out_seg->x;

      if (FontListType(*out_seg->font_entry) == XmFONT_IS_FONT)
      {
         /* for two byte char, measure char by 2 bytes */
         num_byte = 
                  (IsTwoByteFont ((XFontStruct *)
                                  FontListFont(*out_seg->font_entry))) ?  2 : 1;
         width = 0;

         for ( i=0, k=0; i < (offset - out_seg->offset); i++, k += num_byte)

	     /*BOGUS should handle all xlib_buff*/
	     width += _find_width ( out_seg->tab_width, *out_seg->font_entry,
				  &((char *)out_seg->xlib_buff->text)[k], 1,
				  width + accu_width );
      } else {
         width = 0;

         for ( i=0, k=0; 
               i < (offset - out_seg->offset); 
               i++, k += mblen(&((char *)out_seg->xlib_buff->text)[k], MB_CUR_MAX))

	     /*BOGUS should handle all xlib_buff*/
	     width += _find_width ( out_seg->tab_width, *out_seg->font_entry,
				  &((char *)out_seg->xlib_buff->text)[k], 1,
				  width + accu_width );


      } /* XmFONT_IS_FONTSET */
  }

  /*
   * make not segment relative
   */
  if (TextRtoL(widget) )
      width = (out_line ? out_line->x : 0) + 
              (
               out_seg  ? (out_seg->x + out_seg->pixel_width)
                        : (RightMargin( widget ) )
              )
              - width - HorizontalOffset (widget) ;
  else
  width += HorizontalOffset (widget) + 
           (out_line ? out_line->x : 0) + 
	   (out_seg  ? out_seg->x  : 0);

  return (width);
}




static void
_pos_to_xy (widget, position, x, y, x_visible, y_visible)
    DXmCSTextWidget widget;
    DXmCSTextPosition position;
    Position *x, *y;
    Boolean *x_visible, *y_visible;
{
    DXmCSTextOutputData data = OutputData (widget);
    DXmCSTextPosition p;
    TextLocationRec location;
    DXmCSTextOutputLine out_line = NULL;
    DXmCSTextOutputSegment out_seg = NULL;
    TextLine line;
    TextSegment segment;
    int offset;
    int i, j;

    _DXmCSTextSourceLocate (widget, position, &location);
    
    if (location.line == NULL)
      {
        if (TextRtoL ( widget )) 
           *x = -HorizontalOffset (widget);
        else
	*x = HorizontalOffset (widget);
	*y = VerticalOffset (widget);

	*x_visible = (*x >= LeftEdge (widget)) && (*x <= RightEdge  (widget));
	*y_visible = (*y >= TopEdge  (widget)) && (*y < BottomEdge (widget));

	return;
      }
    else
      {
	line = location.line;
/* Bug fix - need fresh info.  /Mak */
        _validate_line(widget, line, NULL);
	out_line = line->output_line;  /* init to first output_line */

	if (location.segment)
	  {
	    segment = location.segment;
	    offset  = location.offset;
	  }
	else
	  {
	    /*
	     * we are at the end of the source line, no segment avail
	     */
/* BOGUS
 * ltor only
 */
	    segment = _DXmCSTextGetLastSegment (line);

	    if (segment)
	      {
		offset  = segment->length;
		out_seg = segment->output_segment;

		if ( out_seg == (DXmCSTextOutputSegment) NULL )
		{
		    _invalidate_segment (widget, line, segment);
		    _compute_segment (widget, line, segment);
		}
	      }
	  }

	  /* find corresponding out_line, if not the same as line->out_line
	   */
	  if ( segment )
	  {
/* Bug fix - need fresh info /Mak */
	    _validate_segment(widget, line, segment);
	    for ( i = segment->num_output_segments - 1; i >= 0; i--)
	    {
	    	out_seg = &(segment->output_segment[i]);

		if ( out_seg->offset <= offset )
		{
		    for ( j = 0; j < line->num_output_lines; j++ )
		    {
			out_line = &(line->output_line[j]);

			if (out_line->y >= out_seg->y)
				break;
		    }
		    break;
	        }
	    }
	  }
      }

    if (out_line == NULL)
      {
	/* have to build it */
      }
    
    if (out_seg == NULL)
      {
	/* have to build it */
      }

    *x = _find_char_location (widget, 
			      line,
			      out_line, 
			      segment, 
			      out_seg, 
			      offset);

    if ( out_seg )
    {
	/* if out_seg not empty, the out_seg->y is enough to determine the
	 * y
	 */
        *y = VerticalOffset (widget) + 
	     (out_seg  ? out_seg->y  : 0);
    } else {
	/* else, this line is empty.  The y should be that of out_line->y
	 */
	*y = VerticalOffset (widget) +
	     (out_line ? out_line->y : 0);
    }

    *x_visible = (*x >= LeftEdge (widget)) && (*x <= RightEdge  (widget));
    *y_visible = (*y >= TopEdge  (widget)) && (*y < BottomEdge (widget));
}



static Boolean 
PosToXY (widget, position, x, y)
    DXmCSTextWidget widget;
    DXmCSTextPosition position;
    Position *x, *y;
{
  Boolean x_visible, y_visible;

  _pos_to_xy (widget, position, x, y, &x_visible, &y_visible);

  return (x_visible && y_visible);
}







static XtGeometryResult 
TryResize (widget, width, height, r_to_l)
    DXmCSTextWidget widget;
    Dimension width, height;
    Boolean r_to_l;
{
    DXmCSTextOutputData data = OutputData (widget);
    XtGeometryResult result;
    Dimension origwidth = InnerWidth (widget);
    Dimension origheight = InnerHeight (widget);
    XtWidgetGeometry request, reply;

    if ((width  == XtWidth (widget)) && 
	(height == XtHeight (widget)))
      return (XtGeometryNo);

    request.width        = width;
    request.height       = height;
    request.request_mode = CWWidth | CWHeight;

    /*
     * if we are RtoL layout then a width change means an X change too
     */
    if (r_to_l)
      {
	XmScrolledWindowWidget pw = (XmScrolledWindowWidget) XtParent (widget);
    	XtWidgetGeometry request2;

	request.x = InnerX (widget) +
		    InnerWidth (widget) - 
		    width;

	if (request.x < 0)
	    request.x = 0;

	/* if this is a scrolled cstext then a request to change x
	   is sent to the scrolled window.
	*/
  	if (XtClass (pw) == xmScrolledWindowWidgetClass  &&
	    pw->swindow.VisualPolicy == XmVARIABLE)
    	{
           request2.request_mode = CWX;
	   request2.x = pw->core.x + InnerWidth (widget) - width;
	   if (request2.x < 0)
	    	request2.x = 0;
    	   result = XtMakeGeometryRequest ((Widget)pw, &request2, &reply);
	   /* it's not so important to check result since at worst case 
	      the x remains stationary and we expand the width 
	   */
    	   request.request_mode = CWWidth | CWHeight;
    	}
        else    /* proceed normally */
            request.request_mode |= CWX;
      }

    result = XtMakeGeometryRequest (InnerWidget (widget), &request, &reply);

    if (result == XtGeometryAlmost) 
      {
	if (r_to_l &&
	    (reply.request_mode & CWX) && request.x != reply.x)
	  {
	    result = XtGeometryNo;
	  }
	else 
	  {
	    if (reply.request_mode & CWWidth) request.width = reply.width;
	    if (reply.request_mode & CWHeight) request.height = reply.height;

	    result = XtMakeGeometryRequest (InnerWidget (widget), 
					    &request, 
					    &reply);

	    if (result != XtGeometryYes)
		    result = XtGeometryNo;
	  }
    }
    return (result);
}








/*
 * mark as complete any cursors which overlap with this region which
 * we are clearing anyway...
 */
static void
_complete_cursors (widget, x, y, width, height)
    DXmCSTextWidget widget;
    Position x, y;
    Dimension width, height;
{
  CSTextCursor c;
  Position c_x, c_y, c_right, right, c_bot, bot;

  right = x + width;
  bot   = y + height;

  c = CursorList (widget);

  if (TextRtoL ( widget )) 
     c_x     = c->x + HorizontalOffset (widget);
  else
  c_x     = c->x - HorizontalOffset (widget);
  c_y     = c->y - VerticalOffset (widget);
  c_right = c->x + c->width;
  c_bot   = c->y + c->height;
	
  if ( ( ((c_x     >= x) && (c_x     <= right)) ||
	 ((c_right >= x) && (c_right <= right)) 
       ) &&
       ( ((c_y     >= y) && (c_y     <= bot)) ||
	 ((c_bot   >= y) && (c_bot   <= bot))))
    {
      _undraw_cursor (widget);
    }
}




#ifdef DEBUG
static int timer_count = 0;
#endif	  



static void
_draw_cursor (widget)
    DXmCSTextWidget widget;
{
  CSTextCursor cursor = CursorList (widget);

  if (cursor && ( ! cursor->on_display))
    {
      XCopyArea (XtDisplay (widget), 
		 cursor->pixmap,
		 InnerWindow (widget), 
		 XorGC (widget),
		 0, 0, 
		 cursor->width,
		 cursor->height,
                 (TextRtoL ( widget )) 
  		     ?  cursor->x - HorizontalOffset (widget)
                     :  cursor->x + HorizontalOffset (widget),
		 cursor->y + VerticalOffset (widget));

      cursor->on_display = TRUE;

      /*
       * start timer that will un-draw it and leave in cursor list
       */

      cursor->proc = (XtWidgetProc) _undraw_cursor;

      if (cursor->active && 
	  (cursor->blink_rate > 0) && 
	  (cursor->timer_id == (XtIntervalId) NULL))
	{
	  XtAppContext a = XtWidgetToApplicationContext((Widget)widget);

#ifdef DEBUG
timer_count++;
#endif	  
	  cursor->timer_id = XtAppAddTimeOut (a,
					      cursor->blink_rate, 
					      (XtTimerCallbackProc) CursorTimer, 
					      cursor);
	}
    }
}




static void
_undraw_cursor (widget)
    DXmCSTextWidget widget;
{
  CSTextCursor cursor = CursorList (widget);

  if (cursor == (CSTextCursor) NULL) 
    return;

  if (cursor->on_display)
    {
      cursor->on_display = FALSE;

      XCopyArea (XtDisplay (widget), 
		 cursor->pixmap,
		 InnerWindow (widget), 
		 XorGC (widget),
		 0, 0, 
		 cursor->width,
		 cursor->height,
                 (TextRtoL ( widget )) 
  		     ?  cursor->x - HorizontalOffset (widget)
                     : 
		 cursor->x + HorizontalOffset (widget),
		 cursor->y + VerticalOffset (widget));
    }

  cursor->proc = (XtWidgetProc) _draw_cursor;

  if (cursor->active && 
      (cursor->blink_rate > 0) && 
      (cursor->timer_id == (XtIntervalId) NULL))
    {
      XtAppContext a = XtWidgetToApplicationContext((Widget) widget);
	  
#ifdef DEBUG
timer_count++;
#endif	  
      cursor->timer_id = XtAppAddTimeOut (a,
					  cursor->blink_rate, 
					  (XtTimerCallbackProc) CursorTimer, 
					  cursor);
    }
}


static void
_stop_cursor (widget, do_undraw)
    DXmCSTextWidget widget;
    Boolean do_undraw;
{
  CSTextCursor cursor = CursorList (widget);

  if (cursor == (CSTextCursor) NULL)
    return;

  if (cursor->timer_id != (XtIntervalId) NULL)
    {
      XtRemoveTimeOut (cursor->timer_id);
      cursor->timer_id = (XtIntervalId) NULL;
#ifdef DEBUG
timer_count--;
#endif	  
    }

  cursor->active = FALSE;

  if (do_undraw)
    _undraw_cursor (widget);
}




static void
_create_cursor (widget, position)
    DXmCSTextWidget widget;
    DXmCSTextPosition position;
{
  CSTextCursor cursor = CursorList (widget);
  Position x, y;
  int i;

  if (( ! XtIsRealized (widget)) || ( ! CursorEnabled (widget)))
    return;

  if (cursor == NULL)
    {
      cursor = 
	CursorList (widget) = (CSTextCursor) 
	                      XtMalloc (sizeof (CSTextCursorRec));

      cursor->widget     = (Widget) widget;
      cursor->timer_id   = (XtIntervalId) NULL;
      cursor->active     = FALSE;
      cursor->on_display = FALSE;
    }

  _stop_cursor (widget, TRUE);

  if (( ! CSTextHasFocus (widget)) ||
      ( ! XtSensitive(widget)) ||
      ( ! PosToXY (widget, position, &x, &y)))
    return;

  cursor->active     = TRUE;
  cursor->blink_rate = BlinkRate (widget);
  cursor->position   = position;
  cursor->proc       = (XtWidgetProc) _undraw_cursor;
  cursor->on_display = FALSE;

  /*
   * curors are kept in line relative co-ords, not window
   */
  if (TextRtoL ( widget )) 
    cursor->x        = x + HorizontalOffset (widget) - 
                           ((cursor->width >> 1) - 1);
  else 
    cursor->x        = x - HorizontalOffset (widget) - 
                           ((cursor->width >> 1) - 1);
  cursor->y          = y - VerticalOffset (widget);
  cursor->height     = TypicalHeight (widget);
  cursor->width	     = (cursor->height <= 19) ? CURSOR_WIDTH : CURSOR_WIDTH + 1;
							/* jv */
  if (OutputData (widget)->bidirectional)
    {
      if (CSTextEditingPath (widget) == 0)
	cursor->pixmap = OutputData (widget)->l_to_r_pix;
      else
	cursor->pixmap = OutputData (widget)->r_to_l_pix;
    }
  else
    cursor->pixmap = OutputData (widget)->normal_pix;

  _draw_cursor (widget);
}






static void 
CursorTimer (closure, id)
   Opaque closure;
   XtIntervalId id;
{
  CSTextCursor cursor = (CSTextCursor) closure;
  DXmCSTextWidget widget = (DXmCSTextWidget) cursor->widget;

#ifdef DEBUG
timer_count--;
#endif	  

  cursor->timer_id = (XtIntervalId) NULL;
     
  if ( ! XtIsRealized (widget))
    {
      cursor->active = FALSE;
    }
  else
    (*cursor->proc) ((Widget)widget);
}





static void 
SetInsertionPoint(widget, position)
    DXmCSTextWidget widget;
    DXmCSTextPosition position;
{
  Position x, y;
  int z;
  Boolean x_visible, y_visible;
  DXmCSTextOutputData data = OutputData(widget);
  XPoint xmim_point;
  Arg args[6];
  Cardinal n = 0;

#ifdef ASIAN_IO_SHR
  AIMStruct aim_ptr = (AIMStruct)AIMRec(widget);
  if (aim_ptr->end_istate_allowed && aim_ptr->aim_widget) {
      DXmAIMCancelIState(aim_ptr->aim_widget);
  }
#endif

  _pos_to_xy (widget, position, &x, &y, &x_visible, &y_visible);

  if (AutoShowCursor (widget))
    {
      if ( (y < TopEdge  (widget)) ||
		(y + (int) TypicalHeight (widget) > BottomEdge (widget)) )
	{
	    int dest_pos = (BottomEdge(widget) - TopEdge(widget)) / 2;
	    dest_pos = dest_pos > TypicalHeight(widget) ? dest_pos :
		                                          TopEdge(widget);
	    z = dest_pos - y;

	  /*
	   * take care of the case where the arithmetic would decide the
	   * amount of scrolling is zero
	   */
	  if (z < 0)
	    {
	      if (-z < ((int) TypicalHeight (widget)))
		z = - ((int) TypicalHeight (widget));
	    }
	    else
	    {
              if (z < ((int) TypicalHeight (widget)))
                z = (int) TypicalHeight (widget);
	    }
 
	  z = z / ((int) TypicalHeight (widget));

	  DXmCSTextVerticalScroll (widget, z);
	}

      if ( ! x_visible && !data->wordwrap )
	{
	  z = (((RightEdge (widget) - LeftEdge (widget)) / 2) - x)
	       / ((int) TypicalWidth (widget));

	  DXmCSTextHorizontalScroll (widget, z);
	}
    }

    n = 0;
    XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
    XmImSetValues((Widget)widget, args, n);

  _create_cursor (widget, position);
}



/*
 * an output method to set a new type of cursor... don't know why
 */
static void
SetCursor (widget, cursor_type)
    DXmCSTextWidget widget;
    int cursor_type;
{
}











static void 
MakePositionVisible (widget, position)
  DXmCSTextWidget widget;
  DXmCSTextPosition position;
{
  Position x, y;
  int z;
  Boolean x_visible, y_visible;
  DXmCSTextOutputData data = OutputData(widget);

  if (position == -1)
      position = CursorPos(widget);

  _pos_to_xy (widget, position, &x, &y, &x_visible, &y_visible);

  if ( (y < TopEdge  (widget)) ||
      (y + (int) TypicalHeight (widget) > BottomEdge (widget)) )
  {
      int dest_pos = (BottomEdge(widget) - TopEdge(widget)) / 2;
      dest_pos = dest_pos > TypicalHeight(widget) ? dest_pos : TopEdge(widget);
      z = dest_pos - y;

      /*
       * take care of the case where the arithmetic would decide the
       * amount of scrolling is zero
       */
      if (z < 0)
      {
	  if (-z < ((int) TypicalHeight (widget)))
	      z = - ((int) TypicalHeight (widget));
      }
      else
      {
	  if (z < ((int) TypicalHeight (widget)))
	      z = (int) TypicalHeight (widget);
      }
 
      z = z / ((int) TypicalHeight (widget));

      DXmCSTextVerticalScroll (widget, z);
  }

  if ( ! x_visible && !data->wordwrap )
  {
      z = (((RightEdge (widget) - LeftEdge (widget)) / 2) - x)
	  / ((int) TypicalWidth (widget));

      DXmCSTextHorizontalScroll (widget, z);
  }
}    







static void Invalidate(widget, left, right)
   DXmCSTextWidget widget;
   DXmCSTextPosition left, right;
{
  /* BOGUS
   */
  _invalidate_all (widget);
}



/* 
 * idea is to make sure the things like rows, margin widths and overall
 * widths are consistent
 */
static void 
_make_stuff_consistent (widget)
     DXmCSTextWidget widget;
{
  int a = widget->primitive.shadow_thickness +
          widget->primitive.highlight_thickness;

  /* leave one pixel for insertion cursor to display
  */
  if( CSTextMarginWidth (widget) <= a ) CSTextMarginWidth (widget) = a;
  if( CSTextMarginHeight (widget) <= a ) CSTextMarginHeight (widget) = a;

  VerticalOffset   (widget) += TopEdge (widget)  - PrevTopEdge (widget);
  HorizontalOffset (widget) += LeftEdge (widget) - PrevLeftEdge (widget);

  PrevTopEdge (widget)  = TopEdge (widget);
  PrevLeftEdge (widget) = LeftEdge (widget);
}




static void 
LoadFontMetrics (widget)
   DXmCSTextWidget widget;
{
  extern XmStringCharSet DXmGetLocaleCharset();
  XmFontList fontlist = FontList (widget);
  unsigned long w;
  XmFontContext context;
  XmFontListEntry next_entry;
  XmFontType type_return = XmFONT_IS_FONT;
  char *font_tag = NULL, *def_charset;
  XtPointer tmp_font;
  XFontStruct *font;
  XFontSetExtents *fs_extents;
  int max_w, max_h, num_font;

  num_font               =
  max_h			 =
  max_w			 =
  MaxDescent (widget)    = 
  MaxAscent (widget)     = 
  TypicalWidth (widget)  = 
  TypicalHeight (widget) = 0;

  def_charset = (char *) DXmGetLocaleCharset();

  if (fontlist != NULL) 
    {
    XmFontListInitFontContext( &context, fontlist );

    do {
         next_entry = XmFontListNextEntry(context);
         if (next_entry) {
            font_tag = XmFontListEntryGetTag(next_entry);

	    if ( def_charset && font_tag  
                 && strcmp( font_tag, def_charset ) == 0 ) {

	       if ( CSTextDefaultCharSet(widget) != NULL ) {
		  CSTextDefaultCharSet(widget) = 
		    XtRealloc( CSTextDefaultCharSet(widget),
		  	       (strlen(font_tag) + 1) );
	       } else {
		 CSTextDefaultCharSet(widget) =
		   XtMalloc( strlen(font_tag) + 1 );
	       }

	       strcpy ((char *) CSTextDefaultCharSet(widget), font_tag);

	    } else if ( CSTextDefaultCharSet(widget) == NULL ) {

              CSTextDefaultCharSet(widget) = 
	        XtMalloc( strlen(font_tag) + 1 );

	      strcpy ((char *) CSTextDefaultCharSet(widget), font_tag);
	    }

	    num_font++;

            tmp_font = XmFontListEntryGetFont(next_entry, &type_return);
            if (type_return == XmFONT_IS_FONTSET) {
               fs_extents = XExtentsOfFontSet((XFontSet)tmp_font);
               w = (unsigned long)fs_extents->max_ink_extent.width;

               /* max_ink_extent.y is number of pixels from origin to top of
                * rectangle (i.e. y is negative) */
               MaxAscent (widget) = MAX (-fs_extents->max_ink_extent.y,
                                         MaxAscent (widget));
	       MaxDescent (widget) = MAX ( fs_extents->max_ink_extent.height +
                                             fs_extents->max_ink_extent.y,
                                           MaxDescent (widget));
            } else {
               font = (XFontStruct *) tmp_font;
	       MaxAscent  (widget) = MAX (font->max_bounds.ascent,  
                                          MaxAscent (widget));
	       MaxDescent (widget) = MAX (font->max_bounds.descent, 
                                          MaxDescent (widget));
	       w = 0;

	       if (( ! XGetFontProperty (font, XA_QUAD_WIDTH, &w)) || w == 0) {
	          if (font->per_char && 
	              font->min_char_or_byte2 <= '0' &&
		      font->max_char_or_byte2 >= '0')
		     w = font->per_char['0' - font->min_char_or_byte2].width;
	          else
		     w = font->max_bounds.width;
	       }

	       if (IsTwoByteFont (font))
	          w /= 2;
            } /* If XmFONT_IS_FONT */

	    if (w <= 0) w = 1;
	  
	    max_w = MAX (w, max_w);
	    max_h = MAX ((MaxAscent  (widget) + MaxDescent (widget)), max_h);

         } /* if next_entry */
       } while (next_entry != NULL);

       XmFontListFreeFontContext( context );

       if (num_font > 0) {
	  TypicalWidth (widget)  = max_w;
	  TypicalHeight (widget) = max_h;
       } else {
	  TypicalWidth (widget)  = 10;           /* gotta be something... */
	  TypicalHeight (widget) = 17;
	}
    }

/*
 * Comment out these lines - as of 1.2 DXmGetLocaleCharset returns a pointer 
 * directly to data stored in a locale cache rather than a copy of this data.
 * Freeing a charset returned by DXmGetLocaleCharset or I18nGetLocaleCharset 
 * will cause random memory corruption to occur.
 *
 *   if ( charset )
 *	XtFree( charset );
 *
 */

}




/* 
 * find an entry in the font list which matches, return the font
 */

static void
SearchFontList (widget, charset, entry)
    DXmCSTextWidget widget;
    XmStringCharSet 	charset;
    XmFontListEntry *entry;
{
    DXmCSTextCallbackStruct cb;
    XmFontList fontlist, old_fontlist;
    short indx;

    old_fontlist = fontlist = FontList(widget);

    /* As described above, this can be replaced by FontListSearch() once
       OSF no longer declares it static
    */
    _XmSearchFontList(fontlist, charset, TRUE, &indx, entry); 

    /* If the fontlist charset (fontlist tag) does not not exactly match the
       charset requested, let the NoFontCallback have a chance.
     */
/*
    if (strcmp (FontListCharset(&fontlist[indx]), charset) != 0) {
*/
    if (strcmp (((XmFontList)(&fontlist[indx]))->tag, charset) != 0) {
      cb.reason  = DXmCR_NOFONT;
      cb.event   = NULL;
      cb.charset = charset;
      cb.charset_len = strlen(charset);

      XtCallCallbackList ((Widget) widget, (XtCallbackList) NoFontCB (widget), 
			  (XtPointer) &cb);

      /* If the fontlist didn't change, use the font list entry which
         has already been set. Otherwise, use the new fontlist to
         set a possibly different font list entry
       */
      fontlist = FontList(widget);

      if (old_fontlist != fontlist)
         /* As described above, this can be replaced by FontListSearch() once
            OSF no longerdeclares it static
          */
         _XmSearchFontList(fontlist, charset, TRUE, &indx, *entry); 
    }
}



static void LoadGCs (widget)
     DXmCSTextWidget widget;
{
  DXmCSTextOutputData data = ((DXmCSTextOutput)widget->cstext.output)->data;
  unsigned long valuemask = (GCGraphicsExposures | GCFunction | 
			     GCForeground | GCBackground | GCClipMask);

  Pixel background = widget->core.background_pixel;
  XGCValues values;
  Pixel foreground;
  XmFontList fontlist;
  XmFontContext fcontext;
  XFontStruct *font;
  XmFontListEntry next_entry;
  XmFontType type_return = XmFONT_IS_FONT;
  XtPointer tmp_font;
  static XContext context = 0;
  Pixmap tw_cache_pixmap;
  Display *display = XtDisplay( (Widget) widget );
  Boolean use_fontset = False;

  if (context == (XContext) NULL)
	context = XUniqueContext();

  if (XFindContext(display, (Window) XtScreen(widget), context, 
		    				(XPointer*)&tw_cache_pixmap))
  {
      /* Get the Pixmap identifier that the X Toolkit uses to cache our */
      /* GC's.  We never actually use this Pixmap; just so long as it's */
      /* a unique identifier. */

      tw_cache_pixmap = XCreatePixmap(display, 
				      RootWindowOfScreen(XtScreen(widget)),
                                      1, 1, 1);
      XSaveContext(display, (Window) XtScreen(widget), context, (void*)tw_cache_pixmap);
  }

  foreground = widget->primitive.foreground;


  fontlist = FontList (widget);
  
  XmFontListInitFontContext( &fcontext, fontlist);
  next_entry = XmFontListNextEntry(fcontext);
  tmp_font = XmFontListEntryGetFont(next_entry, &type_return);
  if (type_return != XmFONT_IS_FONTSET) {
     font = (XFontStruct *)tmp_font;
     values.font = font->fid;
     valuemask |= GCFont;
     use_fontset = True;
  }
  XmFontListFreeFontContext( fcontext );

  values.graphics_exposures = (Bool) TRUE;
  values.foreground         = foreground;
  values.background         = background;
  values.function           = GXcopy;
  values.clip_mask          = tw_cache_pixmap;

  if ( NormalGC(widget) != NULL )
	XtReleaseGC((Widget) widget, NormalGC(widget) );

  if ( InverseGC(widget) != NULL )
	XtReleaseGC((Widget) widget, InverseGC(widget) );

  if ( ImageGC(widget) != NULL )
	XtReleaseGC( (Widget)widget, ImageGC(widget) );

  if ( XorGC(widget) != NULL )
	XtReleaseGC( (Widget)widget, XorGC(widget) );

  NormalGC (widget)         = XtGetGC((Widget) widget, valuemask, &values);

  values.foreground         = background;
  values.background         = foreground;
  InverseGC (widget)        = XtGetGC((Widget) widget, valuemask, &values);

  valuemask = (GCGraphicsExposures | GCFunction | 
		GCForeground | GCBackground | GCClipMask |
		GCFillStyle | GCCapStyle );
  if (!use_fontset) valuemask |= GCFont;

  values.foreground         = foreground ^ background;
  values.background         = 0;
  values.fill_style	    = FillTiled;
  values.cap_style	    = CapProjecting;
  ImageGC (widget)          = XtGetGC((Widget) widget, valuemask, &values);

  valuemask = (GCGraphicsExposures | GCFunction | 
	       GCForeground | GCBackground );
  if (!use_fontset) valuemask |= GCFont;

  values.clip_mask	    = None;
  values.function           = GXxor;
  values.graphics_exposures = (Bool) FALSE;
  XorGC (widget)            = XtGetGC((Widget) widget, valuemask, &values);
}


#ifdef NOT_YET
/*
 * Paint error characters if no font is available 
 */

static void
_paint_error_chars (widget, num_chars, relx, y, invgc)
    DXmCSTextWidget widget;
    int num_chars;
    int relx, y;
    GC invgc;
{
    DXmCSTextOutputData data = GetOutputData (widget);
    int width = num_chars * AvgCharWidth (widget),
    x = relxtox (widget, relx) - (IsLtoRTextPath (widget)) ? width : 0);
    
    XFillRectangle (XtDisplay(widget),
		    InnerWindow (widget), 
		    invgc,
		    x, 
		    y - MaxAscent (widget),
		    width,
		    LineHeight (widget));
    
    for (i=0 ; i < num_chars; i++ , x += AvgCharWidth (widget))
      XCopyArea (XtDisplay (widget), 
		 data->errchar,
		 InnerWindow (widget),
		 XorGC (widget),
		 0, 0, 
		 AvgCharWidth (widget), LineHeight (widget),
		 x, y - MaxAscent (widget));
}
#endif /* NOT_YET */


static void 
MakeCursors (widget)
    DXmCSTextWidget widget;
{
  static Screen *lastscreen = NULL;
  static Pixmap *cache = NULL;
  static Pixmap *ltorcache = NULL;
  static Pixmap *rtolcache = NULL;
  static int cachesize = 0;
  DXmCSTextOutputData data = ((DXmCSTextOutput)widget->cstext.output)->data;
  Screen *screen = XtScreen(widget);
  Display *dpy = XtDisplay(widget);
  GC fillGC;
  XSegment segments[3];
  int i, width, height, depth;
  int midpoint, linewidth;	/* jv */
  XtGCMask imagemask;		/* jv */
  XGCValues imagevalues;	/* jv */

  XRectangle ClipRect;

  if (!XtIsRealized((Widget) widget)) return;
  
  height = TypicalHeight (widget);
  if (height <= 19) {				/* jv */
    width = CURSOR_WIDTH;			/* jv */
    linewidth = 1;				/* jv */
  } else {					/* jv */
    width = CURSOR_WIDTH + 1;			/* jv */
    linewidth = 2;				/* jv */
  }						/* jv */
  midpoint = width/2;				/* jv */

  if (lastscreen == screen && height < cachesize && cache[height] != (Pixmap) NULL) 
    {
      NormalCursor (widget) = cache[height];
      LtoRCursor (widget) = ltorcache[height];
      RtoLCursor (widget) = rtolcache[height];
      return;
    }


  /* !!! BOGUS -- should otherwise figure out the depth; this code may not
     work on multiple screen displays. ||| %%% */

  depth = screen->root_depth;

  NormalCursor (widget) =  XCreatePixmap(dpy, InnerWindow (widget),
				width, height, depth);

  LtoRCursor (widget) =  XCreatePixmap(dpy, 
				       InnerWindow (widget),
				       width,
				       height,
				       screen->root_depth);

  RtoLCursor (widget) =  XCreatePixmap (dpy, 
					InnerWindow (widget),
					width,
					height,
					screen->root_depth);

  /*
   *  Have to set line width of ImageGC each time in case the
   *  cursor height changes.  ImageGC is only used for drawing
   *  cursors, so don't worry about restoring the original value.
   */  
  imagemask = GCLineWidth;
  imagevalues.line_width = linewidth;
  XChangeGC(dpy, ImageGC(widget), imagemask, &imagevalues);

  /* Fill the cursor with 0's, then draw 3 lines to make the correct shape. */
  
  fillGC = XCreateGC(dpy, LtoRCursor (widget), 0, (XGCValues *) NULL);
  
  XFillRectangle(dpy, NormalCursor (widget), fillGC, 0, 0,
		 width, height);
  
  segments[0].x1 = 0;				/* top horiz of the 'I' */
  segments[0].y1 = linewidth - 1;
  segments[0].x2 = width;
  segments[0].y2 = linewidth - 1;
  
  segments[1].x1 = 0;				/* bottom horiz of 'I' */
  segments[1].y1 = height - 1;
  segments[1].x2 = width;
  segments[1].y2 = height - 1;
  
  segments[2].x1 = midpoint;			/* vertical of 'I' */
  segments[2].y1 = linewidth;
  segments[2].x2 = midpoint;
  segments[2].y2 = height - 1;

  /* Set the clipping rectangle of the image GC from drawing */
  ClipRect.width = width;
  ClipRect.height = height;
  ClipRect.x = 0;
  ClipRect.y = 0;

  XSetClipRectangles(dpy, ImageGC(widget), 0, 0,
                       &ClipRect, 1, Unsorted);
  
  XDrawSegments(dpy, NormalCursor (widget), ImageGC (widget), segments, 3);
  
  
  /* Now repeat for the ltor "]" cursor (fix the segments...). */
  
  XFillRectangle(dpy, LtoRCursor (widget), fillGC, 0, 0,
		 width, height);
  
  segments[0].x1 = 0;
  segments[0].x2 = midpoint;
  
  segments[1].x1 = 0;
  segments[1].x2 = midpoint;
  
  XDrawSegments(dpy, LtoRCursor (widget), ImageGC (widget), segments, 3);
  
  
  /* Now repeat for the rtol "[" cursor (fix the segments...). */
  
  XFillRectangle(dpy, RtoLCursor (widget), fillGC, 0, 0,
		 width, height);
  
  segments[0].x1 = midpoint;
  segments[0].x2 = width;
  
  segments[1].x1 = midpoint;
  segments[1].x2 = width;
  
  XDrawSegments(dpy, RtoLCursor (widget), ImageGC (widget), segments, 3);
  
  XFreeGC(dpy, fillGC);

  if (lastscreen != screen) 
    {
      cachesize = 0;
      lastscreen = screen;
    }

  if (height >= cachesize) 
    {
      int hp = height + 1;

      cache = (Pixmap *) XtRealloc((char *) cache,
				   (unsigned) (sizeof(Pixmap) * hp));
      ltorcache = (Pixmap *) XtRealloc((char *) ltorcache,
				     (unsigned) (sizeof(Pixmap) * hp));
    rtolcache = (Pixmap *) XtRealloc((char *) rtolcache,
				     (unsigned) (sizeof(Pixmap) * hp));
    for (i=cachesize ; i<=height ; i++)
      cache[i] = ltorcache[i] = rtolcache[i] = (Pixmap) NULL;
    cachesize = height + 1;
  }
  cache[height] = NormalCursor (widget);
  ltorcache[height] = LtoRCursor (widget);
  rtolcache[height] = RtoLCursor (widget);

}







static void 
OutputGetValues (widget, args, num_args)
     DXmCSTextWidget widget;
     ArgList args;
     Cardinal num_args;
{
  XtGetSubvalues ((XtPointer) OutputData (widget),
		  i_str_output_resources, 
		  XtNumber (i_str_output_resources), 
		  args, num_args);
}


/* ckargs added in JRD */
static Boolean ckargs ( args, num_args, name )
ArgList args;
Cardinal num_args;
char * name;
{
    register ArgList arg;
    for (arg = args ; num_args != 0; num_args--, arg++) {
       if (strcmp(arg->name, name) == 0)
          return(TRUE);
    }
    return(FALSE);
}



static void 
OutputSetValues (widget, args, num_args)
   DXmCSTextWidget widget;
   ArgList args;
   Cardinal num_args;
{
  DXmCSTextOutputDataRec newdatarec;
  DXmCSTextOutputData old = ((DXmCSTextOutput)widget->cstext.output)->data;
  DXmCSTextOutputData new = &newdatarec;
  Boolean needgcs, newsize, newfont, newval, newim;
  Pixel background = widget->core.background_pixel;
  Boolean newdefcharset = FALSE;
  Dimension resize_width, resize_height, d;
  Boolean need_resize = FALSE;
  XPoint xmim_point;
  Arg im_args[6];
  int n = 0;
  CSTextCursor cursor;

  /*
   * default is to make the new the same as the old, hence only have to
   * diddle with fields we don't want the same
   */

  *new = *old;

/* BOGUS
 */
  XtSetSubvalues((XtPointer) new,
		 i_str_output_resources, 
		 XtNumber (i_str_output_resources), 
		 args, num_args);

  if (new->foreground != old->foreground) {
	XtSetArg(im_args[n], XmNforeground, new->foreground); n++;
  }
   
  if ( (old->font_list != new->font_list) && (new->font_list != NULL) )
    {
      need_resize = TRUE;

      XmFontListFree( (XmFontList) old->font_list);

      old->font_list = (XmFontList) XmFontListCopy (new->font_list);

      XtFree ( CSTextDefaultCharSet(widget) );

      CSTextDefaultCharSet(widget) = NULL;

      LoadFontMetrics (widget);

      if (XtIsRealized ((Widget) widget)) 
	{
	  MakeCursors (widget);
	}

      XtSetArg(im_args[n], XmNfontList, old->font_list); n++;
    }
    
    CursorEnabled(widget) = new->ip_visible;

    if (BlinkRate(widget) != new->blink_rate)
    {
	BlinkRate(widget) = new->blink_rate;
	cursor = CursorList (widget);
	if (cursor)
	    cursor->blink_rate = BlinkRate(widget);
	if  (BlinkRate(widget) == 0)
	{
	    /* Shut off cursor if blink rate went from non-zero to zero */
	    _stop_cursor (widget, TRUE);
	}
	else if (cursor && (cursor->timer_id == (XtIntervalId) 0))
	{
	    /* Turn on cursor if blink rate went from zero to non-zero */
	    _draw_cursor (widget);
	}
    }

    old->bidirectional = new->bidirectional;

    CanResizeWidth(widget) = new->resize_width;
    CanResizeHeight(widget) = new->resize_height;
    
    old->auto_show_ip = new->auto_show_ip;

/* If colums or rows is SetValued, set need_resize true.
 * If they are not SetValued but width or height is,
 * re-calculate columns and rows.
 * Added in JRD.
 */
  if ( ckargs ( args, num_args, XmNcolumns ) ||
			ckargs ( args, num_args, XmNrows ) ){
    need_resize = TRUE;
  } else {
    if ( ckargs (args, num_args, XmNwidth ) )
      CSTextCols (widget) = (int) ((XtWidth(widget)  - CSTextMarginWidth(widget) * 2)
				 / (int) TypicalWidth  (widget));
    if ( ckargs (args, num_args, XmNheight ) )
      CSTextRows (widget) = (int) ((XtHeight(widget) - CSTextMarginHeight(widget) * 2)
				 / (int) TypicalHeight (widget));
  }

  newim = False;
  if (old->input_method != new->input_method) {
      newim = True;
      if (old->input_method)
          XtFree(old->input_method);
      if (new->input_method) {
          old->input_method = XtMalloc(strlen(new->input_method) + 1);
          strcpy(old->input_method, new->input_method);
      } else
	  old->input_method = NULL;
  }

#ifdef ASIAN_IO_SHR
  if (old->input_method_type != new->input_method_type ||
      (new->input_method_type == DXmIM_STRING_TYPE && newim)) {
      AIMStruct aim_ptr;
      Arg al[5];
      register int ac;
      char *lang, *ext;

      old->input_method_type = new->input_method_type;
      aim_ptr = (AIMStruct)AIMRec(widget);
      get_im_name(widget, &lang, &ext);
      ac = 0;
      XtSetArg(al[ac], DXmNaimLanguage, lang);        ac++;
      XtSetArg(al[ac], DXmNaimUserDefinedIM, ext);     ac++;
      XtSetValues(aim_ptr->aim_widget, al, ac);
  }    
#endif /* ASIAN_IO_SHR */

  if ( old->wordwrap != new->wordwrap) {
      old->wordwrap = new->wordwrap;
      if (old->wordwrap) CanResizeWidth (widget) = FALSE;
      HorizontalOffset(widget) = PrevLeftEdge(widget);
      _invalidate_all(widget);
  }

  if (XtIsRealized ((Widget) widget)) 
    {
      XClearWindow (XtDisplay(widget), InnerWindow (widget));

      _dump_all (widget);
      _validate_all (widget);

      _undraw_cursor (widget);

      LoadGCs (widget);
    }

  _make_stuff_consistent (widget);

  if (need_resize)
     _compute_output_size (widget, 
			   &XtWidth (widget), 
			   &XtHeight (widget), 
		           _COMPUTE_OPTIMAL_SIZE);

    DXmCSTextPosToXY ((Widget) widget, DXmCSTextGetCursorPosition(widget),
                           &xmim_point.x, &xmim_point.y);
    XtSetArg(im_args[n], XmNbackground, widget->core.background_pixel);n++;
    XtSetArg(im_args[n], XmNbackgroundPixmap, widget->core.background_pixmap);n++;
    XtSetArg(im_args[n], XmNspotLocation, &xmim_point); n++;
    XtSetArg(im_args[n], XmNlineSpace, TypicalHeight(widget)); n++;
    XmImSetValues((Widget)widget, im_args, n);

}








/*ARGSUSED*/
static void 
HandleFocusEvents (w, closure, event)
    Widget w;
    Opaque closure;
    XEvent *event;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    XFocusChangeEvent *focus_event;
    XmAnyCallbackStruct cb;
    XPoint xmim_point;
    Arg  args[6];
    int n = 0;

    DXmCSTextPosToXY ((Widget) widget, DXmCSTextGetCursorPosition(widget),
                              &xmim_point.x, &xmim_point.y);

    switch (event->type) 
      {
        case FocusIn:
        case FocusOut:
	  focus_event = (XFocusChangeEvent*)event;

	  if (focus_event->detail == NotifyPointer  ||
	      focus_event->detail == NotifyVirtual  ||
	      focus_event->detail == NotifyNonlinearVirtual)
	    {
	      return;
	    }

	  /* fire focus callback here
	   */
	  if ( event->xfocus.send_event && !(CSTextHasFocus(widget)) )
	  {
		cb.reason  = XmCR_FOCUS;
		cb.event   = event;

		XtCallCallbackList ( (Widget) widget, 
					(XtCallbackList)FocusCB(widget),
					(XtPointer) &cb );
	  }

	  CSTextHasFocus (widget) = (focus_event->type == FocusIn);

	  if (CSTextHasFocus (widget))
	    {
	      /*
	       * got focus
	       */
	      _create_cursor (widget, CursorPos (widget));
	      n = 0;
              XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
              XmImSetFocusValues(w, args, n);
	    }
	  else
	    {
	      /*
	       * we've just lost focus, kill the cursors
	       */
	      _stop_cursor (widget, TRUE);
	      XmImUnsetFocus(w);
	    }
	  break;

	case EnterNotify:
	case LeaveNotify:
	  /*	newhasfocus = event->xcrossing.focus;   Doesn't quite work. */
	  if ((_XmGetFocusPolicy(w) != XmEXPLICIT) && 
	       event->xcrossing.focus &&
              (event->xcrossing.detail != NotifyInferior)) {
		if (focus_event->type == EnterNotify)
		{
		    n = 0;
		    XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
		    XmImSetFocusValues(w, args, n);
		}
		else /* LeaveNotify */
		{
		    XmImUnsetFocus(w);
		}
	  } 
	  break;
      }
}


static void 
Realize(widget, valueMask, attributes)
    DXmCSTextWidget widget;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
#ifdef ASIAN_IO_SHR
    AIMStruct aim_ptr;
#endif /* ASIAN_IO_SHR */

    XtCreateWindow ((Widget) widget, 
		    (unsigned int) InputOutput,
		    (Visual *) CopyFromParent, 
		    *valueMask, 
		    attributes);

    MakeCursors (widget);

#ifdef ASIAN_IO_SHR

    /* create aim widget
     */
    aim_ptr = (AIMStruct) AIMRec (widget);
    
/* Bug fix 
    aim_ptr->aim_widget = (Widget)_create_aim ( widget );
 */
    XtRealizeWidget(aim_ptr->aim_widget);

#endif /* ASIAN_IO_SHR */
}



static void 
Destroy (widget)
    DXmCSTextWidget widget;
{
#ifdef ASIAN_IO_SHR
    ICharStruct  i_char_tab;
    AIMStruct    aim_ptr;

    /* destroy aim if any
     */
    aim_ptr = (AIMStruct) AIMRec (widget);

    if (aim_ptr->aim_widget)
    {
	XtDestroyWidget(aim_ptr->aim_widget);
    }

    /* clean up intermediate char table
     */

    while ( aim_ptr->i_char_table != NULL )
    {
	if ( aim_ptr->i_char_table->next != NULL )
	{
	     i_char_tab = aim_ptr->i_char_table->next;
	} else {
	     i_char_tab = NULL;
	}

	XtFree (aim_ptr->i_char_table->s);
	XtFree (aim_ptr->i_char_table->charset);
	XtFree (aim_ptr->i_char_table);

	aim_ptr->i_char_table = i_char_tab;
    }

    XtFree (AIMRec(widget));

#endif /* ASIAN_IO_SHR */

    _stop_cursor (widget, FALSE);

    if (CursorList (widget))
      XtFree ((char *)CursorList (widget));

    XmFontListFree (FontList (widget));
    if (InputMethod(widget))
	XtFree((char *)InputMethod(widget));
    RendMgrFree((char *)RenditionMgr(widget));
    XtFree ((char *) ((DXmCSTextOutput) widget->cstext.output)->data);
    XtFree ((char *) widget->cstext.output);
}





static void 
RedrawRegion (widget, x, y, width, height)
    DXmCSTextWidget widget;
    int x, y, width, height;
{
}    







static void DoExpose(widget, event)
DXmCSTextWidget widget;
XEvent *event;
{
    XExposeEvent *xe = (XExposeEvent *) event;
    Arg im_args[1];
    XPoint xmim_point;
    int n = 0;

#ifdef ASIAN_IO_SHR
    ICharStruct  i_char_tab;
    AIMStruct    aim_ptr;
    unsigned long s;
    XFontStruct *font;
    XmFontListEntry entry;
#endif /* ASIAN_IO_SHR */

    if (event->xany.type != Expose)
	return;

    if ( ! DoingExpose (widget)) 
    {
	DoingExpose (widget) = TRUE;
	DXmCSTextDisableRedisplay(widget, FALSE);
    }

    _stop_cursor (widget, TRUE);
    
    if ((xe->x == 0) &&
	(xe->y == 0) &&
	(xe->width == XtWidth (widget)) &&
	(xe->height == XtHeight (widget)))
      {
/*
	if ( TopPos(widget) == 0 && BotPos(widget) == 0 )
	{
		DXmCSTextMarkRedraw(widget,
				    (DXmCSTextPosition) 0,
				    CSTextLength(widget));
	} else {

		DXmCSTextMarkRedraw(widget,
				    TopPos(widget),
				    BotPos(widget));
	}
*/
	RedisplayAll(widget) = True;
      }
    else
      if (TextRtoL (widget))
      DXmCSTextMarkRedraw (widget, 
			   XYToPos (widget, xe->x + xe->width, xe->y),
			   XYToPos (widget, xe->x, xe->y + xe->height));
      else
      DXmCSTextMarkRedraw (widget, 
			   XYToPos (widget, xe->x, xe->y),
			   XYToPos (widget, xe->x + xe->width, 
				    xe->y + xe->height));

    if (event->xexpose.count == 0) 
      {
	DXmCSTextEnableRedisplay(widget);
	DoingExpose (widget) = FALSE;
      }

#ifdef ASIAN_IO_SHR

    /* now, display the intermediate char, if any
     */
    aim_ptr = (AIMStruct) AIMRec (widget);

    i_char_tab = aim_ptr->i_char_table;

    while ( i_char_tab != NULL )
    {
	s = (unsigned long) i_char_tab->s;

    	SearchFontList ( widget, i_char_tab->charset, &entry );

        if (FontListType(entry) == XmFONT_IS_FONT) {
           font = (XFontStruct *)FontListFont(entry);
   	   XSetFont ( XtDisplay (widget), NormalGC(widget), font->fid );
	   XDrawImageString16 ( XtDisplay(widget),
			     XtWindow(widget),
			     NormalGC(widget),
			     i_char_tab->x,
			     i_char_tab->y + font->ascent,
			     s,
			     1 );
        } else { /* XmFONT_IS_FONTSET */
	   XmbDrawImageString ( XtDisplay(widget),
			      XtWindow(widget),
                              (XFontSet)FontListFont(entry),
			      NormalGC(widget),
			      i_char_tab->x,
			      i_char_tab->y + font->ascent,
			      s,
			      1 );
        }

        i_char_tab = i_char_tab->next;

    }

#endif /* ASIAN_IO_SHR */

/* This code is in XmText, may need to be turned on for CSText, which 
 * means adding an in_setvalues field to the CSText instance struture
 */

/* If the expose happened because of SetValues, the font may have changed.
 * At this point, RefigureLines has run and the widget is relayed out.
 * So it is safe to calculate the x,y position of the cursor to pass
 * to the IM.  And we can reset the clip origin so that the I-Beam will
 * be drawn correctly.
 */
/*     if (font_may_have_changed) {
 *	DrawInsertionPoint(widget, widget->text.cursor_position, off);
 *	_XmTextResetClipOrigin(widget, widget->text.cursor_position, False);
 *	DrawInsertionPoint(widget, widget->text.cursor_position, on);
 *	PosToXY(widget, widget->text.cursor_position, &xmim_point.x,
 *		&xmim_point.y);
 *	XtSetArg(im_args[n], XmNspotLocation, &xmim_point); n++;
 *	XmImSetValues(w, im_args, n);
 *   }
 */

    if (CSTextHasFocus (widget))
      _create_cursor (widget, CursorPos (widget));

    _XmTextDrawShadow (widget);
}


static void HandleGraphicsExposure(w, closure, event)
Widget w;
Opaque closure;
XEvent *event;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;

    if (event->xany.type == GraphicsExpose) 
      {
	XGraphicsExposeEvent *xe = (XGraphicsExposeEvent *) event;

	xe->x = 0;
	xe->width = XtWidth (widget);
	
	xe->y = 0;
	xe->height = XtHeight (widget);
	
	if ((xe->x == 0) &&
	    (xe->y == 0) &&
	    (xe->width == XtWidth (widget)) &&
	    (xe->height == XtHeight (widget)))
	  DXmCSTextMarkRedraw (widget, 0, CSTextLength (widget));
	else
/* Not sure if this is right - code never reaches here!
*/
	  if (TextRtoL (widget))
      	     DXmCSTextMarkRedraw (widget, 
			      XYToPos (widget, xe->x + xe->width, xe->y),
			      XYToPos (widget, xe->x, xe->y + xe->height));
          else
	  DXmCSTextMarkRedraw (widget, 
			      XYToPos (widget, xe->x, xe->y),
			      XYToPos (widget, xe->x + xe->width, 
				               xe->y + xe->height));

/* HUH
	if (xe->count == 0) {
	    if (data->exposehscroll) data->exposehscroll--;
	    if (data->exposevscroll) data->exposevscroll--;
	}
*/
    } 

/* HUH
    if (event->xany.type == NoExpose) 
      {
	if (data->exposehscroll) data->exposehscroll--;
	if (data->exposevscroll) data->exposevscroll--;
      }
*/
    _XmTextDrawShadow (widget);
}


static void RedisplayHBar(widget)
DXmCSTextWidget widget;
{
    if ( ! DoHScroll (widget)) return;
    
    /* re-calculate HBar */
    BarManagerNeedHBarUpdate(widget);
}


static void
_set_top_position( widget )
DXmCSTextWidget widget;
{
    TextSegment segment;
    DXmCSTextOutputLine out_line;
    TextLine line;
    DXmCSTextPosition i = 0;
    int j, saved_index;
    TextLine saved_line;
    Boolean top_found, bot_found;

    top_found = False;
    saved_line = _DXmCSTextGetFirstLine (widget);

    for( line = _DXmCSTextGetFirstLine (widget);
         line != NULL;
         line = _DXmCSTextGetNextLine (line))
    {
	for ( j = 0; j < line->num_output_lines; j++ )
	{
	    out_line = &(line->output_line[j]);

            if( out_line && _visible_line (widget, out_line) ) 
            {
		saved_index = j;
		saved_line  = line;
		top_found = True;
	        break;
            }

	    i += out_line->char_count;
	}

	if ( top_found )
	    break;

	i += 1; /* for the new line char */
    }

    TopPos( widget ) = i;

    /* now find BotPos(widget) */

    bot_found = False;

    for ( line = saved_line;
	  line != NULL;
	  line = _DXmCSTextGetNextLine (line) )
    {
	for ( j = saved_index; j < line->num_output_lines; j++ )
	{
	    out_line = &(line->output_line[j]);

	    if ( out_line && !_visible_line (widget, out_line) )
	    {
		bot_found = True;
		break;
	    }

	    i += out_line->char_count;
	}

	if ( bot_found )
	    break;

	saved_index = 0;
	i += 1;  /* new line char */
    }

    /* Set bottom position to the last visible line */
    if (i > TopPos(widget)) i--;

    BotPos(widget) = i;

    return;
}

    

/* Performance /Mak */
static void
_set_redisplay_line (widget, line, segment)
DXmCSTextWidget widget;
TextLine line;
TextSegment segment;
{
   DXmCSTextOutputLine out_line;
   int i;
       
   if ( ! line->output_line) {
       return;
   }
       
   for ( i = 0; i < line->num_output_lines; i ++ ) {
       out_line = &(line->output_line[i]);
       out_line->need_redisplay = TRUE;
   }

   for (segment  = _DXmCSTextGetFirstSegment(line);
	segment != (TextSegment) NULL;
	segment  = _DXmCSTextGetNextSegment (segment)) {
       int i, j;
       DXmCSTextOutputSegment seg;

       if ( ! segment->output_segment) {
	   break;
       } else {
	   for (i=0; i < segment->num_output_segments; i++) {
	       seg = &(segment->output_segment[i]);
	       
	       seg->need_redisplay = TRUE;
	       for (j=0; j < seg->char_count; j++) {
		   seg->per_char[j].need_redisplay = TRUE;
	       }
	   }
       }
   }
}    


static void
VerticalScroll (widget)
    DXmCSTextWidget widget;
{
  int vert_offset, usable_height, typical_height, net_height;
  int top_edge;
  int z;

  top_edge = TopEdge( widget );
  vert_offset = VerticalOffset( widget );
  typical_height = (int) TypicalHeight (widget);
  net_height = (int) NetHeight (widget);

  usable_height = (net_height/typical_height) * typical_height;

  z = usable_height + top_edge - (typical_height * LineCount (widget));
  if (z > top_edge)
    z = top_edge;

  _stop_cursor (widget, TRUE);                   /* before changing offset */

  vert_offset += (PendingVScroll (widget) * ((int) TypicalHeight (widget)));

  if (vert_offset > top_edge )
    {
      VerticalOffset (widget) = top_edge;
    }else{
      if (vert_offset < z)
      {
	VerticalOffset (widget) = z;
      }else{
        VerticalOffset( widget ) = vert_offset;
      }
    }
  PendingVScroll (widget) = 0;

/*BOGUS do we really need to dump all? try invalidate_all()
  _dump_all (widget);
*/
/* Performance /Mak
  _invalidate_all (widget);
  _validate_all (widget);

  _set_vbar (widget);
 */
  RedisplayAll(widget) = True;

  /* BOGUS
   * we shouldn't set vbar if this routine called from scrollbar callback 
   * itself.
   */
  BarManagerNeedVBarUpdate(widget);

  _set_top_position (widget);

}


static void
HorizontalScroll (widget)
    DXmCSTextWidget widget;
{
  int z, w, h;

  _stop_cursor (widget, TRUE);                   /* before changing offset */

  _get_max_line_size (widget, &w, &h);

  z = ((int) NetWidth (widget)) -
       w + LeftEdge (widget);

  if (TextRtoL ( widget ))  
  {

    HorizontalOffset (widget) -= PendingHScroll (widget) * 
                                 ((int) TypicalWidth (widget));
  }
  else 
  HorizontalOffset (widget) += PendingHScroll (widget) * 
                               ((int) TypicalWidth (widget));

  if (HorizontalOffset (widget) > LeftEdge (widget))
    {
      /* trying to position too far to right, so there's a gap on the left
       * stop this 
       */
      HorizontalOffset (widget) = LeftEdge (widget);
    }
  else
    {
      /* make sure we don't show beyond the right edge of the text 
       */
      if (HorizontalOffset (widget) < z)
	HorizontalOffset (widget) = (( z > LeftEdge (widget) ) ?
						LeftEdge (widget) : z );
    }

  PendingHScroll (widget) = 0;

/*BOGUS do we really need to dump all? try invalidate_all()
  _dump_all (widget);
*/
/* Performance /Mak
  _invalidate_all (widget);

  _validate_all (widget);

  _set_hbar (widget);
 */
  RedisplayAll(widget) = True;

  BarManagerNeedHBarUpdate(widget);
}





static void
_compute_hbar (widget, max, slider_position, slider_size, page_inc)
    DXmCSTextWidget widget;
    int *max, *slider_position, *slider_size, *page_inc;
{
  int max_columns, columns_per_page, max_slider_pos, w = 0, h = 0;;

  _get_max_line_size (widget, &w, &h);

  max_columns = w / ((int) TypicalWidth (widget));

  columns_per_page = NetWidth (widget) / ((int) TypicalWidth (widget));

  max_slider_pos   = max_columns - columns_per_page;

  max_slider_pos = MAX (max_slider_pos, 0);

  *slider_position = - ((HorizontalOffset (widget) - LeftEdge (widget)) /
                     ((int) TypicalWidth (widget)));

  Bound (0, *slider_position, max_slider_pos);

  *slider_size     = columns_per_page;
  *max             = (max_columns < columns_per_page) ? 
                     columns_per_page : max_columns;
  *page_inc        = columns_per_page;
}



static void
_compute_vbar (widget, max, slider_position, slider_size, page_inc)
    DXmCSTextWidget widget;
    int *max, *slider_position, *slider_size, *page_inc;
{
  int max_lines, lines_per_page, max_slider_pos;

  max_lines = LineCount (widget);

  lines_per_page = NetHeight (widget) / ((int) TypicalHeight (widget));

  max_slider_pos   = max_lines - lines_per_page;

  if (max_slider_pos < 0)
    max_slider_pos = 0;

  /* scroll bar is zero based, line count is 1 based */
  *slider_position = _find_first_visible_line (widget) - 1;

  Bound (0, *slider_position, max_slider_pos);

  *slider_size     = lines_per_page;
  *max             = (max_lines < lines_per_page) ? lines_per_page : max_lines;
  *page_inc        = lines_per_page;
}



static void
_set_scrollbar_stuff (widget, proc, scroll_bar)
    DXmCSTextWidget widget;
    DXmProc proc;
    Widget scroll_bar;
{
  int n = 0;
  Arg al[30];
  int max, slider_position, slider_size, page_inc, value;

  /*
   * calc values 
   */
  (*proc) (widget, &max, &slider_position, &slider_size, &page_inc);

  if (slider_size < 1) slider_size = 1;
  if (page_inc < 1) page_inc = 1;

  /* if horizontal scroll bar then check out direction */
  if (scroll_bar == HBar (widget))
  {
      XtSetArg(al[n], XmNprocessingDirection,  
                     (TextRtoL ( widget )) ? XmMAX_ON_LEFT : XmMAX_ON_RIGHT );
      n++;
  }

  XtSetArg (al[n], XmNvalue,           slider_position); n++;
  XtSetArg (al[n], XmNsliderSize,      slider_size);     n++;
  XtSetArg (al[n], XmNmaximum,         max);             n++;
  XtSetArg (al[n], XmNpageIncrement,   page_inc);        n++;
  
  XtSetValues (scroll_bar, al, n);
}


static void
_set_hbar (widget)
    DXmCSTextWidget widget;
{
  if (DoHScroll (widget))
    {
      _set_scrollbar_stuff (widget, _compute_hbar, HBar (widget));
    }
}

static void
_set_vbar (widget)
    DXmCSTextWidget widget;
{
  if (DoVScroll (widget))
    {
      _set_scrollbar_stuff (widget, _compute_vbar, VBar (widget));
    }
}



static void
ComputeLineIndex (widget, location, return_value)
    DXmCSTextWidget widget;
    TextLocation location;
    int *return_value;
{
    int char_count, i, index;
    DXmCSTextOutputLine out_line;
    TextLine line;

    char_count = 0;
    index = 0;

    /* first find out the line for the location
     */
    for ( line  = _DXmCSTextGetFirstLine(widget);
	  line != location->line && line != (TextLine) NULL;
	  line  = _DXmCSTextGetNextLine(line) )
    {
	index += line->num_output_lines;
    }

    if ( line == (TextLine) NULL )
    {
	*return_value = 0;
	return;
    }

    /* now line contains the location and index is the index accumulated
     * so far.  compare the line_offset to figure out the index
     */

    if ( location->end_of_line )
    {
	/* location is the end of this line, so index is max of this line
	 */
	index += line->num_output_lines;

	*return_value = index;

	return;
    } else {
	
	for ( i = 0; i < line->num_output_lines; i++ )
	{
	    out_line = &(line->output_line[i]);

	    char_count += out_line->char_count;
	    index += 1;

	    if (char_count >= location->line_offset)
	    {
		*return_value = index;
		return;
	    }
	}
    }
}


static void 
HandleVBar (w, param, cback)
     Widget w;
     Opaque param, cback;
{
    XmScrollBarCallbackStruct *info = (XmScrollBarCallbackStruct *) cback;
    DXmCSTextWidget widget = (DXmCSTextWidget) param;
    int i = 0;
    int vert_offset, top_edge;
    XPoint xmim_point;
    Arg args[1];

    if (IgnoreVBar(widget)) return;

    switch (info->reason) 
      {
        case XmCR_INCREMENT:
	  i = -1;
	  break;

	case XmCR_DECREMENT:
	  i = 1;
	  break;

	case XmCR_PAGE_INCREMENT:
	  i = - ((NetHeight (widget) / ((int) TypicalHeight (widget))) - 1);
	  break;

	case XmCR_PAGE_DECREMENT:
	  i = (NetHeight (widget) / ((int) TypicalHeight (widget))) - 1;
	  break;

        case XmCR_DRAG:
	case XmCR_VALUE_CHANGED:
	  vert_offset = VerticalOffset (widget);
	  top_edge    = TopEdge (widget);
	  i = - info->value 
	     - ((vert_offset - top_edge) / ((int) TypicalHeight (widget)));
	  break;

	case XmCR_TO_TOP  :
/* BOGUS */
	  i = info->pixel / ((int) TypicalHeight (widget));
	  break;

	case XmCR_TO_BOTTOM:
/* BOGUS */
	  i = (XtHeight (VBar (widget)) - info->pixel) / 
	      ((int) TypicalHeight (widget));
	  break;
    }

    DXmCSTextVerticalScroll (widget, i);

    _create_cursor (widget, CursorPos (widget));

    DXmCSTextPosToXY ((Widget) widget, DXmCSTextGetCursorPosition(widget),
                      &xmim_point.x, &xmim_point.y);
    XtSetArg(args[0], XmNspotLocation, &xmim_point);
    XmImSetValues(w, args, 1);
}


static void 
HandleHBar (w, param, cback)
    Widget w;
    Opaque param, cback;
{
    XmScrollBarCallbackStruct *info = (XmScrollBarCallbackStruct *) cback;
    DXmCSTextWidget widget = (DXmCSTextWidget) param;
    int i = 0;
    XPoint xmim_point;
    Arg args[1];

    if (IgnoreHBar(widget)) return;

    switch (info->reason) 
      {
        case XmCR_INCREMENT:
	  i = -1;
	  break;

	case XmCR_DECREMENT:
	  i = 1;
	  break;

	case XmCR_PAGE_INCREMENT:
	  i = - ((NetWidth (widget) / ((int) TypicalWidth (widget))) - 1);
	  break;

	case XmCR_PAGE_DECREMENT:
	  i = ((NetWidth (widget) / ((int) TypicalWidth (widget))) - 1);
	  break;

        case XmCR_DRAG:
	case XmCR_VALUE_CHANGED:
	  i = - info->value - 
	      ((HorizontalOffset (widget) - LeftEdge (widget)) / 
	        ((int) TypicalWidth (widget)));
	  break;

	case XmCR_TO_TOP:
	  i = info->pixel / ((int) TypicalWidth (widget));
	  break;

	case XmCR_TO_BOTTOM:
	  i = (XtWidth (HBar (widget)) - info->pixel) /
	      ((int) TypicalWidth (widget));
	  break;
      }

    if (TextRtoL (widget)) i = -i ;   /* scroll the other way */

    DXmCSTextHorizontalScroll (widget, i);

    _create_cursor (widget, CursorPos (widget));

    DXmCSTextPosToXY ((Widget) widget, DXmCSTextGetCursorPosition(widget),
                      &xmim_point.x, &xmim_point.y);
    XtSetArg(args[0], XmNspotLocation, &xmim_point);
    XmImSetValues(w, args, 1);
}





static void
Resize (widget)
    DXmCSTextWidget widget;
{
  int width, height;
  TextLine line;
  TextSegment segment;
  DXmCSTextOutputData data = OutputData(widget);
  XPoint xmim_point;
  Arg args[1];

  width  = (int) XtWidth (widget);
  height = (int) XtHeight (widget);

  CSTextCols (widget) = (int) ((width  - CSTextMarginWidth(widget) * 2)
				 / (int) TypicalWidth  (widget));
  CSTextRows (widget) = (int) ((height - CSTextMarginHeight(widget) * 2)
				 / (int) TypicalHeight (widget));

  line    = _DXmCSTextGetFirstLine (widget);

  if (line )
     segment = _DXmCSTextGetFirstSegment (line);
  else
     segment = (TextSegment) NULL;

  if (data->wordwrap)
  {
      _dump_rest_line(widget, line, segment);
      _validate_rest_line (widget, line, segment);
  } else {
      _invalidate_rest_line(widget, line, segment);
      _validate_rest_line (widget, line, segment);
  }

  LineCount (widget) = _compute_num_output_lines (widget);
  if (LineCount(widget) <= CSTextRows(widget)) {
      VerticalOffset(widget) = TopEdge(widget);
      _invalidate_all(widget);
  }
/* Performance /Mak
  _set_hbar (widget);
  _set_vbar (widget);
 */
  BarManagerNeedHBarUpdate(widget);
  BarManagerNeedVBarUpdate(widget);
  _set_top_position (widget);

  Draw (widget);
  _XmTextDrawShadow (widget);

    /* Somehow we need to let the input method know that the window has
     * changed size (for case of over-the-spot).  Try telling it that
     * the cursor position has changed and hopefully it will re-evaluate
     * the position/visibility/... of the pre-edit window.
     */

    DXmCSTextPosToXY ((Widget) widget, DXmCSTextGetCursorPosition(widget),
                      &xmim_point.x, &xmim_point.y);
    XtSetArg(args[0], XmNspotLocation, &xmim_point);
    XmImSetValues((Widget)widget, args, 1);

}





#define _set_args(w, cb) \
{\
 n = 0;\
 XtSetArg(al[n], XmNvalueChangedCallback,   (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNincrementCallback,      (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNdecrementCallback,      (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNpageIncrementCallback,  (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNpageDecrementCallback,  (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNtoTopCallback,          (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNtoBottomCallback,       (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNdragCallback,           (XtArgVal) cb); n++; \
 XtSetArg(al[n], XmNbackground,       w->core.background_pixel); n++;\
 XtSetArg(al[n], XmNbackgroundPixmap, w->core.background_pixmap); n++;\
 XtSetArg(al[n], XmNshadowThickness,  w->primitive.shadow_thickness); n++;\
 XtSetArg(al[n], XmNforeground,       w->primitive.foreground); n++;\
 XtSetArg(al[n], XmNtopShadowColor,   w->primitive.top_shadow_color); n++;\
 XtSetArg(al[n], XmNtopShadowPixmap,  w->primitive.top_shadow_pixmap); n++;\
 XtSetArg(al[n], XmNbottomShadowColor, w->primitive.bottom_shadow_color); n++;\
 XtSetArg(al[n], XmNbottomShadowPixmap, w->primitive.bottom_shadow_pixmap); n++;\
 XtSetArg(al[n], XmNminimum,            0); n++;\
 XtSetArg(al[n], XmNvalue,              0); n++;\
 XtSetArg(al[n], XmNtraversalOn,        False); n++;\
}



static void
_setup_scrollbars (widget)
DXmCSTextWidget widget;
{
  int n;
  XtCallbackRec hcallback[2]; 
  XtCallbackRec vcallback[2];
  Arg al[30];
  XmScrolledWindowWidget pw = (XmScrolledWindowWidget) XtParent (widget);
  DXmCSTextOutputData data = OutputData (widget);
  XmScrollBarWidget sbw;
  int j, k;
  int max, slider_pos, slider_size, page_inc, value;

  /* 
   * we use the callback closure as the easy way to find the text widget
   * associcated with the scroll bar
   */
  hcallback[0].closure  = (XtPointer)widget;
  hcallback[0].callback = (XtCallbackProc)HandleHBar;
  hcallback[1].closure  = (XtPointer)NULL;
  hcallback[1].callback = (XtCallbackProc)NULL;

  vcallback[0].closure  = (XtPointer)widget;
  vcallback[0].callback = (XtCallbackProc)HandleVBar;
  vcallback[1].closure  = (XtPointer)NULL;
  vcallback[1].callback = (XtCallbackProc)NULL;

  VBar (widget) = NULL;
  HBar (widget) = NULL;

  if (XtClass (pw) != xmScrolledWindowWidgetClass) 
    {
      DoHScroll (widget)       = FALSE;
      DoVScroll (widget)       = FALSE;
      return;
    }

  if (pw->swindow.VisualPolicy == XmCONSTANT)
    {
      DoHScroll (widget)       = FALSE;
      DoVScroll (widget)       = FALSE;

      CanResizeWidth (widget)  = TRUE;
      CanResizeHeight (widget) = TRUE;

      return;
    }

  if (pw->swindow.VisualPolicy != XmVARIABLE)
    return;

  if (DoHScroll (widget)) 
    {
      CanResizeWidth(widget) = FALSE;
      IgnoreHBar(widget)     = FALSE;

      /*
       * since we've shut down horizontal size changes based on internal
       * changes we better fix the size based on the number of columns
       * the user spec'd or the actual width spec'd.
       */

      if (XtWidth (widget) == 0)
	XtWidth (widget) = (CSTextCols (widget) * 
			    ((int) TypicalWidth (widget))) +
			   (CSTextMarginWidth (widget) << 1);

      _compute_hbar (widget, &max, &slider_pos, &slider_size, &page_inc);

      _set_args (widget, hcallback);

      XtSetArg(al[n], XmNorientation,      XmHORIZONTAL); n++;

      XtSetArg(al[n], XmNprocessingDirection,  
                     (TextRtoL ( widget )) ? XmMAX_ON_LEFT : XmMAX_ON_RIGHT );
      n++;
      XtSetArg(al[n], XmNmaximum,          max); n++;
      XtSetArg(al[n], XmNsliderSize,       slider_size); n++;
      XtSetArg(al[n], XmNpageIncrement,   page_inc);    n++;

/*
 * NOTE: this comment was lifted intact from Xm/TextOut.c.  Seems to be
 * applicable to cstext as well.  jv - 10-feb-1992
 *
 *   Highlight thickness shouldn't be passed onto scrollbars during create.     PIR2328
 *   This also caused a bug in which the addtional space required for the       PIR2328
 *   highlight was factored into the text widget's core.height .   This is      PIR2328
 *   a no no especially if the text widget is given an initial size in either   PIR2328
 *   row or column units.   The extra space was folded into this setting        PIR2328
 *   which made it larger than necessary.   The addtional space always turned   PIR2328
 *   out to be highlight_thickness * 4.                                         PIR2328
 *                                                                              PIR2328
 *   This may not be the right way to fix the bug, but this is such God         PIR2328
 *   awful code that it is impossible to determine where anything gets done     PIR2328
 *                                                                              PIR2328
 *         XtSetArg(arglist[n], XmNhighlightThickness,                          PIR2328
 *                     (XtArgVal) widget->primitive.highlight_thickness); n++;  PIR2328
 */
 /* PIR2328 */
    XtSetArg(al[n], XmNhighlightThickness, 0); n++;

      HBar(widget) = (Widget) XmCreateScrollBar ((Widget) pw, "hbar", al, n);

/* BOGUS */
      sbw = (XmScrollBarWidget) HBar(widget);
      sbw->primitive.highlight_thickness = 0;

      XtManageChild (HBar(widget));
    } 
  
  if (DoVScroll (widget)) 
    {
      CanResizeHeight(widget) = FALSE;
      IgnoreVBar(widget)      = FALSE;
            
      /*
       * since we've shut down vertical size changes based on internal
       * changes we better fix the size based on the number of rows
       * the user spec'd or the actual height spec'd.
       */

      if (XtHeight (widget) == 0)
	XtHeight (widget) = (CSTextRows (widget) * 
			     ((int) TypicalHeight (widget))) + 
			    (CSTextMarginHeight (widget) << 1);

      _compute_vbar (widget, &max, &slider_pos, &slider_size, &page_inc);

      _set_args (widget, vcallback);

      XtSetArg(al[n], XmNorientation,     XmVERTICAL);  n++;
      XtSetArg(al[n], XmNsliderSize,      slider_size); n++;
      XtSetArg(al[n], XmNmaximum,         max);         n++;
      XtSetArg(al[n], XmNpageIncrement,   page_inc);    n++;

      /* see comment about highlight thickness above - jv - 10-feb-1992 */
      XtSetArg(al[n], XmNhighlightThickness, 0); n++;

      VBar(widget) = (Widget) XmCreateScrollBar ((Widget) pw, "vbar", al, n);

/* BOGUS */
      sbw = (XmScrollBarWidget) VBar(widget);
      sbw->primitive.highlight_thickness = 0;

      XtManageChild (VBar(widget));
    } 
  
  if (VSCrollOnLeft (widget)) 
    {
      if (HSCrollOnTop (widget))
	  j = XmTOP_LEFT;
      else
	  j = XmBOTTOM_LEFT;
    } 
  else 
    {
      if (HSCrollOnTop (widget))
	  j = XmTOP_RIGHT;
      else
	  j = XmBOTTOM_RIGHT;
    }

  n = 0;
 
  XtSetArg (al[n], XmNhorizontalScrollBar, (XtArgVal) HBar(widget)); n++;
  XtSetArg (al[n], XmNverticalScrollBar,   (XtArgVal) VBar(widget)); n++;
  XtSetArg (al[n], XmNworkWindow,          (XtArgVal) widget);       n++; 
  XtSetArg (al[n], XmNscrollBarPlacement,             j);            n++;

  XtSetValues ((Widget)pw, al, n);  
} 


/* Performance /Mak */
static void _do_with_visible_line(w, proc, start_line)
DXmCSTextWidget w;
void (*proc)();
TextLine start_line;
{
    TextLine line;
    Boolean enter_visible_area = False;
    DXmCSTextOutputLine  out_line;
    int l;
    Boolean done = False;

    for (line = start_line ? start_line : _DXmCSTextGetFirstLine (w);
	 line != NULL && ! done;
	 line = _DXmCSTextGetNextLine (line)) {
	Boolean visible = False;
	if (!line->output_line)
	  _validate_line(w, line, NULL);
	for (l = 0; l < line->num_output_lines; l++) {
	    out_line = &(line->output_line[l]);
	    
	    if ((_visible_line (w, out_line))) {
		enter_visible_area = True;
		visible = True;
	    }
	    else {
		if (enter_visible_area) {
		    done = True;
		    break;
		}
	    }
	}
	if (visible)
	  (*proc)(w, line, NULL);
    }
}





void
#ifdef _NO_PROTO
#ifdef I18N_IO_SHR
#ifdef VMS
_DXmCSTextOutputCreate (widget, args, num_args)
#else
DXmCSTextOutputCreate (widget, args, num_args)
#endif /* VMS */
#else
DXmCSTextOutputCreate (widget, args, num_args)
#endif /* I18N_IO_SHR */

     DXmCSTextWidget widget;
     ArgList args;
     Cardinal num_args;
#else /* _NO_PROTO */
#ifdef I18N_IO_SHR
#ifdef VMS
_DXmCSTextOutputCreate (
#else
DXmCSTextOutputCreate (
#endif /* VMS */
#else
DXmCSTextOutputCreate (
#endif /* I18N_IO_SHR */
			DXmCSTextWidget widget,
			ArgList		args,
			Cardinal	num_args)
#endif /* _NO_PROTO */
{
  DXmCSTextOutput output;
  DXmCSTextOutputData data;
  Dimension width, height;
  int i;
#ifdef ASIAN_IO_SHR
  AIMStruct aim_ptr;
#endif /* ASIAN_IO_SHR */
  
  InnerWidget (widget) = (Widget)widget;


    output = 
      (DXmCSTextOutput) XtMalloc(sizeof(DXmCSTextOutputRec));
  
  Output( widget ) = output;

  output->data = 
    data = (DXmCSTextOutputData) XtMalloc (sizeof (DXmCSTextOutputDataRec));

  /* set default of vertical bar for RtoL layout */
  if (widget->primitive.dxm_layout_direction == DXmLAYOUT_LEFT_DOWN)
  {
      def_left_side = TRUE ;
  }
  else
  {
      def_left_side = FALSE ;
  }
  
  XtGetSubresources ((Widget) XtParent (widget),
		     (XtPointer) data,
		     widget->core.name, 
		     DXmSClassCSTextWidget, 
		     i_str_output_resources,
		     XtNumber(i_str_output_resources), 
		     args, num_args);
  
  data->widget = (DXmCSTextWidget) widget;

/* I18N START */
    if ( data->font_list == (XmFontList) NULL ){
	data->font_list =
		(XmFontList) DXmFontListCreateDefault (widget, NULL);
#ifdef DEBUG
	printf("I18n default font is set in CSText widget\n");
    } else {
	printf("Use default font in resource for CSText\n");
#endif
    }

    if (data->input_method) {
	char *tmp = XtMalloc(strlen(data->input_method) + 1);
	strcpy(tmp, data->input_method);
	data->input_method = tmp;
    }
/* I18N END */
  
  output->Draw                = (DXmDrawProc) Draw;
  output->SetInsertionPoint   = (DXmSetInsPointProc) SetInsertionPoint;
  output->SetCursor           = (DXmSetCurProc) SetCursor;
  output->MakePositionVisible = (DXmMakePosVisProc) MakePositionVisible;
  output->HandleData          = (DXmHandleDataProc) HandleData;
  output->XYToPos             = XYToPos;
  output->PosToXY             = PosToXY;
  output->Invalidate          = (DXmInvalidateProc) Invalidate;
  output->GetValues           = (DXmGetValuesProc) OutputGetValues;
  output->SetValues           = (DXmSetValuesProc) OutputSetValues;
  output->Realize             = (DXmRealizeProc) Realize;
  output->Destroy             = (DXmDestroyProc) Destroy;
  output->Resize              = (DXmResizeProc) Resize;
  output->Redisplay           = (DXmRedisplayProc) DoExpose;
  output->VerticalScroll      = (DXmVertScrollProc) VerticalScroll;
  output->HorizontalScroll    = (DXmHorScrollProc) HorizontalScroll;
  output->SetCharRedraw       = (DXmSetCharRedrawProc) SetCharRedraw;
/* PERFORMANCE /Mak
  output->SetCharDrawMode     = SetCharDrawMode;
 */
  output->SetCharDrawMode     = (DXmSetCharDrawModeProc) SetDrawMode;
  output->ComputeLineIndex    = (DXmComputeLineIndexProc) ComputeLineIndex;
  output->SearchFontList      = (DXmSearchFontListProc) SearchFontList;
  output->NumLinesOnText      = (DXmNumLinesOnTextProc) NumLinesOnText;
  output->RedisplayHBar       = (DXmRedisplayHBarProc) RedisplayHBar;

  /*DON'T conditionalize ChangeInputMethod
   */
  output->ChangeInputMethod   = (DXmChangeInputMethodProc) ChangeInputMethod;  

  output->ScanNextLine	      = (DXmScanNextLineProc) ScanNextLine;
  output->ScanPrevLine	      = (DXmScanPrevLineProc) ScanPrevLine;
  output->ScanStartOfLine     = (DXmScanStartOfLineProc) ScanStartOfLine;
  output->ScanEndOfLine	      = (DXmScanEndOfLineProc) ScanEndOfLine;

  output->ReadString	      = ReadString;


  /*
    data->hoffset        = 0;
    data->scrollwidth    = 1;
    */
  CursorList (widget)         = NULL;
  
  CSTextHasFocus (widget)     = FALSE;
  
  VBar (widget)               = NULL;
  HBar (widget)               = NULL;
  NormalGC (widget)           = NULL;
  DoingExpose (widget)        = FALSE;
  
  PrevTopEdge (widget)        = 
  PrevLeftEdge (widget)       = 0;
  
  LineCount(widget)	      = 0;

#ifdef ASIAN_IO_SHR

  aim_ptr         = (AIMStruct) XtMalloc ( sizeof (AIMStructRec) );
  AIMRec (widget) = (Opaque)aim_ptr;

/* Bug fix - create AIM now! */
  aim_ptr->aim_widget   = (Widget)_create_aim(widget);
  aim_ptr->i_char_table = NULL;
  aim_ptr->end_istate_allowed = True;

#endif /* ASIAN_IO_SHR */

  /*
   * do the font specific stuff
   */
  FontList (widget) = (XmFontList) XmFontListCopy (FontList (widget));
  LoadFontMetrics (widget);
  
  if (data->wordwrap) CanResizeWidth (widget) = FALSE;
  
  _setup_scrollbars (widget);
  
  LoadGCs (widget);
  
  XtAddEventHandler (InnerWidget (widget),
		     (EventMask)FocusChangeMask, 
		     FALSE, 
		     (XtEventHandler) HandleFocusEvents,
		     (Opaque) NULL);
  
  XtAddEventHandler (InnerWidget (widget),
		     (EventMask) 0, 
		     TRUE, 
		     (XtEventHandler) HandleGraphicsExposure,
		     (Opaque) NULL);
  
  VerticalOffset (widget)   =
  PrevTopEdge (widget)      = TopEdge (widget);

  HorizontalOffset (widget) =
  PrevLeftEdge (widget)     = LeftEdge (widget);

  RedisplayAll(widget) = True;
  BarManagerInit(widget);
  RenditionMgr(widget) = RendMgrInit(widget);

  _invalidate_all (widget);

  _make_stuff_consistent (widget);

  width  = XtWidth (widget);
  height = XtHeight (widget);
  _compute_output_size (widget, &width, &height, _COMPUTE_OPTIMAL_SIZE);
  
/* Use width and height if given.  Prevail over columns and rows. JRD */
  if ( XtWidth(widget) ){
    width = XtWidth(widget);
    CSTextCols (widget) = (int) ((width - CSTextMarginWidth(widget) * 2)
				 / (int) TypicalWidth  (widget));
  }
  if ( XtHeight(widget) ){
    height = XtHeight(widget);
    CSTextRows (widget) = (int) ((height - CSTextMarginHeight(widget) * 2)
				 / (int) TypicalHeight (widget));
  }

  TryResize (widget, width, height, FALSE);     /* No RtoL here! */
}

/*DON'T conditionalize ChangeInputmethod().  Just in case localized Input
 *      module exist but not Output module.  If ASIAN_IO_SHR is not defined,
 *      just return.
 */
static void ChangeInputMethod (widget, event)
DXmCSTextWidget widget;
XEvent *event;
{
#ifdef ASIAN_IO_SHR

   AIMStruct  aim_ptr;
   DXmAIMWidget aim;

   aim_ptr = (AIMStruct) AIMRec(widget);

   if (aim_ptr == NULL || aim_ptr->aim_widget == NULL)
	return;

   aim = (DXmAIMWidget) aim_ptr->aim_widget;

   DXmAIMPopupInputMethodList( aim, event );


#endif /* ASIAN_IO_SHR */
}

#ifdef ASIAN_IO_SHR

/* define any aim specific routines here
 */

/* check to see if the font exists before creating AIM
 */
static Boolean font_exist ( widget, charset )
DXmCSTextWidget widget;
char *charset;
{
    XmFontContext context;
    XmFontListEntry next_entry;
    char * font_tag;

    XmFontListInitFontContext ( &context, FontList (widget) );

    do {
        next_entry = XmFontListNextEntry(context);
        font_tag = XmFontListEntryGetTag(next_entry);

	if (strcmp (charset, font_tag) == 0 )
	{
	   XmFontListFreeFontContext (context);
	   return (True);
	}
    } while (next_entry != NULL);

    XmFontListFreeFontContext (context);

    return (False);
}


/* create the aim widget
 */
static DXmAIMWidget _create_aim ( widget )
Widget widget;
{

  Arg arglist[20];
  int ac;
  DXmAIMWidget w;
  char *lang, *ext;
  XmFontList fontlist = FontList (widget);

  static void _handle_aim_callback();

  static XtCallbackRec handle_aim_cb[] = {
      {(XtCallbackProc)_handle_aim_callback, NULL},
      {NULL, NULL}
  };
  handle_aim_cb[0].closure = (XtPointer)widget;

  get_im_name(widget, &lang, &ext);

  ac = 0;

  XtSetArg (arglist[ac], DXmNaimLanguage, lang);			ac++;
  XtSetArg (arglist[ac], DXmNaimUserDefinedIM, ext);			ac++;

  /* add callback resources here
   */
  XtSetArg (arglist[ac], DXmNaimPreEditStart, handle_aim_cb);		ac++;
  XtSetArg (arglist[ac], DXmNaimSetCursorPosition, handle_aim_cb);	ac++;
  XtSetArg (arglist[ac], DXmNaimGetCurrentCursorChar, handle_aim_cb);	ac++;
  XtSetArg (arglist[ac], DXmNaimSecPreEditStart, handle_aim_cb);	ac++;
  XtSetArg (arglist[ac], DXmNaimPreEditDraw, handle_aim_cb);		ac++;
  XtSetArg (arglist[ac], DXmNaimDrawIntermediateChar, handle_aim_cb);	ac++;
  XtSetArg (arglist[ac], DXmNaimQueryXYPosition, handle_aim_cb);	ac++;
  XtSetArg (arglist[ac], DXmNaimRimpDestroy, handle_aim_cb);		ac++;
  XtSetArg (arglist[ac], DXmNaimTextWidgetFontList, fontlist);		ac++;

  w = (DXmAIMWidget)DXmCreateAIM ( widget, "AIM", arglist, ac);
/* Bug fix - RealizeWidget will be done in realize proc. 
  XtRealizeWidget ( w );
 */

  return (w);
}


static void get_im_name(w, language, extension)
DXmCSTextWidget w;
char **language;
char **extension;
{
    static char lang[100], ext[100];
    int im_type;

    if (InputMethodType(w) != DXmIM_STRING_TYPE)
	im_type = InputMethodType(w);
    else {
	if (InputMethod(w) == NULL ||
	    strcmp(InputMethod(w), DXmIM_NONE) == 0)
	    im_type = DXmIM_NONE_TYPE;
	else if (strcmp(InputMethod(w), DXmIM_DEFAULT) == 0)
	    im_type = DXmIM_DEFAULT_TYPE;
        else
            im_type = DXmIM_STRING_TYPE;
    }

    if (im_type == DXmIM_NONE_TYPE) {
	*language  = NULL;
	*extension = NULL;
    } else if (im_type == DXmIM_DEFAULT_TYPE) {
	char *locale = (char *)xnl_getlanguage();
	XmFontList fontlist = FontList (w);
	Boolean proper_im = False;

	if ( !ASIAN_LANGUAGE( locale ) ) {
	    proper_im = False;
	}

	/* check if font exists
	 */

	if ( strcmp( locale, "ja_JP" ) == 0 ) {
	    if ( font_exist( w, "JISX0208.1983-0" ) ||
		 font_exist( w, "JISX0208.1983-1" ) )
		proper_im = True;
	} else if ( strcmp( locale, "zh_CN" ) == 0 ) {
	    if ( font_exist( w, "GB2312.1980-0" ) ||
		 font_exist( w, "GB2312.1980-1" ) )
		proper_im = True;
	} else if ( strcmp( locale, "zh_TW" ) == 0 ) {
	    if ( font_exist( w, "DEC.CNS11643.1986-2" ) ||
		 font_exist( w, "DEC.DTCS.1990-2" ) )
		proper_im = True;
	} else if ( strcmp( locale, "ko_KR" ) == 0 ) {
	    if ( font_exist( w, "KSC5601.1987-0" ) ||
		 font_exist( w, "KSC5601.1987-1" ) )
		proper_im = True;
	}  /* check Thai here */

	if (proper_im) {
	    strcpy(lang, locale);
	    *language = lang;
	} else {
	    *language = NULL;
	}
	*extension = NULL;
    } else {
	if (InputMethod(w)[2] == '_') {
	    strncpy(lang, InputMethod(w), 5);
	    strcpy(ext, &(InputMethod(w))[5]);
	    *language = lang;
	    *extension = ext;
        } else {
	    *language  = NULL;
	    *extension = NULL;
        }
    }
}


static void _pre_edit_start();
static void _set_cursor_position();
static void _get_current_cursor_char();
static void _sec_pre_edit_start();
static void _pre_edit_draw();
static void _draw_intermediate_char();
static void _query_xy_position();
static void _rimp_destroy();


static void _handle_aim_callback (w, tag, cb_data)
Widget w;
int    *tag;
XmAnyCallbackStruct *cb_data;
{
    DXmAIMCallbackStruct *data = (DXmAIMCallbackStruct *)cb_data;
    DXmCSTextWidget     widget = (DXmCSTextWidget) tag;
    DXmAIMWidget	aim    = (DXmAIMWidget) w;

    switch ( data->reason )
    {
	case DXmCRaimPreEditStart :

		_pre_edit_start ( widget, aim, cb_data );
		break;

	case DXmCRaimSetCursorPosition :

		_set_cursor_position ( widget, aim, cb_data );
		break;

	case DXmCRaimGetCurrentCursorChar :

		_get_current_cursor_char ( widget, aim, cb_data );
		break;

	case DXmCRaimSecPreEditStart :

		_sec_pre_edit_start ( widget, aim, cb_data );
		break;

	case DXmCRaimPreEditDraw :

		_pre_edit_draw ( widget, aim, cb_data );
		break;

	case DXmCRaimDrawIntermediateChar :

		_draw_intermediate_char ( widget, aim, cb_data );
		break;

	case DXmCRaimQueryXYPosition :

		_query_xy_position ( widget, aim, cb_data );
		break;

	case DXmCRaimRimpDestroy :

		_rimp_destroy ( widget, aim, cb_data );
		break;

	default :

		printf("_handle_aim_callback : Wrong callback reason /n");
		break;
    }
}


static void _pre_edit_start( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget    aim;
DXmAIMCallbackStruct *data;
{
    DXmCSTextPosition *posp = (DXmCSTextPosition *)
	XtMalloc(sizeof(DXmCSTextPosition));

    *posp = CursorPos(widget);
    data->base_position = (Opaque *)posp;
}

static void _set_cursor_position( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget    aim;
DXmAIMCallbackStruct *data;
{
    DXmCSTextPosition pos;
    AIMStruct aim_ptr = (AIMStruct)AIMRec(widget);

    pos = (DXmCSTextPosition)(*data->base_position + data->start_offset);
    aim_ptr->end_istate_allowed = False;
    DXmCSTextSetCursorPosition(widget, pos);
    aim_ptr->end_istate_allowed = True;
}

static void _get_current_cursor_char( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget	aim;
DXmAIMCallbackStruct *data;
{
    DXmCSTextPosition pos;
    XmString cs;

    pos = CursorPos ( widget );

    DXmCSTextRead( widget, pos, pos + 1, &cs );

    data->new_string = cs;
}

static void _sec_pre_edit_start( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget	aim;
DXmAIMCallbackStruct *data;
{
    DXmCSTextPosition left, right;
    XmString string;
    AIMStruct aim_ptr = (AIMStruct)AIMRec(widget);

    if ( DXmCSTextGetSelectionInfo( widget, &left, &right ) )
    {
        string = DXmCSTextGetSelection( widget );
        data->base_position  = (Opaque *)
	    XtMalloc( sizeof( DXmCSTextPosition ) );
        *data->base_position = (Opaque)left;
        data->start_offset   = 0;
        data->end_offset     = right - left;
        data->new_string     = string;
    } else {

        data->base_position = NULL;
        data->start_offset  = data->end_offset = 0;
        data->new_string    = NULL;
    }
}

static void _pre_edit_draw( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget    aim;
DXmAIMCallbackStruct *data;
{
    DXmCSTextPosition base_pos, cursor_position;
    ICharStruct       temp_i_char_tab;
    AIMStruct         aim_ptr;

    /* At this point, the intermediate char is no longer needed.  Free
     * memory
     */

    aim_ptr = (AIMStruct) AIMRec (widget);

    while ( aim_ptr->i_char_table != NULL )
    {
	if ( aim_ptr->i_char_table->next != NULL )
	{
	     temp_i_char_tab = aim_ptr->i_char_table->next;
	} else {
	     temp_i_char_tab = NULL;
	}

	XtFree (aim_ptr->i_char_table->s);
	XtFree (aim_ptr->i_char_table->charset);
	XtFree (aim_ptr->i_char_table);

	aim_ptr->i_char_table = temp_i_char_tab;
    }

    DXmCSTextDisableRedisplay( widget, FALSE );

    /* In case the operation is insertion to the current cursor position,
     * attempt pending delete
     */
    if (data->start_offset == data->end_offset) {
	cursor_position = CursorPos( widget );
	if (data->base_position == NULL && data->start_offset == 0) {
	    if (NeedsPendingDelete(widget, cursor_position)) {
		aim_ptr->end_istate_allowed = False;
		DeleteCurrentSelection( widget,
					(XEvent *) NULL,
					(char **)  NULL,
					(Cardinal) 0 );
		aim_ptr->end_istate_allowed = True;
	    }
	} else if (cursor_position == (DXmCSTextPosition)
		      (*data->base_position + data->start_offset)) {
	    if (NeedsPendingDelete(widget, cursor_position)) {

		aim_ptr->end_istate_allowed = False;
		DeleteCurrentSelection( widget,
					(XEvent *) NULL,
					(char **)  NULL,
					(Cardinal) 0 );
		aim_ptr->end_istate_allowed = True;
		*data->base_position = (Opaque)
		    CursorPos( widget ) - data->start_offset;
	    }
	}
    }

    if (data->base_position)
	base_pos = (DXmCSTextPosition)*data->base_position;
    else
	base_pos = CursorPos(widget);

    aim_ptr->end_istate_allowed = False;
    if (data->start_offset == data->end_offset &&
	data->num_rendition == 1) {
	DXmCSTextInsertChar(widget, base_pos + data->start_offset,
			    data->new_string);
    } else {
        DXmCSTextDisableRedisplay ( widget, True );

        _DXmCSTextSourceReplaceString(widget,
		                      base_pos + data->start_offset,
		                      base_pos + data->end_offset,
		                      (XmString) data->new_string);

        DXmCSTextEnableRedisplay ( widget );
    }
    aim_ptr->end_istate_allowed = True;

    if (data->num_rendition) {
        enum {Start, Reverse, Other} state = Start;
	DXmCSTextPosition start, end;
	register int i;

	base_pos += data->start_offset;

	for (i = 0; i < data->num_rendition; i++) {
	    switch (state) {
	      case Start :
		  if (data->renditions[i] == AIMReverse) {
		      state = Reverse;
		      start = i;
		  } else {
		      state = Other;
		      start = i;
		  }
		  break;
	      case Reverse :
		  if (data->renditions[i] != AIMReverse) {
		      end = i;
		      DXmCSTextSetHighlight(widget,
					    base_pos + start,
					    base_pos + end,
					    XmHIGHLIGHT_SELECTED);
		      state = Other;
		      start = i;
		  }
		  break;
	      case Other :
		  if (data->renditions[i] == AIMReverse) {
		      end = i;
		      DXmCSTextSetHighlight(widget,
					    base_pos + start,
					    base_pos + end,
					    XmHIGHLIGHT_NORMAL);
		      state = Reverse;
		      start = i;
		  }
		  break;
	    }
	}
	if (state == Reverse) {
	    end = i;
	    DXmCSTextSetHighlight(widget,
				  base_pos + start,
				  base_pos + end,
				  XmHIGHLIGHT_SELECTED);
	} else {
	    end = i;
	    DXmCSTextSetHighlight(widget,
				  base_pos + start,
				  base_pos + end,
				  XmHIGHLIGHT_NORMAL);
	}
    }

    DXmCSTextEnableRedisplay(widget);
}

static void _draw_intermediate_char( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget	aim;
DXmAIMCallbackStruct *data;
{
    XmString          cs;
    XmStringContext   context;
    char              *text;
    XmStringCharSet   charset;
    XmStringDirection direction;
    Boolean	      seperator;
    ICharStruct	      i_char_tab;

    DXmCSTextPosition pos;
    Position x, y;
    unsigned long s;
    XFontStruct *font;
    AIMStruct  aim_ptr;
    XmFontListEntry entry;

    cs = (XmString) data->new_string;

    aim_ptr = (AIMStruct) AIMRec (widget);

    if ( data->start_offset == 0 && data->end_offset == 0 )
    {
	/* this is the first intermediate char */
    } else {
	/* replace the original intermediate char with this one */
    }


    /* check to see if cs == NULL.  in that case, THW should clear the
     * intermediate char that has been displayed.
     */
    i_char_tab = aim_ptr->i_char_table;

    if ( cs == NULL && i_char_tab != NULL )
    {
	/* free up the intermediate char table and generate an expose
	 * event on THW.  Here I just mark redraw...
	 * aim_ptr->i_char_table should be free and set NULL as well
	 */

	int x, y, width, height;
	DXmCSTextPosition first_pos, last_pos;

	x = i_char_tab->x;
	y = i_char_tab->y;
	width  = i_char_tab->width;
	height = i_char_tab->height;

	DXmCSTextDisableRedisplay(widget, FALSE);

	_stop_cursor (widget, TRUE);

	while ( aim_ptr->i_char_table != NULL )
	{
	    first_pos = XYToPos (widget, x, y);
	    last_pos  = XYToPos (widget, x+width, y+height);

	    if ( first_pos >= last_pos )
		first_pos = first_pos - 1;   /* force a redraw */

	    DXmCSTextMarkRedraw( widget, first_pos, last_pos );

	    if (aim_ptr->i_char_table->next != NULL)
	    {
		 i_char_tab = aim_ptr->i_char_table->next;
	    } else {
		 i_char_tab = NULL;
	    }
			 
	    XtFree (aim_ptr->i_char_table->s);
	    XtFree (aim_ptr->i_char_table->charset);
	    XtFree (aim_ptr->i_char_table);

	    aim_ptr->i_char_table = i_char_tab;
	}
					
        DXmCSTextEnableRedisplay(widget);

	if (CSTextHasFocus (widget))
	{
	    _create_cursor (widget, CursorPos (widget));
	}

	_XmTextDrawShadow (widget);

	return;

    }  /* if cs == NULL */


    /* now, should be able to display the intermediate char, do it
     */
    if ( XmStringInitContext (&context, cs) )
    {

	DXmCSTextDisableRedisplay(widget, FALSE);

	_undraw_cursor (widget);

	pos = CursorPos ( widget );
	PosToXY ( widget, pos, &x, &y );

	/* for Hangul, there is ONLY ONE segment.  coding here is just in case
	 */

	i_char_tab = aim_ptr->i_char_table;

	while ( XmStringGetNextSegment ( context,
					 &text,
					 &charset,
					 &direction,
					 &seperator) )
	{
	    while ( i_char_tab != NULL && i_char_tab->next != NULL)
	    {
		i_char_tab = i_char_tab->next;
	    }

	    if ( aim_ptr->i_char_table == NULL )
	    {
		aim_ptr->i_char_table =
		i_char_tab  = (ICharStruct) XtMalloc(sizeof (ICharStructRec));

		i_char_tab->next = NULL;

	    } else {

		i_char_tab->next = (ICharStruct)
					XtMalloc(sizeof (ICharStructRec));

		i_char_tab = i_char_tab->next;

		i_char_tab->next = NULL;
	    }

	    i_char_tab->s = text;		/* freed in preedit draw */
	    i_char_tab->charset = charset;	/* freed in preedit draw */

	    /* now store x, y and width height
	     */
	    i_char_tab->x = x;
	    i_char_tab->y = y;
	    i_char_tab->height = MaxAscent (widget) +
				 MaxDescent (widget);    /* Fixed by JRD */

	    s = (unsigned long) text;

	    SearchFontList ( widget, charset, &entry );

            if (FontListType(entry) == XmFONT_IS_FONT) {
               font = (XFontStruct *)FontListFont(entry);
   	       XSetFont ( XtDisplay (widget), NormalGC(widget), font->fid );

	       i_char_tab->width  = font->max_bounds.width;

      	       XDrawImageString16 ( XtDisplay(widget),
	          		  XtWindow(widget),
				  NormalGC(widget),
				  x,
				  y + MaxAscent (widget),
				  s,
				  1 );

	       /* advance x to the next position */
	       x = x + (font->max_bounds.rbearing - font->min_bounds.lbearing);
            } else { /* XmFONT_IS_FONTSET */
	       XRectangle ink, logical;
	       XFontSet font_set = (XFontSet)FontListFont(entry);

      	       XmbDrawImageString ( XtDisplay(widget),
	          		  XtWindow(widget),
                                  font_set,
    				  NormalGC(widget),
       				  x,
				  y + MaxAscent (widget),
				  text,
				  1 );

	       /* advance x to the next position */
	       XmbTextExtents(font_set, text,
			      mblen(text, MB_CUR_MAX),
			      &ink, &logical);
	       x = x + abs(ink.width);
            } /* XmFONT_IS_FONTSET */
	} /* while */

	XmStringFreeContext (context);

    	_draw_cursor (widget);

	DXmCSTextEnableRedisplay(widget);

    } else {

        printf("_draw_intermediate_char: XmStringInit error\n");
    }	
}


static void _query_xy_position( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget	aim;
DXmAIMCallbackStruct *data;
{
    DXmCSTextPosition pos;
    Position x, y;

    pos = CursorPos ( widget );

    PosToXY ( widget, pos, &x, &y );

    data->x = (int) x;
    data->y = (int) y;
}


static void _rimp_destroy( widget, aim, data )
DXmCSTextWidget widget;
DXmAIMWidget	aim;
DXmAIMCallbackStruct *data;
{
}


#endif /* ASIAN_IO_SHR */

/* A common routine for setting up the font_entry of a segment
**
** Besides setting the font_entry field itself, it computes the
** appropriate tab_width.
**
** Motivation: QAR 1290 (crash when tabbing on 2nd or later line of
** a word-wrapped DXmCSText widget) - while the equivalent code in
** _compute_segment did the right thing, _check_wrapped_segment was
** setting the font_entry without setting tab_width, resulting in an
** integer divide by zero later on
*/
static void _get_font_entry( widget, in_seg, out_seg)
DXmCSTextWidget widget;
TextSegment	in_seg;
DXmCSTextOutputSegment  out_seg;
{
    unsigned long w;
    XtPointer tmp_font;
    XmFontType type_return = XmFONT_IS_FONT;
    XFontStruct *font;
    XFontSetExtents *fs_extents;

    /* Set up the font entry itself */
    out_seg->font_entry =
	(XmFontListEntry *)XtMalloc(sizeof(XmFontListEntry));
    SearchFontList(widget, in_seg->charset, out_seg->font_entry);

    /* Note that a recompute is needed */
    out_seg->need_recompute = TRUE;
    
    /* Compute the tab width (original comment says: "follow the
    ** same strategy of xmtext") */
    w = 0;
    tmp_font = XmFontListEntryGetFont(*out_seg->font_entry,
				      &type_return);
    if (type_return == XmFONT_IS_FONT)
	{
	font = (XFontStruct *)tmp_font;

	if ( (!XGetFontProperty(font, XA_QUAD_WIDTH, &w)) || (w == 0) )
	    {
	    if (font->per_char && (font->min_char_or_byte2 <= '0') &&
				  (font->max_char_or_byte2 >= '0'))
		w = font->per_char['0' - font->min_char_or_byte2].width;
	    else
		w = font->max_bounds.width;
	    }
	}
    else    /* XmFONT_IS_FONTSET */
	{
	font = (XFontStruct *)tmp_font;
	fs_extents = XExtentsOfFontSet((XFontSet)font);
	w = (unsigned long)fs_extents->max_ink_extent.width;
	}

    /* Make sure we end up with a tab_width > 0 */
    if (w <= 0)
	w = 1;

    out_seg->tab_width = 8 * w;
}
