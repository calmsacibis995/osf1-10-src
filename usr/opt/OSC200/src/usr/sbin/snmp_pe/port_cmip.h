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
 * @(#)$RCSfile: port_cmip.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:23:18 $
 */
/*
**  Copyright (c) Digital Equipment Corporation, 1991, 1992
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*/

/* The names of the included files below were changed to avoid conflicts
   with MCC files beginning "mcc_".
*/
#include "snmppe_descrip.h"
#include "snmppe_interface_def.h"  

#define SUCCESS(status) ((status == MCC_S_NORMAL))

/* DEC/CMS REPLACEMENT HISTORY, Element PORT_CMIP.H*/
/* *3     7-JUN-1991 09:02:48 SANKAR " status "*/
/* *2    10-MAY-1991 10:53:37 SANKAR " needed changes to allow mcc_kernel"*/
/* *1    14-DEC-1990 14:38:09 SANKAR "new cms entry"*/
/* DEC/CMS REPLACEMENT HISTORY, Element PORT_CMIP.H*/
