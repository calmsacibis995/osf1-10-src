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
static char	*sccsid = "@(#)$RCSfile: pwck.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/08 15:47:06 $";
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
/*
 * COMPONENT_NAME: CMDOPER: pwck
 *                                                                    
 * ORIGIN: IBM
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1988
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#include	<sys/types.h>
#include	<locale.h>
#include	<sys/param.h>
#include 	<sys/syslimits.h>
#include	<sys/signal.h>
#include	<sys/stat.h>
#include	<stdio.h>

#ifdef NLS
#include <NLctype.h>
#else
#include <ctype.h>
#endif
#ifdef MSG
#include "pwck_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) NLcatgets(catd, MS_PWCK, Num, Str)
#else
#define MSGSTR(Num, Str) Str
#endif

#define ERROR1  MSGSTR(ERR1, "Too many/few fields")
#define ERROR2  MSGSTR(ERR2, "Bad character(s) in logname")
#define ERROR2a MSGSTR(ERR2a, "First char in logname not lower case alpha")
#define ERROR2b MSGSTR(ERR2b, "Logname field NULL")
#define ERROR3  MSGSTR(ERR3, "Logname too long")
#define ERROR4  MSGSTR(ERR4, "Invalid UID")
#define ERROR5  MSGSTR(ERR5, "Invalid GID")
#define ERROR6  MSGSTR(ERR6, "Login directory not found")
#define ERROR6a MSGSTR(ERR6a, "Login directory null")
#define ERROR7  MSGSTR(ERR7, "Optional shell file not found")
#define ERROR8  MSGSTR(ERR8, "Flattened name not unique")

#define MAXUID  UID_MAX           /* max user id */
#define LOGBUF_LEN	80
#define INITPWDBUF 	9*300 /* buffer size for PASSWDFILE contents */
#define PASSWDFILE	"/etc/passwd"
#define NFIELDS		6    /* number of fields in PASSWDFILE */
#define MAXUID_DIGIT	5   /* max number of uid digit */
#define MAXGID_DIGIT	MAXUID_DIGIT   /* max number of gid digit */
#define MAXGID		UID_MAX	  /* max gid number */

int eflag, code=0;
int badc;
char buf[PATH_MAX];

/*
 * NAME: pwck [file]
 *                                                                    
 * FUNCTION: Checks the password file for inconsistences.
 */  
main(argc,argv)

int argc;
char **argv;

{
	int delim[PATH_MAX];
	char logbuf[LOGBUF_LEN];
	FILE *fopen(), *fptr;
	char *fgets();
	int error();
	struct	stat obuf;
	uid_t uid; 
        gid_t gid;
	int len;
	int i, j, colons;
	char *pw_file;

	int num;
	int namelen;
#ifdef NLS
	NLchar nlsbuf[PATH_MAX];
#endif
	char tbuf[PATH_MAX], *p, *pt;
	char  field[PATH_MAX];
	int ents = 0;
	int size = INITPWDBUF;
	char *mem, *malloc(), *realloc(), *strpbrk();

#ifdef NLS
	(void ) setlocale(LC_ALL,"");     /* set table for current lang */
#endif

#ifdef MSG
catd = NLcatopen(MF_PWCK, NL_CAT_LOCALE);
#endif
	if(argc == 1) pw_file=PASSWDFILE;
	else pw_file=argv[1];

	if((fptr=fopen(pw_file,"r"))==NULL) {
		fprintf(stderr,MSGSTR(CANTOPEN, "cannot open %s\n"),pw_file);
		exit(1);
	}
	if ((mem = malloc(size)) == NULL ) {
		fprintf(stderr, MSGSTR(OUTOFMEM, "out of memory\n"));
		exit(1);
	}
	
	while(fgets(buf,PATH_MAX,fptr)!=NULL) {
		namelen = 0;
		colons=0;
		badc=0;
		uid=gid=0l;
		eflag=0;
#ifdef NLS
		NCdecstr(buf, nlsbuf, PATH_MAX );
#endif
		strcpy(tbuf, buf );

	/*  Check number of fields */


		for(i=0 ; buf[i] !=  NULL ; i++) {
			if(buf[i]==':') {
				++colons;
			}
		}
		if(colons != NFIELDS) {
			error(ERROR1);
			continue;
		}

	/*  Check that first character is alpha and rest alphanumeric  */

		
		
			
		pt = tbuf;
		p = strpbrk(pt,":");
		*p++ = NULL;
#ifdef NLS
		NLflatstr(pt, field, strlen(pt)+1); 
#else
		strcpy(field, pt);
#endif

		if(!islower(field[0])) {
			error(ERROR2a);
		}
		if(field[0] == ':') {
			error(ERROR2b);
		}
		for(i=1; field[i] != NULL; i++) {
			if ( islower(field[i]) ) ;
			else if ( isdigit(field[i]) ) ;
			else ++badc;
		}
		if(badc > 0) {
			error(ERROR2);
		}

	/* Check out uniqueness of flattened name */

		for ( i=0; i < ents; i++ ) {
			if ( strcmp(field,mem+(i*9)) == 0 ) {
				error(ERROR8);
			}
		}
		if ( size <= ents*9 ) {
			size += 9*100;
			if ((mem = realloc(mem, size)) == NULL ) {
				fprintf(stderr,MSGSTR(OUTOFMEM, "out of memory\n"));
				exit(1);
			}
		}
		strcpy(mem+(ents*9), field);
		ents++;
			
			  
	/*  Check for valid number of characters in logname  */

#ifdef NLS
		for(i=0; nlsbuf[i] != ':'; i++) {
			(NCchrlen(nlsbuf[i]) > 1) ? namelen += 2 : namelen++;
		}
#else
		namelen = strlen(field);
#endif

		if(namelen == 0)
			error(ERROR2b);

		if (namelen > 8)
			error(ERROR3);

	/*  Skip over passwd and check that UID is numeric and <= 65535  */
		
		pt = p;
		p = strpbrk(pt,":");
		*p++ = NULL;
		pt = p;
		p = strpbrk(pt,":");
		*p++ = NULL;
		strcpy( field, pt );

		len = strlen( field );
		if(len > MAXUID_DIGIT) {
			error(ERROR4);
		}
		else {
		    for (i=0; field[i] != NULL; i++ ) {
			if(!(isdigit(field[i]))) {
				error(ERROR4);
				break;
			}
			uid = uid*10+(field[i])-'0';
		    }
		    if(uid > MAXUID  ||  uid < 0l) {
			error(ERROR4);
		    }
		}

	/*  Check that GID is numeric and <= 65535  */

		pt = p;
		p = strpbrk(pt,":");
		*p++ = NULL;
		strcpy( field, pt );

		len = strlen( field );
		if(len > MAXGID_DIGIT) {
			error(ERROR5);
		}
		else {
		    for (i=0; field[i] != NULL; i++ ) {
			if(!(isdigit(field[i]))) {
				error(ERROR5);
				break;
			}
			gid = gid*10+(field[i])-'0';
		    }
		    if(gid > MAXGID  ||  gid < 0l) {
			error(ERROR5);
		    }
		}

	/*  Skip info field and stat initial working directory  */

		pt = p;
		p = strpbrk(pt,":");
		*p++ = NULL;
		pt = p;
		p = strpbrk(pt,":");
		*p++ = NULL;
		strcpy(field, pt );

		if((stat(field,&obuf)) == -1) {
			error(ERROR6);
		}
		if(field[0] == NULL) { /* Currently OS translates */
			error(ERROR6a);   /*  "/" for NULL field */
		}

	/*  Stat of program to use as shell  */

		pt = p;
		p = strpbrk( pt,"\n"); 
		*p = '\0';
		*p = NULL;
		strcpy(field, pt );

		if((stat(field,&obuf)) == -1) {
			error(ERROR7);
		}

	}
	fclose(fptr);
	exit(code);
}

/*
 * NAME: error
 *                                                                    
 * FUNCTION:   Error printing routine 
 */  
error(msg)
char *msg;
{
	if(!(eflag)) {
		fprintf(stdout,"\n%s",buf);
		code = 1;
		++eflag;
	}
	fprintf(stdout,"\t%s\n",msg);
	badc=0;
	return;
}
