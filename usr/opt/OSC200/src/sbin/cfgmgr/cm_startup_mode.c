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
static char	*sccsid = "@(#)$RCSfile: cm_startup_mode.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:02:06 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "cfgmgr.h"
#include "cm_method.h"
#include "cm.h"
extern cmgr_cmfunc_t cmgr_cmfunc[];


/*
 *	CFGMGR:	Enter start up mode.
 */
void
startup_mode()
{
        cm_log_t	log;

	/*
	 *	Set up log for replies to syslog
	 */
	log.log_type 	= CM_LOG_SYSLOG;
	log.log_fd 	= 1;
	log.log_domain 	= AF_UNSPEC;
	if (CMGR.dflg)
		log.log_lvl	= LOG_DEBUG;
	else if (CMGR.vflg)
		log.log_lvl	= LOG_INFO;
	else
		log.log_lvl	= LOG_ERR;

	if (CMGR.lflg) {
		/*
		 * Delay for kloadsrv
		 */
		sleep(2);

		cfgmgr_log(LOG_INFO, cm_msg(MSG_ENTER_AUTOMODE), CMGR.progname);

		startup_config_auto_list(&log);

		cfgmgr_log(LOG_INFO, cm_msg(MSG_LEAVE_AUTOMODE), CMGR.progname);
	}
	return;
}


/*
 *	CFGMGR:	Enter mode for loading funcitons that are on the 
 *	MULTIUSER list.
 */
void
startup_multi( cm_log_t * logp )
{
	cfgmgr_log(LOG_INFO, cm_msg(MSG_ENTER_MULTIMODE), CMGR.progname);

	startup_config_multi_list(logp);

	cfgmgr_log(LOG_INFO, cm_msg(MSG_LEAVE_MULTIMODE), CMGR.progname);

	return;
}



/*
 * Get each AUTOMATIC subsystems from configuration database
 */
int
startup_config_auto_list( cm_log_t * logp )
{
	AFILE_t		afd;
	ENT_t		entry;
	ATTR_t		attr;
	char *		val;
	int		rc;
	int		cnt_success;
	int		cnt_failure;

	/*
	 *	Open database
	 */
        if (rc=dbfile_open_dflt(&afd)) {
		cfgmgr_log(LOG_ERR, "%s: %s\n", CMGR.database, cm_msg(rc));
		return(0);
	}

	if (rc=dbent_lookup(afd, AUTOENTRY, &entry)) {
		cfgmgr_log(LOG_INFO, "%s: %s\n", AUTOENTRY, cm_msg(rc));
		return(0);
	}

	if ((attr=AFgetatr(entry, AUTO_DYNAMIC)) == NULL) {
		cfgmgr_log(LOG_INFO, "%s: %s: %s\n", AUTOENTRY,
			AUTO_DYNAMIC, cm_msg(DBATR_ENOENT));
		return(0);
	}

	cnt_success = 0;
	cnt_failure = 0;
	while ( (val=AFnxtval(attr)) != NULL ) {

		if (strcmp(val, KEYWORD_NONE) == 0)
			continue;

		/*
		 *	Load and Configure subsystem
		 */
		rc = method_call(logp, val, 
			CM_OP_START | CM_OP_LOAD | CM_OP_CONFIGURE, 0);
		if (rc == 0) {
			cnt_success++;
		} else if (rc != METHOD_EFAIL) {
			cfgmgr_log(LOG_ERR, "%s: %d\n", val, rc);
			cnt_failure++;
		}
	}

	dbfile_close(afd);

	if (cnt_success || cnt_failure)
		cfgmgr_log(LOG_INFO, cm_msg(MSG_AUTOCONFIGURED),
			CMGR.progname, cnt_success, cnt_failure);
	else
		cfgmgr_log(LOG_INFO, cm_msg(MSG_NOAUTOMATIC),
			CMGR.progname);

	return(0);
}



/*
 * Get each MULTIUSER subsystem from configuration database
 */
int
startup_config_multi_list( cm_log_t * logp )
{
	AFILE_t		afd;
	ENT_t		entry;
	ATTR_t		attr;
	char *		val;
	int		rc;
	int		cnt_success;
	int		cnt_failure;

	/*
	 *	Open database
	 */
        if (rc=dbfile_open_dflt(&afd)) {
		cfgmgr_log(LOG_ERR, "%s: %s\n", CMGR.database, cm_msg(rc));
		return(0);
	}

	if (rc=dbent_lookup(afd, MULTIUSER, &entry)) {
		cfgmgr_log(LOG_INFO, "%s: %s\n", MULTIUSER, cm_msg(rc));
		return(0);
	}

	if ((attr=AFgetatr(entry, AUTO_DYNAMIC)) == NULL) {
		cfgmgr_log(LOG_INFO, "%s: %s: %s\n", MULTIUSER,
			AUTO_DYNAMIC, cm_msg(DBATR_ENOENT));
		return(0);
	}

	cnt_success = 0;
	cnt_failure = 0;
	while ( (val=AFnxtval(attr)) != NULL ) {

		if (strcmp(val, KEYWORD_NONE) == 0)
			continue;

		/*
		 *	Load and Configure subsystem
		 */
		rc = method_call(logp, val, 
			CM_OP_START | CM_OP_LOAD | CM_OP_CONFIGURE, 0);
		if (rc == 0) {
			cnt_success++;
		} else if (rc != METHOD_EFAIL) {
			cfgmgr_log(LOG_ERR, "%s: %d\n", val, rc);
			cnt_failure++;
		}
	}

	dbfile_close(afd);

	if (cnt_success || cnt_failure)
		cfgmgr_log(LOG_INFO, cm_msg(MSG_AUTOCONFIGURED),
			CMGR.progname, cnt_success, cnt_failure);
	else
		cfgmgr_log(LOG_INFO, cm_msg(MSG_NOMULTIUSER),
			CMGR.progname);

	return(0);
}

