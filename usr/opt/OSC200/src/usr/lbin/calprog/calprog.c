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
static char rcsid[] = "@(#)$RCSfile: calprog.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/09/03 03:37:26 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: calprog
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.10  com/cmd/misc/calendar/calprog.c, cmdmisc, bos320, 9132320b 7/30/91 15:56:12
 */
/*
 *	calprog.c - used by /usr/bin/calendar command.
 *
 *  NAME: calprog
 *                                                                     
 *  FUNCTION:  /usr/lib/calprog produces an egrep -f file
 *             that will select today's and tomorrow's
 *             calendar entries, with special weekend provisions
 *             used by calendar command
 */                                                                    

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>
#include <langinfo.h>
#include <ctype.h>
char *nlfile;
int dayfirst=0;
char sep [  3];
char dfmt[256];

#include <time.h>

#define DAY (3600*24L)

struct mname{
	wchar_t c1;		/* First character of month name */
	wchar_t c1_alt;		/* First character of month name (2nd case) */
	wchar_t c1_ab;		/* First character of short month name */
	wchar_t c1_ab_alt;	/* First character of short month (2nd case) */
	char *rest;		/* The rest of the month name  (multi-byte) */
	char *rest_ab;		/* The rest of the abbreviated month name */
} mname;

nl_item months[] = {
	MON_1, MON_2, MON_3, MON_4, MON_5, MON_6,
	MON_7, MON_8, MON_9, MON_10, MON_11, MON_12
};

nl_item abmonths[] = {
	ABMON_1, ABMON_2, ABMON_3, ABMON_4, ABMON_5, ABMON_6,
	ABMON_7, ABMON_8, ABMON_9, ABMON_10, ABMON_11, ABMON_12
};

tprint(t)
time_t t;
{
	struct tm *tm;
	int c1size;
	char *s;
	int mb_cur_max = MB_CUR_MAX;

	tm = localtime(&t);

	/* Get info about month name */
	s=nl_langinfo(months[tm->tm_mon]); 
	while (isspace(*s)) s++ ;	/* Skip space */
	if ((mname.rest = (char *)malloc(strlen(s))) != NULL) {
		c1size = mbtowc(&mname.c1, s, mb_cur_max);
		s += c1size;	/* Skip the first character */
		strcpy(mname.rest, s);
		if (iswupper(mname.c1))
			mname.c1_alt = towlower(mname.c1);
		else
			mname.c1_alt = towupper(mname.c1);
	}

	/* Get info about abbreviated month name */
	s=nl_langinfo(abmonths[tm->tm_mon]); 
	while (isspace(*s)) s++ ;	/* Skip space */
	if ((mname.rest_ab = (char *)malloc(strlen(s))) != NULL) {
		c1size = mbtowc(&mname.c1_ab, s, mb_cur_max);
		s += c1size;	/* Skip the first character */
		strcpy(mname.rest_ab, s);
		if (iswupper(mname.c1_ab))
			mname.c1_ab_alt = towlower(mname.c1_ab);
		else
			mname.c1_ab_alt = towupper(mname.c1_ab);
	}

	/*
	 * Note: The regular expression to be printed is broken up into
	 * 3 printf's because SCCS expands "<percent>C<percent>".
	 */
 	/* dayfirst will never be set, see below */
	if (dayfirst) {
	  printf("(^|[[:blank:][:punct:]])(0*%d([[:punct:]]|[[:blank:]]+|%s)((",
		tm->tm_mday, sep);
	  if (mname.c1 != mname.c1_alt)
	  {
		printf("[%C", mname.c1    ) ;
		printf("%C]", mname.c1_alt) ;
	  }
	  else
		printf("%C", mname.c1) ;

	  printf("%s[[:punct:]]?[[:blank:]]*)", mname.rest);
	  if ((mname.c1_ab == mname.c1) &&
	      (strcmp(mname.rest_ab, mname.rest) == 0))
		;	/* Skip the abbr. month entry */
	  else if (mname.c1_ab != mname.c1_ab_alt)
	  {
		printf("|([%C", mname.c1_ab    ) ;
		printf("%C]%s[[:punct:]]?[[:blank:]]*)",
			mname.c1_ab_alt, mname.rest_ab) ;
	  }
	  else
		printf("|(%C%s[[:punct:]]?[[:blank:]]*)",
			mname.c1_ab_alt, mname.rest_ab) ;

	  printf("|0*%d|[*]))([^0123456789]|$)\n", tm->tm_mon + 1);
	} else {
	  printf("(^|[[:blank:][:punct:]]*)((");
	  if (mname.c1 != mname.c1_alt)
	  {
		printf("[%C", mname.c1    ) ;
		printf("%C]", mname.c1_alt) ;
	  }
	  else
		printf("%C", mname.c1) ;
	  printf("%s[[:punct:]]?[[:blank:]]*)", mname.rest);
	  if ((mname.c1_ab == mname.c1) &&
	      (strcmp(mname.rest_ab, mname.rest) == 0))
		;	/* Skip the abbr. month entry */
	  else if (mname.c1_ab != mname.c1_ab_alt)
	  {
		printf("|([%C", mname.c1_ab    ) ;
		printf("%C]%s[[:punct:]]?[[:blank:]]*)",
			mname.c1_ab_alt, mname.rest_ab) ;
	  }
	  else
		printf("|(%C%s[[:punct:]]?[[:blank:]]*)",
			mname.c1_ab, mname.rest_ab) ;
	  printf("|(0*%d%s)|([*]%s|[*] ))0*%d([^0123456789]|$)\n",
		tm->tm_mon + 1,sep,sep,tm->tm_mday);
	}
}

main()
{
	time_t t;
	int i;

	(void) setlocale(LC_ALL,"");
        if ((nlfile = nl_langinfo(D_FMT)) != NULL) {
		i=0;
		while(nlfile[i++] != '%');		
 
		if ((nlfile[i] == 'E') || (nlfile[i] == 'O'))
			i++ ;	/* Skip %E? & %O? format */
		if (nlfile[i++] == 'd')
 			 dayfirst++;
		if (nlfile[i] == '.' || nlfile[i] == '*' || nlfile[i] == '|'
				|| nlfile[i] == '^' || nlfile[i] == '\\') {
			sep[0] = '\\';	/* must escape special character */
			sep[1] = nlfile[i];
		}
		else if (isascii(nlfile[i]) && !isspace(nlfile[i]) &&
			(nlfile[i] != '%'))
			sep[0] = nlfile[i];
		else
			sep[0] = '/' ;	/* Default is '/' */
	}
	time(&t);
	tprint(t);
	switch(localtime(&t)->tm_wday) {
	/* Cases 5 and 6 take care of the weekend. */
  	case 5:
  		t += DAY;
        	tprint(t);
   	case 6:
  		t += DAY;
  		tprint(t);
	default:
		t += DAY;
		tprint(t);
	}
}
