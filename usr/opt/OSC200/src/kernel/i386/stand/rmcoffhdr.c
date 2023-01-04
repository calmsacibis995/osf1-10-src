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
static char	*sccsid = "@(#)$RCSfile: rmcoffhdr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:15:23 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * 			INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license  agreement or 
 *	nondisclosure agreement with Intel Corporation and may not be copied 
 *	nor disclosed except in accordance with the terms of that agreement.
 *
 *	Copyright 1988 Intel Corporation
 * Copyright 1988, 1989 by Intel Corporation
 */

#include "coff.h"

struct headers {
	struct filehdr mainhdr ;
	struct aouthdr secondhdr ;
	} head ;

char buf[1024] ;

main(argc, argv)
char **argv ;
{
	int ifd, ofd, count;
	long offset;

	if ((ifd = open(argv[1], 0)) == -1) {
		printf ("rmcoffhdr: can't open %s\n", argv[1]);
		exit(1) ;
	}

	if ((ofd = open(argv[2], 01002, 0777)) == -1) {
		printf ("rmcoffhdr: can't create %s\n", argv[2]);
		exit(2) ;
	}

	read(ifd, &head, sizeof(struct headers));

	offset = head.mainhdr.f_nscns * sizeof(struct scnhdr);

	lseek (ifd, offset, 1) ;

	while ((count = read(ifd, buf, 1024)) != 0)
		write(ofd, buf, count);

	close(ifd);

	close(ofd);
	 
}
