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
static char	*sccsid = "@(#)$RCSfile: mkpv.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:22:49 $";
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

/*****************************************************************************
 *
 * mkpv.c - Create a physical volume
 *
 * Syntax:
 * 	mkpv [-t type] [-b bbfile] device
 * 
 *	where:
 *		device: Is the Block Device to make the Physical Volume on,
 *                      for example, "/dev/rz0c", or "/dev/rz3g".
 *              type:   Is the type of the device as used in /ect/disktab.
 *                      This is used to specify the disk type of the physical
 *                      media that contains the Block Device.  This argument
 *                      is necessary if the Unix device driver for the Block
 *                      Device does not support the DIOCGINFO ioctl.
 *              bbfile: Is the bad block file.  This file contains a list of
 *                      bad sectors on the Block Device's physical media.
 *                      These sector numbers are relative to the start of the
 *                      physical media.  This file must be specified if there
 *                      are bad sectors and the physical media's device driver
 *                      does not support the DIOC???? ioctl.
 *
 ****************************************************************************/

/*
 * newfs: friendly front end to mkfs
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <ufs/fs.h>
#include <ufs/dir.h>
#include <sys/ioctl.h>
#ifdef	multimax
#include <layout.h>
#include <mmaxio/msioctl.h>
#endif	/* multimax */
#include <sys/disklabel.h>
#include <sys/file.h>
#include <sys/mount.h>

#include <lvm/lv_defect.h>
#include <lvm/pvres.h>

#include <stdio.h>
#include <ctype.h>

char	*rindex();
char	*index();
struct	disklabel	*getdisklabel();
	lv_lvmrec_t	*lvmrec;

/*
 * These should be obtained from include files, but the include files that
 * are needed include all kinds of other garbage which this program doesn't
 * need to know about, so they are duplicated here.
 */
#define	DEV_BSIZE	512

char	nullbuf[DEV_BSIZE];

char	*progname;
char	*disktype;
char	*bbfile = NULL;
char	*devname;
int	fdev;

int fssize, poffset;

main(argc, argv)
int argc;
char *argv[];
{
    register struct partition *pp;
    register struct disklabel *lp;
    char *cp;
    int	i, partno;
    struct stat st;
    FILE *fp;

    /* Get the program name. */
    progname = argv[0];

    /*
     * Parse the argument list.
     */
    disktype = NULL; bbfile = NULL;
    argc--, argv++;
    while (argc > 0 && argv[0][0] == '-') {
	cp = &argv[0][1];
	switch (*cp) {
	  case 't':
	    if (argc < 1)
		fatal("-t: missing disk type");
	    argc--, argv++;
	    disktype = *argv;
	    break;

	  case 'b':
	    if (argc < 1)
		fatal("-b: missing bad block file");
	    argc--, argv++;
	    bbfile = *argv;
	    if ((fp = fopen(bbfile, "r")) == NULL)
		fatal("couldn't open bad block file %s", bbfile);
	    break;

	  default:
	    fatal("-%c: unknown flag", *cp);
	}
	argc--, argv++;
    }
    if (argc != 1)
	fatal("usage [-t disktype] [-b bbfile] device");

    /*
     * Open the device and make sure that it is a character special file.
     */
    devname = *argv;
    if ((fdev = open(devname, O_RDWR)) < 0) {
	perror(devname);
	exit(2);
    }
    if (fstat(fdev, &st) < 0) {
	fprintf(stderr, "%s: ", progname); perror(devname);
	exit(4);
    }
    if ((st.st_mode & S_IFMT) != S_IFCHR)
	fatal("%s: not a character device", devname);

    /* Determine the partition letter. */
    cp = index(argv[0], '\0') - 1;

#ifdef  multimax
    if (cp == 0 || (*cp < 'a' || *cp > 'p') && !isdigit(*cp))
#else
    if (cp == 0 || (*cp < 'a' || *cp > 'h') && !isdigit(*cp))
#endif
	fatal("%s: can't figure out file system partition", argv[0]);
    partno = isdigit(*cp) ? *cp - '0' : *cp - 'a';
#ifdef  multimax
    /*
     * On the multimax, we use getheaderinfo to get the size of the
     * partition.
     */
    getheaderinfo(devname, partno);
    pp = NULL;
#else
    lp = getdisklabel(devname, fdev, disktype);
    pp = &lp->d_partitions[partno];

    if (pp->p_size == 0)
	fatal("%s: `%c' partition is unavailable", argv[0], *cp);
    fssize = pp->p_size;
    poffset = pp->p_offset;
#endif

    /*
     * Construct the LVM record.
     */
    bzero(&nullbuf[0], DEV_BSIZE);
    lvmrec = (lv_lvmrec_t *)&nullbuf[0];
    strcpy(&lvmrec->lvm_id[0], "LVMREC01");
    lvmrec->pv_id.id1 = gethostid();
    lvmrec->pv_id.id2 = time(NULL);
    lvmrec->last_psn = fssize;

    /*
     * Write the LVM record to the appropriate sectors on disk.
     */
    lseek(fdev, (off_t)(PVRA_LVM_REC_SN1*DEV_BSIZE), L_SET);
    write(fdev, lvmrec, DEV_BSIZE);

    lseek(fdev, (off_t)(PVRA_LVM_REC_SN2*DEV_BSIZE), L_SET);
    write(fdev, lvmrec, DEV_BSIZE);

    /*
     * Clear the primary and secondary bad block directory.
     */
    bzero(&nullbuf[0], DEV_BSIZE);
    for (i = 0; i < PVRA_BBDIR_LENGTH; i++)
	write_to_bbdir(&nullbuf[0], i);

    /*
     * Read the bad block list into the bad block directory.  All bad
     * block numbers in the bad block file must be relative to the start
     * of the physical media.  They are converted into values relative
     * to the start of the partition.  This means that some or all of
     * the bad blocks in the file may be excluded from the bad block
     * directory on the physical volume.
     */
    
    strcpy(&nullbuf[0], "DEFECT01");
    if (bbfile == NULL)
	write_to_bbdir(&nullbuf[0], 0);
    else {
	int bbnum;
	lv_bblk_t *dp;

	i = 0;
	dp = (lv_bblk_t *)&nullbuf[8];
	/* 
	 * Note: this code assumes that the bad block list does not overflow
	 * the bad block directory, and further leaves room for a null
	 * sector to terminate the directory.
	 */
	while (fscanf(fp, "%d\n", &bbnum) != EOF) {
	    /*
	     * Record the block number relative to the start of the
	     * partition
	     */
	    if (bbnum >= poffset && bbnum < poffset+fssize) {
		SET_BB_DEFECT(dp, bbnum - poffset);
		SET_BB_REASON(dp, DEFECT_MFR);
		SET_BB_STATUS(dp, REL_DESIRED);

		/* Write out full defect directory buffers */
		if (++dp == (lv_bblk_t *)&nullbuf[DEV_BSIZE]) {
		    write_to_bbdir(&nullbuf[0], i++);
		    dp = (lv_bblk_t *)&nullbuf[0];
		    bzero(&nullbuf[0], DEV_BSIZE);
		}
	    }
	}

	/* End of list - right out partially filled directory */
	if (dp > (lv_bblk_t *)&nullbuf[0])
	    write_to_bbdir(&nullbuf[0], i++);
	
	/* Terminate the list with a null sector */
	bzero(&nullbuf[0], DEV_BSIZE);
	write_to_bbdir(&nullbuf[0], i);
    }

}

struct disklabel *
getdisklabel(s, fd, type)
char *s, *type;
int fd;
{
    static struct disklabel lab;
    struct disklabel *getdiskbyname();

    if (ioctl(fd, DIOCGDINFO, (char *)&lab) < 0) {
	if (type == NULL) {
	    perror("ioctl (GDINFO)");
	    fatal("%s: can't read disk label; disk type must be specified", s);
	}
	return (getdiskbyname(type));
    }
    return (&lab);
}

/*VARARGS*/
fatal(fmt, arg1, arg2)
char *fmt;
{
    fprintf(stderr, "%s: ", progname);
    fprintf(stderr, fmt, arg1, arg2);
    putc('\n', stderr);
    exit(8);
}

write_to_bbdir(s, i)
char *s;
int i;
{
    if (i < PVRA_BBDIR_LENGTH-1) {
	lseek(fdev, (off_t)((PVRA_BBDIR_SN1+i)*DEV_BSIZE), L_SET);
	write(fdev, s, DEV_BSIZE);

	lseek(fdev, (off_t)((PVRA_BBDIR_SN2+i)*DEV_BSIZE), L_SET);
	write(fdev, s, DEV_BSIZE);
    }
}
#ifdef	multimax

#define BUFSIZE		((sizeof(layout_t) + DEV_BSIZE - 1) & ~(DEV_BSIZE -1))

char	layout_info[BUFSIZE];
char	headerdev[MAXPATHLEN];
extern int errno;

getheaderinfo(rdev, pnum)
	char *rdev;
	int   pnum;
{

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

	tcp = index(headerptr, '\0') - 1;
	cp = tcp; 

	if (isdigit(*tcp))
		*tcp ='3';
	else
		*tcp = 'd';

	fd = open(headerptr, O_RDONLY);   
	if (fd < 0) {
		perror(headerptr);
		exit(1);
	}

	if ((err = ioctl(fd, (int)MSIOCRDLAY, (char *)layout)) == -1) {
		err = errno;
		perror("READ HEADER PARTITION ERROR");
		return(err);
	}

	if ((p_size = layout->partitions[pnum].part_size) == 0)
		fatal("%s: `%c' partition is unavailable", rdev, *cp);
	if (fssize == 0)
		fssize = p_size;
	poffset = layout->partitions[pnum].part_off;
	if (fssize > p_size)
	       fatal("%s: maximum physical volume size on the `%c' partition is %d",
			rdev, *cp, p_size);
	close(fd);
}

#endif	/* multimax */
