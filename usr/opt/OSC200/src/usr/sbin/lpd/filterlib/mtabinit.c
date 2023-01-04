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
static char *sccsid = "@(#)$RCSfile: mtabinit.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1993/01/08 16:37:44 $";
#endif

/*
 * OSF/1 Release 1.0
 */
/* Derived from the work
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
 */


/*
 * File:	mtabinit.c
 * Author:	Adrian Thoms
 * Description:
 *	This file prints out a C source file containing the pre-compiled
 *	version of the magic data from the magic file
 */

#include "magic_data.h"

init_printmtab()
{
	reg Entry *ep;

	printf("#include \"magic_data.h\"\n\n");
	printf("Entry_init magic_tab[] = {\n");
	for(ep = mtab; ep->e_off > -2*maxexecfn; ep++) {
/*		printf("{ %d,\t%DL,\t%DL,\t%d,\t%d,\t", */ /*best guess below*/
		printf("{ %d,\t%d,\t%d,\t%d,\t%d,\t",
		       ep->e_level, ep->e_off, 
		       ep->e_retcode, ep->e_type, ep->e_opcode);
		if (ep->e_type == STR) {
			printf("(long)");
			init_printstr(ep->e_value.str, 50);
			printf(",\t");
		}
		else
		    printf("0%#o,\t", ep->e_value.num);
		printf("\"%s\" },\n", ep->e_str);
	}
/*	printf("{ 0,\t%DL }\n", -2*maxexecfn); */ /* ditto above guess */
	printf("{ 0,\t%d }\n", -2*maxexecfn);
	printf("};\n");
}

init_printstr(p, n)
unsigned char *p;
int n;
{

	register unsigned char *sp;
	register int c;

	putchar('"');
	for (sp = p, c = 0; *sp != '\0' && c++ < n; sp++)
		if (isprint(*sp)) printf("%c", *sp);
		else if (*sp == '\n') printf("\\n");
		else if (*sp == '\r') printf("\\r");
		else if (*sp == '\t') printf("\\t");
		else if (*sp == '\b') printf("\\b");
		else if (*sp == '\f') printf("\\f");
		else printf("\\%03o", *sp);
	putchar('"');
}
