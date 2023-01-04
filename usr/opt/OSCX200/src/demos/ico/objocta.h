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
/* objocta.h - structure values for octahedron */

{	"octahedron", "octa",	/* long and short names */
	"cube",		/* long name of dual */
	6, 12, 8,	/* number of vertices, edges, and faces */
	{		/* vertices (x,y,z) */
			/* all points must be within radius 1 of the origin */
#define T 1.0
		{  T,  0,  0 },
		{ -T,  0,  0 },
		{  0,  T,  0 },
		{  0, -T,  0 },
		{  0,  0,  T },
		{  0,  0, -T },
#undef T
	},
	{	/* faces (numfaces + indexes into vertices) */
		/*  faces must be specified clockwise from the outside */
		3,	0, 4, 2,
		3,	0, 2, 5,
		3,	0, 5, 3,
		3,	0, 3, 4,
		3,	1, 2, 4,
		3,	1, 5, 2,
		3,	1, 3, 5,
		3,	1, 4, 3,
	}
},		/* leave a comma to separate from the next include file */
/* end */
