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
 * @(#)$RCSfile: ncl_interface_def.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:03:37 $
 */
/* File: NCL_INTERFACE_DEF.H */
#define NCL_DSMDT_Boolean  		1
#define NCL_DSMDT_Integer  		2
#define NCL_DSMDT_OctetString  		3
#define NCL_DSMDT_BitString  		4
#define NCL_DSMDT_Null  		5
#define NCL_DSMDT_ObjectID  		6
#define NCL_DSMDT_Sequence  		7
#define NCL_DSMDT_Set  			8

/* DSM-defined data types (Entity Model Ch.8) */
#define NCL_DSMDT_Integer8  		109
#define NCL_DSMDT_Integer16  		110
#define NCL_DSMDT_Integer32  		111
#define NCL_DSMDT_Integer64  		112
#define NCL_DSMDT_Unsigned  		113
#define NCL_DSMDT_Unsigned8  		114
#define NCL_DSMDT_Unsigned16  		115
#define NCL_DSMDT_Unsigned32  		116
#define NCL_DSMDT_Unsigned64  		117
#define NCL_DSMDT_Counter16  		118
#define NCL_DSMDT_Counter32  		119
#define NCL_DSMDT_Counter48  		120
#define NCL_DSMDT_Counter64  		121
#define NCL_DSMDT_LCounter16  		122
#define NCL_DSMDT_LCounter32  		123
#define NCL_DSMDT_Octet  		124

/* DSM-defined and DSSR-registered data types */
#define NCL_DSMDT_CharAbsTime  		16  
#define NCL_DSMDT_BinAbsTime  		17
#define NCL_DSMDT_CharRelTime  		18
#define NCL_DSMDT_BinRelTime  		19
#define NCL_DSMDT_Latin1String  	20
#define NCL_DSMDT_SimpleName  		26
#define NCL_DSMDT_FullName  		27
#define NCL_DSMDT_UID  			28  /* DNA UID */
#define NCL_DSMDT_Known  		29  /* for wildcarding */
#define NCL_DSMDT_FullEntityName  	30  
#define NCL_DSMDT_LocalEntityName 	31
#define NCL_DSMDT_DefaultValue  	32
#define NCL_DSMDT_Version  		33
#define NCL_DSMDT_ID802  		34
#define NCL_DSMDT_DTEAddress  		35
#define NCL_DSMDT_FileSpec  		36
#define NCL_DSMDT_NSAPAddress  		37
#define NCL_DSMDT_NetEntTitle  		38
#define NCL_DSMDT_AreaAddress  		39
/* next three should be Counter16 thru Counter64 */
#define NCL_DSMDT_AddressPrefix  	43
#define NCL_DSMDT_TowerSet  		44
#define NCL_DSMDT_EndUserSpec  		45  
#define NCL_DSMDT_TSelector  		46	/* transport selector */
#define NCL_DSMDT_Phase4Name  		47
#define NCL_DSMDT_Phase4Address  	48
#define NCL_DSMDT_Implementation  	49
#define NCL_DSMDT_VersionEdit  		50
#define NCL_DSMDT_ComponentName  	51
#define NCL_DSMDT_NSCTS  		52  	/* DEC Name Service UID */
#define NCL_DSMDT_EntityClass  		53
#define NCL_DSMDT_HexString  		54
#define NCL_DSMDT_Floor 		55	/* part of a towerset */
#define NCL_DSMDT_ProtocolTower 	56	/* part of a towerset */
#define NCL_DSMDT_SSelector 		57	/* SessionSelector */
#define NCL_DSMDT_DNACMIPMessage 	58	/* for CMIP script processing*/
#define NCL_DSMDT_PSelector  		59	/* PresentationSelector */
#define NCL_DSMDT_EntityClassEvent  	61	/* see DSSR registry */
#define NCL_DSMDT_EntityInstanceEvent  	62	/* see DSSR registry */
#define NCL_DSMDT_FDDITimer  		63	/* see DSSR */
#define NCL_DSMDT_StationID  		64	/* see DSSR */
#define NCL_DSMDT_BinSimpleRelTime  	65	/* see DSSR */
#define NCL_DSMDT_EthernetProtocolType  66  	/* see DSSR registry */
#define NCL_DSMDT_IEEE802SNAPPID  	67	/* see DSSR */
#define NCL_DSMDT_IPAddress  		81	/* see DSSR */

/* meta-types used to create constructed types */
#define NCL_DSMDT_Range  		200
#define NCL_DSMDT_Enumeration 		201
#define NCL_DSMDT_BitSet  		202
#define NCL_DSMDT_Record  		203
#define NCL_DSMDT_SetOf  		204
#define NCL_DSMDT_SequenceOf  		205
