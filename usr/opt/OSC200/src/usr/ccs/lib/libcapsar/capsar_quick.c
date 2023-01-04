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
static char *sccsid = "@(#)$RCSfile: capsar_quick.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/09/29 12:34:28 $";
#endif lint

/*
 * capsar_quick.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : int capsar_quick ( FILE *fileptr )
 *
 * This routine has a quick look at a mail file and gives an
 * estimate of the type of mail message . It does this by looking at the
 * first text line after the headers. If the first text line is an 
 * encapsulation boundery of the correct type then  the mail message is
 * assumed to be a single bodied CDA document and capsar_parse_file must
 * called. Otherwise the message is assumed to be text which can be
 * displayed on the screen.
 * 
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <syslog.h>
#include <capsar.h>
#include "./dtif.h"
#include "libcapsar_msg.h" /*GAG*/

char		*hd[1000];

int capsar_quick(fileptr)
FILE	*fileptr;
{
/*------*\
  Locals
\*------*/
	char	buffer[BUFSIZ],
		*ebpos,
		*gettag();
	int	chlevel,
		eblength,
		check_list();

	if(check_list(fileptr) == NOTOK){
		capsar_log(LOG_INFO,M_MSG_76,"no/bad headers in mail message\n",NULL); /*GAG*/
		(void)fflush(fileptr);		/* XPG4 compliance */
		rewind(fileptr);
		return(NOTOK);
	}	

	while(fgets(buffer,sizeof buffer,fileptr) !=NULL){
		if(!check(buffer,strlen(buffer))){
			if(eb(buffer,0,&chlevel,&eblength) == OK){
				ebpos = gettag(buffer,eblength);

				if(match(strlen(DDIFTAG),DDIFTAG,strlen(ebpos),ebpos)){
		(void)fflush(fileptr);		/* XPG4 compliance */
					rewind(fileptr);
					return(CDA);
				}
				else if(match(strlen(DTIFTAG),DTIFTAG,strlen(ebpos),ebpos)){
		(void)fflush(fileptr);		/* XPG4 compliance */
					rewind(fileptr);
					return(CDA);
				}
				else if(match(strlen(DOTSTAG),DOTSTAG,strlen(ebpos),ebpos)){
		(void)fflush(fileptr);		/* XPG4 compliance */
					rewind(fileptr);
					return(CDA);
				}
				else {
		(void)fflush(fileptr);		/* XPG4 compliance */
					rewind(fileptr);
					return(TEXT);
				}
			}
			else {
		(void)fflush(fileptr);		/* XPG4 compliance */
				rewind(fileptr);
				return(TEXT);
			}
		}
	}

		(void)fflush(fileptr);		/* XPG4 compliance */
	rewind(fileptr);
	return(TEXT);
		
}

int check_list(fileptr)
FILE	*fileptr;
{
	char	buf[BUFSIZ];
	int	eoh=0,
		cont_poss=0,
		state,ret;

	

	while(!eoh){
		state=get_line_from_file_or_core(buf,sizeof buf,fileptr,1,NULL);
		switch (state) {

			case EOA :
				eoh++ ;
				break;

			case ANYTEXT :
				cont_poss=0;
				ret = getstr(buf,DEF_SEP);
				if(ret == HEADER){
					cont_poss++;
				}
				else if(ret == EOA)
					eoh++;
				else return(NOTOK);
				break;
			case CONT_POSS :
				if(!cont_poss){
					return(NOTOK);
				}
				break;
			case EOM :
				return(NOTOK);
		}
	}
	return(OK);
}

check(buffer,n)
char	*buffer;
int	n;

{
	while(--n >0 ){
		if( *buffer == ' ' || *buffer == '\r')
			buffer++;
		else break;
	}

	if(*buffer == '\n')return(1);
	else return(0);
}


	

