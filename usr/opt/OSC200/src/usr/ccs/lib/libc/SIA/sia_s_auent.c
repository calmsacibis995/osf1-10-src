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
static char *rcsid = "@(#)$RCSfile: sia_s_auent.c,v $ $Revision: 1.1.14.6 $ (DEC) $Date: 1994/01/21 20:05:42 $";
#endif
/*****************************************************************************
* Usage:  int * sia_ses_authent((*sia_collect)
*				char	*passkey,
*				SIAENTITY *entityhdl )
*
* Description: The purpose of this routine is to sufficiently identify the 
* calling entity. This routine utilizes a vector of switches to drive the 
* calling sequence of one or more security mechanism dependent authentication
* routines.  The entity_handle will be set non_NULL on a successful return.
*
* The sequence of the authentication switches is typically based on the strength
* and whether or not one mechanism can vouch for a subsequent mechanism. For
* example any root level process running locally can vouch for the local security
* mechanism. However a root level process running the local security mechanism
* may not be capable of vouching for a distributed private or public key mechanism.
* The returns from each call can continue or stop the calling sequence.
* A status/error code, siaderr, is used to process the switches
*
*	Internal return codes, sia_siaderr, are: 	
*	(SUCCESS | CONTINUE) - Used to allow other authentication mechanisms
*				the ability to authenticate. This case might
*				happen when a user is registered both locally
*				and distributed.
*
*	(SUCCESS | STOP) - Used when no other authentication mechanisms are 
*				either necessary and/or allowed. This case
*				might happen when a user like root is only
*				registered locally.
*
*	(FAILURE | CONTINUE) - Used to allow other authentication mechanisms
*				 the ability to authenticate. This case would
*				be used when a user has no local registry but 
*				does have a distributed registry.
*
*	(FAILURE | STOP) - Absolute denial of authentication. This case is used
*				when a local ACL refuses authentication of a 
*				particular entity.
*
* Parameter Descriptions: 
*
*	Param1: sia_collect
*       Usage: Collection of  parameters from the calling program/user.
*	Syntax: n.a.
*
*	Param2: loginname
*       Usage: optional argument containning a precollected loginname
*	Syntax: ascii string
*
*	Param3: command_name
*       Usage: calling command for example "login" or "xdm"
*	Syntax: ascii string
*
*	Param4: entityhdl
*       Usage: Returned as non NULL if authentication is successful. 
*		Used in subsequent SIA calls
*	Syntax: non_NULL if ero
*
*
* Assumed Inputs: None
*
* Success return: SIASUCCESS accompanied with a non_NULL siaentity handle.
*
* Error Conditions and returns otherthan parameter syntax checks listed above:
*
* 	Condition1: loader failure to load libsia or a package within libsia
*	Return: SIAFAIL
*
* 	Condition2: failure to properly authenticate
*	Return: SIAFAIL
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_ses_authent = __sia_ses_authent
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int
  sia_ses_authent (int (*collect)(), char *passkey, SIAENTITY *entity)
{
	
/*****************************************************************************************/
	prompt_t prompt;
        int siaderr=(SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=SIADFAIL;                  /* sia status between mechanism calls */
        int pkgind=0;                   /* package index */
	char	acctname[SIAMXACCTNAME]; 

	SIATHREADLOCK(SIA_AUTHENT_LOCK)
	bzero(acctname,sizeof(acctname));
        if(sia_init() == SIAFAIL)
                {
                SIATHREADREL(SIA_AUTHENT_LOCK)
                return(SIAFAIL);
                }
        if(entity == NULL)
                {
                SIATHREADREL(SIA_AUTHENT_LOCK)
                return(SIAFAIL);
                }
	if(passkey != NULL)
		{
		entity->password = (char *) malloc(strlen(passkey) + 1);
		if (entity->password == NULL) {
			entity->error = ENOMEM;
			SIATHREADREL(SIA_AUTHENT_LOCK)
			return SIAFAIL;
			}
		strcpy(entity->password,passkey);
		}
	entity->authtype = SIA_A_AUTH;
        while(pkgind < SIASWMAX && sia_matrix[SIA_SES_AUTHENT][pkgind].pkgnam != NULL)
                {
		if(sia_matrix[SIA_SES_AUTHENT][pkgind].fp == NULL)
			{
			if (sia_setupfp(SIA_SES_AUTHENT, pkgind) == SIAFAIL) {
				entity->error = EIO;
				SIATHREADREL(SIA_AUTHENT_LOCK);
				return SIAFAIL;
				}
			}
                siaderr = (*sia_matrix[SIA_SES_AUTHENT][pkgind].fp)((collect),entity,siastat,pkgind);
		if(siaderr & SIADSUCCESS)
                        {
                        siastat=SIADSUCCESS;
                        if( acctname[0] == NULL)
				strcpy(acctname,entity->acctname);	
                        else if(strcmp(acctname,entity->acctname))
                                {	
				/*** passports will need much more ***/
				sia_free_entity(entity,SIAAUTHFREE);
				entity->error = ENOMEM;
                                SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_ENTINCON, "SIA configuration entity inconsistency\n"));
                                SIATHREADREL(SIA_AUTHENT_LOCK)
				if(entity->password != NULL)
					bzero(entity->password, strlen(entity->password));
                                return(SIAFAIL);
                                }
                        if(siaderr & SIADSTOP)
                                {
                                SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_SESAUTHGOOD,"Successful session authentication for %s on %s"),entity->name,entity->tty);
                                SIATHREADREL(SIA_AUTHENT_LOCK)
				if(entity->password != NULL)
					bzero(entity->password, strlen(entity->password));
                                return(SIASUCCESS);
                                }
                        }
		else	if(siaderr & SIADSTOP)
                                {
				goto bad;
                                }
                pkgind++;
                }		/* override of last configured mechanism */
         if((siastat & SIADSUCCESS) && (siaderr & SIADSUCCESS))
		{
                SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_SESAUTHGOOD,"Successful session authentication for %s on %s"),entity->name,entity->tty);
                SIATHREADREL(SIA_AUTHENT_LOCK)
                if(entity->password != NULL)
                        bzero(entity->password, strlen(entity->password));
                return(SIASUCCESS);
                }
	 else   {
		int i, j;
		char audtok[N_AUDTUPLES];
		char *audval[N_AUDTUPLES];
		char remhost[MAXHOSTNAMELEN];
		char remuser[MAXHOSTNAMELEN];
		char *cp=0;
bad:		/*** passports will need more ***/
		i = 0;			/* index into audit arrays */
		if (entity->pwd && entity->pwd->pw_name && *entity->pwd->pw_name)
		{
			audtok[i] = T_LOGIN;
			audval[i] = entity->pwd->pw_name;
			i++;
		}
		else if (getpwnam(entity->name) != NULL)
		{
			audtok[i] = T_LOGIN;
			audval[i] = entity->name;
			i++;
			entity->error = EACCES;
		}
		else if ((j=audcntl(GET_AUDSTYLE,0L,0L,0L,0L,0L))>=0 && (j&AUD_LOGIN_UNAME))
		{
			/* only log attempted bad username if AUD_LOGIN_UNAME, since */
			/* it could be a the user's password entered out of order */
			audtok[i] = T_LOGIN;
			audval[i] = entity->name;
			i++;
			entity->error = ENOENT;
		}
		else
		{
			/* Must put in something here so that remote ID is */
			/* recognized as being remote. */

			audtok[i] = T_LOGIN;
			audval[i] = "(unknown)";
			i++;
			entity->error = ENOENT;
		}
		if (entity->pwd && entity->pwd->pw_dir && *entity->pwd->pw_dir)
		{
			audtok[i] = T_HOMEDIR;
			audval[i] = entity->pwd->pw_dir;
			i++;
		}
		if (entity->pwd && entity->pwd->pw_shell && *entity->pwd->pw_shell)
		{
			audtok[i] = T_SHELL;
			audval[i] = entity->pwd->pw_shell;
			i++;
		}
		if (entity->tty && *entity->tty) {
			audtok[i] = T_DEVNAME;
			audval[i] = entity->tty;
			i++;
		}
		audtok[i] = T_CHARP;
		if ((entity->pwd && entity->pwd->pw_name)
		    || entity->error == EACCES)
			audval[i] = "Failed authentication";
		else
			audval[i] = "Invalid account";
		i++;
		if (! entity->error)
			entity->error = EACCES;
		audtok[i] = T_ERRNO;
		audval[i] = (char *) (entity->error);
		i++;
		if (entity->hostname)
		{
			audtok[i] = T_HOSTNAME;
			audval[i] = entity->hostname;
			if ((cp = strchr(entity->hostname, '@')) != NULL)
			{
				(void) strncpy(remuser, entity->hostname,
						cp - entity->hostname);
				remuser[cp - entity->hostname] = '\0';
				(void) strcpy(remhost, ++cp);
				audval[i] = remhost;
				audtok[++i] = T_LOGIN;
				audval[i] = remuser;
			}
			else if ((cp = strstr(entity->hostname, "::")) != NULL
				 && cp > entity->hostname)
			{
				cp += 2;
				(void) strncpy(remhost, entity->hostname,
						cp - entity->hostname);
				remhost[cp - entity->hostname] = '\0';
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

		SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_SESAUTHBAD,"Failure to authenticate session for %s on %s"),entity->name,entity->tty);
		sia_free_entity(entity,SIAAUTHFREE);
		SIATHREADREL(SIA_AUTHENT_LOCK)
		if(entity->password != NULL)
			bzero(entity->password, strlen(entity->password));
		if(siaderr&SIADSTOP)
			return(SIAFAIL|SIADSTOP);
		return(SIAFAIL);
		}
}
