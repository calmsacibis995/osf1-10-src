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
static char	*sccsid = "@(#)$RCSfile: parse.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:49 $";
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

#ifndef lint
#endif

/*
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986,1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#define	QUOTE1	'"'
#define QUOTE2	'\''
#define QUOTE3	'\\'
#define isspace(x) ((x) == ' ' || (x) == '\t')		/* ignore space and tab */

/*
 * argument parser: converts a ascii string into an argc/argv structure
 * given:
 *	parm	the ascii input string
 *	space	the place to store output strings (can be parm)
 *	argv	the place to put the string pointers
 *
 * the function value is the number of arguments found.
 * the following are processed correctly:
 * string string string
 * "string"
 * 'string'
 * string\ string
 * "string\"string"
 * The following is not processed properly:
 * string"string"
 */

int _parse(parm, space, argv)
	register char *parm, *space, **argv;
{
	register int argc = 0;	       /* number of args found */
	register int delim;

	while (isspace(*parm))
		++parm;
	for (; *parm; ++argc) {
		argv[argc] = space;
		/* collect this argument */
		while (*parm && !isspace(*parm)) {
			if (*parm == QUOTE1 || *parm == QUOTE2)
				delim = *parm++;
			else
				delim = ' ';
			while (*parm) {
				if (*parm == QUOTE3 && parm[1] == delim) {
					++parm;
					*space++ = *parm++;
				} else if (*parm == delim) {
					if (!isspace(delim))
						++parm;
					break;
				} else
					*space++ = *parm++;
			}
		}
		while (isspace(*parm))
			++parm;	       /* ignore trailing whitespace */
		*space++ = 0;	       /* end of argument */
	}
	argv[argc] = 0;		       /* make sure properly terminated */
	return (argc);		       /* argc is actual count */
}
