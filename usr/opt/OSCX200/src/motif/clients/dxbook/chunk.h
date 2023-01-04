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
/* DEC/CMS REPLACEMENT HISTORY, Element CHUNK.H*/
/* *2     9-JUL-1990 15:49:28 FITZELL ""*/
/* *1    24-APR-1990 15:43:51 FITZELL "creating initial elements that shipped with V2 VWI"*/
/* DEC/CMS REPLACEMENT HISTORY, Element CHUNK.H*/
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	DECwindows BookReader
**
**  ABSTRACT:
**
**	[@tbs@]
**
**  ENVIRONMENT:
**
**	VAX/VMS Operating System
**
**  MODIFICATION HISTORY:
**
**
**--
**/


/*                                                                          */
/*	POINT -- Defines a polygon point (unscaled) below the VRI interface */
/*                                                                          */
typedef struct _POINT {
    int		    x;
    int		    y;
    }POINT;

/*                                                                          */
/*	XPOINT -- Defines a polygon point (scaled) above the VRI interface  */
/*                                                                          */
typedef struct _XPOINT {
    short	    x;
    short	    y;
    }XPOINT;

/*                                                                          */
/*	CHUNK -- Chunk Block                                                */
/*                                                                          */
typedef struct _CHUNK {
    unsigned short  chunk_type;		/* MAIN_CHUNK, SUB_CHUNK, X_REF	    */
    unsigned	    id;			/* chunk identifier                 */
    unsigned	    parent;		/* chunk identifier of parent       */
    int		    x;			/* x offset within parent           */
    int		    y;			/* Y offset within parent           */
    int		    width;	        /* width of display                 */
    int		    height;		/* height of display                */
    unsigned	    target;		/* target id of cross reference     */
    unsigned	    data_type;		/* data type identifier		    */
    char	    *data_addr;         /* display data                     */
    unsigned	    data_len;		/* length of display data           */
    unsigned	    handle;		/* data type specific information   */
					/* used for display id's etc.       */
    unsigned	    num_points;		/* no. of points for polygon chunks */
    POINT	    *point_vec;		/* polygon points (unscaled)	    */
    XPOINT	    *xpoint_vec;	/* polygon points (scaled)	    */
    } CHUNK;

/*  These constants must agree with the corresponding TLV$K_ tags from ODS  */

# define    DATA_CHUNK		    18
# define    DATA_SUBCHUNK	    19
# define    REFERENCE_RECT	    20
# define    REFERENCE_POLY	    21
# define    EXTENSION_RECT	    22
# define    EXTENSION_POLY	    23

