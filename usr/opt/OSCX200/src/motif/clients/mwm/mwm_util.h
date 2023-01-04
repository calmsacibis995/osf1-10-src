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
/* Mwm includes */

/*******************************************************************/

    /* External declarations */

#ifdef _NO_PROTO
void mwm_subres_get();
extern int mwm_uil_init();
extern int mwm_init();
extern int mwm_alloc();
extern int mwm_str_0();
extern Boolean mwm_str_eq();
extern Boolean mwm_str_find();
extern char *mwm_str_get();
extern int mwm_watch_set();
extern int mwm_res_get();
extern int mwm_num_get();
extern void mwm_set();
extern void mwm_get();
extern void mwm_res_set();
#else
extern void mwm_subres_get( WmScreenData *pSD, XtResource res_list[], int num_res,
                            caddr_t addr );
extern int mwm_uil_init();
extern int mwm_init( int screen, int screen_num );
extern int mwm_alloc( void *block, int size, char *message );
extern int mwm_str_0( char *string );
extern Boolean mwm_str_eq( char *string, char *value );
extern Boolean mwm_str_find( char *string, char *value );
extern char *mwm_str_get( char *string, char *value );
extern int mwm_watch_set( int flag );
extern int mwm_res_get( WmScreenData *pSD, XrmDatabase database, char *resource,
                        int type, void *value, void *default_value );
extern int mwm_num_get( Widget wid, int did, int fid, int *num, char *string );
extern void mwm_set( Widget wid, char *resource, void *value );
extern void mwm_get( Widget wid, char *resource, void *value );
extern void mwm_res_set( XrmDatabase *database, char *resource, 
                       int type, void *value );
#endif /* _NO_PROTO */

/********************************/

    /* Global macros and variables */

#ifndef MIN
#define MIN( arg1, arg2 ) ( ( arg1 >= arg2 ) ? arg2 : arg1 )
#endif
#ifndef MAX
#define MAX( arg1, arg2 ) ( ( arg1 <= arg2 ) ? arg2 : arg1 )
#endif
             
/* Index into array (if one screen which is set to screen 1; index will be 0).
   If multiscreen, index will be screen number. */
#define WID_SCREEN ( ( wmGD.numScreens == 1 ) ? 0 : XScreenNumberOfScreen( XtScreen( wid )))
/* Screen number */
#define WID_SCREEN_NUM ( XScreenNumberOfScreen( XtScreen( wid )))

/*******************************************************************/
