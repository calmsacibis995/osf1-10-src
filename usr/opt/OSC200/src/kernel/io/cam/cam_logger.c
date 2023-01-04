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
static char *rcsid = "@(#)$RCSfile: cam_logger.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/09/22 12:58:06 $";
#endif

/* ---------------------------------------------------------------------- */

/* cam_logger.c		Version 1.00	    June 10, 1991 

    This module is the common interface between the CAM sub-system
    and the ULTRIX BSD based error logger and formatter. We take 
    a error log header from the CAM subsystem and allocated and
    errlog buffer. Fill in all the required information and ship
    it to the error logger.

Modification History

    Version    Date	Who    Reason

    1.00    06/10/91    dallas    Creation date.  
    1.01    07/31/91    dallas    General bug fixes, since I start
				  using it. Now using Robin's dump
				  programs if errlog not running.
    1.02    08/21/91	dallas	  Turned on errlog. General fixes
				  once I started logging errors to
				  the error logger.
    1.03    11/12/91	dallas    Fixed bug where I was never adding
				  in the header and tail sizes to the 
				  request size.
    1.04    11/17/91	dallas	  Refixed the previous fix.
    1.05    11/19/91	dallas	  Added external declarations, and
				  removed unused routines.
    1.06    11/21/91	dallas	  Changed fuctionality, instead of
				  the modules each doing size calculations
				  this module will now do the breaking
				  up of the packets.
    1.07    12/02/91	dallas    Bug fixes. Fixed dropped packet entry
				  and packet over run.

*/		    

/* ---------------------------------------------------------------------- */

/* Include files. */

#include <io/common/iotypes.h>
#include <sys/types.h>	  	/* system level types */
#include <sys/time.h>		/* Have to include it........*/
#include <sys/param.h>		/* Have to include it........*/
#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam_logger.h>	/* CAM errlog logger defines */
#include <io/cam/cam_errlog.h>
#include <io/cam/scsi_all.h>	/* For sense data */
#include <io/cam/cam.h>

/*
 * No error logger proc pointer in OSF, yet.
 */
int elprocp = NULL;

/* ---------------------------------------------------------------------- */

/* Local defines. */

/*
 * This will be removed in the second submit in BL8.
 */
struct binlog_softc {        /* state & status info for binary event logger */
       unsigned int   sc_state;        /* state flags */
       unsigned int   sc_open;         /* number of opens done */
       unsigned int   sc_size;         /* size of event log buffer */
       unsigned int   sc_nobufavail;   /* no buffer available counter */
       unsigned int   sc_badsize;      /* bad record size counter */
       unsigned int   sc_readbusy;     /* log busy on read counter */
       unsigned int   sc_seqnum;       /* sequence number */
};

void cam_logger();


/* ---------------------------------------------------------------------- */

/* External declarations. */


/* ---------------------------------------------------------------------- */

/* Initialized and uninitialized data. */


/* ---------------------------------------------------------------------- */

/* Functional Description:
 *
 * Routine Name: cam_logger
 *    
 *    This routine takes an cam error log header, and allocates an
 *    error log buffer from the pool. It routine will then fill in 
 *    the required information for the BSD based packet and ship it 
 *    to the error logger.
 *
 * Call syntax:
 *	cam_logger( cam_err_hdr, bus, target, lun )
 *		CAM_ERR_HDR	*cam_err_hdr;	 Pointer to the header 
 *		char		bus;		 Bus
 *		char		target		 Target
 *		char		lun		 Lun
 *
 *
 * Implicit Inputs
 *	None
 *
 * Implicit Outputs
 *	Filled in BSD based error log packet.
 *
 * Return Values
 *	NONE 0
 *
 *
 */


void 
cam_logger( cam_err_hdr, bus, target, lun )
    CAM_ERR_HDR		*cam_err_hdr;	/* The errlog header where we get
					 * everything else from.
					 */
    char		bus;		/* Bus				*/
    char		target;		/* Target			*/
    char		lun;		/* Lun				*/

{

    /*
     * Local variables
     */
    
    extern pid_t	binlog_savedpid;
    extern struct binlog_softc blsoftc;

    CAM_ERR_ENTRY	*err_ent;
    struct el_rec  	*elp;
    caddr_t		bufp;	  /* where we copy the data to.	*/
    U32			req_size; /* Size of the errlog buffer 
					   * we will need	
					   */
    U32 		i, j, k, times;  /* For for loops		*/
    U32		cam_nologger = 0; 
    U32		num_entry;	/* 
					 * Number of entries this time
					 * around
					 */
    U32		last_req_size; 	/* 
					 * last valid size index before
					 * size exceded 
					 */
    U32		this_req_size;	/* Current size		*/
    U32		total_entries;	/* Total number of entries */
    U32		start_ent;	/* Starting entry this time around */
    CAM_ERR_ENTRY	ent_cont;	/* Continuation entry struct*/
    static u_char	str_cont[] = 
		"This error entry is a continuation for the previous entry";







    /*
     * Validate.... if the pointer is good and its for me.
     */
    if(cam_err_hdr->hdr_type  != CAM_ERR_PKT ){
	return;
    }
    if(cam_err_hdr->hdr_list == NULL){
	/*
	 * No list
	 */
	return;
    }

    err_ent = cam_err_hdr->hdr_list;

    /*
     * Set up our continuation struct now.......
     */
    ent_cont.ent_type = ENT_STRING;
    ent_cont.ent_data = str_cont;
    ent_cont.ent_size = strlen(ent_cont.ent_data) + 1;
    ent_cont.ent_pri = PRI_BRIEF_REPORT;

    /* Get our total entries */
    total_entries =  cam_err_hdr->hdr_entries;

    /* 
     * Init entry start
     */
    i = 0;

    /* 
     * Do each one of the of the entries
     * The loop termination is now checked here at the top of the loop.
     */
    for(  times = 0; i < total_entries; times++  ){
	/*
	 * Set the number of entries this time around
	 */
	 num_entry = 0;

    	/*
    	 * Get the size of our buffer
    	 * Size of header plus size of element list
    	 */
    	this_req_size = sizeof( CAM_ERR_HDR );
	last_req_size = sizeof( CAM_ERR_HDR );

	/*
	 * If this is not first time around then must include
	 * continuation string
	 */
	if( times != NULL){
	    /*
	     * Since this is the second plus time around 
	     * we must increment the number of entries here
	     */
	    num_entry++;

	    /*
	     * Make sure entry ends on long word boundary
	     */
	    if(( ent_cont.ent_size & (sizeof(long)-1) ) != NULL){
	        /*
	         * calculate total entry size to long word boundary
	         */
	        ent_cont.ent_total_size = (sizeof(long) - 
		( ent_cont.ent_size & (sizeof(long)-1) )) + ent_cont.ent_size;
	    }
	    else {
	        ent_cont.ent_total_size = ent_cont.ent_size;
	    }
	    this_req_size += (ent_cont.ent_total_size + sizeof(CAM_ERR_ENTRY));
	    last_req_size += (ent_cont.ent_total_size + sizeof(CAM_ERR_ENTRY));
	}


	/* 
	 * Check this entry, if it is too large, with the EL_MISCSIZE +
	 * CAM_ERR_HDR + CAM_ERR_ENTRY added in, to fit into an error
	 * log packet, EL_MAXRECSIZE, report and error to the console
	 * and continue on to the next one.
	 *
	 * Note: This is a "quick" check here, this will allow the inner
	 * for-loop to continue with the rest of the entries.  There is no
	 * checking for alignment EL_MISCSIZE already contains a 4 for
	 * trailer, this is the worst case.  This will always pass for the
	 * initial pass, (i = 0).  However for subsequent passes the 
	 * inner for-loop would have stopped at this entry.
	 */
	 if( (this_req_size + err_ent[i].ent_size + EL_MISCSIZE)
	     > EL_MAXRECSIZE) {

	     /*
	      * Increment i, (the CAM error packet index) to index the
	      * next one.  Report the problem and continue with the next
	      * iteration of the outer for-loop.
	      */
	      i++;			/* next entry */
	      printf("cam_logger: CAM_ERROR entry too large to log!\n");
	      continue;
	 }

    	/* 
    	 * get the size of each of the entries
    	 */
    	for( start_ent = i; (i < total_entries) || 
		((this_req_size + EL_MISCSIZE) > EL_MAXRECSIZE); ){

	    this_req_size += sizeof(CAM_ERR_ENTRY);

	    /*
	     * Make sure entry ends on long word boundary
	     */
	    if(( err_ent[i].ent_size & (sizeof(long)-1) ) != NULL){
	        /*
	         * calculate total entry size to long word boundary
	         */
	        err_ent[i].ent_total_size = (sizeof(long) - 
		( err_ent[i].ent_size & (sizeof(long)-1) )) 
			+ err_ent[i].ent_size;
	    }
	    else {
	        err_ent[i].ent_total_size = err_ent[i].ent_size;
	    }
	    this_req_size += err_ent[i].ent_total_size;

	    /*
	     * Check to see if the size of the request needs
	     * to be sent down....
	     */
	     if((this_req_size +  EL_MISCSIZE) > EL_MAXRECSIZE){
		/*
		 * Get out of the for loop
		 */
		break;
	    }
	    else {
	    	/* 
	    	 * Increment this here because we can break out of for...
	    	 */
		 i++;
	    	num_entry++;
		last_req_size = this_req_size;
	    }
		
        } /* End of  entry for... */

	cam_err_hdr->hdr_entries = num_entry;


        /* 
         * If the error logger is not running...... Everthing goes out to 
         * the console in byte format.......
         */
	 elprocp = binlog_savedpid;
	 if (blsoftc.sc_state & BINLOG_ON)
	     cam_nologger = 0;
	 else
	     cam_nologger = 1;

	  if((elprocp == NULL) || 
	     (cam_nologger == 1) ||
	     ((elp = ealloc( last_req_size, cam_err_hdr->hdr_pri)) ==
	      EL_FULL)) {

	    /*
	     * The error logger is not running....or we could not get
	     * a buffer....lets print it out....
	     */

	    /*
	     * Don't print it out unless the packet's are of type
	     * EL_PRISEVERE or EL_PRIHIGH.
	     */
	    if ((cam_err_hdr->hdr_pri != EL_PRISEVERE) &&
                (cam_err_hdr->hdr_pri != EL_PRIHIGH)) {
	        return;
	    }

	    printf("cam_logger: CAM_ERROR packet\n");
	    if( (bus != -1) || (target != -1) || (lun != -1) ){
	        printf("cam_logger: ");
	        if( bus != -1 ){
	            printf("bus %d ", bus);
	        }
	        if( target != -1 ){
	            printf("target %d ", target);
	        }
	        if( lun != -1 ){
	            printf("lun %d ", lun);
	        }
	        printf("\n");
	    }
	    else {
	        printf("cam_logger: No associated bus target lun\n");
	    }

	    for( j = 0, k = start_ent; j < cam_err_hdr->hdr_entries; j++, k++){
		/*
		 * Don't print continuation message..
	         * Find out the type.... 
	         */
	        if( (err_ent[k].ent_type >= STR_START) && 
			(err_ent[k].ent_type <= STR_END)){
		    /*
		     * Only print strings
		     */
		    printf("%s\n", err_ent[k].ent_data);
		    DELAY(100000);
		    continue;
	        }
	
	    } /* end of for */
	    continue;
        } /* end of if errlog not running or can't get buffer */	

	/*
	 * We have an errlog buffer lets start filling it in.
	 */
	
	    /* CAM_ERR_PKT */
	    elp->elsubid.subid_class = cam_err_hdr->hdr_type;

	    /* TAPE DISK PRINTER CDROM ASC SII etc.....
	    elp->elsubid.subid_ctldevtyp = cam_err_hdr->hdr_subsystem;

	    /* CAM_DISK CAM_TAPE ASC_DME etc. */
	    elp->elsubid.subid_type = cam_err_hdr->hdr_class;

	    /*
	     * Get controller and target lun info.
	     */
	    if( bus != -1 ){
	        elp->elsubid.subid_num = bus;
	        elp->elsubid.subid_unitnum = (bus << 6) | (target << 3) | lun;
	    }
	    else{
	        elp->elsubid.subid_num = EL_UNDEF; 
	        elp->elsubid.subid_unitnum = EL_UNDEF; 
	    }

	    /* Error code not used..... */
	    elp->elsubid.subid_errcode = EL_UNDEF;

	    /* 
	     * Now lets get our data into the buffer....
	     * elp is the pointer to head of packet....we must 
	     * adjust the to pointer to data sections and copy...
	     */
	    bufp = (caddr_t)elp + EL_RHDRSIZE + EL_SUBIDSIZE;
    	    err_ent = cam_err_hdr->hdr_list;

	    /*
	     * bzero where we are stuffing the entries.
	     */
	    bzero( bufp, last_req_size );

	    /*
	     * Now copy the CAM header packet.....
	     */
	    bcopy((caddr_t *)cam_err_hdr, bufp, sizeof(CAM_ERR_HDR));
	    bufp += sizeof(CAM_ERR_HDR);

	    /* 
	     * Start at 0 for j
	     */
	    j = 0;
	    
	    /* 
	     * If this is the second time around include the continuation
	     * string
	     * It Has been accounted for......
	     */
	    if( times != NULL){
	        /*
	         * Copy error entry stucture
	         */
	        bcopy( &ent_cont, bufp, sizeof(CAM_ERR_ENTRY));
	        bufp += sizeof(CAM_ERR_ENTRY);

	        /*
	         * Copy data.....
	         */
	        bcopy( ent_cont.ent_data, bufp, ent_cont.ent_size );
	        bufp += ent_cont.ent_total_size; 

		/* 
		 * Since this is a entry increment our count..
		 */
		j++;

	    }

	    /* 
	     * Now get rest
	     */

	    for( k = start_ent; j < cam_err_hdr->hdr_entries; j++, k++){
	        if( err_ent[k].ent_data == NULL ){
		    printf("cam_logger: Null data ptr found in entry list\n");
		    /*
		     * set entries to the number actually in this 
		     * Packet......
		     */
		    cam_err_hdr->hdr_entries = j;
		    break;
	        }
	        /*
	         * Copy error entry stucture
	         */
	        bcopy( &err_ent[k], bufp, sizeof(CAM_ERR_ENTRY));
	        bufp += sizeof(CAM_ERR_ENTRY);

	        /*
	         * Copy data.....
	         */
	        bcopy( err_ent[k].ent_data, bufp, err_ent[k].ent_size );
	        bufp += err_ent[k].ent_total_size; 

	    }

	    /*
	     * Now validate it..
	     */
	    EVALID(elp);
    } /* end of massive for */

    return;
}

