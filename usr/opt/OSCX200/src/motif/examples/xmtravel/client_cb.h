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
/*
 * @(#)$RCSfile: client_cb.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:04:11 $
 */
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */

extern void	c_create_widgets();
extern void 	client_select_activate();
extern void 	client_save_activate();
extern void 	schedule_trip_activate();
extern void	first_class_changed();
extern void	business_class_changed();
extern void	coach_changed();
extern void	non_smoking_changed();
extern void	smoking_changed();
extern void	aisle_changed();
extern void	window_changed();
extern void     none_seat_changed();
extern void	data_changed();
extern void	name_changed();
extern void	ok_response();
extern void     cancel_response();
extern void	help_response();
extern void	nomatch_response();
extern void     nomatch_bill_delete();
extern void     cancel_sb_response();
extern void	name_text_popup();
extern void     bill_client();
extern void     delete_client();
extern void	move_left();
extern void	move_right();
extern void	move_down();
extern void	move_up();
