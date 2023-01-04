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
static char	*sccsid = "@(#)$RCSfile: ListSess.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:53 $";
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



/* audit session list routines */

/*
	entry points:
		void    AuditListSessionStart()
		void    AuditListSessionsFree()
		int     AuditListSessionsGet()
		int     AuditListSessionsValidate()
*/

#include <sys/secdefines.h>

#ifdef SEC_BASE

#include "gl_defs.h"
#include "IfAudit.h"

static int
	make_session();

static char 
	**msg_alistsess,
	 *msg_alistsess_text;
	
void 
AuditListSessionsStart() {
	/* Load message text required by this module */
	LoadMessage("msg_isso_audit_alistsess", &msg_alistsess, &msg_alistsess_text);
}

void 
AuditListSessionsStop() {
	/* Nothing needed here */
}

void 
AuditListSessionsFree(lsfill)
	struct ls_fillin *lsfill;
{
	if (lsfill->sessions) {
		free_cw_table(lsfill->sessions);
		lsfill->sessions = NULL;
	}
}

int 
AuditListSessionsGet(lsfill)
	struct ls_fillin *lsfill;
{
	DIR         *fp;
	struct dirent *d;
	int         count,
		    i;
	char        *cp,
		fname[sizeof (AUDIT_LOGDIR) + sizeof (AUDIT_LOG_FILENAME) - 1];

	/* Open log directory */
	fp = opendir(AUDIT_LOGDIR);
	if (! fp) {
		ErrorMessageOpen(2110, msg_alistsess, 0, NULL);

		/* AUDIT failure to open AUDIT_LOGDIR for reading */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY,
			"Failure to open for reading to list sessions", 
			ET_SYS_ADMIN);
		return 1;
	}
	
	count = 0;
	while (d = readdir(fp)) {
	cp = d->d_name;
#ifdef SCO
	/* XXXXX The readdir function of SCO is broken */
	cp -= 2;
#endif
		if (d->d_fileno &&  
			! strncmp(cp, AUDIT_LOG_FILENAME, 
			  sizeof(AUDIT_LOG_FILENAME) - AUDIT_DIGITS - 1) 
	   ) {
			sprintf(fname, "%s%s", AUDIT_LOGDIR, cp);
			if (! eaccess(fname, 4))
				count++;
		}
	}
	
	/* Be careful with zero sessions count */
	if (count) {
		lsfill->sessions = (char **) 
			alloc_cw_table(count, LSFILLWIDTH + 1);
		if (! lsfill->sessions) {
			closedir(fp);
			MemoryError();
			/* Program exits in call to MemoryError() */
		}
		/* Read in session file names into malloc'ed space */
#if defined(OSF)
	/* rewinddir is not fully supported under OSF yet */
	closedir (fp);
	fp = opendir(AUDIT_LOGDIR);
	if (! fp) {
		ErrorMessageOpen(2110, msg_alistsess, 0, NULL);

		/* AUDIT failure to open AUDIT_LOGDIR for reading */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY,
			"Failure to open for reading to list sessions", 
			ET_SYS_ADMIN);
		return 1;
	}
#else
		rewinddir(fp);
#endif
		count = 0;
		while(d = readdir(fp)) {
		cp = d->d_name;
#ifdef SCO
		/* The readdir function of SCO is broken */
		cp -= 2;
#endif
			if (d->d_fileno 
			&&   ! strncmp(cp, AUDIT_LOG_FILENAME,
			        sizeof(AUDIT_LOG_FILENAME) - AUDIT_DIGITS - 1)
			&&   ! make_session(lsfill->sessions[count], cp)
		   ) 
				count++;
		}
		/* Alpha sort entries */
		sort_cw_table(lsfill->sessions, LSFILLWIDTH + 1, count);
	}
	else
		lsfill->sessions = NULL;
	
	lsfill->nsessions = count;
	closedir(fp);
	return 0;
}

int 
AuditListSessionsValidate() 
{
	/* check read, search perm */
	if (eaccess (AUDIT_LOGDIR, 5) < 0) {
	
		ErrorMessageOpen(2120, msg_alistsess, 3, NULL);

		/* AUDIT failure to access AUDIT_LOGDIR */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY,
			"Failure to open for reading to list sessions", 
			ET_SYS_ADMIN);
		return 1;
	}
	return 0;
}

static int 
make_session(buffer, filename)
	char    *buffer;
	char    *filename;
{
	struct log_header lbuf;
	char    fname[ sizeof(AUDIT_LOGDIR) + sizeof(AUDIT_LOG_FILENAME) - 1 ];
	char    date[DATESTRINGSIZE + 1];
	FILE    *fp;
	int     fd;
	struct audit_init  auinit;
	struct stat sb;
	extern FILE *old_audit_parms_file();

	sprintf(fname, "%s%s", AUDIT_LOGDIR, filename);
	fp = fopen(fname, "r");
	if (! fp)
		return 1;
	fread(&lbuf, sizeof (lbuf), 1, fp);
	fclose(fp);
	if (strncmp(lbuf.id, AUDIT_LOGID, sizeof (lbuf.id)))
		return 1;

	sprintf(buffer, "%5d ", lbuf.session);
	date[DATESTRINGSIZE] = '\0';
	strncpy(date, ctime (&lbuf.start), DATESTRINGSIZE);
	strcat(buffer, date);
	strcat(buffer, " ");
	/* check if stop time is zero.  If so, session crashed or is
	 * current session.
	 */
	if (lbuf.stop == (time_t) 0) {
		fp = old_audit_parms_file();
		if (fp) {
			fread(&auinit, sizeof (auinit), 1, fp);
			fclose(fp);
			if (lbuf.session = auinit.session) {
				fd = open (AUDIT_WDEVICE, O_WRONLY);
				if (fd >= 0) {
					close(fd);
					lbuf.stop = time ((long *) 0);
				}
			}
		}
	}
	
	/* if still zero, use the time the log file last written */
	if (lbuf.stop == (time_t) 0) {
		stat(fname, &sb);
		lbuf.stop = sb.st_mtime;
	}
	strncpy(date, ctime (&lbuf.stop), DATESTRINGSIZE);
	strcat(buffer, date);

	sprintf(date, " %5ld %7ld", lbuf.records, lbuf.total);
	strcat(buffer, date); 
	return 0;
}
#endif /* SEC_BASE */
