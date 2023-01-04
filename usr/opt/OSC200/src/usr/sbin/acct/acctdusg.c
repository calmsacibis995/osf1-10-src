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
static char	*sccsid = "@(#)$RCSfile: acctdusg.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:59:46 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: ccount, charge, clean, dsort, inblks, makdlst, openerr,
 *            output, pdisk, strndx, swapd
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	acctdusg [-u file] [-p file] > dtmp-file
 *	-u	file for names of files not charged to anyone
 *	-p	get password info from file
 *	reads input (normally from find / -print)
 *	and compute disk resource consumption by login
 */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <locale.h>
#include "acct_msg.h"
#define MSGSTR(Num, Str) NLgetamsg(MF_ACCT, MS_ACCT, Num, Str)

#include <userpw.h>

struct disk{
	char	dsk_name[PW_NAMELEN];	/* login name */
	uid_t   dsk_uid;		/* user id of login name */
	int	dsk_dirsz;	/* # letters in pathname of login directory */
	char	*dsk_logdir;	/* ptr to path name of login directory */
	int	dsk_ns;			/* no of slashes in path name */
	long	dsk_du;			/* disk usage */
	struct	disk *dsk_np;		/* ptr to next struct */

};

#include <pwd.h>
char	*pfile =NULL;
/* 
 * since the passwd(3) functions don't allow the specification of an
 * alternate password file, we have our own password file processor.
 * this would be unportable if we ever thought the username and uid fields 
 * would ever be anyplace other than where they are. <snicker> 
 *
 * To use the Yellow Pages service, the following rules apply:
 *   	If there is no  -p argument, we take the standard library
 *      passwd(3) functions. Otherwise, our own functions are used.
 * So the call with "... -p /etc/passwd ..." accesses the "standard"
 * password file as well as a call without the -p option, but in the
 * first case, only the local users are found. nm
 *
 */
extern void endpwent();
static struct passwd	*my_getpwent();		/* local getpwent routine  */
static void	my_endpwent();			/* local endpwent routine  */
static int	 my_setpwent();			/* local setpwent routine  */
struct passwd *(*act_getpwent)() = getpwent;	/* active getpwent routine */
void (*act_endpwent)() = endpwent;		/* active endpwent routine */
static char     *pwskip();
static setup();
char	*afile, *nfile;
 
struct disk	*Dp;	/* ptr to 1st entry in dirlist */
struct stat	statb;

FILE	*pswd, *nchrg;
FILE	*names =stdin;
FILE	*acctf =stdout;

char	fbuf[BUFSIZ];
char	*calloc();
static int	openerr(), output(), strndx();

static int	makdlst(), ccount(), dsort();
static int	swapd(), charge(), clean();
static long	inblks();
#ifdef DEBUG
static int	pdisk();
#endif

main(int argc, char **argv)
{

	(void) setlocale (LC_ALL,"");
	while(--argc > 0){
		++argv;
		if(**argv == '-') switch((*argv)[1]) {
		case 'u':
			if (--argc <= 0)
				break;
			if ((nchrg = fopen(*(++argv),"w")) == NULL)
				openerr(*argv);
			(void)chmod(*argv, 0644);
			continue;
		case 'p':
		/* use the alternate access routines */
			act_getpwent = my_getpwent;
			act_endpwent = my_endpwent;
			if (--argc <= 0)
				break;
			pfile = *(++argv);
			continue;
		}
		(void)fprintf(stderr,MSGSTR( INVALARG, "Invalid argument: %s\n"), *argv);
		exit(1);
	}

	setup(pfile);

	dsort();

	while( fgets(fbuf, sizeof fbuf, names) != NULL) {
		fbuf[strndx(fbuf, '\n') ] = '\0';
		clean(fbuf);
		charge(fbuf);
	}
	if (names != stdin)
		(void)fclose(names);

	output();

	if (acctf != stdout)
		(void)fclose(acctf);
	if (nchrg)
		(void)fclose(nchrg);
#ifdef DEBUG
		pdisk();
#endif

	return(0);
}

static
openerr(file)
char	*file;
{
	(void)fprintf(stderr, MSGSTR( CANTOPEN, "%s: Cannot open %s\n"), 
			"acctdusg", file);
	exit(1);
}

static
output()
{

	register struct disk *dp;

	for(dp = Dp; dp != NULL; dp=dp->dsk_np) {
		if(dp->dsk_du)
			(void)fprintf(acctf,
				"%11lu \t%-8.8s \t%7lu\n",
				dp->dsk_uid,
				dp->dsk_name,
				dp->dsk_du);
	}
}

static
strndx(str, chr)
register char *str;
register char chr;
{

	register index;

	for (index=0; *str; str++,index++)
		if (*str == chr)
			return index;
	return -1;
}

/*
 *	make a list of home directory names
 *	for every entry in password file
 */

static
makdlst(p)
register struct passwd *p;
{

	static struct	disk *dl = {NULL};
	struct disk	*dp;
	int    i;

	if( (dp = (struct disk *)calloc(sizeof(struct disk), 1)) == NULL) {
	nocore:
		(void)fprintf(stderr, MSGSTR( NOCORE,"out of core\n"));
		exit(2);
	}
	
	/* copy login information from pw structure to disk structure */
	(void)strcpy(dp->dsk_name, p->pw_name);
	dp->dsk_uid = p->pw_uid;
	dp->dsk_dirsz = strlen(p->pw_dir); /* length of path name */
	if((dp->dsk_logdir = calloc(dp->dsk_dirsz + 1, 1)) == NULL)
		goto nocore;

	(void)strcpy(dp->dsk_logdir, p->pw_dir);

	if(stat(dp->dsk_logdir,&statb)== -1 ||
			(statb.st_mode & S_IFMT) != S_IFDIR) {
		cfree(dp->dsk_logdir);
		cfree(dp);
		return;
	}
	for(i=0; dp->dsk_logdir[i]; i++)
		if(dp->dsk_logdir[i] == '/')
			dp->dsk_ns++; /* count # of slashes */
	/* treat logdirs "/usr" and "/" different, 
	   because / is the lowest level !!	*/
	if( strcmp(dp->dsk_logdir, "/") == 0 )
		dp->dsk_ns = 0;

	if(dl == NULL) { /* link ptrs */
		Dp = dl = dp;
	} else {
		dl->dsk_np = dp;
		dl = dp;
	}
	return;
}


/*
 *	read to end of line counting
 *	fields (delimited by :
 */
static
ccount(p,c)
register char *p, c;
{

	register i;

	i = 0;
	while(*p)
		if(*p++ == c)
			i++;
	return i;
}
/*
 *	sort by decreasing # of levels in login
 *	pathname and then by increasing uid
 */
static
dsort()
{

	register struct disk *dp1, *dp2, *pdp;
	int	change;

	if(Dp == NULL || Dp->dsk_np == NULL)
		return;

	change = 0;
	pdp = NULL;

	for(dp1 = Dp; ;) {
		if((dp2 = dp1->dsk_np) == NULL) {
			if(!change)
				break;
			dp1 = Dp;
			pdp = NULL;
			change = 0;
			continue;
		}
		if((dp1->dsk_ns < dp2->dsk_ns) ||
		   (dp1->dsk_ns==dp2->dsk_ns && dp1->dsk_uid > dp2->dsk_uid)) {
			swapd(pdp, dp1, dp2);
			change = 1;
			dp1 = dp2;
			continue;
		}
		pdp = dp1;
		dp1 = dp2;
	}
}

static
swapd(p,d1,d2)
register struct disk *p, *d1, *d2;
{
	struct disk *t;

	switch((int)p) {
	default:
		p->dsk_np = d2;
		t = d2->dsk_np;
		d2->dsk_np = d1;
		d1->dsk_np = t;
		break;
	case NULL:
		t = d2->dsk_np;
		d2->dsk_np = d1;
		d1->dsk_np = t;
		Dp = d2;
		break;
	}
}

static
charge(n)
register char *n;
{
	register struct disk *dp;
	register i;
	long	blks;

	if(stat(n,&statb) == -1)
		return;

	i = strlen(n);
	for(dp = Dp; dp != NULL; dp = dp->dsk_np) {
		if(i < dp->dsk_dirsz)
			continue;
		if(strncmp(dp->dsk_logdir, n, dp->dsk_dirsz) == 0 &&
		   (n[dp->dsk_dirsz] == '/' || n[dp->dsk_dirsz] == '\0' || strcmp(dp->dsk_logdir, "/") == 0))
			break;
	}

	blks = inblks(statb.st_size);
	if(blks > 8L)	/* correct for indirect blocks */
/*
 *		blks += (blks-10+127)/128 + (blks-10-128+16383)/16384 +
 *			(blks-10-128-16384+2097151)/2097152;
 */
		blks += (blks+117)/128 + (blks+16245)/16384 + (blks+2080629)/2097152;

	if(dp == NULL) {
		if(nchrg && (statb.st_size) &&
		   (statb.st_mode&S_IFMT) == S_IFDIR)
			(void)fprintf(nchrg, "%11lu \t%7lu \t%s\n",
			statb.st_uid, blks, n);
		return;
	}

	dp->dsk_du += (statb.st_mode&S_IFMT) == S_IFDIR ? blks
		:((statb.st_mode&S_IFMT)==S_IFREG ?
			(blks / statb.st_nlink) : 0L);
}

static long
inblks(bytes)
long bytes;
{
	return(bytes % 512L ? (bytes/512L) + 1L : bytes/512L );
}

#ifdef DEBUG
static
pdisk()
{
	register struct disk *dp;

	for(dp=Dp; dp!=NULL; dp=dp->dsk_np)
		printf("%-15s %11lu %7lu %5u %5u %s\n",
			dp->dsk_name,
			dp->dsk_uid,
			dp->dsk_du,
			dp->dsk_dirsz,
			dp->dsk_ns,
			dp->dsk_logdir);
}
#endif

static
clean(p)
register char *p;
{
	register char *s1, *s2;

	for(s1=p; *s1; ) {
		s2 = s1;
		while(*s1 == '/')
			s1++;
		s1 = s1<= s2 ? s2 : s1-1;
		if(s1 != s2) {
			(void)strcpy(s2,s1);
			s1 = s2;
		}
		if(*++s2 == '.') 
		switch(*++s2) {
		case '/':
			(void)strcpy(s1,s2);
			continue;
		case '.':
			if(*++s2 != '/')
				break;
			if(s1 > p)
				while(*--s1 != '/' && s1 > p)
					;
			(void)strcpy(s1,s2);
			continue;
		}
		while(*s2 && *++s2 != '/')
			;
		s1 = s2;
	}
	return;
}


extern long atol();

static FILE *pwf = NULL;
static char line[BUFSIZ+1];
static struct passwd passwd;

static
setup(pfile)
char	*pfile;
{
	register struct passwd	*pw;

	if( pfile == NULL ) {
		errno = 0;
		setpwent();
		if ( errno ) {
			openerr("/etc/passwd");
		}
	} else {
		if( !my_setpwent(pfile) ) {
			openerr(pfile);
		}
	}
	while ( (pw=act_getpwent()) != NULL )
		makdlst(pw);
	act_endpwent();
}

static int
my_setpwent(pfile)
register char *pfile;
{
	if(pwf == NULL)
		pwf = fopen(pfile, "r");
	else
		rewind(pwf);
	return(pwf != NULL);
}

static void
my_endpwent()
{
	if(pwf != NULL) {
		(void)fclose(pwf);
		pwf = NULL;
	}
}

static char *
pwskip(p)
register char *p;
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if(*p == '\n')
		*p = '\0';
	else if(*p)
		*p++ = '\0';
	return(p);
}

static struct passwd *
my_getpwent()
{
	register char *p;
	long	x;

	if(pwf == NULL) {
		return(0);
	}
	p = fgets(line, BUFSIZ, pwf);
	if(p == NULL)
		return(0);
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	x = atol(p);	
	passwd.pw_uid = (uid_t)x;
	p = pwskip(p);
	x = atol(p);
	passwd.pw_gid = (gid_t)x;
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	(void) pwskip(p);

	return(&passwd);
}
