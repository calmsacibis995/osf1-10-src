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
static char *rcsid = "@(#)$RCSfile: menu_cb.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:05:13 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */

#include <Xm/Text.h>

#include "xmtravel.h"



/* File exit button callback */

/*ARGSUSED*/
void file_exit_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  int	answer;

  /* Post a warning dialog if changes need to be saved */

  if ( globalData.changed && globalData.name_entered ) {
    answer = RET_NONE;
    answer = SaveWarning (widget, "save_warning_file_exit", 
			  RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_OK_BUTTON);

    if ( answer == RET_CANCEL ) {
      return;
    }
  }

  exit(0);
}


/* View data format brief button callback - doesn't do anything but */
/* make the toggle buttons work like a radio box                    */

/*ARGSUSED*/
void view_brief_changed (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{
  XmToggleButtonSetState (l_widget_array[menu_c_detail], False, False);
}


/* View data format detailed button callback - doesn't do anything but */
/* make the toggle buttons work like a radio box                       */

/*ARGSUSED*/
void view_detail_changed (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{
  XmToggleButtonSetState (l_widget_array[menu_c_brief], False, False);
}


/* View address type home button callback - doesn't do anything but */
/* make the toggle buttons work like a radio box                    */

/*ARGSUSED*/
void view_home_changed (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{
  XmToggleButtonSetState (l_widget_array[menu_c_business], False, False);
}


/* View address type business button callback - doesn't do anything but */
/* make the toggle buttons work like a radio box                        */

/*ARGSUSED*/
void view_business_changed (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{
  XmToggleButtonSetState (l_widget_array[menu_c_home], False, False);
}


/*ARGSUSED*/
void help_on_context_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{

       Information(widget, "info_help_on_context" ,RET_OK,XmDIALOG_OK_BUTTON);

}   


/*ARGSUSED*/
void help_on_window_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{

       Information(widget, "info_help_on_window" ,RET_OK,XmDIALOG_OK_BUTTON);

}   


/*ARGSUSED*/
void help_on_version_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{

       Information(widget, "info_help_on_version" ,RET_OK,XmDIALOG_OK_BUTTON);


}   


/*ARGSUSED*/
void help_on_help_activate (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

       Information(widget, "info_help_on_help" ,RET_OK,XmDIALOG_OK_BUTTON);

}


/*ARGSUSED*/
void help_on_keys_activate (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

       Information(widget, "info_help_on_keys" ,RET_OK,XmDIALOG_OK_BUTTON);

}


/*ARGSUSED*/
void help_on_index_activate (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

       Information(widget, "info_help_on_index" ,RET_OK,XmDIALOG_OK_BUTTON);

}


/*ARGSUSED*/
void help_tutorial_activate (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

       Information(widget, "info_help_tutorial" ,RET_OK,XmDIALOG_OK_BUTTON);

}


/*ARGSUSED*/
void file_new_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

  int answer ;

  answer = RET_NONE;

  answer = FileSelectionDialog (widget, "fsd_file_new"
                          ,RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_OK_BUTTON);
  if ( answer == RET_CANCEL )
      return;

  if ( answer == RET_HELP ) { 
      Information(widget, "info_file_new" ,RET_OK,XmDIALOG_OK_BUTTON);
            return;
  }

  Information(widget, "info_file_new2" ,RET_OK,XmDIALOG_OK_BUTTON);

}

/*ARGSUSED*/
void file_open_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

  int answer ;

  answer = RET_NONE;

  answer = FileSelectionDialog (widget, "fsd_file_open"
                          ,RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_OK_BUTTON);
  if ( answer == RET_CANCEL )
      return;

  if ( answer == RET_HELP ) { 
      Information(widget, "info_file_open_help" ,RET_OK,XmDIALOG_OK_BUTTON);
            return;
  }

  Information(widget, "info_file_open" ,RET_OK,XmDIALOG_OK_BUTTON);

}

/*ARGSUSED*/
void file_save_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

  int answer ;

  answer = RET_NONE;

  answer = Question(widget,"ques_file_save" ,RET_OK | RET_CANCEL | RET_HELP,
                    XmDIALOG_CANCEL_BUTTON);

  if ( answer == RET_CANCEL )
      return;

  if ( answer == RET_HELP ) {
      Information(widget, "info_file_save_help" ,RET_OK,XmDIALOG_OK_BUTTON);
            return;
  }

  Information(widget, "info_file_save" ,RET_OK,XmDIALOG_OK_BUTTON);


}

/*ARGSUSED*/
void file_save_as_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

  int answer ;

  answer = RET_NONE;

  answer = FileSelectionDialog (widget, "fsd_file_save_as"
                          ,RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_OK_BUTTON);
  if ( answer == RET_CANCEL )
      return;

  if ( answer == RET_HELP ) { 
      Information(widget, "info_file_save_as_help" ,RET_OK,XmDIALOG_OK_BUTTON);
            return;
  }

  Information(widget, "info_file_save_as",RET_OK,XmDIALOG_OK_BUTTON);

}

/*ARGSUSED*/
void file_print_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

       Information(widget, "info_file_print" ,RET_OK,XmDIALOG_OK_BUTTON);

}

/*ARGSUSED*/
void file_close_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

       Information(widget, "info_file_close",RET_OK,XmDIALOG_OK_BUTTON);

}

/*ARGSUSED*/
void option_print_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{


    PromptDialog(widget, "pd_option_print",
              RET_OK | RET_HELP | RET_CANCEL, XmDIALOG_OK_BUTTON) ;

}

/*ARGSUSED*/
void client_check_iten_activate(widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

     Information(widget, "info_client_check_iten"
     , RET_OK , XmDIALOG_OK_BUTTON) ;
}


