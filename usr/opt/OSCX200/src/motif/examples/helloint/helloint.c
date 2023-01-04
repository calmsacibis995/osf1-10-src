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
static char *rcsid = "@(#)$RCSfile: helloint.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 16:53:45 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */
#include <Xm/Xm.h>                   /* Motif Toolkit */
#include <Mrm/MrmPublic.h>           /* Mrm stuff */

static MrmHierarchy	s_MrmHierarchy;		/* MRM database hierarch id */

static char *vec[]={"helloint.uid","localstrings.uid"}; 

static int vecnum = sizeof(vec) / sizeof(char*);
						
static MrmCode		class ;

static void helloworld_button_activate();

static MrmRegisterArg	regvec[] = {
	{"helloworld_button_activate",(caddr_t)helloworld_button_activate}
	};
static MrmCount regnum = sizeof(regvec) / sizeof(MRMRegisterArg);



/*
 *  Main program
 */
int main(argc, argv)
int argc;
char **argv;
{
    XtAppContext            app_context;
    Widget toplevel, helloworldmain = NULL;
    String language;

    MrmInitialize ();

    toplevel = XtAppInitialize(&app_context, "HelloI18N", NULL, 0,
			       &argc, argv, NULL, NULL, 0);

    if (MrmOpenHierarchy (vecnum, vec, NULL, &s_MrmHierarchy) != MrmSUCCESS) {
	printf ("can't open hierarchy\n");
	exit(1);
     }

    if (MrmRegisterNames (regvec, regnum)!= MrmSUCCESS) {
	printf("can't register names\n");
	exit(1);
    }

    if (MrmFetchWidget (s_MrmHierarchy, "helloworld_main", toplevel,
			&helloworldmain, &class) != MrmSUCCESS) {
	printf("can't fetch interface\n");
	exit(1);
    }

    XtManageChild(helloworldmain);
    
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}


static void helloworld_button_activate( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    Arg arglist[1];

    static int first = 1 ;

    if (first) {
	XtSetArg(arglist[0], XmNlabelString, "bye_label");
	MrmFetchSetValues(s_MrmHierarchy, widget, arglist, 1);
	first = 0 ;
    } else {
	exit(0);
    }
}
