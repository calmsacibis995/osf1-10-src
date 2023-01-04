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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: acc_ffb.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:04:25 $";
#endif
/*
 */

#include <sys/types.h>

#include "scrnintstr.h"

#include "ffbscrinit.h"

#include <sys/workstation.h>
#include "../ws/ws.h"


#ifdef SOFTWARE_MODEL
/* Point any HX or PXG screen at our model */
wsAcceleratorTypes types[] = {
	{"PMAGB-BA", ffb8_InitProc},
	{"PMAGB-BB", ffb8_InitProc},
        {"PMAG-DA ", ffbInitProc},
        {"PMAG-FA ", ffbInitProc},
        {"PMAG-CA ", ffbInitProc},
};

#else
/* Real live hardware, so vector everything correctly */
extern Bool sfbInitProc();
extern Bool ropInitProc();
extern Bool pxInitProc();
wsAcceleratorTypes types[] = {
	{"PMAGB-BA", sfbInitProc},
	{"PMAGB-BB", sfbInitProc},
	{"PMAG-RO ", ropInitProc},
	{"PMAG-JA ", ropInitProc},
/*	{"PMAG-CA ", pxInitProc }, */
/*	{"PMAG-DA ", pxInitProc }, */
/*	{"PMAG-FA ", pxInitProc }, */
	{"PMAGD-AA", ffb8_InitProc },   /*  8-bit sfb+ */
	{"PMAGD-BA", ffb32_InitProc},   /* 32-bit sfb+ */
	{"PMAGD-CA", ffb32_InitProc},   /* 32-bit sfb+ w/8 meg Z buffer */
	{"PMAGD-DA", ffb8_InitProc },   /*  8-bit sfb+, 2 screens */
};
#endif

int num_accelerator_types = sizeof (types) / sizeof (wsAcceleratorTypes);


/*
 * HISTORY
 */
