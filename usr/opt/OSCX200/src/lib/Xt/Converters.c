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
/* $XConsortium: Converters.c,v 1.85 91/07/23 15:38:18 converse Exp $ */
/*LINTLIBRARY*/

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* Conversion.c - implementations of resource type conversion procs */

#include	"IntrinsicI.h"
#include	"StringDefs.h"
#include	<stdio.h>
#include        <X11/cursorfont.h>
#include	<X11/keysym.h>
#include	<X11/Xlocale.h>

#ifdef DEC_EXTENSION
#include "I18N.h"
#endif

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

static Const String XtNwrongParameters = "wrongParameters";
static Const String XtNconversionError = "conversionError";
static Const String XtNmissingCharsetList = "missingCharsetList";

/* Representation types */

#define XtQAtom		XrmPermStringToQuark(XtRAtom)
#define XtQCursor	XrmPermStringToQuark(XtRCursor)
#define XtQDisplay	XrmPermStringToQuark(XtRDisplay)
#define XtQFile		XrmPermStringToQuark(XtRFile)
#define XtQFloat	XrmPermStringToQuark(XtRFloat)
#ifdef STRING_TO_GEOMETRY
#define XtQGeometry	XrmPermStringToQuark(XtRGeometry)
#endif
#define XtQInitialState	XrmPermStringToQuark(XtRInitialState)
#define XtQPixmap	XrmPermStringToQuark(XtRPixmap)
#define XtQShort	XrmPermStringToQuark(XtRShort)
#define XtQUnsignedChar	XrmPermStringToQuark(XtRUnsignedChar)
#define XtQVisual	XrmPermStringToQuark(XtRVisual)

static XrmQuark  XtQBool;
static XrmQuark  XtQBoolean;
static XrmQuark  XtQColor;
static XrmQuark  XtQDimension;
static XrmQuark  XtQFont;
static XrmQuark  XtQFontSet;
static XrmQuark  XtQFontStruct;
static XrmQuark  XtQInt;
static XrmQuark  XtQPixel;
static XrmQuark  XtQPosition;
XrmQuark  _XtQString;

void _XtConvertInitialize()
{
    XtQBool		= XrmPermStringToQuark(XtRBool);
    XtQBoolean		= XrmPermStringToQuark(XtRBoolean);
    XtQColor		= XrmPermStringToQuark(XtRColor);
    XtQDimension	= XrmPermStringToQuark(XtRDimension);
    XtQFont		= XrmPermStringToQuark(XtRFont);
    XtQFontSet		= XrmPermStringToQuark(XtRFontSet);
    XtQFontStruct	= XrmPermStringToQuark(XtRFontStruct);
    XtQInt		= XrmPermStringToQuark(XtRInt);
    XtQPixel		= XrmPermStringToQuark(XtRPixel);
    XtQPosition		= XrmPermStringToQuark(XtRPosition);
    _XtQString		= XrmPermStringToQuark(XtRString);
}

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


#ifdef DEC_EXTENSION

/* These routines support font fallback. */

#ifdef DEBUG
void debugfontfields(data)
    I18NFontNamePtr    data;
{
    int i;

    printf ("TK version:    major = %d; minor = %d\n");
    printf ("XFLD version:  major = %d; minor = %d\n");


    for (i = 0; i < NUMOFI18NFIELDS; i++)
    {
        switch (i)
        {
            case I18NFOUNDRY:
                printf("Foundry: ");
                break;

            case I18NFAMILY:
                printf("Family: ");
                break;

            case I18NWEIGHT:
                printf("Weight: ");
                break;

            case I18NSLANT:
                printf("Slant: ");
                break;

            case I18NSETWIDTH:
                printf("SetWidth: ");
                break;

            case I18NADDSTYLE:
                printf("AddStyle: ");
                break;

            case I18NPIXELSIZE:
                printf("PixelSize: ");
                break;

            case I18NPOINTSIZE:
                printf("PointSize: ");
                break;

            case I18NXRES:
                printf("XRes: ");
                break;

            case I18NYRES:
                printf("YRes: ");
                break;

            case I18NSPACING:
                printf("Spacing: ");
                break;
            case I18NAVGWIDTH:
                printf("AvgWidth: ");
                break;

            case I18NCHARSET:
                printf("Charset: ");
                break;

            case I18NENCODING:
                printf("Encoding: ");
                break;
            default:
              printf("unknown field\n");
              return;
        }

        if (data->fields[i] != NULL)
          printf("%s\n", data->fields[i]);
        else printf ("NULL\n");
    }

}
#endif


static void freefontfields(data)
    I18NFontNamePtr data;
{
    int i;

    for (i = 0; i < NUMOFI18NFIELDS; i++)
    {
        if (data->fields[i] != NULL)
        {
            XtFree(data->fields[i]);
            data->fields[i] = NULL;
        }
    }
}


static int getnexttoken(ptr, token, len)
    char *ptr;
    char **token;
    int  *len;
{

    char *tempptr;
    int  i;
    int  j;


    *token = NULL;
    tempptr = ptr;
    i = 0;
    while ((tempptr[i] != '\0') && ((tempptr[i] != '-') || 
                                    ((tempptr[i] == '-') && (i == 0))))
       i++;

    if (i == 0)
       return (FALSE);

    *token = (char *) XtMalloc(i+1);
    strncpy(*token, tempptr, i);
    (*token)[i] = '\0';

    for (j = 0; j < i; j++)
        (*token)[j] = tolower((*token)[j]);

    *len = i;
    return (TRUE);

}
static char *construct_new_name(old, new)
    I18NFontNamePtr	old;
    I18NFontNamePtr	new;
{

    char *newname;
    int  newlen;
    int  len;
    int  i;


    /* first calculate the length of the string we need; remember the hyphens*/
    newlen = 0;
    for (i = 0; i < NUMOFI18NFIELDS; i++)
    {
        if (new->fields[i] != NULL)
            len = strlen(new->fields[i]);
        newlen += (len + 1);
    }

    newname = (char *) XtMalloc(newlen + 1);
    if (new->fields[0] != NULL)
        strcpy(newname, new->fields[0]);

    for (i = 1; i < NUMOFI18NFIELDS; i++)
    {
        if (new->fields[i] != NULL)
           strcat(newname, new->fields[i]);
    }
    freefontfields(old);
    return (newname);
}




static Boolean parse_fontname(fontname, fontfields)
    char 		*fontname;
    I18NFontNamePtr	*fontfields;
{

    char *ptr;
    char *token;
    int  newlen,i;

    *fontfields = (I18NFontNamePtr) XtMalloc(sizeof(I18NFontNameRec));
    if (*fontfields == NULL)
         return (FALSE);

    for (i = 0; i < NUMOFI18NFIELDS; i++)
        (*fontfields)->fields[i] = NULL;

    /* First, check that all the necessary fields are there. */

    ptr = fontname;
    for (i = 0; i < NUMOFI18NFIELDS; i++)
    {
        if (!getnexttoken(ptr, &token, &newlen))
        {
             freefontfields(*fontfields);
             return (FALSE);
        }
        ptr += newlen;

        if (((i == I18NFAMILY) || (i == I18NWEIGHT) || (i == I18NSLANT) ||
             (i == I18NSETWIDTH) || (i == I18NCHARSET) || 
             (i == I18NENCODING)) &&
            ((token[0] == '\0') || (token[0] == '*') || (token[0] == '?')))
        {
            freefontfields(*fontfields);
            return (FALSE);
        }
        (*fontfields)->fields[i] = token;
    }
    (*fontfields)->TKmajor = I18NTKMAJOR;
    (*fontfields)->TKminor = I18NTKMINOR;
    (*fontfields)->XFLDmajor = I18NXFLDMAJOR;
    (*fontfields)->XFLDminor = I18NXFLDMINOR;

    return (TRUE);
}



char *_Xt_FontFallback(fontname)
    char *fontname;
{

    char		*newfontname;
    I18NFontNamePtr	fontfields;
    I18NFontNamePtr	newfields;

    newfields = NULL;
    newfontname = NULL;
    if (parse_fontname(fontname, &fontfields))
    {
        newfields = (I18NFontNamePtr) XtMalloc(sizeof(I18NFontNameRec));

        if (I18nRemapFontname(fontfields, newfields))
            newfontname = (char *) construct_new_name(fontfields, newfields);
    }

    /* we don't have a remapped name, just return "fixed" */

    if (!newfontname)
    {
        newfontname = (char *) XtMalloc(strlen("fixed") + 1);
        strcpy(newfontname, "fixed");
        newfontname[strlen("fixed")] = '\0';
    }

    freefontfields(fontfields);
    if (newfields != NULL)
        XtFree((char *)newfields);

    return (newfontname);
}

XFontStruct *_Xt_LoadQueryFont(d, fontname)
    Display *d;
    char    *fontname;

{

    XFontStruct *font;
    char *remapname;

    /* try to open the font */

    font = XLoadQueryFont(d, fontname);

    if (font != NULL)
       return (font);

    /* No such luck.  Try remapping it.  If remap fails, just return fixed. */


    remapname = _Xt_FontFallback(fontname);
    if (remapname != NULL)
    {
         font = XLoadQueryFont(d, remapname);
         XtFree(remapname);
    }
    if (!font)
         font = XLoadQueryFont(d, "fixed");
    return (font);
}

#endif /* DEC_EXTENSION */

#if NeedFunctionPrototypes
void XtDisplayStringConversionWarning(
    Display* dpy,
    _Xconst char* from,
    _Xconst char* toType
    )
#else
void XtDisplayStringConversionWarning(dpy, from, toType)
    Display* dpy;
    String from, toType;
#endif
{
#ifndef NO_MIT_HACKS	
    /* Allow suppression of conversion warnings. %%%  Not specified. */

    static enum {Check, Report, Ignore} report_it = Check;

    if (report_it == Check) {
	XrmDatabase rdb = XtDatabase(dpy);
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;
	xrm_name[0] = XrmPermStringToQuark( "stringConversionWarnings" );
	xrm_name[1] = 0;
	xrm_class[0] = XrmPermStringToQuark( "StringConversionWarnings" );
	xrm_class[1] = 0;
	if (XrmQGetResource( rdb, xrm_name, xrm_class,
			     &rep_type, &value ))
	{
	    if (rep_type == XtQBoolean)
		report_it = *(Boolean*)value.addr ? Report : Ignore;
	    else if (rep_type == _XtQString) {
		XrmValue toVal;
		Boolean report;
		toVal.addr = (XPointer)&report;
		toVal.size = sizeof(Boolean);
		if (XtCallConverter(dpy, XtCvtStringToBoolean, (XrmValuePtr)NULL,
				    (Cardinal)0, &value, &toVal,
				    (XtCacheRef*)NULL))
		    report_it = report ? Report : Ignore;
	    }
	    else report_it = Report;
	}
	else report_it = Report;
    }

    if (report_it == Report) {
#endif /* ifndef NO_MIT_HACKS */
	String params[2];
	Cardinal num_params = 2;
	params[0] = (String)from;
	params[1] = (String)toType;
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		   XtNconversionError,"string",XtCXtToolkitError,
		   "Cannot convert string \"%s\" to type %s",
		    params,&num_params);
#ifndef NO_MIT_HACKS	
    }
#endif /* ifndef NO_MIT_HACKS */
}

#if NeedFunctionPrototypes
void XtStringConversionWarning(
    _Xconst char* from,
    _Xconst char* toType
    )
#else
void XtStringConversionWarning(from, toType)
    String from, toType;
#endif
{
	String params[2];
	Cardinal num_params = 2;
	params[0] = (String)from;
	params[1] = (String)toType;
	XtWarningMsg(XtNconversionError,"string",XtCXtToolkitError,
		   "Cannot convert string \"%s\" to type %s",
		    params,&num_params);
}

static int CompareISOLatin1();


static Boolean IsInteger(string, value)
    String string;
    int *value;
{
    Boolean foundDigit = False;
    Boolean isNegative = False;
    Boolean isPositive = False;
    int val = 0;
    char ch;
    /* skip leading whitespace */
    while ((ch = *string) == ' ' || ch == '\t') string++;
    while (ch = *string++) {
	if (ch >= '0' && ch <= '9') {
	    val *= 10;
	    val += ch - '0';
	    foundDigit = True;
	    continue;
	}
	if (ch == ' ' || ch == '\t') {
	    if (!foundDigit) return False;
	    /* make sure only trailing whitespace */
	    while (ch = *string++) {
		if (ch != ' ' && ch != '\t')
		    return False;
	    }
	    break;
	}
	if (ch == '-' && !foundDigit && !isNegative && !isPositive) {
	    isNegative = True;
	    continue;
	}
	if (ch == '+' && !foundDigit && !isNegative && !isPositive) {
	    isPositive = True;
	    continue;
	}
	return False;
    }
    if (ch == '\0') {
	if (isNegative)
	    *value = -val;
	else
	    *value = val;
	return True;
    }
    return False;
}


/*ARGSUSED*/
Boolean XtCvtIntToBoolean(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtIntToBoolean",XtCXtToolkitError,
                  "Integer to Boolean conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    done(Boolean, (*(int *)fromVal->addr != 0));
}


/*ARGSUSED*/
Boolean XtCvtIntToShort(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtIntToShort",XtCXtToolkitError,
                  "Integer to Short conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    done(short, (*(int *)fromVal->addr));
}


/*ARGSUSED*/
Boolean XtCvtStringToBoolean(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    String str = (String)fromVal->addr;
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToBoolean",XtCXtToolkitError,
                  "String to Boolean conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);

    if (   (CompareISOLatin1(str, "true") == 0)
	|| (CompareISOLatin1(str, "yes") == 0)
	|| (CompareISOLatin1(str, "on") == 0)
	|| (CompareISOLatin1(str, "1") == 0))	done( Boolean, True );

    if (   (CompareISOLatin1(str, "false") == 0)
	|| (CompareISOLatin1(str, "no") == 0)
	|| (CompareISOLatin1(str, "off") == 0)
	|| (CompareISOLatin1(str, "0") == 0))	done( Boolean, False );

    XtDisplayStringConversionWarning(dpy, str, XtRBoolean);
    return False;
}


/*ARGSUSED*/
Boolean XtCvtIntToBool(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtIntToBool",XtCXtToolkitError,
                  "Integer to Bool conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    done(Bool, (*(int *)fromVal->addr != 0));
}


/*ARGSUSED*/
Boolean XtCvtStringToBool(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    String str = (String)fromVal->addr;
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		XtNwrongParameters,"cvtStringToBool",
		XtCXtToolkitError,
                 "String to Bool conversion needs no extra arguments",
                  (String *)NULL, (Cardinal *)NULL);

    if (   (CompareISOLatin1(str, "true") == 0)
	|| (CompareISOLatin1(str, "yes") == 0)
	|| (CompareISOLatin1(str, "on") == 0)
	|| (CompareISOLatin1(str, "1") == 0))	done( Bool, True );

    if (   (CompareISOLatin1(str, "false") == 0)
	|| (CompareISOLatin1(str, "no") == 0)
	|| (CompareISOLatin1(str, "off") == 0)
	|| (CompareISOLatin1(str, "0") == 0))	done( Bool, False );

    XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRBool);
    return False;
}

XtConvertArgRec Const colorConvertArgs[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.screen),
     sizeof(Screen *)},
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.colormap),
     sizeof(Colormap)}
};


/* ARGSUSED */
Boolean XtCvtIntToColor(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{    
    XColor	c;
    Screen	*screen;
    Colormap	colormap;

    if (*num_args != 2) {
      XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	 XtNwrongParameters,"cvtIntOrPixelToXColor",XtCXtToolkitError,
         "Pixel to color conversion needs screen and colormap arguments",
          (String *)NULL, (Cardinal *)NULL);
      return False;
    }
    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);
    c.pixel = *(int *)fromVal->addr;

    XQueryColor(DisplayOfScreen(screen), colormap, &c);
    done(XColor, c);
}


Boolean XtCvtStringToPixel(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    String	    str = (String)fromVal->addr;
    XColor	    screenColor;
    XColor	    exactColor;
    Screen	    *screen;
    XtPerDisplay    pd = _XtGetPerDisplay(dpy);
    Colormap	    colormap;
    Status	    status;
    String          params[1];
    Cardinal	    num_params=1;

    if (*num_args != 2) {
     XtAppWarningMsg(pd->appContext, XtNwrongParameters, "cvtStringToPixel",
		     XtCXtToolkitError,
	"String to pixel conversion needs screen and colormap arguments",
        (String *)NULL, (Cardinal *)NULL);
     return False;
    }

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);

    if (CompareISOLatin1(str, XtDefaultBackground) == 0) {
	*closure_ret = False;
	if (pd->rv) done(Pixel, BlackPixelOfScreen(screen))
	else	    done(Pixel, WhitePixelOfScreen(screen));
    }
    if (CompareISOLatin1(str, XtDefaultForeground) == 0) {
	*closure_ret = False;
	if (pd->rv) done(Pixel, WhitePixelOfScreen(screen))
        else	    done(Pixel, BlackPixelOfScreen(screen));
    }

    status = XAllocNamedColor(DisplayOfScreen(screen), colormap,
			      (char*)str, &screenColor, &exactColor);

    if (status == 0) {
	String msg, type;
#ifdef DEC_EXTENSION
	if (strncmp(str, "DXmDynamic", 9) == 0)
	{
	    Atom type;
	    unsigned long nitems, left;
	    unsigned long *colors = NULL;
	    int format, i, index = -1;
	    Atom name_atom = XInternAtom (dpy, str, TRUE);
	    Pixel pix;

	    XGetWindowProperty (dpy, RootWindowOfScreen(screen),
				XInternAtom (dpy, "DXM_DYNAMIC_COLORS", TRUE),
				0, 999999, FALSE, AnyPropertyType,
				&type, &format, &nitems, &left, (unsigned char **)&colors);

	    for (i=0; ((index < 0) && (i < nitems)); i+=2)
		if ((Atom) colors[i] == name_atom)
		    index = i;

	    if (index >= 0)
	    {
		pix = (Pixel) colors[index+1];
		if (colors != NULL)
		    XFree (colors);
		*closure_ret = False;
		done(Pixel, pix);
	    }

	    if (colors != NULL)
		XFree (colors);
	}
#endif
	params[0] = str;
	/* Server returns a specific error code but Xlib discards it.  Ugh */
	if (XLookupColor(DisplayOfScreen(screen), colormap, (char*)str,
			 &exactColor, &screenColor)) {
	    type = "noColormap";
	    msg = "Cannot allocate colormap entry for \"%s\"";
	}
	else {
	    type = "badValue";
	    msg = "Color name \"%s\" is not defined";
	}

	XtAppWarningMsg(pd->appContext, type, "cvtStringToPixel",
			XtCXtToolkitError, msg, params, &num_params);
	*closure_ret = False;
	return False;
    } else {
	*closure_ret = (char*)True;
        done(Pixel, screenColor.pixel);
    }
}

/* ARGSUSED */
static void FreePixel(app, toVal, closure, args, num_args)
    XtAppContext app;
    XrmValuePtr	toVal;
    XtPointer	closure;
    XrmValuePtr	args;
    Cardinal	*num_args;
{
    Screen	    *screen;
    Colormap	    colormap;

    if (*num_args != 2) {
     XtAppWarningMsg(app, XtNwrongParameters,"freePixel",XtCXtToolkitError,
	"Freeing a pixel requires screen and colormap arguments",
        (String *)NULL, (Cardinal *)NULL);
     return;
    }

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);

    if (closure) {
	XFreeColors( DisplayOfScreen(screen), colormap,
		     (unsigned long*)toVal->addr, 1, (unsigned long)0
		    );
    }
}


/* no longer used by Xt, but it's in the spec */
XtConvertArgRec Const screenConvertArg[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.screen),
     sizeof(Screen *)}
};

/*ARGSUSED*/
static void FetchDisplayArg(widget, size, value)
    Widget widget;
    Cardinal *size;
    XrmValue* value;
{
    if (widget == NULL)
	XtErrorMsg("missingWidget", "fetchDisplayArg", XtCXtToolkitError,
		   "FetchDisplayArg called without a widget to reference",
		   (String*)NULL, (Cardinal*)NULL);
        /* can't return any useful Display and caller will de-ref NULL,
	   so aborting is the only useful option */

    value->size = sizeof(Display*);
    value->addr = (XPointer)&DisplayOfScreen(XtScreenOfObject(widget));
}

static XtConvertArgRec Const displayConvertArg[] = {
    {XtProcedureArg, (XtPointer)FetchDisplayArg, 0},
};

#ifdef DEC_EXTENSION

/*
 *  These variables save the DECW$CURSOR font so that we don't need
 *  to load it each time a call to CvTStringToCursor is made with
 *  a decw_ cursor.
 */
static Display *decw_saved_display = NULL;
static XFontStruct *decw_saved_font = NULL;

#endif

/*ARGSUSED*/
Boolean XtCvtStringToCursor(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;

    XtPointer	*closure_ret;
{
    static Const struct _CursorName {
	Const char	*name;
	unsigned int	shape;
    } cursor_names[] = {
			{"X_cursor",		XC_X_cursor},
			{"arrow",		XC_arrow},
			{"based_arrow_down",	XC_based_arrow_down},
			{"based_arrow_up",	XC_based_arrow_up},
			{"boat",		XC_boat},
			{"bogosity",		XC_bogosity},
			{"bottom_left_corner",	XC_bottom_left_corner},
			{"bottom_right_corner",	XC_bottom_right_corner},
			{"bottom_side",		XC_bottom_side},
			{"bottom_tee",		XC_bottom_tee},
			{"box_spiral",		XC_box_spiral},
			{"center_ptr",		XC_center_ptr},
			{"circle",		XC_circle},
			{"clock",		XC_clock},
			{"coffee_mug",		XC_coffee_mug},
			{"cross",		XC_cross},
			{"cross_reverse",	XC_cross_reverse},
			{"crosshair",		XC_crosshair},
			{"diamond_cross",	XC_diamond_cross},
			{"dot",			XC_dot},
			{"dotbox",		XC_dotbox},
			{"double_arrow",	XC_double_arrow},
			{"draft_large",		XC_draft_large},
			{"draft_small",		XC_draft_small},
			{"draped_box",		XC_draped_box},
			{"exchange",		XC_exchange},
			{"fleur",		XC_fleur},
			{"gobbler",		XC_gobbler},
			{"gumby",		XC_gumby},
			{"hand1",		XC_hand1},
			{"hand2",		XC_hand2},
			{"heart",		XC_heart},
			{"icon",		XC_icon},
			{"iron_cross",		XC_iron_cross},
			{"left_ptr",		XC_left_ptr},
			{"left_side",		XC_left_side},
			{"left_tee",		XC_left_tee},
			{"leftbutton",		XC_leftbutton},
			{"ll_angle",		XC_ll_angle},
			{"lr_angle",		XC_lr_angle},
			{"man",			XC_man},
			{"middlebutton",	XC_middlebutton},
			{"mouse",		XC_mouse},
			{"pencil",		XC_pencil},
			{"pirate",		XC_pirate},
			{"plus",		XC_plus},
			{"question_arrow",	XC_question_arrow},
			{"right_ptr",		XC_right_ptr},
			{"right_side",		XC_right_side},
			{"right_tee",		XC_right_tee},
			{"rightbutton",		XC_rightbutton},
			{"rtl_logo",		XC_rtl_logo},
			{"sailboat",		XC_sailboat},
			{"sb_down_arrow",	XC_sb_down_arrow},
			{"sb_h_double_arrow",	XC_sb_h_double_arrow},
			{"sb_left_arrow",	XC_sb_left_arrow},
			{"sb_right_arrow",	XC_sb_right_arrow},
			{"sb_up_arrow",		XC_sb_up_arrow},
			{"sb_v_double_arrow",	XC_sb_v_double_arrow},
			{"shuttle",		XC_shuttle},
			{"sizing",		XC_sizing},
			{"spider",		XC_spider},
			{"spraycan",		XC_spraycan},
			{"star",		XC_star},
			{"target",		XC_target},
			{"tcross",		XC_tcross},
			{"top_left_arrow",	XC_top_left_arrow},
			{"top_left_corner",	XC_top_left_corner},
			{"top_right_corner",	XC_top_right_corner},
			{"top_side",		XC_top_side},
			{"top_tee",		XC_top_tee},
			{"trek",		XC_trek},
			{"ul_angle",		XC_ul_angle},
			{"umbrella",		XC_umbrella},
			{"ur_angle",		XC_ur_angle},
			{"watch",		XC_watch},
			{"xterm",		XC_xterm},
    };
#ifdef DEC_EXTENSION
#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
    static Const struct _DECWCursorName {
        Const char      *name;
        unsigned short  decw_shape;
        unsigned short  shape;
    } decw_cursor_names[] = {
     {"decw_select_cursor",       decw$c_select_cursor,   XC_top_left_arrow},
     {"decw_leftselect_cursor",   decw$c_leftselect_cursor, XC_top_left_arrow},
     {"decw_help_select_cursor",  decw$c_help_select_cursor, XC_hand2},
     {"decw_wait_cursor",         decw$c_wait_cursor,     XC_watch},
     {"decw_inactive_cursor",     decw$c_inactive_cursor, XC_circle},
     {"decw_resize_cursor",       decw$c_resize_cursor,   XC_sizing},
     {"decw_vpane_cursor",        decw$c_vpane_cursor,    XC_sb_v_double_arrow},
     {"decw_hpane_cursor",        decw$c_hpane_cursor,    XC_sb_h_double_arrow},
     {"decw_text_insertion_cursor", decw$c_text_insertion_cursor, XC_xterm},
     {"decw_text_insertion_bl_cursor", decw$c_text_insertion_bl_cursor, XC_xterm},
     {"decw_cross_hair_cursor",   decw$c_cross_hair_cursor, XC_crosshair},
     {"decw_draw_cursor",         decw$c_draw_cursor,     XC_diamond_cross},
     {"decw_pencil_cursor",       decw$c_pencil_cursor,   XC_pencil},
     {"decw_rpencil_cursor",      decw$c_rpencil_cursor,  XC_pencil},
     {"decw_center_cursor",       decw$c_center_cursor,   XC_center_ptr},
     {"decw_rightselect_cursor",  decw$c_rightselect_cursor, XC_arrow},
     {"decw_wselect_cursor",      decw$c_wselect_cursor,  XC_sb_left_arrow},
     {"decw_eselect_cursor",      decw$c_eselect_cursor,  XC_sb_right_arrow},
     {"decw_x_cursor",            decw$c_x_cursor,        XC_X_cursor},
     {"decw_circle_cursor",       decw$c_circle_cursor,   XC_circle},
     {"decw_mouse_cursor",        decw$c_mouse_cursor,    XC_mouse},
     {"decw_lpencil_cursor",      decw$c_lpencil_cursor,  XC_pencil},
     {"decw_leftgrab_cursor",     decw$c_leftgrab_cursor, XC_top_left_arrow},
     {"decw_grabhand_cursor",     decw$c_grabhand_cursor, XC_center_ptr},
     {"decw_rightgrab_cursor",    decw$c_rightgrab_cursor, XC_arrow},
     {"decw_leftpointing_cursor", decw$c_leftpointing_cursor, XC_sb_left_arrow},
     {"decw_uppointing_cursor",   decw$c_uppointing_cursor, XC_sb_up_arrow},
     {"decw_rightpointing_cursor", decw$c_rightpointing_cursor, XC_sb_right_arrow},
     {"decw_check_cursor",        decw$c_check_cursor,    XC_plus},
#ifdef decw$c_questionmark_cursor
     {"decw_questionmark_cursor", decw$c_questionmark_cursor, XC_question_arrow},
#else
     {"decw_questionmark_cursor", 54, XC_question_arrow},
#endif
    };
#endif

    Const struct _CursorName *nP;
    char *name = (char *)fromVal->addr;
    register int i;

    if (*num_args != 1) {
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	     XtNwrongParameters,"cvtStringToCursor",XtCXtToolkitError,
             "String to cursor conversion needs display argument",
              (String *)NULL, (Cardinal *)NULL);
	return False;
    }

#ifdef DEC_EXTENSION
    /*
     *  If the string begins with decw_, look it up in the DECW$CURSOR
     *  table.
     */

    if (strncmp(name, "decw_", 5) == 0)
    {
      Const struct _DECWCursorName *dnP;
      for (i=0, dnP=decw_cursor_names; i < XtNumber(decw_cursor_names); i++, dnP++ ) {
        if (strcmp(name, dnP->name) == 0) {
            Screen      *screen;
            Cursor      cursor = NULL;
            XFontStruct *fontinfo = NULL;
            XColor      cursor_colors[2];
            Display *display = *(Display**)args[0].addr;

            /*
             *  See if we can use the DECW$CURSOR font
             */
            if (display == decw_saved_display)
                fontinfo = decw_saved_font;
            else
            {
                if (decw_saved_font)
                    XFreeFont(decw_saved_display, decw_saved_font);
                decw_saved_display = NULL;
                decw_saved_font = NULL;
                fontinfo = XLoadQueryFont(display, "DECw$Cursor");
                if (fontinfo != (XFontStruct *)NULL)
                {
                    decw_saved_display = display;
                    decw_saved_font = fontinfo;
                }
            }

            if (fontinfo != (XFontStruct *)NULL)
            {
                /* ensure that this cursor exists in the font */

                if (fontinfo->max_char_or_byte2 >= dnP->decw_shape)
                {
                    screen = DefaultScreenOfDisplay(display);

                    cursor_colors[0].pixel = WhitePixelOfScreen(screen);
                    cursor_colors[1].pixel = BlackPixelOfScreen(screen);

                    XQueryColors(display,
                                DefaultColormapOfScreen(screen),
                                cursor_colors,2);

                    cursor = XCreateGlyphCursor(display,
                                                 fontinfo->fid,
                                                 fontinfo->fid,
                                                 dnP->decw_shape,
                                                 dnP->decw_shape + 1,
                                                 &cursor_colors[0],
                                                 &cursor_colors[1]);
               }
            }

            if (cursor == NULL)
                /*
                 *  Use the X cursor fallback
                 */
                cursor = XCreateFontCursor(display, dnP->shape );

            if (cursor != NULL)
                done(Cursor, cursor);
        }
      }
    }
    else
#endif

    for (i=0, nP=cursor_names; i < XtNumber(cursor_names); i++, nP++ ) {
	if (strcmp(name, nP->name) == 0) {
	    Display *display = *(Display**)args[0].addr;
	    Cursor cursor = XCreateFontCursor(display, nP->shape );
	    done(Cursor, cursor);
	}
    }
    XtDisplayStringConversionWarning(dpy, name, XtRCursor);
    return False;
}

/* ARGSUSED */
static void FreeCursor(app, toVal, closure, args, num_args)
    XtAppContext app;
    XrmValuePtr	toVal;
    XtPointer	closure;	/* unused */
    XrmValuePtr	args;
    Cardinal	*num_args;
{
    Display*	display;

    if (*num_args != 1) {
     XtAppWarningMsg(app,
	     XtNwrongParameters,"freeCursor",XtCXtToolkitError,
             "Free Cursor requires display argument",
              (String *)NULL, (Cardinal *)NULL);
     return;
    }

    display = *(Display**)args[0].addr;
    XFreeCursor( display, *(Cursor*)toVal->addr );
#ifdef DEC_EXTENSION
    if (decw_saved_font)
        XFreeFont(decw_saved_display, decw_saved_font);
    decw_saved_display = NULL;
    decw_saved_font = NULL;
#endif

}

/*ARGSUSED*/
Boolean XtCvtStringToDisplay(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    Display	*d;

    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToDisplay",XtCXtToolkitError,
                  "String to Display conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);

    d = XOpenDisplay((char *)fromVal->addr);
    if (d != NULL)
	done(Display*, d);

    XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRDisplay);
    return False;
}


/*ARGSUSED*/
Boolean XtCvtStringToFile(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    FILE *f;

    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		 XtNwrongParameters,"cvtStringToFile",XtCXtToolkitError,
                 "String to File conversion needs no extra arguments",
                 (String *) NULL, (Cardinal *)NULL);

    f = fopen((char *)fromVal->addr, "r");
    if (f != NULL)
	done(FILE*, f);

    XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRFile);
    return False;
}

/* ARGSUSED */
static void FreeFile(app, toVal, closure, args, num_args)
    XtAppContext app;
    XrmValuePtr	toVal;
    XtPointer	closure;	/* unused */
    XrmValuePtr	args;		/* unused */
    Cardinal	*num_args;
{
    if (*num_args != 0)
	XtAppWarningMsg(app,
		 XtNwrongParameters,"freeFile",XtCXtToolkitError,
                 "Free File requires no extra arguments",
                 (String *) NULL, (Cardinal *)NULL);

    fclose( *(FILE**)toVal->addr );
}

/*ARGSUSED*/
Boolean XtCvtIntToFloat(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtIntToFloat",XtCXtToolkitError,
                  "Integer to Float conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    done(float, (*(int *)fromVal->addr));
}

/*ARGSUSED*/
Boolean XtCvtStringToFloat(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    float f;
    extern double atof();

    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		 XtNwrongParameters,"cvtStringToFloat",XtCXtToolkitError,
                 "String to Float conversion needs no extra arguments",
                 (String *) NULL, (Cardinal *)NULL);

    f = atof(fromVal->addr);
    done(float, f);
}

/*ARGSUSED*/
Boolean XtCvtStringToFont(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    Font	f;
    Display*	display;
#ifdef DEC_EXTENSION
    char        *fallfont;
    char        *fallname;
    XFontStruct *fontstruct;
#endif

    if (*num_args != 1) {
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	     XtNwrongParameters,"cvtStringToFont",XtCXtToolkitError,
             "String to font conversion needs display argument",
              (String *) NULL, (Cardinal *)NULL);
	return False;
    }

    display = *(Display**)args[0].addr;
#ifdef DEC_EXTENSION
    fallfont = (char *)fromVal->addr;
#endif

    if (CompareISOLatin1((String)fromVal->addr, XtDefaultFont) != 0) {
#ifdef DEC_EXTENSION
        fontstruct = XLoadQueryFont(display, (char *)fromVal->addr);
        if (fontstruct)
        {
            XFreeFont(display, fontstruct);
#endif
	f = XLoadFont(display, (char *)fromVal->addr);
	if (f != 0) {
  Done:	    done( Font, f );
	}
	XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRFont);
#ifdef DEC_EXTENSION
          }
#endif
    }
    /* try and get the default font */

    {
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;

	xrm_name[0] = XrmPermStringToQuark ("xtDefaultFont");
	xrm_name[1] = 0;
	xrm_class[0] = XrmPermStringToQuark ("XtDefaultFont");
	xrm_class[1] = 0;
	if (XrmQGetResource(XtDatabase(display), xrm_name, xrm_class, 
			    &rep_type, &value)) {
	    if (rep_type == _XtQString) {
		f = XLoadFont(display, (char *)value.addr);
		if (f != 0)
		    goto Done;
		else
#ifdef DEC_EXTENSION
		{
#endif
		    XtDisplayStringConversionWarning(dpy, (char *)value.addr,
						     XtRFont);
#ifdef DEC_EXTENSION
                    fallfont = (char *) value.addr;
		}
#endif
	    } else if (rep_type == XtQFont) {
		f = *(Font*)value.addr;
		goto Done;
	    } else if (rep_type == XtQFontStruct) {
		f = ((XFontStruct*)value.addr)->fid;
		goto Done;
	    }
	}
    }
    /* Should really do XListFonts, but most servers support this */
#ifdef DEC_EXTENSION
    fallname = _Xt_FontFallback(fallfont);
    f = XLoadFont(display, fallname);    
#else
    f = XLoadFont(display, "-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1");
#endif
    if (f != 0)
	goto Done;

    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    "noFont","cvtStringToFont",XtCXtToolkitError,
		    "Unable to load any useable ISO8859-1 font",
		    (String *) NULL, (Cardinal *)NULL);

    return False;
}

/* ARGSUSED */
static void FreeFont(app, toVal, closure, args, num_args)
    XtAppContext app;
    XrmValuePtr	toVal;
    XtPointer	closure;	/* unused */
    XrmValuePtr	args;
    Cardinal	*num_args;
{
    Display *display;
    if (*num_args != 1) {
	XtAppWarningMsg(app,
	     XtNwrongParameters,"freeFont",XtCXtToolkitError,
             "Free Font needs display argument",
              (String *) NULL, (Cardinal *)NULL);
	return;
    }

    display = *(Display**)args[0].addr;
    XUnloadFont( display, *(Font*)toVal->addr );
}

/*ARGSUSED*/
Boolean XtCvtIntToFont(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	   XtNwrongParameters,"cvtIntToFont",XtCXtToolkitError,
           "Integer to Font conversion needs no extra arguments",
            (String *) NULL, (Cardinal *)NULL);
    done(Font, *(int*)fromVal->addr);
}

/*ARGSUSED*/
Boolean XtCvtStringToFontSet(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*    dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer   *closure_ret;
{
    XFontSet  f;
    Display*  display;
    char**    missing_charset_list;
    int       missing_charset_count;
    char*     def_string;

    if (*num_args != 2) {
      XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
           XtNwrongParameters,"cvtStringToFontSet",XtCXtToolkitError,
             "String to FontSet conversion needs display and locale arguments",
              (String *) NULL, (Cardinal *)NULL);
      return False;
    }

    display = *(Display**)args[0].addr;

    if (CompareISOLatin1((String)fromVal->addr, XtDefaultFontSet) != 0) {
      f = XCreateFontSet(display, (char *)fromVal->addr,
              &missing_charset_list, &missing_charset_count, &def_string);
        /* Free any returned missing charset list */
      if (missing_charset_count) {
          XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
                 XtNmissingCharsetList,"cvtStringToFontSet",XtCXtToolkitError,
                 "Missing charsets in String to FontSet conversion",
                 (String *) NULL, (Cardinal *)NULL);
            XFreeStringList(missing_charset_list);
      }
      if (f != NULL) {
  Done:           done( XFontSet, f );
      }
      XtDisplayStringConversionWarning(dpy, (char *)fromVal->addr, XtRFontSet);
    }
    /* try and get the default fontset */

    {
      XrmName xrm_name[2];
      XrmClass xrm_class[2];
      XrmRepresentation rep_type;
      XrmValue value;

      xrm_name[0] = XrmPermStringToQuark ("xtDefaultFontSet");
      xrm_name[1] = 0;
      xrm_class[0] = XrmPermStringToQuark ("XtDefaultFontSet");
      xrm_class[1] = 0;
      if (XrmQGetResource(XtDatabase(display), xrm_name, xrm_class, 
                          &rep_type, &value)) {
          if (rep_type == _XtQString) {
              f = XCreateFontSet(display, (char *)value.addr,
				 &missing_charset_list, &missing_charset_count,
				 &def_string);
                /* Free any returned missing charset list */
              if (missing_charset_count) {
                  XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
			 XtNmissingCharsetList,"cvtStringToFontSet",
			 XtCXtToolkitError,
			 "Missing charsets in String to FontSet conversion",
                         (String *) NULL, (Cardinal *)NULL);
		  XFreeStringList(missing_charset_list);
              }
              if (f != NULL)
                  goto Done;
              else
                  XtDisplayStringConversionWarning(dpy, (char *)value.addr,
                                                   XtRFontSet);
          } else if (rep_type == XtQFontSet) {
              f = *(XFontSet*)value.addr;
              goto Done;
          }
      }
  }

    /* Should really do XListFonts, but most servers support this */
    f = XCreateFontSet(display, "-*-*-*-R-*-*-*-120-*-*-*-*",
          &missing_charset_list, &missing_charset_count, &def_string);
    /* Free any returned missing charset list */
    if (missing_charset_count) {
      XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
             XtNmissingCharsetList,"cvtStringToFontSet",XtCXtToolkitError,
             "Missing charsets in String to FontSet conversion",
             (String *) NULL, (Cardinal *)NULL);
        XFreeStringList(missing_charset_list);
    }
    if (f != NULL)
      goto Done;

    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
           "noFont","cvtStringToFontSet",XtCXtToolkitError,
             "Unable to load any useable fontset",
              (String *) NULL, (Cardinal *)NULL);
    
    return False;
}

/*ARGSUSED*/
static void FreeFontSet(app, toVal, closure, args, num_args)
    XtAppContext	app;
    XrmValuePtr		toVal;
    XtPointer		closure;        /* unused */
    XrmValuePtr		args;
    Cardinal		*num_args;
{
    Display *display;
    if (*num_args != 2) {
      XtAppWarningMsg(app,
           XtNwrongParameters,"freeFontSet",XtCXtToolkitError,
             "FreeFontSet needs display and locale arguments",
              (String *) NULL, (Cardinal *)NULL);
      return;
    }

    display = *(Display**)args[0].addr;
    XFreeFontSet( display, *(XFontSet*)toVal->addr );
}

/*ARGSUSED*/
static void FetchLocaleArg(widget, size, value )
    Widget widget;	/* unused */
    Cardinal *size;	/* unused */
    XrmValue *value;
{
    static XrmString locale;

    locale = XrmQuarkToString(XrmStringToQuark
			      (setlocale(LC_CTYPE, (char*)NULL)));
    value->size = sizeof(XrmString);
    value->addr = (XPointer)&locale;
}

static XtConvertArgRec Const localeDisplayConvertArgs[] = {
    {XtProcedureArg, (XtPointer)FetchDisplayArg, 0},
    {XtProcedureArg, (XtPointer)FetchLocaleArg, 0},
};


/*ARGSUSED*/
Boolean
XtCvtStringToFontStruct(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    XFontStruct	    *f;
    Display*	display;
#ifdef DEC_EXTENSION
    char        *fallfont;
#endif

    if (*num_args != 1) {
     XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	     XtNwrongParameters,"cvtStringToFontStruct",XtCXtToolkitError,
             "String to font conversion needs display argument",
              (String *) NULL, (Cardinal *)NULL);
     return False;
    }

    display = *(Display**)args[0].addr;
#ifdef DEC_EXTENSION
    fallfont = (char *) fromVal->addr;
#endif

    if (CompareISOLatin1((String)fromVal->addr, XtDefaultFont) != 0) {
	f = XLoadQueryFont(display, (char *)fromVal->addr);
	if (f != NULL) {
  Done:	    done( XFontStruct*, f);
	}

	XtDisplayStringConversionWarning(dpy, (char*)fromVal->addr,
					 XtRFontStruct);
    }

    /* try and get the default font */

    {
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;

	xrm_name[0] = XrmPermStringToQuark ("xtDefaultFont");
	xrm_name[1] = 0;
	xrm_class[0] = XrmPermStringToQuark ("XtDefaultFont");
	xrm_class[1] = 0;
	if (XrmQGetResource(XtDatabase(display), xrm_name, xrm_class, 
			    &rep_type, &value)) {
	    if (rep_type == _XtQString) {
		f = XLoadQueryFont(display, (char*)value.addr);
		if (f != NULL)
		    goto Done;
		else
#ifdef DEC_EXTENSION
		{
#endif
		    XtDisplayStringConversionWarning(dpy, (char*)value.addr,
						     XtRFontStruct);
#ifdef DEC_EXTENSION
                    fallfont = (char *) value.addr;
		}
#endif
	    } else if (rep_type == XtQFont) {
		f = XQueryFont(display, *(Font*)value.addr );
		if (f != NULL) goto Done;
	    } else if (rep_type == XtQFontStruct) {
		f = (XFontStruct*)value.addr;
		goto Done;
	    }
	}
    }
    /* Should really do XListFonts, but most servers support this */
#ifdef DEC_EXTENSION
    f = _Xt_LoadQueryFont(display, fallfont);    
#else
    f = XLoadQueryFont(display, "-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1");
#endif
    if (f != NULL)
	goto Done;

    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	     "noFont","cvtStringToFontStruct",XtCXtToolkitError,
             "Unable to load any useable ISO8859-1 font",
              (String *) NULL, (Cardinal *)NULL);
    
    return False;
}

/* ARGSUSED */
static void FreeFontStruct(app, toVal, closure, args, num_args)
    XtAppContext app;
    XrmValuePtr	toVal;
    XtPointer	closure;	/* unused */
    XrmValuePtr	args;
    Cardinal	*num_args;
{
    Display *display;
    if (*num_args != 1) {
     XtAppWarningMsg(app,
	     XtNwrongParameters,"freeFontStruct",XtCXtToolkitError,
             "Free FontStruct requires display argument",
              (String *) NULL, (Cardinal *)NULL);
     return;
    }

    display = *(Display**)args[0].addr;
    XFreeFont( display, *(XFontStruct**)toVal->addr );
}

/*ARGSUSED*/
Boolean XtCvtStringToInt(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    int	i;

    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToInt",XtCXtToolkitError,
                  "String to Integer conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);
    if (IsInteger((String)fromVal->addr, &i))
	done(int, i);

    XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRInt);
    return False;
}

/*ARGSUSED*/
Boolean XtCvtStringToShort(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer	*closure_ret;
{
    int i;

    if (*num_args != 0)
        XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	  XtNwrongParameters,"cvtStringToShort",XtCXtToolkitError,
          "String to Integer conversion needs no extra arguments",
           (String *) NULL, (Cardinal *)NULL);
    if (IsInteger((String)fromVal->addr, &i))
        done(short, (short)i);

    XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRShort);
    return False;
}

/*ARGSUSED*/
Boolean XtCvtStringToDimension(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer	*closure_ret;
{
    int i;

    if (*num_args != 0)
        XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
	  XtNwrongParameters,"cvtStringToDimension",XtCXtToolkitError,
          "String to Dimension conversion needs no extra arguments",
           (String *) NULL, (Cardinal *)NULL);
    if (IsInteger((String)fromVal->addr, &i)) {
        if ( i < 0 )
            XtDisplayStringConversionWarning(dpy, (char*)fromVal->addr,
					     XtRDimension);
        done(Dimension, (Dimension)i);
    }
    XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRDimension);
    return False;
}

/*ARGSUSED*/
Boolean XtCvtIntToUnsignedChar(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtIntToUnsignedChar",XtCXtToolkitError,
                  "Integer to UnsignedChar conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    done(unsigned char, (*(int *)fromVal->addr));
}


/*ARGSUSED*/
Boolean XtCvtStringToUnsignedChar(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer	*closure_ret;
{
    int i;

    if (*num_args != 0)
        XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToUnsignedChar",XtCXtToolkitError,
                  "String to Integer conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    if (IsInteger((String)fromVal->addr, &i)) {
        if ( i < 0 || i > 255 )
            XtDisplayStringConversionWarning(dpy, (char*)fromVal->addr,
					     XtRUnsignedChar);
        done(unsigned char, i);
    }
    XtDisplayStringConversionWarning(dpy, (char*)fromVal->addr,
				     XtRUnsignedChar);
    return False;
}


/*ARGSUSED*/
Boolean XtCvtColorToPixel(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtXColorToPixel",XtCXtToolkitError,
                  "Color to Pixel conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    done(Pixel, ((XColor *)fromVal->addr)->pixel);
}

/*ARGSUSED*/
Boolean XtCvtIntToPixel(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtIntToPixel",XtCXtToolkitError,
                  "Integer to Pixel conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    done(Pixel, *(int*)fromVal->addr);
}

/*ARGSUSED*/
Boolean XtCvtIntToPixmap(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
        XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtIntToPixmap",XtCXtToolkitError,
                  "Integer to Pixmap conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    done(Pixmap, *(Pixmap*)fromVal->addr);
}

#ifdef STRING_TO_GEOMETRY	/* not in the specification */
/*ARGSUSED*/
static Boolean
CvtStringToGeometry(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToGeometry",XtCXtToolkitError,
                  "String to Geometry conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    done(String, *(String*)fromVal->addr);
}
#endif

#ifdef MOTIFBC
void LowerCase(source, dest)
    register char  *source, *dest;
{
    register char ch;
    int i;

    for (i = 0; (ch = *source) != 0 && i < 999; source++, dest++, i++) {
    	if ('A' <= ch && ch <= 'Z')
	    *dest = ch - 'A' + 'a';
	else
	    *dest = ch;
    }
    *dest = 0;
}
#endif

static int CompareISOLatin1 (first, second)
    char *first, *second;
{
    register unsigned char *ap, *bp;

    for (ap = (unsigned char *) first, bp = (unsigned char *) second;
	 *ap && *bp; ap++, bp++) {
	register unsigned char a, b;

	if ((a = *ap) != (b = *bp)) {
	    /* try lowercasing and try again */

	    if ((a >= XK_A) && (a <= XK_Z))
	      a += (XK_a - XK_A);
	    else if ((a >= XK_Agrave) && (a <= XK_Odiaeresis))
	      a += (XK_agrave - XK_Agrave);
	    else if ((a >= XK_Ooblique) && (a <= XK_Thorn))
	      a += (XK_oslash - XK_Ooblique);

	    if ((b >= XK_A) && (b <= XK_Z))
	      b += (XK_a - XK_A);
	    else if ((b >= XK_Agrave) && (b <= XK_Odiaeresis))
	      b += (XK_agrave - XK_Agrave);
	    else if ((b >= XK_Ooblique) && (b <= XK_Thorn))
	      b += (XK_oslash - XK_Ooblique);

	    if (a != b) break;
	}
    }
    return (((int) *bp) - ((int) *ap));
}


/*ARGSUSED*/
Boolean 
XtCvtStringToInitialState(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    String str = (String)fromVal->addr;
    if (*num_args != 0)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToInitialState",XtCXtToolkitError,
                  "String to InitialState conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);

    if (CompareISOLatin1(str, "NormalState") == 0) done(int, NormalState);
    if (CompareISOLatin1(str, "IconicState") == 0) done(int, IconicState);
    {
	int val;
	if (IsInteger(str, &val)) done( int, val );
    }
    XtDisplayStringConversionWarning(dpy, str, XtRInitialState);
    return False;
}

static XtConvertArgRec Const visualConvertArgs[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.screen),
     sizeof(Screen *)},
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.depth),
     sizeof(Cardinal)}
};

/*ARGSUSED*/
Boolean XtCvtStringToVisual(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;		/* Screen, depth */
    Cardinal    *num_args;	/* 2 */
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;	/* unused */
{
    String str = (String)fromVal->addr;
    int vc;
    XVisualInfo vinfo;
    if (*num_args != 2) {
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToVisual",XtCXtToolkitError,
                  "String to Visual conversion needs screen and depth arguments",
                   (String *) NULL, (Cardinal *)NULL);
	return False;
    }

         if (CompareISOLatin1(str, "StaticGray") == 0)	vc = StaticGray;
    else if (CompareISOLatin1(str, "StaticColor") == 0)	vc = StaticColor;
    else if (CompareISOLatin1(str, "TrueColor") == 0)	vc = TrueColor;
    else if (CompareISOLatin1(str, "GrayScale") == 0)	vc = GrayScale;
    else if (CompareISOLatin1(str, "PseudoColor") == 0)	vc = PseudoColor;
    else if (CompareISOLatin1(str, "DirectColor") == 0)	vc = DirectColor;
    else if (!IsInteger(str, &vc)) {
	XtDisplayStringConversionWarning(dpy, str, "Visual class name");
	return False;
    }

    if (XMatchVisualInfo( XDisplayOfScreen((Screen*)*(Screen**)args[0].addr),
		     XScreenNumberOfScreen((Screen*)*(Screen**)args[0].addr),
		     (int)*(int*)args[1].addr,
		     vc,
		     &vinfo) ) {
	done( Visual*, vinfo.visual );
    }
    else {
	String params[2];
	Cardinal num_params = 2;
	params[0] = str;
	params[1] = 
	    DisplayString(XDisplayOfScreen((Screen*)*(Screen**)args[0].addr));
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNconversionError, "stringToVisual", XtCXtToolkitError,
                  "Cannot find Visual of class %s for display %s",
		  params, &num_params );
	return False;
    }
}


/*ARGSUSED*/
Boolean XtCvtStringToAtom(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    Atom atom;
    if (*num_args != 1) {
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		  XtNwrongParameters,"cvtStringToAtom",XtCXtToolkitError,
                  "String to Atom conversion needs Display argument",
                   (String *) NULL, (Cardinal *)NULL);
	return False;
    }
    
    atom =  XInternAtom( *(Display**)args->addr, (char*)fromVal->addr, False );
    done(Atom, atom);
}


_XtAddDefaultConverters(table)
    ConverterTable table;
{
#define Add(from, to, proc, convert_args, num_args, cache) \
    _XtTableAddConverter(table, from, to, proc, \
	    (XtConvertArgList) convert_args, (Cardinal)num_args, \
	    True, cache, (XtDestructor)NULL)

#define Add2(from, to, proc, convert_args, num_args, cache, destructor) \
    _XtTableAddConverter(table, from, to, proc, \
	    (XtConvertArgList) convert_args, (Cardinal)num_args, \
	    True, cache, destructor)

    Add(XtQColor, XtQPixel,       XtCvtColorToPixel,   NULL, 0, XtCacheNone);

    Add(XtQInt,   XtQBool,	  XtCvtIntToBool,      NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQBoolean,     XtCvtIntToBoolean,   NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQColor,	  XtCvtIntToColor,
	colorConvertArgs, XtNumber(colorConvertArgs), XtCacheByDisplay);
    Add(XtQInt,   XtQDimension,   XtCvtIntToShort,     NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQFloat,       XtCvtIntToFloat,     NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQFont,        XtCvtIntToFont,      NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQPixel,       XtCvtIntToPixel,     NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQPixmap,      XtCvtIntToPixmap,    NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQPosition,    XtCvtIntToShort,     NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQShort,       XtCvtIntToShort,     NULL, 0, XtCacheNone);
    Add(XtQInt,   XtQUnsignedChar,XtCvtIntToUnsignedChar,NULL, 0, XtCacheNone);

    Add(XtQPixel, XtQColor,	  XtCvtIntToColor,
	colorConvertArgs, XtNumber(colorConvertArgs), XtCacheByDisplay);

    Add(_XtQString, XtQAtom,      XtCvtStringToAtom,
	displayConvertArg, XtNumber(displayConvertArg), XtCacheNone);
    Add(_XtQString, XtQBool,      XtCvtStringToBool,    NULL, 0, XtCacheNone);
    Add(_XtQString, XtQBoolean,   XtCvtStringToBoolean, NULL, 0, XtCacheNone);
   Add2(_XtQString, XtQCursor,    XtCvtStringToCursor,
	displayConvertArg, XtNumber(displayConvertArg),
	XtCacheByDisplay, FreeCursor);
    Add(_XtQString, XtQDimension, XtCvtStringToDimension,NULL, 0, XtCacheNone);
    Add(_XtQString, XtQDisplay,   XtCvtStringToDisplay, NULL, 0, XtCacheAll);
   Add2(_XtQString, XtQFile,      XtCvtStringToFile,    NULL, 0,
	XtCacheAll | XtCacheRefCount, FreeFile);
    Add(_XtQString, XtQFloat,     XtCvtStringToFloat,   NULL, 0, XtCacheNone);

   Add2(_XtQString, XtQFont,      XtCvtStringToFont,
	displayConvertArg, XtNumber(displayConvertArg),
	XtCacheByDisplay, FreeFont);
   Add2(_XtQString, XtQFontSet,   XtCvtStringToFontSet,
	localeDisplayConvertArgs, XtNumber(localeDisplayConvertArgs),
	XtCacheByDisplay, FreeFontSet);
   Add2(_XtQString, XtQFontStruct,XtCvtStringToFontStruct,
	displayConvertArg, XtNumber(displayConvertArg),
	XtCacheByDisplay, FreeFontStruct);

#ifdef STRING_TO_GEOMETRY /* Not in the specification */
    Add(_XtQString, XtQGeometry,  CvtStringToGeometry, NULL, 0, XtCacheNone);
#endif

    Add(_XtQString, XtQInitialState, XtCvtStringToInitialState, NULL, 0,
	XtCacheNone);
    Add(_XtQString, XtQInt,	     XtCvtStringToInt,    NULL, 0, XtCacheAll);
   Add2(_XtQString, XtQPixel,        XtCvtStringToPixel,
	colorConvertArgs, XtNumber(colorConvertArgs),
	XtCacheByDisplay, FreePixel);
    Add(_XtQString, XtQPosition,     XtCvtStringToShort,  NULL, 0, XtCacheAll);
    Add(_XtQString, XtQShort,        XtCvtStringToShort,  NULL, 0, XtCacheAll);
    Add(_XtQString, XtQUnsignedChar, XtCvtStringToUnsignedChar,
	NULL, 0, XtCacheAll);
   Add2(_XtQString, XtQVisual,       XtCvtStringToVisual,
	visualConvertArgs, XtNumber(visualConvertArgs),
	XtCacheByDisplay, NULL);

   _XtAddTMConverters(table);
}
