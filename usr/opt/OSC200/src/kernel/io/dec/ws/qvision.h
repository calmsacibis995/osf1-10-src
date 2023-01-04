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
 * @(#)$RCSfile: qvision.h,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/11/17 17:28:01 $
 */

#ifndef _QVISION_H_
#define _QVISION_H_

#ifdef KERNEL
/* screen functions */
extern caddr_t	qvision_screen_init_closure();
extern int	qvision_init_screen();
extern int	qvision_clear_screen();
extern int	qvision_scroll_screen();
extern int	qvision_blitc();
extern int	qvision_map_unmap_screen();
extern void	qvision_close();

/* cursor functions */
extern int	qvision_load_cursor();
extern int	qvision_recolor_cursor();
extern int	qvision_set_cursor_position();
extern int	qvision_cursor_on_off();

/* colormap functions */
extern int	qvision_init_color_map();
extern int	qvision_load_color_map_entry();
extern void	qvision_clean_color_map();
extern int	qvision_video_on();
extern int	qvision_video_off();

/* adaptor functions */
extern int  qvision_probe();
extern int  qvision_attach();
extern void qvision_save();
extern void qvision_restore();
extern void qvision_init();
extern void qvision_adjust();
extern void qvision_print();
extern void qvision_interrupt();
extern void qvision_bot();
#endif /* KERNEL */

#endif /* _QVISION_H_ */
