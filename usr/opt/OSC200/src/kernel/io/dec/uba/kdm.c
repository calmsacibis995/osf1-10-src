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

#define DEBUG_PRINT

/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: kdm.c,v $ $Revision: 1.1.2.7 $ (DEC) $Date: 1992/10/06 14:12:50 $";
#endif
/* ------------------------------------------------------------------------
 * Modification History:
 *
 *	03-Aug-1990	rafiey (Ali Rafieymehr)
 *		Changes for Stuart Hollander for multiple XMIs support.
 *
 *	20-Jul-1989	map (Mark Parenti)
 *		First version. Derived from bda.c.
 *
 *
 * ------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/buf.h>
#include <sys/map.h>
#include <sys/param.h>
#include <sys/vmmac.h>
#include <vm/vm_kern.h>

#include <machine/scb.h>
#include <sys/config.h>

#include <io/dec/scs/sca.h>
#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/xmi/xmireg.h>
#include <io/dec/xmi/sspxmi.h>
#include <io/common/devdriver.h>

extern int numuba;
extern int numxmi;
extern int nNUBA;

/*
 *
 *
 */
#define PHYS(addr)	((long) \
		((long)ptob(Sysmap[btop((long)(addr)&0x7fffffff)].pg_pfnum)  \
			| (long)( PGOFSET & (long)(addr) ) ) )

extern boolean_t config_ctlr();

kdminit(nxv, nxp, ctlr, xmidata)
	struct kdm_regs *nxv;
	caddr_t	*nxp;
	register struct controller *ctlr;
	register struct xmidata *xmidata;
{
	register struct bus *bus = xmidata->bus;
	char *vuba;
	vm_offset_t puba;
	register struct uba_hd *uhp;
	int savenumuba;
	int status;

	savenumuba = numuba;
	numuba = ctlr->ctlr_num;
	
#ifdef DEBUG_PRINT_OFF
printf(">>> kdminit(0x%lx, 0x%lx, 0x%lx, 0x%lx)\n",
       (u_long)nxv, (u_long)nxp, (u_long)ctlr, (u_long)xmidata);
#endif /* DEBUG_PRINT */

/*
	SCA_KM_ALLOC(vuba, char *, 2048, KM_SCABUF, KM_NOW_CL_CO_CA)
*/
	vuba = (char *)kmem_alloc(kernel_map, sizeof(struct uba_regs));
	if (vuba == 0) {
		printf("SCA_KM_ALLOC returned 0 size \n");
		return;
	}

#ifdef __vax
	puba = (char *) PHYS(vuba);
	puba = (char *) (((((long)puba)) & 0x7ffffe00)- 2048);
	vuba = (char *) ((((((long)vuba)) & 
				0x7ffffe00)- 2048) | 0x80000000);
#endif /* __vax */
#ifdef __mips
	puba = (char *) PHYS(vuba);
	puba = (char *) (((((long)puba)) & 0x3ffffe00)- 2048);
	vuba = (char *) ((((((long)vuba)) & 
				0x3ffffe00)- 2048) | 0xc0000000);
#endif /* __mips */
#ifdef __alpha
	(void) svatophys (vuba, &puba);
#endif /* __alpha */

	uhp = &uba_hd[numuba];
	
	uhp->uh_vec = SCB_XMI_ADDR(xmidata);
	uhp->uh_lastiv = SCB_XMI_LWOFFSET(ctlr->slot, LEVEL15)+4;
	uhp->uba_type = UBAXMI;

	numxmi = bus->bus_num;

/************************************************************************/
/*	unifind(vuba, puba, nxv, nxp, 0, nxp, nxv, 0, xminum, xminode); */

	/*
	 * Initialize the "UNIBUS", by freeing the map
	 * registers and the buffered data path registers
	 */
	bus->uba_hdp = ( caddr_t )uhp; /* this actually goes in private[5] */

	uhp->uh_map = (struct map *) kalloc(UAMSIZ*sizeof(struct map));
	rminit(uhp->uh_map, (long)NUBMREG, (long)1, "uba", UAMSIZ);

	uhp->uq_map = (struct map *) kalloc(QAMSIZ*sizeof(struct map));
	rminit(uhp->uq_map, (long)QBMREG - 1024, (long)1, "qba", QAMSIZ);

	/*
	 * Save virtual and physical addresses
	 * of adaptor
	 */
	uhp->uh_uba = (struct uba_regs *) vuba;
	uhp->uh_physuba = (struct uba_regs *) puba;

#ifdef DEBUG_PRINT_OFF
printf(">>> kdminit: uhp=0x%lx  uh_vec=0x%x  uh_lastiv=0x%x  uh_map=0x%lx\n",
       (u_long)uhp, (u_long)uhp->uh_vec, uhp->uh_lastiv, (u_long)uhp->uh_map);
printf(">>> kdminit: vuba=0x%lx, puba=0x%lx, numuba: %d -> %d, numxmi=%d\n",
       (u_long)vuba, (u_long)puba, savenumuba, numuba, numxmi);
#endif /* DEBUG_PRINT */

	/*
	 * callback to xmiinit routine to finish configuration of ctlr
	 *
	 * NOTE: must pass address of actual ctlr registers as first param,
	 *       NOT the address of the XMI registers of the controller
	 */
	status = config_ctlr((caddr_t)nxv + 0x40, nxp, ctlr, xmidata);
	
	numuba = savenumuba;

	return(status);
}
