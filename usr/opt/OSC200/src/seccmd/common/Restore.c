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
static char	*sccsid = "@(#)$RCSfile: Restore.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:38 $";
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
 *   All rights reserved
 */



/* Support routines for restoring audit sessions from a device */

#include <sys/secdefines.h>

#ifdef SEC_BASE

/*
	entry points:
		RestoreStart()
		RestoreOpen()
		RestoreClose()
		RestoreStop()
*/

#include "gl_defs.h"
#include "IfAudit.h"

/* Default backup device should be configurable in future release !!!! */
#ifdef AUX
#define DEFAULT_DEVICE "/dev/rfloppy0"
#else
#define DEFAULT_DEVICE "/dev/rfloppy0"
#endif
#if SEC_ARCH
#define RESTORE_PROGRAM    "/bin/mltape"
#define RESTORE_ARGUMENTS  "-imvdBI"
#else
#define RESTORE_PROGRAM	   "/bin/cpio"
#define RESTORE_ARGUMENTS  "-imvdB"
#endif

static char   
	**msg,
	*msg_text;

int
	RestoreFiles(),
	CheckDeviceRead();
		
void 
RestoreStart()                        
{
	/* Load in message text */
	if (! msg)
		LoadMessage("msg_isso_audit_restore", &msg, &msg_text);
}    
	
void 
RestoreEnd() 
{
	/* nothing to do here */
}

int 
RestoreFiles(devicename)
	char *devicename;
{
	char    *argv[20],
		*cp,
		*executable_name;
	int     n,
		pid,
		ret = 0,
		wait_stat;
	struct stat sb;
	privvec_t s;
	void (*old_sighup)(), (*old_sigint)(), (*old_sigquit)();

	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);

	/* Check read, write, search perm to audit log dir */

	if (stat(AUDIT_LOGDIR, &sb) < 0 || !S_ISDIR(sb.st_mode)) {
		ErrorMessageOpen(1610, msg, 16, NULL);
		/* AUDIT permission failure in AUDIT_LOGDIR */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY,
				"Log directory does not exist", ET_SYS_ADMIN);
		seteffprivs(s, NULL);
		return 1;
	}

	/* Prepare arguments for the restore program */

	cp = strrchr(RESTORE_PROGRAM, '/');
	if (cp)
		cp++;
	else
		cp = RESTORE_PROGRAM;
	executable_name = cp;
	
	n = 0;
	argv[n++] = executable_name;
	argv[n++] = RESTORE_ARGUMENTS;
#if SEC_ARCH
	argv[n++] = devicename;
#endif
	argv[n++] = NULL;

	/* Fork and run the restore program */
	old_sighup = signal(SIGHUP, SIG_IGN);
	old_sigint = signal(SIGINT, SIG_IGN);
	old_sigquit = signal(SIGQUIT, SIG_IGN);
	switch (pid = fork())  {
		case    -1: /* error - can't fork sub-process */
			signal(SIGHUP, old_sighup);
			signal(SIGINT, old_sigint);
			signal(SIGQUIT, old_sigquit);
			ret = -1;
			break;
			
		case    0:  /* child */

			/* Reset signals */
			signal(SIGHUP, SIG_DFL);
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);

			/* Run restoration program */
			exit(DisplayCommand(RESTORE_PROGRAM, argv));

			/* Should never get here */
			exit(0x7f);
		
		default:
			while (wait(&wait_stat) == -1)
				;

			/* Put signals back */
			signal(SIGHUP, old_sighup);
			signal(SIGINT, old_sigint);
			signal(SIGQUIT, old_sigquit);
			if (! (wait_stat & 0xFF)) {
				/* terminated due to exit() */
				ret = (wait_stat >> 8) & 0xFF; /* exit status */
				if (ret == 0x7f)
					ret = -1;
			}
			else 
				/* terminated by signal */
				ret = -1;
			break;
	}

	/* 'ret' has result code here from exec */
	if (ret) {
		/* Terminal shell program failed */
		ErrorMessageOpen(1630, msg, 21, NULL);
		audit_no_resource(RESTORE_PROGRAM, OT_PROCESS,
					"Shell program failed", ET_SYS_ADMIN);
		ret = 1;
	}

	if (ret == 0) {
		/* AUDIT run of restore program */
		sa_audit_audit(ES_AUD_ARCHIVE, "Audit file restore complete");
	}

	return ret;
}

static int 
CheckDeviceRead(device)
	char    *device;
{
	struct stat sb;
	int     mode,
			ret;

	ret = stat(device, &sb);
	if (ret < 0)  {
		ErrorMessageOpen(1690, msg, 23, NULL);
		return 1;
	}
	if ((sb.st_mode & S_IFMT) != S_IFCHR) {
		ErrorMessageOpen(1691, msg, 26, NULL);
		return 1;
	}
	if (eaccess(device, 4) < 0) {
		ErrorMessageOpen(1692, msg, 29, NULL);
		return 1;
	}
	return 0;
}
#endif /* SEC_BASE */
