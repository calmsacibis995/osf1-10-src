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
static char	*sccsid = "@(#)$RCSfile: filecopy.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:44:15 $";
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
/*  filecopy  --  copy a file from here to there
 *
 *  Usage:  i = filecopy (here,there);
 *	int i, here, there;
 *
 *  Filecopy performs a fast copy of the file "here" to the
 *  file "there".  Here and there are both file descriptors of
 *  open files; here is open for input, and there for output.
 *  Filecopy returns 0 if all is OK; -1 on error.
 *
 *  I have performed some tests for possible improvements to filecopy.
 *  Using a buffer size of 10240 provides about a 1.5 times speedup
 *  over 512 for a file of about 200,000 bytes.  Of course, other
 *  buffer sized should also work; this is a rather arbitrary choice.
 *  I have also tried inserting special startup code to attempt
 *  to align either the input or the output file to lie on a
 *  physical (512-byte) block boundary prior to the big loop,
 *  but this presents only a small (about 5% speedup, so I've
 *  canned that code.  The simple thing seems to be good enough.
 */

#define BUFFERSIZE 10240

int filecopy (here,there)
int here,there;
{
	register int kount;
	char buffer[BUFFERSIZE];
	kount = 0;
	while (kount == 0 && (kount=read(here,buffer,BUFFERSIZE)) > 0)
		kount -= write (there,buffer,kount);
	return (kount ? -1 : 0);
}
