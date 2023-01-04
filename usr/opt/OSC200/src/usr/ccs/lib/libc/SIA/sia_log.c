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
static char *rcsid = "@(#)$RCSfile: sia_log.c,v $ $Revision: 1.1.15.4 $ (DEC) $Date: 1993/10/19 22:24:00 $";
#endif
/*****************************************************************************
* Usage:  int sia_log(va_alist)
*
* Description: The purpose of this routine is to log events, and error 
* messages into the SIALOG file. Each entry is time and PID stamped to 
* track when and who was making the log entry. There are three types of
* log messages which sia_log supports:
*					
*		SIALOGEVENT - Log an event (not an error or problem)
*		SIALOGERROR - Log an error or problem (not too serious)
*		SIALOGALERT - Log a serious problem (SIA is SERIOUSLY ILL)
*
* Parameter Descriptions:  
*			
*	Param1: loglevel
*	Usage: SIALOGEVENT, SIALOGERROR, SIALOGALERT
*
*	Param2: siafmt 
*	Usage: One of the SIA message formats in sia.h
*
*	Param3: va_alist
*	Usage: list of argument to be printed to the log
*
*
* Assumed Inputs: none
*
* Return Values:
*
* Success return: SIASUCCESS 
*
* Failure return: SIAFAIL 
*
* Error Conditions and returns otherthan parameter syntax checks listed above:
*
* 	Condition1: Unable to open log file
*	Return: SIAFAIL
*
*	Condition2: Unable to append to log file.
*	Return:	SIAFAIL
*
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_audit = __sia_audit
#pragma weak sia_log = __sia_log
#pragma weak sia_warning = __sia_warning
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"
/***#ifdef  __STDC__
***#include <stdarg.h>
***#else
***#include <varargs.h>
***#endif */
#include <varargs.h>

int 
  sia_log (va_alist)
va_dcl
{	
	/*SIALOGLOCK*/
	va_list ap;
	char *loglevel;
	const char *siafmt;
	time_t thetime=0;
	time_t time();
    	char *strtime;
	FILE *sialog = NULL;
	int logfd;
	int status;

#if 0	/* this is a prime example of what not to do */
	if(geteuid() != 0) 	/* sufficiently privilege? */
		return(SIAFAIL);
#endif
	logfd = open(SIALOGFILE, O_WRONLY|O_APPEND, 0);
	if (logfd < 0) {
		if (errno==EPERM || errno==EACCES)
			return SIAFAIL;
		if (errno==ENOENT)
			return SIASUCCESS;
		/* perror(SIALOGFILE); */
		return SIAFAIL;
	}
	sialog = fdopen(logfd, "a");
	if(sialog == NULL)  /* open or creat log file for appends */
		{
		/* fprintf(stderr,"unable to open sialog\n"); */
		(void) close(logfd);
		return(SIAFAIL);
		}
	(void) fcntl(logfd, F_SETFD, FD_CLOEXEC);	/* close-on-exec */
	if(lockf(logfd, F_LOCK, 0))	/* exclusive lock the log file for single threaded appends */
		{
		/* fprintf(stderr,"unable to lock sialog\n"); */
		(void) fclose(sialog);
		return(SIAFAIL);
		}
	/****** Now we are single threaded ******/
    	(void) time(&thetime);
    	strtime = ctime(&thetime);
	va_start(ap);
	loglevel = va_arg(ap, char *);
	siafmt = va_arg(ap, char *);
	(void) fprintf(sialog, SIALOGENTFMT,  loglevel, strtime);
	(void) vfprintf(sialog, siafmt, ap);
	va_end(ap);
	/****** flush and close to assure that the log entry gets to disk ******/
	status = (fflush(sialog) < 0) | (fsync(logfd) < 0)
		 | (fclose(sialog) < 0);
	if (status)
		return SIAFAIL;
	return SIASUCCESS;
	/*SIALOGREL*/
}

sia_warning(va_alist)
va_dcl
{
	va_list ap;
	int (*collect)();
	char msg[513], *fmt;
	prompt_t	warnprompt;
	typedef int (*PFI)();

	va_start(ap);
	collect = va_arg(ap, PFI);
	fmt = va_arg(ap, char *);
	if(collect == NULL)
		return(SIADFAIL);
	vsprintf(msg, fmt, ap);
	va_end(ap);
	warnprompt.prompt= (unsigned char *) msg;
	warnprompt.result=NULL;
	warnprompt.max_result_length=0;
	warnprompt.min_result_length=0;
	(*collect)(0,SIAWARNING,"",0,&warnprompt);
	bzero(msg,strlen(msg));
	return(SIADSUCCESS);
}


/* variable format front-end for audgen(2) */
sia_audit(event, va_alist)
u_int event;
va_dcl
{
	va_list ap;
	char tokenp[N_AUDTUPLES];
	char *audargv[N_AUDTUPLES];
	int i;
	
	va_start(ap);
	for (i = 0;  i < N_AUDTUPLES;  i++) {
		if ((tokenp[i] = va_arg(ap, char)) == 0) break;
		if (A_TOKEN_PTR(tokenp[i])) audargv[i] = va_arg(ap, char *);
		else audargv[i] = (char *) va_arg(ap, int);
	}
	va_end(ap);
	
	i = audgen(event, tokenp, audargv, NULL, NULL);
	return ((i < 0) && (errno != ENOSYS)) ? SIAFAIL : SIASUCCESS;
}
