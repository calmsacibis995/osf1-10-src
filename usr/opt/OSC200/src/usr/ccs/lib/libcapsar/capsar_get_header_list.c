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
static char *sccsid = "@(#)$RCSfile: capsar_get_header_list.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/12/21 15:01:02 $";
#endif lint

/*
 * capsar_get_header_list.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : char **capsar_get_header_list ( MM *m )
 *
 * For a MAIL_MESSAGE return a null-terminated list of the header components.
 * Each header component is a string of a header field name, a colon (:),
 * one or more spaces, and a field value. Each header line may have embedded
 * continuation sequences in it ( ie LF followed by spcaes or tabs.
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <syslog.h>
#include <capsar.h>
#include "headers.h"
#include "libcapsar_msg.h" /*GAG*/

char		*hptr;
int		msize;
char		*h_info[1000];

char **capsar_get_header_list(m)
MM	*m;	/* pointer to message structure */
{
/*------*\
  Locals
\*------*/
	FILE	*fileptr;
	char	buf[BUFSIZ],
		*ptr,
		*realloc();
	int	sw=0,
		eoh=0,
		cont_poss=0,
		i=0,k,lh,
		state,ret;


	if(m==NULL)return(NULL);

	if(m->swapfile != NULL){
			if((fileptr = fopen(m->swapfile,"r")) == NULL){
				(void) capsar_log(LOG_INFO,M_MSG_2,"cannot open message file %s\n",m->swapfile); /*GAG*/
				return(NULL);
			}
			sw++;
	}
	else if(m->dataptr !=NULL)sw=0;
	else{
		return(NULL);
	}

	hptr = m->dataptr;	
	msize = m->size;
	h_info[i] = NULL;

	while(!eoh){
		state=get_line_from_file_or_core(buf,sizeof buf,fileptr,sw,m->separator);
		switch (state) {

			case EOA :
				eoh++ ;
				break;

			case ANYTEXT :
				cont_poss=0;
				ret = getstr(buf,m->separator);
				if(ret == HEADER){
					h_info[i++] = getcpy(buf);
					cont_poss++;
				}
				else if(ret == EOA)
					eoh++;
				else return(NULL);
				break;
			case CONT_POSS :
				if(cont_poss){
					lh = strlen(h_info[i-1]);
					h_info[i-1] = realloc(h_info[i-1],strlen(buf) + lh+1);
					if(h_info[i-1] == NULL){
						(void) capsar_log(LOG_INFO,M_MSG_40,"cannot allocate more space\n",NULL); /*GAG*/
						return(NULL);
					}
					ptr = h_info[i-1] + lh;
					k=strlen(buf);
					for(k=0;k<strlen(buf);k++)	
						*ptr++ = *(buf+k);
					*ptr++ = '\0';
				}
				else {
					return(NULL);
				}
				break;
			case EOM :
				return(NULL);
		}
	}
	h_info[i++] = NULL;
	return(h_info);
}

getstr(buf,sep)	
char	*buf;
char	*sep;
{

	char	c,
		*cp;
	int	j,i,
		field_length;

	cp = buf;

	if(!sep)sep = DEF_SEP;
	if(strncmp(buf,sep,strlen(buf))==0)
		return(EOA);

	field_length =  j = strlen(buf);

	while(--j > 0){
		if(*buf == ' ' || *buf == '\r')
			buf++;
		else break;
	}
	if(*buf == '\n')return(EOA);

	j = field_length;
	buf = cp;

	while((c = *buf++) != ':' && c != '\n' && --j > 0);

	if(j<=0 || c == '\n')
		return(ANYTEXT);	
		
	/* get the header field and check that it is ok */

	i=0;
	while(header_list[i++]){
		if((compare(cp,header_list[i-1],field_length-j)==0))
			return(HEADER);
	}
		
	/* not in primary list however accept it */
	return(HEADER);
}	
	
get_line_from_file_or_core(s,n,fptr,sw,sep)
char	*s;
int	n;
FILE	*fptr;
int	sw;
char	*sep;
{
	register char c;
	register char *cs,*ncs;

        c = '\0';
	cs = s;
	ncs = s;
	if(!sep)sep = DEF_SEP;
	
	while( sw && --n > 0 && (c=getc(fptr))>=0){
		*cs++ =c;
		if(c == '\n')
			break;
	}
	
	
	while( !sw && --n > 0 && --msize >0){
		c = *hptr++;
		*cs++ =c;
		if(c == '\n')
			break;
	}
	if(c < 0 && cs==s)
		return(EOM);
	*cs++ = '\0';

	if(strncmp(ncs,sep,strlen(sep)) == 0)
		return(EOA);

	if(check(ncs,strlen(ncs)))return(EOA);

	else if(*ncs == '\t' || *ncs == ' ')return(CONT_POSS);
	
	return(ANYTEXT);
}

