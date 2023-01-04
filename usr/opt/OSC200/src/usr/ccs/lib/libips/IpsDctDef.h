/*  DEC/CMS REPLACEMENT HISTORY, Element SMIDCTDEF.H */
/*  *7    23-JAN-1991 15:05:05 WEBER "Fix bigendian problems" */
/*  *6    18-DEC-1990 17:54:52 SHELLEY "ADD ERROR HANDING (IDS PRIORITY)" */
/*  *5     1-NOV-1990 10:41:36 SHELLEY "first pass adding error handling" */
/*  *4    24-SEP-1990 16:06:05 WEBER "Add block size constant" */
/*  *3    24-SEP-1990 15:52:22 WEBER "Add IPS definitions" */
/*  *2    18-SEP-1990 08:08:33 WEBER "Integrate XIE/IPS" */
/*  *1    18-SEP-1990 07:57:36 WEBER "DCT compression/decompression sources" */
/*  DEC/CMS REPLACEMENT HISTORY, Element SMIDCTDEF.H */

/***********************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/******************************************************************************
**++
**  FACILITY:
**
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains common definitions for DCT 
**	compression/decompression.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	John Nadai
**	John Weber
**
**  CREATION DATE:
**
**	July 31, 1990
**
*******************************************************************************/

#if defined(IPS)
#include <IpsMemoryTable.h>
#endif

/*
**  Equated symbols
*/
    /*
    **	Define TRUE and FALSE
    */
#ifndef	TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
    /*
    **	Token constants
    */
#define DctK_TokenNone	    0
#define DctK_TokenSOI	    1
#define DctK_TokenSOF	    2
#define DctK_TokenSOS	    3
#define DctK_TokenData	    4
#define DctK_TokenRSC	    5
#define DctK_TokenEOI	    6
#define DctK_TokenError	    7
#define DctK_TokenMax	    8
    /*
    **	State constants
    */
#define DctK_StateNew	    0
#define DctK_StateSOI	    1
#define DctK_StateSOF	    2
#define DctK_StateSOS	    3
#define DctK_StateData	    4
#define DctK_StateRSC	    5
#define DctK_StateEOI	    6
#define DctK_StateErr	    7
#define DctK_StateMax	    8
    /*
    **	Marker code constants
    */
#define	DctK_Marker	    0xFF
#define DctK_MarkerSOI	    0xFC
#define DctK_MarkerSOF	    0xE0
#define DctK_MarkerSOS	    0xF8
#define DctK_MarkerRSC	    0xF0
#define DctK_MarkerRSC0	    0xF0
#define DctK_MarkerRSC1	    0xF1
#define DctK_MarkerRSC2	    0xF2
#define DctK_MarkerRSC3	    0xF3
#define DctK_MarkerRSC4	    0xF4
#define DctK_MarkerRSC5	    0xF5
#define DctK_MarkerRSC6	    0xF6
#define DctK_MarkerRSC7	    0xF7
#define DctK_MarkerEOI	    0xFE
    /*
    **	Field length constants
    */
#define DctK_LenFixedSOI    2

#define DctK_LenFixedSOF    13
#define DctK_LenPerCmpSOF   2

#define DctK_LenFixedSOS    5
#define DctK_LenPerCmpSOS   1

#define DctK_LenFixedRSC    2

#define DctK_LenFixedEOI    2
    /*
    **	Segment routine state values.
    */
#define	DctK_StatusDone	     0
#define DctK_StatusContinue -1
#define DctK_StatusError    -2
#define DctK_StatusInput    -3
#define DctK_StatusOutput   -4
    /*
    **	Generic error codes, defined based on facility being built.
    */
#if	defined(XIESMI) || defined(XIEDIX)
#define DctX_Success	    Success
#define	DctX_BadAlloc	    BadAlloc
#define DctX_OutBufSiz	    BadLength
#define DctX_EncodeFail	    BadValue
#define DctX_DecodeFail	    BadValue
#define DctX_BadFactor	    BadValue
#define DctX_UnsOption	    BadImplementation

#elif	defined(IPS)
#define DctX_Success	    IpsX_SUCCESS
#define DctX_BadAlloc	    IpsX_INSVIRMEM
#define DctX_OutBufSiz	    IpsX_INSVIRMEM
#define DctX_EncodeFail	    IpsX_DCTENCODEFAIL
#define DctX_DecodeFail	    IpsX_DCTDECODEFAIL
#define DctX_BadFactor	    IpsX_DCTFACTERR 
#define DctX_UnsOption	    IpsX_UNSOPTION
#define DctX_DctCompIdxErr  IpsX_DCTCOMPIDXERR
#endif
    /*
    **	Parameter constants
    */
#define DctK_BlockSize	    8
#define DctK_Mode	    0
#define DctK_Precision	    8
    /*
    **	Generic parameters defined differently depending on the facility
    **	being built.
    */
#if	defined(XIESMI) || defined(XIEDIX)
#define DctK_MaxComponents XieK_MaxComponents
#elif	defined(IPS)
#define DctK_MaxComponents MAX_NUMBER_OF_COMPONENTS
#endif

/*
**  MACRO definitions
*/

    /*
    **	MACROs for declaring and referencing external data structures.
    */
#ifndef externalref
#if (!defined(ALPHA) && !defined(__alpha)  && defined(VAXC)) 
#define externalref globalref
#else
#define externalref extern
#endif
#endif

#ifndef externaldef
#if (!defined(ALPHA) && !defined(__alpha)  && defined(VAXC)) 
#define externaldef(psect) globaldef {"psect"} noshare
#else
#define externaldef(psect)
#endif
#endif

    /*
    **	MACROs for allocating memory depending on facility.
    */
#if	defined(XIESMI) || defined(XIEDIX)
#define DctCalloc_(count,size)		SmiCalloc((count),(size))
#define DctFree_(adr)			SmiFree((adr))
#define DctMalloc_(size)		SmiMalloc((size))
#define DctMallocBits_(size)		SmiMallocBits((size))
#define DctRealloc_(adr,size)		SmiRealloc((adr),(size))
#define DctReallocBits_(adr,size)	SmiReallocBits((adr),(size))
#elif	    defined(IPS)
#define DctCalloc_(count,size) \
    (*IpsA_MemoryTable[IpsK_Alloc])((count*size),(IpsM_InitMem),(0))
#define DctFree_(adr) \
    (*IpsA_MemoryTable[IpsK_Dealloc])((adr))
#define DctMalloc_(size)  \
    (*IpsA_MemoryTable[IpsK_Alloc])((size),(0),(0))
#define DctMallocBits_(size) \
    (*IpsA_MemoryTable[IpsK_AllocateDataPlane])(((size)+7>>3),(0),(0))
#define DctRealloc_(adr,size) \
    (*IpsA_MemoryTable[IpsK_Realloc])((adr),(size))
#define DctReallocBits_(adr,size) \
    (*IpsA_MemoryTable[IpsK_ReallocateDataPlane])((adr),(((size)+7>>3)))
#endif

/*
**  Quantization table definition
*/
#define DctK_QuantTableSize	64
#define DctK_MaxQuantTables	 4
typedef	struct _QuantTable {
    unsigned char quant[DctK_QuantTableSize];
} QuantTable, *QuantTablePtr;

/*
**  Huffman table definitions
*/
#define DctK_MaxHuffTables 4
#define DctK_MaxHuffTblSize 512
    /*
    **  Generic huffman decode table definition
    */
typedef struct _HuffDecodeEntry {
    short int w1;
    short int w0;
} HuffDecodeEntry, *HuffDecodeEntryPtr;

typedef struct _HuffDecodeTable {
    int half_table_size ;
    struct _HuffDecodeEntry huff_decode_entry[DctK_MaxHuffTblSize];
} HuffDecodeTable, *HuffDecodeTablePtr;
    /*
    **  Huffman AC table definitions
    */
typedef struct _AcSpecTable {
    unsigned char huff_spec_size;
    unsigned char huff_size_histo[16];
    unsigned char sorted_symbols[256];
} AcSpecTable, *AcSpecTablePtr;

typedef struct _AcEncodeTable {
    unsigned char huff_size[256] ;      
    short int     huff_code[256] ;
} AcEncodeTable, *AcEncodeTablePtr;

typedef struct _AcDecodeTable {
    int half_table_size ;
    /* This is the maximum length needed */
    HuffDecodeEntry huff_decode_entry[512];
} AcDecodeTable, *AcDecodeTablePtr;
    /*
    **  Huffman DC table definitions
    */
typedef struct _DcSpecTable {
    unsigned char huff_spec_size;
    unsigned char huff_size_histo[16];
    unsigned char sorted_symbols[16];
} DcSpecTable, *DcSpecTablePtr;

typedef struct _DcEncodeTable {
    unsigned char huff_size[16] ;
    short int     huff_code[16] ;
} DcEncodeTable, *DcEncodeTablePtr;

typedef struct _DcDecodeTable {
    int half_table_size ;
    /* This is the maximum length needed */
    HuffDecodeEntry huff_decode_entry[32];
} DcDecodeTable, *DcDecodeTablePtr;

/*
**  Default tables.
*/
externalref short int	    DctAW_ZigZag[];
externalref QuantTable	    DctR_QuantDefault[];
externalref DcSpecTable	    DctR_DcSpecTable[];
externalref AcSpecTable	    DctR_AcSpecTable[];
externalref DcEncodeTable   DctR_DcEncodeTable[];
externalref DcDecodeTable   DctR_DcDecodeTable[];
externalref AcEncodeTable   DctR_AcEncodeTable[];
externalref AcDecodeTable   DctR_AcDecodeTable[];
