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
 * @(#)$RCSfile: procfs_l.h,v $ $Revision: 1.1.11.3 $ (DEC) $Date: 1993/09/22 18:03:49 $
 */

#ifndef _SYS_PROCFS_L_H_
#define _SYS_PROCFS_L_H_

#include <procfs/procfs.h>
#include <s5fs/s5dir.h>
#include <dirent.h>

extern int nproc;

/*
 * Use the index in the procnode to verify that the proc table
 * p_pid is the same as the one in the procnode.  Compare the
 * procnode creation time, prc_time, with the start time in the
 * proc table entry, proc[index]->utask.uu_start.  If either
 * check fails, the macro evaluates to TRUE.
 */
#define PROCFS_VNINVALID(pn) ( ((pn)->prc_index == NULL) ? NULL : \
 (proc[(pn)->prc_index].p_pid != (pn)->prc_pid) || \
 (proc[(pn)->prc_index].p_stat == SZOMB) || \
 (proc[(pn)->prc_index].p_stat == SIDL) || \
 (proc[(pn)->prc_index].utask->uu_start.tv_sec != (pn)->prc_time.tv_sec) )


/*
 * Local definitions and structure needed by the procfs filesystem.
 * Definitions that are interfaces to other parts of the kenel are in a
 * different file.
 */
#define	PR_MAX_NAME_SZ	23 /* Max size of a /proc filename */
#define PR_MIN_NAME_SZ	5  /* Report /proc filenames as 5 decimal digits MIN */
#define	PR_LAG_SEC	2 /* Number of seconds allowed between dirbuf updates */

#define PR_GRP_MASK 0x0000ff00
#define PR_GRP_DEF (0x00004600)	/* get 'F' group type */

struct pr_sys5_dir {
	struct s5direct s5_dir;
};

typedef	struct s5direct	pr_s5_dirent_t;

extern	lock_t	pr_s5_dirlock;

struct pr_generic_dir {
	ino_t	d_ino;
	ushort_t	d_reclen;
	ushort_t	d_namlen;
	char	d_name[PR_MAX_NAME_SZ+1];
};

typedef	struct pr_generic_dir	pr_g_dirent_t;

extern	lock_t	pr_g_dirlock;

/* The following macros use a feature of the preprocessor, that comments use
 * no space, to get around the automatic insertion of whitespace around the
 * argument passed into a macro (another preprocessor feature).  This can be
 * replaced by the ## operator in ANSI C when ANSI C is available.
 */

#define PR_DLOCK(xxx)	simple_lock(pr_/**/xxx/**/_dirlock)
#define PR_DUNLOCK(xxx)	simple_unlock(pr_/**/xxx/**/_dirlock)

#endif
