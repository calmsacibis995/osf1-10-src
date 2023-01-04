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
static char *rcsid = "@(#)$RCSfile: kdebug_main.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/27 14:18:51 $";
#endif

#include <sys/types.h>
#include <sys/kdebug.h>
#include <kern/thread.h>
#include <sys/proc.h>
#include <machine/cpu.h>

unsigned long rbuf[MAXPACKET];
static char last_req;
static unsigned long *in, *out;
static unsigned long nwords;
static long nbytes;
extern KdebugInfo kdebug_info;
extern struct processor_set default_pset;
extern struct kdebug_cpusw *kdebug_cpup;
extern struct kdebug_cpusw *kdebug_cpuswitch_entry();
static kdebug_reply(char);
unsigned long kdebug_strtoul(char *);
kdebug_bcopy_fault(u_char *src, u_char *dst, long nbytes);
kdebug_bzero(u_char *addr, long nbytes);

#define INBUF { in = rbuf; out = 0; }
#define OUTBUF { out = rbuf; in = 0; }

#define brk_tid ((struct thread *) active_threads[cpu_number()])

void
kdebug_entry(type, argc, argv)
    long type;
    int argc;
    char *argv[];
{
    long i;

    kprintf(DBG_GENERAL, "kdebug_entry: type %x\n", type);

    install_mm_handler();

    switch (type) {
    case KDEBUG_REQ_INIT:
	/*
	 * set the debug level
	 */
	for (i = 0; i < argc; i++) {
	    if (!kdebug_strncmp("kdebug_dbg", argv[i], 10)) {
		if (argv[i][10] == '=')
		    kdebug_info.debug = kdebug_strtoul(&argv[i][11]);
		else
		    kdebug_info.debug = (unsigned long) -1;
	    }
	}

	break;

    case KDEBUG_REQ_BRKPT:
        if (kdebug_info.state == KDEBUG_DISABLED) {
            /*
             * this is our first breakpoint, so do the necessary
             * initialization
	     */
	    if (!(kdebug_cpup = kdebug_cpuswitch_entry(getcputype()))) {
		/*
		 * no support for this machine type
		 */
		break;
	    }
	    init_saio();
	    kdebug_info.state = KDEBUG_ENABLED;
            kprintf(DBG_BRKPT, "processing first breakpoint\n");
	    kdebug_reply(P_INIT);
	    kdebug_listen();
            install_brkpts();
	} else {
            kprintf(DBG_BRKPT, "processing breakpoint\n");
	    remove_brkpts();
	    kdebug_reply(last_req);
	    kdebug_listen();
            install_brkpts();
	}
	break;
    }

    restore_mm_handler();

    kprintf(DBG_GENERAL, "kdebug_entry: returning\n");
}

/*
 * kdebug_reply -- reply to a dbx request that resulted in the execution
 * of kernel code
 */
static
kdebug_reply(
    char req)
{
    unsigned long maxtids;
    struct thread *th;
    u_long val;

    OUTBUF;

    switch (req) {
    case P_CONT:
	if (brk_tid)
	    out[0] = (ulong) brk_tid->u_address.utask->uu_procp->p_pid;
	else
	    out[0] = 0;

	out[1] = (ulong) brk_tid;
	reg_from_exc(R_EPC, &val);
	val -= 4L;
	reg_to_exc(R_EPC, val);
	out[2] = val;
	reg_from_exc(R_R30, &val); /* sp */
	out[3] = val;
	reg_from_exc(R_R26, &val); /* ra */
	out[4] = val;

	if (brk_tid) {
	    out[5] = 0;
	    maxtids = default_pset.thread_count;
	    th = (struct thread *) default_pset.threads.next;
	    while (maxtids--) {
		if (out[0] == th->u_address.utask->uu_procp->p_pid) {
		    out[out[5]++ + 6] = (unsigned long) th;
		}
		th = (struct thread *) th->pset_threads.next;
	    }
	} else {
	    /*
	     * fake that there is at least one thread
	     */
	    out[5] = 1;
	    out[6] = 0;
	}

	putpkt(req, rbuf, out[5] + 6);
	break;

    case P_STEP:
	if (brk_tid)
	    out[0] = (ulong) brk_tid->u_address.utask->uu_procp->p_pid;
	else
	    out[0] = 0;

	out[1] = (ulong) brk_tid;
	reg_from_exc(R_EPC, &val);
	val -= 4L;
	reg_to_exc(R_EPC, val);
	out[2] = val;
	reg_from_exc(R_R30, &val); /* sp */
	out[3] = val;
	reg_from_exc(R_R26, &val); /* ra */
	out[4] = val;
	putpkt(req, rbuf, 5);
	break;

    case P_INIT:
        putpkt(P_INIT, rbuf, 0);
	break;

    case P_DATA_READ:
        kdebug_bzero((u_char *) out, nbytes);
        putpkt(P_DATA_READ, rbuf, nwords);
	break;

    case P_DATA_WRITE:
        out[0] = EINVAL;
        putpkt(P_ERROR, rbuf, 1);
	break;

    case P_REG_READ:
	out[0] = EINVAL;
	putpkt(P_ERROR, rbuf, 1);
	break;

    case P_REG_WRITE:
	out[0] = EINVAL;
	putpkt(P_ERROR, rbuf, 1);
	break;

    default:
	out[0] = EFAULT;
	putpkt(P_ERROR, rbuf, 1);
	break;
    }
}

/*
 * kdebug_listen -- handle dbx requests
 */
static
kdebug_listen()
{
    unsigned long addr;
    unsigned long src;
    unsigned long reg;
    unsigned long val;
    unsigned long tid;
    unsigned long pid;
    unsigned long count;
    unsigned long maxtids;
    unsigned long done = 0;
    struct thread *th;

    while (!done) {

	last_req = getpkt(rbuf, MAXPACKET);

	switch (last_req) {
	case P_DATA_READ:
	    INBUF;
	    addr = in[0];
	    nbytes = in[1];
	    OUTBUF;

	    if (nbytes > (MAXPACKET * sizeof(long))) {
		kprintf(DBG_WARNING, "data read size too large ");
		kprintf(DBG_WARNING, "(0x%x) > max (0x%x)\n", nbytes,
			MAXPACKET * sizeof(long));
		nbytes = MAXPACKET * sizeof(long);
	    }

	    /*
	     * do some fixups to deal with unaligned accesses and
	     * size values that aren't block of longs
	     */
            if ((addr & 0x7) || (nbytes % sizeof(long))) {
                out[0] = out[nbytes/sizeof(long)] = 0L;
                out = (unsigned long *) ((unsigned long) out + (addr & 0x7));
                nwords = nbytes/sizeof(long);
                nwords = (((unsigned long)out + nbytes) - (unsigned long)rbuf) /
                    sizeof(long);
                if (((unsigned long)out + nbytes) & 0x7)
                    nwords++;
            } else {
                nwords = nbytes / sizeof(long);
            }

	    /*
	     * kdebug_bcopy_fault will return non-zero on fault
	     */
            if (kdebug_bcopy_fault((u_char *) addr, (u_char *) out, nbytes)) {
		kprintf("P_DATA_READ: memory fault\n");
		kdebug_bzero((u_char *) out, nbytes);
	    }

            putpkt(P_DATA_READ, rbuf, nwords);
            break;

        case P_DATA_WRITE:
            INBUF;
            addr = in[0];
            nbytes = in[1];
            /*
             * do some fixups to deal with unaligned accesses
             */
            src = (unsigned long)&in[2] + (addr & 0x7);
            OUTBUF;

	    /*
	     * kdebug_bcopy_fault will return non-zero on fault
	     */
            if (kdebug_bcopy_fault((u_char *) src, (u_char *) addr, nbytes)) {
		kprintf("P_DATA_WRITE: memory fault\n");
	    }

            putpkt(P_DATA_WRITE, rbuf, 0);
            break;

        case P_REG_READ:
            INBUF;
	    tid = in[0];
	    reg = in[1];
	    OUTBUF;

    	    if ((ulong) brk_tid == tid) {
		reg_from_exc(reg, &val);
		out[0] = val;
	    } else {
		if (reg_from_pcb(((struct thread *)tid)->pcb, reg, &val))
		    out[0] = -1;
		else
		    out[0] = val;
	    }

	    putpkt(P_REG_READ, rbuf, 1);
	    break;

	case P_REG_WRITE:
	    INBUF;
	    tid = in[0];
	    reg = in[1];
	    val = in[2];
	    OUTBUF;

    	    if ((ulong) brk_tid == tid) {
		reg_to_exc(reg, val);
	    } else {
		reg_to_pcb(((struct thread *)tid)->pcb, reg, val);
	    }

	    putpkt(P_REG_WRITE, rbuf, 0);
	    break;

	case P_THREAD_CNT:
	    INBUF;
	    pid = in[0];
	    OUTBUF;

	    count = 0;

	    if (brk_tid) {
		maxtids = default_pset.thread_count;
	        th = (struct thread *) default_pset.threads.next;
		while (maxtids--) {
		    if (pid == th->u_address.utask->uu_procp->p_pid) {
			count++;
		    }
		    th = (struct thread *) th->pset_threads.next;
		}
	    } else {
	        /*
	         * fake that there is at least one thread
	         */
	        count = 1;
	    }

	    out[0] = count;
	    kprintf(DBG_RDEBUG, "0x%x threads\n", count);
	    putpkt(P_THREAD_CNT, rbuf, 1);
	    break;

	case P_THREAD_LIST:
	    INBUF;
	    pid = in[0];
	    OUTBUF;

	    count = 0;

	    if (brk_tid) {
		maxtids = default_pset.thread_count;
	        th = (struct thread *) default_pset.threads.next;
		while (maxtids--) {
		    if (pid == th->u_address.utask->uu_procp->p_pid) {
		        out[count++] = (unsigned long) th;
		    }
		    th = (struct thread *) th->pset_threads.next;
		}
	    } else {
		/*
	     	 * fake that there is at least one thread (with id 0)
		 */
		count = 1;
		out[0] = 0;
	    }

	    kprintf(DBG_RDEBUG, "0x%x threads\n", count);
	    putpkt(P_THREAD_LIST, rbuf, count);
	    break;

	case P_SET_PID:
	    INBUF;
	    pid = in[0];
	    OUTBUF;

	    out[0] = -1;

	    if (brk_tid) {
		if (pid == brk_tid->u_address.utask->uu_procp->p_pid) {
		    /*
		     * this is the pid we broke on, so give them back the
		     * thread that hit the breakpoint
		     */
		    out[0] = (unsigned long) brk_tid;
		} else {
		    /*
		     * return a thread for this pid
		     */
		    maxtids = default_pset.thread_count;
	            th = (struct thread *) default_pset.threads.next;
		    while (maxtids--) {
		        if (pid == th->u_address.utask->uu_procp->p_pid) {
		            out[0] = (unsigned long) th;
			    break;
		        }
		        th = (struct thread *) th->pset_threads.next;
		    }
		}
	    } else
	    {
		/*
	     	 * only pid 0 is valid this early (return faked tid 0)
		 */
		if (pid == 0)
		    out[0] = 0;
	    }

	    putpkt(P_SET_PID, rbuf, 1);
	    break;
		
	case P_CONT:
	    done = 1;
	    break;

	case P_STEP:
	    step();
	    done = 1;
	    break;

	case P_EXIT:
	    kprintf(DBG_GENERAL, "Exiting debug monitor\n");
    	    quit();
	    break;

	default:
	    kprintf(DBG_WARNING, "unknown request\n");
	    OUTBUF;
	    out[0] = EINVAL;
	    putpkt(P_ERROR, rbuf, 1);
	    break;
	}
    }
}

quit()
{
    kdebug_halt();
}

/*
 * kdebug_strtoul: converts a hex string to an integer
 */

unsigned long
kdebug_strtoul(char *s)
{
    unsigned long val;
    unsigned long i;
    int xx;

    val = 0L;

    for (i = 0; s[i] != '\0'; i++) {
	if (s[i] >= 'a' && s[i] <= 'f')
	    xx = (s[i] - 'a' + 10);
	else
	    if (s[i] >= 'A' && s[i] <= 'F')
	        xx = (s[i] - 'A' + 10);
	    else
	        if (s[i] >= 'A' && s[i] <= 'F')
	            xx = (s[i] - '0');
		else
		    return 0;
	val = 16 * val + xx;
    }

    return val;
}

/*
 * kdebug_strncmp: compares two strings up to the specified number of
 * characters.
 */

kdebug_strncmp(
    char *st1,
    char *st2,
    long n)
{
    while (--n >= 0 && *st1 == *st2++)
	if (*st1++ == '\0')
	    return(0);
    return(n < 0 ? 0 : *st1 - *--st2);
}

/*
 * kdebug_bcopy_fault adds fault handling to the low-level kdebug_bcopy routine.
 * A non-zero return indicates a fault was taken.
 */

kdebug_bcopy_fault(
    u_char *src,
    u_char *dst, 
    long nbytes)
{
    u_int nofault_save;
    u_long ret;
    
    nofault_save = kdebug_info.nofault;
    kdebug_info.nofault = 1;
    ret = kdebug_bcopy(src, dst, nbytes);
    kdebug_info.nofault = nofault_save;
    return ret;
}

/*
 * kdebug_bzero clears out the desired number of bytes at the specified
 * address.
 */

kdebug_bzero(
    u_char *addr,
    long nbytes)
{
    long i;

    for (i = 0; i < nbytes; i++)
	addr[i] = (char) 0;

    return 0;
}
