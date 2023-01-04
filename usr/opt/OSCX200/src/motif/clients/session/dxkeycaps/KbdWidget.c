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

#include "KbdWidgetP.h"
#include "KeyWidgetP.h"
#include "all-kbds.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

static void KbdResize();
static void KbdRealize();
static void KbdInitialize();
static void ChangeManaged();
static void SetValues();
static XtGeometryResult GeometryManager ();
static XtGeometryResult PreferredSize ();
static void make_key_widgets ();
static void place_keys ();
static struct keyboard *choose_kbd ();

extern void init_modifier_mapping ();


static XtResource keyboard_resources [] = {
  { "keyboard", "Keyboard", XtRString, sizeof (String),
      XtOffset (KeyboardWidget, keyboard.kbd_name), XtRString, "" },
  { "selectCursor", "Cursor", XtRCursor, sizeof (Cursor),
      XtOffset (KeyboardWidget, keyboard.select_cursor), XtRString,
      "crosshair" }
};

extern void highlight_key_action ();
extern void unhighlight_key_action ();
extern void toggle_key_action ();
extern void simulate_KeyPress_action ();
extern void simulate_KeyRelease_action ();
extern void track_key_action ();
extern void untrack_key_action ();
extern void describe_key_action ();
extern void popup_menu_action ();
extern void keyboard_track_motion_hook ();
extern void key_menu_popup_action ();

XtActionsRec keyboard_actions [] = {
  {"HighlightKey",	highlight_key_action},
  {"UnhighlightKey",	unhighlight_key_action},
  {"ToggleKey",		toggle_key_action},
  {"SimulateKeyPress",	simulate_KeyPress_action},
  {"SimulateKeyRelease",simulate_KeyRelease_action},
  {"TrackKey",		track_key_action},
  {"UntrackKey",	untrack_key_action},
  {"DescribeKey",	describe_key_action},
  {"PopupMenu",         popup_menu_action},
};

char keyboard_default_translations[] = "\
<Motion>:	DescribeKey(mouse, unlessTracking)	\n\
\
<KeyDown>F4:	PopupMenu()	                        \n\
\
<KeyDown>F10, <KeyUp>F10:	PopupMenu()	        \n\
\
!Ctrl<KeyDown>F4, !Ctrl<KeyUp>F4:  HighlightKey()	\
		                 DescribeKey(unlessMod)	\
		                 DescribeKey(displayed)	\
		                 SimulateKeyPress()	\
                                 UnhighlightKey()	\
                                 SimulateKeyRelease()	\n\
\
<KeyDown>Return, <KeyUp>Return:  HighlightKey()		\
		                 DescribeKey(unlessMod)	\
		                 DescribeKey(displayed)	\
		                 SimulateKeyPress()	\
                                 UnhighlightKey()	\
                                 SimulateKeyRelease()	\n\
\
<KeyDown>, <KeyUp>:	HighlightKey()			\
                        DescribeKey()	        \
		        DescribeKey(displayed)		\
		        SimulateKeyPress()		\
                        UnhighlightKey()		\
                        SimulateKeyRelease()		\n\
\
<Btn1Down>:	HighlightKey(unlessMod)			\
		ToggleKey(ifMod)			\
		TrackKey(unlessMod)			\
		SimulateKeyPress(ifHighlighted)		\
		SimulateKeyRelease(unlessHighlighted)	\n\
\
<Btn1Up>:	UntrackKey(highlighted)			\
		SimulateKeyRelease(highlighted, unlessMod) \
		UnhighlightKey(highlighted, unlessMod)	\n\
\
<Btn3Down>:	PopupMenu()			\n\
";

KeyboardClassRec keyboardClassRec = {
    { /*
       *	core_class fields
       */
    /* superclass		*/	(WidgetClass)&compositeClassRec,
    /* class_name		*/	"Keyboard",
    /* widget_size		*/	sizeof (KeyboardRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	KbdInitialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	KbdRealize,
    /* actions			*/	keyboard_actions,
    /* num_actions		*/	XtNumber (keyboard_actions),
    /* resources		*/	keyboard_resources,
    /* resource_count		*/	XtNumber (keyboard_resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	KbdResize,
    /* expose			*/	NULL,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	keyboard_default_translations,
    /* query_geometry		*/	PreferredSize,
    /* display_accelerator	*/	NULL,
    /* extension		*/	NULL
    },
    { /*
       *	composite_class fields
       */
    /* geometry_manager		*/	GeometryManager,
    /* change_managed		*/	ChangeManaged,
    /* insert_child		*/	XtInheritInsertChild,
    /* delete_child		*/	XtInheritDeleteChild,
    /* extension		*/	NULL
    }
};

WidgetClass keyboardWidgetClass = (WidgetClass) &keyboardClassRec;
/*
**      make these visible to all interested parties -bg
*/
char	kbInter = ~0;
int	kbType = -1;

static void 
KbdInitialize (request, new)
     KeyboardWidget request, new;
{
  struct keyboard *kbd = choose_kbd (XtDisplay (new), new->keyboard.kbd_name);
  new->keyboard.kbd = kbd;
  new->keyboard.key_under_mouse = 0;
  new->keyboard.mouse_highlighted_key = 0;
  new->keyboard.documented_key = 0;
  new->keyboard.tracking_key = 0;
  new->keyboard.target_window = 0;
  make_key_widgets (new);
  place_keys (new);
  init_modifier_mapping (new);
}


static void KbdResize (w)
     KeyboardWidget w;
{
    do_layout (w);
}


do_layout (parent)
     KeyboardWidget parent;
{
  struct keyboard *kbd = parent->keyboard.kbd;

  int i, j;
  int width = parent->core.width;
  int height = parent->core.height;
  int max_width = parent->keyboard.max_width + (kbd->horiz_border * 2);
  int max_height = parent->keyboard.max_height + (kbd->vert_border * 2);
  float x_scale = (float) width / (float) max_width;
  float y_scale = (float) height / (float) max_height;
  int x_off, y_off;
  KeySym  *keysyms;
  int     count;


  if (x_scale < 1) x_scale = 1;
  if (y_scale < 1) y_scale = 1;

  /* Be square */
  if (x_scale < y_scale) y_scale = x_scale;
  else if (y_scale < x_scale) x_scale = y_scale;

  x_off = ((((float) width) - (max_width * x_scale)) / 2
	   + kbd->horiz_border * x_scale);
  y_off = ((((float) height) - (max_height * y_scale)) / 2
	   + kbd->vert_border * y_scale);
    
  if (XtWindow (parent))
    XUnmapSubwindows (XtDisplay (parent), XtWindow (parent)); /* sleazy */

  for (i = 0; i < kbd->nrows; i++)
    {
      for (j = 0; j < kbd->rows[i].nkeys; j++)
	{
	  int off;
	  struct key *key = &kbd->rows[i].keys[j];
	  KeyWidget child = (KeyWidget) key->widget;
	  if (! child) continue;
	  off = child->core.border_width * 2 + child->key.gutter_width;
        /*
        **      build the list of keycaps based on the server's keymap,
        **      not on the table wired into all-kbds.h...-bg
        */
        if ((key->keycode) 
			&& (keysyms = XGetKeyboardMapping (XtDisplay(parent), 
											   key->keycode, 1,
											   &count)))
		{
                makeKeyCap (kbd, key, keysyms);
                make_key_name (child);
                free (keysyms);
		}

	  XtMoveWidget ((Widget) child,
			(int) (x_off + child->key.x * x_scale),
			(int) (y_off + child->key.y * y_scale));
	  XtResizeWidget ((Widget) child,
			  MAX (1, (int) (key->width * x_scale - off)),
			  MAX (1, (int) (key->height * y_scale - off)),
			  child->core.border_width);
	}
    }

  if (XtWindow (parent))
    XMapSubwindows (XtDisplay (parent), XtWindow (parent)); /* yzaels */

  parent->keyboard.x_scale = x_scale;
  parent->keyboard.y_scale = y_scale;
}


static XtGeometryResult
PreferredSize (w, request, preferred)
     KeyboardWidget w;
     XtWidgetGeometry *request, *preferred;
{
  return XtGeometryYes;
}

static XtGeometryResult
GeometryManager (w, request, reply)
     KeyboardWidget w;
     XtWidgetGeometry *request, *reply;
{
  return XtGeometryNo;
}


static void ChangeManaged (w)
     KeyboardWidget w;
{
    if (w->core.width <= 0 || w->core.height <= 0)
      {
	int default_scale = w->keyboard.kbd->default_scale;
	int horiz = (w->keyboard.kbd->horiz_border * 2) + 1;
	int vert = (w->keyboard.kbd->vert_border * 2) + 1;
	w->core.width  = (w->keyboard.max_width + horiz) * default_scale;
	w->core.height = (w->keyboard.max_height + vert) * default_scale;
        /*
        **      Make sure we're scaling to within the bounds of the root
        **      window. Don't take more than 7/8ths with width (and/or height)
        **      of the display. On low-resolution displays (VGA class), the
        **      keyboard drawing goes off the screen without this.
        */
        {
                Screen  *pscr = w->core.screen;
                int             temporary;

                temporary = pscr->width - (pscr->width >> 3);
                if (temporary < w->core.width)
		{
                        w->core.width = temporary;
		}
                temporary = pscr->height - (pscr->height >> 3);
                if (temporary < w->core.height)
		{
                       w->core.height = temporary;
  	        }
	}

      }
    do_layout (w);
}


static void
KbdRealize (widget, value_mask, attributes)
    Widget widget;
    Mask *value_mask;
    XSetWindowAttributes *attributes;
{
  XtAppContext app = XtWidgetToApplicationContext (widget);
  XtAppAddActionHook (app, keyboard_track_motion_hook, widget);
  if (widget->core.width == 0) widget->core.width = 10;
  if (widget->core.height == 0) widget->core.height = 10;
  XtCreateWindow (widget, (unsigned int) InputOutput,
		  (Visual *) CopyFromParent, *value_mask, attributes);
}



static void
print_kbd_choices ()
{
  int i, row, rows, cols, count = 0;
  char  fmtstr[80]; /* the actual format string */
  char  numstr[20]; /* max size here */

  i = 0;
  while (all_kbds [count])
  {
    row = strlen (all_kbds[count]->name);
    if (row > i) i = row;   /* get length of longest */
    count++;
  }
  /*
  **  with length of longest name, get us a nicely formated
  **  column of this width + 1, with the names left justified
  */
  strcpy (fmtstr, "%-");        /* left-justify, pad with spaces */
  sprintf (numstr, "%ds ", i);  /* (this many spaces) */
  strcat (fmtstr, numstr);      /* something like  "%-21s  " */
  cols = 3;
  rows = count / cols + (count % cols != 0);
  for (row = 0; row < rows; row++)
  {
      for (i = row; i < count; i += rows)
      {
        fprintf (stderr, fmtstr, all_kbds [i]->name);
      }
      putc ('\n', stderr);
  }
  putc ('\n', stderr);
}


#define strprefix(str1,str2)				\
      ((strlen ((str1)) >= strlen ((str2))) &&		\
       !(strncmp ((str1), (str2), strlen ((str2)))))

char *correctNames[] =
{
    "PCXAL (American)",
    "LK201 (American)",
    "LK401 (American)",
    "LK421",
    "LK443 (American)",
    "LK444 (American)",
    0           /* need this terminator!! */
};

char *knownNames[] =
{
    "PCXAL",    /* 0 */
    "LK201",    /* 1 */
    "LK401",    /* 2 */
    "LK421",    /* 3 */
    "LK443",    /* 4 */
    "LK444",    /* 5 */
    0           /* need this terminator!! */
};

static int strn_equal (s1, s2, len)
char *s1, *s2;
int len;
{
    if (s1 == s2) return 1;
    if (!s1 || !s2) return 0;
    while (*s1 && *s2 && len)
    {
        if (_tolower (*s1) != _tolower(*s2)) return 0;
        s1++, s2++; len--;
    }
    if (! len) return 1;
    return ((*s1 || *s2) ? 0 : 1);
}

static struct keyboard *
choose_kbd (dpy, kbd_name)
     Display *dpy;
     char *kbd_name;
{
    extern char getInternatKB ();
    extern int test_pckb_type();
    extern int runningLocal (Display *);
    extern int      baseKeyboards;  /* count base (i.e. not internationals) */
    extern int      internatFlag;
    extern int      iCount;
	char *nameSpecified = kbd_name; /* NULL means take default keyboard */
  /*
  **    Now there is a better way to determine the
  **    default keyboard. The functions we use live
  **    in commands.c. they use new getsysinfo() 
  **	functionality. -bg
  */
  if (!kbd_name || !*kbd_name)
    {
      char *vendor = XServerVendor (dpy);
#if 0
      if (strprefix (vendor,
		     "DECWINDOWS Digital Equipment Corporation OSF/1"))
	kbd_name = "LK401";
      else if (strprefix (vendor, "DECWINDOWS DigitalEquipmentCorporation"))
	kbd_name = "LK201";
#endif
      if (! strcmp (vendor, "Network Computing Devices Inc."))
        kbd_name = "NCD";
      else if (!strcmp (vendor,
	  "DECWINDOWS (Compatibility String) Network Computing Devices Inc."))
	kbd_name = "NCD N97";
      else if (!strcmp (vendor, "X11/NeWS - Sun Microsystems Inc."))
	kbd_name = "Sun type4";
      else if (!strcmp (vendor, "Texas Instruments Explorer"))
	kbd_name = "TI Explorer";
      else if (!strcmp (vendor, "International Business Machines"))
        kbd_name = "IBM RS/6000";
      else if (strprefix (vendor, "AGE Logic, Inc.")) /* Xstation 130 */
        kbd_name = "IBM RS/6000";
      else if (strprefix (vendor, "Hewlett-Packard Company"))
	kbd_name = "HP 720";
      else if (strprefix (vendor, "The Santa Cruz Operation"))
	kbd_name = "SCO-110";
      else
	kbd_name = 0;

      if (kbd_name)
	{
	  fprintf (stderr,
		   "%s: No keyboard type specified.  Defaulting to %s\n",
		   progname, kbd_name);
	}
/*
**	We sense what keyboard(s) we're using, and set up our list
**	accordingly, but we don't necessarily know what a valid DEFAULT
**	will be. If we're running locally (so the new getsysinfo()
**	calls get the right information) try to determine a probably
**	reasonable default keyboard. Note that, in order to use the
**	getsysinfo() calls, we need to be running on the same machine
**	that is serving as the display.
*/
	else
	{
        if (runningLocal (dpy)) /* function in commands.c */
        {
            /*
            **  negative return means don't know.
            **  else 0 means PC keyboard
            **  elses:
            **      1 means LK201 keyboard
            **      2 means LK401
            **      3 means LK421
            **      4 means LK443
            **      5 means LK444
            **          NOTE: the kernel can't distinguish between
            **      LK443 and LK444, so it will NEVER say it's a
            **      444. We check the console's language parameter
            **      to see if it's set to something other than the
            **      default/US keyboard, and use that to justify
            **      the assumption that it's an LK444 for the
            **      international set. -bg
            */
            kbType = test_pckb_type (); /* function in commands.c */
            /*
            **  -1 means no info available.
            **  0 means use default.
            **  else we have character code from
            **      /usr/lib/X11/xdm/Xkeymaps
            */
            kbInter = getInternatKB();  /* function in commands.c */
            if (kbType == 0 || kbType == 4) /* PC or LK443 */
            {
                /*
                **  BEWARE! This code assumes the
                **  struct keyboard *pc102s
                **  array is ordered like this:
                **  struct keyboard *pc102s [] = {
                **   ...
                **    &denmark,            0 DANSK -- danish
                **    &germany,            1 DEUTSCH -- austrian-german
                **    &swiss_german,       2 SCHWEIZ -- swiss-german
                **    &uk_at,              3 BRITISH -- uk
                **    &spain,              4 ESPANOL -- spanish
                **    &france,             5 FRANCAIS -- french
                **    &french_canadian,    6 CANADIEN -- french-canadian
                **    &swiss_french,       7 SUISSEROMANDE -- swiss-french
                **    &italy,              8 ITALIANO -- italian
                **    &netherlands,        9 NEDERLANDS -- dutch
                **    &norway,             10 NORSK -- norwegian
                **    &portugal,           11 PORTUGUES -- portguese
                **    &finnish,            12 SUOMI -- finland
                **    &sweden,             13 SVENSKA  -- swedish
                **    &belgian,            14 VLAAMS -- flemish
                **  ...
                **  This is the order used to create the all_kbds[]
                **  pointer array which is assumed below.
                **  Any changes to the pc102s order in all-kbds.h
                **  require changes HERE, too.
                **      -bg
                */

                struct keyboard **pkb;

                pkb = &all_kbds[baseKeyboards];
                switch (kbInter)
                {
                    case VLAAMS: /* flemish/belgian */
                        if (internatFlag >= BELGIUM)
                            kbd_name = pkb[BELGIUM]->name;
                        break;
                    case DANSK: /* danish */
                        if (internatFlag >= DENMARK)
                            kbd_name = pkb[DENMARK]->name;
                        break;
                    case FRANCAIS_SUISSEROMANDE: /* swiss/french */
                        if (internatFlag >= SWISS_FRENCH)
                            kbd_name = pkb[SWISS_FRENCH]->name;
                        break;
                    case FRANCAIS_CANADIEN: /* french canadian */
                        if (internatFlag >= CANADIEN)
                            kbd_name = pkb[CANADIEN]->name;
                        break;
                    case FRANCAIS: /* french */
                        if (internatFlag >= FRANCE)
                            kbd_name = pkb[FRANCE]->name;
                        break;
                    case DEUTSCH: /* austrian/german */
                        if (internatFlag >= GERMANY)
                            kbd_name = pkb[GERMANY]->name;
                        break;
                    case ITALIANO: /* italian */
                        if (internatFlag >= ITALY)
                            kbd_name = pkb[ITALY]->name;
                        break;
                    case NORSK: /* norwegian */
                        if (internatFlag >= NORWAY)
                            kbd_name = pkb[NORWAY]->name;
                        break;
                    case PORTUGUES: /* portuguese */
                        if (internatFlag >= PORTUGAL)
                            kbd_name = pkb[PORTUGAL]->name;
                        break;
                    case ESPANOL: /* Spanish */
                        if (internatFlag >= SPAIN)
                            kbd_name = pkb[SPAIN]->name;
                        break;
                    case SVENSKA: /* swedish */
                        if (internatFlag >= SWEDEN)
                            kbd_name = pkb[SWEDEN]->name;
                        break;
                    case SUOMI: /* finnish */
                        if (internatFlag >= FINNISH)
                            kbd_name = pkb[FINNISH]->name;
                        break;
                    case DEUTSCH_SCHWEIZ: /* swiss/german*/
                        if (internatFlag >= SWISS_GERMAN)
                            kbd_name = pkb[SWISS_GERMAN]->name;
                        break;
                    case BRITISH_IRISH: /* UK */
                        if (internatFlag >= UK)
                            kbd_name = pkb[UK]->name;
                        break;
                    case NEDERLANDS: /* dutch */
                        if (internatFlag >= DUTCH)
                            kbd_name = pkb[DUTCH]->name;
                        break;
                    case AMERICAN: /* US */
                    default:
                        if (kbType >= 0 && kbType < 6)
                            kbd_name = correctNames [kbType];
                        break;
                }
            }
            else if (kbType >= 0 && kbType < 6)
                kbd_name = correctNames [kbType];
            /*
            **  otherwise default to first name in the list
            */
            else kbd_name = all_kbds[0]->name;
        }/* otherwise running over the net, so wing it... */
        else kbd_name = all_kbds[0]->name;
      	if (kbd_name)
        {
          fprintf (stderr,
               "\n\%s: No keyboard specified.  Defaulting to %s\n",
               progname, kbd_name);
        }
	}
    }
/*
**	come here when he have a keyboard name (possibly from 
**	incoming parameters)
*/
  {
    int i = 0;
    struct keyboard *kbd;

    if (kbd_name)
    {
        int lenin;
	/*
	**	First check for an exact match
	*/
        for (i = 0; kbd = all_kbds [i]; i++)
        {
            if (string_equal (kbd_name, kbd->name)) return kbd;
        }
	/*
	**	now check for fuzzy match
	*/
        lenin = strlen (kbd_name);
        for (i = 0; kbd = all_kbds [i]; i++)
        {
            if (strn_equal (kbd_name, kbd->name, lenin)) return kbd;
        }
    }
	if (nameSpecified)	   		/* keyboard specified but not found */
	{
	  fprintf (stderr, "%s: unknown keyboard type \"%s\".\n\
Selecting default keyboard from among the following:\n", 
			   progname, nameSpecified);
	  print_kbd_choices ();
      return (choose_kbd (dpy, (char *) 0)); /* try selecting default */
    }
    if (kbd_name)				/* OOPS! The default selection failed! */
      fprintf (stderr, "%s: unknown keyboard type \"%s\".\n\
Please specify the -keyboard option with one of the following names:\n\n",
	       progname, kbd_name);
    else
	{
      fprintf (stderr,
	   "%s: please specify -keyboard or -kbd with one of the following names:\n\n",
	       progname);
    }
	print_kbd_choices ();
	exit (1);
  }
}



static void
place_keys (widget)
     KeyboardWidget widget;
{
  int i, j, k;
  int x = 0;
  int y = 0;
  int max_syms = 0;
  struct keyboard *kbd = widget->keyboard.kbd;
  widget->keyboard.max_width = x;
  widget->keyboard.max_height = y;
  for (i = 0; i < kbd->nrows; i++)
    {
      struct row *row = &kbd->rows [i];
      for (j = 0; j < row->nkeys; j++)
	{
	  struct key *key = &row->keys [j];
	  if (key->widget)
	    {
	      key->widget->key.x = x;
	      key->widget->key.y = y;
	    }
	  x += key->width;
	  for (k = 7; k && !key->default_keysyms [k]; k--) ;
	  if (k > max_syms) max_syms = k;
	}
      if (x > widget->keyboard.max_width)
	widget->keyboard.max_width = x;
      x = 0;
      y += row->height;
      if (y > widget->keyboard.max_height)
	widget->keyboard.max_height = y;
    }
  widget->keyboard.default_keysyms_per_code = max_syms + 1;
}


static void
make_key_widgets (widget)
     KeyboardWidget widget;
{
  Display *dpy = XtDisplay (widget);
  struct keyboard *kbd = widget->keyboard.kbd;
  Arg av [20];
  int ac;
  int i, j;
  int default_scale = widget->keyboard.kbd->default_scale;
  int pixel_width = (widget->keyboard.max_width + 1) * default_scale;
  int pixel_height = (widget->keyboard.max_height + 1) * default_scale;
  widget->keyboard.x_scale = widget->keyboard.y_scale = default_scale;
  ac = 0;
  XtSetArg (av[ac], XtNwidth, 10); ac++;
  XtSetArg (av[ac], XtNheight, 10); ac++;

  for (i = 0; i < kbd->nrows; i++)
    for (j = 0; j < kbd->rows[i].nkeys; j++)
      {
	struct key *key = &kbd->rows[i].keys[j];
	if (!key->top_keysym && !key->bottom_keysym && !key->keycode)
	  key->widget = None;
	else
	  {
	    key->widget = (KeyWidget)
	      XtCreateManagedWidget (key->top_keysym, keyWidgetClass,
				     (Widget) widget, av, ac);
	    ((KeyWidget) key->widget)->key.key = key;
	  }
      }
}
#define IS_TOP	1
#define IS_BOTTOM	0
/*
**	This function is part of the manufacturing process for keycaps.
**	It burns in the requested data (string), either to the top (flag == 1)
**	or to the bottom (flag == 0) portion of the keycap widget
*/
static setKeyCap (struct key *pkey, char *string, int flag)
{
	char	*dest;
	int		destlen;
	int		newlen;

	if (! string) 
	{
		if (flag == IS_TOP) pkey->top_keysym = (char *) 0;
		else pkey->bottom_keysym = (char *) 0;
		return;
	}
	if (flag == IS_TOP)
		dest = pkey->top_keysym;
	else if (flag == IS_BOTTOM) dest = pkey->bottom_keysym;
	else dest = (char *) 0;
	if (dest) destlen = strlen (dest);
	else destlen = 0;
	newlen = strlen (string);
	if (newlen < destlen) strcpy (dest, string);
	else
	{
		dest = (char *) (char *) malloc (strlen (string) + 1);
		strcpy (dest, string);
		if (flag == IS_TOP) pkey->top_keysym = dest;
		else pkey->bottom_keysym = dest;
	}
}
/*
**	This function manufactures a new keycap for the struct key entry
**	based on the keysyms passed to it. This makes it so we can 
**	repaint the keyboard showing the keycaps as the server has them
**	mapped, not as the initial data table defines them.
*/
makeKeyCap (struct keyboard *kbd, struct key *pkey, KeySym *keysyms)
{
	char	*forShift;
	char	*forUnshift;
	KeySym	shiftCode;
	KeySym	unshiftCode;
	int		ix;
	char	tempBuf[80];
	KeySym  zero, one;
	int     specialFlag;
	
	/*
	 * some of the European keyboards have definitions 
	 * that cause BOTH upper and lower case keys to
	 * turn up on the keycaps. We don't want this: the
	 * physical keyboard only shows the Shifted character
	 * in such cases. Check to see if this keysym is one 
	 * of those cases, and fix it if it is. So-o-o-o, if 
	 * both keysyms are ascii and both are the same except 
	 * for case distinction, this keycap gets special 
	 * attention -bg
	 */
	zero = keysyms[0];
	one = keysyms[1];
	specialFlag = ((isascii(zero) && isascii(one)) 
				   && ((toupper(zero) == toupper(one))));
	unshiftCode = 0;
	for (ix = 0; ix <= 1; ix++)
	{
		if (shiftCode = keysyms[ix])
		{
			if (! unshiftCode) unshiftCode = shiftCode;
			else if (unshiftCode == shiftCode) return;
			/*
			 * special case? make it pretty... -bg
			 */
			if (specialFlag) 
			  unshiftCode = shiftCode = toupper(shiftCode);
			if (! (shiftCode & ~0xfff))
			{
				tempBuf[0] = shiftCode & 0xff;
				tempBuf[1] = 0;
				setKeyCap (pkey, tempBuf, ix);
			}
			else
			{
				switch (shiftCode)
				{
					case DXK_Remove:
                        if ((!pkey->bottom_keysym)
                            || (!pkey->top_keysym) )
						{
							setKeyCap (pkey, "Re-", 1);
							setKeyCap (pkey, "move", 0);
							return;
						}
                        else forShift = "Remove";
						break;
					case XK_Delete:
					{
						int	temp;

						temp = isLK (kbd);
						if (temp == KB_PCXAL || temp == KB_LK443
							|| temp == KB_LK444) forShift = "Delete";
						else forShift = "<X|";
						break;
					}
					case XK_Multi_key:
                        if ((!pkey->bottom_keysym)
                            || (!pkey->top_keysym) )
                        {
                            setKeyCap (pkey, "Compose", 1);
                            setKeyCap (pkey, "Character", 0);
                            return;
                        }
                        else forShift = "Compose";
						break;
					case XK_KP_F1:
						forShift = "PF1";
						break;
					case XK_KP_F2:
						forShift = "PF2";
						break;
					case XK_KP_F3:
						forShift = "PF3";
						break;
					case XK_KP_F4:
						forShift = "PF4";
						break;
					case XK_KP_1:
						if (isLK (kbd)) forShift = "1";
						else
						{
							setKeyCap (pkey, "1", 1);
							setKeyCap (pkey, "End", 0);
							return;
						}
						break;
					case XK_KP_2:
						if (isLK (kbd)) forShift = "2";
						else
						{
							setKeyCap (pkey, "2", 1);
							setKeyCap (pkey, "downArrow", 0);
							return;
						}
						break;
					case XK_KP_3:
						if (isLK (kbd)) forShift = "3";
						else
						{
							setKeyCap (pkey, "3", 1);
							setKeyCap (pkey, "PgDn", 0);
							return;
						}
						break;
					case XK_KP_4:
						if (isLK (kbd)) forShift = "4";
						else
						{
							setKeyCap (pkey, "4", 1);
							setKeyCap (pkey, "leftArrow", 0);
							return;
						}
						break;
					case XK_KP_5:
						forShift = "5";
						break;
					case XK_KP_6:
						if (isLK (kbd)) forShift = "6";
						else
						{
							setKeyCap (pkey, "6", 1);
							setKeyCap (pkey, "RightArrow", 0);
							return;
						}
						break;
					case XK_KP_7:
						if (isLK (kbd)) forShift = "7";
						else
						{
							setKeyCap (pkey, "7", 1);
							setKeyCap (pkey, "Home", 0);
							return;
						}
						break;
					case XK_KP_8:
						if (isLK (kbd)) forShift = "8";
						else
						{
							setKeyCap (pkey, "8", 1);
							setKeyCap (pkey, "UpArrow", 0);
							return;
						}
						break;
					case XK_KP_9:
						if (isLK (kbd)) forShift = "9";
						else
						{
							setKeyCap (pkey, "9", 1);
							setKeyCap (pkey, "PgUp", 0);
							return;
						}
						break;
					case XK_KP_0:
						if (isLK (kbd)) forShift = "0";
						else
						{
							setKeyCap (pkey, "0", 1);
							setKeyCap (pkey, "Ins", 0);
							return;
						}
						break;
					case XK_Escape:
						forShift = "Esc";
						break;
					case XK_F1:
					case XK_F2:
					case XK_F3:
					case XK_F4:
					case XK_F5:
					case XK_F6:
					case XK_F7:
					case XK_F8:
					case XK_F9:
					case XK_F10:
					case XK_F11:
					case XK_F12:
					case XK_F13:
					case XK_F14:
					case XK_F15:
					case XK_F16:
					case XK_F17:
					case XK_F18:
					case XK_F19:
					case XK_F20:
					case XK_F21:
					case XK_F22:
					case XK_F23:
					case XK_F24:
					case XK_F25:
					case XK_F26:
					case XK_F27:
					case XK_F28:
					case XK_F29:
					case XK_F30:
						forShift = XKeysymToString (shiftCode); /* "F1";*/
						break;
					case XK_Select:
						forShift = "Select";
						break;
					case XK_Find:
						forShift = "Find";
						break;
					case XK_Menu:
						forShift = "Do";
						break;
					case XK_Help:
						forShift = "Help";
						break;
					case XK_Print:
                        if ((!pkey->bottom_keysym)
                            || (!pkey->top_keysym))
                        {
                            setKeyCap (pkey, "Print", 1);
                            setKeyCap (pkey, "Screen", 0);
                            return;
                        }
                        else forShift = "PrtSc";
						break;
					case XK_Cancel:
						forShift = "Cancel";
						break;
					case XK_Scroll_Lock:
                        if ((!pkey->bottom_keysym)
                            || (!pkey->top_keysym))
                        {
                            setKeyCap (pkey, "Scroll", 1);
                            setKeyCap (pkey, "Lock", 0);
                            return;
                        }
                        else forShift = "ScrlLk";
						break;
					case XK_Clear:
						forShift = "Clear";
						break;
					case XK_Pause:
						forShift = "Pause";
						break;
					case XK_BackSpace:
						forShift = "Backspace";
						break;
					case XK_Insert:
						if (isLK (kbd))
						{
							setKeyCap (pkey, "Insert", 1);
							setKeyCap (pkey, "Here", 0);
							return;
						}
						else forShift = "Insert";
						break;
					case XK_Home:
						forShift = "Home";
						break;
					case XK_Prior:
						if (isLK (kbd))
						{
							setKeyCap (pkey, "Prev", 1);
							setKeyCap (pkey, "Screen", 0);
							return;
						}
						else
						{
							setKeyCap (pkey, "Page", 1);
							setKeyCap (pkey, "Up", 0);
							return;
						}
						break;
					case XK_Num_Lock:
                        if ((!pkey->bottom_keysym)
                            || (!pkey->top_keysym))
                        {
                                setKeyCap (pkey, "Num", 1);
                                setKeyCap (pkey, "Lock", 0);
                                return;
                        }
                        else forShift = "NumLk";
						break;
					case XK_Tab:
						forShift = "Tab";
						break;
					case XK_Redo:
						forShift = "Redo";
						break;
					case XK_Execute:
						forShift = "Execute";
						break;
					case XK_Undo:
						forShift = "Undo";
						break;
					case XK_End:
						forShift = "End";
						break;
					case XK_Next:
						if (isLK (kbd))
						{
							setKeyCap (pkey, "Next", 1);
							setKeyCap (pkey, "Screen", 0);
							return;
						}
						else
						{
							setKeyCap (pkey, "Page", 1);
							setKeyCap (pkey, "Down", 0);
							return;
						}
						break;
					case XK_Caps_Lock:
						if (isLK (kbd)) forShift = "Lock";
						else
						{
							setKeyCap (pkey, "Caps", 1);
							setKeyCap (pkey, "Lock", 0);
							return;
						}
						break;
					case XK_Return:
						forShift = "Return";
						break;
					case XK_Shift_L:
					case XK_Shift_R:
						forShift = "Shift";
						break;
					case XK_Up:
						forShift = "UpArrow";
						break;
					case XK_KP_Divide:
						forShift = "/";
						break;
					case XK_KP_Space:
						forShift = "Space";
						break;
					case XK_KP_Separator:
						forShift = ",";
						break;
					case XK_KP_Tab:
						forShift = "Tab";
						break;
					case XK_KP_Add:
						forShift = "+";
						break;
					case XK_KP_Multiply:
						forShift = "*";
						break;
					case XK_KP_Decimal:
						forShift = ".";
						break;
					case XK_KP_Subtract:
						forShift = "-";
						break;
					case XK_KP_Enter:
						forShift = "Enter";
						break;
					case XK_Control_L:
					case XK_Control_R:
						forShift = "Ctrl";
						break;
					case XK_Alt_L:
						forShift = "Alt_L";
						break;
					case XK_Alt_R:
                        if (isLK (kbd)) forShift = "Alt_R";
                        else forShift = "Alt Gr";   /* PC - style */
						break;
					case XK_Begin:
						forShift = "Begin";
						break;
					case XK_Left:
						forShift = "LeftArrow";
						break;
					case XK_Down:
						forShift = "DownArrow";
						break;
					case XK_Right:
						forShift = "RightArrow";
						break;
					/*
					**	except for DXK_diaeresis, the special
					**	DXK_... symbols below are the same as
					**	the standard symbols, OR 0x1000ff00.
					**	but XKeysymToString returns a descriptive
					**	string rather than a single character.
					**	So here, we cheat, and display what
					**	we mean, instead of what we say. -bg
					*/
					case DXK_diaeresis:
						forShift = XKeysymToString (XK_diaeresis);
						break;
					case DXK_macron:
					case DXK_ring_accent:
					case DXK_circumflex_accent:
					case DXK_cedilla_accent:
					case DXK_acute_accent:
					case DXK_grave_accent:
					case DXK_tilde:
						forShift = XKeysymToString (shiftCode & 0xff);
						break;
					default:
						forShift = XKeysymToString (shiftCode);
						break;
				}
				setKeyCap (pkey, forShift, ix);
			}
		}
		else setKeyCap (pkey, (char *) 0, ix);
	}
}
