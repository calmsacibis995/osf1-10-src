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
static char *rcsid = "@(#)$RCSfile: siad_s_reaut.c,v $ $Revision: 1.1.13.4 $ (DEC) $Date: 1993/08/04 21:22:52 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_reauthent((*sia_collect),
*				SIAENTITY *entity,
*				char *password,
*				int siastat,
*				int pkgind)
*
* Description: The purpose of this routine is to do session reauthentication.
* If the collect function pointer is NULL this is a non-interactive request.
* This call is used to perform a terminal lock or workstation pause function.
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
#pragma weak siad_ses_reauthent = __siad_ses_reauthent
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

#undef	MSGSTR
#define	MSGSTR(n,s)	GETMSGSTR(MS_BSD,(n),(s))

#define	MAX_PWD_LENGTH	16
#define	MAX_LOGIN_LENGTH	8

int
  siad_ses_reauthent (int (*collect)(), SIAENTITY *entity, int siastat,
		      int pkgind)
{
	prompt_t prompt;
	unsigned char pass[SIAMXPASSWORD+1];
	int i;
	struct passwd *pwd;
	struct group *grp;
/*
 * Return immediately if already successful.
 */
	if(entity == NULL)
                return SIADFAIL;

	if(siastat==SIADSUCCESS)
		return SIADSUCCESS;

	if(!entity->pwd) {		/*DAL002*/
		 if(!entity->name ||
		    siad_getpwnam(entity->name, &siad_bsd_passwd, siad_bsd_getpwbuf,SIABUFSIZ) == SIADFAIL)
			return SIADFAIL;
		else
			if(sia_make_entity_pwd(&siad_bsd_passwd, entity) == SIAFAIL)
				return SIADFAIL;
	}
	pwd = entity->pwd;
	if(pwd == NULL)
		 return SIADFAIL;
/*
 * Authenticate.
 */
	if(entity->password != NULL)	/* password collected yet? */
		{
		if(strlen(entity->password) > MAX_PWD_LENGTH)
			 return SIADFAIL;
		}
/*
 * No password supplied, prompt interactively.
 */		
	else	{
		if(collect == NULL)
			return SIADFAIL;
		entity->password = (char *) malloc(SIAMXPASSWORD+1);
		if(entity->password == NULL)
                        return SIADFAIL;
		bzero(entity->password, SIAMXPASSWORD+1);
		if(pwd->pw_passwd && *pwd->pw_passwd) 
			{
			prompt.prompt = (unsigned char *) MSGSTR(ENTRY_PWD, "Password:");
			prompt.result = (unsigned char *) entity->password;
			prompt.min_result_length = 0;
			prompt.max_result_length = MAX_PWD_LENGTH;
			prompt.control_flags = SIARESINVIS;
			if ((*collect)(0, SIAONELINER, "", 1, &prompt) != SIACOLSUCCESS)
				return SIADFAIL;
			}
		}
/*
 * No password required, make sure supplied password is empty.
 */
	if((*entity->password == NULL) && (*pwd->pw_passwd == NULL))
		{
		if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
			return SIADFAIL;
		return SIADSUCCESS;
		}
/*
 * Password required and non-empty password supplied. Check for correct password.
 */
        if(entity->password && pwd->pw_passwd && strlen(pwd->pw_passwd) >= 2 && /*DAL001*/
            !strcmp(pwd->pw_passwd, crypt(entity->password, pwd->pw_passwd)))
		{
		if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
                        return SIADFAIL;
		return SIADSUCCESS;
		}
	else
		return SIADFAIL;
return SIADFAIL;
}
