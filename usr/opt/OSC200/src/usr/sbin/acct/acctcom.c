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
static char	*sccsid = "@(#)$RCSfile: acctcom.c,v $ $Revision: 4.3.2.2 $ (DEC) $Date: 1993/10/07 21:53:15 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */ 
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: aread, convtime, doexit, fatal, isdevnull, printhd, 
 *            println, usage
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
#include	<time.h>
#include	<string.h>
#include	<sys/types.h>
#include	"acctdef.h"
#include	<grp.h>
#include	<stdio.h>
#include	<sys/acct.h>
#include	<pwd.h>
#include	<sys/stat.h>

#include <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define	ACCTF	0300		/* record type: 00 = acct */
#define MYKIND(flag)	((flag & ACCTF) == 0)
#define SU(flag)	((flag & ASU) == ASU)
#define PACCT		"/var/adm/pacct"
#define MEANSIZE	01
#define KCOREMIN	02
#define HOGFACTOR	04
#define	SEPTIME		010
#define	CPUFACTOR	020
#define IORW		040
#define	pf(dble)	(void)fprintf(stdout, MSGSTR(PF_FORMAT," %8.2lf"), dble)
#define	ps(rs)		(void)fprintf(stdout, MSGSTR(PS_FORMAT," %8.8s"), rs)
#define	diag(string)	(void)fprintf(stderr, "\t%s\n", string)

struct	acct ab;
char	command_name[16];
char	timestring[16];
char	obuf[BUFSIZ];

double	cpucut,
	syscut,
	hogcut,
	iocut,
	sys,
	user,
	cpu,
	elapsed,
	realtot,
	cputot,
	usertot,
	systot,
	kcoretot,
	iotot,
	rwtot;
long	daydiff,
	offset = -2,
	io,
	rw,
	mem,
	nacct,		/* number of acct structures in PACCT for backward */
	cmdcount;
time_t	tstrt_b,
	tstrt_a,
	tend_b,
	tend_a,
	etime;
int	backward,
	flag_field,
	average,
	quiet,
	option,
	verbose = 1,
	uidflag,
	gidflag,
	unkid,	/*user doesn't have login on this machine*/
	errflg,
	su_user,
	fileout = 0,
	stdinflg,
	nfiles;
dev_t	linedev	= -1;
uid_t	uidval,
	gidval;
char	*cname = NULL; /* command name pattern to match*/

struct passwd *getpwnam(), *pw;
struct group *getgrnam(),*grp;
static int	dofile(), aread(), doexit(), fatal();
static int	printhd(), println();
static int	isdevnull(), usage();
static time_t	convtime();

long	ftell();
int	time();
long	tmsecs();
struct tm *localtime();
char timbuf[BUFSIZ],
	*ofile,
	*devtolin(),
	*uidtonam();
dev_t	lintodev();
FILE	*ostrm;


#define INIT		register char *rexp = instring;
#define GETC()		(* (unsigned char *)rexp++)
#define PEEKC() 	(*(unsigned char *)rexp)
#define UNGETC(c)	((unsigned char *)--rexp)
#define RETURN(c)	return((char *)1);
#define ERROR(c)	{ regerr(c); return((char *)0); }
char expbuf[BUFSIZ];

#include <regexp.h>

main(int argc, char **argv)
{
	register int	c;
	extern int	optind;
	extern char	*optarg;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ACCT,NL_CAT_LOCALE);

	setbuf(stdout,obuf);
	while((c = getopt(argc, argv,
		"C:E:H:I:O:S:abe:fg:hikl:mn:o:qrs:tu:v")) != EOF) {
		switch(c) {
		case 'C':
			(void)sscanf(optarg,"%lf",&cpucut);
			continue;
		case 'O':
			(void)sscanf(optarg,"%lf",&syscut);
			continue;
		case 'H':
			(void)sscanf(optarg,"%lf",&hogcut);
			continue;
		case 'I':
			(void)sscanf(optarg,"%lf",&iocut);
			continue;
		case 'a':
			average++;
			continue;
		case 'b':
			backward++;
			continue;
		case 'g':
			if(sscanf(optarg,"%hu",&gidval) == 1)
				gidflag++;
			else if((grp=getgrnam(optarg)) == NULL)
				fatal( MSGSTR( GUNKNOWN, "Unknown group"), optarg);
			else {
				gidval=grp->gr_gid;
				gidflag++;
			}
			continue;
		case 'h':
			option |= HOGFACTOR;
			continue;
		case 'i':
			option |= IORW;
			continue;
		case 'k':
			option |= KCOREMIN;
			continue;
		case 'm':
			option |= MEANSIZE;
			continue;
		case 'n':
			cname=compile(optarg, expbuf, &expbuf[BUFSIZ], (int)'\0');
			continue;
		case 't':
			option |= SEPTIME;
			continue;
		case 'r':
			option |= CPUFACTOR;
			continue;
		case 'v':
			verbose=0;
			continue;
		case 'l':
			linedev = lintodev(optarg);
			continue;
		case 'u':
			if(*optarg == '?')
				unkid++;
			else if(*optarg == '#')
				su_user++;
			else if(sscanf(optarg, "%lu", &uidval) == 1)
				uidflag++;
			else if((pw = getpwnam(optarg)) == NULL)
				(void)fprintf(stderr, 
				    MSGSTR( UUNKNOWN, "%s: Unknown user %s\n"),
				    argv[0], optarg);
			else {
				uidval = pw->pw_uid;
				uidflag++;
			}
			continue;
		case 'q':
			quiet++;
			verbose=0;
			average++;
			continue;
		case 's':
			tend_a = convtime(optarg);
			continue;
		case 'S':
			tstrt_a = convtime(optarg);
			continue;
		case 'f':
			flag_field++;
			continue;
		case 'e':
			tstrt_b = convtime(optarg);
			continue;
		case 'E':
			tend_b = convtime(optarg);
			continue;
		case 'o':
			ofile = optarg;
			fileout++;
			if((ostrm = fopen(ofile, "w")) == NULL) {
				perror( MSGSTR( OPENERR, "open error on output file"));
				errflg++;
			}
			continue;
		case '?':
			errflg++;
			continue;
		}
	}

	if(errflg) {
		usage();
		exit(1);
	}

	argv = &argv[optind];
	while(optind++ < argc) {
		dofile(*argv);
		nfiles++;
	}

	if(nfiles==0) {
		if(isatty(0) || isdevnull())
			dofile(PACCT);
		else {
			stdinflg = 1;
			backward = offset = 0;
			dofile(NULL);
		}
	}
	doexit(0);
	return(0);
}


static
dofile(fname)
char *fname;
{
	register struct acct *a = &ab;
	long curtime;
	time_t	ts_a = 0,
		ts_b = 0,
		te_a = 0,
		te_b = 0;
	long	daystart;
	long	nsize;
	struct tm *tm ;

	if(fname != NULL)
		if(freopen(fname, "r", stdin) == NULL) {
			(void)fprintf(stderr, 
				MSGSTR( CANTOPEN, "%s: Cannot open %s\n"), 
				"acctcom", fname);
			return;
		}

	if(backward) {
		backward = 0;
		if(aread() == 0)
			return;
		backward = 1;
		nsize = sizeof(struct acct);	/* make sure offset is signed */
		(void)fseek(stdin, (long)(-nsize), 2);
		nacct = ftell(stdin) / nsize + 1;
	} else {
		if(aread() == 0)
			return;
		rewind(stdin);
	}
	tzset();
	daydiff = a->ac_btime - (a->ac_btime % SECSINDAY);
	/* localtime corrects for timezone and day light savings */
	tm = localtime(&a->ac_btime) ;
	daystart = a->ac_btime -
			((3600 * tm->tm_hour) + (60 * tm->tm_min) + tm->tm_sec);
	(void)time(&curtime);
	if(daydiff < (curtime - (curtime % SECSINDAY))) {
		/*
		 * it is older than today
		 */
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", tm);
		(void)fprintf(stdout, 
			MSGSTR( ARFHD, "\nACCOUNTING RECORDS FROM:  %s"), 
			timbuf);
	}

	if(tstrt_a) {
		ts_a = tstrt_a + daystart;
		tm = localtime(&ts_a) ;
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", tm);
		(void)fprintf(stdout, 
			MSGSTR( STAFT, "START AFTER : %s"), timbuf);
	}
	if(tstrt_b) {
		ts_b = tstrt_b + daystart;
		tm = localtime(&ts_b) ;
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", tm);
		(void)fprintf(stdout, 
			MSGSTR( STBEF, "START BEFORE: %s"), timbuf);
	}
	if(tend_a) {
		te_a = tend_a + daystart;
		tm = localtime(&te_a) ;
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", tm);
		(void)fprintf(stdout, 
			MSGSTR(ENDAFT, "END AFTER   : %s"), timbuf);
	}
	if(tend_b) {
		te_b = tend_b + daystart;
		tm = localtime(&te_b) ;
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", tm);
		(void)fprintf(stdout, 
			MSGSTR(ENDBEF, "END BEFORE  : %s"), timbuf);
	}
	if(ts_a) {
		if (te_b && ts_a > te_b) te_b += SECSINDAY;
	}

	while(aread() != 0) {
		elapsed = expacct(a->ac_etime);
		etime = a->ac_btime + (time_t)elapsed;
		if(ts_a || ts_b || te_a || te_b) {

			if(te_a && (etime < te_a)) {
				if(backward) return;
				else continue;
			}
			if(te_b && (etime > te_b)) {
				if(backward) continue;
				else return;
			}
			if(ts_a && (a->ac_btime < ts_a))
				continue;
			if(ts_b && (a->ac_btime > ts_b))
				continue;
		}
		if(!MYKIND(a->ac_flag))
			continue;
		if(su_user && !SU(a->ac_flag))
			continue;
		sys = expacct(a->ac_stime);
		user = expacct(a->ac_utime);
		cpu = sys + user;
		if(cpu <= PRECISION)
			cpu = PRECISION;
		/* KCORE(mem) conversion is done at print out time */
		mem = a->ac_mem;
		substr(a->ac_comm,command_name,0,8);
		io=(long)expacct(a->ac_io);
		rw=(long)expacct(a->ac_rw);
		if(cpucut && cpucut >= cpu)
			continue;
		if(syscut && syscut >= sys)
			continue;
		if(linedev != -1 && a->ac_tty != linedev)
			continue;
		if(uidflag && a->ac_uid != uidval)
			continue;
		if(gidflag && a->ac_gid != gidval)
			continue;
		if(cname && !step(a->ac_comm, expbuf))
			continue;
		if(iocut && iocut > io)
			continue;
		if(unkid && uidtonam(a->ac_uid)[0] != '?')
			continue;
		if(verbose && (fileout == 0)) {
			printhd();
			verbose = 0;
		}
		if(elapsed <= PRECISION)
			elapsed = PRECISION;
		if(hogcut && hogcut >= cpu/elapsed)
			continue;
		if(fileout)
			(void)fwrite(&ab, sizeof(ab), 1, ostrm);
		else
			println();
		if(average) {
			cmdcount++;
			realtot += elapsed;
			usertot += user;
			systot += sys;
			kcoretot += (double)mem;
			iotot += io;
			rwtot += rw;
		};
	}
}


static
aread()
{
	register flag;
	static	 ok = 1;

/* fseek doesn't have to return an error in case of "negative" offset,
 * because they can be treated as large positive numbers. Documentation
 * about the behaviour of fseek, fread, fwrite in acse of "negative"
 * offsets is rather vague. So I inserted the nacct to count backwards.
 */
	if ( backward && (nacct <= 0) )
		return (0);

	if(fread((char *)&ab, sizeof(struct acct), 1, stdin) != 1)
		flag = 0;
	else
		flag = 1;

	if(backward) {
		if(ok) {
			if(fseek(stdin,
				(long)(offset*sizeof(struct acct)), 1) != 0) {
					rewind(stdin);	/* get 1st record */
					ok = 0;
			} else
				nacct--;
		} else
			flag = 0;
	}
	return(flag);
}


static
printhd()
{
	(void)fprintf(stdout, MSGSTR( CMSHED1, 
		    "COMMAND                      START    END          REAL"));
	ps(MSGSTR( CPUSTR, "CPU")); 
	if(option & SEPTIME)
		ps(MSGSTR( SECSSTR, "(SECS)")); 
	if(option & IORW){
		ps(MSGSTR( CHARSTR, "CHARS")); 
		ps(MSGSTR( BLOCKSTR,"BLOCKS")); 
	}
	if(option & CPUFACTOR)
		ps(MSGSTR( CPUSTR, "CPU")); 
	if(option & HOGFACTOR)
		ps(MSGSTR( HOGSTR, "HOG")); 
	if(!option || (option & MEANSIZE))
		ps(MSGSTR( MEANSTR, "MEAN")); 
	if(option & KCOREMIN)
		ps(MSGSTR( KDCORESTR, "KCORE")); 
	(void)printf("\n");
	(void)fprintf(stdout, MSGSTR( CMSHED2, 
		    "NAME       USER     TTYNAME  TIME     TIME       (SECS)"));
	if(option & SEPTIME) {
		ps(MSGSTR( SYSSTR, "SYS")); 
		ps(MSGSTR( USERSTR, "USER")); 
	} else
		ps(MSGSTR( SECSSTR, "(SECS)")); 
	if(option & IORW) {
		ps(MSGSTR( TRNSFDSTR, "TRNSFD")); 
		ps(MSGSTR( READSTR, "READ")); 
	}
	if(option & CPUFACTOR)
		ps(MSGSTR( FACTORSTR, "FACTOR")); 
	if(option & HOGFACTOR)
		ps(MSGSTR( FACTORSTR, "FACTOR")); 
	if(!option || (option & MEANSIZE))
		ps(MSGSTR( SIZEKSTR, "SIZE(K)")); 
	if(option & KCOREMIN)
		ps(MSGSTR( MINSTR, "MIN")); 
	if(flag_field)
		(void)fprintf(stdout, MSGSTR( FSTATSTR, "  F STAT"));
	(void)printf("\n");
	(void)fflush(stdout);
}


static
println()
{

	char name[32];
	register struct acct *a = &ab;

	if(quiet)
		return;
	if(!SU(a->ac_flag))
		(void)strcpy(name,command_name);
	else {
		(void)strcpy(name,"#");
		(void)strcat(name,command_name);
	}
	(void)fprintf(stdout, "%-9.9s", name);
	(void)strcpy(name,uidtonam(a->ac_uid));
	if(*name != '?')
		(void)fprintf(stdout, "  %-8.8s", name);
	else
		(void)fprintf(stdout, "  %-11lu",a->ac_uid);
	(void)fprintf(stdout, " %-8.8s",a->ac_tty != -1? devtolin(a->ac_tty):"?");
	(void)strftime(timbuf,15," %T %y %n",localtime(&a->ac_btime));
	(void)fprintf(stdout, "%.9s", timbuf);
	(void)strftime(timbuf,15," %T %y %n",localtime(&etime));
	(void)fprintf(stdout, "%.9s", timbuf);
	pf(elapsed);
	if(option & SEPTIME) {
		pf(sys);
		pf(user);
	} else
		pf(cpu);
	if(option & IORW)
		(void)fprintf(stdout, " %8ld %8ld",io,rw);
	if(option & CPUFACTOR)
		pf(user / cpu);
	if(option & HOGFACTOR)
		pf(cpu / elapsed);
	if(!option || (option & MEANSIZE))
		pf(KCORE(mem));
	if(option & KCOREMIN)
		pf(MINS(KCORE(mem)*cpu));
	if(flag_field)
		(void)fprintf(stdout, "  %1o %3o", a->ac_flag, a->ac_stat);
	(void)printf("\n");
}

/*
 * convtime converts time arg to internal value
 * arg has form hr:min:sec, min or sec are assumed to be 0 if omitted
 */
static time_t
convtime(str)
char *str;
{
	time_t	hr, min, sec;
	int i;
	char *nlt, *getenv();
	time_t tin[3];

	min = sec = 0;

	if((nlt = getenv("NLTIME")) != NULL)
	{
		if(sscanf(str, "%ld:%ld:%ld", &tin[0], &tin[1], &tin[2]) < 1) {
		 	fatal(MSGSTR( BADTIME, "acctcom: bad time:"), str); 
		}
		for(i=0;i < 3;i++)
		{
			switch(nlt[3*i])
			{
				case 'h':
				case 'H':
					hr = tin[i];
					break;
				case 'm':
				case 'M':
					min = tin[i];
					break;
				case 's':
				case 'S':
					sec = tin[i];
					break;
			}
		}
	}
	else
	{
		if(sscanf(str, "%ld:%ld:%ld", &hr, &min, &sec) < 1) {
			fatal(MSGSTR( BADTIME, "acctcom: bad time:"), str);
		}
	}
	tzset();
	sec += (min*60);
	sec += (hr*3600);
	return(sec);
}

char com_out1[] = "\nCMDS=%ld REAL=%-6.2f CPU=%-6.2f USER=%-6.2f SYS=%-6.2f ";
char com_out2[] = "CHAR=%-8.2f BLK=%-8.2f USR/TOT=%-4.2f HOG=%-4.2f \n";

static
doexit(status)
{
	if(!average)
		exit(status);
	if(cmdcount) {
		cputot = systot + usertot;
		(void)fprintf(stdout, MSGSTR( COM_OUT1, com_out1),
			cmdcount, realtot/cmdcount, cputot/cmdcount,
			usertot/cmdcount, systot/cmdcount);
		(void)fprintf(stdout, MSGSTR( COM_OUT2, com_out2),
			iotot/cmdcount, rwtot/cmdcount, usertot/cputot,
			cputot/realtot);
	}
	else
		(void)fprintf(stdout, MSGSTR( NOCMDS, "\nNo commands matched\n"));
	exit(status);
}

static
isdevnull()
{
	struct stat	filearg;
	struct stat	devnull;

	if(fstat(0,&filearg) == -1) {
		(void)fprintf(stderr,
			MSGSTR( NOSTATIN, "acctcom: cannot stat stdin\n"));
		return(NULL);
	}
	if(stat("/dev/null",&devnull) == -1) {
		(void)fprintf(stderr,
			MSGSTR(NOSTATNULL, "acctcom: cannot stat /dev/null\n"));
		return(NULL);
	}

	if(filearg.st_rdev == devnull.st_rdev) return(1);
	else return(NULL);
}

static
fatal(s1, s2)
char *s1, *s2;
{
	(void)fprintf(stderr,"acctcom: %s %s\n", s1, s2);
	exit(1);
}

static
usage()
{
	(void)fprintf(stderr, 
		MSGSTR( COMUSAGE1, "Usage: acctcom [options] [files]\n"));
	(void)fprintf(stderr, MSGSTR( COMUSAGE2, "\nWhere options can be:\n"));
	diag(MSGSTR( COMUSAGE3, "-b	read backwards through file"));
	diag(MSGSTR( COMUSAGE4, "-f	print the fork/exec flag and exit status"));
	diag(MSGSTR( COMUSAGE5, "-h	print hog factor (total-CPU-time/elapsed-time)"));
	diag(MSGSTR( COMUSAGE6, "-i	print I/O counts"));
	diag(MSGSTR( COMUSAGE7, "-k	show total Kcore minutes instead of memory size"));
	diag(MSGSTR( COMUSAGE8, "-m	show mean memory size"));
	diag(MSGSTR( COMUSAGE9, "-r	show CPU factor (user-time/(sys-time + user-time))"));
	diag(MSGSTR( COMUSAGE10, "-t	show separate system and user CPU times"));
	diag(MSGSTR( COMUSAGE11, "-v	don't print column headings"));
	diag(MSGSTR( COMUSAGE12, "-a	print average statistics of selected commands"));
	diag(MSGSTR( COMUSAGE13, "-q	print average statistics only"));
	diag(MSGSTR( COMUSAGE14, "-l line	\tshow processes belonging to terminal /dev/line"));
	diag(MSGSTR( COMUSAGE15, "-u user	\tshow processes belonging to user name or user ID"));
	diag(MSGSTR( COMUSAGE16, "-u #	\tshow processes executed by super-user"));
	diag(MSGSTR( COMUSAGE17, "-u ?	\tshow processes executed by unknown UID's"));
	diag(MSGSTR( COMUSAGE18, "-g group	show processes belonging to group name of group ID"));
	diag(MSGSTR( COMUSAGE19, "-s time	\tshow processes ending after time (hh[:mm[:ss]])"));
	diag(MSGSTR( COMUSAGE20, "-e time	\tshow processes starting before time"));
	diag(MSGSTR( COMUSAGE21, "-S time	\tshow processes starting after time"));
	diag(MSGSTR( COMUSAGE22, "-E time	\tshow processes ending before time"));
	diag(MSGSTR( COMUSAGE23, "-n regex	select commands matching the ed(1) regular expression"));
	diag(MSGSTR( COMUSAGE24, "-o file	\tdo not print, put selected pacct records into file"));
	diag(MSGSTR( COMUSAGE25, "-H factor	show processes that exceed hog factor"));
	diag(MSGSTR( COMUSAGE26, "-O sec	\tshow processes that exceed CPU system time sec"));
	diag(MSGSTR( COMUSAGE27, "-C sec	\tshow processes that exceed total CPU time sec"));
	diag(MSGSTR( COMUSAGE28, "-I chars	show processes that transfer more than char chars"));
}

regerr( num )
int num;
{
	fprintf(stderr,MSGSTR(REG_ERROR,"acctcom: Error %d in compiling reg. expression\n"), num);
}
