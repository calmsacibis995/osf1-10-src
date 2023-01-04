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
* SCCS_data: @(#) Test.c 1.5 92/06/10 06:17:02
*
* Widget Creation Library Test Program - Test.c
*
* Test.c is derived directly from Mri.c and is used to perform white-box
* testing of Wcl.
*
******************************************************************************
*/


#include <X11/IntrinsicP.h>
#include <X11/Xm/Xm.h>
#include <X11/Wc/WcCreate.h>
#include <X11/Xmp/Xmp.h>

#include <stdio.h>
#include <errno.h>

static XrmOptionDescRec options[] = {
    WCL_XRM_OPTIONS
};

#define RCP( name, class  ) WcRegisterClassPtr   ( app, name, class );
#define RCO( name, constr ) WcRegisterConstructor( app, name, constr );
#define RAC( name, func   ) WcRegisterAction     ( app, name, func );
#define RCB( name, func   ) WcRegisterCallback   ( app, name, func, NULL );
#define RME( class, name, data, func ) \
			    WcRegisterMethod( app, class, name, data, func );

static char *event_names[] = {
"",
"",
"KeyPress",		/* 2 */
"KeyRelease",		/* 3 */
"ButtonPress",		/* 4 */
"ButtonRelease",	/* 5 */
"MotionNotify",		/* 6 */
"EnterNotify",		/* 7 */
"LeaveNotify",		/* 8 */
"FocusIn",		/* 9 */
"FocusOut",		/* 10 */
"KeymapNotify",		/* 11 */
"Expose",		/* 12 */
"GraphicsExpose",	/* 13 */
"NoExpose",		/* 14 */
"VisibilityNotify",	/* 15 */
"CreateNotify",		/* 16 */
"DestroyNotify",	/* 17 */
"UnmapNotify",		/* 18 */
"MapNotify",		/* 19 */
"MapRequest",		/* 20 */
"ReparentNotify",	/* 21 */
"ConfigureNotify",	/* 22 */
"ConfigureRequest",	/* 23 */
"GravityNotify",	/* 24 */
"ResizeRequest",	/* 25 */
"CirculateNotify",	/* 26 */
"CirculateRequest",	/* 27 */
"PropertyNotify",	/* 28 */
"SelectionClear",	/* 29 */
"SelectionRequest",	/* 30 */
"SelectionNotify",	/* 31 */
"ColormapNotify",	/* 32 */
"ClientMessage",	/* 33 */
"MappingNotify",	/* 34 */
"LASTEvent",		/* 35 */	/* must be bigger than any event # */
};

int Test_TraceEvents = 0;

static void TestTraceEventsCB(widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    Test_TraceEvents = 1;
}

static void TestNoTraceEventsCB(widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    Test_TraceEvents = 0;
}

static void TestDeleteWindowCB( widget, clientData, callData )
    Widget	widget;
    XtPointer	clientData;
    XtPointer	callData;
{
    printf("Closed from Mwm frame menu.\n");
}

typedef XrmResource **CallbackTable;

typedef struct internalCallbackRec {
    unsigned short count;
    char           is_padded;   /* contains NULL padding for external form */
    char           call_state;  /* combination of _XtCB{FreeAfter}Calling */
    /* XtCallbackList */
} InternalCallbackRec, *InternalCallbackList;

#define ToList(p) ((XtCallbackList) ((p)+1))

static void TestInvestigateCB( widget, clientData, callData )
    Widget	widget;
    XtPointer	clientData;
    XtPointer	callData;
{
    int				numCbs, numOffs;
    int				inx;
    CallbackTable		offsets;
    char*			cbName;
    char*			cbClass;
    char*			cbType;
    InternalCallbackList*	iclp;
    InternalCallbackList	icl;
    InternalCallbackRec		icr;
    XtCallbackList		xtcbl;

    offsets = (CallbackTable)
	widget->core.widget_class->core_class.callback_private;

    numOffs = (int)*(offsets);
    for (inx = (int) *(offsets++); --inx >= 0; offsets++)
    {
	cbName  = XrmQuarkToString( (*offsets)->xrm_name  );
	cbClass = XrmQuarkToString( (*offsets)->xrm_class );
	cbType  = XrmQuarkToString( (*offsets)->xrm_type  );
	iclp    = (InternalCallbackList*)( (char*)widget - (*offsets)->xrm_offset - 1 );
	icl     = *iclp;
	icr	= *icl; 
	numCbs  = icr.count;
	xtcbl   = (XtCallbackList)( icl+1 );		/* ToList(icl); */
    }
    return;
}

Widget resultWidget;
char*  resultFmt;

static void Thing_ResultWidgetCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    resultWidget = widget;
    resultFmt    = (char*)clientData;
    if ( WcStrStr( resultFmt, "%s" ) == NULL )
	XtError("You must have a %s in the Thing_Result format string.");
}

static void Thing_DisplayResultCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    char* buf;
    if (!resultFmt)
	XtError("You must provide a Thing_Result format string first.");
    buf = XtMalloc( WcStrLen( (char*)clientData ) + WcStrLen( resultFmt) + 1 );
    sprintf( buf, resultFmt, (char*)clientData );
    WcSetValue( resultWidget, buf );
    XtFree( buf );
}


/* Test behavior of Wcl when a widget constructor fails.
*/
static Widget TestConstructorFail( pw, name, args, nargs )
    Widget	pw;
    String	name;
    Arg*	args;
    Cardinal	nargs;
{
    return (Widget)NULL;
}

/* An object, and object methods
 */
typedef struct _TestObj {
    char* name;
    void (*FirstIMF)();		/* instance member function (ptr to func) */
    void (*SecondIMF)();	/* instance member function (ptr to func) */
} TestObjRec, *TestObj;

/* Callback methods
*/
static void TestObj_1stCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    TestObj this = (TestObj)(methodData->object);
    if ( this != (TestObj)0 )
	fprintf(stderr,"TestObj_1stCBM: name=%s, args=%s, closure=%s\n",
		this->name,
		(WcNonNull(methodData->args)?methodData->args:"(nil)"),
		methodData->closure );
    else
	fprintf(stderr,"TestObj_1stCBM: NULL object, args=%s\n",
		(WcNonNull(methodData->args)?methodData->args:"(nil)") );
}

static void TestObj_2ndCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    TestObj this = (TestObj)(methodData->object);
    if ( this != (TestObj)0 )
	fprintf(stderr,"TestObj_2ndCBM: name=%s, args=%s, closure=%s\n",
		   this->name,
		   (WcNonNull(methodData->args)?methodData->args:"(nil)"),
		   methodData->closure );
    else
	fprintf(stderr,"TestObj_2ndCBM: NULL object, args=%s\n",
		(WcNonNull(methodData->args)?methodData->args:"(nil)") );
}

static void TestObj_AttachToCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    TestObj this = (TestObj)(methodData->object);
    Widget target = WcFullNameToWidget( widget, methodData->args );
    if ( target != (Widget)0 )
	WcAttachThisToWidget( (XtPointer)this, "TestObj", target );
    else
	fprintf(stderr,"Could not find %s from %s\n",
			methodData->args, XtName(widget) );
}


/* Does not need object within methodData.
*/
static void TestObj_CreateCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    XtAppContext app = XtWidgetToApplicationContext( widget );
    TestObj this = (TestObj)XtCalloc(sizeof(TestObjRec),1);
    this->name = XtNewString( methodData->args );
    WcAttachThisToWidget( (XtPointer)this, "TestObj", widget );
    WcRegisterMethod( app, "TestObj", "1st", TestObj_1stCBM, "first-data" );
    WcRegisterMethod( app, "TestObj", "2nd", TestObj_2ndCBM, "second-data" );
    WcRegisterMethod( app, "TestObj", "AttachTo", TestObj_AttachToCBM, "" );
}

/* This is a normal callback which is late-bound because it is not
 * originally registered when some widget was created.
 */
static void TestInitiallyNotRegisteredCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    fprintf(stderr,"TestInitiallyNotRegistered(%s)\n",XtName(widget));
}

static void TestLateRegisterCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    XtAppContext app = XtWidgetToApplicationContext( widget );
    RCB( "TestInitiallyNotRegistered", TestInitiallyNotRegisteredCB );
}

static void TestRegisterApplication ( app )
    XtAppContext app;
{
    RCO( "TestConstructorFail", TestConstructorFail	);

    RCB( "TestDeleteWindow",	TestDeleteWindowCB	);
    RCB( "TestTraceEvents",	TestTraceEventsCB	);
    RCB( "TestNoTraceEvents",	TestNoTraceEventsCB	);
    RCB( "TestInvestigate",	TestInvestigateCB	);
    RCB( "Thing_ResultWidget",	Thing_ResultWidgetCB	);
    RCB( "Thing_DisplayResult",	Thing_DisplayResultCB	);
    RCB( "TestLateRegister",	TestLateRegisterCB	);

    WcRegisterMethod( app, "TestObj", "TestObj", TestObj_CreateCBM, "" );
    WcRegisterMethod( app, "TestObj", "Create",  TestObj_CreateCBM, "" );
}

static char* SysErrorMsg(n)
    int n;
{
    extern char *sys_errlist[];
    extern int sys_nerr;
    char *s = ((n >= 0 && n < sys_nerr) ? sys_errlist[n] : "unknown error");

    return (s ? s : "no such error");
}

/* Identical to the Xlib IO error handler, except this dumps core.
*/
static (*DefaultXIOErrorHandler) _((Display*));

int IOErrorHandler( dpy )
    Display* dpy;
{
    (void) fprintf (stderr,
"XIO:  fatal IO error %d (%s) on X server \"%s\"\n",
	errno, SysErrorMsg (errno), DisplayString (dpy));
    (void) fprintf (stderr,
"      after %lu requests (%lu known processed) with %d events remaining.\n",
	NextRequest(dpy) - 1, LastKnownRequestProcessed(dpy), QLength(dpy));

    if (errno == EPIPE) {
	(void) fprintf (stderr,
"      The connection was probably broken by a server shutdown or KillClient.\n"
	);
    }
    abort();
}

/*  -- Main
***********
*/

main ( argc, argv )
    int    argc;
    String argv[];
{   
    XtAppContext app;
    Widget       appShell;

    argv[0] = "Mri";

    DefaultXIOErrorHandler = XSetIOErrorHandler(IOErrorHandler);

    appShell = XtInitialize ( 
	argv[0], WcAppClass( argc, argv ),
	options, XtNumber(options),
	&argc, argv 
    );
    app = XtWidgetToApplicationContext(appShell);

    TestRegisterApplication ( app );

    XmpRegisterAll ( app );

    WcWidgetCreation ( appShell );

    XtRealizeWidget ( appShell );

    XmpAddMwmCloseCallback( appShell, TestDeleteWindowCB, NULL );

    /* Replacement for XtAppMainLoop()
    */
    {
      XEvent event;
      FILE*  log = fopen("log","w");

      for(;;) {
        XtAppNextEvent(app, &event);
        if (Test_TraceEvents)
        {
          Widget widget = XtWindowToWidget(event.xany.display, 
                                           event.xany.window);
          fprintf(log, "%16s %9ld %s %s\n", 
                  event_names[event.type], event.xany.serial,
                  ( event.xany.send_event ? "c" : " " ),
                  ( XtIsWidget(widget) ? widget->core.name : XtName(widget)) );
	  fflush(log);
        }
        XtDispatchEvent(&event);
      }
    }
}
