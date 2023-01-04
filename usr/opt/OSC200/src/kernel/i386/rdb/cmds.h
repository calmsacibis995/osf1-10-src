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
 *	@(#)$RCSfile: cmds.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:19 $
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
#define ASCII 1 /* ARG_1 ascii */
#define BREAK 2 /* ARG_2 break */
#define DISPMEM 3 /* ARG_3 display */
#define DEFINE 4 /* ARG_0 define */
#define DUMP 5 /* ARG_3 dump */
#define VERSION 6 /* ARG_0 version */
#define HELP 7 /* ARG_0 help */
#define QMARK 8 /* ARG_0 ? */
#define MOD 9 /* ARG_2 / */
#define MODHALF 10 /* ARG_2 /half */
#define MODBYTE 11 /* ARG_2 /byte */
#define MODWORD 12 /* ARG_2 /word */
#define CALL 13 /* ARG_3 call */
#define PCALL 14 /* ARG_3 pcall */
#define PRETURN 15 /* ARG_0 preturn */
#define CLEAR 16 /* ARG_1 clear */
#define IDENT 17 /* ARG_3 ident */
#define UNASM 18 /* ARG_2 unasm */
#define REGISTER 19 /* ARG_2 reg */
#define SCR 20 /* ARG_0 sysreg */
#define GO 21 /* ARG_1 go */
#define STEP 22 /* ARG_2 step */
#define SYMBOL 23 /* ARG_1 symbol */
#define SYMTAB 24 /* ARG_1 symtab */
#define MOVE 25 /* ARG_3 move */
#define DOLLAR 26 /* ARG_0 $ */
#define STATUS 27 /* ARG_0 status */
#define SHOW 28 /* ARG_1 show */
#define EQUALS 29 /* ARG_1 = */
#define STAR 30 /* ARG_0 * */
#define WATCH 31 /* ARG_1 watch */
#define LOOKUP 32 /* ARG_0 lookup */
#define TRACEBACK 33 /* ARG_3 trace */
#define CLS 34 /* ARG_0 cls */
#define HOME 35 /* ARG_0 home */
#define OPTION 36 /* ARG_2 option */
#define FILL 37 /* ARG_3 fill */
#define USERCMD 38 /* ARG_3 usercmd */
#define INB 39 /* ARG_2 inb */
#define OUTB 40 /* ARG_2 outb */
#define INHW 41 /* ARG_2 inhw */
#define OUTHW 42 /* ARG_2 outhw */
#define TERM 43 /* ARG_0 term */
#define MACRO 44 /* ARG_0 macro */
#define MACRODELETE 45 /* ARG_0 macrodelete */
#define MACROLIST 46 /* ARG_0 macrolist */
#define ECHO 47 /* ARG_0 echo */
#define IF 48 /* ARG_0 if */
#define WHILE 49 /* ARG_0 while */
#define REPEAT 50 /* ARG_0 repeat */
#define DEBUGREG 51 /* ARG_0 debugreg */
#define SHIFT 52 /* ARG_0 shift */
#define STOP 53 /* ARG_0 stop */
#define SCAN 54 /* ARG_3 scan */
#define PUT 55 /* ARG_0 put */
#define EDIT 56 /* ARG_0 edit */
#define SELECTOR 57 /* ARG_2 selector */
#define MAXCMD 57
