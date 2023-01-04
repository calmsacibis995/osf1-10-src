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
static char	*sccsid = "@(#)$RCSfile: ffilecopy.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:44:10 $";
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
/*  ffilecopy  --  very fast buffered file copy
 *
 *  Usage:  i = ffilecopy (here,there)
 *	int i;
 *	FILE *here, *there;
 *
 *  Ffilecopy is a very fast routine to copy the rest of a buffered
 *  input file to a buffered output file.  Here and there are open
 *  buffers for reading and writing (respectively); ffilecopy
 *  performs a file-copy faster than you should expect to do it
 *  yourself.  Ffilecopy returns 0 if everything was OK; EOF if
 *  there was any error.  Normally, the input file will be left in
 *  EOF state (feof(here) will return TRUE), and the output file will be
 *  flushed (i.e. all data on the file rather in the core buffer).
 *  It is not necessary to flush the output file before ffilecopy.
 */

#include <stdio.h>
int filecopy();

int ffilecopy (here,there)
FILE *here, *there;
{
	register int i, herefile, therefile;

	herefile = fileno(here);
	therefile = fileno(there);

	if (fflush (there) == EOF)		/* flush pending output */
		return (EOF);

	if ((here->_cnt) > 0) {			/* flush buffered input */
		i = write (therefile, here->_ptr, here->_cnt);
		if (i != here->_cnt)  return (EOF);
		here->_ptr = here->_base;
		here->_cnt = 0;
	}

	i = filecopy (herefile, therefile);	/* fast file copy */
	if (i < 0)  return (EOF);

	(here->_flag) |= _IOEOF;		/* indicate EOF */

	return (0);
}
