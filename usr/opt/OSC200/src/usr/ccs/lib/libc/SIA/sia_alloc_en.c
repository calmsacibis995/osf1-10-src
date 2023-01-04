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
static char *rcsid = "@(#)$RCSfile: sia_alloc_en.c,v $ $Revision: 1.1.11.4 $ (DEC) $Date: 1993/08/04 21:19:33 $";
#endif
/*****************************************************************************
* Usage:  int	sia_alloc_entity(SIAENTITY *entity)
*
* Description: The purpose of this routine is to allocate space for an
* entity structure. The space will initialized to zero.
*
*
* Parameter Descriptions:  SIAENTITY *entity
*			   on input this should be a null pointer
*
* Success return: SIASUCCESS or SIAFAIL 
*
* No specific errors
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_alloc_entity = __sia_alloc_entity
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int
sia_alloc_entity (SIAENTITY *(*entity))
{
	if (!entity)
		return SIAFAIL;
	SIATHREADLOCK(SIA_ENTITY_LOCK) /* single thread within process */
	*entity= (SIAENTITY *)malloc(sizeof(SIAENTITY));
	if(*entity==NULL)
		{
		SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_ENTALLOCBAD,"Failure to allocate entity"));
		SIATHREADREL(SIA_ENTITY_LOCK)
		return(SIAFAIL);
		}
	bzero(*entity,sizeof( SIAENTITY ));
	SIATHREADREL(SIA_ENTITY_LOCK)
	return(SIASUCCESS);
}






