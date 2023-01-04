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
static char *rcsid = "@(#)$RCSfile: kern_malloc.c,v $ $Revision: 1.1.3.11 $ (DEC) $Date: 1993/11/11 20:43:56 $";
#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base: kern_malloc.c	7.18 (Berkeley) 6/28/90
 */

#define KMEMTYPES 1
/*#define KMEMSTATS 1*/
/*#define MACH_ASSERT 1*/

#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/unix_defs.h>	/* for VAGUE_STATS. MACH_ASSERT, etc. */
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <machine/cpu.h>
#include <sys/malloc.h>
#include <sys/memlog.h>

/*
 * Reference:
 *   "Design of a General Purpose Memory Allocator for the 4.3BSD UNIX Kernel"
 *	Summer Usenix '88 proceedings pp 295-301.
 *
 * OSF enhanced to include
 *	Parallelization
 *	Asynchronous allocation and freeing of memory buckets
 *	Borrowing of memory for small allocations from larger buckets
 *	Garbage collection and freeing of "extra" memory
 *	Tunable
 */

#ifdef  KMEMTYPES
struct kmemtypes kmemtypes[M_LAST];
const char kmemnames[M_LAST][KMEMNAMSZ] = INITKMEMNAMES;
#endif

#define RESERVE		((kmemreserve * PAGE_SIZE) >> maxallocshift)

struct kmembuckets bucket[MINBUCKET + 16];
struct kmemusage *kmemusage;

void   *kmembase;			/* min va for kmem submap */
long   kmemmap_blocked;			/* # of times that malloc blocked */

static vm_map_t kmemmap;		/* kmem kernel submap */
static thread_t kmemthread;		/* kmem kernel thread */
static struct kmembuckets *mkbp;	/* pointer to MAXALLOCSAVE bucket */
static struct kmembuckets *pkbp;	/* pointer to page sized bucket */
static int maxindx;			/* index to MAXALLOCSAVE bucket */
static int pagindx;			/* index to page sized bucket */
static int wantkmemmap;			/* event that malloc blocks on */ 
static void *kmemlimit;			/* max va for kmem submap */
static void *kmemfreelater;		/* blocks to be freed by kmem thread */ 

static int kmeminitialized=0;		/* initialization flag */

/*
 * Tuning parameters.
 *	kmempages	Size (in pages) of malloc map. Configurable only
 *			at startup, unused after.
 *	kmemreserve	The malloc reserve is some number of pages which
 *			are held as some number of elements in the largest
 *			bucket. This must not be larger than the bucket's
 *			highwater. This is verified at startup, but not if
 *			changed later.
 *	kmemgcintvl	Frequency in seconds of "normal" garbage collection
 *			passes. Settable to 0 to disable all gc's.
 *	kmemgcscale	Scaling factor for gc aggressiveness. Any overage
 *			detected in gc is reduced linearly by this slope
 *			until it reaches 0, then constantly until done.
 */
int kmempages = 0;
int kmemreserve = 32;
int kmemgcintvl = 2;
int kmemgcscale = 8;
int kmemgcshift = 3;
int maxallocshift = 0;

#define	NPRIME	32	/*# of MAXALLOCSAVE chunks to prime the allocator*/
#define HIWAT1	32	/*high water mark for buckets with one element/cell*/
#define HIWAT2	32	/*high water mark for buckets with > one element/cell*/
#define TLIMIT 	6/10	/*limit on each type - fraction of kmempages */

extern int cold;

#define GET_CALLER(caller) \
	void *caller = (void *)asm("lda %v0,-4(%ra)")

/*
 * Allocate a block of memory
 */
void *
malloc(	u_long	size,
	struct kmembuckets *kbp,
	int	type,
	int	flags)
{
	register struct kmemusage *kup;
	int s;
	void *va;
#ifdef KMEMTYPES
        register struct kmemtypes *ktp = &kmemtypes[type];
#endif

#ifdef MEMLOG
	if(memlog) {
 		GET_CALLER(caller);
		memory_log(MALLOC_LOG, caller, size);
	}
#endif

#if MACH_ASSERT
	if (kbp != &bucket[BUCKETINDX(size)]) 
		panic("malloc: incorrect bucket pointer");
#ifdef KMEMTYPES
        if (type <= 0 || type >= M_LAST)
                panic("malloc: bogus type");
#endif /* KMEMTYPES */
#endif /* MACH_ASSERT */

#if     MACH_ASSERT
        if (!cold) {
	    GET_CALLER(caller);
	    s = getspl();
	    if (s > SPLDEVHIGH)
		printf("malloc: called at spl level>spldevhigh, ra=0x%lx\n", caller);
                /*return (0);*/

            if ((flags & M_WAITOK) && (s != SPLNONE)) {
		printf("malloc: called with M_WAITOK at elevated spl, ra=0x%lx\n", caller);
                return (0);
	    }
	}
#endif /* MACH_ASSERT */

        s = spldevhigh();
again:
#ifdef KMEMTYPES
        simple_lock(&ktp->kt_lock);
        if (ktp->kt_memuse >= ktp->kt_limit) {
                if (flags & M_NOWAIT) {
#ifdef KMEMSTATS
                        ktp->kt_failed++;
#endif 
                        simple_unlock(&ktp->kt_lock);
                        splx(s);
                        return (0);
                }
#ifdef KMEMSTATS
                ktp->kt_limblocks++;
#endif 
                assert_wait((vm_offset_t)ktp, FALSE);
                ktp->kt_wait = 1;
                simple_unlock(&ktp->kt_lock);
                thread_block();
                goto again;
        }
        simple_unlock(&ktp->kt_lock);
#endif /* KMEMTYPES */


	simple_lock(&kbp->kb_lock);
	if ((va = kbp->kb_next) == NULL) {   
		u_long allocsize;
		void *cp, *savedlist, *malloc_loan(u_long, int);
#ifdef KMEMSTATS
		kbp->kb_noelem++;
#endif
		simple_unlock(&kbp->kb_lock);
		if (size <= MAXALLOCSAVE) {
			allocsize = kbp->kb_size;
			size = round_page(allocsize);
			va = malloc_loan(size, flags);
#ifdef KMEMSTATS
			if (va)
				kbp->kb_borrowed++;
#endif /* KMEMSTATS */
		}
		else 
			allocsize = size = round_page(size);
		if (va == NULL) {
			if (flags & M_NOWAIT){ 
#ifdef KMEMSTATS
				simple_lock(&kbp->kb_lock);
				kbp->kb_calls++;
				kbp->kb_failed++;
				simple_unlock(&kbp->kb_lock);
#endif /* KMEMSTATS */
				splx(s);
				return 0;    /*** return from malloc ***/
			}
			splx(s);
			va = (char *)kmem_alloc(kmemmap, size);
			s = spldevhigh();
			if (va == NULL) {
				simple_lock(&bucket[0].kb_lock);
#ifdef KMEMSTATS
				kmemmap_blocked++;
#endif
				wantkmemmap = 1;
				assert_wait((vm_offset_t)&wantkmemmap, FALSE); 
				simple_unlock(&bucket[0].kb_lock);
				thread_block();
				goto again;
			}
		}
		kup = btokup(va);
		kup->ku_kbp = kbp;
		simple_lock(&kbp->kb_lock);
		if (kbp->kb_elmpercl == 1) { 	
			kup->ku_size = size;  
			kbp->kb_total++;
#ifdef KMEMSTATS
			kbp->kb_calls++;
#endif /* KMEMSTATS */
			simple_unlock(&kbp->kb_lock);
#ifdef KMEMTYPES
                        simple_lock(&ktp->kt_lock);
                        ktp->kt_memuse += allocsize;
        		if (ktp->kt_memuse > ktp->kt_maxused)
                		ktp->kt_maxused = ktp->kt_memuse;
                        simple_unlock(&ktp->kt_lock);
#endif /* KMEMTYPES */
			splx(s);
			return va;		/*** return from malloc ***/
		}
		kup->ku_freecnt = kbp->kb_elmpercl;
		kbp->kb_totalfree += kbp->kb_elmpercl;
		kbp->kb_total += kbp->kb_elmpercl;
		/*
		 * Just in case we blocked while allocating memory,
		 * and someone else also allocated memory for this
		 * bucket, don't assume the list is still empty.
		 */
		savedlist = kbp->kb_next;
		kbp->kb_next = (char *)va + size - allocsize;
		for (cp = kbp->kb_next; cp > va; cp = (char *)cp - allocsize) {
			*(void **)cp = (char *)cp - allocsize;
		}
		*(void **)cp = savedlist;
		va = kbp->kb_next;
	}
	kbp->kb_next = *(void **)va;
	kbp->kb_totalfree--;
	kup = btokup(va);
	kup->ku_freecnt--;
#ifdef KMEMSTATS
	kbp->kb_calls++;
#endif /* KMEMSTATS */
	simple_unlock(&kbp->kb_lock);

#ifdef KMEMTYPES
	simple_lock(&ktp->kt_lock);
	ktp->kt_memuse += kbp->kb_size;
        if (ktp->kt_memuse > ktp->kt_maxused)
                ktp->kt_maxused = ktp->kt_memuse;
	simple_unlock(&ktp->kt_lock);
#endif /* KMEMTYPES */

	splx(s);
	return va;				/*** return from malloc ***/
}

#if	MACH_ASSERT
static const u_long addrmask[] = { 0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000f,
	0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
	0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
	0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff
};
#endif

/*
 * Free a block of memory allocated by malloc.
 */
void
free(void *addr,
     int   type
)
{
	register struct kmembuckets *kbp;
	register struct kmemusage *kup;
	int s;
        u_long size;
#if	MACH_ASSERT
        long alloc, indx;
#endif /* MACH_ASSERT */
#ifdef KMEMTYPES
        register struct kmemtypes *ktp = &kmemtypes[type];
#endif /* KMEMTYPES */

#if MACH_ASSERT
	if (addr < kmembase || addr >= kmemlimit)
		panic("free: bad addr");
#if KMEMTYPES
        if (type <= 0 || type >= M_LAST)
                panic("free: bogus type");
#endif /* KMEMTYPES */
#endif /* MACH_ASSERT */

	kup = btokup(addr);
	kbp = kup->ku_kbp;

#ifdef MEMLOG
	if(memlog){
 		GET_CALLER(caller);
		memory_log(FREE_LOG, caller, kbp->kb_size);
	}
#endif

#if	MACH_ASSERT
	indx = kbp->kb_indx;
	if (indx < MINBUCKET || indx >= MINBUCKET+16)
		panic("free: bad indx");
	if (indx >= pagindx)
		alloc = addrmask[pagindx];
	else if (indx < sizeof addrmask / sizeof addrmask[0])
		alloc = addrmask[indx];
	else
		panic("free: humungous PAGE_SIZE");
	if (((u_long)addr & alloc) != 0) {
		printf("free: unaligned addr 0x%x, mask %x\n",
			addr, alloc);
		panic("free: unaligned addr");
	}
#endif
	if (kbp->kb_elmpercl == 1){
		if (kbp->kb_size <= MAXALLOCSAVE) {
			s = spldevhigh();
			simple_lock(&kbp->kb_lock);
			*(void **)addr = kbp->kb_next;
			kbp->kb_next = addr;
			kbp->kb_totalfree++;
			simple_unlock(&kbp->kb_lock);
			if (wantkmemmap) {
				simple_lock(&bucket[0].kb_lock);
				wantkmemmap = 0;
				simple_unlock(&bucket[0].kb_lock);
				thread_wakeup((vm_offset_t)&wantkmemmap);
			}
			goto out;
		} else {
			/*
			 * Cannot call kmem_free in interrupt context,
			 * so play it safe and let kmemthread do it.
			 */
			*(int *)((void **)addr + 1) = kup->ku_size;
			s = spldevhigh();
			simple_lock(&bucket[0].kb_lock);
			*(void **)addr = kmemfreelater;
			kmemfreelater = addr;
			simple_unlock(&bucket[0].kb_lock);
			if (kmemthread)
				clear_wait(kmemthread, THREAD_AWAKENED, FALSE);
			goto out;
		}
	}
	s = spldevhigh();
	simple_lock(&kbp->kb_lock);
	*(void **)addr = kbp->kb_next;
	kbp->kb_next = addr;
	kbp->kb_totalfree++;
	kup->ku_freecnt++;
	if ((kup->ku_freecnt >= kbp->kb_elmpercl) && 
	   (kbp->kb_totalfree > kbp->kb_highwat)) { 
		register void *q, *p = kbp->kb_next;
		register void **pp = &kbp->kb_next;
		register int  n = kbp->kb_elmpercl;
#if	MACH_ASSERT
		if (kup->ku_freecnt > kbp->kb_elmpercl)
			panic("free: multiple frees");
#endif
		/*
		 * Bump small, free buckets up to pages.
		 */
		do {
			q = *(void **)p;
			if (btokup(p) == kup) { 
				*pp = q;
				n--;
			}
			else
				pp = (void **)p;
		} while ((p = q) && n );
#ifdef KMEMSTATS
		kbp->kb_couldfree++;
#endif /* KMEMSTATS */
		kbp->kb_total -= kbp->kb_elmpercl;
		kbp->kb_totalfree -= kbp->kb_elmpercl;
		kup->ku_kbp = pkbp;
		simple_unlock(&kbp->kb_lock);
		/* Add page to page-size bucket */
		addr = (void *) trunc_page(addr);
		simple_lock(&pkbp->kb_lock);
		*(void **)addr = pkbp->kb_next;
		pkbp->kb_next = addr;
		pkbp->kb_total++;
		pkbp->kb_totalfree++;
		simple_unlock(&pkbp->kb_lock);
	} else 
		simple_unlock(&kbp->kb_lock);
	if (wantkmemmap) {
		simple_lock(&bucket[0].kb_lock);
		wantkmemmap = 0;
		simple_unlock(&bucket[0].kb_lock);
		thread_wakeup((vm_offset_t)&wantkmemmap);
	}

out:
#ifdef KMEMTYPES
	simple_lock(&ktp->kt_lock);
	ktp->kt_memuse -= kbp->kb_size;
       	if (ktp->kt_wait) {
		ktp->kt_wait = 0;
       		simple_unlock(&ktp->kt_lock);
       		thread_wakeup((vm_offset_t)ktp);
	}
	else
       		simple_unlock(&ktp->kt_lock);
#endif /* KMEMTYPES */
	splx(s);
}

/*
 * When smaller bucket is empty, borrow from larger.
 * Called at splimp, with size rounded to page. If
 * possible, don't loan out the reserve to allocations
 * which can wait.
 */
static void *
malloc_loan(
	u_long	size,
	int	flags)
{
	register struct kmembuckets *kbp;
	register struct kmemusage *kup;
	register long newsize;
	void *va = 0;

	kbp = &bucket[BUCKETINDX(size)];
	for (newsize = size; newsize <= MAXALLOCSAVE; newsize <<= 1) {
		simple_lock(&kbp->kb_lock);
		if (!(flags & M_NOWAIT) && newsize >= MAXALLOCSAVE &&
		     kbp->kb_totalfree <= RESERVE) {
			simple_unlock(&kbp->kb_lock);
			return 0;
		}
		if (va = kbp->kb_next) {
			kbp->kb_next = *(void **)va;
			kbp->kb_total--;
			kbp->kb_totalfree--;
		}
		simple_unlock(&kbp->kb_lock);
		if (va) {
#ifdef KMEMSTATS
			kbp->kb_lent++;
#endif /* KMEMSTATS */
			break;
		}
		++kbp;
	}
	if (newsize >= MAXALLOCSAVE && kmemthread)
		clear_wait(kmemthread, THREAD_AWAKENED, FALSE);

	if (va && newsize != size) {
		void *nva = (char *)va + size;
		/* For simplicity, toss on as pages */
		kbp = pkbp;
		simple_lock(&kbp->kb_lock);
		do {
			kup = btokup(nva);
			kup->ku_kbp = kbp;
			kbp->kb_total++;
			kbp->kb_totalfree++;
			*(void **)nva = kbp->kb_next;
			kbp->kb_next = nva;
			newsize -= PAGE_SIZE;
			nva = (char *)nva + PAGE_SIZE;
		} while (newsize != size);
		simple_unlock(&kbp->kb_lock);
	}
	return va;
}

/*
 * Service thread for malloc/free.
 */

static void
malloc_thread(void)
{
	struct kmembuckets *kbp;
	register struct kmemusage *kup;
	struct timeval now, lastgc;
	vm_offset_t addr;
	long indx;
	void *va;
	int s, i, n, tmo = 0;

	current_thread()->vm_privilege = FALSE;
	spl0();
	microtime(&lastgc);
	for (;;) {
		if (tmo) {
			assert_wait((vm_offset_t)0, FALSE);
			if (tmo > 0) thread_set_timeout(tmo);
			thread_block();
		}
		/*
		 * Zeroth job: perform delayed frees of large allocations.
		 */
		if (kmemfreelater) { 
			s = spldevhigh();
			simple_lock(&bucket[0].kb_lock);
			va = kmemfreelater;
			kmemfreelater = 0;
			if (va && (i = wantkmemmap))
				wantkmemmap = 0;
			else
				i = 0;
			simple_unlock(&bucket[0].kb_lock);
			splx(s);
			while (va) {
				addr = (vm_offset_t)va;
				indx = *(int *)((void **)va + 1); /* size */
				va = *(void **)va;
				kmem_free(kmemmap, addr, indx);
			}
			if (i)
				thread_wakeup((vm_offset_t)&wantkmemmap);
		}
		/*
		 * First job: minimize probability of malloc(M_NOWAIT)
		 * returning NULL by keeping its biggest bucket full.
		 * Note if MAXALLOCSAVE is much greater than PAGE_SIZE,
		 * fragmentation may result, there is a limit enforced
		 * in kmeminit() below which attempts to minimize this.
		 */
		i = RESERVE;		/* target */
		s = spldevhigh();
		simple_lock(&mkbp->kb_lock);
		i -= mkbp->kb_totalfree;
		simple_unlock(&mkbp->kb_lock);
		splx(s);
		if (i > 0) {
			if (va = (char *) kmem_alloc(kmemmap, MAXALLOCSAVE)) {
				kup = btokup(va);
				kup->ku_kbp = mkbp;
				s = spldevhigh();
				simple_lock(&mkbp->kb_lock);
				mkbp->kb_total++;
				mkbp->kb_totalfree++;
				*(void **)va = mkbp->kb_next;
				mkbp->kb_next = va;
				simple_unlock(&mkbp->kb_lock);
				simple_lock(&bucket[0].kb_lock);
				i = wantkmemmap;
				wantkmemmap = 0;
				simple_unlock(&bucket[0].kb_lock);
				splx(s);
				if (i)
					thread_wakeup((vm_offset_t)&wantkmemmap);
				tmo = 0;	/* go around now */
				continue;
			}
			/*
			 * If alloc fails, force a gc pass and go around
			 * in 1/2 sec if we fail to free anything.
			 */
			tmo = hz >> 1;
		} else
			tmo = -1;
		/*
		 * If timeout is -1, then check interval since last gc and
		 *   gc or go back to sleep. Check for time warps.
		 * If timeout < hz, then failure dictates some gc is
		 *   necessary before retry.
		 */
		microtime(&now);
		if (tmo < 0) {
			if (kmemgcintvl <= 0)
				continue;
			tmo = kmemgcintvl - (now.tv_sec - lastgc.tv_sec);
			if (tmo > 0) {
				if (tmo > kmemgcintvl)	/* timewarp */
					tmo = kmemgcintvl;
				tmo *= hz;
				continue;
			}
			tmo = kmemgcintvl * hz;
		}
		lastgc = now;
		/*
		 * Second job: garbage collect pages scavenged in free().
		 * Scale aggressiveness in freeing memory to the amount
		 * of overage and settable scale. Currently functional
		 * only if KMEMSTATS.
		 */
		indx = maxindx;
		kbp = mkbp;
		n = 0;
		s = spldevhigh();
		do {
			simple_lock(&kbp->kb_lock);
			i = (kbp->kb_totalfree - kbp->kb_highwat) *
				(1 << (indx - pagindx));
			if (i > 0)
				n += i;
			simple_unlock(&kbp->kb_lock);
			kbp--;
			indx--;
		} while (indx >= pagindx);
		splx(s);
		/*
		 * Do linear scaling above "scale" extra pages,
		 * constant scaling below "scale", nothing if none.
		 * Note smaller scale = higher aggressiveness.
		 * Typical value == 8 @ 2 sec gcintvl, yielding
		 * recovery from 50 page overage in ~1 minute.
		 * 100 big ethernet packets might cause this, e.g.
		 */
		if (n > 0 && (n >>= kmemgcshift) == 0)
			n = 1;
		++indx;
		++kbp;
		while (n > 0 && indx <= maxindx) {
			va = 0;
			s = spldevhigh();
			simple_lock(&kbp->kb_lock);
			if (kbp->kb_totalfree > kbp->kb_highwat) {
				va = kbp->kb_next;
				kbp->kb_next = *(void **)va;
#ifdef KMEMSTATS
				kbp->kb_couldfree++;
#endif /* KMEMSTATS */
				kbp->kb_total--;
				kbp->kb_totalfree--;
			}
			simple_unlock(&kbp->kb_lock);
			splx(s);
			if (va) {
				n -= 1 << (indx - pagindx);	/* page count */
				if (tmo < hz)			/* want retry */
					tmo = 0;
				i = 1 << indx;
				kmem_free(kmemmap, (vm_offset_t)va, i);
			} else {
				kbp++;
				indx++;
			}
		}
	}
	/*NOTREACHED*/
}


/*
 * Set limit for type.
 */
int
kmemsetlimit(
        int     type,
        long    limit)
{
#ifdef KMEMTYPES
        int s;
        register struct kmemtypes *ktp = &kmemtypes[type];

        if (type > 0 && type < M_LAST) {
                if (limit <= 0)
                        limit = LONG_MAX;
                s = spldevhigh();
                simple_lock(&ktp->kt_lock);
                ktp->kt_limit = limit;
                simple_unlock(&ktp->kt_lock);
                splx(s);
                thread_wakeup((vm_offset_t)ktp);
                return 1;
        }
#endif /* KMEMTYPES */
        return 0;
}


/*
 * Initialize the kernel memory allocator
 */

void
kmeminit(void)
{
	register long indx;
	vm_offset_t min, max;
	int i,j, lastindex;
	void *ptr;

	if(kmeminitialized)
		return;

	for (i = MAXALLOCSAVE; i > 1; i= i>>1)
		maxallocshift++;

	if((MAXALLOCSAVE & (MAXALLOCSAVE - 1)) != 0)
		panic("kmeminit: MAXALLOCSAVE not power of 2");
	if(MAXALLOCSAVE > MINALLOCSIZE * 32768)	/* see BUCKETINDX macro */
		panic("kmeminit: MAXALLOCSAVE too big");

	if (MAXALLOCSAVE > 4 * PAGE_SIZE)
		panic("kmeminit: MAXALLOCSAVE/PAGE_SIZE too big");
	if (MAXALLOCSAVE < PAGE_SIZE)
		panic("kmeminit: MAXALLOCSAVE/PAGE_SIZE too small");

	if (kmempages == 0) {
		extern int vm_page_free_count;
		kmempages = vm_page_free_count;
		if (kmempages < 512 * 1024 / PAGE_SIZE)
			kmempages = 512 * 1024 / PAGE_SIZE;

		/* The following limit of virtual memory to 512MB is   */
		/* a temporary fix.  The 2GB kernel virtual size limit */
		/* was being exceeded when 1GB of physical memory was  */
		/* present. */ 
#define _MALLOC_MAX_VIRT 0x20000000
		if ((kmempages * PAGE_SIZE) > _MALLOC_MAX_VIRT)
			kmempages = _MALLOC_MAX_VIRT / PAGE_SIZE;
	}
	if (kmemmap = kmem_suballoc(kernel_map, &min, &max,
                                (vm_size_t) (kmempages * PAGE_SIZE), FALSE)) {
		kmembase = (void *)min;
		kmemlimit = (void *)max;
	} else
		panic("kmeminit map");
	indx = round_page(sizeof (struct kmemusage) * kmempages);
	if ((kmemusage = (struct kmemusage *)kmem_alloc(kmemmap, indx)) == NULL)
		panic("kmeminit structs");
	bzero((char *)kmemusage, (int)indx);

	pagindx = BUCKETINDX(PAGE_SIZE);
	maxindx = BUCKETINDX(MAXALLOCSAVE);
	mkbp = &bucket[maxindx];
	pkbp = &bucket[pagindx];
	for (indx = 0; indx < MINBUCKET + 16; indx++) {
		if ((bucket[indx].kb_elmpercl = PAGE_SIZE / (1 << indx)) <= 1) {
			bucket[indx].kb_elmpercl = 1;
			bucket[indx].kb_highwat = HIWAT1;
			if (indx >= maxindx && RESERVE > HIWAT1)
				bucket[indx].kb_highwat = RESERVE;
		} else
			bucket[indx].kb_highwat = HIWAT2 * bucket[indx].kb_elmpercl;
		simple_lock_init(&bucket[indx].kb_lock);
		bucket[indx].kb_indx = indx;
		bucket[indx].kb_size = 1 << indx;
	}

#ifdef KMEMTYPES
        for (indx = 0; indx < M_LAST; indx++) {
                kmemtypes[indx].kt_limit = kmempages * PAGE_SIZE * TLIMIT;
                /* Round it off */
                kmemtypes[indx].kt_limit += MINALLOCSIZE - 1;
                kmemtypes[indx].kt_limit &= ~(MINALLOCSIZE - 1);
                kmemtypes[indx].kt_wait = 0;
                simple_lock_init(&kmemtypes[indx].kt_lock);
        }
#endif /* KMEMTYPES */

	/*
	 * Prime the malloc reserve with 32 MAXALLOCSAVE chunks.
	 */
	{
		void *p, *q, **pp = &p;
		int i;
		for (i = 0; i < NPRIME; i++) {
			*pp = malloc(MAXALLOCSAVE, mkbp, M_TEMP, M_WAITOK);
			if( *pp == (void *)0)
				panic("kmeminit: malloc failed\n");
			pp = (void **)*pp;
		}
		*pp = 0;
		do {
			q = *(void **)p;
			free(p, M_TEMP);
		} while (p = q);
	}

	kmeminitialized = 1;
}

void
kmeminit_thread(int pri)
{
	extern task_t first_task;

 	kmemthread = kernel_thread(first_task, malloc_thread); 

	if (kmemthread == NULL)
		panic("kmeminit_thread");
}

