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
static char rcsid[] = "@(#)$RCSfile: who.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/10/11 19:54:24 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
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
 * who.c        1.21  com/cmd/stat,3.1,9013 3/1/90 17:27:46
 */

#include <stdlib.h> 
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include <locale.h>

#include <time.h>
#include <ctype.h>
#include <langinfo.h>

#include "who_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_WHO, Num, Str)

#define FAILURE -1
#define MAXLINE 100
#define FLAGS   UTMAXTYPE
#define SIZEUSER	8	/* size of struct utmp->char ut_user[] */
#define SIZELINE	12	/* size of struct utmp->char ut_line[] */

	/* --- Width of fields (in the output/without spacing) --- */
#define	LOGF_SIZE	 9	/* user name 	: Name */
#define	TTYS_SIZE	 1	/* tty status	: ST */
#define	LINE_SIZE	11	/* tty name	: Line */
#define	TIME_SIZE	14	/* login time	: Time */
#define	IDLE_SIZE	 6	/* Idle time	: Activity */
#define	PID_SIZE	 9	/* user's PID	: PID */
#define	HOST_SIZE	17	/* workstation	: Hostname or Hostname/Exit */
#define	ID_SIZE 	 7	/* id of /Exit  : (id=xxxx) */

	/* --- Width of fields (in the Header) --- */
#define	TTYS_HEAD	 2	/* Due to the fact that the title "ST" and  */
#define	TIME_HEAD	11	/* "Activity" are so long that it is eating */
#define	IDLE_HEAD	 8	/* its previous field.			    */


extern void utmpname(char *newfile);
extern struct utmp *getutent(void);
int usage(char *format, char *arg1);
char *user(struct utmp *utmp);
char *host(struct utmp *utmp);
char *line(struct utmp *utmp);
static void copypad(char *to, char *from, size_t size);
char *id(struct utmp *utmp);
char *ltime(struct utmp *utmp);
char *etime(struct utmp *utmp);
char *mode(struct utmp *utmp);
int alarmclk(void);
char *loc(struct utmp *utmp);


char username[SIZEUSER + 1];

		/* values for 'shortflag' */
#define	S_DFLT	 0	/* no options related to '-s' */
#define	S_SHORT	 1	/* short: suppress Activity and PID (and "/Exit") */
#define	S_LONG	-1	/* long:  display them */

int shortflag;          /* Set when fast form of "who" is to run. */
int ttystatus;          /* Set when write to tty status desired.  */
int allflag;            /* Set when all entries are to be dumped. */
int header=0;		/* Print a header                         */
int quickly=0;		/* Just print the number of people and their names */
int totaluser=0;	/* Count how many users logged in.        */

/*
 * locale's short month date format
 */
static char rec_form_posix[] = "%b %d %H:%M";
static char *loc_rec_form;

main(int argc, char **argv)
{
	static int entries[FLAGS+1];    /* Flag for each type entry */
	register struct utmp *utmp;
	register int i;
	char *ptr;
	struct passwd *passwd;
	char outbuf[BUFSIZ];
	extern int shortflag,ttystatus,allflag;
	int foundone = 0;               /* found a matching utmp entry */
	extern int optind;	/* index to the next arg, set by getopt() */
	int	optlet;		/* an option letter found in getopt() */
	int	am_i, sflag, host_machine;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_WHO, NL_CAT_LOCALE);

	loc_rec_form = nl_langinfo(_M_D_RECENT);
	if (loc_rec_form == NULL || !*loc_rec_form)
		loc_rec_form = rec_form_posix;

/* Make "stdout" buffered to speed things up. */

	setbuf(stdout, outbuf);

/* Look up our own name for reference purposes. */

	if((passwd = getpwuid(getuid())) == (struct passwd *)NULL) {
		 username[0] = '\0';
	} else {
/* Copy the user's name to the array "username". */
		 strncpy(&username[0],passwd->pw_name,(size_t)SIZEUSER);
		 username[SIZEUSER] = '\0';
	}

/* Set "ptr" to null pointer so that in normal case of "who" */
/* the "ut_line" field will not be looked at.  "ptr" is used */
/* only for "who am i" or "who -m" command. */

	ptr = NULL;
	host_machine = 0;
	am_i = 0;
	sflag = 0;
	shortflag = S_DFLT;	/* not set yet */
	ttystatus = FALSE;

/* Analyze the switches and set the flags for each type of entry */
/* that the user requests to be printed out. */

	while ( (optlet = getopt(argc,argv,"AabdHhlMmpqrsTtu")) != EOF ) {
		switch ( optlet ) {
			case 'A' :
				entries[ACCOUNTING] = TRUE;
				break;
			case 'a' :
				for(i = 1; i < FLAGS; i++)
					entries[i] = TRUE;
				shortflag = S_LONG;
				ttystatus = TRUE;
				host_machine++;
				header++;
				allflag = TRUE;
				break;
			case 'b' :
				entries[BOOT_TIME] = TRUE;
				break;
			case 'd' :
				entries[DEAD_PROCESS] = TRUE;
				shortflag = S_LONG;
				break;
			case 'H' :
				header++;
				break;
			case '?' :
			case 'h' :
				usage("", "");
				break;
			case 'l' :
				entries[LOGIN_PROCESS] = TRUE;
				shortflag = S_LONG;
				break;
			case 'm' :	/* POSIX 1003.2a D5, D6 */
				am_i++;
				break;
			case 'M' :
				host_machine++;
				break;
			case 'p' :
				entries[INIT_PROCESS] = TRUE;
				break;
			case 'q' :
				quickly++;
				break;
			case 'r' :
				entries[RUN_LVL] = TRUE;
				break;
			case 's' :
				sflag++;
				break;
			case 'T' :
				ttystatus = TRUE;
				break;
			case 't' :
				entries[OLD_TIME] = TRUE;
				entries[NEW_TIME] = TRUE;
				break;
			case 'u' :
				entries[USER_PROCESS] = TRUE;
				shortflag = S_LONG;
				break;
		}
	}
	argc -= optind;	  /* skip the command name and processed options */
	argv = &argv[optind];
	for ( ; argc > 0 ; ++argv, --argc) {

/* Is this the "who am i" sequence? */
/* In the MSG version, both traditional "who am i" and local version of
   the "am i" are supported. */

		if ( strcmp(*argv,MSGSTR(AMI,"am i")) == 0 ) {
			am_i++;
			continue;
		}
		if ( argc >= 2 && strcmp(*argv,"am") == 0 ){ /* who am what? */
			if ( strcmp(*(argv+1),"i")==0 
			  || strcmp(*(argv+1),"I")==0 ) {
				am_i++;
				argc --;
				argv ++;
				continue;
			}
		}

/* Is there an argument left?  If so, assume that it is a utmp */
/* like file.  If there isn't one, use the default file. */

		if ( access(*argv,0) != FAILURE ) {
			utmpname(*argv);
		}else{
			usage(MSGSTR(NOEXST, 
				"%s doesn't exist or isn't readable"), *argv);
		}
	}

	if ( am_i ) {
	/* Which tty am I at?  Get the name and set "ptr" to just past */
	/* the "/dev/" part of the pathname. */

		if ((ptr = ttyname((int)fileno(stdin))) == NULL
		 && (ptr = ttyname((int)fileno(stdout))) == NULL
		 && (ptr = ttyname((int)fileno(stderr))) == NULL) {
		   usage(MSGSTR(NOTERM,"process not attached to terminal"),"");
		}
		ptr += sizeof("/dev/") - 1;

		entries[USER_PROCESS] = TRUE;
	}

/* Make sure at least one flag is set. */
	for ( i = 0 ; i <= FLAGS ; i++ ) {
		if ( entries[i] == TRUE )
			break;
	}
	if ( i > FLAGS )	/* No entries specified */
		entries[USER_PROCESS] = TRUE;

/* Some options overrides others */
	if ( quickly ) {
		ttystatus = FALSE;
		for(i = 1; i < FLAGS; i++)
		  entries[i] = FALSE;
		shortflag = S_SHORT;
		entries[USER_PROCESS] = TRUE;
		host_machine = allflag = header = 0;
	}
	if ( sflag ) {
		shortflag = S_SHORT;
	}

/* Now scan through the entries in the utmp type file and list */
/* those matching the requested types. */
	if (header) {
		fprintf(stdout,"%-*.*s%*.*s",
			LOGF_SIZE, LOGF_SIZE,
				MSGSTR(NAME,"Name    "),
			TTYS_HEAD, TTYS_HEAD,
				ttystatus ?  MSGSTR(TTYSTATUS,"ST") : "" );
		if ( !quickly ) {
			fprintf(stdout," %-*.*s %-*.*s",
				LINE_SIZE, LINE_SIZE,
					MSGSTR(WHO_LINE," Line"),
				TIME_HEAD, TIME_HEAD,
					MSGSTR(WHO_TIME,"   Time   ") );
			if (shortflag == S_LONG) {
				fprintf(stdout," %*.*s %*.*s",
					IDLE_HEAD, IDLE_HEAD,
						MSGSTR(ACTIVITY,"Activity"),
					PID_SIZE,  PID_SIZE,
						MSGSTR(WHO_PID,"PID") );
			}else{
				fprintf(stdout," %*.*s",
					IDLE_HEAD, IDLE_HEAD, "" );
				/* the space for PID is skipped */
			}
		}
		if (host_machine)
			fprintf(stdout," %-.*s", 
				HOST_SIZE, MSGSTR(WHO_HOST,"Hostname") );
		if ( shortflag != S_SHORT && entries[DEAD_PROCESS] == TRUE )
			fprintf(stdout,"%s", MSGSTR(WHO_EXIT,"/Exit"));
		fprintf(stdout,"\n");
	}

	while((utmp = getutent()) != NULL) {
		short	utype = utmp->ut_type;

		/* Are we looking for this type of entry? */
		if ( ! allflag
		  && (utype < 0 || utype > FLAGS || entries[utype] == FALSE) )
			continue;

		if ( utype == EMPTY ) {
			fprintf(stdout, MSGSTR(EMPTYSLT,"Empty slot.\n"));
			continue;
		}
		if (am_i&&strncmp(utmp->ut_line,ptr,sizeof(utmp->ut_line))!=0)
			continue;
		else{
			foundone = 1;	/* remember that we saw at least one*/
			if (quickly) {
				if (host_machine)
					fprintf(stdout, "%-*s %-*s\n",LOGF_SIZE,user(utmp), HOST_SIZE, host(utmp));
				else
					fprintf(stdout, "%-*s\n",LOGF_SIZE,user(utmp));
				totaluser++;
				continue;
			}
		}

		fprintf(stdout, "%-*.*s %*.*s %-*.*s %-*.*s %*.*s",
				LOGF_SIZE, LOGF_SIZE, user(utmp),
				TTYS_SIZE, TTYS_SIZE, mode(utmp),
				LINE_SIZE, LINE_SIZE, line(utmp),
				TIME_SIZE, TIME_SIZE, ltime(utmp),
				IDLE_SIZE, IDLE_SIZE, etime(utmp) );

		if(utype == RUN_LVL) {
			fprintf(stdout, "    %c    %d    %c",
				utmp->ut_exit.e_termination,
				utmp->ut_pid, utmp->ut_exit.e_exit);

		}
		if(shortflag == S_LONG &&
		   (utype == LOGIN_PROCESS
		    || utype == USER_PROCESS
		    || utype == INIT_PROCESS
		    || utype == DEAD_PROCESS)) {
			fprintf(stdout, " %*d", 
				PID_SIZE, utmp->ut_pid );
			if(utype == INIT_PROCESS
			   || utype == DEAD_PROCESS) {
				fprintf(stdout, MSGSTR(ID, " id=%-*.*s"),
					ID_SIZE, ID_SIZE, id(utmp));
				if(utype == DEAD_PROCESS) {
					fprintf(stdout, MSGSTR(TERMID, " term=%d exit=%d"),
						utmp->ut_exit.e_termination, utmp->ut_exit.e_exit);
				}
			}
			else
				if (host_machine)
					fprintf(stdout, " %-*s", HOST_SIZE,host(utmp));
		} else {
			if (host_machine)
				fprintf(stdout, " %-*s", HOST_SIZE,host(utmp));
		}
		fprintf(stdout,"\n"); 
	}               /* End of "while ((utmp = getutent()" */
	if (quickly) 
		fprintf(stdout,"Total users: %d\n", totaluser);
	else if (!foundone && (entries[USER_PROCESS] == TRUE))
		/* if no match for who am i, print something */
		if (ptr == NULL)
			fprintf(stdout, "%-*s \n",LOGF_SIZE, username);
		else
			fprintf(stdout, "%-*s %s \n",LOGF_SIZE, username,ptr);

	return(0);
}


/*
 *  NAME:  usage
 *
 *  FUNCTION:  print out the usage statement
 *	      
 *  RETURN VALUE:  	 exit 1
 */

usage(char *format, char *arg1)
{
	fprintf(stderr,format,arg1);
	fprintf(stderr,MSGSTR(USAGE, "\nUsage: who [-AabdHhlmMpqrsTtu] [am {i,I}] [file]\n"));
	fprintf(stderr,MSGSTR(ACC_ENT, "A\tAccounting entries\n"));
	fprintf(stderr,MSGSTR(OPTIONS, "a\tAll (AbdHlprTtu) options\n"));
	fprintf(stderr,MSGSTR(BOOTTM,  "b\tBoot time\n"));
	fprintf(stderr,MSGSTR(DEADPROC,"d\tDead processes\n"));
	fprintf(stderr,MSGSTR(HEADERS, "H\tDisplay a header (title)\n"));
	fprintf(stderr,MSGSTR(HELP,    "h\tDisplay this Help\n"));
	fprintf(stderr,MSGSTR(PROCCNT, "l\tLogin processes\n"));
	fprintf(stderr,MSGSTR(CURTERM, "m\tInformation about current terminal (same as 'am i')\n"));
	fprintf(stderr,MSGSTR(HOSTNAME, "M\tDisplay host machine name associated with user\n"));
	fprintf(stderr,MSGSTR(PROCESS, "p\tProcesses other than getty or user process\n"));
 	fprintf(stderr,MSGSTR(QUICK, "q\tQuick (only user name and number of users currently logged on)\n"));
	fprintf(stderr,MSGSTR(RUNLEVEL,"r\tRun level\n"));
	fprintf(stderr,MSGSTR(SHRTFRM, "s\tShort form (suppress Activity and PID)\n"));
	fprintf(stderr,MSGSTR(TTYMSG,"T\tStatus of tty (+ writable, - not writable, x exclusive open, ? hung)\n"));
	fprintf(stderr,MSGSTR(TIMECHG, "t\tTime changes\n"));
	fprintf(stderr,MSGSTR(USEINFO, "u\tActivity and PID of shell\n"));
	fprintf(stderr,"\n");

	exit(1);
}

/*
 *  NAME:  user
 *
 *  FUNCTION:  	return a space padded buffer of the user
 *		name in a utmp structure.
 *	      
 *  RETURN VALUE: a pointer to the array is returned.
 */

char *user(struct utmp *utmp)
{
	static char uuser[SIZEUSER+1];

	copypad(uuser,utmp->ut_user,SIZEUSER);
	return(uuser);
}

/*
 *  NAME: host
 *
 *  FUNCTION: return a space padded buffer of the host
 *             in a utmp structure.
 *
 *  RETURN VALUE: a pointer to the array is returned
*/
#define UTHSZ  	(sizeof(utmp->ut_host))
char *host(struct utmp *utmp)
{
	static char uhost[UTHSZ + 4];

	if (*utmp->ut_host != '\0') {
		sprintf( uhost," (%s)", utmp->ut_host);
		return(uhost);
	}else
		return ("");
}

/*
 *  NAME:  line
 *
 *  FUNCTION:  	return a space padded buffer of the tty line
 *		name in a utmp structure.
 *	      
 *  RETURN VALUE: a pointer to the array is returned.
 */

char *line(struct utmp *utmp)
{
	static char uline[sizeof(utmp->ut_line)+1];

	copypad(uline, utmp->ut_line, sizeof(uline));
	return(uline);
}

/*
 *  NAME:  copypad
 *
 *  FUNCTION:  	Basically do a strcpy and pad the end with spaces.
 *
 *  RETURN VALUE: void
 *  OUTPUT:  *to - printable string from 'from', or "  .  "
 */

static void copypad(char *to, char *from, size_t size)
{
	register int i;
	register char *ptr;
	short printable;

/* Scan for something textual in the field.  If there is */
/* nothing except spaces, tabs, and nulls, then substitute */
/* '.' for the contents. */

	printable = FALSE;
	for ( i=0, ptr=from ; *ptr != '\0' && i < size ; i++ ) {

/* Break out if a printable character found. */

		if( isgraph(*ptr) ) {
			printable = TRUE;
			break;
		}
		ptr += mblen( ptr, MB_CUR_MAX );
	}
	if ( printable ) {
		strncpy( to, from, size );	/* size is specified in BYTE */
	}else{
		int	halfway = size/2 - 1;       /* Where to put '.' */

/* Add pad at end of string consisting of spaces and a '\0'. */
/* Put an period at the halfway position.  (This only happens */
/* when padding out a null field.) */

		size--;		/* size was including terminating null */
		memset( to, ' ', size );
		to[ halfway ] = '.';
		to[ size ] = '\0';
	}
}


/*
 *  NAME:  id
 *
 *  FUNCTION:  return the uid string of the utmp structure.
 *
 */

char *id(struct utmp *utmp)
{
	static char uid[sizeof(utmp->ut_id)+1];
	register char *ptr;
	register int i;

	for(ptr= &uid[0],i=0; i < sizeof(utmp->ut_id);i++) {
		if(isprint((int)utmp->ut_id[i]) || utmp->ut_id[i] == '\0') {
			*ptr++ = utmp->ut_id[i];
		} else {
			*ptr++ = '^';
			*ptr = (utmp->ut_id[i] & 0x17) | 0100;
		}
	}
	*ptr = '\0';
	return(&uid[0]);
}

/*
 *  NAME:  ltime
 *
 *  FUNCTION:  	Return a string pointer to login time.
 */

char *ltime(struct utmp *utmp)
{
static	char	login_time[TIME_SIZE+1];
	struct tm *timestr;

	timestr = localtime(&utmp->ut_time);
	strftime( login_time, TIME_SIZE+1, loc_rec_form, timestr);
	login_time[ TIME_SIZE ] = '\0';

	return(login_time);
}

/*
 *  NAME:  etime
 *
 *  FUNCTION:  	Figure out the elapsed time since last user activity
 *		on a users tty line.
 *
 *  RETURN VALUE:	Return a string pointer to this information.
 */

#define	MIN_S	(time_t)60		/* minutes/hours/days in second */
#define	HOUR_S	(60 * MIN_S)
#define	DAY_S	(24 * HOUR_S)

char *etime(struct utmp *utmp)
{
static	char	eetime[IDLE_SIZE+1];
	time_t lastactivity;
	char device[sizeof( "/dev/" ) + SIZELINE];
	struct stat statbuf;
	extern int shortflag;

	if( shortflag != S_LONG
	   || (utmp->ut_type != INIT_PROCESS 
	    && utmp->ut_type != LOGIN_PROCESS	/* no idle time necessary */
	    && utmp->ut_type != USER_PROCESS
	    && utmp->ut_type != DEAD_PROCESS) )	
	    	return "";	/* empty */

	sprintf(device,"/dev/%.*s", SIZELINE, utmp->ut_line);

/* If the device can't be accessed, put a period insted of time. */

	if ( stat(device,&statbuf) == FAILURE ) {
		strcpy(eetime,"   ?  ");
	}else{
/* Compute the amount of time since the last character was sent to */
/* the device.  If it is older than a day, just exclaim, otherwise */
/* if it is less than a minute, put in an '.', otherwise put in */
/* the hours and the minutes. */

		lastactivity = time((time_t *)NULL) - statbuf.st_mtime;
		if ( lastactivity > DAY_S ) {
			strcpy(eetime,"  old ");
		}else if ( lastactivity < MIN_S ) {
			strcpy(eetime,"   .  ");
		}else{
			sprintf(eetime,"%3u:%02.2u",
			     (unsigned)( lastactivity/HOUR_S),
			     (unsigned)((lastactivity/MIN_S)%MIN_S));
		}
	}
	return(eetime);
}

static int badsys;      /* Set by alarmclk() if open or close times out. */


/*
 *  NAME:  mode
 *
 *  FUNCTION:  
 * 		Check "access" for writing to the line.  To avoid
 *		getting hung, set up any alarm around the open.  If
 * 		the alarm goes off, print a question mark.
 *	      
 *  RETURN VALUE: 	"+" 	writable
 *			"-"	no write permission
 *			"?"	unknown
 */

char *mode(struct utmp *utmp)
{
	char device[20];
	int fd;
	struct stat statbuf;
	register char *answer;
	extern ttystatus;

	if(ttystatus == FALSE
	   || utmp->ut_type == RUN_LVL
	   || utmp->ut_type == BOOT_TIME
	   || utmp->ut_type == OLD_TIME
	   || utmp->ut_type == NEW_TIME
	   || utmp->ut_type == ACCOUNTING
	   || utmp->ut_type == DEAD_PROCESS) return(" ");

	sprintf(&device[0],"/dev/%s",&utmp->ut_line[0]);


	badsys = FALSE;
	(void) signal(SIGALRM,(void (*)(int))alarmclk);
	alarm((unsigned int)3);
#ifdef  CBUNIX
	fd = open(&device[0],O_WRONLY);
#else
	fd = open(&device[0],O_WRONLY|O_NDELAY);
#endif
	alarm((unsigned int)0);
	if(badsys) return("?");

	if(fd == FAILURE) {
/* If our effective id is "root", then send back "x", since it */
/* must be exclusive use. */

		if(geteuid() == 0) return("x");
		else return("-");
	}

/* If we are effectively root or this is a login we own then we */
/* will have been able to open this line except when the exclusive */
/* use bit is set.  In these cases, we want to report the state as */
/* other people will experience it. */

	if(geteuid() == 0 || strncmp(&username[0],&utmp->ut_user[0],
	   (size_t)SIZEUSER) == 0) {
		if(fstat(fd,&statbuf) == FAILURE) answer = "-";
		else answer = "+";
	} else answer = "+";

/* To avoid getting hung set up any alarm around the close.  If */
/* the alarm goes off, print a question mark. */

	badsys = FALSE;
	(void) signal(SIGALRM,(void (*)(int))alarmclk);
	alarm((unsigned int)3);
	close(fd);
	alarm((unsigned int)0);
	if(badsys) answer = "?";
	return(answer);
}

alarmclk(void)
{
/* Set flag saying that "close" timed out. */

	badsys = TRUE;
}
