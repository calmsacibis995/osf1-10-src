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

#ifdef HYPERHELP

#include <stdio.h>
#include "smdata.h"

/* Definitions for Help pulldown selections */

#define k_help_about		1
#define k_help_overview		2
#define	k_help_oncontext	3
#define k_help_onwindow		4
#define k_help_onhelp		5
#define k_help_onversion	6


/* Process errors in help system.  This implementation assumes that such
 * errors are fatal.
 */

void help_error(problem_string, status)
    char    *problem_string;
    int     status;

{
    fprintf(stderr, "%s, %x\n", problem_string, status);
    exit(0);
}



/* Callback for selections from Help pulldown menu */

void help_menu_cb(w, tag, reason)
    Widget		w;
    int			*tag;
    XmAnyCallbackStruct	*reason;

{
    switch (*tag) {
	case k_help_about :
	    DXmHelpSystemDisplay(help_context, dxsession_help, "topic", 
				 "about", help_error, "Help System Error");
	    break;
	case k_help_overview :
	    DXmHelpSystemDisplay(help_context, dxsession_help, "topic", 
				 "overview", help_error, "Help System Error");
	    break;
	case k_help_oncontext :
	    DXmHelpOnContext(smdata.toplevel, FALSE);
	    break;
	case k_help_onwindow :
	    DXmHelpSystemDisplay(help_context, dxsession_help, 
				 "topic", "help_menu_onwindow", 
				 help_error, "Help System Error");
	    break;
	case k_help_onhelp :
	    DXmHelpSystemDisplay(help_context, dxsession_help, 
				 "topic", "help_menu_onhelp", 
				 help_error, "Help System Error");
	    break;
	case k_help_onversion :
	default :
	    DXmHelpSystemDisplay(help_context, dxsession_help, 
				 "topic", "help_menu_onversion", 
				 help_error, "Help System Error");
	    break;
    }
    return;
}



/* Help system callback for context help.  Creates a help system session */

void help_system_proc(w, tag, reason)
    Widget		w;
    int			*tag;
    XmAnyCallbackStruct	*reason;

{
    DXmHelpSystemDisplay(help_context, dxsession_help, "topic", tag,
			 help_error, "Help System Error");
}



#else

#ifdef DOHELP

#include "smdata.h"
#include "smconstants.h"
#include <DXm/DECspecific.h>
#include <DXm/DXmHelpB.h>

/*
 * Prototype
 */
XmString get_drm_message ();

/*
! Table of contents
*/
static void help_unmap ();

int	help_menu_cb(widgetID, tag, reason)
Widget	*widgetID;
caddr_t	tag;
XmRowColumnCallbackStruct	*reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  This is the callback routine when the user selects an entry on the HELP
**  pulldown menu.  It creates and manages the help widget and selects the
**  first topic to be displayed to the user.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
XmString	libname, topic;
unsigned int 	ac;
Arg		arglist[10];
static	XtCallbackRec	help_unmap_CB [2] = {
    		{help_unmap, NULL},
    		{NULL, NULL}
		};

/* If the help widget is already being displayed, this routine simply
   pops the new topic into the existing widget.
*/

topic = NULL;
if (reason->widget == smdata.help_about)
    topic = get_drm_message(k_help_about_msg);
else if (reason->widget == smdata.help_overview)
    topic = get_drm_message(k_help_overview_msg);

if (smdata.help_widget == NULL) 
    {
    libname = get_drm_message (k_help_ultrix_msg);
    XtSetArg (arglist[0], DXmNfirstTopic, topic);
    XtSetArg (arglist[1], DXmNlibrarySpec, libname);
    XtSetArg (arglist[2], XmNunmapCallback, help_unmap_CB);
    MrmFetchWidgetOverride(s_DRMHierarchy, "application_help", smdata.toplevel,
        "application_help", arglist, 3, &smdata.help_widget, &drm_dummy_class);
    XtManageChild (smdata.help_widget);
    }
else
    {
    ac = 0;
    XtSetArg (arglist[ac], DXmNfirstTopic, topic); ac++;
    XtSetValues (smdata.help_widget, arglist, ac);
    }
}

static void help_unmap (widget)
Widget	widget;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects "Exit" in the help widget. 
**	just delete the widget.  If the user requests help again, the widget 
**	will be recreated from scratch.    
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**	
**	We should probably be re-using the help widget.
**--
**/
{
XtUnmanageChild (widget);
XtDestroyWidget (widget);
smdata.help_widget = NULL;
}

#endif /* DOHELP */

#endif /* not HYPERHELP */
