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
static char *rcsid = "@(#)$RCSfile: mbuf.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/10/13 20:05:26 $";
#endif

/* This extension prints the mbuf data structures */

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include "krash.h"

static char *help_string =
"mbuf - print the mbufs or mclusters                                \\\n\
    Usage : mbuf [options]                                          \\\n\
 options:                                                           \\\n\
 (no option) : print the mbufs associated with sockets             \\\n\
";

FieldRec file_fields[] = {
  { ".f_type", NUMBER, NULL, NULL },
  { ".f_count", NUMBER, NULL, NULL },
  { ".f_data", NUMBER, NULL, NULL }
};

#define NUM_FILE_FIELDS (sizeof(file_fields)/sizeof(file_fields[0]))

FieldRec sock_fields[] = {
  { ".so_type", NUMBER, NULL, NULL },
  { ".so_rcv.sb_mb", NUMBER, NULL, NULL },
  { ".so_snd.sb_mb", NUMBER, NULL, NULL },
};

#define NUM_SOCK_FIELDS (sizeof(sock_fields)/sizeof(sock_fields[0]))

FieldRec mbuf_fields[] = {
  { ".m_hdr.mh_next", NUMBER, NULL, NULL },
  { ".m_hdr.mh_nextpkt", NUMBER, NULL, NULL },
  { ".m_hdr.mh_len", NUMBER, NULL, NULL },
  { ".m_hdr.mh_data", NUMBER, NULL, NULL },
  { ".m_hdr.mh_type", NUMBER, NULL, NULL },
  { ".m_hdr.mh_flags", NUMBER, NULL, NULL },
};

#define NUM_MBUF_FIELDS (sizeof(mbuf_fields)/sizeof(mbuf_fields[0]))

static char *mh_type(type)
int type;
{
  switch(type){
  case MT_FREE: return("MT_FREE");
  case MT_DATA: return("MT_DATA");
  case MT_HEADER: return("MT_HEADER");
  case MT_SOCKET: return("MT_SOCKET");
  case MT_PCB: return("MT_PCB");
  case MT_RTABLE: return("MT_RTABLE");
  case MT_HTABLE: return("MT_HTABLE");
  case MT_ATABLE: return("MT_ATABLE");
  case MT_SONAME: return("MT_SONAME");
  case MT_SOOPTS: return("MT_SOOPTS");
  case MT_FTABLE: return("MT_FTABLE");
  case MT_RIGHTS: return("MT_RIGHTS");
  case MT_IFADDR: return("MT_IFADDR");
  case MT_CONTROL: return("MT_CONTROL");
  case MT_OOBDATA: return("MT_OOBDATA");
  case MT_MAX: return("MT_MAX");
  default: return("???");
  }
}


main(argc, argv)
int argc;
char **argv;
{
  DataStruct fil, ele, sock, mbuf;
  char *error, buf[256];
  char fileaddr_s[12], sockaddr_s[12], pcbaddr_s[12], sprocaddr_s[12],
rprocaddr_s[12];
  char mbufaddr_s[12], mbufdata_s[12];
  long nfile, nextmbuf;
  int mh_flags, i, mfreeflag = 0, mclfreeflag = 0;

  check_args(argc, argv, help_string);
  if (argc > 1) {
    fprintf(stderr, "This extension can't have arguments \n");
    quit(1);
  }

  /* print mbufs associated with sockets */

  if(!check_fields("struct file", file_fields, NUM_FILE_FIELDS, NULL)){
    field_errors(file_fields, NUM_FILE_FIELDS);
    quit(1);
  }
  if(!check_fields("struct socket", sock_fields, NUM_SOCK_FIELDS, NULL)){
    field_errors(file_fields, NUM_SOCK_FIELDS);
    quit(1);
  }
  fil = read_sym("file");
  if(!read_sym_val("nfile", NUMBER, &nfile, &error)){
    fprintf(stderr, "Couldn't read nfile:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }

  print("  mbuf_addr   sock_addr    end   mh_len  mh_data      mh_type mh_flags");
  print("============  ===========  ===  =======  ===========  ======= ==========");
  /* go through the file table and find all the sockets */
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
    format_addr((long)ele, fileaddr_s);
    format_addr((long)file_fields[2].data, sockaddr_s);

    /* print all mbufs in the receiving end */
    nextmbuf = (long) sock_fields[1].data;
    while (nextmbuf)  /*so_rcv*/
    {
      if(!cast(nextmbuf, "struct mbuf", &mbuf, &error)){
        fprintf(stderr, "Couldn't cast f_data to mbuf:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }
      if(!read_field_vals(mbuf, mbuf_fields, NUM_MBUF_FIELDS)){
        field_errors(mbuf_fields, NUM_MBUF_FIELDS);
        break;
      }
      format_addr((long)nextmbuf, mbufaddr_s);
      format_addr((long)mbuf_fields[3].data, mbufdata_s);
      mh_flags = (int)mbuf_fields[5].data;
      sprintf(buf,"%12s %12s  %3s  %5d   %12s  %-10s %s%s%s%s",
mbufaddr_s, sockaddr_s, "rcv",
          mbuf_fields[2].data, mbufdata_s, mh_type((int)mbuf_fields[4].data),
          mh_flags & M_EXT ? " M_EXT" : "",
          mh_flags & M_PKTHDR ? " M_PKTHDR" : "",
          mh_flags & M_EOR ? " M_EOR" : "",
          mh_flags & M_FASTFREE ? " M_FASTFREE" : "");
      print(buf);
      nextmbuf = (long)mbuf_fields[0].data;
    }  /* end of while */

    /* print all mbufs in the sending end */
    nextmbuf = (long) sock_fields[2].data;
    while (nextmbuf)  /*so_snd*/
    {
      if(!cast(nextmbuf, "struct mbuf", &mbuf, &error)){
        fprintf(stderr, "Couldn't cast f_data to mbuf:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }
      if(!read_field_vals(mbuf, mbuf_fields, NUM_MBUF_FIELDS)){
        field_errors(mbuf_fields, NUM_MBUF_FIELDS);
        break;
      }
      format_addr((long)nextmbuf, mbufaddr_s);
      format_addr((long)mbuf_fields[3].data, mbufdata_s);
      mh_flags = (int)mbuf_fields[5].data;
      sprintf(buf,"%12s %12s  %3s  %5d   %12s  %-10s %s%s%s%s",
mbufaddr_s, sockaddr_s, "snd",
          mbuf_fields[2].data, mbufdata_s, mh_type((int)mbuf_fields[4].data),
          mh_flags & M_EXT ? " M_EXT" : "",
          mh_flags & M_PKTHDR ? " M_PKTHDR" : "",
          mh_flags & M_EOR ? " M_EOR" : "",
          mh_flags & M_FASTFREE ? " M_FASTFREE" : "");
      print(buf);
      nextmbuf = (long)mbuf_fields[0].data;
    }  /* end of while */

  }  /* end of for */
  quit(0);
}


