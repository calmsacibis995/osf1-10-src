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
static char *rcsid = "@(#)$RCSfile: list_cells.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:52:14 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"";

FieldRec fields[] = {
  { NULL, NUMBER, NULL, NULL }
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

main(int argc, char **argv)
{
  long addr, end_addr;
  char *error, *ptr, *head, *next, *type, *end, *field_name, buf[100];
  DataStruct str;

  check_args(argc, argv, help_string);
  argv++;
  argc--;
  type = argv[0];
  head = argv[1];
  next = argv[2];
  end = argv[3];
  if(!to_number(head, &addr) || !to_number(end, &end_addr)){
    fprintf(stderr, "Couldn't turn one of \"%s\" and \"%s\" into an address\n",
	    head, end);
    quit(1);
  }
  NEW_TYPE(field_name, strlen(next) + 2, char, char *, "main");
  sprintf(field_name, ".%s", next);
  fields[0].name = field_name;
  if(!check_fields(type, fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  do {
    if(!cast(addr, type, &str, &error)){
      fprintf(stderr, "Couldn't cast addr to \"%s\":\n", type);
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_field_vals(str, fields, NUM_FIELDS)){
      field_errors(fields, NUM_FIELDS);
      return(False);
    }
    sprintf(buf, "0x%p", addr);
    print(buf);
    addr = (long) fields[0].data;
  } while(addr != end_addr);
  quit(0);
}
