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
static char *rcsid = "@(#)$RCSfile: priocntl.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/14 13:54:48 $";
#endif

#include <rt_sched_rq.h>
#include <mach/kern_return.h>
#include <sys/types.h>
#include <mach/policy.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/procset.h>
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#include <sys/time.h>
#include <sys/audit.h>
#include <sys/tspriocntl.h>
#include <kern/sched.h>
#include <kern/queue.h>
#include <kern/lock.h>
#include <mach/policy.h>

#define FIXEDPRI_ENABLED 0

#define	PC_VALID (0x01)
#define	IDTYPE_VALID(t) ((t >= P_PID) && (t <= P_ALL))
#define	CLASS_VALID(i) ((i > 0) &&	\
			(i <= CLASS_MAX) && \
			(sched_record[i].flags & PC_VALID))
#define CLASS_MAX   ((sizeof(sched_record) / sizeof(struct sched_record)) -1)

#define get_thread(__p, __t)						\
	task_lock((__p)->task);						\
	(__t) = 							\
	  (thread_t) queue_first((queue_head_t *)&(__p)->task->thread_list); \
	task_unlock((__p)->task);

/*
 * Data/definitions for auditing control.
 */
typedef u_long audvec_t[4];
#define AUD_PID      0
#define AUD_PRIORITY 1
#define AUD_POLICY   2
#define AUD_QUANTUM  3
#define SET_AUDIT_VEC(__vec, __pid, __pri, __pol, __quan)		\
		(__vec)[AUD_PID] = (__pid);				\
		(__vec)[AUD_PRIORITY] = (__pri);			\
		(__vec)[AUD_POLICY] = (u_long)(__pol);			\
		(__vec)[AUD_QUANTUM] = (__quan);

static char *tss = "TS (set)";
static char *rts = "RT (set)";
static char *rrs = "PSX_RR (set)";
static char *ffs = "PSX_FIFO (set)";

/* Each scheduling class has unique requirements set in a framework of class
 * independent activities.  To simplify the code, these class specific
 * requirements are abstracted out into a set of procedures as listed in the
 * sched_ops structure.  The procedures themselves are local and found at the
 * end of this source file.  Following are the forward declarations.
 */

#if FIXEDPRI_ENABLED
int fixedpri_is_class_member();
#endif
int inval_get_parms();
int empty_info();
int inval_set_parms();
int inval_set_permissible();
int posix_fifo_is_class_member();
int posix_rr_is_class_member();
int rt_is_class_member();

#if RT_SCHED_RQ
int rt_get_parms();
int rt_info();
int rt_set_parms();
int rt_set_permissible();
int posix_rr_get_parms();
int posix_fifo_get_parms();
int posix_info();
int posix_rr_set_parms();
int posix_fifo_set_parms();
int posix_set_permissible();
#endif

int ts_get_parms();
int ts_info();
int ts_is_class_member();
int ts_set_parms();
int ts_set_permissible();

/* There is one sched_record associated with each scheduling class.
 * Because the SVR4 RT class uses the posix real-time classes, RT only
 * exists when the real-time classes are defined.  The priocntl() will
 * still work, but will contain only the TS and FIXEDPRI classes.
 */

/*
 * Class ids in SVR4 either start at zero, or use zero for the 'sys' class.
 * There really is no concept of a 'sys' class in OSF/1.  To simplify the
 * code, class ids should be the same as indexes into 'sched_record'.  'C'
 * arrays are always zero-origin, so we can either start at zero (different
 * than SVR4) or have a dummy first element.  The elements are small enough
 * that I prefer to waste the small amount of space and have the class ids
 * similar to SVR4.
 */

/* Just because, with 1-based class ids, the first element is never valid. */
#define FIRST_CLASS_ID 1

static struct sched_record
{
  int flags;
  char name[PC_CLNMSZ];
  int (*info)( /* buffer pointer */ );
  int (*get_parms)( /* process pointer, buffer pointer */ );
  int (*set_parms)( /* process pointer, buffer pointer */ );
  int (*is_class_member)( /* process pointer */ );
  int (*set_permissible)( /* calling and target process pointers, parms */ );
} sched_record[] = 
{
  { (0), "sys", 0, 0, 0, 0, 0, } ,
  { (PC_VALID), "TS",
    ts_info,
    ts_get_parms,
    ts_set_parms,
    ts_is_class_member,
    ts_set_permissible,
  },
  
  /* The SVR4 real-time scheduling class is predicated on the POSIX real-time
   * work.  Thus, if the POSIX work isn't here, we don't have a real-time class
   * to configure.
   */
  
#if RT_SCHED_RQ
  { (PC_VALID), "RT",
    rt_info,
    rt_get_parms,
    rt_set_parms,
    rt_is_class_member,
    rt_set_permissible,
  },
  { (PC_VALID), "PSX_FF",
    posix_info,
    posix_fifo_get_parms,
    posix_fifo_set_parms,
    posix_fifo_is_class_member,
    posix_set_permissible,
  },
  { (PC_VALID), "PSX_RR",
    posix_info,
    posix_rr_get_parms,
    posix_rr_set_parms,
    posix_rr_is_class_member,
    posix_set_permissible,
  },
#endif

#if FIXEDPRI_ENABLED
  { (PC_VALID), "FIXPRI",
    empty_info,
    inval_get_parms,
    inval_set_parms,
    fixedpri_is_class_member,
    inval_set_permissible,
  },
#endif
  
};


struct args
{
  procset_t	*psp;
  /* cmd is really an int, but is passed to the kernel 
   * in a long on an alpha
   */
  long		cmd;
  void		*argp;
};

static char *
hexprint(unsigned long l)
{
  static char buf[11];
  int j;
  
  buf[0] = '0';
  buf[1] = 'x';
  for (j=9; j > 1; j--)
  {
    char x = l & 0xF;
    
    buf[j] = ((x < 10)
	      ? x + '0'
	      : x - 10 + 'A');
    l >>= 4;
  }
  buf[10] = '\0';
  return (buf);
}

/*
 * WARNING - this set of indices must be kept in 
 * sync with priocntl()'s internal class list, above.
 */
#define	TS_CLASS	1
#if RT_SCHED_RQ
#	define	RT_CLASS	2
#	define	PSX_FF_CLASS	3
#	define	PSX_RR_CLASS	4
#	ifdef FIXEDPRI_ENABLED
#		define	FIXPRI_CLASS	5
#	endif	/* FIXEDPRI_ENABLED */
#else	/* RT_SCHED_RQ */
#	ifdef FIXEDPRI_ENABLED
#		define	FIXPRI_CLASS	2
#	endif	/* FIXEDPRI_ENABLED */
#endif	/* RT_SCHED_RQ */

int
priocntlset(p, args, retval)
    struct proc *p;
    void *args;
    long *retval;
{
  struct args *uap = (struct args *)args;
  pcinfo_t parm;
  pcparms_t pcparms;
  procset_t procset;
  int error;
  
  ASSERT(syscall_on_master());
  *retval = 0;
  switch (uap->cmd) {
    default:
      return (EINVAL);
    case PC_ADMIN:
      return (ENOSYS);
    
    /* For PC_GETCID and PC_GETCLINFO, get the class ID or class name and
     * attributes for a specific class given the class name or class id,
     * respectively.  If the argument is non-null, it is assumed
     * to point to a pcinfo_t structure with the pc_cid containing the
     * class id or pc_clname field containing the class name, respectively,
     * of the target being queried.  On success, pc_clname is set to
     * the class name, pc_cid is set to the class ID, pc_clinfo
     * is set to the class-specific attributes.  If pc_cid or pc_clname,
     * respectively, doesn't contain a valid, configured class, return -1
     * with errno set to EINVAL.  On success, the number of currently
     * configured classes is returned.
     */
    
    case PC_GETCID:
    {
      int i;
      if (uap->argp != 0)
      {
	/* If supplied, copy in the pcinfo_t structure. */
	if (error=copyin(uap->argp, &parm, sizeof(pcinfo_t)))
	return (error);
	
	/* We were given the name of the class for which information is
	 * requested.  Search for a string match.
	 */
	for (i=FIRST_CLASS_ID;
	     (i <= CLASS_MAX) &&
	     (strncmp(&(sched_record[i].name[0]),
		      &(parm.pc_clname[0]),
		      PC_CLNMSZ)
	      !=0);
	     i++);
	
	/* If no string matching the class name was found, error out. */
	if (!CLASS_VALID(i))
	  return (EINVAL);
	
	parm.pc_cid = i;
	
	if (error = do_getclinfo(&parm))
	  return (error);
	
	/* copy out the resulting information */
	if (error = copyout(&parm, uap->argp, sizeof(pcinfo_t)))
	  return (error);
      }
      
      /* On success, always return the number of configured classes. */
      *retval = valid_class_count();
      return (KERN_SUCCESS);
    }
    
    case PC_GETCLINFO:
      if (uap->argp != 0) {
	
	/* If the pcinfo_t structure is supplied copy it in. */
	if (error=copyin(uap->argp, &parm, sizeof(pcinfo_t)))
	return (error);
	
	if (!CLASS_VALID(parm.pc_cid))
          return (EINVAL);
	
	/* Fill in the class name of the validated class. */
	strncpy(&(parm.pc_clname[0]),
		&(sched_record[parm.pc_cid].name[0]),
		PC_CLNMSZ);
	
	if (error=do_getclinfo(&parm))
	return (error);
	
	/* The information has been transferred to the kernel variable "parm",
	 * but must still be passed back out to user space.
	 */
	if (error=copyout(&parm, uap->argp, sizeof(pcinfo_t)))
	  return (error);
      }
      
      /* On success, always return the number of configured classes. */
      *retval = valid_class_count();
      return (KERN_SUCCESS);

    case PC_SETPARMS:
      /* There must be a pcparms_t structure as a parameter for this call. */
      if (error=copyin(uap->argp, &pcparms, sizeof(pcparms_t)))
	return (error);
      if (error=copyin(uap->psp, &procset, sizeof(procset_t)))
	return (error);
      error = do_setparms(p, &pcparms, &procset, retval);
      return error;
    
    case PC_GETPARMS:
      /* The pcparms_t parameter is required. */
      if (error = copyin(uap->argp, &pcparms, sizeof(pcparms_t)))
      	return (error);
      if (error = copyin(uap->psp, &procset, sizeof(procset_t)))
	return (error);
      if (!(error = do_getparms(p, &pcparms, &procset, retval)))
	error = copyout(&pcparms, uap->argp, sizeof(pcparms_t));
      return error;
  } /* switch (uap->cmd) */
}

static int
do_getclinfo(parm)
    pcinfo_t *parm;
{
  int count, error;
  id_t id;
  
  /* idtype and id are ignored. */
  
  /* The caller validates pc_cid. */
  id = parm->pc_cid;
  
  if (error = (*sched_record[id].info)(parm->pc_clinfo))
    return (error);
  return (KERN_SUCCESS);
}

static int
valid_class_count()
{
  int i, count;
  
  /* There is one additional "valid, configured class" for which we are unable
   * to get information: 'sys'.  For this reason, count starts at one rather
   * than zero.  (Also, for all other operations, the 'sys' class is NOT
   * valid.)
   */
  for (i = FIRST_CLASS_ID, count = 1; i <= CLASS_MAX; i++) {
    if (sched_record[i].flags & PC_VALID)
      count++;
  }
  return (count);
}


#if RT_SCHED_RQ
#define SOLITARY(p) (((p)->p_pid == 1) || posix_rr_is_class_member(p) || \
		     posix_fifo_is_class_member(p))
#else
#define SOLITARY(p) ((p)->p_pid == 1)
#endif

static int
do_setparms(p, parms, psp, retval)
    struct proc *p;
    pcparms_t *parms;
    procset_t *psp;
    long *retval;
{
  int error, e;	/* error storage: e is a transient error code.  see below */
  int i, is_su, select_count;
  id_t cid_index;
  struct proc *selected_proc, *cached_proc, *current_proc;
  audvec_t audvec; 

  if (!CLASS_VALID(parms->pc_cid))
    return (EINVAL);

  cid_index = parms->pc_cid;
  
  /* The minimum permissions required for setting paramters are either
   * the real or effective uids matching the calling process, or the
   * calling process is superuser.
   */
  is_su = suser(p->p_rcred, 0) == KERN_SUCCESS;
  
  /* If the value of id is P_MYID, the content of id is replaced with
   * the value of the corresponding id_type referring to this process.
   */
  if (error = validated_selection_criterion( p, psp ))
    return (error);

  /* 
   * Note that there is error storage in two variables: error and e. 
   * Error is the error code that will eventually be returned to the
   * caller and e is the error return from an inferior procedure. 
   * SVR4 requires that EPERM be returned if any of the processes
   * specified meet all the selection criteria except that the caller
   * has insufficient privilege to change the target process.  At the
   * same time, all processes in the target that can be changed must
   * be changed if the only error to be returned is EPERM.  Thus, when
   * we receive an EPERM, we have to try to change the rest of the
   * members of the set.
   */
  
  /* We assume that this system call is running on the master processor so
   * that the process table is protected.
   */
  current_proc = allproc;
  select_count = 0;
  cached_proc = (struct proc *)0;
  /* Traverse all active process structures in the system.
   * 'sys' class processes are never considered.
   */
  do {
    if (current_proc != NULL ) {

      if ( !selected(current_proc, psp) )
	goto next_proc;

      select_count++;
      selected_proc = current_proc;
      
      /*
       * There are some situations where we operate on a particular
       * process only if there are no other processes selected.  To
       * do this, cache the first such "solitary" process and allow
       * operation on it only after determining that no other
       * processes have been selected.  If more than one process has
       * already been selected, don't bother caching the "solitary"
       * process.
       */
      if ( (select_count == 1) && (SOLITARY(selected_proc))) {
          cached_proc = selected_proc;
	  selected_proc = (struct proc *)0;
          goto next_proc;	/* delay processing */
      }
    } else {

      /*
       * No more procs in allproc list.  Check if there is a cached
       * proc and it was the only one selected.  If so, operate on it
       * now.  Otherwise, we're finished.
       */
      if ( (select_count == 1) && cached_proc ) {
	selected_proc = cached_proc;
	cached_proc = (struct proc *)0;

      } else {
	/* no more procs to operate on */
	break;
      }
    }
    
    /* perform setparms operation on selected proc */

    /* check permissions */
    if (!is_su &&
	(p->p_ruid != selected_proc->p_ruid) &&
	(p->p_ruid != selected_proc->p_svuid) &&
	(p->p_svuid != selected_proc->p_ruid) &&
	(p->p_svuid != selected_proc->p_svuid))
    {
      error = EPERM;
      SET_AUDIT_VEC(audvec, selected_proc->p_pid, 0, NULL, 0);
      AUDIT_CALL(u.u_event, error, audvec, *retval, AUD_HPR, "Q0000000");
      goto next_proc;	/* try the next proc */
    }
    
    /* There might be additional tests involved when specifying a
     * particular class.
     */
    if (e=((*sched_record[cid_index].set_permissible)
	   ( p, selected_proc, parms->pc_clparms))) {
      if (e == EPERM) {
	error = EPERM;
	SET_AUDIT_VEC(audvec, p->p_pid, 0, NULL, 0);
	AUDIT_CALL(u.u_event, error, audvec, *retval, AUD_HPR, "Q0000000");
	goto next_proc;	/* try the next proc */
      }
      SET_AUDIT_VEC(audvec, p->p_pid, 0, NULL, 0);
      AUDIT_CALL2(u.u_event, e, audvec, *retval, AUD_HPR, "Q0000000");
      return (e);	/* return other errors directly */
    }
    
    /* We have selected a process and have permission to set the
     * parameters. Attempt the operation.
     *
     * This is audited in the subroutine.
     */
    if (e = ((*sched_record[cid_index].set_parms)
	   (p, selected_proc, parms->pc_clparms, audvec))) {
      if (e == EPERM) {
	error = EPERM;
	goto next_proc;	/* try the next proc */
      }
      return (e);	/* return other errors directly */
    }
    
    next_proc:
    current_proc = current_proc->p_nxt;
  } while (1);
  
  /* If there is already an error (EPERM), return that.  If there was no
   * previous error, but no processes were selected, return ESRCH.
   * Otherwise, return success.
   */
  if (error)
    return (error);
  else if (select_count==0) {
    SET_AUDIT_VEC(audvec, 0, 0, NULL, 0);
    AUDIT_CALL2(u.u_event, ESRCH, audvec, *retval, AUD_HPR, "00000000");
    return (ESRCH);
  } else
    return (KERN_SUCCESS);
}

/* Some value that is always smaller than a valid priority. */
#define NO_PRIORITY (-32768)

static int
do_getparms(p, parms, psp, retval)
    struct proc *p;
    pcparms_t *parms;
    procset_t *psp;
    long *retval;
{
  int error;
  pid_t current_pid;
  struct proc *current_proc;
  struct proc *selected_proc = (struct proc *)0; /* chosen process */
  audvec_t audvec;
  
  /* The pc_cid field must either be PC_CLNULL or a valid and
   * configured class.  If the request criterion is a class, then
   * PC_CLNULL doesn't make sense.  However, it doesn't appear to
   * be illegal, even though it doens't make sense, so we'll allow
   * it.  If there ends up being more than one selected process
   * when pc_cid == PC_CLNULL, this will cause an EINVAL error to
   * be returned.
   */
  if ((parms->pc_cid != PC_CLNULL) && (!(CLASS_VALID(parms->pc_cid))))
   	return EINVAL;

  /* If the value of id is P_MYID, the content of id is replaced with
   * the value of the corresponding id_type referring to this process.
   */
  if (error = validated_selection_criterion( p, psp ))
    return (error);

  /* 
   * Note that there is error storage in two variables: error and e. 
   * Error is the error code that will eventually be returned to the
   * caller and e is the error return from an inferior procedure. 
   * SVR4 requires that EPERM be returned if any of the processes
   * specified meet all the selection criteria except that the caller
   * has insufficient privilege to change the target process.  At the
   * same time, all processes in the target that can be changed must
   * be changed if the only error to be returned is EPERM.  Thus, when
   * we receive an EPERM, we have to try to change the rest of the
   * members of the set.
   */
  
  /* Test programs on 386 SVR4 show that a request with an idtype of
   * P_CID and the id of a valid and configured class still results
   * in EINVAL.  I feel this is an implementation anomally and see no
   * real reason for this behavior.  Hence, I won't include the
   * additional check required for it.  If a bug is published due
   * to this behavior, here is where such a test would be added.
   */

  /* We assume that this system call must run on the master processor so
   * that the process table is protected.
   */
  
  /*
   * Traverse all active process structures in the system.  'sys'
   * class processes are never considered.
   *
   * Only look at valid running procs (the allproc list), not all
   * procs in system.  If just looking at the allproc list breaks
   * this system call, then lots of other things can be considered
   * broken as well, because fork(), killpg(), and other kernel
   * routines all use the allproc list.
   */
  for (current_proc = allproc; current_proc != NULL; 
    current_proc = current_proc->p_nxt) {
    
    /* To be selected, the process must
     * 1.) be in the selected set;
     * 2.) either be PC_CLNULL or be in the requested class;
     * 3.) have information successfully returned.
     * 
     * Further, in order for this to be the process about which information
     * is returned, it must be either:
     * 1.) the first process matching the desired information; or,
     * 2.) a better process about which to return information.
     */
    
    if (selected(current_proc, psp)) {
      if (parms->pc_cid == PC_CLNULL) {
	if (selected_proc == (struct proc *)0) {
	  selected_proc = current_proc;
	}
	else
	{
	  return (EINVAL);
	}
      }
      else if ((*sched_record[parms->pc_cid].is_class_member)(current_proc))
      {
	if (selected_proc == (struct proc *)0)
	  selected_proc = current_proc;
	else {
          register thread_t curt, selt;

	  get_thread(current_proc, curt);
	  get_thread(selected_proc, selt);

          if (curt->priority > selt->priority)
	    selected_proc = current_proc;
	}
      }
    }
  }
  
  if (selected_proc == (struct proc *)0)
    return (ESRCH);
  
  /* Since all the operations worked, the return value will be the
   * pid_t of the selected process.  If this operations produced only
   * one process in the set, its possible that the value of pc_cid is
   * PC_CLNULL.  In that case, we have to determine the class id to fill
   * in the pc_cid field and to use as a selector for calling the proper
   * routine requesting the parameters.
   */
  if (parms->pc_cid == PC_CLNULL) {
    int i;
    
    for (i=FIRST_CLASS_ID;
	 (i <= CLASS_MAX) &&
	 !(*sched_record[i].is_class_member)(selected_proc);
	 i++);
    if (i > CLASS_MAX)
      panic("priocntl(): process without a class found.\n");
    parms->pc_cid = (id_t) i;
  }

  if (error = (*sched_record[parms->pc_cid].get_parms)(selected_proc, 
						       parms->pc_clparms) ) {
    SET_AUDIT_VEC(audvec, selected_proc->p_pid, 0,
		  sched_record[parms->pc_cid].name, 0);
    AUDIT_CALL2(u.u_event, error, audvec, selected_proc->p_pid,
	       AUD_HPR, "Q0r00000");
	return(error);
  }

  *retval = selected_proc->p_pid;
  SET_AUDIT_VEC(audvec, selected_proc->p_pid, 0,
		sched_record[parms->pc_cid].name, 0);
  AUDIT_CALL2(u.u_event, 0, audvec, *retval, AUD_HPR, "Q0r00000");
  return (KERN_SUCCESS);
}


int
unix_error(error)
    kern_return_t error;
{
  switch (error) {
    case KERN_INVALID_ARGUMENT:
      return EINVAL;
      break;
    case KERN_FAILURE:
      return EPERM;
      break;
    default:
      /*
       * An unexpected error code was returned.
       */
      panic("priocntl");
      break;
  }
}

/****************************************************************
 * 'TS' scheduling class operations				*
 ****************************************************************/

/*
 * The priority ranges for timesharing processes are defined as the
 * range (-TSMAXUPRI ..  TSMAXUPRI).  This implies an odd number of
 * priorities (counting the priority 0).  OSF/1 may allow (and
 * currently does allow) an even number of priorities.  This has to be
 * adjusted for.  By subtracting one from the OSF/1 range we either
 * change the range from an odd value to the next smallest even
 * number, which will give us a TSMAXUPRI that covers all valid OSF/1
 * values, or we reduce an even value to the next smallest odd number,
 * which gives us a TSMAXUPRI that covers all except the highest OSF/1
 * priority.  Consequently, the value priocntl() returns as the
 * maximum priority may actually be one less than it really is.
 */
#define	TSMAXUPRI	((BASEPRI_LOWEST - BASEPRI_HIGHEST - 1) / 2)

/*
 * These macros change the priority between queue numbers as used
 * internally by OSF/1, and time sharing priorities, as used by SVR4.
 */
#define	TS_TO_Q( p )	(-p + (BASEPRI_LOWEST - TSMAXUPRI))
#define	Q_TO_TS( p )	(-1 * (p - (BASEPRI_LOWEST - TSMAXUPRI)))

static int
ts_info(buf)
    void *buf;
{
  tsinfo_t *tsinfo = (tsinfo_t *)buf;
  
  tsinfo->ts_maxupri = TSMAXUPRI;
  return (KERN_SUCCESS);
}

static int
ts_get_parms( p, buf )
    struct proc *p;
    void *buf;
{
  tsparms_t *tsparms = (tsparms_t *)buf;
  thread_t th;

  get_thread(p, th);

  tsparms->ts_uprilim = Q_TO_TS(th->max_priority);
  tsparms->ts_upri = Q_TO_TS(th->priority);
  return (KERN_SUCCESS);
}

static int
ts_set_parms( p, t, buf, audvec )
    struct proc *p;
    struct proc *t;
    void *buf;
    audvec_t audvec;
{
  tsparms_t *tsparms = (tsparms_t *)buf;
  boolean_t is_su;
  thread_t th;

  get_thread(t, th);

  is_su = suser(p->p_rcred, 0) == KERN_SUCCESS;
  if (!ts_is_class_member(t)) {
    int previously_rt;
    
    if (tsparms->ts_uprilim == TS_NOCHANGE)
      tsparms->ts_uprilim = 0;
    else if ((tsparms->ts_uprilim > 0) && !is_su) {
      SET_AUDIT_VEC(audvec, t->p_pid, TS_TO_Q(tsparms->ts_uprilim), tss, 0);
      AUDIT_CALL2(u.u_event, EPERM, audvec, 0, AUD_HPR, "Qar00000");
      return EPERM;
    }
    
    if ((tsparms->ts_uprilim > TSMAXUPRI) ||
	(tsparms->ts_uprilim < -TSMAXUPRI)) {
      SET_AUDIT_VEC(audvec, t->p_pid, TS_TO_Q(tsparms->ts_uprilim), tss, 0);
      AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
      return EINVAL;
    }

    previously_rt = rt_is_class_member(t);
    if (thread_policy(th, POLICY_TIMESHARE, 0) != KERN_SUCCESS) {
      SET_AUDIT_VEC(audvec, t->p_pid, TS_TO_Q(tsparms->ts_uprilim), tss, 0);
      AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
      return EINVAL;
    }
    /* Only SVR4 RT threads are marked unswappable. */
    if (previously_rt)
      thread_swappable(th, TRUE);
  }

  /* Deal with the upper limit first.  Anyone may lower the priority
   * limit if they are at least owned by the same user.  Raising the
   * priority limit requires superuser privileges.
   */
  if (tsparms->ts_uprilim == TS_NOCHANGE)
    tsparms->ts_uprilim = Q_TO_TS(th->max_priority);
  else {
    if (tsparms->ts_uprilim > Q_TO_TS(th->max_priority)) {
      if (!is_su) {
        SET_AUDIT_VEC(audvec, t->p_pid, TS_TO_Q(tsparms->ts_uprilim), tss, 0);
        AUDIT_CALL2(u.u_event, EPERM, audvec, 0, AUD_HPR, "Qar00000");
        return EPERM;
      }
    }
    if (thread_max_priority(th, th->processor_set,
		    TS_TO_Q(tsparms->ts_uprilim), TRUE) != KERN_SUCCESS) {
      SET_AUDIT_VEC(audvec, t->p_pid, TS_TO_Q(tsparms->ts_uprilim), tss, 0);
      AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
      return EINVAL;
    }
  }
  
  /* Deal with the actual priority.  Any process having a given owner may
   * change any process having the same owner.  A process owned by superuser
   * may change any process' priority.  If a properly privileged process
   * attempts to exceed the priority limit for the target process, the
   * priority is silently changed to conform to the the limit.
   */
  if (tsparms->ts_upri == TS_NOCHANGE)
    tsparms->ts_upri = Q_TO_TS(th->priority);
  if (tsparms->ts_upri > tsparms->ts_uprilim)
    tsparms->ts_upri = tsparms->ts_uprilim;
  if (thread_priority(th, TS_TO_Q(tsparms->ts_upri), FALSE, TRUE)
      != KERN_SUCCESS) {
    SET_AUDIT_VEC(audvec, t->p_pid, TS_TO_Q(tsparms->ts_upri), tss, 0);
    AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
    return EINVAL;
  }

  SET_AUDIT_VEC(audvec, t->p_pid, TS_TO_Q(tsparms->ts_upri), tss, 0);
  AUDIT_CALL2(u.u_event, 0, audvec, 0, AUD_HPR, "Qar00000");
  return (KERN_SUCCESS);
}

static int
ts_is_class_member( p )
    struct proc *p;
{
  register thread_t th;

  get_thread(p, th);

  return ((th->policy & POLICY_TIMESHARE) == th->policy);
}

static int
ts_set_permissible( p, t, parms )
    struct proc *p, *t;
    void *parms;
{
  /* We check for super-user and matching user ids. */
  if ((p->p_ruid != t->p_ruid) &&
      (p->p_ruid != t->p_svuid) &&
      (p->p_svuid != t->p_ruid) &&
      (p->p_svuid != t->p_svuid) &&
      !(suser(p->p_rcred, 0) == KERN_SUCCESS))
    return (EPERM);

  return (KERN_SUCCESS);
}

/****************************************************************
 * 'RT' scheduling class operations				*
 ****************************************************************/

static int
rt_is_class_member( p )
    struct proc *p;
{
  register thread_t th;

  get_thread(p, th);

  return ((th->policy & POLICY_SVID_RT) == POLICY_SVID_RT);
}

#if RT_SCHED_RQ

extern int svr4_rt_max;
extern int svr4_rt_min;
extern struct rt_default_quantum rt_default_quantum[];
extern int tick;

/*
 * There are NRQS scheduling queues enabled.  The maximum number of
 * queues allowed is NRQS (which is set to the same value as
 * NRQS_MAX).  Queues are searched from the beginning (index zero),
 * with the result that internally, lower numbers are higher priority. 
 * For POSIX and SVR4, higher numbers are higher priority.  Internal
 * priorities are converted to external priorities by subtraction from
 * a constant.  Choosing NRQS (NRQS_MAX) as the constant provides
 * consistency across all configurations of Digital's OSF/1 release.
 *
 * SVR4 realtime priorities are required to range from 0 to some
 * maximum.  This implementation reserves a range of priorities from
 * svr4_rt_min to svr4_rt_max.  These constants are internal
 * priorities (svr4_rt_min > svr4_rt_max).  External SVR4 priorities
 * are produced by subtracting the internal priority from svr4_rt_min.
 */

/* XXX these assume svr4_rt_max is zero! */
#define	RT_TO_Q( p )	(-1 * (p - svr4_rt_min))
#define	Q_TO_RT( p )	(svr4_rt_min - p)

static int
rt_info(buf)
    void *buf;
{
  rtinfo_t *rtinfo = (rtinfo_t *)buf;
  
  /* svr4_rt_min defines the minimum POSIX priority to which an SVR4
   *  priority maps.  The range is required to start at zero, so it is
   *  normalized around svr4_rt_min.
   */
  rtinfo->rt_maxpri = Q_TO_RT( 0 );
  return (KERN_SUCCESS);
}

static int
rt_get_parms( p, buf )
    struct proc *p;
    void *buf;
{
  rtparms_t *rtparms = (rtparms_t *)buf;
  thread_t th;
  int quantum;

  get_thread(p, th);

  quantum = th->sched_data;
  rtparms->rt_pri = Q_TO_RT(th->priority);
  
  /* If this is POLICY_FIFO, then tqnsecs is set to RT_TQINF and
   * rt_tqsecs should probably be initialized to zero, although it is
   * supposed to be ignored.  If this is POLICY_RR, tqsecs is the
   * number of seconds in the quantum and tqnsecs is the number
   * of additional nanoseconds in the quantum.  Convert the quantum into
   * seconds and nanoseconds.  Be careful about the arithmetic to prevent
   * overflow.
   */
  if (th->policy & POLICY_FIFO) {
    rtparms->rt_tqsecs = 0;
    rtparms->rt_tqnsecs = RT_TQINF;
  }
  else {
    thread_t th;

    get_thread(p, th);

    quantum = (th->sched_data * tick) / 1000;   /* time quantum 
							  in milli second */
    rtparms->rt_tqsecs  = quantum / 1000;
    rtparms->rt_tqnsecs = (quantum - (rtparms->rt_tqsecs * 1000)) * 1000000;
  }
  return (KERN_SUCCESS);
}


static int
rt_set_parms(p, t, buf, audvec)
    struct proc *p;
    struct proc *t;
    void *buf;
    audvec_t audvec;
{
  rtparms_t *parms = (rtparms_t *)buf;
  rtparms_t curparms;
  boolean_t may_reschedule, change_policy;
  int error, quantum;
  thread_t th;

  if (rt_is_class_member(t)) {
    /*
     * We are already in the SVR4 RT class.  Determine the current
     * parameters.
     */
    if (error=rt_get_parms(t, &curparms))
      return error;
    
    /*
     * Determine the desired priority.  The priority must be validated
     * here in case it is used as an index to rt_default_quantum.
     */
    if (!(may_reschedule = (parms->rt_pri != RT_NOCHANGE)))
      parms->rt_pri = curparms.rt_pri;
    else if ((parms->rt_pri < 0) || (parms->rt_pri > Q_TO_RT(0))) {
      SET_AUDIT_VEC(audvec, t->p_pid, RT_TO_Q(parms->rt_pri), rts, 0);
      AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
      return EINVAL;
    }
    
    /*
     * Determine the desired quantum.  We may have to switch
     * between fifo and round-robin, despite being already in the
     * target class.
     */
    if (parms->rt_tqnsecs == RT_NOCHANGE) {
      parms->rt_tqsecs  = curparms.rt_tqsecs;
      parms->rt_tqnsecs = curparms.rt_tqnsecs;
      change_policy = FALSE;
    } else {
      if (parms->rt_tqnsecs == RT_TQDEF) {
	parms->rt_tqsecs  = rt_default_quantum[parms->rt_pri].rt_tqsecs;
	parms->rt_tqnsecs = rt_default_quantum[parms->rt_pri].rt_tqnsecs;
      }
      
      change_policy = ((parms->rt_tqnsecs == RT_TQINF) ==
		       (curparms.rt_tqnsecs == RT_TQINF));
    }
  } else {
    /*
     * We are moving from a different class to RT.  Validate the
     * parameters.
     */
    may_reschedule = TRUE;
    if (parms->rt_pri == RT_NOCHANGE)
      parms->rt_pri = 0;
    else if ((parms->rt_pri < 0) || (parms->rt_pri > Q_TO_RT(0))) {
      SET_AUDIT_VEC(audvec, t->p_pid, RT_TO_Q(parms->rt_pri), rts, 0);
      AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
      return EINVAL;
    }
    
    if ((parms->rt_tqnsecs == RT_TQDEF) ||
	(parms->rt_tqnsecs == RT_NOCHANGE)) {
      parms->rt_tqsecs  = rt_default_quantum[parms->rt_pri].rt_tqsecs;
      parms->rt_tqnsecs = rt_default_quantum[parms->rt_pri].rt_tqnsecs;
    }
    
    change_policy = TRUE;
  }
  
  /*
   * Determine the quantum in internal form.
   */
  if (parms->rt_tqnsecs != RT_TQINF) {
    /*
     * A quantum of zero or nanoseconds >= one second are specifically
     * disallowed.
     */
    if ((parms->rt_tqnsecs != RT_TQINF) &&
	(((parms->rt_tqsecs == 0) &&
	  (parms->rt_tqnsecs == 0)) ||
	 (parms->rt_tqnsecs < 0) ||
	 (parms->rt_tqnsecs >= NSEC_PER_SEC))) {
      register long q = (parms->rt_tqsecs * 1000) + 
	      (parms->rt_tqnsecs + 999999) / 1000000;
      SET_AUDIT_VEC(audvec, t->p_pid, RT_TO_Q(parms->rt_pri), rts, q);
      AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qara0000");
      return EINVAL;
    }
    
    /*
     * The implementation-specific limit is LONG_MAX ticks.  Note that in
     * OSF/1 this is defined in <sys/time.h> and is supposed to be defined
     * in <limits.h>.
     *
     * WARNING:  it is assumed that the value LONG_MAX will fit in
     * an int, which is how quantum is defined.  (This seems like
     * a reasonable assumption...)
     */
    if ((parms->rt_tqsecs > (LONG_MAX / CLOCKS_PER_SEC)) ||
	((parms->rt_tqsecs == (LONG_MAX / CLOCKS_PER_SEC)) &&
	 (parms->rt_tqnsecs > 0))) {
      register long q = (parms->rt_tqsecs * 1000) + 
	      (parms->rt_tqnsecs + 999999) / 1000000;
      SET_AUDIT_VEC(audvec, t->p_pid, RT_TO_Q(parms->rt_pri), rts, q);
      AUDIT_CALL2(u.u_event, ERANGE, audvec, 0, AUD_HPR, "Qara0000");
      return ERANGE;
    }

    /*
     * The routine thread_policy() expects the process's time quantum to
     * be expressed in milliseconds.
     */
    quantum = (parms->rt_tqsecs * 1000) + 
	      (parms->rt_tqnsecs + 999999) / 1000000;
  }
  
  get_thread(t, th);

  /*
   * Time to do the actual calls.
   */
  if (change_policy) {
    register int pol = POLICY_SVID_RT |
	    (parms->rt_tqnsecs == RT_TQINF ? POLICY_FIFO : POLICY_RR);
    if (error=thread_policy(th, pol, quantum)) {
      /*
       * The return value from thread_policy is of type kern_return_t,
       * not UNIX codes.
       */
      error = unix_error(error);
      SET_AUDIT_VEC(audvec, t->p_pid, RT_TO_Q(parms->rt_pri), rts, quantum);
      AUDIT_CALL2(u.u_event, error, audvec, 0, AUD_HPR, "Qara0000");
      return error;
    }
  }
  
  if (may_reschedule) {
    thread_lock(th);
    th->max_priority = RT_TO_Q(parms->rt_pri);
    thread_unlock(th);
    if (error=thread_priority(th,
			      RT_TO_Q(parms->rt_pri),
			      TRUE,
			      may_reschedule)) {
      error = unix_error(error);
      SET_AUDIT_VEC(audvec, t->p_pid, RT_TO_Q(parms->rt_pri), rts, quantum);
      AUDIT_CALL2(u.u_event, error, audvec, 0, AUD_HPR, "Qara0000");
      return error;
    }
  }
  SET_AUDIT_VEC(audvec, t->p_pid, RT_TO_Q(parms->rt_pri), rts, quantum);
  AUDIT_CALL2(u.u_event, 0, audvec, 0, AUD_HPR, "Qara0000");
  return KERN_SUCCESS;
}

static int
rt_set_permissible( p, t, parms )
    struct proc *p, *t;
    void *parms;
{
  int is_su;
  
  /* If we're not currently in the RT class, the caller must have
   * superuser to change.  To change other parameters we must either
   * be superuser or a realtime process with the same real or effective
   * user ID as the target. We assume that the caller has checked for
   * superuser or having the same real or effective user ID as the target.
   */
  if (!(suser(p->p_rcred, 0) == KERN_SUCCESS) &&
      ((!rt_is_class_member(p)) ||
       ((p->p_ruid  != t->p_ruid) &&
	(p->p_ruid  != t->p_svuid) &&
	(p->p_svuid != t->p_ruid) &&
	(p->p_svuid != t->p_svuid))))
    return (EPERM);
  
  return (KERN_SUCCESS);
}

/****************************************************************
 * 'FIXEDPRI', 'POSIX_RR' and 'POSIX_FIFO'scheduling class	*
 * operations							*
 ****************************************************************/

extern int rt_sched_rr_interval;

/* XXX These macros could change!  Check the macro rt_sched_invalid_posix_pri
 * in kern/sched.h
 */
#define POSIX_MAXPRI (NRQS - 1)
#define PSX_TO_Q( p ) ((NRQS - 1) - p)

static int
posix_fifo_is_class_member( p )
    struct proc *p;
{
  thread_t th;

  get_thread(p, th);

  return ((th->policy & (POLICY_SVID_RT | POLICY_FIFO)) == POLICY_FIFO);
}

static int
posix_rr_is_class_member( p )
    struct proc *p;
{
  thread_t th;

  get_thread(p, th);

  return ((th->policy & (POLICY_SVID_RT | POLICY_RR)) == POLICY_RR );
}

static int
posix_priority( th )
    struct thread *th;
{
  /*
   * This code was paraphrased from getpriority().
   */
  return ((NRQS - 1) - ((th->depress_priority < 0)
			? th->priority
			: th->depress_priority));
}

static int 
posix_rr_get_parms(p, buf )
    struct proc *p;
    void *buf;
{
  rtparms_t *rtparms = (rtparms_t *)buf;
  thread_t th;

  get_thread(p, th);

  rtparms->rt_pri = posix_priority(th);
  rtparms->rt_tqsecs = rt_sched_rr_interval / 1000;
  rtparms->rt_tqnsecs = (rt_sched_rr_interval - (rtparms->rt_tqsecs * 1000)) *
			1000000;
  return(KERN_SUCCESS);
}

static int 
posix_fifo_get_parms( p, buf )
    struct proc *p;
    void *buf;
{
  rtparms_t *rtparms = (rtparms_t *)buf;
  thread_t th;

  get_thread(p, th);

  rtparms->rt_pri = posix_priority(th);
  rtparms->rt_tqsecs = 0;
  rtparms->rt_tqnsecs = RT_TQINF;
  
  return (KERN_SUCCESS);
}

int posix_info( buf )
    void *buf;
{
  rtinfo_t *rtinfo = (rtinfo_t *)buf;
  
  rtinfo->rt_maxpri = POSIX_MAXPRI;
  return (KERN_SUCCESS);
}

static int
posix_rr_set_parms(p, t, buf, audvec)
    struct proc *p;
    struct proc *t;
    void *buf;
    audvec_t audvec;
{
  rtparms_t *parms = (rtparms_t *)buf;
  int error, pri;
  thread_t th;

  get_thread(t, th);

  /*
   * POLICY_RR always uses the default RR time interval, contained
   * in the variable rt_sched_rr_interval.  The value in parms is
   * simply ignored.
   */
  if (parms->rt_pri == RT_NOCHANGE) {
	pri = (posix_rr_is_class_member(t) ? posix_priority(th) : PSX_TO_Q(0));
  } else
	pri = PSX_TO_Q(parms->rt_pri);

  /* this routine is defined in the POSIX scheduling code */
  if (rt_sched_invalid_posix_pri(POSIX_RR, pri)) {
        SET_AUDIT_VEC(audvec, t->p_pid, pri, rrs, 0);
        AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
	return(EINVAL);
  }

  if ((error = thread_policy(th, POLICY_RR, rt_sched_rr_interval)) != KERN_SUCCESS) {
    error = unix_error(error);
    SET_AUDIT_VEC(audvec, t->p_pid, pri, rrs, rt_sched_rr_interval);
    AUDIT_CALL2(u.u_event, error, audvec, 0, AUD_HPR, "Qara0000");
    return error;
  }
  if ((error = thread_priority(th, pri, FALSE, FALSE)) != KERN_SUCCESS) {
    error = unix_error(error);
    SET_AUDIT_VEC(audvec, t->p_pid, pri, rrs, rt_sched_rr_interval);
    AUDIT_CALL2(u.u_event, error, audvec, 0, AUD_HPR, "Qara0000");
    return error;
  }

  SET_AUDIT_VEC(audvec, t->p_pid, pri, rrs, rt_sched_rr_interval);
  AUDIT_CALL2(u.u_event, 0, audvec, 0, AUD_HPR, "Qara0000");
  return KERN_SUCCESS;

}

static int
posix_fifo_set_parms(p, t, buf, audvec)
    struct proc *p;
    struct proc *t;
    void *buf;
    audvec_t audvec;
{
  rtparms_t *parms = (rtparms_t *)buf;
  int error, pri;
  thread_t th;

  get_thread(t, th);

  if (parms->rt_pri == RT_NOCHANGE) {
      pri = (posix_fifo_is_class_member(t) ? posix_priority(th) : PSX_TO_Q(0));
  } else
      pri = PSX_TO_Q(parms->rt_pri);

  /* this routine is defined in the POSIX scheduling code */
  if (rt_sched_invalid_posix_pri(POSIX_FIFO, pri)) {
        SET_AUDIT_VEC(audvec, t->p_pid, pri, ffs, 0);
        AUDIT_CALL2(u.u_event, EINVAL, audvec, 0, AUD_HPR, "Qar00000");
	return(EINVAL);
  }

  if ((error = thread_policy(th, POLICY_FIFO, 0)) != KERN_SUCCESS) {
    error = unix_error(error);
    SET_AUDIT_VEC(audvec, t->p_pid, pri, ffs, 0);
    AUDIT_CALL2(u.u_event, error, audvec, 0, AUD_HPR, "Qar00000");
    return error;
  }
  if ((error = thread_priority(th, pri, FALSE, FALSE)) != KERN_SUCCESS) {
    error = unix_error(error);
    SET_AUDIT_VEC(audvec, t->p_pid, pri, ffs, 0);
    AUDIT_CALL2(u.u_event, error, audvec, 0, AUD_HPR, "Qar00000");
    return error;
  }

  SET_AUDIT_VEC(audvec, t->p_pid, pri, ffs, 0);
  AUDIT_CALL2(u.u_event, 0, audvec, 0, AUD_HPR, "Qar00000");
  return KERN_SUCCESS;

}

static int
posix_set_permissible( p, t, parms )
    struct proc *p, *t;
    void *parms;
{
  int is_su;
  
  return ((suser(p->p_rcred, 0) == KERN_SUCCESS)
	  ? KERN_SUCCESS
	  : EPERM);
}

#endif /* RT_SCHED_RT */

static int
fixedpri_is_class_member( p )
    struct proc *p;
{
  thread_t th;

  get_thread(p, th);

  return ((th->policy & POLICY_FIXEDPRI) ==
	  th->policy);
}

static int
empty_info( buf )
    void *buf;
{
  long *b=buf;
  int i;
  
  for (i=0; i<PC_CLINFOSZ; i++)
    b[i]=0;
  
  return (KERN_SUCCESS);
}

static int
inval_get_parms( p, buf )
    struct proc *p;
    void *buf;
{
  return (EINVAL);
}

static int
inval_set_parms(p, t, buf )
    struct proc *p;
    struct proc *t;
    void *buf;
{
    return (EINVAL);
}

static int
inval_set_permissible( p, t, parms )
    struct proc *p, *t;
    void *parms;
{
  return (EINVAL);
}


/*
 *
 * The following routines are used by kernel routines other
 * than (perhaps in addition to) the priocntl() system call.
 */

id_t
class_id( p )
    struct proc *p;
{
  int i;
  
  /* Determine which class this process corresponds to.  If it doesn't match,
   * the set of scheduling classes has been modified without modifying this
   * code and this must be rectified.
   */
  for (i=FIRST_CLASS_ID;
       ((i <= CLASS_MAX) &&
	!((*sched_record[i].is_class_member)( p )));
       i++);
  if (i > CLASS_MAX)
    return(-1);
  return ((id_t)i);
}

int
class_valid( id_t cid )
{
	return(CLASS_VALID(cid));
}

int
is_class_member( struct proc *p, int cid )
{
	return( (*sched_record[cid].is_class_member)( p ) );
}

void
get_priocntl_clname( char *clname, int policy )
{
	switch(policy) {
#if RT_SCHED_RQ
		case POLICY_FIFO:
			strcpy( clname, sched_record[PSX_FF_CLASS].name );
			break;

		case POLICY_RR:
			strcpy( clname, sched_record[PSX_RR_CLASS].name );
			break;

		case POLICY_FIFO|POLICY_SVID_RT:
		case POLICY_RR|POLICY_SVID_RT:
			strcpy( clname, sched_record[RT_CLASS].name );
			break;
#endif	/* RT_SCHED_RQ */

		case POLICY_TIMESHARE:
			strcpy( clname, sched_record[TS_CLASS].name );
			break;

#ifdef	FIXEDPRI_ENABLED
		case POLICY_FIXEDPRI:
			strcpy( clname, sched_record[FIXPRI_CLASS].name );
			break;
#endif	/* FIXEDPRI_ENABLED */

		default:
			strcpy( clname,"unknown" );
			break;
	}
}

