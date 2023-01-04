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
/* Masks: */
/* CUPDPS_PS_FILE is set if the file identifies itself as a PostScript file. */
/*	(ie. starts with "%!")
/* CUPDPS_PS_STRUCT is set if the file claims to follow the Adobe Structuring */
/*	Conventions.  (ie. starts with "%!PS-Adobe-")   If this flag is set, */
/*	the value of *info_structvers_ret is set to the verion number which */
/*	the file claims to adhere to. */
/* CUPDPS_PS_EPSF is set is the file claims to be an Encapsulated PostScript *.
/*	file. (ie. starts with "PS-Adobe-n.n EPSF-n.n")  If this flag is set, */
/*	the value of *info_epsfvers_ret is set to the EPSF conventions verion */
/*	number which the file claims to adhere to. */
/* CUPDPS_PS_BBOX is set if a %%BoundingBox: comment is actually found in the */
/*	file. */
/* CUPDPS_PS_BBOXVALS is set if the numerical parameters to the */
/*	%%BoundingBox: comment are found. */
#define CUPDPS_PS_FILE		0x01
#define CUPDPS_PS_STRUCT	0x02
#define CUPDPS_PS_EPSF		0x04
#define CUPDPS_PS_BBOX		0x08
#define CUPDPS_PS_BBOXVALS	0x10
