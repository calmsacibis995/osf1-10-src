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
static char	*sccsid = "@(#)$RCSfile: disklabel.c,v $ $Revision: 4.2.11.3 $ (DEC) $Date: 1993/10/29 20:40:06 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 *  Modification History:  disklabel.c
 *
 *  26-Jun-91	Tom Tierney
 *	Modified <makelabel> ULTRIX-specific code to read the
 *	partition table info directly to bypass keeping the
 *	ULTRIX partition ioctls around.  Added a display of what 
 *	the partition layout looks like for the user (some of this
 *	code was lifted from chpt so the display will look 
 *	familiar to the former ULTRIX user).
 *
 *  24-Jun-91    Terry Carruthers
 *	Modified the <makelabel> function to initialize the
 *	prototype disklabel from the BSD partition information,
 *      if that data is available.
 *
 *	This is a short term fix and should be pulled when the
 *      installation standalone is converted to use disklabel
 *	and not chpt.
 *
 *  21-Jun-91     Terry Carruthers
 *	Modified the <display> function to count the number of
 *      partition entries with a size > 0, and print that as 
 *      the partition number.  This number corresponds with the
 *      other partition data actually displayed.
 *
 */
#include <sys/secdefines.h>
#include <stdio.h>
#include <ctype.h>
#include <paths.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mode.h>
#include <sys/types.h>
#if	BSD > 43
#include <ufs/fs.h>
#else
#include <sys/fs.h>
#endif
#define DKTYPENAMES

#include <sys/disklabel.h>
#include <io/common/devio.h>
#include <io/common/pt.h>
#define ULTRIX_PARTS 8	/* # of partitions */

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

/*
 * Forward References:
 */
int edit(struct disklabel *lp, int f);
int editit();
void display(FILE *f, struct disklabel *lp);
void l_perror(char *s);
int checklabel(struct disklabel *lp);
int checkdisktab(struct disklabel *dp, char *dname);
void dyndiskcheck(struct disklabel *lp, int fd);
int getasciilabel(FILE *f, struct disklabel *lp);
void makelabel(char *type, char *name, int f, struct disklabel *lp);
int writelabel(int f, char *boot, struct disklabel *lp);
struct disklabel *readlabel(int f);
struct disklabel *makebootarea(char *boot, struct disklabel *dp);
char *skip(char *cp);
char *word(char *cp);
void usage();
int zerolabel(int f);
void Fprintf(char *fmt, ...);
void Perror(char *str);
void Warning(char *fmt, ...);
#if defined(DEBUG)
void dumplabel(struct disklabel *lp);
#endif /* defined(DEBUG) */

/*
 * No system declarations for these functions.
 */
extern void bzero(char *string, int length);
extern char *mktemp(char *template);

/*
 * Disklabel: read and write disklabels.
 * The label is usually placed on one of the first sectors of the disk.
 * Many machines (VAX 11/750) also place a bootstrap in the same area,
 * in which case the label is embedded in the bootstrap.
 * The bootstrap source must leave space at the proper offset
 * for the label on such machines.
 */

#ifdef vax
#define RAWPARTITION	'c'
#else
#define RAWPARTITION	'a'
#endif

#ifndef BBSIZE
#define	BBSIZE	8192			/* size of boot area, with label */
#endif

#ifdef vax
#define	BOOT				/* also have bootstrap in "boot area" */
#define	BOOTDIR	"/mdec"			/* source of boot binaries */
#endif

#ifdef mips
#define BOOT
#define BOOTDIR "/mdec"
#endif

#ifdef alpha
#define BOOT
#define BOOTDIR "/mdec"
#endif

#ifdef lint
#ifndef BOOT
#define	BOOT
#endif
#endif

#define	DEFEDITOR	"/usr/bin/vi"
#define	streq(a,b)	(strcmp(a,b) == 0)

#ifdef BOOT
char	*xxboot;
char	*bootxx;
#endif

char	*dkname;
char	*specname;
char	tmpfil[] = "/tmp/EdDk.aXXXXXX";

char	namebuf[BBSIZE], *np = namebuf;
struct	disklabel lab;
char	bootarea[BBSIZE];
char	boot0[MAXPATHLEN];
char	boot1[MAXPATHLEN];

enum	{ UNSPEC, EDIT, NOWRITE, READ, RESTORE, WRITE, WRITEABLE, ZERO} op = UNSPEC;

int	rflag,nbflag;

#ifdef DEBUG
int	debug;
#endif

#if SEC_BASE
static privvec_t additional_privs;
#endif

main(argc, argv)
	int argc;
	char *argv[];
{
	extern int optind;
	register struct disklabel *lp;
	FILE *t;
	int ch, f, error = 0;
	char *name = 0, *type, *cp = 0;
	struct stat statbuf;
#if SEC_BASE && !defined(SEC_STANDALONE)
	privvec_t saveprivs;

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

	while ((ch = getopt(argc, argv, "NRWdenrwz")) != EOF)
		switch (ch) {
			case 'N':
				if (op != UNSPEC)
					usage();
				op = NOWRITE;
				break;
			case 'R':
				if (op != UNSPEC)
					usage();
				op = RESTORE;
				break;
			case 'W':
				if (op != UNSPEC)
					usage();
				op = WRITEABLE;
				break;
			case 'e':
				if (op != UNSPEC)
					usage();
				op = EDIT;
				break;
			case 'n':
				++nbflag;
				break;
			case 'r':
				++rflag;
				break;
			case 'w':
				if (op != UNSPEC)
					usage();
				op = WRITE;
				break;
			case 'z':
				if (op != UNSPEC)
					usage();
				op = ZERO;
				break;
			case 'd':
#ifdef DEBUG
				debug++;
#else
				fprintf(stderr, "-d option ignored\n");
#endif
				break;
			case '?':
			default:
				usage();
		}
	argc -= optind;
	argv += optind;
	if (op == UNSPEC)
		op = READ;
	if (argc < 1)
		usage();

	dkname = argv[0];
	cp = strrchr(dkname, '/');
	if (cp == NULL) {
		/*
		 * Check to see if the device file name specified, exists.
		 * If the device name given does not exist, attempt to form
		 * a device name using device file path, raw designator, 
		 * device name and raw partition name.
		 */
		(void)sprintf(np, "%s%s", _PATH_DEV, dkname);
		if (stat(np, &statbuf) < 0) {
			(void)sprintf(np, "%sr%s%c", _PATH_DEV, dkname, 
							RAWPARTITION);
			specname = np;
			np += strlen(specname) + 1;
		} else {
			specname = np;
			np += strlen(specname) + 1;
		}
	} else
		specname = dkname;
	if ((stat(specname, &statbuf)) == 0) {
		if (!S_ISCHR(statbuf.st_mode)) {
			fprintf(stderr, "disklabel: %s not character device\n",
				specname);
			exit(4);
		}
	}
	f = open(specname, op == READ ? O_RDONLY : O_RDWR);
	if (f < 0 && errno == ENOENT && cp == NULL) {
		(void)sprintf(specname, "%sr%s", _PATH_DEV, dkname);
		np = namebuf + strlen(specname) + 1;
		f = open(specname, op == READ ? O_RDONLY : O_RDWR);
	}
	if (f < 0)
		Perror(specname);

	switch(op) {
	case EDIT:
		if (argc != 1)
			usage();
		lp = readlabel(f);
		error = edit(lp, f);
		break;
	case NOWRITE: {
		int flag = 0;
		if (ioctl(f, DIOCWLABEL, (char *)&flag) < 0)
			Perror("ioctl DIOCWLABEL");
		break;
	}
	case READ:
		if (argc != 1)
			usage();
		lp = readlabel(f);
		display(stdout, lp);
		error = checklabel(lp);
		break;
	case RESTORE:
#ifdef BOOT
		if (rflag) {
			if (argc == 4) {	/* [ priboot secboot ] */
				xxboot = argv[2];
				bootxx = argv[3];
				lab.d_secsize = DEV_BSIZE;	/* XXX */
				lab.d_bbsize = BBSIZE;		/* XXX */
			}
			else if (argc == 3) 	/* [ disktype ] */
				makelabel(argv[2], 0, f, &lab);
			else {
				fprintf(stderr,
"Must specify either disktype or bootfiles with -r flag of RESTORE option\n");
				exit(1);
			}
		}
		else
#endif
		if (argc != 2)
			usage();
		lp = makebootarea(bootarea, &lab);
		if (!(t = fopen(argv[1],"r")))
			Perror(argv[1]);
		if (getasciilabel(t, lp))
			error = writelabel(f, bootarea, lp);
		break;
	case WRITE:
		type = argv[1];
#ifdef BOOT
		if (argc > 5 || argc < 2)
			usage();
		if (argc > 3 && !nbflag) {
			bootxx = argv[--argc];
			xxboot = argv[--argc];
		}
#else
		if (argc > 3 || argc < 2)
			usage();
#endif
		if (argc > 2)
			name = argv[--argc];
		makelabel(type, name, f, &lab);
		lp = makebootarea(bootarea, &lab);
		*lp = lab;
		if ((error = checklabel(lp)) == 0)
			error = writelabel(f, bootarea, lp);
		break;
	case WRITEABLE: {
		int flag = 1;
		if (ioctl(f, DIOCWLABEL, (char *)&flag) < 0)
			Perror("ioctl DIOCWLABEL");
		break;
	case ZERO:
		if (argc != 1)
			usage();
		error = zerolabel(f);
		break;
	}
	}
	exit(error);
}

/*
 * Construct a prototype disklabel from /etc/disktab.  As a side
 * effect, set the names of the primary and secondary boot files
 * if specified.
 */
void
makelabel(char *type, char *name, int f, struct disklabel *lp)
{
	register struct disklabel *dp;

        union {
            struct fs fs;
            char pad[MAXBSIZE];
        } fsun;
#define sblock fsun.fs                  /* Used to access super block */
#define sarray fsun.pad                 /* Used to access SB as char array */
	struct pt *part;
        register int p, np;
        register daddr_t bot[8], top[8];
        int flag;
	int i;				/* loop variable */
	int c, c1;			/* read character variables */
	

	/*
	 * Logic Flow
	 * ----------
	 *
	 * Get the basic disk data for the disk type from /etc/disktab
	 * If the device is not supported by /etc/disktab, call the
	 * creatediskbyname routine to attempt to create a disklabel.
	 * If basic data retrieved ok and this is a write operation
	 * Then
	 *     Attempt to read ULTRIX style partition data from the disk
	 *     If successful
	 *     Then
	 *         Inform user and query him/her on data usage
	 *         If response affirmative
	 *         Then
	 *             Move the specific partition size and offset data 
	 *               into a disklabel structure
	 *         endif
	 *     endif
	 * endif
	 */

	dp = getdiskbyname(type);

	/*
	 * If we don't have an entry is disktab, call creatediskbyname
	 * to attempt to create an entry.
	 */
	if (dp == NULL) {
	    dp = creatediskbyname(specname);
	} else if ((dp->d_flags & D_DYNAM_GEOM) == 0) {
	    (void) checkdisktab (dp, specname);
	}

	/*
	 * If we have a label, then check for dynamic geometry 
	 * devices (like RAID) and update our geometry information
	 * if we have a dynamic geometry device.
	 */	
	if (dp){
	    if (dp->d_flags & D_DYNAM_GEOM) 
	        dyndiskcheck(dp, f);
	}
	if (dp != NULL && op == WRITE) {
            /*
             * Close your eyes and gather around the fire for a story:
             * The ULTRIX-style partition ioctls are not supported with
             * with OSF/1.  Here we are going out and reading the partition
             * information attempting to see if an ULTRIX-style partition
             * format is present.  We check the filesystem and partition
             * table magic numbers with the premise that if either is bad
             * this disk and/or the ULTRIX partition info is no good
             * (yes, we're hacking a bit here, but we're trying to be
             * "user friendly"!).
             */
            lseek(f,SBLOCK*DEV_BSIZE,0);
            if (read(f,(char *)&sblock,SBSIZE) >= 0) {
              part=(struct pt *)&sarray[SBSIZE - sizeof(struct pt)];
              if ((sblock.fs_magic == FS_MAGIC) && (part->pt_magic == PT_MAGIC)) 
               { 
                putchar('\n');
	        printf("ULTRIX compatible partition data found.\n");
		printf("This data may be different than the standard\n");
		printf("partition layout information in /etc/disktab.\n");

                for (p = 0; p < 8; p++) {
                  bot[p] = part->pt_part[p].pi_blkoff;
                  top[p] = bot[p] + part->pt_part[p].pi_nblocks;
                  if (top[p] >= 2)
                    top[p]--;
                }
                putchar('\n');
                printf("ULTRIX partition table layout is:\n");
                printf("%-10s%10s %10s %10s %s\n",
                    "partition", "bottom", "top", "size", "   overlap");
                for (p = 0; p < 8; p++) {
                  printf("    %c     %10ld %10ld %10ld    ",
                    'a' + p, bot[p], top[p], part->pt_part[p].pi_nblocks);
                  flag = 0;
                  for (np = 0; np < 8; np++) {
                    if (p == np)
                      continue;
                    if ((bot[p] >= bot[np] && bot[p] <= top[np]) ||
                       (top[p] >= bot[np] && top[p] <= top[np]) ||
                       (bot[np] >= bot[p] && bot[np] <= top[p]))
                        {
                          printf("%s%c", flag ? "," : "", 'a' + np);
                          flag++;
                        }
                  }
                  putchar('\n');
                }

		printf("\nUse the ULTRIX-style partition data? [y]: ");
		fflush(stdout);
		while ((c = getchar()) != EOF) {
		    if ((c == ' ') || (c == '\t')) 
		        continue;
		    if (c != '\n') {
		        while (((c1 = getchar()) != '\n')
				&& (c1 != EOF))
				;
			clearerr(stdin);
		    }
		    if ((c == '\n') || (c == 'y') || (c == 'Y')) {
                        /* 
                         * Copy the partition info from the ULTRIX partition
                         * we just read.  This information will be used to
                         * set up partitions on the new disk label.  Note
                         * that ULTRIX_PARTS is being used rather than
                         * MAXPARTITIONS: ULTRIX allows only up to 8 partitions
                         * whereas here with OSF/1 someone might have jacked
                         * that value up some...
                         */
		        for (i = 0; i < ULTRIX_PARTS; i++ ) {
			    dp->d_partitions[i].p_size =
			      part->pt_part[i].pi_nblocks;
			    dp->d_partitions[i].p_offset = 
			      part->pt_part[i].pi_blkoff;
		        }
		    }
		    break;
		}
		if (c == EOF) 
		    clearerr(stdin);
	      }
            }
	}

	if (dp == NULL) {
		fprintf(stderr, "%s: unknown disk type\n", type);
		exit(1);
	}
	*lp = *dp;
#ifdef BOOT
	/*
	 * Check if disktab specifies the bootstraps (b0 or b1).
	 */
	if (!xxboot && lp->d_boot0) {
		if (*lp->d_boot0 != '/')
			(void)sprintf(boot0, "%s/%s", BOOTDIR, lp->d_boot0);
		else
			(void)strcpy(boot0, lp->d_boot0);
		xxboot = boot0;
	}
	if (!bootxx && lp->d_boot1) {
		if (*lp->d_boot1 != '/')
			(void)sprintf(boot1, "%s/%s", BOOTDIR, lp->d_boot1);
		else
			(void)strcpy(boot1, lp->d_boot1);
		bootxx = boot1;
	}
	/*
	 * If bootstraps not specified anywhere, makebootarea()
	 * will choose ones based on the name of the disk special
	 * file. E.g. /dev/ra0 -> raboot, bootra
	 */
#endif /*BOOT*/
	/* d_packname is union d_boot[01], so zero */
	bzero(lp->d_packname, sizeof(lp->d_packname));
	if (name)
		(void)strncpy(lp->d_packname, name, sizeof(lp->d_packname));
}

int
writelabel(int f, char *boot, struct disklabel *lp)
{
	register int i;
	int flag;

	lp->d_magic = DISKMAGIC;
	lp->d_magic2 = DISKMAGIC;
	lp->d_checksum = 0;
	lp->d_checksum = dkcksum(lp);
	if (rflag) {
		/*
		 * First set the kernel disk label,
		 * then write a label to the raw disk.
		 * If the SDINFO ioctl fails because it is unimplemented,
		 * keep going; otherwise, the kernel consistency checks
		 * may prevent us from changing the current (in-core)
		 * label.
		 */
		if (ioctl(f, DIOCSDINFO, lp) < 0 &&
		    errno != ENODEV && errno != ENOTTY) {
			l_perror("ioctl DIOCSDINFO");
			return (1);
		}
		(void)lseek(f, (off_t)0, L_SET);
		/*
		 * write enable label sector before write (if necessary),
		 * disable after writing.
		 */
		flag = 1;
		if (ioctl(f, DIOCWLABEL, &flag) < 0)
			perror("ioctl DIOCWLABEL");
		if (write(f, boot, lp->d_bbsize) != lp->d_bbsize) {
			perror("write");
			return (1);
		}
		flag = 0;
		(void) ioctl(f, DIOCWLABEL, &flag);
	} 
	if (ioctl(f, DIOCWDINFO, lp) < 0) {
		l_perror("ioctl DIOCWDINFO");
		return (1);
	}
#ifdef vax
	if (lp->d_type == DTYPE_SMD && lp->d_flags & D_BADSECT) {
		daddr_t alt;

		alt = lp->d_ncylinders * lp->d_secpercyl - lp->d_nsectors;
		for (i = 1; i < 11 && i < lp->d_nsectors; i += 2) {
			(void)lseek(f, ((off_t)(alt + i) * lp->d_secsize), 
L_SET);
			if (write(f, boot, lp->d_secsize) < lp->d_secsize) {
				int oerrno = errno;
				fprintf(stderr, "alternate label %d ", i/2);
				errno = oerrno;
				perror("write");
			}
		}
	}
#endif
	return (0);
}

void
l_perror(char *s)
{
	int saverrno = errno;

	fprintf(stderr, "disklabel: %s: ", s);

	switch (saverrno) {

	case ESRCH:
		fprintf(stderr, "No disk label on disk;\n");
		fprintf(stderr,
		    "use \"disklabel -r\" to install initial label\n");
		break;

	case EINVAL:
		fprintf(stderr, "Label magic number or checksum is wrong!\n");
		fprintf(stderr, "(disklabel or kernel is out of date?)\n");
		break;

	case EBUSY:
		fprintf(stderr, "Open partition would move or shrink\n");
		fprintf(stderr, "Use alternate partition\n");
		break;

	case EXDEV:
		fprintf(stderr,
	"Labeled partition or 'a' partition must start at beginning of disk\n");
		break;

	default:
		errno = saverrno;
		perror((char *)NULL);
		break;
	}
}

/*
 * checkdisktab() - Check /etc/disktab Disk Information Entry.
 *
 * Description:
 *	This function verifies the calculated sectors per unit, generated
 * from the /etc/disktab file, agree with what the disk driver returns via
 * the DEVGETGEOM ioctl() command.  If the value is different, we override
 * the label d_secperunit field with the driver value so later checks do not
 * fail and cause installation failures.
 *
 * NOTE:  Even though other information in the disk label structure may be
 * incorrect (e.g., cylinders, tracks, sectors, etc), we do NOT override
 * these fields since many disks have fictitous parameters to adjust for
 * sparring and zone-bit recording.
 *
 * Inputs:	dp = Pointer to disklabel structure.
 *		dname = Pointer to special device name.
 *
 * Return Value:
 *	Returns 0 / 1 / -1 = Entry Valid / Entry Updated / Error.
 *
 */
int
checkdisktab(struct disklabel *dp, char *dname)
{
	int updates = 0;
	struct disklabel *rdp;
	struct partition *pp;
	char response[132];	/* buffer for yes/no response */
	char part;
	int i, reply;

	/*
	 * Since creatediskbyname() obtains all the necessary
	 * information, we'll use it to formulate another label
	 * with the real device information for comparison.
	 */
	if ((rdp = creatediskbyname(dname)) == NULL) {
	    Warning("Unable to obtain disk information for %s", dname);
	    return (0);
	}

	/*
	 * Verify the sectors per unit are correct, and override if the
	 * driver states the value is different from that calculated.
	 */
	if (dp->d_secperunit != rdp->d_secperunit) {
	    updates++;
	    dp->d_secperunit = rdp->d_secperunit;
	}

	/*
	 * Now, verify the paritition sizes do not exceed the disk capacity.
	 */
	for (i = 0; i < MAXPARTITIONS; i++) {
	    part = 'a' + i;
	    pp = &dp->d_partitions[i];
	    /*
	     * If partition is too big, ask user if we should truncate.
	     */
	    if ((pp->p_offset + pp->p_size) > dp->d_secperunit) {
		printf(
	    "partition %c extends past end of disk by %d sectors, truncate? [n]: ",
			part, ((pp->p_offset + pp->p_size) - dp->d_secperunit));
		fflush(stdout);
		fgets(response, sizeof(response), stdin);
		if ( (response[0] == '\n') || (response[0] == '\0')) {
		    response[0] = 'n';
		}
		if ((reply = rpmatch(response)) == 1) {
		    pp->p_size = (dp->d_secperunit - pp->p_offset);
		    updates++;
		} else {
		    return (-1);
		}
	    }
	}
	return (updates);
}

/*
 *  dyndiskcheck () -
 *    For disks flagged as "dynamic_geometry", we will call the
 *    underlying driver to get geometry and partition information
 *    to plug into the disklabel.
 */
void
dyndiskcheck(struct disklabel *lp, int fd)
{
	struct pt_tbl pt_struct;
	register struct partition *pp;
	DEVGEOMST devgeom;
	struct devget devget_st;
	int p;


	/*
	 * Dynamic geometry devices may be of MSCP or SCSI type.
	 * Rather than have duplicate /etc/disktab entries to
	 * differentiate between the two, we perform a DEVIOCGET
	 * to find out whether we are dealing with SCSI or MSCP
	 * (UNKNOWNs may be third party drives who use this service).
	 */
	if (ioctl(fd, DEVIOCGET, (char *)&devget_st) < 0) {
		Perror("ioctl DEVIOCGET");
        }
        else {
		if (strcmp(devget_st.dev_name,"rz") == NULL)
			lp->d_type = DTYPE_SCSI;
		else if (strcmp(devget_st.dev_name,"ra") == NULL)
			lp->d_type = DTYPE_MSCP;
		else
		/* UNKNOWN device */
			lp->d_type = NULL;
	}

	/*
	 * First attempt to obtain geometry information for this
	 * dynamic geometry device.  If we can't get this information,
	 * then we punt.
	 */
	if (ioctl(fd, DEVGETGEOM, (char *)&devgeom) < 0) {
		Perror("ioctl DEVGETGEOM");
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
		Perror("ioctl DIOCGDEFPT");
        }
	for (p = 0; p < MAXPARTITIONS; p++) {
		pp = &lp->d_partitions[p]; 
		pp->p_offset = pt_struct.d_partitions[p].p_offset;
		pp->p_size   = pt_struct.d_partitions[p].p_size;
	}
	return;
}

/*
 * Zero disk label for specified disk.
 */
int
zerolabel(int f)
{
	struct disklabel *lp;
	int flag;

	/*
	 * Sanity check: call the read disk label routine.  If there
	 * is no label on this disk, then we will bomb out here.  We
	 * should not be attempting to zero a non-existant label (note
	 * that the read flag is incremented to ensure readlabel() will
	 * explicitly read and check for a valid label and spew an 
	 * informative message if no label is present -- see readlabel()
	 * below for details.).
	 */
	++rflag;
	lp = readlabel(f);

	/*
	 * Get to our label sector.
	 */
	(void)lseek(f, (off_t)0, L_SET);

	/*
	 * Ensure label sector is write-enabled.
	 */
	flag = 1;
	if (ioctl(f, DIOCWLABEL, &flag) < 0)
		Perror("ioctl DIOCWLABEL");

	/* 
	 * Zero a buffer the size of label area and then 
	 * over write the label.
	 */
	lp = (struct disklabel *)bootarea;
	bzero((char *)lp, sizeof(*lp));
	if (write(f, lp, sizeof(*lp)) != sizeof(*lp)) {
		Perror("write label");
	}
	return(0);
}



/*
 * Fetch disklabel for disk.
 * Use ioctl to get label unless -r flag is given.
 */
struct disklabel *
readlabel(int f)
{
	register struct disklabel *lp;
	static char *no_label_msg = 
	"Bad pack magic number (label is damaged, or pack is unlabeled)\n";

	if (rflag) {
		if (read(f, bootarea, BBSIZE) < BBSIZE)
			Perror(specname);
		for (lp = (struct disklabel *)bootarea;
		    lp <= (struct disklabel *)(bootarea + BBSIZE - sizeof(*lp));
		    lp = (struct disklabel *)((char *)lp + 16))
			if (lp->d_magic == DISKMAGIC &&
			    lp->d_magic2 == DISKMAGIC)
				break;
		if (lp > (struct disklabel *)(bootarea+BBSIZE-sizeof(*lp)) ||
		    lp->d_magic != DISKMAGIC || lp->d_magic2 != DISKMAGIC ||
		    dkcksum(lp) != 0) {
			fprintf(stderr, no_label_msg);
			/* lp = (struct disklabel *)(bootarea + LABELOFFSET); */
			exit (1);
		}
	} else {
		lp = &lab;
		if (ioctl(f, DIOCGDINFO, lp) < 0)
		    if (errno == EINVAL) {
			fprintf(stderr, no_label_msg);
			exit (1);
		    } else {
			Perror("ioctl DIOCGDINFO");
		    }
	}
	return (lp);
}

struct disklabel *
makebootarea(char *boot, struct disklabel *dp)
{
	struct disklabel *lp;
	register char *p;
	int b;
#ifdef BOOT
	char	*dkbasename;
#endif /*BOOT*/

	lp = (struct disklabel *)(boot + (LABELSECTOR * dp->d_secsize) +
	    LABELOFFSET);
#ifdef BOOT
	if (!rflag)
		return (lp);

	if (nbflag)
		return (lp);

	if (xxboot == NULL || bootxx == NULL) {
		dkbasename = np;
		if ((p = rindex(dkname, '/')) == NULL)
			p = dkname;
		else
			p++;
		while (*p && !isdigit(*p))
			*np++ = *p++;
		*np++ = '\0';

		if (xxboot == NULL) {
			(void)sprintf(np, "%s/%sboot", BOOTDIR, dkbasename);
			if (access(np, F_OK) < 0 && dkbasename[0] == 'r')
				dkbasename++;
			xxboot = np;
			(void)sprintf(xxboot, "%s/%sboot", BOOTDIR, dkbasename);
			np += strlen(xxboot) + 1;
		}
		if (bootxx == NULL) {
			(void)sprintf(np, "%s/boot%s", BOOTDIR, dkbasename);
			if (access(np, F_OK) < 0 && dkbasename[0] == 'r')
				dkbasename++;
			bootxx = np;
			(void)sprintf(bootxx, "%s/boot%s", BOOTDIR, dkbasename);
			np += strlen(bootxx) + 1;
		}
	}
#ifdef DEBUG
	if (debug)
		fprintf(stderr, "bootstraps: xxboot = %s, bootxx = %s\n",
			xxboot, bootxx);
#endif

	b = open(xxboot, O_RDONLY);
	if (b < 0)
		Perror(xxboot);
	if (read(b, boot, (int)dp->d_secsize) < 0)
		Perror(xxboot);
	close(b);
	b = open(bootxx, O_RDONLY);
	if (b < 0)
		Perror(bootxx);
	if (read(b, &boot[dp->d_secsize], (int)(dp->d_bbsize-dp->d_secsize)) < 0)
		Perror(bootxx);
	(void)close(b);
#endif /*BOOT*/

	for (p = (char *)lp; p < (char *)lp + sizeof(struct disklabel); p++)
		if (*p) {
			fprintf(stderr,
			    "Bootstrap doesn't leave room for disk label\n");
			exit(2);
		}
	return (lp);
}

void
display(FILE *f, struct disklabel *lp)
{
	register int i, j;
	register struct partition *pp;

	fprintf(f, "# %s:\n", specname);
	if ((unsigned) lp->d_type < DKMAXTYPES)
		fprintf(f, "type: %s\n", dktypenames[lp->d_type]);
	else
		fprintf(f, "type: %d\n", lp->d_type);
	fprintf(f, "disk: %.*s\n", sizeof(lp->d_typename), lp->d_typename);
	fprintf(f, "label: %.*s\n", sizeof(lp->d_packname), lp->d_packname);
	fprintf(f, "flags:");
	if (lp->d_flags & D_REMOVABLE)
		fprintf(f, " removeable");
	if (lp->d_flags & D_ECC)
		fprintf(f, " ecc");
	if (lp->d_flags & D_BADSECT)
		fprintf(f, " badsect");
	if (lp->d_flags & D_DYNAM_GEOM)
		fprintf(f, " dynamic_geometry");
	fprintf(f, "\n");
	fprintf(f, "bytes/sector: %d\n", lp->d_secsize);
	fprintf(f, "sectors/track: %d\n", lp->d_nsectors);
	fprintf(f, "tracks/cylinder: %d\n", lp->d_ntracks);
	fprintf(f, "sectors/cylinder: %d\n", lp->d_secpercyl);
	fprintf(f, "cylinders: %d\n", lp->d_ncylinders);
	fprintf(f, "sectors/unit: %d\n", lp->d_secperunit);
	fprintf(f, "rpm: %d\n", lp->d_rpm);
	fprintf(f, "interleave: %d\n", lp->d_interleave);
	fprintf(f, "trackskew: %d\n", lp->d_trackskew);
	fprintf(f, "cylinderskew: %d\n", lp->d_cylskew);
	fprintf(f, "headswitch: %d\t\t# milliseconds\n", lp->d_headswitch);
	fprintf(f, "track-to-track seek: %d\t# milliseconds\n", lp->d_trkseek);
	fprintf(f, "drivedata: ");
	for (i = NDDATA - 1; i >= 0; i--)
		if (lp->d_drivedata[i])
			break;
	if (i < 0)
		i = 0;
	for (j = 0; j <= i; j++)
		fprintf(f, "%d ", lp->d_drivedata[j]);

	j = MAXPARTITIONS;
	fprintf(f, "\n\n%d partitions:\n", j);

	fprintf(f,
	    "#        size   offset    fstype   [fsize bsize   cpg]\n");
	pp = lp->d_partitions;
	for (i = 0; i < lp->d_npartitions; i++, pp++) {
		fprintf(f, "  %c: %8d %8d  ", 'a' + i,
		   pp->p_size, pp->p_offset);
		if ((unsigned) pp->p_fstype < FSMAXTYPES)
			fprintf(f, "%8.8s", fstypenames[pp->p_fstype]);
		else
			fprintf(f, "%8d", pp->p_fstype);
		switch (pp->p_fstype) {

		case FS_UNUSED:				/* XXX */
			fprintf(f, "    %5d %5d %5.5s ",
			    pp->p_fsize, pp->p_fsize * pp->p_frag, "");
			break;

		case FS_BSDFFS:
			fprintf(f, "    %5d %5d %5d ",
			    pp->p_fsize, pp->p_fsize * pp->p_frag,
			    pp->p_cpg);
				break;

		default:
			fprintf(f, "%20.20s", "");
			break;
		}
		fprintf(f, "\t# (Cyl. %4d",
		    pp->p_offset / lp->d_secpercyl);
		if (pp->p_offset % lp->d_secpercyl)
		    putc('*', f);
		else
		    putc(' ', f);
		fprintf(f, "- %d",
		    (pp->p_offset + 
		    pp->p_size + lp->d_secpercyl - 1) /
		    lp->d_secpercyl - 1);
		if (pp->p_size % lp->d_secpercyl)
		    putc('*', f);
		fprintf(f, ")\n");
	}
	fflush(f);
}

int
edit(struct disklabel *lp, int f)
{
	register int c;
	struct disklabel label;
	FILE *fd;

	(void) mktemp(tmpfil);
	fd = fopen(tmpfil, "w");
	if (fd == NULL) {
		fprintf(stderr, "%s: Can't create\n", tmpfil);
		perror("fopen");
		return (1);
	}
	(void)fchmod(fd, 0600);
	display(fd, lp);
	fclose(fd);
	for (;;) {
		if (!editit()){
			/*
			 * NOTE:  We are flagging an error during the edit
			 * session with a warning message.  The user will
			 * have the choice (see below) to re-edit the label
			 * if they wish (we are allowing a re-edit especially
			 * for 'ed' which exits with error if any typo (!)
			 * occurs during the edit session; though this will
			 * allow a re-edit with any editor).
			 */
			fprintf(stderr, 
			    "Warning, edit session exited abnormally!\n");
		}
		fd = fopen(tmpfil, "r");
		if (fd == NULL) {
			fprintf(stderr, "%s: Can't reopen for reading\n",
				tmpfil);
			break;
		}
		printf("write new label? [y]: "); fflush(stdout);
		while ((c = getchar()) != EOF) {
			int c1;
			if ((c == ' ') || (c == '\t')) continue;
			if (c != '\n') {
				while (((c1 = getchar()) != '\n')
					&& (c1 != EOF))
					;
				clearerr(stdin);
			}

			if ((c == 'n') || (c == 'N'))
				break;

			if ((c != '\n') && (c != 'y') && (c != 'Y')) {
				printf("Must begin with 'y' or 'n'\n");
				printf("write new label? [y]: ");
				fflush(stdout);
				clearerr(stdin);
				continue;
			}

			bzero((char *)&label, sizeof(label));
			if (getasciilabel(fd, &label)) {
				*lp = label;
				if (writelabel(f, bootarea, lp) == 0) {
					(void) unlink(tmpfil);
					return (0);
				}
			}
			break;
		}
		if (c == EOF) clearerr(stdin);

		printf("re-edit the label? [y]: "); fflush(stdout);
		while ((c = getchar()) != EOF) {
			int c1;
			if ((c == ' ') || (c == '\t')) continue;
			if (c != '\n') {
				while (((c1 = getchar()) != '\n')
					&& (c1 != EOF))
					;
				clearerr(stdin);
			}
			if ((c == 'n') || (c == 'N')) {
				(void) unlink(tmpfil);
				return (0);
			}
			if ((c != '\n') && (c != 'y') && (c != 'Y')) {
				printf("Must begin with 'y' or 'n'\n");
				printf("re-edit the label? [y]: "); 
				fflush(stdout);
				continue;
			}
			break;
		}
		if (c == EOF) clearerr(stdin);
	}
	(void) unlink(tmpfil);
	return (1);
}

int
editit()
{
	register pid_t pid, xpid;
	int child_status, omask;

	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = fork()) < 0) {
		if (errno == EPROCLIM) {
			fprintf(stderr, "You have too many processes\n");
			return(0);
		}
		if (errno != EAGAIN) {
			perror("fork");
			return(0);
		}
		sleep(1);
	}
	if (pid == 0) {
		char *tmp;
		char *path;
		char *j;
		char *k;
		char editor[512];
		char work[512];
		struct stat edinfo;
		int i;

		sigsetmask(omask);
		setgid(getgid());
		setuid(getuid());

		tmp = getenv("EDITOR");
		if (tmp == NULL)  {
			strcpy(editor, DEFEDITOR);
		} else {
			strcpy(editor, tmp);
			tmp = getenv("PATH");
			path = (char *)malloc(strlen(tmp+1));
			strcpy(path, tmp);
			k = path;
			while ( (k < strlen(path)+path) && (*editor != '/') ) {
			    i = 0;
			    while( (*k != ':') && (*k != '\0') )  /* get next path */
				work[i++] = *k++;
			    k++;				  /* add slash */
			    work[i++] = '/';
			    for( j = editor; *j != '\0' ; )	  /* add editor name */
				work[i++] = *j++;
			    work[i] = '\0';
			    if ( ((stat(work, &edinfo)) == 0) || (errno != ENOENT) ) {
				strcpy(editor, work);
				break;
			    }
			} 
		}

		if ((stat(editor, &edinfo)) != 0) {
			if (errno == ENOENT)
				strcpy(editor, "/sbin/ed");
		}

		execlp(editor, editor, tmpfil, 0);
		perror(editor);
		exit(1);
	}
	while ((xpid = wait(&child_status)) >= 0)
		if (xpid == pid)
			break;
	sigsetmask(omask);
	return(!child_status);
}

char *
skip(char *cp)
{

	while (*cp != '\0' && isspace(*cp))
		cp++;
	if (*cp == '\0' || *cp == '#')
		return ((char *)NULL);
	return (cp);
}

char *
word(char *cp)
{
	register char c;

	if( cp == NULL )
	    return ((char *)NULL);

	while (*cp != '\0' && !isspace(*cp) && *cp != '#')
		cp++;
	if ((c = *cp) != '\0') {
		*cp++ = '\0';
		if (c != '#')
			return (skip(cp));
	}
	return ((char *)NULL);
}

/*
 * Read an ascii label in from fd f,
 * in the same format as that put out by display(),
 * and fill in lp.
 */
int
getasciilabel(FILE *f, struct disklabel *lp)
{
	register char **cpp, *cp;
	register struct partition *pp;
	char *tp, *s, line[BUFSIZ];
	int v, lineno = 0, errors = 0, npart = 0;

	lp->d_bbsize = BBSIZE;				/* XXX */
	lp->d_sbsize = SBSIZE;				/* XXX */
	while (fgets(line, sizeof(line) - 1, f)) {
		lineno++;
		if (cp = index(line,'\n'))
			*cp = '\0';
		cp = skip(line);
		if (cp == NULL)
			continue;
		tp = index(cp, ':');
		if (tp == NULL) {
			fprintf(stderr, "line %d: syntax error\n", lineno);
			errors++;
			continue;
		}
		*tp++ = '\0', tp = skip(tp);
		if (streq(cp, "type")) {
			if (tp == NULL)
				tp = "unknown";
			cpp = dktypenames;
			for (; cpp < &dktypenames[DKMAXTYPES]; cpp++)
				if ((s = *cpp) && streq(s, tp)) {
					lp->d_type = cpp - dktypenames;
					goto next;
				}
			v = atoi(tp);
			if ((unsigned)v >= DKMAXTYPES)
				fprintf(stderr, "line %d:%s %d\n", lineno,
				    "Warning, unknown disk type", v);
			lp->d_type = v;
			continue;
		}
		if (streq(cp, "flags")) {
			for (v = 0; (cp = tp) && *cp != '\0';) {
				tp = word(cp);
				if (streq(cp, "removeable"))
					v |= D_REMOVABLE;
				else if (streq(cp, "ecc"))
					v |= D_ECC;
				else if (streq(cp, "badsect"))
					v |= D_BADSECT;
				else if (streq(cp, "dynamic_geometry"))
					v |= D_DYNAM_GEOM;
				else {
					fprintf(stderr,
					    "line %d: %s: bad flag\n",
					    lineno, cp);
					errors++;
				}
			}
			lp->d_flags = v;
			continue;
		}
		if (streq(cp, "drivedata")) {
			register int i;

			for (i = 0; (cp = tp) && *cp != '\0' && i < NDDATA;) {
				lp->d_drivedata[i++] = atoi(cp);
				tp = word(cp);
			}
			continue;
		}
		if (sscanf(cp, "%d partitions", &v) == 1) {
			if (v == 0 || (unsigned)v > MAXPARTITIONS) {
				fprintf(stderr,
				    "line %d: bad # of partitions\n", lineno);
				lp->d_npartitions = MAXPARTITIONS;
				errors++;
			} else
				lp->d_npartitions = v;
			continue;
		}
		if (tp == NULL)
			tp = "";
		if (streq(cp, "disk")) {
			strncpy(lp->d_typename, tp, sizeof (lp->d_typename));
			continue;
		}
		if (streq(cp, "label")) {
			strncpy(lp->d_packname, tp, sizeof (lp->d_packname));
			continue;
		}
		if (streq(cp, "bytes/sector")) {
			v = atoi(tp);
			if (v <= 0 || (v % 512) != 0) {
				fprintf(stderr,
				    "line %d: %s: bad sector size\n",
				    lineno, tp);
				errors++;
			} else
				lp->d_secsize = v;
			continue;
		}
		if (streq(cp, "sectors/track")) {
			v = atoi(tp);
			if (v <= 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_nsectors = v;
			continue;
		}
		if (streq(cp, "sectors/cylinder")) {
			v = atoi(tp);
			if (v <= 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_secpercyl = v;
			continue;
		}
		if (streq(cp, "tracks/cylinder")) {
			v = atoi(tp);
			if (v <= 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_ntracks = v;
			continue;
		}
		if (streq(cp, "cylinders")) {
			v = atoi(tp);
			if (v <= 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_ncylinders = v;
			continue;
		}
		if (streq(cp, "sectors/unit")) {
			v = atoi(tp);
			if (v <= 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_secperunit = v;
			continue;
		}
		if (streq(cp, "rpm")) {
			v = atoi(tp);
			if (v <= 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_rpm = v;
			continue;
		}
		if (streq(cp, "interleave")) {
			v = atoi(tp);
			if (v <= 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_interleave = v;
			continue;
		}
		if (streq(cp, "trackskew")) {
			v = atoi(tp);
			if (v < 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_trackskew = v;
			continue;
		}
		if (streq(cp, "cylinderskew")) {
			v = atoi(tp);
			if (v < 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_cylskew = v;
			continue;
		}
		if (streq(cp, "headswitch")) {
			v = atoi(tp);
			if (v < 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_headswitch = v;
			continue;
		}
		if (streq(cp, "track-to-track seek")) {
			v = atoi(tp);
			if (v < 0) {
				fprintf(stderr, "line %d: %s: bad %s\n",
				    lineno, tp, cp);
				errors++;
			} else
				lp->d_trkseek = v;
			continue;
		}
		if ('a' <= *cp && *cp < ('a' + MAXPARTITIONS) && cp[1] == '\0') {
			unsigned part = *cp - 'a';
			npart++;
			if (npart > MAXPARTITIONS) {
				printf("npart %d  d_npart %d cp %c\n",
					npart, lp->d_npartitions, *cp);
				fprintf(stderr,
				    "line %d: bad # of partitions\n", lineno);
				errors++;
				continue;
			} 
			pp = &lp->d_partitions[part];

#define NXTNUM(n) { 		\
	cp = tp, tp = word(cp); \
	if (tp == NULL)  	\
		tp = cp;	\
				\
	if (cp != NULL) 	\
		(n) = atoi(cp);	\
	else 			\
		(n) = 0;	\
     }

			NXTNUM(v);
			if (v < 0) {
				fprintf(stderr,
				    "line %d: %s: bad partition size\n",
				    lineno, cp);
				errors++;
			} else
				pp->p_size = v;
			NXTNUM(v);
			if (v < 0) {
				fprintf(stderr,
				    "line %d: %s: bad partition offset\n",
				    lineno, cp);
				errors++;
			} else
				pp->p_offset = v;
			cp = tp, tp = word(cp);
			cpp = fstypenames;
			for (; cpp < &fstypenames[FSMAXTYPES]; cpp++)
				if ((s = *cpp) && streq(s, cp)) {
					pp->p_fstype = cpp - fstypenames;
					goto gottype;
				}
			if (isdigit(*cp))
				v = atoi(cp);
			else
				v = FSMAXTYPES;
			if ((unsigned)v >= FSMAXTYPES) {
				fprintf(stderr, "line %d: %s %s\n", lineno,
				    "Warning, unknown filesystem type", cp);
				v = FS_UNUSED;
			}
			pp->p_fstype = v;
	gottype:

			switch (pp->p_fstype) {

			case FS_UNUSED:				/* XXX */
				NXTNUM(pp->p_fsize);
				if (pp->p_fsize == 0)
					break;
				NXTNUM(v);
				pp->p_frag = v / pp->p_fsize;
				break;

			case FS_BSDFFS:
				NXTNUM(pp->p_fsize);
				if (pp->p_fsize == 0)
					break;
				NXTNUM(v);
				pp->p_frag = v / pp->p_fsize;
				NXTNUM(pp->p_cpg);
				break;

			default:
				break;
			}
			continue;
		}
		fprintf(stderr, "line %d: %s: Unknown disklabel field\n",
		    lineno, cp);
		errors++;
	next:
		;
	}
	errors += checklabel(lp);
	return (errors == 0);
}

/*
 * Check disklabel for errors and fill in
 * derived fields according to supplied values.
 */
int
checklabel(struct disklabel *lp)
{
	register struct partition *pp;
	int i, errors = 0;
	char part;
#ifdef DEBUG
	if(debug) {
		printf("label being checked:\n");
		dumplabel(lp);
	}
#endif

	if (lp->d_secsize == 0) {
		fprintf(stderr, "sector size %d\n", lp->d_secsize);
		return (1);
	}
	if (lp->d_nsectors == 0) {
		fprintf(stderr, "sectors/track %d\n", lp->d_nsectors);
		return (1);
	}
	if (lp->d_ntracks == 0) {
		fprintf(stderr, "tracks/cylinder %d\n", lp->d_ntracks);
		return (1);
	}
	if  (lp->d_ncylinders == 0) {
		fprintf(stderr, "cylinders/unit %d\n", lp->d_ncylinders);
		errors++;
	}
	if (lp->d_rpm == 0)
		Warning("revolutions/minute %d\n", lp->d_rpm);
	if (lp->d_secpercyl == 0)
		lp->d_secpercyl = lp->d_nsectors * lp->d_ntracks;
	if (lp->d_secperunit == 0)
		lp->d_secperunit = lp->d_secpercyl * lp->d_ncylinders;
	if (lp->d_bbsize == 0) {
		fprintf(stderr, "boot block size %d\n", lp->d_bbsize);
		errors++;
	} else if (lp->d_bbsize % lp->d_secsize)
		Warning("boot block size %% sector-size != 0\n");
	if (lp->d_sbsize == 0) {
		fprintf(stderr, "super block size %d\n", lp->d_sbsize);
		errors++;
	} else if (lp->d_sbsize % lp->d_secsize)
		Warning("super block size %% sector-size != 0\n");
	if (lp->d_npartitions > MAXPARTITIONS)
		Warning("number of partitions (%d) > MAXPARTITIONS (%d)\n",
		    lp->d_npartitions, MAXPARTITIONS);
	for (i = 0; i < lp->d_npartitions; i++) {
		part = 'a' + i;
		pp = &lp->d_partitions[i];
		if (pp->p_size == 0 && pp->p_offset != 0)
			Warning("partition %c: size 0, but offset %d\n",
			    part, pp->p_offset);
#ifdef notdef
		if (pp->p_size % lp->d_secpercyl)
			Warning("partition %c: size %% cylinder-size != 0\n",
			    part);
		if (pp->p_offset % lp->d_secpercyl)
			Warning("partition %c: offset %% cylinder-size != 0\n",
			    part);
#endif
		if (pp->p_offset > lp->d_secperunit) {
			fprintf(stderr,
			    "partition %c: offset past end of unit\n", part);
			errors++;
		}
		if (pp->p_offset + pp->p_size > lp->d_secperunit) {
			fprintf(stderr,
			    "partition %c: partition extends past end of unit\n", part);
			errors++;
		}
	}
	for (; i < MAXPARTITIONS; i++) {
		part = 'a' + i;
		pp = &lp->d_partitions[i];
		if (pp->p_size || pp->p_offset)
			Warning("unused partition %c: size %d offset %d\n",
			    'a' + i, pp->p_size, pp->p_offset);
	}
#ifdef DEBUG
	if(debug) {
		printf("label after checking:\n");
		dumplabel(lp);
	}
#endif
	return (errors);
}

/*VARARGS*/
void
Fprintf(char *fmt, ...)
{
	va_list ap;
	FILE *fp = stderr;

	fprintf(fp, "disklabel: ");
	va_start(ap, fmt);
	vfprintf (fp, fmt, ap);
	va_end(ap);
	fprintf(fp, "\n");
}

/*VARARGS*/
void
Warning(char *fmt, ...)
{
	va_list ap;
	FILE *fp = stderr;

	fprintf(fp, "Warning, ");
	va_start(ap, fmt);
	vfprintf (fp, fmt, ap);
	va_end(ap);
	fprintf(fp, "\n");
}

void
Perror(char *str)
{
	fputs("disklabel: ", stderr); perror(str);
	exit(4);
}

void
usage()
{
#ifdef BOOT
    fprintf(stderr, "%-62s%s\n%-62s%s\n%-62s%s\n%-62s%s\n%-62s%s\n%-62s%s\n%-62s%s\n",
"usage: disklabel [-r] disk", "(to read label)",
"or disklabel -w  [-r] disk type [ packid ] [ xxboot bootxx ]", "(to write label)", 
"or disklabel -wn [-r] disk type [ packid ] ", "(to write label without bootstrap)", 
"or disklabel -e  [-r] disk", "(to edit label)",
"or disklabel -R  [-r] [-n] disk protofile [ type | xxboot bootxx ]", "(to restore label)",
"or disklabel [-NW] disk", "(to write disable/enable label)",
"or disklabel -z disk", "(to zero label)");
#else
    fprintf(stderr, "%-43s%s\n%-43s%s\n%-43s%s\n%-43s%s\n%-43s%s\n%-43s%s\n", 
"usage: disklabel [-r] disk", "(to read label)",
"or disklabel -w [-r] disk type [ packid ]", "(to write label)",
"or disklabel -e [-r] disk", "(to edit label)",
"or disklabel -R [-r] disk protofile", "(to restore label)",
"or disklabel [-NW] disk", "(to write disable/enable label)",
"or disklabel -z disk", "(to zero label)");
#endif
	exit(1);
}

#ifdef DEBUG
void
dumplabel(struct disklabel *lp)
{
	register struct partition *pptr;
	register int i;

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
#endif
