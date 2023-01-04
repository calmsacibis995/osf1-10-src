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
/* Mwm Help internal info */

/*******************************************************************/

#define k_mwm_help_wm_topic "mwm_overview"
#define k_mwm_help_version_topic "mwm_version"
#define k_mwm_help_terms_topic "mwm_terms"
#define k_mwm_help_shortcuts_topic "mwm_shortcuts"

#ifdef HYPERHELP
static Opaque *mwm_hyperhelp_context;
#else
static XmString mwm_help_lib = NULL;
#endif
static XtInitProc mwm_help_xtinitproc = NULL;
/* Help files */
#ifdef VMS
#ifdef HYPERHELP
#define k_mwm_hyperhelp_path "decw$mwm"
#else
static char mwm_help_path[] = { "decw$mwm.hlb" };
#endif

#else
#ifdef HYPERHELP
#define k_mwm_hyperhelp_path "mwm"
#else
static char mwm_help_path[] = { "mwm" };
#endif
#endif

/*******************************************************************/
