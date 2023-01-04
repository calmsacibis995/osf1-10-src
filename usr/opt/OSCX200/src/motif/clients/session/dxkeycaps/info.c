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
/* xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/BulletinB.h>

#include "xkeycaps.h"

#include "KbdWidgetP.h"
#include "KeyWidgetP.h"

struct info_labels {
  Widget info;
#ifdef MULTIPLE_WIDGETS
  Widget keycode [5];
  Widget keysym [9];
  Widget modifiers [2];
  Widget autorepeat [2];
#else
  Widget keycode;
  Widget keysym;
  Widget modifiers;
  Widget autorepeat;
#endif
  Widget message;
  Widget message2;
};

extern void key_to_event ();


Widget
make_label_1 (parent, name, string, left, top, class, callback, data)
     Widget parent;
     char *name, *string;
     Widget left, top;
     WidgetClass class;
     void (*callback) ();
     caddr_t data;
{
  Arg av [20];
  int ac = 0;
  Widget w;

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
  
  if (string) XtSetArg (av[ac], XmNlabelString, XmStringCreate(string,  XmSTRING_DEFAULT_CHARSET)), ac++;
  if (left)
    {
      XtSetArg (av[ac], XmNleftAttachment, XmATTACH_WIDGET), ac++;
      XtSetArg (av[ac], XmNleftWidget, left), ac++;
    }
  
  if (top)
    {
      XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET), ac++;
      XtSetArg (av[ac], XmNtopWidget, top), ac++;
    }

  w = XtCreateManagedWidget (name, class, parent, av, ac);
  
  if (callback) XtAddCallback (w, XmNactivateCallback, callback, data);
  
  return w;
}

Widget

make_label (parent, name, string, left, top)
     Widget parent;
     char *name, *string;
     Widget left, top;
{
  return make_label_1 (parent,name,string,left,top,xmLabelWidgetClass,0L,0L);
}

#ifdef MULTIPLE_WIDGETS

struct info_labels *

make_info_widget (parent)
     Widget parent;
{
    
  Widget info, left, farleft;
  Widget label_col, line_widget;
  struct info_labels *labels =
    (struct info_labels *) malloc (sizeof (struct info_labels));
  Arg av [10];
  int ac = 0;

  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;

  info = XtCreateManagedWidget ("info", xmFormWidgetClass, parent, av, ac);

  ac = 0;
  
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  label_col = XtCreateManagedWidget ("labels", xmBulletinBoardWidgetClass, info, av, ac);

  farleft = 0;

#define NEWLABEL(store, name,val) \
  { ac = 0; \
    XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++; \
    XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++; \
    XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++; \
    XtSetArg (av[ac], XmNorientation, XmHORIZONTAL); ac++; \
    XtSetArg (av[ac], XmNnumColumns, 7); ac++; \
    line_widget = XtCreateManagedWidget("line", xmRowColumnWidgetClass, info, av, ac); \
    farleft = make_label (label_col, (name),(val),0, farleft); \
    left = 0; \
    store = farleft; \
  }

#define NEWVALUE(store, name) \
  { left = make_label (line_widget, (name),"",0, 0); \
    store = left; \
  }

  NEWLABEL (labels->keycode [0], "label", "KeyCode:");
  NEWVALUE (labels->keycode [1], "keycode");
  NEWVALUE (labels->keycode [2], "keycode16");
  NEWVALUE (labels->keycode [3], "keycode10");
  NEWVALUE (labels->keycode [4], "keycode8");

  NEWLABEL (labels->keysym [0], "label", "KeySym:");
  NEWVALUE (labels->keysym [1], "keysym");
  NEWVALUE (labels->keysym [2], "keysym");
  NEWVALUE (labels->keysym [3], "keysym");
  NEWVALUE (labels->keysym [4], "keysym");
  NEWVALUE (labels->keysym [5], "keysym");
  NEWVALUE (labels->keysym [6], "keysym");
  NEWVALUE (labels->keysym [7], "keysym");
  NEWVALUE (labels->keysym [8], "keysym");

  NEWLABEL (labels->modifiers [0], "label", "Modifiers:");
  NEWVALUE (labels->modifiers [1], "modifiers              ");

  NEWLABEL (labels->autorepeat [0], "label", "AutoRepeat:");
  NEWVALUE (labels->autorepeat [1], "autoRepeat");
  
  labels->message = make_label (info, "message", "", info,
				labels->autorepeat [0]);
  labels->message2 = make_label (info, "message2", "", info,
				 labels->message);
#undef NEWLABEL
#undef NEWVALUE

  labels->info = info;
  return labels;
}

#else

struct info_labels *
make_info_widget (parent)
     Widget parent;
{
  Widget info, left, farleft;
  Widget label_col1, label_col2, line_widget;
  struct info_labels *labels =
    (struct info_labels *) malloc (sizeof (struct info_labels));
  Arg av [10];
  int ac = 0;
  char *LONGEST_KEYSYM_NAME = "                                                                                                                                                                                                                  ";
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;

  info = XtCreateManagedWidget ("info", xmFormWidgetClass, parent, av, ac);

  ac = 0;

  farleft = 0;
  line_widget = 0;

    /* For some odd reason, if I don't set recomputeSize, modifiers don't get
       displayed after resizes. And all 8 keysyms and 4 fields of keycodes don't
       display properly...Weirdness rules!
    */

#define NEWLABEL(name,val,store,lname) \
  { ac = 0; \
    XtSetArg (av[ac], XmNorientation, XmHORIZONTAL); ac++; \
    XtSetArg (av[ac], XmNpacking, XmPACK_TIGHT); ac++; \
    if (!line_widget) XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM), ac++;\
    if (line_widget)  XtSetArg(av[ac],XmNtopAttachment,XmATTACH_WIDGET), ac++;\
    if (line_widget)  XtSetArg(av[ac], XmNtopWidget, line_widget), ac++; \
    XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM), ac++; \
    XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM), ac++; \
    line_widget = XtCreateManagedWidget("line", xmRowColumnWidgetClass, \
					info, av, ac); \
    farleft = make_label (line_widget, (name),(val),0,0); \
    left =  make_label (line_widget, (lname), LONGEST_KEYSYM_NAME, 0, 0);\
    XtSetArg(av[0], XmNrecomputeSize, False);\
    XtSetValues(left, av, 1);\
    store = left; \
  }

  NEWLABEL ("keycode", "KeyCode:", labels->keycode, "keycode");

  NEWLABEL ("keysym", "KeySym:", labels->keysym, "keysym");

  NEWLABEL ("modifiers", "Modifiers:", labels->modifiers, "modifiers         ");

  NEWLABEL ("autoRepeat", "Auto Repeat:", labels->autorepeat, "autoRepeat");

  NEWLABEL ("message", "", labels->message, "message");

  NEWLABEL ("message2", "", labels->message2, "message2");

#undef NEWLABEL

  labels->info = info;
  return labels;
}

#endif


void message2 ();

void
message (widget, str)
     KeyboardWidget widget;
     char *str;
{
  Arg av[10];
  int ac = 0;
  XtSetArg (av [ac], XmNlabelString, XmStringCreate(str, XmSTRING_DEFAULT_CHARSET)); ac++;
  XtSetValues (widget->keyboard.label_widgets->message, av, ac);
  message2 (widget, "");
}

void
message2 (widget, str)
     KeyboardWidget widget;
     char *str;
{
  Arg av[10];
  int ac = 0;
  XtSetArg (av [ac], XmNlabelString, XmStringCreate(str, XmSTRING_DEFAULT_CHARSET)); ac++;
  XtSetValues (widget->keyboard.label_widgets->message2, av, ac);
}


int 
string_width (string, font)
     char *string;
     XFontStruct *font;
{
  int size = 0;
  for (; *string; string++)
    if (font->per_char)
      size += font->per_char [(*string) - font->min_char_or_byte2].width;
    else
      size += font->max_bounds.width;
  return size;
}


extern void key_to_event ();

static void
key_to_ascii (widget, string, pretty, keysym, font)
     KeyWidget widget;
     char *string, *pretty;
     KeySym *keysym;
     XFontStruct *font;
{
  XEvent event;
  char *p = pretty;
  int i, size;
  key_to_event (widget, &event, 1);
  size = XLookupString ((XKeyEvent *) &event, string, 50, keysym, 0);
  string [size] = 0;
  for (i = 0; i < size; i++)
    {
      unsigned char c = (unsigned char) string [i];
      unsigned char hic = 0;

      if (c >= 0200)
	{
	  *p++ = 'M';
	  *p++ = '-';
	  hic = c;
	  c -= 0200;
	}

      if (c < 040)
	{
	  *p++ = '^';
	  *p++ = c + ('A'-1);
	  if (! hic) hic = c;
	}
      else if (c == 0177)
	{
	  *p++ = '^';
	  *p++ = '?';
	  if (! hic) hic = c;
	}
      else
	*p++ = c;

      if (hic && font && size == 1 &&
	  hic != '\n' && hic != ' ' &&
	  hic >= font->min_char_or_byte2 &&
	  hic <= font->max_char_or_byte2 &&
	  (!font->per_char ||
	   font->per_char [hic - font->min_char_or_byte2].width))
	{
	  *p++ = ' ';
	  *p++ = '(';
	  *p++ = hic;
	  *p++ = ')';
	}
    }
  *p = 0;
}


void
describe_key (widget)
     KeyWidget widget;
{
  KeyboardWidget keyboard = (KeyboardWidget) widget->core.parent;
  struct info_labels *labels = keyboard->keyboard.label_widgets;
  Arg av [10];
  int i, ac = 0;
  KeySym keysym, *keysyms;
  int syms_per_code;

  char buf [255];
  char buf2 [255];
  char buf3 [255];
  char *b;
  Dimension wid, ht, x, y;


  /*
    XtSetArg(av[0], XmNwidth, &wid);\
    XtGetValues(label, av, 1);\
    fprintf(stderr, "%s and %d\n", string, wid);\
  */
  
#define SETLABEL(label,string) \
  XtSetArg (av[0], XmNlabelString, XmStringCreate(string, XmSTRING_DEFAULT_CHARSET)); \
  XtSetValues (label, av, 1);

  keysyms = 0;
  if (widget->key.key->keycode)
    keysyms = XGetKeyboardMapping (XtDisplay (widget),
				   widget->key.key->keycode,
				   1, &syms_per_code);
  if (! keysyms) syms_per_code = 0;

  sprintf (buf, "%s  0x%02X  %u  0%02o",
	   widget->key.key_name,
	   widget->key.key->keycode,
	   widget->key.key->keycode,
	   widget->key.key->keycode);
  SETLABEL (labels->keycode, buf);

  b = buf;
  *b = 0;
  for (i = 0; i < syms_per_code; i++)
    {
      char *b2 = XKeysymToString (keysyms [i]);
      int i;
      if (! b2) b2 = "NoSymbol";
      i = strlen (b2);
      if (b != buf) {
	b [0] = ' '; b++;
	b [0] = ' '; b++;
      }
      strncpy (b, b2, i+1);
      b += i;
    }
  SETLABEL (labels->keysym, buf);

sprintf (buf, "%s%s%s%s%s%s%s%s",
	 ((widget->key.modifier_bits & ShiftMask)   ? "  Shift        "   : ""),
	 ((widget->key.modifier_bits & LockMask)    ? "  Lock         "    : ""),
	 ((widget->key.modifier_bits & ControlMask) ? "  Control      " : ""),
	 ((widget->key.modifier_bits & Mod1Mask)    ? "  Mod1         "    : ""),
	 ((widget->key.modifier_bits & Mod2Mask)    ? "  Mod2         "    : ""),
	 ((widget->key.modifier_bits & Mod3Mask)    ? "  Mod3         "    : ""),
	 ((widget->key.modifier_bits & Mod4Mask)    ? "  Mod4         "    : ""),
	 ((widget->key.modifier_bits & Mod5Mask)    ? "  Mod5         "    : ""));

  /*
  XtSetArg(av[0], XmNwidth, &wid);
  XtSetArg(av[1], XmNheight, &ht);
  XtSetArg(av[2], XmNx, &x);
  XtSetArg(av[3], XmNy, &y);
  XtGetValues(labels->modifiers, av, 4);
  fprintf(stderr, "%d %d %d %d \n", wid, ht, x, y);
  */
  
  SETLABEL (labels->modifiers, buf);
  
  SETLABEL (labels->autorepeat, (widget->key.auto_repeat_p ? "On" : "Off"));

#undef SETLABEL

  if (keysyms) XFree ((char *) keysyms);

}
