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
/* Mwm customization callbacks */

/*******************************************************************/

static MrmRegisterArg mwm_cust_cblist[] = 
  {                      
    { "mwm_cust_rese", (caddr_t)mwm_cust_rese },
    { "mwm_cust_defa", (caddr_t)mwm_cust_defa },
    { "mwm_cust_apply", (caddr_t)mwm_cust_apply }
  };

static MrmCount mwm_cust_cbnum = ( sizeof mwm_cust_cblist/sizeof mwm_cust_cblist[ 0 ] );

/*******************************************************************/
