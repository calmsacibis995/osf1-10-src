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
static char rcsid[] = "@(#)$RCSfile: uusched.c,v $ $Revision: 4.3.8.3 $ (DEC) $Date: 1993/10/11 19:35:57 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uusched.c
 * 
 * FUNCTIONS: Muusched, cleanup, exuucico, logent, machine 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
1.6  uusched.c, bos320 10/17/90 17:01:38";
*/
#include	"uucp.h"
/* VERSION( uusched.c	5.2 -  -  ); */

#define USAGE		MSGSTR(MSG_UUSCHED1, "[-xNUM] [-uNUM]")

struct m {
	char	mach[15];
	int	ccount;
} M[UUSTAT_TBL+2];

short Uopt;
void cleanup();

void logent(){}		/* to load ulockf.c */

main(argc, argv, envp)
char *argv[];
char **envp;
{
	struct m *m, *machine();
	DIR *spooldir, *subdir;
	char *str, *rindex();
#ifdef PDA
	char f[NAME_MAX+256], subf[NAME_MAX+256];
#else
	char f[256], subf[256];
#endif
	short num, snumber;
	char lckname[MAXFULLNAME];
	int i, maxnumb;
	FILE *fp;
	char lineBuf[BUFSIZ];

	Uopt = 0;
	Env = envp;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);


	(void) strcpy(Progname, "uusched");
	while ((i = getopt(argc, argv, "u:x:")) != EOF) {
		switch(i){
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			break;
		case 'u':
			Uopt = atoi(optarg);
			if (Uopt <= 0)
				Uopt = 1;
			break;
		default:
			(void) fprintf(stderr, MSGSTR(MSG_UUSCHED2, 
				"\tusage: %s %s\n"), Progname, USAGE);
			cleanup(1);
		}
	}
	if (argc != optind) {
		(void) fprintf(stderr, MSGSTR(MSG_UUSCHED2, 
			"\tusage: %s %s\n"), Progname, USAGE);
		cleanup(1);
	}

	DEBUG(9, "Progname (%s): STARTED\n", Progname);
	srand(getuid());	/* Seed the random number generator */
	maxnumb = 1;		/* If there's no ctl file, or we can't read it,
				   we'll allow 1 incarnation. */
	fp = fopen(LMTUUSCHED, "r");
	if (fp == NULL) {
		DEBUG(1, "No limitfile - %s\n", LMTUUSCHED);
	} else {
		/*
		 * Skip lines in Maxuuscheds that have '#' 
		 * in column 1 since that denotes a comment line.
		 */
		while (fgets(lineBuf, BUFSIZ, fp))
			if (*lineBuf != '#')
				if (sscanf(lineBuf, "%d", &maxnumb) == 1)
					break;
		(void) fclose(fp);
		DEBUG(4, "Uusched limit %d -- ", maxnumb);

		for (i=0; i<maxnumb; i++) {
		    (void) sprintf(lckname, "%s.%d", S_LOCK, i);
		    if ( ulockf(lckname, (time_t)  X_LOCKTIME) == 0)
			break;
		}
		if (i == maxnumb) {
		    DEBUG(4, "found %d -- cleanuping\n ", maxnumb);
		    cleanup(0);
		}
		DEBUG(4, "continuing\n", maxnumb);
	}

	if (chdir(SPOOL) != 0 || (spooldir = opendir(SPOOL)) == NULL)
		cleanup(101);		/* good old code 101 */
	while (gnamef(spooldir, f) == TRUE) {
	    if (EQUALSN("LCK..", f, 5))
		continue;

	    if (DIRECTORY(f) && (subdir = opendir(f))) {
	        while (gnamef(subdir, subf) == TRUE)
		    if (subf[1] == '.') {
		        if (subf[0] == CMDPRE) {
				/* Note - we can break now, but the
				 * count may be useful for enhanced
				 * scheduling
				 */
				machine(f)->ccount++;
			}
		    }
		closedir(subdir);
	    }
	}

	/* Make sure the overflow entry is null since it may be incorrect */
	M[UUSTAT_TBL].mach[0] = NULLCHAR;

	/* count the number of systems */
	for (num=0, m=M; m->mach[0] != '\0'; m++, num++) {
	    DEBUG(5, "machine: %s, ", M[num].mach);
	    DEBUG(5, "ccount: %d\n", M[num].ccount);
	}

	DEBUG(5, "Execute num=%d \n", num);
	while (num > 0) {
	    snumber = rand() % num;  /* random number */
	    (void) strcpy(Rmtname, M[snumber].mach);
	    DEBUG(5, "num=%d, ", num);
	    DEBUG(5, "snumber=%d, ", snumber);
	    DEBUG(5, "Rmtname=%s\n", Rmtname);
	    (void) sprintf(lckname, "%s.%s", LOCKPRE, Rmtname);
	    if (checkLock(lckname) != FAIL && callok(Rmtname) == 0) {
		/* no lock file and status time ok */
		DEBUG(5, "call exuucico(%s)\n", Rmtname);
		exuucico(Rmtname);
	    }
	    else {
		/* system locked - skip it */
		DEBUG(5, "system %s locked or inappropriate status--skip it\n",
		    Rmtname);
	    }
	    
	    M[snumber] = M[num-1];
	    num--;
	}
	cleanup(0);
}

struct m	*
machine(name)
char	*name;
{
	struct m *m;
	int	namelen;

	namelen = strlen(name);
	DEBUG(9, "machine(%s) called\n", name);
	for (m = M; m->mach[0] != '\0'; m++)
		/* match on overlap? */
		if (EQUALSN(name, m->mach, SYSNSIZE)) {
			/* use longest name */
			if (namelen > strlen(m->mach))
				(void) strcpy(m->mach, name);
			return(m);
		}

	/*
	 * The table is set up with 2 extra entries
	 * When we go over by one, output error to errors log
	 * When more than one over, just reuse the previous entry
	 */
	if (m-M >= UUSTAT_TBL) {
	    if (m-M == UUSTAT_TBL) {
		errent(MSGSTR(MSG_UUSCHED3, "MACHINE TABLE FULL"), "", 
		UUSTAT_TBL, rcsid, __FILE__, __LINE__);
	    }
	    else
		/* use the last entry - overwrite it */
		m = &M[UUSTAT_TBL];
	}

	(void) strcpy(m->mach, name);
	m->ccount = 0;
	return(m);
}

exuucico(name)
char *name;
{
	char cmd[BUFSIZ];
	short ret;
	char uopt[5];
	char sopt[BUFSIZ];

	(void) sprintf(sopt, "-s%s", name);
	if (Uopt)
	    (void) sprintf(uopt, "-x%.1d", Uopt);

	if (vfork() == 0) {
	    if (Uopt)
	        (void) execle(UUCICO, "UUCICO", "-r1", uopt, sopt, 0, Env);
	    else
	        (void) execle(UUCICO, "UUCICO", "-r1", sopt, 0, Env);

	    cleanup(100);
	}
	(void) wait(&ret);

	DEBUG(3, "ret=%d, ", ret);
	DEBUG(3, "errno=%d\n", errno);
}


void
cleanup(code)
int	code;
{
	rmlock(CNULL);
	clrlock(CNULL);
	catclose(catd);
	exit(code);
}
