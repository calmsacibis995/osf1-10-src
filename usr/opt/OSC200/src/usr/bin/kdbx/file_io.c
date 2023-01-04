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
static char *rcsid = "@(#)$RCSfile: file_io.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/04/23 18:42:58 $";
#endif
#include <errno.h>
#include <sys/signal.h>
#include <sys/fcntl.h>
#include "krash_p.h"

static IORec text_rec = { NULL, False, False, 
			    { FileOutput, write_file, { NULL } },
			    { FileInput, read_file_block, read_file_line, NULL,
				False } };
static Boolean no_page = False;
static int lines_out = 0;
static int columns_out = 0;

static Boolean user_out(IORec *io, char *buf);
static void paginate(FILE *fp);

void set_text_out(IORec *io, FILE *fp, Boolean tty)
{
  io->out_part = text_rec.out_part;
  io->out_part.u.file.fp = fp;
  io->out_part.buf_size = BUF_SIZE(tty);
}

void set_text_in(IORec *io, FILE *fp, Boolean interpret, Boolean tty)
{
  io->in_part = text_rec.in_part;
  io->in_part.interpret = interpret;
  io->in_part.u.file.fp = fp;
  io->in_part.buf_size = BUF_SIZE(tty);
  io->in_part.u.file.buffered = False;
}

Boolean read_file_block(IORec *io, char *ret, int size)
{
  int n;
  Boolean res;

  if(size == 0){
    if(io->in_part.u.file.fp == stdin) return(True);
    if((io->context->level == 1) &&
       (io->in_part.u.file.fp != stdin) &&
       (io->out_part.u.file.fp != io->context->dbx_fp)){
      fclose(io->in_part.u.file.fp);
    }
    return(False);
  }
  if(io->in_part.u.file.buffered){
    io->in_part.u.file.buffered = False;
    *ret = '\0';
    return(True);
  }
  if((n = read(fileno(io->in_part.u.file.fp), ret, size)) <= 0){
    if(io->in_part.u.file.fp != stdin){
      if(n == -1) perror("read_file_block - read failed");
    }
    *ret = '\0';
    res = False;
  }
  else {
    ret[n] = '\0';
    res = True;
  }
  if(io->in_part.u.file.fp == stdin){
    res = True;
    io->in_part.u.file.buffered = True;
  }
  return(res);
}

Boolean read_file_line(IORec *io, char **ret)
{
  char *line, *ptr, buf[256];
  int olen, len;
  FILE *fp;

  fp = io->in_part.u.file.fp;
  line = NULL;
  *ret = NULL;
  while(1){
    do {
      errno = 0;
      if(fgets(buf, sizeof(buf)/sizeof(buf[0]), fp) != NULL) break;
      else if(errno != EINTR){
	if(feof(fp) || ferror(fp)){
	  if(fp == stdin) clearerr(fp);
	  else fclose(fp);
	  set_null_in(io);
	}
	return(False);
      }
    } while(1);
    if(!line){
      len = strlen(buf);
      NEW_TYPE(line, len + 1, char, char *, "read_file_line");
      ptr = line;
    }
    else {
      olen = len;
      len += strlen(buf);
      line = (char *) realloc(line, (len + 1) * sizeof(char));
      ptr = &line[olen];
    }
    *ret = line;
    strcpy(ptr, buf);
    if(line[len - 1] == '\n'){
#ifdef notdef
      line[len - 1] = '\0';
#endif
      if((len < 2) || (line[len - 2] != '\\')) break;
    }
  }
  return(True);
}

Boolean write_file(IORec *io, char *str)
{
  int n;
  char eof = '\004';

  if(str == NULL){
    if((io->context->level == 1) &&
       (io->out_part.u.file.fp != stdout) &&
       (io->out_part.u.file.fp != io->context->dbx_fp) &&
       io->context->close_on_end){
      if(io->context->close_with_eof){
	write(fileno(io->out_part.u.file.fp), &eof, sizeof(eof));
	sleep(1);
      }
      else close(fileno(io->out_part.u.file.fp));
    }
  }
  else if(io->out_part.u.file.fp != stdout){
    if((n = write(fileno(io->out_part.u.file.fp), str, strlen(str))) < 0)
      perror("write_file - write failed");
  }
  else user_out(io, str);
  return(True);
}

void prompt(void *arg){
  prompting = True;
  printf("(kdbx) ");
  fflush(stdout);
  lines_out = 0;
  columns_out = 0;
  no_page = False;
}

static void paginate(FILE *fp)
{
  char c;
  int i;

  if(terminal && !no_page){
    fprintf(fp,
	    "<cr> for next line, q to quit, ^d to stop pagination, <sp> to continue ");
    fflush(fp);
    ttybnew.sg_flags |= CBREAK;
    ttybnew.sg_flags &= ~ECHO;
    (void) ioctl(fileno(fp), TIOCSETP, (char *) &ttybnew);
    read(fileno(fp),&c,1);
    for(i=0;i<72;i++) putchar(010);
    for(i=0;i<72;i++) putchar(' ');
    for(i=0;i<72;i++) putchar(010);
    (void) ioctl(fileno(fp), TIOCSETP, (char *)&ttybsave);	
    (void) ioctl(fileno(fp), TIOCFLUSH, (char *)FREAD);
    switch(c) {
    case '\r':
    case '\n':
      lines_out--;
      break;
    case 'q':
    case 'Q':
      putchar('\n');
      if(dbx_dirty && (dbx_pid > 0)) kill(dbx_pid, SIGINT);
      longjmp(current_jmp, 2);
      /*NOTREACHED*/
    case '\04':
      no_page = True;
      break;
    default:
      lines_out = 0;      
    }
    fflush(fp);
  }
}

static Boolean user_out(IORec *io, char *buf)
{
  char *ptr, *save, c;
  FILE *fp;

  if(buf == NULL) return(True);
  fp = io->out_part.u.file.fp;
  ptr = buf;
  save = buf;
  while(*ptr){
    while(columns_out < win.ws_col){
      if(lines_out >= win.ws_row - 1){
	c = *ptr;
	*ptr = '\0';
	fputs(buf, fp);
	*ptr = c;
	buf = ptr;
	fflush(fp);
	paginate(fp);
      }
      if(*ptr == '\n') break;
      else if(*ptr == '\t') columns_out = (columns_out/8 + 1) * 8;
      else if(*ptr == '\0') break;
      ptr++;
      columns_out++;
    }
    if(*ptr != '\0'){
      columns_out = 0;
      ptr++;
      lines_out++;
    }
  }
  fprintf(fp, "%s", buf);
  fflush(fp);
  return(True);
}

