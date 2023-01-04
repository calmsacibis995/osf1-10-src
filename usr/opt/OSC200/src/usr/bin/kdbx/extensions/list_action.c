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
static char *rcsid = "@(#)$RCSfile: list_action.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/11/17 14:54:33 $";
#endif
#include <stdio.h>
#include <ctype.h>
#include "krash.h"

static char *help_string = 
"list_action - perform some action on each element of an list             \\\n\
    Usage : list_action type next-field end-addr start-addr [-head header]\\\n\
            command                                                       \\\n\
	type - type of address of list element                            \\\n\
        next-field - name of the field that points to the next element    \\\n\
        end-addr - value of the next field that terminates the list       \\\n\
	address - address of list - may be a variable name or a number    \\\n\
	command - kdbx or dbx command to perform on each element of list  \\\n\
	    printf-like substitutions will be performed on the command    \\\n\
            for each element.  The possible substitutions are:            \\\n\
		%a - address of element                                   \\\n\
		%c - cast of address to pointer to list element           \\\n\
		%i - index of element within the list                     \\\n\
                %n - name of next field                                   \\\n\
		%t - type of pointer to element                           \\\n\
        If -head is present, the next argument is printed as the table    \\\n\
            header                                                        \\\n\
        If -cond is present, the next arg is used as a filter.  It is     \\\n\
            evaluated by dbx for each array element, and if it evaluates  \\\n\
            to true, the action is taken on the element.  The same        \\\n\
            substitutions that are applied to the command are applied     \\\n\
            to the condition.                                             \\\n\
    Example: list_action \"struct proc *\" p_nxt 0 allproc \"p %c->p_pid\"    \\\n\
";

static void Usage(){
  fprintf(stderr,
	  "Usage : list_action type next-field end-addr start-addr command\n");
  quit(1);
}

static void format(command, buf, type, addr, last, i, next)
char **command, *buf, *type;
long addr, last, next;
int i;
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
	sprintf(&buf[strlen(buf)], "0x%p", addr);
      }
      else if(!strncmp(sub, "%c", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "((%s) 0x%p)", type, addr);
      }
      else if(!strncmp(sub, "%e", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "0x%p", last);
      }
      else if(!strncmp(sub, "%i", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "%d", i);
      }
      else if(!strncmp(sub, "%n", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "0x%p", next);
      }
      else if(!strncmp(sub, "%t", 2)){
	*sub = '\0';
	strcat(buf, save);
	sprintf(&buf[strlen(buf)], "%s", type);
      }
      else sprintf(&buf[strlen(buf)], "%%%c", *(sub + 1));
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
  char *type, *ptr, **command, *field, buf[256], *line, *error, *cond[2];
  char *head;
  int i;
  long addr, last, next;
  Boolean doit;
  Status status;

  check_args(argc, argv, help_string);
  if(argc < 6) Usage();
  type = argv[1];
  field = argv[2];
  last = strtoul(argv[3], &ptr, 16);
  if(*ptr != '\0'){
    if(!read_sym_val(argv[3], NUMBER, &last, &error)){
      fprintf(stderr, "Couldn't read %s:\n", argv[4]);
      fprintf(stderr, "%s\n", error);
      Usage();
    }
  }
  addr = strtoul(argv[4], &ptr, 16);
  if(*ptr != '\0'){
    if(!read_sym_val(argv[4], NUMBER, &addr, &error)){
      fprintf(stderr, "Couldn't read %s:\n", argv[4]);
      fprintf(stderr, "%s\n", error);
      Usage();
    }
  }
  argv += 5;
  argc -= 5;
  head = NULL;
  cond[0] = NULL;
  cond[1] = NULL;
  while(**argv == '-'){
    if(!strcmp(*argv, "-head")){
      if(argc < 2) Usage();
      head = argv[1];
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
  command = argv;
  if(head) print(head);
  i = 0;
  do {
    buf[0] = '\0';
    if(!list_nth_cell(addr, type, 1, field, False, &next, &error)){
      fprintf(stderr, "Couldn't get next cell:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if((cond[0] == NULL) || !strcmp(cond[0], "true")) doit = True;
    else if(!strcmp(cond[0], "false")) doit = False;
    else {
      sprintf(buf, "dbx print ");
      format(cond, &buf[strlen(buf)], type, addr, last, i, next);
      context(True);
      krash(buf, False, True);
      line = read_response(NULL);
      if((line == NULL) || strncmp(line, "true", strlen("true"))) doit = False;
      else doit = True;
      context(False);
    }
    if(doit){
      format(command, buf, type, addr, last, i, next);
      context(True);
      krash(buf, False, True);
      while((line = read_line(&status)) != NULL){
	print(line);
	free(line);
      }
      if(status.type != OK){
	print_status("read_line failed", &status);
	quit(1);
      }
      context(False);
    }
    addr = next;
    i++;
  } while(addr != last);
  quit(0);
}
