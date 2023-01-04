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
 * @(#)$RCSfile: man.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:01:05 $
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
 *    Miriam Nihart
 *
 * Date:
 *
 *    October 31, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 10, 1990
 *
 *    Add the definitions for the *_set_attributes to be used in the
 *    avl with the oid and value.  Tells whether it is a modiy, add,
 *    or remove type of SET.
 *
 *    Kathy Faust, June 11, 1990
 *
 *    Added MAN_C__NO_SUCH_ATTRIBUTE_GROUP and MAN_C__FILTER_USED_WITH_CREATE.
 *
 *    Miriam Amos Nihart, July 13th, 1990
 *
 *    Added MAN_C__END_OF_MIB in the reply return codes.
 *
 *    Miriam Amos Nihart, July 23rd, 1990
 *
 *    Added modifier field definitions for get, action, event, create, and delete.
 *
 *    Oscar Newkerk  October 4, 1990
 *
 *    Add the MAN_C_READ_ONLY error code for the routines that might attempt to
 *    modify an AVL created by moss_avl_index.
 *
 *    Miriam Amos Nihart, November 5th, 1990
 *
 *    Add MAN_C_EQUAL and MAN_C_NOT_EQUAL to man_status.
 *
 *    Miriam Amos Nihart, February 25th, 1991
 *
 *    Modify the SET modifier field values and add values to man_status and
 *    the reply codes as defined in the meeting 2-8-91 in LKG.
 *
 *    Miriam Amos Nihart, June 13th, 1991
 *
 *    Merge the reply codes and management return codes into on enumeration
 *    list.
 *
 *    Wim Colgate, August 21st, 1991
 *
 *    Added bit masks (MAN_M_xxxxx) for registration of interfaces
 *
 *    Michael T. Scott, August 29th, 1991
 *
 *    Added relational return-codes MAN_C_GREATER and MAN_C_LESS
 *
 *    Miriam Amos Nihart, November 15th, 1991.
 *
 *    Add return codes for mutexes.
 */

#ifndef MAN
#define MAN

/*
 *  General definitions
 */

#ifndef NIL
#define NIL 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef void PFV() ;

/*
 *  Management Return Codes
 */

typedef enum {
    MAN_C_SUCCESS                    = -1 ,

/*
 *  CMIP (ISO and DNA) error return values
 */

    MAN_C_NO_SUCH_CLASS              = 0 ,
    MAN_C_NO_SUCH_OBJECT_INSTANCE    = 1 ,
    MAN_C_ACCESS_DENIED              = 2 ,
    MAN_C_SYNC_NOT_SUPPORTED         = 3 ,
    MAN_C_INVALID_FILTER             = 4 ,
    MAN_C_NO_SUCH_ATTRIBUTE_ID       = 5 ,
    MAN_C_INVALID_ATTRIBUTE_VALUE    = 6 ,
    MAN_C_GET_LIST_ERROR             = 7 ,
    MAN_C_SET_LIST_ERROR             = 8 ,
    MAN_C_NO_SUCH_ACTION             = 9 ,
    MAN_C_PROCESSING_FAILURE         = 10 ,
    MAN_C_DUPLICATE_M_O_INSTANCE     = 11 ,
    MAN_C_NO_SUCH_REFERENCE_OBJECT   = 12 ,
    MAN_C_NO_SUCH_EVENT_TYPE         = 13 ,
    MAN_C_NO_SUCH_ARGUMENT           = 14 ,
    MAN_C_INVALID_ARGUMENT_VALUE     = 15 ,
    MAN_C_INVALID_SCOPE              = 16 ,
    MAN_C_INVALID_OBJECT_INSTANCE    = 17 ,
    MAN_C_MISSING_ATTRIBUTE_VALUE    = 18 ,
    MAN_C_CLASS_INSTANCE_CONFLICT    = 19 ,
    MAN_C_COMPLEXITY_LIMITATION      = 20 ,
    MAN_C_MISTYPED_OPERATION         = 21 ,
    MAN_C_NO_SUCH_INVOKE_ID          = 22 ,
    MAN_C_OPERATION_CANCELLED        = 23 ,
    MAN_C_INVALID_OPERATION          = 24 ,  /*  from NMF  */
    MAN_C_INVALID_OPERATOR           = 25 ,  /*  from NMF  */
    MAN_C_DIRECTIVE_NOT_SUPPORTED    = 32 ,
    MAN_C_ENTITY_CLASS_NOT_SUPPORTED = 33 ,
    MAN_C_INVALID_USE_OF_WILDCARD    = 34 ,
    MAN_C_CONSTRAINT_VIOLATION       = 36 ,
    MAN_C_WRITE_ONLY_ATTRIBUTE       = 37 ,
    MAN_C_READ_ONLY_ATTRIBUTE        = 38 ,
    MAN_C_DUPLICATE_ATTRIBUTE        = 39 ,
    MAN_C_DUPLICATE_ARGUMENT         = 40 ,
    MAN_C_REQUIRED_ARGUMENT_OMITTED  = 42 ,
    MAN_C_FILTER_INVALID_FOR_ACTION  = 43 ,
    MAN_C_INSUFFICIENT_RESOURCES     = 44 ,
    MAN_C_NO_SUCH_ATTRIBUTE_GROUP    = 45 ,
    MAN_C_FILTER_USED_WITH_CREATE    = 46 ,

/*
 *  Additional CMIP error values (currently not defined in a spec)
 */

    MAN_C_WILD_NOT_AT_LOWEST_LEVEL   = 1000 ,
    MAN_C_WILD_CLASS_WITH_FILTER     = 1001 ,
    MAN_C_WILD_INVALID_DIRECTIVE     = 1002 ,
    MAN_C_WILD_WITH_CREATE           = 1003 ,
    MAN_C_WILD_INVALID_GROUP         = 1004 ,
    MAN_C_SCOPE_TOO_COMPLEX          = 1010 ,
    MAN_C_SYNC_TOO_COMPLEX           = 1011 ,
    MAN_C_FILTER_TOO_COMPLEX         = 1012 ,

/*
 *  MOSS specific error return values
 */

    MAN_C_ALREADY_INITIALIZED        = 1200 ,
    MAN_C_BAD_PARAMETER              = 1201 ,
    MAN_C_FAILURE                    = 1202 ,
    MAN_C_HANDLE_NOT_BOUND           = 1203 ,
    MAN_C_HAS_ACTIVE_CHILDREN        = 1204 ,
    MAN_C_MO_TIMEOUT                 = 1205 ,
    MAN_C_MOLD_TIMEOUT               = 1206 ,
    MAN_C_NO_ELEMENT                 = 1207 ,
    MAN_C_NO_MOLD                    = 1208 ,
    MAN_C_NO_REPLY                   = 1209 ,
    MAN_C_NO_SUCH_PARENT_CLASS       = 1210 ,
    MAN_C_NOT_CONSTRUCTED            = 1211 ,
    MAN_C_NOT_INITIALIZED            = 1212 ,
    MAN_C_OBJECT_ALREADY_EXISTS      = 1213 ,
    MAN_C_PE_TIMEOUT                 = 1214 ,
    MAN_C_READ_ONLY                  = 1215 ,

/*
 *  MOSS conditional return codes
 */

    MAN_C_EQUAL                      = 1300 ,
    MAN_C_TRUE                       = 1301 ,
    MAN_C_NOT_EQUAL                  = 1302 ,
    MAN_C_FALSE                      = 1303 ,
    MAN_C_LESS                       = 1304 ,
    MAN_C_GREATER                    = 1305 ,

/*
 *  SNMP specific return codes
 */

    MAN_C_NOT_SUPPORTED              = 2000 ,
    MAN_C_END_OF_MIB                 = 2001 ,

/*
 *  EVD return codes
 */

    MAN_C_NO_QUEUE                   = 3000 ,
    MAN_C_BAD_HANDLE                 = 3001 ,
    MAN_C_ILLEGAL_EVENT_SIZE         = 3002 ,
    MAN_C_NO_EVENT                   = 3003 ,
    MAN_C_TRUNCATED                  = 3004 ,
    MAN_C_ILLEGAL_INSTANCE_STATE     = 3005 ,

/*
 *  MUTEX return codes
 */

    MAN_C_MUTEX_ALREADY_EXISTS       = 4001 ,
    MAN_C_MUTEX_EXISTS               = 4002 ,
    MAN_C_MUTEX_IS_LOCKED            = 4003 ,
    MAN_C_NO_MUTEX                   = 4004

} man_status ;

/*
 *  Management Flags
 */

#define MAN_C_NO_MORE_REPLIES 0
#define MAN_C_MORE_REPLIES    1

/*
 *  Definitions for the *_set_attributes call.  These values are passed
 *  in the avl in the modifier field to indicate what type of set is to
 *  be done with the attribute - modify, add, or remove.
 */

#define MAN_C_SET_MODIFY        0
#define MAN_C_SET_ADD           1
#define MAN_C_SET_REMOVE        2
#define MAN_C_SET_DEFAULT       3

/*
 *  Definitions for the modifier field for the other operations.
 */

#define MAN_C_GET_OPERATION     0
#define MAN_C_ACTION_OPERATION  0
#define MAN_C_EVENT_OPERATION   0
#define MAN_C_CREATE_OPERATION  0
#define MAN_C_DELETE_OPERATION  0

#define MAN_M_CONTAINMENT 0X1
#define MAN_M_LEXIGRAPHIC 0X2

#define MAN_M_MSI  0X1
#define MAN_M_SNMP 0X2

/*
 *  Management Operations
 */

#define MAN_C_GET        1
#define MAN_C_SET        2
#define MAN_C_CREATE     3
#define MAN_C_DELETE     4
#define MAN_C_ACTION     5

#endif /* man.h */
