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
 *	@(#)$RCSfile: ypdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:20:09 $
 */
/*
 */


/*
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

/*
 * ypdefs.h
 * Special, internal keys to NIS maps.  These keys are used
 * by various maintain functions of the NIS and invisible
 * to NIS clients.  By definition, any key beginning with yp_prefix is
 * an internal key.
 */

#define USE_YP_PREFIX \
	static char yp_prefix[] = "YP_"; \
	static int  yp_prefix_sz = sizeof (yp_prefix) - 1;

#define USE_YP_MASTER_NAME \
	static char yp_master_name[] = "YP_MASTER_NAME"; \
	static int  yp_master_name_sz = sizeof (yp_master_name) - 1;
#define MAX_MASTER_NAME 256

#define USE_YP_LAST_MODIFIED \
	static char yp_last_modified[] = "YP_LAST_MODIFIED"; \
	static int  yp_last_modified_sz = sizeof (yp_last_modified) - 1;
#define MAX_ASCII_ORDER_NUMBER_LENGTH 10

#define USE_YP_INPUT_FILE \
	static char yp_input_file[] = "YP_INPUT_FILE"; \
	static int  yp_input_file_sz = sizeof (yp_input_file) - 1;

#define USE_YP_OUTPUT_NAME \
	static char yp_output_file[] = "YP_OUTPUT_NAME"; \
	static int  yp_output_file_sz = sizeof (yp_output_file) - 1;

#define USE_YP_DOMAIN_NAME \
	static char yp_domain_name[] = "YP_DOMAIN_NAME"; \
	static int  yp_domain_name_sz = sizeof (yp_domain_name) - 1;

#define USE_YP_SECURE \
	static char yp_secure[] = "YP_SECURE"; \
	static int  yp_secure_sz = sizeof (yp_secure) - 1;

#define USE_YP_INTERDOMAIN \
	static char yp_interdomain[] = "YP_INTERDOMAIN"; \
	static int  yp_interdomain_sz = sizeof (yp_interdomain) - 1;

/*
 * Definitions of where the NIS servers keep their databases.
 * These are really only implementation details.
 */

#define USE_YPDBPATH \
	static char ypdbpath[] = "/var/yp"; \
	static int  ypdbpath_sz = sizeof (ypdbpath) - 1;

#define USE_DBM \
	static char dbm_dir[] = ".dir"; \
	static char dbm_pag[] = ".pag";

