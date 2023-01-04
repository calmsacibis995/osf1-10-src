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
#ifndef _WcCreateP_h
#define _WcCreateP_h
#include <X11/Wc/COPY>

/*
* SCCS_data: @(#) WcCreateP.h	1.7 92/06/10 06:11:14
*
* Widget Creation Library - WcCreateP.h
*
* Private defines for the Widget Creation Library.  Clients generally
* should not need to include this file.
*
* Anything and everything in here may change dramatically.
*
*******************************************************************************
*/

#include <ctype.h>		/* isupper, tolower, atoi macros */
#include <X11/Wc/WcCreate.h>
#include <X11/Wc/MapAg.h>

/*
*******************************************************************************
* Private_constant_declarations.
*******************************************************************************
*/

#undef  NUL
#define NUL '\0'
#define MAX_XRMSTRING   4096		/* max length of the Xrm DB string  */
#define MAX_ERRMSG      1024		/* max length of error message      */
#define MAX_CHILDREN    1024		/* max number of widget's children  */
#define MAX_PATHNAME    1024		/* max length of the pathname       */
#define INCR_ALLOC        32		/* initial incr of malloc'd arrays  */
#define MAX_CALLBACKS     64            /* max callbacks per Xrm resource   */
#define MAX_WIDGETS      512		/* max depth of a widget tree       */
#define MAX_ROOT_WIDGETS  32		/* max # separate widget trees	    */
#define MAX_RES_FILES    512		/* max # res file names per interf  */
#ifndef MAX_ARGS
#define MAX_ARGS	 128		/* max # args for callback / action */
#define NAME_RESOLUTION	 128		/* #chars resolving registered names*/
#endif

/*
*******************************************************************************
* Private_type_declarations.
*******************************************************************************
*/

typedef struct _WclRec
{
    /* Application Wide Resources - Fetched During Wcl Initialization
    */
    char*	resFiles;
    char*	errorDatabaseFile;
    char*	widgetResourceFiles;
    char*	templateFiles;
    Boolean	traceTemplateDef;
    Boolean	verboseWarnings;
    char*	dynamicLibs;		/* just to force conversion, not used */

} WclRec, *WclRecPtr, *Wcl;

typedef struct	_QuarkRec
{
    char*	string;		/* as seen in resource db		*/
    XrmQuark	quark;		/* made from lower case of string	*/
} QuarkRec;

typedef struct  _ResourceRec
{
    /* Pre-Creation Resources
    */
    Boolean         preCreateDump;	/* dump resources pre-create	*/
    XrmQuark	    template;		/* name of template		*/
    Boolean	    traceTemplate;	/* template trace required	*/
    Boolean         postTemplateDump;	/* dump resources post-template	*/
    QuarkRec	    create;		/* WcMapFind gives class/constr	*/
    QuarkRec	    className;		/* backward compatibility	*/
    QuarkRec	    class;		/* backward compatibility	*/
    QuarkRec	    constructor;	/* backward compatibility	*/

    /* Post-Creation Resources
    */
    Boolean	    trace;		/* creation trace required          */
    Boolean         postCreateDump;	/* dump resources post-create	    */
    XtCallbackList  callback;		/* invoked after creation	    */
    char*	    popups;		/* list of popup children to create */
    char*	    children;		/* list of children names to create */
    Boolean	    managed;		/* created  managed (default TRUE)  */
    XtCallbackList  afterPopups; 	/* invoked after popups created     */
    XtCallbackList  afterChildren; 	/* invoked after children created   */
    XtCallbackList  afterManageChildren;/* invoked after kids are managed   */

} ResourceRec, *ResourceRecPtr;

/* Used by WcLateBinderCB for late binding of callbacks.
*/
typedef struct _WcLateBind 
{
 XtAppContext	app;		/* Always seem to need an app!		*/
 Widget		widget;		/* widget invoking the callback		*/
 XtPointer	callData;	/* callback specific data from widget	*/

 /* These are quarkified versions of what we saw as the callback resource
  * value.  args is also specially derived from what we saw in the callback
  * resource value (careful! see WcxClosureFromSeg() for concerns about
  * being able to remove callbacks).
  */
 XrmQuark	libQ;		/* shared library abbreviation: -lXt	*/
 XrmQuark	classQ;		/* class, for methods			*/
 XrmQuark	nameQ;		/* case sensitive callback name		*/
 XrmQuark	nameq;		/* case insensitive callback name braindamage */

 /* These are needed for dynamic linking of shared libraries
 */
 char*		libFullPath;	/* if doing dynamic linking		*/
 void*		libHandle;	/* from dlopen()			*/

 /* The callback procedure address which gets invoked.
 */ 
 XtCallbackProc	Callback;	/* what finally gets invoked		*/
 char*		args;		/* careful! see WcxClosureFromSeg()	*/
 XtPointer	regClosure;	/* client data as registered		*/
 XtPointer	object;		/* instance of the class - this changes */

} WcLateBindRec, *WcLateBind;

/*
*******************************************************************************
* Private_macro_definitions.
*******************************************************************************
    ONCE_PER_XtAppContext(app) should be invoked at the beginning of each 
    function which performs registration, like WcRegisterWcCallbacks.
    Note that this IS a macro: therefore, the return statement actually
    causes the return from the registration function.
*/

#define ONCE_PER_XtAppContext( app )	\
{					\
    static XtAppContext already[1024];	\
    static int numApps = 0;		\
    int i;				\
					\
    for (i = 0; i < numApps ; i++)	\
        if (app == already[i])		\
            return;			\
					\
    already[numApps++] = app;		\
}

/*
    -- converter done macro
*******************************************************************************
    This macro is invoked when a resource converter has completed the
    conversion.  It is taken directly from the Xt Reference Manual which came
    with Release 4 of the X11 Window System from MIT.
*/

#define done( type, value ) 			\
{						\
    if ( toVal->addr != NULL )			\
    {						\
	if ( toVal->size < sizeof( type ) )	\
	{					\
	    toVal->size = sizeof( type );	\
	    return;				\
	}					\
	*(type*)(toVal->addr) = (value);	\
    }						\
    else					\
    {						\
	static type static_val;			\
	static_val = (value);			\
	toVal->addr = (caddr_t)&static_val;	\
    }						\
    toVal->size = sizeof(type);			\
    return;					\
}

/* For compatibility with old Xt libraries
*/
#ifndef XtIsWidget
#ifdef XtSpecificationRelease
#define XtIsWidget(obj) XtIsSubclass((obj),(WidgetClass)coreWidgetClass)
#else
#define XtIsWidget(obj) XtIsSubclass((obj),(WidgetClass)widgetClass)
#endif
#endif

/*
*******************************************************************************
* Private_function_declarations.
*******************************************************************************
    The following functions are generally private functions to the
    WcCreate routines, but they may be defined in different files from
    where they are used.  Client programs probably should not invoke
    these functions directly.
*/

#if NeedFunctionPrototypes
/****************************** ANSI FUNC DECLS ******************************/
#define CONVERTER(arg) XrmValue*, Cardinal*, XrmValue*, XrmValue*
#define CALLBACK(arg) Widget, XtPointer, XtPointer
#define ACTION(arg) Widget, XEvent*, char**, Cardinal*
#else
/****************************** K&R FUNC DECLS ******************************/
#define CONVERTER(arg) /**/
#define CALLBACK(arg) /**/
#define ACTION(arg) /**/
#endif

/*  -- Wcl Initialization Procedures
*/
extern void WcWarningInitialize		_(( XtAppContext, WclRecPtr ));
extern void WcWidgetResourcesInitialize	_(( XtAppContext, WclRecPtr ));
extern void WcTemplateInitialize	_(( XtAppContext, WclRecPtr ));
extern void WcRegisterIntrinsic		_(( XtAppContext ));
extern void WcAddConverters		_(( XtAppContext ));
extern void WcRegisterWcCallbacks	_(( XtAppContext ));
extern void WcRegisterWcActions		_(( XtAppContext ));

extern int WcMoreResourceFilesToLoad _(( Widget, WclRecPtr ));

/*  -- Wcl Templates
*/
extern int WcApplyTemplate _(( XrmQuark, Widget, char*, int ));

/*  -- Support for providing sub-part resources for widgets (safe WcSetValues)
*/
#define WcWidgetResourcesInitialize(a,w) /* not yet implemented */

/*  -- Find root widget of argument, remember if never seen before
*/
extern Widget WcRootWidget _(( Widget ));

/*  -- Use HOME etc to find user's home directory (static storage of rtn val)
*/
extern char* WcHomeDirectory _(( char* /*user*/ ));

/*  -- String to Widget Converter.  So libXmp and libXp can use easily
*/
extern int wcWidgetCvtArgsCount;
extern XtConvertArgRec wcWidgetCvtArgs[];
extern void WcCvtStringToWidget _((XrmValue*, Cardinal*, XrmValue*, XrmValue*));

/*  -- Callback List stuff for Add/Remove callbacks
*/
XtCallbackRec* WcStringToCallbackList _(( Widget, char* ));
void WcFreeCallbackList _(( XtCallbackRec* ));

/*  -- Perform late binding of callbacks and methods - NOT REGISTERED!
*/
extern void WcLateBinderCB _(( Widget, XtPointer, XtPointer ));

/*  -- Similar to XtWarningMsg()
================================================================
    These use a Widget argument to identify the application context.

    1st str: name of procedure where problem occurred.
    2nd str: name of warning message.
    3rd str: default warning message.
    4th... : args to be put in for %s in warning message.

  TIP:	Wherever possible, use no more than one argument - makes
	translating or changing the messages ALOT easier.
*/
#ifndef WCL_ERRORDB
#define WCL_ERRORDB "/usr/lib/X11/WclErrorDB"
#endif
void WcWARN  _(( Widget, char*, char*, char* ));
void WcWARN1 _(( Widget, char*, char*, char*, char* ));
void WcWARN2 _(( Widget, char*, char*, char*, char*, char* ));
void WcWARN3 _(( Widget, char*, char*, char*, char*, char*, char* ));

char* WcErrorDatabaseText _(( Widget, char*, char* ));

/*  -- Mapping Agents used by Wcl
================================================================
    Need to have these visible only so macros below can be used.
*/
extern MapAg wcAgent, cbAgent, cdAgent, clAgent, conAgent;

/*  -- Mapping Agent Access Macros
=====================================================================
    Used for consistent access to mapping agents.  If a single agent is
    used for more than one type of data, be certain that the arguments
    do NOT collide!
*/

/*============== wcAgent ==============*/
#define WcMapWcl( app, wcl ) \
	MapAg_Define( wcAgent, (app), NULL, NULL, (wcl) )

#define WcMapWclFind( app ) \
	(WclRecPtr)MapAg_Find( wcAgent, (app), NULL, NULL )

#define WcMapDynLib( app, quark, name ) \
	MapAg_Define( wcAgent, (app), (quark), 1, (name) )

#define WcMapDynLibFind( app, quark ) \
	(char*)MapAg_Find( wcAgent, (app), (quark), 1 )

/*============== cbAgent ==============*/
#define WcMapCallback( app, quark, cbRecPtr ) \
	MapAg_Define( cbAgent, (app), (quark), NULL, (cbRecPtr) )

#define WcMapCallbackFind( app, quark ) \
	(XtCallbackRec*)MapAg_Find( cbAgent, (app), (quark), NULL )

#define WcMapCallbackMethod( app, classQ, nameQ, cbRecPtr ) \
	MapAg_Define( cbAgent, (app), (classQ), (nameQ), (cbRecPtr) )

#define WcMapCallbackMethodFind( app, classQ, nameQ ) \
	(XtCallbackRec*)MapAg_Find( cbAgent, (app), (classQ), (nameQ) )

/*============== cdAgent ==============*/
#define WcMapClosure( quark, string ) \
	MapAg_Define( cdAgent, (quark), NULL, NULL, (string) )

#define WcMapClosureFind( quark ) \
	(char*)MapAg_Find( cdAgent, (quark), NULL, NULL )

#define WcMapObject( wid, classQ, object ) \
	MapAg_Define( cdAgent, (wid), (classQ), NULL, (object) )

#define WcMapObjectFind( wid, classQ ) \
	(XtPointer)MapAg_Find( cdAgent, (wid), (classQ), NULL )

#define WcMapObjectForget( wid, classQ ) \
	MapAg_Forget( cdAgent, (wid), (classQ), NULL )

/*============== conAgent ==============*/
#define WcMapConstructor( app, quark, Constr ) \
	MapAg_Define( conAgent, (app), (quark), NULL, (Constr) )

#define WcMapConstructorFind( app, quark ) \
	(WcWidgetConstructor)MapAg_Find( conAgent, (app), (quark), NULL )

/*============== clAgent ==============*/
#define WcMapClass( app, quark, class ) \
	MapAg_Define( clAgent, (app), (quark), NULL, (class) )

#define WcMapClassFind( app, quark ) \
	(WidgetClass)MapAg_Find( clAgent, (app), (quark), NULL )

#endif /* _WcCreateP_h */
