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
static char *rcsid = "@(#)$RCSfile: man.c,v $ $Revision: 4.2.13.4 $ (DEC) $Date: 1993/10/11 17:26:00 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: runpath, manual, pathstat, scanpath, select_manpage,
 *	      nroff, troff, any, local_remove,
 *	      blklen, apropos, match, amatch, lmatch_wc, lmatch, whatis,
 *	      wmatch, trim, tail, usage, usage2
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
 * 
 * 	man.c	1.21  com/cmd/man,3.1,9021 3/6/90 18:06:26
 */

/* Modification History
 * 
 * 001	gws 17-Apr-91
 *	fixed up existing single-character subsection code so it works
 *	added code for multi-character subsections
 *	  looks for an exact match for
 *		<manpath>/{man,cat}<section>/<manpage>.<section><subsection>
 *			or
 *		<manpath>/{man,cat}<section>/<manpage>.<section><subsection>.z
 *	  if "<section><subsection> <manpage>"  is specified
 *	added code to search for <manpage>.<section>*
 *	  if a manpage is specified with no section[subsection]
 *		or
 *	  if a manpage is specified with a section (no subsection)
 *	  AND <manpage.<section> nor <manpage>.<section>.z
 *		do not exist
 *
 *	realname[] is now external
 *	subsec string added
 *	scanpath(name, path, stbuf) function added:
 *	  given a root manpage name <name>.<section>, it searches
 *	  <manpath>/{man,cat}<section> for the first alphabetically-occuring
 *	  manpage, if any, that whose name matches "<name>.<section>*", for
 *	  each existing {man,cat}<section> subdirectory in each <manpath>
 *	  directory.
 *
 *	  The root manpage name is contained in external realname[], and
 *	  the matching manpage, if any, is returned in realname[].  The "name"
 *	  parameter is actually the initial manpath subdirectory to search. The
 *	  matching full pathname of the directory containing realname[] is
 *	  returned in the "path" parameter, along with its stat() status in the
 *	  "stbuf" parameter.  Success status is 1.
 *
 *	  The C library scandir() and alphasort() functions are used to scan
 *	  and sort the <manpath>/{man,cat}<section> directory.  The internal
 *	  function select_manpage() is used by scandir() to select the
 *	  directory records to be sorted by alphasort(), if any.
 *	select_manpage(dirp) added:
 *	  selects the directory records located by scandir() that are to be
 *	  sorted by alphasort().  The records to be selected are those whose
 *	  names contain <name>.<section> as the root of their names, if any.
 *
 * 002	gws	(OSF_QAR 1670)
 *	changed to no longer ignore manpages whose 1st line contains
 *		.soman?*
 *		.so?*
 *	or	.so<blank>?*
 *	where:		?*  = anything other than "man"
 *	This change permits .so calls to non-manpages, such as .so calls to SML
 *	and RSML macros, to appear on the first line, so they can be formatted
 *	instead of skipped.  It also no longer strictly requires a <blank>
 *	after ".so", because nroff does not require it.
 *
 * 003	gws	(OSF_QAR 1432)
 *	now uses more(1) -vf options, so files containing Escape sequences
 *	can be viewed, and keywords searched.  This goes along with a change
 *	in /usr/share/lib/term, to link the "lp" nroff device to the "vt100"
 *	device.  The -vf options are necessary to the viewing of files
 *	formatted for VT100(tm) series devices.
 *		VT and VT100 are trademarks of Digital Equipment Corporation
 *
 * 004	gws 11/05/91	(OSF_QAR 1695)
 *	now pre-processes source manpages through tbl(1) and neqn(1) before
 *	  being formatted by nroff.
 *	now post-processes manpages through "col -p" after they are formatted
 *	  by nroff.  The "-p" option of col(1) is specified in case nroff
 *	  "lp" device is something other than a dumb printer (nroff "lpr"
 *	  device)
 *	now pre-processes source manpages through tbl(1) and the eqn command
 *	  (not provided by OSF/1 R1.0) before being formatted by troff (not
 *	  provided by OSF/1 R1.0).
 *	cmdbuf[] in the manual() function is increased to BUFSIZ+1 in size
 *	  to make sure the formatted nroff command string accomodates very
 *	  long manpage names
 *
 * 005	gws 11/27/91	(OSF_QAR 2217, 2253)
 *	 remove postprocessing of manpages through "col -p", because
 *	  col(1) command counts the Escape codes as text that "-p" passes
 *	  through, so col miscalculates no. of tabs to generate when it
 *	  converts spaces into tabs by default.  Also, col is NOT required
 *	  to make tables vertically alligned, when the "-Tlp" device is
 *	  associated with a device that has reverse-linefeed AND the "-Tlp"
 *	  device is also the default terminal display device for the system.
 *	add the -h option to all nroff calls, because this generates tabs,
 *	  and the "-Tlp" default device needs the tabs to step over left
 *	  columns in multicolumn tables containing text blocks, else the
 *	  left columns get wiped out.
 *	the result of the above changes is that NROFFCAT is restored to
 *	  its original R1.0 value.
 *	  
 */


/*              include file for message texts          */

#include "man_msg.h"
nl_catd  catd;   /* Cat descriptor for scmc conversion */

#define MSGSTR(num,str)         catgets(catd, MS_man, num, str)

#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include <sgtty.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <signal.h>
#include <string.h>

#include <sys/types.h>						/* 001 */
#include <sys/dir.h>						/* 001 */

/*
 * man
 * link also to apropos and whatis
 * This version uses more for underlining and paging.
 */
#define	chrlen(ptr)		(mblen(ptr, MB_LEN_MAX))

#ifndef	NROFFX
#define NROFFX "/usr/bin/nroff"        /*  need to check if nroff installed */
#endif
#define TROFFX "/usr/bin/troff"        /*  need to check if troff installed */
#define NROFFCAT "nroff -h -man -Tlp"  /* for nroffing to cat file */
#define NROFF   "nroff -h -man -Tlpr"  /* for nroffing to tty    * 004 * 005 */
#define MORE    "more -svf"            /* paging filter */	      /* 003 */
#define CAT_    "/usr/bin/cat"         /* for when output is not a tty */
#define CAT_S   "/usr/bin/cat -s"      /* for '-' opt (no more) */
#define PCAT    "/usr/bin/pcat"        /* for when output is compressed */

#define TROFFCMD "tbl %s | eqn | troff -man -"			      /* 004 */

/*
 * Change the order such that the more frequently used 1-8 man pages
 * are search first. Original definition is
 * #define ALLSECT "CLFnlpo18623457"
 */
#define ALLSECT "18623457CLFnlpo" /* Order to search for sections */  /* 006 */
#define CSECT   "Cnlpo1"   /* Sections to search if C is specified */

#define WHATIS  "whatis"

int     nomore;
char    *CAT    = CAT_;
#ifdef	MANPATH
char    *manpath = MANPATH;
#else	/* MANPATH */
char    *manpath = "/usr/share/man:/usr/local/man";
#endif	/* MANPATH */
char    *pager = MORE;

char    *getenv();
char    *calloc();
char    *trim();
char    *tail();
int     local_remove(void);
int     apropos();
int     whatis();

int     section;
char	*subsec;						/* 001 */
int     troffit;
int     mypid;
int     zflag;

char realname[BUFSIZ+1];					/* 001 */

char *progname;
								/* start 012 */
int	whatis_found;	/* 0 if no "whatis" databases found,	       
			 * 1 if any "whatis" database found.
			 */	      				
int	manpath_change;	/* 0 if default manpath is not overridden by
			 *      -M <manpath> or MANPATH environment variable.
			 * 1 if -M <manpath> or MANPATH specified.
			 */
								/* end 012 */

/*
 * Look for the supported format codes %L, %l, %t, %c & %%.
 * These format codes are then translated into :
 *	%L -> value of LC_MESSAGES environment variable
 *	%P -> value of LC_MESSAGES environment variable excluding the modifier
 *	%l -> language element of the LC_MESSAGES environment variable
 *	%t -> territory element of the LC_MESSAGES environment variable
 *	%c -> codeset element of the LC_MESSAGES environment variable
 * assuming that the value of LC_MESSAGES environment variable will be
 * in the format:
 *	language[_territory[.codeset]][@modifier]
 *
 * It also remove any inaccessible path.
 */
static char *
expand_path(path)
	char *path ;
{
	static char  buf[BUFSIZ+1] ;
	char	     tmp[BUFSIZ+1] ;
	char	    *Pptr	   ;	/* Path name pointer	  */
	char	    *lptr	   ;	/* Language name pointer  */
	char	    *tptr	   ;	/* Territory name pointer */
	char	    *cptr	   ;	/* Codeset name pointer   */
	char	    *fptr	   ;	/* Format code pointer	  */
	char	    *bptr	   ;
	char	    *eptr	   ;
	char	    *pptr	   ;
	int	     len	   ;

	if ((fptr = (char *)index(path, '%')) == NULL)
		goto remove_path ;	/* No format code */
	/*
	 * Fetch current locale name
	 */
	eptr = (char *)setlocale(LC_MESSAGES, NULL) ;
	if ((eptr == NULL) || (strcmp(eptr, "C") == 0))
		goto remove_path ;

	strcpy(buf, eptr) ;
	/*
	 * The locale name is duplicated to isolate the language, territory
	 * and codeset component.
	 */
	Pptr = buf ;
	lptr = buf + strlen(buf) + 1 ;
	strcpy(lptr, eptr) ;
	if ((eptr = (char *)index(lptr, '@')) != NULL) {
		*eptr = '\0' ;	/* Remove modifier */
		Pptr  = lptr ;
		lptr  = eptr + 1   ;
		strcpy(lptr, Pptr) ;
	}
	if ((cptr = (char *)index(lptr, '.')) != NULL)
		*cptr++ = '\0';	/* Isolate codeset component */
	if ((tptr = (char *)index(lptr, '_')) != NULL)
		*tptr++ = '\0';	/* Isolate territory component */
	/*
	 * Expand the path name
	 */
	pptr = path ;
	bptr = tmp  ;
	do {
		len = fptr - pptr ;
		if (len >= BUFSIZ - 20 - (bptr - tmp))
			return(path)	;	/* Path too long */
		bcopy(pptr, bptr, len)	;
		pptr += len + 1		;
		bptr += len		;
		switch (*pptr++) {
		    case 'L':	strcpy(bptr, buf)    ;
				bptr += strlen(buf)  ;
				break		     ;
		    case 'P':	strcpy(bptr, Pptr)   ;
				bptr += strlen(Pptr) ;
				break		     ;
		    case 'l':	strcpy(bptr, lptr)   ;
				bptr += strlen(lptr) ;
				break		     ;
		    case 't':	if (tptr != NULL) {
				    strcpy(bptr, tptr)   ;
				    bptr += strlen(tptr) ;
				}
				break ;
		    case 'c':	if (cptr != NULL) {
				    strcpy(bptr, cptr)   ;
				    bptr += strlen(cptr) ;
				}
				break ;
		    case '%':   *bptr++ = '%' ;
				break 	      ;
		    default :	*bptr++ = '%' ;
				pptr--	      ;
				break	      ;
		}
	} while ((fptr = (char *)index(pptr, '%')) != NULL) ;
	strcpy(bptr, pptr) ;
	path   = tmp  ;
	buf[0] = '\0' ;

remove_path:
	/*
	 * Remove manpath entry with %L format code or is inaccessible
	 */
	pptr = path ;
	bptr = buf  ;
	do {
		lptr = (char *)strchr(pptr, ':') ;
		if (lptr != NULL)
			*lptr++ = '\0' ;
		if (access(pptr, F_OK) == -1) {
			pptr = lptr ;
			continue    ;
		}
		strcpy(bptr, pptr)   ;
		bptr += strlen(pptr) ;
		if (lptr != NULL) {
			pptr    = lptr	;
			*bptr++ = ':'	;
		}
	} while (lptr != NULL) ;
	if (bptr[-1] == ':')
		*(--bptr) = '\0' ;
	return(buf) ;
}

main(argc, argv)
        int argc;
        char *argv[];
{


        char *mp;
        progname = argv[0];


        (void) setlocale(LC_ALL, "");

	catd = catopen(MF_MAN, NL_CAT_LOCALE);

        if ((mp = getenv("MANPATH")) != NULL) {			/* 012 */
		manpath_change = (strcmp(mp,manpath) != 0);	/* 012 */
                manpath = expand_path(mp);
	}							/* 012 */
        if ((mp = getenv("PAGER")) != NULL)
                pager = mp;
        umask((mode_t)0);
        mypid = getpid();					/* 011 */
/*  test if command is apropos or whatis and call
        (only in this case) runpath()
*/
        if (strcmp(tail(argv[0]), "apropos") == 0) {
                runpath(argc-1, argv+1, apropos);
                exit(0);
        }
        if (strcmp(tail(argv[0]), "whatis") == 0) {
                runpath(argc-1, argv+1, whatis);
                exit(0);
        }
        if (argc <= 1)
        {
                usage(progname);
                exit(1);
        }
        argc--, argv++;
        while (argc > 0 && argv[0][0] == '-')
        {

               if (!isascii(argv[0][1]))
                {
                    usage(progname);
                    exit(1);
                }
                switch(argv[0][1]) {

                case 0:
                        nomore++;
                        CAT = CAT_S;
                        break;

                case 't':
                        troffit++;
                        break;

                case 'k':
			if (argc < 2 ) { /* -k with no parameter */   /* 008 */
				usage(progname);		      /* 008 */
				exit(1);			      /* 008 */
			}			      		      /* 008 */
                        apropos(argc-1, argv+1);
                        exit(0);

                case 'f':
			if (argc < 2 ) { /* -f with no parameter */   /* 007 */
				usage(progname);		      /* 007 */
				exit(1);			      /* 007 */
			}			      		      /* 007 */
                        whatis(argc-1, argv+1);
                        exit(0);

                case 'P':               /* backwards compatibility */
                case 'M':
                        if (argc < 2) {
                                fprintf(stderr,  MSGSTR(M_MSG_0, "%s: missing path\n") , *argv);
                                usage(progname);
                                exit(1);
                        }
                	mp = manpath;				      /* 012 */
                        if(argv[0][2] == 0) {
                                argc--, argv++;
				manpath = expand_path(*argv);
                        }
                        else manpath = expand_path(*argv + 2);
			manpath_change = (strcmp(mp,manpath) != 0);   /* 012 */
                        break;
                default:
                        usage(progname);
                        exit(1);
                }
                argc--, argv++;
        }

								/* start 015 */
	/* There must be at least one arugment left,		
	 * else <title> or [section] <title> is missing.
	 */

	if (argc == 0) {
		usage(progname);
		exit(1);
	}	
								/* end 015 */

        if (troffit == 0 && nomore == 0 && !isatty(1))
                nomore++;
        section = 0;
	subsec = "";						      /* 001 */
        while (argc > 0) { /* find section on command line, if there is one */
                if (

                isascii(argv[0][0]) &&       /* a section is ASCII only */

                (strchr((void*)ALLSECT, (int)argv[0][0]) &&
                     (argv[0][1] == 0)) ||

                    /* This clause supports sub section names */
                    (isdigit((int)argv[0][0]) &&
		      (atoi(argv[0]) <= 9))) {			      /* 001 */
                        section = argv[0][0];
			subsec = &argv[0][1];			      /* 001 */
                        argc--, argv++;
                        if (argc == 0) {
                                fprintf(stderr,  MSGSTR(M_MSG_1, "Missing section\n") );
                                usage(progname);
                                exit(1);
                        }
                        continue;
                }
                manual(section, argv[0]);
                argc--, argv++;
        }
        exit(0);
}

runpath(ac, av, f)
        int ac;
        char *av[];
        int (*f)();
{

        if (ac > 0 && (strcmp(av[0], "-M") == 0 || strcmp(av[0], "-P") == 0)) {
                if (ac < 2) {
                        fprintf(stderr,  MSGSTR(M_MSG_2, "%s: missing path\n") , av[0]);
                        usage(progname);
                        exit(1);
                }
		manpath_change = (strcmp(av[1],manpath) != 0);        /* 012 */
                manpath = av[1];
                ac -= 2, av += 2;
        }
	if (ac == 0) {
	  usage(progname);
	  exit(1);
	}

        (*f)(ac, av);
        exit(0);
}

manual(sec, name)
        char sec, *name;
{
        char section = sec;

        char work[200], work2[200];
        char cmdbuf[BUFSIZ+1];					/* 004 */
        int  i;

        char path[BUFSIZ+1];					/* 001 */

	char realpath[BUFSIZ+1];				/* 001 */

        struct stat stbuf, stbuf2, stbuf3;
        struct sigaction sigact, osigact;
        int last;
        char *sp = ALLSECT;
        FILE *it;
        char abuf[BUFSIZ];
        char sflag = 0;
        char sbuf[2];


        strcpy(work, "manX/");
        strcat(work, name);
        last = strlen(work);
        if (section == 'C' || section == '1')  /* Search the sections in */
                sp = CSECT;                    /* CSECT for command */
        else if (section != 0) {  /* Section name was provided */
                sbuf[0] = section;
                sbuf[1] = '\0';
                sp = sbuf;
        }
        else sflag = 1;           /* No section given */
        for (section = *sp++; section; section = *sp++)
        { 
                zflag=0;
                work[3] = section;
                work[last] = 0;

                if (pathstat(work, path, &stbuf))
                        break;

                work[last] = '.';
                work[last+1] = section;
                work[last+2] = '\0';

		if ( *subsec != '\0' ) strcat(work, subsec);	      /* 001 */
                if (pathstat(work, path, &stbuf))
                        break;

                /*  The filename may have the <.section> extension and
                    be in compressed format <.z> */
                work[last] = '.';
                work[last+1] = section;
                work[last+2] = '\0';
		if ( *subsec != '\0' ) strcat(work, subsec);	      /* 001 */
		strcat(work, ".z");				      /* 001 */
                zflag++; 
                if (pathstat(work, path, &stbuf))
                       break;

								/* start 001 */
		if ( *subsec != '\0' )
		       continue;

		/* no match was found.  now retry to find any manpage whose
		 * root name is:
		 *	<path>/<name>.<section>
		 */

		work[last+2] = '\0';
		zflag=0;
								      /* 014 */
		strcpy(realname, name); 			/* 001 * 014 */
		strcat(realname, ".");
		realname[strlen(name)+1] = section;		/* 001 * 014 */
		realname[strlen(name)+2] ='\0';			/* 001 * 014 */
								      /* 014 */
		strcpy(realpath, "man");			/* 001 * 014 */
		realpath[3] = section;				/* 001 * 014 */
		realpath[4] = '\0';				/* 001 * 014 */

		if (scanpath(realpath, path,  &stbuf)) {
			work[5]='\0';
			strcat(work, realname);
			path[strlen(path)-5]='\0'; /* chop off the /manX */
			if (strcmp(&work[strlen(work)-2], ".z") == 0)
				zflag++;
			break;
		}
								/* end 001 */
        }

        if (!section)   /* Search failed */
        {
                if(sflag) {
                        fprintf(stderr, MSGSTR(M_MSG_3,
                          "No manual entry found for %s.\n") , name);
			  exit(1);
		 }
                else
                        fprintf(stderr, MSGSTR(M_MSG_4, 
                          "No entry found for %s in section %c of the manual\n") ,
                           name, sec);
                return;
        }

        sprintf(realname, "%s/%s", path, work);         /* Search succeeded */
        if (troffit) {
                troff(path, work);
                return;
        }
        if (!nomore) {
                if ((it = fopen(realname, "r")) == NULL) {
                        goto catit;
                }
                if (fgets(abuf, BUFSIZ-1, it) &&

		   /* if first line of the file starts out with:       * 002 *
		    *		.so man				       * 002 *
		    * or	.soman				       * 002 *
		    */						      /* 002 */
		   (strncmp(abuf, ".so man", 7) == 0 ||		      /* 002 */
		    strncmp(abuf, ".soman", 6) == 0)) {		      /* 002 */
                        register char *cp = abuf+3;		      /* 002 */
                        char *dp;

                        while (*cp && *cp != '\n')
                                cp++;
                        *cp = 0;
                        while (cp > abuf && *--cp != '/')
                                ;

			/* Check if the line is:		       * 002 *
			 *	.so man....			       * 002 *
			 * or	.soman .....			       * 002 *
			 */					      /* 002 */
								      /* 002 */
			if (strncmp(abuf+3, " man", 4) == 0) dp = ".so man";
			else dp = ".soman";			      /* 002 */

                        if (cp != abuf+strlen(dp)+1) {
tohard:
                                fclose(it);
				nomore = 1;
                                strcpy(work, abuf+3);		      /* 002 */
                                goto hardway;
                        }

                        for (cp = abuf; *cp == *dp && *cp;
			     cp+=chrlen(cp), dp+=chrlen(dp))
                                ;
                        if (*dp)
                                goto tohard;
			/*
                        for (i=0, dp = cp; i < 3; i++)
                            if (NCisshift(dp[-2]))
                                dp -= 2;
                            else
                                dp--;
			*/
			/*
			 * Temporary fix - assume ASCII input only
			 */
			dp = cp - 3 ;
                        strcpy(work, dp);
                }
                fclose(it);
        }
catit:
        strcpy(work2, "cat");
        work2[3] = work[3];
        work2[4] = 0;
        sprintf(realname, "%s/%s", path, work2);

        if (stat(realname, &stbuf2) < 0)
                goto hardway;
        strcpy(work2+4, work+4);
        sprintf(realname, "%s/%s", path, work2);

        if (stat(realname, &stbuf2) < 0 || stbuf2.st_mtime < stbuf.st_mtime) {
                if (nomore)
                        goto hardway;

		if ( (stat(NROFFX, &stbuf3) == -1) && (stat(TROFFX, &stbuf3) == -1) )
		{
			fprintf(stderr, MSGSTR(M_MSG_18, "Nroff/troff is not currently installed, this must be \n \tinstalled in order to use formatted man pages.\n"));
			exit(1);
			}
                fprintf(stdout, MSGSTR(M_MSG_5, "Reformatting page.  Wait...") );
                fflush(stdout);
                unlink(work2);
                (void) sigaction(SIGINT, (struct sigaction *)NULL,&osigact);
                if (osigact.sa_handler == SIG_DFL) {
                        sigact.sa_handler = (void (*)(int))local_remove;
                        sigact.sa_flags   = (int)NULL;
                        (void) sigaction(SIGINT, &sigact,(struct sigaction *)NULL);
                        (void) sigaction(SIGQUIT, &sigact,(struct sigaction *)NULL);
                        (void) sigaction(SIGTERM, &sigact,(struct sigaction *)NULL);
                }
								      /* 004 */
#ifdef	SETPATH
                sprintf(cmdbuf, "%s ; tbl %s/%s | neqn | %s > /tmp/man%d",
                        SETPATH, path, work, NROFFCAT, mypid);
#else	/* SETPATH */
                sprintf(cmdbuf, "tbl %s/%s | neqn | %s > /tmp/man%d", /* 010 */
                        path, work, NROFFCAT, mypid);		      /* 004 */
#endif	/* SETPATH */

                if (system(cmdbuf)) {
                        fprintf(stdout, MSGSTR(M_MSG_6, " aborted (sorry)\n") );
                        local_remove();
                        /*NOTREACHED*/
                }
                sprintf(cmdbuf, "/bin/mv -f /tmp/man%d %s/%s 2>/dev/null",
                        mypid, path, work2);

                if (system(cmdbuf)) {
                        sprintf(path,  "/");
                        sprintf(work2, "tmp/man%d", mypid);
                }
                fprintf(stdout, MSGSTR(M_MSG_7, " finished\n") );
        }
        strcpy(work, work2);
hardway:
        nroff(path, work);

        if (work2[0] == 't')
                local_remove();

}

/*
 * Use the manpath to look for
 * the file name.  The result of
 * stat is returned in stbuf, the
 * successful path in path.
 */
pathstat(name, path, stbuf)
        char *name, path[];
        struct stat *stbuf;
{
        char *cp, *tp, *ep;
        char **cpp;
        static char *manpaths[] = {"man", "cat", 0};
        static char *nopaths[]  = {"", 0};

        if (strncmp(name, "man", (size_t)3) == 0)
                cpp = manpaths;
        else
                cpp = nopaths;
        for ( ; *cpp ; cpp++) {
                for (cp = manpath; cp && *cp; cp = tp) {
                        tp = (void*)strchr((void*)cp, ':');
                        if (tp) {
                                if (tp == cp) {
                                        sprintf(path, "%s%s", *cpp,
                                                name+strlen(*cpp));
                                }
                                else {
                                        sprintf(path, "%.*s/%s%s", tp-cp, cp,
                                                *cpp, name+strlen(*cpp));
                                }
                                ep = path + (tp-cp);
                                tp++;
                        } else {
                                sprintf(path, "%s/%s%s", cp, *cpp,
                                        name+strlen(*cpp));
                                ep = path + strlen(cp);
                        }
                        if (stat(path, stbuf) >= 0) {
                                *ep = '\0';

                                return (1);
                        }
                }
        }
        return (0);
}


/*
 * Use the manpath to look for
 * the root manpage name and root pathname. The result of
 * stat is returned in stbuf, the
 * successful root pathname in path, the first-match found manpage
 * realname[], which is external.
 */
scanpath(name, path, stbuf)
        char *name, path[];
        struct stat *stbuf;
{
        char *cp, *tp, *ep;
        char **cpp;
        static char *manpaths[] = {"man", "cat", 0};
        static char *nopaths[]  = {"", 0};

	int mandir_count;
	struct dirent **mandir_entries;
	int select_manpage(), alphasort();
	char fullpath[BUFSIZ+1];

        if (strncmp(name, "man", (size_t)3) == 0)
                cpp = manpaths;
        else
                cpp = nopaths;

        for ( ; *cpp ; cpp++) {
                for (cp = manpath; cp && *cp; cp = tp) {
                        tp = (void*)strchr((void*)cp, ':');
                        if (tp) {
                                if (tp == cp) {
                                        sprintf(path, "%s%s", *cpp,
                                                name+strlen(*cpp));
                                }
                                else {
                                        sprintf(path, "%.*s/%s%s", tp-cp, cp,
                                                *cpp, name+strlen(*cpp));
                                }
                                ep = path + (tp-cp);
                                tp++;
                        } else {
                                sprintf(path, "%s/%s%s", cp, *cpp,
					name+strlen(*cpp));
                                ep = path + strlen(cp);
                        }
                        if (stat(path, stbuf) >= 0) {
				mandir_count = 0;
				mandir_count = scandir(path, &mandir_entries, select_manpage, alphasort);
				if (mandir_count == -1) {
					fprintf(stderr,  MSGSTR(M_MSG_19,
					  "man: cannot read directory %s.\n"), path);
					perror("man");
			
					continue;
				}
		
				if (mandir_count > 0) {
					strcpy(realname, mandir_entries[0]->d_name);
					fullpath[0]='\0';
					strcpy(fullpath, path);
					strcat(fullpath, "/");
					strcat(fullpath, realname);
					if (stat(fullpath, stbuf) < 0)
						continue;

					return(1);
				}
                        }
                }
        }
        return (0);
}


/* select man directory records */
select_manpage(dirp)
	struct dirent *dirp;
{
	int len;

	len=strlen(realname);
	if (dirp->d_namlen >= len &&
	     !strncmp(dirp->d_name, realname, (size_t) len))
		return (1); /* found a match */

	return (0); 
}


nroff(pp, wp)
        char *pp, *wp;
{
        char cmdbuff[BUFSIZ];

        chdir(pp);

        if (wp[0] == 'c' || wp[0] == 't')
	        {
                     if(zflag)  /* file is compressed with .z*/
            	     {
                           wp[strlen(wp)-2]='\0';
                           nomore++;
                           sprintf(cmdbuff, "%s %s|%s", PCAT, wp, pager);
                           (void) system(cmdbuff);
                           return; 
                     }
                sprintf(cmdbuff, "%s %s", nomore? CAT : pager, wp);
	       }
        else 
								      /* 004 */
                sprintf(cmdbuff, nomore? "tbl %s | neqn | %s" : "tbl %s | neqn | %s | %s", wp, NROFF, pager);
        (void) system(cmdbuff);
        return;
}

troff(pp, wp)
        char *pp, *wp;
{
        char cmdbuf[BUFSIZ];

        chdir(pp);
        sprintf(cmdbuf, TROFFCMD, wp);
						
        (void) system(cmdbuf);
}

any(c, sp)
        register int c;
        register char *sp;
{
        register int d;

        if (!isascii(c))
                return(0);

        while (d = *sp++)
                if (c == d)
                        return (1);
        return (0);
}

local_remove(void)
{
        char name[15];

        sprintf(name, "/tmp/man%d", mypid);
        unlink(name);
        exit(1);
}

unsigned int
blklen(ip)
        register char **ip;
{
        register unsigned int i = 0;

        while (*ip++)
                i++;
        return (i);
}

apropos(argc, argv)
        int argc;
        char **argv;
{
        char buf[BUFSIZ], file[BUFSIZ+1];
        char *gotit, *cp, *tp;
        register char **vp;
	FILE *fp;

        if (argc == 0) {
                fprintf(stderr,  MSGSTR(M_MSG_8, "Missing argument\n") );
                usage2("apropos");
                exit(1);
        }
        gotit = calloc(1, blklen(argv));
        /*      First Check the "whatis" file           */
        for (cp = manpath; cp; cp = tp) {
                tp = (void*)strchr((void*)cp, ':');
                if (tp) {
                        if (tp == cp)
                                strcpy(file, WHATIS);
                        else
                                sprintf(file, "%.*s/%s", tp-cp, cp, WHATIS);
                        tp++;
                } else
                        sprintf(file, "%s/%s", cp, WHATIS);
                if ((fp = fopen(file, "r")) != NULL) {
			whatis_found++;		/* found one */       /* 013 */
                	while (fgets(buf, (int)(sizeof buf), fp) != NULL)
                        	for (vp = argv; *vp; vp++)
                               		if (match(buf, *vp)) {
                                        	printf("%s", buf);
                                        	gotit[vp - argv] = 1;
                                      	  	for (vp++; *vp; vp++)
                                               		if (match(buf, *vp))
                                                        	gotit[vp - argv] = 1;
                                        	break;
                                	}
			fclose(fp);
        	}
	}
								/* start 013 */
	if (!whatis_found) {	/* no "whatis" database exists in any
				 * manpath directory.
				 */
		fprintf(stderr, MSGSTR(NO_WHATIS_1,
			"%s: No whatis database found in <%s>.\n"),
			progname, manpath);
		if (!manpath_change)
			fprintf(stderr, MSGSTR(NO_WHATIS_2,
				"\tRun:\tcatman -w\n\tor:\tcatman -M %s -w\n"),
				manpath);
		else
			fprintf(stderr, MSGSTR(NO_WHATIS_3,
				"\tRun:\tcatman -M %s -w\n"), manpath);
		exit(2);
	}							  /* end 013 */
		
	for (vp = argv; *vp; vp++)
		if (gotit[vp - argv] == 0) {
			fprintf(stdout, MSGSTR(M_MSG_9, "%s: nothing appropriate\n") , *vp);
			exit(1);
		}
	}

match(bp, str)
	register char *bp;
	char *str;
{

	for (;;) {
		if (*bp == 0)
			return (0);
		if (amatch(bp, str))
			return (1);

		bp += chrlen(bp);
	}
}

amatch(cp, dp)
	register char *cp, *dp;
{

	while (*cp && *dp && lmatch_wc(cp, dp))
		dp += chrlen(dp), cp += chrlen(cp);
	if (*dp == 0)
		return (1);
	return (0);
}


lmatch_wc(c, d)
register char    *c, *d;
{
    wchar_t	 wc1 , wc2  ;
    register int len1, len2 ;

    len1 = mbtowc(&wc1, c, MB_LEN_MAX) ;
    len2 = mbtowc(&wc2, d, MB_LEN_MAX) ;

    if (len1 != len2)
	return(0) ;
    return(lmatch(wc1, wc2)) ;
}

lmatch(c, d)
	register int c, d;
{

	if (c == d)
		return (1);
	if (!isalpha(c) || !isalpha(d))
		return (0);
	if (islower(c))
		c = _toupper(c);
	if (islower(d))
		d = _toupper(d);
	return (c == d);
}

whatis(argc, argv)
	int argc;
	char **argv;
{
	register char *gotit, **vp;
	char buf[BUFSIZ], file[BUFSIZ+1], *cp, *tp;
	FILE *fp;

	if (argc == 0) {
		fprintf(stderr,  MSGSTR(M_MSG_10, "Missing argument\n") );
		usage2("whatis");
		exit(1);
	}

	for (vp = argv; *vp; vp++)
		*vp = trim(*vp);
	gotit = calloc(1, blklen(argv));
	for (cp = manpath; cp; cp = tp)
	{
		tp = (void*)strchr((void*)cp, ':');
		if (tp)
		{
			if (tp == cp)
				strcpy(file, WHATIS);
			else
				sprintf(file, "%.*s/%s", tp-cp, cp, WHATIS);
			tp++;
		} else
			sprintf(file, "%s/%s", cp, WHATIS);
		if ((fp = fopen(file, "r" )) != NULL) {
			whatis_found++;		/* found one */       /* 012 */
			while (fgets(buf, (int)sizeof buf, fp) != NULL)
				for (vp = argv; *vp; vp++)
					if (wmatch(buf, *vp)) {
						printf("%s", buf);
						gotit[vp - argv] = 1;
						for (vp++; *vp; vp++)
                               		                if (wmatch(buf, *vp))
                               		                        gotit[vp - argv] = 1;
                               		        break;
                               		}
			fclose(fp);
		}
 	}

								/* start 012 */
	if (!whatis_found) {	/* no "whatis" database exists in any
				 * manpath directory.
				 */
		fprintf(stderr, MSGSTR(NO_WHATIS_1,
			"%s: No whatis database found in <%s>.\n"),
			progname, manpath);
		if (!manpath_change)
			fprintf(stderr, MSGSTR(NO_WHATIS_2,
				"\tRun:\tcatman -w\n\tor:\tcatman -M %s -w\n"),
				manpath);
		else
			fprintf(stderr, MSGSTR(NO_WHATIS_3,
				"\tRun:\tcatman -M %s -w\n"), manpath);
		exit(2);
	}							  /* end 012 */

        for (vp = argv; *vp; vp++)
                if (gotit[vp - argv] == 0) {
								      /* 009 */
                        fprintf(stdout, MSGSTR(M_MSG_11, "%s: not found\n") , *vp);
			exit(1);
			}
}

wmatch(buf, str)
        char *buf, *str;
{
        register char *bp, *cp;

        bp = buf;
again:
        cp = str;

        while (*bp && *cp && lmatch_wc(bp, cp))
                bp += chrlen(bp), cp += chrlen(cp);

        if (*cp == 0 && (*bp == '(' || *bp == ',' || *bp == '\t' || *bp == ' '))
                return (1);

        while (isalpha((int)*bp) || isdigit((int)*bp))
                bp += chrlen(bp);
        if (*bp != ',')
                return (0);
        bp += chrlen(bp);
        while (isspace((int)*bp))
                bp += chrlen(bp);
        goto again;
}

char *
trim(cp)
        register char *cp;
{
        register char *dp;

        for (dp = cp; *dp; dp++)
                if (*dp == '/')
                        cp = dp + 1;
        if (cp[0] != '.') {
                if (cp + 3 <= dp && dp[-2] == '.' &&
                    any(dp[-1], "cosa12345678npP"))
                        dp[-2] = 0;
                if (cp + 4 <= dp && dp[-3] == '.' &&
                    any(dp[-2], "13") && isalpha((int)dp[-1]))
                        dp[-3] = 0;
        }
        return (cp);
}

char *
tail(s)
        char *s;
{
        char *suffix;

                /* find the last '/' if there is one */ ;
        suffix = (void*)strrchr((void*)s,'/');
        if(suffix!=NULL)
                return(suffix+1);
        else
                return(s);

}

usage(progname)
char *progname;
{
							 /* 007 */
        fprintf(stderr,  MSGSTR(M_MSG_12, "Usage: %s [-] [-M path] ") , progname);
        fprintf(stderr,  MSGSTR(M_MSG_13, "[section] name ...\n") , progname);
        fprintf(stderr,  MSGSTR(M_MSG_14, "  %s -k keyword ... \n") , progname);
        fprintf(stderr,  MSGSTR(M_MSG_15, "  %s -f file ... \n") , progname);
}
usage2(progname)
char *progname;
{
        fprintf(stderr,  MSGSTR(M_MSG_16, "Usage: %s ") , progname);
        fprintf(stderr,  MSGSTR(M_MSG_17, "[-M path] keyword ...\n") , progname);
}
