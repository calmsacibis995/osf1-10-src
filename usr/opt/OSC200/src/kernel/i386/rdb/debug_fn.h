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
 *	@(#)$RCSfile: debug_fn.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:51 $
 */ 
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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#ifndef _H_DEBUG_FN
#define _H_DEBUG_FN
/*
 * COMPONENT_NAME: (sysdb) Kernel debugger
 *
 * FUNCTIONS: (none)
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
 
/*=====================================================================*/
/* This file is:        /sys/db/debug_fn.h                             */
/* This file uses:      (None)                                         */
/* This file used by:   /sys/db/debug.h                                */
/* This file used as:   #include "../db/debug.h"                       */
/*=====================================================================*/
 
#if defined(__STDC__) && !defined(NO_PROTOTYPES)
 
/* from debug.c and debug_machine.c */
extern int help_cmd(
	int n);
 
extern int get_cmd(
	char *str,
	int *nargs);
 
extern int dump_cmd(
	int argc,
	char **argv,
	int depth);
 
extern int edit_break_watch(
	int type,
	char *name,
	int count);
 
extern int debug_print(
	int debug_state);
 
extern int ascii(
	int start,
	int count);
 
extern int db_newline(
	int *lineptr);
 
extern int display(
	word *start,
	int count,
	word *addr,
	int dispsize);
 
extern int dump_mem(
	int *start,
	int count,
	int incr);
 
extern int modify(
	int start,
	int addr,
	int count,
	int length,
	char **argv);
 
extern int db_pause(
	void);
 
extern int expr(
	char *ptr,
	int base);
 
extern int _expr(
	char **pptr,
	int base);
 
extern int item(
	char **pptr,
	int base);
 
extern int db_index(
	char *name);
 
extern int str_const(
	char **pptr);
 
extern int expr_fn(
	char **pptr,
	int base,
	int (*fn_addr)());
 
extern int do_unasm(
	int start,
	int count,
	char *string);
 
extern int db_search(
	char *str1,
	char *str2);
 
extern int ident(
	unsigned int p,
	unsigned int e,
	int flag);
 
extern int bad_addr(
	void);
 
extern int break_cmd(
	int start,
	int count,
	char *cmd);
 
extern int watch_cmd(
	int start,
	char *how,
	char *cmd);
 
extern int get_break(
	int start);
 
extern int get_watch(
	int start);
 
extern int set_break(
	void);
 
extern int rm_break(
	void);
 
extern int break_list(
	void);
 
extern int db_lookup(
	char *ptr,
	int *errflag,
	struct symtab *symtab);
 
extern struct symtab *locate(
	char *ptr,
	int *errflag,
	struct symtab *symtab);
 
extern int define(
	char *ptr,
	struct symtab *symtab,
	int value);
 
extern struct symtab *closest(
	unsigned int addr,
	unsigned int offset);
 
extern int lookup_sym(
	char *str,
	struct symtab *symtab);
 
extern int prsym(
	unsigned int start,
	unsigned int offset,
	char *fmt);
 
extern int watch_list(
	void);
 
extern int badreg(
	void);
 
extern struct symtab *scansym(
	int addr,
	int offset);
 
extern int tlbprint(
	void);
 
extern int call(
	FN fn,
	int arg1,
	int arg2,
	int arg3);
 
extern int pcall(
	FN fn,
	int arg1,
	int arg2,
	int arg3);
 
extern int pcall_done(
	int type);
 
extern int preturn(
	void);
 
extern int getreg(
	int reg);
 
extern int printregs(
	int state);
 
extern int print_reg(
	char *name,
	struct symtab *sym,
	int (*fn)());
 
extern int print_registers(
	struct symtab *sym,
	int reg_mask,
	int (*fn)());
 
extern int set_reg(
	char *regname,
	int new);
 
extern int sys_regs(
	void);
 
extern int sys_reg(
	char *regname,
	int new);
 
extern int debug_reg(
	int reg);
 
extern int debug_regs(
	void);
 
extern int set_debug_reg(
	char *regname,
	int value);
 
extern int debug_status(
	int state);
 
extern int show_instn(
	int debug_state);
 
extern int print_seg_except(
	unsigned long error_code);
 
extern int print_page_except(
	unsigned long error_code);
 
extern int step(
	int count);
 
extern int restart(
	int v,
	int t);
 
extern int trap(
	int type,
	unsigned long error);
 
extern int debug_go(
	int flag,
	int dot);
 
extern int xprsym(
	unsigned int start,
	unsigned int offset,
	char *fmt);
 
extern int set_watch(
	void);
 
extern int cvt_watch_type(
	char *how);
 
char * pr_wtype(
	int type);
 
extern struct symtab *next_sym(
	struct symtab *s);
 
extern int _debugger(
	int state,
	char *cmd);
 
extern int debug_cmd(
	char *cmd,
	int depth);
 
extern int not_stopped(
	void);
 
extern int db_scan(
	int start,
	int length,
	int size,
	int argc,
	char **argv);
 
/* from atox.c */
extern int atox(
	char *ptr);
/* from edit.c */
 
extern int db_edit(
	char *line);
 
extern int edit_macro(
	char *name);
 
/* from gets.c */
 
extern char *gets(
	char *buf);
/* from macro.c */
 
extern int macro_cmd(
	int argc,
	char **argv,
	int depth);
 
extern int macro_expand(
	char *source,
	char *dest,
	int argc,
	char **argv);
 
extern void macro_define(
	char *name);
 
extern void macro_delete(
	char *name);
 
extern void macro_list(
	char *name);
 
extern int macro_display(
	struct symtab *s);
 
macro_help(
	char *name);
 
extern char *put_line(
	char *line);
 
extern char *db_gets(
	char *line);
/* from parse.c */
 
extern int _parse(
	char *parm,
	char *space,
	char **argv,
	char **lastparm);
/* from puts.c */
 
extern int puts(
	char *str);
/* from sadebug.c */
 
extern int debugger_init(
	void);
 
extern int foo(
	void);
 
extern void prgdt(
	struct gdt *gdt,
	int count);
 
extern void prdtentry(
	struct gdt *dt,
	int selector);
 
extern int makedes(
	struct gdt *gdt,
	unsigned int vector,
	unsigned int type,
	unsigned int base,
	unsigned int limit);
 
extern void setgate(
	struct idt *idt,
	int vector,	/* see /sys/ios/i386.h for TRP_... symbolics */
	int type,
	int count,
	int selector,
	int offset );	/* used	as: (int) &function */
 
extern int lps(
	void);
 
extern int wfetch(
	int addr);
 
extern int hfetch(
	int addr);
 
extern int bfetch(
	int addr);
 
extern int wstore(
	int addr,
	unsigned int value);
 
extern int hstore(
	int addr,
	int hvalue);
 
extern int bstore(
	int addr,
	int bvalue);
 
extern int mfsr(
	int reg);
 
extern int mtsr(
	int reg,
	int value);
 
extern int callabs(
	void);
 
extern int _init_vectors(
	int n);
 
extern int init_kbd(
	void);
 
extern int put_status(
	int pos,
	char *str);
 
extern int fix_internal(
	void);
 
extern int vtop(
	unsigned int where);
 
extern void save_screen(
	char *buffer,
	char *save,
	int size);
 
extern void screen_restore(
	char *buffer,
	char *save,
	int size);
 
extern int INW(
	int);
 
extern int MFSR(
	int);
 
extern int MTSR(
	int,
	int);
 
extern int OUTW(
	int,
	int);
 
extern int ___Fp_Init(
	void);
 
extern int db_delay(
	void);
 
extern int prhex(
	unsigned int value);
 
extern int putchar(
	int c);
 
extern int debugswap(
	void);
 
extern int db_reboot(
	void);
 
extern int getchar(
	void);
 
extern int _getchar(
	void);
 
extern int make_debug_idt(
	int cs);
 
extern int set_regsave(
	void);
 
extern int pridtentry(
	int selector);
 
extern int prgdtentry(
	int selector);
 
extern int get_cs_size(
	void);
 
extern int fixsym(
	struct symtab *sym);
/* from trace.c */
 
extern int traceback(
	int ip,
	int bp,
	int count);
/* from unasm.c */
 
extern unsigned int printop(
	unsigned int addr,
	char *buff);
 
extern int getnumargs(
	unsigned int addr);
 
extern int xprintf(
	char *fmt,
	...);
 
/* from printf.c */
extern int printf(
	char *fmt,
	...);
 
extern int sprintf(
	char *string,
	char *format,
	...);
 
/* from parse.c */
extern int _parse(
	char *parm,
	char *space,
	char **argv,
	char **lastparm);
#endif
 
#endif
