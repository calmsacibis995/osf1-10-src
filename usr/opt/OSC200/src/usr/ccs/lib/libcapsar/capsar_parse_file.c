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
static char *sccsid = "@(#)$RCSfile: capsar_parse_file.c,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/04/16 10:07:07 $";
#endif lint

/*
 * capsar_parse_file.c  01/14/88
 * Lynn C Wood,
 *
 * This routine is provides as part of the capsar library for handling
 * of Multiple Body Part Messages in Ultrix Mail.
 * 
 * syntax : MM *capsar_parse_file ( int fd )
 *
 * A flat representation of a message is read from fd and a pointer
 * is returned to the internal representation of the message. 
 * If fd is a descriptor for a file, the file should not be changed for
 * as long as the internal representation of the message exists.
 * If fd is a descriptor for a pipe or socket, the file will be copied
 * into a temporary location.
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

/*char	newfile[]="/tmp/capsarXXXXX";*/
char	newfile[80];
int	HIGH_TIDE_MARK;
int	data_file_cnt=0;

MM *capsar_parse_file(fd,smtp)
int	fd;		/* file descriptor for flat representation */
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
unsigned int	eblength;	/* encapsulation boundery */
	FILE	*pfd;		/* file pointer */
	struct	rlimit	rlp;	
	char	*separator;
	char	*getname();
	char	*gettag ();
	char	*malloc();
	char	tmpname[BUFSIZ];
	char	**cps,*cp;
	MM	*capsar_take_whole();

	if(smtp == NOTSMTP || smtp == SMTPLF || smtp == SMTPCRLF)
			;
	else 
		capsar_log(LOG_INFO,M_MSG_45,"illegal smtp type\n",NULL); /*GAG*/

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
				break;
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
	if(socket_flag || pipe_flag || smtp){
		if((fd = copytmp(fd,tmpname,smtp)) == NOTOK){
			(void) capsar_log(LOG_INFO,M_MSG_46,"cannot copy mail data to temp file\n",NULL); /*GAG*/
			return(NULL);	
		}
	}
	else {
		if((stbuf.st_mode & S_IFMT) == S_IFDIR){
			(void)capsar_log(LOG_INFO,M_MSG_64,"file descriptor refers to a directory\n",NULL); /*GAG*/
			return(NULL);
		}
	}

	if((pfd = fdopen(fd,"r")) == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_65,"error in opening %s\n",tmpname); /*GAG*/
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

	/*if(pos == 0 && smtp == NOTSMTP){
		if(access(newfile,F_OK)==0)
			(void) unlink(newfile);
		free(m);
		capsar_log(LOG_INFO,M_MSG_67,"no headers in message\n",NULL);
		return(NULL);
	}*/

	
	/* at this point have either found the first encapsulation
         * boundery or end of flattened message
 	 */
	m->size = pos-soffset ;
	m->flat_fd = fd;
	m->offset = 0;
	m->complete = 2;
	oldoffset = ftell(pfd);
	(void) fseek(pfd,soffset,0);

	/* allocate enough space for mail data
	 * and read in text from flattened form 
	 */
	if(m->size > 0){	
		if((tm = writemm(pfd,m,m->size,soffset,tm,fd)) == NOTOK)
			return(NULL);
	}
	
	(void) fseek(pfd,oldoffset,0);
	m->message_type = MAIL_MESSAGE;
	m->body_type=capsar_get_body_type(m);

	cps = capsar_get_header_list(m);
	/*if(cps == NULL && smtp == NOTSMTP){
		capsar_log(LOG_INFO,M_MSG_68,"mail message has no header lines\n",NULL);
		return(NULL);
	}*/

	if(cps)
		while(cp = *cps++)
			if(cp)free(cp);
	
	if(smtp !=NOTSMTP){
		if(m->size ==0)m->message_type = EMPTY;
		else if(*cps)m->message_type = MAIL_MESSAGE;
		else m->message_type = SIMPLE_MESSAGE;
	}

	if(ret==EOM){
		if(access(newfile,F_OK) == OK)
			(void) unlink(newfile);
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
					if((tm=writemm(pfd,m,pos,soffset,tm,fd)) == NOTOK)
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
				if((tm=writemm(pfd,m,pos,soffset,tm,fd)) == NOTOK)
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
			/* chack for completeness */
			m=mm1;
			while(m != NULL){
				if(m->complete !=2)
					goto clearup;
				m = m->mm_next;
			}
		}
		currpos -= chpos;
			
	}/* E of while eom */	
	if(access(newfile,F_OK) == OK)
		(void) unlink(newfile);
	return(mm1);

clearup: ;

	lseek(fd,0,0);
	mt = capsar_take_whole(fd);
	if(access(newfile,F_OK)  == OK)
		(void) unlink(newfile);
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
 * Function :  eb
 *
 * Description : check encapsulation boundery
 *
 * Each line of text is examined to determine is a valid
 * encapsulation boundery occurs. A valid encapsulation
 * boundery is defined as a sequence of dashes followed
 * by zero or one white space followed by more than one
 * dash.
 *
 *
 */

eb(buffer,oldpos,chpos,eblength)
char	*buffer;
int	oldpos;
int	*chpos;
int	*eblength;
{
	int	a,b,c;	/* count characters */
	int	pos;
	char	*cp;
	char	*strchr();

	int	level=0,
		bad=0;

	*chpos =0;
	cp = buffer;
	if(*cp == '-'){
		while(*cp != '\n' && !bad){
			if(*cp == '-')cp++;
			else if (*cp == ' '){
				if(*(cp-1) == '-' && *(cp+1) == '-'){
					level++;
					cp++;
				}
				else bad++;
			}
			else bad++;
		}
	}
	else {
		*eblength =0;
		return(NOTOK);
	}

	/* AJ:  3 changed to 8, as Forwarded and Bcc */
        /* messages have 7 '-'s before the msg, while capsar has 10 */
	if((cp - buffer) > 8 ){ 
	  /* AJ: check for " : ", as capsar put it in when formating  */
	        if ( strstr(buffer," : ") == NULL){
		  *eblength =0;
		  return(NOTOK);
		}
		*eblength = cp -buffer;
		*chpos = oldpos -level; 
		return(OK);
	}
	else {
		*eblength =0;
		return(NOTOK);
	}
	
}

/*
 * getline()
 *
 * this function returns a line of text at a time in order
 * to scan for an encapsuation boundery.
 *
 */
getline(s,n,pfd,smtp )
char	*s;		/* line buffer */
int	n;		/* size of buffer */
FILE    *pfd;		/* file descriptor */
int	smtp;
{
	register char c;
	register char *cs;

	cs=s;
	
	while(--n>0 && (c=getc(pfd))>=0){
		*cs++ = c;
		if(c == '\n' && (cs-s) ==1){
			*cs++='\0';return(EBSEP);
		}
			
		if(c == '\n')
			break;
	}
	
	if(c < 0 && cs==s)
		return(EOM);
	*cs++ = '\0';
	if(check(s,strlen(s)))return(EBSEP);
	return(OK);
}

/*
 * writemm()
 * 
 * this routine write the data part of each message from the
 * flattened file into the mail structure
 *
 */

writemm(pfd,m,length,soffset,tm,fd)
FILE	*pfd;	/* file descriptor */
MM	*m;	/* mail strucure */
int	length;	/* message size */
int 	soffset;/* data offset */
int	tm;	/* data tide mark */
int	fd;
{
	int	oldoffset;
	FILE	*dataptr;
	char	buf[BUFSIZ];
	char	tmpfile[80];
	int	bufacc=0,nwrite;

	m->size = length ;
	m->flat_fd = fd;
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
		(void) sprintf(tmpfile,"/tmp/cparse%d.XXXXXX",data_file_cnt++);
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
char *getname(s)
char	*s;	/* buffer */
{
	char	*cs;
	char	*s1;	
	int	n;
	char	*strchr();
	char	*name;
	
	n = ':';
	s1 =s;

	if((cs = strchr(s1,n)) == NULL)return(NULL);

	n= cs-s1;
	name = (char *)malloc((unsigned) n+1);
	if(name == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_51,"cannot malloc space\n"); /*GAG*/
		return(NULL);
	}
	(void) strncpy(name,s1,n);
	return(name);
}

	
/*
 * char *gettag()
 *
 * get encapsulation bounderttag into managed memory
 *
 */

char   *gettag (str,length)
register char  *str;
unsigned int	length;
{
    register char  *cp;
    unsigned int   n;

    if((n = (unsigned) strlen(str) +1 - length) < 1)return(NULL);
    if ((cp = malloc ((unsigned) n)) == NULL){
	(void) capsar_log(LOG_INFO,M_MSG_71,"unable to allocate string storage",NULL); /*GAG*/
	return(NULL);
    }

    (void) strcpy (cp, str+length);
    return cp;
}

paranoia(tag1,tag2)
char	*tag1;
char	*tag2;
{
	char	str[80];
	char	*t1,*t2;
	char	*strchr();
	int	c;

	(void) strcpy(str,"End of ");

	if(tag1 == NULL && tag2 == NULL)return(OK);

	if(tag1 == NULL || tag2 == NULL)
		return(NOTOK);
	
	if((t1 = strchr(tag2,'E')) == NULL)
		return(NOTOK);

	if(strncmp(t1,str,strlen(str)) !=NULL)
		return(NOTOK);

	t2 = t1 + strlen(str);

	c = *t2;

	if((t1 = strchr(tag1,c)) == NULL)
		return(NOTOK);

	if((strcmp(t1,t2) != NULL)) 
		return(NOTOK);

	return(OK);

}

get_eb_boundery(obuffer,level,chlevel,eblength,pfd,pos,smtp)
char	*obuffer;
int	level;
int	*chlevel;
unsigned int	*eblength;
FILE    *pfd;
int	*pos;
int	smtp;
{
	int	state,
		previous,
		lem;
	char	buffer[512];

	previous = EBSEP;
	*pos =0;
	lem=0;

	for(;;){
		state = getline(buffer,sizeof buffer,pfd,smtp);
/*		printf("\n %d  %s :  ",state,buffer);*/
		if(state != EOM)*pos = *pos + strlen(buffer);
		switch(state){
			case EBSMTP :
			case EBSEP :
				if(previous==EBDASH){
					*pos = *pos - 2 - lem ;
					if(*pos <0)*pos =0;
					return(EBFOUND);
				}
				else previous = EBSEP;
				
				break;
			case EOM :
				if(previous==EBDASH){
					*pos = *pos -1 -lem;
					if(*pos <0)*pos =0;
					return(EBFOUND);
				}
				return(EOM);
			case DOT :
			default :
				if(previous == EBSEP){
					if(eb(buffer,level,chlevel,eblength) == OK){
						previous=EBDASH;
						(void) strcpy(obuffer,buffer);
						lem = strlen(obuffer); 
					}
					else
						previous=ANYTEXT;
				}
				else previous = ANYTEXT;
				break;
		}
	}	

}

#define STREQL(a,b)	(strcmp(a,b) == 0)

int copytmp(fd,tmpname,smtp)
int	fd;
char	*tmpname;
{
	int	newfd;
	FILE	*pfil,*nfil;
	char	buffer[BUFSIZ],*p;
	char	*index();

	if((pfil = fdopen(fd,"r")) == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_72,"error in file manipulation\n"); /*GAG*/
		return(NOTOK);
	}

	strcpy(newfile,"/tmp/capsarXXXXX");
	(void) unlink(mktemp(newfile));
	strcpy(tmpname,newfile);
	(void) chmod(newfile,0660);

	if((nfil = fopen(newfile,"w")) == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_55,"cannot open temporary file for writing\n"); /*GAG*/
		(void) unlink(newfile);
		return(NOTOK);
	}

	while(fgets(buffer,sizeof(buffer),pfil) != NULL){
		if((p = index(buffer,'\n')) == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_56,"error in copyng data\n",NULL); /*GAG*/
			(void)unlink(newfile);
			return(NOTOK);
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
		(void) unlink(newfile);
		return(NOTOK);
	}
	}
	
	(void) fclose(pfil);
	(void) fclose(nfil);
	/*(void) close(fd);*/
	
	if((newfd = open(newfile,O_RDONLY)) == NOTOK){
		(void) capsar_log(LOG_INFO,M_MSG_75,"cannot open %s \n",newfile); /*GAG*/
		(void) unlink(newfile);
		return(NOTOK);
	}
	return(newfd);
}

capsar_setup(m)
MM	*m;
{
	m->dataptr = NULL;
	m->start = NULL;
	m->stop = NULL;
	m->separator=NULL;
	m->name=NULL;
	m->body_type = NULL;
	m->swapfile = NULL;
	m->mm_next= NULL;
}
