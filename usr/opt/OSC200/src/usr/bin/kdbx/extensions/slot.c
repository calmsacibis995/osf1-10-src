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
static char *rcsid = "@(#)$RCSfile: slot.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:54:16 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"slot - print the address of the nth cell of a list                       \\\n\
    Usage : slot start-address list-type field-name slot-number           \\\n\
    start-address is the address of the head of the list                  \\\n\
    list-type is the type of a pointer to a list cell                     \\\n\
    field-name is the name of the field that contains the next cell       \\\n\
    slot-number is the index of the desired cell                          \\\n\
";

Usage(){
  fprintf(stderr,
	  "Usage : slot start-address list-type field-name slot-number\n");
  quit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  long start_addr, addr;
  char *ptr, *error, *type, *field, buf[256];
  int slot;

  check_args(argc, argv, help_string);
  if(argc < 5) Usage();
  else {
    start_addr = strtoul(argv[1], &ptr, 16);
    if(*ptr != '\0'){
      if(!read_sym_val(argv[1], NUMBER, &start_addr, &error)){
	fprintf(stderr, "Couldn't read %s:\n", argv[1]);
	fprintf(stderr, "%s\n", error);
	Usage();
      }
    }
    type = argv[2];
    field = argv[3];
    slot = strtoul(argv[4], &ptr, 0);
    if(*ptr != '\0') Usage();
    if(!list_nth_cell(start_addr, type, slot, field, False, &addr, &error)){
      fprintf(stderr, "Couldn't get %d'th element of list\n", slot);
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    sprintf(buf, "0x%p", addr);
    print(buf);
    quit(0);
  }
}
