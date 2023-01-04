/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
 * @(#)$RCSfile: lmf_smm.h,v $ $Revision: 1.1.11.4 $ (DEC) $Date: 1993/06/24 22:29:37 $
 */
/*
 * @(#)lmf_smm.h	5.2	(ULTRIX)	5/29/91
 */

/*	Modification History
 *
 *	Randall Brow	06-Sep-90
 *		Added SMMs for DS5000_100 (3MIN).
 *
 * 	Robin		25-May-90
 *		added DS5100 SMM
 *
 *	Lisa Allgood	20-MAR-90
 *	Fix re-defines pulled in from inaccurate VMS file.
 *
 *	Lisa Allgood	5-MAR-90
 *	Added new SMMs to be compatible with the LURT
 */


#ifndef _LMF_SMM_H_
#define _LMF_SMM_H_

/* Symbolic names for LMF system Marketing models */

#define SMM_V780	1
#define SMM_V782	2
#define SMM_V750	3
#define SMM_V730	4
#define SMM_V785	5
#define SMM_VUV1	6
#define SMM_VWS1	7
#define SMM_VUV2	8
#define SMM_VWS2	9
#define SMM_VWSD	10
#define SMM_V8600	11
#define SMM_V8650	12
#define SMM_V8200	13
#define SMM_V8300	14
#define SMM_V8530	15
#define SMM_V8550	16
#define SMM_V8700	17
#define SMM_V8800	18
#define SMM_VWS2000	19
#define SMM_VUV2000	20
#define SMM_VWSD2000	21
#define SMM_V009	22
#define SMM_V8250	23
#define SMM_V8350	24
#define SMM_V3600	25
#define SMM_V3600W	26
#define SMM_V3600D	27
#define SMM_V6210_T	28
#define SMM_V3520	29
#define SMM_V3520L	30
#define SMM_V8840	31
#define SMM_V9RR	32
#define SMM_VUV2_S	33
#define SMM_VUV2_J	34
#define SMM_VWS2_T	35
#define SMM_VWS2_J	36
#define SMM_VWSD_T	37
#define SMM_VWSD_J	38
#define SMM_VUV2000_S	39
#define SMM_VUV2000_J	40
#define SMM_VWS2000_T	41
#define SMM_VWS2000_J	42
#define SMM_VWSD2000_T	43
#define SMM_VWSD2000_J	44
#define SMM_V3600_S	45
#define SMM_V3600_J	46
#define SMM_V3600W_T	47
#define SMM_V3600W_J	48
#define SMM_V3600D_T	49
#define SMM_V3600D_J	50
#define SMM_V3520_S	51
#define SMM_V3520_J	52
#define SMM_V3520L_T	53
#define SMM_V3520L_J	54
#define SMM_V8250L	55
#define SMM_V8250L_J	56
#define SMM_VCV		57
#define SMM_VCVWS	58
#define SMM_VCVWSD	59
#define SMM_VCV_S	60
#define SMM_VCV_J	61
#define SMM_VCVWS_T	62
#define SMM_VCVWS_J	63
#define SMM_VCVWSD_T	64
#define SMM_VCVWSD_J	65
#define SMM_V8500	66
#define SMM_V8370	67
#define SMM_V8651	68
#define SMM_V6220_T	69
#define SMM_V6230_T	70
#define SMM_V6240_T	71
#define SMM_V6250_T	72
#define SMM_V6260_T	73
#define SMM_V6270_T	74
#define SMM_V6280_T	75
#define SMM_V6310_T	76
#define SMM_V6320_T	77
#define SMM_V6330_T	78
#define SMM_V6340_T	79
#define SMM_V6350_T	80
#define SMM_V6360_T	81
#define SMM_V6370_T	82
#define SMM_V6380_T	83
#define SMM_V8810	84
#define SMM_V8820	85
#define SMM_V8830	86
#define SMM_V3400	87
#define SMM_V3400W	88
#define SMM_V3400D	89
#define SMM_V3400_S	90
#define SMM_V3400_J	91
#define SMM_V3400W_T	92
#define SMM_V3400W_J	93
#define SMM_V3400D_T	94
#define SMM_V3400D_J	95
#define SMM_VUV2000_O	96
#define SMM_VWS2000_O	97
#define SMM_VWSD2000_O	98
#define SMM_VWSK2000	99
#define SMM_V6210_S	100
#define SMM_V6220_S	101
#define SMM_V6230_S	102
#define SMM_V6240_S	103
#define SMM_V6250_S	104
#define SMM_V6260_S	105
#define SMM_V6270_S	106
#define SMM_V6280_S	107
#define SMM_V6310_S	108
#define SMM_V6320_S	109
#define SMM_V6330_S	110
#define SMM_V6340_S	111
#define SMM_V6350_S	112
#define SMM_V6360_S	113
#define SMM_V6370_S	114
#define SMM_V6380_S	115
#define SMM_V6200_J	116
#define SMM_V6300_J	117
#define SMM_V3900	118
#define SMM_V3900_S	119
#define SMM_V3900D	120
#define SMM_V3900D_T	121
#define SMM_V3900_J	122
#define SMM_V3900D_J	123
#define SMM_V2000A	124
#define SMM_V2000A_S	125
#define SMM_V2000AW	126
#define SMM_V2000AD	127
#define SMM_V2000AW_T	128
#define SMM_V2000AD_T	129
#define SMM_V2000A_J	130
#define SMM_V2000AW_J	131
#define SMM_V2000AD_J	132
#define SMM_V3540	133
#define SMM_V3540_S	134
#define SMM_V3540L	135
#define SMM_V3540L_T	136
#define SMM_V3560	137
#define SMM_V3560_S	138
#define SMM_V3560L	139
#define SMM_V3560L_T	140
#define SMM_V3580	141
#define SMM_V3580_S	142
#define SMM_V3580L	143
#define SMM_V3580L_T	144
#define SMM_V35A0	145
#define SMM_V35A0_S	146
#define SMM_VPV		147
#define SMM_VPVWS	148
#define SMM_VPVWSD	149
#define SMM_VPV_S	150
#define SMM_VPV_J	151
#define SMM_VPVWS_T	152
#define SMM_VPVWS_J	153
#define SMM_VPVWSD_T	154
#define SMM_VPVWSD_J	155
#define SMM_VTM		156
#define SMM_VTM_S	157
#define SMM_VTM_J	158
#define SMM_V9RR10_T	159
#define SMM_V9RR20_T	160
#define SMM_V9RR30_T	161
#define SMM_V9RR40_T	162
#define SMM_V9RR50_T	163
#define SMM_V9RR60_T	164
#define SMM_V9RR70_T	165
#define SMM_V9RR80_T	166
#define SMM_V9RR10_S	167
#define SMM_V9RR20_S	168
#define SMM_V9RR30_S	169
#define SMM_V9RR40_S	170
#define SMM_V9RR50_S	171
#define SMM_V9RR60_S	172
#define SMM_V9RR70_S	173
#define SMM_V9RR80_S	174
#define SMM_V9RR10_J	175


#define SMM_V9AR10	176
#define SMM_V9AR20	177
#define SMM_V9AQ10     	178
#define SMM_V9AQ20	179
#define SMM_V9AQ30	180
#define SMM_V9AQ40	181
#define SMM_V6305E_T	182
#define SMM_V6305E_S	183
#define SMM_V6305E_J	184
#define SMM_V1201_1T	185
#define SMM_V1201_2T	186
#define SMM_V1201_3T	187
#define SMM_V1201_4T	188
#define SMM_V1201_5T	189
#define SMM_V1201_6T	190
#define SMM_V1201_7T	191
#define SMM_V1201_8T	192
#define SMM_V1201_1S	193
#define SMM_V1201_2S	194
#define SMM_V1201_3S	195
#define SMM_V1201_4S	196
#define SMM_V1201_5S	197
#define SMM_V1201_6S	198
#define SMM_V1201_7S	199
#define SMM_V1201_8S	200
#define SMM_V1201_1J	201
#define SMM_VPV2M_S	202
#define SMM_VPV2M_T	203
#define SMM_VPV2M_J	204
#define SMM_VPV2C_S	205
#define SMM_VPV2C_T	206
#define SMM_VPV2C_J	207
#define SMM_VPV2_S	208
#define SMM_VPV2_T	209
#define SMM_VPV2_J	210
#define SMM_V670	211
#define SMM_V670_S	212
#define SMM_V670_J	213
#define SMM_V520FT	214
#define SMM_VRMAXM_S	215
#define SMM_VRMAXM_T	216
#define SMM_VRMAXM_J	217
#define SMM_VRMAXS_S	218
#define SMM_VRMAXS_T	219
#define SMM_VRMAXS_J	220
#define SMM_VRMAX_S	221
#define SMM_VRMAX_T	222
#define SMM_VRMAX_J	223
#define SMM_VRMAXD_S	224
#define SMM_VRMAXD_T	225
#define SMM_VRMAXD_J	226
#define SMM_VPV0S_S	227
#define SMM_VPV0S_T	228
#define SMM_VPV0S_J	229
#define SMM_VPV1S_S	230
#define SMM_VPV1S_T	231
#define SMM_VPV1S_J	232
#define SMM_VPV2S_S	233
#define SMM_VPV2S_T	234
#define SMM_VPV2S_J	235
#define SMM_VKA46M_S	236
#define SMM_VKA46M_T	237
#define SMM_VKA46M_J	238
#define SMM_VKA46C_S	239
#define SMM_VKA46C_T	240
#define SMM_VKA46C_J	241
#define SMM_VKA46S_S	242
#define SMM_VKA46S_T	243
#define SMM_VKA46S_J	244
#define SMM_VKA46_S	245
#define SMM_VKA46_T	246
#define SMM_VKA46_J	247
#define SMM_V660	248
#define SMM_V660_S	249
#define SMM_V660_J	250
#define SMM_V3820	251
#define SMM_V3820L	252
#define SMM_V3820_S	253
#define SMM_V3820_J	254
#define SMM_V3820L_T	255
#define SMM_V3820L_J	256
#define SMM_V3840	257
#define SMM_V3840_S	258
#define SMM_V3840L	259
#define SMM_V3840L_T	260
#define SMM_V3860	261
#define SMM_V3860_S	262
#define SMM_V3860L	263
#define SMM_V3860L_T	264
#define SMM_V3880	265
#define SMM_V3880_S	266
#define SMM_V3880L	267
#define SMM_V3880L_T	268
#define SMM_V38A0	269
#define SMM_V38A0_S	270
#define SMM_VPV1A	271
#define SMM_VPV1AW	272
#define SMM_VPV1AD	273
#define SMM_VPV1A_S	274
#define SMM_VPV1A_J	275
#define SMM_VPV1AW_T	276
#define SMM_VPV1AW_J	277
#define SMM_VPV1AD_T	278
#define SMM_VPV1AD_J	279
#define SMM_VPV1AS	280
#define SMM_VPV1AS_T	281
#define SMM_VPV1AS_J	282

#define SMM_M3100T	283
#define SMM_M3100M	285
#define SMM_M3100C	286
#define SMM_M2100T	287
#define SMM_M2100M	288
#define SMM_M2100C	289
#define SMM_M5000C	290
#define SMM_M5000_2	291
#define SMM_M5000_3	292
#define SMM_M5000T	293
#define SMM_M5400	294
#define SMM_M5500	295
#define SMM_M5810	296
#define SMM_M5820	297
#define SMM_M5830	298
#define SMM_M5840	299
#define SMM_MCMAX	300
/* Dropped		301 */
#define SMM_M5100	302
#define	SMM_M5000M	303
#define SMM_M5000_100C	304
#define SMM_M5000_100_2	305
#define SMM_M5000_100_3	306
#define SMM_M5000_100T	307
#define SMM_M5000_100M	308
#define SMM_M5000_300C	309
#define SMM_M5000_300_2	310
#define SMM_M5000_300_3	311
#define SMM_M5000_300T	312
#define SMM_M5000_300M	313
#define SMM_MAXINEC	314
#define SMM_MAXINE_2	315
#define SMM_MAXINE_3	316
#define SMM_MAXINET	317
#define SMM_MAXINEM	318

#define SMM_A7000_610	1025
#define SMM_A7000_620	1026
#define SMM_A7000_630	1027
#define SMM_A7000_640	1028
#define SMM_A10000_610	1029
#define SMM_A10000_620	1030
#define SMM_A10000_630	1031
#define SMM_A10000_640	1032
#define SMM_A3000_500W	1033
#define SMM_A3000_500S	1034
#define SMM_A3000_400W	1035
#define SMM_A3000_400S	1036
#define SMM_A4000_610	1037
#define SMM_A4000_620	1038
#define SMM_A4000_810	1039
#define SMM_A4000_820	1040
#define SMM_A_DEMO	1041
#define SMM_A3000_300W	1042
#define SMM_A3000_300S	1043
#define SMM_A2000_300	1044
#define SMM_A4000_710	1057
#define SMM_A4000_720	1058
 
#endif
