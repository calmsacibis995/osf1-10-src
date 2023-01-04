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
static char *rcsid = "@(#)$RCSfile: ciconfig.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/10 10:22:31 $";
#endif
/*
 * Abstract:
 */

#include <sys/types.h>
#include <io/common/devdriver.h>
#include <io/dec/tc/tc.h>


int     cidebug = 0;
/* Define debugging stuff.
 */
#define DEBUG
#ifdef DEBUG
#define Cprintf if(cidebug)printf
#define Dprintf if( cidebug >= 2 )printf
#else
#define Cprintf ;
#define Dprintf ;
#endif

ciconfl1(bustype, binfo, bus)
int bustype;
caddr_t binfo;
struct bus *bus;
{

	caddr_t addr, physaddr;
	int slot, unit;
	int (**intr)();
	int (*confrtn)();

	switch (bustype) {

	      case BUS_TC:
		{
		    struct tc_info *tinfo = (struct tc_info *)binfo;

		    confrtn = tc_slot[(int)bus->tcindx].adpt_config;

		    Cprintf("ciconfl1: index = %d\n",(int)bus->tcindx); 
		    Cprintf("ciconfl1: addr = 0x%x, physaddr = 0x%x, slot = %d, unit = %d, confrtn = 0x%x\n", tinfo->addr, tinfo->physaddr, tinfo->slot, tinfo->unit, confrtn);
		    Cprintf("ciconfl1: Call configuration routine\n");

		    return((*confrtn)( binfo, bus));
		}

	      case BUS_XMI:
		{
/* ciconfl1() does not get called during xmi bus config since xminpinit is
   called from the xmisw table, not from the bus structure. 
		    confrtn = xminpinit;
		    return((*confrtn)(addr, physaddr, xminumber, xminode, xmidata, bus));
*/
		}
	      default:
		panic("ciconfl1: Unsupported bus type \n");
	}

}
		
ciconfl2(bustype, binfo, bus)
int bustype;
caddr_t binfo;
struct bus *bus;
{
	return(1);
}
