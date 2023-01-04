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
static char *rcsid = "@(#)$RCSfile: rld_initfini.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/18 17:15:03 $";
#endif

#include <stdio.h>
#include <filehdr.h>
#include <aouthdr.h>
#include <scnhdr.h>
#include <sys/errno.h>
#include <sys/auxv.h>
#include <sex.h>
#include "sysmips.h"
#include <obj.h>
#include "obj_list.h"
#include "obj_ext.h"
#include "elf_abi.h"
#include "elf_mips.h"
#include "rld.h"

/*
 *
 *		I N I T	/ F I N I	H A N D L I N G
 *
 * Rather than running init routines when an object is processed,
 * we delay execution of inits until just before the program
 * starts up.  This has some important effects:
 *
 *    1)    dbx has a chance to set breakpoints in init sections.
 *    2)    any necessary crt0 setup can be done before inits are run.
 *    3)    init routines can be run in the proper order.
 *
 * The inits are run in a post-order depth-first walk of the dependencies.
 * During the walk, if an object has an init-routine, the init is run.
 * If the object has a fini-routine, the object is pushed onto the finis
 * stack.  This stack arrangement guarantees that finis are run in the
 * opposite order of inits.  Special precautions are taken to ensure that
 * no init or fini is run twice.
 *
 * The stack implementation uses just an array that is doubled whenever
 * it overflows.  This simple structure also provides reasonable protection
 * for the possibility of a signal interrupt causing fini routines to
 * run while, say, init routines are being processed.  It's true that
 * the rest of rld is not protected at all against this kind of abuse.
 *
 */
typedef struct obj_stack_struct
{
    Rld_Obj		**objs_stack; /* The array of stack elements */
    int			  objs_size;  /* The size of the allocated array */
    int			  objs_top;   /* first unused element in stack */
} obj_stack;

static obj_stack finis;		/* objs with fini routines pending */

#define OBJS_INITIAL_SIZE 10

/*
 * o b j _ s t a c k _ p u s h
 *
 * Push an element onto the stack.
 * If the stack overflows, allocate
 * a new stack that is double the size.
 *
 */
static void
obj_stack_push(Rld_Obj *o)
{
    if (finis.objs_top >= finis.objs_size) {
	if (finis.objs_size == 0) {
	    finis.objs_size = OBJS_INITIAL_SIZE;
	    finis.objs_stack = chkmalloc(sizeof(Rld_Obj *) * OBJS_INITIAL_SIZE);
	}
	else {
	    finis.objs_size *= 2;
	    finis.objs_stack = chkrealloc(finis.objs_stack, 
					  sizeof(Rld_Obj *) * finis.objs_size);
	}
    }
    finis.objs_stack[finis.objs_top++] = o;
}

/*
 * o b j _ s t a c k _ p o p
 *
 * Pop an element from a stack.
 * Return the element, or NULL if empty.
 *
 */
static Rld_Obj *
obj_stack_pop()
{
    Rld_Obj *o;

    for(;;){
	if (finis.objs_top <= 0) {
	    return(NULL);
	}
	if ((o = finis.objs_stack[--finis.objs_top]) != (Rld_Obj *)NULL) {
	    return(o);
	}
    }
}

/*
 * o b j _ s t a c k _ d e l e t e
 *
 * Delete an element from the middle of the stack.  Search for
 * the given element, and delete by sliding everything down.
 * If a signal interrupt arises during the sliding, there are
 * windows during which an object may appear on the list twice.
 * (See the discussion above under INIT/FINI handling).
 *
 * This routine may be called as a side effect of dlclose().
 * For efficiency, the stack is searched from the top down,
 * because dlopen() causes objects to be pushed on the top
 * of the stack.
 *
 * Return 1 for success; 0 for failure.
 */
static int
obj_stack_delete(Rld_Obj *o)
{
    int i;			/* loop iterator */

    for (i = finis.objs_top - 1; i >= 0; i-- ) {
	if (finis.objs_stack[i] == o) {
	    break;
	}
    }

    if (i >= 0) {

	/* Delete the object by shifting the contents of the
	 * finis stack.
	 */
	for (; i<finis.objs_top-1; i++) {
	    finis.objs_stack[i] = finis.objs_stack[i+1];
	}
	finis.objs_top--;
	return(1);
    }

    return(0);
}

/*
 * r u n _ i n i t _ f o r _ o b j
 *
 * Implements a depth first post-order walk of the dependency graph.
 * Init routines are run during the walk, and fini routines are
 * pushed onto the fini stack to ensure that they are executed in
 * the opposite order that the inits are run.
 */

void run_init_for_obj(HANDLE *h)
{
    int i;

    if (h->flags & RAN_INITS) return;

    /* Prevent recursion on cyclic dependencies */

    h->flags |= RAN_INITS;

    for (i = h->liblist_count - 1; i >= 0; i--) {
	if (h->liblist[i]) {
	    run_init_for_obj(h->liblist[i]);
	}
    }

    /* Dynamically opened objects will cause multiple walks of the
     * graph.  Check the flags to avoid running inits multiple times
     * for a shared object.
     */

    if (obj_init_address(h->obj)) {
        TRACE("calling .init section 0x%p\n", obj_init_address(h->obj));
        init_bridge(obj_init_address(h->obj));
    }

    if (obj_fini_address(h->obj)) {
        obj_stack_push(h->obj);
    }
}

/*
 * r l d _ r u n _ i n i t s
 *
 * Run any inits queued up.  This is called
 * at program startup time, or when a dlopen
 * is ready to return.
 *
 */
void
rld_run_inits()
{
    Rld_Obj *o;			/* obj with init or fini section */

    TRACE("Run inits\n");
    run_init_for_obj(pObj_Head);
    if (clearFlag) {
	osf_clearstack(osf_stackdepth());
    }
}

/*
 * r l d _ r u n _ f i n i s
 *
 * Run any finis queued up.  This is called at
 * exit time.
 *
 */
void
rld_run_finis()
{
    Rld_Obj *o;			/* obj with fini section */

    TRACE("Run finis\n");
    while ((o = obj_stack_pop()) != NULL) {
	TRACE("calling .fini section 0x%p\n", obj_fini_address(o));
	fini_bridge(obj_fini_address(o));
    }
}

/*
 * r l d _ r u n _ o n e _ f i n i
 *
 * Run just a single fini section.  Used for dlclose.
 * Find the given obj in the fini stack, run the associated
 * routine in the list and delete it from the list.
 *
 */
void
rld_run_one_fini(Rld_Obj *o)
{
    if (obj_fini_address(o)) {
	/* Only run the fini if it hasn't already been run.  This can
	 * happen if dlclose() is called from another fini routine.
	 */
	if (obj_stack_delete(o)) {
	    TRACE("calling .fini section 0x%p\n", obj_fini_address(o));
	    fini_bridge(obj_fini_address(o));
	}
    }
}


