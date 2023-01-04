/********************************************************************************************************************************/
/* Created 27-JUL-1993 17:18:18 by VAX SDL V3.2-12     Source:  2-APR-1993 12:56:35 VAMASD$:[IMG.SRC]IMG$ENTRY.SDL;1 */
/********************************************************************************************************************************/
 
/*** MODULE IMG$ENTRY ***/
#ifndef IMGENTRY_H
#define IMGENTRY_H
/*
 * The DAS_EXPAND_PROTO flag along with the PROTO macro allow for tailoring
 * routine declarations to expand to function prototypes or not depending
 * on the particular platform (compiler) capabilities.
 * If DAS_EXPAND_PROTO is defined, the PROTO macro will expand to function
 * prototypes.  If OS2 or msdos turn on flag as prototypes must be used
 * on these platforms.  For other platforms it is left to the application
 * to #define DAS_EXPAND_PROTO before #include of this file if function
 * prototyping is desired.
 */
#if defined(OS2) || defined(msdos) || defined(__vaxc__) || defined(__STDC__)
#ifndef DAS_EXPAND_PROTO
#define DAS_EXPAND_PROTO 1
#endif
#endif
/*
 * usage: PROTO (return_type function, (arg1, arg2, arg3))
 */
#ifndef DAS_PROTO
#if DAS_EXPAND_PROTO == 1
#define PROTO(name, arg_list) name arg_list
#else
#define PROTO(name, arg_list) name ()
#endif
#endif
PROTO( char *ImgAllocateDataPlane, (
	int /*size*/, 
	int /*fill_mask*/));
PROTO( unsigned long ImgAllocateFrame, (
	unsigned long /*data_class*/, 
	struct ITMLST */*itmlst*/, 
	unsigned long /*frame_def*/, 
	unsigned long /*flags*/));
PROTO( unsigned char *ImgAllocateLut, (
	unsigned long /*lut_size*/, 
	unsigned long /*lut_def*/, 
	unsigned long /*flags*/));
PROTO( char *ImgAllocDataPlane, (
	long /*size*/, 
	long /*flags*/, 
	int /*fill_mask*/));
PROTO( unsigned long ImgAttachDataPlane, (
	unsigned long /*fid*/, 
	char */*data_plane*/, 
	long /*index*/));
PROTO( unsigned long ImgAttachLut, (
	unsigned char */*lut*/, 
	unsigned long /*lut_size*/, 
	unsigned long /*lut_def*/, 
	unsigned long /*flags*/));
PROTO( long ImgCloseFile, (
	unsigned long /*dcb*/, 
	unsigned long /*flags*/));
PROTO( void ImgCloseDDIFFile, (
	unsigned long /*ctx_id*/, 
	long /*flags*/));
PROTO( unsigned long ImgCombine, (
	unsigned long /*dstfid*/, 
	struct ROI */*dstroi*/, 
	unsigned long /*srcfid*/, 
	struct ROI */*srcroi*/, 
	char */*mask*/, 
	long /*rule*/));
PROTO( unsigned long ImgCombineFrame, (
	unsigned long /*srcfid1*/, 
	unsigned long /*roi1*/, 
	unsigned long /*srcfid2*/, 
	unsigned long /*roi2*/, 
	unsigned long /*rule*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgCompress, (
	unsigned long /*src_fid*/, 
	char /*scheme*/, 
	char */*param_blk*/));
PROTO( unsigned long ImgCompressFrame, (
	unsigned long /*src_fid*/, 
	char /*scheme*/, 
	unsigned long /*flags*/, 
	struct ITMLST */*comp_params*/));
PROTO( unsigned long ImgConvolve, (
	unsigned long /*src_fid*/, 
	unsigned long /*desc*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/));
PROTO( unsigned long ImgConvolveFrame, (
	unsigned long /*src_fid*/, 
	unsigned char */*kernel*/, 
	unsigned long /*k_width*/, 
	unsigned long /*k_height*/, 
	unsigned long /*k_type*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgCopy, (
	unsigned long /*src_fid*/, 
	struct ROI */*roi*/, 
	struct PUT_ITMLST */*itmlst*/));
PROTO( unsigned long ImgCopyFrame, (
	unsigned long /*srcfid*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgCreateDDIFStream, (
	long /*mode*/, 
	char */*bufadr*/, 
	long /*buflen*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*userprm*/));
PROTO( unsigned long ImgCreateFrame, (
	struct PUT_ITMLST */*itmlst*/, 
	unsigned long /*data_class*/));
PROTO( unsigned long ImgCreateHistogram, (
	unsigned long /*src_fid*/, 
	unsigned long /*roi*/, 
	unsigned long /*table_type*/, 
	unsigned long /*flags*/, 
	unsigned long /*component_idx*/));
PROTO( unsigned long ImgCreateLutDef, (
	unsigned long /*dfn_type*/, 
	struct ITMLST */*table_attrs*/, 
	unsigned long /*srcltd*/, 
	unsigned long /*flags*/));
PROTO( long ImgCreateRoi, (
	long /*type*/, 
	char */*bufptr*/, 
	long /*buflen*/));
PROTO( unsigned long ImgCreateRoiDef, (
	struct ITMLST */*roi_info*/, 
	unsigned long /*flags*/));
PROTO( void ImgDeallocateFrame, (
	unsigned long /*fid*/));
PROTO( void ImgDeallocateLut, (
	unsigned char */*lut*/));
PROTO( unsigned long ImgDecompress, (
	unsigned long /*fid*/));
PROTO( unsigned long ImgDecompressFrame, (
	unsigned long /*src_fid*/, 
	unsigned long /*flags*/, 
	struct ITMLST */*comp_params*/));
PROTO( void ImgDeleteDDIFStream, (
	unsigned long /*dcb*/));
PROTO( void ImgDeleteFrame, (
	unsigned long /*fid*/));
PROTO( void ImgDeleteHistogram, (
	unsigned long /*hcb*/));
PROTO( void ImgDeleteLutDef, (
	unsigned long /*ltd*/));
PROTO( void ImgDeleteRoi, (
	struct ROI */*roi_id*/));
PROTO( void ImgDeleteRoiDef, (
	unsigned long /*roi*/));
PROTO( char *ImgDetachDataPlane, (
	unsigned long /*fid*/, 
	unsigned long /*index*/));
PROTO( unsigned char *ImgDetachLut, (
	unsigned long /*ltd*/, 
	unsigned long /*flags*/, 
	unsigned long */*ret_lut_size*/));
PROTO( unsigned long ImgDither, (
	unsigned long /*src_fid*/, 
	struct PUT_ITMLST */*itmlst*/, 
	unsigned long /*dither_type*/, 
	unsigned long /*threshold*/, 
	unsigned long /*flags*/, 
	struct ROI */*roi*/, 
	unsigned long /*output_levels*/[]));
PROTO( unsigned long ImgDitherFrame, (
	unsigned long /*src_fid*/, 
	unsigned long /*dither_type*/, 
	unsigned long /*threshold*/, 
	unsigned long */*output_levels*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgErase, (
	unsigned long /*fid*/, 
	long /*itemcode*/, 
	long /*index*/));
PROTO( unsigned long ImgExportBitmap, (
	unsigned long /*srcfid*/, 
	struct ROI */*roi_id*/, 
	char */*bufptr*/, 
	long /*buflen*/, 
	long */*bytcnt*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*userprm*/));
PROTO( unsigned long ImgExportDataPlane, (
	unsigned long /*fid*/, 
	unsigned long /*plane_idx*/, 
	unsigned char */*bufadr*/, 
	unsigned long /*buflen*/, 
	unsigned long /*flags*/, 
	unsigned long (*/*action*/)(), 
	long /*usrparam*/));
PROTO( unsigned long ImgExportDDIFFrame, (
	unsigned long /*fid*/, 
	struct ROI */*roi_id*/, 
	unsigned long /*sid*/, 
	char */*bufadr*/, 
	long /*buflen*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*user_param*/));
PROTO( void ImgExportDDIFPageBreak, (
	unsigned long /*ctx*/, 
	long /*flags*/));
PROTO( unsigned long ImgExportFrame, (
	unsigned long /*srcfid*/, 
	unsigned long /*ctx*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgExportPageBreak, (
	unsigned long /*ctx*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgExportPS, (
	unsigned long /*fid*/, 
	struct ROI */*roi_id*/, 
	unsigned char */*out_buf_ptr*/, 
	int /*out_buflen*/, 
	int */*byt_cnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*userparm*/));
PROTO( unsigned long ImgExportSixels, (
	unsigned long /*fid*/, 
	struct ROI */*roi_id*/, 
	unsigned char */*out_buf_ptr*/, 
	unsigned int /*out_buf_len*/, 
	unsigned int */*byt_cnt*/, 
	unsigned int /*flags*/, 
	long (*/*action*/)(), 
	long /*userparm*/));
PROTO( unsigned long ImgExtractRoi, (
	unsigned long /*fid*/, 
	unsigned long /*roi*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgFilter, (
	unsigned long /*src_fid*/, 
	unsigned long /*filter_type*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/));
PROTO( unsigned long ImgFilterFrame, (
	unsigned long /*src_fid*/, 
	long /*filter_type*/, 
	long /*flags*/));
PROTO( unsigned long ImgFlip, (
	unsigned long /*src_fid*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/));
PROTO( unsigned long ImgFlipFrame, (
	unsigned long /*src_fid*/, 
	unsigned long /*roi_id*/, 
	long /*flags*/));
PROTO( void ImgFreeDataPlane, (
	char */*image_data*/));
PROTO( unsigned long ImgGet, (
	unsigned long /*fid*/, 
	long /*selector*/, 
	char */*bufptr*/, 
	long /*buflen*/, 
	long */*retlen*/, 
	long /*index*/));
PROTO( unsigned long ImgGetFrameAttributes, (
	unsigned long /*fid*/, 
	struct GET_ITMLST */*itmlst*/));
PROTO( unsigned long ImgGetFrameSize, (
	unsigned long /*fid*/, 
	float */*x_size*/, 
	float */*y_size*/, 
	long /*type*/));
PROTO( unsigned long ImgImportBitmap, (
	unsigned long /*srcfid*/, 
	char */*bufptr*/, 
	int /*buflen*/, 
	int */*bytcnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*userprm*/));
PROTO( unsigned long ImgImportDataPlane, (
	unsigned long /*fid*/, 
	unsigned long /*plane_idx*/, 
	unsigned char */*bufadr*/, 
	unsigned long /*buflen*/, 
	unsigned long /*flags*/, 
	unsigned long (*/*action*/)(), 
	long /*usrparam*/));
PROTO( unsigned long ImgImportDDIFFrame, (
	unsigned long /*sid*/, 
	char */*bufadr*/, 
	long /*buflen*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*user_param*/));
PROTO( unsigned long ImgImportFrame, (
	unsigned long /*ctx*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgOpenFile, (
	long /*access_mode*/, 
	long /*file_type*/, 
	long /*filename_len*/, 
	char */*filename_str*/, 
	struct ITMLST */*itmlst*/, 
	long /*flags*/));
PROTO( unsigned long int ImgPixelRemap, ());
PROTO( char *ImgReallocateDataPlane, (
	char */*image_data*/, 
	int /*new_size*/));
PROTO( unsigned long ImgRemapDataPlane, (
	unsigned long /*srcfid*/, 
	unsigned long /*dp_idx*/, 
	unsigned long /*lut*/, 
	unsigned long /*roi*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgRotate, (
	unsigned long /*src_fid*/, 
	float */*angle*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/, 
	long /*alignment*/));
PROTO( unsigned long ImgRotateFrame, (
	unsigned long /*src_fid*/, 
	float */*angle*/, 
	long /*flags*/));
PROTO( unsigned long ImgScale, (
	unsigned long /*src_fid*/, 
	float */*scale_1*/, 
	float */*scale_2*/, 
	struct ROI */*roi_id*/, 
	int /*flags*/, 
	int /*alignment*/));
PROTO( unsigned long ImgScaleFrame, (
	unsigned long /*src_fid*/, 
	float */*scale_1*/, 
	float */*scale_2*/, 
	int /*flags*/));
PROTO( unsigned long ImgSet, (
	unsigned long /*fid*/, 
	long /*itemcode*/, 
	char */*bufptr*/, 
	long /*buflen*/, 
	long /*index*/));
PROTO( unsigned long ImgSetFrameAttributes, (
	unsigned long /*fid*/, 
	struct PUT_ITMLST */*itmlst*/));
PROTO( unsigned long ImgSetFrameSize, (
	unsigned long /*fid*/, 
	float */*x_size*/, 
	float */*y_size*/, 
	long /*type*/));
PROTO( unsigned long ImgSetRectRoi, (
	unsigned long /*srcfid*/, 
	unsigned long /*roi*/, 
	unsigned long /*flags*/));
PROTO( unsigned long ImgStoreDataPlane, (
	unsigned long /*fid*/, 
	char */*image_data*/));
PROTO( unsigned long ImgTonescaleAdjust, (
	unsigned long /*srcfid*/, 
	float */*punch1*/, 
	float */*punch2*/, 
	struct ROI */*roi*/, 
	long /*flags*/, 
	long /*comp_index*/));
PROTO( unsigned long ImgUnsetRectRoi, (
	unsigned long /*srcfid*/));
PROTO( void ImgVerifyFrame, (
	unsigned long /*srcfid*/, 
	unsigned long /*flags*/));
#endif /* IMGENTRY_H */
