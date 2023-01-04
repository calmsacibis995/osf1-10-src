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
static char	*sccsid = "@(#)$RCSfile: opset.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:28 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
  Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

		All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <i386/kdb/defs.h>

typedef long		L_INT;
typedef unsigned long	ADDR;

char *sl_name;

char *regname[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
		    "eip", "flag", "trapno", "err",
	            "ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di",
	            "al",  "cl",  "dl",  "bl" , "ah",  "ch",  "dh",  "bh",
	            "st(0)", "st(1)", "st(2)", "st(3)",
	            "st(4)", "st(5)", "st(6)", "st(7)", "st", 0};

/*
 *	UNIX debugger
 *
 *		Instruction printing routines.
 *		MACHINE DEPENDENT.
 *		3B: dis_dot() in "dis" subdirectory;
 *		    routines take form 3B disassembler
 */

/* prassym(): symbolic printing of disassembled instruction */
prassym()
{
	extern long dot;
	extern INT dotinc;
	int cnt, regno, jj;
	long value;
	char rnam[9];
	register char *os;
	extern	char	mneu[];		/* in dis/extn.c */

#define os_bkup	0
	int len;
	char *pos;
	char *oldpos;
	char *regidx;
	int idxsize;
	extern char *strchr();

	/* depends heavily on format output by disassembler */
	printf("\t[");
	cnt = 0;
	os = mneu;	/* instruction disassembled by dis_dot() */
	while(*os != '\t' && *os != ' ' && *os != '\0')
		os++;		/* skip over instr mneumonic */
	while (*os) {
		while(*os == '\t' || *os == ',' || *os == ' ')
			os++;
		value = 0;
		regno = -1;
		rnam[0] = '\0';
		pos = os;
		idxsize = 0;
		switch (*os) {
		/*
		** This counts on disassembler not lying about
		** lengths of displacements.
		*/
		case '$':
			pos++;
			/* fall through */

		case '0':
			oldpos = pos;
			value = strtol(pos,&pos,0);
			len = pos - oldpos;
			if (len == 4) /* includes 0x; sign-extension */
				if (value & 0x80)
					value |= ~0xffL;
				else
					value &= 0xffL;
			else if (len == 6)
				value = (short) value;
			else ; /* either was a long or nothing */
			if (value == 0)
				jj = 0;
			else
				jj = 1;
			if (*pos != '(')
				break;
			/* fall through */

		case '(':
			if (pos[1] == '%') {
				char *cp;

				pos += 2;
				cp = rnam;
				while (*pos != ')' && *pos != ',')
					*cp++ = *pos++;
				*cp = '\0';
				if (jj<0)
					jj = 1;
				else
					jj++;
				if (*pos == ',') {
					idxsize = strchr(pos,')') - pos + 1;
					regidx = pos;
				}
			}
			break;

		case '%':
			if (*os == '%') {
				char *cp, *cp1;

				cp = os + 1;
				cp1 = rnam;
				while (*cp &&
				      (*cp != ')' && *cp != ',' &&
				       *cp != ' ' && *cp != '\t'))
					*cp1++ = *cp++;
				*cp1 = '\0';
			}
			break;

		case '+':
		case '-':
			while(*os != '\t' && *os != ' ' && *os != '\0')
				os++;
			jj = 0;
			if ((os[0] == ' ' || os[0] == '\t') && os[1] == '<') {
				char *cp;

				value = strtol(os + 2, &cp, 16);
				value += dot + dotinc;
				if (*cp == '>')
					jj = 1;
			}
			if (value == 0) /* probably a .o, unrelocated displacement*/
				jj = 0;
			break;

		default:
			jj = 0;
			break;
		}
		if (*rnam)
			for(jj = 0; regname[jj]; jj++)
				if (eqstr(rnam,regname[jj]))
					regno = jj;
		if(jj > 0) {
			if(cnt++ > 0)
				printf(",");
			jj = prsym(value, regno, 'i');
		}
		if (jj == (-1)) os -= os_bkup;
		while(*os != '\t' && *os != ',' && *os != ' ' && *os != '\0') {
			if(jj == (-1))
				printf("%c", *os);   /* just as is */
			os++;
		}
		/*
		** When there is a scale-index byte and there are additional commas
		** enclosed in the parenthesis, print the s-i-b out and skip
		** the associated operand in os.
		*/
		if (idxsize) {
			while (idxsize--) {
				if (*regidx == ',') {
					os++;
					while(*os != '\t' && *os != ',' && *os != ' ' && *os != '\0')
						os++;
				}
				printf("%c", *regidx++);
			}
		}
	} /* for */
	printf("]");
}


prdiff(diff) {
	if (diff) {
		printf("+");
		prhex(diff);
	}
}

adrtoext(val)
long val;
{
	extern char end;
	extern char *sl_name;
	extern struct nlist *cursym;

	if (val < VM_MIN_KERNEL_ADDRESS || val > (unsigned long)&end)
		return(-1);
	findsym(val, ISYM);
#ifdef	wheeze
	sl_name = cursym->n_name;
#else	wheeze
	if ( !(int) cursym)
		return (-1);
	sl_name = cursym->n_un.n_name;
#endif	wheeze
	return(val - cursym->n_value);
}

prsym(val, regno, fmt)
long val; char fmt; int regno; {
	struct proct *procp;
	register long diff = -1;

	if (fmt == 'i') {
		if (regno != -1)
		{
			return(-1);
		}

		if ((diff = adrtoext((ADDR) val)) != -1) {
			printf( "%s", sl_name );
			prdiff(diff);
			return(0);
		}
	}
	prhex(val);
	return(1);
}
