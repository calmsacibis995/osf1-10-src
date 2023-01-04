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
static char *rcsid = "@(#)$RCSfile: sia_auth.c,v $ $Revision: 1.1.11.3 $ (DEC) $Date: 1993/06/08 01:19:33 $";
#endif
/*****************************************************************************
* Usage:  int * sia_authorize(
*				char *entity)
*
* Description: The purpose of this routine is to implement a general purpose
* authorization capability. This routine will check some information base,
* sia_authordb, to establish whether this particular command should allow
* this particular entity access from this particular host. If the access is
* local the hostname pointer will be set to NULL. This routine is called by
* all of the authentication routines before returning SIASUCCESS. No call is
* made to this routine if SIAFAIL has already been determined. 
*
* Parameter Descriptions: 
*
*	Param1: cmnd_name 
*       Usage: calling command name like: "telnet"
*	Syntax: ascii string if cmnd_name is NULL return SIAFAIL
*
*	Param2: hostname
*       Usage: hostname of source system requesting access.
*	Syntax: if hostname is NULL then assume local host
*
*	Param3: entity
*       Usage: entity->name must be preset with the loginname.
*	Syntax: if entity->name is NULL SIAFAIL should be returned.
*
*
* Assumed Inputs: sia_authordb must be presetup. 
*
* Success return: SIASUCCESS if check is successful or sia_authordb does not
*		exist or if nothing is found in it 
*
* Error Conditions: sia_authordb does not exist
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_authorize = __sia_authorize
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int
  sia_authorize(SIAENTITY *entity)
{
	/* ASSUME SIA initialization is done by calling sia_*_authent routine*/
	/* ASSUME already thread locked via SIA_AUTHENT_LOCK mutex */

	return(SIASUCCESS); /* for now we like most everyone */
}
