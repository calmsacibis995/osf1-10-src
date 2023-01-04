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
static char	*sccsid = "@(#)$RCSfile: swapon.c,v $ $Revision: 4.3.14.4 $ (DEC) $Date: 1994/01/10 18:37:17 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <strings.h>
#include <fstab.h>
#include <errno.h>
#include <sys/secdefines.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <sys/table.h>

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#if defined(NLS) || defined(KJI)
#define	NLSKJI 1
#include <NLctype.h>
#include <NLchar.h>
#endif
#include <locale.h>

#ifdef MSG
#include "swapon_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) NLcatgets(catd,MS_SWAPON,Num,Str)
#else
#define MSGSTR(Num,Str) Str
#endif

#define	MS_NONE		0
#define	MS_PREFER	1

extern char *rawname();

char *program;

static  int	add();
static	void	usage();
static  int	numarg();
static  void    show_paging_files();
static  char   *swap_partition_name(dev_t);

void
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	register struct fstab *fsp;
	int ch;
	int verbose = 0, doall = 0, doshow = 0;
	unsigned int flags;
	int lowat, hiwat;
	int stat;
	int not_supported = 0;

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("mount")) {
		fprintf(stderr, "%s: need mount authorization\n", command_name);
		exit(1);
	}
	if (forceprivs(privvec(SEC_MOUNT, -1), (priv_t *) 0)) {
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
#endif
	if (program = rindex(argv[0], '/'))
		program++;
	else
		program = argv[0];

	flags = MS_NONE;
	lowat = 20 * 1024 * 1024;
	hiwat = 0;	/* unbounded growth */

	while ((ch = getopt(argc, argv, "avpl:h:s")) != EOF)
		switch((char)ch) {
		case 'a':
			doall = 1;
			break;
                case 'v':
                        verbose = 1;
                        break;
                case 'p':
			not_supported = 1;
			flags |= MS_PREFER;
                        break;
                case 'l':
			not_supported = 1;
			lowat = numarg(optarg);
                        break;
                case 'h':
			not_supported = 1;
			hiwat = numarg(optarg);
                        break;
		case 's':
			doshow = 1;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;

	if (not_supported)
		fprintf(stderr, MSGSTR(NOT_SUPPORTED, "%s: warning: options 'p', 'l', and 'h' not supported.\n"), program);
        
	stat = 0;
	if (doall)
		while (fsp = getfsent()) {
			if (strcmp(fsp->fs_type, FSTAB_SW))
				continue;
			if (add(fsp->fs_spec, flags, lowat, hiwat, 1))
				stat = 1;
			else
				printf(MSGSTR(VERBOSE1, "%s: adding %s as swap device.\n"),
                                       program, fsp->fs_spec);
		}
	else if (!*argv && !doshow)
		usage();

	for (; *argv; ++argv) {
                if (verbose)
			printf(MSGSTR(VERBOSE1, "%s: adding %s as swap device.\n"),
                                       program, *argv); 
		stat |= add(*argv, flags, lowat, hiwat, 0);
        }

	/*
	** If -s is specified, show that now.
	*/
	if (doshow)
	  show_paging_files();
	exit(stat);
}

static int
numarg(arg)
    char *arg;
{
	int i, len = strlen(arg);
	int size = 0;
	int	page_size;

	page_size = getpagesize();
	for (i = 0; i < len; i++) {
		char c = arg[i];

		if (('0' <= c) && (c <= '9')) {
			size = 10*size + (c - '0');
			continue;
		}

		if ((i == 0) || (i != len-1))
			usage();

		switch (c) {
		case 'p': case 'P':
			size *= page_size;
			break;
		case 'k': case 'K':
			size *= 1024;
			break;
		case 'm': case 'M':
			size *= 1024*1024;
			break;
		case 'g': case 'G':
			size *= 1024*1024*1024;
			break;
		default:
			usage();
		}
	}
	return (size);
}

static int
add(name, flags, lo, hi, ignoreebusy)
	char *name;
	int flags, lo, hi, ignoreebusy;
{
	extern int errno;

#if SEC_BASE
	disablepriv(SEC_SUSPEND_AUDIT);
#endif

	if (check_for_overlap(name))
	    return (1);
	if (swapon(name, flags, lo, hi) == -1) {
		switch (errno) {
		case EINVAL:
			fprintf(stderr,
                                MSGSTR(NO_DEV, "%s: %s: device not configured\n"),
                                program, name);
			break;
		case EBUSY:
			if (!ignoreebusy)
				fprintf(stderr,
				    MSGSTR(IN_USE, "%s: %s: device already in use\n"),
				     program, name);
			break;
		default:
			fprintf(stderr, "%s: %s: ", program, name);
			perror((char *)NULL);
			break;
		}
		return(1);
	}
	set_dump_device(name);
	return(0);
}

set_dump_device(name)
char *name;
{
    int err;
    dev_t	system_dumpdev = (dev_t)0;
    struct stat	dumpdev_stat;

    if ((err = getsysinfo(GSI_DUMPDEV, &system_dumpdev, sizeof(dev_t), 0, 0, 0)) < 0)
	return;
    if (system_dumpdev != NODEV)
	return;
    if (stat(name, &dumpdev_stat) != 0)
	return;
    if (S_ISCHR(dumpdev_stat.st_mode) || S_ISBLK(dumpdev_stat.st_mode)) {
	system_dumpdev = dumpdev_stat.st_rdev;
	if ((err = setsysinfo(SSI_DUMPDEV, &system_dumpdev, 0, 0, 0)) != 0)
	    return;
    }
    return;
}

static void
show_paging_files()
{
  int index;

  for (index = -1;; index++) {
    dev_t  default_dev;
    ino_t  default_ino;
    struct tbl_swapinfo swapinfo;
    char   swapsize[32];
    int    value;

    /*
    ** Attempt to read the swap information.
    */
    if (table(TBL_SWAPINFO, index, &swapinfo, 1, sizeof(swapinfo)) < 0)
      return;

    /*
    ** Calculate the swapsize for display.
    */
    value = (getpagesize()/1024) * swapinfo.size;
    if (value > 1024)
      sprintf(swapsize, "%3dMB", value/1024);
    else
      sprintf(swapsize, "%3dKB", value);
    if (index == -1) {
      /*
      ** Show the general swap information first.
      */
      printf("Total swap allocation:\n");
      printf("    Allocated space:   %10d pages (%s)\n",
		swapinfo.size,
		swapsize);
      printf("    Reserved space:    %10d pages (%3d%%)\n",
		swapinfo.size - swapinfo.free,
		(swapinfo.size == 0) ? 0 :
		((swapinfo.size - swapinfo.free)*100+50)/swapinfo.size);
      printf("    Available space:   %10d pages (%3d%%)\n",
		swapinfo.free,
		(swapinfo.size == 0) ? 0 :
		(swapinfo.free*100+50)/swapinfo.size);
      printf("\n");

      /*
      ** Record the default swap returned on the -1 call.
      ** This is checked later on for determining the swap
      ** mode (eager vs. lazy) for display.  When the index
      ** returns a matching dev_t/ino_t combination, the
      ** message `(default swap)' will be appended to the
      ** partition name.
      */
      default_dev = swapinfo.dev;
      default_ino = swapinfo.ino;
      }
    else {
      /*
      ** Show each individual swap partition.
      */
      printf("Swap partition %s", swap_partition_name(swapinfo.dev));
      if (swapinfo.dev == default_dev &&
          swapinfo.ino == default_ino)
        printf(" (default swap)");
      printf(":\n");
      printf("    Allocated space:   %10d pages (%s)\n",
		swapinfo.size,
		swapsize);
      printf("    In-use space:      %10d pages (%3d%%)\n",
		swapinfo.size - swapinfo.free,
		(swapinfo.size == 0) ? 0 :
		((swapinfo.size - swapinfo.free)*100+50)/swapinfo.size);
      printf("    Free space:        %10d pages (%3d%%)\n",
		swapinfo.free,
		(swapinfo.size == 0) ? 0 :
		(swapinfo.free*100+50)/swapinfo.size);
      printf("\n");
      }
    }
  }

static char *
swap_partition_name(dev_t dev)
{
  static char device_name[256+4+1];     /* `/dev/file-name-up-to-255-characters'<NUL> */
  DIR *dir;
  struct dirent *dirent;

  /*
  ** Find the device in the /dev directory.
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
        return (device_name);
        }
      }
    }
  closedir(dir);
  return (NULL);
  }

static void
usage()
{
	fprintf(stderr, MSGSTR(USAGE, "usage: %s [-avps] [-l size] [-h size] filename\n"), program);
	exit(1);
}

/*
 * routines used to see if partitions overlap, i.e. we don't want to
 * simultaneously mount overlapping partitions.  this code is 'borrowed'
 * from newfs/newfs.c:
 *	ultrix_style()
 *	init_partid()
 *	check_for_overlap()
 *	verify_ok()
 * Detect overlapping mounted partitions.
 *
 * This code is also used almost verbatim in usr/sbin/newfs/newfs.c,
 * usr/sbin/mount/mount.c, and usr/sbin/ufs_fsck/setup.c
 * Change all places!
 * (XXX maybe we should pull this out into a shared source file?)
 *
 *
 * XXX - Only 8 partitions are used in overlap detection for ultrix style,
 *       otherwise MAXPARTITIONS are used.
 */
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <ufs/fs.h>
#include <fcntl.h>
#include <sys/mount.h>

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

#define DEV_NAME_LSM	"LSM"
#define DEV_NAME_LVM	"LVM"

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
	int	pt_magic;       /* magic no. indicating part. info exits */
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
 */

static	int    verify_ok(struct ufs_partid *,
			 const char *,
			 const char *,
			 const char *,
			 const char *);

/*
 * Return non zero if overlap.
 */
check_for_overlap(device)
	char *device;
{
	struct ufs_partid *partid = &ufs_partid;
	struct statfs *mntbuf;
	char *raw;
	int i, mntsize, ret;

	ret = 0;
	/* convert block device to char device for init_partid(),
	 * just in case the block device is somehow in-use(ebusy).
	 */
	raw = rawname(device);
	if (init_partid(raw, partid))
		return(ret);

	/* Get swap info & verify non-overlapping */
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
	 * if the exact same partition (same device),
	 * this is OK; the kernel will catch it and return EBUSY.
	 */
    	if (mnt_partid.lvm_or_lsm_dev == partid->lvm_or_lsm_dev)
		return(0);
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
    if (bot == mbot && top == mtop) {
	/*
	 * if the exact same partition (same device),
	 * this is OK; the kernel will catch it and return EBUSY.
	 */
	struct stat stb1, stb2;
	if (stat(blockdev, &stb1) == 0 && stat(device, &stb2) == 0) {
	    if (stb1.st_rdev == stb2.st_rdev)
		return (0);
	}
    }
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
	    fprintf(stderr, MSGSTR(OVERLAP, "%s: New swap area would overlap mounted filesystem(s)\n\tor another active swap area\n"), program);
	    fprintf(stderr, MSGSTR(UMOUNT_REQ, "%s: Unmount required before swapping on %s (start %d end %d)\n"), program, device, bot, top);
	}
	fprintf(stderr, MSGSTR(START_END, "\t%s %s %s (start %d end %d)\n"),
	       blockdev, msg, usename, mbot, mtop);
    }
    return (ret);
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

