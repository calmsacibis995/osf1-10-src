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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: fwtmp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/10/19 11:12:46 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */ 
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: inp
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include <stdio.h>
# include <sys/types.h>
# include "acctdef.h"
# include <sys/acct.h>
# include <locale.h>

static int	inp();
struct	utmp	Ut;
char    timbuf[BUFSIZ];
struct tm *localtime();

#define OFMT "%-*.*s %-*.*s %-*.*s %*.*hd %*.*hd %*.*ho %*.*ho %10ld %-*.*s %s"  /* pjd 101 */

main ( c, v, e )
char	**v, **e;
int	c;
{

	int	iflg,cflg,count;

	(void) setlocale (LC_ALL,"");
	iflg = cflg = count = 0;

	if ( --c > 0 ) {
		if(**++v == '-') while(*++*v) switch(**v){
		case 'c':
			cflg++;
			continue;
		case 'i':
			iflg++;
			continue;
		}
	}

	for(;;){
		if(iflg){
			if(inp(stdin,&Ut) == EOF)
				break;
		} else {
			if(fread(&Ut,sizeof Ut, 1, stdin) != 1)
				break;
		}
		if(cflg)
			(void)fwrite(&Ut,sizeof Ut, 1, stdout);
		else {
	/*
	 * The utmp string elements use the whole array, so there need not be
	 * a trailing \0. Therefore string output is EXACTLY the field size. 
	 */
			(void)strftime(timbuf, BUFSIZ, "%c %Z %n", 
				localtime(&Ut.ut_time));
			(void)printf( OFMT,
				NSZ, NSZ, Ut.ut_user,
				ISZ, ISZ, Ut.ut_id,
				LSZ, LSZ, Ut.ut_line,
				PSZ, PSZ, Ut.ut_pid,
				TSZ, TSZ, Ut.ut_type,
				ETSZ, ETSZ, Ut.ut_exit.e_termination,
				EESZ, EESZ, Ut.ut_exit.e_exit,
				"Ut.ut_time",
				HSZ, HSZ, Ut.ut_host,
				timbuf);
		}
	}
	return ( 0 );
}

static
inp(file, u)
FILE *file;
register struct utmp *u;
{

	char	buf[BUFSIZ];
	register char *p;
	register int i, len;

	if(fgets((p = buf), BUFSIZ, file)==NULL)
		return EOF;
	len = strlen(p);
	/*
	 * There can be empty input strings! So we can't use sscanf("..%s...").
	 * Fill the strings with \0 from the end.
	 */
	strncpy(u->ut_user, p, NSZ);
	for( i = NSZ-1; i >= 0 && u->ut_user[i] == ' '; i-- ) u->ut_user[i] = '\0';
        p += (NSZ+1);
        if((p-buf) > len)       return 0;

        strncpy(u->ut_id, p, ISZ);
	for( i = ISZ-1; i >= 0 && u->ut_id[i] == ' '; i-- ) u->ut_id[i] = '\0';
        p += (ISZ+1);
        if((p-buf) > len)       return 0;

        strncpy(u->ut_line, p, LSZ);
	for( i = LSZ-1; i >= 0 && u->ut_line[i] == ' '; i-- ) u->ut_line[i] = '\0';
        p += (LSZ+1);
        if((p-buf) > len)       return 0;

        u->ut_pid = (pid_t) atol(p);
        p += (PSZ+1);
        if((p-buf) > len)       return 0;

        u->ut_type = (short) atoi(p);
        p += (TSZ+1);
        if((p-buf) > len)       return 0;

        u->ut_exit.e_termination = (short) atoi(p);
        p += (ETSZ+1);
        if((p-buf) > len)       return 0;

        u->ut_exit.e_exit = (short) atoi(p);
        p += (EESZ+1);
        if((p-buf) > len)       return 0;

        u->ut_time = atol(p);
        p += (TISZ+1);
        if((p-buf) > len)      return 0;

        strncpy(u->ut_host, p, HSZ);
	for( i = HSZ-1; i >= 0 && u->ut_host[i] == ' '; i-- ) u->ut_host[i] = '\0';
        return 0;
}
