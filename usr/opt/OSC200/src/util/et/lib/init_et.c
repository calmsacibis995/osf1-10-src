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
/*
 * $Header: /usr/sde/alpha/rcs/alpha/src/./util/et/lib/init_et.c,v 1.1.4.2 1992/11/19 11:44:12 Aaron_Sawyer Exp $
 * $Source: /usr/sde/alpha/rcs/alpha/src/./util/et/lib/init_et.c,v $
 * $Locker:  $
 *
 * Copyright 1986, 1987, 1988 by MIT Information Systems and
 *	the MIT Student Information Processing Board.
 *
 * For copyright info, see mit-sipb-copyright.h.
 */

#include <stdio.h>
#include "error_table.h"
#include "mit-sipb-copyright.h"

#ifndef __STDC__
#define const
#endif

#ifndef	lint
static const char rcsid_init_et_c[] =
    "$Header: /usr/sde/alpha/rcs/alpha/src/./util/et/lib/init_et.c,v 1.1.4.2 1992/11/19 11:44:12 Aaron_Sawyer Exp $";
#endif

extern char *malloc(), *realloc();

struct foobar {
    struct et_list etl;
    struct error_table et;
};

extern struct et_list * _et_list;

int init_error_table(msgs, base, count)
    const char * const * msgs;
    int base;
    int count;
{
    struct foobar * new_et;

    if (!base || !count || !msgs)
	return 0;

    new_et = (struct foobar *) malloc(sizeof(struct foobar));
    if (!new_et)
	return errno;	/* oops */
    new_et->etl.table = &new_et->et;
    new_et->et.msgs = msgs;
    new_et->et.base = base;
    new_et->et.n_msgs= count;

    new_et->etl.next = _et_list;
    _et_list = &new_et->etl;
    return 0;
}
