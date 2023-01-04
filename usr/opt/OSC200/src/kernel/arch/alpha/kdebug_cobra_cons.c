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
static char *rcsid = "@(#)$RCSfile: kdebug_cobra_cons.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/09/27 14:18:15 $";
#endif

/*
 * Description of kdebug_cobra_tt(command,unit,character)
 *
 * command      = 0:    init
 *              = 1:    getc_ready
 *                      in:     unit
 *                      return val = 0, not ready
 *                                 = 1, ready
 *              = 2:    getc
 *                      in:    init
 *                      return value = character
 *              = 3:    putc_ready
 *                      in:     unit
 *                      out:    return val = 0, not ready
 *                                 = 1, ready
 *              = 4:    putc
 *                      in:     unit, character
 *              = 5:    rx_int
 *                      in:     unit
 *                              character = 0, disable
 *                                        = 1, enable
 *                      return val = 0, previously disabled
 *                                 = 1, previously enabled
 *              = 6:    tx_int
 *                      in:     unit
 *                              character = 0, disable
 *                                        = 1, enable
 *                      return val = 0, previously disabled
 *                                 = 1, previously enabled
 */

#define CONS_INIT       0
#define CONS_RXRDY      1
#define CONS_GETC       2
#define CONS_TXRDY      3
#define CONS_PUTC       4
#define DUMMY           0
#define UNIT_1          1

long
kdebug_cobra_init(
    unsigned long addr)
{
    (void) kdebug_cobra_tt(CONS_INIT, UNIT_1, (unsigned char)DUMMY);
    return(0);
}

unsigned char
kdebug_cobra_rx_read()
{
    return((unsigned char)kdebug_cobra_tt(CONS_GETC, UNIT_1, (unsigned char)DUMMY));
}

long
kdebug_cobra_tx_write(
    unsigned char c)
{
    (void) kdebug_cobra_tt(CONS_PUTC, UNIT_1, c);
    kdebug_mb();
}

long
kdebug_cobra_rx_rdy()
{  
    return(kdebug_cobra_tt(CONS_RXRDY, UNIT_1, (unsigned char)DUMMY));
}

long
kdebug_cobra_tx_rdy()
{
    return(kdebug_cobra_tt(CONS_TXRDY, UNIT_1, (unsigned char)DUMMY));
}
