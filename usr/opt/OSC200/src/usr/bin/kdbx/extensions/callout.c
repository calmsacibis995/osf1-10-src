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
static char *rcsid = "@(#)$RCSfile: callout.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:49:01 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"callout - print the callout table                                        \\\n\
    Usage : callout                                                       \\\n\
";

FieldRec fields[] = {
  { ".c_time", NUMBER, NULL, NULL },
  { ".c_arg", NUMBER, NULL, NULL },
  { ".c_func", NUMBER, NULL, NULL },
  { ".c_next", NUMBER, NULL, NULL },
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

main(int argc, char **argv)
{
  DataStruct callout;
  long next;
  char buf[256], *func, *error, arg[13];

  check_args(argc, argv, help_string);
  if(!check_fields("struct callout", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }  
  if(!read_sym_val("callout", NUMBER, &next, &error)){
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  sprintf(buf, "FUNCTION                         ARGUMENT          TIME");
  print(buf);
  do {
    if(!cast(next, "struct callout", &callout, &error)){
      fprintf(stderr, "Couldn't cast to a callout:\n");
      fprintf(stderr, "%s:\n", error);
    }
    if(!read_field_vals(callout, fields, NUM_FIELDS)){
      field_errors(fields, NUM_FIELDS);
      break;
    }
    func = addr_to_proc((long) fields[2].data);
    format_addr((long) fields[1].data, arg);
    sprintf(buf, "%-32.32s %s %10u", func, arg,
	    fields[0].data);
    print(buf);
    next = (long) fields[3].data;
  } while(next != 0);
  quit(0);
}
