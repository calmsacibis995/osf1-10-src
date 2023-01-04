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
static char *rcsid = "@(#)$RCSfile: kdebug_gbus_cons.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/27 14:18:17 $";
#endif

#include <sys/types.h>
#include <io/common/devio.h>
#include <hal/kn7aa.h>

#define READ	1
#define WRITE	2

static char *regs;

long
kdebug_gbus_init(
    unsigned long addr)
{
    regs = (char *) addr;
    return(0);
}

/*
 * Routine to access to 8530 UART data and control registers.
 *
 * cnrw_8350 routine parameters:
 *   read : boolean. If true then perform read, else do write.
 *   unit : Not implemented... will be used to provide access to the
 *          other uart registers...
 *   reg  : used for a 'switch' RR0, RR8, other. performs direct access
 *          for RR0 and RR8, performs indexed access for other.
 *   data : pointer to data buffer. data used to write UART, or buffer
 *          used to store data read from UART.
 *
 * Make sure referance is byte only, not quadword.
 */

static int
cnrw_8530(
    unsigned long access,
    unsigned long reg,
    unsigned char *data)
{
    char *base = (char *) regs;
        
    switch (reg) {
    case RR0:
        if (access == READ)
	    *data = *base;
        else {
            *base = *data;
            kdebug_mb();
        }
        break;

    case RR8:
        base = base + 0x40;
        if (access == READ)
	    *data = *base;
        else {
            *base = *data;
            kdebug_mb();
        }
        break;

    default:
        *base = (char) reg;      /* setup next access for correct register */
        kdebug_mb();
                
        if (access == READ)
	    *data = *base;
        else {
            *base = *data;
            kdebug_mb();
        }
        break;
    }
}

unsigned char
kdebug_gbus_rx_read()
{
    unsigned char result;
        
    cnrw_8530(READ, RR8, &result);
    return(result);
}

long
kdebug_gbus_tx_write(
    unsigned char c)
{
    cnrw_8530(WRITE, WR8, &c);
}

long
kdebug_gbus_rx_rdy()
{  
    unsigned char state;
        
    cnrw_8530(READ, RR0, &state);
    state &= (1<<00);
    return(state & 0x01);
}

long
kdebug_gbus_tx_rdy()
{
    unsigned char state;
        
    cnrw_8530(READ, RR0, &state);
    state &= (1<<GBUS_UART_TxEM);
    return(state);
}
