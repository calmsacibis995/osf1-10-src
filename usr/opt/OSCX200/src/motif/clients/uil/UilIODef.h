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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: UilIODef.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:34:04 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This include file defines the interface to the operating system
**	io facilities.  
**
**--
**/

#ifndef UilIODef_h
#define UilIODef_h


/*
**  Define a UIL File Control Block or FCB.
*/

#ifdef VMS

#include <rms.h>

#ifndef dsc$descriptor
#include <descrip.h>
#endif

typedef struct
{
    struct RAB  rab;
    struct FAB  fab;
    struct NAM  nam;
    boolean     v_position_before_get;
    z_key       last_key;
    char        expanded_name[ NAM$C_MAXRSS+1 ];
} uil_fcb_type;

status  sys$open   ( struct FAB *rms_fab);
status  sys$connect( struct RAB *rms_rab);
status  sys$get    ( struct RAB *rms_rab);
status  sys$close  ( struct FAB *rms_fab);

status  sys$filescan (
                struct dsc$descriptor * file_name,
                unsigned long * valuelst,
                unsigned long * fldflags );

#else

#ifndef ALPHA_BUG_FIX
#undef NULL
#endif /* ALPHA_BUG_FIX */
#include <stdio.h>
#ifndef NULL
#define NULL (void *)0
#endif

typedef struct  
{
    FILE	*az_file_ptr;
    char	*c_buffer;
    boolean	v_position_before_get;
    z_key	last_key;
    char	expanded_name[ 256 ];
} uil_fcb_type;

#endif
#endif /* UilIODef_h */
/* DON'T ADD STUFF AFTER THIS #endif */
