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
static char *rcsid = "@(#)$RCSfile: netaf_config.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 17:50:00 $";
#endif

/*
 *	netaf_config.c - Allows domain initialization for network
 *	subsystems supplied separately from the base operating system.
 *	This file is compiled and loaded at kernel build time
 *
 */

#if MACH
#include <sys/sysconfig.h>
#endif

#ifdef __STDC__

#ifndef CONST
#define CONST const
#endif /* CONST */

#if defined(MACH)
#define	CONFIG_ARGS	int , void *, int , void *, int
#else
#define CONFIG_ARGS	void
#endif /* MACH */

#else

#define CONFIG_ARGS
#ifndef CONST
#define CONST
#endif

#endif  /* __STDC__ */

/*
 * To have a new socket domain initialised, declare the domain's
 * config function and add the domain's config function name to 
 * the initialisation array.
 *
 * The following example uses 'xyz' as the domain to be initialised.
 *
 * Declare the domain's config function by adding the declaration
 * after the line
 *	typedef int (* domain_init_fp_t)(CONFIG_ARGS);
 * e.g
 *  	extern int xyz_config(CONFIG_ARGS);
 *
 * Add the domain's config function name to the initialization
 * array, e.g.: 
 * 	#if !defined(XYZ) 
 * 	#include <xyz.h>
 * 	#if	XYZ_DYNAMIC
 * 	#undef  XYZ
 * 	#define	XYZ	0
 * 	#endif
 * 	#endif
 * 	#if	!XYZ_DYNAMIC && XYZ
 * 		xyz_config,
 * 	#endif
 *
 *	XYZ will be defined if the kernel config file specifies this
 *	as an option. XYZ_DYNAMIC indicates a dynamically loadable
 *	option which is deferred to the config manager.
 */

typedef int (* domain_init_fp_t)(CONFIG_ARGS);

CONST domain_init_fp_t init_array[] = {

	NULL		/* must be at end of array, used by function below */
};

/* 
 * This routine is called by netinit() at system startup and is 
 * responsible for calling the xxx_config function for each socket domain. 
 */

void netinit_domains()
{
    domain_init_fp_t *fp;

    for (fp = init_array; *fp != NULL; fp++) {
#if	MACH
	(**fp)(SYSCONFIG_CONFIGURE, (void *)NULL, 0, (void *)NULL, 0);
#else
	(**fp)();
#endif
    }
}

