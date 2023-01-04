
/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1991
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
************************************************************************/

/************************************************************************
**  IMG$STANDARDIZE_FILE
**
**  FACILITY:
**
**	DECimage Application Services, Image Services Library
**
**  ABSTRACT:
**
**	The purpose of this program is to convert image data in
**	.DDIF files.  The default is to convert data organized as
**	band-interleaved-by-pixel into band-interleaved-by-plane
**	organization.  Through the use of the appropriate switches,
**	any input data format may be converted into any output
**	data format.
**
**	It reads the file a CDA aggregate at a time, and writes out
**	all non-image data unchanged.  Image data (i.e., DDIF$_SEG
**	aggregates which are marked as "$I" are examined, and
**	converted if necessary).
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
**	10-JAN-1991
**
**  MODIFICATION HISTORY:
**
**	V1-001	MWS001	Mark W. Sornson	    10-JAN-1991
**		Initial module creation.
**
************************************************************************/

/*
**  Table of contents:
*/
		main();

void		GetImgContent();
void		GetSrcImageAndPutDstImage();
unsigned long	TestSegment();

/*
**  Include files:
*/
#if defined(__VMS) || defined(VMS)
#include    <cda$msg.h>
#include    <ddif$def.h>
#else
#include    <cda_msg.h>
#include    <ddif_def.h>
#endif
#include    <stdio.h>

#include    <img/ChfDef.h>
#include    <img/ImgDef.h>

/*
**  MACRO definitions:
**
**	none
*/

/*
**  Equated Symbols:
**
**	constants
*/
#define	NOFLAGS	    0
#define NOINDEX	    0
#define	NORETLEN    0

#define	FMT_STANDARDIZE			0
#define	FMT_BANDBYPIXEL			1
#define	FMT_BANDBYPLANE			2
#define	FMT_BITBYPLANE			3
#define	FMT_BANDBYLINE			4
#define	FMT_BANDBYPIXEL_BITALIGNED	5
#define FMT_LINECNT_CORRECTION		6

/*
**  External References:
**
**	External Entry Points
*/
unsigned long	 cda$close_file();
unsigned long	 cda$create_file();
unsigned long	 cda$create_root_aggregate();
unsigned long	 cda$delete_aggregate();
unsigned long	 cda$get_aggregate();
unsigned long	 cda$insert_aggregate();
unsigned long	 cda$locate_item();
unsigned long	 cda$open_file();
unsigned long	 cda$put_aggregate();
unsigned long	 cda$store_item();

unsigned long	 ImgCvtAlignment();
void		 ImgDeallocateFrame();
unsigned long	 ImgStandardizeFrame();

unsigned long	_ImgCreateFrameFromSegAggr();

unsigned long	IpsCorrectScanlineCount();

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
** Main routine
******************************************************************************/
main( argc, argv )
long	  argc;
char	**argv;
{
char		*dst_file_name	    = NULL;
char		 fname_buf[255];
char		*src_file_name	    = NULL;
int		 error_flag	    = 0;
int		 format_type	    = FMT_STANDARDIZE;
int		 i;
long		 seg_is_an_image;
unsigned long	 aggr_type;
unsigned long	 dst_aggr_handle;
unsigned long	 dst_file_ctx;
unsigned long	 dst_file_name_len  = 0;
unsigned long	 dst_root_aggr;
unsigned long	 dst_stream_ctx;
unsigned long	 get_next_aggr;
unsigned long	 scope;
unsigned long	 src_aggr_handle;
unsigned long	 src_fid	    = 1;
unsigned long	 src_file_ctx;
unsigned long	 src_file_name_len  = 0;
unsigned long	 src_root_aggr;
unsigned long	 src_stream_ctx;
unsigned long	 status;

/*
** Parse the command line.
*/
for (i = 1; i < argc; i++ )
    {
    /*
    ** Get the arguments
    */
    if ( strcmp( argv[i], "-f" ) == 0 )
	{
	if ( ++i >= argc )
	    break;
	/*
	** Get the output format type
	*/
	if ( strcmp( argv[i], "0" ) == 0 )
	    format_type = FMT_STANDARDIZE;
	else if ( strcmp( argv[i], "1" ) == 0 )
	    format_type = FMT_BANDBYPIXEL;
	else if ( strcmp( argv[i], "2" ) == 0 )
	    format_type = FMT_BANDBYPLANE;
	else if ( strcmp( argv[i], "3" ) == 0 )
	    format_type = FMT_BITBYPLANE;
	else if ( strcmp( argv[i], "4" ) == 0 )
	    format_type = FMT_BANDBYLINE;
	else if ( strcmp( argv[i], "5" ) == 0 )
	    format_type = FMT_BANDBYPIXEL_BITALIGNED;
	else if ( strcmp( argv[i], "6" ) == 0 )
	    format_type = FMT_LINECNT_CORRECTION;
	else
	    {
	    error_flag = 1;
	    break;
	    }
	}
    /*
    ** Get the Input and Output Filenames
    */
    else
	{
	src_file_name = argv[i++];
	src_file_name_len = strlen( src_file_name );
	/*
	** Get the output file
	*/
	if ( i < argc )
	    {
	    dst_file_name = argv[i++];
	    dst_file_name_len = strlen( dst_file_name );
	    /*
	    ** Make sure the output file is the last thing on the command line
	    */
	    if ( i < argc )
		error_flag = 1;
	    }
	break;
	}
    }

/*
** Input and/or Output Filenames are missing
*/
if ( src_file_name == NULL)
    {
    memset( fname_buf, 0, 255 );
    printf( "Enter input file name: " );
    (void) fgets( fname_buf, 255, stdin );
    src_file_name_len = strlen( fname_buf );
    if ( src_file_name_len <= 1 )
	error_flag = 1;
    else
	{
	/*
	** NOTE that the filename length includes the carriage return
	*/
	src_file_name = (char *) calloc( 1, src_file_name_len );
	memcpy( src_file_name, fname_buf, src_file_name_len - 1 );
	}
    }
if ( dst_file_name == NULL)
    {
    memset( fname_buf, 0, 255 );
    printf( "Enter output file name: " );
    (void) fgets( fname_buf, 255, stdin );
    dst_file_name_len = strlen( fname_buf );
    if ( dst_file_name_len <= 1 )
	error_flag = 1;
    else
	{
	/*
	** NOTE that the filename length includes the carriage return
	*/
	dst_file_name = (char *) calloc( 1, dst_file_name_len );
	memcpy( dst_file_name, fname_buf, dst_file_name_len - 1 );
	}
    }
/*
** Print the usage message
*/
if ( error_flag )
    {
    printf ("Usage:  %s\n\
\t[-f (output format)\n\
\t[0] (standardize format; V3.0, the default)\n\
\t[1] (band-interleaved-by-pixel format)\n\
\t[2] (band-interleaved-by-plane format)\n\
\t[3] (bit-interleaved-by-plane format)\n\
\t[4] (band-interleaved-by-line format)\n\
\t[5] (band-interleaved-by-line, bit aligned format; V2.0)\n\
\t[6] (line count correction for scan-compressed bitonal images)\n\
] input_file output_file\n",
	argv[0]);
    exit (0);
    }
/*
** Open the source file and create the destination file.
*/
aggr_type = DDIF$_DDF;
status = cda$open_file(
	     &src_file_name_len
	    ,src_file_name
	    ,0,0		/* no default filename string	*/
	    ,0,0,0		/* use default memory mgt	*/
	    ,&aggr_type
	    ,0			/* no file options		*/
	    ,0,0,0		/* no return filename info	*/
	    ,&src_stream_ctx
	    ,&src_file_ctx
	    ,&src_root_aggr
	    );

if ( !(status & 1) )
    ChfStop( 1, status );

status = cda$create_root_aggregate(
	     0,0,0		/* use default memory mgt	*/
	    ,0			/* no file options		*/
	    ,&aggr_type
	    ,&dst_root_aggr
	    );

if ( !(status & 1) )
    ChfStop( 1, status );

status = cda$create_file(
	     &dst_file_name_len
	    ,dst_file_name
	    ,0,0		/* no default filename string	*/
	    ,0,0,0		/* use default memory mgt	*/
	    ,&dst_root_aggr
	    ,0,0,0		/* no return filename info	*/
	    ,&dst_stream_ctx
	    ,&dst_file_ctx
	    );

if ( !(status & 1) )
    ChfStop( 1, status );

/*
** Read the source file an aggregate at a time, looking for DDIF$_SEG
** aggregates.  If we find one, test to see if it's an image frame.
** If it's an image frame, get the image content elements, and use
** the aggregates to create an ISL image frame.  Test the frame to see
** if it needs to be converted, and convert it if it does.
*/

/*
**	Get the first aggr of data from the source ...
*/
status = cda$get_aggregate(
	     &src_root_aggr
	    ,&src_stream_ctx
	    ,&src_aggr_handle
	    ,&aggr_type
	    );
if ( !(status & 1) )
    ChfStop( 1, status );

/*
**	Loop, and put aggrs to dst file, testing for img segments as
**	we go ...
*/
get_next_aggr = 1;
while ( get_next_aggr ) 
    {
    /*
    ** Test for IMG seg ...
    */
    switch ( aggr_type )
	{
	case DDIF$_DSC:
	    scope = DDIF$K_DOCUMENT_SCOPE;
	    status = cda$enter_scope(
			 &dst_root_aggr
			,&dst_stream_ctx
			,&scope );
	    if ( !(status & 1) )
		ChfSignal( 1, status );

	    status = cda$put_aggregate(
			  &src_root_aggr
			 ,&dst_stream_ctx
			 ,&src_aggr_handle
			 );
	    if ( !(status & 1) )
		ChfSignal( 1, status );

	    dst_aggr_handle = src_aggr_handle;
	    break;
	case DDIF$_DHD:
	    status = cda$put_aggregate(
			  &src_root_aggr
			 ,&dst_stream_ctx
			 ,&src_aggr_handle
			 );
	    if ( !(status & 1) )
		ChfSignal( 1, status );

	    scope = DDIF$K_CONTENT_SCOPE;
	    status = cda$enter_scope(
			 &dst_root_aggr
			,&dst_stream_ctx
			,&scope );
	    if ( !(status & 1) )
		ChfSignal( 1, status );

	    dst_aggr_handle = src_aggr_handle;
	    break;
	case DDIF$_SEG:
	    seg_is_an_image = TestSegment(   src_root_aggr
					    ,src_aggr_handle );

	    if ( seg_is_an_image )
		{
		GetSrcImageAndPutDstImage(   src_root_aggr
					    ,src_aggr_handle
					    ,src_stream_ctx
					    ,dst_root_aggr
					    ,dst_stream_ctx
					    ,format_type );

		dst_aggr_handle = src_aggr_handle;
		}
	    else
		{
		scope = DDIF$K_SEGMENT_SCOPE;
		status = cda$enter_scope(
			     &dst_root_aggr
			    ,&dst_stream_ctx
			    ,&scope
			    ,&src_aggr_handle );
		if ( !(status & 1) )
		    ChfSignal( 1, status );

		dst_aggr_handle = src_aggr_handle;
		}
	    break;
	case DDIF$_EOS:
	    scope = DDIF$K_SEGMENT_SCOPE;
	    status = cda$leave_scope(
			 &dst_root_aggr
			,&dst_stream_ctx
			,&scope
			);
	    if ( !(status & 1) )
		ChfSignal( 1, status );
	    break;
	default:
	    /*
	    ** Put the non-image aggr info to the dst file ...
	    */
	    dst_aggr_handle = src_aggr_handle;
	    status = cda$put_aggregate(
			 &src_root_aggr
			,&dst_stream_ctx
			,&src_aggr_handle
			);
	    if ( !(status & 1) )
		ChfSignal( 1, status );
	    break;
	} /* end of switch */


    /*
    ** Delete the dst and src aggrs
    */
    if ( aggr_type != DDIF$_EOS )
	{
	if ( dst_aggr_handle != src_aggr_handle )
	    {
	    status = cda$delete_aggregate(
			 &dst_root_aggr
			,&dst_aggr_handle );

	    if ( !(status & 1) )
		ChfSignal( 1, status );
	    }

	status = cda$delete_aggregate(
		     &src_root_aggr
		    ,&src_aggr_handle );
	if ( !(status & 1) )
	    ChfSignal( 1, status );
	}

    /*
    ** Get the next aggr from the source file ...
    */
    status = cda$get_aggregate(
		 &src_root_aggr
		,&src_stream_ctx
		,&src_aggr_handle
		,&aggr_type
		);
    switch ( status )
	{
	case CDA$_NORMAL:
	    break;
	case CDA$_ENDOFDOC:
	    get_next_aggr = 0;
	    break;
	default:
	    ChfSignal( 1, status );
	} /* end of switch */
    } /* end of while */

/*
** All done ... close the dst content and document scopes, and close
** all the open files.
*/
scope = DDIF$K_CONTENT_SCOPE;
status = cda$leave_scope(
	     &dst_root_aggr
	    ,&dst_stream_ctx
	    ,&scope
	    );
if ( !(status & 1) )
    ChfStop( 1, status );

scope = DDIF$K_DOCUMENT_SCOPE;
status = cda$leave_scope(
	     &dst_root_aggr
	    ,&dst_stream_ctx
	    ,&scope
	    );
if ( !(status & 1) )
    ChfStop( 1, status );

status = cda$close_file(
	     &src_stream_ctx
	    ,&src_file_ctx );
if ( !(status & 1) )
    ChfStop( 1, status );


status = cda$close_file(
	     &dst_stream_ctx
	    ,&dst_file_ctx );
if ( !(status & 1) )
    ChfStop( 1, status );

return;
} /* end of main routine */


/******************************************************************************	
** GetImgContent
******************************************************************************/
void GetImgContent( src_root_aggr, src_seg_aggr, src_stream_ctx )
unsigned long	src_root_aggr;
unsigned long	src_seg_aggr;
unsigned long	src_stream_ctx;
{
unsigned long	aggr_handle;
unsigned long	aggr_item;
unsigned long	aggr_type;
unsigned long	get_next_aggr	    = 1;
unsigned long	ice_count	    = 0;
unsigned long	item_size;
unsigned long	prev_aggr_handle;
unsigned long	status;

/*
** Get image data units.
*/
while ( get_next_aggr )
    {
    status = cda$get_aggregate(
		&src_root_aggr,		/* NOTE that we GET the	*/
		&src_stream_ctx,	/* aggr with the USER	*/
		&aggr_handle,		/* root, but store it	*/
		&aggr_type );		/* with the frame root.	*/
    if ( (status&1) != 1 )
	ChfSignal( 1,  status );

    switch ( aggr_type )
    	{
    	case DDIF$_IMG:
	    /*
	    ** Note that Getting the DDIF$_IMG aggregate gets ALL
	    ** the DDIF$_IDU aggregates attached to it.  (Yahoo!)
	    */
	    if (ice_count == 0)
		/*
		** "Store" the first DDIF$_IMG aggregate as an item
		** of the DDIF$_SEG aggregate.
		*/
		{
                aggr_item = DDIF$_SEG_CONTENT;
                item_size = sizeof(aggr_item);
		status = cda$store_item(
			    &src_root_aggr,
			    &src_seg_aggr,
			    &aggr_item,
			    &item_size,
			    &aggr_handle,
			    0, 0 );
		if ( (status&1) != 1 )
		    ChfSignal( 1,  status );
		prev_aggr_handle = aggr_handle;
		}
	    else
		/*
		** Insert all subsequent DDIF$_IMG aggregates as elements
		** of a sequence of DDIF$_IMG aggregates, with each new
		** aggregate following the previous aggregate.
		*/
		{
		status = cda$insert_aggregate(
			    &aggr_handle,
			    &prev_aggr_handle );
		if ( (status&1) != 1 )
		    ChfSignal( 1,  status );
		prev_aggr_handle = aggr_handle;
		}
	    ++ice_count;		    /* count the ICEs		*/
            break;
    	case DDIF$_EOS:
            get_next_aggr = 0;
            break;
    	default:			    /* skip non-img aggrs	*/
	    break;			    /* though there shouldn't	*/
					    /* be any.			*/
    	} /* end of switch */
    } /* end of while loop */

return;
} /* end of GetImgContent */



/******************************************************************************
** GetSrcImageAndPutDstImage
******************************************************************************/
void GetSrcImageAndPutDstImage( src_root_aggr, src_seg_aggr, src_stream_ctx,
				dst_root_aggr, dst_stream_ctx, format_type )
unsigned long	src_root_aggr;
unsigned long	src_seg_aggr;
unsigned long	src_stream_ctx;
unsigned long	dst_root_aggr;
unsigned long	dst_stream_ctx;
int		format_type;
{
int		 bit_align_flag 	= 0;
int		 new_frame_flag 	= 0;
long		 totalquantbitsperpixel;
unsigned long	 aggr_item;
unsigned long	 compression_type;
unsigned long	 format, format_return;
unsigned long	 frame_img_aggr;
unsigned long	 frame_seg_aggr;
unsigned long	 frame_root_aggr;
unsigned long	 itemcode;
unsigned long	 img_aggr;
unsigned long	*img_aggr_ptr;
unsigned long	 length;
unsigned long	 outfid			= 0;
unsigned long	 scope_code;
unsigned long	 srcfid;
unsigned long	 srcfid_was_compressed	= 0;
unsigned long	 status;
unsigned long	 wrkfid;

/*
** Get the DDIF$_IMG content aggregates from the source file and
** attach them to the DDIF$_SEG aggregate that was passed in.
*/

GetImgContent( src_root_aggr, src_seg_aggr, src_stream_ctx );

/*
** Create a dummy frame using the cda aggrs read in from the sourc
*/
srcfid = _ImgCreateFrameFromSegAggr( src_root_aggr, src_seg_aggr );

/*
** Verify that the frame attributes aren't corrupted in some way
*/
ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

/*
** Determine what to do (including which type of output format to use) ...
*/
switch (format_type)
    {
    case FMT_STANDARDIZE:
	itemcode = Img_StandardFormat;
	format = 1;
	break;

    case FMT_BANDBYPIXEL:
	itemcode = Img_CompSpaceOrg;
	format = ImgK_BandIntrlvdByPixel;
	break;

    case FMT_BANDBYPLANE:
	itemcode = Img_CompSpaceOrg;
	format = ImgK_BandIntrlvdByPlane;
	break;

    case FMT_BITBYPLANE:
	itemcode = Img_CompSpaceOrg;
	format = ImgK_BitIntrlvdByPlane;
	break;

    case FMT_BANDBYLINE:
	itemcode = Img_CompSpaceOrg;
	format = ImgK_BandIntrlvdByLine;
	break;

    case FMT_BANDBYPIXEL_BITALIGNED:
	itemcode = Img_CompSpaceOrg;
	format = ImgK_BandIntrlvdByPixel;
	bit_align_flag = 1;
	break;

    case FMT_LINECNT_CORRECTION:
	break;

    default:
	break;

    } /* end of switch */

if ( format_type != FMT_LINECNT_CORRECTION )
    {
    /*
    ** If the frame is OK up to this point (meaning that ImgVerifyFrame
    ** didn't generate any signals that would stop the process), decompress it 
    ** if it's compressed, so we can tell if it's in standard format.
    **
    **  NOTE:   assume all data planes are compressed with the same scheme
    */
    ImgGet( srcfid, Img_CompressionType, &compression_type, 
		    sizeof(compression_type), 0, 0 );
    if ( compression_type != ImgK_PcmCompression )
	{
	outfid = ImgDecompressFrame( srcfid, 0, 0 );
	srcfid_was_compressed = 1;
	}
    else
	outfid = srcfid;


    /*
    ** See if the frame is already in the proper format.  If not, format it
    */
    ImgGet( outfid, itemcode, &format_return, sizeof( format_return ), 0, 0 );
    if ( format_return != format )
	{
	if ( itemcode == Img_StandardFormat )
	    wrkfid = ImgStandardizeFrame( outfid, 0 );
	else
	    {
	    struct ITMLST 	itmlst[2];

	    itmlst[0].ItmL_Code = itemcode;
	    itmlst[0].ItmL_Length = sizeof( format );
	    itmlst[0].ItmA_Buffer = (char *) &format;
	    itmlst[0].ItmA_Retlen = 0;
	    itmlst[0].ItmL_Index = 0;
	    itmlst[1].ItmL_Code = 0;
	    itmlst[1].ItmL_Length = 0;
	    itmlst[1].ItmA_Buffer = 0;
	    itmlst[1].ItmA_Retlen = 0;
	    itmlst[1].ItmL_Index = 0;

	    wrkfid = ImgConvertFrame( outfid, itmlst, 0 );
	    }
	/*
	** Deallocate the unused fid and indicate that a new frame was created
	*/
	ImgDeallocateFrame( outfid );
	outfid = wrkfid;
	new_frame_flag = 1;
	}
    }
else
    /*
    ** Look for and correct line count problem in compressed bitonal
    ** images which came from scanners which compress the data but
    ** don't correctly report the number of lines (when the images
    ** are actually shorter than whatever the expected size was).
    */
    {
    long	ips_status;
    long	scanline_count;
    struct UDP	udp;

    ImgGet( srcfid, Img_CompressionType, &compression_type, 
		    sizeof(compression_type), 0, 0 );

    switch ( compression_type )
	{
	/*
	** If the image is CCITT compressed, get the scanline count
	** from the compressed data and put the found value into the
	** source frame.
	*/
	case ImgK_G31dCompression:
	case ImgK_G32dCompression:
	case ImgK_G42dCompression:
	    ImgGet( srcfid, Img_Udp, &udp, sizeof( udp ), 0, 0 );
	    status = IpsCorrectScanlineCount(	&udp,
						compression_type,
						&scanline_count );
	    ImgSet( srcfid, Img_NumberOfLines, &scanline_count, 
			sizeof( scanline_count ), 0 );
	    break;
	/*
	** Don't do anything to any other type of frame.
	*/
	default:
	    break;
	} /* end switch */

    /*
    ** Note that for this function, we're all done.  The srcfid will be
    ** used as the output fid below.  Also note that we don't have a
    ** new frame.
    */
    }

/*
** Make data bit aligned (remove padding), if requested
*/
if ( bit_align_flag )
    {
    unsigned long	totalbitsperpixel;


    ImgGet( outfid, Img_TotalQuantBitsPerPixel, &totalquantbitsperpixel,
		    sizeof( totalquantbitsperpixel ), 0, 0);
    ImgGet( outfid, Img_TotalBitsPerPixel, &totalbitsperpixel,
		    sizeof( totalbitsperpixel ), 0, 0);
    /*
    ** Only bit align the data if it's padded
    */
    if ( totalquantbitsperpixel < totalbitsperpixel )
	{
	int		i;
	unsigned long	pixelsperline;
	unsigned long	numberofcomp;
	unsigned long	*quantbitspercomp, *qp;
	unsigned long	dataoffset;
	unsigned long	planebitsperpixel;
	unsigned long	pixelstride;
	unsigned long	scanlinestride;
	struct ITMLST	*itmlst, *ip;

	ImgGet( outfid, Img_PixelsPerLine, &pixelsperline,
			sizeof( pixelsperline ), 0, 0);
	ImgGet( outfid, Img_NumberOfComp, &numberofcomp,
			sizeof( numberofcomp ), 0, 0);
	/*
	** Allocate an item list large enough to hold the 4 we
	** know we'll need, 1 item for termination, and 1 item
	** for each component in the number of components.
	*/
	itmlst = (struct ITMLST *) 
	    malloc( sizeof (struct ITMLST) * (5 + numberofcomp) );

	dataoffset = 0;
	planebitsperpixel = totalquantbitsperpixel;
	pixelstride = totalquantbitsperpixel;
	scanlinestride = (pixelsperline * totalquantbitsperpixel);
	/*
	** Set the bit allignment to be fully packed
	*/
	ip = itmlst;
	ip->ItmL_Code = Img_DataOffset;
	ip->ItmL_Length = sizeof( dataoffset );
	ip->ItmA_Buffer = (char *) &dataoffset;
	ip->ItmA_Retlen = 0;
	ip->ItmL_Index = 0;
	ip++;
	ip->ItmL_Code = Img_PlaneBitsPerPixel;
	ip->ItmL_Length = sizeof( planebitsperpixel );
	ip->ItmA_Buffer = (char *) &planebitsperpixel;
	ip->ItmA_Retlen = 0;
	ip->ItmL_Index = 0;
	ip++;
	ip->ItmL_Code = Img_PixelStride;
	ip->ItmL_Length = sizeof( pixelstride );
	ip->ItmA_Buffer = (char *) &pixelstride;
	ip->ItmA_Retlen = 0;
	ip->ItmL_Index = 0;
	ip++;
	ip->ItmL_Code = Img_ScanlineStride;
	ip->ItmL_Length = sizeof( scanlinestride );
	ip->ItmA_Buffer = (char *) &scanlinestride;
	ip->ItmA_Retlen = 0;
	ip->ItmL_Index = 0;
	ip++;
	/*
	** For each component, get and store the quant bits per component
	*/
	quantbitspercomp = (unsigned long *) 
		    malloc (sizeof (unsigned long) * numberofcomp);
	for (i = 0, qp = quantbitspercomp; i < numberofcomp; i++, qp++, ip++)
	    {
	    ImgGet( outfid, Img_QuantBitsPerComp, qp, sizeof( long ), 0, i);
	    ip->ItmL_Code = Img_BitsPerComp;
	    ip->ItmL_Length = sizeof( long );
	    ip->ItmA_Buffer = (char *) qp;
	    ip->ItmA_Retlen = 0;
	    ip->ItmL_Index = 0;
	    }
	/*
	** Terminate the item list
	*/
	ip->ItmL_Code = 0;
	ip->ItmL_Length = 0;
	ip->ItmA_Buffer = 0;
	ip->ItmA_Retlen = 0;
	ip->ItmL_Index = 0;
	/*
	** Convert the alignment
	*/
	wrkfid = ImgCvtAlignment( outfid, itmlst, 0 );
	/*
	** Deallocate the unused fid and ndicate that a new frame was created
	*/
	ImgDeallocateFrame( outfid );
	outfid = wrkfid;
	new_frame_flag = 1;
	/*
	** Free up the space
	*/
	free( quantbitspercomp );
	free( itmlst );
	}
    }

/*
** Decide what to output.
**	If we have a new frame, output that.
**	Otherwise, output the original, source frame.
*/
if ( new_frame_flag )
    {
    /*
    ** If the original frame was compressed, compress the output frame.
    ** Otherwise, the output frame is fine as it is.
    */
    if ( srcfid_was_compressed )
	{
	ImgDeallocateFrame( srcfid );
	/*
	** Recompress the new frame.
	**
	**  NOTE:   Since compression parameters aren't returned by the
	**	    decompression code, it's not possible to use the same
	**	    ones when recompressing.
	*/
	outfid = ImgCompressFrame( outfid, compression_type, ImgM_InPlace, 0 );
	}
    }
else
    {
    if ( srcfid_was_compressed )
	ImgDeallocateFrame( outfid );
    outfid = srcfid;
    }
/*
** Put the image data in the frame out to the destination
** stream an aggregate at a time.
*/
ImgGet( outfid, Img_RootAggr, &frame_root_aggr, sizeof(long), 0, 0 );
ImgGet( outfid, Img_SegAggr, &frame_seg_aggr, sizeof(long), 0, 0 );

scope_code = DDIF$K_SEGMENT_SCOPE;
status = cda$enter_scope(
	     &dst_root_aggr
	    ,&dst_stream_ctx
	    ,&scope_code
	    ,&frame_seg_aggr
	    );
if ( !(status & 1) )
    ChfSignal( 1, status );

aggr_item = DDIF$_SEG_CONTENT;
status = cda$locate_item(
	     &frame_root_aggr
	    ,&frame_seg_aggr
	    ,&aggr_item
	    ,&img_aggr_ptr
	    ,&length
	    ,0 ,0 );
if ( !(status & 1) )
    ChfSignal( 1, status );
img_aggr = *img_aggr_ptr;

status = cda$put_aggregate(
	     &frame_root_aggr
	    ,&dst_stream_ctx
	    ,&img_aggr
	    );
if ( !(status & 1) )
    ChfSignal( 1, status );

status = cda$leave_scope(
	     &dst_root_aggr
	    ,&dst_stream_ctx
	    ,&scope_code
	    );
if ( !(status & 1) )
    ChfSignal( 1, status );

/*
** Deallocate the output ISL frame created
*/
ImgDeallocateFrame( outfid );

return;
} /* end of GetSrcImageAndPutDstImage */


/******************************************************************************
** TestSegment
**
**  Determine whether this is an image frame segment (containing the
**  $I tag), or not.
******************************************************************************/
unsigned long TestSegment( root_aggr, seg_aggr )
unsigned long	root_aggr;
unsigned long	seg_aggr;
{
char	*content_category_tag_string;
long	 add_info;
long	 aggr_index;
long	 aggr_item;
long	 argcnt;
long	 img_seg_found = 0;
long	*item;
long	 length;
long	 sga_aggr;
long	 status;

/*
** Locate the specific segment attributes.  NOTE that if there aren't
** any, this means that the segment is definitely NOT an image segment.
*/
aggr_item = DDIF$_SEG_SPECIFIC_ATTRIBUTES;
status = cda$locate_item(
	    &root_aggr,
	    &seg_aggr,
	    &aggr_item,
	    &item,
	    &length, 0, 0 );

switch ( status )
    {
    case CDA$_NORMAL:
    case CDA$_DEFAULT:
	sga_aggr = *item;
	/*
	** Locate the content category tag and see if it contains the
	** image content tag string "$I".  (This is determined by looking
	** at the format returned as add_info.)
	*/
        aggr_item = DDIF$_SGA_CONTENT_CATEGORY;
	status = cda$locate_item(
	        &root_aggr,
		&sga_aggr,
	        &aggr_item,
		&item,
	        &length,
		0,
	        &add_info );
	switch ( status )
	    {
	    case CDA$_NORMAL:
	    case CDA$_DEFAULT:
		content_category_tag_string = (char *) *item;
		if ( add_info == DDIF$K_I_CATEGORY )
		    img_seg_found = 1;	/* we have an image segment */
		break;
	    case CDA$_EMPTY:
		break;
	    default:
		ChfSignal( 1,  status );
	    } /* end switch */

    case CDA$_EMPTY:
	break;
    default:
	ChfSignal( 1,  status );
    } /* end switch */

return img_seg_found;
} /* end of TestSegment */
