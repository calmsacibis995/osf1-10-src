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
static char *rcsid = "@(#)$RCSfile: xlamb.c,v $ $Revision: 1.2.9.2 $ (DEC) $Date: 1993/05/25 21:29:14 $";
#endif


#include <sys/types.h>
#include <sys/time.h>

#include <vm/vm_map.h>
#include <io/dec/mbox/mbox.h>
#include <io/dec/lsb/lsb_iopreg.h>
#include <io/dec/xmi/xlambreg.h>
#include <io/dec/xmi/xmireg.h>
#include <io/common/devdriver.h>
#include <machine/scb.h>

#include <dec/binlog/errlog.h>

extern int hz;

void 	lamb_mbox_cmd();
int 	lamberror();
int 	lamb_set_diag();
u_int 	lamb_interrupted = 0;
int	lamb_errors = 1;


extern	struct bus bus_list[];


lambconfl1(connbus, xminum)
	struct bus *connbus;
	u_int xminum;
{
	volatile struct lambreg	*lva;
	struct xmidata *xmidata;
	struct bus *xmibus;
	vm_offset_t vecaddr;
	u_int nodeid;

	/*
	 * the xmi number is derived as follows:
	 * xminum = connecting bus # << 12 | iop node << 8 | hose 
	 */
	printf("xmi%d at %s%d hose %d", xminum, connbus->bus_name, 
		connbus->bus_num, (xminum & 0xff));
	
	/*
	 * we do this here because get_bus() uses slot number as 
	 * a comparator, and slot number is irrelevant to us.
	 * we're looking to match on the name, xmi number.
	 * if( bus name == xmi && bus num == xminum && 
	 *   (bus conn num == connbus num || bus conn num == wildcard) &&
	 *   (bus conn name == connbus name || bus connname == wildcard) &&
	 *   bus ! alive)
	 *      found it!
	 */
	for(xmibus = bus_list; xmibus->bus_name != 0; xmibus++) {
		if( (!strcmp(xmibus->bus_name, "xmi")) &&
		    (xmibus->bus_num == xminum) &&
		    ((xmibus->connect_num == connbus->bus_num) ||
		     (xmibus->connect_num == -1) ) &&
		    ( (!strcmp(xmibus->connect_bus, connbus->bus_name)) ||
		    (!strcmp(xmibus->connect_bus, "*")) ) && 
		   (xmibus->alive == 0))
			break;
	}

	if( ! xmibus->bus_name) {
		/*
		 * we didn't find it in the config file,
		 * so we'll try to allocate a bus structure and 
		 * add it to the bus tree and mark it as here 
		 * but not alive
		 */
		if(xmibus = (struct bus *)kalloc(sizeof(struct bus))) {
			xmibus->bus_name = "xmi";
			xmibus->bus_num = -99;
			xmibus->alive = ALV_PRES;
			xmibus->connect_bus = connbus->bus_name;
			xmibus->connect_num = connbus->bus_num;
			MBOX_GET(connbus, xmibus);
			((mbox_t)xmibus->bus_mbox)->mbox_cmd = 
				(void *)lamb_mbox_cmd;
			((mbox_t)xmibus->bus_mbox)->bus_timeout = 
				XMI_BUS_TIMEOUT;
			conn_bus(connbus, xmibus);
		}
		printf(" found but not configured.\n");
		return(0);
	}

	printf("\n");
	xmibus->connect_bus = connbus->bus_name;
	xmibus->connect_num = connbus->bus_num;
	MBOX_GET(connbus, xmibus);
	((mbox_t)xmibus->bus_mbox)->mbox_cmd = (void *)lamb_mbox_cmd;
	((mbox_t)xmibus->bus_mbox)->bus_timeout = XMI_BUS_TIMEOUT;
	
	if((xmidata = (struct xmidata *)kalloc(sizeof(struct xmidata))) == 0)
		panic("lambconfl1: cannot alloc xmidata struct");
	xmidata->next = head_xmidata;
	head_xmidata = xmidata;
	xmidata->xminum = xminum;
	xmidata->xmiphys = (struct xmi_reg *)XMI_START_PHYS;
	/* we use node private addr space for the lamb */
	xmidata->cpu_xmi_addr = (struct xmi_reg *)XMI_BASE_PHYS;
	lva = (struct lambreg *) XMI_BASE_PHYS;

	xmibus->bus_xmidata = (caddr_t)xmidata;
	xmidata->bus = xmibus;
	
	/* get the lamb xmi node id */
	nodeid = RDCSR(LONG_32, xmibus, &lva->ldiag);
	nodeid = (nodeid & LAMB_NODE_ID) >> 28;
	xmidata->xmiintr_dst = 1 << nodeid;
	
	/* disable lamb interrupts */
	/*** might want to let err ints through ***/
	WRTCSR( LONG_32, xmibus, &lva->imsk, 0);
	/* cautious: clean xber before touching anything */
	WRTCSR( LONG_32, xmibus, &lva->xber,
	       (XMI_CC | XMI_WEI | XMI_IPE | XMI_WSE | 
		XMI_RIDNAK | XMI_WDNAK | XMI_CRD | XMI_NRR |
		XMI_RSE | XMI_RER | XMI_CNAK | XMI_TTO |
		XMI_EHWW | XMI_MORE));
	
	/* allocate an scb vector for the lamb */
	if(allocvec(1, &vecaddr) != KERN_SUCCESS)
		panic("lambconfl1: allocvec");
	/* set lamb SCB entry */	
	intrsetvec(vecoffset(vecaddr), lamberror, xmibus);
	/* now tell lamb its scb vector */
	WRTCSR( LONG_32, xmibus, &lva->levr, vecoffset(vecaddr));

	/* allocate the xmi scb entries */
	if(allocvec(NXMI_VEC, &vecaddr) != KERN_SUCCESS)
		panic("lambconfl1: no xmi scb entries");
	/* and point the xmi to them */
	xmidata->xmivec_page = (int (**)())vecaddr;
	
	/* 
	 * the xmiconfl rtns will connect the xmi to 
	 * the connbus (iop) and mark it alive
	 */
	if(!(*xmibus->confl1)(connbus, xmibus))
		panic("lambconfl1: xmiconfl1");
	if(!(*xmibus->confl2)(connbus, xmibus))
		panic("lambconfl1: xmiconfl2");
	
	/*** clean up any lamb csr's after probes ***/
	/*
	 * the lerr must be written before the xber
	 * after an err int, the xber must be written
	 * to allow the lamb to send another err int
	 * even if there were no xber err's
	 * see chap 7 of the lamb spec
	 */
	WRTCSR( LONG_32, xmibus, &lva->lerr,
	       (LERR_DHDPE | LERR_MBPE | LERR_MBIC | LERR_MBIA | 
		LERR_DFDPE | LERR_RBDPE | LERR_MBOF | LERR_FE));
	WRTCSR( LONG_32, xmibus, &lva->xber, 
	       (XMI_CC | XMI_WEI | XMI_IPE | XMI_WSE | XMI_RIDNAK | 
		XMI_WDNAK | XMI_CRD | XMI_NRR | XMI_RSE | 
		XMI_RER | XMI_CNAK | XMI_TTO |
		XMI_EHWW | XMI_MORE));
	/*** re-enable lamb interrupts here ***/
	WRTCSR( LONG_32, xmibus, &lva->imsk,
	       (IMSK_ICC | IMSK_IWEI | IMSK_IIPE |
		IMSK_IXPE | IMSK_IWSE | IMSK_IRIDNAK |
		IMSK_IWDNAK | IMSK_ICRD | IMSK_INRR |
		IMSK_IRSE | IMSK_IRER | IMSK_ICNAK | 
		IMSK_ITTO | IMSK_IDFDPE | 
		IMSK_IRBDPE | IMSK_IMBER));


	return(1);
}

int
lambconfl2(connbus, xminum)
	struct bus *connbus;
	u_int xminum;
{
	return(1);
}

/*
 * Got a lamb error interrupt, log and panic
*/
int
lamberror(xmibus)
	struct bus *xmibus;
{

    if ( lamb_errors )
    {
	lamb_interrupted = 1;
	log_lamberror(xmibus);
	lamb_interrupted = 0;

	panic("lamberror");
    }

    return ( 1 );
}

/*
 * Just log lamb errors and call to log_xmierrors
*/
int
log_lamberror(xmibus)
	struct bus *xmibus;
{
	volatile struct lambreg *lva;
	struct xmidata *xmidata;
	u_int xber, lerr;
	extern log_xmierrors();

	struct elr_soft_flags	sw_flags;

	u_int	errors_found = 0;

	sw_flags.error_flags[0] = 0;
	sw_flags.error_flags[1] = 0;

	xmidata = get_xmi(xmibus->bus_num);
	lva = (struct lambreg *) xmidata->cpu_xmi_addr;

	lerr = RDCSR( LONG_32, xmibus, &lva->lerr );
	xber = RDCSR( LONG_32, xmibus, &lva->xber );

	/*
	** Parse the error
	*/
	printf ( "\n\n" );

	if ( xber & XMI_NSES )
	{
		if ( lerr & LERR_DHDPE )
		{
		    printf ( "lamberror: down hose parity error, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_DHDPE_ERROR;
		}

		if ( lerr & LERR_MBPE )
		{
		    printf ( "lamberror: mailbox parity error, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_MBPE_ERROR;
		}

		if ( lerr & LERR_MBIC )
		{
		    printf ( "lamberror: illegal mailbox command, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_MBIC_ERROR;
		}

		if ( lerr & LERR_MBIA )
		{
		    printf ( "lamberror: illegal mailbox address, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_MBIA_ERROR;
		}

		if ( lerr & LERR_DFDPE )
		{
		    printf ( 
			"lamberror: down hose FIFO data parity error, hose %d",
			xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_DFDPE_ERROR;
		}

		if ( lerr & LERR_RBDPE )
		{
		    printf ( 
			"lamberror: read buffer data parity error, hose %d",
			xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_RBDPE_ERROR;

		    sw_flags.error_flags[1] |= SW_FLAG1_XMI_SUBPACKET_PRES;
		}
		
		if ( lerr & LERR_MBOF )
		{
		    /*
		    ** Handled by the IOP error interrupt routine
		    */
		    printf ( "lamberror: mailbox overflow, hose %d",
				xmibus -> bus_num );
		}

		if ( lerr & LERR_FE )
		{
		    /*
		    ** Handled by the IOP error interrupt routine
		    */
		    printf ( "lamberror: fatal error, hose %d",
				xmibus -> bus_num );
		}

	}
	else if ( xber & XMI_ES )
	{
		if ( xber & XMI_WEI )
		{
		    printf ( "lamberror: XMI write error interrupt, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_WEI_ERROR;

		    sw_flags.error_flags[1] |= SW_FLAG1_XMI_SUBPACKET_PRES;
		}

		if ( xber & XMI_CC )
		{
		    printf ( "lamberror: corrected confirmation, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_RESPONDER_ERROR;
		    sw_flags.error_flags[0] |= SW_LAMB_CC_ERROR;

		    sw_flags.error_flags[1] |= SW_FLAG1_XMI_SUBPACKET_PRES;
		}

		if ( xber & XMI_IPE )
		{
		    printf ( "lamberror: inconsistent parity error, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_RESPONDER_ERROR;
		    sw_flags.error_flags[0] |= SW_LAMB_IPE_ERROR;

		    sw_flags.error_flags[1] |= SW_FLAG1_XMI_SUBPACKET_PRES;
		}

		if ( xber & XMI_CRD )
		{
		    printf ( "lamberror: corrected read data response, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_COMMANDER_ERROR;
		    sw_flags.error_flags[0] |= SW_LAMB_CRD_ERROR;

		    sw_flags.error_flags[1] |= SW_FLAG1_XMI_SUBPACKET_PRES;
		}

		if ( xber & XMI_RER )
		{
		    printf ( "lamberror: read error response, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_COMMANDER_ERROR;
		    sw_flags.error_flags[0] |= SW_LAMB_RER_ERROR;

		    sw_flags.error_flags[1] |= SW_FLAG1_XMI_SUBPACKET_PRES;
		}

		if ( xber & XMI_RSE )
			
			if ( xber & XMI_PE )
			{
		    	    printf ( 
			"lamberror: read sequence error parity error, hose %d",
				xmibus -> bus_num );
			    errors_found++;

			    sw_flags.error_flags[0] |= SW_LAMB_COMMANDER_ERROR;
			    sw_flags.error_flags[0] |= SW_LAMB_RSE_ERROR;
			    sw_flags.error_flags[0] |= SW_LAMB_PE_ERROR;

			    sw_flags.error_flags[1] |= 
						SW_FLAG1_XMI_SUBPACKET_PRES;
			}
			else
			{
		    	    printf ( "lamberror: read sequence error, hose %d",
				xmibus -> bus_num );
			    errors_found++;

			    sw_flags.error_flags[0] |= SW_LAMB_COMMANDER_ERROR;
			    sw_flags.error_flags[0] |= SW_LAMB_RSE_ERROR;

			    sw_flags.error_flags[1] |=
						SW_FLAG1_XMI_SUBPACKET_PRES;
			}

		if ( xber & XMI_TTO )

			if ( xber & XMI_WDNAK )

				if ( xber & XMI_PE )
				{
				    printf ( 
			"lamberror: write data noack parity error, hose %d",
					xmibus -> bus_num );
				    errors_found++;

				    sw_flags.error_flags[0] |=
						SW_LAMB_COMMANDER_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_TTO_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_WDNAK_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_PE_ERROR;

				    sw_flags.error_flags[1] |=
						SW_FLAG1_XMI_SUBPACKET_PRES;
				}
				else
				{
				    printf ( 
					"lamberror: write data noack, hose %d",
					xmibus -> bus_num );
				    errors_found++;

				    sw_flags.error_flags[0] |=
						SW_LAMB_COMMANDER_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_TTO_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_WDNAK_ERROR;

				    sw_flags.error_flags[1] |=
						SW_FLAG1_XMI_SUBPACKET_PRES;
				}

			else if ( xber & XMI_CNAK )

				if ( xber & XMI_PE ) 
				{
				    printf ( 
			"lamberror: command noack parity error, hose %d",
					xmibus -> bus_num );
				    errors_found++;

				    sw_flags.error_flags[0] |=
						SW_LAMB_COMMANDER_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_TTO_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_PE_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_CNAK_ERROR;

				    sw_flags.error_flags[1] |=
						SW_FLAG1_XMI_SUBPACKET_PRES;
				}
				else
				{
				    printf ( 
					"lamberror: command noack, hose %d",
					xmibus -> bus_num );
				    errors_found++;

				    sw_flags.error_flags[0] |=
						SW_LAMB_COMMANDER_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_TTO_ERROR;
				    sw_flags.error_flags[0] |=
						SW_LAMB_CNAK_ERROR;

				    sw_flags.error_flags[1] |=
						SW_FLAG1_XMI_SUBPACKET_PRES;
				}

			else if ( xber & XMI_NRR )
	
				if ( xber & XMI_PE )
				{
				    printf ( 
			"lamberror: no read response parity error, hose %d",
					xmibus -> bus_num );
				    errors_found++;

				    sw_flags.error_flags[0] |= 
						SW_LAMB_COMMANDER_ERROR;
				    sw_flags.error_flags[0] |= 
						SW_LAMB_TTO_ERROR;
				    sw_flags.error_flags[0] |= 
						SW_LAMB_NRR_ERROR;
				    sw_flags.error_flags[0] |= 
						SW_LAMB_PE_ERROR;

				    sw_flags.error_flags[1] |= 
						SW_FLAG1_XMI_SUBPACKET_PRES;
				}
				else
				{
				    printf ( 
					"lamberror: no read response, hose %d",
					xmibus -> bus_num );
				    errors_found++;

				    sw_flags.error_flags[0] |= 
						SW_LAMB_COMMANDER_ERROR;
				    sw_flags.error_flags[0] |= 
						SW_LAMB_TTO_ERROR;
				    sw_flags.error_flags[0] |= 
						SW_LAMB_NRR_ERROR;

				    sw_flags.error_flags[1] |= 
						SW_FLAG1_XMI_SUBPACKET_PRES;
				}

			else
			{
			    printf ( "lamberror: no XMI grant, hose %d",
				xmibus -> bus_num );
			    errors_found++;

			    sw_flags.error_flags[0] |= SW_LAMB_COMMANDER_ERROR;
			    sw_flags.error_flags[0] |= SW_LAMB_TTO_ERROR;
			}

		else if ( xber & XMI_RIDNAK )

			if ( xber & XMI_PE )
			{
			    printf ( 
		"lamberror: read/ident data noack parity error, hose %d",
				xmibus -> bus_num );
			    errors_found++;

			    sw_flags.error_flags[0] |= SW_LAMB_RESPONDER_ERROR;
			    sw_flags.error_flags[0] |= SW_LAMB_RIDNAK_ERROR;
			    sw_flags.error_flags[0] |= SW_LAMB_PE_ERROR;

			    sw_flags.error_flags[1] |= 
						SW_FLAG1_XMI_SUBPACKET_PRES;
			}
			else
			{
			    printf ( 
				"lamberror: read/ident data noack, hose %d",
				xmibus -> bus_num );
			    errors_found++;

			    sw_flags.error_flags[0] |= SW_LAMB_RESPONDER_ERROR;
			    sw_flags.error_flags[0] |= SW_LAMB_RIDNAK_ERROR;

			    sw_flags.error_flags[1] |= 
						SW_FLAG1_XMI_SUBPACKET_PRES;
			}

		else if ( xber & XMI_WSE )

			if ( xber & XMI_PE )
			{
			    printf ( 
			"lamberror: write sequence error parity error, hose %d",
				xmibus -> bus_num );
			    errors_found++;

			    sw_flags.error_flags[0] |= SW_LAMB_WSE_ERROR;
		    	    sw_flags.error_flags[0] |= SW_LAMB_PE_ERROR;

		    	    sw_flags.error_flags[1] |= 
						SW_FLAG1_XMI_SUBPACKET_PRES;
			}
			else
			{
			    printf ( "lamberror: write sequence error, hose %d",
				xmibus -> bus_num );
			    errors_found++;

			    sw_flags.error_flags[0] |= SW_LAMB_WSE_ERROR;

		    	    sw_flags.error_flags[1] |= 
						SW_FLAG1_XMI_SUBPACKET_PRES;
			}

		else if ( xber & XMI_PE )
		{
		    printf ( "lamberror: parity error, hose %d",
				xmibus -> bus_num );
		    errors_found++;

		    sw_flags.error_flags[0] |= SW_LAMB_RESPONDER_ERROR;
		    sw_flags.error_flags[0] |= SW_LAMB_PE_ERROR;

		    sw_flags.error_flags[1] |= SW_FLAG1_XMI_SUBPACKET_PRES;
		}
	}

	/*
	** If no acual error bits were found to be set, just log an
	** "Inconsistent Error", *IF* we got here via interrupt
	*/
	if ( errors_found == 0 )
	{
	    if ( lamb_interrupted )
	    {
		printf ( "lamberror: inconsistent error interrupt" );
		errors_found++;

		sw_flags.error_flags[0] |= SW_LAMB_INCON;
	    }
	}

	/*
	** Log the error to the binary error logger, and dump to console,
	** if any errors were found
	*/

	if ( errors_found > 0 )	
	    lamb_log_error ( &sw_flags, xmibus );


	/* now look for errors on the xmi */
	log_xmierrors(xmibus->bus_num);

	/*
	** Attempt to reset the I/O subsystem to give the console the best 
	** shot at making the dump file.  
	** We just call iop_reset, since resetting the IOP resets all of the
	** LAMBS/FLAGS and there is no interface to reset a single LAMB
	** ( NRST not implemented ).
	*/
	iop_reset ( xmibus -> bus_hd );


	return ( 1 );
}

/*
 * there is a stub lamb_mbox_cmd() in autoconf_data.c if there is no lamb
 */
void
lamb_mbox_cmd(mbp, rwflag, wmask_as)
	struct mbox *mbp;          /* mailbox pointer */
	u_int rwflag;              /* bus/adapter specific command */
	u_int wmask_as;            /* data type, wrtite mask and addr space */
{

#define WRT_XMI_CSR 0x00000007

	mbp->cmd = XMI_CSR;
	if(rwflag & WRT_CSR)
		mbp->cmd |= WRT_XMI_CSR;
	/* 
	 * lamb I/O access
	 * <31:30:29> = 011#2, lamb spec, rev 1.0, pg 92
	 */
	mbp->rbadr |= 0x60000000;
}

int
xlambinit(nxv, nxp, xminum, xminode, xmidata, xmibus)
{
	/* do nothing */
	return(0);
}




lamb_log_error ( struct elr_soft_flags* sw_flags, 
		 struct bus* xmibus )

{

    struct el_rec*			elp;
    vm_offset_t				el_data_ptr;

    u_int				el_size;

    struct el_laser_lamb_data*		lamb_subp_ptr;
    struct elr_xmi_subpacket*		xmi_subp_ptr;

    u_int				i, logger_running;

    volatile struct lambreg		*lva;
    volatile struct xmi_reg 		*nxv; /* virtual pointer to XMI node */

    register struct xmidata 		*xmidata;
    u_int				xminode;

    /*
    ** Calculate the errlog buffer size that's required
    */
    el_size = sizeof ( struct el_laser_lamb_data );

    /* Any more subpackets? */
    if ( sw_flags -> error_flags[1] & SW_FLAG1_XMI_SUBPACKET_PRES )
	el_size += sizeof ( struct elr_xmi_subpacket );

    /*
    ** Allocate an errlog buffer
    */

    elp = (struct el_rec *) binlog_alloc ( el_size, EL_PRISEVERE );
    if ( elp == (struct el_rec *) BINLOG_NOBUFAVAIL )
    {
	/*
	** Either the error logger is not running or there
	** are no buffers available..  Allocate enough memory
	** for the event-specific data, and fill in anyway.. then
	** printf the information to the screen.
	*/
	logger_running = 0;
	elp = (struct el_rec *) kalloc ( el_size + EL_MISCSIZE );
	if ( elp == NULL )
	    printf ( "\nlamb_log_error: unable to alloc error buffer\n" );
	    return ( 0 );
    }
    else
	logger_running = 1;

    /*
    ** Fill in the 'subid' section of the errlog record
    */
    elp -> elsubid.subid_class = ELCT_ADPTR;
    elp -> elsubid.subid_ctldevtyp = ELMACH_DEC7000;
    elp -> elsubid.subid_type = ELADP_LAMB;
    /* elp -> elsubid.subid_errcode = sw_flags -> error_flags[0]; */
    elp -> elsubid.subid_errcode = EL_UNDEF;

    elp -> elsubid.subid_num = EL_UNDEF;
    elp -> elsubid.subid_unitnum = EL_UNDEF;

    /*
    ** Copy the sw_flags into the error record
    */
    lamb_subp_ptr = ( struct el_laser_lamb_data * ) &(elp -> el_body);
    bcopy ( sw_flags, &(lamb_subp_ptr -> soft_flags), 
				sizeof ( struct elr_soft_flags ) );

    xmidata = get_xmi(xmibus->bus_num);
    lva = (struct lambreg *) xmidata->cpu_xmi_addr;

    /*
    ** Copy the LAMB data and XMI subpackets into the 
    ** error record
    */
    lamb_subp_ptr -> lamb_chan = xmibus -> bus_num;
    lamb_subp_ptr -> lamb_valid = 0xFFFFFFFF; 

    lamb_subp_ptr -> lamb_xdev = RDCSR ( LONG_32, xmibus, &lva->xdev );
    lamb_subp_ptr -> lamb_xber = RDCSR ( LONG_32, xmibus, &lva->xber );
    lamb_subp_ptr -> lamb_xfadr = RDCSR ( LONG_32, xmibus, &lva->xfadr );
    lamb_subp_ptr -> lamb_xfaer = RDCSR ( LONG_32, xmibus, &lva->xfaer );
    lamb_subp_ptr -> lamb_ldiag = RDCSR ( LONG_32, xmibus, &lva->ldiag );
    lamb_subp_ptr -> lamb_imsk = RDCSR ( LONG_32, xmibus, &lva->imsk );
    lamb_subp_ptr -> lamb_levr = RDCSR ( LONG_32, xmibus, &lva->levr );
    lamb_subp_ptr -> lamb_lerr = RDCSR ( LONG_32, xmibus, &lva->lerr );
    lamb_subp_ptr -> lamb_lgpr = RDCSR ( LONG_32, xmibus, &lva->lgpr );
    lamb_subp_ptr -> lamb_ipr1 = RDCSR ( LONG_32, xmibus, &lva->ipr1 );
    lamb_subp_ptr -> lamb_ipr2 = RDCSR ( LONG_32, xmibus, &lva->ipr2 );
    lamb_subp_ptr -> lamb_iipr = RDCSR ( LONG_32, xmibus, &lva->iipr );


    if ( sw_flags -> error_flags[1] & SW_FLAG1_XMI_SUBPACKET_PRES )
    {
	nxv = xmidata->xmivirt;

	xmi_subp_ptr = ( struct elr_xmi_subpacket * )
	    ( ((char *) lamb_subp_ptr) + sizeof ( struct el_laser_lamb_data ) );

	i = 0;

	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) 
	{

            if(BADADDR( &nxv->xmi_dtype, sizeof(long), xmibus))
                        continue;

            if(xmidata->xminodes_alive & (1 << xminode)) 
	    {
		xmi_subp_ptr -> node_present |= 1 << xminode;

		xmi_subp_ptr -> xmi_registers[i++] = 
			RDCSR ( LONG_32, xmibus, &nxv->xmi_dtype );
		xmi_subp_ptr -> xdev_valid |= 1 << xminode;

		xmi_subp_ptr -> xmi_registers[i++] = 
			RDCSR ( LONG_32, xmibus, &nxv->xmi_xbe );
		xmi_subp_ptr -> xbe_valid |= 1 << xminode;

		xmi_subp_ptr -> xmi_registers[i++] = 
			RDCSR ( LONG_32, xmibus, &nxv->xmi_fadr );
		xmi_subp_ptr -> xfadr_valid |= 1 << xminode;

		xmi_subp_ptr -> xmi_registers[i++] = 
			RDCSR ( LONG_32, xmibus, &nxv->xmi_xfaer );
		xmi_subp_ptr -> xfaer_valid |= 1 << xminode;
            }
	}

	xmi_subp_ptr -> size = i * sizeof ( int );
	xmi_subp_ptr -> channel = xmibus -> bus_num;

    }

    /*
    ** Validate the error record
    */
    if ( logger_running )
	binlog_valid ( elp );

    /*
    ** Print the information to the console - if this wasn't due to a 
    ** CI reset
    */
    lamb_print_error_log ( elp );


    return ( 1 );
}


lamb_print_error_log ( struct el_rec* elp )

{
    u_int 			i = 0;
    u_int			xminode;

    struct el_laser_lamb_data*	lamb_subp_ptr;
    struct elr_xmi_subpacket*	xmi_subp_ptr;

    printf ( "\n\ndumping binary error log data:\n\n" );

    /*
    ** Copy the sw_flags into the error record
    */
    lamb_subp_ptr = ( struct el_laser_lamb_data * ) &(elp -> el_body);

    printf ( "\nS/W Flags 0 - 63 = %l016x", 
			lamb_subp_ptr -> soft_flags.error_flags[0] );
    printf ( "\nS/W Flags 64 - 127 = %l016x", 
			lamb_subp_ptr -> soft_flags.error_flags[1]);

    /*
    ** Copy the LAMB data and XMI subpackets into the 
    ** error record
    */
    printf ( "\n\n\nLAMB adapter packet contents:\n" );

    printf ( "\nchannel %d", lamb_subp_ptr -> lamb_chan );
    printf ( "\nvalid bits = %x", lamb_subp_ptr -> lamb_valid ); 
    printf ( "\nxdev = 0x%08x", lamb_subp_ptr -> lamb_xdev );
    printf ( "\nxber = 0x%08x", lamb_subp_ptr -> lamb_xber );
    printf ( "\nxfadr = 0x%08x", lamb_subp_ptr -> lamb_xfadr );
    printf ( "\nxfaer = 0x%08x", lamb_subp_ptr -> lamb_xfaer );
    printf ( "\nldiag = 0x%08x", lamb_subp_ptr -> lamb_ldiag );
    printf ( "\nlerr = 0x%08x", lamb_subp_ptr -> lamb_lerr );
    printf ( "\nimsk = 0x%08x", lamb_subp_ptr -> lamb_imsk );
    printf ( "\nlevr = 0x%08x", lamb_subp_ptr -> lamb_levr );
    printf ( "\nipr1 = 0x%08x", lamb_subp_ptr -> lamb_ipr1 );
    printf ( "\nipr2 = 0x%08x", lamb_subp_ptr -> lamb_ipr2 );
    printf ( "\niipr = 0x%08x", lamb_subp_ptr -> lamb_iipr );


    if ( lamb_subp_ptr -> soft_flags.error_flags[1] & 
		SW_FLAG1_XMI_SUBPACKET_PRES )
    {
	printf ( "\n\n\nXMI subpacket contents:\n" );

	xmi_subp_ptr = ( struct elr_xmi_subpacket * )
	    ( ((char *) lamb_subp_ptr) + sizeof ( struct el_laser_lamb_data ) );

        printf ( "\nxdev valid bits = %x", xmi_subp_ptr -> xdev_valid );
        printf ( "\nxbe valid bits = %x", xmi_subp_ptr -> xbe_valid );
        printf ( "\nxfadr valid bits = %x", xmi_subp_ptr -> xfadr_valid );
        printf ( "\nxfaer valid bits = %x", xmi_subp_ptr -> xfaer_valid );

        printf ( "\nnodes present = %x", xmi_subp_ptr -> node_present );

	i = 0;

	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++ ) 
	{
            if(xmi_subp_ptr -> node_present & (1 << xminode)) 
	    {
		printf ( "\n\nnode %d", xminode );

		printf ( "\nxdev = 0x%08x", xmi_subp_ptr->xmi_registers[i++] );
		printf ( "\nxbe = 0x%08x", xmi_subp_ptr->xmi_registers[i++] );
		printf ( "\nxfadr = 0x%08x", xmi_subp_ptr->xmi_registers[i++] );
		printf ( "\nxfaer = 0x%08x", xmi_subp_ptr->xmi_registers[i++] );
	    }

	}

    }

    printf ( "\n\n" );

    return ( 1 );

}


void
lamb_disable_errors ()
{
	lamb_errors = 0;
}

void
lamb_enable_errors ()
{
	lamb_errors = 1;
}


lamb_set_diag ( struct bus* xmibus )

{

    struct lambreg *lva;	/* virtual pointer to XMI node */
    struct xmidata *xmidata;


    xmidata = get_xmi(xmibus->bus_num);
    lva = (struct lambreg *)xmidata->cpu_xmi_addr;
/*	WRTCSR( LONG_32, xmibus, &lva->ldiag, 0x600000 );   */
    WRTCSR( LONG_32, xmibus, &lva->ldiag, 0x8000 );


    return ( 1 );

}
