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
/* Mwm help forward declarations */

/*******************************************************************/

#ifdef _NO_PROTO
int mwm_help_xtinit();
int mwm_help_init();
#ifdef HYPERHELP
void mwm_help_error();
void mwm_help_exit();
#endif
void mwm_help_screen_up();
void mwm_help_up();
int mwm_help();
#else
int mwm_help_xtinit( Widget req, DXmHelpShellWidgetRec *new,
ArgList args, Cardinal *num_args );
int mwm_help_init();   
#ifdef HYPERHELP
void mwm_help_error( char *string, int status );
void mwm_help_exit();
#endif
void mwm_help_screen_up( int screen, char *topic );
void mwm_help_up( Widget wid, int *tag, unsigned int *reason );
int mwm_help( String arg_list, ClientData *pCD, Widget wid );
#endif /* _NO_PROTO */

/*******************************************************************/
