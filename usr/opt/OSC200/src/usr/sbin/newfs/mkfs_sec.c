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
static char	*sccsid = "@(#)$RCSfile: mkfs_sec.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:23:25 $";
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
 * Copyright (c) 1988 SecureWare, Inc.  All rights reserved.
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/

/*
 * This file is part of a library to make commands more secure.
 * This file contains those routines that are added to the
 * mkfs command.
 */

#include <sys/secdefines.h>

#if SEC_FSCHANGE /*{*/

#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>

#include <sys/security.h>
#if SEC_ARCH
#include <sys/secpolicy.h>
#if SEC_MAC
#include <mandatory.h>
#endif
#if SEC_ACL
#include <acl.h>
#endif
#endif /* SEC_ARCH */

#ifdef AUX
#include <sys/time.h>
#include <sys/vnode.h>
#include <svfs/inode.h>
#include <svfs/filsys.h>
#endif

#ifdef SYSV
#include <sys/ino.h>
#ifdef SYSV_3
#include <sys/fs/s5filsys.h>
#else
#include <sys/filsys.h>
#endif
#endif /* SYSV */

#ifdef AUX
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/fs.h>
#include <sys/sysmacros.h>
#endif /* AUX */

#ifdef _OSF_SOURCE
#include <ufs/dinode.h>
#include <ufs/fs.h>
#endif /* _OSF_SOURCE */

#define SETFSTAG	"/tcb/bin/setfstag"

extern char *strrchr();
extern int errno;

/*
 * Set the type of the file system.  It will be secure -- the only
 * variant is the block size.
 */
#ifdef	AUX
void
mkfs_set_type(filsys)
	struct filsys *filsys;
{
	if (mkfs_extended_fs)
		filsys->s_type |= FsSW;
	disk_set_file_system(filsys, FsBSIZE(filsys));
}
#endif /* AUX */

#ifdef SYSV
void
mkfs_set_type(filsys, bsize)
	struct filsys *filsys;
	int bsize;
{
	filsys->s_magic = FsMAGIC;

	if (bsize == 512)
		filsys->s_type = Fs1b;
	else
		filsys->s_type = Fs2b;

	if (mkfs_extended_fs)
		filsys->s_type |= FsSW;

	disk_set_file_system(filsys, bsize);
}
#endif /* SYSV */

#ifdef _OSF_SOURCE
void
mkfs_set_type(fs, bsize)
	struct fs *fs;
	int bsize;
{
	extern int mkfs_extended_fs;

	if (mkfs_extended_fs)
		fs->fs_magic = FS_SEC_MAGIC;
	else
		fs->fs_magic = FS_MAGIC;
	disk_set_file_system(fs, bsize);
}
#endif /* _OSF_SOURCE */

#if defined(AUX) || defined(SYSV)
/*
 * Fill in the secure parts of a newly created allocated inode.
 * Those parts that are currently reserved or not used are cleared.
 */
void
mkfs_extended_inode(dp)
#ifdef AUX
	struct sec_dinode *dp;
#else
	struct dinode *dp;
#endif
{
	register int i;

	for (i=0; i < SEC_SPRIVVEC_SIZE; i++)  {
		dp->di_gpriv[i] = (priv_t) 0;
		dp->di_ppriv[i] = (priv_t) 0;
	}
	for (i=0; i < SEC_TAG_COUNT; i++)
		dp->di_tag[i] = (tag_t) 0;

	for (i=0; i < sizeof(dp->di_fill1) / sizeof(dp->di_fill1[0]); i++)
		dp->di_fill1[i] = (priv_t) 0;
	for (i=0; i < sizeof(dp->di_fill2) / sizeof(dp->di_fill2[0]); i++)
		dp->di_fill2[i] = (priv_t) 0;
	for (i=0; i < sizeof(dp->di_fill3) / sizeof(dp->di_fill3[0]); i++)
		dp->di_fill3[i] = (priv_t) 0;

	dp->di_parent = 0;
	dp->di_type_flags = (ushort) 0;
	dp->di_checksum = 0;
}
#endif /* AUX || SYSV */

#ifdef _OSF_SOURCE
/*
 * Place the inode into the proper place within the disk block and
 * fill in the extra fields.
 */

mkfs_pack_inode(fs, buf, ino, dip)
	struct fs *fs;
	char *buf;
	ino_t ino;
	struct dinode *dip;
{
	struct sec_dinode *sdip;

	disk_inode_in_block(fs, buf, &sdip, ino);

	/*
	 * fill the traditional inode
	 */

	sdip->di_node = *dip;

	if (FsSEC(fs))
		/*
		 * zero the fields in the extended part of the inode
		 */
		bzero(&sdip->di_sec, sizeof(struct dinode_sec));
}
#endif /* _OSF_SOURCE */

#ifndef SEC_STANDALONE /*{*/

/*
 * Run the setfstag program after mapping the external representations to
 * tags.  Don't run in stand-alone mode because no daemons are present.
 */

setfstag(filesysname)
	char *filesysname;
{
	int argcnt, child_pid, wait_pid;
	ushort waitstat;
	char **fstagargv;
#if SEC_ACL
	int acl_cnt;
	acle_t *acl_ir;
	tag_t acl_tag;
	extern char *acl_label;
	char acl_tag_str[11];  /* max 32 bit number is 11 digits */
	char acl_tag_index[2];
#endif
#if SEC_MAC
	mand_ir_t *mand_ir;
	tag_t mand_tag;
	extern char *mand_label;
	char mand_tag_str[11];
	char mand_tag_index[2];
#endif
#if SEC_ILB
	ilb_ir_t *ilb_ir;
	tag_t ilb_tag;
	extern char *ilb_label;
	char ilb_tag_str[11];
	char ilb_tab_index[2];
#endif
	privvec_t saveprivs;
	priv_t *privvec();

	/* Allocate the argument vector for setfstag */

	argcnt = 3;
#if SEC_ACL
	if (acl_label)
		argcnt += 4;
#endif
#if SEC_MAC
	if (mand_label)
		argcnt += 4;
#endif
#if SEC_ILB
	if (ilb_label)
		argcnt += 4;
#endif
	fstagargv = (char **) malloc(argcnt * sizeof(char *));
	if (fstagargv == (char **) 0) {
		fprintf(stderr, "No space for setfstag arguments\n");
		exit(1);
	}

	argcnt = 0;
	fstagargv[argcnt++] = strrchr(SETFSTAG, '/') + 1;

	/*
	 * Convert the security policy arguments to tag values.
	 */

#if SEC_ACL
	if (acl_label) {
		acl_ir = acl_er_to_ir(acl_label, &acl_cnt);
		if (acl_ir == (acle_t *) 0) {
			fprintf(stderr,"Can not convert '%s' to an ACL ir\n",
				acl_label);
			exit(1);
		}

		if (forceprivs(privvec(SEC_ALLOWDACACCESS, -1), saveprivs) ||
		    !acl_ir_to_tag(acl_ir, acl_cnt, &acl_tag))
		{
			fprintf(stderr,"Can not convert '%s' to an ACL tag\n",
				acl_label);
			exit(1);
		}
		seteffprivs(saveprivs, (priv_t *) 0);

		fstagargv[argcnt++] = "-i";
		sprintf(acl_tag_index, "%d", acl_config.first_obj_tag);
		fstagargv[argcnt++] = acl_tag_index;
		fstagargv[argcnt++] = "-t";
		sprintf(acl_tag_str, "%ld", acl_tag);
		fstagargv[argcnt++] = acl_tag_str;
	}
#endif /* SEC_ACL */

#if SEC_MAC
	if (mand_label) {
		mand_ir = mand_er_to_ir(mand_label);
		if (mand_ir == (mand_ir_t *) 0) {
			fprintf(stderr, "Can not convert '%s' to an SL ir\n",
				mand_label);
			exit(1);
		}

		if (forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs) ||
		    !mand_ir_to_tag(mand_ir, &mand_tag))
		{
			fprintf(stderr,"Can not convert '%s' to an SL tag\n",
				mand_label);
			exit(1);
		}
		seteffprivs(saveprivs, (priv_t *) 0);

		fstagargv[argcnt++] = "-i";
		sprintf(mand_tag_index, "%d", mand_config.first_obj_tag);
		fstagargv[argcnt++] = mand_tag_index;
		fstagargv[argcnt++] = "-t";
		sprintf(mand_tag_str, "%ld", mand_tag);
		fstagargv[argcnt++] = mand_tag_str;
	}
#endif /* SEC_MAC */

#if SEC_ILB
	if (ilb_label) {
		ilb_ir = ilb_er_to_ir(ilb_label);
		if (ilb_ir == (ilb_ir_t *) 0) {
			fprintf(stderr,"Can not convert '%s' to an IL ir\n",
				ilb_label);
			exit(1);
		}

		if (forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs) ||
		    !macilb_ir_to_tag(ilb_ir, 1, &ilb_tag))
		{
			fprintf(stderr,"Can not convert '%s' to an IL tag\n",
				ilb_label);
			exit(1);
		}
		seteffprivs(saveprivs, (priv_t) 0);

		fstagargv[argcnt++] = "-i";
		sprintf(ilb_tag_index, "%d", mand_config.first_obj_tag + 1);
		fstagargv[argcnt++] = ilb_tag_index;
		fstagargv[argcnt++] = "-t";
		sprintf(ilb_tag_str, "%ld", ilb_tag);
		fstagargv[argcnt++] = ilb_tag_str;
	}
#endif /* SEC_ILB */

	fstagargv[argcnt++] = filesysname;
	fstagargv[argcnt] = (char *) 0;

	/*
	 * Now that the filesystem has been created, invoke setfstag to
	 * initialize the tags for the specified policies.
	 */

	switch(child_pid = fork()) {

	   case -1:

		perror("Could not fork process for setfstag");
		exit(1);

	   case 0:

		/*
		 * Child process thread-exec(2) the setfstag program.
		 */

		execvp(SETFSTAG,fstagargv);

		fprintf(stderr,"Exec(2) of '%s' failed-%d\n",SETFSTAG,errno);
		exit(1);

	   default:

		/* Parent process thread-wait for the child to complete */

		if(((wait_pid = wait(&waitstat)) == -1) ||
		    (wait_pid != child_pid) || (waitstat != 0)) {
			if(waitstat)
			    fprintf(stderr,"Error status from setfstag-%d\n",
				waitstat);
			else
			    fprintf(stderr,
				"Error on wait for setfstag child-%d\n",
				errno);
			exit(1);
		}
		break;

	}
}
#endif /*} !SEC_STANDALONE */

#endif /*} SEC_FSCHANGE */
