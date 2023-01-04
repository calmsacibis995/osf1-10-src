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
static char	*sccsid = "@(#)$RCSfile: subr_rmap.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/02/11 16:25:30 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/kernel.h>

#define USE_MACH_MAPS	0		/* they don't always work at interrupt level */

/* XXX Should excise this option if we don't think it will ever be viable */

#if	USE_MACH_MAPS
#include <mach/kern_return.h>
#include <vm/vm_map.h>
#endif	USE_MACH_MAPS

/*
 * Resource map handling routines.
 *
 * A resource map is an array of structures each
 * of which describes a segment of the address space of an available
 * resource.  The segments are described by their base address and
 * length, and sorted in address order.  Each resource map has a fixed
 * maximum number of segments allowed.  Resources are allocated
 * by taking part or all of one of the segments of the map.
 *
 * Returning of resources will require another segment if
 * the returned resources are not adjacent in the address
 * space to an existing segment.  If the return of a segment
 * would require a slot which is not available, then one of
 * the resource map segments is discarded after a warning is printed.
 * Returning of resources may also cause the map to collapse
 * by coalescing two existing segments and the returned space
 * into a single segment.  In this case the resource map is
 * made smaller by copying together to fill the resultant gap.
 *
 * N.B.: the current implementation uses a dense array and does
 * not admit the value ``0'' as a legal address, since that is used
 * as a delimiter.
 */

/*
 * Initialize map mp to have (mapsize-2) segments
 * and to be called ``name'', which we print if
 * the slots become so fragmented that we lose space.
 * The map itself is initialized with size elements free
 * starting at addr.
 */
rminit(mp, size, addr, name, mapsize)
	register struct map *mp;
	long size, addr;
	char *name;
	long mapsize;
{
	register struct mapent *ep = (struct mapent *)(mp+1);

	mp->m_name = name;
#if	USE_MACH_MAPS
	mp->m_limit = (struct mapent *) vm_map_create(0, addr, addr+size, FALSE);
#else	USE_MACH_MAPS
/* N.B.: WE ASSUME HERE THAT sizeof (struct map) == sizeof (struct mapent) */
	/*
	 * One of the mapsize slots is taken by the map structure,
	 * segments has size 0 and addr 0, and acts as a delimiter.
	 * We insure that we never use segments past the end of
	 * the array which is given by mp->m_limit.
	 * Instead, when excess segments occur we discard some resources.
	 */
	mp->m_limit = (struct mapent *)&mp[mapsize];
	/*
	 * Simulate a rmfree(), but with the option to
	 * call with size 0 and addr 0 when we just want
	 * to initialize without freeing.
	 */
	ep->m_size = size;
	ep->m_addr = addr;
#endif	USE_MACH_MAPS
}

/*
 * Allocate 'size' units from the given
 * map. Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 *
 * Algorithm is first-fit.
 *
 * This routine knows about the interleaving of the swapmap
 * and handles that.
 */
long
rmalloc(mp, size)
	register struct map *mp;
	long size;
{
	register struct mapent *ep = (struct mapent *)(mp+1);
	register long addr;
	register struct mapent *bp;

	if (size <= 0)
		panic("rmalloc");
#if	USE_MACH_MAPS
	{
		vm_map_t	map;
		kern_return_t	retval;
		long		my_addr;

		map = (vm_map_t) mp->m_limit;
		my_addr = map->min_offset;
		retval = vm_map_find(map, 0, 0, &my_addr, size, TRUE);
		if (retval != KERN_SUCCESS)
			return(0);
		return(my_addr);
	}
#else	USE_MACH_MAPS
	/*
	 * Search for a piece of the resource map which has enough
	 * free space to accomodate the request.
	 */
	for (bp = ep; bp->m_size; bp++) {
		if (bp->m_size >= size) {
			/*
			 * Allocate from the map.
			 * If there is no space left of the piece
			 * we allocated from, move the rest of
			 * the pieces to the left.
			 */
			addr = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
			}
			return (addr);
		}
	}
	return (0);
#endif	USE_MACH_MAPS
}

/*
 * Free the previously allocated space at addr
 * of size units into the specified map.
 * Sort addr into map and combine on
 * one or both ends if possible.
 */
rmfree(mp, size, addr)
	struct map *mp;
	long size, addr;
{
	struct mapent *firstbp;
	register struct mapent *bp;
	register long t;

	/*
	 * Address must be non-zero and size must be
	 * positive, or the protocol has broken down.
	 */
	if (addr == 0 || size <= 0)
		goto badrmfree;
#if	USE_MACH_MAPS
	{
		vm_map_t	map;

		map = (vm_map_t) mp->m_limit;
		(void) vm_map_remove(map, addr, addr+size);
		goto done;
	}
#else	USE_MACH_MAPS
	/*
	 * Locate the piece of the map which starts after the
	 * returned space (or the end of the map).
	 */
	firstbp = bp = (struct mapent *)(mp + 1);
	for (; bp->m_addr <= addr && bp->m_size != 0; bp++)
		continue;
	/*
	 * If the piece on the left abuts us,
	 * then we should combine with it.
	 */
	if (bp > firstbp && (bp-1)->m_addr+(bp-1)->m_size >= addr) {
		/*
		 * Check no overlap (internal error).
		 */
		if ((bp-1)->m_addr+(bp-1)->m_size > addr)
			goto badrmfree;
		/*
		 * Add into piece on the left by increasing its size.
		 */
		(bp-1)->m_size += size;
		/*
		 * If the combined piece abuts the piece on
		 * the right now, compress it in also,
		 * by shifting the remaining pieces of the map over.
		 */
		if (bp->m_size && addr+size >= bp->m_addr) {
			if (addr+size > bp->m_addr)
				goto badrmfree;
			(bp-1)->m_size += bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
		}
		goto done;
	}
	/*
	 * Don't abut on the left, check for abutting on
	 * the right.
	 */
	if (addr+size >= bp->m_addr && bp->m_size) {
		if (addr+size > bp->m_addr)
			goto badrmfree;
		bp->m_addr -= size;
		bp->m_size += size;
		goto done;
	}
	/*
	 * Don't abut at all.  Make a new entry
	 * and check for map overflow.
	 */
	do {
		t = bp->m_addr;
		bp->m_addr = addr;
		addr = t;
		t = bp->m_size;
		bp->m_size = size;
		bp++;
	} while (size = t);
	/*
	 * Segment at bp is to be the delimiter;
	 * If there is not room for it 
	 * then the table is too full
	 * and we must discard something.
	 */
	if (bp+1 > mp->m_limit) {
		/*
		 * Back bp up to last available segment.
		 * which contains a segment already and must
		 * be made into the delimiter.
		 * Discard second to last entry,
		 * since it is presumably smaller than the last
		 * and move the last entry back one.
		 */
		bp--;
		printf("%s: rmap ovflo, lost [%d,%d)\n", mp->m_name,
		    (bp-1)->m_addr, (bp-1)->m_addr+(bp-1)->m_size);
		bp[-1] = bp[0];
		bp[0].m_size = bp[0].m_addr = 0;
	}
#endif	USE_MACH_MAPS
done:
	return;
badrmfree:
#if	USE_MACH_MAPS
	vm_map_print((vm_map_t) mp->m_limit);
#else	USE_MACH_MAPS
	printf("map \"%s\" @%x Limit %x... ",
		mp->m_name, mp, mp->m_limit);
#endif	USE_MACH_MAPS
	panic("bad rmfree");
}

/*
 * Allocate 'size' units from the given map, starting at address 'addr'.
 * Return 'addr' if successful, 0 if not.
 * This may cause the creation or destruction of a resource map segment.
 *
 * This routine will return failure status if there is not enough room
 * for a required additional map segment.
 *
 * An attempt to use this on 'swapmap' will result in
 * a failure return.  This is due mainly to laziness and could be fixed
 * to do the right thing, although it probably will never be used.
 */
rmget(mp, size, addr)
	register struct map *mp;
	long size, addr;
{
	register struct mapent *ep = (struct mapent *)(mp+1);
	register struct mapent *bp, *bp2;

	if (size <= 0)
		panic("rmget");
#if	USE_MACH_MAPS
	{
		vm_map_t	map;
		kern_return_t	retval;

		map = (vm_map_t) mp->m_limit;
		retval = vm_map_find(map, 0, 0, &addr, size, FALSE);
		if (retval != KERN_SUCCESS)
			return(0);
		return(addr);
	}
#else	USE_MACH_MAPS
	/*
	 * Look for a map segment containing the requested address.
	 * If none found, return failure.
	 */
	for (bp = ep; bp->m_size; bp++)
		if (bp->m_addr <= addr && bp->m_addr + bp->m_size > addr)
			break;
	if (bp->m_size == 0)
		return (0);

	/*
	 * If segment is too small, return failure.
	 * If big enough, allocate the block, compressing or expanding
	 * the map as necessary.
	 */
	if (bp->m_addr + bp->m_size < addr + size)
		return (0);
	if (bp->m_addr == addr)
		if (bp->m_addr + bp->m_size == addr + size) {
			/*
			 * Allocate entire segment and compress map
			 */
			bp2 = bp;
			while (bp2->m_size) {
				bp2++;
				(bp2-1)->m_addr = bp2->m_addr;
				(bp2-1)->m_size = bp2->m_size;
			}
		} else {
			/*
			 * Allocate first part of segment
			 */
			bp->m_addr += size;
			bp->m_size -= size;
		}
	else
		if (bp->m_addr + bp->m_size == addr + size) {
			/*
			 * Allocate last part of segment
			 */
			bp->m_size -= size;
		} else {
			/*
			 * Allocate from middle of segment, but only
			 * if table can be expanded.
			 */
			for (bp2=bp; bp2->m_size; bp2++)
				;
			if (bp2 + 1 >= mp->m_limit)
				return (0);
			while (bp2 > bp) {
				(bp2+1)->m_addr = bp2->m_addr;
				(bp2+1)->m_size = bp2->m_size;
				bp2--;
			}
			(bp+1)->m_addr = addr + size;
			(bp+1)->m_size =
			    bp->m_addr + bp->m_size - (addr + size);
			bp->m_size = addr - bp->m_addr;
		}
	return (addr);
#endif	USE_MACH_MAPS
}

#ifdef	notdef
rmprint(rmap)
	register struct map *rmap;
{
	register struct mapent *ep = (struct mapent *) (rmap + 1);

	printf("map \"%s\" @%x limit %x\n",
		rmap->m_name, rmap, rmap->m_limit);

	for ( ; ep->m_size && ep < rmap->m_limit; ep++)
		printf("\t%x %x\n", ep->m_addr, ep->m_size);

	if (ep == rmap->m_limit)
		printf("MAP FULL\n");
}
#endif	notdef
