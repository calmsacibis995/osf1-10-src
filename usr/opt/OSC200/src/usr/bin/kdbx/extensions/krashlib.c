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
static char *rcsid = "@(#)$RCSfile: krashlib.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/08/23 21:32:17 $";
#endif
/* ***
   Pointer size = 0
   handle unions and enums
   printf cpu

   beatnk:/osfdumps
*/

#define KRASHLIB

#define KERNEL
#ifdef __alpha
#include <arch/alpha/machparam.h>
#endif
#include <arch/alpha/pmap.h>
#undef KERNEL
#include <errno.h>
#include <math.h>
#include "krashlib.h"
#include "krash.h"
#include "array.h"

static Boolean quote_special_chars(char **buf_ret);
static Boolean check_fields1(char *symbol, FieldRec *fields, int nfields,
			     char **hints, DataStruct *type_ret);
static DataStruct whatis(char *name, FieldRec *fields, int nfields,
			 char **hints, Boolean flush);
static DataStruct new_struct(StructType type);
static Boolean read_packet(CommandResponsePkt *packet, Boolean consume_input,
			   Status *status);
static Boolean check_progress(void);
static Boolean check_message_buf(Status *status);
static long *parse_memory(char *buf, int offset, int size);
static Boolean get_val(DataStruct sym, int type, Boolean by_ref,
		       long *ret_val, char **error);
static Boolean search_tab(char *name, DataStruct data_ret);
static void replace_sym(char *name, DataStruct data);
static char *unit_end(char *buf);
static void canonicalize(char *buf);
static Boolean make_tmparray(char *type, char *bracket, DataStruct ret,
			     char **error);
static Boolean make_tmpstruct(char *type, DataStruct ret, char **error);
static Boolean make_number(DataStruct ret, char **error);
static DataStruct parse_id(char *name, DataStruct type, char **name_ret);
static void add_type(char *type, DataStruct data, Boolean whatisable);
static DataStruct parse_whatis(char *type, char **error, char **name_ret);
static DataStruct find_struct_field(FieldArray *fields, char *name,
				    Boolean checking, int *index_ret);
static void update_size(DataStruct data);
static Boolean size_cb(char *buf, void *arg, char **end_ret);
static Boolean offset_size_cb(char *buf, void *arg, char **end_ret);
static Boolean array_ele_size(char *foo, void *arg, char **bar);
static void batch(char *cmd, Boolean (*proc)(char *, void *, char **),
		  caddr_t arg, int size);
static void append_field(OffsetSizeRec *rec, char *name);
static Boolean read_offset_size(int *offset_ptr, int *size_ptr, char **error);
static Boolean offset_and_size(FieldRec *field, FieldType type, char *name,
			       void *arg);
static Boolean walk_field(Boolean (*proc)(FieldRec *, FieldType, char *, 
			       void *), caddr_t arg, FieldRec *field);
static void lookup_unknown_structs(DataStruct data, DataStructArray *done,
				   DataStructArray *not_done);
static long get_sym_val(char *name, DataStruct data);
static Boolean parse_val(char *val, int type, long *ret_val);
static Boolean check_type(DataStruct sym, int type, char **error);
static Boolean read_val(FieldRec *field, FieldType type, char *name,
		        void *arg);
Boolean read_field_vals(DataStruct data, FieldRec *fields, int nfields);
static void read_offset(unsigned int *buf, int offset, int size, caddr_t ptr);
static Boolean check_list_params(char *type, char *field, char **error);
static Boolean list_walk(void *addr, char *type, char *next_field,
			 void **val_ret, char **error, IntArray *rec);

static char *message = NULL;
static char *message_end = NULL;
static char *message_ptr = NULL;
static CommandResponsePkt last_packet = { CRPMAGIC, x_null };
static Boolean in_progress = False;
static Boolean input_consumed = False;

static TypeArray *type_tab = NULL;
static PacketArray *packets = NULL;
static DataStructRec TemplateDataStruct = { 0, 0, 0, NULL, False, -1, 0, 0, 0,
					      -1 };
static StructU TemplateStructU = { NULL, (1 << 30), 0, NULL };
static TmpStructU TemplateTmpStructU = { NULL, 0, NULL, NULL, NULL };
static ArrayU TemplateArrayU = { 0, NULL };
static TmpArrayU TemplateTmpArrayU = { 0, NULL, 0, NULL, 0, NULL };

char *copy(char *ptr, int len)
{
  char *ret;

  if(ptr == NULL) return(NULL);
  if(len == -1) len = strlen(ptr);
  NEW_TYPE(ret, len + 1, char, char *, "copy");
  strncpy(ret, ptr, len);
  ret[len] = '\0';
  return(ret);
}

void print_status(char *message, Status *status)
{
  int save;

  switch(status->type){
  case Comm:
    fprintf(stderr, "%s\n", message);
    save = errno;
    errno = status->u.comm;
    perror("Communication error");
    errno = save;
    break;
  case Local:
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "Local error - ");
    switch(status->u.local){
    case READ_EOF:
      fprintf(stderr, "Unexpectedly read EOF\n");
      break;
    case NOT_PROMPT:
      fprintf(stderr, "%s\n", message);
     fprintf(stderr, "Next packet was not a prompt packet\n");
      break;
    default:
      fprintf(stderr, "Unknown error - %d\n", status->u.local);
      break;
    }
    break;
  case OK:
    break;
  default:
    fprintf(stderr, "Unknown status type - %d\n", status->type);
    break;
  }
}

static Boolean read_packet(CommandResponsePkt *packet, Boolean consume_input,
			   Status *status)
{
  int remain, n, len;
  char *ptr;

  if(last_packet.command != x_null){
    *packet = last_packet;
    last_packet.command = x_null;
  }
  else {
    packet->command = x_null;
    do {
      remain = sizeof(*packet);
      ptr = (char *) packet;
      while(remain != 0){
	n = read(0, ptr, remain);
	if(n < 0){
	  if(status){
	    status->type = Comm;
	    status->u.comm = errno;
	  }
	  return(False);
	}
	else if(n == 0){
	  if(status){
	    status->type = Local;
	    status->u.local = READ_EOF;
	  }
	  return(False);
	}
	remain -= n;
	ptr += n;
      }
      if(consume_input){
	input_consumed = True;
	if((packet->command == x_debugger) || (packet->command == x_unknown)){
	  if(!message){
	    NEW_TYPE(message, strlen(packet->msg) + 1, char, char *,
		     "read_packet");
	    *message = '\0';
	    ptr = message;
	  }
	  else {
	    len = strlen(message);
	    if((ptr = (char *) realloc(message, (len + strlen(packet->msg) + 1)
				       * sizeof(char))) == NULL){
	      fprintf(stderr, "realloc failed in read_packet\n");
	      return(False);
	    }
	  }
	  if(ptr != message){
	    message_end = &ptr[strlen(ptr)];
	    if(!message_ptr) message_ptr = message_end;
	    else message_ptr = ptr + (message_ptr - message);
	    message = ptr;
	  }
	  strcat(message, packet->msg);
	  message_end = &message[strlen(message)];
	}
	else if(packet->command != x_prompt){
	  fprintf(stderr, "consume_input is True and packet is not text\n");
	  return(False);
	}
      }
      if(packet->command == x_prompt) in_progress = False;
    } while(consume_input && (packet->command != x_prompt));
  }
  if(status) status->type = OK;
  return(True);
}

static Boolean check_progress(void)
{
  CommandResponsePkt packet;
  Status status;

  if(in_progress){
    if(!read_packet(&packet, True, &status)){
      print_status("check_progress - reading packet", &status);
    }
  }
  in_progress = True;
  return(True);
}

Boolean read_prompt(Status *status)
{
  CommandResponsePkt packet;

  if(last_packet.command != x_null){
    if(last_packet.command == x_prompt){
      last_packet.command = x_null;
      return(True);
    }
    if(status){
      status->type = Local;
      status->u.local = NOT_PROMPT;
    }
    return(False);
  }
  else if(read_packet(&packet, False, status)){
    if(packet.command == x_prompt) return(True);
    else {
      last_packet = packet;
      return(False);
    }
  }
  else return(False);
}

static Boolean check_message_buf(Status *status)
{
  CommandResponsePkt packet;
  int len;

  if((message_ptr == NULL) || (message_ptr == message_end)){
    if(input_consumed){
      input_consumed = False;
      return(False);
    }
    if(read_packet(&packet, False, status) != True) return(False);
    if((packet.command == x_debugger) || (packet.command == x_unknown)){
      if(message) free(message);
      len = strlen(packet.msg);
      if(len >= sizeof(packet.msg)) len = sizeof(packet.msg);
      NEW_TYPE(message, len + 1, char, char *, "check_message_buf");
      strncpy(message, packet.msg, len);
      message[len] = '\0';
      message_end = &message[strlen(message)];
      message_ptr = message;
    }
    else return(False);
  }
  return(True);
}

char *read_response(Status *status)
{
  char *buf;
  int len, olen;
  Status s;

  buf = NULL;
  olen = 0;
  len = 0;
  while(check_message_buf(status)){
    len += strlen(message_ptr);
    if(buf == NULL){
      NEW_TYPE(buf, len + 1, char, char *, "read_response");
      strcpy(buf, message_ptr);
    }
    else {
      buf = (char *) realloc(buf, (len + 1) * sizeof(char *));
      strcpy(&buf[olen], message_ptr);
    }
    olen = len;
    message_ptr = NULL;
  }
#ifdef notdef
  if(!read_prompt(&s)){
    if(status && (status->type == OK)) *status = s;
    free(buf);
    buf = NULL;
  }
#endif
  return(buf);
}

char *read_line(Status *status)
{
  char *buf, *ptr, *eol;
  int len, olen;
  Boolean val;
  CommandResponsePkt packet;

  buf = NULL;
  len = 0;
  olen = 0;
  do {
    if((val = check_message_buf((Status *) status)) == False) break;
    if((eol = index(message_ptr, '\n')) == NULL)
      eol = &message_ptr[strlen(message_ptr)];
    len += eol - message_ptr;
    if(buf == NULL){
      NEW_TYPE(buf, len + 1, char, char *, "read_line");
      strncpy(buf, message_ptr, len);
    }
    else {
      buf = (char *) realloc(buf, (len + 1) * sizeof(char));
      strncpy(&buf[olen], message_ptr, len - olen);
    }
    buf[len] = '\0';
    olen = len;
    if(*eol == '\n') message_ptr = eol + 1;
    else message_ptr = eol;
  } while(*eol == '\0');
  if(val == False) return(NULL);
  else return(buf);
}

int read_char(void){
  char c;

  if(check_message_buf((Status *) NULL)){
    c = *message_ptr;
    message_ptr++;
    return((int) c);
  }
  else return(EOF);
}

void quit(int i)
{
  if(check_progress()){
    printf("quit\n");
    fflush(stdout);
    exit(i);
  }
}

static Boolean quote_special_chars(char **buf_ret)
{
  int nquotes;
  char *new, *ptr, *ptr1, *c;
  Boolean ret;

  nquotes = 0;
  for(c="\"'\\";*c;c++){
    ptr = *buf_ret;
    while((ptr = index(ptr, *c)) != NULL){
      nquotes++;
      ptr++;
    }
  }  
  if(nquotes > 0){
    NEW_TYPE(new, strlen(*buf_ret) + nquotes + 1, char, char *, "print");
    ptr1 = new;
    for(ptr=*buf_ret;*ptr;ptr++){
      if((*ptr == '\"') || (*ptr == '\\') || (*ptr == '\'')) *ptr1++ = '\\';
      *ptr1++ = *ptr;
    }
    *ptr1 = '\0';
    ret = True;
    *buf_ret = new;
  }
  else ret = False;
  return(ret);
}

void print(char *message)
{
  Status status;
  CommandResponsePkt packet;
  Boolean free_message;

  if(check_progress()){
    free_message = quote_special_chars(&message);
    printf("print \"%s\"\n", message);
    if(free_message) free(message);
    fflush(stdout);
    if(!read_packet(&packet, False, &status))
      print_status("krashlib - reply failed", &status);
  }
}

void context(Boolean user)
{
  Status status;
  CommandResponsePkt packet;

  if(check_progress()){
    if(user) printf("context user\n");
    else printf("context proc\n");
    fflush(stdout);
    if(!read_packet(&packet, False, &status))
      print_status("krashlib - reply failed", &status);
  }
}

void dbx(char *command, Boolean expect_output)
{
  Status status;
  CommandResponsePkt packet;
  Boolean free_message;

  if(check_progress()){
    free_message = quote_special_chars(&command);    
    printf("dbx \"%s\"\n", command);
    if(free_message) free(command);
    fflush(stdout);
    if(!expect_output && !read_packet(&packet, False, &status))
      print_status("krashlib - reply failed", &status);
  }
}

void krash(char *command, Boolean quote, Boolean expect_output)
{
  Status status;
  CommandResponsePkt packet;
  Boolean free_message;

  free_message = False;
  if(check_progress()){
    if(quote) free_message = quote_special_chars(&command);    
    printf("%s\n", command);
    if(free_message) free(command);
    fflush(stdout);
    if(!expect_output && !read_packet(&packet, False, &status))
      print_status("krashlib - reply failed", &status);
  }
}

char *next_token(char *ptr, int *len_ret, char **next_ret)
{
  char *ret;

  ret = ptr;
  while(isspace(*ret) && (*ret != '\0')) ret++;
  if(*ret == '\0') return(NULL);
  ptr = ret;
  while(!isspace(*ptr) && (*ptr != '\0')) ptr++;
  if(len_ret) *len_ret = ptr - ret;
  if(next_ret) *next_ret = ptr;
  return(ret);
}

static DataStruct new_struct(StructType type)
{
  DataStruct ret;

  NEW_TYPE(ret, 1, DataStructRec, DataStructRec *, "new_struct");
  *ret = TemplateDataStruct;
  ret->type = type;
  switch(type){
  case Structure:
  case Union:
    ret->u.structure = TemplateStructU;
    break;
  case TmpStruct:
  case TmpUnion:
    ret->u.tmpstruct = TemplateTmpStructU;
    break;
  case Pointer:
    ret->u.pointer = NULL;
    break;
  case ArrayType:
    ret->u.array = TemplateArrayU;
    break;
  case TmpArray:
    ret->u.tmparray = TemplateTmpArrayU;
    break;
  case ErrorType:
    ret->u.error = NULL;
    break;
  case StringType:
  case Number:
  case BitString:
    break;
  default:
    fprintf(stderr, "new_struct - Bad type : %d\n", type);
    break;
  }
  return(ret);
}

char *read_response_status(void)
{
  Status status;
  char *ret;

  if((ret = read_response(&status)) == NULL){
    print_status("krashlib - read failed", &status);
    quit(1);
  }
  else return(ret);
}

void free_sym(DataStruct sym)
{
  Field *f;
  DataStruct s;
  int i;

  switch(sym->type){
  case Pointer:
  case Number:
  case BitString:
  case StringType:
    free(sym);
    break;
  case Structure:
  case Union:
    if(sym->u.structure.fields){
      FieldArrayIterPtr(sym->u.structure.fields, f){
	if(f->name) free(f->name);
	if(f->data) free_sym(f->data);
      }
    }
    break;
  case ArrayType:
    if(sym->u.array.type) free_sym(sym->u.array.type);
    break;
  case ErrorType:
    if(sym->u.error) free(sym->u.error);
    break;
  default:
    fprintf(stderr, "free_sym - Bad DataStruct type : %d\n", sym->type);
    break;
  }
}

static long *parse_memory(char *buf, int offset, int size)
{
  long *buffer, *ret;
  int index, len;
  char *ptr, *token, *next;

  NEW_TYPE(buffer, offset + size, long, long *, "parse_memory");
  ret = buffer;
  index = offset;
  ptr = buf;
  while(index < offset + size){
    if((token = next_token(ptr, &len, &next)) == NULL){
      ret = NULL;
      break;
    }
    ptr = next;
    if(token[len - 1] == ':') continue;
    buffer[index] = strtoul(token, &ptr, 16);
    if(ptr != &token[len]){
      ret = NULL;
      break;
    }
    index++;
  }
  if(ret == NULL) free(buffer);
  return(ret);
}

static Boolean get_val(DataStruct sym, int type, Boolean by_ref,
		       long *ret_val, char **error)
{
  Boolean ret;
  char *buf, *ptr, *ptr1;
  long *mem;
  unsigned int size, offset;
  Status status;

  if(sym->addr == 0){
    if(error) *error = "Can't dereference a NULL pointer";
    in_progress = False;
    return(False);
  }
  if((type == STRING) && (sym->type == ArrayType) &&
     (sym->u.array.type->size == sizeof(char))){
    printf("dbx \"0x%p/s\"\n", sym->addr);
    fflush(stdout);
    buf = read_response_status();
    ptr = index(buf, '"') + 1;
    ptr1 = index(ptr, '"');
    mem = (long *) copy(ptr, ptr1 - ptr);
    free(buf);
    *ret_val = (long) mem;
    return(True);
  }
  if(sym->size == 0) sym->size = sizeof(long *);
  if(sym->type == Structure){
    if(sym->u.structure.low > sym->size) sym->u.structure.low = 0;
    if(sym->u.structure.high == 0) sym->u.structure.high = sym->size;
    offset = sym->u.structure.low & ~(sizeof(long) - 1);
    size = sym->u.structure.high - offset;
  }
  else {
    size = sym->size;
    offset = 0;
  }
  printf("coredata 0x%p 0x%p\n", sym->addr + offset,
	 sym->addr + offset + size);
#ifdef notdef
  printf("dbx \"0x%p/%dX\"\n", sym->addr + offset,
	 (size - 1)/sizeof(long) + 1);
#endif
  fflush(stdout);
  buf = read_response_status();
  if((mem = parse_memory(buf, offset/sizeof(long),
			 (size - 1)/sizeof(long) + 1)) == NULL){
    if(error) *error = "Couldn't parse memory output";
    free(buf);
    return(False);
  }
  if(type == STRING && ((sym->type == Pointer) || (sym->type == StringType))){
    if(mem[0] == 0) mem = NULL;
    else {
      printf("dbx \"0x%p/s\"\n", mem[0]);
      fflush(stdout);
      buf = read_response_status();
      free(mem);
      ptr = index(buf, '"') + 1;
      ptr1 = index(ptr, '"');
      mem = (long *) copy(ptr, ptr1 - ptr);
      free(buf);
    }
  }
  ret = True;
  if(by_ref) *ret_val = (long) mem;
  else {
    bcopy(mem, ret_val, sym->size);
    free(mem);
  }
  if(error) *error = NULL;
  return(ret);
}

static Boolean search_tab(char *name, DataStruct data_ret)
{
  Type *t;
  DataStruct ret;

  if(type_tab){
    TypeArrayIterPtr(type_tab, t){
      if(!strcmp(t->name, name)){
	*data_ret = *t->data;
	return(True);
      }
    }
  }
  else type_tab = TypeArrayNew(NULL);
  return(False);
}

static void replace_sym(char *name, DataStruct data)
{
  Type *t;
  DataStruct ret;

  if(type_tab){
    TypeArrayIterPtr(type_tab, t){
      if(!strcmp(t->name, name)){
        *t->data = *data;
        return;
      }
    }
  }
  fprintf(stderr, "replace_sym - Didn't find symbol \"%s\"\n", name);
}

static char *unit_end(char *buf)
{
  int level;

  level = 0;
  while(1){
    if(*buf == '{') level++;
    else if(*buf == '}') level--;
    else if(level == 0){
      if(*buf == ';') return(buf + 1);
      if(*buf == '\0') return(buf);
    }
    else if(*buf == '\0') return(NULL);
    buf++;
  }
}

Boolean next_number(char *buf, char **next, long *ret)
{
  char *token, *ptr;
  int len;
  long val;

  if((token = next_token(buf, &len, next)) == NULL) return(False);  
  val = strtoul(token, &ptr, 0);
  if(ret) *ret = val;
  if(ptr == token) return(False);
  if(next) *next = ptr;
  return(True);
}

static void canonicalize(char *buf)
{
  char *start, *ptr;

  start = buf;
  while(*buf){
    if(isspace(*buf)){
      ptr = buf;
      while(isspace(*ptr) && (*ptr != '\0')) ptr++;
      if((buf == start) || (*ptr == '\0') || !IS_IDCHAR(*ptr) ||
	 !IS_IDCHAR(*(buf - 1)))
	strcpy(buf, ptr);
      else {
	if(ptr > buf + 1) strcpy(buf + 1, ptr);
	*buf = ' ';
      }
    }
    buf++;
  }
}

static Boolean make_tmparray(char *type, char *bracket, DataStruct ret,
			     char **error)
{
  int len, element_len;
  long size;
  char *ptr, *end, *name, *element;

#ifdef __mips
  if(type == bracket){
    name = NULL;
    len = 0;
  }
  else {
    /*
    ** Strip the type from the name.
    */
    name = type;
    while (IS_IDCHAR(*name)) name++;
    while (*name == '*' || *name == ' ') name++;
    len = bracket - name;
    element = type;
    element_len = bracket - type;
  }
#endif
#ifdef __alpha
  if(type == bracket){
    name = NULL;
    len = 0;
  }
  else {
    name = type;
    len = bracket - type;
  }  
#endif
  if(!next_number(&bracket[1], &ptr, &size)){
    *error = "Couldn't parse array size";
    return(False);
  }
  else if(*ptr != ']'){
    *error = "Expected matching ']'";
    return(False);
  }
  ptr += 1;
  if((end = unit_end(ptr)) == NULL){
    *error = "Couldn't find end of array element";
    return(False);
  }
#ifdef __alpha
  element = ptr + 3;
  element_len = end - element - 1;
#endif
  *ret = TemplateDataStruct;
  ret->type = TmpArray;
  ret->u.tmparray.size = size;
  ret->u.tmparray.element = element;
  ret->u.tmparray.len = element_len;
  ret->u.tmparray.name = name;
  ret->u.tmparray.name_len = len;
  ret->u.tmparray.after = ptr;
  return(True);
}

static Boolean make_tmpstruct(char *type, DataStruct ret, char **error)
{
  StructType ret_type;
  TmpFieldArray *fields;
  TmpField field;
  char *name, *ptr, *signature_end, *brace, *file_char;
  int name_len;
  Boolean signature;

  signature_end = NULL;
  if(!strncmp(type, "struct", 6)){
    ret_type = TmpStruct;
    ptr = &type[6];
  }
  else if(!strncmp(type, "union", 5)){
    ret_type = TmpUnion;
    ptr = &type[5];
  }
  else {
    *error = "Didn't find 'struct' or 'union'";
    return(False);
  }
  if(*ptr == '{'){ /* struct{...}x */
    name = NULL;
    name_len = 0;
    signature = True;
  }
  else if(*ptr != ' '){ /* struct*x */
    if((brace = strchr(ptr, '{')) == NULL) brace = &ptr[strlen(ptr)];
    if((((file_char = strchr(ptr, '/')) != NULL) && (file_char < brace)) ||
       (((file_char = strchr(ptr, '.')) != NULL) && (file_char < brace))){
      while(IS_IDCHAR(*ptr) || (*ptr == '/') || (*ptr == '.')) ptr++;
    }
    name = NULL;
    name_len = 0;
    signature = False;
  }
  else { /* struct foo{...} */    
    ptr++;
    name = ptr;
    while(IS_IDCHAR(*ptr)) ptr++;
    name_len = ptr - name;
    signature = True;
  }
  if(*ptr != '{'){
    fields = NULL;
    if(signature) signature_end = ptr;
  }
  else {
    if(signature && name) signature_end = ptr;
    ptr++;
    fields = TmpFieldArrayNew(NULL);
    while(*ptr != '}'){
      field.field = ptr;
      if((ptr = unit_end(ptr)) == NULL){
	*error = "Couldn't find end of field";
	TmpFieldArrayDestroy(fields);
	return(False);
      }
      field.len = ptr - field.field;
      FieldArrayAppend(fields, &field);
    }
    if(signature && !signature_end) signature_end = ptr;
    ptr++;
  }
  *ret = TemplateDataStruct;
  ret->type = ret_type;
  ret->u.tmpstruct.name = name;
  ret->u.tmpstruct.name_len = name_len;
  ret->u.tmpstruct.fields = fields;
  ret->u.tmpstruct.signature_end = signature_end;
  ret->u.tmpstruct.after = ptr;
  return(True);
}

static Boolean make_number(DataStruct ret, char **error)
{
  *ret = TemplateDataStruct;
  ret->type = Number;
  return(True);
}

static DataStruct parse_id(char *name, DataStruct type, char **name_ret)
{
  DataStruct new;
  char *ptr;

  while(*name == '*'){
    new = new_struct(Pointer);
    new->u.pointer = type;
    new->size = sizeof(void *);
    type = new;
    name++;
  }

  /*
  ** Get the name of the item.
  */
  ptr = name;
  while(IS_IDCHAR(*ptr)) ptr++;
  if(name_ret)
    if(*name == ';') *name_ret = NULL;
    else
      *name_ret = copy(name, ptr - name);

  /*
  ** Check for an array of structures.
  */
  if (*ptr++ == '[') {
    new = new_struct(ArrayType);
    new->u.array.type = type;
    new->u.array.size = 0;
    while (isdigit(*ptr))
      new->u.array.size = (new->u.array.size * 10) + (*ptr++ - '0');
    type = new;
    }
  return(type);
}

static void add_type(char *type, DataStruct data, Boolean whatisable)
{
  Type type_rec, *t;
  Boolean found;

  if(!type_tab) type_tab = TypeArrayNew(NULL);
  found = False;
  TypeArrayIterPtr(type_tab, t){
    if(!strcmp(t->name, type)){
      found = True;
      break;
    }
  }
  if(!found){
    type_rec.name = type;
    type_rec.data = data;
    type_rec.whatisable = whatisable;
    TypeArrayAppend(type_tab, &type_rec);
  }
}

static void adjust_offsets(DataStruct d, int adjust)
{
  Field *f;
  
  FieldArrayIterPtr(d->u.structure.fields, f){
    f->data->offset += adjust;
    if(f->data->type == Structure) adjust_offsets(f->data, adjust);
  }
}

static int alignment_size(DataStruct d)
{
  DataStruct new_d;
  Field *f;
  int size, max;

  while((d->type != Number) && (d->type != Pointer) && (d->type != BitString)){
    switch(d->type){
    case Structure:
#ifdef notdef
      d = FieldArrayElement(d->u.structure.fields, 0)->data;
      break;
#endif
    case Union:
      max = 0;
      FieldArrayIterPtr(d->u.structure.fields, f){
	if((d->type == Structure) || (f->data->size == d->size)){
	  size = alignment_size(f->data);
	  if(size > max){
	    max = size;
	    new_d = f->data;
	  }
	}
      }
      d = new_d;
      break;
    case ArrayType:
      d = d->u.array.type;
      break;
    }
  }
  return(d->size);
}

static DataStruct parse_whatis(char *type, char **error, char **name_ret)
{
  DataStructRec new;
  DataStruct ret, ele;
  FieldArray *fields;
  TmpField *f;
  Field field;
  long size;
  int len, offset, align_size, max_align;
  char c, *name, *ptr, *signature, buf[256], *resp, *start, *end;
  Boolean whatisable, skip_fields;
  static int next_bit_offset = 0;

  len = strlen("not defined or not active");
  if(!strcmp(&type[strlen(type) - len], "not defined or not active")){
    if(error) *error = type;
    return(NULL);
  }
  if(!strncmp(type, "typedef ", strlen("typedef ")))
    type += strlen("typedef ");
  ptr = type;
  while(IS_IDCHAR(*ptr) || (*ptr == ' ') || (*ptr == '*')) ptr++;
  if(*ptr == '['){
    if(!make_tmparray(type, ptr, &new, error)) return(NULL);
    if(name_ret)
      *name_ret = copy(new.u.tmparray.name, new.u.tmparray.name_len);
    c = new.u.tmparray.element[new.u.tmparray.len];
    new.u.tmparray.element[new.u.tmparray.len] = '\0';
    ele = parse_whatis(new.u.tmparray.element, error, NULL);
    new.u.tmparray.element[new.u.tmparray.len] = c;
    if(ele == NULL) ret = NULL;
    else {
      ret = new_struct(ArrayType);
      ret->u.array.size = new.u.tmparray.size;
      ret->u.array.type = ele;
      ret->size = ret->u.array.size * ele->size;
    }
  }
  else if(!strncmp(type, "struct", 6) || !strncmp(type, "union", 5)){
    if(!make_tmpstruct(type, &new, error)) return(NULL);
    if(!strncmp(type, "union", 5)) ret = new_struct(Union);
    else ret = new_struct(Structure);
    if(new.u.tmpstruct.fields) ret->u.structure.fields = FieldArrayNew(NULL);
    else ret->u.structure.fields = NULL;
    ret->u.structure.signature = NULL;
    skip_fields = False;
    if(new.u.tmpstruct.signature_end){
      signature = copy(type, new.u.tmpstruct.signature_end - type);
      ret->u.structure.signature = signature;
      if(search_tab(signature, ret)){
	free(signature);
	skip_fields = True;
      }
      else {
	if(new.u.tmpstruct.name) whatisable = True;
	else whatisable = False;
	if(new.u.tmpstruct.fields) add_type(signature, ret, whatisable);
      }
    }
    else signature = NULL;
    if(!skip_fields){
      if(new.u.tmpstruct.fields){
	fields = ret->u.structure.fields;
	offset = -1;
	size = -1;
	max_align = 0;
	TmpFieldArrayIterPtr(new.u.tmpstruct.fields, f){
	  c = f->field[f->len];
	  f->field[f->len] = '\0';
	  ele = parse_whatis(f->field, error, &name);
	  f->field[f->len] = c;
	  if(ele == NULL){
	    FieldArrayDestroy(fields);
	    return(NULL);
	  }
	  field.name = name;
	  field.data = ele;
	  align_size = alignment_size(ele);
	  if(offset == -1){
	    offset = 0;
	    ele->offset = 0;
	  }
	  else {
	    offset = (offset + size + (align_size - 1)) & ~(align_size - 1);
	    ele->offset = offset;
	  }
	  if(align_size > max_align) max_align = align_size;
	  size = ele->size;
	  if((ret->type == Union) && (size > ret->size)) ret->size = size;
#ifdef notdef
	  if(ele->type == Structure) adjust_offsets(ele, offset);
#endif
	  FieldArrayAppend(fields, &field);	  
	}
	if(ret->type == Structure)
	  ret->size = (offset + size + (max_align - 1)) & ~(max_align - 1);
      }
      else fields = NULL;
      ret->u.structure.signature = signature;
    }
    ret = parse_id(new.u.tmpstruct.after, ret, name_ret);
  }
  else {
    /*
    ** Parse the bitstring field size.
    */
    if (*ptr == ':') {
      /*
      ** Bitstring type.  We must record the bit offset and
      ** size fields in the DataStruct.  These will be used
      ** later on to modify the data item.  Note that this
      ** code strips any `:n' value from the name string.
      */
      ret = new_struct(BitString);
      ret->bit_offset = next_bit_offset;
      ret->bit_size   = 0;
      *ptr++ = 0;
      while (*ptr >= '0' && *ptr <= '9') {
        ret->bit_size = (ret->bit_size * 10) + (*ptr - '0');
        *ptr++ = 0;
        }
      while (*ptr != ';') *ptr++ = 0;
      next_bit_offset = next_bit_offset + ret->bit_size;
      }
    else {
      /*
      ** Type is none of the above; default to Number.
      */      
      start = type;
      end = type;
      while(IS_IDCHAR(*end)) end++;
      len = end - start;
      if(!strncmp(start, "unsigned", len)){
	start = end + 1;
	end = start;
	while(IS_IDCHAR(*end)) end++;
	len = end - start;
      }
      size = -1;
      if(!strncmp(start, "char", len)) size = sizeof(char);
      else if(!strncmp(start, "short", len)) size = sizeof(short);
      else if(!strncmp(start, "int", len)) size = sizeof(int);
      else if(!strncmp(start, "long", len)) size = sizeof(long);
      else if(!strncmp(start, "void", len)) size = 0;
      else if(!strncmp(start, "enum", len)) size = sizeof(int);
      if(size != -1){
	ret = new_struct(Number);
	ret->size = size;
      }
      else {
	c = *end;
	*end = '\0';
	ret = new_struct(Number);
	if(!search_tab(start, ret)){
	  printf("dbx \"whatis %s\"\n", start);
	  fflush(stdout);
	  resp = read_response_status();
	  canonicalize(resp);
	  free(ret);
	  ret = parse_whatis(resp, error, name_ret);
	  free(resp);
	  if(ret == NULL) return(NULL);
#ifdef notdef
	  sprintf(buf, "print sizeof(%s)", type);
	  dbx(buf, True);
	  resp = read_response_status();
	  next_number(resp, NULL, &size);
	  ret->size = size;
	  free(resp);
#endif
	  add_type(copy(start, -1), ret, False);
	}
	*end = c;
      }
    }

    /*
    ** If the current item is not a bitstring, then
    ** clear out the offset field so that we start anew
    ** in the next occurrance of bitstrings.
    */
    if (ret->type != BitString)
      next_bit_offset = 0;

    /*
    ** Record the name of the field.
    */
    ptr = &type[strlen(type) - 1];
    if(*ptr == ';') ptr--;
    while(IS_FIELDNAME_CHAR(*ptr)) ptr--;
    while(*ptr == '*') ptr--;
    ret = parse_id(ptr + 1, ret, name_ret);
  }
  return(ret);
}
  
static DataStruct find_struct_field(FieldArray *fields, char *name,
				    Boolean checking, int *index_ret)
{
  Field *f;
  int i;

  i = 0;
  FieldArrayIterPtr(fields, f){
    if(!strcmp(f->name, name)){
      if((checking == False) &&
	 ((f->data->offset == -1) || (f->data->size == 0))){
	fprintf(stderr, "Field \"%s\" wasn't checked\n", name);
	if(index_ret) *index_ret = -1;
	return(NULL);
      }
      if(index_ret) *index_ret = i;
      return(f->data);
    }
    i++;
  }
  if(index_ret) *index_ret = -1;
  return(NULL);
}

static void update_size(DataStruct data)
{
  DataStructRec new;

  if((data->type == Structure) || (data->type == Union)){
    if((data->u.structure.signature != NULL) &&
       (search_tab(data->u.structure.signature, &new) == True)){
      if(new.size == 0){
	new.size = data->size;
	replace_sym(data->u.structure.signature, &new);
      }
      else if(new.size != data->size){
	fprintf(stderr, "update_size - old size != new size\n");
      }
    }
  }
}

static Boolean size_cb(char *buf, void *arg, char **end_ret)
{
  char *next;
  long size;
  SizeCbRec *rec;

  rec = (SizeCbRec *) arg;
  if(!next_number(buf, &next, &size)){
    if(rec->field) rec->field->error = "Couldn't read size";
    else fprintf(stderr, "Couldn't read size");
    return(False);
  }
  rec->data->size = size;
  while((*next != '\n') && (*next != '\0')) next++;
  if(*next == '\n') *end_ret = next + 1;
  else *end_ret = next;
  if(rec->replace) replace_sym(rec->data->u.structure.signature, rec->data);
  else update_size(rec->data);
  return(True);
}

static Boolean offset_size_cb(char *buf, void *arg, char **end_ret)
{
  char *next;
  long offset, size;
  OffsetCbRec *rec;
  
  rec = (OffsetCbRec *) arg;
  if(buf != NULL){
    if(!next_number(buf, &next, &offset) ||
       !next_number(next, &next, &size)){
      if(rec->field) rec->field->error = "Couldn't read size or offset";
      else fprintf(stderr, "Couldn't read or offset size");
      return(False);
    }
    rec->data->offset = offset;
    rec->data->size = size;
    while((*next != '\n') && (*next != '\0')) next++;
    if(*next == '\n') *end_ret = next + 1;
    else *end_ret = next;
    update_size(rec->data);
    if(rec->type == Dot) rec->data->offset -= rec->rec->parent_offset;
  }
  if(rec->type == Dot) rec->rec->parent_offset += rec->data->offset;
  else if(rec->type == Arrow){
    if(rec->set_offset) rec->rec->parent_offset = rec->data->offset;
    else rec->rec->parent_offset = 0;
  }  
  if(rec->data->offset < rec->parent->u.structure.low)
    rec->parent->u.structure.low = rec->data->offset;
  if(rec->data->offset + rec->data->size > rec->parent->u.structure.high)
    rec->parent->u.structure.high = rec->data->offset + rec->data->size;
  rec->data->flag = False;
  return(True);
}

static Boolean array_ele_size(char *foo, void *arg, char **bar)
{
  DataStruct data;

  data = (DataStruct) arg;
  data->u.array.type->size = data->size/data->u.array.size;
  update_size(data->u.array.type);
  return(True);
}

Boolean do_batching = True;
#define DBX_PREFIX "dbx \""

static void batch(char *cmd, Boolean (*proc)(char *, void *, char **),
		  caddr_t arg, int size)
{
  static BatchArray *batches = NULL;
  static char cmd_buf[200];
  static int cmd_len;
  Boolean ret;
  char *resp, *next;
  Batch *b, batch_rec;
  int len;

  if(batches == NULL){
    batches = BatchArrayNew(NULL);
    strcpy(cmd_buf, DBX_PREFIX);
    cmd_len = strlen(cmd_buf);
  }
  if(cmd == NULL) len = 0;
  else len = strlen(cmd);
  if((proc == NULL) || (cmd_len + len + 2 >= sizeof(cmd_buf))){
    if(cmd_len > strlen(DBX_PREFIX)){
      printf("%s\"\n", cmd_buf);
      fflush(stdout);
      resp = read_response_status();
      next = resp;
      BatchArrayIterPtr(batches, b){
	if(b->send_output) ret = (*b->proc)(next, b->arg, &next);
	else ret = (*b->proc)(NULL, b->arg, &next);
	if(b->free) free(b->arg);
	if(ret == False) break;
      }
      free(resp);
      BatchArrayEmpty(batches);
      strcpy(cmd_buf, DBX_PREFIX);
      cmd_len = strlen(cmd_buf);
    }
  }
  if(proc != NULL){
    batch_rec.proc = proc;
    if(cmd == NULL) batch_rec.send_output = False;
    else {
      batch_rec.send_output = True;
      if(cmd_len > strlen(DBX_PREFIX)){
	cmd_buf[cmd_len] = ';';
	cmd_len++;
	cmd_buf[cmd_len] = '\0';
      }
      strcpy(cmd_buf + cmd_len, cmd);
      cmd_len += strlen(cmd);
    }
    if(size != 0){
      NEW_TYPE(batch_rec.arg, size, char, caddr_t, "batch");
      bcopy(arg, batch_rec.arg, size);
      batch_rec.free = True;
    }
    else {
      batch_rec.arg = arg;
      batch_rec.free = False;
    }
    BatchArrayAppend(batches, &batch_rec);
  }
  if((do_batching == False) && (proc != NULL)) batch(NULL, NULL, NULL, 0);
}

static void append_field(OffsetSizeRec *rec, char *name)
{
  char *new_name;

  NEW_TYPE(new_name, strlen(rec->base_name) + strlen(name) + 2,
	   char, char *, "append_field");
  sprintf(new_name, "%s.%s", rec->base_name, name);
  free(rec->base_name);
  rec->base_name = new_name;
}

static Boolean read_offset_size(int *offset_ptr, int *size_ptr, char **error)
{
  char *resp, *next;
  long offset, size;

  fflush(stdout);
  resp = read_response_status();
  if(!next_number(resp, &next, &offset) ||
     !next_number(next, NULL, &size)){
    if(error) *error = "Couldn't parse offset or size";
    return(False);
  }
  *offset_ptr = offset;
  *size_ptr = size;
  return(True);
}

static Boolean offset_and_size(FieldRec *field, FieldType type, char *name,
			       void *arg)
{
  Boolean ret;
  char *ptr, *resp, buf[256];
  long ind;
  DataStruct new, parent;
  Field *f;
  OffsetCbRec cb_rec;
  SizeCbRec size_rec;
  OffsetSizeRec *rec;

  rec = (OffsetSizeRec *) arg;
  ret = True;
  switch(type){
  case Dot:
    if((rec->data->type != Structure) && (rec->data->type != Union)){
      field->error = "Not a structure";
      return(False);
    }
    parent = rec->data;
    if((new = find_struct_field(rec->data->u.structure.fields,
                                name, True, (int *) &ind)) == NULL){
      field->error = "Field not found";
      return(False);
    }
    f = FieldArrayElement(rec->data->u.structure.fields, ind);
    rec->dataptr = &f->data;
    rec->data = new;
    cb_rec.data = rec->data;
    cb_rec.rec = rec;
    cb_rec.parent = parent;
    cb_rec.name = name;
    cb_rec.set_offset = True;
    cb_rec.field = field;
    cb_rec.type = type;

    /*
    ** If *either* the size or offset are not known, we must inquire
    ** about it.  Note that while the size could be known, the offset
    ** might not (in the case of multiple occurrences of the same
    ** structure within another structure).
    */
    if(((rec->data->size == 0) || (rec->data->offset == -1)) &&
       (rec->data->flag == False)){
      sprintf(buf, "print (int)&(%s.%s),sizeof(%s.%s)",
	      rec->base_name, name, rec->base_name, name);
      batch(buf, offset_size_cb, (void *) &cb_rec, sizeof(cb_rec));
      rec->data->flag = True;
    }
    else batch(NULL, offset_size_cb, (void *) &cb_rec, sizeof(cb_rec));
    append_field(rec, name);
    break;
  case Arrow:
    if(rec->data->type == Pointer){
      if((rec->data->u.pointer->type != Structure) &&
	 (rec->data->u.pointer->type != Union)){
	field->error = "Not a pointer to a structure";
	return(False);
      }
      if(rec->data->u.pointer->u.structure.fields == NULL){
	if(rec->data->u.pointer->u.structure.signature == NULL){
	  field->error = "Can't determine type of structure";
	  return(False);
	}
	if(!search_tab(rec->data->u.pointer->u.structure.signature,
		       rec->data->u.pointer)){
	  new = whatis(rec->data->u.pointer->u.structure.signature, NULL,
		       0, NULL, False);
	  if(new == NULL){
	    field->error = "Couldn't find type of structure";
	    return(False);
	  }
	  free_sym(rec->data->u.pointer);
	  rec->data->u.pointer = new;
	}
      }
      rec->data = rec->data->u.pointer;
      free(rec->base_name);
      NEW_TYPE(rec->base_name,
               strlen(rec->data->u.structure.signature) + strlen("(()0)"),
               char, char *, "offset_and_size");
      sprintf(rec->base_name, "((%s)0)", new->u.structure.signature);
      if((new = find_struct_field(rec->data->u.structure.fields,
				  name, True, (int *) &ind)) == NULL){
	field->error = "Field not found";
	return(False);
      }
      f = FieldArrayElement(rec->data->u.structure.fields, ind);
      parent = rec->data;
      rec->dataptr = &f->data;
      rec->data = new;
      cb_rec.data = rec->data;
      cb_rec.rec = rec;
      cb_rec.parent = parent;
      cb_rec.name = name;
      cb_rec.set_offset = True;
      cb_rec.field = field;
      cb_rec.type = type;
      if((rec->data->size == 0) && (rec->data->offset == -1) &&
       (rec->data->flag == False)){
	sprintf(buf, "print (int)&(%s.%s),sizeof(%s.%s)",
	       rec->base_name, name, rec->base_name, name);
	batch(buf, offset_size_cb, (void *) &cb_rec, sizeof(cb_rec));
	rec->data->flag = True;
      }
      else batch(NULL, offset_size_cb, (void *) &cb_rec, sizeof(cb_rec));
      append_field(rec, name);
    }
    else if((rec->data->type == Structure) || (rec->data->type == Union)){
      if(rec->data->u.structure.fields == NULL){
	if(rec->data->u.structure.signature == NULL){
	  field->error = "Can't determine type of structure";
	  return(False);
	}
	if(!search_tab(rec->data->u.structure.signature, rec->data)){
	  new = whatis(rec->data->u.structure.signature, NULL, 0, NULL, False);
	  if(new == NULL){
	    field->error = "Couldn't find type of structure";
	    return(False);
	  }
	  free_sym(rec->data);
	  rec->data = new;
	  *(rec->dataptr) = new;
	}
      }
      free(rec->base_name);
      NEW_TYPE(rec->base_name,
	       strlen(rec->data->u.structure.signature) +
	       strlen("(()0)"),
	       char, char *, "offset_and_size");
      sprintf(rec->base_name, "((%s)0)",
	      rec->data->u.structure.signature);
      if((new = find_struct_field(rec->data->u.structure.fields,
				  name, True, (int *) &ind)) == NULL){
	field->error = "Field not found";
	return(False);
      }
      f = FieldArrayElement(rec->data->u.structure.fields, ind);
      parent = rec->data;
      rec->dataptr = &f->data;
      rec->data = new;
      if(rec->data->type != Pointer){
	field->error = "Field is not a pointer";
	return(False);
      }
      cb_rec.data = rec->data;
      cb_rec.rec = rec;
      cb_rec.parent = parent;
      cb_rec.name = name;
      cb_rec.set_offset = False;
      cb_rec.field = field;
      cb_rec.type = type;
      if((rec->data->size == 0) && (rec->data->offset == -1) &&
	 (rec->data->flag == False)){
	sprintf(buf, "print (int)&(%s.%s),sizeof(%s.%s)",
	       rec->base_name, name, rec->base_name, name);
	batch(buf, offset_size_cb, (void *) &cb_rec, sizeof(cb_rec));
	rec->data->flag = True;
      }
      else batch(NULL, offset_size_cb, (void *) &cb_rec, sizeof(cb_rec));
      rec->dataptr = &rec->data->u.pointer;
      rec->data = rec->data->u.pointer;
      if(((rec->data->type == Structure) || (rec->data->type == Union)) &&
	 (rec->data->u.structure.fields == NULL) &&
	 (rec->data->u.structure.signature == NULL) &&
	 (rec->hint != NULL)){
	new = whatis(rec->hint, NULL, 0, NULL, False);
	if(new != NULL) *(rec->dataptr) = new;
      }
      if((rec->data->size == 0) && (rec->data->offset == -1)){
	sprintf(buf, "print sizeof(*%s.%s)",
		rec->base_name, name);
	size_rec.replace = False;
	size_rec.data = rec->data;
	size_rec.field = field;
	batch(buf, size_cb, (void *) &size_rec, sizeof(size_rec));
      }
      append_field(rec, name);
    }
    else {
      field->error = "Not a pointer or structure";
      return(False);
    }
    break;
  case Index:
    if(rec->data->type != ArrayType){
      field->error = "Not an array";
      return(False);
    }
    if(!next_number(name, NULL, &ind)){
      field->error = "Couldn't parse element index";
      return(False);
    }
    if((ind >= rec->data->u.array.size) || (ind < 0)){
      field->error = "Index out of bounds";
      return(False);
    }
    batch(NULL, array_ele_size, (void *) rec->data, 0);
    rec->dataptr = &rec->data->u.array.type;
    rec->data = rec->data->u.array.type;
    break;
  default:
    fprintf(stderr, "offset_and_size - Bad type : %d\n", type);
    ret = False;
  }
  return(ret);
}

static Boolean other_types(FieldRec *field, FieldType type, char *name,
			   void *arg)
{
  Boolean ret;
  DataStruct new, *rec;
  long ind;
  
  rec = (DataStruct *) arg;
  ret = True;
  switch(type){
  case Dot:
    if(((*rec)->type != Structure) && ((*rec)->type != Union)){
      field->error = "Not a structure";
      return(False);
    }
    if((new = find_struct_field((*rec)->u.structure.fields,
                                name, True, NULL)) == NULL){
      field->error = "Field not found";
      return(False);
    }
    *rec = new;
    break;
  case Arrow:
    if((*rec)->type == Pointer){
      if(((*rec)->u.pointer->type != Structure) &&
	 ((*rec)->u.pointer->type != Union)){
	field->error = "Not a pointer to a structure";
	return(False);
      }
      if((*rec)->u.pointer->u.structure.fields == NULL){
	if((*rec)->u.pointer->u.structure.signature == NULL){
	  field->error = "Can't determine type of structure";
	  return(False);
	}
	if(!search_tab((*rec)->u.pointer->u.structure.signature,
		       (*rec)->u.pointer)){
	  new = whatis((*rec)->u.pointer->u.structure.signature, NULL,
		       0, NULL, False);
	  if(new == NULL){
	    field->error = "Couldn't find type of structure";
	    return(False);
	  }
	  free_sym((*rec)->u.pointer);
	  (*rec)->u.pointer = new;
	}
      }
      *rec = (*rec)->u.pointer;
      if((new = find_struct_field((*rec)->u.structure.fields,
				  name, True, (int *) &ind)) == NULL){
	field->error = "Field not found";
	return(False);
      }
      *rec = new;
    }
    else if(((*rec)->type == Structure) || ((*rec)->type == Union)){
      if((*rec)->u.structure.fields == NULL){
	if((*rec)->u.structure.signature == NULL){
	  field->error = "Can't determine type of structure";
	  return(False);
	}
	if(!search_tab((*rec)->u.structure.signature, (*rec))){
	  new = whatis((*rec)->u.structure.signature, NULL, 0, NULL, False);
	  if(new == NULL){
	    field->error = "Couldn't find type of structure";
	    return(False);
	  }
	  free_sym((*rec));
	  *rec = new;
	}
      }
      if((new = find_struct_field((*rec)->u.structure.fields,
				  name, True, NULL)) == NULL){
	field->error = "Field not found";
	return(False);
      }
      *rec = new;
      if((*rec)->type != Pointer){
	field->error = "Field is not a pointer";
	return(False);
      }
      *rec = (*rec)->u.pointer;
    }
    break;
  case Index:
    if((*rec)->type != ArrayType){
      field->error = "Not an array";
      return(False);
    }
    if(!next_number(name, NULL, &ind)){
      field->error = "Couldn't parse element index";
      return(False);
    }
    if((ind >= (*rec)->u.array.size) || (ind < 0)){
      field->error = "Index out of bounds";
      return(False);
    }
    *rec = (*rec)->u.array.type;
    break;
  default:
    fprintf(stderr, "other_types - Bad type : %d\n", type);
    ret = False;
  }
  return(ret);
}

static Boolean walk_field(Boolean (*proc)(FieldRec *, FieldType, char *, 
				         void *), caddr_t arg, FieldRec *field)
{
  char *name, *end, *next, c;
  FieldType type;
  Boolean ret;

  ret = True;
  name = field->name;
  while(*name){
    if(*name == '.'){
      name++;
      end = name;
      while(IS_IDCHAR(*end)) end++;
      type = Dot;
      next = end;
      }
    else if(*name == '['){
      name++;
      end = name;
      while(isdigit(*end)) end++;
      if(*end != ']'){
	field->error = "No matching ']'";
	break;
      }
      type = Index;
      next = ++end;
    }
    else if(!strncmp(name, "->", 2)){
      name += 2;
      end = name;
      while(IS_IDCHAR(*end)) end++;
      type = Arrow;
      next = end;
    }
    else {
      field->error = "Couldn't parse field";
      break;
    }
    c = *end;
    *end = '\0';
    ret = (*proc)(field, type, name, arg);
    *end = c;
    if(ret == False) break;
    name = next;
  }
  return(ret);
}

static void lookup_unknown_structs(DataStruct data, DataStructArray *done,
				   DataStructArray *not_done)
{
  Field *f;
  int i;
  DataStruct d;

  DataStructArrayIter(done, d, i) if(data == d) return;
  switch(data->type){
  case Pointer:
    DataStructArrayAppend(not_done, &data->u.pointer);
    break;
  case Structure:
  case Union:
    if(((data->u.structure.fields == NULL) || (data->size == 0) ||
        (data->offset == -1)) &&
       (data->u.structure.signature != NULL)){
      search_tab(data->u.structure.signature, data);
    }
    if(data->u.structure.fields != NULL){
      FieldArrayIterPtr(data->u.structure.fields, f){
	DataStructArrayAppend(not_done, &f->data);
      }
    }
    break;
  case ArrayType:
    DataStructArrayAppend(not_done, &data->u.array.type);
    break;
  case StringType:
  case Number:
  case BitString:
    break;
  case TmpStruct:
  case TmpUnion:
  case TmpArray:
  case ErrorType:
  default:
    fprintf(stderr, "lookup_unknown_structs - Bad type : %d\n", data->type);
    break;
  }
  DataStructArrayAppend(done, &data);
}

static DataStruct whatis(char *name, FieldRec *fields, int nfields,
			 char **hints, Boolean flush)
{
  DataStruct new;
  char *error, *name_ret, *resp, *type, buf[256];
  int i;
  FieldRec *f;
  DataStruct rec;
  SizeCbRec size_rec;
  Boolean ret;

  new = new_struct(Structure);
  if(!search_tab(name, new)){
    printf("dbx \"whatis %s\"\n", name);
    fflush(stdout);
    type = read_response_status();
    canonicalize(type);
    free_sym(new);
    new = NULL;
    if((new = parse_whatis(type, &error, &name_ret)) == NULL){
      FIELD_ERROR(fields, nfields, error);
      return(NULL);
    }
#ifdef notdef
    if(new->size == 0){
      sprintf(buf, "print sizeof((%s)0)", name);
      size_rec.replace = True;
      size_rec.data = new;
      if(nfields > 0) size_rec.field = &fields[0];
      else size_rec.field = NULL;
      batch(buf, size_cb, (void *) &size_rec, sizeof(size_rec));
    }
#endif
  }
  ret = True;
  for(i=0;i<nfields;i++){
    rec = new;
    if(!walk_field(other_types, (void *) &rec, &fields[i])) ret = False;
  }
  if(!ret) return(NULL);
  else return(new);
#ifdef notdef
  ret = True;
  for(i=0;i<nfields;i++){
    NEW_TYPE(rec, 1, OffsetSizeRec, OffsetSizeRec *, "whatis");
    fields[i].data = (caddr_t) rec;
    rec->data = new;
    rec->dataptr = &new;
    NEW_TYPE(rec->base_name, strlen(name) + strlen("(()0)") + 1, char, char *,
	     "whatis");
    sprintf(rec->base_name, "((%s)0)", name);
    rec->whatisable = True;
    rec->parent_offset = 0;
    if(hints) rec->hint = hints[i];
    if(!walk_field(offset_and_size, (void *) rec, &fields[i])) ret = False;
    free(rec->base_name);
  }
  if(flush) batch(NULL, NULL, 0, 0);
  for(i=0;i<nfields;i++){
    free(fields[i].data);
    fields[i].data = NULL;
  }
  if(!ret) return(NULL);
  else return(new);
#endif
}

static long get_sym_val(char *name, DataStruct data)
{
  char *token, *ptr, *buf;
  long size, ret;

  printf("dbx \"print sizeof(%s)\"\n", name);
  fflush(stdout);
  buf = read_response_status();
  if(!next_number(buf, NULL, &size)) return(NULL);
  data->size = size;
  if(!get_val(data, NUMBER, True, &ret, NULL)) return(0L);
  else return(ret);
}

static Boolean check_type(DataStruct sym, int type, char **error)
{
  Boolean ret;

  switch(type){
  case POINTER:
  case STRING:
  case NUMBER:
    if((sym->type == Pointer) || (sym->type == Number) ||
       (sym->type == StringType) || (sym->type == BitString) ||
       ((sym->type == ArrayType) && (sym->u.array.type->size == sizeof(char))))
      ret = True;
    else {
      ret = False;
      if(error) *error = "Can't convert type to number, string, or pointer";
    }
    break;
  case STRUCTURE:
    if((sym->type == Structure) || (sym->type == Union)) ret = True;
    else {
      ret = False;
      if(error) *error = "Can't convert type to structure";
    }
    break;
  case ARRAY:
    if(sym->type == ArrayType) ret = True;
    else {
      ret = False;
      if(error) *error = "Can't convert type to array";
    }
    break;
  default:
    ret = False;
    if(error) *error = "Bad type";
  }
  return(ret);
}

Boolean read_sym_addr(char *name, long *ret_val, char **error)
{
  Boolean ret;
  char *resp, *ptr;

  ret = True;
  printf("dbx \"print &%s\"\n", name);
  fflush(stdout);
  resp = read_response_status();
  if(!next_number(resp, NULL, ret_val)){
    *error = "strtoul failed";
    ret = False;
  }
  free(resp);
  return(ret);
}

static Boolean parse_val(char *val, int type, long *ret_val)
{
  char *token, *next;
  int len;
  long n;
  Boolean ret;

  if(type == NUMBER){
    return(next_number(val, NULL, ret_val));
  }
  else if(type == STRING){
    if((token = next_token(val, &len, NULL)) == NULL) return(False);
    if((*token == '"') && ((next = index(token + 1, '"')) != NULL)){
      *((char **) ret_val) = copy(&token[1], next - token - 1);
      return(True);
    }
    if(!next_number(val, &next, NULL)) return(False);
    if((token = next_token(next, &len, &next)) == NULL) return(False);
    if((len != 1) || strncmp(token, "=", len)) return(False);
    if((token = next_token(next, &len, &next)) == NULL) return(False);
    if((*token == '"') && ((next = index(token + 1, '"')) != NULL)){
      *((char **) ret_val) = copy(&token[1], next - token - 1);
      return(True);
    }
  }
  return(False);
}

Boolean read_sym_val(char *name, int type, long *ret_val, char **error)
{
  DataStruct sym;
  Boolean ref, ret;
  char buf[256], *resp;

  sprintf(buf, "print %s", name);
  dbx(buf, True);
  resp = read_response_status();
  ret = parse_val(resp, type, ret_val);
  free(resp);
  if(!ret){
    if((type == NUMBER) && (*name == '&')){
      return(read_sym_addr(&name[1], ret_val, error));
    }
    if((sym = read_sym(name)) == NULL){
      *error = "Couldn't read symbol";
      return(False);
    }
    if(!check_type(sym, type, error)){
      return(False);
    }
    if(type == NUMBER) ref = False;
    else ref = True;
    return(get_val(sym, type, ref, ret_val, NULL));
  }
  else return(True);
}

DataStruct read_sym(char *name)
{
  DataStruct ret;
  char *buf, *ptr, *last;
  long addr;

  if(check_progress()){
    printf("dbx \"whatis %s\"\n", name);
    fflush(stdout);
    buf = read_response_status();
    canonicalize(buf);
    last = NULL;
    ptr = buf - 1;
    while((ptr = strstr(ptr + 1, name)) != NULL) last = ptr;
    if(last)
      memmove(last, last + strlen(name), strlen(last) - strlen(name) + 1);
    if((ret = parse_whatis(buf, &ptr, NULL)) == NULL){
      fprintf(stderr, "%s\n", ptr);
      return(NULL);
    }
    printf("dbx \"print &%s\"\n", name);
    fflush(stdout);
    buf = read_response_status();
    if(!next_number(buf, NULL, &addr)) ret = NULL;
    if(ret) ret->addr = (void *) addr;
    free(buf);
    return(ret);
  }
  else return(NULL);
}

static Boolean read_val(FieldRec *field, FieldType type, char *name,
		        void *arg)
{
  DataStruct f;
  long ind;
  char *ptr;
  ReadValRec *rec;

  rec = (ReadValRec *) arg;
  switch(type){
  case Dot:
    if((rec->base->type != Structure) && (rec->base->type != Union)){
      field->error = "Not a structure";
      return(False);
    }
    if((f = find_struct_field(rec->base->u.structure.fields, name, False,
			      NULL)) == NULL){
      field->error = "Couldn't find field";
      return(False);
    }
    f->addr = rec->base->addr + f->offset;
    if(strcmp(name, &rec->name[strlen(rec->name) - strlen(name)]) ||
       (rec->type != STRING) || (f->type == ArrayType)){
      NEW_TYPE(f->data, f->size, char, long *, "read_val");
      bcopy(((char *) rec->base->data) + f->offset, f->data, f->size);
    }
    else {
      if(!get_val(f, STRING, True, (long *) &f->data, &field->error))
	return(False);   
    }
    rec->base = f;
    break;
  case Arrow:
    if(rec->base->type == Pointer){
      if((rec->base->u.pointer->type != Structure) &&
	 (rec->base->u.pointer->type != Union)){
	field->error = "Not a pointer to a structure";
	return(False);
      }
      rec->base->u.pointer->addr = *((char **) rec->base->data);
      rec->base = rec->base->u.pointer;
      if(rec->base->info != rec->info){
	if(rec->base->data) free(rec->base->data);
	if(!get_val(rec->base, STRUCTURE, True, (long *) &rec->base->data,
		    &field->error)) return(False);
        rec->base->info = rec->info;
      }
      if((f = find_struct_field(rec->base->u.structure.fields, name, False,
				NULL)) == NULL){
	field->error = "Couldn't find field";
	return(False);
      }
      f->addr = rec->base->addr + f->offset;
      if(strcmp(name, &rec->name[strlen(rec->name) - strlen(name)]) ||
	 (rec->type != STRING) || (f->type == ArrayType)){
	NEW_TYPE(f->data, f->size, char, long *, "find_field");
	bcopy(((char *) rec->base->data) + f->offset, f->data, f->size);
      }
      else {
	if(!get_val(f, STRING, True, (long *) &f->data, &field->error))
	  return(False);
      }
      rec->base = f;
    }
    else if((rec->base->type == Structure) || (rec->base->type == Union)){
      if(rec->base->info != rec->info){
	if(rec->base->data) free(rec->base->data);
	if(!get_val(rec->base, STRUCTURE, True, (long *) &rec->base->data,
		    &field->error)) return(False);
        rec->base->info = rec->info;
      }
      if((f = find_struct_field(rec->base->u.structure.fields, name, False,
				NULL)) == NULL){
	field->error = "Couldn't find field";
	return(False);
      }
      if(f->type != Pointer){
	field->error = "Field is not a pointer";
	return(False);
      }
      f->addr = rec->base->addr + f->offset;
      NEW_TYPE(f->data, f->size, char, long *, "find_field");
      bcopy(((char *) rec->base->data) + f->offset, f->data, f->size);
      f->u.pointer->addr = *((char **) f->data);
      rec->base = f->u.pointer;
      if(strcmp(name, &rec->name[strlen(rec->name) - strlen(name)]) ||
	 (rec->type != STRING) || (rec->base->type == ArrayType)){
	if(!get_val(rec->base, STRUCTURE, True, (long *) &rec->base->data,
		    &field->error)) return(False);
      }
      else {
	if(!get_val(rec->base, STRING, True, (long *) &rec->base->data,
		    &field->error))
	  return(False);
      }
    }
    else {
      field->error = "Not a structure or pointer";
      return(False);
    }
    break;
  case Index:
    if((rec->base->type != ArrayType) && (rec->base->type != StringType) &&
       (rec->base->type != Pointer)){
      field->error = "Not an array, pointer, or string";
      return(False);
    }
    if(!next_number(name, NULL, &ind)){
      field->error = "Couldn't parse array index";
      return(False);
    }
    if((ind < 0) || ((rec->base->type == ArrayType) &&
		     (ind > rec->base->u.array.size))){
      field->error = "Index out of bounds";
      return(False);
    }
    f = rec->base->u.array.type;
    f->addr = rec->base->addr + (ind * f->size);
    if(strcmp(name, &rec->name[strlen(rec->name) - strlen(name)]) ||
       (rec->type != STRING) || (f->type == ArrayType)){
      NEW_TYPE(f->data, f->size, char, long *, "read_val");
      f->offset = ind * f->size;
      bcopy(((char *) rec->base->data) + f->offset, f->data, f->size);
    }
    else {
      if(!get_val(f, STRING, True, (long *) &f->data, &field->error))
	return(False);
    }
    rec->base = f;
    break;
  default:
    fprintf(stderr, "read_val - Bad type : %d\n", type);
    return(False);
  }
  return(True);
}

Boolean read_field_vals(DataStruct data, FieldRec *fields, int nfields)
{
  static int info = 0;
  ReadValRec rec;
  Boolean ret;
  int i;

  if(check_progress()){
    if(data->data) free(data->data);
    if(!get_val(data, STRUCTURE, True, (long *) &data->data, &fields[0].error))
      return(False);
    ret = True;
    rec.info = info++;
    for(i=0;i<nfields;i++){
      rec.base = data;
      rec.type = fields[i].type;
      rec.name = copy(fields[i].name, -1);
      ret = walk_field(read_val, (void *) &rec, &fields[i]);
      free(rec.name);
      if(!ret) break;
      if(fields[i].type == NUMBER){
	if(rec.base->size > sizeof(fields[i].data)){
	  fields[i].error = copy("Data too large for field", -1);
	  ret = False;
	}
	else {
	  bcopy(rec.base->data, &fields[i].data, rec.base->size);

	  /*
	  ** It dealing with a bitstring data element, extract the string from
	  ** the surrounding bits prior to assigning the value.
	  */
	  if (rec.base->type == BitString) {
	    fields[i].data = (caddr_t) (((int) fields[i].data) >> (rec.base->bit_offset % 8));
	    fields[i].data = (caddr_t) (((int) fields[i].data) & ((1 << rec.base->bit_size) - 1));
	    }
	  }
      }
      else if(fields[i].type == STRING){
	fields[i].data = (caddr_t) rec.base->data;
      }
      else fields[i].data = (caddr_t) rec.base;
    }
  }
  else ret = False;
  return(ret);

}

static void read_offset(unsigned int *buf, int offset, int size, caddr_t ptr)
{
  bcopy(&buf[offset], ptr, size);
}

static Boolean check_fields1(char *symbol, FieldRec *fields, int nfields,
			     char **hints, DataStruct *type_ret)
{
  DataStruct new;
  DataStructArray *done, *not_done;

  new = whatis(symbol, fields, nfields, hints, True);
  if(type_ret) *type_ret = new;
  if(new == NULL) return(False);  
  done = DataStructArrayNew(NULL);
  not_done = DataStructArrayNew(NULL);
  DataStructArrayAppend(not_done, &new);
  while(DataStructArraySize(not_done) != 0){
    new = *DataStructArrayFirstElement(not_done);
    lookup_unknown_structs(new, done, not_done);
    DataStructArrayDelete(not_done, 0);
  }
  DataStructArrayDestroy(done);
  DataStructArrayDestroy(not_done);
  return(True);
}

Boolean check_fields(char *symbol, FieldRec *fields, int nfields,
			    char **hints)
{
  Boolean ret;
  
  if(check_progress()){
    ret = check_fields1(symbol, fields, nfields, hints, NULL);
    in_progress = False;
    return(ret);
  }
  else return(False);
}

void field_errors(FieldRec *fields, int nfields)
{
  int i;

  for(i=0;i<nfields;i++){
    if(fields[i].error != (caddr_t) NULL){
      fprintf(stderr, "field %s: %s\n", fields[i].name, fields[i].error);
    }
  }
}

DataStruct array_element(DataStruct sym, int i, char **error)
{
  DataStruct ret;

  if((sym->type != ArrayType) && (sym->type != Pointer)){
    if(error) *error = "Symbol is not an array or pointer";
    return(NULL);
  }
  if(sym->type == ArrayType){
    if((i < 0) || (i > sym->u.array.size)){
      if(error) *error = "Index out of bounds";
      return(NULL);
    }
    NEW_TYPE(ret, 1, DataStructRec, DataStruct, "array_element");
    *ret = *sym->u.array.type;
    ret->addr = sym->addr + i * ret->size;
    if(ret->data == NULL){
      if(!get_val(sym, ARRAY, True, (long *) &sym->data, error)) return(NULL);
    }
    NEW_TYPE(ret->data, ret->size, char, long *, "array_element");
    memcpy(ret->data, &((char *) sym->data)[i * ret->size], ret->size);
  }
  else if(sym->type == Pointer){
    ret = sym->u.pointer;
    if(sym->data == NULL){
      if(!get_val(sym, NUMBER, True, (long *) &sym->data, error)) return(NULL);
    }
    ret->addr = *((char **) sym->data) + i * ret->size;
  }
  return(ret);
}

#define EQUIV_NUMBER(t) (((t) == Number) || ((t) == Pointer))

Boolean array_element_val(DataStruct sym, int i, long *ele_ret, char **error)
{
  DataStruct ret;
  int size;
  char *addr;

  if((sym->type != ArrayType) && (sym->type != Pointer)){
    if(error) *error = "Symbol is not an array or pointer";
    return(False);
  }
  if((sym->type == ArrayType) && ((i < 0) || (i > sym->u.array.size))){
    if(error) *error = "Index out of bounds";
    return(False);
  }
  if(((sym->type == ArrayType) && !EQUIV_NUMBER(sym->u.array.type->type)) ||
     ((sym->type == Pointer) && !EQUIV_NUMBER(sym->u.pointer->type))){
    if(error) *error = "Element is not a number";
    return(False);
  }
  if(sym->type == ArrayType) size = sym->size/sym->u.array.size;
  else size = sym->u.pointer->size;
  if(size > sizeof(ele_ret)){
    fprintf(stderr, "size = %d\tele_size = %d\n", size, sizeof(ele_ret));
    if(error) *error = "Element is too large";
    return(False);
  }
  if(sym->type == ArrayType){
    if(sym->data == NULL){
      if(!get_val(sym, ARRAY, True, (long *) &sym->data, error)) return(False);
    }
    bcopy(&((char *) sym->data)[i * size], ele_ret, size);
  }
  else {
    addr = sym->u.pointer->addr;
    sym->u.pointer->addr += i * size;
    if(!get_val(sym->u.pointer, NUMBER, False, ele_ret, error)) return(False);
    sym->u.pointer->addr = addr;
  }
  return(True);
}

Boolean set_array_element_size(DataStruct sym, int size, char **error)
{
  if(sym->type == ArrayType){
    sym->u.array.size = size;
    sym->size = size * sym->u.array.type->size;
  }
  else if(sym->type == Pointer){
    sym->u.pointer->size = size;
  }
  else {
    if(error) *error = "Symbol must be array or pointer";
    return(False);
  }
  return(True);
}

unsigned int array_size(DataStruct sym, char **error)
{
  if(sym->type != ArrayType){
    if(error) *error = "Symbol is not an array";
    return(-1);
  }
  return(sym->u.array.size);
}

int field_offset(char *type, char *field, char **error)
{
  DataStructRec sym;
  Field *f;
  int index;

  if(!search_tab(type, &sym)){
    if(error) *error = "Couldn't look up type";
    return(-1);
  }
  if((sym.type != Structure) && (sym.type != Union)){
    if(error) *error = "type is not struct or union";
    return(-1);
  }
  if(find_struct_field(sym.u.structure.fields, field, False, &index) == NULL){
    if(error) *error = "field not found";
    return(-1);
  }
  f = FieldArrayElement(sym.u.structure.fields, index);
  if(f->data->offset == -1){
    if(error) *error = "Offset of field is unknown";
    return(-1);
  }
  return(f->data->offset);
}

unsigned int sizeof_type(char *type, char **error)
{
  DataStructRec sym;
  char buf[256], *resp;
  long size;

  if(!search_tab(type, &sym)){	
    sprintf(buf, "print sizeof(%s)", type);
    dbx(buf, True);
    resp = read_response_status();
    size = 0;
    if(!next_number(resp, NULL, &size)){
      if(error) *error = "Couldn't parse output from dbx sizeof\n";
      return(0);
    }
    return(size);
  }
  else return(sym.size);
}

Boolean set_array_size(DataStruct sym, int size, int ele_size, char **error)
{
  DataStruct type;
  char *addr;

  if(sym->type != Pointer){
    if(error) *error = "set_array_size only implemented for pointers";
    return(False);
  }
  type = sym->u.pointer;
  addr = *((char **) sym->data);
  sym->type = ArrayType;
  sym->addr = addr;
  sym->size = size;
  sym->u.array.size = size/ele_size;
  sym->u.array.type = type;
  if(!get_val(sym, NUMBER, True, (long *) &sym->data, error)) return(False);
  return(True);
}

typedef struct {
  char *name;
  long addr;
} ProcCache;

static ProcCache proc_cache[100];

static int cache_size = 0;

static char *search_cache(long addr)
{
  ProcCache save;
  int i;

  for(i=0;i<cache_size;i++){
    if(addr == proc_cache[i].addr){
      save = proc_cache[i];
      memmove(&proc_cache[1], &proc_cache[0], i * sizeof(proc_cache[0]));
      proc_cache[0] = save;
      return(copy(proc_cache[i].name, -1));
    }
  }
  return(NULL);
}

static void add_cache(long addr, char *name)
{
  int size;
  
  if(cache_size < sizeof(proc_cache)/sizeof(proc_cache[0]))
    size = cache_size++;
  else {
    size = cache_size - 1;
    free(proc_cache[size].name);
  }
  memmove(&proc_cache[1], &proc_cache[0], size * sizeof(proc_cache[0]));
  proc_cache[0].addr = addr;
  proc_cache[0].name = copy(name, -1);  
}

char *addr_to_proc(long addr)
{
  char *resp, *resp1, *token, *ptr, *ret, buf[128];
  int len;
  long proc_addr;

  if(ret = search_cache(addr)) return(ret);
  ret = NULL;
  if(check_progress()){
    printf("dbx \"0x%p/i\"\n", addr);
    fflush(stdout);
    resp = read_response_status();
    token = next_token(resp, &len, NULL);
    if(((token[0] == '[') || (token[2] == '[')) && (token[len - 1] == ',')){
      token[len - 1] = '\0';
      if(token[0] == '[') token++;
      else token += 3;
      if((ptr = index(token, ':')) != NULL) *ptr = '\0';
      printf("dbx \"print &%s\"\n", token);
      fflush(stdout);
      resp1 = read_response_status();
      proc_addr = strtoul(resp1, &ptr, 16);
      if(ptr != resp1){
	if(proc_addr == addr) ret = copy(token, -1);
	else {
	  sprintf(buf, "%s+0x%x", token, addr - proc_addr);
	  ret = copy(buf, -1);
	}
	free(resp1);
      }
    }
    free(resp);
  }
  if(!ret){
    sprintf(buf, "0x%p", addr);
    ret = copy(buf, -1);
  }
  add_cache(addr, ret);
  return(ret);
}

Boolean cast(long addr, char *type, DataStruct *ret_type, char **error)
{
  DataStruct new;

  if((new = whatis(type, NULL, 0, NULL, False)) == NULL){
    if(error) *error = "Couldn't get type";
    return(False);
  }
  new->addr = (char *) addr;
  *ret_type = new;
  return(True);
}

char *struct_addr(DataStruct data)
{
  return(data->addr);
}

DataStruct deref_pointer(DataStruct data)
{
  if(data->type != Pointer){
    fprintf(stderr, "deref_pointer - argument not a pointer\n");
    return(NULL);
  }
  if((data->data == NULL) && !get_val(data, STRUCTURE, True,
				      (long *) &data->data, NULL))
    return(NULL);
  data->u.pointer->addr = *((void **) data->data);
  return(data->u.pointer);
}

DataStruct get_field(DataStruct sym, char *field, char **error)
{
  DataStruct new;
  Field *f;
  int index;
  
  if((sym->type != Structure) && (sym->type != Union)){
    if(error) *error = "type is not struct or union";
    return(NULL);
  }
  if(sym->u.structure.fields == NULL){
    if(!search_tab(sym->u.structure.signature, sym)){
      new = whatis(sym->u.structure.signature, NULL, 0, NULL, True);
      new->addr = sym->addr;
      new->offset = sym->offset;
      new->data = sym->data;
      sym = new;
      if(!sym) return(NULL);
    }    
  }
  if(find_struct_field(sym->u.structure.fields, field, False, &index) == NULL){
    if(error) *error = "field not found";
    return(NULL);
  }
  f = FieldArrayElement(sym->u.structure.fields, index);
  f->data->addr = sym->addr + f->data->offset;
  return(f->data);
}

char *struct_signature(DataStruct sym)
{
  return(sym->u.structure.signature);
}

int sym_type(DataStruct sym)
{
  if(sym->type == Pointer) return(POINTER);
  else if((sym->type == Structure) || (sym->type == Union)) return(STRUCTURE);
  else if(sym->type == ArrayType) return(ARRAY);
  else if(sym->type == Number) return(NUMBER);
  else fprintf(stderr, "sym_type : Bad symbol : 0x%lx\n", sym);
  return(0);
}

void new_proc(char *args, char **output_ret)
{
  char *resp;

  printf("proc %s\n", args);
  fflush(stdout);
  resp = read_response_status();
  if(output_ret) *output_ret = resp;
  else free(resp);
}

#define FILL_ERROR_RETURN_FALSE(var, error) { \
  if(var) *var = copy(error, -1); \
  return(False); \
}

static Boolean check_list_params(char *type, char *field, char **error)
{
  DataStructRec sym;
  DataStruct f;

  if(!search_tab(type, &sym))
    FILL_ERROR_RETURN_FALSE(error, "Couldn't lookup type");
  if(sym.type != Structure)
    FILL_ERROR_RETURN_FALSE(error, "Type isn't a structure");
  if((f = find_struct_field(sym.u.structure.fields, field, False, NULL))
     == NULL)
    FILL_ERROR_RETURN_FALSE(error, "Couldn't find field");
  if((f->type != Pointer) || (f->u.pointer->type != Structure) ||
     strcmp(f->u.pointer->u.structure.signature, sym.u.structure.signature))
    FILL_ERROR_RETURN_FALSE(error,
			    "Field is not a pointer to original structure");
  return(True);
}

Boolean list_nth_cell(long addr, char *type, int n, char *next_field,
		      Boolean do_check, long *val_ret, char **error)
{
  char buf[200], *ptr, *resp;
  long val;
  int field_len, base_len, incr, i, remain;
  Boolean ret;

  if(do_check && !check_list_params(type, next_field, error)) return(False);
  field_len = strlen(next_field) + 1;
  if(strlen(type) + strlen("(long) (() 0xffffffffffffffff)") + field_len >=
     sizeof(buf))
    FILL_ERROR_RETURN_FALSE(error,
			    "Type and field name together are too long");
  if(!val_ret) return(True);
  sprintf(buf, "(long) ((%s) 0x%p)", type, addr);
  base_len = strlen(buf);
  incr = (sizeof(buf) - base_len - 1)/field_len;
  ptr = &buf[base_len];
  for(i=0;i<incr;i++){
    strcpy(ptr, ".");
    strcpy(&ptr[1], next_field);
    ptr += field_len;
  }
  for(i=0;i<n/incr;i++){
    printf("dbx \"print %s\"\n", buf);
    fflush(stdout);
    resp = read_response_status();
    ret = next_number(resp, NULL, &val);
    addr = val;
    sprintf(&buf[base_len - 19], "0x%p", addr);
    buf[base_len - 1] = ')';
    free(resp);
    if(!ret) FILL_ERROR_RETURN_FALSE(error, "Couldn't parse address");
  }
  ptr = &buf[base_len];
  remain = n % incr;
  if(remain > 0){
    ptr += remain * (field_len + 1);
    *(ptr - 1) = '\0';
    printf("dbx \"print %s\"\n", buf);
    fflush(stdout);
    resp = read_response_status();
    ret = next_number(resp, NULL, &val);
    addr = val;
    free(resp);
    if(!ret) FILL_ERROR_RETURN_FALSE(error, "Couldn't parse address");    
  }
  *val_ret = addr;
  return(True);
}

static Boolean list_walk(void *addr, char *type, char *next_field,
			 void **val_ret, char **error, IntArray *rec)
{
  char buf[200];
  char *resp;
  long val;
  Boolean ret;
  int i;

  if(!check_list_params(type, next_field, error)) return(False);
  if(rec == NULL) rec = IntArrayNew(NULL);
  if(IntArraySize(rec) == 0){
    sprintf(buf, "((%s) 0x%p)", type, addr);
    for(i=0;i<10;i++){
      strcat(buf, ".");
      strcat(buf, next_field);
      printf("dbx \"print %s\"\n", buf);
      resp = read_response_status();
      ret = next_number(resp, NULL, &val);
      addr = (void *) val;
      free(resp);
      if(!ret) FILL_ERROR_RETURN_FALSE(error, "Couldn't parse address");
      if(addr == 0) break;
      IntArrayAppend(rec, addr);
    }
  }
  *val_ret = (void *) *IntArrayFirstElement(rec);
  IntArrayDelete(rec, 0);
  return(True);
}

void check_args(int argc, char **argv, char *help_string)
{
  if(argc > 1){
    if(!strcmp(argv[1], "-help")){
      printf("%s\n", help_string);
      exit(0);
    }
    else if(!strcmp(argv[1], "-version-check")){
      exit(0);
    }
  }
}

Boolean to_number(char *str, long *val)
{
  char *ptr, buf[256], *resp;

  if(!val) return(False);
  *val = strtoul(str, &ptr, 0);
  if(*ptr != '\0'){
    sprintf(buf, "p %s", str);
    dbx(buf, True);
    if((resp = read_response_status()) == NULL){
      fprintf(stderr, "Couldn't read %s:\n", str);
      return(False);
    }
    if(!next_number(resp, NULL, val)){
      fprintf(stderr, "Couldn't read %s as a number:\n", str);
      return(False);
    }
    free(resp);
  }
  return(True);
}

/* This routine returns the memory contents of the nbytes bytes starting
 * at the address, start_addr.
 * It can be used to look up any types of values pointed at by a pointer.
 * The function returns True (1) on success and False (0) on failure.
 */
Boolean read_memory(long start_addr, int nbytes, char *buf, char **error)
{
  int nlongwords;
  char *resp;
  long *mem;

  if((! start_addr) || (! nbytes)) {
    *error = "Null starting address or zero byte count";
    return(False);
  }
  nlongwords = (nbytes)/sizeof(long);
  if (nbytes % sizeof(long)) nlongwords++;
  input_consumed=False;
  printf("dbx \"0x%p/%dX\"\n", start_addr, nlongwords);
  fflush(stdout);
  resp = read_response_status();
  if((mem = parse_memory(resp, 0, nlongwords)) == NULL){
    if(error) *error = "Couldn't parse memory output";
    /* the buffer space was freed in parse_memory() when failed */
    return(False);
  }
  free(resp);
  bcopy(mem, buf, nbytes); /* copy the result into user's buffer */
  free(mem);
  return(True);
}

#ifdef __alpha
#define VIRT_BASE (0xffffffff00000000)
#define KSEG_BASE (0xfffffc0000000000)
#define USER_BASE (0x0000000100000000)

long to_address(char *addr, char **error)
{
  long n;
  char *ptr, c;

  switch(*addr){
  case 'v':
  case 'k':
  case 'u':
    c = *addr;
    break;
  default:
    c = '\0';
    break;
  }
  if(c) addr++;
  n = strtoul(addr, &ptr, 0);
  if(*ptr != '\0'){
    if(error) *error = "Couldn't parse as an address";
    return(0);
  }
  switch(c){
  case 'v':
    n += VIRT_BASE;
    break;
  case 'k':
    n += KSEG_BASE;
    break;
  case 'u':
    n += USER_BASE;
    break;
  }
  if(error) *error = NULL;
  return(n);
}

char *format_addr(long addr, char *buffer)
{
  if(addr == 0) sprintf(buffer, "NULL");
  else if(IS_SEG1_VA(addr)) sprintf(buffer, "v0x%08x", addr);
  else if(IS_KSEG_VA(addr)) sprintf(buffer, "k0x%08x", addr);
  else if(IS_SEG0_VA(addr)) sprintf(buffer, "u0x%08x", addr);
  else sprintf(buffer, "?0x%08p", addr);
  return(buffer);
}
#endif

#ifdef __mips

long to_address(char *addr, char **error)
{
  long n;
  char *ptr;

  n = strtoul(addr, &ptr, 0);
  if(*ptr != '\0'){
    if(error) *error = "Couldn't parse as an address";
    return(0);
  }
  if(error) *error = NULL;
  return(n);
}

#endif

void data_symbol(int nargs, char **syms_ret, ...)
{
  va_list ap;
  char *resp, *ptr, *end;
  long addr;
  int i;

  if(check_progress()){
    va_start(ap, syms_ret);
    if(nargs > 0){
      printf("data_symbol ");
      for(i=0;i<nargs;i++){
	addr = va_arg(ap, long);
	printf("0x%p ", addr);
      }
      printf("\n");
      fflush(stdout);
      resp = read_response_status();
      ptr = resp;
      i = 0;
      while(*ptr != '\0'){
	end = index(ptr, '\n');
	if(ptr == end) syms_ret[i] = NULL;
	else syms_ret[i] = copy(ptr, end - ptr);
	ptr = end + 1;
      }
      free(resp);
    }
    va_end(syms_ret);
  }
}

static char *check_kernel_cache(long addr)
{
  static AddrArray *procs = NULL;
  static FieldRec proc_field = { ".p_nxt", NUMBER, NULL, NULL };
  DataStruct proc;
  long procaddr, a;
  int i;
  char *error, buf[256];

  if(procs == NULL){
    procs = AddrArrayNew(NULL);
    if(!check_fields("struct proc", &proc_field, 1, NULL)){
      field_errors(&proc_field, 1);
      return(NULL);
    }
    if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
      fprintf(stderr, "Couldn't read allproc:\n");
      fprintf(stderr, "%s\n", error);
      return(NULL);
    }
    do {
      if(!cast(procaddr, "struct proc", &proc, &error)){
	fprintf(stderr, "Couldn't cast addr to a proc:\n");
	fprintf(stderr, "%s\n", error);
	return(NULL);
      }      
      if(!read_field_vals(proc, &proc_field, 1)){
	field_errors(&proc_field, 1);
	return(NULL);
      }
      AddrArrayAppend(procs, &procaddr);
      procaddr = (long) proc_field.data;
    } while(procaddr != 0);
  }
  AddrArrayIter(procs, a, i){
    if(a == addr){
      sprintf(buf, "proc[%d]", i);
      return(copy(buf, -1));
    }
  }
  return(NULL);
}

char *addr_symbol(long addr)
{
  char *sym, buf[12];
  
  if(addr == 0) sym = NULL;
  else data_symbol(1, &sym, addr);
  if(!sym) sym = check_kernel_cache(addr);
  if(!sym){
    format_addr(addr, buf);
    return(copy(buf, -1));
  }
  else return(sym);
}
