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
static char *rcsid = "@(#)$RCSfile: dma_direct_map.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/10/19 21:49:52 $";
#endif
/*
 * Abstract:
 * 	Per-platform support to obtain the necessary scatter-gather
 * 	maps for DMA devices.
 *
 * 	These routines are geared for systems that have no hardware
 * 	scatter-gather mapping support. i.e., io bus memory addresses
 * 	directly map to system memory/core addresses.
 *
 * Revision History: 
 *	June 8, 1993	Donald Dutile	Original version
 *	
 */

#include <sys/types.h>
#include <io/common/devdriver.h>
#include <mach/vm_param.h>	/* to get page_size */
#include <sys/proc.h>
#include <kern/task.h>
#include <vm/vm_map.h>

/* For Rev. 2.0 for JENSEN - larger (first/super) sglist structure and
 *			     min-sized zalloc'd babc structure/space 
 */

/* #define DMA_MAP_DEBUG */

/* #define DMA_ZALLOC_DEBUG 1 */



/************************************************************************/
/*									*/
/* direct_map_alloc  -  Allocate sufficient kernel resources to perform */
/*			a dma transfer of "bc" bytes.			*/
/*		      							*/
/* SYNOPSIS								*/
/*	u_long	direct_map_alloc(u_long bc, struct controller *cntrlrp, */
/*				 sglist_t *sglistp, int flags)		*/
/*		      							*/
/* PARAMETERS								*/
/* 	bc	Number of DMA bytes that kernel resources must be 	*/
/*		capable of supporting.					*/
/*	cntrlrp Pointer to controller structure that is associated with */
/*		DMA controller.						*/
/*	sglistp Pointer to a ptr to a scatter-gather cntrl list struct.	*/
/*	flags	Flag word indicating if caller wants to sleep if system */
/*		capable of supporting a bc-sized transfer, but limited  */
/*		resources at this moment; return success only if all    */
/*		requested resources are returned, 			*/
/*									*/
/* DESCRIPTION								*/
/*	This function allocates the kernel data structures necessary to */
/*	map a DMA transfer of size "bc" (max.).  If flags indicates a   */
/*	sleep if sufficient resources not currently available (and      */
/*	system is able to support bc-sized transfer), this routine      */
/*	will sleep in a FIFO-style queue.  If flags indicates only      */
/*	succeed if *all* resources to perform a "bc-sized" transfer are */
/*	available, the function will return failure if system unable to */
/*	perform a transfer of size "bc".				*/
/*									*/
/*      V2.0: allocate a "super" sglist structure as first sglist       */
/*		and link normal sglist struct for other babc lists;	*/
/*		babc lists may now vary in size.			*/
/*									*/
/* RETURN VALUES						 	*/
/*	A return byte-count value of zero (0) indicates a failure.      */
/*	Otherwise, the returned value is the maximum number of bytes    */
/*	a DMA transfer can have for the (higher-level) dma_map_alloc()  */
/*	call.								*/
/*	Note: *ALL* drivers must be able to support a return bc value   */
/*	      that is less than the "bc" value requested.		*/
/************************************************************************/

u_long
direct_map_alloc(u_long bc, struct controller *cntrlrp, sglist_t *sglistpp,
		 int flags)
{
	sglist_t	sglistp = (sglist_t)(NULL);
	struct	ovrhd	*sgl_ovrhdp;
	sglist_t	sglistp_curr = (sglist_t)(NULL);
	sglist_t	sglistp_next = (sglist_t)(NULL);
	sg_entry_t	sg_entryp= (sg_entry_t)(NULL);
	u_long		rtn_bc = 0L;
	long		ttl_num_ents;
	long		num_ents;
	int		num_guard_pages = 0;

#ifdef DMA_MAP_DEBUG
	printf("ENTERED direct_map_alloc, calling params: \n");
	printf("\t bc      = 0x%lx \n", bc);
	printf("\t cntrlrp = 0x%lx \n", cntrlrp);
	printf("\t flags   = 0x%x \n", flags);
#endif	/* DMA_MAP_DEBUG */

	/* 
	 * add one page_size to bc in case buffer crosses a page boundary;
	 * add one page_size per guard page requested
	 * add one page_size if bc less than page_size, since div = 0;
	 */

	if (flags & DMA_GUARD_UPPER) num_guard_pages++;
	if (flags & DMA_GUARD_LOWER) num_guard_pages++;

	num_ents = ttl_num_ents = bc/page_size + 1L + num_guard_pages;
	/* in following case, bc/page_size div. = 0 */
	if (bc < page_size) {
		ttl_num_ents = ++num_ents;
	}

	/* Alloc first "super" sglist struct = 2X sizeof(sglist);
	 * use "super" space to store overhead: load's va & bc; zone size;
	 */
	sglistp_curr = sglistp = (sglist_t)dma_zalloc_super_sglist(flags);
	if (sglistp == (sglist_t)NULL) {
		return(0L);		/* no mem. resources! */
	}
	sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
	sglistp->flags = flags; 	/* copy flags */
	sgl_ovrhdp->cntrlrp = cntrlrp;	/* cntrlrp for dma_unload/_dealloc */
	*sglistpp = sglistp;		/* set return value		*/
#ifdef DMA_ZALLOC_DEBUG
	printf("ENTERED direct_map_alloc \n");
	printf("\t number of babc pairs calc-needed: 0x%lx \n", ttl_num_ents);
	printf("\t super sglistp value:              0x%lx \n", sglistp);
#endif /* DMA_ZALLOC_DEBUG */


	/* Try to fill babc list for first sglist struct;
	 * for the majority of DMA transfers, this alloc is sufficient,
	 * so optimize code path for this case.
	 */
	sg_entryp =
	   (sg_entry_t)dma_zalloc(num_ents*sizeof(struct sg_entry), flags);
	if (sg_entryp == (sg_entry_t)NULL) { 		        /* fail if not all */
	   dma_zfree_super_sglist((vm_offset_t)sglistp); /* free sglist */
		return(0L);
	}
	sglistp->sgp = sg_entryp;
	/* ba = number of babc elements alloc/supported in zone   */
	sglistp->num_ents = (unsigned int)sg_entryp->ba; 
	/* must zero 1st babc list entry for load's concat alg. to work */
	bzero(sg_entryp, sizeof(struct sg_entry)); 
#ifdef DMA_ZALLOC_DEBUG
	printf("\t babc sgp value:		0x%lx \n", sglistp->sgp);
	printf("\t no. of babc entrys alloc'd:	0x%lx \n", sglistp->num_ents);
#endif /* DMA_ZALLOC_DEBUG */

	/* are more babc lists needed? */
	num_ents -= (long)(sglistp->num_ents);
	if (num_ents <= 0L) {
		return(bc);   /* all necessary resources obtained */
	}

	/*
	 * get more sglist structs & assoc. babc lists if necessary.
	 */
#ifdef DMA_ZALLOC_DEBUG
	printf("\t Entering for-loop to get multiple babc lists\n");
#endif /* DMA_ZALLOC_DEBUG */
	for (; num_ents > 0; num_ents -= (unsigned int)sg_entryp->ba) {
	   /*
 	    * first get an sglist structure 
	    */
	   sglistp_next = (sglist_t)dma_zalloc_sglist(flags);
	   /* if can't get anymore sglist struct's, return what one can */
	   if (sglistp_next == (sglist_t)NULL) {
	     if (flags & DMA_ALL) { /* DMA_ALL or nothing */
		dma_map_dealloc(*sglistpp);
             } else { 
		break;	/* jump out of for-loop when out of mem. */
	     }
	   }
	   /* next, get assoc. babc list */
	   sg_entryp = 
	       (sg_entry_t)dma_zalloc(num_ents*sizeof(struct sg_entry), flags);
           if (sg_entryp == (sg_entry_t)NULL) { 
	     if (flags & DMA_ALL) { /* DMA_ALL or nothing */
                dma_map_dealloc(*sglistpp);
             } else {
	 	dma_zfree_sglist((vm_offset_t)sglistp_next);	
		break;
             }
	   }
	   sglistp_next->sgp = sg_entryp;
	   sglistp_next->num_ents = (unsigned int)sg_entryp->ba;
	   /* must zero 1st babc list entry for load's concat alg. to work */
	   bzero(sg_entryp, sizeof(struct sg_entry)); 
	   sglistp_curr->next = sglistp_next;
	   sglistp_curr = sglistp_next;
#ifdef DMA_ZALLOC_DEBUG
	printf("\t Obtained new sglistp = 0x%lx \n", sglistp_next);
	printf("\t Obtained new sgp     = 0x%lx \n", sg_entryp);
	printf("\t No. of babc entries  = 0x%x  \n", sglistp->next->num_ents);
#endif /* DMA_ZALLOC_DEBUG */

	   /* if here, then sglist & babc list memory obtained.
	    * so let's update looping params and check for completion
	    */

	} /* end of for-loop */

	if (num_ents == 0L) { 
		rtn_bc = bc;
	} else {
	/* Note: this calc. wouldn't work if num_ents remaining
	 *	 was close to ttl_num_ents;  it does work since
	 *	 min.-size babc alloc guarantees a size of at
	 *	 least an sglist struct size (32 bytes).
	 */
		rtn_bc = page_size*(ttl_num_ents - num_ents - num_guard_pages 
									- 1L);
	}

#ifdef DMA_ZALLOC_DEBUG
	printf("\t rtn_bc value:	0x%lx \n", rtn_bc);
#endif /* DMA_ZALLOC_DEBUG */
        return(rtn_bc);
}

/************************************************************************/
/*									*/
/* direct_map_load  -  Load a scatter-gather list structure with valid  */
/*		       bus address & byte count info for a DMA transfer.*/
/*		      							*/
/* SYNOPSIS								*/
/*	int	direct_map_load(u_long bc, vm_offset_t va, struct proc 	*/
/*			*procp, struct sglist *sglistp, u_long max_bc)	*/
/*		      							*/
/* PARAMETERS								*/
/* 	bc	Number of bytes that DMA transfer wants mapping for.    */
/*	va	Starting virtual address of DMA buffer. 		*/
/*	procp	Pointer to proc struct that va is valid in. 		*/
/*		procp = 0 for kernel/system va.				*/
/*	sglistp Pointer to a scatter-gather cntrl list structure.	*/
/*	max_bc	The maximum contiguous number of bytes a ba-bc pair     */
/*		should be valid in the s-g list.			*/
/*									*/
/* DESCRIPTION								*/
/*	This function fills in a previously allocated scatter-gather    */
/*	list and respective control structure with the necessary bus	*/
/*	addresses and byte count information needed for a DMA transfer	*/
/*	by a controller on a bus.					*/
/*	The function will pack contiguous memory sections into a single */
/*	ba-bc pair up to the maximum byte count indicated by max_bc.	*/
/*	max_bc allows i/o hw that has byte count limitations (e.g.,     */
/*	use a 16-bit counter) to have optimal s-g lists returned to it  */
/*	for use (by the driver).					*/
/*									*/
/* RETURN VALUES						 	*/
/*	A successful return is one (1). A failure a zero (0).		*/
/*	V2.0: Fail if dma_map_load bc is > dma_map_alloc bc.
/*									*/
/************************************************************************/

int
direct_map_load(u_long bc, vm_offset_t va, struct proc *procp,
		struct sglist *sglistp, unsigned long max_bc)
{
	/* dgdfix: phys_addr, va_to_map, sg_entryp, index should probably 
	 *	    have "register" storage class specifiers.
	 */
	vm_offset_t 	phys_addr;
	vm_offset_t	va_to_map = va;
	sg_entry_t	sg_entryp = sglistp->sgp; /* top of ba-bc list 	*/
	sg_entry_t	prev_sg_entryp; 
	u_long		dma_page_offset;  /* to match bc variable type 	*/
	u_long		sgentry_bc;	  /* bc for ba-bc pair         	*/
	/* overhead area of super sglistp */
	struct	ovrhd	*sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
	int		add_new_sgentry;  /* flag : new sgentry needed 	*/

	sgl_ovrhdp->va = va;		/* save for dma_kmem_buffer 	*/
	sgl_ovrhdp->procp = procp;	/* save for dma_kmem_buffer 	*/
	sglistp->val_ents = 0;
	sglistp->index = 0; 	 	/* just in case	*/

	if (max_bc == 0)
		max_bc--;	/* set to infinity */

#ifdef DMA_MAP_DEBUG
	printf("\n ENTERED direct_map_load(). params : \n");
	printf("\t bc	      = 0x%lx \n", bc);
	printf("\t va_to_map  = 0x%lx \n", va_to_map );
	printf("\t proc       = 0x%lx \n", procp);
	printf("\t sglistp    = 0x%lx \n", sglistp);
	printf("\t max_bc     = 0x%lx \n", max_bc);
#endif	/* DMA_MAP_DEBUG */

	for(; bc > 0;va_to_map += sgentry_bc, bc -= sgentry_bc) {
	   /* First, get physical address */
	   if (procp) {
	 	phys_addr = pmap_extract(procp->task->map->vm_pmap,va_to_map);
	   } else	{	/* 0 == kernel */
		svatophys(va_to_map,&phys_addr);
	   }

	   /* Next, calc. assoc. byte count (to end of page) */
	   dma_page_offset = (va_to_map & page_mask);
		
	   /* for a transfer totally within a page */
	   if ((bc + dma_page_offset) <= page_size) {
		sgentry_bc = bc;
	   } else {	/* to end of page */
		sgentry_bc = (page_size - dma_page_offset);
	   }

	   /* for max_bc's less than an alpha page 
	    * reduce calc'd bc and skip ba-bc concatenation
	    */
	   add_new_sgentry = 0;
	   if (sgentry_bc > max_bc) {
		sgentry_bc = max_bc;
	   	add_new_sgentry++;
	   } else {
	   /*
	    * Concatenate ba-bc entries up to a bc <= max_bc
	    * Note: only concatenate ba-bc's within a single ba-bc list;
	    *	    don't go backward if multiple ba-bc lists
	    */
		if (sg_entryp != sglistp->sgp) {   /* prev. entry exists */
		   prev_sg_entryp = &sg_entryp[-1];
		   if ((bus_addr_t)phys_addr == 
			     (prev_sg_entryp->ba + prev_sg_entryp->bc)) {
		   	if ((prev_sg_entryp->bc + sgentry_bc) <= max_bc) {
			    prev_sg_entryp->bc += sgentry_bc;
		   	} else { /* if entries compacted, would exceed max_bc */
#ifdef DMA_MAP_DEBUG
			    printf("\t sgentry->ba    = 0x%lx \n", sg_entryp->ba);
			    printf("\t sgentry->bc    = 0x%lx \n", sg_entryp->bc);
#endif	/* DMA_MAP_DEBUG */
			    add_new_sgentry++;
		   	}
	   	   } else { /* page not contig. w/previous page */
			add_new_sgentry++;
		   }
	   	} else { /* top of contig. s-g list, just put it in */
		   add_new_sgentry++;
		}

	   } /* end of if(sgentry_bc > max_bc) */

	   /* Common path if new ba-bc pair must be added to sg list */
	   if (add_new_sgentry) { 
#ifdef DMA_MAP_DEBUG
		printf("New sgentry added to sg list \n");
#endif	/* DMA_MAP_DEBUG */
		sg_entryp->ba = (bus_addr_t)phys_addr;
		sg_entryp->bc = sgentry_bc;
		sg_entryp++;
	        /*
		 * See if run out of room to load babc's in this list
	         */
		if (++(sglistp->val_ents) == sglistp->num_ents) {
#ifdef DMA_ZALLOC_DEBUG
		   printf("sglistp->val_ents = 0x%lx \n", sglistp->val_ents);
#endif	/* DMA_ZALLOC_DEBUG */
		   if (sglistp->next != (sglist_t)NULL) {
			sglistp = sglistp->next; /* go to next babc list */
			sglistp->index = 0; 	 /* just in case 	 */
			sglistp->val_ents = 0;	 /* zero valid entries   */
			sg_entryp = sglistp->sgp; /* get next babc list ptr */
		   } else { /* requesting more resources than can be mapped? */
			if ((bc - sgentry_bc) > 0)   /* load bc > alloc bc  */
				return(0); 	      /* error rtn */
		   }
		}
	   } /* end of add_new_sgentry */

#ifdef DMA_MAP_DEBUG
	   printf("\t sglistp->num_ents = 0x%x \n", sglistp->num_ents);
	   printf("\t sglistp->val_ents = 0x%x \n", sglistp->val_ents);
	   printf("\t sglistp->sg_entryp = 0x%lx \n", sg_entryp);
#endif	/* DMA_MAP_DEBUG */
	} /* end for-loop */

	 return(1);	/* success = 1 */
}


/************************************************************************/
/*									*/
/* direct_map_unload  -  Clear/clean/init a scatter-gather list struct. */
/*			 for future (re-)use.				*/
/*		      							*/
/* SYNOPSIS								*/
/*	int	direct_map_unload(struct sglist *sglistp)		*/
/*		      							*/
/* PARAMETERS								*/
/*	sglistp Pointer to a scatter-gather cntrl list structure.	*/
/*									*/
/* DESCRIPTION								*/
/*	Since no resources are released or invalidated on direct-map	*/
/*	systems, there is little more to do than set the "index" in     */
/*	the s-g list structure to 0, indicating read/write at beginning */
/*	of ba-bc pair list.						*/
/*									*/
/*      V1.0: ASSUMES a single sglist structure that points to a        */
/*		(8K) page of available ba-bc pairs to be filled in.	*/
/*									*/
/* RETURN VALUES						 	*/
/*	A successful return is one (1). A failure a zero (0).		*/
/*									*/
/************************************************************************/
int
direct_map_unload(struct sglist *sglistp)
{
	/* Zero index & valid entries fields */

	sglistp->val_ents = 0;
	sglistp->index = 0;
	return(1);
}

/************************************************************************/
/*									*/
/* direct_map_dealloc  -  Free up in-memory data structures allocated   */
/*			  by dma_map_alloc()				*/
/*		      							*/
/* SYNOPSIS								*/
/*	int	direct_map_dealloc(struct sglist *sglistp)		*/
/*		      							*/
/* PARAMETERS								*/
/*	sglistp Pointer to a scatter-gather cntrl list structure.	*/
/*									*/
/* DESCRIPTION								*/
/*	The function does a kfree of previously allocated sglist &      */
/*	ba-bc pair in-memory data structures.				*/
/*									*/
/*      V1.0: ASSUMES a single sglist structure that points to a        */
/*		(8K) page of available ba-bc pairs to be filled in.	*/
/*									*/
/* RETURN VALUES						 	*/
/*	A successful return is one (1). A failure a zero (0).		*/
/*	V1.0 note: only succeeds.					*/
/*									*/
/************************************************************************/
int
direct_map_dealloc(struct sglist *sglistp)
{
	/* free everything but super sglist & assoc. sgentry/babc list */
	if (sglistp->next != (sglist_t)NULL) {
		free_sglist(sglistp->next);
        }
	/* dma_map_alloc fails if super's babc can't be alloc'd */
	dma_zfree(sizeof(struct sg_entry)*sglistp->num_ents, 
					(vm_offset_t)sglistp->sgp);
	/* first sglist is a "super"-sized one */
	dma_zfree_super_sglist((vm_offset_t)sglistp);

	return(1);	/* always succeed releasing resources */
}

/************************************************************************/
/*                                                                      */
/* free_sglist  -  Deallocates memory resources associated w/DMA        */
/*		   *except* for first/super sglist and its assoc. babc  */
/*		   list.
/*                                                                      */
/************************************************************************/
int
free_sglist(struct sglist *sglistp)
{
	while(sglistp->next != NULL) {
		free_sglist(sglistp->next);
	}

	/* may not have gotten a babc list, so check first */
	if (sglistp->sgp != (sg_entry_t)NULL)
		dma_zfree(sglistp->num_ents*sizeof(struct sg_entry), 
						(vm_offset_t)sglistp->sgp);
	/* ok, now get rid of this sglist structure */
	dma_zfree_sglist((vm_offset_t)sglistp);
	return(0);
}
