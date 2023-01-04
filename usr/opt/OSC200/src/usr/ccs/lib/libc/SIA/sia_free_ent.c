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
static char *rcsid = "@(#)$RCSfile: sia_free_ent.c,v $ $Revision: 1.1.11.3 $ (DEC) $Date: 1993/06/08 01:19:40 $";
#endif
/*****************************************************************************
* Usage:  int sia_free_entity(SIAENTITY *entity)
*
* Description: The purpose of this routine is to zero and free memory 
* associated with an SIAENTITY structure. This is typically done as 
* part of cleaning up after either success or failure of a SIA routine.
*
*	NOTE: If mechanism pointers are non_NULL a free call is done 
*	to free the associated memory. The entire entity structure is then
*	zeroed out.
*
*
* Parameter Descriptions:  SIAENTITY *entity
*
*
* Success return: SIASUCCESS or SIAFAIL 
*
* No specific errors
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_free_entity = __sia_free_entity
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int
  sia_free_entity (SIAENTITY *entity, int reauth)
{
	int i;

	SIATHREADLOCK(SIA_ENTITY_LOCK) /* single thread within process */
	
	if(entity==NULL)
                {
                SIATHREADREL(SIA_ENTITY_LOCK)
                return(SIAFAIL);
                }
	if(entity->password)
		{
		bzero(entity->password,strlen(entity->password));
		free(entity->password);
		entity->password = NULL;
		}
	if(entity->pwd)
		{
		if(entity->pwd->pw_name)
			{
			bzero(entity->pwd->pw_name,strlen(entity->pwd->pw_name));
			free(entity->pwd->pw_name);
			}
		if(entity->pwd->pw_passwd)
			{
			bzero(entity->pwd->pw_passwd,strlen(entity->pwd->pw_passwd));
			free(entity->pwd->pw_passwd);
			}
		if(entity->pwd->pw_comment)
			{
			bzero(entity->pwd->pw_comment,strlen(entity->pwd->pw_comment));
			free(entity->pwd->pw_comment);
			}
		if(entity->pwd->pw_gecos)
			{
			bzero(entity->pwd->pw_gecos,strlen(entity->pwd->pw_gecos));
			free(entity->pwd->pw_gecos);
			}
		if(entity->pwd->pw_dir)
			{
			bzero(entity->pwd->pw_dir,strlen(entity->pwd->pw_dir));
			free(entity->pwd->pw_dir);
			}
		if(entity->pwd->pw_shell)
			{
			bzero(entity->pwd->pw_shell,strlen(entity->pwd->pw_shell));
			free(entity->pwd->pw_shell);
			}
		bzero(entity->pwd,sizeof(struct passwd));
		free(entity->pwd);
		entity->pwd = NULL;
		}
        if(entity->acctname != NULL)
		{
                free(entity->acctname);
		entity->acctname=NULL;
		}
	if(reauth)
		{
		if(entity->colinput && (entity->name != NULL))
			{
			free(entity->name);
			entity->name=NULL;
			}
		entity->authcount++;
		SIATHREADREL(SIA_ENTITY_LOCK)
		return(SIASUCCESS);
		}
	if(entity->argc > 0)
		for(i=0; i < entity->argc; i++)
			{
			free(entity->argv[i]);
			}
	if(entity->name != NULL)
		{
                free(entity->name);
		entity->name=NULL;
		}
	if(entity->argv != NULL)
		{
		free(entity->argv);
		entity->argv=NULL;
		}
	if(entity->hostname != NULL)
		{
		free(entity->hostname);
		entity->hostname=NULL;
		}
	if(entity->tty != NULL)
		{
		free(entity->tty);
		entity->tty=NULL;
		}
	bzero(entity,sizeof( SIAENTITY ));
	free(entity);
	entity=NULL;
	SIATHREADREL(SIA_ENTITY_LOCK)
	return(SIASUCCESS);
}













