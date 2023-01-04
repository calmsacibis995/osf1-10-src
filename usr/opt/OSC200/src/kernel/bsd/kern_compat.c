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
static char *rcsid = "@(#)$RCSfile: kern_compat.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/04/23 17:20:26 $";
#endif

/*
 * Kernel services related to compatability load modules.
 */

#define STATIC static

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <kern/lock.h>
#include <sys/exec_incl.h>
#include <sys/habitat.h>
#include <kern/kalloc.h>


extern int nproc;
STATIC int cm_validate();

/*
 * These functions provide the interface between the kernel and the 
 * compatability load modules.
 */

struct compat_mod * cm_head_ptr;	/* head of module list	*/
lock_data_t cm_lock;			/* control block lock	*/

struct compat_mod *habitats[MAXHABITATS];



/* 
 * Debugging and tracing
 */
extern int	bin_compat_debug;
extern int	bin_compat_trace;

/*
 * cm_init
 *	Initialize the kernel module interface
 *	Also call the static initialization points of modules that are
 *	statically linked into the kernel.
 */
int
cm_init()
{
int i;

	cm_head_ptr = (struct compat_mod *)0;
	lock_init(&cm_lock, 1);

	/* init the habitat vector */
	bzero(habitats, sizeof(habitats));

	/* loop over all statically linked modules */
	/* the data is in data/cm_data.c */
	for (i = 0; cm_static[i]; i++)
		(* cm_static[i])();
}



/*
 * cm_setup
 *	This is called by a module to initialize its' compat_mod struct
 *	before the call to cm_add. The struct is initialized only when
 *	the cm_name[0] is 0. The passed string args are checked.
 */
int
cm_setup(mod, stanza, stanzalen, name, rev, revision, cfg, rec, syscall,
	call_nam, stats, trace, hab, base, last)
struct compat_mod *mod;
char	*stanza;
int	stanzalen;
char	*name;
char	*rev;
int	revision;
int	(* cfg)();
struct compat_mod *(* rec)();
struct sysent   *(* syscall)();
char	**call_nam;
int	*stats;
char	*trace;
int	hab;
int	base;
int	last;
{
int i;

	if(!mod) {
		printf("cm_setup: compat_mod struct missing\n");
		return(EINVAL);
	}

	/* If name is null go ahead and initialize the module */
	if(mod->cm_name[0] == 0)
		bzero(mod, sizeof(struct compat_mod));
	else
		return(0);

	if(name && name[0]) 
		strncpy(mod->cm_name, name, MAXCOMPATNAMSZ -1);

	if(stanza && stanza[0])
		strncpy(mod->cm_ld_name, stanza, MAXSTANZANAMSZ -1);

	if(rev &&rev[0])
		strncpy(mod->cm_rev, rev, MAXREVSZ -1);

	mod->cm_revision = revision;
	mod->cm_configure = cfg;
	mod->cm_recognizer = rec;
	mod->cm_syscall = syscall;
	mod->call_name		= call_nam;
	mod->cm_habitat		= hab;
	mod->cm_base		= base;
	mod->cm_nsysent		= (last - base + 1);
	mod->cm_stats		= stats;
	mod->cm_trace		= trace;

	/* default trace vector and zero the counters */
	for (i = base; i <= last; i++) {
		trace[i] = bin_compat_trace;
		stats[i] = 0;
	}

	return(0);
}



/*
 * cm_add
 *	This is called by a module's configuration entry point to 
 *	push the module's control block onto the stack. Returns 0
 * 	for O.K. and otherwise the error.
 *
 * NOTE: since each load of a module results in it being at a 
 *	different address, there is no way (at present) to tell
 *	how many copies of the module are active at one time.
 *	(Hopefully the utility software handles this)
 */
int
cm_add(block)
struct compat_mod *block;	/* block to be added */
{
struct cm_valid *cm_pt;
int	ret = 0, hab_indx;

	if(bin_compat_debug) 
		printf("cm_add: 0x%x\n", block);

	if(ret = cm_validate(block)) {
		return(ret);
	}
	if(block->cm_flags & CM_CONFIG)
		return(EINVAL);	/* module is currently configured */

	/* habitats */
	if(block->cm_habitat) {
		hab_indx = (block->cm_habitat>>HABITAT_SHIFT) & HABITAT_LOW;
		if(hab_indx <= 0 || hab_indx >= MAXHABITATS)
			return(EINVAL);
	}

	/* This initializes the ref count (don't trust caller) */
	block->cm_refcount = 0;
	block->cm_totalcount = 0;

	/* default debug flag */
	if(bin_compat_debug)
		block->cm_flags |= CM_DEBUG;
	/* default trace flag */
	if(bin_compat_trace)
		block->cm_flags |= CM_TRACE;

	/* allocate module's auditmask */
	if ( ((char *)block->cm_auditmask = kalloc ( AUDMASK_LEN(block->cm_nsysent) )) == NULL )
		return(ENOMEM);
	bzero ( (char *)block->cm_auditmask, AUDMASK_LEN(block->cm_nsysent) );

	lock_write(&cm_lock);

	if(block->cm_habitat) {
		if(habitats[hab_indx]) {
			lock_done(&cm_lock);
			return(EBUSY);
		}
		habitats[hab_indx] = block;
		block->cm_flags |= CM_CONFIG;
	} else {
		block->cm_next = cm_head_ptr;
		block->cm_flags |= CM_CONFIG;
		cm_head_ptr = block;
	}

	lock_done(&cm_lock);

	if(bin_compat_debug)
		printf("cm_add: complete: cm_head_ptr 0x%x\n", cm_head_ptr);

	return(0);
}



/*
 * cm_del
 *	This is called by a module's configuration entry point when
 *	attempting to un-configure a module. The function returns 0
 *	if successful otherwise it returns the error.
 */
int
cm_del(block)
struct compat_mod *block;	/* block to be added */
{
struct compat_mod *pt, **ptp;

	if(bin_compat_debug)
		printf("cm_del: 0x%x\n", block);

	if( ! block || (block->cm_flags & CM_STATIC))
		return(EINVAL);

	lock_write(&cm_lock);

	/* release module's auditmask */
	if ( block->cm_auditmask ) {
		kfree ( (char *)block->cm_auditmask, AUDMASK_LEN(block->cm_nsysent) );
		block->cm_auditmask = NULL;
	}

#ifdef PHIL
	ASSERT(  block->cm_refcount >= 0 );
#endif
	if (block->cm_habitat) {
		habitats[(block->cm_habitat & HABITAT_MASK)>>HABITAT_SHIFT] = 0;
	} else {
	    if( block->cm_refcount < 0 )
		printf("cm_del: negative refcount %d\n",  block->cm_refcount);

	    /* if busy, return the error */
	    if ( block->cm_refcount > 0 ) {
		lock_done(&cm_lock);
		return (EBUSY);
	    }

	    /* look for the block on the chain */
	    for (pt = cm_head_ptr, ptp = &cm_head_ptr; 
		pt && pt != block; 
		ptp = &pt->cm_next, pt = pt->cm_next)
			;

	    /* if not found return the error */
	    if ( ! pt ) {
		lock_done(&cm_lock);
		return (EINVAL);
	    }

	    /* cut block out of the chain */
	    *ptp = pt->cm_next;
	    pt->cm_next = (struct compat_mod *)0;

	    if(bin_compat_debug){
		printf("cm_del: module chain after delete\n");
		for (pt = cm_head_ptr; pt ; pt = pt->cm_next) 
			printf("\t0x%x\n", pt);
	    }
	}

	lock_done(&cm_lock);

	block->cm_flags &= ~CM_CONFIG;

	return(0);
}





/*
 * cm_newproc
 *	This function is called at process fork when a non-OSF executable
 *	is executing. It is also called if the execve fails and control
 *	is to be returned to the caller. It validates the pointer and 
 *	increments the number of processes currently using the module.
 */
int
cm_newproc(block)
struct compat_mod *block;	/* block to be added */
{
struct compat_mod *ptr = cm_head_ptr;

	/* this process is running an OSF executable */
	if( ! block)
		return;

	lock_write(&cm_lock);

	/* validate the pointer */
	for(; ptr && ptr != block; ptr = ptr->cm_next)
		;
	if(!ptr)
		return;

	/* adjust the counts */
	block->cm_refcount ++;
	block->cm_totalcount ++;

#ifdef PHIL
	ASSERT(  block->cm_refcount >= 0 );
#endif
	if( block->cm_refcount < 0 )
		printf("cm_newproc: negative refcount %d\n",  
			block->cm_refcount);

	lock_done(&cm_lock);
}




/*
 * cm_terminate
 *	This function is called at process termination when the executable
 *	is closed to decrement the reference count. It doesn't return 
 *	anything.
 */
int
cm_terminate(block)
struct compat_mod *block;	/* block to be added */
{
	/* this process is running an OSF executable */
	if( ! block)
		return;

	lock_write(&cm_lock);

	block->cm_refcount --;

#ifdef PHIL
	ASSERT(  block->cm_refcount >= 0 );
#endif
	if( block->cm_refcount < 0 )
		printf("cm_terminate: negative refcount %d\n",
			block->cm_refcount);

	lock_done(&cm_lock);
}



/*
 * cm_recognizer
 *	This function goes down the list of modules looking for a module
 *	that recognizes the current executable. Processing stops when 
 *	a module is found or all of them have been tried. The later 
 *	case will happen when an OSF/1 executable is being run.
 *	The input consists of pointers to the header and auxilliary 
 *	headers of the executable (once execve has decided that it is
 * 	an executable).
 *	The address of the modules control block or 0 (for a miss) is
 *	is set into u.u_compat_mod.
 */
int
cm_recognizer(hdr, aux, vp)
struct filehdr *hdr;
struct aouthdr *aux;
struct vnode   *vp;
{
struct compat_mod *pt, *found = 0;

#if	SYSV_COFF
	lock_write(&cm_lock);

	for (pt = cm_head_ptr; 
	     pt && ! (found = (* pt->cm_recognizer)(hdr, aux, vp)); 
	     pt = pt->cm_next)
		;

	/* adjust the reference counts (increment then decrement
	 * so we don't accidentally go to zero) */
	if(found) {
		found->cm_refcount ++;
		found->cm_totalcount ++;
	}
	if(pt = u.u_compat_mod)
		pt->cm_refcount --;

#ifdef PHIL
	ASSERT( u.u_compat_mod->cm_refcount >= 0 );
#endif
	if(bin_compat_debug){
		if( found && found->cm_refcount)
			printf("cm_recognizer: refcount %d, total %d\n",
				found->cm_refcount, found->cm_totalcount);
	}
	if( pt && pt->cm_refcount < 0 )
		printf("cm_recognizer: negative refcount %d, total %d\n",
			pt->cm_refcount, found->cm_totalcount);
	
	u.u_compat_mod = found;

	lock_done(&cm_lock);
#endif	/* SYSV_COFF */
}


/* 
 * cm_get_struct
 *	Given a pointer to a compat_mod struct, pass back a pointer to
 *	the next struct in the chain and the content of the struct.
 *	At the end of the chain, pass back habitats.
 *	If the supplied ptr is null pass the first block, if any.
 *	Take the lock so things don't change while we are looking around.
 *	If the pointer is invalid return EINVAL otherwise return 0
 */
int
cm_get_struct(ptr, buf)
struct compat_mod **ptr;
struct compat_mod *buf;
{
struct compat_mod *cm_ptr = cm_head_ptr;
struct compat_mod *list[MAXHABITATS*5+1];
int	max = MAXHABITATS*5;	/* a "large" number */
int	i, j;

	bzero (list, sizeof(list));

	/* lock the chain */
	lock_write(&cm_lock);

	/* get a consolidated list of all compat_mod structs that are 
	 * currently in the kernel. The list includes compatability
	 * modules and habitats. The vector size is larger than needed.
	 */
	for (i = 0; cm_ptr && i < (MAXHABITATS*4); cm_ptr=cm_ptr->cm_next, i++)
		list[i] = cm_ptr;
	if(i == MAXHABITATS*4) {
		lock_done(&cm_lock);
		return(EINVAL);
	}
	for (j = 0; j < MAXHABITATS; j++)
		if(habitats[j]) 
			list[i++] = habitats[j];

	if (*ptr) {
		/* validate the pointer */
		for( i = 0; list[i]; i++)
			if(*ptr == list[i])
				break;
		if (!list[i] ) {
			lock_done(&cm_lock);
			return(EINVAL);
		}
		/* on to the next block */
		i++;
	} else
		i = 0;

	/* pass back the desired struct */
	if(list[i])
		*buf = *list[i];
	else
		bzero(buf, sizeof(struct compat_mod));

	/* pass back the new buffer pointer */
	*ptr = list[i];

	lock_done(&cm_lock);
	return (0);
}


/*
 * cm_query
 *	pass back a query struct.
 */
int
cm_query(mod, indata, indatalen, outdata, outdatalen)
struct compat_mod *mod;
int	*indata;
size_t	indatalen;
int	*outdata;
size_t	outdatalen;
{
struct compat_query qu;
int	ret_val = 0, base, last;

	if(!mod || !indata || !outdata || outdatalen != sizeof(qu) || 
	   indatalen != 4 ) {
		return(EINVAL);
	}

	base = mod->cm_base;
	last = mod->cm_nsysent;

	/* the first query passes 0 since it doesn't know where
	 * the vector begins */
	if(*indata < mod->cm_base)
		*indata = mod->cm_base;
	if(*indata > (base + last)) {
printf("cm_query: fail *indata\n", *indata);
		return(EINVAL);
	}

	bzero(&qu, sizeof(qu));
	qu.count = mod->cm_stats[*indata-base];
	qu.svc = *indata;
	strncpy(qu.name, mod->call_name[*indata-base], 32);
	qu.trace = mod->cm_trace[*indata-base];
	for (qu.next = (*indata) +1; 
	    qu.next <= (base+last) && mod->cm_stats[qu.next-base] == 0 ; 
	    qu.next++)
		;

	/* Pass back the data */
	bcopy(&qu, outdata, sizeof(qu));
	return 0;
}



/*
 * cm_operate
 */
int
cm_operate(mod, indata, indatalen, outdata, outdatalen)
struct compat_mod *mod;
int	*indata;
size_t	indatalen;
int	*outdata;
size_t	outdatalen;
{
struct compat_operate *opr;
int	trace_call, i, base, last, ret_val = 0;

#ifndef PHIL
	if( ! mod || ! indata || indatalen != sizeof(struct compat_operate)) {
printf("cm_operate: fail indata 0x%x, indatalen %d\n", indata, indatalen);
		return (EINVAL);
	}

	opr = (struct compat_operate *)indata;
	base = mod->cm_base;
	last = base + mod->cm_nsysent - 1;

	/* When a string is passed in svc[], the CM_TRACE flags
	 * tells whether to turn tracing on or off. In this case
	 * the general trace flag is not changed */
	trace_call = (opr->on_flgs & CM_TRACE);

	if(bin_compat_debug)
	   printf("cm_oprerate, set 0x%x, clear 0x%x, skip %d -%s-\n",
	   opr->on_flgs, opr->off_flgs, opr->skip, (*opr->svc) ? opr->svc : "");

	/* make sure we only deal with the valid flags */
	if(opr->svc[0]) {
		/* we are managing a name */
		opr->on_flgs    &= CM_DEBUG;
		opr->off_flgs	&= CM_DEBUG;
	} else {
		opr->on_flgs    &= (CM_DEBUG | CM_TRACE);
		opr->off_flgs	&= (CM_DEBUG | CM_TRACE);
	}
	if(opr->on_flgs)		/* turn these on */
		mod->cm_flags |= opr->on_flgs;
	if(opr->off_flgs)	/* turn these off */
		mod->cm_flags &= ~opr->off_flgs;

	/* clear trace counters */
	if(opr->skip == -2) {
		bzero(mod->cm_stats, (last - base + 1) * 4);
		mod->cm_nsyscalls = 0;
	}

	/* set skip count */
	if(opr->skip >= 0)
		mod->cm_skipcount = opr->skip;

	/* handle "{no}trace=all" */
	if(opr->svc[0] == 0)
		return(ret_val);
	if(strcmp(opr->svc, "all") == 0) {
		for(i = base; i <= last; i++)
			if(trace_call)
				mod->cm_trace[i-base] = 1;
			else
				mod->cm_trace[i-base] = 0;
	} else {
	    /* handle an individual name */
	    for(i = base; i <= last; i++) {
		if(strcmp(opr->svc, mod->call_name[i-base])==0)
			break;
	    }
	    /* didn't find the name */
	    if(i > last)
		ret_val = EINVAL;
	    else {
		if(trace_call)
			mod->cm_trace[i-base] = 1;
		else
			mod->cm_trace[i-base] = 0;
	    }
	}
#endif
	return (ret_val);
}




/* ------------------ module support functions ------------- */


STATIC char *cm_msgs[] = {
	"compat_mod struct missing",	/*  0 */
	"module name is missing",	/*  1 */
	"revision error",		/*  2 */
	"invalid version",		/*  3 */
	"stanza missing",		/*  4 */
	"missing configure function",	/*  5 */
	"mising recognizer function",	/*  6 */
	"missing syscall function",	/*  7 */
	"missing vector of names",	/*  8 */
	"invalid nsysent",		/*  9 */
	"missing stats vector",		/* 10 */
	"missing trace vector",		/* 11 */
};
#define CM_ERR(x)	{ msg = x ; goto cm_err; }

/*
 * cm_validate
 *	This function validates a compat_mod struct. It returns non-zero
 *	if a problem is discovered.
 */
STATIC
int
cm_validate(mod)
struct compat_mod *mod;
{
int msg;
struct cm_valid *cm_pt;

	if(!mod) 
		CM_ERR(0)
	if(!mod->cm_name || !mod->cm_name[0])
		CM_ERR(1)
	if(!mod->cm_rev || !mod->cm_rev[0] || !mod->cm_revision)
		CM_ERR(2)
	/* module name must be in the list of valid names */
	for (cm_pt = cm_valid; cm_pt->cm_name ; cm_pt++) {
	if ((strcmp(mod->cm_name, cm_pt->cm_name) == 0) &&
	    (mod->cm_revision == cm_pt->cm_rev))
		break;
	}
	if(!cm_pt->cm_name)
		CM_ERR(3)
	if(!mod->cm_ld_name || !mod->cm_ld_name[0])
		CM_ERR(4)
	if(!mod->cm_configure)
		CM_ERR(5)
	if(!mod->cm_habitat && !mod->cm_recognizer)
		CM_ERR(6)
	if(!mod->cm_syscall)
		CM_ERR(7)
	if(!mod->call_name)
		CM_ERR(8)
	if(mod->cm_nsysent <= 0 )
		CM_ERR(9)
	if(!mod->cm_stats)
		CM_ERR(10)
	if(!mod->cm_trace)
		CM_ERR(11)

	return(0);

cm_err:
	printf("cm_validate: %s\n", cm_msgs[msg]);
	return(EINVAL);
}



/*
 * cm_trace_this
 *	Decides whether of not to trace this syscall.
 *	Return 1 to trace otherwise return 0.
 */
int
cm_trace_this(mod)
struct compat_mod *mod;
{
	mod->cm_flags &= ~CM_TRACE_THIS;
	if(mod->cm_flags & CM_TRACE) {
		if(mod->cm_skipcount <= 0) {
			mod->cm_skipcount = 0;
			return (1);
		} else
			mod->cm_skipcount--;
	}

	return(0);
}




/*
 * cm_display
 *	Display each of the fields in the bin_compat struct
 */
int
cm_display(mod)
struct compat_mod *mod;
{
	if(!mod) {
		printf("cm_display: address = 0x0\n");
		return;
	}

	printf("cm_display: address = 0x%x, mod %s, stanza %s, rev:%s 0x%x\n", 
	    mod, mod->cm_name, mod->cm_ld_name, mod->cm_rev, mod->cm_revision);
	printf(
	  "cm_display: config 0x%x, recognizer 0x%x, syscall 0x%x, name 0x%x\n",
	    mod->cm_configure, mod->cm_recognizer, mod->cm_syscall,
	    mod->call_name);
	printf("cm_display: habitat %d, base %d, nsysent %d, nsyscalls %d\n",
	    mod->cm_habitat, mod->cm_base, mod->cm_nsysent,
	    mod->cm_nsyscalls);
	printf(
	    "cm_display: ref %d, total %d, skip %d, flags 0x%x\n",
	    mod->cm_refcount, mod->cm_totalcount, mod->cm_skipcount,
	    mod->cm_flags);
	printf( "cm_display: auditmask 0x%x, stats 0x%x, trace 0x%x\n",
	    mod->cm_auditmask, mod->cm_stats, mod->cm_trace);
}
