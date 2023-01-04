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
#include <X11/Wc/COPY>

/*
* SCCS_data: %Z% %M%    %I% %E% %U%
*
* Widget Creation Library - WcLateBind.c - Late Binder for Callbacks and Methods
*
* The LateBinder supports dynamic linking of libraries for callbacks,
* searching up the widget tree to find objects of named classes attached
* to widgets for the purpose of invoking methods, and invoking callbacks
* and methods.
*
* LateBinding allows callbacks and methods to be resolved at invocation time,
* so callbacks can be registered after the widget is created (but still before
* the callback can be invoked!).  Methods always require late binding, because
* we must find the appropriate instance at invocation time: we can RARELY
* assume the instance is constant, or that registration time data can work
* for object pointers.  This is because typical, real programs CHANGE the
* objects represented by interfaces dynamically.  For example, a mail
* interface changes the mail message being operated on by the same wudget
* interface.  A text editor changed the paragraph being manipulated by
* a paragraph property sheet as the insertion point moves.
*
* See also WcCvtStringToCallback() and WcxBuildCallbackList().
*
*******************************************************************************
*/

#ifdef WC_HAS_dlopen_AND_dlsym
#include <dlfcn.h>
#endif

#include <X11/Intrinsic.h>
#include <X11/Wc/WcCreateP.h>

/*  -- Late Binder Methods
************************************************************************
*/

static void  WcxLateBinder_Invoke                  _(( WcLateBind ));
static int   WcxLateBinder_DynamicallyBindCallback _(( WcLateBind ));
static int   WcxLateBinder_BindFromRegistration    _(( WcLateBind ));  
static void  WcxLateBinder_InvokeMethod            _(( WcLateBind ));
static char* WcxLateBinder_InvocationLine          _(( WcLateBind ));

/*  -- Late Binding Callback
************************************************************************
    *** THIS CALLBACK CANNOT BE REGISTERED WITH Wcl  ***
    *** IT CANNOT BE DIRECTLY INVOKED FROM RESOURCES ***
    This callback is added to a widget callback list be WcCvtStringToCallback()
    and by WcStringToCallbackList() when a callback cannot be resolved, when a
    callback references a dynamic library, or when a callback references a
    class name (i.e., the callback is really a method).
*/

/*ARGSUSED*/
void WcLateBinderCB( orig, clientData, callData )
    Widget	orig;
    XtPointer	clientData, callData;
{
    WcLateBind lb = (WcLateBind)clientData;

    lb->widget   = orig;
    lb->callData = callData;

    WcxLateBinder_Invoke( lb );
}

/*  -- Callback Not Defined
************************************************************************
    This callback is invoked when an undefined callback is specified
    in a resource file and then invoked from some widget.  It is used
    for evolutionary software development, and to make an application
    more resistant to typos in the resource files.
*/
/*ARGSUSED*/
static void WcxUndefinedCallbackCB( w, clientData, callData )
    Widget	w;
    XtPointer	clientData, callData;
{
    WcWARN1( w, "WcUndefinedCallback", "invoked",
	"Undefined Callback Invoked: %s", (char*)clientData );
}

/*  -- Set the undefined callback
************************************************************************
*/
static XtCallbackProc WcxUndefinedCallbackAddr = WcxUndefinedCallbackCB;

XtCallbackProc WcSetUndefinedCallback( Proc )
    XtCallbackProc Proc;
{
    XtCallbackProc Old = WcxUndefinedCallbackAddr;

    WcxUndefinedCallbackAddr = Proc;

    return Old;
}


/*  -- Use the WcLateBind object to invoke a callback or a method
************************************************************************
    The WcLateBind object was created by WcCvtStringToCallback() or by
    WcStringToCallbackList().  It was passed as client data to the
    WcLateBinderCB().
*/
static void WcxLateBinder_Invoke( lb )
    WcLateBind	lb;
{
    if ( lb->Callback == (XtCallbackProc)0 )
    {
	/*================================================
	 * We do not have the callback addr, need to bind:
	 *================================================
	 */
	if ( lb->libQ != NULLQUARK )
	{
	    /*==================================================
	     * Library specification - bind from shared library:
	     *==================================================
	     */
	    if ( WcxLateBinder_DynamicallyBindCallback( lb ) )
		return;						/* failed */
	}
	else
	{
	    /*=================================================================
	     * No library spec, so must be registered (known by mapping agent).
	     * If not found, invoke the undefined callback.
	     *=================================================================
	     */
	    if ( WcxLateBinder_BindFromRegistration( lb ) )
		return;						/* failed */
	}
    }

    /*======================================================================
     * OK, we have the callback proc address.  If its a method, need to find
     * the object, otherwise its an old-fashioned Wcl style callback proc.
     *======================================================================
     */

    if ( lb->classQ )
    {
	/*===============================================
	 * Method, find instance from this widget, invoke
	 *===============================================
	 */
	WcxLateBinder_InvokeMethod( lb );
    }
    else
    {
	/*===============================================
	 * Just an old-fashioned, Wcl style callback proc
	 *===============================================
	 */
	lb->Callback( lb->widget, (XtPointer)lb->args, lb->callData );
    }
}

/* -- Dynamically link a shared library to resolve callback proc address
****************************************************************************
   We have a library name, but no callback address yet.
   Return 1 if something goes wrong.
*/
static int WcxLateBinder_DynamicallyBindCallback( lb )
    WcLateBind	lb;
{
#ifdef WC_HAS_dlopen_AND_dlsym
    lb->libFullPath = WcMapDynLibFind( lb->app, lb->libQ );

    if ( (char*)0 == lb->libFullPath )
    {
	/* This abbreviation was not registered.  Complain, for now,
	** someday search LD_RUN_PATH and LD_LIBRARY_PATH
	*/
	char* line = WcxLateBinder_InvocationLine( lb );
	WcWARN2( lb->widget, "WcLateBinder", "libNotRegistered",
"WcLateBinder cannot invoke `%s'\n\
\t- library `%s' not registered",
		line, XrmQuarkToString( lb->libQ ) );
	XtFree( line );
	return 1;
    }

    if ( NULL == (lb->libHandle = dlopen( lb->libFullPath, RTLD_LAZY )) )
    {
	char* line = WcxLateBinder_InvocationLine( lb );
	WcWARN2( lb->widget, "WcLateBinder", "dlopenFailed",
"WcLateBinder cannot invoke `%s'\n\
\t- dlopen() failed, dlerror() says:\n\
%s",
		line, dlerror() );
	XtFree( line );
	return 1;
    }

    /* The callback proc name is case-sensitive, and it may be munged.
    */
    lb->Callback = (XtCallbackProc)dlsym( lb->libHandle,
					  XrmQuarkToString( lb->nameQ ) );

    if ( lb->Callback == (XtCallbackProc)0 )
    {
	char* line = WcxLateBinder_InvocationLine( lb );
	WcWARN2( lb->widget, "WcLateBinder", "dlsymFailed",
"WcLateBinder cannot invoke `%s'\n\
\t- dlsym() failed, dlerror() says:\n\
%s",
		line, dlerror() );
	XtFree( line );
	return 1;
    }
    /* We have sucessfully dynamically bound the callback address.
    */
    return 0;

#else /*WC_HAS_dlopen_AND_dlsym*/

    /*============================================================
     * Library specified, and we have not resolved address before,
     * but this system does not support dynamic binding...
     *============================================================
     */
    char* line = WcxLateBinder_InvocationLine( lb );
    WcWARN1( lb->widget, "WcLateBinder", "notSupported",
"WcLateBinder cannot invoke `%s'\n\
\t- this system does not have dlopen() and dlsym().",
		line );
    XtFree( line );
    return 1;

#endif /*WC_HAS_dlopen_AND_dlsym*/
}

/* Find Callback or Method Address in Mapping Agent 
****************************************************************************
   By now, the address of the callback or method should have been registered.
   If we do not find it, we will invoke the undefined callback.
*/

static int WcxLateBinder_BindFromRegistration( lb )
    WcLateBind	lb;
{
    XtCallbackRec* cbRecPtr;

    if ( lb->classQ )
    {
	/* Methods are case sensitive
	*/
	cbRecPtr = WcMapCallbackMethodFind( lb->app, lb->classQ, lb->nameQ );
    }
    else
    {
	/* Support historical brain fade, callbacks are NOT case sensitive
	*/
	cbRecPtr = WcMapCallbackFind( lb->app, lb->nameq );
    }
    if ( cbRecPtr == (XtCallbackRec*)0 )
    {
	/* Note!  undefined callback is invoked for undefined methods too!
	*/
	char* line = WcxLateBinder_InvocationLine(lb);
	WcxUndefinedCallbackAddr( lb->widget, (XtPointer)line, (XtPointer)0 );
	XtFree( line );
	return 1;
    }
    /* We have found the registered callback address.
    */
    lb->Callback   = cbRecPtr->callback;
    lb->regClosure = cbRecPtr->closure;

    return 0;
}

/* Find Object - Default Function
***************************************************************************
   Wcl by default finds the object from the widget by searching up the
   widget tree.  The application may want to use any other mechanism, like
   looking in a dictionary, asking a server process, whatever.  We do not
   care if the object is found: the methods must be able to deal with getting
   a NULL object pointer.
*/
static XtPointer WcxFindObject_DefaultSearch( w, classQ )
    Widget   w;		/* the widget whcih invoked the callback */
    XrmQuark classQ;	/* quarkified case sensitive class name */
{
    XtPointer object;
    do
    {
	object = WcMapObjectFind( w, classQ );
    }
    while ( object == (XtPointer)0 && (Widget)0 != ( w = XtParent(w) ) );

    return object;
}

static WcFindObjectFunc WcxFindObject = WcxFindObject_DefaultSearch;

WcFindObjectFunc WcSetFindObjectFunc( FindFunc )
    WcFindObjectFunc FindFunc;
{
    WcFindObjectFunc Old = WcxFindObject;
    WcxFindObject = FindFunc;
    return Old;
}

/* Invoke Method after finding the appropriate object instance
**********************************************************************
*/
static void WcxLateBinder_InvokeMethod( lb )
    WcLateBind	lb;
{
    WcMethodDataRec methodData;

    lb->object = WcxFindObject( lb->widget, lb->classQ );

    if ( lb->object == (XtPointer)0
      && (WcMapWclFind(lb->app))->verboseWarnings )
    {
	/*==========================================
	 * Could not find object of appropriate type
	 * - complain if the user wanted verbose junk
	 *==========================================
	 */
	char* line     = WcxLateBinder_InvocationLine( lb );
	char* fullName = WcWidgetToFullName( lb->widget );

	WcWARN3( lb->widget, "WcLateBinder", "cantFindObject",
"WcLateBinder trying to invoke `%s'\n\
\tno object of class `%s'\n\
\tfound from widget %s",
		line, XrmQuarkToString( lb->classQ ), fullName );
	XtFree( line );
	XtFree( fullName );
    }

    /*===============================================================
     * Invoke Method: the clientData is a WcCallbackMethodRec pointer
     * instead of just an XtPointer.
     *===============================================================
     */
    methodData.object  = lb->object;	/* might be NULL! */
    methodData.args    = lb->args;
    methodData.closure = lb->regClosure;
    lb->Callback( lb->widget, (XtPointer)(&methodData), lb->callData );
}


/*  -- String from LateBinder data, for warning messages
************************************************************************
    We do not care how slow this proc is, its just for warning messages.
    XtFree the returned string!
*/
static char* WcxLateBinder_InvocationLine( lb )
    WcLateBind lb;
{
    char*	line;
    int		len = 1;	/* always at least a null terminator */

    if ( lb->libQ )	len += 2 + WcStrLen( XrmQuarkToString( lb->libQ ) );
    if ( lb->classQ )	len += 2 + WcStrLen( XrmQuarkToString( lb->classQ ) );
    len += WcStrLen( XrmQuarkToString( lb->nameQ ) );
    len += 2 + WcStrLen( lb->args );

    line = XtCalloc( len, 1 );

    if ( lb->libQ )
    {
	WcStrCat( line, XrmQuarkToString(lb->libQ) );   WcStrCat( line, "::" );
    }
    if ( lb->classQ )
    {
	WcStrCat( line, XrmQuarkToString(lb->classQ) ); WcStrCat( line, "::" );
    }
    WcStrCat( line, XrmQuarkToString(lb->nameQ ) );
    WcStrCat( line, "(" ) ; WcStrCat( line, lb->args ) ; WcStrCat( line, ")" );

    return line;
}

