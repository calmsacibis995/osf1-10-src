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
static char *rcsid = "@(#)$RCSfile: sia_mk_en_pw.c,v $ $Revision: 1.1.13.2 $ (DEC) $Date: 1993/06/08 02:07:19 $";
#endif
/*****************************************************************************
* Usage:  struct passwd *sia_make_entity_pwd(struct passwd *pwd, SIAENTITY *entity)
*
* Description: The purpose of this routine is to allocate space and copy
* passwd struct fields into the entity structure.
*
*
* Parameter Descriptions:  SIAENTITY *entity
*
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_make_entity_pwd = __sia_make_entity_pwd
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int 
  sia_make_entity_pwd (struct passwd *pwd,  SIAENTITY *entity)
{
	SIATHREADLOCK(SIA_ENTITY_LOCK) /* single thread within process */
	if(entity->pwd != NULL)	       /* assume reuse so wipe it out  */
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
		}
	else	{
		entity->pwd= (struct passwd *) malloc(sizeof (struct passwd));
		if(entity->pwd == NULL)
			return(SIAFAIL);
		}
	bzero(entity->pwd, sizeof(struct passwd));
	if (entity->acctname)
		{
		bzero(entity->acctname, strlen(entity->acctname));
		free(entity->acctname);
		}
	if(pwd->pw_name != NULL)
		{
		entity->pwd->pw_name= (char *) malloc(strlen(pwd->pw_name) + 1);
		if(entity->pwd->pw_name == NULL)
			return(SIAFAIL);
		entity->acctname= (char *) malloc(strlen(pwd->pw_name) + 1);
		if(entity->acctname == NULL)
		        return(SIAFAIL);
		strcpy(entity->pwd->pw_name,pwd->pw_name);
		strcpy(entity->acctname,pwd->pw_name);
		}
	if(pwd->pw_passwd != NULL)
		{
		entity->pwd->pw_passwd= (char *) malloc(strlen(pwd->pw_passwd) + 1);
		if(entity->pwd->pw_passwd == NULL)
			return(SIAFAIL);
		strcpy(entity->pwd->pw_passwd,pwd->pw_passwd);
		}
	if(pwd->pw_comment != NULL)
		{
		entity->pwd->pw_comment= (char *) malloc(strlen(pwd->pw_comment) + 1);
		if(entity->pwd->pw_comment == NULL)
			return(SIAFAIL);
		strcpy(entity->pwd->pw_comment,pwd->pw_comment);
		}
	if(pwd->pw_gecos != NULL)
		{
		entity->pwd->pw_gecos= (char *) malloc(strlen(pwd->pw_gecos) + 1);
		if(entity->pwd->pw_gecos == NULL)
			return(SIAFAIL);
		strcpy(entity->pwd->pw_gecos,pwd->pw_gecos);
		}
	if(pwd->pw_dir != NULL)
		{
		entity->pwd->pw_dir= (char *) malloc(strlen(pwd->pw_dir) + 1);
		if(entity->pwd->pw_dir == NULL)
			return(SIAFAIL);
		strcpy(entity->pwd->pw_dir,pwd->pw_dir);
		}
	 if(pwd->pw_shell != NULL)
                {
		entity->pwd->pw_shell= (char *) malloc(strlen(pwd->pw_shell) + 1);
		if(entity->pwd->pw_shell == NULL)
			return(SIAFAIL);
		strcpy(entity->pwd->pw_shell,pwd->pw_shell);
		}
	entity->pwd->pw_uid=pwd->pw_uid;
	entity->pwd->pw_gid=pwd->pw_gid;
	entity->pwd->pw_quota=pwd->pw_quota;
	SIATHREADREL(SIA_ENTITY_LOCK)
	return(SIASUCCESS);
}
