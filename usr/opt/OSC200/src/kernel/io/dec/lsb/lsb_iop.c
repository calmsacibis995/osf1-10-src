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
static char *rcsid = "@(#)$RCSfile: lsb_iop.c,v $ $Revision: 1.2.15.2 $ (DEC) $Date: 1993/07/27 14:39:33 $";
#endif
 /*
 * Revision History: 
 *
 *	Nov-1991 jaa:	incorporate new csr interfaces and bug fixes
 *			from ruby boot
 *
 *	Sept-1991 jaa:	created this file for configuring the iop 
 *
 */

#include <sys/types.h>
#include <sys/time.h>

#include <vm/vm_kern.h>
#include <io/common/devdriver.h>
#include <io/dec/mbox/mbox.h>
#include <io/dec/lsb/lsbreg.h>
#include <io/dec/lsb/lsb_iopreg.h>
#include <io/dec/xmi/xlambreg.h>
#include <io/dec/xmi/xmireg.h>
#include <machine/machparam.h>
#include <machine/pmap.h>

#include <dec/binlog/errlog.h>


iop_set_diags ();
extern int hz;

int
iopconfl1(lsb, iop, lsbnode)
	struct bus *lsb, *iop;
	int lsbnode;
{
	struct lsbdata *lsbdata, *get_lsb();
	struct iopreg *iopreg;
	mbox_t mbp;
	vm_offset_t vecaddr;
	u_int cpumask;
	u_int csr;
	u_int ipchst;
	u_int s = splextreme();
	int nbus = 0;
	u_char hose;

	if(lsb == (struct bus *)0)
		panic("iopconfl1: no system bus");

	iop->bus_type = BUS_IOP;
	iop->alive |= ALV_ALIVE;
	conn_bus(lsb, iop);

	printf("%s%d at %s%d node %d\n", iop->bus_name, iop->bus_num, 
		lsb->bus_name, lsb->bus_num, lsbnode);

	iop->bus_node = (caddr_t)lsbnode;
	lsbdata = get_lsb(lsb->bus_num);
	iopreg = (struct iopreg *)lsbdata->lsbvirt[lsbnode];

	/* disable all IOP interrupts */
	iopreg->ipcnse = ~INTR_NSES;
	if(*iop->intr == NULL )
		panic("iopconfl1: no IOP adapter error vector");
	/* allocate an scb vector */
	if(allocvec(1, &vecaddr) != KERN_SUCCESS)
		panic("iopconfl1: allocvec");
	/* set up scb entry for the IOP */
        intrsetvec(vecoffset(vecaddr), *iop->intr, iop);
	/* now tell IOP its scb vector */
	iopreg->ipcvr = vecoffset(vecaddr);

	/*
	 * allocate and init a mailbox for the iop
	 * this will stay with the iop for the life of the system
	 * to be used in communicating with the remote adapters/busses 
	 */
	mbp = MBOX_ALLOC();
	mbp->bus_timeout = 10000;
	mbp->mbox_reg = (vm_offset_t)&iopreg->lmbpr;
	mbp->cmd = WHOAREYOU;
	mbp->mbox_cmd = null_mbox_cmd;
	iop->bus_mbox = (u_long *)mbp;

	/* get the status of the iop's hoses */
	ipchst = iopreg->ipchst;

	for(hose = 0; hose < nHOSES; hose++) {
		register struct bus *bus = (struct bus *)0;
		int dly;

		/* check for a hose with someone attached */
		if((ipchst & (IPCHST_HOSE_STAT << (hose * 4))) != 
		   ((IPCHST_CBLOK | IPCHST_PWROKC) << (hose * 4))) {
			/* 
			 * nobody home, reset the hose, and clear the power 
			 * transitioned and error bits 
			 */
			iopreg->ipchst = ((IPCHST_HOSE_RESET << hose) | 
					  ((IPCHST_PWROKT | IPCHST_ERROR) 
					   << (hose * 4)));
			continue;
		}

		mbp->hose = hose;
#ifdef MBOX_DEBUG	
		printf("iopconfl1: mbox before MBOX_GO\n");
		dumpmbox(mbp);
#endif /* MBOX_DEBUG */

		/* TODO: backoff delay */
		if(MBOX_GO(mbp) == 0) 
			panic("iopconfl1: mbox queue full\n");
		for(dly = 0; dly <= 200; ++dly) {
			mb();
			if(mbp->mb_status & MBOX_DON_BIT) {
				if(mbp->mb_status & MBOX_ERR_BIT) {
					printf("iopconfl1: resetting hose %d\n",
					       hose);
					csr = -1;
					mbp->mb_status = 1;
				} else
					csr = mbp->rdata;
				break;
			}
			DELAY(10000);
		}

		if((csr & RBUS_DEV_TYPE) == LAMB) {
			int xminum = (iop->bus_hd->bus_num << 12 | 
				      iop->bus_num << 8 | hose);
			if(lambconfl1(iop, xminum))
				++nbus;
		} else if((csr & RBUS_DEV_TYPE) == FLAG) {
			if(flagconfl1(iop, hose))
				++nbus;
		} else {
			printf("unknown adapter at %s%d hose %d ", 
			       iop->bus_name, iop->bus_num, hose);
			/* WHOAREYOU command timed out, reset the hose */
			iopreg->ipchst = ((IPCHST_HOSE_RESET << hose) | 
					  ((IPCHST_PWROKT | IPCHST_ERROR) 
					   << (hose * 4)));
		}

		/* clean up error bits */
		/* reset lber */
		iopreg->ipcnse = (INTR_NSES | MULT_INTR_ERR | 
				  DN_VRTX_ERR | UP_VRTX_ERR | IPC_IE | 
				  UP_HIC_IE | UP_HOSE_PAR_ERR | 
				  UP_HOSE_PKT_ERR | UP_HOSE_OFLO );

		iopreg->lber = (LSB_CTCE | LSB_DTCE | LSB_DIE | 
				LSB_SHE | LSB_CAE | LSB_NXAE | LSB_CNFE | 
				LSB_STE | LSB_TDE | LSB_CDPE2 | LSB_CDPE | 
				LSB_CPE2 | LSB_CPE | LSB_CE2 | LSB_CE | 
				LSB_UCE2 | LSB_UCE | LSB_E);
	}
	((mbox_t)iop->bus_mbox)->hose = (u_char)LOCAL_CSR;
	((mbox_t)iop->bus_mbox)->mbox_cmd = (void *)null_mbox_cmd;

	/*	
	 * Early Ruby protos have backplanes which are miss-wired and can
	 * return slot numbers greater then 3. This breaks the setup
	 * of the lcpumask, since it uses lsbintr_dst to identify the CPU
	 * to target interrupts to. Just jam lcpumask to interrupt all
	 * CPUs (this will work until MP...)
	*/
	if (mfpr_whami() > 7) {
		printf("iopconfl1: Illegal configuration - CPU in slot greater then 7\n");
		cpumask = 0xffff;
 	}
	else {
		cpumask = 0xf << ((ffs(lsbdata->lsbintr_dst) - 1) * 4);
	}
	iopreg->lcpumask = cpumask;

	/* enable interrupts and clear any pending err ints */
	iopreg->ipcnse = (INTR_NSES | MULT_INTR_ERR | DN_VRTX_ERR | 
			  UP_VRTX_ERR | IPC_IE | UP_HIC_IE | UP_HOSE_PAR_ERR |
			  UP_HOSE_PKT_ERR | UP_HOSE_OFLO );
	splx(s);

	return(nbus);
}

int
iopconfl2(iop, lsbdata)
	struct bus *iop;
	struct lsbdata *lsbdata;
{ 
	return(1);
}

ioperror(iop)
    struct bus *iop;
{
    struct lsbdata 		*lsbdata;
    struct iopreg 		*iopreg;
    u_int 			lber, ipcnse, ipchst;
    int 			hose;
    int 			bad_hose_msk = 0;
    extern int 			cold;

    struct elr_soft_flags	sw_flags;
    u_int			chan_subp_mask = 0;
	
    u_int			errors_found = 0;
    u_int			reset_flag = 0;
    u_int			delay;


    lsbdata = get_lsb(iop->connect_num);
    iopreg = (struct iopreg *)lsbdata->lsbvirt[(int)iop->bus_node];

    bzero ( &sw_flags, sizeof ( struct elr_soft_flags ) );
	
    /*
     * Read the laser bus error register, 
     * the iop node specific error register and the 
     * iop hose status register and log them.
     */

    lber = iopreg->lber;
    ipcnse = iopreg->ipcnse;
    ipchst = iopreg->ipchst;

    /*
    ** For now, always try and reset the IOP ( and thus the LAMBS )
    ** This flag was originally created in order to follow the
    ** PFMS document as to which errors indicate that a reset is 
    ** needed.  In most cases it seems that a reset is necessary
    ** in order to allow panic dumps to work.
    ** Resets can be made "per error" by removing this line of code
    ** and only setting the flag for the desired error.
    */
    reset_flag = 1;

    printf ( "\n\n" );

    if ( lber & LSB_NSES )
    {
	/*
	 * Up hose errors (parity, pkt, oflo) are not fatal, 
	 * even tho the packet is lost, the IOP can still function.
	 * However we can't relate the error to a device driver for 
	 * them to handle recovery, so we will panic in here.
	 * (we may be able to revover from these someday)
	 */
	for(hose = 0; hose < nHOSES; hose++) 
	{
		if(ipcnse & (1 << (hose + UP_HOSE_PAR_SHFT))) 
		{
		    printf("ioperror: up hose %d parity error\n", hose);

		    sw_flags.error_flags[0] |= 
		        SW_IOP_UPHOSE3PAR_ERROR << (MAX_HOSE - hose);
		    sw_flags.error_flags[1] |= SW_FLAG1_CHANNEL_SUBPACKET_PRES;

		    chan_subp_mask |= 1 << hose;

		    errors_found++;
		}

		if(ipcnse & (1 << (hose + UP_HOSE_PKT_SHFT))) 
		{
		    printf("ioperror: up hose %d packet error\n", hose);

		    sw_flags.error_flags[0] |= 
			SW_IOP_UPHOSE3PKT_ERROR << (MAX_HOSE - hose);
		    sw_flags.error_flags[1] |= SW_FLAG1_CHANNEL_SUBPACKET_PRES;

		    chan_subp_mask |= 1 << hose;

		    errors_found++;
		}
		if(ipcnse & (1 << (hose + UP_HOSE_OFLO_SHFT)))
		{
		    printf("ioperror: up hose %d overflow\n", hose);

		    sw_flags.error_flags[0] |= 
			SW_IOP_UPHOSE3OFLO_ERROR << (MAX_HOSE - hose);
		    sw_flags.error_flags[1] |= SW_FLAG1_CHANNEL_SUBPACKET_PRES;

		    chan_subp_mask |= 1 << hose;

		    errors_found++;
		}
	}

	/* FATAL ERRORS */

	/*
	 * IOP fatal errors.
	 *
	 * The IOP must be reset for these errors
	 * because they leave the IOP in an unpredictable state.
	 * So don't even bother trying to chase down other pending 
	 * errors on attached busses.
	 */
	if(ipcnse & MULT_INTR_ERR) 
	{
		printf ("ioperror: multi interrupt error\n");

		sw_flags.error_flags[0] |= SW_IOP_MULTINTR_ERROR;
		sw_flags.error_flags[1] |= 	
		    SW_FLAG1_CHANNEL_SUBPACKET_PRES | SW_FLAG1_BUGCHECK;

		chan_subp_mask = 0xF; /* all hoses */

		errors_found++;
		bad_hose_msk = 0xff; /* all hoses bad */
	}

	if(ipcnse & DN_VRTX_ERR) 
	{
		printf ("ioperror: down vortex error\n");

		sw_flags.error_flags[0] |= SW_IOP_DNVRTX_ERROR;

		errors_found++;
		bad_hose_msk = 0xff; /* all hoses bad */
	}

	if(ipcnse & UP_VRTX_ERR)
	{
		printf ("ioperror: up vortex error\n");

		sw_flags.error_flags[0] |= SW_IOP_UPVRTX_ERROR;
		sw_flags.error_flags[1] |= SW_FLAG1_BUGCHECK;

		errors_found++;
		bad_hose_msk = 0xff; /* all hoses bad */
	}

	if(ipcnse & IPC_IE)
	{
		printf ("ioperror: ipc internal error\n");

		sw_flags.error_flags[0] |= SW_IOP_IPCIE_ERROR;
		sw_flags.error_flags[1] |= SW_FLAG1_BUGCHECK;

		errors_found++;
		bad_hose_msk = 0xff; /* all hoses bad */
	}

	if(ipcnse & UP_HIC_IE)
	{
		printf ("ioperror: up hic internal error\n");

		sw_flags.error_flags[0] |= SW_IOP_UPHICIE_ERROR;
		sw_flags.error_flags[1] |= SW_FLAG1_BUGCHECK;

		errors_found++;
		bad_hose_msk = 0xff; /* all hoses bad */
	}
 
	/* 
	 * Fatal errors on either the hose or remote adapter.
	 *
	 * Log the error saving which hose(s) is(are) bad
	 * so we can chase down other pending errors on attached 
	 * busses later.
	 */
	for(hose = 0; hose < nHOSES; hose++) {

		if(ipchst & (IPCHST_PWROKT << (hose * 4))) {
			bad_hose_msk |= (1 << hose);

			errors_found++;

			/* 
			 * The hose power transitioned, 
			 * Hx_STAT<1> has more info.
			 * mark this hose as bad for later
			 */
			printf("ioperror: fatal error: hose %d power ", hose);
			if(ipchst & (IPCHST_PWROKC << (hose * 4))) 
			{
				/*** future hot swap ***/
				printf("up\n");

				sw_flags.error_flags[0] |= 
				    SW_IOP_HOSE_PWRUP_3 << 
						(MAX_HOSE - hose);

			}
			else
			{
				printf("loss\n");
				sw_flags.error_flags[0] |= 
				    SW_IOP_HOSE_PWRFAIL_3 << 
						(MAX_HOSE - hose);
			}
		}

		if(ipchst & (IPCHST_ERROR << (hose * 4))) {
			/* 
			 * something bad happened on the remote adapter
			 * mark this hose as bad for later
			 */
			bad_hose_msk |= (1 << hose);
			printf("ioperror: hose %d: fatal adapter error\n",hose);
			sw_flags.error_flags[0] |= 
			    SW_IOP_HOSE3_ERROR << (MAX_HOSE - hose);
			sw_flags.error_flags[1] |= 
				SW_FLAG1_CHANNEL_SUBPACKET_PRES;

		        chan_subp_mask |= 1 << hose;
			errors_found++;
		}
	}

	/*
	** Did we find any error bits set?
	** If not, log an "inconsistent" error
	*/
	if ( errors_found == 0 )
	    sw_flags.error_flags[0] |= SW_IOP_INCON;

	/*
	** Log Binary Error Record
	*/
	iop_log_error ( iop, &sw_flags, chan_subp_mask, bad_hose_msk );

	/*
	** Check for pending errors on the attached LAMBS
	*/
	log_att_bus_err(iop, bad_hose_msk); 

	/*
	** We need to reset the IOP ( which results in a reset of the 
	** entire IO subsystem ) to give the console a shot at creating 
	** the dump file
	*/
	iop_reset ( iop );

	/*
	** Crash the system
	*/
	panic ( "ioperror");

    }
    else
    {
	/*
	 * Starting with the 5.5 NS LEP's we started to get
	 * occational interrupts during config. Just ignore them...
	*/
	if (cold) 
	{
		printf("\n\nioperror: dismissing a benign iop interrupt\n");
		return(0);
	}
	else
	{
		/*
		** Log an "inconsistent" error
		*/
		sw_flags.error_flags[0] |= SW_IOP_INCON;

		/*
		** Log Binary Error Record
		*/
		iop_log_error ( iop, &sw_flags, chan_subp_mask );
	}
    }

    return ( 1 );
}

/*
 * find all busses attached to the iop and call there error routines
 * to log any errors they may have pending, skipping those on bad hoses.
 */
log_att_bus_err(iop, bad_hose_msk)
	struct bus *iop;
	u_int bad_hose_msk;
{
	struct bus *rbus;
	/*
	 * Chase down any errors that might be pending on all attached 
	 * busses except if its on a hose that's bad.
	 */
	for(rbus = iop->bus_list; rbus; rbus = rbus->nxt_bus) {
		/* skip the bus if its on a bad hose */
		if(bad_hose_msk & (1 << ((mbox_t)rbus->bus_mbox)->hose))
			continue;
		/* call the remote bus error rtn */
		switch(rbus->bus_type) {
		      case BUS_XMI:
		  	/* get to the xmi via lamb */
			log_lamberror(rbus);
			break;
		      default:
			printf("\n\nioperror: unknown bus type %d on hose %d\n",
			       rbus->bus_type, ((mbox_t)rbus->bus_mbox)->hose);
			break;
		}
	}
}



iop_log_error ( struct bus* iop,
		struct elr_soft_flags* sw_flags, 
		u_int	chan_subp_mask,
		u_int	bad_hose_msk )

{

    struct el_rec*			elp;
    vm_offset_t				el_data_ptr;

    u_int				el_size;

    struct el_laser_iop_data*		iop_data;
    struct elr_iop_adp_subp*		iop_subp_ptr;
    struct elr_xmi_channel_subp*	xmi_chan_subp_ptr;

    struct elr_chan_subp_hdr {
	u_int	revision;
	u_short flags;
	u_short chan_count;
    };

    struct elr_chan_subp_hdr*		chan_subp_hdr_ptr;
    struct elr_chan_subp_hdr		chan_subp_hdr;

    struct bus* 			rbus;
    struct bus*				busses[nHOSES];

    struct lambreg*			lva;

    u_int				i, logger_running;

    struct lsbdata *lsbdata;
    struct iopreg *iopreg;


    lsbdata = get_lsb(iop->connect_num);
    iopreg = (struct iopreg *)lsbdata->lsbvirt[(int)iop->bus_node];

    /*
    ** Calculate the errlog buffer size that's required
    */
    el_size = sizeof ( struct el_laser_iop_data );

    /* Any more subpackets? */
    if ( sw_flags -> error_flags[1] & SW_FLAG1_CHANNEL_SUBPACKET_PRES )
	el_size += sizeof ( struct elr_chan_subp_hdr );

    chan_subp_hdr.chan_count = 0;
    chan_subp_hdr.flags = 0;

    for ( i = 0; i < nHOSES; i++ )
	if ( chan_subp_mask & ( 1 << i ) )
	{
	    el_size += sizeof ( struct elr_xmi_channel_subp );
	    chan_subp_hdr.chan_count++;
	}

    /*
    ** Figure out which hoses have lambs and which have flags
    */
    for(rbus = iop->bus_list; rbus; rbus = rbus->nxt_bus) {

	/*
	** Set the appropriate flags bit in the channel subpacker header,
	** as per the PFMS..
	**
	** Also, create an array of bus structures indexed by hose number
	** so that later on we can access the LAMB's registers ( via mailboxes)
	** for each hose which requires a channel subpacket.
	**
	** This counts on the assuumption that the xmi bus numbers
	** correspond with the hose number!
	*/
	switch(rbus->bus_type) {
	      case BUS_XMI:
	
		chan_subp_hdr.flags |= 1 << rbus->bus_num;
		busses[rbus->bus_num] = rbus;

		break;

	      case BUS_FBUS:

		chan_subp_hdr.flags |= 1 << ( rbus->bus_num + 4 );
		busses[rbus->bus_num] = rbus;

		break;
	}
    }

    /*
    ** Allocate an errlog buffer
    */
    elp = (struct el_rec *) binlog_alloc ( el_size, EL_PRISEVERE );
    if ( elp == (struct el_rec *) BINLOG_NOBUFAVAIL )
    {
	/*
	** Either the error logger is not running or there
	** are no buffers available..  Allocate enough memory
	** or the event-specific data, and fill in anyway.. then
	** printf the information to the screen.
	*/
	logger_running = 0;
	elp = (struct el_rec *) kalloc ( el_size + EL_MISCSIZE );
	if ( elp == NULL )
	    printf ( "\niop_log_error: unable to allocate error buffer\n" );
	    return ( 0 );
    }
    else
	logger_running = 1;

    /*
    ** Fill in the 'subid' section of the errlog record
    */
    elp -> elsubid.subid_class = ELCT_ADPTR;
    elp -> elsubid.subid_ctldevtyp = ELMACH_DEC7000;
    elp -> elsubid.subid_type = ELADP_IOP;
    /* elp -> elsubid.subid_errcode = sw_flags -> error_flags[0]; */
    elp -> elsubid.subid_errcode = EL_UNDEF;

    elp -> elsubid.subid_num = EL_UNDEF;
    elp -> elsubid.subid_unitnum = EL_UNDEF;


    iop_data = (struct el_laser_iop_data *) &(elp->el_body);

    /*
    ** Copy the software flags into the error record
    */
    bcopy ( sw_flags, &(iop_data ->soft_flags), 
			sizeof ( struct elr_soft_flags ) );

    /*
    ** Copy the IOP Subpacket and Channel subpacket(s) into the 
    ** error record
    */
    iop_subp_ptr = ( struct elr_iop_adp_subp * ) &(iop_data -> iop_adp_subp);

    iop_subp_ptr -> iop_addr = (long) iopreg;
    iop_subp_ptr -> valid = 0xFFFFFFFF; 

    iop_subp_ptr -> iop_ldev = iopreg -> ldev;
    iop_subp_ptr -> iop_lber = iopreg -> lber;
    iop_subp_ptr -> iop_lcnr = iopreg -> lcnr;
    iop_subp_ptr -> iop_lbesr0 = iopreg -> lbesr0;
    iop_subp_ptr -> iop_lbesr1 = iopreg -> lbesr1;
    iop_subp_ptr -> iop_lbesr2 = iopreg -> lbesr2;
    iop_subp_ptr -> iop_lbesr3 = iopreg -> lbesr3;
    iop_subp_ptr -> iop_lbecr0 = iopreg -> lbecr0;
    iop_subp_ptr -> iop_lbecr1 = iopreg -> lbecr1;
    iop_subp_ptr -> iop_ipcnse = iopreg -> ipcnse;
    iop_subp_ptr -> iop_ipcvr = iopreg -> ipcvr;
    iop_subp_ptr -> iop_ipcmsr = iopreg -> ipcmsr;
    iop_subp_ptr -> iop_ipchst = iopreg -> ipchst;


    if ( sw_flags -> error_flags[1] & SW_FLAG1_CHANNEL_SUBPACKET_PRES )
    {
	chan_subp_hdr_ptr = ( struct elr_chan_subp_hdr * )
		( ((char *) iop_data) + sizeof ( struct el_laser_iop_data ) );
	chan_subp_hdr_ptr -> revision = 1;
	chan_subp_hdr_ptr -> flags = chan_subp_hdr.flags;
	chan_subp_hdr_ptr -> chan_count = chan_subp_hdr.chan_count;    

	/*
	** Any channel subpackets?
	*/
	xmi_chan_subp_ptr = ( struct elr_xmi_channel_subp * ) 
	    ((char *) chan_subp_hdr_ptr) + sizeof ( struct elr_chan_subp_hdr );

	/*
	** Read all of the lamb/flag registers for each hose indicated
	** by the chan_subp_mask
	*/
	lva = (struct lambreg *) XMI_BASE_PHYS;

	for ( i = 0; i < nHOSES; i++ )
	    if ( chan_subp_mask & ( 1 << i ) )
	    {

		xmi_chan_subp_ptr -> channel = i;
		xmi_chan_subp_ptr -> valid = 0xFFFFFFFF;

		xmi_chan_subp_ptr -> xdev = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> xdev );

		/*
		** if this hose is 'bad', don't try to 
		** read all of the registers, we'll just panic
		** in rdcsr anyway
		*/
		if ( bad_hose_msk & (1 << i) )
		    continue;

		xmi_chan_subp_ptr -> xber = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> xber );
		xmi_chan_subp_ptr -> xfadr = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> xfadr );
		xmi_chan_subp_ptr -> xfaer = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> xfaer );
		xmi_chan_subp_ptr -> ldiag = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> ldiag );
		xmi_chan_subp_ptr -> imsk = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> imsk );
		xmi_chan_subp_ptr -> levr  = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> levr );
		xmi_chan_subp_ptr -> lerr = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> lerr );
		xmi_chan_subp_ptr -> lgpr = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> lgpr );
		xmi_chan_subp_ptr -> ipr1 = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> ipr1 );
		xmi_chan_subp_ptr -> ipr2 = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> ipr2 );
		xmi_chan_subp_ptr -> iipr = RDCSR ( 	LONG_32, 
							busses[i], 
							&lva -> iipr );

		xmi_chan_subp_ptr++;
	    }

    } /* if channel subpackets */

    /*
    ** Validate the error record
    */
    if ( logger_running )
	binlog_valid ( elp );

    /*
    ** Print the record on the console until dumps after I/O subsystem
    ** errors works
    */
    iop_print_error_log ( elp );


    return ( 1 );

}

iop_print_error_log ( struct el_rec* elp )

{
    u_int 				i = 0;

    struct elr_soft_flags*		sw_flags;

    struct el_laser_iop_data*		iop_data;
    struct elr_iop_adp_subp*		iop_subp_ptr;
    struct elr_xmi_channel_subp*	xmi_chan_subp_ptr;

    struct elr_chan_subp_hdr {
	u_int	revision;
	u_short flags;
	u_short chan_count;
    };

    struct elr_chan_subp_hdr*		chan_subp_hdr_ptr;

    printf ( "\n\ndumping binary error log record\n" );

    iop_data = (struct el_laser_iop_data *) &( elp -> el_body );
    sw_flags = ( struct elr_soft_flags * ) &( iop_data -> soft_flags );

    printf ( "\nS/W flags 0-63   = %l016x", sw_flags -> error_flags[0] );
    printf ( "\nS/W flags 64-127 = %l016x", sw_flags -> error_flags[1] );

    
    iop_subp_ptr = ( struct elr_iop_adp_subp * ) &( iop_data -> iop_adp_subp );

    printf ( "\n\nIOP adapter subpacket contents:\n" );

    printf ( "\nIOP Address : %x", iop_subp_ptr -> iop_addr );
    printf ( "\nvalid bits = %x", iop_subp_ptr -> valid ); 
    printf ( "\nldev = 0x%08x", iop_subp_ptr -> iop_ldev );
    printf ( "\nlber = 0x%08x", iop_subp_ptr -> iop_lber );
    printf ( "\nlcnr = 0x%08x", iop_subp_ptr -> iop_lcnr );
    printf ( "\nlbesr0 = 0x%08x", iop_subp_ptr -> iop_lbesr0 );
    printf ( "\nlbesr1 = 0x%08x", iop_subp_ptr -> iop_lbesr1 );
    printf ( "\nlbesr2 = 0x%08x", iop_subp_ptr -> iop_lbesr2 );
    printf ( "\nlbesr3 = 0x%08x", iop_subp_ptr -> iop_lbesr3 );
    printf ( "\nlbecr0 = 0x%08x", iop_subp_ptr -> iop_lbecr0 );
    printf ( "\nlbecr1 = 0x%08x", iop_subp_ptr -> iop_lbecr1 );
    printf ( "\nipcnse = 0x%08x", iop_subp_ptr -> iop_ipcnse );
    printf ( "\nipcvr = 0x%08x", iop_subp_ptr -> iop_ipcvr );
    printf ( "\nipcmsr = 0x%08x", iop_subp_ptr -> iop_ipcmsr );
    printf ( "\nipchst = 0x%08x", iop_subp_ptr -> iop_ipchst );


    if ( sw_flags -> error_flags[1] & SW_FLAG1_CHANNEL_SUBPACKET_PRES )
    {

	printf ( "\n\nXMI channel subpacket contents:\n" );

	chan_subp_hdr_ptr = ( struct elr_chan_subp_hdr * )
		( ((char *) iop_data) + sizeof ( struct el_laser_iop_data ) );

	printf ( "\nchannel subpacket header contents:" );
	printf ( "\nrevision = %x", chan_subp_hdr_ptr -> revision );
	printf ( "\nflags = %x", chan_subp_hdr_ptr -> flags );
	printf ( "\nchannel count = %d", chan_subp_hdr_ptr -> chan_count );    

	/*
	** Any channel subpackets?
	*/
	xmi_chan_subp_ptr = ( struct elr_xmi_channel_subp * ) 
	    ((char *) chan_subp_hdr_ptr) + sizeof ( struct elr_chan_subp_hdr );

	/*
	**
	*/
	for ( i = 0; i < chan_subp_hdr_ptr -> chan_count; i++ )
	{
		printf ( "\n" );

		printf ( "\nchannel %d", xmi_chan_subp_ptr -> channel );
		printf ( "\nvalid bits = %x", xmi_chan_subp_ptr -> valid );

		printf ( "\nxdev = 0x%08x", xmi_chan_subp_ptr -> xdev );
		printf ( "\nxber = 0x%08x", xmi_chan_subp_ptr -> xber );
		printf ( "\nxfadr = 0x%08x", xmi_chan_subp_ptr -> xfadr );
		printf ( "\nxfaer = 0x%08x", xmi_chan_subp_ptr -> xfaer );
		printf ( "\nldiag = 0x%08x", xmi_chan_subp_ptr -> ldiag );
		printf ( "\nimsk = 0x%08x", xmi_chan_subp_ptr -> imsk );
		printf ( "\nlevr = 0x%08x", xmi_chan_subp_ptr -> levr );
		printf ( "\nlerr = 0x%08x", xmi_chan_subp_ptr -> lerr );
		printf ( "\nlgpr = 0x%08x", xmi_chan_subp_ptr -> lgpr );
		printf ( "\nipr1 = 0x%08x", xmi_chan_subp_ptr -> ipr1 );
		printf ( "\nipr2 = 0x%08x", xmi_chan_subp_ptr -> ipr2 );
		printf ( "\niipr = 0x%08x", xmi_chan_subp_ptr -> iipr );

		xmi_chan_subp_ptr++;

	}
    }

    printf ( "\n\n" );

    return ( 1 );

}


/*
** iop_reset
**
** This routine attempts to clear all error bits and reset the IOP
** ( and thus all hoses ).  This might not be necessary, except to 
** help the console to be able to open the dump device ( we're panic'ing
** right after this..
*/
iop_reset ( struct bus* iop )

{

	u_int	delay;
	u_int	hose;

	struct lsbdata 		*lsbdata;
	struct iopreg 		*iopreg;
	u_int 			ipchst;


	/*
	** All of this resetting doens't appear to help anything
	** yet.  We're panicing, and despite all of this effort to
	** reset, the console memory dump fails.  When this code
	** can be fixed, or it's determined that it's even
	** necessary, we can re-engage this routine.  For now..
	*/
	return ( 1 );

	lsbdata = get_lsb(iop->connect_num);
	iopreg = (struct iopreg *)lsbdata->lsbvirt[(int)iop->bus_node];


	/* enable interrupts and clear any pending err ints */
	iopreg->ipcnse = (INTR_NSES | MULT_INTR_ERR | DN_VRTX_ERR | 
			  UP_VRTX_ERR | IPC_IE | UP_HIC_IE | UP_HOSE_PAR_ERR |
			  UP_HOSE_PKT_ERR | UP_HOSE_OFLO );

	/* clear any ipchst errors */
	iopreg->ipchst = ( (IPCHST_ERROR | IPCHST_PWROKT) |
			  ((IPCHST_ERROR | IPCHST_PWROKT) << 4) |
			  ((IPCHST_ERROR | IPCHST_PWROKT) << 8) |
			  ((IPCHST_ERROR | IPCHST_PWROKT) << 12) ) ;

	/*
	** First, reset the LAMBs using the ipchst register.
	** Write the register, then wait 4ms to allow the LAMB to
	** reset, then go ahead and reset the IOP.  This is necessary
	** because the LAMB takes too long to reset and get its error
	** lines clear.  Otherwise we could simply write lcnr<nrst>
	** and that would reset everything.
	**
	** We just go ahead and reset all LAMBS because resetting the IOP 
	** will reset them all anyway.
	*/
	for ( hose = 0; hose < nHOSES; hose++ )
	{
	    /* 
	    ** Set the hose reset bit for this hose.
	    */
	    ipchst |= IPCHST_HOSE_RESET << hose;
	}

	/* Write the reset bits */	
	iopreg -> ipchst = ipchst;

	/*
	** Hang on a bit
	*/
	DELAY ( 5000 );  /* the PFMS asks for 4 msec */

	/*
	** Reset the IOP
	*/
	iopreg -> lcnr |= LSB_NRST;

	/*
	** Don't Poll the lcnr<stf> bit for completion of the reset
	** It seems to get cleared and never set.  This is because
	** the IOP has no built in self-test, the self test is done
	** at power-up only by the cpu
	*/
	printf ( "\n\niop_reset: Resetting the I/O Subsystem" );
	DELAY ( 15000000 );


	/*
	** How to determine that the IOP is done resetting?
	*/

	return ( 1 );

}


/*
** diag routine used to force errors through the diag register
*/
iop_set_diags ( struct bus *iop )

{

    struct lsbdata *lsbdata;
    struct iopreg *iopreg;

    lsbdata = get_lsb(iop->connect_num);
    iopreg = (struct iopreg *)lsbdata->lsbvirt[(int)iop->bus_node];

    iopreg -> ipcdr = 4; 

    return ( 1 );
}
