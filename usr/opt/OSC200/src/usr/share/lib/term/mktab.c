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
static char	*sccsid = "@(#)$RCSfile: mktab.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/12/11 14:07:05 $";
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
 * Copyright (c) 1989 Regents of the University of California.
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
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * mktab.c
 *
 * Function:	Build nroff terminal tables in a compiler-independent way.
 * Usage:	cc -Itroff mktab.c tabnnn.c -o mktab; mktab > tabnnn
 * Date:	Sat Feb 25 00:10:06 MST 1989
 * Author:	Donn Seeley
 * Remarks:
 *	Traditional nroff terminal table construction works by compiling
 *	a C file into a binary that is read directly into nroff as it runs.
 *	If your C compiler or your relocation format differ from what nroff
 *	expects, you lose.  This program, when linked with a terminal table
 *	object file, fakes up an 'object' file that looks enough like the
 *	traditional one to fool nroff.
 */

#include <stdio.h>
#include "tw.h"

#define TSIZE 	sizeof(t)

main()
{
	static struct fake_exec {
		int bogus[8];	/* bogus[2] == a_data */
	} fe;
	register long *bip;
	register char **tip;
	register long offset = sizeof t;
	int buf[TSIZE];
	char *malloc();
	long twbase = (char *) &t.twinit - (char *)&t.bset;


	/*
	 * Copy the integers at the start of the table.
	 */
	bcopy((char *) &t, (char *) buf, twbase );

	/*
	 * Replace the character pointers in the copy with integer offsets.
	 * This duplicates the effect of relocation offsets.
	 * Take care to count the possibly null control bytes in the codetab
	 *	section.
	 */
	for (bip = (long *)&buf[twbase/(sizeof(int))], tip = &t.twinit;/*001*/
	     tip < &t.codetab[0];
	     ++bip, ++tip)
		if (*tip) {
			*bip = offset;
			offset += (long)(strlen(*tip) + 1L);
		} else
			*bip = 0;
	for (; tip < &t.zzz; ++bip, ++tip)
		if (*tip) {
			*bip = offset;
			offset += (long)(strlen(*tip + 1) + 2L);
		} else
			*bip = 0;
	/*
	 * Patch in a fake data segment size field.
	 */
	fe.bogus[2] = offset; 
	/*
	 * Dump the header and the table.
	 */
	(void) fwrite((char *) &fe, sizeof fe, 1, stdout);
	(void) fwrite((char *) buf, sizeof t, 1, stdout);

	/*
	 * Dump the strings.
	 */
	for (tip = &t.twinit; tip < &t.codetab[0]; ++tip)
		if (*tip) {
			fputs(*tip, stdout);
			putchar('\0');
		}
	for (tip = &t.codetab[0]; tip < &t.zzz; ++tip)
		if (*tip) {
			putchar(**tip);
			fputs(*tip + 1, stdout);
			putchar('\0');
		}

	return 0;
}
