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
/* @(#)$RCSfile: uucp_msgdf.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/09/07 16:09:13 $ */
/* uucp_msgdf.h	1.2  com/cmd/uucp,3.1,9013 10/10/89 13:52:36 */
/* 
 * COMPONENT_NAME: UUCP uucp_msgdf.h
 * 
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*  Where applicable, the following prefixes have been used in 
 *  assigning define names:
 *
 *	"D"   -  message is used for a call to DEBUG()
 *	"CD"  -  message is used for a call to CDEBUG()
 *	"V"   -  message is used for a call to VERBOSE()
 *	"L"   -  message is used for a call to Logent()
 *	"A"   -  message is used for a call to ASSERT()
 * 	"e"   -  message is used for a call to errent()
 */

/* There are no message related defines from the following soruce files:
 *
 *	uucpname.c	chremdir.c 	cpmv.c
 *	eio.c	 	getargs.c 	getpwinfo.c
 *	getprm.c	gio.c 		gnamef.c
 *	gwd.c	 	imsg.c 		pkdefs.c
 *	strpbrk.c	uucpd.c 	xio.c
 *	xqt.c	 	dbg.h 		parms.h
 *	pk.h
 */

/* The following message related defines are from uucheck.c */ 
/* uucheck.c defines start at 0 */

#define  USAGE_M 	0
#define  USAGE_E 	1
#define  CHK_REQUIRED 	2
#define  DIR_CHK 	3
#define  CHECK_FILE	4
#define  CHK_COMPLETE  	5
#define  MACHINE_PHASE  6
#define  LOGNAME_PHASE 	7
#define  OPEN_ERR	8
#define  WHEN_ASYS	9
#define  CALL_BACK	10
#define  WE_CALL	11
#define  DO_ALLOW 	12
#define  DO_NOT_ALLOW 	13
#define  WILL_SEND 	14
#define  WILL_NOT_SEND 	15
#define  SEND_FILES_TO 	16
#define  DEFAULT_PUBDIR	17
#define  EXCEPT		18
#define  FILES_FROM	19
#define	 CAN_EXEC 	20
#define  MY_NAME 	21
#define  PUBDIR_NAME 	22
#define  MACHINES	23
#define  SOFT_ERR 	24
#define  COMMAND_NAME 	25



/* These message defines are from callers.c */
/* callers.c defines start at 0 */

#define D_INT_CALLER 			50
#define D_MLOCK_A_FAILED		51
#define D_MLOCK_FAILED			53
#define D_MLOCK_SUCCESS			54
#define D_GOPEN_TIMEOUT 		55
#define L_GEN_OPEN			56
#define L_TIMEOUT			57
#define D_G_OPEN_FAILED 		58
#define L_G_OPEN			59
#define L_FAILED			60
#define D_NOT_FOUND_DFILE 		61
#define L_GCALL_GDIAL			62
#define D_GDIAL_CALLED			64
#define D_DKCALLS 			65
#define A_NO_SERVER 			66
#define L_NO_HOST			67
#define D_TCPDIAL_HOST			68
#define D_PORT				69
#define D_NO_SOCKET_ 			70
#define L_no_socket			71
#define D_NO_SOCKET_E			72
#define L_TCPOPEN			73
#define L_NO_SOCKET			74
#define D_TIMEOUT_TCPOPEN		75
#define D_FAMILY			78
#define D_CON_FAILED 			80
#define L_con_failed 			81
#define D_ECON_FAILED 			82
#define L_CON_FAILED 			83
#define D_UNETDIAL			84
#define D_TCOPEN_FAILED 		85
#define D_S_OPEN_FAILED 		86
#define D_COM1_RET			87
#define D_TIMEOUT_SYTEK			88
#define D_OPEN2F			89
#define D_MLOCK_A_SUCCESS 		91
#define CCD_USEPORT 			93
#define D_M_ACU				94
#define CCD_PHONE_NUMBER		95
#define V_TRY_MODEM			96
#define V_ACU				97
#define V_CALLING 			98
#define V_SUCCEEDED			99
#define V_FAILED 			100
#define D_CANT_OPEN			101
#define D_IS_OPEN 			102
#define L_801_OPEN			103
#define D_ACU_WR_E			105
#define L_ACU_WR 			106
#define D_ACU_WR_OK 			108
#define D_DCF				109
#define D_FORKSTAT 			110
#define D_DNWRITE			111
#define D_timeout 			112
#define D_DCF_IS 			116
#define D_ACU_WR 			117
#define D_LINE_OPEN 			119
#define D_failed  			120

/* The following message related defines are from conn.c */ 
/* conn.c defines start at 200 */

#define CD_CONN_ 		200
#define CD_GETTO 		201
#define D_CLOSE_CALLER  	202
#define D_DELOCK_		203
#define CD_CALL_FAILED		204
#define CD_DEV_TYPE 		205
#define CD_REQ_DEV_TNF 		206
#define A_BAD_LINE 		207
#define CD_WRONG_TIME		208
#define D_PROSTR 		209
#define CD_EXPECT 		211
#define CD_LOST_LINE 		212
#define L_LOGIN			213
#define L_LOST_LINE		214
#define CD_ENOUGH		215
#define CD_GOT_IT 		216
#define CD_TIMED_OUT 		217
#define CD_BREAK		218
#define CD_EOT 			219
#define CD_SENTTHEM  		220
#define CD_DELAY		221
#define CD_NO_CR 		222
#define CD_NO_CR_MI 		223
#define CD_PAUSE		224
#define CD_E_CK_ON 		225
#define CD_E_CK_OFF 		226

/* The following message related defines are from ct.c */ 
/* ct.c defines start at 250 */

#define MSG_CT1			250
#define MSG_CT2 		251
#define MSG_CT3 		252
#define MSG_CT5			253
#define MSG_CT6			254
#define MSG_CT7			255
#define MSG_CT20		256
#define MSG_CT8 		257
#define MSG_CT9 		258 
#define MSG_CT10		259
#define MSG_CTV1		260
#define MSG_CTV2		261
#define MSG_CTV3		262
#define MSG_CTV4		263
#define MSG_CT11		264
#define MSG_CT12		265
#define MSG_CT13		266
#define MSG_CT14		267
#define MSG_CT15		268
#define MSG_CT16		269
#define MSG_CT17		270
#define MSG_CT18		271
#define MSG_CTV5		272
#define MSG_CT22 		273
#define MSG_CTCD1		274
#define MSG_CTCD2		275
#define MSG_CT19 		276
#define MSG_CTV6 		277
#define MSG_CT21		278
#define MSGSTR_CTV7		279
#define MSG_CTCD3 		280
#define MSG_CTCD4 		281
#define MSG_CTCD5 		282
#define MSG_CTCD6 		283

/* The following message related defines are from line.c */ 
/* line.c defines start at 300 */

#define MSG_LINEA1 		300
#define MSG_LINEA2 		301
#define MSG_LINEA4		302
#define MSG_LINEA5		303

/* The following message related defines are from ulockf.c */ 
/* ulockf.c defines start at 350 */

#define MSG_ULOCKFA1		350
#define MSG_ULOCKFA2		351
#define MSG_ULOCKFL1		352
#define MSG_ULOCKFL2 		353
#define MSG_ULOCKFL3		354

/* The following message related defines are from altconn.c */ 
/* ultconn.c defines start at 375 */

#define	 MSG_ALTCONNCD1		375
#define	 MSG_ALTCONNCD2		376

/* The following message related defines are from anlwrk.c */ 
/* anlwrk.c defines start at 400 */

#define  MSG_ANLWRKe1 		400
#define  MSG_ANLWRKe2		401

/* The following message related defines are from cico.c */ 
/* cico.c defines start at 425 */

#define  MSG_CICOA1		425
#define  MSG_CICOA2		426
#define  MSG_CICO1 		427
#define  MSG_CICO6		428
#define  MSG_CICOL1		429
#define  MSG_CICOL2		430
#define  MSG_CICOL3		431
#define  MSG_CICO2 		432
#define  MSG_CICOL4		433
#define  MSG_CICO3		434
#define  MSG_CICO4		435
#define  MSG_CICOL5 		436
#define  MSG_CICO5		437
#define  MSG_CICOA3		438
#define  MSG_CICOL21		439
#define  MSG_CICOL6		440
#define  MSG_CICO9		441
#define  MSG_CICOL22		442
#define  MSG_CICOL7		443
#define  MSG_CICOL8		444
#define  MSG_CICOCD1		445
#define  MSG_CICOCD2		446
#define  MSG_CICOL10		447
#define  MSG_CICOL11 		448
#define  MSG_CICO7 		449
#define  MSG_CICOL17		450
#define  MSG_CICOCD3		451
#define  MSG_CICOCD4 		452
#define  MSG_CICOCD5 		453
#define  MSG_CICOL19		454
#define  MSG_CICOL20		455

/* The following message related defines are from dbg.c */ 
/* dbg.c defines start at 480 */

#define  MSG_DBG1 		480
#define  MSG_DBG2		481
#define  MSG_DBG3		482

/* The following message related defines are from expfile.c */ 
/* expfile.c defines start at 490 */

#define  MSG_EXPFILE1		490

/* The following message related defines are from culine.c */ 
/* culine.c defines start at 500 */

#define  MSG_CULINECD1 		500
#define  MSG_CULINEA1		501
#define  MSG_CULINECD2 		502
#define  MSG_CULINEA2  		503

/* The following message related defines are from cu.c */ 
/* cu.c defines start at 525 */

#define  SG_CU1			525
#define  SG_CU2			526
#define  SG_CU3			527
#define  SG_CU4			528
#define  SG_CU5			529
#define  SG_CU6			530
#define  SG_CU7			531
#define  SG_CU8			532
#define  SG_CU9			533
#define  SG_CU10		534
#define  SG_CU11		535
#define  SG_CU12		536
#define  SG_CU13		537
#define  SG_CU14		538
#define  SG_CU15		539
#define  SG_CU16		540
#define  SG_CU17		541
#define  SG_CU18		542
#define  SG_CU19		543
#define  SG_CU20		544
#define  SG_CU21		545
#define  SG_CU22		546
#define  SG_CUV1		547
#define  SG_CUV2		548
#define  SG_CUCD1		549
#define  SG_CUCD2		550
#define  SG_CUCD3		551
#define  SG_CUCD4	 	552
#define  SG_CUV3		553
#define  SG_CUCD5		554
#define  SG_CUCD6		555
#define  SG_CUV4		556
#define  SG_CUV5		557
#define  SG_CUV6		558
#define  SG_CUV7		559
#define  SG_CUV12		560
#define  SG_CUCD8		561
#define  SG_CUV13		562
#define  SG_CUCD9		563
#define  SG_CUCD10		564
#define  SG_CUV8		565
#define  SG_CUCD11		566
#define  SG_CUCD12		567
#define  SG_CUCD13		568
#define  SG_CUV9		569
#define  SG_CUCD14		570
#define  SG_CUV10		571
#define  SG_CUV11		572

/* The following message related defines are from cntrl.c */ 
/* cntrl.c defines start at 600 */

#define MSG_CNTRL1 		600
#define MSG_CNTRL2 		601
#define MSG_CNTRL3 		602
#define MSG_CNTRL4		603
#define MSG_CNTRL5 		604
#define MSG_CNTRL6 		605
#define MSG_CNTRL7		606
#define MSG_CNTRL8		607
#define MSG_CNTRL9 		608
#define MSG_CNTRLCD1		609
#define MSG_CNTRL10		610
#define MSG_CNTRL11		611
#define MSG_CNTRL12 		612
#define MSG_CNTRL13 		613
#define MSG_CNTRL14 		614
#define MSG_CNTRL15 		615
#define MSG_CNTRL16 		616
#define MSG_CNTRL17 		617
#define MSG_CNTRLCD2 		619
#define MSG_CNTRL18		620
#define MSG_CNTRLCD3 		621
#define MSG_CNTRL20 		622
#define MSG_CNTRL22		623
#define MSG_CNTRL23 		624
#define MSG_CNTRL24		625
#define MSG_CNTRL26		626
#define MSG_CNTRL27		627
#define MSG_CNTRL29 		628
#define MSG_CNTRL30		629
#define MSG_CNTRL31		630
#define MSG_CNTRL33		631
#define MSG_CNTRL35		632
#define MSG_CNTRL37 		633
#define MSG_CNTRL39		634
#define MSG_CNTRL40		635
#define MSG_CNTRL19		636
#define MSG_CNTRL21 		637
#define MSG_CNTRL41		638
#define MSG_CNTRL42 		639
#define MSG_CNTRL43		640
#define MSG_CNTRL44 		641
#define MSG_CNTRL45 		642
#define MSG_CNTRL46 		643
#define MSG_CNTRL47 		644
#define MSG_CNTRL48 		645
#define MSG_CNTRL49		646
#define MSG_CNTRL50 		647
#define MSG_CNTRL51 		648
#define MSG_CNTRL52		649
#define MSG_CNTRL53		650
#define MSG_CNTRL54		652

/* The following message related defines are from gnxseq.c */ 
/* gnxseq.c defines start at 675 */

#define MSG_GX1 		675
#define MSG_GX2 		676

/* The following message related defines are from gtcfile.c */ 
/* gtcfile.c defines start at 690 */

#define MSG_GTC1 		690
#define MSG_GTC2		691
#define MSG_GTC3		692 

/* The following message related defines are from gename.c */ 
/* gename.c defines start at 700 */

#define MSG_GE1 		700
#define MSG_GE2 		702
#define MSG_GE3 		703
#define MSG_GE4 		704
#define MSG_GE5 		705

/* The following message related defines are from mailst.c */ 
/* mailst.c defines start at 750 */

#define MSG_MAILST1 		750
#define MSG_MAILST2 		751
#define MSG_MAILST3 		752
#define MSG_MAILST4		753

/* The following message related defines are from pk0.c */ 
/* pk0.c defines start at 775 */

#define MSG_PK0_1  		775
#define MSG_PK0_2  		776
#define MSG_PK0_3  		777
#define MSG_PK0_4  		778
#define MSG_PK0_7  		779
#define MSG_PK0_5  		780
#define MSG_PK0_6  		781

/* The following message related defines are from pk1.c */ 
/* pk1.c defines start at 800 */		
		
#define MSG_PK1_1   		801
#define MSG_PK1_2   		802
#define MSG_PK1_3   		803
#define MSG_PK1_4   		804
#define MSG_PK1_5   		805
#define MSG_PK1_6   		806
#define MSG_PK1_7   		806
#define MSG_PK1_8   		808
#define MSG_PK1_9   		809
#define MSG_PK1_10   		810
#define MSG_PK1_11   		812
#define MSG_PK1_12   		813
#define MSG_PK1_13   		814
#define MSG_PK1_14   		815
#define MSG_PK1_15   		816
#define MSG_PK1_16   		817
#define MSG_PK1_17   		818
#define MSG_PK1_18   		819
#define MSG_PK1_19   		820
#define MSG_PK1_20  		821
#define MSG_PK1_21   		822
#define MSG_PK1_23   		823
#define MSG_PK1_22   		824

/* The following message related defines are from shio.c */ 
/* shio.c defines start at 840 */		

#define MSG_SHIO1 		840

/* The following message related defines are from tio.c */ 
/* tio.c defines start at 875 */		

#define MSG_TIO1  		875
#define MSG_TIO2  		876
#define MSG_TIO3  		877
#define MSG_TIO4  		878
#define MSG_TIO5  		879
#define MSG_TIO6  		880

/* The following message related defines are from permission.c */ 
/* permission.c defines start at 900 */		

#define  MSG_PERM1 	 	900
#define  MSG_PERM2 	  	901
#define  MSG_PERM3 	 	902
#define  MSG_PERM4 	 	903
#define  MSG_PERM5 	 	904
#define  MSG_PERM6 	 	905
#define  MSG_PERM7 	 	906
#define  MSG_PERM8 	 	907
#define  MSG_PERM9 	 	908
#define  MSG_PERM10 	 	909
#define  MSG_PERM11 	 	910
#define  MSG_PERM12 	 	911
#define  MSG_PERM13 	 	912
#define  MSG_PERM14 	 	913
#define  MSG_PERM15 	 	914
#define  MSG_PERM16 		915
#define  MSG_PERM17 		916
#define  MSG_PERM18 	 	917
#define  MSG_PERM19 	 	918
#define  MSG_PERM20 	 	919
#define  MSG_PERM21 	 	920

/* The following message related defines are from sys5uux.c */ 
/* sys5uux.c defines start at 875 */		

#define MSG_SYS5_1 		875
#define MSG_SYS5_2 		876
#define MSG_SYS5_3 		877

/* The following message related defines are from systat.c */ 
/* systat.c defines start at 900 */		

#define MSG_SYSTAT_A1 		900
#define MSG_SYSTAT_L1 		901
#define MSG_SYSTAT_L2 		902
#define MSG_SYSTAT_D1 		904

/* The following message related defines are from uucleanup.c */ 
/* uucleanup.c defines start at 950 */		
 
#define MSG_UCL_1	 	950
#define MSG_UCL_2	 	951
#define MSG_UCL_3	 	952
#define MSG_UCL_4	 	953
#define MSG_UCL_5	 	954
#define MSG_UCL_6	 	955
#define MSG_UCL_7	 	956
#define MSG_UCL_8	 	957
#define MSG_UCL_9	 	958
#define MSG_UCL_10	 	959
#define MSG_UCL_11 	 	960
#define MSG_UCL_12	 	961
#define MSG_UCL_13	 	962
#define MSG_UCL_14	 	963
#define MSG_UCL_15	 	964
#define MSG_UCL_16	 	965
#define MSG_UCL_17	 	966
#define MSG_UCL_18	 	967
#define MSG_UCL_19	 	968
#define MSG_UCL_20		969
#define MSG_UCL_21	 	970
#define MSG_UCL_22	 	971
#define MSG_UCL_23	 	972
#define MSG_UCL_24	 	973

/* The following message related defines are from uucp.c */ 
/* uucp.c defines start at 1000 */		

#define MSG_UUCP1	 	1000
#define MSG_UUCP2	 	1001
#define MSG_UUCP3	 	1002
#define MSG_UUCP4	 	1003
#define MSG_UUCP5	 	1004
#define MSG_UUCP6	 	1005
#define MSG_UUCP7	 	1006
#define MSG_UUCP8	 	1007
#define MSG_UUCP9	 	1008
#define MSG_UUCP10	 	1009
#define MSG_UUCP11	 	1010
#define MSG_UUCP12	 	1011
#define MSG_UUCP13	 	1012
#define MSG_UUCP14	 	1013
#define MSG_UUCP15	 	1014
#define MSG_UUCP16	 	1015
#define MSG_UUCP17	 	1016
#define MSG_UUCP18	 	1017
#define MSG_UUCP19	 	1018

/* The following message related defines are from uucpdefs.c */ 
/* uucpdefs.c defines start at 1050 */		

#define MSG_UDEFS_1	 	1050
#define MSG_UDEFS_2	 	1051
#define MSG_UDEFS_3	 	1052
#define MSG_UDEFS_4	 	1053
#define MSG_UDEFS_5	 	1054
#define MSG_UDEFS_6	 	1055
#define MSG_UDEFS_7	 	1056
#define MSG_UDEFS_8	 	1057
#define MSG_UDEFS_9	 	1058
#define MSG_UDEFS_10	 	1059
#define MSG_UDEFS_11	 	1060
#define MSG_UDEFS_12	 	1061
#define MSG_UDEFS_13	 	1062
#define MSG_UDEFS_14	 	1063
#define MSG_UDEFS_15	 	1064
#define MSG_UDEFS_16	 	1065
#define MSG_UDEFS_17	 	1066


/* The following message related defines are from uuname.c */ 
/* uuname.c defines start at 1090 */		

#define MSG_UUNAME1 	1090
#define MSG_UUNAME2 	1091

/* The following message related defines are from uusched.c */ 
/* uusched.c defines start at 1100 */		

#define MSG_UUSCHED1 	1100
#define MSG_UUSCHED2 	1101
#define MSG_UUSCHED3 	1102

/* The following message related defines are from uustat.c */ 
/* uustat.c defines start at 1150 */		

#define MSG_UUSTAT1	 1150
#define MSG_UUSTAT2	 1151
#define MSG_UUSTAT3	 1152
#define MSG_UUSTAT4	 1153
#define MSG_UUSTAT5	 1154
#define MSG_UUSTAT6	 1155
#define MSG_UUSTAT7	 1156
#define MSG_UUSTAT8	 1157
#define MSG_UUSTAT9	 1158
#define MSG_UUSTAT10	 1159
#define MSG_UUSTAT11	 1160
#define MSG_UUSTAT12	 1161
#define MSG_UUSTAT13	 1162
#define MSG_UUSTAT16	 1163
#define MSG_UUSTAT17	 1164
#define MSG_UUSTAT18	 1165
#define MSG_UUSTAT19	 1166
#define MSG_UUSTAT_E1	 1167

/* The following message related defines are from uux.c */ 
/* uux.c defines start at 1200 */		

#define MSG_UUX_1	 	1201
#define MSG_UUX_2	 	1202
#define MSG_UUX_3	 	1203
#define MSG_UUX_4	 	1204
#define MSG_UUX_5	 	1205
#define MSG_UUX_6	 	1206
#define MSG_UUX_7	 	1207
#define MSG_UUX_8	 	1208
#define MSG_UUX_9	 	1209
#define MSG_UUX_11	 	1210
#define MSG_UUX_12	 	1211
#define MSG_UUX_13	 	1212
#define MSG_UUX_14	 	1213
#define MSG_UUX_15	 	1214

/* The following message related defines are from uuxqt.c */ 
/* uuxqt.c defines start at 1250 */		

#define MSG_UUXQT_1	 1250
#define MSG_UUXQT_2	 1251
#define MSG_UUXQT_3	 1252
#define MSG_UUXQT_4	 1253
#define MSG_UUXQT_5	 1254
#define MSG_UUXQT_6	 1255
#define MSG_UUXQT_7	 1256
#define MSG_UUXQT_8	 1257
#define MSG_UUXQT_9	 1258
#define MSG_UUXQT_10 	 1259

/* The following message related define is from versys.c */ 
/* versys.c defines start at 1300 */		

#define MSG_VER_1  	1300

/* The following message related defines are from getopt.c */ 
/* getopt.c defines start at 1325 */		

#define MSG_GOPT_1 	1325
#define MSG_GOPT_2 	1326
