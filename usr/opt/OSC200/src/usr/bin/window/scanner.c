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
static char	*sccsid = "@(#)$RCSfile: scanner.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:11:39 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	scanner.c	3.11 (Berkeley) 6/29/88
 */


#include <stdio.h>
#include "value.h"
#include "token.h"
#include "context.h"
#include "string.h"

s_getc()
{
	register c;

	switch (cx.x_type) {
	case X_FILE:
		c = getc(cx.x_fp);
		if (cx.x_bol && c != EOF) {
			cx.x_bol = 0;
			cx.x_lineno++;
		}
		if (c == '\n')
			cx.x_bol = 1;
		return c;
	case X_BUF:
		if (*cx.x_bufp != 0)
			return *cx.x_bufp++ & 0xff;
		else
			return EOF;
	}
	/*NOTREACHED*/
}

s_ungetc(c)
{
	if (c == EOF)
		return EOF;
	switch (cx.x_type) {
	case X_FILE:
		cx.x_bol = 0;
		return ungetc(c, cx.x_fp);
	case X_BUF:
		if (cx.x_bufp > cx.x_buf)
			return *--cx.x_bufp = c;
		else
			return EOF;
	}
	/*NOTREACHED*/
}

s_gettok()
{
	char buf[100];
	register char *p = buf;
	register c;
	register state = 0;

loop:
	c = s_getc();
	switch (state) {
	case 0:
		switch (c) {
		case ' ':
		case '\t':
			break;
		case '\n':
		case ';':
			cx.x_token = T_EOL;
			state = -1;
			break;
		case '#':
			state = 1;
			break;
		case EOF:
			cx.x_token = T_EOF;
			state = -1;
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z':
		case '_': case '.':
			*p++ = c;
			state = 2;
			break;
		case '"':
			state = 3;
			break;
		case '\'':
			state = 4;
			break;
		case '\\':
			switch (c = s_gettok1()) {
			case -1:
				break;
			case -2:
				state = 0;
				break;
			default:
				*p++ = c;
				state = 2;
			}
			break;
		case '0':
			cx.x_val.v_num = 0;
			state = 10;
			break;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			cx.x_val.v_num = c - '0';
			state = 11;
			break;
		case '>':
			state = 20;
			break;
		case '<':
			state = 21;
			break;
		case '=':
			state = 22;
			break;
		case '!':
			state = 23;
			break;
		case '&':
			state = 24;
			break;
		case '|':
			state = 25;
			break;
		case '$':
			state = 26;
			break;
		case '~':
			cx.x_token = T_COMP;
			state = -1;
			break;
		case '+':
			cx.x_token = T_PLUS;
			state = -1;
			break;
		case '-':
			cx.x_token = T_MINUS;
			state = -1;
			break;
		case '*':
			cx.x_token = T_MUL;
			state = -1;
			break;
		case '/':
			cx.x_token = T_DIV;
			state = -1;
			break;
		case '%':
			cx.x_token = T_MOD;
			state = -1;
			break;
		case '^':
			cx.x_token = T_XOR;
			state = -1;
			break;
		case '(':
			cx.x_token = T_LP;
			state = -1;
			break;
		case ')':
			cx.x_token = T_RP;
			state = -1;
			break;
		case ',':
			cx.x_token = T_COMMA;
			state = -1;
			break;
		case '?':
			cx.x_token = T_QUEST;
			state = -1;
			break;
		case ':':
			cx.x_token = T_COLON;
			state = -1;
			break;
		case '[':
			cx.x_token = T_LB;
			state = -1;
			break;
		case ']':
			cx.x_token = T_RB;
			state = -1;
			break;
		default:
			cx.x_val.v_num = c;
			cx.x_token = T_CHAR;
			state = -1;
			break;
		}
		break;
	case 1:				/* got # */
		if (c == '\n' || c == EOF) {
			(void) s_ungetc(c);
			state = 0;
		}
		break;
	case 2:				/* unquoted string */
		switch (c) {
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z':
		case '_': case '.':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (p < buf + sizeof buf - 1)
				*p++ = c;
			break;
		case '"':
			state = 3;
			break;
		case '\'':
			state = 4;
			break;
		case '\\':
			switch (c = s_gettok1()) {
			case -2:
				(void) s_ungetc(' ');
			case -1:
				break;
			default:
				if (p < buf + sizeof buf - 1)
					*p++ = c;
			}
			break;
		default:
			(void) s_ungetc(c);
		case EOF:
			*p = 0;
			cx.x_token = T_STR;
			switch (*buf) {
			case 'i':
				if (buf[1] == 'f' && buf[2] == 0)
					cx.x_token = T_IF;
				break;
			case 't':
				if (buf[1] == 'h' && buf[2] == 'e'
				    && buf[3] == 'n' && buf[4] == 0)
					cx.x_token = T_THEN;
				break;
			case 'e':
				if (buf[1] == 'n' && buf[2] == 'd'
				    && buf[3] == 'i' && buf[4] == 'f'
				    && buf[5] == 0)
					cx.x_token = T_ENDIF;
				else if (buf[1] == 'l' && buf[2] == 's')
					if (buf[3] == 'i' && buf[4] == 'f'
					    && buf[5] == 0)
						cx.x_token = T_ELSIF;
					else if (buf[3] == 'e' && buf[4] == 0)
						cx.x_token = T_ELSE;
				break;
			}
			if (cx.x_token == T_STR
			    && (cx.x_val.v_str = str_cpy(buf)) == 0) {
				p_memerror();
				cx.x_token = T_EOF;
			}
			state = -1;
			break;
		}
		break;
	case 3:				/* " quoted string */
		switch (c) {
		case '\n':
			(void) s_ungetc(c);
		case EOF:
		case '"':
			state = 2;
			break;
		case '\\':
			switch (c = s_gettok1()) {
			case -1:
			case -2:	/* newlines are invisible */
				break;
			default:
				if (p < buf + sizeof buf - 1)
					*p++ = c;
			}
			break;
		default:
			if (p < buf + sizeof buf - 1)
				*p++ = c;
			break;
		}
		break;
	case 4:				/* ' quoted string */
		switch (c) {
		case '\n':
			(void) s_ungetc(c);
		case EOF:
		case '\'':
			state = 2;
			break;
		case '\\':
			switch (c = s_gettok1()) {
			case -1:
			case -2:	/* newlines are invisible */
				break;
			default:
				if (p < buf + sizeof buf - 1)
					*p++ = c;
			}
			break;
		default:
			if (p < buf + sizeof buf - 1)
				*p++ = c;
			break;
		}
		break;
	case 10:			/* got 0 */
		switch (c) {
		case 'x':
		case 'X':
			cx.x_val.v_num = 0;
			state = 12;
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7':
			cx.x_val.v_num = c - '0';
			state = 13;
			break;
		case '8': case '9':
			cx.x_val.v_num = c - '0';
			state = 11;
			break;
		default:
			(void) s_ungetc(c);
			state = -1;
			cx.x_token = T_NUM;
		}
		break;
	case 11:			/* decimal number */
		switch (c) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			cx.x_val.v_num = cx.x_val.v_num * 10 + c - '0';
			break;
		default:
			(void) s_ungetc(c);
			state = -1;
			cx.x_token = T_NUM;
		}
		break;
	case 12:			/* hex number */
		switch (c) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			cx.x_val.v_num = cx.x_val.v_num * 16 + c - '0';
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			cx.x_val.v_num = cx.x_val.v_num * 16 + c - 'a' + 10;
			break;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			cx.x_val.v_num = cx.x_val.v_num * 16 + c - 'A' + 10;
			break;
		default:
			(void) s_ungetc(c);
			state = -1;
			cx.x_token = T_NUM;
		}
		break;
	case 13:			/* octal number */
		switch (c) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7':
			cx.x_val.v_num = cx.x_val.v_num * 8 + c - '0';
			break;
		default:
			(void) s_ungetc(c);
			state = -1;
			cx.x_token = T_NUM;
		}
		break;
	case 20:			/* got > */
		switch (c) {
		case '=':
			cx.x_token = T_GE;
			state = -1;
			break;
		case '>':
			cx.x_token = T_RS;
			state = -1;
			break;
		default:
			(void) s_ungetc(c);
			cx.x_token = T_GT;
			state = -1;
		}
		break;
	case 21:			/* got < */
		switch (c) {
		case '=':
			cx.x_token = T_LE;
			state = -1;
			break;
		case '<':
			cx.x_token = T_LS;
			state = -1;
			break;
		default:
			(void) s_ungetc(c);
			cx.x_token = T_LT;
			state = -1;
		}
		break;
	case 22:			/* got = */
		switch (c) {
		case '=':
			cx.x_token = T_EQ;
			state = -1;
			break;
		default:
			(void) s_ungetc(c);
			cx.x_token = T_ASSIGN;
			state = -1;
		}
		break;
	case 23:			/* got ! */
		switch (c) {
		case '=':
			cx.x_token = T_NE;
			state = -1;
			break;
		default:
			(void) s_ungetc(c);
			cx.x_token = T_NOT;
			state = -1;
		}
		break;
	case 24:			/* got & */
		switch (c) {
		case '&':
			cx.x_token = T_ANDAND;
			state = -1;
			break;
		default:
			(void) s_ungetc(c);
			cx.x_token = T_AND;
			state = -1;
		}
		break;
	case 25:			/* got | */
		switch (c) {
		case '|':
			cx.x_token = T_OROR;
			state = -1;
			break;
		default:
			(void) s_ungetc(c);
			cx.x_token = T_OR;
			state = -1;
		}
		break;
	case 26:			/* got $ */
		switch (c) {
		case '?':
			cx.x_token = T_DQ;
			state = -1;
			break;
		default:
			(void) s_ungetc(c);
			cx.x_token = T_DOLLAR;
			state = -1;
		}
		break;
	default:
		abort();
	}
	if (state >= 0)
		goto loop;
	return cx.x_token;
}

s_gettok1()
{
	register c;
	register n;

	c = s_getc();			/* got \ */
	switch (c) {
	case EOF:
		return -1;
	case '\n':
		return -2;
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	default:
		return c;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7':
		break;
	}
	n = c - '0';
	c = s_getc();			/* got \[0-7] */
	if (c < '0' || c > '7') {
		(void) s_ungetc(c);
		return n;
	}
	n = n * 8 + c - '0';
	c = s_getc();			/* got \[0-7][0-7] */
	if (c < '0' || c > '7') {
		(void) s_ungetc(c);
		return n;
	}
	return n * 8 + c - '0';
}
