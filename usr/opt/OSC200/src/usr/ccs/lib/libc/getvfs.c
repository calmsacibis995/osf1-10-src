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
static char *rcsid = "@(#)$RCSfile: getvfs.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 23:06:40 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak getvfsbyname = __getvfsbyname
#pragma weak getvfsbynumber = __getvfsbynumber
#endif
#endif
#include <sys/mount.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

/* Keep mnt_names out of the user's namespace. */    
#define mnt_names __mnt_names
#include <sys/fs_types.h>

int
getvfsbyname( const char * aname )
{
  char *name;
  char *p;
  int	i;

  if (!aname) {
      _Seterrno(EFAULT);
      return (-1);
  }

  name = strdup((char *)aname);

  if (!name) {
      _Seterrno(ENOMEM);
      return (-1);
  }

  p=name;

  while (*p) {*p = tolower(*p); p++;}	/* downcase it */

  /*
   * Check that mnt_names[i] isn't NULL just in case
   * someone adds a new filesystem type, but forgets
   * to update mnt_names.  It happens.
   */
  for(i=MOUNT_NONE+1; i<=MOUNT_MAXTYPE && mnt_names[i]; i++)
    if (!strcmp(name,mnt_names[i])) {
      free(name);
      return (i);
    }

  free(name);
  return (MOUNT_NONE);
}

char *
getvfsbynumber( int fsid )
{
  if ( (fsid < MOUNT_NONE) || (fsid > MOUNT_MAXTYPE)) {
      _Seterrno(EINVAL);
      return (char *)-1;
  }

  return ((char *)mnt_names[fsid]);
}

