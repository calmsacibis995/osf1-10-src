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
 * @(#)$RCSfile: locdef.h,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/11/19 23:27:11 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.4  com/cmd/nls/locdef.h, , bos320, 9135320l 8/12/91 17:09:41
 *
 */

extern char *tpath;	/* Used in localedef.c and sem_method.c */
extern char *ccopts;	/* "" */
extern char *ldopts;	/* "" */

extern char	*yyfilenm;
extern int	warn;

extern int	copying_collate;	/* used by copy.c and gen.c */
extern int	copying_ctype;		/* "" */

/*
  Ignore and UNDEFINED.
*/
#define IGNORE    (-1)
#define UNDEFINED 0

#define INT_METHOD(n)	(n)

/* init.c */
void init_symbol_tbl(int);

/* sem_chr.c */

char *copy( const char * );
char *copy_string( const char * );
