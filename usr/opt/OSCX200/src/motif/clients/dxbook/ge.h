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
/* DEC/CMS REPLACEMENT HISTORY, Element GE.H*/
/* *7     5-MAY-1992 12:20:51 GOSSELIN "removed GESAMPLEDASHLIST (again)"*/
/* *6    30-APR-1992 22:20:52 GOSSELIN "updating with RAGS animation fixes"*/
/* *5    30-MAR-1992 20:01:19 GOSSELIN "removed gesampledashlist"*/
/* *4    19-MAR-1992 13:38:18 GOSSELIN "added new RAGS support"*/
/* *3     3-MAR-1992 17:19:22 KARDON "UCXed"*/
/* *2    16-SEP-1991 13:03:50 BALLENGER "test protections"*/
/* *1    16-SEP-1991 12:48:58 PARMENTER "Rags"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GE.H*/
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
**	GE.H         	  	Graphics Editor major include file
**
**  ABSTRACT:
**
**	Main include file for the Graphics Editor
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 10/29/86 Created
**	DAA 11/22/88 Revised
**
**--
**/

/* static char GESid[] = "@(#)ge.h	1.1 11/22/88";                       */ 

#include <limits.h>
#ifdef VMS
#include <float.h>
#endif
/*
 * Some very basic stuff
 */

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef min
#define min(mp_a,mp_b)	((mp_a)<=(mp_b)?(mp_a):(mp_b))
#endif
#ifndef max
#define max(mp_a,mp_b)	((mp_a)>=(mp_b)?(mp_a):(mp_b))
#endif

#ifdef VMS
#define ImgScale                        img$scale
#define ImgDeleteFrame                  img$delete_frame
#define ImgRotate                       img$rotate
#define ImgGetFrameAttributes           img$get_frame_attributes
#define ImgDecompress                   img$decompress
#define ImgCopy                         img$copy
#define ImgExportBitmap                 img$export_bitmap
#define ImgSetFrameAttributes           img$set_frame_attributes
#define ImgCreateFrame                  img$create_frame
#define ImgImportBitmap                 img$import_bitmap
#define ImgOpenDDIFFile                 img$open_ddif_file
#define ImgCloseDDIFFile                img$close_ddif_file
#define ImgImportDDIFFrame              img$import_ddif_frame
#define ImgExportDDIFFrame              img$export_ddif_frame
#endif

#define GEZERO				0
#define GEMAXLONG                       LONG_MAX
#define GEMAXINT	                INT_MAX
#define GEMAXSHORT                      SHRT_MAX
#define GEMAXULONG                      ULONG_MAX
#define GEMAXUSHORT                     USHRT_MAX
#define GEMAXUSHORT3                    (3 * GEMAXUSHORT)
#define GEMAXFLOAT                      FLT_MAX
#define	GESCRSIZE			14
#define GEXORPIXEL                      0xffffffff
/*
 * The following is used to restrict the size of the coordinates which are
 * fed to XPolygonRegion (used in the selection code) because it blows up
 * if fed points whose value exceeds this number.  This can happen
 * most notably if a drawing is zoomed up greatly.
 */
#define GEMAXVERT                       10000
/*
 * Small Internal Units - SUIs.  One screen pixel is 1024 SUIs (2 ** 10).
 *This may change in the future, if it is deemed inadequate.  Given that
 *the screen resolution today (Sep. 25, 1987) is approx. 78 pixels/in. (VSII),
 *this translates into 79,872 SUIs/in., well above the resolution of any
 *printers currently out.  Another reason for the hi-res internal coordinate
 *base is to mitigate round-off errors in functions such as zoom up-down.
 *Note:  SUIs should be kept at the minimal acceptable level to avoid overflow
 *in arithematic operations.
 */
#define GEVSDPI          78.106905              /* DPI of 19" VSII           */
#define GEMIN_DPI_DELTA  10.                    /* GEVSDPI+-this still 75dpi */
#define	GEFSUIPP	 1024.			/* Suis per pixel            */
#define	GESUI		 10			/* Power of 2 for SUIs	     */
#define	GESUIPP	 	(1 << GESUI)		/* SUIs per pixel	     */
#define	GESUI1	 	(1 << GESUI)		/* SUI equivalent of 1 pixel */
#define	GESUI2	 	(GESUI1 << 1)		/* SUI equivalent of 2 pixels*/
#define	GESUIPH	 	(GESUIPP >> 1)		/* SUIs per Half pixel	     */
#define	GETSUI(mp_x)	((mp_x) << GESUI)	/* Transform Pixels to SUIs  */
/*
 * Transform SUIs to pixels - take into consideration the sign for 
 *rounding purposes. Rounding x; (e.g. x = 3, -3)
 *		 3.0 to  3.4999...	 3
 *		-3.0 to -3.4999...	-3
 *		-3.5 to -3.9999...	-4
 *		 3.5 to  3.9999...	 4
 */
#define	GETPIXP(mp_x)	(((mp_x) + GESUIPH) >> GESUI)
#define	GETPIXN(mp_x)	((mp_x) >> GESUI)
#define	GETPIX(mp_x)	((mp_x) >= 0 ? GETPIXP(mp_x) : GETPIXN(mp_x))
/*
 * Kludge - as soon as you find the keysym definitions - trash these
 */
#define GEKSYM_TAB	65289			/* KeySym for tab	key  */
#define GEKSYM_UAR	65362			/* KeySym for UP  arrow	key  */
#define GEKSYM_LAR	65361			/* KeySym for LEFTarrow	key  */
#define GEKSYM_DAR	65364			/* KeySym for DOWNarrow key  */
#define GEKSYM_RAR	65363			/* KeySym for RIGHTarrowkey  */
#define GEKSYM_CR	65293			/* KeySym for CariageRetkey  */
#define GEKSYM_DEL	65535			/* KeySym for DELETE	key  */
#define GEKSYM_ENT	65421			/* KeySym for ENTER	key  */
#define GEKSYM_HLP	65386			/* KeySym for HELP	key  */
#define GEKSYM_DO	65383			/* KeySym for DO	key  */
#define GEKSYM_SP	32			/* KeySym for Space bar      */
#define GEKSYM_TIL	126			/* KeySym for tilda "~"	     */
#define GEKSYM_ALT_COM	65452			/* KeySym for "," ALT   pad  */
#define GEKSYM_ALT_9	65465			/* KeySym for "9" ALT   pad  */

#define GEIsKeypadKey(keysym) \
    ( (unsigned)keysym >= XK_space && (unsigned)keysym <= XK_BackSpace)
#define GEIsAltKeypadKey(keysym) \
    ( (unsigned)keysym >= XK_KP_Separator && (unsigned)keysym <= XK_KP_9)
/*
 * FILE definitions - found in the file pulldown menu   
 * gefileops.c references these
 *
 * Note that since these IMGTYPEs are written to the metafile (and used to
 * determine the data type of the image for the case of the image
 * storage method being REFERENCE) they must NEVER change - FURTHERMORE
 * these MUST match the image create pull-down return values, which is used
 * as the image priv->Type that finally gets written to the metafile.
 */
#define GEIMGTYPE_NONE                 -2
#define GEIMGTYPE_END_LIST             -1
#define GEIMGTYPE_X                     0
#define GEIMGTYPE_FSE                   1
#define GEIMGTYPE_SIX                   2
#define GEIMGTYPE_PS                    3
#define GEIMGTYPE_UIS                   4
#define GEIMGTYPE_DDIF                  5
#define GEIMGTYPE_RAGS                  6 
#define GEIMGTYPE_TEXT                  7
#define GE_IMAGE_READ	                8
#define GE_FILE_OPEN	                9
#define GE_FILE_INCLUDE                 10
#define GE_FILE_IMPORT_TXT              11
#define GE_FILE_SAVE	                12
#define GE_FILE_SAVEAS	                13
#define GE_FILE_IMPORT_PS               14
#define GE_FILE_PREVIEW                 15
#define GE_FILE_PREVIEWAS               16

/*
 * EDIT definitions - found in global pulldown menu
 * geeditops.c references these
 */
#define GE_EDIT_JOINALL	        0
#define GE_EDIT_JOINALLTXT	1
#define GE_EDIT_XDEL	        2
#define GE_EDIT_SELALL	        3
#define GE_EDIT_REVEAL	        4
#define GE_EDIT_MARK_VP         5
#define GE_EDIT_SHIFT_VP        6
#define GE_EDIT_ROUND	        7
#define GE_EDIT_PASTE	        8
#define GE_EDIT_CUT	        9

#define GECMAP_NEW_SAV  0
#define GECMAP_NEW      1

#define GESET_MARK_1    0
#define GESET_MARK_2    1
#define GESET_MARK_3    2
#define GESET_MARK_4    3

#define GESHIFT_PREV    0
#define GESHIFT_TOPT    1
#define GESHIFT_L       2
#define GESHIFT_R       3
#define GESHIFT_T       4
#define GESHIFT_B       5
#define GESHIFT_C       6
#define GESHIFT_UL      7
#define GESHIFT_UR      8
#define GESHIFT_LL      9
#define GESHIFT_LR      10
#define GESHIFT_MARK_1  11
#define GESHIFT_MARK_2  12
#define GESHIFT_MARK_3  13
#define GESHIFT_MARK_4  14
/*
 * READ-WRITE pull-down definitions - the numbers correspond to entry position
 *in the pull-down counting from the top down (starting at 0)
 */
#define GE_RW_ASCII     0
#define GE_RW_DDIF      1
/*
 * Rotation direction definitions
 */
#define GECLOCK				-1
#define GECCLOCK			1
/*
 * Set-up options
 */
#define	GEHILIGHT_BLINK			0
#define	GEHILIGHT_BOX			1
#define GEIMGSTORAGE_DATA               0
#define GEIMGSTORAGE_REF                1
#define	GEANIM_LIN			0
#define	GEANIM_TRUE			1
#define	GEANIM_BOX			2
#define	GEANIM_NONE			3 
/*
 * Bits in IMAGE flags
 */
#define GEIMGSTORAGE                    1
#define GEIMGOPACITY_CLR                2
#define GEV5IMG		                4
/*
 * Bits in PS object flags
 */
#define GEPSSTORAGE			1
#define GEPSOPACITY_CLR			2
#define GEPSCOMPLIMENTED		4
#define GEPSXORDRAWN			8
/*
 * General
 */
#define	GEDOWN				1
#define	GEUP				-1
#define	GESELRAD			4
#define GEQUIETMSG                      30
#define GEDASHSTYLE                     LineOnOffDash
#define GEPANTHRES			15
#define GECURRENTVERSION		8
/*
 * X Event types
 */
#define	GE_Kd	KeyPress			/* Keyboard key pressed	     */
#define GE_Ku	KeyRelease			/* Keyboard key released     */
#define	GE_Bd	ButtonPress			/* Mouse button pressed	     */
#define GE_Bu	ButtonRelease			/* Mouse button released     */
#define	GE_We	EnterNotify			/* Mouse entering window     */
#define	GE_Wx	LeaveNotify			/* Mouse leaving window      */
#define GE_Wd	UnmapNotify			/* Window is unmapped	     */
#define GE_Wu	Expose				/* Full windo chngd &or xposd*/
#define GE_Wr 	ConfigureRequest                /* Window reconfigure        */
#define	GE_Mx	MotionNotify			/* Mouse moves within window */
#define	GE_Km	MappingNotify			/* Keyboard map change       */
#define	GE_Cm	ColormapNotify			/* Colormap change           */
#define GE_Ws   (GE_We|GE_Wx|GE_Bd)	        /* Select menu item-PSUEDO   */
#define GE_Xs   ~(GE_Ws) 			/* Deselect menu item-PSUEDO */
/*
 * X Event masks
 */
#define	GE_MKd	KeyPressMask			/* Keyboard key pressed	     */
#define GE_MKu	KeyReleaseMask			/* Keyboard key released     */
#define	GE_MBd	ButtonPressMask			/* Mouse button pressed	     */
#define GE_MBu	ButtonReleaseMask		/* Mouse button released     */
#define	GE_MWe	EnterWindowMask			/* Mouse entering window     */
#define	GE_MWx	LeaveWindowMask			/* Mouse leaving window      */
#define GE_MWd	UnmapNotify			/* Window is unmapped	     */
#define GE_MWu	ExposureMask			/* Full windo chngd &or xposd*/
#define GE_MWr	StructureNotifyMask|SubstructureNotifyMask
                                                /* Window reconfigure        */
#define	GE_MMx	PointerMotionMask		/* Mouse moves within window */
#define	GE_MCm	ColormapChangeMask		/* Colormap change           */
#define GE_MWs  (GE_MWe|GE_MWx|GE_MBd|GE_MWu)   /* Select menu item-PSUEDO   */
#define GE_MXs  (GE_MWs|GE_MBu)			/* Deselect menu item-PSUEDO */
/*
 * Mouse Buttons & Motion Constraints
 */
#define GEBUT_L				Button1
#define GEBUT_M				Button2
#define GEBUT_R				Button3
#define GEMOU_X                         -1      /* ONLY Horiz Mouse move acpt*/
#define GEMOU_A                         0       /* Mouse may roam freely     */
#define GEMOU_Y                         1       /* ONLY Vert  Mouse move acpt*/
#define GEMOU_D                         2       /* ONLY Diag  Mouse move acpt*/

#define	GENOCONSTRAINT			0	/* No constraints at all     */
#define	GEVERTICAL			1	/* Only VERTICAL manip allowd*/
#define	GEHORIZONTAL			2	/* Only HORIZON  manip allowd*/

/*
 * Text justification
 */
#define GEJUSTTXTC			0	/* CENTERED                  */
#define GEJUSTTXTR			1	/* RIGHT justified           */
#define GEJUSTTXTL			2	/* LEFT  justified           */

/*
 * Key grab modifier mask
 */
#define GEANYMODIFIER 			ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask
/*         
 * Radians <--> degrees
 */
#define	GE_PI				3.141592
#define	GE_2PI				(2. * GE_PI)
#define GE_RAD_TO_DEG			(360. / GE_2PI)
#define GE_DEG_TO_RAD			(GE_2PI / 360.)
#define GESIND(mp_x)			(sin((mp_x) * GE_DEG_TO_RAD))
#define GECOSD(mp_x)			(cos((mp_x) * GE_DEG_TO_RAD))
#define ATAND(mp_x,mp_y)		(atan2((mp_x), (mp_y)) * GE_RAD_TO_DEG)
#define GE_WORLD_TO_DEV			1. / 100.
#define GE_DEV_TO_WORLD			100.
/*
 * Work station ID's - TRASH when rid of GKS
 */
#define	GE_WS_EDIT_ID			1
#define	GE_WS_PRINT_ID			2
#define GE_WS_MI_ID			3
#define GE_WS_MO_ID			4
#define GE_WS_WISS_ID			5
#define GE_WS_MENU_ID			6
/*
 * Fonts
 */
#define GE_MENU_FONT			-4;           
/*
 * Units and resolution stuff                                      
 */
#define GE_CM_PER_INCH			2.540
#define GE_POINTS_PER_PICA		12.0
#define GE_POINTS_PER_CM   		28.4527551
#define GE_POINTS_PER_METER   		2845.27551
#define GE_POINTS_PER_INCH 		72.27
#define GE_PICAS_PER_CM	   		2.3710629
#define GE_PICAS_PER_METER   		237.10629
#define GE_PICAS_PER_INCH  		6.0225
/*
 * Limits
 */
#define GE_MAX_MSGS			92
#define GE_MAX_FNTS			1000
#define GE_MAX_SYSTEM_FNTS 		174
#define GE_MAX_USER_FNTS		(GE_MAX_FNTS - GE_MAX_SYSTEM_FNTS)
#define GE_MAX_COLS 			1000
#define GE_MAX_MSG_CHAR			1000
#define GE_MAX_LW                       7
#define GE_MAX_UNITS                    50
#define GEMAX_PTS                       10000
#define GEMSIZE				16
#define GEMAXHT				257
#define GEMAXPAT			30
#define GEMAX_FILE_LEN			34

#ifdef GERAGS                             	
#define GEMAX_WIDGETS 			1024    
#else                                           /* don't need a big heirarchy */
#define GEMAX_WIDGETS 			10      /* for voyer or mops */
#endif
/*
 * Internal, communications fonts
 * Menu, Help, etc - all text messages that come from the ascii start-up file
 */
#define	GEXFIXED_NAME			"fixed"
#define	GEDPS_FONT_NAME			"-Adobe-Helvetica-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1"
#define GEFONT_DEFAULT			0
#define GEFONT_STATUS			1
#define GEFONT_LIST			2
#define GEFONT_USER_DEFAULT		26
#define GEFONT_EXIT_MSG			0
/*             
 * Error codes 
 */
#define GERRXOPENMETAI     		1 	/* Can't OPEN Meta In  file  */
#define GERRVERSION     		2 	/* In Meta version mismatch  */
#define GERRBADMETA     		3 	/* Input Meta file is corrupt*/
#define GERRBADIMAGE 			4 	/* Input Image file is corrupt*/
#define GERRDDIFO			5       /* Error returned from DDIF  */
#define GERRUNAVAILABLE                 6       /* Not yet available         */
#define GERRXOPEN     		        7 	/* Can't OPEN a file         */
#define GERRWRONGWINDOW		        8 	/* Button pick in wrong win  */
#define GERRNOTMETA     		9 	/* NOT a meta file           */
#define GERROUTOFCOLS     		10 	/* Color map is exhausted    */
#define GERRGRAVERSION     		11 	/* gra.dat version skew      */
#define GERRPIXMAPALLOC     		12 	/* Pixmapalloc has failed    */
/*
 * Miscellaneous
 */
#define GEBEEP_DONE                    -50      /* Beep for lengthy op DONE  */
#define GEBEEP_ERR                      4       /* Beep for an ERROR         */
#define GETRANSPARENT	(~(FillSolid|FillTiled|FillStippled|FillOpaqueStippled))
                                                /* Turns out to  be -4       */
#define GEDESCRIP_BEG_V8		"GRA-BEG-COM"
#define GEDESCRIP_END_V8		"GRA-END-COM"
#define GEDESCRIP_BEGIN			"GRA-BEGIN-DRAWING-DESCRIPTION"
#define GEDESCRIP_END			"GRA-END-DRAWING-DESCRIPTION"
#define GEPSDATA_BEGIN			"GRA-BEGIN-PS-DATA"
#define GEPSDATA_END			"GRA-END-PS-DATA"
/*
 * Attributes Get-Put bit flags
 */
#define GEGETPUTCOL			1	/* Bit  0 - color	     */
#define GEGETPUTHT			2	/* Bit  1 - half-tone fill   */
#define GEGETPUTSTYLE			4	/* Bit  2 - writing style    */
#define GEGETPUTWRITEMODE		8	/* Bit  3 - writing mode     */
#define GEGETPUTCAP			16	/* Bit  4 - cap style	     */
#define GEGETPUTJOIN			32	/* Bit  5 - join style	     */
#define GEGETPUTLINW			64	/* Bit  6 - line weight	     */
#define GEGETPUTLINP			128	/* Bit  7 - line pattern     */
#define GEGETPUTOPRINT                  256     /* Bit  8 - OverPrint flag   */

#define GEMAIN_ICON_WIDTH	50
#define GEMAIN_ICON_HEIGHT	50
#define GESMALL_ICON_WIDTH	17
#define GESMALL_ICON_HEIGHT	17

/*
 * Color allocation status flags - allocation of colors is a fairly con-
 * voluted procedure; as such, these flags are only SET by geColCr, it is
 * the responsibility of the interested party to clear the flag before
 * geColCr is called
 */
#define GECOL_CMAP_FILLED_PASS1		1	/* Bit  0 - cmap filled,maybe*/
#define GECOL_CMAP_FILLED_PASS2		2	/* Bit  1 - cmap filled,defnt*/
#define GECOL_CMAP_INHIBIT_FREE		4	/* Bit  2 - DON'T free unused*/
#define GECOL_CMAP_INHIBIT_FREE_COND	8	/* Bit  3 - DON'T free if    */
						/* PASS1 filled flag is set  */
#define GECOL_CMAP_NOALLOC		16	/* Bit  4 - alloc failed     */
#define GECOL_OKTO_REPORT_NOALLOC	32	/* Bit  5 - REPORT alloc fail*/
/*
 * Crop types
 */
#define GECROPMAN                       0       /* Crop to user specified box*/
#define GECROPWIN                       1       /* Crop to window size       */
#define GECROPAUTO                      2       /* Corp to data in drawing   */

/*
 * Typedefs                      
 */
struct GE_GEO					/* Used for geometry of      */
       {Position 		x;		/* application widgets 	     */
	Position 		y;
	Dimension		width;
	Dimension		height;
	short			manage_flag;
       };
struct GE_ACC					/* Accelerators		     */
       {char			Move;
	char			Scale;
	char			Edit;
	char			Copy;
	char			Delete;
	char			Mirror;
	char			CopyMirror;
	char			Group;
	char			Ungroup;
	char			AboveObj;
	char			BelowObj;
	char			Top;
	char			Bottom;
	char			Hide;
	char			Complement;
	char			Rotate;
	char			Align;
	char			NoOp;
	char			Arc;
	char			Circle;
	char			Ellipse;
 	char			Line;
	char			Rectangle;
	char			RoundRect;
	char			Pie;
	char			Polygon;
	char			Horizontal;
	char			Vertical;
	char			EditPoints;
	char			IgnoreGroups;
	char			Gravity;
	char			CreateToSize;
	char			StackOnTop;
	char			Clockwise;
	char			Interactive;
       };                               
/*
 * Resource management parms
 */
struct  GE_RESOURCE
        {XrmDatabase		UserDb;
	 XrmDatabase		SysDb;
	 struct GE_GEO		Main;
	 struct GE_GEO		Panel;
	 struct GE_GEO		Hints;
	 struct GE_GEO		Grid;
	 struct GE_GEO		Fonts;
	 struct GE_GEO		Units;
	 struct GE_GEO		Attr;
	 struct GE_GEO		Customize;
	 struct GE_ACC		Acc;
	 float			ZoomUserDef;
        };

struct	GE_INITMANAGE
	{int		attr_init;
	 int		fonts_init;
	 int		units_init;
	 int		grid_init;
	 int		hints_init;
	 int		customize_init;
	};

/*
 * Run parameters
 */
struct  GE_RUN
	{float		CurRes;
	 unsigned long	Sui2;
	 unsigned	Dpi75		: 1;
	 unsigned	ErrReport	: 1;
	 unsigned	MopsCalling	: 1;
	 unsigned	RagsCalling	: 1;
	 unsigned	VoyerCalling	: 1;
	 unsigned			: 0;
	};
/*
 * In Memory I/O
 */
struct  GE_MEMIO
        {char           *PtrS;
         char           *PtrC;
         char           *PtrE;
	 int            NumBytes;
	 unsigned	InMem           : 1;
	 unsigned	RagsOwn         : 1;
	 unsigned			: 0;
        };
struct 	GE_PT
	{long	x;
	 long	y;
	};

struct 	GE_FPT
	{float	x;
	 float	y;
	};

struct	GE_READ_COL_NAME
       {struct	GE_READ_COL_NAME	*Next;
	char                            *Name;
       };

struct	GE_ALLOC_COL
       {struct	GE_ALLOC_COL	*Next;
	XColor                  Col;
       };

struct 	GE_CMYK
        {unsigned short  C;
	 unsigned short  M;
	 unsigned short  Y;
	 unsigned short  K;
        };

struct 	GE_COL_OBJ
        {char   		*Name;
	 short  		OverPrint;
	 XColor                 RGB;
	 struct GE_CMYK         CMYK;
        };

struct 	GE_COL_LIST
        {struct GE_COL_LIST             *Next;
	 struct GE_COL_OBJ              Col;
        };

struct 	GE_CO
	{long	x1;
	 long	y1;
	 long	x2;
	 long	y2;
	};
          
/*
 * Transformation matrix
 *
 *      [ a  b  0 ]
 *      [ c  d  0 ]
 *      [ tx ty 1 ]
 */
struct GE_TM
        {float  a;
	 float  b;
	 float  c;
	 float  d;
	 float  tx;
	 float  ty;
        };
          
struct 	GE_BX
	{long	x1;
	 long	y1;
	 long	x2;
	 long	y2;
	 long	x3;
	 long	y3;
	 long	x4;
	 long	y4;
	};
          
struct 	GE_SZ
	{long	w;
	 long	h;
	};          
          
struct	GE_RG
	{struct	GE_CO 	Co;
	 struct GE_SZ 	Sz;
	};
struct GE_IMG_REF
        {int            Num;
	 XImage         *Ptr;
        };

struct GE_PS_REF
        {int            Num;
	 unsigned char	*Ptr;
        };
/*
 * Viewport Marks
 */
struct  GE_VP_MARKS
       {struct GE_PT            Mark1;
        struct GE_PT            Mark2;
        struct GE_PT            Mark3;
        struct GE_PT            Mark4;
        struct GE_PT            MarkPrev;
       };
/*
 * Visual Attributes - volatile
 */
struct  GE_ATTR
	{struct GE_COL_OBJ         FilCol;
	 struct GE_COL_OBJ         LinCol;
	 struct GE_COL_OBJ         TxtBgCol;
	 struct GE_COL_OBJ         TxtFgCol;
	 struct GE_COL_OBJ         PagCol;
	 struct GE_COL_OBJ         ImgBgCol;
	 struct GE_COL_OBJ         ImgFgCol;
	 struct GE_COL_OBJ         Col;
	 long		ColorList;	/* used for color list index */
	 long	        MnCur;
	 long	        Font;
	 long           LinW;
	 char           LinP;
	 char		DashIndx;
	 int		DashLen;
	 short		FillHT;
	 int		FillStyle;
	 int		FillWritingMode;
	 short		LineHT;
	 int		LineStyle;
	 int		WritingMode;
	 int		CapStyle;
	 int		JoinStyle;
	 short		TxtBgHT;
	 int		TxtBgStyle;
	 short		TxtFgHT;
	 int		TxtFgStyle;
	 short		PagHT;
	 int		PagStyle;
	};
/*
 * Display attributes - used to render color drawings as half-tone
 * representations on monochrome workstations.
 */
struct  GE_DISP_ATTR
	{int		BgHT;
	 int		BgStyle;
	 unsigned long  BgPixel;
	 float		BgLumi;
	 int		FgHT;
	 int		FgStyle;
	 unsigned long  FgPixel;
	 float		FgLumi;
	};

/*
 * Menu State
 */
struct  GE_MENUSTATE
	{unsigned	User_set 	: 1;
	 unsigned			: 0;
	};
/*
 * Gravity
 */
struct  GE_GRAV
        {struct GE_PT   OPt;
         struct GE_PT   Pt;
	 long           Dx;
	 long           Dy;
	 long           Rad;
	 short          Sign;
	 unsigned	Align           : 1;
	 unsigned       Lock            : 1;
	 unsigned			: 0;
        };
/*
 * Grid
 */
struct  GE_GRID
        {long           Xorg;			/*  All grid parameters are  */
         long           Yorg;			/* in ZOOMED SUIs	     */
         long           MajorX;			/* These are used for display*/
	 long           MajorY;			/* and snap calculations.    */
	 long           MinorX;			/* MinorX,Y are derived from */
	 long           MinorY;			/* from FMajorX,Y and DivX,Y */
	 float		FMajorX;		/* Major grid spacing in     */
	 float		FMajorY;		/* zoomed SUIs		     */
         short          DivX;			/* The number of divisions   */
         short          DivY;			/* between MAJOR grid lines  */
	 unsigned	On              : 1;
	 unsigned	Top             : 1;
	 unsigned	XAlign          : 1;
	 unsigned	YAlign          : 1;
	 unsigned       Lock            : 1;
	 unsigned			: 0;
        };
/*
 * Zoom parameters
 */
struct	GE_ZOOM
        {int		ZoomF;
	 float		ZoomCum;
	 long		ZoomCx;
	 long		ZoomCy;
	};
/*
 * Checkpoint - 
 */
struct	GE_CHKPT
        {int		DoItAt;
	 int		Cur;
	 short		On;
	};
/*
 * Display Characteristics
 */
struct	GE_DISPCHAR
        {int		Planes;
	 int		Depth;
	};
/*
 * Set-Up
 */
struct  GE_SETUP
	{unsigned	Blink		: 1;
	 unsigned	LinWModOnMag	: 1;
	 unsigned			: 0;
	};
/*
 * Operational STATE - volatile
 */
struct  GE_STATE
	{struct GE_PAG	*APagP;
	 struct GE_PAG	*FileIOPagP;
	 struct GE_SEG	*ASegP;
	 struct GE_SEG	*GSegP;
	 struct GE_SEG	*PGrpP;
	 struct GE_XDO	*XDoP;
	 struct GE_GRAV Grav;
	 struct GE_GRID Grid;
	 struct GE_PT 	Mouse;
	 struct GE_CHKPT ChkPt;
         struct GE_VP_MARKS VpMarks;
         XEvent         Event;
	 Window		Window;
  	 Drawable	Drawable;
	 Pixmap		Pixmap;
	 char           MouLock;
	 char           MouLockCr;
	 char           Constraint;
	 short		HilightMode;
	 short		AnimDispMode;
	 short          MsgNum;
	 short          Tab;
	 short		SelRadC;
	 short		CropMode;
	 short		stmp1;
	 short		stmp2;
	 short		stmp3;
	 short		stmp4;
	 short		stmp5;
	 long		LiveSegs;	
	 long           VisibleSegs;
	 long		EditCmd;
	 long		EditMsg;
	 long		Mode;
	 long           Dx;
	 long           Dy;
	 long		State;
	 long		DigMode;
	 long		GraDatVersionMatch;
	 long		HaltCmd;
	 long		ltmp3;
	 long		ltmp4;
	 long		ltmp5;
	 unsigned	ScrAuto		: 1;
	 unsigned	GrpEdit		: 1;
	 unsigned	Exit            : 1;
	 unsigned	InAnim		: 1;
	 unsigned	InCrop		: 1;
	 unsigned	ZoomLineThick   : 1;
	 unsigned	DamageRepair	: 1;
	 unsigned	AnimCursor	: 1;
	 unsigned	ObjEdCr	        : 1;
	 unsigned	EditCmdTmp      : 1;
	 unsigned       ImgStorage      : 1;
	 unsigned       CutOn           : 1;
	 unsigned       PrintPgBg       : 1;
	 unsigned       PrintPgBgNu     : 1;
	 unsigned       RoundCoordsOnSave  : 1;
	 unsigned       Insert          : 1;
	 unsigned       EditPts         : 1;
	 unsigned       StackOnTop      : 1;
	 unsigned       Confirm         : 1;
	 unsigned       GroupOnImport   : 1;
	 unsigned       ManualPlacement : 1;
	 unsigned       GroupOnOpen 	: 1;
	 unsigned       AutoApply       : 1;
	 unsigned       LinWPoints      : 1;
	 unsigned       HintsAllowed    : 1;
	 unsigned       ForceHTDisp     : 1;
	 unsigned       DigConvCol      : 1;
	 unsigned       Hidden          : 1;
	 unsigned       Clipart         : 1;
	 unsigned       ObjAction       : 1;
	 unsigned       AutoMove        : 1;
	 unsigned       WhiteOut        : 1;
	 unsigned       AnimPreview     : 1;
	 unsigned       InCreateMode    : 1;
	 unsigned       NeedToPostPixmap : 1;
	 unsigned       Locked          : 1;
	 unsigned       HaltRequested   : 1;
	 unsigned       tmpbit7         : 1;
	 unsigned       tmpbit8         : 1;
	 unsigned       tmpbit9         : 1;
	 unsigned       tmpbit10        : 1;
	 unsigned			: 0;
	}; 

/*
 * Window definitions - used to be essentially BatchFrame in X10
 */
struct  GE_WIN
	{Window				parent;
	 Window				self;
	 short				x;
	 short				y;
	 short				width;
	 short				height;
	 short				bdrwidth;
	 unsigned long			BgNorm;
	 unsigned long			BgCur;
	 unsigned long			background_pixel;
	}; 
/*
 * Units
 */
struct  GE_UNIT
        {short          Map;
         short          LMap;
	 short          DMap;			/* default unit */
	 char           *Str[GE_MAX_UNITS];
	 float          Con[GE_MAX_UNITS];
         long           XO;
         long           YO;
         long           XR;
         long           YR;
	 long           X;
         long           Y;
	 unsigned	Abs             : 1;
	 unsigned			: 0;
	}; 
/*
 * Fixed angle rotation parameters
 */
struct  GE_ROT
        {float          Alpha;
         float          Beta;
	 unsigned	Clockwise       : 1;
	 unsigned			: 0;
	}; 
/*
 * Alignment reference points
 */
struct  GE_ALN
	{struct GE_PAG	       *APagP;
	 struct	GE_CO		Co;
       	 struct	GE_PT		Pt;
	 long			AlnFrom;
	 long			AlnTo;
	}; 
/*
 * Undo (UnDelete, really) template
 *
 */
struct	GE_XDO
       {struct	GE_XDO		*Prev;          /* Previous mem in undo chain*/
	struct	GE_XDO		*Next;          /* Next mem     in undo chain*/
	struct  GE_SEG		*Seg;           /* First seg flg'd for delete*/
	struct  GE_SEG		*LSeg;          /* Last  seg-if set call Grf */
       };
/*
 * Line Pattern
 */
struct GE_PAT
       {int			LinP;
	char			DashList[4];
	int			DashLen;
	char			DashIndx;
       };

struct	GE_ANIM_FRAME
       {int			Repeat;
	int			DrawDelay;
	int			EraseDelay;
	int			Dx;
	int			Dy;
	float			MagFX;
	float			MagFY;
	float			Rot;
	unsigned		Erase		: 1;	/* Erase after draw  */
	unsigned		Redraw		: 1;	/* Final state of obj*/
	unsigned		tempbit3	: 1;
	unsigned		tempbit4	: 1;
	unsigned				: 0;
       };

struct	GE_ANIM_CTRL
       {int			NumPackets;		/* Num cmprsed frames*/
        int			FirstFrame;		/* First viewed frame*/
	int			NumFrames;		/* Num Frames viewed */
	int			LastFrame;		/* Last  viewed frame*/
	int			SumDx;			/* X motion from strt*/
	int			SumDy;			/* Y motion from strt*/
	float                   SumMagFX;		/* X mag from start  */
	float                   SumMagFY;		/* Y mag from start  */
	float                   SumRot;			/* Rotation from strt*/
	struct GE_ANIM_FRAME    *Frames;		/* Ptr to anim frames*/
	int			CurFrame;		/* Current frame num */
	int			InterruptResponse;	/* What do on intrupt*/
	int			NumFramesRemaining;	/* Frames still todo */
	int			itemp4;
	int			itemp5;
	unsigned		Animate		: 1;	/*-Can be animated   */
	unsigned		Animated	: 1;	/* IS animated       */
	unsigned		Interrupt	: 1;	/*-Can be interrupted*/
	unsigned		Interrupted	: 1;	/* HAS Ben interuptd?*/
	unsigned		Block		: 1;	/*-Hold DISP, maybe? */
	unsigned		Blocked		: 1;	/* IS Holding DISP   */
	unsigned		StartForward	: 1;	/*-Dir of fram travel*/
	unsigned		CurDirForward	: 1;	/* Cur dir fram trvl */
	unsigned		ReverseAtEnd	: 1;	/*-AutoReverse ON\OFF*/
	unsigned		CdispOverride   : 1;	/*-Like Window Wiping*/
	unsigned		CZupdisp	: 1;    /*-Obj inFRONTof anim*/
	unsigned		FastPlay	: 1;    /*-FAST mode         */
	unsigned		Paused		: 1;    /*-Currently paused  */
	unsigned				: 0;    /* "-" = read-written*/

       };
/*
 * Interrupt responses
 */
#define GEANIMCTRL_INTERRUPT_IGNORE	0
#define GEANIMCTRL_INTERRUPT_FLPBLOCK	1
#define GEANIMCTRL_INTERRUPT_PAUSE	2
#define GEANIMCTRL_INTERRUPT_STOP	3
#define GEANIMCTRL_INTERRUPT_RESTART	4
#define GEANIMCTRL_INTERRUPT_REVERSE	5
#define GEANIMCTRL_INTERRUPT_GOTOEND	6
#define GEANIMCTRL_INTERRUPT_FAST	7
#define GEANIMCTRL_INTERRUPT_SPEAK	8
#define GEANIMCTRL_INTERRUPT_SYSCOM	9
/*
 * Animation control structure bit flags
 */
#define GEANIMCTRL_ANIMATE		1
#define GEANIMCTRL_ANIMATED		2
#define GEANIMCTRL_INTERRUPT		4
#define GEANIMCTRL_INTERRUPTED		8
#define GEANIMCTRL_BLOCK		16
#define GEANIMCTRL_BLOCKED		32
#define GEANIMCTRL_STARTFORWARD		64
#define GEANIMCTRL_CURDIRFORWARD	128
#define GEANIMCTRL_REVERSEATEND		256
#define GEANIMCTRL_CDISPOVERRIDE	512
#define GEANIMCTRL_CZUPDISP		1024
#define GEANIMCTRL_FASTPLAY		2048
#define GEANIMCTRL_PAUSED		4096

/*
 * Animation frame bit flags
 */
#define GEANIMCTRL_FRAME_ERASE		1
#define GEANIMCTRL_FRAME_REDRAW		2
/*
 * Segments Template
 *
 * Generic Object information
 * Note:  A word about Color.  All segments have
 *either or both a SKIN or INTERIOR color.  To avoid duplication in tracking
 *color in the segment and private data structures the following
 *convention is hereby adopted.  If a segment has just one color
 *associated with it - such as 
 *                    Lin Pag Pol Grp
 *this color will be tracked in GE_SEG.  If on the other hand it
 *has two colors associated with it - such as
 *                    Cir Arc Elp, etc (ie all other objects)
 *then its FILL color will be in GE_SEG and its skin color will be in PRIV.
 *For text, the foreground color is in private data and the background color
 *(used if FillStyle = solid) is contained in GE_SEG.
 *
 *The reason for this apparent inconsistency (for a LINE obj the LINE COLOR
 *is contained in SEG, and for a CIRCLE obj the LINE COLOR is in priv data
 *and the SEG contains its FILL color), is because SEGMENT attributes are
 *conglomerate wide.  For example, for a polygon, there is only ONE FILL COLOR,
 *however, potentially, each individual member of the polygon may have a
 *different skin color.  For this reason, for now, we have to live with the
 *clumsy mechanism of having the SEG color mean different things for
 *Line objects.  As a consequence of this, geSegCr will stuff the currently
 *active Fill color into the newly created segment -
 *so Lin and Pag will want to override this
 *with their own values.
 */
struct	GE_SEG
       {struct	GE_SEG		*Prev;
	struct	GE_SEG		*Next;
	struct	GE_SEG		*Xref;
	struct	GE_CO		Co;
	struct  GE_COL_OBJ      Col;
	char			Handle[4];
	int                     (*Handler)();
	int			Z;
	short			FillHT;
	int			FillStyle;
	int			FillWritingMode;
	struct GE_ANIM_CTRL  	AnimCtrl;
	unsigned char		*Descrip;
	char			*Private;
	unsigned		Live		: 1;
	unsigned		Visible		: 1;
	unsigned		InFrame	        : 1;
	unsigned		WhiteOut        : 1;
	unsigned		ConLine         : 1;
	unsigned                Talk            : 1;
	unsigned                HotSpot         : 1;
	unsigned				: 0;
       };
/*
 * The following are the OBJECT SEGMENT bit flags - the one used in IO
 */
#define GEBIT_SEG_VISIBLE		1	/* Bit  0 - Hidden or Visible*/
#define GEBIT_SEG_WHITEOUT		2	/* Bit  1 - Eraser like obj  */
#define GEBIT_SEG_CONLINE		4	/* Bit  2 - Construction line*/
#define GEBIT_SEG_TALK			8	/* Bit  3 - Use DECtalk?     */
#define GEBIT_SEG_HOTSPOT		16	/* Bit  4 - Is SEG HOT SPOT? */

struct	GE_LIST
       {struct	GE_LIST		*Next;
	unsigned long           *Ptr;
       };
/*
 * Group stuff
 */
struct  GE_GRP
	{struct GE_SEG	*GSegP;
	 struct GE_SEG	*PGrpP;
	 struct GE_SEG	*FSegP;
	}; 
struct	GE_PAG
	{struct	GE_PAG		*Prev;
	 struct	GE_PAG		*Next;
	 char			*Name;
	 unsigned char		*Descrip;
	 struct GE_CO		Crop;
	 int			PixWidth;
	 int			MMWidth;
	 struct	GE_WIN		Surf;
	 struct	GE_WIN		ScrH;
	 struct	GE_WIN		ScrV;
	 struct	GE_SEG		*Seg0;
	 struct GE_GRID		Grid;
	 struct GE_ZOOM		Zoom;
	 struct GE_XDO		*XDoP;
         struct GE_VP_MARKS     VpMarks;
	 Widget			WidgetId;
	 Widget			WidgetArray[GEMAX_WIDGETS];
	 Pixmap			Pixmap;
	 int			ViewPortXoff;
	 int			ViewPortYoff;
	 int			ViewPortWidth;
	 int			ViewPortHeight;
	 unsigned		Modified        : 1;
	 unsigned		PixmapAnim      : 1;
	 unsigned				: 0;
	};
/*
 * Private (Object Specific) data - 			Arc
 */
struct	GE_ARC
       {struct GE_PT		Pta;
	struct GE_PT            Ptm;
	struct GE_PT            Ptb;
	struct GE_FPT		C;
	struct GE_COL_OBJ	Col;
	float			Rad;
	int                     Alpha;
	int                     Beta;
	long			LinW;
	struct GE_PAT		Pat;
	short			LineHT;
	int			LineStyle;
	int			WritingMode;
	int			CapStyle;
	int			JoinStyle;
	unsigned                EditPt1         : 1;
	unsigned                EditPt2         : 1;
	unsigned                EditPt3         : 1;
	unsigned			        : 0;
       };
/*
 * Private (Object Specific) data - 			Pie
 */
struct	GE_PIE
       {struct GE_PT		Pt0;
        struct GE_PT		Pta;
	struct GE_PT            Ptm;
	struct GE_PT            Ptb;
	struct GE_FPT		C;
	struct GE_COL_OBJ	Col;
	float			Rad;
	int                     Alpha;
	int                     Beta;
	long			LinW;
	struct GE_PAT		Pat;
	short			LineHT;
	int			LineStyle;
	int			WritingMode;
	int			CapStyle;
	int			JoinStyle;
	unsigned                EditPt1         : 1;
	unsigned                EditPt2         : 1;
	unsigned                EditPt3         : 1;
	unsigned                EditPt4         : 1;
	unsigned			        : 0;
       };
/*
 * Private (Object Specific) data - 			Box
 */
struct	GE_BOX
       {struct GE_BX		Bx;
	struct GE_COL_OBJ	Col;
	long			LinW;
	struct GE_PAT		Pat;
	short			LineHT;
	int			LineStyle;
	int			WritingMode;
	int			CapStyle;
	int			JoinStyle;
	unsigned                EditPt1         : 1;
	unsigned                EditPt2         : 1;
	unsigned                EditPt3         : 1;
	unsigned                EditPt4         : 1;
	unsigned			        : 0;
       };
/*
 * Private (Object Specific) data - 			Image
 */
struct	GE_IMG
       {struct GE_PT            Pt;
        struct GE_BX		Bx;
	struct GE_COL_OBJ	ColForeGround;
	struct GE_COL_OBJ	ColBackGround;
	int                     Type;
	int			WritingMode;
	int                     Mirrored;
	int                     RotAngle;
	long                    Flags;
        XImage                  *ImgPtr;
	unsigned char		*FileName;
	unsigned			        : 0;
       };


/*
 * Private (Object Specific) data - 			PostScript
 *
 * PostScript can have a colored and/or halftoned background (mini page color
 * and halftone) as well as a halftoned foreground.  It can't have a colored
 * foreground because the foreground colors are controlled by the PostScript
 * "program" itself.
 */
struct	GE_PS
       {unsigned char	*PsBuf;
        int		PsBufLen;
	unsigned char	*Filename;
	long		w_pix, h_pix;	   /* width and height in pixels */
	float		llx_pts, lly_pts,  /* 4 corners of PS image */
			lrx_pts, lry_pts,
			urx_pts, ury_pts,
			ulx_pts, uly_pts;
	float		rt_pts, lft_pts,	/* 4 edges of PS image: */
			top_pts, bot_pts;	/* right, left, top, bottom */
	struct GE_PT	Pt;	/* placement of PS in window */
        struct GE_BX	Bx;	/* crop box in rela to origin of PS */
	short		PenHT;	/* halftone ptn of the drawing "pen" */
	int		PenStyle;	/* ? */
	struct GE_TM	tmatrix;	/*  transformation matrix */
	long		Flags;
	unsigned	: 0;
       };

/*
 * Private (Object Specific) data - 			Circle
 */
struct	GE_CIR
       {struct GE_COL_OBJ			Col;
        long			Cx;
	long			Cy;
	long			Rad;
	long			LinW;
	struct GE_PAT		Pat;
	short			LineHT;
	int			LineStyle;
	int			WritingMode;
	unsigned                EditPt1         : 1;
	unsigned                EditPt2         : 1;
	unsigned                EditPt3         : 1;
	unsigned                EditPt4         : 1;
	unsigned			        : 0;
       };
/*
 * Private (Object Specific) data - 			Ellipse
 */
struct	GE_ELP
       {struct GE_COL_OBJ			Col;
        long			Cx;
	long			Cy;
	long			A;
	long			B;
	float			Gamma;
	long			LinW;
	struct GE_PAT		Pat;
        struct GE_BX		Bx;
	short			LineHT;
	int			LineStyle;
	int			WritingMode;
	unsigned                EditPt1         : 1;
	unsigned                EditPt2         : 1;
	unsigned                EditPt3         : 1;
	unsigned                EditPt4         : 1;
	unsigned			        : 0;
       };
/*
 * Private (Object Specific) data - 			Line
 */
struct	GE_LIN
       {struct GE_CO		Co;
	struct	GE_COL_OBJ	Col;
	long			LinW;
	struct GE_PAT		Pat;
	int			CapStyle;
	int			JoinStyle;
	short			LineHT;
	int			LineStyle;
	int			WritingMode;
	unsigned                EditPt1         : 1;
	unsigned                EditPt2         : 1;
	unsigned			        : 0;
       };
/*
 * Private (Object Specific) data - 			Txt
 */
struct	GE_TXT
       {struct GE_PT            Pt;
	struct GE_COL_OBJ       Col;
	short                   Just;
	int                     Kern;
        int                     Font;
	unsigned char		*Str;
	short			LineHT;
	int			LineStyle;
	int			WritingMode;
	unsigned                DispBg            : 1;
	unsigned			          : 0;
       };
/*
 * Group Template
 */
struct	GE_GROUP
       {struct	GE_GROUP	*Prev;
	struct	GE_GROUP	*Next;
       };		                          
/*
 * Area Template
 */
struct  GE_AREA
       {struct GE_PT            Pt;
	struct GE_SZ            Sz;
       };
struct 	GE_WS /* TRASH when dumping gks */
	{long	Id;
	 long	Dev;
	 long 	Type;
	 long	Class;
	 long	Units;
	 char   Open;
	 char   Active;
         float	MaxX;
         float	MaxY;
         long	MaxRasterX;
         long	MaxRasterY;
	 struct                
	       {long	MaxFonts;
		long	*FontList;
		long	*PrecList;
		long	NumHeights;
		float	Hmin;
		float	Hmax;
		long	CharExps;
		float	CharExpMin;
		float	CharExpMax;
		long	NumIndexes;
		long	FontListSz;
		long	FontPrecSz;
	       } Text;
        };

/*
 * Structure templates
 *
 * GE_UI                Widget related globals
 */
struct  GE_UI
	{MrmHierarchy    Id;                    /* MRM database hierarchy ID */
	 MrmType         Class;                 /* and class variable.       */
	 XtAppContext	 AppContext;		/* Application context 	     */
	 Widget          ToplevelWidget;        /* Root widget ID            */
	 Widget          MainWindowWidget; 	/* Main window ID 	     */
	 Widget          WidgetArray[GEMAX_WIDGETS]; /* Array of widget IDs  */
	 Window          HintsMsg;          	/* Hints Message ID          */
	 Opaque		 HelpContext;		/* Motif Help 		     */
	};                             

/*
 * Globals
 */
#ifndef GEGLOBALS
extern struct GE_UI	geUI;
extern struct GE_RUN    geRun;
extern struct GE_MEMIO  geMemIO;
extern struct GE_ALLOC_COL *geCol0, *geCol0User, *geColL;
extern struct GE_READ_COL_NAME *geReadColName0;
extern struct GE_CO	geClip,  geSel, geGenBx, geClipAnim;
extern struct GE_TM	geCTM;
extern struct GE_ATTR	geAttr,  geOldAttr, geSampleAttr;
extern struct GE_DISP_ATTR geDispAttr;
extern struct GE_MENUSTATE geMenuState;
extern struct GE_SETUP	geSetUp;
extern struct GE_STATE	geState, geOldState;
extern struct GE_GRP    geGrpStat;
extern struct GE_DISPCHAR	geDispChar;
extern struct GE_WS	geOldWs, geWs, geWi, gePr, geMi, geMo, geMn;
extern struct GE_SEG	*geSeg0, *geGrpSel, *geZRef, *geSegSel, *geSegCut;
extern struct GE_LIST	*geListSel;
extern struct GE_PAG	*gePag0;
extern struct GE_AREA   geEditBox;
extern struct GE_COL_LIST *geColList0;
extern struct GE_UNIT   geU;
extern struct GE_ROT    geRot;
extern struct GE_ALN    geAln;
extern struct GE_IMG_REF geImgRef;
extern struct GE_PS_REF	gePsRef;
extern struct GE_RESOURCE geRM;
extern struct GE_INITMANAGE geInitManage;
extern struct GE_WIN	geRgRoot;

extern char	    	geUtilBuf[GE_MAX_MSG_CHAR],
		        geErrBuf[GE_MAX_MSG_CHAR], /* For error strings      */
		        geTitleBuf[GE_MAX_MSG_CHAR], /* Application banner   */
                        *geRagLoc,              /* Path to location of support*/
                        *geUsrLoc,              /* Path to user's HOME dir   */
  			*geInFile,		/* Input file name	     */
		      	*geMsgPtr[GE_MAX_MSGS], /* Ptrs to messages          */
			*geFntPtr[GE_MAX_FNTS], /* User friendly font names  */
			*geFntAlloced[GE_MAX_FNTS], /* Ptrs to mallocd fonts */
			*geFntXName[GE_MAX_FNTS],/* Ptrs to fonts            */
			geDefRags[100],         /* Default Metafile Name     */
			geDefMeta[100],         /* Default Metafile Ext      */
  			geDefDDIF[100],         /* Default DDIF file Ext     */
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
			geDefDirInPs[100],	/* Default Directory - INPUT */
			geDefDirOutScrDmp[100], /* Default Directory - OUTPUT*/
			geDashList[4][4],	/* Line Pattern dashes	     */
			geParmC,		/* Byte all purpose var.     */
			geMainIconBits[],
			gePagesIconBits[],
                        geSmallIconBits[],
                        geDefColString[],
			geEditCmdString[100];   /* Holds current edit command*/

extern short            geGrey[16],             /* All purpose grey level    */
			geJustTxt,		/* Txt justify -1=L  0=C 1=R */
			geMagGrp,
			geBreakIn,              /* 1=Send cmd to grp members */
			geDebug,
			geReadWriteType,        /* Meta file type ASCIIorDDIF*/
			geObjSel, 		/* Object selection	     */	
			geImgType,		/* Image type       	     */
			geExportFlag;		/* When calling MOPS 	     */

extern int              (*gePHandler)(),        /* Poly sub-seg handler      */
			geIT,                   /* Very volatile             */
			geMultiShellCount,	/* # of shells 		     */
			geScreen, 
			geVersionIn, 
			geVersionOut, 
			geNumCols,
			geColStat,
			geStartIconified,
			geFontMenuWidth,
			geFontMenuHeight,
			geHasDPS,
			geDrawPS;

extern long             geGksLevel,             /* Gks Implementation Level  */
			geUtilX[GEMAX_PTS],
			geUtilY[GEMAX_PTS],
                        geVn,                   /* Index for geVert list     */
			geLinWL,		/* Last Line weight menu val */
    			geErr,			/* Internal Error Code       */
			geGetPutFlgs,           /* Attribute get-put flags   */
                        geAnimMin,              /* Inside indicator disp cntr*/
			geInqRes,		/* Inquire return value      */
			geMagCx,
			geMagCy,
			geMagMode,
			gePolXStart, gePolYStart,
			geLT1, geLT2;
extern unsigned long   	geXBPixel, geXWPixel;
extern unsigned long	geUL1, geUL2,
                        geCdaStat, geCdaTrk, geCdaLen,
		     	geCdaAggType,
		     	geCdaRtAgg, geCdaAgg,
			geCdaPrevAgg, geCdaSegAgg,
			geCdaNextAgg,
		     	geCdaStrm, geCdaFile;

extern float     	geSin[361],		/* Sin and cos in 1 degree   */
			geCos[361],		/*increments - saves time    */
		     	geUtilFX[GEMAX_PTS],    /* For Gks ONLY              */
			geUtilFY[GEMAX_PTS],
			geFT,			/* Highly volatile           */
			geMagFX, geMagFY;       /* Magnify x&y		     */

extern double		geTD;

extern XPoint           geVert[GEMAX_PTS];      /* Mostly for filled polys   */
extern XFontStruct      *geDefaultFont,
			*geFontMenu,
                        *geFontUser[GE_MAX_FNTS];
extern XColor           geBlack,                /* Solid BLACK               */
                        geWhite,                /* Pure  WHITE               */
                        geRed,
                        geGreen,
                        geBlue,
                       	geGenCol;

extern FILE             *geFdO,
                        *geFdI,
                        *geFdL;

extern Display		*geDispDev;
extern Colormap		geCmap;
extern Status		geStat;
extern GC		geGC0,			/* Default GC                */
			geGC1,			/* GC for menu items	     */
			geGC2,			/* Dynamic GC - obj outline  */
			geGC3,			/* Dynamic GC - obj fill     */
			geGC4,			/* Dynamic GC - text         */
			geGC5,			/* Static  GC - used for XOR */
			geGC6,			/* Static  GC - used for STAT*/
		       	geGCImg,		/* Used for images           */
			geGCHT,			/* GC - h-tone stipple pixmp */
			geGCPs;			/* GC dir drawing PostScript */

extern XGCValues	geGCV;			/* Struct for all GCs	     */

extern Cursor		geTxtOCursor,		/* Cursor used in txt ovrstrk*/
			geTxtICursor,		/* Cursor used in txt insert */
			geWaitCursor,		/* Cursor indicating pause   */
			geAnimCursor,		/* Sub for NO Animation cursr*/
			geHandCursor,		/* Used for panning	     */
			geMouseCursor,		/* Used for group/undelete   */
			geFingerCursor,		/* Awaiting keyboard input   */
			geCrossHairCursor;	/* Used to select regions    */

extern Pixmap		geHTStipple[GEMAXHT],	/* Stipples for Half-tones   */
			gePATStipple[GEMAXPAT],	/* Stipples for fills	     */
			geMainIcon,             /* Icon for Main Shell       */
			gePagesIcon,            /* Icon for Main Shell       */
              	       	geSmallIcon,            /* Icon for Small Shell(XUI) */
              	       	geLinPrim_icon,         /* Icons - polygon primitives*/
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

extern XComposeStatus	geComposeStatus;

extern Visual		*geVisual;

#endif GEGLOBALS
