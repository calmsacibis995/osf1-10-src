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
 *       @(#)$RCSfile: storclass.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/01/28 20:01:47 $
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/PMAX/storclass.h,v 4.2.4.2 1992/01/28 20:01:47 Al_Delorey Exp $ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#if __mips__
/*  storclass.h should not be included for "mips" machines */

#else	/* !__mips__ */

/*
 *   STORAGE CLASSES
 */

#define  C_EFCN          -1    /* physical end of function */
#define  C_NULL          0
#define  C_AUTO          1     /* automatic variable */
#define  C_EXT           2     /* external symbol */
#define  C_STAT          3     /* static */
#define  C_REG           4     /* register variable */
#define  C_EXTDEF        5     /* external definition */
#define  C_LABEL         6     /* label */
#define  C_ULABEL        7     /* undefined label */
#define  C_MOS           8     /* member of structure */
#define  C_ARG           9     /* function argument */
#define  C_STRTAG        10    /* structure tag */
#define  C_MOU           11    /* member of union */
#define  C_UNTAG         12    /* union tag */
#define  C_TPDEF         13    /* type definition */
#define C_USTATIC	 14    /* undefined static */
#define  C_ENTAG         15    /* enumeration tag */
#define  C_MOE           16    /* member of enumeration */
#define  C_REGPARM	 17    /* register parameter */
#define  C_FIELD         18    /* bit field */
#define  C_BLOCK         100   /* ".bb" or ".eb" */
#define  C_FCN           101   /* ".bf" or ".ef" */
#define  C_EOS           102   /* end of structure */
#define  C_FILE          103   /* file name */

	/*
	 * The following storage class is a "dummy" used only by STS
	 * for line number entries reformatted as symbol table entries
	 */

#define  C_LINE		 104
#define  C_ALIAS	 105   /* duplicate tag */
#define  C_HIDDEN	 106   /* special storage class for external */
			       /* symbols in dmert public libraries  */
#define  C_SHADOW    107   /* shadow symbol */

#endif	/* __mips__ */
