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
static char	*sccsid = "@(#)$RCSfile: diskusg.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/07 21:53:40 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: 
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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/filsys.h> 
#include <sys/ioctl.h>
#include <sys/file.h>
#include <pwd.h>

#include <ufs/inode.h>
#include <ufs/fs.h>

#include <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define BLOCK		512		/* Block size for reporting */

#define	MAXIGN		10		/* max # of fs's to ignore */
#define	UNUSED		(uid_t)-1	/* flag for uid field in hash table */
#define	FAIL		-1		/* failure return code */
#define	MAXNAME		8		/* length of a user name */
#define	SUCCEED		0		/* success return code */
/*
#define	TRUE		1	
#define	FALSE		0
*/

struct	fs	sblock;
struct dinode itab[MAXBSIZE/sizeof(struct dinode)];

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
static int	adduser(), ignore(), ilist();
static int	count(), bread(), output();
static int	hashinit(), setup(), todo();
static long	blocks();
static char 	*skip(), *pwskip();
static unsigned	hash();

int	VERBOSE = 0;
int 	MAXUSERS = 1000; 	/* default # of users */
FILE	*ufd = 0;
static int	index;

/* 
 * struct Acct holds the diskusg information for a specific user.
 * the array userlist is malloc'ed with MAXUSERS elements in hashinit().
 */
struct Acct  { 
	uid_t	uid;
	long	usage;
	char	name [MAXNAME+1];
} *userlist;

char	*ignlist[MAXIGN];
int	igncnt = 0;

char	*cmd;

main(int argc, char **argv)
{
	extern	int	optind;
	extern	char	*optarg;
	register c;
	register FILE	*fd;
	register	rfd;
	struct	stat	sb;
	int	sflg = FALSE;
	char	*pfile = NULL;
	int	errfl = FALSE;

	cmd = argv[0];

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ACCT,NL_CAT_LOCALE);

	while((c = getopt(argc, argv, "vU:u:p:si:")) != EOF) switch(c) {
	case 's':
		sflg = TRUE;
		break;
	case 'v':
		VERBOSE = 1;
		break;
	case 'i':
		ignore(optarg);
		break;
	case 'u':
		ufd = fopen(optarg, "w");
		break;
	case 'p':
		pfile = optarg;
		/* use the alternate access routines */
		act_getpwent = my_getpwent;
		act_endpwent = my_endpwent;
		break;
	case 'U':			/* set max users */
		MAXUSERS = atoi(optarg);
		if (MAXUSERS <= 0) {
		 	(void)fprintf(stderr, MSGSTR(BADUVALUE,
					"%s: Value for -U must be > 0\n"), cmd);
		    errfl++;
		}
		break;
	case '?':
		errfl++;
		break;
	}

	if(errfl) {
		(void)fprintf(stderr, MSGSTR( DUSAGE,"Usage: %s [-U number] [-sv] [-p pw_file] [-u ufile] [-i ignlist] [file ...]\n"), cmd);
		exit(10);
	}

	hashinit();
	if(sflg == TRUE) {
		if(optind == argc){
			(void)adduser(stdin);
		} else {
			for( ; optind < argc; optind++) {
				if( (fd = fopen(argv[optind], "r")) == NULL) {
					(void)fprintf(stderr, MSGSTR( CANTOPEN, 
							"%s: Cannot open %s\n"),
							 cmd, argv[optind]);
					continue;
				}
				if(stat(argv[optind], &sb) >= 0){
					if ( (sb.st_mode & S_IFMT) != S_IFREG ){
						(void)fprintf(stderr, MSGSTR( NOTREG, 
						"%s: %s is not a regular file -- ignored\n"), 
						cmd, argv[optind]);
						continue;
					}
				} else {
					(void)fprintf(stderr, MSGSTR( CANTSTAT, 
						        "%s: Cannot stat %s\n"),
						 cmd, argv[optind]);
					continue;
				}
				(void)adduser(fd);
				(void)fclose(fd);
			}
		}
	}
	else {
		(void)setup(pfile);
		for( ; optind < argc; optind++) {
			/* check for filesystem names to ignore */
			if (!todo(argv[optind]))
				continue;
			if( (rfd = open(argv[optind], O_RDONLY)) < 0) {
				(void)fprintf(stderr, MSGSTR( CANTOPEN, 
							"%s: Cannot open %s\n"),
						cmd, argv[optind]);
				continue;
			}
			if(fstat(rfd, &sb) >= 0){
				if ( (sb.st_mode & S_IFMT) == S_IFCHR ||
				     (sb.st_mode & S_IFMT) == S_IFBLK ) {
#ifdef	IOCINFO
				{
					static struct devinfo devinfo;

					if((ioctl(rfd,IOCINFO,&devinfo) < 0) ||
					   ((devinfo.devtype == DD_DISK) && 
					   (!(devinfo.flags & DF_FIXED)))) {
					     (void)fprintf(stderr, MSGSTR( FSINVAL, 
					    "%s: Invalid filesystem type %s\n"),
							cmd, argv[optind]);
						(void)close(rfd);
						continue;
					}
				}
#endif	IOCINFO
					(void)ilist(argv[optind],rfd);
				} else {
				  (void)fprintf(stderr, MSGSTR( NOTSPECL, 
				   "%s: %s is not a special file -- ignored\n"),
						 cmd, argv[optind]);
				}
			} else {
				(void)fprintf(stderr, MSGSTR( CANTSTAT, 
						        "%s: Cannot stat %s\n"),
						 cmd, argv[optind]);
			}
			(void)close(rfd);
		}
	}
	output();
	return(0);
}

static
adduser(fd)
register FILE	*fd;
{
	uid_t	usrid;
	long	blcks;
	char	login[MAXNAME+10];

	while(fscanf(fd, "%ld %s %ld\n", &usrid, login, &blcks) == 3) {
		if( (index = hash(usrid)) == FAIL) return(FAIL);
		if(userlist[index].uid == UNUSED) {
			userlist[index].uid = usrid;
			(void)strncpy(userlist[index].name, login, MAXNAME);
		}
		userlist[index].usage += blcks;
	}
	return(0);
}

static
ignore(str)
register char	*str;
{
	for( ; *str && igncnt < MAXIGN; str = skip(str), igncnt++)
		ignlist[igncnt] = str;
	if(igncnt == MAXIGN) {
		(void)fprintf(stderr, MSGSTR( IGNOVR, 
		    "%s: ignore list overflow. Recompile with larger MAXIGN\n"),
				 cmd);
	}
}

char baduid[] = " BAD UID: file system = %s, inode = %u, uid = %u\n";

static
ilist(file,fd)
char *file;
register fd;
{
	register dev_t	dev;
	register i, j, nfiles, ino;
	long dev_bsize;
	daddr_t iblk;

	if (fd < 0 ) {
		return (FAIL);
	}

	sync();

	/* Read in super-block of filesystem */
	bread(fd, (off_t)SBOFF, (char *)&sblock, sizeof(struct fs));
	dev_bsize = sblock.fs_fsize / fsbtodb(&sblock, 1);

	nfiles = sblock.fs_ipg * sblock.fs_ncg;
	for (ino = 0; ino < nfiles; ) {
		iblk = fsbtodb(&sblock, itod(&sblock, ino));
		bread(fd, ((off_t)iblk * dev_bsize), (char *)itab,
		      (int)sblock.fs_bsize);
		for (j = 0; j < INOPB(&sblock) && ino < nfiles; j++, ino++) {
			if (ino < ROOTINO)
				continue;
			if (itab[j].di_mode & S_IFMT)
			   if(count(j, dev_bsize) == FAIL) {
				if(VERBOSE)
					(void)fprintf(stderr,MSGSTR(BADUID, baduid),
						file, ino, itab[j].di_uid);
				if(ufd)
					(void)fprintf(ufd, "%s %u %u\n", 
						file, ino, itab[j].di_uid);
			   }
		}
	}
	return (0);
}


static
count(j, bsize)
register j;
register long bsize;
{
	if ( itab[j].di_nlink == 0 || itab[j].di_mode == 0 )
		return(SUCCEED);
	if( (index = hash(itab[j].di_uid)) == FAIL || userlist[index].uid == UNUSED )
		return (FAIL);
	userlist[index].usage += (blocks(j) * bsize) / BLOCK; 
	return (SUCCEED);
}

static long
blocks(j)
register int j;
{
	return( itab[j].di_blocks );
}

char ioerror[] = "%s: IO error. off=%d, cnt=%d, seek=%d, read=%d\n";

static
bread(fd, off, buf, cnt)
register fd, cnt;
register off_t off;
register char *buf;
{
	off_t l;
	int r;

	l = lseek(fd, off, L_SET);
    	if (l<0 || ((r=read(fd, buf, cnt)) != cnt)) {
		(void)fprintf(stderr, MSGSTR(IOERROR, ioerror), 
				"diskusg", off, cnt, l, r);
		exit(1);
	}
}

static
output()
{
	for (index=0; index < MAXUSERS ; index++)
		if (userlist[index].uid != UNUSED && userlist[index].usage != 0)
	    		(void)printf("%11lu \t%-8.8s \t%7ld\n",
			    userlist[index].uid,
			    userlist[index].name,
			    userlist[index].usage);
}

static unsigned
hash(j)
register uid_t j;
{
	register uid_t start;
	register uid_t circle;
	circle = start = j % MAXUSERS;
	do
	{
		if ( userlist[circle].uid == j || userlist[circle].uid == UNUSED )
			return (circle);
		circle = (circle + 1) % MAXUSERS;
	} while ( circle != start);
	return (FAIL);
}

static
hashinit() 
{
    	userlist = (struct Acct *) malloc(sizeof(struct Acct) * MAXUSERS);
	for(index=0; index < MAXUSERS ; index++)
	{
		userlist[index].uid = UNUSED;
		userlist[index].usage = 0;
		userlist[index].name[0] = '\0';
	}
}

static
setup(pfile)
char	*pfile;
{
	register struct passwd	*pw;

	if( pfile == NULL ) {
		errno = 0;
		setpwent();
		if ( errno ) {
			(void)fprintf(stderr,
				MSGSTR( CANTOPEN,"%s: Cannot open %s\n"),
				cmd, "/etc/passwd");
			exit(5);
		}
	} else {
		if( !my_setpwent(pfile) ) {
			(void)fprintf(stderr,
				MSGSTR( CANTOPEN,"%s: Cannot open %s\n"),
				cmd, pfile);
			exit(5);
		}
	}
	while ( (pw=act_getpwent()) != NULL )
	{
		if ( (index=hash(pw->pw_uid)) == FAIL )
		{
			(void)fprintf(stderr, MSGSTR(USERLIM,
			        "%s: User limit reached, use -U # flag > %d\n"),
				cmd, MAXUSERS);
			act_endpwent();
			return (FAIL);
		}
		if ( userlist[index].uid == UNUSED )
		{
			userlist[index].uid = pw->pw_uid;
			(void)strncpy( userlist[index].name, pw->pw_name, MAXNAME);
		}
	}
	act_endpwent();
	return(0);
}

static
todo(fname)
register char	*fname;
{
	register	i;

	for(i = 0; i < igncnt; i++) {
		if(strcmp(fname, ignlist[i]) == 0) return(FALSE);
	}
	return(TRUE);
}

static char *
skip(str)
register char	*str;
{
	while(*str) {
		if(*str == ' ' ||
		    *str == ',') {
			*str = '\0';
			str++;
			break;
		}
		str++;
	}
	return(str);
}

extern long atol();
extern char *fgets();

static FILE *pwf = NULL;
static char line[BUFSIZ+1];
static struct passwd passwd;

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
