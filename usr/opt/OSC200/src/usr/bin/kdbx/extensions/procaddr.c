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
static char *rcsid = "@(#)$RCSfile: procaddr.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:53:08 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"procaddr - convert an address to a procedure name                        \\\n\
    Usage : procaddr [address...]                                         \\\n\
";

static void Usage(){
  fprintf(stderr, "Usage : procaddr [address...]\n");
  quit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  long addr, naddr;
  char *ptr, *proc, buf[256], *error;

  check_args(argc, argv, help_string);
  argv++;
  while(*argv){
    addr = strtoul(*argv, &ptr, 16);
    if(*ptr != '\0'){
      if(!read_sym_val(*argv, NUMBER, &addr, &error)){
	fprintf(stderr, "Couldn't read %s:\n", *argv);
	fprintf(stderr, "%s\n", error);
	Usage();
      }
    }
    proc = addr_to_proc(addr);
    naddr = strtoul(proc, &ptr, 16);
    if(*ptr != '\0') print(proc);
    else {
      sprintf(buf, "0x%p not a proc", addr);
      print(buf);
    }
    free(proc);
    argv++;
  }
  quit(0);
}
