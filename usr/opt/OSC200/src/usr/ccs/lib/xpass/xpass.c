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
static char	*sccsid = "@(#)$RCSfile: xpass.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:23:17 $";
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
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: CXBeginBlk, CXDefFtn, CXDefName, CXEndBlk, CXRefName
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef MSG
#include "cxref_msg.h"
#define         MSGSTR(Num, Str) NLcatgets(catd, MS_CXREF, Num, Str)
nl_catd catd;
#else
#define         MSGSTR(Num, Str) Str
#endif

# include "mfile1.h"

int blocknos[BNEST];
int blockptr = 0;
int nextblock = 1;		/* block counter */

/*
** The following functions are embedded in the grammar of the 1st pass
** whenever a new block is created or destroyed.
*/
CXBeginBlk()
{
	/* Code for beginning a new block. */
	blocknos[blockptr] = nextblock++;
	printf("B%d\t%05d\n", blocknos[blockptr], lineno);
	blockptr++;
}

CXEndBlk()
{
	/* Code for ending a block. */
	if (--blockptr < 0)
		uerror(MSGSTR(M_MSG_1, "bad block nesting"));
	else
		printf("E%d\t%05d\n", blocknos[blockptr], lineno);
}

/*
** The following functions are embedded in the grammar of the 1st pass
** whenever a NAME is seen.
*/

CXRefName(i, line)
int i, line;
{
	/* Code for referencing a NAME. */
	printf("R%s\t%05d\n", stab[i].psname, line);
}

CXDefName(i, line)
int i, line;
{
	/* Code for defining a NAME. */
	if (stab[i].sclass == EXTERN)
		CXRefName(i, line);
	else
		printf("D%s\t%05d\n", stab[i].psname, line);
}

CXDefFtn(i, line)
int i, line;
{
	/* Code for defining a function NAME. */
	printf("F%s\t%05d\n", stab[i].psname, line);
}
