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
static char	*sccsid = "@(#)$RCSfile: copy.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:16:41 $";
#endif 
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

struct thread	*current_thread();
#define CURRENT_THREAD

/*
**
**  Copyright (c) 1988 Prime Computer, Inc.  Natick, MA 01760
**  All Rights Reserved
**
**  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Prime Computer, Inc.
**  Inclusion of the above copyright notice does not evidence any actual
**  or intended publication of such source code.
*/


#include <sys/param.h>
#include <sys/errno.h>
#include <kern/thread.h>
#include <mach/kern_return.h>
#include <mach/vm_param.h>

#ifdef	COPY_USER_IS_BROKEN
copyin(from, to, count)
char *from, *to;
vm_size_t count;
{
	register int odd;
	vm_offset_t FAULT_ERROR();

	if ((from >= (char *)VM_MIN_KERNEL_ADDRESS) ||
	    ((from + count) > (char *) VM_MIN_KERNEL_ADDRESS)) 
		return (EFAULT);

	current_thread()->recover = (vm_offset_t)FAULT_ERROR;
	if(count <= NBPW) {
		while(count--)
			*to++ = *from++;
		current_thread()->recover = 0;
		return(0);
	}
	odd = (int) ((int)from & (NBPW - 1));
	count -= odd;
	while(odd--)
		*to++ = *from++;
	bcopy(from,to,count);	
	current_thread()->recover = 0;
	return(0);
}

copyout(from,to,count)
char *from,*to;
vm_size_t count;
{
	register odd;
	vm_offset_t FAULT_ERROR();

	if ((to >= (char *)VM_MIN_KERNEL_ADDRESS) ||
	    ((to + count) > (char *) VM_MIN_KERNEL_ADDRESS)) 
		return(EFAULT);
	current_thread()->recover = (vm_offset_t)FAULT_ERROR;
	if(count <= NBPW) {
		while(count--)
			*to++ = *from++;
		current_thread()->recover = 0;
		return(0);
	}
	odd = (int)((int)from & (NBPW - 1));
	count -= odd;
	while(odd--)
		*to++ = *from++;
	bcopy(from,to,count);
	current_thread()->recover = 0;
	return(0);
}

copystr(s1, s2, max, len)
register char *s1, *s2;
register int max;
int *len;
{
	int n;

	n = 0;
	while (*s2++ = *s1++)
		if (max-- <= 0)
			return ENOENT;
		else
			n++;
	if (len)
		*len = n + 1;
	return(0);
}

/*
 *	copyinstr(user, kernel, max, &actual)
 *	needs checking on valid addresses
 */
copyinstr(up, kp, max, len)
char *up, *kp;
int max, *len;
{
	int n;
	vm_offset_t FAULT_ERROR();

	if (up >= (char *)VM_MIN_KERNEL_ADDRESS)
		return (EFAULT);		
	if ((up + max) > (char *) VM_MIN_KERNEL_ADDRESS)
		max = (int) ((char *) VM_MIN_KERNEL_ADDRESS - (char *) up);

	current_thread()->recover = (vm_offset_t)FAULT_ERROR;
	n = 0;
	while (*kp++ = *up++)
		if (max-- <= 0) {
			current_thread()->recover = 0;
			return ENOENT;
		} else
			n++;
	if (len)
		*len = n + 1;
	current_thread()->recover = 0;
	return(0);
}

/*
 *	copyoutstr(kernel, user, max, &actual)
 *	needs checking on valid addresses
 */
copyoutstr(kp, up, max, len)
char *kp, *up;
int max, *len;
{
	int n;
	vm_offset_t FAULT_ERROR();

	if (up >= (char *)VM_MIN_KERNEL_ADDRESS)
		return EFAULT;
	if ((up + max) > (char *) VM_MIN_KERNEL_ADDRESS)
		max = (int) ((char *) VM_MIN_KERNEL_ADDRESS - (char *) up);

	current_thread()->recover = (vm_offset_t)FAULT_ERROR;
	n = 0;
	while (*up++ = *kp++)
		if (max-- <= 0) {
			current_thread()->recover = 0;
			return ENOENT;
		} else
			n++;
	if (len)
		*len = n + 1;
	current_thread()->recover = 0;
	return(0);
}
#endif	COPY_USER_IS_BROKEN
