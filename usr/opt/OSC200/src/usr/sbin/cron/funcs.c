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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: funcs.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 14:22:46 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: funcs
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.9  com/cmd/cntl/cron/funcs.c, cmdcntl, bos320, 9125320 6/8/91 21:22:40
 */

/*
 * module used for crontab, cron, and at
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "cron_msg.h"

extern nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)

/*****************/
time_t num(ptr)
/*****************/
char **ptr;
{
	time_t n=0;
	while (isdigit(**ptr)) {
		n = n*10 + (**ptr - '0');
		*ptr += 1; }
	return(n);
}


int dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};

/*****************/
days_btwn(m1,d1,y1,m2,d2,y2)
/*****************/
int m1,d1,y1,m2,d2,y2;
{
	/* calculate the number of "full" days in between m1/d1/y1 and m2/d2/y2.
	   NOTE: there should not be more than a year separation in the dates.
		 also, m should be in 0 to 11, and d should be in 1 to 31.    */
	
	int days,m;

	if ((m1==m2) && (d1==d2) && (y1==y2)) return(0);
	if ((m1==m2) && (d1<d2)) return(d2-d1-1);
	/* the remaining dates are on different months */
	days = (days_in_mon(m1,y1)-d1) + (d2-1);
	m = (m1 + 1) % 12;
	while (m != m2) {
		if (m==0) y1++;
		days += days_in_mon(m,y1);
		m = (m + 1) % 12; }
	return(days);
}


/*****************/
days_in_mon(m,y)
/*****************/
int m,y;
{
	/* returns the number of days in month m of year y
	   NOTE: m should be in the range 0 to 11	*/

	return( dom[m] + (((m==1)&&((y%4)==0)) ? 1:0 ));
}

/*****************/
char *xmalloc(size)
/*****************/
int size;
{
	char *p;

	if((p=(char *) malloc((size_t)size)) == NULL) {
		fprintf(stderr, MSGSTR(MS_XMALLOC, 
				"cannot allocate %d bytes of space\n"),size);
		fflush(stderr);
		exit(55);
	}
	return p;
}

/*****************/
char *xrealloc(p, size)
/*****************/
char *p;
int size;
{

	if((p= (char *) realloc((void *)p, (size_t)size)) == NULL) {
		printf("cannot allocate %d bype of space\n",size);
		fflush(stdout);
		exit(55);
	}
	return p;
}

/*****************/
char *cat2(s1, s2)
/*****************/
char *s1, *s2;
{
    return (strcat(strcpy(xmalloc((int)(strlen(s1)+strlen(s2)+1)), s1), s2));
}

/*****************/
static int sameenv(s, t)
/*****************/
char *s, *t;
{
    while (*s != '\0' && *s != '=')
	if (*s++ != *t++) return (0);
    return (*t == '\0' || *t == '=');
}

/*****************/
/* void putenv(s)		NOT USED */
/*****************/
/* char *s;
{
    extern char **environ;
    static char **envmalloced = NULL;
    static char *nullenv[1] = { NULL };
    register char **ep;
    if (environ == NULL) environ = nullenv;
    ep = environ;
    for (ep = environ; *ep != NULL && !sameenv(*ep, s); ++ep);
    if (*ep == NULL) {
	register char **newenv;
	if (environ == envmalloced) {
	    newenv = (char **)xrealloc((char *)environ,
		    (ep-environ+2)*sizeof(char *));
	} else {
	    newenv = (char **)memcpy((void *)xmalloc((ep-environ+2)*sizeof(char *)), (void *)environ, (size_t)((ep-environ)*sizeof(char *)));
	}
	ep = newenv + (ep - environ);
	envmalloced = environ = newenv;
	ep[1] = NULL;
    }
    *ep = s;
}
*/
