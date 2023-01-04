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
/* objcube.h - structure values for cube */

{	"cube", "cube",	/* long and short names */
	"octahedron",	/* long name of dual */
	8, 12, 6,	/* number of vertices, edges, and faces */
	{		/* vertices (x,y,z) */
			/* all points must be within radius 1 of the origin */
#define T 0.577
		{  T,  T,  T },
		{  T,  T, -T },
		{  T, -T, -T },
		{  T, -T,  T },
		{ -T,  T,  T },
		{ -T,  T, -T },
		{ -T, -T, -T },
		{ -T, -T,  T },
#undef T
	},
	{	/* faces (numfaces + indexes into vertices) */
		/*  faces must be specified clockwise from the outside */
		4,	0, 1, 2, 3,
		4, 	7, 6, 5, 4,
		4, 	1, 0, 4, 5,
		4,	3, 2, 6, 7,
		4,	2, 1, 5, 6,
		4,	0, 3, 7, 4,
	}
},		/* leave a comma to separate from the next include file */
/* end */
