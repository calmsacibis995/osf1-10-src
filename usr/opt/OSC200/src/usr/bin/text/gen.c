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
static	char	*sccsid = "@(#)$RCSfile: gen.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/09/21 17:11:44 $";
#endif lint
/*
 */
/*
 *
 *   File name: gen.c
 *
 *   Source file description:
 *	A few useful functions to help with string extraction.
 *
 *   Functions:
 *	fixsuffix()
 *	fixprefix()
 *	loadignore()
 *	addignore()
 *	sopen()
 *
 *   Modification history:
 *	Tom Woodburn, 02-May-1991.
 *		Changed copyright notice to DIGITAL_COPYRIGHT macro.
 *
 *	Andy Gadsby,  5-Jan-1987.
 *		Created.
 *
 */

#include <stdio.h>
#include "defs.h"

/* 
 * fixsuffix()
 *	Given a file name return a pointer to a new name with the suffix
 *	replaced by suffix. Returns a pointer to a static structure.
 */

char *
fixsuffix(name, suffix)
char *name, *suffix;
{	char *slash;			/* position of last slash	*/
	char *dot;			/* position of last dot		*/
	static char rname[255];		/* return value			*/
	char *rindex();

	strcpy(rname, name);
	if ((slash = rindex(rname, '/')) == NULL)
		slash = rname;
	if ((dot = rindex(slash, '.')) == NULL)
		strcat(rname, suffix);
	else
		strcpy(dot, suffix);

	return rname;
}


/* 
 * fixprefix()
 *	Given a file name return a pointer to a new name with the prefix
 *	set to prefix. Returns a pointer to a static structure.
 */

char *
fixprefix(prefix, name)
char *name, *prefix;
{	char *slash;			/* position of last slash	*/
	static char rname[255];		/* return value			*/
	char *rindex();

	if (slash = rindex(rname, '/')) {
		strncpy(rname, name, slash - name + 1);
		strcat(rname, prefix);
		strcat(rname, slash + 1);
	} else {
		strcpy(rname, prefix);
		strcat(rname, name);
	}
	return rname;
}

/*
 * loadignore()
 *	Read from the file given the ignore list. This is a list of specific
 *	strings which we wish to ignore wherever encountered.
 */

loadignore(fname)
char *fname;
{	FILE *fp;			/* the file pointer		*/
	FILE *sopen();
	int   len;			/* length of the string read	*/
	char  string[LINESIZE];		/* the string			*/
	struct element elem;		/* used to do the saving	*/

	fp = fname ? fopen(fname, "r") : sopen(IGNFILE, "r");
	if (fp == (FILE *)NULL)
		return ERROR;

	while (fgets(string, LINESIZE, fp) != (char *)NULL) {
		len = strlen(string);	/* zap any newline read		*/
		if (string[len - 1] == '\n')
			len--;
		if (len == 0)
			continue;
		elem.len = len;
		elem.flags = STR_IGNORE;
		elem.linenum = 0;	/* not true but a good hint	*/
		savestr(string, &elem);
	}
	fclose(fp);
	return OK;
}

/*
 * addignore()
 *	Append text to the ignore file so that on subsequent invocations
 *	the string will be ignored.
 */

addignore(fname, string, len)
char *fname;				/* the ignore file name		*/
char *string;				/* the string to ignore		*/
int   len;				/* the length of the string	*/
{ 	FILE *fp;

	if ((fp = fopen(fname ? fname : IGNFILE, "a")) == (FILE *)NULL)
		return ERROR;
	
	while (len-- > 0)
		fputc(*string++, fp);
	fputc('\n', fp);
	fclose(fp);
	return OK;
}

/*
 * sopen()
 *	Open the file name given trying the current directory, $HOME
 *	and the system directory
 */

FILE *
sopen(name, mode)
char *name;
char *mode;
{	FILE *fp;
	char buf[255];
	char *getenv();

	if (fp = fopen(name, mode))
		return fp;

	strcpy(buf, getenv("HOME"));
	strcat(buf, "/");
	strcat(buf, name);
	if (fp = fopen(buf, mode))
		return fp;

	strcpy(buf, LIB_DIR);
	strcat(buf, name);
	return (fp = fopen(buf, mode));
}
