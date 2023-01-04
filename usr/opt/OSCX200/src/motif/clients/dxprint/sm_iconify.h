/*
 *****************************************************************************
 **									    *
 **  COPYRIGHT (c) 1988, 1989 BY					    *
 **  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		    *
 **  ALL RIGHTS RESERVED.						    *
 ** 									    *
 **  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED  *
 **  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE  *
 **  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER  *
 **  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
 **  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY  *
 **  TRANSFERRED.							    *
 ** 									    *
 **  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE  *
 **  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
 **  CORPORATION.							    *
 ** 									    *
 **  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS  *
 **  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		    *
 ** 									    *
 **									    *
 *****************************************************************************
 *++
 * FACILITY:  Session
 *
 * ABSTRACT:
 *
 *	The bitmap data for icon in title bar
 *
 * ENVIRONMENT:
 *
 *	VAX/VMS operating system.
 *
 * AUTHOR:  Karen Brouillette June 1988
 *
 * Modified by:
 *	02-FEB-1989	Karen Brouillette
 *	    Update copyright
 *
 */
#define sm_iconify_width 17
#define sm_iconify_height 17
static char sm_iconify_bits[] = {
   0x00, 0x00, 0xfe, 0xfe, 0xff, 0xfe, 0x02, 0x9e, 0xfe, 0x02, 0xbf, 0xfe,
   0x02, 0xf3, 0xfe, 0x02, 0xe3, 0xfe, 0x02, 0xe7, 0xfe, 0x02, 0xff, 0xfe,
   0x82, 0xbf, 0xfe, 0xc2, 0x81, 0xfe, 0xe2, 0x80, 0xfe, 0x72, 0x80, 0xfe,
   0x7a, 0x80, 0xfe, 0xe2, 0x80, 0xfe, 0x22, 0x80, 0xfe, 0xfe, 0xff, 0xfe,
   0x00, 0x00, 0xfe};
