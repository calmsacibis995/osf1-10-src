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
static char *rcsid = "@(#)$RCSfile: mbox.c,v $ $Revision: 1.2.9.4 $ (DEC) $Date: 1993/11/19 13:36:08 $";
#endif

/*
 * Revision History: 
 *	Sept-1991 jaa:	creation
 *
 */

#include <vm/vm_kern.h>
#include <mach/kern_return.h>
#include <machine/machparam.h>
#include <io/dec/mbox/mbox.h>
#include <kern/lock.h>

/*** TODO: change the printf's to log to the console, ascii log and uerf ***/

decl_simple_lock_data(,mbox_avpool_lock);
#ifdef __RELEASE_HAS_LOCKINFO
decl_simple_lock_info(,mbox_avpool_lockinfo);
#endif

#define AVPOOL_FREE(mbp) mbox_add_to_avpool(mbp)
#define AVPOOL_ALLOC() mbox_alloc_from_avpool()
#define AVPOOL_PENDING_FREE(mbp) mbox_add_to_pending_list(mbp)
void mbox_add_to_avpool();
void mbox_add_to_pending_list();
mbox_t mbox_alloc_from_avpool();
mbox_t mbox_get_from_pending_list();

#define POOL_SIZE 32
	/* Max number of outstanding concurrent mbox
	 * accesses.  This should become a config
	 * time value.
	 */

mbox_cnt_avpool = 0;
mbox_cnt_avpool_min = POOL_SIZE;
mbox_cnt_pending = 0;
mbox_cnt_pending_max = 0;

/*
 * synchronous read of a csr
 */
u_long
rdcsr(type_as, ptr, remaddr)
	register u_int type_as;    /* data type, write mask and addr space */
	register struct bus_ctlr_common *ptr; /* bus/ctlr this csr access is for */
	register u_long remaddr;              /* remote bus addr */
{
	register mbox_t mbp2 = (mbox_t)ptr->mbox;
	register mbox_t mbp;
	u_long return_data;
	vm_offset_t saved_pa;
	int stat;

	mbp = AVPOOL_ALLOC();
	saved_pa = mbp->mbox_pa;
	*mbp = *mbp2;
	mbp->mbox_pa = saved_pa;
		/* simulate a hardware done transaction */
	mbp->mb_status = 1;
#ifdef MBOX_DEBUG
	if(mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("rdcsr: this mbox is already in use!!!");
	}
	mbp->mbox_inuse = 1;
#endif /* MBOX_DEBUG */

	stat = mbox_setup(RD_CSR, type_as, mbp, remaddr, 0);
	if(stat != MBOX_SUCCESS)
		mbox_error(mbp, "rdcsr", stat);
	/* reads are synchronous, so we wait awhile for it to complete */
	MBOX_WAIT(mbp, "rdcsr");
	if(mbp->mb_status & MBOX_ERR_BIT) 
		mbox_error(mbp, "rdcsr", MBOX_ERR);
#ifdef MBOX_DEBUG
	if( ! mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("rdcsr: mbox in use corruption!!!");
	}
	mbp->mbox_inuse = 0;
#endif /* MBOX_DEBUG */
	return_data = (u_long)(mbp->rdata);
	AVPOOL_FREE(mbp);
	return(return_data);
}

/*
 * write (dump and run) to a remote csr
 */
void
wrtcsr(type_as, ptr, remaddr, wrtdata)
	register u_int type_as;    /* data type, write mask and addr space */
	register struct bus_ctlr_common *ptr;/* bus/ctlr this csr access is for */
	register u_long remaddr;    /* remote bus addr */
	register u_long wrtdata;
{
	register mbox_t mbp;
	register mbox_t mbp2 = (mbox_t)ptr->mbox;
	vm_offset_t saved_pa;
	int stat;

	mbp = AVPOOL_ALLOC();
	saved_pa = mbp->mbox_pa;
	*mbp = *mbp2;
	mbp->mbox_pa = saved_pa;
		/* simulate a hardware done transaction */
	mbp->mb_status = 1;
#ifdef MBOX_DEBUG
	if(mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("wrtcsr: this mbox is already in use!!!");
	}
	mbp->mbox_inuse = 1;
#endif /* MBOX_DEBUG */

	stat = mbox_setup(WRT_CSR, type_as, mbp, remaddr, wrtdata);
	if(stat != MBOX_SUCCESS)
		mbox_error(mbp, "wrtcsr", stat);
#ifdef MBOX_DEBUG_WRITES
	/* make writes synchronous for debugging */
	MBOX_WAIT(mbp, "wrtcsr: synch write");
	if(mbp->mb_status & MBOX_ERR_BIT) 
		mbox_error(mbp, "wrtcsr: synch write", MBOX_ERR);
#endif /* MBOX_DEBUG_WRITES */
#ifdef MBOX_DEBUG
	if( ! mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("wrtcsr: mbox in use!!!");
	}
	mbp->mbox_inuse = 0;
#endif /* MBOX_DEBUG */
	AVPOOL_PENDING_FREE(mbp);
}

/*
 * this is a synchronous version of wrtcsr to force synchronous writes.
 * primarily to be used by busses that need software to do busy retries
 * the adapter writer should provide a mbox err_rtn to look for
 * a busy status and perform the retry logic.
 */
void
wrtcsrs(type_as, ptr, remaddr, wrtdata)
	register u_int type_as;   /* data type, write mask and addr space */
	register struct bus_ctlr_common *ptr;/* bus/ctlr this csr access is for */
	register u_long remaddr;             /* remote bus addr */
	register u_long wrtdata;
{
	register mbox_t mbp2 = (mbox_t)ptr->mbox;
	register mbox_t mbp;
	vm_offset_t saved_pa;
	int stat;

	mbp = AVPOOL_ALLOC();
	saved_pa = mbp->mbox_pa;
	*mbp = *mbp2;
	mbp->mbox_pa = saved_pa;
		/* simulate a hardware done transaction */
	mbp->mb_status = 1;
#ifdef MBOX_DEBUG
	if(mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("wrtcsr: this mbox is already in use!!!");
	}
	mbp->mbox_inuse = 1;
#endif /* MBOX_DEBUG */

	stat = mbox_setup(WRT_CSR, type_as, mbp, remaddr, wrtdata);
	if(stat != MBOX_SUCCESS)
		mbox_error(mbp, "wrtcsrs", stat);
	MBOX_WAIT(mbp, "wrtcsrs");
	if(mbp->mb_status & MBOX_ERR_BIT) 
		mbox_error(mbp, "wrtcsrs", MBOX_ERR);
#ifdef MBOX_DEBUG
	if( ! mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("wrtcsr: mbox in use!!!");
	}
	mbp->mbox_inuse = 0;
#endif /* MBOX_DEBUG */
	AVPOOL_FREE(mbp);
}

int
mbox_setup(cmd, type_as, mbp, remaddr, wrtdata)
	register int cmd;          /* command for remote bus */
	register u_int type_as;   /* data type, write mask and addr space */
	register mbox_t mbp;
	register u_long remaddr;           /* remote bus addr */
	register u_long wrtdata;           /* data to be written */	
{
	register boolean_t stat;

	/* wait for any previous writes or asynch reads by this ctlr */
	MBOX_WAIT(mbp, "mbox_setup");
	/* whatever we had done before, didn't work */
	if(mbp->mb_status & MBOX_ERR_BIT) 
		mbox_error(mbp, "mbox_setup", MBOX_ERR);
	mbp->mask = ~(type_as & MBOX_MASK_BITS);
	mbp->rbadr = remaddr;
	mbp->wdata = wrtdata;
	mbp->rdata = (u_long)0;
	/* the adapter MUST fill in the mailbox command field */
	(*mbp->mbox_cmd)(mbp, cmd, type_as);
	/* start the hardware */
	stat = MBOX_GO(mbp);
	return((stat == 0) ? MBOX_QFULL : MBOX_SUCCESS);
}

/* rdcsra does not use the avail_pool of mboxes because
 *   we cannot know when to return the mbox to the avail_pool.
 *   It is the responsibility of the rdcsra user to ensure
 *   that there are no smp or thread/interrupt or 
 *   thread/thread mailbox corruption problems.
 */
void
rdcsra(type_as, ptr, remaddr)
	register u_int type_as;   /* data type, write mask and addr space */
	register struct bus_ctlr_common *ptr;/* bus/ctlr this csr access is for */
	register u_long remaddr;             /* remote bus addr */
{
	register mbox_t mbp = (mbox_t)ptr->mbox;
	int stat;

	stat = mbox_setup(RD_CSR, type_as, mbp, remaddr, 0);
	if(stat != MBOX_SUCCESS)
		mbox_error(mbp, "rdcsra", stat);
}

boolean_t
mbox_badaddr(addr, len, mbp)
	register caddr_t addr;
	register int len;
	register mbox_t mbp;
{
	register u_int retries, type_as;
	register boolean_t stat;
	
#ifdef MBOX_DEBUG
	if(mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("rdcsr: this mbox is already in use!!!");
	}
	mbp->mbox_inuse = 1;
#endif /* MBOX_DEBUG */
	retries = 0;
	while((mbp->mb_status & MBOX_DON_BIT) == 0) {
		if(retries++ >= mbp->bus_timeout)
			mbox_error(mbp, "mbox_badaddr", MBOX_HUNGHOSE);
		DELAY(1);
	} 
	if(mbp->mb_status & MBOX_ERR_BIT) 
		mbox_error(mbp, "mbox_badaddr", MBOX_ERR);

	mb();
	mbp->cmd = RD_CSR;
	mbp->rbadr = (u_long)addr;
	mbp->wdata = 0;
	mbp->rdata = (u_long)0;
	/* len has already been verified by cpu->badaddr */
        switch (len) {
        case 1: type_as = BYTE_32;
                break;
        case 2: type_as = WORD_32;
                break;
        case 4: type_as = LONG_32;
                break;
        case 8: type_as = QUAD_32;
                break;
        default:
                type_as = LONG_32;
                break;
        }
	mbp->mask = ~(type_as & MBOX_MASK_BITS);
	(*mbp->mbox_cmd)(mbp, RD_CSR, type_as);
	if(MBOX_GO(mbp) == 0) /* can't get it into the queue */
		panic("mbox_badaddr: can't probe");
	retries = 0;
	while((mbp->mb_status & MBOX_DON_BIT) == 0) {
		if(retries++ >= mbp->bus_timeout)
			mbox_error(mbp, "mbox_badaddr: 2", MBOX_HUNGHOSE);
		DELAY(1);
	} 
	mb();              /* might not be necessary... */
	stat = mbp->mb_status & MBOX_ERR_BIT;
	/*** clear the error bit BUT leave the done bit set ***/
	mbp->mb_status = 1;
#ifdef MBOX_DEBUG
	if( ! mbp->mbox_inuse) {
		dumpmbox(mbp);
		panic("mbox_badaddr: mbox in use corruption!!!");
	}
	mbp->mbox_inuse = 0;
#endif /* MBOX_DEBUG */
	return(stat);
}

/* 
 * get mbox off free list.  primarily called during sys config
 * if the mailbox pool is empty, 
 *     allocate another page and break it up into mboxes
 *     and put them onto the free list
 * NB: This routine calls back to the vm subsystem to allocate
 * memory for the mailbox pool and could sleep
 * DO NOT call this routine from an ISR
 */
mbox_t
mbox_alloc(ptr)
	register struct bus_ctlr_common *ptr; /* bus/ctlr */
{
	int i;
	register mbox_t mbp;
	int s = splbio();
        extern vm_offset_t Mbox_free_list;
	static mbox_availpool_initialized = 0;
	
	if( !mbox_availpool_initialized ) {
		mbox_availpool_initialized = 1;
#ifdef __RELEASE_HAS_LOCKINFO
		simple_lock_setup(mbox_avpool_lock, mbox_avpool_lockinfo);
#else
		simple_lock_init(mbox_avpool_lock);
#endif
		for(i=0; i< POOL_SIZE; i++){
			mbox_add_to_avpool(mbox_alloc((struct bus_ctlr_common *)0));
		}
	}
	if((mbp = (mbox_t)Mbox_free_list) == (mbox_t)0) {
		register mbox_t fmbp;
		register caddr_t cp;
		register int i;

		splx(s);
		if(sizeof(struct mbox) % 64)
			panic("mbox_alloc: bad mbox size");

		/* start on a pg boundary to get 64 byte alignment */
		if((cp = (caddr_t)kmem_alloc(kernel_map, NBPG)) == (caddr_t)0)
			panic("mbox_alloc: no memory for mailbox pool");

		bzero(cp, NBPG);
		i = NBPG/sizeof(struct mbox);
		for(fmbp = (mbox_t)cp; i; --i, fmbp++) {
			if(svatophys(fmbp, &fmbp->mbox_pa) != KERN_SUCCESS)
				panic("mbox_alloc: svatophys");
			/* simulate a hardware done transaction */
			fmbp->mb_status = 1;
			MBOX_FREE(fmbp);
		}
		s = splbio();
		mbp = (mbox_t)Mbox_free_list;
	}
	Mbox_free_list = *(vm_offset_t *)mbp;
	*(vm_offset_t *)mbp = 0;
	splx(s);
	return(mbp);
}

/* 
 * put mbox onto free list 
 */
void
mbox_free(mbp)
	register vm_offset_t mbp;
{
	int s = splbio();
        extern vm_offset_t Mbox_free_list;

	*(vm_offset_t *)mbp = Mbox_free_list;
	Mbox_free_list = (vm_offset_t)mbp;
	splx(s);
}

boolean_t
mbox_go(mbp)
	register mbox_t mbp;
{
	register int retry;
	register boolean_t stat;
	extern struct mbox_debug mbox_debug[];
	extern active_mbox;

#ifdef MBOX_DEBUG
	/* 
	 * save the pa so we can debug at the console and 
	 * the va for dumps 
	 */
	mbox_debug[cpu_number()].mbd[active_mbox].pa = mbp->mbox_pa;
	mbox_debug[cpu_number()].mbd[active_mbox++].va = (vm_offset_t)mbp;
	active_mbox %= 8;
#endif /* MBOX_DEBUG */

	/* hardware own's the mailbox */
	/* 
	 * NB: the stqc routine in pal_lib.s does the mb to 
	 * guarantee ordering 
	 */
	mbp->mb_status = 0;
	for(stat = 0, retry = 0; retry < 20; retry++) {
	    	/* 
		 * See Section 8.3.1 Mailbox Posting in the
		 * SRM for an explanation of mailbox access
		 * and the use of stq_c instruction in this 
		 * context.
		 */
		if((stat = stqc(mbp->mbox_reg, mbp->mbox_pa))) 
			break;
		DELAY(1);
#ifdef MBOX_DEBUG
		printf("mbox_go: stq_c faild, retrying %d\n", retry + 1);
#endif MBOX_DEBUG
	}
	return(stat);
}

boolean_t
check_mbox(ptr, retdata)
	register struct bus_ctlr_common *ptr; /* bus/ctlr this csr access is for */
	register u_long *retdata;
{
	register mbox_t mbp = (mbox_t)ptr->mbox;

	if(mbp->mb_status & MBOX_DON_BIT) {
		mb();
		*retdata = mbp->rdata;
		return(1);
	}
	return(0);
}

void
null_mbox_cmd(mbp, cmd, type_as)
	register mbox_t mbp;
	register u_int cmd;
	register u_int type_as;
{
	mbp->cmd = cmd;
#ifdef MBOX_DEBUG
	printf("null_mbox_cmd: mbox 0x%x cmd 0x%x type_as 0x%x\n",
		mbp, cmd, type_as);
	dumpmbox(mbp);
#endif /* MBOX_DEBUG */
}

static char *mbox_panicstr[] = {
	"success", /* place holder */
	"hunghose", 
	"qfull", 
	"mbox_error",
	""
};

/*
 * common error leg
 * TODO: add error retry logic
 */
mbox_error(mbp, rtn, err)
	register mbox_t mbp;
	register char *rtn;
	register int err;
{
	if(mbp->err_rtn) {
		/* sanity */
		if(mbp->bus_ctlr_ptr == 0) {
			dumpmbox(mbp);
			printf("%s: ", rtn);
			panic("mbox_error: no bus/ctlr for error handler");
		}
		return((*mbp->err_rtn)(mbp, rtn, err));
	}
	dumpmbox(mbp);
	printf("%s: ", rtn);
	panic(mbox_panicstr[err]);
}

void
dumpmbox(mbp)
	register mbox_t mbp;
{
	printf("dumpmbox:\n\tcommand 0x%x\tmask 0x%x\those %d\n", 
	       mbp->cmd, mbp->mask, mbp->hose);
	printf("\trbadr 0x%l016x\n\twdata 0x%l016x\trdata 0x%x\n", 
	       mbp->rbadr, mbp->wdata, mbp->rdata);
	printf("\tstatus 0x%l016x\n\tbus_timeout %d\n", 
	       mbp->mb_status, mbp->bus_timeout);
	printf("\tmbpr 0x%l016x\n\tmbox_pa 0x%l016x\n",
	       mbp->mbox_reg, mbp->mbox_pa);
	printf("\tmbox_cmd %l016x\n\terr_rtn 0x%l016x\n",
	       mbp->mbox_cmd, mbp->err_rtn);
	DELAY(3000000);
}

	/* Available list of mbox structures.
	   Mbox structures are not locked to prevent
	   corruption by interrupt code or by multiple
	   threads.  We now provide a non-shared mbox
	   for each mbox IO access to prevent corruption.
	   The pending list is for mboxes that can be
	   returned to the available list after the done
	   bit gets set.  We did not wait for the transaction
	   to complete.  So far, only the async write case
	   (wrtcsr) puts mbox structures on the pending list.
	 */
mbox_t Mbox_avpool_list = 0;
mbox_t Mbox_pending_list = 0;

static void
mbox_add_to_avpool(mbp)
mbox_t mbp;
{
	int s = splhigh();
	simple_lock(mbox_avpool_lock);
	*(vm_offset_t *)mbp =
		(vm_offset_t)Mbox_avpool_list;
	Mbox_avpool_list = (mbox_t)mbp;
	++mbox_cnt_avpool;
	simple_unlock(mbox_avpool_lock);
	splx(s);
}

static
mbox_t
mbox_alloc_from_avpool()
{
	int s = splhigh();
	register mbox_t mbp;
	simple_lock(mbox_avpool_lock);
	mbp = mbox_get_from_pending_list();
	if(mbp==(mbox_t)0){
	    mbp=(mbox_t)Mbox_avpool_list;
	    if(mbp==(mbox_t)0){
		panic("Mbox available pool is depleted");
	    }
	    Mbox_avpool_list = (mbox_t)(*(vm_offset_t *)mbp);
	    *(vm_offset_t *)mbp = (vm_offset_t)0;
	    if(--mbox_cnt_avpool < mbox_cnt_avpool_min)
		mbox_cnt_avpool_min = mbox_cnt_avpool;
	}
	simple_unlock(mbox_avpool_lock);
	splx(s);
	return(mbp);
}
		

static mbox_t
mbox_get_from_pending_list()
{
	mbox_t mbp, mbp_prev;

	/* locks are done in the calling function */
	/*   mbox_alloc_from_avpool() */
	mbp_prev = (mbox_t)0;
	mbp = Mbox_pending_list;
	while(mbp){
	    if(mbp->mb_status & MBOX_DON_BIT) {
			/* check for mbox errors here */
		if(mbp->mb_status & MBOX_ERR_BIT){
			mbox_error(mbp,
			  "wrtcsr: synch write completion",
			  MBOX_ERR);
		}
			/* remove from list */
		if(mbp == Mbox_pending_list){
		    Mbox_pending_list = (mbox_t)mbp->next;
		} else {
		    mbp_prev->next = mbp->next;
		}
		mbp->next = (mbox_t)0;
		mbox_cnt_pending--;
		return(mbp);
	    }
	    mbp_prev = mbp;
	    mbp = (mbox_t)mbp->next;
	}
	return((mbox_t)0);
}

void
mbox_add_to_pending_list(mbp)
register mbox_t mbp;
{
	int s = splhigh();
	simple_lock(mbox_avpool_lock);
	mbp->next = (mbox_t)Mbox_pending_list;
	Mbox_pending_list = (mbox_t)mbp;
	if(++mbox_cnt_pending > mbox_cnt_pending_max)
		mbox_cnt_pending_max = mbox_cnt_pending;
	simple_unlock(mbox_avpool_lock);
	splx(s);
}
