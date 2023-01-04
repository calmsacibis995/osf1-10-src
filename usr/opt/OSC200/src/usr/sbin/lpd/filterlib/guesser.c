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
static char *sccsid = "@(#)$RCSfile: guesser.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1993/01/08 16:35:32 $";
#endif

/*
 * OSF/1 Release 1.0
 */
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
 * File:	guesser_new.c
 * Author:	Adrian Thoms (thoms@wessex)
 * Description:
 *	This file exports the interface to the new file guesser
 *	It maps across the major and minor file type numbers produced
 *	by the fdtype() function into the much smaller and simpler set
 *	of types understood by the switch code in the filters.
 *
 * Modification History:
 */
#include "guesser.h"
#include "filetype.h"

int determinefile();

int in;
int globi;
char	filestorage[HOW_MUCH_TO_CHECK+1];		/* first chunk of pipe	*/

int
determinefile(fd)
	int fd;
{
	long mashed_type;
	register int major, minor;
	register int file_type;

	binary_mkmtab();
#ifdef TESTING
	mashed_type = fdtype(fd, 0, &in, filestorage, HOW_MUCH_TO_CHECK, PRINT);
#else
	mashed_type = fdtype(fd, 0, &in, filestorage, HOW_MUCH_TO_CHECK, NOPRINT);
#endif
	major = MAJOR(mashed_type);
	minor = MINOR(mashed_type);
#ifdef TESTING
	printf("File type (%d, %d)\n", major, minor);
#endif
	switch(major) {
	case UNKNOWN:
	default:
		file_type = TEXT_FILE; /* When in doubt, print it */
		break;

	case SLINK:
	case DIRECTORY:
		/* case APPENDONLY: */
		/* case STANDARD: */
	case NAMEDPIPE:
	case SOCKET:
	case SPECIAL:
		/* case BLOCK: */
		/* case CHARACTER: */
	case EMPTY:
		file_type = EMPTY_FILE;
		break;

	case ASCII:
	case ASCIIwGARBAGE:
		switch(minor) {
		case SCCS:
			file_type = DATA_FILE;
			break;

		case SHELL:
		case BSHELL:
		case CSHELL:
		case CPROG:
		case FORTPROG:
		case ASSEMBLER:
		case NROFF:
		case TEXT:
			file_type = TEXT_FILE;
			break;

		case CPIOARCHIVE:/* also used under DATA */
		case TROFFINT:
			file_type = DATA_FILE;
			break;

		case POSTSCRIPT:
			file_type = POSTSCRIPT_FILE;
			break;

		case COMMANDS:
		case ENGLISH:
			file_type = TEXT_FILE;
			break;
		}
		break;

	case PRESS:
		file_type = DATA_FILE;
		break;

	case DATA:
		switch(minor) {
		case DDIF:
			file_type = DATA_FILE;
			break;

		case CAT_TROFF:
			file_type = CAT_FILE;
			break;

		case X_IMAGE:
			file_type = XIMAGE_FILE;
			break;

		case COMPACTED:
		case COMPRESSED:
		case UUENCODED:
		case PACKED:
		case UNKNOWN:
			file_type = DATA_FILE;
			break;

		case LN03:
			file_type = ANSI_FILE;
			break;
		}
		break;

	case EXECUTABLE:
		/* case PDP11SA: */
		/* case E411: */
		/* case E410: */
		/* case E413: */
		/* case PDP430: */
		/* case PDP431: */
		/* case PDP450: */
		/* case PDP451: */
	case ARCHIVE:
		/* case VERYOLD: */
		/* case OLDARCH: */
		/* case STANDARD: */
		/* case RANLIB: */
		file_type = DATA_FILE;
		break;
	}
	return file_type;
}

#ifdef TESTING
#include <stdio.h>

static char *file_types[] = {
	"EMPTY_FILE",		/* 0 */
	"EXECUTABLE_FILE",	/* 1 */
	"ARCHIVE_FILE",		/* 2 */
	"DATA_FILE",		/* 3 */
	"TEXT_FILE",		/* 4 */
	"CTEXT_FILE",		/* 5 */
	"ATEXT_FILE",		/* 6 */
	"RTEXT_FILE",		/* 7 */
	"FTEXT_FILE",		/* 8 */
	"CAT_FILE",		/* 9 */
	"XIMAGE_FILE",		/* 10 */
	"POSTSCRIPT_FILE",	/* 11 */
	"ANSI_FILE"		/* 12 */
};

#define NFILE_TYPES	(sizeof(file_types)/sizeof(file_types[0]))

FILE *input;

main(argc, argv)
int argc;
char *argv[];
{
	char *file;
	if (argc == 1) {
		/* ghastly kludge */
		argv[1] = "-"; argc=2;
	}
	while (--argc) {
		int file_type;

		file = *++argv;
		if (!strcmp(file, "-")) {
			input = stdin;
			file="<stdin>";
		} else {
			input=fopen(file, "r");
			if (input == NULL) {
				fprintf(stderr, "Can't open %s\n", file);
				continue;
			}
		}

		file_type=determinefile(fileno(input));
		if ((unsigned) file_type > NFILE_TYPES) {
			fprintf(stderr, "File %s is of file_type %d\n", file, file_type);
		} else {
			fprintf(stderr, "File %s is of file_type %s\n", file, file_types[file_type]);
		}

	}

}

#endif
