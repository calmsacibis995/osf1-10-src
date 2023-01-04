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
static char     *sccsid = "@(#)$RCSfile: authreadkeys.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:37:33 $";
#endif
/*
 */

/*
 * authreadkeys.c - routines to support the reading of the key file
 */
#include <stdio.h>
#include <sys/types.h>
#include <strings.h>
#include <ctype.h>
#include <syslog.h>

/*
 * Types of ascii representations for keys.  "Standard" means a 64 bit
 * hex number in NBS format, i.e. with the low order bit of each byte
 * a parity bit.  "NTP" means a 64 bit key in NTP format, with the
 * high order bit of each byte a parity bit.  "Ascii" means a 1-to-8
 * character string whose ascii representation is used as the key.
 */
#define	KEY_TYPE_STD	1
#define	KEY_TYPE_NTP	2
#define	KEY_TYPE_ASCII	3

extern int authusekey();
extern void auth_delkeys();

/*
 * nexttok - basic internal tokenizing routine
 */
static char *
nexttok(str)
	char **str;
{
	register char *cp;
	char *starttok;

	cp = *str;

	/*
	 * Space past white space
	 */
	while (*cp == ' ' || *cp == '\t')
		cp++;
	
	/*
	 * Save this and space to end of token
	 */
	starttok = cp;
	while (*cp != '\0' && *cp != '\n' && *cp != ' '
	    && *cp != '\t' && *cp != '#')
		cp++;
	
	/*
	 * If token length is zero return an error, else set end of
	 * token to zero and return start.
	 */
	if (starttok == cp)
		return 0;
	
	if (*cp == ' ' || *cp == '\t')
		*cp++ = '\0';
	else
		*cp = '\0';
	
	*str = cp;
	return starttok;
}


/*
 * authreadkeys - (re)read keys from a file.
 */
int
authreadkeys(file)
	char *file;
{
	FILE *fp;
	char *line;
	char *token;
	u_int keyno;
	int keytype;
	char buf[512];		/* lots of room for line? */

	/*
	 * Open file.  Complain and return if it can't be opened.
	 */
	fp = fopen(file, "r");
	if (fp == NULL) {
		syslog(LOG_ERR, "can't open key file %s: %m", file);
		return 0;
	}

	/*
	 * Remove all existing keys
	 */
	auth_delkeys();

	/*
	 * Now read lines from the file, looking for key entries
	 */
	while ((line = fgets(buf, sizeof buf, fp)) != NULL) {
		token = nexttok(&line);
		if (token == 0)
			continue;
		
		/*
		 * First is key number.  See if it is okay.
		 */
		keyno = (u_int)atoi(token);
		if (keyno == 0) {
			syslog(LOG_ERR,
			    "cannot change keyid 0, key entry `%s'ignored",
			    token);
			continue;
		}

		/*
		 * Next is keytype.  See if that is all right.
		 */
		token = nexttok(&line);
		if (token == 0) {
			syslog(LOG_ERR,
			    "no key type for key number %d, entry ignored",
			    keyno);
			continue;
		}
		if (*token == 'S' || *token == 's')
			keytype = KEY_TYPE_STD;
		else if (*token == 'N' || *token == 'n')
			keytype = KEY_TYPE_NTP;
		else if (*token == 'A' || *token == 'a')
			keytype = KEY_TYPE_ASCII;
		else {
			syslog(LOG_ERR,
			    "invalid key type for key number %d, entry ignored",
			    keyno);
			continue;
		}

		/*
		 * Finally, get key and insert it
		 */
		token = nexttok(&line);
		if (token == 0) {
			syslog(LOG_ERR,
			    "no key for number %d entry, entry ignored",
			    keyno);
		} else if (!authusekey(keyno, keytype, token)) {
			syslog(LOG_ERR,
			    "format/parity error for key %d, not used",
			    keyno);
		}
	}
	(void) fclose(fp);
	return 1;
}
