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
 * @(#)$RCSfile: moss_extend.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:32:07 $
 */
/*
**++
**
**  Copyright (c) Digital Equipment Corporation, 1991, 1992
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
**  FACILITY:  MOSS, Managed Object Support Routines
**
**  MODULE DESCRIPTION:
**
**      This header file defines function prototypes for the VMS-supplied
**	extensions to the MOSS routine set.
**
**  AUTHORS:
**
**	VMS System Management Engineering
**
**  CREATION DATE:  August 5, 1991
**
**  MODIFICATION HISTORY:
**
**	F-9	RJB1222	    Richard J. Bouchard Jr.	24-Jul-1992
**		Move 'common' MOSS routines into common code area.
**
**	F-8	DMR1227     David M. Rosenberg		13-May-1992
**		Add moss_avl_remove_construct().
**
**	F-7	DMR1177     David M. Rosenberg		 8-Apr-1992
**		Add copyright notice and module header.
**		Add prototypes for moss_avl_verify() and moss_avl_copy_all().
**--
*/

#ifdef VMS

/*
 * moss_extend_dns.c
 */
int moss_dns_opaque_to_simplename(char *opaque_buffer, 
				  short int *opaque_len, 
				  char *SimpleName_buffer, 
				  short int *SimplaName_len);

int moss_dns_simplename_to_opaque(char *SimpleName_buffer, 
				  short int *SimpleName_len, 
				  char *opaque_buffer, 
				  short int *opaque_len);

int moss_dns_opaque_to_fullname( char *opaque_buffer,
				short int *opaque_len,
				char *FN_buffer,
				short int *FN_len );

int moss_dns_fullname_to_opaque( char *FN_buffer,
				short int *FN_len,
				char *opaque_buffer,
				short int *opaque_len );

/*
 * moss_extend_avl.c
 */
man_status
moss_avl_verify (
	avl *	avl_handle
	);

man_status
vms_moss_avl_point( 
               avl	         *avl_handle , 
               object_id         **oid , 
               unsigned long int *modifier , 
               unsigned long int *tag , 
               octet_string      **octet , 
               int		 *last_one 
              );

#endif
