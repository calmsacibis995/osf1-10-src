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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: append.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 19:22:46 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * append.c - append to a tape archive. 
 *
 * DESCRIPTION
 *
 *	Routines to allow appending of archives
 *
 * AUTHORS
 *
 *     	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Revision 1.2  89/02/12  10:03:58  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:00  mark
 * Initial revision
 * 
 */


/* Headers */

#include "pax.h"
#include <sys/mtio.h>

/* append_archive - main loop for appending to a tar archive
 *
 * DESCRIPTION
 *
 *	Append_archive reads an archive until the end of the archive is
 *	reached once the archive is reached, the buffers are reset and the
 *	create_archive function is called to handle the actual writing of
 *	the appended archive data.  This is quite similar to the
 *	read_archive function, however, it does not do all the processing.
 */


void append_archive(void)

{
    Stat            sb;
    char            name[PATH_MAX + 1];

    name[0] = '\0';
    while (get_header(name, &sb) == 0) {
	if (!f_unconditional)
	    hash_name(name, &sb);

	if (((ar_format == TAR)
	     ? buf_skip(ROUNDUP((OFFSET) sb.sb_size, BLOCKSIZE))
	     : buf_skip((OFFSET) sb.sb_size)) < 0) {
	    warn(name, MSGSTR(APPEND_CORRUPT, "File data is corrupt"));
	}
    }
    /* we have now gotten to the end of the archive... */

    backup();	/* adjusts the file descriptor and buffer pointers */

    create_archive();
}


/* backup - back the tape up to the end of data in the archive.
 *
 * DESCRIPTION
 *
 *	The last header we have read is either the cpio TRAILER!!! entry
 *	or the two blocks (512 bytes each) of zero's for tar archives.
 * 	adjust the file pointer and the buffer pointers to point to
 * 	the beginning of the trailer headers.
 */


void backup(void)

{
    static int mtdev = 1;
    static struct mtop mtop = {MTBSR, 1};	/* Backspace record */
    struct mtget mtget;
	
	if (mtdev == 1)
	    mtdev = ioctl(archivefd, MTIOCGET, (char *)&mtget);
	if (mtdev == 0) {
	    if (ioctl(archivefd, MTIOCTOP, (char *)&mtop) < 0) {
		fatal(MSGSTR(APPEND_INVALID, "The append option is not valid forspecified device."));
	    }
	} else {
	    if (lseek(archivefd, -(off_t)(bufend-bufstart), SEEK_CUR) == -1) {
		warn("lseek", strerror(errno));
		fatal(MSGSTR(APPEND_BACK2, "backspace error"));
	    }
	}

	bufidx = lastheader;	/* point to beginning of trailer */
	/*
	 * if lastheader points to the very end of the buffer
	 * Then the trailer really started at the beginning of this buffer
	 */
	if (bufidx == bufstart+blocksize)
		bufidx = bufstart;
}
