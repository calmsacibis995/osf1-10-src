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
/* @(#)$RCSfile: dbg.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/09/07 16:04:51 $ */
/* dbg.h	1.3  com/cmd/uucp,3.1,9013 10/10/89 13:37:09 */
/* 
 * COMPONENT_NAME: UUCP dbg.h
 * 
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef dbg_h

extern char  dbgflag;
extern char *dbgfile;
extern char *dbgtime();
extern int   errno;

#define D1 if(dbgflag>=1)dbgmsg
#define D2 if(dbgflag>=2)dbgmsg
#define D3 if(dbgflag>=3)dbgmsg
#define D4 if(dbgflag>=4)dbgmsg
#define D5 if(dbgflag>=5)dbgmsg
#define D6 if(dbgflag>=6)dbgmsg
#define D7 if(dbgflag>=7)dbgmsg
#define D8 if(dbgflag>=8)dbgmsg
#define D9 if(dbgflag>=9)dbgmsg

#define dbg_h
#endif
