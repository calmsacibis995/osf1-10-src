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
static char *rcsid = "@(#)$RCSfile: sys_sysinfo.c,v $ $Revision: 1.1.17.4 $ (DEC) $Date: 1993/07/19 18:34:37 $";
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
 * defines for bootdevice_parser output
#define DEBUG_PRINT_INPUT
#define DEBUG_PRINT_ERRORS
 */

#include "ult_bin_compat.h"
#include "bin_compat.h"
#include "_lmf_.h"

#include <sys/sysinfo.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/conf.h>

#if BIN_COMPAT
#include <sys/systm.h>
extern bin_compat_debug;
#endif /*BIN_COMPAT*/

#if _LMF_
#define SYSINFO_DEBUG 0
int sysinfo_debug = 0;
#endif /*_LMF_*/

/* see comments in sysinfo.h, the order of addresses must match savecore */
extern dev_t dumpdev;

/*
 *	Routines for generic retrieval and storage of kernel information.
 *	These routines are meant to serve as a substitute for the abuse of
 *	ioctl(), fcntl(), and reading/writing /dev/mem.
 *
 *      These system calls are extensible.  In order to  prevent  them
 *      from becoming dumping grounds for inappropriate operations, new
 *      operations should be proposed and reviewed via the design review
 *	process.
 */


extern	int	maxuprc;
int	sys_uac;
extern	Assign_table	*assign_ptr;

#define BOOTDEVLEN 80
#define BOOTTYPELEN 10
extern char bootdevice[BOOTDEVLEN];	/* the name of the bootdevice */
extern char boottype[BOOTTYPELEN];	/* the name of the boottype */

/*
 *	Retrieve system information
 */
int
getsysinfo(p, args, retval) 		/* 256 = getsysinfo */
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		unsigned long	 op;	/* real type: 'unsigned' */
		char		*buffer;/* real type: 'char *' */
		unsigned long	 nbytes;/* real type: 'unsigned' */
		int		*start;	/* real type: 'int *' */
		char		*arg;	/* real type: 'char *' */
	} *uap = (struct args *) args;

	int error = 0;
	u_short progenv = 2;
	dev_t ttyd;
	Assign_entry	*aptr;
	extern hal_getsysinfo();

	switch ((int)uap->op) {

	      case GSI_PROG_ENV:	/* get programming environment */
		if( (int)uap->nbytes < sizeof(progenv) ) 
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)&progenv,
			(caddr_t)uap->buffer, sizeof(progenv) );
		    *retval = 1;
		}
		break;

	      case GSI_MAX_UPROCS:	/* get max number of procs per uid */
		if( (int)uap->nbytes < sizeof(maxuprc) ) 
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)&maxuprc,
			(caddr_t)uap->buffer, sizeof(maxuprc) );
		    *retval = 1;
		}
		break;

	      case GSI_TTYP:		/* get controlling tty dev */
		if( (int)uap->nbytes < sizeof(dev_t) ) 
		    error = EINVAL;
		else {
		    struct vnode *vnp;

		    PROC_LOCK(p);
		    if(p->p_flag & SCTTY) {
			SESS_LOCK(p->p_session);
			if  (vnp = p->p_session->s_ttyvp) 
			    ttyd = vnp->v_rdev;
			else
			    ttyd = NODEV;
		        SESS_UNLOCK(p->p_session);
		    }
		    PROC_UNLOCK(p);
		    if (ttyd != NODEV)  {
			error = copyout((caddr_t)&ttyd,
				    (caddr_t)uap->buffer, 
				    sizeof(dev_t) );
		        *retval = 1;
		    }
		    else
		        *retval = 0;
		}
		break;

#if	_LMF_
	      case GSI_LMF:	/* LMF (Licence Mmanagement Facility) */
#if	SYSINFO_DEBUG
                if( sysinfo_debug ) {
                    printf("OK: getsysinfo -- Enter: op=0x%lx\n", uap->op);
	        }
#endif	/* SYSINFO_DEBUG */
                error = getlminfo( p, args, retval );
#if	SYSINFO_DEBUG
                if( sysinfo_debug )
                    printf("OK: getsysinfo -- Return: error=%d  *retval=%ld\n",
                           error,*retval);
#endif	/* SYSINFO_DEBUG */
		break;
#else
#if	ULT_BIN_COMPAT
	      case GSI_LMF:	/* LMF facility */
		*retval = 1;
		break;
#endif	/* ULT_BIN_COMPAT */
#endif	/*_LMF_*/

#if	ULT_BIN_COMPAT
	      case GSI_UACSYS:       	/* get system wide flag */
		if( uap->nbytes < sizeof(sys_uac) )
		  error = EINVAL;
		else {
		    error = copyout((caddr_t)&sys_uac,
			(caddr_t)uap->buffer,
			sizeof(sys_uac) );
		    *retval = 1;
		}
		break;
	      case GSI_UACPARNT:	/* get parents */
		if( uap->nbytes < sizeof(p->p_pptr->p_uac) )
		  error = EINVAL;
		else {
		  error = copyout((caddr_t)&p->p_pptr->p_uac,
			(caddr_t)uap->buffer, sizeof(p->p_pptr->p_uac) );
		  *retval = 1;
		}
		break;

	      case GSI_UACPROC:		/* get current proc */
		if( uap->nbytes < sizeof(p->p_uac) )
		  error = EINVAL;
		else {
		  error = copyout((caddr_t)&p->p_uac,
			(caddr_t)uap->buffer, sizeof(p->p_uac) );
		  *retval = 1;
		}
		break;

	      case GSI_MMAPALIGN:	/* support for mmap device drivers */
		uprintf("getsysinfo: %d is not implemented\n", (int)uap->op);
		break;

	      /* /usr/sbin/sizer needs physmem */
	      case GSI_PHYSMEM:		/* Amount of physical memory in KB */
		{
		    extern int physmem;
		    int physmem_kb = (physmem * PAGE_SIZE) / 1024;

		    error = copyout((caddr_t)&physmem_kb, (caddr_t)uap->buffer,
				min((int)uap->nbytes, sizeof(physmem_kb)));
		    *retval = 1;
		}
		break;

#endif /* ULT_BIN_COMPAT */

		/* DDF added boottype */
	      case GSI_BOOTTYPE:	/* Network Interface boot type */
		if( (int)uap->nbytes < BOOTTYPELEN ) 
		    error = EINVAL;
		else {
		    error = copyoutstr((caddr_t)boottype,
			(caddr_t)uap->buffer, BOOTTYPELEN, (int*)0);
		    *retval = 1;
		}
		break;

#ifndef __alpha
	      case GSI_BOOTDEV:	
		if( (int)uap->nbytes < BOOTDEVLEN ) 
		    error = EINVAL;
		else {
		    error = copyoutstr((caddr_t)bootdevice,
			(caddr_t)uap->buffer, BOOTDEVLEN, (int*)0);
		    *retval = 1;
		}
		break;
#endif /* __alpha */

	      case GSI_ROOTDEV:	
		{
			extern dev_t rootdev;
			error = copyout((caddr_t)&rootdev, (caddr_t)uap->buffer,
					min((int)uap->nbytes, sizeof(rootdev)));
			*retval = 1;
		}
		break;

	      case GSI_DUMPDEV:
		{
			extern dev_t dumpdev;
			error = copyout((caddr_t)&dumpdev, (caddr_t)uap->buffer,
					min((int)uap->nbytes, sizeof(dumpdev)));
			*retval = 1;
		}
		break;

#ifdef TODO
	      case GSI_SWAPDEV:	
		{
			extern swapdev;
			error = copyout((caddr_t)&swapdev, (caddr_t)uap->buffer,
					min((int)uap->nbytes, sizeof(swapdev)));
			*retval = 1;
		}
		break;
#endif /* TODO */

	      case GSI_STATIC_DEF:	/* get minor/devname pair */
		{
		char 		*cname;
		int		s;
		long		nxt;	/* holds a pointer */
		Assign_table	*t;
		char 		*dname;
		int		i;
		Assign_entry	*aptr;
		int		*iptr;

		if (assign_ptr == 0) {
			*retval = 0;
		} else if ((uap->buffer == 0) || 
		           (uap->start  == 0) || 
		           (uap->arg    == 0))  {
			error = EFAULT;
		} else if (uap->nbytes < sizeof (Assign_entry)) {
			error = EINVAL;
		} else {
			/* get pointer to callers Assign_entry buffer. 
			 */
			aptr = (Assign_entry *) uap->buffer;

			/* get index into minor and dev_name arrays
			 */
			copyin (uap->start, &s, sizeof(int));

			/* get config_name to search for.  If input
			 * pointer was zero, then set config_name to be
			 * the first entry in assign table.
			 */
			copyin (uap->arg, &nxt, sizeof(char *));

			/* if the index into the minor array is a -1, then this 
			 * is the signal that the caller is looking for the 
			 * device info for a specific config name.  The 
			 * config_name is in the callers Assign_entry.
			 */
			if (nxt == (-1)) {
				cname = (char *) aptr->config_name;
				/* search for config_name in assign_table
				 * maybe need to match on bc too???
				 */
				t = assign_ptr;
				while ((t->config_name[0] != '\0') && 
				       (strcmp(t->config_name, cname))) {
					t++;
				}
			} else {
				t = assign_ptr + nxt;
				cname = (char *) (t->config_name);
			}

			if (t->config_name[0] == '\0') {
				/* did not find match.  that's ok.
				 * just means the driver isn't installed.
				 * flag this case by setting the return
				 * value to 0
				 */
				*retval = 0;
			} else {

				/* t is now pointing to the entry in
				 * assign_table which hold the information
				 * for the request device.
				 * now, find the index into the minor
				 * and dev_name arrays.
				 */
				dname = t->dev_name;
				for (i=0; i<=s; i++) {
					if (dname[0] == '\0') {
						error = EFAULT;
					}
					dname += ANAMELEN;
				}
				if (error == EFAULT) {
					*retval = -1;
				} else {
					/* return dname to point to
					 * desired entry in dev_name array
					 */
					dname -= ANAMELEN;

					/* t, s and dname are now set
					 * up so that they point to the
					 * info for this device's
					 * info.  copy the data into
					 * the callers buffer.  three
					 * copyouts are needed because
					 * the base information is in
					 * the assign_table, but the
					 * minor number and device name
					 * are in two separate arrays.
					 */
					copyout (t, aptr, sizeof(Assign_table));
					copyout (&(t->at_minor[s]), 
					    &(aptr->ae_minor), sizeof(int));
					copyout (dname, aptr->dev_name, 
					    sizeof(aptr->dev_name));

					/* set up the return value, the
					 * number of bytes copied.
					 */
					*retval = sizeof (Assign_entry);

					/* Finally, set up pointers for
					 * the next call to GSI_STATIC_DEF
					 */
					dname += ANAMELEN;
					if (dname[0] != '\0') {
						/* there is another
						 * minor device for this 
						 * device driver
						 */
						s++;
					} else {
						s = 0;
						t++;

						if (t->config_name[0] == '\0') {
							nxt = 0;
						} else {
							nxt++;
						}
					}

					/* copy these values back to user space
					 */
					copyout (&s, uap->start, sizeof(int));
					copyout (&nxt, uap->arg, 
					    sizeof(char *));
				}
			}
		}
		}
		break;


#if BIN_COMPAT
		/* Return the addres of the first binary compatability
		 * module's configure function. The cfmgr can call this
		 * function to perform operations on the module */
		case GSI_COMPAT_MOD:
		 {
			struct compat_mod buf;
			char *start;

			/* Get the suggested start value */
			if (error = copyin(uap->start,&(char *)start,
				sizeof(char *)))
				break;

			/* Get the needed struct */
			if(error = cm_get_struct(&start, &buf))
				break;

			/* Copy the struct to use space */
			if(uap->nbytes != sizeof(struct compat_mod)) {
				error = EINVAL;
				break;
			}
			if(error = copyout((caddr_t)&buf, 
			    (caddr_t)uap->buffer, uap->nbytes))
				break;

			/* Update start */
			error = copyout((caddr_t)&start, (caddr_t)uap->start,
                                        sizeof(char *));
                        *retval = 0;
		 }
		break;
#endif /* BIN_COMPAT */

	      case GSI_SIA_PROC_CRED_VAL:
		{
		  pid_t pid, mypid;
		  struct proc *p, *proc_ptr;
		  long proc_cred_val;

		  p = u.u_procp;

		  if(uap->nbytes < (sizeof(pid_t)))
		    {
		      error = EINVAL;
		      break;
		    }

		  if (error = copyin(uap->arg, &pid, sizeof(pid_t)))
		    break;

		  PROC_LOCK(p);
		  mypid = p->p_pid;
		  PROC_UNLOCK(p);

		  if(pid != -1 && pid != mypid)
		    {
		      error = EACCES;
#if     SEC_BASE
		      if (!privileged(SEC_SYSATTR, 0))
			break;
#else
		      if (suser(u.u_cred, &u.u_acflag)) {
			break;
		      }
#endif
		      if((proc_ptr = pfind(pid)) == (struct proc *) 0)
			{
			  error = EINVAL; /* pid not found */
			  break;
			}

		      PROC_LOCK(proc_ptr);
		      if(proc_ptr->p_pid != pid)
			{
			  error = EINVAL; /* pid not found */ 
			  PROC_UNLOCK(proc_ptr);
			  break;
			}
		      crhold(proc_ptr->p_rcred);
		      proc_cred_val = proc_ptr->p_rcred->cr_sia_proc_cred_val;
		      crfree(proc_ptr->p_rcred);
		      PROC_UNLOCK(proc_ptr);

		    }
		  else
		    {
		      proc_cred_val = u.u_cred->cr_sia_proc_cred_val;
		    }

		  error = copyout((caddr_t)&proc_cred_val,
				  (caddr_t)uap->buffer,
				  sizeof(proc_cred_val));
		}
		break;

	      case GSI_CLK_TCK:		/* get ticks per second */
		{
		  extern int	hz;
		  if( (int)uap->nbytes < sizeof(hz) ) 
		    error = EINVAL;
		  else {
		    error = copyout((caddr_t)&hz,
				    (caddr_t)uap->buffer, sizeof(hz));
		    *retval = 1;
		  }
	        }
		break;

	      case GSI_IPRSETUP:	/* get IP routing status */
		{
		  extern int ipforwarding, ipgateway;
		  int t;

		  if ((int)uap->nbytes < sizeof(int)) {
		    error = EINVAL;
		    break;
		  }
		  t = ((ipforwarding != 0) << 1) | (ipgateway != 0);
		  if (!(error = copyout((caddr_t)&t, uap->buffer, sizeof(int))))
		    *retval = 1;
		}
		break;

	      default:

		error = hal_getsysinfo(p, args, retval);
		break;
	}
	return (error);
}

/*
 *	Set/Store system information
 */

#define MAX_NVPAIRS_NAMES 100	/* max number of name-value pairs per call */

int	nfsportmon;		/* NFS secure port monitoring? */

#ifdef	MACH
#else	MACH
extern int	kernel_locking; /* kernel style file locking? */
#endif	MACH

int tta;
int mlil = 0; /* BVT */
int
setsysinfo(cp, args, retval) 		/* 257 = setsysinfo */
	struct proc *cp;
	void *args;
	long *retval;
{
	register struct args {
		unsigned long	op;	/* real type: 'unsigned' */
		caddr_t		buffer;	/* real type: 'caddr_t' */
		unsigned long	nbytes;	/* real type: 'unsigned' */
		caddr_t		arg;	/* real type: 'caddr_t' */
		unsigned long	flag;	/* real type: 'unsigned' */
	} *uap = (struct args *) args;

	register int		i;
	caddr_t			p;
	int			name;
	int			int_val;
	int			error = 0;
	dev_t			tmp_dumpdev;
	extern			hal_setsysinfo();

/*
 * "normal" indentation not followed here to leave a meaningful amount
 * of space per line for the action routines
 */

switch ((int)uap->op) {

      /*
       * Name-Value pairs form of input
       *
       * An action routine for each `name' is found in the switch statement
       * below. It is the responsibility of this action routine to validate
       * its value (setting error if an error exists) and also
       * to advance the argument buffer pointer `p' so that it is ready for
       * the next `name' copyin in the for-loop below.
       */

      case SSI_NVPAIRS:

	if ((int)uap->nbytes < 1 || (int)uap->nbytes > MAX_NVPAIRS_NAMES) {
		/* nbytes = number of pairs, let's not get crazy here */
		error = EINVAL;
		break;
	}

	p = uap->buffer;

	for (i = 0; i < (int)uap->nbytes; ++i) {
		if (error = copyin(p, &name, sizeof(int)))
			break;
		p += sizeof(int);

		switch (name) {
		      /*
		       * Turn NFS secure port monitoring on/off
		       */
		      case SSIN_NFSPORTMON:
#if     SEC_BASE
			if (!privileged(SEC_SYSATTR, 0)) {
				error = EPERM;
				break;
			}
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				break;
#endif
			if (error = copyin(p, &int_val, sizeof(int)))
				break;
			if (int_val < 0 || int_val > 1) {
				error = EINVAL;
				break;
			}
			p += sizeof(int);
			nfsportmon = int_val;
		        break;

		      /*
		       * Turn kernel style file locking on/off
		       */
		      case SSIN_NFSSETLOCK:
			uprintf("setsysinfo: SSIN_NFSSETLOCK doesn't work\n");
			break;
		      case SSIN_UACSYS:
#if     SEC_BASE
			if (!privileged(SEC_SYSATTR, 0)) {
				error = EPERM;
				break;
			}
#else
			if(suser(u.u_cred, &u.u_acflag)) {
			  error = EPERM;
			  break;
			}
#endif
			if(error = copyin(p, &int_val, sizeof(int)))
			  break;
		 	p += sizeof(int);
			sys_uac = int_val;
			break;
		      case SSIN_UACPARNT:
			if (error = copyin(p, &int_val, sizeof(int)))
				break;
			p += sizeof(int);
			if(cp->p_pptr->p_pid != 1)
			  cp->p_pptr->p_uac = int_val;
			else
			  error = EPERM;
			break;
		      case SSIN_UACPROC:
			if (error = copyin(p, &int_val, sizeof(int)))
				break;
			p += sizeof(int);
			cp->p_uac = int_val;
			break;
		      /*
		       * Set programming environment
		       */
		      case SSIN_PROG_ENV:
#ifdef	MACH
			if (error = copyin(p, &int_val, sizeof(int)))
				break;
			p += sizeof(int);
/*			uprintf("setsysinfo: SSIN_PROG_ENV %d\n", int_val);*/
#else	MACH
		/* We don't have ultrix code for this */
#endif	MACH
			break;
		      default:
#ifdef	MACH
			uprintf("setsysinfo: name = %d ????\n", name);
#endif	MACH
			/*
			 * The name is not a SWOE type.  Call down to
			 * the HAL portion for processing.  This approach
			 * enables a user level program to mix a set of
			 * nvparis which includes both HAL & SWOE types.
			 */
	  		error = hal_setsysinfo_nvpair(&p, name);
			break;
		}
		if (error)
			return (error);
	}
	break;

      case SSI_ZERO_STRUCT:	/* zero a structure */
	error = EOPNOTSUPP;
	break;

      case SSI_SET_STRUCT:	/* set a structure to supplied */
                                /* values */
	error = EOPNOTSUPP;
	break;

#if	_LMF_
      case SSI_LMF:		/* LMF (License Management Facility) */
#if	SYSINFO_DEBUG
        if( sysinfo_debug ) {
            printf("OK: setsysinfo -- Enter: op = %lx\n", uap->op);
	}
#endif	/* SYSINFO_DEBUG */
        error = setlminfo( cp, args, retval );
#if	SYSINFO_DEBUG
        if( sysinfo_debug )
            printf("OK: setsysinfo -- Return error=%d  *retval=%ld\n",error,*retval);
#endif	/* SYSINFO_DEBUG */
	break;
#else
#if	ULT_BIN_COMPAT
      case SSI_LMF:		/* LMF facility */
	error = 0;		/* all requests return O.K. */
	break;
#endif	/* ULT_BIN_COMPAT */
#endif	/* _LMF_ */

#if	ULT_BIN_COMPAT
      case SSI_LOGIN:		/* Identify caller as a login process */
	uprintf("setsysinfo: SSI_LOGIN doesn't work\n");
	error = EINVAL;
	break;
#endif /* ULT_BIN_COMPAT */

      case SSI_SLIMIT: /* BVT */

        error = EACCES;
#if     SEC_BASE
        if (!privileged(SEC_SYSATTR, 0))
          break;
#else
        if (suser(u.u_cred, &u.u_acflag)) {
          uprintf("setsysinfo: Sorry, must be super-user\n");
          break;
        }
#endif
        error = 0;
        tta = (int)uap->nbytes;
        break;

      case SSI_ULIMIT:
        error = tta ? qulim() : 0;
        break;

      case SSI_DUMPDEV:
        error = EACCES;
#if     SEC_BASE
        if (!privileged(SEC_SYSATTR, 0))
          break;
#else
        if (suser(u.u_cred, &u.u_acflag)) {
          uprintf("setsysinfo: Sorry, must be super-user\n");
          break;
        }
#endif
	if (error = copyin(uap->buffer, &tmp_dumpdev, sizeof(dev_t)))
	    break;
	dumpdev = tmp_dumpdev;
	break;

      case SSI_SIA_PROC_CRED_VAL:
	{
	  pid_t pid, mypid;
	  struct proc *p, *proc_ptr;
	  struct ucred *newcr, *opcred;
	  long proc_cred_val;

	  p = u.u_procp;

	  if(uap->nbytes != (sizeof(pid_t)))
	    {
	      error = EINVAL; /* bad length in */
	      break;
	    }

	  error = EACCES;

#if     SEC_BASE
	  if (!privileged(SEC_SYSATTR, 0))
	    break;
#else
	  if (suser(u.u_cred, &u.u_acflag)) {
	    break;
	  }
#endif
	  if (error = copyin(uap->arg, &pid, sizeof(pid_t)))
	    break;
		  
	  if (error = copyin(uap->buffer, &proc_cred_val,sizeof(proc_cred_val)))
	    break;

	  PROC_LOCK(p);
	  mypid = p->p_pid;
	  PROC_UNLOCK(p);

	  if(pid != -1 && pid != mypid)
	    {

	      if((proc_ptr = pfind(pid)) == (struct proc *) 0)
		{
		  error = EINVAL; /* pid not found */
		  break;
		}

	      PROC_LOCK(proc_ptr);
	      crhold(proc_ptr->p_rcred);
	      newcr = crcopy(proc_ptr->p_rcred);
	      newcr->cr_sia_proc_cred_val = proc_cred_val;

	      if(proc_ptr->p_pid != pid)
		{
		  error = EINVAL; /* pid not found */
		  PROC_UNLOCK(proc_ptr);
		  crfree(newcr);
		  break;
		}
	      opcred = proc_ptr->p_rcred;
	      proc_ptr->p_rcred = newcr;
	      crfree(opcred);
	      PROC_UNLOCK(proc_ptr);
	    }
	  else
	    {
	      PROC_LOCK(p);
	      newcr = crcopy(p->p_rcred);
	      crhold(newcr);
	      newcr->cr_sia_proc_cred_val = proc_cred_val;
	      opcred = p->p_rcred;
	      p->p_rcred = newcr;
	      u.u_cred = newcr;
	      crfree(opcred);
	      PROC_UNLOCK(p);
	    }
	}
	break;

      case SSI_IPRSETUP:	/* set IP routing status */
	{
	  extern int ipforwarding, ipgateway;
	  int t;

	  error = EACCES;
#if     SEC_BASE
	  if (!privileged(SEC_SYSATTR, 0))
            break;
#else
	  if (suser(u.u_cred, &u.u_acflag)) {
            uprintf("setsysinfo: Sorry, must be super-user\n");
            break;
	  }
#endif
	  if (!(error = copyin(uap->buffer, &t, sizeof(int)))) {
	    ipforwarding = ((t & 2) != 0);
	    ipgateway = ((t & 1) != 0);
	  }
	}
	break;

      default:
	  error = hal_setsysinfo(cp, args, retval);
	  break;
      }
      return (error);
}

int qulim()
{  register int s;
   struct proc *procp;
   int ret_val = 0;

   procp = u.u_procp;
   s = splhigh();
   while( (procp->p_ppid != 1) && (procp->p_flag & SLOGIN) == 0)
     procp = procp->p_pptr;
   (void) splx(s);
   if( (procp->p_flag & SLOGIN) == 0) {
     if(mlil < tta) {
       ++mlil;
       u.u_procp->p_flag |= SLOGIN;
     } else {
       ret_val = 1;
     }
   }
   return(ret_val);
}
