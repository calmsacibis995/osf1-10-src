/********************************************************************************************************************************/
/* Created  6-OCT-1993 09:15:44 by VAX SDL V3.2-12     Source:  6-OCT-1993 09:15:21 VAMASD$:[SYSLIB.VAXVMS]IPSDEF.SDL;1 */
/********************************************************************************************************************************/
 
/*** MODULE IPSDEF ***/
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
