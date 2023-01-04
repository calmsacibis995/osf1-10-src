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
 *	@(#)$RCSfile: lvmtab.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/22 16:07:32 $
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
 *   lvmtab.h
 *   
 *   Contents:
 *	All the functions to deal with the "lvmtab" file, under "/etc/lvm".
 *	See the LVM commands library module "lvmtab.c".
 */

#define LVMTABPATH	"/etc/lvmtab"
#define SCRATCHLVMTABPATH	"/etc/lvmtab.tmp"
#define LVMTABPERMISS	0644			/* permission bits */

/* Flags for "must_write" */
#define DOWRITE		TRUE
#define NOWRITE		FALSE

int lvmtab_read();
int lvmtab_write();
int lvmtab_getvgnames(char ***name_list, int *items_in_list);
int lvmtab_getpvnames(char *vg_path, char ***name_list, int *items_in_list);
int lvmtab_isvgdefined(char *vg_path);
int lvmtab_getvgid(char *vg_path, lv_uniqueID_t *vg_id);
int lvmtab_addvg(char *vg_path, lv_uniqueID_t *vg_id, int must_write);
int lvmtab_removevg(char *vg_path, int must_write);
int lvmtab_ispvinvg(char *vg_path, char *pv_path);
int lvmtab_ispvinsomevg(char *pv_path, char **vg_path);
int lvmtab_addpvtovg(char *vg_path, char *pv_path, int must_write);
int lvmtab_removepvfromvg(char *vg_path, char *pv_path, int must_write);
