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
static char *rcsid = "@(#)$RCSfile: zdump.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/11/19 16:23:23 $";
#endif
/************************************************************************
 *			Modification History				*
 *									*
 * 001	Ken Lesniak, 17-Jul-1989					*
 *	TZ environment string was missing leading ":".			*
 ************************************************************************/

#include <stdio.h>

#include <sys/types.h>
#include <tzfile.h>
#include <sys/time.h>
#include <time.h>

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

extern char *		asctime();
extern char **		environ;
extern struct tm *	gmtime();
extern char *		imalloc();
extern char *		optarg;
extern int		optind;
extern time_t		time();
extern char *		tzname[2];
extern void		tzset();

/*
** For the benefit of cyntax...
*/
extern int                     _zdump;
static long		tzdecode();
static			readerr();
static			show();

static int		longest;

static long
tzdecode(codep)
char *	codep;
{
	register int	i;
	register long	result;

	result = 0;
	for (i = 0; i < 4; ++i)
		result = (result << 8) | (codep[i] & 0xff);
	return result;
}

main(argc, argv)
int	argc;
char *	argv[];
{
	register FILE *	fp;
	register int	i, j, c;
	register int	vflag;
	register char *	cutoff;
	register int	cutyear;
	register long	cuttime;
	time_t		now;
	time_t		t;
	long		timecnt;
	char		buf[BUFSIZ];

	vflag = 0;
	cutoff = NULL;
	while ((c = getopt(argc, argv, "c:v")) == 'c' || c == 'v')
		if (c == 'v')
			vflag = 1;
		else	cutoff = optarg;
	if (c != EOF || optind == argc) {
		(void) fprintf(stderr, 
			"%s: usage is %s [ -v ] [ -c cutoffyear ] zonename ...\n",
			argv[0], argv[0]);
		exit(1);
	}
	if (cutoff != NULL)
		cutyear = atoi(cutoff);
	/*
	** VERY approximate.
	*/
	cuttime = (long) (cutyear - EPOCH_YEAR) *
		SECS_PER_HOUR * HOURS_PER_DAY * DAYS_PER_NYEAR;
	(void) time(&now);
	longest = 0;
	for (i = optind; i < argc; ++i)
		if (strlen(argv[i]) > longest)
			longest = strlen(argv[i]);
	for (i = optind; i < argc; ++i) {
		register char **	saveenv;
		char *			tzequals;
		char *			fakeenv[2];

		tzequals = imalloc(strlen(argv[i]) + 5);
		if (tzequals == NULL) {
			(void) fprintf(stderr, "%s: can't allocate memory\n",
				argv[0]);
			exit(1);
		}
		(void) sprintf(tzequals, "TZ=:%s", argv[i]);
		fakeenv[0] = tzequals;
		fakeenv[1] = NULL;
		saveenv = environ;
		environ = fakeenv;
		(void) tzset();
		environ = saveenv;
		show(argv[i], now, FALSE);
		if (!vflag)
			continue;
		if (argv[i][0] == '/')
			fp = fopen(argv[i], "r");
		else {
			j = strlen(TZDIR) + 1 + strlen(argv[i]) + 1;
			if (j > sizeof buf) {
				(void) fprintf(stderr,
					"%s: timezone name %s/%s is too long\n",
					argv[0], TZDIR, argv[i]);
				exit(1);
			}
			(void) sprintf(buf, "%s/%s", TZDIR, argv[i]);
			fp = fopen(buf, "r");
		}
		if (fp == NULL) {
			(void) fprintf(stderr, "%s: Can't open ", argv[0]);
			perror(argv[i]);
			exit(1);
		}
		{
			char		code[4];

(void) fseek(fp, (long) sizeof ((struct tzhead *) 0)->tzh_reserved, 0);
			if (fread((char *) code, sizeof code, 1, fp) != 1)
				readerr(fp, argv[0], argv[i]);
			timecnt = tzdecode(code);
			(void) fseek(fp, (long) (2 * sizeof code), 1);
		}
		t = 0x80000000;
		if (t > 0)		/* time_t is unsigned */
			t = 0;
		show(argv[i], t, TRUE);
		t += SECS_PER_HOUR * HOURS_PER_DAY;
		show(argv[i], t, TRUE);
		while (timecnt-- > 0) {
			char	code[4];

			if (fread((char *) code, sizeof code, 1, fp) != 1)
				readerr(fp, argv[0], argv[i]);
			t = tzdecode(code);
			if (cutoff != NULL && t > cuttime)
				break;
			show(argv[i], t - 1, TRUE);
			show(argv[i], t, TRUE);
		}
		if (fclose(fp)) {
			(void) fprintf(stderr, "%s: Error closing ", argv[0]);
			perror(argv[i]);
			exit(1);
		}
		t = 0xffffffff;
		if (t < 0)		/* time_t is signed */
			t = 0x7fffffff ;
		t -= SECS_PER_HOUR * HOURS_PER_DAY;
		show(argv[i], t, TRUE);
		t += SECS_PER_HOUR * HOURS_PER_DAY;
		show(argv[i], t, TRUE);
		free(tzequals);
	}
	if (fflush(stdout) || ferror(stdout)) {
		(void) fprintf(stderr, "%s: Error writing standard output ",
			argv[0]);
		perror("standard output");
		exit(1);
	}
	return 0;
}

static
show(zone, t, v)
char *	zone;
time_t	t;
{
	struct tm *		tmp;
	extern struct tm *	localtime();

	(void) printf("%-*s  ", longest, zone);
	if (v)
		(void) printf("%.24s GMT = ", asctime(gmtime(&t)));
	_zdump = 1;
	tmp = localtime(&t);
	_zdump = 0;
	(void) printf("%.24s", asctime(tmp));
	if (*tzname[tmp->tm_isdst] != '\0')
		(void) printf(" %s", tzname[tmp->tm_isdst]);
	if (v) {
		(void) printf(" isdst=%d", tmp->tm_isdst);
		(void) printf(" gmtoff=%ld", tmp->tm_gmtoff);
	}
	(void) printf("\n");
}

static
readerr(fp, progname, filename)
FILE *	fp;
char *	progname;
char *	filename;
{
	(void) fprintf(stderr, "%s: Error reading ", progname);
	if (ferror(fp))
		perror(filename);
	else	(void) fprintf(stderr, "%s: Premature EOF\n", filename);
	exit(1);
}
