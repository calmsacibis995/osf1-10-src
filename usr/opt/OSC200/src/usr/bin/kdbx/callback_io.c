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
static char *rcsid = "@(#)$RCSfile: callback_io.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:55:18 $";
#endif
#include "krash_p.h"

static IORec callback_rec = { NULL, False, False,
				{ NoOutput, NULL, { NULL } },
				{ NoInput, NULL, NULL, False } };

void set_cb_out(IORec *io, IOOutType type, Boolean (*proc)(struct _IORec *,
							   char *), void *arg)
{
  io->out_part = callback_rec.out_part;
  io->out_part.type = type;
  io->out_part.proc = proc;
  io->out_part.u.arg = arg;
  io->out_part.buf_size = HUGE_BUF;
}

void set_cb_in(IORec *io, IOInType type,
	       Boolean (*block_proc)(struct _IORec *, char *, int),
	       Boolean (*line_proc)(struct _IORec *, char **),
	       Boolean interpret, void *arg)
{
  io->in_part = callback_rec.in_part;
  io->in_part.type = type;
  io->in_part.block_proc = block_proc;
  io->in_part.line_proc = line_proc;
  io->in_part.interpret = interpret;
  io->in_part.u.arg = arg;
  io->in_part.buf_size = HUGE_BUF;
}

Boolean write_string(IORec *io, char *str)
{
  int len;
  char **ptr;

  if(str != NULL){
    len = strlen(str);
    ptr = (char **) io->out_part.u.arg;
    if(*ptr == NULL) *ptr = copy(str, -1);
    else {
      *ptr = realloc(*ptr, len + strlen(*ptr) + 1);
      strcat(*ptr, str);
    }
  }
  return(True);
}

Boolean write_nothing(IORec *io, char *str)
{
  return(True);
}
