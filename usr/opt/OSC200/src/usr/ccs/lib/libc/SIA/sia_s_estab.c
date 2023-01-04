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
static char *rcsid = "@(#)$RCSfile: sia_s_estab.c,v $ $Revision: 1.1.12.7 $ (DEC) $Date: 1994/01/21 20:05:45 $";
#endif
/*****************************************************************************
* Usage:  int * sia_ses_estab((*sia_collect),
*                               SIAENTITY *sia_entity_handle )
*
* Description: The purpose of this routine is to complete the process of 
* establishing a session for login. This processing allocates appropriate
* security relevent structures and forms the user information, pwent, 
* by calling one or all of the configured security mechanisms. If this routine
* is successful 
* This routine utilizes a vector of switches to drive the calling sequence of 
* one or more security mechanism dependent session establishment routines.
* The only processing which should remain is the actual logging, accounting,
* and process startup associated with a login. This final section of the 
* login process is done by sia_launch_session.
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
*	Param1: sia_collect
*       Usage: Collection of  parameters from the calling program/user.
*	Syntax: If this parameter is NULL non-interactive is assumed.
*
*	Param2: loginname
*	Usage: this field is only necessary if the entity has none been established
*		rlogin may use this option when .rhosts has authorized the skipping
*		of authentication.
*
*	Param3: command_name
*	Usage: this is a mandatory parameter to be filled in with the calling commands name 
*		Like: "login" or "su" or "ftp"
*
*	Param2: sia_entity_handle
*       Usage: Returned as non NULL if session establishment is successful. Used in subsequent SIA calls
*	Syntax: non_zero
*
* Assumed Inputs: None
*
* Success return: SIASUCCESS accompanied with a non_NULL entityport handle.
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
#pragma weak sia_ses_estab = __sia_ses_estab
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"
#include <sys/secdefines.h>
#include <sys/security.h>	/* needed for SEC_SETLUID */
#include "pathnames.h"		/* needed for _PATH_BSHELL */

int
  sia_ses_estab (int (*collect)(), SIAENTITY *entityhdl)
{
        int siaderr=(SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=SIADFAIL;                /* sia status between mechanism calls */
	char audtok[N_AUDTUPLES];	/* audit type codes */
	char *audval[N_AUDTUPLES];	/* audit information */
	char remhost[MAXHOSTNAMELEN];	/* parsed-out remote host for audit */
	char remuser[MAXHOSTNAMELEN];	/* parsed-out remote user for audit */
	char *cp=0;			/* pointer for above */
        int pkgind=0;                   /* package index */
	int i,j;

	SIATHREADLOCK(SIA_ESTAB_LOCK)
	
	if(entityhdl==NULL)		/* sia_ses_init must be called before establish session */
		{
		SIATHREADREL(SIA_ESTAB_LOCK)
		return(SIAFAIL);
                }
        if(sia_init() == SIAFAIL)
                {
                SIATHREADREL(SIA_ESTAB_LOCK)
		sia_ses_release(&entityhdl);
                return(SIAFAIL);
                }
	if(sia_authorize(entityhdl)==SIAFAIL)
		{
                SIATHREADREL(SIA_ESTAB_LOCK)
                sia_ses_release(&entityhdl);
                return(SIAFAIL);
                }
        while(pkgind < SIASWMAX && sia_matrix[SIA_SES_ESTAB][pkgind].pkgnam != NULL)
                {
		if(sia_matrix[SIA_SES_ESTAB][pkgind].fp == NULL)
			{
			if (sia_setupfp(SIA_SES_ESTAB, pkgind) == SIAFAIL) {
				SIATHREADREL(SIA_ESTAB_LOCK);
				return SIAFAIL;
				}
			}
		siaderr = (*sia_matrix[SIA_SES_ESTAB][pkgind].fp)((collect),entityhdl,pkgind);
		if(siaderr & SIADSUCCESS)
			{
			siastat=SIADSUCCESS;
			if (siaderr & SIADSTOP)
				goto out;
			}
		else if(siaderr & SIADSTOP) /* Fail and stop case */
			goto out;
		pkgind++;
		}
    out:			/* make an audit entry */
	if ((siaderr & SIADSUCCESS)
		&& entityhdl->pwd && entityhdl->pwd->pw_uid != (uid_t)-1)
	{
		errno = 0;	/* just in case */
		if (security(SEC_SETLUID, entityhdl->pwd->pw_uid,
			     0L, 0L, 0L, 0L) == -1 && errno != 0)
			siaderr = SIADFAIL; /* can't set it? */
	}
	if (siaderr & SIADSUCCESS)
	{
		goto out1;	/* defer audit of success to launch code */
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
	if ((audgen(LOGIN, audtok, audval, 0L, 0L) < 0) && errno != ENOSYS) {
		(void) syslog(LOG_ERR|LOG_AUTH, "audgen(LOGIN): %m");
	}
	if (cp) {
		bzero(remhost, sizeof remhost);
		bzero(remuser, sizeof remuser);
	}
out1:
	if((siastat & SIADSUCCESS) && (siaderr & SIADSUCCESS)) /* local mechanism override */
		{
                SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_SESESTABGOOD, "Successful establishment of session"));
                SIATHREADREL(SIA_ESTAB_LOCK)
                return(SIASUCCESS);
                }
	else	{
		sia_ses_release(&entityhdl);
                SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_SESESTABAD,"Failure to establish session"));
                SIATHREADREL(SIA_ESTAB_LOCK)
		return(SIAFAIL);
        	}
}
