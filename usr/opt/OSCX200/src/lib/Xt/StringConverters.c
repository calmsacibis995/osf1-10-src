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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: StringConverters.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/23 19:43:39 $";
#endif
#include <X11/Intrinsic.h>
#include "../Xt/IntrinsicI.h"
#include <X11/Shell.h>
#include <X11/StringDefs.h>

#include <X11/Xmu/Converters.h>

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <values.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

void XtDisplayConversionWarning();

static String XtNwrongParameters = "wrongParameters";
static String XtNconversionError = "conversionError";
static String XtNmissingCharsetList = "missingCharsetList";

typedef struct {
  int value;
  char * name;
  int size;
} ConversionMap;

static ConversionMap boolean_map[] = {
  { True, XtEtrue, sizeof(XtEtrue) },
  { False, XtEfalse, sizeof(XtEfalse) },
  { True, XtEon, sizeof(XtEon) },
  { False, XtEoff, sizeof(XtEoff) },
  { True, XtEyes, sizeof(XtEyes) },
  { False, XtEno, sizeof(XtEno) },
  };

static ConversionMap shape_style_map[] = {
  { XmuShapeRectangle, XtERectangle, sizeof(XtERectangle) },
  { XmuShapeRectangle, "ShapeRectangle", sizeof("ShapeRectangle") },
  { XmuShapeOval, XtEOval, sizeof(XtEOval) },
  { XmuShapeOval, "ShapeOval", sizeof("ShapeOval") },
  { XmuShapeEllipse, "ShapeEllipse", sizeof("ShapeEllipse") },
  { XmuShapeEllipse, XtEEllipse, sizeof(XtEEllipse) },
  { XmuShapeRoundedRectangle, "ShapeRoundedRectangle",
      sizeof("ShapeRoundedRectangle") },
  { XmuShapeRoundedRectangle,  XtERoundedRectangle,
      sizeof( XtERoundedRectangle) },
  };


#define	string_convert_done(value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < size) {		        \
		    toVal->size = size;			        \
		    return False;				\
		}						\
		strcpy((char *) toVal->addr, value, size);	\
	    }							\
	    else {						\
		toVal->addr = (XtPointer)value;			\
	    }							\
	    toVal->size = size; 				\
	    return True;					\
	}

#define map_convert_done(value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < value.size) {		        \
		    toVal->size = value.size;		        \
		    return False;				\
		}						\
		strcpy((char *) toVal->addr, value.name, value.size);	\
	    }							\
	    else {						\
		toVal->addr = value.name;       		\
	    }							\
	    toVal->size = value.size; 				\
	    return True;					\
	}



Boolean XtCvtShapeStyleToString ();
Boolean XtCvtBooleanToString ();
Boolean XtCvtBoolToString ();


void
XtDisplayConversionWarning(type, from, to)
String type;
int from;
String to;
{
  String params[2];
  Cardinal num_params = 2;

  params[0] = (String)from;
  params[1] = (String)to;
  XtWarningMsg("conversionError", type, XtCXtToolkitError,
	       "Cannot convert value %d to type %s",
	       params,&num_params);
}



Boolean XtCvtShapeStyleToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int * i = (int *) (fromVal->addr);
  int index;
  
  
  for (index = 0; XtNumber(shape_style_map); index++)
    if (*i == shape_style_map[index].value)
      map_convert_done(shape_style_map[index]);
  
  XtDisplayConversionWarning(XtRShapeStyle, *i, XtRString);
  return False;
}





Boolean XtCvtBooleanToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int * i = (int *) (fromVal->addr);
  int index;
  
  
  for (index = 0; XtNumber(boolean_map); index++)
    if (*i == boolean_map[index].value)
      map_convert_done(boolean_map[index]);
  
  XtDisplayConversionWarning(XtRBoolean, *i, XtRString);
  return False;
}


Boolean XtCvtBoolToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int * i = (int *) (fromVal->addr);
  int index;
  
  
  for (index = 0; XtNumber(boolean_map); index++)
    if (*i == boolean_map[index].value)
      map_convert_done(boolean_map[index]);
  
  XtDisplayConversionWarning(XtRBool, *i, XtRString);
  return False;
}


Boolean XtCvtPositionToString (dpy, args, numArgs, fromVal, toVal, data)
Display*	dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int size;
  static char buffer[32];
  
  if (*numArgs != 0)
    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    XtNwrongParameters,"cvtPositionToString",XtCXtToolkitError,
		    "Position to String conversion needs no extra arguments",
		    (String *)NULL, (Cardinal *)NULL);

  sprintf(buffer, "%d", * (Position *) fromVal->addr);
  size = strlen(buffer);
  string_convert_done(buffer);
}

Boolean XtCvtIntToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int size;
  static char buffer[32];
  
  if (*numArgs != 0)
    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    XtNwrongParameters,"cvtIntToString",XtCXtToolkitError,
		    "Int to String conversion needs no extra arguments",
		    (String *)NULL, (Cardinal *)NULL);

  sprintf(buffer, "%d", * (int *) fromVal->addr);
  size = strlen(buffer);
  string_convert_done(buffer);
}

Boolean XtCvtCardinalToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int size;
  static char buffer[32];
  
  if (*numArgs != 0)
    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    XtNwrongParameters,"cvtCardinalToString",XtCXtToolkitError,
		    "Cardinal to String conversion needs no extra arguments",
		    (String *)NULL, (Cardinal *)NULL);

  sprintf(buffer, "%d", * (Cardinal *) fromVal->addr);
  size = strlen(buffer);
  string_convert_done(buffer);
}


Boolean XtCvtPixelToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int size;
  static char buffer[32];
  
  if (*numArgs != 0)
    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    XtNwrongParameters,"cvtPixelToString",XtCXtToolkitError,
		    "Pixel to String conversion needs no extra arguments",
		    (String *)NULL, (Cardinal *)NULL);

  sprintf(buffer, "%d", * (Pixel *) fromVal->addr);
  size = strlen(buffer);
  string_convert_done(buffer);
}

Boolean XtCvtColorToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int size;
  static char buffer[32];
  
  if (*numArgs != 0)
    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    XtNwrongParameters,"cvtColorToString",XtCXtToolkitError,
		    "Color to String conversion needs no extra arguments",
		    (String *)NULL, (Cardinal *)NULL);

  sprintf(buffer, "rgb:%04hx/%04hx/%04hx", ((XColor *)fromVal->addr)->red, 
	    ((XColor *)fromVal->addr)->green, ((XColor *)fromVal->addr)->blue);
  size = strlen(buffer);
  string_convert_done(buffer);
}


Boolean XtCvtUnsignedCharToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int size;
  static char buffer[32];
  
  if (*numArgs != 0)
    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    XtNwrongParameters,"cvtUnsignedCharToString",
		    XtCXtToolkitError,
		  "UnsignedChar to String conversion needs no extra arguments",
		    (String *)NULL, (Cardinal *)NULL);

  sprintf(buffer, "%d", * (unsigned char *) fromVal->addr);
  size = strlen(buffer);
  string_convert_done(buffer);
}

Boolean XtCvtDimensionToString (dpy, args, numArgs, fromVal, toVal, data)
Display * dpy;
XrmValue * args;
Cardinal * numArgs;
XrmValue * fromVal;
XrmValue * toVal;
XtPointer * data;
{
  int size;
  static char buffer[32];
  
  if (*numArgs != 0)
    XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
		    XtNwrongParameters,"cvtDimensionToString",XtCXtToolkitError,
		    "Dimension to String conversion needs no extra arguments",
		    (String *)NULL, (Cardinal *)NULL);

  sprintf(buffer, "%d", * (Dimension *) fromVal->addr);
  size = strlen(buffer);
  string_convert_done(buffer);
}


void
_XtInitStringConverters()
{
  XtSetTypeConverter(XtRShapeStyle, XtRString, XtCvtShapeStyleToString,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRBoolean, XtRString, XtCvtBooleanToString,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRBool, XtRString, XtCvtBoolToString,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRShapeStyle, XtRString , XtCvtShapeStyleToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRBoolean, XtRString , XtCvtBooleanToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRBool, XtRString , XtCvtBoolToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRPosition, XtRString , XtCvtPositionToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRInt, XtRString , XtCvtIntToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRCardinal, XtRString , XtCvtCardinalToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRPixel, XtRString , XtCvtPixelToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRColor, XtRString , XtCvtColorToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRUnsignedChar, XtRString , XtCvtUnsignedCharToString ,
		     NULL, 0, XtCacheNone, NULL);
  XtSetTypeConverter(XtRDimension, XtRString , XtCvtDimensionToString ,
		     NULL, 0, XtCacheNone, NULL);
}

