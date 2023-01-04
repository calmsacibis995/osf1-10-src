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
static char	*sccsid = "@(#)$RCSfile: machdep.c,v $ $Revision: 1.2.3.8 $ (DEC) $Date: 1992/10/13 15:36:35 $ ";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from machdep.c	2.1	(ULTRIX/OSF)	12/3/90";
 */
/* from static char *sccsid = "@(#)machdep.c	4.12      (ULTRIX)  11/14/90"; */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/************************************************************************
 *
 *	Modification History: machdep.c
 *
 * 08-Oct-91	Robert R. Mc Guirk
 *  Replaced an assignment to u.u_signal[SIGILL] in sendsig() with an 
 *  assignment to signal_disposition(SIGILL). 
 *
 * 30-May-91 mbs
 *      Added the atomic_op system call code.
 *
 * 05-Feb-90 -- dws
 *	Fixed handling of SIGCONT in sigreturn() for POSIX mode.
 *
 * 26-Oct-89 -- afd
 *	for network boot, pick up network device unit number if its not 0.
 *
 * 03-Oct-89 -- Joe Szczypek
 *	Changed set_lock, clear_lock, set_bit_atomic, and clear_bit_atomic
 *      to use interlocked operations if system is MP.
 *
 * 18-July-89	kong
 *	Rewrote the routine useracc.  The original routine was
 *	located in usercopy.s and checks user access by reading
 *	and writing the page.  If the cache is incoherent at the
 *	time, or if DMA is going on, the contents of the buffer
 *	being probed gets corrupted.  The original routine also
 *	has the side-effect of causing paging in of pages.
 *	The new routine simply checks the protection field of the
 *	PTE to determine if access is allowed.  This is similar
 *	to the implementation on a VAX which uses the PROBEr/w instruction
 *	to check access.  
 *
 *************************************************************************/

#include <machine/reg.h>
#include <mach/machine/thread_status.h>

#include <sys/proc.h>
#include <sys/buf.h>

#include <kern/xpr.h>

#include <machine/cpu.h>
#include <machine/fpu.h>	/* need for FPCSR_EXCEPTIONS */
#include <machine/vmparam.h>
#include <dec/sas/mop.h>

#include <sys/mbuf.h>

#include <sys/fs_types.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>

#include <sys/lock.h>

/*
 * NOTE: The following definitions must match the setjmp values,
 *	 which must match the struct sigcontext offsets in order
 *	 for it all to work.
 *	 So, for the sake of reducing errors, the following
 *	 values will be based from the setjmp.h values.
 *	 JB_REGS = offset (in integers) to sc_regs array.
 *
 *	 Wouldn't it be nice if genassym did it all based
 *	 on struct sigcontext!
 */

/*
 * Must keep these old values for now, due to
 * cfe warnings on redefinition conflicts between
 * JB_ definitions in pcb.h, and JB_ definitions in setjmp.h.
 * pcb.h included by user.h.
 *
 * #include <machine/setjmp.h>	
 */
#define	R_ZERO	0 
#define	R_AT	1
#define	R_V0	2
#define	R_A0	4
#define	R_A1	5
#define	R_A2	6
#define	R_A3	7
#define	R_SP	29

/* desired definitions
#define	R_ZERO	JB_ZERO-JB_REGS
#define	R_AT	JB_AT-JB_REGS
#define	R_V0	JB_V0-JB_REGS
#define	R_A0	JB_A0-JB_REGS
#define	R_A1	JB_A1-JB_REGS
#define	R_A2	JB_A2-JB_REGS
#define	R_A3	JB_A3-JB_REGS
#define	R_SP	JB_SP-JB_REGS
 */

#ifdef PGINPROF
/* NOT ported to MIPS yet! */
/*
 * Return the difference (in microseconds)
 * between the  current time and a previous
 * time as represented  by the arguments.
 * If there is a pending clock interrupt
 * which has not been serviced due to high
 * ipl, return error code.
 */
vmtime(otime, olbolt, oicr)
	register int otime, olbolt, oicr;
{

	if (mfpr(ICCS)&ICCS_INT)
		return(-1);
	else
		return(((time.tv_sec-otime)*60 + lbolt-olbolt)*16667 + mfpr(ICR)-oicr);
}
#endif

/*
 * Clear registers on exec
 */
setregs(entry)
	long entry;
{
	/* don't bother cleaning registers or pcb */

	u.u_ar0[EF_EPC] = (int)entry;
	u.u_ar0[EF_FP] = 0;
	u.u_ar0[EF_RA] = 0;
	current_thread()->pcb->pcb_ownedfp = 0; /* speedup forking */
}


/*
 * Send an interrupt to process.
 *
 * Stack is set up to allow sigcode in libc
 * to call user signal handling routine, followed by syscall
 * to sigreturn routine below.  After sigreturn
 * restores critical registers, resets the signal mask and the
 * stack, it returns to user mode.
 */

#define	STACK_ALIGN(x)	((unsigned)(x) &~ ((4*sizeof(int))-1))

sendsig(p, sig, signalmask)
int (*p)(), sig, signalmask;
{
	register struct sigcontext *scp;	/* ptr to user space context */
	struct sigcontext	scontext;	/* signal context */
	register int *srp;
	register int *frp;
	int oonstack;
	register struct pcb *pcb = current_thread()->pcb;

#define	mask(s)	(1<<((s)-1))

	XPR(XPR_SIGNAL, ("enter sendsig %d, 0x%x",sig,signalmask,0,0,0));
	oonstack = u.u_onstack;

	/*
	 * decide which stack signal is to be taken on, and make
	 * sure its aligned and accessable.
	 */
	if (!u.u_onstack && (u.u_sigonstack & mask(sig))) {
		scp = (struct sigcontext *)STACK_ALIGN(u.u_sigsp) - 1;
		u.u_onstack = 1;
	} else
		scp = (struct sigcontext *)STACK_ALIGN(USER_REG(EF_SP)) - 1;

	/*
	 * Save into the sigcontext the signal state and that portion
	 * of process state that we trash in order to transfer control
	 * to the signal trampoline code.
	 * The remainder of the process state is preserved by
	 * signal trampoline code that runs in user mode.
	 */
	scontext.sc_onstack = oonstack;
	scontext.sc_mask = signalmask;
	scontext.sc_pc = USER_REG(EF_EPC);
	scontext.sc_regs[R_V0] = USER_REG(EF_V0);
	scontext.sc_regs[R_A0] = USER_REG(EF_A0);
	scontext.sc_regs[R_A1] = USER_REG(EF_A1);
	scontext.sc_regs[R_A2] = USER_REG(EF_A2);
	scontext.sc_regs[R_A3] = USER_REG(EF_A3);
	scontext.sc_regs[R_SP] = USER_REG(EF_SP);

	/*
	 * if this process has ever used the fp coprocessor, save
	 * its state into the sigcontext
	 */
	scontext.sc_ownedfp = pcb->pcb_ownedfp;
	if (pcb->pcb_ownedfp) {
		checkfp(current_thread(), 0);	/* dump fp to pcb */
		for (srp = &scontext.sc_fpregs[0], frp = pcb->pcb_fpregs;
		    frp < &pcb->pcb_fpregs[32]; srp++, frp++)
			*srp = *frp;
		scontext.sc_fpc_csr = pcb->pcb_fpc_csr;
		scontext.sc_fpc_eir = pcb->pcb_fpc_eir;
		pcb->pcb_fpc_csr &= ~FPCSR_EXCEPTIONS;
	}
	scontext.sc_cause = USER_REG(EF_CAUSE);
	scontext.sc_badvaddr = USER_REG(EF_BADVADDR);

	if (copyout(&scontext, scp, sizeof (struct sigcontext)))
		goto bad;

	/*
	 * setup registers to enter signal handler
	 * when resuming user process
	 */
	USER_REG(EF_A0) = sig;
	if (sig == SIGFPE || sig == SIGSEGV || sig == SIGILL ||
	    sig == SIGBUS || sig == SIGTRAP || sig == SIGSYS) {
		USER_REG(EF_A1) = u.u_code;
		u.u_code = 0;
	} else
		USER_REG(EF_A1) = 0;
	USER_REG(EF_A2) = (unsigned)scp;
	USER_REG(EF_A3) = (unsigned)p;
	USER_REG(EF_SP) = STACK_ALIGN(scp);
	USER_REG(EF_EPC) = (unsigned)u.u_sigtramp;
	XPR(XPR_SIGNAL,("exit sendsig %d, 0x%x",sig,signalmask,0,0,0));
	return;

bad:
	uprintf("sendsig: can't grow stack, pid %d, proc %s\n", 
		u.u_procp->p_pid, u.u_comm);
	/*
	 * Process has trashed its stack; give it an illegal
	 * instruction to halt it in its tracks.
	 */
	signal_disposition(SIGILL) = SIG_DFL;
	sig = mask(SIGILL);
	u.u_procp->p_sigignore &= ~sig;
	u.u_procp->p_sigcatch &= ~sig;
	u.u_procp->p_sigmask &= ~sig;
	psignal(u.u_procp, SIGILL);
}

/*
 * Routine to cleanup state after a signal
 * has been taken.  Reset signal mask and
 * stack state from context left by sendsig (above).
 */
sigreturn(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	struct args {
		struct sigcontext *sigcntxtp;
	} *uap = (struct args *) args;
	struct sigcontext scontext;
	register int *srp;
	register u_int *urp;
	register int *frp;
	register struct sigcontext *scp;
	struct pcb *pcb;
	int	error;

	scp = uap->sigcntxtp;
	if (!useracc((caddr_t)scp, sizeof (*scp), B_READ))
		return (EINVAL);
	if (error = copyin(scp, &scontext, sizeof (struct sigcontext)))
		return(error);

	u.u_onstack = scontext.sc_onstack & 01;
	p->p_sigmask = scontext.sc_mask & ~sigcantmask;
	/*
	 * copy entire user process state from sigcontext into
	 * exception frame, a special exit from syscall insures
	 * that the entire exception frame gets restored
	 */
	USER_REG(EF_EPC) = scontext.sc_pc;
	USER_REG(EF_MDLO) = scontext.sc_mdlo;
	USER_REG(EF_MDHI) = scontext.sc_mdhi;
	for (urp = &USER_REG(EF_AT), srp = &scontext.sc_regs[R_AT];
	    urp <= &USER_REG(EF_RA); urp++, srp++)
			*urp = *srp;
	pcb = current_thread()->pcb;
	if (pcb->pcb_ownedfp) {
		checkfp(current_thread(), 1);	/* toss current fp contents */
		for (frp = pcb->pcb_fpregs, srp = &scontext.sc_fpregs[0];
		    frp < &pcb->pcb_fpregs[32]; frp++, srp++)
			*frp = *srp;
		pcb->pcb_fpc_csr = scontext.sc_fpc_csr & ~FPCSR_EXCEPTIONS;
	}
	return (EJUSTRETURN);
}
#undef mask

/* No consumers, so lets comment it out for now... */
#ifdef notdef
physstrat(bp, strat, prio)
	struct buf *bp;
	int (*strat)(), prio;
{
	int s;
	int count = bp->b_bcount;
	caddr_t addr = bp->b_un.b_addr;

	(*strat)(bp);
	/* TODO: pageout stuff commented out....
	/* pageout daemon doesn't wait for pushed pages
	if (bp->b_flags & (B_DIRTY|B_RAWASYNC))
		return;
	    ********/

	LASSERT(BUF_LOCK_HOLDER(bp));
	(void) event_wait(&bp->b_iocomplete, FALSE, 0);

#ifdef oldmips
	if (bp->b_flags & B_READ)
		bufflush(bp);	/* do this if you do dma....rr */
#endif /* oldmips */
#ifdef	MSERIES
	/*
	 * If the machine has devices capable of DMA we got a problem here.
	 * Since the cache is non-snooping, DMA can generate memory
	 * inconsistencies.  We must therefore flush the cache here.
	 * For normal buffer cache operations buffers are made non-cachable.
	 */
	if (bp->b_flags & B_READ)
		bufflush(bp);
#endif	/* MSERIES */
}
#endif	/* notdef */




gets(cp)
	char *cp;
{
	register char *lp;
	register c;

	lp = cp;
	for (;;) {
		c = getchar() & 0177;
		if (c == -1)		/* lk201 hickup */
			continue;
		switch (c) {
		case '\n':
		case '\r':
			*lp++ = '\0';
			return;
		case '\b':
		case '#':
		case '\177':
			lp--;
			if (lp < cp)
				lp = cp;
			continue;
		case '@':
		case 'u'&037:
			lp = cp;
			cnputc('\n');
			continue;
		default:
			*lp++ = c;
		}
	}
}

getchar()
{
	register c;

	c = cngetc();
	if (c == '\r')
		c = '\n';
	if (c != -1)		/* lk201 hickup */
		cnputc(c);
	return (c);
}


char boottype[10];
struct netblk netblk;
static int forcebootp = 0; /* ==1 to force a network boot */
char netdevice[80];
static int bootp_errno;

/*
 * called from init_main to see if a network boot has occurred.  If so,
 * available information is read into a local copy of the netblk
 * structure.  As per original design the changing of 'roottype' is what
 * triggers init_main to assume a diskless network environment.
 */
netbootchk()
{

	struct netblk *netblk_ptr;
	extern int roottype;
	extern int swaptype;
	extern int dumptype;
	extern int swapsize;
	char *bootdev;
	char *bootstring;
	char *boottypep;
	int i;

	/*
	 * Note: network boot can be for diskless or Standalone 
	 * ULTRIX.  The caller (init_main.c) checks roottype to
	 * sense the difference.
	 */
	
	boottype[0] = '\0';
	boottypep = &boottype[0];

	bootdev = (char *)getbootdev();
	i=0;
	while(bootdev[i] != NULL) {      /* Save for network crashing */
	        netdevice[i] = bootdev[i];
		i++;
	}
	netdevice[i] = bootdev[i];

	if (forcebootp > 0 || (!strncmp(bootdev, "mop", 3)) ||
	    (!strncmp(bootdev, "tftp", 4)) ||
	    (!strncmp(&bootdev[2], "mop", 3)) ||
	    (!strncmp(&bootdev[2], "tftp", 4))) { 
		int i; char *j;

		/*
		 * Take a picture of the netblk structure in case
		 * it gets tromped on by someone other than the
		 * kernel.
		 */
		bcopy((char *)NETBLK_LDADDR, (char *)&netblk,
			sizeof (struct netblk));
		/*
		 * Clear out that which was just copied so it isn't
		 * hanging around for a subsequent boot.  This will
		 * guarantee that NETBLK_LDADDR points to real data
		 * pertaining to this boot.
		 */
		j = (char *)NETBLK_LDADDR;
		for (i = 0; i < sizeof (struct netblk); i++)
			j[i] = 0;
		netblk_ptr = &netblk;
		getboottype(boottypep, bootdev);

		/* for tftp boots, send a BOOTP request packet */
		if (forcebootp > 0 || (!strncmp(bootdev, "tftp", 4)) ||
		  (!strncmp(&bootdev[2], "tftp", 4))) {
			if (bootp_info(boottype) < 0)
				printf("bootp: request failed, errno=%d\n",
					bootp_errno);
		}
		/* TODO: fill in netblk_ptr with rootfs */

		/*** no diskless support yet
		if (netblk_ptr->rootfs == GT_NFS) {
			roottype= (int) netblk_ptr->rootfs;
			swaptype= (int) netblk_ptr->swapfs;
			swapsize= ((int) netblk_ptr->swapsz) * 1024;
			if (netblk_ptr->dmpflg != -1)
			     dumptype= ((int) netblk_ptr->dmpflg) * 1024;
		}
		***/
	}

}

/*
 * UDP port numbers for BOOTP server and client.
 */
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68

#define BOOTREQUEST     1
#define BOOTREPLY       2

/*
 * Bootstrap Protocol (BOOTP).  RFC 951.
 */
struct bootp {
        unsigned char   bp_op;          /* packet opcode type */
        unsigned char   bp_htype;       /* hardware addr type */
        unsigned char   bp_hlen;        /* hardware addr length */
        unsigned char   bp_hops;        /* gateway hops */
        unsigned long   bp_xid;         /* transaction ID */
        unsigned short  bp_secs;        /* seconds since boot began */
        unsigned short  bp_unused;
        struct in_addr  bp_ciaddr;      /* client IP address */
        struct in_addr  bp_yiaddr;      /* 'your' IP address */
        struct in_addr  bp_siaddr;      /* server IP address */
        struct in_addr  bp_giaddr;      /* gateway IP address */
        unsigned char   bp_chaddr[16];  /* client hardware address */
        unsigned char   bp_sname[64];   /* server host name */
        unsigned char   bp_file[128];   /* boot file name */
	unsigned char   bp_vend[64];    /* vendor-specific area */
};

/* globals */
#define Printf printf
int bootpq_maxlen = IFQ_MAXLEN;
int bootp_active=0;     /* for ether_read(), ==1 we are currently using bootp */
int bootpdebug=0;       /* debug flag */
struct ifqueue bootpq;  /* bootp input packet queue; see ether_read(); */

/* locals */
static int dst_port_addr, my_port_addr;
static struct bootp sendbuf, recvbuf;

/*
 * Broadcast a bootp packet to find out our IP address and
 * the server IP address.  Return with the netblk filled in,
 * or -1 on error.
 */
static 
bootp_info(bp)
	char *bp;	/* string name of interface to use (ie. "ln0") */
{
	struct netblk *netblk_ptr;		/* ptr to netblk */
	register struct in_ifaddr *ia;		/* for ifp search */
	struct ifnet *ifp;			/* ptr to ifnet */
	char devname[10];			/* device name (only) */
	int devunit;				/* device unit # */
	struct ifdevea ifd;			/* for SIOCRPHYSADDR ioctl */
	struct ifaddr ifa;			/* for SIOCSIFADDR */
	struct sockaddr sa;			/* for SIOCSIFADDR */
	int i, s, found, error, cmd, count;
	struct mbuf *m;
	int sts=0, retrys=5;

	/* initialize bootpq */
	bootpq.ifq_head = bootpq.ifq_tail = (struct mbuf *)0;
	bootpq.ifq_len = 0;
	bootpq.ifq_drops = 0;
	bootpq.ifq_maxlen = bootpq_maxlen;
	bootp_active = 1;	/* this hooks into ether_read() */
	netblk_ptr = &netblk;
	/*
	 * Set our addresses for bootp transaction.
	 */
	dst_port_addr = htons(IPPORT_BOOTPS);
	my_port_addr = htons(IPPORT_BOOTPC);

	bootp_errno = 0;
	i=0;
	while(*bp != NULL) {	/* copy device name, then zap the unit number */
		devname[i++] = *bp++;
	}
	devunit = devname[--i] - '0';	/* XXX assumes single digit unit # */
	devname[i] = NULL;

	if (bootpdebug>0) Printf("bootp: checking ifnet for %s, unit %d\n",
		devname, devunit);
	/* find the ifp of this device so we can call the driver directly */
	found=0;
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if ((strcmp (ifp->if_name, devname))==0) {
			if (ifp->if_unit == devunit) {
				found=1;
				break;
			} else {
				if (bootpdebug>0) Printf("bootp: found name %s but wrong unit (%d)\n",
				ifp->if_name, ifp->if_unit);
			}
		} else {
			if (bootpdebug>0) Printf("bootp: name %s doesn't match\n", ifp->if_name);
		}
	}
	if (found>0) {
		if (bootpdebug>0) Printf("bootp: found ifp OK, %s%d\n", ifp->if_name, ifp->if_unit);
	} else {
		/* hardwire to line 0, unit 0 if search fails */
		ifp = ifnet;
		Printf("bootp: ifnet entry for %s%d not found, using %s%d\n",
			devname, devunit, ifp->if_name, ifp->if_unit);
	}

	/* get hardware address */
	if (ifp == 0 || ifp->if_ioctl == 0) {
		Printf("bootp: no ifp->if_ioctl() for device %s%d\n",
			ifp->if_name, ifp->if_unit);
		bootp_errno = ENXIO;
		return(-1);
	}
	cmd=SIOCRPHYSADDR;
	if (bootpdebug>0) Printf("bootp: calling *ifp->if_ioctl(SIOCRPHYSADDR)\n");
	error = (*ifp->if_ioctl)(ifp, cmd, &ifd);
	if (error != 0) {
		if (bootpdebug>0)
			Printf("bootp: non-zero return from ifp->if_ioctl(SIOCRPHYSADDR)\n");
		/* keep going... */
	}
	if (bootpdebug>0)
		Printf("bootp: hardware addr=%s\n",
			ether_sprintf(ifd.default_pa));

	/*
	 * Set the network interface up and running, using INET
	 * addr 0.0.0.0, so we can send/receive BOOTP packets.
	 */
	bzero((char *) &ifa, sizeof(struct ifaddr));
	bzero((char *) &sa, sizeof(struct sockaddr));
	ifa.ifa_addr = &sa;	/* dummy sockaddr */
	ifa.ifa_addr->sa_family = AF_INET;
	ifp->if_addrlist = &ifa;
	cmd = SIOCSIFADDR;
	if (bootpdebug>0)
		Printf("bootp: calling *ifp->if_ioctl(SIOCSIFADDR)\n");
	error = (*ifp->if_ioctl)(ifp, cmd, &ifa);
	if (error != 0) {
		if (bootpdebug>0)
			Printf("bootp: non-zero return from ifp->if_ioctl(SIOCSIFADDR)\n");
		/* keep going... */
	}

	/*
	 * Generate the BOOTP request message.
	 */
	bzero((char *)&sendbuf, sizeof(sendbuf));
	sendbuf.bp_op = BOOTREQUEST;
	sendbuf.bp_htype = ARPHRD_ETHER;
	sendbuf.bp_hlen = 6;
	/* sendbuf.bp_xid = getrand(1024,0xff00); */
	sendbuf.bp_xid = 19338; /* XXX: not random */

	/* XXX: 6 is hardwired to Ethernet */
	sendbuf.bp_ciaddr.s_addr = 0;
	bcopy((char *)ifd.default_pa, (char *)sendbuf.bp_chaddr, 6);

	/*
	 * Send BOOTP request and get a response.
	 *
	 * Repeat transaction attempt up to 5 times.
	 *   Send the request.
	 *   Read response.
	 *     If error, retry.
	 *     If success, return.
	 *     If no response, repeat until timeout, then retry.
	 */
	while (retrys--) {
		count=0; /* error count */
		sendbuf.bp_xid++;
		if (bootpdebug>0) Printf("bootp: trying %d\n",(retrys+1));
		if ((sts=bootp_write(ifp, &sendbuf, sizeof (sendbuf))) != 0) {
			Printf("bootp: network write failed, errno=%d\n", bootp_errno);
			continue;
		}
		sts=0;
		while (sts == 0) {
			sts = bootp_read(ifp, &recvbuf, sizeof (recvbuf));
			if (sts > 0) {
				netblk_ptr->cliipadr = ntohl(recvbuf.bp_yiaddr.s_addr);
				netblk_ptr->srvipadr = ntohl(recvbuf.bp_siaddr.s_addr);

				/* future: fill in real names here */
				bcopy("SERVER", netblk_ptr->srvname, 6);
				netblk_ptr->srvname[6] = NULL;
				bcopy("CLIENT", netblk_ptr->cliname, 6);
				netblk_ptr->cliname[6] = NULL;
				netblk_ptr->brdcst = (unsigned long)-1; /* all 1's */
				netblk_ptr->netmsk = 0; /* force default */
				bootp_active = 0; /* done */
				ifp->if_addrlist = NULL;
				s = splimp();
				for(;;) {
					/* drain input queue */
					IF_DEQUEUE(&bootpq, m);
					if (m == 0)
						break; /* done */
					m_freem(m);
				}
				splx(s);
				return(0); /* success */
			}
			if (sts == 0) {
				/* returned without reading any input packets.
				 * Go around the loop trying "retry" times.
				 */
				sts++;
			}
			if (sts == -1) {
				/* error, not our port, length error, etc. */
				sts++; /* == 0 */
				if (count++ > 12) /* allow 12 "error" packets */
					sts++; /* == 1, goto retry */
			}
		} /* end while (sts == 0) */
	} /* end while (retrys--) */

	/* fallthrough is error case */
	bootp_active = 0;
	ifp->if_addrlist = NULL; /* cleanup */
	s = splimp();
	for(;;) {
		/* drain input queue */
		IF_DEQUEUE(&bootpq, m);
		if (m == 0)
			break; /* done */
		m_freem(m);
	}
	splx(s);
	bootp_errno=ETIMEDOUT;
	return(-1);
}
/*
 * bootp_write
 *
 * Note that eh is setup to point to an odd halfword.  This then causes the
 * ip header and udp headers to be word aligned.
 * return 0 on success, -1 on error and errno set appropriately.
 */
static int 
bootp_write(ifp, buf, cnt)
	struct ifnet *ifp;
	char *buf;
	int cnt;
{
	struct ether_header *eh;
	struct ip *iph;
	struct udphdr *udph;
	char *data;
	static unsigned short ip_id = 0;
	struct mbuf *m, *mp;
	int s;

	bootp_errno = 0;
	s = splimp();
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m) {
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
			Printf("bootp_write: no mbuf clusters!\n");
			bootp_errno = ENOBUFS;
			goto bad;
		}
		m->m_len = MCLBYTES;
		m->m_pkthdr.len = MCLBYTES;
		m->m_data += 2;  /* word align IP/UDP header */
		m->m_len -= 2;
	} else {
		Printf("bootp_write: no mbufs!\n");
		bootp_errno = ENOBUFS;
		goto bad;
	}
	splx(s);

	eh = (struct ether_header*)(mtod(m, char *));
	iph = (struct ip*)((char*)eh + sizeof(struct ether_header));
	udph = (struct udphdr*)((char*)iph + sizeof(struct ip));
	data = (char*)udph + sizeof(struct udphdr);

	/*
	 * Copy data to buffer.
	 */
	bcopy(buf, data, cnt);
	m->m_len = cnt + sizeof(struct ip)
		+ sizeof(struct udphdr) + sizeof(struct ether_header);
	m->m_pkthdr.len = m->m_len; /* needed? */

	/*
	 * Fill in the Ethernet header.
	 */
	eh->ether_type = htons(ETHERTYPE_IP);
	bcopy((caddr_t)etherbroadcastaddr, (caddr_t)eh->ether_dhost,
		sizeof(eh->ether_dhost));
	bcopy((caddr_t)((struct bootp *)data)->bp_chaddr,
		(caddr_t)eh->ether_shost, sizeof(eh->ether_shost));

	/*
	 * Construct IP header.
	 */
	bzero((char *)iph, sizeof(struct ip));
	iph->ip_vhl = (sizeof(struct ip)>>2 | IPVERSION<<4);
	iph->ip_len = htons(sizeof(struct udphdr) + sizeof(struct ip) + cnt);
	iph->ip_id = htons(ip_id++);
	iph->ip_off = htons(IP_DF);
	iph->ip_ttl = MAXTTL;
	iph->ip_p = IPPROTO_UDP;
	iph->ip_src.s_addr = 0;
	iph->ip_dst.s_addr = INADDR_BROADCAST;
	iph->ip_sum = 0;
	/* point mbuf at the start of IP header before doing checksum */
	m->m_len -= sizeof (struct ether_header);
	m->m_data += sizeof (struct ether_header);
	iph->ip_sum = in_cksum(m, sizeof(struct ip));
	/* put it back again */
	m->m_data -= sizeof (struct ether_header);
	m->m_len += sizeof (struct ether_header);

	/*
	 * Construct UDP header.  (No checksum.)
	 */
	udph->uh_sport = my_port_addr;
	udph->uh_dport = dst_port_addr;
	udph->uh_ulen  = htons(cnt + sizeof(struct udphdr));
	udph->uh_sum = 0;

	/*
	 * Transmit the packet.
	 */
	s = splimp();
	IFQ_LOCK(&ifp->if_snd);
	if (IF_QFULL(&ifp->if_snd)) {
		Printf("bootp_write: IF_QFULL for ifp %s%d\n",
			ifp->if_name, ifp->if_unit);
		IF_DROP(&ifp->if_snd);
		IFQ_UNLOCK(&ifp->if_snd);
		bootp_errno=ENOBUFS;
		goto bad;
	}
	IF_ENQUEUE_NOLOCK(&ifp->if_snd, m);
	IFQ_UNLOCK(&ifp->if_snd);
	if ((ifp->if_flags & IFF_OACTIVE) == 0)
		(*ifp->if_start)(ifp);	/* start is really xxoutput() */
	splx(s);
	return(0);

bad:
	m_freem(m);
	splx(s);
	return(bootp_errno);
}

/*
 * bootp_read
 * Read a BOOTP packet from the network.  Return length of buffer read,
 * or return -1 if error (packet not for us, etc.)
 */
static int 
bootp_read(ifp, buf, cnt)      /* NOTE: ifp not used */
	struct ifnet *ifp;
	char *buf;
	int cnt;
{
	struct ip *iph;
	struct udphdr *udph;
	struct mbuf *m;
	int s;
	int sts = 0;
	int count = 0;


again:
	s = splimp();
	IF_DEQUEUE(&bootpq, m);
	splx(s);
	if (m == 0) {
		if (count++ > 10) {
			/* nothing came in for 5 seconds */
			if (bootpdebug>0)
				Printf("bootp_read: IF_DEQUEUE bootpq is empty\n");
			return(0);
		}
		DELAY(500000);  /* 1/2 sec */
		goto again;
	}
	if (bootpdebug>0) 
		Printf("bootp: got %d bytes at addr %X\n", m->m_len, m->m_data);
	sts = m->m_pkthdr.len;

	if ((m->m_len < sizeof (struct ip) + sizeof(struct udphdr)) &&
		(m = m_pullup(m, sizeof(struct ip) + sizeof(struct udphdr))) == 0) {
		  goto drop; /* packet too short */
	}

	iph = (struct ip*)(mtod(m, char *));
	udph = (struct udphdr*)((char*)iph + sizeof(struct ip));

	/*
	 * Validate IP data...
	 *   Check the header checksum.
	 *   Check that length is le than packet size.
	 *   Check that it is a udp packet.
	 */
	if (in_cksum(m, sizeof(struct ip))) {
		if (bootpdebug>0) Printf("bootp_read: IP in_cksum failed\n");
		goto drop;
	}

	if (ntohs(iph->ip_len) > sts) {
		/*Printf("bootp_read: IP header len > readsize\n");*/
		goto drop;
	}
	if (iph->ip_p != IPPROTO_UDP) {
		/*Printf("bootp_read: IP header proto not UDP/IP\n"); */
		goto drop;
	}

	/*
	 * Validate UDP data...
	 *  Length checks.
	 *  Check that it is to us.
	 *  Record where it came from.
	 */
	sts -= sizeof (struct ip);
	udph->uh_ulen = ntohs(udph->uh_ulen);
	if (udph->uh_ulen > sts) {
		/*Printf("bootp_read: UDP header len > readsize\n"); */
		goto drop;
	}
	sts = udph->uh_ulen - sizeof(struct udphdr) ;

	/* make sure packet is for us */
	if (udph->uh_dport != my_port_addr ) {
		/*Printf("bootp_read: UDP dest port not BOOTPC\n"); */
		goto drop;
	}
	if (bootpdebug>0) Printf("bootp_read: got bootp reply, size=%d\n", sts);

	if (sts > cnt) {
		if (bootpdebug>0)
			Printf("bootp_read: got bootp packet too large for bootp buffer\n");
		goto drop;
	}

	/*
	 * Copy data to caller buffer, skipping the IP/UDP header.
	 */
	m_copydata(m, sizeof(struct ip) + sizeof(struct udphdr), sts, buf);
	m_freem(m);
	return(sts);
drop:
	m_freem(m);
	return(-1);
}


/* 
 * Handle common configure operations and 
 * call the machine dependent configure routine.
 */
configure()
{
	extern int cold;
	extern int lvprobe();

	cold = 1;

	/*
 	 * Initialize PROM environment entries.
	 */
	hwconf_init();

	/*
	 * Initialize interrupt handler kernel framework.
	 * Initialize loadable driver framework.
	 */
	handler_init();
	loadable_driver_init();

	/*
	 * Call machine dependent code to handle machine specific
	 * operations
	 */
	machine_configure();

	/*
	 * Configure logical volume manager(s).
	 */
	lvprobe(0);

	/*
	 * Configure swap area and related system
	 * parameter based on device(s) used.
	 */
	setconf();
	swapconf();

	/*
	 * Safe to take interrupts now.
	 */
	splnone();

	cold = 0;
	return (0);
}



#ifdef notdef
setlock(l) 
struct lock_t *l;
{
        if(!smp)                 /* If 1 CPU, don't need interlocked operations */

	  if (l->l_lock) return(0);

	  else {
	    l->l_lock=1;
	    return(1);
	  }
	
	else return(bbssi(31,l));        /* Set bit 31 with interlocked operation */
}

clearlock(l) 
struct lock_t *l;
{
        if(!smp)                 /* If 1 CPU, don't need interlocked operations */
	  l->l_lock = 0;

	else return(bbcci(31,l));        /* Clear bit 31 with interlocked operation */
	  
}
#else /* notdef */
setlock(arg)
{
}
clearlock(arg)
{
}
#endif /* notdef */



/*
 * Procedure atomic_op()
 *
 * Purpose:
 * 
 * To perform an atomic "interlocked" operation on the location 
 * specified by arg. 
 */

decl_simple_lock_data(,atomic_op_lock)

atomic_op(p, args, retval)
        struct proc *p;
        void *args;
        int *retval;
{
        register struct args {
                int opcode;                /* ATOMIC_SET or ATOMIC_CLEAR */
                int *address;              /* target address of the operation */
        } *uap = (struct args *) args;

	/* Keep old errno codes as much as possible. */
	if (!useracc(uap->address, sizeof(int), B_WRITE))
	        return EACCES;

	if ((int)uap->address % (sizeof (int)))
	        return EINVAL;

	switch(uap->opcode) {
	      case ATOMIC_SET:
			return set_bit_atomic(ATOMIC_LOCKBIT, uap->address);

	      case ATOMIC_CLEAR:
			return clear_bit_atomic(ATOMIC_LOCKBIT, uap->address);

	      default:
			return EINVAL;   /* error if illegal atomic op */
	}
}

set_bit_atomic(bitpos, base)
	unsigned bitpos;
	caddr_t base; 
{
	int error;
	int lock;
	
	simple_lock(&atomic_op_lock);
	if (error = copyin(base, (caddr_t)&lock, sizeof(int)))
		return error;	/* EFAULT */
	
        if (lock & (1<<bitpos)) {
		simple_unlock(&atomic_op_lock);
	        return EBUSY;
	}
        lock |= (1<<bitpos);
	error = copyout((caddr_t)&lock, base, sizeof(int));
	simple_unlock(&atomic_op_lock);
	return error;	/* ESUCCESS or EFAULT */
}

clear_bit_atomic(bitpos, base)
	unsigned bitpos;
	caddr_t base;
{
	int lock;
	int error;

	simple_lock(&atomic_op_lock);
	if (error = copyin(base, (caddr_t)&lock, sizeof(int)))	
		return error;	/* EFAULT */
        lock &= ~(1<<bitpos);
	error = copyout((caddr_t)&lock, base, sizeof(int));
	simple_unlock(&atomic_op_lock);
	return error;	/* ESUCCESS or EFAULT */
}




/*
 * This atoi came with minor mods from libc.
 * It's intended use is for those
 * few cases where the conversion of a string to
 * an integer in the kernel is required like
 * generic boot code. Please do not abuse it.
 */
atoi(s1)
register char *s1;
{
        register int n;
        register int f;

        n = 0;
        f = 0;
        for(;;s1++) {
		switch(*s1) {
                case ' ':
                case '\t':
                        continue;
                case '-':
                        f++;
                case '+':
                        s1++;
                }
                break;
	}
	while(*s1 >= '0' && *s1 <= '9')
                n = n*10 + *s1++ - '0';
        return(f ? -n : n);
}

itoa(number, string)	/* integer to ascii routine */
int number;
char string[];
{
	int a, b, c;
	int i, sign;
	if (( sign = number) < 0)
		number = number * -1;
	i = 0;
	do {
		string[i++] = number % 10 + '0';
	} while ((number /= 10) > 0);
	if (sign < 0)
		string[i++] = '-';
	string[i] = '\0';
	/* flip the string in place */
	for (b = 0, c = strlen(string)-1; b < c; b++, c--) {
		a = string[b];
		string[b] = string[c];
		string[c] = a;
	}
	return( strlen(string) );
}


/*
 * setup a load quad word physical (ldqp) routine to mimic a
 * simular routine in the alpha project. That way it can be called and
 * not be undefined at load time.
 */

unsigned long
ldqp(phys_addr)
unsigned long phys_addr;
{
       return((unsigned long)(PHYS_TO_K1(phys_addr)));
}
