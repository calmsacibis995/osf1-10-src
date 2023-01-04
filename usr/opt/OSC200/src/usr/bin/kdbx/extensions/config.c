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
static char *rcsid = "@(#)$RCSfile: config.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/03/16 19:02:11 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"config - print out the configuration of the machine                      \\\n\
    Usage: config                                                         \\\n\
";

FieldRec bus_fields[] = {
  { ".ctlr_list", NUMBER, NULL, NULL },
  { ".bus_name", STRING, NULL, NULL },
  { ".connect_bus", STRING, NULL, NULL },
  { ".confl1", NUMBER, NULL, NULL },
  { ".confl2", NUMBER, NULL, NULL },
};

char *bus_hints[] = { NULL, "struct controller", NULL, NULL, NULL, NULL };

#define NUM_BUS_FIELDS (sizeof(bus_fields)/sizeof(bus_fields[0]))

FieldRec ctlr_fields[] = {
  { ".nxt_ctlr", NUMBER, NULL, NULL },
  { ".dev_list", POINTER, NULL, NULL },
  { ".ctlr_name", STRING, NULL, NULL }
};

#define NUM_CTLR_FIELDS (sizeof(ctlr_fields)/sizeof(ctlr_fields[0]))

static Boolean prctlr(long addr)
{
  DataStruct ctlr;
  char *error, buf[256];

  if(addr == 0){
    print("\tNULL controller");
    return(True);
  }
  if(!cast(addr, "struct controller", &ctlr, &error)){
    fprintf(stderr, "Couldn't cast addr to controller:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
  if(!read_field_vals(ctlr, ctlr_fields, NUM_CTLR_FIELDS)){
    field_errors(ctlr_fields, NUM_CTLR_FIELDS);
    return(False);
  }
  sprintf(buf, "\tController \"%s\" (0x%p)", ctlr_fields[2].data, addr);
  print(buf);
  return(True);
}

main(int argc, char **argv)
{
  char buf[256], *error, *conf1, *conf2;
  DataStruct busses, bus;
  int i, n;

  check_args(argc, argv, help_string);
  if(!check_fields("struct bus", bus_fields, NUM_BUS_FIELDS, bus_hints)){
    field_errors(bus_fields, NUM_BUS_FIELDS);
    quit(1);
  }
  if(!check_fields("struct controller", ctlr_fields, NUM_CTLR_FIELDS, NULL)){
    field_errors(bus_fields, NUM_BUS_FIELDS);
    quit(1);
  }
  busses = read_sym("bus_list");
  if((n = array_size(busses, &error)) == -1){
    fprintf(stderr, "Couldn't call array_size:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  for(i=0;i<n;i++){
    if((bus = array_element(busses, i, &error)) == NULL){
      fprintf(stderr, "Couldn't read bus[%d]:\n", i);
      fprintf(stderr, "%s\n", error);
    }
    if(!read_field_vals(bus, bus_fields, NUM_BUS_FIELDS)){
      field_errors(bus_fields, NUM_BUS_FIELDS);
      quit(1);
    }
    if(bus_fields[1].data != 0){
      sprintf(buf, "Bus #%d (0x%p): Name - \"%s\"\tConnected to - \"%s\"",
	      i, struct_addr(bus), bus_fields[1].data, bus_fields[2].data);
      print(buf);
      conf1 = addr_to_proc((long) bus_fields[3].data);
      conf2 = addr_to_proc((long) bus_fields[4].data);
      sprintf(buf, "\tConfig 1 - %s\tConfig 2 - %s", conf1, conf2);
      free(conf1);
      free(conf2);
      print(buf);
      if(!prctlr((long) bus_fields[0].data)) quit(1);
      print("");
    }
  }
  quit(0);
}
