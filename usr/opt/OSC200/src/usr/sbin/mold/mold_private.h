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
 * @(#)$RCSfile: mold_private.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:08:49 $
 */
/*
 *  static char *sccsid = "%W%	DECwest	%G%" ;
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
 *    This the header file containing public definitions.
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
 *    Jim Teague, March 20, 1991
 *
 *    Changes for port from NCS to DCE RPC.
 * 
 *    Miriam Amos Nihart, July 31st, 1991.
 *
 *    Remove duplicate declaration for hash_table_hit().
 *
 *    Wim Colgate, August 21st, 1991.
 *
 *    Modified the data structure: 'object' to support interface registration and pid.
 *    Also added some empty prototypes for routines added to MOLD.
 *
 *    Kathy Faust, Sept 6th, 1991
 *
 *    Added MAN_C_MOLD_MAX_CALLS for specifying maximum concurrent calls
 *    supported by MOLD.
 *
 *    Miriam Amos Nihart, October 16th, 1991.
 *
 *    Put in prototyping.
 *
 *    Wim Colgate, November 18th, 1991.
 *
 *    Added some routines to the prototype definitions.
 */

#ifndef MOLD_PRIVATE
#define MOLD_PRIVATE

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
#define PROTOTYPE(args) args
#else
#define PROTOTYPE(args) ()
#endif

/* 
 *  This header file only exists on ULTRIX 4.0.
 *
 *  #include <stdlib.h>
 */

#define MAXERRMSG 300
#define LINEBUFSIZE 250

#define MAN_C_MOLD_MAX_CALLS 1
#define MAN_C_HASH_PRIME 1048583
#define MAN_C_HASH_VALUE (8 * 256) /* 8192 hash table entries, 8K longwords for table */

#define MAN_C_LESS_THAN (-1)
#define MAN_C_EQUAL_TO (0)
#define MAN_C_GREATER_THAN (1)

#define MAN_C_UP (1)
#define MAN_C_DOWN (-1)
#define MAN_C_NEXT (0) 

#define MAN_C_SKIP (0) 
#define MAN_C_ADD (1)

#define MAN_C_DEREGISTERED (0)
#define MAN_C_REGISTERED (1)
#define MAN_C_PREREGISTERED (2)

#define CONTAINING_RECORD_BY_NAME( curr_address, type, curr_field ) \
    (type *)((long)curr_address - (long)(&((type *)0)->curr_field))

#define CONTAINING_RECORD_BY_OFFSET( curr_address, type, offset ) \
    (type *)((long)curr_address - (long)(offset))

#define CONTAINED_FIELD_BY_OFFSET( curr_address, type, offset ) \
    (type *)((long)curr_address + (long)offset)

#define BYTE_OFFSET( type, field_name ) \
    (long)(&((type *)0)->field_name)

#ifdef NOIPC
#define MALLOC( ptr, type, byte_size )  \
    if (( ptr = (type) malloc( byte_size )) == NULL) \
    {                                                \
        fprintf(stderr, MSG(mold_msg021, "MOLD: out of memory. \n")) ;          \
        abort() ;                                    \
    }
#else
#ifdef RPCV2
#define MALLOC( ptr, type, byte_size )               \
{                                                    \
    cma_lock_global( ) ;                             \
    ptr = ( type )malloc( byte_size ) ;              \
    cma_unlock_global( ) ;                           \
    if( ptr == NULL )                                \
    {                                                \
        fprintf(stderr, MSG(mold_msg022, "MOLD: out of memory. \n")) ;          \
        fprintf(stderr, MSG(mold_msg023, "MOLD: Attempting deregistration from rpcd\n") ) ; \
        mold_signal_handler() ;                      \
        abort() ;                                    \
     }                                               \
} 
#else
#define MALLOC( ptr, type, byte_size )               \
    if (( ptr = (type) malloc( byte_size )) == NULL) \
    {                                                \
        fprintf(stderr, MSG(mold_msg024, "MOLD: out of memory. \n")) ;          \
        fprintf(stderr, MSG(mold_msg025, "MOLD: Attempting deregistration from rpcd\n") ) ; \
        mold_signal_handler() ; \
        abort() ;                                    \
    }
#endif /* RPCV2 */
#endif /* NOIPC */

#ifdef RPCV2
#define FREE( ptr )                                  \
{                                                    \
    cma_lock_global( ) ;                             \
    if ( ptr ) free( ptr ) ;                         \
    cma_unlock_global( ) ;                           \
}
#else    
#define FREE( ptr )                                  \
    if ( ptr ) free( ptr ) ;
#endif /* RPCV2 */

/* 
 * These are synchronization macros. In the Ultrix implementation, they
 * are merely placeholders.
 */

#define READ_LOCK()
#define READ_UNLOCK()
#define WRITE_LOCK()
#define WRITE_UNLOCK()

/*
 * The family relation structure embodies the basis of the containment
 * hierarchy. The parent points to one child only. This child is
 * considered to be the 'least ' child insofar as its object id
 * collates lowest. The sibling pointers (fsibling and bsibling) point
 * to 'forward' siblings or 'backward' siblings based on collating
 * value. Note that the 'last' child points back to the first (or least)
 * child. The ONLY way to tell that we are wrapping around in our doubly
 * linked list is that there is a disparity between the two object
 * id's.
 */

typedef struct family_relation
{
    struct family_relation *parent ;
    struct family_relation *children ;
    struct family_relation *fsibling ;
    struct family_relation *bsibling ;
} family_relation_t ;

/*
 * This is the doubly linked queue structure for queue manipulations
 */

typedef struct queue
{
    struct queue *flink ;
    struct queue *blink ;
} queue_t ;

/*
 * This is the object data structure. The next_hash field is used when
 * more than one object fits in a hash bucket.
 */

typedef struct object
{
    struct object *next_hash;
    struct family_relation relation ;
    struct object_id oid ;
    struct object_id parent_oid ;
    management_handle man_handle ;
					/*                               */
    unsigned int state_flag ;		/* This is temporary for EFT to  */
					/* provide populate MOLD feature */
					/*				 */
    unsigned int supported_interface ;
    queue_t lexi_q ;
} object_t ;

/*
 * Define some external and forward routines.
 */

void
mold_log PROTOTYPE((
char * ,
int
)) ;

static
void
initialize_mold( ) ;

object_t *
find_object PROTOTYPE((
object_id *
)) ;

object_t *
create_object PROTOTYPE((
object_id *
)) ;

void 
insert_containment PROTOTYPE((
family_relation_t ** ,
family_relation_t * ,
family_relation_t * ,
object_id *
)) ;

man_status
remove_object PROTOTYPE((
object_t *
)) ;

static
man_status
remove_containment PROTOTYPE((
family_relation_t ** ,
object_t *
)) ;

man_status
find_scoped_objects PROTOTYPE((
object_t * ,
int ,
avl *
)) ;

static int
compare_oid PROTOTYPE((
object_id * ,
object_id *
)) ;

static
unsigned int
hash PROTOTYPE((
object_id *
)) ;

static
object_t *
hash_table_hit PROTOTYPE((
int ,
object_id * ,
object_t **
)) ;

static
unsigned int
convert_oid PROTOTYPE((
object_id *
)) ;

static object_t *
allocate_object( ) ;

void
insert_lexi PROTOTYPE((
queue_t * ,
queue_t * ,
object_id *
)) ;

man_status
remove_lexi PROTOTYPE((
queue_t * ,
object_t *
)) ;

void
queue_insert PROTOTYPE((
queue_t * ,
queue_t *
)) ;

queue_t *
queue_remove PROTOTYPE((
queue_t * ,
queue_t *
)) ;

void
queue_init() ;

man_status
register_containment PROTOTYPE((
management_handle * ,
object_id * ,
object_id * ,
object_t **
)) ;

static
man_status
register_lexi PROTOTYPE((
management_handle * ,
object_id * ,
object_t **
)) ;

#endif /* end of file mold_private.h */
