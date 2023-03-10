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
 *      @(#)$RCSfile: ypupdate_prot.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/03/26 10:54:01 $
 */
/*
 */

/*
 * Compiled from ypupdate_prot.x using rpcgen
 * This is NOT source code!
 * DO NOT EDIT THIS FILE!
 */
#define MAXMAPNAMELEN 255
#define MAXYPDATALEN 1023
#define MAXERRMSGLEN 255

#define YPU_PROG 100028
#define YPU_VERS 1
#define YPU_CHANGE 1
extern u_int *ypu_change_1();
#define YPU_INSERT 2
extern u_int *ypu_insert_1();
#define YPU_DELETE 3
extern u_int *ypu_delete_1();
#define YPU_STORE 4
extern u_int *ypu_store_1();


typedef struct {
	u_int yp_buf_len;
	char *yp_buf_val;
} yp_buf;
bool_t xdr_yp_buf();


struct ypupdate_args {
	char *mapname;
	yp_buf key;
	yp_buf datum;
};
typedef struct ypupdate_args ypupdate_args;
bool_t xdr_ypupdate_args();


struct ypdelete_args {
	char *mapname;
	yp_buf key;
};
typedef struct ypdelete_args ypdelete_args;
bool_t xdr_ypdelete_args();

