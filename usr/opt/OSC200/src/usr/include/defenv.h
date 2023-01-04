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
/*	
 *	@(#)$RCSfile: defenv.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:14:35 $
 */ 
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

#ifndef _DEFENV_H_
#define _DEFENV_H_

/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Default environment definitions */

#define DEFCTAB	"/etc/nls/ctab/default"
#define NDATVAR	"NLDATE"
#define NFILVAR	"NLFILE"
#define NLDTVAR "NLLDATE"
#define NTIMVAR "NLTIME"
#define NLCTVAR "NLCTAB"

/*  Table of default parameter values
 */
typedef struct nppair {
	char *np_name;
	char *np_val;
	} NPPAIR;

extern NPPAIR deftable[]; /* defined in NLgetenv() */
extern int deftsize;      /* defined in NLgetenv() */

struct envpair {
	char *name;			/* Name of param */
	char *def;			/* Value of param */
	char parsed;			/* Bool:  value parsed? */
	struct envpair *next;		/* Next param struct */
};
extern struct envpair *__paramtab; 

extern char *__filbuf; /* NLgetfile(), NLgetenv() and setlocale()  */
extern char *__bufp;   /* are using __filbuf, __bufp and __ovflg   */
extern char __ovflg;   /* They are all defined in NLgetenv()       */

#endif /* _DEFENV_H_ */

