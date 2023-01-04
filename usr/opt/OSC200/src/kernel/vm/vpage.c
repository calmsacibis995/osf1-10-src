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
static char *rcsid = "@(#)$RCSfile: vpage.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/05/05 14:38:57 $";
#endif
#include <vm/heap_kmem.h>
#include <mach/kern_return.h>
#include <vm/vm_map.h>
#include <vm/vm_umap.h>
#include <vm/vpage.h>
#include <vm/vm_tune.h>


#define vpage_alloc(SIZE,SLEEP) (struct vpage *)			\
	h_kmem_zalloc_memory((SIZE) * sizeof (struct vpage), (SLEEP))

#define vpage_free(PTR,SIZE,SLEEP) (void *) 				\
	h_kmem_free_memory((PTR), (SIZE) * sizeof (struct vpage), (SLEEP))

kern_return_t
u_vpage_alloc(struct vm_map *map, 
	vm_size_t size,
	struct vpage **vpp,
	boolean_t canwait)
{
	register struct u_map_private *up;
	up = (struct u_map_private *) (map->vm_private);

	usimple_lock(&up->um_resource);
	if ((up->um_vpage + size) > vm_tune_value(vpagemax)) {
		usimple_unlock(&up->um_resource);
		return KERN_RESOURCE_SHORTAGE;
	}
	else {
		up->um_vpage += size;
		usimple_unlock(&up->um_resource);
	}

	*vpp = vpage_alloc(size, canwait);

	if (*vpp) return KERN_SUCCESS;
	else {
		u_vpage_free_reserve(map, size);
		return KERN_RESOURCE_SHORTAGE;
	}
}

void
u_vpage_free(struct vm_map *map,
	struct vpage *vp,
	vm_size_t size,
	boolean_t canwait)
{
	register struct u_map_private *up;
	up = (struct u_map_private *) (map->vm_private);

	usimple_lock(&up->um_resource);
	up->um_vpage -= size;
	usimple_unlock(&up->um_resource);
	vpage_free(vp, size, canwait);
}

u_vpage_reserve(struct vm_map *map,
	vm_size_t increase)
{
	register struct u_map_private *up;
	up = (struct u_map_private *) (map->vm_private);

	usimple_lock(&up->um_resource);
	if ((up->um_vpage + increase) > vm_tune_value(vpagemax)) {
		usimple_unlock(&up->um_resource);
		return KERN_RESOURCE_SHORTAGE;
	}
	up->um_vpage += increase;
	usimple_unlock(&up->um_resource);
	return KERN_SUCCESS;
}

void
u_vpage_free_reserve(struct vm_map *map,
	vm_size_t decrease)
{
	register struct u_map_private *up;
	up = (struct u_map_private *) (map->vm_private);

	usimple_lock(&up->um_resource);
	up->um_vpage -= decrease;
	usimple_unlock(&up->um_resource);
}

/*
 * This assumes they've already been reserved.
 */

void
u_vpage_expand(struct vpage **vpp,
	vm_size_t oldsize,
	vm_size_t increase,
	vm_size_t offset)
{
	register struct vpage *nvp;

	nvp = vpage_alloc(oldsize + increase, TRUE);
	bcopy((*vpp), nvp + offset, oldsize * sizeof (struct vpage));
	vpage_free(*vpp, oldsize, TRUE);
	*vpp = nvp;
}

