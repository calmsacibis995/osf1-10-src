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
static char *rcsid = "@(#)$RCSfile: xmtravel.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:07:26 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */


#include "xmtravel.h"

static char	*files[] = {
	"client.uid",
	"trip.uid",
	"menu.uid"
	};

static MrmRegisterArg bound_names[] = {
  /* menu choices */
  {"view_brief_changed",	(XtPointer) view_brief_changed},
  {"view_detail_changed",	(XtPointer) view_detail_changed},
  {"view_home_changed",		(XtPointer) view_home_changed},
  {"view_business_changed",	(XtPointer) view_business_changed},
  
  /* help choices */
  {"help_on_context_activate",  (XtPointer) help_on_context_activate},
  {"help_on_window_activate",   (XtPointer) help_on_window_activate},
  {"help_on_version_activate",  (XtPointer) help_on_version_activate},
  {"help_on_help_activate",     (XtPointer) help_on_help_activate},
  {"help_on_keys_activate",     (XtPointer) help_on_keys_activate},
  {"help_on_index_activate",    (XtPointer) help_on_index_activate},
  {"help_on_index_activate",    (XtPointer) help_on_index_activate},
  {"help_tutorial_activate",    (XtPointer) help_tutorial_activate},


  /* create callbacks */
  {"c_create_widgets", 		(XtPointer) c_create_widgets},
  {"t_create_widgets", 		(XtPointer) t_create_widgets},

  /* option callbacks */
  {"option_print_activate",     (XtPointer) option_print_activate},

  /* client operations */
  {"client_select_activate", 	(XtPointer) client_select_activate},
  {"client_save_activate", 	(XtPointer) client_save_activate},
  {"client_check_iten_activate",(XtPointer) client_check_iten_activate},
  {"client_bill_activate",      (XtPointer) client_bill_activate},
  {"client_delete_activate",	(XtPointer) client_delete_activate},

  /* menu_choices */
  {"file_exit_activate",        (XtPointer) file_exit_activate},
  {"file_new_activate",         (XtPointer) file_new_activate},
  {"file_open_activate",        (XtPointer) file_open_activate},
  {"file_save_activate",        (XtPointer) file_save_activate},
  {"file_save_as_activate",     (XtPointer) file_save_as_activate},
  {"file_print_activate",       (XtPointer) file_print_activate},
  {"file_close_activate",       (XtPointer) file_close_activate},


  {"schedule_trip_activate",	(XtPointer) schedule_trip_activate},
  {"first_class_changed",      	(XtPointer) first_class_changed},
  {"business_class_changed",   	(XtPointer) business_class_changed},
  {"coach_changed",            	(XtPointer) coach_changed},
  {"non_smoking_changed",      	(XtPointer) non_smoking_changed},
  {"smoking_changed",          	(XtPointer) smoking_changed},
  {"aisle_changed",            	(XtPointer) aisle_changed},
  {"window_changed",           	(XtPointer) window_changed},
  {"none_seat_changed",        	(XtPointer) none_seat_changed},
  {"data_changed",        	(XtPointer) data_changed},
  {"name_changed",        	(XtPointer) name_changed},

  /* misc traversal routines */
  {"move_left",                 (XtPointer) move_left},
  {"move_right",                (XtPointer) move_right},
  {"move_down",                 (XtPointer) move_down},
  {"move_up",                   (XtPointer) move_up},

  /* trip operations */
  {"number_changed",		(XtPointer) number_changed},
  {"date_changed",		(XtPointer) date_changed},
  {"airlines_selected",		(XtPointer) airlines_selected},
  {"origin_selected",		(XtPointer) origin_selected},
  {"destination_selected",	(XtPointer) destination_selected},
  {"find_flights_activate",	(XtPointer) find_flights_activate},
  {"cancel_activate",		(XtPointer) cancel_activate}
};

static XtActionsRec actionsList[]=
{
  {"name_text_popup", (XtActionProc) name_text_popup},
};


TravelDataRec	globalData;

Widget          l_widget_array [L_MAX_WIDGETS];



/* Main */

int main (argc, argv)
int 	argc;
char 		*argv[];

{
  Widget 	toplevel, c_main_window = NULL;
  Arg		args[10];
  int		n;
  XtAppContext	app_context;
  MrmHierarchy	hierarchy;
  MrmType       class;

  MrmInitialize();

  n = 0;
  XtSetArg (args[n], XmNtitle, "xmtravel: Client Data"); n++;
  toplevel = XtAppInitialize (&app_context, "XMtravel", NULL, 0,
			      &argc, argv, NULL, args, n);

  XtAppAddActions(app_context, actionsList, XtNumber(actionsList));
  
  if ( MrmOpenHierarchy (XtNumber (files), files, NULL, 
			 &hierarchy) != MrmSUCCESS) {
    printf ("Can't open hierarchy\n");
    exit (1);
  }

  if ( MrmRegisterNames (bound_names, XtNumber (bound_names)) != MrmSUCCESS ) {
    printf ("Can't register names\n");
    exit (1);
  }

  if ( MrmFetchWidget (hierarchy, "client_main_window", toplevel, 
		       &c_main_window, &class) != MrmSUCCESS ) {
    printf ("Can't fetch %s\n", "client_main_window");
    exit (1);
  }

  /* Set global data */

  globalData.c_toplevel = toplevel;
  globalData.app_context = app_context;
  globalData.hierarchy = hierarchy;
  globalData.changed = False;
  globalData.name_entered = False;
  globalData.t_toplevel = NULL;

  XtManageChild (c_main_window);

  XtRealizeWidget (toplevel);
  XtAppMainLoop (app_context);

  MrmCloseHierarchy (hierarchy);
}
