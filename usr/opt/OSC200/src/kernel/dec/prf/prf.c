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

/*
 * prf.c - profiler pseudo driver
 */

#if defined(SVR4_COMPAT)
#	include <dec/svr4_defines.h>
#endif

/* Param.h includes machparam.h, but not psl.h.  Machparam.h uses psl.h but 
 * expects the calling file to include it. 
 */
#include	<machine/psl.h>
#include	"sys/signal.h"
#include	"sys/errno.h"
#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/uio.h"
#include	"sys/user.h"
#include	"sys/buf.h"
#include	"sys/lock.h"
#include	"dec/prf/prof.h"

#define PRFCTR_INDEX(i,j)	(i == 0 ? i : i*(j+1))

lock_data_t	prf_lock;	/* write lock address */

#define PRF_DEBUG 0

/* set/reset in prfwrite */
unsigned int prfstat=0x0;	/* state of profiler */
int prfmax;			/* number of loaded text symbols */
int findx;			/* first symbol/counter index */
int lindx;			/* last symbol/counter index */
int uindx;			/* user counter index */
int prfmaxdata;			/* max amount of data (bytes) of symbols */
int prfmaxctrs;			/* max amount of data (bytes) of counter */
void (*prfintr_func)() = NULL;	/* pointer to prfintr() we when want calls */
void prfintr();

ulong_t (*prfctrp)[], *prfctr;	/* address counters */
ulong_t (*prfsymp)[], *prfsym;	/* text symbols */

/* 
 * the profiling psuedo driver does not have either an open or 
 * close routine defined in cdev as nulldev's.
 */


/* prfread: copy the profiling data to user space. */

prfread(dev, uio, flag)
dev_t dev;
struct uio *uio;
int flag;
{
        char *datap = uio->uio_iov->iov_base;
	int datalen = uio->uio_iov->iov_len;
        int error = 0;
	unsigned  min();


#if PRF_DEBUG
        printf("prfread: dev uio flag (%x %lx %x)\n",dev,uio,flag);
        printf("prfread: datalen (%d)\n",datalen);
#endif 
        lock_write(&prf_lock);
        /*
	   Make sure text addresses are valid and there's
	   enough user space to do the copyouts.
         */
	if (!(prfstat & PRF_VAL) || 
	    (datalen != (prfmaxdata + prfmaxctrs))) {
		lock_write_done(&prf_lock);
		return(ENXIO);
	}
	if (!(error = copyout(prfsym, datap, prfmaxdata)))
	    if (!(error = copyout(prfctr, datap+prfmaxdata, prfmaxctrs)))
        	uio->uio_resid -= datalen;

        lock_write_done(&prf_lock);

#if PRF_DEBUG
	printf("prfread: returns error = %d\n", error);
#endif 

	return(error);
}


/* prfwrite: copy the namelist to the kernel data area */

prfwrite(dev, uio, flag)
dev_t dev;
struct uio *uio;
int flag;
{
	int datalen = uio->uio_iov->iov_len;
	int i;
        int error=0;

#if PRF_DEBUG
        printf("prfwrite: dev uio flag (%x %lx %x)\n",dev,uio,flag);
        printf("prfwrite: datalen (%d)\n",datalen);
#endif 
	prfintr_func = NULL;
        lock_write(&prf_lock);
        /*
	   If prfstat is valid, we are reloading the kernel addresses
	   to be profiled.  So we must free the kernel memory allocated
           during the last load and reset all global variables.
	 */
        if (prfstat & PRF_VAL) {
	        prfstat = PRF_RESET;
		kfree(prfsym, prfmax * NBPW);		/* free text memory */
		kfree(prfctr, (prfmax+1)* NBPW * MAX_CPU);/* free counter memory*/
        } else
		prfreset();		/* reset all global data */

	/* Setup for load of new symbols to be profiled. */
	if (datalen & (NBPW - 1)) {	/* data must be on a word boundry */
        	lock_write_done(&prf_lock);
		return(E2BIG);
        }

        prfmax = (datalen / NBPW); /* bytes / bytes per word = text symbols */
	prfmaxdata = datalen;
	prfmaxctrs = (prfmax+1) * NBPW * MAX_CPU;
        findx = 0;
        lindx = prfmax-1;
	uindx = prfmax;

        /* allocte kernel memory for symbols */
	if ((prfsymp = (ulong_t (*)[])kalloc(prfmaxdata)) == NULL) {
        	lock_write_done(&prf_lock);
        	return(ENOSPC);
	}
        prfsym = (*prfsymp);

#if PRF_DEBUG
        printf("prfwrite: allocated symbol memory!\n");
#endif 
	/*
           Allocate kernel memory for counters.  All kalloc allocated 
	   memory is zero filled.
	 */
	if ((prfctrp = (ulong_t (*)[])kalloc(prfmaxctrs)) == NULL) {
        	lock_write_done(&prf_lock);
		return(ENOSPC);
	}
        prfctr = (*prfctrp);
        
#if PRF_DEBUG
        printf("prfwrite: allocated counters memory!\n");
#endif 

        /* get the namelist data */
	if ((error = copyin(uio->uio_iov->iov_base, prfsym, datalen)) != 0) {
        	lock_write_done(&prf_lock);
		return(error);
	}
        uio->uio_resid -= datalen;

#if PRF_DEBUG
        printf("prfwrite: first last count (%d %d %d)\n",
        	prfsym[findx], prfsym[lindx], prfmax);
#endif 

	for (i = 1; i < prfmax; i++) {
		if (prfsym[i] < prfsym[i-1]) {
			prfreset();
        		lock_write_done(&prf_lock);
			return(EINVAL);
		}
	}

	prfstat = PRF_VAL;
        lock_write_done(&prf_lock);

#if PRF_DEBUG
        printf("prfwrite: ... \n");
#endif 

	return(0);
}


/* prfioctl: process prf commands
             PRF_STATUS - get profiling mode (ON or OFF).
             PRF_MAX    - get total number of text addresses being profiled.
             PRF_MODE   - set kernel profiling mode (ON or OFF).
             PRF_DEBUG  - set kernel debugging (ON or OFF).

   OSF ioctl system call does a copyin of user data.  So at the time the
   driver ioctl is called, data has been copied into kernel space.
*/

prfioctl(dev, cmd, data, flag)
dev_t dev;
register int cmd;
caddr_t data;
int flag;
{
        unsigned int udata;

#if PRF_DEBUG
        printf("prfioctl: dev cmd data flag (%x %x %lx %x)\n",
		dev,cmd,data,flag);
#endif 

	switch(cmd & 0xFF) {
	case PRF_STATUS:
                *(int *)data = prfstat;
#if PRF_DEBUG
                printf("prfioctl: PRF_STATUS: data = %x\n", *(int *)data);
#endif 
		break;
	case PRF_MAX:
                *(int *)data = prfmax;
#if PRF_DEBUG
                printf("prfioctl: PRF_MAX: data = %x\n",*(int *)data);
#endif 
		break;
	case PRF_MODE:
                udata = (unsigned int)(*data);
#if PRF_DEBUG
                printf("prfioctl: PRF_MODE: data = %x\n", udata);
#endif 

		if (prfstat & PRF_VAL) {
			prfstat = PRF_VAL | udata & PRF_ON;
			if (udata & PRF_ON)
				prfintr_func = prfintr;
			break;
		} else
                	/*
		   	  Not allowed to turn prfstat on without valid
		   	  addresses, so return error.
		 	*/
			return(EINVAL);
	case PRF_FREE: /* reset: free up kernel memory */
#if PRF_DEBUG
		printf("prfioctl: PRF_FREE: prfstat = %x\n", prfstat);
#endif
		if (prfstat & PRF_VAL) {
			prfintr_func = NULL;
			prfstat = PRF_RESET;
			/* free text memory */
			kfree(prfsym, prfmax * NBPW);
			/* free counter memory */
			kfree(prfctr, (prfmax+1) * NBPW * MAX_CPU);
		}
		break;
	default:
#if PRF_DEBUG
		printf("prfioctl: invalid argument %x\n", cmd & 0xFF);
#endif
		return(EINVAL);
	}

#if PRF_DEBUG
        printf("prfioctl: ... \n");
#endif 

	return(0);
}

/* 
 * prfintr: increment text address counter array for current pc.
*/

void
prfintr(ps,ppc,cpu_num)
register int ps;
caddr_t	 ppc;
register int cpu_num;
{
	register ulong_t *ip;
	register int hi, lo, index;
	register ulong_t pc;

	if (prfmax < 1) {
	    printf("kernel: prfintr: invalid number of kernel symbols %d\n",
		    prfmax);
	    return;
	}

	if ((cpu_num < 0) || (cpu_num > MAX_CPU - 1)) {
	    printf("kernel: prfintr: cpu number %d out of range\n", cpu_num);
	    return;
	}

	/* set ip to proper cpu counters */
	ip = &(*prfctrp)[PRFCTR_INDEX(cpu_num,prfmax)];

	/* this works in alpha and mips because:
	   (1) pointer and long are both 4 bytes in mips
	   (2) pointer and long are both 8 bytes in alpha
	*/
	pc = (ulong_t)ppc;

	if (USERMODE(ps))
		ip[uindx]++;
	else {
                if (pc <= prfsym[findx])
			ip[findx]++;
		else if (pc >= prfsym[lindx])
			ip[lindx]++;
		else {
			lo = 0;
			hi = prfmax;
                	/*
                 	* simple divide and conquor, incrementing the
                 	* the nearest text address
                	*/
			while ((index = (lo + hi) / 2) != lo) {
				if (pc >= prfsym[index])
					lo = index;
				else
					hi = index;
                	}
			ip[index]++;
		}
	}
}

/* reset all global data values */
prfreset()
{
	prfstat	= PRF_RESET;
	prfmax  = PRF_RESET;
	prfmaxdata = PRF_RESET;
	prfmaxctrs = PRF_RESET;
	findx  = PRF_RESET;
	lindx  = PRF_RESET;
	uindx  = PRF_RESET;
	prfctr = PRF_RESET;
	prfsym = PRF_RESET;
}
