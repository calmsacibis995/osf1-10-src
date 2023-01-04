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
static char *rcsid = "@(#)$RCSfile: libloader.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/09/29 17:04:20 $";
#endif
/*
 * libloader.c
 * Multiplex all calls into _rld_new_interface.
 * _rld_new_interface has been extended in rld
 * to know about all these functions, which are
 * here to support OSF and TIN functionality.
 *
 */
#include <sys/types.h>
#include <sys/file.h>
#include <loader.h>
#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>
#include <rld_interface.h>

int
ldr_sbrk(int arg1, char **arg2)
{
    return((int)_rld_new_interface(_RLD_LDR_SBRK, arg1, arg2));
}

int
ldr_brk(char *arg1)
{
    return((int)_rld_new_interface(_RLD_LDR_BRK, arg1));
}

int
ldr_context_atexit(ldr_context_t arg1)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_ATEXIT, arg1));
}

#ifdef __tin_compat__

int
ldr_context_remove(ldr_context_t arg1, char *arg2)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_REMOVE, arg1, arg2));
}

int
ldr_context_install(ldr_context_t arg1, char *arg2)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_INSTALL, arg1, arg2));
}

int
ldr_context_inq_region(ldr_context_t arg1,
		       ldr_module_t arg2,
		       ldr_region_t arg3,
		       ldr_region_info_t *arg4,
		       size_t arg5,
		       size_t *arg6)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_INQ_REGION,
				   arg1, arg2, arg3, arg4, arg5, arg6));
}

int
ldr_context_inq_module(ldr_context_t arg1,
		       ldr_module_t arg2,
		       ldr_module_info_t *arg3,
		       size_t arg4,
		       size_t *arg5)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_INQ_MODULE,
				   arg1, arg2, arg3, arg4, arg5));
}

int
ldr_context_next_module(ldr_context_t arg1,
			ldr_module_t *arg2)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_NEXT_MODULE, arg1, arg2));
}

int
ldr_context_lookup_package(ldr_context_t arg1,
			   char *arg2,
			   char *arg3,
			   void **arg4)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_LOOKUP_PACKAGE,
				   arg1, arg2, arg3, arg4));
}

int
ldr_context_lookup(ldr_context_t arg1,
		   ldr_module_t arg2,
		   char *arg3,
		   void **arg4)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_LOOKUP,
				   arg1, arg2, arg3, arg4));
}

int
ldr_context_unload(ldr_context_t arg1,
		   ldr_module_t arg2)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_UNLOAD, arg1, arg2));
}

int
ldr_context_get_entry_pt(ldr_context_t arg1,
			 ldr_module_t arg2,
			 ldr_entry_pt_t *arg3)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_GET_ENTRY_PT,
				   arg1, arg2, arg3));
}

int
ldr_context_load(ldr_context_t arg1,
		 char *	arg2,
		 ldr_load_flags_t arg3,
		 ldr_module_t *	arg4)
{
    return((int)_rld_new_interface(_RLD_LDR_CONTEXT_LOAD,
				   arg1, arg2, arg3, arg4));
}

#endif /* __tin_compat__ */
