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
static char	*sccsid = "@(#)$RCSfile: extend.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:58:46 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/* Extend table pointed to by tb.  Returns NULL if unsuccessful. */

#ifdef NLS
#include <standards.h>
#endif
#ifndef NULL
#define NULL	((char *)0)
#endif
#include <sys/types.h>
#include "table.h"
char *realloc(), *malloc();

char *
extend(tb)
register struct table *tb;
{       register unsigned sz = (unsigned)tb->tb_nel * tb->tb_elsize;
	register char *space = *tb->tb_base;

	extern void free();
	/* Check for overflow */
	if ((unsigned) tb->tb_nel != sz/tb->tb_elsize)
		space = NULL;
	else if (space) {
		free(space);
		space = realloc(space,sz);
	} else
		space = malloc(sz);
	return(*tb->tb_base = space);
}

