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
static char *rcsid = "@(#)$RCSfile: kdebug_conf.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/07 21:01:28 $";
#endif

#include <sys/kdebug.h>
#include <machine/cpu.h>

long kdebug_scc_init();
unsigned char kdebug_scc_rx_read();
long kdebug_scc_tx_write();
long kdebug_scc_tx_rdy();
long kdebug_scc_rx_rdy();

long kdebug_cobra_init();
unsigned char kdebug_cobra_rx_read();
long kdebug_cobra_tx_write();
long kdebug_cobra_tx_rdy();
long kdebug_cobra_rx_rdy();

long kdebug_gbus_init();
unsigned char kdebug_gbus_rx_read();
long kdebug_gbus_tx_write();
long kdebug_gbus_tx_rdy();
long kdebug_gbus_rx_rdy();

long kdebug_ace_init();
unsigned char kdebug_ace_rx_read();
long kdebug_ace_tx_write();
long kdebug_ace_tx_rdy();
long kdebug_ace_rx_rdy();

static struct device_table scc_dev = {
    kdebug_scc_init,		kdebug_scc_rx_read,	kdebug_scc_tx_write,
    kdebug_scc_rx_rdy,		kdebug_scc_tx_rdy
    };

static struct device_table cobra_dev = {
    kdebug_cobra_init,		kdebug_cobra_rx_read,	kdebug_cobra_tx_write,
    kdebug_cobra_rx_rdy,	kdebug_cobra_tx_rdy
    };

static struct device_table gbus_dev = {
    kdebug_gbus_init,		kdebug_gbus_rx_read,	kdebug_gbus_tx_write,
    kdebug_gbus_rx_rdy,	kdebug_gbus_tx_rdy
    };

static struct device_table ace_dev = {
    kdebug_ace_init,		kdebug_ace_rx_read,	kdebug_ace_tx_write,
    kdebug_ace_rx_rdy,		kdebug_ace_tx_rdy
    };

#define SCC_FLAMINGO_ADDR (0xfffffc0000000000 | (0x1f0000000 + 0x200000))
#define GBUS_RUBY_ADDR (0xfffffc0000000000 | 0x3F4800000)
#define ACE_JENSEN_ADDR 0x2f8
#define SCC_PELICAN_ADDR (0xfffffc0000000000 | (0x1b0000000 + 0x200000))
#define ACE_MORGAN_ADDR 0x2f8

static struct kdebug_cpusw _kdebug_cpusw[] = 
{
    {	ST_DEC_3000_500,	"DEC3000 400/500",
        &scc_dev,		SCC_FLAMINGO_ADDR
    },
    {	ST_DEC_4000,		"DEC4000",
        &cobra_dev,		0x0,
    },
    {	ST_DEC_7000,		"DEC7000",
        &gbus_dev,		GBUS_RUBY_ADDR
    },
    {	ST_JENSEN,		"JENSEN",
        &ace_dev,		ACE_JENSEN_ADDR
    },
    {	ST_DEC_3000_300,	"DEC3000 300",
        &scc_dev,		SCC_PELICAN_ADDR
    },
    {	ST_MORGAN,		"MORGAN",
        &ace_dev,		ACE_MORGAN_ADDR
    },
    {	0,			0,
	0,			0
    }
};

struct kdebug_cpusw *kdebug_cpup;

/*
 * Get pointer to cpusw table entry for the system we are currently running
 * on.  The pointer returned by this routine will go into "cpup".
 *
 * The "cpu" variable is passed in and compared to the
 * system_type entry in the cpusw table for a match.
 */
struct kdebug_cpusw *
kdebug_cpuswitch_entry(
    unsigned long cpu)
{
    unsigned long i;

    for (i = 0; _kdebug_cpusw[i].system_type != 0; i++) {
        if (_kdebug_cpusw[i].system_type == cpu)
            return(&_kdebug_cpusw[i]);
    }
    return(0);
}
