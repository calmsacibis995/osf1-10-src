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
static char *rcsid = "@(#)$RCSfile: array_action.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/11/17 14:54:17 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string = 
"array_action - perform some action on each element of an array           \\\n\
    Usage : array_action type length start-address [switches] command     \\\n\
	type - type of address of array element                           \\\n\
        length - number of elements in array                              \\\n\
	address - address of array - may be a variable name or a number   \\\n\
	command - kdbx or dbx command to perform on each element of array \\\n\
	    printf-like substitutions will be performed on the command    \\\n\
            for each element.  The possible substitutions are:            \\\n\
		%a - address of element                                   \\\n\
		%c - cast of address to pointer to array element          \\\n\
		%i - index of element within the array                    \\\n\
		%s - size of element                                      \\\n\
		%t - type of pointer to element                           \\\n\
        If -head is present, the next argument is printed as the table    \\\n\
            header                                                        \\\n\
        If -size is present, the next argument is used as the array       \\\n\
	    element size.  Otherwise, the size is calculated from the     \\\n\
            element type.                                                 \\\n\
        If -cond is present, the next arg is used as a filter.  It is     \\\n\
            evaluated by dbx for each array element, and if it evaluates  \\\n\
            to true, the action is taken on the element.  The same        \\\n\
            substitutions that are applied to the command are applied     \\\n\
            to the condition.                                             \\\n\
    Example: array_action \"struct proc *\" nproc allproc \"p %c->p_pid\"   \\\n\
";

static void Usage(void){
  fprintf(stderr,
	  "Usage : array_action type length start-address [-head head] [-size size] command\n");
  quit(1);
}

static void format(char **command, char *buf, char *type, long addr, int size,
		   int i)
{
  char *sub, **cmd_ptr, *save, *ptr;

  ptr = buf;
  *ptr = '\0';
  for(cmd_ptr=command;*cmd_ptr;cmd_ptr++){
    sub = *cmd_ptr;
    save = sub;
    while((sub = index(sub, '%')) != NULL){
      if(!strncmp(sub, "%a", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "0x%p", addr + i *size);
      }
      else if(!strncmp(sub, "%c", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "((%s) 0x%p)", type, addr + i * size);
      }
      else if(!strncmp(sub, "%i", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "%d", i);
      }
      else if(!strncmp(sub, "%s", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "%d", size);
      }
      else if(!strncmp(sub, "%t", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "%s", type);
      }
      else sprintf(&buf[strlen(buf)], "%%c", *(sub + 1));
      *sub = '%';
      sub += 2;
      save = sub;
    }
    strcat(buf, save);
    for(;*ptr;ptr++){
      if(isspace(*ptr)){
	memmove(ptr + 1, ptr, strlen(ptr) + 1);
	*ptr = '\\';
	ptr++;
      }
    }
    strcat(buf, " ");
    ptr = &buf[strlen(buf)];
  }
}

main(int argc, char **argv)
{
  char *type, *ptr, **command, buf[256], *resp, *line, *error, *cond[2];
  char *save, *head;
  long addr, len, size;
  int i;
  Status status;
  Boolean doit;

  check_args(argc, argv, help_string);
  if(argc < 5) Usage();
  size = 0;
  type = argv[1];
  if(!to_number(argv[2], &len)) Usage();
  addr = strtoul(argv[3], &ptr, 16);
  if(*ptr != '\0'){
    if(!read_sym_val(argv[3], NUMBER, &addr, &error)){
      fprintf(stderr, "Couldn't read %s:\n", argv[3]);
      fprintf(stderr, "%s\n", error);
      Usage();
    }
  }
  argv += 4;
  argc -= 4;
  head = NULL;
  cond[0] = NULL;
  cond[1] = NULL;
  while(**argv == '-'){
    if(!strcmp(*argv, "-head")){
      head = argv[1];
      argv += 2;
      argc -= 2;
    }
    else if(!strcmp(*argv, "-size")){
      if(!to_number(argv[1], &size)) Usage();
      argv += 2;
      argc -= 2;
    }
    else if(!strcmp(*argv, "-cond")){
      if(argc < 2) Usage();
      cond[0] = argv[1];
      argv += 2;
      argc -= 2;
    }
  }
  if(!*argv) Usage();
  command = argv;
  if(size == 0){
    sprintf(buf, "print sizeof(*((%s) 0))", type);
    dbx(buf, True);
    if((resp = read_response(&status)) == NULL){
      print_status("Couldn't read sizeof", &status);
      quit(1);
    }
    size = strtoul(resp, &ptr, 0);
    if(ptr == resp){
      fprintf(stderr, "Couldn't parse sizeof(%s):\n", type);
      quit(1);    
    }
    free(resp);
  }
  if(head) print(head);
  context(True);
  for(i=0;i<len;i++){
    if((cond[0] == NULL) || !strcmp(cond[0], "true")) doit = True;
    else if(!strcmp(cond[0], "false")) doit = False;
    else {
      sprintf(buf, "dbx print ");
      format(cond, &buf[strlen(buf)], type, addr, size, i);
      krash(buf, False, True);
      line = read_response(NULL);
      if((line == NULL) || strncmp(line, "true", strlen("true"))) doit = False;
      else doit = True;
    }
    if(doit){
      format(command, buf, type, addr, size, i);
      krash(buf, False, True);
      while((line = read_line(&status)) != NULL){
	print(line);
	free(line);
      }
      if(status.type != OK){
	print_status("read_line failed", &status);
	quit(1);
      }    
    }
  }
  quit(0);
}
