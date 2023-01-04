/*
!****************************************************************************
!*									    *
!*  COPYRIGHT (c) 1988, 1989, 1992 BY					    *
!*  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		    *
!*  ALL RIGHTS RESERVED.						    *
!* 									    *
!*  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
!*  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
!*  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
!*  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
!*  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
!*  TRANSFERRED.							    *
!* 									    *
!*  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
!*  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
!*  CORPORATION.							    *
!* 									    *
!*  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
!*  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		    *
!* 									    *
!*									    *
!****************************************************************************

!++
! FACILITY:  PrintScreen
!
! ABSTRACT:
!
!
! ENVIRONMENT:
!
!	VAX/VMS operating system.
!
! AUTHOR:  Karen Brouillette, October, 1989
!
! Modified by:
!
*/

/* BuildSystemHeader added automatically */
/* $Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/smshare.h,v 1.2 91/12/30 12:48:20 devbld Exp $ */

#ifndef SMSHARE
#define SMSHARE 1
/*
! Include files
*/

/*
#ifndef OSF1
#include <stddef.h>
#endif
*/

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif

#define black_white_system 0
#define color_system    1
#define gray_system     2
#define num_system_types 3

/* resource indexes */
#define rdb_color   0
#define rdb_generic 1
#define rdb_merge 2
#define rdb_user 3
#define num_databases 3

/* define string macro */

#define init_str_desc(desc, string) 		\
{   desc.dsc$w_length = strlen(string);		\
    desc.dsc$a_pointer = string;		\
    desc.dsc$b_class = DSC$K_CLASS_S;		\
    desc.dsc$b_dtype = DSC$K_DTYPE_T;		\
}
#define init_null_str_desc(desc, string)	\
{   desc.dsc$w_length = sizeof(string);		\
    desc.dsc$a_pointer = string;		\
    desc.dsc$b_class = DSC$K_CLASS_S;		\
    desc.dsc$b_dtype = DSC$K_DTYPE_T;		\
}

struct statusblock
	{
	short	stat;
	short	count;
	short	terminator;
	short	terminator_size;
	};

#endif /* SMSHARE */
