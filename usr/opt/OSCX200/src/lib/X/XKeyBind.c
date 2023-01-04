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
/* $XConsortium: XKeyBind.c,v 11.67 92/05/19 11:23:14 converse Exp $ */
/* Copyright 1985, 1987, Massachusetts Institute of Technology */

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

/* Beware, here be monsters (still under construction... - JG */

#define NEED_EVENTS
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#define XK_MISCELLANY
#define XK_LATIN1
#define XK_LATIN2
#define XK_LATIN3
#define XK_LATIN4
#include <X11/keysymdef.h>
#ifdef DEC
#define DEC_COMPOSE_SUPPORT /* support for Int'l compose key */
#include  <X11/DECkeysym.h>
#include  "MuteKeysym.h" /* AIX, HP & SUN dead keys */
#endif /* DEC */
#include <stdio.h>

#define AllMods (ShiftMask|LockMask|ControlMask| \
		 Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask)

static ComputeMaskFromKeytrans();
static int Initialize();
static void XConvertCase();

#ifdef DEC_COMPOSE_SUPPORT
#define IsISOControlKey(ks) ((ks) >= XK_2 && (ks) <= XK_8)

#define IsValidControlKey(ks)   (((ks)>=XK_A && (ks)<=XK_asciitilde || \
                (ks)==XK_space || (ks)==XK_Delete) && \
                ((ks)!=0))

#define COMPOSE_LED 2
#define BellVolume 0

static unsigned int numLockMask;  /* Needs to be in dpy struct? (kbm) */
typedef KeySym (*StateProc)();

/*
 * macros to classify XKeyEvent state field
 */

#define IsShift(state) (((state) & ShiftMask) != 0)
#define IsLock(state) (((state) & LockMask) != 0)
#define IsControl(state) (((state) & ControlMask) != 0)
#define IsMod1(state) (((state) & Mod1Mask) != 0)
#define IsMod2(state) (((state) & Mod2Mask) != 0)
#define IsMod3(state) (((state) & Mod3Mask) != 0)
#define IsMod4(state) (((state) & Mod4Mask) != 0)
#define IsMod5(state) (((state) & Mod5Mask) != 0)

/*
 * Access XComposeStatus structure using appropriate names, as
 * the current field names do not describe their actual usage in
 * this case
 */
#define ComposeState(status) ((status)->chars_matched)
#define ComposeKey(status) ((status)->compose_ptr)

/* key is two key compose initiator if:
 *  - DEC private keysym for two-key compose accent was pressed
 *  - HP, SUN, AIX private keysym for two-key compose accent was pressed
 *  - the key wasn't intended as a Control-Key combination
 *    (Control-Mod1, or Mod1 is ok tho')
 */

int IsAccentKey(ks, event)
KeySym ks;
XKeyEvent *event;
{
    if((ks < 0x10000) ||
       (IsControl(event->state) && !IsMod1(event->state)))
	return(0);
    if((ks >> 8) == 0x1000FE) /* DEC private */
	return(ks &  0xFF);
    switch(ks) {
    /* AIX keys */
    case XK_dead_circumflex:	return(0x5e);
    case XK_dead_grave:		return(0x60);
    case XK_dead_tilde:		return(0x7e);
    case XK_dead_diaeresis:	return(0x22);
#if 0 /* need to verify that we have a match for this */
    case XK_dead_macron:	return(0xaf);
#endif
    case XK_dead_degree:	return(0xb0);
    case XK_dead_acute:		return(0x27);
    case XK_dead_cedilla:	return(0x2c);
    /* HP */
    case XK_mute_acute:		return(0x27);
    case XK_mute_grave:		return(0x60);
    case XK_mute_asciicircum:	return(0x5e);
    case XK_mute_diaeresis:	return(0x22);
    case XK_mute_asciitilde:	return(0x7e);
    /* Sun */
    case SunXK_FA_Grave:	return(0x60);
    case SunXK_FA_Circum:	return(0x5e);
    case SunXK_FA_Tilde:	return(0x7e);
    case SunXK_FA_Acute:	return(0x27);
    case SunXK_FA_Diaeresis:	return(0x22);
    case SunXK_FA_Cedilla:	return(0x2c);
    default:
	return(0);
    }
}

/*
 * key starts compose sequence if :
 * the key pressed is Multi_key
 * The cord pressed is MOD1 and Space (not shifted or cntl)
 */

#define IsComposeKey(ks, event) (( ks==XK_Multi_key ||\
    (ks==XK_space && \
    (IsMod1(event->state) && !IsControl(event->state) && \
                        !IsShift(event->state)) )) \
                ? True : False)

/*
 * alpha Keysym classification and case conversion macros
 */

#define IsUpperCaseKey(ks) (((ks >= XK_A && ks <= XK_Z) || \
               (ks >= XK_Agrave && ks <= XK_Odiaeresis) || \
               (ks >= XK_Ooblique && ks <= XK_Thorn)) \
                        ? True : False)

#define IsLowerCaseKey(ks) (((ks >= XK_a && ks <= XK_z) || \
               (ks >= XK_ssharp && ks <= XK_odiaeresis) || \
               (ks >= XK_oslash && ks <= XK_ydiaeresis)) \
                        ? True : False)

#define IsAlphaKey(ks) (IsLowerCaseKey(ks) || IsUpperCaseKey(ks))

#define ToUpperCaseKey(ks) \
    ((IsLowerCaseKey(ks) && ks != XK_ssharp && ks != XK_ydiaeresis ) \
            ? (ks-0x20) : ks)

#define ToLowerCaseKey(ks) (IsUpperCaseKey(ks) ? (ks+0x20) : ks)

StateProc *state_handler = NULL;
int nstate_handlers = 0;
void DXSetComposeHandler();
StateProc DXGetComposeHandler();

extern int IsCancelComposeKey();
extern void SetLed();
extern KeySym NormalKey();
extern KeySym FirstComposeKey();
extern KeySym SecondComposeKey();
extern KeySym LookupComposeSequence();

#endif /* DEC_COMPOSE_SUPPORT */

struct _XKeytrans {
	struct _XKeytrans *next;/* next on list */
	char *string;		/* string to return when the time comes */
	int len;		/* length of string (since NULL is legit)*/
	KeySym key;		/* keysym rebound */
	unsigned int state;	/* modifier state */
	KeySym *modifiers;	/* modifier keysyms you want */
	int mlen;		/* length of modifier list */
};

static KeySym
KeyCodetoKeySym(dpy, keycode, col)
    register Display *dpy;
    KeyCode keycode;
    int col;
{
    register int per = dpy->keysyms_per_keycode;
    register KeySym *syms;
    KeySym lsym, usym;

    if ((col < 0) || ((col >= per) && (col > 3)) ||
	((int)keycode < dpy->min_keycode) || ((int)keycode > dpy->max_keycode))
      return NoSymbol;

    syms = &dpy->keysyms[(keycode - dpy->min_keycode) * per];
    if (col < 4) {
	if (col > 1) {
	    while ((per > 2) && (syms[per - 1] == NoSymbol))
		per--;
	    if (per < 3)
		col -= 2;
	}
	if ((per <= (col|1)) || (syms[col|1] == NoSymbol)) {
	    XConvertCase(dpy, syms[col&~1], &lsym, &usym);
	    if (!(col & 1))
		return lsym;
	    else if (usym == lsym)
		return NoSymbol;
	    else
		return usym;
	}
    }
    return syms[col];
}

#if NeedFunctionPrototypes
KeySym
XKeycodeToKeysym(Display *dpy,
#if NeedWidePrototypes
		 unsigned int kc,
#else
		 KeyCode kc,
#endif
		 int col)
#else
KeySym
XKeycodeToKeysym(dpy, kc, col)
    Display *dpy;
    KeyCode kc;
    int col;
#endif
{
    if ((! dpy->keysyms) && (! Initialize(dpy)))
	return NoSymbol;
    return KeyCodetoKeySym(dpy, kc, col);
}

KeyCode
XKeysymToKeycode(dpy, ks)
    Display *dpy;
    KeySym ks;
{
    register int i, j;

    if ((! dpy->keysyms) && (! Initialize(dpy)))
	return (KeyCode) 0;
    for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	for (i = dpy->min_keycode; i <= dpy->max_keycode; i++) {
	    if (KeyCodetoKeySym(dpy, (KeyCode) i, j) == ks)
		return i;
	}
    }
    return 0;
}

KeySym
XLookupKeysym(event, col)
    register XKeyEvent *event;
    int col;
{
    if ((! event->display->keysyms) && (! Initialize(event->display)))
	return NoSymbol;
    return KeyCodetoKeySym(event->display, event->keycode, col);
}

static int
InitModMap(dpy)
    Display *dpy;
{
    register XModifierKeymap *map;
    register int i, j, n;
    KeySym sym;
    register struct _XKeytrans *p;

    if (! (dpy->modifiermap = map = XGetModifierMapping(dpy)))
	return 0;
    dpy->free_funcs->modifiermap = XFreeModifiermap;
    if ((! dpy->keysyms) && (! Initialize(dpy)))
	return 0;
    LockDisplay(dpy);
    /* If any Lock key contains Caps_Lock, then interpret as Caps_Lock,
     * else if any contains Shift_Lock, then interpret as Shift_Lock,
     * else ignore Lock altogether.
     */
    dpy->lock_meaning = NoSymbol;
    /* Lock modifiers are in the second row of the matrix */
    n = 2 * map->max_keypermod;
    for (i = map->max_keypermod; i < n; i++) {
	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    sym = KeyCodetoKeySym(dpy, map->modifiermap[i], j);
	    if (sym == XK_Caps_Lock) {
		dpy->lock_meaning = XK_Caps_Lock;
		break;
	    } else if (sym == XK_Shift_Lock) {
		dpy->lock_meaning = XK_Shift_Lock;
	    }
	}
    }
    /* Now find any Mod<n> modifier acting as the Group modifier */
    dpy->mode_switch = 0;
    n *= 4; /* now end of mod5 table (e.g. 8 * map->max_keypermod) */
    for (i = 3*map->max_keypermod; i < n; i++) {
	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    sym = KeyCodetoKeySym(dpy, map->modifiermap[i], j);
	    if (sym == XK_Mode_switch)
		dpy->mode_switch |= 1 << (i / map->max_keypermod);
	}
    }
    numLockMask = 0;
    /* Now find any Mod<n> modifier acting as the NumLock modifier */
    for (i = 3*map->max_keypermod; i < n; i++) {
	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    sym = KeyCodetoKeySym(dpy, map->modifiermap[i], j);
	    if (sym == XK_Num_Lock)
		numLockMask |= 1 << (i / map->max_keypermod);
	}
    }
    for (p = dpy->key_bindings; p; p = p->next)
	ComputeMaskFromKeytrans(dpy, p);
    UnlockDisplay(dpy);
    return 1;
}

XRefreshKeyboardMapping(event)
    register XMappingEvent *event;
{

    if(event->request == MappingKeyboard) {
	/* XXX should really only refresh what is necessary
	 * for now, make initialize test fail
	 */
	LockDisplay(event->display);
	if (event->display->keysyms) {
	     Xfree ((char *)event->display->keysyms);
	     event->display->keysyms = NULL;
	}
	UnlockDisplay(event->display);
    }
    if(event->request == MappingModifier) {
	LockDisplay(event->display);
	if (event->display->modifiermap) {
	    XFreeModifiermap(event->display->modifiermap);
	    event->display->modifiermap = NULL;
	}
	UnlockDisplay(event->display);
	/* go ahead and get it now, since initialize test may not fail */
	(void) InitModMap(event->display);
    }
}

static int
Initialize(dpy)
    Display *dpy;
{
    int per, n;
    KeySym *keysyms;

    /* 
     * lets go get the keysyms from the server.
     */
    if (!dpy->keysyms) {
	n = dpy->max_keycode - dpy->min_keycode + 1;
	keysyms = XGetKeyboardMapping (dpy, (KeyCode) dpy->min_keycode,
				       n, &per);
	/* keysyms may be NULL */
	if (! keysyms) return 0;

	LockDisplay(dpy);
	dpy->keysyms = keysyms;
	dpy->keysyms_per_keycode = per;
	UnlockDisplay(dpy);
    }
    if (!dpy->modifiermap)
        return InitModMap(dpy);
    return 1;
}

/*ARGSUSED*/
static void
XConvertCase(dpy, sym, lower, upper)
    Display *dpy;
    register KeySym sym;
    KeySym *lower;
    KeySym *upper;
{
    *lower = sym;
    *upper = sym;
    switch(sym >> 8) {
    case 0:
	if ((sym >= XK_A) && (sym <= XK_Z))
	    *lower += (XK_a - XK_A);
	else if ((sym >= XK_a) && (sym <= XK_z))
	    *upper -= (XK_a - XK_A);
	else if ((sym >= XK_Agrave) && (sym <= XK_Odiaeresis))
	    *lower += (XK_agrave - XK_Agrave);
	else if ((sym >= XK_agrave) && (sym <= XK_odiaeresis))
	    *upper -= (XK_agrave - XK_Agrave);
	else if ((sym >= XK_Ooblique) && (sym <= XK_Thorn))
	    *lower += (XK_oslash - XK_Ooblique);
	else if ((sym >= XK_oslash) && (sym <= XK_thorn))
	    *upper -= (XK_oslash - XK_Ooblique);
	break;
#ifdef XK_LATIN2
    case 1:
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym == XK_Aogonek)
	    *lower = XK_aogonek;
	else if (sym >= XK_Lstroke && sym <= XK_Sacute)
	    *lower += (XK_lstroke - XK_Lstroke);
	else if (sym >= XK_Scaron && sym <= XK_Zacute)
	    *lower += (XK_scaron - XK_Scaron);
	else if (sym >= XK_Zcaron && sym <= XK_Zabovedot)
	    *lower += (XK_zcaron - XK_Zcaron);
	else if (sym == XK_aogonek)
	    *upper = XK_Aogonek;
	else if (sym >= XK_lstroke && sym <= XK_sacute)
	    *upper -= (XK_lstroke - XK_Lstroke);
	else if (sym >= XK_scaron && sym <= XK_zacute)
	    *upper -= (XK_scaron - XK_Scaron);
	else if (sym >= XK_zcaron && sym <= XK_zabovedot)
	    *upper -= (XK_zcaron - XK_Zcaron);
	else if (sym >= XK_Racute && sym <= XK_Tcedilla)
	    *lower += (XK_racute - XK_Racute);
	else if (sym >= XK_racute && sym <= XK_tcedilla)
	    *upper -= (XK_racute - XK_Racute);
	break;
#endif
#ifdef XK_LATIN3
    case 2:
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Hstroke && sym <= XK_Hcircumflex)
	    *lower += (XK_hstroke - XK_Hstroke);
	else if (sym >= XK_Gbreve && sym <= XK_Jcircumflex)
	    *lower += (XK_gbreve - XK_Gbreve);
	else if (sym >= XK_hstroke && sym <= XK_hcircumflex)
	    *upper -= (XK_hstroke - XK_Hstroke);
	else if (sym >= XK_gbreve && sym <= XK_jcircumflex)
	    *upper -= (XK_gbreve - XK_Gbreve);
	else if (sym >= XK_Cabovedot && sym <= XK_Scircumflex)
	    *lower += (XK_cabovedot - XK_Cabovedot);
	else if (sym >= XK_cabovedot && sym <= XK_scircumflex)
	    *upper -= (XK_cabovedot - XK_Cabovedot);
	break;
#endif
#ifdef XK_LATIN4
    case 3:
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Rcedilla && sym <= XK_Tslash)
	    *lower += (XK_rcedilla - XK_Rcedilla);
	else if (sym >= XK_rcedilla && sym <= XK_tslash)
	    *upper -= (XK_rcedilla - XK_Rcedilla);
	else if (sym == XK_ENG)
	    *lower = XK_eng;
	else if (sym == XK_eng)
	    *upper = XK_ENG;
	else if (sym >= XK_Amacron && sym <= XK_Umacron)
	    *lower += (XK_amacron - XK_Amacron);
	else if (sym >= XK_amacron && sym <= XK_umacron)
	    *upper -= (XK_amacron - XK_Amacron);
	break;
#endif
    }
}


static int
#ifdef DEC_COMPOSE_SUPPORT
XTranslateKey(dpy, keycode, modifiers, modifiers_return, keysym_return,
              lsym_return, usym_return)
    register Display *dpy;
    KeyCode keycode;
    register unsigned int modifiers;
    unsigned int *modifiers_return;
    KeySym *keysym_return,*lsym_return,*usym_return;
#else /* no DEC_COMPOSE_SUPPORT */
XTranslateKey(dpy, keycode, modifiers, modifiers_return, keysym_return)
    register Display *dpy;
    KeyCode keycode;
    register unsigned int modifiers;
    unsigned int *modifiers_return;
    KeySym *keysym_return;
#endif /* DEC_COMPOSE_SUPPORT */
{
    int per;
    register KeySym *syms;
    KeySym sym = 0, lsym = 0, usym = 0;

    if ((! dpy->keysyms) && (! Initialize(dpy)))
	return 0;
    *modifiers_return = (ShiftMask|LockMask) | dpy->mode_switch | numLockMask;
    if (((int)keycode < dpy->min_keycode) || ((int)keycode > dpy->max_keycode))
    {
	*keysym_return = NoSymbol;
	return 1;
    }
    per = dpy->keysyms_per_keycode;
    syms = &dpy->keysyms[(keycode - dpy->min_keycode) * per];
    while ((per > 2) && (syms[per - 1] == NoSymbol))
	per--;
    if ((per > 2) && !(modifiers & ControlMask) &&
	(modifiers & dpy->mode_switch)) {
	syms += 2;
	per -= 2;
    }
    if ((modifiers & numLockMask) && (IsKeypadKey(syms[0]) || IsKeypadKey(syms[1]))) {
        /* numlock case appl to keypad keysyms only */
	if ((per == 1) || ((sym = syms[1]) == NoSymbol))
	    sym = syms[0];
	XConvertCase(dpy, sym, &lsym, &usym);
	if ((modifiers & ShiftMask) || !IsKeypadKey(sym))
	    XConvertCase(dpy, syms[0], &lsym, &usym);
	*keysym_return = lsym;
    }
    else if (!(modifiers & ShiftMask) && (!(modifiers & LockMask) || 
        (dpy->lock_meaning == NoSymbol))) {
        /* normal un-shifted case */
	if ((per == 1) || (syms[1] == NoSymbol))
	    XConvertCase(dpy, syms[0], keysym_return, &usym);
	else {
#ifdef DEC_COMPOSE_SUPPORT
	XConvertCase(dpy, syms[0], &lsym, &usym); /* KLee 8/24/90 */
#endif DEC_COMPOSE_SUPPORT
	    *keysym_return = syms[0];
	}
    } else if (!(modifiers & LockMask) || (dpy->lock_meaning != XK_Caps_Lock)) {
        /* Shift case */
	if ((per == 1) || ((usym = syms[1]) == NoSymbol))
	    XConvertCase(dpy, syms[0], &lsym, &usym);
	*keysym_return = usym;
    } else {
        /* CapsLock Case */
	if ((per == 1) || ((sym = syms[1]) == NoSymbol))
	    sym = syms[0];
	XConvertCase(dpy, sym, &lsym, &usym);
	if (!(modifiers & ShiftMask) && (sym != syms[0]) &&
	    ((sym != usym) || (lsym == usym)))
	    XConvertCase(dpy, syms[0], &lsym, &usym);
	*keysym_return = usym;
    }
    if (*keysym_return == XK_VoidSymbol)
	*keysym_return = NoSymbol;
#ifdef DEC_COMPOSE_SUPPORT
    *lsym_return = lsym;
    *usym_return = usym;
#endif
    return 1;
}

static int
#ifdef DEC_COMPOSE_SUPPORT
XTranslateKeySym(dpy, symbol, lsym, usym, modifiers, buffer, nbytes)
    Display *dpy;
    register KeySym symbol, lsym, usym;
    unsigned int modifiers;
    char *buffer;
    int nbytes;
#else
XTranslateKeySym(dpy, symbol, modifiers, buffer, nbytes)
    Display *dpy;
    register KeySym symbol;
    unsigned int modifiers;
    char *buffer;
    int nbytes;
#endif
{
#ifdef DEC_COMPOSE_SUPPORT
    KeySym ckey;
#endif
    register struct _XKeytrans *p; 
    int length;
    unsigned long hiBytes;
    register unsigned char c;

    /*
     * initialize length = 1 ;
     */
    length = 1;

    if (!symbol)
	return 0;
    /* see if symbol rebound, if so, return that string. */
    for (p = dpy->key_bindings; p; p = p->next) {
	if (((modifiers & AllMods) == p->state) && (symbol == p->key)) {
	    length = p->len;
	    if (length > nbytes) length = nbytes;
	    bcopy (p->string, buffer, length);
	    return length;
	}
    }
    /* try to convert to Latin-1, handling control */
    hiBytes = symbol >> 8;
    if (!(nbytes &&
	  ((hiBytes == 0) ||
	   ((hiBytes == 0xFF) &&
	    (((symbol >= XK_BackSpace) && (symbol <= XK_Clear)) ||
	     (symbol == XK_Return) ||
	     (symbol == XK_Escape) ||
	     (symbol == XK_KP_Space) ||
	     (symbol == XK_KP_Tab) ||
	     (symbol == XK_KP_Enter) ||
	     ((symbol >= XK_KP_Multiply) && (symbol <= XK_KP_9)) ||
	     (symbol == XK_KP_Equal) ||
#ifdef DEC_COMPOSE_SUPPORT
         (symbol == XK_Scroll_Lock) ||
         (symbol == DXK_Remove) ||
         (symbol == NoSymbol) ||
#endif /* DEC_COMPOSE_SUPPORT */
	     (symbol == XK_Delete))))))
	return 0;

    /* if X keysym, convert to ascii by grabbing low 7 bits */
    if (symbol == XK_KP_Space)
	c = XK_space & 0x7F; /* patch encoding botch */
    else if (symbol == XK_hyphen)
	c = XK_minus & 0xFF; /* map to equiv character */
    else if (hiBytes == 0xFF)
	c = symbol & 0x7F;
    else
	c = symbol & 0xFF;
    /* only apply Control key if it makes sense, else ignore it */
    if (modifiers & ControlMask) {
#ifdef DEC_COMPOSE_SUPPORT
    if (!(IsKeypadKey(lsym) || lsym==XK_Return || lsym==XK_Tab)) {
        if (IsISOControlKey(lsym)) ckey = lsym;
        else if (IsISOControlKey(usym)) ckey = usym;
        else if (lsym == XK_question) ckey = lsym;
        else if (usym == XK_question) ckey = usym;
        else if (IsValidControlKey(lsym)) ckey = lsym;
        else if (IsValidControlKey(usym)) ckey = usym;
        else length = 0;

        if (length != 0) {
        if (ckey == XK_2) c = '\000';
        else if (ckey >= XK_3 && ckey <= XK_7)
            c = (char)(ckey-('3'-'\033'));
        else if (ckey == XK_8) c = '\177';
        else if (ckey == XK_Delete) c = '\030';
        else if (ckey == XK_question) c = '\037';
        else if (ckey == XK_quoteleft) c = '\036';  /* KLee 1/24/91 */
        else c = (char)(ckey & 0x1f);
        }
    }
#else /* no DEC_COMPOSE_SUPPORT */
	if ((c >= '@' && c < '\177') || c == ' ') c &= 0x1F;
	else if (c == '2') c = '\000';
	else if (c >= '3' && c <= '7') c -= ('3' - '\033');
	else if (c == '8') c = '\177';
	else if (c == '/') c = '_' & 0x1F;
#endif /* DEC_COMPOSE_SUPPORT */
    }
    buffer[0] = c;
    return 1;
}
  
/*ARGSUSED*/
int
XLookupString (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;	/* buffer */
    int nbytes;	/* space in buffer for characters */
    KeySym *keysym;
    XComposeStatus *status;	/* not implemented */
{
    unsigned int modifiers;
    KeySym symbol;
	int length = 0;

#ifdef DEC_COMPOSE_SUPPORT
    KeySym lsym,usym;
    int state;
    if (! XTranslateKey(event->display, event->keycode, event->state,
      &modifiers, &symbol, &lsym, &usym))
	return 0;
/*
 * route state processing (compose, or other..)
 */

    if (status != NULL) {
    if (state_handler == NULL)
        DXSetComposeHandler(0,NULL);  /* default to compose handler */

    state = ComposeState(status);
    if (state >= 0 && state < nstate_handlers) /* call handler for state */
        symbol = (* state_handler[state])(status, symbol, event);
    }
    if (keysym)
	*keysym = symbol;

    /* arguable whether to use (event->state & ~modifiers) here */
     length = XTranslateKeySym(event->display, symbol, lsym, usym, event->state,
			    buffer, nbytes);
#else /* no DEC_COMPOSE_SUPPORT */
    if (! XTranslateKey(event->display, event->keycode, event->state,
      &modifiers, &symbol))
    return 0;
    if (keysym)
    *keysym = symbol;

    /* arguable whether to use (event->state & ~modifiers) here */
    length = XTranslateKeySym(event->display, symbol, event->state,
                buffer, nbytes);
#endif DEC_COMPOSE_SUPPORT
	return (length);
}

static void
_XFreeKeyBindings (dpy)
    Display *dpy;
{
    register struct _XKeytrans *p, *np;

    for (p = dpy->key_bindings; p; p = np) {
	np = p->next;
	Xfree(p->string);
	Xfree((char *)p->modifiers);
	Xfree((char *)p);
    }   
}

#if NeedFunctionPrototypes
XRebindKeysym (
    Display *dpy,
    KeySym keysym,
    KeySym *mlist,
    int nm,		/* number of modifiers in mlist */
    _Xconst unsigned char *str,
    int nbytes)
#else
XRebindKeysym (dpy, keysym, mlist, nm, str, nbytes)
    Display *dpy;
    KeySym keysym;
    KeySym *mlist;
    int nm;		/* number of modifiers in mlist */
    unsigned char *str;
    int nbytes;
#endif
{
    register struct _XKeytrans *tmp, *p;
    int nb;

    if ((! dpy->keysyms) && (! Initialize(dpy)))
	return;
    LockDisplay(dpy);
    tmp = dpy->key_bindings;
    nb = sizeof(KeySym) * nm;

    if ((! (p = (struct _XKeytrans *) Xmalloc( sizeof(struct _XKeytrans)))) ||
	((! (p->string = (char *) Xmalloc( (unsigned) nbytes))) && 
	 (nbytes > 0)) ||
	((! (p->modifiers = (KeySym *) Xmalloc( (unsigned) nb))) &&
	 (nb > 0))) {
	if (p) {
	    if (p->string) Xfree(p->string);
	    if (p->modifiers) Xfree((char *) p->modifiers);
	    Xfree((char *) p);
	}
	UnlockDisplay(dpy);
	return;
    }

    dpy->key_bindings = p;
    dpy->free_funcs->key_bindings = _XFreeKeyBindings;
    p->next = tmp;	/* chain onto list */
    bcopy ((char *) str, p->string, nbytes);
    p->len = nbytes;
    bcopy ((char *) mlist, (char *) p->modifiers, nb);
    p->key = keysym;
    p->mlen = nm;
    ComputeMaskFromKeytrans(dpy, p);
    UnlockDisplay(dpy);
    return;
}

/*
 * given a KeySym, returns the first keycode containing it, if any.
 */
static CARD8
FindKeyCode(dpy, code)
    register Display *dpy;
    register KeySym code;
{

    register KeySym *kmax = dpy->keysyms + 
	(dpy->max_keycode - dpy->min_keycode + 1) * dpy->keysyms_per_keycode;
    register KeySym *k = dpy->keysyms;
    while (k < kmax) {
	if (*k == code)
	    return (((k - dpy->keysyms) / dpy->keysyms_per_keycode) +
		    dpy->min_keycode);
	k += 1;
	}
    return 0;
}

	
/*
 * given a list of modifiers, computes the mask necessary for later matching.
 * This routine must lookup the key in the Keymap and then search to see
 * what modifier it is bound to, if any.  Sets the AnyModifier bit if it
 * can't map some keysym to a modifier.
 */
static
ComputeMaskFromKeytrans(dpy, p)
    Display *dpy;
    register struct _XKeytrans *p;
{
    register int i;
    register CARD8 code;
    register XModifierKeymap *m = dpy->modifiermap;

    p->state = AnyModifier;
    for (i = 0; i < p->mlen; i++) {
	/* if not found, then not on current keyboard */
	if ((code = FindKeyCode(dpy, p->modifiers[i])) == 0)
		return;
	/* code is now the keycode for the modifier you want */
	{
	    register int j = m->max_keypermod<<3;

	    while ((--j >= 0) && (code != m->modifiermap[j]))
		;
	    if (j < 0)
		return;
	    p->state |= (1<<(j/m->max_keypermod));
	}
    }
    p->state &= AllMods;
}

#ifdef DEC_COMPOSE_SUPPORT
void DXSetComposeHandler (state, handler)
    int state;
    StateProc handler;
{
    int i;
    StateProc *new;

    LockMutex(&xkeybind_mutex);
    if (handler==NULL)		/* reset to standard compose handler */
	{
	if (state_handler != NULL) Xfree (state_handler);
	state_handler = (StateProc *) Xmalloc (3 * sizeof (StateProc));
	nstate_handlers=3;
	state_handler[0] = NormalKey;
	state_handler[1] = FirstComposeKey;
	state_handler[2] = SecondComposeKey;
        UnlockMutex(&xkeybind_mutex);
	return;
	}
    if (state < 0) 
	{
        UnlockMutex(&xkeybind_mutex);
	return;
	}
    if (state < nstate_handlers)		/* no new entries necessary */
	{
	state_handler[state] = handler;
        UnlockMutex(&xkeybind_mutex);
	return;
	}

    new = (StateProc *) Xmalloc ((state+1) * sizeof (StateProc));
    for (i=0; i < nstate_handlers; i++)	/* rebuild table with new entry */
	new[i] = state_handler [i];
    new[state] = handler;
    Xfree (state_handler);
    state_handler=new;
    nstate_handlers = state+1;
    UnlockMutex(&xkeybind_mutex);
}

StateProc DXGetComposeHandler (state)
    int state;
{
    StateProc h;

    if (state < 0 || state >= nstate_handlers || state_handler == NULL)
	h = NULL;
    else
	h = state_handler[state];

    return (h);
}


/************************************************************************
 *
 *
 * Compose handling routines - default as compose handlers 0,1,2
 * 
 * 
 ************************************************************************/

#define NORMAL_KEY_STATE 0
#define FIRST_COMPOSE_KEY_STATE 1
#define SECOND_COMPOSE_KEY_STATE 2

KeySym NormalKey (status, symbol, event)
    XComposeStatus *status;
    KeySym symbol;
    XKeyEvent *event;

{
    int key;
    if (IsComposeKey (symbol, event))	/* start compose sequence	*/
	{
	if (event->type != KeyPress) return (symbol);
	SetLed (event->display,COMPOSE_LED, LedModeOn);
	ComposeState(status)=FIRST_COMPOSE_KEY_STATE;
	return (NoSymbol);
	}
    if (key = IsAccentKey (symbol, event))	
	    /* start two-key compose sequence */
	{
	if (event->type != KeyPress) return (symbol);
	SetLed (event->display,COMPOSE_LED, LedModeOn);
	ComposeState(status)=SECOND_COMPOSE_KEY_STATE;
	ComposeKey(status)= (char *)key; 	/* save first key */
	return (NoSymbol);
	}
    return (symbol);
}


KeySym FirstComposeKey (status, symbol, event)	
    XComposeStatus *status;
    KeySym symbol;
    XKeyEvent *event;

{
    KeySym key;

    if (event->type != KeyPress) return (symbol);
    if (IsModifierKey (symbol)) return (symbol); /* ignore shift etc. */
    if (IsCancelComposeKey (&symbol, event))	/* cancel sequence */
	{
	SetLed (event->display,COMPOSE_LED, LedModeOff);
	ComposeState(status)=NORMAL_KEY_STATE;
	return (symbol);
	}
    if (IsComposeKey (symbol, event))		/* restart sequence ?? */
	{
	return (NoSymbol);			/* no state change necessary */
	}
    if(key = IsAccentKey(symbol, event))
	symbol = key; /* use accents as normal keys */

    ComposeKey(status)= (char *)symbol;		/* save key pressed */
    ComposeState(status)=SECOND_COMPOSE_KEY_STATE;
    return (NoSymbol);
}


KeySym SecondComposeKey (status, symbol, event)
    XComposeStatus *status;
    KeySym symbol;
    XKeyEvent *event;

{
    KeySym key;

    if (event->type != KeyPress) return (symbol);
    if (IsModifierKey (symbol)) return (symbol);	/* ignore shift etc. */
    if (IsComposeKey (symbol, event))		/* restart sequence ? */
	{
	ComposeState(status)=FIRST_COMPOSE_KEY_STATE;
	return (NoSymbol);
	}
    SetLed (event->display,COMPOSE_LED, LedModeOff);
    if (IsCancelComposeKey (&symbol, event))	/* cancel sequence ? */
	{
	ComposeState(status)=NORMAL_KEY_STATE;
	return (symbol);
	}
    if(key = IsAccentKey(symbol, event))
	symbol = key; /* use accents as normal keys */

    if ((symbol = LookupComposeSequence ((KeySym)ComposeKey(status), symbol))
								==NoSymbol)
	{ /* invalid compose sequence */
	XBell(event->display, BellVolume);
	}
    ComposeState(status)=NORMAL_KEY_STATE;		/* reset to normal state 
*/
    return (symbol);
}


/*
 * matches two keysyms entered with the resulting valid keysym in
 * compose key sequence table
 */
#define COMPOSE_SEQUENCE(first, second, result) (first),(second),(result),

KeySym LookupComposeSequence (ks1, ks2)

KeySym ks1, ks2;
{
int i; 
static struct
	{
	KeySym first;
	KeySym second;
	KeySym result;
	} compose_table[] =
{
#include "iso_compose.h"	/* table of compose key sequences */
NoSymbol, NoSymbol, NoSymbol	/* terminate list		  */
};


/* sequential search... optimize for larger compose-key tables... */

    i=0;
    while (compose_table[i].first != NoSymbol)
	{
	if (compose_table[i].first == ks1 && compose_table[i].second == ks2)
		return (compose_table[i].result);
	i++;
	}
    return (NoSymbol);		/* no matching sequence */
}


/*
 * routine determines
 *	1) whether key event should cancel a compose sequence
 *	2) whether cancelling key event should be processed or ignored
 */

int IsCancelComposeKey(symbol, event)
    KeySym *symbol;
    XKeyEvent *event;
{
    if (*symbol==XK_Delete && !IsControl(event->state) &&
						!IsMod1(event->state)) {
	*symbol=NoSymbol;  /* cancel compose sequence, and ignore key */
	return (True);
    }
    if (IsComposeKey(*symbol, event)) return (False);
    return (
	IsControl (event->state) ||
	IsMod1(event->state) ||
	IsKeypadKey (*symbol) ||
	IsFunctionKey (*symbol) ||
	IsMiscFunctionKey (*symbol) ||
	*symbol == DXK_Remove ||
	IsPFKey (*symbol) ||
	IsCursorKey (*symbol) ||
	*symbol >= XK_Tab && *symbol < XK_Multi_key
		? True : False);	/* cancel compose sequence and pass */
					/* cancelling key through	    */
}


/*
 *	set specified keyboard LED on or off
 */

void SetLed (dpy, num, state)
    Display *dpy;
    int num;
    int state;
{
    XKeyboardControl led_control;

    led_control.led_mode = state;
    led_control.led = num;
    XChangeKeyboardControl (dpy, KBLed | KBLedMode,	&led_control);
}
#endif /* DEC_COMPOSE_SUPPORT */

#if defined(DEC)
/* 
 * need access to private data in display
 * needed until MIT addes access to this element
 */
unsigned int ModeSwitchOfDisplay(dpy)
Display *dpy;
{
    return(dpy->mode_switch);
}
#endif
