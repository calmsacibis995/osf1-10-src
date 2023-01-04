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
static char	*sccsid = "@(#)$RCSfile: setfstag.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 15:18:02 $";
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
 * Copyright (c) 1988, 1989 SecureWare, Inc.  All Rights Reserved.
 */



/*
 * This program updates all the given security policy tags in a file
 * system to a provided value.  This is used to initialize a new file
 * system to a tag value from a security policy.  An example use is
 * to quickly initialize all files to the mandatory level syslo.
 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "setfstag_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SETFSTAG,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_ARCH /* { */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#ifdef SYSV_3
#include <sys/fs/s5param.h>
#include <sys/fs/s5macros.h>
#include <sys/fs/s5inode.h>
#endif
#include <sys/file.h>
#include <sys/param.h>

#ifdef AUX
#include <sys/time.h>
#include <sys/vnode.h>
#include <svfs/inode.h>
#include <svfs/filsys.h>
#include <sys/mount.h>
#endif

#ifdef SYSV
#ifdef SYSV_3
#include <sys/fs/s5filsys.h>
#else
#include <sys/filsys.h>
#endif
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/ustat.h>
#endif

#ifdef _OSF_SOURCE
#include <ufs/fs.h>
#include <ufs/inode.h>
#include <ufs/dinode.h>
#include <sys/mount.h>
#endif

#include <sys/security.h>
#include <sys/secpolicy.h>
#include <prot.h>


/*
 * On sequential printing of inode numbers
 * group this many blocks for printing to save time.
 */
#define	MOD			50

/*
 * Used to find the byte offset for the beginning of block number bn
 */

#if defined(_OSF_SOURCE)
static struct fs f;
#else
static struct filsys f;
#define	fs_offset(inum)		(bsize * bn)
#endif

static int bsize;
static int imax;
static int inopb;
static int file_sys;
static int verbose = 0;
#if defined(SEC_STANDALONE) || defined(STANDALONE)
static char *command_name;
#endif

extern char *optarg;	/* arg pointer for getopt */
extern int optind;	/* option index for getopt */
extern int opterr;

extern priv_t *privvec();

static void validate_file_system();
#ifdef SYSV
static void invalidate_superblock();
#endif
static void traverse_file_system();
static tag_t number();
static void printnum();
static void leave();

tag_t tag_value[SEC_NUM_TAGS];
short tag_valid[SEC_NUM_TAGS];

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int i;
	char *file_name = (char *) 0;
	int option;
	int error = 0;
	int all = 0;
	int index = 0;
	int tag = 0;
	int index_val;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_SETFSTAG,NL_CAT_LOCALE);
#endif

#if defined(SEC_STANDALONE) || defined(STANDALONE)
	command_name = argv[0];
#else
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin"))
		leave(MSGSTR(SETFSTAG_1,
			"%s: need sysadmin authorization\n"), command_name);

	if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(SETFSTAG_33,
			"%s: insufficient privileges\n"), command_name);
		exit(1);
	}
#endif /* SEC_STANDALONE || STANDALONE */

	opterr = 0;
	while ((option = getopt(argc, argv, "ai:t:v")) != EOF)
		switch (option)  {
		case 'a':
			all++;
			break;

		case 'i':
			index++;
			tag = 0;
			index_val = number(optarg);
			if (index_val < 0 || index_val >= SEC_NUM_TAGS)
				leave(MSGSTR(SETFSTAG_2, "%s: index value out of range (%d:%d)\n"),
					command_name, 0, SEC_NUM_TAGS - 1);
			break;

		case 't':
			tag++;
			if (index == 0)
				leave(MSGSTR(SETFSTAG_3, "%s: -i must precede -t\n"), command_name);
			if (tag_valid[index_val])
				leave(MSGSTR(SETFSTAG_4, "%s: multiple values for tag #%d\n"),
					command_name, index_val);
			tag_valid[index_val] = 1;
			tag_value[index_val] = number(optarg);
			break;

		case 'v':
			verbose++;
			break;

		case '?':
			error = 1;
			break;
	}

	if (error || !index || !tag || (argc != optind + 1)) {
		fprintf(stderr, MSGSTR(SETFSTAG_5, "usage:  %s [-v] [-a] {-i# -t#} ... device\n"),
			command_name);
		fprintf(stderr, MSGSTR(SETFSTAG_6, "Where -v is verbose\n"));
		fprintf(stderr,
	MSGSTR(SETFSTAG_7, "      -a updates all inodes (default updates 0-value tags only)\n"));
		fprintf(stderr, MSGSTR(SETFSTAG_8, "      -i notes tag index to use (from 0)\n"));
		fprintf(stderr, MSGSTR(SETFSTAG_9, "      -t is tag value to set\n"));
		fprintf(stderr, MSGSTR(SETFSTAG_10, "      device is the file system to update\n"));
		exit(1);
	}

#if defined(AUX) && defined(STANDALONE)
	chroot("(6,0,31)");
#endif
	file_name = argv[optind];
	file_sys = open(file_name, 2);
	if (file_sys < 0) {
		perror(file_name);
		leave(MSGSTR(SETFSTAG_11, "Cannot open %s for reading and writing\n"), file_name);
	}
#ifdef _OSF_SOURCE
	lseek(file_sys, (long) SBLOCK * DEV_BSIZE, 0);
#else
	lseek(file_sys, (long) SUPERBOFF, 0);
#endif
	if (read(file_sys, &f, sizeof(f)) != sizeof(f))
		leave(MSGSTR(SETFSTAG_12, "Cannot read super block from %s\n"), file_name);

	if (verbose)  {
		printf(" %s\n", file_name);
#ifdef _OSF_SOURCE
		printf(MSGSTR(SETFSTAG_13, " Last mounted on: %s\n"), f.fs_fsmnt);
#else
		printf(MSGSTR(SETFSTAG_14, " File System: %.6s Volume %.6s\n\n"),
			f.s_fname, f.s_fpack);
#endif
	}


	if (verbose)
		printf(MSGSTR(SETFSTAG_15, " ** Validate File System\n"));
	validate_file_system();

#if !defined(AUX) && !defined(_OSF_SOURCE)
	if (verbose)
		printf(MSGSTR(SETFSTAG_16, " ** Invalidate Superblock\n"));
	invalidate_superblock();
#endif

	if (verbose)
		printf(MSGSTR(SETFSTAG_17, " ** Set Tag on Ilist\n"));
	traverse_file_system(all);

#if !defined(AUX) && !defined(_OSF_SOURCE)
	if (verbose)
		printf(MSGSTR(SETFSTAG_18, " ** Restore Superblock State\n"));
	f.s_state = FsOKAY - (long) f.s_time;
	lseek(file_sys, (long) SUPERBOFF, 0);
	if (write(file_sys, &f, sizeof(f)) != sizeof f)
		leave(MSGSTR(SETFSTAG_19, "Cannot rewrite super block\n"));
#endif

	sync();

	return 0;
}


/*
 * Make sure the program can run, because if, in the middle, something
 * bad happens, the file system may be corrupted.  Check to make sure the
 * file system is unmounted, in a consistent state (fsck should really
 * be run prior to this), and the sufficient free space remains to upgrade
 * the file system.
 */
static void
validate_file_system()
{
	struct stat dev_stat_buf;
#ifdef _OSF_SOURCE
	struct statfs statfs_buf;
	struct stat   fs_stat_buf;
#else
	struct ustat ustat_buf;
#endif

#ifdef SYSV
	if (f.s_magic == FsMAGIC)  {
		if (f.s_type == (FsSW | Fs1b))
			bsize = 512;
		else
			bsize = 1024;
	}
	else
		bsize = 512;
#endif

#if !defined(AUX) || !defined(STANDALONE)
	if (fstat(file_sys, &dev_stat_buf) != 0)
		leave(MSGSTR(SETFSTAG_20, "Cannot stat file system\n"));

	if ((dev_stat_buf.st_mode & S_IFMT) != S_IFBLK &&
	    (dev_stat_buf.st_mode & S_IFMT) != S_IFCHR)
		leave(MSGSTR(SETFSTAG_21, "Block or character special file required\n"));

#endif

#ifdef _OSF_SOURCE

	/*
	 * For now, check whether the "last mounted on" indicator in
	 * the file system matches the device number of that pathname.
	 * When the FS_CLEAN logic is implemented, can just check that flag.
	 */

	if (f.fs_fsmnt[0] != '\0')
		if (stat(f.fs_fsmnt, &fs_stat_buf) == 0)
			if (fs_stat_buf.st_dev == dev_stat_buf.st_rdev)
				leave(
			MSGSTR(SETFSTAG_22, "File system is not clean -- use fsck first\n"));

	disk_set_file_system(&f, f.fs_bsize);

	if (!disk_secure_file_system())
		leave(MSGSTR(SETFSTAG_23, "File system not built to accommodate security tags\n"));
#else
	
	/*
	 * Check if file system is currently mounted.
	 */

	if (ustat(dev_stat_buf.st_rdev, &ustat_buf) == 0)
		leave(MSGSTR(SETFSTAG_24, "File system is mounted -- umount first\n"));
#endif

#ifdef SYSV
	if (f.s_state != FsOKAY - (long) f.s_time)
		leave(MSGSTR(SETFSTAG_25, "File system is not OKAY -- use fsck first\n"));

	disk_set_file_system(&f, bsize);

	if (!disk_valid(&f))
		leave(MSGSTR(SETFSTAG_26, "File does not appear to be a file system\n"));
	if (!disk_secure_file_system())
		leave(MSGSTR(SETFSTAG_23, "File system not built to accommodate security tags\n"));
#endif

#ifdef AUX
	if (!FsSEC(&f))
		leave(MSGSTR(SETFSTAG_23, "File system not built to accommodate security tags\n"));
#endif

#ifdef AUX
	inopb = FsINOPB(&f);
	bsize = FsBSIZE(&f);
	imax = (f.s_isize - (SUPERB+1)) * inopb;
#endif
#ifdef _OSF_SOURCE
	inopb = INOPB(&f);
	bsize = f.fs_bsize;
	imax = f.fs_ncg * f.fs_ipg - 1;
#endif
#ifdef SYSV
	inopb = disk_inodes_per_block();
	imax = (f.s_isize - (SUPERB+1)) * inopb;
#endif
}


#ifdef SYSV
/*
 * Mark the superblock as bad so it cannot be mounted until the transformation
 * is complete.
 */
static void
invalidate_superblock()
{
	register int i;

	f.s_state = FsBAD;
	lseek(file_sys, (long) SUPERBOFF, 0);
	if (write(file_sys, &f, sizeof(f)) != sizeof f)
		leave(MSGSTR(SETFSTAG_19, "Cannot rewrite super block\n"));

	sync();
}
#endif


#ifndef _OSF_SOURCE/*{*/
/*
 * Read in the ilist, setting tags as we go.
 */
static void
traverse_file_system(all)
	int all;
{
	register int i, bn, blockdirty, index;
#ifdef AUX
#define DITYPE struct sec_dinode
#else
#define DITYPE struct dinode;
#endif
	register DITYPE *dp, *buf;

	buf = (DITYPE *) calloc(inopb, sizeof(DITYPE));
	if (buf == (DITYPE *) 0)
		leave(MSGSTR(SETFSTAG_27, "Can't allocate space for disk block buffer\n"));

	if (verbose)
		printf(MSGSTR(SETFSTAG_28, "\tThere are %d inodes.\n"), imax);

	if (verbose)
		printf(MSGSTR(SETFSTAG_29, "\tUpdating inode # "));

	for (i = 1, bn = SUPERB + 1; bn < f.s_isize; ++bn) {

		lseek(file_sys, fs_offset(bn), 0);
		if (read(file_sys, buf, sizeof *buf * inopb) != sizeof *buf * inopb)
			leave(MSGSTR(SETFSTAG_30, "Read error on block %d, aborting\n"), bn);

		for (blockdirty = 0, dp = buf; dp < &buf[inopb]; ++dp, ++i) {

			if (verbose)
				printnum(i, imax);

			/*
			 * Skip unallocated inodes
			 */
#ifdef AUX
			if (dp->di_node.di_mode == 0)
#else
			if (dp->di_mode == 0)
#endif
				continue;

			/*
			 * Only update 0-valued tags by default.  The all
			 * flag overrides this and resets non-0 tag values
			 * also.
			 */
			for (index = 0; index < SEC_NUM_TAGS; ++index)
				if (tag_valid[index] &&
				    (all || dp->di_tag[index] == (tag_t) 0) &&
				    tag_value[index] != dp->di_tag[index]) {
					dp->di_tag[index] = tag_value[index];
					blockdirty = 1;
				}
		}

		if (blockdirty) {
			lseek(file_sys, fs_offset(bn), 0);
			if (write(file_sys, buf, sizeof *buf * inopb) !=
					sizeof *buf * inopb)
				leave(MSGSTR(SETFSTAG_31, "Write error on block %d, aborting\n"),
					bn);
		}
	}

	if (verbose)
		printf("\n");
}
#endif /*}*/

#ifdef _OSF_SOURCE /*{*/
static void
traverse_file_system(all)
	int all;
{
	char ibuf[MAXBSIZE];
	int cyl;
	int block;
	register struct fs *fs = &f;
	struct sec_dinode *dp;
	ino_t inum;
	int i;
	int index;
	int blockdirty;

	for (cyl = 0, inum = 0; cyl < fs->fs_ncg; cyl++) {
	    /*
	     * for each block in the group, set tags on the inodes.
	     */
	    for (block = cgimin(fs, cyl);
		 block < cgdmin(fs, cyl);
		 block += fs->fs_frag)
	    {
		blockdirty = 0;

		/*
		 * read in a block of inodes
		 */
		lseek(file_sys, fsbtodb(fs, block) * DEV_BSIZE, 0);
		read(file_sys, ibuf, fs->fs_bsize);
		dp = (struct sec_dinode *) ibuf;

		/*
		 * set tags on the inodes in the block.
		 */
		for (i = 0; i < fs->fs_inopb; i++, inum++, dp++) {
		    if (verbose)
			printnum(inum, imax);

		    if (dp->di_node.di_mode != 0) {
			/*
			 * Only update 0-valued tags by default.  The all
			 * flag overrides this and resets non-0 tag values
			 * also.
			 */
			for (index = 0; index < SEC_NUM_TAGS; ++index)
				if (tag_valid[index] &&
				   (all || dp->di_sec.di_tag[index] == (tag_t)0)
				    && tag_value[index] != dp->di_sec.di_tag
				    [index]) {
					dp->di_sec.di_tag[index] = 
						tag_value[index];
					blockdirty = 1;
				}
		    }
		}
		/*
		 * write the block back to the disk
		 */
		if (blockdirty) {
			lseek(file_sys, fsbtodb(fs, block) * DEV_BSIZE , 0);
			if (write(file_sys, ibuf, fs->fs_bsize) != fs->fs_bsize)
				leave(MSGSTR(SETFSTAG_31, "Write error on block %d, aborting\n"),
				  block);
		}
	    }
	}
	if (verbose)
		printf ("\n");
}
#endif /*}*/


/*
 * Take a string of digits and return the decimal number.
 */
static tag_t
number(string)
	char *string;
{
	tag_t collect_num = 0;

	while (*string != '\0')  {
		if (!isascii(*string) || !isdigit(*string) ||
		    (*string > '9'))  {
			fprintf(stderr,
				MSGSTR(SETFSTAG_32, "%s: mode contains non-digit '%c'\n"),
				command_name, *string);
			exit(2);
		}

		collect_num = (collect_num * 10) + (tag_t) (*string - '0');
		string++;
	}

	return collect_num;
}


/*
 * Print a number and back up to the first character on the screen.  This is
 * an output line saving feature.
 */
static void
printnum(i, always)
	register int i;
	register int always;
{
	register int j;
	register int len;
	char num[10];

	if ((i == always) || ((i % MOD) == 0))  {
		sprintf(num, "%-7d", i);
		len = strlen(num);
		printf("%s", num);
		for (j = 0; j < len; j++)
			putchar('\b');
		fflush(stdout);
	}
}


/*
 *
 *	An error has been detected.  Print a message and exit.
 *
 */

static void
leave(fmt, a, b, c, d, e, f)
	char *fmt;
	int a, b, c, d, e, f;
{
	fflush(stdout);

	fprintf(stderr, fmt, a, b, c, d, e, f);
	exit(1);
}

#endif /* SEC_ARCH */
