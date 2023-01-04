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
static char *rcsid = "@(#)$RCSfile: inpcb.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/03/16 19:02:20 $";
#endif
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include "krash.h"

static char *help_string =
"inpcb - print the udb and tcb tables                                     \\\n\
    Usage : inpcb [-udp] [-tcp] [address...]                              \\\n\
    If no arguments are present, both tables are printed.  If either -udp \\\n\
    or -tcp are present, then the corresponding table is printed.  If     \\\n\
    addresses are present, then -udp and -tcp are ignored and the entries \\\n\
    named by the addresses are printed.                                   \\\n\
";

static char *head =
"    Foreign Host  FPort       Local Host  LPort      Socket         PCB  Options";

FieldRec fields[] = {
  { ".inp_faddr", NUMBER, NULL, NULL },
  { ".inp_fport", NUMBER, NULL, NULL },
  { ".inp_laddr", NUMBER, NULL, NULL },
  { ".inp_lport", NUMBER, NULL, NULL },
  { ".inp_socket", NUMBER, NULL, NULL },
  { ".inp_ppcb", NUMBER, NULL, NULL },
  { ".inp_options", NUMBER, NULL, NULL },
  { ".inp_next", NUMBER, NULL, NULL },
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

static char *prinaddr(struct in_addr *addr, char *buf)
{
  static struct in_addr last;
  static struct hostent host, *h;

  if(addr->s_addr != 0){
    if(addr->s_addr != last.s_addr)
      h = gethostbyaddr((char *) addr, 4, AF_INET);
    if(h != NULL){
      sprintf(buf, "%-16.16s ",h->h_name);
      last.s_addr = addr->s_addr;
      host = *h;
    }
    else sprintf(buf, "%-16s ", inet_ntoa(*addr));
  }
  else sprintf(buf, "%-16s ", inet_ntoa(*addr));
  return(buf);
}

static Boolean prpcb(long addr, long *next_ret)
{
  DataStruct block;
  char *error, *options, buf[256], socket[12], pcb[12], lmach[18], fmach[18];

  if(!cast(addr, "struct inpcb", &block, &error)){
    fprintf(stderr, "Couldn't cast to inpcb:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
  if(!read_field_vals(block, fields, NUM_FIELDS)){
    field_errors(fields, NUM_FIELDS);
    return(False);;
  }
  if((long) fields[6].data == NULL) options = "";
  else options = "*";
  format_addr((long) fields[4].data, socket);
  format_addr((long) fields[5].data, pcb);
  sprintf(buf, "%s%6d %s%6d %s %s %s",
	  prinaddr((struct in_addr *) &fields[0].data, lmach),
	  ntohs((long) fields[1].data),
	  prinaddr((struct in_addr *) &fields[2].data, fmach),
	  ntohs((long) fields[3].data), socket, pcb, options);
  print(buf);
  if(next_ret) *next_ret = (long) fields[7].data;
  return(True);
}

static Boolean do_block(char *name, char *header)
{
  long in_head, next;
  char buf[256], *error;

  if(!read_sym_addr(name, &in_head, &error)){
    fprintf(stderr, "Couldn't read addr of %s:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
  sprintf(buf, "%s:", header);
  print(buf);
  sprintf(buf, head);
  print(head);
  next = in_head;
  do {
    if(!prpcb(next, &next)) return(False);
  } while((next != 0) && (next != in_head));
  return(True);
}

static void Usage(void){
  fprintf(stderr, "Usage : inpcb [-udp] [-tcp] [address...]\n");
  quit(1);
}

main(int argc, char **argv)
{
  Boolean tcp, udp, all;
  long addr;
  char *ptr, buf[256];

  check_args(argc, argv, help_string);
  all = True;
  tcp = False;
  udp = False;
  argv++;
  argc--;
  while(argc != 0){
    if(!strcmp(*argv, "-tcp")){
      tcp = True;
      all = False;
      argv++;
      argc--;
    }
    else if(!strcmp(*argv, "-udp")){
      udp = True;
      all = False;
      argv++;
      argc--;
    }
    else break;    
  }  
  if(!check_fields("struct inpcb", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  if(argc == 0){
    if(all || udp) if(!do_block("udb", "UDP")) quit(1);
    if(all || tcp) if(!do_block("tcb", "TCP")) quit(1);
  }
  else {
    print(head);
    while(*argv){
      addr = strtoul(*argv, &ptr, 16);
      if(*ptr != '\0'){
	fprintf(stderr, "Couldn't parse \"%s\" to an address\n", *argv);
	Usage();
      }
      prpcb(addr, NULL);
      argv++;
    }
  }
  quit(0);
}
