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
static char *rcsid = "@(#)$RCSfile: lwc.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1993/01/12 16:22:17 $";
#endif
#include <sys/types.h>
#include <sys/unix_defs.h>
#include <mach/machine/vm_types.h>
#include <sys/lwc.h>
#include "cpus.h"
#include <kern/ast.h>

/*
 * Currently we only support as many lwc as there are bits in a long.
 * The priority starts at 0 (highest) and sizeof (long) * NBBY - 1 lowest.
 * Also there maximumly (sizeof long) * NBBY cpus.
 * Some of these restrictions can we removed with further coding effort.
 * LWC can be scheduled by spllwc device interrupts and lower.
 */

/*
 * Outstanding priorities per cpu that desire queue to be scanned.
 */

long lwc_cpu_queue[NCPUS];
udecl_simple_lock_data(,lwc_cpu_lock[NCPUS]);

#define	NLWC	((sizeof (long)) * NBBY)
struct lwc lwc_qhead [NLWC];

#define	lwc_qempty(LWCH) ((LWCH)->lwc_fl == (LWCH))

#define	lwc_remove(LWCP) (						\
	(LWCP)->lwc_fl->lwc_bl = (LWCP)->lwc_bl,			\
	(LWCP)->lwc_bl->lwc_fl = (LWCP)->lwc_fl				\
) 

#define lwc_insert(LWCP,LWCH) (						\
	(LWCP)->lwc_bl = (LWCH)->lwc_bl,				\
	(LWCP)->lwc_fl = (LWCH),					\
	(LWCH)->lwc_bl->lwc_fl = (LWCP),				\
	(LWCH)->lwc_bl = (LWCP)						\
)


#define	lwcp_active(LWCP) 						\
	((LWCP)->lwc_cpu[0] & (1 << cpu_number()))

#define lwcp_interrupt(LWCP) 						\
	((LWCP)->lwc_cpu[0] |= (1 << cpu_number()))

#define lwcp_rfc(LWCP) 							\
	((LWCP)->lwc_cpu[0] ^= (1 << cpu_number()))

/*
 * Until we add to the PCB structure lwc_interrupt_dispatch is the
 * area on a UNI where the interrupts are recognized.
 */



#define	lwc_cpu_interrupt(LWCP) (					\
	((lwc_cpu_queue[cpu_number()] & (1 << (LWCP)->lwc_pri)) ? 0 :	\
	(lwc_cpu_queue[cpu_number()] |= (1 << (LWCP)->lwc_pri))),	\
	(kernel_async_trap[cpu_number()].flags.lwc_pending ? 0 : (kernel_async_trap[cpu_number()].flags.lwc_pending = 1))	\
)

#define	lwc_cpu_rfe() kernel_async_trap[cpu_number()].flags.lwc_pending = 0

static int lwc_initialized;
udecl_simple_lock_data(,lwc_lock)

lwc_init()
{
	register lwc_t lwcp;
	register int i;

	usimple_lock_init(&lwc_lock);
	for (lwcp = &lwc_qhead[0]; lwcp < &lwc_qhead[NLWC]; lwcp++)
		lwcp->lwc_fl = lwcp->lwc_bl = lwcp;
	for (i = 0; i < NCPUS; i++) usimple_lock_init(&lwc_cpu_lock[i]);
	lwc_initialized++;
}

lwc_id_t 
lwc_create(lwc_pri_t pri, void (*ctxt)())
{
	register int s;
	register lwc_t lwcp, lwch;

	if (!lwc_initialized) return LWC_ID_NULL;
	else if (pri >= NLWC || pri < 0) panic("lwc_create: invalid priority");
	
	lwcp = (lwc_t) kalloc(sizeof (struct lwc));
	bzero((caddr_t) lwcp, sizeof (*lwcp));

	lwch = &lwc_qhead[pri];
	lwcp->lwc_ctxt = ctxt;
	lwcp->lwc_pri = pri;

	s = spllwc();
	usimple_lock(&lwc_lock);
	if (!lwc_qempty(lwch)) {
		usimple_unlock(&lwc_lock);
		(void) splx(s);
		(void) panic("lwc_create: lwc at pri already");
	}
	lwc_insert(lwcp,lwch);
	usimple_unlock(&lwc_lock);
	(void) splx(s);
	return (lwc_id_t) lwcp;
}

int 
lwc_destroy(lwc_id_t id)
{
	panic("lwc_destroy: currently not supported");
}

int 
lwc_schedule()
{
	register int s, p;
	register lwc_t lwcp, lwch;
	register long *lcqp, m;
	
	lcqp = &lwc_cpu_queue[cpu_number()];
	s = spllwc();
	usimple_lock(&lwc_cpu_lock[cpu_number()]);
	usimple_lock(&lwc_lock);
loop:
	if ((m = *lcqp)) {
		for (p = 0; (p < NLWC) && m; m >>= 1, p++) {
			if ((m & 1) == 0) continue;
			lwch = &lwc_qhead[p];
			for (lwcp = lwch->lwc_fl; lwch != lwcp; 
				lwcp = lwcp->lwc_fl)
				if (lwcp_active(lwcp)) {
					lwcp->lwc_ref++;
					usimple_unlock(&lwc_lock);
					(void) splx(s);
					(void) (*lwcp->lwc_ctxt)();
					s = spllwc();
					usimple_lock(&lwc_lock);
					lwcp->lwc_ref--;
					goto loop;
				}
			*lcqp ^= (1 << p);
		}
	}

	lwc_cpu_rfe();
	usimple_unlock(&lwc_lock);
	usimple_unlock(&lwc_cpu_lock[cpu_number()]);
	(void) splx(s);
	return;
}

int 
lwc_interrupt(lwc_id_t id)
{
	register lwc_t lwcp = (lwc_t) id;
	register int s;

	s = spllwc();
	usimple_lock(&lwc_lock);
	if (!lwcp_active(lwcp)) {
		lwcp_interrupt(lwcp);
		lwc_cpu_interrupt(lwcp);
	}
	usimple_unlock(&lwc_lock);
	(void) splx(s);
}

int 
lwc_rfc(lwc_id_t id)
{
	register lwc_t lwcp = (lwc_t) id;
	register int s;
	
	s = spllwc();
	usimple_lock(&lwc_lock);
	lwcp_rfc(lwcp);
	usimple_unlock(&lwc_lock);
	(void) splx(s);
}
