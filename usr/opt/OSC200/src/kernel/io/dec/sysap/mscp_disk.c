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
static char *rcsid = "@(#)$RCSfile: mscp_disk.c,v $ $Revision: 1.1.23.2 $ (DEC) $Date: 1993/07/13 16:27:09 $";
#endif
/*
 * derived from mscp_disk.c	2.7	(ULTRIX)	1/19/90";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Disk Class Driver
 *
 *   Abstract:	This module contains the standard ULTRIX "top half"
 *		routines and other routines specific to the disk
 *		variant of MSCP.
 *
 *   Author:	David E. Eiche	Creation Date:	September 30, 1985
 *
 *   History:
 *
 *   31-Oct-1991	Pete Keilty
 *	- Changed mscp_open, mscp_ioctl, mscp_strategy & mscp_size to use 
 *	  OSF disklabel.  
 *	- Modified mscp_check_sysdev comment out swdevt checks not in OSF.
 *
 *   02-Apr-1991	Tom Tierney
 *	Fixed recovery problem: mscp_markoffline routine was updated to
 *	check if we are currently in recovery processing and if so, to 
 *	return correct new event to continue recovery processing along
 *	with notification via printf of error.  This corrects a problem
 *	where the system would hang if recovery failed for a particular
 *	disk.
 *
 *   22-Feb-1991	Brian Nadeau
 *	Allow up to 2 minutes for critical devices (swap, dump, root) to
 *	become available and print a status message after we wait the first
 *	minute.
 *
 *   19-Feb-1991	Tom Tierney
 *	ULTRIX to OSF/1 porting:
 *	  
 *   Dec - 1990		Matthew S Sacks
 *	Made the handler SMP safe.  Changes for each routine are
 *	documented at the beginning of each routine.  For a discussion
 *	of the changes, see the comments at the end of mscp_defs.h
 *
 *   09-Nov-1989	David E. Eiche		DEE0083
 *	Fix status subcode test to use MSCP_SC_EXUSE instead of MSCP_ST_OFFLN.
 *
 *   23-Oct-1989	Tim Burke
 *	Added support for the exclusive access unit attribute.  This involved
 *	adding the DKIOCEXCL ioctl, the set unit characteristics state table
 *	and routines and modifiying online and available command routines.
 *
 *   09-Aug-1989	Tim Burke		
 *	Added the DEVGETGEOM ioctl which is used to pass disk geometry 
 *	information.  Removed all references to dk_busy because it is no
 *	longer used.
 *
 *   08-Apr-1989	Tom Kong
 *	In mscp_close, took out the floating point operation for mips.
 *
 *   15-Mar-1989	Tim Burke
 *	Changed splx( IPL_SCS ) to Splscs();
 *
 *   07-Mar-1989	Todd M. Katz		TMK0002
 *	1. Include header file ../vaxmsi/msisysap.h.
 *	2. Use the ../machine link to refer to machine specific header files.
 *
 *   17-Oct-1988	Pete Keilty
 *	Changed mscp_open and mscp_close now does explicit wakeup on
 *	up at end of routine.
 *
 *   27-Sep-1988	David E. Eiche		DEE0057
 *	Modified panic message formats to make them consistent.
 *	
 *   13-Sep-1988	Pete Keilty
 *	Changed transferem routine so the requested lbn is not
 *	over written on a BBR status return which was causing the 
 *	wrong lbn to be re-read.
 *
 *   19-Aug-1988	Pete Keilty
 *	Added synchronization code to the open and close routine.
 *	New flags busy and close_ip.
 *
 *   05-Aug-1988	Pete Keilty
 *	Change mscp_close added sleep on rp. Corrects case where
 *	an open_ip flags is clear out when it should not have been
 *	durning a open.
 *
 *   17-Jul-1988	David E. Eiche		DEE0045
 *	Change the get device information ioctl code to use the
 *	connection block as the source of its controller model
 *	information.
 *
 *   08-Jul-1988	Pete Keilty
 *	Added accscancm and accscanem routines, ioctl ACC_SCAN code,
 * 	and changes force_ip to force_scan_ip where used.
 *
 *   02-Jun-1988     Ricky S. Palmer
 *      Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   16-May-1988	Stephen Reilly
 *	Close is only called once per device.
 *
 *   25-Apr-1988	Robin
 *	Added code in ioctl to stop bbr from starting on controllers that do
 *	BBR. Also added code to keep track of I/O which is used by iostat(1).
 *
 *   19-Apr-1988	Robin
 *	To stop a unit from going avail when it is still in use the
 *	partition mask was changed to an open counter.  A unit can have
 *	several opens on one or more partitions and the unit should
 *	never go avail unless all opens are followed by a close.
 *
 *   07-Apr-1988	David E. Eiche		DEE0027
 *	Fix DEE0022 to not panic when booting from an HSC that doesn't
 *	come online.
 *
 *   03-Apr-1988	David E. Eiche		DEE0024
 *	Update DEE0013 to make write protection work with open NDELAY.
 *
 *   03-Apr-1988	David E. Eiche		DEE0023
 *	Add code to implement exclusive access to disk units, leaving
 *	it commented out pending action by the HSC group.
 *
 *   03-Apr-1988	David E. Eiche		DEE0022
 *	Reorder mscp_open routine, eliminate the mscp_onlineinit
 *	routine and references to it, and add an additional test
 *	to mscp_size, to eliminate another race window caused by
 *	the interaction of open NDELAY and close.
 *
 *   23-Mar-1988	David E. Eiche		DEE0019
 *	Fix race between open and close in which a partition was
 *	reopened before the close processing had completed.
 *
 *   21-Mar-1988	David E. Eiche		DEE0018
 *	Change the mscp_open routine to process NDELAY flag in the
 *	historically approved manner.
 *
 *   17-Mar-1988	David E. Eiche		DEE0017
 *	Add jacket routines mscp_bopen, mscp_copen, mscp_bclose and
 *	mscp_cclose whose purpose is to pass a flag into mscp_open and
 *	mscp_close indicating whether the operation is being done in
 *	raw or block mode.  The flag is used with the partition	index
 *	portion of the dev parameter to determine which partitions are
 *	active.
 *
 *   12-Feb-1988	David E. Eiche		DEE0013
 *	Change mscp_onlgtuntem and mscp_ioctl to detect and report
 *	write protection status correctly.
 *	
 *   02-Feb-1988	David E. Eiche		DEE0011
 *	Change mscp_strategy to sleep if the request block is not
 *	available immediately.  Also remove code that initialized
 *	unused request block wait queue.
 *
 *   15-Jan-1988	Todd M. Katz		TMK0001
 *	Include new header file ../vaxmsi/msisysap.h.
 */
/**/


/* Libraries and Include Files.
 */
#include	<labels.h>
#include	<sys/dk.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/buf.h>
#include	<sys/errno.h>
#include	<sys/ioctl.h>
#include	<sys/file.h>
#include	<io/common/devio.h>
#include	<io/dec/sysap/dkio.h>
#include	<sys/disklabel.h>
#include	<dec/binlog/errlog.h>
#include	<sys/syslog.h>
#include	<sys/user.h>
#include	<sys/secdefines.h>
#if SEC_BASE
#include	<sys/security.h>
#endif
#include	<io/common/pt.h>
#include	<io/common/devdriver.h>
#include	<io/dec/scs/sca.h>
#include	<io/dec/sysap/sysap.h>
#include	<io/dec/sysap/mscp_bbrdefs.h>



/* External Variables and Routines.
 */
extern	REQB 		*mscp_alloc_reqb();
extern	u_long		mscp_alloc_rspid();
extern	u_long		mscp_bbr_online();
extern	u_long		mscp_bbr_replace();
extern	void		mscp_common_init();
extern	void		mscp_control();
extern	void		mscp_datagram();
extern	void		mscp_dealloc_reqb();
extern	void		mscp_getdefpt();
extern	void		mscp_media_to_ascii();
extern	void		mscp_message();
extern	u_long		mscp_recycle_rspid();
extern	void		mscp_restart_next();
extern	u_long		mscp_send_msg();
extern	void		mscp_system_poll();
extern	void		mscp_timer();
extern	CLASSB		mscp_classb;
extern	UNITB		*mscp_unit_tbl[];
extern	RSPID_TBL	mscp_rspid_tbl[];
extern	RSPID_LK	Rspid_lk;
extern	LISTHD		mscp_rspid_lh;
extern	STATE		mscp_avl_states[];
extern	STATE		mscp_onl_states[];
extern	STATE		mscp_rec_states[];
extern	STATE		mscp_accscan_states[];
extern	STATE		mscp_stu_states[];
extern	STATE		mscp_xfr_states[];
extern	STATE		mscp_repl_states[];
extern	DMSCP_MEDIA	dmscp_media[];

extern	int		MSCP_log_label_info;
extern	int		dmscp_media_ct;
extern	int		hz;
extern	int		lbolt;
extern	int		wakeup();
extern	dev_t		rootdev;
extern	dev_t		dumpdev;
extern	struct	swdevt	swdevt[];
extern  int             partial_dump;
extern  int             partial_dumpmag;
extern  int             dumpsize;
extern	int		MSCP_MAXPHYS;

UNITB			*mscp_check_sysdev();
void			mscp_minphys();

/*
 * We are adding some "latent" support for the HSX00 (single-spindle)
 * and HSX01 (multi-spindle) FIB RAID devices.  These media id definitions
 * are temporarily being defined here so we can differentiate between 
 * devices and set the dynamic geometry flag in the GEOM ioctl for these
 * RAID devices.  The proper fix will be to include a flag along with
 * other device information specific maintained in mscp_data.c. 
 */
#define	HSX00_MEDIA 0x25513c00		/* FIB RAID single-spindle */
#define	HSX01_MEDIA 0x25513c01		/* FIB RAID multi-spindle  */
/**/

/*
 *
 *   Name:	mscp_init_driver	- Initialize class driver
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	protect the sysap class block.
 *
 *   Return	NONE
 *   Values:
 */

/* TODO (?) Should initial sequence number be randomized?
 */

void
mscp_init_driver()
{
    int			i, s;
    u_short		init_seq_no = 1;
    CLASSB		*clp = &mscp_classb;
    CMSB		*cmsp = &clp->cmsb;

    s = Splscs();
    Lock_classb (clp);

    /* Do initialization common to disk and tape class drivers.
     */
    mscp_common_init();

    /* Prevent re-entry during initialization
     */
    /* Since this runs at init time on the boot cpu, we do not really
	need to lock, but for the sake of style we do.  The lock was
	initialized by scs_start_sysaps.
    */

    if ( clp->flags.init_ip || clp->flags.init_done ) {
	Unlock_classb (clp);
	( void )splx( s );
	return;
    }
    /* Inititialize class block flags
     */
    clp->flags.init_ip = 1;
    clp->flags.disk = 1;

    /* Init class driver block listheads
     */
    clp->flink = ( CONNB * )&clp->flink;
    clp->blink = ( CONNB * )&clp->flink;

    /* Zero and fill in the embedded connection management
     * services block (CMSB).
     */
    cmsp->control = mscp_control;
    cmsp->msg_event = mscp_message;
    cmsp->dg_event = mscp_datagram;
    cmsp->lport_name = 0;
    Zero_scaaddr( cmsp->rport_addr );
    cmsp->init_dg_credit = 2;
    cmsp->min_snd_credit = 2;
    cmsp->init_rec_credit = 10;
    ( void )bcopy( "U32_DISK_CL_DRVR",
		    cmsp->lproc_name,
		    NAME_SIZE );
    ( void )bcopy( "MSCP$DISK       ",
		    cmsp->rproc_name,
		    NAME_SIZE );
    ( void )bcopy( "                ",
		    cmsp->conn_data,
		    DATA_SIZE );

    /* Initialize the unit table and point the class block at it.
     */
    for( i = 0; i < NUNIT; i++ )
	mscp_unit_tbl[ i ] = ( UNITB * )NULL;

    clp->unit_tbl = mscp_unit_tbl;

    /* Initialize the ULTRIX device disk device name string.
     */
    clp->dev_name = "ra";

    /* Initialize recovery state table pointer
     */
    clp->recov_states = mscp_rec_states;

    /* Start up a 1 second timer for use by connection management and
     * the resource allocation routines.
     */
    (void) timeout(mscp_timer, (caddr_t)clp, hz);

    /* Find all the currently known subsystems, start the
     * connection process (which will complete asynchronously),
     * restore the entry IPL and exit.
     */
    mscp_system_poll(clp);

    Unlock_classb (clp);
    (void) splx(s);

    return;
}

/**/

/*
 *
 *   Name:	mscp_onlinecm - Format and send an MSCP ONLIN command.
 *
 *   Abstract:	Format and queue an MSCP online command to be sent.
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	Protect unit block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_onlinecm( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
    UNITB		*up = rp->unitb;
    u_long		new_event;

    /* Format the message buffer as a ONLINE command with the exclusive
     * access modifier.  Update the state and send the message, then exit
     * to wait for the ONLINE end message to come back.
     * If excl_acc is specified then set the unit into the exclusive access
     * pseudo state.  The unit will remain in exclusive access mode if the
     * modifier was previously set.  If the exclusive access modifier is not
     * set and the unit is presently in the exclusive access state the unit
     * will no longer be in exclusive access mode.
     */
    Lock_unitb (up);
    Init_msg( mp, rp->rspid, up->unit );
    mp->mscp_opcode = MSCP_OP_ONLIN;
    if ( up->flags.excl_acc )
        mp->mscp_modifier = MSCP_MD_EXCAC;
    Unlock_unitb (up);
    new_event = mscp_send_msg( rp );
    return ( new_event );
}

/**/

/*
 *
 *   Name:	mscp_onlineem - process an online end message.
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	protect modifications of the unit block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_onlineem( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
    UNITB		*up = rp->unitb;
    u_long		new_event;
    u_short		em_status = mp->mscp_status & MSCP_ST_MASK;
    u_short		em_subcode = mp->mscp_status >> MSCP_ST_SBBIT;

    /* Check to see if the online succeeded.
     */
    if( em_status ==  MSCP_ST_SUCC ) {

	Lock_unitb (up);

	/* Set a unit flag to remember to bypass BBR processing
	 * if the unit was already online.
	 */
	up->flags.alonl = ( em_subcode == MSCP_SC_ALONL );

	/* If the unit is formatted for the DEC10/20, it is not
	 * usable.  Redispatch to avail the unit.
	 * TODO - Check D0 and D1 of media ID against unitb.
	 */
	if( mp->mscp_unt_flgs & MSCP_UF_576 ) {
	    Unlock_unitb (up);
	    mscp_recycle_rspid( rp );
	    new_event = EV_ONLERRAVAIL;

	/* Store the contents of the online end message, recycle the
	 * RSPID, then format and send a get unit status message.
	 */
	} else {
	    up->unit_id = *( UNIQ_ID * )mp->mscp_unit_id;
	    up->media_id = mp->mscp_media_id;
	    up->unt_size = mp->mscp_unt_size;
	    up->vol_ser = mp->mscp_vol_ser;
	    up->unt_flgs = mp->mscp_unt_flgs;
	    Unlock_unitb (up);

	    mscp_recycle_rspid( rp );
	    Init_msg( mp, rp->rspid, up->unit );
	    mp->mscp_opcode = MSCP_OP_GTUNT;
	    new_event = mscp_send_msg( rp );
	}
		
    /* The online command did not succeed.  If the error was caused by
     * a duplicate unit number, print a message on the console.  For all
     * errors, reset online-in-progress and redispatch with an online
     * complete event.
     */
    } else { 
	if( em_status == MSCP_ST_OFFLN) {
	    switch (em_subcode) {
		case MSCP_SC_DUPUN:
		    printf( "mscp - Duplicate unit %d detected", up->unit );
		    printf( " on controller xxx\n");
		    break;
		case MSCP_SC_EXUSE:
		    printf( "mscp - Unit %d is exclusive access", up->unit );
                    printf( "to another host.\n" );
		    break;
	    }
	} else if(( em_status == MSCP_ST_AVLBL ) &&
		  ( em_subcode == MSCP_SC_ALUSE )) {
	    printf( "mscp - Unit %d is online to another host.\n",up->unit );
	}
	new_event = EV_ONLERROR;
    }

    return( new_event );
}
/**/

/*
 *
 *   Name:	mscp_onlgtuntem - process GTUNT end message.
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	protect modification of the unit block
 *
 *   Return	NONE.
 *   Values:
 */

u_long 
mscp_onlgtuntem( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
    UNITB		*up = rp->unitb;
    u_long		new_event = EV_ONLCOMPLETE;
    u_short		em_status = mp->mscp_status & MSCP_ST_MASK;
    u_short		em_subcode = mp->mscp_status >> MSCP_ST_SBBIT;
	
    /* If the Get Unit Status command succeeded, store the unit
     * status in the unit block.
     */
    if( em_status == MSCP_ST_SUCC ) {
	Lock_unitb(up);
	Pad_msg( mp, rp->msgsize );
	up->mult_unt = mp->mscp_mult_unt;
	up->unt_flgs = mp->mscp_unt_flgs;
	up->unit_id = *( UNIQ_ID * )mp->mscp_unit_id;
	up->media_id = mp->mscp_media_id;
	up->shdw_unt = mp->mscp_shdw_unt;
	up->shdw_sts = mp->mscp_shdw_sts;
	up->track = mp->mscp_track;
	up->group = mp->mscp_group;
	up->cylinder = mp->mscp_cylinder;
	up->unit_svr = mp->mscp_unit_svr;
	up->unit_hvr = mp->mscp_unit_hvr;
	up->rct_size = mp->mscp_rct_size;
	up->rbns = mp->mscp_rbns;
	up->rct_cpys = mp->mscp_rct_cpys;
	up->flags.wrtp = (( mp->mscp_unt_flgs &
			 ( MSCP_UF_WRTPH |
			   MSCP_UF_WRTPS |
			   MSCP_UF_WRTPD )) != 0 );
	up->flags.rct_pres = ( mp->mscp_rct_cpys != 0 );
	up->tot_size = up->unt_size + ( up->rct_size * up->rct_cpys );

	/* If BBR is done by the host, the unit is write enabled, and the
	 * unit was not already online when we issued the most recent ONLINE
	 * command, call the BBR code to do ONLINE-time processing.
	 * Otherwise, redispatch with the (preset) online complete event.
	 */
	if( !( up->unt_flgs & MSCP_UF_REPLC ||
	       up->flags.wrtp ||
	       up->flags.alonl )) {
	    Unlock_unitb (up);
	    new_event = mscp_bbr_online( rp );
	    }
	    else Unlock_unitb (up);

    /* The Get Unit Status failed.  Recycle the RSPID and redispatch
     * to retry the ONLINE command.
     */
    } else {
	mscp_recycle_rspid( rp );
	new_event = EV_ONLERROR;
    }

    return( new_event );
}    
/**/

/*
 *
 *   Name:	mscp_markonline - Mark unit online.
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	protect modification of the unit block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long 
mscp_markonline( event, rp )
    u_long		event;
    REQB		*rp;
{
    UNITB		*up = rp->unitb;
    u_long		new_event = EV_NULL;
	
    Lock_unitb (up);
    up->flags.online = 1;
    up->flags.online_ip = 0;
    Unlock_unitb (up);
    mscp_dealloc_reqb( rp );

    return( new_event );
}

/**/

/*
 *
 *   Name:	mscp_availcm - Send an AVAIL command message
 *
 *   Abstract:	
 *
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	Protect the unit block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_availcm( event, rp )
    u_long		event;
    REQB		*rp;
{
    UNITB       	*up = rp->unitb;
    MSCP		*mp = rp->msgptr;
    u_long		new_event;

    /* Format the message buffer as a AVAILABLE command.  Queue the message
     * for transmission, then wait for the AVAILABLE end message.
     * If the unit is set to exclusive access mode then set the exclusive
     * modifier to hold onto the exclusive access operation.  This implies that
     * the only way to clear exclusive access is to issue an ioctl to clear
     * the excl_acc flag and then do an avail.  The other way exclusive access
     * is cleared is when the unit becomes inoperative, disabled or unknown.
     */
    Lock_unitb (up);
    Init_msg( mp, rp->rspid, rp->unitb->unit );
    mp->mscp_opcode = MSCP_OP_AVAIL;
    if ( up->flags.excl_acc ) {
        mp->mscp_modifier = MSCP_MD_EXCAC;
    }
    Unlock_unitb (up);
    new_event = mscp_send_msg( rp );
    return( new_event );
}
	
/**/

/*
 *
 *   Name:	mscp_markoffline - Mark a unit offline
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	protect modification of unit block
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_markoffline( event, rp )
    u_long		event;
    REQB		*rp;
{
    UNITB		*up = rp->unitb;
    CONNB               *cp = rp->connb;
    u_long		new_event = EV_NULL;

    /* Reset the online-in-progress bit, deallocate the request block
     * and exit with an EV_NULL event.
     */
    Lock_unitb (up);
    up->flags.online_ip = 0;
    up->flags.close_ip = 0;
    up->flags.online = 0;
    Unlock_unitb (up);

    /* If unit recovery is in progress: this unit did not come back 
     * online so document the failure (printf) and return EV_ONLERROR
     * to continue the unit recovery process.
     */
    if(( cp->flags.restart) && (rp->flags.perm_reqb)) {
        printf("\nDisk unit #%d failed unit recovery.\n",up->unit);
        return (EV_ONLERROR);
    }

    mscp_dealloc_reqb( rp );
    return( new_event );
}

/**/

int	print_cpu = 0;
int	cpucount[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
/*
 *
 *   Name:	mscp_transfercm - Send a READ/WRITE command message
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	Protect unit block while it is being accessed for the
 *		the data provided to the iostat utility.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_transfercm( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
    struct buf		*bp = rp->bufptr;
    UNITB		*up = rp->unitb;
    struct device	*devp = up->ubdev;

    Lock_unitb (up);
    /* set up stat data for iostat utilities.
     */
    if((int)devp->perf >= 0)
    {
	dk_xfer[(int)devp->perf]++;
	dk_wds[(int)devp->perf] += bp->b_bcount>>6;
    }
    Unlock_unitb (up);
/* TMP CPUDATA not in OSF 
	cpucount[CURRENT_CPUDATA->cpu_num]++;
	if (print_cpu > 0) {
		int	i;
		printf ("Transfer_cm: ");
		for (i=0; i < 32; i++)
			if (cpucount[i])
				printf ("cpu %d: %d\t", i, cpucount[i]);
		printf ("\n");
		print_cpu = 0;
		}
*/
    /* Format the message buffer as a READ/WRITE command and fill in
     * the transfer byte count, local buffer handle, and logical
     * block number.  Update the state and send the message, then
     * wait for the end message to come back.
     */
    Init_msg( mp, rp->rspid, rp->unitb->unit );
    mp->mscp_opcode = ( bp->b_flags & B_READ ) ?
			MSCP_OP_READ : MSCP_OP_WRITE;
    mp->mscp_byte_cnt = bp->b_bcount;
    Move_bhandle( rp->lbhandle, mp->mscp_buffer[ 0 ] ); 
    mp->mscp_lbn = rp->p1;
    return( mscp_send_msg( rp ));
	
}

/**/

/*
 *
 *   Name:	mscp_transferem - process a data transfer end message.
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	all set
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_transferem( event, rp )
    u_long		event;
    REQB		*rp;

{
    MSCP		*mp = rp->msgptr;
    struct buf		*bp = rp->bufptr;
    u_long		new_event = EV_NULL;
    u_short		em_status = mp->mscp_status & MSCP_ST_MASK;
    u_short		em_subcode = mp->mscp_status >> MSCP_ST_SBBIT;

    if( mp->mscp_flags & MSCP_EF_BBLKR ) {
	new_event = mscp_bbr_replace( rp );
    } else {

        /* If an error of any kind occurred during the data transfer,
         * mark the buf structure accordingly.
         */

        if( em_status != MSCP_ST_SUCC ) {
	    bp->b_flags |= B_ERROR;
	    bp->b_error = EIO;

	    /* temporary log force error here - pmk */

	    if (em_status == MSCP_ST_DATA && em_subcode == MSCP_SC_FRCER) {
		printf("ra%d%c: hard error sn %d\n",
		       Ux( bp->b_dev ),
		       'a' + ( u_char )Px( bp->b_dev ),
		       bp->b_blkno + (mp->mscp_byte_cnt / 512));
		printf("ra%d%c: Force Error Modifer Set: LBN %d\n",
		       Ux( bp->b_dev ),
		       'a' + ( u_char )Px( bp->b_dev ),
		       rp->p1 + (mp->mscp_byte_cnt / 512));
	    }
        }

        /* Save the bytes not transferred in the request block. (It can't
         * be saved in b_resid, because mapping information is stuffed in
         * there by the port driver.  I/O done is likewise deferred until
         * the buffer has been unmapped.) 
         * Deallocate the request block with all the resources that it 
         * holds and terminate the thread.
         */
        rp->p1 = bp->b_bcount - mp->mscp_byte_cnt;
	mscp_dealloc_reqb( rp );
    }

    return( new_event );
}

/**/

/*
 *
 *   Name:	mscp_accscancm - access command message
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	all set
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_accscancm( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;

    Init_msg( mp, rp->rspid, rp->unitb->unit );
    mp->mscp_opcode = MSCP_OP_ACCESS;
    mp->mscp_byte_cnt = rp->p2;
    mp->mscp_lbn = rp->p1;
    return( mscp_send_msg( rp ));
	
}

/**/

/*
 *
 *   Name:	mscp_accscanem - process an access end message.
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	protect modification of the unit block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_accscanem( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
    u_long		new_event = EV_NULL;
    UNITB		*up = rp->unitb;


    Lock_unitb (up);
    up->acc_status = mp->mscp_status;
    up->acc_flags = mp->mscp_flags;
    up->acc_badlbn = mp->mscp_lbn;
    up->acc_bytecnt = mp->mscp_byte_cnt;

    if( mp->mscp_flags & MSCP_EF_BBLKR ) {
	Unlock_unitb (up);
	new_event = mscp_bbr_replace( rp );
    }
    else {
        up->flags.force_scan_ip = 0;
	Unlock_unitb (up);
	mscp_dealloc_reqb( rp );
    }

    return( new_event );
}
/**/

/*
 *
 *   Name:	mscp_setunitcm - set unit command message
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	Protect accest to the unit block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_setunitcm( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
    UNITB		*up = rp->unitb;

    Lock_unitb (up);
    Init_msg( mp, rp->rspid, rp->unitb->unit );
    mp->mscp_opcode = MSCP_OP_STUNT;
    if ( up->flags.excl_acc ) 
        mp->mscp_modifier = MSCP_MD_EXCAC;
    Unlock_unitb (up);
    return( mscp_send_msg( rp ) );
	
}

/**/

/*
 *
 *   Name:	mscp_setunitem - process a set unit end message.
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	protect modification of the unit block
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_setunitem( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
    u_long		new_event = EV_NULL;
    UNITB		*up = rp->unitb;
    u_short		em_status = mp->mscp_status & MSCP_ST_MASK;
    u_short		em_subcode = mp->mscp_status >> MSCP_ST_SBBIT;

    Lock_unitb (up);
    /*
     * Check to see that the command succeeded.  A status of available with
     * a subcode of available implies success subcode normal when exclusive
     * access is granted while the unit was in the "Unit-Available" state.
     */
    if (( em_status == MSCP_ST_SUCC ) ||
	(( em_status == MSCP_ST_AVLBL ) && (em_subcode == MSCP_SC_AVAIL))) {
	/* Make sure the set of exclusive access succeeded.  If it does fail
	 * then clear the excl_acc flag so that the ioctl routine will know to
	 * return a failure status.
	 */
	if ( up->flags.excl_acc ) {
	    if (( mp->mscp_unt_flgs & MSCP_UF_EXACC ) == 0 ) {
	        up->flags.excl_acc = 0;
	    }
	}
	else {
	    if ( mp->mscp_unt_flgs & MSCP_UF_EXACC ) {
	        up->flags.excl_acc = 1;
	    }
	}
    }
    else {
	if ( up->flags.excl_acc ) {
	    up->flags.excl_acc = 0;
	}
	else {
	    up->flags.excl_acc = 1;
	}
    }
    up->flags.mscp_wait = 0;
    Unlock_unitb (up);
    return( new_event );
}


/**/

/*
 *
 *   Name:	mscp_recovinit - Initiate restoration of unit states.
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	protect access to the unit block and the connection block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_recovinit( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    UNITB		*up;
    u_long		new_event = EV_NULL;


    /* Find the first unit that was online when the connection dropped.
     */
    Lock_connb(cp);
    for( up = cp->unit.flink;
	 up != ( UNITB * )&cp->unit.flink;
	 up = up->flink )
	{
	Lock_unitb (up);
	if (up->flags.online) break;
	Unlock_unitb (up);
	}
    
    /* If we found a previously online unit, mark it offline with
     * online in progress, store the unit block pointer in the request
     * block and allocate a RSPID to use in the online sequence.
     */
    if( up != ( UNITB * )&cp->unit.flink ) {
	up->flags.online_ip = 1;
	up->flags.online = 0;
	rp->unitb = up;
	Unlock_unitb (up);
	Unlock_connb (cp);
	new_event = mscp_alloc_rspid( event, rp );

    /* No unit was online when the connection dropped; however, there
     * may have been an online request enqueued, so attempt to restart
     * the next request in the restart queue.
     */
    } else {
	Unlock_connb (cp);
	mscp_restart_next( cp );
	mscp_dealloc_all( rp );
    }

    return( new_event );
}
/**/

/*
 *
 *   Name:	mscp_recovnext - Mark current unit online and process next
 *
 *   Abstract:	
 *
 *   Inputs:	rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	protect access to the unit and connection blocks.
 *
 *   Return	NONE.
 *   Values:
 */

u_long 
mscp_recovnext( event, rp )
    u_long		event;
    REQB		*rp;
{
    UNITB		*up = rp->unitb;
    CONNB		*cp = rp->connb;
    u_long		new_event = EV_NULL;
	
    Lock_connb (cp);

    /* If this routine was entered as a result of successful completion
     * of the BBR algorithm or of the GTUNT command, set the unit online
     */
    if( event == EV_ERRECOV || event == EV_ONLCOMPLETE ) {
	Lock_unitb (up);
	up->flags.online = 1;
	up->flags.online_ip = 0;
	Unlock_unitb (up);
    }

    /* Find the next unit that was online when the connection dropped.
     */
    for( up = up->flink;
	 up != ( UNITB * )&cp->unit.flink;
	 up = up->flink )
	{
	Lock_unitb (up);
	if (up->flags.online) break;
	else Unlock_unitb (up);
	}
    
    /* If we found another previously online unit, mark it offline with
     * online in progress, store the unit block pointer in the request
     * block and redispatch to bring the unit online.
     */
    if( up != ( UNITB * )&cp->unit.flink ) {
	up->flags.online_ip = 1;
	up->flags.online = 0;
	rp->unitb = up;
	Unlock_unitb (up);
	Unlock_connb (cp);
	new_event = EV_ONLDONEXT;

    /* No more units were online.  Attempt to start the first request on
     * the restart queue, deallocate the resources held by the request
     * block and terminate the thread.
     */
    } else {
	Unlock_connb (cp);
	mscp_restart_next( cp );
	mscp_dealloc_all( rp );
    }

    return( new_event );
}
/**/

int	print_cpu_strat = 0;
int	cpucount_strat[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
/*
 *
 *   Name:	mscp_strategy - rtn description
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	Protect access to the unit block.
 *
 *   Return	NONE
 *   Values:
 */

int 
mscp_strategy( bp )
    struct buf		*bp;
{
    UNITB		*up;
    int			px;
    daddr_t		pstart;
#if	LABELS
    struct partition 	*pp;
    int			sz;
#else	/* LABELS */
    int			psize;
#endif	/* LABELS */
    int			errno = 0;
    int			s;

/* TMP CPUDATA not in OSF
	cpucount_strat[CURRENT_CPUDATA->cpu_num]++;
	if (print_cpu_strat > 0) {
		int	i;
		printf ("Strategy: ");
		for (i=0; i < 32; i++)
			if (cpucount_strat[i])
				printf ("cpu %d: %d\t", i, cpucount_strat[i]);
		printf ("\n");
		print_cpu_strat = 0;
	}
*/
    /* Check to make sure that the unit corresponding to the device number
     * exists and is online.  If not, return an error to the user.
     */
    up = Dev_to_unitb( bp->b_dev );
    if( up == NULL || ( !up->flags.online && !up->flags.online_ip )) {
	errno = ENXIO;
    } else {
        s = Splscs();
        Lock_unitb(up);
        px = Px( bp->b_dev );
    
    /* Get the partition offset and size from the current partition
     * table in the unit block.  If the partition size is specified
     * as -1, calculate the partition size as the number of blocks
     * from the start of the partition to the end of the user area.
     */
#if	LABELS
        pp = &up->disklabel.d_partitions[px];
        sz = bp->b_bcount >> DEV_BSHIFT;
        pstart = pp->p_offset;
        if((( bp->b_blkno + pstart ) <= LABELSECTOR ) &&
#if LABELSECTOR != 0
            (( bp->b_blkno + pstart + sz ) > LABELSECTOR ) &&
#endif
            /*
             * Avoid checking further if this is a zero-length partition,
             * we'll fall out and do POSIX check below.
             */
            ( pp->p_size != 0 ) &&
            (( bp->b_flags & B_READ) == 0) && 	/* this is a write */
             ( up->disklabel.d_magic == DISKMAGIC) &&   /* there is a label */
             ( up->wlabel == 0 )) {   		/* the label is protected */
                errno = EROFS;
        } else if(( bp->b_blkno < 0 ) || ( bp->b_blkno >= pp->p_size )) {
            /*
             * To conform with POSIX we will always return no error and
             * a resid count equal to the byte count requested for READs
             * outside of a partition.  For WRITEs outside a partition,
             * we will always return ENOSPC and the resid equal to the
             * byte count requested.
             */
            if (bp->b_flags & B_READ) {
                bp->b_resid = bp->b_bcount;
                iodone(bp);
                return;
            } else {
                errno = ENOSPC;
            }
        } else if(( bp->b_blkno + sz ) > pp->p_size ) {
            /*
             * A read or write request which spans a partition
             * is truncated to fit within a partition. 
             */
            bp->b_bcount = ( pp->p_size - bp->b_blkno ) << DEV_BSHIFT;
        }
#else	/* LABELS */
        /* If the partition table for the unit is not valid, panic.
         */
        if( up->part_info.pt_valid != PT_VALID ) {
	    Unlock_unitb(up);
	    splx(s);
	    panic( "mscp_strategy: invalid partition table\n" );
        } else {
	    pstart = up->part_info.pt_part[ px ].pi_blkoff;
	    if(( psize = up->part_info.pt_part[ px ].pi_nblocks ) == -1 )
	        psize = up->unt_size - pstart;

	    /* Ensure that the transfer lies entirely within the bounds of 
	     * the partition.  If so, allocate a request block and start the
	     * data transfer.  Otherwise return an error to the user.  (Note
	     * that the process sleeps until the request block is allocated.)
	     */
	    if( pstart < 0 && bp->b_blkno + 
			((bp->b_bcount + 511) >> 9) > psize ) {
                /*
                 * To conform with POSIX we will always return no error and
                 * a resid count equal to the byte count requested for READs
                 * outside of a partition.  For WRITEs outside a partition,
                 * we will always return ENOSPC and the resid equal to the
                 * byte count requested.
                 */
                if (bp->b_flags & B_READ) {
                    bp->b_resid = bp->b_bcount;
                    iodone(bp);
                    return;
                } else {
                    errno = ENOSPC;
                }
	    }
        }
#endif	/* LABELS */
        Unlock_unitb(up);
        splx(s);
    }
    /* If an error has been detected, set the error indicator, error
     * number and residual byte count in the buf structure and terminate 
     * the I/O operation.
     */ 
    if( errno ) {
	bp->b_error = errno;
	bp->b_flags |= B_ERROR;
        bp->b_resid = bp->b_bcount;
	( void )iodone( bp );
    } else {
	( void )mscp_alloc_reqb( up, bp, mscp_xfr_states,
				 pstart + bp->b_blkno, 0 );
    }
    return;
}
/**/

/*
 *
 *   Name:	mscp_bopen - Block mode open routine
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	all set
 *
 *   Return	NONE
 *   Values:
 */

int 
mscp_bopen( dev, flag )
    dev_t		dev;
    int			flag;
{
    return( mscp_open( dev, flag, 0 ));
}

/**/

/*
 *
 *   Name:	mscp_copen - Raw (character) mode open routine
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	all set
 *
 *   Return	NONE
 *   Values:
 */

int 
mscp_copen( dev, flag )
    dev_t		dev;
    int			flag;
{
    return( mscp_open( dev, flag, 1 ));
}

/**/

/*
 *
 *   Name:	mscp_getpt_info	- Get partition information 
 *
 *   Abstract:	This routine will attempt to get partition information
 *		from the disk device input.  We will first try to read
 *		an OSF/1-style disk label and if that fails, check to 
 *		see if an ULTRIX-style partition table is present.  If
 *		there is no partition information on the disk, we use
 *		the default partition table for the underlying device
 *		type. 
 *
 *   Inputs:
 *	dev --	The device major/minor number of the disk unit to check.
 *      up  --	A pointer to the unit block for the specified disk unit.
 *
 *   Outputs:
 *		The unit block will be updated with the correct 
 *		partition table information.
 *
 *   Return	
 *   Values:	None.
 */

void
mscp_getpt_info(dev, up)
    dev_t		dev;
    UNITB		*up;
{
    REQB		*rp;
    int			errno = 0;
    int			s;

#if     LABELS
    char 		*msg, *readdisklabel();
    struct disklabel 	*lp;
#endif
    int			i,error;

#if	LABELS
    lp = &up->disklabel;
    lp->d_magic = 0;

    /*
     * Setup some default values and call readdisklabel service
     * to attempt to read the OSF/1-style disk label.
     */
    lp->d_secsize = DEV_BSIZE;
    lp->d_secperunit = up->unt_size;
    lp->d_secpercyl = 1;
    lp->d_nsectors = lp->d_secperunit;
    lp->d_npartitions = 1;
    lp->d_partitions[0].p_size = lp->d_secperunit;
    lp->d_partitions[0].p_offset = 0;
	    
    msg = readdisklabel( makedev(major(dev),
                         MAKEMINOR(GETUNIT(dev),0)),
                         mscp_strategy, lp );
    s = Splscs();
    Lock_unitb(up);

    if( msg == NULL ) {
        /*
         * We've successfully read a disk label.  Copy the disk label
         * partition information into our unit block structure for
         * this unit and mark the partition valid.
         */
        for (i = 0; i < 8; i++) {
            up->part_info.pt_part[i].pi_nblocks =
            lp->d_partitions[i].p_size;
            up->part_info.pt_part[i].pi_blkoff =
                lp->d_partitions[i].p_offset;
        }
        up->part_info.pt_valid = PT_VALID;
        up->def_labelinfo = 0;
    }else{
        /*
         * There is no disk label.  Log a message to the user.
         * Next, set up a default partition table that will let
         * us get through mscp_strategy() so we can attempt a read to
         * see if there is an ULTRIX-style partition table on
         * this disk unit.  If extended label info (log_label_info)
         * is off, don't log a message.
         */
        if (MSCP_log_label_info){
            log(LOG_ERR,
                "ra%d: error reading disk label -- %s\n",
                 up->ubdev->logunit, msg);
         } 
	 lp->d_magic = 0;

         /*
          * Check if we have a valid ULTRIX or default partition table.
          * If we have a valid in-memory partition table continue, otherwise
          * attempt to read ULTRIX-style partition table and if one doesn't
          * exist, use the default partition table for this device.
          */
         if( !up->part_info.pt_valid ) {
             mscp_getdefpt( up->media_id, ( struct pt * )&up->part_info );
             up->part_info.pt_valid = 1;
             Unlock_unitb(up);
             splx(s);
             error = rsblk( mscp_strategy,dev,( struct pt *)&up->part_info);
             s = Splscs();
             Lock_unitb(up);

             if (error){
                 /*
                  * No ULTRIX-style partition info found on this disk, so
                  * let the user know we'll be using defaults for this
                  * disk (based on its disk type).  If extended label
                  * info (MSCP_log_label_info) is off, don't log a message.
                  */
                 if (MSCP_log_label_info) {
                    log(LOG_ERR,
                    "ra%d: no partition info on disk - using default values.\n",
                     up->ubdev->logunit);
                 } 
                 up->def_labelinfo = DEF_LABELINFO;
             }else{
                 /*
                  * NOTE: We'll still spew a message about ULTRIX file systems,
                  * just so the user doesn't accidentally trash their disk.
                  * Eventually, this should be put under "MSCP_log_label_info"
                  * also...
                  */
                 log(LOG_ERR,
                    "ra%d: using ULTRIX partition info found on disk.\n",
                    up->ubdev->logunit);
                 up->def_labelinfo = 0;
             }

             /*
              * If no Ultrix partition table is found, the default
              * values from mscp_data.c are undisturbed, and become
              * the active partition info.  Here we copy those defaults,
              * or the ULTRIX partition info found, to the in-memory
              * disk label.
              */
             lp->d_npartitions = 8;
             for (i = 0; i < 8; i++) {
                 int size = up->part_info.pt_part[i].pi_nblocks;
                 if (size == -1) {
                     size = lp->d_secperunit;
                     size -= up->part_info.pt_part[i].pi_blkoff;
                 }
                 lp->d_partitions[i].p_size = size;
                 lp->d_partitions[i].p_offset =
                     up->part_info.pt_part[i].pi_blkoff;
            }
        }
    }

  
#else	/* LABELS */

/*
 * NOTE: Non-label code is being placed here though for all practical
 *       purposes, the system cannot be built without label support.
 *       This is being retained as a historical reference for now.
 */
    s = Splscs();
    Lock_unitb(up);

    if( !up->part_info.pt_valid ) {
        mscp_getdefpt( up->media_id, ( struct pt * )&up->part_info );
        up->part_info.pt_valid = 1;
        Unlock_unitb(up);
        splx(s);
        error = rsblk( mscp_strategy,dev,( struct pt *)&up->part_info);
        s = Splscs();
        Lock_unitb(up);

        if (error){
            /*
             * No ULTRIX-style partition info found on this disk, so
             * let the user know we'll be using defaults for this
             * disk (based on its disk type).  If extended label
             * info (MSCP_log_label_info) is off, don't log a message.
             */
             if (MSCP_log_label_info) {
                 log(LOG_ERR,
                 "ra%d: no partition info on disk - using default values.\n",
                  up->ubdev->logunit);
             } 
             up->def_labelinfo = DEF_LABELINFO;
        }else{
             up->def_labelinfo = 0;
             }
    }

#endif	/* LABELS */

    Unlock_unitb(up);
    splx(s);

    return;
}


/**/

/*
 *
 *   Name:	mscp_open	- Open Disk Unit
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	protect the unit block mod that is synchronized with SCS
 *
 *   Return	NONE
 *   Values:
 */

int
mscp_open(dev, flag, fmt)
    dev_t		dev;
    int			fmt,flag;
{
    UNITB		*up;
    REQB		*rp;
    int			errno = 0;
    int			s, old_part_mask;

    /* If there is no unit block corresponding to the device number,
     * wait for the system devices to be configured.  If the unit
     * still cannot be found, return an error.
     */
    if(( up = Dev_to_unitb( dev )) == NULL  &&
       ( up = mscp_check_sysdev( dev )) == NULL ) {
	    errno = ENXIO;

    /* If the unit is not online, start a thread to bring it online
     * and wait for the thread to complete.
     */
    } else {
    	s = Splscs();
	while( 1 ) {
	    Lock_unitb(up);
	    if( !up->flags.busy ) {
		up->flags.busy = 1;
		break;
	    } else {
		/* needs to be a sleep_unlock for SMP */
		sleep((caddr_t)up, PSWP+1 );
	    }
	}

	if( !up->flags.online ) {
	    if( !up->flags.online_ip ) {
		up->flags.online_ip = 1;
		Unlock_unitb(up);
		splx(s);
		rp = ( REQB * )mscp_alloc_reqb( up, NULL, mscp_onl_states, 0, 0 );
	    } else {
		Unlock_unitb(up);
		splx(s);
	    }
	    while( up->flags.online_ip ) {
		timeout( wakeup, (caddr_t)up, 3 * hz );
		sleep((caddr_t)up, PSWP+1 );
		untimeout( wakeup, (caddr_t)up );
	    }
	    s = Splscs();
	    Lock_unitb(up);
	}

	/* If the device is now online, bump the open count and update
	 * the partition information if necessary.
	 */
	if( up->flags.online ) {
	/*
	 * Device is online: we'll first attempt to read an OSF/1-style
	 * disklabel.  If that fails, we will next attempt to read an
	 * ULTRIX-style partition table.  If that fails, we will default
	 * to a compiled in (default) partition table.
	 */
	    old_part_mask = up-> part_mask;
	    up->part_mask |= ( 1 << (( fmt ? 8 : 0 ) + Px( dev )));
	    /*
	     * Only read in the disk's disk label if this is the first
	     * open on the device.   
	     */
	    if (old_part_mask == 0) {
	        up->part_info.pt_valid = 0;
	        Unlock_unitb(up);
	        splx(s);
	        mscp_getpt_info( dev, up );
	        s = Splscs();
	        Lock_unitb(up);
	    }
	/*
	 * If the device could not be brought online and the NDELAY 
	 * flag is not set, return an error.
	 */
	} else if(( flag & O_NDELAY ) == 0 ) {
	    errno = ENXIO;
	}

        up->flags.busy = 0;
	Unlock_unitb(up);
	splx(s);
	wakeup(( caddr_t )up );
    }
    
    /* Return status to the caller.
     */
    return( errno );
}

/**/

/*
 *
 *   Name:	mscp_bclose - Block mode close routine
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	all set
 *
 *   Return	NONE
 *   Values:
 */

int 
mscp_bclose( dev, flag )
    dev_t		dev;
    int			flag;
{
    return( mscp_close( dev, flag, 0 ));
}

/**/

/*
 *
 *   Name:	mscp_cclose - Raw (character) mode close routine
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	all set
 *
 *   Return	NONE
 *   Values:
 */

int 
mscp_cclose( dev, flag )
    dev_t		dev;
    int			flag;
{
    return( mscp_close( dev, flag, 1 ));
}

/**/

/*
 *
 *   Name:	mscp_close	- Close Disk Unit
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	protect mod to the unit block which is synchronized with SCS.
 *
 *   Return	NONE
 *   Values:
 */

int 
mscp_close(dev, flag, raw )
    dev_t		dev;
    int			flag;
    int			raw;
{

    UNITB		*up;
    REQB		*rp;
    int			s;
    struct device	*devp;

    /* If there is no known unit corresponding to the device
     * number, return an error.
     */
    if(( up = Dev_to_unitb( dev )) == NULL ) {
	return( ENXIO );
    }

    s = Splscs();
    while( 1 ) {
	Lock_unitb(up);
	if( !up->flags.busy ) {
	    up->flags.busy = 1;
	    break;
	} else {
	    /* needs to be a sleep_unlock for SMP */
	    sleep((caddr_t)up, PSWP+1 );
	}
    }

    /* Overwrite default throughput constant so it reflects the
     * correct number of sectors per track
     */
    if(up->track){
	devp = up->ubdev;
        if (((int)devp->perf >= 0) && (up->track != 0))
        {   /* assume 60 revs */
#ifdef __vax
	    dk_mspw[(int)devp->perf] = 1.0 / ( 60 * up->track * 256); 
#else
	    dk_wpms[(int)devp->perf] = 0;  /* OSF no */
#endif /* __vax */
/*
	    dk_wpms[(int)devp->perf] = ( 60 * up->track * 256); 
*/
	}
    }

    /* Reduce the open outstanding counter.  If no
     * active partitions remain, and the unit is online, set the unit
     * available.
     */
    up->part_mask &= ~( 1 << (( raw ? 8 : 0 ) + Px( dev )));
    if( up->part_mask == 0 && up->flags.online ) {
	up->flags.close_ip = 1;
	Unlock_unitb(up);
	splx(s);
	rp = ( REQB * )mscp_alloc_reqb( up, NULL, mscp_avl_states, 0, 0 );
	while( up->flags.close_ip ) {
    	    timeout( wakeup, ( caddr_t )rp, 3 * hz );
    	    sleep(( caddr_t )rp, PSWP+1 );
 	    untimeout( wakeup, ( caddr_t )rp );
	}
	s = Splscs();
	Lock_unitb(up);

	up->wlabel = 0;
	up->part_info.pt_valid = 0;
    }

    up->flags.busy = 0;
    Unlock_unitb(up);
    splx(s);

    wakeup(( caddr_t )up );

    return( 0 );
}

/**/

/*
 *
 *   Name:	mscp_read - 
 *
 *   Abstract:	Start a raw I/O, using the unit block's buffer
 *		which is named rawbuf, and which was allocated
 *		specifically for this purpose; the I/O is started
 *		by calling physio.
 *
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Since there is only one buffer per unit for raw I/O,
 *		we must SMP synchronize access to that buffer.  We
 *		use a flag in the unit block, rawb_busy, for the
 *		synchronization.  If rawb_busy is nonzero we sleep,
 *		using the &rawb_busy as the sleep channel.  After
 *		returning from physio, we do a wakeup.
 *
 *   Return
 *   Values:	returns what physio returns.
 */

int 
mscp_read( dev, uio )
    dev_t		dev;
    struct uio		*uio;
{
    UNITB		*up;
    int			status, s;

    /* If there is no known unit corresponding to the device
     * number, return an error.
     */
    if(( up = Dev_to_unitb( dev )) == NULL ) {
	return( ENXIO );
    }

    /* Wait for the unit's raw I/O buffer to be ready */
    s = Splscs();
    while( 1 ) {
	Lock_unitb(up);
	if( !up->rawb_busy ) {
	    up->rawb_busy = -1;
	    break;
	} else {
	    /* needs to be a sleep_unlock for SMP */
	    sleep((caddr_t)&((up)->rawb_busy), PSWP+1 );
	}
    }
    Unlock_unitb(up);
    splx(s);

    /* Invoke physio to fire off the strategy routine and return
     * the resulting status.
     */
    status = physio( mscp_strategy, &up->rawbuf, dev, B_READ, 
                     mscp_minphys, uio );

    s = Splscs();
    Lock_unitb(up);
    up->rawb_busy = 0;
    Unlock_unitb(up);
    splx(s);

    wakeup((caddr_t)&((up)->rawb_busy));

    return( status );
}
/**/

/*
 *
 *   Name:	mscp_write - 
 *
 *   Abstract:	Start a raw I/O, using the unit block's buffer
 *		which is named rawbuf, and which was allocated
 *		specifically for this purpose; the I/O is started
 *		by calling physio.
 *
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Since there is only one buffer per unit for raw I/O,
 *		we must SMP synchronize access to that buffer.  We
 *		use a flag in the unit block, rawb_busy, for the
 *		synchronization.  If rawb_busy is nonzero we sleep,
 *		using the &rawb_busy as the sleep channel.  After
 *		returning from physio, we do a wakeup.
 *
 *   Return
 *   Values:	returns what physio returns.
 */

int 
mscp_write( dev, uio )
    dev_t		dev;
    struct uio 		*uio;
{
    UNITB		*up;
    int			status, s;

    /* If there is no known unit corresponding to the device
     * number, return an error.
     */
    if(( up = Dev_to_unitb( dev )) == NULL ) {
	return( ENXIO );
    }

    /* Wait for the unit's raw I/O buffer to be ready */
    s = Splscs();
    while( 1 ) {
	Lock_unitb(up);
	if( !up->rawb_busy ) {
	    up->rawb_busy = -1;
	    break;
	} else {
	    /* needs to be a sleep_unlock for SMP */
	    sleep((caddr_t)&((up)->rawb_busy), PSWP+1 );
	}
    }
    Unlock_unitb(up);
    splx(s);

    /* Invoke physio to fire off the strategy routine and return
     * the resulting status.
     */
    status = physio( mscp_strategy, &up->rawbuf, dev, B_WRITE, 
                     mscp_minphys, uio );

    s = Splscs();
    Lock_unitb(up);
    up->rawb_busy = 0;
    Unlock_unitb(up);
    splx(s);

    wakeup((caddr_t)&((up)->rawb_busy));

    return( status );
}
/**/

/*
 *
 *   Name:	mscp_ioctl	- Process I/O Control Functions
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the unit block.
 *
 *   Return	NONE
 *   Values:
 */

/* TODO - check for possible races between get/set partition tables
 * and a unit becoming available or online.
 */

int
mscp_ioctl(dev, cmd, data, flag)
    dev_t		dev;
    u_int		cmd;
    caddr_t		data;
    int			flag;
{
    UNITB		*up;
    int			s;
#if	LABELS
    struct disklabel 	*lp;
#endif	/* LABELS */

    /* If there is no known unit corresponding to the device
     * number, return an error.
     */
    if(( up = Dev_to_unitb( dev )) == NULL ) {
	return( ENXIO );
    }

#if	LABELS
    lp = &up->disklabel;
#endif	/* LABELS */

    switch( cmd ) {

#ifdef 0
/*
 * ULTRIX-style partitioning is read-only with OSF/1: the old style
 * partition ioctls should not be used, OSF/1 label support should be
 * used.  Currently just saving this code for historical reasons.
 */

  
    /* Get the current or default partition table values for the device
     * 'dev' and store them in the buffer pointed to by 'data'.
     */
    case DIOCGETPT:
    case DIOCDGTPT:

	{
	    struct pt	*ptp = ( struct pt * )data;
	    int	i;


	    s = Splscs();
	    Lock_unitb(up);

	    /* If the current partition table values are wanted, use a structure
	     * copy to get them into the user buffer.
	     */
	    if( cmd == DIOCGETPT ) {
		*ptp = up->part_info;
	    }

	    /* Get the default partition table values based on the storage
	     * medium currently loaded in the device.
	     */
	    else {
		mscp_getdefpt( up->media_id, ptp );
	    }

	    /* Go through the user buffer, changing any partition sizes that
	     * were specified as -1 to the actual partition size, calculated as
	     * the size of the user-accessible portion of the disk minus the
	     * starting LBN of the partition.
	     */
	    for( i = 0; i <= 7; i++ )
		if( ptp->pt_part[ i ].pi_nblocks == -1 )
		    ptp->pt_part[ i ].pi_nblocks = 
			up->unt_size - ptp->pt_part[ i ].pi_blkoff;

	    Unlock_unitb(up);
	    splx(s);
	    return( 0 );
	}
	    
    /* Replace the current partition table for the device with the contents
     * of the user supplied buffer (after appropriate checking).
     */
    case DIOCSETPT:

	{
	    struct pt	*ptp = ( struct pt * )data;
	    int			error;

	    /* If the caller is not the super-user, return a permission
	     * denied (EACCES) error.
	     */
#if SEC_BASE
	    if( !privileged(SEC_FILESYS, 0))
#else
	    if( !suser() )
#endif
		return( EACCES );

	    s = Splscs();
	    Lock_unitb(up);

	    /* If the user-supplied partition values conflict with the
	     * active partition table, return an error.
	     */
	    if( ( error = ptcmp( dev, &up->part_info, ptp )) != 0 ) {
		Unlock_unitb(up);
		splx(s);
		return( error );
		}
	    
	    /* Set the partition tables in the UNITB using the data in the
	     * user's buffer, update the superblock in the 'a' partition
	     * if necessary, set the partition table valid bit, and return
	     * success to the caller.
	     */
	    up->part_info = *ptp;
	    Unlock_unitb(up);
	    splx(s);
	    ( void )ssblk( dev, ptp );
	    up->part_info.pt_valid = PT_VALID;
	    return( 0 );
	}    
#endif /* ifdef 0 */

#if     LABELS
    case DIOCGDINFO:
	    { 
	    int error;
	    /* If this disk doesn't have a label, pretend that we don't
	     * implement them. 
	     */
	    if( lp->d_magic != DISKMAGIC ) {
	        error = EINVAL;
	    } else {
	        *(struct disklabel *)data = *lp;
		error = 0;
	    }
	    return( error );
	    }

    case DIOCGPART:
	    {
	    int error;
	    if( lp->d_magic != DISKMAGIC ) {
	        error = EINVAL;
	    } else {
	        ((struct partinfo *)data)->disklab = lp;
	        ((struct partinfo *)data)->part = &lp->d_partitions[Px(dev)];
		error = 0;
	    }
	    return( error );
	    }

    case DIOCSDINFO:
	    {
	    int	error;
#if     SEC_BASE
	    if( !privileged(SEC_FILESYS, 0))
#else
	    if( suser(u.u_cred, &u.u_acflag))
#endif
	        error = EACCES;

	    if((flag & FWRITE) == 0) {
	        error = EBADF;
	    } else {
	        error = setdisklabel( lp, (struct disklabel *)data, 
			( up->def_labelinfo ? 0 : 
			( up->part_mask >> 8 | up->part_mask & 0xff )));
	    }
	    return( error );
	    }
    case DIOCWLABEL:
	    {
	    int	error;
#if     SEC_BASE
	    if (!privileged(SEC_FILESYS, 0))
#else
	    if( suser(u.u_cred, &u.u_acflag) )
#endif
	        error = EACCES;

	    if ((flag & FWRITE) == 0) {
	        error = EBADF;
	    } else {
	        up->wlabel = *(int *)data;
		error = 0;
	    }
	    return( error );
	    }

    case DIOCWDINFO:
	    {
	    int	error;
	    if ((flag & FWRITE) == 0) {
	        error = EBADF;
	    } else if(( error = setdisklabel( lp, (struct disklabel *)data,
		     ( up->def_labelinfo ? 0 : 
		     ( up->part_mask >> 8 | up->part_mask & 0xff )))) == 0 ) {
	            int wlab;

		    up->def_labelinfo = 0;
	            wlab = up->wlabel;
	            up->wlabel = 1;
	            error = writedisklabel(dev, mscp_strategy, lp);
	            up->wlabel = wlab;
            }
	    return( error );
	    }

    case DIOCGDEFPT:
	    {
    	    struct pt def;
	    struct pt *ptd = (struct pt *)&def;
	    struct pt_tbl *ptp = (struct pt_tbl *)data;
	    int i,size;

	    mscp_getdefpt( up->media_id, ptd );

	    /*
	     * We walk through the default partition for this device
	     * and copy size and offset of each supported partition.
	     */
	    for(i=0; i<8; i++)   {
	        size = ptd->pt_part[i].pi_nblocks;
	        if(size == -1)  {
	            size = up->unt_size;
	            size -= ptd->pt_part[i].pi_blkoff;
	        }
	        ptp->d_partitions[i].p_size = size;
	        ptp->d_partitions[i].p_offset =
	            ptd->pt_part[i].pi_blkoff;
	    }
	    return( 0 );
	    }

    case DIOCGCURPT:
	    {
	    *(struct pt_tbl *)data = *(struct pt_tbl *)lp->d_partitions;
	    return( 0 );
	    }

#endif  /* LABELS */


    case DEVIOCGET:
	{
	    struct devget	*dp = ( struct devget * )data;
	    struct device	*devp = up->ubdev;
	    CONNB		*cp = up->connb;

	    s = Splscs();
	    Lock_unitb(up);

	    bzero( dp, sizeof( struct devget ));
	    dp->category = DEV_DISK;
	    dp->bus = cp->bus_type;
	    bcopy( cp->model_name, dp->interface, strlen( cp->model_name ));
	    mscp_media_to_ascii( up->media_id, dp->device );
/*
	    dp->adpt_num = ui->ui_adpt;
	    dp->nexus_num = ui->ui_nexus;
*/
	    dp->bus_num = devp->ctlr_hd->bus_num;
	    dp->rctlr_num = devp->ctlr_hd->rctlr;
	    dp->ctlr_num = devp->ctlr_num;
	    dp->slave_num = up->unit;
	    bcopy( devp->dev_name, dp->dev_name, strlen( devp->dev_name ));
	    dp->unit_num = devp->logunit;
	    dp->soft_count = 0;
	    dp->hard_count = 0;
	    dp->stat = 0;
	    if( up->flags.online == 0 )
		dp->stat |= DEV_OFFLINE;
	    if( up->flags.wrtp == 1 )
		dp->stat |= DEV_WRTLCK;
	    dp->category_stat = GETDEVS(dev);

	    Unlock_unitb(up);
	    splx(s);
	    return( 0 );
	}

    /* Ioctl to obtain device geometry information.
     */
    case DEVGETGEOM:
	{
	    DEVGEOMST	*devgeom = ( DEVGEOMST * )data;
	    int calcs, ncyl;

	    bzero( devgeom, sizeof( DEVGEOMST ));
	    s = Splscs();
	    Lock_unitb(up);
	    calcs = up->group * up->cylinder;
	    devgeom->geom_info.dev_size = up->unt_size;
	    devgeom->geom_info.ntracks = calcs;
	    devgeom->geom_info.nsectors = up->track;
	    calcs *= up->track;
	    if (calcs > 0) {
	    	ncyl = up->unt_size / calcs;
	    	if (up->unt_size % calcs)
			ncyl++;   /* round up */
	    }
	    else {
		ncyl = 0;
	    }
	    devgeom->geom_info.ncylinders = ncyl;
	    if (up->unt_flgs & MSCP_UF_RMVBL)
		devgeom->geom_info.attributes |= DEVGEOM_REMOVE;
	    /*
	     * HSX00 and HSX01 FIB RAID devices are flagged as having
	     * "dynamic geometry" because the geometry of the underlying
	     * device can change depending on the configuration of units.
	     */
	    if ((up->media_id==HSX00_MEDIA) || (up->media_id==HSX01_MEDIA)) 
		devgeom->geom_info.attributes |= DEVGEOM_DYNAMIC;

	    Unlock_unitb(up);
	    splx(s);
	    return( 0 );
	}

    /* Ioctl used by radisk(8) to scan the disk or force a replacement
     */
  case DKIOCACC:
	{
	struct dkacc *dkacc = (struct dkacc *)data;
        REQB	*rp;
        long 	totbytes;
        long	length;
        long	lbn;

	/* Only super users can beat on the pack
	 */
#if     SEC_BASE
	if( !privileged(SEC_FILESYS, 0))
#else
	if( suser(u.u_cred, &u.u_acflag))
#endif
	    return(EACCES);

	s = Splscs();
	Lock_unitb(up);

	switch (dkacc->dk_opcode) {
	case ACC_REVEC:
	    /* Return if the controller does the work
	     */
            if(( up->unt_flgs & MSCP_UF_REPLC || up->flags.wrtp )) {
		Unlock_unitb(up);
		splx(s);
		return(0);
	    }
	    if( up->flags.force_scan_ip == 1 ) {
		Unlock_unitb(up);
		splx(s);
	        return(EBUSY);
	    }
	    up->flags.force_scan_ip = 1;
	    Unlock_unitb(up);
	    splx(s);
	    /* Force LBN  to be re-vectored
	     */
	    rp = (REQB *)mscp_alloc_reqb( up, NULL, mscp_repl_states,
			     		  dkacc->dk_lbn, 0 );

	    while( up->flags.force_scan_ip ) {
    		timeout( wakeup, ( caddr_t )rp, 5 * hz );
    		sleep((caddr_t)rp, PSWP+1 );
 		untimeout( wakeup, ( caddr_t )rp );
	    }

	    s = Splscs();
	    Lock_unitb(up);
	    dkacc->dk_status = 0;	
	    break;

	case ACC_SCAN:
	    if( dkacc->dk_lbn > up->unt_size ) {
    	        Unlock_unitb(up);
    	        splx(s);
	        return(EINVAL);
	    }

	    lbn = dkacc->dk_lbn;
	    totbytes = 0;

	    if( up->connb->max_bcnt == 0 )
	        up->connb->max_bcnt = 16777216;

	    if( up->flags.force_scan_ip == 1 ) {
	        Unlock_unitb(up);
	        splx(s);
	        return(EBUSY);
	    }

	    while( dkacc->dk_length ) {

	        up->flags.force_scan_ip = 1;

		length = (dkacc->dk_length > up->connb->max_bcnt)
	         	? up->connb->max_bcnt : dkacc->dk_length;

		Unlock_unitb(up);
		splx(s);
	        rp = (REQB *)mscp_alloc_reqb( up, NULL, 
				mscp_accscan_states, lbn, length);

	        while( up->flags.force_scan_ip ) {
    		    timeout( wakeup, ( caddr_t )rp, 5 * hz );
		    sleep((caddr_t)rp, PSWP+1 );
    		    untimeout( wakeup, ( caddr_t )rp );
	        }
		s = Splscs();
		Lock_unitb(up);

		totbytes += up->acc_bytecnt;

		if (up->acc_status != MSCP_ST_SUCC) 
		    break;
		
		lbn += btodb(up->acc_bytecnt);
		dkacc->dk_length -= up->acc_bytecnt;
	    }

    	    dkacc->dk_status = up->acc_status;
    	    dkacc->dk_flags = up->acc_flags;
    	    dkacc->dk_lbn = up->acc_badlbn;
    	    dkacc->dk_length  = totbytes;
	    
	    break;

	default:
	    break;	
  	}

	Unlock_unitb(up);
	splx(s);

	return( 0 );
	}
    /* Ioctl used by radisk(8) to control the exclusive access attribute.
     */
  case DKIOCEXCL:
	{
	    int *action = (int *)data;
            REQB	*rp;

	    /* Only super users can modify this attribute.
	     */
#if     SEC_BASE
	    if( !privileged(SEC_FILESYS, 0))
#else
	    if( suser(u.u_cred, &u.u_acflag))
#endif
	        return(EACCES);

	    s = Splscs();
	    Lock_unitb(up);

	    /* Set the up->flags.excl_acc flag to be the specified mode of
	     * exclusive access.  Next call the set unit characteristics state
	     * table.  When that completes if the value of excl_acc changes it
	     * means that the operation failed.
	     */

	    if( *data == 0 ) {	/* clear */
		if( up->flags.excl_acc == 0 ) {
		    Unlock_unitb(up);
		    splx(s);
		    return(EIO);
		}
		up->flags.excl_acc = 0;
	    } else {		/* set */
		up->flags.excl_acc = 1;
	    }
	    /* Force a set unit characteristics to set exclusive mode.
	     */
	    if( up->flags.mscp_wait == 1 ) {
		Unlock_unitb(up);
		splx(s);
	        return(EBUSY);
	    }
	    up->flags.mscp_wait = 1;

	    Unlock_unitb(up);
	    splx(s);
	    rp = (REQB *)mscp_alloc_reqb( up, NULL, mscp_stu_states, 0, 0);

	    while( up->flags.mscp_wait ) {
    	 	timeout( wakeup, ( caddr_t )rp, 5 * hz );
    	 	sleep((caddr_t)rp, PSWP+1 );
 	 	untimeout( wakeup, ( caddr_t )rp );
	    }
	    s = Splscs();
	    Lock_unitb(up);

	    if( *data == 0 ) {			/* clear failed */
		if( up->flags.excl_acc ) {
			Unlock_unitb(up);
			splx(s);
			return(EIO);
		}
	    } else {
		if( up->flags.excl_acc == 0 ) {
			Unlock_unitb(up);
			splx(s);
			return(EIO);
		}
	    }
	    Unlock_unitb(up);
	    splx(s);
	    return( 0 );
	}

    default:
	return( 0 );

    }
}
/**/

/*
 *
 *   Name:	mscp_forcecm - Start a forced replace
 *
 *   Abstract:	This function calls the bbr_force() routine
 *		to force a replacement.  This function is entered
 *		as a result of an radisk(8) request.
 *
 *
 *   Inputs:	event			Event code.
 *		rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	all set
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_forcecm( event, rp )
    u_long		event;
    REQB		*rp;
{
    MSCP		*mp = rp->msgptr;
	
    mp->mscp_lbn = rp->p1;
    mp->mscp_status = 0;
    return( mscp_bbr_force( rp ) );
}

/**/

/*
 *
 *   Name:	mscp_forceem - Cleanup afer a forced replace
 *
 *   Abstract:	This function deallocates the resources allocated for
 *		the replacement and clears the force_scan_ip bit to indicate
 *		the replacement is complete.  The ioctl() function is
 *		spinning on this bit. The access scan function is also
 *		spinning on this bit after a bad block replace.
 *
 *
 *   Inputs:	event			Event code.
 *		rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	Protect access to the unit block.
 *
 *   Return	NONE.
 *   Values:
 */

u_long
mscp_forceem( event, rp )
    u_long		event;
    REQB		*rp;
{
    UNITB		*up = rp->unitb;

    Lock_unitb (up);
    up->flags.force_scan_ip = 0;
    Unlock_unitb (up);
    mscp_dealloc_reqb( rp );
    return( EV_NULL);	
}

/**/

/*
 *
 *   Name:	mscp_size - Find the size of a partition.
 *
 *   Abstract:	This routine returns the size of the partition specified
 *		by a given device number.
 *
 *   Inputs:	dev			Device number.
 *
 *   Outputs:	NONE
 *
 *   SMP:	Protect unit block.
 *
 *   Return	Size of partition specified by dev.
 *   Values:
 */

int 
mscp_size( dev )
    dev_t		dev;
{
    int			part_size = -1; 	/* Default error */
    UNITB		*up;
    int			px = Px( dev );
    int			s;

    /* If the unit index is greater than the assembled-in maximum,
     * or if there is no unit in the configuration corresponding to
     * the unit index, return -1.
     */
    up = Dev_to_unitb( dev );

    s = Splscs();
    Lock_unitb(up);
    if( up != NULL && up->flags.online ) {

#if     LABELS
        if (px < up->disklabel.d_npartitions) {
    	    part_size = up->disklabel.d_partitions[px].p_size;
        }
#else	/* LABELS */
        /* As a sanity check, panic if the partition table information
         * is not marked valid.
         */
        if( up->part_info.pt_valid != PT_VALID ) {
	    Unlock_unitb(up);
	    splx(s);
	    panic( "mscp_size: invalid partition table\n" );
	}

    /* If the actual size of the partition is specified (not -1), return it
     * to the caller.  Otherwise, calculate the partition size as the size of
     * the user-accessible area of the disk minus the starting LBN of the
     * partition, and return it to the caller.
     */
        if(( part_size = up->part_info.pt_part[ px ].pi_nblocks ) == -1 ) {
	   part_size = ( up->unt_size - up->part_info.pt_part[ px ].pi_blkoff );
        }
#endif /* LABELS */
    }
    if( up != NULL ) Unlock_unitb(up);
    splx(s);
    return( part_size );
}
/**/

/*
 *
 *   Name:	mscp_getdefpt - get default partition information.
 *
 *   Abstract:	Find the partition information corresponding to the
 *		input media ID and store it in a user-provided
 *		partition structure.
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	all set here, but the caller may have locked the relevent
 *		unit block.
 *
 *   Return	NONE
 *   Values:
 */

void 
mscp_getdefpt( media_id, ptp )
    u_long		media_id;
    struct pt		*ptp;
{
    int			i;
    PART_SIZE		*psp;

    /* Look up the media ID in the disk media table.  If it isn't found, use
     * reserved entry 0 as the default media type.
     */
    for( i = dmscp_media_ct - 1;
	 i > 0 && dmscp_media[ i ].media_id != media_id;
	 i-- )
	;

    /* Move the default partition values for the device (medium) into
     * the user's partition structure.
     */
    psp = dmscp_media[ i ].part_sizes;
    for( i = 0; i <= 7; i++, psp++ ) {
	ptp->pt_part[ i ].pi_nblocks = psp->p_nblocks;
	ptp->pt_part[ i ].pi_blkoff = psp->p_blkoff;
    }
    return;
}

/**/

/*
 *
 *   Name:	mscp_check_sysdev - check/wait for system device availability.
 *
 *   Abstract:	A requested device has been found to be unconfigured.
 *		Check to see if the device is the root, swap or dump
 *		device specified in the configuration file and if so,
 *		wait for the device to become available.  If the device
 *		is none of the foregoing, return an ENXIO error.
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	all set here.   Note that swdevt, which describes the
 *		swap device, is global symbol which is readonly; and so
 *		access to it is not locked.
 *
 *   Return	NONE
 *   Values:
 */

UNITB *
mscp_check_sysdev( dev )
    dev_t		dev;
{
    UNITB		*up = NULL;
    int			retries = 120;
    dev_t		dev_min, dev_maj;
#ifndef	OSF
    struct swdevt	*sp;

    /* If the given device is one of the swap devices or is
     * the root or dump device, wait for it to be configured.
     */
    for( sp = &swdevt[ 0 ];
 	 ( u_short )sp->sw_dev != 0 &&
	 ( u_short )dev != ( u_short )sp->sw_dev;
	 sp++ )
	 ;
    if( ( u_short )dev == ( u_short )sp->sw_dev ||
#endif	/* !OSF */

    if(
	dev == rootdev || 
        dev == dumpdev )
	while(( up = Dev_to_unitb( dev )) == NULL && ( --retries >= 0 )) {

	    /* if we're half way through the wait give some status
	     */
	    if( retries == 60 ) {
	        dev_maj = major(dev);
	        dev_min = minor(dev);
#ifndef OSF
                if( ( dev_t )dev == ( dev_t )sp->sw_dev )
                    printf("Waiting up to 1 more minute for swap device \(%d,%d\) to become available\n", dev_maj,dev_min);
#endif  /* !OSF */

	        if( ( dev_t )dev == ( dev_t )rootdev )
                    printf("Waiting up to 1 more minute for root device \(%d,%d\) to become available\n", dev_maj,dev_min);
	        else if( ( dev_t )dev == ( dev_t )dumpdev )
                    printf("Waiting up to 1 more minute for dump device \(%d,%d\) to become available\n", dev_maj,dev_min);
	    }

	    timeout( wakeup, ( caddr_t )dev, 1*hz );
	    ( void )sleep(( caddr_t )dev, PSWP+1 );
	}
    return( up );
}

/*
/*^L*/

/*
 *
 *   Name:	mscp_info() - MSCP unit information  
 *
 *   Abstract:	This routine returns a disk unit number and name for
 *		a given 'dkn' number passed (dkn is a drive-unique
 *		identifier used by the statistics gathering code). 
 *
 *   Inputs:	dkn -- the drive-unique identifier for 
 *		       performance statistics
 *
 *   Outputs:	unit -- the drive unit number for given dkn
 *
 *   Return	
 *   Values:	string identifying drive
 *
 * EDITORIAL: In the future,  there must be a beter way (driver
 * independent) for TBL_DKINFO to get this information.  This
 * definitely needs to be addressed when loadable driver support
 * is done (adding the name string and unit number to dkinfo removes
 * this junk; the MSCP and SCSI drivers would just need to jam those
 * values.  OK, I'll stop complaining...!!!).
 */
char *
mscp_info(dkn, unit)
    int 		dkn;
    int 		*unit; 	/* returned */
{
    CLASSB    		*clp = &mscp_classb;
    CONNB		*cp;
    UNITB		*up;
    struct device	*devp;

    /*
     * Here we just kind of brute force things: we walk through all
     * unit blocks on each connection and search for the dkn number
     * which matches.
     */
    if( clp->flink ) {
        for( cp = clp->flink; cp != (CONNB *)&clp->flink; cp = cp->flink ) {
            for( up = cp->unit.flink; 
                 up != (UNITB *)&cp->unit.flink; 
                 up = up->flink) {
                devp = up->ubdev;
                if ((devp) && ((int)devp->perf == dkn)) {
                    *unit = devp->logunit;
                    return ( (char *)devp->dev_name );
                }
            }
        } 
    }
    /* dkn not found */
    return (NULL);
}


/*^L*/

/*
 *
 *   Name:	mscp_dump() - MSCP crash dump service 
 *
 *   Abstract:	This routine performs generic dump service for MSCP
 *		class disk devices.  
 *
 *   Inputs:	No formal inputs.  Implicit inputs are "dumpdev" which
 *		defines the device (dev_t) of the dump device.
 *
 *   Outputs:	If successful, we dump memory to the dump device.
 *
 *   Return	
 *   Values:	None.
 */
#define	ONE_MEGABYTE	0x00100000
#define DBSIZE          0x20                    /* Dump blocksize (32)  */

#ifdef __mips

extern rex_base;

mscp_dump()
{
    extern 	daddr_t dumplo;
    extern	int	cpu;
    extern 	dev_t dumpdev;
    UNITB	*up;
    CONNB	*cp;
    struct 	device *devp;
    struct	controller *ctlr;
    daddr_t 	blkcnt, blkoff;
    int 	h, float_num, part, status, i;
#if 	LABELS
    struct partition *pp;
    int			sz;
#else
    int			maxsz;
#endif
    int			errno = 0;
    char prom_name[80];
    int dump_io;
    char *start;
    register int blk, bc;
    register char *CP;
 
    /*
     * Attempt to get dump device's unit and connection pointers.  Do
     * some sanity checks to see if the device device is *really* there
     * and online.
     */ 
    up = Dev_to_unitb(dumpdev);

    if ( up == NULL ) { 
        dprintf("radump: dump device %x non-existant, can not dump!\n",dumpdev);
        return( ENXIO );
    }
    if(!up->flags.online ) {
        dprintf("radump: dump device ra%d offine, can not dump!\n",up->unit);
        return( ENXIO );
    }

    devp = up->ubdev;
    cp = up->connb;
    ctlr = devp->ctlr_hd;
 
    /*
     * Get partition to use and block count and do some sanity checks
     * on the dump partition to make sure dump will be within bounds
     * of parititon.
     */
    part = Px( dumpdev );
    blkcnt = ctod(dumpsize);

#if 	LABELS
    pp = &up->disklabel.d_partitions[part];
    dprintf("partition size1 %d dump size %d\n", pp->p_size,
	    dumplo + blkcnt + 1);
    dprintf("partition size2 %d dump size %d\n",
	    pp->p_offset + dumplo + blkcnt + 1, up->unt_size);
    if ((dumplo + blkcnt + 1> pp->p_size) ||
	(pp->p_offset + dumplo + blkcnt + 1>= up->unt_size))
#else
    pt = &up->part_info.pt_part;
    if (pt->pt_part[part].pi_nblocks == -1)
        maxsz = up->unt_size - pt->pt_part[part].pi_blkoff;
    else maxsz = pt->pt_part[part].pi_nblocks;
    dprintf("partition size %d dump size %d\n", maxsz, dumplo + blkcnt + 1);
    if ((dumplo + blkcnt + 1> maxsz) ||
        (pt->pt_part[part].pi_blkoff >= up->unt_size))
#endif	/* LABELS */
        {
        dprintf("radump: dump device too small\n");
        return ( ENOSPC );
        }
 
    /* 
     * Open a channel for the Prom code.
     */
    CP = prom_name;
    i = 0;

    switch (cp->bus_type) {
        case DEV_QB:
            /* The dump device is a 'ra' device. */
            CP[i++]='r';
            CP[i++]='a';
            break;

        case DEV_MSI:
            /* The dump device is a 'rf' device. */
            CP[i++]='r';
            CP[i++]='f';
            break;

        default:
            dprintf("radump: can not open dump device, can not dump!\n");
            dprintf("radump: bus type=%d, dumpdev=%x\n",cp->bus_type,dumpdev);
    }

    if (!rex_base) {
        CP[i++]='(';
    }

    /* Fill in the dump ctlr part of the string.
     * We need to process one char at a time
     * and the number may be bigger that one
     * digit.  
     */
 
    if ((devp == 0) || (devp->alive == 0)) {
        dprintf("radump: cannot open dump device 0x%x\n",dumpdev);
        return ( 0 );
    }
    else { 
        i = i + itoa(ctlr->ctlr_num,&CP[i]);

	/*
	 *
        switch (cp->bus_type) {

          case DEV_MSI:
            i = i + itoa(devp->unit,&CP[i]);
            break;

          case DEV_QB:
            /* Go get the CSR for this controller number
             * and convert it into a hardware controller 
             * number for the open prom call.  The same
             * algorithm is used here as is used in the
             * console for the Q-bus.  That is:
             *	- If its at the fixed address its
             *    controller number 0.
             *
             *  - If its at the first floating address
             *    its controller number 1
             *
             *  - If its at the second floating address
             *    its controller number 2
             *
             *  - And so on, for a total of 16 
             *    or up to controller 15.  The first
             *    floating address is at 760334 (octal)
             *    and the go up by a value of 4.
            if(((int)ctlr->addr & 0xfff) == (0172150 & 0xfff)){
                i = i + itoa(0,&CP[i]);
            } 
            else {
                for( float_num = 0160334, h = 1;h < 16; h++) {
                    if ((float_num & 0xfff) == ((int)ctlr->addr & 0xfff)){
                        i = i + itoa(h,&CP[i]);
                        break;
                    }
                    float_num += 4;
                }
                if ((float_num & 0xfff) != ((int)ctlr->addr & 0xfff)) {
                    dprintf("radump: Can't open dump device - %x\n", (int)ctlr->addr);
                    return (0);
                }
            }
            break;

          default:
            dprintf("radump: unsupp. dump ctrlr, can not dump!\n");
            dprintf("radump: ctrlr type=%d, dumpdev=%x\n",cp->bus_type,dumpdev);
        }
	*
        */
    }
 
    /* Now load up the unit number part of the dump device */
    if (!rex_base) {
        CP[i++] = ',';	/* comma seperator */
    }
 
    /* Now load up the unit number part of the dump device
     * string.
     */
    if (!rex_base) {
        i = i + itoa(devp->unit,&CP[i]);
        CP[i++] = ',';	/* comma seperator */
    } 
    else {
        i = i + itoa(devp->unit,&CP[i]);
    }
 
    /*
     * Horses in midstream - Third arg to console used to
     * be partition, now is a block offset for the device.
     * The console will no longer have knowledge of 'partitions'.
     * Here we assume that PMAX and MIPSfair will not change
     * to the new format and that everything else will.
     */
    if (!rex_base) {
        if (cpu == DS_5500)
            CP[i++] = 'c';	/* make it the C partition */
        else
            CP[i++] = '0';	/* give it a zero offset */
 	  
        CP[i++] = ')';	/* end string here, don't */
        CP[i++] = '\0';	/* need the file name part*/
    } 
    else {
        CP[i++] = '/';
        CP[i++] = 'l';         /* pick any old name! */
        CP[i++] = ' ';
        CP[i++] = '-';
        CP[i++] = 'N';
        CP[i++] = 'o';
        CP[i++] = 'b';
        CP[i++] = 'o';
        CP[i++] = 'o';
        CP[i++] = 't';
        CP[i] = '\0';
    }
 
    if (rex_base) {
        /*
         * Check here to see if this is the boot disk.
         * If not boot disk we must do more setup,
         */
 
        *(int *)(rex_base + 0x54) = 0;
        dprintf("radump: REX command %s\n", cp);
        rex_execute_cmd(cp);
        if(*(int *)(rex_base + 0x54) == 0) {
            dprintf("radump: console dump initialization failed\n");
            return(-1);
        }
        if (rex_bootinit() < 0) {
            dprintf("radump: can't init console boot device\n");
            return( 0 );
        }
    } 
    else {
        /*
         * Open the 'c' partition so that we have full access
         * to the device.  We will provide the offset through
         * prom_lseek.
         * Printing the following will only scare people
         * when they see the 'c' partition. It is for debug.
         */
dprintf("mscp_dump: prom_open: prom_name= %s\n",prom_name);
        if ((dump_io = prom_open(prom_name, 2)) < 0) {
            dprintf("radump: can't open dump device\n");
            return ( 0 );
 	}
    }
 
#if 	LABELS
 	blkoff = pp->p_offset + dumplo;
#else
 	blkoff = pt->pt_part[part].pi_blkoff + dumplo;
#endif
 
    if (!rex_base) {
        if (prom_lseek(dump_io,blkoff * DEV_BSIZE,0) < 0) {
            dprintf("radump: can't lseek to dump device\n");
            prom_close(dump_io);
            return( 0 );
        }
    }
 
    start = (char *)0x80000000;

    dprintf("radump: dumping to ra%d, partition %d...\n",devp->unit,part); 
    /*
     * Dump the kernel.
     */
    if(partial_dump){
	vm_offset_t blocks[DEV_BSIZE/sizeof(vm_offset_t)], *ptr;
	int count, i, total, num;

	total = 0;
	bc = ctob(1);
	ptr = &blocks[1];
	num = DEV_BSIZE/sizeof(vm_offset_t) - 1;
	blocks[0] = (vm_offset_t) partial_dumpmag;
	while((count = get_next_page(ptr, num)) != 0){
	    for(i=count;i<num;i++) blocks[i+(ptr-blocks)] = 0;
	    if (rex_base)
	         status = (rex_bootwrite(blkoff, blocks, DEV_BSIZE)
					  == DEV_BSIZE) ? 1 : 0; 
	    else
	         status = (prom_write(dump_io, blocks, DEV_BSIZE) == DEV_BSIZE)
		   ? 1 :0; 
	    if (status & 1) blkoff++;
	    else {
	        dprintf("dump i/o error: bn = %d, ", blkoff);
	        if(!rex_base) prom_close(dump_io);
	        return(EIO);
	    }
	    for(i=ptr-blocks;i<count+(ptr-blocks);i++){
	        if (rex_base)
		    status = (rex_bootwrite(blkoff, blocks[i], bc) == bc)
		      ? 1 : 0;
	        else
		    status = (prom_write(dump_io,blocks[i], bc) == bc)
		      ? 1 :0; 
	         if (status & 1) blkoff++;
	         else {
		     dprintf("dump i/o error: bn = %d, ", blkoff);
		     if(!rex_base) prom_close(dump_io);
		     return(EIO);
	         }
	  	 if(((total + i) * NBPG) % ONE_MEGABYTE == 0) dprintf(".");
	    }
	    total += count;
	    ptr = blocks;
	    num = DEV_BSIZE/sizeof(vm_offset_t);
	}
	if(total != dumpsize){
	    dprintf("Mismatched dump size : Expected %d got %d\n",
		    dumpsize, total);
	}  	
    }
    else {
        while (blkcnt) {
            blk = blkcnt > DBSIZE ? DBSIZE : blkcnt;
     	    bc  = blk * DEV_BSIZE;
 	    if (rex_base)
                status = (rex_bootwrite(blkoff, start, bc) == bc) ? 1 : 0; 
            else
                status = (prom_write(dump_io, start, bc) == bc) ? 1 :0; 
            if (status & 1) {
                start += bc;
                blkcnt -= blk;
                blkoff += blk;
            }
            else {
                dprintf("radump: i/o error: bn = %d, ", blkoff);
                if(!rex_base)
                    prom_close(dump_io);
                    return( EIO );
            }
            /* Print a progress indicator to save some nail biting... */
            if ((int)start % ONE_MEGABYTE == 0) {
                dprintf(".");
	    }
 	}
    }
    return ( 0 );
}
#else	/* __alpha */
/*
 *
 *   Name:	mscp_dump() - MSCP crash dump service 
 *
 *   Abstract:	This routine returns info specific to the MSCP device
 *		that is needed to perform a dump.
 *
 *   Inputs:	Dump_req - generic dump info 
 *
 *   Outputs:	mscpdump_info - dump info unique to the MSCP data structures
 *
 *   Return	
 *   Values:	ESUCCESS, EXIO.
 */
#include "io/dec/mbox/mbox.h"
#define	ONE_MEGABYTE	0x00100000
#define DBSIZE          0x20                    /* Dump blocksize (32)  */

mscp_dump(dump_req)
	struct dump_request *dump_req;
{
    UNITB       *up;
    int         part;

#if     LABELS
    struct partition *pp;
#endif

    /*
     * Attempt to get dump device's unit pointer.
     */

	up = Dev_to_unitb(dump_req->dump_dev);
	dump_req->device = up->ubdev;

    /*
     * Get partition 
     */
	part = Px( dump_req->dump_dev );

#if     LABELS
	pp = &up->disklabel.d_partitions[part];
        dump_req->blk_offset = pp->p_offset + dump_req->blk_offset;
#else
	pt = &up->part_info.pt_part;
        dump_req->blk_offset = pt->pt_part[part].pi_blkoff + dump_req->blk_offset;
#endif  /* LABELS */

	return(ENOSYS);

}
#endif /*alpha*/


/*
 *
 *   Name:	mscp_minphys() -  maximum xfer check routine 
 *
 *   Abstract:	This routine will check the byte count (b_bcount) 
 *		requested from the supplied buffer (bp) to see if
 *		the transfer exceeds the maximum transfer size allowed
 *		by this device.  If the requested byte count exceeds
 *		the limits of this device, the requested byte count
 *		is set to the maximum transfer size allowed. 
 * 
 *   Inputs:
 *		bp  -  buf pointer for this request
 *
 *   Implicit
 *   Inputs:	The maximum allower transfer size (connb->max_bcnt) is
 *		used to check requested transfer byte count (the maximum
 *		byte count is an optional field in a set controller
 *		characteristics response message and is either set to
 *		a value for a controller or defaults to an MSCP or TMSCP
 *		default, 16mb or 64kb respectively.  See set controller
 *		characteristics response processing in mscp_conpol.c for
 *		more details).
 *
 *		NOTE: For now, we are using a configurable global
 *		MSCP_MAXPHYS (mscp_data.c).  There are problems with
 *		controllers which support larger transfers than the
 *		associated port driver will allow, and vice-versa.
 *		This will be modified in the future to take into account
 *		both the port driver and controller maximum physical
 *		transfer limits. 
 *
 *   Implicit
 *   Outputs:	The byte count may be modified if it exceeds max_bcnt.
 *
 *   Return	
 *   Values:	None.
 */
void
mscp_minphys( bp )
    struct buf 		*bp;
{
    UNITB		*up;


    /* 
     * Check to make sure that the unit corresponding to the device number
     * exists.  If not, return (strategy will bounce this request...).
     * NOTE: we are not checking online or online in progress, these will
     *       be checked at mscp_strategy().
     */
    up = Dev_to_unitb( bp->b_dev );
    if( up == NULL ) {
        return;
    }

    /*
     * Check the requested transfer size against the maximum supported
     * for this controller.  If this request exceeds the device's maximum
     * transfer size, truncate this request to equal that maximum.
     */
    if (bp->b_bcount > MSCP_MAXPHYS) {
        bp->b_bcount = MSCP_MAXPHYS;
    }

    return;
}
