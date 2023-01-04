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
 * @(#)$RCSfile: err.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 22:07:29 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: err.h,v $ $Revision: 1.1.5.2 $ (OSF) $Date: 1993/06/10 22:07:29 $ */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.2  com/cmd/nls/err.h, cmdnls, bos320, 9125320 6/1/91 14:41:50
 */

#include "localedef_msg.h"

#define MALLOC(t,n)   ((t *)safe_malloc(sizeof(t)*(n),__FILE__,__LINE__))
extern void * safe_malloc(size_t, const char *, int);

extern void error(int,...);
extern void diag_error(int, ...);
extern char *msgstr(int);
extern void usage(int);
extern void yyerror(const char *);
#define INTERNAL_ERROR   error(ERR_INTERNAL, __FILE__, __LINE__)
