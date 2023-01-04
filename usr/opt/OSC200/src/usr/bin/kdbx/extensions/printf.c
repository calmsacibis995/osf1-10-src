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
static char *rcsid = "@(#)$RCSfile: printf.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/03/16 19:02:42 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"printf - use the dbx printf capability                                   \\\n\
    Usage : printf format-string [args...]                                \\\n\
    printf formats one argument at a time to get around dbx's command     \\\n\
    length limitation.  It also implements %s, which dbx's printf doesn't.\\\n\
";

static void dbl_slash(char *buf)
{
  char *ptr;

  for(ptr=buf;*ptr;ptr++){
    if(*ptr == '\\'){
      memmove(ptr + 1, ptr, strlen(ptr) + 1);
      *ptr = '\\';
      ptr++;
    }
  }
}

static char *concat(char *so_far, char *prefix)
{
  char *resp;

  if((resp = read_response(NULL)) == NULL) return(NULL);
  resp[strlen(resp) - 1] = '\0';
  if(!so_far){
    NEW_TYPE(so_far, strlen(resp) + strlen(prefix) + 1, char, char *,
	     "concat");
    strcpy(so_far, prefix);
    strcat(so_far, resp);
  }
  else {
    if((so_far = (char *) realloc(so_far, strlen(resp) + strlen(so_far) +
				  strlen(prefix) + 1)) == NULL){
      perror("realloc");
      quit(1);
    }
    strcat(so_far, prefix);
    strcat(so_far, resp);
  }
  free(resp);
  return(so_far);
}

static char *format_end(char *buf)
{
  if(*buf == '%') return(NULL);
  else {
    if((*buf == '-') || (*buf == '+') || isspace(*buf) || (*buf == '#')) buf++;
    if(*buf == '-') buf++;
    while(isdigit(*buf)) buf++;
    if(*buf == '.'){
      buf++;
      while(isdigit(*buf)) buf++;
    }
    if((*buf == 'h') || (*buf == 'H') || (*buf == 'l') || (*buf == 'L')) buf++;
    buf++;
    return(buf);
  }
}

static void Usage(void)
{
  print("Usage : printf format_string [args...]");
  quit(1);
}

main(int argc, char **argv)
{
  char *begin, *end, *resp, buf[256], c, *out, *ptr;
  Boolean sflag = False;

  check_args(argc, argv, help_string);
  if(argc < 2) Usage();
  begin = argv[1];
  argv += 2;
  out = NULL;
  while(*argv){
    sflag = False;
    for(end=begin;*end;end++){
      if(*end == '%'){
	if(*(end + 1) == 's'){
	  sflag = True;
	  ptr = end;
	  c = *ptr;
	  *ptr = '\0';
	  break;
	}
	else if((ptr = format_end(end + 1)) == NULL) end++;
	else {
	  c = *ptr;
	  *ptr = '\0';
	  break;
	}
      }
    }
    if(sflag){
      sprintf(buf, "print %s", *argv);
#ifdef notdef
      dbl_slash(buf);
#endif
      dbx(buf, True);
      out = concat(out, begin);
    }
    else {
      sprintf(buf, "printf \"%s\",%s", begin, *argv);
#ifdef notdef
      dbl_slash(buf);
#endif
      dbx(buf, True);
      out = concat(out, "");
    }
    *ptr = c;    
    begin = ptr;
    if(sflag) begin += 2;
    argv++;
  }
  if(*begin){
    if((out = (char *)
	realloc(out, (strlen(out) + strlen(begin) + 1) * sizeof(char)))
	== NULL){
      perror("realloc");
      quit(1);
    }
    strcat(out, begin);
  }
  begin = out;
  for(end=out;*end;end++){
    if(*end == '\n'){
      *end = '\0';
      print(begin);
      begin = end + 1;
    }
  }
  if(*begin) print(begin);
  quit(0);
}
