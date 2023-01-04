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
static char	*sccsid = "@(#)$RCSfile: Reports.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:31 $";
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
 * Copyright (c) 1989-90 SecureWare, Inc.
 *   All rights reserved
 */



/* collect report names routines */

#include <sys/secdefines.h>

#if SEC_BASE

#include "gl_defs.h"
#include "IfAudit.h"

/* Message stuff */

static char **msg, *msg_text;

/*
 * Collect file names in the reports directory
 * Allocates table, and assigns pointer and length.
 * Returns 0 on success and 1 on failure.
 */

int 
ReportNamesGet(ntablep, tablep)
	int *ntablep;
	char ***tablep;
{
	char        buf[512],
		    *cp,
		    dirname[512];
	int         i;
	DIR         *fp;
	struct dirent *d;
	struct stat sb;
	privvec_t s;
	struct a {
		char *filename;
		struct a *next;
	} *ap, *first = NULL, *last;
	char **report_names;
	int report_count;

        if (! msg)
                LoadMessage("msg_isso_audit_showreports", &msg, &msg_text);

	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	
	/* Test reports file directory exists; create if not */
	if (stat(REDUCE_REPORT_PATH, &sb) < 0) {
		int ret;

		forcepriv(SEC_CHOWN);
		ret = create_file_securely(REDUCE_REPORT_PATH, AUTH_VERBOSE);
		if (ret != CFS_GOOD_RETURN) {
			/* "Cannot create audit reports directory." */
			/* "Please check permission on directory and on this program." */
			SystemErrorMessageOpen(1915, msg, 26, NULL);
			seteffprivs(s, NULL);
			return(1);
		}
	}    
	
	/* Open report files directory */
	fp = opendir(REDUCE_REPORT_PATH);

	seteffprivs(s, NULL);

	if (fp == (DIR *) 0) {
		/* "Cannot open audit reports directory for read." */
		/* "Please check permission on directory and on this program." */
		SystemErrorMessageOpen(1930, msg, 5, NULL);
		return(1);
	}
	
	/* Scan report files directory, count report files as go */
	report_count = 0;
	while (d = readdir(fp)) {
		cp = d->d_name;
#ifdef SCO
		/* There is a bug in readdir function on SCO platform */
		cp -= 2;
#endif

		/* ignore '.', '..', and deleted files */

		if (strcmp(cp, ".") == 0 ||
		    strcmp (cp, "..") == 0 ||
		    d->d_fileno == 0)
			continue;

		/* save filename into newly allocated list entry */

		report_count++;

		ap = (struct a *) Malloc(sizeof(struct a ));
		if (ap == (struct a *) 0)
			MemoryError();
		ap->filename = Malloc(strlen(cp) + 1);
		if (ap->filename == NULL)
			MemoryError();
		strcpy(ap->filename, cp);
		
		/* append to end of list */

		if (first == (struct a *) 0)
			first = ap;
		else
			last->next = ap;
		last = ap;
	}
	closedir(fp);

	/* terminate the list */

	if (first != (struct a *) 0)
		last->next = (struct a *) 0;
	
	/* Malloc array of pointers to report file names */
	if (report_count) {
		report_names = (char **) Malloc(sizeof(char *) * report_count);
		if (report_names == (char **) 0) {
			MemoryError();
			/* Dies */
		}
	}
	else {
		report_names = NULL;
		ErrorMessageOpen(1940, msg, 24, NULL);
		/* "No reports to display."*/
		return(1);
	}   

	/* assign report names to table */

	for (i = 0, ap = first; i < report_count; i++) {
		report_names[i] = first->filename;
		first = first->next;
		free(ap);
		ap = first;
	}

	/* If more than one, then alpha sort */
	if (report_count > 1)
		qsort(report_names, report_count, sizeof(char *), strcmp);
		
	*tablep = report_names;
	*ntablep = report_count;

	return 0;
}

/*
 * Free the report names.
 * note that this is not a constant width table, and therefore the
 * strings must be freed individually.
 */

void
ReportNamesFree(table, ntable)
	char **table;
	int ntable;
{
	int i;

	/* sanity check */

	if (table == (char **) 0)
		return;

	for (i = 0; i < ntable; i++)
		if (table[i] != NULL)
			free(table[i]);

	free((char *) table);
}

/*
 * delete a report file
 */

int
ReportFileDelete(filename)
	char *filename;
{
	privvec_t s;
	int ret;
	char buf[sizeof(REDUCE_REPORT_PATH) + PATH_MAX];

	sprintf(buf, "%s%s", REDUCE_REPORT_PATH, filename);
	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	ret = unlink(buf);
	seteffprivs(s, NULL);
	return ret;
}

#endif /* SEC_BASE */
