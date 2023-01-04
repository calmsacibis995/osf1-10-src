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
static char	*sccsid = "@(#)$RCSfile: ibmdebug.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:21 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986,1987,1988,1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef lint
#endif

#if defined(ibm032) && defined(i386)
#undef ibm032
#endif

#include "i386/rdb/ctype.h"
#include "i386/rdb/cmds.h"
#ifdef ibm032
#include "init.h"		/* for hardware dependent stuff */
#include "sa.h"
#endif /* ibm032 */


#ifdef ATR

#define OTHER_ENTER() debug_flag = ps2_enter_debugger();

#define OTHER_EXIT() ps2_exit_debugger();

#else ATR
#define OTHER_ENTER()
#define OTHER_EXIT()
#endif ATR

#ifdef TTY_CONSOLE
int (*alt_getchar)();
int (*alt_putchar)();
int (*deb_getchar)();
int (*deb_putchar)();
#define ENTER_DEBUGGER() { alt_getchar = deb_getchar; alt_putchar = deb_putchar; OTHER_ENTER() }
#define EXIT_DEBUGGER()  { alt_getchar = 0; alt_putchar = 0 ; OTHER_EXIT() }
#else
#define ENTER_DEBUGGER() { OTHER_ENTER() }
#define EXIT_DEBUGGER() { OTHER_EXIT() }
#endif TTY_CONSOLE

#define OUT_W(addr,value) * (int *) addr = value

#ifndef isdigit
#define isdigit(c) ('0' <= (c) && (c) <= '9')
#define isXdigit(c) (('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F' ))
#define isxdigit(c) (isdigit(c) || isXdigit(c))
#define isspace(c) (c == ' ' || c == '\t')
#define ispunct(c) 1	/* only used as an optimization anyway */
#endif

#define isop(c) (ispunct(c) && ((c) == '+' || (c) == '-' || (c) == '*' || \
(c) == '/' || (c) == '!' || (c) == RIGHT_PAREN || (c) == '^' || (c) == '&' || \
(c) == '|' || (c) == '<' || (c) == '>' || c == '=' || c == ',' || \
c == LEFT_PAREN))	/* test if (c) is diadic operator */
#define ATOX(ptr) expr(ptr,0x10)	/* convert - hex default */
#define ATON(ptr) expr(ptr,10)		/* convert - decimal default */
#define LEFT_PAREN	'('		/* left parenthesis */
#define RIGHT_PAREN	')'		/* right parenthesis */

#define NWORDS 8

#include "i386/rdb/debug.h"

#define BREAK_MAX_LINE (64-2)		/* "" (3 is to align) */

struct break_point {
	break_t *addr;			/* address of break point */
	int counter;			/* times to ignore */
	break_t instn;			/* the actual instruction */
	char temp;			/* if temp or not */
	char cmd[BREAK_MAX_LINE];	/* the command line */
} break_point[MAX_BREAK] = { 0 };

int dispsize = 2;
int break_count = 0;		       /* the number of break points set */
int break_set = 0;		       /* when breakpoints are set */
int err_flag = 0;
struct symtab *closest(), *scansym();
char *index();
char *pr_wtype();
char *screen_buffer = 0, *screen_save = 0;
				/* the screen buffer and where to save it */
int screen_size = 0;		       /* how long the screen buffer is */
static int screen_saved = 0;	       /* if we have saved the screen */
int iar_mask = 0xffffffff;	       /* bits to mask out in the iar */
struct watch_point {
	int	*addr;			/* address to watch */
	int	value;			/* the old value */
	char	type;			/* the type */
	char cmd[BREAK_MAX_LINE+1];	/* the command line */
} watch_point[MAX_BREAK] =
	{ 0 };
static int wtype = WTYPE_DEFAULT;	/* default watch point type */
int watch_count = 0;
int go_step = 0;			/* = 1 if step after go */
int save_mq = 0;			/* the saved MQ register */
int save_mcpc = 0;			/* the saved MCSPCS register */
int debug_option = 0;			/* the debugger debug flag etc */
int option_mask = 0;			/* mask for step count */
int debug_size = 0;			/* previous pattern size */
int debug_scan = 0;			/* how much is left */
int debug_lastscan = 0;			/* where to start next time */
int debug_patlen = 0;			/* scan pattern length */
union pattern
{
	int	words[4];
	short	halfs[8];
	char	bytes[16];
} debug_pattern = {0};
int db_offset = 0x10000;		/* how far away is far */
int db_radix1 = 0x10;			/* for addresses */
int db_radix2 = 10;			/* for counts etc. */

#ifdef ROMPC
int ioim1_gsr;		/* IOIM GSR */
#endif ROMPC

#ifdef DEBUG
#define DEBUGX(stmt) if (debug_option&DEBUG_DEBUG) stmt
#else
#define DEBUGX(stmt)
#endif


#define ARG_0	0
#define ARG_1	1
#define ARG_2	2
#define ARG_3	3
#define ARG_4	4


struct cmd {
	short cmd_number;	       /* the number of the command */
	short cmd_args;		       /* the number of args */
	char *cmd;		       /* the name of command */
	char *help_text;	       /* some helpfull text */
} cmds[] = {
/*
 * NOTE: the makefile looks for lines with the a particular format in order
 * to build the #define's in cmds.h - be careful when changing.
 * in particular be careful of changing the format of the following lines.
 * order is important only to the 'help' command printout and to resolve
 * ambiguous commands (e.g. "s" = "sysreg" because it is first).
 */
ASCII,	       ARG_1,  "ascii",	"[addr]			display in ascii",
BREAK,	       ARG_2,  "break",	"[addr] [count] ['cmd']	set/print break points",
DISPMEM,       ARG_3,  "display",	"addr [count] [width]	display memory",
DEFINE,	       ARG_0,  "define", "symbol addr		create symbol table entry",
DUMP,	       ARG_3,  "dump",	"addr [count incr]		dump memory",
VERSION,       ARG_0,  "version",	"			print out current version number",
HELP,	       ARG_0,  "help",	"[cmd|*]			print help text",
QMARK,	       ARG_0,  "?",	0,
MOD,	       ARG_2,  "/",	"addr [count]	modify",
MODHALF,       ARG_2,  "/half",	"addr [count]		modify halfword",
MODBYTE,       ARG_2,  "/byte",	"addr [count]		modify byte",
MODWORD,       ARG_2,  "/word",	"addr [count]		modify word",
CALL,	       ARG_3,  "call",	"addr [args]		call C routine",
PCALL,	       ARG_3,  "pcall",	"addr [args]		program call C routine",
PRETURN,       ARG_0,  "preturn","			program call return",
CLEAR,	       ARG_1,  "clear",	"[addr]			clear given/all break/watch points",
IDENT,	       ARG_3,  "ident",	"[addr count flag]		print identification",
UNASM,	       ARG_2,  "unasm",	"addr [count]		disassemble code",
REGISTER,      ARG_2,  "reg",	"[reg] [value]		print/change register",
SCR,	       ARG_0,  "sysreg",	"[reg] [value]		print/change system register",
GO,	       ARG_1,  "go",	" 				restart after stopped",
STEP,	       ARG_2,  "step",	"[[addr] [count]]		step count instructions",
SYMBOL,	       ARG_1,  "symbol","[addr]			symbol >= addr",
SYMTAB,	       ARG_1,  "symtab",	"addr			provide symbol table",
MOVE,	       ARG_3,  "move",	"source count target - move memory",
DOLLAR,	       ARG_0,  "$",     "				display status",
STATUS,	       ARG_0,  "status",     "				display status",
SHOW,	       ARG_1,	"show", "[addr]			display/set save screen",
EQUALS,	       ARG_1,  "=","addr [fmt=%x] [nl]		printf fmt value",
STAR,	       ARG_0,  "*", 0,
WATCH,	       ARG_1,  "watch","[addr] [type] ['cmd']	set/print watch points",
LOOKUP,        ARG_0,  "lookup","symbol			lookup symbol in symtab",
TRACEBACK,     ARG_3,  "trace","[addr stack]		trace stack",
CLS,	       ARG_0,  "cls",   "				clear screen",
HOME,	       ARG_0,  "home",   "				home position cursor",
OPTION,	       ARG_2,	"option","value [count]		set option flags",
FILL,	       ARG_3,  "fill","[addr] [count] [value]	fill memory",
USERCMD,       ARG_3,  "usercmd", "?			user command",
INB,	       ARG_2,  "inb","port [count]		read count byte(s) from the PC's I/O space",
OUTB,	       ARG_2,  "outb","port value			write a byte to the PC's I/O space",
INHW,	       ARG_2,  "inhw","port [count]		read count half word(s) [16 bits] from the PC's I/O space",
OUTHW,	       ARG_2,  "outhw","port  value		write a half word [16 bits] to the PC's I/O space",
TERM,		ARG_0, "term","home clear clreol		set termcap strings",
MACRO,		ARG_0, "macro", "name			define macro",
MACRODELETE,	ARG_0, "macrodelete", "name		delete macro definition",
MACROLIST,	ARG_0, "macrolist", "name			list macro definition",
ECHO,		ARG_0, "echo",	"[-n] args			echo arguments",
IF,		ARG_0, "if",	"expr truecmd [falsecmd]	conditional",
WHILE,		ARG_0, "while",	"expr command",
REPEAT,		ARG_0, "repeat", "count command",
DEBUGREG,	ARG_0, "debugreg","register [value]	display/change debug register",
SHIFT,		ARG_0, "shift",	"				shift macro arguments",
STOP,		ARG_0, "stop","				stop break/cmd/macro",
SCAN,		ARG_3, "scan", "addr length size pattern ... scan memory",
PUT,		ARG_0, "put", "text			put text to input stream",
EDIT,		ARG_0, "edit","type			edit text",
#include "i386/rdb/cmds_machine.h"
0,	ARG_0,	0,	0		/* terminate command list */
};

struct symtab internal[];		/* the internal symbols */
struct symtab deb_internal[];		/* the debugger internal symbols */

static char home[10] = "\33H";
static char clear[10] = "\33K";
static char clear_eol[10] = "\33I";

char *HO = home;		/* home screen */
char *CL = clear;		/* clear screen */
char *CE = clear_eol;		/* clear to eol */

help_cmd(n)
	register int n;
{
	register struct cmd *p = cmds;
	register char *s, *h;
	register int i = 0;
	int lines = 0;

	if (n == 0)
		printf("Commands available are: (with leading * from top level)\n\n");
	for (; (s = p->cmd) != 0; ++p)
		if ((h = p->help_text) != 0)
			if (n == 0)
				printf("%s%s", s, (++i) & 07 ? "\t" : "\n");
			else if (n == p->cmd_number || n == STAR) {
				printf("%s %s\n", s, h);
				if (!db_newline(&lines))
					break;
			}
	if ((i & 07) != 0)
		printf("\n");
}


/*
 * lookup the command in "str" in the command table.
 * commands may be abbreviated. If requested we return
 * the number of arguments allowed.
 */
get_cmd(str,nargs)
	register char *str;
	int *nargs;
{
	register struct cmd *p = cmds;
	register int len = strlen(str);

	for (; p->cmd; ++p)
		if (str[0] == p->cmd[0])	/* check 1st letter */
			if (strncmp(str, p->cmd, len) == 0)
				{
				if (nargs)
					*nargs = p->cmd_args;
				return (p->cmd_number);
				}
	return (0);
}


static int dot = 0;
int lastdebugcmd = HELP;	       /* default is to give help */
int lastcmdargs = 0;			/* no args specified */
/*
 * user cmd is a mechanism for a user to add additional commands 
 * to the debugger by just pointing usercmd to a function to be
 * called; simplest way of doing that is to put a file into libc 
 * that has the following:
 *	int fn();
 *	int (*usercmd)() = fn;
 * one can also set usercmd on the fly but that is more difficult
 * and 'call' serves much the same purpose.
 */
int (*usercmd)() = 0;

dump_cmd(argc, argv, depth)
	int argc;
	char **argv;
	int depth;
{
	register int count;
	register int i;
	register char *p;
	int arg2, arg3, arg4;
	static int modsize = 2;		/* default to half */

	p = argv[0];
	while (p && *p == '*')
		++p;		       /* ignore leading *'s */
	if (p == 0 || *p == 0 || *p == '.')
		i = lastdebugcmd;
	else
		i = lastdebugcmd = get_cmd(p, &lastcmdargs);

	err_flag = 0;

	if (i > 0) {
/* nargs is the actual number of args (including command name) that was
 * given and expected.
 */
		register int nargs = lastcmdargs + 1;
		/* actual number of args allowed */

		if (nargs > argc)
			nargs = argc;
		if (nargs > 1)
			dot = ATOX(argv[1]);
		count = nargs > 2 ? ATON(argv[2]) : 10;
		arg2 = nargs > 3 ? ATOX(argv[3]) : 0;
		arg3 = nargs > 4 ? ATOX(argv[4]) : 0;
	}
	if (err_flag)
		return(err_flag);	       /* argument in error */
	switch (i) {
	case BREAK:		       /* set break point */
		if (argc < 2)
			break_list();
		else
			break_cmd(dot, argc < 3 ? 0 : count, argc > 3 ? argv[3] : "");
		break;
	case WATCH:		       /* set watch point */
		if (argc < 2)
			watch_list();
		else
			watch_cmd(dot, argv[2], argc > 3 ? argv[3] : "");
		break;
	case UNASM:
		dot = do_unasm(dot, count, argc > 3 ? argv[3] : (char *) 0);
		break;
	case HELP:
	case QMARK:
		if (argc <= 1)
			{
			macro_help((char *)0);
			help_cmd(0);
			}
		else
			if (!macro_help(argv[1]))
				help_cmd(get_cmd(argv[1],(int *) 0));
		break;
	case VERSION:
		printf("%s\n", rcsid);
		break;
	case GO:		       /* go on with execution */
		printf("jdxxx saw go\n");
		if (debug_state == NORMAL_STATE || debug_state == PM_STATE) {
			not_stopped();
			break;
		}
		debug_go(argc > 1, dot);	/* continue execution */
		break;
	case ASCII:		       /* display memory in ascii */
		dot = ascii(dot, count);
		break;
	case DUMP:		       /* display memory */
		dot = dump_mem((int *) dot, count, arg2);
		break;
	case DISPMEM:		       /* display memory */
		dot = display((word *) dot, count, (word *) dot, arg2 ? arg2 : dispsize);
		break;
	case CLEAR:
		if (argc < 2) {
			break_count = 0; /* get rid of em */
			watch_count = 0; /* watch points too */
		} else
			break_clear(dot);
		break;
	case HOME:
		printf(HO);	       /* home cursor */
		break;
	case CLS:
		printf(CL);	       /* clear screen (ibm3101) */
		break;
	case CALL:
		dot=call((FN) dot, count, arg2, arg3);
		break;
	case PCALL:
		pcall((FN) dot, count, arg2, arg3);	 /* doesn't return */
		break;
	case PRETURN:
		preturn();
		break;
	case REGISTER:
		if (debug_state == NORMAL_STATE) {
			not_stopped();
			break;
		}
		if (argc >= 3) {
			int new = ATOX(argv[2]);
			set_reg(argv[1], new);
		} else if (argc >= 2)
			printf("%x\n", dot);	/* value already in dot */
		else
			printregs(debug_state);
		break;
	case STATUS:
		debug_status(debug_state);
		break;
	case DOLLAR:
		debug_print(debug_state); /* print out status */
		break;

	case IDENT:
		{
#ifdef XCOFF
			extern char _etext, _edata, _end;
#else
			extern char etext, edata, end;
#endif

			if (argc < 2) {
#ifdef XCOFF
				dot = (int)&_etext;/* probably should be long */
				count = &_edata - &_etext;
#else
				dot = (int)&etext; /* probably should be long */
				count = &edata - &etext;
#endif
			}
			if (argc < 3)
				count = 0x10000;
			ident(dot, dot + count, arg2); /* print ident info */
			break;
		}
	case STEP:
		go_step = 0;	       /* not in go step mode */
		switch (argc) {
		case 0:
		case 1:
			count = 1;     /* default is 1 step */
			break;
		case 2:
			count = ATON(argv[1]); /* use given step count */
			break;
		default:	       /* step addr count */
			MTSR(SCR_IAR, dot); /* start at given location */
			break;
		}
		step(count);
		break;
	case SYMTAB:
		if (dot != 0 && hfetch(dot) == -1)
			bad_addr();
		else
			{
			printf("was %x\n",symtab);
			symtab = (struct symtab *)dot;
			}
		break;
	case SYMBOL:
		prsym(dot, argc > 2 ? count : db_offset, "%x = ");
		printf("\n");
		break;
	case EQUALS:
		printf(argc > 2 ? argv[2] : "0x%x", dot);
		if (argc < 4)
			printf("\n");
		break;
	case MOD:
		dot = modify(dot, dot, count,
			(debug_option&OPTION_MOD) ? modsize : 2,
			argc > 3 ? argv+3 : 0);
		break;
	case MODBYTE:
		dot = modify(dot, dot, count, modsize=1,
		    argc > 3 ? argv+3 : 0);
		break;
	case MODWORD:
		dot = modify(dot, dot, count, modsize=4,
		    argc > 3 ? argv+3 : 0);
		break;
	case MODHALF:
		dot = modify(dot, dot, count, modsize=2,
		    argc > 3 ? argv+3 : 0);
		break;
	case MOVE:
		{
			register int last;
			printf("move %x (%d bytes) to %x\n", dot, count, last = ATOX(argv[3]));
			bcopy(dot, last, count);
			break;
		}
	case SHOW:		       /* display the screen buffer */
		if (argc > 1) {
			screen_saved = 0; /* prevent trouble */
			screen_save = (char *)dot; /* hope its actually there */
			break;
		}
		if (screen_saved) {
			save_screen(screen_save, screen_buffer, screen_size);
			getchar();
		}
		break;
	case DEFINE:
		if (argc <= 1)
			printf("name required\n");
		else
			define(argv[1], symtab, argc <= 2 ? dot : ATOX(argv[2]));
		break;
	case LOOKUP:
		lookup_sym(argc <= 1 ? "" : argv[1], symtab);
		if (symtab2 && hfetch((int) symtab2) >= 0)
			lookup_sym(argc <= 1 ? "" : argv[1], symtab2);
		break;
	case OPTION:
		if (argc > 1)
			debug_option = dot;
		if (argc > 2)
			option_mask = (1 << count) - 1;
		printf("options=%b mask=%x\n", debug_option, OPTION_FMT, option_mask);
		break;
	case FILL:
		printf("filling %x ... %x (%d bytes) with %x\n",dot,dot+count-1,count,arg2);
		while (--count >= 0)
			bstore(dot++, arg2);
		break;
	case USERCMD:
		if (usercmd == 0)
			printf("no usercmd available\n");
		else
			(*usercmd)(dot,count,arg2,arg3);
		break;
	case INB:		       /* ior or iow */
		if(argc == 2)
			count = 1;
		for (;--count >= 0 && !err_flag; ) {
#ifdef ATR
#define PC_IO_LIMIT	0x100
			if ((unsigned) dot < PC_IO_LIMIT)
				printf("warning: cannot reference I/O addresses < %x\n",PC_IO_LIMIT);
			printf("inb(%x / %x) --> ", dot,pcif_io_b+dot);
#endif
#ifdef IBMRTPC
			printf("inb(%x / %x) --> ", dot,IO_BASE+dot);
#endif
			i = in(dot);
			printf("0x%x (%d)\n", i, i);
			dot += 1;      /* advance to next address */
		}
		break;

	case OUTB:		       /* ior or iow */
		out(dot, count);
		++dot;
		break;

	case INHW:		       /* ior or iow */
		if(argc == 2)
			count = 1;
		for (;--count >= 0 && !err_flag; ) {
#ifdef ATR
			if ((unsigned) dot < PC_IO_LIMIT)
				printf("warning: cannot reference I/O addresses < %x\n",PC_IO_LIMIT);
			printf("inh(%x / %x) --> ", dot,pcif_io_hw+dot);
#endif
#ifdef IBMRTPC
			printf("inh(%x / %x) --> ", dot,IO_BASE+dot);
#endif
			i = INW(dot);
			printf("0x%x (%d)\n", i, i);
			dot += 1;      /* advance to next address */
		}
		break;

	case OUTHW:		       /* ior or iow */
		OUTW(dot, count);
		++dot;
		break;

	case TERM:			/* specify terminal chars */
		if (argv[1])
			strcpy(HO=home,argv[1]);
		if (argv[2])
			strcpy(CL=clear,argv[2]);
		if (argv[3])
			strcpy(CE=clear_eol,argv[3]);
		break;
	case MACRO:
		if (argc <= 1 || argv[1] == 0)
			macro_help((char *) 0);
		else
			macro_define(argv[1]);
		break;
	case MACRODELETE:
		if (argc <= 1 || argv[1] == 0)
			printf("name required\n");
		else
			macro_delete(argv[1]);
		break;
	case MACROLIST:
		macro_list(argc <= 1 ? (char *) 0 : argv[1]);
		break;
	case STOP:
		if (depth == 0)
			printf("already stopped\n");
		/* fall thru to the echo code to display any message */
	case ECHO:		/* echo the parameters */
		{ int n = 0, j = 1;
		if (argc > 1 && strcmp(argv[1],"-n")==0)
			n = ++j;
		for (; j<argc; ++j)
			printf("%s%s",argv[j],(j+1)==argc ? "" : " ");
		if (n == 0 && (i != STOP || argc > 1))
			printf("\n");  /* output NL unless not required */
		if (i == STOP)
			return(1);	/* stop current command execution */
		break;
		}
	case IF:
		if (ATOX(argv[1]))
			i = debug_cmd(argv[2],depth);
		else if (argc > 3)
			i = debug_cmd(argv[3],depth);
		else
			i = 0;
		if (i)
			return(i);
		break;
	case WHILE:
		while (ATOX(argv[1]))
			if (debug_cmd(argv[2],depth))
				return(1);
		break;
	case REPEAT:
		count = ATOX(argv[1]);
		while (--count >= 0)
			if (debug_cmd(argv[2],depth))
				return(1);
		break;
	case DEBUGREG:
		if (argc >= 3) {
			int new = ATOX(argv[2]);
			set_debug_reg(argv[1], new);
		} else if (argc >= 2) {
			int debug_reg();
			print_reg(argv[1],deb_internal,debug_reg);
		} else
			debug_regs();
		break;
	case SHIFT:
		break;		/* outside macro do nothing */
	case SCAN:
		switch(argc)
			{
		case 0:
		case 1: dot = debug_lastscan;
		case 2:	count = debug_scan;
		case 3: arg2 = debug_size;
			}
		dot = db_scan(dot, count, arg2, argc-4, argv+4);
		break;
	case PUT:
		if (argc > 1)
			put_line(argv[1]);
		break;
	case EDIT:
		{
		int i;
		if (argc < 2)
			{
			printf("edit arg should be break or watch or macro\n");
			break;
			}
		i = get_cmd(argv[1],(int *) 0);
		if (i == BREAK)
			edit_break_watch(i, "break", break_count);
		else if (i == WATCH)
			edit_break_watch(i, "watch", watch_count);
		else if (i == MACRO)
			edit_macro(argv[2]);
		else
			printf("%s not break or watch\n",argv[1]);
		break;
		}
	default:
		printf("%s: unknown command\n",argv[0]?argv[0]:"");
		return(1);
#include "i386/rdb/cmds_machine.c"	/* machine dependent commands */
	}
	return(err_flag);
}


edit_break_watch(type, name, count)
int type;
char *name;
int count;
{
	char cmd[MAX_LINE];
	int i;
	int result;

	if (count <= 0)
		printf("no %s points defined\n", name);
	for (i=0; i<count; )
		{
		if (type == BREAK)
			sprintf(cmd, "break %x %d '%s'", break_point[i].addr,
				break_point[i].counter, break_point[i].cmd);
		else
			sprintf(cmd, "watch %x %s '%s'", watch_point[i].addr,
				pr_wtype(watch_point[i].type),
				watch_point[i].cmd);
		result = db_edit(cmd);
		if (result&EDIT_CHANGE)
			put_line(cmd);
		if (result&EDIT_DOWN)
			++i;
		else
			if (--i < 0)
				i = 0;
		}
}

debug_print(debug_state)
int debug_state;
{

	printf("Status: ");
	if (debug_state != NORMAL_STATE) {
		debug_status(debug_state);
#ifdef IBMRTPC
		csr_print();
#endif IBMRTPC
		printregs(debug_state);
		sys_regs();
		show_instn(debug_state);
	} else
		not_stopped();
}


ascii(start, count)
	int start;
	register int count;
{

	register int c;
	int lines = 0;

	for (; (c = bfetch((int) start)) != 0; ++start) {
		if (c <= 0 || c & 0x80)
			break;
		if (c < 040 && c != '\t' && c != '\n') {
			putchar('^');
			putchar(c+'@');
		} else
			putchar(c);
		if (c == '\n' && !db_newline(&lines))
			break;
	}
	putchar('\n');
	return((int)start+1);
}

/*
 * count lines and handle going past end of screen where we pause
 * and allow the user to cancel with a ^D or ^C.
 */
db_newline(lineptr)
int *lineptr;
{
	if (++*lineptr >= screen_lines-2)
		if (db_pause() <= 04)
			return(0);
		else
			*lineptr = 0;
	return(1);
}

/*
 * display memory at "start" with the label "addr"
 * normally "start" and "addr" are the same except
 * when displaying PC memory.
 */
display(start, count, addr, dispsize)
	register word * start;
	register int count;
	register word * addr;
	int dispsize;
{
	register int i;
	register char *p;
	register int c;
	register value;
	int lines = 0;
	int sum;
	int same = 0;

	do {
		for (i = 0; i < NWORDS; ++i)
			if ((sum = hfetch((int) (start + i))) != 0)
				break;
		if (sum == 0) {
			if (same++) {
				if (same == 2) {
					printf("...\n");
					if (!db_newline(&lines))
						break;
				}
				start += NWORDS;
				addr += NWORDS;
				continue;
			}
		} else
			same = 0;
		printf("%08x: ", addr);
		p = (char *)start;
		if (dispsize == 4)
			for (i = 0; i < NWORDS/2; ++i) {
				value = wfetch((int) start);
				if (value == -1 && (hfetch((int) start) == -1 ||
					hfetch((int) (start+2)) == -1)) {
					bad_addr();
					return ((int)start);
				}
				printf("%08x  ", value);
				start += 2;
				addr += 2;
			}
		else if (dispsize == 2)
			for (i = 0; i < NWORDS; ++i) {
				if ((value = hfetch((int) start)) == -1) {
					bad_addr();
					return ((int)start);
				}
				printf("%04x ", value);
				++start;
				++addr;
			}
		else {
			for (i = 0; i < NWORDS * sizeof(word); ++i) {
				c = bfetch((int) p++);
				if (c == -1) {
					bad_addr();
					return ((int)start);
				}
				printf("%02x ",c);
			}
			p = (char *)start;
			start += NWORDS;
			addr += NWORDS;
		}
		printf("  |");
		for (i = 0; i < NWORDS * sizeof(word); ++i) {
			c = bfetch((int) p++);
			if (c < 040 || c >= 0177)
				c = '.';
			putchar(c);
		}
		printf("|");
		printf("\n");
		if (!db_newline(&lines))
			break;
	} while (--count > 0);
	return ((int)addr);
}


/*
 * print out memory in a full format 
 * one line per word
 */
dump_mem(start, count, incr)
	register int *start;
	register int count;
	register int incr;
{
	register value;
	int lines = 0;

	if (incr == 0)
		incr = 1;	/* default increment */
	for (; --count >= 0; start += incr) {
		prsym((int) start, db_offset, "%08x ");
		if ((value = hfetch((int) start)) == -1) {
			bad_addr();
			return ((int)start);
		}
		value = wfetch((int) start);
		printf("0x%08x ", value);
		printf("%12d ", value);
		printf("'%c' ",(value < 040 || value >= 0177) ? '.' : value);
		prsym(value, db_offset, "%08x ");
		printf("\n");
		if (!db_newline(&lines))
			break;
	}
	return ((int)start);
}



/*
 * change value at "start" (for "count" lines) of length "length"
 * start is the actual address we find the data at, addr is the
 * form that we print.
 * length == 1, 2 , or 4
 * if count <=0 then don't check address validity as this can cause
 * problems when accessing the I/O map.
 * if count < 0 then don't display the old value as this can also
 * cause problems when addess the I/O map.
 * if argv is specified it contains an array of strings used to
 * supply the the value. This is used inside macro definitions. 
 */
modify(start, addr, count, length, argv)
	int start, addr, length, count;
	char **argv;
{
	char buff[SHORT_LINE];
	register int n;
	register oldcount = count;     /* special case for I/O bus */

	if (count <= 0)
		count = 1;	       /* always do at least one */
	for (; --count >= 0;) {
		if (argv == 0) {
			prsym(addr, db_offset, "%08x ");
			putchar(':');
		}
		if (oldcount > 0) {
			if (length == 1)
				n = hfetch(start & ~1);	/* get the half word */
			else if (length == 2)
				n = hfetch((int) start); /* get the half word */
			else
				n = hfetch((int) start) | hfetch(start + 2);
			if (n == -1) {
				bad_addr();
				return (addr);
			}
		}
		if (argv && *argv)
			strcpy(buff, *argv++);
		else {
			if (oldcount >= 0) {
				if (length == 1)
					printf("%02x ", bfetch((int) start));
				else if (length == 2)
					printf("%04x ", hfetch((int) start));
				else
					printf("%08x ", wfetch((int) start));
			}
			db_gets(buff);
		}
		if (*buff == '^') {
			start -= length;
			addr -= length;
			continue;
		}
		if (strcmp(buff, "end") == 0 || strcmp(buff, ".") == 0 ||
				(*buff && *buff < 012))
			break;
		if (*buff && !isspace(*buff)) {
			err_flag = 0;
			n = ATOX(buff);
			if (err_flag)
				continue;
			if (length == 1)
				bstore(start,n);
			else if (length == 2)
				hstore(start,n);
			else
				wstore(start,n);
		}
		start += length;
		addr += length;
	}
	return (addr);
}


db_pause()
{
	register int c;

	put_status(50, "<HOLDING>");
	c = _getchar();
	put_status(50, "         ");
	return (c);
}


/*
 * interface to the real expression routine
 * which takes a 'char **' and updates it
 */
expr(ptr, base)
	char *ptr;
	register int base;
{
	return (_expr(&ptr, base));
}


/*
 * pick up operands of the form:
 * item [+- item] ...
 */

#define TWOBYTE(a,b) ((a<<8)+b)
_expr(pptr, base)
	register char **pptr;
	register int base;
{
	register int n1 = item(pptr, base), n2, op;

	for (;;) {
		op = *(*pptr);
		if (op == 0 || op == RIGHT_PAREN || op == ',')
			break;
		++(*pptr);	       /* point at next item */
		if (op == '<' || op == '>' || op == '=' || op == '&' || op == '|')
			{
			if (*(*pptr) == op || *(*pptr) == '=')
				op = TWOBYTE(op,*(*pptr)++);
			}
		else if (op == '!' && *(*pptr) == '=')
			op = TWOBYTE(op,*(*pptr)++);
		n2 = item(pptr, base);
		switch (op) {
		case TWOBYTE('=','='):
			n1 = n1 == n2;
			break;
		case TWOBYTE('!','='):
			n1 = n1 != n2;
			break;
		case TWOBYTE('>','='):
			n1 = n1 >= n2;
			break;
		case TWOBYTE('<','='):
			n1 = n1 <= n2;
			break;
		case TWOBYTE('&','&'):
			n1 = n1 && n2;
			break;
		case TWOBYTE('|','|'):
			n1 = n1 || n2;
			break;
		case '<':
			n1 = n1 < n2;
			break;
		case '>':
			n1 = n1 > n2;
			break;
		case '-':
			n1 -= n2;
			break;
		case '+':
			n1 += n2;
			break;
		case '*':
			n1 *= n2;
			break;
		case '/':
			n1 /= n2;
			break;
		case '!':
			if (hfetch(n1) == -1) {
				bad_addr();
				break;
			}
			switch (n2) {
			case 1:
				n1 = bfetch(n1);
				break;
			case 2:
				n1 = hfetch(n1);
				break;
			case 4:
				n1 = wfetch(n1);
				break;
			default:
				bad_addr();
			}
			break;
		case '^':
			n1 ^= n2;
			break;
		case '&':
			n1 &= n2;
			break;
		case '|':
			n1 |= n2;
			break;
		case TWOBYTE('<','<'):
			n1 <<= n2;
			break;
		case TWOBYTE('>','>'):
			n1 >>= n2;
			break;
		default:
		badop:
			printf("bad operator");
			++err_flag;
			break;
		}
	}
	return (n1);
}


/* get an item (hex, decimal, number or a symbol value */
int item(pptr, base)
	register char **pptr;
	register int base;
{
	register int n2;
	register char *ptr = *pptr;
	register char *s = ptr + 1;
	register int op;
	int err = 0;
	register int flag = 0;
#define NOT_DIGIT 1
#define NOT_XDIGIT 2
#define IS_DIGIT 4

	if (*ptr == '=') {
		++(*pptr);	       /* bump the pointer past = */
		return (vtop(_expr(pptr, base)));
	}
	if (*ptr == LEFT_PAREN) {
		++(*pptr);	       /* bump the pointer past = */
		n2 = _expr(pptr, base);
		if (**pptr != ')') {
			printf(") expected\n");
			++err_flag;
		} else ++(*pptr);
		return (n2);
	}
	if (*s == '\'')
		return(str_const(pptr));	/* string literal */
	for (; *s && !isop(*s); ++s) {
		if (isdigit(*s))
			{
			flag |= IS_DIGIT;
			continue;		/* it is a normal digit */
			}
		if (!isxdigit(*s))
			flag |= NOT_XDIGIT;
		else
			flag |= NOT_DIGIT;
	}
	op = *s;
	*s = 0;			       /* terminate the string */
	if (ptr[0] == '0' && ptr[1] == 'x')
		n2 = (atox(ptr + 2));
	else if (ptr[0] == '0' && ptr[1] == 't')
		n2 = (atoi(ptr + 2));
	else if (isdigit(*ptr) && (flag & (NOT_XDIGIT|NOT_DIGIT)))
		n2 = base == 10 ? atoi(ptr) : atox(ptr);	/* number */
	else if (*ptr == '_')
		n2 = (lookup(ptr + 1, 0, symtab));
	else if (*ptr == '.' && ptr[1] == 0)
		n2 = dot;	       /* current location */
	else if (*ptr == '\'')
		n2 = ptr[1];	       /* just single letter for now */
	else if (*ptr == '+' || *ptr == '-' ||
	    (base == 10 && isdigit(*ptr) && (flag & NOT_DIGIT) == 0))
		n2 = atoi(ptr);
	else if (base == 16 && isdigit(*ptr) && (flag & NOT_XDIGIT) == 0)
		n2 = atox(ptr);		/* hex number */
	else if (*ptr == '&' && (n2 = db_index(ptr+1)) != 0) ;
	else if ((n2 = lookup(ptr, &err, internal)), err == 0)
		n2 = GET_REG(n2);       /* return value of internals */
	else if ((n2 = lookup(ptr, &err, deb_internal)), err == 0)
		n2 = debug_reg(n2);	/* internal debugger symbol */
	else if ((n2 = lookup(ptr, &err, sys_internal)), err == 0)
		n2 = MFSR(n2);		/* system register symbol */
	else if ((debug_option&OPTION_NOSYM) == 0 &&
		((n2 = lookup(ptr, &err, symtab)), err == 0));
	else if (symtab2 && hfetch((int) symtab2) >= 0 &&
		((n2 = lookup(ptr, &err, symtab2)), err == 0));
	else if (base == 16 && isxdigit(*ptr) && (flag & NOT_XDIGIT) == 0)
		n2 = atox(ptr);
	else {
		++err_flag;
		printf("%s ??\n", ptr);
	}
	*pptr = s;		       /* point to delimeter */
	*s = op;		       /* restore  delimeter */
	if (op == LEFT_PAREN)
		{
		++(*pptr);
		n2 = expr_fn(pptr,base,(FN) n2);	/* function invocation */
		}
	return (n2);
}

/*
 * lookup name in internal symbol tables and return the mask
 * bit for the mask variables
 */
db_index(name)
char *name;
{
	struct symtab *locate();
	register struct symtab *s;
	int err;
	int n;

	if ((s = locate(name, &err, internal)) != 0)
		n = s - internal;
	else if ((s = locate(name, &err, sys_internal)) != 0)
		n = s - sys_internal;
	else if ((s = locate(name, &err, deb_internal)) != 0)
		n = s - deb_internal;
	else
		return(0);
	return(1<<n);
}

/*
 * get a character constant 
 */
str_const(pptr)
char **pptr;
{
	char *s = *pptr;
	char delim = *s++;
	int result = 0;

	while (*s && *s != delim)
		result = (result<<8) | *s++;
	if (*s)
		++s;
	*pptr = s;
	return(result);
}

/*
 * invoke specified function
 */
#define MAXARG 5	/* max number of arguments we allow on a function */

expr_fn(pptr,base,fn_addr)
char **pptr;
int base;
int (*fn_addr)();
{
	int args[MAXARG];			/* limit it to 5 for now */
	int argcnt = 0;
	while (*(*pptr) != RIGHT_PAREN && err_flag == 0)
		{
		if (argcnt >= MAXARG)
			{
			printf("too many arguments to function\n");
			++err_flag;
			break;
			}
		args[argcnt++] = _expr(pptr,base);
		if (*(*pptr) == ',')
			++(*pptr);
		}
	++(*pptr);
	while (argcnt < MAXARG)
		args[argcnt++] = 0;	/* set unused args to 0 */
	if (err_flag == 0)
		{
		if (hfetch((int) fn_addr) == -1)
			++err_flag, printf("bad function address %x\n", fn_addr);
		else
			return((*fn_addr)(args[0],args[1],args[2],args[3],args[4]));
		}
	return(0);
}

/*
 * display memory at "start" in unassembled format
 * if "string" is specified only display lines that have "string"
 * in them.
 */
do_unasm(start, count, string)
	register int start;
	int count;
	char *string;
{
	register int n;
	int lines = 0;
	char buff[80];		/* must be enough for longest asm line */
	register char *p;
	short instns[2];
	register int value;

	for (; --count >= 0; ) {
		if ((value = hfetch((int) start)) == -1) {
			printf("%08x: ", start);
			bad_addr();
			break;
		}
#if defined(ibm032) && !defined(PS2)
		instns[0] = value;
		instns[1] = hfetch(start+2);
		n = _unasm(start, instns, buff);
		if (p = index(buff, '\n'))
			*p = 0;	       /* remove the \n */
		n &= 07;
		if (string && !db_search(buff, string)) {
			start += n;	       /* length in bytes ? */
			continue;
		}
		printf("%s", buff);
		prsym(start, 0x1000, "		");
		printf("\n");
		start += n;	       /* length in bytes ? */
#endif /* ibm032 */
#ifdef i386
		db_16bit = (cs_size ? cs_size : get_cs_size()) == 16;
		n = printop(start,buff);
		if (string && !db_search(buff, string)) {
			start = n;
			continue;
		}
		printf("%-62s", buff);
		prsym(start, 0x1000, " ");
		printf("\n");
		start = n;
#endif
		if (!db_newline(&lines))
			break;
	}
	return ((int)start);
}

/*
 * look for str2 as a substring of str1
 */
db_search(str1, str2)
char *str1, *str2;
{
	register int c = *str2;
	register int len = strlen(str2);
	char *end = str1 + strlen(str1) - len;

	for (;str1 <end; ++str1)
		if (*str1 == c && bcmp(str1, str2, len) == 0)
			return(1);
	return(0);
}


ident(p, e, flag)
	unsigned int p, e;
	int flag;
{
	char a, b, c, d;
	int lines = 0;

	printf("ident: %x ... %x\n", p, e);
	for (; p < e; ++p) {
		if (((a = bfetch(p)) == '$' || a == '@') &&
		((b = bfetch(p+1)) == 'H' || b == LEFT_PAREN) &&
		((c = bfetch(p+2)) == 'e' || c == '#') &&
		((d = bfetch(p+3)) == 'a' || d == RIGHT_PAREN)) {
			if (flag)
				printf("%x: ", p);
			p = ascii(p, 512);
			if (!db_newline(&lines))
				break;
		}
	}
}

bad_addr()
{
	printf("bad address!\n");
	++err_flag;
}

break_cmd(start,count,cmd)
	register int start;
	int count;
	char *cmd;
{
	register int i;
	register int instn;
	register int temp = 0;

	if (count < 0) {
		count = -count;
		temp++;
	}

	for (i = 0; i < break_count; ++i)
		if (break_point[i].addr == (break_t *) start) {
			break_point[i].counter = count;
			break_point[i].temp = temp;
			if (*cmd)
				strcpy(break_point[i].cmd, cmd); 
			return;	       /* rest are already set */
		}
	if (i >= MAX_BREAK) {
		printf("only %d breakpoints allowed\n", MAX_BREAK);
		return;
	}
	if (((int)start & (sizeof (break_t) -1)) ||	/* test align */
			(instn = hfetch((int) start)) == -1) {
		bad_addr();
		return;
	}
	break_point[i].addr = (break_t *) start;
	break_point[i].instn = instn;
	break_point[i].counter = count;
	break_point[i].temp = temp;
	strcpy(break_point[i].cmd, cmd); 
	break_count++;
}

watch_cmd(start,how,cmd)
	int start;
	char *how;
	char *cmd;
{
	register int i;
	register int t = wtype;

	if (how && *how!=0 && *how!='-')
		t = cvt_watch_type(how);
	for (i = 0; i < watch_count; ++i)
		if (watch_point[i].addr == (int *)start) {
			watch_point[i].type = t;	/* watchpoint type */
			if (*cmd)
				strcpy(watch_point[i].cmd, cmd); 
			return;	       /* already set */
		}
	if (i >= MAX_BREAK) {
		printf("only %d watchpoints allowed\n", MAX_BREAK);
		return;
	}
	if (((int)start & WATCH_ALIGN) || hfetch((int) start) == -1) {
		bad_addr();
		return;
	}
	watch_point[i].addr = (int *)start;
	watch_point[i].value = *(int *)start;	/* prime it */
	watch_point[i].type = t;		/* watchpoint type */
	strcpy(watch_point[i].cmd, cmd); 
	watch_count++;
}


get_break(start)
	register int start;
{
	register int i;

	for (i = 0; i < break_count; ++i)
		if (break_point[i].addr == (break_t *) start)
			return (i);    /* got it! */
	return (-1);
}

get_watch(start)
	register int start;
{
	register int i;

	for (i = 0; i < watch_count; ++i)
		if (watch_point[i].addr == (int *) start)
			return (i);    /* got it! */
	return (-1);
}


set_break()
{
/*
 * put breakpoints into the code
 */
	register int i;

	printf("jdxxx set_break break_set %x\n",break_set);
	if (break_set)
		return;
	for (i = 0; i < break_count; ++i)
		*break_point[i].addr = BPT;  /* put in break point */
	break_set = 1;
}


rm_break()
{
/*
 * remove breakpoints
 * we presume that we can directly write the instructions that
 * are to be replaced. We do this for efficiency because we've
 * already checked when the inital breakpoint was set.
 */
	register int i;

	if (break_set)
		for (i = 0; i < break_count; ++i)
			*break_point[i].addr = break_point[i].instn;
	break_set = 0;
}


break_list()
{
/*
 * list breakpoints
 */
	register int i;
	int argc;

	printf("%d break points ...\n", break_count);
	for (i = 0; i < break_count; ++i)
		{
		printf("%6d",break_point[i].counter);
		printf(break_point[i].temp ? "T " : "  ");
		do_unasm((int)break_point[i].addr, 1, (char *) 0); /* show it */
		if (break_point[i].cmd[0])
			printf("        %s\n",break_point[i].cmd);
		}
}

static break_clear(start)
	int start;
{
/*
 * remove given breakpoint
 */
	register int i;

	if ((i = get_break(start)) >= 0)
		break_delete(i);
	else if ((i = get_watch(start)) >= 0)
		watch_delete(i);
	else 
		printf("no break or watch point at %x\n", start);
}

/* remove i'th break point by moving up the other breakpoint info */
static break_delete(i)
int i;
{
	--break_count;
	for (; i < break_count; ++i) {
		break_point[i] = break_point[i+1];
	}
}

/* remove i'th watch point by moving up the other watchpoint info */
static watch_delete(i)
int i;
{
	--watch_count;
	for (; i < watch_count; ++i) {
		watch_point[i] = watch_point[i+1];
	}
}


lookup(ptr, errflag, symtab)
	register char *ptr;
	int *errflag;
	struct symtab *symtab;
{
	register struct symtab *s;

	if (errflag)
		*errflag = 0;
	for (ALLSYMS(s))
		if (s->symbol[0] == ptr[0] &&
		    strcmp(s->symbol, ptr) == 0)
			return (s->value);
	if (errflag)
		*errflag = 1;
	else {
		err_flag++;	       /* global error flag */
		printf("%s not found\n", ptr);
	}
	return (0);
}

struct symtab *locate(ptr, errflag, symtab)
	register char *ptr;
	register int *errflag;
	register struct symtab *symtab;
{
	register struct symtab *s;

	if (errflag)
		*errflag = 0;
	for (ALLSYMS(s))
		if (strcmp(s->symbol, ptr) == 0)
			return (s);
	if (errflag)
		*errflag = 1;
	else {
		err_flag++;	       /* global error flag */
		printf("%s not found\n", ptr);
	}
	return (0);
}


define(ptr, symtab, value)
	register char *ptr;
	register struct symtab *symtab;
	register int value;
{
	register struct symtab *s;
	static char nullsymbol[MAXSYMLEN] = {0};

	for (ALLSYMS(s))
		if (strcmp(s->symbol, ptr) == 0)
			{
			printf("%s was %x, now %x\n",ptr,s->value,value);
			s->value = value;
			return;
			}
	if (s && bcmp(s->symbol,nullsymbol,MAXSYMLEN) == 0 &&
			bcmp((s+1)->symbol,nullsymbol,MAXSYMLEN) == 0)
		{
		strcpy(s->symbol, ptr);
		s->value = value;
		printf("%s defined as %x\n",ptr,s->value);
		}
	else
		printf("no room for more symbols\n");
	return (0);
}


/*
 * find closest symbol at or before the given address
 * return it's symbol table pointer.
 */
struct symtab *closest(addr, offset)
	register unsigned addr, offset;
{
	register struct symtab *s, *p = 0;
	register unsigned n;

	if ((symtab == 0 && symtab2 == 0) || (debug_option&SCAN_SYM))
		{
		if ((s = scansym(addr, offset)) != 0 && (n = addr - s->value) < offset) 
			offset = n, p = s;
		}
	for (ALLSYMS(s))
		if (s->value <= addr && (n = addr - s->value) < offset)
			offset = n, p = s;
	if (symtab2 && hfetch((int) symtab2) >= 0)
		for (ALLSYMS2(s))
			if (s->value <= addr && (n = addr - s->value) < offset)
				offset = n, p = s;
	return (p);
}


/*
 * find all the symbols begining with "str"
 * and print them out.
 */
lookup_sym(str,symtab)
	register char *str;
	register struct symtab *symtab;
{
	register struct symtab *s;
	register int len = strlen(str);
	int lines = 0;

	for (ALLSYMS(s))
		if (strncmp(s->symbol, str, len) == 0)
			{
			printf("%08x %s\n", s->value,s->symbol);
			if (!db_newline(&lines))
				break;
			}
}


#define MIN_ADDR 0x100	/* minimal address we print symbolicly */
int prsym(start, offset, fmt)
	register unsigned start, offset;
	register char *fmt;
{
	register unsigned nstart = start & iar_mask; /* get masked version */
	register struct symtab *s = closest(nstart, offset);

	if (fmt)
		printf(fmt, start);
	if (nstart >= MIN_ADDR && s) {
		printf("%s", s->symbol);
		if (s->value != nstart)
			printf("+0x%x ", nstart - s->value);
		else
			printf(" ");
	}
	return (s != 0);
}


watch_list()
{
/*
 * list watchpoints
 */
	register int i;

	printf("%d watch points ...\n", watch_count);
	for (i = 0; i < watch_count; ++i) {
		prsym((unsigned)watch_point[i].addr, db_offset, "%08x ");	/* show it */
		printf(" now %08x", *watch_point[i].addr);
		printf(" %s", pr_wtype(watch_point[i].type));
		printf(" %s",watch_point[i].cmd);
		printf("\n");
	}
}


badreg()
{
	printf("bad register number\n");
}

#include "i386/rdb/debug_mach.c"		/* get machine dependent part */

_debugger(state,cmd)
	register int state;
	char *cmd;
{
	register int argc;
	char *argv[MAX_ARG];
	char line[MAX_LINE];

	DEBUGX(printf("debugger\n"));
	if (debug_state != NORMAL_STATE)
		printf("HELP: recursive entry to debugger!\n");
	go_step = 0;		       /* just in case */
	debug_state = state;	       /* keep track of the current state */
#ifdef ibm032
	if (state != PM_STATE)
		internal[0].value = state;     /* KLUDGE the value of _iar */
#endif ibm032
	if (cmd) {				/* execute specified command */
		int lastcmd = lastdebugcmd, lastargs = lastcmdargs;
		int result;
		argv[0] = cmd;
		argc = 1;
		result = macro_cmd(argc,argv,1);
		lastdebugcmd = lastcmd;
		lastcmdargs = lastargs;
		if (result)
			return(result);		/* allow it to be a macro */
	}
	DEBUGX(printf("debugger: for\n"));
	for (;;) {
		printf("DEBUG> ");
		if (db_gets(line) == 0)
			return(0);
		argc = _parse(line, line, argv, (char **) 0);
		if (debug_state == NORMAL_STATE &&
		    (argc == 0 || strcmp(argv[0], "return") == 0))
			return(0);
		macro_cmd(argc, argv, 0);
	}
	return(0);
}

/*
 * take a string, parse it, and execute it
 * multiple commands separated by ";" are supported.
 */
debug_cmd(cmd, depth)
char *cmd;
int depth;
{
	char line[MAX_LINE];
	char *argv[MAX_ARG];
	int argc;

	do
		{
		argc = _parse(cmd, line, argv, &cmd);
		if (macro_cmd(argc, argv, depth))
			return(1);
	} while (*cmd++ == ';');
	return(0);
}

not_stopped()
{
	if (debug_state == PM_STATE)
		printf("Can't do that in POST MORTEM mode!\n");
	else
		printf("not in DEBUG mode\n");
}

#define SCAN_ASCII 0x0a		/* ascii specified */

/*
 * scan memory from "start" thru start+length-1 for the pattern specified
 * size indicates how many bytes are to be tested for.
 */

db_scan(start,length,size,argc,argv)
int start, length, size, argc;
char **argv;
{
	int i, c;
	char *ptest = (char *) &debug_pattern;
	int pattern_length = debug_patlen;
	int inc;

	if (size == SCAN_ASCII)
		{
		if (argc > 1)
			printf("only first string used for scan ascii");
		
		if (argc > 0)
			{
			pattern_length = strlen(argv[0]);
			if (pattern_length > sizeof debug_pattern)
				printf("pattern truncated to %d bytes\n", 
					pattern_length = sizeof debug_pattern);
			bcopy(argv[0], ptest, pattern_length);
			}
		inc = 1;
		}
	else
		{
		if (size != 1 && size != 2 && size != 4)
			{
			printf("scan size set to 1 (was %d)\n", size);
			size = 1;
			}
		if (argc > sizeof debug_pattern/size)
			argc = sizeof debug_pattern/size;	/* truncate it to fit */
		for (i=0; i<argc; )
			{
			c = ATOX(argv[i]);
			switch(size)
				{
			case 4:
				debug_pattern.words[i] = c;
				break;
			case 2:
				debug_pattern.halfs[i] = c;
				break;
			case 1:
				debug_pattern.bytes[i] = c;
				break;
				}
			pattern_length = ++i * size;
			}
		inc = size;
		}
	debug_patlen = pattern_length;
	printf("scanning from %x (%d bytes) to %x for",
		start,length,start+length-1);
	for (i=0; i<pattern_length; ++i)
		printf(" %02x", ptest[i]);
	printf("\n");

	for (length -= pattern_length-1;length > 0; (length -= inc),start += inc)
		{
		for (i=0; i<pattern_length; ++i)
			{
			c = bfetch((int)(start+i));
			if (c < 0)
				{
				printf("scan stopped %x (bad address)\n",start+i);
				goto done;
				}
			if (c != ptest[i])
				break;
			}
		if (i == pattern_length)
			{
			printf("scan matched at ",start);
			prsym(start, db_offset, "%08x ");
			printf("\n");
			break;
			}
		}
	if (i != pattern_length)
		printf("scan failed\n");
done:
	debug_scan = length;
	debug_size = size;
	debug_lastscan = start+size;
	return(start);
}
