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
static char *rcsid = "@(#)$RCSfile: file_magic.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/08/25 21:56:34 $";
#endif

/*
 * OSF/1 Release 1.0
 */
#include "magic_data.h"

Entry_init magic_tab[] = {
{ 0,	0,	917505,	4,	0,	00177555,	"very old pdp11 archive" },
{ 0,	0,	917506,	2,	0,	00177545,	"old pdp11 archive" },
{ 0,	0,	917507,	8,	0,	(long)"\r\002<\002a\002r\002>\013",	"System V archive" },
{ 0,	0,	917508,	8,	0,	(long)"\r\002!\002<\002a\002r\002c\002h\002>\002\n\002_\002_\002.\002S\002Y\002M\002D\002E\002F\013",	"archive random library" },
{ 0,	0,	917510,	8,	0,	(long)"\r\002!\002<\002a\002r\002c\002h\002>\002\n\002_\002_\002_\002_\002_\002_\002_\002_\002_\002_\002E\013",	"MIPS archive" },
{ 1,	20,	917510,	8,	0,	(long)"\r\002U\013",	"with mipsucode members" },
{ 1,	21,	917510,	8,	0,	(long)"\r\002L\013",	"with mipsel members" },
{ 1,	21,	917510,	8,	0,	(long)"\r\002B\013",	"with mipseb members" },
{ 1,	19,	917510,	8,	0,	(long)"\r\002L\013",	"and a EL hash table" },
{ 1,	19,	917510,	8,	0,	(long)"\r\002B\013",	"and a EB hash table" },
{ 1,	22,	917510,	8,	0,	(long)"\r\002X\013",	"-- out of date" },
{ 0,	0,	917511,	8,	0,	(long)"\r\002!\002<\002a\002r\002c\002h\002>\002\n\002_\002_\002_\002_\002_\002_\002_\002_\0026\0024\002E\013",	"Alpha archive" },
{ 1,	22,	917511,	8,	0,	(long)"\r\002X\013",	"-- out of date" },
{ 0,	0,	917509,	8,	0,	(long)"\r\002!\002<\002a\002r\002c\002h\002>\002\n\013",	"archive" },
{ 0,	0,	458756,	8,	64,	(long)"\r\002#\002!\007\002 \021\t\003 \n\021\013",	"%s" },
{ 1,	-2,	458756,	4,	64,	000,	"%sscript" },
{ 0,	0,	524297,	8,	0,	(long)"\017\002\031\002\001\013",	"suspend BSD lpd output filter" },
{ 0,	0,	786434,	2,	0,	00407,	"VAX executable (object file)" },
{ 1,	8,	786434,	2,	1,	000,	"not stripped" },
{ 0,	0,	786435,	4,	0,	00410,	"VAX pure" },
{ 1,	-2,	786435,	4,	64,	000,	"%sexecutable" },
{ 1,	16,	786435,	4,	1,	000,	"not stripped" },
{ 1,	-1,	786435,	4,	64,	000,	"%s" },
{ 0,	0,	786436,	2,	0,	00413,	"VAX demand paged pure" },
{ 1,	2,	786436,	2,	0,	002,	"POSIX" },
{ 1,	2,	786436,	2,	0,	001,	"SVID" },
{ 1,	-2,	786436,	4,	64,	000,	"%sexecutable" },
{ 1,	16,	786436,	4,	1,	000,	"not stripped" },
{ 1,	-1,	786436,	4,	64,	000,	"%s" },
{ 0,	0,	786433,	2,	0,	00401,	"Ultrix-11 Stand-alone or boot executable" },
{ 0,	0,	786437,	2,	0,	00430,	"Ultrix-11 overlay text kernel executable" },
{ 0,	0,	786438,	2,	0,	00431,	"Ultrix-11 user overlay (separated I&D) executable" },
{ 0,	0,	786439,	2,	0,	00450,	"Ultrix-11 overlay kernel executable" },
{ 0,	0,	786440,	2,	0,	00451,	"Ultrix-11 overlay kernel (separated I&D) executable" },
{ 0,	0,	458753,	8,	0,	(long)"\r\002\001\002h\006\f01234567899\006\f01234567899\006\f01234567899\006\f0123",	"sccsfile" },
{ 0,	0,	458757,	8,	0,	(long)"\r\002#\002i\002f\002n\002d\002e\002f\013",	"c program" },
{ 0,	0,	458858,	8,	0,	(long)"\r\0020\0027\0020\0027\0020\0027\013",	"ASCII cpio archive" },
{ 0,	0,	458763,	8,	64,	(long)"\r\002%\002!\002P\002S\002-\002A\002d\002o\002b\002e\002-\021\007\r.01234567899\021\002\n\013",	"PostScript (v%s) text" },
{ 0,	0,	458763,	8,	0,	(long)"\r\002%\002!\013",	"Unstructured PostScript text" },
{ 0,	0,	720897,	8,	0,	(long)"\r\002\377\002\377\002\177\013",	"ddis/ddif " },
{ 0,	0,	720905,	8,	0,	(long)"\r\002\377\002\377\002|\013",	"ddis/dots archive" },
{ 0,	0,	720906,	8,	0,	(long)"\r\002\377\002\377\002~\013",	"ddis/dtif table data" },
{ 0,	0,	720904,	8,	0,	(long)"\r\002\033\002c\002\033\013",	"LN03 output" },
{ 0,	0,	720898,	8,	0,	(long)"\r\002@\002\357\013",	"troff (CAT) output" },
{ 0,	0,	720899,	4,	0,	004553207,	"X image" },
{ 0,	0,	720900,	2,	0,	0017777,	"compacted data" },
{ 0,	0,	720901,	2,	0,	00116437,	"compressed data " },
{ 0,	0,	721002,	2,	0,	0070707,	"cpio archive" },
{ 0,	0,	720903,	2,	0,	0017037,	"packed data" },
{ 0,	0,	720902,	8,	0,	(long)"\r\002b\002e\002g\002i\002n\002 \006\f01234567899\013",	"uuencoded data" },
{ 0,	0,	720905,	4,	0,	00100200401,	"MMDF mailbox" },
{ 0,	0,	786441,	2,	0,	00542,	"mipsel" },
{ 1,	20,	786441,	2,	0,	00407,	"407" },
{ 1,	20,	786441,	2,	0,	00410,	"pure" },
{ 1,	20,	786441,	2,	0,	00413,	"demand paged" },
{ 1,	-2,	786441,	4,	64,	000,	"%sexecutable" },
{ 1,	8,	786441,	4,	1,	000,	"not stripped" },
{ 1,	8,	786441,	4,	2,	000,	"not stripped" },
{ 1,	23,	786441,	0,	68,	000,	"- version %d." },
{ 1,	22,	786441,	0,	68,	000,	"%d" },
{ 0,	0,	786442,	2,	0,	0061001,	"swapped mipsel" },
{ 1,	20,	786442,	2,	0,	003401,	"407" },
{ 1,	20,	786442,	2,	0,	004001,	"pure" },
{ 1,	20,	786442,	2,	0,	005401,	"demand paged" },
{ 1,	-2,	786442,	4,	64,	000,	"%sexecutable" },
{ 1,	8,	786442,	4,	1,	000,	"not stripped" },
{ 1,	8,	786442,	4,	2,	000,	"not stripped" },
{ 1,	22,	786442,	0,	68,	000,	"- version %d." },
{ 1,	23,	786442,	0,	68,	000,	"%d" },
{ 0,	0,	786443,	2,	0,	00600,	"mipseb ucode" },
{ 0,	0,	786444,	2,	0,	00602,	"mipsel ucode" },
{ 0,	0,	786445,	2,	0,	00617,	"alpha AD ucode" },
{ 0,	0,	786446,	2,	0,	00603,	"alpha AD" },
{ 1,	24,	786446,	2,	0,	00407,	"407" },
{ 1,	24,	786446,	2,	0,	00410,	"pure" },
{ 1,	24,	786446,	2,	0,	00413,	"demand paged" },
{ 1,	-2,	786446,	4,	64,	000,	"%sexecutable" },
{ 1,	8,	786446,	4,	1,	000,	"not stripped" },
{ 1,	8,	786446,	4,	2,	000,	"not stripped" },
{ 1,	27,	786446,	0,	68,	000,	"- version %d." },
{ 1,	26,	786446,	0,	68,	000,	"%d" },
{ 0,	0,	786447,	4,	0,	0031676567676,	"OSF/Rose object" },
{ 0,	0,	983041,	2,	0,	002401,	"locale data table" },
{ 1,	6,	983041,	2,	0,	0044,	"for MIPS" },
{ 1,	6,	983042,	2,	0,	00100,	"for Alpha" },
{ 0,	-4 }
};
