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
/* Copyright (c) 1988 SecureWare, Inc.
 *   All rights reserved.
 *
 * read a security attribute from a file
 * treats lines that start with '#' and blank lines as comments.
 * treats lines that end with '\\' as continuation lines
 * lines after continuation lines are stripped of leading spaces and tabs.
 */

/* #ident "@(#)filetobuf.c	2.1 11:51:28 1/25/89" */

#include <stdio.h>
#include <locale.h>
#include "policy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)

#define INIT_BUFSIZ 128

extern char *malloc();
extern char *realloc();

#if SEC_ACL_POSIX
extern int acl_or_label;
#endif

char *
file_to_buf (filename)
char *filename;
{
	char *buffer;
	FILE *fp;
	char linebuf[128];
	int count;
	int bufsize;
	char continuation = 0;
	register char *cp;
	int len;

	buffer = malloc (INIT_BUFSIZ);
	bufsize = INIT_BUFSIZ;
	count = 1;	/* at least need a '\0' */
	if (buffer == (char *) 0)
		return (buffer);
	fp = fopen (filename, "r");
	if (fp == (FILE *) 0)
		return ((char *) 0);
	while (fgets (linebuf, sizeof (linebuf), fp) != NULL) {
		if (linebuf[0] == '#')
			continue;
		if (linebuf[0] == '\n')
			continue;
		if (continuation)
			cp = &linebuf[strspn (linebuf, " \t")];
		else
			cp = linebuf;
		len = strlen (cp);
		if (cp[len - 2] == '\\') {
			continuation = 1;
#ifdef SEC_ACL_POSIX
			if(acl_or_label) {  /* if 1 then acl */
                        /* we want to keep '\n' */
                        	cp[len - 1] = '\0';
                        	count += len - 1;
			}
			else {
#endif
       	                 	cp[len - 2] = '\0';
				count += len - 2;
#ifdef SEC_ACL_POSIX
				}
#endif
		} else {
			continuation = 0;
#ifdef SEC_ACL_POSIX
                        if(acl_or_label) { /* if 1 then acl */
				/* we want to keep '\n' */	
                        	cp [len] = '\0';
                        	count += len;
			}
			else {
#endif
                        cp [len - 1] = '\0';
                        count += len - 1;
#ifdef SEC_ACL_POSIX
			}
#endif
		}
		if (count > bufsize) {
			buffer = realloc (buffer, count);
			if (buffer == (char *) 0)
				goto out;
			bufsize = count;
		}
		strcat (buffer, linebuf);
	}
out:
	fclose (fp);
	return (buffer);
}
