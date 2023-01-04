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
/* Mwm_dialog forward declarations */

/*******************************************************************/

#ifdef _NO_PROTO
int mwm_dialog_shell_init();
int mwm_dialog_xtinit();
int mwm_dialog_init();
int mwm_dialog_text_get();
int mwm_dialog_error_up();
void mwm_dialog_crea();
void mwm_dialog_field_init();
void mwm_dialog_field_crea();
void mwm_dialog_field_set();
void mwm_dialog_col_rese();
void mwm_dialog_rese();
int mwm_dialog_icon_set();
int mwm_dialog_up();
int mwm_dialog_get();
void mwm_dialog_ok();
void mwm_dialog_cancel();
#else
int mwm_dialog_xtinit( Widget req, XmDialogShellWidgetRec *new, ArgList args, 
Cardinal *num_args );
int mwm_dialog_shell_init( int screen, int screen_num );
int mwm_dialog_init();
int mwm_dialog_text_get( char *source, XmString *result );
int mwm_dialog_error_up( Widget wid, int did, int error_fid, char *field );
void mwm_dialog_crea( Widget wid, int *tag, unsigned int *reason );
void mwm_dialog_field_init( Widget wid, int fid );
void mwm_dialog_field_crea( Widget wid, int *tag, unsigned int *reason );
void mwm_dialog_field_set( Widget wid, int *tag, unsigned int *reason );
void mwm_dialog_col_rese( int screen, int fid_list[] );
void mwm_dialog_rese( int did, int fid_list[] );
int mwm_dialog_icon_set( Widget wid, int did );
int mwm_dialog_up( Widget wid, int did, Widget parent_wid );
int mwm_dialog_get( int screen, Widget *wid, int did, char *name, 
                    Widget parent_wid );
void mwm_dialog_ok( Widget wid, int *tag, unsigned int *reason );
void mwm_dialog_cancel( Widget wid, int *tag, unsigned int *reason );
#endif /* _NO_PROTO */

/*******************************************************************/
