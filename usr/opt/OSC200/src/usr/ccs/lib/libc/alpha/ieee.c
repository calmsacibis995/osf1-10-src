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
static char *rcsid = "@(#)$RCSfile: ieee.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/08 01:24:20 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak ieee_get_fp_control = __ieee_get_fp_control
#pragma weak ieee_get_state_at_signal = __ieee_get_state_at_signal
#pragma weak ieee_ignore_state_at_signal = __ieee_ignore_state_at_signal
#pragma weak ieee_set_fp_control = __ieee_set_fp_control
#pragma weak ieee_set_state_at_signal = __ieee_set_state_at_signal
#endif
#endif
#include <sys/types.h>
#include <alpha/hal_sysinfo.h>

void
ieee_set_fp_control(long mask)
{

	if(setsysinfo(SSI_IEEE_FP_CONTROL, &mask,0,0,0) < 0) {
		perror("setsysinfo - SSI_IEEE_FP_CONTROL");
		return;
	}
}

long
ieee_get_fp_control()
{
	long arg;
	if(getsysinfo(GSI_IEEE_FP_CONTROL, &arg,0,0,0) < 0) {
		perror("getsysinfo - GSI_IEEE_FP_CONTROL");
		return(0);
	}
	return (arg);
}

void
ieee_set_state_at_signal(ulong_t fp_control, ulong_t fpcr)
{
	ulong_t parg[2];
	parg[0] = fp_control;
	parg[1] = fpcr;
	if(setsysinfo(SSI_IEEE_STATE_AT_SIGNAL, parg,0,0,0) < 0) {
		perror("setsysinfo - SSI_IEEE_STATE_AT_SIGNAL");
		return;
	}
}

int
ieee_get_state_at_signal(ulong_t *fp_control, ulong_t *fpcr)
{
        ulong_t parg[3];
	if(getsysinfo(GSI_IEEE_STATE_AT_SIGNAL, parg,0,0,0) < 0) {
		perror("getsysinfo - GSI_IEEE_STATE_AT_SIGNAL");
		return(0);
	}
	*fp_control = parg[0];
	*fpcr = parg[1];
	return(parg[2]);
}

void
ieee_ignore_state_at_signal()
{
	if(setsysinfo(SSI_IEEE_IGNORE_STATE_AT_SIGNAL, 0,0,0,0) < 0) {
		perror("setsysinfo - SSI_IEEE_IGNORE_STATE_AT_SIGNAL");
		return;
	}
}
