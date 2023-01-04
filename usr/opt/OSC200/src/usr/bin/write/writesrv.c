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
static char rcsid[] = "@(#)$RCSfile: writesrv.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/10/11 19:54:48 $";
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
 * FUNCTIONS: writesrv
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* writesrv.c	1.14  com/cmd/write/writesrv.c, bos, bos320 6/19/91 10:46:14 */
/*
 *
 * NAME: writesrv
 * 
 * FUNCTION:  Writesrv is a daemon that is started by one of the rc files.
 *    Writesrv provides services to the command write.  Services:
 *       1)   Automaticly keeps track of messages awaiting replies
 *            from users on writesrv's host.
 * 	 2)   RELAY - relays messages to users logged in on writesrv's
 *            host from other hosts.
 *       3)   RWRITE - relays messages that need a reply from an
 *            user on writesrv's host. (Generates a handle and waits for
 *            a reply).
 *       4)   HWRITE - relays replies from users on writesrv's host to
 *            the appropriate write command.
 *       5)   QUERY - Transmists a list of messages awaiting replies.
 *  
 */                                                                   
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <utmp.h>
#include <pwd.h>
#include <sys/limits.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
/* #include <sys/access.h> */
#include <stdlib.h>
#include <langinfo.h>
#include "signal.h"
#include "write.h"
#include "writesrv_msg.h" 

nl_catd  catd;             /* Cat descriptor for scmc conversion */
#define MSGSTR(Num,Str) catgets(catd,MS_writesrv,Num,Str)

#define	HEADER	MSGSTR(M_MSG_17,"Node     service  handle      time\n")
#define DASHES	"-------- -------- ----------- ---------------\n"
#define TRUE 1
#define SDIR	"/var/spool/writesrv"
#define SIGNORM		30
#define SIGFORCE	31


/*	structure for remembering the messages awaiting a reply */
struct srv {
	struct srv	*next,*prev;
	char		host[MAX_HOST_LEN];
	char		service[MAX_SER_LEN];
	int		handle;
	struct tm      	tm_time;
	pid_t		pid;
};

struct srvhd {
	struct srv	*first,*last;
};
  
int relay(),rwrite(),hwrite(),query();
void updatesrv(),myread(),mywrite(),stdclose();
struct utmp *getutent();
int opentty(int msgsock, FILE **pFile);

struct optable {                      /* the op code table */
	int	(* op_fun)();       /* function that provides the service */
	int	op_suid;            /* service requires setuid */
};

struct optable optab[] =
{
	{ relay , 1 },            /* RELAY */
	{ rwrite , 1 },           /* RWRITE */
	{ hwrite , 0 },           /* HWRITE */
	{ query , 1 }             /* QUERY */
};
	
struct message {   /* used to store entire messages (write -h handle query) */
	struct message *next;
	char buf[MAX_INPUT];
};

int 		handle = 1, sigcld(void),sigterm(void),signorm(void),signalrm(void),snorm = 0;
struct srvhd 	head;
	
        /*      
        *       change from old style signals (call to signal) to
        *       call sigaction. From ksh/sh/fault.c
        */

void (*new_signal( int sig, void (*func)(int))) (int)
{
        struct sigaction        act, oact;
	int iTmp;

	sigemptyset(&act.sa_mask);
	sigfillset(&act.sa_mask);
        act.sa_handler = func;                  /* set new handler */
	act.sa_flags = SA_RESTART;

        iTmp = sigaction(sig, &act, &oact);
        if ( 0 != iTmp ) {
                fprintf(stderr,"sigaction(%d,,) returned %d\n", sig, iTmp);
                perror("");
        }

        return(oact.sa_handler);                /* return old signal handler */
}

int sigpipe(void)
{
	exit(-1);
}

main(argc,argv)
int argc;
char *argv[];
{
	int sock,msgsock,rval,i,hand=0;
	pid_t pid;
	char buf[MAX_INPUT];
	char c;
	
        (void)setlocale(LC_ALL,"C");	     /* set locale to "C" */
	catd = catopen(MF_WRITESRV,NL_CAT_LOCALE);

	for (i=1; i<NSIG;i++) {              /* need to catch all signals */
		switch (i) {
			case SIGKILL:
			case SIGSTOP:
				continue;
			case SIGTERM:
				new_signal(SIGTERM,(void (*)(int))sigterm);
				continue;
			case SIGFORCE:
				new_signal(SIGFORCE,(void (*)(int))sigterm);
				continue;
			case SIGNORM:
				new_signal(SIGNORM,(void (*)(int))signorm);
				continue;
			case SIGCHLD:
				new_signal(SIGCHLD,(void (*)(int))sigcld);
				continue;
			default:
				new_signal(i,SIG_IGN);
		}
	}

	/*
	 * Check for our socket directory.
	 * Create it if we can't find it.
	 */
	if(access(SDIR, W_OK|X_OK) < 0) 
	    if (errno == ENOENT) {
		if (mkdir(SDIR, 0755) < 0) {
			perror(SDIR);
			exit(1);
		}
	    }
	    else {
		perror(SDIR);
		exit(1);
	    }

	
      /*
       * force into background unless
       * in debugging mode
       */
	if (argc == 1) {
		daemon(1,0);	/* From libutil.a */
	}


	head.first = NULL;	 /* initialize srv pointers */
	head.last = NULL;	

	sock = setupsock();                             /* create socket */
	do {
		msgsock = accept(sock,0,0);         /* accept connection */
		if (msgsock == -1) {
		  if ( EINTR != errno )
			perror( MSGSTR(E_MSG_1, "accept") );
		} else {
		  bzero(buf,sizeof(buf));         /* get service request */
		  if (rval = read(msgsock, buf, sizeof(buf)) < 0)
			fprintf(stderr, MSGSTR(M_MSG_2,
					 "reading stream message"));
		  c = buf[0];     /* get service request number */
		  switch (pid = fork()) { 
			case -1:                    /* problem with fork */
				close(msgsock);
				continue;
			case 0:                              /* in child */
				for (i=1;i<NSIG;i++) 
					switch(i) {
					  case SIGKILL:
					  case SIGSTOP:
						continue;
		  			  case SIGINT:
					  case SIGQUIT:
					  case SIGHUP:
						new_signal(i,SIG_IGN);
						break;
					  case SIGUSR1:
						break;
					  case SIGPIPE:
						new_signal(SIGPIPE,(void (*)(int))sigpipe);
						break;
					  default:
						new_signal(i,SIG_DFL);
		                        }
				setgid((gid_t)optab[c-'0'].op_suid);
				exit((* optab[c-'0'].op_fun)(msgsock));
			default:                            /* in parent */
				if (c == RWRITE)
				  updatesrv(&head,&handle,buf,pid);
				else {
				  hand = 0;
				  updatesrv(&head,&hand,buf,pid);
				}
				break;
		  } /* end switch */
		close(msgsock);
		} /* end else */
	} while (TRUE);
}

/*
 * NAME: sigterm
 * FUNCTION: to catch SIGTERM and SIGFORCE, kill all children and die
 */
int sigterm(void)
{
	struct srv *current;
	
	current = head.first;
	while (current != NULL) {
		kill(current->pid,SIGKILL);
		current = current->next;
	}	
	exit(1);
}

/*
 * NAME: signorm
 * FUNCTION: catch SIGNORM and wait for all current activity to stop
 * NOTE: It is not possible to stop new service request from coming in
 *    but, it is possible for every service to decide if the service must
 *    be provide when the signorm flag is set
 */
signorm(void)
{
	snorm++;
	new_signal(SIGALRM,(void (*)(int))signalrm);
	alarm((unsigned)30);
}
/* NAME: signalarm
 * FUNCTION: SIGALRM catcher for the parent only, check to see if there is any
 *  active children.
 */
signalrm(void) 
{
	if (head.first == NULL)
		exit(0);
	else {
		new_signal(SIGALRM,(void (*)(int))signalrm);
		alarm((unsigned)30);
	}
}
/*
 * NAME: sigcld
 * FUNCTION:  (signal catcher) when child dies remove its entry in
 *     servies table (srv).
 */
int sigcld(void)
{
	pid_t pid;
	struct srv *current;
	
	while ( (pid = waitpid(-1, NULL, WNOHANG )) > 0 ) {
		/* new_signal(ISGCHLD,(void (*)(int))sigcld); */
		current = head.first;
		while (current != NULL) {
		  if (current->pid == pid) {
						    /* only one element in the list */
			if (head.first == head.last && current == head.first) {
				head.first = NULL;
				head.last = NULL;
			}
			else if (head.first == current) {   /* remove first element */
				/*
				head.first = current;
				head.first->next = current->next;
				*/
				head.first = head.first->next;
				head.first->prev = NULL;
			}
			else if (head.last == current) {      /* remove last element */
				/*
				head.last = current->prev;
				*/
				head.last = head.last->prev;
				head.last->next = NULL;
			}
			else {                            /* remove a middle element */
				current->prev->next = current->next;
				current->next->prev = current->prev;
			}
			free(current);
			break;
		  }
		  else
		    current = current->next;
		} /* end while */
	}
}

/*
 * NAME: setupsock
 * FUNTION: set up the socket to receive messages
 */

int setupsock()
{	
	int sock, length;
	struct sockaddr_in server;
	struct servent *sp;

	sp = getservbyname("writesrv","tcp");
	if (sp == NULL) {
		perror(MSGSTR(M_MSG_3,"writesrv: unknown service\n"));
		exit(-1);
	}
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0) {
		perror(MSGSTR(M_MSG_4, "opening stream socket") );
		exit(1);
	}
	server.sin_family= AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = sp->s_port;
	if (bind(sock,(struct sockaddr *)&server,sizeof(server))) {
		perror( MSGSTR(E_MSG_5, "binding stream socket") );
		exit(1);
	}
	length = sizeof (server);
	if (getsockname(sock,(struct sockaddr *)&server,&length)) {
		perror( MSGSTR(M_MSG_6, "getting socket name") );
		exit(1);
	}
	listen(sock,5);
	return (sock);
}

/* 
 * NAME: relay
 * FUNCTION:  Opens local tty and relays message
 */

int relay(msgsock)
int msgsock;
{
	char buf[MAX_INPUT];
	char buf2[MAX_INPUT + 1];
	FILE *pFile;
	int iTmp;

	bzero(buf2,sizeof(buf2));
	if (snorm) {         /* refuse service if SIGNORM has been recieved */
		bzero(buf,sizeof(buf));
		sprintf(buf,"%d",NOSERVICE);
		mywrite(msgsock,buf,sizeof(buf));
	}
		
	iTmp = opentty(msgsock, &pFile); 	   /* open user's tty */
	if (iTmp <= 0 || pFile == NULL) {    /* send error number to command */
		bzero(buf,sizeof(buf));
		sprintf(buf,"%d %d",iTmp,0);
		mywrite(msgsock,buf,sizeof(buf)); 
		return(1);
	}
	do {
		bzero(buf,sizeof(buf));
		myread(msgsock,buf,sizeof(buf));    /* get message */

		/* If buffer is full, copy to another buffer
		   to ensure there is a null. */
		if ( strlen (buf) >= MAX_INPUT ) {
			strncpy (buf2, buf, MAX_INPUT);
			fprintf(pFile,"%s",buf2); /* send user message */
		}
		else
			if (buf[0] != '\0')
				fprintf(pFile,"%s",buf); /* send user message */
	} while (buf[0] != '\0');
	fclose(pFile);
	close(msgsock);
	return(0);
}

/*
 * NAME: sigalrm
 * FUNCTON:  catches SIGALRM, if parent has die then child has to die
 */
sigalrm(void)
{
	new_signal(SIGALRM,(void (*)(int))sigalrm);
	if ((int) getppid() == 1)
		exit(-1);
	alarm((unsigned)30);
}
/*
 * NAME: rwrite
 * FUNCTION:  opens local tty, relays and saves message, then waits 
 *      for reply.
 */

int rwrite(msgsock)
int msgsock;
{
	char buf[MAX_INPUT];
	char buf2[MAX_INPUT + 1];
	char name[PATH_MAX];
	int reply,sock;
	FILE *pFile;
	struct message *hd,*line,*prev;
	int iTmp;

	bzero(buf2,sizeof(buf2));
	if (snorm) {         /* refuse service if SIGNORM has been recieved */
		bzero(buf,sizeof(buf));
		sprintf(buf,"%d",NOSERVICE);
		mywrite(msgsock,buf,sizeof(buf));
	}
	reply = -1;
	iTmp = opentty(msgsock, &pFile);     /* open user's tty */
	if (iTmp <= 0 || pFile == NULL){     /* send error number to command */
		bzero(buf,sizeof(buf));
		sprintf(buf,"%d %d",iTmp,0);
		mywrite(msgsock,buf,sizeof(buf)); 
		return(1);
	}
	sock = setupunixsock(handle,name);
	if ( sock < 0 )
		return (sock);
	hd = NULL;
	prev = NULL;
	line = (struct message *) malloc(sizeof(struct message));
	if (line == NULL)
		return(1);
	bzero(line->buf,sizeof(line->buf));
	myread(msgsock,line->buf,sizeof(line->buf));    /* get message header */
	fprintf(pFile,"%s",line->buf); /* send user message header */
	hd = line;
	prev = line;
	line->next = NULL;
	line = (struct message *) malloc(sizeof(struct message));
	if (line == NULL) 
		return(1);
	bzero(line->buf,sizeof(line->buf));
	myread(msgsock,line->buf,sizeof(line->buf)); /* get message header */
	fprintf(pFile,"%s",line->buf);   /* send user message header */
	prev->next = line;
	prev = line;
	line->next = NULL;
	line = (struct message *) malloc(sizeof(struct message));
	if (line == NULL)
		return(1);
	bzero(line->buf,sizeof(line->buf));
	sprintf(line->buf,MSGSTR(M_MSG_18,
		"   [ Use 'write -h %d,%s|%s|%s' to reply ]\n"),
		handle,"ok","cancel","query");
	fprintf(pFile,"%s",line->buf);
	prev->next = line;
	line->next = NULL;
	prev = line;
	do {
		line = (struct message *) malloc(sizeof(struct message));
		if (line == NULL)
			return(1);
		bzero(line->buf,sizeof(line->buf));
		myread(msgsock,line->buf,sizeof(line->buf)); /* get message */

		/* If buffer is full, copy to another buffer
		   to ensure there is a null. */
		if (line->buf[0] != '\0' && (strlen(line->buf) >= MAX_INPUT))
		{
			strncpy(buf2, line->buf, MAX_INPUT);
			fprintf(pFile,"%s",buf2);/* send user message */
		}
		else
			if (line->buf[0] != '\0')
				fprintf(pFile,"%s",line->buf);/* send user message */
		prev->next = line;
		prev = line;
		line->next = NULL;
	} while (line->buf[0] != '\0');
	reply = getreply(sock,hd);	
	sprintf(buf,"%d",reply);
	mywrite(msgsock,buf,sizeof(buf));    /* send reply */
	bzero(buf,sizeof(buf));
	myread(msgsock,buf,sizeof(buf));     /* get acknowledgement */
	unlink(name);              /* delete file */
	fclose(pFile);
	close(msgsock);
	return(reply);
}

/*
 * NAME: hwrite
 * FUNCTION:  Receives endcoded reply and sends it to the awaiting message
 *     unless the reply is equal to 2 then it gets the orginal message
 *     and sends it back over the socket.
 */

int hwrite(msgsock)
int msgsock;
{
	char buf[MAX_INPUT];
	char handles[MAX_INPUT],replys[MAX_INPUT];
	int handle,reply,err;
	
	bzero(buf,sizeof(buf));	
	myread(msgsock,buf,sizeof(buf));	 /* get handle and reply */
	sscanf(buf,"%s%s",handles,replys);
	handle = atoi(handles);
	reply = atoi(replys);
	err = checkhandle(handle);
	if (err == 0) 
		sprintf(buf,"%d",sendreply(handle,reply,msgsock));
	else {
		sprintf(buf,"%d",err);
		mywrite(msgsock,buf,sizeof(buf));       /* send status */
	}
	mywrite(msgsock,buf,sizeof(buf));
	bzero(buf,sizeof(buf));
	myread(msgsock,buf,sizeof(buf));     /* get acknowledgement */
	close(msgsock);
	return(0);
}

/* 
 * NAME: checkhandle
 * FUNCTION:  compares handle with the valid handles in the servies table
 *     if not there return -5
 */
int checkhandle(handle)
int handle;
{
	struct srv *current;
	
	current = head.last;
	while (current != NULL && current->handle != handle)
		current = current->prev;
	if (current != NULL && current->handle == handle) return (0);
	else return(BADHAND);
}

/* 
 * NAME: sendreply
 * FUNCTION:  send reply to the awaiting message. if reply equals 2
 *  then get and send message back over socket.
 */
int sendreply(handle,reply,msgsock)
int handle,reply,msgsock;
{
	int sock,r;
	FILE *f;
	char buf[MAX_INPUT];

	sock = conunixsock(handle);	
	if (sock < 0) return(sock);
	bzero(buf,sizeof(buf));
	sprintf(buf,"%d",reply);
	mywrite(sock,buf,sizeof(buf));     /* send reply to rwrite */
	mywrite(msgsock,buf,sizeof(buf));  /* send status to command */
	if (reply == MQUERY) {
		bzero(buf,sizeof(buf));
		myread(msgsock,buf,sizeof(buf));      /* sync */
		do {   /* get orginal message and rely to command */
			bzero(buf,sizeof(buf));
			myread(sock,buf,sizeof(buf));      /* rwrite */
			mywrite(msgsock,buf,sizeof(buf));  /* command */
		} while (buf[0] != '\0');
	}	
	mywrite(sock,buf,sizeof(buf));    /* sync with rwrite */
	close(sock);
	return(reply);
}

/*
 * NAME: query
 * FUNCTION: send by the socket the list of messages awaiting replies.
 */ 
int query(msgsock)
int msgsock;
{
	char buf[MAX_INPUT];
	struct srv *current;
	char date[NLTBMAX];
	char *date_format;
        char default_fmt[]="%b %e %H:%M";
	
	bzero(buf,sizeof(buf));
	strcpy(buf,HEADER);
	mywrite(msgsock,buf,sizeof(buf));       /* send header */
	bzero(buf,sizeof(buf));
	strcpy(buf,DASHES); 
	mywrite(msgsock,buf,sizeof(buf));       /* send rest of header */
	current = head.first;
	date_format = nl_langinfo(_M_D_RECENT);
	if (date_format == NULL || !*date_format)
		date_format = default_fmt;
	while (current != NULL) {   /* send list */
	  if (current->handle != 0) {
		bzero(buf,sizeof(buf));
		(void) strftime(date, (size_t)NLTBMAX, date_format, &current->tm_time);
		sprintf(buf,"%-8.8s %-8.8s %11d %s\n",
		   current->host, current->service,current->handle,
		   date);
		mywrite(msgsock,buf,sizeof(buf));	
	  }
	  current = current->next;
	}
	buf[0] = '\0';  /* marks end of list */
	mywrite(msgsock,buf,sizeof(buf));	
	bzero(buf,sizeof(buf));
	myread(msgsock,buf,sizeof(buf));	      /* sync */
	close(msgsock);
	return(0);
}

/*
 * NAME: updatesrv
 * FUNCTION: add a new entry to the services table (i.e. entry for 
 *    a message awaiting a reply).
 */
void updatesrv(head,handle,buf,pid)
struct srvhd 	*head;
int		*handle;
char		*buf;
pid_t 		pid;
{
	struct srv *entry;
	time_t timet;
	struct tm *tm_time;

	entry = (struct srv *) malloc (sizeof(struct srv));
	if (entry == NULL) return;
	if (head->first == NULL) {       /* first entry */
		head->first = entry;
		entry->prev = NULL;
	}
	else {                           /* last entry */
		head->last->next = entry;
		entry->prev = head->last;
	}
	entry->next = NULL;		
	head->last = entry;
	strcpy (entry->host,buf+1);           /* host */
	entry->handle = (*handle)++;          /* handle */
	strcpy(entry->service,"write");       /* service */
	timet = time((time_t *) 0);
	tm_time = localtime(&timet);          	
	entry->tm_time = *tm_time;	      /* date */
	entry->pid = pid;                    /* pid */
}

/*
 * NAME: opentty
 * FUNCTION:  find utmp entry and open the target tty
 * RETURN VALUES:     1  OK
 *                    0  user not logged in
 *                   -1  No such tty
 *                   -2  Permission denied
 *                   -3  can't open utmp file
 *                   -4  malloc failed 
 *
 */

int opentty( int msgsock, FILE **pFile )
{
	int i;
	FILE *utmpfile;
	struct utmp *ubuf;
	int found = 0;
	char tty[PATH_MAX+1],tty1[PATH_MAX+1],buf[MAX_INPUT],user[MAX_USERID_LEN+1];
	struct ttys *head = NULL,*current = NULL,*test = NULL;

	bzero(buf,sizeof(buf));
	myread(msgsock,buf,sizeof(buf));     /* get user ID */
	strcpy(user,buf);
	bzero(buf,sizeof(buf));
	myread(msgsock,buf,sizeof(buf));     /* get tty */
	if (buf[0] == '.') buf[0] = '\0';
	strcpy(tty,buf);
	if (user[0] == '-' && tty[0] != '\0') {
		found++;
		if (tty[0] != '/') {
			strcpy(tty1,tty);
			strcpy(tty,"/dev/");
			strcat(tty,tty1);
		} /* end of if */
	} /* end of if */
	else if ((ubuf = getutent()) == NULL) {
		fprintf (stderr, MSGSTR(M_MSG_12, "writesrv: Can't open %s\n") 
							,UTMP_FILE);
		if (tty[0] == '\0'){
			fprintf (stderr, MSGSTR(M_MSG_13,
					 "writesrv: can not continue\n") );
			return(NOOPEN);
		} /* end of if */
	}  /* end of elseif */
	else {
	  while (ubuf != NULL) {
	    if (
		ubuf->ut_type == USER_PROCESS &&
		strncmp(user,ubuf->ut_user,(size_t)MAX_USERID_LEN) == 0
	    )
		if (tty[0] == '\0') {
		    test = (struct ttys *) malloc (sizeof(struct ttys));
		    if (test == NULL) return(MALLOC);
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
		else if (strncmp(tty,ubuf->ut_line,(size_t)MAX_TTY_LEN) == 0) {
			found++;
			break;
		} /* end of else if */
		ubuf = getutent();
	  } /* end of while */
	  endutent();
	} /* end of else */
	if (found == 0) return(NOTLOG);
	if (tty[0] == '\0') 
		strcpy(tty,head->tty);
	if (tty[0] != '/' && tty[0] != '\0') {
		strcpy(tty1,tty);
		strcpy(tty,"/dev/");
		strcat(tty,tty1);
	} /* end of if */
	if (access(tty,F_OK) < 0) {
	    fprintf(stderr,MSGSTR(M_MSG_14,"writesrv: No such tty\n"));
	    return(NOTTY);
	}
	if (access(tty,W_OK) < 0) {
	    fprintf(stderr, MSGSTR(M_MSG_15,"writesrv: Permission denied\n"));
	    return(NOPERM);
  	}
	if ( (*pFile =  fopen(tty,"w")) == NULL) {
	    fprintf(stderr,MSGSTR(M_MSG_16,"writesrv: Permission denied\n"));
	    return(NOPERM);
  	}
	bzero(buf,sizeof(buf));
	sprintf(buf,"%d %d",fileno(*pFile),found);
	mywrite(msgsock,buf,sizeof(buf));    /* send status of open */
	if (found > 1) {                      /* send list of ttys */
		for (i=0;i<found;i++) {
			bzero(buf,sizeof(buf));
			sprintf(buf,"%s",head->tty);
			mywrite(msgsock,buf,sizeof(buf));
			head=head->next;
		}
	}
	return(1);
}

/* 
 * NAME: conunixsock
 * FUNCTION: connect to unix domain socket (this socket allows rwrite and
 *    hwrite to communicate)
 */
int conunixsock(handle)
int handle;
{
	int sock;
	struct sockaddr_un server;	
	
	sock = socket(AF_UNIX,SOCK_STREAM,0);
	if (sock < 0) {
		perror("socket in conunixsock()");
		return(SNDRPLY);
	}
	server.sun_family = AF_UNIX;
	sprintf(server.sun_path,"%s/%d",SDIR,handle);
	if (connect(sock,(struct sockaddr *)&server,sizeof(struct sockaddr_un))
	< 0) {
		close(sock);
		perror("connect() in conunixsock()");
		return(SNDRPLY);
	}
	return(sock);
}

/*
 * NAME: getreply
 * FUNCTION: getreply from hrwite via the unix socket.
 */
int getreply(sock,head)
int sock;
struct message *head;
{
	char buf[MAX_INPUT];
	int reply;
	struct message *current;
	int msgsock,length;
	FILE *f;

	do {
		for (;;) { 
			msgsock = accept(sock,0,0);  /* wait for connection */
			if (msgsock == -1) {
				perror( MSGSTR(E_MSG_20, "accept in getreply") );
				return(GETRPLY);
			} else
				break;
		}
		bzero(buf,sizeof(buf));
		myread(msgsock,buf,sizeof(buf));  /* get reply */
		reply = atoi(buf);
		if (reply == MQUERY) {    /* send orginal message */
			current = head;
			while (current != NULL) {
			    length = strlen(current->buf);
			    mywrite(msgsock,current->buf, ((length==0)?(sizeof(current->buf)):length));
			    current = current->next;
			}
		}
		bzero(buf,sizeof(buf));
		myread(msgsock,buf,sizeof(buf));       /* sync */
		close(msgsock);	        /* close this conncection */
	} while (reply == MQUERY);
	close(sock);                 /* close unix domain socket */
	return(reply);
}
 
/* 
 * NAME: setupunixsock
 * FUNCTION:  sets up the unix domain socket so that rwrite and hwrite 
 *     can communicate.
 * RETURN: -7 if problem with socket
 *        else return socket file descriptor.
 */
int setupunixsock(handle,name)
int handle;
char *name;
{
	struct sockaddr_un server;	
	int sock;
	
	sock = socket(AF_UNIX,SOCK_STREAM,0);
	if (sock < 0) {
		perror("socket() in setupunixsocket()");
		return(GETRPLY);
	}
	server.sun_family = AF_UNIX;
	bzero(name,sizeof(name));
	sprintf(name,"%s/%d",SDIR,handle);
	strcpy(server.sun_path,name);
	if (bind(sock,(struct sockaddr *)&server,sizeof(struct sockaddr_un))
	< 0) {
		close(sock);
		perror("bind() in setupunixsocket()");
		return(GETRPLY);
	}
	listen(sock,5);
	return(sock);
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
	
	do {
		nchar = read(fldes,buf,(unsigned)len);
	} while  (nchar == -1 && EINTR == errno );
	if ((nchar == -1) && (errno != EINTR)) {
	  fprintf(stderr,MSGSTR(M_MSG_19,
		"Can not communicate with write command\n"));
	  perror("read() in myread()");
#ifdef DONT
			/*
			 *  Need to let this continue so that the
			 * sockets will be closed & the files in
			 * /usr/spool/writesrv will be cleaned up
			 */
	  exit(-1);
#endif
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
	  fprintf(stderr,MSGSTR(M_MSG_19,
		"Can not communicate with write command\n"));
	  perror("write() in mywrite()");
	  exit(-1);
	}
}

/*
 * NAME: stdclose
 * FUNCTION:  redirect all file descriptors to /dev/null
 */
void stdclose(argc)
int argc;
{
	int i;

	if (argc == 1) {           /* redirect all file descriptors to  */
		close(0);          /* /dev/null if no args are given as */
		close(1);          /* would be the case when executed   */
		close(2);          /* from /etc/rc.                     */
		i = open("/dev/null",2);
		dup(i);
		dup(i);
	}
}

