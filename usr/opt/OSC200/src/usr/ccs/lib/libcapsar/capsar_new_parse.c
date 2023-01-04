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
static char *sccsid = "@(#)$RCSfile: capsar_new_parse.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/09/29 12:34:22 $";
#endif lint

/*
 * capsar_new_parse.c  01/14/88
 * Lynn C Wood,
 *
 * This routine is provides as part of the capsar library for handling
 * of Multiple Body Part Messages in Ultrix Mail.
 * 
 * syntax : MM *capsar_new_parse ( FILE *fd )
 *
 * A flat representation of a message is read from fd and a pointer
 * is returned to the internal representation of the message. 
 * If fd is a descriptor for a file, the file should not be changed for
 * as long as the internal representation of the message exists.
 * If fd is a descriptor for a pipe or socket, the file will be copied
 * into a temporary location.
 *
 * THIS NEEDS SOME WORK ON IT ...
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

/*char	xnewfile[]="/tmp/capsarXXXXX";*/
char	xnewfile[80];
int	HIGH_TIDE_MARK;
int	xdata_file_cnt=0;

MM *capsar_new_parse(fd,smtp)
FILE	*fd;		/* file descriptor for flat representation */
int	smtp;
{
/*------*\
  Locals
\*------*/

	struct	stat	stbuf;	/* for holding fstat information */
	MM	*m,*mm1,*mmp;	/* mail structure */
	MM	*mt;
	int	mode;		/* returned from fstat */
	int	socket_flag=0;	/* socket flag */
	int	pipe_flag=0;	/* descriptor points to pipe */
	int	soffset=0;	/* lseek offset */
	int	oldoffset;	/* lseek offset */
	int	pos=0;		/* file descriptor position */
	int	chpos;		/* encapsulation boundery */
	int	buffer[512];	/* temporary buffer */
	int	currpos=0;	/* current level position */
	int	eom=0;		/* end of mail message flag */
	int	ret;		/* returned status from getline */
	int	tm=0;		/* tide mark */
	int	mrm=0,sot=0;
unsigned int	eblength;	/* encapsulation boundery */
	FILE	*pfd;		/* file pointer */
	struct	rlimit	rlp;	
	char	*separator,
		*getname(),
		*gettag (),
		*malloc(),
		*tmpname,
		**cps,
		buf[BUFSIZ],
		*Xcopytmp();
	MM	*capsar_take_whole();

	if(smtp == NOTSMTP || smtp == SMTPLF || smtp == SMTPCRLF)
			;
	else 
		capsar_log(LOG_INFO,M_MSG_45,"illegal smtp type\n",NULL); /*GAG*/

	(void) getrlimit(RLIMIT_CORE,&rlp);
	HIGH_TIDE_MARK = rlp.rlim_max*TIDE_MARK_RATIO;


	if(stbuf.st_mode == 0)pipe_flag=1;

	/* if descriptor points to a socket or pipe 
         * file is copied into a temporary location
	 */
	if((tmpname = Xcopytmp(fd,smtp)) == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_46,"cannot copy mail data to temp file\n",NULL); /*GAG*/
		return(NULL);	
	}
	if((pfd = fopen(tmpname,"r")) == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_47,"cannot open temporary file name %s\n",tmpname); /*GAG*/
		return(NULL);
	}
	
	
	/* now do the message descapsulation */

	/* allocate mail structure */
	m = (MM *)malloc ((unsigned) sizeof *m);
	if(m == NULL){ 
		(void) capsar_log(LOG_INFO,M_MSG_11,"unable to allocate mail structure\n",NULL); /*GAG*/
		return(NULL);
	}
	capsar_setup(m);
	mm1 = m;	

	m->message_type = MAIL_MESSAGE;

	
	if(smtp == SMTPCRLF)
		separator = getcpy(SMTP_SEP);
	else 
		separator = getcpy(DEF_SEP);
	m->separator = separator;	
	
	mmp = m;

	/* first portion must be a content header */
	/* read the data a line a a time until find
	 * an encapsulation boundery.
	 */

	soffset = ftell(pfd);
	ret = get_eb_boundery(buffer,currpos,&chpos,&eblength,pfd,&pos,NOTSMTP);

	if(pos == 0 && smtp == NOTSMTP){
		if(access(tmpname,F_OK)==0)
			(void) unlink(tmpname);
		free(m);

		capsar_log(LOG_INFO,M_MSG_49,"no headers lines in message\n",NULL); /*GAG*/
		return(NULL);
	}

	
	/* at this point have either found the first encapsulation
         * boundery or end of flattened message
 	 */
	m->size = pos-soffset ;
	m->offset = 0;
	m->complete = 2;
	oldoffset = ftell(pfd);
	(void) fseek(pfd,soffset,0);

	/* allocate enough space for mail data
	 * and read in text from flattened form 
	 */
	if(m->size > 0 ){	
		if((tm = xwritemm(pfd,m,m->size,soffset,tm)) == NOTOK)
			return(NULL);
	}

	(void) fseek(pfd,oldoffset,0);
	m->message_type = MAIL_MESSAGE;
	m->body_type=capsar_get_body_type(m);
	
	cps = capsar_get_header_list(m);
	if(cps == NULL && smtp == NOTSMTP){
		capsar_log(LOG_INFO,M_MSG_50,"data is not a mail message\n",NULL); /*GAG*/
		return(NULL);
	}
	
	if(smtp !=NOTSMTP){
		while(!sot && capsar_get_buf(m,buf,sizeof buf) > 0){
			if(!check(buf,strlen(buf)))
				sot++;
		}
		if(!sot || m->size <= 0)mrm++;
		else m->message_type = COMPOUND_MESSAGE;
		m->offset = 0;
	}

	if(ret==EOM){
		if(access(tmpname,F_OK) == OK)
			(void) unlink(tmpname);
		return(mm1);
	}
	chpos=0;
	
	while(!eom){
		switch(chpos){
			case 0: /* stay same level */
				if(m->complete == 2){
					if((m->mm_next = (MM *)malloc((unsigned) sizeof *m)) == NULL){
						(void) capsar_log(LOG_INFO,M_MSG_51,"cannot malloc space\n"); /*GAG*/
						return(NULL);
					}
					m=m->mm_next;
					capsar_setup(m);
					m->separator=getcpy(separator);
					m->parent = mmp; 
					m->start = gettag(buffer,eblength);
					m->name=getname(m->start);
					m->message_type = SIMPLE_MESSAGE;
					m->complete = 1;
				}
				else if(m->complete == 1){
					m->message_type=capsar_message_type(m);
					m->body_type=capsar_get_body_type(m);
					m->stop = gettag(buffer,eblength);
					if(paranoia(m->start,m->stop)==NOTOK)
						goto clearup;	
					if((tm=xwritemm(pfd,m,pos,soffset,tm)) == NOTOK)
						return(NULL);	
					m->complete =2;
				}
				else if(m->complete ==0)
					goto clearup;
				break;

			case -1: /* down one level */
				/* make a directory node */
				/* this directory node will also contain
         			 * a content header if one exists.
				 */
				if((tm=xwritemm(pfd,m,pos,soffset,tm)) == NOTOK)
					return(NULL);	
				m->message_type = COMPOUND_MESSAGE;
				m->message_type = capsar_message_type(m);
				m->body_type=capsar_get_body_type(m);
				/* allocate a new strucure for next 
  				 * message in linked list */
				mmp=m;
				if((m->mm_next = (MM *)malloc((unsigned) sizeof *m)) == NULL){
					(void) capsar_log(LOG_INFO,M_MSG_51,"cannot malloc space\n"); /*GAG*/
					return(NULL);
				}
				m=m->mm_next;
				capsar_setup(m);
				m->separator=separator;
				m->parent = mmp;
				m->start = gettag(buffer,eblength);
				m->name=getname(m->start);
				m->message_type = SIMPLE_MESSAGE;
				m->complete = 1;
				break;
			
			case 1:	/* up one level */
				/* make sure current level is
				 * correctly terminated 
				 */
				if(m->complete!=2 )
					goto clearup;
					
				/* make sure can point to 
				 * parent and it exists
				 */
				if(mmp  == NULL)	
					goto clearup;

				if(mmp->complete==1){
					mmp->stop = gettag(buffer,eblength);
					if(paranoia(mmp->start,mmp->stop)==NOTOK)
						goto clearup;
					mmp->complete =2;
					mmp = mmp->parent;	
				}
				else 
					goto clearup;	
				break;
			default	: /* incorrect level change */
				goto clearup;
				break;
		}/* E of switch chpos */
			
		/* extract next encapsualation boundery */
		pos=0;
		soffset = ftell(pfd);
		ret = get_eb_boundery(buffer,currpos,&chpos,&eblength,pfd,&pos,NOTSMTP);
		if(ret == EOM){
			eom=1;
			/* check for completeness */
			m=mm1;
			while(m != NULL){
				if(m->complete !=2)
					goto clearup;
				m = m->mm_next;
			}
		}
		currpos -= chpos;
			
	}/* E of while eom */	
	if(access(tmpname,F_OK) == OK)
		(void) unlink(tmpname);
	if(mrm)
		return(mm1->mm_next);
	else
		return(mm1);

clearup: ;
	fflush(pfd);	
	rewind(pfd);

	mt = (MM *)malloc ((unsigned) sizeof *mt);
	capsar_setup(mt);
	if(mt == NULL) {
		(void) capsar_log(LOG_INFO,M_MSG_11,"unable to allocate mail structure\n",NULL); /*GAG*/
		return(NULL);
	}
	mt->message_type = MAIL_MESSAGE;
	if(stat(tmpname,&stbuf) < 0){
		capsar_log(LOG_INFO,M_MSG_7,"cannot stat %s\n",tmpname); /*GAG*/
		unlink(tmpname);
		return(NULL);
	}
	mt->size = stbuf.st_size-soffset ;
	mt->offset = 0;
	mt->complete = 2;
	if((tm = xwritemm(pfd,mt,mt->size,soffset,tm)) == NOTOK)
		return(NULL);
	mt->body_type = capsar_get_body_type(mt);
	fflush(pfd);	
	rewind(pfd);

	if(access(tmpname,F_OK)  == OK)
		(void) unlink(tmpname);
	if(!mt) {
		return(NULL);
	}
	if(smtp)
		m->separator = getcpy(SMTP_SEP);
	else 
		mt->separator = getcpy(DEF_SEP);

	if(mt->size == 0){
		free(mt);
		return(NULL);
	}
	return(mt);
}

/*
 * xwritemm()
 * 
 * this routine write the data part of each message from the
 * flattened file into the mail structure
 *
 */

xwritemm(pfd,m,length,soffset,tm)
FILE	*pfd;	/* file descriptor */
MM	*m;	/* mail strucure */
int	length;	/* message size */
int 	soffset;/* data offset */
int	tm;	/* data tide mark */
{
	int	oldoffset;
	FILE	*dataptr;
	char	buf[BUFSIZ];
	char	tmpfile[80];
	int	bufacc=0,nwrite;

	m->size = length ;
	m->offset = 0;
	if(m->size == 0) return(tm);
	oldoffset = ftell(pfd);
	(void) fseek(pfd,soffset,0);


	if(tm+m->size < HIGH_TIDE_MARK){

		/* allocate enough space for mail data
		 * and read in text from flattened form 
		 */
		m->dataptr = (char *) malloc((unsigned) m->size);
		if(m->dataptr == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_53,"unable to allocate data storage\n",NULL); /*GAG*/
			return(NOTOK);
		}

		if(fread(m->dataptr,1,m->size,pfd) == NULL ){
			(void) capsar_log(LOG_INFO,M_MSG_54,"error in reading mail data\n",NULL); /*GAG*/
			m->dataptr=NULL;
			return(NOTOK);
		}
		
		tm += m->size;
	}
	else {
		/* create temporary files to store the data
		 * in sedondary storage 
		 */
		
		m->dataptr = NULL;
		(void) sprintf(tmpfile,"/tmp/cparse%d.XXXXXX",xdata_file_cnt++);
		(void) unlink(mktemp(tmpfile));
	
		dataptr = fopen(tmpfile,"w");
		(void)chmod(tmpfile,0660);
		m->swapfile = getcpy(tmpfile);

		if(m->size < sizeof(buf)) nwrite = m->size;
		else nwrite = sizeof(buf);
		while(fread(buf,1,nwrite,pfd)){
			fwrite(buf,1,nwrite,dataptr);
			bufacc = bufacc + nwrite;
			if(bufacc + sizeof(buf) <= m->size)nwrite = sizeof(buf);
			else nwrite = m->size - (bufacc);
		}
		
		(void) fclose(dataptr);
		
	}

	(void) fseek(pfd,oldoffset,0); 
	
	return(tm);
}

/* 
 * getname()
 *
 * this routine extracts the message name from the encapsulation
 * boundery tag.
 *
 */

#define STREQL(a,b)	(strcmp(a,b) == 0)

char	*Xcopytmp(pfil,smtp)
FILE	*pfil;
int	smtp;
{
	FILE	*nfil;
	char	buffer[BUFSIZ],*p;
	char	*index();
	char	*tmpname;

	strcpy(xnewfile,"/tmp/capsarXXXXX");
	(void) unlink(mktemp(xnewfile));
	tmpname = getcpy(xnewfile);
	(void) chmod(xnewfile,0660);

	if((nfil = fopen(xnewfile,"w")) == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_55,"cannot open temporary file for writing\n"); /*GAG*/
		(void) unlink(xnewfile);
		return(NULL);
	}

	while(fgets(buffer,sizeof(buffer),pfil) != NULL){
		if((p = index(buffer,'\n')) == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_56,"error in copyng data\n",NULL); /*GAG*/
			(void)unlink(xnewfile);
			return(NULL);
		}
		if(p != buffer && p[-1] == '\r')
			strcpy(&p[-1],"\n");
		
		if((smtp == SMTPLF || smtp == SMTPCRLF) && STREQL(buffer,".\n"))
			break;

		if(buffer[0] == '.')
			fputs(&buffer[1],nfil);
		else
			fputs(buffer,nfil);

	} 

	if(smtp == SMTPLF || smtp == SMTPCRLF){
	if(ferror(pfil) || feof(pfil) || ferror(nfil)){

		(void) capsar_log(LOG_INFO,M_MSG_57,"unexpected eof\n",NULL); /*GAG*/
		(void) unlink(xnewfile);
		return(NULL);
	}
	}
	
	fclose(nfil);
	
	return(tmpname);
}
