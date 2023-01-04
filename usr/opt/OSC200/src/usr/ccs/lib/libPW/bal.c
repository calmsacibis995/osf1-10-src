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
static char	*sccsid = "@(#)$RCSfile: bal.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:42:03 $";
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
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: balbrk
 *
 * ORIGINS: 3
 *
 * OBJECT CODE ONLY SOURCE MATERIALS
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 * 
 */

/*
	Function to find the position, in str, of the first of the char-
	acters in end occurring outside a balanced string.  A balanced string
	contains matched occurrences of any character in open and the corres-
	ponding character in clos.  Balanced strings may be nested.  The null
	at the end of str is considered to belong to end.  Unmatched members
	of open or clos result in an error return.
*/

#define ifany(x) for (p=x; *p; p++) if (c == *p)
#define matching_clos clos[p-open]
#define error -1
#define position s-str-1

balbrk(str,open,clos,end)
char *str,*open,*clos,*end;
{
	register char *p, *s, c;
	char opp[2];
	opp[1] = '\0';
	for (s = str; c = *s++;  ) {
		ifany(end) return position;
		ifany(clos) return error;
		ifany(open) {
			opp[0] = matching_clos;
			s += balbrk(s,open,clos,opp);
			if (*s++ != matching_clos) return error;
			break;
		}
	}
	return position;
}
