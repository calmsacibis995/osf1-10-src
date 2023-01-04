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
/* @(#)$RCSfile: guesser.h,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/12/14 18:47:37 $ */

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
 * Modification History:
 *
 * 26-Sep-1990 - Adrian Thoms (thoms@wessex)
 *	Modified from code in old ln03of filter to export the interface
 *	of the guesser library module
 */

#define EMPTY_FILE		0
#define EXECUTABLE_FILE		1
#define ARCHIVE_FILE		2
#define DATA_FILE		3
#define TEXT_FILE		4
#define CTEXT_FILE		5
#define ATEXT_FILE		6
#define RTEXT_FILE		7
#define FTEXT_FILE		8
#define CAT_FILE		9
#define XIMAGE_FILE		10
#define POSTSCRIPT_FILE		11
#define ANSI_FILE               12


#define HOW_MUCH_TO_CHECK	4096		/* input buffer amount	*/
#define MB_MAX			4

extern int determinefile();
extern char filestorage[];
extern int globi, in;
