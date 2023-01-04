/************************************************************************
**
**			  COPYRIGHT (c) 1989 BY
**	      DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
** ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
** COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
************************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      ISL
**
**  ABSTRACT:
**
**      Examples of ISL routines for Ultrix
**
**	dither - qualifiers bluenoise, clustered, dispersed
**	flip   - qualifiers horizontal, vertical, both
**
**  ENVIRONMENT:
**
**      VAX Ultrix
**
**  AUTHOR(S):
**
**      John M. Roycroft
**
**  CREATION DATE:
**
**      March 13, 1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
#include <math.h>
#include <img/ImgDef.h>
#ifdef VMS
#include <stdlib.h>
#endif

#define BITMAP		1
#define COMBINE		2
#define COMPRESS	3
#define CONVERT		4
#define CONVOLVE	5
#define COPY		6
#define DITHER		7
#define DSTROI		8
#define ERASE		9
#define FILTER		10
#define FLIP		11
#define HISTOGRAM	12
#define PAGE		13
#define POLARITY	14
#define PS		15
#define REMAP		16
#define REQUANTIZE	17
#define ROI		18
#define ROTATE		19
#define SCALE		20
#define SIXEL		21
#define TONESCALE	22

#define FLAG_COMPRESS	1 << 0
#define FLAG_PAGEBREAK	1 << 1

#define RED		0
#define GREEN		1
#define BLUE		2

#define BUFFER_SIZE 75000 /* NOTE: Check image size */
/*
** User switches
*/
char *parse_table[] =
    {"bitmap","combine","compress","convert","convolve",
      "copy","dither","dstroi","erase","filter",
      "flip","histogram","page","polarity","ps",
      "remap","requantize","roi","rotate","scale",
      "sixel","tonescale",0};

static char user_label[] = {"This string was produced using ImgExamples"};
static long test_mask;
/*
**  Structures GET_ITMLST and PUT_ITMLST exist in ImgDef.h
*/
struct GET_ITMLST getlst[9];

struct PUT_ITMLST *dither_itmlst;
struct PUT_ITMLST greyscale_itmlst[5];
struct PUT_ITMLST rgb_itmlst[7];

struct PUT_ITMLST remap_itmlst[10];

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


#define E_A2D(ardesc,x,y)  ((long) (((((x) - (ardesc)->A2D_L_L1) * \
    (ardesc)->A2D_L_M2) + ((y) - (ardesc)->A2D_L_L2)) \
    * (ardesc)->A2D_W_LENGTH)  + ((long) (ardesc)->A2D_A_POINTER))
 
#define DSC_K_DTYPE_F   10 /* F_floating, 32-bit single-precision floating point */
#define DSC_K_CLASS_A   4               /* array descriptor */
/*
** Local functions
*/
int GetSwitchValue();
unsigned long RGBtoGreyscale();
void PrintSparseHistogram();

main(argc,argv)
int argc;
char *argv[];
{/* Main program */
/*
** Parameter variables used by ISL routines
*/
unsigned long	compression_scheme, fid, fid2, new_fid, 
		hist_id, roi, save_roi, dstroi, save_dstroi, 
	        input_ctx, output_ctx;
/*
** Program variables
*/
FILE *file_ptr;
char *data_buffer;
int menu_choice;
int index;
int i,j,k,l,m,n;
/*
** Image attributes used by routines
*/
    int spect_type;
    int brt_polarity;
    int spectral_mapping;
    int comp_space_org;
    int number_of_comp;
    int img_bits_per_comp[3];
    int pixels_per_line;
    int compression_type;
    int bits_per_pixel;

/* 
** Image attributes set by routines (ImgDither), (ImgPixelRemap)
*/
    int new_brt_polarity; 
    int new_spectral_mapping;
    int new_number_of_comp;
    int new_img_bits_per_comp[3];
    int new_pixels_per_line;
    int new_pixel_stride;
    int new_scanline_stride;
    int new_bits_per_pixel;

if (argc < 2)
    {
    printf("Specify INPUT and OUTPUT file\n");
    exit(0);
    }

/*
** Open file for import
*/
input_ctx = ImgOpenDDIFFile(
                ImgK_ModeImport,           /* MODE, Import or Export         */
		strlen(argv[1]),           /* INFILELEN, Input file length   */
		argv[1],	           /* INFILEBUF, Input file ptr      */
		0,			   /* RETFILELEN, Return file length */
		0,			   /* RETFILEBUF, Return file ptr    */
                0);			   /* FLAGS,     Processing flags    */

fid = ImgImportDDIFFrame(
         input_ctx,		            /* CTX,DDIF stream context       */
         0,			            /* BUFPTR, Pointer to data       */
         0,			            /* BUFLEN, Size of buffer        */
         0,			            /* FLAGS,  Processing flags      */
         0,			            /* ACTION, User routine          */
         0);			            /* USRPRM, Action parameter      */

ImgCloseDDIFFile(
   input_ctx,		                    /* CTX,   DDIF stream context    */
   0);			                    /* FLAGS, Processing flags       */
 
/*
** Initialize item list used by ImgGetFrameAttributes()
*/
getlst[0].GetL_Code    = Img_SpectType;
getlst[0].GetL_Length  = sizeof(spect_type);
getlst[0].GetA_Buffer  = (char *)&spect_type;
getlst[0].GetA_Retlen  = 0;
getlst[0].GetL_Index   = 0;

getlst[1].GetL_Code    = Img_BrtPolarity;
getlst[1].GetL_Length  = sizeof(brt_polarity);
getlst[1].GetA_Buffer  = (char *)&brt_polarity;
getlst[1].GetA_Retlen  = 0;
getlst[1].GetL_Index   = 0;

getlst[2].GetL_Code    = Img_SpectralMapping;
getlst[2].GetL_Length  = sizeof(spectral_mapping);
getlst[2].GetA_Buffer  = (char *)&spectral_mapping;
getlst[2].GetA_Retlen  = 0;
getlst[2].GetL_Index   = 0;

getlst[3].GetL_Code    = Img_CompSpaceOrg;
getlst[3].GetL_Length  = sizeof(comp_space_org);
getlst[3].GetA_Buffer  = (char *)&comp_space_org;
getlst[3].GetA_Retlen  = 0;
getlst[3].GetL_Index   = 0;

getlst[4].GetL_Code    = Img_NumberOfComp;
getlst[4].GetL_Length  = sizeof(number_of_comp);
getlst[4].GetA_Buffer  = (char *)&number_of_comp;
getlst[4].GetA_Retlen  = 0;
getlst[4].GetL_Index   = 0;

getlst[5].GetL_Code    = Img_PixelsPerLine;
getlst[5].GetL_Length  = sizeof(pixels_per_line);
getlst[5].GetA_Buffer  = (char *)&pixels_per_line;
getlst[5].GetA_Retlen  = 0;
getlst[5].GetL_Index   = 0;

getlst[6].GetL_Code    = Img_CompressionType;
getlst[6].GetL_Length  = sizeof(compression_type);
getlst[6].GetA_Buffer  = (char *)&compression_type;
getlst[6].GetA_Retlen  = 0;
getlst[6].GetL_Index   = 0;

getlst[7].GetL_Code    = Img_BitsPerPixel;
getlst[7].GetL_Length  = sizeof(bits_per_pixel);
getlst[7].GetA_Buffer  = (char *)&bits_per_pixel;
getlst[7].GetA_Retlen  = 0;
getlst[7].GetL_Index   = 0;

getlst[8].GetL_Code    = 0;                /* Itemlist terminated with zero */

/*
**	Using itemlist, obtain image attributes
*/
ImgGetFrameAttributes(
   fid,	                                        /* FID, ISL frame ID        */
   getlst);                                     /* ITMLST, Frame attributes */

/*
** If image is compressed, then decompress it.
*/
if (compression_type != ImgK_PcmCompression)
    ImgDecompress(fid);                          /* FID, ISL frame ID        */

/*
** Label the frame to demonstrate the use of IMGSET
*/
fid = ImgSet(
         fid,                                 /* FID, ISL frame identifier  */
         Img_UserLabel,                       /* ITEMCODE, See ImgDef.h     */
         user_label,                          /* BUFPTR, Buffer with item   */
         sizeof(user_label),                  /* BUFLEN, Sizeof buffer      */
         0);                                  /* INDEX, Used in array       */
/*
** Reserve index 1 and 2 for input and output files (start at 3).
*/
index       = 3;
/*
** Zero all roi identifiers.
**
** NOTE: ImgCreateRoi should be called first in the command string.
*/
roi         = 0;
dstroi      = 0;
save_roi    = 0;
save_dstroi = 0;

while (index < argc)
/*
** At this point both the input and output files have been parsed
** now the switches are parsed and the ISL routines are called. Since this
** is only example program it should be noted that no code insures the order 
** of operation of routines. For example, unexpected results may happen if 
** one switch is set before another (i.e. dithering before sharpening).
*/
    {
    switch (GetSwitchValue(argv[index++]))
        {

    case BITMAP:
	{
	/*
	** First open a file for output of bitmap
	*/
	file_ptr = fopen(argv[index++], "w");
	/*
	** First calloc a buffer to receive the bitmap data
	*/
	data_buffer = (char *)calloc(BUFFER_SIZE, sizeof(char));

	fid = ImgExportBitmap(
                 fid,		            /* FID, ISL frame ID	    */
                 roi,			    /* ROI, ISL roi ID		    */
                 data_buffer,		    /* BUFPTR, I/O buffer address   */
                 BUFFER_SIZE,	            /* BUFLEN, I/O buffer size	    */
                 0,			    /* BYTCNT, return byte count    */
                 0,			    /* FLAGS, processing flags	    */
                 0,	                    /* ACTION, user action routine  */
                 0);		            /* USRPRM, action routine param */

	fwrite(data_buffer, sizeof(char), BUFFER_SIZE, file_ptr);
	break;
	}
    case COMBINE:
	{
	unsigned long rule;
	/*
	** Open second file for import (to be combined with first)
	*/
	input_ctx = ImgOpenDDIFFile(
                       ImgK_ModeImport,     /* MODE, Import or Export       */
                       strlen(argv[index]), /* INFILELEN, Input file length */
                       argv[index],         /* INFILEBUF, Input file ptr    */
                       0,		    /* RETFILELEN, Return file len  */
                       0,		    /* RETFILEBUF, Return file ptr  */
                       0);		    /* FLAGS,     Processing flags  */

	fid2 = ImgImportDDIFFrame(
                  input_ctx,	            /* CTX,DDIF stream context      */
                  0,		            /* BUFPTR, Pointer to data      */
                  0,		            /* BUFLEN, Size of buffer       */
                  0,		            /* FLAGS,  Processing flags     */
                  0,		            /* ACTION, User routine         */
                  0);		            /* USRPRM, Action parameter     */

	ImgCloseDDIFFile(
	   input_ctx,	                    /* CTX,   DDIF stream context   */
           0);		                    /* FLAGS, Processing flags      */
 
	index++;
	rule = atoi(argv[index++]);

	fid = ImgCombine(
                 fid,	/* DSTFID, Destination frame for combine operation  */
                 dstroi,/* DSTROI, Destination roi identifier		    */
                 fid2,	/* SRCFID, Source frame for combine operation	    */
                 roi,	/* SRCROI, Source roi identifier		    */
                 0,	/* MASK,   8 x 8 bit pattern ANDed with source data */
                 rule);	/* RULE,   Combination rule (in ImgDef.h)	    */

	roi = 0;
	dstroi = 0;
	break;
	}
    case COMPRESS:
	{
	/*
	** Set flag and compression scheme
	*/
	test_mask |= FLAG_COMPRESS;

	if (strcmp(argv[index],"g31d") == 0)
	    compression_scheme = ImgK_G31dCompression;
        else if 
           (strcmp(argv[index],"g32d") == 0)
	    compression_scheme = ImgK_G32dCompression;
	else if
           (strcmp(argv[index],"g42d") == 0)
	    compression_scheme = ImgK_G42dCompression;

	index++;
	break;
	}
    case CONVERT:
	{
	printf("%s routine not implemented\n", argv[index - 1]);
	break;
	}
    case CONVOLVE:
	{
        struct A2D_DESC *kernel = (struct A2D_DESC *)
                              _ImgCalloc(sizeof(struct A2D_DESC),1);
        float *data;

        kernel->A2D_W_LENGTH  = sizeof(float);
        kernel->A2D_B_DTYPE   = DSC_K_DTYPE_F;
        kernel->A2D_B_CLASS   = DSC_K_CLASS_A;
        kernel->A2D_B_SCALE   = 0;
        kernel->A2D_B_DIGITS  = 0;
        kernel->A2D_B_DIMCT   = 2;
        kernel->A2D_L_ARSIZE  = sizeof(float) * 9;
        kernel->A2D_A_POINTER = (unsigned char *)
                            _ImgCalloc(kernel->A2D_L_ARSIZE,1);
        kernel->A2D_L_M1      = 3;
        kernel->A2D_L_M2      = 3;
        kernel->A2D_L_L1      = -1;
        kernel->A2D_L_U1      =  1;
        kernel->A2D_L_L2      = -1;
        kernel->A2D_L_U2      =  1;
        kernel->A2D_A_A0      = (unsigned char *) E_A2D(kernel,0,0);
 
        data = (float *) kernel->A2D_A_POINTER;

        *data++ = atof(argv[index++]); /* [-1, -1] */
        *data++ = atof(argv[index++]); /* [ 0, -1] */
        *data++ = atof(argv[index++]); /* [ 1, -1] */
        *data++ = atof(argv[index++]); /* [-1,  0] */
        *data++ = atof(argv[index++]); /* [ 0,  0] */
        *data++ = atof(argv[index++]); /* [ 0,  1] */
        *data++ = atof(argv[index++]); /* [ 1, -1] */
        *data++ = atof(argv[index++]); /* [ 1,  0] */
        *data++ = atof(argv[index++]); /* [ 1,  1] */
        new_fid = ImgConvolve(
                     fid,                      /* FID, ISL frame identifier  */
                     kernel,                   /* FILTER to use              */
                     roi,                      /* ROI, ROI identifier        */
                     0);                       /* PRMLST, Address of list    */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	break;
	}
    case COPY:
	{

	new_fid = ImgCopy(
                     fid,                    /* FID, Image frame identifier */
                     roi,	             /* ROI, ROI identifier         */
                     0);                     /* ITMLST, Attributes          */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	break;
	}
    case DITHER:
	{
	long dither_type;
        /*
        ** Using getlst, obtain current image attributes
        */
        ImgGetFrameAttributes(
           fid,			                /* FID, ISL frame ID	    */  
           getlst);			        /* ITMLST, Frame attributes */
        /*
        ** Create itemlists to supply to routine
        */
        greyscale_itmlst[0].PutL_Code    = Img_ImgBitsPerComp;
        greyscale_itmlst[0].PutL_Length  = sizeof(new_img_bits_per_comp[0]);
        greyscale_itmlst[0].PutA_Buffer  = (char *)&new_img_bits_per_comp[0];
        greyscale_itmlst[0].PutL_Index   = 0;
  
        greyscale_itmlst[1].PutL_Code    = Img_PixelStride;
        greyscale_itmlst[1].PutL_Length  = sizeof(new_pixel_stride);
        greyscale_itmlst[1].PutA_Buffer  = (char *)&new_pixel_stride;
        greyscale_itmlst[1].PutL_Index   = 0;
  
        greyscale_itmlst[2].PutL_Code    = Img_ScanlineStride;
        greyscale_itmlst[2].PutL_Length  = sizeof(new_scanline_stride);
        greyscale_itmlst[2].PutA_Buffer  = (char *)&new_scanline_stride;
        greyscale_itmlst[2].PutL_Index   = 0;

        greyscale_itmlst[3].PutL_Code    = Img_BitsPerPixel;
        greyscale_itmlst[3].PutL_Length  = sizeof(new_bits_per_pixel);
        greyscale_itmlst[3].PutA_Buffer  = (char *)&new_bits_per_pixel;
        greyscale_itmlst[3].PutL_Index   = 0;
  
        greyscale_itmlst[4].PutL_Code    = 0;                    /* Terminate itemlist */

        rgb_itmlst[0].PutL_Code    = Img_ImgBitsPerComp;
        rgb_itmlst[0].PutL_Length  = sizeof(new_img_bits_per_comp[0]);
        rgb_itmlst[0].PutA_Buffer  = (char *)&new_img_bits_per_comp[0];
        rgb_itmlst[0].PutL_Index   = 0;
  
        rgb_itmlst[1].PutL_Code    = Img_ImgBitsPerComp;
        rgb_itmlst[1].PutL_Length  = sizeof(new_img_bits_per_comp[1]);
        rgb_itmlst[1].PutA_Buffer  = (char *)&new_img_bits_per_comp[1];
        rgb_itmlst[1].PutL_Index   = 1;
  
        rgb_itmlst[2].PutL_Code    = Img_ImgBitsPerComp;
        rgb_itmlst[2].PutL_Length  = sizeof(new_img_bits_per_comp[2]);
        rgb_itmlst[2].PutA_Buffer  = (char *)&new_img_bits_per_comp[2];
        rgb_itmlst[2].PutL_Index   = 2;
  
        rgb_itmlst[3].PutL_Code    = Img_PixelStride;
        rgb_itmlst[3].PutL_Length  = sizeof(new_pixel_stride);
        rgb_itmlst[3].PutA_Buffer  = (char *)&new_pixel_stride;
        rgb_itmlst[3].PutL_Index   = 0;
  
        rgb_itmlst[4].PutL_Code    = Img_ScanlineStride;
        rgb_itmlst[4].PutL_Length  = sizeof(new_scanline_stride);
        rgb_itmlst[4].PutA_Buffer  = (char *)&new_scanline_stride;
        rgb_itmlst[4].PutL_Index   = 0;

        rgb_itmlst[5].PutL_Code    = Img_BitsPerPixel;
        rgb_itmlst[5].PutL_Length  = sizeof(new_bits_per_pixel);
        rgb_itmlst[5].PutA_Buffer  = (char *)&new_bits_per_pixel;
        rgb_itmlst[5].PutL_Index   = 0;
  
        rgb_itmlst[6].PutL_Code    = 0;               /* Terminate itemlist */

	if (strcmp(argv[index],"bluenoise") == 0)
	    dither_type = ImgK_DitherBluenoise;
        else if 
           (strcmp(argv[index],"clustered") == 0)
	    dither_type = ImgK_DitherClustered;
	else if
           (strcmp(argv[index],"dispersed") == 0)
	    dither_type = ImgK_DitherDispersed;

        switch (spect_type)
	{
	case ImgK_ClassBitonal:
            printf("Image is bitonal, dithering not possible\n");
            break;
	case ImgK_ClassGreyscale:
	    /*
	    ** Prompt for dithered size
	    */
            printf("OLD bits_per_pixel =  %d\n",bits_per_pixel);
            printf("NEW bits_per_pixel => ",new_bits_per_pixel);
            scanf("%d", &new_bits_per_pixel);
            /*
            ** Calculate new attribute values
            */
	    new_img_bits_per_comp[0] = new_bits_per_pixel;
            new_pixel_stride = new_bits_per_pixel;
            new_scanline_stride = pixels_per_line * new_pixel_stride;
	    /*
	    ** Point to proper itemlist before calling dither.
	    */
	    dither_itmlst = (struct PUT_ITMLST *)greyscale_itmlst;
	    break;
	case ImgK_ClassMultispect:
            new_pixel_stride = 0;
	    for (i = 0;  i < number_of_comp;  i++)
               {
               /*
               ** Get old bits_per_comp and prompt for new values 
               */
               fid = ImgGet(
                        fid,                              /* FID, ISL id    */
                        Img_ImgBitsPerComp,               /* ITEMCODE, code */
                        &img_bits_per_comp[i],            /* BUFPTR, buffer */
                        sizeof(img_bits_per_comp[i]),     /* BUFLEN, size   */
                        0,                                /* RETLEN, size   */
                        i);                               /* INDEX, attrib. */

               printf("OLD img_bits_per_comp[%d] =  %d\n", 
                      i, img_bits_per_comp[i]);
               printf("NEW img_bits_per_comp[%d] => ",i);
               scanf("%d", &new_img_bits_per_comp[i]);
               new_pixel_stride += new_img_bits_per_comp[i];
               }/* End for */
	    /*
	    ** Calculate new attribute values
	    */
	    new_bits_per_pixel = new_pixel_stride;
	    new_scanline_stride = pixels_per_line * new_pixel_stride;
	    /*
	    ** Point to proper itemlist before calling dither
	    */
	    dither_itmlst = (struct PUT_ITMLST *)rgb_itmlst;
	    break;
	}/* End spect_type switch */

        new_fid = ImgDither(
                     fid,             /* FID, ISL frame identifier          */
                     dither_itmlst,   /* ITMLST, Itemlist to apply          */
                     dither_type,     /* ALGORITHM, Function code           */
                     0,	              /* THRESHOLD_SPEC, Matrix value       */
                     0,               /* FLAGS, See ImgDef.h                */
                     roi,             /* ROI, ROI identifier                */
                     0);              /* LEVELS, = or < bits per pixel      */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	/*
	** Increment index to point to next routine
	*/
	index++;
	break;
	}
    case DSTROI:
	{
	/*
        ** Structure ROI_RECT is the user defined structure for a rectangular
        ** region of interest. It is found in ImgDef.h
        */
        struct ROI_RECT *dstroi_rect_ptr;
        int dstroi_buflen;

        dstroi_rect_ptr = (struct ROI_RECT *)calloc(1,sizeof(struct ROI_RECT));
        dstroi_buflen = sizeof(struct ROI_RECT);

        dstroi_rect_ptr->RoiL_RectUlx    = atoi(argv[index++]);
        dstroi_rect_ptr->RoiL_RectUly    = atoi(argv[index++]);
        dstroi_rect_ptr->RoiL_RectPxls   = atoi(argv[index++]);
        dstroi_rect_ptr->RoiL_RectScnlns = atoi(argv[index++]);

        roi = ImgCreateRoi(
                 ImgK_RoitypeRect,
                 dstroi_rect_ptr,
                 dstroi_buflen);

        save_dstroi = dstroi;
	break;
	}
    case ERASE:
	{
	fid = ImgErase(
                 fid,                         /* FID, ISL frame identifier  */
                 Img_UserLabel,               /* ITEMCODE, See ImgDef.h     */
                 0);                          /* INDEX, Used in array       */

	new_fid = fid;
	break;
	}
    case FILTER:
	{
	long filter_type ;
	if (strcmp(argv[index],"ImgK_FilterEdge") == 0)
	    filter_type = ImgK_FilterEdge;
        else if 
	  (strcmp(argv[index],"ImgK_FilterMedian") == 0)
	    filter_type = ImgK_FilterMedian;
	else if
	   (strcmp(argv[index],"ImgK_FilterPrewitt") == 0)
	    filter_type = ImgK_FilterPrewitt;
	else if
	   (strcmp(argv[index],"ImgK_FilterLightSharpen") == 0)
	    filter_type = ImgK_FilterLightSharpen;
	else if
	   (strcmp(argv[index],"ImgK_FilterHeavySharpen") == 0)
	    filter_type = ImgK_FilterHeavySharpen;
	else if
	   (strcmp(argv[index],"ImgK_FilterLaplacian") == 0)
	    filter_type = ImgK_FilterLaplacian;
	else if
	   (strcmp(argv[index],"ImgK_FilterLightSmooth") == 0)
	    filter_type = ImgK_FilterLightSmooth;
	else if
	   (strcmp(argv[index],"ImgK_FilterHeavySmooth") == 0)
	    filter_type = ImgK_FilterHeavySmooth;
	else if
	   (strcmp(argv[index],"ImgK_FilterSobel") == 0)
	    filter_type = ImgK_FilterSobel;
	else if
	   (strcmp(argv[index],"ImgK_FilterClear") == 0)
	    filter_type = ImgK_FilterClear;
	index++;
        new_fid = ImgFilter(
                     fid,                      /* FID, ISL frame identifier  */
                     filter_type,              /* FILTER_TYPE, code          */
                     roi,                      /* ROI, ROI identifier        */
                     0);                       /* PRMLST, Address of list    */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	break;
	}
    case FLIP:
	{
	long flip_direction;

	if (strcmp(argv[index],"horizontal") == 0)
	    flip_direction = ImgM_FlipHorizontal;
        else if (strcmp(argv[index],"vertical") == 0)
	    flip_direction = ImgM_FlipVertical;
	else if (strcmp(argv[index],"both") == 0)
	    flip_direction = (ImgM_FlipHorizontal | ImgM_FlipVertical);

        new_fid = ImgFlip(
                     fid,                      /* FID, ISL frame identifier */
                     roi,                      /* ROI, ROI identifier       */
                     flip_direction);          /* FLAGS, See ImgDef.h       */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	/*
	** Increment index to point to next routine
	*/
	index++;
	break;
	}
    case HISTOGRAM:
	{
	unsigned long comp_index = atoi(argv[index++]);
	/*
	** Create a histogram structure
	*/
	hist_id = ImgCreateHistogram(
                     fid,		            /* FID, ISL frame ID    */
                     roi,			    /* ROI, ROI identifier  */
                     ImgK_HtypeExplicitIndex,       /* TYPE, Histogram type */
                     0,			            /* FLAGS, See ImgDef.h  */
                     comp_index);                   /* COMP_INDEX,          */

	/*
	** Print the histogram values
	*/
	PrintSparseHistogram(hist_id);

	/*
	** Delete the histogram structure
	*/
	ImgDeleteHistogram(
	    hist_id);				 /* HIST_ID, Histogram object */

	roi = 0;
	break;
	}
    case PAGE:
	{
	test_mask |= FLAG_PAGEBREAK;
	printf("%s routine produced test_mask = %d\n", argv[index - 1], test_mask);
	break;
	}
    case POLARITY:
	{
        /*
        ** Use ImgGet and ImgSet to obtain and set single attributes.
        */
	fid = ImgGet(
                 fid,                      /* FID, ISL frame identifier     */
                 Img_BrtPolarity,          /* ITEMCODE, See ImgDef.h        */
                 (char *)&brt_polarity,    /* BUFPTR, Buffer to store code  */
                 sizeof(brt_polarity),     /* BUFLEN, Size of buffer        */
                 0,                        /* RETLEN, Length of item        */
                 0);                       /* INDEX, Used in array of attr. */

	/*
	** Toggle the ImgBrtPolarity attribute
	*/
        if (brt_polarity == ImgK_ZeroMinIntensity)
	    brt_polarity =  ImgK_ZeroMaxIntensity;
	else
	    brt_polarity =  ImgK_ZeroMinIntensity;
	/*
	** Set the new attribute
	*/
	new_fid = ImgSet(
                     fid,                     /* FID, ISL frame identifier  */
                     Img_BrtPolarity,         /* ITEMCODE, See ImgDef.h     */
                     (char *)&brt_polarity,   /* BUFPTR, Buffer with item   */
                     sizeof(brt_polarity),    /* BUFLEN, Sizeof buffer      */
                     0);                      /* INDEX, Used in array       */

        fid = new_fid;
	break;
	}    
    case PS:
	{
	/*
	** First open a file for output of postscript
	*/
	file_ptr = fopen(argv[index++], "w");
	/*
	** Next calloc a buffer to receive the postscript data
	*/
	data_buffer = (char *)calloc(BUFFER_SIZE, sizeof(char));

	fid = ImgExportPS(
                 fid,		            /* FID, ISL frame ID	    */
                 roi,			    /* ROI, ISL roi ID		    */
                 data_buffer,		    /* BUFPTR, I/O buffer address   */
                 BUFFER_SIZE,	            /* BUFLEN, I/O buffer size	    */
                 0,			    /* BYTCNT, return byte count    */
                 0,			    /* FLAGS, processing flags	    */
                 0,	                    /* ACTION, user action routine  */
                 0);		            /* USRPRM, action routine param */

	fwrite(data_buffer, sizeof(char), BUFFER_SIZE, file_ptr);
	break;
	}
    case REMAP:
	{
	/*
	**	Using itemlist, obtain image attributes
	*/
	ImgGetFrameAttributes(
	   fid,	                                /* FID, ISL frame ID        */
	   getlst);                             /* ITMLST, Frame attributes */
	for (i = 0;  i < number_of_comp;  i++)
             {
             /*
             ** Get image attribute img_bits_per_comp 
             */
             fid = ImgGet(
                      fid,                                /* FID, ISL id    */
                      Img_ImgBitsPerComp,                 /* ITEMCODE, code */
                      &img_bits_per_comp[i],              /* BUFPTR, buffer */
                      sizeof(img_bits_per_comp[i]),       /* BUFLEN, size   */
                      0,                                  /* RETLEN, size   */
                      i);                                 /* INDEX, attrib. */

	    }/* End for */

	/*
	** Test for specific attributes that indicate that frame is RGB
        */
        if (spect_type           != ImgK_ClassMultispect ||
            spectral_mapping     != ImgK_RGBMap ||
            comp_space_org       != ImgK_PixelSequential ||
            number_of_comp       != 3)
	    printf("Image attributes not RGB , unable to remap\n");
	else
	    {
            /*
            ** Create itemlist to supply to routine
            */
            remap_itmlst[0].PutL_Code    = Img_SpectralMapping;
            remap_itmlst[0].PutL_Length  = sizeof(new_spectral_mapping);
            remap_itmlst[0].PutA_Buffer  = (char *)&new_spectral_mapping;
            remap_itmlst[0].PutL_Index   = 0;

            remap_itmlst[1].PutL_Code    = Img_NumberOfComp;
            remap_itmlst[1].PutL_Length  = sizeof(new_number_of_comp);
            remap_itmlst[1].PutA_Buffer  = (char *)&new_number_of_comp;
            remap_itmlst[1].PutL_Index   = 0;

            remap_itmlst[2].PutL_Code    = Img_ImgBitsPerComp;
            remap_itmlst[2].PutL_Length  = sizeof(new_img_bits_per_comp[0]);
            remap_itmlst[2].PutA_Buffer  = (char *)&new_img_bits_per_comp[0];
            remap_itmlst[2].PutL_Index   = 0;

            /*
            ** NOTE - The remaining indexed items must be set to CDAEMPTY
            **        this is accomplished by setting the length and buffer
            **        to zero. Also it is important to note that the item
            **        must be emptied from the upper index, to avoid index out
            **        of range type errors (i.e. index 2 becomes index 1).
            */

            remap_itmlst[3].PutL_Code    = Img_ImgBitsPerComp;
            remap_itmlst[3].PutL_Length  = 0;
            remap_itmlst[3].PutA_Buffer  = 0;
            remap_itmlst[3].PutL_Index   = 2;

            remap_itmlst[4].PutL_Code    = Img_ImgBitsPerComp;
            remap_itmlst[4].PutL_Length  = 0;
            remap_itmlst[4].PutA_Buffer  = 0;
            remap_itmlst[4].PutL_Index   = 1;

            remap_itmlst[5].PutL_Code    = Img_PixelsPerLine;
            remap_itmlst[5].PutL_Length  = sizeof(new_pixels_per_line);
            remap_itmlst[5].PutA_Buffer  = (char *)&new_pixels_per_line;
            remap_itmlst[5].PutL_Index   = 0;

            remap_itmlst[6].PutL_Code    = Img_PixelStride;
            remap_itmlst[6].PutL_Length  = sizeof(new_pixel_stride);
            remap_itmlst[6].PutA_Buffer  = (char *)&new_pixel_stride;
            remap_itmlst[6].PutL_Index   = 0;

            remap_itmlst[7].PutL_Code    = Img_ScanlineStride;
            remap_itmlst[7].PutL_Length  = sizeof(new_scanline_stride);
            remap_itmlst[7].PutA_Buffer  = (char *)&new_scanline_stride;
            remap_itmlst[7].PutL_Index   = 0;

            remap_itmlst[8].PutL_Code    = Img_BitsPerPixel;
            remap_itmlst[8].PutL_Length  = sizeof(new_bits_per_pixel);
            remap_itmlst[8].PutA_Buffer  = (char *)&new_bits_per_pixel;
            remap_itmlst[8].PutL_Index   = 0;

            remap_itmlst[9].PutL_Code    = 0;        /* Itemlist terminator */

	    /*
            ** Set greyscale attributes 
            */
            new_brt_polarity = ImgK_ZeroMinIntensity;               
            new_spectral_mapping = ImgK_MonochromeMap;
            new_number_of_comp = 1;
            new_img_bits_per_comp[0] = 8;
            new_pixels_per_line = pixels_per_line;
            new_pixel_stride = 8;
            new_scanline_stride = pixels_per_line * new_pixel_stride;
            new_bits_per_pixel = 8;
	    }/* End if-else greyscale remap */
        new_fid = ImgPixelRemap(
                     fid,                              /* FID, ISL frame ID */
                     0,             /* LUT, Address of an lookup descriptor */
                     remap_itmlst,      /* ITMLST, Itemlist to apply to fid */
                     0,                    /* FLAGS, See ImgDef.h for flags */
                     RGBtoGreyscale,    /* USER_REMAP, User written routine */
                     img_bits_per_comp,           /* USRPRM, User parameter */
                     roi);                           /* ROI, ROI identifier */
        
	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	break;
	}
    case REQUANTIZE:
	{
	printf("%s routine not implemented\n", argv[index - 1]);
	new_fid = fid;
	break;
	}
    case ROI:
	{
	/*
        ** Structure ROI_RECT is the user defined structure for a rectangular
        ** region of interest. It is found in ImgDef.h
        */
        struct ROI_RECT *roi_rect_ptr;
        int roi_buflen;

        roi_rect_ptr = (struct ROI_RECT *)calloc(1,sizeof(struct ROI_RECT));
        roi_buflen = sizeof(struct ROI_RECT);

        roi_rect_ptr->RoiL_RectUlx    = atoi(argv[index++]);
        roi_rect_ptr->RoiL_RectUly    = atoi(argv[index++]);
        roi_rect_ptr->RoiL_RectPxls   = atoi(argv[index++]);
        roi_rect_ptr->RoiL_RectScnlns = atoi(argv[index++]);

        roi = ImgCreateRoi(
                 ImgK_RoitypeRect,
                 roi_rect_ptr,
                 roi_buflen);

        save_roi = roi;
	break;
	}
    case ROTATE:
	{
	float degrees_rotation = atof(argv[index++]);

	new_fid = ImgRotate(
                     fid,			/* FID,	ISL frame ID        */
                     (char *)&degrees_rotation, /* ANGLE, Floating angle    */
                     roi,	  		/* ROI, ROI identifier	    */
                     0,		        	/* FLAGS, See ImgDef.h	    */
                     0);		 	/* ALIGNMENT, See ImgDef.h  */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	break;
	}
    case SCALE:
	{
	float x_scale = atof(argv[index++]);
	float y_scale = atof(argv[index++]);

        new_fid = ImgScale(
                     fid,              /* FID, ISL frame ID                 */
                     (char *)&x_scale, /* SCALE_X, Floating x scale factor  */
                     (char *)&y_scale, /* SCALE_Y, Floating y scale factor  */
                     roi,              /* ROI, ROI identifier               */
                     0,                /* FLAGS, See ImgDef.h for flags     */
                     0);               /* ALIGNMENT, See IMGDEF             */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	break;
	}
    case SIXEL:
	{
	/*
	** First open a file for output of sixel
	*/
	file_ptr = fopen(argv[index++], "w");
	/*
	** Next calloc a buffer to receive the sixel data
	*/
	data_buffer = (char *)calloc(BUFFER_SIZE, sizeof(char));

	fid = ImgExportSixels(
                 fid,		            /* FID, ISL frame ID	    */
                 roi,			    /* ROI, ISL roi ID		    */
                 data_buffer,		    /* BUFPTR, I/O buffer address   */
                 BUFFER_SIZE,	            /* BUFLEN, I/O buffer size	    */
                 0,			    /* BYTCNT, return byte count    */
                 0,			    /* FLAGS, processing flags	    */
                 0,	                    /* ACTION, user action routine  */
                 0);		            /* USRPRM, action routine param */

	fwrite(data_buffer, sizeof(char), BUFFER_SIZE, file_ptr);
	break;
	}
    case TONESCALE:
	{
	float punch_1 = atof(argv[index++]);
	float punch_2 = atof(argv[index++]);

        new_fid = ImgTonescaleAdjust(
                     fid,              /* FID, ISL frame ID                 */
                     (char *)&punch_1, /* PUNCH_1, Floating lower bound     */
                     (char *)&punch_2, /* PUNCH_2, Floating upper bound     */
                     roi,              /* ROI, ROI identifier               */
                     0,                /* FLAGS, See ImgDef.h for flags     */
                     0);               /* COMPONENT INDEX, for multi-spect  */

	ImgDeleteFrame(fid);
	fid = new_fid;
	roi = 0;
	break;
	}
    default:
	printf("%s routine not found in parse table\n", argv[index - 1]);
	break;
	}                                        /* switch (GetSwitchValue) */
    }                                            /* while (index < argc)    */
/*
** If compression was requested, compress the data first
*/
if ((test_mask & FLAG_COMPRESS) != 0)
    fid = ImgCompress(
             fid,                        /* FID, ISL frame identifier       */
             compression_scheme,         /* SCHEME, Compression scheme      */
             0);                         /* CPB, Compression parameter      */

/*
** Export the DDIF stream
*/
output_ctx = ImgOpenDDIFFile(
                ImgK_ModeExport,	  /* MODE, Import or Export         */
		strlen(argv[2]),          /* INFILELEN, Input file length   */
		argv[2],	          /* INFILEBUF, Input file ptr      */
		0,			  /* RETFILELEN, Return file length */
		0,			  /* RETFILEBUF, Return file ptr    */
                0);			  /* FLAGS,     Processing flags    */

/*
** If flag is set for pagebreak then insert one
*/
if ((test_mask & FLAG_PAGEBREAK) != 0)

    ImgExportDDIFPageBreak(
       output_ctx,	         	  /* CTX, DDIF stream ID	    */
       0);			          /* FLAGS, Processing flags	    */

fid = ImgExportDDIFFrame(
         fid,                             /* FID,    ISL frame ID           */
         roi,	                          /* ROI,    ISL roi ID             */
         output_ctx,	                  /* CTX,    DDIF stream ID         */
         0,		                  /* BUFPTR, Image buffer           */
         0,		                  /* BUFLEN, Size of buffer         */
         0,		                  /* FLAGS,  Processing flags       */
         0,		                  /* ACTION, User routine           */
         0);		                  /* USRPRM, User parameter         */

ImgCloseDDIFFile(
   output_ctx,		                  /* CTX,   DDIF stream ID          */
   0);			                  /* FLAGS, Processing flags        */

/*
** If a ROI existed, delete it
*/
if(save_roi != 0)

   ImgDeleteRoi(save_roi);

/*
** If a DSTROI existed, delete it
*/
if(save_dstroi != 0)

   ImgDeleteRoi(save_dstroi);

}/* End main */

/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      GetSwitchValue
**
**  FORMAL PARAMETERS:
**
**      pointer to argument string
**
**  IMPLICIT INPUTS:
**
**      parse_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      index of a string entry in parse_table[]
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
int GetSwitchValue(arg_ptr)
char *arg_ptr;
{
int value;
int i,j,k;
for (i = 0, value = 1 ; parse_table[i] != 0 ; i++, value++)
    {
    if (strcmp(arg_ptr,parse_table[i]) == 0)
	return (value);
    };
return (0);
}

/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      RGBtoGrey
**
**  FORMAL PARAMETERS:
**
**      integer pixel value
**	array of bits_per_component
**
**  IMPLICIT INPUTS:
**
**      parse_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      index of a string entry in parse_table[]
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
unsigned long RGBtoGreyscale(pixel,x,y,component_array)
unsigned long pixel;
int x,y;
int component_array[3];
{
unsigned long  y_long;
unsigned long  red_mask, green_mask, blue_mask;
float y_float;
float red_float, red_levels, green_float, green_levels, blue_float, blue_levels;
/*
** Build a mask for each component 
*/
red_mask   = (1 << component_array[RED]) - 1;
green_mask = (1 << component_array[GREEN]) - 1;
blue_mask  = (1 << component_array[BLUE]) - 1;
/*
** Cast as float to perform floating normalization
*/
red_levels   = (float)red_mask;
green_levels = (float)green_mask;
blue_levels  = (float)blue_mask;
/*
** Mask and shift to isolate each component
*/
red_float   = (float)(pixel & red_mask);
pixel >>= component_array[RED];
green_float = (float)(pixel & green_mask);
pixel >>= component_array[GREEN];
blue_float  = (float)(pixel & blue_mask);
/*
** Calculate greyscale value as float with normalized values
*/
y_float =   .299 * (red_float/red_levels) + 
	    .587 * (green_float/green_levels) +
	    .114 * (blue_float/blue_levels);

y_long = (long)(y_float * 255);
return y_long;
}/* End RGBtoGreyscale */

/************************************************************************
**
**  PrintSparseHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine takes a histogram context block (HCB) and prints the pixel
**	value and the corresponding count. Only explicitly indexed histograms
**	are supported.
**
**  FORMAL PARAMETERS:
**
**      HCB - a pointer to a histogram context block data structure (ImgDef.h)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
void PrintSparseHistogram(hcb)
struct HCB *hcb;
{
int upper_bound;
int i,j,k;
struct	EXPLICIT_TABLE_ENTRY
	{
	unsigned long pixel_value;
	unsigned long frequency;
	} *entry;
/*
** Initialize variables
*/
entry = (struct EXPLICIT_TABLE_ENTRY *)hcb->HcbA_TablePointer;
upper_bound = hcb->HcbL_EntryCount;
if (hcb->HcbL_Flags.HcbV_SortByFreq == 0)
    for (i = 0;  i < upper_bound;  i++, entry++)
        {
        printf((i%4)?"pix=%-5x freq=%-5d":"pix=%-5x freq=%-5d\n",
    	   entry->pixel_value,entry->frequency);
        }
else
    for (i = 0;  i < upper_bound;  i++, entry++)
        {
        printf((i%3)?"freq=%-5d pix=%-5x":"freq=%-5d pix=%-5x\n",
    	   entry->frequency, entry->pixel_value);
        }
}/* End PrintSparseHistogram */
