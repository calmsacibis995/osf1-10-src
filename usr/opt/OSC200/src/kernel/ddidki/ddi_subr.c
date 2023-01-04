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
 *
 *  Module: 	ddi_subr.c
 *
 *  Description:
 *  	This module contains DDI/DKI support functions that are invoked
 *	by the OSF/1 kernel.
 *
 */

#include <sys/types.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/vnode.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <sys/uio.h>

/*
 * This is the descriptor for the memory used by the DDI/DKI functions
 * kmem_alloc(), kmem_zalloc(), etc.
 */
vm_map_t    	    ddi_map = NULL;
extern int          ddi_map_size;

/*
 * Name:	
 *	ddi_init
 *
 * Description:
 *	This function should contain any initialization code for the DDI/DKI
 *	package. 
 *
 *	Allocate a chunk of kernel memory that is non-pageable and will
 *	permit the user to wait if there is not enough memory available 
 *	to immediately satisfy the request. 
 *
 * Inputs:
 *	None
 *
 * Outputs:
 *	None
 *
 * Calls:
 *	kmem_suballoc()
 *
 * Called by:
 *	vm_mem_init()
 *	
 * Side effects:
 *	Create and initialize the memory submap ddi_map.
 *	
 * Notes:
 *	Only the DDI/DKI functions ddi_kmem_alloc() and ddi_kmem_zalloc()
 *	allocate memory from the submap ddi_map. They do not use kernel_map
 *	because it does not allow a user to wait if insufficient space is
 *	available to satisfy the request. ddi_map_size is a configurable
 *	option.  The default is 64K. See param.c
 */

ddi_init()
{
    vm_offset_t	    min,max;

    /*
     * Allocate the memory from the kernel map. If this doesn't work, the
     * kernel will panic in kmem_suballoc, so there is no need to check for
     * errors here.  min and max are set in kmem_suballoc to the start and end
     * addresses of the map.
     */
    ddi_map = kmem_suballoc(kernel_map, &min, &max, (vm_size_t)ddi_map_size, FALSE);

    /* Allow processes to wait for space if so desired*/
    ddi_map->vm_wait_for_space = TRUE;
    return;
}


/*
 * Name:
 *	ddi_init_uio
 *
 * Description:
 *	Initialize uio fields added to support the DDI/DKI interfaces
 *
 * Inputs:
 *	vp 	- pointer to vnode involved in I/O operation
 *	up	- pointer to uio structure
 *	fflags 	- flags from file block if we're called by vn_read or 
 *		  vn_write, OSF/1 input/output flags if called from vn_rdwr
 *	trans	- FALSE if we ioflags are ok as is (called from vn_read
 *		  or vn_write). TRUE if we need to translate ioflags.
 *
 * Outputs:
 *	None
 *
 * Calls:
 *	None
 *
 * Called By:
 *	vn_read, vn_write, vn_rdwr
 *
 * Side effects:
 *	Initialize uio_fmode and uio_limit
 *
 * Notes:
 *	The DDI/DKI Reference Manual is not particularly clear on what
 *	exactly is stored in the uio_fmode field.  It states on pg. 4-19
 *	that uio_fmode contains "file mode flags".  After reading pg. 153
 *	of "Writing a Unix Device Driver" (Egan and Teixeira), it seems
 *	most likely that uio_fmode is supposed to contain the open file
 *	flags that are returned by the fcntl system call for the F_GETFL
 *	command.  For read and write system calls, the base OSF/1 kernel 
 *	extracts these flags from a file  structure, translates the values 
 *	and then passes the result as a parameter through the file system 
 *	layers to the appropriate device driver function.  See (vn_read
 *	and vn_write).  The flags extracted from the file block will be
 *	passed to this routine and directly stored in uio_fmode. 
 *
 *  	The function vn_rdwr is called all over the kernel for things like
 *	reading/writing a directory, reading in an executable file's header
 *	etc.  vn_rdwr is invoked with flags that must be translated before
 *	initializing uio_fmode.
 */

ddi_init_uio(vp, fflags, up, trans)

struct	vnode	*vp;
int	fflags;
struct	uio	*up;
int	trans;

{
    if (trans == FALSE)	    /*flags are fine as is*/
        up->uio_fmode = fflags;
    else {  	    	    /*flags must be translated*/
    	up->uio_fmode = 0;
    	if (fflags & IO_APPEND)
    	    up->uio_fmode |= FAPPEND;
    	if (fflags & IO_SYNC)
    	    up->uio_fmode |= FSYNC;
    	if (fflags & (IO_NDELAY|IO_NONBLOCK))
    	    up->uio_fmode |= FNDELAY;
    }

    /*
     * File limit from u_rlimit. We have to caste here because rlim_cur
     * is an unsigned int, while uio_limit is daddr_t.  Unfortunately,
     * a daddr_t is just a long.  Hopefully, we won't run into maximum
     * block offsets of greater than 0x7fffffff.  If it's going to be a
     * problem, we should make uio_limit unsigned int and ignore the DDI/DKI
     * Reference Manual.
     */
    up->uio_limit = (daddr_t)u.u_rlimit[RLIMIT_FSIZE].rlim_cur;
    return;
}
