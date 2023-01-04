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
static char	*sccsid = "@(#)$RCSfile: ruserpass.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/23 18:30:13 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * ruserpass.c	5.5 (Berkeley) 6/27/88
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak mkpwclear = __mkpwclear
#pragma weak mkpwunclear = __mkpwunclear
#pragma weak ruserpass = __ruserpass
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <nl_types.h>
#include "libc_msg.h"
static nl_catd catd;

#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _environ_rmutex;
#endif	/* _THREAD_SAFE */

void	mkpwclear(char *, char, char *);
void	mkpwunclear(char *, char, char *);
char	*index(), *malloc(), *getenv(), *getpass(), *getlogin();
static char	*renvlook();

static	renv(), rnetrc(), token(), getutmp();

void
ruserpass(char *host, char **aname, char **apass)
{

#ifdef	_THREAD_SAFE
	char myname_buf[16];
	char *myname = myname_buf;
#endif	/* _THREAD_SAFE */

	if (renv(host, aname, apass))
		return;
	if ((*aname == 0 || *apass == 0) && rnetrc(host, aname, apass))
		return;
	if (*aname == 0) {
#ifdef	_THREAD_SAFE
		getlogin_r(myname_buf, 16);
#else
		char *myname = getlogin();
#endif	/* _THREAD_SAFE */
		if (!(*aname = malloc(16)))
			return;
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                printf(catgets(catd,LIBCNET,NET33,
                                        "Name (%s:%s): "), host, myname);
		fflush(stdout);
		if (read(2, *aname, (unsigned)16) <= 0)
			exit(1);
		if ((*aname)[0] == '\n')
#ifdef	_THREAD_SAFE
			strncpy(*aname, myname_buf, 16);
#else
			*aname = myname;
#endif	/* _THREAD_SAFE */
		else
			if (index(*aname, '\n'))
				*index(*aname, '\n') = 0;
	}
	if (*aname && *apass == 0) {
#ifdef	_THREAD_SAFE
		/*
	 	 * Since getpass() is not a thread-safe function, this code
	 	 * has to be out.
	 	 */
		exit(1);
#else
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                printf(catgets(catd,LIBCNET,NET34,
                                        "Password (%s:%s): "), host, *aname);
		fflush(stdout);
		*apass = getpass("");
#endif	/* _THREAD_SAFE */
	}
}


static int
renv(char *host, char **aname, char **apass)
{
	register char *cp;
	char *comma;
	int aname_allocd = 0;

	cp = renvlook(host);
	if (cp == NULL)
		return (0);
	if (!isalpha(cp[0]))
		return (0);
	comma = index(cp, ',');
	if (comma == 0)
		return (0);
	if (*aname == 0) {
		if (!(*aname = malloc(comma - cp + 1)))
			return (-1);
		aname_allocd = 1;
		strncpy(*aname, cp, (size_t)(comma - cp));
	} else
		if (strncmp(*aname, cp, (size_t)(comma - cp)))
			return (0);
	comma++;
	if (!(cp = malloc(strlen(comma)+1))) {
		if (aname_allocd) {
			free(*aname);
			*aname = 0;
		}
		return (-1);
	}
	strcpy(cp, comma);
	if (!(*apass = malloc(16))) {
		if (aname_allocd) {
			free(*aname);
			*aname = 0;
		}
		free(cp);
		return (-1);
	}
	mkpwclear(cp, host[0], *apass);
	return (0);
}


static char *
renvlook(char *host)
{
	register char *cp, **env;
	extern char **environ;

	TS_LOCK(&_environ_rmutex);
	env = environ;
	for (env = environ; *env != NULL; env++)
		if (!strncmp(*env, "MACH", (size_t)4)) {
			cp = index(*env, '=');
			if (cp == 0)
				continue;
			if (strncmp(*env+4, host, (size_t)(cp-(*env+4))))
				continue;
			TS_UNLOCK(&_environ_rmutex);
			return (cp+1);
		}
        TS_UNLOCK(&_environ_rmutex);
	return (NULL);
}

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	NOTIFY	4
#define	WRITE	5
#define	YES	6
#define	NO	7
#define	COMMAND	8
#define	FORCE	9
#define	ID	10
#define	MACHINE	11


static struct toktab {
	char *tokstr;
	int tval;
} toktab[]= {
	"default",	DEFAULT,
	"login",	LOGIN,
	"password",	PASSWD,
	"notify",	NOTIFY,
	"write",	WRITE,
	"yes",		YES,
	"y",		YES,
	"no",		NO,
	"n",		NO,
	"command",	COMMAND,
	"force",	FORCE,
	"machine",	MACHINE,
	0,		0
};


static int
rnetrc(char *host, char **aname, char **apass)
{
	char *hdir, buf[BUFSIZ];
	int t;
	struct stat stb;

	char tokval[100];
	FILE *cfile;

	hdir = getenv("HOME");
	if (hdir == NULL)
		hdir = ".";
	(void)sprintf(buf, "%s/.netrc", hdir);
	cfile = fopen(buf, "r");
	if (cfile == NULL) {
		if (_Geterrno() != ENOENT)
			perror(buf);
		return (0);
	}
next:
	while ((t = token(cfile, tokval))) switch(t) {

	case DEFAULT:
		(void) token(cfile, tokval);
		continue;

	case MACHINE:
		if (token(cfile, tokval) != ID || strcasecmp(host, tokval))
			continue;
		while ((t = token(cfile, tokval)) && t != MACHINE) switch(t) {

		case LOGIN:
			if (token(cfile, tokval))
				if (*aname == 0) { 
					if (!(*aname =
					      malloc(strlen(tokval) + 1)))
						return (-1);
					strcpy(*aname, tokval);
				} else {
					if (strcmp(*aname, tokval))
						goto next;
				}
			break;
		case PASSWD:
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0) {
                                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                                fprintf(stderr, catgets(catd,LIBCNET,NET29,
                                   "Error - .netrc file not correct mode.\n"));
                                fprintf(stderr, catgets(catd,LIBCNET,NET30,
                                   "Remove password or correct mode.\n"));
				exit(1);
			}
			if (token(cfile, tokval) && *apass == 0) {
				if (!(*apass = malloc(strlen(tokval) + 1)))
					return (-1);
				strcpy(*apass, tokval);
			}
			break;
		case COMMAND:
		case NOTIFY:
		case WRITE:
		case FORCE:
			(void) token(cfile, tokval);
			break;
		default:
                        catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                        fprintf(stderr, catgets(catd,LIBCNET,NET31,
                                        "Unknown .netrc option %s\n"), tokval);
			break;
		}
		goto done;
	}
done:
	fclose(cfile);
	return (0);
}


static int
token(FILE *cfile, char *tokval)
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

/*
	getutmp()
	return a pointer to the system utmp structure associated with
	terminal sttyname, e.g. "/dev/tty3"
	Is version independent-- will work on v6 systems
	return NULL if error
*/
static int
getutmp(char *sttyname, struct utmp *utmpstr_ptr)
{
	FILE *fdutmp;

	if (sttyname == NULL || sttyname[0] == 0)
		return (-1);

	if ((fdutmp = fopen("/etc/utmp","r")) == NULL)
		return (-1);

	while (fread((void *)utmpstr_ptr, (size_t)1,
	       (size_t)sizeof(struct utmp), fdutmp) == sizeof(struct utmp))
		if (strcmp(utmpstr_ptr->ut_line, sttyname+5) == 0){
			fclose(fdutmp);
			return(0);
		}
	fclose(fdutmp);
	return (-1);
}


static void
sreverse(register char *sto, register char *sfrom)
{
	register int i;

	i = strlen(sfrom);
	while (i >= 0)
		*sto++ = sfrom[i--];
}


#ifdef _THREAD_SAFE
#define	TTYNAME(n, s)	ttyname_r(n, s, sizeof(s))
#else
#define	TTYNAME(n, s)	strcpy(s, ttyname(n))
#endif	/* _THREAD_SAFE */


static char *
mkenvkey(char mch, char *skey)
{
	struct utmp putmp;
	char stemp[40], stemp1[40], sttyname[30];
	register char *sk,*p;

	if (isatty(2))
		TTYNAME(2, sttyname);
	else if (isatty(0))
		TTYNAME(0, sttyname);
	else if (isatty(1))
		TTYNAME(1, sttyname);
	else
		return (NULL);
	if (getutmp(sttyname, &putmp) != 0)
		return (NULL);
	sk = skey;
	p = putmp.ut_line;
	while (*p)
		*sk++ = *p++;
	*sk++ = mch;
	(void)sprintf(stemp, "%ld", putmp.ut_time);
	sreverse(stemp1, stemp);
	p = stemp1;
	while (*p)
		*sk++ = *p++;
	*sk = 0;
	return (skey);
}


void
mkpwunclear(char *spasswd, char mch, char *sencpasswd)
{
	char skey_buf[40];
	register char *skey;

	if (spasswd[0] == 0) {
		sencpasswd[0] = 0;
		return;
	}
	skey = mkenvkey(mch, skey_buf);
	if (skey == NULL) {
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                fprintf(stderr, catgets(catd,LIBCNET,NET32,"Can't make key\n"));
		exit(1);
	}
	_nbsencrypt(spasswd, skey, sencpasswd);
}


void
mkpwclear(char *sencpasswd, char mch, char *spasswd)
{
	char skey_buf[40];
	register char *skey;

	if (sencpasswd[0] == 0) {
		spasswd[0] = 0;
		return;
	}
	skey = mkenvkey(mch, skey_buf);
	if (skey == NULL) {
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                fprintf(stderr, catgets(catd,LIBCNET,NET32,"Can't make key\n"));
		exit(1);
	}
	_nbsdecrypt(sencpasswd, skey, spasswd);
}
