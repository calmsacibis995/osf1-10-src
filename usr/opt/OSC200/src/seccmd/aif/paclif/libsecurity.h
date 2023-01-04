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
 * @(#)$RCSfile: libsecurity.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:12:08 $
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	libsecurity.h,v $
 * Revision 1.1.1.1  92/05/12  01:45:00  devrcs
 *  *** OSF1_1B29 version ***
 * 
 * Revision 1.1.2.3  1992/04/05  18:19:49  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:36  marquard]
 *
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1991, SecureWare Inc.
 * ALL RIGHTS RESERVED
 * @(#)libsecurity.h	1.1 11:17:08 11/8/91 SecureWare
 */

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "libsec_msg.h"
#define MSGSTR(n,s) NLgetamsg(MF_LIBSEC,MS_LIBSEC,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif
