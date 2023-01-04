/********************************************************************************************************************************/
/* Created 29-OCT-1993 16:55:38 by VAX SDL V3.2-12     Source: 29-OCT-1993 11:31:19 VAMASD$:[IMG.SRC]IMG$DEF.SDL;2 */
/********************************************************************************************************************************/
/**************************************************************************** */
/* IMG$DEF.SDL                                                              */
/*                                                                          */
/* FACILITY:                                                                */
/*                                                                          */
/*	Image Services Library (ISL)                                        */
/*                                                                          */
/* ABSTRACT:                                                                */
/*                                                                          */
/* 	This file contains the data structure definitions for in-memory     */
/* 	representation of DDIF/IIF image-frame attributes and data.         */
/*                                                                          */
/* AUTHORS:                                                                 */
/*                                                                          */
/*	Mark W. Sornson (and the gang)                                      */
/*                                                                          */
/* CREATION DATE:                                                           */
/*                                                                          */
/*	June 25, 1986                                                       */
/*                                                                          */
/**************************************************************************** */
 
/*** MODULE IMG$DEF ***/
#ifndef IMGDEF_H
#define IMGDEF_H
#ifndef	UDPDEF
#define UDPDEF
struct UDP {
    unsigned short int UdpW_PixelLength;
    unsigned char UdpB_DType;
    unsigned char UdpB_Class;
    unsigned char *UdpA_Base;
    unsigned long int UdpL_ArSize;
    unsigned long int UdpL_PxlStride;
    unsigned long int UdpL_ScnStride;
    long int UdpL_X1;
    long int UdpL_X2;
    long int UdpL_Y1;
    long int UdpL_Y2;
    unsigned long int UdpL_PxlPerScn;
    unsigned long int UdpL_ScnCnt;
    long int UdpL_Pos;
    unsigned long int UdpL_CompIdx;
    unsigned long int UdpL_Levels;
    } ;
typedef struct UDP UdpRec, *UdpPtr;
#define UdpK_DTypeUndefined 0
#define UdpK_DTypeMin 1
#define UdpK_DTypeBU 1
#define UdpK_DTypeWU 2
#define UdpK_DTypeLU 3
#define UdpK_DTypeF 4
#define UdpK_DTypeC 5
#define UdpK_DTypeVU 6
#define UdpK_DTypeV 7
#define UdpK_DTypeCL 8
#define UdpK_DTypeMax 8
#define UdpK_ClassUBA 1
#define UdpK_ClassUBS 2
#define UdpK_ClassA 3
#define UdpK_ClassCL 4
#endif	/* UDPDEF */
#define IpsK_Addition 1
#define IpsK_Multiplication 2
#define IpsK_Maximum 3
#define IpsK_Minimum 4
#define IpsK_SubtractionByConstant 5
#define IpsK_SubtractionFromConstant 6
#define IpsK_DivisionByConstant 7
#define IpsK_DivisionFromConstant 8
#define IpsK_Modulo 9
#define IpsK_SetToConstant 10
#define IpsK_SubtractSrc2FromSrc1 5
#define IpsK_SubtractSrc1FromSrc2 6
#define IpsK_DivideSrc1BySrc2 7
#define IpsK_DivideSrc2BySrc1 8
#define IpsK_Src1ModuloSrc2 9
#define IpsK_Wrap 0
#define IpsK_Clip 1
#define IpsK_Round 2
#define IpsM_RetainSrcDim 1
#define IpsM_ExcludeBorder 2
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL (void *)0
#endif
#define IpsK_EQ 0
#define IpsK_NE 1
#define IpsK_LT 2
#define IpsK_LE 3
#define IpsK_GT 4
#define IpsK_GE 5
#define IpsK_Sin 1
#define IpsK_Cos 2
#define IpsK_Tan 3
#define IpsK_ArcSin 4
#define IpsK_ArcCos 5
#define IpsK_ArcTan 6
#define IpsK_Sinh 7
#define IpsK_Cosh 8
#define IpsK_Tanh 9
#define IpsK_Exp 10
#define IpsK_NaturalLog 11
#define IpsK_Log2 12
#define IpsK_Log10 13
#define IpsK_AbsVal 14
#define IpsK_Square 15
#define IpsK_Sqrt 16
#define IpsK_Cube 17
#define IpsK_CubeRoot 18
#define IpsK_Negative 19
#define IpsK_Reciprocal 20
#define IpsK_SUCCESS_CLIP 1
#define IpsK_SUCCESS_NOCLIP 2
#define IpsK_SUCCESS_WRAP 3
#define IpsK_SUCCESS_SET 1
#define IpsK_SUCCESS_CLEAR 2
#define IpsK_SUCCESS_SOME 3
#define IpsK_LogicalMin 1
#define IpsK_Src1AndSrc2 1
#define IpsK_Src1AndNotSrc2 2
#define IpsK_NotSrc1AndSrc2 3
#define IpsK_Src1XorSrc2 4
#define IpsK_Src1OrSrc2 5
#define IpsK_NotSrc1AndNotSrc2 6
#define IpsK_NotSrc1XorSrc2 7
#define IpsK_Src1OrNotSrc2 8
#define IpsK_NotSrc1OrSrc2 9
#define IpsK_NotSrc1OrNotSrc2 10
#define IpsK_NotSrc1 11
#define IpsK_SetSrc1 12
#define IpsK_SetToSrc2 13
#define IpsK_SetToNotSrc2 14
#define IpsK_LogicalMax 14
#define IpsK_DitherClustered 1
#define IpsK_DitherBluenoise 2
#define IpsK_DitherDispersed 3
#define IpsK_DitherErrorDiffused 4
#define IpsM_DitherToBitonal 1
#define IpsM_SaveVertical 1
#define IpsM_SaveHorizontal 2
#define IpsM_ReversePreference 4
#define IpsM_DisablePreference 8
#define IpsM_NearestNeighbor 16
#define IpsM_ReverseEdgeFill 1
#define IpsK_Gaussian 1
#define IpsK_Flat 2
#define IpsK_UserDensity 3
#define IpsK_Hyperbolic 4
#define NBITS 7
#define BIT_ALIGNED 7
#define BYTE_ALIGNED 8
#define WORD_ALIGNED 16
#define LONG_ALIGNED 32
#define BYTE_SIZE 8
#define WORD_SIZE 16
#define LONG_SIZE 32
#define BYTE_SHIFT 3
#define WORD_SHIFT 4
#define LONG_SHIFT 5
#define LINE_PACKED_PIXEL_PACKED 1
#define LINE_PACKED_PIXEL_PADDED 2
#define LINE_PADDED_PIXEL_PACKED 4
#define LINE_PADDED_PIXEL_PADDED 8
#define IpsK_DTypeUndefined 0
#define IpsK_DTypeMin 1
#define IpsK_DTypeVU 6
#define IpsK_DTypeV 7
#define IpsK_DTypeBU 1
#define IpsK_DTypeWU 2
#define IpsK_DTypeLU 3
#define IpsK_DTypeF 4
#define IpsK_DTypeC 5
#define IpsK_DTypeMax 8
#define IpsK_DTypeBitstream 10
#define IpsK_DTypeIntBit 11
#define IpsK_DTypeBit 11
#define IpsK_DTypeIntBits 12
#define IpsK_DTypeBits 12
#define IpsK_DTypeIntBitsU 13
#define IpsK_DTypeBitsU 13
#define IpsK_DTypeIntByte 14
#define IpsK_DTypeIntB 14
#define IpsK_DTypeB 14
#define IpsK_DTypeIntByteU 15
#define IpsK_DTypeIntBU 15
#define IpsK_DTypeIntWord 16
#define IpsK_DTypeIntW 16
#define IpsK_DTypeW 16
#define IpsK_DTypeIntWordU 17
#define IpsK_DTypeIntWU 17
#define IpsK_DTypeIntLongword 18
#define IpsK_DTypeIntL 18
#define IpsK_DTypeL 18
#define IpsK_DTypeIntLongwordU 19
#define IpsK_DTypeIntLU 19
#define IpsK_DTypeFloat 20
#define IpsK_DTypeFloatD 21
#define IpsK_DTypeFD 21
#define IpsK_DTypeComplex 22
#define IpsK_DTypeComplexD 23
#define IpsK_DTypeCD 23
#define IpsK_ComboMax 16
#define IpsK_ComboMin 1
#define IpsK_InPlaceAllowed 1
#define MAX_NUMBER_OF_COMPONENTS 3
#define IpsM_SrcBitsPerPix 1
#define IpsM_ReverseUdpList 2
#define IpsM_EncodeScanlineAlign 1
#define IpsM_UpdateQuantLevels 1
#define IpsK_ByPixelValue 0
#define IpsK_ByFreq 1
#define IpsM_ScaleIfNecessary 1
#define IpsM_ClipToFit 2
#define IpsM_ScaleToFit 4
#define IpsM_FFTCenterDC 1
#define IpsK_Ideal_LP 0
#define IpsK_Ideal_HP 1
#define IpsK_Trapezoidal_LP 2
#define IpsK_Trapezoidal_HP 3
#define IpsK_Butterworth_LP 4
#define IpsK_Butterworth_HP 5
#define IpsK_Real 0
#define IpsK_Imag 1
/* special processing code                                                  */
/* file I/O attribute                                                       */
/* lookup table def. attribute                                              */
#define ImgK_FctItem 1
#define ImgK_DscAggrItem 2
#define ImgK_DhdAggrItem 3
#define ImgK_SegAggrItem 4
#define ImgK_SgaAggrItem 5
#define ImgK_IduAggrItem 6
#define ImgK_SplProcItem 7
#define ImgK_FioAttrItem 8
#define ImgK_RoiAttrItem 9
#define ImgK_LutAttrItem 10
#define ImgK_PrvtType 1
#define ImgK_IntegerType 2
#define ImgK_StringType 3
#define ImgK_ArrayTxtUnitType 4
#define ImgK_VariableType 5
#define ImgK_SeqOfDDIFIduType 6
#define ImgK_SeqOfDDIFPvtType 7
#define ImgK_EnumType 8
#define ImgK_ArrayOfStringType 9
#define ImgK_ArrayOfIntegerType 10
#define ImgK_ArrayOfFloatType 11
/*                                                                          */
/* Block typecode definitions                                               */
/*                                                                          */
#define ImgK_BlktypLhd 1              /* list head                        */
#define ImgK_BlktypFct 2              /* frame context block              */
#define ImgK_BlktypRoi 3              /* region of interest block         */
#define ImgK_BlktypHcb 4              /* histogram context block          */
#define ImgK_BlktypFdf 5              /* frame definition block           */
#define ImgK_BlktypLut 6              /* lookup table block               */
/*                                                                          */
/* Generic quadword buffer descriptor                                       */
/* (same as STARLET DSC$t_xxx fields)                                       */
/*                                                                          */
struct BUFDSC {
    unsigned short int DescW_Length;
    unsigned char DescB_Dtype;
    unsigned char DescB_Class;
    char *DescA_Pointer;
    } ;
/*                                                                          */
/* Generic Block Head structure (BHD)                                       */
/*                                                                          */
#define BhdM_InternalAlloc 1
struct BHD {
    struct BHD *BhdA_Flink;
    struct BHD *BhdA_Blink;
    unsigned short int BhdW_Length;
    unsigned char BhdB_Type;
    unsigned char BhdB_Refcnt;
    struct  {
/* Internal Dynamic Allocation                                              */
/* value: Static(0), Dynamic(1)                                             */
        unsigned BhdV_InternalAlloc : 1;
        unsigned BhdV_ReservedBits : 31;
        } BhdL_Flags;
    struct BUFDSC BhdR_IdstringDesc;
    } ;
/*                                                                          */
/* Generic List Head structure                                              */
/*                                                                          */
struct LHD {
    struct BHD *LhdA_Flink;
    struct BHD *LhdA_Blink;
    unsigned short int LhdW_ListCnt;
    unsigned char LhdB_Type;
    unsigned char LhdB_ReservedByte;
    } ;
/*                                                                          */
/* constants to define CONSTANT (integer) and VARIABLE (string) data for    */
/* DDIF items having "CHOICE" item codes .....                              */
/*                                                                          */
#define ImgK_ValueConstant 0
#define ImgK_ValueVariable 1
/*                                                                          */
/* Compressed Data Plane (CDP) block descriptions                           */
/* which corresponds to a VAX UBS descriptor with                           */
/* some extensions (to handle longer bit strings)                           */
/*                                                                          */
struct CDP_DESC {
    unsigned short int CdpW_Length;
    unsigned char CdpB_Dtype;
    unsigned char CdpB_Class;
    char *CdpA_Base;
    long int CdpL_Pos;
    unsigned long int CdpL_Bufsiz;
    unsigned long int CdpL_Strlen;
    } ;
#ifndef _cdatyp_
#if defined (VMS) && !defined(NEW_CDA_SYMBOLS)
#include <cda$typ.h>
#else
#if defined (VMS) && defined(NEW_CDA_SYMBOLS)
#include <cdatyp.h>
#else
#include <cdatyp.h>
#endif
#endif
#endif
#define FctM_ApplContent 1
#define FctM_NativeFormat 2
#define FctM_PvtQLevels 4
#define FctK_Size 84
struct FCT {
    struct BHD		FctR_Blkhd;
    struct  {
/* This flag will mar the frame content                                     */
/* as being application private, and not                                    */
/* to be deleted when the frame is                                          */
/* deleted.                                                                 */
        unsigned FctV_ApplContent : 1;
        unsigned FctV_NativeFormat : 1;
        unsigned FctV_PvtQLevels : 1;
        unsigned FctV_ReservedBits : 29;
        }		FctL_Flags;
    CDArootagghandle	FctL_RootAggr;
    CDAagghandle	FctL_SegAggr;
    CDAagghandle	FctL_SgaAggr;
    CDAagghandle	FctL_ImgAggr;
    unsigned int	FctL_IceCnt;
    CDAagghandle	FctL_IduAggr;
    unsigned long int	FctL_UserField;
    unsigned int	FctL_SpectType;
    unsigned int	FctL_PageBreak;
    long int		*FctA_PvtQLevels;
    long int		*FctA_RectRoiInfo;
    struct LHD		FctR_Fctstk;
    } ;
/**                                                                         */
/*	Item code related structure definitions                             */
/**                                                                         */
/*	Layout of item code fields                                          */
/**                                                                         */
#define ItmM_StructType 251658240
#define ItmM_Nowrite 268435456
#define ItmM_SplPrc 536870912
#define ItmM_IslPrvt 1073741824
#define ItmM_UserNowrite -2147483648
struct ITEMCODE {
    short int ItmW_Offset;
    char ItmB_DataType;               /* INTEGER, ARRAY_OF_STRING...      */
    unsigned ItmV_StructType : 4;     /* FCT, DOC_DSC, etc....            */
    unsigned ItmV_Nowrite : 1;         /* item readonly                    */
    unsigned ItmV_SplPrc : 1;         /* special processing req.          */
    unsigned ItmV_IslPrvt : 1;        /* ISL internal use only flag       */
    unsigned ItmV_UserNowrite : 1;    /* item not user-writable           */
    } ;
/*                                                                          */
/* Codes for items contained in the Frame Context Block                     */
/*                                                                          */
#define Img_RootAggr 16908316
#define Img_SegAggr 16908320
#define Img_SgaAggr 16908324
#define Img_ImgAggr 16908328
#define Img_IceCnt 16908332
#define Img_Icecnt 16908332
#define Img_IduAggr 16908336
#define Img_UserField 16908340
#define Img_ImageDataClass -2130575304
#define Img_SpectType -2130575304
#define Img_PageBreak -2130575300
#define Img_PvtQLevels -2130247616
#define Img_RectRoiInfo -2130575292
/*                                                                          */
/* Assorted DDIF item codes, not directly image related                     */
/*                                                                          */
#define Img_MajorVersion 302120960
#define Img_MinorVersion 302120961
#define Img_ProductIdentifier 302186498
#define Img_ProductName 302252035
#define Img_Title 319029252
#define Img_Author 319029253
#define Img_Version 319029254
#define Img_Date 318963719
#define Img_UserLabel 67371016
/*                                                                          */
/* item codes for Segment Attributes aggregate                              */
/*                                                                          */
#define Img_FrmBoxLlXC 84410377
#define Img_FrmBoxLLXC 84410377
#define Img_FrmBoxLlX 84213770
#define Img_FrmBoxLLX 84213770
#define Img_FrmBoxLlYC 84410379
#define Img_FrmBoxLLYC 84410379
#define Img_FrmBoxLlY 84213772
#define Img_FrmBoxLLY 84213772
#define Img_FrmBoxUrXC 84410381
#define Img_FrmBoxURXC 84410381
#define Img_FrmBoxUrX 84213774
#define Img_FrmBoxURX 84213774
#define Img_FrmBoxUrYC 84410383
#define Img_FrmBoxURYC 84410383
#define Img_FrmBoxUrY 84213776
#define Img_FrmBoxURY 84213776
#define Img_FrmPositionC 84213777
#define Img_FrmfxdPositionXC 84410386
#define Img_FrmFxdPositionXC 84410386
#define Img_FrmfxdPositionX 84213779
#define Img_FrXpos 84213779
#define Img_FrmFxdPositionX 84213779
#define Img_FrmfxdPositionYC 84410388
#define Img_FrmFxdPositionYC 84410388
#define Img_FrmfxdPositionY 84213781
#define Img_FrmFxdPositionY 84213781
/*                                                                          */
/* item codes for Presentation Attributes aggregate                         */
/*                                                                          */
#define Img_PrivateData 84344854
#define Img_PixelPath 84017175
#define Img_LineProgression 84017176
#define Img_PpPixelDist 84017177
#define Img_PPPixelDist 84017177
#define Img_LpPixelDist 84017178
#define Img_LPPixelDist 84017178
#define Img_BrtPolarity 84410395
#define Img_GridType 84410396
#define Img_TimingDesc 84017181
#define Img_SpectralMapping 84410398
#define Img_LookupTables 84475935
#define Img_CompWavelengthC 84410400
#define Img_CompWavelength 84213793
#define Img_CompSpaceOrg 84410402
#define Img_PlanesPerPixel 84017187
#define Img_PlaneSignif 84410404
#define Img_NumberOfComp 84017189
#define Img_BitsPerComp 84541478
#define Img_ImgBitsPerComp 84541478
/*                                                                          */
/* item codes for Image Data Unit aggregate                                 */
/*                                                                          */
#define Img_PrivateCodingAttr 101122087
#define Img_PixelsPerLine 100794408
#define Img_NumberOfLines 100794409
#define Img_CompressionType 101187626
#define Img_CompressionParams 101122091
#define Img_DataOffset 100794412
#define Img_PixelStride 100794413
#define Img_ScanlineStride 100794414
#define Img_BitOrder 101187631
#define Img_PixelOrder 101187631
#define Img_PlaneBitsPerPixel 100794416
#define Img_BitsPerPixel 100794416
#define Img_PlaneData 1174601777
#define Img_ByteUnit 101187634
#define Img_ByteOrder 100794419
#define Img_DataType 101187636
/*                                                                          */
/* more componant space attributes (new ones)                               */
/*                                                                          */
#define Img_PixelGroupSize 84410421
#define Img_PixelGroupOrder 84017206
#define Img_QuantLevelsPerComp 621412407
/*                                                                          */
/* Item codes for Special Processing items                                  */
/*                                                                          */
#define Img_DataPlaneBase 922877952
#define Img_PlaneDataBase 922877952
#define Img_TotalBitsPerPixel 922877953
/*                                                                          */
/* Sequence spacer, to preserve itemcode values from shifting up after      */
/* having moved an itemcode def that was here somewhere else.               */
/*                                                                          */
#define Img_PxlAspectRatio 654639107
/*                                                                          */
/* NOTE:    This attribute number used to be assigned to Img_DataPlaneSize, */
/*	    but a V2 bug returned size in bits.  It was renamed to          */
/*	    provide support for this behavior (and to not break V2 applications) */
/*                                                                          */
/*	    Img_DataPlaneSize is supported below with a new attribute    */
/*	    number.                                                         */
/*                                                                          */
#define Img_DataPlaneBitSize 922877956
#define Img_LutCnt 922877957
#define Img_LookupTablesCnt 922877957
#define Img_IduCnt 922877958
#define Img_BpcListCnt 922877959
#define Img_BPCListCnt 922877959
#define Img_UdpPrsnt 922877960
#define Img_Udp 1728184329
#define Img_CdpPrsnt 922877962
#define Img_Cdp 1728184331
#define Img_UserLabelCnt 922877964
#define Img_UserLabelLen 922877965
#define Img_QuantBitsPerComp 922877966
#define Img_TotalQuantBitsPerPixel 922877967
#define Img_PixelAlignment 922877968
#define Img_ScanlineAlignment 922877969
#define Img_VirtualArsize 922877970
#define Img_Dtype 922877971
#define Img_DType 922877971
/*                                                                          */
/* NOTE:    This is attribute has a new number.  The old number has been    */
/*	    renamed to be Img_DataPlaneBitSize.                         */
/*                                                                          */
#define Img_DataPlaneSize 922877972
#define Img_CdpArsize 922877972
#define Img_UdpArsize 922877972
#define Img_StandardFormat 922877973
/*                                                                          */
/* File I/O itemcodes                                                       */
/*                                                                          */
#define Img_DocRootAggr 134348800
#define Img_DscAggr 134348801
#define Img_DhdAggr 134348802
#define Img_FullFileSpec 134348803
#define Img_ResultantFileSpec 134348803
#define Img_AccessType 134348804
#define Img_IoBufSize 134348805
/*                                                                          */
/* Bounding Rectangle Block (BRECT)                                         */
/*                                                                          */
#define BrectK_Size 16
struct BRECT {
    unsigned long int BrectL_Ulx;
    unsigned long int BrectL_Uly;
    unsigned long int BrectL_XPixels;
    unsigned long int BrectL_YPixels;
    } ;
/*                                                                          */
/* Region of Interest Block (ROI)                                           */
/*                                                                          */
#define RoiM_ShapeBufIntAlloc 1
#define RoiK_Size 60
struct ROI {
    struct BHD RoiR_Blkhd;
    struct BRECT RoiR_BoundingRect;
    unsigned long int RoiL_StartX;
    unsigned long int RoiL_StartY;
    struct  {
        unsigned RoiV_ShapeBufIntAlloc : 1;
        unsigned RoiV_ReservedBits : 31;
        } RoiL_Flags;
    unsigned short int RoiW_Length;
    unsigned char RoiB_Type;
    unsigned char RoiB_Class;
    char *RoiA_Shape;
    } ;
/*                                                                          */
/* Itemcodes for ROI definition item list(s)                                */
/*                                                                          */
#define Img_RoiRectangle 151126016
/*                                                                          */
/* Component Space Organization Typecode definitions                        */
/*                                                                          */
#define ImgK_BandIntrlvdByPixel 1
#define ImgK_BandIntrlvdByPlane 2
#define ImgK_BitIntrlvdByPlane 3
#define ImgK_BandIntrlvdByLine 4
#define ImgK_OrgFulpxlcmp 1
#define ImgK_OrgFulPxlCmp 1
#define ImgK_OrgParpxlexp 2
#define ImgK_OrgParPxlExp 2
#define ImgK_OrgFulpxlexp 3
#define ImgK_OrgFulPxlExp 3
#define ImgK_PixelSequential 1
#define ImgK_CompSequential 2
#define ImgK_ExpCompSequential 3
/*                                                                          */
/* Dither type selection codes                                              */
/*                                                                          */
#define ImgK_DitherClustered 1        /* classic clustered dot dither     */
#define ImgK_DitherBluenoise 2        /* blue noise dither                */
#define ImgK_DitherDispersed 3        /* ordered dither                   */
#define ImgK_DitherErrorDiffused 4   /* Floyd Steinberg error diffusion  */
/*                                                                          */
/* Grid Typecode definitions                                                */
/*                                                                          */
#define ImgK_RectangularGrid 1
#define ImgK_HexEvenIndent 2
#define ImgK_HexOddIndent 3
/*                                                                          */
/* Component Mapping Codes                                                  */
/*                                                                          */
#define ImgK_PrivateMap 1
#define ImgK_MonochromeMap 2
#define ImgK_GeneralMap 3
#define ImgK_ColorMap 4
#define ImgK_RgbMap 5
#define ImgK_RGBMap 5
#define ImgK_CmyMap 6
#define ImgK_CMYMap 6
#define ImgK_YuvMap 7
#define ImgK_YUVMap 7
#define ImgK_HsvMap 8
#define ImgK_HSVMap 8
#define ImgK_HlsMap 9
#define ImgK_HLSMap 9
#define ImgK_YiqMap 10
#define ImgK_YIQMap 10
/*                                                                          */
/* Component wavelength information type codes                              */
/*                                                                          */
#define ImgK_ApplicationWavelength 0
#define ImgK_WavelengthMeasure 1
#define ImgK_WavelengthBandId 2
/*                                                                          */
/* standard pixel path and line progression angles (in degrees of arc)      */
/*                                                                          */
#define ImgK_BottomToTop 90
#define ImgK_TopToBottom 270
#define ImgK_LeftToRight 0
#define ImgK_RightToLeft 180
/*                                                                          */
/* Pixel Order Values (for item code Img_PxlOrder)                        */
/*                                                                          */
#define ImgK_StandardPixelOrder 1
#define ImgK_ReversePixelOrder 2
/*                                                                          */
/* Data Plane Significance Values (for item code Img_DpSignif)            */
/* [This attribute only has meaning when the component space                */
/*  organization is BIT-INTRVLD-BY-PLANE.]                                  */
/*                                                                          */
#define ImgK_LsbitFirst 1
#define ImgK_MsbitFirst 2
#define ImgK_LsbMsb 1
#define ImgK_MsbLsb 2
#define ImgK_LsbyteFirst 1
#define ImgK_MsbyteFirst 2
/*                                                                          */
/* Brightness Polarity Values (for item code Img_BrtPolarity)             */
/*                                                                          */
#define ImgK_ZeroMaxIntensity 1
#define ImgK_ZeroMinIntensity 2
/*                                                                          */
/* Compression Typecode definitions                                         */
/*                                                                          */
#define ImgK_CtypeMin 0
#define ImgK_PrivateCompression 1
#define ImgK_PcmCompression 2
#define ImgK_G31dCompression 3
#define ImgK_G32dCompression 4
#define ImgK_G42dCompression 5
#define ImgK_MonoCompression 6
#define ImgK_DctCompression 7
#define ImgK_CtypeMax 8
/*                                                                          */
/* Compression flags                                                        */
/*                                                                          */
#define ImgM_AlignGroup3 1
/*                                                                          */
/* Frame Block Spectral Typecode definitions                                */
/*                                                                          */
#define ImgK_ClassPrivate 1
#define ImgK_ClassBitonal 2
#define ImgK_ClassGreyscale 3
#define ImgK_ClassMultispect 4
#define ImgK_ClassGrayscale 3
/* equivalence names for backward compatibility                             */
#define ImgK_StypePrivate 1
#define ImgK_StypeBitonal 2
#define ImgK_StypeGreyscale 3
#define ImgK_StypeGrayscale 3
#define ImgK_StypeMultispect 4
/*                                                                          */
/* Region of Interest typecode definitions                                  */
/*                                                                          */
#define ImgK_RoitypeRect 1
#define ImgK_RoitypeCcode 2
#define ImgK_RoitypeCCode 2
/*                                                                          */
/*	Constants for combination rules                                     */
/*                                                                          */
#define ImgK_ComboRule01 1
#define ImgK_ComboRule02 2
#define ImgK_ComboRule03 3
#define ImgK_ComboRule04 4
#define ImgK_ComboRule05 5
#define ImgK_ComboRule06 6
#define ImgK_ComboRule07 7
#define ImgK_ComboRule08 8
#define ImgK_ComboRule09 9
#define ImgK_ComboRule10 10
#define ImgK_ComboRule11 11
#define ImgK_ComboRule12 12
#define ImgK_ComboRule13 13
#define ImgK_ComboRule14 14
#define ImgK_ComboRule15 15
#define ImgK_ComboRule16 16
#define ImgK_ComboMin 1
#define ImgK_ComboMax 16
#define ImgK_ClrDst 1
#define ImgK_False 1
#define ImgK_SrcAndDst 2
#define ImgK_And 2
#define ImgK_SrcAndNotDst 3
#define ImgK_SrcImpliesDstNot 3
#define ImgK_Src 4
#define ImgK_NotSrcAndDst 5
#define ImgK_DstImpliesSrcNot 5
#define ImgK_Dst 6
#define ImgK_SrcXorDst 7
#define ImgK_Xor 7
#define ImgK_SrcOrDst 8
#define ImgK_Or 8
#define ImgK_NotSrcAndNotDst 9
#define ImgK_PeirceArrow 9
#define ImgK_NotSrcXorDst 10
#define ImgK_Equivalence 10
#define ImgK_NotDst 11
#define ImgK_SrcOrNotDst 12
#define ImgK_DstImpliesSrc 12
#define ImgK_NotSrc 13
#define ImgK_NotSrcOrDst 14
#define ImgK_SrcImpliesDst 14
#define ImgK_NotSrcOrNotDst 15
#define ImgK_ShefferStroke 15
#define ImgK_SetDst 16
#define ImgK_True 16
/*                                                                          */
/*  Logical operators used with IMG$COMBINE_FRAME                           */
/*                                                                          */
#define ImgK_Src1AndSrc2 1
#define ImgK_Src1AndNotSrc2 2
#define ImgK_NotSrc1AndSrc2 3
#define ImgK_Src1XorSrc2 4
#define ImgK_Src1OrSrc2 5
#define ImgK_NotSrc1AndNotSrc2 6
#define ImgK_NotSrc1XorSrc2 7
#define ImgK_Src1OrNotSrc2 8
#define ImgK_NotSrc1OrSrc2 9
#define ImgK_NotSrc1OrNotSrc2 10
#define ImgK_NotSrc1 11
#define ImgK_SetSrc1 12
#define ImgK_SetToSrc2 13
#define ImgK_SetToNotSrc2 14
/*                                                                          */
/*	Flag masks for use with IMG$SCALE                                   */
/*                                                                          */
#define ImgM_SaveVertical 1
#define ImgM_SaveHorizontal 2
#define ImgM_ReversePreference 4
#define ImgM_DisablePreference 8
#define ImgM_NearestNeighbor 16
/*                                                                          */
/*	User level ROI definition structures                                */
/*                                                                          */
#define RoiK_RectLength 16
struct ROI_RECT {
    long int RoiL_RectUlx;            /* Upper left X coordinate          */
    long int RoiL_RectUly;            /* Upper left Y coordinate          */
    unsigned long int RoiL_RectPxls;  /* Pixels per scanline              */
    unsigned long int RoiL_RectScnlns; /* Scanline count                  */
    } ;
#define RoiK_CcodeLength 12
struct ROI_CCODE {
    long int RoiL_CcodeX1;            /* Upper left X coordinate          */
    long int RoiL_CcodeY1;            /* Upper left Y coordinate          */
    unsigned long int RoiL_CcodeLength; /* Number of chain codes          */
    unsigned char *RoiAB_CcodeData;
    } ;
/* Chain code data                                                          */
/**                                                                         */
/*	Item list structure for GET operations                              */
/**                                                                         */
struct GET_ITMLST {
    unsigned long int GetL_Code;
    unsigned long int GetL_Length;
    char *GetA_Buffer;
    unsigned long int *GetA_Retlen;
    unsigned long int GetL_Index;
    } ;
/**                                                                         */
/*	Item list structure for PUT operations                              */
/**                                                                         */
struct PUT_ITMLST {
    unsigned long int PutL_Code;
    unsigned long int PutL_Length;
    char *PutA_Buffer;
    unsigned long int PutL_Index;
    } ;
/**                                                                         */
/*	Generic item list structure for new 3.0 functions                   */
/**                                                                         */
struct ITMLST {
    unsigned long int ItmL_Code;
    unsigned long int ItmL_Length;
    char *ItmA_Buffer;
    unsigned long int *ItmA_Retlen;
    unsigned long int ItmL_Index;
    } ;
/*                                                                          */
/* Literals used for stream and file operations.                            */
/*                                                                          */
#define ImgK_ModeImport 1
#define ImgK_ModeExport 2
#define ImgK_ModeTransfer 3
/*                                                                          */
/* Flag masks used with IMG$CREATE_DDIF_STREAM and IMG$OPEN_DDIF_FILE       */
/*                                                                          */
/* ImgM_SaveRoot: for IMPORT operations only.  Will save the root aggr    */
/* used in parallel with the input stream.  On import, the document desc    */
/* (DDif_Dsc) aggr and document header (DDif_Dhd) aggr are attached to    */
/* this root.  Normally, the root aggr is deleted once the stream/file      */
/* is closed, which will delete these other two aggrs.                      */
#define ImgM_SaveRoot 1
/*                                                                          */
/* Flag masks for use with IMG$IMPORT_DDIF_FRAME.                           */
/*                                                                          */
#define ImgM_SkipFrame 1              /* skip next unread img frame       */
/*                                                                          */
/* Turn off inheritance on frame import.  Pass into IMG$OPEN_DDIF_FILE,     */
/* IMG$CREATE_DDIF_STREAM, or IMG$IMPORT_DDIF_FRAME when using implicit     */
/* stream support for the import of a single frame.                         */
/*                                                                          */
#define ImgM_Noinheritance 2           /* turn off attr inheritance        */
/*                                                                          */
/* Flag masks for use with IMG$EXPORT_DDIF_FRAME.                           */
/*                                                                          */
#define ImgM_PageBreak 1              /* puts a hard page break out       */
#define ImgM_Nobufrealloc 2            /* no auto-internal-buf-realloc     */
#define ImgM_NoBufRealloc 2
/*                                                                          */
/* Flag masks for use with IMG$EXPORT_DDIF_FRAME and                        */
/* IMG$EXPORT_DDIF_PAGE_BREAK.                                              */
/*                                                                          */
#define ImgM_SoftPageBreak 4         /* puts a soft page break out       */
#define ImgK_NoPageBreak 0
#define ImgK_HardPageBreak 1
#define ImgK_SoftPageBreak 2
/*                                                                          */
/* Masks used for general processing flags                                  */
/*                                                                          */
#define ImgM_Abort 4
/*                                                                          */
/* Flag masks used for use with IMG$$IMPORT_DDIF_FRAME                      */
/*                                                                          */
#define ImgM_IslPrvtSeg 1
/*                                                                          */
/* constants used for IMG$SET_FRAME_SERVICE                                 */
/*                                                                          */
#define ImgK_ResolutionI 1
#define ImgK_ResolutionF 2
#define ImgK_Inches 3
#define ImgK_Centimeters 4
#define ImgK_Bmus 5
/*                                                                          */
/* Scanline stride alignment constants.                                     */
/*                                                                          */
#define ImgK_AlignBit 1
#define ImgK_AlignByte 8
#define ImgK_AlignWord 16
#define ImgK_AlignLongword 32
/*                                                                          */
/* Flags masks for use with IMG$ROTATE                                      */
/*                                                                          */
/*	IMG$ROTATE also takes the ImgM_NearestNeighbor flag               */
/*                                                                          */
#define ImgM_ReverseEdgeFill 1
/*                                                                          */
/* Flags mask for use with IMG$FLIP                                         */
/*                                                                          */
#define ImgM_FlipHorizontal 1
#define ImgM_FlipVertical 2
/*                                                                          */
/* Flags mask for specifying 'serial line binary' PostScript (R) encoding   */
/* in IMG$EXPORT_PS                                                         */
/*                                                                          */
#define ImgM_SerialBinaryEncoding 1
/*                                                                          */
/* Histogram Context Block                                                  */
/*                                                                          */
#define HcbM_SortByFreq 1
struct HCB {
    struct BHD HcbR_Blkhd;
    struct  {
        unsigned HcbV_SortByFreq : 1;
        unsigned HcbV_ReservedBits : 31;
        } HcbL_Flags;
    unsigned long int HcbL_TableType;
    unsigned long int HcbL_Fid;
    unsigned long int HcbL_Roi;
    unsigned long int HcbL_ComponentIdx;
    unsigned long int HcbL_EntryCount;
    unsigned long int *HcbA_TablePointer;
    } ;
/*                                                                          */
/* Histogram type codes                                                     */
/*                                                                          */
#define ImgK_HtypeImplicitIndex 1    /* count only, default              */
#define ImgK_HtypeExplicitIndex 2    /* value, count ordered pair        */
/*                                                                          */
/* Flags to pass into IMG$CREATE_HISTOGRAM                                  */
/*                                                                          */
#define ImgM_SortByFreq 1
/*                                                                          */
/* Constants for FILTER                                                     */
/*                                                                          */
#define ImgK_FilterClear 1
#define ImgK_FilterEdge 2
#define ImgK_FilterMedian 3
#define ImgK_FilterPrewitt 4
#define ImgK_FilterSharpen 5
#define ImgK_FilterLightSharpen 5
#define ImgK_FilterHeavySharpen 6
#define ImgK_FilterLaplacian 7
#define ImgK_FilterSmooth 8
#define ImgK_FilterLightSmooth 8
#define ImgK_FilterHeavySmooth 9
#define ImgK_FilterSobel 10
/* Constants for conformance verification                                   */
/*                                                                          */
#define ImgK_CompOpEql 1
#define ImgK_CompOpNeq 2
#define ImgK_CompOpGtr 3
#define ImgK_CompOpLss 4
#define ImgK_CompOpGeq 5
#define ImgK_CompOpLeq 6
#define ImgK_CompOpBetween1 7
#define ImgK_CompOpBetween2 8
#define ImgK_CompOpBetween3 9
#define ImgK_CompOpBetween4 10
#define ImgK_CompOpOutside1 11
#define ImgK_CompOpOutside2 12
#define ImgK_CompOpOutside3 13
#define ImgK_CompOpOutside4 14
#define ImgK_CompTypeAllow 15
#define ImgK_CompTypeDisallow 16
#define ImgK_CompTypeNochange 17
#define ImgK_ConfDomContinuous 18
#define ImgK_ConfDomDiscrete 19
/*                                                                          */
/* Constants and flags for use with data services                           */
/*                                                                          */
#define ImgM_PassByReference 1       /* bit 1                            */
/*                                                                          */
/* Flag for use with memory management                                      */
/*                                                                          */
#define ImgM_InitMem 1                /* bit 1                            */
/*                                                                          */
/* Data Type constants                                                      */
/*                                                                          */
#define ImgK_DTypeVU 6
#define ImgK_DTypeV 7
#define ImgK_DTypeBitstream 10
#define ImgK_DTypeIntBit 11
#define ImgK_DTypeBit 11
#define ImgK_DTypeIntBits 12
#define ImgK_DTypeBits 12
#define ImgK_DTypeIntBitsU 13
#define ImgK_DTypeBitsU 13
#define ImgK_DTypeIntByte 14
#define ImgK_DTypeIntB 14
#define ImgK_DTypeB 14
#define ImgK_DTypeIntByteU 1
#define ImgK_DTypeIntBU 1
#define ImgK_DTypeBU 1
#define ImgK_DTypeIntWord 16
#define ImgK_DTypeIntW 16
#define ImgK_DTypeW 16
#define ImgK_DTypeIntWordU 2
#define ImgK_DTypeIntWU 2
#define ImgK_DTypeWU 2
#define ImgK_DTypeIntLongword 18
#define ImgK_DTypeIntL 18
#define ImgK_DTypeL 18
#define ImgK_DTypeIntLongwordU 3
#define ImgK_DTypeIntLU 3
#define ImgK_DTypeLU 3
#define ImgK_DTypeFloat 4
#define ImgK_DTypeF 4
#define ImgK_DTypeFloatD 21
#define ImgK_DTypeFD 21
#define ImgK_DTypeComplex 22
#define ImgK_DTypeC 22
#define ImgK_DTypeComplexD 23
#define ImgK_DTypeCD 23
/*                                                                          */
/* Obsolete Lut structure ....                                              */
/*                                                                          */
struct LUT {
    unsigned char LutB_Dtype;
    unsigned char LutB_Dimct;
    unsigned char LutB_Class;
    unsigned short int LutW_Length;
    unsigned long int LutL_Arsize;
    unsigned char *LutA_Pointer;
    } ;
#define LutK_DTypeVU 1              /* Unaligned Bitstream              */
#define LutK_DTypeV 2                /* Aligned Bitstream                */
#define LutK_DTypeBU 3              /* Byte Unsigned                    */
#define LutK_DTypeWU 4              /* Word Unsigned                    */
#define LutK_DTypeLU 5              /* Longword Unsigned                */
#define LutK_DTypeF 6                /* Single Precision Float           */
/*                                                                          */
/* Lookup Table Definition block (LTD)                                      */
/*                                                                          */
#define LutM_PredefinedLut 1
#define LutM_UserOwnedLut 2
#define LutK_Size 104
struct LTD {
    struct BHD LutR_Blkhd;
    struct  {
        unsigned LutV_PredefinedLut : 1;
        unsigned LutV_UserOwnedLut : 1;
        unsigned LutV_ReservedBits : 30;
        } LutL_Flags;
    unsigned long int *LutA_BitsPerComp;
    unsigned long int LutL_DfnType;
    unsigned long int LutL_EntryCount;
    unsigned long int LutL_EntryDataType;
    unsigned long int LutL_EntrySize;
    unsigned long int LutL_EntryTuppleSize;
    float *LutA_MaxFltValue;
    unsigned long int *LutA_MaxIntValue;
    unsigned long int LutL_NumberOfComp;
    unsigned long int *LutA_QuantLevels;
    unsigned long int LutL_SpectralMapping;
    unsigned char *LutA_Table;
    char *LutA_TableLabel;
    unsigned long int LutL_TableLabelSize;
    unsigned long int LutL_TableSize;
    unsigned long int LutL_TableType;
    unsigned long int LutL_UserField;
    char *LutA_UserLabel;
    unsigned long int LutL_UserLabelSize;
    } ;
/*                                                                          */
/* Lookup Table Definition Object itemcodes                                 */
/*                                                                          */
#define Img_LutBitsPerComp -1979056100
#define Img_LutDfnType -1979580384
#define Img_LutEntryCount -1979580380
#define Img_LutEntryDataType -1979580376
#define Img_LutEntrySize -1979580372
#define Img_LutEntryTuppleSize -1979580368
#define Img_LutMaxFltValue -1978990540
#define Img_LutMaxIntValue -1979056072
#define Img_LutNumberOfComp -1979580356
#define Img_LutQuantLevels -1979056064
#define Img_LutSpectralMapping -1979580348
#define Img_LutTableAdr -1979580344
#define Img_LutTableLabel -1979711409
#define Img_LutTableLabelSize -1979580336
#define Img_LutTableSize -1979580332
#define Img_LutTableType -1979580328
#define Img_LutUserField -1979580324
#define Img_LutUserLabel -1979711389
#define Img_LutUserLabelSize -1979580316
/*                                                                          */
/* Udp Constants - Class Descriptors                                        */
/*                                                                          */
#define LutK_ClassUBA 1             /* Unaligned bit array              */
#define LutK_ClassUBS 2             /* Unaligned bit string             */
#define LutK_ClassA 3                 /* Atomic Array                     */
#define LutK_ClassCL 4               /* Change list                      */
/*                                                                          */
/* More Frame utils flags                                                   */
/*                                                                          */
#define ImgM_NoDataPlaneAlloc 1024
#define ImgM_AutoDeallocate 2048
#define ImgM_NoStandardize 4096
#define ImgM_NoStructureVerify 8192
#define ImgM_NoAttrVerify 16384
#define ImgM_NoDataPlaneVerify 32768
#define ImgM_NonstandardVerify 65536
#define ImgM_NoChf 131072
#define ImgM_InPlace 262144
#define ImgM_VerifyOn 524288
/*                                                                          */
/* File I/O constants and masks                                             */
/*                                                                          */
#define ImgK_FtypeDDIF 1
#define ImgK_Cda 1
#define ImgK_Qio 2
#define ImgK_RmsBlk 3
#define ImgK_RmsBio 3
#define ImgK_UltrixIo 4
#define ImgM_Asynchronous 32
/*                                                                          */
/* Masks to be used with lookup table functions                             */
/*                                                                          */
#define ImgM_NoLutAttach 1
#define ImgM_NoTableVerify 2
#define ImgM_UserOwnedLut 4
#define ImgM_GetLutAdrOnly 8
/*                                                                          */
/* Symbolic constants used with lookup table utils                          */
/*                                                                          */
#define ImgK_UserLut 1
#define ImgK_PredefLutNoChange 2
#define ImgK_PredefLutBitRvsd 3
#define ImgK_PredefLutNibRvsd 4
#define ImgK_PredefLutLWBin 5
#define ImgK_PredefLutLWBinRvsd 6
#define ImgK_PredefLutASCIIHex 7
#define ImgK_PredefLutASCIIHexRvsd 8
#define ImgK_PredefLutTrashToZeros 9
#define ImgK_PredefLutRGB332ToL1 10
#define ImgK_PredefLutRGB332ToL8 11
#define ImgK_LtypeImplicitIndex 1
#define ImgK_LtypeExplicitIndex 2
/*                                                                          */
/* Compression parameter itemcodes                                          */
/*                                                                          */
#define Img_G32dKfactor 1
#define Img_DctCompFactor 2
/*                                                                          */
/* Constants used to define values for the Img_DataType (Img_DataType)    */
/* attribute.                                                               */
/*                                                                          */
#define ImgK_DataTypePrivate 1
#define ImgK_DataTypeBitstream 2
#define ImgK_DataTypeInteger 3
#endif
