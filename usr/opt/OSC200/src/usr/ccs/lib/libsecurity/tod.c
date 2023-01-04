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
static char	*sccsid = "@(#)$RCSfile: tod.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:26:37 $";
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
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <prot.h>

#define	SPEC_SEP	','

#define	GEN_WILD_CARD	MSGSTR(TOD_6, "Any")
#define	GEN_UNWILD_CARD	MSGSTR(TOD_7, "Never")
#define	WEEK_WILD_CARD	MSGSTR(TOD_8, "Wk")

static char *days[] =  {
	"Su",
	"Mo",
	"Tu",
	"We",
	"Th",
	"Fr",
	"Sa",
};

static int in_time_period();
static int day_match();


extern char *strchr();


#ifdef	TOD_DEBUG
main(argc, argv)
	int argc;
	char *argv[];
{
	char *salt;
	time_t curr_time;

	set_auth_parameters(argc, argv);

	if (argc != 2)  {
		printf(MSGSTR(TOD_1, "use: tod tod_spec\n"));
		exit(1);
	}

	curr_time = time((long *) 0);
	(void) printf(MSGSTR(TOD_2, "current date/time = %s"), ctime(&curr_time));

	if (in_time_period(curr_time, argv[1]))
		(void) printf(MSGSTR(TOD_3, "current time is within range\n"));
	else
		(void) printf(MSGSTR(TOD_4, "current time is not within range\n"));
}
#endif



/*
 * Returns 0 if we can enter the account now and 1 if we cannot.  This
 * is based on the u_tod string in the Protected Password entry.  If there
 * is no u_tod entry in either the user-specific or system parts, let
 * the user enter the account.  Audit time locks.
 */
int
time_lock(prpwd)
	struct pr_passwd *prpwd;
{
	char *tod_spec;
	static char _attempts[255];
	int attempts;
	int is_time_locked;
	time_t current_time;

	check_auth_parameters();

	if (prpwd->uflg.fg_tod)
		tod_spec = prpwd->ufld.fd_tod;
	else if (prpwd->sflg.fg_tod)
		tod_spec = prpwd->sfld.fd_tod;
	else
		tod_spec = (char *) 0;

	current_time = (time_t) time((time_t *) 0);

	is_time_locked = ((tod_spec != (char *) 0) &&
			  !in_time_period(current_time, tod_spec));

	if (is_time_locked)  {
		if (prpwd->uflg.fg_nlogins)
			attempts = prpwd->ufld.fd_nlogins;
		else if (prpwd->sflg.fg_nlogins)
			attempts = prpwd->sfld.fd_nlogins;
		else
			attempts = 0;

		sprintf(_attempts, "%d attepmts", attempts);
		if(audgenl(AUTH_EVENT,
			T_CHARP, prpwd->ufld.fd_name,
			T_CHARP, MSGSTR(TOD_5,"time lock denies account access"),
			T_CHARP, _attempts,NULL) == -1)
			perror("audgenl");
	}

	return is_time_locked;
}


/*
 * Returns 1 if req_time is within the time period specified by tod_spec.
 * Returns 0 otherwise.  The tod_spec is in the same syntax as the similar
 * UUCP string.
 *
 * The syntax is a number of comma-separated date/time specifications.
 * Each specification is a concatenation of
 *
 *	<day>...<day><time_range>
 *
 * where day is `Mo', `Tu', `We', `Th', `Fr', `Sa', `Su', or the
 * wild-card `Any' name, or the Mon-Fri `Wk' wild-card.
 * The time_range is #-#, where # is a military time, like 0900 (9am)
 * or 1700 (5pm).  The range can span a day boundary, where if the second
 * number is less than the first, the second number refers to the beginning
 * of the same day, not the next day.  When the time_range part is missing,
 * or the endpoints are the same, all times within the day(s) are valid.
 */
static int
in_time_period(req_time, tod_spec)
	time_t req_time;
	char *tod_spec;
{
	register int found;
	register int mil_time;
	register struct tm *ydhms;
	register int entries;
	char *p_spec;
	long start_time;
	long end_time;

	ydhms = localtime(&req_time);
	mil_time = (ydhms->tm_hour * 100) + ydhms->tm_min;

	p_spec = tod_spec;
	found = 0;
	while (!found && (p_spec != (char *) 0))  {
		if (day_match(ydhms->tm_wday, &p_spec))  {
			entries = sscanf(p_spec, "%ld-%ld\n",
					 &start_time, &end_time);
			found =  ((entries < 2) ||
				  (start_time == end_time) ||

				  ((start_time < end_time) &&
				   (start_time <= mil_time) &&
				   (mil_time <= end_time)) ||

				  ((start_time > end_time) &&
				   ((start_time <= mil_time) ||
				    (mil_time <= end_time))));
		}

		if (!found)  {
			/*
			 * Find next specification.
			 */
			p_spec = strchr(p_spec, SPEC_SEP);
			if (p_spec != (char *) 0)
				p_spec++;
		}
	}

	return found;
}


/*
 * Try to match the day specification to the known ones.  If found, return 1
 * and advance the specification string over the match.  If not found, return
 * 0 with no side-affects.
 */
static int
day_match(day_of_week, spec)
	register int day_of_week;
	register char **spec;
{
	int found = 0;

	while (isascii(**spec) && isalpha(**spec))  {
		if (strncmp(*spec, GEN_WILD_CARD, strlen(GEN_WILD_CARD)) == 0) {
			found = 1;
			*spec += strlen(GEN_WILD_CARD);
		}
		else if (strncmp(*spec, GEN_UNWILD_CARD,
				 strlen(GEN_UNWILD_CARD)) == 0) {
			/*
			 * If found was already set, this is like a NOP.
			 * Such a condition can occur with a AnyNever string.
			 * If not set, ignore all days in rest of this spec.
			 */
			*spec += strlen(GEN_UNWILD_CARD);
			while (isascii(**spec) && isalpha(**spec))
				(*spec)++;
		}
		else if ((day_of_week >= 1) && (day_of_week < 5) &&
			 (strncmp(*spec, WEEK_WILD_CARD,
				  strlen(WEEK_WILD_CARD)) == 0)){
			found = 1;
			*spec += strlen(WEEK_WILD_CARD);
		}
		else if (strncmp(*spec, days[day_of_week],
				 strlen(days[day_of_week])) == 0)  {
				found = 1;
				*spec += strlen(days[day_of_week]);
			}
		else if (isascii(**spec) && isalpha(**spec))
			(*spec)++;
	}

	return found;
}
/* #endif */ /*} SEC_BASE */
