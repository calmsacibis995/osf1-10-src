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
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>

#define	STRING	"Hello,  World"

Arg wargs[] = {
    {XtNlabel,	(XtArgVal) STRING},
};

main(argc, argv)
    int argc;
    char **argv;
{
    Widget      toplevel, label;

    /*
     * Create the Widget that represents the window.
     * See Section 14 of the Toolkit manual.
     */
    toplevel = XtInitialize(argv[0], "XLabel", NULL, 0, &argc, argv);

    /*
     * Create a Widget to display the string,  using wargs to set
     * the string as its value.  See Section 9.1.
     */
    label = XtCreateWidget(argv[0], labelWidgetClass,
			   toplevel, wargs, XtNumber(wargs));

    /*
     * Tell the toplevel widget to display the label.  See Section 13.5.2.
     */
    XtManageChild(label);

    /*
     * Create the windows,  and set their attributes according
     * to the Widget data.  See Section 9.2.
     */
    XtRealizeWidget(toplevel);

    /*
     * Now process the events.  See Section 16.6.2.
     */
    XtMainLoop();
}
