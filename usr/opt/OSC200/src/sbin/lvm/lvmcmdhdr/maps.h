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
 *	@(#)$RCSfile: maps.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:47:53 $
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
 *   maps.h
 *   
 *   Contents:
 *	Functions to deal with the allocation maps of LV's and PV's;
 *	These functions access the device driver (via ioctl), and build
 *	a reasonable data structure for them.
 *
 *	See the LVM cmd library module maps.c for further details.
 */

/* Values for conversion functions (from path to "number") */
#define BAD_PV_KEY      ((unsigned short) -1)
#define BAD_LV_MINOR    ((unsigned short) -1)

/* Structure containing information for one copy of a LE */
typedef struct {
   unsigned short pv_key;	/* as returned by the driver */
   unsigned short px_index;	/* which PE contains the data for this LE */
   unsigned short status;	/* as returned by the driver */
   unsigned short in_use;	/* all above fields could be 0 => check this
				   to see if the data here contained is
				   meaningful */
} lx_copy_t;

/* Description of one LE: the 3 data copies of one LE */
typedef lx_copy_t lx_descr_t[LVM_MAXCOPIES];

/* Description of one PE */
typedef struct {
   unsigned short lv_min;	/* to which LV these data belong */
   unsigned short lx_index;	/* to which LE these data belong */
   unsigned short status;	/* as returned by the driver */
} px_descr_t;

int pvkeys_and_names(int vg_fd, char *vg_path, char ***pv_names,
		int *pv_cnt);
unsigned short pvpathtopvkey(char *pv_path);
char *pvkeytopvpath(unsigned short pv_key);
int getpvmap(int vg_fd, char *vg_path, unsigned short pv_key,
		px_descr_t **pv_map, int *px_cnt);

int lvminors_and_names(int vg_fd, char *vg_path, char ***lv_names,
		int *lv_cnt);
unsigned short lvpathtolvminor(char *lv_path);
char *lvminortolvpath(unsigned short lv_minor);
int getlvmap(int vg_fd, char *vg_path, unsigned short lv_minor,
		lx_descr_t **lv_map, int *maxlx_cnt, int *curlx_cnt,
		int *pxperlx_cnt);
int reduce_lv(int vg_fd, unsigned short lv_minor, lx_descr_t *lv_map, int mode,
		int old_size, int new_size, int invariant);
int extend_lv(int vg_fd, unsigned short lv_minor, lx_descr_t *lv_map, 
	        int old_lx_cnt, int old_mirr_cnt, int new_lx_cnt,
		int new_mirr_cnt);
