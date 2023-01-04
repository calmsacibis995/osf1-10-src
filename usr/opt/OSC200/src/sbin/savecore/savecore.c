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
static char *rcsid = "@(#)$RCSfile: savecore.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/12/09 20:37:11 $";
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
 * Copyright (c) 1980, 1986, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1986, 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * savecore
 *
 *  Change History
 *
 * 05-Jun-91	Scott Cranston
 *	- Added ability to recover the syslog msgbuf from the dump.
 *      - Added -e option.
 *
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif
#include <stdio.h>
#include <stddef.h>
#include <nlist.h>
#include <sys/param.h>
#if	BSD > 43
#include <ufs/fs.h>
#else
#include <sys/fs.h>
#endif
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/msgbuf.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <dec/binlog/binlog.h>     /* for recovery of binary error log buffer */
#include <dirent.h>
#include <paths.h>
#include <fstab.h>

#ifdef __alpha
#include <arch/alpha/hal/hal_sysinfo.h>
#endif

#define	DAY	(60L*60L*24L)
#define	LEEWAY	(3*DAY)
#define MAXLINE 1024
#define DUMPOFF_INCR (2*1024*1024)
#define DUMPOFF_MAX (1024*1024*1024)

#define NOTOK		((off_t)-1)	/* failure return value from vtop */

#ifdef __alpha
off_t ok(vm_offset_t number);
#else
#if defined(vax) || defined(mips)
#define ok(number) ((number)&0x7fffffff)
#else
#ifdef tahoe
#define ok(number) ((number)&~0xc0000000)
#else
#define ok(number) (number)
#endif
#endif
#endif

struct dumpinfo dumpinfo;
char	*sc_dirname;			/* directory to save dumps in */
char	*unixname;			/* kernel that core dumped */
char	*ddname;			/* name of block device with dump */
int	dumpfd = -1;			/* read descriptor on dump device */
char	*find_dev();
time_t	dumptime;			/* time the dump was taken */
off_t	dumpoff;			/* byte offset of the dump */
int	dumpsize;			/* pages of memory dumped */
int	csum;				/* savecore's checksum of dump */
off_t	dumpbytes;			/* size of dump in bytes */
int     imagesize;                      /* size of kernel image */
time_t	now;				/* current date */
char	*path();
char	*malloc();
char	*ctime();
char	vers[80];
char	panic_mesg[80];
char	*panicstr;
off_t	lseek();
off_t	Lseek();
off_t	partial_lseek();
int	dump_read(off_t offset, void *ptr, ulong size, int err_return);
int	Verbose;
int	force;
int	clear;
int     copy_image = 1;              /* enough room to save the kernel? */
extern	int errno;

int     savemsgbuf;                  /* flag to save only the msgbuf & blbuf */
caddr_t	msgbufaddr;                  /* address of msgbuf buffer */
caddr_t	binbufaddr;                  /* address of binlog buffer */
char	bufsavepath[MAXLINE];        /* where to save buffer dump file */

char	*usage =
	"usage: savecore [-c] [-e] [-f] [-v] dirname\n";

char	*nodumpmsg = "no dump exists\n";
char	*nodevmsg = "can't determine name of dump device\n";
char	*nomemmsg = "savecore: can't allocate memory for %s\n";
caddr_t virtaddr;


main(argc, argv)
	char **argv;
	int argc;
{
	char *cp;
	struct fstab *fsp;

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, "%s: need sysadmin authorization\n",
			command_name);
		exit(1);
	}
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
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
#endif /* SEC_BASE */
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'f':
			force++;
			break;

		case 'v':
			Verbose++;
			break;

		case 'c':
			clear++;
			break;

		case 'e':
			savemsgbuf++;  /* save only the msgbuf */
			break;

		default:
			fprintf(stderr, usage);
			exit(1);
		}
		argc--, argv++;
	}

	if (argc == 1) {
		sc_dirname = argv[0];
	} else {
	    /* allow no save directory name if just want to clear dump */
	    if (argc || !clear) {
		fprintf(stderr, usage);
		exit(1);
	    }
	}

	openlog("savecore", LOG_ODELAY, LOG_AUTH);

	/*
	 * scan /etc/fstab for the dump device
	 */
	ddname = NULL;
	if (clear) {
          /* if the clear flag is set, find and clean all dumps */
          while (fsp = getfsent()) {
	    if (!strcmp(fsp->fs_type, "sw")) {
	    	    if (Verbose)
			printf("Searching %s for dump\n", fsp->fs_spec);
		    if (dump_exists(fsp->fs_spec)) {
			if (!(ddname = malloc(strlen(fsp->fs_spec) + 1))) {
				log(LOG_ERR, nomemmsg, "fstab name buffer");
				exit(1);
			}
			strcpy(ddname, fsp->fs_spec);
		        clear_dump();
                        free(ddname);
		    }
	    }
	  }
          exit(0);
        }
        else
          while (!ddname && (fsp = getfsent())) {
	    if (!strcmp(fsp->fs_type, "sw")) {
	    	    if (Verbose)
			printf("Searching %s for dump\n", fsp->fs_spec);
		    if (dump_exists(fsp->fs_spec)) {
			if (!(ddname = malloc(strlen(fsp->fs_spec) + 1))) {
				log(LOG_ERR, nomemmsg, "fstab name buffer");
				exit(1);
			}
			strcpy(ddname, fsp->fs_spec);
		    }
	    }
	  }

	if (!ddname) {
	    /*
	     * couldn't find a dump
	     */
	    if (Verbose)
		printf(nodumpmsg);
	    exit(0);
	}

	if (access(sc_dirname, W_OK) < 0) {
		Perror(LOG_ERR, "%s: %m\n", sc_dirname);
		exit(1);
	}

	read_dumpinfo_data();
	(void) time(&now);
	if (panicstr)
		syslog(LOG_CRIT, "reboot after panic: %s\n", panic_mesg);
	else
		syslog(LOG_CRIT, "reboot\n");
	save_msgbuf();
	save_blbuf();      /* always save the binary event log buffer */

	if (!savemsgbuf)  {
	    
	    printf("System went down at %s", ctime(&dumptime));

	    if(!check_space() && !force)
		exit(2);

	    csum = 0;
	    save_core();
	    if (dumpinfo.csum != csum)
		printf("Warning: Bad checksum of dump.\n");
	}

        clear_dump();
        free(ddname);
	exit(0);
}


/* Here we read all of the data pointed to by the dumpinfo addresses and
 * fill in the various global variables that we will need later.
 */

static read_dumpinfo_data()
{
    char kernel_name[MAXPATHLEN];
    int len, offset;

    dumpfd = Open(ddname, O_RDONLY);
    if (lseek(dumpfd, dumpoff, L_SET) == -1L) {
	fprintf(stderr, "savecore: can't seek to start of dump\n");
        exit(1);
    }
    if(read(dumpfd, &dumpinfo, sizeof(dumpinfo)) != sizeof(dumpinfo)){
	if (errno) perror(ddname);
	fprintf(stderr, "savecore: can't read %s starting at byte %d\n",
		ddname, dumpoff);
        exit(1);
    }

    dump_read(ok((vm_offset_t) dumpinfo.addr[X_VERSION]), vers, sizeof(vers),
	      0);
    dump_read(ok((vm_offset_t) dumpinfo.addr[X_DUMPSIZE]), &dumpsize,
	      sizeof(dumpsize), 0);	    
    dump_read(ok((vm_offset_t) dumpinfo.addr[X_TIME]), &dumptime,
	      sizeof(dumptime), 0);	    
    dumpbytes = (off_t)dumpsize * (off_t)NBPG;
    dump_read(ok((vm_offset_t) dumpinfo.addr[X_PANICSTR]), &panicstr,
	      sizeof(panicstr), 0);
    dump_read(ok((vm_offset_t) dumpinfo.addr[X_MSGBUF]), &msgbufaddr,
	      sizeof(msgbufaddr), 0);
    dump_read(ok((vm_offset_t) dumpinfo.addr[X_BLBUFPADR]), &binbufaddr,
	      sizeof(binbufaddr), 0);
    dump_read(ok((vm_offset_t) dumpinfo.addr[X_BLBUF]), &virtaddr,
	      sizeof(virtaddr), 0);
    dump_read(ok((vm_offset_t) dumpinfo.addr[X_BOOTEDFILE]),
	      kernel_name, sizeof(kernel_name), 0);
    if(kernel_name[0] != '/'){
        len = strlen(kernel_name) + 2;
        offset = 1;
    } else {
        len = strlen(kernel_name) + 1;
        offset = 0;
    }
    if((unixname = malloc(len)) == NULL){
        log(LOG_ERR, nomemmsg, "kernel name buffer");
        exit(1);
    }
    unixname[0] = '/';
    strcpy(&unixname[offset], kernel_name);
}

/*
 * This routine determines if there is a dump.  It scans the partition
 * looking for a valid dump header every DUMPOFF_INCR bytes.
 * The offset of a valid dump is saved in the global dumpoff. 
 */
dump_exists(
    char	*dev)
{
    int fd;
    int valid = 0;
    long part_size = 0;
    struct disklabel lp;
    int partition;

    dumpoff = 0;

    /*
     * open the dump device and fetch its size
     */
    fd = Open(dev, O_RDONLY);
    if (fd < 0)
	return(valid);

    if (ioctl(fd, DIOCGDINFO, &lp) == 0) {
	partition = dev[strlen(dev)-1] - 'a';
	part_size = lp.d_partitions[partition].p_size * lp.d_secsize;
    }

    if (part_size <= 0) {
	part_size = DUMPOFF_MAX;
    }

    while (!valid && dumpoff < part_size) {
	if (lseek(fd, dumpoff, L_SET) == -1L) {
	    break;
	}
        if (read(fd, &dumpinfo, sizeof(dumpinfo)) != sizeof(dumpinfo)) {
	    break;
        }
        if (!strncmp(dumpinfo.signature, DUMPINFO_SIGNATURE,
		    strlen(DUMPINFO_SIGNATURE) - 1)){
	    if(strcmp(dumpinfo.signature, DUMPINFO_SIGNATURE)){
	        fprintf(stderr, "Found an incomplete %s dump.\n",
		        dumpinfo.partial_dump ? "partial" : "full");
	    }
	    valid = 1;
    	    if (Verbose)
        	printf("dumpoff = %ld bytes\n", dumpoff);
	    break;
	}
	dumpoff += DUMPOFF_INCR;
    }
    close(fd);
    return(valid);
}

clear_dump()
{
	int fd;

    	fd = Open(ddname, O_WRONLY);
	if (lseek(fd, dumpoff, L_SET) == -1L) {
	        Perror(LOG_ERR, "lseek: %m", "lseek");
		exit(1);
	}
	bzero(&dumpinfo, sizeof(dumpinfo));
	Write(fd, &dumpinfo, sizeof(dumpinfo));
	close(fd);
	sync();
}


char *
find_dev(dev, type)
	register dev_t dev;
	register int type;
{
	register DIR *dfd = opendir(_PATH_DEV);
	struct dirent *dir;
	struct stat statb;
	static char devname[MAXPATHLEN + 1];
	char *cp;
	int n;

	n = strlen(_PATH_DEV);
	strcpy(devname, _PATH_DEV);
	while ((dir = readdir(dfd))) {
		strcpy(&devname[n], dir->d_name);
		if (stat(devname, &statb)) {
			perror(devname);
			continue;
		}
		if ((statb.st_mode & S_IFMT) == type && statb.st_rdev == dev) {
			if (!(cp = malloc(strlen(devname) + 1))) {
				log(LOG_ERR, nomemmsg, "device name buffer");
				exit(1);
			}
			strcpy(cp, devname);
			closedir(dfd);
			return (cp);
		}
	}
	closedir(dfd);
/*	log(LOG_ERR, "Can't find device %d/%d\n", major(dev), minor(dev)); */
	return (NULL);
}

/* this routine is responsible for setting ddname on successful returns */
int
check_fstab()
{
	register struct fstab *fsp;

	while (fsp = getfsent()) {
		if (strcmp(fsp->fs_type, FSTAB_SW) == 0) {
			if (!(ddname = malloc(strlen(fsp->fs_spec) + 1))) {
				log(LOG_ERR, nomemmsg, "fstab name buffer");
				exit(1);
			}
			strcpy(ddname, fsp->fs_spec);
			return (1);
		}
	}
	return (0);
}

char *
path(file)
	char *file;
{
	register char *cp;

	if (!(cp = malloc(strlen(file) + strlen(sc_dirname) + 2))) {
		log(LOG_ERR, nomemmsg, "path name buffer");
		exit(1);
	}
	(void) strcpy(cp, sc_dirname);
	(void) strcat(cp, "/");
	(void) strcat(cp, file);
	return (cp);
}

check_space()
{
	long minfree, spacefree;
	struct statfs fsbuf;
	struct stat buf;

	if (access(unixname, R_OK) < 0) {
		log(LOG_WARNING, "Kernel %s can not be accessed\n", unixname);
		imagesize = 0;
		copy_image = 0;
	}
	else {
   		buf.st_size = 0;
		stat(unixname, &buf);
		imagesize = buf.st_size;
	}
	if (statfs(sc_dirname, &fsbuf) < 0) {
		Perror(LOG_ERR, "%s: %m\n", sc_dirname);
		exit(1);
	}
 	spacefree = fsbuf.f_bavail * fsbuf.f_fsize;
	minfree = read_number("minfree");
 	if (spacefree < dumpbytes) {
		if(force) {
		        log(LOG_WARNING,
			"Dump will be performed, but not enough space on device\n");
		} else {
			log(LOG_WARNING,
			"Dump will be omitted, not enough space on device\n");
			return (0);
		}
	}
	if (spacefree < dumpbytes + imagesize) {
		log(LOG_WARNING, "Not enough space for kernel image\n");
		copy_image = 0;
	}
	if (spacefree - dumpbytes - imagesize < minfree)
		log(LOG_WARNING,
		"Dump will be performed, but free space threshold crossed\n");
	return (1);
}

read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	fp = fopen(path(fn), "r");
	if (fp == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}

checksum(ptr, size)
    char *ptr;
    int size;
{
    while (size)
        csum += ptr[--size];
}

#define	BUFFERSIZ		(256*1024)		/* 1/4 Mb */

int full_core(int ifd, int ofd, off_t size)
{
	register char *cp;
	int ret, n;

	ret = 1;
	cp = malloc(BUFFERSIZ);
	if (cp == NULL) {
		log(LOG_ERR, nomemmsg, "i/o buffer");
		return(0);
	}
	while (size > 0) {
		n = read(ifd, cp, size > BUFFERSIZ ? BUFFERSIZ : size);
		checksum(cp, size > BUFFERSIZ ? BUFFERSIZ : size);
		if (n <= 0) {
			if (n == 0)
				log(LOG_WARNING,
				    "WARNING: EOF on dump device; %s\n",
				    "vmcore may be incomplete");
			else
				Perror(LOG_ERR, "savecore: full_core read",
				       "read");
			ret = 0;
			break;
		}
		if ((ret = write(ofd, cp, n)) < n) {
			if (ret < 0)
				Perror(LOG_ERR, "write: %m", "write");
			else
				log(LOG_ERR, "short write: wrote %d of %d\n",
				    ret, n);
			log(LOG_WARNING,
			    "WARNING: vmcore may be incomplete\n");
			ret = 0;
			break;
		}
		size -= n;
	}
	free(cp);
        return(ret);
}

int partial_core(int in_fd, int out_fd, off_t size)
{
        int i, n;
	caddr_t pages[DEV_BSIZE/sizeof(caddr_t)];
	char page[NBPG/sizeof(char)];
	
        while(size > 0){
                n = read(in_fd, pages, sizeof(pages));
		checksum(pages, sizeof(pages));
                if(n == 0) break;
                else if(n == -1){
                        Perror(LOG_ERR, "read : %m", "read");
                        exit(1);
                }
                for(i=0;i<sizeof(pages)/sizeof(caddr_t);i++){
                        if(pages[i] == 0) break;
                        if(lseek(out_fd, ok((ulong) pages[i]), L_SET) == -1){
            	                Perror(LOG_ERR, "lseek: %m", "lseek");
            	                exit(1);
                        }
                        if(read(in_fd, page, sizeof(page)) != sizeof(page)){
	                        Perror(LOG_ERR, "read: %m", "read");
	                        exit(1);
                        }
			checksum(page, sizeof(page));
                        if(write(out_fd, page, sizeof(page)) != sizeof(page)){
	                        Perror(LOG_ERR, "write: %m", "write");
	                        exit(1);
                        }
			size -= sizeof(page);
                }
        }
	return(1);
}

save_core()
{
	register int n;
	register int ifd, ofd, bounds;
	int ret;
	char *bfile;
	register FILE *fp;
	register char *cp;
	struct stat statbuf;
	char *rddname;

	ifd = -1;
	n = strlen(_PATH_DEV);
	if (strncmp(ddname, _PATH_DEV, n) == 0 &&
	    stat(ddname, &statbuf) >= 0 &&
	    S_ISBLK(statbuf.st_mode) &&
	    (rddname = malloc(strlen(ddname) + 2))) {
		/* construct name of raw device to be used for full image */
		strncpy(rddname, ddname, n);
		rddname[n] = 'r';
		strcpy(&rddname[n + 1], &ddname[n]);
		if (stat(rddname, &statbuf) >= 0 &&
		    S_ISCHR(statbuf.st_mode))
			ifd = open(rddname, O_RDONLY);
	}
	if (ifd < 0) {
		/* here to fall back to the block device (or argv[3] name) */
		rddname = ddname;
		ifd = dumpfd;
	}
	if (Verbose)
		printf("Attempting read of %s dump from %s\n",
			(dumpinfo.partial_dump) ? "partial" : "full", rddname);

	cp = malloc(BUFFERSIZ);
	if (cp == NULL) {
		log(LOG_ERR, nomemmsg, "i/o buffer");
		return(0);
	}
	bounds = read_number("bounds");
	(void)sprintf(cp, "vmcore.%d", bounds);
#if SEC_BASE
	ofd = Create(path(cp), 0600);
#else
	ofd = Create(path(cp), 0640);
#endif
	if(lseek(ifd, dumpoff + DEV_BSIZE, L_SET) == -1){
	        Perror(LOG_ERR, "lseek: %m", "lseek");
	        exit(1);
	}
	log(LOG_NOTICE, "Saving %d bytes of image in vmcore.%d\n",
		dumpbytes, bounds);
	if (dumpinfo.partial_dump)
		ret = partial_core(ifd, ofd, dumpbytes);
	else
		ret = full_core(ifd, ofd, dumpbytes);
	if (ifd != dumpfd)
		close(ifd);
	close(ofd);
	if(copy_image){
		ifd = Open(unixname, O_RDONLY);
		(void)sprintf(cp, "vmunix.%d", bounds);
#if SEC_BASE
		ofd = Create(path(cp), 0600);
#else
		ofd = Create(path(cp), 0640);
#endif
		while((n = Read(ifd, cp, BUFFERSIZ, 0)) > 0)
		Write(ofd, cp, n);
		close(ifd);
		close(ofd);
	}
	bfile = path("bounds");
	fp = fopen(bfile, "w");
	if (fp) {
		fprintf(fp, "%d\n", bounds+1);
		fclose(fp);
	} else
		Perror(LOG_ERR, "Can't create bounds file %s: %m", bfile);
	free(cp);
}



/*
 *  Save the syslog kernel message buffer
 *
 *  There may be valuable information setting in the kernel syslog 
 *  message buffer when the system when down.  Grab the kernel syslog 
 *  message buffer (msgbuf) from the dump and save it.  Syslogd when 
 *  it starts up will look for it and process it.  In the event the
 *  system goes down again before syslogd can run keep appending the
 *  msgbuf's from the dump to the save file.
 */

save_msgbuf()
{
   register char *cp;        /* buffer to read into from dump file */
   register int ofd;         /* output file descriptor */
   int ret, n;               /* return status values */
   long bufmagic;            /* msgbuf magic number */


   cp = (char *)malloc(sizeof(struct msgbuf));   
   if (cp == NULL) {
	   log(LOG_ERR, nomemmsg, "msgbuf i/o buffer");
	   return;
   }

   /* check if msgbuf is valid */
   if (dump_read(ok((vm_offset_t)msgbufaddr), &bufmagic,
		 sizeof(bufmagic), 0) == -1) {
	Perror(LOG_ERR, "savecore: save_msgbuf read", "read");
	free(cp);
        return;
   }

   if (bufmagic != MSG_MAGIC)  {   /* msgbuf not valid */
	if (Verbose)
	    printf("msgbuf not valid, addr: 0x%lx, magic: 0x%lx\n",
		    msgbufaddr, bufmagic);
	free(cp);
        return;
   }

   /* read in the whole msgbuf */
   n = dump_read(ok((vm_offset_t)msgbufaddr), cp, sizeof(struct msgbuf), 1);
   if (n <= 0) {
      if (n == 0)
	 log(LOG_WARNING,"WARNING: EOF on dump device, msgbuf not saved\n");
      else
	 Perror(LOG_ERR, "savecore: save_msgbuf read", "read");
      free(cp);
      return;
   }
   
   /* open the save file */
   if (get_savepath("/etc/syslog.conf", "msgbuf.err"))  {
#if SEC_BASE
        ofd = open(bufsavepath, O_WRONLY|O_APPEND|O_CREAT,0600);
#else
        ofd = open(bufsavepath, O_WRONLY|O_APPEND|O_CREAT,0640);
#endif
        if (ofd < 0) {
           log(LOG_WARNING,"WARNING: can't open msgbuf save file: %s\n", bufsavepath);
           free(cp);
           return;
	}
   } else  {
       free(cp);
       log(LOG_WARNING,"WARNING: can't find msgbuf save path in syslog.conf\n");
       return;
   }

   /* write msgbuf out to save file */
   if ((ret = write(ofd, cp, n)) < n) {
      log(LOG_WARNING,"WARNING: can't save msgbuf\n");
      close(ofd);
      free(cp);
      return;
   }

   /* successfully  done */
   if (Verbose)
      printf("saved syslog message buffer\n");
   close(ofd);
   free(cp);
}






/*
 *  Save the kernel binary event log buffer
 *
 *  There may be valuable information setting in the binary event log
 *  buffer when the system when down.  Grab the buffer (blbuf) from the dump
 *  and save it.  Binlogd when it starts up will look for it and process it.
 *  In the event the system goes down again before binlogd can run keep 
 *  appending the save buffers from the dump to the save file.
 */

save_blbuf()
{
   register struct binlog_bufhdr *bp;
   register int ofd;         /* output file descriptor */
   int ret, n;               /* return status values */
   unsigned long size;
   char *buf;


   if (binbufaddr == NULL)      /* no binlog kernel buffer was allocated */
	  return;

   /* get the buffer size */
   if (dump_read((off_t)binbufaddr, &size, sizeof(size), 0) == -1) {
       Perror(LOG_ERR, "savecore: save_blbuf read", "read");
       return;
   }

   if (size == 0L)  {   /* buffer was never allocated */
	if (Verbose)
	    printf("blbuf was not allocated\n");
        return;
   }
   if (!(buf = malloc(size))) { /* get an I/O buffer */
	log(LOG_ERR, nomemmsg, "binlog buffer");
	return;
   }


   /* read in the whole buffer */
   n = dump_read((off_t)binbufaddr, buf, size, 1);
   if (n <= 0) {
      if (n == 0)
	 log(LOG_WARNING,"WARNING: EOF on dump device, blbuf not saved\n");
      else
	 Perror(LOG_ERR, "savecore: save_blbuf read", "read");
      free(buf);
      return;
   }

   bp = (struct binlog_bufhdr *)buf;
   /* check if buffer is valid or if buffer is empty */
   if ( (bp->magic != BINLOG_MAGIC) || (bp->in == bp->out) )  {
       free(buf);
       return;
   }

   /* change 'in', 'out' and 'le' pointers into offsets from start of buffer */
   bp->in = (bp->in - (unsigned long)virtaddr);
   bp->out = (bp->out - (unsigned long)virtaddr);
   bp->le = (bp->le - (unsigned long)virtaddr);

   /* open the save file */
   if (get_savepath("/etc/binlog.conf", "dumpfile"))  {
#if SEC_BASE
        ofd = open(bufsavepath, O_WRONLY|O_APPEND|O_CREAT,0600);
#else
        ofd = open(bufsavepath, O_WRONLY|O_APPEND|O_CREAT,0640);
#endif
        if (ofd < 0) {
           log(LOG_WARNING,"WARNING: can't open blbuf save file: %s\n", bufsavepath);
           free(buf);
           return;
	}
   } else  {
       free(buf);
       log(LOG_WARNING,"WARNING: can't find blbuf save path in binlog.conf\n");
       return;
   }

   /* write msgbuf out to save file */
   if ((ret = write(ofd, buf, n)) < n) {
      log(LOG_WARNING,"WARNING: can't save msgbuf\n");
      close(ofd);
      free(buf);
      return;
   }

   /* successfully  done */
   if (Verbose)
      printf("saved binlog buffer\n");
   close(ofd);
   free(buf);
}



/*
 * get_savepath()
 *
 * This routine searches an ascii config file (syslog.conf or binlog.conf) for 
 * an entry that points to where the dump file should be saved.  The
 * pathname given by the entry is placed in bufsavepath[].
 */

int get_savepath(conffile, facility)
char *conffile;
char *facility;
{
   register FILE *cfp;
   register char *p;
   char linebuf[MAXLINE];
   char fac[MAXLINE];

   /* open the configuration file */
   if ((cfp = fopen(conffile, "r")) == NULL)
       return(0);     /* error can't open config file */

    /* search the file line by line */
    while (fgets(linebuf, MAXLINE, cfp) != NULL) {
	for (p = linebuf; isspace(*p); ++p);   /* skip blank and comment lines */
        if (*p == NULL || *p == '#')
	         continue;                          /* try another line */
        sscanf(linebuf, "%s %s",fac, bufsavepath);  /* separate the line */
	if ((strcmp(fac , facility)) != 0)          /* is it the one we want */
	         continue;                          /* try another line */
	p = bufsavepath;
        if (*p != '/')                              /* must be a file */
	         continue;                          /* keep looking */ 
	return(1);                                  /* got it, all done */
    }
    return(0);                                      /* can't find it */
}






/*
 * Versions of std routines that exit on error.
 */
Open(name, rw)
	char *name;
	int rw;
{
	int fd;

	fd = open(name, rw);
	if (fd < 0) {
		Perror(LOG_ERR, "%s: %m", name);
		exit(1);
	}
	return (fd);
}

Read(int fd, char *buff, int size, int err_return)
{
	int ret;

	ret = read(fd, buff, size);
	if ((ret < 0) && (err_return == 0)){
		Perror(LOG_ERR, "read: %m", "read");
		exit(1);
	}
	return (ret);
}

#define ADDR_PER_BLK (sizeof(addrs) / sizeof(addrs[0]))

off_t
partial_lseek(int fd, long off, int flag)
{
	vm_offset_t addrs[DEV_BSIZE/sizeof(vm_offset_t)], addr;
	off_t incr, ret;
	int i, n;

	if(lseek(fd, (off_t)dumpoff + DEV_BSIZE, L_SET) == -1){
	        Perror(LOG_ERR, "lseek: %m", "lseek");
		exit(1);
	}
	for(n = dumpsize; !dumpsize || n > 0;){
	        if(Read(fd, (char *) addrs, sizeof(addrs), 0) < sizeof(addrs)){
		        Perror(LOG_ERR, "read: %m", "read");
		        exit(1);
		}
		for(i = 0; i < ADDR_PER_BLK && (!dumpsize || n > 0); i++, n--){
			if((addr = (vm_offset_t)ok(addrs[i])) == NOTOK){
			        fprintf(stderr,
					"savecore : Bogus address in partial"
					" dump\n");
				return(-1);
			}
			if(off >= addr && off - addr < NBPG){
			         incr = i * NBPG + off - addr;
				 ret = lseek(fd, incr, L_INCR);
			         if(ret == -1){
				         Perror(LOG_ERR, "lseek: %m", "lseek");
				         exit(1);
				 }
				 return(ret);
			 }
		}
		if(lseek(fd, (off_t)(i * (off_t)NBPG), L_INCR) == -1){
		        Perror(LOG_ERR, "lseek: %m", "lseek");
			exit(1);
		}
	}
	fprintf(stderr, "savecore: Couldn't locate data for address 0x%lx"
		" in partial dump\n", off);
	return(-1);
}

off_t
Lseek(fd, off, flag)
	int fd, flag;
	long off;
{
	long ret;

	if(dumpinfo.partial_dump) ret = partial_lseek(fd, off - dumpoff, flag);
	else ret = lseek(fd, off, flag);
	if (ret == -1) {
		Perror(LOG_ERR, "lseek: %m", "lseek");
		return -1;
	}
	return (ret);
}

Create(file, mode)
	char *file;
	int mode;
{
	register int fd;

	fd = creat(file, mode);
	if (fd < 0) {
		Perror(LOG_ERR, "%s: %m", file);
		exit(1);
	}
	return (fd);
}

Write(fd, buf, size)
	int fd, size;
	char *buf;
{
	int n;

	if ((n = write(fd, buf, size)) < size) {
		if (n < 0)
			Perror(LOG_ERR, "write: %m", "write");
		else
			log(LOG_ERR, "short write: wrote %d of %d\n", n, size);
		exit(1);
	}
}

log(level, msg, a1, a2)
	int level;
	char *msg;
	long a1, a2;
{

	fprintf(stderr, msg, a1, a2);
	syslog(level, msg, a1, a2);
}

Perror(level, msg, s)
	int level;
	char *msg;
        char *s;
{
	int oerrno = errno;
	
	perror(s);
	errno = oerrno;
	syslog(level, msg, s);
}

int
dump_read(off_t offset, void *ptr, ulong size, int err_return)
{
	ulong remain;
	int len, n;
	char *current;

	offset += dumpoff;
	if (dumpinfo.partial_dump) {
		remain = size;
		current = (char *) ptr;
		while (remain > 0) {
			if (Lseek(dumpfd, offset, L_SET) == -1)
			    return -1;
			if (remain > NBPG)
				len = (offset & ~(NBPG - 1)) + NBPG - offset;
			else
				len = remain;
			if ((n = Read(dumpfd, current, len, err_return)) < 0)
				return(n);
			if (n == 0)
				return((int)(size - remain));
			remain -= n;
			current += n;
			offset += n;
		}
		return((int)size);
	}
	/* DEV_BSIZE skips header*/
	if (Lseek(dumpfd, offset + DEV_BSIZE, L_SET) == -1)
	    return -1;
	n = Read(dumpfd, ptr, size, err_return);
	return(n);
}

#ifdef __alpha

/*
 * Alpha-specific routine to translate a kernel virtual address to its
 * corresponding physical address.  Since this is called for addresses
 * of global kernel variables, the address should be in the kseg range
 * for non-generic kernels or should have the top 32 bits set for generic
 * kernels.  In the latter case, a page table walk is necessary to do
 * the translation.
 */

#define PTESIZE		8		/* size of a pte in bytes */
#define PTESHIFT	3		/* log2 of the pte size */
#define PTEMASK		(NBPG/PTESIZE - 1) /* pte index mask */
#define KSEGMIN		((vm_offset_t)0xfffffc0000000000L) /* base of kseg */
#define KSEGMAX		((vm_offset_t)0xfffffe0000000000L) /* top of kseg */
#define RPBLIMIT	((vm_offset_t)0x200000) /* max phys addr for rpb */

off_t
ok(vm_offset_t va)
{
	static off_t save_base0, save_base1, save_base2, save_base3;
	static ulong save_index1, save_index2, save_index3;
	static vm_offset_t last_va;
	static off_t last_pa;
	static int recurse;

	unsigned long pte, index1, index2, index3;
	vm_offset_t t, pa, offset;
	long rpb_software;

	if (!va)
		return(NOTOK); /* shouldn't happen */
	if (va >= KSEGMIN && va < KSEGMAX)
		return((off_t)(va - KSEGMIN)); /* kseg address */
	if ((va & KSEGMAX) != KSEGMAX)
		return(NOTOK); /* bogus mapped address */
	if (va == last_va)
		return(last_pa); /* cache hit for duplicate call */
	if (recurse++ > 0) /* ensure bogus partial dump data won't recurse */
		goto fail;

	/*
	 * At this point we need to walk the kernel page tables.  A
	 * virtual address is comprised of a page offset (the lowest
	 * 13 bits) and 3 pte indexes (each 10 bits).  The remaining
	 * 21 bits are ignored (they should all be identical to bit 42).
	 */

	t = va;
	offset = (t & (vm_offset_t)PGOFSET);
	t >>= PGSHIFT;
	index3 = (unsigned long)(t & (vm_offset_t)PTEMASK);
	t >>= (PGSHIFT - PTESHIFT);
	index2 = (unsigned long)(t & (vm_offset_t)PTEMASK);
	t >>= (PGSHIFT - PTESHIFT);
	index1 = (unsigned long)(t & (vm_offset_t)PTEMASK);

	/*
	 * Page table walks begin with a root pointer.  The first time
	 * we come through here, we search for the hardware RPB which
	 * must be found within the first 2 megabytes of the dump.  It
	 * contains the physical address of the kernel data structure
	 * named "rpb_software_data", whose first quadword is the page
	 * table root pointer.
	 */

	if (!save_base0) {
		if (dumpinfo.partial_dump) {
			/*
			 * In partial dumps, the rpb page must be the first
			 * page of physical memory saved in the dump.  Its
			 * physical address is the 1st quadword in the
			 * first disk block after the dump header.
			 */
			if (lseek(dumpfd, dumpoff + DEV_BSIZE, L_SET) == -1L ||
			    read(dumpfd, &pa, sizeof(pa)) != sizeof(pa) ||
			    pa < KSEGMIN ||
			    pa >= KSEGMIN + RPBLIMIT ||
			    (pa & PGOFSET) ||
			    !valid_rpb(pa - KSEGMIN, &rpb_software))
				goto fail;
		} else {
			/*
			 * In full dumps, the rpb page must be within the
			 * first 2 megabytes of the dump.  We try 0x50000
			 * first (for Flamingos), 0x2000 second for Cobras
			 * and Rubies), but fall back to a sequential search.
			 */
			if (!valid_rpb((vm_offset_t)0x50000, &rpb_software) &&
			    !valid_rpb((vm_offset_t)0x2000, &rpb_software)) {
				for (pa = 0L; pa < RPBLIMIT; pa += NBPG) {
					if (valid_rpb(pa, &rpb_software))
						break;
				}
				if (pa >= RPBLIMIT)
					goto fail;
			}
		}
		t = 0L;
		if (!(pa = (vm_offset_t)rpb_software) ||
		    (pa & (sizeof(vm_offset_t) - 1)) ||
		    dump_read((off_t)pa, &t, sizeof(t), 1) != sizeof(t) ||
		    !t ||
		    (t & PGOFSET))
			goto fail;
		save_base0 = (off_t)t;
		save_base1 = save_base2 = save_base3 = 0L;
	}

	/*
	 * Now perform 3 levels of page table walk, caching physical
	 * addresses of pte page bases for future calls.  Each pte has
	 * a "valid" bit in bit 0, and a page frame number in bits 32-63.
	 */

	if (!save_base1 || index1 != save_index1) {
		pte = 0L;
		t = save_base0 + (index1 << PTESHIFT);
		if (dump_read((off_t)t, &pte, sizeof(pte), 1) != sizeof(pte) ||
		    !(pte & 1) || !(t = pte >> 32)) {
			save_base1 = save_base2 = save_base3 = 0L;
			goto fail;
		}
		save_index1 = index1;
		save_base1 = t << PGSHIFT;
		save_base2 = save_base3 = 0L;
	}
	if (!save_base2 || index2 != save_index2) {
		pte = 0L;
		t = save_base1 + (index2 << PTESHIFT);
		if (dump_read((off_t)t, &pte, sizeof(pte), 1) != sizeof(pte) ||
		    !(pte & 1) || !(t = pte >> 32)) {
			save_base2 = save_base3 = 0L;
			goto fail;
		}
		save_index2 = index2;
		save_base2 = t << PGSHIFT;
		save_base3 = 0L;
	}
	if (!save_base3 || index3 != save_index3) {
		pte = 0L;
		t = save_base2 + (index3 << PTESHIFT);
		if (dump_read((off_t)t, &pte, sizeof(pte), 1) != sizeof(pte) ||
		    !(pte & 1) || !(t = pte >> 32)) {
			save_base3 = 0L;
			goto fail;
		}
		save_index3 = index3;
		save_base3 = t << PGSHIFT;
	}

	/*
	 * The physical address is calculated from the physical page
	 * number found in the 3rd-level pte and the 13-bit page offset.
	 */
	recurse--;
	last_va = va;
	return(last_pa = (save_base3 | offset));
fail:
	recurse--;
	return(NOTOK);
}

/*
 * The following function determines if there is a valid rpb within the
 * dump at the specified physical address.  This is determined by reading
 * the first 2 quadwords and checking for the physical address in the first
 * one and a magic number in the second one.  If the rpb is valid, the
 * reserved-for-software field is fetched and returned to the caller.
 */

#include <machine/rpb.h>

int
valid_rpb(vm_offset_t pa, long *rpb_software_ptr)
{
	static union {
		long quad;
		char buf[sizeof(long)];
	} rpb_magic;
	struct rpb rpb;

	if (!rpb_magic.quad)
		strncpy(&rpb_magic.buf[0], "HWRPB", 5);

	if (dump_read((off_t)pa, &rpb, sizeof(rpb), 1) != sizeof(rpb) ||
	    rpb.rpb_selfref != (struct rpb *)pa ||
	    rpb.rpb_string != rpb_magic.quad)
		return(0);

	*rpb_software_ptr = rpb.rpb_software;
	return(1);
}

#endif /* __alpha */
