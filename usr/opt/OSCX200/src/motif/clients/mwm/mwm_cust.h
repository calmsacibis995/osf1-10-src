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
/* Mwm customize includes */

/*******************************************************************/

    /* External routines */

#ifdef _NO_PROTO
extern int mwm_cust_info_init();
extern int mwm_cust();
extern int mwm_cust_apply_up();
extern void mwm_cust_size_other();
extern void mwm_cust_size_none();
extern void mwm_cust_get();
extern void mwm_cust_info_rese();
extern void mwm_cust_rese();
extern int mwm_cust_ws_ok();
extern int mwm_cust_border_ok();
extern int mwm_cust_icons_ok();
extern int mwm_cust_border_col_ok();
extern int mwm_cust_icon_col_ok();
extern int mwm_cust_matte_ok();
#else
extern int mwm_cust_info_init();
extern int mwm_cust( String args, ClientData *pCD, Widget wid );
extern int mwm_cust_apply_up( int screen,  int restart );

extern void mwm_cust_size_other(int new_fid, 
				int other_fid, 
				int current_fid,
				Widget label_wid,
				Widget text_wid,
				int value,
				char *size);

extern void mwm_cust_size_none(Widget wid, 
			       int new_fid, 
			       int current_fid );

extern void mwm_cust_get(int did);

extern void mwm_cust_info_rese(Widget wid, int did,
			       mwm_cust_info_type *mwm_cust_info_ptr );
extern void mwm_cust_rese(Widget wid,
			  int *tag,
			  unsigned int *reason);
extern int mwm_cust_ws_ok( Widget wid );
extern int mwm_cust_border_ok( Widget wid );     
extern int mwm_cust_icons_ok( Widget wid );
extern int mwm_cust_border_col_ok( Widget wid );    
extern int mwm_cust_icon_col_ok( Widget wid );       
extern int mwm_cust_matte_ok( Widget wid );       
#endif /* _NO_PROTO */

/*******************************************************************/
