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
 * @(#)$RCSfile: mntent.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/26 17:47:13 $
 */

/*
 *      Sun style. Needed for ONC's rquotad.
 *	@(#)mntent.h	1.5 90/07/23 4.1NFSSRC SMI;	from SMI 1.13 99/01/06	*/

/*
 * File system table, see mntent (5)
 *
 * Used by dump, mount, umount, swapon, fsck, df, ...
 *
 * Quota files are always named "quotas", so if type is "rq",
 * then use concatenation of mnt_dir and "quotas" to locate
 * quota file.
 */

#ifndef _mntent_h
#define _mntent_h

#define	MNTTAB		"/etc/fstab"
#define	MOUNTED		"/etc/mtab"

#define	MNTMAXSTR	128

#define	MNTTYPE_43	"4.3"	/* 4.3 file system */
#define	MNTTYPE_42	"4.2"	/* 4.2 file system */
#define	MNTTYPE_NFS	"nfs"	/* network file system */
#define	MNTTYPE_PC	"pc"	/* IBM PC (MSDOS) file system */
#define	MNTTYPE_SWAP	"swap"	/* swap file system */
#define	MNTTYPE_IGNORE	"ignore"/* No type specified, ignore this entry */
#define MNTTYPE_LO      "lo"    /* Loop back File system */

/*  mount options  */
#define	MNTOPT_RO	"ro"	/* read only */
#define	MNTOPT_RW	"rw"	/* read/write */
#define MNTOPT_GRPID 	"grpid"	/* SysV-compatible group-id on create */
#define MNTOPT_REMOUNT	"remount"/* change options on previous mount */
#define	MNTOPT_NOAUTO	"noauto"/* hide entry from mount -a */
#define MNTOPT_NOSUB	"nosub"  /* disallow mounts beneath this one */

/*  4.2 specific options  */
#define	MNTOPT_QUOTA	"quota"	/* quotas */
#define	MNTOPT_NOQUOTA	"noquota"/* no quotas */

/*  NFS specific options  */
#define	MNTOPT_SOFT	"soft"	/* soft mount */
#define	MNTOPT_HARD	"hard"	/* hard mount (default) */
#define	MNTOPT_NOSUID	"nosuid"/* no set uid allowed */
#define	MNTOPT_INTR	"intr"	/* allow interrupts on hard mount */
#define MNTOPT_SECURE 	"secure"/* use secure RPC for NFS */
#define MNTOPT_NOAC 	"noac"	/* don't cache file attributes */
#define MNTOPT_NOCTO 	"nocto"	/* no "close to open" attr consistency */
#define MNTOPT_PORT	"port"	/* server IP port number */
#define MNTOPT_RETRANS 	"retrans" /* set number of request retries */
#define MNTOPT_RSIZE 	"rsize" /* set read size (bytes) */
#define MNTOPT_WSIZE 	"wsize" /* set write size (bytes) */
#define MNTOPT_TIMEO 	"timeo"	/* set initial timeout (1/10 sec) */
#define MNTOPT_ACTIMEO 	"actimeo" /* attr cache timeout (sec) */
#define MNTOPT_ACREGMIN "acregmin" /* min ac timeout for reg files (sec) */
#define MNTOPT_ACREGMAX "acregmax" /* max ac timeout for reg files (sec) */
#define MNTOPT_ACDIRMIN "acdirmin" /* min ac timeout for dirs (sec) */
#define MNTOPT_ACDIRMAX "acdirmax" /* max ac timeout for dirs (sec) */
#define	MNTOPT_POSIX	"posix"	/* ask for static pathconf values from mountd */

/* Information about the mount entry */
#define MNTINFO_DEV	"dev"	/* device number of the mounted file system */

struct	mntent {
	char	*mnt_fsname;		/* name of mounted file system */
	char	*mnt_dir;		/* file system path prefix */
	char	*mnt_type;		/* MNTTYPE_* */
	char	*mnt_opts;		/* MNTOPT* */
	int	mnt_freq;		/* dump frequency, in days */
	int	mnt_passno;		/* pass number on parallel fsck */
};

struct	mntent *getmntent();
char	*hasmntopt();
FILE	*setmntent();
int	endmntent();

#endif /*!_mntent_h*/






