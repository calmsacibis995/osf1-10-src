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
static char	*sccsid = "@(#)$RCSfile: umount_sec.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:34:44 $";
#endif 
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
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 */


/*

 */

#include <sys/secdefines.h>
#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <stdio.h>
#include <errno.h>
#include <prot.h>
#include <string.h>

#ifdef AUX
#include <mntent.h>
#define NEW_MNTTAB	"/etc/mnttab-t"
#define OLD_MNTTAB	"/etc/mnttab-o"
#endif

#if SEC_BASE
extern priv_t *privvec();

/*
 * Check user's authorization to unmount filesystems.
 */
void
umount_checkauth()
{
	if (!authorized_user("mount"))  {
		fprintf(stderr, "%s: need mount authorization\n",
			command_name);
		exit(1);
	}
}


/*
 * Perform the unmount system call after raising the required privileges.
 */
#ifdef _OSF_SOURCE
umount_do_umount(path, flags)
	char	*path;
	int	flags;
#else
umount_do_umount(path)
	char	*path;
#endif
{
	int	ret;
	privvec_t	saveprivs;

	if (forceprivs(privvec(SEC_MOUNT,
#ifdef SEC_MAC
			       SEC_MULTILEVELDIR, 
#endif
			       -1), saveprivs))
	{
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);

#ifdef _OSF_SOURCE
	ret = umount(path, flags);
#else
#ifdef AUX
	ret = unmount(path);
#else
	ret = umount(path);
#endif
#endif

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

#ifndef _OSF_SOURCE
/*
 * allow rewriting of mount table regardless of invoker's sensitivity
 * level or discretionary identity.
 */

FILE *
umount_new_mnttab(file, mode)
	char	*file;
	char	*mode;
{
	int	ret;
	FILE	*fp;
	privvec_t	saveprivs;

	if (forceprivs(privvec(SEC_OWNER, SEC_CHOWN, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1, saveprivs))) {
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}

	ret = create_file_securely(NEW_MNTTAB, AUTH_VERBOSE,
			"create temporary file for rewrite of mount table");
	if (ret != CFS_GOOD_RETURN) {
		fprintf (stderr, "Cannot create %s\n", NEW_MNTTAB);
		exit(2);
	}
	/* ensuing setmntent should refer to OUR temporary file */
	strcpy(file, NEW_MNTTAB);
	fp = fopen(NEW_MNTTAB, mode);
	seteffprivs(saveprivs, (priv_t *) 0);
	return(fp);
}

/*
 * open the mount table for writing, regardless of invoker's sensitivity
 * level or discretionary identity.
 */

FILE *
umount_setmntent(file, mode)
	char	*file;
	char	*mode;
{
	FILE	*fp;
	privvec_t	saveprivs;

	if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1, saveprivs))) {
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}

	fp = setmntent(file, mode);
	seteffprivs(saveprivs, (priv_t *) 0);
	return(fp);
}

/* 
 * replace the old mount table with the new one just built.
 * use the rename call, if it exists on the base system.
 */
umount_rename(nfile, ofile)
	char	*nfile;
	char	*ofile;
{
	int	save_error = 0, ret = 0;
	char	*oldfile, *tempfile;
	extern int	errno;
	privvec_t	saveprivs;

	if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1, saveprivs))) {
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
	
#ifdef AUX
	ret = rename(NEW_MNTTAB, ofile);
#else
	oldfile = strdup(OLD_MNTTAB);
	tempfile = strdup(NEW_MNTTAB);
	if (!replace_file(tempfile, ofile, oldfile)) {
		errno = EACCES;
		ret = -1;
	}
#endif
	seteffprivs(saveprivs, (priv_t *) 0);
	return(ret);
}
#endif /* !_OSF_SOURCE */
#endif SEC_BASE
