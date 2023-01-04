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
#ifndef NO_PWD
#include <pwd.h>
#endif

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Command.h>
#include <Xm/ScrolledW.h>
#include <Xm/MessageB.h>

/* We need all this bullshit because the Toggle widget (really, the Command
   widget) doesn't provide a GetValues-hook to access the "state" property.
 */
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#include "KbdWidgetP.h"
#include "KeyWidgetP.h"

#include "xkeycaps.h"

#include "vroot.h"	/* This is standard in R5 but not R4 */
/*
**	include the following three for GSI_... defines
**	& getsysinfo() call stuff -bg
*/
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>
#include <sys/utsname.h>	/* uname test for local execution */
/*
**  if we're building ahead of the kernel changes,
**  add these getsysinfo() defines here. The code
**  that uses them tests for error return and
**  covers for the case of the kernel not yet
**  supporting these calls.
*/
#ifndef GSI_PROM_ENV
#define GSI_PROM_ENV	47
#endif
#ifndef GSI_KEYBOARD
#define GSI_KEYBOARD	52
#endif
/*
**  international keyboard type from console monitor
*/
char	internatKBcode = ~0;
/*
**  the type of keyboard, according to the kernel
*/
char	kbNameFromSys[64] = {0};

extern Opaque help_context;
extern void help_error();

extern int mappingNotify_event_expected;

/*
**	this flag coordinates edit-key-menu action with cursor movement.
**	When the edit key menu is up, it sets this flag to tell the
**	cursor movement function (in actions.c) to shut itself off,
**	so the key chosen for editing doesn't get de-selected. -bg
*/
int	menuIsUp = 0;

static Widget last_selected_keysym = NULL;

struct key_menus {
  Widget popup;
  Widget popup_kids [10];
  struct edit_key_box *edit_key_box;
};

#ifndef isupper
# define isupper(c)  ((c) >= 'A' && (c) <= 'Z')
#endif
#ifndef _tolower
# define _tolower(c)  ((c) - 'A' + 'a')
#endif

int
string_equal (s1, s2)
     char *s1, *s2;
{
  if (s1 == s2) return 1;
  if (!s1 || !s2) return 0;
  while (*s1 && *s2)
    {
      if ((isupper (*s1) ? _tolower (*s1) : *s1) !=
	   (isupper (*s2) ? _tolower (*s2) : *s2))
	 return 0;
       s1++, s2++;
     }
  return ((*s1 || *s2) ? 0 : 1);
}



extern Widget make_label (), make_label_1 ();

Widget
make_button (parent, name, string, left, top, callback, data, menu_name)
     Widget parent;
     char *name, *string;
     Widget left, top;
     void (*callback) ();
     caddr_t data;
     char *menu_name;
{
  Widget w = make_label_1 (parent, name, string, left, top,
			   xmPushButtonWidgetClass,
			   callback, data);
  return w;
}


Widget

make_toggle (parent, name, left, top, state, callback, data, label,
	     radio_group, radio_data, radio)
     Widget parent;
     char *name;
     Widget left, top;
     int state;
     void (*callback) ();
     caddr_t data;
     char *label;
     Widget radio_group;
     caddr_t radio_data;
     Boolean radio;
  
{
  Arg av [20];
  int ac = 0;
  Widget w;

  
  XtSetArg (av[ac], XmNset, (state ? True : False)); ac++;
  
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

  if (label) XtSetArg (av[ac], XmNlabelString,
		       XmStringCreate(label, def_char_set)),  ac++;

  if (radio) 
    {
      XtSetArg (av[ac], XmNindicatorType, XmONE_OF_MANY); ac++;
      XtSetArg (av[ac], XmNvisibleWhenOff, TRUE); ac++;
    }
  
  XtSetArg (av[ac], XmNmarginTop, 2); ac++;
  XtSetArg (av[ac], XmNmarginRight, 2); ac++;
  
  w = XtCreateManagedWidget (name, xmToggleButtonWidgetClass, parent, av, ac);
  if (callback) XtAddCallback (w, XmNvalueChangedCallback, callback, data);
  return w;
}




static void
button_quit (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  DXmHelpSystemClose(help_context, help_error, 
		      "dxkeycaps: help system error");
  exit (0);
}


static int modify_keyboard_modifiers ();

static void
perform_button_restore_callback (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = *((KeyboardWidget *) client_data);
  KeySym *keysyms;
  KeyCode lowest = 255, highest = 0;
  XModifierKeymap *modmap;
  struct keyboard *kbd = widget->keyboard.kbd;
  int per_code = widget->keyboard.default_keysyms_per_code;
  int i, j, k, error;

  if (! (keysyms = (KeySym *) calloc (sizeof (KeySym), per_code * 256)))
  {
	fprintf (stderr, "no memory available\n");
	exit (1);
  }
  modmap = XNewModifiermap (2); /* It'll grow */
  bzero (modmap->modifiermap, modmap->max_keypermod * 8);

  for (i = 0; i < kbd->nrows; i++)
    {
      struct row *row = &kbd->rows [i];
      for (j = 0; j < row->nkeys; j++)
	{
	  struct key *key = &row->keys [j];
	  if (key->keycode)
	    {
	      if (key->keycode < lowest)
		lowest = key->keycode;
	      if (key->keycode > highest)
		highest = key->keycode;
	      for (k = 0; k < per_code; k++)
		keysyms [key->keycode * per_code + k] =
		  key->default_keysyms [k];
	      if (key->default_mods)
		for (k = 0; k < 8; k++)
		  if (key->default_mods & (1<<k))
		    XInsertModifiermapEntry (modmap, key->keycode, k);
	    }
	}
    }
  if (highest <= lowest) exit (-69); /* can't happen */

  error = modify_keyboard_modifiers (widget, modmap);
  if (! error)
    {
      XChangeKeyboardMapping (XtDisplay (widget), lowest, per_code,
			      keysyms + (lowest * per_code),
			      highest - lowest);
      XSync (XtDisplay (widget), 0);
      mappingNotify_event_expected = 1;
      message (widget, "Keyboard restored to default state.");
    }
  XFreeModifiermap (modmap);
  free (keysyms);
	/*
	**	make the keycaps reflect the changes 
	*/
	do_layout (widget);
}

static void
button_restore (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = *((KeyboardWidget *) client_data);
  y_or_n_p (widget, "Restore Bindings");
  return;

}

extern char *version;

static FILE *fp;

static int
perform_save_bindings_callback (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
{
    KeyboardWidget widget = *((KeyboardWidget *) client_data);
  struct keyboard *kbd = widget->keyboard.kbd;
  int per_code = widget->keyboard.default_keysyms_per_code;
  int mod_count [8];
  KeySym mods [8][255];
  struct { struct key *key; int count; char *names[8]; }
     all [256], differences [256];
  int all_count = 0, diff_count = 0;
  KeySym *keysyms;
  int count, i, j, k;
  int partial = 0;
  /*
  **	need a buffer to build the $HOME/.dxkeycaps filename
  **	in... -bg
  */
  char	homeDotFile[256];
  char	*pEnv;
  extern char *getenv();
    
    
    long now = time ((long *) 0);
  
#ifdef NO_PWD
  char *userid = 0L;
#else
  struct passwd *pw = (struct passwd *) getpwuid (getuid ());
  char *userid = (pw ? pw->pw_name : 0L);
#endif

  if (partial >= 2)
  {
      message (widget, "Canceled writing keysym bindings.");
      return;
  }
  /*
  **	build the correct pathname to home directory... -bg
  */
  if (pEnv = getenv ("HOME"))
  {
    strcpy (homeDotFile, pEnv);
    if (homeDotFile[strlen(pEnv) - 1] != '/')
	strcat (homeDotFile, "/");
  }
  else 
  {
    fprintf (stderr, 
	"WARNING: dxkeycaps unable to find $HOME directory\n");
	homeDotFile[0] = '\0';
  }
  strcat (homeDotFile, ".dxkeycaps");
  fp = fopen(homeDotFile, "w");
    
  /*
  ** need to remind user to append a line to .X11Startup if it exists
  ** (else create one) to do a "xmodmap ~/.dxkeycaps"
  */
  
  if (fp == NULL) 
    {
		/*
		**	make the error message more specific... -bg
		*/
      fprintf(stderr, 
		"ERROR: Cannot open file %s.\n", homeDotFile);
      return(0);
    }
  else 
    {
      fprintf(stderr, "Remember to add 'xmodmap ~/.dxkeycaps' as ");
      fprintf(stderr, "a separate line in your .X11Startup file\n");
    }
    
      
  bzero (mod_count, sizeof (mod_count));

  for (i = 0; i < kbd->nrows; i++)
    {
      struct row *row = &kbd->rows [i];
      for (j = 0; j < row->nkeys; j++)
	{
	  struct key *key = &row->keys [j];
	  if (key->keycode)
	    {
	      unsigned long bits = key->widget->key.modifier_bits;
	      keysyms = XGetKeyboardMapping (XtDisplay (widget), key->keycode,
					     1, &count);
	      all [all_count].key = key;
	      for (; count > 0; count--)
		if (keysyms [count-1]) break;
	      if (count == 0)
		{
		  all [all_count].names [0] = "NoSymbol";
		  count = 1;
		}
	      else
		for (k = 0; k < count; k++)
		  {
		    char *str = XKeysymToString (keysyms [k]);
		    if (! str) str = "NoSymbol";
		    all [all_count].names [k] = str;
		  }
	      all [all_count].count = count;
	      if (count > per_code ||
		  bits != key->default_mods ||
		  (count > 0 && keysyms [0] != key->default_keysyms [0]) ||
		  (count > 1 && keysyms [1] != key->default_keysyms [1]) ||
		  (count > 2 && keysyms [2] != key->default_keysyms [2]) ||
		  (count > 3 && keysyms [3] != key->default_keysyms [3]) ||
		  (count > 4 && keysyms [4] != key->default_keysyms [4]) ||
		  (count > 5 && keysyms [5] != key->default_keysyms [5]) ||
		  (count > 6 && keysyms [6] != key->default_keysyms [6]) ||
		  (count > 7 && keysyms [7] != key->default_keysyms [7]))
		differences [diff_count++] = all [all_count];
	      all_count++;
	      for (k = 0; k < 8; k++)
		if (bits & (1<<k))
		  mods [k] [mod_count [k]++] = keysyms [0];
	      XFree ((char *) keysyms);
	    }
	}
    }

  fprintf (fp, "!\n! This is an `xmodmap' input file for the %s keyboard.\n",
	  kbd->name);
  fprintf (fp, "! This file was automatically generated on %s", ctime (&now));
  if (userid)
    fprintf (fp, "! by %s with %s.\n!\n", userid, version);
  else
    fprintf (fp, "! with %s.\n!\n", version);
  if (!partial)
    fprintf (fp, "! This file makes the following changes:\n!\n");

  for (i = 0; i < diff_count; i++)
    {
      if (partial)
	fprintf (fp, "keycode 0x%02X =\t", differences[i].key->keycode);
      else
	fprintf (fp, "! The \"%s\" key generates ",
		differences[i].key->widget->key.key_name);
      for (j = 0; j < differences[i].count; j++)
	{
	  fprintf (fp, "%s ", differences[i].names[j]);
	  if (j+1 == differences[i].count) continue;
	  if (partial)
	    {
	      putchar ('\t');
	      if (strlen (differences[i].names[j]) < 8) putchar ('\t');
	    }
	  else
	    {
	      if (j+1 == differences[i].count-1)
		fprintf (fp, (j == 0) ? " and " : ", and ");
	      else
		fprintf (fp, ", ");
	    }
	}
      if (!partial &&
	  differences[i].key->default_mods !=
	  differences[i].key->widget->key.modifier_bits)
	{
	  unsigned long bits = differences[i].key->widget->key.modifier_bits;
	  int mod_count = 0;
	  if (! bits)
	    fprintf (fp, ", and has no modifiers\n");
	  else
	    {
	      fprintf (fp, ", and the ");
	      for (k = 0; k < 8; k++)
		{
		  if (bits & (1<<k))
		    {
		      if (mod_count++) fprintf (fp, "/");
		      switch (k) {
		      case 0: fprintf (fp, "Shift"); break;
		      case 1: fprintf (fp, "Lock"); break;
		      case 2: fprintf (fp, "Control"); break;
		      case 3: fprintf (fp, "Mod1"); break;
		      case 4: fprintf (fp, "Mod2"); break;
		      case 5: fprintf (fp, "Mod3"); break;
		      case 6: fprintf (fp, "Mod4"); break;
		      case 7: fprintf (fp, "Mod5"); break;
		      }
		    }
		}
	      fprintf (fp, " modifier%s\n", (mod_count>1 ? "s" : ""));
	    }
	}
      else
	fprintf (fp, "\n");
    }

  if (!partial)
    {
      fprintf (fp, "\n");
      for (i = 0; i < all_count; i++)
	{
	  fprintf (fp, "keycode 0x%02X =\t", all [i].key->keycode);
	  for (j = 0; j < all[i].count; j++)
	    {
	      fprintf (fp, "%s ", all[i].names[j]);
	      if (j == all[i].count - 1) continue;
	      putchar ('\t');
	      if (strlen (all[i].names[j]) < 8) putchar ('\t');
	    }
	  fprintf (fp, "\n");
	}
    }

  fprintf (fp, "\nclear Shift\nclear Lock\nclear Control\nclear Mod1\n");
  fprintf (fp, "clear Mod2\nclear Mod2\nclear Mod4\nclear Mod5\n\n");
  for (i = 0; i < 8; i++)
    {
      if (mod_count [i] == 0) continue;
      fprintf (fp, "add ");
      switch (i) {
      case 0: fprintf (fp, "Shift   ="); break;
      case 1: fprintf (fp, "Lock    ="); break;
      case 2: fprintf (fp, "Control ="); break;
      case 3: fprintf (fp, "Mod1    ="); break;
      case 4: fprintf (fp, "Mod2    ="); break;
      case 5: fprintf (fp, "Mod3    ="); break;
      case 6: fprintf (fp, "Mod4    ="); break;
      case 7: fprintf (fp, "Mod5    ="); break;
      }
      for (j = 0; j < mod_count [i]; j++)
	{
	  char *str = XKeysymToString (mods [i][j]);
	  fprintf (fp, " %s", (str ? str : "NoSymbol"));
	}
      fprintf (fp, "\n");
    }
  fclose(fp);

}


static int
button_write (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = *((KeyboardWidget *) client_data);
  y_or_n_p (widget, "Save Session");
  return;

}


static void
select_keyboard_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
{
  char *name;
  XmString xmstr;
  
  KeyboardWidget *keyboard = (KeyboardWidget *) client_data;
  Arg av [10];
  int ac = 0;
  XtSetArg (av [ac], XmNlabelString, &xmstr);
  XtGetValues (button, av, 1);
  XmStringGetLtoR(xmstr, def_char_set, &name);
  replace_keyboard (keyboard, name);
  /*
   * If we're choosing a different keyboard, give the
   * user the option of *REALLY* seeing what it looks like
   */
  button_restore (button, client_data, call_data);
}



static int
modify_keyboard_modifiers (widget, modmap)
     Widget widget;
     XModifierKeymap *modmap;
{
  Display *display = XtDisplay (widget);
  int retries, timeout;
  char buf [255];
  for (retries = 4, timeout = 2; retries > 0; retries--, timeout *= 2)
    {
      int result;
      result = XSetModifierMapping (display, modmap);
      switch (result)
	{
	case MappingSuccess:
	  mappingNotify_event_expected = 1;
	  return 0;
	case MappingBusy:
	  sprintf (buf, "please release all keys withing %d seconds", timeout);
	  /* Calling message() doesn't work because exposes don't get
	     processed while we're sleeping.
	   */
	  message (widget, buf);
	  fprintf (stderr, "%s: %s\n", progname, buf);
	  XSync (display, 0);
	  sleep (timeout);
	  continue;
	case MappingFailed:
	  message (widget, "XSetModifierMap() failed");
	  XBell (display, 0);
	  return -1;
	default:
	  sprintf (buf, "unknown return code %d from XSetModifierMapping()",
		   result);
	  message (widget, buf);
	  XBell (display, 0);
	  return -1;
	}
    }
  sprintf (buf, "XSetModifierMapping() failed", timeout);
  message (widget, buf);
  XBell (display, 0);
}


int
modify_keyboard (widget, first_keycode, keysyms_per_keycode, keysyms,
		 num_codes, modmap)
     Widget widget;
     int first_keycode;
     int keysyms_per_keycode;
     KeySym *keysyms;
     int num_codes;
     XModifierKeymap *modmap;
{
  Display *display = XtDisplay (widget);
  int error = 0;
  if (modmap) error = modify_keyboard_modifiers (widget, modmap);
  if (keysyms_per_keycode == 0) keysyms_per_keycode = 1;
  if (!error)
  {
    XChangeKeyboardMapping (display, first_keycode, keysyms_per_keycode,
			    keysyms, num_codes);
	do_layout (widget);
  }
  return error;
}


static void
restore_key_default (parent, client_data, call_data)
     Widget parent;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = (KeyboardWidget) client_data;
  KeyWidget key = widget->keyboard.key_under_mouse;
  KeySym *keysyms = key->key.key->default_keysyms;
  int per_code = widget->keyboard.default_keysyms_per_code;
  KeyCode code = key->key.key->keycode;
  unsigned long bits = key->key.key->default_mods;
  XModifierKeymap *modmap;
  int i, j, error;

  if (! code)
    {
      XBell (XtDisplay (widget), 0);
      message (widget, "This key can't be changed.");
      return;
    }

  modmap = XGetModifierMapping (XtDisplay (widget));
  for (i = 0; i < 8; i++)
    if (bits & (1<<i))
      modmap = XInsertModifiermapEntry (modmap, code, i);
    else
      modmap = XDeleteModifiermapEntry (modmap, code, i);

  XSync (XtDisplay (widget), 0);
  error = modify_keyboard (widget, code, per_code, keysyms, 1, modmap);
  XFreeModifiermap (modmap);

  if (! error)
    {
      char *k, buf [255], buf2 [255], *b = buf;
      for (i = 0; i < per_code; i++)
	{
	  if (i) *b++ = ',', *b++ = ' ';
	  k = XKeysymToString (keysyms [i]);
	  if (! k) k = "NoSymbol";
	  strcpy (b, k);
	  b += strlen (k);
	}
      sprintf (buf2, "KeyCode 0x%02X restored to default state (%s)", 
	       key->key.key->keycode, buf);
      message (widget, buf2);
    }
  XSync (XtDisplay (widget), 0);
}


KeyWidget prompt_for_key ();


static void
swap_key (parent, client_data, call_data)
     Widget parent;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = (KeyboardWidget) client_data;
  XModifierKeymap *modmap = XGetModifierMapping (XtDisplay (widget));
  KeyWidget source_key = widget->keyboard.key_under_mouse;
  KeyWidget target_key;
  KeySym *source_keysyms;
  KeySym *target_keysyms;
  int source_count, target_count;
  unsigned long source_bits = source_key->key.modifier_bits;
  unsigned long target_bits;
  KeyCode source_code = source_key->key.key->keycode;
  KeyCode target_code;
  char buf [255];
  int i, error;

								/* make sure key is a valid one -bg */
  if (!source_key || !source_key->key.key->keycode) 
  {
      XBell (XtDisplay (widget), 0);
      message (widget, "This key can't be changed.");
	  return;
  }
  sprintf (buf, "Click on the key to swap with 0x%02X (%s)",
	   source_key->key.key->keycode,
	   source_key->key.key_name);
  target_key = prompt_for_key (widget, buf);
  if (! target_key) return;

  target_bits = target_key->key.modifier_bits;
  target_code = target_key->key.key->keycode;
  
  if (source_code == target_code)
    {
      XBell (XtDisplay (widget), 0);
      message (widget, "Those keys are the same");
      return;
    }

  for (i = 0; i < 8; i++)
    {
      if (source_bits & (1<<i))
	modmap = XInsertModifiermapEntry (modmap, target_code, i);
      else
	modmap = XDeleteModifiermapEntry (modmap, target_code, i);

      if (target_bits & (1<<i))
	modmap = XInsertModifiermapEntry (modmap, source_code, i);
      else
	modmap = XDeleteModifiermapEntry (modmap, source_code, i);
    }

  source_keysyms = XGetKeyboardMapping (XtDisplay (widget), source_code,
					1, &source_count);
  target_keysyms = XGetKeyboardMapping (XtDisplay (widget), target_code,
					1, &target_count);

  error = modify_keyboard (widget, target_code,
			   source_count, source_keysyms, 1, modmap);
  if (error) return;
  error = modify_keyboard (widget, source_code,
			   target_count, target_keysyms, 1, 0L);
  if (error) return;

  sprintf (buf, "Keys 0x%02x (%s) and 0x%02x (%s) swapped.",
	   source_key->key.key->keycode, source_key->key.key_name,
	   target_key->key.key->keycode, target_key->key.key_name);
  message (widget, buf);

  if (source_keysyms) XFree ((char *) source_keysyms);
  if (target_keysyms) XFree ((char *) target_keysyms);
  if (modmap) XFreeModifiermap (modmap);
}


static void
clone_key (parent, client_data, call_data)
     Widget parent;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = (KeyboardWidget) client_data;
  XModifierKeymap *modmap = XGetModifierMapping (XtDisplay (widget));
  KeyWidget source_key = widget->keyboard.key_under_mouse;
  KeyWidget target_key;
  KeySym *source_keysyms;
  int source_count;
  unsigned long source_bits = source_key->key.modifier_bits;
  KeyCode source_code = source_key->key.key->keycode;
  KeyCode target_code;
  char buf [255];
  int i, error;

								/* make sure key is a valid one -bg */
  if (!source_key || !source_key->key.key->keycode) 
  {
      XBell (XtDisplay (widget), 0);
      message (widget, "This key can't be changed.");
	  return;
  }
  sprintf (buf, "Click on the key to turn into a copy of 0x%02X (%s)",
	   source_key->key.key->keycode,
	   source_key->key.key_name);
  target_key = prompt_for_key (widget, buf);
  if (! target_key) return;

  target_code = target_key->key.key->keycode;
  
  if (source_code == target_code)
    {
      XBell (XtDisplay (widget), 0);
      message (widget, "Those keys are the same");
      return;
    }
  for (i = 0; i < 8; i++)
    {
      if (source_bits & (1<<i))
	modmap = XInsertModifiermapEntry (modmap, target_code, i);
      else
	modmap = XDeleteModifiermapEntry (modmap, target_code, i);
    }

  source_keysyms = XGetKeyboardMapping (XtDisplay (widget), source_code,
					1, &source_count);
  error = modify_keyboard (widget, target_code,
			   source_count, source_keysyms, 1, modmap);
  if (source_keysyms) XFree ((char *) source_keysyms);
  if (modmap) XFreeModifiermap (modmap);
  if (error) return;

  sprintf (buf, "Keys 0x%02x (%s) and 0x%02x (%s) are now the same.",
	   source_key->key.key->keycode, source_key->key.key_name,
	   target_key->key.key->keycode, target_key->key.key_name);
  message (widget, buf);
}



static void
disable_key (parent, client_data, call_data)
     Widget parent;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = (KeyboardWidget) client_data;
  KeyWidget key = widget->keyboard.key_under_mouse;
  KeyCode code = key->key.key->keycode;
  KeySym keysym = 0L;
  int i, error;
  char buf [255];
  XModifierKeymap *modmap = XGetModifierMapping (XtDisplay (widget));

  if (! key->key.key->keycode) 
  {
      XBell (XtDisplay (widget), 0);
      message (widget, "This key can't be changed.");
	return; /* guarantee valid key -bg */
  }
  for (i = 0; i < 8; i++)
    modmap = XDeleteModifiermapEntry (modmap, code, i);
  error = modify_keyboard (widget, code, 1, &keysym, 1, modmap);
  XFreeModifiermap (modmap);
  if (! error)
    {
      sprintf (buf, "KeyCode 0x%02X (%s) disabled.", key->key.key->keycode,
	       key->key.key_name);
      message (widget, buf);
    }
  XSync (XtDisplay (widget), 0);
}



extern void key_menu_pre_popup_hook ();


void pop_up_key_dbox ();
static struct edit_key_box * make_edit_key_dbox ();

struct key_menus *
make_key_menus (widget)
     KeyboardWidget widget;
{
  Arg av [20];
  int ac = 0, i = 0;
  Widget menu, title, item;
  extern Widget global_menu;
  struct key_menus *key_menus = (struct key_menus *)
    malloc (sizeof (struct key_menus));
  
  bzero (key_menus->popup_kids, sizeof (key_menus->popup_kids));

  ac = 0;
  XtSetArg (av[ac], XmNlabelString,
	    XmStringCreate("keyMenu", def_char_set)); ac++;
  menu = XmCreatePopupMenu ((Widget) widget, "keyMenu", av, ac);
  XtAddCallback (menu, XmNmapCallback, key_menu_pre_popup_hook, widget);
  key_menus->popup = menu;
  global_menu = menu;

  ac = 1;
  XtSetArg(av[0], XmNmnemonic, 'E' );
  item = XtCreateManagedWidget ("editKeysyms", xmPushButtonWidgetClass,
				menu, av,ac);
  XtAddCallback (item, XmNactivateCallback, pop_up_key_dbox, widget);
  key_menus->popup_kids [i++] = item;

  XtSetArg(av[0], XmNmnemonic, 'x' );
  item = XtCreateManagedWidget ("swapKey", xmPushButtonWidgetClass, menu,
				av, ac);
  XtAddCallback (item, XmNactivateCallback, swap_key, widget);
  key_menus->popup_kids [i++] = item;

  XtSetArg(av[0], XmNmnemonic, 'D' );
  item = XtCreateManagedWidget ("cloneKey", xmPushButtonWidgetClass, menu,
				av, ac);
  XtAddCallback (item, XmNactivateCallback, clone_key, widget);
  key_menus->popup_kids [i++] = item;

  XtSetArg(av[0], XmNmnemonic, 's' );
  item = XtCreateManagedWidget ("disableKey",xmPushButtonWidgetClass, menu,
				av, ac);
  XtAddCallback (item, XmNactivateCallback, disable_key, widget);
  key_menus->popup_kids [i++] = item;

  XtSetArg(av[0], XmNmnemonic, 'R' );
  item = XtCreateManagedWidget ("restoreKey", xmPushButtonWidgetClass, menu,
				av, ac);
  XtAddCallback (item, XmNactivateCallback, restore_key_default, widget);
  key_menus->popup_kids [i++] = item;

  key_menus->edit_key_box = make_edit_key_dbox (widget);
  return key_menus;
}

void
sensitize_menu (widget, menu, sensitive)
     KeyboardWidget widget;
     Widget menu;
     Bool sensitive;
{
  Arg av [10];
  int ac = 0, i = 0;
  struct key_menus *key_menus = widget->keyboard.key_menus;
  if (menu != key_menus->popup) return;
  XtSetArg (av [ac], XmNsensitive, sensitive); ac++;
  for (i = 0; i < sizeof (key_menus->popup_kids); i++)
    if (! key_menus->popup_kids [i]) return;
    else XtSetValues (key_menus->popup_kids [i], av, ac);
}


void pop_up_menu (w, menu, event)
  Widget w;
  Widget menu;
  XEvent event;
  {
      Arg av [20];
      int i, ac = 0;
      Position x, y;
      Dimension wid, ht;

      XtSetArg(av[0], XmNx, &x);
      XtSetArg(av[1], XmNy, &y);
      XtSetArg(av[2], XmNwidth, &wid);
      XtSetArg(av[3], XmNheight, &ht);
      XtGetValues(w, av, 4);
      XmMenuPosition(menu, event);
      XtSetArg(av[0], XmNx, x+wid);
      XtSetArg(av[1], XmNy, y+ht);
      XtSetValues(menu, av, 2);
      XtManageChild(menu);
  }
  

void pop_down_kbd_menu (w, menu, event)
  Widget w;
  Widget menu;
  XEvent event;
  {
    XtUnmanageChild(menu);
  }


int		iCount;
int		baseKeyboards;	/* count base (i.e. not internationals) */
int		internatFlag;	/* count internationals here */
/*
**	create_commands_menu checks the min_ and max_scancodes for
**	the display's keyboard to determine which keycap maps it
**	should incorporate into its list of options. It requires
**	a real Display * to do this, so it has been added...-bg
*/
Widget
create_commands_menu (disp, menubar, kbd)
Display *disp;
  Widget menubar;
  Widget *kbd;
  {
    Widget box, button, pane, kmenu, item, sep, buttons; 
	extern char getInternatKB ();
    extern char *knownNames[];    extern char *knownNames[];
    int namLen;
	/*
	**	create stuff for the international submenu 
	*/
	Widget	imenu, iutil;
	int		ix;
	static unsigned	minKactual, maxKactual;
	KeyCode	minKtable, maxKtable;
    char    buffer[512];
    int     utility;
    Arg av [20];
    int i, ac = 0;
	int	kbTypeFlag;	/* for figuring out what we have -bg */
	int	kflg;

	/*
	**	see if getsysinfo() will tell us about the keyboard...
	**		-bg
	*/
	if (runningLocal (disp))
	{
		getsysinfo (GSI_KEYBOARD, kbNameFromSys, 64);
		internatKBcode = getInternatKB();
		kbTypeFlag = test_pckb_type();
	}
	else kbTypeFlag = KB_UNKNOWN;
	/*
	**	get the actual min and max keycodes from the server
	*/

    if (kbTypeFlag >= 0)
        namLen = strlen (knownNames[kbTypeFlag]);
    else namLen = 0;
	/*
	 * not all servers tell the absolute truth (or even a relative
	 * truth) about the min and max keycodes at first blush. What
	 * we will do is call our subroutine that reads in the entire
	 * keyboard map for this display, and looks to find the lowest
	 * and the highest assigned keycodes. -bg
	 */
	getServerMinMax (disp, &minKactual, &maxKactual);
    /*
    **  if we have a pcxal keyboard and the server tells
    **  us it's not an XOpen-legal keyboard (minKactual == 7),
    **  then we need to rampage through the tables and
    **  fix them (because the pcxal hardware generates an
    **  illegal minimum scancode of 7, but the kernel
    **  remaps the scan-code 7 to a 9). This logic provides
    **  backwards compatibility for kernels that do not
    **  provide this remapping. It assumes that the tables
    **  built into dxkeycaps are build on the legal minkeycode
    **  == 8. -bg
    */
    if ((kbTypeFlag <= KB_PCXAL) /* -1 is 'dontknow' and 0 is KB_PCXAL */
		 && (minKactual == 7))
    {
        int tempix;
        for (tempix = 0; base_kbds[tempix]; tempix++)
        {
            fixMinKey (base_kbds[tempix]);
        }
        for (tempix = 0; pc102s[tempix]; tempix++)
        {
            fixMinKey (pc102s[tempix]);
        }
    }
	/*
	**	count base and international keyboards (the
	**	internationals need to be separate because they have
	**	their own pop-up menu).
	*/
	for (baseKeyboards = 0, ix = 0; base_kbds [ix]; ix++)
	{
        if (testValidKeyboard (disp, kbTypeFlag, base_kbds[ix],
							   minKactual, maxKactual))
                baseKeyboards++;
	}
	for (internatFlag = 0, ix = 0; pc102s [ix]; ix++)
	{
        if (testValidKeyboard (disp, kbTypeFlag, pc102s[ix],
							   minKactual, maxKactual))
                internatFlag++;
	}
    iCount = baseKeyboards + internatFlag;
	/*
	**	check to make sure we found something we know about
	*/
	if (! iCount)
	{
		return ((Widget) 0);
	}
	all_kbds = (struct keyboard **) calloc (iCount + 1, 
		sizeof (struct keyboard *));
	if (! all_kbds)
	{
		fprintf (stderr, "no memory available\n");
		exit (1);
	}
	for (iCount = 0, ix = 0; base_kbds [ix]; ix++)
	{
        if (testValidKeyboard (disp, kbTypeFlag, base_kbds[ix],
							   minKactual, maxKactual))
		{
			all_kbds[iCount++] = base_kbds[ix];
		}
	}
	for (ix = 0; pc102s [ix]; ix++)
	{
        if (testValidKeyboard (disp, kbTypeFlag, pc102s[ix],
							   minKactual, maxKactual))
		{
			all_kbds[iCount++] = pc102s[ix];
		}
	}
    /* 
	**	Create the main menu 
	*/
    pane = XmCreatePulldownMenu(menubar, "CommandsMenu", NULL, 0L);
    /* 
	**	Create submenu pane whose parent is main menu pane.  Fill submenu 
	*/
#ifdef ONE_DOT_TWO
    XtSetArg(av[0], XmNtearOffModel, (XtArgVal)XmTEAR_OFF_ENABLED );
    kmenu = XmCreatePulldownMenu (pane, "keyboardMenu", av, 1 );
#else
    kmenu = XmCreatePulldownMenu (pane, "keyboardMenu", NULL, 0L);
#endif
    ac = 0;

    for (i = 0; i < baseKeyboards; i++)
    {
        /*
        **  if we have international keyboards to deal with
        */
		kflg = isLK (all_kbds[i]);
		switch (kflg)
		{
			case KB_LK401:
			case KB_LK201:
			case KB_LK443:
			case KB_LK444:
			case KB_PCXAL:
				if (internatFlag)
					makeInternatKeyboardMenu (kmenu, i, kbd);
				break;
			case KB_LK421:
				XtSetArg (av[0], XmNlabelString,
				  XmStringCreate(all_kbds[i]->name, def_char_set));
				item = XtCreateManagedWidget ("selectKeyboard",
				  xmPushButtonWidgetClass, kmenu, av, 1);
				XtAddCallback (item, XmNactivateCallback,
				  select_keyboard_cb, *kbd);
				break;
			case KB_NCD:
				makeOutsideMenu (kmenu, i, kbd);
				i = baseKeyboards;
				break;
			default:
				printf ("unknown keyboard\n");
				exit (1);
				break;
        }
    }

    
    /* Populate the main menu with one cascade button for submenu */
    
    XtSetArg(av[0], XmNmnemonic, 'K' );
    button = XtCreateManagedWidget("keyboard", xmCascadeButtonWidgetClass,
				   pane, (ArgList)av, 1);
    XtSetArg(av[0], XmNsubMenuId, kmenu);
    XtSetValues(button, av, 1);
    XtSetArg(av[0], XmNmnemonic, 'R' );
    button = XtCreateManagedWidget("restore", xmPushButtonWidgetClass, pane,
				   (ArgList)av, 1);
    XtAddCallback(button, XmNactivateCallback, button_restore, *kbd);

    XtSetArg(av[0], XmNmnemonic, 'S' );
    button = XtCreateManagedWidget("write", xmPushButtonWidgetClass, pane,
				   (ArgList)av, 1);
    XtAddCallback(button, XmNactivateCallback, (XtCallbackProc)button_write,
		  *kbd);

    XtSetArg(av[0], XmNmnemonic, 'x' );
    button = XtCreateManagedWidget("quit", xmPushButtonWidgetClass, pane,
				   (ArgList)av, 1);
    XtAddCallback(button, XmNactivateCallback, button_quit, *kbd);
    
    return pane;

}

void
help_callback (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  DXmHelpSystemDisplay(help_context, "dxkeycaps", "topic", "dxkeycaps",
		      help_error, "dxkeycaps: help system error");
}




Widget
create_help_menu (menubar,kbd)
  Widget menubar;
  Widget *kbd;
  {
    Widget box, button, pane, kmenu, item, sep, buttons;
    Arg av [20];
    int i, ac = 0;

    /* Create the main menu */
    
    pane = XmCreatePulldownMenu(menubar, "HelpMenu", NULL, 0L);

    /* Populate the main menu with one cascade button for submenu */
    
    XtSetArg(av[0], XmNmnemonic, 'O' );
    button = XtCreateManagedWidget("overview", xmPushButtonWidgetClass, pane,
				   av, 1 );
    XtAddCallback (button, XmNactivateCallback, help_callback, *kbd);

    return pane;

}



/*
**	this function now needs a Display * for create_commands_menu
**		-bg
*/
Widget
make_menu_bar (disp, parent, kbd)
	Display *disp;
     Widget parent;
     Widget *kbd;
{
  Widget help_menu;  
  Widget commands_menu, menubar, commands, help;
  Widget box, button, pane, kmenu, item, sep, buttons; 
  Arg av [20];
  int i, ac = 0;

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  menubar = XmCreateMenuBar(parent, "MenuBar", NULL, 0L);
  XtManageChild(menubar);

  if (! (commands_menu = create_commands_menu (disp, menubar, &kbd)))
  {
	/* 
	**	failed for some reason (probably didn't understand 
	**	what kind of keyboard we have).
	*/
	return ((Widget) 0);
  }

  help_menu = create_help_menu( menubar, &kbd );

  /* main menubar stuff */
  
  XtSetArg (av[0], XmNlabelString,  XmStringCreate("Commands",
						    def_char_set));
  XtSetArg(av[1], XmNsubMenuId, commands_menu);
  XtSetArg(av[2], XmNmnemonic, 'F' );
  commands = XtCreateManagedWidget("commands", xmCascadeButtonWidgetClass,
				   menubar, av, 3);

  XtSetArg (av[0], XmNlabelString,  XmStringCreate("Help",
						   def_char_set));
  XtSetArg(av[1], XmNmnemonic, 'H' );
  XtSetArg(av[2], XmNsubMenuId, help_menu);
  help = XtCreateManagedWidget("Help", xmCascadeButtonWidgetClass,
			       menubar, av, 3);
  XtAddCallback (help, XmNactivateCallback, help_callback, *kbd);
  
  XtSetArg(av[0], XmNmenuHelpWidget, help);
  XtSetValues(menubar, av, 1);

  return menubar;
}



/* These are used to compute the default sizes of the windows.  Hack hack.
 */
#define LONGEST_KEYSYM_NAME "Greek_upsilonaccentdieresis"
#define MEDIUM_LENGTH_KEYSYM_NAME "Greek_IOTAdiaresis"

static char * all_keyset_names [] = {
  "Latin1",
  "Latin2",
  "Latin3",
  "Latin4",
  "Kana",
  "Arabic",
  "Cyrillic",
  "Greek",
  "Technical",
  "Special",
  "Publishing",
  "APL",
  "Hebrew",
  "Keyboard",
  0
};

static int number_of_keyset_names = 13;

static char *keysym_name_buffer [256];

struct edit_key_box {
  KeyboardWidget keyboard;
  Widget shell;
  Widget label;
  Widget keysym_buttons [8];
  Widget mod_buttons [8];
  Widget keyset_list, keysym_list;
  Widget autorepeat_widget;
  int autorepeat;
};


static void
keyset_list_cb (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  XmListCallbackStruct *lr = (XmListCallbackStruct *) call_data;
  struct edit_key_box *box = (struct edit_key_box *) client_data;
  int set = lr->item_position;
  int i, j = 0;
  Arg av [10];
  int ac = 0, list_size;
  XmString xmstr, *xmstr_array;
  char *charSet;

  set = set - 1;

  XmStringGetLtoR( lr->item, def_char_set, &charSet );
  if (!strcmp (charSet, "Keyboard")) set = 255; /* Evil hack */

  for (i = 0; i < 256; i++)
    {
      char *name = XKeysymToString ((set << 8) | i);
      if (! name && i == 0) name = "NoSymbol";
      if (name) {
	  keysym_name_buffer [j] = name;
	  list_size = j++;
      }
    }

  /* Character set list now */

  xmstr_array = (XmString *) XtMalloc(sizeof(XmString) * ++list_size);
  xmstr = (XmString) NULL;
  
  for (ac = 0; ac < list_size; ac++) 
    {
       xmstr = (XmString) XmStringCreate(keysym_name_buffer[ac],
					 def_char_set);
       xmstr_array[ac] = xmstr;
      
    }

  ac = 0;
  XtSetArg (av[ac], XmNitems, xmstr_array); ac++;
  XtSetArg (av[ac], XmNitemCount, list_size); ac++;
  XtSetArg (av[ac], XmNvisibleItemCount, 17); ac++;
  XtSetValues(box->keysym_list, av, ac);
  
}


static void
keysym_list_cb (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  XmListCallbackStruct *lr = (XmListCallbackStruct *) call_data;
  struct edit_key_box *box = (struct edit_key_box *) client_data;
  int set = lr->item_position;
  int state, i = 1, ac;
  Arg av1[10];
  XmString label;
  char *text;

  i = 0;
  state = FALSE;

  XmStringGetLtoR(lr->item, def_char_set, &text);

  for (i=0; i<8 && !state; i++) 
    {
      state = XmToggleButtonGetState(box->keysym_buttons [i]);
    }

  /*
     in case no keysym button are selected.
  */
  
  if ((i >= 8) && (state == 0)) return;
  
  if (i > 0)
    {
      Arg av [10];
      int ac = 0;
      XtSetArg (av [ac], XmNlabelString,
		XmStringCreate(text, def_char_set));
      XtSetValues (box->keysym_buttons [i-1], av, 1);
      
    }

}


static void
autorepeat_button_cb (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  struct edit_key_box *box = (struct edit_key_box *) client_data;
  Arg av [10];
  int ac = 0;
  XmString str;
  
  box->autorepeat = !box->autorepeat;
  str = (XmString) (box->autorepeat ? "Yes" : "No");
  
  XtSetArg (av [ac], XmNlabelString,
	    XmStringCreate(str, def_char_set)); ac++;
  XtSetValues (widget, av, ac);
}


static void
abort_button_cb (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  struct edit_key_box *box = (struct edit_key_box *) client_data;
  XtPopdown (box->shell);
  menuIsUp = 0;	   	/* menu coming down--clear the flag! -bg */
  message (box->keyboard, "Aborted...");
}


static void
ok_button_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
{
  struct edit_key_box *box;
  KeyboardWidget widget;
  KeyWidget key;
  KeyCode code;
  KeySym keysyms [8];
  int keysym_count;
  XModifierKeymap *modmap;
  int i, error;
  Arg av [10];
  int ac = 0;
  XmString xmstr;
  char *text ;
  Boolean state;
  
  box = (struct edit_key_box *) client_data;
  if (! box) return;
  if (widget = box->keyboard)
	  key = widget->keyboard.key_under_mouse;
  else key = (KeyWidget) 0;	/* go for robust checking!! -bg */
  if ( !(error = !(widget && key && key->key.key->keycode)))
  {
  	modmap = XGetModifierMapping (XtDisplay (widget));
	code = key->key.key->keycode;
	ac = 0;
	XtSetArg (av [ac], XmNlabelString, &xmstr);
		  
	for (i = 0; i < 8; i++)
    {
      	XtGetValues (box->keysym_buttons [i], av, 1);
      	if (XmStringGetLtoR(xmstr, def_char_set, &text))
		{
			/*
			**	hackaway!!!! XStringToKeysym() coredumps if it
			**	gets a string > length 25 or so. When the fix
			**	catches up with us, we can remove this hack
			**	Since we only see it on "NoSymbol                  "
			**	we'll just look for "NoSymbol" and fill in
			**	the expected NULL. -bg
			*/
			if (!(strncmp (text, "NoSymbol", 8)))
				keysyms[i] = 0;
			else keysyms [i] = XStringToKeysym (text);
		}
      	else keysyms [i] = 0;
	}
  	for (keysym_count = 8; keysym_count > 0; keysym_count--)
    	if (keysyms [keysym_count-1]) break;

  	ac = 0;
  	XtSetArg (av [ac], XmNset, &state);
  	for (i = 0; i < 8; i++)
    {
      state = XmToggleButtonGetState(box->mod_buttons [i]);
      
      if (state)
	  modmap = XInsertModifiermapEntry (modmap, code, i);
      	else
	  modmap = XDeleteModifiermapEntry (modmap, code, i);
    }

	XSync (XtDisplay (widget), 0);
	error = modify_keyboard (widget, code, keysym_count, keysyms, 1, modmap);
	XFreeModifiermap (modmap);

  	if (!error && box->autorepeat != key->key.auto_repeat_p)
    {
      XKeyboardControl values;
      values.key = key->key.key->keycode;
      values.auto_repeat_mode =
		(box->autorepeat ? AutoRepeatModeOn : AutoRepeatModeOff);
		  XChangeKeyboardControl (XtDisplay (widget), 
			KBKey | KBAutoRepeatMode, &values);
    }
  }
  XtPopdown (box->shell);
  /*
  **	enable the cursor movement stuff now. -bg
  */
  menuIsUp = 0;
  if (! error) message (box->keyboard, "Modified.");
}


static void
move_scrollbar (list, percent)
     Widget list;
     float percent;
{
  Widget vp = XtParent (list);
  Widget scrollbar = XtNameToWidget (vp, "vertical");
  float visible_fraction = (((float) vp->core.height) /
			    ((float) list->core.height));
  XEvent event;
  Arg av [10];
  int ac = 0;
  if (visible_fraction < 1.0)
    percent -= (visible_fraction / 2.0);
  XtCallCallbacks (scrollbar, XtNjumpProc, &percent);
}


static void
keysym_button_cb (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  struct edit_key_box *box = (struct edit_key_box *) client_data;
  KeyboardWidget keyboard = box->keyboard;
  KeyWidget key = keyboard->keyboard.key_under_mouse;
  char *keysym_name;
  KeySym real_keysym;
  int keyset, keysym, index, list_size;
  Arg av[10];
  int ac = 0;
  XmString xmstr;
  char *keysym_name_text;

  if (call_data == 0)	/* we're being toggled off */
    return;

  if (last_selected_keysym != widget) 
    {
      XtSetArg (av [ac], XmNset, False);
      XtSetValues (last_selected_keysym, av, 1);
      last_selected_keysym = widget; 
    }
  
  XtSetArg (av [ac], XmNlabelString, &xmstr);
  XtGetValues (widget, av, 1);
  if (XmStringGetLtoR(xmstr, def_char_set, &keysym_name_text))
	{
		/*
		**	hackaway!!!! XStringToKeysym() coredumps if it
		**	gets a string > length 25 or so. When the fix
		**	catches up with us, we can remove this hack
		**	Since we only see it on "NoSymbol                  "
		**	we'll just look for "NoSymbol" and fill in
		**	the expected NULL. -bg
		*/
		if (!(strncmp (keysym_name_text, "NoSymbol", 8)))
			real_keysym = 0;
		else real_keysym = XStringToKeysym (keysym_name_text);
	}
  else real_keysym = 0;

  /* Get the one that's in the list */
  keysym_name = XKeysymToString (real_keysym);
  keyset = (real_keysym >> 8);
  keysym = (real_keysym & 0xff);
  if (keyset == 255) keyset = 13;  /* #### EEEEvil hack */

  list_size = (sizeof (all_keyset_names) / sizeof (all_keyset_names[0]));
  
  for (list_size = 0; list_size < 256; list_size++)
    if (keysym_name_buffer [list_size] == keysym_name)
      index = list_size;
    else if (! keysym_name_buffer [list_size])
      break;
  if (! keysym_name) index = 0;
  
}


static void
undo_button_cb (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;

  {
    extern KeyWidget CurrentKey;
  struct edit_key_box *box = (struct edit_key_box *) client_data;
  KeyboardWidget keyboard = box->keyboard;
  KeyWidget key = CurrentKey;
  KeySym *keysyms;
  int syms_per_code;
  char buf [255];
  Arg av[20];
  int ac = 0;
  int i, j;

  keysyms = XGetKeyboardMapping (XtDisplay (widget), key->key.key->keycode,
				 1, &syms_per_code);

  sprintf (buf, "Definition of key 0x%02X (%s)", key->key.key->keycode,
	   key->key.key_name);
  XtSetArg (av[ac], XmNlabelString,
	    XmStringCreate(buf, def_char_set)); ac++;
  XtSetValues (box->label, av, ac);
  ac = 0;

  for (i = 0; i < syms_per_code; i++)
  {
      char *sym = XKeysymToString (keysyms [i]);
      if (! sym) sym = "NoSymbol                  ";
      ac = 0;
      XtSetArg (av[ac], XmNlabelString,
		XmStringCreate(sym, def_char_set)); ac++;
      XtSetValues (box->keysym_buttons [i], av, ac);
  }
  if (keysyms) XFree ((char *) keysyms);
  ac = 0;
  XtSetArg (av[ac], XmNlabelString,
	    XmStringCreate("NoSymbol      ", def_char_set)); ac++;
  for (; i < 8; i++)
    XtSetValues (box->keysym_buttons [i], av, ac);

  for (i = 0; i < 8; i++)
  {
      ac = 0;
      XtSetArg (av[ac], XmNset,
		((key->key.modifier_bits & 1<<i) ? True : False)); ac++;
      XtSetValues (box->mod_buttons [i], av, ac);
  }

  XmToggleButtonSetState(box->keysym_buttons [0], TRUE, FALSE);
  keysym_button_cb (box->keysym_buttons [0], box, 0L);
  box->autorepeat = !key->key.auto_repeat_p;
  autorepeat_button_cb (box->autorepeat_widget, box, 0L);
}


static struct edit_key_box *
make_edit_key_dbox (widget)
     KeyboardWidget widget;
{
  struct edit_key_box *box = (struct edit_key_box *)
    malloc (sizeof (struct edit_key_box));
  Arg av [20];
  int ac = 0, list_size;
  Widget toplevel, box1, box2, box3;
  Widget keysym_box, button_box, keyset_box, keyset_syms_box, mod_box;
  Widget line_box, prev, prev_tog;
  Widget set_list, sym_list;
  Widget set_vp, sym_vp;
  Widget label, separator, frame, scrolled_window;
  Widget set_list2, scrolled_window2;
  XmString xmstr, *xmstr_array;
  XmString xmstr2, *xmstr2_array;
  char buf1 [100];
  
  toplevel = XtCreatePopupShell ("EditKey", transientShellWidgetClass,
				 (Widget) widget, av, ac);
  
  box1 = XtCreateManagedWidget ("form1", xmFormWidgetClass, toplevel, av, ac);
  box->label = label = make_label (box1, "label", 0L, 0L, 0L);
  ac = 0;
  
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNtopWidget, label); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNhorizontalSpacing, 5); ac++;
  
  box2 = XtCreateManagedWidget ("form2", xmFormWidgetClass, box1, av, ac);

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  keysym_box = XtCreateManagedWidget ("keysymBox", xmFormWidgetClass, box2,
				      av, ac);
  
  prev = make_label (keysym_box, "symsOfCode", 0L, 0L, 0L);
  prev = make_label (keysym_box, "spacer", "      ", 0L, prev);
  ac = 0;
  prev_tog = 0;
  line_box = prev;

#define TOG(var, name, index) \
   { ac = 0; \
     XtSetArg (av[ac], XmNorientation, XmHORIZONTAL); ac++; \
     XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++; \
     XtSetArg (av[ac], XmNtopWidget, line_box); ac++; \
     XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++; \
     line_box = XtCreateManagedWidget("keysymLine", xmRowColumnWidgetClass,\
				      keysym_box, av, ac); \
     ac = 0; \
     XtSetArg (av[ac], XmNradioBehavior, True); ac++; \
     XtSetArg (av[ac], XmNradioAlwaysOne, True); ac++; \
     XtSetArg (av[ac], XmNisHomogeneous, False); ac++; \
     XtSetArg (av[ac], XmNentryClass, NULL); ac++; \
     XtSetValues(line_box, av, ac); \
     var = make_label (line_box, name, 0L, 0L, prev); \
     if (index) \
       var = make_toggle (line_box, "keysymValue" ,var, prev, 0L, \
			  keysym_button_cb, box, MEDIUM_LENGTH_KEYSYM_NAME, \
			  prev_tog, index, TRUE); \
     else \
       var = make_button (line_box, "autoRepeatValue", "Yes", var, prev, \
			  autorepeat_button_cb, box, 0L); \
       prev_tog = prev = var; }
  TOG (box->keysym_buttons [0], "keysym1", 1);
  TOG (box->keysym_buttons [1], "keysym2", 2);
  TOG (box->keysym_buttons [2], "keysym3", 3);
  TOG (box->keysym_buttons [3], "keysym4", 4);
  TOG (box->keysym_buttons [4], "keysym5", 5);
  TOG (box->keysym_buttons [5], "keysym6", 6);
  TOG (box->keysym_buttons [6], "keysym7", 7);
  TOG (box->keysym_buttons [7], "keysym8", 8);
  
  XtSetArg(av[0], XmNrecomputeSize, False);
  XtSetValues(box->keysym_buttons [0], av, 1);

  prev = prev_tog = 0;
  line_box = make_label (keysym_box, "spacer2", "", 0L, line_box);
  TOG (box->autorepeat_widget, "autoRepeat", 0L);
#undef TOG

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNleftWidget, keysym_box); ac++;
  XtSetArg (av[ac], XmNorientation, XmVERTICAL); ac++;
  XtSetArg (av[ac], XmNseparatorType, XmSINGLE_LINE); ac++;
  XtSetArg (av[ac], XmNwidth, 4); ac++;
  separator = XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
				    box2, av, ac);
  
  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNleftWidget, separator); ac++;
  mod_box = XtCreateManagedWidget ("modifierBox", xmFormWidgetClass,
				   box2, av, ac);
  prev = make_label (mod_box, "modifiers", 0L, 0L, 0L);
  prev = make_label (mod_box, "spacer", "", 0L, prev);
#define TOG(var, name) \
   { var = make_toggle (mod_box,name,0L,prev,0L,0L,0L,0L,0L,0L,FALSE); \
     prev = var; }
  TOG (box->mod_buttons [0], "modShift");
  TOG (box->mod_buttons [1], "modLock");
  TOG (box->mod_buttons [2], "modControl");
  TOG (box->mod_buttons [3], "mod1");
  TOG (box->mod_buttons [4], "mod2");
  TOG (box->mod_buttons [5], "mod3");
  TOG (box->mod_buttons [6], "mod4");
  TOG (box->mod_buttons [7], "mod5");
#undef TOG

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNleftWidget, mod_box); ac++;
  XtSetArg (av[ac], XmNorientation, XmVERTICAL); ac++;
  XtSetArg (av[ac], XmNseparatorType, XmSINGLE_LINE); ac++;
  XtSetArg (av[ac], XmNwidth, 4); ac++;
  separator = XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
				    box2, av, ac);
  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNleftWidget, separator); ac++;
  
  keyset_box = XtCreateManagedWidget("keysetBox", xmFormWidgetClass,
				     box2, av, ac);
  prev = make_label (keyset_box, "allKeySets", 0L, 0L, 0L);
  
  
  /* Character set list now */

  list_size = (sizeof (all_keyset_names) / sizeof (all_keyset_names[0]));
  xmstr_array = (XmString *) XtMalloc(sizeof(XmString) * list_size);
  xmstr = (XmString) NULL;
  
  for (ac = 0; ac < list_size-1; ac++) 
    {
       xmstr = (XmString) XmStringCreate(all_keyset_names[ac],
					 def_char_set);
       xmstr_array[ac] = xmstr;
      
    }

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNtopWidget, prev); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNscrollBarPlacement, XmTOP_LEFT); ac++;
  XtSetArg (av[ac], XmNvisualPolicy, XmVARIABLE); ac++;
  XtSetArg (av[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;

  scrolled_window = XtCreateManagedWidget("scrolledWin",
					  xmScrolledWindowWidgetClass,
					  keyset_box, av, ac);

  ac = 0;
  XtSetArg (av[ac], XmNitems, xmstr_array); ac++;
  XtSetArg (av[ac], XmNitemCount, list_size-1); ac++;
  XtSetArg (av[ac], XmNvisibleItemCount, 17); ac++;
  XtSetArg (av[ac], XmNselectionPolicy, XmSINGLE_SELECT); ac++;
  set_list = XtCreateManagedWidget("keysets", xmListWidgetClass,
				   scrolled_window,
				   av, ac);
  XtAddCallback (set_list, XmNsingleSelectionCallback, keyset_list_cb, box);

  ac = 0;
  XtSetArg (av[ac], XmNworkWindow, set_list); ac++;
  XtSetValues(scrolled_window, av, ac);
  
  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNleftWidget, keyset_box); ac++;
  XtSetArg (av[ac], XmNorientation, XmVERTICAL); ac++;
  XtSetArg (av[ac], XmNseparatorType, XmSINGLE_LINE); ac++;
  XtSetArg (av[ac], XmNwidth, 4); ac++;
  separator = XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
				    box2, av, ac);

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNleftWidget, separator); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;

  keyset_syms_box = XtCreateManagedWidget ("keysetSymsBox", xmFormWidgetClass,
					   box2, av, ac);
  prev = make_label (keyset_syms_box, "keySymsOfSet", 0L, 0L, 0L);

  bcopy (all_keyset_names, keysym_name_buffer, sizeof (all_keyset_names));
  keysym_name_buffer [0] = LONGEST_KEYSYM_NAME;

  ac = 0;

  /* Setting keysym list here */

  list_size = (sizeof (keysym_name_buffer) / sizeof (keysym_name_buffer[0]));

  xmstr2_array = (XmString *) XtMalloc(sizeof(XmString) * list_size);
  
  for (ac = 0; ac < list_size; ac++) 
    {
      xmstr2 = (XmString) XmStringCreate(keysym_name_buffer[ac],
					def_char_set);
      xmstr2_array[ac] = xmstr2;
    }

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNtopWidget, prev); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNscrollBarPlacement, XmTOP_LEFT); ac++;

  scrolled_window2 = XtCreateManagedWidget("scrolledWin2",
					   xmScrolledWindowWidgetClass,
					  keyset_syms_box, av, ac);

  ac = 0;
  XtSetArg (av[ac], XmNitems, xmstr2_array); ac++;
  XtSetArg (av[ac], XmNitemCount, 1); ac++;
  XtSetArg (av[ac], XmNvisibleItemCount, 17); ac++;
  XtSetArg (av[ac], XmNselectionPolicy, XmSINGLE_SELECT); ac++;
  
  set_list2 = XtCreateManagedWidget("keysets", xmListWidgetClass,
				    scrolled_window2,
				    av, ac);
  ac = 0;
  XtSetArg (av[ac], XmNworkWindow, set_list2); ac++;
  XtSetValues(scrolled_window2, av, ac);
  
  XtAddCallback (set_list2, XmNsingleSelectionCallback, keysym_list_cb, box);

    
  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNtopWidget, box2); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNorientation, XmHORIZONTAL); ac++;
  XtSetArg (av[ac], XmNseparatorType, XmSINGLE_LINE); ac++;
  XtSetArg (av[ac], XmNwidth, 4); ac++;
  separator = XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
				    box1, av, ac);

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNtopWidget, separator); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNorientation, XmHORIZONTAL); ac++;
  
  button_box = XtCreateManagedWidget ("buttons", xmRowColumnWidgetClass,
				      box1, av, ac);

  prev = make_button (button_box, "ok", "OK", prev, 0L, ok_button_cb, box, 0L);
  prev = make_button (button_box, "undo", "Reset", 0L, 0L, undo_button_cb, 
		      box, 0L);
  prev = make_button (button_box, "abort", "Cancel", prev, 0L, abort_button_cb,
		      box,0L);

  box->keyboard = widget;
  box->shell = toplevel;
  box->keyset_list = set_list;
  box->keysym_list = set_list2;
  last_selected_keysym = box->keysym_buttons [0];
  XmListSelectPos(box->keyset_list, 1, True);
  return box;
}

void
pop_up_key_dbox  (parent, client_data, call_data)
     Widget parent;
     XtPointer client_data, call_data;
{
  KeyboardWidget widget = (KeyboardWidget) client_data;
  KeyWidget key = widget->keyboard.key_under_mouse;
  struct edit_key_box *edit_key_box =
    widget->keyboard.key_menus->edit_key_box;
  {
    Widget topmost = parent;
    int x, y, w, h;
								/* try this (bg) */
	if (! key->key.key->keycode) 
	{
		XBell (XtDisplay (widget), 0);
		message (widget, "This key can't be changed.");
		return;
	}
								/* try this (bg) */
    XtRealizeWidget (edit_key_box->shell);
    w = edit_key_box->shell->core.width;
    h = edit_key_box->shell->core.height;
    while (topmost->core.parent)
      topmost = topmost->core.parent;
    if (topmost->core.width < w) x = topmost->core.x;
    else x = topmost->core.x + ((topmost->core.width - w) / 2);
    if (topmost->core.height < h) y = topmost->core.y;
    else y = topmost->core.y + ((topmost->core.height - h) / 2);
    XtMoveWidget (edit_key_box->shell, x, y);
  }
  /*
  **    disable the cursor movement stuff now. -bg
  */
  menuIsUp = 1;
  XtPopup (edit_key_box->shell, XtGrabNonexclusive);
  undo_button_cb (edit_key_box->shell, edit_key_box, 0L);
}



static void
yorn_button_cb (widget, client_data, call_data)
     Widget widget;
     XtPointer client_data, call_data;
{
  *((int *) client_data) = 1;
}

static int
db_ok_callback (widget, client_data, call_data)
  Widget widget;
  XtPointer client_data, call_data;
  {
    XtUnmanageChild (widget);
    XtDestroyWidget (widget);
  }

static int
db_cancel_callback (widget, client_data, call_data)
  Widget widget;
  XtPointer client_data, call_data;
  {
    KeyboardWidget kbd = (KeyboardWidget) client_data;
    message (kbd, "Cancelled...");
    XtUnmanageChild (widget);
    XtDestroyWidget (widget);
  }

static int
db_help_callback (widget, client_data, call_data)
  Widget widget;
  XtPointer client_data, call_data;
  {
    fprintf(stderr, "Help is not currently available\n");
    
    XtUnmanageChild (widget);
    XtDestroyWidget (widget);
  }

int
y_or_n_p (widget, name)
  KeyboardWidget widget;
  char *name;
  
{
  XtAppContext app = XtWidgetToApplicationContext ((Widget) widget);
  Widget topmost = (Widget) widget;
  XEvent event;
  int x, y, w, h;
  Arg av [10];
  int ac = 0;
  Widget shell, box1, box2, prev;
  int ans [3];
  Widget label;

  shell = XmCreateMessageDialog ((Widget) widget, name, av, ac);
  if (strcmp(name, "Restore Bindings") == 0) 
    {
      
      XtAddCallback(shell, XmNokCallback,
		    perform_button_restore_callback, widget);

    }
  else
      /* save session */
    {
      XtAddCallback(shell, XmNokCallback,
		    (XtCallbackProc)perform_save_bindings_callback, widget);
    }
  
  XtAddCallback(shell, XmNcancelCallback,
		(XtCallbackProc)db_cancel_callback, widget);
  XtAddCallback(shell, XmNhelpCallback,
		(XtCallbackProc)db_help_callback, widget);  
  
  XtRealizeWidget (shell);
  w = shell->core.width;
  h = shell->core.height;
  while (topmost->core.parent)
    topmost = topmost->core.parent;
  if (topmost->core.width < w) x = topmost->core.x;
  else x = topmost->core.x + ((topmost->core.width - w) / 2);
  if (topmost->core.height < h) y = topmost->core.y;
  else y = topmost->core.y + ((topmost->core.height - h) / 2);
  XtMoveWidget (shell, x, y);

  XtManageChild(shell);
  
}



KeyWidget
prompt_for_key (keyboard, msg)
     KeyboardWidget keyboard;
     char *msg;
{
  XtAppContext app = XtWidgetToApplicationContext ((Widget) keyboard);
  KeyWidget key;
  XEvent event;
  message (keyboard, msg);

  if (XGrabPointer (XtDisplay (keyboard), XtWindow (keyboard), True,
		    ButtonPressMask | ButtonReleaseMask,
		    GrabModeAsync, GrabModeAsync, 0L,
		    keyboard->keyboard.select_cursor,
		    CurrentTime))
    {
      XBell (XtDisplay (keyboard), 0);
      message (keyboard, "Grab failed.");
      return NULL;
    }
  
  while (1)
    {
      XtAppNextEvent (app, &event);

      if (event.xany.type == ButtonPress)
	{
	  XUngrabPointer (XtDisplay (keyboard), CurrentTime);
	  if (key = keyboard->keyboard.key_under_mouse)
	    return key;
	  XBell (XtDisplay (keyboard), 0);
	  message (keyboard, "You must click on a key.");
	  return NULL;
	}
      else if (event.xany.type == ButtonRelease ||
	       event.xany.type == KeyPress ||
	       event.xany.type == KeyRelease)
	{
	  XUngrabPointer (XtDisplay (keyboard), CurrentTime);
	  XBell (XtDisplay (keyboard), 0);
	  XPutBackEvent (XtDisplay (keyboard), &event);
	  message (keyboard, "Aborted key modification...");
	  return NULL;
	}
      else
	{
	  if (event.xany.type == KeymapNotify)
	    keyboard_handle_keymap_notify (keyboard, 0L, &event);
	  else if (event.xany.type == MappingNotify)
	    keyboard_handle_mapping_notify (keyboard, 0L, &event);
	  XtDispatchEvent (&event);
	}
    }
}
/*
**
**	this function tries new getsysinfo() call to find out
**	what the console's "language" parameter is set to
**	to attempt to determine a valid international default
**	based on the configuration rather than the command line.
**	If the new function is not supported, it returns all binary
**	ones (~0). If it works but there is no language parameter
**	set, it returns 0. Otherwise, it returns the ascii character
**	that the environment parameter says, as per the list above
**	(taken from /usr/lib/X11/xdm/Xkeymaps) -bg
*/
#ifndef BSIZ
#define BSIZ 32
#endif
char getInternatKB ()
{
	char buffer [BSIZ];
	int	retval;

	retval =  getsysinfo (GSI_PROM_ENV, buffer, BSIZ, 0, "language");
	if (retval < 0) return ((char) ~0);
	else return (buffer[0]);
}
/*
**	if kbNameFromSys has been appropriately set up by getsysinfo,
**	return the appropriate code for the keyboard. If it has not 
**	been set up, or if we don't understand what the kernel told us,
**	return minus 1. -bg
*/
test_pckb_type()
{
	extern char *knownNames[];
	int	ix;

	if (kbNameFromSys[0] == 0) return -1;	/* DUUHHH... */
	for (ix = 0; knownNames[ix]; ix++)
	{
		if (!(strcmp (kbNameFromSys, knownNames[ix]))) return ix;
	}
	/*
	**	fall through if the system identifies a keyboard that
	**	we don't know about. -bg
	*/
	printf ("system keyboard %s is unknown\n", kbNameFromSys);
	return -1;	/* HUNH? */
}
/*
**	test to see if we're running locally
**	return 1 (TRUE) if we are, 0 (FALSE) otherwise
*/
runningLocal (Display *pdisp)
{
	struct utsname	my;
	int	retval;
	char	dispName[128];	/* DISPLAY = "" goes here */
	char	nodeName[128];	/* name of host system goes here*/
	char	*ptr;	/* utility pointers */
	char	*dst;

	/*
	**	DISPLAY = :{d}.{s} is, by definition, local 
	*/
	ptr = DisplayString(pdisp);
	if (! ptr ) return 0;
	if (*ptr == ':') return 1;
	/*
	**	extract the system's node name from the stucture
	**	filled in by uname() (eliminate domain stuff by 
	**	copying up to but excluding the first piece of
	**	punctuation -- i.e. '.' or ':'); then extract the
	**	display name in the same way.
	*/
	if ((retval = uname (&my) >= 0))
	{
		/*
		**	first do display ('cuz we set up ptr before)
		*/
		dst = dispName;
		while (*ptr && !ispunct(*ptr)) *dst++ = *ptr++;
		*dst = '\0';	/* terminate the string */
		/*
		**	now identify our host node 
		*/
		ptr = my.nodename;
		dst = nodeName;
		while (*ptr && !ispunct (*ptr)) *dst++ = *ptr++;
		*dst = '\0';
		/*
		**	if they're the same, we're running
		**	locally.
		*/
		if (strcmp (dispName, nodeName) == 0) return 1;
	}
	/*
	**	either we can't find out, or we're not local
	*/
	return 0;
}


/*
**	geMinMaxKeys() scans the keyboard table for the min/max scancode.
**	Intent: results of this scan compare with min_keycode and
**	max_keycode as maintained by server to verify that this table
**	represents a physical keyboard that at least COULD be connected
**	to the display.
*/
getMinMaxKeys (kbd, minK, maxK)
	struct keyboard *kbd;
	KeyCode	*minK, *maxK;
{
	int i, j, k, error;

	*minK = 255;
	*maxK = 0;
  	for (i = 0; i < kbd->nrows; i++)
  	{
		struct row *row = &kbd->rows [i];
   		for (j = 0; j < row->nkeys; j++)
		{
	  		struct key *key = &row->keys [j];
	  		if (key->keycode)
	    	{
	      		if (key->keycode < *minK)
					*minK = key->keycode;
	      		if (key->keycode > *maxK)
					*maxK = key->keycode;
	    	}
		}
    }
  	if (*maxK <= *minK) exit (-69); /* can't happen */
}
/*
**	check the keyboard structure's name field to see what
**	it is. This is called to see if the keyboard is in the
**	LK401/LK201/LK421 group or in the PC keyboard group. So
**	it returns:
**		0 if it is a pc 101/102 keyboard (PCXAL, LK443, LK444)
**		1 if it is LK201
**		2 if it is LK401
**		3 if it is LK421
**		4/5 if LK443/4
**		6 if NCD terminal
*/
isLK (struct keyboard *pk)
{
	/*
	**	we currently know about PCXAL, LK401, LK201, LK421,
	**		LK443, LK444
	*/
	if (pk)
	{
		if (toupper (pk->name[0]) == 'N' && toupper (pk->name[1]) == 'C'
			&& toupper (pk->name[2]) == 'D') return KB_NCD;
		if (toupper (pk->name[0]) == 'P' && toupper (pk->name[1]) == 'C') 
		  return KB_PCXAL;
		if (toupper (pk->name[0]) == 'L' && toupper (pk->name[1]) == 'K') 
		{
			/*
			 **	LK44something?
			 */
			if (pk->name[2] == '4' && pk->name[3] == '4') return KB_PCXAL;
			if (pk->name[2] == '2') return KB_LK201; /* LK201 */
			if (pk->name[3] == '2') return KB_LK421; /* LK421 */
			return KB_LK401;	/* LK401 */
		}
		printf ("UNKNOWN keyboard type (%s)!!!\n", pk->name);
	}
	else printf ("WARNING: void key pointer in isLK()!\n");
	return KB_UNKNOWN;
}
/*
**	this code assumes that the tables built into dxkeycaps
**	are for legally-mapped pc keyboards (i.e., where the 
**	min keycode is 8, and the kernel remaps scancode 7 to be
**	scancode 9). It should be called only to provide backward
**	compatibility, when dxkeycaps is running on an older system
**	that does NOT provide the kernel remapping.... -bg
*/
fixMinKey (kbd)
	struct keyboard *kbd;
{
	int i, j, k, error;

	/*
	**	if this isn't pcxal keyboard, don't bother fixing it
	**	'cuz it ain't broke.
	*/
	if (strncmp (kbd->name, "PCXAL", 5)) return 0;
	/*
	**	otherwise, scan the rows looking for the legal 9 keycode and
	**	fudge it back to a 7. This isn't as bad as it looks, since
	**	the offending key is about the second or third key into the
	**	the list of tables....
	*/
  	for (i = 0; i < kbd->nrows; i++)
  	{
		struct row *row = &kbd->rows [i];
   		for (j = 0; j < row->nkeys; j++)
		{
	  		struct key *key = &row->keys [j];
			if (key->keycode == 9)
			{
				key->keycode = 7;
				return 1;
			}
		}
    }
  	return 0;
}
/*
**	build menu of international
**	keyboards
*/
makeInternatKeyboardMenu (Widget kmenu, int baseIndex, Widget *kbd)
{
	int	start, finish;
	int	i;
	Arg	av [20];
	Widget	item, imenu, button;
	char	thisName[128];

	if (baseKeyboards > 1)
	{
		for (i = 0; all_kbds[baseIndex]->name[i] &&
		  (all_kbds[baseIndex]->name[i] != ' '); i++)
			thisName[i] = all_kbds[baseIndex]->name[i];
		thisName[i] = 0;
		/* 
		**	Create submenu pane whose parent is keyboard
		**	 menu pane.  Fill submenu 
		*/
		imenu = XmCreatePulldownMenu (kmenu, "internatlMenu", NULL, 0L);
	}
	else 
	{
		imenu = kmenu;
	}
	XtSetArg (av[0], XmNlabelString,
		  XmStringCreate(all_kbds[baseIndex]->name, def_char_set));
	item = XtCreateManagedWidget ("selectKeyboard", 
		  xmPushButtonWidgetClass, imenu, av, 1);
	XtAddCallback (item, XmNactivateCallback, select_keyboard_cb, *kbd);
	start = baseKeyboards + (15 * baseIndex);
	finish = start + 15;
	if (finish > iCount)
	{
		printf ("something's wrong\n");
		finish = iCount;
	}
	for (i = start; i < finish; i++)
	{
		XtSetArg (av[0], XmNlabelString,
			  XmStringCreate(all_kbds[i]->name, 
				def_char_set));
		item = XtCreateManagedWidget ("selectKeyboard", 
			xmPushButtonWidgetClass, imenu, av, 1);
		XtAddCallback (item, XmNactivateCallback, 
			select_keyboard_cb, *kbd);
	}
	if (baseKeyboards > 1)
	{
		/* 
		**	Populate the keyboard menu with one cascade button 
		**	for the international submenu 
		*/
		button = XtCreateManagedWidget(thisName,
			xmCascadeButtonWidgetClass, kmenu, NULL, 0L);
		XtSetArg(av[0], XmNsubMenuId, imenu);
		XtSetValues(button, av, 1);
	}
}
makeOutsideMenu (Widget kmenu, int baseIndex, Widget *kbd)
{
	int	thisSize, finish;
	int	i;
	Arg	av [20];
	Widget	item, imenu, button;
	char	thisName[128];

	for (i = 0; all_kbds[baseIndex]->name[i] &&
	  (all_kbds[baseIndex]->name[i] != ' '); i++)
		thisName[i] = all_kbds[baseIndex]->name[i];
	thisName[i] = 0;
	thisSize = i;
	/* 
	**	Create submenu pane whose parent is keyboard
	**	 menu pane.  Fill submenu 
	*/
	imenu = XmCreatePulldownMenu (kmenu, "internatlMenu", NULL, 0L);
	XtSetArg (av[0], XmNlabelString,
		  XmStringCreate(all_kbds[baseIndex]->name, def_char_set));
	item = XtCreateManagedWidget ("selectKeyboard", 
		  xmPushButtonWidgetClass, imenu, av, 1);
	XtAddCallback (item, XmNactivateCallback, select_keyboard_cb, *kbd);
	for (i = baseIndex+1; all_kbds[i] 
	  && !strncmp (thisName, all_kbds[i]->name, thisSize); i++)
	{
		XtSetArg (av[0], XmNlabelString,
			  XmStringCreate(all_kbds[i]->name, 
				def_char_set));
		item = XtCreateManagedWidget ("selectKeyboard", 
			xmPushButtonWidgetClass, imenu, av, 1);
		XtAddCallback (item, XmNactivateCallback, 
			select_keyboard_cb, *kbd);
	}
	/* 
	**	Populate the keyboard menu with one cascade button 
	**	for the international submenu 
	*/
	button = XtCreateManagedWidget(thisName,
		xmCascadeButtonWidgetClass, kmenu, NULL, 0L);
	XtSetArg(av[0], XmNsubMenuId, imenu);
	XtSetValues(button, av, 1);
}
/*
 * Not all servers are particularly straightforward about the min
 * and max keycodes of the display's keyboard, at least as reported
 * in the disp->min_/max_keycode fields. So here we scan the keyboard
 * map from what these fields say is the min/max, and check which
 * keycodes are the ASSIGNED minimum and maximum.
 */
getServerMinMax (Display *disp, KeyCode *mink, KeyCode *maxk)
{
	unsigned minKactual, maxKactual;
	KeySym *keysyms;
	int count;
	int howmany;
	int ix;
	int realMin, realMax;
	int iy;
	int flag;
#ifdef ONE_DOT_TWO
	minKactual = (unsigned) disp->min_keycode;
	maxKactual = (unsigned) disp->max_keycode;
#else
	minKactual = (unsigned) ((_XPrivDisplay) disp)->min_keycode;
	maxKactual = (unsigned) ((_XPrivDisplay) disp)->max_keycode;
#endif
	if (minKactual < 8) minKactual = 8;
	howmany = (int) (maxKactual - minKactual + 1);
	realMin = 255;
	realMax = 0;

	/*
	 * for each key, get the mapping and scan the keycodes 
	 * assigned to that key, counting assignments. This
	 * eliminates null keys.
	 */
	for (ix = minKactual; ix <= maxKactual; ix++)
	{
		keysyms = XGetKeyboardMapping (disp, ix, 1, &count);
		flag = 0;				/* counts assigned keycodes */
		for (iy = 0; iy < count; iy++)
		{
			if (keysyms[iy]) flag++; /* not null, so count it */
		}
		if (flag)			/* i.e., if there were assigned keycodes */
		{
			if (ix < realMin)
			  realMin = ix;
			if (ix > realMax)
			  realMax = ix;
		}
		XFree (keysyms);
	}
	*mink = (KeyCode) realMin;
	*maxk = (KeyCode) realMax;
}
/*
**	testValidKeyboard()
**		checks various system resources and returns:
**			TRUE (1) if the keyboard described in struct keyboard
**				*pkeyb is valid for this display
**			FALSE (0) if the keyboard described in struct keyboard
**				is not.
**	the 'int typeFlag' parameter is the return from test_pckb_type()
**	the 'struct keyboard *pkeyb' is a pointer to a keyboard
**	structure describing a particular keyboard.
**	if typeflag is -1, we are: 1) running over the network; or
**		2) running on a system whose kernel doesn't support
**		the new getsysinfo() calls to identify the keyboard.
**		So the logic goes, if typeFlag tells us what to look for,
**		just check the name; else compare the minkeycodes between
**		the display and the pkeyb table. This should check the
**		maxkeycode, too, but this info isn't reliable from the
**		server....
*/
testValidKeyboard (Display *disp, int typeFlag, struct keyboard *pkeyb,
				   int serverMinKey, int serverMaxKey)
{
	static int	minKtable, maxKtable;
	extern int forceAll;
	int	lenme;


	/*
	 * kernel can't distinguish between lk443 and lk444, but all
	 * we really need to know here is whether we have an lk44SOMETHING.
	 */
	if (typeFlag >= 0 && (! forceAll))
	{
		lenme = (typeFlag == KB_LK443) 
		    ? 4
			: strlen (knownNames[typeFlag]);
		if (! strncmp (knownNames [typeFlag],
			pkeyb->name, lenme ))
		{
			return 1;
		}
	}
	else
	{
		getMinMaxKeys (pkeyb, &minKtable, &maxKtable);
		/*
		 * now that we know the correct min and max keys, we can be
		 * somewhat more certain in our comparisons that our internal
		 * tables will at least fall within the exactly correct range
		 * of keycodes for the keyboard attached to the server's 
		 * display.
		 */
		if ((minKtable == serverMinKey) 
			&& (maxKtable == serverMaxKey))
		  return 1;
	}
	return 0;
}


