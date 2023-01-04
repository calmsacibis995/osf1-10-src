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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: find.c,v $ $Revision: 4.3.13.8 $ (DEC) $Date: 1993/10/18 15:11:40 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * 1.47  com/cmd/scan/find.c, cmdscan, bos320, 9132320b 7/30/91 17:39:36
 */

#include	<stdio.h>
#include	<string.h>
#include	<limits.h>
#include	<pwd.h>
#include	<grp.h>
#include	<utmp.h>
#include	<time.h>
#ifdef _AIX
#include	<fshelp.h>
#include	<sys/fullstat.h>
#else
#include	<sys/stat.h>
#endif /* _AIX */
#include	<fcntl.h>
#include        <sys/errno.h>
#include        <sys/types.h>
#include        <sys/param.h>
#ifdef _AIX
#include        <sys/vfs.h>
#endif /* _AIX */
#include	<sys/mount.h>
#include        <dirent.h>
#include        <sys/utsname.h>
#include        <sys/param.h>
#include	<locale.h>
#include 	<sys/signal.h>
#include 	<langinfo.h>
#include	<stdlib.h>
#include	<fnmatch.h>
#include	<errno.h>
#include	<unistd.h>
#include	<sys/wait.h>

#include	<machine/endian.h>
#include 	"find_msg.h"

nl_catd catd;
char	*ystr;

#define MSGSTR(Num, Str) catgets(catd,MS_FIND, Num, Str)

#define RPT_BLK_SZ	512		/* P1003.2/Draft 11 now says 512 */
					/* used to be 1024, with comments:*/
					/* Used to be UBSIZE, but we
					 * need to report in 1024 not 512
					 */
#define A_DAY	86400L /* a day full of seconds */
#define EQ(x, y)	(strcmp(x, y)==0)
#define BUFSIZE	512	/* In u370 I can't use BUFSIZ nor BSIZE */
#define CPIOBSZ	4096
#define Bufsize	5120

int	Randlast;
char	Pathname[MAXPATHLEN+1];
#define DEV_BSHIFT      9               /* log2(DEV_BSIZE) */
/* DEV_BSIZE is in sys/dir.h, included from dirent.h */
#define dbtob(db)               /* calculates (db * DEV_BSIZE) */ \
        ((unsigned)(db) << DEV_BSHIFT)

#define MAXNODES	100
#define PREVDIR		".."       /* A13989 */

#define UUID     1
#define GGID     2

int	Randlast;

typedef	int (*FUNC)();
struct anode {
	FUNC F;
	struct anode *L, *R;
} Node[MAXNODES];
int Nn=0;  /* number of nodes */
char	*Fname;
char    fstyped;		/* -fstype option specified */
int	Fstype;			/* Holds fstype from st_ftype field */
time_t	Now;
int	Argc,
	Ai,
	Pi;
char	**Argv;
/* cpio stuff */
int	Cpio;
short	*SBuf, *Dbuf, *Wp;
char	*Buf, *Cbuf, *Cp;
char	Strhdr[500],
	*Chdr = Strhdr;
int	Wct = Bufsize / 2, Cct = Bufsize;
int	Cflag;
int	depthf = 0;
#ifdef _AIX
struct  xutsname xname;
#endif

int     pruned = 0;    		/* do not descend into directories */
int	Xdev = 1;		/* true if SHOULD cross devices (filesystems) */
int	printflag;
int	execflag;
int	okflag;

struct statfs Devstatfs;	/* File-system information */
struct stat   Devstat;		/* File information */


/* mount stuff */						/* 001 */
int	do_mount = 1;	/* go down mount points if true */	/* 001 */
int	mount_dev = -1;/* file must reside here or we've crossed a mount point*/
								/* 001 */

struct stat Statb;

static struct anode	*exp(),
		*e1(),
		*e2(),
		*e3(),
		*mk(int (*f)(), struct anode *, struct anode *);

#define MAKE(f,l,r)	mk(f, (struct anode *)(l), (struct anode *)(r))

static char	*nxtarg();
static char	*Home;
static long	Blocks;



#ifdef __alpha
#define PTR_INT	long
#else
#define PTR_INT int
#endif
struct glob_str		{ FUNC f; char *pat;};
struct mtime_str	{ FUNC f; PTR_INT t, s; };
struct atime_str	{ FUNC f; PTR_INT t, s; };
struct lctime_str	{ FUNC f; PTR_INT t, s; };
struct user_str		{ FUNC f; PTR_INT u, s; };
struct ino_str		{ FUNC f; PTR_INT u, s; };
struct group_str	{ FUNC f; PTR_INT u; };
struct links_str	{ FUNC f; PTR_INT link, s; };
struct size_str		{ FUNC f; PTR_INT sz, s; };
struct perm_str		{ FUNC f; PTR_INT per, s; };
struct type_str		{ FUNC f; PTR_INT per, s; };
struct fstype_str	{ FUNC f; PTR_INT fstype; };
struct prune_str	{ FUNC f; PTR_INT per, s; };
struct exeq_str		{ FUNC f; PTR_INT com; };
struct ok_str		{ FUNC f; PTR_INT com; };
struct newer_str	{ FUNC f; PTR_INT time;};

static int ckmount();					/* 001 */
static int scomp(int,int,int);
static int descend();
static int lctime();
static int chdir_access();
static int size();
static int exeq(), ok(), glob(),  mtime(), atime(), lctime(), user(),
  	   and(), or(), not(),
	   group(), perm(), links(), print(), prune(),
	   type(), ino(), depth(),
#ifdef _AIX
  	   nnode(),
#endif
  	   cpio(), newer(), fstype(),
	   ls(), crossdev(), nouser(), nogroup();

static int getunum( int, char *);
static int doex(int);
static int list( char *, struct stat *);

static void bwrite( short *, int );
static int chgreel( int, int );
static void writehdr( char *, int );

static char *whereami();
static char *getname();
static char *getgroup();
struct	vfs_ent *vfsp;

struct header {
	short	h_magic,
		h_dev;
	ushort	h_ino,
		h_mode,
		h_uid,
		h_gid;
	short	h_nlink,
		h_rdev,
		h_mtime[2],
		h_namesize,
		h_filesize[2];
	char	h_name[256];
} hdr;

struct  utmp utmp;
#define NMAX    (sizeof (utmp.ut_name))
#define SCPYN(a, b)     (void)strncpy(a, b, NMAX)

#define NUID_INC    100
int MAXUID = 0;

#define NGID        300

struct ncache {
        int     uid;
        char    name[NMAX+1];
} *ncptr;

#ifdef _AIX
char    names[NUID_INC][NMAX]+1;
#endif _AIX

char    outrangename[NMAX+1];
int     outrangeuid = -1;
char    groups[NGID][NMAX+1];
char    outrangegroup[NMAX+1];
int     outrangegid = -1;

extern mode_t permissions( const char * );

int status = 0;		/* Summary of all errors */

/*
 * locale's short month_date format
 *
 */
static char rec_form_posix[] = "%b %e %H:%M";
static char old_form_posix[] = "%b %e  %Y";
static char *loc_rec_form;
static char *loc_old_form;

void
main(int argc, char *argv[]) 
{

	struct anode *exlist;
	int paths;
	register char *cp, *sp = 0;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_FIND,NL_CAT_LOCALE);
	ystr = MSGSTR(YES,"yes");
	time(&Now);

	loc_rec_form = nl_langinfo(_M_D_RECENT);
	if (loc_rec_form == NULL || !*loc_rec_form)
		loc_rec_form = rec_form_posix;
	loc_old_form = nl_langinfo(_M_D_OLD);
	if (loc_old_form == NULL || !*loc_old_form)
		loc_old_form = old_form_posix;

#ifdef _AIX
	unamex(&xname);
#endif

	Home = getcwd( NULL, PATH_MAX+1);
	if (!Home) {
		fprintf(stderr, MSGSTR( BADPWD, "Cannot get current directory\n"));
		exit(2);
	}

	Argc = argc; Argv = argv;
	if(argc<2) {
usage:		fprintf(stderr, MSGSTR( USAGE, "Usage: find path-list predicate-list\n"));
		exit(1);
	}
	for(Ai = paths = 1; Ai < argc; ++Ai, ++paths)
		if(*Argv[Ai] == '-' || *Argv[Ai] == '(' || *Argv[Ai] == '!')
			break;
	if(paths == 1) /* no path-list */
		goto usage;

	if (Ai == argc) {
		/*
		 * 'find paths...' without any selectors
		 */
		exlist = MAKE(print, 0, 0);
		printflag++;

	} else if(!(exlist = exp())) { /* parse and compile the arguments */
		fprintf(stderr,MSGSTR( PARSERR, "find: parsing error\n"));
		exit(1);
	}
	if(!okflag && !execflag && !printflag)
		/*
		 * POSIX gives you free -print, when no -exec, -ok or -print present
		 */
	  	exlist = MAKE(and, exlist, MAKE(print,0,0));
	if(Ai<argc) {
		fprintf(stderr, MSGSTR( CONJ, "find: missing conjunction\n"));
		exit(1);
	}
	for(Pi = 1; Pi < paths; ++Pi) {
		sp = NULL;
		mount_dev = -1;					/* 001 */
		(void)strcpy(Pathname, Argv[Pi]);
		if(*Pathname != '/')
			(void)chdir_access(Home);
		if(cp = strrchr(Pathname, '/')) {
			sp = cp + 1;
			*cp = '\0';
			if(chdir_access(*Pathname? Pathname: "/") == -1) {
				fprintf(stderr,MSGSTR( BADSTART, "find: bad starting directory\n"));
				exit(2);
			}
			*cp = '/';
		}
		Fname = sp ? sp: Pathname;
		if ( '\0' == *Fname )
			Fname = ".";
		
		if (fstyped || !Xdev || pruned) {
			if( statfs(Fname, &Devstatfs)<0) {
				fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), Pathname);
				exit(2);
			}

			if (fstyped) {
			  	Fstype = Devstatfs.f_type;
			}
		}

		if(!Xdev) {
			if( stat(Pathname, &Devstat)<0) {
				fprintf(stderr,MSGSTR( BADSTAT, 
					"find: bad status-- %s\n"), Pathname);
				exit(2);
			}
		}

		/* to find files that match  */
		descend(Pathname, Fname, Fstype, 0, exlist);
	}

	if(Cpio) {
		strcpy(Pathname, "TRAILER!!!");
		Statb.st_size = 0;
		cpio();
		printf("%ld blocks\n", Blocks*10);
	}
	exit(status);
}

/* compile time functions:  priority is  exp()<e1()<e2()<e3()  */
struct anode *exp() /* parse ALTERNATION (-o)  */
{
	int or();
	register struct anode * p1;

	p1 = e1() /* get left operand */ ;
	if(EQ(nxtarg(), "-o")) {
		Randlast--;
		return(MAKE(or, p1, exp()));
	}
	else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e1()  /* parse CONCATENATION (formerly -a) */
{
	int and();
	register struct anode * p1;
	register char *a;

	p1 = e2();
	a = nxtarg();
	if(EQ(a, "-a")) {
And:
		Randlast--;
		return(MAKE(and, p1, e1()));
	} else if(EQ(a, "(") || EQ(a, "!") || (*a=='-' && !EQ(a, "-o"))) {
		--Ai;
		goto And;
	} else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e2()  /* parse NOT (!) */
{
	int not();

	if(Randlast) {
		fprintf(stderr,MSGSTR( OPFOP, "find: operand follows operand\n"));
		exit(1);
	}
	Randlast++;
	if(EQ(nxtarg(), "!"))
		return(MAKE(not, e3(), 0));
	else if(Ai <= Argc) --Ai;
	return(e3());
}
struct anode *e3()  /* parse parens and predicates */
{
	struct anode *p1;
	int i;
#ifdef _AIX
	unsigned long n;
	static unsigned long getnid();
#endif
	register char *a, *b;
	register int  s=0x0000;

	a = nxtarg();
	if(EQ(a, "(")) {
		Randlast--;
		p1 = exp();
		a = nxtarg();
		if(!EQ(a, ")")) goto err;
		return(p1);
	}
	/****************************************
	 * Look for predicates with no arguments
	 ****************************************/

	else if(EQ(a, "-print")) {
	  	printflag++;
		return(MAKE(print, 0, 0));
	}
	else if (EQ(a, "-nouser")) {
		return (MAKE(nouser,0,0));
	}
        else if (EQ(a, "-ls")) {
		printflag++;			/* Avoid double-printing */
                return (MAKE(ls, 0, 0));
        }
	else if(EQ(a, "-prune")) {
		return(MAKE(prune,0,0));
	}
	else if (EQ(a, "-nogroup")) {
		return (MAKE(nogroup, 0, 0));
	}
	else if(EQ(a, "-xdev")) {
		Xdev = 0;
		return(MAKE(crossdev, 0, 0));
	}
	else if(EQ(a, "-depth")) {
		depthf = 1;
		return(MAKE(depth,0,0));
	}
	else if(EQ(a, "-mount")) {				/* 001 */
		do_mount = 0;					/* 001 */
		return(MAKE(ckmount,0,0));			/* 001 */
								/* 001 */
	}							/* 001 */

	/******************************
	 * Predicates with an argument
	 ******************************/

	b = nxtarg();
	s = *b;
	if(s=='+') b++;

	if (EQ(a, "-name")) {				/* pjd 101 start  */
		if ( b[0] == '\0') {			/* henrypau 001 */
			fprintf(stderr, MSGSTR(NONAME, "find: The -name option requires filename argument.\n"));
			exit(1);
		}
		return(MAKE(glob, b, 0));
	}						/* pjd 101 finish */
	else if(EQ(a, "-mtime"))
		return(MAKE(mtime, atoi(b), s));
	else if(EQ(a, "-atime"))
		return(MAKE(atime, atoi(b), s));
	else if(EQ(a, "-ctime"))
		return(MAKE(lctime, atoi(b), s));
	else if(EQ(a, "-user")) {
		if((i=getunum(UUID, b)) == -1) {
			if ((*b) && (*b >= '0') && (*b <= '9')) 
				return MAKE(user, atoi(b), s);

			fprintf(stderr, MSGSTR( NOUSER, "find: cannot find -user name\n"));
			exit(1);
		}
		return(MAKE(user, i, s));
	}
	else if(EQ(a, "-inum") || EQ(a, "-i"))
		return(MAKE(ino, atoi(b), s));
	else if(EQ(a, "-group")) {
		if((i=getunum(GGID, b)) == -1) {
			if ((*b) && (*b >= '0') && (*b <= '9')) 
				return MAKE(group, atoi(b), s);
			fprintf(stderr,MSGSTR( NOGRP, "find: cannot find -group name\n"));
			exit(1);
		}
		return(MAKE(group,i,s));
	} else if(EQ(a, "-size")) {
		int factor=512;
		char c = *b ? b[strlen(b)-1] : '\0';

		switch (c) {
		case 'k':			/* OSF extension */
		case 'K': factor = 1024;
			  break;
		case 'c': factor = 1;
		}
			
		return(MAKE(size, atoi(b), (s|(factor<<8))));
	}
	else if(EQ(a, "-links"))
		return(MAKE(links, atoi(b), s));
	else if(EQ(a, "-perm")) {
	  	char *tail;
	  	if(s=='-') b++;
		i = strtoul(b,&tail,8);
		if( tail && *tail ) {		/* Not an octal number! */
		  i = permissions(b); 		/* Get the o+rw,g=u syntax */
		}

		return(MAKE(perm,i,s));
	}
	else if(EQ(a, "-type")) {
		switch (s) {
		case 'd' : i =  S_IFDIR; break;
		case 'b' : i =  S_IFBLK; break;
		case 'c' : i =  S_IFCHR; break;
		case 'p' : i =  S_IFIFO; break;
		case 'f' : i =  S_IFREG; break;
		case 'l' : i =  S_IFLNK; break;
		case 's' : i =  S_IFSOCK; break;
		default:
			fprintf(stderr, MSGSTR(BADTYPE,
				"find: Unknown -type name %c.\n"), (char)s);
			exit(1);
		}
		return(MAKE(type,i,0));
	}
	else if(EQ(a, "-fstype")) {
		fstyped = 1;
		return(MAKE(fstype, getvfsbyname(b), 0));
	}
	else if (EQ(a, "-exec")) {
		execflag++;
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(MAKE(exeq,i,0));
	}
	else if (EQ(a, "-ok")) {
		okflag++;
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(MAKE(ok,i,0));
	}
	else if(EQ(a, "-cpio")) {
		if((Cpio = creat(b, 0666)) < 0) {
			fprintf(stderr,MSGSTR( NOCREATE, "find: cannot create %s\n"), b);
			exit(1);
		}
 		SBuf = (short *)malloc(CPIOBSZ);
		if (!SBuf) {
		    fprintf(stderr, MSGSTR( NOMEM, "find: No memory\n"));
		    exit(1);
		}

		Wp = Dbuf = (short *)malloc(Bufsize);
		if (!Wp) {
		    fprintf(stderr, MSGSTR( NOMEM, "find: No memory\n"));
		    exit(1);
		}

		depthf = 1;
		return(MAKE(cpio,0,0));
	}
	else if(EQ(a, "-ncpio")) {
		if((Cpio = creat(b, 0666)) < 0) {
			fprintf(stderr,MSGSTR( NOCREATE, "find: cannot create %s\n"), b);
			exit(1);
		}

		Buf = (char*)malloc(CPIOBSZ);
		if (!Buf) {
		    fprintf(stderr, MSGSTR(NOMEM, "find: no memory\n"));
		    exit(1);
		}

		Cp = Cbuf = (char *)malloc(Bufsize);
		if (!Cp) {
		    fprintf(stderr, MSGSTR(NOMEM, "find: no memory\n"));
		    exit(1);
		}
		Cflag++;
		depthf = 1;
		return(MAKE(cpio,0,0));
	}
	else if(EQ(a, "-newer")) {
		if(stat(b, &Statb) < 0) {
			fprintf(stderr,MSGSTR(NOACCES,"find: cannot access %s\n"), b);
			exit(1);
		}
		return MAKE(newer, Statb.st_mtime, 0);
#ifdef _AIX
	} else if(EQ(a, "-node")) {
		if((n=getnid(b)) == -1) {
			fprintf(stderr,MSGSTR( BADNODE, "find: invalid node id %s\n"), b);
			exit(1);
		}
		return(MAKE(nnode, n, 0));
#endif /* _AIX */
	}
err:	fprintf(stderr,MSGSTR( BADOPTION, "find: bad option %s\n"), a);
	exit(1);
	return (NULL);
}
struct anode *mk(int (*f)(), struct anode *l, struct anode *r)
{
	if (Nn >= MAXNODES) {
		fprintf(stderr, MSGSTR(TOOMANYOPTS,"find: Too many options\n"));
		exit(1);
	}

	Node[Nn].F = f;
	Node[Nn].L = l;
	Node[Nn].R = r;
	return(&(Node[Nn++]));
}

static char *
nxtarg()  /* get next arg from command line */
{
	static strikes = 0;

	if(strikes==3) {
		fprintf(stderr,MSGSTR( INCSTATE, "find: incomplete statement\n"));
		exit(1);
	}
	if(Ai>=Argc) {
		strikes++;
		Ai = Argc + 1;
		return("");
	}
	return(Argv[Ai++]);
}

/* execution time functions */
static int
and(p)
register struct anode *p;
{
	return(((*p->L->F)(p->L)) && ((*p->R->F)(p->R))?1:0);
}
static int
or(p)
register struct anode *p;
{
	 return(((*p->L->F)(p->L)) || ((*p->R->F)(p->R))?1:0);
}
static int
not(p)
register struct anode *p;
{
	return( !((*p->L->F)(p->L)));
}
static int
glob(p)
register struct glob_str *p; 
{	/* fnmatch() returns a zero if Fname matches pattern */
	return(!fnmatch(p->pat, Fname, FNM_PERIOD));
}
static int
print()
{
	puts(Pathname);
	return(1);
}
static int
mtime(p)
register struct mtime_str *p;
{
        if ( (p->s)=='-' || (p->s)=='+' )
	return(scomp((int)((Now - Statb.st_mtime) / A_DAY), p->t, p->s));
        else return(scomp((int)((Now - Statb.st_mtime) / A_DAY), (p->t)-1, (p->s)-1));
}       /* change 'p->t' to '(p->t)-1', and 'p->s' to '(p->s)-1' for fixing qar#15468  -xz*/
static int
atime(p)
register struct atime_str *p;
{
        if ( (p->s)=='-' || (p->s)=='+' )
	return(scomp((int)((Now - Statb.st_atime) / A_DAY), p->t, p->s));
        else return(scomp((int)((Now - Statb.st_atime) / A_DAY), (p->t)-1, (p->s)-1));
}       /* change 'p->t' to '(p->t)-1', and 'p->s' to '(p->s)-1' for fixing qar#15468  -xz*/
static int
lctime(p)
register struct lctime_str *p; 
{
        if ( (p->s)=='-' || (p->s)=='+' )
	return(scomp((int)((Now - Statb.st_ctime) / A_DAY), p->t, p->s));
        else return(scomp((int)((Now - Statb.st_ctime) / A_DAY), (p->t)-1, (p->s)-1));
}       /* change 'p->t' to '(p->t)-1', and 'p->s' to '(p->s)-1' for fixing qar#15468  -xz*/
static int
user(p)
register struct user_str *p; 
{
#ifdef _AIX
	if (xname.nid != Statb.fst_nid) {
		Statb.st_uid = p->u;
		Statb.st_gid = 0;
#endif
		if( lstat(Fname, &Statb)<0) {
			fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), Fname);
			status = 2;
			return(0);
		}
#ifdef _AIX
	}
#endif

	return(scomp(Statb.st_uid, p->u, p->s));
}
static int
nouser(p)
struct anode *p;
{
	return (getname(Statb.st_uid) == (char *) NULL);
}
static int
ino(p)
register struct ino_str *p;
{
	return(scomp((int)Statb.st_ino, p->u, p->s));
}
static int
group(p)
register struct group_str *p; 
{
#ifdef _AIX
	if (xname.nid != Statb.fst_nid) {
		Statb.st_uid = 0;
		Statb.st_gid = p->u;
#endif /* _AIX */

		if( lstat(Fname, &Statb)<0) {
			fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), Fname);
			status = 2;
			return(0);
		}
#ifdef _AIX
	}
#endif

	return(p->u == Statb.st_gid);
}
static int
nogroup(p)
struct anode *p;
{

	return (getgroup(Statb.st_gid) == NULL);
}
static int
links(p)
register struct links_str *p; 
{
	return(scomp(Statb.st_nlink, p->link, p->s));
}

static int
size(p)
register struct size_str *p; 
{
  int temp=Statb.st_size;

  int scale = (p->s>>8);	/* Get resolution of comparison */
				/*  (bytes, blocks, or k-blocks */
  temp = (temp + (scale-1)) / scale;

  return(scomp(temp, p->sz, (char)p->s));
}
static int
perm(p)
register struct perm_str *p; 
{
	register i;
	i = (p->s=='-') ? p->per : 0740007777; /* '-' means only arg bits */
	return((Statb.st_mode & i & 0740007777) == p->per);
}
static int
type(p)
register struct type_str *p;
{
	return((Statb.st_mode&S_IFMT)==p->per);
}
static int
fstype(p)
register struct fstype_str *p;
{
	return(Fstype == p->fstype);
}
static int
prune(p)
register struct prune_str *p;
{
	pruned = 1;
	return(1);
}
static int
exeq(p)
register struct exeq_str *p;
{
	fflush(stdout); /* to flush possible `-print' */
	return(doex(p->com));
}
static int
ok(p)
struct ok_str  *p;
{
	int yes=0;
	char c[35];

	fflush(stdout); /* to flush possible `-print' */
	fprintf(stderr,MSGSTR(OKPROMT,"< %s ... %s > (%s)?   "), 
					Argv[p->com], Pathname, ystr);
	fflush(stderr);
	if (scanf("%20s", c) == EOF)
		exit(2);
	if (rpmatch(c) )
		yes = 1;
	return(yes? doex(p->com): 0);
}


#define MKSHORT(v, lv) {U.l=1L;if(U.c[0]) U.l=lv, v[0]=U.s[1], v[1]=U.s[0]; else U.l=lv, v[0]=U.s[0], v[1]=U.s[1];}

static union { long l; short s[2]; char c[4]; } U;

static long mklong(v)
short v[];
{
	U.l = 1;
	if(U.c[0] /* VAX */)
		U.s[0] = v[1], U.s[1] = v[0];
	else
		U.s[0] = v[0], U.s[1] = v[1];
	return U.l;
}

static int
crossdev()
{
	return(1);
}

static int
depth()
{
	return(1);
}

static int							/* 001 */
ckmount()							/* 001 */
{								/* 001 */
	if (mount_dev == -1) mount_dev = Statb.st_dev; /* first time through */
								/* 001 */
	if (mount_dev == Statb.st_dev) return(1);		/* 001 */
	return(0);						/* 001 */
}								/* 001 */

#ifdef _AIX
static int
nnode(p)
register struct { int f, nid; } *p;
{
	return(p->nid == Statb.fst_nid);
}
#endif /* _AIX */

static void bintochar(long int);

static int
cpio()
{
#define MAGIC 070707
#define HDRSIZE	(sizeof hdr - 256)
#define CHARS	76
	register ifile, ct;
	static long fsz;
	register i;

	strcpy(hdr.h_name, !strncmp(Pathname, "./", 2)? Pathname+2: Pathname);
	hdr.h_magic = MAGIC;
	hdr.h_namesize = strlen(hdr.h_name) + 1;
	hdr.h_uid = Statb.st_uid;
	hdr.h_gid = Statb.st_gid;
	hdr.h_dev = Statb.st_dev;
	hdr.h_ino = Statb.st_ino;
	hdr.h_mode = Statb.st_mode;
	hdr.h_nlink = Statb.st_nlink;
	hdr.h_rdev = Statb.st_rdev;
	MKSHORT(hdr.h_mtime, Statb.st_mtime);
	fsz = (hdr.h_mode & S_IFMT) == S_IFREG? Statb.st_size: 0L;
	MKSHORT(hdr.h_filesize, fsz);

	if (Cflag)
		bintochar(fsz);

	if(EQ(hdr.h_name, "TRAILER!!!")) {
		Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
			bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
		for (i = 0; i < 10; ++i)
			Cflag? writehdr(Buf, BUFSIZE): bwrite(SBuf, BUFSIZE);
		return (1);
	}
	if(!mklong(hdr.h_filesize)) {
		Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
			bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
		return(1);
	}
	if((ifile = open(Fname, 0)) < 0) {
		fprintf(stderr,MSGSTR( NOCOPY, "find: cannot copy %s\n"), hdr.h_name);
		status = 2;
		return(0);
	}
	Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
		bwrite((short *)&hdr, HDRSIZE+hdr.h_namesize);
	for(fsz = mklong(hdr.h_filesize); fsz > 0; fsz -= CPIOBSZ) {
		ct = fsz>CPIOBSZ? CPIOBSZ: fsz;
		if(read(ifile, Cflag? Buf: (char *)SBuf, ct) < 0)  {
			fprintf(stderr,MSGSTR( NOREAD, "Cannot read %s\n"), hdr.h_name);
			status = 2;
			continue;
		}
		Cflag? writehdr(Buf, ct): bwrite(SBuf, ct);
	}
	close(ifile);
	return 1;
}

static void
bintochar(t)
long t;
{
	sprintf(Chdr, "%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.11lo%.6ho%.11lo%s",
		MAGIC,Statb.st_dev,Statb.st_ino,Statb.st_mode,Statb.st_uid,
		Statb.st_gid,Statb.st_nlink,Statb.st_rdev & 00000177777,
		Statb.st_mtime,(short)strlen(hdr.h_name)+1,t,hdr.h_name);
}

int
ls( struct anode *p)
{
        list(Pathname, &Statb);
        return (1);
}

int newer(struct anode *fp)
{
    	struct newer_str *p = (void *)fp;	/* A little bit of trickery here */

	return Statb.st_mtime >  p->time;
}

#ifdef _AIX
static char *
getname(uid)
{
        register struct passwd *pw;
        static init;
        struct passwd *getpwent();

        if (uid >= 0 && uid < NGID && names[uid][0])
                return (&names[uid][0]);
        if (uid >= 0 && uid == outrangeuid)
                return (outrangename);
rescan:
        if (init == 2) {
                if (uid < NGID)
                        return (0);
                setpwent();
                while (pw = getpwent()) {
                        if (pw->pw_uid != uid)
                                continue;
                        outrangeuid = pw->pw_uid;
                        SCPYN(outrangename, pw->pw_name);
                        endpwent();
                        return (outrangename);
                }
                endpwent();
                return (0);
        }
        if (init == 0)
                setpwent(), init = 1;
        while (pw = getpwent()) {
                if ( pw->pw_uid >= NGID ) {
                        if (pw->pw_uid == uid) {
                                outrangeuid = pw->pw_uid;
                                SCPYN(outrangename, pw->pw_name);
                                return (outrangename);
                        }
                        continue;
                }
                if (names[pw->pw_uid][0])
                        continue;
                SCPYN(names[pw->pw_uid], pw->pw_name);
                if (pw->pw_uid == uid)
                        return (&names[uid][0]);
        }
        init = 2;
        goto rescan;
}


static char *
getgroup(gid)
{
        register struct group *gr;
        static init;
        struct group *getgrent();

        if (gid >= 0 && gid < NGID && groups[gid][0])
                return (&groups[gid][0]);
        if (gid >= 0 && gid == outrangegid)
                return (outrangegroup);
rescan:
        if (init == 2) {
                if (gid < NGID)
                        return (0);
                setgrent();
                while (gr = getgrent()) {
                        if (gr->gr_gid != gid)
                                continue;
                        outrangegid = gr->gr_gid;
                        SCPYN(outrangegroup, gr->gr_name);
                        endgrent();
                        return (outrangegroup);
                }
                endgrent();
                return (0);
        }
        if (init == 0)
                setgrent(), init = 1;
        while (gr = getgrent()) {
                if ( gr->gr_gid >= NGID ) {
                        if (gr->gr_gid == gid) {
                                outrangegid = gr->gr_gid;
                                SCPYN(outrangegroup, gr->gr_name);
                                return (outrangegroup);
                        }
                        continue;
                }
                if (groups[gr->gr_gid][0])
                        continue;
                SCPYN(groups[gr->gr_gid], gr->gr_name);
                if (gr->gr_gid == gid)
                        return (&groups[gid][0]);
        }
        init = 2;
        goto rescan;
}
#endif /* _AIX */

int
scomp(int a, int b, int s) /* funny signed compare */
{
	if(s == '+')
		return(a > b);
	if(s == '-')
		return(a < (b * -1));
	return(a == b);
}

static caught;

void
catcher(int sig)
{
	caught = sig;
}

static int
doex(int com)
{
	register np;
	register char *na;
	void (*old_quit)(int), (*old_intr)(int);
	static char *nargv[50];
	static ccode;
	static pid;

	ccode = np = 0;
	while (na=Argv[com++]) {
		if(strcmp(na, ";")==0) break;
		if(strcmp(na, "{}")==0) nargv[np++] = Pathname;
		else nargv[np++] = na;
	}
	nargv[np] = 0;
	if (np==0) return(9);
	/*
	 * catch deadly signals, since the programme we exec may ignore them.
	 */
	old_quit = signal(SIGQUIT, catcher);
	old_intr = signal(SIGINT,  catcher);
	caught = 0;

	if(pid = fork())
		while(wait(&ccode) != pid);
	else { /*child*/
		chdir_access(Home);
		execvp(nargv[0], nargv);
		/* PTM # 34450 */
 		fprintf(stderr, MSGSTR(CANTEXEC,"find: Cannot execute %s"), nargv[0]);
 		fflush(stdout);
 		perror(":");
		exit(1);
	}

	signal(SIGQUIT, old_quit);
	signal(SIGINT,  old_intr);

	if (caught)
		kill(0, caught);

	return(ccode ? 0:1);
}

static int
getunum(int t, char *s)
{
	register i;
	struct	passwd	*pw;
	struct	group	*gr;

	i = -1;
	if( t == UUID ){
		if( ((pw = getpwnam( s )) != (struct passwd *)NULL) && pw != (struct passwd *)EOF )
			i = pw->pw_uid;
	} else {
		if( ((gr = getgrnam( s )) != (struct group *)NULL) && gr != (struct group *)EOF )
			i = gr->gr_gid;
	}
	return(i);
}

/************************************************************************/
#ifdef _AIX
char    valid_chars[] = "0123456789abcdefABCDEF";

unsigned long
getnid(nodename)
char *nodename;
{
	unsigned long nid;
	register char *testp;   /* used in nodename validation          */
	char * strchr();
	int length;             /* length of nodename                   */
	int i;                  /* counter and error returns            */

	/* is this nodename actually a nickname?                        */

	/* NOT AVAILABLE 
	i = drsname(nodename,&nid);
	*/
	return (-1);

	/*NOTREACHED*/
	if (i == 0)
		return(nid);      /* nodename translated into node id   */

	/* validate characters in nodename against allowable characters
	   in hex representation                                        */
	testp = (char *) ~0;
	if ((length = strlen(nodename)) != 8)
		return(-1);
	for (i=0; (i<length) && (testp); i++)
		testp = strchr( valid_chars, nodename[i] );
	if (!testp)
		return(-1);
	sscanf(nodename, "%x", &nid);
	return(nid);
}
#endif /* _AIX */

/************************************************************************/
#define ROOT "/"
static int
descend(name, fname, pfstype, pfsno, exlist)
	struct anode *exlist;
	char *name,*fname;
	int pfstype;		/* fstype  of parent dir */
	dev_t pfsno;		/* device number of parent dir */
{
	DIR	*dir = NULL;
	register struct dirent	*dp;
	register char *c1;
	int rv = 0;
	char *endofname;
	dev_t cfsno;
	int cfstype;		/* Suspicious about *fstype uses here??? */
	char *wd; 		/* working directory */
	char backptr[PATH_MAX + 1];
	struct statfs Statb2;

	strcpy(backptr,PREVDIR);
	if( lstat(fname, &Statb)<0) {
		fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), name);
		status = 2;
		return(0);
	}
	cfsno = Statb.st_dev;
	if (fstyped) {
		if (cfsno != pfsno) {
			if( statfs(fname, &Statb2)<0) {
				fprintf(stderr,MSGSTR( BADSTAT, 
					"find: bad status-- %s\n"), Fname);
				status = 2;
				return(0);
			}
			Fstype = Statb2.f_type;

			if ( Devstatfs.f_type != Statb2.f_type)
			        return(1);
		} else
		  	Fstype = pfstype;

		cfstype = Fstype;

	}

	if (!depthf)
		(*exlist->F)(exlist);

	if (pruned) {
	    pruned = 0;
	    return (1);
	}

	if((Statb.st_mode&S_IFMT)!=S_IFDIR ||
	   !Xdev && Devstat.st_dev != Statb.st_dev){    
		if (depthf)
			(*exlist->F)(exlist);
		return(1);
		}              

	if((!do_mount) && (mount_dev != Statb.st_dev))		/* 001 */
			return(1);				/* 001 */

	for (c1 = name; *c1; ++c1);
	if (*(c1-1) == '/')
		--c1;
	endofname = c1;


	/* if filesystem is remote, save previous directory incase we can't
	** chdir("..")   A13989
	*/
#ifndef FS_REMOTE
#define FS_REMOTE MOUNT_NFS
#endif

	if( statfs(fname, &Statb2)<0) {
	    fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), name);
	    status = 2;
	    return(0);
	}
	    
	if (Statb2.f_type == FS_REMOTE || Statb.st_nlink > 2) {
		strcpy(backptr,whereami());
	}


	if (chdir_access(fname) == -1) {
		wd=whereami();
		if (strcmp(ROOT,wd) == 0)
			fprintf(stderr,MSGSTR(NOCDROOT,"find: cannot chdir to </%s>"),fname);
		else

			fprintf(stderr,MSGSTR(NOCD,"find: cannot chdir to <%s/%s>"), wd, fname);
		perror(" ");
		status = 2;
		return(0);
	}
	c1 = endofname;
	*c1++ = '/';
	Fname = c1;
	if ((dir = opendir(".")) == NULL) {
		fprintf(stderr, MSGSTR(NOOPEN,"find: cannot open < %s >\n"), name);
		status = 2;
		rv = 0;
		goto ret;
	}
	for (dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
		if ((dp->d_name[0]=='.' && dp->d_name[1]=='\0') ||
		    (dp->d_name[0]=='.' && dp->d_name[1]=='.' && dp->d_name[2]=='\0'))
			continue;
		if ( ((c1 - name) + strlen(dp->d_name)) > PATH_MAX ) {
		    fprintf(stderr, MSGSTR(BADFILE, "find: %s%s. Path name too long.\n"),
			    name, dp->d_name);
		    status = 2;
		    rv = 0;
		    goto ret;
		}
		strcpy(c1, dp->d_name);
		(void) descend(name, Fname, cfstype, cfsno, exlist);
	}
	rv = 1;
ret:
	if(dir)
		closedir(dir);
	*endofname = '\0';
	Fname = fname;
	if(chdir_access(backptr) == -1) {       /* A13989 */
		*endofname = '\0';
		fprintf(stderr, MSGSTR(BADDIR,"find: bad directory <%s>\n"), name);
		exit(1);
	}
	if(depthf){
		if( lstat(fname, &Statb)<0) {
			fprintf(stderr, MSGSTR(NOSTAT,"find: Cannot get information about <%s>\n"), fname);
			status = 2;
		}
		(*exlist->F)(exlist);
	}
	return(rv);
}

void
bwrite(short *rp, int c)
{
	register short *wp = Wp;

	c = (c+1) >> 1;
	while(c--) {
		if(!Wct) {
again:
			if(write(Cpio, (char *)Dbuf, Bufsize)<0) {
				Cpio = chgreel(1, Cpio);
				goto again;
			}
			Wct = Bufsize >> 1;
			wp = Dbuf;
			++Blocks;
		}
		*wp++ = *rp++;
		--Wct;
	}
	Wp = wp;
}

static void
writehdr(char *rp, int c)
{
	register char *cp = Cp;

	while (c--)  {
		if (!Cct)  {
again:
			if(write(Cpio, Cbuf, Bufsize) < 0)  {
				Cpio = chgreel(1, Cpio);
				goto again;
			}
			Cct = Bufsize;
			cp = Cbuf;
			++Blocks;
		}
		*cp++ = *rp++;
		--Cct;
	}
	Cp = cp;
}

static int
chgreel(int x, int fl)
{
	register f;
	char str[22];
	FILE *devtty;
	struct stat statb;

	fprintf(stderr,( x ? MSGSTR( WRTOUT, "find: can't write output\n") 
		      	   : MSGSTR( READIN, "find: can't read input\n")));

	fstat(fl, &statb);
	if((statb.st_mode&S_IFMT) != S_IFCHR)
		exit(1);
again:
	fprintf(stderr,MSGSTR( ASKDEV, "If you want to go on, type device/file name when ready\n"));
	devtty = fopen("/dev/tty", "r");
	 fgets(str, 20, devtty);
	str[strlen(str) - 1] = '\0';
	if(!*str)
		exit(1);
	close(fl);
	if((f = open(str, x? 1: 0)) < 0) {
		fprintf(stderr,MSGSTR( NOGOOD, "That didn't work"));
		fclose(devtty);
		goto again;
	}
	return f;
}


/*
 * This function assumes that the password file is hashed
 * (or some such) to allow fast access based on a name key.
 * If this isn't true, duplicate the code for getgroup().
 */
char *
getname(uid_t uid)
{
	register struct passwd *pw;
	struct passwd *getpwent();
	int xfactor,newmax;

        if(uid >= MAXUID)
        {
	      xfactor = uid / NUID_INC;
	      if( uid % NUID_INC > 0)
		      xfactor++;
	      if(xfactor == 0)
		  xfactor = 1;
	      newmax = NUID_INC * xfactor;
	      if(MAXUID == 0)
		    ncptr = (struct ncache *) malloc(sizeof(struct ncache) * newmax);
	      else
		    ncptr = (struct ncache *) realloc(ncptr, sizeof(struct ncache) * newmax);
	      MAXUID = newmax;
        }

	if(ncptr == NULL)
        {
	      fprintf(stderr,MSGSTR(NOMEM, "Out of memory"));
	      exit(1);
	}
	if (uid >= 0 && ncptr[uid].uid == uid && ncptr[uid].name[0])
		return (ncptr[uid].name);
	pw = getpwuid(uid);
	if (!pw)
		return (0);
	ncptr[uid].uid = uid;
	SCPYN(ncptr[uid].name, pw->pw_name);
	return (ncptr[uid].name);
}

char *
getgroup(gid_t gid)
{
	register struct group *gr;
	static init;

	if (gid >= 0 && gid < NGID && groups[gid][0])
		return (&groups[gid][0]);
	if (gid >= 0 && gid == outrangegid)
		return (outrangegroup);
rescan:
	if (init == 2) {
		if (gid < NGID)
			return (0);
		setgrent();
		while (gr = getgrent()) {
			if (gr->gr_gid != gid)
				continue;
			outrangegid = gr->gr_gid;
			SCPYN(outrangegroup, gr->gr_name);
			endgrent();
			return (outrangegroup);
		}
		endgrent();
		return (0);
	}
	if (init == 0)
		setgrent(), init = 1;
	while (gr = getgrent()) {
		if (gr->gr_gid >= NGID) {
			if (gr->gr_gid == gid) {
				outrangegid = gr->gr_gid;
				SCPYN(outrangegroup, gr->gr_name);
				return (outrangegroup);
			}
			continue;
		}
		if (groups[gr->gr_gid][0])
			continue;

		SCPYN(groups[gr->gr_gid], gr->gr_name);
		if (gr->gr_gid == gid)
			return (&groups[gid][0]);
	}
	init = 2;
	goto rescan;
}
 
 
#define permoffset(who)		((who) * 3)
#define permission(who, type)	((type) >> permoffset(who))
#define kbytes(bytes)		(((bytes) + 1023) / 1024)

static int
list(char *file, struct stat *stp)
{
	char pmode[32], uname[32], gname[32], fsize[32], ftime[32];
	char *ctime();
	static long special[] = { S_ISUID, 's', S_ISGID, 's', S_ISVTX, 't' };
	static time_t sixmonthsago = -1;
#ifdef	S_IFLNK
	char flink[MAXPATHLEN + 1];
#endif
	register int who;
	register char *cp;
	time_t now;

	if (file == NULL || stp == NULL)
		return (-1);

	time(&now);
	if (sixmonthsago == -1)
		sixmonthsago = now - 6L*30L*24L*60L*60L;

	switch (stp->st_mode & S_IFMT) {
#ifdef	S_IFDIR
	case S_IFDIR:	/* directory */
		pmode[0] = 'd';
		break;
#endif
#ifdef	S_IFCHR
	case S_IFCHR:	/* character special */
		pmode[0] = 'c';
		break;
#endif
#ifdef	S_IFBLK
	case S_IFBLK:	/* block special */
		pmode[0] = 'b';
		break;
#endif
#ifdef	S_IFLNK
	case S_IFLNK:	/* symbolic link */
		pmode[0] = 'l';
		break;
#endif
#ifdef	S_IFSOCK
	case S_IFSOCK:	/* socket */
		pmode[0] = 's';
		break;
#endif
#ifdef	S_IFREG
	case S_IFREG:	/* regular */
#endif
	default:
		pmode[0] = '-';
		break;
	}

	for (who = 0; who < 3; who++) {
		if (stp->st_mode & permission(who, S_IREAD))
			pmode[permoffset(who) + 1] = 'r';
		else
			pmode[permoffset(who) + 1] = '-';

		if (stp->st_mode & permission(who, S_IWRITE))
			pmode[permoffset(who) + 2] = 'w';
		else
			pmode[permoffset(who) + 2] = '-';

		if (stp->st_mode & special[who * 2])
			pmode[permoffset(who) + 3] = special[who * 2 + 1];
		else if (stp->st_mode & permission(who, S_IEXEC))
			pmode[permoffset(who) + 3] = 'x';
		else
			pmode[permoffset(who) + 3] = '-';
	}
	pmode[permoffset(who) + 1] = '\0';

	cp = getname(stp->st_uid);
	if (cp != NULL)
		sprintf(uname, "%-9.9s", cp);
	else
		sprintf(uname, "%-9d", stp->st_uid);

	cp = getgroup(stp->st_gid);
	if (cp != NULL)
		sprintf(gname, "%-9.9s", cp);
	else
		sprintf(gname, "%-9d", stp->st_gid);

	if (pmode[0] == 'b' || pmode[0] == 'c')
		sprintf(fsize, "%3d,%3d",
			major(stp->st_rdev), minor(stp->st_rdev));
	else {
		sprintf(fsize, "%8ld", stp->st_size);
#ifdef	S_IFLNK
		if (pmode[0] == 'l') {
			/*
			 * Need to get the tail of the file name, since we have
			 * already chdir()ed into the directory of the file
			 */
			cp = strrchr(file, '/');
			if (cp == NULL)
				cp = file;
			else
				cp++;
			who = readlink(cp, flink, sizeof flink - 1);
			if (who >= 0)
				flink[who] = '\0';
			else
				flink[0] = '\0';
		}
#endif
	}

	if (stp->st_mtime < sixmonthsago || stp->st_mtime > now)
	  	strftime(ftime, sizeof(ftime), loc_old_form, localtime(&stp->st_mtime));
        else
	  	strftime(ftime, sizeof(ftime), loc_rec_form, localtime(&stp->st_mtime));

	printf("%8lu %4ld %s %2d %s%s%s %s %s%s%s\n",
		stp->st_ino,				/* inode #	*/
#ifdef	S_IFSOCK
		(long) kbytes(dbtob(stp->st_blocks)),	/* kbytes       */
#else
		(long) kbytes(stp->st_size),		/* kbytes       */
#endif
		pmode,					/* protection	*/
		stp->st_nlink,				/* # of links	*/
		uname,					/* owner	*/
		gname,					/* group	*/
		fsize,					/* # of bytes	*/
		ftime,					/* modify time	*/
		file,					/* name		*/
#ifdef	S_IFLNK
		(pmode[0] == 'l') ? " -> " : "",
		(pmode[0] == 'l') ? flink  : ""		/* symlink	*/
#else
		"",
		""
#endif
	);

	return (0);
}
/*
 * Make sure the directory can be read before actually doing the cd
 */
static int
chdir_access(directory)
        char *directory;
{
        DIR *dir_desc;
        struct dirent *dir_pointer;
        int saved_errno;
        if ((dir_desc = opendir(directory)) == NULL) 
        	return (-1);
        dir_pointer = readdir(dir_desc);
       	saved_errno = errno;
        closedir(dir_desc);
        if (dir_pointer == NULL) {
                errno = saved_errno;
                return (-1);
        }
        return (chdir(directory));
}
static char *
whereami()
{
static char pathname[PATH_MAX + 1];

        if(getcwd(pathname,sizeof(pathname)) == 0) {
                perror("pwd");
                exit(1);
        }
        return(pathname);
}
