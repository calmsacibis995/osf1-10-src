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
static char *rcsid = "@(#)$RCSfile: ses_launch.c,v $ $Revision: 1.1.9.7 $ (DEC) $Date: 1993/12/16 23:55:55 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_launch( int (*collect)(),
*			SIAENTITY *entity,
*			int pkgind)
*
* Description: The purpose of this routine is to do any final logging or 
* accounting of the session which will begin on return to the calling 
* program.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*			Very little should cause a FAIL at this time.
******************************************************************************/
#include <paths.h>
#include <utmp.h>
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"
#include "pathnames.h"
#ifndef	AUDIT_MASK_TYPE
#define	AUDIT_MASK_TYPE	char
#endif
#ifndef	PRIO_PROCESS
#include <sys/resource.h>
#endif

#ifdef MSG
#include "libsec_msg.h" 
#define MSGSTR_SEC(n,s) GETMSG(MS_SIA,n,s) 
#else
#define MSGSTR_SEC(n,s) s
#endif

/*
 * This routine sets up some things in the mechanism structure to be used by
 * the release routine, since some actions should not be taken until then.
 */

static int
setup_mechp(mechp)
C2_MECH *mechp;
{
	struct pr_passwd *pr = mechp->prpwd;
	AUDIT_MASK_TYPE am[AUDIT_MASK_LEN];
	char *audmaskp;
	int nicety;

/* Add show_perror & audgenl's here for error cases ? */

	if (pr && pr->uflg.fg_auditdisp)
		audmaskp = pr->ufld.fd_auditdisp;
	else if (pr && pr->sflg.fg_auditdisp)
		audmaskp = pr->sfld.fd_auditdisp;
	else
		audmaskp = NULL;
	if (audmaskp) {
		(void) audit_build_mask(audmaskp, mechp->audmask, (char *)0, 0);
		/* add perror, audgenl, etc. on error check ? */
	}
	if (pr && pr->uflg.fg_auditcntl)
		mechp->audit_cntl = pr->ufld.fd_auditcntl;
	else if (pr && pr->sflg.fg_auditcntl)
		mechp->audit_cntl = pr->sfld.fd_auditcntl;
	else if (!mechp->setup_done)
		mechp->audit_cntl = AUDIT_OR;
	if (pr && pr->uflg.fg_nice)
		mechp->new_nice = pr->ufld.fd_nice;
	else if (pr && pr->sflg.fg_nice)
		mechp->new_nice = pr->sfld.fd_nice;
	else if (!mechp->setup_done) {
		errno = 0;
		nicety = getpriority(PRIO_PROCESS, 0);
		if (errno && nicety==-1)
			nicety = PRIO_MAX+1;
		mechp->new_nice = nicety;
	}
	mechp->setup_done = 1;	/* tell ses_release there's something to do */
}

/*
 * Internal function to update the last login database. -DAL001
 */
static void dolastlog(int (*collect)(), SIAENTITY *entity, struct utmp *ut)
{
	struct lastlog ll;
	int fd;

	if ((fd = open(_PATH_LASTLOG, (O_RDWR|O_CREAT), 0666)) >= 0)
	{
		(void)lseek(fd, (off_t)entity->pwd->pw_uid * sizeof(ll), L_SET);
		bzero((char *)&ll, sizeof(ll));
		(void)time(&ll.ll_time);
		(void)strncpy(ll.ll_line, ut->ut_line, sizeof(ll.ll_line));
		if(entity->hostname)
			(void)strncpy(ll.ll_host, ut->ut_host, sizeof(ll.ll_host));
		(void)write(fd, (char *)&ll, sizeof(ll));
		(void)close(fd);
	}
}


siad_ses_launch(collect, entity, pkgind)
int (*collect)();
SIAENTITY *entity;
int pkgind;
{
	struct passwd *pwd;
	struct pr_passwd *prpwd=NULL;
	struct pr_term *prtc=NULL;
	extern int (*_c2_collect)();
	extern int _c2_collinput;
	char *audmaskp;

	_c2_collect = collect;
	if(!entity)
		return SIADFAIL;
	_c2_collinput = entity->colinput;
/*
 * If we don't have a passwd struct yet we have a problem.
 */
	if(!(pwd=entity->pwd)) {
		if(EN_MECH(entity, pkgind)) {
			free(EN_MECH(entity, pkgind));
			EN_MECH(entity, pkgind) = (C2_MECH *) 0;
		}
		if(!entity->acctname)
			if(entity->name)
				entity->acctname = strdup(entity->name);
			else
				return SIADFAIL;
		if(!(pwd=getpwnam(entity->acctname)))
			return SIADFAIL;
		sia_make_entity_pwd(pwd, entity);
	}

	if(!EN_MECH(entity, pkgind)) {
		prpwd = getprpwnam(entity->acctname);
		c2_make_mech(entity, pkgind, prpwd, (PR_TERM *) 0);
	}

	if(entity->authtype != SIA_A_SUAUTH) {
		struct utmp utmp;
		char *tty, tname[PATH_MAX], *s;
/*
 * Look up the passwd and protected passwd information and copy it into
 * the entity structure.  Not that a local name must exist by now or
 * this will fail.
 */
		if(!EN_PR_PWD(entity, pkgind)) {
			if(!entity->acctname)
				return SIADFAIL;
			if(!login_fillin_user(entity->acctname, &prpwd, &pwd))
				return SIADFAIL;
			EN_PR_PWD(entity, pkgind) = prpwd;
			sia_make_entity_pwd(pwd, entity);
		} else
			prpwd = EN_PR_PWD(entity, pkgind);
		prtc = EN_PR_TRM(entity, pkgind);
/*
 * Last check to make sure the passwd and protected passwd information
 * has been retrieved.
 */
		if(!(entity->pwd) || !(prpwd))
			return SIADFAIL;
/*
 * Set up auditmask and save auditcntl & priority values for ses_release.
 */
		(void) setup_mechp(EN_MECH(entity, pkgind));

/*
 * Change to accounts initial working directory.
 */
		if (sia_chdir(pwd->pw_dir, -1) < 0) {
			(void)show_error(MSGSTR(NO_DIR, "No directory!"));
			if (chdir(DEF_DIR))
				return SIADFAIL;
			pwd->pw_dir = DEF_DIR;
			(void)show_error(MSGSTR(LOG_IN, "Logging in with home = \"/\"."), DEF_DIR);
		}
/*
 * Establish the user name for the kernel.
 */
		setlogin(entity->acctname);

		if(entity->tty) {
		if(!prtc)
			return SIADFAIL;
/*
 * Write utmp record into utmp file to indicate a logged-in user.
 */
		bzero((char *)&utmp, sizeof(utmp));
		(void)time(&utmp.ut_time);
		(void)strncpy(utmp.ut_name, entity->acctname, sizeof utmp.ut_name);
		if(entity->hostname && *entity->hostname) {
			s = strchr(entity->hostname, '@');
			if (s)
				s++;
			else
				s = entity->hostname;
			(void)strncpy(utmp.ut_host, s, sizeof utmp.ut_host);
		}
		tty = entity->tty;
		if(!tty || !*tty) {
			strncpy(tname, _PATH_TTY, sizeof tname);
			strncat(tname, "??", sizeof tname);
			tty = tname;
		}
		if(s=strrchr(tty, '/'))
			s++;
		else
			s = tty;
		(void)strncpy(utmp.ut_line, s, sizeof utmp.ut_line);
		login(&utmp);
		dolastlog(collect, entity, &utmp);
/*
 * Set terminal ownerships and modes and update terminal database.
 */
		prtc = login_good_tty(prtc, prpwd);
		}
/*
 * Display and record new user access information.
 */
		if(!login_good_user(&prpwd, &prtc, pwd))
			return SIADFAIL;
/*
 * Log dial-in activity.
 */
	}
	else {
/*
 * Launch sub-session as for the "su" utility.
 */
		char *p;

#ifdef	XYZ
/*
 * Change to accounts initial working directory.
 */
/* Another hack to look for "su -". */
		if(entity->argc >=2 && !strcmp(entity->argv[1], "-"))
			if (chdir(pwd->pw_dir) < 0) {
				(void)show_error(MSGSTR(NO_DIR, "No directory!"));
				if (chdir(DEF_DIR))
					return SIADFAIL;
				pwd->pw_dir = DEF_DIR;
				(void)show_error(MSGSTR(LOG_IN, "Logging in withhome = \"/\"."), DEF_DIR);
			}
#endif	/* XYZ */
		su_set_login(pwd->pw_uid);
/*
 * Set up auditmask and save auditcntl & priority values for ses_release.
 */
		(void) setup_mechp(EN_MECH(entity, pkgind));

/*
 * Set process identities.
 */
		if (setgid(pwd->pw_gid) < 0) {
			audit_subsystem(MSGSTR_SEC(S_SET_GROUP_ID, "set group ID to specified user"),
			MSGSTR_SEC(S_INVALID, "invalid id; unable to set, abort su"), 18 /* ET_SUBSYSTEM */);
			show_error("%s %s\n", MSGSTR(SET_GID, "su: setgid"), sys_errlist[errno]);
			return SIADFAIL;
		}
		if (initgroups(pwd->pw_name, pwd->pw_gid)) {
			audit_subsystem(MSGSTR_SEC(S_GROUP_ID, "set supplementary group IDs to specified user"),
			MSGSTR_SEC(S_INVALID, "invalid id; unable to set, abort su"), 18 /* ET_SUBSYSTEM */);
			show_error(MSGSTR(INITGROUPS, "su: initgroups failed\n"));
			return SIADFAIL;
		}
/*
 * Set only the effective UID to allow the application a last chance
 * to do something before possibly losing superuser privileges.
 */
		if (seteuid(pwd->pw_uid) < 0) {
			audit_subsystem(MSGSTR_SEC(S_SET_USER_ID, "set user ID to specified user"),
			  MSGSTR_SEC(S_INVALID, "invalid id; unable to set, abort su"), 18 /* ET_SUBSYSTEM */);
			show_error("%s %s\n", MSGSTR(SETUID, "su: setuid"), sys_errlist[errno]);
			return SIADFAIL;
		}
        /* This was raised in su_set_login */
		disablepriv(SEC_SETPROCIDENT);
	}
	return SIADSUCCESS;
}
