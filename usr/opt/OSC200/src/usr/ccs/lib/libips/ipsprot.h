/*
** ipsprot.h - contains the IPS prototypes used by the IMG facility
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
#ifndef DAS_PROTO
#if DAS_EXPAND_PROTO == 1
#define PROTO(name, arg_list) name arg_list
#else
#define PROTO(name, arg_list) name ()
#endif
#endif

#ifndef IPSPROT_H
#define IPSPROT_H

#include <IpsDef.h>
#ifdef IPS
#include <IpsDefP.h>
#include <ips_fax_paramdef.h>
#endif

PROTO( int _IpsCombineBits, (
	long /*size*/, 
	long /*src_pos*/, 
	unsigned char * /*src_addr*/, 
	long /*dst_pos*/, 
	unsigned char * /*dst_addr*/, 
	long /*mask*/, 
	long /*rule*/));

PROTO( int _IpsMovv5, (
	long /*size*/, 
	long /*src_pos*/, 
	unsigned char * /*src_addr*/, 
	long /*dst_pos*/, 
	unsigned char * /*dst_addr*/));

PROTO( long * _IpsBuildChangelist, (
	unsigned char * /*src_addr*/, 
	long /*src_pos*/, 
	long /*size*/, 
	long * /*cl_addr*/, 
	long /*cl_len*/));

PROTO( long _IpsCombine, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/, 
	char /*mask*/[], 
	long /*rule*/));

PROTO( long _IpsConvolve, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long */*kernel*/, 
	unsigned long /*m*/, 
	unsigned long /*n*/, 
	long /*kernel_data_type*/));

PROTO( long _IpsCopy, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO( long _IpsCopyBitonal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO( long _IpsCopyData, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO( long _IpsDecodeDct, (
	UdpPtr /*cdp*/, 
	UdpPtr /*udplst*/[], 
	int /*udpcnt*/));

PROTO( long _IpsDecodeG31d, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flag*/));

#ifdef IPS
PROTO( long _IpsDecodeG31dScan, (
	struct FAX_DECODE_PARAMS */*parm*/));
#else
PROTO( long _IpsDecodeG31dScan, (
	void */*parm*/));
#endif

PROTO( long _IpsDecodeG32d, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flag*/));

PROTO( long _IpsDecodeG42d, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flag*/));

#ifdef IPS
PROTO( long _IpsDecodeG42dScan, (
	struct FAX_DECODE_PARAMS */*parm*/));
#else
PROTO( long _IpsDecodeG42dScan, (
	void */*parm*/));
#endif

PROTO( long _IpsDitherBluenoise, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*output_levels*/, 
	unsigned long /*flags*/));

PROTO( long _IpsDitherOrdered, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	long /*dither_type*/, 
	long /*threshold_spec*/, 
	long /*output_levels*/, 
	unsigned long /*flags*/));

PROTO( long _IpsEncodeDct, (
	UdpPtr /*udplst*/[], 
	int /*udpcnt*/, 
	UdpPtr /*cdp*/, 
	unsigned long /*factor*/, 
	unsigned long /*resync*/, 
	int */*size_ret*/));

PROTO( long _IpsEncodeG31d, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	long /*flags*/, 
	unsigned long */*compressed_data_size*/));

PROTO( long _IpsEncodeG32d, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	long /*k_factor*/, 
	long /*flags*/, 
	unsigned long */*compressed_data_size*/));

PROTO( long _IpsEncodeG42d, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long */*compressed_data_size*/));

PROTO( long _IpsFlipHorizontal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO( long _IpsFlipHorizontalBitonal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO( long _IpsFlipVertical, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO( long _IpsFlipVerticalAll, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

PROTO( long _IpsFlipVerticalBitonal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO( long _IpsGetUdpElementAlignment, (
	struct UDP */*udp*/));

PROTO( long _IpsGetUdpInfo, (
	struct UDP */*udp*/));

PROTO( long _IpsGetUdpScanlineAlignment, (
	struct UDP */*udp*/));

PROTO( long _IpsHistogram, (
	struct UDP */*udp*/, 
	struct UDP */*cpp*/, 
	unsigned long **/*tab_ptr*/));

PROTO( long _IpsHistogramSorted, (
	struct UDP */*udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*sort_type*/, 
	unsigned long **/*tab_ptr*/, 
	unsigned long */*tab_cnt*/));

PROTO( long _IpsLogical, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*operator*/));

PROTO( long _IpsLogicalBitonal, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*operator*/));

PROTO( long _IpsLogicalCombine, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*operator*/));

PROTO( long _IpsMergePlanes, (
	struct UDP */*udp_ptr*/[], 
	struct UDP */*dst_udp*/, 
	unsigned long /*num_of_planes*/, 
	unsigned long /*flags*/));

PROTO( long _IpsRemapUdp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long */*tab_ptr*/, 
	unsigned long /*tab_cnt*/, 
	unsigned long /*tab_data_type*/, 
	unsigned /*flags*/));

PROTO( long _IpsRotateBitonal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	float */*angle*/, 
	long /*flags*/));

PROTO( long _IpsRotateInterpolation, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	float */*angle*/, 
	long /*flags*/));

PROTO( long _IpsRotateInterpolationByte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	float */*angle_ptr*/, 
	long /*flags*/));

PROTO( long _IpsRotateNearestNeighbor, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	float */*angle*/, 
	long /*flags*/));

PROTO( long _IpsRotateNearestNeighborByte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	float */*angle_ptr*/, 
	long /*flags*/));

PROTO( long _IpsRotateOrthogonal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	long */*angle*/, 
	long /*flags*/));

PROTO( long _IpsScaleBitonal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*fxscale*/, 
	double /*fyscale*/, 
	int /*options*/));

PROTO( long _IpsScaleInterpolation, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*fxscale*/, 
	double /*fyscale*/));

PROTO( long _IpsScaleInterpolationByte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

PROTO( long _IpsScaleNearestNeighbor, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*fxscale*/, 
	double /*fyscale*/));

PROTO( long _IpsScaleNearestNeighborByte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

#ifdef IPS
PROTO( void _IpsEncodeG31dScan, (
	struct FAX_ENCODE_PARAMS */*parm*/));

PROTO( void _IpsEncodeG42dScan, (
	struct FAX_ENCODE_PARAMS */*parm*/));
#else
PROTO( void _IpsEncodeG31dScan, (
	void */*parm*/));

PROTO( void _IpsEncodeG42dScan, (
	void */*parm*/));
#endif

PROTO( void _IpsInitMemoryTable, (
	unsigned char */*allocate_image*/, 
	unsigned char */*deallocate_image*/, 
	unsigned char */*reallocate_image*/, 
	unsigned char */*allocate_buffer*/, 
	unsigned char */*deallocate_buffer*/, 
	unsigned char */*reallocate_buffer*/));

PROTO( void _IpsMovc5Long, (
	long int /*srclen*/, 
	unsigned char */*srcbuf*/, 
	long int /*fillbyte*/, 
	long int /*dstlen*/, 
	unsigned char */*dstbuf*/));

PROTO( void _IpsMovtcLong, (
	long int /*srclen*/, 
	unsigned char */*srcbuf*/, 
	long int /*fillbyte*/, 
	unsigned char */*xlate_table*/, 
	long int /*dstlen*/, 
	unsigned char */*dstbuf*/));

PROTO( void _IpsSetUdpClassAndDType, (
	struct UDP */*udp*/, 
	long /*data_type*/));

/*
** IPS internal prototypes
*/

#ifdef IPS

/* ips__arithmetic.c */

PROTO (long _IpsArithmetic, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsArithmeticByt, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsArithmeticBytCpp, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsArithmeticW, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsArithmeticWCpp, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsArithmeticFlt, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*operator*/));

PROTO (long _IpsArithmeticFltCpp, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*operator*/));

/* ips__change_list.c */

PROTO (int _IpsPutRuns, (
	unsigned char * /*dst_addr*/, 
	long /*dst_pos*/, 
	long * /*cl_addr*/));

/* ips__compare.c */

PROTO (long _IpsCompare, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	long /*operator*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsCompareByt, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*operator*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsCompareW, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*operator*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsCompareFlt, (
	struct UDP */*src1_udp*/, 
	struct UDP */*src2_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*operator*/, 
	unsigned long */*result_flag*/));

/* ips__constant_arithmetic.c */

PROTO (long _IpsConstantArithmetic, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long /*flags*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantArithmeticByt, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantArithmeticBytCpp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantArithmeticW, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantArithmeticL, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantArithmeticWCpp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long /*control*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantArithmeticFlt, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	long /*operator*/));

PROTO (long _IpsConstantArithmeticFltCpp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	double /*constant*/, 
	long /*operator*/));

/* ips__constant_compare.c */

PROTO (long _IpsConstantCompare, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantCompareByt, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantCompareW, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long */*result_flag*/));

PROTO (long _IpsConstantCompareFlt, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*constant*/, 
	unsigned long /*operator*/, 
	unsigned long */*result_flag*/));

/* ips__constrain_cont.c */

PROTO (long _IpsConstrainCont, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*dst_dtype*/, 
	unsigned long /*flags*/));

PROTO (static long IpsScaleData, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*levels*/, 
	unsigned long /*flags*/));

/* ips__copy_utils.c */

PROTO (long _IpsGetUdpPadding, (
	struct UDP */*udp*/));

PROTO (static void Move_bits_by_plane, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (static void Move_bits_by_scanline, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

/* ips__create_lut_mh.c */

PROTO (long _IpsCreateLut_MH, (
	struct UDP */*src_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*density_type*/, 
	double /*param1*/, 
	double /*param2*/, 
	float */*user_pd*/, 
	unsigned long **/*lut_ptr*/));

PROTO (long generate_cdf, (
	unsigned long /*density_type*/, 
	double /*param1*/, 
	double /*param2*/, 
	float */*out_ptr*/, 
	unsigned long /*levels*/, 
	float */*user_pd*/));

/* ips__fft.c */

PROTO (long _IpsForwardFFT, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (long _IpsInverseFFT, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (long _IpsInverseRealFFT, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (void IpsFFT_1D, (
	COMPLEX /*data*/[], 
	unsigned long /*n*/, 
	unsigned long /*ln*/));

PROTO (void IpsShiftForCenterDc, (
	struct UDP */*udp*/));

/* ips__ffx.c */

PROTO (int _IpsFfsLong, (
	long /*start_pos*/, 
	long /*size*/, 
	unsigned char * /*base*/, 
	long */*find_pos*/));

PROTO (int _IpsFfcLong, (
	long /*start_pos*/, 
	long /*size*/, 
	unsigned char * /*base*/, 
	long */*find_pos*/));

/* ips__fill.c */

PROTO (int _IpsFill, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	long /*flags*/));

PROTO (long filter_triplet, (
	unsigned char */*fill_plane*/, 
	long /*d_x1*/, 
	long /*d_x2*/, 
	unsigned long /*s_scanline_stride*/, 
	unsigned long /*d_scanline_stride*/, 
	unsigned char */*border_plane*/, 
	long */*frame_change_flag*/, 
	long /*flags*/));

/* ips__find_deltas.c */

PROTO (void _IpsFindDeltas, (
	long /*x*/[4], 
	long /*y*/[4], 
	unsigned long /*dst_x*/, 
	unsigned long /*dst_y*/, 
	float */*dx*/, 
	float */*dy*/, 
	float */*d_dx*/, 
	float */*d_dy*/, 
	float */*dxr*/, 
	float */*dyr*/, 
	float */*d_dxr*/, 
	float */*d_dyr*/));

/* ips__flip_horizontal.c */

PROTO (long _IpsFlipHByte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

/* ips__flip_horizontal_bitonal.c */

PROTO (long _IpsFlipHBitonal, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

/* ips__freq_filter.c */

PROTO (long _IpsCreateFilter, (
	unsigned long /*filter_type*/, 
	unsigned long /*size*/, 
	unsigned long /*f_pass*/, 
	unsigned long /*f_cutoff*/, 
	unsigned long /*order*/, 
	struct UDP **/*udp*/));

PROTO (long _IpsFilter, (
	struct UDP */*src_udp*/, 
	struct UDP */*filter_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

/* ips__freq_utils.c */

PROTO (long _IpsPowerSpec, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (long _IpsMagnitude, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (long _IpsLogMagnitude, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (long _IpsPhase, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (long _IpsExtractComp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*flags*/));

PROTO (long build_dst, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long */*mem_alloc*/));

PROTO (long _IpsVerifySizeFFT, (
	long /*size*/));

/* ips__geometric.c */

PROTO (long _IpsGeometric, (
	struct UDP */*src_udp*/, 
	unsigned long /*src_x*/, 
	unsigned long /*src_y*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*dst_x_size*/, 
	unsigned long /*dst_y_size*/, 
	double /*dx*/, 
	double /*dy*/, 
	double /*d_dx*/, 
	double /*d_dy*/, 
	double /*dxr*/, 
	double /*dyr*/, 
	double /*d_dxr*/, 
	double /*d_dyr*/, 
	unsigned long /*flags*/));

/* ips__logical.c */

PROTO (long _BufferCombineByte, (
	long /*size*/, 
	long /*src1_pos*/, 
	unsigned char * /*src1_addr*/, 
	long /*src2_pos*/, 
	unsigned char * /*src2_addr*/, 
	long /*dst_pos*/, 
	unsigned char * /*dst_addr*/, 
	long /*rule*/));

PROTO (long _BufferCombineWord, (
	long /*size*/, 
	long /*src1_pos*/, 
	unsigned char * /*src1_addr*/, 
	long /*src2_pos*/, 
	unsigned char * /*src2_addr*/, 
	long /*dst_pos*/, 
	unsigned char * /*dst_addr*/, 
	long /*rule*/));

PROTO (long _BufferCombineLong, (
	long /*size*/, 
	long /*src1_pos*/, 
	unsigned char * /*src1_addr*/, 
	long /*src2_pos*/, 
	unsigned char * /*src2_addr*/, 
	long /*dst_pos*/, 
	unsigned char * /*dst_addr*/, 
	long /*rule*/));

/* ips__logical_bitonal.c */

PROTO (long _BufferCombineBitonal, (
	long /*size*/, 
	long /*src1_pos*/, 
	unsigned char */*src1_addr*/, 
	long /*src2_pos*/, 
	unsigned char */*src2_addr*/, 
	long /*dst_pos*/, 
	unsigned char */*dst_addr*/, 
	long /*rule*/));

PROTO (void _DoBits, (
	long /*bits*/, 
	long /*src1_offset*/, 
	unsigned char */*src1*/, 
	long /*src2_offset*/, 
	unsigned char */*src2*/, 
	long /*dst_offset*/, 
	unsigned char */*dst*/, 
	long /*rule*/));

/* ips__match_histogram.c */

PROTO (long _IpsMatchHistogram, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*density_type*/, 
	double /*param1*/, 
	double /*param2*/, 
	float */*user_pd*/, 
	unsigned long **/*lut_ptr*/, 
	unsigned long /*flags*/));

/* ips__mathematics.c */

PROTO (long _IpsMathematics, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*math_function*/));

PROTO (long _IpsMathByte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

PROTO (long _IpsMathByteCpp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

PROTO (long _IpsMathWord, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

PROTO (long _IpsMathWordCpp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

PROTO (long _IpsMathLong, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

PROTO (long _IpsMathLongCpp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

PROTO (long _IpsMathFloat, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

PROTO (long _IpsMathFloatCpp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	struct UDP */*cpp*/, 
	unsigned long /*function_type*/, 
	double (*/*math_func_ptr*/)(), 
	double /*second_arg*/));

/* ips__movc5.c */

PROTO (void IPS__MOVC3_LONG, (
	int /*length*/, 
	char */*srcbuf*/, 
	char */*dstbuf*/));

/* ips__move_bits.c */

PROTO (long _IpsMoveBits, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedToAligned, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedByteToAlignedByte, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedByteToAlignedWord, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedByteToAlignedLong, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedWordToAlignedWord, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedWordToAlignedLong, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedLongToAlignedLong, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedToUnaligned, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedByteToUnaligned, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedByteToUnalignedPad, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedWordToUnaligned, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedWordToUnalignedPad, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedLongToUnaligned, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsAlignedLongToUnalignedPad, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsUnalignedPadToAlignedByte, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsUnalignedPadToAlignedWord, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsUnalignedPadToAlignedLong, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsUnalignedToAligned, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsUnalignedToAlignedByte, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsUnalignedToAlignedWord, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsUnalignedToAlignedLong, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (long _IpsUnalignedToUnaligned, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsBytePaddedToPadded, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

PROTO (void _IpsMoveBitByBit, (
	struct UDP */*srcudp*/, 
	struct UDP */*dstudp*/));

/* ips__point_statistics.c */

PROTO (long _IpsPointStatistics, (
	struct UDP */*src_udp*/, 
	struct UDP */*cpp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsByt, (
	struct UDP */*src_udp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsCppByt, (
	struct UDP */*src_udp*/, 
	struct UDP */*cpp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsW, (
	struct UDP */*src_udp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsCppW, (
	struct UDP */*src_udp*/, 
	struct UDP */*cpp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsLong, (
	struct UDP */*src_udp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsCppLong, (
	struct UDP */*src_udp*/, 
	struct UDP */*cpp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsFlt, (
	struct UDP */*src_udp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

PROTO (long _IpsStatisticsCppFlt, (
	struct UDP */*src_udp*/, 
	struct UDP */*cpp*/, 
	double */*number_of_pixels*/, 
	double */*number_of_zeroes*/, 
	double */*minimum*/, 
	double */*maximum*/, 
	double */*sum*/, 
	double */*sum_of_squares*/));

/* ips__promote_data_type.c */

PROTO (long _IpsPromoteDataType, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long /*dst_dtype*/, 
	unsigned long /*flags*/));

/* ips__remap_services.c */

PROTO (long _remap_byte_udp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	unsigned long */*tab_ptr*/, 
	unsigned long /*tab_cnt*/, 
	unsigned long /*tab_data_type*/));

/* ips__rotate_cont.c */

PROTO (long _IpsRotateCont90Byte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

PROTO (long _IpsRotateCont180Byte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

PROTO (long _IpsRotateCont270Byte, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/));

PROTO (void _IpsGetRotatedUdp, (
	struct UDP */*src_udp*/, 
	struct UDP */*dst_udp*/, 
	double /*angle*/, 
	float */*xmin*/, 
	float */*ymin*/));

/* ips__separate_plane.c */

PROTO (long _IpsSeparatePlane, (
	struct UDP */*src_udp*/, 
	struct UDP */*udp_ptr*/[], 
	unsigned long /*num_of_planes*/, 
	unsigned long */*bits_per_plane*/, 
	unsigned long */*dtypes*/));

/* ips__udp_utils.c */

PROTO (long _IpsBuildDstUdp, (
	struct UDP */*src*/, 
	struct UDP */*dst*/, 
	unsigned long /*init_flag*/, 
	unsigned long /*retain_flag*/, 
	unsigned long /*in_place*/));

PROTO (long _IpsCreateUdp, (
	unsigned long /*x_size*/, 
	unsigned long /*y_size*/, 
	unsigned long /*x_sub_size*/, 
	unsigned long /*y_sub_size*/, 
	unsigned long /*x_offset*/, 
	unsigned long /*y_offset*/, 
	unsigned long /*dtype*/, 
	struct UDP **/*dst_udp*/));

PROTO (long _IpsMinRectUdp, (
	struct UDP */*src*/));

PROTO (long _IpsVerifyNotInPlace, (
	struct UDP */*src*/, 
	struct UDP */*dst*/));

PROTO (long _IpsGetStartAddress, (
	struct UDP */*udp*/, 
	unsigned long */*start_addr*/));

/* uips__combine_bits .c */
PROTO (void _IpsLogicalCombineBits, (
	long	/*size	    */,
	long	/*src1_pos  */,
	long	/*src1_addr */,
	long	/*src2_pos  */,
	long	/*src2_addr */,
	long	/*dst_pos   */,
	long	/*dst_addr  */,
	long	/*rule	    */
	));

PROTO (void _IpsLogicalCombineBitsBitonal, (
	long	/*size	    */,
	long	/*src1_pos  */,
	long	/*src1_addr */,
	long	/*src2_pos  */,
	long	/*src2_addr */,
	long	/*dst_pos   */,
	long	/*dst_addr  */,
	long	/*rule	    */
	));


#endif /* IPS */

#endif /* IPSPROT_H */
