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
static char *sccsid = "@(#)$RCSfile: capsar_delete_header.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:03:01 $";
#endif lint

/*
 * capsar_delete_header.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : capsar_delete_header ( MM *m , char * h )
 *
 * Delete the header line h to the set of header lines in m.
 * The pointer h is the one returned from capsar_get_header_list
 * or capsar_get_header
 *
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <syslog.h>
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

int		msize;
char		*h_info[100];
/*char		delete_file[]="/tmp/delete_fileXXXXXX";*/
char		delete_file[80];

capsar_delete_header(m,h)
MM	*m;	/* pointer to message structure */
char	*h;	/* header line to be deleteed */
{
/*------*\
  Locals
\*------*/
	FILE	*fileptr,*outptr;
	char	buf[BUFSIZ],
		*realloc(),
		**cp,**cp0,
		*cps,
		*data,
		*data0,
		*newplace,
		*dataptr;
	int	pos=0,pos1=0,pos2=0,
		cnt,
		slen,
		matchm=0;

	if(m==NULL || m->message_type != MAIL_MESSAGE)return(NULL);
	
	slen = strlen(h);

	cp = capsar_get_header_list(m);
	cp0 = cp;

	if(!cp)return(NOTOK);

	/* try to find header h in the list of heade lines */

	pos1=0;
	while(!matchm && (cps = *cp++)){
		if(strncmp(cps,h,slen) == 0)matchm++;
		else pos1 = pos1 + strlen(cps);
	}
	if(!matchm) return(NOTOK);
	pos2 = pos1 + strlen(cps);
	
	if(m->swapfile){
		if((fileptr = fopen(m->swapfile,"r")) == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_2,"cannot open message file %s\n",m->swapfile); /*GAG*/
			return(NULL);
		}
		strcpy(delete_file,"/tmp/delete_fileXXXXXX");
		(void) unlink(mktemp(delete_file));
		if((outptr = fopen(delete_file,"w")) == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_2,"cannot open message file %s\n",delete_file); /*GAG*/
			(void) unlink(delete_file);
			return(NULL);
		}
		cp=cp0;
		pos=0;
		while((cps = *cp++) && pos < pos1){
			(void) fputs(cps,outptr);			
			pos = pos + strlen(cps);
		}

		(void) fseek(fileptr,pos2,0);
			
		while(fgets(buf,sizeof buf,fileptr))
			(void) fputs(buf,outptr);

		m->size = m->size - slen;

		(void)fclose(fileptr);
		(void)fclose(outptr);
		(void) unlink(m->swapfile);
		free(m->swapfile);
		m->swapfile = getcpy(delete_file);
		return(OK);

	}
	else if(m->dataptr){
		data = m->dataptr;
		newplace = data + pos1;
		data0 = data + pos2;

		for(cnt=0;cnt<m->size-pos2;cnt++)
			*newplace++ = *data0++;

		m->size = m->size - slen;
		dataptr = (char *)realloc(m->dataptr,(unsigned int)(m->size));

		m->dataptr = dataptr;
		if(m->dataptr == NULL){
			(void) capsar_log(LOG_INFO,M_MSG_4,"cannot realloc space\n"); /*GAG*/
			return(NULL);
		}
		return(OK);
	}
	else
		return(NOTOK);
}	

