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
 * @(#)$RCSfile: moss_private.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:49 $
 */
/*
 *  static char *sccsid = "%W%	DECwest	%G%"
 */
/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1989, 1990, 1991, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    This the header file containing public definitions for MOSS.
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    October 31, 1989
 *
 * Revision History :
 *
 *    Oscar Newkerk Oct 4, 1990
 *
 *    Add the item_flag field to the avl_element stucture.  This will be used 
 *    to support moss_avl_find_item.  Also added the index_flag field to the iavl
 *    structure.  This will be used to indicat that an AVL is an index AVL that 
 *    points to a buffer created by moss_avl_to_buf.  These index AVLs are read only.
 *
 *    Kathy Faust, July 18th 1990
 *
 *    Removed state1 and state2 fields (unused) from iavl.
 *
 *    Miriam Amos Nihart, October 16th, 1991.
 *
 *    Put in prototyping.
 *
 *    Miriam Amos Nihart, November 8th, 1991.
 *
 *    Add a pointer to a mutex to the avl header.
 *
 *    Miriam Amos Nihart, December 3rd, 1991.
 *
 *    Add macros for malloc, free, and cma_init check.
 */

#ifndef MOSS_PRIVATE
#define MOSS_PRIVATE

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
#define PROTOTYPE(args) args
#else
#define PROTOTYPE(args) ()
#endif

#include "moss.h"
#ifndef MAN_NIL
#include "extern_nil.h"
#endif /* MAN_NIL */
#ifdef RPCV2
#include "pthread.h"
#endif /* RPCV2 */

/*
 * Define longword alignment
 */

#define ALIGN(val) ((((long)(val)&(~3))==(long)(val)) ? \
                           (long)(val) : (((long)(val)&(~3)) + 4))

/*
 * Define malloc and free macros
 */

#ifndef MOSS_MALLOC
#ifdef RPCV2
#define MOSS_MALLOC( address, structure, size )              \
{                                                            \
    cma_lock_global( ) ;                                     \
    address = ( structure * )malloc( size ) ;                \
    cma_unlock_global( ) ;                                   \
}
#else
#define MOSS_MALLOC( address, structure, size )              \
    address = ( structure * )malloc( size ) ;
#endif /* RPCV2 */
#endif /* MOSS_MALLOC */

#ifndef MOSS_FREE
#ifdef RPCV2
#define MOSS_FREE( address )                                 \
{                                                            \
    cma_lock_global( ) ;                                     \
    free( address ) ;                                        \
    cma_unlock_global( ) ;                                   \
}
#else
#define MOSS_FREE( address )                                 \
    free( address ) ;
#endif /* RPCV2 */
#endif /* MOSS_FREE */

/*
 * cma_init check macro
 */

#ifndef MOSS_INIT_CHECK
#ifdef RPCV2
#define MOSS_INIT_CHECK()                    \
    if( moss_init_global == FALSE )          \
    {                                        \
        moss_init_global = TRUE ;            \
        cma_init( ) ;                        \
    }
#endif /* RPCV2 */
#endif /* MOSS_INIT_CHECK */
/*
 * avl element internal representation
 */

typedef struct _avl_element
{
    struct _avl_element *next_avl ;
    struct _avl_element *prev_avl ;
    object_id oid ;
    octet_string oct ;
    unsigned int tag ;
    unsigned int modifier ;
    unsigned int item_flag ;
    int terminator ;
} avl_element ;

/*
 * avl handle internal represnetation
 */

typedef struct _avl
{
    avl_element *first_avl ;
    avl_element *curr_avl ;
    int construction_level ; 
    int initialized ;
    int index_flag ;
#ifdef RPCV2
    pthread_mutex_t *mutex_handle ;
#endif /* RPCV2 */
} iavl ;

extern
avl_element *
moss_avl_element PROTOTYPE((
iavl *
)) ;

#endif /* end of moss_private.h */
