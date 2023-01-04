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
**	 8 Feb 1993  Removed padding_* fields from struct _DXmCSTextPart
**
**	15 Mar 1993  Added verify_bell field
**
**--
**/

#ifndef _DXmCSTextP_h
#define _DXmCSTextP_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:XmP.h>
#include <DECW$INCLUDE:IntrinsicP.h>
#include <DECW$INCLUDE:PrimitiveP.h>
#include <DECW$INCLUDE:DXmCSText.h>
#else
#include <Xm/XmP.h>
#include <X11/IntrinsicP.h>
#include <Xm/PrimitiveP.h>
#include <DXm/DXmCSText.h>
#endif


typedef struct {
        XtPointer         extension;      /* Pointer to extension record */
} CSTextClassPart;

typedef struct _DXmCSTextClassRec {
    CoreClassPart 		core_class;
    XmPrimitiveClassPart 	primitive_class;
    CSTextClassPart 		cstext_class;
} DXmCSTextClassRec, *DXmCSTextClass;

#ifndef NO_CSTEXT_EXTERNALREF
externalref DXmCSTextClassRec dxmCSTextClassRec;
#endif


typedef struct _DXmCSTextInputRec 	*DXmCSTextInput;
typedef struct _DXmCSTextOutputRec 	*DXmCSTextOutput;
typedef struct _TextLine		*TextLine;

typedef void (*DXmCSTextOutputCreateProc)(); /* widget, arglist, argcount */
typedef void (*DXmCSTextInputCreateProc)();  /* widget, args, num_args */

typedef struct _DXmCSTextPart 
{
    DXmCSTextOutput    output;
    DXmCSTextInput     input;
    DXmCSTextOutputCreateProc  output_create; /* Creates output portion. */
    DXmCSTextInputCreateProc   input_create;  /* Creates input portion.  */

    XtCallbackProc    nofont_callback;
    XtCallbackProc    activate_callback;
    XtCallbackProc    focus_callback;
    XtCallbackProc    losing_focus_callback;
    XtCallbackProc    value_changed_callback;
    XtCallbackProc    modify_verify_callback;
    XtCallbackProc    motion_verify_callback;

    XmString 	      value;

    TextLine          lines;            /* source's data structures hook     */

    int 	      max_length;	/* max allowable length of string    */
    int               length;           /* current char count                */

    int 	      text_path;
    int 	      editing_path;
    int 	      edit_mode;	/* Sets the line editing mode        */

    Dimension 	      margin_height;
    Dimension         margin_width;

    Dimension	      rows;
    Dimension	      columns;

    DXmCSTextPosition  top_position;     /* First position to display.        */
    DXmCSTextPosition  bottom_position;  /* Last position to display.         */
    DXmCSTextPosition  cursor_position;  /* Location of the insertion point.  */

    Boolean 	      add_mode;	        /* Determines the state of add mode  */
    Boolean 	      auto_show_cursor_position; 
					/* do we automatically try to show   */
					/* the cursor position whenever it   */
					/* changes.                          */
    Boolean 	      editable;	        /* Determines if text is editable    */
    Boolean 	      traversed;	/* Flag used with losing focus       */
					/* verification to indicate a        */
                                        /* traversal key pressed event       */

    Boolean 	      needs_redisplay;  /* If need to do things              */
    Boolean 	      needs_refigure_lines;

    Boolean 	      in_redisplay;     /* If in various proc's              */
    Boolean 	      in_resize;
    Boolean 	      in_refigure_lines;

    Boolean	      has_selection;    /* Does the widget own primary sel?  */
    Boolean	      has_focus;        /* Does the widget have the focus?   */

    Boolean	      verify_bell;	/* Determines if bell is sounded     */
					/* when verify callback returns	     */
    					/* doit - False			     */
    DXmCSTextPosition  sel_left;	/* left position of the selection    */
    DXmCSTextPosition  sel_right;	/* right position of the selection   */

    DXmCSTextPosition  new_top;	        /* Desired new top position.         */

    int 	      disable_depth;    /* How many levels of disable we've  */
					/* done.                             */

                                        /* current unhandled scroll demands  */
    int		      pending_vertical_scroll,
                      pending_horiz_scroll;

    Widget 	      inner_widget;     /* Ptr to widget which really has    */
				        /* text (may be same or different    */
                                        /* from this widget                  */

    XmStringCharSet    def_charset;     /* char set to be used in SelfInsert */
    
} 
    DXmCSTextPart;


typedef struct _DXmCSTextRec 
{
    CorePart		core;
    XmPrimitivePart 	primitive;
    DXmCSTextPart 	cstext;
} 
    DXmCSTextRec;


typedef DXmCSTextPosition (*DXmCSTextGetLastPositionProc)(
#ifndef _NO_PROTO
    DXmCSTextWidget 	/* widget */
#endif
);

typedef void (*DXmCSTextSetSelectionProc)(
#ifndef _NO_PROTO
    Widget          	/* widget */,
    DXmCSTextPosition   /* first  */,
    DXmCSTextPosition   /* last   */,
    Time                /* time   */
#endif
);

typedef struct {
    XtPointer next_extension;	/* 1st 4 mandated for all extension records */
    XrmQuark record_type;	/* NULLQUARK */
    long version;		/* must be DXmCSTextExtVersion */
    Cardinal record_size;	/* sizeof(DXmCSTextClassExtRec) */
    DXmCSTextGetLastPositionProc get_last_position;
    DXmCSTextSetSelectionProc set_selection;
} DXmCSTextClassExtRec, *DXmCSTextClassExtension;

#define DXmCSTextClassExtVersion 1L


#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DXmCSTextP_h */
