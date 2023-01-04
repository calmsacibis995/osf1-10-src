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
 *	@(#)$RCSfile: logvol.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:47:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 */

/*
 *   logvol.h
 *   
 *   Contents:
 *	Functions related to logical volumes (search, add to VG, etc.)
 *	See the LVM commands library module "logvol.c".
 */

#define ASK_USER	TRUE
#define DONT_ASK_USER	FALSE

int getlvnames(char *vg_path, char ***name_list, int *items_in_list);
int lvtovg(char *lv_path, char **vg_path);
int isalv(char *path);
int openvg_and_querylv(char *lvpath, char **vgpath, int *vg_fd, 
		   struct lv_querylv *querylv);
int cutdown_lv(int vg_fd, char *vgpath, unsigned short lv_minor_num,
		   char *lvpath, int op_type, int new_val,
		   int user_confirmation);
int resync_lv(int vg_fd, unsigned short lv_minor);
int fill_lv_map(int vg_fd, char *vgpath, char *lvpath, 
		   unsigned short lv_minor_num, unsigned short lv_flags, 
		   lx_descr_t **lv_map, char **pvls, int pvl_cnt,
  		   int mirr_cnt, int le_cnt);
int extend_lv_map(int vg_fd, unsigned short lv_minor, char *lvpath, 
	 	   lx_descr_t *lv_map, int old_lx_cnt, int old_mirr_cnt, 
		   int new_lx_cnt, int new_mirr_cnt);
int local_getpvmap(int vg_fd, char *vgpath, char **pvs, char *lvpath,
		   px_descr_t **pv_map, int *px_cnt, unsigned short *pv_key);
int check_lv_map(lx_descr_t *lv_map, int total_mirr, int start, int end);
