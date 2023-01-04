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
/* Mwm includes for customize */

/*******************************************************************/

/* mwm_cust_info is the current state, mwm_cust_info_last is the previous
state, mwm_cust_info_save is the initial state. */
externaldef( $DATA ) mwm_cust_info_type mwm_cust_info, mwm_cust_info_save,
mwm_cust_info_last;
externaldef( $DATA ) Boolean mwm_cust_init = false;

#define k_mwm_menu_col_active 1
#define k_mwm_menu_col_inactive 2
#define k_mwm_menu_col_none 0

#define k_mwm_icon_small_size 32
#define k_mwm_icon_medium_size 50
#define k_mwm_icon_large_size 75  
#define k_mwm_icon_wide_width 125  
#define k_mwm_icon_wide_height 75  

#define k_mwm_border_small_size 5
#define k_mwm_border_medium_size 10
#define k_mwm_border_large_size 15

#define k_mwm_matte_none_size 0
#define k_mwm_matte_small_size 10
#define k_mwm_matte_medium_size 25
#define k_mwm_matte_large_size 40

/* Resource names and values */
#define k_mwm_icon_geo_str "Mwm*iconBoxGeometry"
#define k_mwm_inactive_auto_shade_str "autoShade"
#define k_mwm_matte_auto_shade_str "matteAutoShade"
#define k_mwm_active_auto_shade_str "activeAutoShade"
#define k_mwm_image_auto_shade_str "iconImageAutoShade"
#define k_mwm_menu_auto_shade_str "menu*autoShade"
#define k_mwm_pointer_str "pointer"
#define k_mwm_implicit_str k_mwm_pointer_str
#define k_mwm_explicit_str "explicit"
#define k_mwm_all_str "all"
#define k_mwm_none_str "none"
#define k_mwm_min_str "minimize"
#define k_mwm_max_str "maximize"
#define k_mwm_border_str "border"
#define k_mwm_menu_str "menu"
#define k_mwm_title_str "title"
#define k_mwm_resizeh_str "resizeh"
#define k_mwm_behavior_str "behavior"
#define k_mwm_kill_str "kill"
#define k_mwm_move_str "move"
#define k_mwm_resize_str "resize"
#define k_mwm_quit_str "quit"
#define k_mwm_placement_str "placement"
#define k_mwm_restart_str "restart"

#define k_mwm_top_left_str "left top"
#define k_mwm_top_right_str "right top"
#define k_mwm_bot_left_str "left bottom"
#define k_mwm_bot_right_str "right bottom"
#define k_mwm_left_top_str "top left"
#define k_mwm_left_bot_str "bottom left"
#define k_mwm_right_top_str "top right"
#define k_mwm_right_bot_str "bottom right"
#define k_mwm_active_icon_str "activelabel"
#define k_mwm_icon_image_str "image"    
#define k_mwm_icon_label_str "label"

#define k_mwm_vert_str "vertical"      
#define k_mwm_horiz_str "horizontal"      

/*******************************************************************/
