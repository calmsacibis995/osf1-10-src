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
 * @(#)$RCSfile: mman.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/27 15:50:58 $
 */
/*
 *  Module Name:
 *	mman.h
 *
 *  Description:
 *	header file for SVR4 memcntl() system call
 */

#ifndef _SYS_SVR4_MMAN_H
#define _SYS_SVR4_MMAN_H

/* 
 * page lock status  structure is used to save status of
 * the pages locked or unlocked, in a given address space .
 */

struct pg_lk_stat {
	struct pg_lk_stat *st_next;	/* list of next set of pages */
	int st_size;			/* Size of this list 	  */
	char st_flag[1];		/* Status, if locked/unlocked */
};

struct pg_lk_stat * save_pgstat();
void  undo_lockop();

extern vm_size_t u_seg_mask;
#define trunc_seg(VA)   ((VA) & ~u_seg_mask)

/* The st_flag takes one of the following values */

#define LOCKED  1
#define UNLOCKED  2

#endif	/* _SYS_SVR4_MMAN_H */
