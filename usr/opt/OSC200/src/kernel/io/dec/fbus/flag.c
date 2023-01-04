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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif

#include <sys/types.h>
#include <vm/vm_kern.h>
#include <io/common/devdriver.h>
#include <io/dec/mbox/mbox.h>
#include <io/dec/fbus/flagreg.h>
#include <io/dec/fbus/fbusreg.h>
#include <machine/machparam.h>


flagconfl2(connbus, hose)
	struct bus *connbus;
	u_char hose;
{
	return(1);
}

extern	struct bus bus_list[];
kern_return_t allocvec();
u_short vecoffset();
void intrsetvec();
int flag_err_rtn();
int flag_errint();

flagconfl1(connbus, hose)
	struct bus *connbus;
	u_char hose;
{

	struct bus *fbus;
	vm_offset_t vecaddr;
	u_int nodeid;
	u_short voff;
	u_long *newvec;

	printf("fbus at %s%d hose %d", connbus->bus_name, 
		connbus->bus_num, hose);

        if((fbus = get_bus("fbus", hose, connbus->bus_name, connbus->bus_num)) ||
           (fbus = get_bus("fbus", hose, connbus->bus_name, -1)) ||
           (fbus = get_bus("fbus", -1, connbus->bus_name, connbus->bus_num)) ||
           (fbus = get_bus("fbus", -1, connbus->bus_name, -1)) ||
           (fbus = get_bus("fbus", -1, "*", -1))) {
		printf("\n");
	} else {
		/*
		 * we didn't find it in the config file,
		 * so we'll try to allocate a bus structure and 
		 * add it to the bus tree and mark it as here 
		 * but not alive
		 */
		if(fbus = (struct bus *)kalloc(sizeof(struct bus))) {
			fbus->bus_name = "fbus";
			fbus->bus_num = -99;
			fbus->alive = ALV_PRES;
			fbus->connect_bus = connbus->bus_name;
			fbus->connect_num = connbus->bus_num;
			MBOX_GET(connbus, fbus);
			((mbox_t)fbus->bus_mbox)->mbox_cmd = 
				(void *)fbus_mbox_cmd;
			((mbox_t)fbus->bus_mbox)->bus_timeout = 
				FBUS_TIMEOUT;
			((mbox_t)fbus->bus_mbox)->err_rtn = flag_err_rtn;
			conn_bus(connbus, fbus);
		}
		printf(" found but not configured.\n");
		return(0);
	}

	fbus->connect_bus = connbus->bus_name;
	fbus->connect_num = connbus->bus_num;
	MBOX_GET(connbus, fbus);
	((mbox_t)fbus->bus_mbox)->mbox_cmd = (void *)fbus_mbox_cmd;
	((mbox_t)fbus->bus_mbox)->bus_timeout = FBUS_TIMEOUT;
	((mbox_t)fbus->bus_mbox)->err_rtn = flag_err_rtn;
	conn_bus(connbus, fbus);

#ifdef DEBUG
	printf("FLAG FCTL (%x) = %x\n", FLAG_FCTL, RDCSR(LONG_32, fbus, FLAG_FCTL));
#endif /* DEBUG */
		/* enable FLAG error interrupts */
	WRTCSRS(LONG_32, fbus, FLAG_FCTL, FLAG_FCTL_V_ERRINT_ENABLE);
	mb();
#ifdef DEBUG
	printf("FLAG FCTL (%x) = %x\n", FLAG_FCTL, RDCSR(LONG_32, fbus, FLAG_FCTL));
#endif /* DEBUG */

		/* allocate interrupt vector for FLAG errors */
	do {
		if(allocvec(1,&newvec) != KERN_SUCCESS) {
			printf("flagconfl1: unable to allocvec\n");
			return(0);
		}
	}while (vecoffset(newvec) < 0x900); /* DEC_7000 uses vecs up to 0x900. */
	voff = vecoffset(newvec);
	intrsetvec(voff, &flag_errint, fbus);
		/* tell FLAG vector for servicing err interrupts */
	WRTCSRS(LONG_32, fbus, FLAG_FINT, voff);

	if(!fbusconfl1(connbus, fbus))
		panic("flagconfl1: fbusconfl1 failed");
	if(!fbusconfl2(connbus, fbus))
		panic("flagconfl1: fbusconfl2 failed");
	
	return(1);
}

flag_errint(fbus)
	struct bus *fbus;
{
		/* Clear Nonexistent_addr errors which occurred during probe time. */
	if((RDCSR(LONG_32, fbus, FLAG_ERRHI) & FLAG_ERRHI_V_MASK) == FLAG_ERRHI_V_NXA) {
#ifdef DEBUG
		printf("Fbus FLAG, clearing nxa int\n");
#endif /* DEBUG */
#ifdef notnow /* dont need explicit clear ? */
		WRTCSRS(LONG_32, fbus, FLAG_ERRHI, FLAG_ERRHI_V_NXA);
		mb();
#endif
		return(0);
	}
	printf("Fbus FLAG error interrupt:\n");
#define FLAG_ERR_PRINT(reg) \
	printf("FLAG reg (%x) = %x\n", reg, RDCSR(LONG_32, fbus, reg))
	DELAY(1000000);
	FLAG_ERR_PRINT(FLAG_FINT);
	FLAG_ERR_PRINT(FLAG_NID);
	FLAG_ERR_PRINT(FLAG_STO);
	DELAY(2000000);
	FLAG_ERR_PRINT(FLAG_ERRHI);
	FLAG_ERR_PRINT(FLAG_ERRLO);
	FLAG_ERR_PRINT(FLAG_FADDRHI);
	FLAG_ERR_PRINT(FLAG_FADDRLO);
	DELAY(2000000);
	FLAG_ERR_PRINT(FLAG_TTO);
	FLAG_ERR_PRINT(FLAG_BZRTRY);
	FLAG_ERR_PRINT(FLAG_FCTL);
	DELAY(2000000);
	FLAG_ERR_PRINT(FLAG_DIAG);
	FLAG_ERR_PRINT(FLAG_FGPR);
	FLAG_ERR_PRINT(FLAG_FERR);
	DELAY(1000000);
}

static char *flag_panicstr[] = {
	"success", /* place holder */
	"hunghose", 
	"qfull", 
	"mbox_error",
	""
};

flag_err_rtn(mbp, rtn, err)
	register mbox_t mbp;
	register char *rtn;
	register int err;
{
	flag_errint((bus_ctlr_common_t)mbp->bus_ctlr_ptr);
	dumpmbox(mbp);
	printf("%s: %s\n", rtn, flag_panicstr[err]);
	mbp->mb_status = MBOX_DON_BIT;
	mb();
	panic(flag_panicstr[err]);
}
