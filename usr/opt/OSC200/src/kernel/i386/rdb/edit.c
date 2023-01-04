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
static char	*sccsid = "@(#)$RCSfile: edit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:12 $";
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

/*
 * edit line 
 */

#define ctl(ch) ((ch)&037)

#define UP ctl('k')
#define DOWN ctl('j')
#define LEFT ctl('h')
#define RIGHT ctl('l')
#define INS ctl('i')
#define ENTER ctl('m')
#define DEL ctl('d')
#define ESC 0x1b

static pos;
static len;
static char *lineptr;
static ins;

#ifdef EDIT_TEST
edit_test()
{
	char line[80];
	printf("enter line to edit: ");
	gets(line);
	db_edit(line);
	printf("line=%s\n", line);
}
#endif /* EDIT_TEST */

db_edit(line)
char *line;
{
	int ch;
	int x;
	int result = 0;

	ins = 0;
	lineptr = line;
	len = strlen(line);
	pos = len;	/* position at end of line */
	printf("%s", line);

	for (;;)
	{
		ch = token();

		switch(ch)
		{
		case 0:
			break;
		case LEFT:
			posn(-1);
			break;
		case RIGHT:
			posn(1);
			break;
		case INS:
			ins = !ins;
			break;
		case DEL:
			x = len-pos;
			if (x <= 0)
				break;
			bcopy(line+pos+1, line+pos, x);
			if (len > 0)
				--len;
			line[len] = 0;
			printf("%s \b",line+pos);
			pos = len;	/* now at end of line */
			posn(-x+1);	/* reposition cursor */
			result |= EDIT_CHANGE;
			break;
		case UP:
			printf("\n");
			return(result|EDIT_UP);
		case DOWN:
		case ENTER:
			printf("\n");
			return(result|EDIT_DOWN);
		default:
			if (ins)
				{
				/* to insert we copy bytes from pos down
				 * and insert a new character pos
				 */
				x = len-pos;
				if (x > 0)
					db_bcopy(line+pos, line+pos+1, x+1);
				line[pos] = ch;
				line[++len] = 0;
				printf("%s",line+pos);
				pos = len;	/* now at end of line */
				posn(-x);	/* reposition cursor */
				}
			else
				{
				line[pos] = ch;
				putchar(line[pos++]);
				}
			if (pos > len)
				{
				len = pos;
				line[len] = 0;
				}
			result |= EDIT_CHANGE;
			break;
		}
		
	}
}

static posn(n)
int n;
{
	if (n > 0)
		{
		for (;n > 0 && pos < len; --n)
			putchar(lineptr[pos++]);
		}
	else if (n < 0)
		{
		for (; n < 0 && pos > 0; ++n, --pos)
			putchar('\b');
		}
}


static token()
{
	int c = _getchar();

	if (c == ESC)
		{
		c = _getchar();
		if (c == '[')
			{
			int num = 0;
			while ((c = getchar()) && '0' <= c && c <= '9')
				num = num * 10 + c - '0';
			if (c == 'A')
				return(UP);
			if (c == 'B')
				return(DOWN);
			if (c == 'C')
				return(RIGHT);
			if (c == 'D')
				return(LEFT);
			if (c == 'P')
				return(DEL);
			if (c == 'q' && num == 139)
				return(INS);	/* AIX ins key */
			if (c == '~' && num == 2)
				return(INS);	/* xterm ins key */
			}
		return(0);
		}
	if (c == 0x7f)
		c = DEL;
	return(c);
}

/*
 * bcopy that allows "from" to overlap "to".
 */
static db_bcopy(from,to,length)
char *from, *to;
int length;
{

	to += length;
	from += length;

	while (length-- > 0)
		*--to = *--from;
}
