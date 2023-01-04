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
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * io.c
 */

#if !defined(lint) && defined(SECONDARY)
static char	*sccsid = "@(#)$RCSfile: io.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:10 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#include "../../../sys/param.h"
#include "gnode_common.h"
#include "ufs_inode.h"
#include "gnode.h"
#include "saio.h"
#ifdef vax
#include "vax/vmb.h"
#include "../vax/rpb.h"
#endif vax


int	errno;
extern	mode;
int	prom_io = -1;
int	devpart = 0;
#define printf _prom_printf

devread(io)
        register struct iob *io;
{
#ifdef vax
	int qio_status;
	int bc, i;

	if (mode & ROM_BOOT)
		bc=512;			/* ROM driver can only read 512 */
	else
		bc=io->i_cc;

	for (i = 0; i*bc < io->i_cc; i++) {
	    qio_status = 
		qio(PHYSMODE, IO$_READLBLK, io->i_bn+i, bc, (int)io->i_ma+(i*512));
	    if ((qio_status & 1) == 0)
			break;
	}
	if (qio_status & 1) {
		errno = 0;
        	return (io->i_cc);
	} else {
		printf("Read error: bn = %d, %s\n",
			io->i_bn, geterr(qio_status));
		io->i_error = EIO;
		return (-1);
	}
#endif vax
#ifdef mips
	_prom_lseek(prom_io, io->i_bn*512, 0);
	return (_prom_read(prom_io, io->i_ma, io->i_cc));
#endif mips
}

#ifdef mips
#define RPAREN ')'
#define LPAREN '('

int
devopen(str, flag)
register char *str;
register int flag;
{
#if LABELS
register char *cp;
char devname[16];
	/* Find the partition designator in the device string.
	 * this parser assumes it is handed valid input, and
	 * does very little error checking (after all, the ROMs
	 * have already placed their restrictions on this string
	 * anyway, and we have to live in the primary bootstrap).
	 */
	cp = str;
	str = devname;
	/* skip through to (past) first close paren */
	while (*cp && (*cp != RPAREN)) *str++ = *cp++;
	*str++ = ','; *str++ = 'c'; *str++ = RPAREN; *str = '\0';
	str = devname;

	/* cp points either at a NULL, or a right paren */

	/* Check for partition designator. */
	if (*cp && (*(cp + 1) == LPAREN)) {
		cp += 2;
		if ((*cp >= '0') && (*cp <= '7')) {
			devpart = *cp - '0';
		} else if ((*cp >= 'a') && (*cp <= 'h')) {
			devpart = *cp - 'a';
		} else {
			printf("Bad partition specification '%c'.\n", *cp);
			return (-1);
		}
	}
#if SECONDARY
	if (prom_io >= 0) {
		_prom_close(prom_io);
	}
#endif
#endif
	prom_io = _prom_open(str, flag);
	return (prom_io);
}
#endif
