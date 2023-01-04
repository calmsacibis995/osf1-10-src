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
static char *rcsid = "@(#)$RCSfile: hyperhelp.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/12/20 21:30:44 $";
#endif

#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#include <stdio.h>
#include <DXm/DXmHelpB.h>
#include <X11/ShellP.h>
#include <DXm/DXmHelpSP.h>
#include <Xm/MessageB.h>

/* "handle" for Help calls */

static Opaque xif_hyperhelp_context;
#define k_xif_hyperhelp_path "xif"

static char *help_topic[] = {
"isso_overview",
"keyboard",
"help",
"mod_user_box",
"mod_temp_box",
"ad_overview",
"create_acc_box",
"create_group_box",
"retire_dialog_box",
"select_dev_box",
"mod_print_box",
"add_term_box",
"add_remove_box",
"mod_remove_box",
"mod_term_box",
"dev_defaults_box",
"modsel_dialog_box",
"modsel2_dialog_box",
"mod_audevents_box",
"report_dialog_box",
"audit_report_box",
"audit_chg_log_box",
"audit_status_box",
"audit_newdir_box",
"moddsel_dialog_box",
};

static int numHelpTopics = (sizeof help_topic / sizeof help_topic[0]);

void HyperHelpErrorCB();
Boolean hyperhelpInit = FALSE;
Widget  hyperhelpWidget;

void
HyperHelpInit(topLevelWidget)
Widget	topLevelWidget;

{
    DXmHelpSystemOpen(&xif_hyperhelp_context, 
 		      topLevelWidget,  
		      k_xif_hyperhelp_path, 
		      HyperHelpErrorCB, 
		      "Help Error: DXmHelpSystemOpen" );

    hyperhelpInit 	= TRUE;
    hyperhelpWidget 	= topLevelWidget;

} /* end HyperHelpInit() */


/*******************************************************************
**
**  Description: Help close (on restart or exit).
**
**  Formal Parameters
**                                    
********************************************************************/

void
HyperHelpTerm()

{
    /* Was help initialized ? */
    if ( !hyperhelpInit )
        return;

    /* Close help on all active screens. */
    DXmHelpSystemClose(xif_hyperhelp_context, 
		       HyperHelpErrorCB, 
		       "Help Error: DXmHelpSystemClose");
}




void 
HyperHelpErrorCB( string, status )
char *string;
int status;

{
    Cardinal	n;
    Arg		args[1];
    Widget	errorWidget;
    XmString	xmstring;

    xmstring = XmStringCreate(string,
			      XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNmessageString, xmstring);  n++;
    
    errorWidget = XmCreateErrorDialog(hyperhelpWidget,
				      "Help System Error",
				      args,
				      n);
    XtManageChild( errorWidget );
    XmStringFree( xmstring );
}


void
HyperHelpDisplay( topic_index )
int topic_index;
{

    if(topic_index >= 0 && topic_index < numHelpTopics)
    {
	DXmHelpSystemDisplay(xif_hyperhelp_context, 
			     k_xif_hyperhelp_path,
			     "topic", 
			     help_topic[topic_index], 
			     HyperHelpErrorCB,
			     "Help Error: DXmHelpSystemDisplay" );
    }
}

