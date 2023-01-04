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
/*******************************************************************************
**  Copyright 1989-1991 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**  
**                          All Rights Reserved
**  
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**  
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**  
*******************************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**	The Xie module contains definitions used by all of the XIE layers
**	-- from the applications level through the XIE server levels.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Richard J. Piccolo
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      April 20, 1989
**
************************************************************************/

    /*
    **	Symbol XIEAPPL allows XieAppl.h to be included multiple times.
    */
#ifndef XIEAPPL
#define XIEAPPL	    /* The "endif" MUST be the last line of this file. */

/*****************************************************************************
**  Constants
*****************************************************************************/
    /*
    **	X Image Extension name string
    */
#define XieS_Name	    "Xie"

    /*
    **  Number of supported Events and Errors
    */
#define XieK_NumErrors		0
#define XieK_NumEvents		3
    /*
    **	Xie Event constants
    */
#define XieK_ComputationEvent	0
#define XieK_PhotofloEvent	1
#define XieK_DisplayEvent	2
    /*
    **	Xie Event masks
    */
#define XieM_ComputationEvent	(1<<XieK_ComputationEvent)
#define XieM_PhotofloEvent	(1<<XieK_PhotofloEvent)
#define XieM_DisplayEvent	(1<<XieK_DisplayEvent)

    /*
    **  Xie Resource Types
    */
#define XieK_Photoflo		1   /* process control object (pipeline)      */
#define XieK_Photomap		2   /* permanent image object		      */
#define XieK_Phototap		3   /* ephemeral image object		      */
#define XieK_IdcCpp		4   /* control processing plane		      */
#define XieK_IdcRoi		5   /* region of interest		      */
#define XieK_IdcTmp		6   /* template (eg. convolution kernel)      */

    /*
    **	Implementation bounds
    */
#define XieK_MaxComponents	 3  /* number of spectral components supported*/
#define XieK_MaxComponentDepth	16  /* bits per spectral component supported  */
#define XieK_MaxPlanes		24  /* number of image data planes supported  */
#define XieK_AllPlanes	0x00FFFFFF  /* mask indicating all image data planes  */

    /*
    **	Spectral Component Mappings
    */
#define XieK_Bitonal		1
#define XieK_GrayScale		2
#define XieK_RGB		3

    /*
    **	Component Space Organizations
    */
#define XieK_BandByPixel        1
#define XieK_BandByLine		2
#define XieK_BandByPlane        3
#define XieK_BitByPlane         4

    /*
    **	Transport modes
    */
#define XieK_GetStream		1
#define XieK_PutStream		2
#define XieK_Tile		3

    /*
    **	Get/Put Stream status
    */
#define XieK_StreamFinal	0
#define XieK_StreamEmpty	1
#define XieK_StreamMore		2
#define XieK_StreamError	3

    /*
    **	Compression Schemes
    */
#define XieK_PCM                0
#define XieK_G31D               1
#define XieK_G32D               2
#define XieK_G42D               3
#define XieK_DCT                4

    /*
    **	Line Progressions
    */
#define XieK_LP90		1
#define XieK_LP270		2

    /*
    **	Pixel Progressions
    */
#define XieK_PP0		1
#define XieK_PP90		2
#define XieK_PP180		3
#define XieK_PP270		4

    /*
    **	Polarities
    */
#define XieK_ZeroBright		0
#define XieK_ZeroDark		1

    /*
    **	Photoflo status
    */
#define XieK_PhotofloFormation	0
#define XieK_PhotofloComplete	1
#define XieK_PhotofloRunning	2
#define XieK_PhotofloAborted	3

    /*
    **	Statistics Types
    */
#define XieK_Minimum		1
#define XieK_Maximum		2
#define XieK_Mean		3
#define XieK_StdDev		4
#define XieK_Variance		5

    /*
    **	Gray to Bitonal bit reduction methods
    */
#define XieK_Dither		1
#define XieK_Threshold		2

    /*
    **	Conditional Operator Types
    */
#define XieK_LT			1
#define XieK_LE			2
#define XieK_EQ			3
#define XieK_NE                 4
#define XieK_GT			5   
#define XieK_GE			6

    /*
    ** Area Operators
    */
#define XieK_AreaOp1Add		1
#define XieK_AreaOp1Mult	2
#define XieK_AreaOp2Min		3
#define XieK_AreaOp2Max		4
#define XieK_AreaOp2Sum		5


    /*
    ** Arithmetical Operators
    */
#define XieK_AddII		 1
#define XieK_AddIC		 2
#define XieK_SubII		 3
#define XieK_SubIC		 4
#define XieK_SubCI		 5
#define XieK_MulII		 6
#define XieK_MulIC		 7
#define XieK_DivII		 8
#define XieK_DivIC		 9
#define XieK_DivCI		10
#define XieK_MinII		11
#define XieK_MinIC		12
#define XieK_MaxII		13
#define XieK_MaxIC		14

    /*
    ** Logical Operators
    */
#define XieK_AND		1
#define XieK_OR			2
#define XieK_XOR		3
#define XieK_NOT		4
#define XieK_ShiftUp		5
#define XieK_ShiftDown		6

    /*
    ** Mathematical Functions
    */
#define XieK_Exp		1
#define XieK_Ln			2
#define XieK_Log2		3
#define XieK_Log10		4
#define XieK_Square		5
#define XieK_Sqrt		6

    /*
    ** Constraint Models
    */
#define XieK_MoveMode           1	/* Move without changing data */
#define XieK_HardClip           2
#define XieK_ClipScale          3
#define XieK_HardScale          4

    /*
    ** MatchHistogram Distribution Shapes
    */
#define XieK_Flat		1
#define XieK_Gaussian           2
#define XieK_Hyperbolic         3


/*****************************************************************************
**  Structures
*****************************************************************************/
    /*
    **  none
    */

/*
**  This "endif" MUST be the last line of this file.
*/
#endif	/* end of XIEAPPL -- NO DEFINITIONS ALLOWED BEYOND THIS POINT */

