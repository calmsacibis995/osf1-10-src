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
#include <X11/Xcp/COPY>
/*
* SCCS_data: @(#) Xcp.c	1.1 92/03/18 11:08:26
*
* Xcp - Cornell Widget Set Public Utilities for Wcl - Xcp.c
*
* This module contains registration routine, and convenience
* callbacks for the Cornell widget set.
*
*******************************************************************************
*/

#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xcp/Xcp.h>
#include <X11/Xcp/XcuCornell.h>		/* All Cornell widgets */
#include <X11/Xp/Table.h>

#include <X11/Wc/WcCreateP.h>

#ifdef DEBUG
#include <X11/Xcp/XcuCornellP.h>
#endif

/* Shorthand to make Wcl registration easier
*/
#define RCP( name, class ) WcRegisterClassPtr  ( app, name, class );
#define RCR( name, func )  WcRegisterConstructor(app, name, func  );
#define RCB( name, cb )    WcRegisterCallback( app, name, cb, NULL );

/* Some arbitrary constants for Xcp */
#define MAX_BMGR_WIDGETS         20        /* Arbitrary */

void XcpRegisterAll(app)
XtAppContext app;
{
  XcpRegisterCornell(app);
}

void CriRegisterCornell(app)
XtAppContext app;
{
  XcpRegisterCornell(app);
}

void XcpRegisterCornell(app)
XtAppContext app;
{
  ONCE_PER_XtAppContext( app );

  /* Add the Cornell specific callbacks */
  XcuRegisterXcuCBs( app );

    /* -- register the Table widget classes */
  RCP("XpTable",		xpTableWidgetClass	)
  RCP("xpTableWidgetClass",	xpTableWidgetClass	)

  /* -- register all Cornell Widget classes */
  RCP("Command",		xcuCommandWidgetClass	)
  RCP("xcuCommandWidgetClass",  xcuCommandWidgetClass	)
  RCP("Label",			xcuLabelWidgetClass	)
  RCP("xcuLabelWidgetClass",	xcuLabelWidgetClass	)
  RCP("Button",                 xcuButtonWidgetClass    )
  RCP("xcuButtonWidgetClass",   xcuButtonWidgetClass    )
  RCP("Simple",			xcuSimpleWidgetClass	)
  RCP("xcuSimpleWidgetClass",	xcuSimpleWidgetClass	)

  /* Composite and Constraint Widgets */
  RCP("ButtManager",            xcuBmgrWidgetClass      )
  RCP("xcuBmgrWidgetClass",     xcuBmgrWidgetClass      )
  RCP("Deck",			xcuDeckWidgetClass	)
  RCP("xcuDeckWidgetClass",	xcuDeckWidgetClass	)
  RCP("RowCol",			xcuRcWidgetClass	)
  RCP("xcuRcWidgetClass",	xcuRcWidgetClass	)
  RCP("Tbl",			xcuTblWidgetClass	)
  RCP("xcuTblWidgetClass",	xcuTblWidgetClass	)
/* Not supported yet.
  RCP("Wlm",                    xcuWlmWidgetClass       )
  RCP("xcuWlmWidgetClass",      xcuWlmWidgetClass       )
*/
  /* Others */
  RCP("Entry",                  xcuEntryWidgetClass     )
  RCP("xcuEntryWidgetClass",    xcuEntryWidgetClass     )
}

/* -----Xcu specific callbacks----------*/
/* First the button manager */

/* Callback for XcuBmgrManage.  This is wired to the not-to-elegant
 * value MAX_BMGR_WIDGETS.  I didn't want to count the names or allocate
 * memory on the fly. Maybe next time.
 */
void XcuBmgrManageCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  char cleanName[MAX_PATHNAME];
  Widget buttonwidgets[MAX_BMGR_WIDGETS];
  caddr_t buttonvals[MAX_BMGR_WIDGETS];
  int buttoncount;
  char *children;
  Widget child;
  char *wnames = (char*)client_data;

  if ( WcNull(wnames) )
  {
    WcWARN( w, "XcuBmgrManage", "noNames",
	"XcuBmgrManage() - No widget names provided.", XtName(w));
    WcWARN( w, "XcuBmgrManage", "usage",
	"Usage: XcuBmgrManage( button [,button ] ...)" );
    return;
  }

  /* Get the widget values for each of the bmgr children
  */
  for ( buttoncount = 0, children = wnames  ;  WcNonNull(children)  ;  )
  {
    if (buttoncount >= MAX_BMGR_WIDGETS)
    {
      WcWARN1( w, "XcuBmgrManage", "tooManyButtons",
		"XcuBmgrManage(%s) - Too many buttons to manage.",
		wnames );
      break;
    }
    children = WcCleanName(children, cleanName);
    children = WcSkipWhitespace_Comma(children);

    if ((Widget)0 != (child = WcFullNameToWidget(w, cleanName)))
    {
      buttonwidgets[buttoncount] = child;
      buttonvals[buttoncount] = (caddr_t)buttoncount; /* seems silly... */
      buttoncount++;		/* only increment if found the widget */
    }
    else
    {
      WcWARN2( w, "XcuBmgrManage", "buttonNotFound",
	"XcuBmgrManage() could not find %s from %s",
	cleanName, XtName(w) );
    }
  }
  /* Call the Xcu function with the arrays */
  XcuBmgrManage(w, buttonwidgets, buttonvals, buttoncount);
}

typedef void (*XcuMgrMethod) _(( Widget ));

static void Invoke( w, xcuName, caller, type, Method )
  Widget	w;
  char*		xcuName;
  char*		caller;		/* for warning messages */
  char*		type;		/* for warning messages (bmgr or deck) */
  XcuMgrMethod	Method;
{
  Widget	xcuWidget;
  char		cleanName[MAX_XRMSTRING];
  char		msg[MAX_XRMSTRING];

  if ( WcNull(xcuName) )
  {
    sprintf( msg, "%s(): Need name of %s widget", caller, type );
    WcWARN( w, caller, "noName", msg );
    return;
  }

  (void)WcCleanName(xcuName, cleanName);
  if ( (Widget)0 == (xcuWidget = WcFullNameToWidget(w, cleanName)) )
  {
    /* need to maintain %s in format string */
    sprintf( msg, "%s(%s): Widget not found from %s", caller, "%s", "%s" );
    WcWARN2( w, caller, "notFound", msg, cleanName, XtName(w) );
    return;
  }

  Method( xcuWidget );
}

void XcuBmgrSampleCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  Invoke( w, (char*)client_data, "XcuBmgrSample", "XcuBmgr", XcuBmgrSample );
}

void XcuBmgrSetAllCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  Invoke( w, (char*)client_data, "XcuBmgrSetAll", "XcuBmgr", XcuBmgrSetAll );
}

void XcuBmgrUnsetAllCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  Invoke( w, (char*)client_data, "XcuBmgrUnsetAll", "XcuBmgr", XcuBmgrUnsetAll);
}

void XcuBmgrToggleAllCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  Invoke(w, (char*)client_data, "XcuBmgrToggleAll", "XcuBmgr",XcuBmgrToggleAll);
}

/* -------Support for decks----------- */

void XcuDeckRaiseLowestCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  Invoke(w, (char*)client_data,
	"XcuDeckRaiseLowest", "XcuDeck", XcuDeckRaiseLowest );
}

void XcuDeckLowerHighestCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  Invoke(w, (char*)client_data,
	"XcuDeckLowerHighest", "XcuDeck", XcuDeckLowerHighest );
}

void XcuDeckRaiseWidgetCB(w, client_data, call_data)
  Widget	w;
  XtPointer	client_data, call_data;
{
  char* names = (char*)client_data;
  if ( WcNull(names) )
  {
    WcWARN( w, "XcuDeckRaiseWidget", "noNames",
	"XcuDeckRaiseWidget(): Need name of deck and widget to raise" );
    return;
  }
  else
  {
    Widget	deckWidget, raiseWidget;
    char	cleanName[MAX_XRMSTRING];
    char*	raiseName = WcCleanName( names, cleanName );

    if ( (Widget)0 == (deckWidget = WcFullNameToWidget( w, cleanName )) )
    {
      WcWARN1( w, "XcuDeckRaiseWidget", "deckNotFound",
	"XcuDeckRaiseWidget( %s ... ): Deck widget not found", cleanName );
      return;
    }
    raiseName = WcSkipWhitespace_Comma( raiseName );
    (void)WcCleanName( raiseName, cleanName );
    if ( (Widget)0 == (raiseWidget = WcFullNameToWidget( w, cleanName )) )
    {   
      WcWARN1( w, "XcuDeckRaiseWidget", "raiseNotFound",
        "XcuDeckRaiseWidget( %s ): Widget to raise not found", names );
      return;
    }
    
  XcuDeckRaiseWidget(deckwidget, raisewidget);
}

/* Register all the Xcu callbacks */
void XcuRegisterXcuCBs( app )
  XtAppContext app;
{
  ONCE_PER_XtAppContext( app );

  RCB("XcuBmgrManageCB",	XcuBmgrManageCB)
  RCB("XcuBmgrManage",		XcuBmgrManageCB)
  RCB("XcuBmgrSampleCB",	XcuBmgrSampleCB)
  RCB("XcuBmgrSample",		XcuBmgrSampleCB)
  RCB("XcuBmgrSetAllCB",	XcuBmgrSetAllCB)
  RCB("XcuBmgrSetAll",		XcuBmgrSetAllCB)
  RCB("XcuBmgrUnsetAllCB",	XcuBmgrUnsetAllCB)
  RCB("XcuBmgrUnsetAll",	XcuBmgrUnsetAllCB)
  RCB("XcuBmgrToggleAllCB",	XcuBmgrToggleAllCB)
  RCB("XcuBmgrToggleAll",	XcuBmgrToggleAllCB)
  RCB("XcuDeckLowerHighestCB",	XcuDeckLowerHighestCB)
  RCB("XcuDeckLowerHighest",	XcuDeckLowerHighestCB)
  RCB("XcuDeckRaiseLowestCB",	XcuDeckRaiseLowestCB)
  RCB("XcuDeckRaiseLowest",	XcuDeckRaiseLowestCB)
  RCB("XcuDeckRaiseWidgetCB",	XcuDeckRaiseWidgetCB)
  RCB("XcuDeckRaiseWidget",	XcuDeckRaiseWidgetCB)
/*
  RCB("XcuBmgrSetChildCB",	XcuBmgrSetChildCB)
  RCB("XcuBmgrUnsetChildCB",	XcuBmgrUnsetChildCB)
*/
}
