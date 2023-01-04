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
static char *rcsid = "@(#)$RCSfile: fields.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:22:45 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	fields.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.9.2.2  1992/06/11  14:28:04  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  14:25:59  hosking]
 *
 * Revision 1.9  1991/03/23  17:57:14  devrcs
 * 	<<<replace with log message for ./usr/ccs/lib/libsecurity/fields.c>>>
 * 	[91/03/12  11:30:23  devrcs]
 * 
 * 	Merge fixes up from 1.0.1
 * 	[91/03/11  15:25:06  seiden]
 * 
 * Revision 1.6  90/10/07  20:06:15  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:13:24  gm]
 * 
 * Revision 1.5  90/08/24  13:48:09  devrcs
 * 	Changes for type-widening.
 * 	[90/08/20  14:16:29  seiden]
 * 
 * Revision 1.4  90/07/27  10:31:17  devrcs
 * 	Any function using NLS messages will now open the catalog
 * 	[90/07/16  09:53:33  staffan]
 * 
 * Revision 1.3  90/07/17  12:18:59  devrcs
 * 	Internationalized
 * 	[90/07/05  07:23:39  staffan]
 * 
 * Revision 1.2  90/06/22  21:46:32  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:12:38  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 *
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */

/* #ident "@(#)fields.c	6.2 09:13:11 2/26/91 SecureWare" */
/*
 * Based on:
 *   "@(#)fields.c	2.4.3.1 17:25:39 1/8/90 SecureWare, Inc."
 */

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

#include "libsecurity.h"

#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <macros.h>
#include <string.h>
#include <time.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#define	FIELDSPERLINE	4

/*
 * Determine if the pr_passwd entry contains information that will
 * cause lockout from the account.  If locked_out() returns 1, some
 * condition caused the lock-out.  If it returns 0, there is no
 * password management parameter that would lock out the user.
 *
 * This excluded other account-management parameters which can lock out
 * a user, such as the 'retired' flag, the 'is_template' flag, and the
 * passing of the account expiration date ('expdate').  Those are checked
 * separately in login_sec.
 */
int
locked_out(pr)
	register struct pr_passwd *pr;
{
	register int max_tries;
	register int attempts;
	register time_t now;
	register time_t lifetime;
	register time_t last_change;
	register time_t re_open;
	register int locked;
	register int is_locked_out;

	check_auth_parameters();

	now = time((time_t *) 0);

	/*
	 * Check for password lifetime being over.
	 * Null passwords have infinite lifetime.
	 */
	if (pr->uflg.fg_encrypt && pr->ufld.fd_encrypt[0] != '\0') {
		if (pr->uflg.fg_schange)
			last_change = pr->ufld.fd_schange;
		else if (pr->sflg.fg_schange)
			last_change = pr->sfld.fd_schange;
		else
			last_change = (time_t) 0;

		if (pr->uflg.fg_lifetime)
			lifetime = pr->ufld.fd_lifetime;
		else if (pr->sflg.fg_lifetime)
			lifetime = pr->sfld.fd_lifetime;
		else
			lifetime = (time_t) 0;
	} else
		lifetime = 0;

	/*
	 * Check for unconditional lock on the account.
	 */
	if (pr->uflg.fg_lock)
		locked = pr->ufld.fd_lock;
	else if (pr->sflg.fg_lock)
		locked = pr->sfld.fd_lock;
	else
		locked = 0;

	/*
	 * Check for too many tries on the account.
	 */
	if (pr->uflg.fg_max_tries)
		max_tries = pr->ufld.fd_max_tries;
	else if (pr->sflg.fg_max_tries)
		max_tries = pr->sfld.fd_max_tries;
	else
		max_tries = 0;

	if (pr->uflg.fg_unlockint) {
		if (pr->uflg.fg_ulogin && pr->ufld.fd_unlockint)
			re_open = pr->ufld.fd_unlockint + pr->ufld.fd_ulogin;
		else
			re_open = now+1;
	}
	else if (pr->sflg.fg_unlockint) {
		if (pr->sfld.fd_unlockint && pr->uflg.fg_ulogin)
			re_open = pr->sfld.fd_unlockint + pr->ufld.fd_ulogin;
		else
			re_open = now+1;
	}
	else
		re_open = now+1;

	if (pr->uflg.fg_nlogins)
		attempts = pr->ufld.fd_nlogins;
	else if (pr->sflg.fg_nlogins)
		attempts = pr->sfld.fd_nlogins;
	else
		attempts = 0;

	/*
	 * Any of these conditions is enough to keep the user from
	 * using the account.  The system administrator must re-enable
	 * the account first.
	 */
	is_locked_out =
		(lifetime && last_change && last_change + lifetime < now) ||
		locked ||
		(max_tries && attempts >= max_tries && re_open > now);

	if (is_locked_out)  {
		char buf1[80];
		sprintf (buf1, MSGSTR(FIELDS_1, 
			"account lockout denies account access"));
		audgenl( AUTH_EVENT, T_CHARP, buf1 , T_INT, attempts, NULL);
			
	}

	return is_locked_out;
}


/*
 * The protected password database entry for the user is passed to this
 * routine along with the terminal name. The terminal name is used to
 * look up the entry for the terminal in the device assignment database.
 * If there is no entry the function returns 1. If found, the authorized
 * list is searched for the user name. If the list exists but the name
 * is not found, the function returns 0. If the name is in the list or
 * if there is no list for the terminal, the function returns a 1.
 *
 * Returns: 0 if not authorized, 1 if authorized
 */

auth_for_terminal(prpwd,prtc)
struct pr_passwd *prpwd;
struct pr_term   *prtc;
{
	register struct dev_asg *prdevasg;
	register char *ttynm;
	register char *list;
	register int i;

	/*
	 * Check to see that username is present. Then lookup the entry
	 * for the terminal after getting terminal name.
	 */

	if((prpwd->uflg.fg_name == 0) || (prpwd->ufld.fd_name[0] == '\0') ||
	   (prtc == (struct pr_term *) 0) || (prtc->uflg.fg_devname == 0))
		return(0);

	enddvagent();

	ttynm = prtc->ufld.fd_devname;

	if(((prdevasg = getdvagnam(ttynm)) == (struct dev_asg *) 0) ||
	    (prdevasg->uflg.fg_users == 0)) {
		enddvagent();
		return(1);
	}

	/*
	 * Walk the terminal authorization list looking for the username.
	 */

	if(!prdevasg->ufld.fd_users)
		return(1);

	for(i=0; ; i++) {

		list = prdevasg->ufld.fd_users[i];
		if(list[0] == '\0')
			break;

		if(strcmp(list,prpwd->ufld.fd_name) == 0) {
			enddvagent();
			return(1);
		}
	}

	enddvagent();
	return(0);
}

/*
 * This routine parses a comma-separated field into a mask.
 * vec and maxval describe the mask and size.
 * names is the comma-separated list.
 * pairings is the name to mask offset mapping table.
 * pairingtype is the field within the database (externally visible)
 * database is the database being parsed.
 * entry_name is the entry being parsed.
 * The last three fields are used for auditing purposes.
 */

void
loadnamepair(vec, maxval, names, pairings, pairingtype, database, entry_name)
	register mask_t *vec;
	register int maxval;
	register char *names;
	struct namepair pairings[];
	char *pairingtype;
	int database;
	char *entry_name;
{
	register int scan_val;
	register int found;
	register char *scan_word;
	register char *comma;
	register int vec_size;
	char comma_contents;

	/*
	 * First, clear the vector.  We'll set the found bits later.
	 */
	vec_size = WORD_OF_BIT(maxval) + 1;
	for (scan_val = 0; scan_val < vec_size; scan_val++)
		vec[scan_val] = (mask_t) 0;

	scan_word = names;
	while ((scan_word != (char *) 0) && (*scan_word != '\0'))  {

		/*
		 * Skip any whitespace at the start of this word
		 */
		while (*scan_word == ' ' || *scan_word == '\t')
			++scan_word;
		if (*scan_word == '\0')
			break;

		/*
		 * Find the end and the size of this word.
		 */
		comma = strchr(scan_word, ',');
		if (comma != (char *) 0)  {
			comma_contents = *comma;
			*comma = '\0';
		}

		/*
		 * Find the word in the list.  If it doesn't exist, consider
		 * it an integrity error.
		 */
		found = 0;
		scan_val = 0;
		while (!found && (pairings[scan_val].name != (char *) 0))  {
			if ((strcmp(pairings[scan_val].name, scan_word) == 0) &&
			    (pairings[scan_val].value <= maxval))  {
				found = 1;
				ADDBIT(vec, pairings[scan_val].value);
			}
			scan_val++;
		}


		/*
		 * The scan word is not in the list of words.  This is an
		 * error.
		 */
		if (!found) {
			char buf1[80], buf2[80];

			sprintf(buf1, MSGSTR(FIELDS_4,
			  "Parse field '%s' for entry '%s'."),
			  pairingtype, entry_name);
			sprintf(buf2, MSGSTR(FIELDS_5,
			  "Word '%s' is not a valid value for that field."),
			  scan_word);

			audgenl(AUTH_EVENT, T_CHARP, buf1, T_CHARP, buf2, NULL);
		}

		/*
		 * Advance the word in the comma separated string.
		 */
		if (comma == (char *) 0)
			scan_word = (char *) 0;
		else  {
			*comma = comma_contents;
			scan_word = comma + 1;
		}
	}
}


/*
 * Given values of bits, look through a name pairing and find those
 * strings corresponding to the bits turned on.  Return the string.
 * (It points to a calloc'd area and should be released when no longer
 * used.)
 */
char *
storenamepair(values, maxval, pairings, pairingtype)
	register mask_t *values;
	register int maxval;
	struct namepair pairings[];
	char *pairingtype;
{
	char *names;
	char badval [BUFSIZ];
	register int scanpair;
	register int found;
	register int total_size;
	register int scan;

	total_size = sizeof('\0');
	names = calloc(1, total_size);
	if (names == (char *) 0)  {
		return names;
	}

	/*
	 * Test each bit in the vector.
	 */
	for (scan = 0; scan <= maxval; scan++)  {
		if (ISBITSET(values, scan))  {
			/*
			 * Find the bit in the list.  If it doesn't exist,
			 * consider it an integrity error.
			 */
			found = 0;
			scanpair = 0;
			while (!found &&
			       (pairings[scanpair].name != (char *) 0))  {
				if (scan == pairings[scanpair].value)  {
					found = 1;
					/*
					 * We use calloc() rather than malloc()
					 * because we want the initial space 0'd
					 * as the means to test for a new
					 * string or the continuation of a
					 * string we already started.
					 */

					total_size += sizeof(',') +
					     strlen(pairings[scanpair].name);
					names = realloc(names, total_size);

					if (names == (char *) 0)  {
						return 0;
					}

					if (*names != '\0')
						(void) strcat(names, ",");
					(void) strcat(names,
						pairings[scanpair].name);
				}
				scanpair++;
			}


			if (!found)  {
				(void) sprintf(badval,
						MSGSTR(FIELDS_3, "internal offset %ld of %s"),
						scan, pairingtype);
				/* audgenl(??); */
			}
		}
	}

	return names;
}


/*
 * Return the boolean symbol to be placed into the Authentication database
 * for an entry.
 */
char *
storebool(bool)
	char *bool;
{
	register char *bool_rep;

	if (bool)
		bool_rep = "";
	else
		bool_rep = "@";

	return bool_rep;
}


/*
 * If the FIELDPERLINE threshold is passed, output a newline in the
 * file being written so that a line does not get too long.
 */
int
pr_newline(file, field, error)
	FILE *file;
	register int field;
	int *error;
{
	if (!(*error))  {
		if ((field >= FIELDSPERLINE) || (field <= 0))  {
			*error = (fprintf(file, "\\\n\t:") < 0);
			field = 1;
		}
		else
			field++;
	}

	return field;
}
/* #endif */ /*} SEC_BASE */
