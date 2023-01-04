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
static char	*sccsid = "@(#)$RCSfile: path.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:45:02 $";
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
/*  path  --  break filename into directory and file
 *
 *  path (filename,direc,file);
 *  char *filename,*direc,*file;
 *  filename is input; direc and file are output (user-supplied).
 *  file will not have any trailing /; direc might.
 *
 *  Note these rules:
 *  1.  trailing / are ignored (except as first character)
 *  2.  x/y is x;y where y contains no / (x may contain /)
 *  3.  /y  is /;y where y contains no /
 *  4.  y   is .;y where y contains no /
 *  5.      is .;. (null filename)
 *  6.  /   is /;. (the root directory)
 *
 * Algorithm is this:
 *  1.  delete trailing / except in first position
 *  2.  if any /, find last one; change to null; y++
 *      else y = x;		(x is direc; y is file)
 *  3.  if y is null, y = .
 *  4.  if x equals y, x = .
 *      else if x is null, x = /
 */

path (original,direc,file)
char *original,*direc,*file;
{
	register char *y;
	/* x is direc */
	register char *p;

	/* copy and note the end */
	p = original;
	y = direc;
	while (*y++ = *p++) ;		/* copy string */
	/* y now points to first char after null */
	--y;	/* y now points to null */
	--y;	/* y now points to last char of string before null */

	/* chop off trailing / except as first character */
	while (y>direc && *y == '/') --y;	/* backpedal past / */
	/* y now points to char before first trailing / or null */
	*(++y) = 0;				/* chop off end of string */
	/* y now points to null */

	/* find last /, if any.  If found, change to null and bump y */
	while (y>direc && *y != '/') --y;
	/* y now points to / or direc.  Note *direc may be / */
	if (*y == '/') {
		*y++ = 0;
	}

	/* find file name part */
	if (*y)  strcpy (file,y);
	else     strcpy (file,".");

	/* find directory part */
	if (direc == y)        strcpy (direc,".");
	else if (*direc == 0)  strcpy (direc,"/");
	/* else direc already has proper value */
}
