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
static char *rcsid = "@(#)$RCSfile: sia_s_suauth.c,v $ $Revision: 1.1.12.7 $ (DEC) $Date: 1993/12/21 21:52:25 $";
#endif
/*****************************************************************************
* Usage:  int * sia_ses_suauthent((*sia_collect)
*                               SIAENTITY *entityhdl )
*
* Description: The purpose of this routine is to provide authentication for
* the "su" command. The su command can allow a user to become a different
* authenticatible entity. Since certain security mechanisms may exercise 
* alternative procedures for this authentication process this has become
* a separate call. The intention is that su will first call this routine
* to properly authenticate and then call the establish session routine
* to complete the formation of the new entity's passport.
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
*	Param2: suname
*       Usage: name of user being sued to
*	Syntax: char string can be null if root is implied
*
*	Param3: entityhdl
*	Usage: upon successful return this pointer can be used in subsequent sia calls
*		like sia_ses_estab and sia_ses_launch.
*	Syntax: on inpu this should be a NULL pointer
*
* Assumed Inputs: None
*
* Success return: SIASUCCESS accompanied with a non_NULL passport handle.
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
#pragma weak sia_ses_suauthent = __sia_ses_suauthent
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int
  sia_ses_suauthent (int (*collect)(), SIAENTITY *entity)
{
        
/*****************************************************************************************/
        int siaderr=(SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=SIADFAIL;                  /* sia status between mechanism calls */
        int pkgind=0;                   /* package index */
	struct passwd *supass;
	char	*suname=NULL;
        char    acctname[SIAMXACCTNAME];
	char	fromname[SIAMXACCTNAME];

	SIATHREADLOCK(SIA_AUTHENT_LOCK)
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
        bzero(acctname,sizeof(acctname));
	bzero(fromname,sizeof(fromname));
	supass=getpwuid(getuid());
	if(supass == NULL)
		{
                SIATHREADREL(SIA_AUTHENT_LOCK)
                return(SIAFAIL);
                }
        else    {
                suname=fromname;
                strcpy(suname,supass->pw_name);
                }
	entity->authtype = SIA_A_SUAUTH;
        while(pkgind < SIASWMAX && sia_matrix[SIA_SES_SUAUTHENT][pkgind].pkgnam != NULL)
                {
                if(sia_matrix[SIA_SES_SUAUTHENT][pkgind].fp == NULL)
                        {
			if (sia_setupfp(SIA_SES_SUAUTHENT, pkgind) == SIAFAIL)
				{
				SIATHREADREL(SIA_AUTHENT_LOCK);
				return SIAFAIL;
				}
			}
		siaderr = (*sia_matrix[SIA_SES_SUAUTHENT][pkgind].fp)((collect),entity,siastat,pkgind);
                if(siaderr & SIADSUCCESS)
                        {
                        siastat=SIADSUCCESS;
                        if( acctname[0] == NULL)
                                strcpy(acctname,entity->acctname);
                        else if(strcmp(acctname,entity->acctname))
                                {
                                /*** passports will need much more ***/
				sia_free_entity(entity,SIAAUTHFREE);
                                SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_ENTINCON, "SIA configuration entity inconsistency\n"));
                                SIATHREADREL(SIA_AUTHENT_LOCK)
                                return(SIAFAIL);
                                }
                        }
		if (siaderr & SIADSTOP)
			break;
                pkgind++;
                }		/* override of last configured security mechanism */
	if (acctname[0] == '\0') {
		if (entity->acctname && entity->acctname[0])
			(void) strcpy(acctname, entity->acctname);
		else if (entity->name && entity->name[0])
			(void) strcpy(acctname, entity->name);
		else
			(void) strcpy(acctname, "?");
	}
	if((siastat & SIADSUCCESS) && (siaderr & SIADSUCCESS))
		{
		(void) sia_audit(AUTH_EVENT, T_CHARP,
				MSGSTR(SIA_MSG_SUTRY,"su"),
				T_LOGIN, suname,
				T_LOGIN, acctname,
				T_RESULT, 0, 0);
		if (entity->pwd->pw_uid == 0)
			{
			syslog(LOG_NOTICE|LOG_AUTH,
				GETMSGSTR(MS_BSDSU, SU_ON, "SU %s on %s"),
				suname, entity->tty ? entity->tty : "?");
			}
                SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_SUAUTHGOOD,"Successful authentication for su from %s to %s"),suname,entity->pwd->pw_name);
                SIATHREADREL(SIA_AUTHENT_LOCK)
                if(entity->password != NULL)
                       bzero(entity->password,strlen(entity->password));
                return(SIASUCCESS);
                }
        else	{
                /*** passports will need more ***/
		(void) sia_audit(AUTH_EVENT, T_CHARP,
				MSGSTR(SIA_MSG_SUTRY,"su"),
				T_LOGIN, suname,
				T_LOGIN, acctname,
				T_ERRNO, EACCES,
				T_RESULT, -1, 0);
		if (!entity->pwd || entity->pwd == 0)
			{
			syslog(LOG_WARNING|LOG_AUTH,
				GETMSGSTR(MS_BSDSU, BAD_SU2, "BADSU %s on %s"),
				suname, entity->tty ? entity->tty : "?");
			}
                SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_SUAUTHBAD,"Failure on authentication for su from %s to %s"),suname,entity->name);
		sia_free_entity(entity,SIAAUTHFREE);
                SIATHREADREL(SIA_AUTHENT_LOCK)
                return(SIAFAIL);
                }
}
