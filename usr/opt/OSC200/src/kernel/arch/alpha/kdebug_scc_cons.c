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
static char *rcsid = "@(#)$RCSfile: kdebug_scc_cons.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/27 14:18:21 $";
#endif

#include <sys/types.h>
#include <io/common/devio.h>
#include <io/dec/tc/sccreg.h>

#define SCC_READ(reg, val) { \
    regs->SCC_CMD = ((unsigned int)(reg))<<8; \
    kdebug_mb(); \
    (val) = ((regs->SCC_CMD)>>8)&0xff; \
    kdebug_mb(); \
    }

#define SCC_WRITE(reg, val) { \
    regs->SCC_CMD = ((unsigned int)(reg))<<8; \
    kdebug_mb(); \
    regs->SCC_CMD = ((unsigned int)(val))<<8; \
    kdebug_mb(); \
    }

static struct scc_reg *regs;

long
kdebug_scc_init(
    unsigned long addr)
{
    regs = (struct scc_reg *) addr;
    return(0);
}

unsigned char
kdebug_scc_rx_read()
{
    unsigned char result;

    SCC_READ(SCC_RR8, result);
    return(result);
}

long
kdebug_scc_tx_write(
    unsigned char c)
{
    SCC_WRITE(SCC_WR8, c);
}

long
kdebug_scc_rx_rdy()
{
    unsigned char state;

    SCC_READ(SCC_RR0, state);
    return(state & SCC_RR0_RCHAR_AVAIL);
}

long
kdebug_scc_tx_rdy()
{
    unsigned char state;

    SCC_READ(SCC_RR0, state);
    return(state & SCC_RR0_TBUF_EMPTY);
}
