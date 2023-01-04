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
static char	*sccsid = "@(#)$RCSfile: xsymtab.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:16:11 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: CMDMSG
 *
 * FUNCTIONS: insert, nsearch, hash
 *
 * ORIGINS: 27
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

#define HASHSIZE 256			/* must be a power of 2 */
#define HASHMAX HASHSIZE - 1


/*
 * EXTERNAL PROCEDURES CALLED: None
 */

struct name {
  char *regname;
  int   regnr;
  struct name *left;
  struct name *right;
  };

struct name *symtab[HASHSIZE];		/* hashed pointers to binary trees */

char *calloc();
char *malloc();

/*
 * NAME: insert
 *
 * FUNCTION: Insert symbol
 *
 * NOTES: These routines manipulate a symbol table for the mkcatdefs program.
 *  	  The symbol table is organized as a hashed set of binary trees.  If the
 *  	  symbol being passed in is found then a -1 is returned, otherwise the
 * 	  symbol is placed in the symbol table and a 0 is returned.  The purpose
 *  	  of the symbol table is to keep mkcatdefs from assigning two different
 *   	  message set / message numbers to the same symbol.
 *
 *	  Read the next line from the open message catalog descriptor file
 *
 * (DATA STRUCTURES:) Effects on global data structures -- none.
 *
 * RETURNS: 0 - symbol inserted
 *         -1 - symbol exists
 */

insert(tname,seqno)
				/*
				  tname - pointer to symbol
				  seqno - integer value of symbol
				*/
  char *tname;
  int seqno;
{
	register struct name *ptr,*optr;
	int rslt = -1,i,hashval;

	hashval = hash(tname);
	ptr = symtab[hashval];

	/* search the binary tree for specified symbol */
	while (ptr && (rslt = strcmp(tname,ptr->regname))) {
		optr=ptr;  
		if (rslt<0)
			ptr = ptr->left;
		else
			ptr = ptr->right;
	}

	if (rslt == 0)		/* found the symbol already defined */
		return (-1);

	/* symbol not defined yet so put it into symbol table */
	else {
		ptr = (struct name *)calloc(sizeof(struct name), 1);
		ptr->regname = malloc(strlen(tname) + 1);
		strcpy (ptr->regname, tname);
		ptr->regnr = seqno;

		/* not first entry in tree so update branch pointer */
		if (symtab[hashval]) {
			if (rslt < 0)
				optr->left = ptr;
			else
				optr->right = ptr;

		/* first entry in tree so set the root pointer */
		} else
			symtab[hashval] = ptr;

		return (0);
	}
}

/*
 * NAME: insert
 *
 * FUNCTION: Insert symbol
 *
 * NOTES: Searches for specific identifies. If found, returns allocated number.
 *        If not found, return -1.
 *
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 *
 * RETURNS: symbol sequence number - symbol found
 *          -1 - symbol not found
 */

nsearch(tname)
				/*
				  tname - pointer to symbol
				*/
  char *tname;
{
	register struct name *ptr,*optr;
	int rslt = -1,i,hashval;

	hashval = hash(tname);
	ptr = symtab[hashval];

	/* search the binary tree for specified symbol */
	while (ptr && (rslt = strcmp(tname,ptr->regname))) {
		optr=ptr;  
		if (rslt<0)
			ptr = ptr->left;
		else
			ptr = ptr->right;
	}

	if (rslt == 0)		/* found the symbol already defined */
		return(ptr->regnr);
	else
		return (-1);
}


/*
 * NAME: hash
 *
 * FUNCTION: Creates hash value from symbol name.
 *
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 *
 * RETURNS: hash value
 */

hash(name)
				/*
				  name - pointer to symbol
				*/
  register char *name;
{
	register int hashval = 0;

	while (*name)
		hashval += *name++;

	return (hashval & HASHMAX);
}
