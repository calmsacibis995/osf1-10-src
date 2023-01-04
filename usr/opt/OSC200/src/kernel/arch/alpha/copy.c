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
static char *rcsid = "@(#)$RCSfile: copy.c,v $ $Revision: 1.2.14.3 $ (DEC) $Date: 1993/07/12 15:45:44 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)copy.c	9.1	(ULTRIX/OSF)	10/21/91";
#endif lint

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/regdef.h>
#include <machine/psl.h>
#include <machine/pcb.h>
#include <machine/machparam.h>
#include <mach/machine/vm_param.h>
#include <sys/types.h>
#include <sys/errno.h>


/*
 * copyin(user_src, kernel_dst, bcount)
 */

copyin(udaddr, kaddr, n)
	register caddr_t udaddr, kaddr;
	register u_int n;
{
	register int ret;

	if((long)udaddr < 0)		/* kernel address */
		return (EFAULT);

	current_pcb->pcb_nofault = NF_COPYIO;
	/*
	 * bcopy will return EFAULT, through cerror(), if user fault failed.
	 */
	ret = bcopy(udaddr, kaddr, n);
	current_pcb->pcb_nofault = 0;
	return (ret);
}

/*
 * copyout(kernel_src, user_dst, bcount)
 */

copyout(kaddr, udaddr, n)
	register caddr_t kaddr, udaddr;
	register u_int n;
{
	register int ret;

	if((long)udaddr < 0)		/* kernel address */
		return (EFAULT);

	/*
	 * bcopy will return EFAULT, through cerror(), if user fault failed.
	 */
	current_pcb->pcb_nofault = NF_COPYIO;
	ret = bcopy(kaddr, udaddr, n);
	current_pcb->pcb_nofault = 0;
	return (ret);
}

blkcpy(src, dst, len)
	register caddr_t src;
	register caddr_t dst;
	register int len;
{
	return (bcopy(src, dst, len));
}

eblkcpy(src, dst, len)
	register caddr_t src;
	register caddr_t dst;
	register int len;
{
	register int ret;

	/*
	 * bcopy will return EFAULT, through cerror(), if user fault failed.
	 */
	current_pcb->pcb_nofault = NF_COPYIO;
	ret = bcopy(src, dst, len);
	current_pcb->pcb_nofault = 0;
	return (ret);
}


struct bcopystats {
	long	scount,
		icount,
		lcount;
} bcstat;


blkclr(base, count)
	register caddr_t base;
	register unsigned count;
{
	return(bzero(base, count));
}

clearseg(pfn)
	register int pfn;
{
	panic("clearseg");
}


/*
 * Copy a null terminated string from one point to another in 
 * kernel address space.
 *
 * copystr(src, dest, maxlength, &lencopied)
 *	returns:
 *		0		- success
 *		EFAULT		- address not accessable (bogus length)
 *		ENAMETOOLONG	- string exceeded maxlength
 */
copystr(kfaddr, kdaddr, maxlength, lencopied)
	register caddr_t kfaddr, kdaddr;
	register u_int maxlength, *lencopied;
{
	register u_int i;

	i = 0;
	while (i < maxlength) {
		*kdaddr = *kfaddr;
		i++;
		if (*kfaddr == '\0') {
			break;
		}
		kdaddr++;
		kfaddr++;
	}
	if (lencopied != 0)
		*lencopied = i;
	if (*kfaddr != '\0')
		return (ENAMETOOLONG);
	else
		return (0);
}


strncmp(st1, st2, n)
register char *st1, *st2;
register n;
{

	while (--n >= 0 && *st1 == *st2++)
		if (*st1++ == '\0')
			return(0);
	return(n<0 ? 0 : *st1 - *--st2);
}

char *
strcpy(dst, src)
register char *dst, *src;
{
	while (*dst++ = *src++)
		continue;
	return (dst - 1);
}

strcmp(st1, st2)
register char *st1, *st2;
{

	while (*st1 == *st2++)
		if (*st1++=='\0')
			return(0);
	return(*st1 - *--st2);
}


/*
 * Copy st2 to st1, truncating to copy n bytes
 * return ptr to null in st1 or st1 + n
 */
char *
strncpy(st1, st2, n)
	register char *st1, *st2;
{
	register i;

	for (i = 0; i < n; i++) {
		if ((*st1++ = *st2++) == '\0') {
			return (st1 - 1);
		}
	}
	return (st1);
}
