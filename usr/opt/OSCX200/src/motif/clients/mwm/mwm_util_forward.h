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
/* Mwm forward declarations */

/*******************************************************************/

#ifdef _NO_PROTO
void mwm_subres_get();
int mwm_uil_init();
int mwm_init();
int mwm_alloc();
int mwm_str_0();
void mwm_str_ws();
Boolean mwm_str_eq();
Boolean mwm_str_find();
char *mwm_str_get();
int mwm_watch_set();
int mwm_res_get();
int mwm_num_get();
void mwm_set();
void mwm_get();
void mwm_res_set();
#else
void mwm_subres_get( WmScreenData *pSD, XtResource res_list[], int num_res,
                     caddr_t addr );
int mwm_uil_init();
int mwm_init( int screen, int screen_num );
int mwm_alloc( void *block, int size, char *message );
int mwm_str_0( char *string );
void mwm_str_ws( char *string );
Boolean mwm_str_eq( char *string, char *value );
Boolean mwm_str_find( char *string, char *value );
char *mwm_str_get( char *string, char *value );
int mwm_watch_set( int flag );
int mwm_res_get( WmScreenData *pSD, XrmDatabase database, char *resource,
                 int type, void *value, void *default_value );
int mwm_num_get( Widget wid, int did, int fid, int *num, char *string );
void mwm_set( Widget wid, char *resource, void *value );
void mwm_get( Widget wid, char *resource, void *value );
void mwm_res_set( XrmDatabase *database, char *resource, 
                       int type, void *value );
#endif /* _NO_PROTO */

/*******************************************************************/
