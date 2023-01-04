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
 * $XConsortium: mipsOsBSD.c,v 1.2 91/07/18 22:58:52 keith Exp $
 *
 * Copyright 1991 MIPS Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsOsBSD.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

/*
 * OS support code which requires bsd43 include files
 */

#include <bsd43/syscall.h>
#include <bsd43/sysmips.h>
#include <bsd43/mips/debug.h>
#include <bsd43/mips/hwconf.h>

#include "mips.h"

#ifdef SYSV
#define	hwconf(option,conf) \
	(syscall(BSD43_(SYS_sysmips), BSD43_(MIPS_HWCONF), \
	BSD43_(option), (conf)))

#define	kopt(option,value,op) \
	(syscall(BSD43_(SYS_sysmips), BSD43_(MIPS_KOPT), \
	(option), (value), BSD43_(op), 0, 0))
#endif /* SYSV */

cpubd()
{
	struct bsd43_(hw_config) Conf;

	if (hwconf(HWCONF_GET,&Conf) < 0)
		return 0;

	return Conf.cpubd_type;
}

koptKeyswtch()
{
	return kopt("keyswtch",0,KOPT_GET);
}
