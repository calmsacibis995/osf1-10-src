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
static char *sccsid = "@(#)$RCSfile: capsar_get_buf.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:03:27 $";
#endif lint

/*
 * capsar_get_buf.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : int capsar_get_buf ( MM *m ,char *buf , int bufsize )
 *
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <syslog.h>
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

int capsar_get_buf(m,buf,bufsize)
MM	*m;	/* pointer to message structure */
char	*buf;
int	bufsize;
{
/*------*\
  Locals
\*------*/

	FILE	*fileptr;
	int	nwrite,i;
	char	*dataptr,
		*cps;
	
	cps = buf;
	if(m->offset >= m->size)return(NOTOK);
	if(m->offset + bufsize > m->size)nwrite = m->size - m->offset;
	else nwrite = bufsize;
	
	if(m->dataptr){
		dataptr = m->dataptr;
		dataptr = dataptr + m->offset;
	
		for(i=0;i<nwrite;i++)
			*cps++ = *dataptr++;
	/*	*cps++ = '\0';*/
	}
	else if(m->swapfile){
		if((fileptr = fopen(m->swapfile,"r")) == NULL){
			capsar_log(LOG_INFO,M_MSG_2,"cannot open %s for reading\n",m->swapfile); /*GAG*/
			return(NOTOK);
		}

		(void) fseek(fileptr,m->offset,0);
		
		if(fread(buf,1,nwrite,fileptr) == NULL){
			capsar_log(LOG_INFO,M_MSG_35,"write error on %s\n",m->swapfile); /*GAG*/
			return(NOTOK);
		}
		
		fclose(fileptr);
	}
	
	else {
		capsar_log(LOG_INFO,M_MSG_36,"empty message\n",NULL); /*GAG*/
		return(NOTOK);
	}
	m->offset += nwrite;
	if(nwrite < bufsize){
		cps = buf + nwrite;
		*cps++ = '\0';
	}
	return(nwrite);
}

