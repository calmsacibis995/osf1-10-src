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
/* Mwm_dialog external includes */

/*******************************************************************/

    /* External variables */

#define k_mwm_dialog_resource_name "mwmDialog"
#define k_mwm_dialog_resource_class "MwmDialog"         

externalref MrmHierarchy mwm_hierarchy;
externalref Boolean mwm_hierarchy_init;
externalref Boolean mwm_cb_init;
externalref Widget *mwm_dialog_shell;
externalref Widget mwm_dialog_apply_wid;

/* Uil files */
#ifdef VMS
externalref char *mwm_uil_file[];
#else
externalref char *mwm_uil_file[];
#endif

/* Dialog information */
typedef struct
  {
    char arg[ k_mwm_max_str_len ];
    char name[ k_mwm_max_str_len ];
    Boolean up;
    int screen;
  } mwm_did_type;

/* Dialog information per screen */
typedef struct
  {
    Widget wid;
    Widget error_wid;
    Boolean up;
  } mwm_did_screen_type;

/* Dialog box screen info */
externalref mwm_did_screen_type (*mwm_screen_did)[ k_mwm_max_screen ][ k_mwm_max_did ];
/* Dialog box descriptions */
externalref mwm_did_type mwm_did[ k_mwm_max_did ];

externalref Widget (*mwm_fid)[ k_mwm_max_screen ][ k_mwm_fid ];

/* List of fields that need to be updated on a dialog box */
externalref int *mwm_fid_list[ k_mwm_max_did ];

/********************************/

    /* External declarations */

#ifdef _NO_PROTO
extern int mwm_dialog_shell_init();
extern int mwm_dialog_init();
extern int mwm_dialog_text_get();
extern int mwm_dialog_error_up();
extern void mwm_dialog_rese();
extern int mwm_dialog_up();
extern int mwm_dialog_get();
extern void mwm_dialog_cancel();
#else
extern int mwm_dialog_shell_init( int screen, int screen_num );
extern int mwm_dialog_text_get( char *source, XmString *result );
extern int mwm_dialog_error_up( Widget wid, int did, int error_fid, char *field );
extern void mwm_dialog_rese( int did, int fid_list[] );
extern int mwm_dialog_up( Widget wid, int did, Widget parent_wid );
extern int mwm_dialog_get( int screen, Widget *wid, int did, 
                           char *name, Widget parent_wid );
extern void mwm_dialog_cancel( Widget wid, int *tag, unsigned int *reason );
#endif /* _NO_PROTO */

/*******************************************************************/
