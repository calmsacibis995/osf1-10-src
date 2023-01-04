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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/demos/hellomotif.c,v 1.1.2.2 92/06/11 09:08:00 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)hellomotif.c	1.15 91/01/10";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  ALL RIGHTS RESERVED
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
#include <stdio.h>


#include <Xm/Xm.h>          /* Motif Toolkit */
#include <Mrm/MrmPublic.h>   /* Mrm */

static MrmHierarchy	s_MrmHierarchy;		/* MRM database hierarch id */
static char		*vec[]={"hellomotif.uid"};
						/* MRM database file list   */
static MrmCode		class ;

static void helloworld_button_activate();

static MrmCount		regnum = 1 ;
static MrmRegisterArg	regvec[] = {
	{"helloworld_button_activate",(caddr_t)helloworld_button_activate}
	};

Display  	*display;
XtAppContext  	app_context;

/*
 *  Main program
 */
int main(argc, argv)
int argc;
char **argv;
{
     /*
     *  Declare the variables to contain the two widget ids
     */
    Widget toplevel, helloworldmain;
    Arg arglist[1] ;
    int n;

    /*
     *  Initialize the MRM
     */



    XtToolkitInitialize();

    app_context = XtCreateApplicationContext();

    display = XtOpenDisplay(app_context, NULL, argv[0], "helloworldclass",
                            NULL, 0, &argc, argv);
    if (display == NULL) {
        fprintf(stderr, "%s:  Can't open display\n", argv[0]);
        exit(1);
    }

    MrmInitialize ();  

    n = 0;
    XtSetArg(arglist[n], XmNallowShellResize, True);  n++;
    toplevel = XtAppCreateShell(argv[0], NULL, applicationShellWidgetClass,
                              display, arglist, n);
    /*
     *  Define the Mrm.hierarchy (only 1 file)
     */
    if (MrmOpenHierarchy (1,			    /* number of files	    */
			vec, 			    /* files     	    */
			NULL,			    /* os_ext_list (null)   */
			&s_MrmHierarchy)	    /* ptr to returned id   */
			!= MrmSUCCESS) {
	printf ("can't open hierarchy\n");
     }

    /*
     * 	Register our callback routines so that the resource manager can 
     * 	resolve them at widget-creation time.
     */

    if (MrmRegisterNames (regvec, regnum)
			!= MrmSUCCESS)
			    printf("can't register names\n");

    /*
     *  Call MRM to fetch and create the pushbutton and its container
     */

    if (MrmFetchWidget (s_MrmHierarchy,
			"helloworld_main",
			toplevel,
			&helloworldmain,
			&class)
			!= MrmSUCCESS)
			    printf("can't fetch interface\n");

    /*
     *  Make the toplevel widget "manage" the main window (or whatever the
     *  the uil defines as the topmost widget).  This will
     *  cause it to be "realized" when the toplevel widget is "realized"
     */

    XtManageChild(helloworldmain);
    
    /*
     *  Realize the toplevel widget.  This will cause the entire "managed"
     *  widget hierarchy to be displayed
     */

    XtRealizeWidget(toplevel);

    /*
     *  Loop and process events
     */

    XtAppMainLoop(app_context);

    /* UNREACHABLE */
    return (0);
}

static void helloworld_button_activate( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    Arg arglist[2];

    static int call_count = 0;


    call_count += 1 ;
    switch ( call_count )
        {
        case 1:
            XtSetArg( arglist[0], XmNlabelString,
                XmStringLtoRCreate("Goodbye\nWorld!",""));
            XtSetArg( arglist[1], XmNx, 11 );  
            XtSetValues( widget, arglist, 2 );
            break ;
        case 2:
            exit(1);
            break ;
        }
}
