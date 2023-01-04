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


#ifndef _KbdWidgetP_H_
#define _KbdWidgetP_H_

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/cursorfont.h>
#include "KbdWidget.h"
#include "xkeycaps.h"

typedef struct {
     int mumble;   /* No new procedures */
} KeyboardClassPart;

/* Full class record declaration */
typedef struct _KeyboardClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    KeyboardClassPart	keyboard_class;
} KeyboardClassRec;

extern KeyboardClassRec keyboardClassRec;

/* New fields for the Keyboard widget record */
typedef struct {
  String kbd_name;
  Cursor select_cursor;
  struct keyboard *kbd;

  int max_width, max_height;	 /* In key units (computed) */
  int x_scale, y_scale;
  int default_keysyms_per_code;	/* computed... */
  char key_state_vector [32];	/* Bit-vector of keys down (redundant.) */
  char modifier_vector [32];	/* Bit-vector of modifier-key-p. */

  KeyWidget key_under_mouse;
  KeyWidget mouse_highlighted_key;	/* Key the mouse is pressed on */
  KeyWidget documented_key;		/* Key about which info is presented */
  unsigned int tracking_key;		/* Mask of buttons down */
  Window target_window;			/* window we're typing at */

  struct info_labels *label_widgets;	/* For info.c */
  struct key_menus *key_menus;		/* For commands.c */
} KeyboardPart;

typedef struct _KeyboardRec {		/* full instance record */
    CorePart		core;
    CompositePart	composite;
    KeyboardPart	keyboard;
} KeyboardRec;

#endif /* _KbdWidgetP_H_ */
