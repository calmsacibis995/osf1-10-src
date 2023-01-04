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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: fstab.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/08/16 13:39:29 $";
#endif

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1980, 1988 Regents of the University of California.
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
 * fstab.c	5.8 (Berkeley) 8/21/89
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endfsent_r = __endfsent_r
#pragma weak getfsent_r = __getfsent_r
#pragma weak getfsfile_r = __getfsfile_r
#pragma weak getfsspec_r = __getfsspec_r
#pragma weak setfsent_r = __setfsent_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak endfsent = __endfsent
#pragma weak getfsent = __getfsent
#pragma weak getfsfile = __getfsfile
#pragma weak getfsspec = __getfsspec
#pragma weak setfsent = __setfsent
#endif
#endif
#include <stdio.h>
#include <fcntl.h>
#include <fstab.h>

#include "ts_supp.h"

#define	MAXLINELENGTH	1024

#ifdef _THREAD_SAFE

extern char	*strtok_r();

#define	FS_FP			(*fs_fp)

#define	SETFSENT()		setfsent_r(fs_fp)
#define	FSTABSCAN(f, l)		fstabscan(f, l, len, FS_FP)
#define STRTOK(s1, s2)		strtok_r(s1, s2, &tokp)

#else

extern char	*strtok();

#define	FS_FP			fs_fp

#define	SETFSENT()		setfsent()
#define	FSTABSCAN(f, l)		fstabscan(f, l, MAXLINELENGTH, FS_FP)
#define STRTOK(s1, s2)		strtok(s1, s2)

static struct fstab	fs_fsent;
static FILE		*fs_fp;
static char		line[MAXLINELENGTH];

#endif	/* _THREAD_SAFE */


static int
fstabscan(struct fstab *fsent, char *line, int len, FILE *fp)
{
	register char *cp;
	int typexx;
	char subline[MAXLINELENGTH];
#ifdef _THREAD_SAFE
	char *tokp;
#endif	/* _THREAD_SAFE */
	char *fgets();

	for (;;) {
		/*
		 * Skip lines that start with hash sign
		 */
		do{
                        cp = fgets(line, len, fp);
                        if (cp == NULL)
                                return(0);
                } while (*cp == '#');

		/* original osf code */
/*
		if (!(cp = fgets(line, len, fp)))
			return (0);
*/
		fsent->fs_spec = STRTOK(cp, " \t\n");
		fsent->fs_file = STRTOK((char *)NULL, " \t\n");
		fsent->fs_vfstype = STRTOK((char *)NULL, " \t\n");
		fsent->fs_mntops = STRTOK((char *)NULL, " \t\n");
		if (fsent->fs_mntops == NULL)
			continue;
		fsent->fs_freq = 0;
		fsent->fs_passno = 0;
		if ((cp = STRTOK((char *)NULL, " \t\n")) != NULL) {
			fsent->fs_freq = atoi(cp);
			if ((cp = STRTOK((char *)NULL, " \t\n")) != NULL)
				fsent->fs_passno = atoi(cp);
		}
#ifdef _THREAD_SAFE
		/* paranoia - check for a very long list of mntops */
		if (strlen(fsent->fs_mntops) > MAXLINELENGTH)
			continue;
#endif	/* _THREAD_SAFE */
		strcpy(subline, fsent->fs_mntops);
		for (typexx = 0, cp = STRTOK(subline, ","); cp;
		     cp = STRTOK((char *)NULL, ",")) {
			if (strlen(cp) != 2)
				continue;
			if (!strcmp(cp, FSTAB_RW)) {
				fsent->fs_type = FSTAB_RW;
				break;
			}
			if (!strcmp(cp, FSTAB_RQ)) {
				fsent->fs_type = FSTAB_RQ;
				break;
			}
			if (!strcmp(cp, FSTAB_RO)) {
				fsent->fs_type = FSTAB_RO;
				break;
			}
			if (!strcmp(cp, FSTAB_SW)) {
				fsent->fs_type = FSTAB_SW;
				break;
			}
			if (!strcmp(cp, FSTAB_XX)) {
				fsent->fs_type = FSTAB_XX;
				typexx++;
				break;
			}
		}
		if (typexx)
			continue;
		if (cp != NULL)
			return (1);
	}
	/* NOTREACHED */
}


#ifdef _THREAD_SAFE
int
getfsent_r(struct fstab *fsent, char *line, int len, FILE **fs_fp)
{
#else
struct fstab *
getfsent(void)
{
	register struct fstab	*fsent = &fs_fsent;
#endif	/* _THREAD_SAFE */

	TS_EINVAL((fsent == 0 || line == 0 || len <= 0 || fs_fp == 0));

	if (!FS_FP && SETFSENT() != TS_SUCCESS || !FSTABSCAN(fsent, line))
		return (TS_NOTFOUND);
	return (TS_FOUND(fsent));
}


#ifdef _THREAD_SAFE
int
getfsspec_r(const char *name, struct fstab *fsent,
	    char *line, int len, FILE **fs_fp)
{
#else
struct fstab *
getfsspec(register const char *name)
{
	register struct fstab	*fsent = &fs_fsent;
#endif	/* _THREAD_SAFE */

	TS_EINVAL((fsent == 0 || line == 0 || len <= 0 || fs_fp == 0));

	if (SETFSENT() == TS_SUCCESS)
		while (FSTABSCAN(fsent, line))
			if (!strcmp(fsent->fs_spec, name))
				return (TS_FOUND(fsent));
	return (TS_NOTFOUND);
}


#ifdef _THREAD_SAFE
int
getfsfile_r(const char *name, struct fstab *fsent,
	    char *line, int len, FILE **fs_fp)
{
#else
struct fstab *
getfsfile(register const char *name)
{
	register struct fstab	*fsent = &fs_fsent;
#endif	/* _THREAD_SAFE */

	TS_EINVAL((fsent == 0 || line == 0 || len <= 0 || fs_fp == 0));

	if (SETFSENT() == TS_SUCCESS)
		while (FSTABSCAN(fsent, line))
			if (!strcmp(fsent->fs_file, name))
				return (TS_FOUND(fsent));
	return (TS_NOTFOUND);
}


#ifdef _THREAD_SAFE
int
setfsent_r(FILE **fs_fp)
#else
int
setfsent(void)
#endif	/* _THREAD_SAFE */
{
	int	flags;

	TS_EINVAL((fs_fp == 0));

	if (FS_FP) {
		rewind(FS_FP);
		return (TS_SUCCESS);
	}

	if ((FS_FP = fopen(_PATH_FSTAB, "r")) != NULL) {
		flags = fcntl(fileno(FS_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(FS_FP), F_SETFD, flags) == 0)
			return (TS_SUCCESS);
		(void)fclose(FS_FP);
	}
	return (TS_FAILURE);
}


#ifdef _THREAD_SAFE
void
endfsent_r(FILE **fs_fp)
#else
void
endfsent(void)
#endif	/* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (fs_fp == 0) return;
#endif	/* _THREAD_SAFE */

	if (FS_FP) {
		(void)fclose(FS_FP);
		FS_FP = NULL;
	}
}
