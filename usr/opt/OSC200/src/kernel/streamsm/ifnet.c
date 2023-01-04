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
static char *rcsid = "@(#)$RCSfile: ifnet.c,v $ $Revision: 1.1.12.3 $ (DEC) $Date: 1993/08/27 20:00:34 $";
#endif
/* -- history from OSF --
 * Revision 1.1.7.2  1992/10/22  14:02:27  tmt
 * 	Many many changes to fix leaks, close holes, do flow control
 * 	properly, for compiling with MACH_ASSERT==0, remove unnecessary
 * 	real_tsleep(), fix timing hole in LOCK_MP(), add static where
 * 	appropriate, fix declarations to match functions, go back to
 * 	QUEUEPAIR (bug in streams fixed), use SEC_REMOTE not SEC_LIMIT
 * 	and check for error.  Tested and operational under MP config.
 * 	Add hooks for supporting Sequent vernacular.
 * 	[1992/10/22  14:01:41  tmt]
 */

/*
 * Include files for ifnet layer and stream modules.
 */
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <kern/lock.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h> 
#include <sys/dlpihdr.h> 
#include <streamsm/ifnet.h>

/*
 * Hack for Sequent device and protocol
 */
#ifdef	IFNET_SQT
#include <i386/SQT/eth_prim.h>
#undef	IFNET_DEV_NAME
#define	IFNET_DEV_NAME	"eg"
#define ifnet_configure	ifnet_sqt_configure
/* Marginally cleaner with this... */
#define DL_primitives	primitives
#define	dl_primitive	type
#endif


/*
 * internal data structures
 */
typedef struct unit_control_block 
{
    struct arpcom u_is_ac;
#define is_ac   u_is_ac			
#define is_if   u_is_ac.ac_if             /* network-visible interface    */
#define is_addr u_is_ac.ac_hwaddr         /* hardware address    */
#define is_ipaddr u_is_ac.ac_ipaddr       /* IP address (XXX)    */
} ucb_t, *ucbPtr;

typedef struct ifnet_queues
{
    struct ifnet_queues *next, *prev;/* linked list */
    queue_t         *ifnet_write_q;  /* attached streams queue */
    int             unit;            /* unit number of downstream device*/
    u_short         protocol;        /* protocol bound to this stream */
    u_short         uprotocol;       /* upper bound of same for SQT */
    short           state;           /* BOUND, ATTACH... */
    u_char          hwaddr[14];      /* hwaddr saved for later attach */
    int             ref_cnt;         /* # of threads using this context */
    struct {
        mblk_t      *mp;             /* place to put driver response */
	int         state;           /* true if resp is locked */
    } resp;
    ucbPtr          ucb;             /* ptr to ucb struct for this unit */
    decl_simple_lock_data(,lock)     /* spin lock for ref_cnt and resp */
} context_t, *contextPtr;

#define IFNET_SOCKETS   1
#define IFNET_UPSTREAM  2 
#define IFNET_WAKEUP    3
#define IFNET_ERROR     4
#define IFNET_DISCARD   5

#define LOCK_LINKED_LIST()           \
do {                                 \
	x = splimp();                \
	simple_lock(&list_lock);     \
} while (0)

#define UNLOCK_LINKED_LIST()         \
do {                                 \
	simple_unlock(&list_lock);   \
	splx(x);                     \
} while (0)

#define GRAB_CONTEXT_STRUCTURE(p)    \
do {                                 \
        int x;                       \
	x = splimp();                \
	simple_lock(&p->lock);       \
	p->ref_cnt++;                \
	simple_unlock(&p->lock);     \
	splx(x);                     \
} while (0)

#define UNGRAB_CONTEXT_STRUCTURE(p)  \
do {                                 \
        int x;                       \
	x = splimp();                \
	simple_lock(&p->lock);       \
	p->ref_cnt--;                \
	simple_unlock(&p->lock);     \
	splx(x);                     \
} while (0)

/*
 * static data 
 */
decl_simple_lock_data(static, list_lock)  /* lock for context linked list */
static struct { contextPtr next, prev; } list_head;
static ucb_t ifnet_ucb[MAX_UNITS];
static char ifnetName[] = IFNET_NAME;
static char devName[]   = IFNET_DEV_NAME;

/* 
 * Debug 
 */
#if MACH_ASSERT
int ifnet_log = 0;
#define IFNET_ALL      ~0x00000000
#define IFNET_NONE      0x00000000
#define IFNET_READ      0x00000001
#define IFNET_WRITE     0x00000002
#define IFNET_OPEN      0x00000004
#define IFNET_IOCTL     0x00000008
#define IFNET_RESET     0x00000010
#define IFNET_WATCH     0x00000040
#define IFNET_OUTPUT    0x00000080
#define IFNET_DUMP_DATA 0x00000100
#define IFNET_CLOSE     0x00000200
#define IFNET_TRACE(f, v)  do { if (ifnet_log & (f)) { v } } while (0)
#define static
#else
#define IFNET_TRACE(f, v)
#endif

/*
 * declare forward references
 */
int		ifnet_configure(sysconfig_op_t,
				str_config_t *, size_t, str_config_t *, size_t);
static int	ifnet_ioctl(struct ifnet *, int, caddr_t);
static int	ifnet_output(struct ifnet *);
static int	ifnet_open(queue_t *, dev_t *, int, int, cred_t *);
static int	ifnet_close(queue_t *, int, cred_t *);
static int	ifnet_rput(queue_t *, mblk_t *);
static int	ifnet_wput(queue_t *, mblk_t *);
static int	ifnet_wsrv(queue_t *);
static int	do_driver_request(contextPtr, mblk_t **);
static int	find_rep_type(mblk_t *);
static void	attach_ifnet(unsigned int, contextPtr);
static int	dlpi_get_phys_addr_req(int, contextPtr, u_char *);
static int	dlpi_set_phys_addr_req(contextPtr, u_char *, int);


/*
 * Module Declarations for Stream Modules
 */
static struct module_info ReadModuleInfo = {
    IFNET_ID,         /* Module Id number */
    ifnetName,          /* Module Name */
    ifnetMinPacket,     /* min packet size accepted */
    ifnetMaxPacket,     /* max packet size accepted */
    ifnetHiWater,       /* hi-water mark, for flow control */
    ifnetLoWater        /* low-water mark, for flow control */
};    

static struct module_info WriteModuleInfo = {
    IFNET_ID,        /* Module Id number */
    ifnetName,         /* Module Name */
    ifnetMinPacket,    /* min packet size accepted */
    ifnetMaxPacket,    /* max packet size accepted */
    ifnetHiWater,      /* hi-water mark, for flow control */
    ifnetLoWater       /* low-water mark, for flow control */
};    

/*
 * Queue Init 
 */
static struct qinit ReadInit = {
        ifnet_rput,         /* put procedure */
        NULL,               /* service procedure (none) */
        ifnet_open,         /* called on each open or a push */
        ifnet_close,        /* called on last close or a pop */ 
        NULL,                   /* reserved */
        &ReadModuleInfo,        /* Information Structure */
        NULL                    /* stats structure */
};

static struct qinit WriteInit = {
            ifnet_wput,         /* put procedure */
            ifnet_wsrv,         /* service procedure */
            NULL,            /* called on each open or a push */
            NULL,            /* called on last close or a pop */ 
            NULL,            /* reserved */
            &WriteModuleInfo,/* Information Structure */
            NULL             /* stats structure */
};

struct streamtab ifnetinfo = { 
            &ReadInit,     
            &WriteInit, 
            NULL, 
            NULL
};


/*
 *        i f n e t _ c o n f i g u r e
 *
 * Called at this entry point by the STREAMS subsytem
 * configuration code.
 *
 * Inputs:
 *        op         = Options for system configuration
 *        indata     = Pointer to configuration input buffer
 *        indatalen  = Byte size of input data buffer
 *        outdata    = Pointer to configuration output buffer
 *        outdatalen = Byte size of output data buffer
 *
 * Outputs:
 *
 *        TBD
 *
 * Returns:
 *       
 *        0, if success, ERRNO error otherwise.
 */
int
ifnet_configure(op, indata, indatalen, outdata, outdatalen)
	sysconfig_op_t  op;
	str_config_t *  indata;
	size_t          indatalen;
	str_config_t *  outdata;
	size_t          outdatalen;
{
	struct streamadm	sa;
	dev_t			devno;

	if (op != SYSCONFIG_CONFIGURE)
		return EINVAL;

	devno = NODEV;
	sa.sa_version		= OSF_STREAMS_10;
	sa.sa_flags		= STR_IS_MODULE | STR_SYSV4_OPEN;
	sa.sa_ttys		= 0;
	sa.sa_sync_level	= SQLVL_QUEUEPAIR;
	sa.sa_sync_info		= 0;

	strcpy(sa.sa_name, ReadModuleInfo.mi_idname);

	if ((devno = strmod_add(devno, &ifnetinfo, &sa)) == NODEV)
		return ENODEV;

        if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
		bzero(outdata, outdatalen);
                outdata->sc_version = OSF_STREAMS_CONFIG_10;
                outdata->sc_devnum = NODEV;
                outdata->sc_sa_flags = sa.sa_flags;
                strcpy(outdata->sc_sa_name, sa.sa_name);
		bcopy(indata,outdata,min(indatalen,outdatalen));
        }

	simple_lock_init(&list_lock);
	list_head.next = list_head.prev = (contextPtr)&list_head;

	return 0;
}



/*
 *        i f n e t _ i o c t l
 *
 * Routine to handle BSD style socket ioctl's
 *
 * Inputs:
 *        ifp        = pointer to ifnet structure
 *        cmd        = ioctl command
 *        data       = data associated with command
 *
 * Outputs:
 *
 *        data
 *
 * Returns:
 *       
 *        0, if success, ERRNO error otherwise.
 */
static int
ifnet_ioctl(ifp, cmd, data)
	struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
        struct ifaddr *ifa = (struct ifaddr *)data;
	int x, error = EINVAL;
	contextPtr context;

    	IFNET_TRACE(IFNET_IOCTL, 
		printf(" ifnet_ioctl(ifp = 0x%x, cmd = 0x%x, data = 0x%x )\n",
			ifp, cmd, data););


	/*
	 * set ref_cnt on list of context structures -so close doesn't 
	 * remove the stream from under us.
	 */
	LOCK_LINKED_LIST();

	/*
	 * find a stream for sending requests to the driver.
	 */
	for (context = list_head.next;
	     context != (contextPtr)&list_head;
	     context = context->next)
	{
	       if ((context->unit == ifp->if_unit) &&
		   (context->state == IFNET_BOUND))
	       {
		     /*
		      * we found a good Stream context structure -
		      * now reference the structure.
		      */
		     GRAB_CONTEXT_STRUCTURE(context);
		     break;
	       }
	}

	UNLOCK_LINKED_LIST();

        if (context == (contextPtr)&list_head)
        {
	      /*
	       * no stream was open to the driver
	       */
	      return(EIO);
	}

	switch( cmd ) {

#ifdef  SIOCENABLBACK
	case SIOCENABLBACK:
	    IFNET_TRACE(IFNET_IOCTL,
			 printf("  ifnet_ioctl() - SIOCENABLBACK (0x%x)\n", 
				cmd););
	    break;

	case SIOCDISABLBACK:
	    IFNET_TRACE(IFNET_IOCTL,
			 printf("  ifnet_ioctl() - SIOCDISBLBACK (0x%x)\n",
				cmd););
	    break;
#endif /* SIOCENABLBACK */

#ifdef SIOCRPHYSADDR
	case SIOCRPHYSADDR:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCRPHYSADDR (0x%x)\n",
				cmd););
	    error = dlpi_get_phys_addr_req(DL_CURR_PHYS_ADDR, context,
					 ((struct ifdevea *)data)->current_pa);
	    if (error)
	        break;
	    error = dlpi_get_phys_addr_req(DL_FACT_PHYS_ADDR, context,
					 ((struct ifdevea *)data)->default_pa);
	    break;
#endif /* SIOCRPHYSADDR */

#ifdef SIOCSPHYSADDR
	case SIOCSPHYSADDR:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCSPHYSADDR (0x%x)\n",
				cmd););
	    error = dlpi_set_phys_addr_req(context,
			(u_char *)((struct ifreq *)data)->ifr_addr.sa_data,
			ifp->if_addrlen);
	    break;
#endif /* SIOCSPHYSADDR */

#ifdef SIOCADDMULTI
	case SIOCDELMULTI:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCDELMULTI (0x%x)\n",
				cmd););
	    /*
	     * XXX could add support for DL_DISABLMULTI_REQ
	     */
	    break;

	case SIOCADDMULTI:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCADDMULTI (0x%x)\n",
				cmd););
	    /*
	     * XXX could add support for DL_ENABLMULTI_REQ
	     */
	    break;
#endif /* SIOCADDMULTI */

#ifdef SIOCRDCTRS
	case SIOCRDCTRS:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCRDCTRS (0x%x)\n",
				cmd););
	    /*
	     * XXX could add support for DL_GET_STATISTICS_REQ
	     * note: would have to know format of data coming back from
	     * driver in order to put the data into the SIOCRDCTRS format.
	     */
	    break;
	    
	case SIOCRDZCTRS:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCRDZCTRS (0x%x)\n",
				cmd););
	    break;
#endif /* SIOCRDCTRS */

	case SIOCSIFADDR:
	    x = splimp();
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCSIFADDR (0x%x)\n",
				cmd););
	    
	    ifp->if_flags |= (IFF_UP | IFF_RUNNING);
	    ifp->if_flags &= ~IFF_OACTIVE;

	    switch(ifa->ifa_addr->sa_family) {
	      
	    case AF_INET:
	        context->ucb->is_ipaddr = IA_SIN(ifa)->sin_addr;
	        arpwhohas(&context->ucb->is_ac, &IA_SIN(ifa)->sin_addr);
		break;

	    default:
		IFNET_TRACE(IFNET_IOCTL, 
	        printf("  ifnet_ioctl()-Unknown Protocol SIOCSIFADDR (0x%x)\n",
		       ifa->ifa_addr->sa_family););
		break;
	    }

	    error = 0;
	    splx(x);
	    break;
	
	case SIOCSIFFLAGS:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - SIOCSIFFLAGS (0x%x)\n",
				cmd););
	    /*
	     * XXX could add code to turn on and off promisc mode.
	     */
	    error = 0;
	    break;

	default:
	    IFNET_TRACE(IFNET_IOCTL, 
			 printf("  ifnet_ioctl() - ??? (0x%x)\n", cmd););
	    
	    break;
	};

	UNGRAB_CONTEXT_STRUCTURE(context);
	return(error);
}



/*
 *        i f n e t _ o u t p u t
 *
 * output routine for "ifnet" layer.
 * This routine takes packets from the "ifnet" layer and writes them
 * to the driver.
 *
 */
static int
ifnet_output(ifp)
	struct ifnet *ifp;
{
	register struct mbuf *m;
	register struct ether_header *eh;
	mblk_t *mp;
	int x;
	contextPtr context;

    	IFNET_TRACE(IFNET_OUTPUT, 
           	strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		       "  ifnet_output( ifp = 0x%x )\n", ifp););

	/*
	 * . DQ xmit mbuf packets off of ifq
	 * . repackage into mblk format
	 * . pass onto to our write queue
	 */
	for (;;) {
		x = splimp();
		IF_DEQUEUE(&ifp->if_snd, m);
		splx(x);
		if (m == NULL)
			break;
	        /*
		 *  extract ethernet header from mbuf and find the Stream
		 *  corresponding to this packet type and unit number
		 */
	        eh = mtod(m, struct ether_header *);	
	        LOCK_LINKED_LIST();
	        for (context = list_head.next;
		     context != (contextPtr)&list_head;
		     context = context->next)
	        {
	            if ((context->unit == ifp->if_unit) && 
			(context->state == IFNET_BOUND) &&
#ifndef	IFNET_SQT
	                (context->protocol == eh->ether_type)
#else
			(context->protocol <= eh->ether_type &&
			 context->uprotocol >= eh->ether_type)
#endif
	   	    ) { 
		   	GRAB_CONTEXT_STRUCTURE(context);
	   	        break;
	            }
	        }
	        UNLOCK_LINKED_LIST();

		if (context == (contextPtr)&list_head) {
			ifp->if_oerrors++;
			m_freem(m);
			continue;
		}
		/*
		 * If driver flow controlled, put packet back on
		 * _sockets_ queue and retry in service.
		 */
		if (!canput(context->ifnet_write_q->q_next)) {
			/* <- timing hole here */
			x = splimp();
			IF_PREPEND(&ifp->if_snd, m);
			splx(x);
			/* cover timing hole */
			if (canput(context->ifnet_write_q->q_next)) {
				UNGRAB_CONTEXT_STRUCTURE(context);
				continue;
			}
			UNGRAB_CONTEXT_STRUCTURE(context);
			break;
		}
#ifndef	IFNET_SQT
		if ((mp = allocb(DL_UNITDATA_REQ_SIZE +
				sizeof (struct ether_header), BPRI_HI)) == NULL)
#else
		if ((mp = allocb(ETH_UNITDATA_REQ_SIZE, BPRI_HI)) == NULL)
#endif
			goto fail;

		/*
		 * . chain data mblk(s) to header mbuf
		 * . place ethernet header into mblk in a UNITDATA_REQ
		 */
		m->m_data += sizeof( struct ether_header );
		m->m_len  -= sizeof( struct ether_header );
		mp->b_cont = (struct msgb *)mbuf_to_mblk(m, BPRI_MED);
		if (mp->b_cont)
		{
#ifndef	IFNET_SQT
		    dl_unitdata_req_t *urp;
		    urp = (dl_unitdata_req_t *)mp->b_rptr;	
		    urp->dl_primitive = DL_UNITDATA_REQ;
		    urp->dl_dest_addr_length = 6;
		    urp->dl_dest_addr_offset = sizeof(dl_unitdata_req_t);
		    urp->dl_priority.dl_min = urp->dl_priority.dl_max = 0L;
		    bcopy(eh, 
			  mp->b_rptr + sizeof(dl_unitdata_req_t),
			  sizeof(struct ether_header));
		    mp->b_wptr = mp->b_rptr + DL_UNITDATA_REQ_SIZE +
						sizeof (struct ether_header);
#else
		    eth_unitdata_req_t *urp;
		    urp = (eth_unitdata_req_t *)mp->b_rptr;
		    urp->PRIM_type = ETH_UNITDATA_REQ;
		    urp->PKT_type = ntohs(eh->ether_type);
		    bcopy(eh->ether_dhost, &urp->DST_addr, 6);
		    mp->b_wptr = mp->b_rptr + ETH_UNITDATA_REQ_SIZE;
#endif
		    mp->b_datap->db_type = M_PROTO;
		    IFNET_TRACE((IFNET_OUTPUT), 
		    {
		      strlog(IFNET_ID, 0, 9, 
			     SL_TRACE|SL_ERROR, 
			"ifnet_output: mp=%x, context=%x, writeq=%x\n",
			     mp, context, context->ifnet_write_q);
		    });
		    putnext(context->ifnet_write_q, mp);
		    UNGRAB_CONTEXT_STRUCTURE(context);
		    ifp->if_opackets++;
		}
		else
		{
	fail:
		    UNGRAB_CONTEXT_STRUCTURE(context);
		    /*
		     * mbuf to mblk buffer alloc problem -
		     * just drop packet for now...
		     */
		    /*ifp->if_oerrors++;*/
		    ifp->if_collisions++;
		    m_freem(m);
		    if (mp) freemsg(mp);
		}
	}
}



/*
 *         i f n e t _ o p e n
 * 
 * This routine is called when the module is pushed onto the stream 
 * This routine will attach the ifnet structure into the ifnet chain. 
 * 
 * Inputs
 *    SVR4 Streams open signature
 *
 * Outputs
 *    none
 * 
 * Returns
 *    error on failure
 */
static int
ifnet_open(q, devp, flag, sflag, credp)
	queue_t *q;
	dev_t * devp;
	int   flag, sflag;
	cred_t *credp;
{
	contextPtr context;
	int x, error;

	IFNET_TRACE(IFNET_OPEN, 
        {
	     strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
	    " ifnet_open(q = 0x%x, flag = 0x%x, sflag = 0x%x)",
			    q, flag, sflag);
	});
    
#if SEC_BASE
        if (error = !privileged(SEC_SYSATTR, 0)) 
        {
            return(OPENFAIL);
        }
#else
        if (error = suser(u.u_cred, &u.u_acflag))
        {
            u.u_error = u.u_error;
            return(OPENFAIL);
        }
#endif

	error = streams_open_comm(sizeof(context_t), q, devp, flag, sflag, credp);
	if (error)
		return error;

	/*
	 * Initialize context on first open.
	 */
	if ((context = (contextPtr)q->q_ptr)->next == NULL) {
		simple_lock_init(&context->lock);

		/*
		 * Context structure also points to the write queue.
		 */
		context->ifnet_write_q = WR(q);

		/*
		 * Hook the context structure into the linked list 
		 */
		LOCK_LINKED_LIST();
		insque(context, &list_head);
		UNLOCK_LINKED_LIST();

		/* 
		 * Ask the driver what the hardware address is
		 */
		error = dlpi_get_phys_addr_req(DL_CURR_PHYS_ADDR,
						context, context->hwaddr);
		if (error)
			(void) ifnet_close(q, flag, credp);
	}

	IFNET_TRACE(IFNET_OPEN,
		     strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
			    " ifnet_open() - return %d", error););

	return error;
}



/*
 *         i f n e t _ c l o s e
 *
 * This routine is called when a stream is closed down. Reset queue
 * pointers to indicate that this resource may be used by another
 * client.
 *
 * Inputs
 *     SVR4 Streams close signature
 *
 * Outputs
 *     none
 *
 */
static int
ifnet_close(q, flag, credp)
	register queue_t *q;
	int flag;
	cred_t * credp;
{
	contextPtr context = (contextPtr)q->q_ptr;
        int x;

	IFNET_TRACE(IFNET_CLOSE,
	{
	     strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		    " ifnet_close(q = 0x%x)", q);
        });

	LOCK_LINKED_LIST();
	remque(context);
	UNLOCK_LINKED_LIST();
	x = splimp();
	simple_lock(&context->lock);
	while (context->ref_cnt) {
		assert_wait((int)&ifnet_close, FALSE);
		simple_unlock(&context->lock);
		splx(x);
		IFNET_TRACE(IFNET_CLOSE,
		{
		     strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
			    " ifnet_close waiting, ref_cnt %d", context->ref_cnt);
		});
		(void)tsleep((caddr_t)0, PZERO, (const char *)0, hz);
		x = splimp();
		simple_lock(&context->lock);
	}
	splx(x);

	return streams_close_comm(q, flag, credp);
}



/*
 *        i f n e t _ w p u t
 *
 * The write put routine is called when a message is traveling downstream
 * This routine processes messages received on the downstream
 * (write) queue. The message will be passed downstream to the driver
 *
 * Inputs
 *    q     - write queue
 *    mp    - message pointer
 * 
 * Outputs
 *    none 
 *
 */

static int
ifnet_wput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	contextPtr context = (contextPtr)q->q_ptr;
	union DL_primitives *request = (union DL_primitives *)mp->b_rptr;
	struct iocblk *iocp;
	unsigned int unit;

	IFNET_TRACE(IFNET_WRITE, 
	{
	     strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		    "  ifnet_wput(q = 0x%x, mp = 0x%x)", q, mp);
	});


	switch (mp->b_datap->db_type)
	{
	      case M_IOCTL:
	              iocp = (struct iocblk *)mp->b_rptr;
		      if (iocp->ioc_cmd == IFNET_IOCTL_UNIT &&
			  mp->b_cont && iocp->ioc_count >= sizeof(short) &&
			  (unit = *(short *)mp->b_cont->b_rptr) < MAX_UNITS)
		      {
			    attach_ifnet(unit, context);
			    freemsg(mp->b_cont); 
			    mp->b_cont = 0;
			    mp->b_datap->db_type = M_IOCACK;
			    qreply(q, mp);
		      } else
		            putnext(q, mp);
		      break;
	      case M_PROTO:
	      case M_PCPROTO:
		      IFNET_TRACE(IFNET_WRITE,
				   strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
				 " ifnet_wput() - request->prim_type = %d. ", 
					  request->dl_primitive););
#ifndef	IFNET_SQT
		      switch (request->dl_primitive)
		      {
			   case DL_BIND_REQ:
			        if (context->state == IFNET_ATTACHED)
				{
				        context->protocol = context->uprotocol =
					  htons(request->bind_req.dl_sap);
				        context->state = IFNET_BOUND;
				}
				break;
			   case DL_UNBIND_REQ:
				context->state = IFNET_ATTACHED;
				break;
		      }
#else
		      switch (request->type)
		      {
			   case ETH_BIND_REQ:
			        if (context->state == IFNET_ATTACHED)
				{
				        context->protocol =
					  htons(request->ETH_bind_req.eth_lwb);
				        context->uprotocol =
					  htons(request->ETH_bind_req.eth_upb);
				        context->state = IFNET_BOUND;
				}
				break;
			   case ETH_UNBIND_REQ:
				context->state = IFNET_ATTACHED;
				break;
		      }
#endif
		      /* Fall through */
	       default:
		      if ((mp->b_datap->db_type >= QPCTL) ||
			  (!q->q_first && canput(q->q_next)))
			   putnext(q, mp);
		      else
			   putq(q, mp);
		      break;
	}
	return 0;
}



/*
 *         i f n e t _ w s r v
 *
 * Write service procedure
 *
 */
static int
ifnet_wsrv(q)
	register queue_t *q;
{
	mblk_t *mp;
    
	IFNET_TRACE(IFNET_WRITE, 
        {
	     strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		    "  ifnet_wsrv(q = 0x%x)", q);
	});

	while ((mp = getq(q)) != NULL)
	{
	     if (!canput(q->q_next)) {
	         putbq(q, mp);
		 return 0;
	     }
	     putnext(q, mp);
	}
	(void) ifnet_output(&((contextPtr)q->q_ptr)->ucb->is_if);
	return 0;
} 



/*
 *         i f n e t _ r p u t 
 * 
 * This read put routine will be called when a message is traveling
 * upstream from the driver.  Depending on the message, the routine
 * will either: 1) pass the message upstream
 *              2) pass the message to ifnet layer (ether_input)
 *              3) put the message in resp.mp and awaken someone
 *		4) discard the message.
 */
static int
ifnet_rput(q, mp)
	register queue_t *q;
	mblk_t *mp;
{
	contextPtr context = (contextPtr)q->q_ptr;

	IFNET_TRACE(IFNET_READ, 
	{
	     strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		    " ifnet_rput: ifnet_rput(q = 0x%x)", q);
	});

	/*
	 * Pass the message upstream
	 */
	switch (find_rep_type(mp))
	{
	       case (IFNET_SOCKETS):
	       {
		      register struct ifnet *ifp = &context->ucb->is_if;
		      register struct mbuf *m;
		      /*	
		       * This packet for Q dedicated to 'ifnet'
		       * 
		       * Mblk consists of an M_PROTO message of the 
		       * type UNITDATA_IND message with ether header 
		       * info included, followed by one or more 
		       * M_DATA blocks.
		       */
		      m = (struct mbuf *)mblk_to_mbuf(mp->b_cont, M_DONTWAIT);
		      if (m)
		      {
			      /*
			       * Dispatch packet to 'ifnet' handler
			       */
			      struct ether_header eh;
			      struct ether_header *ether_hdr = &eh;
#ifndef	IFNET_SQT
			      dl_unitdata_ind_t *uip;
			      uip = (dl_unitdata_ind_t *)mp->b_rptr;	
			      bcopy(mp->b_rptr + uip->dl_dest_addr_offset,
				    ether_hdr, uip->dl_dest_addr_length);
			      bcopy(mp->b_rptr + uip->dl_src_addr_offset,
				    (caddr_t)ether_hdr+uip->dl_dest_addr_length,
				    uip->dl_src_addr_length);
			      ether_hdr->ether_type = ntohs(context->protocol);
#else
			      eth_unitdata_ind_t *uip;
			      uip = (eth_unitdata_ind_t *)mp->b_rptr;
			      /* The message is inside out... sigh */
			      bcopy(&uip->DST_addr, ether_hdr->ether_dhost, 6);
			      bcopy(&uip->SRC_addr, ether_hdr->ether_shost, 6);
			      ether_hdr->ether_type = uip->PKT_type;
#endif
 			      freeb(mp);
			      IFNET_TRACE((IFNET_READ | IFNET_DUMP_DATA),
			      {
				      char buf[100];
				      char s[32];
				      char d[32];
				      sprintf(buf, "%04x %s->%s",
					(int)ether_hdr->ether_type,
					arp_sprintf(s, ether_hdr->ether_shost, 6),
					arp_sprintf(d, ether_hdr->ether_dhost, 6));
				      strlog(IFNET_ID, 0, 9, 
					     SL_TRACE|SL_ERROR,
					     " ifnet: rput() - calling ether_input() with data size = %d, header %s",
					     m->m_pkthdr.len, buf);
			      });
			      m->m_pkthdr.rcvif = ifp;
			      ifp->if_ipackets++;
			      ether_input(ifp, ether_hdr, m);
		      }
		      else
		      {
			      ifp->if_ierrors++;
			      freemsg(mp);
		      }
		      break;
	    }
	    case (IFNET_ERROR):
	    {
		      context->ucb->is_if.if_ierrors++;
		      freemsg(mp);
		      break;
	    }
	    case (IFNET_UPSTREAM):
	    {
		    /*
		     * reply - pass it up STREAMS stack
		     */	
		    IFNET_TRACE(IFNET_READ, 
		    {
			 strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		         " ifnet: rput() passing primitive upstream");
		    });
		    putnext(q, mp);
		    break;
	    }
	    case (IFNET_WAKEUP):
	    {
		    int x = splimp();
		    simple_lock(&context->lock);
		    if (context->resp.mp || context->resp.state == 0)
			freemsg(mp);
		    else {
			context->resp.mp = mp;
			wakeup((caddr_t)&context->resp.mp);
		    }
		    simple_unlock(&context->lock);
		    splx(x);
		    break;
	    }
	    case (IFNET_DISCARD):
	    {
	            freemsg(mp);
		    break;
	    }
#if MACH_ASSERT
	    default:
	            panic("ifnet_rput: unexpected rep_type");
#endif
	} /* switch (rep_type) */
	return 0;
}

static int
find_rep_type(mp)
	mblk_t *mp;
{
	union DL_primitives *request = (union DL_primitives *)mp->b_rptr;
	int rep_type = IFNET_DISCARD;

	switch (mp->b_datap->db_type)
	{
	case M_DATA:
		/* Discard */
		break;
	case M_PROTO:
	case M_PCPROTO:
#ifndef	IFNET_SQT
		switch (request->dl_primitive)
		{
		      case DL_ERROR_ACK:
		      case DL_OK_ACK:
		          switch (((dl_error_ack_t *)request)->dl_error_primitive )
			  {
			      case (DL_BIND_REQ):
			      case (DL_UNBIND_REQ):
			      case (DL_ATTACH_REQ):
			      case (DL_DETACH_REQ):
				  rep_type = IFNET_UPSTREAM;
				  break;
			      default:
				  rep_type = IFNET_WAKEUP;
				  break;
			  }
			  break;
		      case DL_BIND_ACK:
  			  rep_type = IFNET_UPSTREAM;
			  break;
		      case DL_PHYS_ADDR_ACK:
			  rep_type = IFNET_WAKEUP;
			  break;
		      case DL_UNITDATA_IND:
			  rep_type = IFNET_SOCKETS;
			  break;
		      case DL_UDERROR_IND:
			  rep_type = IFNET_ERROR;
			  break;
		}
#else
		switch (request->type)
		{
		      case ETH_ERROR_ACK:
		          switch (((eth_error_ack_t *)request)->ERROR_prim )
			  {
			      case (ETH_BIND_REQ):
			      case (ETH_UNBIND_REQ):
				  rep_type = IFNET_UPSTREAM;
				  break;
			      default:
				  rep_type = IFNET_WAKEUP;
				  break;
			  }
			  break;
		      case ETH_BIND_ACK:
		      case ETH_UNBIND_ACK:
  			  rep_type = IFNET_UPSTREAM;
			  break;
		      case ETH_INFO_ACK:
			  rep_type = IFNET_WAKEUP;
			  break;
		      case ETH_UNITDATA_IND:
			  rep_type = IFNET_SOCKETS;
			  break;
		      case ETH_UDERROR_IND:
			  rep_type = IFNET_ERROR;
			  break;
		}
#endif
		IFNET_TRACE(IFNET_READ, 
		    strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		    "ifnet: find_rep_type() primitive = %x, rep_type = %d", 
				    request->dl_primitive, rep_type);
		);
		break;
	default:
		rep_type = IFNET_UPSTREAM;
		break;
	}
	return rep_type;
}

/*
 * Send request to driver and wait for reply.
 * Sleeps, interruptibly.
 */
static int
do_driver_request(
	contextPtr context,
	mblk_t **mpp)
{
	mblk_t *mp = *mpp;
	int x, error = 0;

	x = splimp();
	simple_lock(&context->lock);
	/*
	 * Claim ownership of "resp".
	 */
	while (context->resp.state) {
		/* Sleep until other owner lets go - &state */
		assert_wait((int)&context->resp.state, TRUE);
		simple_unlock(&context->lock);
		splx(x);
		error = tsleep((caddr_t)0, PZERO|PCATCH, (const char *)0, 0);
		if (error)
			return error;
		x = splimp();
		simple_lock(&context->lock);
	}
	context->resp.state++;
	if (context->resp.mp) {
		freemsg(context->resp.mp);
		context->resp.mp = NULL;
	}
	simple_unlock(&context->lock);
	splx(x);
	/*
	 * Send request.
	 */
	putnext(context->ifnet_write_q, mp);
	/*
	 * Wait for reply.
	 */
	x = splimp();
	simple_lock(&context->lock);
	while ((mp = context->resp.mp) == NULL) {
		/* Sleep until timeout or message received - &mp */
		assert_wait((int)&context->resp.mp, TRUE);
		simple_unlock(&context->lock);
		splx(x);
		error = tsleep((caddr_t)0, PZERO|PCATCH, (const char *)0, hz*5);
		x = splimp();
		simple_lock(&context->lock);
		if (error)
			break;
	}
	context->resp.state = 0;
	context->resp.mp = NULL;
	simple_unlock(&context->lock);
	splx(x);
	wakeup((caddr_t)&context->resp.state);
	*mpp = mp;
	return error;
}


/*
 *        d l p i _ g e t _ p h y s _ a d d r _ r e q
 *
 * This routine creates and send a DL_PHYS_ADDR_REQ packet downstream
 * to the driver.  It then sleeps until a reply is received
 *
 * input
 *    addr_type - defines which address to request
 *    context   - pointer to data associated with Stream
 *    
 * output
 *    phys_addr - physical address returned from driver
 * 
 */
static int
dlpi_get_phys_addr_req(
	int addr_type,
	contextPtr context,
	u_char *phys_addr)
{
	mblk_t *mp;
	union DL_primitives *req;
	int x, error;

	/* 
	 * create request to get physical address
	 */
#ifndef	IFNET_SQT
	mp = allocb(DL_PHYS_ADDR_REQ_SIZE, BPRI_HI );
#else
	mp = allocb(ETH_INFO_REQ_SIZE, BPRI_HI );
#endif
	if (mp == NULL)
		return ENOSR;
	mp->b_datap->db_type = M_PROTO;
	req = (union DL_primitives *)mp->b_rptr;	
#ifndef	IFNET_SQT
	req->dl_primitive = DL_PHYS_ADDR_REQ;
	req->physaddr_req.dl_addr_type = addr_type;
	mp->b_wptr = mp->b_rptr + DL_PHYS_ADDR_REQ_SIZE;
#else
	req->type = ETH_INFO_REQ;
	mp->b_wptr = mp->b_rptr + ETH_INFO_REQ_SIZE;
#endif

	/*
	 * send request and wait for a reply
	 */
	IFNET_TRACE(IFNET_IOCTL, strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
		    "ifnet: about to send DL_PHYS_ADDR_REQ");
	);
	if (error = do_driver_request(context, &mp))
		return error;

	/*
	 * process reply
	 */
	req = (union DL_primitives *)(mp->b_rptr);
#ifndef	IFNET_SQT
	if (req->dl_primitive == DL_PHYS_ADDR_ACK)
	{
	    bcopy(mp->b_rptr + req->physaddr_ack.dl_addr_offset,
		  phys_addr,
		  req->physaddr_ack.dl_addr_length);
	}
#else
	if (req->type == ETH_INFO_ACK)
	{
	    if (addr_type == DL_CURR_PHYS_ADDR)
		    bcopy(&req->ETH_info_ack.PHYS_addr, phys_addr, 6);
	    else
		    bcopy(&req->ETH_info_ack.factory_PHYS_addr, phys_addr, 6);
	}
#endif
	else
	    error = EIO;

	IFNET_TRACE(IFNET_IOCTL,
	{ char s[32]; strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
	       "ifnet: reply-prim = %d, addr = %s", req->dl_primitive,
		arp_sprintf(s, phys_addr, 6));
	});
	freemsg(mp);
	return error;
}


#ifndef	IFNET_SQT
#ifdef SIOCSPHYSADDR
/*
 *        d l p i _ s e t _ p h y s _ a d d r _ r e q
 *
 * This routine creates and send a DL_SET_PHYS_ADDR_REQ packet downstream
 * to the driver.  It then sleeps until a reply is received
 *
 * input
 *    context   - pointer to data associated with stream
 *    addr      - address to set
 *    len       - len of address
 *    
 * output
 * 
 */
static int
dlpi_set_phys_addr_req(
	contextPtr context,
	u_char *addr,
	int len)
{
	mblk_t *mp;
	union DL_primitives *req;
	int x, error;

	/* 
	 * create request to set physical address
	 */
	mp = allocb(DL_SET_PHYS_ADDR_REQ_SIZE + len, BPRI_HI );
	if (mp == NULL)
		return ENOSR;
	mp->b_datap->db_type = M_PROTO;
	req = (union DL_primitives *)mp->b_rptr;	
	req->dl_primitive = DL_SET_PHYS_ADDR_REQ;
	req->set_physaddr_req.dl_addr_length = len;
	req->set_physaddr_req.dl_addr_offset = DL_SET_PHYS_ADDR_REQ_SIZE;
	mp->b_wptr = mp->b_rptr + DL_PHYS_ADDR_REQ_SIZE;
	bcopy(addr, mp->b_wptr, len);
	mp->b_wptr += len;

	/*
	 * send request and wait for a reply
	 */
	IFNET_TRACE(IFNET_IOCTL, strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
			    "ifnet: about to send DL_SET_PHYS_ADDR_REQ");
	);
	if (error = do_driver_request(context, &mp))
		return error;

	/*
	 * process reply
	 */
	req = (union DL_primitives *)(mp->b_rptr);
	if (req->dl_primitive == DL_OK_ACK)
	{
		IFNET_TRACE(IFNET_IOCTL,
			strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
			"ifnet: dlpi_set_phys_addr: success\n"););
		bcopy(addr, context->ucb->is_addr, len);
	        if (context->ucb->is_ipaddr.s_addr)
			arpwhohas(&context->ucb->is_ac, &context->ucb->is_ipaddr);
	}
	else
	{
		error = EIO;
		if (req->dl_primitive == DL_ERROR_ACK)
		{
		    IFNET_TRACE(IFNET_IOCTL,
			  strlog(IFNET_ID, 0, 9, SL_TRACE|SL_ERROR,
			  "ifnet: dlpi_set_phys_addr: error %d/%d\n",
			  req->error_ack.dl_errno, req->error_ack.dl_unix_errno););
		}
	}
	freemsg(mp);
	return error;
}
#endif /* SIOCSPHYSADDR */
#endif /* !IFNET_SQT */


/*
 *        a t t a c h _ i f n e t
 *
 * This routine fills in the ifnet structure and attaches it to the ifnet
 * linked list.
 *
 * input
 *    unit      - unit number - need to put this into the ifnet structure
 *    context   - context structure associated with this stream.
 */
static void
attach_ifnet(
	unsigned int unit,
	contextPtr context)
{
	register ucbPtr ucb;
	register struct ifnet *ifp;

	IFNET_TRACE(IFNET_IOCTL,
	   printf(" ifnet: attach_ifnet() %s%d 0x%x\n", devName, unit, context);
	);

	/*
	 * if this unit has not yet done a if_attach - do it now
	 */
	ucb = &ifnet_ucb[unit];
	if (ucb->is_if.if_name == NULL) 
	{
		ifp = &ucb->is_if;
		ifp->if_name = devName;
		ifp->if_unit = unit;
		ifp->if_mtu = ETHERMTU - 8;
		ifp->if_type = IFT_ETHER;
		ifp->if_addrlen = 6;
		ifp->if_hdrlen = sizeof(struct ether_header) + 8;
		ifp->if_flags |= (IFF_BROADCAST | IFF_NOTRAILERS);
		ifp->if_output = ether_output;
		ifp->if_start = ifnet_output;
		ifp->if_ioctl = ifnet_ioctl;
		/*
		 * normal devices get this filled in from ifinit()
		 */
		ifp->if_snd.ifq_maxlen = IFQ_MAXLEN;

		ucb->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
		ucb->is_ac.ac_arphrd = ARPHRD_ETHER;
		bcopy(context->hwaddr, ucb->is_ac.ac_enaddr, 6);
		IFNET_TRACE(IFNET_IOCTL,
		     printf(" ifnet: attach_ifnet(), if_attach(0x%x)\n", ifp);
		);
		if_attach(ifp);
	}
	context->ucb = &ifnet_ucb[unit];
	context->state = IFNET_ATTACHED; 
}
