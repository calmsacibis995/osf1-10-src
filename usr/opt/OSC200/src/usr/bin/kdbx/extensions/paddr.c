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
static char *rcsid = "@(#)$RCSfile: paddr.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 10:00:08 $";
#endif
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "krash.h"

static char *help_string =
"paddr - convert a range of memory to symbolic references                 \\\n\
    Usage : paddr address number-of-longwords                             \\\n\
    address is the starting address                                       \\\n\
    number-of-longwords is the number of words to dump out                \\\n\
";

/* *** Check for error messages in dbx output
   *** change the addr/xX format */

static void translate(start, end, buf)
char *start, *end, *buf;
{
  Status status;
  CommandResponsePkt packet;
  char cmd[12], *ptr, *begin;
  int n;

  ptr = cmd;
  if(strncmp(start, "0x", 2)){
    strcpy(ptr, "0x");
    ptr += 2;
  }
  strncpy(ptr, start, end - start);
  ptr[end - start] = '\0';
  ptr += strlen(ptr);
  strcpy(ptr, "/i");
  dbx(cmd, True);
  if((ptr = read_response(&status)) != NULL){
    while(isspace(*ptr)) ptr++;
    begin = ptr;
    if(*begin == '['){
      if((ptr = index(begin, ']')) == NULL)
	ptr = &packet.msg[strlen(packet.msg)];
      else ptr++;
      *ptr = '\0';
      strcpy(buf, begin);
    }
    else {
      strcpy(buf, "0x");
      strncpy(&buf[2], start, end - start);
      buf[end - start + 2] = '\0';
    }
  }
  else {
    print_status("translate", &status);
    quit(1);
  }
}

static void process_buf(buf)
char *buf;
{
  char *ptr, *start, line_buf[256], *line_ptr;

  ptr = buf;
  while(*ptr != '\0'){
    line_ptr = line_buf;
    while(isspace(*ptr) && (*ptr != '\n') && (*ptr != '\0')) ptr++;
    if(*ptr == '\n'){
      ptr++;
      continue;
    }
    else if(*ptr == '\0') break;
    start = ptr;
    while((*ptr != ':') && (*ptr != '\0')) ptr++;
    if(*ptr == '\0'){
      fprintf(stderr, "Couldn't parse dbx output:\n");
      fprintf(stderr, "%s\n", buf);
      Usage();
    }
    translate(start, ptr, line_ptr);
    ptr++;
    line_ptr += strlen(line_ptr);
    *line_ptr++ = ':';
    *line_ptr++ = ' ';
    while((*ptr != '\n') && (*ptr != '\0')){
      while(isspace(*ptr) && (ptr != '\0')) ptr++;
      start = ptr;
      while(!isspace(*ptr) && (*ptr != '\0')) ptr++;
      translate(start, ptr, line_ptr);
      line_ptr += strlen(line_ptr);
      if((*ptr == '\n') || (*ptr == '\0')){
	*line_ptr++ = '\0';
	break;
      }
      else *line_ptr++ = ' ';
    }
    print(line_buf);
  }
}

Usage(){
  print("Usage : paddr address number-of-longwords");
  quit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  Status status;
  char *buf, out[256], *ptr;
  int n;

  check_args(argc, argv, help_string);
  if(argc != 3) Usage();
  else {
    n = strtoul(argv[2], &ptr, 0);
    if(*ptr != '\0') Usage();
    sprintf(out, "%s/%dX", argv[1], atoi(argv[2]));
    dbx(out, True);
    if((buf = read_response(&status)) == NULL){
      print_status("main", &status);
      quit(1);
    }
    else {
      process_buf(buf);
      quit(0);
    }
  }
}
