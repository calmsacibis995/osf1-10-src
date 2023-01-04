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
static char *rcsid = "@(#)$RCSfile: np_block.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/07/13 17:45:18 $";
#endif
/*
 * derived from np_block.c	5.3	(ULTRIX)	10/16/91";
 */
/************************************************************************
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port 
 *		Driver block data communication service functions.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Function/Routines:
 *
 *   np_req_data		Request Block Data
 *   np_send_data		Send Block Data
 *   np_map_buf			Map local buffer
 *   np_unmap_buf		Unmap local buffer
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *	Changed memory allocation to use OSF zones.
 *	Changed buffer mapping to use OSF pmap_extact. 
 *
 *   23-Jul-1991	Brian Nadeau
 *	New NPORT module.
 *
 *   16-Oct-1991	Brian Nadeau
 *	Updates/bug fixes.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<sys/buf.h>
#include		<sys/ipc.h>
#include		<sys/proc.h>
#include		<sys/map.h>
#include		<dec/binlog/errlog.h>
#include                <machine/pmap.h>
#include                <machine/cpu.h>
#include		<hal/cpuconf.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/np/npport.h>
#include		<io/dec/np/npadapter.h>

#ifdef __alpha
#include		<io/common/devdriver.h>
#include		<io/dec/mbox/mbox.h>
#endif
/* External Variables and Routines.
 */
extern	int		cpu;
extern	SCSIB		lscs;
extern	struct proc	*proc;
extern	NPBDDB		*np_bddb;
extern	char    	*np_align_array[BDL_NUM][BDL_SIZE];
extern	SCSH		*np_alloc_msg();

/*   Name:	np_req_data	- Request Block Data
 *
 *   Abstract:	This function initiates transfer of block data from a remote
 *		buffer into a local buffer over a specific path.  The transfer
 *		is initiated by placing a REQDAT1 command packet onto the
 *		second lowest priority port command queue and notifying the
 *		port when the queue was previously empty.
 *
 *		A CI message buffer is temporarily allocated by this function
 *		and used to contain the command.  The buffer is placed onto the
 *		message free queue following command execution for use in
 *		containing confirmation of the transfer.  It is deallocated
 *		following transfer confirmation.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   lbh			- Local buffer handle pointer
 *   lboff			- Offset into local buffer
 *   rbh			- Remote buffer handle pointer
 *   rboff			- Offset into remote buffer
 *   pb				- Path Block pointer
 *   pccb			- Port Command and Control Block pointer
 *   size			- Size of transfer
 *   tid			- Transaction identifier pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Transfer successfully initiated
 *   RET_ALLOCFAIL		- Failed to allocate message buffer
 *
 *   SMP:	No locks are required; however, the PB must EXTERNALLY be
 *		prevented from deletion.  PCCB addresses are always valid
 *		allowing access to static fields because these data structures
 *		are never deleted once their corresponding ports have been
 *		initialized.
 *
 *		Locks lower than the PCCB in the SCA locking hierarchy may NOT
 *		be held EXTERNALLY without also holding the PCCB lock in case
 *		the port requires crashing.
 */
u_int
np_req_data( pccb, pb, size, tid, lbh, lboff, rbh, rboff )
    PCCB		*pccb;
    PB			*pb;
    u_int		size;
    TID			*tid;
    BHANDLE		*lbh;
    u_int		lboff;
    BHANDLE		*rbh;
    u_int		rboff;
{
    NPBH		*cibp;
    SCSH		*scsbp;
    REQDATH		*bp;

    /* The steps involved in requesting block data are:
     *
     * 1. Allocate a CI message buffer to contain the REQDAT command.
     * 2. Format the REQDAT specific portion of the command packet.
     * 3. Format the  N_PORT header.
     * 4. Initiate the block data transfer.
     *
     * The port is crashed if transfer can not be successfully initiated.
     */
    if(( scsbp = np_alloc_msg( pccb ))) {
	cibp = Scs_to_pd( scsbp, pccb );
	bp = ( REQDATH * )Pd_to_ppd( cibp, pccb );
	bp->xctid = *tid;
	bp->length = size;
	bp->sbname = rbh->Bname;
	bp->sboff = rbh->Boff + rboff;
	bp->rbname = lbh->Bname;
	bp->rboff = lbh->Boff + lboff;
	Format_nph( pccb, cibp, SNDPM, REQDAT1,
		    Scaaddr_low( pb->pinfo.rport_addr ), RECEIVE_BUF )
	Insqt_dccq1( cibp, pccb )
	return( RET_SUCCESS );
    } else {
	return( RET_ALLOCFAIL );
    }
}

/*   Name:	np_send_data	- Send Block Data
 *
 *   Abstract:	This function initiates transfer of block data from a local
 *		buffer into a remote buffer over a specific path.  The transfer
 *		is initiated by placing a SNDDAT command packet onto the second
 *		lowest priority port command queue and notifying the port when
 *		the queue was previously empty.
 *
 *		A CI message buffer is temporarily allocated by this function
 *		and used to contain the command.  The buffer is placed onto the
 *		message free queue following command execution for use in
 *		containing confirmation of the transfer.  It is deallocated
 *		following transfer confirmation.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   lbh			- Local buffer handle pointer
 *   lboff			- Offset into local buffer
 *   rbh			- Remote buffer handle pointer
 *   rboff			- Offset into remote buffer
 *   pb				- Path Block pointer
 *   pccb			- Port Command and Control Block pointer
 *   size			- Size of transfer
 *   tid			- Transaction identifier pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Transfer successfully initiated
 *   RET_ALLOCFAIL		- Failed to allocate message buffer
 *
 *   SMP:	No locks are required; however, the PB must EXTERNALLY be
 *		prevented from deletion.  PCCB addresses are always valid
 *		allowing access to static fields because these data structures
 *		are never deleted once their corresponding ports have been
 *		initialized.
 *
 *              Locks lower than the PCCB in the SCA locking hierarchy may NOT
 *              be held EXTERNALLY without also holding the PCCB lock in case
 *              the port requires crashing.
 */
u_int
np_send_data( pccb, pb, size, tid, lbh, lboff, rbh, rboff )
    PCCB		*pccb;
    PB			*pb;
    u_int		size;
    TID			*tid;
    BHANDLE		*lbh;
    u_int		lboff;
    BHANDLE		*rbh;
    u_int		rboff;
{
    NPBH		*cibp;
    SCSH		*scsbp;
    SNDDATH		*bp;

    /* The steps involved in sending block data are as follows:
     *
     * 1. Allocate a CI message buffer to contain the SNDDAT command.
     * 2. Format the SNDDAT specific portion of the command packet.
     * 3. Format the  N_PORT header.
     * 4. Initiate the block data transfer.
     *
     * The port is crashed if transfer can not be successfully initiated.
     */
    if(( scsbp = np_alloc_msg( pccb ))) {
	cibp = Scs_to_pd( scsbp, pccb );
	bp = ( SNDDATH * )Pd_to_ppd( cibp, pccb );
	bp->xctid = *tid;
	bp->length = size;
	bp->rbname = rbh->Bname;
	bp->rboff = rbh->Boff + rboff;
	bp->sbname = lbh->Bname;
	bp->sboff = lbh->Boff + lboff;
	Format_nph( pccb, cibp, SNDPM, SNDDAT,
		    Scaaddr_low( pb->pinfo.rport_addr ), RECEIVE_BUF )
	Insqt_dccq1( cibp, pccb )
	return( RET_SUCCESS );
    } else {
	return( RET_ALLOCFAIL );
    }
}

/*   Name:	np_map_buf	- Map Buffer
 *
 *   Abstract:	This function maps a local buffer.
 *
 *		A buffer descriptor is allocated from the  N_PORT
 *		buffer descriptor free list.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   bhp			- Buffer handle pointer
 *   np_bddb			-  N_PORT Buffer Descriptor Database
 *   pccb			- Port Command and Control Block pointer
 *   proc			- Process table
 *   sbh			- System buffer handle pointer
 *   scsid			- SCS identification number
 *   Sysmap			- Address of System page table
 *   usrpt			- Address of Process page tables
 *   Usrptmap			- Address of System ptes mapping process ptes
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   bhp			- Buffer handle pointer
 *	pd.np.boff		-  0
 *	pd.np.bname		-   N_PORT buffer name
 *	scsid			-  SCS identification number
 *   np_bddb			-  N_PORT Buffer Descriptor Database
 *	free_bd			-  Free buffer descriptor list head
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Successful allocation and initialization
 *   RET_ALLOCFAIL		- Allocation of free buffer descriptor failed
 *
 *   SMP:	The  N_PORT buffer descriptor database is locked for
 *		allocation of a buffer descriptor from the free list.
 */
u_long
np_map_buf( pccb, bhp, sbh, scsid )
    PCCB		*pccb;
    BHANDLE		*bhp;        
    struct buf		*sbh;
    u_int		scsid;
{
    BD			*bdp;
    DBP			*dbp;
    char		*addr;
    pmap_t		pmap;
    u_long		pfn, status = RET_SUCCESS;
    int			size, count;

    /* Local buffer mapping is accomplished as follows:
     * 
     * 1. Lock the  N_PORT buffer descriptor database.
     * 2. Allocate a free  N_PORT buffer descriptor.
     * 3. Unlock the  N_PORT buffer descriptor database.
     * 4. Map the local buffer by filling in the allocated buffer descriptor.
     * 5. Construct a handle on the local buffer.
     * 6. Return an appropriate status.
     *
     * How the local buffer is actually mapped depends upon what type of buffer
     * it is.  There currently are 3 types of local buffers:
     *
     */
    Lock_npbd()
    if(( bdp = np_bddb->free_bd )) {
	np_bddb->free_bd = ( BD * )( bdp->Next_free_bd );
    } else {
	status = RET_ALLOCFAIL;
    }
    Unlock_npbd()
    if( status == RET_SUCCESS ) {
#ifdef CI_DEBUG
	if (sbh->b_bcount > pccb->Typ0_max) {
	printf("np_map_buf: sbh = %lx\n",sbh);
	printf("np_map_buf: bdp = %lx\n",bdp);
	printf("np_map_buf: sbh->b_un.b_addr = %lx\n",sbh->b_un.b_addr);
	}
#endif /* CI_DEBUG */
 	U_int( *bdp ) = (( u_long )sbh->b_un.b_addr & PGOFSET ) | 
				NPBD_VALID;
        addr = ( char * )(( u_long )sbh->b_un.b_addr & ~(long)PGOFSET );
	bdp->buf_len = sbh->b_bcount;
#ifdef CI_DEBUG
	if (bdp->buf_len > pccb->Typ0_max) {
        printf("np_map_buf: bdp->boff = %x\n",U_int(*bdp));
        printf("np_map_buf: bdp->buf_name = %x\n",bdp->buf_name);
        printf("np_map_buf: bdp->buf_len = %x\n",bdp->buf_len);
        printf("np_map_buf: addr = %lx\n",addr);
        }
#endif /* CI_DEBUG */

	/* Turbochannel bus has 4 bytes as the smallest addressable unit into*/
	/* memory. So on READS that are not aligned we need get a buffer that*/
	/* is aligned for the Xfer, and then bcopy to the user buffer when   */
	/* done.  UGH!!!   						     */
	if(( pccb->lpinfo.type.hwtype == HPT_CITCA ) && 
	   ( sbh->b_flags & B_READ )) {
	    if((( u_long )sbh->b_un.b_addr & ALIGN_4BYTES ) || 
			( sbh->b_bcount & ALIGN_4BYTES )) {
		size = bdp->buf_len + ALIGN_4BYTES;
		size &= ~ALIGN_4BYTES;
                SCA_KM_ALLOC( addr, char *, size, KM_SCA, KM_NOW_CL_CA )
		if( addr ) {
		    np_align_array[bdp->Bdlt_idx][bdp->Bdl_idx] = addr;
	            U_int( *bdp ) = (( u_long )addr & PGOFSET ) | NPBD_VALID;
		    addr = ( char * )(( u_long )addr & ~PGOFSET );
		    bdp->buf_len = size;
		} else {
        	    U_int( *bdp ) = 0;
        	    bdp->buf_len = 0;
    		    Lock_npbd()
    		    bdp->Next_free_bd = ( caddr_t )( np_bddb->free_bd );
    		    np_bddb->free_bd = bdp;
    		    Unlock_npbd()
    		    return( RET_ALLOCFAIL );
		}
	    }
	}
	/* Odd byte transfer fix not necessary for CIMNA */

#ifdef CI_DEBUG
	if (bdp->buf_len > pccb->Typ0_max) {
		printf("np_map_buf: bdp->poff = %x,bdp->buf_len = %x\n",
			bdp->poff, bdp->buf_len);
	}
#endif /* CI_DEBUG */
	if(( count = bdp->poff + bdp->buf_len ) <= pccb->Typ0_max ) {
#ifdef __alpha
	    if(!IS_SYS_VA( addr )) 
#else
	    if( IS_KUSEG( addr )) 
#endif /* __alpha */
	    {
              	pmap = sbh->b_proc->task->map->vm_pmap; 
                pfn = pmap_extract( pmap, addr );
            } else 
		svatophys( addr, &pfn );
	    bdp->Rptr1 = ( caddr_t )pfn;
	    addr  += pccb->Pgsize;
            if( count > pccb->Pgsize ) {
#ifdef __alpha
		if(!IS_SYS_VA( addr )) 
#else
		if( IS_KUSEG( addr )) 
#endif /* __alpha */
                    pfn = pmap_extract( pmap, addr );
                else 
		    svatophys( addr, &pfn );
	        bdp->Rptr2 = ( caddr_t )pfn; 
	    } else 
		bdp->Rptr2 = 0;
	} else if( count <= 139264 ) {
/*
   for silver we must limit nport I/O to 128K+8K instead of Typ1_max
   since we can not allocate the memory without the possibility
   of putting the dsaisr thread to sleep forever! 

   below we always allocate the memory from the 256 byte zone so
   we can track it and so the alignment is correct per the nport
   spec.  Previously we always allocated pccb->Pgsize (8192 for
   alpha and 4096 for mips.  If you change the km_alloc you also
   need to fix the free call in np_unmap.

 */
            SCA_KM_ALLOC( dbp, DBP *, 256, KM_SCA, KM_NOW_CL_CA )
	    if( dbp ) {
		svatophys( dbp, &pfn );
		bdp->Rptr1 = ( caddr_t )( pfn | BP_TYP1 );
		bdp->Rptr2 = ( caddr_t )dbp;
		while( count > 0 ) {
#ifdef __alpha
		    if(!IS_SYS_VA( addr )) 
#else
	    	    if(IS_KUSEG(addr))
#endif /* __alpha */
			{
			pmap = sbh->b_proc->task->map->vm_pmap;
                        pfn = pmap_extract(pmap, addr);
			}
            	    else 
                        svatophys(addr, &pfn);

		    dbp->Dbptr = ( caddr_t )pfn;
		    count -= pccb->Pgsize;
		    addr  += pccb->Pgsize;
		    dbp++;
		}
	    } else {
        	U_int( *bdp ) = 0;
        	bdp->buf_len = 0;
    		Lock_npbd()
    		bdp->Next_free_bd = ( caddr_t )( np_bddb->free_bd );
    		np_bddb->free_bd = bdp;
    		Unlock_npbd()
    		return( RET_ALLOCFAIL );
	    }
	} else {		/* type2 pointer */
#ifdef CI_DEBUG
	    printf("np_map_buf: Type2 ptrs not implemented\n");
#endif /* CI_DEBUG */
	    ( void )panic( NPPANIC_XFERSIZE );
	}
	bhp->Boff = 0;
	bhp->Bname = bdp->buf_name;
	bhp->scsid = scsid;
#ifdef CI_DEBUG
	printf("np_map_buf: bhp->Boff = %x\n",bhp->Boff);
	printf("np_map_buf: bhp->Bname = %x\n",bhp->Bname);
	printf("np_map_buf: bhp->scsid = %x\n",bhp->scsid);
#endif /* CI_DEBUG */
    }
    return( status );
}

/*   Name:	np_unmap_buf	- Unmap Buffer
 *
 *   Abstract:	This function unmaps a local buffer.
 *
 *		A buffer descriptor is deallocated to the  N_PORT
 *		buffer descriptor free list after its key is incremented.
 *
 *		Encountering an invalid buffer handle causes a panic.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   bhp			- Buffer handle pointer
 *   np_bddb			-  N_PORT Buffer Descriptor Database
 *   lscs			- Local system permanent information
 *   pccb			- Port Command and Control Block pointer
 *   sbh			- System buffer handle pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   np_bddb			-  N_PORT Buffer Descriptor Database
 *	free_bd			-  Free buffer descriptor list head
 *
 *   SMP:	The N_PORT buffer descriptor database is locked for
 *		buffer descriptor deallocation to the free list.
 */
void
np_unmap_buf( pccb, bhp, sbh )
    PCCB		*pccb;
    BHANDLE		*bhp;        
    struct buf		*sbh;
{
    BD			*bdp;
    BDLP		*bdlp;
    u_int		bdl_idx;

    /* Local buffer unmapping is accomplished as follows:
     *
     * 1. Position to the appropriate N_PORT buffer descriptor.
     * 2. Verify the authenticity of the targeted buffer descriptor.
     * 3. Clear the targeted buffer descriptor.
     * 4. Lock the N_PORT buffer descriptor database.
     * 5. Return the targeted buffer descriptor to the N_PORT buffer
     *	  descriptor free list.
     * 6. Unlock the N_PORT buffer descriptor database.
     */

    bdlp = np_bddb->bdlt + bhp->Bname.bdlt_idx;
    if( !(( u_long )bdlp->bdl_ptr.a.ptr & BDLP_VALID ) ||
        (( bdl_idx = bhp->Bname.bdl_idx ) > ( BDL_SIZE -1 )) ||
  	( bhp->Bname.key != 
	  (( bdp = ( BD * )bdlp->vbdl_ptr.a.ptr + bdl_idx )->buf_name.key ))) {
	( void )panic( NPPANIC_BNAME );
    }
    if(( pccb->lpinfo.type.hwtype == HPT_CITCA ) && ( sbh->b_flags & B_READ )){
	if((( u_long )sbh->b_un.b_addr & ALIGN_4BYTES ) || 
			( sbh->b_bcount & ALIGN_4BYTES )) {
	    if( !np_align_array[bdp->Bdlt_idx][bdp->Bdl_idx] ) {
		( void )panic( NPPANIC_ALIGN_ARRAY );
	    } 

	    {
    	    u_long      pfn;           
    	    int		rem; 
    	    pmap_t	pmap;
    	    int		cnt  = sbh->b_bcount; 
    	    caddr_t	to   = (caddr_t)sbh->b_un.b_addr;
    	    caddr_t	from = (caddr_t)np_align_array[bdp->Bdlt_idx][bdp->Bdl_idx];
#ifdef	mips
    	    for( ; cnt > 0; cnt -= rem, to += rem, from += rem ) {	
        	if((rem = MIPS_PGBYTES - ((u_long)from & PGOFSET)) > cnt )
            	    rem = cnt;		
	        Clean_dcache_addr( from, rem, 0 ); 

                /* Map user using Kseg0 address will work with up to 512MB */
	        if( IS_KUSEG( to )) {
    	    	    pmap = sbh->b_proc->task->map->vm_pmap;
                    pfn = pmap_extract( pmap, to );
	            bcopy( from, (caddr_t)PHYS_TO_K0( pfn ), rem );
	        } else
	            bcopy( from, to, rem );
    	    }
#else		    /* ALPHA, need!!!! to map Map user b_addr */
	    	    bcopy( from, to, sbh->b_bcount );
#endif	/* mips */
	    }

	    bcopy( np_align_array[bdp->Bdlt_idx][bdp->Bdl_idx],
	           sbh->b_un.b_addr, 
		   sbh->b_bcount );

            SCA_KM_FREE( np_align_array[bdp->Bdlt_idx][bdp->Bdl_idx], 
			 bdp->buf_len, KM_SCA )
            np_align_array[bdp->Bdlt_idx][bdp->Bdl_idx] = 0;
	}
    } 

    if(( u_long )bdp->Rptr1 & (long)BP_TYP1 ) {
        SCA_KM_FREE(( char * )bdp->Rptr2, 256, KM_SCA )
    }
    U_int( *bdp ) = 0;
    bdp->buf_len = 0;
    bdp->Rptr1 = 0;
    if(( ++bdp->Key ) == 0 ) {
	++bdp->Key;
    }
    Lock_npbd()
    bdp->Next_free_bd = ( caddr_t )( np_bddb->free_bd );
    np_bddb->free_bd = bdp;
    Unlock_npbd()

#ifdef mips
/*
 * On a mips machine, if we did a DMA read, we need the flush
 * the cache.
 */
    if(sbh->b_flags & B_READ) {
        switch(cpu) {
            case DS_5800:           /* Hardware assist here.  */
                break;
            default:
                bufflush(sbh);
        }
    }
#endif mips
}
