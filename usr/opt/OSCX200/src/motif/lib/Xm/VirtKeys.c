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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: VirtKeys.c,v $ $Revision: 1.1.6.5 $ $Date: 1993/12/17 21:19:46 $"
#endif
#endif
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1990 MOTOROLA, INC. */
#include      	<stdio.h>
#include	<ctype.h>
#include 	<string.h>
#include	<X11/keysym.h>
#include 	<Xm/VirtKeysP.h>
#include 	<Xm/DisplayP.h>
#include 	<Xm/TransltnsP.h>
#include        <Xm/AtomMgr.h>
#include	<Xm/XmosP.h>
#ifndef X_NOT_STDC_ENV
#include        <stdlib.h>
#endif


#define	done(type, value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}

#define defaultFallbackBindings _XmVirtKeys_fallbackBindingString

#define BUFFERSIZE 2048
#define MAXLINE 256

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static Boolean CvtStringToVirtualBinding() ;
static XmKeyBindingRec * FillBindingsFromDB() ;
static Boolean GetBindingsProperty() ;
static void FindVirtKey() ;
static void LoadVendorBindings() ;
#ifdef DEC_MOTIF_EXTENSION
static String _DEC_GuessKeyboardBindings();
#endif /* DEC_MOTIF_EXTENSION */
#else

static Boolean CvtStringToVirtualBinding( 
                        Display *dpy,
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValuePtr fromVal,
                        XrmValuePtr toVal,
                        XtPointer *closure_ret) ;
static XmKeyBindingRec * FillBindingsFromDB( 
                        Display *dpy,
                        XrmDatabase rdb) ;
static Boolean GetBindingsProperty( 
                        Display *display,
			String property,
                        String *binding) ;
static void FindVirtKey( 
                        XmDisplay xmDisplay,
#if NeedWidePrototypes
                        int keycode,
#else
                        KeyCode keycode,
#endif /* NeedWidePrototypes */
                        Modifiers modifiers,
                        Modifiers *modifiers_return,
                        KeySym *keysym_return) ;
static void LoadVendorBindings(
			Display *display,
			char *path,
			FILE *fp,
			String *binding) ;
#ifdef DEC_MOTIF_EXTENSION
static String _DEC_GuessKeyboardBindings( Display *display );
#endif /* DEC_MOTIF_EXTENSION */

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

static XmKeyBindingRec nullBinding = { 0L, 0 };

static XmVirtualKeysymRec virtualKeysyms[] =
{
    {(String)XmVosfBackSpace	, (KeySym)0x1004FF08	},
    {(String)XmVosfInsert	, (KeySym)0x1004FF63	},
    {(String)XmVosfDelete	, (KeySym)0x1004FFFF	},
    {(String)XmVosfCopy		, (KeySym)0x1004FF02	},
    {(String)XmVosfCut		, (KeySym)0x1004FF03	},
    {(String)XmVosfPaste	, (KeySym)0x1004FF04	},
    {(String)XmVosfAddMode	, (KeySym)0x1004FF31	},
    {(String)XmVosfPrimaryPaste	, (KeySym)0x1004FF32	},
    {(String)XmVosfQuickPaste	, (KeySym)0x1004FF33	},
    {(String)XmVosfPageLeft	, (KeySym)0x1004FF40	},
    {(String)XmVosfPageUp	, (KeySym)0x1004FF41	},
    {(String)XmVosfPageDown	, (KeySym)0x1004FF42	},
    {(String)XmVosfPageRight	, (KeySym)0x1004FF43	},
    {(String)XmVosfEndLine	, (KeySym)0x1004FF57	},
    {(String)XmVosfBeginLine	, (KeySym)0x1004FF58	},
    {(String)XmVosfActivate	, (KeySym)0x1004FF44	},
    {(String)XmVosfMenuBar	, (KeySym)0x1004FF45	},
    {(String)XmVosfClear	, (KeySym)0x1004FF0B	},
    {(String)XmVosfCancel	, (KeySym)0x1004FF69	},
    {(String)XmVosfHelp		, (KeySym)0x1004FF6A	},
    {(String)XmVosfMenu		, (KeySym)0x1004FF67	},
    {(String)XmVosfSelect	, (KeySym)0x1004FF60	},
    {(String)XmVosfUndo		, (KeySym)0x1004FF65	},
    {(String)XmVosfLeft		, (KeySym)0x1004FF51	},
    {(String)XmVosfUp		, (KeySym)0x1004FF52	},
    {(String)XmVosfRight	, (KeySym)0x1004FF53	},
    {(String)XmVosfDown		, (KeySym)0x1004FF54	},
};

static XmDefaultBindingStringRec fallbackBindingStrings[] =
{
    {"Acorn Computers Ltd"			, _XmVirtKeys_acornFallbackBindingString},
    {"Apollo Computer Inc."			, _XmVirtKeys_apolloFallbackBindingString},
    {"DECWINDOWS DigitalEquipmentCorp."		, _XmVirtKeys_decFallbackBindingString},
    {"Data General Corporation  Rev 04"		, _XmVirtKeys_dgFallbackBindingString},
    {"Double Click Imaging, Inc. KeyX"		, _XmVirtKeys_dblclkFallbackBindingString},
    {"Hewlett-Packard Company"			, _XmVirtKeys_hpFallbackBindingString},
    {"International Business Machines"		, _XmVirtKeys_ibmFallbackBindingString},
    {"Intergraph Corporation"			, _XmVirtKeys_ingrFallbackBindingString},
    {"Megatek Corporation"			, _XmVirtKeys_megatekFallbackBindingString},
    {"Motorola Inc. (Microcomputer Division) "	, _XmVirtKeys_motorolaFallbackBindingString},
    {"Silicon Graphics Inc."			, _XmVirtKeys_sgiFallbackBindingString},
    {"Silicon Graphics"				, _XmVirtKeys_sgiFallbackBindingString},
    {"Siemens Munich by SP-4's Hacker Crew"	, _XmVirtKeys_siemensWx200FallbackBindingString},
    {"Siemens Munich (SP-4's hacker-clan)"	, _XmVirtKeys_siemens9733FallbackBindingString},
    {"X11/NeWS - Sun Microsystems Inc."		, _XmVirtKeys_sunFallbackBindingString},
    {"Tektronix, Inc."				, _XmVirtKeys_tekFallbackBindingString},
};


/*ARGSUSED*/
static Boolean 
#ifdef _NO_PROTO
CvtStringToVirtualBinding( dpy, args, num_args, fromVal, toVal, closure_ret )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *num_args ;
        XrmValuePtr fromVal ;
        XrmValuePtr toVal ;
        XtPointer *closure_ret ;
#else
CvtStringToVirtualBinding(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *num_args,
        XrmValuePtr fromVal,
        XrmValuePtr toVal,
        XtPointer *closure_ret )
#endif /* _NO_PROTO */
{
    char 		*str = (char *)fromVal->addr;
    XmKeyBindingRec	keyBindingRec;
    XKeyEvent		event;
    char 		buffer[20];
    int			eventType;
    unsigned            convert_keysym ;
    int			j;
    int			codes_per_sym;
    int			minK, maxK;
    KeySym *		junk;

    XDisplayKeycodes(dpy, &minK, &maxK);
    junk = XGetKeyboardMapping(dpy, minK, (maxK - minK + 1), &codes_per_sym);
    XFree((char *)junk);

    if (_XmMapKeyEvent(str, &eventType, 
			    &convert_keysym,
			    &keyBindingRec.modifiers))
      {
	keyBindingRec.keysym = convert_keysym ;
/****************
 *
 * Here's a nasty bit of code. If some vendor defines one of the standard
 * modifiers to be the mode switch mod, the keysym returned by
 * XtTranslateKey is the mode-shifted one. This may or may not be bogus, but
 * we have to live with it :-(. Soo, we need to translate the keysym to a
 * keycode, then ask someone to translate the combo for us. Why doesn't he
 * use XtTranslateKey, you ask? Well, the code goes to the per-display info
 * for the min and max keycodes, even though they are in the display struct.
 * The pd stuff isn't set up until a map event, though, and that's way too
 * late for us. So, we call XLookupString, which calls XTranslateKey, and
 * get the desired result. And it's just a table lookup, so it's fast enuff
 * for us to use. Pretty cool, huh?
 *
 ****************/
	  event.display = dpy;
	  event.keycode = XKeysymToKeycode(dpy,keyBindingRec.keysym);
/****************
 * In case the guy specifies a symbol that is modified (like HP's Del which
 * is <shift><escape>), we'll find the implied modifers and 'OR' it together 
 * with the explicitly stated modifers.
 ****************/
	  event.state = 0;
	  if (XKeycodeToKeysym(dpy, event.keycode, 0) != keyBindingRec.keysym)
	    for(j=1; j<codes_per_sym; j++)
	      if (XKeycodeToKeysym(dpy, event.keycode, j) == 
		  keyBindingRec.keysym)
	      {
		event.state = 1 << (j-1);
		break;
	      }
	  event.state |= keyBindingRec.modifiers;
	  XLookupString(&event, buffer, 1, &keyBindingRec.keysym, NULL);	  
	  done(XmKeyBindingRec,  keyBindingRec);
      }
    XtDisplayStringConversionWarning(dpy, str, XmRVirtualBinding);
    return False;
}


static XmKeyBindingRec * 
#ifdef _NO_PROTO
FillBindingsFromDB( dpy, rdb )
        Display *dpy ;
        XrmDatabase rdb ;
#else
FillBindingsFromDB(
        Display *dpy,
        XrmDatabase rdb )
#endif /* _NO_PROTO */
{
    XmKeyBindingRec	*keyBindings, *virtBinding;
    XmVirtualKeysym	virtKey;
    XrmName 		xrm_name[2];
    XrmClass 		xrm_class[2];
    XrmRepresentation 	rep_type;
    XrmValue 		value;
    Cardinal		i;

    xrm_class[0] = XrmStringToQuark(XmRVirtualBinding);
    xrm_class[1] = 0;

    keyBindings = (XmKeyBindingRec *)
      XtMalloc(sizeof(XmKeyBindingRec) * XtNumber(virtualKeysyms));

    for (virtKey = virtualKeysyms, virtBinding = keyBindings, i = 0; 
	 i < XtNumber(virtualKeysyms);
	 virtKey++, virtBinding++, i++)
      {
	  xrm_name[0] = XrmStringToQuark(virtKey->name);
	  xrm_name[1] = 0;
	  if (XrmQGetResource(rdb, xrm_name, xrm_class,
			      &rep_type, &value ))
	    {
		if (rep_type == XrmStringToQuark(XmRVirtualBinding))

		  *virtBinding = *(XmKeyBindingRec *)value.addr;

		else if (rep_type == XrmStringToQuark(XmRString)) 
		  {
		      XrmValue toVal;
		      toVal.addr = (XPointer)virtBinding;
		      toVal.size = sizeof(XmKeyBindingRec);
		      if (!XtCallConverter(dpy, 
					  CvtStringToVirtualBinding, 
					  NULL,
					  0,
					  &value, 
					  &toVal,
					  (XtCacheRef*)NULL))
			    *virtBinding = nullBinding;
		  }
		else 
		  *virtBinding = nullBinding;
	    }
	  else
	    *virtBinding = nullBinding;
      }
    return keyBindings;
}


static Boolean 
#ifdef _NO_PROTO
GetBindingsProperty( display, property, binding )
        Display *display;
	String	property;
        String  *binding;
#else
GetBindingsProperty(
	Display *display,
	String	property,
	String	*binding )
#endif /* _NO_PROTO */
{
    char		*prop = NULL;
    Atom		actual_type;
    int			actual_format;
    unsigned long	num_items;
    unsigned long	bytes_after;


    if ( binding == NULL ) return( False );

    XGetWindowProperty (display, 
			RootWindow(display, 0),
			XmInternAtom(display, property, FALSE),
			0, (long)1000000,
			FALSE, XA_STRING,
			&actual_type, &actual_format,
			&num_items, &bytes_after,
			(unsigned char **) &prop);

    if ((actual_type != XA_STRING) ||
	(actual_format != 8) || 
	(num_items == 0))
    {
        if (prop != NULL) XFree(prop);
        return( False );
    }
    else
    {
        *binding = prop;
        return( True );
    }
}

	   
/*
 * This routine is called by the XmDisplay Initialize method to set
 * up the virtual bindings table, XtKeyProc, and event handler.
 */
void 
#ifdef _NO_PROTO
_XmVirtKeysInitialize( widget )
        Widget widget ;
#else
_XmVirtKeysInitialize(
        Widget widget )
#endif /* _NO_PROTO */
{
    XmDisplay xmDisplay = (XmDisplay) widget;
    Display *dpy = XtDisplay(xmDisplay);
    Cardinal i;
    XrmDatabase keyDB;
    String bindingsString;
    String fallbackString = NULL;
    Boolean needXFree = False;

    if ( !XmIsDisplay (widget) ) return;

    bindingsString = xmDisplay->display.bindingsString;
    xmDisplay->display.lastKeyEvent = XtNew(XKeyEvent);

    if (bindingsString == NULL) {

	/* XmNdefaultVirtualBindings not set, try _MOTIF_BINDINGS */

	if (GetBindingsProperty( XtDisplay(xmDisplay),
				 "_MOTIF_BINDINGS",
				 &bindingsString) == True) {
	    needXFree = True;
	}
	else if (GetBindingsProperty( XtDisplay(xmDisplay),
				      "_MOTIF_DEFAULT_BINDINGS",
				      &bindingsString) == True) {
	    needXFree = True;
	}
	else {
	    /* property not set, find a useful fallback */

	    _XmVirtKeysLoadFallbackBindings( XtDisplay(xmDisplay),
					     &fallbackString );
	    bindingsString = fallbackString;
	}
    }

    keyDB = XrmGetStringDatabase( bindingsString );
    xmDisplay->display.bindings = FillBindingsFromDB (XtDisplay(xmDisplay),
							keyDB);
    XrmDestroyDatabase(keyDB);
    if (needXFree) XFree (bindingsString);
    if (fallbackString) XtFree (fallbackString);

    XtSetKeyTranslator(dpy, (XtKeyProc)XmTranslateKey);

 /*
  * The Xt R5 translation manager cache conflicts with our XtKeyProc.
  * A virtual key with modifiers bound will confuse the cache.
  *
  * The keycode_tag[] has one bit for each possible keycode. If a virtual
  * key has modifiers bound, we flag its keycode. In _XmVirtKeysHandler
  * we check the event keycode, and reset the cache to get the correct
  * behavior.
  */
    memset (xmDisplay->display.keycode_tag, 0, XmKEYCODE_TAG_SIZE);
    for (i = 0; i < XtNumber(virtualKeysyms); i++) {
	KeyCode kc = XKeysymToKeycode (dpy,
				       xmDisplay->display.bindings[i].keysym);
	if (kc != 0 && xmDisplay->display.bindings[i].modifiers != 0)
	    xmDisplay->display.keycode_tag[kc/8] |= 1 << (kc % 8);
    }
}


/*
 * This routine is called by the XmDisplay Destroy method to free
 * up the virtual bindings table.
 */
void 
#ifdef _NO_PROTO
_XmVirtKeysDestroy( widget )
        Widget widget ;
#else
_XmVirtKeysDestroy(
        Widget widget )
#endif /* _NO_PROTO */
{
    XmDisplay xmDisplay = (XmDisplay) widget;

    XtFree ((char *)xmDisplay->display.lastKeyEvent);
    XtFree ((char *)xmDisplay->display.bindings);
}


static void 
#ifdef _NO_PROTO
FindVirtKey( xmDisplay, keycode, modifiers, modifiers_return, keysym_return )
        XmDisplay xmDisplay ;
        KeyCode keycode ;
        Modifiers modifiers ;
        Modifiers *modifiers_return ;
        KeySym *keysym_return ;
#else
FindVirtKey(
        XmDisplay xmDisplay,
#if NeedWidePrototypes
        int keycode,
#else
        KeyCode keycode,
#endif /* NeedWidePrototypes */
        Modifiers modifiers,
        Modifiers *modifiers_return,
        KeySym *keysym_return )
#endif /* _NO_PROTO */
{
    XmKeyBinding	keyBindings = xmDisplay->display.bindings;
    Cardinal		i;
    XmKeyBinding	currBinding;
    Modifiers		eventMods;

    /*
     * get the modifiers from the actual event
     */
    eventMods = (Modifiers)(xmDisplay->display.lastKeyEvent->state);

    for (i = 0; i < XtNumber(virtualKeysyms); i++)
    {
        /*
	 * the null binding should not be interpreted as a match
	 * keysym is zero (e.g. pre-edit terminator)
	 */
	 currBinding = (XmKeyBinding) &keyBindings[i];
	 if ((currBinding->modifiers == (modifiers & eventMods)) &&
	     (currBinding->keysym &&
	     (currBinding->keysym == *keysym_return)))  
	 {
	     *keysym_return = virtualKeysyms[i].keysym;
	     break;
	 }
    }
    *modifiers_return |= ControlMask | Mod1Mask;
}


/************************************************************************
 *
 *  _XmVirtKeysHandler
 *
 *  This handler provides all kind of magic. It is added to all widgets.
 *     
 ************************************************************************/
/* ARGSUSED */
void 
#ifdef _NO_PROTO
_XmVirtKeysHandler( widget, client_data, event, dontSwallow )
        Widget widget ;
        XtPointer client_data ;
        XEvent *event ;
        Boolean *dontSwallow ;
#else
_XmVirtKeysHandler(
        Widget widget,
        XtPointer client_data,
        XEvent *event,
        Boolean *dontSwallow )
#endif /* _NO_PROTO */
{
    XmDisplay xmDisplay = (XmDisplay)XmGetXmDisplay (XtDisplay (widget) );
    KeyCode keycode;

    if (widget->core.being_destroyed)
      {
	  *dontSwallow = False;
	  return;
      }
    switch( event->type ) {
      case KeyPress:
	*(xmDisplay->display.lastKeyEvent) = *((XKeyEvent *)event);

	/*
	 * if keycode is tagged as a modified virtual key, reset
	 * the Xt translation manager cache.
	 */
	keycode = ((XKeyEvent *)event)->keycode;
	if ((xmDisplay->display.keycode_tag[keycode/8] & (1 << (keycode % 8)))
									!= 0) {
	    XtSetKeyTranslator (XtDisplay(widget), (XtKeyProc)XmTranslateKey);
	}
	break;
    }
}

void
#ifdef _NO_PROTO
XmTranslateKey( dpy, keycode, modifiers, modifiers_return, keysym_return )
        Display *dpy ;
        KeyCode keycode ;
        Modifiers modifiers ;
        Modifiers *modifiers_return ;
        KeySym *keysym_return ;
#else
XmTranslateKey(
        Display *dpy,
#if NeedWidePrototypes
        unsigned int keycode,
#else
        KeyCode keycode,
#endif /* NeedWidePrototypes */
        Modifiers modifiers,
        Modifiers *modifiers_return,
        KeySym *keysym_return )
#endif /* _NO_PROTO */
{
    XmDisplay xmDisplay = (XmDisplay)XmGetXmDisplay (dpy);

    XtTranslateKey(dpy, keycode, modifiers, modifiers_return, keysym_return);

    FindVirtKey(xmDisplay, keycode, modifiers, modifiers_return,
                keysym_return);
}

void 
#ifdef _NO_PROTO
_XmVirtualToActualKeysym( dpy, virtKeysym, actualKeysymRtn, modifiersRtn )
        Display *dpy ;
        KeySym virtKeysym ;
        KeySym *actualKeysymRtn ;
        Modifiers *modifiersRtn ;
#else
_XmVirtualToActualKeysym(
        Display *dpy,
        KeySym virtKeysym,
        KeySym *actualKeysymRtn,
        Modifiers *modifiersRtn )
#endif /* _NO_PROTO */
{
    Cardinal		i;
    XmKeyBinding	currBinding;
    XmKeyBinding        keyBindings;
    XmDisplay		xmDisplay = (XmDisplay)XmGetXmDisplay (dpy);
    
    keyBindings = xmDisplay->display.bindings;

    for (i = 0; i < XtNumber(virtualKeysyms); i++)
	 {
	     if (virtualKeysyms[i].keysym == virtKeysym)
	       {
		   currBinding = (XmKeyBinding) &keyBindings[i];
		   
		   *actualKeysymRtn = currBinding->keysym;
		   *modifiersRtn = currBinding->modifiers;
		   return;
	       }
	 }
    *actualKeysymRtn = NoSymbol;
    *modifiersRtn = 0;
}


Boolean 
#ifdef _NO_PROTO
_XmVirtKeysLoadFileBindings( fileName, binding )
    char      *fileName;
    String    *binding;
#else
_XmVirtKeysLoadFileBindings(
    char      *fileName,
    String    *binding )
#endif /* _NO_PROTO */
{
    FILE *fileP;
    int offset = 0;
    int count;

    if ((fileP = fopen (fileName, "r")) != NULL) {
	*binding = NULL;
	do {
	    *binding = XtRealloc (*binding, offset + BUFFERSIZE);
	    count = (int)fread (*binding + offset, 1, BUFFERSIZE, fileP);
	    offset += count;
	} while (count == BUFFERSIZE);
	(*binding)[offset] = '\0';

	/* trim unused buffer space */
	*binding = XtRealloc (*binding, offset + 1);

	return (True);
    }
    else {
	return (False);
    }
}


static void 
#ifdef _NO_PROTO
LoadVendorBindings(display, path, fp, binding )
    Display   *display;
    char      *path;
    FILE      *fp;
    String    *binding;
#else
LoadVendorBindings(
    Display   *display,
    char      *path,
    FILE      *fp,
    String    *binding )
#endif /* _NO_PROTO */
{
    char buffer[MAXLINE];
    char *bindFile;
    char *vendor;
    char *vendorV;
    char *ptr;
    char *start;

    vendor = ServerVendor(display);
    vendorV = XtMalloc (strlen(vendor) + 20); /* assume rel.# is < 19 digits */
    sprintf (vendorV, "%s %d", vendor, VendorRelease(display));

    while (fgets (buffer, MAXLINE, fp) != NULL) {
	ptr = buffer;
	while (*ptr != '"' && *ptr != '!' && *ptr != '\0') ptr++;
	if (*ptr != '"') continue;
	start = ++ptr;
	while (*ptr != '"' && *ptr != '\0') ptr++;
	if (*ptr != '"') continue;
	*ptr = '\0';
	if ((strcmp (start, vendor) == 0) || (strcmp (start, vendorV) == 0)) {
	    ptr++;
	    while (isspace(*ptr) && *ptr != '\0') ptr++;
	    if (*ptr == '\0') continue;
	    start = ptr;
	    while (!isspace(*ptr) && *ptr != '\n' && *ptr != '\0') ptr++;
	    *ptr = '\0';
	    bindFile = _XmOSBuildFileName (path, start);
	    if (_XmVirtKeysLoadFileBindings (bindFile, binding)) {
		XtFree (bindFile);
		break;
	    }
	    XtFree (bindFile);
	}
    }
    XtFree (vendorV);
    return;
}


int 
#ifdef _NO_PROTO
_XmVirtKeysLoadFallbackBindings( display, binding )
    Display	*display;
    String	*binding;
#else
_XmVirtKeysLoadFallbackBindings(
    Display	*display,
    String	*binding )
#endif /* _NO_PROTO */
{
    XmDefaultBindingString currDefault;
    int i;
    FILE *fp;
    char *homeDir;
    char *fileName;
    char *bindDir;
    static char xmbinddir_fallback[] = XMBINDDIR_FALLBACK;

    *binding = NULL;

    /* load .motifbind - necessary, if mwm and xmbind are not used */

    homeDir = _XmOSGetHomeDirName();
    fileName = _XmOSBuildFileName (homeDir, MOTIFBIND);
    _XmVirtKeysLoadFileBindings (fileName, binding);
    XtFree (fileName);

    /* look for a match in the user's xmbind.alias */

    if (*binding == NULL) {
	fileName = _XmOSBuildFileName (homeDir, XMBINDFILE);
	if ((fp = fopen (fileName, "r")) != NULL) {
	    LoadVendorBindings (display, homeDir, fp, binding);
	    fclose (fp);
	}
	XtFree (fileName);
    }

    if (*binding != NULL) {

	/* set the user property for future Xm applications */

	XChangeProperty (display, RootWindow(display, 0),
		XInternAtom (display, "_MOTIF_BINDINGS", False),
		XA_STRING, 8, PropModeReplace,
		(unsigned char *)*binding, strlen(*binding));
	return (0);
    }

    /* look for a match in the system xmbind.alias */

    if (*binding == NULL) {
	if ((bindDir = getenv(XMBINDDIR)) == NULL)
	    bindDir = xmbinddir_fallback;
	fileName = _XmOSBuildFileName (bindDir, XMBINDFILE);
	if ((fp = fopen (fileName, "r")) != NULL) {
	    LoadVendorBindings (display, bindDir, fp, binding);
	    fclose (fp);
	}
	XtFree (fileName);
    }

#ifdef DEC_MOTIF_EXTENSION
    /* At this point, we've checked all the normal user & system customization
     * and the next step is to use the hardcoded fallbacks.  Make one last
     * attempt to guess what kind of keyboard we're dealing with so that we
     * can try to do the right thing for LK*01 and PC-style keyboards.
     */

    if (*binding == NULL) {
	*binding = _DEC_GuessKeyboardBindings(display);
    }
#endif /* DEC_MOTIF_EXTENSION */

    /* check hardcoded fallbacks (for 1.1 bc) */

    if (*binding == NULL) for (i = 0, currDefault = fallbackBindingStrings;
         i < XtNumber(fallbackBindingStrings);
         i++, currDefault++) {
	if (strcmp(currDefault->vendorName, ServerVendor(display)) == 0) {
	    *binding = XtMalloc (strlen (currDefault->defaults) + 1);
	    strcpy (*binding, currDefault->defaults);
	    break;
	}
    }

    /* use generic fallback bindings */

    if (*binding == NULL) {
	*binding = XtMalloc (strlen (defaultFallbackBindings) + 1);
	strcpy (*binding, defaultFallbackBindings);
    }

    /* set the fallback property for future Xm applications */

    XChangeProperty (display, RootWindow(display, 0),
		XInternAtom (display, "_MOTIF_DEFAULT_BINDINGS", False),
		XA_STRING, 8, PropModeReplace,
		(unsigned char *)*binding, strlen(*binding));

    return (0);
}

#ifdef DEC_MOTIF_EXTENSION
static String
#ifdef _NO_PROTO
_DEC_GuessKeyboardBindings( display )
    Display	*display;
#else
_DEC_GuessKeyboardBindings( Display *display )
#endif /* _NO_PROTO */
{
    String	binding = NULL;

    /* If not talking to a DECwindows server, bail out */
    if ((strncmp(ServerVendor(display), "DECWINDOWS", 10) != 0) &&
	(strncmp(ServerVendor(display), "DECwindows", 10) != 0))
	return NULL;

    if (   ( XKeysymToKeycode(display, XK_Scroll_Lock) != NoSymbol )
	&& ( XKeysymToKeycode(display, XK_Pause) != NoSymbol )
	&& ( XKeysymToKeycode(display, XK_Print) != NoSymbol ) ) {
	/*
	 * Appears to be a PC-style keyboard.  Now which keymap is loaded?
	 * A PC-style keyboard in "LK*01 compatibility mode" turned on
	 * causes the key label "Home" to be mapped to Find (among other
	 * things).
	 * %%% At this point, we've decided not to worry about eXcursion on
	 * %%% older DEC pc's with the LK250 keyboard, mostly because we can't
	 * %%% find one to test with.
	 */
	if (  ( XKeysymToKeycode(display, XK_Find) != NoSymbol )
	    &&( XKeysymToKeycode(display, XK_Home) == NoSymbol ) ) {
	    /* PC-style keyboard with LK*01 function keys mapped */
	    binding = _XmVirtKeys_decFallbackBindingString;
	} else {
	    /* PC-style keyboard in standard PC mode */
	    binding = _XmVirtKeys_decpcFallbackBindingString;
	}
    } else {
	/*
	 * Appears to be some flavor of LK keyboard.  VMS doesn't declare
	 * an Escape keysym, so check for that.  It's the only difference in
	 * the default bindings.
	 */
	if ( XKeysymToKeycode(display, XK_Escape) != NoSymbol ) {
	    binding = _XmVirtKeys_decFallbackBindingString;
	} else {
	    binding = _XmVirtKeys_declkf11FallbackBindingString;
	}
    }

    return binding;
}
#endif /* DEC_MOTIF_EXTENSION */
