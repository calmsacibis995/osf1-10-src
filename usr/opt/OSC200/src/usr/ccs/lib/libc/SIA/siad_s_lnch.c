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
static char *rcsid = "@(#)$RCSfile: siad_s_lnch.c,v $ $Revision: 1.1.15.5 $ (DEC) $Date: 1993/10/29 17:36:30 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak siad_ses_launch = __siad_ses_launch
#endif
#endif
#include <sys/types.h>
#include <pathnames.h>
#include <lastlog.h>

#include "siad.h"
#include "siad_bsd.h"

/*
 * Internal function to update the last login database. -DAL001
 */
static void dolastlog(int (*collect)(), SIAENTITY *entity, struct utmp *ut)
{
	struct lastlog ll;
	int fd;

	if ((fd = open(_PATH_LASTLOG, (O_RDWR|O_CREAT), 0666)) < 0)
		SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"), MSGSTR(CANTOPEN,"Can't open %s"), _PATH_LASTLOG);
	else {
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

/*****************************************************************************
*
*	int	siad_ses_launch( (*sia_collect),
*                               SIAENTITY *entityhdl)
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
#define TTYGRPNAME      "tty"
#define	DEF_DIR		"/"

int
  siad_ses_launch (int (*collect)(), SIAENTITY *entity, int pkgind)
{
	struct utmp utmp;
	struct passwd *pwd;
	char *s, *tty, ttypath[MAXPATHLEN];
	struct group *gr;
	prompt_t info_prompt;

	if(entity == NULL)
                return SIADFAIL;
        if(entity->pwd == NULL)
                return SIADFAIL;
        if(entity->authtype != SIA_A_SUAUTH)
                setlogin(entity->acctname);
	pwd = entity->pwd;
	bzero((char *)&utmp, sizeof(utmp));
	(void)time(&utmp.ut_time);
	(void)strncpy(utmp.ut_name, pwd->pw_name, sizeof(utmp.ut_name));
	s=entity->tty;
	if(s) {
		if(tty = rindex(s, '/'))
			++tty;
		else
			tty = s;
		(void)strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));
		if(entity->hostname) {			/* DAL001 */
			s = strchr(entity->hostname, '@');
			if (s)
				s++;
			else
				s = entity->hostname;
			strncpy(utmp.ut_host, s, sizeof utmp.ut_host);	/* DAL001 */
		}
		login(&utmp);
		dolastlog(collect, entity, &utmp);	/* DAL001 */
		if(entity->tty[0] == '/') {			/*DAL002*/
			chown(entity->tty, entity->pwd->pw_uid,(gr = getgrnam(TTYGRPNAME)) ? gr->gr_gid : pwd->pw_gid);
			(void)chmod(entity->tty, 0620);
		}
	}
	if(setgid((gid_t)entity->pwd->pw_gid) < 0 ||		/*DAL002*/
	    initgroups(entity->pwd->pw_name, entity->pwd->pw_gid) != 0 ||
	    seteuid(entity->pwd->pw_uid) < 0)
		return SIADFAIL;
	if(entity->authtype != SIA_A_SUAUTH)			/*DAL003*/
	if (sia_chdir(pwd->pw_dir, -1) != SIASUCCESS) {		/*DAL002*/
		if(collect != NULL)
			sia_warning(collect, MSGSTR(NO_DIR, "No directory!"));
		if (chdir(DEF_DIR)) 	/* should this be sia_chdir, too? */
			return SIADFAIL;
		pwd->pw_dir = DEF_DIR;
		if(collect != NULL)
                	{
			info_prompt.prompt = (unsigned char *) MSGSTR(LOG_IN, 
		       "Logging in with home = \"/\".");
			(void)(*collect)(0, SIAINFO, "", 1, &info_prompt);
			}
		
	}
	return SIADSUCCESS;
}
