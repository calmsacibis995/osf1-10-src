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
/* polyinfo.h
 * This is the description of one polyhedron file
 */

#define MAXVERTS 120
	/* great rhombicosidodecahedron has 120 vertices */
#define MAXNV MAXVERTS
#define MAXFACES 30
	/* (hexakis icosahedron has 120 faces) */
#define MAXEDGES 180
	/* great rhombicosidodecahedron has 180 edges */
#define MAXEDGESPERPOLY 20

typedef struct {
	double x, y, z;
} Point3D;

/* structure of the include files which define the polyhedra */
typedef struct {
	char *longname;		/* long name of object */
	char *shortname;	/* short name of object */
	char *dual;		/* long name of dual */
	int numverts;		/* number of vertices */
	int numedges;		/* number of edges */
	int numfaces;		/* number of faces */
	Point3D v[MAXVERTS];	/* the vertices */
	int f[MAXEDGES*2+MAXFACES];	/* the faces */
} Polyinfo;

/* end */
