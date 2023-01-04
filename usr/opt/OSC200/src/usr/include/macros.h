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
 *	@(#)$RCSfile: macros.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/28 23:03:56 $
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

/* macros.h	5.1 - 86/12/09 - 06:04:58 */
#ifndef _MACROS_H_
#define _MACROS_H_

/*
	numeric() is useful in while's, if's, etc., but don't use *p++
	max() and min() depend on the types of the operands
	abs() is absolute value
*/

# define numeric(c)		(c >= '0' && c <= '9')
# define min(a,b)		( ((a) < (b)) ? (a) : (b) )
# define max(a,b)		( ((a) > (b)) ? (a) : (b) )
# define abs(x)			(x>=0 ? x : -(x))

# define copy(srce,dest)	cat(dest,srce,0)
# define compare(str1,str2)	strcmp(str1,str2)
# define equal(str1,str2)	!strcmp(str1,str2)
# define length(str)		strlen(str)
# define size(str)		(strlen(str) + 1)

/*
	The global variable Statbuf is available for use as a stat(II)
	structure.  Note that "stat.h" is included here and should
	not be included elsewhere.
	Exists(file) returns 0 if the file does not exist;
	the flags word if it does (the flags word is always non-zero).
*/
# include <sys/stat.h>
extern struct stat Statbuf;
# define exists(file)		(stat(file,&Statbuf)<0 ? 0:Statbuf.st_mode)

extern long itol();
/*
	libS.a interface for xopen() and xcreat()
*/
# define xfopen(file,mode)	fdfopen(xopen(file,mode),mode)
# define xfcreat(file,mode)	fdfopen(xcreat(file,mode),1)

# define remove(file)		xunlink(file)

/*
	SAVE() and RSTR() use local data in nested blocks.
	Make sure that they nest cleanly.
*/
# define SAVE(name,place)	{ int place = name;
# define RSTR(name,place)	name = place;}

/*
	Use: DEBUG(sum,d) which becomes fprintf(stderr,"sum = %d\n",sum)
*/
# define DEBUG(variable,type)	fprintf(stderr,"variable = %type\n",variable)


/*
	Use of ERRABORT() will cause libS.a internal
	errors to cause aborts
*/
# define ERRABORT()	_error() { abort(); }

/*
	Use of USXALLOC() is required to force all calls to alloc()
	(e.g., from libS.a) to call xalloc().
*/
# define USXALLOC() \
		char *alloc(n) {return((char *)xalloc((unsigned)n));} \
		free(n) char *n; {xfree(n);} \
		char *malloc(n) unsigned n; {int p; p=xalloc(n); \
			return((char *)(p != -1?p:0));}

# define NONBLANK(p)		while (*p==' ' || *p=='\t') p++


/*
	A global null string.
*/
extern char	Null[1];

/*
	A global error message string.
*/
extern char	Error[128];

#endif /* _MACROS_H_ */
