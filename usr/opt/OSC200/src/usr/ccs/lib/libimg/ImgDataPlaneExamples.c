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
**      Image Service Library (ISL)
**
**  ABSTRACT:
**
**      This program demonstrates the use of the Data plane utilities to
**      create a small (64 pixels) frame from an array.
**
**  ENVIRONMENT:
**
**      VAX/VMS
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
#include <string.h>
#include <img/ImgDef.h>

#define RAMP 1
#define STAR 2

#define DP_SIZE 32

static char output_filename[] = {"example_output.ddif"};

static char *parse_table[] = {"ramp","star",0};   /* Different patterns */

/*
**  Structures PUT_ITMLST exist in ImgDef.h
*/
struct PUT_ITMLST put_itmlst[7];                /* PUT_ITMLST is in ImgDef.h */

main(argc,argv)
int argc;
char *argv[];
{/* Main program */

unsigned long dp_addr, new_dp_addr, size;
int fid, new_fid, input_ctx, output_ctx;
int index, menu_choice;

/*
** Create small matrices to use as a test patterns
*/
static unsigned char bad_matrix[] =  {255,255,255,255,255,255,255,255,
                                      255,127,255,127,127,127,127,127,
                                      255, 63,255, 63, 63, 63, 63, 63,
                                      255, 31, 31, 31, 31, 31, 31, 31,
                                        1,255,255, 15, 15, 15, 15, 15,
                                        7,  7,  7,  7,  7,  7,  7,  7,
                                        3,  3,  3,  3,  3,  3,  3,  3,
                                        1,  1,  1,  1,  1,  1,  1,  1};

static unsigned char ramp[] =        {255,255,255,255,255,255,255,255,
                                      127,127,127,127,127,127,127,127,
                                       63, 63, 63, 63, 63, 63, 63, 63,
                                       31, 31, 31, 31, 31, 31, 31, 31,
                                       15, 15, 15, 15, 15, 15, 15, 15,
                                        7,  7,  7,  7,  7,  7,  7,  7,
                                        3,  3,  3,  3,  3,  3,  3,  3,
                                        1,  1,  1,  1,  1,  1,  1,  1};

static unsigned char star[] =        {  1,  3,  7, 15, 15,  7,  3,  1,
                                        3,  7, 15, 31, 31, 15,  7,  3,
                                        7, 15, 31, 63, 63, 31, 15,  7,
                                       15, 31, 63,127,255, 63, 31, 15,
                                       15, 31, 63,255,127, 63, 31, 15,
                                        7, 15, 31, 63, 63, 31, 15,  7,
                                        3,  7, 15, 31, 31, 15,  7,  3,
                                        1,  3,  7, 15, 15,  7,  3,  1};

unsigned char *matrix_ptr;
/*
** Itemlist variables
*/
int img_bits_per_comp;
int pixels_per_line;
int number_of_lines;
int pixel_stride;
int scanline_stride;
int bits_per_pixel;

/*
** Read command line switches
*/
if (argc == 1)
    {
    printf("No switches, demonstrating reallocate and free data plane\n");
    /*
    ** Allocate data planes to hold our test data
    */
    dp_addr = ImgAllocateDataPlane(
                  DP_SIZE,           /* SIZE, Size in bytes of the data plane */
                  0);                /* FILL, Initial value of each byte      */
    /*
    ** Now resize the data plane to the proper size
    */
    size = sizeof(bad_matrix);

    new_dp_addr = ImgReallocateDataPlane(
                      dp_addr,          /* OLD_DP_ADDR, Address of data plane */
                      size);            /* SIZE, New size of data plane       */

    dp_addr = new_dp_addr;
    /*
    ** Data plane will now be freed
    */
    ImgFreeDataPlane(
        dp_addr);                           /* DP_ADDR, Address of data plane */
    /*
    ** Leave without doing anything
    */              
    exit();
    }

/*
**  Initialize an itemlist used by Create Frame
*/
put_itmlst[0].PutL_Code    = Img_ImgBitsPerComp;
put_itmlst[0].PutL_Length  = sizeof(img_bits_per_comp);
put_itmlst[0].PutA_Buffer  = (char *)&img_bits_per_comp;
put_itmlst[0].PutL_Index   = 0;
 
put_itmlst[1].PutL_Code    = Img_PixelsPerLine;
put_itmlst[1].PutL_Length  = sizeof(pixels_per_line);
put_itmlst[1].PutA_Buffer  = (char *)&pixels_per_line;
put_itmlst[1].PutL_Index   = 0;

put_itmlst[2].PutL_Code    = Img_NumberOfLines;
put_itmlst[2].PutL_Length  = sizeof(number_of_lines);
put_itmlst[2].PutA_Buffer  = (char *)&number_of_lines;
put_itmlst[2].PutL_Index   = 0;

put_itmlst[3].PutL_Code    = Img_PixelStride;
put_itmlst[3].PutL_Length  = sizeof(pixel_stride);
put_itmlst[3].PutA_Buffer  = (char *)&pixel_stride;
put_itmlst[3].PutL_Index   = 0;
  
put_itmlst[4].PutL_Code    = Img_ScanlineStride;
put_itmlst[4].PutL_Length  = sizeof(scanline_stride);
put_itmlst[4].PutA_Buffer  = (char *)&scanline_stride;
put_itmlst[4].PutL_Index   = 0;

put_itmlst[5].PutL_Code    = Img_BitsPerPixel;
put_itmlst[5].PutL_Length  = sizeof(bits_per_pixel);
put_itmlst[5].PutA_Buffer  = (char *)&bits_per_pixel;
put_itmlst[5].PutL_Index   = 0;
 
put_itmlst[6].PutL_Code    = 0;                        /* Terminate itemlist */

/*
** Now parse through the command line to determine which synthetic pattern to
** create
*/
index = 1;
while (index < argc)
    {
    switch (GetSwitchValue(argv[index++]))
        {
    case RAMP:
	    {
            /*
            ** Load image attributes for grey ramp
            */
            img_bits_per_comp = 8;
            pixels_per_line = 8;
            number_of_lines = 8;
            pixel_stride = 8;
            scanline_stride = pixel_stride * pixels_per_line;
            bits_per_pixel = 8;             
            /*
            ** Set pointer at proper matrix and calculate size
            */
            matrix_ptr = ramp;
            size = sizeof(ramp);
            printf("Creating a %s pattern\n", argv[index - 1]);
            /*
            ** Create a grey_scale frame with the put_itmlst attributes
            */
            fid = ImgCreateFrame(
                      put_itmlst,             /* ITMLST, Address of item list */
                      ImgK_ClassGreyscale);   /* SPECTRAL_TYPE, Spectral type */
	    break;
            }
    case STAR:
	    {
            /*
            ** Load image attributes for grey star
            */
            img_bits_per_comp = 8;
            pixels_per_line = 8;
            number_of_lines = 8;
            pixel_stride = 8;
            scanline_stride = pixel_stride * pixels_per_line;
            bits_per_pixel = 8;             
            /*
            ** Set pointer at proper matrix and calculate size
            */
            matrix_ptr = star;
            size = sizeof(star);
            printf("Creating a %s pattern\n", argv[index - 1]);
            /*
            ** Create a grey_scale frame with the put_itmlst attributes
            */
            fid = ImgCreateFrame(
                      put_itmlst,             /* ITMLST, Address of item list */
                      ImgK_ClassGreyscale); /* SPECTRAL_TYPE, Spectral type */
	    break;
            }
    default:
	    {
            printf("%s pattern not found in parse table\n", argv[index - 1]);
	    exit();
            }
	}                                          /* switch (GetSwitchValue) */
    }                                              /* while (index < argc)    */

/*
** Allocate data planes to hold our test data
*/
dp_addr = ImgAllocateDataPlane(
              size,                  /* SIZE, Size in bytes of the data plane */
              0);                    /* FILL, Initial value of each byte      */

/*
** Copy image data to data plane
*/
memcpy(dp_addr,matrix_ptr,size);

/*
** Attach the data plane with the created frame
*/
new_fid = ImgStoreDataPlane(
              fid,                          /* FID, ISL frame identifier      */
              dp_addr);                     /* DP_ADDR, Address of data plane */

/*
** Export the DDIF stream
*/
output_ctx = ImgOpenDDIFFile(
                ImgK_ModeExport,	    /* MODE,      Import or Export    */
		strlen(output_filename),    /* INFILELEN, Input file length   */
		output_filename,	    /* INFILEBUF, Input file ptr      */
		0,			    /* RETFILELEN, Return file length */
		0,			    /* RETFILEBUF, Return file ptr    */
                0);			    /* FLAGS,     Processing flags    */

/*
** Export the frame
*/
ImgExportDDIFFrame(
   new_fid,		                    /* FID,    ISL frame ID           */
   0,			                    /* ROI,    ISL roi ID             */
   output_ctx,		                    /* CTX,    DDIF stream ID         */
   0,			                    /* BUFPTR, Image buffer           */
   0,			                    /* BUFLEN, Size of buffer         */
   0,			                    /* FLAGS,  Processing flags       */
   0,			                    /* ACTION, User routine           */
   0);			                    /* USRPRM, User parameter         */

ImgCloseDDIFFile(
   output_ctx,		                    /* CTX,   DDIF stream ID          */
   0);			                    /* FLAGS, Processing flags        */

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
GetSwitchValue(arg_ptr)
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
