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
**
**  FACILITY:
**
**      Image Service Library (ISL)
**
**  ABSTRACT:
**
**      This program reads the command line and strings together ISL routines,
**      using IDS to display the processed frame
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
**      October 16, 1990
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
** Include files
*/
#include <stdio.h>

#include <img/ImgDef.h>
#include <img/IdsImage.h>
#ifdef VMS
#include <decw$include/xatom.h>
#endif
#ifdef ultrix
#include <X11/Xatom.h>
#endif

/*
** Local functions
*/
static unsigned long    GetSwitchValue();
void                    PrintHistogram();
static Window           *DisplayImage();
/*
**  Equated Symbols
*/
#define CHARSIZE        (sizeof(char))
#define ELEMENT_COUNT   65536
#define MAX_WINDOWS     12
#define X_SIZ           240
#define Y_SIZ           250
#define X_POS           (x_position[window_idx])
#define Y_POS           (y_position[window_idx])

#define ADJUST_COMP_TONESCALE   1
#define COMPRESS_FRAME          2
#define CREATE_HISTOGRAM        3
#define CREATE_ROI_DEF          4
#define FLIP_FRAME              5
#define OPEN_FILE               6
#define REMAP_DATA_PLANE        7
#define ROTATE_FRAME            8
#define SCALE_FRAME             9

static char *output_filename;
static char *parse_table[] = 
   {"adjust_comp_tonescale","compress_frame","create_histogram",
    "create_roi_def","flip_frame","open_file","remap_data_plane",
    "rotate_frame","scale_frame",0};

static long x_position[MAX_WINDOWS];
static long y_position[MAX_WINDOWS];

/*
** X Structures and variables
*/
Display       *display;
Window        *window;

main(argc,argv)
int argc;
char *argv[];
{/* Main program */
unsigned char *data_plane[MAX_NUMBER_OF_COMPONENTS];         /* data planes */

unsigned long byte_count;                              /* bytes transferred */
unsigned long compression_type;                           /* compress type  */
unsigned long data_plane_size;                             /* size in bytes */
unsigned long fid;                                      /* frame identifier */
unsigned long hist_id;                              /* histogram identifier */
unsigned long index;                                         /* array index */
unsigned long image_data_class;                                /* data type */
unsigned long input_ctx;                         /* input context identifer */
unsigned long idx, idy, idz;                               /* loop variable */
unsigned long lut_def;                /* lookup table definition identifier */
unsigned char *lut_addr;                            /* lookup table address */
unsigned long number_of_comp;                       /* number of components */
unsigned long number_of_lines;                            /* scanline count */
unsigned long output_ctx;                       /* output context identifer */
unsigned char pix_value;                                     /* pixel value */
unsigned long pixels_per_line;                       /* pixels per scanline */
unsigned long retfid;                          /* returned frame identifier */
unsigned long roi_id;                      /* region of interest identifier */
unsigned long window_idx;                          /* window location index */
/*
**  Structure ITMLST exist in ImgDef.h
*/
struct ITMLST *itmlst, *itmlst_ptr;


/*
** Create coordinates for 12 possible display windows
*/
for (idy = 0, idz = 0; idy < 3; idy++)
    for (idx = 0; idx < 4; idx++)
        {
        x_position[idz] = idx * 250;
        y_position[idz] = idy * 300;
        idz++;
        }/* end x loop */

/*
** Initialize the ISL identifiers
*/
hist_id = 0;
lut_def = 0;
roi_id  = 0;

/*
** Open the display 
*/
if(!(display = XOpenDisplay(getenv("DECW$DISPLAY"))))
    {
    perror("Can't open display");
    exit(0);
    }


/*
** Allocate itemlist, for a 3 plane RGB frame of 256 pixels by 256 scanlines
*/
number_of_comp  = 3;                           /* for three plane RGB frame */
number_of_lines = 256;                                /* for 256 line frame */
pixels_per_line = 256;                          /* all the values in 8 bits */

itmlst = (struct ITMLST *)
    calloc((2 * number_of_comp) + 1, sizeof(struct ITMLST));

itmlst_ptr = itmlst;

for (idx = 0; idx < number_of_comp; idx++)
    {
    itmlst_ptr->ItmL_Code = Img_NumberOfLines;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&number_of_lines;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_PixelsPerLine;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&pixels_per_line;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;
    }/* end indexed ITMLST variables */

/*
** Now allocate a frame with no data plane (i.e. frame with just attributes)
*/
fid = ImgAllocateFrame(
         ImgK_ClassMultispect,                          /* image data class */
         itmlst,                                  /* itemlist of attributes */
         0,                                              /* srcfid template */
         ImgM_NoDataPlaneAlloc);                              /* flag value */

/*
** Allocate data planes for frame id (fid) defined above;
*/
data_plane_size = pixels_per_line * number_of_lines;
for (idx = 0; idx < number_of_comp; idx++)
    {
    data_plane[idx] = ImgAllocDataPlane(
                         data_plane_size,                  /* size in bytes */
                         0,                                        /* flags */
                         0);                                /* fill pattern */

    fid = ImgAttachDataPlane(
             fid,                                      /* frame identifier */
             data_plane[idx],                                /* data plane */
             idx);                                   /* index to attach to */

    }/* end plane loop */

/*
** Create ramps in previously allocated data_planes
*/
for (idx = 0; idx < ELEMENT_COUNT; idx++)
    {
    *(data_plane[0] + idx) = idx;
    *(data_plane[1] + idx) = ~idx;
    *(data_plane[2] + idx) = idx * 2;
    }

/*
** Display the frame
*/
window_idx = 0;
window = DisplayImage(display,fid,X_POS,Y_POS,X_SIZ,Y_SIZ,"Original ramp");
window_idx++;

/*
** Parse through the command line to determine which routine to demonstate
*/
index = 1;
while (index < argc)
    {
    /*
    ** Check number of windows displayed
    */
    if(window_idx >= MAX_WINDOWS)
      {
      perror("No more room for windows, Goodbye!");
      exit(1);
      }

    /*
    ** Get the compression type of the frame
    */
    fid = ImgGet(
             fid,
             Img_CompressionType,
             &compression_type,
             sizeof(compression_type),
             0,
             0);

    if (compression_type != ImgK_PcmCompression)
    /*
    ** Decompress the frame
    */
        fid = ImgDecompressFrame(
                 fid,                                   /* frame identifier */
                 ImgM_InPlace,       /* flag to deallocate the old frame id */
                 0);                                         /* comp_params */

    /*
    ** Get the number of components in the current fid, this is 
    ** useful in routines that take a component index as a parameter
    ** such as ImgAdjustCompTonescale and ImgCreateHistogram
    */
    fid = ImgGet(
             fid,
             Img_NumberOfComp,
             &number_of_comp,
             sizeof(number_of_comp),
             0,
             0);
        
    switch (GetSwitchValue(argv[index++]))
    {
    case ADJUST_COMP_TONESCALE:
        {
        /*
        ** Perform a tonescale adjustment on all components with
        ** contrast setting of:
        */
        float lower_limit        = .3;
        float upper_limit        = .7;        

        for (idx = 0; idx < number_of_comp; idx++)

             retfid = ImgAdjustCompTonescale(
                         fid,                           /* frame identifier */
                         idx,                            /* component index */
                         &lower_limit,           /* lower bound of function */
                         &upper_limit,           /* upper limit of function */
                         roi_id,           /* region of interest identifier */
                         0);                                       /* flags */

        /*
        ** Display the frame
        */
        window = DisplayImage(
        display,retfid,X_POS,Y_POS,X_SIZ,Y_SIZ,argv[index - 1]);
        window_idx++;

        /*
        ** Delete old frame and make source fid equal the resultant fid
        */
        ImgDeallocateFrame(fid);                   /* frame identifier */
        fid = retfid;

        }/* end adjust comp tonescale */
        break;
    case COMPRESS_FRAME:
        {
        /*
        ** Find out what class of image data exists
        */
        unsigned long scheme;

            fid = ImgGet(
                     fid,
                     Img_ImageDataClass,
                     &image_data_class,
                     sizeof(image_data_class),
                     0,
                     0);

        if( image_data_class != ImgK_ClassBitonal)
            scheme = ImgK_DctCompression;
        else
            scheme = ImgK_G42dCompression;

        fid = ImgCompressFrame(
                 fid,                                   /* frame identifier */
                 scheme,                           /* compression algorithm */
                 ImgM_InPlace,                           /* processing flag */
                 0);                                         /* comp params */
        }/* end compress frame */
        break;
    case CREATE_HISTOGRAM:
        {
        /*
        ** Create a sparse table histogram sorted by frequency
        */

        for (idx = 1; idx <= number_of_comp; idx++)
            {
            hist_id = ImgCreateHistogram(
                         fid,
                         roi_id,
                         ImgK_HtypeExplicitIndex,
                         ImgM_SortByFreq,
                         idx);

            printf("\n\nThe histogram for plane number %d is:\n",idx);
            PrintHistogram(hist_id);

            }/* end component loop */             
        }/* end create histogram */
        break;
    case CREATE_ROI_DEF:
        {
        /*
        ** Create ITMLST structure (in ImgDef.h) to pass in ROI coordinates
        */
        struct ITMLST roi_itmlst[2];    /* second element is terminator */
        struct ROI_RECT roi_rect;              /* structure in ImgDef.h */

        /*
        ** Get current dimensions of the frame
        */
        fid = ImgGet(
                 fid,
                 Img_PixelsPerLine,
                 &pixels_per_line,
                 sizeof(pixels_per_line),
                 0,
                 0);
        
        fid = ImgGet(
                 fid,
                 Img_NumberOfLines,
                 &number_of_lines,
                 sizeof(number_of_lines),
                 0,
                 0);
        
        /*
        ** Create roi_rect in center quarter of the frame
        */
        roi_rect.RoiL_RectUlx    = pixels_per_line >> 2;
        roi_rect.RoiL_RectUly    = number_of_lines >> 2;
        roi_rect.RoiL_RectPxls   = pixels_per_line >> 1;
        roi_rect.RoiL_RectScnlns = number_of_lines >> 1;

        /*
        ** Next setup the itemlist containing the pointer to the roi_rect
        */
        roi_itmlst[0].ItmL_Code   = Img_RoiRectangle;
        roi_itmlst[0].ItmL_Length = sizeof(struct ROI_RECT);
        roi_itmlst[0].ItmA_Buffer = (char *)&roi_rect;
        roi_itmlst[0].ItmA_Retlen = 0;
        roi_itmlst[0].ItmL_Index  = 0;

        /*
        ** Terminate item list
        */
        roi_itmlst[1].ItmL_Code   = 0;
        roi_itmlst[1].ItmL_Length = 0;
        roi_itmlst[1].ItmA_Buffer = 0;
        roi_itmlst[1].ItmA_Retlen = 0;
        roi_itmlst[1].ItmL_Index  = 0;

        roi_id = ImgCreateRoiDef(
                    roi_itmlst,            /* region of interest identifier */
                    0);                                            /* flags */

        }/* end create roi def */
        break;
    case FLIP_FRAME:
        {
        /*
        ** Flip the frame (or portion) about the y axis
        */
        unsigned long flags = ImgM_FlipHorizontal;

        retfid = ImgFlipFrame(
                    fid,                                /* frame identifier */
                    roi_id,                /* region of interest identifier */
                    flags);                                        /* flags */

        /*
        ** Display the frame
        */
        window = DisplayImage(
        display,retfid,X_POS,Y_POS,X_SIZ,Y_SIZ,argv[index - 1]);
        window_idx++;

        /*
        ** Delete old frame and make source fid equal the resultant fid
        */
        ImgDeallocateFrame(fid);                   /* frame identifier */
        fid = retfid;

        }/* end flip frame */
        break;
    case OPEN_FILE:
        {
        /*
        ** Open an external file 
        */

        input_ctx = ImgOpenFile(
                       ImgK_ModeImport,            /* mode,Import or Export */
                       ImgK_FtypeDDIF,              /* file_type, DDIF type */
                       strlen(argv[index]),            /* Input file length */
                       argv[index],                       /* Input file ptr */
                       0,                                /* Return file ptr */
                       0);                              /* Processing flags */

        fid = ImgImportFrame(
                 input_ctx,                               /* stream context */
                 0);                                    /* Processing flags */
  
        ImgCloseFile(
           input_ctx,                                /* DDIF stream context */
           0);                                          /* Processing flags */

        /*
        ** Display the frame
        */
        window = DisplayImage(
        display,fid,X_POS,Y_POS,X_SIZ,Y_SIZ,argv[index - 1]);
        window_idx++;

        index++;
        }/* end open_file */
        break;
    case REMAP_DATA_PLANE:
        {
        /*
        ** First create a lookup table which will be used as an argument 
            ** to the remap routine
        */
        unsigned long   count;
        unsigned long   lut_data_type = ImgK_DTypeBU;
        unsigned long   lut_entry_count = 256;
        unsigned long   lut_quant_levels = 256;
        unsigned char   *lut_invert;
        struct ITMLST   lut_itmlst[4];
        unsigned long   lut_size = 256;
        unsigned long   lut_entry;

        /*
        ** Fill in the itemlist
        */
        lut_itmlst[0].ItmL_Code = Img_LutEntryDataType;
        lut_itmlst[0].ItmL_Length = sizeof(long);
        lut_itmlst[0].ItmA_Buffer =  (char *)&lut_data_type;
        lut_itmlst[0].ItmA_Retlen = 0;
        lut_itmlst[0].ItmL_Index = 0;

        lut_itmlst[1].ItmL_Code = Img_LutQuantLevels;
        lut_itmlst[1].ItmL_Length = sizeof(long);
        lut_itmlst[1].ItmA_Buffer = (char *)&lut_quant_levels;
        lut_itmlst[1].ItmA_Retlen = 0;
        lut_itmlst[1].ItmL_Index = 0;

        lut_itmlst[2].ItmL_Code = Img_LutEntryCount;
        lut_itmlst[2].ItmL_Length = sizeof(long);
        lut_itmlst[2].ItmA_Buffer = (char *)&lut_entry_count;
        lut_itmlst[2].ItmA_Retlen = 0;
        lut_itmlst[2].ItmL_Index = 0;

        /*
        ** Terminate item list
        */
        lut_itmlst[3].ItmL_Code = 0;
        lut_itmlst[3].ItmL_Length = 0;
        lut_itmlst[3].ItmA_Buffer = 0;
        lut_itmlst[3].ItmA_Retlen = 0;
        lut_itmlst[3].ItmL_Index = 0;

        /*
        ** Now create the LUT definition which includes the lookup table
        ** address
        */
        lut_def = ImgCreateLutDef(
                     ImgK_UserLut,
                     lut_itmlst,
                     0,
                     0);

        
        /*
        ** Get the address of the lookup table without detaching anything
        */
        lut_addr = ImgDetachLut(
                      lut_def,
                      ImgM_GetLutAdrOnly,
                      0);

        /*
        ** fill in the lookup table which maps each pixel value to
        ** its value plus 16 capped at 255
        */
        for(idx = 0, lut_entry = 16; idx < 256; idx++, lut_entry++)
            *(lut_addr + idx) = lut_entry < 255 ? lut_entry : 255;

        for (idx = 0; idx < number_of_comp; idx++)
             fid = ImgRemapDataPlane(
                      fid,
                      idx,
                      lut_def,
                      roi_id,
                      ImgM_InPlace);        /* flag to overwrite source fid */

        /*
        ** Display the frame
        */
        window = DisplayImage(
        display,fid,X_POS,Y_POS,X_SIZ,Y_SIZ,argv[index - 1]);
        window_idx++;

        }/* end remap_data_plane */
        break;
    case ROTATE_FRAME:
        {
        /*
        ** Rotate the frame 45 degrees and set the edge area to the 
        ** minimum pixel value (to demonstate the flag)
        */
        float angle        = 45.0;
        unsigned long flags = ImgM_ReverseEdgeFill;

        retfid = ImgRotateFrame(
                    fid,                                /* frame identifier */
                    &angle,                               /* rotation angle */
                    flags);                                        /* flags */

        /*
        ** Display the frame
        */
        window = DisplayImage(
        display,retfid,X_POS,Y_POS,X_SIZ,Y_SIZ,argv[index - 1]);
        window_idx++;

        /*
        ** Delete old frame and make source fid equal the resultant fid
        */
        ImgDeallocateFrame(fid);                   /* frame identifier */
        fid = retfid;

        }/* end rotate frame */
        break;
    case SCALE_FRAME:
        {
        /*
        ** Scale the ramp 1.5X in the x direction and .75X in the y 
        ** direction
        */
        float scale_x    = 1.50;
        float scale_y    = 0.75;
        unsigned long flags = ImgM_NearestNeighbor;

        retfid = ImgScaleFrame(
                    fid,                                /* frame identifier */
                    &scale_x,                   /* x-direction scale factor */
                    &scale_y,                   /* y-direction scale factor */
                    flags);                                        /* flags */

        /*
        ** Display the frame
        */
        window = DisplayImage(
        display,retfid,X_POS,Y_POS,X_SIZ,Y_SIZ,argv[index - 1]);
        window_idx++;

        /*
        ** Delete old frame and make source fid equal the resultant fid
        */
        ImgDeallocateFrame(fid);                   /* frame identifier */
        fid = retfid;

        }/* end scale frame */
        break;
    default:
        {
        printf("%s pattern not found in parse table\n", argv[index - 1]);
        exit();
        }
    }/* switch (GetSwitchValue) */
    }/* while (index < argc)    */

/*
** Deallocate ramps, ROI's, LUT's and histograms id's
*/
if (roi_id != 0)
    ImgDeleteRoiDef(roi_id);

if (lut_def != 0)
    ImgDeleteLutDef(lut_def);

if (hist_id != 0)
    ImgDeleteHistogram(hist_id);


/*
** Export the DDIF stream of the resultant frame
*/
output_filename = "example_output.ddif";
output_ctx = ImgOpenFile(
                ImgK_ModeExport,                /* MODE, Import or Export    */
                ImgK_FtypeDDIF,
                strlen(output_filename),
                output_filename,    
                0,
                0);

/*
** Export the frame
*/
byte_count = ImgExportFrame(
                fid,                      /* FID,    ISL frame ID           */
                output_ctx,               /* CTX,    DDIF stream ID         */
                0);

byte_count = ImgCloseFile(
                output_ctx,               /* CTX,   DDIF stream ID          */
                0);                      /* FLAGS, Processing flags        */

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
static unsigned long GetSwitchValue(arg_ptr)
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
**  PrintHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine takes a histogram context block (HCB) and prints the pixel
**    value and the corresponding count. Only explicitly indexed histograms
**    are supported.
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
void PrintHistogram(hcb)
struct HCB *hcb;
{
int upper_bound;
int i,j,k;
unsigned long *implicit_table_entry;
struct    EXPLICIT_TABLE_ENTRY
    {
    unsigned long pixel_value;
    unsigned long frequency;
    } *entry;
/*
** Initialize variables
*/
upper_bound = hcb->HcbL_EntryCount;
implicit_table_entry = (unsigned long*)hcb->HcbA_TablePointer;
switch (hcb->HcbL_TableType)
    {
    case ImgK_HtypeImplicitIndex:
        {
        for (i = 0;  i < upper_bound;  i++, implicit_table_entry++)
                printf((i%4)?"pix=%-7x freq=%-3d":"pix=%-7x freq=%-3d\n",
                   i,*implicit_table_entry);
        }
    break;
    case ImgK_HtypeExplicitIndex:
        {
        entry = (struct EXPLICIT_TABLE_ENTRY *)hcb->HcbA_TablePointer;
        if (hcb->HcbL_Flags.HcbV_SortByFreq == 0)
            for (i = 0;  i < upper_bound;  i++, entry++)
                {
                printf((i%4)?"pix=%-7x freq=%-3d":"pix=%-7x freq=%-3d\n",
                   entry->pixel_value,entry->frequency);
                }
        else
            for (i = 0;  i < upper_bound;  i++, entry++)
                {
                printf((i%4)?"freq=%-3d pix=%-7x":"freq=%-3d pix=%-7x\n",
                   entry->frequency, entry->pixel_value);
                }
        }/* End Explicit table type */
        break;
    default:
        printf("Table type not supported\n");
        break;
    }/* End switch */
}/* End PrintHistogram */

/************************************************************************
**
**  DisplayImage
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine demonstrates calls to IDS
**
**  FORMAL PARAMETERS:
**
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

static Window *DisplayImage(display,fid,x_offset,y_offset,width,height,title)
Display          *display;
unsigned long    fid;
long             x_offset;
long             y_offset;
unsigned long    width;
unsigned long    height;
unsigned char    *title;
{
/*
** presentation and rendering itemlists and variables
*/
IdsItmlst2    ps_itmlst[6], rn_itmlst[4];
IdsRendering    *rendering;
unsigned long    psid;

/*
** window structures and characteristics
*/
XSetWindowAttributes    xswa;
XImage                  *ximage;
Window                  window;
unsigned long           border_width = 5;
unsigned long           depth;

/*
**    Create a window to display the image in
*/
xswa.background_pixel    = BlackPixel(display, 0);
xswa.backing_store       = Always;
xswa.border_pixel        = WhitePixel(display, 0);

window = XCreateWindow(
          display,                                            /* Display ID */
          RootWindow(display,0),                                  /* Parent */
          x_offset,                                         /* X coordinate */
          y_offset,                                         /* Y coordinate */
          width,                                            /* window width */
          height,                                          /* window height */
          border_width,                                     /* Border width */
          XDefaultDepth(display,0),                                /* Depth */
          InputOutput,                                             /* Class */
          DefaultVisual(display, 0),                         /* Visual type */
          CWBackPixel | CWBackingStore | CWBorderPixel,       /* attr. mask */
          &xswa);                                           /* Window struct*/

/*
** Now that we have allocated the window, create the itemlists for IDS
*/
ps_itmlst[0].item_name = (char *)IdsNworkstation;
ps_itmlst[0].value = display;
ps_itmlst[1].item_name = (char *) IdsNwsWindow;
ps_itmlst[1].value = window;
ps_itmlst[2].item_name = (char *) IdsNprotocol;
ps_itmlst[2].value = Ids_XImage; 
ps_itmlst[3].item_name = (char *) IdsNwindowWidth;
ps_itmlst[3].value = (unsigned long)width;
ps_itmlst[4].item_name = (char *) IdsNwindowHeight;
ps_itmlst[4].value = (unsigned long)height;
ps_itmlst[5].item_name = (char *) NULL;
ps_itmlst[5].value = NULL;

/*
** create the rendering itemlist
*/
rn_itmlst[0].item_name = (char *) IdsNscaleMode;
rn_itmlst[0].value =  Ids_FitWithin;
rn_itmlst[1].item_name = (char *) NULL;
rn_itmlst[1].value = NULL;

/*
**    Set window manager title
*/
XChangeProperty(
 display,                                           /* X display identifier */
 window,                                             /* X window identifier */
 XA_WM_NAME,                                               /* Name property */
 XA_STRING,                                       /* Property type constant */
 8,                                                     /* Eight-bit format */
 PropModeReplace,                                  /* Change mode = replace */
 title,                                                /* Value of property */
 strlen(title));                                  /* Size of property value */

/*
**    Map the window and wait for stuff to happen...
*/
XMapWindow(display,window);
XSync(display, 0);        

/*
**    Define a presentation surface which describes this window
*/
psid  = IdsCreatePresentSurface(ps_itmlst);

/*
**    Render the image for the window
*/
rendering = IdsCreateRendering(fid,psid,rn_itmlst);

/*
**    Copy the image or the pixmap into the window and wait for it to happen
*/
ximage  = (XImage *) rendering->type_spec_data.xlib.ximage;
printf("\nRendered Image %s: %5d by %5d",title, ximage->width, ximage->height);

/*
**    Copy the image into the window and wait for it to happen
*/
if( rendering->type == Ids_XImage)
     {
     XPutImage(
      display,                                                /* display id */
      window,                                                  /* window id */
      rendering->type_spec_data.xlib.image_gc,               /* GC returned */
      ximage,                                          /* X image structure */
      0,                                                /* Src X coordinate */
      0,                                                /* Src Y coordinate */
      0,                                                /* Dst X coordinate */
      0,                                                /* Dst Y coordinate */
      ximage->width,                                      /* Width of image */
      ximage->height);                                   /* Height of image */

     XSync(display, 0);        
     }/* end XImage processing */

return(window);
}/* end DisplayImage */
