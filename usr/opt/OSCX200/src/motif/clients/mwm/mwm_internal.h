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
/* Mwm internal includes */

/*******************************************************************/

/* Dialog ids */
#define k_mwm_cust_ws_did 0
#define k_mwm_cust_border_did ( k_mwm_cust_ws_did + 1 )
#define k_mwm_cust_icons_did ( k_mwm_cust_border_did + 1 )
#define k_mwm_cust_matte_did ( k_mwm_cust_icons_did + 1 )
#define k_mwm_cust_border_col_did ( k_mwm_cust_matte_did + 1 )
#define k_mwm_cust_icon_col_did ( k_mwm_cust_border_col_did + 1 )
#define k_mwm_cust_col_mix_did ( k_mwm_cust_icon_col_did + 1 )
#define k_mwm_cust_error_did ( k_mwm_cust_col_mix_did + 1 )
#define k_mwm_cust_apply_did ( k_mwm_cust_error_did + 1 )
#define k_mwm_help_did ( k_mwm_cust_apply_did + 1 )
#define k_mwm_cust_restart_did ( k_mwm_help_did + 1 )
#define k_mwm_max_did ( k_mwm_cust_restart_did + 1 )

#define k_mwm_last_main_did ( k_mwm_cust_icon_col_did )
#define k_mwm_last_fid -1

/* Fields */
#define k_mwm_use_icon_box_fid 0
#define k_mwm_explicit_focus_fid k_mwm_use_icon_box_fid + 1
#define k_mwm_implicit_focus_fid k_mwm_explicit_focus_fid + 1
#define k_mwm_icon_placement_fid k_mwm_implicit_focus_fid + 1
#define k_mwm_icon_top_left_fid k_mwm_icon_placement_fid + 1
#define k_mwm_icon_top_right_fid k_mwm_icon_top_left_fid + 1
#define k_mwm_icon_bot_left_fid k_mwm_icon_top_right_fid + 1
#define k_mwm_icon_bot_right_fid k_mwm_icon_bot_left_fid + 1
#define k_mwm_icon_left_top_fid k_mwm_icon_bot_right_fid + 1
#define k_mwm_icon_left_bot_fid k_mwm_icon_left_top_fid + 1
#define k_mwm_icon_right_top_fid k_mwm_icon_left_bot_fid + 1
#define k_mwm_icon_right_bot_fid k_mwm_icon_right_top_fid + 1
#define k_mwm_wmenu_dclick_fid k_mwm_icon_right_bot_fid + 1
#define k_mwm_fore_fid k_mwm_wmenu_dclick_fid + 1
#define k_mwm_back_fid k_mwm_fore_fid + 1
#define k_mwm_top_fid k_mwm_back_fid + 1
#define k_mwm_bot_fid k_mwm_top_fid + 1
#define k_mwm_active_fore_fid k_mwm_bot_fid + 1
#define k_mwm_active_back_fid k_mwm_active_fore_fid + 1
#define k_mwm_active_top_fid k_mwm_active_back_fid + 1
#define k_mwm_active_bot_fid k_mwm_active_top_fid + 1
#define k_mwm_icon_image_fore_fid k_mwm_active_bot_fid + 1
#define k_mwm_icon_image_back_fid k_mwm_icon_image_fore_fid + 1
#define k_mwm_icon_image_top_fid k_mwm_icon_image_back_fid + 1
#define k_mwm_icon_image_bot_fid k_mwm_icon_image_top_fid + 1
#define k_mwm_icon_box_auto_shade_fid k_mwm_icon_image_bot_fid + 1
#define k_mwm_icon_box_back_fid k_mwm_icon_box_auto_shade_fid + 1
#define k_mwm_icon_box_back_label_fid k_mwm_icon_box_back_fid + 1
#define k_mwm_icon_box_bot_fid k_mwm_icon_box_back_label_fid + 1
#define k_mwm_col_mix_button_fid k_mwm_icon_box_bot_fid + 1
#define k_mwm_active_icon_fid k_mwm_col_mix_button_fid + 1
#define k_mwm_icon_label_fid k_mwm_active_icon_fid + 1
#define k_mwm_icon_image_fid k_mwm_icon_label_fid + 1
#define k_mwm_min_fid k_mwm_icon_image_fid + 1
#define k_mwm_max_fid k_mwm_min_fid + 1
#define k_mwm_resize_fid k_mwm_max_fid + 1
#define k_mwm_border_fid k_mwm_resize_fid + 1
#define k_mwm_title_fid k_mwm_border_fid + 1
#define k_mwm_menu_fid k_mwm_title_fid + 1
#define k_mwm_focus_raised_fid k_mwm_menu_fid + 1
#define k_mwm_focus_start_fid k_mwm_focus_raised_fid + 1
#define k_mwm_focus_deicon_fid k_mwm_focus_start_fid + 1
#define k_mwm_focus_removed_fid k_mwm_focus_deicon_fid + 1
#define k_mwm_focus_fid k_mwm_focus_removed_fid + 1
#define k_mwm_active_sample_fid k_mwm_focus_fid + 1
#define k_mwm_inactive_sample_fid k_mwm_active_sample_fid + 1
#define k_mwm_icon_box_sample_fid k_mwm_inactive_sample_fid + 1
#define k_mwm_icon_image_sample_fid k_mwm_icon_box_sample_fid + 1
#define k_mwm_icon_size_fid k_mwm_icon_image_sample_fid + 1
#define k_mwm_icon_small_fid k_mwm_icon_size_fid + 1
#define k_mwm_icon_medium_fid k_mwm_icon_small_fid + 1
#define k_mwm_icon_large_fid k_mwm_icon_medium_fid + 1
#define k_mwm_icon_wide_fid k_mwm_icon_large_fid + 1
#define k_mwm_icon_other_fid k_mwm_icon_wide_fid + 1
#define k_mwm_icon_width_fid k_mwm_icon_other_fid + 1
#define k_mwm_icon_height_fid k_mwm_icon_width_fid + 1
#define k_mwm_icon_height_label_fid k_mwm_icon_height_fid + 1
#define k_mwm_icon_width_label_fid k_mwm_icon_height_label_fid + 1
#define k_mwm_border_size_fid k_mwm_icon_width_label_fid + 1
#define k_mwm_border_size_text_fid k_mwm_border_size_fid + 1
#define k_mwm_border_size_label_fid k_mwm_border_size_text_fid + 1
#define k_mwm_border_small_fid k_mwm_border_size_label_fid + 1
#define k_mwm_border_medium_fid k_mwm_border_small_fid + 1
#define k_mwm_border_large_fid k_mwm_border_medium_fid + 1

#define k_mwm_border_other_fid 64
#define k_mwm_cust_col_mix_label_fid k_mwm_border_other_fid + 1
#define k_mwm_inactive_auto_shade_fid k_mwm_cust_col_mix_label_fid + 1
#define k_mwm_active_auto_shade_fid k_mwm_inactive_auto_shade_fid + 1
#define k_mwm_top_label_fid k_mwm_active_auto_shade_fid + 1
#define k_mwm_bot_label_fid k_mwm_top_label_fid + 1
#define k_mwm_active_top_label_fid k_mwm_bot_label_fid + 1
#define k_mwm_active_bot_label_fid k_mwm_active_top_label_fid + 1
#define k_mwm_icon_image_top_label_fid k_mwm_active_bot_label_fid + 1
#define k_mwm_icon_image_bot_label_fid k_mwm_icon_image_top_label_fid + 1
#define k_mwm_matte_fore_fid k_mwm_icon_image_bot_label_fid + 1
#define k_mwm_matte_back_fid k_mwm_matte_fore_fid + 1
#define k_mwm_matte_top_fid k_mwm_matte_back_fid + 1
#define k_mwm_matte_bot_fid k_mwm_matte_top_fid + 1
#define k_mwm_matte_sample_fid k_mwm_matte_bot_fid + 1
#define k_mwm_matte_size_fid k_mwm_matte_sample_fid + 1
#define k_mwm_matte_size_text_fid k_mwm_matte_size_fid + 1
#define k_mwm_matte_size_label_fid k_mwm_matte_size_text_fid + 1
#define k_mwm_matte_small_fid k_mwm_matte_size_label_fid + 1
#define k_mwm_matte_medium_fid k_mwm_matte_small_fid + 1
#define k_mwm_matte_large_fid k_mwm_matte_medium_fid + 1
#define k_mwm_matte_other_fid k_mwm_matte_large_fid + 1
#define k_mwm_matte_none_fid k_mwm_matte_other_fid + 1
#define k_mwm_matte_top_label_fid k_mwm_matte_none_fid + 1
#define k_mwm_matte_bot_label_fid k_mwm_matte_top_label_fid + 1
#define k_mwm_matte_fore_label_fid k_mwm_matte_bot_label_fid + 1
#define k_mwm_matte_back_label_fid k_mwm_matte_fore_label_fid + 1
#define k_mwm_matte_auto_shade_fid k_mwm_matte_back_label_fid + 1
#define k_mwm_image_auto_shade_fid k_mwm_matte_auto_shade_fid + 1
#define k_mwm_col_mix_fid k_mwm_image_auto_shade_fid + 1
#define k_mwm_menu_col_fid k_mwm_col_mix_fid + 1
#define k_mwm_menu_col_active_fid k_mwm_menu_col_fid + 1
#define k_mwm_menu_col_inactive_fid k_mwm_menu_col_active_fid + 1   
#define k_mwm_menu_col_none_fid k_mwm_menu_col_inactive_fid + 1
#define k_mwm_force_alt_space_fid k_mwm_menu_col_none_fid + 1
#define k_mwm_ignore_mods_fid k_mwm_force_alt_space_fid + 1
#define k_mwm_move_fid k_mwm_ignore_mods_fid + 1
#define k_mwm_move_outline_fid k_mwm_move_fid + 1
#define k_mwm_move_window_fid k_mwm_move_outline_fid + 1
#define k_mwm_fb_move_size_fid k_mwm_move_window_fid + 1
#define k_mwm_fid k_mwm_fb_move_size_fid + 1

/* Yes, no */
#define k_mwm_yes_id 1
#define k_mwm_no_id 2

/* Resource names and values */
#define k_mwm_true_str "True"
#define k_mwm_false_str "False"
#define k_mwm_yes_str "Yes"
#define k_mwm_no_str "No"
#define k_mwm_1_str "1"
#define k_mwm_0_str "0"
#define k_mwm_prefix_str "Mwm*"
#define k_mwm_full_prefix_str "Mwm."

/* Maximum possible screens (for debugging only) */
#define k_mwm_max_screen 2

/* Maximum color name size */
#define k_mwm_max_color_name_size 33

#define k_mwm_max_name 32

/* Error messages */
typedef struct      
  {
    char icon_height_name[ k_mwm_max_name ];
    XmString icon_height;   
    char icon_width_name[ k_mwm_max_name ];
    XmString icon_width;   
    char border_size_name[ k_mwm_max_name ];
    XmString border_size;   
    char matte_size_name[ k_mwm_max_name ];
    XmString matte_size;
    char number_name[ k_mwm_max_name ];
    XmString number;
    char apply_name[ k_mwm_max_name ];
    XmString apply;
    char restart_name[ k_mwm_max_name ];
    XmString restart;
    char apply_title_name[ k_mwm_max_name ];
    XmString apply_title;
    char restart_title_name[ k_mwm_max_name ];
    XmString restart_title;
  } mwm_cust_error_type;

externalref mwm_cust_error_type mwm_cust_error;

/* Max unix pathname */
#define maxpath 1023
/* No value */
#define k_mwm_none -1

/* Maximum of a size string */
#define k_mwm_size_str 8
#define k_mwm_name_len 32     

/* Make the boolean values case insensitive */
#define true True
#define false False

#define k_mwm_blank_str "   "
#define k_mwm_hex_0_str "0000"
#define k_mwm_space_str " "
#define k_mwm_tab_str "	"
#define k_mwm_not_str "-"

/* Indices for color */
#define k_mwm_active_fore_x 0
#define k_mwm_active_back_x 1
#define k_mwm_active_top_x 2
#define k_mwm_active_bot_x 3
#define k_mwm_inactive_fore_x 4
#define k_mwm_inactive_back_x 5      
#define k_mwm_inactive_top_x 6 
#define k_mwm_inactive_bot_x 7 
#define k_mwm_icon_image_fore_x 8
#define k_mwm_icon_image_back_x 9
#define k_mwm_icon_image_top_x 10
#define k_mwm_icon_image_bot_x 11
#define k_mwm_icon_box_back_x 12
#define k_mwm_matte_fore_x 13
#define k_mwm_matte_back_x 14
#define k_mwm_matte_top_x 15 
#define k_mwm_matte_bot_x 16
#define k_mwm_color_max k_mwm_matte_bot_x + 1

/* Resource types */
#define k_mwm_res_boolean 1   
#define k_mwm_res_string 2
#define k_mwm_res_col 3
#define k_mwm_res_int 4
#define k_mwm_res_pixel 5
/* Allocate string */
#define k_mwm_res_alloc_string 6
#define k_mwm_subres_boolean 7
#define k_mwm_subres_int 8

/* Thickness for highlighting */
#define k_mwm_highlight_thickness 3

/* Maximum argument length */
#define k_mwm_max_str_len 31

/* Color mix widgets */
typedef struct
  {
    /* Button widget */
    Widget button_wid;
    /* Button fid */
    int fid;    
  } color_mix_type;

/* Color mix box info */
typedef struct 
  {
    /* Color index */
    int index;
    Widget button_wid;
    /* Button fid */
    int button_fid;    
    /* sample fid */
    int sample_fid;
    /* sample type */
    int sample;   
  } color_mix_box_type;

/* Color info */
typedef struct
  {
    /* Color value */
    XColor xcol;
    /* name */
    char *name;
  } mwm_col_type;

typedef struct
  {
    /* Has it been initialized ? */
    Boolean init[ k_mwm_max_did ];
    /* Has it been modified ? */
    Boolean mod[ k_mwm_max_did ];
    Boolean use_icon_box;
    Boolean implicit_focus;
    Boolean active_icon;
    Boolean icon_label;
    Boolean icon_image;
    Boolean focus_raised;
    Boolean focus_removed;
    Boolean focus_start;
    Boolean focus_deicon;
    Boolean wmenu_dclick;
    Boolean min;
    Boolean max;
    Boolean title;
    Boolean border;
    Boolean menu;
    Boolean resize;
    Boolean inactive_auto_shade;
    Boolean active_auto_shade;
    Boolean image_auto_shade;
    Boolean matte_auto_shade;                         
    Boolean icon_box_auto_shade;
    Boolean force_alt_space;
    int menu_col;
    int icon_placement; 
    int icon_size_fid;
    int icon_height;
    int icon_width;
    int border_size_fid;
    int border_size;
    int matte_size_fid;
    int matte_size;
    /* Color mix info. */
    color_mix_type (*mix)[ k_mwm_max_screen ][ k_mwm_color_max ];
    /* Current index */
    color_mix_box_type (*mix_box);
    /* Color */
    mwm_col_type col[ k_mwm_color_max ];
    /* For named colors */
    char *col_names;
    char border_size_str[ k_mwm_size_str ];
    char matte_size_str[ k_mwm_size_str ];
    char icon_height_str[ k_mwm_size_str ];
    char icon_width_str[ k_mwm_size_str ];
    /* True if dual head and different colors. */
    Boolean multicolor; 
    /* show feedback. */
    struct                          
      {
        Boolean behavior;
        Boolean kill;
        Boolean move;
        Boolean resize;
        Boolean quit;
        Boolean restart;
        Boolean placement;
      } fb;
    /* Move entire window or just outline */
    Boolean move_window;
    /* Ignore mod keys */
    Boolean ignore_mods;
  } mwm_cust_info_type;

/* mwm_cust_info is the current state, mwm_cust_info_last is the previous
state, mwm_cust_info_save is the initial state. */
externalref mwm_cust_info_type mwm_cust_info, mwm_cust_info_last,
mwm_cust_info_save;

/* If there is only 1 screen return 0; Otherwise return the active screen */
#define CURR_SCREEN   ( ( wmGD.numScreens == 1 ) ? 0 : wmGD.pActiveSD->screen) 

/*******************************************************************/
