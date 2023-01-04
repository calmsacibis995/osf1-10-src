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
static char *rcsid = "@(#)$RCSfile: arp.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/06/25 14:59:17 $";
#endif
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include "krash.h"

static char *help_string =
"arp - print contents of the arp table                                    \\\n\
    Usage : arp [-]                                                       \\\n\
    If the optional - is present, arp prints out the entire arp table;    \\\n\
    otherwise it prints out those entries which have non-zero             \\\n\
    at_iaddr.s_addr and at_flags fields.                                   \\\n\
";

/* *** Implement hostnames *** */

FieldRec fields[] = {
  { ".at_iaddr", STRUCTURE, NULL, NULL },
  { ".at_iaddr.s_addr", NUMBER, NULL, NULL },
  { ".at_flags", NUMBER, NULL, NULL },
  { ".at_hwaddr[0]", NUMBER, NULL, NULL },
  { ".at_hwaddr[1]", NUMBER, NULL, NULL },
  { ".at_hwaddr[2]", NUMBER, NULL, NULL },
  { ".at_hwaddr[3]", NUMBER, NULL, NULL },
  { ".at_hwaddr[4]", NUMBER, NULL, NULL },
  { ".at_hwaddr[5]", NUMBER, NULL, NULL },
  { ".at_iaddr", NUMBER, NULL, NULL },
  { ".at_hold", NUMBER, NULL, NULL },
  { ".at_timer", NUMBER, NULL, NULL }
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

static void Usage(){
  print("Usage : arp [-]\n");
  quit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  DataStruct tab, ele;
  int i, j, index;
  long n, arptab_bsiz;
  char ether[20], buf[256], *name, *error;
  struct hostent *hp;
  Boolean all;

  check_args(argc, argv, help_string);
  if(argc == 1) all = False;
  else if(argc == 2){
    if(!strcmp(argv[1], "-")) all = True;
    else Usage();
  }
  else Usage();
  if(!check_fields("struct arptab", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  tab = read_sym("arptab");
  if(!read_sym_val("arptab_size", NUMBER, &n, &error) ||
     !read_sym_val("arptab_bsiz", NUMBER, &arptab_bsiz, &error)){
    fprintf(stderr, "Couldn't read arptab_size or arptab_bsiz:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  hp = NULL;
  sprintf(buf, "        NAME         BUCK SLOT          IPADDR        ETHERADDR   MHOLD  TIMER  FLAGS");
  print(buf);
  print(       "==================== ==== ====  ==============  ===============   =====  =====  =====");

  for(i=0;i<n;i++){
    if((ele = array_element(tab, i, &error)) == NULL){
      fprintf(stderr, "Couldn't get array element\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_field_vals(ele, fields, NUM_FIELDS)){
      field_errors(fields, NUM_FIELDS);
      break;
    }
    if(!all && (((int) fields[1].data == 0) ||
		((int) fields[2].data == 0))) continue;
    sprintf(ether, "%d", (int) fields[3].data);
    for(j=1;j<6;j++){
      index = strlen(ether);
      sprintf(&ether[index], ".%x", (int) fields[j+3].data);
    }
    hp = gethostbyaddr((char *) &fields[1].data, 4, AF_INET);
    if(hp) name = hp->h_name;
    else name = "_no_name__ ";
    sprintf(buf, "%-20.20s %4d %4d %15s  %15s  %6x  %5d  %5x", name,
            i/arptab_bsiz, i%arptab_bsiz,
	    inet_ntoa(fields[1].data), ether, fields[10].data,
	    fields[11].data, fields[2].data);
    print(buf);
  }
  quit(0);
}
