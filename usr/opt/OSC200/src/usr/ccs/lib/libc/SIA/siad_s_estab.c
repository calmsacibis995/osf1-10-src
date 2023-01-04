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
static char *rcsid = "@(#)$RCSfile: siad_s_estab.c,v $ $Revision: 1.1.13.7 $ (DEC) $Date: 1993/12/08 22:53:07 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_estab((*sia_collect),
*				SIAENTITY *entityhdl)
*
* Description: The purpose of this routine is to do session establishment.
* The assumption is that all necessary authntication has been accomplished.
* The work done by this routine consists of collecting any additional state
* required to actually launch a session. If the collect function pointer is
* NULL this is a non_interactive request.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak siad_ses_estab = __siad_ses_estab
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"
#include "pathnames.h"
#include <stdio.h>
#include <sys/secdefines.h>
#include <sys/security.h>	/* needed for SEC_SETLUID */

static void
dolastlog(collect, entity, quiet)
	int (*collect)();
	SIAENTITY *entity;
	int quiet;
{
	struct lastlog ll;
	int fd;
	char *ctime();
	struct passwd *pwd;
	char obuf[BUFSIZ];

	pwd = entity->pwd;
	if ((fd = open(_PATH_LASTLOG, (O_RDWR|O_CREAT), 0666)) < 0)
		sia_warning(collect, MSGSTR(CANTOPEN,"Can't open %s"), _PATH_LASTLOG);
	else {
		(void)lseek(fd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		if ((!entity->tty || entity->tty[0] != '/') &&
		    collect != sia_collect_trm)
			quiet = 1;	/* don't do this with xdm */
		if (!quiet) {
			if (read(fd, (char *)&ll, sizeof(ll)) == sizeof(ll) &&
			    ll.ll_time != 0) {
				(void)sprintf(obuf, MSGSTR(SIA_LAST_LOG, "Last login: %.*s "),
				    24-5, (char *)ctime(&ll.ll_time));
				if (*ll.ll_host != '\0')
					(void)sprintf(&obuf[strlen(obuf)], MSGSTR(SIA_LAST_FROM, "from %.*s\n"),
					    sizeof(ll.ll_host), ll.ll_host);
				else
					(void)sprintf(&obuf[strlen(obuf)], MSGSTR(SIA_LAST_ON, "on %.*s\n"),
					    sizeof(ll.ll_line), ll.ll_line);
			}
			sia_warning(collect, "%s", obuf);
		}
		(void)close(fd);
	}
}

#ifdef OSF
/*
 *  Check if the specified program is really a shell (e.g. "sh" or "csh").
 */
static
usershell(shell)
	char *shell;
{
	register char *p;
	char *getusershell();

	setusershell();
	while ((p = getusershell()) != NULL)
		if (strcmp(p, shell) == 0)
			break;
	endusershell();
	return(!!p);
}

#endif	/* OSF */

int
  siad_ses_estab (int (*collect)(), SIAENTITY *entity, int pkgind)
{
	struct passwd *pwd;
	int quietlog;
	char *hush_path[PATH_MAX];

	if(entity == NULL)
		return SIADFAIL;	/* What no entity? */

	if(entity->acctname == NULL)	/* What no account name? */
		{
		if(entity->name == NULL) { /* how about just a name? */
			entity->error = EINVAL;
			return SIADFAIL;
		}
		entity->acctname = (char *)malloc(strlen(entity->name)+1);
		if(entity->acctname == NULL) {
			entity->error = ENOMEM;
			return SIADFAIL;
		}
		strcpy(entity->acctname, entity->name);
		}
	if(entity->pwd == NULL)
		{	/* No pwd yet! I guess that means its my job */
		if(siad_getpwnam(entity->acctname, &siad_bsd_passwd, siad_bsd_getpwbuf,SIABUFSIZ) == SIADFAIL) {
			entity->error = ESRCH;
                	return SIADFAIL;
		}
		else	pwd = &siad_bsd_passwd;
		if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
			return(SIADFAIL);
		}
	pwd=entity->pwd;
/*
 * Check for logins disabled. -DAL001
 */
	if(entity->tty && pwd->pw_uid) {		/*DAL002*/
		FILE *fp;

		if(fp=fopen(_PATH_NOLOGIN, "r")) {
			char s[8192];
			int i;

			if((i=fread(s, sizeof (char), (sizeof s)-1, fp)) > 0) {
				s[i] = '\0';
				sia_warning(collect, s);
			}
			fclose(fp);
			entity->error = EEXIST;	/* pick one */
			return SIADFAIL;
		}
	}

	(void) security(SEC_SETLUID, pwd->pw_uid, 0L, 0L, 0L, 0L);

#ifdef Q_SETUID
	if (quota(Q_SETUID, pwd->pw_uid, 0, (char *)0) < 0 && errno != EINVAL) {
		entity->error = errno;
		switch(errno) {
		case EUSERS:
			if(entity->tty) {
				if(collect != NULL)
					sia_warning(collect, MSGSTR(TOO_MANY_USR,
					"Too many users logged on already.\nTry again later."));
			}
			break;
		case EPROCLIM:
			if(collect != NULL)
				sia_warning(collect, MSGSTR(TOO_MANY_PROC, "You have too many processes running."));
			return SIADFAIL;
			break;
		default:
			perror(MSGSTR(QUOTA, "quota (Q_SETUID)"));
			return SIADFAIL;
		}
	}
#endif
        /* Capacity limit exceeded? */                              /* BVT */

	if ( entity->tty && (pwd == NULL || pwd->pw_uid))
		if(setsysinfo(SSI_ULIMIT,0,0,0,0) != 0) {
			entity->error = errno;
			if(collect != NULL)
		        	sia_warning(collect, MSGSTR(TOO_MANY_USR,
				"Too many users logged on already.\nTry again later."));
			return SIADFAIL;
		}
	entity->error = 0;

	/*
	 * Display and update lastlog file entry.
	 */
	if ((entity -> pwd           != NULL) &&
	    (entity -> pwd -> pw_dir != NULL) &&
	    (entity -> pwd -> pw_dir [0] != 0))
		sprintf (hush_path, "%s/%s",
			 entity -> pwd -> pw_dir,
			 _PATH_HUSHLOGIN);
	else	strcpy (hush_path, _PATH_HUSHLOGIN);
	quietlog = access(hush_path, F_OK) == 0;
	if(!quietlog)
		quietlog = !*entity->pwd->pw_passwd && !usershell(entity->pwd->pw_shell);
	dolastlog(collect, entity, quietlog);

	return SIADSUCCESS;
}
