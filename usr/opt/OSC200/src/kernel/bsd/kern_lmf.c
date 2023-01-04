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
static char *rcsid = "@(#)$RCSfile: kern_lmf.c,v $ $Revision: 1.1.12.2 $ (DEC) $Date: 1993/10/06 21:49:58 $";
#endif
#ifndef lint
static char *sccsid = "@(#)kern_lmf.c	5.4      (ULTRIX)  6/19/91";
#endif lint

/*
 *
 *   Modification history:
 *
 * 18-Oct-91    Carlos Christensen
 *      Port of code from ULTRIX to TIN.  
 *      Changes made to this module as follows:
 *      .  Use lock_write instead of smp_lock for locking kernel cache.
 *      .  Initialize lock for kernel cache (lk_lmf).
 *      .  Create and process exit action for activity licenses.
 *      .  Use of microtime function (instead of timepick variable) for time.
 *      .  Use of kalloc (instead of KM_ALLOC) for storage allocation.
 *      .  Insertion code to reject a P_FAMILY license (conditional).
 *      .  Remove code for P_FAMILY licenses (conditional).
 *      .  Remove code for multiple processor systems (conditional).
 *      .  Remove of certain machine types not available (conditional).
 *      .  Review of code for determination of SMM (no changes).
 *      .  Change of handling of errno and returned values (systemmatic).
 *      .  Remove special code to distinguish between 2100 and 3100
 *      Changes made in other kernel modules as follows:
 *      .  Add calls on {get|set}lminfo to kernel/dec/machine/mips/sys_sysinfo.c
 *      .  Call on lmf_init routine from kernnel/bsd/init_main.c
 *      .  Call on exit_actn routine from kernel/bsd/kern_exit.c
 *      .  Initialize u.u_exitp to NULL in kernel/bsd/kern_fork.c
 *      .  Add kernel variables {lmf|sysinfo}_debug to kernel/dec/machine/mips/kopt.c
 *      .  Add kern_lmf to kernel/conf/files
 *      Changes made in kernel/sys files as follows:
 *      .  Modify OSF header file user.h by adding u_exitp field
 *      .  Add new headers from ULTRIX lmf_smm.h, lmfklic.h, lmf.h, exit_actn.h
 *
 * 01-Jul-91    Carlos Christensen
 *      Copied this file from ULTRIX kern_lmf.c (sccs 5.4 dated 6/19/91)
 *
 * 29-May-91	Paul Grist
 *	Added DS_5000_300 support - 3max+/bigmax.
 *
 * 14-May-91	Joe Szczypek
 *	Added MAXine support.  Use new entries in lmf_smm.h.
 *
 * 06-Sep-90	Randall Brown 
 *	Added DS5000_100 Support. 
 *
 *  3-Aug-90	rafiey (Ali Rafieymehr)
 *	Added VAX9000 support.
 *
 * 05-Jun-90
 *      Added Mariah (VAX65xx) support.
 *
 * 20-Mar-90	Lisa Allgood
 *	Fix old SMM values.
 *
 * 20-Dec-89	Giles Atkinson
 *	Improve support for starting CPUs on a multi-processor
 *	and fix some bugs in VAX SMM recognition.
 *	Reflect new meaning of ws_display_type.
 *
 * 23-Oct-89	Giles Atkinson
 *	SMM determination for MIPS processors
 *
 * 21-Sep-89 	jaw
 *	put in locking in lmf_auth.
 *
 * 29-Jun-89	Giles Atkinson
 *	Added P_FAMILY changes and kernel authorization call.
 *
 * 30-May-89	darrell
 *	Added include of ../../machine/common/cpuconf.h -- cpu types
 *	were moved there.
 *
 *  6 Dec 1988		Lisa Allgood and Giles Atkinson
 *	Original version
 *
 */

/* This module is used only when the kernel is built with _LMF_ */

/* Some code in this module that supports P_FAMILY LMF licenses has not
 * been ported from ULTRIX (and therefore this module does not support 
 * P_FAMILY licenses).  The code for P_FAMILY licenses has been left in
 * place, but is conditional under the P_FAMILY flag.  The following 
 * #define sets that flag to 0 and effectively removes the code. -- carlosc
 */
#define P_FAMILY 0

/* SILVER doesn't support any multi-processor systems.  Thus the number of 
 * cpus running is always 1.  The ULTRIX code for multi-processor systems
 * has been left in place, but some of it has not been ported and is
 * conditional under the MULTI_PROCESSOR flag.  The following #define
 * sets that flag to 0 and effectively removes the code. -- carlosc
 */
#define MULTI_PROCESSOR 0

/* Some CPU types are not defined in SILVER.  The ULTRIX code for these
 * cpus has been left in place, but it is conditional under the
 * EXTRA_CPU_TYPE flag.  The following define sets that flag to 0 and
 * effectively removes the code. -- carlosc
 */
#define EXTRA_CPU_TYPE 0

#if P_FAMILY
/* This is the ULTRIX lock for process queues.  It must be replaced
 * by the corresponding SILVER lock when family licenses are implemented.
 * -- carlosc
 */
extern lock_data_t lk_procqs;
#endif /*P_FAMILY*/

/* For production, the constant LMF_DEBUG must be defined here as 0.
 * -- carlosc
 */
#define LMF_DEBUG 0
extern int lmf_debug;

#include <sys/param.h>      /* from OSF, unchanged */
#include <sys/systm.h>      /* from OSF, unchanged */
#include <sys/exit_actn.h>  /* from ULTRIX for LMF */
#include <sys/user.h>       /* from OSF, add u_exitp field */
#include <sys/errno.h>      /* from OSF, unchanged */
#include <sys/lmfklic.h>    /* from ULTRIX for LMF -- includes sys/lmf.h */
#include <sys/time.h>       /* from OSF, unchanged */
#include <sys/proc.h>       /* from OSF, unchanged */
#include <kern/lock.h>      /* from OSF, unchanged */
#include <io/common/devio.h> /* from OSF, unchanged */
#include <sys/security.h>   /* from OSF, unchanged */

/*
 *	Machine independent part of the Kernel part of the Ultrix LMF.
 *      The get/setlminfo functions are ccalled from the get/setinfo 
 *      system calls.
 */

/* Permanent LMF data */

lock_data_t lk_lmf;		        /* Lock for LMF license cache */
int lmf_smm;				/* System marketing model */
klic_t *klic_head = NULL;		/* Head of klic_t chain */
int lic_cpus;				/* Number of CPUs currently allowed */
unsigned short *smm_tablep = NULL;	/* Pointer to table of MP SMMs */

/* External references */

int smp_debug;

/* Functions */

int lmf_init();
int getlminfo();
int setlminfo();
lmf_auth();
lmf_relauth();
static int getauth();
static int relauth();
static klic_t *find_klic();
static struct exit_klic *find_xa();
static struct exit_klic *new_xk();
static int rel_units();
int lmf_exit_actn();
static void test_del();
#if P_FAMILY
static int auth_family();
static void add_family();
static int release_family();
static void unlink_family();
extern void set_lmf_login();
static int clean_family();
static struct proc *proc_rele_hold();
#endif /*P_FAMILY*/

/*
 *      Initialize Lock
 * 
 * This routine is called from init_main to initialize the lock
 * associated with the kernel license cache.
 */

int
lmf_init()
{
    lock_init(&lk_lmf,1);
}

/*
 *	Retrieve LMF kernel information
 */

int
getlminfo(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
    register struct a {
	unsigned long	 op;	  /* careful: top 32 bits may be junk */
	char		*buffer;
	unsigned long	 nbytes;  /* careful: top 32 bits may be junk */
	long		*start;   /* changed *start from 'int' to 'long' */
	char		*arg;     /*     to accommodate 'klic_t *' value */
	unsigned long	 flag;	  /* careful: top 32 bits may be junk */
    } *uap = (struct a *) args;

    register klic_t *kp, *skp;		/* Pointers to unit cache */
    klic_t *context;			/* Copy of start argument */
    klic_t data;			/* For copy of user's buffer */
    unsigned short smmdata[MAX_SMMD];	/* Used by LMF_GETSMM */
    int size = 0;			/* Size and address of data that is */
    caddr_t addr;                       /*   copied into buffer before return */
    int error = ESUCCESS;               /* Assume no error */

#if LMF_DEBUG
    if( lmf_debug )
        printf("getlminfo: *1* -- nbytes=%ld start=0x%lx flag=%ld",
               uap->nbytes,uap->start,uap->flag);
        if( uap->start ) printf( "    *start=0x%lx\n", *(uap->start) );
#endif

    *retval = 1L;			/* Assume success */
		
/*
 * "normal" indentation not followed here to leave a meaningful amount
 * of space per line for the action routines
 */

    switch ((int)(uap->flag)) {         /* Alpha: clear top 32 bits */
    case LMF_GETSMM:			/* System marketing model */
	/* This subfunction returns an array of unsigned shorts.
	 * The first entry is the current SMM.   It is followed by the
	 * maximum number of CPUs supported by LMF on this processor type
	 * and all possible SMMs which this machine might have.
	 * The array is built in the automatic variable smmdata.
	 */
	smmdata[0] = lmf_smm;
	addr = (caddr_t) smmdata;
	if (smm_tablep) {		/* This is a multi-processor */
		size = (*smm_tablep + 2) * sizeof (unsigned short);
		if (size > MAX_SMMD * sizeof (unsigned short))	/* Paranoia */
			size = MAX_SMMD * sizeof (unsigned short);
		bcopy((caddr_t) smm_tablep,
                      (caddr_t) &smmdata[1],
                      size - sizeof (unsigned short));
	} else {			/* Uniprocessor: only one SMM */
		smmdata[1] = 1;
		smmdata[2] = lmf_smm;
		size = 3 * sizeof (unsigned short);
	}
	break;
    case LMF_GETLIC:			/* Read from license unit cache */
	kp = NULL;
	if (uap->start) {		/* Context variable specified */
		if (error = copyin((caddr_t)uap->start, (caddr_t)&context,
				       sizeof context))
			return (error);
		kp = context;
		lock_write(&lk_lmf); 
		if (kp) {		/* Context specified */
			skp = klic_head;  /* Verify context */
			while (skp != kp && skp != NULL)
				skp = skp->kl_next;
			if (skp == NULL) {
				error = ENOENT;
				break;
			}
		} else			/* Context is NULL - first call */
			kp = klic_head;
	} else {
		if (error = copyin(uap->buffer, (caddr_t) &data,
                        	 min(sizeof data, (int)(uap->nbytes))))
                                         /* Alpha: clear top 32 bits ^ */
			return (error);
		lock_write(&lk_lmf); 
#if LMF_DEBUG
                if( lmf_debug )
                        printf("getlminfo: product_name=%s  producer=%s\n",
                               data.kl_product_name, data.kl_producer);
#endif
		kp = find_klic(data.kl_product_name, data.kl_producer);
#if LMF_DEBUG
                if( lmf_debug )
                        printf("getlminfo: find_klic returns kp=0x%x\n",kp);
#endif
	}
	if (kp == NULL) {
	        *retval = 0L;	/* No data to return */
		break;
	}

	if (uap->arg) {			/* Return this process' usage */
		register struct exit_klic *xp;
		int usage;

		if (kp->kl_act_charge) { /* Activity license */
			xp = find_xa(kp);
			usage = xp ? (xp->xk_indirect ? LMF_FAMILY
				      : xp->xk_uses) : 0;
		} else
			usage = LMF_ACTIVITY;
		if (error = copyout((caddr_t) &usage, (caddr_t) uap->arg,
					sizeof usage))
			break;
	}

	if (uap->start) {
		if (error = copyout((caddr_t) &(kp->kl_next),
					(caddr_t)uap->start, sizeof (klic_t *)))
			break;
	}
	size = sizeof (klic_t);
	addr = (caddr_t) kp;
	break;
    default:
	return(EINVAL);
    }

    /* Copy the data back to the caller */

    if (size) {
	if ((int)(uap->nbytes) < size)   /* Alpha: clear top 32 bits */
	    	error = EINVAL;
	else {
#if LMF_DEBUG
                if( lmf_debug ) {
                    klic_t *klic_temp = (klic_t *)addr;
                    printf("getlminfo: product_name=%s\n",klic_temp->kl_product_name);
                    printf("getlminfo: producer=%s\n",klic_temp->kl_producer);
	        }
#endif
		error = copyout(addr, uap->buffer, size);
	}
    }

    /* Release the LMF lock if we have it */

    if ((int)(uap->flag) == LMF_GETLIC) {  /* Alpha: clear top 32 bits */
	lock_done(&lk_lmf);
    }

#if LMF_DEBUG
    if( lmf_debug )
        printf("getlminfo: Exit error=%d  *retval=%ld\n",
               error, *retval);
#endif

    return (error);
}


/*
 *	Set/Store LMF kernel information
 */

int
setlminfo(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
    register struct a {
	unsigned long	op;	    /* careful: top 32 bits may be junk */
	caddr_t		buffer;
	unsigned long	nbytes;	    /* careful: top 32 bits may be junk */
	caddr_t		arg;
	unsigned long	flag;	    /* careful: top 32 bits may be junk */
    } *uap = (struct a *) args;

    register klic_t *kp;		/* Points into unit cache */
    register struct exit_klic *xp;
    int uses;
    klic_t data;			/* For copy of user's buffer */
    int error = ESUCCESS;               /* Assume no error */

/*
 * "normal" indentation not followed here to leave a meaningful amount
 * of space per line for the action routines
 */

#if LMF_DEBUG
    if( lmf_debug )
        printf("setlminfo: *1* nbytes=%ld flag=%ld\n", uap->nbytes,uap->flag);
#endif

    if ((int)(uap->flag) == LMF_SETSMM) {   /* Alpha: clear top 32 bits */
	register unsigned int act_cpus, max_cpus;
	unsigned int cpus = 0; 

	/* Reset the SMM according to the actual or provided
	 * number of processors.
	 */

#if     SEC_BASE
        if (!privileged(SEC_SYSATTR, EPERM))
		return (EPERM);
#else
	if (suser(u.u_cred,&u.u_acflag))
		return (error);
#endif

	if (uap->buffer) {
		if (error = copyin(uap->buffer, (caddr_t) &cpus,
				       sizeof cpus))
			return (error);
	}
	if (!smm_tablep || (max_cpus = *smm_tablep) < 2)
		return (error);
	act_cpus = count_cpus();
	if (cpus < act_cpus)
		cpus = act_cpus;
	if (cpus > max_cpus)
		cpus = max_cpus;
	lmf_smm = smm_tablep[cpus];
	lic_cpus = cpus;
#if LMF_DEBUG
        if( lmf_debug )
            printf("setlminfo: cpus=%d act_cpus=%d max_cpus=%d\n",
                    cpus,act_cpus,max_cpus);
            printf("setlminfo: lmf_smm=%d lic_cpus=%d\n",
                    lmf_smm,lic_cpus);
#endif
	return (error);
    }

    /* If we get here then the buffer should be a klic_t.
     * Copy it in and see if we have a match on product/producer names.
     */

    if ((int)(uap->nbytes) < sizeof (klic_t)) {  /* Alpha: clear top 32 bits */
	return(EINVAL);
    }

    if (error = copyin(uap->buffer, (caddr_t) &data, sizeof data)) {
	return (error);
    }

    lock_write(&lk_lmf);
    kp = find_klic(data.kl_product_name, data.kl_producer);

#if LMF_DEBUG
    if( lmf_debug )
        printf("setlminfo: find_klic returned 0x%x\n", kp);
#endif

    switch ((int)(uap->flag) & LMF_CMASK) { /* Alpha: clear top 32 bits */
    case LMF_SETLIC:			/* Initial load of license data */
#if     SEC_BASE
        if (!privileged(SEC_SYSATTR, EPERM))
		break;
#else
	if (suser(u.u_cred,&u.u_acflag))
		break;
#endif

#if !P_FAMILY

/*  The following statement clears the P_FAMILY flag of a license
 *  so that the license becomes an ordinary license.  Thus P_FAMILY
 *  licenses are accepted, but do not confer the intended benefit
 *  of the P_FAMILY option (of allowing a user to run the licensed 
 *  program in several processes and just pay for the first process).
 *
 *  At present, the routines that implement family licenses are 
 *  present in this module in their ULTRIX form, and will not run 
 *  in SILVER.  Remove this comment and the following statement when 
 *  the necessary porting changes have been made and family licenses 
 *  work.  -- carlosc
 */
	data.kl_flags.fl_family = 0;

#endif /*!P_FAMILY*/

	if (kp) {
		error = EBUSY;
		break;
	}
	if (data.kl_flags.fl_family) {	/* Needs more space */
		kp = (klic_t *)kalloc(sizeof (struct xklic));
		if(!kp) 
			return ENOMEM; 
	} else {
#if LMF_DEBUG
                if( lmf_debug )
                        printf("setlminfo: Allocate non-family license\n");
#endif
		kp = (klic_t *)kalloc(sizeof (klic_t));
		if(!kp) 
			return ENOMEM; 
	} 
	data.kl_next = klic_head;	/* Ready for structure copy */
	klic_head = kp;
	*kp = data;			/* Copy the data structure */
	kp->kl_product_name[LMF_PRODUCT-1] = '\0';	/* Healthy paranoia */
	kp->kl_producer[LMF_PRODUCER-1] = '\0';
#if LMF_DEBUG
        if( lmf_debug ) {
                printf("setlminfo: product <%s>\n", kp->kl_product_name);
                printf("producer<%s>\n", kp->kl_producer);
        }
#endif
	break;
    case LMF_ADJLIC:			/* Modify loaded license data */
#if     SEC_BASE
        if (!privileged(SEC_SYSATTR, EPERM))
		break;
#else
	if (suser(u.u_cred,&u.u_acflag))
		break;
#endif

	if (!kp) {
		error = ENOENT;
		break;
	}

	/* Copy option */

	if ((int)(uap->flag) & LMFF_COPY) {  /* Alpha: clear top 32 bits */
		/* Error to change the fl_family flag. */
		if ( kp->kl_flags.fl_family != data.kl_flags.fl_family) {
			error = EBUSY;
			break;
		}
		kp->kl_release_date = data.kl_release_date;
		kp->kl_termination = data.kl_termination;
		kp->kl_version = data.kl_version;
		kp->kl_act_charge = data.kl_act_charge;
		kp->kl_max_cpus = data.kl_max_cpus;
		kp->kl_flags = data.kl_flags;
	}

	/* Update units fields */

	kp->kl_locked_units += 
		data.kl_locked_units;
	kp->kl_total_units += 
		data.kl_total_units;
	kp->kl_usable_units += 
		data.kl_usable_units;

	test_del(kp);
	break;
    case LMF_GETAUTH:		/* Check/charge for authorised use of product */
	error = getauth(&data, kp,(int)(uap->flag));  /* Alpha: clr top 32 */
	break;
    case LMF_RELAUTH:		/* Release any units charged for product use */
	if (uap->arg) {
		if (error= copyin(uap->arg, (caddr_t) &uses, sizeof uses))
			break;
	} else 
		uses = 0;

	error = relauth(kp, uses);
	break;
    default:
#if LMF_DEBUG
        if( lmf_debug )
                printf("setlminfo: ERROR sub-op is wrong, = %ld\n",uap->flag);
#endif
	error = EINVAL;
	break;
    }
    lock_done(&lk_lmf);
#if LMF_DEBUG
        if( lmf_debug )
                printf("setlminfo: return, error=0x%x, *retval=0x%lx\n",
                        error, *retval);
#endif
    return (error);
}

/* Functions for requesting and releasing authorisation from within the kernel.
 * Added for DECnet.  Only for DEC products, so producer is hard-coded.
 */

lmf_auth(product, vp, date, flags)
char *product;
ver_t *vp;
time_t date;
int flags;
	{
	register klic_t *kp;
	klic_t data;
	int error = ESUCCESS;             /* Assume no error */

#if LMF_DEBUG
        if( lmf_debug )
                printf("lmf_auth: Enter\n");
#endif

	lock_write(&lk_lmf);

	kp = find_klic(product, "DEC");
	data.kl_version = *vp;		/* These two are all getauth() needs */
	data.kl_release_date = date;
	error = getauth(&data, kp, flags);
	lock_done(&lk_lmf);
	return error;
}

lmf_relauth(product, uses)
char *product;
int uses;
	{
	register klic_t *kp;
	int error = ESUCCESS;             /* Assume no error */

#if LMF_DEBUG
        if( lmf_debug )
                printf("lmf_relauth: Enter routine\n");
#endif

	kp = find_klic(product, "DEC");
	error = relauth(kp, uses);
	return error ? -1 : 0;
}

/* Internal function for LMF_GETAUTH.
 * Args are requestor's klic pointer, internal klic pointer and flags.
 */

static int
getauth(dp, kp, flags)
register klic_t *dp, *kp;
int flags;
	{
	register struct exit_klic *xp;
        struct timeval tv;
        int error = ESUCCESS;             /* Assume no error */

#if LMF_DEBUG
        if( lmf_debug )
                printf("getauth: Enter routine\n");
#endif

	if (!kp) {
		return ENOENT;
	}

	/* Check that the license data we have found is good for the
	 * version making the request.  Only one of version and
	 * release date should be present.  Version 0.0 or release
	 * date 0 mean no check.
	 */
	if (((kp->kl_version.v_major != 0 || kp->kl_version.v_minor != 0) &&
	     (kp->kl_version.v_major < dp->kl_version.v_major ||
	      kp->kl_version.v_major == dp->kl_version.v_major &&
	      kp->kl_version.v_minor < dp->kl_version.v_minor)) ||
	    (kp->kl_release_date != 0 &&
	     kp->kl_release_date < dp->kl_release_date)) {
#if LMF_DEBUG
        if( lmf_debug )
                printf("ERR: License is not current\n");
#endif
		return ERANGE;
	}

	/* Check that the license has not expired */

#if LMF_DEBUG
        if( lmf_debug )
                printf("getauth: Before call on microtime\n");
#endif

        microtime( &tv ); 

#if LMF_DEBUG
        if( lmf_debug )
                printf( "getauth: termination=%d tv.tv_sec=%d\n", 
                         kp->kl_termination, tv.tv_sec );
#endif

	if (kp->kl_termination !=0 &&
	    kp->kl_termination < tv.tv_sec) {
		return ETIMEDOUT;
	}

	/* A license can be blocked by turning on unlicensed CPUs.
	 */

	if (kp->kl_flags.fl_blocked) {
		return EDOM;
	}

	/* If this is a valid availability license, there is no more to do.
	 */

	if (kp->kl_act_charge != 0) {

		/* Activity license.
		 * Usually no need to allocate units if we already have some.
		 */
#if LMF_DEBUG
        if( lmf_debug )
                printf("getauth: Activity license, charge = %d\n", kp->kl_act_charge);
#endif

		xp = find_xa(kp);
		if (xp && (flags & LMFF_MORE) == 0) {
#if LMF_DEBUG
        if( lmf_debug )
                printf("getauth: OK -- Has exit action and is not LMFF_MORE\n");
#endif
			return error;		/* Success! */
		}

		/* If the product has the P_FAMILY option
		 * check for an inherited "right to execute".
		 */
#if P_FAMILY
		if (kp->kl_flags.fl_family) {
			if (!xp)	/* Ignore LMFF_MORE for this */
				error = auth_family(kp);
			return error;
		}
#endif /*P_FAMILY*/

		/* Allocate units */
		
		if (kp->kl_act_charge <= kp->kl_usable_units)
		    kp->kl_usable_units -= kp->kl_act_charge;
		else {
			return EDQUOT;
		}
#if LMF_DEBUG
                if( lmf_debug )
                        printf("getauth: OK, charge=%d new usable=%d\n",
                            kp->kl_act_charge,kp->kl_usable_units);
#endif

		/* Record the allocation */

		if (xp == NULL) {
			xp = new_xk(kp);
			if(!xp) 
				return ENOMEM; 
			xp->xk_next = u.u_exitp;
			u.u_exitp = (struct exit_actn *)xp;
		} else
			++xp->xk_uses;
#if LMF_DEBUG
                if( lmf_debug ) {
                    printf("getauth: Current exit action chain:\n");
                        {
                        struct exit_klic *xp_temp = (struct exit_klic *)u.u_exitp;
                        while( xp_temp ) {
                            printf("  uses=%d charge=%d isrel=%d klic=0x%x\n",
                                xp_temp->xk_uses,
                                xp_temp->xk_charge,
                                xp_temp->xk_func == rel_units,
                                (int)xp_temp->xk_klic );
                            xp_temp = (struct exit_klic *)xp_temp->xk_next;
        		    }
                        }
        	    }
#endif
	}
        return error;
}

/* Internal function for LMF_RELAUTH */

static int
relauth(kp, uses)
register klic_t *kp;
register int uses;
	{
	register struct exit_klic *xp;
	register struct exit_actn **pp;
        int error = ESUCCESS;             /* Assume no error */

#if LMF_DEBUG
        if( lmf_debug )
                printf("relauth: Enter, uses=%d\n",uses);
#endif

	if (!kp) {
		return ENOENT;
	}
	
	xp = find_xa(kp);
	if (!xp)
		return error;
#if P_FAMILY
	if (kp->kl_flags.fl_family) {
		/* release_family() returns 1 for preserved master exit_klic */
		if (release_family(xp))
			uses = 0;
		else {
			unlink_family(xp);
			uses = xp->xk_uses;
		}
	} else
#endif /*P_FAMILY*/
        {
		if (uses < 0 || uses > xp->xk_uses)
			return EINVAL;
		if (uses == 0)
			uses = xp->xk_uses;
#if LMF_DEBUG
                if( lmf_debug )
                        printf("relauth: non-family, new uses=%d, old usable=%d\n",
                                uses, kp->kl_usable_units);
#endif
	}
	if (uses) {
		if ((kp->kl_usable_units += uses*xp->xk_charge) == 0)
#if LMF_DEBUG
        if( lmf_debug )
                printf("relauth: usable units is zero\n");
#endif
		    test_del(kp);
	}

	if ((xp->xk_uses -= uses) <= 0 || kp->kl_flags.fl_family) {

		/* Disconnect the exit_actn structure */

#if LMF_DEBUG
        if( lmf_debug )
                printf("relauth: xk_uses<=0 or family: xk_uses=%d\n",xp->xk_uses);
#endif
		pp = &u.u_exitp;
		while (*pp != (struct exit_actn *)xp) {
			pp = &(*pp)->xa_next;  /* Search down chain */
		}
		*pp = xp->xk_next;
	}

	if (xp->xk_uses <= 0) {			/* Delete exit_klic struct */
#if LMF_DEBUG
        if( lmf_debug )
                printf("relauth: free exit_actn storage\n");
#endif
		kfree((caddr_t)xp,sizeof *xp);
#if LMF_DEBUG
        if( lmf_debug ) {
                printf("relauth: Current exit action chain:\n");
                    {
                    struct exit_klic *xp_temp = (struct exit_klic *)u.u_exitp;
                    while( xp_temp ) 
		        {
                        printf("  uses=%d charge=%d isrel=%d klic=0x%x\n",
                            xp_temp->xk_uses,
                            xp_temp->xk_charge,
                            xp_temp->xk_func == rel_units,
                            (int)xp_temp->xk_klic );
                        xp_temp = (struct exit_klic *)xp_temp->xk_next;
		        }
		    }
        }
#endif
	}
        return error;
}

/* Find the klic_t which matches a given product and producer name */

static klic_t *
find_klic(pdct, pdcr)
char *pdct, *pdcr;
	{
	register char *p1, *p2;
	register klic_t *kp;

#if LMF_DEBUG
        if( lmf_debug ) {
                printf("find_klic: Enter routine\n");
                printf("  product = %s\n", pdct );
                printf("  producer = %s\n", pdcr );
        }
#endif

#ifdef SMP_DEBUG
	if (smp_debug)
		lsert(&lk_lmf, "find_klic");
#endif

	for (kp = klic_head; kp; kp = kp->kl_next) {
		p1 = pdct;
		p2 = kp->kl_product_name;
#if LMF_DEBUG
        if( 0 )  /* This is in the inner loop and  produces too much output ! */
                printf("find_klic: check product = %s\n", p2);
#endif
		while (*p1) {
			if (*p1++ != *p2++)
				goto bad;
		}
		if (*p2)
			goto bad;
		p1 = pdcr;
		p2=kp->kl_producer;
#if LMF_DEBUG
        if( lmf_debug )
                printf("find_klic: check producer = %s\n", p2);
#endif
		while (*p1) {
			if (*p1++ != *p2++)
				goto bad;
		}
		if (*p2 == '\0')
			return kp;
bad:		;
	}
#if LMF_DEBUG
        if( lmf_debug )
                printf("find_klic: return failure\n");
#endif
	return 0;
}

/* Find the exit_klic structure pointing to a particular klic_t */

static struct exit_klic *
find_xa(dp)
register klic_t *dp;
	{
	register struct exit_klic *xp, *rxp;

#if LMF_DEBUG
        if( lmf_debug )
                printf("find_xa: Enter routine\n");
#endif

#ifdef SMP_DEBUG
	if (smp_debug)
		lsert(&lk_lmf, "find_xa");
#endif

	xp = (struct exit_klic *)u.u_exitp;	/* List header */
	while (xp && (xp->xk_func != rel_units || xp->xk_klic != dp))
		xp = (struct exit_klic *) xp->xk_next;

	/* Because CPUs can be turned on and off, the SMM may have changed
	 * and with it the activity charge for this product.  Fix it.
	 */
	if (rxp = xp) {
		if (xp->xk_indirect)
		    xp = xp->xk_indirect;
		if (xp && xp->xk_charge != dp->kl_act_charge) {
			dp->kl_usable_units += xp->xk_uses *
			    (xp->xk_charge - dp->kl_act_charge);
			xp->xk_charge = dp->kl_act_charge;
		}
	}
	return rxp;
}

/* Create and initialise an exit_klic struct */
/* Returns NULL if kalloc fails */

static struct exit_klic *
new_xk(kp)
register klic_t *kp;
	{
	register struct exit_klic *xp;

#if LMF_DEBUG
        if( lmf_debug )
                printf("new_xk: Enter routine\n");
#endif

	if( xp = (struct exit_klic *)kalloc(sizeof *xp) ) {
		bzero(xp, sizeof *xp);
		xp->xk_func = rel_units;
		xp->xk_klic = kp;
		xp->xk_uses = 1;
		xp->xk_charge = kp->kl_act_charge; }
	return xp;
}

/* Exit action routine to release activity units.   Called on process exit */

static int
rel_units(xp)
register struct exit_klic *xp;
	{
	register klic_t *kp;
	register int no_del;

#if LMF_DEBUG
        if( lmf_debug ) {
                printf("rel_units: Enter routine\n");
	}
#endif

	kp = xp->xk_klic;
	lock_write(&lk_lmf);
#if P_FAMILY
	if (kp->kl_flags.fl_family) {
		no_del = release_family(xp);
		unlink_family(xp);
	} else
#endif /*P_FAMILY*/
		no_del = 0;
	if (!no_del) {
		if ((kp->kl_usable_units += xp->xk_uses*xp->xk_charge) == 0) {
			test_del(kp);
		}
	}
#if LMF_DEBUG
        if( lmf_debug )
                printf("rel_units: uses=%d usable=%d\n",
                    xp->xk_uses, kp->kl_usable_units);
#endif
	lock_done(&lk_lmf);
	return no_del;
}

/*
 * Function to process exit actions that are chained to the u_exitp
 * field of the user struct.  This function is called from the exit
 * routine in lmf_exit.c when the process dies.
 *
 * This function is more general than LMF requires.  It was introduced
 * (into the original ULTRIX version) with the idea that it might be
 * used for end action outside of LMF; however, at present, the only
 * user is LMF.
 */
int
lmf_exit_actn()
{
    register struct exit_actn *xp, *nxp, *pxp;

    for (pxp=NULL,xp = u.u_exitp; xp; xp = nxp) {
	nxp = xp->xa_next;
	if ((*xp->xa_func)(xp)) {

#if LMF_DEBUG
	if( lmf_debug )
		printf("lmf_exit_actn: do not deallocate\n");
#endif
	    pxp = xp;
	    continue;       /* Don't deallocate */
	}
	else {
#if LMF_DEBUG
	    if( lmf_debug )
		    printf("lmf_exit_actn: deallocate\n");
#endif
	    if (pxp)
		pxp->xa_next = nxp;
	    else
		u.u_exitp = nxp;

	    kfree((caddr_t)xp,sizeof *xp);
	}
    }
}

/* Function to delete klic_t structures that are no longer meaningful */

static void
test_del(kp)
register klic_t *kp;
	{
	register klic_t **skp;

#if LMF_DEBUG
        if( lmf_debug )
                printf("test_del: Enter routine\n");
#endif

#ifdef SMP_DEBUG
	if (smp_debug)
		lsert(&lk_lmf, "test_del");
#endif
	if (kp->kl_usable_units != 0 || kp->kl_total_units != 0)
		return;

	/* Remove from klic_t chain */

	skp = &klic_head;
	while (skp && *skp != kp)
		skp = &(*skp)->kl_next;
	if (skp)
		*skp = kp->kl_next;
	kfree((caddr_t)kp,sizeof *kp);
}

#if P_FAMILY

/* Functions to handle the P_FAMILY option on a PAK.
 * This says that if the calling process has an ancestor which has
 * allocated units or shares a login ancestor with an allocator
 * then it can run for free.
 * This is mainly for Ultrix OS licenses, but it may be useful
 * to applications.
 * To avoid moving the exit_action list pointer to the proc struct and
 * locking it, we do this by chaining the exit_klic structures to the klic_t
 * and hashing on the owner pid to improve performance.
 */

/* Handle the GETAUTH function for products licensed with the
 * P_FAMILY option.   The first part is a search for a
 * related process which has already allocated units.
 *
 * There are four possible results of the search: one of these
 * values is put in the variable outcome:
 */

#define FAILL	0	/* No match, lk_runqs locked */
#define FAILR	1	/* No match, *pp is ref'ed */
#define FOUND	2	/* Found an exit_klic structure, *pp is ref'ed */
#define LOGINP	3	/* No match, but found a login process, which is ref'ed */

static int
auth_family(kp)
klic_t *kp;
	{
	register struct xklic *xkp = (struct xklic *) kp;
	register struct proc *pp;
	register struct exit_klic *nxp = 0;
	register int pid;
	struct exit_klic *ixp;		/* Pointer to master exit_klic */
	int outcome;			/* Result from process search loop */
	struct proc *wp = 0;		/* Identifies process being waited on */
	static struct proc *proc_rele_hold();
        int error = ESUCCESS;           /* Assume no error */

#ifdef SMP_DEBUG
	if (smp_debug)
		lsert(&lk_lmf, "test_family");
#endif
	pp = u.u_procp;

	/* Handle the special case that this is a login process or has 
	 * previously used and released authorization for the product and a
	 * decendant is using the product.  Then a master exit_klic is
	 * accessible via our pid.  Attach it.
	 */

	pid = pp->p_pid;
	nxp=xkp->xkl_usage[pid & HASHMASK];
	while (nxp ) {
		if (nxp->xk_pid == pid)
		    break;
		nxp = nxp->xk_forw;
	}
	if (nxp) {
		nxp->xk_next = u.u_exitp;	/* Attach this exit_klic */
		u.u_exitp = (struct exit_actn *)nxp;
		++nxp->xk_refs;
		return error;
	}

	outcome = FAILL;
	if (pp->p_ppid != 1) {		/* Avoid locking! */

		/* Climb up the process tree looking at our ancestors to see if
		 * any of them are already authorised to use this product.
		 * The SMP lock on the proc struct linkages is released when
		 * searching for a matching exit_klic in case this is
		 * a popular product!
		 */

		spl5();
		lock_write(&lk_procqs);
		do {			/* Search process tree up to init */
			pp = pp->p_pptr;	/* Parent */
			pid = pp->p_pid;
			nxp=xkp->xkl_usage[pid & HASHMASK]; /* Head of list */
			if (nxp) {
				/* Release lk_procqs before searching */
				
				if (!proc_ref(pp)) {	/* Process is dying */
					outcome = FAILL;
					break;
				}
 				lock_done(&lk_procqs);
				spl0();
				if (wp)	/* Wakeup deferred by proc_rele_hold()*/
				    wakeup(&wp->p_ref);
				
				do {	/* Search for matching exit_klic */
					if (nxp->xk_pid == pid)
						break;
					nxp = nxp->xk_forw;
				} while (nxp);
				
				if (nxp) {
					outcome = FOUND;
					break;
				}
				if (pp->p_type & SLOGIN) {
					outcome = LOGINP;
					break;
				}
				if (pp->p_ppid == 1) {
					outcome = FAILR;
					break;
				}

				/* Get back lk_procqs and release process */
				wp = proc_rele_hold(pp);
			}
		} while (!(pp->p_type & SLOGIN) && pp->p_ppid != 1);

		/* Cleanup locking.  The LMF lock is enough to continue. */

		if (outcome == FAILL) {
			if (pp->p_type & SLOGIN)	/* Normal loop exit */
				outcome = LOGINP;
			lock_done(&lk_procqs);
			spl0();
		} else
			proc_rele(pp);
		if (wp)	/* Wakeup deferred from proc_rele_hold() */
			wakeup(&wp->p_ref);
	}

	if (outcome != FOUND) {

		/* Allocate units.  Note kp is same as xkp. */

		if (kp->kl_act_charge <= kp->kl_usable_units)
		    kp->kl_usable_units -= kp->kl_act_charge;
		else {
			return EDQUOT;
		}

		/* This process will now be recorded as the `head' of a
                 * new family of processes whose members are authorised to
		 * use this product.   Ensure this is cleaned up.
		 */

		set_lmf_login();
	}

	if (outcome == LOGINP) {
		/* Make a master exit klic for this family and associate it
		 * with the login ancestor.
		 */
		nxp = new_xk(xkp);
		if(!nxp) 
			return ENOMEM;
		add_family(nxp, pid);
	}

	/* Make an exit_klic to remove allocation for family if this
	 * process is the last survivor.
	 */
	ixp = nxp;
	nxp = new_xk((klic_t *)xkp);
	if(!nxp) 
		return ENOMEM;
	nxp->xk_next = u.u_exitp;
	u.u_exitp = (struct exit_actn *)nxp;
	add_family(nxp, u.u_procp->p_pid);
	if (ixp) {
		if (ixp->xk_indirect)	/* Point to master. */
			ixp = ixp->xk_indirect;
		++ixp->xk_refs;		/* Count extra reference to master. */
		nxp->xk_indirect = ixp;
		nxp->xk_uses = 0;
	} else
		nxp->xk_refs = 1;

        return error;
}

static void
add_family(xp, pid)
register struct exit_klic *xp;
pid_t pid;
	{
	register struct xklic *xkp = (struct xklic *) xp->xk_klic;
	register struct exit_klic *head;

#ifdef SMP_DEBUG
	if (smp_debug)
		lsert(&lk_lmf, "add_family");
#endif
	head = xkp->xkl_usage[pid & HASHMASK];
	if (head)
		head->xk_back = xp;
	xp->xk_forw = head;
	xp->xk_back = 0;
	xp->xk_pid = pid;
	xkp->xkl_usage[pid & HASHMASK] = xp;
}

/* Release the notional usage of units associated with the argument.
 * If it is the master exit_klic for a family this means just decrementing
 * the reference count.  Otherwise the reference count on the master must be
 * decremented and if it becomes zero the master is deallocated after 
 * tranferring its units to the argument for release by the caller.
 * The return status is non-zero if the argument exit_klic should be saved.
 */

static int
release_family(xp)
register struct exit_klic *xp;
	{
	register struct exit_klic *nxp;

#ifdef SMP_DEBUG
	if (smp_debug)
		lsert(&lk_lmf, "release_family");
#endif
	if (nxp = xp->xk_indirect) {
		if (--nxp->xk_refs <= 0) {
			xp->xk_charge = nxp->xk_charge;
			xp->xk_uses = nxp->xk_uses;
			unlink_family(nxp);
			kfree((caddr_t)nxp,sizeof *nxp);
		}
		return 0;
	}
	return --xp->xk_refs > 0;
}

/* Unlink an exit_klic from the hash chains of its klic.
 * It may not be linked, hence the test of xp->xk_pid.
 * The return value is used when this is called as an exit action function.
 */

static void
unlink_family(xp)
register struct exit_klic *xp;
	{
	register struct xklic *xkp = (struct xklic *) xp->xk_klic;
	register struct exit_klic *nxp;
	register int pid;

#ifdef SMP_DEBUG
	if (smp_debug)
		lsert(&lk_lmf, "unlink_family");
#endif

	if (pid = xp->xk_pid) {
		if (nxp = xp->xk_forw)
		    nxp->xk_back = xp->xk_back;
		if (xkp->xkl_usage[pid & HASHMASK] == xp)
		    xkp->xkl_usage[pid & HASHMASK] = nxp;
		if (nxp = xp->xk_back)
		    nxp->xk_forw = xp->xk_forw;
		xp->xk_pid = 0;
	}
}

/* This function is called for a login process (from sys_sysinfo.c) and
 * for any process which becomes head of a `family' of processes by being
 * the first to allocate units from a P_FAMILY license.
 * It makes sure that any exit_klic structures for this process are
 * removed from the klic_t hash chains.
 */

void
set_lmf_login() {
	register struct exit_actn *cp;

	for (cp = u.u_exitp; cp; cp = cp->xa_next)
		if (cp->xa_func == clean_family)
			break;
	if (!cp) {
		if( cp = (struct exit_actn *)kalloc(sizeof *cp) ) {
			cp->xa_func = clean_family;
			cp->xa_next = u.u_exitp;
			u.u_exitp = cp;
		}
                else {
                        action when kalloc fails [to be determined]
		}
	}
}

/* Call unlink_family to delete current process' pid from klic hash chains.
 * This is called at process exit for any process which has been head of
 * a family for any product.  Done through the exit_actn mechanism
 * in auth_family().
 */

static int
clean_family() {
	register klic_t *kp;
	register struct exit_klic *xp;
	register pid_t pid;

	pid = u.u_procp->p_pid;
	lock_write(&lk_lmf);
	for (kp = klic_head; kp; kp = kp->kl_next) {
		if (kp->kl_flags.fl_family) {
			xp = ((struct xklic *)kp)->xkl_usage[pid & HASHMASK];
			while (xp) {
				if (xp->xk_pid == pid)
				    break;
				xp = xp->xk_forw;
			}
			if (xp)
				(void) unlink_family(xp);
		}
	}
	lock_done(&lk_lmf);
	return 0;
}


/* proc_rele_hold() is similar to proc_rele() but returns holding
 * lk_procqs at the correct priority level.  Use it when releasing
 * ref'ed process with the intention to chain on to another proc.
 * If the return value is non-null then it is a pointer to the
 * process which must be woken up *after* lk_procqs is released.
 */

static struct proc *
proc_rele_hold(p)
	register struct proc *p;
{

	spl5();
	lock_write(&lk_procqs);
	if (--p->p_ref < 0) {
		panic("proc_rele_hold: bad ref");
	}
#ifdef SMP_DEBUG
	if (smp_debug) {
		if (p->p_exist != P_ALIVE && p->p_exist != P_DYING)
			panic("proc_rele_hold: invalid exist");
	}
#endif SMP_DEBUG
	if (p->p_ref == 0 && p->p_exist == P_DYING) {
		return p;
	} else {
		return 0;
	}
}

#endif /*P_FAMILY*/









