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
static char	*sccsid = "@(#)$RCSfile: ruserpass.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:34:57 $";
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/types.h>
#include <stdio.h>
#include <utmp.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include "ftp_var.h"

char	*renvlook(), *malloc(), *index(), *getenv(), *getpass(), *getlogin();
char	*strcpy();
struct	utmp *getutmp();
static	FILE *cfile;
static	token();

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	ACCOUNT 4
#define MACDEF  5
#define	ID	10
#define	MACHINE_NAME	11

static char tokval[100];

static struct toktab {
	char *tokstr;
	int tval;
} toktab[]= {
	"default",	DEFAULT,
	"login",	LOGIN,
	"password",	PASSWD,
	"passwd",	PASSWD,
	"account",	ACCOUNT,
	"machine",	MACHINE_NAME,
	"macdef",	MACDEF,
	0,		0
};

ruserpass(host, aname, apass, aacct)
	char *host, **aname, **apass, **aacct;
{
	char *hdir, buf[BUFSIZ], *tmp;
	char myname[MAXHOSTNAMELEN], *mydomain;
	int t, i, c, usedefault = 0;
	struct stat stb;
	extern int errno;

	hdir = getenv("HOME");
	if (hdir == NULL)
		hdir = ".";
	(void) sprintf(buf, MSGSTR(NETRC, "%s/.netrc"), hdir);
	cfile = fopen(buf, "r");
	if (cfile == NULL) {
		if (errno != ENOENT)
			perror(buf);
		return(0);
	}
	if (gethostname(myname, sizeof(myname)) < 0)
                myname[0] = '\0';
        if ((mydomain = index(myname, '.')) == NULL)
                mydomain = "";
next:
	while ((t = token())) switch(t) {

	case DEFAULT:
		 usedefault = 1;
                /* FALL THROUGH */

	case MACHINE_NAME:
		 if (!usedefault) {
			if (token() != ID || strcasecmp(host, tokval))
                                continue;
                        /*
                         * Allow match either for user's input host name
                         * or official hostname.  Also allow match of
			 * incompletely-specified host in local domain.
                         */
                        if (strcasecmp(host, tokval) == 0)
                                goto match;
                        if (strcasecmp(hostname, tokval) == 0)
                                goto match;
			if ((tmp = index(hostname, '.')) != NULL &&
                            strcasecmp(tmp, mydomain) == 0 &&
                            strncasecmp(hostname, tokval, tmp-hostname) == 0 &&
                            tokval[tmp - hostname] == '\0')
                                goto match;
			if ((tmp = index(host, '.')) != NULL &&
                            strcasecmp(tmp, mydomain) == 0 &&
                            strncasecmp(host, tokval, tmp - host) == 0 &&
                            tokval[tmp - host] == '\0')
                                goto match;
			continue;
		}
	match:
		while ((t = token()) && t != MACHINE_NAME && t != DEFAULT) switch(t) {

		case LOGIN:
			if (token())
				if (*aname == 0) { 
					*aname = malloc((unsigned) strlen(tokval) + 1);
					(void) strcpy(*aname, tokval);
				} else {
					if (strcmp(*aname, tokval))
						goto next;
				}
			break;
		case PASSWD:
			if (strcmp(*aname, "anonymous") &&
			   (fstat(fileno(cfile), &stb) >= 0
			   && (stb.st_mode & 077) != 0)) {
	fprintf(stderr, MSGSTR(NETRC_ERR, "Error - .netrc file not correct mode.\n"));
	fprintf(stderr, MSGSTR(NETRC_ERR_PASS, "Remove password or correct mode.\n"));
				goto bad;
			}
			if (token() && *apass == 0) {
				*apass = malloc((unsigned) strlen(tokval) + 1);
				(void) strcpy(*apass, tokval);
			}
			break;
		case ACCOUNT:
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0) {
	fprintf(stderr, MSGSTR(NETRC_ERR, "Error - .netrc file not correct mode.\n"));
	fprintf(stderr, MSGSTR(NETRC_ERR_ACCT, "Remove account or correct mode.\n"));
				goto bad;
			}
			if (token() && *aacct == 0) {
				*aacct = malloc((unsigned) strlen(tokval) + 1);
				(void) strcpy(*aacct, tokval);
			}
			break;
		case MACDEF:
			if (proxy) {
				(void) fclose(cfile);
				return(0);
			}
			while ((c=getc(cfile)) != EOF && c == ' ' || c == '\t');
			if (c == EOF || c == '\n') {
				printf(MSGSTR(MDEF, "Missing macdef name argument.\n"));
				goto bad;
			}
			if (macnum == 16) {
				printf(MSGSTR(MACRO_LIMIT, "Limit of 16 macros have already been defined\n"));
				goto bad;
			}
			tmp = macros[macnum].mac_name;
			*tmp++ = c;
			for (i=0; i < 8 && (c=getc(cfile)) != EOF &&
			    !isspace(c); ++i) {
				*tmp++ = c;
			}
			if (c == EOF) {
				printf(MSGSTR(MACRO_DEF, "Macro definition missing null line terminator.\n"));
				goto bad;
			}
			*tmp = '\0';
			if (c != '\n') {
				while ((c=getc(cfile)) != EOF && c != '\n');
			}
			if (c == EOF) {
				printf(MSGSTR(MACRO_DEF, "Macro definition missing null line terminator.\n"));
				goto bad;
			}
			if (macnum == 0) {
				macros[macnum].mac_start = macbuf;
			}
			else {
				macros[macnum].mac_start = macros[macnum-1].mac_end + 1;
			}
			tmp = macros[macnum].mac_start;
			while (tmp != macbuf + 4096) {
				if ((c=getc(cfile)) == EOF) {
				printf(MSGSTR(MACRO_DEF, "Macro definition missing null line terminator.\n"));
					goto bad;
				}
				*tmp = c;
				if (*tmp == '\n') {
					if (*(tmp-1) == '\0') {
					   macros[macnum++].mac_end = tmp - 1;
					   break;
					}
					*tmp = '\0';
				}
				tmp++;
			}
			if (tmp == macbuf + 4096) {
				printf(MSGSTR(MBUF_EXCEEDED, "4K macro buffer exceeded\n"));
				goto bad;
			}
			break;
		default:
	fprintf(stderr, MSGSTR(NETRC_KEYWORD, "Unknown .netrc keyword %s\n"), tokval);
			break;
		}
		goto done;
	}
done:
	(void) fclose(cfile);
	return(0);
bad:
	(void) fclose(cfile);
        return(-1);
	
}

static
token()
{
	char *cp;
	int c;
	struct toktab *t;

	if (feof(cfile))
		return (0);
	while ((c = getc(cfile)) != EOF &&
	    (c == '\n' || c == '\t' || c == ' ' || c == ','))
		continue;
	if (c == EOF)
		return (0);
	cp = tokval;
	if (c == '"') {
		while ((c = getc(cfile)) != EOF && c != '"') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	} else {
		*cp++ = c;
		while ((c = getc(cfile)) != EOF
		    && c != '\n' && c != '\t' && c != ' ' && c != ',') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	}
	*cp = 0;
	if (tokval[0] == 0)
		return (0);
	for (t = toktab; t->tokstr; t++)
		if (!strcmp(t->tokstr, tokval))
			return (t->tval);
	return (ID);
}
