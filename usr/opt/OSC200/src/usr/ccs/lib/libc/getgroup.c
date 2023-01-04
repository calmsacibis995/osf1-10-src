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
static char *rcsid = "@(#)$RCSfile: getgroup.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/06/07 22:58:23 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak endgrent = __endgrent
#pragma weak getgrent = __getgrent
#pragma weak getgrgid = __getgrgid
#pragma weak getgrnam = __getgrnam
#pragma weak setgrent = __setgrent
#endif
#endif
#include "ts_supp.h"
#include "SIA/siad.h"

int
  setgrent (void)
{
  union sia_get_params params;

  if (sia_getgroup (G_SET, NON_REENTRANT, &params) == SIAFAIL)
	return TS_FAILURE;
    else
	return TS_SUCCESS;
}

void
  endgrent (void)
{
  union sia_get_params params;

  sia_getgroup (G_END, NON_REENTRANT, &params);
}

struct group *
  getgrent (void)
{
  union sia_get_params params;

  if (sia_getgroup (G_ENT, NON_REENTRANT, &params) == SIAFAIL)
    return ((struct group *) NULL);

  return (params.group.result);
}

struct group *
  getgrnam (char *name)
{
  union sia_get_params params;
  
  params.group.name = name;
  if (sia_getgroup (G_NAM, NON_REENTRANT, &params) == SIAFAIL)
    return ((struct group *) NULL);

  return (params.group.result);
}

struct group *
  getgrgid (gid_t gid)
{
  union sia_get_params params;
  
  params.group.gid = gid;
  if (sia_getgroup (G_GID, NON_REENTRANT, &params) == SIAFAIL)
    return ((struct group *) NULL);

  return (params.group.result);
}
	
