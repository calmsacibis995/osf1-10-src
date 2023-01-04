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
static char SccsId[] = "@(#)geglobals.c	1.1\t11/22/88";
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
**	GEGLOBALS.C         	  	All Globals are defined here
**
**  ABSTRACT:
**
**      This module contains only definitions, NO executable code
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 12/24/86 Created
**	DAA 11/22/88 Revised
**
**--
**/

#define GEGLOBALS
#include "geGks.h"
#undef	GEGLOBALS

       struct GE_UI     geUI;
       struct GE_RUN    geRun;
       struct GE_MEMIO  geMemIO;
       struct GE_ALLOC_COL *geCol0 = NULL, *geCol0User = NULL, *geColL = NULL;
       struct GE_READ_COL_NAME *geReadColName0 = NULL;
       struct GE_CO	geClip,  geSel, geGenBx, geClipAnim;
       struct GE_TM     geCTM;
       struct GE_ATTR	geAttr,  geOldAttr, geSampleAttr;
       struct GE_DISP_ATTR geDispAttr;
       struct GE_MENUSTATE geMenuState;
       struct GE_SETUP	geSetUp;
       struct GE_STATE	geState, geOldState;
       struct GE_GRP    geGrpStat;
       struct GE_DISPCHAR	geDispChar;
       struct GE_WS	geOldWs, geWs, geWi, gePr, geMi, geMo, geMn;
       struct GE_SEG	*geSeg0 = NULL, *geGrpSel = NULL, *geZRef = NULL,
			*geSegSel = NULL, *geSegCut = NULL;
       struct GE_LIST	*geListSel = NULL;
       struct GE_PAG	*gePag0 = NULL;
       struct GE_AREA   geEditBox;
       struct GE_COL_LIST *geColList0 = NULL;
       struct GE_UNIT   geU;
       struct GE_ROT    geRot;
       struct GE_ALN    geAln;
       struct GE_IMG_REF geImgRef;
       struct GE_PS_REF	gePsRef;
       struct GE_RESOURCE geRM;
       struct GE_INITMANAGE geInitManage;
       struct GE_WIN	geRgRoot;

       char	    	geUtilBuf[GE_MAX_MSG_CHAR],
		        geErrBuf[GE_MAX_MSG_CHAR], /* For error strings      */
               	    	geTitleBuf[GE_MAX_MSG_CHAR], /* Application banner   */
                        *geRagLoc = NULL,       /* Path to location of rags  */
                        *geUsrLoc = NULL,       /* Path to user's HOME dir   */
  			*geInFile = NULL,	/* Input file name	     */
			*geMsgPtr[GE_MAX_MSGS],
			*geFntPtr[GE_MAX_FNTS],
			geDefRags[100],         /* Default Metafile Name     */
			geDefMeta[100],         /* Default Metafile Ext      */
			geDefDDIF[100],         /* Default DDIFmetafile Name */
  			geDefSDML[100],         /* Default SDML file Ext     */
  			geDefLIST[100],         /* Default LIST file Ext     */
			geDefProofPs[100],      /* Default ProofPostScriptExt*/
			geDefProofSix[100],     /* Default ProofSixel Ext    */
			geDefProofDDIF[100],    /* Default ProofSixel Ext    */
			geDefChkPt[100],        /* Default checkpoint prefix */
			geDefProofFSE[100],     /* Default ProofFSE   Ext    */
			geDefProofX[100],       /* Default ProofXfmt  Ext    */
			geDefProofText[100],    /* Default Text File  Ext    */
			geDefDirIn[100],        /* Default Directory - INPUT */
			geDefDirInClip[100],    /* Default Directory - CLIPART INPUT*/
			geDefDirOut[100],       /* Default Directory - OUTPUT*/
			geDefDirInImg[100],     /* Default Directory - INPUT */
			geDefDirOutScrDmp[100], /* Default Directory - OUTPUT*/
			geDefDirInPs[100],	/* Default Directory - INPUT */
			geDashList[4][4];	/* Line Pattern dashes	     */
			geParmC,		/* Byte all purpose var.     */
                        geDefColString[10];	/* Prefix for mixed colors   */
			geEditCmdString[100];   /* Holds current edit command*/
		

       short            geGrey[16],             /* All purpose grey level    */
			geJustTxt, 		/* Txt justify 0=C 1=R 2=L  */
			geMagGrp,	
			geBreakIn, 		/* 1=Send cmd to grp members */
			geReadWriteType, 	/* Meta file type ASCIIorDDIF*/
			geDebug,
			geObjSel,               /* Object selection	     */	
			geImgType,              /* type of image	     */
			geExportFlag; 		/* When calling MOPS 	     */

       int              (*gePHandler)(),        /* Poly sub-seg handler      */
			geIT,                   /* Very volatile             */
			geMultiShellCount,    	/* # of shells 		     */
			geScreen, 
			geVersionIn, 
			geVersionOut,
			geNumCols,
			geColStat,
			geStartIconified,
			geFontMenuWidth,
			geFontMenuHeight;

       long             geGksLevel,             /* Gks Implementation Level  */
			geUtilX[GEMAX_PTS],
			geUtilY[GEMAX_PTS],
                        geVn,                   /* Index for geVert list     */
			geLinWL,	        /* Last Line weight menu val */
    			geErr,			/* Internal Error Code       */
			geGetPutFlgs,           /* Attribute get-put flags   */
                        geAnimMin = GETSUI(10), /* Inside indicator disp cntr*/
			geInqRes,		/* Inquire return value      */
			geMagCx,
			geMagCy,
			geMagMode,
			gePolXStart, gePolYStart,
			geLT1, geLT2;
       unsigned long   	geXBPixel, geXWPixel;
       unsigned long	geUL1, geUL2,
                        geCdaStat, geCdaTrk, geCdaLen,
		     	geCdaAggType,
		     	geCdaRtAgg, geCdaAgg,
			geCdaPrevAgg, geCdaSegAgg,
			geCdaNextAgg,
		     	geCdaStrm, geCdaFile;

      float     	geSin[361],
			geCos[361],
		     	geUtilFX[GEMAX_PTS],    /* For Gks ONLY              */
			geUtilFY[GEMAX_PTS],
			geFT,			/* Highly volatile           */
			geMagFX,
			geMagFY;

       double		geTD;

       XPoint           geVert[GEMAX_PTS];      /* Mostly for filled polys   */
       XFontStruct      *geDefaultFont = 0,
                        *geFontMenu = 0,
                        *geFontUser[GE_MAX_FNTS] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

       XColor           geBlack,                /* Solid BLACK               */
                        geWhite,                /* Pure  WHITE               */
                        geRed,
                        geGreen,
                        geBlue,
                        geMenuFG,
                      	geGenCol;

       FILE             *geFdO,
                        *geFdI,
                        *geFdL;

       Display		*geDispDev = 0;
       Colormap		geCmap = 0;
       Status		geStat;
       GC		geGC0 = 0,		/* Default GC                */
			geGC1 = 0,		/* GC for menu items	     */
			geGC2 = 0,		/* Dynamic GC - obj outline  */
			geGC3 = 0,		/* Dynamic GC - obj fill     */
			geGC4 = 0,		/* Dynamic GC - text         */
			geGC5 = 0,		/* Static  GC - used for XOR */
			geGC6 = 0,		/* Static  GC - used for STAT*/
			geGCImg = 0,		/* Used for images           */
			geGCPs = 0,		/* Used for PS		     */
			geGCHT = 0;		/* GC - h-tone stipple pixmp */
       XGCValues	geGCV;			/* Struct for all GCs	     */

       Cursor		geTxtOCursor = 0,	/* Cursor used in txt ovrstrk*/
			geTxtICursor = 0,	/* Cursor used in txt insert */
			geWaitCursor = 0,	/* Cursor indicating pause   */
			geAnimCursor = 0,	/* Sub for NO Animation cursr*/
		       	geHandCursor = 0,	/* Used for panning          */
		       	geMouseCursor = 0,	/* Used for group/undelete   */
			geFingerCursor,		/* Awaiting Keyboard input   */
			geCrossHairCursor;	/* Used to select regions  */

       Pixmap		geHTStipple[GEMAXHT],	/* Stipples for Half-tones   */
			gePATStipple[GEMAXPAT],	/* Stipples for fills	     */
			geMainIcon,		/* Icon for Main Shell 	     */
			gePagesIcon,		/* Icon for Pages Shell      */
             	       	geSmallIcon,		/* Icon for Small Shell(XUI) */
             	       	geLinPrim_icon,         /* Icons for polygon primitives*/
			geBoxPrim_icon,			
			geBxrPrim_icon,
			geCirPrim_icon,
			geArcPrim_icon,
                        gePiePrim_icon,
                        geElpPrim_icon,
			geLinPrimH_icon,         /* Highlighted version */
			geBoxPrimH_icon,			
			geBxrPrimH_icon,
			geCirPrimH_icon,
			geArcPrimH_icon,
                        gePiePrimH_icon,
                        geElpPrimH_icon;

	char 		geMainIconBits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x02, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfe, 0xf2, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x12,
   0x00, 0x00, 0x00, 0x03, 0x48, 0xfe, 0x12, 0x00, 0x00, 0x80, 0x01, 0x64,
   0xfe, 0x12, 0x00, 0x00, 0xc0, 0x00, 0x72, 0xfe, 0x12, 0x00, 0x00, 0x60,
   0x00, 0x59, 0xfe, 0x12, 0x00, 0x00, 0x30, 0x80, 0x6c, 0xfe, 0x12, 0x00,
   0x00, 0x18, 0x40, 0x76, 0xfe, 0x12, 0x00, 0x00, 0x18, 0x20, 0x7b, 0xfe,
   0x12, 0x00, 0x00, 0x18, 0x90, 0x5d, 0xfe, 0x12, 0x00, 0x00, 0x18, 0xc8,
   0x6e, 0xfe, 0x12, 0x00, 0x00, 0x18, 0x64, 0x77, 0xfe, 0x12, 0x00, 0x00,
   0x18, 0xb2, 0x7b, 0xfe, 0x12, 0x00, 0x00, 0x1c, 0xd9, 0x7d, 0xfe, 0x12,
   0x00, 0x00, 0x9c, 0xec, 0x7e, 0xfe, 0x12, 0x00, 0x80, 0x5f, 0x76, 0x7f,
   0xfe, 0x12, 0x00, 0x40, 0x27, 0xbb, 0x5f, 0xfe, 0x12, 0x00, 0x20, 0x4e,
   0xdd, 0x4f, 0xfe, 0x12, 0x00, 0xe0, 0x9c, 0xee, 0x47, 0xfe, 0x12, 0x00,
   0xe0, 0x39, 0xf7, 0x43, 0xfe, 0x12, 0x00, 0xe0, 0x73, 0xff, 0x41, 0xfe,
   0x12, 0x00, 0x20, 0xe7, 0x07, 0x40, 0xfe, 0x12, 0x00, 0x20, 0xe6, 0x07,
   0x40, 0xfe, 0x12, 0x00, 0xe0, 0xcc, 0x01, 0x40, 0xfe, 0x12, 0x00, 0xf8,
   0x19, 0x01, 0x40, 0xfe, 0x12, 0x00, 0xec, 0xbb, 0x00, 0x40, 0xfe, 0x12,
   0x00, 0xc6, 0x7f, 0x00, 0x40, 0xfe, 0x12, 0x00, 0xe3, 0x00, 0x00, 0x40,
   0xfe, 0x12, 0x80, 0xd1, 0x00, 0x00, 0x40, 0xfe, 0x12, 0xc0, 0x68, 0x00,
   0x00, 0x40, 0xfe, 0x12, 0x60, 0x34, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x30,
   0x1a, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x18, 0x0d, 0x00, 0x00, 0x40, 0xfe,
   0x12, 0x98, 0x06, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x58, 0x03, 0x00, 0x00,
   0x40, 0xfe, 0x12, 0xf8, 0x01, 0x00, 0x00, 0x40, 0xfe, 0x12, 0xfc, 0x00,
   0x00, 0x00, 0x40, 0xfe, 0x12, 0x06, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x12,
   0x03, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x92, 0x01, 0x00, 0x00, 0x00, 0x40,
   0xfe, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x00, 0x00, 0x00,
   0x00, 0x40, 0xfe, 0x12, 0x00, 0x00, 0x00, 0x00, 0x40, 0xfe, 0xf2, 0xff,
   0xff, 0xff, 0xff, 0x7f, 0xfe, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff},
			gePagesIconBits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x02, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfe, 0x02, 0xf0, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x02,
   0x18, 0x00, 0x00, 0x03, 0x48, 0xfe, 0x02, 0x14, 0x00, 0x80, 0x01, 0x64,
   0xfe, 0x02, 0x12, 0x00, 0xc0, 0x00, 0x72, 0xfe, 0x02, 0x11, 0x00, 0x60,
   0x00, 0x59, 0xfe, 0x82, 0x10, 0x00, 0x30, 0x80, 0x6c, 0xfe, 0x42, 0x10,
   0x00, 0x18, 0x40, 0x76, 0xfe, 0x22, 0x10, 0x00, 0x18, 0x20, 0x7b, 0xfe,
   0xf2, 0x1f, 0x00, 0x18, 0x90, 0x5d, 0xfe, 0x12, 0x00, 0x00, 0x18, 0xc8,
   0x6e, 0xfe, 0x12, 0x00, 0x00, 0x18, 0x64, 0x77, 0xfe, 0x12, 0x00, 0x00,
   0x18, 0xb2, 0x7b, 0xfe, 0x12, 0x00, 0x00, 0x1c, 0xd9, 0x7d, 0xfe, 0x12,
   0x00, 0x00, 0x9c, 0xec, 0x7e, 0xfe, 0x12, 0x00, 0x80, 0x5f, 0x76, 0x7f,
   0xfe, 0x12, 0x00, 0x40, 0x27, 0xbb, 0x5f, 0xfe, 0x12, 0x00, 0x20, 0x4e,
   0xdd, 0x4f, 0xfe, 0x12, 0x00, 0xe0, 0x9c, 0xee, 0x47, 0xfe, 0x12, 0x00,
   0xe0, 0x39, 0xf7, 0x43, 0xfe, 0x12, 0x00, 0xe0, 0x73, 0xff, 0x41, 0xfe,
   0x12, 0x00, 0x20, 0xe7, 0x07, 0x40, 0xfe, 0x12, 0x00, 0x20, 0xe6, 0x07,
   0x40, 0xfe, 0x12, 0x00, 0xe0, 0xcc, 0x01, 0x40, 0xfe, 0x12, 0x00, 0xf8,
   0x19, 0x01, 0x40, 0xfe, 0x12, 0x00, 0xec, 0xbb, 0x00, 0x40, 0xfe, 0x12,
   0x00, 0xc6, 0x7f, 0x00, 0x40, 0xfe, 0x12, 0x00, 0xe3, 0x00, 0x00, 0x40,
   0xfe, 0x12, 0x80, 0xd1, 0x00, 0x00, 0x40, 0xfe, 0x12, 0xc0, 0x68, 0x00,
   0x00, 0x40, 0xfe, 0x12, 0x60, 0x34, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x30,
   0x1a, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x18, 0x0d, 0x00, 0x00, 0x40, 0xfe,
   0x12, 0x98, 0x06, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x58, 0x03, 0x00, 0x00,
   0x40, 0xfe, 0x12, 0xf8, 0x01, 0x00, 0x00, 0x40, 0xfe, 0x12, 0xfc, 0x00,
   0x00, 0x00, 0x40, 0xfe, 0x12, 0x06, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x12,
   0x03, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x92, 0x01, 0x00, 0x00, 0x00, 0x40,
   0xfe, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x12, 0x00, 0x00, 0x00,
   0x00, 0x40, 0xfe, 0x12, 0x00, 0x00, 0x00, 0x00, 0x40, 0xfe, 0xf2, 0xff,
   0xff, 0xff, 0xff, 0x7f, 0xfe, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff},
			geSmallIconBits[] = {
   0x00, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x02, 0xcc, 0x00, 0x02, 0xa6, 0x00,
   0x02, 0xd6, 0x00, 0x02, 0xee, 0x00, 0x82, 0xfd, 0x00, 0x42, 0xbb, 0x00,
   0xc2, 0x86, 0x00, 0x42, 0x85, 0x00, 0xe2, 0x83, 0x00, 0x62, 0x80, 0x00,
   0x12, 0x80, 0x00, 0x0d, 0x80, 0x00, 0x02, 0x80, 0x00, 0xfe, 0xff, 0x00,
   0x00, 0x00, 0x00};

/*
 * Font Names
 */
char  *geFntAlloced[GE_MAX_FNTS];                   /* Ptrs to mallocd fonts */
/*
 * Kludge - for anyone who wants to be restriced to 75 dpi fonts
 */
#ifdef GEDPI75

char  *geFntXName[GE_MAX_FNTS] =
{"-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Courier-Medium-R-Normal--*-80-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-100-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-120-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-140-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-180-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-240-75-75-M-*-ISO8859-1",

 "-Adobe-Courier-Bold-R-Normal--*-80-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-100-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-120-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-140-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-180-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-240-75-75-M-*-ISO8859-1",

 "-Adobe-Courier-Bold-O-Normal--*-80-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-100-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-120-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-140-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-180-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-240-75-75-M-*-ISO8859-1",

 "-Adobe-Courier-Medium-O-Normal--*-80-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-100-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-120-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-140-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-180-75-75-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-240-75-75-M-*-ISO8859-1",

 "-Adobe-Helvetica-Medium-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Helvetica-Bold-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Helvetica-Bold-O-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Helvetica-Medium-O-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Demi-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Demi-I-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Light-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Light-I-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Symbol-Medium-R-Normal--*-80-75-75-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-100-75-75-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-120-75-75-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-140-75-75-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-180-75-75-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-240-75-75-P-*-ADOBE-FONTSPECIFIC",

 "-Adobe-Times-Bold-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Times-Bold-I-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Times-Medium-I-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-240-75-75-P-*-ISO8859-1",

 "-Adobe-Times-Medium-R-Normal--*-80-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-100-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-120-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-140-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-180-75-75-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-240-75-75-P-*-ISO8859-1",
 0,0,0,0
 };
#else
char  *geFntXName[GE_MAX_FNTS] =
{"-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Book-O-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Avant Garde Gothic-Demi-O-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Courier-Medium-R-Normal--*-80-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-100-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-120-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-140-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-180-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-R-Normal--*-240-*-*-M-*-ISO8859-1",

 "-Adobe-Courier-Bold-R-Normal--*-80-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-100-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-120-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-140-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-180-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-R-Normal--*-240-*-*-M-*-ISO8859-1",

 "-Adobe-Courier-Bold-O-Normal--*-80-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-100-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-120-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-140-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-180-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Bold-O-Normal--*-240-*-*-M-*-ISO8859-1",

 "-Adobe-Courier-Medium-O-Normal--*-80-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-100-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-120-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-140-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-180-*-*-M-*-ISO8859-1",
 "-Adobe-Courier-Medium-O-Normal--*-240-*-*-M-*-ISO8859-1",

 "-Adobe-Helvetica-Medium-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Helvetica-Bold-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Helvetica-Bold-O-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Bold-O-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Helvetica-Medium-O-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Helvetica-Medium-O-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Book-O-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Lubalin Graph-Demi-O-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Bold-I-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-I-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Demi-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Demi-I-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Demi-I-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Light-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-ITC Souvenir-Light-I-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-ITC Souvenir-Light-I-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Symbol-Medium-R-Normal--*-80-*-*-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-100-*-*-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-120-*-*-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-140-*-*-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-180-*-*-P-*-ADOBE-FONTSPECIFIC",
 "-Adobe-Symbol-Medium-R-Normal--*-240-*-*-P-*-ADOBE-FONTSPECIFIC",

 "-Adobe-Times-Bold-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-R-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Times-Bold-I-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Bold-I-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Times-Medium-I-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-I-Normal--*-240-*-*-P-*-ISO8859-1",

 "-Adobe-Times-Medium-R-Normal--*-80-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-100-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-180-*-*-P-*-ISO8859-1",
 "-Adobe-Times-Medium-R-Normal--*-240-*-*-P-*-ISO8859-1",
 0,0,0,0
 };
#endif

	XComposeStatus	geComposeStatus;

	Visual		*geVisual = NULL;

	int		geHasDPS;
	int		geDrawPS;
