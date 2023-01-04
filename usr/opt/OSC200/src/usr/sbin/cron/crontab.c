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
static char rcsid[] = "@(#)$RCSfile: crontab.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/12/21 19:14:39 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: crontab
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * crontab.c   1.19  com/cmd/cntl/cron bos320, 9125320 6/7/91 07:58:13
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>

extern priv_t *privvec();
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <locale.h>
#include <time.h>
#include "cron.h"
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "cron_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
nl_catd catd;
#ifdef SEC_BASE
#define MSGSTR_SEC(Num,Str) catgets(catd,MS_CRON_SEC,Num,Str)
#define	TMPDIR		"/tmp"
#endif


#define TMPFILE		"_cron"		/* prefix for tmp file */
#define CRMODE		0400	/* mode for creating crontabs */

#define DEFAULT_EDITOR	"vi"	/* default editor for -e option */

#define BADCREATE	"cannot create your crontab file in the crontab directory."
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)."
#define NOTALLOWED	"you are not authorized to use cron.  Sorry."
#define EOLN		"unexpected end of line."
#define UNEXPECT	"unexpected character found in line."
#define OUTOFBOUND	"number out of bounds."
#define BADOPEN		"cannot open your crontab file."
#define BADPARM		"%s: specify only one flag or file name\n"
#define NOBLANK		"Blank lines are not allowed.\n"
#define NOACCESS	"File does not exist, or you have no read permission."
#if SEC_BASE
#define INSUFFPRIV      "%s: insufficient privileges\n"
#endif
#define RFLAG		0x00000001
#define LFLAG		0x00000002
#define VFLAG		0x00000004
#define EFLAG		0x00000008

int err,cursor;
char *cf,*tnam,line[CTLINESIZE];
extern char *xmalloc();
char login[UNAMESIZE];

extern int list_cj();
extern int remove_cj();

extern struct passwd *getpwuid(uid_t);
extern uid_t getuid(void);

int usage(void);
extern int send_msg( char, char, char *, char *);
void crabort( char *);
void cerror( char *);
void catch(int);
int next_field( int, int);
void copycron( FILE *);
void editcron(void);

/*
 * NAME: crontab
 *                                                                    
 * FUNCTION:  crontab: Submits a schedule of commands to cron.
 *	description:	This program implements crontab (see cron(1)).
 *			This program should be set-uid to root.
 *	files:
 *		/var/adm/cron 			drwxr-xr-x root sys
 *		/var/adm/cron/cron.allow 	-rw-r--r-- root sys
 *		/var/adm/cron/cron.deny 	-rw-r--r-- root sys
 */  
main(int argc, char **argv)
{
	char *pp;
	FILE *fp;
	struct passwd *nptr;
	int funct=0;
	int flag,rstat;
	uid_t luid;
	char *s;
	int num;

	(void ) setlocale(LC_ALL,"");

	catd = catopen(MF_CRON,NL_CAT_LOCALE);

#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();
#if SEC_MAC
        disablepriv(SEC_MULTILEVELDIR);
#endif
        if (forceprivs(privvec(
			        SEC_CHOWN,
#if SEC_MAC
                                SEC_WRITEUPSYSHI,
#endif
                                -1), (priv_t *) 0)) {
                fprintf(stderr, MSGSTR(MS_INSUFFPRIV, INSUFFPRIV), "crontab");
                exit(1);
        }
#endif /* SEC_BASE */


#if SEC_BASE

	/* Accountability -- don't allow user to submit a job as somebody else */
	if ((luid = getluid()) != geteuid())
		crabort(MSGSTR_SEC(CRONTAB_SEC_7,
			       "Login UID does not match effective UID.\n"));
        if ((nptr=getpwuid(luid)) == NULL) /* check the user id */
#else
	if ((nptr=getpwuid(getuid())) == NULL) 	/* check the user id */
#endif 
		crabort(MSGSTR(MS_INVALIDUSER, INVALIDUSER));
	else
		pp = nptr->pw_name;		/* user's crontab file */

	strcpy(login,pp);			/* save login name */

	/*
	 * determine function: list, verbose list, remove or add
	 */

	num = 0;				/* number of options */
	while ((flag = getopt (argc, argv, "rlve")) != EOF) {
		switch (flag) {
			case 'r':
				funct = RFLAG;
				break;
			case 'l':
				funct = LFLAG;
				break;
			case 'v':
				funct = VFLAG;
				break;
			case 'e':
				funct = EFLAG;
				break;
			default:
				usage ();
		}
		num++;
	}
	argc -= optind;
	argv += optind;
		
	/*
	 * only 1 option or file allowed
	 */

	if (num + argc > 1) {
		fprintf(stderr, MSGSTR(MS_BADPARM, BADPARM),"crontab");
		usage();
	}

	/*
	 * check the .permit and .deny files (part of SVID)
	 */

	if (!allowed(login,CRONALLOW,CRONDENY)) 
		crabort(MSGSTR(MS_CRNOTALLOWED, NOTALLOWED));

	switch(funct) {
		case VFLAG:     		/* vebose listing */
			list_cj(CRON_NON_VERBOSE,pp);
			break;

		case LFLAG:     		/* list */
			list_cj(CRON_VERBOSE,pp);
			break;

		case RFLAG:     		/* remove */
			remove_cj(CRON_QUIET,pp);
			break;

		case EFLAG:     		/* edit */
			editcron();
			rstat = send_msg(CRON, ADD, login, pp);
			if (rstat) exit(1);
			break;
		default:			/* add */
			if (argc==0) 
				copycron(stdin);
			else {			/* open the crontab file */
                if (access(argv[0], R_OK) != 0) 
		           crabort(MSGSTR(MS_NOACCESS, NOACCESS));
				if ((fp=fopen(argv[0],"r"))==NULL) {
					perror(argv[0]);
					exit(1);
		     		} else 
					copycron(fp);
			}
			rstat = send_msg(CRON, ADD,login,pp);
			if (rstat) exit(1);
	}
	exit(0);
}

/*
 * NAME: copycron
 *
 * FUNCTION: copy cron event to cron directory from the file(fp)
 *
 * EXECUTION ENVIRONMENT:
 *      This function overrides the contents of the old crontab file.
 *      If there is no crontab file then it will be created.
 * 	The crontab file name is the login name.
 *
 * (NOTES:)  This function uses a temporary file.
 *
 * RETURNS:  none
 */

void
copycron(FILE *fp)
{
	FILE *tfp;
	char pid[6];
	int t;
	int line_num=0;
#if	SEC_BASE
	struct stat statbuf;
#endif

	/*
	 * create the new crontab file name
	 */

	cf = xmalloc(strlen(CRONDIR)+strlen(login)+2);
	sprintf(cf,"%s/%s",CRONDIR,login);

	sprintf(pid,"%-5d",getpid());
	tnam=xmalloc(strlen(CRONDIR)+strlen(TMPFILE)+7);
	sprintf(tnam,"%s/%s%s",CRONDIR,TMPFILE,pid);
	/* catch SIGINT, SIGHUP, SIGQUIT signals */
	if (signal(SIGINT,catch) == SIG_IGN) 
		signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP,catch) == SIG_IGN) 
		signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,catch) == SIG_IGN) 
		signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,catch) == SIG_IGN) 
		signal(SIGTERM,SIG_IGN);
#if SEC_BASE
        if ((t = crontab_secure_create(tnam)) == -1)
#else
	if ((t=creat(tnam,(mode_t)CRMODE))==-1) 
#endif
		crabort(MSGSTR(MS_BADCREATE, BADCREATE));
	if ((tfp=fdopen(t,"w"))==NULL) {
		unlink(tnam);
		crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
	}
	
	err=0;	/* if errors found, err set to 1 */
	while (fgets(line,CTLINESIZE,fp) != NULL) {
		cursor=0;
		line_num++;
		while(isspace(line[cursor]) || line[cursor] == '\t')
			cursor++;
/*
		if ((line[cursor] == '\n') || (line[cursor] == '\0'))
			crabort(MSGSTR(MS_NOBLANK, NOBLANK));
*/
		if ((line[cursor] == '\n') || (line[cursor] == '\0'))
			continue;

		if (strchr(line,' ') == NULL) 
			goto cont;
		if (line[cursor] == '#')
			goto cont;
		if (next_field(0,59)) continue;
		if (next_field(0,23)) continue;
		if (next_field(1,31)) continue;
		if (next_field(1,12)) continue;
		if (next_field(0,06)) continue;
		if (line[++cursor] == '\0') {
			cerror(MSGSTR(MS_EOLN, EOLN));
			continue; 
		}
cont:
		if (fputs(line,tfp) == EOF) {
			unlink(tnam);
			crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
		}
	}
	fclose(fp);
	fclose(tfp);
	if (!err) {
#if	SEC_MAC
		/*
		 * cron daemon wants to put output file in multilevel child
		 * dir of /tmp corresponding to our current sensitivity
		 * label.  Life will be easier if this directory exists,
		 * so try to ensure its existence by referencing it.
		 */
		(void) stat(TMPDIR, &statbuf);
#endif
		/* make file tfp the new crontab */
		unlink(cf);
		if (link(tnam,cf)==-1) {
			unlink(tnam);
			crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
		} 
	}
	unlink(tnam);
}

/*
 * NAME: next_field
 *                                                                    
 * FUNCTION:  check the next field for invalid input
 *
 * RETURNS:  0 - success, 1 - failure
 */  
int
next_field(int lower, int upper)
{
	int num,num2;

	while ((line[cursor]==' ') || (line[cursor]=='\t')) cursor++;
	if (line[cursor] == '\0') {
		cerror(MSGSTR(MS_EOLN, EOLN));
		return(1); 
	}
	if (line[cursor] == '*') {
		cursor++;
		if ((line[cursor]!=' ') && (line[cursor]!='\t')) {
			cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
			return(1); 
		}
		return(0); 
	}
	while (TRUE) {
		if (!isdigit(line[cursor])) {
			cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
			return(1); 
		}
		num = 0;
		do { 
			num = num*10 + (line[cursor]-'0'); 
			++cursor;	/* for KANJI */
		} while (isdigit(line[cursor]));
		if ((num<lower) || (num>upper)) {
			cerror(MSGSTR(MS_OUTOFBOUND, OUTOFBOUND));
			return(1); 
		}
		if (line[cursor]=='-') {
			++cursor;	/* for KANJI */
			if (!isdigit(line[cursor])) {
				cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
				return(1); 
			}
			num2 = 0;
			do { 
				num2 = num2*10 + (line[cursor]-'0'); 
				++cursor;	/* for KANJI */
			} while (isdigit(line[cursor]));
			if ((num2<lower) || (num2>upper)) {
				cerror(MSGSTR(MS_OUTOFBOUND, OUTOFBOUND));
				return(1); 
			}
		}
		if ((line[cursor]==' ') || (line[cursor]=='\t')) break;
		if (line[cursor]=='\0') {
			cerror(MSGSTR(MS_EOLN, EOLN));
			return(1); 
		}
		if (line[cursor++]!=',') {
			cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
			return(1); 
		}
	}
	return(0);
}


/*
 * NAME: cerror
 *                                                                    
 * FUNCTION:  print out error message for cron event
 *
 * RETURNS:  set err 1.
 */  
void
cerror( char *msg)
{
	char message[256];
	strcpy(message,msg);	/* save message */
	fprintf(stderr,MSGSTR(MS_LINERR, "%scrontab: error on previous line; %s\n"),line,message);
	err=1;
}


/*
 * NAME: catch
 *                                                                    
 * FUNCTION: unlink if crontab object is invalid
 */  
void
catch(int sig)
{
	unlink(tnam);
	exit(1);
}


/*
 * NAME: crabort
 *                                                                    
 * FUNCTION:  print error message if crontab failed
 */  
void
crabort( char *msg)
{
	fprintf(stderr,MSGSTR(MS_CRABORT, "crontab: %s\n"),msg);
	exit(1);
}


/*
 * NAME: usage
 *                                                                    
 * FUNCTION: print usage message and exit
 *                                                                    
 * EXECUTION ENVIRONMENT: user process
 *                                                                   
 * RETURNS: does not return exit(1)
 */  

int
usage(void)
{
	fprintf(stderr, MSGSTR(MS_CRBADUSAGE, 
	"Usage: crontab [-l|-r|-v|-e|File]\n"));
	exit(1);
}



/*
 * NAME: editcron
 *
 * FUNCTION: edit cron event in the file under cron directory
 *
 * EXECUTION ENVIRONMENT:
 *      This function overrides the contents of the old crontab file.
 *      If there is no crontab file then it will be created.
 * 	The crontab file name is the login name.
 *
 * (NOTES:)  This function uses a temporary file.
 *
 *	Get a temp file name.
 *	If we had an old crontab file, copy it in to our temp file.
 *	Invoke the editor on the tempfile.
 *	Read the tempfile and parse it into a tempfile in the cron directory.
 *	If the parsing went well, link it in as the new crontab file.
 *
 * RETURNS:  none
 */

void
editcron()
{
        FILE *tfp, *tfp2, *fp;
        char pid[6];
        char *edit,*s;
	char *tnam2;
        int wpid,fid,st;
	int j,t,line_num=0;
	int i;
        struct stat fst;
        /*
         * create the crontab file  and temporary file names
         */
        cf = xmalloc(strlen(CRONDIR)+strlen(login)+2);
        sprintf(cf,"%s/%s",CRONDIR,login);

	tnam = tmpnam(NULL);

	/* catch SIGINT, SIGHUP, SIGQUIT signals */
	if (signal(SIGINT,catch) == SIG_IGN) 
		signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP,catch) == SIG_IGN) 
		signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,catch) == SIG_IGN) 
		signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,catch) == SIG_IGN) 
		signal(SIGTERM,SIG_IGN);

        if ((tfp=fopen(tnam,"w"))== NULL) {
                unlink(tnam); 
		free(cf);
       		crabort(MSGSTR(MS_BADCREATE, BADCREATE));
	}

	/* if the crontab exist? then open for read */
        if ((fid = stat(cf,&fst)) != -1) { 
        	if ((fp=fopen(cf,"r"))==NULL) {
			unlink(tnam);
			free(cf);
       			crabort(MSGSTR(MS_BADOPEN, BADOPEN));
		}
		/* copy the contents */
        	while (fgets(line,CTLINESIZE,fp) != NULL) 
                	if (fputs(line,tfp) == EOF ) {
        			fclose(fp);
        			fclose(tfp);
				unlink(tnam);
				free(cf);
        			crabort(MSGSTR(MS_BADCREATE, BADCREATE));
			}
        	fclose(fp);
        }
        fclose(tfp);

	/* Edit the temp file */
        wpid = fork();
        if (wpid <0) { 
		unlink(tnam);
		free(cf);
                crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
        }
        if (wpid == 0) { /* child process */

		/* give the file to the user */
		chmod(tnam, S_IRUSR|S_IWUSR);
		chown(tnam, getuid(), getgid());

                edit = getenv("EDITOR");
                if (edit == NULL )
                	edit= DEFAULT_EDITOR;

		/* give up privileges */
		setgid(getgid());	/* for security and just in case */
		setuid(getuid());

		/* Reset all signals that we tend to modify to their defaults so */
		/* that the editor does not inherit our SIG_IGN state for them. */
		signal(SIGALRM, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
                execlp(edit, edit, tnam, 0);
                perror(edit);
                exit(1);
        }
	/* Let these signals go through to editor for now */
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
        if ((wait(&st) != wpid) || (st)) {
		unlink(tnam);
		free(cf);
                crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
        } 
	/* Reset these signals to catch. */
	signal(SIGINT, catch);
	signal(SIGHUP, catch);
	signal(SIGQUIT, catch);
	signal(SIGTERM, catch);

	/* Copy the tempfile in to a tempfile in the cron directory */
	/* We parse it as we go along for errors.  */

        if ((tfp=fopen(tnam,"r"))==NULL) {
		unlink(tnam);
		free(cf);
                crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
	}

        sprintf(pid,"%-5d\0",getpid());
        tnam2=xmalloc(strlen(CRONDIR)+strlen(TMPFILE)+7);
        sprintf(tnam2,"%s/%s%s\0",CRONDIR,TMPFILE,pid);
        for (j=strlen(tnam2)-1 ; j > 0 ; j--) {
                if (tnam2[j] == ' ') tnam2[j] = '\0';
                else break;
        }
#if SEC_BASE
        if ((t = crontab_secure_create(tnam2)) == -1)
#else
	if ((t = creat(tnam2,CRMODE))== -1) 
#endif
        	crabort(MSGSTR(MS_BADCREATE, BADCREATE));

	if ((tfp2=fopen(tnam2,"w"))== NULL) {
		unlink(tnam2);
		free(tnam2);
		free(cf);
		crabort(MSGSTR(MS_BADCREATE, BADCREATE));
	}

	/* At this point:
	 *		tfp = /tmp/foo  (source)
	 * 		tfp2 = /var/adm/cron/crontabs/_cron12345 (dest)
	 *
	 * Parse tfp in to tfp2.
	 */

        err=0;  /* if errors found, err set to 1 */
	while (fgets(line,CTLINESIZE,tfp) != NULL) {
		cursor=0;
		line_num++;
		while(isspace(line[cursor]) || line[cursor] == '\t')
			cursor++;
/*
		if ((line[cursor] == '\n') || (line[cursor] == '\0')) {
			unlink(tnam);
			crabort(MSGSTR(MS_NOBLANK, NOBLANK));
		}
*/
		if ((line[cursor] == '\n') || (line[cursor] == '\0'))
			continue;
		if (strchr(line,' ') == NULL) 
			goto cont;
		if (line[cursor] == '#')
			goto cont;
		if (next_field(0,59)) continue;
		if (next_field(0,23)) continue;
		if (next_field(1,31)) continue;
		if (next_field(1,12)) continue;
		if (next_field(0,06)) continue;
		if (line[++cursor] == '\0') {
			cerror(MSGSTR(MS_EOLN, EOLN));
			continue; 
		}
cont:
		if (fputs(line,tfp2) == EOF) {
			unlink(tnam2);
			crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
		}
	}
	fclose(tfp2);
        fclose(tfp);
	unlink(tnam);
        if (!err) {
                /* make file tfp2 the new crontab */
                unlink(cf);
                if (link(tnam2,cf)==-1) {
                        unlink(tnam2);
			free(tnam2);
			free(cf);
                        crabort(MSGSTR(MS_BADCREATE, BADCREATE)); 
		}
        } else {
		unlink(tnam2);
		free(cf);
		free(tnam2);
		exit(1);
	}
        unlink(tnam2);
	free(tnam2);
	free(cf);
}
