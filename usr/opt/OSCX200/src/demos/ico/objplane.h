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
/* objplane.h - structure values for plane */

{       "plane", "plane",       /* long and short names */
        "cube",         /* long name of dual */
        4, 4, 1,        /* number of vertices, edges, and faces */
        {               /* vertices (x,y,z) */
                        /* all points must be within radius 1 of the origin */
#define T 1.0
                {  T,  0,  0 },
                { -T,  0,  0 },
                {  0,  T,  0 },
                {  0, -T,  0 },
#undef T
        },
        {       /* faces (numfaces + indexes into vertices) */
                /*  faces must be specified clockwise from the outside */
                4,      0, 2, 1, 3,
        }
},              /* leave a comma to separate from the next include file */
/* end */
