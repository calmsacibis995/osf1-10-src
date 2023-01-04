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
static char *rcsid = "@(#)$RCSfile: walk.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/29 15:26:35 $";
#endif

#include "krash.h"

char *help_string = 
"walk - interactively traverse data structures                            \\\n\
    Usage : walk                                                          \\\n\
    walk accepts the following commands:                                  \\\n\
        .fieldname - descends into the fieldname field of the current     \\\n\
                     structure or union.                                  \\\n\
        [index - descends into the index-th element of the current array. \\\n\
                 Note that the closing bracket is not used.               \\\n\
        symbol - examines symbol                                          \\\n\
        struct tag address - examines address cast to struct tag *        \\\n\
        union tag address - examines address cast to union tag *          \\\n\
        >b - Back out one level                                           \\\n\
    The current context is printed before each command.  If there is no   \\\n\
    context, \"?\" is printed.  The current data structure is printed in  \\\n\
    full when it is first descended into, and when the user ascends back  \\\n\
    out of one of its parts.  An empty command line will also cause the   \\\n\
    current structure to be printed.                                      \\\n\
                                                                          \\\n\
    You can jump from inside a structure to a completely different symbol \\\n\
    at any point.  When you back out of that symbol, you will be back to  \\\n\
    the original structure.                                               \\\n\
";

static void print_buf(char *str)
{
  char *start, c;
  Boolean out;
  
  while(1){
    start = str;
    out = False;
    while((*start != '\n') && (*start != '\0')){
      if(!isspace(*start)) out = True;
      start++;
    }
    if(out){
      c = *start;
      *start = '\0';
      print(str);
      *start = c;
    }
    str = start + 1;
    if(*start == '\0') break;
  }
}

static Boolean input_loop(char *command, DataStruct context, char *where,
			  char *prev_name)
{
  DataStruct sym;
  char buf[256], name[256], where_buf[256], *resp, *data, *error, *signature;
  char *tag, *addr;
  long index, address;
  int type, len;
  Boolean ret;

  if(strlen(command) == 0) return(True);
  strcpy(where_buf, where);
  if(!strncmp(command, "struct ", strlen("struct ")) ||
     !strncmp(command, "union ", strlen("union "))){
    next_token(command, NULL, &tag);
    if((tag = next_token(tag, &len, &addr)) == NULL){
      print("Expected [struct or union] tag address");
      return(False);      
    }
    if(!next_number(addr, NULL, &address)){
      print("Couldn't parse address");
      return(False);
    }
    tag[len] = '\0';
    if(!cast(address, command, &sym, &error)){
      print(error);
      return(False);
    }
    sprintf(name, "(*((%s *) 0x%lx))", command, address);
    if(strlen(where_buf) > 0) strcat(where_buf, " ");
    sprintf(&where_buf[strlen(where_buf)], "((%s *) 0x%lx)", command, address);
  }
  else if(command[0] == '.'){
    if(context == NULL) return(True);
    type = sym_type(context);
    if(type == POINTER) context = deref_pointer(context);
     sym = get_field(context, &command[1], &error);
    if(sym == NULL){
      print(error);
      return(False);
    }
    signature = struct_signature(context);
    if(!signature || strchr(signature, '{')){
      strcpy(name, prev_name);
      strcat(name, command);
    }
    else sprintf(name, "((%s *) 0x%lx)%s", signature, struct_addr(context),
		 command);
    strcat(where_buf, command);
  }
  else if(command[0] == '['){
    if(!to_number(&command[1], &index)){
      print("Index not a number");
      return(False);
    }
    sym = array_element(context, index, &error);
    if(sym == NULL){
      print(error);
      return(False);
    }
    strcpy(name, prev_name);
    strcat(name, "[");
    sprintf(&name[strlen(name)], "%d", index);
    strcat(name, "]");
    strcat(where_buf, "[");
    sprintf(&where_buf[strlen(where_buf)], "%d", index);
    strcat(where_buf, "]");    
  }
  else {
    if((sym = read_sym(command)) == NULL){
      print("Symbol not found");
      return(False);
    }
    strcpy(name, command);
    if(strlen(where_buf) > 0) strcat(where_buf, " ");
    strcat(where_buf, command);
  }
  type = sym_type(sym);
  if(type == POINTER) sprintf(buf, "print *%s", name);
  else sprintf(buf, "print %s", name);
  dbx(buf, True);
  data = read_response_status();
  ret = True;
  while(1){
    if(ret) print_buf(data);
    if(type == NUMBER) return(False);
    print(where_buf);  
    if((resp = read_response_status()) != NULL){
      if(resp[strlen(resp) - 1] == '\n') resp[strlen(resp) - 1] = '\0';
      if(!strcmp(resp, ">b")) break;
      ret = input_loop(resp, sym, where_buf, name);
    }
    free(resp);
  }
  free(data);
}

main(int argc, char **argv)
{
  char *resp;

  check_args(argc, argv, help_string);
  while(1){
    print("?");  
    if((resp = read_response_status()) != NULL){
      if(resp[strlen(resp) - 1] == '\n') resp[strlen(resp) - 1] = '\0';
      if(!strcmp(resp, ">b")) break;
      input_loop(resp, NULL, "", "");
      free(resp);
    }
  }
  quit(0);
}
