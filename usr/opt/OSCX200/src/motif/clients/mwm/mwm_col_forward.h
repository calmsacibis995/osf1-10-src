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
/* Mwm color customize forward declarations */

/*******************************************************************/

#ifdef _NO_PROTO
int mwm_col_init();
void mwm_col_range();
double mwm_col_value();
void mwm_rgbtohls();
void mwm_hlstorgb();
int mwm_col_auto_set();
int mwm_col_index_get();
int mwm_col_did_get();
void mwm_col_get();
void mwm_col_mix_auto_chec();
void mwm_col_auto_chec();
int mwm_col_sample_fid();
void mwm_col_mix_up();
void mwm_col_reg();
void mwm_col_sample_init();
void mwm_col_sample_crea();
void mwm_col_expo();
void mwm_col_mix_set();
void mwm_col_ok();
int mwm_col_mix_ok();
#else
int mwm_col_init();
void mwm_col_range( double r, double g,  double b, double *min, double *max );
double mwm_col_value( double n1, double n2, double hue ); 
void mwm_rgbtohls( double red, double green, double blue, double *hue,
                    double *light, double *sat );
void mwm_hlstorgb( double dhue, double dlight, double dsat, 
                   unsigned short *red, unsigned short *green, 
                   unsigned short *blue );
int mwm_col_auto_set( Widget wid, int top_x, int bot_x, int back_x, int top_fid,
                      int bot_fid, int sample );
void mwm_col_mix_mess_set( Widget wid );
int mwm_col_index_get( int fid );
int mwm_col_did_get( int index );
void mwm_col_get( Widget wid, char *resource, XColor *value );
void mwm_col_mix_auto_chec( Widget wid, int fid );
void mwm_col_auto_chec( Widget wid, int fid );
int mwm_col_sample_fid( int sample );
void mwm_col_mix_up( Widget wid, int *tag, unsigned int *reason );
void mwm_col_reg( Widget wid, int sample, int fid, Pixel col );
void mwm_col_sample_init( Widget wid );
void mwm_col_sample_crea( Widget wid, int fid );
void mwm_col_expo( Widget wid, int *tag, 
                        XmAnyCallbackStruct *callback_data );
void mwm_col_mix_set( Widget wid );
int mwm_col_mix_ok( Widget wid, int *tag, DXmColorMixCallbackStruct *cbs );
#endif /* _NO_PROTO */

/*******************************************************************/
