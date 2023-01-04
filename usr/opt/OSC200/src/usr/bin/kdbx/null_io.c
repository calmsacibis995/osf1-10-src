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
static char *rcsid = "@(#)$RCSfile: null_io.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 16:22:32 $";
#endif
#include "krash_p.h"

static IORec null_rec = { NULL, False, False, 
			    { NoOutput, write_null, { NULL } },
			    { NoInput, read_null_block, read_null_line,
				False } };

void set_null_in(IORec *io)
{
  io->in_part = null_rec.in_part;
}

void set_null_out(IORec *io)
{
  io->out_part = null_rec.out_part;
}

Boolean read_null_block(IORec *io, char *ret, int size)
{
  return(False);
}

Boolean read_null_line(IORec *io, char **ret)
{
  return(False);
}

Boolean write_null(IORec *io, char *str)
{
  return(True);
}
