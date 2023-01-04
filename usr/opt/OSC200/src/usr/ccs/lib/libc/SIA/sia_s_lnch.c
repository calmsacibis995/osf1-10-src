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
static char *rcsid = "@(#)$RCSfile: sia_s_lnch.c,v $ $Revision: 1.1.12.6 $ (DEC) $Date: 1994/01/21 20:05:49 $";
#endif
/*****************************************************************************
* Usage:  int * sia_ses_launch(((*sia_collect),
*				 SIAENTITY *sia_entity_handle )
*
* Description: The purpose of this routine is to complete the process of 
* establishing a session for login. This is the last sia routine called by
* login before execing the shell for the session. The launch function should
* not require any additional prompting and is primarily used to do accounting
* or auditing of the new session. No errors are anticipated from this call 
* however system resources may become depleated causing this call to fail.
* This routine utilizes a vector of switches to drive the calling sequence of 
* one or more security mechanism dependent session launch routines.
* The execing of the users
* specified shell in the home directory will not happen in this routine.
* this is the responsibility of the calling login or xdm program. The returned
* sia_entity_handle points to a entity structure which contains the appropriate
* shell and home.
*
*	Internal return codes, sia_siaderr, are: 	
*	(SUCCESS | CONTINUE) - Used to allow other session estblishment mechanisms
*				the ability to setup the session. This case might
*				happen when a user is registered with more than one
*				mechanism. This is the default SUCCESS return.
*
*	(SUCCESS | STOP) - Used when no other session estblishment is required. "root"
*
*	(FAILURE | CONTINUE) - Used to allow other session establishment mechanisms
*				the ability to setup. This case would
*				be used when a user has no distributed registry but 
*				may be regitered locally.
*
*	(FAILURE | STOP) - Absolute denial of session establishment. 
*
* Parameter Descriptions: 
*
*	Param1: sia_entity_handle (Input parameter)
*       Usage: Returned as non NULL if session establishment is successful. Used in subsequent SIA calls
*	Syntax: non_zero
*
*
* Assumed Inputs: None
*
* Success return: SIASUCCESS accompanied with a non_NULL sia_entity handle.
*
* Error Conditions and returns otherthan parameter syntax checks listed above:
*
* 	Condition1: loader failure to load libsia or a package within libsia
*	Return: SIALDRERR
*
* 	Condition2: failure to properly establish session
*	Return: SIAFAIL
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_ses_launch = __sia_ses_launch
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"
#include "pathnames.h"		/* needed for _PATH_BSHELL */

int
  sia_ses_launch (int (*collect)(), SIAENTITY *entityhdl)
{
#ifndef SV_SHLIBS
        ldr_module_t    lib_handle = NULL;
        int             (*fp) ();
#else
        void *lib_handle;
        void *fp;
#endif
        int siaderr=(SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=SIADFAIL;                  /* sia status between mechanism calls */
        int pkgind=0;                   /* package index */
	char audtok[N_AUDTUPLES];	/* audit type codes */
	char *audval[N_AUDTUPLES];	/* audit information */
	char remhost[MAXHOSTNAMELEN];	/* parsed-out remote host for audit */
	char remuser[MAXHOSTNAMELEN];	/* parsed-out remote user for audit */
	char *cp=0;			/* pointer for above */
	uid_t savuid;			/* so we can use audgen(2) */
	int i,j;

	SIATHREADLOCK(SIA_LAUNCH_LOCK)
	
        if(sia_init() == SIAFAIL)
                {
		sia_ses_release(&entityhdl);
                SIATHREADREL(SIA_LAUNCH_LOCK)
                return(SIAFAIL);
                }
	if(entityhdl == NULL)
		{
		SIATHREADREL(SIA_LAUNCH_LOCK)
		return(SIAFAIL);
                }
        while(pkgind < SIASWMAX && sia_matrix[SIA_SES_LAUNCH][pkgind].pkgnam != NULL)
                {
		if(sia_matrix[SIA_SES_LAUNCH][pkgind].fp == NULL)
			{
			if (sia_setupfp(SIA_SES_LAUNCH, pkgind) == SIAFAIL) {
				SIATHREADREL(SIA_LAUNCH_LOCK);
				return SIAFAIL;
				}
			}
		siaderr = (*sia_matrix[SIA_SES_LAUNCH][pkgind].fp)(collect,entityhdl,pkgind);
		if(siaderr & SIADSUCCESS)
			{
			siastat=SIADSUCCESS;
			}
		if (siaderr & SIADSTOP)
			break;
		pkgind++;
		}
	/* make audit entry if this is a login */
	if (entityhdl->authtype == SIA_A_SUAUTH || entityhdl->authtype == SIA_A_REAUTH)
	{
		goto out;	/* skip if already sent the audit record */
	}
	i = 0;			/* index into audit arrays */
	if (entityhdl->pwd && entityhdl->pwd->pw_name)
	{
		audtok[i] = T_LOGIN;
		audval[i] = entityhdl->pwd->pw_name;
		i++;
	}
	else if (getpwnam(entityhdl->name) != NULL)
	{
		audtok[i] = T_LOGIN;
		audval[i] = entityhdl->name;
		i++;
		entityhdl->error = EACCES;
	}
	else if ((j=audcntl(GET_AUDSTYLE,0L,0L,0L,0L,0L))>=0 && (j&AUD_LOGIN_UNAME))
	{
		/* only log attempted bad username if AUD_LOGIN_UNAME, since */
		/* it could be a the user's password entered out of order */
		audtok[i] = T_LOGIN;
		audval[i] = entityhdl->name;
		i++;
		entityhdl->error = ENOENT;
	}
	else
	{
		/* Must put in something here so that remote ID is */
		/* recognized as being remote. */

		audtok[i] = T_LOGIN;
		audval[i] = "(unknown)";
		i++;
		entityhdl->error = ENOENT;
	}
	if (entityhdl->pwd && entityhdl->pwd->pw_dir && *entityhdl->pwd->pw_dir)
	{
		audtok[i] = T_HOMEDIR;
		audval[i] = entityhdl->pwd->pw_dir;
		i++;
	}
	if (entityhdl->pwd) {
		audtok[i] = T_SHELL;
		if (entityhdl->pwd->pw_shell && *entityhdl->pwd->pw_shell)
			audval[i] = entityhdl->pwd->pw_shell;
		else
			audval[i] = _PATH_BSHELL;
		i++;
	}
	if (entityhdl->tty && *entityhdl->tty) {
		audtok[i] = T_DEVNAME;
		audval[i] = entityhdl->tty;
		i++;
	}
	if (siaderr & SIADSUCCESS) {
		audtok[i] = T_CHARP;
		audval[i] = "Login succeeded";
		i++;
		audtok[i] = T_RESULT;
		audval[i] = 0;
		i++;
	}
	else {
		audtok[i] = T_CHARP;
		if ((entityhdl->pwd && entityhdl->pwd->pw_name)
		    || entityhdl->error == EACCES)
			audval[i] = "Failed authentication";
		else
			audval[i] = "Invalid account";
		i++;
		audtok[i] = T_ERRNO;
		audval[i] = (char *) (entityhdl->error);
		i++;
	}
	if (entityhdl->hostname)
	{
		audtok[i] = T_HOSTNAME;
		audval[i] = entityhdl->hostname;
		if ((cp = strchr(entityhdl->hostname, '@')) != NULL)
		{
			(void) strncpy(remuser, entityhdl->hostname,
				       cp - entityhdl->hostname);
			remuser[cp - entityhdl->hostname] = '\0';
			(void) strcpy(remhost, ++cp);
			audval[i] = remhost;
			audtok[++i] = T_LOGIN;
			audval[i] = remuser;
		}
		else if ((cp = strstr(entityhdl->hostname, "::")) != NULL
			 && cp > entityhdl->hostname)
		{
			cp += 2;
			(void) strncpy(remhost, entityhdl->hostname,
				       cp - entityhdl->hostname);
			remhost[cp - entityhdl->hostname] = '\0';
			(void) strcpy(remuser, cp);
			audval[i] = remhost;
			audtok[++i] = T_LOGIN;
			audval[i] = remuser;
		}
		i++;
	}
	audtok[i] = 0;
	audval[i] = 0;
	savuid = geteuid();
	setreuid(getuid(),0);
	if ((audgen(LOGIN, audtok, audval, 0L, 0L) < 0) && errno != ENOSYS)
	{
		(void) syslog(LOG_ERR|LOG_AUTH, "audgen(LOGIN): %m");
	}
	setreuid(getuid(),savuid);
	if (cp)
	{
		bzero(remhost, sizeof remhost);
		bzero(remuser, sizeof remuser);
	}
	if (entityhdl->tty && (siaderr & SIADSUCCESS))
	{
		cp = strrchr(entityhdl->tty, '/');
		if (cp)
			cp++;
		else
			cp = entityhdl->tty;
		if (cp[sizeof "tty" - 1] == 'd')
			syslog(LOG_INFO|LOG_AUTH, MSGSTR(SIA_MSG_DIALUP, "DIALUP: %s %s"),
				entityhdl->tty, entityhdl->pwd->pw_name);
		if (entityhdl->pwd->pw_uid == 0)
			syslog(LOG_NOTICE|LOG_AUTH,
				MSGSTR(SIA_MSG_ROOTLOGIN, "ROOT login on %s"),
				entityhdl->tty);
	}
out:
	if((siastat & SIADSUCCESS) && (siaderr & SIADSUCCESS)) /* local-last  mechanism override */
		{
                SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_SESLAUNCHGOOD,"Successful launching of session"));
                SIATHREADREL(SIA_LAUNCH_LOCK)
                return(SIASUCCESS);
                }
	else	{
		sia_ses_release(&entityhdl);
                SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_SESLAUNCHBAD,"Failure to launch  session"));
                SIATHREADREL(SIA_LAUNCH_LOCK)
		return(SIAFAIL);
        	}
}
