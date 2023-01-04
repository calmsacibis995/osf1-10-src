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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: hupdate.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/07/30 18:30:15 $";
#endif

/*
 *	This file is used to determine if a hardware upgrade install should
 *	be done on this piece of hardware.  The 'hupdate' executable is
 *	loaded by osf_boot into the bootstrap address space.  osf_boot then
 *	executes 'hupdate' and expects a return value of non-zero if 
 *	the hardware requires a hardware upgrade install.
 *
 *	This executable will normally be loaded into the address space
 *	occupied by the primary bootstrap loader.  The same interfaces
 *	are available that are available to osf_boot.
 *
 *	NOTE:  The size of the entire executable can not be larger than 64K,
 *		because it would collide with the osf_boot address space.
 */

main()
{
    /* For a main release, we always boot vmunix */
    return (0);
}
