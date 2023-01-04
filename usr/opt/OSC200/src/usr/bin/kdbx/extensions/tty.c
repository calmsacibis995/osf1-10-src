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
static char *rcsid = "@(#)$RCSfile: tty.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/03/16 19:02:53 $";
#endif
#include <stdio.h>
#define _USE_OLD_TTY
#include <sys/ttydev.h>
#include <sys/termio.h>
#include "krash.h"

static char *help_string =
"tty - print information about a terminal                                 \\\n\
    Usage : tty proc-addr                                                 \\\n\
    proc-addr is the address of a proc structure that is attached to the  \\\n\
        terminal of interest                                              \\\n\
";

FieldRec proc_fields[] = {
  { ".p_pgrp->pg_id", NUMBER, NULL, NULL },
  { "->p_pgrp->pg_session->s_ttyp", STRUCTURE, NULL, NULL }
};

char *hints[] = { NULL, "struct tty" };

#define NUM_PROC_FIELDS (sizeof(proc_fields)/sizeof(proc_fields[0]))

FieldRec tty_fields[] = {
#ifdef notdef
  { ".T_LINEP",
  { ".t_addr",
#endif
  { ".t_termios.c_cc", ARRAY, NULL, NULL },
#ifdef notdef
  { ".t_cflag", NUMBER, NULL, NULL },
  { ".t_cflag_ext", NUMBER, NULL, NULL },
  { ".t_col",
  { ".t_delct",
#endif
  { ".t_dev", NUMBER, NULL, NULL },
  { ".t_flags", NUMBER, NULL, NULL },
#ifdef notdef
  { ".t_iflag",
  { ".t_iflag_ext",
  { ".t_language",
  { ".t_lflag",
  { ".t_lflag_ext",
  { ".t_line",
  { ".t_lk_tty",
  { ".t_oflag",
  { ".t_oflag_ext",
  { ".t_oproc",
  { ".t_poststart",
  { ".t_rsel",
  { ".t_sid",
  { ".t_smp",
#endif
  { ".t_state", NUMBER, NULL, NULL },
  { ".t_termios.c_ispeed", NUMBER, NULL, NULL },
  { ".t_termios.c_ospeed", NUMBER, NULL, NULL },
#ifdef notdef
  { ".t_tpath",
  { ".t_winsize.ws_col",
  { ".t_winsize.ws_row",
  { ".t_winsize.ws_xpixel",
  { ".t_winsize.ws_ypixel",
  { ".t_wsel",
#endif
};

#define NUM_TTY_FIELDS (sizeof(tty_fields)/sizeof(tty_fields[0]))

static char *flags[] = {
	"tandem",	"cbreak", 	"lcase",	"echo",
	"crmod",	"raw",		"oddp",		"evenp",
	"nl1",		"nl2",		"tab1",		"tab2",
	"cr1",		"cr2",		"vtdelay",	"bs1",
	"crtbs",	"prtera",	"crtera",       "tilde",
	"mdmbuf",	"litout",	"tostop",       "flusho",
	"nohang",	"autoflow",	"crtkil",       "pass8",
	"ctlech",	"pendin",	"decctq",       "bnoflsh"
};

static char *state[] = {
	"timeout",	"wopen",	"isopen",	"flush",
	"carr_on",	"busy",		"asleep",	"xclude",
	"ttstop",	"tblock",	"rcoll",	"wcoll",
	"nbio",		"async",	"ondelay",	"bksl",
	"quot",		"erase",	"lnch",		"typen",
	"cnttb",	"igncar",	"closing",	"inuse",
	"lrto",		"ltact",	"isusp",	"oabort",
	"onoctty"
};

char *
getcharrep(c, cb)
unsigned char c;
char *cb;
{
	char *t = cb;
	if(c > 0177) {
		c &= 0x7f;
		*t++ = 'M';
		*t++ = '-';
	}
	if(c < 033) {
		*t++ = '^';
		*t++ = c + '@';
		*t = '\0';
		return(cb);
	}
	if(c < ' ') {
		char *str;
		switch(c) {
			case 033 :
				str = "esc";
				break;
			case 034 :
				str = "fs";
				break;
			case 035 :
				str = "gs";
				break;
			case 036 :
				str = "rs";
				break;
			case 037 :
				str = "rs";
			
		}
		strcpy(t, str);
		return(cb);
	}
	if(c == 0177) {
		*t++ = 'd';
		*t++ = 'e';
		*t++ = 'l';
		*t = '\0';
		return(cb);
	}
	*t++ = c;
	*t = '\0';
	return(cb);
}

char *
getspeed(speed)
	int speed;
{
	switch(speed) {
		case B0 : return("0");
		case B50 : return("50");
		case B75 : return("75");
		case B110 : return("110");
		case B134 : return("134");
		case B150 : return("150");
		case B200 : return("200");
		case B300 : return("300");
		case B600 : return("600");
		case B1200 : return("1200");
		case B1800 : return("1800");
		case B2400 : return("2400");
		case B4800 : return("4800");
		case B9600 : return("9600");
		case EXTA : return("EXTA");
		case EXTB : return("EXTB");
	}
	return("???");
}

static char get_ele(array, i)
DataStruct array;
int i;
{
  char *error, ret;
  long val;

  if(!array_element_val(array, i, &val, &error)){
    fprintf(stderr, "Couldn't read array element:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  ret = val;
  return(ret);
}

static void Usage(){
  fprintf(stderr, "Usage : tty proc-addr\n");
  quit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  int i, j, tty_flags, tty_state, numcnt;
  long next, procaddr;
  DataStruct proc, tty;
  char buf[256], cb1[10], cb2[10], cb3[10], cb4[10], *resp, *ptr, *error;

  check_args(argc, argv, help_string);
  if(argc != 2) Usage();
  if((procaddr = to_address(argv[1], &error)) == 0){
    if(!read_sym_val(argv[1], NUMBER, &procaddr, &error)){
      fprintf(stderr, "Couldn't read %s:\n", argv[1]);
      fprintf(stderr, "%s\n", error);
      Usage();
    }
  }
  if(!check_fields("struct proc", proc_fields, NUM_PROC_FIELDS, hints)){
    field_errors(proc_fields, NUM_PROC_FIELDS);
    quit(1);
  }
  if(!check_fields("struct tty", tty_fields, NUM_TTY_FIELDS, NULL)){
    field_errors(tty_fields, NUM_TTY_FIELDS);
    quit(1);
  }
  if(!cast(procaddr, "struct proc", &proc, &error)){
    fprintf(stderr, "Couldn't cast procslot to a proc:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  if(!read_field_vals(proc, proc_fields, NUM_PROC_FIELDS)){
    field_errors(proc_fields, 2);
    quit(1);
  }
  tty = (DataStruct) proc_fields[1].data;
  if(struct_addr(tty) == 0){
    sprintf(buf, "No tty for proc 0x%p\n", procaddr);
    quit(1);
  }
  if(!read_field_vals(tty, tty_fields, NUM_TTY_FIELDS)){
    field_errors(tty_fields, NUM_TTY_FIELDS);
    quit(1);
  }
  sprintf(buf, "TTY Structure: address 0x%p", struct_addr(tty));
  print(buf);
  sprintf(buf, "dev (%d, %d) pgrp %d ispeed %d ospeed %d",
	  major((int) tty_fields[1].data), minor((int) tty_fields[1].data),
	  proc_fields[0].data, tty_fields[4].data, tty_fields[5].data);
  print(buf);
  tty_flags = (int) tty_fields[2].data;
  sprintf(buf, "flags: \(0x%p\) ", tty_flags);
  print(buf);
  buf[0] = '\0';
  for(i=0;i<(sizeof(flags)/sizeof(char *));i++){
    if(tty_flags & (1 << i)) {
      if(((numcnt++ % 5) == 0) && (numcnt > 1)) print(buf);
      sprintf(&buf[strlen(buf)], "\t%s\t", flags[i]);
    }
  }
  if(--numcnt % 5) print("");
  tty_state = (int) tty_fields[3].data;
  sprintf(buf, "state: \(0x%p\) ", tty_state);
  print(buf);
  numcnt = 0;
  for(i=0;i<(sizeof(state)/sizeof(char *));i++) {
    if(tty_state & (1 << i)) {
      if(((numcnt++ % 5) == 0) && (numcnt > 1)) print(buf);
      sprintf(&buf[strlen(buf)], "\t%s\t", state[i]);
    }
  }
  if(--numcnt % 5) print("");
  print("Control characters:");
  sprintf(buf, "\tintr '%s' \tquit '%s' \terase '%s' \tkill '%s'",
	  getcharrep((u_char)get_ele(tty_fields[0].data, VINTR), cb1),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VQUIT), cb2),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VERASE), cb3),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VKILL), cb4));
  print(buf);
  sprintf(buf, "\teof '%s' \teol '%s' \teol2 '%s' \tswtch '%s'",
	  getcharrep((u_char)get_ele(tty_fields[0].data, VEOF), cb1),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VEOL), cb2),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VEOL2), cb3),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VSWTCH), cb4));
  print(buf);
  sprintf(buf, "\tmin  %d  \ttime  %d \tstart '%s' \tstop '%s'",
	  get_ele(tty_fields[0].data, VMIN),
	  get_ele(tty_fields[0].data, VTIME),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VSTART), cb1),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VSTOP), cb2));
  print(buf);
  sprintf(buf, "\tsusp '%s' \tdsusp '%s' \trprnt '%s' \tflush '%s'",
	  getcharrep((u_char)get_ele(tty_fields[0].data, VSUSP), cb1),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VDSUSP), cb2),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VREPRINT), cb3),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VFLUSH), cb4));
  print(buf);
  sprintf(buf, "\twerase '%s' \tlnext '%s' \tquote '???'",
	  getcharrep((u_char)get_ele(tty_fields[0].data, VWERASE), cb1),
	  getcharrep((u_char)get_ele(tty_fields[0].data, VLNEXT), cb2));
  print(buf);
  quit(0);
}
