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
static char *rcsid = "@(#)$RCSfile: priocntl.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/07/20 22:24:18 $";
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define priocntl __priocntl
#pragma weak priocntl = __priocntl
#endif

#include <stdio.h>
#include <sys/procset.h>

long
priocntl(idtype_t idtype, id_t id, int cmd, void *argp)
{
	procset_t procset;

	setprocset(&procset, POP_AND, idtype, id, P_ALL, 0);
	return(priocntlset(&procset, cmd, argp));
}
