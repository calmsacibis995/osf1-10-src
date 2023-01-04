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
 *	@(#)$RCSfile: macros.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:35 $
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
/*
 * this file defines the various default macros provided
 * change this for personal preferences
 * what's sneaky is that this file is included multiple times
 * once per data structure to be initialized.
 * note use of \0 to separate strings. we require a double \0 at
 * the end of a macro's definition.
 */

#ifdef ibm032
DEFINE("next","#increment iar past instruction\0"
	"unasm iar 1\0"
	"deb V0 .\0"
	"= V0-iar \"iar advanced %d bytes (${1-1} instruction(s))\"\0"
	"r iar V0\0"
	"u iar 1\0\0")
#endif /* ibm032 */

#ifdef i386
DEFINE("$","#status display\0"
	"echo -n 'Status: '\0"
	"\\status\0"
	"reg\0"
	"if eip!=0 'u eip 1'\0\0")
DEFINE("next","#[count]	increment eip past N instructions\0"
	"unasm eip ${1-1}\0"
	"deb V0 .\0"
	"= V0-eip \"eip advanced %d bytes (${1-1} instruction(s))\"\0"
	"r eip V0\0"
	"u eip 1\0\0")
DEFINE("RET", "#	go until fn return\0"
	"if (eip!1)==0x55 'break esp!4 -1' 'break ebp!4 -1'\0"
	"go\0\0")
DEFINE("usym","#	turn user symtab on\0"
	"deb option option|0x400\0"
	"deb symtab2 0x400000\0\0")
DEFINE("ksym","#	turn kernel symtab on\0"
	"deb option option|0x400-0x400\0\0")
DEFINE("status","#	status display with unasm\0"
	"\\status\0"
	"if eip!=0 'unasm eip 1'\0\0")
#ifdef MACH
DEFINE("pname","#pname ... process name\0"
	"ascii U_ADDRESS+4!4+4\0\0")
DEFINE("tss", "#tss addr ... display addr as a tss\0"
	"if ($1+0)==0 \"stop 'tss: no argument specified'\"\0"
	"= $1!2 'link=%x'\0"
	"= $1+4!4 'esp0=%x'\0"
	"= $1+8!2 'ss0=%x'\0"
	"= $1+1c!4 'cr3=%x'\0"
	"= $1+20!4 'eip=%x'\0"
	"= $1+24!4 'eflags=%x'\0"
	"= $1+28!4 'eax=%x'\0"
	"= $1+2c!4 'ecx=%x'\0"
	"= $1+30!4 'edx=%x'\0"
	"= $1+34!4 'ebx=%x'\0"
	"= $1+38!4 'esp=%x'\0"
	"= $1+3c!4 'ebp=%x'\0"
	"= $1+40!4 'esi=%x'\0"
	"= $1+44!4 'edi=%x'\0"
	"= $1+48!2 'es=%x'\0"
	"= $1+4c!2 'cs=%x'\0"
	"= $1+50!2 'ss=%x'\0"
	"= $1+54!2 'ds=%x'\0"
	"= $1+60!2 'ldt=%x'\0"
	"\0")
#endif /* MACH */
#endif /* i386 */

DEFINE("dw", "#[[addr] length] display words\0"
	"display ${1-.} ${2-10} ${3-4}\0\0")
DEFINE("dh", "#[[addr] length] display halfwords\0"
	"display ${1-.} ${2-10} ${3-2}\0\0")
DEFINE("db", "#[[addr] length] display bytes\0"
	"display ${1-.} ${2-10} ${3-1}\0\0")
DEFINE("scana","#address count 'ascii' scan for ascii text\0"
	"if $#>2 "
	"\"scan ${1-scan} ${2-scanlen} 0xa '$3' $4\" "
	"\"scan ${1-scan} ${2-scanlen} 1\"" /* otherwise repeat previous */
	"\0\0")
DEFINE("scanb","#address count bytes ... scan for bytes\0"
	"scan ${1-scan} ${2-scanlen} 1 $3 $4 $5 $6 $7 $8 $9\0\0")
DEFINE("scanh","#address count halfs ... scan for 16 bit halfwords\0"
	"scan ${1-scan} ${2-scanlen} 2 $3 $4 $5 $6 $7 $8 $9\0\0")
DEFINE("scanw","#address count words ... scan for 32 bit words\0"
	"scan ${1-scan} ${2-scanlen} 4 $3 $4 $5 $6 $7 $8 $9\0\0")

#include "i386/rdb/localmacros.h"
