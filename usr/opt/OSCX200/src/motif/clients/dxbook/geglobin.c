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
static char SccsId[] = "@(#)geglobin.c	1.1\t11/22/88";
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
**	GEGLOBIN.C        		RAGS globals definitions & initializations
**
**  ABSTRACT:
**
** 	This module contains only initializations
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 09/24/86 Created
**	DAA 11/22/88 Revised
**
**--
**/

#include "geGks.h"
#include "Xatom.h"
#include <math.h>

                 
extern               geLin();
extern unsigned long geAllocColor();
extern char	     *geMalloc();
extern XFontStruct   *XLoadQueryFont();

geGlobin1()
{
static int 	i;

/*  Initialize globals that cannot be initialized at compiletime.
 */

#ifdef GEVERSION5
  geVersionOut = 5;
#else
  geVersionOut = GECURRENTVERSION; 
#endif

geRun.Sui2   	   = GESUI2;
geRun.ErrReport    = TRUE;
geRun.MopsCalling  = FALSE;
geRun.RagsCalling  = FALSE;
geRun.VoyerCalling = FALSE;

geFdL			= NULL;
geState.EditCmdTmp      = FALSE;
geState.Mode		= -1;
geState.InCrop		= FALSE;
geState.ASegP           = NULL;			/* No active segment	     */
geState.GSegP           = NULL;			/* No active group	     */
geState.PGrpP           = NULL;		        /* No parent group	     */
geState.LiveSegs	= 0;                  
geState.XDoP            = NULL;
geState.Exit            = FALSE;
geState.MsgNum          = GEQUIETMSG;
geState.ChkPt.Cur 	= 0;
geState.State		= GEINIT;
geState.HintsAllowed	= TRUE;
geState.ForceHTDisp 	= FALSE;
geState.Pixmap          = NULL;
geState.NeedToPostPixmap = FALSE;
geState.Locked          = FALSE;
geState.HaltCmd         = 0;
geState.HaltRequested   = FALSE;

#ifdef GERAGS
geState.AnimPreview     = FALSE;
#else
geState.AnimPreview     = TRUE;
#endif

geMagGrp = FALSE;
geBreakIn = FALSE;
geReadWriteType = 0;
geDebug = FALSE;
geExportFlag = FALSE;
geMultiShellCount = 0;
geColStat = 0;
geLinWL = 6;  
geGetPutFlgs = 0;
geMagCx = 0;
geMagCy = 0;
geMagFX = 100.0; 
geMagFY = 100.0;

geCTM.a  = 1.0;
geCTM.b  = 0.0;
geCTM.c  = 0.0;
geCTM.d  = 1.0;
geCTM.tx = 0.0;
geCTM.ty = 0.0;

geSetUp.Blink	     = FALSE;
geSetUp.LinWModOnMag = TRUE;

geRot.Alpha 	= 90.0;
geRot.Beta 	= 90.0;
geRot.Clockwise = FALSE;

geAln.APagP   = NULL;
geAln.Co.x1   = 0;
geAln.Co.y1   = 0;
geAln.Co.x2   = 0;
geAln.Co.y2   = 0;
geAln.Pt.x    = GEMAXLONG;
geAln.Pt.y    = GEMAXLONG;
geAln.AlnFrom = 0;
geAln.AlnTo   = 0;

geInitManage.attr_init      = 0;
geInitManage.fonts_init     = 0;
geInitManage.units_init     = 0;
geInitManage.grid_init      = 0;
geInitManage.hints_init	    = 0;
geInitManage.customize_init = 0;

geDashList[0][0] = 2; geDashList[0][1] = 2;
geDashList[0][2] = 2; geDashList[0][3] = 2;
geDashList[1][0] = 1; geDashList[1][1] = 3;
geDashList[1][2] = 1; geDashList[1][3] = 3;
geDashList[2][0] = 6; geDashList[2][1] = 2;
geDashList[2][2] = 1; geDashList[2][3] = 2;
geDashList[3][0] = 4; geDashList[3][1] = 2;
geDashList[3][2] = 4; geDashList[3][3] = 2;

geState.Grid.On     = 0;
geState.Grid.Xorg   = 0;
geState.Grid.Yorg   = 0;
geState.Grid.MajorX = 51200;
geState.Grid.MajorY = 51200;
geState.Grid.DivX   = 5;
geState.Grid.DivY   = 5;
geState.Grid.Top    = 1;
geState.Grid.XAlign = 0;
geState.Grid.YAlign = 0;

/*
 * Units
 */
geU.Map = geU.LMap      = -1;
geU.Abs                 = TRUE;
geU.XO  = geU.YO = geU.XR = geU.YR = 0;

/*
 * Yet more initialization the VMS compiler is screwing up for some reason
 */
i = 0;
geSin[i++]    =  0.0000; geSin[i++]    =  0.0175; geSin[i++]    =  0.0349;
geSin[i++]    =  0.0523; geSin[i++]    =  0.0698; geSin[i++]    =  0.0872;
geSin[i++]    =  0.1045; geSin[i++]    =  0.1219; geSin[i++]    =  0.1392;
geSin[i++]    =  0.1564; geSin[i++]    =  0.1736; geSin[i++]    =  0.1908;
geSin[i++]    =  0.2079; geSin[i++]    =  0.2250; geSin[i++]    =  0.2419;
geSin[i++]    =  0.2588; geSin[i++]    =  0.2756; geSin[i++]    =  0.2924;
geSin[i++]    =  0.3090; geSin[i++]    =  0.3256; geSin[i++]    =  0.3420;
geSin[i++]    =  0.3584; geSin[i++]    =  0.3746; geSin[i++]    =  0.3907;
geSin[i++]    =  0.4067; geSin[i++]    =  0.4226; geSin[i++]    =  0.4384;
geSin[i++]    =  0.4540; geSin[i++]    =  0.4695; geSin[i++]    =  0.4848;
geSin[i++]    =  0.5000; geSin[i++]    =  0.5150; geSin[i++]    =  0.5299;
geSin[i++]    =  0.5446; geSin[i++]    =  0.5592; geSin[i++]    =  0.5736;
geSin[i++]    =  0.5878; geSin[i++]    =  0.6018; geSin[i++]    =  0.6157;
geSin[i++]    =  0.6293; geSin[i++]    =  0.6428; geSin[i++]    =  0.6561;
geSin[i++]    =  0.6691; geSin[i++]    =  0.6820; geSin[i++]    =  0.6947;
geSin[i++]    =  0.7071; geSin[i++]    =  0.7193; geSin[i++]    =  0.7314;
geSin[i++]    =  0.7431; geSin[i++]    =  0.7547; geSin[i++]    =  0.7660;
geSin[i++]    =  0.7771; geSin[i++]    =  0.7880; geSin[i++]    =  0.7986;
geSin[i++]    =  0.8090; geSin[i++]    =  0.8192; geSin[i++]    =  0.8290;
geSin[i++]    =  0.8387; geSin[i++]    =  0.8480; geSin[i++]    =  0.8572;
geSin[i++]    =  0.8660; geSin[i++]    =  0.8746; geSin[i++]    =  0.8829;
geSin[i++]    =  0.8910; geSin[i++]    =  0.8988; geSin[i++]    =  0.9063;
geSin[i++]    =  0.9135; geSin[i++]    =  0.9205; geSin[i++]    =  0.9272;
geSin[i++]    =  0.9336; geSin[i++]    =  0.9397; geSin[i++]    =  0.9455;
geSin[i++]    =  0.9511; geSin[i++]    =  0.9563; geSin[i++]    =  0.9613;
geSin[i++]    =  0.9659; geSin[i++]    =  0.9703; geSin[i++]    =  0.9744;
geSin[i++]    =  0.9781; geSin[i++]    =  0.9816; geSin[i++]    =  0.9848;
geSin[i++]    =  0.9877; geSin[i++]    =  0.9903; geSin[i++]    =  0.9925;
geSin[i++]    =  0.9945; geSin[i++]    =  0.9962; geSin[i++]    =  0.9976;
geSin[i++]    =  0.9986; geSin[i++]    =  0.9994; geSin[i++]    =  0.9998;

geSin[i++]    =  1.0000; geSin[i++]    =  0.9998; geSin[i++]    =  0.9994;
geSin[i++]    =  0.9986; geSin[i++]    =  0.9976; geSin[i++]    =  0.9962;
geSin[i++]    =  0.9945; geSin[i++]    =  0.9925; geSin[i++]    =  0.9903;
geSin[i++]    =  0.9877; geSin[i++]    =  0.9848; geSin[i++]    =  0.9816;
geSin[i++]    =  0.9781; geSin[i++]    =  0.9744; geSin[i++]    =  0.9703;
geSin[i++]    =  0.9659; geSin[i++]    =  0.9613; geSin[i++]    =  0.9563;
geSin[i++]    =  0.9511; geSin[i++]    =  0.9455; geSin[i++]    =  0.9397;
geSin[i++]    =  0.9336; geSin[i++]    =  0.9272; geSin[i++]    =  0.9205;
geSin[i++]    =  0.9135; geSin[i++]    =  0.9063; geSin[i++]    =  0.8988;
geSin[i++]    =  0.8910; geSin[i++]    =  0.8829; geSin[i++]    =  0.8746;
geSin[i++]    =  0.8660; geSin[i++]    =  0.8572; geSin[i++]    =  0.8480;
geSin[i++]    =  0.8387; geSin[i++]    =  0.8290; geSin[i++]    =  0.8192;
geSin[i++]    =  0.8090; geSin[i++]    =  0.7986; geSin[i++]    =  0.7880;
geSin[i++]    =  0.7771; geSin[i++]    =  0.7660; geSin[i++]    =  0.7547;
geSin[i++]    =  0.7431; geSin[i++]    =  0.7314; geSin[i++]    =  0.7193;
geSin[i++]    =  0.7071; geSin[i++]    =  0.6947; geSin[i++]    =  0.6820;
geSin[i++]    =  0.6691; geSin[i++]    =  0.6561; geSin[i++]    =  0.6428;
geSin[i++]    =  0.6293; geSin[i++]    =  0.6157; geSin[i++]    =  0.6018;
geSin[i++]    =  0.5878; geSin[i++]    =  0.5736; geSin[i++]    =  0.5592;
geSin[i++]    =  0.5446; geSin[i++]    =  0.5299; geSin[i++]    =  0.5150;
geSin[i++]    =  0.5000; geSin[i++]    =  0.4848; geSin[i++]    =  0.4695;
geSin[i++]    =  0.4540; geSin[i++]    =  0.4384; geSin[i++]    =  0.4226;
geSin[i++]    =  0.4067; geSin[i++]    =  0.3907; geSin[i++]    =  0.3746;
geSin[i++]    =  0.3584; geSin[i++]    =  0.3420; geSin[i++]    =  0.3256;
geSin[i++]    =  0.3090; geSin[i++]    =  0.2924; geSin[i++]    =  0.2756;
geSin[i++]    =  0.2588; geSin[i++]    =  0.2419; geSin[i++]    =  0.2250;
geSin[i++]    =  0.2079; geSin[i++]    =  0.1908; geSin[i++]    =  0.1736;
geSin[i++]    =  0.1564; geSin[i++]    =  0.1392; geSin[i++]    =  0.1219;
geSin[i++]    =  0.1045; geSin[i++]    =  0.0872; geSin[i++]    =  0.0698;
geSin[i++]    =  0.0523; geSin[i++]    =  0.0349; geSin[i++]    =  0.0175;
geSin[i++]    = -0.0000; geSin[i++]    = -0.0175; geSin[i++]    = -0.0349;
geSin[i++]    = -0.0523; geSin[i++]    = -0.0698; geSin[i++]    = -0.0872;
geSin[i++]    = -0.1045; geSin[i++]    = -0.1219; geSin[i++]    = -0.1392;
geSin[i++]    = -0.1564; geSin[i++]    = -0.1736; geSin[i++]    = -0.1908;
geSin[i++]    = -0.2079; geSin[i++]    = -0.2250; geSin[i++]    = -0.2419;
geSin[i++]    = -0.2588; geSin[i++]    = -0.2756; geSin[i++]    = -0.2924;
geSin[i++]    = -0.3090; geSin[i++]    = -0.3256; geSin[i++]    = -0.3420;
geSin[i++]    = -0.3584; geSin[i++]    = -0.3746; geSin[i++]    = -0.3907;
geSin[i++]    = -0.4067; geSin[i++]    = -0.4226; geSin[i++]    = -0.4384;
geSin[i++]    = -0.4540; geSin[i++]    = -0.4695; geSin[i++]    = -0.4848;
geSin[i++]    = -0.5000; geSin[i++]    = -0.5150; geSin[i++]    = -0.5299;
geSin[i++]    = -0.5446; geSin[i++]    = -0.5592; geSin[i++]    = -0.5736;
geSin[i++]    = -0.5878; geSin[i++]    = -0.6018; geSin[i++]    = -0.6157;
geSin[i++]    = -0.6293; geSin[i++]    = -0.6428; geSin[i++]    = -0.6561;
geSin[i++]    = -0.6691; geSin[i++]    = -0.6820; geSin[i++]    = -0.6947;
geSin[i++]    = -0.7071; geSin[i++]    = -0.7193; geSin[i++]    = -0.7314;
geSin[i++]    = -0.7431; geSin[i++]    = -0.7547; geSin[i++]    = -0.7660;
geSin[i++]    = -0.7771; geSin[i++]    = -0.7880; geSin[i++]    = -0.7986;
geSin[i++]    = -0.8090; geSin[i++]    = -0.8192; geSin[i++]    = -0.8290;
geSin[i++]    = -0.8387; geSin[i++]    = -0.8480; geSin[i++]    = -0.8572;
geSin[i++]    = -0.8660; geSin[i++]    = -0.8746; geSin[i++]    = -0.8829;
geSin[i++]    = -0.8910; geSin[i++]    = -0.8988; geSin[i++]    = -0.9063;
geSin[i++]    = -0.9135; geSin[i++]    = -0.9205; geSin[i++]    = -0.9272;
geSin[i++]    = -0.9336; geSin[i++]    = -0.9397; geSin[i++]    = -0.9455;
geSin[i++]    = -0.9511; geSin[i++]    = -0.9563; geSin[i++]    = -0.9613;
geSin[i++]    = -0.9659; geSin[i++]    = -0.9703; geSin[i++]    = -0.9744;
geSin[i++]    = -0.9781; geSin[i++]    = -0.9816; geSin[i++]    = -0.9848;
geSin[i++]    = -0.9877; geSin[i++]    = -0.9903; geSin[i++]    = -0.9925;
geSin[i++]    = -0.9945; geSin[i++]    = -0.9962; geSin[i++]    = -0.9976;
geSin[i++]    = -0.9986; geSin[i++]    = -0.9994; geSin[i++]    = -0.9998;

geSin[i++]    = -1.0000; geSin[i++]    = -0.9998; geSin[i++]    = -0.9994;
geSin[i++]    = -0.9986; geSin[i++]    = -0.9976; geSin[i++]    = -0.9962;
geSin[i++]    = -0.9945; geSin[i++]    = -0.9925; geSin[i++]    = -0.9903;
geSin[i++]    = -0.9877; geSin[i++]    = -0.9848; geSin[i++]    = -0.9816;
geSin[i++]    = -0.9781; geSin[i++]    = -0.9744; geSin[i++]    = -0.9703;
geSin[i++]    = -0.9659; geSin[i++]    = -0.9613; geSin[i++]    = -0.9563;
geSin[i++]    = -0.9511; geSin[i++]    = -0.9455; geSin[i++]    = -0.9397;
geSin[i++]    = -0.9336; geSin[i++]    = -0.9272; geSin[i++]    = -0.9205;
geSin[i++]    = -0.9135; geSin[i++]    = -0.9063; geSin[i++]    = -0.8988;
geSin[i++]    = -0.8910; geSin[i++]    = -0.8829; geSin[i++]    = -0.8746;
geSin[i++]    = -0.8660; geSin[i++]    = -0.8572; geSin[i++]    = -0.8480;
geSin[i++]    = -0.8387; geSin[i++]    = -0.8290; geSin[i++]    = -0.8192;
geSin[i++]    = -0.8090; geSin[i++]    = -0.7986; geSin[i++]    = -0.7880;
geSin[i++]    = -0.7771; geSin[i++]    = -0.7660; geSin[i++]    = -0.7547;
geSin[i++]    = -0.7431; geSin[i++]    = -0.7314; geSin[i++]    = -0.7193;
geSin[i++]    = -0.7071; geSin[i++]    = -0.6947; geSin[i++]    = -0.6820;
geSin[i++]    = -0.6691; geSin[i++]    = -0.6561; geSin[i++]    = -0.6428;
geSin[i++]    = -0.6293; geSin[i++]    = -0.6157; geSin[i++]    = -0.6018;
geSin[i++]    = -0.5878; geSin[i++]    = -0.5736; geSin[i++]    = -0.5592;
geSin[i++]    = -0.5446; geSin[i++]    = -0.5299; geSin[i++]    = -0.5150;
geSin[i++]    = -0.5000; geSin[i++]    = -0.4848; geSin[i++]    = -0.4695;
geSin[i++]    = -0.4540; geSin[i++]    = -0.4384; geSin[i++]    = -0.4226;
geSin[i++]    = -0.4067; geSin[i++]    = -0.3907; geSin[i++]    = -0.3746;
geSin[i++]    = -0.3584; geSin[i++]    = -0.3420; geSin[i++]    = -0.3256;
geSin[i++]    = -0.3090; geSin[i++]    = -0.2924; geSin[i++]    = -0.2756;
geSin[i++]    = -0.2588; geSin[i++]    = -0.2419; geSin[i++]    = -0.2250;
geSin[i++]    = -0.2079; geSin[i++]    = -0.1908; geSin[i++]    = -0.1736;
geSin[i++]    = -0.1564; geSin[i++]    = -0.1392; geSin[i++]    = -0.1219;
geSin[i++]    = -0.1045; geSin[i++]    = -0.0872; geSin[i++]    = -0.0698;
geSin[i++]    = -0.0523; geSin[i++]    = -0.0349; geSin[i++]    = -0.0175;
geSin[i++]    = -0.0000;
i = 0;
geCos[i++]    =  1.0000; geCos[i++]    =  0.9998;
geCos[i++]    =  0.9994; geCos[i++]    =  0.9986; geCos[i++]    =  0.9976;
geCos[i++]    =  0.9962; geCos[i++]    =  0.9945; geCos[i++]    =  0.9925;
geCos[i++]    =  0.9903; geCos[i++]    =  0.9877; geCos[i++]    =  0.9848;
geCos[i++]    =  0.9816; geCos[i++]    =  0.9781; geCos[i++]    =  0.9744;
geCos[i++]    =  0.9703; geCos[i++]    =  0.9659; geCos[i++]    =  0.9613;
geCos[i++]    =  0.9563; geCos[i++]    =  0.9511; geCos[i++]    =  0.9455;
geCos[i++]    =  0.9397; geCos[i++]    =  0.9336; geCos[i++]    =  0.9272;
geCos[i++]    =  0.9205; geCos[i++]    =  0.9135; geCos[i++]    =  0.9063;
geCos[i++]    =  0.8988; geCos[i++]    =  0.8910; geCos[i++]    =  0.8829;
geCos[i++]    =  0.8746; geCos[i++]    =  0.8660; geCos[i++]    =  0.8572;
geCos[i++]    =  0.8480; geCos[i++]    =  0.8387; geCos[i++]    =  0.8290;
geCos[i++]    =  0.8192; geCos[i++]    =  0.8090; geCos[i++]    =  0.7986;
geCos[i++]    =  0.7880; geCos[i++]    =  0.7771; geCos[i++]    =  0.7660;
geCos[i++]    =  0.7547; geCos[i++]    =  0.7431; geCos[i++]    =  0.7314;
geCos[i++]    =  0.7193; geCos[i++]    =  0.7071; geCos[i++]    =  0.6947;
geCos[i++]    =  0.6820; geCos[i++]    =  0.6691; geCos[i++]    =  0.6561;
geCos[i++]    =  0.6428; geCos[i++]    =  0.6293; geCos[i++]    =  0.6157;
geCos[i++]    =  0.6018; geCos[i++]    =  0.5878; geCos[i++]    =  0.5736;
geCos[i++]    =  0.5592; geCos[i++]    =  0.5446; geCos[i++]    =  0.5299;
geCos[i++]    =  0.5150; geCos[i++]    =  0.5000; geCos[i++]    =  0.4848;
geCos[i++]    =  0.4695; geCos[i++]    =  0.4540; geCos[i++]    =  0.4384;
geCos[i++]    =  0.4226; geCos[i++]    =  0.4067; geCos[i++]    =  0.3907;
geCos[i++]    =  0.3746; geCos[i++]    =  0.3584; geCos[i++]    =  0.3420;
geCos[i++]    =  0.3256; geCos[i++]    =  0.3090; geCos[i++]    =  0.2924;
geCos[i++]    =  0.2756; geCos[i++]    =  0.2588; geCos[i++]    =  0.2419;
geCos[i++]    =  0.2250; geCos[i++]    =  0.2079; geCos[i++]    =  0.1908;
geCos[i++]    =  0.1736; geCos[i++]    =  0.1564; geCos[i++]    =  0.1392;
geCos[i++]    =  0.1219; geCos[i++]    =  0.1045; geCos[i++]    =  0.0872;
geCos[i++]    =  0.0698; geCos[i++]    =  0.0523; geCos[i++]    =  0.0349;
geCos[i++]    =  0.0175;

geCos[i++]    = -0.0000; geCos[i++]    = -0.0175;
geCos[i++]    = -0.0349; geCos[i++]    = -0.0523; geCos[i++]    = -0.0698;
geCos[i++]    = -0.0872; geCos[i++]    = -0.1045; geCos[i++]    = -0.1219;
geCos[i++]    = -0.1392; geCos[i++]    = -0.1564; geCos[i++]    = -0.1736;
geCos[i++]    = -0.1908; geCos[i++]    = -0.2079; geCos[i++]    = -0.2250;
geCos[i++]    = -0.2419; geCos[i++]    = -0.2588; geCos[i++]    = -0.2756;
geCos[i++]    = -0.2924; geCos[i++]    = -0.3090; geCos[i++]    = -0.3256;
geCos[i++]    = -0.3420; geCos[i++]    = -0.3584; geCos[i++]    = -0.3746;
geCos[i++]    = -0.3907; geCos[i++]    = -0.4067; geCos[i++]    = -0.4226;
geCos[i++]    = -0.4384; geCos[i++]    = -0.4540; geCos[i++]    = -0.4695;
geCos[i++]    = -0.4848; geCos[i++]    = -0.5000; geCos[i++]    = -0.5150;
geCos[i++]    = -0.5299; geCos[i++]    = -0.5446; geCos[i++]    = -0.5592;
geCos[i++]    = -0.5736; geCos[i++]    = -0.5878; geCos[i++]    = -0.6018;
geCos[i++]    = -0.6157; geCos[i++]    = -0.6293; geCos[i++]    = -0.6428;
geCos[i++]    = -0.6561; geCos[i++]    = -0.6691; geCos[i++]    = -0.6820;
geCos[i++]    = -0.6947; geCos[i++]    = -0.7071; geCos[i++]    = -0.7193;
geCos[i++]    = -0.7314; geCos[i++]    = -0.7431; geCos[i++]    = -0.7547;
geCos[i++]    = -0.7660; geCos[i++]    = -0.7771; geCos[i++]    = -0.7880;
geCos[i++]    = -0.7986; geCos[i++]    = -0.8090; geCos[i++]    = -0.8192;
geCos[i++]    = -0.8290; geCos[i++]    = -0.8387; geCos[i++]    = -0.8480;
geCos[i++]    = -0.8572; geCos[i++]    = -0.8660; geCos[i++]    = -0.8746;
geCos[i++]    = -0.8829; geCos[i++]    = -0.8910; geCos[i++]    = -0.8988;
geCos[i++]    = -0.9063; geCos[i++]    = -0.9135; geCos[i++]    = -0.9205;
geCos[i++]    = -0.9272; geCos[i++]    = -0.9336; geCos[i++]    = -0.9397;
geCos[i++]    = -0.9455; geCos[i++]    = -0.9511; geCos[i++]    = -0.9563;
geCos[i++]    = -0.9613; geCos[i++]    = -0.9659; geCos[i++]    = -0.9703;
geCos[i++]    = -0.9744; geCos[i++]    = -0.9781; geCos[i++]    = -0.9816;
geCos[i++]    = -0.9848; geCos[i++]    = -0.9877; geCos[i++]    = -0.9903;
geCos[i++]    = -0.9925; geCos[i++]    = -0.9945; geCos[i++]    = -0.9962;
geCos[i++]    = -0.9976; geCos[i++]    = -0.9986; geCos[i++]    = -0.9994;
geCos[i++]    = -0.9998;

geCos[i++]    = -1.0000; geCos[i++]    = -0.9998;
geCos[i++]    = -0.9994; geCos[i++]    = -0.9986; geCos[i++]    = -0.9976;
geCos[i++]    = -0.9962; geCos[i++]    = -0.9945; geCos[i++]    = -0.9925;
geCos[i++]    = -0.9903; geCos[i++]    = -0.9877; geCos[i++]    = -0.9848;
geCos[i++]    = -0.9816; geCos[i++]    = -0.9781; geCos[i++]    = -0.9744;
geCos[i++]    = -0.9703; geCos[i++]    = -0.9659; geCos[i++]    = -0.9613;
geCos[i++]    = -0.9563; geCos[i++]    = -0.9511; geCos[i++]    = -0.9455;
geCos[i++]    = -0.9397; geCos[i++]    = -0.9336; geCos[i++]    = -0.9272;
geCos[i++]    = -0.9205; geCos[i++]    = -0.9135; geCos[i++]    = -0.9063;
geCos[i++]    = -0.8988; geCos[i++]    = -0.8910; geCos[i++]    = -0.8829;
geCos[i++]    = -0.8746; geCos[i++]    = -0.8660; geCos[i++]    = -0.8572;
geCos[i++]    = -0.8480; geCos[i++]    = -0.8387; geCos[i++]    = -0.8290;
geCos[i++]    = -0.8192; geCos[i++]    = -0.8090; geCos[i++]    = -0.7986;
geCos[i++]    = -0.7880; geCos[i++]    = -0.7771; geCos[i++]    = -0.7660;
geCos[i++]    = -0.7547; geCos[i++]    = -0.7431; geCos[i++]    = -0.7314;
geCos[i++]    = -0.7193; geCos[i++]    = -0.7071; geCos[i++]    = -0.6947;
geCos[i++]    = -0.6820; geCos[i++]    = -0.6691; geCos[i++]    = -0.6561;
geCos[i++]    = -0.6428; geCos[i++]    = -0.6293; geCos[i++]    = -0.6157;
geCos[i++]    = -0.6018; geCos[i++]    = -0.5878; geCos[i++]    = -0.5736;
geCos[i++]    = -0.5592; geCos[i++]    = -0.5446; geCos[i++]    = -0.5299;
geCos[i++]    = -0.5150; geCos[i++]    = -0.5000; geCos[i++]    = -0.4848;
geCos[i++]    = -0.4695; geCos[i++]    = -0.4540; geCos[i++]    = -0.4384;
geCos[i++]    = -0.4226; geCos[i++]    = -0.4067; geCos[i++]    = -0.3907;
geCos[i++]    = -0.3746; geCos[i++]    = -0.3584; geCos[i++]    = -0.3420;
geCos[i++]    = -0.3256; geCos[i++]    = -0.3090; geCos[i++]    = -0.2924;
geCos[i++]    = -0.2756; geCos[i++]    = -0.2588; geCos[i++]    = -0.2419;
geCos[i++]    = -0.2250; geCos[i++]    = -0.2079; geCos[i++]    = -0.1908;
geCos[i++]    = -0.1736; geCos[i++]    = -0.1564; geCos[i++]    = -0.1392;
geCos[i++]    = -0.1219; geCos[i++]    = -0.1045; geCos[i++]    = -0.0872;
geCos[i++]    = -0.0698; geCos[i++]    = -0.0523; geCos[i++]    = -0.0349;
geCos[i++]    = -0.0175;

geCos[i++]    =  0.0000; geCos[i++]    =  0.0175;
geCos[i++]    =  0.0349; geCos[i++]    =  0.0523; geCos[i++]    =  0.0698;
geCos[i++]    =  0.0872; geCos[i++]    =  0.1045; geCos[i++]    =  0.1219;
geCos[i++]    =  0.1392; geCos[i++]    =  0.1564; geCos[i++]    =  0.1736;
geCos[i++]    =  0.1908; geCos[i++]    =  0.2079; geCos[i++]    =  0.2250;
geCos[i++]    =  0.2419; geCos[i++]    =  0.2588; geCos[i++]    =  0.2756;
geCos[i++]    =  0.2924; geCos[i++]    =  0.3090; geCos[i++]    =  0.3256;
geCos[i++]    =  0.3420; geCos[i++]    =  0.3584; geCos[i++]    =  0.3746;
geCos[i++]    =  0.3907; geCos[i++]    =  0.4067; geCos[i++]    =  0.4226;
geCos[i++]    =  0.4384; geCos[i++]    =  0.4540; geCos[i++]    =  0.4695;
geCos[i++]    =  0.4848; geCos[i++]    =  0.5000; geCos[i++]    =  0.5150;
geCos[i++]    =  0.5299; geCos[i++]    =  0.5446; geCos[i++]    =  0.5592;
geCos[i++]    =  0.5736; geCos[i++]    =  0.5878; geCos[i++]    =  0.6018;
geCos[i++]    =  0.6157; geCos[i++]    =  0.6293; geCos[i++]    =  0.6428;
geCos[i++]    =  0.6561; geCos[i++]    =  0.6691; geCos[i++]    =  0.6820;
geCos[i++]    =  0.6947; geCos[i++]    =  0.7071; geCos[i++]    =  0.7193;
geCos[i++]    =  0.7314; geCos[i++]    =  0.7431; geCos[i++]    =  0.7547;
geCos[i++]    =  0.7660; geCos[i++]    =  0.7771; geCos[i++]    =  0.7880;
geCos[i++]    =  0.7986; geCos[i++]    =  0.8090; geCos[i++]    =  0.8192;
geCos[i++]    =  0.8290; geCos[i++]    =  0.8387; geCos[i++]    =  0.8480;
geCos[i++]    =  0.8572; geCos[i++]    =  0.8660; geCos[i++]    =  0.8746;
geCos[i++]    =  0.8829; geCos[i++]    =  0.8910; geCos[i++]    =  0.8988;
geCos[i++]    =  0.9063; geCos[i++]    =  0.9135; geCos[i++]    =  0.9205;
geCos[i++]    =  0.9272; geCos[i++]    =  0.9336; geCos[i++]    =  0.9397;
geCos[i++]    =  0.9455; geCos[i++]    =  0.9511; geCos[i++]    =  0.9563;
geCos[i++]    =  0.9613; geCos[i++]    =  0.9659; geCos[i++]    =  0.9703;
geCos[i++]    =  0.9744; geCos[i++]    =  0.9781; geCos[i++]    =  0.9816;
geCos[i++]    =  0.9848; geCos[i++]    =  0.9877; geCos[i++]    =  0.9903;
geCos[i++]    =  0.9925; geCos[i++]    =  0.9945; geCos[i++]    =  0.9962;
geCos[i++]    =  0.9976; geCos[i++]    =  0.9986; geCos[i++]    =  0.9994;
geCos[i++]    =  0.9998; geCos[i++]    =  1.0000;

/*
 * Kludge - for anyone who wants to be locked into 75 dpi fonts
 */
#ifdef VMS

#ifdef GEDPI75

geFntXName[0] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[1] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[2] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[3] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[4] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[5] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[6] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[7] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[8] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[9] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[10] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[11] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[12] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[13] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[14] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[15] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[16] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[17] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[18] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[19] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[20] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[21] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[22] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[23] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[24] =
 "-Adobe-Courier-Medium-R-Normal--*-80-75-75-M-*-ISO8859-1";
geFntXName[25] =
 "-Adobe-Courier-Medium-R-Normal--*-100-75-75-M-*-ISO8859-1";
geFntXName[26] =
 "-Adobe-Courier-Medium-R-Normal--*-120-75-75-M-*-ISO8859-1";
geFntXName[27] =
 "-Adobe-Courier-Medium-R-Normal--*-140-75-75-M-*-ISO8859-1";
geFntXName[28] =
 "-Adobe-Courier-Medium-R-Normal--*-180-75-75-M-*-ISO8859-1";
geFntXName[29] =
 "-Adobe-Courier-Medium-R-Normal--*-240-75-75-M-*-ISO8859-1";
geFntXName[30] =
 "-Adobe-Courier-Bold-R-Normal--*-80-75-75-M-*-ISO8859-1";
geFntXName[31] =
 "-Adobe-Courier-Bold-R-Normal--*-100-75-75-M-*-ISO8859-1";
geFntXName[32] =
 "-Adobe-Courier-Bold-R-Normal--*-120-75-75-M-*-ISO8859-1";
geFntXName[33] =
 "-Adobe-Courier-Bold-R-Normal--*-140-75-75-M-*-ISO8859-1";
geFntXName[34] =
 "-Adobe-Courier-Bold-R-Normal--*-180-75-75-M-*-ISO8859-1";
geFntXName[35] =
 "-Adobe-Courier-Bold-R-Normal--*-240-75-75-M-*-ISO8859-1";
geFntXName[36] =
 "-Adobe-Courier-Bold-O-Normal--*-80-75-75-M-*-ISO8859-1";
geFntXName[37] =
 "-Adobe-Courier-Bold-O-Normal--*-100-75-75-M-*-ISO8859-1";
geFntXName[38] =
 "-Adobe-Courier-Bold-O-Normal--*-120-75-75-M-*-ISO8859-1";
geFntXName[39] =
 "-Adobe-Courier-Bold-O-Normal--*-140-75-75-M-*-ISO8859-1";
geFntXName[40] =
 "-Adobe-Courier-Bold-O-Normal--*-180-75-75-M-*-ISO8859-1";
geFntXName[41] =
 "-Adobe-Courier-Bold-O-Normal--*-240-75-75-M-*-ISO8859-1";
geFntXName[42] =
 "-Adobe-Courier-Medium-O-Normal--*-80-75-75-M-*-ISO8859-1";
geFntXName[43] =
 "-Adobe-Courier-Medium-O-Normal--*-100-75-75-M-*-ISO8859-1";
geFntXName[44] =
 "-Adobe-Courier-Medium-O-Normal--*-120-75-75-M-*-ISO8859-1";
geFntXName[45] =
 "-Adobe-Courier-Medium-O-Normal--*-140-75-75-M-*-ISO8859-1";
geFntXName[46] =
 "-Adobe-Courier-Medium-O-Normal--*-180-75-75-M-*-ISO8859-1";
geFntXName[47] =
 "-Adobe-Courier-Medium-O-Normal--*-240-75-75-M-*-ISO8859-1";
geFntXName[48] =
 "-Adobe-Helvetica-Medium-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[49] =
 "-Adobe-Helvetica-Medium-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[50] =
 "-Adobe-Helvetica-Medium-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[51] =
 "-Adobe-Helvetica-Medium-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[52] =
 "-Adobe-Helvetica-Medium-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[53] =
 "-Adobe-Helvetica-Medium-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[54] =
 "-Adobe-Helvetica-Bold-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[55] =
 "-Adobe-Helvetica-Bold-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[56] =
 "-Adobe-Helvetica-Bold-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[57] =
 "-Adobe-Helvetica-Bold-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[58] =
 "-Adobe-Helvetica-Bold-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[59] =
 "-Adobe-Helvetica-Bold-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[60] =
 "-Adobe-Helvetica-Bold-O-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[61] =
 "-Adobe-Helvetica-Bold-O-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[62] =
 "-Adobe-Helvetica-Bold-O-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[63] =
 "-Adobe-Helvetica-Bold-O-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[64] =
 "-Adobe-Helvetica-Bold-O-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[65] =
 "-Adobe-Helvetica-Bold-O-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[66] =
 "-Adobe-Helvetica-Medium-O-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[67] =
 "-Adobe-Helvetica-Medium-O-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[68] =
 "-Adobe-Helvetica-Medium-O-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[69] =
 "-Adobe-Helvetica-Medium-O-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[70] =
 "-Adobe-Helvetica-Medium-O-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[71] =
 "-Adobe-Helvetica-Medium-O-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[72] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[73] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[74] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[75] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[76] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[77] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[78] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[79] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[80] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[81] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[82] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[83] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[84] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[85] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[86] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[87] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[88] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[89] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[90] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[91] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[92] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[93] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[94] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[95] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[96] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[97] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[98] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[99] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[100] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[101] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[102] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[103] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[104] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[105] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[106] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[107] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[108] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[109] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[110] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[111] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[112] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[113] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[114] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[115] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[116] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[117] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[118] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[119] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[120] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[121] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[122] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[123] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[124] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[125] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[126] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[127] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[128] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[129] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[130] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[131] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[132] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[133] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[134] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[135] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[136] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[137] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[138] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[139] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[140] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[141] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[142] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[143] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[144] =
 "-Adobe-Symbol-Medium-R-Normal--*-80-75-75-P-*-ADOBE-FONTSPECIFIC";
geFntXName[145] =
 "-Adobe-Symbol-Medium-R-Normal--*-100-75-75-P-*-ADOBE-FONTSPECIFIC";
geFntXName[146] =
 "-Adobe-Symbol-Medium-R-Normal--*-120-75-75-P-*-ADOBE-FONTSPECIFIC";
geFntXName[147] =
 "-Adobe-Symbol-Medium-R-Normal--*-140-75-75-P-*-ADOBE-FONTSPECIFIC";
geFntXName[148] =
 "-Adobe-Symbol-Medium-R-Normal--*-180-75-75-P-*-ADOBE-FONTSPECIFIC";
geFntXName[149] =
 "-Adobe-Symbol-Medium-R-Normal--*-240-75-75-P-*-ADOBE-FONTSPECIFIC";
geFntXName[150] =
 "-Adobe-Times-Bold-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[151] =
 "-Adobe-Times-Bold-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[152] =
 "-Adobe-Times-Bold-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[153] =
 "-Adobe-Times-Bold-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[154] =
 "-Adobe-Times-Bold-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[155] =
 "-Adobe-Times-Bold-R-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[156] =
 "-Adobe-Times-Bold-I-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[157] =
 "-Adobe-Times-Bold-I-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[158] =
 "-Adobe-Times-Bold-I-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[159] =
 "-Adobe-Times-Bold-I-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[160] =
 "-Adobe-Times-Bold-I-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[161] =
 "-Adobe-Times-Bold-I-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[162] =
 "-Adobe-Times-Medium-I-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[163] =
 "-Adobe-Times-Medium-I-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[164] =
 "-Adobe-Times-Medium-I-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[165] =
 "-Adobe-Times-Medium-I-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[166] =
 "-Adobe-Times-Medium-I-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[167] =
 "-Adobe-Times-Medium-I-Normal--*-240-75-75-P-*-ISO8859-1";
geFntXName[168] =
 "-Adobe-Times-Medium-R-Normal--*-80-75-75-P-*-ISO8859-1";
geFntXName[169] =
 "-Adobe-Times-Medium-R-Normal--*-100-75-75-P-*-ISO8859-1";
geFntXName[170] =
 "-Adobe-Times-Medium-R-Normal--*-120-75-75-P-*-ISO8859-1";
geFntXName[171] =
 "-Adobe-Times-Medium-R-Normal--*-140-75-75-P-*-ISO8859-1";
geFntXName[172] =
 "-Adobe-Times-Medium-R-Normal--*-180-75-75-P-*-ISO8859-1";
geFntXName[173] =
 "-Adobe-Times-Medium-R-Normal--*-240-75-75-P-*-ISO8859-1";

#else

geFntXName[0] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[1] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[2] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[3] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[4] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[5] =
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[6] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[7] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[8] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[9] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[10] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[11] =
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[12] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[13] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[14] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[15] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[16] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[17] =
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[18] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[19] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[20] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[21] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[22] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[23] =
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[24] =
 "-Adobe-Courier-Medium-R-Normal--*-80-*-*-M-*-ISO8859-1";
geFntXName[25] =
 "-Adobe-Courier-Medium-R-Normal--*-100-*-*-M-*-ISO8859-1";
geFntXName[26] =
 "-Adobe-Courier-Medium-R-Normal--*-120-*-*-M-*-ISO8859-1";
geFntXName[27] =
 "-Adobe-Courier-Medium-R-Normal--*-140-*-*-M-*-ISO8859-1";
geFntXName[28] =
 "-Adobe-Courier-Medium-R-Normal--*-180-*-*-M-*-ISO8859-1";
geFntXName[29] =
 "-Adobe-Courier-Medium-R-Normal--*-240-*-*-M-*-ISO8859-1";
geFntXName[30] =
 "-Adobe-Courier-Bold-R-Normal--*-80-*-*-M-*-ISO8859-1";
geFntXName[31] =
 "-Adobe-Courier-Bold-R-Normal--*-100-*-*-M-*-ISO8859-1";
geFntXName[32] =
 "-Adobe-Courier-Bold-R-Normal--*-120-*-*-M-*-ISO8859-1";
geFntXName[33] =
 "-Adobe-Courier-Bold-R-Normal--*-140-*-*-M-*-ISO8859-1";
geFntXName[34] =
 "-Adobe-Courier-Bold-R-Normal--*-180-*-*-M-*-ISO8859-1";
geFntXName[35] =
 "-Adobe-Courier-Bold-R-Normal--*-240-*-*-M-*-ISO8859-1";
geFntXName[36] =
 "-Adobe-Courier-Bold-O-Normal--*-80-*-*-M-*-ISO8859-1";
geFntXName[37] =
 "-Adobe-Courier-Bold-O-Normal--*-100-*-*-M-*-ISO8859-1";
geFntXName[38] =
 "-Adobe-Courier-Bold-O-Normal--*-120-*-*-M-*-ISO8859-1";
geFntXName[39] =
 "-Adobe-Courier-Bold-O-Normal--*-140-*-*-M-*-ISO8859-1";
geFntXName[40] =
 "-Adobe-Courier-Bold-O-Normal--*-180-*-*-M-*-ISO8859-1";
geFntXName[41] =
 "-Adobe-Courier-Bold-O-Normal--*-240-*-*-M-*-ISO8859-1";
geFntXName[42] =
 "-Adobe-Courier-Medium-O-Normal--*-80-*-*-M-*-ISO8859-1";
geFntXName[43] =
 "-Adobe-Courier-Medium-O-Normal--*-100-*-*-M-*-ISO8859-1";
geFntXName[44] =
 "-Adobe-Courier-Medium-O-Normal--*-120-*-*-M-*-ISO8859-1";
geFntXName[45] =
 "-Adobe-Courier-Medium-O-Normal--*-140-*-*-M-*-ISO8859-1";
geFntXName[46] =
 "-Adobe-Courier-Medium-O-Normal--*-180-*-*-M-*-ISO8859-1";
geFntXName[47] =
 "-Adobe-Courier-Medium-O-Normal--*-240-*-*-M-*-ISO8859-1";
geFntXName[48] =
 "-Adobe-Helvetica-Medium-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[49] =
 "-Adobe-Helvetica-Medium-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[50] =
 "-Adobe-Helvetica-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[51] =
 "-Adobe-Helvetica-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[52] =
 "-Adobe-Helvetica-Medium-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[53] =
 "-Adobe-Helvetica-Medium-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[54] =
 "-Adobe-Helvetica-Bold-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[55] =
 "-Adobe-Helvetica-Bold-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[56] =
 "-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[57] =
 "-Adobe-Helvetica-Bold-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[58] =
 "-Adobe-Helvetica-Bold-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[59] =
 "-Adobe-Helvetica-Bold-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[60] =
 "-Adobe-Helvetica-Bold-O-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[61] =
 "-Adobe-Helvetica-Bold-O-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[62] =
 "-Adobe-Helvetica-Bold-O-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[63] =
 "-Adobe-Helvetica-Bold-O-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[64] =
 "-Adobe-Helvetica-Bold-O-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[65] =
 "-Adobe-Helvetica-Bold-O-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[66] =
 "-Adobe-Helvetica-Medium-O-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[67] =
 "-Adobe-Helvetica-Medium-O-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[68] =
 "-Adobe-Helvetica-Medium-O-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[69] =
 "-Adobe-Helvetica-Medium-O-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[70] =
 "-Adobe-Helvetica-Medium-O-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[71] =
 "-Adobe-Helvetica-Medium-O-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[72] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[73] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[74] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[75] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[76] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[77] =
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[78] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[79] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[80] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[81] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[82] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[83] =
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[84] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[85] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[86] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[87] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[88] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[89] =
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[90] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[91] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[92] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[93] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[94] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[95] =
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[96] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[97] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[98] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[99] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[100] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[101] =
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[102] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[103] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[104] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[105] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[106] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[107] =
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[108] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[109] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[110] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[111] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[112] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[113] =
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[114] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[115] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[116] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[117] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[118] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[119] =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[120] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[121] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[122] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[123] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[124] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[125] =
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[126] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[127] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[128] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[129] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[130] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[131] =
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[132] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[133] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[134] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[135] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[136] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[137] =
 "-Adobe-ITC Souvenir-Light-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[138] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[139] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[140] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[141] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[142] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[143] =
 "-Adobe-ITC Souvenir-Light-I-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[144] =
 "-Adobe-Symbol-Medium-R-Normal--*-80-*-*-P-*-ADOBE-FONTSPECIFIC";
geFntXName[145] =
 "-Adobe-Symbol-Medium-R-Normal--*-100-*-*-P-*-ADOBE-FONTSPECIFIC";
geFntXName[146] =                           
 "-Adobe-Symbol-Medium-R-Normal--*-120-*-*-P-*-ADOBE-FONTSPECIFIC";
geFntXName[147] =
 "-Adobe-Symbol-Medium-R-Normal--*-140-*-*-P-*-ADOBE-FONTSPECIFIC";
geFntXName[148] =
 "-Adobe-Symbol-Medium-R-Normal--*-180-*-*-P-*-ADOBE-FONTSPECIFIC";
geFntXName[149] =
 "-Adobe-Symbol-Medium-R-Normal--*-240-*-*-P-*-ADOBE-FONTSPECIFIC";
geFntXName[150] =
 "-Adobe-Times-Bold-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[151] =
 "-Adobe-Times-Bold-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[152] =
 "-Adobe-Times-Bold-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[153] =
 "-Adobe-Times-Bold-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[154] =
 "-Adobe-Times-Bold-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[155] =
 "-Adobe-Times-Bold-R-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[156] =
 "-Adobe-Times-Bold-I-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[157] =
 "-Adobe-Times-Bold-I-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[158] =
 "-Adobe-Times-Bold-I-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[159] =
 "-Adobe-Times-Bold-I-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[160] =
 "-Adobe-Times-Bold-I-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[161] =
 "-Adobe-Times-Bold-I-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[162] =
 "-Adobe-Times-Medium-I-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[163] =
 "-Adobe-Times-Medium-I-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[164] =
 "-Adobe-Times-Medium-I-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[165] =
 "-Adobe-Times-Medium-I-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[166] =
 "-Adobe-Times-Medium-I-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[167] =
 "-Adobe-Times-Medium-I-Normal--*-240-*-*-P-*-ISO8859-1";
geFntXName[168] =
 "-Adobe-Times-Medium-R-Normal--*-80-*-*-P-*-ISO8859-1";
geFntXName[169] =
 "-Adobe-Times-Medium-R-Normal--*-100-*-*-P-*-ISO8859-1";
geFntXName[170] =
 "-Adobe-Times-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1";
geFntXName[171] =
 "-Adobe-Times-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1";
geFntXName[172] =
 "-Adobe-Times-Medium-R-Normal--*-180-*-*-P-*-ISO8859-1";
geFntXName[173] =
 "-Adobe-Times-Medium-R-Normal--*-240-*-*-P-*-ISO8859-1";
#endif

#endif     
}

/****************************************************************************/

geGlobin2()
{
XFontStruct	*dpsFont;
float		CurRes;

/*
 * Determine resolution of the device
 */
CurRes			= (float)XDisplayWidth(geDispDev, geScreen) /
			  ((float)XDisplayWidthMM(geDispDev, geScreen) / 25.4);
if (fabs(CurRes - GEVSDPI) >= GEMIN_DPI_DELTA)
   {geRun.CurRes = CurRes;
/*
 * Note:  I've decided to back this out for now.  geRun.Sui2 is used to expand
 * the extent box around objects, mostly for the purpose of insuring that
 * autocropping a drawing will not clip off any piece of any object near the
 * edge of the drawing.  I intended this amount to be physically the same at
 * all resolutions, the equivalent of 2 75 dpi pixels, so that drawings will
 * report the same dimensions on all devices regardless of resolution.  However,
 * this then introduces a slight discrepancy between PS and image objects
 * generated from the same metafile.

    CurRes	 = (float)GESUI2 * geRun.CurRes / GEVSDPI;
    GEINT(CurRes, geRun.Sui2);
*/
    geRun.Dpi75  = FALSE;
   }
else 
   { geRun.Dpi75  = TRUE;
     geRun.CurRes = GEVSDPI;
   }		

/*
 * Screen and color map
 */
geDispChar.Planes	= XDisplayPlanes  (geDispDev, geScreen);
geDispChar.Depth	= XDefaultDepth   (geDispDev, geScreen);
geNumCols               = XDisplayCells   (geDispDev, geScreen);
geCmap                  = XDefaultColormap(geDispDev, geScreen);
geVisual                = XDefaultVisual  (geDispDev, geScreen);

geXBPixel               = XBlackPixel(geDispDev, geScreen);
geXWPixel               = XWhitePixel(geDispDev, geScreen);


/*
 * Kludge - memio on ULTRIX is about twice as fast as straight disk, but on
 * VMS it is MUCH slower, so for VMS stick to disk IO
 *
 * It no longer seems to be the case the in-mem io on ULTRIX is faster than
 * straight disk io.  So turning off in-mem io flag, now.  If this works
 * out ok, then eventually should change all "ge*" calls to the routine
 * scontained in "gememio.c" to the normal RTL calls and throw away gememio.c
 * 1-27-92 GNE
 */
#ifdef VMS
geMemIO.InMem           = FALSE;
#else
/* geMemIO.InMem           = TRUE; */
geMemIO.InMem           = FALSE;
#endif

geMemIO.PtrS            = geMemIO.PtrC = geMemIO.PtrE = NULL;
geMemIO.NumBytes        = 0;
geMemIO.RagsOwn         = FALSE;

#ifdef GERAGS

geGrpStat.FSegP         = NULL;

#endif

/*
 * The color allocations of BLACK and WHITE, "must not" fail, so check to
 * see whether or not this is a monochrome system, and if it is just use
 * geXBPixel and geXWPixel;
 */
geBlack.red = geBlack.green = geBlack.blue = 0;
geWhite.red = geWhite.green = geWhite.blue = GEMAXUSHORT;
if (GEMONO)
  {geBlack.pixel         = geXBPixel;
   geWhite.pixel         = geXWPixel;
  }
else
  {/*
    * Pure Black 
    */
   geBlack.pixel          = geAllocColor(geBlack.red,
				       geBlack.green,
				       geBlack.blue, NULL);
   /*
    * Pure WHITE
    */
   geWhite.pixel          = geAllocColor(geWhite.red,
				        geWhite.green,
				        geWhite.blue, NULL);
   } 

geXBPixel               = geBlack.pixel;
geXWPixel               = geWhite.pixel;
#ifdef	GERAGS
/*
 * RAGS' ROOT Window
 */
geRgRoot.parent 	= XRootWindow   (geDispDev, geScreen);
#endif

/*
 * Default Color = Black for Line and Text, Clear for Fill pattern
 */
geAttr.Font 		= GEFONT_USER_DEFAULT;
geAttr.DashIndx	        = 0;
geAttr.DashLen		= 2;
geAttr.LinW             = 0;
geAttr.LinP             = LineSolid;
geAttr.LineHT		= 100;
geAttr.LineStyle  	= FillOpaqueStippled;
geAttr.LinCol.RGB       = geBlack;              /* Def. for outline = black  */
geAttr.LinCol.Name      = "Black";
geAttr.LinCol.OverPrint = FALSE;
geGenRgbToCmyk(&(geAttr.LinCol.RGB), &(geAttr.LinCol.CMYK));
geAttr.FillHT		= 100;
geAttr.FillStyle  	= GETRANSPARENT;
geAttr.FilCol.RGB       = geWhite;              /* Def. col for fill = white */
geAttr.FilCol.RGB.flags = 0;		        /* Def. for fill=TRANSPARENT */
geAttr.FilCol.Name      = "White";
geAttr.FilCol.OverPrint = FALSE;
geGenRgbToCmyk(&(geAttr.FilCol.RGB), &(geAttr.FilCol.CMYK));
geAttr.TxtFgHT		= 100;
geAttr.TxtFgStyle 	= FillOpaqueStippled;
geAttr.TxtFgCol         = geAttr.LinCol;
geAttr.TxtFgCol.Name    = "Black";
geAttr.TxtFgCol.OverPrint = FALSE;
geAttr.TxtBgHT		= 100;
geAttr.TxtBgStyle 	= GETRANSPARENT;
geAttr.TxtBgCol.RGB     = geWhite;
geAttr.TxtBgCol.RGB.flags = 0;		        /* Def. for fill=TRANSPARENT */
geAttr.TxtBgCol.Name    = "White";
geAttr.TxtBgCol.OverPrint = FALSE;
geGenRgbToCmyk(&(geAttr.TxtBgCol.RGB), &(geAttr.TxtBgCol.CMYK));
geAttr.PagHT		= 100;
geAttr.PagStyle         = GETRANSPARENT;	/* Def. Pag Fill =TRANSPARENT*/
geAttr.PagCol.RGB       = geWhite;
geAttr.PagCol.RGB.flags	= 0;		        /* Def. for fill=TRANSPARENT */
geAttr.PagCol.Name      = "White";
geAttr.PagCol.OverPrint = FALSE;
geGenRgbToCmyk(&(geAttr.PagCol.RGB), &(geAttr.PagCol.CMYK));
geAttr.ImgBgCol.RGB     = geWhite;              /* Image background = white  */
geAttr.ImgBgCol.Name    = "White";
geAttr.ImgBgCol.OverPrint = FALSE;
geGenRgbToCmyk(&(geAttr.ImgBgCol.RGB), &(geAttr.ImgBgCol.CMYK));
geAttr.ImgFgCol.RGB     = geBlack;              /* Image foreground = black  */
geAttr.ImgFgCol.Name    = "Black";
geAttr.ImgFgCol.OverPrint = FALSE;
geGenRgbToCmyk(&(geAttr.ImgFgCol.RGB), &(geAttr.ImgFgCol.CMYK));

geMenuState.User_set	= TRUE;
geSeg0                  = NULL;

#ifdef GERAGS

/*
 * Colors
 */
/*
 * Pure RED
 */
if (GEMONO)
  {geRed.pixel = geWhite.pixel;}
else
  {geRed.pixel           = geAllocColor((geRed.red   = GEMAXUSHORT),
					(geRed.green = 0),
					(geRed.blue  = 0), NULL);
  }
/*
 * Pure GREEN
 */
if (GEMONO)
  {geGreen.pixel = geWhite.pixel;}
else
  {geGreen.pixel         = geAllocColor((geGreen.red   = 0),
					(geGreen.green = GEMAXUSHORT),
					(geGreen.blue  = 0), NULL);
  }
/*
 * Pure BLUE
 */
if (GEMONO)
  {geBlue.pixel = geWhite.pixel;}
else
  {geBlue.pixel          = geAllocColor((geBlue.red   = 0),
					(geBlue.green = 0),
					(geBlue.blue  = GEMAXUSHORT), NULL);
  }

#endif

/*
 * Don't trust the defaults ....
 */
geGCV.function   		= GXcopy;
geGCV.plane_mask   		= 0xffffffff;
geGCV.foreground		= geBlack.pixel;
geGCV.background		= geWhite.pixel;
geGCV.line_width   		= 0;
geGCV.line_style   		= LineSolid;
geGCV.cap_style   		= CapButt;
geGCV.join_style   		= JoinMiter;
geGCV.fill_style   		= FillSolid;
geGCV.fill_rule   		= EvenOddRule;
geGCV.arc_mode   		= ArcChord;
geGCV.ts_x_origin      		= 0;
geGCV.ts_y_origin      		= 0;
geGCV.subwindow_mode   		= ClipByChildren;
geGCV.graphics_exposures	= False;
geGCV.clip_x_origin    		= 0;
geGCV.clip_y_origin    		= 0;
geGCV.clip_mask    		= None;
geGCV.dash_offset    		= 0;

geGC2			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);
geGC3			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);
geGC4			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);
geGCImg			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);
geGCPs			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);
geGC0			       	= XDefaultGC(geDispDev, geScreen);
       dpsFont = XLoadQueryFont(geDispDev, GEDPS_FONT_NAME);
       if (!dpsFont)
	    dpsFont = XLoadQueryFont(geDispDev, GEXFIXED_NAME);
       XSetFont(geDispDev, geGC0, dpsFont->fid);
       XSetGraphicsExposures(geDispDev, geGC0, False);

#ifdef GERAGS

geGCV.font       		= geFontMenu->fid;
geGCV.subwindow_mode	       	= IncludeInferiors;
geGC1			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCFont|GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);

geGCV.font       		= geFontMenu->fid;
geGCV.function			= GXcopy;
geGC6			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCFont|GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);

#endif

geGCV.foreground		= GEXORPIXEL;
geGCV.function			= GXxor;
geGCV.plane_mask   		= geXWPixel ^ geXBPixel;
geGC5			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);

geGCV.foreground		= geBlack.pixel;
geGCV.background		= geWhite.pixel;
geGCV.function			= GXcopy;
geGCV.fill_style		= FillOpaqueStippled;
geGCHT			       	= XCreateGC(geDispDev, geRgRoot.self,
       GCFunction|GCPlaneMask|GCForeground|GCBackground|GCLineWidth|
       GCLineStyle|GCCapStyle|GCJoinStyle|GCFillStyle|GCFillRule|GCArcMode|
       GCTileStipXOrigin|GCTileStipYOrigin|
       GCSubwindowMode|GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin|
       GCClipMask|GCDashOffset, &geGCV);


#ifdef GERAGS

XSetDashes(geDispDev, geGC1, 0, &geDashList[geAttr.DashIndx][0],geAttr.DashLen);

#endif

XSetDashes(geDispDev, geGC2, 0, &geDashList[geAttr.DashIndx][0],geAttr.DashLen);

geAttr.CapStyle		= CapButt;
geAttr.JoinStyle	= JoinMiter;
geAttr.WritingMode	= GXcopy;
geAttr.FillWritingMode	= GXcopy;
gePHandler		= geLin;

geOldState		= geState;
geOldAttr		= geAttr;
geSampleAttr		= geAttr;

geHasDPS = -1;	/* Initialization to -1 means this flag has not yet been set */
#ifndef GERAGS
geDrawPS = TRUE;  /* For RAGS, this flag is picked up in the resource file */
#endif

strcpy(geDefColString, "%CMYK");
}

