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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: fstab.h,v $ $Revision: 4.2.7.3 $ (OSF) $Date: 1993/12/15 22:13:30 $ */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	%W% (Berkeley) %G%
 */

#ifndef _FSTAB_H_
#define _FSTAB_H_

#include <standards.h>

/*
 * File system table, see fstab(5).
 *
 * Used by dump, mount, umount, swapon, fsck, df, ...
 *
 * The fs_spec field is the block special name.  Programs that want to use
 * the character special name must create that name by prepending a 'r'
 * after the right most slash.  Quota files are always named "quotas", so
 * if type is "rq", then use concatenation of fs_file and "quotas" to locate
 * quota file.
 */
#define	_PATH_FSTAB	"/etc/fstab"
#define	FSTAB		"/etc/fstab"	/* deprecated */

#define	FSTAB_RW	"rw"		/* read/write device */
#define	FSTAB_RQ	"rq"		/* read/write with quotas */
#define	FSTAB_RO	"ro"		/* read-only device */
#define	FSTAB_SW	"sw"		/* swap device */
#define	FSTAB_XX	"xx"		/* ignore totally */

struct fstab {
	char	*fs_spec;		/* block special device name */
	char	*fs_file;		/* file system path prefix */
	char	*fs_type;		/* FSTAB_* */
	int	fs_freq;		/* dump frequency, in days */
	int	fs_passno;		/* pass number on parallel dump */
	char	*fs_vfstype;		/* File system type, ufs, nfs */
	char	*fs_mntops;		/* Mount options ala -o */
};

_BEGIN_CPLUSPLUS
extern struct fstab *getfsent __((void));
extern struct fstab *getfsspec __((const char*));
extern struct fstab *getfsfile __((const char*));
extern int setfsent __((void));
extern void endfsent __((void));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int getfsent_r __((struct fstab *, char *, int, FILE **));
extern int getfsspec_r __((const char *, struct fstab *, char *, int, FILE **));
extern int getfsfile_r __((const char *, struct fstab *, char *, int, FILE **));
extern int setfsent_r __((FILE **));
extern void endfsent_r __((FILE **));
#endif	/* _REENTRANT || _THREAD_SAFE */
_END_CPLUSPLUS

#endif	/* _FSTAB_H_ */
