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
 * @(#)$RCSfile: vm_anon.h,v $ $Revision: 1.1.13.2 $ (DEC) $Date: 1993/10/06 13:52:21 $
 */
#ifndef __VM_ANON__
#define	__VM_ANON__ 1
#include <sys/unix_defs.h>
#include <kern/lock.h>
#include <vm/vm_debug.h>


struct anon_klock {
	decl_simple_lock_data(, akl_slock)	
	unsigned int
			akl_want  : 8,		/* lock wanted */
			akl_lock  : 1,		/* lock held */
			akl_mlock : 1,		/* mem expansion */
			akl_plock : 1,		/* swap/pageout lock */
				  : 5,
			akl_anon  :16;		/* anon allocated */
	unsigned int
			akl_pages : 16,		/* vpage references to lock */
			akl_rpages: 16;		/* resident pages */
	struct vm_page 	*akl_pagelist;		/* resident page list */
};

typedef	struct anon_klock *alock_t;		/* Anon klock type */

struct vm_anon_object {
	struct vm_object
			ao_object;		/* Object common part */
	unsigned short 	ao_flags;		/* Flags for anon memory */
	int		ao_rbase;		/* Relative base for pages */
	short		ao_crefcnt;		/* Incore reference count */
	unsigned short	ao_rswanon;		/* Reserved swap for anon */
	struct vm_anon	**ao_swanon;		/* Swap anon array */
	vm_size_t	ao_ranon;		/* Reserved anon memory */
	struct vm_object	
			*ao_bobject;		/* Object backed by if any */
	vm_offset_t	ao_boffset;		/* Offset of backing object */
	struct vm_anon	**ao_anon;		/* Anon pointer array */
	unsigned short	ao_nklock;		/* Number of klocks */
	alock_t 	ao_klock;		/* Anon kluster locking */
};

typedef	struct vm_anon_object *vm_anon_object_t;

struct vm_shm_object {
        struct vm_anon_object
                        so_anon_object;         /* Anon portion of the SV shm object */
        struct shmid_internal   *so_sp;         /* Shared memory internal structure */
	vm_offset_t	so_attach;		/* pointer to the attach array */
};

typedef struct vm_shm_object *vm_shm_object_t;

struct	vm_shm_attach {
	int		at_count;		/* Number of times attached to process */
	struct vm_map	*at_map;		/* The vm_map that we're attached to */
	pid_t		at_pid;			/* The pid of the attacher */
};

typedef struct vm_shm_attach *vm_shm_attach_t;


#define	ao_size		ao_object.ob_size
#define	ao_refcnt	ao_object.ob_ref_count
#define ao_rescnt       ao_object.ob_res_count
#define ao_oflags	ao_object.ob_flags

#define so_refcnt	so_anon_object.ao_object.ob_ref_count
#define so_rescnt       so_anon_object.ao_object.ob_res_count
#define so_crefcnt	so_anon_object.ao_crefcnt
#define so_flag		so_anon_object.ao_flags
#define so_anon		so_anon_object.ao_anon
#define so_size		so_anon_object.ao_object.ob_size
#define so_nklock	so_anon_object.ao_nklock
#define so_klock	so_anon_object.ao_klock
#define so_bobject	so_anon_object.ao_bobject
#define so_ranon	so_anon_object.ao_ranon
#define so_oflags	so_anon_object.ao_object.ob_flags

/*
 * anon object flags
 */

#define	AF_PRIVATE	0x01			/* private anon */
#define	AF_SHARED	0x02			/* shared anon */
#define	AF_NOGROW	0x04			/* can't grow anon */
#define	AF_SWAPPED	0x08			/* swapped out */


/*
 * An anon object is protected by an array of locks whose granularity
 * is determined at boot time by the tuning parameter anonklshift.
 * The lock is computed the following way:
 *
 *	(object offset) >> klshift = lock index
 *
 */ 


struct vm_anon {
	union {
		struct vm_page *_an_page;	/* Page hint for anon */
		struct vm_anon *_an_next;	/* When on free list or hash */
	} _uanonx;
	union {
		struct {
			unsigned int 
			_an_refcnt:24,		/* Anon references */
			_an_cowfaults:5,	/* Runtime hueristic */
			_an_hasswap:1,		/* Has swap list */
			_an_type:2;		/* Type of anon */
		} _an_bits0;
		struct {
			unsigned int
			_an_anon:30,		/* Lazy anon */
			_an_type1:2;		/* Same position as above */
		} _an_bits1;
	} _uanony;
};

#define an_page 	_uanonx._an_page
#define an_next 	_uanonx._an_next
#define an_refcnt	_uanony._an_bits0._an_refcnt
#define an_cowfaults	_uanony._an_bits0._an_cowfaults
#define an_type		_uanony._an_bits0._an_type
#define an_hasswap	_uanony._an_bits0._an_hasswap
#define	an_anon		_uanony._an_bits1._an_anon

#define ANT_SWAP	0x0			/* Swap anon cell */
#define ANT_LAZY	0x1			/* Lazy anon cell */
#define	ANT_LAZYSWAP	0x2			/* Lazy swap cell */
#define	ANT_XLATESWAP	0x3			/* Translated swap */

#define	ANON_COWMAX	0x1f

/*
 * The anon structure size can impact the locking
 * hash constant defined below.  A constant is
 * is used for performance reasons instead of computing
 * the value.
 */

#define	A_LSHIFT	0x4			/* 16 is size maximum */

/*
 * For anon swap hashing
 */

struct vm_anon_swaphash {
	udecl_simple_lock_data(, ah_lock)
	struct vm_anon *ah_next;
};

#define	A_SWAPHASH_SIZE	64

#define	a_hash(AP)						\
	(anon_swaphash + ((((vm_offset_t) (AP)) >> A_LSHIFT) & hanon_mask))

/*
 * On MP machines concurrent anon updates are serialized
 * by a spin lock.  Having a lock for each anon structure
 * would be very wasteful.  Instead the anon virtual address
 * is used in a hash function to compute an index into a
 * anon simple lock array.
 */

#define	ANON_NLANON		128		/* 128 anon locks */

#if	UNIX_LOCKS
extern simple_lock_t anon_lanon;
extern int anon_nlanon;
vm_offset_t anon_lanon_mask;


#define	a_lockaddr(AP)						\
		(anon_lanon +					\
		((((vm_offset_t) (AP)) >> A_LSHIFT) 		\
		& anon_lanon_mask))
	
#define	a_lock(AP)		usimple_lock(a_lockaddr(AP))

#define a_lock_try(AP)		usimple_lock_try(a_lockaddr(AP))

#define a_unlock(AP)		usimple_unlock(a_lockaddr(AP))

#define a_locklp(ALP)		usimple_lock((ALP))

#define a_unlocklp(ALP)		usimple_unlock((ALP))

#define	a_access(AP, AOP) {					\
	register simple_lock_t AL;				\
	AL = a_lockaddr(AP);					\
	usimple_lock(AL);					\
	AOP;							\
	simple_unlock(AL);					\
}

#else	/* UNIX_LOCKS */

#define	a_access(AP, AOP) AOP
#define	a_lockaddr(AP) (simple_lock_t) 0
#define a_lock(AP)
#define a_lock_try(AP)	1
#define a_unlock(AP)
#define a_locklp(ALP)
#define a_unlocklp(ALP)

#endif	/* UNIX_LOCKS */


extern int anon_klshift, anon_klpages, anon_klpagesize;
extern vm_offset_t anon_klsize;
extern int anon_pagesinkl;

#define	anon_klround(KOFF)					\
	(((vm_offset_t)(KOFF) + (anon_klsize - 1)) & ~(anon_klsize - 1))
#define anon_kltrunc(KOFF)					\
	(((vm_offset_t)(KOFF) & ~(anon_klsize - 1)))
#define anon_kl(KOFF)	((KOFF) >> anon_klshift)


#define lk_lock_init(LP) {						\
		simple_lock_init(&(LP)->akl_slock);			\
		(LP)->akl_want = 0;					\
		(LP)->akl_lock = 0;					\
		(LP)->akl_mlock = 0;					\
		(LP)->akl_plock = 0;					\
}	

#define	lk_slock(LP)	simple_lock(&(LP)->akl_slock)
#define	lk_sunlock(LP)	simple_unlock(&(LP)->akl_slock)

/*
 * Memory lock on LK
 */

#define lk_mlock(LP) {							\
		lk_slock((LP));						\
		while ((LP)->akl_lock) {				\
			assert_wait((vm_offset_t)(LP), FALSE);		\
			(LP)->akl_want++;				\
			lk_sunlock((LP));				\
			thread_block();					\
			lk_slock((LP));					\
		}							\
		(LP)->akl_lock = 1;					\
		(LP)->akl_mlock = 1;					\
		lk_sunlock((LP));					\
}

/*
 * Memory or paging unlock
 */

#define lk_mpunlock(LP,L,H) {						\
		lk_slock((LP));						\
		if (!(LP)->akl_/**/H/**/lock) {				\
			if  ((LP)->akl_want) {				\
				thread_wakeup_one((vm_offset_t) (LP));	\
				(LP)->akl_want--;			\
			}						\
			(LP)->akl_lock = 0;				\
		}							\
		(LP)->akl_/**/L/**/lock = 0;				\
		lk_sunlock((LP));					\
}

/*
 * Release the paging or memory lock
 * whe the spin lock already held.
 */

#define lk_hmpunlock(LP,L,H) {						\
		if (!(LP)->akl_/**/H/**/lock) {				\
			if  ((LP)->akl_want) {				\
				thread_wakeup_one((vm_offset_t) (LP));	\
				(LP)->akl_want--;			\
			}						\
			(LP)->akl_lock = 0;				\
		}							\
		(LP)->akl_/**/L/**/lock = 0;				\
		lk_sunlock((LP));					\
}

#define lk_lock(LP) {							\
		lk_slock((LP));						\
		while ((LP)->akl_lock) {				\
			assert_wait((vm_offset_t)(LP), FALSE);		\
			(LP)->akl_want++;				\
			lk_sunlock((LP));				\
			thread_block();					\
			lk_slock((LP));					\
		}							\
		(LP)->akl_lock = 1;					\
		lk_sunlock((LP));					\
}

#define lk_unlock(LP) {							\
		lk_slock((LP));						\
		if ((LP)->akl_want) {					\
			thread_wakeup_one((vm_offset_t) (LP));		\
			(LP)->akl_want--;				\
		}							\
		(LP)->akl_lock = 0;					\
		lk_sunlock((LP));					\
}

extern boolean_t lk_lock_try(alock_t lp);

#define	ANON_KLSHIFT	(16)

#define	ANON_KLPAGES	16

#ifdef	KERNEL

#if	VM_ANON_TRACK

struct an_track {
	char	at_written;
	short	at_writes;
	short	at_reads;
	long	at_writepc;
	long	at_readpc;
};

#define	APTOAT(AP) {							\
	register struct vm_swap *SP;					\
	vm_offset_t SOFFSET;						\
	SP = a_aptosp((AP), &SOFFSET);					\
	((struct an_track *) 						\
	(SP->vs_anbase + SP->vs_swapsize)) + 				\
	((AP) - SP->vs_anbase)						\
}

#define	AN_TRACK_WRITTEN(AP)	an_track_written((AP))
#define	AN_TRACK_WRITE(AP)	an_track_write((AP))
#define	AN_TRACK_READ(AP)	an_track_read((AP))
#define	AN_TRACK_FREE(AP)	an_track_free((AP))

#else

#define	AN_TRACK_WRITTEN(AP)
#define	AN_TRACK_WRITE(AP)
#define	AN_TRACK_READ(AP)
#define	AN_TRACK_FREE(AP)

#endif	/* VM_ANON_TRACK */

/*
 * Return ap which owns page from ap computed by
 * page object's swap structure and page offset ap.
 */

#define	a_saptoap(AP)						\
	(((AP)->an_type == ANT_LAZYSWAP) ?			\
	(vm_swap_lazy->vs_anbase + (AP)->an_anon) : (AP))

extern struct vm_swap *a_aptosp(/* struct vm_anon *ap,
	 vm_offset_t *soffset */);
struct vm_anon * a_swap_xlate(/* register struct vm_anon *ap,
	register struct vm_anon **app */);
extern struct vm_anon *a_swap_hash_free(/* struct vm_anon * ap */);
extern struct vm_anon *a_anon_allocate();
extern struct vm_anon *a_swap_alloc(/* boolean_t canwait,
	boolean_t reserved, int lazyswap */);
extern boolean_t a_swap_lazy_alloc(/* struct vm_anon *ap,
	struct vm_page *pp, boolean_t canwait */);
extern struct vm_anon *a_swap_hash_lookup(/* struct vm_anon *ap */);
extern boolean_t a_reserve(/* vm_anon_object_t op, vm_size_t size */);
extern void a_free(/* vm_anon_object_t op, vm_size_t size */);

extern struct vm_anon *a_anon_appage_alloc(/* struct vm_anon_object *aop, 
	vm_offset_t offset, alock_t lp, struct vm_anon **app */);
extern struct vm_anon *a_anon_cowpage_alloc(/* 
	register struct vm_anon_object *aop,
	register vm_offset_t offset, alock_t lp,
	register struct vm_anon *cap,
	struct vm_anon **app */);
extern struct vm_anon *a_anon_pagezero_alloc(/*
	struct vm_anon_object *aop, 
	vm_offset_t offset, 
	alock_t lp */);

#endif	/* KERNEL */
#endif /* !__VM_ANON */
