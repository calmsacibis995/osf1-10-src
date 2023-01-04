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
 * @(#)$RCSfile: krash_p.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/04/23 18:43:18 $
 */
#include <setjmp.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include "vdComResPkt.h"
#include "util.h"
#include "array.h"
#include "krash_arrays.h"

#define SEMICOLON 1
#define PIPE 2
#define DOUBLE_STRING 3
#define SINGLE_STRING 4
#define BACK_QUOTE 5
#define WHITESPACE 6
#define ID 7
#define COMMENT 8
#define ERROR 9

#define COMMAND_HELP 1
#define ALIAS_HELP 2
#define EXTENSION_HELP 3
#define NO_HELP 4

#define MAX_BUF ((TTYHOG > PIPE_BUF) ? TTYHOG : PIPE_BUF)
#define BUF_SIZE(tty) (tty ? TTYHOG : PIPE_BUF)
#define HUGE_BUF (1 << ((sizeof(int) * NBBY) - 2))

#define STRING_END(string) (&string[strlen(string)])

#define BIT_INDEX(n) ((n)/(sizeof(unsigned int) * NBBY))

#define BIT_POS(n) ((n) % (sizeof(unsigned int) * NBBY))

#define BIT_ALLOC(var, n) \
{ \
  int bit_index; \
\
  NEW_TYPE((var), BIT_INDEX(n) + 1, unsigned int, unsigned int *, \
	   "BIT_ALLOC"); \
  for(bit_index=0;bit_index<=BIT_INDEX(n);bit_index++) (var)[bit_index] = 0; \
}

#define BIT_FREE(bits) free(bits)

#define BIT_SET(bits, n) (bits[BIT_INDEX(n)] |= (1 << BIT_POS(n)))

#define BIT_CLEAR(bits, n) (bits[BIT_INDEX(n)] &= ~(1 << BIT_POS(n)))

#define BIT_CHECK(bits, n) (bits[BIT_INDEX(n)] & (1 << BIT_POS(n)))

#define BIT_TOGGLE(bits, n) \
  (BIT_CHECK(bits, n) ? BIT_CLEAR(bits, n) : BIT_SET(bits, n))

#define BIT_SAME(bits1, bits2, n) \
  (!bcmp((bits1), (bits2), (BIT_INDEX(n) + 1) * sizeof(unsigned int)))

typedef union {
  CommandResponsePkt packet;
  char text[sizeof(CommandResponsePkt)];
} db_output;

typedef struct _AliasRec {
  char *cmd;
  TokenArray *args;
} AliasRec;

typedef enum { String, Packet, Ready } OutType;

typedef enum { UserContext, ProcContext } ContextType;

typedef struct {
  AliasArray *aliases;
} VisContext;

typedef struct {
  caddr_t data;
  int len;
} Data;

typedef struct _Token {
  int type;
  char *val;
} Token;

typedef enum { NoOutput, FileOutput, CallbackOutput } IOOutType;
typedef enum { NoInput, ImmediateInput, FileInput, CallbackInput } IOInType;

typedef struct _IORec {
  struct _Context *context;
  Boolean output_to_user;
  Boolean free_context;
  struct {
    IOOutType type;
    Boolean (*proc)(struct _IORec *, char *);
    int buf_size;
    union {
      struct {
	FILE *fp;
	Boolean append_prompt;
      } file;
      void *arg;
    } u;
  } out_part;
  struct {
    IOInType type;
    Boolean (*block_proc)(struct _IORec *, char *, int);
    Boolean (*line_proc)(struct _IORec *, char **);
    Boolean interpret;
    int buf_size;
    union {
      struct {
	char *data;
	char *ptr;
      } immediate;
      struct {
	FILE *fp;
	Boolean buffered;
      } file;
      void *arg;
    } u;
  } in_part;
} IORec;

typedef struct _Context {
  VisContext vis;
  FILE *dbx_fp;
  IORec in_rec;
  IORec out_rec;
  IORec print_rec;
  Boolean debug_recurse;
  Boolean debug;
  Boolean redirect_output;
  void (*prompt_proc)(void *);
  VisContext *user, *current;
  struct _Context *parent;
  int pid;
  Boolean close_on_end;
  Boolean close_with_eof;
  int level;
  int sub_level;
  Boolean prompt_after;
#ifdef notdef
  DataArray *unsent;
#endif
  jmp_buf jmp;
} Context;

typedef struct {
  char *command;
  Boolean (*proc)(TokenArray *, Context *, IORec *);
  Boolean copy_context;
  Boolean user_input;
  char *help;
} ComTable;

typedef struct _PidRec {
  pid_t pid;
  IOArray *array;
  int index;
} PidRec;

typedef struct {
  char *in;
  char *out;
} PipeRec;

typedef struct _CleanupRec {
  void (*proc)(void *);
  void *arg;
} CleanupRec;

typedef struct {
  Boolean done;
  Boolean did_head;
  Boolean read_more;
  StringArray *exe_done;
  char *ext_path_save;
  char *ext_path;
  DIR *dirp; 
} ExtHelp;

typedef struct {
  Boolean found_topic;
  Boolean first;
  int where;
  Context *context;
  char *subject;
  Boolean long_help;
  IORec io;
  ExtHelp ext;
} HelpRec;

typedef struct {
  Context *context;
  void (*prompt_proc)(void *);
  IOArray *ios;
} CleanParse;

extern Boolean dbx_dirty;
extern Boolean dbx_up;

extern char *copy(char *ptr, int len);
extern void new_cleanup_level(void);
extern int add_cleanup(void (*proc)(void *), void *arg);
extern void do_cleanups(void);
extern void delete_cleanup(int cleanup, Boolean do_cleanup);

extern void set_string_in(IORec *io, char *buf);
#ifdef notdef
extern void set_string_out(IORec *io, char *buf);
#endif
extern Boolean read_string_block(IORec *io, char *ret, int size);
extern Boolean read_string_line(IORec *io, char **ret);
extern Boolean append_string(IORec *io, char *str);

extern void set_null_in(IORec *io);
extern void set_null_out(IORec *io);
extern Boolean read_null_block(IORec *io, char *ret, int size);
extern Boolean read_null_line(IORec *io, char **ret);
extern Boolean write_null(IORec *io, char *str);

extern void set_text_in(IORec *io, FILE *fp, Boolean interpret, Boolean tty);
extern void set_text_out(IORec *io, FILE *fp, Boolean tty);
extern Boolean read_file_block(IORec *io, char *ret, int size);
extern Boolean read_file_line(IORec *io, char **ret);
extern Boolean write_file(IORec *io, char *str);
extern void prompt(void *arg);

extern void set_packet_in(IORec *io, FILE *fp, Boolean tty);
extern void set_packet_out(IORec *io, FILE *fp, Boolean tty);
extern Boolean read_packet(IORec *io, char *ret, int size);
extern Boolean write_packet(IORec *io, char *str);

extern void set_cb_out(IORec *io, IOOutType type,
		       Boolean (*proc)(struct _IORec *, char *),
		       void *arg);
extern void set_cb_in(IORec *io,
		      IOInType type,
		      Boolean (*block_proc)(struct _IORec *, char *, int),
		      Boolean (*line_proc)(struct _IORec *, char **),
		      Boolean interpret, void *arg);
extern Boolean write_string(IORec *io, char *str);
extern Boolean write_nothing(IORec *io, char *str);

extern char *yylval;
extern Boolean dbx_dirty;
extern Boolean dbx_up;
extern Boolean terminal;
extern struct sgttyb ttybsave;
extern struct sgttyb ttybnew;
extern Boolean prompting;
extern int dbx_pid;
extern jmp_buf current_jmp;
extern struct winsize win;
