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
static char *sccsid = "@(#)$RCSfile: capsar_message_type.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/12/21 15:01:08 $";
#endif lint

/*
 * capsar_message_type.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : int capsar_message_type (MM *m)
 *
 * This routine returns an integer denoting the type of the mail message
 *	EMPTY	          The message is empty
 *	SIMPLE_MESSAGE	  The message consists of exactly one body part.
 *	COMPOUND_MESSAGE  The mesage consists of two+ body parts 
 *	MAIL_MESSAGE      The message contains a content header and one
 *                        or more body parts.
 *      MULTIPLE_FORMAT   The message contains two or more representations
 *                        of the same document : plus optional comments and,
 *                        usually, a master copy of the document.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <capsar.h>
#include "cdoc.h"

capsar_message_type(m)
MM	*m;	/* pointer to message structure */
{
/*------*\
  Locals
\*------*/
	struct cdoc	*tp;
	char		*s,*tcp,*s1;
	char		*first_char();	
	char		lower_case();
	char		**cps,*cp;
	
	if(m==NULL)return(EMPTY);

	s = m->start;	
	for(tp=message_tag;tcp=tp->message;tp++){
		s1=s;	
		while(s1 != NULL){
			s1=first_char(s1,*tcp);
			if(s1==NULL)break;
			if(compare(s1,tcp,strlen(tcp))==0){
				return(tp->m_type);
			}
			if(s1)s1++;
		}
		
			
	}
	
	cps =capsar_get_header_list(m);
	if(cps != NULL){
		while(cp = *cps++)
			if(cp)free(cp);
		return(MAIL_MESSAGE);
	}

	return(SIMPLE_MESSAGE);

}

compare(s1,s2,n)
register char	*s1,*s2;
register int	n;
{
	while(--n >= 0 && lower_case(*s1)==lower_case(*s2++))
		if(*s1++ == '\0')
			return(0);
	return(n<0 ? 0 : *s1 - *--s2);
}
char lower_case(c)
char	c;
{
char    lc = c;

        if ( c && isalnum(c) )
	   lc = isupper(c) ? tolower(c) : c;

        return( c );
}

char *first_char(sp,c)
register char	*sp;
char		c;
{
	do {
		if(lower_case(*sp)==lower_case(c))
			return(sp);
	} while (*sp++ != '\0');

	return(NULL);
}


