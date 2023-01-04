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
/* Mwm dialog callbacks */

/*******************************************************************/

static MrmRegisterArg mwm_dialog_cblist[] = 
  {                      
    { "mwm_dialog_ok", (caddr_t)mwm_dialog_ok },
    { "mwm_dialog_cancel", (caddr_t)mwm_dialog_cancel }, 
    { "mwm_dialog_field_crea", (caddr_t)mwm_dialog_field_crea },
    { "mwm_dialog_field_set", (caddr_t)mwm_dialog_field_set }
  };

static MrmCount mwm_dialog_cbnum = ( sizeof mwm_dialog_cblist/sizeof mwm_dialog_cblist[ 0 ] );

/*******************************************************************/
