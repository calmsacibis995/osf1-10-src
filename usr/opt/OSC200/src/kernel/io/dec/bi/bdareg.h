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
/*	7/2/90 (ULTRIX-32) @(#)bdareg.h	4.1	*/	
/* ------------------------------------------------------------------------
 * Modification History: /sys/vaxbi/bdareg.h
 *
 * 04-feb-86 -- jaw  get rid of biic.h.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 *	20 Mar 85 -- jaw
 *		add support for VAX 8200.
 *
 * ------------------------------------------------------------------------
 */
#ifndef _BDAREG_H_
#define _BDAREG_H_
struct bda_regs
{
	struct 	biic_regs bda_biic;	/* BIIC specific registers */
};

#endif

