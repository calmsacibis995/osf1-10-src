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
 * @(#)$RCSfile: ca_config.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:00:17 $
 */
/*
 * Copyright (c) Digital Equipment Corporation, 1991, 1992, 1993.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 */

/*
 * Default file locations for U*x Common Agent
 */

/** MIR defaults **/

#if defined(__osf__)
# define DEFAULT_DATABASE_PATH "/etc/eca/mir.dat"
#else
# if defined(ultrix) || defined(__ultrix)
# define DEFAULT_DATABASE_PATH "/etc/eca/mir.dat"
# else
# define DEFAULT_DATABASE_PATH "/etc/eca/mir.dat"
# endif
#endif

/** SNMP_PE defaults **/

/*
|  The following definition supplies the default value for the configuration
|  file name (stored in "config_file_name" in "big_picture") when it is not
|  supplied on the command line -- see routine "init_cmdline_args()" in
| "snmppe_init.c".
*/

#if defined(__osf__)
# define DEFAULT_SNMPPE_CONFIG_PATH "/etc/eca/snmp_pe.conf"
#else
# if defined(ultrix) || defined(__ultrix)
# define DEFAULT_SNMPPE_CONFIG_PATH "/etc/eca/snmp_pe.conf"
# else
# define DEFAULT_SNMPPE_CONFIG_PATH "/etc/eca/snmp_pe.conf"
# endif
#endif
