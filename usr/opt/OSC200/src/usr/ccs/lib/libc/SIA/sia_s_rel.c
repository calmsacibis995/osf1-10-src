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
static char *rcsid = "@(#)$RCSfile: sia_s_rel.c,v $ $Revision: 1.1.12.4 $ (DEC) $Date: 1993/08/04 21:21:35 $";
#endif
/*****************************************************************************
* Usage:  int *sia_ses_release( SIAENTITY *sia_entity_handle )
*
* Description: The purpose of this routine is to deallocate the entity 
* structure. This includes calls to the security mechanisms to deallocate
* their respective mechanism areas.
*
*	Internal return codes: 	
*	(SUCCESS | CONTINUE) - used to allow other authentication mechanisms
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
* Parameter Descriptions:  none
*
*
* Success return: SIASUCCESS .
*
* Error Conditions and returns otherthan parameter syntax checks listed above:
*
* 	Condition1: loader failure to load libsia or a package within libsia
*	Return: SIALDRERR
*
* 	Condition2: failure to properly authenticate
*	Return: SIAFAIL
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_ses_release = __sia_ses_release
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int 
  sia_ses_release (SIAENTITY *(*entityhdl))
{
	
/*****************************************************************************************/
        int siaderr=(SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=0;                  /* sia status between mechanism calls */
        int pkgind=0;                   /* package index */
/*****************************************************************************************/
	SIATHREADLOCK(SIA_SES_LOCK)
	if(sia_init() == SIAFAIL)	/* check if sia has been initialized */
		{
		SIATHREADREL(SIA_SES_LOCK)
		return(SIAFAIL);
		}
        if(entityhdl == NULL || *entityhdl == NULL) /* Has entity already been freed */
                {                       
		return(SIASUCCESS);
		}
        while(pkgind < SIASWMAX && sia_matrix[SIA_SES_RELEASE][pkgind].pkgnam != NULL)
                {
		if(sia_matrix[SIA_SES_RELEASE][pkgind].fp == NULL)
			{
			if (sia_setupfp(SIA_SES_RELEASE, pkgind) == SIAFAIL)
				{
				SIATHREADREL(SIA_SES_LOCK);
				return SIAFAIL;
				}
			}
		siaderr = (*sia_matrix[SIA_SES_RELEASE][pkgind].fp)(*entityhdl,pkgind);
		if(siaderr == SIADFAIL)
			{
			SIALOG(MSGSTR(SIA_MSG_LOGERROR, "ERROR"),MSGSTR(SIA_MSG_BADRELEASE,"Failure to release  session\n"));
			SIATHREADREL(SIA_SES_LOCK)
			return(SIAFAIL);
			}
		else	pkgind++;
		}
	if(sia_free_entity(*entityhdl,SIAFREEALL) == SIAFAIL)
		{
		SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_BADRELEASE, "Failure to release session\n"));
        	SIATHREADREL(SIA_SES_LOCK)
        	return(SIAFAIL);
		}
	else	{
		SIATHREADREL(SIA_SES_LOCK)
		*entityhdl=NULL;
		return(SIASUCCESS);
		}
}
