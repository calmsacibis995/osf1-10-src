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
/* Mwm customize forward declarations */

/*******************************************************************/

#ifdef _NO_PROTO
int mwm_cust_info_init();
int mwm_cust_box_geo_get();
int mwm_cust_icons_get();
int mwm_cust_icon_col_get();
int mwm_cust_ws_get();
int mwm_cust_border_col_get();
int mwm_cust_border_get();
void mwm_cust_get();
void mwm_cust_size_other();
void mwm_cust_size_none();
int mwm_cust_ws_ok();
int mwm_cust_border_ok();
int mwm_cust_icons_ok();
int mwm_cust_border_col_ok();
int mwm_cust_icon_col_ok();
int mwm_cust();
void mwm_cust_ws_save();
void mwm_cust_icon_col_save();
void mwm_cust_border_col_save();
void mwm_cust_menu_col_save();
void mwm_cust_icons_save();
void mwm_cust_border_save();
void mwm_cust_matte_save();
void mwm_cust_matte_col_save();
void mwm_cust_apply();
void mwm_cust_apply_all();
int mwm_cust_apply_up();
void mwm_cust_col_copy();
void mwm_cust_ws_copy();
void mwm_cust_border_copy();
void mwm_cust_border_col_copy();
void mwm_cust_icons_copy();
void mwm_cust_icon_col_copy();
void mwm_cust_matte_copy();
void mwm_cust_all_rese();
void mwm_cust_info_rese();
void mwm_cust_rese();
void mwm_cust_all_defa();
void mwm_cust_defa();
#else
int mwm_cust_info_init();
int mwm_cust_box_geo_get( char *string, char *orient );
int mwm_cust_icons_get( XrmDatabase database );
int mwm_cust_icon_col_get( XrmDatabase database );
int mwm_cust_ws_get( XrmDatabase database );
int mwm_cust_border_col_get( XrmDatabase database );
int mwm_cust_border_get( XrmDatabase database );
void mwm_cust_get( int did );
void mwm_cust_size_other( int new_fid, int other_fid, int current_fid,
                          Widget label_wid, Widget text_wid, int value,
                          char *size );
void mwm_cust_size_none( Widget wid, int new_fid, int current_fid );
int mwm_cust_ws_ok( Widget wid );
int mwm_cust_border_ok( Widget wid );     
int mwm_cust_icons_ok( Widget wid );
int mwm_cust_border_col_ok( Widget wid );    
int mwm_cust_icon_col_ok( Widget wid );       
int mwm_cust( String args, ClientData *pCD, Widget wid );
void mwm_cust_ws_save( XrmDatabase *database );
void mwm_cust_icon_col_save( XrmDatabase *database );
void mwm_cust_border_col_save( XrmDatabase *database );
void mwm_cust_menu_col_save( XrmDatabase *database );
void mwm_cust_icons_save( XrmDatabase *database );
void mwm_cust_border_save( XrmDatabase *database );
void mwm_cust_matte_save( XrmDatabase *database );
void mwm_cust_matte_col_save( XrmDatabase *database );
void mwm_cust_apply_all( int screen, Boolean apply );
void mwm_cust_apply( Widget wid, int *tag, unsigned int *reason );
int mwm_cust_apply_up( int screen, int screen_num, int restart );
void mwm_cust_col_copy( mwm_cust_info_type *info, 
                        mwm_cust_info_type *info_save, int index );
void mwm_cust_ws_copy( mwm_cust_info_type *info, 
                       mwm_cust_info_type *info_save );
void mwm_cust_border_copy( mwm_cust_info_type *info, 
                           mwm_cust_info_type *info_save );
void mwm_cust_border_col_copy( mwm_cust_info_type *info, 
                               mwm_cust_info_type *info_save );
void mwm_cust_icons_copy( mwm_cust_info_type *info, 
                          mwm_cust_info_type *info_save );
void mwm_cust_icon_col_copy( mwm_cust_info_type *info, 
                             mwm_cust_info_type *info_save );
void mwm_cust_matte_copy( mwm_cust_info_type *info, 
                          mwm_cust_info_type *info_save );
void mwm_cust_all_rese( Widget wid );         
void mwm_cust_info_rese( Widget wid, int did, mwm_cust_info_type *mwm_cust_info_ptr );
void mwm_cust_rese( Widget wid, int *tag, unsigned int *reason );
void mwm_cust_all_defa( Widget wid );
void mwm_cust_defa( Widget wid, int *tag, unsigned int *reason );
#endif /* _NO_PROTO */

/*******************************************************************/
