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
static char *rcsid = "@(#)$RCSfile: util.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/25 22:03:39 $";
#endif

#include <stdlib.h>
#include <wchar.h>
#include "xpg4demo.h"
#include "xpg4demo_msg.h"

int (*CompareNames)(const void *e1, const void *e2);

/*
 * Compare two employee names using the surnames as the primary key.
 */
int
CompSurnames(const void *e1, const void *e2)
{
	Employee *emp1 = (Employee *) e1,
		 *emp2 = (Employee *) e2;
	int order;		            	/* Relative order of records */

	/*
	 * Return the order of the surnames if they are different.
	 */
	order = wcscoll(emp1->surname, emp2->surname);
	if (order != 0)
		return (order);

	/*
	 * The surnames are the same, so return the order of the given
	 * names if they are different.
	 */
	order = wcscoll(emp1->first_name, emp2->first_name);
	if (order != 0)
		return (order);
	/*
	 * If the names are all the same, return the difference of the badge
	 * numbers.
	 */
	return (emp1->badge_num - emp2->badge_num);
}

/*
 * Compare two employee names using the given names as the primary
 * key.
 */
int
CompFirstnames(const void *e1, const void *e2)
{
	Employee *emp1 = (Employee *) e1,
		 *emp2 = (Employee *) e2;
	int order;           	/* Relative order of records */

	/*
	 * Return the order of the given names if they are different.
	 */
	order = wcscoll(emp1->first_name, emp2->first_name);
	if (order != 0)
		return (order);

	/*
	 * The given names are the same, so return the order of the
	 * surnames if they are different.
	 */
	order = wcscoll(emp1->surname, emp2->surname);
	if (order)
		return (order);
	/*
	 * If the names are all the same, return the difference of the badge
	 * numbers.
	 */
	return (emp1->badge_num - emp2->badge_num);
}

int
MbsWidth(const char *mbs)
{
	wchar_t wcs[100];
	size_t n;

	if (mbs == (char *) NULL)
		return (0);
	n = mbstowcs(wcs, mbs, sizeof(wcs)/sizeof(wchar_t));
	return (wcswidth(wcs, n));
}

char *
GetMessage(int set_id, int msg_id, const char *s)
{
	char *msg;
	char *smsg;

	msg = catgets(MsgCat, set_id, msg_id, s);
	if ((smsg = malloc(strlen(msg) + sizeof(char))) == (char *) NULL) {
		NoMemory(GetErrorMsg(E_UTL_NOMEMORY,
				     "message string"));
		/*NOTREACHED*/
	}
	(void) strcpy(smsg, msg);
	return (smsg);
}
