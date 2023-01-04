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
static char	*sccsid = "@(#)$RCSfile: newfs.c,v $ $Revision: 4.3.10.7 $ (DEC) $Date: 1994/01/12 00:05:49 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1983, 1989 The Regents of the University of California.
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983, 1989 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/* ULTRIX/OSF:  Made changes to have newfs work with logical volumes
 *              20-Mar-91 Terry Carruthers
 *
 *              Current restrictions with logical volume support are:
 *                  - must provide absolute path name of log vol
 */

#include <lvm/lvm.h>

/*
 * newfs: friendly front end to mkfs
 */

#include <sys/secdefines.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <ufs/fs.h>
#include <ufs/dir.h>
#include <sys/ioctl.h>
#ifdef	multimax
#include <layout.h>
#include <mmaxio/msioctl.h>
#undef MAXPARTITIONS
#endif	/* multimax */
#ifdef i386
#define DKTYPENAMES
#endif
#include <sys/disklabel.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/table.h>

#include <stdio.h>
#include <ctype.h>
#include <paths.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#ifndef	multimax
#define COMPAT			/* allow non-labeled disks */
#endif

/*
 * The following two constants set the default block and fragment sizes.
 * Both constants must be a power of 2 and meet the following constraints:
 *	MINBSIZE <= DESBLKSIZE <= MAXBSIZE
 *	sectorsize <= DESFRAGSIZE <= DESBLKSIZE
 *	DESBLKSIZE / DESFRAGSIZE <= 8
 *
 * If making a System V file system, the default block size is DFL_S5BLKSIZE.
 */
#define	DFL_FRAGSIZE	1024
#define	DFL_BLKSIZE	8192
#define DFL_S5BLKSIZE   512

/*
 * Cylinder groups may have up to many cylinders. The actual
 * number used depends upon how much information can be stored
 * on a single cylinder. The default is to use 16 cylinders
 * per group.
 */
#define	DESCPG		16	/* desired fs_cpg */

/*
 * MINFREE gives the minimum acceptable percentage of file system
 * blocks which may be free. If the freelist drops below this level
 * only the superuser may continue to allocate blocks. This may
 * be set to 0 if no reserve of free blocks is deemed necessary,
 * however throughput drops by fifty percent if the file system
 * is run at between 90% and 100% full; thus the default value of
 * fs_minfree is 10%. With 10% free space, fragmentation is not a
 * problem, so we choose to optimize for time.
 */
#define MINFREE		10
#define DEFAULTOPT	FS_OPTTIME

/*
 * ROTDELAY gives the minimum number of milliseconds to initiate
 * another disk transfer on the same cylinder. It is used in
 * determining the rotationally optimal layout for disk blocks
 * within a file; the default of fs_rotdelay is 0ms.
 */
#define ROTDELAY	0
#define MAX_ROTDELAY    999

/*
 * MAXCONTIG sets the default for the maximum number of blocks
 * that may be allocated sequentially. Since UNIX drivers are
 * not capable of scheduling multi-block transfers, this defaults
 * to 1 (ie no contiguous blocks are allocated).
 *
 * Now, MAXCONTIG is the maxumum number of consecutive blocks
 * which can be read/written. The default is 8 (64Kb for 8K/1K).
 */
#define MAXCONTIG	8

/*
 * MAXBLKPG determines the maximum number of data blocks which are
 * placed in a single cylinder group. The default is one indirect
 * block worth of data blocks.
 */
#define MAXBLKPG(bsize)	((bsize) / sizeof(daddr_t))

/*
 * Each file system has a number of inodes statically allocated.
 * We allocate one inode slot per NFPI fragments, expecting this
 * to be far more than we will ever need.
 */
#define	NFPI		4

/*
 * For each cylinder we keep track of the availability of blocks at different
 * rotational positions, so that we can lay out the data to be picked
 * up with minimum rotational latency.  NRPOS is the default number of
 * rotational positions that we distinguish.  With NRPOS of 8 the resolution
 * of our summary information is 2ms for a typical 3600 rpm drive.
 */
#define	NRPOS		8	/* number distinct rotational positions */


int	mfs;			/* run as the memory based filesystem */
int	Nflag;			/* run without writing file system */
int	fssize;			/* file system size */
int	ntracks;		/* # tracks/cylinder */
int	nsectors;		/* # sectors/track */
int	nphyssectors;		/* # sectors/track including spares */
int	secpercyl;		/* sectors per cylinder */
int	trackspares = -1;	/* spare sectors per track */
int	cylspares = -1;		/* spare sectors per cylinder */
int	sectorsize;		/* bytes/sector */
#ifdef tahoe
int	realsectorsize;		/* bytes/sector in hardware */
#endif
int	rpm;			/* revolutions/minute of drive */
int	interleave;		/* hardware sector interleave */
int	trackskew = -1;		/* sector 0 skew, per track */
int	headswitch = 0;		/* head switch time, usec */
int	trackseek = 0;		/* track-to-track seek, usec */
int	fsize = 0;		/* fragment size */
int	bsize = 0;		/* block size */
int	cpg = DESCPG;		/* cylinders/cylinder group */
int	cpgflg;			/* cylinders/cylinder group flag was given */
int	minfree = MINFREE;	/* free space threshold */
int	opt = DEFAULTOPT;	/* optimization preference (space or time) */
int	density;		/* number of bytes per inode */
int	maxcontig = MAXCONTIG;	/* max contiguous blocks to allocate */
int	rotdelay = ROTDELAY;	/* rotational delay between blocks */
int	maxbpg;			/* maximum blocks per file in a cyl group */
int	nrpos = NRPOS;		/* # of distinguished rotational positions */
int	bbsize = BBSIZE;	/* boot block size */
int	sbsize = SBSIZE;	/* superblock size */
int	mntflags;		/* flags to be passed to mount */
u_int	memleft;		/* virtual memory available */
caddr_t	membase;		/* start address of memory based filesystem */

int     fstype = FS_BSDFFS;     /* type of file system, currently supported are
				   BSD FFS and System V style file systems.
				   Default file system type is BSD fast file
				   system */

#if	defined(COMPAT) || defined(i386)
char	*disktype;
int	unlabelled;
#endif
#if	defined(i386)
int	partno = -1;
#endif

char	device[MAXPATHLEN];
char	*progname;

#if SEC_ACL
char *acl_label = (char *) 0;
#endif
#if SEC_MAC
char *mand_label = (char *) 0;
#endif
#if SEC_ILB
char *ilb_label = (char *) 0;
#endif
#if SEC_BASE
static privvec_t additional_privs;
#endif
#if SEC_FSCHANGE
int mkfs_extended_fs;		/* make an extended format filesystem */
static int untagged;		/* make an untagged format filesystem */
#endif

extern	int errno;
extern  char *rawname();
char	*strchr();
char	*strrchr();
char    *itoa();
char	*rindex();

struct ufs_partid {
	dev_t lvm_or_lsm_dev;		/* lvm/lsm device		*/
	short bus;			/* Bus				*/
	short adpt_num;			/* Adapter number		*/
	short nexus_num;		/* Nexus or node on adapter no.	*/
	short bus_num;			/* Bus number			*/
	short ctlr_num;			/* Controller number		*/
	short rctlr_num;		/* Remote controller number	*/
	short slave_num;		/* Plug or line number		*/
	short unit_num;			/* Ultrix device unit number	*/
	long  category_stat;		/* Category specific mask	*/
	struct pt {
		long part_blkoff;	/* Beginning partition sector	*/
		long part_nblocks;	/* Number of sectors in partition */
	} pt_part[MAXPARTITIONS];
} ufs_partid;

main(argc, argv)
	int argc;
	char *argv[];
{
	char *cp, *special;
	register struct partition *pp = NULL;
	register struct disklabel *lp = NULL;
	struct disklabel *getdisklabel();
	void dyndiskcheck();
	struct partition oldpartition;
	struct mfs_args args;
	struct stat st;
	int fsi, fso;
	register int i;
	int status;
#if	!defined(i386)
	int partno = -1;
#endif
	char buf[BUFSIZ];
	char dev_path[MAXPATHLEN];		/* device's path name. */

	int s5mkfs_pid;
	int stat_loc;				/* result of wait */
	char *s5mkfs_arg;
	char bsize_string[30];
	char fssize_string[30];
	char *s5fs_proto = NULL;
	int s5fs_gap = 0;
	int s5fs_blk_per_cyl = 0;
	int s5fs_inodes = 0;
	char *s5fs_gap_string = "0";
	char *s5fs_blk_string = "0";
	char *s5fs_inode_string;
	int error;
	extern char *optarg;
        extern int optind;
        char *opstring;                 /* option string for getop */
        char *endptr;
        int ch;
	int islsm();

#if SEC_BASE && !defined(SEC_STANDALONE)
	privvec_t saveprivs;

#ifdef DEBUG
	printf("Entering newfs\n");
#endif
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, "%s: need sysadmin authorization\n",
			command_name);
		exit(1);
	}
	setprivvec(additional_privs, SEC_ALLOWDACACCESS, SEC_FILESYS,
#if SEC_MAC
			SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
			SEC_ALLOWNCAVACCESS,
#endif
			-1);
#endif /* SEC_BASE && !defined(SEC_STANDALONE) */

	if ((progname = strrchr(*argv, '/') + 1) == (char *)1)
		progname = *argv;
	if ( !strcmp(progname, "mfs") || !strcmp(progname, "mount_mfs") ) {
		Nflag++;
		mfs++;
	}
#ifdef DEBUG
	printf("Doing arg processing\n");
#endif
#ifdef COMPAT
#if SEC_FSCHANGE
	opstring = "F:NS:D:a:b:c:d:e:f:i:k:l:m:n:o:p:P:G:C:I:r:s:t:u:x:T:L:";
#else
	opstring = "F:NS:D:a:b:c:d:e:f:i:k:l:m:n:o:p:P:G:C:I:r:s:t:u:x:T:";
#endif  /* SEC_FSCHANGE */
#endif

#ifndef COMPAT
#if SEC_FSCHANGE
	opstring = "F:NS:a:b:c:d:e:f:i:k:l:m:n:o:p:P:G:C:I:r:s:t:u:x:T:L:";
#else
	opstring = "F:NS:a:b:c:d:e:f:i:k:l:m:n:o:p:P:G:C:I:r:s:t:u:x:T:";
#endif /* SEC_FSCHANGE */
#endif

	while ((ch = getopt(argc, argv, opstring)) != EOF)
		switch((char)ch) {
			case 'F':
				if (!mfs)
					fatal("-F: unknown flag");
				mntflags = strtol(optarg, &endptr, 0);
				if (mntflags == 0 || *endptr != '\0')
					fatal("%s: bad mount flags", optarg);
				break;

			case 'N':
				Nflag++;
				break;

			case 'S':
				sectorsize = strtol(optarg, &endptr, 0);
				if (sectorsize <= 0 || *endptr != '\0')
					fatal("%s: bad sector size", optarg);
				break;
#ifdef COMPAT
			case 'D':
				disktype = optarg;
				break;
#endif

			case 'a':
				maxcontig = strtol(optarg, &endptr, 0);
				if (maxcontig <= 0 || *endptr != '\0')
					fatal("%s: bad max contiguous blocks",
						optarg);
				break;

			case 'b':
				bsize = strtol(optarg, &endptr, 0);
				if (bsize <= 0 || *endptr != '\0')
					fatal("%s: bad block size", optarg);
				break;

			case 'c':
				cpg = strtol(optarg, &endptr, 0);
				if (cpg <= 0 || *endptr != '\0')
					fatal("%s: bad cylinders/group", optarg);
				cpgflg++;
				break;

			case 'd':
				rotdelay = strtol(optarg, &endptr, 0);
				if (rotdelay < 0 || rotdelay > MAX_ROTDELAY
						 || *endptr != '\0')
					fatal("%s: bad rotational delay",
						optarg);
				break;

			case 'e':
				maxbpg = strtol(optarg, &endptr, 0);
				if (maxbpg <= 0 || *endptr != '\0')
					fatal("%s: bad blocks per file in a cyl group",
						optarg);
				break;

			case 'f':
				fsize = strtol(optarg, &endptr, 0);
				if (fsize <= 0 || *endptr != '\0')
					fatal("%s: bad frag size", optarg);
				break;

			case 'i':
				density = strtol(optarg, &endptr, 0);
				if (density <= 0 || *endptr != '\0')
					fatal("%s: bad bytes per inode",
						optarg);
				break;

			case 'k':
				trackskew = strtol(optarg, &endptr, 0);
				if (trackskew < 0 || *endptr != '\0')
					fatal("%s: bad track skew", optarg);
				break;

			case 'l':
				interleave = strtol(optarg, &endptr, 0);
				if (interleave <= 0 || *endptr != '\0')
					fatal("%s: bad interleave", optarg);
				break;

			case 'm':
				minfree = strtol(optarg, &endptr, 0);
				if (minfree < 0 || minfree > 99 || *endptr != '\0')
					fatal("%s: bad free space %%",
						optarg);
				break;

			case 'n':
				nrpos = strtol(optarg, &endptr, 0);
				if (nrpos <= 0 || *endptr != '\0')
					fatal("%s: bad rotational layout count",
						optarg);
				break;

			case 'o':
				if (strcmp(optarg, "space") == 0)
					opt = FS_OPTSPACE;
				else if (strcmp(optarg, "time") == 0)
					opt = FS_OPTTIME;
				else
					fatal("%s: bad optimization preference %s",
					optarg,
					"(options are `space' or `time')");
				break;

			case 'p':
				trackspares = strtol(optarg, &endptr, 0);
				if (trackspares < 0 || *endptr != '\0')
					fatal("%s: bad spare sectors per track", optarg);
				break;
#ifdef notdef
			case 'P':
				s5fs_proto = optarg;
				break;

			case 'G':
				s5fs_gap = atoi(optarg);
				s5fs_gap_string = optarg;
				break;

			case 'C':
				s5fs_blk_per_cyl = atoi(optarg);
				s5fs_blk_string = optarg;
				break;

			case 'I':
				s5fs_inodes = atoi(optarg);
				s5fs_inode_string = optarg;
				break;
#endif
			case 'r':
				rpm = strtol(optarg, &endptr, 0);
				if (rpm <= 0 || *endptr != '\0')
					fatal("%s: bad revs/minute", optarg);
				break;

			case 's':
				fssize = strtol(optarg, &endptr, 0);
				if (fssize <= 0 || *endptr != '\0')
					fatal("%s: bad file system size",
						optarg);
				break;

			case 't':
				ntracks = strtol(optarg, &endptr, 0);
				if (ntracks <= 0 || *endptr != '\0')
					fatal("%s: bad total tracks", optarg);
				break;

			case 'u':
				nsectors = strtol(optarg, &endptr, 0);
				if (nsectors <= 0 || *endptr != '\0')
					fatal("%s: bad sectors/track", optarg);
				break;

			case 'x':
				cylspares = strtol(optarg, &endptr, 0);
				if (cylspares < 0 || *endptr != '\0')
					fatal("%s: bad spare sectors per cylinder", optarg);
				break;

			case 'T':
				if ( !strcmp(optarg, "ufs"))
				      fstype = FS_BSDFFS;
#ifdef notdef
				else if ( !strcmp(*argv, "s5fs"))
				      fstype = FS_SYSV;
#endif
				else
				         fatal("%s: bad file system type", *argv);
				break;
#if SEC_FSCHANGE
			case 'L':
				switch (*optarg) {
#if SEC_ACL
				case 'A':
					if (optarg[1] != '\0') {
						acl_label = optarg+1;
						mkfs_extended_fs = 1;
					}
					else if (optind < argc) {
						acl_label = argv[optind++];
						mkfs_extended_fs = 1;
					}
					else
						fatal("-LA specified but no ACL Label");
					break;
#endif
#if SEC_ILB
				case 'I':
					if (optarg[1] != '\0') {
						ilb_label = optarg+1;
						mkfs_extended_fs = 1;
					}
					else if  (optind < argc) {
						ilb_label = argv[optind++];
						mkfs_extended_fs = 1;
					}
					else
						fatal("-LI specified but no Information Label");
					break;
#endif
#if SEC_MAC
				case 'S':
					if (optarg[1] != '\0') {
						mand_label = optarg+1;
						mkfs_extended_fs = 1;
					}
					else if (optind < argc) {
						mand_label = argv[optind++];
						mkfs_extended_fs = 1;
					}
					else	
						fatal("-LS specified but no Sensitivity Label");
					break;
#endif
				case 'U':
					untagged = 1;
					break;
				default:
					fatal("-L%c: unknown label option",
						*optarg);
				}
				break;
#endif	/* SEC_FSCHANGE */
			default:
				usage();
				exit(1);
			}
	argc -= optind;
	argv += optind;
#if SEC_FSCHANGE
	if (mkfs_extended_fs && untagged)
		fatal("-LU option is illegal when labels are specified");

	if (!mkfs_extended_fs && !untagged) {
#ifndef SEC_STANDALONE
#if SEC_MAC
		/*
		 * Must specify a sensitivity label when
		 * building tagged filesystem.
		 */
		if (mand_label == (char *) 0)
			fatal("sensitivity label required for extended filesystem");
#endif
#if SEC_ILB
		/*
		 * Print warning if no information label specified
		 */
		if (ilb_label == (char *) 0)
			fprintf(stderr,
				"WARNING: using WILDCARD information label\n");
#endif
#endif /* !SEC_STANDALONE */
		/* Default to tagged format */
		mkfs_extended_fs = 1;
	}
#endif /* SEC_FSCHANGE */
#ifdef DEBUG
	printf("Continuing #1\n");
#endif
	if (argc < 1) {
		usage();
		exit(1);
	}
	/*
	 * If size was specified for mfs, use it, even if device was
	 * also specified; the device string could be bogus.
	 * Either a valid size or device must be specified.
	 */
	if (mfs) {
		if (fssize == 0) {
			if (argc == 1)
                                fatal("must specify size or device");
		} else {
			if (nsectors == 0)
				nsectors = 36;
			if (ntracks == 0)
				ntracks = 15;
		}
		if (argc == 1) {
			argv[1] = argv[0];
			goto skip;
		}
		/* else fall through and try to read device info. */
	}
	/* Locate the appropriate file/device to create the filesystem
	 * on. The user may specify:
	 *  (1) a simple device name of the block device, to be
	 *	located in /dev. The character device name will be
	 *	be formed by inserting an 'r' in front of the name.
	 *  (2) a simple device name of the character device, to be
	 *	located in /dev.
	 *  (3) an absolute or relative pathname (including /'s) of
	 *	the block device. A character device name will be
	 *	be formed by inserting an 'r' in front of the final
	 * 	pathname component.
	 *  (4) an absolute or relative pathname of the character
	 *	device.
	 *  (5) an absolute or relative pathname of an ordinary file.
	 *	The file must be pre-existing.
	 */
	special = argv[0];
	cp = strrchr(special, '/');
	if (cp == 0) {
		/*
		 * For mfs, it's ok to pass a garbage string for special,
		 * provided that the file system size was specified in
		 * the command line
		 */
		if (mfs) {
			if (fssize == 0)
				fatal("must specify size or device");
			goto skip;
		}
		/* The argument is either case (1) or (2) above. */
		/* Check for device in /dev/ */
		(void)sprintf(device, "%s%s", _PATH_DEV, special);
		if (stat(device, &st) < 0)
			fatal("cannot find %s", device);
		if ((st.st_mode & S_IFMT) == S_IFBLK) {
			/* This is a block device, case (1), form the
			 * character device name. */
			(void)sprintf(device, "%sr%s", _PATH_DEV, special);
		}
		/* special is now the path to an allegedly character device */
		special = device;
	} else {
		/* The argument is either case (3), (4), or (5) */
		if (stat(special, &st) < 0)
			fatal("cannot find %s", special);
		if ((st.st_mode & S_IFMT) == S_IFBLK) {
			/* This is a block device, case (3), form the
			 * character device name. */
		/*
			(void)strncpy(dev_path, special, (cp+1) - special);
			(void)sprintf(device, "%sr%s", dev_path, cp + 1);
		 */
			strcpy(device, rawname(special));
			special = device;
		}

		/* special is now the path to an allegedly character
		 * device, or an existing ordinary file. */
	}

#if SEC_BASE && !defined(SEC_STANDALONE)
	if (forceprivs(additional_privs, saveprivs))
		fatal("%s: insufficient privileges", command_name);
#endif
#ifdef DEBUG
	printf("Continuing #2\n");
#endif
	if (!Nflag) {
		fso = open(special, O_WRONLY);
		if (fso < 0) {
			perror(special);
			exit(2);
		}
	} else
		fso = -1;
	fsi = open(special, O_RDONLY);
	if (fsi < 0) {
		perror(special);
		exit(3);
	}
#ifdef DEBUG
	printf("Continuing #3\n");
#endif
	if (fstat(fsi, &st) < 0) {
		int err = errno;
		fprintf(stderr, "%s: ", progname);
		errno = err;
		perror(special);
		exit(4);
	}
#if SEC_BASE && !defined(SEC_STANDALONE)
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
#ifdef DEBUG
	printf("Continuing #4\n");
#endif
	if ((st.st_mode & S_IFMT) == S_IFCHR) {

		/* ULTRIX/OSF:  Check if the special device is actually a logical
		 *              volume
		 */

#ifdef DEBUG
		printf("Checking for lvm or lsm volumes \n");
#endif
		if (islsm(special, &fssize)){
                    /* It is a LSM volume */
#ifdef DEBUG
		    printf("It was a lsm volume...\n");
#endif
		    /* Make sure that fssize was set for LSM volumes */
		    if (fssize == 0)
			    fatal("%s: can't figure out file system size for LSM volume",
				  argv[0]);
			
		    /* Default the partition used to get disk information
		     * to "c" [this is ugly, but it works until we divorce
		     *         disk information from partition tables]
		     */
		    partno = 2;
#ifdef DEBUG
		    printf("It is a LSM volume with size %d\n",fssize);
#endif
                }
		else
		if (islv(special,&fssize))
		{
#ifdef DEBUG
		    printf("It was a lvm volume...\n");
#endif
		    /* Make sure that fssize was set for logical volumes */
		    if (fssize == 0)
			    fatal("%s: can't figure out file system size for logical volume",
				  argv[0]);
			
		    /* Default the partition used to get disk information
		     * to "c" [this is ugly, but it works until we divorce
		     *         disk information from partition tables]
		     */
		    partno = 2;
		}
                else
		{
#ifdef DEBUG
		    printf("It wasn't a lsm or lvm volume...\n");
#endif
		    cp = strchr(argv[0], '\0') - 1;
		    if (cp == 0 ||
#if	defined(multimax) || defined(i386)
			(*cp < 'a' || *cp > 'p')
#else
			(*cp < 'a' || *cp > 'h') 
#endif
			&& !isdigit(*cp))
			    fatal("%s: can't figure out file system partition",
					    argv[0]);
		    partno = isdigit(*cp) ? *cp - '0' : *cp - 'a'; 
		}

#ifdef	multimax
		getheaderinfo(special, partno);
#else
#ifdef COMPAT
		if (!mfs && disktype == NULL)
			disktype = argv[1];
#endif
#ifdef DEBUG
	printf("Continuing #4a\n");
#endif
		lp = getdisklabel(special, fsi);
	
#ifdef	DEBUG
dumplabel(lp);
#endif
#ifdef DEBUG
	printf("Continuing #5\n");
#endif
		if (lp) {
			if (partno >= lp->d_npartitions) {
				fatal("%s : only %d partitions", argv[0],
					lp->d_npartitions);
			}
			pp = &lp->d_partitions[partno];
		}
#ifndef COMPAT
		else {
			fatal("%s: can't read disk label", argv[0]);
		}
#endif	/* COMPAT */
#endif /* multimax */
	} else if ((st.st_mode & S_IFMT) != S_IFREG)
		fatal("%s: bad file type", argv[0]);

#ifdef DEBUG
	printf("Continuing #6\n");
#endif
	if (fssize == 0) {
		if (pp) {
			if (pp->p_size == 0)
				fatal("%s: `%c' partition is unavailable",
					argv[0], *cp);
		} else {
#ifdef COMPAT
			if (unlabelled && disktype)
		fatal("%s: disk is unlabeled and no entry found in disktab",
				argv[0]);
			else if (unlabelled) {
				if (mfs)
		fatal("%s: can't read disk label; disk type must be specified with -D option",
		      argv[0]);
				else
		fatal("%s: can't read disk label; disk type must be specified",
				argv[0]);
			}
#endif
			fatal("%s: must specify file system size", argv[0]);
		}
		fssize = pp->p_size;
	}
	/* ULTRIX/OSF:  Delete the next two lines.
	 *              The mkfs routines check for maximum file size
	 *              by trying to write to the last block -- that
	 *              check is more generic
	 */
#if FALSE
	if (pp && (fssize > pp->p_size) && !mfs)
	       fatal("%s: maximum file system size on the `%c' partition is %d",
			argv[0], *cp, pp->p_size);
#endif
	/*
	 * Check for overlapping partitions. Warn user if the partition
	 * we are writing overlaps any mounted file system.
	 */
	if (!Nflag && !mfs && !init_partid(special, &ufs_partid)) {
		if (check_for_overlap(special, &ufs_partid)) {
#ifdef notdef
			char c;

			do      {
				printf("CONTINUE? [y/n] ");
				(void) fflush(stdout);
				c = getc(stdin);
				while (c != '\n' && getc(stdin) != '\n')
					if (feof(stdin))
						exit(8);
			} while (c != 'y' && c != 'Y' && c != 'n' && c != 'N');
			if (c != 'y' && c != 'Y')
				exit(8);
#endif
			exit(8);
		}
	}
skip:
#ifdef DEBUG_S5FS
	printf("making fstype %s\n",fstypenames[fstype]);
#endif DEBUG_S5FS
#ifdef DEBUG
	printf("Continuing #7\n");
#endif
	if (rpm == 0) {
		if (lp) rpm = lp->d_rpm;
		if (rpm <= 0)
			rpm = 3600;
	}
	if (ntracks == 0) {
		if (lp) ntracks = lp->d_ntracks;
		if (ntracks <= 0)
			fatal("%s: no default #tracks", argv[0]);
	}
	if (nsectors == 0) {
		if (lp) nsectors = lp->d_nsectors;
		if (nsectors <= 0)
			fatal("%s: no default #sectors/track", argv[0]);
	}
	if (sectorsize == 0) {
		if (lp) sectorsize = lp->d_secsize;
		else sectorsize = DEV_BSIZE;
		if (sectorsize <= 0)
			fatal("%s: no default sector size", argv[0]);
	}
	if (trackskew == -1) {
		if (lp) trackskew = lp->d_trackskew;
		if (trackskew < 0)
			trackskew = 0;
	}
	if (interleave == 0) {
		if (lp) interleave = lp->d_interleave;
		if (interleave <= 0)
			interleave = 1;
	}
	if (fsize == 0) {
		if (pp) fsize = pp->p_fsize;
		if (fsize <= 0)
			fsize = MAX(DFL_FRAGSIZE, lp?lp->d_secsize:0);
	}
	if (bsize == 0) {
	        if (fstype == FS_SYSV) {
		        bsize = DFL_S5BLKSIZE;
	        } else {                     /* it must be a BSDFFS */
		  if (pp) bsize = pp->p_frag * pp->p_fsize;
		  if (bsize <= 0)
			bsize = MIN(DFL_BLKSIZE, 8 * fsize);
		}
	      }
	if (fstype == FS_BSDFFS && bsize < MINBSIZE)
		fatal("%s: bad block size", *argv);
	/*
	 * PRS - XXX
	 * Ensure block size == 8192 and frag size >= 1024
	 */
	if (fstype == FS_BSDFFS && bsize != DFL_BLKSIZE) {
		fprintf(stderr, "Warning: changing block size to 8192\n");
		bsize = DFL_BLKSIZE;
	}
	if (fstype == FS_BSDFFS && fsize < DFL_FRAGSIZE) {
		fprintf(stderr, "Warning: changing frag size to 1024\n");
		fsize = DFL_FRAGSIZE;
	}
	if (density == 0)
		density = NFPI * fsize;
	if (minfree < 10 && opt != FS_OPTSPACE) {
		fprintf(stderr, "Warning: changing optimization to space ");
		fprintf(stderr, "because minfree is less than 10%%\n");
		opt = FS_OPTSPACE;
	}
#ifdef DEBUG
	printf("Continuing #8\n");
#endif
	if (trackspares == -1) {
		if (lp) trackspares = lp->d_sparespertrack;
		if (trackspares < 0)
			trackspares = 0;
	}
	nphyssectors = nsectors + trackspares;
	if (cylspares == -1) {
		if(lp) cylspares = lp->d_sparespercyl;
		if (cylspares < 0)
			cylspares = 0;
	}
	secpercyl = nsectors * ntracks - cylspares;
	if (lp && (secpercyl != lp->d_secpercyl))
		fprintf(stderr, "%s (%d) %s (%d)\n",
			"Warning: calculated sectors per cylinder", secpercyl,
			"disagrees with disk label", lp->d_secpercyl);
#ifdef DEBUG
	printf("Continuing #9\n");
#endif
	if (maxbpg == 0)
		maxbpg = MAXBLKPG(bsize);
	if (lp) {
		headswitch = lp->d_headswitch;
		trackseek = lp->d_trkseek;
		bbsize = lp->d_bbsize;
		sbsize = lp->d_sbsize;
		oldpartition = *pp;
	}
#ifdef tahoe
	realsectorsize = sectorsize;
	if (sectorsize != DEV_BSIZE) {		/* XXX */
		int secperblk = DEV_BSIZE / sectorsize;

		sectorsize = DEV_BSIZE;
		nsectors /= secperblk;
		nphyssectors /= secperblk;
		secpercyl /= secperblk;
		fssize /= secperblk;
		pp->p_size /= secperblk;
	}
#endif
	if (fstype == FS_BSDFFS) {
#ifdef DEBUG
		printf("Trying to run mkfs\n");
#endif
		mkfs((pp ? pp : &oldpartition), special, fsi, fso);
#ifdef tahoe
		if (realsectorsize != DEV_BSIZE)
			pp->p_size *= DEV_BSIZE / realsectorsize;
#endif
	} else {        /* System V file system */
	    if (s5fs_proto == NULL) {
	        itoa(bsize,bsize_string);
		itoa(fssize, fssize_string);
		s5mkfs_arg = (char *) (strcat (strcat(bsize_string, ":"), fssize_string));
		if (s5fs_inodes != 0)
		    s5mkfs_arg = (char *) (strcat (strcat(s5mkfs_arg, ":"), s5fs_inode_string));
	      }
	      else {
		s5mkfs_arg = s5fs_proto;
	      }

		argv[0] = "/sbin/s5mkfs";
		argv[1] = special;
		argv[2] = s5mkfs_arg;
		argv[3] = s5fs_gap_string;

		if (s5fs_blk_per_cyl) {
		            argv[4] = s5fs_blk_string;
		 }
		else argv[4] = (char * ) 0;
	        argv[5] = (char *) 0;


	  if ((s5mkfs_pid = fork()) == 0) {             /* perform the mkfs */
	        execvp ("/sbin/s5mkfs", argv);
		fatal("Returned from execl");
	   }
	  else {                /* wait for s5mkfs to finish */
	        wait(&stat_loc);
		if (WIFEXITED(stat_loc))
			if ((error = WEXITSTATUS(stat_loc)) != 0){
				fatal("/sbin/s5mkfs returned %d\n",error);
			} else if (pp) {
				pp->p_fstype = FS_SYSV;
				pp->p_fsize = bsize;
			}
		else
		      fatal ("%s: /sbin/s5mkfs unexpected termination\n",argv[0]);
		}
	}             /* System V file system */
#ifndef	multimax
	if (!Nflag && pp && bcmp(pp, &oldpartition, sizeof(oldpartition)))
		rewritelabel(special, fso, lp);
#endif
	if (!Nflag)
		close(fso);
	close(fsi);
#if SEC_ARCH && !defined(SEC_STANDALONE)
	{
		/*
		 * Run the setfstag program if any policies specify labels
		 * Cannot be run in stand-alone mode because there are no
		 * daemons.
		 */

		int have_labels = 0;
#if SEC_ACL
		if (acl_label)
			have_labels++;
#endif
#if SEC_MAC
		if (mand_label)
			have_labels++;
#endif
#if SEC_ILB
		if (ilb_label) 
			have_labels++;
#endif
		if (have_labels) {
		        cp = strrchr(special, '/');
                        if (cp != 0) {
			    (void)strncpy(dev_path, special, (cp+1) - special);
			    (void)sprintf(device, "%s%s", dev_path, cp + 2);
			  }
			setfstag(device);
		      }
	}
#endif /* SEC_ARCH */

	if (mfs) {
		sprintf(buf, "mfs:%d", getpid());
		args.name = buf;
		args.base = membase;
		args.size = fssize * sectorsize;
		if (mount(MOUNT_MFS, argv[1], mntflags, &args) < 0) {
			perror("mfs: mount");
			exit(5);
		}
	}
	exit(0);
}

usage()
{
	if (mfs)
		fprintf(stderr,
			"usage: mfs [ fsoptions ] special-device mount-point\n");
	else {
#ifdef COMPAT
		fprintf(stderr, "usage: %s\n",
			"newfs [ fsoptions ] special-device [device-type]");
#else
		fprintf(stderr,
			"usage: newfs [ fsoptions ] special-device\n");
#endif
	}
	fprintf(stderr, "where fsoptions are:\n");
	fprintf(stderr, "\t-N do not create file system, %s\n",
		"just print out parameters");
#ifdef COMPAT
	fprintf(stderr, "\t-D disktype\n");
#endif
	fprintf(stderr, "\t-a maximum contiguous blocks\n");
	fprintf(stderr, "\t-b block size\n");
	fprintf(stderr, "\t-c cylinders/group\n");
/*	fprintf(stderr, "\t-C blocks/cyl (s5fs only)\n"); */
	fprintf(stderr, "\t-d rotational delay between %s\n",
		"contiguous blocks");
	fprintf(stderr, "\t-e maximum blocks per file in a %s\n",
		"cylinder group");
	fprintf(stderr, "\t-f frag size\n");
	if (mfs)
		fprintf(stderr, "\t-F mount flags\n");
/*	fprintf(stderr, "\t-G gap (s5fs only)\n"); */
	fprintf(stderr, "\t-i number of bytes per inode\n");
/*	fprintf(stderr, "\t-I number of inodes (s5fs only)\n"); */
	fprintf(stderr, "\t-k sector 0 skew, per track\n");
	fprintf(stderr, "\t-l hardware sector interleave\n");
	fprintf(stderr, "\t-m minimum free space %%\n");
	fprintf(stderr, "\t-n number of distinguished %s\n",
		"rotational positions");
	fprintf(stderr, "\t-o optimization preference %s\n",
		"(`space' or `time')");
	fprintf(stderr, "\t-p spare sectors per track\n");
/*	fprintf(stderr, "\t-P proto file name (s5fs only)\n"); */
	fprintf(stderr, "\t-r revolutions/minute\n");
	fprintf(stderr, "\t-s file system size (sectors)\n");
	fprintf(stderr, "\t-S sector size\n");
	fprintf(stderr, "\t-t tracks/cylinder\n");
	fprintf(stderr, "\t-T file system type [ufs]\n");
	fprintf(stderr, "\t-u sectors/track\n");
	fprintf(stderr, "\t-x spare sectors per cylinder\n");
#if SEC_ACL
	fprintf(stderr, "\t-LA ACL label\n");
#endif
#if SEC_ILB
	fprintf(stderr, "\t-LI Information label\n");
#endif
#if SEC_MAC
	fprintf(stderr, "\t-LS Sensitivity label\n");
#endif
#if SEC_FSCHANGE
	fprintf(stderr, "\t-LU make unextended filesystem\n");
#endif
}

#ifndef	multimax


/* 
 * dyndiskcheck() -
 *    For disks flagged as "dynamic_geometry", we will call the
 *    underlying driver to get geometry and partition information
 *    to plug into the disklabel.
 */
void
dyndiskcheck(lp, s, fd)
	struct disklabel *lp;
	char *s;
	int fd;
 {
 	struct pt_tbl pt_struct;
 	register struct partition *pp;
 	DEVGEOMST devgeom;
 	int p;

	/*
	 * First attempt to obtain geometry information for this
	 * dynamic geometry device.  If we can't get this information,
	 * then we punt.
	 */
	if (ioctl(fd, DEVGETGEOM, (char *)&devgeom) < 0) {
		perror("ioctl (DEVGETGEOM)");
		fatal("%s: can't get device geometry", s);
	} else {
		lp->d_ntracks = devgeom.geom_info.ntracks;
		lp->d_nsectors = devgeom.geom_info.nsectors;
		lp->d_ncylinders = devgeom.geom_info.ncylinders;
		lp->d_secperunit = devgeom.geom_info.dev_size;
		lp->d_rpm = 3600; 
	}
 
	/*
	 * OK, we've got geometry, now get the default partition table
	 * for this device.  Again, if we can't get a default pt, we
	 * punt.
	 */
	if (ioctl(fd, DIOCGDEFPT, (char *)&pt_struct) < 0) {
		perror("ioctl (DIOCGDEFPT)");
		fatal("%s: can't get default partition table", s);
	}
	for (p = 0; p < MAXPARTITIONS; p++) {
		pp = &lp->d_partitions[p]; 
		pp->p_offset = pt_struct.d_partitions[p].p_offset;
		pp->p_size   = pt_struct.d_partitions[p].p_size;
	}
	return;
}

struct disklabel *
getdisklabel(s, fd)
	char *s;
	int fd;
{
	static struct disklabel lab;
	struct disklabel *creatediskbyname();
	struct disklabel *lp;

#ifdef DEBUG
	printf("In getdisklabel\n");
#endif
	if (ioctl(fd, DIOCGDINFO, (char *)&lab) < 0) {
#ifdef i386
		if (get_disk_parms(fd, &lab)) {
			unlabelled++;
			return(&lab);
		}
#endif
#ifdef COMPAT
		unlabelled++;
		if (disktype) {
			struct disklabel *getdiskbyname();

#ifdef DEBUG
			printf("doing getdiskbyname\n");
#endif
			if (lp = getdiskbyname(disktype)) {
			/*
			 * If we have a label, then check for dynamic 
			 * geometry devices (like RAID) and update our 
			 * geometry information if we have a dynamic 
			 * geometry device.
			 */	
			if (lp->d_flags & D_DYNAM_GEOM) 
			    dyndiskcheck(lp, s, fd);
			return(lp);
			}
		}
		return (creatediskbyname(s));
#endif
#ifdef DEBUG
		printf("ioctl failed\n");
#endif
		return(NULL);
	}
#ifdef DEBUG
	printf("label returned\n");
#endif
	return (&lab);
}

rewritelabel(s, fd, lp)
	char *s;
	int fd;
	register struct disklabel *lp;
{
#if SEC_BASE && !defined(SEC_STANDALONE)
	privvec_t saveprivs;
#endif

#if	defined(COMPAT) || defined(i386)
	if (unlabelled)
		return;
#endif
	lp->d_checksum = 0;
	lp->d_checksum = dkcksum(lp);
#if SEC_BASE && !defined(SEC_STANDALONE)
	forceprivs(additional_privs, saveprivs);
#endif
	if (ioctl(fd, DIOCWDINFO, (char *)lp) < 0) {
		perror("ioctl (WDINFO)");
		fatal("%s: can't rewrite disk label", s);
	}
#if vax
	if (lp->d_type == DTYPE_SMD && lp->d_flags & D_BADSECT) {
		register i;
		int cfd;
		daddr_t alt;
		char specname[64];
		char blk[1024];
		char *cp;

		/*
		 * Make name for 'c' partition.
		 */
		strcpy(specname, s);
		cp = specname + strlen(specname) - 1;
		if (!isdigit(*cp))
			*cp = 'c';
		cfd = open(specname, O_WRONLY);
		if (cfd < 0) {
			perror(specname);
			exit(6);
		}
		bzero(blk, sizeof(blk));
		*(struct disklabel *)(blk + LABELOFFSET) = *lp;
		alt = lp->d_ncylinders * lp->d_secpercyl - lp->d_nsectors;
		for (i = 1; i < 11 && i < lp->d_nsectors; i += 2) {
			if (lseek(cfd, (off_t)(alt + i) * lp->d_secsize, L_SET) == -1) {
				perror("lseek to badsector area");
				exit(7);
			}
			if (write(cfd, blk, lp->d_secsize) < lp->d_secsize) {
				int oerrno = errno;
				fprintf(stderr, "alternate label %d ", i/2);
				errno = oerrno;
				perror("write");
			}
		}
		close(cfd);
	}
#endif
#if SEC_BASE && !defined(SEC_STANDALONE)
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
}
#endif	/* multimax */

/*VARARGS*/
fatal(fmt, arg1, arg2)
	char *fmt;
	caddr_t arg1, arg2;
{
	fprintf(stderr, "%s: ", progname);
	fprintf(stderr, fmt, arg1, arg2);
	putc('\n', stderr);
	exit(8);
}

#ifdef	multimax

#define BUFSIZE		((sizeof(layout_t) + DEV_BSIZE - 1) & ~(DEV_BSIZE -1))

char	layout_info[BUFSIZE];
char	headerdev[MAXPATHLEN];

getheaderinfo(rdev, pnum)
	char *rdev;
	int   pnum;
{
#if SEC_BASE && !defined(SEC_STANDALONE)
	privvec_t saveprivs;
#endif

	register layout_t 	*layout=(layout_t *)&layout_info[0];
				/* Pointer to the disk header layout */
	char *headerptr, *cp, *tcp; 
	int fd, err, p_size;
	
	/* 
	 * open the header parition of this disk. The 4th partition of 
	 * this disk, ie., rmdxd, will be used as header diskpartition. 
	 */
	headerptr = &headerdev[0];
	strcpy(headerptr, rdev);

	tcp = strchr(headerptr, '\0') - 1;
	cp = tcp; 

	if (isdigit(*tcp))
		*tcp ='3';
	else
		*tcp = 'd';
#if SEC_BASE && !defined(SEC_STANDALONE)
	forceprivs(additional_privs, saveprivs);
#endif
	fd = open(headerptr, O_RDONLY);   
	if (fd < 0) {
		if (fssize == 0) {
			perror(headerptr);
			exit(1);
		}
		return;
	}
#if SEC_BASE && !defined(SEC_STANDALONE)
	seteffprivs(saveprivs, (priv_t *) 0);
#endif

	if ((err = ioctl(fd, (int)MSIOCRDLAY, (char *)layout)) == -1) {
		if (fssize == 0) {
			err = errno;
			perror("READ HEADER PARTITION ERROR");
			exit(1);
		}
		return;
	}

	if ((p_size = layout->partitions[pnum].part_size) == 0)
		fatal("%s: `%c' partition is unavailable", rdev, *cp);
	if (fssize == 0)
		fssize = p_size;
	if (fssize > p_size && !mfs)
	       fatal("%s: maximum file system size on the `%c' partition is %d",
			rdev, *cp, p_size);
	/* 
	 * NOTE: Multimax disk header does not provide the following info: 
	 *  - rpm
	 *  - trackskew
	 *  - interleave
	 *  - headswitch
	 *  - trackseek
	 *  - cylspares
	 *  - trackspares
	 *  - secpercyl
	 * These values are all defaulted by the caller of this
	 * routine if not provided by the user.
	 */
	if (ntracks == 0) {
		ntracks = layout->lay_geom.track_cylinder;
		if (ntracks <= 0)
			fatal("%s: no default #tracks", rdev);
	}

	if (nsectors == 0) {
		nsectors = layout->lay_geom.sector_track;
		if (nsectors <= 0)
			fatal("%s: no default #sectors/track", rdev);
	}

	if (sectorsize == 0) {
		sectorsize = layout->lay_geom.byte_sector;
		if (sectorsize <= 0)
			fatal("%s: no default sector size", rdev);
	}

	if (fsize == 0) 
		fsize = MAX(DFL_FRAGSIZE, layout->lay_geom.byte_sector);

	close(fd);
}

#endif	/* multimax */

#ifdef i386 /* really should be more specific than this */

/* The following #define is due to an unfortunate namespace
 * collision between this disk driver and the disklabel code above.
 */
#define partition PaRtItIoN

#include <i386/disk.h>

#undef partition

int
get_disk_parms(fd, lp)
int fd;
struct disklabel *lp;
{
struct disk_parms disk_parms;

	if (ioctl(fd, V_GETPARMS, &disk_parms) < 0) {
		return(0);
	}
	/*
	 * Fake up the contents of a disk label so that the
	 * caller will be happy enough to create a filesystem.
	 */
	lp->d_magic	= DISKMAGIC;
	lp->d_type = (disk_parms.dp_type==DPT_WINI)?DTYPE_ESDI:DTYPE_FLOPPY;
	lp->d_subtype	= 0;	/* no clue */
	strcpy(dktypenames, lp->d_typename);
	bzero(lp->d_packname, 16);
	lp->d_secsize	= disk_parms.dp_secsiz;	/* size of a sector (bytes) */
	lp->d_nsectors	= disk_parms.dp_sectors;/* sectors per track */
	lp->d_ntracks	= disk_parms.dp_heads;	/* tracks per cylinder */
	lp->d_ncylinders = disk_parms.dp_cyls;	/* total # of cylinders */
	lp->d_secpercyl = lp->d_nsectors * lp->d_ntracks;
	lp->d_secperunit = 0;			/* will be computed */
	lp->d_sparespertrack = 0;
	lp->d_sparespercyl = 0;
	lp->d_acylinders = 0;
	lp->d_rpm	= 0;				/* will be defaulted */
	lp->d_interleave = 1;	 /* This disk doesn't provide this info */
	lp->d_cylskew	= 0;	 /* This disk doesn't provide this info */
	lp->d_headswitch = 0;	 /* This disk doesn't provide this info */
	lp->d_trkseek	= 0;	 /* This disk doesn't provide this info */
	lp->d_flags = (lp->d_type==DTYPE_FLOPPY)?D_REMOVABLE:0;
	bzero(lp->d_drivedata, sizeof(u_int) * NDDATA);
	bzero(lp->d_spare, sizeof(u_int) * NSPARE);
	lp->d_magic2 = DISKMAGIC;
	lp->d_checksum = 0;

	lp->d_npartitions = 1;
	lp->d_bbsize = BBSIZE;	/* meaningless on this disk */
	lp->d_sbsize = SBSIZE;
	lp->d_partitions[0].p_size = disk_parms.dp_pnumsec;
	lp->d_partitions[0].p_offset = 0;	/* always */
	lp->d_partitions[0].p_fsize = 0; /* default or set from argv */
	lp->d_partitions[0].p_fstype = FS_UNUSED;/* we just don't know */
	lp->d_partitions[0].p_frag = 0;	/* will be overridden */
	lp->d_partitions[0].p_cpg = 0;	/* unused anyway */

	partno = 0;

	return(1);
}
#endif	/* i386 */

#ifdef	DEBUG
dumplabel(lp)
register struct disklabel *lp;
{
register struct partition *pptr;
register int i;

	if (lp == 0)
		return;
	printf("d_magic: %x\n", lp->d_magic);
	printf("d_type: %d\n", lp->d_type);
	printf("d_subtype: %d\n", lp->d_subtype);
	printf("d_typename: %16s\n", lp->d_typename);
	printf("d_secsize: %d\n", lp->d_secsize);
	printf("d_nsectors: %d\n", lp->d_nsectors);
	printf("d_ntracks: %d\n", lp->d_ntracks);
	printf("d_ncylinders: %d\n", lp->d_ncylinders);
	printf("d_secpercyl: %d\n", lp->d_secpercyl);
	printf("d_secperunit: %d\n", lp->d_secperunit);
	printf("d_rpm: %d\n", lp->d_rpm);
	printf("d_npartitions: %d\n", lp->d_npartitions);
	for (i=0; i< lp->d_npartitions; i++) {
		pptr = &(lp->d_partitions[i]);
		printf("partition %d\n", i);
		printf("        p_size %d\n", pptr->p_size);
		printf("        p_offset %d\n", pptr->p_offset);
		printf("        p_fsize %d\n", pptr->p_fsize);
		printf("        p_fstype %d\n", pptr->p_fstype);
		printf("        p_frag %d\n", pptr->p_frag);
		printf("        p_cpg %d\n", pptr->p_cpg);
		printf("\n");
	}
}
#endif	/* DEBUG */

char * itoa (x,itoa_string)
int x;
char itoa_string[];
{
  char tmp[30];
  int y=0;
  int z=0;
  int foo;

  for (; tmp[y++] = x % 10, (x = x/10) > 0; );

  while (--y >= 0)
    itoa_string[z++] = tmp[y] + '0';

  itoa_string[z] = 0;

  return (itoa_string);
}

islv(lv_path, fssize)
   char *lv_path;
   int *fssize;
{
   char *vg_name;
   char buf[PATH_MAX];
   char wdir[PATH_MAX];
   register int c;
   register int src_i, dst_i;
   register char *cp;
   int vg_fd;
   struct stat st;
   struct lv_querylv querylv;
   struct lv_queryvg queryvg;

/*
 *   int islv(char *lv_path, int *fssize)
 *
 *      ULTRIX/OSF:  added this function
 *
 *      Function for dealing with logical devices vs partitions;
 *      currently optimized for LVM devices.
 *
 *      Tries to determine if the name passed in is a logical
 *      volume name (vs a UNIX-type partition name).  It does this
 *      by checking that the minor number is acceptable, by
 *      constructing a volume group name, and then trying
 *      to open the volume group.
 *
 *      If the above looks good, then the function checks if it
 *      needs to retrieve the logical volume size and set fssize.
 *
 *      Returns TRUE if the passed in path is a logical volume
 *      otherwise returns FALSE.
 */

   /* Check for range of minor number */
   
   if (stat(lv_path, &st) < 0) {
      return(FALSE);
   }
   if ( ! ( minor(st.st_rdev) >= 1 && minor(st.st_rdev) <= LVM_MAXLVS ) ) {
      return(FALSE);
   }

   /* Skip sequences of "./" */
   while (lv_path[0] == '.' && lv_path[1] == '/')
      lv_path += 2;

   /* Remove sequences of "///", and "/./." while copying lv_path into buf */
   for (src_i = dst_i = 0; (c = lv_path[src_i]) != '\0'; ) {
      buf[dst_i++] = c;
      src_i++;
      if (c == '/') {
	 while (lv_path[src_i] == '/')
	    src_i++;

	 while (lv_path[src_i] == '.' && lv_path[src_i + 1] == '/')
	    src_i += 2;
      }
   }
   buf[dst_i] = '\0';

   /*
    *   Now, figure out what sort of VG does this belong to.
    *   Let vg_name point to something like "/dev/vgXX/lv...";
    *   later on we cut it to "/dev/vgXX", and allocate space for 
    *   the string to be returned.
    */

   /* First, see if it is an absolute pathname */
   if (strncmp(buf, "/dev/", sizeof("/dev/") - 1) == 0) 
      vg_name = buf;
   else {
      
      /*
       *   Second, it might be just the name; something like "lv03", or
       *   "vgXX/lv03"
       */

      if (isalnum(buf[0])) {

	 /* Are we under "/" or "/dev" or "/dev/vgXX"? */
	 if (getcwd(wdir, sizeof(wdir)) == NULL) {
	    /* can't figure out volume group name from file name input */
	    return(FALSE);
	 }
	 strcat(wdir, buf);
	 vg_name = wdir;
      }
      else {

	/*
	 * Bad thing: something like (cwd: /dev/vgYY) ../vgXX/lv03
	 * can't figure out volume group name from file name input
	 */
	 return(FALSE);
      }
   }

   /* Clean it up, and store it back; count the '/' (should be 3) */
   for (cp = vg_name, c = 0; *cp != '\0' && c != 3; )
      if (*cp++ == '/')
	 c++;

   /* Sanity check */
   if (c != 3) {
      /*
       * can't figure out volume group name still
       * This check catches most /dev/rrxnx partition names 
       */
      return(FALSE);
   }

   /* 
    * Now attach the "group" part of the vg name
    * We write over the last section of the lv name
    */
   strcpy(cp,"group");

   /* open the VG control file (final check) */
   if ((vg_fd = open(vg_name, O_RDONLY)) == -1) {

       /* 
	* Oops, something went wrong, or again we don't have a 
	* logical volume! 
	*/
       return(FALSE);
   }

   /* 
    * Now check if the filesystem size has already
    * been set.  If not we need to retrieve and set 
    * that information.
    */
   if (*fssize == 0) {
       querylv.minor_num = minor(st.st_rdev);
       /* 
	* Make system calls to get logical volume and
	* volume group information.
	*/
       if ( (ioctl(vg_fd,LVM_QUERYLV,&querylv) == 0) &&
	    (ioctl(vg_fd,LVM_QUERYVG,&queryvg) == 0)   ) {
           
	   /* 
	    * querylv.numlxs = current number of logical extents
	    * queryvg.pxsize = extent size in bytes
	    * calculate fssize = filesystem size in sectors
	    */
	   *fssize = (queryvg.pxsize / DEV_BSIZE) * querylv.numlxs;
        } else {
	   /*
	    * ioctl call failed because vg_fd does not refer to a logical
	    * volume. Close down the file descriptor and return FALSE.
	    */
	   close(vg_fd);
	   return(FALSE);
	}
   }
   close(vg_fd);   
   return(TRUE);
}

#define DEV_NAME_LSM	"LSM"
#define DEV_NAME_LVM	"LVM"

/*
 * Detect overlapping mounted partitions.
 *
 * This code is also used almost verbatim in usr/sbin/mount/mount.c,
 * usr/sbin/swapon/swapon.c, and usr/sbin/ufs_fsck/setup.c
 * Change all places!
 * (XXX maybe we should pull this out into a shared source file?)
 *
 * XXX - Only 8 partitions are used in overlap detection
 */

devget_to_partid(fd, devget, partid)
	int fd;
	struct devget *devget;
	struct ufs_partid *partid;
{
#ifdef DEBUG
    printf("devget_to_partid\n");
#endif
    bzero(partid, sizeof(*partid));
    if (strncmp(devget->dev_name, DEV_NAME_LVM, strlen(DEV_NAME_LVM)) == 0 ||
	strncmp(devget->dev_name, DEV_NAME_LSM, strlen(DEV_NAME_LSM)) == 0)
    {
	struct stat info;

	if (fstat(fd, &info) < 0)
		return(-1);
	partid->lvm_or_lsm_dev = info.st_rdev;
#ifdef DEBUG
	printf("lvm/lsm partition %d, %d\n",
		major(partid->lvm_or_lsm_dev),
		minor(partid->lvm_or_lsm_dev));
#endif
    } else
    {
        partid->bus = devget->bus;
        partid->adpt_num = devget->adpt_num;
        partid->nexus_num = devget->nexus_num;
        partid->bus_num = devget->bus_num;
        partid->ctlr_num = devget->ctlr_num;
        partid->rctlr_num = devget->rctlr_num;
        partid->slave_num = devget->slave_num;
        partid->unit_num = devget->unit_num;
        partid->category_stat = devget->category_stat & (MAXPARTITIONS - 1);
    }
    return (0);
}

print_partid(device, partid)
	char *device;
	struct ufs_partid *partid;
{
	int ind;

	printf("device %s\n", device);
	if (partid->lvm_or_lsm_dev)
		printf("\tLVM/LSM %d, %d\n",
			major(partid->lvm_or_lsm_dev),
			minor(partid->lvm_or_lsm_dev));
	else
	{
		printf("\tbus %d\n", partid->bus);
		printf("\tadpt_num %d\n", partid->adpt_num);
		printf("\tnexus_num %d\n", partid->nexus_num);
		printf("\tbus_num %d\n", partid->bus_num);
		printf("\tctlr_num %d\n", partid->ctlr_num);
		printf("\trctlr_num %d\n", partid->rctlr_num);
		printf("\tslave_num %d\n", partid->slave_num);
		printf("\tunit_num %d\n", partid->unit_num);
		printf("\tcategory_stat %d\n", partid->category_stat);
		for (ind = 0; ind < MAXPARTITIONS; ind++)
			printf("\tpt_offset %d length %d\n",
		       		partid->pt_part[ind].part_blkoff,
		       		partid->pt_part[ind].part_nblocks);
	}
	printf("\n");
}

#define PT_MAGIC        0x032957        /* Partition magic number */
#define PT_VALID        1               /* Indicates if struct is valid */

/*
 * Structure that is used to determine the partitioning of the disk.
 * It's location is at the end of the superblock area.
 * The reason for both the cylinder offset and block offset
 * is that some of the disk drivers (most notably the uda
 * driver) require the block offset rather than the cyl.
 * offset.
 */
struct ult_pt {
	int     pt_magic;       /* magic no. indicating part. info exits */
	int     pt_valid;       /* set by driver if pt is current */
	struct  pt_info {
		int     pi_nblocks;     /* no. of sectors for the partition */
		daddr_t pi_blkoff;      /* block offset for start of part. */
	} pt_part[8];
};

ultrix_style(device, partid, dl)
	char *device;
	struct ufs_partid *partid;
	struct disklabel *dl;
{
	struct fs *fs;
	struct ult_pt *pt;
	char buf[32];
	char sb[SBSIZE];
	int len, ind, fd;

	len = strlen(device);
	strcpy(buf, device);
	buf[len - 1] = 'c';

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return(-1);
	lseek(fd, SBOFF, 0);
	if (read(fd, sb, SBSIZE) != SBSIZE) {	
		close(fd);
		return(-1);
	}
	close(fd);

	fs = (struct fs *)sb;
	if (fs->fs_magic != FS_MAGIC)
		return(-1);

	pt = (struct ult_pt *)&sb[8192 - sizeof(struct ult_pt)];
	if (pt->pt_magic != PT_MAGIC || pt->pt_valid != PT_VALID)
		return(-1);

	/*
	 * Valid ULTRIX Super block and partition table
	 */
	for (ind = 0; ind < 8; ind++) {
		dl->d_partitions[ind].p_offset = pt->pt_part[ind].pi_blkoff;
		dl->d_partitions[ind].p_size = pt->pt_part[ind].pi_nblocks;
	}
	return(0);
}

/*
 * Return -1 if partid could not be initialized.
 * Rerurn 0 if partid was initialized.
 */
init_partid(file, partid)
	char *file;
	struct ufs_partid *partid;
{
	int fd, ind;
	struct devget devget;
	struct disklabel disklabel, *dl;
	

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return(-1);
	/*
	 * Call drive to fill in devget struct
	 */
	if (ioctl(fd, DEVIOCGET, (char *)&devget) < 0) {
		close(fd);
		return(-1);
	}
	if (devget_to_partid(fd, &devget, partid) < 0)
	{
		close(fd);
		return(-1);
	}

	/*
	 * If logical volume then we are finished.
	 */
	if (partid->lvm_or_lsm_dev)
	{
		close(fd);
		return(0);
	}	

	/*
	 * Get partition table info for drive:
	 *
	 *	Check for a disklabel
	 * 	Check for ULTRIX style partition table
	 *	Use disktab
	 */
	dl = &disklabel;
	if (ioctl(fd, DIOCGDINFO, dl) < 0 && ultrix_style(file, partid, dl) < 0)
		dl = (struct disklabel *)getdiskbyname(devget.device);
	close(fd);
	
	/*
	 * If no partition table found, no testing possible.
	 */
	if (!dl)
		return(-1);

	for (ind = 0; ind < MAXPARTITIONS; ind++) {
		partid->pt_part[ind].part_blkoff = dl->d_partitions[ind].p_offset;
		partid->pt_part[ind].part_nblocks = dl->d_partitions[ind].p_size;
	}
	return(0);
}

/*
 * Code to form the swap partition name lifted from usr/sbin/swapon/swapon.c.
 * Code to call table() inspired by swapon.c
 *
 * verify_ok() is common code to both swap checking and mounted file checking
 * code; it formerly was inline in check_for_overlap().
 *
 * This code is also used almost verbatim in usr/sbin/mount/mount.c.
 * Change both places!
 * (XXX maybe we should pull this out into a shared source file?)
 */

static  char   *swap_partition_name(dev_t);
static	int    verify_ok(struct ufs_partid *,
			 const char *,
			 const char *,
			 const char *,
			 const char *);

/*
 * Return non zero if overlap.
 */
check_for_overlap(device, partid)
	char *device;
	struct ufs_partid *partid;
{
	struct statfs *mntbuf;
	int i, mntsize, ret;

	ret = 0;
	/* Get swap info & verify non-overlapping; code lifted from swapon.c */
	for (i = 0;; i++) {
	    struct tbl_swapinfo swapinfo;
	    char *name;

	    /*
	     ** Attempt to read the swap information.
	     */
	    if (table(TBL_SWAPINFO, i, &swapinfo, 1, sizeof(swapinfo)) < 0)
		break;

	    /*
	     ** check this swap partition.
	     */
	   
	    if ((name = swap_partition_name(swapinfo.dev)) == NULL) {
		/* Name is not in a standard place. */	
		continue; 
	    }
	    ret += verify_ok(partid, device, name, "swap", "in use as");
	}

	/* Check all the mounted UFS filesystems too: */

	if ((mntsize = getmntinfo(&mntbuf, MNT_NOWAIT)) == 0)
		return(ret);
	for (i = 0; i < mntsize; i++) {
		if (mntbuf[i].f_type != MOUNT_UFS)
			continue;
		ret += verify_ok(partid, device, mntbuf[i].f_mntfromname,
				 mntbuf[i].f_mntonname, "mounted on");
	}
	return(ret);
}

/*
 * Return 0 if no overlap or if we could not tell
 *	if overlapped or not.
 * Return 1 if overlap
 *
 * WARNING:
 *	THIS CODE DOES NOT DETECT AN ATTEMPT TO USE
 *	A PARTITION WHICH IS USE BY THE LVM.
 *
 *	THIS CODE DOES NOT DETECT AN ATTEMPT TO USE
 *	A PARTITION WHICH WOULD OVERLAP A PARTITION
 *	WHICH IS IN USE BY THE LVM.
 *
 *	AT PRESENT NO EASY WAY EXISTS TO PERFORM
 *	THESE TESTS
 */  
static int
verify_ok(struct ufs_partid *partid,
	  const char *device,
	  const char *blockdev,
	  const char *usename,
	  const char *msg)
{
    struct devget mnt_devget;
    struct ufs_partid mnt_partid;
    int fd, ret = 0;
    long bot, top, mbot, mtop;
    char *raw = rawname(blockdev);

    fd = open(raw, O_RDONLY);
    if (fd < 0)
	return (0);
    /*
     * Call drive to fill in devget struct
     */
    if (ioctl(fd, DEVIOCGET, (char *)&mnt_devget) < 0) {
	close(fd);
	return (0);
    }

    /*
     * Extract what we need
     */
    if (devget_to_partid(fd, &mnt_devget, &mnt_partid) < 0)
    {
	close(fd);
	return(0);
    }
    close(fd);

    /*
     * Either one a logical volume ?
     */
    if (mnt_partid.lvm_or_lsm_dev || partid->lvm_or_lsm_dev)
    {
	/*
	 * Same logical volume ?
	 */
    	if (mnt_partid.lvm_or_lsm_dev == partid->lvm_or_lsm_dev)
	{
	    printf("New filesystem would overlap mounted filesystem(s) or active swap area\n");
	    printf("Unmount required before creating %s\n", device);
		return (1);
	}
	/*
	 * Assume no overlap if different logical volume or
	 * only one logical volume.
	 */
	return(0);	
    }
    
    /*
     * Check for drive match
     */
    if (mnt_partid.bus != partid->bus ||
	mnt_partid.adpt_num != partid->adpt_num ||
	mnt_partid.nexus_num != partid->nexus_num ||
	mnt_partid.bus_num != partid->bus_num ||
	mnt_partid.ctlr_num != partid->ctlr_num ||
	mnt_partid.rctlr_num != partid->rctlr_num ||
	mnt_partid.slave_num != partid->slave_num ||
	mnt_partid.unit_num != partid->unit_num)
	return (0);

    bot = partid->pt_part[partid->category_stat].part_blkoff;
    top = bot + partid->pt_part[partid->category_stat].part_nblocks - 1;

    mbot = partid->pt_part[mnt_partid.category_stat].part_blkoff;
    mtop = mbot + partid->pt_part[mnt_partid.category_stat].part_nblocks - 1;
    /*
     * If same partition or partitions overlap,
     * return true
     */
    if (top > 0 && mtop > 0 &&
	(mnt_partid.category_stat == partid->category_stat ||
	 bot >= mbot && bot <= mtop ||
	 top >= mbot && top <= mtop ||
	 mbot >= bot && mbot <= top)) {
	if (ret++ == 0) {
	    printf("New filesystem would overlap mounted filesystem(s) or active swap area\n");
	    printf("Unmount required before creating %s (start %d end %d)\n", device, bot, top);
	}
	printf("\t%s %s %s (start %d end %d)\n",
	       blockdev, msg, usename, mbot, mtop);
    }
    return (ret);
}

static char *
swap_partition_name(dev_t dev)
{
  static char device_name[256+4+1];     /* `/dev/file-name-up-to-255-characters'<NUL> */
  DIR *dir;
  struct dirent *dirent;

  /*
  ** Find the device in the /tmp directory.
  */
  if (!(dir = opendir("/dev")))
    return (NULL);

  /*
  ** Directory is accessed, read through each name in `/dev', and
  ** stat the file to obtain the (possible) major/minor numbers.
  ** When we have a block device which matches, return its name.
  */
  while (dirent = readdir(dir)) {
    /*
    ** Got some directory information.  Check for a block special
    ** with matching major and minor numbers.  If (when) found,
    ** return a pointer to the generated name.
    */
    struct stat local_stat;

    sprintf(device_name, "/dev/%s", dirent->d_name);
    if (!stat(device_name, &local_stat)) {
      if (S_ISBLK(local_stat.st_mode) &&
          major(local_stat.st_rdev) == major(dev) &&
          minor(local_stat.st_rdev) == minor(dev)) {
        closedir(dir);
        return device_name;
        }
      }
    }
  closedir(dir);
  return (NULL);
  }

int
islsm(lsm_path, volsize)
char *lsm_path;
int *volsize;
{
   int lsm_fd;
   DEVGEOMST volgeom;
   struct devget devget;
   struct stat st;


#ifdef DEBUG
	printf("Inside islsm: \n");
#endif DEBUG

   /* Check if the device exists */
   
   if (stat(lsm_path, &st) < 0) {
      return(FALSE);
   }

   /* open the volume */
   if ((lsm_fd = open(lsm_path, O_RDONLY)) == -1) {
       return(FALSE);
    }
#ifdef DEBUG
	printf("Inside islsm: Successfully opened %s \n", lsm_path);
#endif DEBUG
   if ( ioctl(lsm_fd, DEVIOCGET , (char *)(&devget)) < 0){
		close(lsm_fd);
		return(FALSE);
   }

   if(devget.dev_name &&
	(strncmp(devget.dev_name, DEV_NAME_LSM, strlen(DEV_NAME_LSM)) == 0)){
#ifdef DEBUG
	printf("Inside islsm: %s is a LSM volume name \n", lsm_path);
#endif DEBUG
   	}
   else{
        close(lsm_fd);
        return(FALSE);
   }

   /* 
    * Now check if the filesystem size has already
    * been set.  If not we need to retrieve and set 
    * that information.
    */
   if (*volsize == 0) {
       /* 
	* Make ioctl call to get LSM volume length.
	*/
       if ( ioctl(lsm_fd, DEVGETGEOM , (char *)(&volgeom)) == 0) {
           
#ifndef lint  /* lint warns about assigning longs to ints */
		*volsize = (int) volgeom.geom_info.dev_size;
#endif lint

#ifdef DEBUG
		printf("Inside islsm: Size of LSM volume %s is %d \n", 
			lsm_path, *volsize);
#endif DEBUG
        } else {
	   /*
	    * ioctl call failed because lsm_fd does not refer to a LSM
	    * volume. Close down the file descriptor and return FALSE.
	    */
	   close(lsm_fd);
	   return(FALSE);
	}
   }
   close(lsm_fd);   
   return(TRUE);
}
#define	LSM_CDEV	"rvol"
#define	LSM_CDEV_LEN	4
#define	LSM_BDEV	"vol"
#define	LSM_BDEV_LEN	3

char *
unrawname(name)
	char *name;
{
	char *dp;
	char *ddp;
	char *p;
	struct stat stb;

	if (stat(name, &stb) < 0)
		return (name);
	if ((stb.st_mode & S_IFMT) != S_IFCHR)
		return (name);

	for (;;)	/* just so we can use break */
	{
		/* see if any '/' */
		if ((dp = rindex(name, '/')) == 0)
		{
			dp = name-1;	/* simple name */
			break;
		}

		/* look for a second slash */
		p = dp-1;
		ddp = 0;
		while (p >= name)
		{
			if (*p == '/')
			{
				ddp = p;
				break;
			}
			p--;
		}
		if (ddp)
		{
			/* look for "rvol" */
			p = ddp - LSM_CDEV_LEN;
			/* is place we are looking at valid ? */
			if (p == name || (p > name && p[-1] == '/'))
			{
				/* actually look for string */
				if (strncmp(p, LSM_CDEV, LSM_CDEV_LEN) == 0)
				{
					dp = p-1; /* name preceeded by rvol/xxxx/ */
					break;
				}
			}
		}

		/* look for "rvol" */
		p = dp - LSM_CDEV_LEN;
		/* is place we are looking at valid ? */
		if (p == name || (p > name && p[-1] == '/'))
		{
			/* actually look for string */
			if (strncmp(p, LSM_CDEV, LSM_CDEV_LEN) == 0)
			{

				dp = p-1; /* name preceeded by rvol/ */
				break;
			}
		}

		break;
	}
	
	if (*(dp + 1) != 'r')
		return (name);
	(void)strcpy(dp + 1, dp + 2);
	return (name);
}

char *
rawname(name)
	char *name;
{
	static char rawbuf[MAXPATHLEN];
	char *dp;
	char *ddp;
	char *p;

	for (;;)	/* just so we can use break */
	{
		/* see if any '/' */
		if ((dp = (char *)rindex(name, '/')) == 0)
		{
			dp = name-1;	/* a simple name */
			break;
		}

		/* look for a second '/' */
		p = dp-1;
		ddp = 0;
		while (p >= name)
		{
			if (*p == '/')
			{
				ddp = p;
				break;
			}
			p--;
		}
		if (ddp)
		{
			/* look for "vol" */
			p = ddp - LSM_BDEV_LEN;
			/* is place we are looking at valid ? */
			if (p == name || (p > name && p[-1] == '/'))
			{
				/* actually look for string */
				if (strncmp(p, LSM_BDEV, LSM_BDEV_LEN) == 0)
				{
					dp = p-1; /* name preceeded by vol/xxxx/ */
					break;
				}
			}
		}

		/* look for "vol" */
		p = dp - LSM_BDEV_LEN;
		/* is place we are looking at valid ? */
		if (p == name || (p > name && p[-1] == '/'))
		{
			/* actually look for string */
			if (strncmp(p, LSM_BDEV, LSM_BDEV_LEN) == 0)
			{
				dp = p-1; /* a name preceeded by vol/ */
				break;
			}
		}

		break;
	}
	
	dp++;
	memcpy(rawbuf, name, dp - name);
	strcpy(rawbuf + (dp-name), "r");
	strcat(rawbuf, dp);

	return (rawbuf);
}

