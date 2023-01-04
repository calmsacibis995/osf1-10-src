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
static char *sccsid = "@(#)$RCSfile: capsar_extract.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:03:17 $";
#endif lint

/*
 * capsar_extract.c 01/25/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part Messages in Ultrix Mail.
 *
 * syntax : capsar_extract (MM *m, FILE *out)
 *
 * This routine decodes a ddif type message sent through mail. The
 * message must be a simple message.
 */

/*	Generic Includes
 */
#include <stdio.h>
#include <locale.h> /*GAG*/
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <syslog.h>
#include "libcapsar_msg.h" /*GAG*/

/*char	extfile[] = "/tmp/ddif_extXXXXXX";*/
char	extfile[80];

/*	Capsar Specific Includes
*/
#include <capsar.h>

struct opt {
	char	*tagline;
	int	cnt;
} procopt;

static struct opt procopts[] = {
	"cat.compress.uuencode",1,
	"ctod.compress.uuencode",1,
	"cat.uuencode",0,
	"ctod.uuencode",0,
	NULL,NULL
};

capsar_extract(m,out)
MM	*m;
FILE	*out;
{
/*------*\
  Locals
\*------*/

	char		*concat();	/* concatenate strings */

	if(strcmp(m->body_type,DOTSTAG)==0 || strcmp(m->body_type,DDIFTAG)==0){
		if(decode(m,out,1)== OK)return(OK);
		else return(NOTOK);
	}
	if(decode(m,out,0)==OK)return(OK);
	else return(NOTOK);
}

decode(m,out,decodeflag)
MM		*m;
FILE		*out;
int		decodeflag;
{
	char	*command;
	char	*concat();
	char	buf[2*BUFSIZ];
	char	file[128];
	FILE	*pipeptr,*fileptr;
	struct  opt *tp;
	char	*tcp;
	char	*match();
	char	stdbuf[BUFSIZ];
	register int	n,nwritten,offset;

	if(m->dataptr != NULL){
		strcpy(extfile,"/tmp/ddif_extXXXXXX");
		(void) unlink(mktemp(extfile));
		if((fileptr = fopen(extfile,"w")) == NULL){
			error_extract(M_MSG_26,"cannot open %s for writing\n",extfile); /*GAG*/
			return(NOTOK);
		}
		(void) chmod(extfile,0660);
		if(fwrite(m->dataptr,1,m->size,fileptr) == NULL){
			error_extract(M_MSG_27,"write error on file %s\n",extfile); /*GAG*/
			return(NOTOK);
		}
		(void) fclose(fileptr);
		(void) strcpy(file,extfile);
	}
	else if(m->swapfile !=NULL)
		(void) strcpy(file,m->swapfile);
	else return(NOTOK);

	if(decodeflag){	

	if(access(UUDECODE_CMD,F_OK) == NOTOK){
		(void) error_extract(M_MSG_19,"cannot access cmd : %s\n",UUDECODE_CMD); /*GAG*/
		return(NOTOK);
	}

	for(tp= procopts;tcp=tp->tagline;tp++){
		if(match(strlen(tp->tagline),tp->tagline,strlen(m->start),m->start) )
				break;
	}
	if(tcp == NULL)return(NOTOK);	

	if(tp->cnt == NULL)
		command = concat(UUDECODE_CMD,"  ",file,NULLCP);
	
	else {
		if(access(UNCOMPRESS_CMD,F_OK) == NOTOK){
			(void) error_extract(M_MSG_19,"cannot access cmd : %s\n",UNCOMPRESS_CMD); /*GAG*/
			return(NOTOK);
		}
		command = concat(UUDECODE_CMD,"  ",file," | ",UNCOMPRESS_CMD,NULLCP);
	}
	
	if((pipeptr=popen(command,"r"))== NULL){
		error_extract(M_MSG_17,"cannot execute command\n %s\n",command); /*GAG*/
		return(NOTOK);
	}


	while((n=read(fileno(pipeptr),stdbuf,sizeof(stdbuf))) > 0){
		offset=0;
		do {
			nwritten = write(fileno(out),&stdbuf[offset],n);
			if(nwritten <=0){
				error_extract(M_MSG_31,"extract : write error "); /*GAG*/
				return(NOTOK);
			}
			offset += nwritten;
		} while ((n -= nwritten) > 0);
	}
	if(n <0){
		error_extract(M_MSG_32,"extract read error\n",NULL); /*GAG*/
		return(NOTOK);
	}
	
	(void) fclose(fileptr);		
	(void) free(command);
	}
	else {
		if((fileptr=fopen(file,"r"))== NULL){
			(void) error_extract(M_MSG_33,"cannot open file %s for reading\n",file); /*GAG*/
			return(NOTOK);
		}
		while(fgets(buf,sizeof buf,fileptr) != NULL)
			fputs(buf,out);

		(void) fclose(fileptr);		
	}
	
	if(access(file,F_OK) ==0)unlink(file);
	return(OK);
}

error_extract(msg_num,s1,s2) /*GAG*/
int	msg_num;
char	*s1,*s2;
{
	capsar_log(LOG_INFO,msg_num,s1,s2); /*GAG*/
	if(access(extfile,F_OK) == OK)
		(void)unlink(extfile);
}

