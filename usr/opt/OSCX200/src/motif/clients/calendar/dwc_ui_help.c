/* dwc_ui_help.c */
#ifndef lint
static char rcsid[] = "$Id$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Marios Cleovoulou,  November-1987
**	Denis G. Lacroix,   April-1989
**
**  ABSTRACT:
**
**	Generic help routines.
**
**--
*/

#define HELP_MODULE 1

#include "dwc_compat.h"

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/cursorfont.h>		/* temporarily for oncontext cursor */
#include <Xm/XmP.h>
#include <X11/ShellP.h>
#include <X11/VendorP.h>
#include <DXm/DXmHelpB.h>		/* DxmNfirstTopic, etc. */
#include <DXm/DXmHelpSP.h>		/* helpshell widgetclass */
#include <X11/DECwI18n.h>
#include <DXm/DECspecific.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_dateformat.h"
#include "dwc_ui_calendar.h"
#include "dwc_ui_catchall.h"		/* DW$DRM... */
#include "dwc_ui_helpp.h"
#include "dwc_ui_help_const.h"
#include "dwc_ui_help.h"	
#include "dwc_ui_misc.h"		/* MISCRaiseToTheTop */
#include "dwc_ui_icons.h"

/*
**  Global Data Definitions
*/
XmString DwcCSHelpLibrary;
XmString DwcCSHelpOnContext;
XmString DwcCSHelpOnWindow;
XmString DwcCSHelpOnTerms;
XmString DwcCSHelpOnVersion;
XmString DwcCSHelpOnHelp;

XtCallbackRec	HELPwidget_help_cb[2] =
{
    {(XtCallbackProc) HELPForWidget,	NULL},
    {NULL,				NULL}
};


#if defined(HELPWIDGET)
static void unmap_help PROTOTYPE ((Widget w));

static XtCallbackRec unmap_helpCB [2] =
{
    {(XtCallbackProc) unmap_help,	NULL},
    {NULL,				NULL}
};

static Widget find_help PROTOTYPE ((Widget w));
#endif

#if defined(OLD_HYPERHELP)
static void help_error PROTOTYPE ((char *problem_string, int status));
#endif

/*
**  External Data Declarations
*/
extern AllDisplaysRecord ads;		/* defined in dwc_ui_calendar.c */


void HELPInitialize
#ifdef _DWC_PROTO_
	(AllDisplaysRecord *ads)
#else	/* no prototypes */
	(ads)
	AllDisplaysRecord *ads;
#endif	/* prototype */
{
    
#if defined(NEW_HYPERHELP)
#elif defined(OLD_HYPERHELP)
#elif defined(HELPWIDGET)
    DwcCSHelpOnContext = XmStringCreateSimple(DwcTHelpOnContext);
    DwcCSHelpOnWindow = XmStringCreateSimple(DwcTHelpOnWindow);
    DwcCSHelpOnTerms = XmStringCreateSimple(DwcTHelpOnTerms);
    DwcCSHelpOnVersion = XmStringCreateSimple(DwcTHelpOnVersion);
    DwcCSHelpOnHelp = XmStringCreateSimple(DwcTHelpOnHelp);

    DwcCSHelpLibrary	= XmStringCreateSimple(DwcTHelpLibrary);
#endif
    /*
    **  That's it
    */
    return;
}

void HELPClose
#if defined(_DWC_PROTO_)
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
{
#if defined(NEW_HYPERHELP)
    /* put new hyper help close function here */
#endif
}

void HELPInitializeForDisplay
#if defined(_DWC_PROTO_)
    (CalendarDisplay cd)
#else
    (cd)
    CalendarDisplay	cd;
#endif
{
#if defined(NEW_HYPERHELP)
#elif defined(OLD_HYPERHELP)
    DXmHelpSystemOpen
    (
	&cd->help_context,
	cd->toplevel,
	DwcTHelpLibrary,
	help_error,
	"Help System Error"
    );
#elif defined(HELPWIDGET)
#endif
}

void HELPCloseForDisplay
#if defined(_DWC_PROTO_)
    (CalendarDisplay cd)
#else
    (cd)
    CalendarDisplay	cd;
#endif
{
#if defined(NEW_HYPERHELP)
#elif defined(OLD_HYPERHELP)
    if (cd->help_context != (Opaque)0)
	DXmHelpSystemClose (cd->help_context, help_error, "Help System Error"); 
#elif defined(HELPWIDGET)
#endif
}

void HELPFromMenu
#ifdef _DWC_PROTO_
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    XmString	    CStopic;
    char	    *ttopic;
    int		    status;
    CalendarDisplay cd;
    Widget	    track_widget;
    Cursor	    fontcursor;
    int		    type_of_help;
    Time	    event_time;

    status = MISCFindCalendarDisplay(&cd, widget);

    type_of_help = (int)*tag;

    switch (type_of_help)
    {
    case k_oncontext:
	DXmHelpOnContext (cd->toplevel, False);
	return;
    case k_onwindow:
	ttopic = DwcTHelpOnWindow;
	break;
    case k_onterms:
	ttopic = DwcTHelpOnTerms;
	break;
    case k_onversion:
	ttopic = DwcTHelpOnVersion;
	break;
    case k_onhelp:
	ttopic = DwcTHelpOnHelp;
	break;
    default:
	ttopic = DwcTHelpOnWindow;
	break;
    }
    
    HELPForWidget (widget, (caddr_t) ttopic, cbs);

    /*
    **  That's it
    */
    return;
}

void HELPForWidget
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    Widget		    cursor_widget;
    CalendarDisplay	    cd;
    int			    status;
    char		    *topic = (char *) tag;
    Widget		    foo;

    status = MISCFindCalendarDisplay (&cd, w);

#if defined(NEW_HYPERHELP) || defined(OLD_HYPERHELP)
    if (!status)
    {
	HELPDisplay(NULL, w, topic);
    }
    else
    {
	HELPDisplay ((CalendarDisplay)cd, cd->toplevel, topic);
    }
#elif defined(HELPWIDGET)
    if (!status)
    {
	HELPDisplay
	    (w, w, &foo, topic, TRUE);
    }
    else
    {
	HELPDisplay
	    (cd->toplevel, cd->toplevel, &(cd->main_help_widget), topic, TRUE);
    }
#endif

    /*
    **  That's it
    */
    return;
} 

void HELPForModalWidget
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
#if defined(NEW_HYPERHELP) || defined(OLD_HYPERHELP)
    Widget		parent_widget;
    Widget		help_widget;
    char		*topic = (char *)tag;
    Boolean		hw_exists;
    CalendarDisplay	cd;
    int			status;

    status = MISCFindCalendarDisplay (&cd, w);

    if (!status)
    {
	HELPDisplay(NULL, w, topic);
    }
    else
    {
        HELPDisplay ((CalendarDisplay)cd, cd->toplevel, topic);
    }

    /*
    **  That's it
    */
    return;
#elif defined(HELPWIDGET)
    Widget		parent_widget;
    Widget		help_widget;
    char		*topic = (char *)tag;
    Boolean		hw_exists;
    int			status;
    CalendarDisplay	cd;
    Widget		foo;


    status = MISCFindCalendarDisplay (&cd, w);

    if (!status)
    {
	HELPDisplay (w, w, &foo, topic, TRUE);
    }
    else
    {
	parent_widget = MISCFindParentShell (w);
	help_widget = find_help (parent_widget);
	hw_exists = (help_widget != NULL);
	HELPDisplay
	    (cd->toplevel, parent_widget, &help_widget, topic, hw_exists);
    }

    /*
    **  That's it
    */
    return;
#endif
} 

void HELPDisplay
#if defined(OLD_HYPERHELP) || defined(NEW_HYPERHELP)
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Widget		cursor_widget,
	char		*topic
	)
#else	/* no prototypes */
	(cd, cursor_widget, topic)
	CalendarDisplay	cd;
	Widget		cursor_widget;
	char		*topic;
#endif	/* prototype */
#else
#ifdef _DWC_PROTO_
	(
	Widget		parent_widget,
	Widget		cursor_widget,
	Widget		*help_widget,
	char		*topic,
	Boolean		reuse)
#else	/* no prototypes */
	(parent_widget, cursor_widget, help_widget, topic, reuse)
	Widget		parent_widget;
	Widget		cursor_widget;
	Widget		*help_widget;
	char		*topic;
	Boolean		reuse;
#endif	/* prototype */
#endif
{
    static Opaque  help_context = (Opaque) 0;
#if defined(OLD_HYPERHELP) || defined(NEW_HYPERHELP)
    /*
    **  Put up wait cursor
    */
    if (cd != NULL)
	ICONSWaitCursorDisplay(cursor_widget, ads.wait_cursor);

    if (topic == NULL)
    {
	topic = DwcTHelpOnWindow;
    }
#if defined(OLD_HYPERHELP)
    if (cd == NULL)
    {
	if (help_context == (Opaque) 0)
	{
	    DXmHelpSystemOpen
	    (
		&help_context,
		cursor_widget,
		DwcTHelpLibrary,
		help_error,
		"Help System Error"
	    );
	}
    }
    else
    {
	if (cd->help_context == (Opaque)0)
	    HELPInitializeForDisplay (cd);
	help_context = cd->help_context;
    }

    DXmHelpSystemDisplay
    (
	help_context,
	DwcTHelpLibrary,
	"topic",
	topic,
	help_error,
	"Help System Error"
    );
#elif defined(NEW_HYPERHELP)
#endif
    /*
    **  Remove wait cursor
    */
    if (cd != NULL)
	ICONSWaitCursorRemove (cursor_widget);

    /*
    **  That's it
    */
    return;
#elif defined(HELPWIDGET)
    Cursor	cursor;
    Cardinal	ac;
    Arg		arglist [10];		
    int		argcount;        
    MrmType	class;
    int		status;
    XmString	help_tag;
    MrmCode	code;
    char	real_topic[32];

    /*
    **  Put up wait cursor
    */
    ICONSWaitCursorDisplay(cursor_widget, ads.wait_cursor);

    if (topic == NULL)
    {
	topic = DwcTHelpOnWindow;
    }

    strcpy (real_topic, "OLD_HLP_");
    strncat (real_topic, topic, 31-strlen("OLD_HLP_"));
    status = MrmFetchLiteral
    (
	ads.hierarchy,
	real_topic,
	XtDisplay(ads.root),
	(char **)&help_tag,
	&code
    );


    ac = 0;
    XtSetArg(arglist[ac], DXmNfirstTopic,  help_tag); ac++;
    XtSetArg(arglist[ac], DXmNlibrarySpec, DwcCSHelpLibrary); ac++;

    if (reuse)
    {
	/*
	** Try to reuse an already existing one
	*/
	if (*help_widget == NULL)
	{
            /*	  
	    **  Doesn't exist, get it from scratch
	    */	  
            status = MrmFetchWidgetOverride
	    (
		ads.hierarchy,
		"help_box",
		parent_widget,
		NULL,
		arglist,
		ac,
		help_widget,
		&class
	    );
	    if (status != MrmSUCCESS)
		DWC$UI_Catchall(DWC$DRM_HELP, status, 0);
	}
	else
	{
            /*	  
	    **  Set the appropriate firstTopic and librarySpec for the existing
	    **	one.
	    */	  
            XtSetValues(*help_widget, arglist, ac);
	}
    }
    else
    {
	/*
	** Don't bother reusing an existing one. Make sure that our unmap
	** callback gets processed so that the toolkit bug doesn't get us.
	*/
	XtSetArg(arglist[ac], XmNautoUnmanage, FALSE); ac++;
	XtSetArg(arglist[ac], XmNunmapCallback, unmap_helpCB); ac++;

	status = MrmFetchWidgetOverride
	(
	    ads.hierarchy,
	    "help_box",
	    parent_widget,
	    NULL,
	    arglist,
	    ac,
	    help_widget,
	    &class
	);
    }
	
    XtManageChild(*help_widget);
    MISCRaiseToTheTop(*help_widget);

    /*
    **  Remove wait cursor
    */
    ICONSWaitCursorRemove (cursor_widget);

    XmStringFree (help_tag);

    /*
    **  That's it
    */
    return;
#endif
}

#if defined(HELPWIDGET)
static void unmap_help
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    XtUnmanageChild (w);
    XtDestroyWidget (w);
} 
#endif

/*
**++
**  Functional Description:
**	HELPDestroyForModal
**	This routine goes up the widget tree to find the parent shell,
**	the through the popup list looking for help widgets to be 
**	destroyed.
**
**  Keywords:
**	None.
**
**  Arguments:
**	TBD.
**
**  Result:
**	TBD..
**
**  Exceptions:
**	None.
**--
*/
void HELPDestroyForModal
#ifdef _DWC_PROTO_
	(
	Widget			w,
	int			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	int			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
#if defined(HELPWIDGET)
    Widget		parent_widget;
    Widget		help_widget;

    parent_widget = MISCFindParentShell( w );
    help_widget = find_help( parent_widget );

    if (help_widget != NULL)
    {
	XtDestroyWidget( help_widget );
    }

    ICONSInactiveCursorRemove(w);

    return;
#endif
}

#if defined(HELPWIDGET)
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine takes a shell class widget and find the associated
**	help widget.
**
**  FORMAL PARAMETERS:
**
**      [@description or none@]
**
**  IMPLICIT INPUTS:
**
**      [@description or none@]
**
**  IMPLICIT OUTPUTS:
**
**      [@description or none@]
**
**  {@function value or completion codes@}
**
**      [@description or none@]
**
**  SIDE EFFECTS:
**
**      [@description or none@]
**
**--
*/
static Widget find_help
#ifdef _DWC_PROTO_
	(
	Widget	parent)
#else	/* no prototypes */
	(parent)
	Widget	parent;
#endif	/* prototype */
{
    int	    i;
    Widget  *children;
    Widget  help_widget;

    help_widget = NULL;
    children    = parent->core.popup_list;

    for (i = 0;  i < parent->core.num_popups;  i++)
    {
	if (XtClass (children [i]) == (WidgetClass)dxmHelpShellWidgetClass)
	{
	    children    = (Widget *) DWC_CHILDREN(children [i]);
	    help_widget = children [0];
	    break;
	}
    }
    
    return help_widget;
}
#endif

#if defined(OLD_HYPERHELP)

static void help_error
#if defined(_DWC_PROTO_)
    (
    char    *problem_string,
    int     status
    )
#else
(problem_string, status)
    char    *problem_string;               
    int     status;
#endif
/*
**+
**
** Brain dead error routine for fatal help system errors for new error handling.
**
**-
*/
{
    printf("%s, %x\n", problem_string, status);
    return;
}
#endif
