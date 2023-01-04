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
/*
 * @(#)$RCSfile: fileview.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 19:38:38 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: fileview.h,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 19:38:38 $ */

#include <stdlib.h>
#include <stdio.h>
#if defined(VMS) && defined(VAXC)
#include <X11/Xlocale.h>
#else
#include <locale.h>
#endif

#if (! defined X_H)
#include <X11/X.h>
#endif

#if (! defined _XLIB_H)
#include <X11/Xlib.h>
#endif

#if (! defined _Xm_h)
#include <Xm/Xm.h>
#endif

#if (defined DECLAREGLOBAL)
#define global 
#else
#define global extern
#endif

/*
 * The "more" program Motif-ied
 * 
 *	Allow a user to view files in separate windows.
 * 	There is one top level shell created per file opened.
 * 	Each top level shell is parent of a Main Window, the work area
 * 	of which is a PanedWindow.
 * 
 *
 * The File menu contains commands Open, and Exit or Close
 * In the primary top level shell, Exit exits the client
 *  In the secondaries, Close closes the shell
 *
 *  Open
 * 	On selecting Open/File in the menu bar, a file selection box
 *	is mapped to choose the file.
 *
 *	If OpenFile is successful, close first the current file opened, 
 *      destroy all existing panes, and display the new file.
 *
 * 	Close: this entry only exists on the secondary windows.
 *	It destroys the top level shell and closes the file.
 *	
 *	Exit: this entry only exists on the primary window
 *	 closes the file and exits.
 *
 * The View menu:
 *  	
 *	New Pane: Creates a new pane in the paned window
 *	Delete Pane: delete the current pane.
 *	Search: pops a dialogue box for searching text in the file
 *	On the OK callback of the dialogue box
 *		search the string.
 *		if found show in current pane
 *		else pop up dialogue box again.
 *
 */

/*
 * Implementation notes.
 *
 * There is a data structure of type View that encapsulates
 * all information relative to each top level shell in the application
 * A View contains: the shell id, the panedwindow id, the file descriptor,
 * the number of panes, the file selection box id, 
 * 
 * This data structure is allocated at shell creation time
 * and passed around as client data to the callbacks or as the userdata
 * of the widgets.
 * 
 * For each pane in the paned window, there is one Pane data structure.
 * THIS PROGRAM IS NOT REALLY INTERNATONALIZED.
 *  LABELS ARE IN ENNGLISH
 */


typedef struct _Pane {
   Widget text;
   struct _Pane * next;
   struct _Pane * previous;
} Pane, *PanePtr;

typedef struct {
	Widget shell;
	Widget path;
	Widget paned_window;
	Widget fsb;
	Widget view_cascade;
	Widget search_entry;
	int n_panes;
	PanePtr panes;
	PanePtr current_pane;
	Widget text_source;
	Widget search_box;
	Widget warning_box;
	Widget error_box;
	Widget direction;
	Dimension text_height;
	} View, *ViewPtr;

/*
 * Global variables, initialized in main.c
 */

global XtAppContext theContext; 
global Display * theDisplay;
global Widget theWidgetRoot;
global Widget thePrimaryShell;

/* Public Widget names, initialized in main.c  */

global String new_pane ;
global String kill_pane ;
global String search ;
global String help ;
