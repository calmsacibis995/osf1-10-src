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
static char *rcsid = "@(#)$RCSfile: scsi_adu.c,v $ $Revision: 1.2.2.9 $ (DEC) $Date: 1992/08/31 12:21:52 $";
#endif

/*
 * Modification History: /sys/io/scsi/alpha/scsi_adu.c
 *
 * 15-Oct-91 -- Farrell Woods
 *	Changed to use new device structure
 *	beat up to run under OSF/MACH:
 *	u_short szsdt => caddr_t
 *	
 *
 * 11-Mar-91 -- rjl
 *	Added support for unaligned and non-contigous data transfers
 *
 * 30-Nov-90 -- Tim Burke
 *	Created this file for Alpha adu scsi support.
 */

/*
 * Notes:
 *
 * - I found out halfway through development of this driver that there is no
 *   command queuing supported in our present SCSI drivers.  This means that
 *   there can be only one command outstanding to an individual target at any
 *   time.  This simplifies a lot of things and may render the waitq and 
 *   command tagging useless.  Live and learn!
 */

#include <data/scsi_data.c>
#include <sys/proc.h>

/*
 * These defines are just here for debug purposes to get desired printf
 * messages.  These should all be removed later.
 */
#define ADUDEBUG	/* tim fix - remove later */
#undef  ADU_LOGERR	/* tim fix - don't use error logger yet, use printf */
/*
 * tim fix - usage of SCSI_POLLED assembler directive:
 * Define this symbol and the driver will not operate in interrupt driven
 * mode.  This should be deleted soon.
 */
#ifdef 0
#define SCSI_POLLED	/* tim fix - remove later */
#endif

extern int adu_vector_alloc();
extern int adu_scsi_start();
extern int adu_scsi_reset();
extern int adu_scsi_intr();
extern struct adu_scsi_wait *adu_scsi_getqent();
extern struct tv_config *tv_cnfg_base;	/* TV node configuration pointer */

/*
 * This is an array of pointers to the scsi related IO space registers.
 * The array is here initialized to the offsets from the base address (BB).
 * Prior to actual usage of these registers the base address (BB) will be
 * added to the intialized offset to result in a pointer to the actual IO
 * space register itself.
 */
struct adu_scsiregs adu_scsi_regs = {
	(u_long *)ADU_SCSI_BASE,	
	(u_long *)ADU_SCSI_ICR,
	(u_long *)ADU_SCSI_DB,
};
/*
 * Need a few items for autoconfig of devices on "ibus"
 */
/*extern  u_short szstd[]; */
extern caddr_t szstd[];

extern int     szslave(), szattach(), sz_start(), szerror();
int adu_scsi_probe();
char *scsi_map_user();

extern struct device *szdinfo[];

struct	driver aduszdriver =
	{ adu_scsi_probe,		/* ud_probe		*/
	  szslave, 			/* ud_slave		*/
	  0,
	  szattach,			/* ud_attach		*/
	  sz_start,			/* ud_dgo		*/
	  szstd,			/* ud_addr		*/
	  "rz",				/* ud_dname		*/
	  szdinfo,			/* ud_dinfo		*/
	  "adusz",			/* ud_mname		*/
	  szminfo,			/* ud_minfo		*/
	  0 };				/* ud_xclu		*/

/*
 * Global variables used for command ring and message ring manipulation.
 */
/* These should be in the softc to be on a per-bus basis */
struct scsi_ring *scsi_ringptr;		/* Pointer to the actual rings */
long	 adu_scsi_intr_reg = 0;		/* Used to set and clear int reg */
struct adu_scsi_wait *scsi_waitq_base;  /* Base addr of queue alloc entries */
struct 	aduqueue scsi_waitq;		/* Queue of cmds waiting on ring entry*/
struct 	aduqueue scsi_freeq;		/* Empty waitq structs		      */

#ifdef SCSI_POLLED
int adu_scsi_forcepoll = 0;
#endif /* SCSI_POLLED */

int adu_wait_count = 10000;  
extern struct scsi_devtab szp_rz_udt;
extern struct scsi_devtab szp_tz_udt;
extern struct scsi_devtab szp_cz_udt;
extern int szp_nrz;
extern int szp_ntz;
extern int szp_ncz;
extern int szp_nrx;
extern int rzcomplete();
#if 0
extern int tzcomplete();
#endif
extern int wakeup();
extern int hz;

/* ADU scsi is single-threaded so we can get away with this
 */
#ifdef MACH
char alignment_buffer[SCSI_MAXPHYS + 32];
#endif

/******************************************************************
 *
 * Name:	adu_scsi_probe
 *
 * Abstract:	Probe for any SCSI devices on this controller.
 *		Allocate and initialize command and message rings.
 *		Register SCB vectors for this IO module.
 *		Initialize any fields in the softc structure.  For
 *		each possible target:
 *	    	    o	build SCSI Inquiry command
 *		    o	call adu_scsi_start()
 *		    o	if command successful,
 *			    initialize target-specific data structures
 *			    to indicate a usable target.
 *
 *		Clear any pending interrupts for this controller.
 *
 * Inputs:
 *
 *	io_base_addr	- Base virtual address for this IO module.
 *	um		- uba controller struct pointer.
 *
 * Outputs:	None.
 *
 * Return values: None.
 *
 ******************************************************************/

adu_scsi_probe(io_base_addr, um)
	register char *io_base_addr;
	struct controller *um;
{
	int cntlr = um->ctlr_num;
	int targid;
	struct sz_inq_dt *idp;
	struct scsi_devtab *sdp;
	struct scsi_devtab *usdp;
	int retries, status;
	int dev_wait;
	int i, s, stat;
	int alive;
	char *p;
	register struct sz_softc *sc = &sz_softc[cntlr];
	int sdp_match;
	int vector_return;

        PRINTD(targid, SCSID_ADU_PROBE,
		("adu_scsi_probe: begin probe of SCSI bus.\n"));
	
	s = splbio();

	alive = 1; 		/* On ADU the SCSI port is always present */

	/*
	 * Initialize certain fields in the softc structure.
	 * Allocate and initialize the ring structures.
	 */

	if (alive) {
	    /*
	     * Paranoia checks:
	     */
	    if (sizeof(struct send_command) != 128) {
		printf("adu_scsi_probe: ERROR: size of send_command is %d.\n",
			sizeof(struct send_command));
			alive = 0;
	    }
	    if (sizeof(struct command_complete) != 128) {
		printf("adu_scsi_probe:ERROR: size of command_complete is %d\n",
			sizeof(struct command_complete));
			alive = 0;
	    }
	    /*
	     * In order to force 32 byte allignment of the sz_datfmt structs
	     * it is necessary that the size of the struct be an even multiple
	     * of 32 because they are kmalloc'ed contiguously.
	     */
	    if ((sizeof(struct sz_datfmt)) % 32) {
		printf("adu_scsi_probe:ERROR: size of sz_datfmt is %d\n",
			sizeof(struct sz_datfmt));
			alive = 0;
	    }
	    sc->sc_siinum = cntlr;		/* save HBA number */

	    /*
	     * The scsi register pointers are already initialized to the 
	     * offset from the base register.  Now add the base address of the
	     * IO module's TVBus registers to these offsets to get pointers to 
	     * the registers themselves.
	     * 
	     * Jump through hoops with the casting!
 	     */
	    adu_scsi_regs.base = (u_long *) ((u_long)io_base_addr +
					     (u_long)adu_scsi_regs.base);
	    adu_scsi_regs.icr = (u_long *) ((u_long)io_base_addr +
					     (u_long)adu_scsi_regs.icr);
	    adu_scsi_regs.db = (u_long *) ((u_long)io_base_addr +
					     (u_long)adu_scsi_regs.db);
	    /*
	     * Allocate and initialize the command and message rings.
	     */
	    if (adu_init_rings(sc) != SZ_SUCCESS) {
		printf("adu_scsi_probe: ring initialization failure.\n");
		alive = 0;
	    }
	    /*
	     * Allocate the needed sz_datfmt structures.
	     */
	    if (adu_alloc_datfmt(sc) != SZ_SUCCESS) {
		printf("adu_scsi_probe: sz_datfmt allocation failure.\n");
		alive = 0;
	    }
    	    /* The configuration data structure supplies system software with 
     	     * the SCSI id that is used by the I/O module itself.  There is an
	     * array of configuration data structs one for each node on the 
	     * TVbus.  Based on the TVbus node find the specified ID on the
	     * SCSI bus of the host.
     	     */
/*
    	    sc->sc_sysid = 
		tv_cnfg_base[um->bus_num].tv_config_spec.tv_io_config.tv_scsiid;
	    if ((sc->sc_sysid < 0) || (sc->sc_sysid >= NDPS)) {
		printf("adu_scsi_probe: ERROR, tv_config host scsi node %d.\n",
			sc->sc_sysid);
		printf("adu_scsi_probe: using host scsi node 7.\n");
		sc->sc_sysid = 7;
	    }
sc->sc_sysid = 7;
*/
	    sc->sc_sysid = old_get_scsiid(um->bus_num);
	    /*
	     * Allocate SCB vector to enable interrupt handling.  The params
	     * are (interrupt ipl, interrupt routine, int routine parameter).
	     * Use the returned value to setup the IRQNODE and IRQCHAN fields
	     * of the icr register.
	     *
	     * Since there are only 2 SCB vectors at SPL21 and 20 at SPL20,
	     * request that the ipl be 20 instead of 21.  Thats why this has a 
	     * hardcoded value of 20 instead of SPLBIO which equals 21.
	     */
	    vector_return = adu_vector_alloc(20, adu_scsi_intr, cntlr);
	    if (vector_return == 0) {
		printf("adu_scsi_probe: call to adu_vector_alloc failed.\n");
		alive = 0;
	    }
	    else {
	    	adu_scsi_intr_reg = ((ADU_GETIRQNODE(vector_return) << 1) |
				      (ADU_GETIRQCHAN(vector_return) << 5));
	    }

	    sc->sc_active = 0;			/* init the active flag */
	    sc->port_start = adu_scsi_start;	/* init the port_start switch */
	    sc->port_reset = adu_scsi_reset;	/* init the port_reset switch */

	    /*
	     * Disable any SCSI interrupts because probe uses polled mode.
	     */
	    DISABLE_SCSI_INTERRUPT(ADU_SCSI_IE);

#ifdef SCSI_POLLED
	    printf("adu_scsi_probe: Operate SCSI driver in polled mode.\n");
#endif /* SCSI_POLLED */
	}	/* end if (alive) */

	/*
	 * Use the inquiry command to determine number
	 * and type of targets on this controller.
	 * If this controller does not exist alive will
	 * be zero and the for loop won't do anything.
	 */

	for (targid = 0; targid < NDPS; targid++) { 

		if (alive == 0) {
			/*
			 * This should not be necessary, but due to previous
			 * bugs is seems necessary.  This could be removed,
			 * but seems harmless.
			 */
			sc->align_buf_inuse[targid] = 0;
			break;
		}

		sc->sc_active = (1 << targid);		/* signal active */
		sc->sc_selstat[targid] = SZ_IDLE;
		sc->sc_attached[targid] = 0;		
		sc->sc_no_disconnects[targid] = 0;
		sc->sc_rzspecial[targid] = 0;
		sc->align_buf_inuse[targid] = 0;
		sc->align_buf_alloc[targid] = 0;
		sc->sc_rmv_media &= ~(1 << targid);
		sc->sc_segcnt[targid] = SCSI_ADU_MAXXFER;

		if (targid == sc->sc_sysid)
			continue;	/* skip initiator */

/* need to do here instead of on the fly 'cause the call to pmap_dup in
 * scsi.c requires the resources
 */
#ifdef MACH
                /*
                 * Get the mapping resources we need to map the
                 * unaligned buffer.
                 */
                sc->sc_SZ_bufmap[targid] = (char *)vm_alloc_kva(66*NBPG);
                if (sc->sc_SZ_bufmap[targid] == NULL) {
                    printf("scsi_map_user: scsi %d targetID %d: %s\n",
                        0, targid, "cannot get PTEs for bufmap");
                    break;
                }
#endif

		retries = 2;
		status = 1;
		dev_wait = sz_wait_for_devices;
        	PRINTD(targid, SCSID_ADU_PROBE,
		("----------------- TARGET %d -----------------------\n",targid));
		while (retries)
		{
		    sc->sc_szflags[targid] = SZ_NORMAL; 
		    sc->sc_curcmd[targid] = SZ_INQ;

		    sz_bldpkt(sc, targid, SZ_INQ, 0, 0);

		    stat = adu_scsi_start(sc, targid, 0);

		    if (stat == SZ_SUCCESS) {
			status = 0;
			break;
		    }			/* end if stat == SUCCESS */
		    else  {
			if (stat == SZ_RET_ABORT) {
			    PRINTD(targid, SCSID_ERRORS,
			       ("adu_probe: stat == RET_ABORT\n"));
			    status = 1;
			    break;
			}
		    }  /* end else stat == RET_ABORT */

		    DELAY(1000);	
		    retries--;
		    continue;

		}	/* end while */

		if (status != 0) {
        	    PRINTD(targid, SCSID_ADU_PROBE,
		        ("adu_scsi_probe: Giving up on targid %d\n",targid));
		    continue;
		}

		/*
		 * The simulator puts out a message when a scsi device has
		 * come online.  Aparently this string is not newline
		 * terminated.
		 */
		printf("\n");

        	PRINTD(targid, SCSID_ADU_PROBE,
		    ("adu_scsi_probe: Good status on targid %d\n",targid));

		/*
		 * Initialize data structures for this target and
		 * save all pertinent inquiry data (device type, etc.).
		 */

		idp = (struct sz_inq_dt *)&sc->sz_dat[targid];

		adu_print_inq_info(idp, targid);	

		/* Save removable media bit for each target */

		if (idp->rmb)
		    sc->sc_rmv_media |= (1 << targid);

		PRINTD(targid, SCSID_CMD_EXP,
		   ("\n",adu_print_inq_info(idp, targid)));

		/*
		 * Zero device name and revsion level
		 * ASCII strings, so we know whether or
		 * not they were loaded by the INQUIRY.
		 */

		for (i = 0; i < SZ_DNSIZE; i++)
		    sc->sc_devnam[targid][i] = (char)0;

		for (i = 0; i < SZ_REV_LEN; i++)
		    sc->sc_revlvl[targid][i] = (char)0;
		/*
		 * Save the device name and revision level.
		 * DEC combines vendor & product ID strings.
		 */
		p = &sc->sc_devnam[targid][0];

		for (i=0; i<SZ_VID_LEN; i++)
		    *p++ = idp->vndrid[i];
		for (i=0; i<SZ_PID_LEN; i++)
		    *p++ = idp->prodid[i];
		p = &sc->sc_revlvl[targid][0];
		for (i=0; i<SZ_REV_LEN; i++)
		    *p++ = idp->revlvl[i];

		PRINTD(targid, SCSID_FLOW,
		  ("adu_probe: targid %d perfdt=0x%x\n", targid, idp->perfdt));

	    switch(idp->perfdt) {
	    default:		/* Unknown device type */
		printf("adu_probe: scsi %d targetID %d: %s (%d).\n",
		       cntlr, targid, "unknown peripheral device type",
		       idp->perfdt);
		sc->sc_alive[targid] = 0;
		sc->sc_devtyp[targid] = SZ_UNKNOWN;
		sc->sc_xstate[targid] = SZ_NEXT;
		sc->sc_xevent[targid] = SZ_BEGIN;
		bcopy(DEV_UNKNOWN, sc->sc_device[targid],
			strlen(DEV_UNKNOWN));
		break;
	    case 0:		/* Direct-access device (disk) */
	    case 1:		/* Sequential-access device (tape) */
	    case 5:		/* Read-only direct-access device (CDROM) */
        		PRINTD(targid, SCSID_ADU_PROBE,
			    ("aduprobe: valid idp->perfdt %d\n",idp->perfdt));
		sc->sc_alive[targid] = 1;
		sc->sc_szflags[targid] = SZ_NORMAL;	/* house keeping */
		sc->sc_xstate[targid] = SZ_NEXT;
		sc->sc_xevent[targid] = SZ_BEGIN;
		/*
		 * Find this device in the scsi_devtab in scsi_data.c.
		 * The answer could come back as unknown or missing.
		 */
		usdp = (struct scsi_devtab *)0;
		sdp_match = 0;
		for (sdp=scsi_devtab; sdp->namelen; sdp++) {
		    if ((idp->perfdt == 0) && ((sdp->devtype&SZ_DISK) == 0))
			continue;
		    if ((idp->perfdt == 1) && ((sdp->devtype&SZ_TAPE) == 0))
			continue;
		    if ((idp->perfdt == 5) && ((sdp->devtype&SZ_CDROM) == 0))
			continue;
		    /* Save address of unknown device entry, if it exists. */
		    if ((sdp->name) && (strcmp("UNKNOWN", sdp->name) == 0))
			usdp = (struct scsi_devtab *)sdp;

		    /* HACK: DEC tapes don't use vendor/product ID fields. */
		    if ((sdp->devtype & SZ_TAPE) &&
			(idp->perfdt == 1) &&
			(sc->sc_devnam[targid][0] == 0) &&
			(idp->devtq == sdp->tapetype)) {
        			PRINTD(targid, SCSID_ADU_PROBE,
				    ("aduprobe: valid SZ_TAPE.\n"));
				sdp_match++;
				break;
		    }
		    if (sdp->name) {
			if (strncmp(sc->sc_devnam[targid], sdp->name,
						    sdp->namelen) == 0) {
        			PRINTD(targid, SCSID_ADU_PROBE,
				    ("aduprobe: valid sc_devnam.\n"));
				sdp_match++;
				break;

			    }	/* end if strncmp(devnam, sdp->name) */

		    }	/* end if (sdp->name) */

		}	/* end for */

		/*
		 * If the device name did not match call it RZxx or TZxx.
		 * Use the UNKNOWN entry from scsi_devtab (if it exists),
		 * otherwise use our internal UNKNOWN entry.
		 */

		if (!sdp_match) {
        	    PRINTD(targid, SCSID_ADU_PROBE,
		        ("aduprobe: no match on sc_devnam .\n"));
		    if (usdp)
			sdp = usdp;
		    else if (idp->perfdt == 0)
			sdp = &szp_rz_udt;
		    else if (idp->perfdt == 1)
			sdp = &szp_tz_udt;
		    else
			sdp = &szp_cz_udt;

		}	/* end if !match */

		/*
		 * Update counters and set the pointer to the completion 
		 * handler.
		 */
		if (sdp->devtype & SZ_DISK){
			szp_nrz++;
			sc->device_comp[targid] = rzcomplete;
		}
#if 0
		if (sdp->devtype & SZ_TAPE){
			szp_ntz++;
			sc->device_comp[targid] = tzcomplete;
		}
#endif
		if (sdp->devtype & SZ_CDROM){
			szp_ncz++;
			sc->device_comp[targid] = rzcomplete;
		}
		if ((sdp->devtype == RX23) || (sdp->devtype == RX33)) {
			szp_nrx++;
			sc->sc_mc_cnt[targid] = 1;
			sc->device_comp[targid] = rzcomplete;
		}

		/* TODO: assumes length < 8 bytes */
		bcopy(sdp->sysname, sc->sc_device[targid],
			strlen(sdp->sysname));
		sc->sc_devtab[targid] = sdp;
		sc->sc_devtyp[targid] = sdp->devtype;
		sc->sc_dstp[targid] = sdp->disksize;

		/*
		 * Act on the flags in device's scsi_devtab entry.
		 */

		if (sdp->flags & SCSI_REQSNS) {
		    sc->sc_curcmd[targid] = SZ_RQSNS;
		    sz_bldpkt(sc, targid, SZ_RQSNS, 0, 1);
		    adu_scsi_start(sc, targid, 0);
		}
		if (sdp->flags & SCSI_STARTUNIT) {
		    /*
		     * Send two start unit commands because a pending unit
		     * attention may cause the first one to fail. We don't
		     * for the drive to spin up here (happens in rzopen).
		     */
		    sc->sc_curcmd[targid] = SZ_P_SSUNIT;
		    sz_bldpkt(sc, targid, SZ_P_SSUNIT, 0, 1);
		    adu_scsi_start(sc, targid, 0);
		    adu_scsi_start(sc, targid, 0);
		}
		if (sdp->flags & SCSI_TESTUNITREADY) {
		    sc->sc_curcmd[targid] = SZ_TUR;
		    sz_bldpkt(sc, targid, SZ_TUR, 0, 1);
		    adu_scsi_start(sc, targid, 0);
		}
		if (sdp->flags & SCSI_READCAPACITY) {
		    sc->sc_curcmd[targid] = SZ_RDCAP;
		    sz_bldpkt(sc, targid, SZ_RDCAP, 0, 1);
		    adu_scsi_start(sc, targid, 0);
		}
		if (sdp->probedelay > 0) {
		    DELAY(sdp->probedelay);
		}

		if (sdp->flags & SCSI_NODIAG)
		    sz_unit_rcvdiag[targid] = 1;

		break;
	    }		/* end of switch */
	}		/* end of for loop */

	DELAY(10000);

	sc->sc_active = 0;			/* clear active */

	splx(s);

#ifdef SCSI_POLLED
	adu_scsi_forcepoll = 1;
#endif /* SCSI_POLLED */
	return(alive);

}	/* end adu_probe */

/******************************************************************
 *
 * Name:	adu_scsi_reset
 *
 * abstract:	On a normal system this would reset the SCSI bus.  Since
 *		the SCSI bus is under the controller of the DS5000 this routine
 *		does nothing and simply returns.
 *
 * inputs:
 *
 * 	sc	- the softc data structure
 *
 * outputs:		none.
 *
 * return values: none.
 *
 ******************************************************************/

adu_scsi_reset(sc)

register struct sz_softc *sc;
{
}	/* end adu_scsi_reset */


/******************************************************************
 *
 * Name:	adu_scsi_start
 *
 * Abstract:	Start a SCSI operation on the IO controller.
 *
 *		o	Call adu_post_command to place the command into the
 *			command ring for transmission on the SCSI bus.
 *		o	If not polled mode, return SZ_IP to upper state machine.
 *		o	If polled mode, spin wait for interrupt, then
 *			call adu_scsi_intr() to process.
 *
 * Inputs:
 *
 * 	sc	- a pointer to the controller data structure
 *	targid  - the target id
 *	bp	- the buf pointer for this operation
 *	
 *
 * Outputs:		None.
 *
 * Return values: 	Essentialy 2 forms of operation: polled and non-polled.
 *
 *			Polled mode: Waits for command completion and returns
 *				     final completion status as follows:
 *			SZ_SUCCESS	- command completed successfully
 *			SZ_RET_ERR	- command completed with an error
 *			SZ_RET_ABORT	- the cmd was aborted due to an error
 *					  or a timeout waiting for the polled
 *					  response.
 *
 *			Non-polled mode: Simply posts the command to the
 *					 command ring and then returns the
 *					 in progress status.
 *			SZ_IP		- command in progress
 *
 ******************************************************************/

adu_scsi_start(sc, targid, bp)

	register struct sz_softc *sc;
	register int targid;
	register struct buf *bp;
{
    int cntlr = sc->sc_siinum;
    int retval;
    int timer;
    int phase;
    u_char stat;
    int	s;
    register int ring_index;

    /*
     * If "bp" is "0" we use polled scsi mode, disallow reselect
     * attempts, and disable interrupts for all transfers. We 
     * poll for completion instead of allowing interrupts.
     */

    if (bp == (struct buf *)0) {
	sc->scsi_polled_mode = 1;
    }
    else {
        sc->scsi_polled_mode = 0;
    }

    /* Start a SCSI command on the passed target. */

    sc->scsi_completed[targid] = 0;

    /* Place the command into the command ring */

    /* 
     * Call adu_post_command to arange to have this command sent out onto
     * the SCSI bus by placing it into the command ring.  If the command ring
     * is full the command will be chained on a list of pending commands.
     */
    if ((retval = adu_post_command(sc, targid)) != SZ_SUCCESS) {

#ifdef ADU_LOGERR
	/* if bp == 0, this is probe(), and it is not an error to timeout
	   then... */

	if (bp != 0) {	 
	    scsi_logerr(sc, 0, targid, SZ_ET_SELTIMO, 0, 0, SZ_HARDERR);
	}
#else /* ADU_LOGERR */
	printf("adu_scsi_start: bad return adu_post_command...0x%x, targid=%d\n", retval, targid);
#endif /* ADU_LOGERR */

	/*
	 * The return value would be SZ_BUSBUSY if there is presently an
	 * unaligned data transfer being conducted on this target.  In this
	 * case all further commands will be stalled in the state table by
	 * returning busy here.
	 */
	if (retval == SZ_BUSBUSY) {
		return(SZ_BUSBUSY);
	}
	return(SZ_RET_ABORT);

    }	/* end if adu_post_command != SUCCESS */

    /* If in poll mode spin waiting for a send command complete message. */


#ifdef SCSI_POLLED
    if ((sc->scsi_polled_mode) || (adu_scsi_forcepoll)) {
#else /* SCSI_POLLED */
    if (sc->scsi_polled_mode) {
#endif /* SCSI_POLLED */

	PRINTD(targid, SCSID_PHASE_STATE,
	    ("adu_scsi_start: COMMAND IN PROGRESS poll mode\n"));

	/* spin until the message ring is not empty */
	SZWAIT_UNTIL(((ring_index = get_next_msg_ring(sc)) != -1),
		adu_wait_count,retval);

	if (retval >= adu_wait_count) {
	    int flags;
#ifdef ADUDEBUG
	    {
	    int i;
	    char *p;

	    cprintf("adu_scsi_start: Timedout ADU status: cntlr=%x, targid=%x\n",
			sc->sc_siinum,targid);

	    p = (char*)&sc->sz_opcode;
	    cprintf("cdb: ");
	    for (i = 0; i < GET_SCSI_CMD_LEN(sc->sz_opcode); i++) {
		cprintf(" %x", *p++);
	    }
	    cprintf("\n");
	    }
#endif /* ADUDEBUG */

#ifdef ADU_LOGERR
	    scsi_logerr(sc, 0, targid, SZ_ET_BUSERR, 0x45, 0, flags);
#else /* ADU_LOGERR */
	    printf("adu_scsi_start: response timeout in polled mode.\n");
#endif /* ADU_LOGERR */
	    goto HANDLE_ABORT;

	}	/* end if retval > adu_wait_count */
	else {
	    if (ring_index == -1) {		/* paranoia check */
		printf("adu_scsi_start: invalid ring index %d\n",ring_index);
	    }
	}

	adu_scsi_intr(cntlr);		/* let the intr handle clean up */

	PRINTD(targid, SCSID_ENTER_EXIT,
	       ("adu_scsi_start: flags after interrupt 0x%x\n",
		sc->sc_szflags[targid]));

	/*
	 * Check the status of the SCSI operation. If the SCSI
	 * operation completed with a good status return success, otherwise
	 * indicate a problem.  
	 */

	if (sc->sc_status[targid] == SZ_GOOD) {
	    return(SZ_SUCCESS);	
	}
	else {
	    return(SZ_RET_ERR);	
	}

    }	/* end while scsi_polled_mode */ 

    /*
     * For the non-polled mode case return that the command is in progress to
     * allow the thread to sleep waiting for an interupt upon command
     * completion.
     */
    return( SZ_IP );


HANDLE_ABORT:

    /* Abort the current SCSI operation due to error */

    printf("adu_scsi_start: command aborted (bus=adu%d target=%d cmd=0x%x)\n",
	    cntlr, targid, sc->sc_curcmd[targid]);

    /*
     * tim fix - 
     * If a command does timeout some action may be necessary to make sure that
     * when the response comes back later it does not flag an error as a 
     * response to an inactive target.
     */
    sc->sc_selstat[targid] = SZ_IDLE;
    sc->sc_active = 0;

    return(SZ_RET_ABORT);

}	/* end adu_scsi_start */

/******************************************************************
 *
 * Name:	adu_init_rings
 *
 * Abstract:	Allocate the SCSI rings.  The command and message rings
 *		fit on an 8k page.  The first 4096 bytes are for the command
 *		ring; the second 4096 bytes are for the message ring.  The
 *		command ring and the message ring each contain 32 entries of
 *		length 128 bytes.
 *
 *		After the space has been allocated the page-aligned physical
 *		address of the rings is written to the SCSI base register.
 *		The interrupt control register is written with the value of
 *	        the TVbus node number of the CPU.  Defer ringing of the 
 *		doorbell register until there a command to post.
 *		Initialize the status of each entry to empty.  This is actually
 *		not necessary because the flag field should be 0 anyways.
 *
 * Inputs:
 *      sc      - the softc data structure
 *
 * Outputs:		None.
 *
 * Return values: SZ_SUCCESS on success.
 *		  SZ_FATAL   on error.
 *
 ******************************************************************/
adu_init_rings(sc)
	register struct sz_softc *sc;
{
    register int i;

#ifdef MACH
    scsi_ringptr = (struct scsi_ring *)kmem_alloc(kernel_map, 8192);
#else
    KM_ALLOC(scsi_ringptr, struct scsi_ring *, 8192, KM_DEVBUF, 
		KM_NOW_CL_CO_CA | KM_NOCACHE );
#endif
    if (scsi_ringptr == (struct scsi_ring *)NULL) {
	printf("adu_init_rings: Failed to kmalloc scsi rings.\n");
	return(SZ_FATAL);
    }
    for (i = 0; i < ADU_RING_ENTRIES; i++) {
	scsi_ringptr->command_ring[i].flag = SCSI_EMPTY;
	scsi_ringptr->message_ring[i].flag = SCSI_EMPTY;   /* tresspassing ? */
	/* Set command tag to a bogus value to ease debug */
	scsi_ringptr->message_ring[i].cmdtag = 0xffff;
    }
    /*
     * These index pointers point to the next active entry in the ring.
     * Initialize to 0 to indicate that the first ring entry is currently
     * the active entry.
     */
    sc->sc_ring_cmd_index = 0;
    sc->sc_ring_msg_index = 0;
    /*
     * Setup the waitq which is used to store command requests when the actual
     * command ring is full.
     */
    if (adu_init_queues(sc) != SZ_SUCCESS) {
	printf("adu_init_rings: Failed to initialize adu scsi waitq.\n");
	return(SZ_FATAL);
    }
#ifdef MACH
    if (pmap_svatophys(scsi_ringptr, adu_scsi_regs.base) == KERN_INVALID_ADDRESS) {
	printf("can't get kva for scsi_ringptr\n");
	return (SZ_FATAL);
    }
#else
    *adu_scsi_regs.base = svtophy(scsi_ringptr);
#endif
    mb();
    return(SZ_SUCCESS);
}

/******************************************************************
 *
 * Name:	adu_alloc_datfmt
 *
 * Abstract:	The softc structure includes a structure of type sz_datfmt
 *		for each target on the bus.  This structure is used as a buffer
 *		to store information DMA'ed in from the scsi bus for non-read
 *	    	and non-write commands.  For example the inquiry command will
 *		have the inquiry data put in this buffer.
 *
 *		A restriction on this buffer is that it be 32-byte alligned.
 *		For this reason the structures are gang allocated from kmalloc
 *		to start on a known hex-alligned boundary.  In order for this
 *		scheme to work the size of the sz_datfmt structure must be a
 *		multiple of 32 bytes.
 *
 * Inputs:
 *      sc      - the softc data structure
 *
 * Outputs:		None.
 *
 * Return values: SZ_SUCCESS on success.
 *		  SZ_FATAL   on error.
 *
 ******************************************************************/
adu_alloc_datfmt(sc)
	register struct sz_softc *sc;
{
#ifdef MACH
    sc->sz_dat = (struct sz_datfmt *)kmem_alloc(kernel_map, NDPS * sizeof(struct  sz_datfmt));
#else
    KM_ALLOC(sc->sz_dat, struct sz_datfmt *, (NDPS * sizeof(struct  sz_datfmt)),
		KM_DEVBUF, KM_NOW_CL_CO_CA | KM_NOCACHE );
#endif
    if (sc->sz_dat == (struct sz_datfmt *)NULL) {
	printf("adu_alloc_datfmt: Failed to kmalloc scsi sz_datfmt structs.\n");
	return(SZ_FATAL);
    }
    return(SZ_SUCCESS);
}

/******************************************************************
 *
 * Name:	adu_init_queues
 *
 * Abstract:	Allocates the waitq entries, initializes them and assigns them
 *		all to the freeq.  
 *
 *		These queues are used to store command requests when the actual
 *		command rings are full.
 *
 *		There really is no need for a free queue.  Each time
 *		an entry is needed it could be kmalloc'ed and then freed when
 *		no longer needed.  This way there would be no need to guess
 *		what the maximum queue depth should be and statically consume
 *		that many entries.
 *
 * Inputs:
 *      sc      - the softc data structure
 *
 * Outputs:		None.
 *
 * Return values: SZ_SUCCESS on success.
 *		  SZ_FATAL   on error.
 *
 ******************************************************************/
adu_init_queues(sc)
	register struct sz_softc *sc;
{
    register struct adu_scsi_wait *entry;
    register int i;

#ifdef MACH
    scsi_waitq_base = (struct adu_scsi_wait *)kmem_alloc(kernel_map, (sizeof(struct adu_scsi_wait)) * ADU_SCSI_NUMWAITS);
#else
    KM_ALLOC(scsi_waitq_base, struct adu_scsi_wait *, 
	((sizeof(struct adu_scsi_wait)) * ADU_SCSI_NUMWAITS), KM_DEVBUF, 
		KM_NOW_CL_CA);
#endif
    if (scsi_waitq_base == (struct adu_scsi_wait *)NULL) {
	printf("adu_init_queues: Failed to kmalloc scsi waitq elements.\n");
	return(SZ_FATAL);
    }
    /*
     * Setup queues to be initially empty by having both flink and blink
     * pointing to the address of the flink itself.
     */
    scsi_waitq.flink = (struct adu_scsi_wait *)&scsi_waitq;
    scsi_waitq.blink = (struct adu_scsi_wait *)&scsi_waitq;
    scsi_freeq.flink = (struct adu_scsi_wait *)&scsi_freeq;
    scsi_freeq.blink = (struct adu_scsi_wait *)&scsi_freeq;
    /*
     * Insert all the entries onto the free queue.
     */
    entry = scsi_waitq_base;
    for (i = 0; i < ADU_SCSI_NUMWAITS; i++) {
	Insert_entry(entry, scsi_freeq);
	entry++;
    }
    return(SZ_SUCCESS);
}	/* end of routine adu_init_queues */

/******************************************************************
 *
 * Name:	adu_scsi_getqent
 *
 * Abstract:	Retruns a pointer to an entry that has been removed from the
 *		head of the queue.
 *
 * Inputs:
 *      scsi_queue      - Pointer to the queue from which an entry is to be
 *			  removed.
 *
 * Outputs:		None.
 *
 * Return values: queue entry pointer on success.
 *		  0   on empty queue.
 *
 ******************************************************************/
struct adu_scsi_wait *
adu_scsi_getqent( scsi_queue )
	register struct aduqueue *scsi_queue;
{
	register struct adu_scsi_wait *retval;

	if ((scsi_queue->flink == (struct adu_scsi_wait *)scsi_queue) &&
	    (scsi_queue->blink == (struct adu_scsi_wait *)scsi_queue)) {
		return((struct adu_scsi_wait *) 0);	/* Empty queue */
	}
	retval = scsi_queue->flink;
	Remove_entry(scsi_queue->flink);
	return(retval);
}       /* end of routine adu_scsi_getqent */

/******************************************************************
 *
 * Name:	adu_scsi_queue_empty
 *
 * Abstract:	Used to determine if a queue is empty.
 *
 * Inputs:
 *      scsi_queue      - Pointer to the queue being examined.
 *
 * Outputs:		None.
 *
 * Return values: 
 *		  1   on empty queue.
 *		  0   on non-empty queue.
 *
 ******************************************************************/
adu_scsi_queue_empty( scsi_queue )
	register struct aduqueue *scsi_queue;
{
	if ((scsi_queue->flink == (struct adu_scsi_wait *)scsi_queue) &&
	    (scsi_queue->blink == (struct adu_scsi_wait *)scsi_queue)) {
		return(1);	/* Empty queue */
	}
	return(0);
}

/******************************************************************
 *
 * Name:	adu_post_command
 *
 * Abstract:	Cause a command to be sent out on the SCSI bus.  This 
 *		consists of copying the command into an empty entry in the
 *		command ring and changing the entry status from empty to
 *		active.  Finally the doorbell register must be rung to notify
 *		the IO controller that there has been a change in the ring
 *		status.
 *
 *		If the command ring is full then chain up the command
 *		request for later transmission.
 *
 *
 * Inputs:
 *
 * 	sc	- the softc data structure
 *  targid	- the target id
 *
 * Outputs:		None.
 *
 * Return values: Status of command posting.
 *		SZ_SUCCESS	Command posted to command ring or placed
 *				in a list of command requests.
 *		SZ_FATAL	Command ring is full in polled mode.  Or both
 *				command ring and waitq are full in non-polled
 *				mode.  Or the address of the dma buffer does
 *				not meet necessary requirements.
 *		SZ_BUSBUSY	There is presently an unaligned data transfer
 *				command in progress.  Stall everything until
 *				that command completes by returning busy status.
 *
 ******************************************************************/

adu_post_command(sc, targid)

register struct sz_softc *sc;
register int targid;
{
    int cntlr = sc->sc_siinum;
    int retval, i, s;
    u_long messg;
    int	messg_len;
    int	cmdcnt = 0;
    int lun = sc->sz_t_read.lun;
    char *cmdptr, *cmdout, cmdstr[20];
    int	ring_index;
    int	datai_sink = 0;
    register int bytecount = 0;
    int	alignment_needed = 0;
    register struct send_command *scsi_cmd_ptr, scsi_cmd;
    register struct adu_scsi_wait *entry;
    struct mode_sel_sns_params *msp;
    struct reassign_params *rp;
    struct format_params *fp;
    register u_char *buf_addr = 0;
    struct buf *bp = 0;

    /*
     * Do not accept any further commands until the unaligned data transfer
     * operation has completed.
     */
    if (sc->align_buf_inuse[targid]) {
	return(SZ_BUSBUSY);
    }
    cmdout = &cmdstr[0];
    bzero(&scsi_cmd,sizeof(struct send_command));

    sc->sc_active |= (1 << targid);	

#ifdef SZDEBUG
    /*
     * According to the ADU spec at least the following commands are supported:
     * This is actually only here for debugging purposes.
     */
    switch(sc->sz_opcode) {
	case SZ_INQ:			/* INQUIRY		*/
	case SZ_TUR:			/* TEST_UNIT_READY	*/
	case SZ_RQSNS:			/* REQUEST_SENSE	*/
	case SZ_SSLU:			/* START_UNIT		*/
	case SZ_MODSEL:			/* MODE_SELECT		*/
	case SZ_MODSNS:			/* MODE_SENSE		*/
	case SZ_RDCAP:			/* READ_CAPACITY	*/
	case SZ_READ:			/* READ			*/
	case SZ_WRITE:			/* WRITE		*/
	case SZ_REASSIGN:		/* REASSIGN_BLOCKS	*/
		break;
	default:
		 PRINTD(targid, SCSID_CMD_EXP, ("adu_post_command: Command 0x%x may be unsupported on the adu.\n",sc->sc_actcmd[targid]));
		break;
    }
#endif /* SZDEBUG */
    PRINTD(targid, SCSID_ADU_POSTCMD,
      ("adu_post_command: command = 0x%x, targid = %d\n",sc->sz_opcode,targid));
    cmdcnt = GET_SCSI_CMD_LEN(sc->sz_opcode);
    bcopy((char*)&(sc->sz_command), &cmdstr[0], cmdcnt);
    /*
     * When in polled mode you do not select with attention; rather you select
     * without attention and directly enter command mode.  The implication 
     * here is that the NMSG field will be set to 0.
     */
    if (sc->scsi_polled_mode) {
	messg_len = 0; 				/* No message sent      */
    }	/* end if polled_mode || no_disc */
    else {
    	/*
     	 * When not in polled mode you select with attention; This means
	 * that in the selection phase you send a message indicating that the
	 * scsi HBA can disconnect from the bus.  For this case set the
	 * NMSG field to 1 and the message is identify.  Not sure if lun is
	 * needed there.  This setup allows disconnects so the identify with
	 * disconnects message is used.
	 */
	messg = SZ_ID_DIS | lun;  	/* Allow disconnects */
	messg_len = 1;
    }	/* end else ! polled mode */

    PRINTD(targid, SCSID_ADU_POSTCMD,
       ("adu_post_command: cmdcnt = 0x%x, CDB: ", cmdcnt));

    SZDEBUG_EXPAND(targid, &cmdstr[0], cmdcnt);

    PRINTD(targid, SCSID_CMD_EXP, ("\n"));

    /*
     * Format an entry to go into the scsi command ring.  If there is an
     * empty slot in the command ring this command will be bcopy'ed into the
     * command ring.  Otherwise the command will be queued onto the wait
     * queue.
     */
    scsi_cmd_ptr = &scsi_cmd;
    bcopy(&cmdstr[0], (char *) &(scsi_cmd.cmd[0]), cmdcnt);
    scsi_cmd_ptr->ncmd = cmdcnt;
    /*
     * The bus field contains the bus portion of the physical SCSI address.
     * The ADU has only 1 SCSI bus so this field should be set to 0.
     */
    scsi_cmd_ptr->bus = 0;
    scsi_cmd_ptr->target = targid;

    scsi_cmd_ptr->nmsg = messg_len;
    if (messg_len) {
	/*
	 * Nonzero message length means that a message is sent during the 
	 * device selection phase rather than proceedeing directly into
	 * the command phase.  This message usually indicates that disconnects
	 * are possible (and the logical unit number also?).
	 */
	scsi_cmd_ptr->msg = messg;
	/*
	 * Disable the automatic sensing when a check condition occurs.
	 * Generally the driver would probably be faster if this is not set.
	 * The reason to set this field is just to ease the porting effort.
	 * The miserable scsi state machine would have to be changed all over
	 * the place not to issue its own request sense upon error.
	 */
	scsi_cmd_ptr->options = SCSI_NOSENSE;
    }
    else {
	/*
         * Here I am assuming that if messg_len is 0 then
	 * we are in polled mode from probe.  Here we do not want to enable
	 * automatic bus retries because we are probing for devices and 
	 * expect some selection failures.
	 */
	scsi_cmd_ptr->options = SCSI_NORETRY;
    }
    /*
     * The dataaddr field is setup to be the physical address of the buffer that
     * is used as a source or sink during the DATAI or DATAO phases.  The buffer
     * will differ depending on the command and some commands will have no
     * buffer at all.  Note also that since transfers on the TVBus are in 32
     * byte "chunks" the size of the data buffer must be a multiple of 32.
     *
     * In many of these cases there is a test of sc_rzspecial.  When this
     * variable is set it is assumed that the rzdisk utility is posting
     * this request.  Here the returned data is not placed somewhere in the
     * softc but rather into an alternate buffer.  Apparently this buffer then
     * gets returned back to rzdisk which knows what the format of the buffer
     * is and prints it out or whatever.  In this case the address in sc_rzaddr
     * better be 32-byte alligned or some fixup will be required.
     */
    switch(sc->sz_opcode) {
	case SZ_READ:			/* READ			*/
		datai_sink = 1;
					/* FALLTHROUGH		*/
	case SZ_WRITE:			/* WRITE		*/
		/*
		 * The buffer address to DMA from was setup in the state 
		 * machine.
		 */
		buf_addr = (u_char *)sc->sc_bufp[targid];
		bytecount = sc->sz_d_read.xferlen * 512;
		bp = sc->sc_bp[targid];
		break;
	case SZ_TUR:			/* TEST_UNIT_READY	*/
	case SZ_SSLU:			/* START_UNIT		*/
			/* No data buffer needed */
		buf_addr = (u_char *)0;
		bytecount = 0;
		break;
	case SZ_INQ:			/* INQUIRY		*/
		datai_sink = 1;
		bytecount = SZ_INQ_MAXLEN;
		if (sc->sc_rzspecial[targid]) {
		    buf_addr = (u_char *)sc->sc_rzaddr[targid];
		}
		else {
		    buf_addr = (u_char *)&sc->sz_dat[targid];
		}
		break;
	case SZ_RQSNS:			/* REQUEST_SENSE	*/
		datai_sink = 1;
		bytecount = SZ_RQSNS_LEN;
		if (sc->sc_rzspecial[targid]) {
		    buf_addr = (u_char *)sc->sc_rzaddr[targid];
		}
		else {
		    buf_addr = (u_char *)&sc->sc_sns[targid];
		}
		break;
	case SZ_RDCAP:			/* READ_CAPACITY	*/
		datai_sink = 1;
		buf_addr = (u_char *)&sc->sz_dat[targid];
		bytecount = SZ_RDCAP_LEN;
		break;
	case SZ_MODSNS:			/* MODE_SENSE		*/
		datai_sink = 1;
		if (sc->sc_rzspecial[targid]) {
		    buf_addr = (u_char *)sc->sc_rzaddr[targid];
		    msp = (struct mode_sel_sns_params *)sc->sc_rzparams[targid];
		    bytecount = msp->msp_length;
		}
		else {
		    buf_addr = (u_char *)&sc->sz_dat[targid];
		    bytecount = (int) sc->sz_modsns.alclen;
		}
		break;
	case SZ_REASSIGN:		/* REASSIGN_BLOCKS	*/
		datai_sink = 1;
		buf_addr = (u_char *)&sc->sc_rzparams[targid];
                rp = (struct reassign_params *)sc->sc_rzparams[targid];
		bytecount = ((rp->rp_header.defect_len0 << 0) & 0x00ff) +
			    ((rp->rp_header.defect_len1 << 8) & 0xff00) + 4;

		break;
	case SZ_MODSEL:			/* MODE_SELECT		*/
		if (sc->sc_rzspecial[targid]) {
		    msp = (struct mode_sel_sns_params *)sc->sc_rzparams[targid];
		    buf_addr = (u_char *)sc->sc_rzaddr[targid];
		    bytecount = msp->msp_length;
		}
		else {
		    buf_addr = (u_char *)&sc->sz_dat[0];
		    bytecount = (int) sc->sz_modsel.pll;
		}
		break;
    }
    /*
     * The data address (buffer) must be physically contiguous and alligned on
     * a 32-byte boundary.  For read type operations (datai_sink will be set)
     * insure that the buffer size can accomodate transfers which are a multiple
     * in size of 32 bytes (TVbus restriction).  When the read size is not an
     * even multiple of 32 bytes it is necessary to allocate a new buffer so 
     * that the reaminder bytes (bytecount % 32) do not end up writing into
     * an unexpected address range (buffer overflow).
     * Also limit the maximum data transfer size so that we don't try to 
     * kmalloc an unreasonable amount of memory.
     */
    if (bytecount) {
	if (bytecount > SCSI_ADU_MAXXFER)
	    return(SZ_FATAL);

	/*
	 * Is the transfer a multiple of  32 bytes
	 */
	if ((datai_sink) & (bytecount % 32))
	    alignment_needed = 1;

	/*
	 * Is the address on a 32 byte boundary
	 */
        if ( (long)buf_addr & (32-1) )
	        alignment_needed = 1;

	/*
	 * If it's not physically contiguous we have to copy it as well.
	 */
	if ( !scsi_dmabuf_contig(buf_addr, bytecount, bp) )
		alignment_needed = 1;
    }
    /*
     * Raise IPL when allocating the alignment buffer or posting a command to
     * the commmand ring
     */
    s = splbio();
    if (alignment_needed) {
	/*
	 * The buffer address for a data transfer operation does not meet the
	 * alignment criteria.  To fixup this situation allocate a properly
	 * aligned buffer of the correct length.  Can't sleep waiting for an
	 * alignment buffer because this could be entered at interrupt
	 * context.  Use this aligned buffer to perform the data
	 * transfer.  For write commands bcopy the data now.  For reads
	 * the data will be bcopy'ed back into the unaligned buffer later.
	 */
	if (sc->align_buf_inuse[targid]) {	/* paranoia check */
    	    splx(s);
	    return(SZ_FATAL);
	}

	/*
	 * If there is not already an alignment buffer associated with this
	 * target, allocate one.  Only do this allocation once per target and
	 * hold onto it to make remapping easier for each time alignment is
	 * needed.  Since the alignment buffer is only allocated once the
	 * size of the allocation is set to the maximum allowable transfer
	 * length to be able to accomodate any possible transaction (gack).
	 */
	if (sc->align_buf_alloc[targid] == 0) {
		/*
		 * Allocate the aligned I/O buffer
		 */
#ifdef MACH
		sc->align_buf[targid] = (char *)(((long)alignment_buffer + 32) & (long)~31);
#else
		KM_ALLOC(sc->align_buf[targid], char *, SCSI_MAXPHYS, KM_DEVBUF, 
			    KM_NOW_CL | KM_CONTIG );
#endif
		if ( sc->align_buf[targid] == (char *)NULL ) {
			splx(s);
			return(SZ_FATAL);
		}

		sc->align_buf_alloc[targid] = 1;
	}

	/*
	 * If this is a data transmit command (such as a write) bcopy the data
	 * to be transmitted into the new properly aligned transmit buffer.
	 * Specify the transfer type in the align_buf_inuse field so we know
	 * if a bcopy is needed upon command completion (for reads).
	 * Save off the unaligned buffer pointer for later bcopy usage.
	 */
	if (datai_sink) {  				/* read command */
	    sc->align_buf_inuse[targid] = SCSI_ALIGN_READ;
	    sc->unalign_buf[targid] = (char *)buf_addr;
	    sc->align_length[targid] = bytecount;
	    buf_addr = (u_char *)sc->align_buf[targid];
	} else {					/* write command */
	    sc->align_buf_inuse[targid] = SCSI_ALIGN_WRITE;
	    /*
	     * If the address is a system address we can just copy it.
	     * If it's a user address we need to double map it to insure
	     * that the copy is valid as we might have been called from
	     * interrupt context.
	     */
	    if( IS_SYS_VA( (long)buf_addr )){
	    	bcopy(buf_addr, sc->align_buf[targid], bytecount);
		buf_addr = (u_char *)sc->align_buf[targid];
	    } else {
		buf_addr = (u_char *)scsi_map_user(buf_addr, bytecount,
						bp, targid, sc);
	    	bcopy(buf_addr, sc->align_buf[targid], bytecount);
		buf_addr = (u_char *)sc->align_buf[targid];
	    }
	}
    } else {	/* No alignment needed */
	sc->align_buf_inuse[targid] = 0;
	sc->align_length[targid] = 0;
	sc->unalign_buf[targid] = (char *)0;
    }

    if (buf_addr != (u_char *)NULL) {
	/*
	 * Set that dataaddress to the proper physical address
	 * for the I/O. At this point if we still have a user address
	 * it has to be properly aligned and contiguous otherwise
	 * we would have already pointed the buf_addr to the aligment buffer.
	 *
	 * Note that the user buffer for RAW IO has already been probed by
	 * useracc which is called from physio.
	 */
        if (IS_SYS_VA((u_long)buf_addr)) {
#ifdef MACH
	    if (pmap_svatophys(buf_addr, &scsi_cmd_ptr->dataaddr) == KERN_INVALID_ADDRESS) {
		panic("can't map kernel buffer!\n");
	    }
#else
	    scsi_cmd_ptr->dataaddr = (u_char *)svtophy((u_char *)buf_addr);
#endif
	} else {
            bp = sc->sc_bp[targid];     
	    scsi_cmd_ptr->dataaddr = (u_char *)
#ifdef MACH
		pmap_extract(bp->b_proc->task->map->vm_pmap, (long)buf_addr);
#else
		((vtopte(bp->b_proc, btop(buf_addr))->pg_pfnum << PGSHIFT) + 
			((u_long)buf_addr & PGOFSET));
#endif
	}
    } else {
	scsi_cmd_ptr->dataaddr = (u_char *)0;
    }

    /*
     *
     * Round up to an even multiple of 32 before allocating space.  This is
     * necessary because the TVbus transfers in 32 byte multiples. Do this
     * here instead of above so that the sc->align_length[targid] has the
     * correct lenght for copying back to the user.
     */
    if( datai_sink )
	    bytecount += (32-(bytecount & (32-1)));
    scsi_cmd_ptr->ndata = bytecount;

    /*
     * The command tag field is an arbitrary 64-bit tag that is copied into 
     * the tag field of the command complete message.
     *
     * Set the command tag to be the target id and the command.  This way
     * there is some means of identifying what entries in the command 
     * complete message ring are.
     */
    scsi_cmd_ptr->cmdtag = (u_long)((targid << 8) | scsi_cmd_ptr->cmd[0]);

    /*
     * This field specifies the longest time in seconds that this driver will
     * wait for command completion.  If set to 0 there is no timeout.
     * JAG says use 0.
     */
    scsi_cmd_ptr->timeout = 0;

    /*
     * At this point we are ready to send the command to the scsi
     * interface.  This involves finding a free send command entry in the
     * ring, formatting the ring entry, and hitting the doorbell register.
     *
     * If the ring is presently full then queue the request on the wait 
     * queue.  Later the interrupt routine will start up this request.
     */
    ring_index = get_free_cmd_ring(sc);
    if (ring_index == -1) {
	/*
	 * There are no empty slots available in the command ring.  Queue
         * the request on the wait queue.  This involves obtaining a free queue
	 * entry from the free list.  Next the formatted command ring entry
	 * is copied into the wait list entry.  Finally the entry is added to
	 * the list of waiting commands.
	 *
	 * Presently can't post commands to the waitq in polled mode because
	 * interrupts are disabled.  If this is a problem it should be able to
	 * be fixed.
         */
	if (sc->scsi_polled_mode) {
    	    PRINTD(targid, SCSID_ADU_POSTCMD,
	        ("adu_post_command: command ring full in polled mode!\n"));
    	    splx(s);
	    return(SZ_FATAL);
	}

    	PRINTD(targid, SCSID_ADU_WAITQ,
	   ("adu_post_command: command ring full - queue request!\n"));
	entry = adu_scsi_getqent( &scsi_freeq );
	if (entry == (struct adu_scsi_wait *)0) {
	    printf("adu_post_command:Failed to get an entry from the freeq.\n");
    	    splx(s);
	    return(SZ_FATAL);
	}
	else {
	    bcopy(scsi_cmd_ptr, &entry->send_command, 
		sizeof(struct send_command));
	    Insert_entry( entry, scsi_waitq );
	    /*
	    adu_scsi_printq(&scsi_waitq, "scsi_waitq"); 
	     */
	}
    }
    else {
        /*
         * In polled mode there is no need to enable interrupts because the
         * driver will be polling for the response rather than sleeping for
         * the interrupt.
         *
         * Note that interrupts are being enabled even if the send queue is full
         * and the command has been posted to the waitq.  This is necessary to 
         * allow an interrupt to be generated when an entry frees up in the 
         * command ring which will allow commands to be moved from the waitq to
         * the command ring.  I guess this implies that commands can't be posted
         * to the waitq while in polled mode because there would be no interrupt
         * to allow the command to proceed.
	 *
         * IMPORTANT NOTE: Enable interrupts before copying the command into
         * the ring buffer.  This is necessary because the adu is asynchronously
         * scanning the ring looking for commands.  It is possible that the
         * command could be completed before interrupts are enabled if ints
         * were enabled after posting the command.  Ring the doorbell register
	 * to notify the ADU of the interrupt status.
         */
        if (sc->scsi_polled_mode) {
    	    DISABLE_SCSI_INTERRUPT(ADU_SCSI_IE);
    	    RING_SCSI_DOORBELL;
        }
        else {
    	    /*
    	     * Enable interrupts if the value has changed from interrupts
    	     * disabled.  This test is done because enabling interrupts is
    	     * probably an expensive operation which includes a memory
    	     * barrier.
    	     */
    	    if ((adu_scsi_intr_reg & ADU_SCSI_IE) == 0) {
#ifndef SCSI_POLLED
    	        ENABLE_SCSI_INTERRUPT(ADU_SCSI_IE);
    		RING_SCSI_DOORBELL;
#endif /* SCSI_POLLED */
    	    }
        }
        /*
	 * Paranoia set of the flag field to be empty before initiating the
	 * bcopy operation.  This way the ADU won't get all excited and think
	 * that it has a valid command until all the other fields are filled
	 * in first.
         */
        scsi_cmd_ptr->flag = SCSI_EMPTY;
	/*
	 * There is an empty slot in the command ring.  Copy the already
	 * formatted command ring entry into the slot and ring the doorbell
	 * register to notify the IO module of the change in ring status.
	 */
	bcopy(scsi_cmd_ptr, &scsi_ringptr->command_ring[ring_index], 
		sizeof(struct send_command));
	/*
 	 * Insert a memory barrier to insure the correct ordering of the
	 * writes here.  Without this call it is possible that the flag
	 * field could be set proir to the rest of the command packet
	 * arriving in the device memory.
	 */
	mb();
        /*
         * Notify the io module that this packet is ready to go by setting the
         * flag field and ringing the doorbell register.  Don't set this field
	 * until the doorbell register can be rung.
         */
        scsi_ringptr->command_ring[ring_index].flag = SCSI_SEND_COMMAND;

	/*
        adu_print_cmd(&scsi_ringptr->command_ring[ring_index, targid]);	 
	 */

    	RING_SCSI_DOORBELL;
    }
    splx(s);
    return(SZ_SUCCESS);

}	/* end adu_post_command */

/******************************************************************
 *
 * Name:	get_free_cmd_ring
 *
 * Abstract:	Returns a pointer to the next free entry in the command ring.
 *		This is called to transmit a command to the scsi bus.
 *
 * Inputs:
 *
 * 	sc	- the softc data structure
 *
 * Outputs:		None.
 *
 * Return values: Index into command ring of a free entry.
 *		  -1  if the command ring is full.
 *
 *
 ******************************************************************/

get_free_cmd_ring(sc)
	register struct sz_softc *sc;
{
	register int retval;

	/* 
	 * Look at this entry to see if it is empty.  If the entry
	 * is empty then use it.  If the entry is not empty then
	 * the command queue is presently full.  
	 */
	if (scsi_ringptr->command_ring[sc->sc_ring_cmd_index].flag != SCSI_EMPTY) {
		return(-1);
	}
	retval = sc->sc_ring_cmd_index;
	sc->sc_ring_cmd_index++;
	if (sc->sc_ring_cmd_index == ADU_RING_ENTRIES) {
		sc->sc_ring_cmd_index = 0;
	}
	return(retval);
}

/******************************************************************
 *
 * Name:	get_next_msg_ring
 *
 * Abstract:	Returns a pointer to the next command complete message.
 *		This is called to receive a completion status on commands
 *		issued to the SCSI bus.
 *
 *		IMPORTANT NOTE: This routine returns an index to the next
 *		active command complete message.  It does NOT increment the
 *		active message ring index.  Therefore if this routine is
 *		called twice in a row it will return the same value.  This
 *		is being done to allow polled mode to spin waiting for a
 *		response, then the adu_scsi_intr routine will be called which 
 *		will call this routine again and get the same ring index.
 *
 * Inputs:
 *
 * 	sc	- the softc data structure
 *
 * Outputs:		None.
 *
 * Return values: Index into message ring of the next command complete message.
 *		  -1  if the message ring is empty.
 *
 *
 ******************************************************************/

get_next_msg_ring(sc)
	register struct sz_softc *sc;
{
	register struct command_complete *scsi_msg;

	scsi_msg = &scsi_ringptr->message_ring[sc->sc_ring_msg_index];
	/* 
	 * Look at this entry to see if it is empty.  If the entry
	 * is empty then there are no command completion messages available
	 * at this time.  On the other hand if the message is not empty it is
	 * ready to be acted upon.
	 */
	if (scsi_msg->flag == SCSI_EMPTY) {
		return(-1);
	}
	/*
	 * Paranoia check to see if the flag is what we expect:
	 * If this happens the scsi subsystem will probably lock up because
	 * the index is not being incremented.
	 */
	if (scsi_msg->flag != SCSI_COMMAND_COMPLETE) {
		printf("get_free_cmd_ring: invalid flag field: 0x%x\n",
		        scsi_msg->flag);
		return(-1);
	}
	return(sc->sc_ring_msg_index);
}

/******************************************************************
 *
 * Name:	adu_free_msg_entry
 *
 * Abstract:	Free up a message entry in the command complete message ring.
 *		Increments the index of the presently active command complete
 *		message in the ring.  This operation signifies that processing
 *		of this command complete message has completed.
 *
 *		Set the command tag field to an invalid value to make it more
 *		obvious if something is wrong.
 *
 *		Return ownership of the ring entry over to the IO module by
 *		writing empty to the flag field and ringing the doorbell reg.
 *
 * Inputs:
 *
 * 	sc	- the softc data structure
 *
 * Outputs:		None.
 *
 * Return values: 	None.
 *
 ******************************************************************/

adu_free_msg_entry(sc)
	register struct sz_softc *sc;
{
	scsi_ringptr->message_ring[sc->sc_ring_msg_index].cmdtag = 0xffff;
	scsi_ringptr->message_ring[sc->sc_ring_msg_index].flag = SCSI_EMPTY;   
	sc->sc_ring_msg_index++;
	if (sc->sc_ring_msg_index == ADU_RING_ENTRIES) {
		sc->sc_ring_msg_index = 0;	/* wrap around */
	}
        RING_SCSI_DOORBELL;
}

/******************************************************************
 *
 * Name:	adu_scsi_dumpregs
 *
 * Abstract:	Dump out the registers on the ADU related to SCSI operation.
 *		In its present form this is merely a stub routine.  Since there
 *		really are no readable registers all state is software which
 *		may or may not be of interest to print out.
 *
 * Outputs:     None.
 *
 * Return values: None.
 *
 ******************************************************************/
adu_scsi_dumpregs(cntlr, who)
	int	cntlr;
	int	who;
{
	cprintf("adu_scsi_dumpregs: not yet implemented.\n");
}	/* end adu_scsi_dumpregs */

/*****************************************************************
 *
 * Name:	adu_scsi_intr
 *
 * Abstract:	An interrupt has occured to signify a change in ring status.
 *		At this point a send command complete message may be available.
 *		It is also possible that an entry has freed up in the command
 *		ring.
 *
 * Inputs:
 *
 * 	cntrl	- the controller number which is interrupting
 *	
 *
 * Outputs:	None.
 *
 * Return values: None.
 *
 ******************************************************************/

adu_scsi_intr(cntlr)

int cntlr;
{
    register struct sz_softc *sc = &sz_softc[cntlr];

    register int targid;
    int retval, timer;
    register int msg_ring_index;
    register struct adu_scsi_wait *entry;
    register int ring_index;
    int length;

    int s;
    int	flags;

    if (sc == NULL) { 		/* Stray interrupt on reboot */
        cprintf("adu_scsi_intr: noprobe intr, cntlr = %d\n",cntlr);
	return;
    }	/* end if sc == NULL */

    s = splbio();

    /*
     * Alpha ADU ring handling code:
     *
     * The causes of interrupts are status changes in the rings.  A command
     * complete message may have been received or a send command has completed.
     *
     */

    /*
     * Look to see if the interrupt was generated because an entry in the
     * command ring has freed up.  In this case if there are any commands in
     * the waitq waiting to be sent out to the IO module post those commands.
     * If there are no commands waiting in the waitq then there is no need to
     * do anything.
     */
    while (adu_scsi_queue_empty( &scsi_waitq ) == 0) {
    	PRINTD(targid, SCSID_ADU_WAITQ,
	   ("adu_scsi_intr: waitq not empty.\n"));
    	ring_index = get_free_cmd_ring(sc);
    	if (ring_index == -1) {	/* Command ring is full */
	    break;
	}
	else {
	    /*
	     * There is an empty slot in the command ring.  Copy the already
	     * formatted command ring entry into the slot and ring the doorbell
	     * register to notify the IO module of the change in ring status.
	     * Return the queue entry to the free list.
	     */
	    entry = adu_scsi_getqent( &scsi_waitq );
	    if (entry == (struct adu_scsi_wait *)0) {
    		PRINTD(targid, SCSID_ADU_WAITQ,
		   ("adu_scsi_intr: Failed to get an entry from waitq.\n"));
		continue;	/* This should never happen! */
	    }
	    bcopy(&entry->send_command, 
		    &scsi_ringptr->command_ring[ring_index], 
		    sizeof(struct send_command));
            /*
             * Notify the io module that this packet is ready to go by setting 
             * the flag field and ringing the doorbell register.  Don't set this
	     * field until the doorbell register can be rung.
             */
	    scsi_ringptr->command_ring[ring_index].flag = SCSI_SEND_COMMAND;
    
    	    PRINTD(targid, SCSID_ADU_WAITQ,
	       ("adu_scsi_intr: moved entry from waitq to command ring.\n"));
	    /*
            adu_print_cmd(&scsi_ringptr->command_ring[ring_index, targid]); 
	     */
	    Insert_entry( entry, scsi_freeq );
	    /*
	     * Interrupts should have been enabled if the command is not in
	     * polled mode.  No need to reset that here.
	     */
    	    RING_SCSI_DOORBELL;
	}
    }

    /*
     * Look to see if there are any new entries in the command response ring.
     */
    while((msg_ring_index = get_next_msg_ring(sc)) != -1) {
	/*
	 * We now have an active command response.  Determine the target that
	 * this command is associated with by looking at the command tag field.
	 */
	targid = ((scsi_ringptr->message_ring[msg_ring_index].cmdtag)>>8) & 0x7;
	if ((targid >= NDPS) || (targid < 0)) {
	    printf("adu_scsi_intr: ERROR: invalid targid %d\n",targid);
       	    /*
       	     * Return ownership of the entry in the command complete message
       	     * ring over to the SCSI interface.
       	     */
       	    adu_free_msg_entry(sc);
	    break;
	}
	if ((sc->sc_active & (1 << targid)) == 0) {
	    printf("SCSI ERROR: response on inactive target%d, command 0x%x ",
		targid,
		((scsi_ringptr->message_ring[msg_ring_index].cmdtag) & 0xff));
	    adu_print_command(((scsi_ringptr->message_ring[msg_ring_index].cmdtag) & 0xff));
       	    /*
       	     * Return ownership of the entry in the command complete message
       	     * ring over to the SCSI interface.
       	     */
       	    adu_free_msg_entry(sc);
	    break;
	}
	/*
	 * If buffer alignment is necessary on a read type command bcopy the
	 * data back into the unaligned buffer.  The number of bytes to bcopy
	 * is set to the number of bytes actually read as specified in the
	 * command response.  Don't just blindly bcopy the number of requested
	 * bytes in case there was an error and no bytes were actually read.
	 * As a paranoia check, make sure that the number of bytes read is not
	 * more than was asked for to prevent buffer overflow.
	 */
	if (sc->align_buf_inuse[targid]) {
	    if (sc->align_buf_inuse[targid] & SCSI_ALIGN_READ) {
	        if (scsi_ringptr->message_ring[msg_ring_index].ndata >
		    sc->align_length[targid])
			length = sc->align_length[targid];
	        else 
		    length = scsi_ringptr->message_ring[msg_ring_index].ndata;

		if( IS_SYS_VA( (long)sc->unalign_buf[targid] ) )
			bcopy(sc->align_buf[targid],sc->unalign_buf[targid],length);
		else
			bcopy(sc->align_buf[targid],
				scsi_map_user( sc->unalign_buf[targid], length,
					sc->sc_bp[targid], targid, sc), length);
	    }
	    sc->align_buf_inuse[targid] = 0;
	}

	if (sc->scsi_polled_mode) {
    	    PRINTD(targid, SCSID_FLOW,
       	    ("adu_scsi_intr: polled mode command response on index %d\n",msg_ring_index));
	    /*
	     * As a debug step print out the entire message.
	    adu_print_msg(&scsi_ringptr->message_ring[msg_ring_index], targid);
	     */
	
	    /*
	     * Examine command completion status and set the sc_status field
	     * to communicate the status with the upper layers.
	     */
	    adu_check_status(sc, sc->sc_ring_msg_index, targid);
	    /*
	     * Indicate that this target is no longer active following command
	     * completion.
	     */
	    sc->sc_active &= ~(1 << targid);
       	    /*
       	     * Return ownership of the entry in the command complete message
       	     * ring over to the SCSI interface.
       	     */
       	    adu_free_msg_entry(sc);
	}
	else {
    	    PRINTD(targid, SCSID_FLOW,
       	    ("adu_scsi_intr: non-polled mode command response on index %d\n",msg_ring_index));
	    /*
	     * As a debug step print out the entire message.
	    adu_print_msg(&scsi_ringptr->message_ring[msg_ring_index], targid);
	     */
	
	    /*
	     * Examine command completion status and set the sc_status field
	     * to communicate the status with the upper layers.
	     */
	    adu_check_status(sc, sc->sc_ring_msg_index, targid);
	    /*
	     * Indicate that this target is no longer active following command
	     * completion.
	     */
	    sc->sc_active &= ~(1 << targid);
       	    /*
       	     * Return ownership of the entry in the command complete message
       	     * ring over to the SCSI interface.  Do this before calling sz_start
	     * to insure that this command response does not get reused.
       	     */
       	    adu_free_msg_entry(sc);
#ifndef SCSI_POLLED
	    sc->scsi_completed[targid] = 0;	
	    /*
	     * In non-polled mode the upper half of the driver will now be
	     * asleep in iowait on the bp.  Call sz_start to allow completion
	     * of the current IO request and a wakeup.
	     */
	    sz_start(sc, targid);
#endif /* SCSI_POLLED */
	}
    }

    splx(s);
} 	/* end adu_scsi_intr */

/*****************************************************************
 *
 * Name:	adu_check_status
 *
 * Abstract:	Examine the contents of a command complete message to determine
 *		the completion status of a command.  Set the appropriate status
 *		field in the sc structure to allow the upper layers to know
 *		what errors occured.
 *
 * Inputs:
 *
 * 	sc	   - the softc data structure
 *	ring_index - Index into the command complete message ring to examine
 *		     for completion status.
 *	
 *	
 *
 * Outputs:	None.
 *
 * Return values: None.
 *
 ******************************************************************/

adu_check_status(sc, ring_index, targid)
	register struct sz_softc *sc;
	register int ring_index;
        register int targid;
{
    register u_char scsi_status;
    register struct command_complete *scsi_msg;

    scsi_msg = &scsi_ringptr->message_ring[ring_index];

    /* Check the command completion status */
    switch (scsi_msg->cstatus) {
	case SCSI_OK:	/* Success status as long as the bytes in
			 * sstatus agree.
			 *
			 * nstatus==1 in the following cases:
			 *	- If there are no errors, or if the
			 *	  automatic sensing of status bytes has
			 *	  been disabled (by setting the SCSI_
			 *	  NOSENSE in the options field of the
			 *	  command).  In this case sstatus[0]
			 * 	  contains the SCSI status byte.
			 *	- If sstatus[0] indicates a check
			 *	  condition the automatic request sense
			 *	  received some kind of error.
			 */
		scsi_status = scsi_msg->sstatus[0];
#ifdef 0
		if (scsi_msg->nsstatus == 1) {
#endif
		    sc->sc_status[targid] = scsi_status;
		    switch (scsi_status) {
			case SZ_GOOD:
    	    			PRINTD(targid, SCSID_DMA_FLOW,
				    ("adu_check_stat:SCSI_OK, good status.\n"));
				break;
			case SZ_CHKCND:
    	    			PRINTD(targid, SCSID_ERRORS,
				   ("adu_check_stat: SCSI_OK, check cond.\n"));
				break;
			case SZ_BUSY:
    	    			PRINTD(targid, SCSID_ERRORS,
				   ("adu_check_status: SCSI_OK, dev busy.\n"));
				break;
			case SZ_INTRM:
    	    			PRINTD(targid, SCSID_ERRORS,
				   ("adu_check_status: SCSI_OK, intermed.\n"));
				break;
			case SZ_RESCNF:
    	    			PRINTD(targid, SCSID_ERRORS,
				   ("adu_check_status: SCSI_OK, res confl.\n"));
				break;
			default:
    	    			PRINTD(targid, SCSID_ERRORS,
				   ("adu_check_status: SCSI_OK,unk stat 0x%x.\n", scsi_status));
				break;
    		    } /* end case scsi_status */
#ifdef 0
		}	
		/*
		 * The else clause handles the case where (nstatus != 1).
		 * Here if sstatus[0] indicates a check condition, and the
		 * automatic sensing of SCSI bytes has not been disabled, the
		 * SCSI interface automatically issues a request sense command,
		 * appends the extended status bytes to the sstatus buffer and
		 * increments nstatus appropriately.
		 */
		else { 
		    if (scsi_status == SZ_CHKCND) {
    	    		PRINTD(targid, SCSID_ERRORS,
			   ("adu_check_status: check condition.\n"));
    	    		PRINTD(targid, SCSID_ERRORS,
			   ("adu_check_status: nstatus=%d, scsi_status = 0x%x\n",scsi_msg->nsstatus, scsi_status));
		    }
		    /*
		     * The following case is not expected to be entered because
		     * the only time there should be more than 1 status byte
		     * is when a check condition has occured.
		     */
		    else {
    	    		PRINTD(targid, SCSID_ERRORS,
			   ("adu_check_status: unexpected condition!\n"));
    	    		PRINTD(targid, SCSID_ERRORS,
			   ("adu_check_status: nstatus=%d, scsi_status = 0x%x\n",scsi_msg->nsstatus, scsi_status));
		    }
		}
#endif
		break;
	case SCSI_SELECT: /* The target would not select, target is
			   * non-existant or broken
			   */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		   ("adu_check_status: select error.\n"));
		break;
	case SCSI_REJECT: /* The SCSI interface rejected the command
			   * with the matching command tag.
			   */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		  ("adu_check_stat: cmd rejected tag 0x%x\n",scsi_msg->cmdtag));
		break;
	case SCSI_TIMEOUT: /* The command started, but it did not
			    * complete before the timeout specified in
			    * the command.
			    */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		   ("adu_check_status: timeout error.\n"));
		break;
	case SCSI_BUS:	/* Arbitration failure, bus parity error, etc.
			 */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		   ("adu_check_status: bus error.\n"));
		break;
	case SCSI_OVERRUN: /* The device presented data in a DATAI
			    * phase but the DATA count ran out.
			    */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		   ("adu_check_status: overrun error.\n"));
		break;
	case SCSI_UNDERRUN: /* The device asked for data in a DATAO
			     * phase, but the DATA count ran out.
			     */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		   ("adu_check_status: underrun error.\n"));
		break;
	case SCSI_FIRMWARE: /* Something is wrong that can only be a
			     * firmware error.
			     */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		   ("adu_check_status: firmware error.\n"));
		break;
	default:	/* Unknown status code.
			 */
		sc->sc_status[targid] = SZ_CHKCND;
    	    	PRINTD(targid, SCSID_ERRORS,
		  ("adu_check_status: unknown cstatus 0x%x.\n",scsi_msg->cstatus));
		break;
    } /* end case cstatus */
} /* end adu_check_status routine */

/******************************************************************
 *
 * Name:	adu_print_cmd
 *
 * Abstract:	This is a debug routine that prints out the contents of a
 *		command entry in the command ring.
 *
 * Outputs:     None.
 *
 * Return values: None.
 *
 ******************************************************************/
adu_print_cmd(cmdptr, targid)
	register struct send_command *cmdptr;
	register int targid;
{
	register int i;
	PRINTD(targid, SCSID_CMD_EXP,
	    ("-----------------------------------------------------------\n"));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("adu_print_cmd: command ring entry dump.\n"));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("flag = 0x%x\n", cmdptr->flag));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("cmdtag = 0x%x\n", cmdptr->cmdtag));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("target = 0x%x\n", cmdptr->target));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("bus = 0x%x\n", cmdptr->bus));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("nmsg = 0x%x\n", cmdptr->nmsg));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("ncmd = 0x%x\n", cmdptr->ncmd));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("ndata = %d\n", cmdptr->ndata));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("options = 0x%x\n", cmdptr->options));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("timeout = 0x%x\n", cmdptr->timeout));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("dataaddr = 0x%x\n", cmdptr->dataaddr));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("msg = 0x%x\n", cmdptr->msg));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("CDB cmd packet is: "));
	if (cmdptr->ncmd > 48) {
		PRINTD(targid, SCSID_CMD_EXP,
	    ("ERROR: ncmd should not be greater than 48.\n"));
		cmdptr->ncmd = 48;
	}
	for (i = 0 ; i < cmdptr->ncmd; i++) {
		if ((i % 9) == 0) {
			PRINTD(targid, SCSID_CMD_EXP, ("\n"));
		}
		PRINTD(targid, SCSID_CMD_EXP, ("%02x ",cmdptr->cmd[i]));
	}
	PRINTD(targid, SCSID_CMD_EXP, ("\n"));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("-----------------------------------------------------------\n"));
}	/* end adu_print_cmd */

/******************************************************************
 *
 * Name:	adu_print_msg
 *
 * Abstract:	This is a debug routine that prints out the contents of a
 *		message entry in the command complete message ring.
 *
 * Outputs:     None.
 *
 * Return values: None.
 *
 ******************************************************************/
adu_print_msg(msgptr, targid)
	register struct command_complete *msgptr;
	register int targid;
{
	register int i;

	PRINTD(targid, SCSID_CMD_EXP,
	    ("adu_print_msg: message ring entry dump.\n"));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("adu_print_msg: targid = %d, scsi command byte = 0x%x\n", 
		((msgptr->cmdtag >> 8) & 0xff),
		((msgptr->cmdtag >> 0) & 0xff)));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("adu_print_msg: command completion status is "));
	switch (msgptr->cstatus) {
		case SCSI_OK: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_OK\n")); break;
		case SCSI_SELECT: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_SELECT\n")); break;
		case SCSI_REJECT: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_REJECT\n")); break;
		case SCSI_TIMEOUT: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_TIMEOUT\n")); break;
		case SCSI_BUS: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_BUS\n")); break;
		case SCSI_OVERRUN: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_OVERRUN\n")); break;
		case SCSI_UNDERRUN: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_UNDERRUN\n")); break;
		case SCSI_FIRMWARE: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("SCSI_FIRMWARE\n")); break;
		default: PRINTD(targid, SCSID_CMD_EXP,
	    	    ("UNKNOWN\n")); break;
	}
	PRINTD(targid, SCSID_CMD_EXP,
	    ("adu_print_msg: Number of data bytes read or written is %d\n",msgptr->ndata));
	PRINTD(targid, SCSID_CMD_EXP,
	    ("adu_print_msg: The number of status bytes returned is %d\n",msgptr->nsstatus));
	PRINTD(targid, SCSID_CMD_EXP, ("adu_print_msg: Status bytes are: "));
	for (i = 0 ; i < msgptr->nsstatus; i++) {
		if ((i % 9) == 0) {
			PRINTD(targid, SCSID_CMD_EXP, ("\n"));
		}
		PRINTD(targid, SCSID_CMD_EXP, ("%02x ",msgptr->sstatus[i]));
	}
	PRINTD(targid, SCSID_CMD_EXP, ("\n"));
}	/* end adu_print_msg */

/******************************************************************
 *
 * Name:	adu_print_inq_info
 *
 * Abstract:	This is a debug routine that prints out the contents of 
 *		the inquiry data information.
 *
 * Outputs:     None.
 *
 * Return values: None.
 *
 ******************************************************************/
adu_print_inq_info(idp, targid)
	register struct sz_inq_dt *idp;
	register int targid;
{
    
    char hold[SZ_PID_LEN+1];
    register int i;
    register u_char *ptr;
    u_char *phys_address;

    PRINTD(targid, SCSID_CMD_EXP,
	("***************************************************\n"));
#ifdef MACH
    if (pmap_svatophys(idp, &phys_address) == KERN_INVALID_ADDRESS) {
	panic("can't map inquiry data!");
    }
#else
    phys_address = (u_char *)svtophy((u_char *)idp);
#endif
    PRINTD(targid, SCSID_CMD_EXP,
	("Dumping Out Inquiry Data from 0x%x, 0x%x:\n",idp, phys_address));

    for (i = 0; i < SZ_VID_LEN; i++)
	hold[i] = idp->vndrid[i];

    hold[i] = '\0';
    PRINTD(targid, SCSID_CMD_EXP, ("Vendor ID = %s\n", hold));
    for (i = 0; i < SZ_PID_LEN; i++)
	hold[i] = idp->prodid[i];
    hold[i] = '\0';
    PRINTD(targid, SCSID_CMD_EXP, ("Product ID = %s\n", hold));
    PRINTD(targid, SCSID_CMD_EXP,
	("Peripheral Device Type = 0x%x\n",idp->perfdt));
    PRINTD(targid, SCSID_CMD_EXP,
	("Device Type Qualifier = 0x%x\n",idp->devtq));

    for (i = 0; i < SZ_REV_LEN; i++)
	hold[i] = idp->revlvl[i];
    hold[i] = '\0';
    PRINTD(targid, SCSID_CMD_EXP, ("Revision Level = %s\n", hold));
    PRINTD(targid, SCSID_CMD_EXP,
	("***************************************************\n"));

}	/* end adu_print_inq_info */

/******************************************************************
 *
 * Name:	adu_print_sense
 *
 * Abstract:	DEBUG ROUTINE to print out the request sense data.
 *
 * Inputs:
 * sc		Pointer to sz_softc structure for this controller.
 * targid	Target Id of device (0 - 7).
 *
 * Outputs:	None.
 *
 * Return values: None.
 ******************************************************************/

adu_print_sense(sc, targid)

register struct sz_softc *sc;
int targid;
{
    u_char *byteptr;
    int i;

    byteptr = (u_char *)&sc->sc_sns[targid];

    PRINTD(targid, SCSID_ERRORS,
	("\nadu_print_sense: (targid=%d) ", targid));

    for (i = 0; i < SZ_RQSNS_LEN; i++)
	PRINTD(targid, SCSID_ERRORS, ("%x ", *(byteptr + i)));

    switch (sc->sc_sns[targid].snskey) {
    case 0x0:
	PRINTD(targid, SCSID_ERRORS, ("(No Sense)")); break;
    case 0x1:
	PRINTD(targid, SCSID_ERRORS, ("(Soft Error)")); break;
    case 0x2:
	PRINTD(targid, SCSID_ERRORS, ("(Not Ready)")); break;
    case 0x3:
	PRINTD(targid, SCSID_ERRORS, ("(Medium Error)")); break;
    case 0x4:
	PRINTD(targid, SCSID_ERRORS, ("(Hardware Error)")); break;
    case 0x5:
	PRINTD(targid, SCSID_ERRORS, ("(Illegal Request)")); break;
    case 0x6:
	PRINTD(targid, SCSID_ERRORS, ("(Unit Attention)")); break;
    case 0x7:
	PRINTD(targid, SCSID_ERRORS, ("(Write Protected)")); break;
    case 0x8:
	PRINTD(targid, SCSID_ERRORS, ("(Blank Check)")); break;
    case 0x9:
	PRINTD(targid, SCSID_ERRORS, ("(Vendor Unique)")); break;
    case 0xa:
	PRINTD(targid, SCSID_ERRORS, ("(Copy Aborted)")); break;
    case 0xb:
	PRINTD(targid, SCSID_ERRORS, ("(Aborted Command)")); break;
    case 0xc:
	PRINTD(targid, SCSID_ERRORS, ("(Equal Error)")); break;
    case 0xd:
	PRINTD(targid, SCSID_ERRORS, ("(Volume Overflow)")); break;
    case 0xe:
	PRINTD(targid, SCSID_ERRORS, ("(Miscompare Error)")); break;
    case 0xf:
	PRINTD(targid, SCSID_ERRORS, ("(Reserved)")); break;
    default:
	PRINTD(targid, SCSID_ERRORS, ("(Unknown)")); break;
    }
    if(sc->sc_sns[targid].filmrk) {
	PRINTD(targid, SCSID_ERRORS, (" filmrk"));
    }
    else if(sc->sc_sns[targid].eom) {
	PRINTD(targid, SCSID_ERRORS, (" eom"));
    }
    else if(sc->sc_sns[targid].ili) {
	PRINTD(targid, SCSID_ERRORS, (" ili"));
    }
    PRINTD(targid, SCSID_ERRORS, ("\n"));

}	/* end adu_print_sense */

/******************************************************************
 *
 * Name:	adu_scsi_printq
 *
 * Abstract:	DEBUG ROUTINE to print out the waitq or freeq.
 *
 * Inputs:
 * queue	Pointer to the actual queue.
 * queue_name	String of queue name.
 *
 * Outputs:	None.
 *
 * Return values: None.
 ******************************************************************/

adu_scsi_printq(scsiqueue, queue_name)
	struct aduqueue *scsiqueue;
	char *queue_name;
{
	struct adu_scsi_wait *entry;
	int index = 0;
	int targid = 0;
	PRINTD(targid,SCSID_ADU_WAITQ,("Contents of the %s queue:",queue_name));
	if ((scsiqueue->flink == (struct adu_scsi_wait *)scsiqueue) && 
	    (scsiqueue->blink == (struct adu_scsi_wait *)scsiqueue)) {
		PRINTD(targid, SCSID_ADU_WAITQ,("Empty queue.\n"));
		return;
	}
	PRINTD(targid, SCSID_ADU_WAITQ,("\n"));
	for ( entry = scsiqueue->flink; 
		entry != (struct adu_scsi_wait *)&scsiqueue->flink;
			entry = entry->flink ) {
		index++;
		if (index > (ADU_SCSI_NUMWAITS)) {
		    PRINTD(targid,SCSID_ADU_WAITQ,("ERROR: queue overflow.\n"));
		    break;
		}
		PRINTD(targid, SCSID_ADU_WAITQ,("Entry number %d : ",index));
		adu_scsi_printentry( entry , targid );

	}
}	/* end adu_scsi_printq */

/******************************************************************
 *
 * Name:	adu_scsi_printentry
 *
 * Abstract:	DEBUG ROUTINE to print out the waitq or freeq.
 *
 * Inputs:
 * entry	Pointer to a queue entry which is to be printed out.
 *
 * Outputs:	None.
 *
 * Return values: None.
 ******************************************************************/

adu_scsi_printentry(entry, targid)
	register struct adu_scsi_wait *entry;
	register int targid;
{
	PRINTD(targid, SCSID_ADU_WAITQ,("Queue entry contents:\n"));
	PRINTD(targid, SCSID_ADU_WAITQ,
	    ("flink = 0x%x, blink = 0x%x\n",entry->flink, entry->blink));
	adu_print_cmd(&entry->send_command, targid);
}	/* end adu_scsi_printentry */


/******************************************************************
 *
 * Name:	adu_print_command
 *
 * Abstract:	DEBUG ROUTINE to translate a command to a string.  If the
 *		command is not in the list of common commands given here just
 *		put out a linefeed.  Note that this used printf instead of
 *		PRINTD.
 *
 * Inputs:
 * entry	SCSI command byte.
 *
 * Outputs:	None.
 *
 * Return values: None.
 ******************************************************************/

adu_print_command(cmd)
	register u_char cmd;
{
	switch(cmd) {
		case SZ_TUR: printf("SZ_TUR"); break;
		case SZ_REWIND: printf("SZ_REWIND"); break;
		case SZ_RQSNS: printf("SZ_RQSNS"); break;
		case SZ_RBL: printf("SZ_RBL"); break;
		case SZ_READ: printf("SZ_READ"); break;
		case SZ_WRITE: printf("SZ_WRITE"); break;
		case SZ_TRKSEL: printf("SZ_TRKSEL"); break;
		case SZ_RESUNIT: printf("SZ_RESUNIT"); break;
		case SZ_WFM: printf("SZ_WFM"); break;
		case SZ_SPACE: printf("SZ_SPACE"); break;
		case SZ_INQ: printf("SZ_INQ"); break;
		case SZ_VFY: printf("SZ_VFY"); break;
		case SZ_RBD: printf("SZ_RBD"); break;
		case SZ_MODSEL: printf("SZ_MODSEL"); break;
		case SZ_RELUNIT: printf("SZ_RELUNIT"); break;
		case SZ_ERASE: printf("SZ_ERASE"); break;
		case SZ_MODSNS: printf("SZ_MODSNS"); break;
		case SZ_SSLU: printf("SZ_SSLU"); break;
		case SZ_RECDIAG: printf("SZ_RECDIAG"); break;
		case SZ_SNDDIAG: printf("SZ_SNDDIAG"); break;
		case SZ_P_FSPACER: printf("SZ_P_FSPACER"); break;
		case SZ_P_FSPACEF: printf("SZ_P_FSPACEF"); break;
		case SZ_RDCAP: printf("SZ_RDCAP"); break;
		case SZ_P_BSPACER: printf("SZ_P_BSPACER"); break;
		case SZ_P_BSPACEF: printf("SZ_P_BSPACEF"); break;
		case SZ_P_CACHE: printf("SZ_P_CACHE"); break;
		case SZ_P_NOCACHE: printf("SZ_P_NOCACHE"); break;
		case SZ_P_LOAD: printf("SZ_P_LOAD"); break;
		case SZ_P_UNLOAD: printf("SZ_P_UNLOAD"); break;
		case SZ_P_SSUNIT: printf("SZ_P_SSUNIT"); break;
		case SZ_FORMAT: printf("SZ_FORMAT"); break;
		case SZ_REASSIGN: printf("SZ_REASSIGN"); break;
		case SZ_VFY_DATA: printf("SZ_VFY_DATA"); break;
		case SZ_RDD: printf("SZ_RDD"); break;
		case SZ_READL: printf("SZ_READL"); break;
		case SZ_WRITEL: printf("SZ_WRITEL"); break;
	}
	printf("\n");
}	/* end adu_print_command */

/******************************************************************
 *
 * Name:	adu_scsi_kick
 *
 * Abstract:	DEBUG ROUTINE 
 *		I suspect that interrupts are not being delivered in all cases.
 *		As a result, commands posted to the SCSI driver will end up
 *		hanging in physstrat because a wakeup will never be done on
 *		the bp.  This routine is called from physstrat periodically 
 *		while sleeping on the bp to see if a response has been put 
 *		onto the command response ring without generating an interrupt.
 *
 * Inputs:
 * entry	
 *
 * Outputs:	None.
 *
 * Return values: None.
 ******************************************************************/

adu_scsi_kick(bp)
	register struct buf *bp;
{
    register int msg_ring_index;
    register int cntlr = 0;
    register struct sz_softc *sc;

    sc = &sz_softc[cntlr];
    msg_ring_index = get_next_msg_ring(sc);
    if (msg_ring_index == -1) {
	printf("adu_scsi_kick: command response ring is EMPTY.\n");
    }
    else {
	printf("adu_scsi_kick: command response ring is NOT empty.\n");
	printf("adu_scsi_kick: generate fake scsi interrupt...\n");
	adu_scsi_intr(cntlr);
    }
}	/* end adu_scsi_kick */

/******************************************************************
 *
 * Name:	scsi_dmabuf_contig
 *
 * Abstract:	
 *      This routine is used to determine if a DMA buffer resides in physically
 *      contiguous memory.  This is a requirement for DMA.  This is determined
 *      by looking at the PFN's associated with the buffer to see if they are
 *      consecutive.
 *
 * Inputs:
 *	vaddr	- Virtual address of the buffer.
 *	length	- Length of the buffer used in DMA transfers.
 *
 * Return values: 
 * 	    0	- If the buffer is NOT physically contiguous.
 *	    1	- If the buffer IS physically contiguous.
 ******************************************************************/
scsi_dmabuf_contig(vaddr, length, bp)
	char *vaddr;
	int  length;
	struct buf *bp;
{
	unsigned long pfn, next_pfn;	/* Used to store page frame numbers */
	u_long page_offset;		/* Offset within a page */
	int npages;			/* number of pages to check	*/
	int rlength;			/* length from beginning of page */
#ifdef MACH
	pmap_t mypmap;			/* pmap on whose behalf we're mapping */
	vm_offset_t pptr;
#else
	struct pte *pptr;		/* page table pointer		*/
#endif

	page_offset = (long)vaddr & PGOFSET;
	/*
	 * If the buffer is page aligned and not greater than a page
	 * then it must be aligned.
	 */
	if ( page_offset == 0 && length <= NBPG )
		return 1;

	/*
	 * Number of pages has to include an extra page
	 * if the length including the offset is not
	 * a multiple of nbpg.
	 */
	rlength = length+page_offset;
	npages = rlength / NBPG;
	if ( rlength & PGOFSET )
		npages++;
#ifdef MACH
	if (IS_SYS_VA((long)vaddr))
		mypmap = kernel_pmap;
	else
		mypmap = bp->b_proc->task->map->vm_pmap;
	pptr = pmap_extract(mypmap, (long)vaddr);
#else
	pptr = IS_SYS_VA((long)vaddr) ? svtopte( vaddr ) : vtopte( bp->b_proc ,btop(vaddr) ); 
#endif

	/*
	 * Walk through the page table to see if the buffer is contigous
	 */
#ifdef MACH
	pfn = (unsigned long)pptr & ~PGOFSET;	/* beginning pfn */
	--npages;				/* skip first page */
	vaddr += NBPG;
	pfn += NBPG;
	while (npages) {
		if (((unsigned long)pmap_extract(mypmap, vaddr) & ~PGOFSET) != pfn)
			return (0);
		--npages;
		pfn += NBPG;
		vaddr += NBPG;
	}
#else
	pfn = pptr->pg_pfnum;
	while( pptr++, --npages )
		if( pptr->pg_pfnum != ++pfn)
			return 0;
#endif
	return 1;
}
/******************************************************************
 *
 * Name:	scsi_map_user
 *
 * Abstract:	
 *	This routine double maps a user buffer
 *
 * Inputs:
 *	vaddr	- Virtual address of the buffer.
 *	length	- Length of the buffer used in DMA transfers.
 *	targid  - scsi target
 *
 * Return values: 
 * 	    returns the system virtual address that the user buffer
 *	    is mapped at.
 ******************************************************************/
char *
scsi_map_user(buf_addr, length, bp, targid, sc)
char *buf_addr;
int length;
struct buf *bp;
int targid;
struct sz_softc *sc;
{
#if     MACH
        /*
         * Isn't this MUCH simpler
         */
	pmap_t  pmap;
	vm_offset_t virt;
	kern_return_t ret;
	u_long page_offset;
	vm_offset_t v;
	char *bufp;

	pmap = bp->b_proc->task->map->vm_pmap;
	v = (vm_offset_t)buf_addr;
	page_offset = v & PGOFSET;
	if( sc->sc_SZ_bufmap[targid] == 0 ){
		/*
		 * Get the mapping resources we need to map the
		 * unaligned buffer.
		 */
		sc->sc_SZ_bufmap[targid] = (char *)vm_alloc_kva(66*NBPG);
		if (sc->sc_SZ_bufmap[targid] == NULL) {
		    printf("scsi_map_user: scsi %d targetID %d: %s\n",
			0, targid, "cannot get PTEs for bufmap");
		    panic("scsi_map_user - map allocation failed");
		}
	}
	virt = (vm_offset_t)sc->sc_SZ_bufmap[targid];
	bufp = (char *) virt + page_offset;
	ret = pmap_dup(pmap, v, length, bufp, VM_PROT_WRITE, TB_SYNC_ALL);
	if(ret != KERN_SUCCESS)
		panic("sz_start: pmap_dup");
#else   /* MACH */
	struct pte *ipte, *opte;	/* pte pointers		*/
	u_long page_offset;		/* Offset within a page */
	int npages;			/* number of pages to check	*/
	int rlength;			/* length from beginning of page */
	char *vaddr;			/* system virtual address to use */

	if( sc->sc_SZ_bufmap[targid] == 0 ){
		/*
		 * Get the mapping resources we need to map the
		 * unaligned buffer.
		 */
		sc->sc_SZ_bufmap[targid] = (char *)
			get_sys_ptes( (SCSI_MAXPHYS/NBPG)+1, &sc->sc_szbufmap[targid]);
		if( sc->sc_SZ_bufmap[targid] == 0 )
			panic("scsi_map_user - map allocation failed");
	}

	opte = sc->sc_szbufmap[targid];
	ipte = vtopte( bp->b_proc ,btop(buf_addr));
	page_offset = (long)buf_addr & PGOFSET;
	vaddr = sc->sc_SZ_bufmap[targid] + page_offset;

	/*
	 * Number of pages must include an extra page
	 * if the length including the offset is not
	 * a multiple of nbpg.
	 */
	rlength = length+page_offset;
	npages = rlength / NBPG;
	if ( rlength & PGOFSET )
		npages++;
	/*
	 * Copy the pte's, protection and all
	 */
	do {
		*(long *)opte = *(long *)ipte;
		mtpr_tbis( vaddr );
		opte++;
		ipte++;
		vaddr += NBPG;
	} while( --npages );
#endif
	return (sc->sc_SZ_bufmap[targid] + page_offset);
}
