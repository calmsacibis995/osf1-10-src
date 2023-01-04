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
/* @(#)$RCSfile: filetype.h,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1993/01/08 16:34:05 $ */
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
 * Modification History:
 *
 * 26-Sep-1990 - Adrian Thoms (thoms@wessex)
 *	Added addition stuff to export new fdtype() library interface
 */


#include <sys/stat.h>

#define PRINT		1
#define NOPRINT		0

#define MASH(x, y)	(((x)<<16) + (y))
#define MAJOR(l)	((unsigned)(l)>>16)
#define MINOR(l)	((l)&0xffff)

/*		      Major   Minor		*/
#define UNKNOWN		65535
#define SLINK		1
#define	DIRECTORY	2
#define	APPENDONLY		2
#define	STANDARD		1
#define	NAMEDPIPE	3
#define SOCKET		4
#define	SPECIAL		5
#define	BLOCK			1
#define	CHARACTER		2
#define	EMPTY		6
#define	ASCII		7
#define ASCIIwGARBAGE	8
#define	SCCS			1
#define	SHELL			2
#define	BSHELL			3
#define CSHELL			4
#define	CPROG			5
#define FORTPROG		6
#define	ASSEMBLER		7
#define NROFF			8
#define TEXT			9
#define CPIOARCHIVE		106 /* also used under DATA */
#define TROFFINT		10
#define POSTSCRIPT		11
#define COMMANDS		12
#define ENGLISH			100
#define	PRESS		9
#define DATA		11
#define DDIF			1
#define CAT_TROFF		2
#define X_IMAGE			3
#define COMPACTED		4
#define COMPRESSED		5
#define UUENCODED		6
#define PACKED			7
#define LN03			8
#define EXECUTABLE	12
#define PDP11SA			1
#define E411			2
#define E410			3
#define E413			4
#define PDP430			5
#define	PDP431			6
#define	PDP450			7
#define PDP451			8
#define MIPSSEL			9
#define SWAPPED_MIPSSEL		10
#define MIPSSEB_UCODE		11
#define MIPSSEL_UCODE		12
#define ALPHA_AD_UCODE		13
#define ALPHA_AD		14
#define OSF_ROSE		15
#define ARCHIVE		14
#define VERYOLD			3
#define OLDARCH			2
#define STANDARD		1
#define RANLIB			4
#define MIPS_ARCHIVE		6
#define ALPHA_ARHIVE		7
#define LOCALE		15
#define	MIPS_LOCALE		1
#define ALPHA_LOCALE		2
#define OSF_CORE	16

extern int	binary_mktab();
extern char *	execmodes(struct stat *mbuf, char *fbuf);
extern int	fdtype(int fd, struct stat *pmbuf, int *pcount,
			char *buf, unsigned bufsiz, int pflg);
extern int	filetype(char *file, int pflg);
extern void	mkmtab(register int cflg);
extern void	printmtab();
