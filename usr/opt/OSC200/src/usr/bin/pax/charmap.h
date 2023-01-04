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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: charmap.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 19:30:38 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * charmap.h - defnitions for charmap parsing
 *
 * DESCRIPTION
 *
 *	This file contains definitions required for parsing the
 *	charmap files.   It is included by charmap.c
 *
 * AUTHOR
 *
 *     Tom Jordahl	- The Open Software Foundation
 *
 */

#ifndef _CHARMAP_H
#define _CHARMAP_H

#define LMAX		1024		/* max number of chars in a line */

#define	CODE_SET_NAME	1
#define MB_MAX		2
#define MB_MIN		3
#define ESCAPE_CHAR	4
#define COMMENT_CHAR	5

/*
 * isportable is defined to return true if the character is
 * a member of the POSIX 1003.2 portable character set.
 *
 * right now we use isascii(), this should be correct
 */
#define isportable(c)	(isascii(c)) 

typedef struct value_struct {
    unsigned char	mbvalue[10];	/* multi-byte encoding */
    short		nbytes;		/* number of bytes in mbvalue */
} Value;

typedef struct charmap_struct {
    char		  *symbol;	/* symbol name */
    unsigned char 	  value[10];	/* symbols multi-byte encoding */
    short		  nbytes;	/* number of bytes in value */
    struct charmap_struct *next;	/* pointer to next struct */
} Charentry;

#endif /* _CHARMAP_H */
