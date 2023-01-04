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
/*
 * @(#)$RCSfile: krash.h,v $ $Revision: 1.1.7.4 $ (DEC) $Date: 1993/08/23 21:32:09 $
 */
#ifndef __KRASH_H__
#define __KRASH_H__

#include <stdio.h>
#include <stdarg.h>
#include "vdComResPkt.h"
#include "util.h"

#define READ_EOF 1
#define NOT_PROMPT 2
#define IN_PROGRESS 3

#define POINTER 1
#define STRUCTURE 2
#define ARRAY 3
#define STRING 4
#define NUMBER 5

typedef enum { OK, Comm, Local } StatusType;

typedef struct {
  StatusType type;
  union {
    int comm;
    int local;
  } u;
} Status;

#ifndef KRASHLIB
typedef struct {
  char *name;
  int type;
  caddr_t data;
  char *error;
} FieldRec;

typedef long DataStruct;
#endif

typedef struct {
  char **how_hints;
  char **type_hints;
} KHints;

extern char *read_response(Status *status);
extern char *read_line(Status *status);
extern int read_char(void);
extern char *next_token(char *ptr, int *len_ret, char **next_ret);
extern char *copy(char *ptr, int len);
extern char *addr_to_proc(long addr);
extern unsigned int array_size(DataStruct sym, char **error);
extern char *struct_addr(DataStruct data);
extern Boolean read_prompt(Status *status);
extern Boolean read_sym_val(char *name, int type, long *ret_val, char **error);
extern Boolean read_sym_addr(char *name,  long *ret_val, char **error);
extern Boolean read_field_vals(DataStruct data, FieldRec *fields, int nfields);
extern Boolean check_fields(char *symbol, FieldRec *fields, int nfields,
			    char **hints);
extern Boolean cast(long addr, char *type, DataStruct *ret_type,
		    char **error);
extern Boolean array_element_val(DataStruct sym, int i, long *ele_ret,
				 char **error);
extern Boolean to_number(char *str, long *val);
extern Boolean next_number(char *buf, char **next, long *ret);
extern void quit(int i);
extern void print(char *message);
extern void dbx(char *command, Boolean expect_output);
extern void print_status(char *message, Status *status);
extern void krash(char *command, Boolean quote, Boolean expect_output);
extern void free_sym(DataStruct sym);
extern void field_errors(FieldRec *fields, int nfields);
extern void new_proc(char *args, char **output_ret);
extern void context(Boolean user);
extern DataStruct read_sym(char *name);
extern DataStruct array_element(DataStruct sym, int i, char **error);
extern DataStruct deref_pointer(DataStruct data);
extern Boolean set_array_size(DataStruct sym, int size, int ele_size,
			      char **error);
extern void check_args(int argc, char **argv, char *help_string);
extern Boolean list_nth_cell(long addr, char *type, int n,
			     char *next_field, Boolean do_check, 
			     long *val_ret, char **error);
extern Boolean read_memory(long start_addr, int nbytes, char *buf, char **error);
extern char *format_addr(long addr, char *buffer);
extern long to_address(char *addr, char **error);
extern unsigned int sizeof_type(char *type, char **error);
extern Boolean set_array_element_size(DataStruct sym, int size, char **error);
extern int field_offset(char *type, char *field, char **error);
extern int sym_type(DataStruct sym);
extern char *read_response_status(void);
extern DataStruct get_field(DataStruct sym, char *field, char **error);
extern char *struct_signature(DataStruct sym);

#endif
