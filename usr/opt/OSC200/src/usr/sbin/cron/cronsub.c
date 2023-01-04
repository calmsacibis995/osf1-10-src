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
static char rcsid[] = "@(#)$RCSfile: cronsub.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/09/03 03:25:47 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME:  (CMDOPER) - cronsub.c
 *
 *
 * Copyright International Business Machines Corp. 1988
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * 
 *  1.15  com/cmd/cntl/cron/cronsub.c, , bos320, 9134320c 8/18/91 20:42:32
 * cronsub.c	4.1 10:17:10 7/12/90 SecureWare 
 */                                                                   

#include <sys/secdefines.h>

#include <sys/types.h>
#include <sys/param.h>        /* for BSIZE needed in dirent.h */
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <langinfo.h>
#include <ctype.h>
#include <nl_types.h>
#include "cron.h"

#include "cron_msg.h"
nl_catd catd;
#define	MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)

#define ATRMDEL		"at: only jobs belonging to user : %s may be deleted\n"
#define ATCANTCD	"at: can't change directory to the at directory\n"
#define CRCANTCD   "crontab: can't change directory to the crontab directory\n"
#define BADOPEN		"crontab: can't open your crontab file.\n"
#define CRNOREADDIR	"crontab: can't read the crontabs directory\n"
#define ATNOREADDIR	"at: can't read the atjobs directory\n"
#define BADSTAT		"crontab: stat failed\n"
#define BADUNLINK	"crontab: unlink of crontab file failed\n"
#define BADATUNLINK	"at: unlink of atjob file failed\n"
#define BADSCANDIR	"at: scandir of /var/spool/cron/atjobs failed\n"
#define BADJOBOPEN	"at: unable to read your at job.\n"
#define NONEXIST	"at: Job name %s does not exist.\n"
#define NOTOWNER	"at: You must be the owner of job %s.\n"

#define	TIMESIZE	256

struct dirent *dp;
DIR *dir;
char *getfilenam();
int list_aj();
int list_cj();
int remove_cj();
int remove_aj();
int send_msg(char, char, char *, char *);


/*
 * NAME: rm_at_jobs
 *                                                                    
 * FUNCTION: remove at jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function will find the specified job and delete it.
 * 	It will also send a mesage to cron.
 * 
 * (NOTES:) flag : CRONUSER (If not then name is job name)
 *	           CRON_PROMPT prompt when the job is removed.
 *		   CRON_QUIET  not prompt anythig when the job is removed.
 * 	    ujname :  jobname or user name .
 *	             not support now:(if NULL then delete all of the at jobs)
 *
 * RETURNS: 0 - successful or 1 - (BADCD), 2 - (BADJOBOPEN), 3 - (BADUNLINK)
 *	    4 - (NONEXIST), 5-(NOTOWNER)
 *          exit(1) ATRMDEL
 */  

rm_at_jobs(flags,ujname)
int flags;
char *ujname;
{
	FILE *fp;
	char *uname;
	time_t t, num();
	uid_t uid;
	char *ptr;
	struct passwd *pwinfo;
	struct stat buf;
	int rstat;
	static char dummy_string[1]; 	/* pointer to null string */

	if (ujname == NULL)
		ujname = dummy_string;	/* avoid core dump of function strcmp */

#if SEC_BASE && defined(BUILDING_AT)
	uid = getluid();
#else
	uid = getuid();
#endif
        pwinfo = getpwuid(uid);

        /* permission check */
#if SEC_BASE && defined(BUILDING_AT)
	if (!at_authorized() && (strcmp(ujname,pwinfo->pw_name))
#else
        if (uid && (strcmp(ujname,pwinfo->pw_name))
#endif
                && (strcmp(getfilenam(ujname),pwinfo->pw_name))) {
                        /* don't have permission to remove file(s) */
                fprintf(stderr,MSGSTR(MS_ATRMDEL,ATRMDEL),pwinfo->pw_name);
                exit(1);
        }
	if (!strcmp(ujname,".") || !strcmp(ujname,".."))
		return(3);

        /* permission granted  !!! pass through !!! */
	if (chdir(ATDIR)==-1) 
		return(1);
	if (flags & CRON_USER) {  /* remove all of the jobs of user */
		if((dir=opendir(ATDIR)) == NULL)
			return(2);
		while ((dp = readdir(dir)) != NULL ) {
			if (!(strcmp(getfilenam(dp->d_name),ujname)))  {
				if (flags & CRON_PROMPT)
					if (prompt_del(dp->d_name)==1)
						continue;
				if (unlink(dp->d_name) != 0) {
					return(3);
				}
				rstat = send_msg(AT,DELETE,dp->d_name,dp->d_name);
				if (!rstat && !(flags & CRON_QUIET))
					printf(MSGSTR(MS_ATDELTD,
					"at file: %s deleted\n"), dp->d_name);
			}
		}
		closedir(dir);
	}
	else {		/* remove a job */
		if (ujname[0] != NULL) {
			if (stat(ujname, &buf))
				return(4);	/* job file does not exist */
#if SEC_BASE && defined(BUILDING_AT)
			if ((uid = getluid()) != buf.st_uid && !at_authorized())
#else
			if (((uid=getuid()) != buf.st_uid) && (uid != ROOT))
#endif
				return(5);	/* they don't own this one... */
 			/* should we prompt for confirmation */
			if ((flags & CRON_PROMPT) && (prompt_del(ujname)!=0))
				return(0);
			else {
				if (unlink(ujname) != 0)  {
					return(3);
				}
				rstat = send_msg(AT, DELETE,ujname,ujname);
				if (!rstat && !(flags & CRON_QUIET))
					printf(MSGSTR(MS_ATDELTD,"at file: %s deleted\n"), ujname);
			}
		}
		else { /* root is the only one to get this far */
                        if((dir=opendir(ATDIR)) == NULL)
                                return(2);
                        while ((dp = readdir(dir)) != NULL ) {
                                uname = getfilenam(dp->d_name);

                                if (*uname == '\0')
                                        continue;
                                if (flags & CRON_PROMPT)
                                         if (prompt_del(dp->d_name)!=0)
                                                continue;
                                if (unlink(dp->d_name) != 0)  {
                                        return(3);
                                }
                                rstat=send_msg(AT,DELETE,dp->d_name,dp->d_name);
                                if (!rstat && !(flags & CRON_QUIET))
                                        printf(MSGSTR(MS_ATDELTD,"at file: %s deleted\n"), dp->d_name);
                        }
                        closedir(dir);
		}
	}
}

/*
 * NAME: prompt_del
 *                                                                    
 * FUNCTION: verify file deletion
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function will prompt the user to make sure he really wants
 *	to delete the file, user responds yes or no.
 *                                                                   
 * RETURNS: 0 - yes or 1 - no
 */  


prompt_del(user)
char *user;
{
	char	ans[BUFSIZ];
	char	*ystr;

	ystr = MSGSTR(MS_YESLOW, "y");

	printf(MSGSTR(MS_DELETE,"delete %s? (%s for yes) "), user, ystr);

	gets(ans);

	/* if EOF or newline, be safe and forget the whole thing */
	if ((ans == NULL) || (ans[0] == '\0'))
		return(1);

	/* else normal check */
	if (rpmatch(ans) == 1)
		return(0);
	else 
		return(1);

}

/*
 * NAME: ls_at_jobs
 *                                                                    
 * FUNCTION: list cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function will list all files for a user or administrator in
 *	the /usr/spool/cron/atjobs directory.
 *
 * (NOTES:) flag : CRON_SORT_E , CRON_SORT_M ,CRON_COUNT_JOBS, AT_JOB_ID
 *                                                                   
 * RETURNS: 0 - successful or 1 - BADCD, 2 - NOREADDIR, 3 - BADSCANDIR
 */  

ls_at_jobs(flags, user, lq)
int flags;
char *user;
char lq;
{
	time_t t, num();
	int count=0;
	int i,alphasort();		/* sort jobs by date of execution */
	int numentries;
	struct dirent **queue;		/* the queue itself */
	char *timeptr, *qptr;		/* time and queue in filename */
	char	timestr[TIMESIZE];	/* array to hold formatted date */
	struct stat buf;

	if (chdir(ATDIR)==-1) 
		return(1); 

	if (flags & AT_JOB_ID) { /* list a single job, user = job id */
	  	if (user == NULL) 
			return(0);  /* no job can be found without a job id*/
		if (stat(user, &buf))
			return(0);  /* no job found */
		if ((timeptr = (strchr(user,'.'))) == NULL)
			return(0);  /* wrong job name format */
		++timeptr;
		t = num(&timeptr);
		(void)strftime(timestr,TIMESIZE,
				nl_langinfo(D_T_FMT), localtime(&t));
		printf("%s\t%s\n",user, timestr);
		return(0);
	} else if (flags & CRON_SORT_E) { /* sort in order of submission */
		if ((numentries = scandir(".",&queue,NULL,NULL)) < 0) 
			return(3);
	}
	else {				/* sort in order of execution */
		if ((numentries = scandir(".",&queue,NULL,alphasort)) < 0) 
			return(3);
	}

	/* 
	 * a valid job filename looks like this:
	 *    <userid>.<runtime>.<queue>
	 */
	for (i=0;numentries>i;i++)  {
		if (!strcmp(queue[i]->d_name,"."))
			continue;
		if (!strcmp(queue[i]->d_name,".."))
			continue;
		/* all valid at job filenames must contain 2 .'s */
		if ((timeptr = (strchr(queue[i]->d_name,'.'))) == NULL)
			continue;
		if ((qptr = strchr(++timeptr,'.')) == NULL)
			continue;

		/* is this the correct user and queue */
		if ((user==NULL) || 
		   !(strcmp(getfilenam(queue[i]->d_name),user)))  {
			if (lq && lq != *(++qptr))
				continue;	    /* not in right queue */
			count++;
			if (flags & CRON_COUNT_JOBS)
				continue;
			t = num(&timeptr);
			(void)strftime(timestr,TIMESIZE,
					nl_langinfo(D_T_FMT), localtime(&t));
			printf("%s\t%s\n",queue[i]->d_name, timestr);
		}
	}
	if (flags & CRON_COUNT_JOBS)
		printf(MSGSTR(MS_CRONCNT,"%d files in the queue\n"),count);
	return(0);
}

/*
 * NAME: ls_cron_tabs
 *                                                                    
 * FUNCTION: list cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function will find the specified job and delete it.
 *                                                                   
 * RETURNS: 0 - successful or 1 - failure
 */  

ls_cron_tabs(flags,user)
int flags;
char *user;
{
	FILE *fp;
	char line[CTLINESIZE];
	struct stat buf;
	int found=0;
	char timestr[TIMESIZE];		/* buffer to hold nls date */

	if (chdir(CRONDIR)==-1) 
		return(1); 
	if((dir=opendir(CRONDIR)) == NULL)
		return(2);
	while ((dp = readdir(dir)) != NULL ) {
		if (!strcmp(dp->d_name,"..") || !strcmp(dp->d_name,"."))
			continue;
		if ((user==NULL) || !(strcmp(dp->d_name,user))) {
			found++;
			if (flags & CRON_VERBOSE) {
				if ((fp = fopen(dp->d_name,"r")) == NULL)
					return(3);
				if (user == NULL)
					fprintf(stdout,MSGSTR(MS_FILENM,"\ncrontab: filename %s\n"), dp->d_name);
				while(fgets(line,CTLINESIZE,fp) != NULL) {
					fputs(line,stdout);
				}
				fclose(fp);
			}
			else {
				if (stat(dp->d_name,&buf))
					return(4);
				else {
			(void)strftime(timestr, TIMESIZE, 
				nl_langinfo(D_T_FMT), localtime(&buf.st_mtime));
			printf(MSGSTR(MS_SUBTIME,"crontab file: %s\tsubmission time: %s\n"),dp->d_name, timestr);
				}
			}
		}
	}
	closedir(dir);
	if ((!found) && (user != NULL)) 
		return(3);
	return(0);
}

/*
 * NAME: rm_cron_tabs
 *                                                                    
 * FUNCTION: remove cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function will find the specified job and delete it.
 *	The administrator can delete anyone's job
 *                                                                   
 * RETURNS: 0 - successful or 1 - failure
 */  

rm_cron_tabs(flags,user)
int flags;
char *user;
{
	int rstat;

	if (chdir(CRONDIR)==-1) 
		return(1);

	if (unlink(user) != 0) 
		return(2);

	rstat = send_msg(CRON,DELETE,user,user);
	if (!rstat && !(flags & CRON_QUIET))
		printf(MSGSTR(MS_CRDELETD,"crontab file: %s deleted\n"),user);
	return(0);
}

/*
 * NAME: getfilenam
 *                                                                    
 * FUNCTION: parses the file name to return just the chars before 1st '.'
 *                                                                    
 * EXECUTION ENVIRONMENT:
 * 		sets the user name to the static variable tmpptr.
 *                                                                   
 * RETURNS: pointer to the array that contains the user name.
 */  

static char tmpptr[UNAMESIZE];

char *getfilenam(base)
char *base;
{
	strcpy(tmpptr,base);
	if ((strchr(tmpptr, '.')) != NULL)
		* (strchr(tmpptr, '.')) = '\0';
	return(tmpptr);
}

/*
 * NAME: list_cj
 *                                                                    
 * FUNCTION: list cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *      call ls_at_jobs and check the exit code.
 *                                                                   
 * (NOTES:) flag : CRON_VERVOSE -l list crontab file contents.
 *	           CRON_NON_VERBOSE -v list the information of the file.
 * 		      -l and -v option chnged because of the POSIX conformance.
 * 	    user : user name , if NULL then list all of the files.
 *
 * RETURNS: 0 - successful or exit 1 - failure
 */  
list_cj(flags,user)
int flags;
char *user;
{
	switch(ls_cron_tabs(flags,user)) {
	   case 1: 
		fprintf(stderr,(MSGSTR(MS_CRCANTCD, CRCANTCD)));
		exit(1); 
	   case 2: 
		fprintf(stderr, (MSGSTR(MS_CRNOREADDIR, CRNOREADDIR)));
		exit(1); 
 	   case 3: 
		fprintf(stderr,(MSGSTR(MS_BADOPEN, BADOPEN)));
		exit(1); 
	   case 4: 
		fprintf(stderr, (MSGSTR(MS_BADSTAT, BADSTAT)));
		exit(1); 
	}
	return(0); 
}

/*
 * NAME: remove_cj
 *
 * FUNCTION: remove cron jobs for a user or the administrator
 *
 * EXECUTION ENVIRONMENT:
 *      This function will find the specified job and delete it.
 *      The administrator can delete anyone's job
 *      call ls_at_jobs and check the exit code.
 *
 * (NOTES:) flag : CRON_QUIET and others
 *
 * RETURNS: 0 - successful or exit 1 - failure
*/

remove_cj(flags,user)
char *user;
{
	switch(rm_cron_tabs(flags,user)) {
	   case 1: 
		fprintf(stderr,(MSGSTR(MS_CRCANTCD, CRCANTCD)));
		exit(1); 
	   case 2: 
	       fprintf(stderr,(MSGSTR(MS_BADUNLINK,BADUNLINK)));
		exit(1); 
	}
	return(0); 
}
		

/*
 * NAME: list_aj
 *
 * FUNCTION: list at jobs for a user or the administrator
 *
 * EXECUTION ENVIRONMENT:
 *      call ls_at_jobs and check the exit code.
 *
 * (NOTES:) flag : CRON_SORT_E , CRON_SORT_M ,CRON_COUNT_JOBS
 *                 CRON_USER (ujname is user name  else
 *                              ujname is a job name  if NULL then liat all)
 * RETURNS: 0 - successful or  exit 1 - failure
 */

list_aj(flags,user,queue)
int flags;
char *user;
char queue;
{
	switch(ls_at_jobs(flags,user, queue)) {
	   case 1: 
		fprintf(stderr,(MSGSTR(MS_ATCANTCD, ATCANTCD)));
		exit(1);
	   case 2: 
		fprintf(stderr,(MSGSTR(MS_ATNOREADDIR, ATNOREADDIR)));
		exit(1);
	   case 3: 
		fprintf(stderr,(MSGSTR(MS_BADSCANDIR, BADSCANDIR)));
		exit(1);
	}
	return(0);
}

/*
 * NAME: remove_aj
 *
 * FUNCTION: remove at jobs for a user or the administrator
 *
 * EXECUTION ENVIRONMENT:
 *           call rm_at_jobs and check the exit code.
 *
 * (NOTES:) flag : CRONUSER (If not then name is job name)
 *                 CRON_PROMPT prompt when the job is removed.
 *                 CRON_QUIET  not prompt anythig when the job is removed.
 *          ujname : if NULL then delete all of the at jobs by root user
 *
 * RETURNS: 0 - successful or exit 1 - failure
 */

remove_aj(flags,user)
int flags;
char *user;
{
	switch(rm_at_jobs(flags,user)) {
	   case 1: 
		fprintf(stderr,(MSGSTR(MS_ATCANTCD, ATCANTCD)));
		exit(1); 
	   case 2: 
	        fprintf(stderr,(MSGSTR(MS_BADJOBOPEN,BADJOBOPEN)));
		exit(1); 
	   case 3: 
	        fprintf(stderr,(MSGSTR(MS_BADATUNLINK,BADATUNLINK)));
		exit(1); 
	   case 4:
		fprintf(stderr, (MSGSTR(MS_NONEXIST, NONEXIST)), user);
		exit(1);
	   case 5:
		fprintf(stderr, (MSGSTR(MS_NOTOWNER, NOTOWNER)), user);
		exit(1);
	}
	return(0); 
}

/*
 * NAME: send_msg
 *                                                                    
 * FUNCTION: Send message to cron daemon
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) etype   : event type CRON or AT
 *	    action  : request action DELETE
 *	    fname   : file name (username  or jobname(at))
 *	    logname : user name
 *
 * RETURNS: 0 - successful or 1 - failure
 */  

int
send_msg(char etype, char action, char *fname, char *logname)
{

	static	int	msgfd = -2;
	struct	message	*pmsg;

#if SEC_MAC || SEC_NCAV
	pmsg = (struct message *) crontab_set_message(sizeof *pmsg);
#else
	pmsg = &msgbuf;
#endif
	if(msgfd == -2)
		if((msgfd = open(FIFO,O_WRONLY|O_NDELAY)) < 0) {
			if(errno == ENXIO || errno == ENOENT)
				fprintf(stderr,MSGSTR(MS_NOCRON, "cron may not be running - call your system administrator\n"));
			else
				fprintf(stderr,MSGSTR(MS_MSGQERROR, "error in message queue open\n"));
			return(1);
		}
	pmsg->etype = etype;
	pmsg->action = action;
	strncpy(pmsg->fname,fname,(size_t)FLEN);
	strncpy(pmsg->logname,logname,(size_t)LLEN);
#if SEC_MAC || SEC_NCAV
	if (!crontab_write_message(msgfd, (char *) pmsg, sizeof *pmsg)) {
#else
	if(write(msgfd,(char *)pmsg,sizeof(struct message)) != sizeof(struct message)) {
#endif
		 fprintf(stderr,MSGSTR(MS_MSGSERROR, 
				"error in message send\n"));
		 return(1);
	}
	return(0);
	
}

