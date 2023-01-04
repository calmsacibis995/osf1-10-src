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
static char	*sccsid = "@(#)$RCSfile: trace.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:15 $";
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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#include "i386/rdb/debug.h"

#define ADDL 0x83
#define SP_ADDL 0xc4

struct symtab *closest();
int trflag = 0;

/* heuristic to determine if a given address is virtual or real */
#define ISVIRTUAL(addr) ((unsigned) (addr) >= 0x1000000)

traceback(ip,bp,count)
int ip, bp, count;
{
	int x;
	int level;
	struct symtab *sym;
	int offset;
	extern int err_flag;
	extern int iar_mask;
	extern db_min_args;
	int entry;
	int lines = 0;
	int nbytes;		/* number of bytes of args */

	for (level=0;err_flag == 0 && level < count; ++level)
	{
		printf("level %d: ip=%x bp=%x ", level,  ip, bp);
		if ((ip & iar_mask) == 0)
			{
			printf("\n");
			break;
			}
		sym = closest(ip & iar_mask,db_offset);	/* get closest entry point */
		if (sym == 0)
			{
			printf("no symbol found near %x\n",ip);
			break;
			}
		entry = sym->value;
		offset = (ip & iar_mask) - entry;
		printf(" routine %s + %d (0x%x)\n", sym->symbol,offset,offset);
		++lines;
		if (bp == 0)
			break;
		x = wfetch(bp);
		if (x == -1)
			break;
		ip = wfetch(bp+4);
		if (ip == -1)
			break;
		printf("	%s(",sym->symbol);
		if (bfetch(ip) == ADDL && bfetch(ip+1) == SP_ADDL)
			nbytes = bfetch(ip+2);
		else
			nbytes = db_min_args << 2;
		if (nbytes > 0)
			{
			int i;
			for (i=0; i<nbytes; i += 4)
				{
				int temp = wfetch(bp+4+4+i);
				printf("%s%x",i ? "," : "" ,temp);
				}
			}
		printf(")\n");
		if (!db_newline(&lines))
			break;
		bp = x;
	}

}
