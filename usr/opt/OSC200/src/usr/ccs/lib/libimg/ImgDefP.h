
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**  ImgDefP.h  (Private ISL definitions)
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This include file contains structure and constant definitions
**	for the C language that are for internal use by ISL modules only.
**	They are NOT for use by the outside world as are the definitions 
**	in IMG$DEF.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson
**
**  CREATION DATE:
**
**	18-SEP-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

#ifndef IMGDEFP_H

#define IMGDEFP_H

/*
** Old obsolete UDP_DESC structure still used by some routines
*/
struct UDP_DESC {
    unsigned short int UdpW_Length;    /* Bits Per Pixel                   */
    unsigned char UdpB_Dtype;
    unsigned char UdpB_Class ;
    char *UdpA_Base ;
    unsigned char UdpB_Scale;
    unsigned char UdpB_Digits;
    unsigned char UdpB_Aflags;
    unsigned char UdpB_Dimct;
    unsigned long int UdpL_Arsize;
    long int UdpL_V0;
    long int UdpL_S1;                  /* Pixel Stride                     */
    long int UdpL_S2;                  /* Scanline Stride                  */
    long int UdpL_L1;
    long int UdpL_U1;                  /* Pixels Per Scanline              */
    long int UdpL_L2;
    long int UdpL_U2;                  /* Scanline Count                   */
    long int UdpL_Pos ;                 /* inition POSition bit offset      */
    } ;

/*
** Structure to UDP characteristic constants
*/
struct UDP_INFO {
    short	LENGTH;
    char	ALIGNMENT;
    char	PADDING;
    };

/*
** UDP characteristic constants
*/

/*  #define NBITS	    7	-- defined by IPSDEF.H	*/
#define BYTE	    8
#define WORD	    16
#define TRIPLE	    24
#define LONGWORD    32

#define BIT_ALIGNED_BITS    (BIT_ALIGNED << 16) + NBITS
#define BIT_ALIGNED_BYTE    (BIT_ALIGNED << 16) + BYTE
#define BIT_ALIGNED_WORD    (BIT_ALIGNED << 16) + WORD
#define BIT_ALIGNED_TRIPLE  (BIT_ALIGNED << 16) + TRIPLE
#define BIT_ALIGNED_LONG    (BIT_ALIGNED << 16) + LONGWORD

#define BYTE_ALIGNED_BITS   (BYTE_ALIGNED << 16) + NBITS
#define BYTE_ALIGNED_BYTE   (BYTE_ALIGNED << 16) + BYTE
#define BYTE_ALIGNED_WORD   (BYTE_ALIGNED << 16) + WORD
#define BYTE_ALIGNED_TRIPLE (BYTE_ALIGNED << 16) + TRIPLE
#define BYTE_ALIGNED_LONG   (BYTE_ALIGNED << 16) + LONGWORD

#define WORD_ALIGNED_BITS   (WORD_ALIGNED << 16) + NBITS
#define WORD_ALIGNED_BYTE   (WORD_ALIGNED << 16) + BYTE
#define WORD_ALIGNED_WORD   (WORD_ALIGNED << 16) + WORD
#define WORD_ALIGNED_TRIPLE (WORD_ALIGNED << 16) + TRIPLE
#define WORD_ALIGNED_LONG   (WORD_ALIGNED << 16) + LONGWORD

#define LONG_ALIGNED_BITS   (LONG_ALIGNED << 16) + NBITS
#define LONG_ALIGNED_BYTE   (LONG_ALIGNED << 16) + BYTE
#define LONG_ALIGNED_WORD   (LONG_ALIGNED << 16) + WORD
#define LONG_ALIGNED_TRIPLE (LONG_ALIGNED << 16) + TRIPLE
#define LONG_ALIGNED_LONG   (LONG_ALIGNED << 16) + LONGWORD

/*
** Size Constants
*/
#define CHARSIZE    sizeof(char)
#define SHORTSIZE   sizeof(short)
#define LONGSIZE    sizeof(long)
#define QUADSIZE    8
 
/*
** Miscellaneous constants
*/
#define NOINDEX	    0


/*
** Master itemcode-value list for frames and frame definitions
*/
struct FAT {
    long	        FatL_Flags;
    struct ICI	       *FatR_Ici;
    long	        FatL_IceCnt;
    struct ICE	       *FatR_Ice;
    struct ASP {
	long		FatL_PixelPath;
	long		FatL_LinePath;
	}		FatR_PxlAspectRatio;
    long	        FatL_BrtPolarity;
    long	        FatL_CompSpaceOrg;
    long	        FatL_CompWavelengthCnt;
    long	        FatL_CompWavelengthC;
    long	    *((*FatL_CompWavelength)[]);  /* [comp_wavelen_cnt]   */
    long	        FatL_GridType;
    long	        FatL_FrmBoxLLX;
    long	        FatL_FrmBoxLLXC;
    long	        FatL_FrmBoxLLY;
    long	        FatL_FrmBoxLLYC;
    long	        FatL_FrmBoxURX;
    long	        FatL_FrmBoxURXC;
    long	        FatL_FrmBoxURY;
    long	        FatL_FrmBoxURYC;
    long	        FatL_FrmPositionC;
    long	        FatL_FrmfxdPositionX;
    long	        FatL_FrmfxdPositionXC;
    long	        FatL_FrmfxdPositionY;
    long	        FatL_FrmfxdPositionYC;
    long	        FatL_ImageDataClass;
    long	        FatL_LineProgression;
    long	        FatL_LookupTablesCnt;    /* same as LUT_CNT	    */
    char	    *((*FatR_LookupTables)[]);    /* [lookup_tables_cnt]  */
    long	        FatL_LpPixelDist;
    long	        FatL_NumberOfComp;
    long	        FatL_PixelGroupOrder;
    long	        FatL_PixelGroupSize;
    long	        FatL_PixelPath;
    long	        FatL_PlaneSignif;
    long	        FatL_PlanesPerPixel;
    long	        FatL_PpPixelDist;
    long	        FatR_PrivateData;
    long	        FatL_SpectralMapping;
    long		FatL_StandardFormat;
    long	        FatL_TotalBitsPerPixel;
    long		FatL_TotalQuantBitsPerPixel;
    long	        FatL_UserField;
    long	        FatL_UserLabelCnt;
    char	      **FatA_UserLabelStrptrs;
    };

#define FatK_Size  sizeof( struct FAT )


/*
** Image Component Information list (for info on
** each individual component)
*/
struct ICI {
/*    struct BHD	     IciR_Bhd;		    */
    long	     IciL_BitsPerComp;
    long	     IciL_QuantBitsPerComp;
    long	     IciL_QuantLevelsPerComp;
    };

#define IciK_Size  sizeof( struct ICI )

/*
** Image Content Element descriptor (points to IDU list)
*/
struct ICE {
/*    struct BHD	     IceR_Bhd;		    */
    long	     IceL_IduCnt;
    struct IDU	    *IceR_Idu;
    };

#define IceK_Size  sizeof( struct ICE )


/*
** Image Data Unit items
*/
struct IDU {
/*    struct BHD     IduR_Bhd;		*/
    long	     IduL_BitOrder;
    long	     IduL_ByteOrder;
    long	     IduL_ByteUnit;
    struct UDP	     IduR_Cdp;
    long	     IduL_CdpPrsnt;
    char	    *IduR_CompressionParams;
    long	     IduL_CompressionType;
    long	     IduL_DataOffset;
    long	     IduA_DataPlaneBase;
    long	     IduL_DataPlaneBitSize;
    long	     IduL_DataPlaneSize;
    long	     IduL_DataType;
    long	     IduL_Dtype;
    long	     IduL_NumberOfLines;
    long	     IduL_PixelAlignment;
    long	     IduL_PixelStride;
    long	     IduL_PixelsPerLine;
    long	     IduL_PlaneBitsPerPixel;
    long	    *IduR_PrivateCodingAttrs;
    long	     IduL_ScanlineAlignment;
    long	     IduL_ScanlineStride;
    struct UDP	     IduR_Udp;
    long	     IduL_UdpPrsnt;
    long	     IduL_VirtualArsize;
    };

#define IduK_Size  sizeof( struct IDU )


/*
** Structures used internally by convert functions
*/
struct CSA {
    long	*CsaL_BitsPerComp;
    long	 CsaL_CompSpaceOrg;
    long	 CsaL_ImageDataClass;
    long	 CsaL_NumberOfComp;
    long	*CsaL_QuantBitsPerComp;
    long	 CsaL_PlanesPerPixel;
    long	 CsaL_PlaneSignif;
    long	 CsaL_SpectralMapping;
    long	 CsaL_TotalBitsPerPixel;
    long	 CsaL_TotalQuantBitsPerPixel;
    struct UDP	*CsaR_PlaneUdpList;
    struct UDP	*CsaR_CompUdpList;
    };

#define CsaK_Size   sizeof( struct CSA )


/*
**	DDIF Context Block (DCB)
**
**	The address of this block is passed back to ISL applications
**	as the DDIF context identifier (ctx-id).  The ctx-id serves
**	a dual function:  it acts as a stream identifier if the application
**	is doing its own stream management, and acts as a file identifier
**	if the application is using the ISL/DDIF supplied file handling
**	functions.
*/

#ifndef _cdatyp_

#ifdef VMS
#include <cda$typ.h>
#else
#include <cdatyp.h>
#endif

#endif

struct	DCB {
    struct	BHD	  DcbR_Bhd;
    struct	DCB	 *DcbA_Self;                                    
    struct {
	unsigned	  DcbV_SaveRoot: 1;	/* save root aggr on close  */
    	unsigned	  DcbV_Dsc: 1;		/* Set after DDIF dsc 
						   processed		    */
    	unsigned	  DcbV_Hdr: 1;		/* Set after DDIF hdr
						   processed		    */
      	unsigned	  DcbV_DocContent: 1;	/* Set when DDIF content 
						   has been found	    */
    	unsigned	  DcbV_AccessMode: 2;	/* Access mode		    */
	unsigned	  DcbV_CtxMode: 2;	/* File or Stream ctx mode  */
    	unsigned	  DcbV_IntIoBuf: 1;	/* Prvtly alloc'd IObuf	    */
	unsigned	  DcbV_DefSize: 1;	/* Deflt iobuf size used    */
	unsigned	  DcbV_1stBufRead: 1;	/* Set after IMPORT buf 
						   read for the 1st time    */
    	unsigned	  DcbV_RetBytCnt: 1;	/* Return byte count?	    */
    	unsigned	  DcbV_NoSid: 1;	/* No stream id was passed
						   into xxPORT routine	    */
    	unsigned	  DcbV_DDIFId: 1;	/* EXPORT only: set after
						   initial DDIFDOCUMENT
						   tag has been put	    */
    	unsigned	  DcbV_DDIFEoc: 1;	/* Set when final DDIF-
						   stream eoc put	    */
	unsigned	  DcbV_Eos: 1;		/* IMPORT only: set when
						   end of input stream 	
						   found		    */
	unsigned	  DcbV_NoBufRealloc: 1;/* No IOBUF reallocation    */
	unsigned	  DcbV_SkipFrame: 1;	/* skip img frame on import */
	unsigned	  DcbV_SkipImgAggr: 1;/* skip the next DDIF_IMG   */
	unsigned	  DcbV_PageBreak: 1;	/* page break boolean	    */
	unsigned	  DcbV_SoftPageBreak: 1;
	unsigned	  DcbV_NoInheritance: 1;/* inhibit inheritance	    */
    	unsigned	 : 10;
    	}		  DcbL_Flags;		/* Flags		    */

    int			  DcbL_FileNameLen;
    char		 *DcbA_FileNameBuf;
    int			  DcbL_FileRetLen;
    CDAfilehandle	  DcbL_FileCtx;	/* DDIS context id	    */
    DDISstreamhandle	  DcbL_StrmCtx;
    CDArootagghandle	  DcbL_RootAggr;
    CDAagghandle	  DcbL_DscAggr;
    CDAagghandle	  DcbL_DhdAggr;
    CDAagghandle	  DcbL_SegAggr;
    long		(*DcbA_PrefixRtn)();
    long		  DcbL_PrefixRtnParam;
    long		(*DcbA_AggrRtn)();
    long		  DcbL_AggrRtnParam;
    struct	FCT	 *DcbL_Fid;		/* Current Frame id	*/

    int			  DcbL_IobLen;		/* IO block length	*/
    int			  DcbL_IobExtend;	/* IO buffer extend size*/
    char		 *DcbA_IobAdr;		/* IO block address	*/
    long		  DcbL_IoParm;		/* User IO param	*/
    long		(*DcbA_Action)();	/* User IO action rtn	*/
    int			  DcbL_ActionBytCnt;	/* action rtn thru-put	*/
    long		  DcbL_User;		/* Misc. user parameter	*/
    long		  DcbL_Ftype;		/* File type		*/
    struct	FCB	 *DcbA_Fcb;		/* file context block	*/
    }; /* end of struct DCB */

#define DcbK_Size		sizeof( struct DCB )
#define DcbK_DefIobExt		65024		/* default IOB extend length*/

#define DcbK_ModeRead		1		/* same as IMG$K_MODE_xx    */
#define DcbK_ModeWrite	2		/* codes		    */
#define DcbK_ModeImport	1		/* same as IMG$K_MODE_xx    */
#define DcbK_ModeExport	2		/* codes		    */

#define DcbK_File		1		/* using CDA file support   */
#define	DcbK_Stream		2		/* using appl stream support*/

       
/*
**  Revisable Default Strings for DDIF Descriptor and Header.
*/
#define IMG_KT_PRODID	"IMG_"
#define IMG_KT_PRODNAM	"DECimage Application Services V3.2"
#define IMG_KT_AUTHOR	"A. DeFault"
#define IMG_KT_TITLE	"DECimage File"
#define IMG_KT_VERSION	"3.2"

/*
** Block type definitions
*/
#define IMG_K_BLKTYP_DCB	81


struct A2D_DESC
        {
        unsigned short  A2D_W_LENGTH;
        unsigned char   A2D_B_DTYPE;
        unsigned char   A2D_B_CLASS;
        unsigned char      *A2D_A_POINTER;
        unsigned char       A2D_B_SCALE;
        unsigned char   A2D_B_DIGITS;
        struct {
                unsigned                 : 4;
                unsigned A2D_V_FL_REDIM  : 1;
                unsigned A2D_V_FL_COLUMN : 1;
                unsigned A2D_V_FL_COEFF  : 1;
                unsigned A2D_V_FL_BOUNDS : 1;
                   }        A2D_B_AFLAGS;
        unsigned char   A2D_B_DIMCT;    /* MUST BE 2 */
        unsigned long   A2D_L_ARSIZE;
        unsigned char      *A2D_A_A0;
        unsigned long       A2D_L_M1;
        unsigned long       A2D_L_M2;
        long        A2D_L_L1;
        long        A2D_L_U1;
        long        A2D_L_L2;
        long        A2D_L_U2;
        };


/*
**  I/O Status Block (IOSB) and File control block (FCB) structures 
**  used by the low-level file I/O mgt routines.
*/
struct IOSB
   {
   short        IosbW_Status;
   long         IosbL_ByteCnt;
   short        IosbW_Dummy;
   };

struct FCB {
    long	      FcbL_AccessType;
    long	      FcbL_CFilePtr;
    long	      FcbL_DDISParserCnt;
    long	      FcbL_EventFlag;
    long	      FcbL_FileChannel;
    char	     *FcbA_FilenameBuf;
    long	      FcbL_FilenameLen;
    long	      FcbL_FileType;
    struct {
	unsigned      FcbV_Asynchronous	    : 1;
	unsigned      FcbV_AsynchIONotDone  : 1;
	unsigned      FcbV_IoBuf2InUse	    : 1;
	unsigned			    : 29;
	}	      FcbL_Flags;
    long	    (*FcbA_IoAction)();
    long	      FcbL_IoActionParam;
    long	      FcbL_IoBufSize;
    char	     *FcbA_IoBuf1Adr;
    char	     *FcbA_IoBuf2Adr;
    char	     *FcbA_IoBufInUse;
    long	     *FcbA_IoBufRetLen;
    long	      FcbL_IoBytesTransfered;   /* used by import operations    */
    long	      FcbL_IoMode;
    long	      FcbL_IoStatus;
    long	      FcbL_IoStatus2;
    long	    (*FcbA_QioAst)();
    long	      FcbL_QioAstParam;
    struct IOSB	     *FcbA_QioIosb;
    char	     *FcbA_RfilenameBuf;
    long	      FcbL_RfilenameLen;
    long	     *FcbA_RfilenameRetlen;
    struct FAB	     *FcbR_RmsFab;
    struct NAM	     *FcbR_RmsNam;
    struct RAB	     *FcbR_RmsRab;
    struct XABALL    *FcbR_RmsXaball;
    struct XABITM    *FcbR_RmsXabitm;
    long	      FcbL_TotalByteCnt;
    long	      FcbL_VirtualBlkPos;
    };

/*
**  Private ISL flags
*/
#define ImgM_InhibitPrefixIO	(1 << 30)

/*
**  Rectangular ROI Info (RRI) that's used by
**  ImgSetRectRoi and ImgUnsetRectRoi.
*/
struct RRI {
    long    RriL_DataOffset;
    long    RriA_DataPlaneBase;
    long    RriL_DataPlaneSize;
    long    RriL_NumberOfLines;
    long    RriL_OverwrittenData;
    long    RriL_PixelsPerLine;
    };

#ifndef __DESCRIP_LOADED
#define __DESCRIP_LOADED	1
/*
 *	Atomic data types:
 */
#define DSC_K_DTYPE_Z	0		/* unspecified */
#define DSC_K_DTYPE_BU	2		/* byte (unsigned);  8-bit unsigned quantity */
#define DSC_K_DTYPE_WU	3		/* word (unsigned);  16-bit unsigned quantity */
#define DSC_K_DTYPE_LU	4		/* longword (unsigned);  32-bit unsigned quantity */
#define DSC_K_DTYPE_QU	5		/* quadword (unsigned);  64-bit unsigned quantity */
#define DSC_K_DTYPE_OU	25		/* octaword (unsigned);  128-bit unsigned quantity */
#define DSC_K_DTYPE_B	6		/* byte integer (signed);  8-bit signed 2's-complement integer */
#define DSC_K_DTYPE_W	7		/* word integer (signed);  16-bit signed 2's-complement integer */
#define DSC_K_DTYPE_L	8		/* longword integer (signed);  32-bit signed 2's-complement integer */
#define DSC_K_DTYPE_Q	9		/* quadword integer (signed);  64-bit signed 2's-complement integer */
#define DSC_K_DTYPE_O	26		/* octaword integer (signed);  128-bit signed 2's-complement integer */
#define DSC_K_DTYPE_F	10		/* F_floating;  32-bit single-precision floating point */
#define DSC_K_DTYPE_D	11		/* D_floating;  64-bit double-precision floating point */
#define DSC_K_DTYPE_G	27		/* G_floating;  64-bit double-precision floating point */
#define DSC_K_DTYPE_H	28		/* H_floating;  128-bit quadruple-precision floating point */
#define DSC_K_DTYPE_FC	12		/* F_floating complex */
#define DSC_K_DTYPE_DC	13		/* D_floating complex */
#define DSC_K_DTYPE_GC	29		/* G_floating complex */
#define DSC_K_DTYPE_HC	30		/* H_floating complex */
#define DSC_K_DTYPE_CIT	31		/* COBOL Intermediate Temporary */
/*
 *	String data types:
 */
#define DSC_K_DTYPE_T	14		/* character string;  a single 8-bit character or a sequence of characters */
#define DSC_K_DTYPE_VT	37		/* varying character string;  16-bit count, followed by a string */
#define DSC_K_DTYPE_NU	15		/* numeric string, unsigned */
#define DSC_K_DTYPE_NL	16		/* numeric string, left separate sign */
#define DSC_K_DTYPE_NLO	17		/* numeric string, left overpunched sign */
#define DSC_K_DTYPE_NR	18		/* numeric string, right separate sign */
#define DSC_K_DTYPE_NRO	19		/* numeric string, right overpunched sign */
#define DSC_K_DTYPE_NZ	20		/* numeric string, zoned sign */
#define DSC_K_DTYPE_P	21		/* packed decimal string */
#define DSC_K_DTYPE_V	1		/* aligned bit string */
#define DSC_K_DTYPE_VU	34		/* unaligned bit string */
/*
 *	Miscellaneous data types:
 */
#define DSC_K_DTYPE_ZI	22		/* sequence of instructions */
#define DSC_K_DTYPE_ZEM	23		/* procedure entry mask */
#define DSC_K_DTYPE_DSC	24		/* descriptor */
#define DSC_K_DTYPE_BPV	32		/* bound procedure value */
#define DSC_K_DTYPE_BLV	33		/* bound label value */
#define DSC_K_DTYPE_ADT	35		/* absolute date and time */
/*
 *	Reserved data type codes:
 *	codes 38-191 are reserved to DIGITAL;
 *	codes 160-191 are reserved to DIGITAL facilities for facility-specific purposes;
 *	codes 192-255 are reserved for DIGITAL's Computer Special Systems Group
 *	  and for customers for their own use.
 */


/*
 *	Codes for DSC_b_class:
 */
#define DSC_K_CLASS_S	1		/* fixed-length descriptor */
#define DSC_K_CLASS_D	2		/* dynamic string descriptor */
/*	DSC_K_CLASS_V			** variable buffer descriptor;  reserved for use by DIGITAL */
#define DSC_K_CLASS_A	4		/* array descriptor */
#define DSC_K_CLASS_P	5		/* procedure descriptor */
/*	DSC_K_CLASS_PI			** procedure incarnation descriptor;  obsolete */
/*	DSC_K_CLASS_J			** label descriptor;  reserved for use by the VMS Debugger */
/*	DSC_K_CLASS_JI			** label incarnation descriptor;  obsolete */
#define DSC_K_CLASS_SD	9		/* decimal string descriptor */
#define DSC_K_CLASS_NCA	10		/* noncontiguous array descriptor */
#define DSC_K_CLASS_VS	11		/* varying string descriptor */
#define DSC_K_CLASS_VSA	12		/* varying string array descriptor */
#define DSC_K_CLASS_UBS	13		/* unaligned bit string descriptor */
#define DSC_K_CLASS_UBA	14		/* unaligned bit array descriptor */
#define DSC_K_CLASS_SB	15		/* string with bounds descriptor */
#define DSC_K_CLASS_UBSB 16		/* unaligned bit string with bounds descriptor */
/*
 *	Reserved descriptor class codes:
 *	codes 15-191 are reserved to DIGITAL;
 *	codes 160-191 are reserved to DIGITAL facilities for facility-specific purposes;
 *	codes 192-255 are reserved for DIGITAL's Computer Special Systems Group
 *	  and for customers for their own use.
 */

#endif					/* __DESCRIP_LOADED */

/*
** For use with _ImgCreateRootAggregate()
*/
struct	ITEMLIST_ELEMENT {
    short   ITEM_LENGTH;
    short   ITEM_CODE;
    long    ITEM_ADDRESS;
    };

#endif
