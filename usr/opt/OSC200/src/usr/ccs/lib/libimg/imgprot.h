/*
** imgprot.h - IMG prototypes; used internally
*/

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
#if defined(OS2) || defined(msdos) || defined(vaxc) || defined(__STDC__)
#ifndef DAS_EXPAND_PROTO
#define DAS_EXPAND_PROTO 1
#endif
#endif

/*
 * usage: PROTO (return_type function, (arg1, arg2, arg3))
 */
#ifndef PROTO
#if DAS_EXPAND_PROTO == 1
#define PROTO(name, arg_list) name arg_list
#else
#define PROTO(name, arg_list) name ()
#endif
#endif

#ifndef IMGPROT_H

#define IMGPROT_H

#include <img/ipsprot.h>
#include <cda_typ.h>	/* put on 11/01/93 by Dhiren */
#include <ImgDefP.h>

/* calc_crc.c */

PROTO( int calc_crc, (
	long /*fid*/));

/* img__attribute_access_utils.c */

PROTO( struct FCT *_ImgErase, (
	struct FCT */*fct*/, 
	long /*itemcode*/, 
	long /*index*/));

PROTO( struct FCT *_ImgGet, (
	struct FCT */*fct*/, 
	long /*itemcode*/, 
	void */*bufptr*/, 
	long /*buflen*/, 
	long */*retlen*/, 
	long /*index*/));

PROTO( void _ImgLoadSelector, (
	unsigned int /*itemcode*/, 
	struct ITEMCODE */*selector*/));

PROTO( struct FCT *_ImgPut, (
	struct FCT */*fct*/, 
	long /*itemcode*/, 
	void */*bufptr*/, 
	long /*buflen*/, 
	long /*index*/));

/* img__attribute_utils.c */

PROTO( struct FAT *_ImgAdjustFat, (
	struct FAT */*fat_in*/, 
	struct ITMLST */*itmlst*/, 
	unsigned long /*flags*/));

PROTO( long _ImgConvertLevelsToBits, (
	long /*levels*/));

PROTO( struct FAT *_ImgCreateFat, (
	long /*image_data_class*/, 
	struct ITMLST */*itmlst*/));

PROTO( void _ImgDeleteCsa, (
	struct CSA */*csa*/));

PROTO( void _ImgDeleteFat, (
	struct FAT */*fat*/));

PROTO( struct CSA *_ImgExtractCsa, (
	struct FCT */*fid*/));

PROTO( struct FAT *_ImgExtractFat, (
	struct FCT */*fid*/));

PROTO( struct ITMLST *_ImgGetStandardizedAttrlst, (
	struct FCT */*fid*/));

PROTO( void _ImgVerifyFat, (
	struct FAT */*fat*/, 
	unsigned long /*flags*/));

/* img__block_utils.c */

PROTO( struct DCB *_ImgAllocAndInitDcb, (
	long /*access_mode*/, 
	long /*ctx_mode*/));

PROTO( struct DCB *_ImgAllocateDcb, (
	void));

PROTO( struct FCT *_ImgAllocFct, (
	void));

PROTO( struct BHD *_ImgBlkAlloc, (
	long /*blk_size*/, 
	long /*blk_type*/));

PROTO( void _ImgBlkDealloc, (
	struct BHD */*block*/));

PROTO( void _ImgDeallocateDcb, (
	struct DCB */*dcb*/));

PROTO( void _ImgDeallocFct, (
	struct FCT */*fct_adr*/));

PROTO( void _ImgVerifyDcb, (
	struct DCB */*dcb*/, 
	long /*access_mode*/, 
	long /*ctx_mode*/));

/* img__convert_utils.c */

PROTO( long _ImgCvtCompSpaceOrg, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrgGen, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrg1To3, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrg2To3, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrg3To1, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrg3To2, (
 	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrg3To3, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrg3To4, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

PROTO( long _ImgCvtCsOrg4To3, (
	struct CSA */*src_cs_attrs*/, 
	struct CSA */*ret_cs_attrs*/));

/* img__data_plane_utils.c */

PROTO( char *_ImgAllocateDataPlane, (
	int /*size*/, 
	int /*fill_mask*/));

PROTO( char *_ImgAllocDataPlane, (
	long /*size*/, 
	long /*flags*/, 
	int /*fill_mask*/));

PROTO( struct FCT *_ImgAttachDataPlane, (
	struct FCT */*fid*/, 
	char */*data_plane*/, 
	long /*index*/));

PROTO( char *_ImgDetachDataPlane, (
	struct FCT */*fid*/, 
	unsigned long /*index*/));

PROTO( void _ImgFreeDataPlane, (
	char */*image_data*/));

PROTO( char *_ImgReallocateDataPlane, (
	char */*image_data*/, 
	int /*new_size*/));

PROTO( void _ImgStoreDataPlane, (
	struct FCT */*fid*/, 
	char */*image_data*/));

PROTO( void _ImgVerifyDataPlane, (
	struct	FCT */*fid*/, 
	char */*data_plane*/, 
	long /*index*/));

/* img__data_utils.c */

PROTO( long _ImgInterleaveBits, (
	long /*src_plane_cnt*/, 
	long /*src_plane_signif*/, 
	struct UDP */*src_udp_lst*/, 
	struct UDP */*dst_udp*/));

PROTO( long _ImgInterleaveComponents, (
	long /*src_plane_cnt*/, 
	struct UDP */*src_udp_lst*/, 
	long /*dst_cs_org*/, 
	long */*dst_comp_offset*/, 
	struct UDP */*dst_udp*/));

PROTO( long _ImgSeparateBits, (
	struct UDP */*src_udp*/, 
	long /*dst_plane_signif*/, 
	struct UDP */*dst_udp_lst*/));

PROTO( long _ImgSeparateComponents, (
	long /*src_comp_cnt*/, 
	long */*src_comp_offset_lst*/, 
	long */*src_bpc_lst*/, 
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp_lst*/));

/* img__ddif_prefix_mgt.c */

PROTO( void _ImgGetDDIFPrefix, (
	struct DCB */*dcb*/));

PROTO( void _ImgPutDDIFDsc, (
	struct DCB */*dcb*/));

PROTO( void _ImgPutDDIFDhd, (
	struct DCB */*dcb*/));

PROTO( void _ImgPutDDIFPrefix, (
	struct DCB */*dcb*/));

/* img__frame_utils.c */

PROTO( struct FCT *_ImgCloneFrame, (
	struct FCT */*src_fid*/));

PROTO( long _ImgCreateFrameFromSegAggr, (
	CDArootagghandle /*root_aggr*/, 
	CDAagghandle /*seg_aggr*/));

PROTO( struct FCT *_ImgFrameAlloc, (
	CDArootagghandle /*root_aggr*/, 
	CDAagghandle /*seg_aggr*/));

PROTO( struct FCT *_ImgFrameAppendIce, (
	struct FCT */*fct*/, 
	CDAagghandle /*img_aggr*/));

PROTO( struct FCT *_ImgFrameAppendIdu, (
	struct FCT */*fct*/, 
	CDAagghandle /*img_aggr*/, 
	CDAagghandle /*idu_aggr*/));

PROTO( void _ImgFrameDealloc, (
	struct FCT */*fct*/));

PROTO( void _ImgSetFrameDataType, (
	struct FCT */*fid*/));

/* img__io_mgt.c */

PROTO( long _ImgCloseFile, (
	struct FCB */*fcb*/));

PROTO( long _ImgExportAction, (
	unsigned char */*iobufadr*/, 
	unsigned long /*iobufcnt*/, 
	struct FCB */*fcb*/));

PROTO( long _ImgImportAction, (
	unsigned char */*iobufadr*/, 
	unsigned long /*iobuflen*/, 
	unsigned long */*iobufretlen*/, 
	struct FCB */*fcb*/));

PROTO( struct FCB *_ImgOpenFile, (
	unsigned long /*io_mode*/, 
	unsigned long /*access_type*/, 
	unsigned long /*file_type*/, 
	unsigned long /*filename_len*/, 
	unsigned char */*filename_buf*/, 
	unsigned long /*rfilename_len*/, 
	unsigned char */*rfilename_buf*/, 
	unsigned long */*rfilename_retlen*/, 
	unsigned long /*io_bufsize*/, 
	unsigned long /*flags*/));

/* img__itemlist_utils.c */

PROTO( struct GET_ITMLST *_ImgAppendGetlstItem, (
	struct GET_ITMLST */*itmlst*/, 
	unsigned long /*itmcode*/, 
	long /*itmadr*/, 
	long /*itmlen*/, 
	unsigned long int */*retlenadr*/, 
	long /*index*/));

PROTO( struct ITMLST *_ImgAppendItmlstItem, (
	struct ITMLST */*itmlst*/, 
	unsigned long /*itmcode*/, 
	long /*itmadr*/, 
	long /*itmlen*/, 
	unsigned long int */*retlenadr*/, 
	long /*index*/));

PROTO( struct PUT_ITMLST *_ImgAppendPutlstItem, (
	struct PUT_ITMLST */*itmlst*/, 
	unsigned long /*itmcode*/, 
	long /*itmadr*/, 
	long /*itmlen*/, 
	long /*index*/));

PROTO( struct GET_ITMLST *_ImgCreateGetlst, (
	long /*element_count*/));

PROTO( struct ITMLST *_ImgCreateItmlst, (
	long /*element_count*/));

PROTO( struct PUT_ITMLST *_ImgCreatePutlst, (
	long /*element_count*/));

PROTO( struct ITMLST *_ImgCvtPutlstToItmlst, (
	struct PUT_ITMLST */*putlst*/));

PROTO( void _ImgDeleteItmlst, (
	struct ITMLST */*itmlst*/));

PROTO( long _ImgExtractGetlstItem, (
	struct GET_ITMLST */*itmlst*/, 
	unsigned long /*itmcode*/, 
	long */*itmadr*/, 
	long */*itmlen*/, 
	unsigned long int **/*retlenadr*/, 
	long */*index*/, 
	long /*occurance*/));

PROTO( long _ImgExtractItmlstItem, (
	struct ITMLST */*itmlst*/, 
	unsigned long /*itmcode*/, 
	long */*itmadr*/, 
	long */*itmlen*/, 
	unsigned long int **/*retlenadr*/, 
	long */*index*/, 
	long /*occurance*/));

PROTO( long _ImgExtractItmlstItemAdr, (
	struct ITMLST */*itmlst*/, 
	unsigned long /*itmcode*/, 
	long */*itmadr*/, 
	long */*itmlen*/, 
	unsigned long int **/*retlenadr*/, 
	long */*index*/, 
	long /*occurance*/));

PROTO( long _ImgExtractPutlstItem, (
	struct PUT_ITMLST */*itmlst*/, 
	unsigned long /*itmcode*/, 
	long */*itmadr*/, 
	long */*itmlen*/, 
	long */*index*/, 
	long /*occurance*/));

PROTO( long _ImgGetlstItemCount, (
	unsigned long /*item_code*/, 
	struct GET_ITMLST */*itmlst*/));

PROTO( long _ImgItmlstItemCount, (
	unsigned long /*item_code*/, 
	struct ITMLST */*itmlst*/));

PROTO( long _ImgPutlstItemCount, (
	unsigned long /*item_code*/, 
	struct PUT_ITMLST */*itmlst*/));

PROTO( long _ImgTotalGetlstItems, (
	struct GET_ITMLST */*itmlst*/));

PROTO( long _ImgTotalItmlstItems, (
	struct ITMLST */*itmlst*/));

PROTO( long _ImgTotalPutlstItems, (
	struct PUT_ITMLST */*itmlst*/));

/* img__linked_list_utils.c */

PROTO( void _ImgLLAppendElement, (
	struct LHD */*llhead*/, 
	struct BHD */*new_element*/));

PROTO( struct BHD *_ImgLLFirstElement, (
	struct LHD */*llhead*/));

PROTO( struct BHD *_ImgLLLastElement, (
	struct LHD */*llhead*/));

PROTO( struct BHD *_ImgLLNextElement, (
	struct LHD */*llhead*/, 
	struct BHD */*cur_element*/));

PROTO( void _ImgLLPrependElement, (
	struct LHD */*llhead*/, 
	struct BHD */*new_element*/));

PROTO( struct BHD *_ImgLLPrevElement, (
	struct LHD */*llhead*/, 
	struct BHD */*cur_element*/));

PROTO( struct BHD *_ImgLLRemoveElement, (
	struct LHD */*llhead*/, 
	struct BHD */*cur_element*/));

/* img__memory_mgt.c */

PROTO( void ImgResetMemoryMgt, (
	void));

PROTO( void ImgSetMemoryMgtToCda, (
	void));

PROTO( char *_ImgAlloc, (
	unsigned long /*size*/, 
	unsigned long /*flags*/, 
	int /*fill_mask*/));

PROTO( char *_ImgCalloc, (
	int /*number*/, 
	int /*user_size*/));

PROTO( void _ImgCfree, (
	void */*memptr*/));

PROTO( void _ImgDealloc, (
	unsigned char */*bufptr*/));

PROTO( void _ImgFree, (
	void */*memptr*/));

PROTO( long _ImgFree_VM, (
	long */*num_bytes*/, 
	char **/*base_adr*/, 
	long /*param*/));

PROTO( long _ImgGetVM, (
	long */*num_bytes*/, 
	char **/*base_adr*/, 
	long /*param*/));

PROTO( void _ImgInitMemoryMgt, (
	void));

PROTO( char *_ImgMalloc, (
	int /*user_size*/));

PROTO( char *_ImgRealloc, (
	char */*oldbuf*/, 
	int /*user_size*/));

PROTO( void _ImgRestoreMemoryMgt, (
	CDArootagghandle /*user_root_aggr*/, 
	void **/*memctx*/));

PROTO( void _ImgSetMemoryMgt, (
	CDArootagghandle /*user_root_aggr*/, 
	void **/*memctx*/));

/* img__roi.c */

PROTO( long ImgCreateRoi, (
	long /*type*/, 
	char */*bufptr*/, 
	long /*buflen*/));

PROTO( void ImgDeleteRoi, (
	struct ROI */*roi_id*/));

PROTO( struct ROI *_ImgVerifyRoi, (
	struct ROI */*roi_id*/, 
	long /*level*/));

PROTO( long _ImgValidateRoi, (
	struct ROI */*roi*/, 
	struct UDP */*udp*/));

PROTO( long _ImgExportRoi, (
	struct UDP */*udp*/, 
	char */*bufptr*/, 
	long /*buflen*/, 
	long (*/*action*/)(), 
	long /*userparam*/));

PROTO( struct UDP *_ImgSetRoi, (
	struct UDP */*udp*/, 
	struct ROI */*roi*/));

PROTO( struct ROI *roi_create_ccode_brect, (
	struct ROI */*roi_id*/));

PROTO( void roi_verify_ccode_content, (
	struct ROI */*roi_id*/));

PROTO( void roi_verify_rect_content, (
	struct ROI */*roi_id*/));

PROTO( void roi_verify_struct_content, (
	struct ROI */*roi_id*/));

/* img__verify_utils.c */

PROTO( long _ImgCheckNormal, (
	struct FCT */*fid*/));

PROTO( long _ImgGetVerifyStatus, (
	void));

PROTO( long _ImgVerifyAttributes, (
	struct FCT */*fct*/, 
	unsigned long /*flags*/));

PROTO( long _ImgVerifyDataPlanes, (
	struct FCT */*fct*/));

PROTO( long _ImgVerifyNativeFormat, (
	struct FCT */*fid*/, 
	long /*flags*/));

PROTO( long _ImgVerifyStandardFormat, (
	struct FCT */*fid*/, 
	long /*flags*/));

PROTO( long _ImgVerifyStructure, (
	struct FCT */*fct*/));

/* img_adjust_comp_contrast.c */

PROTO( struct FCT *ImgAdjustCompTonescale, (
	struct FCT */*srcfid*/, 
	unsigned long /*comp_index*/, 
	float */*punch_1*/, 
	float */*punch_2*/, 
	struct ROI */*roi*/, 
	unsigned long /*flags*/));

/* img_attribute_access_utils.c */

PROTO( struct FCT *ImgErase, (
	struct FCT */*fid*/, 
	long /*itemcode*/, 
	long /*index*/));

PROTO( struct FCT *ImgGet, (
	struct FCT */*fid*/, 
	long /*selector*/, 
	char */*bufptr*/, 
	long /*buflen*/, 
	long */*retlen*/, 
	long /*index*/));

PROTO( struct FCT *ImgGetFrameAttributes, (
	struct FCT */*fid*/, 
	struct GET_ITMLST */*itmlst*/));

PROTO( struct FCT *ImgGetFrameSize, (
	struct FCT */*fid*/, 
	float */*x_size*/, 
	float */*y_size*/, 
	long /*type*/));

PROTO( struct FCT *ImgSet, (
	struct FCT */*fid*/, 
	long /*itemcode*/, 
	char */*bufptr*/, 
	long /*buflen*/, 
	long /*index*/));

PROTO( struct FCT *ImgSetFrameAttributes, (
	struct FCT */*fid*/, 
	struct PUT_ITMLST */*itmlst*/));

PROTO( struct FCT *ImgSetFrameSize, (
	struct FCT */*fid*/, 
	float */*x_size*/, 
	float */*y_size*/, 
	long /*type*/));

/* img_bitmap_utils.c */

PROTO( struct FCT *ImgExportBitmap, (
	struct FCT */*srcfid*/, 
	struct ROI */*roi_id*/, 
	char */*bufptr*/, 
	long /*buflen*/, 
	long */*bytcnt*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*userprm*/));

PROTO( struct FCT *ImgImportBitmap, (
	struct FCT */*srcfid*/, 
	char */*bufptr*/, 
	int /*buflen*/, 
	int */*bytcnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*userprm*/));

/* img_combine.c */

PROTO( struct FCT *ImgCombine, (
	struct FCT */*dstfid*/, 
	struct ROI */*dstroi*/, 
	struct FCT */*srcfid*/, 
	struct ROI */*srcroi*/, 
	char */*mask*/, 
	long /*rule*/));

PROTO( struct FCT *ImgCombineFrame, (
	struct FCT */*srcfid1*/, 
	struct ROI */*roi1*/, 
	struct FCT */*srcfid2*/, 
	struct ROI */*roi2*/, 
	unsigned long /*rule*/, 
	unsigned long /*flags*/));

/* img_compress.c */

PROTO( struct FCT *ImgCompress, (
	struct FCT */*src_fid*/, 
	int /*scheme*/, 
	char */*param_blk*/));

PROTO( struct FCT *ImgCompressFrame, (
	struct FCT */*src_fid*/, 
	int /*scheme*/, 
	unsigned long /*flags*/, 
	struct ITMLST */*comp_params*/));

/* img_context_utils.c */

PROTO( long ImgFirstContentElement, (
	struct FCT */*fid*/));

PROTO( long _ImgFirstContentElement, (
	struct FCT */*fid*/));

PROTO( long ImgFirstDataPlane, (
	struct FCT */*fid*/));

PROTO( long _ImgFirstDataPlane, (
	struct FCT */*fid*/));

PROTO( long ImgLastContentElement, (
	struct FCT */*fid*/));

PROTO( long _ImgLastContentElement, (
	struct FCT */*fid*/));

PROTO( long ImgLastDataPlane, (
	struct FCT */*fid*/));

PROTO( long _ImgLastDataPlane, (
	struct FCT */*fid*/));

PROTO( long ImgNextContentElement, (
	struct FCT */*fid*/));

PROTO( long _ImgNextContentElement, (
	struct FCT */*fid*/));

PROTO( long ImgNextDataPlane, (
	struct FCT */*fid*/));

PROTO( long _ImgNextDataPlane, (
	struct FCT */*fid*/));

PROTO( struct FCT *ImgPurgeCtx, (
	struct FCT */*fct*/));

PROTO( struct FCT *ImgResetCtx, (
	struct FCT */*fid*/));

PROTO( struct FCT *ImgRestoreCtx, (
	struct FCT */*fct*/));

PROTO( struct FCT *ImgSaveCtx, (
	struct FCT */*fct*/));

PROTO( struct FCT *ImgSetContentElement, (
	struct FCT */*fid*/, 
	long /*content_element_index*/));

PROTO( struct FCT *_ImgSetContentElement, (
	struct FCT */*fid*/, 
	long /*content_element_index*/));

PROTO( struct FCT *ImgSetDataPlane, (
	struct FCT */*fid*/, 
	long /*plane_index*/));

PROTO( struct FCT *_ImgSetDataPlane, (
	struct FCT */*fid*/, 
	long /*plane_index*/));

/* img_convert_utils.c */

PROTO( struct FCT *ImgCvtAlignment, (
	struct FCT */*srcfid*/, 
	struct ITMLST */*itemlist*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgCvtByteBits, (
	struct FCT */*srcfid*/, 
	unsigned long /*bit_order*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgCvtCompSpaceOrg, (
	struct FCT */*src_fid*/, 
	unsigned long /*ret_org*/, 
	unsigned long /*ret_dp_signif*/, 
	unsigned long /*flags*/));

/* img_convolve.c */

PROTO( struct FCT *ImgConvolve, (
	struct FCT */*src_fid*/, 
	struct A2D_DESC */*desc*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/));

PROTO( struct FCT *ImgConvolveFrame, (
	struct FCT */*src_fid*/, 
	unsigned char */*kernel*/, 
	unsigned long /*k_width*/, 
	unsigned long /*k_height*/, 
	unsigned long /*k_type*/, 
	unsigned long /*flags*/));

/* img_data_plane_utils.c */

PROTO( char *ImgAllocateDataPlane, (
	int /*size*/, 
	int /*fill_mask*/));

PROTO( char *ImgAllocDataPlane, (
	long /*size*/, 
	long /*flags*/, 
	int /*fill_mask*/));

PROTO( char *ImgDetachDataPlane, (
	struct FCT */*fid*/, 
	unsigned long /*index*/));

PROTO( struct FCT *ImgAttachDataPlane, (
	struct FCT */*fid*/, 
	char */*data_plane*/, 
	long /*index*/));

PROTO( unsigned long ImgExportDataPlane, (
	struct FCT */*fid*/, 
	unsigned long /*plane_idx*/, 
	unsigned char */*bufadr*/, 
	unsigned long /*buflen*/, 
	unsigned long /*flags*/, 
	unsigned long (*/*action*/)(), 
	long /*usrparam*/));

PROTO( void ImgFreeDataPlane, (
	char */*image_data*/));

PROTO( unsigned long ImgImportDataPlane, (
	struct FCT */*fid*/, 
	unsigned long /*plane_idx*/, 
	unsigned char */*bufadr*/, 
	unsigned long /*buflen*/, 
	unsigned long /*flags*/, 
	unsigned long (*/*action*/)(), 
	long /*usrparam*/));

PROTO( char *ImgReallocateDataPlane, (
	char */*image_data*/, 
	int /*new_size*/));

PROTO( struct FCT *ImgRemapDataPlane, (
	struct FCT */*srcfid*/, 
	unsigned long /*dp_idx*/, 
	struct LTD */*lut*/, 
	struct ROI */*roi*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgStoreDataPlane, (
	struct FCT */*fid*/, 
	char */*image_data*/));

/* img_ddif_export_frame.c */

PROTO( struct FCT *ImgExportDDIFFrame, (
	struct FCT */*fid*/, 
	struct ROI */*roi_id*/, 
	struct DCB */*sid*/, 
	char */*bufadr*/, 
	long /*buflen*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*user_param*/));

PROTO( struct FCT *_ImgExportDDIFFrame, (
	struct FCT */*fid*/, 
	CDArootagghandle /*root_aggregate*/, 
	DDISstreamhandle /*stream_handle*/, 
	long /*flags*/));

/* img_ddif_export_page_break.c */

PROTO( void ImgExportDDIFPageBreak, (
	struct DCB */*ctx*/, 
	long /*flags*/));

/* img_ddif_import_frame.c */

PROTO( struct FCT *ImgImportDDIFFrame, (
	struct DCB */*sid*/, 
	char */*bufadr*/, 
	long /*buflen*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*user_param*/));

PROTO( struct FCT *_ImgImportDDIFFrame, (
	CDAagghandle /*user_seg_aggr*/, 
	CDArootagghandle /*user_root_aggr*/, 
	DDISstreamhandle /*stream_handle*/, 
	long /*flags*/));

PROTO( long _ImgVerifySegmentType, (
	CDArootagghandle /*root_aggr*/, 
	CDAagghandle /*seg_aggr*/, 
	long /*aggr_type*/));

/* img_ddif_io_mgt.c */

PROTO( void ImgCloseDDIFFile, (
	struct DCB */*ctx_id*/, 
	long /*flags*/));

PROTO( struct DCB *ImgCreateDDIFStream, (
	long /*mode*/, 
	char */*bufadr*/, 
	long /*buflen*/, 
	long /*flags*/, 
	long (*/*action*/)(), 
	long /*userprm*/));

PROTO( void ImgDeleteDDIFStream, (
	struct DCB */*dcb*/));

PROTO( struct DCB *ImgOpenDDIFFile, (
	long /*access_mode*/, 
	long /*infilelen*/, 
	char */*infilebuf*/, 
	long /*retfilelen*/, 
	char */*retfilebuf*/, 
	int /*flags*/));

PROTO( long _ImgActionFlush, (
	struct DCB */*dcb*/));

PROTO( CDAstatus _ImgActionGet, (
	CDAuserparam /*usrparm*/, 
	CDAsize */*num_bytes*/, 
	CDAbufaddr */*buf_adr*/));

PROTO( CDAstatus _ImgActionPut, (
	CDAuserparam /*dcb*/, 
	CDAsize */*buflen*/, 
	CDAbufaddr /*bufadr*/, 
	CDAsize */*nextbuflen*/, 
	CDAbufaddr */*nextbufadr*/));

#ifdef IMGDEFP_H
PROTO( CDArootagghandle _ImgCreateRootAggregate, (
	struct ITEMLIST_ELEMENT */*options*/));
#endif

PROTO( void _ImgReallocateIOBuf, (
	struct DCB */*dcb*/));

PROTO( void _ImgPutDocumentEnd, (
	struct DCB */*dcb*/));

/* img_decompress.c */

PROTO( struct FCT *ImgDecompress, (
	struct FCT */*fid*/));

PROTO( struct FCT *ImgDecompressFrame, (
	struct FCT */*src_fid*/, 
	unsigned long /*flags*/, 
	struct ITMLST */*comp_params*/));

/* img_definition_utils.c */

PROTO( struct FCT *ImgAdjustFrameDef, (
	struct FCT */*fdf*/, 
	struct ITMLST */*itmlst*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgCreateFrameDef, (
	long /*image_data_class*/, 
	struct ITMLST */*itmlst*/));

PROTO( void ImgDeleteFrameDef, (
	struct FCT */*fdf*/));

PROTO( struct FCT *ImgExtractFrameDef, (
	struct FCT */*fid*/, 
	unsigned long /*flags*/));

PROTO( void ImgVerifyFrameDef, (
	struct FCT */*frame_def*/, 
	int /*flags*/));

PROTO( struct FCT *_ImgCopyFrameDef, (
	struct FCT */*srcfdf*/, 
	unsigned long /*flags*/));

PROTO( CDAagghandle _ImgCloneIduAggr, (
	CDArootagghandle /*ret_root_aggr*/, 
	CDArootagghandle /*src_root_aggr*/, 
	CDAagghandle /*src_idu_aggr*/));

PROTO( CDAagghandle _ImgCloneImageAggr, (
	CDArootagghandle /*ret_root_aggr*/, 
	CDArootagghandle /*src_root_aggr*/, 
	CDAagghandle /*src_img_aggr*/));

PROTO( CDAagghandle _ImgCloneSegmentAggr, (
	struct FCT */*src_fid*/, 
	CDArootagghandle /*dst_root_aggr*/));

/* img_dither.c */

PROTO( struct FCT *ImgDither, (
	struct FCT */*src_fid*/, 
	struct PUT_ITMLST */*itmlst*/, 
	unsigned long /*dither_type*/, 
	unsigned long /*threshold*/, 
	unsigned long /*flags*/, 
	struct ROI */*roi*/, 
	unsigned long /*output_levels*/[]));

PROTO( struct FCT *ImgDitherFrame, (
	struct FCT */*src_fid*/, 
	unsigned long /*dither_type*/, 
	unsigned long /*threshold*/, 
	unsigned long */*output_levels*/, 
	unsigned long /*flags*/));

/* img_error_handler.c */

PROTO( void _ImgErrorHandler, (
	int /*status*/));

/* img_export_ps.c */

PROTO( struct FCT *ImgExportPS, (
	struct FCT */*fid*/, 
	struct ROI */*roi_id*/, 
	unsigned char */*out_buf_ptr*/, 
	int /*out_buflen*/, 
	int */*byt_cnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*userparm*/));

/* img_export_sixels.c */

PROTO( struct FCT *ImgExportSixels, (
	struct FCT */*fid*/, 
	struct ROI */*roi_id*/, 
	unsigned char */*out_buf_ptr*/, 
	unsigned int /*out_buf_len*/, 
	unsigned int */*byt_cnt*/, 
	unsigned int /*flags*/, 
	long (*/*action*/)(), 
	long /*userparm*/));

/* img_file_io_mgt.c */

PROTO( long ImgCloseFile, (
	struct DCB */*dcb*/, 
	unsigned long /*flags*/));

PROTO( struct DCB *ImgOpenFile, (
	long /*access_mode*/, 
	long /*file_type*/, 
	long /*filename_len*/, 
	char */*filename_str*/, 
	struct ITMLST */*itmlst*/, 
	long /*flags*/));

/* img_filter.c */

PROTO( struct FCT *ImgFilter, (
	struct FCT */*src_fid*/, 
	unsigned long /*filter_type*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/));

PROTO( struct FCT *ImgFilterFrame, (
	struct FCT */*src_fid*/, 
	long /*filter_type*/, 
	long /*flags*/));

/* img_flip.c */

PROTO( struct FCT *ImgFlip, (
	struct FCT */*src_fid*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/));

PROTO( struct FCT *ImgFlipFrame, (
	struct FCT */*src_fid*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/));

/* img_frame_io_mgt.c */

PROTO( unsigned long ImgExportFrame, (
	struct FCT */*srcfid*/, 
	struct DCB */*ctx*/, 
	unsigned long /*flags*/));

PROTO( unsigned long ImgExportPageBreak, (
	struct DCB */*ctx*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgImportFrame, (
	struct DCB */*ctx*/, 
	unsigned long /*flags*/));

/* img_frame_utils.c */

PROTO( struct FCT *ImgAllocateFrame, (
	unsigned long /*data_class*/, 
	struct ITMLST */*itmlst*/, 
	struct FCT */*frame_def*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgConvertFrame, (
	struct FCT */*srcfid*/, 
	struct ITMLST */*itmlst*/, 
	long /*flags*/));

PROTO( struct FCT *ImgCopy, (
	struct FCT */*src_fid*/, 
	struct ROI */*roi*/, 
	struct PUT_ITMLST */*itmlst*/));

PROTO( struct FCT *ImgCopyFrame, (
	struct FCT */*srcfid*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgCreateFrame, (
	struct PUT_ITMLST */*itmlst*/, 
	unsigned long /*data_class*/));

PROTO( void ImgDeallocateFrame, (
	struct FCT */*fid*/));

PROTO( void ImgDeleteFrame, (
	struct FCT */*fid*/));

PROTO( void ImgDumpFrame, (
	struct FCT * /*fid*/,
	char * /*outfile*/ ));

PROTO( void ImgSaveFrame, (
	struct FCT * /*fid*/,
	char * /*outfile*/ ));

PROTO( struct FCT *ImgStandardizeFrame, (
	struct FCT */*srcfid*/, 
	long /*flags*/));

PROTO( void ImgVerifyFrame, (
	struct FCT */*srcfid*/, 
	unsigned long /*flags*/));

/* img_histogram.c */

PROTO( struct HCB *ImgCreateHistogram, (
	struct FCT */*src_fid*/, 
	struct ROI */*roi*/, 
	unsigned long /*table_type*/, 
	unsigned long /*flags*/, 
	unsigned long /*component_idx*/));

PROTO( void ImgDeleteHistogram, (
	struct HCB */*hcb*/));

PROTO( void _ImgVerifyHcb, (
	struct HCB */*hcb*/));

/* img_lut_utils.c */

PROTO( unsigned char *ImgAllocateLut, (
	unsigned long /*lut_size*/, 
	struct LTD */*lut_def*/, 
	unsigned long /*flags*/));

PROTO( struct LTD *ImgAttachLut, (
	unsigned char */*lut*/, 
	unsigned long /*lut_size*/, 
	struct LTD */*lut_def*/, 
	unsigned long /*flags*/));

PROTO( struct LTD *ImgCreateLutDef, (
	unsigned long /*dfn_type*/, 
	struct ITMLST */*table_attrs*/, 
	struct LTD */*srcltd*/, 
	unsigned long /*flags*/));

PROTO( void ImgDeallocateLut, (
	unsigned char */*lut*/));

PROTO( unsigned char *ImgDetachLut, (
	struct LTD */*ltd*/, 
	unsigned long /*flags*/, 
	unsigned long */*ret_lut_size*/));

PROTO( void ImgDeleteLutDef, (
	struct LTD */*ltd*/));

PROTO( void ImgVerifyLutDef, (
	struct LTD */*ltd*/, 
	unsigned long /*flags*/, 
	unsigned long /*fid*/, 
	unsigned long /*dp_index*/));

/* img_roi_utils.c */

PROTO( struct ROI *ImgCreateRoiDef, (
	struct ITMLST */*roi_info*/, 
	unsigned long /*flags*/));

PROTO( void ImgDeleteRoiDef, (
	struct ROI */*roi*/));

PROTO( struct FCT *ImgExtractRoi, (
	struct FCT */*fid*/, 
	struct ROI */*roi*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgSetRectRoi, (
	struct FCT */*srcfid*/, 
	struct ROI */*roi*/, 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgUnsetRectRoi, (
	struct FCT */*srcfid*/));

PROTO( void ImgVerifyRoi, (
	struct ROI */*roi*/, 
	struct FCT */*fid*/, 
	unsigned long /*flags*/));

/* img_rotate.c */

PROTO( struct FCT *ImgRotate, (
	struct FCT */*src_fid*/, 
	float */*angle*/, 
	struct ROI */*roi_id*/, 
	long /*flags*/, 
	long /*alignment*/));

PROTO( struct FCT *ImgRotateFrame, (
	struct FCT */*src_fid*/, 
	float */*angle*/, 
	long /*flags*/));

/* img_scale.c */

PROTO( struct FCT *ImgScale, (
	struct FCT */*src_fid*/, 
	float */*scale_1*/, 
	float */*scale_2*/, 
	struct ROI */*roi_id*/, 
	int /*flags*/, 
	int /*alignment*/));

PROTO( struct FCT *ImgScaleFrame, (
	struct FCT */*src_fid*/, 
	float */*scale_1*/, 
	float */*scale_2*/, 
	int /*flags*/));

/* img_tonescale.c */

PROTO( struct FCT *ImgTonescaleAdjust, (
	struct FCT */*srcfid*/, 
	float */*punch1*/, 
	float */*punch2*/, 
	struct ROI */*roi*/, 
	long /*flags*/, 
	long /*comp_index*/));

/* img_user_utils.c */

PROTO( struct FCT *ImgMakeFrame, (
	unsigned long /*data_type*/, 
	unsigned long /*x*/, 
	unsigned long /*y*/, 
	unsigned long /*num_of_planes*/, 
	unsigned char */*data_ptr_list*/[], 
	unsigned long /*flags*/));

PROTO( struct FCT *ImgMakeCompressedFrame, (
	unsigned long /*compression_type*/, 
	unsigned long /*x*/, 
	unsigned long /*y*/, 
	unsigned long /*num_of_planes*/, 
	unsigned char */*data_ptr_list*/[], 
	unsigned long /*flags*/));

PROTO( void ImgMakeFile, (
	unsigned char */*filename_str*/, 
	unsigned long /*data_type*/, 
	unsigned long /*x*/, 
	unsigned long /*y*/, 
	unsigned long /*num_of_planes*/, 
	unsigned char */*data_ptr_list*/[], 
	unsigned long /*flags*/));

PROTO( void ImgMakeCompressedFile, (
	unsigned char */*filename_str*/, 
	unsigned long /*compression_type*/, 
	unsigned long /*x*/, 
	unsigned long /*y*/, 
	unsigned long /*num_of_planes*/, 
	unsigned char */*data_ptr_list*/[], 
	unsigned long /*flags*/));

#endif /* IMGPROT_H */
