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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: uuq.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1993/10/11 19:35:55 $";
#endif
/* 
 * COMPONENT_NAME: CMDUUCP uuq.c
 * 
 * FUNCTIONS: Muuq, analjob, cleanup, gather, getsys, jcompare, 
 *            prefix, wprefix 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * uuq - looks at uucp queues
 *
 * Lou Salkind
 * New York University
 *
 * "uuq.c	4.6 (Berkeley) 10/9/85";
 */

#include "uucp.h"
#include <stdio.h>
#include <sys/timeb.h>
#include <sys/dir.h>
#include <sys/syslimits.h>
#include <sys/stat.h>

#define	NOSYS		(struct sys *)0

#define W_TYPE		wrkvec[0]
#define W_FILE1		wrkvec[1]
#define W_FILE2		wrkvec[2]
#define W_USER		wrkvec[3]
#define W_OPTNS		wrkvec[4]
#define W_DFILE		wrkvec[5]
#define W_MODE		wrkvec[6]
#define WSUFSIZE 5	/* work file name suffix size */

#define	USAGE "usage: uuq [-l] [-h] [-s system] [-u user] [-d jobno] [-r spool] [-b baudrate]\n"

struct sys {
	char	s_name[8];
	int	s_njobs;
	off_t	s_bytes;
	struct job	*s_jobp;
	struct sys	*s_sysp;
};

struct job {
	int	j_files;
	int	j_flags;
	char	j_jobno[WSUFSIZE];
	char	j_user[22];
	char	j_fname[128];
	char	j_grade;
	off_t	j_bytes;
	time_t	j_date;
	struct job	*j_jobp;
};

nl_catd catd;
struct sys *syshead;
struct sys *getsys();
int jcompare();
char *sysname;
char *user;
char *rmjob;
int hflag = 0;
int lflag = 0;

float baudrate = 1200.;
char Username[BUFSIZ];
char Filename[BUFSIZ];
int Maxulen = 0;
struct timeb Now;

main(argc, argv)
char **argv;
{
	register i;
	register struct sys *sp;
	register struct job *jp;
	struct job **sortjob;
	int nsys, ret;

	strcpy(Progname, "uuq");
	strcpy(Spool, SPOOL);
	myName(Myname);

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);

	while ((ret = getopt(argc, argv, "lhs:u:d:r:b:")) != EOF) {
		switch (ret) {
		case 'l':
			lflag++;
			break;
		case 'h':
			hflag++;
			break;
		case 'r':
			strcpy(Spool, optarg);
			break;
		case 's':
			sysname = optarg;
			break;
		case 'u':
			user = optarg;
			break;
		case 'd':
			rmjob = optarg;
			break;
		case 'b':
			baudrate = strtoul(optarg);
			break;
		default:
			fprintf(stderr, MSGSTR(MSG_UUQ1,USAGE));
			exit(2);
		}
	}

	ASSERT(chdir(Spool) == 0, Ct_CHDIR, Spool, errno);
	baudrate *= 0.7;	/* reduce speed because of protocol overhead */
	baudrate *= 6.; 	/* convert to chars/minute (60/10) */
	gather();
	nsys = 0;

	for (sp = syshead; sp; sp = sp->s_sysp) {
		if (sp->s_njobs == 0)
			continue;
		if (!hflag && nsys++ > 0)
			putchar('\n');
		printf("%s: %d %s", sp->s_name,
			sp->s_njobs, sp->s_njobs > 1 ? "jobs" : "job");
		if (lflag) {
			float minutes;
			int hours;
			/* The 80 * njobs is because of the uucp handshaking */
			minutes = (float)(sp->s_bytes+80 * sp->s_njobs)/baudrate;
			hours = minutes/60;
			printf(", %d bytes, ", sp->s_bytes);
			if (minutes > 60){
				printf("%d hour%s, ",hours,
					hours > 1 ? "s": "");
				minutes -= 60 * hours;
			}
			printf(MSGSTR(MSG_UUQ8,"%3.1f minutes (@ effective baudrate of %d)"),
				minutes,(int)baudrate/6);
		}
		putchar('\n');
		if (hflag)
			continue;
		/* sort them babies! */
		sortjob = (struct job **)calloc(sp->s_njobs, sizeof (struct job
 *));
		for (i=0, jp=sp->s_jobp; i < sp->s_njobs; i++, jp=jp->j_jobp)
			sortjob[i] = jp;
		qsort(sortjob, sp->s_njobs, sizeof (struct job *), jcompare);
		for (i = 0; i < sp->s_njobs; i++) {
			jp = sortjob[i];
			if (lflag) {
				printf("%s %2d %-*s%7d%5.1f %-12.12s %c %.*s\n",
	jp->j_jobno, jp->j_files, Maxulen, jp->j_user, jp->j_bytes, jp->j_bytes/baudrate,
	ctime(&jp->j_date) + 4, jp->j_flags, sizeof (jp->j_fname), jp->j_fname
				);
			} else {
				printf("%s", jp->j_jobno);
				putchar((i+1)%10 ? '\t' : '\n');
			}
			/* There's no need to keep the force poll if jobs > 1*/
/*
			if (sp->s_njobs > 1 && strcmp("POLL", jp->j_jobno)==0) {
				char pbuf[BUFSIZ];
				sprintf(pbuf,"%s/%c.%szPOLL", subdir(Spool, CMDPRE), CMDPRE,sp->s_name);
				unlink(pbuf);
			}
*/
		}
		if (!lflag && (sp->s_njobs%10))
			putchar('\n');
	}
	exit(0);
}

jcompare(j1, j2)
struct job **j1, **j2;
{
	int delta;

	delta = (*j1)->j_grade - (*j2)->j_grade;
	if (delta)
		return delta;
	return(strcmp((*j1)->j_jobno,(*j2)->j_jobno));
}

/*
**	Get all the command (C.*) filenames
*/

gather()
{
	struct dirent *d;
	DIR *df, *sd;			/* df = dir, sd = subdir */
	char	f[NAME_MAX], name[NAME_MAX], tmp[NAME_MAX];

	/*
	** Find all the spool files.  At the /usr/spool/uucp level there
	** should only be directory entries.  Soooo...we will grab a directory
	** name and then recursively travel downward and grab all the command
	** filenames.
	*/

	if ((df = opendir(Spool)) == NULL) {
		fprintf(stderr, MSGSTR(MSG_UUQ2,"can't examine spooling area"));
		exit(1);
	}
				/*
				** here we are looking at directories...
				*/
	while (gnamef(df, f) == TRUE) {
		if (DIRECTORY(f)) {
			if ((sd = opendir(f)) != NULL) {
				while(gnamef(sd, name) == TRUE) {
					if ((strlen(name) > 2) &&
					  (EQUALSN(name, "C.", 2))) {
						if (analjob(f, name) < 0) {
							fprintf(stderr, MSGSTR(MSG_UUQ3,
							  "out of memory\n"));
							exit(1);
						}
					}
				}
				closedir(sd);
			} else {
				fprintf(stderr, MSGSTR(MSG_UUQ4,"could not open '%s'\n"), f);
				exit(1);
			}
		}
	}
	closedir(df);
}

/*
**	analjob(f, filename)
**	char *f, filename;
**
**	"f" is the directory containing "filename".  "f" is relative
**	to /usr/spool/uucp.  The "f" parameter was added because 
**	Berkeley 'stock' UUCP would simply throw every single outgoing
**	UUCP chore in the /usr/spool/uucp directory;  HoneyDanBer creates
**	a separate directory for each system with work queued.
*/

analjob(f, filename)
char	*f;
char	*filename;
{
	struct job *jp;
	struct sys *sp;
	char sbuf[NAME_MAX+1], str[NAME_MAX], nbuf[NAME_MAX];
	char  *jptr, *wrkvec[20];
	char grade;
	FILE *fp, *df;
	struct stat statb;
	int files, gotname, i;
	off_t bytes;
	char	tmp[NAME_MAX], tmp2[NAME_MAX];

	strncpy(sbuf, filename, NAME_MAX);
	sbuf[NAME_MAX] = '\0';
	jptr = sbuf + strlen(sbuf) - WSUFSIZE;
	grade = *jptr;
	*jptr++ = 0;
	/*
	 * sbuf+2 now points to sysname name (null terminated)
	 * jptr now points to job number (null terminated)
	 */
	if (rmjob) {
		if (strcmp(rmjob, jptr))
			return(0);
	} else {
		if ((sp = getsys(sbuf+2)) == NOSYS)
			return(0);
		if (!lflag) {
			/* SHOULD USE A SMALLER STRUCTURE HERE */
			jp = (struct job *)malloc(sizeof(struct job));
			if (jp == (struct job *)0)
				return(-1);
			strcpy(jp->j_jobno, jptr);
			jp->j_jobp = sp->s_jobp;
			jp->j_grade = grade;
			sp->s_jobp = jp;
			sp->s_njobs++;
			return(1);
		}
	}

	sprintf(tmp, "%s/%s", f, filename);
	if ((fp = fopen((char *)subfile(tmp), "r")) == NULL) {
		perror((char *)subfile(tmp));
		return(0);
	}
	files = 0;
	bytes = 0;
	gotname = 0;
	while (fgets(str, sizeof str, fp)) {
		if (getargs(str, wrkvec, 20) <= 0)
			continue;
		if (rmjob) {
			if (W_TYPE[0] == 'S' && !index(W_OPTNS, 'c')) {
				sprintf(tmp2, "%s/%s", f, W_DFILE);
				unlink((const char *)subfile(tmp2));
				fprintf(stderr, MSGSTR(MSG_UUQ5,"Removing data file %s\n"), W_DFILE);
			}
			continue;
		}
		if (user && (W_TYPE[0] == 'X' || !prefix(user, W_USER))) {
			fclose(fp);
			return(0);
		}
		files++;
		if (W_TYPE[0] == 'S') {
			if (strcmp(W_DFILE, "D.0") &&
			    stat((char *)subfile(W_DFILE), &statb) >= 0)
				bytes += statb.st_size;
			else if (stat((char *)subfile(W_FILE1), &statb) >= 0)
				bytes += statb.st_size;
		}
		/* amusing heuristic */
#define	isXfile(s)	(s[0]=='D' && s[strlen(s)-WSUFSIZE]=='X')
		if (gotname == 0 && isXfile(W_FILE1)) {
			if ((df = fopen((char *)subfile(W_FILE1), "r")) == NULL)
				continue;
			while (fgets(nbuf, sizeof nbuf, df)) {
				nbuf[strlen(nbuf) - 1] = '\0';
				if (nbuf[0] == 'C' && nbuf[1] == ' ') {
					strcpy(Filename, nbuf+2);
					gotname++;
				} else if (nbuf[0] == 'R' && nbuf[1] == ' ') {
					register char *p, *q, *r;
					r = q = p = nbuf+2;
					do {
						if (*p == '!' || *p == '@'){
							r = q;
							q = p+1;
						}
					} while (*p++);

					strcpy(Username, r);
					W_USER = Username;
				}
			}
			fclose(df);
		}
	}
	fclose(fp);
	if (rmjob) {
		unlink((const char *)subfile(tmp));
		fprintf(stderr, MSGSTR(MSG_UUQ6,"Removing command file %s\n"), filename);
		exit(0);
	}
	if (files == 0) {
		static char *wtype = "X";
		static char *wfile = "forced poll";
		if (strcmp("POLL", &tmp[strlen(tmp)-4])) {
		     fprintf(stderr, MSGSTR(MSG_UUQ7,"%.14s: empty command file\n"), filename);
		     return(0);
		}
		W_TYPE = wtype;
		W_FILE1 = wfile;
	}
	jp = (struct job *)malloc(sizeof(struct job));
	if (jp == (struct job *)0)
		return(-1);
	strcpy(jp->j_jobno, jptr);
	jp->j_files = files;
	jp->j_bytes = bytes;
	jp->j_grade = grade;
	jp->j_flags = W_TYPE[0];
	strncpy(jp->j_user, W_TYPE[0]=='X' ? "---" : W_USER, 20 );
	jp->j_user[20] = '\0';
	i = strlen(jp->j_user);
	if (i > Maxulen)
		Maxulen = i;
	/* SHOULD ADD ALL INFORMATION IN THE WHILE LOOP */
	if (gotname)
		strncpy(jp->j_fname, Filename, sizeof jp->j_fname);
	else
		strncpy(jp->j_fname, W_FILE1, sizeof jp->j_fname);
	stat((char *)subfile(tmp), &statb);
	jp->j_date = statb.st_mtime;
	jp->j_jobp = sp->s_jobp;
	sp->s_jobp = jp;
	sp->s_njobs++;
	sp->s_bytes += jp->j_bytes;
	return(1);
}

struct sys *
getsys(s)
register char *s;
{
	register struct sys *sp;

	for (sp = syshead; sp; sp = sp->s_sysp)
		if (strcmp(s, sp->s_name) == 0)
			return(sp);

	if (sysname && (!EQUALSN(sysname, s, (strlen(sysname) < strlen(s)) ? 
			strlen(sysname) : strlen(s))))
		return(NOSYS);
	sp = (struct sys *)malloc(sizeof(struct sys));
	if (sp == NOSYS)
		return(NOSYS);
	strcpy(sp->s_name, s);
	sp->s_njobs = 0;
	sp->s_jobp = (struct job *)0;
	sp->s_sysp = syshead;
	sp->s_bytes = 0;
	syshead = sp;
	return(sp);
}


/*
**	cleanup(value)
**	int	value;
**
**	This is here because many of the generic UUCP routines call
**	cleanup() versus just preforming an exit.  This very simple
**	routine was included primarily to prevent unresolved references
**	at link time.
*/
cleanup(value)
int	value;
{
	catclose(catd);

	exit(value);
}


/*
**	prefix(s1, s2)
**	char *s1, *s2;
**
**	Does s1 contain s2? 
**	return 0 - !=
**	return 1 - ==
*/
prefix(s1, s2)
register char *s1, *s2;
{
	if (EQUALSN(s1, s2, (strlen(s1) < strlen(s2)) ? strlen(s1) : 
			strlen(s2)))
		return(1);
	else
		return(0);
}


/*
 *	check s2 for prefix s1 with a wildcard character ?
 *
 *	return 0 - !=
 *	return 1 - == 
 */

wprefix(s1, s2)
register char *s1, *s2;
{
	register char c;

	while ((c = *s1++) != '\0')
		if (*s2 == '\0'  ||  (c != *s2++  &&  c != '?'))
			return 0;
	return 1;
}
