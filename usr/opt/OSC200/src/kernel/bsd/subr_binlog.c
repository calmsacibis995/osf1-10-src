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
static char *rcsid = "@(#)$RCSfile: subr_binlog.c,v $ $Revision: 1.1.11.2 $ (DEC) $Date: 1993/05/10 02:17:11 $";
#endif

/*
 *   File name: subr_binlog.c
 *
 *   Source file description: 
 *   	This file contains the binary event logging routines used to log
 *	hardware and general kernel errors into a global kernel buffer.
 *	These errors are then read by a user-level daemon process,
 *	via the special device, /dev/kbinlog.
 *
 *      The functions binlog_hdrinit() and binlog_valid() are the only ones
 *      that manipulate the contents of an event record . These functions
 *      directly manipulate the non-event-specific elements of an event 
 *      record (stuff in the header).
 *
 *      The binlog_logmsg() function knows 'of' the non-event-specific record
 *      data structures, but does fiddle directly with whats in them... it
 *      does uses the LSUBID macro to do this.  This function actually acts 
 *      like any other user of binary event logging (for example a device 
 *      driver) and is just logging data.... It uses a template of what the 
 *      event record looks like and fills in the non-event-specific stuff 
 *      using the LSUBID macro and then appropriately fills in the 
 *      event-specific data.
 *
 *      As far as the other functions in this file are concerned an event 
 *      record is just a block of data.  binlog_write() and binlog_alloc() do
 *      need to know the sizes of the non-event-specific sections of the event
 *      record.  This is done using #define's and sizeof() in sys/binlog.h....
 *      where the non-event-specific data structures that make up an event 
 *      record are defined.
 * 
 *      If the format of an event record changes the functions discussed
 *      above need to follow those changes.  And, of course the daemon 
 *      (binlogd) will need changes, (only in one function) as it needs 
 *      knowledge of the non-event-specific sections of the event record.
 *
 *   Functions:
 *     driver:
 *	  binlog_open, binlog_close	open/close special char device
 *	  binlog_read, binlog_write	read/write special char device
 *	  binlog_ioctl, binlog_select	ioctl/select on special char device
 *
 *     programming interface:
 *	  binlog_alloc			allocate error log buffer space
 *        binlog_valid                  valid event record
 *
 *     misc:
 *        binlog_init                   initialize kernel binary event logging
 *	  binlog_wakeup                 wake up daemon
 *        binlog_hdrinit                initialize event record header
 *        binlog_logmsg                 steals copy of message from syslog buf
 *
*/


#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <dec/binlog/binlog.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <machine/cpu.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>
#include <kern/lock.h>
#include <mach/vm_param.h>
#include <mach/machine.h>
#include <vm/vm_kern.h>
#include <sys/lwc.h>



#define SUCCESS 0
#define ERROR  -1


extern unsigned int binlog_bufsize;       /* in binlog_data.c */
extern unsigned int binlog_status;        /* in binlog_data.c */ 

decl_simple_lock_data(,binlog_lock)      
#define splbinlog splextreme

static void binlog_wakeup_lwc();
static void binlog_wakeup();

struct binlog_softc {        /* state & status info for binary event logger */
       unsigned int   sc_state;        /* state flags */
       unsigned int   sc_open;         /* number of opens done */
       unsigned int   sc_size;         /* size of event log buffer */
       unsigned int   sc_nobufavail;   /* no buffer available counter */
       unsigned int   sc_badsize;      /* bad record size counter */
       unsigned int   sc_readbusy;     /* log busy on read counter */
       unsigned int   sc_seqnum;       /* sequence number */
       struct queue_entry sc_selq;     /* threads waiting for select */
       lwc_id_t sc_lwcid;	       /* id of lightweight context */
};


/* 
 * Have things pre initialized to look like binary event logging is disabled.
 * This is a safety net should binlog_init() never be called or fails.
 */
int binlog_onoff = 0;  /* logger status for others to use:    1=on 0=off  */
struct binlog_softc blsoftc = {0, 0, 0, 0, 0, 0};
struct binlog_bufhdr *blbuf = NULL;    /* pointer to binary event log buffer  */
vm_offset_t blbufphysaddr = NULL;      /* saved physical address of blbuf  */
pid_t binlog_savedpid = 0L;            /* pid of daemon */

char *RecordTrailer = trailer;      /* trailer for end of an event record */


/********************************************************************
 *                                                                  *
 *         Driver Routines (/dev/kbinlog) for Binary Event Logger   *
 *                                                                  *
 ********************************************************************/



/*
 * Function: binlog_open(dev)
 *
 * Function description:  Pretty much just satisfies caller.
 *
 * Arguments: 
 *	dev	device (not used)
 *
 * Return value: errno value
 *
 * Side effects:  Increments open count (blsoftc.sc_open)
*/
binlog_open(dev)
dev_t dev;
{
	register int s;

	/*
	 * No opens allowed to succeed unless the kernel binary event log
	 * buffer has been allocated (binlog_init() succeeded).  The fact that
	 * binary event logging may be disabled is not of concern here, the 
	 * other functions handle that condition.
	 */
        if ((blsoftc.sc_state & BINLOG_BUFALLOCED) == 0)
	      return(ENOBUFS);

	s = splbinlog();
	simple_lock(&binlog_lock);
	blsoftc.sc_open++;                   /* count the opens */
	simple_unlock(&binlog_lock);
        splx(s);
	return(ESUCCESS);
}

/*
 * Function: binlog_close()
 *
 * Function description:  releases any sleepers and waiting selects on last
 *                        close.
 *
 * Arguments:  None
 *
 * Return value: Always succeeds
 *
 * Side effects: Decrements the open count (blsoftc.sc_open).
 *
 * Restrictions: Must not be called from above SPLHIGH.
 */
binlog_close()
{
	register int s;

	if (getspl() > SPLHIGH) panic("binlog_close:SPLEXTREME");

	s = splbinlog();
	simple_lock(&binlog_lock);
	blsoftc.sc_open--;                      
        if (blsoftc.sc_open == 0)  {
		simple_unlock(&binlog_lock);
		select_wakeup(&blsoftc.sc_selq); 
		select_dequeue_all(&blsoftc.sc_selq);
        }
	else simple_unlock(&binlog_lock);
	splx(s);
	return(ESUCCESS);
}

/*
 * Function: binlog_read(dev,uio)
 *
 * Function description: Pass error log buffer contents to caller. Usually
 *              the binary event log daemon is the only reader.  If there
 *              are multiple readers than its first come first serve, no
 *              guarantee that everybody gets a copy of everything.
 *
 * Arguments: 
 *	dev	device (not used)
 *	uio	pointer to uio structure
 *
 * Return value: errno value
 *
 * Side effects: None
 *
 * Restrictions: Must not be called with ipl above SPLHIGH.
 */
binlog_read(dev,uio) 
dev_t dev;
struct uio *uio;
{
   register int s;
   int rval = 0;
   caddr_t output_start;       /* start of data for uiomove */
   int output_length;          /* amount of data for uiomove */
   int error = 0;


   /* only a privileged user can read the kernel binary error log buffer */
#if     SEC_BASE
		if (!privileged(SEC_DEBUG, EPERM))
			return (EACCES);
#else
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif

   if (getspl() > SPLHIGH) panic("binlog_read: SPLEXTREME");

   s = splbinlog();
   simple_lock(&binlog_lock);

   /*
    * If there are records in process of being built (pending != 0L) or
    * there is no data to read (input ptr == output ptr) then sleep.
    *
    * Only the reader which has done a BINLOG_SETPID is guaranteed to see
    * all records which were written into the buffer.  Other readers may
    * not be awakened before the buffer is reset.
    */
   while ((blbuf->pending != 0L) || (blbuf->in == blbuf->out))  {
      if (blbuf->pending != 0L)  /* read attempt with incomplete records */
             blsoftc.sc_readbusy++; 
      blsoftc.sc_state |= BINLOG_RDWAIT;  /* flag that someone is waiting */
      simple_unlock(&binlog_lock);
      if (error = tsleep((caddr_t)blbuf, (PZERO+1) | PCATCH, "binlog", 0)) {
               splx(s);
               return (error);
	}
      simple_lock(&binlog_lock);
   }
   blsoftc.sc_state &= ~BINLOG_RDWAIT;

    /*
     * When we do the uiomove we drop IPL and release the lock incase a page
     * fault occurs.  So, get where and how much data to output now so stuff
     * can potentially be put in the buffer while we are doing the uiomove.
     * Update the buffer header info appropriately.
     */
    output_length = 0;
    /*
     * output ptr behind input ptr... the usual case... output the data
     * that is in between.
     */
    if (blbuf->out <= blbuf->in)  {
	   output_start = (caddr_t)(blbuf->out);
	   output_length = (int)(blbuf->in - blbuf->out);

	   /* Only the process that has previously done a BINLOG_SETPID
	    * ioctl is allowed to change buffer pointers.
	    */
	   if (u.u_procp->p_pid == binlog_savedpid)
	          blbuf->out = blbuf->in;

    }
    /*
     * Input ptr was wrapped to start of buffer so there is data remaining
     * at the end of the buffer to be output.  There could be data to
     * output at the front of the buffer...which we'll handle at the end of
     * this routine.
     */
    else  {  
	 output_start = (caddr_t)(blbuf->out);
         output_length = (int)(blbuf->le - blbuf->out);


	 /* Only the process that has previously done a BINLOG_SETPID
	  * ioctl is allowed to change buffer pointers.
	  */
	 if (u.u_procp->p_pid == binlog_savedpid)  {
                blbuf->out = ((char *)blbuf + BUFHDRSIZE);  /* reset out ptr */
	        blbuf->le =  ((char *)blbuf + blsoftc.sc_size); /* reset le */
	 }
    }

    simple_unlock(&binlog_lock);
    splx(s);

    /* any data to output ? shouldn't be here if there isn't! */
    if (output_length)  { 
          uio->uio_rw = UIO_READ;
          rval = uiomove(output_start, output_length, uio);
    }

    /*
     * If nothing is pending and there is no data in the buffer when we are 
     * done with the uiomove than reset the buffer to the beginning.
     */
    s = splbinlog();
    simple_lock(&binlog_lock);
    if ((blbuf->pending == 0L) && (blbuf->in == blbuf->out))  {

	      /* Only the process that has previously done a BINLOG_SETPID
	       * ioctl is allowed to change buffer pointers.
	       */
	      if (u.u_procp->p_pid == binlog_savedpid)  {
                       blbuf->in =  ((char *)blbuf + BUFHDRSIZE);
                       blbuf->out = ((char *)blbuf + BUFHDRSIZE);
	               blbuf->le =  ((char *)blbuf + blsoftc.sc_size);
	      }
	      simple_unlock(&binlog_lock);
    }
    /*
     * If the buffer wrapped there could be more data at the beginning to
     * output.  If there is something also pending than it will all get
     * scheduled to be read later when binlog_valid() is called by the
     * pending event.
     */
    else if ((blbuf->pending == 0L) && (blbuf->in != blbuf->out))
	   binlog_wakeup();
    else simple_unlock(&binlog_lock);
    splx(s);

    return(rval);
}

/*
 * Function: binlog_write(dev,uio)
 *
 * Function description:  User mode access to the binary event logger.
 *
 *      The behaviour here is analgous to that with binary event logging
 *      done in the kernel....  the component doing the event logging
 *      is responsible for building all of the event record except the
 *      event record header, which binlog_alloc() does.  The data buffer
 *      the user mode application writes to /dev/kbinlog MUST fit this
 *      model... should use routine(s) in libbinlog.a for this.  No check
 *      is made here to see if does.... we just stick a valid record header
 *      and trailer on it and log it.
 *
 *      All events logged from user mode are given a severity of EL_PRILOW.
 *
 * Arguments:
 *	dev	device (not used)
 *	uio	pointer to uio structure
 *
 * Return value: errno value 
 *
 * Side effects: None.
*/
binlog_write(dev, uio) 
dev_t dev;
struct uio *uio; 
{
   register struct iovec *iov;
   register int rval = 0;
   register char *buf;
   char *binlog_alloc();

   /* only a privileged user can write to the kernel binary error log buffer */
#if     SEC_BASE
		if (!privileged(SEC_DEBUG, EPERM))
			return (EACCES);
#else
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif

   /* has event logging been initialized ? */
   if ((blsoftc.sc_state & BINLOG_BUFALLOCED) == 0)
	       return(ENOBUFS);

   iov = uio->uio_iov;

   /*
    *  Allocate space.  Since what we get includes the event-specific data
    *  section AND the 'sub_id' section we must subtract the sub_id size
    *  because binlog_alloc() only wants to be passed the size of the event
    *  specific data.
    */
   buf = binlog_alloc((iov->iov_len - EL_SUBIDSIZE), EL_PRILOW);
   if (buf != BINLOG_NOBUFAVAIL)  {
	    /*
	     *  Copy directly from user address space into the event record
	     *  buffer.  The header has been initalized by binlog_alloc()
	     *  so point after that for where to put the data.
	     */
	    rval = uiomove((buf + EL_RHDRSIZE), iov->iov_len, uio);
	    binlog_valid(buf);
   }
   else
	   return(ENOBUFS);
   return(rval);
}

/*
 * Function: binlog_ioctl(com, data, flag)
 *
 * Function description: Binary event logger control and status interface
 *
 * Arguments: 
 *	dev	device (not used)
 *	com 	command type specified by user
 *	data	argument passed by user, or loc to return value to user
 *	f_flag	not used
 *
 * Return value: SUCCESS or ERROR
 *
 * Side effects: None
 *
 * Restrictions: Must not be called with ipl above SPLHIGH.
 */
binlog_ioctl(dev, com, data, flag)
dev_t dev;
unsigned int com;
caddr_t data; 
int flag;
{
   register int s;
   int rval = 0;
   struct  binlog_getstatus *sc;


   if (getspl() > SPLHIGH) panic("binlog_ioctl: SPLEXTREME");


   /* has event logging been initialized ? */
   if ((blsoftc.sc_state & BINLOG_BUFALLOCED) == 0)
             return(ENOBUFS);

   switch (com) {            /* handle the specific ioctl command */

        case BINLOG_ENABLE:                        /* turn binary logging on */
#if     SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EACCES);
#else
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif
		s = splbinlog();
		simple_lock(&binlog_lock);
                blsoftc.sc_state |= BINLOG_ON;
 		binlog_onoff = 1;
	        simple_unlock(&binlog_lock);
	        splx(s);
		break;



       case BINLOG_DISABLE:                       /* turn binary logging off */
#if     SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EACCES);
#else
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif
		s = splbinlog();
		simple_lock(&binlog_lock);
		blsoftc.sc_state &= ~BINLOG_ON;
 		binlog_onoff = 0;
	        simple_unlock(&binlog_lock);
	        splx(s);
		break;


	case BINLOG_ASCIIENABLE:  /* enable logging of kernel ascii messages */
#if     SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EACCES);
#else
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif
		s = splbinlog();
		simple_lock(&binlog_lock);
	        blsoftc.sc_state |= BINLOG_ASCIION;
	        simple_unlock(&binlog_lock);
	        splx(s);
	        break;


	case BINLOG_ASCIIDISABLE: /* disable logging of kernel ascii messages */
#if     SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EACCES);
#else
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif
		s = splbinlog();
		simple_lock(&binlog_lock);
	        blsoftc.sc_state &= ~BINLOG_ASCIION;
	        simple_unlock(&binlog_lock);
	        splx(s);
	        break;


	case BINLOG_GETSTATUS:             /* return kernel event log status */
                sc = (struct binlog_getstatus *)data;
                sc->sc_state = blsoftc.sc_state;
                sc->sc_size = blsoftc.sc_size;
		sc->sc_nobufavail = blsoftc.sc_nobufavail;
		sc->sc_badsize = blsoftc.sc_badsize;
		sc->sc_readbusy = blsoftc.sc_readbusy;
		break;


        case BINLOG_CLRCNTRS:                       /* clear status counters */
#if     SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EACCES);
#else
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif
	       blsoftc.sc_nobufavail = 0;
	       blsoftc.sc_badsize = 0;
	       blsoftc.sc_readbusy = 0;
	       break;


	 /* The process that does this ioctl is the one that is allowed to
	  * do a read AND then move the buffer pointers.  Everybody else can
	  * just do a read.  The daemon should be the only one that does this.
	  */
         case BINLOG_SETPID:  
	 
#if     SEC_BASE
	       if (!privileged(SEC_DEBUG, EPERM))
			return (EACCES);
#else
	       if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif

	       binlog_savedpid =  *(long *)data;
	       break;


	default:                          /* invalid ioctl command */
	       rval = EINVAL;
	       break;
   }
   return(rval);
}

/*
 * Function: binlog_select(dev, rw)
 *
 * Function description:  Set return value so that select code sets select
 *                bits properly and wakes things up if there is data in the
 *                binary event log buffer.
 *
 * Arguments: 
 *	dev 	device (not used)
 *	rw	request type (read or write)
 *
 * Return value:
 *
 * Side effects: None
*/
binlog_select(dev, events, revents, scanning)
dev_t dev;
short   *events, *revents;
int     scanning;

{

	register int s;
	register long wakeup_allowed;

	if (*events & POLLNORM) {      /* select for a read */
		wakeup_allowed = (getspl() <= SPLHIGH);
		s = splbinlog();
		simple_lock(&binlog_lock);
		if (scanning) {
			if ((blbuf->pending == 0L) && 
			    (blbuf->in != blbuf->out)) {
				simple_unlock(&binlog_lock);
				*revents |= POLLNORM;
			}
			else {
				simple_unlock(&binlog_lock);
				select_enqueue(&blsoftc.sc_selq);
			}
		} else {
			simple_unlock(&binlog_lock);
			select_dequeue(&blsoftc.sc_selq);     /* clean up */
	        }
		splx(s);
	}
	if (*events & POLLOUT)         /* select for a write not allowed */
	       *revents |= POLLNVAL;

	return(0);
}

/***********************************************************************
 *                                                                     *
 *                     End of Driver Routines                          *
 *                                                                     *
 ***********************************************************************/

/*
 * Function: binlog_init()
 *
 * Function description: Allocate and initialize memory to be used for the
 *        kernel binary event log buffer.  This function should be called
 *        only once: as soon as kmem_alloc() can be used and before calls
 *        to the binary event logger are made.  Things still behave in a
 *        predictable and safe mannor if binlog_init()is not called, it 
 *        fails, or is not called soon enough.
 *
 *        No SMP locking is required here.  This function is only called
 *        once, during system initialization.
 *
 * Return value:  SUCCESS,  ERROR
*/
binlog_init()
{
    /* has buffer already been allocated ? */
    if (blsoftc.sc_state & BINLOG_BUFALLOCED)
	  return(SUCCESS);

    simple_lock_init(&binlog_lock);       /* initialize locks */

    /*  enforce minimum and maximun sizes for buffer */
    if (binlog_bufsize < BINLOG_BUFMIN)
	     binlog_bufsize = BINLOG_BUFMIN;
    else if (binlog_bufsize > BINLOG_BUFMAX)
	           binlog_bufsize = BINLOG_BUFMAX;


    /* get the buffer from non paged kernel pool */
    blbuf = (struct binlog_bufhdr *)kmem_alloc(kernel_map, binlog_bufsize);
    if (blbuf == NULL)  {              /* can't allocate the buffer */
       blsoftc.sc_state = 0;   /* disable the binary event logging */
       return(ERROR);
    }

    /* initialize the softc structure */
    blsoftc.sc_state = BINLOG_BUFALLOCED;
    blsoftc.sc_open = 0;
    blsoftc.sc_size = binlog_bufsize;
    blsoftc.sc_nobufavail = 0;
    blsoftc.sc_readbusy = 0;
    blsoftc.sc_seqnum = -1;
    queue_init(&blsoftc.sc_selq);

    blsoftc.sc_lwcid = lwc_create(LWC_PRI_BINLOG, &binlog_wakeup_lwc);
    if (blsoftc.sc_lwcid == LWC_ID_NULL) panic("binlog_init: lwc_id null");

    bzero ((char *)blbuf, blsoftc.sc_size);          /* clean the buffer */


    /* initialize the buffer header */
    blbuf->size = (long)blsoftc.sc_size;             /* the buffer size */
    blbuf->magic = BINLOG_MAGIC;                     /* validate buffer */
    blbuf->pending = 0L;
    blbuf->in =  ((char *)blbuf + BUFHDRSIZE);        /* input pointer */
    blbuf->out = ((char *)blbuf +BUFHDRSIZE);         /* output pointer */
    blbuf->le =  ((char *)blbuf + blsoftc.sc_size);   /* logical end */

    /* event logger ready for use ! */
    blsoftc.sc_state |=  (binlog_status & (BINLOG_ON | BINLOG_ASCIION));
    if (blsoftc.sc_state & BINLOG_ON)
	binlog_onoff = 1;   

    /*
     * kmem_alloc() returns a kernel mapped address.  Savecore(8) can't
     * deal with this.  Now that everything is all set to go it is safe to 
     * get and save away the physical address of the buffer.  When we reboot
     * after a crash savecore(8) will use this address to find and save away
     * on disk the kernel event log buffer.  The daemon, binlogd(8) when it
     * initializes will then read and process it.
     */
    blbufphysaddr = pmap_extract(kernel_pmap, blbuf);

    return(SUCCESS);
}

/*
 * Function: binlog_alloc(size, pri)
 *
 * Function description: Allocate space in the error log buffer for the 
 *	caller.  Caller must use no more space than it requests.
 *
 * Arguments: 
 *	size 	size of ONLY the event-specific data to be logged (caller 
 *              should use sizeof()).
 *
 *	pri	priority (severity) of event to be logged (severe, high, low);
 *
 * Return value: pointer to location in buffer, or EL_FULL if buffer full
 *
 * Side effects: None
 *
 * NOTE!  Be careful if putting any printf()'s or log()'s in here!  If 
 *        BINLOG_ASCIION is set it will cause the binlog_logmsg() function
 *        to get into an endless loop.
*/

char *binlog_alloc(asize,pri)
int asize;	/* size in bytes of record less the header and trailer */
int pri;        /* event priority */
{

    char *erp;                /* generic event record pointer */
    int size;                 /* total size of event record */
    int s;
    char *next;

    s = splbinlog();

    /*
     * do a series of safety checks before we actually allocate a place
     * to build a binary event record.  Make sure the buffer is allocated
     * (event logging is on) and logging is currently enabled, and that the
     * log buffer hasn't been trashed.
     */

    if (((blsoftc.sc_state & (BINLOG_BUFALLOCED|BINLOG_ON)) 
	 != (BINLOG_BUFALLOCED|BINLOG_ON))
	|| (blbuf->magic != BINLOG_MAGIC)) {
	    splx(s);
	    return(BINLOG_NOBUFAVAIL);
    }

    simple_lock(&binlog_lock);

    /*
     * If we get this far the event logger is running.  Increment the sequence
     * number regardless of whether the event gets logged.  When looking at
     * event log reports a sequence number gap indcates we missed logging 
     * something.
     */
    blsoftc.sc_seqnum++;

    /*
     * Calculate total record size - pad it to align for a 64-bit machine
     * so that the next allocation we do is on an appropriatly aligned 
     * boundry [for a 32 or 64 bit machine].
     */
    size = (asize + EL_MISCSIZE + 0x7) & (~0x7);

    /* The minimum size of an event record can be just headers and a trailer.
     * We don't know what the max record size could be, so just check that
     * we are dealing with something less than the kernel buffer size that we 
     * have.... we worry if there is actually room in the buffer for it later.
     */
     if ((size < EL_MISCSIZE) || (size > (blsoftc.sc_size - BUFHDRSIZE)))  {
             blsoftc.sc_badsize++;
             simple_unlock(&binlog_lock);
	     splx(s);
             return(BINLOG_NOBUFAVAIL);
     }

    /*
     * Allocate space for the event record
     *
     * The record allocation space in the kernel binary event log buffer
     * is handled as a circular buffer.  It is protected from overwriting
     * data in the buffer by returning a no-buffer-available condition
     * to the caller as appropriate.  Under normal conditions the buffer
     * won't wrap and you won't get a BINLOG_NOBUFAVAIL.  The system would
     * need to be heavely loaded so the daemon can't run often enough for
     * this to happen.  However, this is also moderated by the size of the
     * buffer... which is controlled by the 'binlog_bufsize' parameter in 
     * data/binlog_data.c.
     */
    erp = BINLOG_NOBUFAVAIL;
    next = blbuf->in + size;            /* predict next input location ptr */
    if (blbuf->out <= blbuf->in)   {    /* output ptr behind the input ptr */
	 /*
	  *  First see if there is room between current input pointer and
	  *  the logical end of the buffer... the usual situation.
	  */
	 if (next < blbuf->le)  {
	       erp = blbuf->in;
	       blbuf->in = next;       /* point to next input location */
	 }
	 /*
	  * See if there is any room at beginning of the buffer.  If there is
	  * than wrap around to the beginning and allocate space at the start.
	  * There may be pending data at the end of the buffer so make the 
	  * logical end the last input location.... this keeps the now unused 
	  * space at the end from being output by binlog_read().... all this
	  * can be a bit wasteful of space but it makes life easier by not
	  * breaking a record apart and puting some at the end of the buffer
	  * and the rest at the beginning!  The logical end ptr gets reset to
	  * be the physical buffer end once the data is output.
	  */
	 else   {
	       /* predict next input location ptr */
	       next = ((char *)blbuf + BUFHDRSIZE + size);

	       if (next < blbuf->out)  {
		        if (blbuf->out == blbuf->in)
			      /* no pending data - wrap output ptr */
			      blbuf->out = ((char *)blbuf + BUFHDRSIZE);
		        else
		              /* pending output, mark logical end */
		              blbuf->le = blbuf->in;
		        erp = ((char *)blbuf + BUFHDRSIZE);
		        blbuf->in = next;
	       }
	 }
    }

    /*
     * Input ptr is behind the output ptr - the buffer has already wrapped.
     * See if there is room to allocate space for this record.
     */
    else  { 
	   if (next < blbuf->out)  {
		erp = (blbuf->in);
		blbuf->in = next;       /* point to next input location */
	   }
    }

    if (erp != BINLOG_NOBUFAVAIL)  {
	 binlog_hdrinit(erp, size, pri);   /* initialize the event record */
	 blbuf->pending++;                 /* count the active event records */
    }
    else
	 blsoftc.sc_nobufavail++;          /* no buffer to be had */


    simple_unlock(&binlog_lock);
    splx(s);

    return(erp);
}

/*
 * Function: binlog_hdrinit(loc, length, priority)
 *
 * Function description:
 *       Initialize as much of the event record header as we can in the kernel.
 *       The host name is filled in by the daemon because we can log stuff
 *       before the hostname is set.  This is true for the time too,  when
 *       the system is booting we log stuff before the time is set, in which
 *       case the daemon will fill it in.  However, we always attempt to set it
 *       here so its usually as close to when the event actually happend as is 
 *       possible.  We also fill in the event record trailer here.
 *
 *             IT IS THE RESPONSIBILITY OF THE CALLER TO INSURE THAT A PLACE
 *             TO BUILD THE EVENT RECORD HAS AND BEEN ALLOCATED!
 *
 * Arguments:
 *      loc        A pointer to the location in the buffer where the space was
 *                 allocated in which to build the event record.
 *
 *      length     The total size (length) of the entire event record.  This is
 *                 also the amount of space allocated (no more, no less)
 *                 in which to build the event record.
 *
 *      priority   The priority (severity level) of the event being logged.
 *
 * Return value:  none
 *
 * Side effects: none
 *
 *
 * NOTE!  Be careful if putting any printf()'s or log()'s in here!  If 
 *        BINLOG_ASCIION is set it will cause the binlog_logmsg() function
 *        to get into an endless loop.
 *
*/

binlog_hdrinit(loc, length, priority)
char *loc;
int length;
int priority;
{

   register struct el_rhdr *p;
   register int whichcpu;
   register int sysid;

   /* get info about the processor doing the logging */
   whichcpu = cpu_number();
   sysid = ((machine_slot[whichcpu].cpu_subtype << 16) & 0xffff0000);
   sysid |= (machine_slot[whichcpu].cpu_type & 0x0000ffff);

   bzero(loc, length);       /* clean out any previous junk */

   /* fill in the event record header */
   p = (struct el_rhdr *)loc;
   p->rhdr_reclen = (short)length;
   p->rhdr_seqnum = (short)blsoftc.sc_seqnum;
   p->rhdr_time = (int)time.tv_sec;
   p->rhdr_sid = sysid;
   p->rhdr_valid = (char)(EL_INVALID);       /* validated by binlog_valid() */
   p->rhdr_pri = (char)priority;
   p->rhdr_elver = ELVER;
   p->rhdr_systype = 0;                      /* not used, always zero */
   p->rhdr_mpnum = machine_info.avail_cpus;           
   p->rhdr_mperr = whichcpu;

   /* Fill in the event record trailer.  We know its 4 bytes so we 
    * just do some casting and set it rather than do a bcopy.
    */
   loc += (length - EL_TRAILERSIZE);           /* point to the trailer */
   *(int *)loc = *(int *)RecordTrailer;        /* set the trailer */
   
}

/*
 * Function: binlog_valid(ep)
 *
 * Function description:  Last step in event record logging... marks record
 *        valid (complete) and updates event record buffer status.
 *
 * Arguments: 
 *	erp	The value returned from binlog_alloc(), the pointer to the
 *              event record being built.
 *
 * Return value:  None
 *
 * Side effects: None
 *
 * NOTE!  Be careful if putting any printf()'s or log()'s in here!  If 
 *        BINLOG_ASCIION is set it will cause the binlog_logmsg() function
 *        to get into an endless loop.
 *
*/

binlog_valid(buf)
char *buf;
{
    register int s;
    register struct el_rhdr *erp;


    erp = (struct el_rhdr *)buf;
    erp->rhdr_valid = EL_VALID;     /* validate event record */

    /*
     * Can only do the wakeup directly if we started at or below splhigh,
     * where the scheduler runs.  Otherwise, have to defer the wakeup.  The
     * wakeup is deferred with the lwc utility, which will invoke the
     * binlog_wakeup_lwc routine when returning to user mode.
     *
     * Requires that lwc be protected at splextreme().
     */
    s = splbinlog();
    simple_lock(&binlog_lock);
    blbuf->pending--;                      /* update buffer status */
    simple_unlock(&binlog_lock);
    splx(s);
    lwc_interrupt(blsoftc.sc_lwcid);	   /* defer until back to spl0 */
}

/*
 * Function: binlog_wakeup()
 *
 * Function description:  Wakeup readers that the driver made wait,
 *                        usually just the daemon.
 *
 * Arguments: None
 *
 * Return value: None
 *
 * Preconditions: Must be called at splbinlog with the binlog_lock taken.
 * 
 * Side effects: The binlog_lock is released.
 *
 * NOTE!  Be careful if putting any printf()'s or log()'s in here!  If 
 *        BINLOG_ASCIION is set it will cause the binlog_logmsg() function
 *        to get into an endless loop.
 *
*/
static void binlog_wakeup()
{
	register long readers_waiting;

	if ((blsoftc.sc_open) == 0) {
		simple_unlock(&binlog_lock);
		return;
	}
	if (readers_waiting = (blsoftc.sc_state & BINLOG_RDWAIT)) {
		blsoftc.sc_state &= ~BINLOG_RDWAIT;
	}
	simple_unlock(&binlog_lock);
	select_wakeup(&blsoftc.sc_selq);
	if (readers_waiting) wakeup((caddr_t)blbuf);
}

/*
 * Function: binlog_logmsg()
 *
 * Function description:   This function steals a copy of ascii messages 
 *       from the kernel syslog buffer.  The kernel prf() routine
 *       has already done all the formatting work and nicely put the 
 *       ascii string in a buffer for us.
 *
 *       When the system is booting we want to capture ALL the ascii messages
 *       that result from the startup() routine running... there are a whole
 *       bunch of them! So, in init_main.c in setup_main() after we have 
 *       called binlog_init() we call this routine with arguements such 
 *       that we make a copy of the entire syslog buffer.
 *
 *       For all other situations we only want a single ascii message 
 *       string, the last one just put into the syslog buffer.  There are 
 *       hooks in the prf() routine (binlog_begin, binlog_end) that save 
 *       the beginning and end locations of the string to copy.
 *
 *  Arguments:
 *        class     The subid_clsss element of struct el_sub_id.  This
 *                  The class code of the event being logged.  See
 *                  sys/binlog.h from these codes.
 *
 *         begin    The offset in the kernel syslog buffer where the
 *                  message we are stealing a copy of starts.
 *
 *         end      The offset in the kernel syslog buffer where the
 *                  message we are stealing a copy of ends.
 *
 * Return value: None
 *
 * Side effects: None
 *
 *
 * NOTE!  Be careful if putting any printf()'s or log()'s in here!  If 
 *        BINLOG_ASCIION is set it will cause the binlog_logmsg() function
 *        to get into an endless loop.
 *
*/

binlog_logmsg(class, ptr, size)
int class;		/* class of the ascii message */
char *ptr;		/* start of message to log */
int size;		/* size of message to log */
{
  struct msgevent {                       /* event record template */
	     struct el_rhdr    elrhdr;    /* record header */
	     struct el_sub_id  elsubid; 
	     struct el_msg     elmsg;     /* the ascii message being logged */
  };
  register struct msgevent *msg;


  /* Has binary event logger been initialized ? */
  if ((blsoftc.sc_state & BINLOG_BUFALLOCED) == 0) 
	return;
  /*
   * Always log panic messages and startup messages.  Otherwise, also log 
   * ascii messages (kernel printf's) when doing so is enabled... default = off.
   */
  if (((blsoftc.sc_state & BINLOG_ASCIION) == 0) && (class != ELMSGT_SU) &&
						       (class != ELMSGT_PANIC))
	return;

  if (!size)
	size = strlen(ptr);
  msg = (struct msgevent *)binlog_alloc(
	sizeof(msg->elmsg.msg_len) + sizeof(msg->elmsg.msg_type) + 1 + size,
	(class == ELMSGT_PANIC) ? EL_PRISEVERE : EL_PRILOW);
  if ((char *)msg != BINLOG_NOBUFAVAIL)  {
	LSUBID(msg, (short)class, EL_UNDEF, EL_UNDEF, EL_UNDEF, EL_UNDEF,
								EL_UNDEF);
	msg->elmsg.msg_len = size + 1;
	msg->elmsg.msg_type = EL_UNDEF; /* msg type not currently used */
	bcopy((caddr_t)ptr, (caddr_t)&msg->elmsg.msg_asc[0], size);
	msg->elmsg.msg_asc[size] = '\0'; /* null terminated string */
	binlog_valid((char *)msg);
  }
}

/*
 * Function: binlog_wakeup_lwc()
 *
 * Function description:
 * 	This function is called from the lwc scheduler just before return to
 *	user mode.  The lwc scheduler is told to call this function whenever
 *	a message is logged from splextreme().  The lwc dispatcher must be
 *	synchronized at splextreme() for this to work right.
 * Arguments:	 None
 *
 * Return value: None
 *
 * Side effects: None
 *
 */
static void binlog_wakeup_lwc()
{
	register int s;
	register long wakeup_needed, readers_waiting;

	lwc_rfc(blsoftc.sc_lwcid);
	s = splbinlog();
	simple_lock(&binlog_lock);
	if (binlog_onoff &&			/* logging is enabled */
	    (blsoftc.sc_open > 0) &&		/* potential readers */
	    (blbuf->in != blbuf->out)) {	/* buffer nonempty */
		binlog_wakeup();
	}
	else simple_unlock(&binlog_lock);
	splx(s);
}
