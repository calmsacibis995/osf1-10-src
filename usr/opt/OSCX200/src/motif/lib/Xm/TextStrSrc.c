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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/TextStrSrc.c,v 1.1.2.4 92/04/15 19:18:34 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)TextStrSrc.c	3.20.1.9 91/07/02";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*
*       THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*
*       THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS
*
*       OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED
*
*
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
#include <X11/Xatom.h>
#include <Xm/TextP.h>
#include <Xm/AtomMgr.h>

#ifdef VAXC
#pragma inline (Look)
#endif

#ifdef DEC_MOTIF_BUG_FIX
static XmTextPosition Scan();
#endif
static void SetSelection();
static void CountTotalLines();
static XmTextPosition ReadSource();

#ifdef _NO_PROTO
char *_XmStringSourceGetString(w, from, to)
XmTextWidget w;
XmTextPosition from, to;
#else /* _NO_PROTO */
char *_XmStringSourceGetString(XmTextWidget w, XmTextPosition from, XmTextPosition to)
#endif /* _NO_PROTO */
{
   char *buf;
   XmTextBlockRec text;
   int destpos;
   XmTextPosition pos;

   buf = XtMalloc((to - from)+1);
   destpos = 0;
   for (pos = from; pos < to; ){
       pos = ReadSource(w->text.source, pos, to, &text);
       if (text.length == 0)
          break;
       strncpy(&buf[destpos], text.ptr, text.length);
       destpos += text.length;
   }
   buf[destpos] = 0;
   return buf;
}


/* ARGSUSED */
static void InsertSelection(w, closure, seltype, type, value, length, format)
Widget w;
Opaque closure;
Atom *seltype;
Atom *type;
char *value;
int *length;
int *format;
{
    XmAnyCallbackStruct cb;
    Boolean *select_done = (Boolean *)closure;
    XmTextWidget widget = (XmTextWidget) w;
    XmTextPosition left, right;
    XmTextPosition cursorPos;
    XmTextBlockRec block;
    XSelectionRequestEvent *req_event;

    if (!value) {
        select_done[0] = True;
        return;
    }

  /* Don't do replace if there is not text to add */
    if (*value == NULL || *length == 0){
       XtFree(value);
       select_done[0] = True;
       return;
    }

    block.ptr = value;
    block.length = *length;
    block.format = (int) *type;

    if (!(*widget->text.source->GetSelection)
                            (widget->text.source, &left, &right)) {
       XBell(XtDisplay(widget), 0);

       XtFree(value);
       select_done[0] = True;
       select_done[1] = False;
       return;
    }

    if (left > right) right = left; 

    req_event = XtGetSelectionRequest((Widget)widget, *seltype, NULL);
    if ((*widget->text.source->Replace)(widget->text.source,
		      (XEvent *)req_event, left, right, &block) != EditDone) {
       if (widget->text.verify_bell) XBell(XtDisplay(widget), 0);
       select_done[1] = False;
    } else {
      /*
       * Call ValueChanged Callback to indicate that
       * text has been modified.
       */
       cb.reason = XmCR_VALUE_CHANGED;
       cb.event = (XEvent *) req_event;
       XtCallCallbackList ((Widget) widget, (XtCallbackList)widget->text.value_changed_callback, (XtPointer) &cb);
       select_done[1] = True;
    }
    cursorPos = left + *length;

    if (widget->text.add_mode &&
        cursorPos >= left && cursorPos <= right)
       widget->text.pendingoff = FALSE;
    else
       widget->text.pendingoff = TRUE;

    _XmTextSetCursorPosition((Widget)widget, cursorPos);
    (*widget->text.output->DrawInsertionPoint)(widget,
                                            widget->text.cursor_position, on);
    XtFree(value);
    select_done[0] = True;
}


/*
 * Converts requested target of insert selection.
 */
/*--------------------------------------------------------------------------+*/
static Boolean ConvertInsertSelection(w, selection, type,
                                      value, length, format)
/*--------------------------------------------------------------------------+*/
Widget w;
Atom *selection, *type;
caddr_t *value;
int *length, *format;
{
   XtAppContext app = XtWidgetToApplicationContext(w);
   XSelectionRequestEvent * req_event;
   static unsigned long old_serial = 0;
   Atom actual_type;
   int actual_format;
   unsigned long nitems;
   unsigned long bytes;
   unsigned char *prop;
   Boolean select_done[2];
   TextInsertPair *pair;

   select_done[0] = False;
   select_done[1] = False;

   req_event = XtGetSelectionRequest(w, *selection, NULL);

  /* Work around for intrinsics selection bug */
   if (old_serial != req_event->serial)
      old_serial = req_event->serial;
   else
      return False;

   XGetWindowProperty(req_event->display, req_event->requestor,
                      req_event->property, 0L, 10000000, False,
                      AnyPropertyType, &actual_type, &actual_format,
                      &nitems, &bytes, &prop);

   pair = (TextInsertPair *)prop;

  /*
   * Make selection request to replace the primary selection
   * with the insert selection.
   */
   XtGetSelectionValue(w, pair->selection, pair->target,
		       (XtSelectionCallbackProc) InsertSelection,
                       (Opaque)select_done, CurrentTime);
  /*
   * Make sure the above selection request is completed
   * before returning from the convert proc.
   */
   for (;;) {
       XEvent event;

       if (select_done[0])
          break;
       XtAppNextEvent(app, &event);
       XtDispatchEvent(&event);
   }

   *type = XmInternAtom(XtDisplay(w), "INSERT_SELECTION", False);
   *format = 8;
   *value = NULL;
   *length = 0;

   XtFree((char *)prop);

   return (select_done[1]);
}


/* ARGSUSED */
static Boolean Convert(w, selection, target, type, value, length, format)
Widget w;
Atom *selection;
Atom *target;
Atom *type;
caddr_t *value;
int *length;
int *format;
{
    XmTextWidget widget = (XmTextWidget) w;
    XmSourceData data = widget->text.source->data;
    Atom XA_INSERT_SELECTION = XmInternAtom(XtDisplay(widget),
					   "INSERT_SELECTION", False);
    Atom XA_DELETE = XmInternAtom(XtDisplay(widget), "DELETE", False);
    Atom XA_TARGETS = XmInternAtom(XtDisplay(widget), "TARGETS", False);
    Atom XA_TEXT = XmInternAtom(XtDisplay(widget), "TEXT", False);
    Atom XA_TIMESTAMP = XmInternAtom(XtDisplay(w), "TIMESTAMP", False);
    XSelectionRequestEvent * req_event;

    if (*selection != XA_PRIMARY) return FALSE;

  /*
   * XA_TARGETS identifies what targets the text widget can
   * provide data for.
   */
    if (*target == XA_TARGETS) {
      Atom *targs = (Atom *)XtMalloc((unsigned) (6 * sizeof(Atom)));

      *value = (caddr_t)targs;
      *targs++ = XA_TARGETS;
      *targs++ = XA_STRING;
      *targs++ = XA_TEXT;
      *targs++ = XA_INSERT_SELECTION;
      *targs++ = XA_TIMESTAMP;
      *targs = XA_DELETE;
      *type = XA_TARGETS;
      *length = (6*sizeof(Atom)) >> 2; /*convert to work count */
      *format = 32;
   } else if (*target == XA_TIMESTAMP) {
      Time *timestamp;
      timestamp = (Time *) XtMalloc(sizeof(Time));
      *timestamp = data->prim_time;
      *value = (char *) timestamp;
      *type = XA_TIMESTAMP;
      *length = sizeof(Time);
      *format = 32;
  /* Provide data for XA_STRING and XA_TEXT requests */
    } else if (*target == XA_STRING || *target == XA_TEXT) {
      *type = (Atom) FMT8BIT;	
      *format = 8;
      if (data == NULL || !data->hasselection) return FALSE;
      *length = data->right - data->left;
      *value = _XmStringSourceGetString(widget, data->left, data->right);
  /*
   * Provide data for XA_INSERT_SELECTION requests, used in
   * swaping selections.
   */
    } else if (*target == XA_INSERT_SELECTION) {
      return (ConvertInsertSelection(w, selection, type,
                                     value, length, format));

  /* Delete the selection */
    } else if (*target == XA_DELETE) {
       XmTextBlockRec block;

       block.length = 0;
       block.format = FMT8BIT; /* ensure that callback will be given OK data */

       req_event = XtGetSelectionRequest(w, *selection, NULL);
       if ((*widget->text.source->Replace)(widget->text.source,
	       (XEvent*)req_event, data->left, data->right, &block) != EditDone)
	   return False;

       if (!widget->text.input->data->has_destination)
          widget->text.input->data->anchor = widget->text.cursor_position;

       data->right = data->left = widget->text.input->data->anchor;

       *type = XA_DELETE;
       *value = NULL;
       *length = 0;
       *format = 8;
  /* unknown selection type */
    } else
      return FALSE;
    return TRUE;
}

/* ARGSUSED */
static void LoseSelection(w, selection)
Widget w;
Atom *selection;
{
    XmTextWidget widget = (XmTextWidget) w;
    XmSourceData data = widget->text.source->data;
    if (data && data->hasselection) {
       XmAnyCallbackStruct cb;

       (*data->source->SetSelection)(data->source, 1, -999, CurrentTime);

       cb.reason = XmCR_LOSE_PRIMARY;
       cb.event = NULL;
       XtCallCallbackList ((Widget) widget,
			   (XtCallbackList)widget->text.lose_primary_callback, 
			   (XtPointer) &cb);

    }
}

static void AddWidget(source, widget)
XmTextSource source;
XmTextWidget widget;
{
    XmSourceData data = source->data;
    data->numwidgets++;
    data->widgets = (XmTextWidget *)
	XtRealloc((char *) data->widgets,
		  (unsigned) (sizeof(XmTextWidget) * data->numwidgets));
    data->widgets[data->numwidgets - 1] = widget;

    if (data->hasselection && data->numwidgets == 1)
	if (!XtOwnSelection((Widget) data->widgets[0], XA_PRIMARY,
			    CurrentTime,
			    (XtConvertSelectionProc) Convert,
			    (XtLoseSelectionProc) LoseSelection,
			    (XtSelectionDoneProc) NULL)) {
	    SetSelection(source, 1, 0, CurrentTime);
        } else {
            XmAnyCallbackStruct cb;

            data->prim_time = CurrentTime;
            cb.reason = XmCR_GAIN_PRIMARY;
            cb.event = NULL;
            XtCallCallbackList ((Widget) data->widgets[0],
				(XtCallbackList)data->widgets[0]->text.gain_primary_callback,

				(XtPointer) &cb);
        }

    if (data->numwidgets == 1 &&
        _XmTextScrollable(widget) &&
        !_XmTextShouldWordWrap(widget))
       CountTotalLines(source, 0, data->length);

}

/********************************<->***********************************/
static char _XmStringSourceGetChar(data, position)
/********************************<->***********************************/
XmSourceData data;
XmTextPosition position;        /* starting position */
{
    int gap_size = data->gap_end - data->gap_start;

    if (data->ptr + position < data->gap_start)
       return (data->ptr[position]);
    return (data->ptr[position + gap_size]);
}



static int CountLines(source, start, length)
XmTextSource source;
XmTextPosition start;
int length;
{
    XmSourceData data = source->data;
    int num_lines = 0;
    int seg_length;
    char *ptr;


  /* verify that the 'start' and 'length' parameters are reasonable */
    if (start + length > data->length)
       length = data->length - start;
    if (length <= 0) return num_lines;

  /* setup the variables for the search of new lines before the gap */
    ptr = data->ptr + start; 
    seg_length = data->gap_start - ptr;

  /* make sure the segment length is not greater than the length desired */
    if (length < seg_length) seg_length = length;

  /* make sure the segment length is not less than 0, indicating
   * that start begins after gap_start.
   */
    if (seg_length < 0) seg_length = 0;

  /* search up to gap */
    while (seg_length--) {
	if (*ptr++ == '\012') ++num_lines;
    }
	
  /* check to see if we need more data after the gap */
    if (length > data->gap_start - (data->ptr + start)) {
       length -= data->gap_start - (data->ptr + start);
       ptr += data->gap_end - data->gap_start;

  /* continue search till length is completed */
       while (length--) {
	   if (*ptr++ == '\012') ++num_lines;
       }
    }
	
    return num_lines;
}


static void CountTotalLines(source, start, length)
XmTextSource source;
XmTextPosition start;
int length;
{
    XmSourceData data = source->data;
    int num_lines = 0;
    int i;

    if (length > 0) {
      num_lines = CountLines(source, start, length);
      for (i = 0; i < data->numwidgets; i++) {
         XmTextWidget tw = (XmTextWidget) data->widgets[i];
         tw->text.total_lines += num_lines;
     	} 
     } else if (length < 0){
       	       length = -length;
               num_lines = CountLines(source, start, length);
       	       for (i = 0; i < data->numwidgets; i++) {
                       XmTextWidget tw = (XmTextWidget) data->widgets[i];
          	       tw->text.total_lines -= num_lines;
          	       if (tw->text.total_lines < 1)
	                  tw->text.total_lines = 1;
       	       }
     } else if (_XmStringSourceGetChar(data, 0) != NULL){
        length = ((XmTextWidget)data->widgets[0])->text.last_position;
        start = 0;
        num_lines = CountLines(source, start, length);
	for (i = 0; i < data->numwidgets; i++) {
           XmTextWidget tw = (XmTextWidget) data->widgets[i];
           tw->text.total_lines = num_lines;
        }
     }
}


static void RemoveWidget(source, widget)
XmTextSource source;
XmTextWidget widget;
{
    XmSourceData data = source->data;
    int i;
    for (i=0 ; i<data->numwidgets ; i++) {
	if (data->widgets[i] == widget) {
            XmTextPosition left, right;
            Boolean had_selection = False;
	    if (data->hasselection) {
                (*source->GetSelection)(source, &left, &right);
                (*source->SetSelection)(source, 1, -999, CurrentTime);
                had_selection = True;
            }
	    data->numwidgets--;
	    data->widgets[i] = data->widgets[data->numwidgets];
	    if (i == 0 && data->numwidgets > 0 && had_selection)
	       SetSelection(source, left, right, CurrentTime);
            if (data->numwidgets == 0) _XmStringSourceDestroy(source);
	    return;
	}
    }
}

/*
 * Physically moves the memory.
 */
/********************************<->***********************************/
static void _XmStringSourceMoveMem(from, to, length)
/********************************<->***********************************/
char *from, *to;   /* from & to are low-address markers. */
int length;
{
    if (from < to) {
       /* move gap to the left */
        --length;
	to += length;
	from += length;
        ++length;
        while(length--) *to-- = *from --;
    } else
       /* move gap to the right */
        while(length--) *to++ = *from ++;
}

/*
 * Determines where to move the gap and calls _XmStringSourceMoveMem()
 * to do the physical move of the gap.
 */
#ifdef _NO_PROTO
/********************************<->***********************************/
void _XmStringSourceSetGappedBuffer(data, position)
/********************************<->***********************************/
XmSourceData data;
XmTextPosition position;        /* starting position */
#else /* _NO_PROTO */
void _XmStringSourceSetGappedBuffer (XmSourceData data, XmTextPosition position)
#endif /* _NO_PROTO */
{
    int count;


   /* if no change in gap placement, return */
    if (data->ptr + position == data->gap_start) return;

    if (data->ptr + position < data->gap_start) {
       /* move gap to the left */
        count = data->gap_start - (data->ptr + position);
	_XmStringSourceMoveMem((data->ptr + position),
			       (data->gap_end - count), count);
        data->gap_start -= count; /* ie, data->gap_start = position; */
        data->gap_end -= count;   /* ie, data->gap_end = position + gap_size; */
    } else {
       /* move gap to the right */
        count = (data->ptr + position) - data->gap_start;
	_XmStringSourceMoveMem(data->gap_end, data->gap_start, count);
        data->gap_start += count; /* ie, data->gap_start = position; */
        data->gap_end += count;   /* ie, data->gap_end = position + gap_size; */
    }
}

/********************************<->***********************************/
static void _XmStringSourceReadString(source, start, block)
/********************************<->***********************************/
XmTextSource source;
int start;
XmTextBlock block;              /* RETURN: text read in */
{
    XmSourceData data = source->data;
    int gap_size = data->gap_end - data->gap_start;

    if (data->ptr + start + block->length <= data->gap_start)
        block->ptr = data->ptr + start;
    else if (data->ptr + start + gap_size >= data->gap_end)
            block->ptr = data->ptr + start + gap_size;
	 else {
            int size_to_gap_start = data->gap_start - (data->ptr + start);
            block->ptr = data->ptr + start;
            block->length = data->gap_start - (data->ptr + start);
         }
}


static XmTextPosition ReadSource(source, position, last_position, block)
XmTextSource source;
XmTextPosition position;        /* starting position */
XmTextPosition last_position;   /* The last position we're interested in.
                                   Don't return info about any later
                                   positions. */
XmTextBlock block;              /* RETURN: text read in */
{
    XmSourceData data = source->data;
    if (last_position > data->length) last_position = data->length;
    block->length = last_position - position;
    block->format = FMT8BIT;
    _XmStringSourceReadString(source, (int)position, block);
    return position + block->length;
}

static XmTextStatus Replace(source, event, start, end, block)
XmTextSource source;
XEvent *event;
XmTextPosition start, end;
XmTextBlock block;
{
    XmSourceData data = source->data;
    Boolean do_call = False;
    int i, gap_size;
    long delta;
    XmTextVerifyCallbackStruct tvcb;
    XmTextBlockRec newblock;
    int old_maxlength;
    char *saved_block_ptr;
#ifdef DEC_MOTIF_EXTENSION
    XmTextPosition oldlinebegin, newlinebegin;
    XmTextPosition oldlineend, newlineend;
    int *oldcount;
    Boolean anyshouldwordwrap = False;
    Boolean do_count = False;
#endif

    delta = block->length - (end -start);
    if ( !data->editable ||
	  (delta > 0 && data->length + delta > data->maxallowed))
	return EditError;

   /* Make sure there are callbacks registered */
    for (i=0 ; i<data->numwidgets ; i++) {
	if (data->widgets[i]->text.modify_verify_callback) {
	   do_call = True;
           break;
        }
    }

   /* If there are callbacks registered, call them. */
    if (do_call) {
     /* Fill in the block to pass to the callback. */
      saved_block_ptr = block->ptr;
      newblock.format = block->format;
      newblock.length = block->length;
      if (block->length) {
        newblock.ptr = (char *) XtMalloc(block->length);
        strncpy(newblock.ptr, block->ptr, block->length);
      } else newblock.ptr = NULL;

      for (i=0 ; i<data->numwidgets ; i++) {
       /* Call Verification Callback to indicate that text is being modified */
        tvcb.reason = XmCR_MODIFYING_TEXT_VALUE;
        tvcb.event = event;
        tvcb.currInsert = (XmTextPosition)
		       ((XmTextWidget)(data->widgets[i])->text.cursor_position);
        tvcb.newInsert = (XmTextPosition)
		       ((XmTextWidget)(data->widgets[i])->text.cursor_position);
        tvcb.startPos = start;
        tvcb.endPos = end;
        tvcb.doit = True;
        tvcb.text = &newblock;
        XtCallCallbackList ((Widget) data->widgets[i], 
			    (XtCallbackList)data->widgets[i]->text.modify_verify_callback,
			      (XtPointer)&tvcb);
       /* If doit flag is false, application wants to negate the action,
        * so free allocate space and return EditReject.
        */
        if (!tvcb.doit) {
          XtFree(newblock.ptr);
          block->length = 0; /* Necessary inorder to return to initial state */
          block->ptr = saved_block_ptr;
          return EditReject;
        } else {
          start = tvcb.startPos;
          if (start > data->length) start = data->length;
          if (start < 0) start = 0;
          end = tvcb.endPos;
          if (end > data->length) end = data->length;
          if (end < 0) end = 0;
          if (end < start) end = start;
          if (tvcb.text != &newblock) {
            newblock.length = tvcb.text->length;
            XtFree(newblock.ptr);
            if (newblock.length) {
              newblock.ptr = (char *) XtMalloc(newblock.length);
              strncpy(newblock.ptr, tvcb.text->ptr, newblock.length);
            } else newblock.ptr = NULL;
          }
          delta = newblock.length - (end -start);
	  if (delta > 0 && data->length + delta > data->maxallowed) {
             XtFree(newblock.ptr);
             block->ptr = saved_block_ptr;
	     return EditError;
        }
      }
    }
     /* Point block pointer at possibly modified, possibly re-allocated text */
      block->ptr = newblock.ptr;
      block->length = newblock.length;
    }

#ifdef DEC_MOTIF_EXTENSION
    /*
     *  The purpose of this DEC_MOTIF_EXTENSION is to speed up text
     *  widget interactive performance when word wrapping and
     *  vertical scrolling are set.  The problem without the extension
     *  is that every time this routine is called, TextOut.c\ChangeVSB
     *  gets called to count the number of lines in the file so that
     *  the size of the scroll bar can be set properly.
     *
     *  This extension adds some logic to help limit the number of
     *  times TextOut.c\ChangeVSB is called.  It works by finding the
     *  previous and next line feeds in the text, and counting the
     *  number of lines in between.  After the replace has occurred,
     *  it counts again.  If there was no line feed in the added or
     *  deleted text, and the count hasn't changed, then a flag is
     *  set to prevent TextOut.c\ChangeVSB from being called.
     */

   /* Determine if any should word wrap */
    for (i=0 ; i<data->numwidgets ; i++) {
	if (_XmTextShouldWordWrap(data->widgets[i])) {
	   anyshouldwordwrap = True;
           break;
        }
    }
    if (anyshouldwordwrap)
    {
	XmTextPosition tpos;

	do_count = True;
        /*
         *  First determine if there is a line feed in the text to be
	 *  added/deleted
	 */
	if (block->length > 0)			/* Adding text... */
	{
	    for (i = 0; i < block->length; i++)
		if (block->ptr[i] == '\n')
		{
            	    do_count = False;
		    break;
		}
	}
	if (do_count && (end - start) > 0)      /* Deleting text... */
	{
	    static char Look();
	    for (tpos = start; tpos < end; tpos++)
	        if (Look(data, tpos, XmsdRight) == '\n')
		{
            	    do_count = False;
		    break;
		}
	}
        if (do_count) 
	{
	    /*
	     *  See if we can use the saved beginning and end positions
	     */
	    if ((start >= data->opaque1) && (end <= data->opaque2))
	    {
		oldlinebegin = data->opaque1;
		oldlineend = data->opaque2;
		data->opaque2 += delta;
	    }
	    else
	    {
	        /*
                 *  Call Scan to find out the beginning and end position of 
		 *  the current line
                 */
                oldlinebegin = Scan (source, start, XmSELECT_LINE, XmsdLeft,  1, FALSE);
                oldlineend   = Scan (source, start, XmSELECT_LINE, XmsdRight, 1, FALSE);
		data->opaque1 = oldlinebegin;
		data->opaque2 = oldlineend + delta;
	    }
	    oldcount = (int *)XtCalloc(data->numwidgets, sizeof(int*));
	    for (i=0 ; 
		 i<data->numwidgets && _XmTextShouldWordWrap(data->widgets[i])
		  && data->widgets[i]->text.output_create == _XmTextOutputCreate;
		 i++)
	        for (tpos = oldlinebegin; tpos <= oldlineend; oldcount[i]++)
	            tpos = FindLineEnd(data->widgets[i], tpos, NULL);
	}
	else
	    data->opaque1 = data->opaque2 = -1;
    }
    else
	data->opaque1 = data->opaque2 = -1;
#endif

   /* Move the gap to the editing position (start). */
    _XmStringSourceSetGappedBuffer(source->data, start);

   for (i=0 ; i<data->numwidgets ; i++) {
	_XmTextDisableRedisplay(data->widgets[i], TRUE);
	if (data->hasselection)
	    XmTextSetHighlight((Widget)data->widgets[i], data->left, data->right,
				XmHIGHLIGHT_NORMAL);
    }

    old_maxlength = data->maxlength;
    while (data->length + delta >= data->maxlength) {
	data->maxlength *= 2;

        if (data->length + delta < data->maxlength) {
           int gap_start_offset, gap_end_offset;

           gap_start_offset = data->gap_start - data->ptr;
           gap_end_offset = data->gap_end - data->ptr;
	   data->ptr = XtRealloc(data->ptr, (unsigned) (data->maxlength));
           data->gap_start = data->ptr + gap_start_offset;
           data->gap_end = data->ptr + gap_end_offset +
			   (data->maxlength - old_maxlength);
           if (gap_end_offset != old_maxlength)
              _XmStringSourceMoveMem(data->ptr + gap_end_offset,
			       data->gap_end,
			       old_maxlength - gap_end_offset);

/* Do something to move the allocated space into the buffer */
	
        }
    }
    if (_XmTextScrollable(data->widgets[0]) &&
	 !_XmTextShouldWordWrap(data->widgets[0]) &&
        (start - end < 0))
            CountTotalLines(source, start, start - end);

    data->length += delta;

/* delete data */
    gap_size = data->gap_end - data->gap_start;
   /* expand the end of the gap to the right */
    if ((data->ptr + gap_size + end) > data->gap_end) 
       data->gap_end += (end - start);

/* add data */
   /* copy the data into the gap_start and increment the gap start pointer */
    for (i=0 ; i<block->length ; i++) {
        if (data->gap_start == data->gap_end) break;
	*data->gap_start++ = block->ptr[i];
    }
	
#ifdef DEC_MOTIF_EXTENSION
#define no_needs_count 2
    if (do_count) 
    {
	XmTextPosition tpos;
        /*
         *  Call Scan to find out the beginning and end position of the current line
         */
        newlinebegin = oldlinebegin;
        newlineend   = oldlineend + delta;

	for (i=0 ; 
	     i<data->numwidgets && _XmTextShouldWordWrap(data->widgets[i])
	      && data->widgets[i]->text.output_create == _XmTextOutputCreate;
	     i++)
        {
	    int newcount = 0;
	    for (tpos = newlinebegin; tpos <= newlineend; newcount++)
	        tpos = FindLineEnd(data->widgets[i], tpos, NULL);
	    if (oldcount[i] == newcount)
                data->widgets[i]->text.vsbar_scrolling |= no_needs_count;
	}
	XtFree((char*)oldcount);
    }
#endif

    if (data->hasselection) {
	if (data->left > start) data->left += delta;
	if (data->right > start) data->right += delta;
	if (data->left > data->right) data->right = data->left;
    }
    if (_XmTextScrollable(data->widgets[0]) &&
	 !_XmTextShouldWordWrap(data->widgets[0]) &&
         block->length != 0)
	CountTotalLines(source, start, block->length);
    for (i=0 ; i<data->numwidgets ; i++) {
	_XmTextInvalidate(data->widgets[i], start, end, delta);
	if (data->hasselection)
	    XmTextSetHighlight((Widget)data->widgets[i], data->left, data->right,
				XmHIGHLIGHT_SELECTED);

	_XmTextEnableRedisplay(data->widgets[i]);
    }
    if (do_call) {
      XtFree(newblock.ptr);
      block->ptr = saved_block_ptr;
    }
    return EditDone;
}

#define Increment(data, position, direction)\
{\
    if (direction == XmsdLeft) {\
	if (position > 0) \
	    position--;\
    }\
    else {\
	if (position < data->length)\
	    position++;\
    }\
}


static char Look(data, position, direction)
XmSourceData data;
XmTextPosition position;
XmTextScanDirection direction;
{
/* Looking left at pos 0 or right at position data->length returns newline */
    if (direction == XmsdLeft) {
	if (position == 0)
	    return(0);
	else
	    return(_XmStringSourceGetChar(data, position - 1));
    }
    else {
	if (position == data->length)
	    return(0);
	else
	    return(_XmStringSourceGetChar(data, position));
    }
}

static void ScanParagraph(data, new_position, dir, ddir, last_char)
XmSourceData data;
XmTextPosition *new_position;
XmTextScanDirection dir;
int ddir;
XmTextPosition *last_char;
{
   Boolean found = False;
   XmTextPosition position = *new_position;
   char c;

   while (position >= 0 && position <= data->length) {
       c = Look(data, position, dir);
       if (c == '\n') {
          c = Look(data, position + ddir, dir);
          while (isspace(c)) {
              if (c == '\n') {
                 found = True;
                 while (isspace(c)) {
                     c = Look(data, position + ddir, dir);
                     Increment(data, position, dir);
                 }
                 break;
              }
              c = Look(data, position + ddir, dir);
              Increment(data, position, dir);
          }
          if (found) break;
       } else if (!isspace(c)) {
          *last_char = (position) + ddir;
       }
          
       if(((dir == XmsdRight) && (position == data->length)) ||
           (dir == XmsdLeft) && ((position == 0)))
           break;
       Increment(data, position, dir);
   }

   *new_position = position;
}



static XmTextPosition Scan(source, pos, sType, dir, count, include)
XmTextSource source;
XmTextPosition pos;
XmTextScanType sType;
XmTextScanDirection dir;
int count;
Boolean include;
{
    XmSourceData data = source->data;
    XmTextPosition position;
    int i, whiteSpace;
    char c;
    int ddir = (dir == XmsdRight) ? 1 : -1;
    position = pos;
    switch (sType) {
	case XmSELECT_POSITION: 
	    if (!include && count > 0)
		count -= 1;
	    for (i = 0; i < count; i++) {
		Increment(data, position, dir);
	    }
	    break;
	case XmSELECT_WHITESPACE: 
	case XmSELECT_WORD:
#ifdef DEC_MOTIF_EXTENSION
            /*
	     *  This DEC_MOTIF_EXTENSION shortcuts calling the Look routine for
	     *  a common case...
	     */
 	    if ((dir == XmsdRight) && (data->ptr + position > data->gap_start))
	    {
		int gap_size =  data->gap_end - data->gap_start;
		XmTextPosition gapped_position = position + gap_size;
	        for (i = 0; i < count; i++) {
		    whiteSpace = -1;
		    while (position >= 0 && position < data->length) {
		        c = data->ptr[gapped_position];
		        if ((c == ' ') || (c == '\t') || (c == '\n')){
		            if (whiteSpace < 0) whiteSpace = position;
		        } else if (whiteSpace >= 0)
			    break;
		        position++;
		        gapped_position++;
		    }
	        }		
	    }
	    else
	    {
#endif
	    for (i = 0; i < count; i++) {
		whiteSpace = -1;
		while (position >= 0 &&	position <= data->length) {
		    c = Look(data, position, dir);
		    if ((c == ' ') || (c == '\t') || (c == '\n')){
		        if (whiteSpace < 0) whiteSpace = position;
		    } else if (whiteSpace >= 0)
			break;
		    position += ddir;
		}
	    }
#ifdef DEC_MOTIF_EXTENSION
	    }
#endif
 	    if (!include) {
	       	if(whiteSpace < 0 && dir == XmsdRight)
		     whiteSpace = data->length;
		position = whiteSpace;
	    }
	    break;
	case XmSELECT_LINE: 
	    for (i = 0; i < count; i++) {
#ifdef DEC_MOTIF_EXTENSION
            /*
	     *  This DEC_MOTIF_EXTENSION shortcuts calling the Look routine for
	     *  a common case...
	     */
 	        if ((dir == XmsdRight) && (data->ptr + position > data->gap_start))
	        {
		    int gap_size =  data->gap_end - data->gap_start;
		    XmTextPosition gapped_position = position + gap_size;
		    while (position >= 0 && position < data->length) {
		        if (data->ptr[gapped_position] == '\n')
			    break;
		        position++;
		        gapped_position++;
		    }
	        }
	        else
	        {
#endif
		while (position >= 0 && position <= data->length) {
		    if (Look(data, position, dir) == '\n')
			break;
		    if(((dir == XmsdRight) && (position == data->length)) || 
			(dir == XmsdLeft) && ((position == 0)))
			break;
		    Increment(data, position, dir);
		}
#ifdef DEC_MOTIF_EXTENSION
	        }
#endif
		if (i + 1 != count)
		    Increment(data, position, dir);
	    }
	    if (include) {
	    /* later!!!check for last char in file # eol */
		Increment(data, position, dir);
	    }
	    break;
	case XmSELECT_PARAGRAPH: 
           /* Muliple paragraph scanning is not guarenteed to work. */
            for (i = 0; i < count; i++) {
                XmTextPosition start_position = position; 
                XmTextPosition last_char = position; 

               /* if scanning forward, check for between paragraphs condition */
                if (dir == XmsdRight) {
                   c = Look(data, position, dir);
                  /* if is space, go back to first non-space */
                   while (isspace(c)) {
	               if (position > 0) {
	                  position--;
	               }
                       c = Look(data, position, XmsdLeft);
                   }
                }

                ScanParagraph(data, &position, dir, ddir, &last_char);

               /*
                * If we are at the beginning of the paragraph and we are
                * scanning left, we need to rescan to find the character
                * at the beginning of the next paragraph.
                */  
		if (dir == XmsdLeft) {
                  /* If we started at the beginning of the paragraph, rescan */
                   if (last_char == start_position)
                      ScanParagraph(data, &position, dir, ddir, &last_char);
                  /*
                   * Set position to the last non-space 
                   * character that was scanned.
                   */
                   position = last_char;
                }

                if (i + 1 != count)
                    Increment(data, position, dir);
                
            }
            if (include) {
                Increment(data, position, dir);
            }
            break;
	case XmSELECT_ALL: 
	    if (dir == XmsdLeft)
		position = 0;
	    else
		position = data->length;
    }
    if (position < 0) position = 0;
    if (position > data->length) position = data->length;
    return(position);
}



static Boolean GetSelection(source, left, right)
XmTextSource source;
XmTextPosition *left, *right; 
{
    XmSourceData data = source->data;

    if (data->hasselection && data->left <= data->right && data->left >= 0) {
	*left = data->left;
	*right = data->right;
	return TRUE;
    } else {
        Widget widget = (Widget) data->widgets[0];
        data->hasselection = FALSE;
    }
    return FALSE;
}


static void SetSelection(source, left, right, set_time)
XmTextSource source;
XmTextPosition left;
XmTextPosition right;	/* if right == -999, then  we're in LoseSelection,
			   so don't call XtDisownSelection. */
Time set_time;
{
    XmSourceData data = source->data;
    int i;

    if (!XtIsRealized((Widget)data->widgets[0])) return;

    if (left < 0) left = right = 0;
    for (i=0 ; i<data->numwidgets; i++) {
	_XmTextDisableRedisplay(data->widgets[i], FALSE);
	if (data->hasselection)
	    XmTextSetHighlight((Widget)data->widgets[i], data->left, data->right,
				XmHIGHLIGHT_NORMAL);
	if (left < right)
	    XmTextSetHighlight((Widget)data->widgets[i], left, right,
				XmHIGHLIGHT_SELECTED);
	_XmTextEnableRedisplay(data->widgets[i]);
    }
    data->left = left;
    data->right = right;
    if (data->numwidgets > 0) {
	Widget widget = (Widget) data->widgets[0];
        XmTextWidget tw = (XmTextWidget)widget;

	if (left <= right) {
	   if (!data->hasselection) {
	      if (!XtOwnSelection(widget, XA_PRIMARY, set_time,
				  (XtConvertSelectionProc) Convert, 
				  LoseSelection, (XtSelectionDoneProc) NULL)) {
		SetSelection(source, 1, 0, CurrentTime);
              } else {
                XmAnyCallbackStruct cb;

                data->prim_time = set_time;
                (*tw->text.output->DrawInsertionPoint)(tw,
					         tw->text.cursor_position, off);
                data->hasselection = True;

                cb.reason = XmCR_GAIN_PRIMARY;
                cb.event = NULL;
                XtCallCallbackList ((Widget) data->widgets[0],
 			           (XtCallbackList)data->widgets[0]->text.gain_primary_callback,

				   (XtPointer) &cb);
              }
           }
           if (left == right) {
              (*tw->text.output->DrawInsertionPoint)(tw,
				         tw->text.cursor_position, off);
              XmTextSetAddMode((Widget)tw, False);
           }
	} else {
	    if (right != -999)
	       XtDisownSelection(widget, XA_PRIMARY, set_time);

            (*tw->text.output->DrawInsertionPoint)(tw,
					         tw->text.cursor_position, off);
            data->hasselection = False;
            XmTextSetAddMode((Widget)tw, False);
        }
    }
}

/* Public routines. */

#ifdef _NO_PROTO
XmTextSource _XmStringSourceCreate(value)
char *value;
#else /* _NO_PROTO */
XmTextSource _XmStringSourceCreate (char *value)
#endif /* _NO_PROTO */
{
    XmTextSource source;
    XmSourceData data;
    source = (XmTextSource) XtMalloc((unsigned) sizeof(XmTextSourceRec));
    data = source->data = (XmSourceData)
	XtMalloc((unsigned) sizeof(XmSourceDataRec));
    source->AddWidget = AddWidget;
    source->CountLines = CountLines;
    source->RemoveWidget = RemoveWidget;
    source->ReadSource = ReadSource;
    source->Replace = Replace;
    source->Scan = (ScanProc)Scan;
    source->GetSelection = GetSelection;
    source->SetSelection = SetSelection;


    data->source = source;
    data->length = strlen(value);
    data->maxlength = data->length + 100;
    data->old_length = 0;
    data->ptr = XtMalloc((unsigned)data->maxlength);
    data->value = NULL;
    bcopy(value, data->ptr, data->length);
    data->numwidgets = 0;
    data->widgets = (XmTextWidget *) XtMalloc((unsigned) sizeof(XmTextWidget));
    data->hasselection = FALSE;
    data->left = data->right = 0;
    data->editable = TRUE;
    data->maxallowed = MAXINT;
    data->gap_start = data->ptr + data->length;
    data->gap_end = data->ptr + data->maxlength;
    data->prim_time = 0;
#ifdef DEC_MOTIF_EXTENSION
    data->opaque1 = data->opaque2 = -1;
#endif
    return source;
}


#ifdef _NO_PROTO
void _XmStringSourceDestroy(source)
XmTextSource source;
#else /* _NO_PROTO */
void _XmStringSourceDestroy (XmTextSource source)
#endif /* _NO_PROTO */
{
    XtFree((char *) source->data->ptr);
    XtFree((char *) source->data->value);
    XtFree((char *) source->data->widgets);
    XtFree((char *) source->data);
    XtFree((char *) source);
    source = NULL;
}


#ifdef _NO_PROTO
char *_XmStringSourceGetValue(source)
XmTextSource source;
#else /* _NO_PROTO */
char *_XmStringSourceGetValue(XmTextSource source)
#endif /* _NO_PROTO */
{
    XmSourceData data = source->data;
    XmTextBlockRec block;
    int length = 0;
    XmTextPosition pos = 0;
    XmTextPosition last_pos = 0;

    if (data->length > 0) {
       if (data->old_length == 0) {
	  data->value = (char *)XtMalloc(data->length + 1);
          data->old_length = data->length;
       } else if (data->length != data->old_length) {
	  data->value = XtRealloc(data->value, (unsigned) (data->length + 1));
          data->old_length = data->length;
       }
    } else
       return(XtNewString(""));

    last_pos = (XmTextPosition) data->length;
    while (pos < last_pos) {
         pos = ReadSource(source, pos, last_pos, &block);
         if (block.length == 0)
	    break;

         strncpy(&data->value[length], block.ptr, block.length);
         length += block.length;
    }
    data->value[length] = 0;
    return (XtNewString(data->value));
}


#ifdef _NO_PROTO
void _XmStringSourceSetValue(source, value)
XmTextSource source;
char *value;
#else /* _NO_PROTO */
void _XmStringSourceSetValue (XmTextSource source, char *value)
#endif /* _NO_PROTO */
{
    XmSourceData data = source->data;
    Boolean editable;
    int maxallowed;
    XmTextBlockRec block;
    SetSelection(source, 1, 0, CurrentTime);
    block.format = FMT8BIT;
    block.length = strlen(value);
    block.ptr = value;
    editable = data->editable;
    maxallowed = data->maxallowed;
    data->editable = TRUE;
    data->maxallowed = MAXINT;
    (void) (*source->Replace)(source, NULL, 0, data->length, &block);
    data->editable = editable;
    data->maxallowed = maxallowed;
}

#ifdef _NO_PROTO
Boolean _XmStringSourceHasSelection(source)
XmTextSource source;
#else /* _NO_PROTO */
Boolean _XmStringSourceHasSelection (XmTextSource source)
#endif /* _NO_PROTO */
{
   return source->data->hasselection;
}


#ifdef _NO_PROTO
Boolean _XmStringSourceGetEditable(source)
XmTextSource source;
#else /* _NO_PROTO */
Boolean _XmStringSourceGetEditable (XmTextSource source)
#endif /* _NO_PROTO */
{
    return source->data->editable;
}


#ifdef _NO_PROTO
void _XmStringSourceSetEditable(source, editable)
XmTextSource source;
Boolean editable;
#else /* _NO_PROTO */
void _XmStringSourceSetEditable (XmTextSource source, Boolean editable)
#endif /* _NO_PROTO */
{
    source->data->editable = editable;
}



#ifdef _NO_PROTO
int _XmStringSourceGetMaxLength(source)
XmTextSource source;
#else /* _NO_PROTO */
int _XmStringSourceGetMaxLength (XmTextSource source)
#endif /* _NO_PROTO */
{
    return source->data->maxallowed;
}



#ifdef _NO_PROTO
void _XmStringSourceSetMaxLength(source, max)
XmTextSource source;
int max;
#else /* _NO_PROTO */
void _XmStringSourceSetMaxLength (XmTextSource source, int max)
#endif /* _NO_PROTO */
{
    source->data->maxallowed = max;
}
