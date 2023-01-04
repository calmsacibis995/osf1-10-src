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
 *	@(#)$RCSfile: dump.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:47:34 $
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
 *   dump.h
 *   
 *   Contents:
 *	All the functions to deal with dumping (i.e., visualizing information)
 *	about volume groups, logical volumes and physical volumes.
 *	See the LVM commands library module "dump.c".
 *
 */

/* Values for the mode argument */
#define TERSE		1
#define VERBOSE		2

int dump_vg(char *vg_path, int mode);
void dump_vg_title();
int dump_lv(char *lv_path, int mode);
void dump_lv_title();
int dump_pv(char *pv_path, int mode);
void dump_pv_title();
