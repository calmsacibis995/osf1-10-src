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
static char *rcsid = "@(#)$RCSfile: vmmap.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:56:51 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string = 
"vmmap - print a vm_map_t structure                                       \\\n\
    Usage : vmmap [addresses...]                                          \\\n\
";

static void prmap(long addr)
{
  char cast_addr[36], buf[256], *resp;  

  sprintf(cast_addr, "((struct\\ vm_map_t\\ *)\\ 0x%p)", addr);
  sprintf(buf, "printf \"Map\\ 0x%p:\\ pmap\\ =\\ 0x%%p\" %s.pmap", addr,
	  cast_addr);
  new_proc(buf, &resp);
  print(resp);
  free(resp);
#ifdef notdef
  sprintf(buf, "printf \"ref=%%d.nentries=%%d,version=%%d\" %s.");
#endif
}

static void Usage(void){
  fprintf(stderr, "Usage : vmmap [addresses...]\n");
  quit(1);
}

main(int argc, char **argv)
{
  long addr;

  check_args(argc, argv, help_string);
  if(argc == 1) Usage();
  argv++;
  while(*argv){
    if(!to_number(*argv, (int) &addr)){
      fprintf(stderr, "Couldn't parse \"%s\" to a number\n", *argv);
    }
    else prmap(addr);
    argv++;
  }
}
