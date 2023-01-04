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
static char	*sccsid = "@(#)$RCSfile: lpd_sec.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/19 18:10:05 $";
#endif 
/*
 */
#if SEC_BASE
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1990 SecureWare, Inc.  All Rights Reserved.
 */



#include "lp.h"

#if SEC_MAC
#include <sys/security.h>
#include <mandatory.h>
#define MSGSTR_SEC(n,s) catgets(catd,MS_PRINTER_SEC,n,s)

extern char	*sys_secerrlist[];
extern int	sec_errno;
#endif

uid_t	lp_uid;
gid_t	lp_gid;

/*
 * lpd_initialize
 *
 * Called upon startup by lpd.  Verify that the process's luid is not
 * already set, then set it as well as the uid and gid to "lp".
 */

lpd_initialize()
{
	errno = 0;
	(void) getluid();
	if (errno == 0) {
		fprintf(stderr, MSGSTR_SEC(LPDSEC1, "lpd: must be run from init\n"));
		exit(1);
	}

	lp_uid = pw_nametoid("lp");
	if (lp_gid == (gid_t) -1) {
		fprintf(stderr, MSGSTR_SEC(GROUPUNDEF, "%s: group id \"lp\" is not defined\n"), "lpd");
		exit(1);
	}

	lp_gid = gr_nametoid("lp");
	if (lp_uid == (uid_t) -1) {
		fprintf(stderr, MSGSTR_SEC(USERUNDEF, "%s: user id \"lp\" is not defined\n"), "lpd");
		exit(1);
	}

	setluid(lp_uid);
	setgid(lp_gid);
	setuid(lp_uid);
}

/*
 * Set the discretionary attributes of the UNIX domain socket through which
 * the daemon receives requests.  This hook should be called after the
 * bind() system call and before the listen() system call.
 */
lpd_setsockattr(path)
	char	*path;
{
	if (chown(path, lp_uid, lp_gid) == -1) {
		syslog(LOG_ERR, MSGSTR_SEC(LPDSEC4, "%s: cannot change owner/group: %m"), path);
		exit(1);
	}
	if (chmod(path, 0660) == -1) {
		syslog(LOG_ERR, MSGSTR_SEC(LPDSEC5, "%s: cannot change mode: %m"), path);
		exit(1);
	}
#if SEC_MAC
	if (chslabel(path, (mand_ir_t *) 0) == -1) {
		syslog(LOG_ERR, MSGSTR_SEC(LPDSEC6, "%s: cannot set MAC label: %s"), path,
			sys_secerrlist[sec_errno]);
		exit(1);
	}
#endif
}

/*
 * lpd_checkclient
 *
 * Verify that the process at the other end of a UNIX domain socket is
 * privileged.  The socket descriptor is passed as an argument.
 */
lpd_checkclient(fd)
{
	int	priv;

#if SEC_ARCH
	if (ioctl(fd, SIOCGPEERPRIV, &priv) == -1 || priv == 0) {
		syslog(LOG_ERR, MSGSTR_SEC(LPDSEC7, "lpd: ignoring untrusted UNIX domain client"));
		exit(1);
	}
#endif
}

#endif /* SEC_BASE */
