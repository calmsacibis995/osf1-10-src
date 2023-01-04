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
#if !defined(lint) && defined(SECONDARY)
static char	*sccsid = "@(#)$RCSfile: io.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/04/09 13:35:48 $";
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
 * io.c
 */
/* 
 * derived from io.c	3.1	(ULTRIX/OSF)	2/28/91";
 */

#include <sys/param.h>
#include <ufs/inode.h>
#include "saio.h"

int	prom_io = -1;
int	devpart = 0;

#ifdef mips
#define printf _prom_printf
extern  int rex_base;
extern  int ub_argc;
extern  char **ub_argv;
#endif /* mips */

#ifdef __alpha
#define _prom_open prom_open
#endif

devread(io)
        register struct iob *io;
{
#ifdef mips
	int i;
	char *cp;

	if (rex_base) {
		for(i=1; i < ub_argc; i++) {
			if(ub_argv[i][0] == '-') {
				cp = &ub_argv[i][1]; 
				switch (*cp++) {
				case 'b':
				   i++;
				   while(ub_argv[i][0] == NULL) {
					i++;
				        if(i >= ub_argc)
					  rex_rex('h');
				   }
				   if(ub_argv[i][0] == '0') { 
				     i = ub_argc;
				     break;
				   } else {
				     io->i_bn += dctob(ub_argv[i]);
				     i = ub_argc;
				     break;
				   }
				 }
			}
	        }
		return(rex_bootread(io->i_bn, io->i_ma, io->i_cc));
	}
	else {
		_prom_lseek(prom_io, io->i_bn*512, 0);
		return (_prom_read(prom_io, io->i_ma, io->i_cc));
	}
#else /* __alpha */
	return (prom_read(prom_io, io->i_cc, io->i_ma,io->i_bn));
#endif /* mips */
}

#define RPAREN ')'
#define LPAREN '('

int
devopen(str, flag)
register char *str;
register int flag;
{
#if LABELS
/*
 * the following code is ifdef'd out to
 * eliminate boot string format of devtype(ctrlr,unit)(partition)file.
 * this code elimination effectively wires the partition to
 * always be the a partition (LBN 0) for LABEL'd boots (disks).
 */
#ifdef notdef
register char *cp;
char devname[16];
	/* Find the partition designator in the device string.
	 * this parser assumes it is handed valid input, and
	 * does very little error checking (after all, the ROMs
	 * have already placed their restrictions on this string
	 * anyway, and we have to live in the primary bootstrap).
	 */
	cp = str;
	str = devname;
	/* skip through to (past) first close paren */
	while (*cp && (*cp != RPAREN)) *str++ = *cp++;
	*str++ = ','; *str++ = 'c'; *str++ = RPAREN; *str = '\0';
	str = devname;

	/* cp points either at a NULL, or a right paren */

	/* Check for partition designator. */
	if (*cp && (*(cp + 1) == LPAREN)) {
		cp += 2;
		if ((*cp >= '0') && (*cp <= '7')) {
			devpart = *cp - '0';
		} else if ((*cp >= 'a') && (*cp <= 'h')) {
			devpart = *cp - 'a';
		} else {
			printf("Bad partition specification '%c'.\n", *cp);
			return (-1);
		}
	}
#endif /* notdef */
#if SECONDARY
	if (prom_io >= 0) {
		_prom_close(prom_io);
	}
#endif /* SECONDARY */
#endif /* LABELS    */
	prom_io = _prom_open(str, flag);
	return (prom_io);
}

#ifdef mips
/*
 * Functional Discription:
 * 	Convert an ASCII string of decimal characters to a binary number.
 *
 * Inputs:
 *	pointer to string
 *
 * Outputs:
 *	value (int)
 *
 */

dctob(str)
	char *str;
{
	register int hexnum;
	if (str == NULL || *str == NULL)
		return(0);
	hexnum = 0;
	if (str[0] == '0' && str[1] == 'x')
		str = str + 2;
	for ( ; *str; str++) {
		if (*str >= '0' && *str <= '9')
			hexnum = hexnum * 10 + (*str - 48);
/*		else if (*str >= 'a' && *str <= 'f')
			hexnum = hexnum * 10 + (*str - 87);
		else if (*str >= 'A' && *str <= 'F')
			hexnum = hexnum * 10 + (*str - 55);
*/
	}
	return (hexnum);
}
#endif /* mips */
