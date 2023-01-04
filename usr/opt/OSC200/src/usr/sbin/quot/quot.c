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
static char	*sccsid = "@(#)$RCSfile: quot.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1994/01/12 00:06:05 $";
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
#ifndef lint

#endif

/*
 * quot
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>
extern priv_t *privvec();
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#define group i_group

#include <sys/time.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <string.h>
#include <sys/stat.h>

#undef	 group

#include <ufs/fs.h>
#include <sys/file.h>

#include <locale.h>
#include "quot_msg.h" 

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_QUOT,n,s)
#ifdef SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_QUOT_SEC,n,s)
#endif

#define	ISIZ	(MAXBSIZE/sizeof(struct dinode))
union {
	struct fs u_sblock;
	char dummy[SBSIZE];
} sb_un;
#define sblock sb_un.u_sblock
#if SEC_FSCHANGE
char itab[MAXBSIZE];
#else
struct dinode itab[MAXBSIZE/sizeof(struct dinode)];
#endif

struct du {
	struct	du *next;
	long	blocks;
	long	blocks30;
	long	blocks60;
	long	blocks90;
	long	nfiles;
	int	uid;
	int	gid;
#define	NDU	2048
} du[NDU];
int	ndu;
#define	DUHASH	8209	/* smallest prime >= 4 * NDU */
#define	HASH(u)	((u) % DUHASH)
struct	du *duhash[DUHASH];

#define	TSIZE	500
int	sizes[TSIZE];
long	overflow;

int	gflg;
int	nflg;
int	fflg;
int	cflg;
int	vflg;
int	hflg;
long	now;

unsigned	ino;

char	*malloc();
char	*getname();
char	*groupname();
char	*rawname();

main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	int ch;
	time_t time();

	setlocale( LC_ALL, "" );
	catd = catopen(MF_QUOT,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
        if (!authorized_user("sysadmin")) {
		fprintf(stderr, MSGSTR_SEC(AUTH,
			"%s: need sysadmin authorization\n"),
			command_name);
		exit(1);
	}
#endif

	while ((ch = getopt(argc, argv, "cfghnv")) != EOF)
		switch((char)ch) {
		case 'c':
			cflg++; break;
		case 'f':
			fflg++; break;
		case 'g':
			gflg++; break;
		case 'h':
			hflg++; break;
		case 'n':
			nflg++; break;
		case 'v':
			vflg++; break;
		case '?':
		default:
			fputs(MSGSTR(USAGE, "usage: quot [-cfghnv] [filesystem ...]\n"), stderr);
			exit(1);
		}
	argc -= optind;
	argv += optind;

	(void)time(&now);
	if (argc)
		for (; *argv; ++argv) {
			if (check(*argv, (char *)NULL) == 0)
				report();
		}
	else
		quotall();
	exit(0);
}

#include <sys/dir.h>
#include <fstab.h>

quotall()
{
	register struct fstab *fs;
	char rawbuf[PATH_MAX + 1]; 
	char *dp; 

	while (fs = getfsent()) {
		if (strcmp(fs->fs_vfstype, "ufs") ||
		   (strcmp(fs->fs_type, FSTAB_RO) &&
		    strcmp(fs->fs_type, FSTAB_RW)))
			continue;
		/* 
                 * create a character special file from  
                 * the block special file fs->fs_spec 
                 * found in /etc/fstab   
                 */ 
		if ((dp = rindex(fs->fs_spec, '/')) == 0) { 
			rawbuf[0] = 'r'; 
			(void)strcpy(rawbuf+1, fs->fs_spec); 
		} else { 
		/*
			*dp = 0; 
			(void)strcpy(rawbuf, fs->fs_spec); 
			*dp = '/'; 
			(void)strcat(rawbuf, "/r"); 
			(void)strcat(rawbuf, dp + 1); 
		 */
			(void)strcpy(rawbuf, rawname(fs->fs_spec));
		} 

 		if (check(rawbuf, fs->fs_file, fs->fs_vfstype) == 0) 
			report();
	}
}

check(file, fsdir)
	char *file;
	char *fsdir;
{
	register int i, j, nfiles;
	register struct du **dp;
	daddr_t iblk;
	long dev_bsize;
	int c, fd;
	struct stat statbuf;

	/*
	 * Initialize tables between checks;
	 * because of the qsort done in report()
	 * the hash tables must be rebuilt each time.
	 */
	for (i = 0; i < TSIZE; i++)
		sizes[i] = 0;
	overflow = 0;
	for (dp = duhash; dp < &duhash[DUHASH]; dp++)
		*dp = 0;
	ndu = 0;

	if ((stat(file, &statbuf)) < 0) { 
		fprintf(stderr, 
			MSGSTR(BADSTAT, "quot: could not stat %s\n"), 
			file); 
		exit(1); 
	} 
	if (((statbuf.st_mode & S_IFMT) != S_IFCHR) &&
	    ((statbuf.st_mode & S_IFMT) != S_IFBLK)) {
		fprintf(stderr,
			MSGSTR(BADTYPE, "quot: not a character or block device\n"),
			file);
		exit(1);
	}
#if SEC_BASE
	{
	    privvec_t saveprivs;

	    if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr,
			MSGSTR(PRIV, "%s: insufficient privileges\n"),
			command_name);
		exit(1);
	    }
	    fd = open(file, O_RDONLY);
	    seteffprivs(saveprivs, (priv_t *) 0);
	}
#else
	fd = open(file, O_RDONLY);
#endif
	if (fd < 0) {
		fprintf(stderr, "quot: ");
		perror(file);
		return (-1);
	}
	printf("%s", file);
	if (fsdir == NULL) {
		register struct fstab *fs = getfsspec(file);
		if (fs != NULL)
			fsdir = fs->fs_file;
	}
	if (fsdir != NULL && *fsdir != '\0')
		printf(" (%s)", fsdir);
	printf(":\n");
	sync();
	bread(fd, (off_t)SBOFF, (char *)&sblock, SBSIZE);
#if SEC_FSCHANGE
	disk_set_file_system(&sblock, sblock.fs_bsize);
#endif
	dev_bsize = sblock.fs_fsize / fsbtodb(&sblock, 1);
	if (nflg) {
		if (isdigit(c = getchar()))
			(void)ungetc(c, stdin);
		else while (c != '\n' && c != EOF)
			c = getchar();
	}
	nfiles = sblock.fs_ipg * sblock.fs_ncg;
	for (ino = 0; ino < nfiles; ) {
#if SEC_FSCHANGE
		struct dinode *dip = (struct dinode *) itab;
#endif
		iblk = fsbtodb(&sblock, itod(&sblock, ino));
		bread(fd, ((off_t)iblk * dev_bsize), (char *)itab,
		      (int)sblock.fs_bsize);
		for (j = 0; j < INOPB(&sblock) && ino < nfiles; j++, ino++) {
#if SEC_FSCHANGE
			if (ino >= ROOTINO)
				acct(dip);
			disk_inode_incr(&dip, 1);
#else
			if (ino < ROOTINO)
				continue;
			acct(&itab[j]);

#endif
		}
	}
	close(fd);
	return (0);
}

acct(ip)
	register struct dinode *ip;
{
	register struct du *dp;
	struct du **hp;
	long blks, frags, size;
	int n;
	static fino;

	if ((ip->di_mode & IFMT) == 0)
		return;
	/*
	 * By default, take block count in inode.  Otherwise (-h),
	 * take the size field and estimate the blocks allocated.
	 * The latter does not account for holes in files.
	 */
	if (!hflg)
		size = ip->di_blocks * DEV_BSIZE / 1024;
	else {
		blks = lblkno(&sblock, ip->di_size);
		frags = blks * sblock.fs_frag +
			numfrags(&sblock, dblksize(&sblock, ip, blks));
		size = frags * sblock.fs_fsize / 1024;
	}
	if (cflg) {
		if ((ip->di_mode&IFMT) != IFDIR && (ip->di_mode&IFMT) != IFREG)
			return;
		if (size >= TSIZE) {
			overflow += size;
			size = TSIZE-1;
		}
		sizes[size]++;
		return;
	}
	hp = &duhash[HASH(ip->di_uid)];
	for (dp = *hp; dp; dp = dp->next)
		if (dp->uid == ip->di_uid && (!gflg || dp->gid == ip->di_gid))
			break;
	if (dp == 0) {
		if (ndu >= NDU)
			return;
		dp = &du[ndu++];
		dp->next = *hp;
		*hp = dp;
		dp->uid = ip->di_uid;
		dp->gid = ip->di_gid;
		dp->nfiles = 0;
		dp->blocks = 0;
		dp->blocks30 = 0;
		dp->blocks60 = 0;
		dp->blocks90 = 0;
	}
	dp->blocks += size;
#define	DAY (60 * 60 * 24)	/* seconds per day */
	if (now - ip->di_atime > 30 * DAY)
		dp->blocks30 += size;
	if (now - ip->di_atime > 60 * DAY)
		dp->blocks60 += size;
	if (now - ip->di_atime > 90 * DAY)
		dp->blocks90 += size;
	dp->nfiles++;
	while (nflg) {
		register char *np;

		if (fino == 0)
			if (scanf("%d", &fino) <= 0)
				return;
		if (fino > ino)
			return;
		if (fino < ino) {
			while ((n = getchar()) != '\n' && n != EOF)
				;
			fino = 0;
			continue;
		}
		if (np = getname(dp->uid))
			printf("%.7s\t", np);
		else
			printf("%u\t", ip->di_uid);
		if (gflg) {
			if (np = groupname(dp->gid))
				printf("%.7s\t", np);
			else
				printf("%u\t", ip->di_gid);
		}
		while ((n = getchar()) == ' ' || n == '\t')
			;
		putchar(n);
		while (n != EOF && n != '\n') {
			n = getchar();
			putchar(n);
		}
		fino = 0;
		break;
	}
}

bread(fd, bno, buf, cnt)
	off_t bno;
	char *buf;
{
	off_t lseek();

	(void)lseek(fd, bno, SEEK_SET);
	if (read(fd, buf, cnt) != cnt) {
		fprintf(stderr, MSGSTR(READERR, "quot: read error at block %ld\n"), bno);
		exit(1);
	}
}

qcmp(p1, p2)
	register struct du *p1, *p2;
{
	char *s1, *s2;

	if (p1->blocks > p2->blocks)
		return (-1);
	if (p1->blocks < p2->blocks)
		return (1);
	s1 = getname(p1->uid);
	if (s1 == 0)
		return (0);
	s2 = getname(p2->uid);
	if (s2 == 0)
		return (0);
	return (strcmp(s1, s2));
}

report()
{
	register i;
	register struct du *dp;

	if (nflg)
		return;
	if (cflg) {
		register long t = 0;

		for (i = 0; i < TSIZE - 1; i++)
			if (sizes[i]) {
				t += i*sizes[i];
				printf("%d\t%d\t%ld\n", i, sizes[i], t);
			}
		printf("%d\t%d\t%ld\n",
		    TSIZE - 1, sizes[TSIZE - 1], overflow + t);
		return;
	}
	qsort(du, ndu, sizeof (du[0]), qcmp);
	for (dp = du; dp < &du[ndu]; dp++) {
		register char *cp;

		if (dp->blocks == 0)
			return;
		printf("%5d\t", dp->blocks);
		if (fflg)
			printf("%5d\t", dp->nfiles);
		if (cp = getname(dp->uid))
			printf("%-8.8s", cp);
		else
			printf("#%-8d", dp->uid);
		if (gflg) {
			if (cp = groupname(dp->gid))
				printf(" %-8.8s", cp);
			else
				printf(" #%-8d", dp->gid);
		}
		if (vflg)
			printf("\t%5d\t%5d\t%5d",
			    dp->blocks30, dp->blocks60, dp->blocks90);
		printf("\n");
	}
}

#include <grp.h>
#include <utmp.h>

struct	utmp utmp;

#define NGID	2048
#define	NMAX	(sizeof (utmp.ut_name))

char	gnames[NGID][NMAX+1];
char	outrangename[NMAX+1];
int	outrangegid = -1;

char *
groupname(gid)
	int gid;
{
	register struct group *gr;
	static init;
	struct group *getgrent();

	if (gid >= 0 && gid < NGID && gnames[gid][0])
		return (&gnames[gid][0]);
	if (gid >= 0 && gid == outrangegid)
		return (outrangename);
rescan:
	if (init == 2) {
		if (gid < NGID)
			return (0);
		setgrent();
		while (gr = getgrent()) {
			if (gr->gr_gid != gid)
				continue;
			outrangegid = gr->gr_gid;
			strncpy(outrangename, gr->gr_name, NMAX);
			endgrent();
			return (outrangename);
		}
		endgrent();
		return (0);
	}
	if (init == 0)
		setgrent(), init = 1;
	while (gr = getgrent()) {
		if (gr->gr_gid < 0 || gr->gr_gid >= NGID) {
			if (gr->gr_gid == gid) {
				outrangegid = gr->gr_gid;
				strncpy(outrangename, gr->gr_name, NMAX);
				return (outrangename);
			}
			continue;
		}
		if (gnames[gr->gr_gid][0])
			continue;
		strncpy(gnames[gr->gr_gid], gr->gr_name, NMAX);
		if (gr->gr_gid == gid)
			return (&gnames[gid][0]);
	}
	init = 2;
	goto rescan;
}

/*
 * getname:
 *	purpose:
 *		getname returns the user from the password file
 *		associated with a specified user-id. The user-id
 *		is passed as an integer parameter to the routine.
 *
 *	If a user whose id matches the parameter uid is found in the
 *	password file, the name corresponding to that user is returned.
 *	If no match is found, NULL is returned.  If malloc() fails,
 *	NULL is returned.  If id is -1, all allocated memory is freed.
 *
 *	During the search for a particular user-id, a hash table names
 *	is built for storing user-id's and their corresponding names
 *	from the password file. When the routine is first called, entries
 *	are read from the password file until a match for uid, or the
 *	end of the password file, is found. Any entries not matching uid
 *	and not already stored in the names structure are then stored in
 *	names.
 *
 *	On subsequent calls to getname, the structure names is checked
 *	first for a match for uid. If no match is found, and  if the entire
 *	password file has not been stored, entries from it are stored in
 *	names as before.
 *
 *	When the entire password file has been stored, only names
 *	is checked for a matching user-id on subsequent calls to getname.
 */

#include <pwd.h>			/* passwd struct include file */
#include <utmp.h>			/* utmp struct include file */
#include <stdio.h>

#define HASHBITS 6			/* number of bits to hash */
#define HASHSIZE (1<<HASHBITS)		/* limit on the number of buckets
					   in the hash table */
#define HASHMASK (HASHSIZE-1)		/* mask of bits to hash */
#define hash(uid) ((uid)&HASHMASK)	/* determines in which of the HASHSIZE
					   buckets uid would belong */
static struct utmp ut;

/* struct for storing an entry of the hash table */
struct entry {
	char a_name[NMAX+1];		/* stores the name of the user  */
	int a_uid;			/* stores the uid of the user   */
	struct entry *next;		/* stores the pointer to the
					   next entry of the hash table */
};

static struct entry *names[HASHSIZE];	/* the hash table of users */

/* returns the entry of names that stores the user of id uid if
   one exists in names, else NULL is returned */
static struct entry *inset(uid)
	register int uid;
{
	register struct entry *temp;

	for (temp = names[hash(uid)]; temp != NULL; temp = temp->next)
		if (temp->a_uid == uid)
			return(temp);
	return(NULL);
}

/* allocates space for an entry in names, setting next field to NULL */
static struct entry *make_blank_entry()
{
	register struct entry *blank_entry;

	blank_entry = (struct entry*)(malloc(sizeof(struct entry)));
	if (blank_entry == NULL)
		return(NULL);
	blank_entry->next = NULL;
	return(blank_entry);
}

/* inserts a blank entry into the correct position of names for a user
   whose id is uid */
static struct entry *place_blank_entry(uid)
	register int uid;
{
	register struct entry **temp;
	struct entry *make_blank_entry();

	temp = &names[hash(uid)];
	while (*temp != NULL)
		temp = &((*temp)->next);
	return(*temp = make_blank_entry());
}

/* inserts the data of an entry from the password file (stored in pw_entry)
   into an entry of names (the parameter blank_entry) */
static fill_in(blank_entry, pw_entry)
	register struct entry *blank_entry;
	register struct passwd *pw_entry;
{
	strncpy(blank_entry->a_name, pw_entry->pw_name, NMAX);
	blank_entry->a_name[NMAX] = '\0';
	blank_entry->a_uid = pw_entry->pw_uid;
}

/* creates an entry in names storing the data of an entry from the
   password file which stored in the paramter passwd_entry */
static struct entry *create_names_entry(passwd_entry)
	register struct passwd *passwd_entry;
{
	register struct entry *new_entry;
	struct entry *place_blank_entry();

	new_entry = place_blank_entry(passwd_entry->pw_uid);
	if (new_entry == NULL)
		return(NULL);
	fill_in(new_entry, passwd_entry);
	return(new_entry);
}

/* initializes hash table */
static set_hash_table()
{
	register int i;

	for (i = 0; i < HASHSIZE; i++)
		names[i] = NULL;
}

/* free hash table */
static free_hash_table()
{
	register int i;
	register struct entry *temp;

	for (i = 0; i < HASHSIZE; i++)
		while ((temp = names[i]) != NULL) {
			names[i] = temp->next;
			free((char *)temp);
		}
}

/* returns the name of the user in the passwords file whose id is uid.
   NULL is returned if none exists */
char *getname(uid)
register uid;
{
	register struct passwd *pw;	/* pre-defined struct */
	static init;			/* indicates status of the password file
						0 = file unopen
						1 = file open
						2 = file entirely read
					 */
	/* pre-defined routines for accessing the password routine */
	struct passwd *getpwent();

	struct entry *inset();
	register struct entry *Location;

	if (uid == -1) {
		if (init != 0)
			free_hash_table();
		if (init == 1)
			endpwent();
		init = 0;
		return(NULL);
	}

	if (init == 0) {
		set_hash_table();
		setpwent();
		init = 1;
	}

	Location = inset(uid);		/* check if user is in names */
	if (Location != NULL)
		return(Location->a_name); /* user already stored in names */
	if (init == 2)
		return(NULL);		/* entire password file has been
					   stored in names */

       /* continue reading entries from the password file, storing any in
	  names whose uid is not already located in names, stopping when
	  a match is found for the uid or the entire password file has
	  been stored */

	while ((pw = getpwent()) != NULL) {
		Location = inset(pw->pw_uid);
		if (Location != NULL)
			continue;
		if (create_names_entry(pw) == NULL)
			return(NULL);
		if (pw->pw_uid == uid)
			return(pw->pw_name);
	}
	init = 2;
	endpwent();
	return(NULL);
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

