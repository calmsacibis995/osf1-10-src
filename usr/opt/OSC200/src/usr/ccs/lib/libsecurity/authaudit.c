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
static char *rcsid = "@(#)$RCSfile: authaudit.c,v $ $Revision: 4.2.9.5 $ (DEC) $Date: 1993/11/18 01:13:36 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 */

/* 
 * NOTE: THIS FILE NOW OBSOLETE!
 *
 * 	The interfaces with in this file have been left behind
 * to provide backward compatability.  At some time in the future
 * these interfaces will more than likely go away.
 * 
 */

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/security.h>
#include <sys/secioctl.h>
#include <sys/audit.h>
#include <prot.h>
#include <signal.h>

#if SEC_MAC
caddr_t er_buffer;	/* for Sensitivity level auditing */
#endif

#include <sys/sec_et_types.h>

/*
 *
 *	This routine produces an audit record noting a security failure
 *	in a protected subsystem or TCB.  Even though kernel auditing may be
 *	done at the system call level, this auditing done here notes a
 *	high-level security problem having significance to a subsystem.
 *
 */

void
audit_security_failure(object, action, result, event_type)

int	object;
caddr_t action;
caddr_t result;
int	event_type;

{
	audgenl(SW_COMPATIBILITY,
		T_INT, event_type, 
		T_INT, 	object,
		T_CHARP, action,
		T_CHARP, result,
		T_CHARP, "Security action failed in auth database",
		T_CHARP, command_name, NULL);
}

/*
 *	In an action with the TCB, record both that a resource has expired
 *	and the problem for the TCB that results from the resource depletion.
 *	An audit record is produced for this.
 *
 */

void
audit_no_resource(resource, object, problem, event_type)

caddr_t resource;
int	object;
caddr_t problem;
int	event_type;

{
	audgenl(SW_COMPATIBILITY,
		T_INT, event_type, 
		T_INT, 	object,
		T_CHARP, resource,
		T_CHARP, problem,
		T_CHARP, "Auth database resource denial",
		T_CHARP, command_name, NULL);
}

/*
 *
 *	Produce an audit record when something is wrong with the Authentication
 *	database.  Note the type of database, the entry name, and the problem.
 *
 */

void
audit_auth_entry(desired_entry, type, problem, event_type)

caddr_t desired_entry;
int	type;
caddr_t problem;
int	event_type;

{
	audgenl(SW_COMPATIBILITY,
		T_INT, event_type, 
		T_CHARP, desired_entry,
		T_INT, type,
		T_CHARP, problem,
		T_CHARP, "Auth database integrity",
		T_CHARP, "audit_auth_entry",
		T_CHARP, command_name, NULL);
}

/*
 *
 *	Audit an action peculiar to a subsystem.  Note the subsystem, the
 *	action that had been occurring, and the result of the action.
 *
 */

void
audit_subsystem(action, result, event_type)

caddr_t action;
caddr_t result;
int	event_type;

{
	audgenl(SW_COMPATIBILITY,
		T_INT, event_type, 
		T_CHARP, action,
		T_CHARP, result,
		T_CHARP, "audit_subsystem",
		T_CHARP, command_name, NULL);
}

/*
 * Audit both successful and unsuccessful logins.  This audit record is always
 * done directly to the audit device.
 *
 */

void
audit_login(pr, pwd, terminal_name, problem, code)

register struct pr_passwd	*pr;
register struct passwd		*pwd;
register caddr_t terminal_name;
caddr_t problem;
int				code;

{
	audgenl(LOGIN, 
		T_LOGIN,   pwd->pw_name,
		T_SHELL,   pwd->pw_shell,
		T_UID,     pwd->pw_uid,
		T_DEVNAME, terminal_name,
		T_HOMEDIR, pwd->pw_dir,
		T_CHARP,   problem,
#if SEC_MAC
		T_CHARP,     er_buffer,
#endif
		T_INT,    code, NULL);
}

void
audit_rcmd(pr, pwd, terminal_name, problem, rhost, ruser, rcmd, agent, error)

register struct pr_passwd	*pr;
register struct passwd		*pwd;
register caddr_t 		terminal_name;
caddr_t				problem;
caddr_t				rhost;
caddr_t				ruser;
caddr_t				rcmd;
caddr_t				agent;
caddr_t				error;

{
	audgenl(LOGIN, 
		T_LOGIN,   pwd->pw_name,
		T_SHELL,   pwd->pw_shell,
		T_UID,     pwd->pw_uid,
		T_DEVNAME, terminal_name,
		T_HOMEDIR, pwd->pw_dir,
		T_SERVICE, "rcmd",
		T_CHARP,   problem,
		T_CHARP,   rhost,
		T_CHARP,   ruser,
		T_CHARP,   rcmd,
		T_CHARP,   agent,
#if SEC_MAC
		T_CHARP,     er_buffer,
#endif
		T_CHARP,    error, NULL);
}

/*
 * Audit both successful and unsuccessful password changes.  This audit
 * record is always done directly to the audit device.
 */

void
audit_passwd(name, code, event_type)

caddr_t	name;
int	code;
int	event_type;

{
	audgenl(AUTH_EVENT, 
		T_CHARP,   name,
		T_CHARP, "password change",
		T_INT,    code, NULL);
}


/*
 * Audit both successful and unsuccessful database locking operations.
 * This audit record is always done directly to the audit device.
 */

void
audit_lock(name, code, trys, event_type)

caddr_t	name;
int	code;
int	trys;
int	event_type;

{
	audgenl(SW_COMPATIBILITY,
		T_INT, event_type,
		T_CHARP,   name,
		T_CHARP, "database lock operation",
		T_INT,    trys,
		T_INT,    code, NULL);
}


/*
 * If the user has a special mask, present it here to the audit subsystem.
 */

void
audit_adjust_mask(pr)
register struct pr_passwd *pr;
{
	fprintf(stderr, "audit_adjust_mask: obsolete.\n");
}

/*
 *
 *	Produce an audit record for lock and unlock of user terminals.
 *
 */

void
sa_audit_lock(code, user_term)
int	code;
caddr_t	user_term;
{
	int sw_event = ET_SYS_ADMIN;

	audgenl(SW_COMPATIBILITY,
		T_INT, sw_event,
		T_CHARP,   user_term,
		T_CHARP,   "terminal lock/unlock",
		T_INT,    code, NULL);
}

/*
 * This routine produces an audit record for the audit record type.
 */
void
sa_audit_audit(code, string)
int code;
caddr_t	string;
{
	int sw_event = ET_SYS_ADMIN;

	audgenl(SW_COMPATIBILITY,
		T_INT, sw_event,
		T_CHARP,   string,
		T_CHARP, "sa_audit_audit",
		T_INT,    code, NULL);
}

/* #endif */ /*} SEC_BASE */
