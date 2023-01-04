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
 * $XConsortium: mipsMap.c,v 1.4 91/07/18 22:58:44 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsMap.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sysv/sys/types.h>
#include <sysv/sys/sysmacros.h>
#include <sysv/sys/ipc.h>
#include <sysv/sys/shm.h>

extern int shmget();
extern char *shmat();
extern int shmdt();

char *
mipsMapit(addr, key, size)
	char *addr;
	int key;
	int size;
{
	char *new;
	int shmid;
	static int first;
	int dummy;
	extern char *sbrk();

	/*
	 * If we use an address of zero, the 4.5x kernel will only give
	 * us 2M of headroom over the highest existing segment, which
	 * doesn't allow enough room for heap growth.  So, the first time
	 * we map something, pick an address mid-way between the heap and
	 * the stack.
	 */
	if (!first && !addr) {
		first = 1;
		addr = (char *) (((int) &dummy - (int) sbrk(0) / 2) &
			~(SHMLBA - 1));
	}

	if ((shmid = shmget(key, size, 0666)) < 0 ||
		(new = shmat(shmid, addr, 0)) == (char *) -1)
		return 0;

	if (addr && new != addr) {
		(void) shmdt(new);
		return 0;
	}

	return new;
}
