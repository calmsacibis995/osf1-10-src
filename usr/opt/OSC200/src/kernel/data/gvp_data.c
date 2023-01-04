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
 * derived from gvp_data.c	4.1  (ULTRIX)        7/2/90";
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Generic Vaxport Port Driver
 *
 *   Abstract:	This module contains  Generic Vaxport Port Driver( GVP )
 *		configurable variables.
 *
 *   Creator:	Todd M. Katz	Creation Date:	September 22, 1987
 *
 *   Modification History:
 *
 *   08-Jan-1988	Todd M. Katz
 *	Formated module, revised comments, and made GVP completely
 *	independent from underlying port drivers.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<io/dec/scs/sca.h>
#include		<io/dec/scs/scaparam.h>
#include		"bvpssp.h"
#include		"dssc.h"
#include		"hsc.h"
#include		"scsnet.h"

/* Generic Vaxport Driver Configuration Variables.
 */
u_long		gvp_queue_retry		/* Queuing failure retry account     */
		    = 10000;		/*  MAX:    -1; DEF: 10000;  MIN:  1 */
u_long		gvp_max_bds		/* Maximum number Buffer Descriptors */
		    = GVP_MAX_BDS;	/*  MAX: 32767; DEF:     50; MIN:  0 */
