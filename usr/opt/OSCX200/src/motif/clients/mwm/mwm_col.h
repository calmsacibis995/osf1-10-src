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
/* Mwm color customize includes */

/*******************************************************************/

    /* External declarations */

#ifdef _NO_PROTO
extern int mwm_col_init();
extern int mwm_col_index_get();
extern int mwm_col_did_get();
extern void mwm_col_mix_mess_set();
extern void mwm_col_get();
extern void mwm_col_auto_chec();
extern void mwm_col_sample_crea();
extern void mwm_col_mix_set();
extern int mwm_col_mix_ok();
#else
extern int mwm_col_init();
extern int mwm_col_index_get( int fid );
extern int mwm_col_did_get( int index );
extern void mwm_col_mix_mess_set( Widget wid );
extern void mwm_col_get( Widget wid, char *resource, XColor *value );
extern void mwm_col_auto_chec( Widget wid, int fid );
extern void mwm_col_sample_crea( Widget wid, int fid );
extern void mwm_col_mix_set( Widget wid );
extern int mwm_col_mix_ok( Widget wid, int *tag, unsigned int *reason );
#endif /* _NO_PROTO */

/*******************************************************************/
