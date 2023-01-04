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
/* Mwm color customize internals */

/*******************************************************************/

/* Strings and Message names for mix boxes */
static XmString *mwm_mix_str[ k_mwm_color_max ] =
{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static char *mwm_mix_mess[ k_mwm_color_max ] = {
    "k_mwm_active_fore_mess_text",
    "k_mwm_active_back_mess_text",
    "k_mwm_active_top_mess_text",
    "k_mwm_active_bot_mess_text",
    "k_mwm_fore_mess_text",
    "k_mwm_back_mess_text",
    "k_mwm_top_mess_text",
    "k_mwm_bot_mess_text",
    "k_mwm_image_fore_mess_text",
    "k_mwm_image_back_mess_text",
    "k_mwm_image_top_mess_text",
    "k_mwm_image_bot_mess_text",
    "k_mwm_icon_box_mess_text",
    "k_mwm_matte_fore_mess_text",
    "k_mwm_matte_back_mess_text",
    "k_mwm_matte_top_mess_text",
    "k_mwm_matte_bot_mess_text" };

#define k_mwm_window_width 100
#define k_mwm_window_height 50                          
#define k_mwm_delta 10
#define k_mwm_max_window_width k_mwm_window_width + k_mwm_delta*2
#define k_mwm_max_window_height k_mwm_window_height + k_mwm_delta*2
                       
#define k_mwm_active_sample 0
#define k_mwm_inactive_sample 1
#define k_mwm_icon_box_sample 2
#define k_mwm_icon_image_sample 3
#define k_mwm_matte_sample 4
#define k_mwm_sample 5     

static Pixmap (*mwm_col_pix)[ k_mwm_max_screen ][ k_mwm_sample ]  = NULL;
static GC (*mwm_col_gc)[ k_mwm_max_screen ][ k_mwm_sample ] = NULL;

static XPoint 
    top_points[6] = { { 0, k_mwm_window_height },
		      { 0, 0 },
		      { k_mwm_window_width, 0 },
		      { (k_mwm_window_width - k_mwm_delta), k_mwm_delta},
		      { k_mwm_delta, k_mwm_delta },
		      { k_mwm_delta, (k_mwm_window_height - k_mwm_delta) } },
    bot_points[6] =  {{ 0, k_mwm_window_height },
		      { k_mwm_delta, (k_mwm_window_height - k_mwm_delta) },
		      { (k_mwm_window_width - k_mwm_delta), (k_mwm_window_height - k_mwm_delta)}, 
		      { (k_mwm_window_width - k_mwm_delta), k_mwm_delta },
		      { k_mwm_window_width, 0},
		      { k_mwm_window_width, k_mwm_window_height } },
    full_points[4] =  { { 0, 0 },
            	        { k_mwm_window_width + k_mwm_delta*2, 0 },
            	        { k_mwm_window_width + k_mwm_delta*2, 
                          k_mwm_window_height + k_mwm_delta*2 },
        	        { 0, k_mwm_window_height + k_mwm_delta*2 } },
    back_points[4] = {{ k_mwm_delta, k_mwm_delta },
	              { (k_mwm_window_width - k_mwm_delta), k_mwm_delta},
		      { (k_mwm_window_width - k_mwm_delta), (k_mwm_window_height - k_mwm_delta)}, 
		      { k_mwm_delta, (k_mwm_window_height - k_mwm_delta) } };

/* Sample sizes are initialized at runtime */
static short mwm_col_sample_height = 0;
static short mwm_col_sample_width = 0;

/*******************************************************************/
