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
static char *rcsid = "@(#)$RCSfile: socket.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/25 14:56:22 $";
#endif
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include "krash.h"

static char *help_string =
"socket - print out the sockets in the file table                         \\\n\
    Usage : socket                                                        \\\n\
";

FieldRec file_fields[] = {
  { ".f_type", NUMBER, NULL, NULL },
  { ".f_count", NUMBER, NULL, NULL },
  { ".f_data", NUMBER, NULL, NULL }
};

#define NUM_FILE_FIELDS (sizeof(file_fields)/sizeof(file_fields[0]))

FieldRec sock_fields[] = {
  { ".so_type", NUMBER, NULL, NULL },
  { ".so_pcb", NUMBER, NULL, NULL },
  { ".so_qlen", NUMBER, NULL, NULL },
  { ".so_qlimit", NUMBER, NULL, NULL },
  { ".so_snd.sb_cc", NUMBER, NULL, NULL },
  { ".so_rcv.sb_cc", NUMBER, NULL, NULL },
  { ".so_snd.sb_select.sb_selproc", NUMBER, NULL, NULL },
  { ".so_rcv.sb_select.sb_selproc", NUMBER, NULL, NULL }
};

#define NUM_SOCK_FIELDS (sizeof(sock_fields)/sizeof(sock_fields[0]))

static char *sock_type(type)
int type;
{
  switch(type){
  case SOCK_STREAM: return(" STRM ");
  case SOCK_DGRAM: return("DGRAM ");
  case SOCK_RAW: return(" RAW  ");
  case SOCK_RDM: return(" RDM  ");
  case SOCK_SEQPACKET: return("SQPAK ");
  default: return(" BAD  ");
  }
}

main(argc, argv)
int argc;
char **argv;
{
  DataStruct fil, ele, sock;
  char *error, buf[256], fileaddr[12], sockaddr[12], sprocaddr[12], rprocaddr[12], pcbaddr[12];
  long nfile;
  int i;

  check_args(argc, argv, help_string);
  if(!check_fields("struct file", file_fields, NUM_FILE_FIELDS, NULL)){
    field_errors(file_fields, NUM_FILE_FIELDS);
    quit(1);
  }
  if(!check_fields("struct socket", sock_fields, NUM_SOCK_FIELDS, NULL)){
    field_errors(sock_fields, NUM_SOCK_FIELDS);
    quit(1);
  }
  fil = read_sym("file");
  if(!read_sym_val("nfile", NUMBER, &nfile, &error)){
    fprintf(stderr, "Couldn't read nfile:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  print("  Fileaddr   Sockaddr   Type        PCB     Qlen Qlim  Scc     Sproc    Rcc      Rproc");
  print("=========== =========== =====  ===========  ==== ==== ==== =========== ==== ==========");
  for(i=0;i<nfile;i++){
    if((ele = array_element(fil, i, &error)) == NULL){
      fprintf(stderr, "Couldn't get array element\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_field_vals(ele, file_fields, NUM_FILE_FIELDS)){
      field_errors(file_fields, NUM_FILE_FIELDS);
      break;
    }
    if(((int) file_fields[1].data == 0) ||
       ((int) file_fields[0].data != 2)) continue;
    if(!cast((long) file_fields[2].data, "struct socket", &sock, &error)){
      fprintf(stderr, "Couldn't cast f_data to socket:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_field_vals(sock, sock_fields, NUM_SOCK_FIELDS)){
      field_errors(sock_fields, NUM_SOCK_FIELDS);
      break;
    }
    sprintf(buf, "%s %s %s %s %4d %4d %4d  %s %4d %s",
	    format_addr(struct_addr(ele),fileaddr), format_addr((long)file_fields[2].data,sockaddr),
	    sock_type(sock_fields[0].data), format_addr((long)sock_fields[1].data,pcbaddr),
	    sock_fields[2].data, sock_fields[3].data,
	    sock_fields[4].data, format_addr((long)sock_fields[6].data,sprocaddr),
            sock_fields[5].data, format_addr((long)sock_fields[7].data,rprocaddr));
    print(buf);
  }
  quit(0);
}

