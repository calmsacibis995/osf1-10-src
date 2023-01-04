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
static char	*sccsid = "@(#)$RCSfile: lastcomm.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/10/11 17:18:45 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/param.h>
#include <sys/acct.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <utmp.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <locale.h>

#include <NLchar.h>
#include <NLctype.h>
struct tm *localtime();
char timbuf[BUFSIZ];
#ifdef DEBUG
char *ctime();
#endif

#include "lastcomm_msg.h" 
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_LASTCOMM, Num, Str) 

struct  acct *buf;
void *calloc();

extern double	expacct();
char	*flagbits();
char	*getname();
char	*getdev();
void	setupdevs();

#define ACCTFILE	"/var/adm/pacct"

#define	fldoff(str, fld)	((int)&(((struct str *)0)->fld))
#define	fldsiz(str, fld)	(sizeof(((struct str *)0)->fld))
#define	strbase(str, ptr, fld)	((struct str *)((char *)(ptr)-fldoff(str, fld)))
#define NWIDTH  8                       /* printing width of ut_user */
#define LWIDTH  12                      /* printing width of ut_line */
#define PRECISION	0.01	/* precision for rounding double values */


main(argc, argv)
int  argc;
char *argv[];
{
	register int bn, cc;
	register struct acct *acp;
	int fd;
	struct stat sb;
	int lastblock;

#ifndef KJI
	(void) setlocale (LC_ALL,"");
#endif

	if ((fd = open(ACCTFILE, O_RDONLY)) < 0) {
		perror(ACCTFILE);
		exit(1);
	}
	catd = catopen(MF_LASTCOMM,NL_CAT_LOCALE);

	(void)fstat(fd, &sb);

	/* check that pacct data exists */  			/* FPM001 */ 
        if (!sb.st_size)
        {
	        fprintf(stderr, MSGSTR(NODATA,
		"No information available. Accounting may be turned off.\n"));
		exit(0);
	}

	if ((buf = calloc(sb.st_size / sizeof(struct acct), sizeof(struct acct))) != NULL) {
	        cc = read(fd, buf, sb.st_size);
		if (cc < 0) {
			perror(MSGSTR(READ, "read"));
		}
		acp = buf + (cc / sizeof(buf[0])) - 1;
		for (; acp >= buf; acp--) {
			register char *cp;
			double x;
			struct tm *tm;
#ifdef DEBUG

printf("co:|%s| ut:%d st:%d et:%d bt:%ld=%s", acp->ac_comm, acp->ac_utime, 
	acp->ac_stime, acp->ac_etime, acp->ac_btime, ctime(&acp->ac_btime));
printf("uid:%d gid:%d mem:%d io:%d tty:%d fl:%x\n\n", acp->ac_uid, acp->ac_gid,
	acp->ac_mem, acp->ac_io, acp->ac_tty, acp->ac_flag);

#endif /* DEBUG */
			if (acp->ac_comm[0] == '\0')
				(void)strcpy(acp->ac_comm, "?");
			for (cp = &acp->ac_comm[0];
			     cp < &acp->ac_comm[fldsiz(acct, ac_comm)] && *cp;
			     cp += NLchrlen(cp))
				if (!isprint(*cp))
					*cp = '?';
			if (argc > 1 && !ok(argc, argv, acp))
				continue;
			x = expacct(acp->ac_utime) + expacct(acp->ac_stime);
				
			tm = localtime(&acp->ac_btime);
			(void)strftime(timbuf, BUFSIZ, "%c %Z %n", tm);
			(void)printf("%-*.*s %s %-*s %-*s %6.2f secs %.16s\n",
				fldsiz(acct, ac_comm), fldsiz(acct, ac_comm),
				acp->ac_comm,
				flagbits(acp->ac_flag),
				NWIDTH, getname(acp->ac_uid),
				LWIDTH, getdev(acp->ac_tty),
				(x <= PRECISION) ? PRECISION : x, timbuf);
		}
		free(buf);
	}
	else
	        fprintf(stderr,"\nlastcomm: insufficient memory to read %s\n",ACCTFILE);
	return(0);
}


/*
 *  NAME:  flagbits
 *
 *  FUNCTION:  Fill in flag array with symbols of particular flags set.
 *	      
 *  RETURN VALUE:  	 Pointer to the flag array is returned.
 */

char *
flagbits(f)
register int f;
{
	register int i = 0;
	static char flags[20];

#define BIT(flag, ch)	flags[i++] = (f & flag) ? ch : ' '
	BIT(ASU, 'S');
	BIT(AFORK, 'F');
	flags[i] = '\0';
	return (flags);
}

/*
 *  NAME:  ok
 *
 *  FUNCTION:  Check to see if given accounting record should be
 *		printed out.
 *	      
 *  RETURN VALUE:  	 0 if record is NOT to be printed out.
 *		
 */

ok(argc, argv, acp)
register int argc;
register char *argv[];
register struct acct *acp;
{
	register int j;

	for (j = 1; j < argc; j++)
		if (strcmp(getname(acp->ac_uid), argv[j]) &&
		    strcmp(getdev(acp->ac_tty), argv[j]) &&
		    strncmp(acp->ac_comm, argv[j], fldsiz(acct, ac_comm)))
			break;
	return (j == argc);
}

/* should be done with nameserver or database */

struct	utmp utmp;

#define NUID	2048
#define	NMAX	(sizeof(utmp.ut_name))

char	names[NUID][NMAX+1];
char	outrangename[NMAX+1];
int	outrangeuid = -1;

/*
 *  NAME:  getname
 *
 *  FUNCTION AND 
 *	RETURN VALUE:   Give a uid return a pointer to the
 *			associated user name.  Return NULL if it
 *			doesn't exist.
 */

char *
getname(uid)
int uid;
{
	register struct passwd *pw;
	static init;
	struct passwd *getpwent();
	char uid_str[NWIDTH+1];

	if (uid >= 0 && uid < NUID && names[uid][0])
		return (&names[uid][0]);
	if (uid >= 0 && uid == outrangeuid)
		return (outrangename);
	(void)sprintf(uid_str, "%d", uid);
	if (init) {
		if (uid < NUID)
			return (uid_str);
		(void)setpwent();
		while (pw = getpwent()) {
			if (pw->pw_uid != uid)
				continue;
			outrangeuid = pw->pw_uid;
			(void)strncpy(outrangename, pw->pw_name, NMAX);
			(void)endpwent();
			return (outrangename);
		}
		(void)endpwent();
		return (uid_str);
	}

	(void)setpwent();
	while (pw = getpwent()) {
		if (pw->pw_uid < 0 || pw->pw_uid >= NUID) {
			if (pw->pw_uid == uid) {
				outrangeuid = pw->pw_uid;
				(void)strncpy(outrangename, pw->pw_name, NMAX);
				(void)endpwent();
				return (outrangename);
			}
			continue;
		}
		if (names[pw->pw_uid][0])
			continue;
		(void)strncpy(names[pw->pw_uid], pw->pw_name, NMAX);
		if (pw->pw_uid == uid) {
			(void)endpwent();
			return (&names[uid][0]);
		}
	}
	init++;
	(void)endpwent();
	return (uid_str);
}


#define N_DEVS		43		/* hash value for device names */
#define NDEVS		500		/* max number of file names in /dev */

struct	devhash {
	dev_t	dev_dev;
	char	dev_name [fldsiz(utmp, ut_line) + 1];
	struct	devhash * dev_nxt;
};
struct	devhash *dev_hash[N_DEVS];
struct	devhash *dev_chain;
#define HASH(d)	(((int) d) % N_DEVS)
char * strchr();

/*
 *  NAME:  setupdevs
 *
 *  FUNCTION:  Set up a hash table of all the devices in /dev.
 *	      
 *  RETURN VALUE: void
 *		
 */

static void
setupdevs()
{
	register DIR *fd;
	register struct devhash *hashtab;
	register ndevs = NDEVS;
	struct dirent * dp;

	if ((fd = opendir("/dev")) == NULL) {
		perror("/dev");
		exit(1);
	}
	hashtab = (struct devhash *) malloc(NDEVS * sizeof(struct devhash));
	if (hashtab == (struct devhash *)0) {
		(void)fprintf(stderr, MSGSTR(NOMEM, "No mem for dev table\n"));
		closedir(fd);
		exit(1);
	}
	while (dp = readdir(fd)) {
		if (dp->d_fileno == 0)
			continue;
		if (dp->d_name[0] != 't' && strcmp(dp->d_name, "console"))
			continue;
		(void)strncpy(hashtab->dev_name, dp->d_name, 
						fldsiz(utmp, ut_line));
		hashtab->dev_name[fldsiz(utmp, ut_line)] = 0;
		hashtab->dev_nxt = dev_chain;
		dev_chain = hashtab;
		hashtab++;
		if (--ndevs <= 0)
			break;
	}
	closedir(fd);
}

/*
 *  NAME:  getdev
 *
 *  FUNCTION: Find the device name associated with a device number.
 *	      
 *  RETURN VALUE: pointer to the device name.
 */

char *
getdev(dev)
dev_t dev;
{
	register struct devhash *hp, *nhp;
	struct stat statb;
	char name[fldsiz(devhash, dev_name) + 6];
	static dev_t lastdev = (dev_t) -1;
	static char *lastname;
	static int init = 0;

	if (dev == NODEV)
		return ("__");
	if (dev == lastdev)
		return (lastname);
	if (!init) {
		setupdevs();
		init++;
	}
	for (hp = dev_hash[HASH(dev)]; hp; hp = hp->dev_nxt)
		if (hp->dev_dev == dev) {
			lastdev = dev;
			return (lastname = hp->dev_name);
		}
	for (hp = dev_chain; hp; hp = nhp) {
		nhp = hp->dev_nxt;
		(void)strcpy(name, "/dev/");
		(void)strcat(name, hp->dev_name);
		if (stat(name, &statb) < 0)	/* name truncated usually */
			continue;
		if ((statb.st_mode & S_IFMT) != S_IFCHR)
			continue;
		hp->dev_dev = statb.st_rdev;
		hp->dev_nxt = dev_hash[HASH(hp->dev_dev)];
		dev_hash[HASH(hp->dev_dev)] = hp;
		if (hp->dev_dev == dev) {
			dev_chain = nhp;
			lastdev = dev;
			return (lastname = hp->dev_name);
		}
	}
	dev_chain = (struct devhash *) 0;
	return ("??");
}
