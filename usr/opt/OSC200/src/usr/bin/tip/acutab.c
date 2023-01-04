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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: acutab.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/30 18:39:32 $";
#endif
/*
acutab.c	1.3  com/cmd/tip,3.1,9013 10/15/89 10:41:20";
 */
/* 
 * COMPONENT_NAME: UUCP acutab.c
 * 
 * FUNCTIONS: MSGSTR 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "acutab.c	5.2 (Berkeley) 4/3/86"; */

#include "tip.h"

extern int df02_dialer(), df03_dialer(), df_disconnect(), df_abort(),
	   biz31f_dialer(), biz31_disconnect(), biz31_abort(),
	   biz31w_dialer(),
	   biz22f_dialer(), biz22_disconnect(), biz22_abort(),
	   biz22w_dialer(),
	   ven_dialer(), ven_disconnect(), ven_abort(),
	   hay_dialer(), hay_disconnect(), hay_abort(),
	   cour_dialer(), cour_disconnect(), cour_abort(),
	   v3451_dialer(), v3451_disconnect(), v3451_abort(),
	   v831_dialer(), v831_disconnect(), v831_abort(),
	   dn_dialer(), dn_disconnect(), dn_abort();
           dn_dialer(), dn_disconnect(), dn_abort(),
           schol_dialer(), schol_disconnect(), schol_abort(),
           dmcl_dialer(), dmcl_disconnect(), dmcl_abort();


acu_t acutable[] = {
#if BIZ1031
	"biz31f", biz31f_dialer, biz31_disconnect,	biz31_abort,
	"biz31w", biz31w_dialer, biz31_disconnect,	biz31_abort,
#endif
#if BIZ1022
	"biz22f", biz22f_dialer, biz22_disconnect,	biz22_abort,
	"biz22w", biz22w_dialer, biz22_disconnect,	biz22_abort,
#endif
#if DF02
	"df02",	df02_dialer,	df_disconnect,		df_abort,
#endif
#if DF03
	"df03",	df03_dialer,	df_disconnect,		df_abort,
#endif
#if DN11
	"dn11",	dn_dialer,	dn_disconnect,		dn_abort,
#endif
#ifdef VENTEL
	"ventel",ven_dialer,	ven_disconnect,		ven_abort,
#endif
#ifdef HAYES
	"hayes",hay_dialer,	hay_disconnect,		hay_abort,
	"hayes-V",hay_dialer,	hay_disconnect,		hay_abort,
#endif
#ifdef COURIER
	"courier",cour_dialer,	cour_disconnect,	cour_abort,
#endif
#ifdef DECMOD
        "scholar",schol_dialer, schol_disconnect,       schol_abort,
        "dmcl",dmcl_dialer,     dmcl_disconnect,        dmcl_abort,
#endif
#ifdef V3451
#ifndef V831
	"vadic",v3451_dialer,	v3451_disconnect,	v3451_abort,
#endif
	"v3451",v3451_dialer,	v3451_disconnect,	v3451_abort,
#endif
#ifdef V831
#ifndef V3451
	"vadic",v831_dialer,	v831_disconnect,	v831_abort,
#endif
	"v831",v831_dialer,	v831_disconnect,	v831_abort,
#endif
	0,	0,		0,			0
};

