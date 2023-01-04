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
/************************************************************************
# Copyright 1991 OPEN SOFTWARE FOUNDATION, INC.
#
# Permission is hereby granted to use, copy, modify and freely distribute
# the software in this file and its documentation for any purpose without
# fee, provided that the above copyright notice appears in all copies and
# that both the copyright notice and this permission notice appear in
# supporting documentation.  Further, provided that the name of Open
# Software Foundation, Inc. ("OSF") not be used in advertising or
# publicity pertaining to distribution of the software without prior
# written permission from OSF.  OSF makes no representations about the
# suitability of this software for any purpose.  It is provided "as is"
# without express or implied warranty.
#
# Open Software Foundation is a trademark of The Open Software Foundation, Inc.
# OSF is a trademark of Open Software Foundation, Inc.
# OSF/Motif is a trademark of Open Software Foundation, Inc.
# Motif is a trademark of Open Software Foundation, Inc.
************************************************************************/

/***********************************/
/* gendi_c                        */
/* Author: Ken Flowers            */
/* This program builds a GENeric  */
/* DIalogbox for prompting for    */
/* the answer to simple questions.*/
/* Its first argument is used     */
/* as a label for the dialogbox.  */
/* The rest of the arguments are  */
/* used to label the buttons.     */
/***********************************/

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>

void 
buttonCB(w, client_data, call_data)
	Widget          w;
	XtPointer       client_data;
	XtPointer       call_data;
{
	exit((int) client_data);
}

Widget          application;

void 
main(argc, argv)
	unsigned int    argc;
	char          **argv;
{
	Widget          main_window;
	Widget          frame;
	Widget          form;
	Widget          rowcolumn;
	Widget          label;
	Widget          separator;
	Widget          button[9];

	XmString        cs_label;
	XmString        cs_button[9];

	Arg             args[10];
	register int    n;

	int             i;

	if (argc <= 2) exit(0); else
	  application = XtInitialize("gendi", "XMdemos", NULL, NULL, &argc, argv);

	n = 0;
	main_window = XmCreateMainWindow(application, "main_window", args, n);
	XtManageChild(main_window);

	n = 0;
	frame = XmCreateFrame(main_window, "frame", args, n);
	XtManageChild(frame);

	n = 0;
	XtSetArg(args[n], XmNrubberPositioning, True);n++;
	XtSetArg(args[n], XmNheight, 85);n++;
	form = XmCreateForm(frame, "form", args, n);
	XtManageChild(form);

	n = 0;
	XtSetArg(args[n], XmNx, 0);n++;
	XtSetArg(args[n], XmNy, 0);n++;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING);n++;
	XtSetArg(args[n], XmNhighlightThickness, 0);n++;
	XtSetArg(args[n], XmNheight, 50);n++;
	XtSetArg(args[n], XmNmarginLeft, 10);n++;
	label = XmCreateLabel(form, argv[1], args, n);
	XtManageChild(label);

	n = 0;
	XtSetArg(args[n], XmNx, 0);n++;
	XtSetArg(args[n], XmNy, 50);n++;
	separator = XmCreateSeparator(form, "separator", args, n);
	XtManageChild(separator);

	n = 0;
	XtSetArg(args[n], XmNx, 0);n++;
	XtSetArg(args[n], XmNy, 55);n++;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL);n++;
	rowcolumn = XmCreateRowColumn(form, "rowcolumn", args, n);
	XtManageChild(rowcolumn);

	for (i = 1; i < argc - 1; i++) {
	    n = 0;
	    button[i] = XmCreatePushButton(rowcolumn, argv[i + 1], args, n);
	    XtManageChild(button[i]);
	    XtAddCallback(button[i], XmNactivateCallback, buttonCB, (XtPointer) i);
	}

	XtRealizeWidget(application);
	XtMainLoop();
}
