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
static char *rcsid = "@(#)$RCSfile: XvendorWrap.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/09 15:59:56 $";
#endif
/*
 * $Header: /usr/sde/osf1/rcs/x11/src/lib/X/XvendorWrap.c,v 1.1.4.2 1993/04/09 15:59:56 Pete_Nishimoto Exp $
 */

/*
*****************************************************************************

              Copyright (c) Digital Equipment Corporation, 1992
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
**      Xlib
**
**  ABSTRACT:
**
**      Vendor Pluggable Layer
**
**  MODIFICATION HISTORY:
**
**
**      26-Jun-1992 Begin work on R5 I18n Pluggalbe Layer
**       3-Jul-1992 Version 0.5
**       4-Aug-1992 some modification for #include/#define
**      26-Aug-1992 Use System V style shared library
**      28-Sep-1992 some bug-fixes and improvements on VMS portion
**       5-Nov-1992 improve the way of finding the shared library
**
**  COMPILE OPTIONS:
**
**      If this file is compiled without VMS or __osf__ defined, it means
**      nothing.
**
**--
**/

/*
 * Includes
 */
#include <stdio.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include "Xlcint.h"

#ifdef VMS
#include <descrip.h>
#include <ssdef.h>
#include <lnmdef.h>
#include <fscndef.h>
#endif /* VMS */

#ifdef __osf__
#include <dlfcn.h>
#endif /* __osf__ */

/*
 * Defines
 */
#ifdef VMS
#define XDEFAULTSTRING                  "_XDefaultString"
#define XWCFREESTRINGLIST               "_XwcFreeStringList"
#define XWCTEXTLISTTOTEXTPROPERTY       "_XwcTextListToTextProperty"
#define XMBTEXTLISTTOTEXTPROPERTY       "_XmbTextListToTextProperty"
#define XWCTEXTPROPERTYTOTEXTLIST       "_XwcTextPropertyToTextList"
#define XMBTEXTPROPERTYTOTEXTLIST       "_XmbTextPropertyToTextList"
#define _XRMINITPARSEINFO               "__XrmInitParseInfo"
#define XLCDEFAULTLOADER                "__XlcDefaultLoader"
#else
#define XDEFAULTSTRING                  "XDefaultString"
#define XWCFREESTRINGLIST               "XwcFreeStringList"
#define XWCTEXTLISTTOTEXTPROPERTY       "XwcTextListToTextProperty"
#define XMBTEXTLISTTOTEXTPROPERTY       "XmbTextListToTextProperty"
#define XWCTEXTPROPERTYTOTEXTLIST       "XwcTextPropertyToTextList"
#define XMBTEXTPROPERTYTOTEXTLIST       "XmbTextPropertyToTextList"
#define _XRMINITPARSEINFO               "_XrmInitParseInfo"
#define XLCDEFAULTLOADER                "_XlcDefaultLoader"
#endif

#ifdef VMS
#define VENDORLAYER "DECW$XVENDORLAYER"
#define VENDOR1STDEFAULT "DECW$XI18NLIBSHR"
#define VENDOR2NDDEFAULT "DECW$XSILIBSHR"
#define VENDOR3RDDEFAULT "DECW$XIMPLIBSHR"
#endif /* VMS */

#ifdef __osf__
#define VENDORLAYER "XVENDORLAYER"
#define VENDOR1STDEFAULT "libXi18n.so"
#define VENDOR2NDDEFAULT "libXsi.so"
#define VENDOR3RDDEFAULT "libXimp.so"
#endif /* __osf__ */

typedef void (*I18nProc)(
#if NeedFunctionPrototypes
			 void
#endif
			 );

#ifdef WRAPPED_I18N
extern _XDefaultString();
extern _XwcFreeStringList();
extern _XwcTextListToTextProperty();
extern _XmbTextListToTextProperty();
extern _XwcTextPropertyToTextList();
extern _XmbTextPropertyToTextList();
extern __XrmInitParseInfo();
extern __XlcDefaultLoader();
#endif


I18nProc
#if NeedFunctionPrototypes
_load_symbol(char *symbol_name)
#else
_load_symbol(symbol_name)
  char *symbol_name;
#endif
{
#ifdef VMS
    long cond_value;    
    struct dsc$descriptor_s func_symbol, file_name, image_name;
    static char *file_name_string = NULL;
#endif /* VMS */
#ifdef __osf__
    static void *lib_handle = NULL;
    static int tried = False;
    char * layer;
#endif /* __osf__ */
    I18nProc func_addr;

#ifdef VMS
    if (!file_name_string) 
	file_name_string = _get_shared_library_name();

    if (!file_name_string) return(NULL);

    file_name.dsc$w_length  = strlen(file_name_string);
    file_name.dsc$b_dtype   = DSC$K_DTYPE_T;
    file_name.dsc$b_class   = DSC$K_CLASS_S;
    file_name.dsc$a_pointer = file_name_string;

    func_symbol.dsc$w_length  = strlen(symbol_name);
    func_symbol.dsc$b_dtype   = DSC$K_DTYPE_T;
    func_symbol.dsc$b_class   = DSC$K_CLASS_S;
    func_symbol.dsc$a_pointer = symbol_name;

    cond_value = LIB$FIND_IMAGE_SYMBOL(&file_name,
				       &func_symbol,
				       &func_addr,
				       NULL);
    if(cond_value != SS$_NORMAL)
	func_addr = NULL;
#endif /* VMS */

#ifdef ultrix
    return(NULL);
#endif

#ifdef __osf__
#ifdef STATIC_OBJ
    return(NULL);
#else
    if(!tried) {
	tried = True;
	/* try to open the library if needed */
	layer = (char *)getenv(VENDORLAYER);

	if((layer) &&
	   ((lib_handle = dlopen(layer, RTLD_LAZY)) == NULL)) {
	    fprintf(stderr,
	    "Xlib: warning: I18N library \"%s\" specified by %s not found\n",
		layer,VENDORLAYER);
	}
#ifdef DUBUG_MESSAGES
	  else {
	  printf("VENDORLAYER found !\n");
	  }
#endif

	/* nothing specified, lets try some defaults */
	if(!lib_handle) {
	    if ((lib_handle=dlopen(VENDOR1STDEFAULT, RTLD_LAZY)) == NULL) {
#ifndef WRAPPED_I18N
		if ((lib_handle=dlopen(VENDOR2NDDEFAULT, RTLD_LAZY)) == NULL) {
		    lib_handle=dlopen(VENDOR3RDDEFAULT, RTLD_LAZY);
		}
#endif
	    }
#ifdef DUBUG_MESSAGES
	      else {
		  printf("using VENDOR1STDEFAULT\n");
	  }
#endif
	}
	
#ifdef WRAPPED_I18N
#ifdef DUBUG_MESSAGES
	if(!lib_handle) {
	    printf("defaulting to builtin\n");
	}
#endif
#else
	if(!lib_handle) {
	    fprintf(stderr,"Xlib: warning: could not load any internationalization module\n");
	    return(NULL);
	}
#endif
    }

    if(!lib_handle)
	return(NULL);
    
    func_addr = (I18nProc)dlsym(lib_handle, symbol_name);

#endif
#endif /* __osf__ */

    if(!func_addr)
	printf("Xlib: error loading internationalization routine %s\n",
	symbol_name);

    return(func_addr);
}

#ifdef VMS
static int
#if NeedFunctionPrototypes
exception_handler(long *sigargs, long *mechargs)
#else
exception_handler(sigargs, mechargs)
  long *sigargs;
  long *mechargs;
#endif
{
    return(SS$_CONTINUE);
}		  
#endif	/* VMS */


#ifdef VMS
#if NeedFunctionPrototypes
static Bool if_exist(char* file_name_string)
#else
static Bool if_exist(file_name_string)
  char* file_name_string;
#endif
{
    long cond_value;
    struct dsc$descriptor_s file_name, func_symbol;
    I18nProc func_addr;

    if (file_name_string == NULL)
	return False;

    file_name.dsc$w_length  = strlen(file_name_string);
    file_name.dsc$b_dtype   = DSC$K_DTYPE_T;
    file_name.dsc$b_class   = DSC$K_CLASS_S;
    file_name.dsc$a_pointer = file_name_string;
    /*
     * "XDefaultString" is a representative of functions
     * in each vendor's shared library
     */
    func_symbol.dsc$w_length  = strlen("_XDefaultString");
    func_symbol.dsc$b_dtype   = DSC$K_DTYPE_T;
    func_symbol.dsc$b_class   = DSC$K_CLASS_S;
    func_symbol.dsc$a_pointer = "_XDefaultString";

    cond_value = LIB$FIND_IMAGE_SYMBOL(&file_name,
				       &func_symbol,
				       &func_addr,
				       NULL);
    if(cond_value != SS$_NORMAL)
	return False;

    return True;
}
#endif /* VMS */


#ifdef VMS
char *
#if NeedFunctionPrototypes
_get_shared_library_name(void)
#else
_get_shared_library_name()
#endif
{
    static char *file_spec = NULL;
    char tmp[256], *cur_file = NULL;

    if(!file_spec) {
	cur_file = VENDORLAYER;
	LIB$ESTABLISH(exception_handler); /* this condition handler remains 
					   * in effect only until control 
					   * returns to the caller of this 
					   * routine
					   */

	/*
	 * Check if the file exists.  If it doesn't.
	 * Default is used instead.
	 */
	if (cur_file == NULL || !if_exist(cur_file)) {
	    cur_file = VENDOR1STDEFAULT;
	    if (!if_exist(cur_file)) {
		cur_file = VENDOR2NDDEFAULT;
		if (!if_exist(cur_file)) {
		    cur_file = VENDOR3RDDEFAULT;
		    if (!if_exist(cur_file)) {
			cur_file = NULL;
		    }
		}
	    }
	}

	if (cur_file) {
	    register int len = strlen(cur_file) + 1;

	    file_spec = Xmalloc(len);
	    bcopy(cur_file, file_spec, len);
#if 1
	    fprintf(stderr, "Xlib: Load vendor layer: %s.\n", file_spec);
#endif
	} else {
	    fprintf(stderr, "Xlib: No vendor layer found.\n");
	}
    }
	
    return(file_spec);
}
#endif /* VMS */
    

XLCd
#if NeedFunctionPrototypes
_XlcDefaultLoader(char *osname)
#else
_XlcDefaultLoader(osname)
  char *osname;
#endif
{
    typedef XLCd (*DefLdrFunc)(
#if NeedFunctionPrototypes
			       char *
#endif
			       );

    static DefLdrFunc func = (DefLdrFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (DefLdrFunc)_load_symbol(XLCDEFAULTLOADER);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (DefLdrFunc)__XlcDefaultLoader;
#endif
    }
    if (func != NULL) {
	return (*func)(osname);
    } else {
	return NULL;
    }
}


XrmMethods
#if NeedFunctionPrototypes
_XrmInitParseInfo(XPointer *state)
#else
_XrmInitParseInfo(state)
  XPointer *state;
#endif
{
    typedef XrmMethods (*InitParseInfoFunc)(
#if NeedFunctionPrototypes
					    XPointer *
#endif
					    );

    static InitParseInfoFunc func = (InitParseInfoFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (InitParseInfoFunc)_load_symbol(_XRMINITPARSEINFO);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (InitParseInfoFunc)__XrmInitParseInfo;
#endif
    }
    if (func != NULL) {
	return (*func)(state);
    } else {
	return NULL;
    }
}


int
#if NeedFunctionPrototypes
XmbTextPropertyToTextList(Display *dpy, 
			  XTextProperty *tp, 
			  char ***list_return, 
			  int *count_return)
#else
XmbTextPropertyToTextList(dpy, tp, list_return, count_return)
  Display *dpy;
  XTextProperty *tp;
  char ***list_return;
  int *count_return;
#endif
{
    typedef int (*TxtPropToTxtLstFunc)(
#if NeedFunctionPrototypes
				       Display *, 
				       XTextProperty *, 
				       char ***, 
				       int *
#endif
				       );

    static TxtPropToTxtLstFunc func = (TxtPropToTxtLstFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (TxtPropToTxtLstFunc)_load_symbol(XMBTEXTPROPERTYTOTEXTLIST);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (TxtPropToTxtLstFunc)_XmbTextPropertyToTextList;
#endif
    }
    if (func != NULL) {
	return (*func)(dpy, tp, list_return, count_return);
    } else {
	return 0;
    }
}


int
#if NeedFunctionPrototypes
XwcTextPropertyToTextList(Display *dpy, 
			  XTextProperty *tp, 
			  wchar_t ***list_return, 
			  int *count_return)
#else
XwcTextPropertyToTextList(dpy, tp, list_return, count_return)
  Display *dpy;
  XTextProperty *tp;
  wchar_t ***list_return;
  int *count_return;
#endif
{
    typedef int (*TxtPropToTxtLstFunc)(
#if NeedFunctionPrototypes
				       Display *, 
				       XTextProperty *, 
				       wchar_t ***, 
				       int *
#endif
				       );

    static TxtPropToTxtLstFunc func = (TxtPropToTxtLstFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (TxtPropToTxtLstFunc)_load_symbol(XWCTEXTPROPERTYTOTEXTLIST);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (TxtPropToTxtLstFunc)_XwcTextPropertyToTextList;
#endif
    }
    if (func != NULL) {
	return (*func)(dpy, tp, list_return, count_return);
    } else {
	return 0;
    }
}


int
#if NeedFunctionPrototypes
XmbTextListToTextProperty(Display *dpy, 
			  char **list, 
			  int count, 
			  XICCEncodingStyle style, 
			  XTextProperty *text_prop)
#else
XmbTextListToTextProperty(dpy, list, count, style, text_prop)
  Display *dpy;
  char **list;
  int count;
  XICCEncodingStyle style;
  XTextProperty *text_prop;
#endif
{
    typedef int (*TxtLstToTxtPropFunc)(
#if NeedFunctionPrototypes
				       Display *, 
				       char **, 
				       int , 
				       XICCEncodingStyle , 
				       XTextProperty *
#endif
				       );

    static TxtLstToTxtPropFunc func = (TxtLstToTxtPropFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (TxtLstToTxtPropFunc)_load_symbol(XMBTEXTLISTTOTEXTPROPERTY);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (TxtLstToTxtPropFunc)_XmbTextListToTextProperty;
#endif
    }
    if (func != NULL) {
	return (*func)(dpy, list, count, style, text_prop);
    } else {
	return 0;
    }

}


int
#if NeedFunctionPrototypes
XwcTextListToTextProperty(Display *dpy, 
			  wchar_t **list, 
			  int count, 
			  XICCEncodingStyle style, 
			  XTextProperty *text_prop)
#else
XwcTextListToTextProperty(dpy, list, count, style, text_prop)
  Display *dpy;
  wchar_t **list;
  int count;
  XICCEncodingStyle style;
  XTextProperty *text_prop;
#endif
{
    typedef int (*TxtLstToTxtPropFunc)(
#if NeedFunctionPrototypes 
				       Display *, 
				       wchar_t **, 
				       int , 
				       XICCEncodingStyle , 
				       XTextProperty *
#endif
				       );

    static TxtLstToTxtPropFunc func = (TxtLstToTxtPropFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (TxtLstToTxtPropFunc)_load_symbol(XWCTEXTLISTTOTEXTPROPERTY);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (TxtLstToTxtPropFunc)_XwcTextListToTextProperty;
#endif
    }
    if (func != NULL) {
	return (*func)(dpy, list, count, style, text_prop);
    } else {
	return 0;
    }
}


void
#if NeedFunctionPrototypes
XwcFreeStringList(wchar_t **list)
#else
XwcFreeStringList(list)
  wchar_t **list;
#endif
{
    typedef void (*FreStrLstFunc)(
#if NeedFunctionPrototypes
				  wchar_t **
#endif
				  );

    static FreStrLstFunc func = (FreStrLstFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (FreStrLstFunc)_load_symbol(XWCFREESTRINGLIST);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (FreStrLstFunc)_XwcFreeStringList;
#endif
    }
    if (func != NULL) {
	(*func)(list);
	return;
    } else {
	return;
    }
}


char *
#if NeedFunctionPrototypes
XDefaultString(void)
#else
XDefaultString()
#endif
{
    typedef char *(*DefStrFunc)(
#if NeedFunctionPrototypes
				void
#endif
				);

    static DefStrFunc func = (DefStrFunc)NULL;
    static int tried = False;

    if((!func) && (!tried)) {
	tried = True;
	func = (DefStrFunc)_load_symbol(XDEFAULTSTRING);
#ifdef WRAPPED_I18N
	if(!func)
	    func = (DefStrFunc)_XDefaultString;
#endif
    }
    if (func != NULL) {
	return (*func)();
    } else {
	return NULL;
    }
}

