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
static char *sccsid = "@(#)$RCSfile: capsar_unparse_file.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/12/21 15:01:12 $";
#endif lint

/*
 * capsar_unparse_file.c  01/19/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail.
 *
 * syntax : (void) capsar_unparse_file( MM *m; int fd )
 *
 * This routine writes a flat representation of the message to
 * the file descriptor fd.
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
#include <syslog.h>

/*	Capsar specific includes
 */
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

capsar_unparse_file(m,fd)
MM	*m;		/* message structure */
int	fd;		/* file descriptor */
{
/*------*\
  Locals
\*------*/

	struct 	stat	stbuf;	/* holds fstat info */
	int		mode;	/* returned from fstat */
	MM		*do_flat();
	
	/* get file status from argument descriptor */	
	mode = fstat(fd, &stbuf);

	if(mode < 0){
		switch (errno) {
			case EBADF:
				capsar_log(LOG_INFO,M_MSG_58,"not valid file descriptor\n",NULL); /*GAG*/
				return(NOTOK);
			case EFAULT:
				capsar_log(LOG_INFO,M_MSG_59,"file descriptor points to an invalid address\n",NULL); /*GAG*/
				return(NOTOK);
			case EIO:
				capsar_log(LOG_INFO,M_MSG_60,"I/O error on read/write\n",NULL); /*GAG*/
				return(NOTOK);
			case EOPNOTSUPP:
				capsar_log(LOG_INFO,M_MSG_79,"socket descriptor\n",NULL); /*GAG*/
				return(NOTOK);
			case ETIMEDOUT:
				capsar_log(LOG_INFO,M_MSG_61,"connect request or remote file operation failed\n",NULL); /*GAG*/
				return(NOTOK);
			default:
				capsar_log(LOG_INFO,M_MSG_62,"cannot stat file descriptor\n",NULL); /*GAG*/
				return(NOTOK);
		}/*E switch(errno) */
	}/* E if i < 0 */

	/*if((stbuf.st_mode & S_IFMT) != S_IFREG){
		capsar_log(LOG_INFO,M_MSG_78,"illegal file descriptor\n");
		return(NOTOK);
	}*/


	
	
	/* go an unparse message structure */
	
	return do_flat(m,fd,-1) == NULL? NOTOK: OK;
}

/*
 * MM *do_flat( MM *m,fd,level)
 *
 * Thus routine is a recursive function which unparse a message
 * structure into its flattened form
 *
 */
MM *do_flat(m,fd,level)
MM	*m;	/* message structure */
int	fd;	/* file descriptor */
int	level;	/* encapsulation level */
{
	MM	*mp;	/* parent pointer */
	MM	*mptmp,*mt;
	char	*ebs;
	char	*ebmake();
	char	*strcat();

	mp = m->parent;
	ebs = NULL;
	
	while(m != NULL){
	if(level >=0 && m->parent !=mp)break;

	switch(m->message_type){
		case EMPTY :
			if(level < 0)level=0;
			break;
		case SIMPLE_MESSAGE :
			if(level < 0)level=0;
			ebs = ebmake(level);
			if(!ebs)return(NULL);
			puttag(fd,ebs,m->start);
			if(getmm(fd,m) == NOTOK)
				return(NULL);
			puttag(fd,ebs,m->stop);
			free(ebs);
			break;
		case COMPOUND_MESSAGE :
		case MULTIPLE_FORMAT :
		case MAIL_MESSAGE:
			if(level >= 0){
				ebs = ebmake(level);
				if(!ebs)return(NULL);
				puttag(fd,ebs,m->start);
				free(ebs);
			}
			if(getmm(fd,m) == NOTOK)
				return(NULL);
			mptmp=m;
			if(m->mm_next == NULL){
				ebs = ebmake(level);
				if(!ebs)return(NULL);
				puttag(fd,ebs,m->stop);
				free(ebs);
				return(m);
			}
			m = do_flat(m->mm_next,fd,level+1);
			if(!m)return(NULL);
			mt=m->parent;
			while(mt !=mptmp){
				if(mt->parent == NULL)return(m);
				mt= mt->parent;
			}
			if(level >= 0){
				ebs = ebmake(level);
				if(!ebs)return(NULL);
			}
			if(mt->message_type == MAIL_MESSAGE && level < 0)
				/* noop */;
			else puttag(fd,ebs,mt->stop);
			if(ebs)free(ebs);
			else ebs = NULL;
			break;
	}
	mptmp = m->mm_next;
	if(mptmp == NULL || mptmp->parent !=mp)return(m);
	m = m->mm_next;
	}
	return(m);
}

/*
 * create the encapsulation string 
 *
 */
char *ebmake(level)
int	level;
{
	char	*ebs,*eb0;
	int	n,i;
	char	*malloc();
#define NO_DASH 10
#define MAX_DASH 25
	
	if(level < 0)return(NULL);
	if((2*level+1) < NO_DASH-1)n =NO_DASH; 	
	else if((2*level-1) < MAX_DASH)n = NO_DASH + (2*level+1);
	else return(NULL);
	ebs = (char *)malloc((unsigned)(n+1));
	if(ebs == NULL){
		capsar_log(LOG_INFO,M_MSG_80,"cannot allocate space for eb\n",NULL); /*GAG*/
		return(NULL);
	}

	eb0 = ebs;

	while(n-- >=0)*eb0++ = '-';
	eb0 = '\0';

	if(level > 0){
		for(i=0;i<level;i++)
			*(ebs+2*i+1) = ' ';
	}
	
	return(ebs);
}

getmm(fd,m)
int	fd;
MM	*m;
{
	char	buf[BUFSIZ];
	int	fd1,n;
	
	if(m->size <= 0)return(OK);
	if(m->dataptr == NULL){
		if(m->swapfile == NULL) return(OK);
		/* data is stored in file */
		if((fd1 = open(m->swapfile,O_RDONLY)) == NULL){
			capsar_log(LOG_INFO,M_MSG_81,"error opening %s\n",m->swapfile); /*GAG*/
			return(NOTOK);
		}
		while((n=read(fd1,buf,BUFSIZ)) >0)
			(void) write(fd,buf,n);	
		(void) close(fd1);
		return(OK);
	}

	if(write(fd,m->dataptr,m->size) != m->size){
		(void) capsar_log(LOG_INFO,M_MSG_82,"write error\n"); /*GAG*/
		return(NOTOK);
	}
	return(OK);

}
	
	
puttag(fd,s1,s2)
int	fd;
char	*s1,*s2;
{
	char c;

	c = '\n';
	
	if(s2 == NULL)
		return;

	write(fd,&c,1);
	write(fd,s1,strlen(s1));
	write(fd,s2,strlen(s2));
	write(fd,&c,1);
	
}

	
