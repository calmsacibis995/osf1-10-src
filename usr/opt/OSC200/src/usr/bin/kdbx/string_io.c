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
static char *rcsid = "@(#)$RCSfile: string_io.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 12:13:49 $";
#endif
#include "krash_p.h"

static IORec string_in_rec = { NULL, False, False, 
				 { NoOutput, write_null, { NULL } },
				 { ImmediateInput, read_string_block,
				     read_string_line, False } };

void set_string_in(IORec *io, char *buf)
{
  io->in_part = string_in_rec.in_part;
  if(buf) io->in_part.u.immediate.data = copy(buf, -1);
  else io->in_part.u.immediate.data = NULL;
  io->in_part.u.immediate.ptr = io->in_part.u.immediate.data;
  io->in_part.buf_size = HUGE_BUF;
}
#ifdef notdef
void set_string_out(io, buf)
IORec *io;
char *buf;
{
  io->out_part = string_in_rec.out_part;
  if(buf) io->in_part.u.immediate.data = copy(buf, -1);
  else io->in_part.u.immediate.data = NULL;
  io->in_part.u.immediate.ptr = io->in_part.u.immediate.data;
}
#endif
Boolean read_string_block(IORec *io, char *ret, int size)
{
  int len;

  if(size == 0){
    if(io->in_part.u.immediate.data) free(io->in_part.u.immediate.data);
    return(False);
  }
  if(io->in_part.u.immediate.data == NULL){
    io->in_part.type = NoInput;
    *ret = '\0';
    return(False);
  }
  len = strlen(io->in_part.u.immediate.ptr);
  if(len == 0){
    free(io->in_part.u.immediate.data);
    set_null_in(io);
    *ret = '\0';
    return(False);
  }
  else if(len > size){
    strncpy(ret, io->in_part.u.immediate.ptr, size);
    io->in_part.u.immediate.ptr += size;
    ret[size] = '\0';
    return(True);
  }
  else {
    strcpy(ret, io->in_part.u.immediate.ptr);
    free(io->in_part.u.immediate.data);
    io->in_part.u.immediate.data = NULL;
    io->in_part.u.immediate.ptr = NULL;
    return(True);
  }
}

Boolean read_string_line(IORec *io, char **ret)
{
  int len;
  char *data, *ptr;

  data = io->in_part.u.immediate.ptr;
  if(data) len = strlen(data);
  else len = 0;
  if(len == 0) return(False);
  if((ptr = index(data, '\n')) != NULL){
    *ret = copy(data, ptr - data);
    io->in_part.u.immediate.ptr = ptr + 1;
    return(True);
  }
  else {
    *ret = copy(data, -1);
    io->in_part.u.immediate.ptr = STRING_END(data);
    free(io->in_part.u.immediate.data);
    io->in_part.u.immediate.data = NULL;
    return(False);
  }
}

Boolean append_string(IORec *io, char *str)
{
  char *new;
  int len;

  if(str != NULL){
    if(io->in_part.u.immediate.data == NULL){
      NEW_TYPE(new, strlen(str) + 1, char, char *, "append_string");
      *new = '\0';
    }    
    else {
      len = strlen(io->in_part.u.immediate.data);
      if((new = realloc(io->in_part.u.immediate.data, len + strlen(str) + 1))
	 == NULL){
	perror("append_string");
	return(False);
      }
    }
    strcat(new, str);
    io->in_part.u.immediate.data = new;
    io->in_part.u.immediate.ptr = new;
  }
  return(True);
}
