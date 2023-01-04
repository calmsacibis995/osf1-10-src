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
/* Mwm Dialog internal info */

/*******************************************************************/

externaldef( $DATA ) MrmHierarchy mwm_hierarchy;
externaldef( $DATA ) Boolean mwm_hierarchy_init = false;
externaldef( $DATA ) Boolean mwm_cb_init = false;
externaldef( $DATA ) Widget *mwm_dialog_shell;
externaldef( $DATA ) XtInitProc mwm_dialog_xtinitproc = NULL;
externaldef( $DATA ) Widget mwm_dialog_apply_wid = NULL;

externaldef( $DATA ) int mwm_cust_ws_fid[] = {
  k_mwm_focus_raised_fid, k_mwm_focus_start_fid,
  k_mwm_focus_deicon_fid, k_mwm_focus_start_fid, k_mwm_focus_removed_fid,
  k_mwm_focus_fid, k_mwm_force_alt_space_fid, k_mwm_move_fid,
  k_mwm_ignore_mods_fid, k_mwm_fb_move_size_fid,
  k_mwm_last_fid };

externaldef( $DATA ) int mwm_cust_border_fid[] = {
  k_mwm_min_fid, k_mwm_max_fid, k_mwm_resize_fid, k_mwm_border_fid,
  k_mwm_title_fid, k_mwm_menu_fid, k_mwm_border_size_fid,
  k_mwm_border_size_text_fid, 
  k_mwm_wmenu_dclick_fid,
  k_mwm_last_fid };
                                
externaldef( $DATA ) int mwm_cust_border_col_fid[] = {
  k_mwm_fore_fid, k_mwm_back_fid, k_mwm_top_fid, k_mwm_bot_fid,
  k_mwm_active_fore_fid, k_mwm_active_back_fid, k_mwm_active_top_fid,
  k_mwm_active_back_fid, k_mwm_active_bot_fid, k_mwm_inactive_auto_shade_fid,
  k_mwm_inactive_sample_fid, k_mwm_active_sample_fid,
  k_mwm_active_auto_shade_fid, k_mwm_menu_col_fid, k_mwm_last_fid };

externaldef( $DATA ) int mwm_cust_border_col_res_fid[] = {
  k_mwm_fore_fid, k_mwm_back_fid, k_mwm_top_fid, k_mwm_bot_fid,
  k_mwm_active_fore_fid, k_mwm_active_back_fid, k_mwm_active_top_fid,
  k_mwm_active_back_fid, k_mwm_active_bot_fid, k_mwm_inactive_sample_fid, 
  k_mwm_active_sample_fid, k_mwm_last_fid };

externaldef( $DATA ) int mwm_cust_icons_fid[] = {
      k_mwm_active_icon_fid, k_mwm_icon_label_fid, k_mwm_icon_image_fid,
      k_mwm_use_icon_box_fid, k_mwm_icon_placement_fid, k_mwm_icon_size_fid,
      k_mwm_icon_width_fid, k_mwm_icon_height_fid, 
      k_mwm_last_fid };

externaldef( $DATA ) int mwm_cust_icon_col_fid[] =
  { k_mwm_icon_image_fore_fid, k_mwm_icon_image_back_fid,
    k_mwm_icon_image_top_fid, k_mwm_icon_image_bot_fid,
    k_mwm_icon_box_back_fid, k_mwm_icon_image_sample_fid,
    k_mwm_icon_box_auto_shade_fid,
    k_mwm_icon_box_sample_fid, k_mwm_image_auto_shade_fid, k_mwm_last_fid }; 

externaldef( $DATA ) int mwm_cust_icon_col_res_fid[] =
  { k_mwm_icon_image_fore_fid, k_mwm_icon_image_back_fid,
    k_mwm_icon_image_top_fid, k_mwm_icon_image_bot_fid,
    k_mwm_icon_box_back_fid, k_mwm_icon_image_sample_fid,
    k_mwm_icon_box_sample_fid, k_mwm_last_fid }; 

externaldef( $DATA ) int mwm_cust_matte_fid[] = {                                        
     k_mwm_matte_fore_fid, k_mwm_matte_back_fid, 
     k_mwm_matte_top_fid, k_mwm_matte_bot_fid,
     k_mwm_matte_sample_fid, k_mwm_matte_size_fid,
     k_mwm_matte_size_text_fid, /* k_mwm_matte_size_label_fid, */
     k_mwm_matte_auto_shade_fid, k_mwm_last_fid };
                       
externaldef( $DATA ) int mwm_cust_matte_res_fid[] = {
     k_mwm_matte_fore_fid, k_mwm_matte_back_fid, 
     k_mwm_matte_top_fid, k_mwm_matte_bot_fid,
     k_mwm_matte_sample_fid, k_mwm_last_fid };
                       
externaldef( $DATA ) int mwm_cust_col_mix_fid[] = {
     k_mwm_col_mix_fid, k_mwm_last_fid };
                       
/* List of fields that need to be updated on a dialog box */
externaldef( $DATA ) int *mwm_fid_list[ k_mwm_max_did ] =
{ (int *)mwm_cust_ws_fid, (int *)mwm_cust_border_fid, (int *)mwm_cust_icons_fid,
  (int *)mwm_cust_matte_fid, (int *)mwm_cust_border_col_fid, 
  (int *)mwm_cust_icon_col_fid, (int *)mwm_cust_col_mix_fid };

/* List of color fields that need to be reset for multiscreen systems. */
externaldef( $DATA ) int *mwm_fid_res_list[ k_mwm_max_did ] =
{ (int *)NULL, (int *)NULL, (int *)NULL,
  (int *)mwm_cust_matte_res_fid, (int *)mwm_cust_border_col_res_fid, 
  (int *)mwm_cust_icon_col_res_fid, (int *)NULL }; 

externaldef( $DATA ) Widget (*mwm_fid)[ k_mwm_max_screen ][ k_mwm_fid ] = NULL;

externaldef( $DATA ) mwm_cust_error_type mwm_cust_error = { 
  "k_mwm_icon_height_error", (XmString)NULL,
  "k_mwm_icon_width_error", (XmString)NULL, 
  "k_mwm_border_size_error", (XmString)NULL,
  "k_mwm_matte_size_error", (XmString)NULL,
  "k_mwm_number_error", (XmString)NULL,
  "k_mwm_cust_apply_text", (XmString)NULL,
  "k_mwm_cust_restart_text", (XmString)NULL,
  "k_mwm_cust_apply_title", (XmString)NULL,
  "k_mwm_cust_restart_title", (XmString)NULL };

/* Dialog box screen info */
externaldef( $DATA ) mwm_did_screen_type (*mwm_screen_did)[ k_mwm_max_screen ][ k_mwm_max_did ] = NULL;

/* Customize dialog box names */
#define k_mwm_cust_ws_name "mwm_cust_workspace"
#define k_mwm_cust_border_name "mwm_cust_border"    
#define k_mwm_cust_icons_name "mwm_cust_icons"   
#define k_mwm_cust_border_col_name "mwm_cust_border_color"    
#define k_mwm_cust_icon_col_name "mwm_cust_icon_color"   
#define k_mwm_cust_matte_name "mwm_cust_matte"  
#define k_mwm_cust_col_mix_name "mwm_cust_col_mix"
#define k_mwm_cust_error_name "mwm_cust_error"
#define k_mwm_cust_apply_name "mwm_cust_apply_mess"
#define k_mwm_help_name "mwm_help"
#define k_mwm_cust_restart_name "mwm_cust_restart_mess"

/* Dialog box descriptions */
externaldef( $DATA ) mwm_did_type mwm_did[ k_mwm_max_did ] =
{ { k_mwm_cust_ws_arg, k_mwm_cust_ws_name, false, k_mwm_none },
  { k_mwm_cust_border_arg, k_mwm_cust_border_name, false, k_mwm_none },
  { k_mwm_cust_icons_arg, k_mwm_cust_icons_name, false, k_mwm_none }, 
  { k_mwm_cust_matte_arg, k_mwm_cust_matte_name, false, k_mwm_none },
  { k_mwm_cust_border_col_arg, k_mwm_cust_border_col_name, false, k_mwm_none },
  { k_mwm_cust_icon_col_arg, k_mwm_cust_icon_col_name, false, k_mwm_none },
  { "", k_mwm_cust_col_mix_name, false, k_mwm_none },
  { "", k_mwm_cust_error_name, false, k_mwm_none },
  { "", k_mwm_cust_apply_name, false, k_mwm_none },
  { k_mwm_help_arg, k_mwm_help_name, false, k_mwm_none },
  { "", k_mwm_cust_restart_name, false, k_mwm_none }};
                                                             
Pixmap (*mwm_icon)[ k_mwm_max_screen ][ k_mwm_max_did ] = NULL;

/* Uil files */
#ifdef VMS
externaldef( $DATA ) char *mwm_uil_file[] = { "decw$mwm.uid" };
#else
externaldef( $DATA ) char *mwm_uil_file[] = { "DXmwm.uid" };
#endif

/*******************************************************************/
