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
static char	*sccsid = "@(#)$RCSfile: Reduce.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:24 $";
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
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved.
 */



/* Support routines for running the audit reduction program */

#include <sys/secdefines.h>

#if SEC_BASE

#include "gl_defs.h"
#include "IfAudit.h"

#define REDUCE_COMMAND      "/tcb/bin/reduce"
#define REDUCE_REPORT_FILE  "AuditReport"

static char 
		**msg,
		*msg_text;

/*
 * Run the reduction program against the specified session using the
 * selection file and producing the output file.
 */

int
ReduceProgram(session, report_pfile, report_file_name)
	int session;
	char *report_pfile;	/* selection file */
	char *report_file_name;	/* output file */
{
	char fname[sizeof(REDUCE_REPORT_PATH) + NAME_MAX];
	privvec_t s;
	int ret;
	char report_session[10];
	int pid;
	int wait_stat;
	struct stat sb;
	char *argv[10];

	if (! msg)
		LoadMessage( "msg_isso_audit_reduce", &msg, &msg_text);

	sprintf(report_session, "%d", session);

	/**********************************************************************/
	/* Build report                                                       */
	/**********************************************************************/
	
	forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_CHOWN,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);

	/* Test reports file directory exists; create if no */
	if (stat(REDUCE_REPORT_PATH, &sb) < 0) {
		ret = create_file_securely(REDUCE_REPORT_PATH, AUTH_VERBOSE);
		if (ret != CFS_GOOD_RETURN) {
	/* "Cannot create audit reports directory." */
	/* "Please check permission on directory and on this program." */
			ErrorMessageOpen(1545, msg, 27, NULL);
			seteffprivs(s, NULL);
			return 1;
		}
	}
	
	/* Build name of reduction report file */
	strcpy(fname, REDUCE_REPORT_PATH);
	strcat(fname, report_file_name);
	ret = create_file_securely(fname, AUTH_SILENT);
	if (ret != CFS_GOOD_RETURN) {
		seteffprivs(s, NULL);
		ErrorMessageOpen(1560, msg, 14, "Cannot create report file");
		return 1;
	}
	
	/* Run reduction program */
	if (eaccess(REDUCE_COMMAND, 1) < 0) {
		ErrorMessageOpen(1550, msg, 10, NULL);
		/* AUDIT failure to access reduce command */
		audit_no_resource(REDUCE_COMMAND, OT_REGULAR,
			"Reduce command is not executable", ET_SYS_ADMIN);
		seteffprivs(s, NULL);
		return;
	}
	
	forcepriv(SEC_LIMIT);
	pid = fork();
	disablepriv(SEC_LIMIT);
	switch(pid) {
		case    -1:
			ErrorMessageOpen(1555, msg, 12, NULL);
			/* AUDIT fork failure */
			audit_no_resource("Process", OT_PROCESS,
				"Cannot fork to run reduce command",
				ET_SYS_ADMIN);
			ret = 1;
			
		case    0:
			/* Point stdout and stderr at report file */
			if (!freopen(fname, "w", stdout))
				exit(0x7f);
			fclose(stderr);
			dup(1);
			argv[0] = strrchr(REDUCE_COMMAND, '/') + 1;
			argv[1] = "-s";
			argv[2] = report_session;
			argv[3] = "-p";
			argv[4] = report_pfile;
			argv[5] = NULL;
			execv(REDUCE_COMMAND, argv);
			/* Should never get here */
			exit(0x7f);
			
		default:
			while (wait(&wait_stat) != pid)
				;
			WorkingClose();

			if (wait_stat & 0xff00) { 
				/* exit status non-zero */
				char ebuf[80];

				sprintf(ebuf, "%d", (wait_stat >> 8) & 0xff);
				ErrorMessageOpen(1560, msg, 14, ebuf); 
				sa_audit_audit (ES_AUD_ERROR,
				  "Reduce command terminated with error exit");
				ret = 1;
			} else if (wait_stat & 0xff) { 
				/* signal received */
				ErrorMessageOpen(1570, msg, 16, NULL);
				sa_audit_audit(ES_AUD_ERROR, msg[16]);
				ret = 1;
			} else
				ret = 0;
			break;
	}                  
	seteffprivs(s, NULL);
	return ret;
}
#endif /* SEC_BASE */
