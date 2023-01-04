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
/*LINTLIBRARY*/

#ifndef lint
static char *sccsid = "@(#)$RCSfile: capsar_take_whole.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:04:44 $";
#endif lint

/*
 * capsar_take_whole.c  01/14/88
 * Lynn C Wood,
 *
 * This routine is provides as part of the capsar library for handling
 * of Multiple Body Part Messages in Ultrix Mail.
 * 
 * syntax : MM *capsar_take_whole ( int fd )
 *
 */

/*	Generic Includes 
*/
#include <stdio.h>
#include <locale.h> /*GAG*/
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <syslog.h>

/*	Capsar Specific Includes
*/
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

int	HIGH_TIDE_MARK;

MM *capsar_take_whole(fd)
int	fd;		/* file descriptor for flat representation */
{
/*------*\
  Locals
\*------*/

	struct	stat	stbuf;	/* for holding fstat information */
	struct	rlimit	rlp;	/* resource limit */
	int		mode;	/* returned from fstat */
	int	socket_flag=0;	/* socket flag */
	int	pipe_flag=0;	/* descriptor points to pipe */
	int	soffset=0;	/* lseek offset */
	int	oldoffset;	/* lseek offset */
	int	tm=0;		/* tide mark */
	MM	*m,*mm1,*mmp;	/* mail structure */
	FILE	*pfd;		/* file pointer */
	char	tmpname[BUFSIZ];/* tempory file name */

	char	*malloc();

	(void) getrlimit(RLIMIT_CORE,&rlp);
	HIGH_TIDE_MARK = rlp.rlim_max*TIDE_MARK_RATIO;

	/* get file status from argument descriptor */	
	mode = fstat(fd, &stbuf);

	if(mode < 0){
		switch (errno) {
			case EBADF:
				(void) capsar_log(LOG_INFO,M_MSG_58,"not valid file descriptor\n",NULL); /*GAG*/
				return(NULL);
			case EFAULT:
				(void) capsar_log(LOG_INFO,M_MSG_59,"file descriptor points to an invalid address\n",NULL); /*GAG*/
				return(NULL);
			case EIO:
				(void) capsar_log(LOG_INFO,M_MSG_60,"I/O error on read/write\n",NULL); /*GAG*/
				return(NULL);
			case EOPNOTSUPP:
				socket_flag++;
			case ETIMEDOUT:
				(void) capsar_log(LOG_INFO,M_MSG_61,"connect request or remote file operation failed\n",NULL); /*GAG*/
				return(NULL);
			default:
				(void) capsar_log(LOG_INFO,M_MSG_62,"cannot stat file descriptor\n",NULL); /*GAG*/
				return(NULL);
		}/*E switch(errno) */
	}/* E if i < 0 */

	if(stbuf.st_mode == 0)pipe_flag=1;

	/* if descriptor points to a socket or pipe 
         * file is copied into a temporary location
	 */
	if(socket_flag || pipe_flag){
		if((fd = copytmp(fd,tmpname,NOTSMTP)) == NOTOK){
			(void)capsar_log(LOG_INFO,M_MSG_77,"error copying data to temporary file\n",NULL); /*GAG*/
			return(NULL);	
		}
	}

	if((pfd = fdopen(fd,"r")) == NULL){
		(void)capsar_log(LOG_INFO,M_MSG_65,"error in opening %s \n",tmpname); /*GAG*/
		return(NULL);
	}
	oldoffset = ftell(pfd);

	if((stbuf.st_mode & S_IFMT) != S_IFREG){
		(void) capsar_log(LOG_INFO,M_MSG_78,"illegal file descriptor\n"); /*GAG*/
		return(NULL);
	}
	
	/* now do the message descapsulation */

	/* allocate mail structure */
	m = (MM *)malloc ((unsigned) sizeof *m);
	if(m == NULL) {
		(void) capsar_log(LOG_INFO,M_MSG_11,"unable to allocate mail structure\n",NULL); /*GAG*/
		return(NULL);
	}
	capsar_setup(m);
	mm1 = m;	
	m->message_type = MAIL_MESSAGE;
	
	mmp = m;

	/* at this point have either found the first encapsulation
         * boundery or end of flattened message
 	 */
	m->size = stbuf.st_size-soffset ;
	m->flat_fd = fd;
	m->offset = 0;
	m->complete = 2;

	/* allocate enough space for mail data
	 * and read in text from flattened form 
	 */
	
	if((tm = writemm(pfd,m,m->size,soffset,tm,fd)) == NOTOK)
		return(NULL);

	(void) fseek(pfd,oldoffset,0);
	m->message_type = MAIL_MESSAGE;
	m->body_type = capsar_get_body_type(m);
	
	return(mm1);
}
