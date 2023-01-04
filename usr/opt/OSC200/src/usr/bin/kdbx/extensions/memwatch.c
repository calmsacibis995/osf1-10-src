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
static char *rcsid = "@(#)$RCSfile: memwatch.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/17 14:54:42 $";
#endif
#include <stdio.h>
#include "krash.h"

#define MALLOC 1
#define REALLOC 2
#define FREE 3

#define ALLOCED 1
#define FREED 2

typedef struct {
  char *file;
  char *func;
  int line;
} Frame;

typedef struct _Mem {
  unsigned int addr;
  int alloced;
  Frame *stack;
  int nframes;
  char *stack_text;
  struct _Mem *next;
} Mem;

static int bps[4] = { 0, 0, 0, 0 };

static Mem *mem_head = NULL;

static void free_mem(Mem *mem)
{
  int i;

  for(i=0;i<mem->nframes;i++){
    free(mem->stack[i].file);
    free(mem->stack[i].func);
  }
  free(mem->stack_text);
  free(mem);
}

static void print_stack(Mem *mem)
{
  char *ptr, *end;

  ptr = mem->stack_text;
  while(*ptr){
    for(end=ptr;*end != '\n';end++) ;
    *end = '\0';
    print(ptr);
    *end = '\n';
    ptr = end + 1;
  }
#ifdef notdef
  for(i=0;i<mem->nframes;i++){
    sprintf(buf, "%s\t%s:%d", mem->stack[i].func, mem->stack[i].file,
	    mem->stack[i].line);
    print(buf);
  }
#endif
}

static int get_bp(void)
{
  char *resp, *ptr, *start;
  int bp;
  
  resp = read_response(NULL);
  resp[strlen(resp) - 1] = '\0';
  if((start = rindex(resp, '\n')) == NULL) start = resp;
  else start++;
  if((start = index(start, '[')) == NULL){
    fprintf(stderr, "get_bp - Couldn't find breakpoint\n");
    quit(1);
  }
  bp = strtoul(&start[1], &ptr, 10);
  if(ptr == &start[1]){
    fprintf(stderr, "get_bp - Couldn't parse breakpoint\n");
    quit(1);
  }
  if(bp == bps[MALLOC]) return(MALLOC);
  else if(bp == bps[REALLOC]) return(REALLOC);
  else if(bp == bps[FREE]) return(FREE);
  else {
    fprintf(stderr, "Hit unexpected breakpoint : %d\n", bp);
    return(0);
  }
}

static void set_bp(void)
{
  char *resp, *ptr, name[32];
  int bp;
  Status status;

  dbx("j", True);
  do resp = read_response(&status);
  while(!resp);
  ptr = resp;
  while(*ptr){
    if(sscanf(ptr, "[%d] stop in %s", &bp, name) != 2){
      fprintf(stderr, "set_bp - sscanf failed\n");
      quit(1);
    }
    if(!strcmp(name, "malloc")){
      if(bps[MALLOC] != 0){
	fprintf(stderr, "Duplicate breakpoints for %s\n", name);
      }
      else bps[MALLOC] = bp;
    }
    else if(!strcmp(name, "realloc")){
      if(bps[REALLOC] != 0){
	fprintf(stderr, "Duplicate breakpoints for %s\n", name);
      }
      else bps[REALLOC] = bp;
    }
    else if(!strcmp(name, "free")){
      if(bps[FREE] != 0){
	fprintf(stderr, "Duplicate breakpoints for %s\n", name);
      }
      else bps[FREE] = bp;
    }
    while(*ptr && (*ptr != '\n')) ptr++;
    if(*ptr == '\n') ptr++;
  }
  free(resp);
}

static Mem *lookup_mem(long addr)
{
  Mem *ptr;

  for(ptr=mem_head;ptr;ptr=ptr->next){
    if(ptr->addr == addr) return(ptr);
  }
  return(NULL);
}

static void delete_mem(long addr)
{
  Mem *ptr, *last;

  for(ptr=mem_head;ptr;last=ptr,ptr=ptr->next){
    if(ptr->addr == addr){
      if(ptr == mem_head) mem_head = ptr->next;
      else last->next = ptr->next;
      free_mem(ptr);
      return;
    }
  }  
}

static void insert_mem(Mem *new)
{
  Mem *ptr, *last;

  if(!mem_head) mem_head = new;
  else if(new->addr < mem_head->addr){
    new->next = mem_head;
    mem_head = new;
  }
  else {
    for(ptr=mem_head;ptr;last=ptr,ptr=ptr->next){
      if(ptr->addr > new->addr){
        last->next = new;
	new->next = ptr;
	return;
      }
    }
    last->next = new;
    new->next = ptr;   
  }
}

static long read_number(void)
{
  char *resp, *ptr, *start;
  long ret;

  do resp = read_response(NULL);
  while(!resp || (strlen(resp) < 10));
  resp[strlen(resp) - 1] = '\0';
  if((start = rindex(resp, '\n')) == NULL) start = resp;
  else start++;
  ret = strtoul(start, &ptr, 0);
  if(ptr == start){
    fprintf(stderr, "read_number - couldn't parse return\n");
    quit(1);
  }
  return(ret);
}

static void get_frame(char *str, Frame *frame)
{
  char *begin, *end, *ptr;

  begin = &str[5];
  if((end = index(begin, '(')) == NULL){
    fprintf(stderr, "get_frame - couldn't find end of func name\n");
    quit(1);
  }
  frame->func = copy(begin, end - begin);
  if((begin = rindex(end, '[')) == NULL){
    fprintf(stderr, "get_frame - couldn't find begin bracket\n");
    quit(1);
  }
  begin += 2;
  if((end = index(begin, '"')) == NULL){
    fprintf(stderr, "get_frame - couldn't find end quote\n");
    quit(1);
  }
  frame->file = copy(begin, end - begin);
  end += 2;
  frame->line = strtoul(end, &ptr, 10);
  if(ptr == end){
    fprintf(stderr, "get_frame - couldn't parse line number\n");
    quit(1);
  }
}

static int get_stack(Frame **ret, char **text)
{
  char *resp, *ptr, *begin;
  int nlines, line;
  
  dbx("where", True);
  resp = read_response(NULL);
  *text = copy(resp, -1);
  nlines = 0;
  for(ptr=resp;*ptr;ptr++) if(*ptr == '\n') nlines++;
  NEW_TYPE(*ret, nlines, Frame, Frame *, "get_stack");
  line = 0;
  ptr = resp;
  do {
    for(begin=ptr;*begin && (*begin != '\n');begin++) ;
    if(!*begin) break;
    else *begin = '\0';
    begin++;
    get_frame(ptr, &(*ret)[line]);
    ptr = begin;
    line++;
  } while(1);
  return(nlines);
}

static Mem *alloc_mem(long addr, Frame *stack, int nframes, char *text)
{
  Mem *new_mem;

  NEW_TYPE(new_mem, 1, Mem, Mem *, "alloc_mem");
  new_mem->addr = addr;
  new_mem->stack = stack;
  new_mem->nframes = nframes;
  new_mem->stack_text = text;
  new_mem->next = NULL;
  return(new_mem);
}

static void handle_malloc(void)
{
  int stack_size;
  Frame *frames;
  char *resp, *ptr, buf[256], *text;
  long addr;
  Mem *mem, *new_mem;

  stack_size = get_stack(&frames, &text);
  dbx("return ; p $r2", True);
  addr = read_number();
  new_mem = alloc_mem(addr, frames, stack_size, text);
  if((mem = lookup_mem(addr)) != NULL){
    sprintf(buf, "malloc returned alloced memory : Addr 0x%p", addr);
    print(buf);
    print("Previous malloc :");
    print_stack(mem);
    print("This malloc :");
    print_stack(new_mem);
    print("");
  }
  insert_mem(new_mem);
}

static void handle_realloc(void)
{
  int stack_size;
  Frame *frames;
  long addr, new_addr;
  Mem *new_mem, *mem;
  char buf[256], *text, *resp;

  stack_size = get_stack(&frames, &text);
  dbx("p *((int *) ($sp + 88))", True);
  addr = read_number();
  new_mem = alloc_mem(addr, frames, stack_size, text);
  if(lookup_mem(addr) == NULL){
    sprintf(buf, "realloc called with non-alloced memory - addr = 0x%p",
	    addr);
    print(buf);
    print("Stack :");
    print_stack(new_mem);
    print("");
  }
  dbx("return", True);
  resp = read_response(NULL);
  if(resp[0] == '[') dbx("return ; return", True);
  resp = read_response(NULL);
  dbx("p $r2", True);
  new_addr = read_number();
  if(new_addr != addr){
    if((mem = lookup_mem(new_addr)) != NULL){
      sprintf(buf, "realloc returned alloced memory : Addr 0x%p", new_addr);
      print(buf);
      print("Previous malloc :");
      print_stack(mem);
      print("This realloc :");
      print_stack(new_mem);
      print("");
    }
    delete_mem(addr);
    insert_mem(new_mem);
  }
}

static void handle_free(void)
{
  int stack_size;
  Frame *frames;
  long addr;
  char buf[256], *text;
  Mem *new_mem;

  dbx("p *((int *) ($sp + 56))", True);
  addr = read_number();
  if(lookup_mem(addr) == NULL){
    sprintf(buf, "Freeing freed or non-alloced memory : Addr 0x%p", addr);
    print(buf);
    stack_size = get_stack(&frames, &text);
    new_mem = alloc_mem(addr, frames, stack_size, text);
    print_stack(new_mem);
    print("");
  }
  delete_mem(addr);
}

main(int argc, char **argv)
{
  char *run, *resp;
  int where, len, i;

  len = strlen("r ") + 1;
  for(i=1;i<argc;i++) len += strlen(argv[i]) + 1;
  NEW_TYPE(run, len, char, char *, "main");
  strcpy(run, "r ");
  for(i=1;i<argc;i++){
    strcat(run, argv[i]);
    strcat(run, " ");
  }
  dbx("stop in malloc", False);
#ifdef notdef
  dbx("stop in realloc", False);
#endif
  dbx("stop in free", False);
  set_bp();
  krash(run, False, True);
  while(where = get_bp()){
    switch(where){
    case MALLOC:
      handle_malloc();
      break;
    case REALLOC:
      handle_realloc();
      break;
    case FREE:
      handle_free();
      break;
    default:
      break;
    }
    krash("cont", False, True);
  }
  quit(0);
}
