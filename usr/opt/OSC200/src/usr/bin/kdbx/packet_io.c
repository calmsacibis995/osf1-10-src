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
static char *rcsid = "@(#)$RCSfile: packet_io.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/04/23 18:43:23 $";
#endif
#include <errno.h>
#include "krash_p.h"

static IORec packet_rec = { NULL, False, False, 
			      { FileOutput, write_packet, { NULL } },
			      { FileInput, read_packet, NULL, False } };

void set_packet_in(IORec *io, FILE *fp, Boolean tty)
{
  io->in_part = packet_rec.in_part;
  io->in_part.u.file.fp = fp;
  io->in_part.u.file.buffered = False;
  io->in_part.buf_size = BUF_SIZE(tty);
}

void set_packet_out(IORec *io, FILE *fp, Boolean tty)
{
  io->out_part = packet_rec.out_part;
  io->out_part.u.file.fp = fp;
  io->out_part.buf_size = BUF_SIZE(tty);
}

static CommandResponsePkt *compact(CommandResponsePkt *packet)
{
  static Boolean start_over = True;
  static CommandResponsePkt save = { CRPMAGIC, x_null };

  if((packet->command != x_prompt) && (dbx_up == True)){
    if(start_over){
      save = *packet;
      start_over = False;
      return(NULL);
    }
    else if((packet->command == x_unknown) || (packet->command == x_debugger)){
      if((strlen(save.msg) + strlen(packet->msg) < sizeof(save.msg)) &&
	 !strncmp(packet->channel, save.channel, sizeof(save.channel)) &&
	 (packet->stop == save.stop) && (packet->lineno == save.lineno) &&
	 !strncmp(packet->file, save.file, sizeof(save.file))){
	strcat(save.msg, packet->msg);
	start_over = False;
	return(NULL);
      }
      else {
	if(start_over == True) save.command = x_null;
	else start_over = True;
	return(&save);
      }
    }
    else return(NULL);
  }
  else {
    if(start_over == True) save.command = x_null;
    else start_over = True;
    return(&save);
  }
}

static int fill_packet(FILE *fp, db_output *packet, int start)
{
  int remain, n, i;
  char *ptr;

  remain = sizeof(*packet) - start;
  ptr = &packet->text[start];
  while(remain != 0){
    if(ptr + remain > &packet->text[sizeof(*packet)])
      fprintf(stderr, "Reading past end of out\n");
    n = read(fileno(fp), ptr, remain);
    if(n < 1) break;
    remain -= n;
    ptr += n;
  }
  n = sizeof(*packet) - start;
  ptr = &packet->text[start];
  for(i=0;i<n;i++){
    if(!strncmp(ptr + i, "\r\n", 2) || !strncmp(ptr + i, "\n\n", 2)){
      bcopy(ptr + i + 1, ptr + i, n - i - 1);
      n--;
    }
  }
  return(n);
}

static char *find_magic(char *buf, int len)
{
  int magic, i;

  magic = CRPMAGIC;
  for(i=0;i<len;i++){
    if(!bcmp(&buf[i], &magic, sizeof(int))) return(&buf[i]);
  }
  return(NULL);
}

Boolean read_packet(IORec *io, char *ret, int size)
{
  int n, i, text;
  char *ptr, c;
  CommandResponsePkt *packet;
  char *magic;
  FILE *fp;
  static char buffer[2 * sizeof(packet->msg)] = "";
  static Boolean buffer_ret;
  static int remain = sizeof(db_output);
  static db_output out;

  if(size == 0) return(False);
  if(buffer[0] != '\0'){
    n = strlen(buffer);
    if(n > size){
      strncpy(ret, buffer, size);
      ret[size] = '\0';
      strcpy(buffer, &buffer[size]);
    }
    else {
      strcpy(ret, buffer);
      buffer[0] = '\0';
      io->in_part.u.file.buffered = False;
    }
    return(buffer_ret);
  }
  *ret = '\0';
  fp = io->in_part.u.file.fp;
  while(1){
    if(remain != sizeof(db_output)){
      bcopy(&out.text[remain], &out.text[0], sizeof(db_output) - remain);
    }
    else out.packet.magic = 0;
    n = fill_packet(fp, &out, sizeof(db_output) - remain);
    remain = sizeof(db_output);
    if(n < 0){
      if(errno != EINTR){
	perror("Reading from dbx");
	return(False);
      }
    }
    else if(n == 0) return(False);
    if(out.packet.magic == CRPMAGIC){
      if(dbx_up){
	if((packet = compact(&out.packet)) != NULL){
	  if(packet->command != x_null){
	    if(strlen(packet->msg) > size){
	      strncpy(ret, packet->msg, size);
	      ret[size] = '\0';
	      strcpy(buffer, &packet->msg[size]);
	      io->in_part.u.file.buffered = True;
	    }
	    else strcpy(ret, packet->msg);
	    if(out.packet.command != x_prompt){
	      strcat(buffer, out.packet.msg);
	      io->in_part.u.file.buffered = True;
	      buffer_ret = True;
	    }
	    else buffer_ret = False;
	    if(buffer[0] != '\0') return(True);
	    else return(False);
	  }	     
	  dbx_up = True;
	  dbx_dirty = False;
	  return(False);	  
	}
      }
      else if(out.packet.command != x_prompt){
	if(strlen(out.packet.msg) > size){
	  strncpy(ret, out.packet.msg, size);
	  ret[size] = '\0';
	  strcpy(buffer, &out.packet.msg[size]);
	}
	else strcpy(ret, out.packet.msg);
	return(True);
      }
      else {
	dbx_up = True;
	dbx_dirty = False;
	return(False);
      }
    }
    else {
      magic = find_magic(out.text, n);
      if(magic){
	text = magic - out.text;
	remain = magic - out.text;
      }
      else {
	text = n;
	remain = sizeof(db_output);
      }
      for(i=0;i<text;i++){
	if(out.text[i] == '\0'){
	  bcopy(&out.text[i+1], &out.text[i], n - i + 1);
	  i--;
	  n--;
	  text--;
	  magic--;
	}	
      }
      if(text > size){
	strncpy(ret, out.text, size);
	ret[size] = '\0';	    
	strncpy(buffer, &out.text[size], text - size);
	buffer[text - size] = '\0';
	buffer_ret = True;
      }
      else {
	strncpy(ret, out.text, text);
	ret[text] = '\0';
      }
      bcopy(magic, &out.text[remain], n - text);
      return(True);
    }
  }
  return(True);
}

Boolean write_packet(io, buf)
IORec *io;
char *buf;
{
  char *ptr;
  int buf_len, len, num;
  FILE *fp;
  CommandResponsePkt p;

  fp = io->out_part.u.file.fp;
  p.magic = CRPMAGIC;
  if(buf == NULL){
    p.command = x_prompt;
    fwrite(&p, sizeof(p), 1, fp);
    if((io->context->close_on_end) && (io->context->level == 1)){
      if(io->context->close_with_eof)
	fprintf(io->out_part.u.file.fp, "%s", "\004");
      else fclose(io->out_part.u.file.fp);
    }
  }
  else {
    ptr = buf;
    p.command = x_debugger;
    buf_len = sizeof(p.msg)/sizeof(p.msg[0]);
    while(1){
      if((len = strlen(ptr)) >= buf_len) num = buf_len;
      else num = len + 1;
      strncpy(p.msg, ptr, num);
      fwrite(&p, sizeof(p), 1, fp);
      if(len < buf_len) break;
      ptr += num;
    }
  }
  fflush(fp);
  return(True);  
}
