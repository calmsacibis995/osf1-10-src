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
static char	*sccsid = "@(#)$RCSfile: dispatcher.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:17:24 $";
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
#include <i386/handler.h>
#include <i386/dispatcher.h>
#include <sys/syslog.h>


/*
 * NAME: 	handler_dispatch()
 *
 * FUNCTION:	Interrupt Dispatcher
 *
 * EXEC ENV: 	When a hardware interrupt is recieved (in locore), this routine
 *		is called, passing interrupt identification information.
 *		This information is used to locate and call the corresponding
 *		interrupt handler(s) to satisfy the interrupt request.
 *
 * NOTE:	This algorithm assumes that the handler which handles the
 *		interrupt request will return non-zero else zero.
 * 		
 * RETURNS:	If a handler appears to have satisfied the interrupt 0 is 
 *		returned, else a non-zero value is returned.
 */

/* DEBUG */
int handler_debug = 0;
int handler_trace = 0;
long intr_stray = 0;

int 
handler_dispatch( index )
short	index;
{
	register ihandler_t *	ih;			/* Intr handler stuct */
	register int		dispatched;
        
	/*
	 * Call each handler for this interrupt, 
	 * until a handler returns that the interrupt was handled
	 */
        if (handler_trace == -1 || handler_trace == index)
                printf("interrupt trace %d\n", index);
        
	dispatched = 0;
	ITABLE_READ_LOCK( index );
	for (ih=itable[index].it_ih; ih ; ih=ih->ih_next) {

		if ( ! IH_STATE_QUERY( ih, IH_STATE_ENABLED ) )
			continue;

		ih->ih_stats.intr_cnt++;        /* handler interrupt count */

		if ((ih->ih_handler)( ih->ih_hparam[0] )) {
			++dispatched;
			/* Ordinarily, we would break, but the interrupt
			 * inputs to the PIC are edge-triggered, so our
			 * EOI in locore might inhibit more requestors
			 * down the pic line. Therefore, continue poll. */
			/* break; */
		}
	}
	ITABLE_READ_UNLOCK( index );

	if (dispatched)
                return (0);

	log(LOG_ERR, "Interrupt dispatcher: stray interrupt on irq line %d\n",
		index);
        intr_stray++;
        
	return( -1 );
}

long
handler_stats( type )
        int     type;
{
        register ihandler_t *   ih;
        register int            index;
        register long           cnt;

        cnt = 0;

        for (index=0; index < ITABLE_SIZE; index++) {

                ITABLE_READ_LOCK(index);
                for (ih=itable[index].it_ih; ih; ih=ih->ih_next) {

                        if (!IH_STATE_QUERY(ih, IH_STATE_ENABLED))
                                continue;

                        if (ih->ih_stats.intr_type & type)
                                cnt += ih->ih_stats.intr_cnt;
                }
                ITABLE_READ_UNLOCK(index);
        }
        return (cnt);


}
