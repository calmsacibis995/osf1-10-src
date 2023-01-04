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

static char SccsId[] = "@(#)gechecklicense.c	1.1\t11/22/88";
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	GECHECKLICENSE				Checks for presence of DOCUMENT
**						license on VMS systems
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	07/05/90	Compliments of Chris Peltz
**
**--                
**/

#ifdef VMS

#include <lmfdef.h>
#include <descrip.h>

/* LMF licence information defines, used in CheckLicense() */
#define PRODUCT_NAME            "DOCUMENT"
#define PRODUCER                "DEC"
/* these last three always need to be updated to include the date closest to  */
/* the release date (for all releases of VAX Document), and the major and     */
/* minor version numbers                                                      */
#define LATEST_BUILD_DATE       "04-AUG-1990"
#define MAJOR_VERSION           2
#define MINOR_VERSION           0



typedef struct {
	short buffer_length;
	short item_code;
	short *buffer_address;
	short *return_length_address;
	} lmf$item_list;


/* _____________________________CheckLicense__________________________________*/
/* setup and call the license checking routine, it never returns when there   */
/* isnt a license.                                                            */

geCheckLicense()
{unsigned int product_date[2] = {0,0}, stat;
 unsigned short product_version[2] = {MINOR_VERSION,MAJOR_VERSION};

#ifndef GEINTERNAL

 lmf$item_list lmf_items[3] = {{8,LMF$_PROD_DATE,&product_date,0},
                               {4,LMF$_PROD_VERSION,&product_version,0},
                                {0,0,0,0}};
 $DESCRIPTOR(build_date_d,LATEST_BUILD_DATE);
 $DESCRIPTOR(product_name_d,PRODUCT_NAME);
 $DESCRIPTOR(producer_d,PRODUCER);  

if (!((stat = sys$bintim(&build_date_d,&product_date)) & 01)) exit(stat);

                                        /* let me see you license, user! */
                                        /* did you know that you were */
                                        /* speeding at 80 miles an hour?! */

 sys$lookup_license(&product_name_d,/*** this doesn't return if bad ****/
                    &lmf_items,
                    &producer_d);/* but officer, I wasn't even driving */
                   /* 0,0,0);           /* an hour! */
#endif

} /* CheckLicense */

#endif
