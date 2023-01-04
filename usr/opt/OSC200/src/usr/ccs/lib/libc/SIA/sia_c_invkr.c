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
static char *rcsid = "@(#)$RCSfile: sia_c_invkr.c,v $ $Revision: 1.1.11.4 $ (DEC) $Date: 1993/08/04 21:19:46 $";
#endif
/*****************************************************************************
* Usage:  int *sia_chk_invoker()
*
* Description: The purpose of this routine is to check if the calling program
* has sufficient privilage to do sia functions, ussualy uid==0. 
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
* Assumed Inputs: sia_matrix is properly configured with installed security
*			mechanisms.
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
#pragma weak sia_chk_invoker = __sia_chk_invoker
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int 
  sia_chk_invoker (void)
{
	
/*****************************************************************************************/
#ifndef SV_SHLIBS
        ldr_module_t    lib_handle = NULL;
        int             (*fp) ();
#else
        void *lib_handle;
        void *fp;
#endif
        int siaderr=(SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=0;                  /* sia status between mechanism calls */
        int pkgind=0;                   /* package index */
/*****************************************************************************************/
	SIATHREADLOCK(SIA_AUTHENT_LOCK)
	if(sia_init() == SIAFAIL)
		{
		SIATHREADREL(SIA_AUTHENT_LOCK)
		return(SIAFAIL);
		}
        while(pkgind < SIASWMAX && sia_matrix[SIA_CHK_INVOKER][pkgind].pkgnam)
                {
		if(sia_matrix[SIA_CHK_INVOKER][pkgind].fp == NULL)
			{
			if (sia_setupfp(SIA_CHK_INVOKER, pkgind) == SIAFAIL)
				{
				SIATHREADREL(SIA_AUTHENT_LOCK);
				return SIAFAIL;
				}
			}
		if((*sia_matrix[SIA_CHK_INVOKER][pkgind].fp)())
				siaderr = SIADSUCCESS;
		else   {
			SIATHREADREL(SIA_AUTHENT_LOCK)
			return(SIAFAIL);
			}
		pkgind++;
		}
	if(siaderr==SIADSUCCESS)
		{
		SIATHREADREL(SIA_AUTHENT_LOCK)
		return(SIASUCCESS);
		}
	else	{
		SIATHREADREL(SIA_AUTHENT_LOCK)
		return(SIAFAIL);
		}
}
