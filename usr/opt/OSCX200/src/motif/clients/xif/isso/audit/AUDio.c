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
static char *rcsid = "@(#)$RCSfile: AUDio.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/20 21:31:53 $";
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <Intrinsic.h>

#include "AUD.h"

#define max(a,b) (((a) >= (b)) ? (a) : (b))

#define ERRSTR "** Audit log change to"
#define BUFSIZE       65536
#define ERROR_BUFSIZE 16384

WorkData	work;

/*==========================================================================*/


void
shutdown_audit_tool(data)
WorkDataPtr	data;
{
    /* kill child */
    kill(data->childpid, SIGKILL); 

    /* free buffer */
    if(data->buf)
    {
	free(data->buf);
	data->buf = (char *)NULL;
    } 

    /* shutdown sockets/pipe */
    if(data->listen)
    {
	if ( (shutdown(data->fd1,2) == -1))
	    PostErrorDialog("AUDmsg_err_audit_tool",
			    data->w,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL);
    }
    if((shutdown(data->fd2,2) == -1)
       || (close(data->fd3) == -1) )
      PostErrorDialog("AUDmsg_err_audit_tool",
                      data->w,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL);

    /* let the rest of XIsso know we're done */
    AUDReportFinished();
}


int
s_pipe(fd)
int fd[2];
{
  return( socketpair(AF_UNIX, SOCK_STREAM, 0, fd) );
}

int
exec_audit_tool(fd1, fd2, fd3, instr, listen, file)
int fd1, fd2, fd3, listen;
char *instr, *file;
{
  int fd4, i, count;
  char **argvec, *argvecptr, *ptr, *str;

  /* stdin */
  if ((close(0) == -1) || (dup(fd3) != 0)) {
    return(-1); 
  }

  /* stdout */
  if (file != (char *) NULL)
  {
      if ((fd4 = open(file,O_CREAT|O_WRONLY,0600)) == -1)
      {
	return(-1);
      }
      if ((close(1) == -1) || (dup(fd4) != 1) || (close(fd4) == -1)) {
    	return(-1);
      }
  }
  else if(listen)
  {
      if ((close(1) == -1) || (dup(fd1) != 1)) {
        return(-1);
      }
  }
  else
  {
      if ((close(1) == -1)) {
        return(-1);
      }
  }

  /* stderr */
  if ((close(2) == -1) || (dup(fd2) != 2)) {
    return(-1);
  }
    
  /* close off file descriptors */
  if (fd1 != -1)
  {
    if (close(fd1) == -1) {
      return(-1);
    }
  }

  if ((close(fd2) == -1) || (close(fd3) == -1)) {
    return(-1);
  } 

  /* Prepare argument vector */
  count = 1;
  if ((ptr = strchr(instr,' ')) != NULL) {
    count++;
    ptr++;
  }
  while (*ptr && ((ptr = strchr(ptr,' ')) != NULL)) {
    count++;
    ptr++;
  }

  if (count && ((argvec = (char **) malloc(sizeof(char *) * count)) == NULL ) )
    return(-1);

  i = 0;
  if ( (str = strdup(instr)) == NULL ) {
    return(-1);
  }
  argvecptr = strtok(str," \"");
  if (argvecptr != NULL) {
    if ( (argvec[i] = strdup(argvecptr)) == NULL ) {
      return(-1);
    }
  }
  else 
    if ( (argvec[i] = strdup(instr)) == NULL ) {
      return(-1);
    }
  i++;
  while ( argvecptr && ((argvecptr = strtok((char *)NULL," \"")) != NULL) ) {
    if ( (argvec[i] = strdup(argvecptr)) == NULL ) {
      return(-1);
    }
    i++;
  }
  argvec[i] = (char *)0;

  if (execv(AUDIT_TOOL,argvec) == -1) 
    exit(errno);

  exit(0);
}


int
invoke_audit_tool(app_con, w, command, listenTostdout, fileName, data)
XtAppContext app_con;
Widget w;
char *command;
int  listenTostdout;
char *fileName;
WorkDataPtr *data;
{
  int fd1[2], fd2[2], fd3[2], n, len;
  int sendbuff, recvbuff;
  pid_t childpid;
  char *argstr;

  /* stderr, stdin */
  if ((s_pipe(fd2) == -1) || (pipe(fd3) == -1)) {
    return(-1);
  }    
  
  if (listenTostdout) {
    if (s_pipe(fd1) == -1) {
      return(-1);
    }
  }
  else
  {
    fd1[0] = fd1[1] = -1;
  }

  if ( !(childpid = fork()) ) {

  /* Child */

    if (!listenTostdout)
      if (close(fd1[0]) == -1) {
        exit(errno);
      }
    if ( (close(fd2[0]) == -1) || (close(fd3[1]) == -1) ) {
      exit(errno);
    }
/*    if ( fcntl(fd1[1],F_SETFL,FNONBLOCK) == -1 ) 
      exit(errno);
      if ( fcntl(fd2[1],F_SETFL,FNONBLOCK) == -1 )
      exit(errno);
*/

    sendbuff = BUFSIZE;
    if (setsockopt(fd1[1],SOL_SOCKET,SO_SNDBUF,(char *) &sendbuff, sizeof(sendbuff) ) < 0) 
      exit(errno);

    if(exec_audit_tool(fd1[1],fd2[1],fd3[0],command,listenTostdout,fileName) == -1)
    {
      exit(errno);
    }
    else
    {
      exit(0);
    }
  }
  
  if (childpid != -1)
  {
    /* Parent */
    if (!listenTostdout && fd1[1] != -1) {
      if (close(fd1[1]) == -1) {
        return(-1);
      }
    }
    if ((close(fd2[1]) == -1) || (close(fd3[0]) == -1)) {
      return(-1);
    }

  /* set all data */
  work.fd1 = fd1[0];
  work.fd2 = fd2[0];
  work.fd3 = fd3[1];
  work.childpid = childpid;

  if(listenTostdout) {
    if(fcntl(work.fd1,F_SETFL,FNONBLOCK) == -1)
      return(-1);
    recvbuff = BUFSIZE;
    if (setsockopt(work.fd1,SOL_SOCKET,SO_RCVBUF,(char *) &recvbuff, sizeof(recvbuff) ) < 0) 
      return(-1);
  }
  if(fcntl(work.fd2,F_SETFL,FNONBLOCK) == -1)
    return(-1);
  
  FD_ZERO(&work.readmask);

  work.timeout.tv_sec  = 0L;
  work.timeout.tv_usec = 0L;

  work.listen = listenTostdout;

  if(listenTostdout)
  {
    work.buf = (char *) malloc(BUFSIZE);

    if (work.buf == (char *) NULL) 
      return(-1);

  }
  work.nbytes = 0;
  work.eof    = 0;

  work.app_context = app_con;
  work.w           = w;

  *data = &work; 

  return(0);
  }
}


XtTimerCallbackProc
listen_to_audit_tool(data, id)
WorkDataPtr	data;
XtIntervalId	*id;
{
  int nfds, nread1, nread2, len, status;
  char errors[ERROR_BUFSIZE];
  char *buf;
  pid_t pid;

  nread1 = 0;
  nread2 = 0;
  nfds = 0;

  /* first, find out if we have space to read some data */
  if(data->listen && (data->nbytes < BUFSIZE))
  {
      FD_SET(data->fd1, &data->readmask);
      nfds = data->fd1 + 1;
  }
  FD_SET(data->fd2, &data->readmask);
  nfds = max(nfds, data->fd2 + 1);

  if ( (select(nfds, &data->readmask, (fd_set *)0, (fd_set *)0, &data->timeout)) < 0 ) {
      PostErrorDialog("AUDmsg_err_audit_tool",
                      data->w,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
      
      kill(data->childpid, SIGKILL);
      return;
  }

  /* ship the error messages */ 
  if ( FD_ISSET(data->fd2, &data->readmask) ) {

    if((nread2 = read(data->fd2, errors, ERROR_BUFSIZE-1)) > 0)
    {
	errors[nread2] = '\0';
	AUDHandleIncomingErrorData(errors);

	/* Possible issue if message gets split up on read */
	if ( strstr(errors,ERRSTR) )
	    AUDHandleErrorCondition();
    }
    else if ( nread2 < 0 ) {
      PostErrorDialog("AUDmsg_err_audit_tool",
                      data->w,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
      kill(data->childpid, SIGKILL);
      return;
    }
  }

  /* handle actual report */
  if (data->listen)
  {
    if (((BUFSIZE - data->nbytes) > 0)  &&
       FD_ISSET(data->fd1, &data->readmask))
      nread1 = read(data->fd1, data->buf+data->nbytes, BUFSIZE - data->nbytes);

    if ( nread1 < 0 ) {
      PostErrorDialog("AUDmsg_err_audit_tool",
                        data->w,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL);
      kill(data->childpid, SIGKILL);
      return;
    }
    else
    {
      data->nbytes += nread1;
    }

    if ( data->nbytes )
    {
	int took = AUDHandleIncomingReportData(data->buf, data->nbytes);

	if(took < data->nbytes)
	{
	    /* shift buffer */
	    if((buf = (char *) malloc(BUFSIZE)) == (char *)NULL)
		MemoryError();

	    bcopy((data->buf+took), buf, data->nbytes - took);

	    free(data->buf);
	    data->buf = buf;
	}
	data->nbytes -= took; 
    }
  }

  /* detect if we are at end-of-file */
  if ((((pid = waitpid(data->childpid,&status,WNOHANG)) == -1) && errno == ECHILD) || (pid > 0)) {
    data->eof = 1;
    if ( WIFEXITED(status) && (WEXITSTATUS(status) == ENOENT) ) {
      PostErrorDialog("AUDmsg_err_invoke_audit_tool",
                    data->w,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL); 
    }
  }
  else if (pid < 0) {
    PostErrorDialog("AUDmsg_err_audit_tool",
                    data->w,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
    kill(data->childpid, SIGKILL);
    return;
  }

  if (data->eof && (data->nbytes == 0))
    shutdown_audit_tool(data);
  else
    data->timerID = XtAppAddTimeOut(data->app_context, 
				    AUD_TIME_SLICE,
				    (XtTimerCallbackProc) listen_to_audit_tool,
				    (XtPointer) data);
}


void
write_pathname(pathname,data)
char *pathname;
WorkDataPtr data;
{
  char stop = '\n', *answer;
  int n, len;

  if ( pathname == (char *)NULL )
  {
    answer = &stop;
    len = 1;
  }
  else
  {
    answer = (char *) malloc((len = strlen(pathname)) + 1);
    strcpy(answer, pathname);
    answer[len] = '\n';
  }

  n = write(data->fd3,answer,len+1);
  if ( n < 0 || n != len+1) {
    PostErrorDialog("AUDmsg_err_audit_tool",
                    data->w,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
    kill(data->childpid, SIGKILL);
    return;
  }

  if (pathname == (char *) NULL)
    shutdown_audit_tool(data);
  else
    free(answer);
}


