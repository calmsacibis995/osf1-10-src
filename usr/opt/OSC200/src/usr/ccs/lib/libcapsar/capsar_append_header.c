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
static char *sccsid = "@(#)$RCSfile: capsar_append_header.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:02:42 $";
#endif lint

/*
 * capsar_append_header.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : capsar_append_header ( MM *m , char * h )
 *
 * Append the header line h to the set of header lines in m. A
 * new header is created if one does not exist and the message type
 * is modified to reflect the addition.
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <syslog.h>
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

/*char		append_file[]="/tmp/append_fileXXXXXX";*/
char		append_file[80];

capsar_append_header(m,h)
MM	*m;	/* pointer to message structure */
char	*h;	/* header line to be appended */
{
/*------*\
  Locals
\*------*/
	FILE	*fileptr,
		*outptr;
	int	j,
		pos=0,
		cnt,
		ssep,
		slen;
	char	**cp,
		buf[BUFSIZ],
		*realloc(),
		*cps,
		*data,
		*data0,
		*newplace,
		*dataptr,
		*dataend,
		*sep,
		c,*h1;

	if(m==NULL  &&  (m->message_type == COMPOUND_MESSAGE || m->message_type == MULTIPLE_FORMAT) )return(NOTOK);

	
	h1 = h;
	slen = strlen(h);

	/* sort out separator */
	sep = m->separator;
	if(!sep)sep = DEF_SEP;
	ssep = strlen(sep);

	/* find stem for header line h */
	j=slen;
	while((c= *h1++) != ':' && c != '\n' && --j >0);
	if(j<=0 || c== '\n')
		return(NOTOK);

	if(m->message_type == EMPTY){
		dataptr = (char *) malloc(slen + ssep);
		if(dataptr == NULL){
			capsar_log(LOG_INFO,M_MSG_1, "cannot malloc enough space\n",NULL); /*GAG*/
			return(NULL);
		}

		strcpy(dataptr,h);
		dataend = dataptr + strlen(h);
		while(ssep-- > 0)
			*dataend++ = *sep++;
		
		m->dataptr = dataptr;
		m->size = strlen(dataptr);
		m->message_type = MAIL_MESSAGE;
		return(OK);
	}

	/* get null terminated list of header lines */
	cp = capsar_get_header_list(m);
	
	if(m->swapfile){
		if((fileptr = fopen(m->swapfile,"r")) == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_2,"cannot open message file %s\n",m->swapfile); /*GAG*/
			return(NULL);
		}
		strcpy(append_file,"/tmp/append_fileXXXXXX");
		(void) unlink(mktemp(append_file));
		if((outptr = fopen(append_file,"w")) == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_2,"cannot open message file %s\n",append_file); /*GAG*/
			(void) unlink(append_file);
			return(NULL);
		}
		while(cps = *cp++){
			(void) fputs(cps,outptr);			
			pos = pos + strlen(cps);
		}
		(void) fputs(h,outptr);
		(void) fputs(sep,outptr);

		(void) fseek(fileptr,pos,0);
			
		while(fgets(buf,sizeof buf,fileptr))
			(void) fputs(buf,outptr);

		m->size = m->size + slen + ssep;
		m->message_type = MAIL_MESSAGE;

		(void)fclose(fileptr);
		(void)fclose(outptr);
		(void)unlink(m->swapfile);
		free(m->swapfile);
		m->swapfile = getcpy(append_file);
		return(OK);

	}
	else if(m->dataptr){
		dataptr = (char *)realloc(m->dataptr,(unsigned int)(m->size+strlen(h) + ssep));

		m->dataptr = dataptr;
		if(m->dataptr == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_4," cannot realloc space\n"); /*GAG*/
			return(NULL);
		}

		
		while(cps = *cp++){
			pos = pos + strlen(cps);
		}	
		data = m->dataptr;
		data0 = data + m->size;
		data = data +pos;
		newplace = data0 + slen +ssep;

		for(cnt=0;cnt<m->size-pos+ssep;cnt++)
			*newplace-- = *data0--;

		for(cnt=0;cnt< slen;cnt++){
			*data = *h++;
			data++;
		}
		while(ssep-- > 0)
			*data++ = *sep++;

		m->size = m->size + slen + ssep;
		m->message_type = MAIL_MESSAGE;
		return(OK);
	}
	else
		return(NOTOK);
}	

