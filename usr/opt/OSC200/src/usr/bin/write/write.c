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
static char rcsid[] = "@(#)$RCSfile: write.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/10/18 21:11:37 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: write
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * write.c	1.10  com/cmd/comm/write,3.1,9021 4/4/90 14:11:57 
 */
#include <sys/secdefines.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <utmp.h>
#include <pwd.h>
#include <sys/limits.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include "write.h"
#include "write_msg.h" 
#include "pathnames.h" 

nl_catd  catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(Num,Str)		catgets(catd,MS_write,Num,Str)
#ifdef	SEC_BASE
#define	MSGSTR_SEC(Num,Str)	catgets(catd,MS_write_SEC,Num,Str)
#endif

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();

privvec_t saveprivs;
#endif

extern int gethostname(char *,int);

#define USAGE MSGSTR(M_MSG_35 \
   ,"write [-r|-q|-h handle,%s|%s|%s] [-n host] [user][@host] [line]\n")

struct flag {	
  short int nflag;       /* remote node flag */
  short int rflag;       /* wait for rely flag */
  short int qflag;       /* query flag */
  short int hflag;       /* handle flag */
  long int handle;       /* handle  */
};

struct userinfo {
  char host[MAX_HOST_LEN];
  char user[MAX_USERID_LEN+1]; 
  char tty[PATH_MAX+1];
  int fldes;
};

int	mb_cur_max;
int	outputfd;

void	getoptions(),validoptions(),gettargetinfo(),openlink(),exshell(),
	sendreply(),srvinfo(),getuserinfo(),myread(),mywrite(),uzero(),
	timeread(),Usage(void);

void error(int,char*,char*,int);
int setupsock(char *);
int encoder(char*);
void expand_control(char *);
int getreply(int);

void sigpipe(int);
void timeout(int);
void done(int);
extern struct utmp *getutent();
/*
 * NAME: write [-r] [-q] [-h handle,reply] [-n host] [user][@host] [line]
 * FUNCTION: Sends messages to other users on the system.
 * RETURN VALUE DESCRIPTION: exits with 0 if everything is ok
 *                           exits with 1 if reply = "cancel"
 *                           exits with -1 if error.
 */  

        /*      
        *       change from old style signals (call to signal) to
        *       call sigaction. From ksh/sh/fault.c
        */

void (*new_signal( int sig, void (*func)(int))) (int)
{
        struct sigaction        act, oact;

	sigemptyset( &act.sa_mask );
        sigaction(sig, (struct sigaction *)NULL, &act); /* get current signal */
        act.sa_handler = func;                  /* set new handler */
        switch (sig) {
        case SIGCONT:
        case SIGTTIN:
        case SIGTTOU:
        case SIGTSTP:
        case SIGSTOP:
                /* system calls are restartable after a job control signal */
                act.sa_flags |= SA_RESTART;
                break;
        }
	sigemptyset( &oact.sa_mask );
        sigaction(sig, &act, &oact);
        return(oact.sa_handler);                /* return old signal handler */
}

int
main(int argc,char *argv[])

{
	struct flag flags;
	struct userinfo userinfo,targetinfo;	
	char message[MAX_INPUT];
  	char *host;
	int reply = 0;
	char *eof = NULL;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_WRITE,NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

        if (authorized_user("sysadmin"))
	{
		if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			SEC_ALLOWMACACCESS,
#endif
			-1), saveprivs))
		{
			fprintf(stderr,
			MSGSTR_SEC(PRIV, "%s: insufficient privileges\n"),
			command_name);
			exit(1);
		}
	}
#endif /* SEC_BASE */

	new_signal(SIGCONT, SIG_IGN); /*
				       * This keeps read from failing
				       * it may not be necessary in the future
				       */
	new_signal(SIGPIPE,sigpipe);
	new_signal(SIGINT, done);	/* exit on interrupt */
	getoptions(argc,argv,&flags,&targetinfo,message);
	validoptions(flags,&targetinfo);
	getuserinfo(&userinfo);	
	if (targetinfo.host[0] != '\0')
		host = targetinfo.host;
	else
		host = userinfo.host;
	if (flags.hflag) {
		sendreply(flags.handle,encoder(message),host);
		exit(0);
	}
	if (flags.qflag) {
		srvinfo(host,userinfo.host);
		exit(0);
	}
	gettargetinfo(&targetinfo,&flags);
	if (targetinfo.host[0] == '\0')
		strcpy(targetinfo.host,userinfo.host);
	openlink(&userinfo,&targetinfo,&flags);
	outputfd = targetinfo.fldes;
	printf("");
	do {
		bzero(message,sizeof(message));
		if ((eof = fgets(message,(int)sizeof(message),stdin)) == NULL)
			strcpy(message,MSGSTR(M_MSG_39,"<EOT>\n"));
		if (message[0] == '!' && flags.rflag == 0) {
			exshell(message, &targetinfo);
			continue;
		}
		expand_control ( message );  /*   expand control characters   */
		mywrite (targetinfo.fldes,message,sizeof(message));
	} while (eof != NULL);
	if (flags.nflag || flags.rflag ) {
		bzero(message,sizeof(message));
		message[0] ='\0';
		mywrite (targetinfo.fldes,message,sizeof(message));
	}
	if (flags.rflag)
		reply = getreply(targetinfo.fldes);
	close(targetinfo.fldes);
	exit(reply);	
/*NOTREACHED*/
}

/*
 * NAME: getoptions
 * FUNCTION: get the options from the command line and set the proper
 *           flags and target information.  calls Usage if an error is 
 *           detected.
 */
void getoptions(argc,argv,flags,tarinfo,message)
int argc;
char *argv[];
struct flag *flags;
struct userinfo *tarinfo;
char *message;
{
	int j,k;
	extern int optind;
	extern char *optarg;
	int c;
	char *h;

	if (argc < 2) {
		Usage();
	}
	flags->nflag = 0;    /* initialize flags */
	flags->rflag = 0;
	flags->qflag = 0;
	flags->hflag = 0;
	flags->handle = 0;
	tarinfo->user[0] = '\0';
	tarinfo->host[0] = '\0';
	tarinfo->tty[0] = '\0';
	tarinfo->fldes = 0;
	message[0] = '\0';

	while ((c = getopt(argc,argv,"n:rqh:@:")) != EOF) {
		switch (c) {
			case 'n':
				strcpy(tarinfo->host,optarg);
				flags->nflag++;
				break;
			case 'r':
				flags->rflag++;
				break;
			case 'q':
				flags->qflag++;
				break;
			case 'h':
				flags->hflag++;
				j = 0;
				h = optarg;
	    			while (h[j] != '\0'){
				  	if (h[j] == ',') { 
			            		message[j] = '\0';
				    		flags->handle = atoi(message);
				    		j++;
						break;
				  	}
				  	else {
				    		message[j] = h[j];
				    		j++;
				  	}
				}
				strcpy(message,h+j);
				break;
			case '@':
				tarinfo->user[0] = '-';
				strcpy(tarinfo->host,optarg);
				flags->nflag++;
				break;
			default:
				Usage();
			break;
		}
	}		

	for (; optind < argc; optind++){
		if (tarinfo->user[0] != '\0') {    /* get tty */
			if (tarinfo->tty[0] != '\0') Usage();
	    		strcpy(tarinfo->tty,argv[optind]);
	   		continue;
	  	}
	  	if (argv[optind][0] == '@') {
			if (argv[optind][1] == '\0') Usage();
			flags->nflag++;
			if (tarinfo->host[0] != '\0') Usage();
			strcpy(tarinfo->host,argv[optind] + 1);
			continue;
	  	}
	  	k=0;
	  	while (argv[optind][k] != '\0'){
	    		if (argv[optind][k] == '@') {  /* remote host */
	      			flags->nflag++;
	      			if (tarinfo->host[0] != '\0') Usage();
		  		strcpy(tarinfo->host,argv[optind] + k + 1);
		  		argv[optind][k] = '\0';
		  		break;
	      		}
	      		k++;
	  	} /* end of while loop */
	  	if (k > 0) {
	    		if (tarinfo->user[0] != '\0') Usage();
			strcpy(tarinfo->user,argv[optind]);
	    	}
	} /*end of for loop */
} /* end of getoptions */

/*
 * NAME: Usage
 * FUNCTION:  prints the usage statement to stderr and then exits.
 */
void
Usage(void)
{
	fprintf(stderr,USAGE,"ok","cancel","query");
	exit(-1);
}

/*
 * NAME: validoptions
 * FUNCTION: Checks for incompatible flags.  If incompatible options are
 *           found then validoptions will issue an error message and exit
 * 	     the program.
 */
void validoptions(flags,targetinfo)
struct flag flags;
struct userinfo *targetinfo;
{
	if (flags.rflag + flags.qflag + flags.hflag > 1)
		Usage();
	if (flags.hflag + flags.qflag > 0 && targetinfo->user[0] != '\0')
		Usage();
	if (targetinfo->user[0] == '-' && targetinfo->tty[0] == '\0')
		Usage();
	if (flags.rflag > 0 && targetinfo->user[0] == '\0')
		Usage();
	if (flags.rflag + flags.qflag + flags.hflag == 0 
				&& targetinfo->user[0] == '\0')
		Usage();
	if (flags.nflag > 0 && targetinfo->host[0] == '\0')
		Usage();
	if (flags.hflag > 0 && flags.handle == 0)
		Usage();
	if(flags.hflag > 0 && targetinfo->tty[0] != '\0')
		Usage();
}

/*
 * NAME: getuserinfo
 * FUNCTION:  This function will get the user information (i.e. user id,
 *   host name and tty).
 */
void getuserinfo(userinfo)
struct userinfo *userinfo;
{
	struct passwd *pbuf;
	char *tty;

	pbuf = getpwuid(getuid());    /* get user id */
	if (pbuf == NULL) {
		fprintf (stderr, MSGSTR(M_MSG_2, 
			"write: unable to find your login ID\n") );
		exit(-1);
	}
	strcpy(userinfo->user,pbuf->pw_name);	
	tty = ttyname((int)fileno(stdin));
	if (tty == NULL)
		tty = ttyname((int)fileno(stdout));
	if (tty == NULL)
		tty = ttyname((int)fileno(stderr));
	if (tty == NULL) {
		fprintf(stdout, MSGSTR(M_MSG_3, 
			"write: unable to determine your tty\n") );
		strcpy(userinfo->tty,"/dev/");
		strncat(userinfo->tty,MSGSTR(UNKNOWN,"UNKNOWN"),(size_t)(MAX_TTY_LEN-5));
	}
	else
		strncpy(userinfo->tty,tty,(size_t)MAX_TTY_LEN);
	if (gethostname(userinfo->host,sizeof(userinfo->host)) < 0) {
		fprintf(stderr, MSGSTR(M_MSG_4, 
			"write: unable to determine your host name\n") );
		exit(-1);
	}
	userinfo->fldes = fileno(stdin);
}

/*
 * NAME: encoder
 * FUNCTION:  encode the message into 0 for 'ok'
 *                                    1 for 'cancel'
 *				      2 for 'query'
 *            exit if anything else
 */
int encoder(char *message)
{
	if (message[0] == 'o' || message[0] == 'O')
		return(OK);
	else if (message[0] == 'c' || message[0] == 'C')
		return(CANCEL);
	else if (message[0] == 'q' || message[0] == 'Q')
		return(MQUERY);
	else {
		fprintf(stderr,MSGSTR(M_MSG_5,
			"Invalid reply, must be %s, %s or %s\n"),
			SOK,SCANCEL,SQUERY);
		exit(-1);
	}
/*NOTREACHED*/
}

/*
 * NAME: sendreply
 * FUNCTION:  This function will send the handle and the encoded reply to 
 *    writesrv.  
 */
void sendreply(handle,code,host)
int handle, code;
char *host;
{
	char service[MAX_INPUT];
	int sock;
	int status;

	sock = setupsock(host);                /* set up link to host */
	bzero(service,sizeof(service));
	sprintf(service,"%c%s",HWRITE,host);
        mywrite(sock,service,sizeof(service));    /* send service request */
	bzero(service,sizeof(service));
	sprintf(service,"%d %d",handle,code);
        mywrite(sock,service,sizeof(service));    /* send handle and code */
	bzero(service,sizeof(service));
        timeread(sock,service,sizeof(service));     /* get status */
	status = atoi(service);
	if (status == 2) {        /* get original message */
		mywrite(sock,service,sizeof(service));   /* sync */
		do {     /* get original message */
			bzero(service,sizeof(service));
			myread(sock,service,sizeof(service));
			if (service[0] != '\0') printf("%s",service);
		} while (service[0] != '\0');			
		bzero(service,sizeof(service));
	        myread(sock,service,sizeof(service));     /* get status */
		status = atoi(service);
	}
	if (status < 0) error(status,"",host,0);
	mywrite(sock,service,sizeof(service));    /* use as sync */
	return;
}

/* 
 * NAME: srvinfo
 * FUNCTION:  This function gets a list of all messages a waiting a reply,
 *    from the writesrv and then displays them.
 * RETURN:  returns -1 if error
 *          else  0
 */
void srvinfo(tohost,fromhost)
char *tohost;
char *fromhost;
{
	int sock;
	char service[MAX_INPUT];
	
	bzero(service,sizeof(service));
	sock = setupsock(tohost);      /* set up link to host */
	service[0] = QUERY;
	strcat(service,fromhost);
	mywrite (sock,service,sizeof(service));  /* send service request */
	bzero(service,sizeof(service));
	timeread(sock,service,sizeof(service));
	do {               /* get list */
		if (service[0] != '\0') printf("%s",service);
		bzero(service,sizeof(service));
		myread(sock,service,sizeof(service));
	} while (service[0] != '\0');
	mywrite(sock,service,sizeof(service));   /* use as sync */
	close(sock);
}

/*
 * NAME: gettargetinfo
 * FUNCTION:  This function will verify all target information that
 *    was recieved from the command line and then it will gather any 
 *    remaining information it needs.  
 */
void gettargetinfo(targetinfo,flags)
struct userinfo *targetinfo;
struct flag *flags;
{
	struct utmp *ubuf;
	int found = 0;
	int got_user = 0;
	char tty[PATH_MAX+1];
	struct ttys *head = NULL,*current = NULL,*test = NULL;

	targetinfo->fldes = 0;
	if (flags->nflag == 0 && flags->rflag == 0) { /* for local only */
		if (targetinfo->user[0] == '-' && targetinfo->tty[0] != '\0') {
			if (targetinfo->tty[0] != '/') {
				strcpy(tty,targetinfo->tty);
				strcpy(targetinfo->tty,"/dev/");
				strcat(targetinfo->tty,tty);
			} /* end of if */
			return;
		} /* end of if */
		if ((ubuf = getutent()) == NULL) { /* open utmp file */
			fprintf (stderr, MSGSTR(M_MSG_8, 
				"write: Can't open %s\n") ,UTMP_FILE);
			if (targetinfo->tty[0] == '\0'){
				fprintf (stderr, MSGSTR(M_MSG_9, 
					"write: can not continue\n") );
				exit(-1);
			} /* end of if */
		}  /* end of if */
		else {                            /* check entry in utmp file */
		  while (ubuf != NULL) {
		    if (ubuf->ut_type == USER_PROCESS &&
			strncmp(ubuf->ut_user,
				targetinfo->user,
				(size_t)MAX_USERID_LEN) == 0) {
			got_user++;
			if (targetinfo->tty[0] == '\0') {
			    errno = 0;
			    test = (struct ttys *) malloc (sizeof(struct ttys));
			    if (test == NULL) {
			        if (errno == 0) error(MALLOC,"","",0);
				perror (MSGSTR(M_MSG_38,"malloc"));
				exit(-1);
			    }
			    if (current == NULL) 
				current = test;
			    else {
				current->next = test;
			    	current = current->next;
			    } /* end of else */
		            current->next = NULL;
			    if (head == NULL) head = current;	
			    strncpy(current->tty,ubuf->ut_line,(size_t)MAX_TTY_LEN);
			    found++;
			} /* end of if */
			else {
				char *p = targetinfo->tty;
				if (strncmp(p, "/dev/", (size_t)5) == 0)
					p += 5;
				if (strncmp(p, ubuf->ut_line, 
					(size_t)MAX_TTY_LEN) == 0) {
					found++;
					break;
				}
			}
		      }
		      ubuf = getutent();
		    } /* end of while */
		    endutent();
		    if ( !got_user ){
			fprintf(stderr,MSGSTR(M_MSG_10, 
				"%s is not logged on.\n"),targetinfo->user);
			exit(-1);
		    }
		    if ( !found ){
			fprintf(stderr,MSGSTR(M_MSG_19,
					      "tty %s does not exist.\n"),
				targetinfo->tty);
			exit(-1);
		    } /* not found */
		  } /* end of else */
		  if (targetinfo->tty[0] == '\0') 
			strcpy(targetinfo->tty,head->tty);
		  if (targetinfo->tty[0] != '/' && targetinfo->tty[0] != '\0') {
			strcpy(tty,targetinfo->tty);
			strcpy(targetinfo->tty,"/dev/");
			strcat(targetinfo->tty,tty);
		  } /* end of if */
		  if (head != NULL && head->next != NULL) {
			fprintf(stdout, MSGSTR(M_MSG_11, 
				"%s is logged on more than one place.\n") ,
					targetinfo->user);
			fprintf(stdout, MSGSTR(M_MSG_12, 
					"You are connected to %s.\n") ,
					targetinfo->tty);
			fprintf(stdout, MSGSTR(M_MSG_13, 
					"Other locations are:\n") );
			head = head->next;
			found--;
			while(head != NULL) {
				fprintf(stdout,"%s\n",head->tty);
				head = head->next;
			} /* end of while */
		  } /* end of if */
	} /* end of if */
} /* end of gettargetinfo */
	
/*
 * NAME: openlink
 * FUNCTION:  This function opens the target tty or sends a service repuest to
 *   the remote writesrv to open it's target tty.  In either case after it is
 *   opened, the header informing the target user of a incoming message will 
 *   be displayed.  The file descriptor of the target will be set.
 */
void openlink (userinfo,targetinfo,flags)
struct userinfo *userinfo,*targetinfo;
struct flag *flags;
{
	time_t timet;
	struct tm *tm_time;
	char date[NLTBMAX],errs[MAX_INPUT],nums[MAX_INPUT]; 
	int len,err,num,i;
	int sock;
	char service[MAX_INPUT];

	if (flags->nflag > 0 || flags->rflag > 0) {    /* remote */
		bzero(service,sizeof(service));
		sock = setupsock(targetinfo->host);
		if (flags->rflag > 0) service[0] = RWRITE;
		else service[0] = RELAY;
		strcat(service,userinfo->host);
		mywrite(sock,service,sizeof(service));/* send service request */
		targetinfo->fldes = sock;
		bzero(service,sizeof(service));
		strcpy(service,targetinfo->user);
		mywrite(sock,service,sizeof(service));  /* send user  */
		bzero(service,sizeof(service));
		if (targetinfo->tty[0] == '\0') strcpy(service,".");
		else strcpy(service,targetinfo->tty);
		mywrite(sock,service,sizeof(service));  /* send tty */
		bzero(service,sizeof(service));
		timeread(sock,service,sizeof(service));    /* get status */
		sscanf(service,"%s%s",errs,nums);
		err = atoi(errs);
		num = atoi(nums);
		if (err <= 0) 
		  error(err,targetinfo->user,targetinfo->host,flags->rflag);
		if (num > 1) {             /* get list of ttys user is using */
			bzero(service,sizeof(service));
			myread(sock,service,sizeof(service));
			fprintf(stderr, MSGSTR(M_MSG_15, 
			    "%s on %s is logged on more than one place.\n"),
				targetinfo->user,targetinfo->host);
			fprintf(stderr, MSGSTR(M_MSG_16, 
				"You are connected to %s.\n"),service);
			fprintf(stderr, MSGSTR(M_MSG_17,
				"Other locations are:\n"));
			for (i=1;i<num;i++) {
				bzero(service,sizeof(service));
				myread(sock,service,sizeof(service));
				printf("%s\n",service);
			}
		}
	}
	else {     /* open local tty */
		if (access(targetinfo->tty,F_OK) < 0) {
			fprintf(stderr,MSGSTR(M_MSG_19, 
				"write: No such tty\n"));
			exit(-1);
		}
		targetinfo->fldes = open (targetinfo->tty,O_WRONLY,0);
		if (targetinfo->fldes < 0) {
			fprintf(stderr, MSGSTR(M_MSG_21, 
				"write: Permission denied\n"));
			exit(-1);
  		}
	}
	timet = time((time_t *) 0);
	tm_time = localtime(&timet);
	len = strftime(date, NLTBMAX, "%c", tm_time);
	bzero(service,sizeof(service));
	service[0] = '\n';
	mywrite(targetinfo->fldes,service,sizeof(service)); /* send blank line */
	bzero(service,sizeof(service));
	sprintf(service,
		MSGSTR(M_MSG_1,
		"   Message from %s on %s (%s) [%s] ...\n")
		,userinfo->user,userinfo->host,userinfo->tty+5,date);
	mywrite(targetinfo->fldes,service,sizeof(service));     /* send header */
}

/*
 * NAME: exshell
 * FUNCTION:  This function will execute the rest of the message line as
 *    a shell command.  
 */ 
void exshell(message, targetinfo)
char *message;
struct userinfo *targetinfo;
{
	int pid;

	pid = fork();
	if (pid < 0) {
		perror(MSGSTR(M_MSG_22,"execute shell command"));
		return;
	}
	if (pid == 0) {    /* inside of child */
		(void) setgid (getgid());
		(void) close (targetinfo->tty);
		execl(getenv("SHELL") ?
		    getenv("SHELL") : _PATH_SH, "sh", "-c", ++message, 0);
		exit(0);
	}
	while (wait((int *)NULL) != pid)   /* in parent */
		;
	printf("!\n");
}

/* 
 * NAME: getreply
 * FUNCTION: This function will wait for writesrv to send an encoded reply.
 *    This encoded reply will then be returned. 
 */
int getreply(fldes)
int fldes;
{
	char buf[MAX_INPUT];
	int reply;

	bzero(buf,sizeof(buf));
	myread(fldes,buf,sizeof(buf));
	reply=atoi(buf);	
	if (reply < 0)
		error(reply,"","",0);
	mywrite(fldes,buf,sizeof(buf));  /* send acknowledgement */
	return(reply);
}

/*
 * NAME: setupsock:
 * FUCNTION: sets up the connection to proper socket.
 */
int setupsock(char *host)
{
	struct servent *sp;
	struct sockaddr_in server;
	struct hostent *hp, *gethostbyname(),*gethostbyaddr();
	int sock;
	
	sp = getservbyname("writesrv","tcp");
	if (sp == NULL) {
		fprintf(stderr,MSGSTR(M_MSG_24,"writesrv: unknown service\n"));
		exit(-1);
	}
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0) {
		perror( MSGSTR(E_MSG_25, "opening stream socket") );
		exit(-1);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(host);  /* if internet address */
	if ( server.sin_addr.s_addr == -1) {               /* if hostname */
		hp = gethostbyname(host); 
		if (hp == 0) { 
		    fprintf(stderr,MSGSTR(M_MSG_26,"write: unknown host\n"));
		    exit(-1);
		}
		bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	}
	server.sin_port = sp->s_port;
	if (connect(sock,(struct sockaddr *)&server,sizeof(server)) < 0) {
		perror ( MSGSTR(E_MSG_27, "connecting stream socket") );
		exit(-1);
	}
	return(sock);
}

/*
 * NAME: error
 * FUNCTION:  Displays the proper error message based on the status 
 *     returned by a remote host.
 */
void
error(int err,char *user, char *host, int rflag)
{
	if ( err == NOTLOG && rflag > 0) 
		exit(2);
	else if ( err == NOTLOG) fprintf(stderr,MSGSTR(M_MSG_28, 
			"%s not logged in on %s\n") ,user,host);
	else if (err == NOTTY) 
		fprintf(stderr,MSGSTR(M_MSG_29,"No such tty\n") );
	else if (err == NOPERM ) fprintf(stderr,MSGSTR(M_MSG_30,
					"Permission denied\n"));	
	else if (err == NOOPEN) fprintf(stderr,MSGSTR(M_MSG_31,
			"write: Can't open %s on %s\n"),UTMP_FILE,host);	
	else if (err == MALLOC) fprintf(stderr, MSGSTR(M_MSG_32, 
					"malloc: FATAL ERROR\n") );
	else if (err == BADHAND) fprintf(stderr,MSGSTR(M_MSG_33,
					"Invalid handle on %s\n"),host);
	else if (err == SNDRPLY) fprintf(stderr,MSGSTR(M_MSG_34, 
						"Could not send reply\n"));
	else if (err == GETRPLY) fprintf(stderr,MSGSTR(M_MSG_36, 
						"Could not get reply\n"));
	else if (err == NOSERVICE) {
		fprintf(stderr,MSGSTR(M_MSG_41,
			"writesrv can not provide this service right now.\n")); 
		fprintf(stderr,MSGSTR(M_MSG_42,
			"writesrv is shutting down\n"));
	}
	exit(-1);

}

/* 
 * NAME: myread
 * FUNCTION:  perform the read and checks the return code.
 */
void myread (fldes,buf,len)
int fldes;
char *buf;
int len;
{
	int nchar;
	
	nchar = read(fldes,buf,(unsigned)len);
	if (nchar != len) {
		fprintf(stderr,MSGSTR(M_MSG_37,
		"Can not communicate with the daemon writesrv\n"));
		perror("read() in myread()");
		exit(-1);
	}
}

/* 
 * NAME: mywrite
 * FUNCTION:  perform the write and checks the return code.
 */
void mywrite (fldes,buf,len)
int fldes;
char *buf;
int len;
{
	int nchar;
	
	nchar = write(fldes,buf,(unsigned)len);
	if (nchar != len) {
		fprintf(stderr,MSGSTR(M_MSG_37,
		        "Can not communicate with the daemon writesrv\n"));
		perror("write() in mywrite()");
		exit(-1);
	}
}
/*
 * NAME: sigpipe
 * FUNCTION: catches SIGPIPE, issues error message and exits.
 */
void
sigpipe(int s)
{
	fprintf(stderr,MSGSTR(M_MSG_37,
		"Can not communicate with the daemon writesrv\n"));
	exit(-1);
/*NOTREACHED*/
}

/*
 * NAME: timeout
 * FUNCTION: catches the alarm signal prints error message and exits
 */
void
timeout(int s)
{
	fprintf(stderr,MSGSTR(M_MSG_40,"%s must not be running\n"),"writesrv");
	exit(-1);
/*NOTREACHED*/
}
/*
 * NAME: timeread
 * FUNCTION: only wait 1 miniute for a response from writesrv
 *  this function is only used for the first read from writesrv for a
 * connection.
 */
void timeread(fldes,buf,len)
int fldes;
char *buf;
int len;
{
	new_signal (SIGALRM,timeout);
	alarm ((unsigned)60); 
	myread(fldes,buf,len);
	alarm ((unsigned)0);
}

void
expand_control ( char *str )
{
	char str2[MAX_INPUT];
	unsigned char printable;
	int i = 0;
	int j = 0;
	unsigned char c;
	bzero (str2, MAX_INPUT);
	if (mb_cur_max > 1) {
		wchar_t wc;
		int len;

		while (( i < MAX_INPUT) && (j < MAX_INPUT)) {
			if (str[i] == '\0')
				break;
			len = mbtowc(&wc, &str[i], mb_cur_max);
			if (len < 0) {
				/* bail out */
				perror("mbtowc");
				done(0);
			}
			if (iswprint(wc) || iswspace(wc) || (wc == WRT_BELL)) {
				do {
					str2[j++] = str[i++];
				} while (--len > 0);
			} else {
				str2[j++] = '^';
				printable = toascii(wc ^ 0x40);
				if ((isprint(printable)) &&
				   ((j + 1) < MAX_INPUT))
					str2[j++] = printable;
				i += len;
			}
		}
	} else {
		while (( i < MAX_INPUT) && (j < MAX_INPUT)) {
			if((c = str[i]) == '\0')
				break;
			if (isprint(c) || isspace(c) || (c == WRT_BELL))
				str2[j++] = c;
			else {
				str2[j++] = '^';
				printable = c^0x40;
				if ((isprint(printable)) && 
				   ((j + 1) < MAX_INPUT))
					str2[j++] = printable;
			}
			++i;
		}
	}

	if (j >= (MAX_INPUT - 1))
	   str2[MAX_INPUT-1] = WRT_NEWLINE;

	strcpy (str, str2);
}

void
done(int s)
{
	char *buf = MSGSTR(M_MSG_39,"<EOT>\n");

	mywrite(outputfd, buf, strlen(buf));
	exit(0);
}
