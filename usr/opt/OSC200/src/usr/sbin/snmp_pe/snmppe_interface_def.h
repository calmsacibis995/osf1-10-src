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
 * @(#)$RCSfile: snmppe_interface_def.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:24:52 $
 */
/*
**  Copyright (c) Digital Equipment Corporation, 1991, 1992
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
*/

/*
  October 1991 - Converted for use with the SNMP Protocol Engine of
                 the Common Agent.  Filename changed to avoid conflict with
                 MCC file of the name "mcc_interface_def.h".

NOTE: The comments containing '!!' denote changes made for CA V1.1 port 
      to Alpha.

*/

#include "moss_asn1.h"   /* needed for MCC_K_ASN_DT_SMI_IPADDR support */
#include "moss_inet.h"   /* needed for MCC_K_ASN_DT_SMI_IPADDR support */


/*** MODULE MCC_INTERFACE_DEF IDENT T1.1.0 ***/
#define MCC_INTERFACE_DEF 1
typedef char MCC_T_Integer8;
typedef short int MCC_T_Integer16;
typedef int MCC_T_Integer32;                  /* !! */
typedef int MCC_T_Integer64 [2];
typedef unsigned char MCC_T_Unsigned8;
typedef unsigned short int MCC_T_Unsigned16;
typedef unsigned int MCC_T_Unsigned32;
typedef unsigned int MCC_T_Unsigned64 [2];
typedef unsigned short int MCC_T_Counter16;
typedef unsigned int MCC_T_Counter32;         /* !! */
typedef unsigned int MCC_T_Counter64 [2];
typedef unsigned int MCC_T_Counter48 [2];
typedef unsigned short int MCC_T_Lcounter16;
typedef unsigned int MCC_T_Lcounter32;        /* !! */
typedef unsigned char MCC_T_Boolean;
typedef unsigned int MCC_T_Enumeration;       /* !! */
typedef unsigned int MCC_T_Component;         /* !! */
typedef char MCC_T_Phase4Name [6];
typedef unsigned int MCC_T_VMSError;          /* !! */
typedef unsigned int MCC_T_MCCError;          /* !! */
typedef unsigned char MCC_T_Octet [1];
typedef unsigned char MCC_T_OctetString [65535];
typedef unsigned char MCC_T_Latin1String [65535];
typedef unsigned char MCC_T_FullName [404];
typedef unsigned char MCC_T_ID802 [6];
typedef unsigned char MCC_T_ID802_SNAP [5];
typedef unsigned char MCC_T_ID802_SAP [1];
typedef unsigned char MCC_T_IDENETV2 [2];
typedef unsigned char MCC_T_UID [16];
typedef unsigned char MCC_T_HexString [65535];
#define MCC_K_HEX_EVEN 0
#define MCC_K_HEX_ODD 4
typedef unsigned char MCC_T_FileSpec [255];
typedef unsigned char MCC_T_DirectorySpec [255];
typedef unsigned char MCC_T_CharAbsTime [36];
typedef unsigned char MCC_T_CharRelTime [36];
typedef unsigned char MCC_T_DTEAddress [15];
typedef unsigned char MCC_T_NSAP [41];
typedef unsigned char MCC_T_NET [40];
typedef unsigned char MCC_T_AreaAddress [34];
typedef unsigned char MCC_T_AddressPrefix [40];
typedef unsigned char MCC_T_TSEL [32];
typedef unsigned int MCC_T_EntityClass [10];         /* !! */
typedef unsigned char MCC_T_Expression [65535];
typedef float MCC_T_FloatF;
typedef unsigned int MCC_T_Real [2];
typedef unsigned char MCC_T_IPAddress [4];
typedef unsigned int MCC_T_ONCE [4];                 /* !! */
typedef struct _MCC_T_FW_HANDLE {
    char *MCC_Ta_fwhfld1;
    char *MCC_Ta_fwhfld2;
    } MCC_T_FW_HANDLE;
typedef MCC_T_FW_HANDLE MCC_T_MUTEX;
typedef MCC_T_FW_HANDLE MCC_T_CONDVAR;
typedef MCC_T_FW_HANDLE MCC_T_THREAD;
typedef struct _MCC_T_NSCTS {
    MCC_T_ID802 nscts_b_node;
    int nscts_q_time [2];
    } MCC_T_NSCTS;
typedef struct _MCC_T_SimpleName {
    unsigned char sn_b_type;
    unsigned char sn_b_count;
    unsigned char sn_b_string [255];
    } MCC_T_SimpleName;
typedef unsigned char MCC_T_InternetName [255];
typedef unsigned int MCC_T_AttribIdentifier [11];    /* !! */
typedef struct _MCC_T_Time24 {
    unsigned char tm24_b_hour;
    unsigned char tm24_b_minute;
    } MCC_T_Time24;
typedef unsigned int MCC_T_UsageState;               /* !! */
typedef unsigned int MCC_T_AdminState;               /* !! */
typedef unsigned int MCC_T_OperState;                /* !! */
typedef unsigned char MCC_T_SNARESNAME [8];
typedef unsigned char MCC_T_QUALSNANAME [17];
#define MCC_K_SN_NULL 0
#define MCC_K_SN_NORMAL 1
#define MCC_K_SN_QUOTED 2
#define MCC_K_SN_BINARY 3
#define MCC_K_SN_WILDCARD 4
#define MCC_K_SN_ELLIPSIS 5
#define bitset_m_bit1 1
#define bitset_m_bit2 2
#define bitset_m_bit3 4
#define bitset_m_bit4 8
#define bitset_m_bit5 16
#define bitset_m_bit6 32
#define bitset_m_bit7 64
#define bitset_m_bit8 128
#define bitset_m_bit9 256
#define bitset_m_bit10 512
#define bitset_m_bit11 1024
#define bitset_m_bit12 2048
#define bitset_m_bit13 4096
#define bitset_m_bit14 8192
#define bitset_m_bit15 16384
#define bitset_m_bit16 32768
#define bitset_m_bit17 65536
#define bitset_m_bit18 131072
#define bitset_m_bit19 262144
#define bitset_m_bit20 524288
#define bitset_m_bit21 1048576
#define bitset_m_bit22 2097152
#define bitset_m_bit23 4194304
#define bitset_m_bit24 8388608
#define bitset_m_bit25 16777216
#define bitset_m_bit26 33554432
#define bitset_m_bit27 67108864
#define bitset_m_bit28 134217728
#define bitset_m_bit29 268435456
#define bitset_m_bit30 536870912
#define bitset_m_bit31 1073741824
#define bitset_m_bit32 -2147483648
typedef struct _MCC_T_BitSet {
    unsigned bitset_v_bit1 : 1;
    unsigned bitset_v_bit2 : 1;
    unsigned bitset_v_bit3 : 1;
    unsigned bitset_v_bit4 : 1;
    unsigned bitset_v_bit5 : 1;
    unsigned bitset_v_bit6 : 1;
    unsigned bitset_v_bit7 : 1;
    unsigned bitset_v_bit8 : 1;
    unsigned bitset_v_bit9 : 1;
    unsigned bitset_v_bit10 : 1;
    unsigned bitset_v_bit11 : 1;
    unsigned bitset_v_bit12 : 1;
    unsigned bitset_v_bit13 : 1;
    unsigned bitset_v_bit14 : 1;
    unsigned bitset_v_bit15 : 1;
    unsigned bitset_v_bit16 : 1;
    unsigned bitset_v_bit17 : 1;
    unsigned bitset_v_bit18 : 1;
    unsigned bitset_v_bit19 : 1;
    unsigned bitset_v_bit20 : 1;
    unsigned bitset_v_bit21 : 1;
    unsigned bitset_v_bit22 : 1;
    unsigned bitset_v_bit23 : 1;
    unsigned bitset_v_bit24 : 1;
    unsigned bitset_v_bit25 : 1;
    unsigned bitset_v_bit26 : 1;
    unsigned bitset_v_bit27 : 1;
    unsigned bitset_v_bit28 : 1;
    unsigned bitset_v_bit29 : 1;
    unsigned bitset_v_bit30 : 1;
    unsigned bitset_v_bit31 : 1;
    unsigned bitset_v_bit32 : 1;
    } MCC_T_BitSet;
typedef struct _MCC_T_Phase4Address {
    union  {
        unsigned short int p4a_W_Phase4Address;
        struct  {
            unsigned p4a_V_LocalAddress : 10;
            unsigned p4a_V_AreaAddress : 6;
            } p4a_r_fill_1;
        } p4a_r_fill_0;
    } MCC_T_Phase4Address;
typedef struct _MCC_T_BinAbsTim {
    int bat_q_utc [2];
    unsigned char bat_b_inacc [6];
    unsigned bat_v_tdf : 12;
    unsigned bat_v_vers : 4;
    } MCC_T_BinAbsTim;
typedef struct _MCC_T_BinRelTim {
    int brt_q_utc [2];
    unsigned char brt_b_inacc [6];
    unsigned brt_v_tdf : 12;
    unsigned brt_v_vers : 4;
    } MCC_T_BinRelTim;
typedef struct _MCC_T_Version {
    char ver_t_level [1];
    MCC_T_Unsigned8 ver_b_major;
    MCC_T_Unsigned8 ver_b_minor;
    MCC_T_Unsigned8 ver_b_eco;
    } MCC_T_Version;
typedef struct _MCC_T_EndUserSpec {
    unsigned char eus_b_format;
    unsigned char eus_B_ApplicType;
    union  {
        unsigned char eus_b_description [17];
        struct  {
            unsigned short int eus_w_groupCode;
            unsigned short int eus_w_userCode;
            unsigned char eus_b_descrip [13];
            } eus_r_type2;
        unsigned char eus_b_name [404];
        } eus_r_spec;
    } MCC_T_EndUserSpec;
#define MCC_K_EUS_NUMBER 0
#define MCC_K_EUS_NAME 1
#define MCC_K_EUS_UIC 2
#define MCC_K_EUS_FULLNAME 3
typedef unsigned int *MCC_A_HANDLE;         /* !! */
typedef unsigned int *MCC_A_AES;            /* !! */
typedef unsigned int *MCC_A_TIMEFRAME;      /* !! */
typedef unsigned char MCC_T_ILV;
typedef unsigned char MCC_T_ASN;
typedef unsigned int MCC_T_IDCode;          /* !! */
typedef unsigned int MCC_T_CVR;             /* !! */
typedef unsigned int MCC_T_UNSLONG;         /* !! */
typedef unsigned short int MCC_T_UNSSHORT;
typedef long int MCC_T_LONG;
typedef int MCC_T_INT;                      /* !! */
typedef short int MCC_T_SHORT;
typedef char MCC_T_CHAR;
typedef unsigned char MCC_T_UNSBYTE;
typedef char MCC_T_BYTE;
typedef struct _MCC_R_Mesg_List {
    unsigned short int mesg_w_count;
    unsigned short int mesg_w_id;
    MCC_T_CVR *mesg_A_CVR;
    unsigned int *mesg_A_Directive;         /* !! */
    MCC_A_AES mesg_a_out_Entity;
    int *mesg_a_out_P;
    } MCC_R_Mesg_List;
typedef struct _MCC_R_Reply_List {
    unsigned short int rpl_w_count;
    unsigned short int rpl_w_id;
    MCC_T_Component *rpl_A_Component;
    MCC_T_Version *rpl_A_Version;
    MCC_T_CVR *rpl_A_CVR;
    unsigned int *rpl_A_Directive;         /* !! */
    MCC_A_AES rpl_a_in_Entity;
    MCC_T_IDCode *rpl_A_Attribute;
    int *rpl_a_in_Q;
    int *rpl_a_in_P;
    MCC_A_AES rpl_a_out_Entity;
    MCC_T_BinAbsTim *rpl_A_Timestamp;
    int *rpl_a_out_P;
    int *rpl_a_out_Q;
    } MCC_R_Reply_List;
typedef struct _MCC_T_Descriptor {
    unsigned short int mcc_w_maxstrlen;
    unsigned char mcc_b_dtype;
    unsigned char mcc_b_class;
    unsigned char *mcc_a_pointer;
    unsigned short int mcc_w_curlen;
    unsigned char mcc_b_flags;
    unsigned char mcc_b_ver;
    unsigned int mcc_l_id;                 /* !! */
    unsigned int mcc_l_dt;                 /* !! */
    struct _MCC_T_Descriptor *mcc_a_link;
    } MCC_T_Descriptor;
/* #define MCC_S_DESCRIPTOR 24 -- OLD */
/* #define MCC_S_DESCRIPTOR 32 -- NEW, but use def'n below: */
#define MCC_S_DESCRIPTOR sizeof(struct _MCC_T_Descriptor)


#define MCC_K_VER_DESCRIPTOR 1
#define MCC_M_FLAGS_DEFAULT 1
#define MCC_K_NULL_PTR 0
#define MCC_K_OFF 0
#define MCC_K_ON 1
#define MCC_K_FALSE 0
#define MCC_K_TRUE 1
#define MCC_K_FUNCTION 15
#define MCC_K_ACCESS 16
#define MCC_K_QUAL_VIA_PATH 1
#define MCC_K_QUAL_VIA_PORT 2
#define MCC_K_QUAL_IN_DOMAIN 3
#define MCC_K_QUAL_BY_ACCT 4
#define MCC_K_QUAL_BY_PASS 5
#define MCC_K_QUAL_BY_USER 6
#define MCC_K_QUAL_VIA_MANAGER 7
#define MCC_K_PERS_TMP_PROB 0
#define MCC_K_PERS_UNCL_PROB 1
#define MCC_K_PERS_PERM_PROB 2
#define MCC_K_EEXIS_EXISTS 0
#define MCC_K_EEXIS_EXCANNOTBEDET 1
#define MCC_K_EEXIS_INACCESS 2
#define MCC_K_EEXIS_UNKNOWN 3
#define MCC_K_EEXIS_NON_EXIS 4
#define MCC_K_REASN_SUCC_OP 0
#define MCC_K_REASN_NO_SUCH_ATTR 1
#define MCC_K_REASN_ATTR_NOT_GET 2
#define MCC_K_REASN_ATTR_NOT_AVAIL 3
#define MCC_K_REASN_NOT_SETVALATTR 4
#define MCC_K_REASN_INV_VAL 5
#define MCC_K_REASN_ATTR_NOTSET 6
#define MCC_K_REASN_PART_ATM 7
#define MCC_K_REASN_ACC_DEN 8
#define MCC_K_REASN_CONS_VIOL 9
#define MCC_K_REASN_DUPL_VAL 10
#define MCC_K_REASN_ATTR_OP_FAIL 255
#define MCC_K_SETABLE 0
#define MCC_K_NONSETABLE 1
#define MCC_K_WRITEONLY 2
#define MCC_K_CDT_FNAME 1
#define MCC_K_CDT_FCODE 2
#define MCC_K_CDT_FDT 3
#define MCC_K_CDT_VFIELD 4
#define MCC_K_CDT_SELFLD 5
#define MCC_K_CDT_SELVAL 6
#define MCC_K_CDT_LOWLIM 7
#define MCC_K_CDT_HIGHLIM 8
#define MCC_K_CDT_SUBFIELDS 9
#define MCC_K_DEPENDS_OPERATION 0
#define MCC_K_DEPENDS_CHAR_ATTR 1
#define MCC_K_DEPENDS_VALUES 2
#define MCC_K_VAR_OP_EQUAL 0
#define MCC_K_VAR_OP_IN_SET 1
#define MCC_K_INVCND_REPLY 0
#define MCC_K_INVCND_MCCERROR 1
#define MCC_K_INVCND_VMSERROR 2
#define MCC_K_ARG_ENT_EXIST 5
#define MCC_K_ARG_ENT_EXISTS 5
#define MCC_K_ARG_MCC_ERR 2
#define MCC_K_ARG_PROB_PERSIST 1
#define MCC_K_ARG_QUAL_INV_ARG 10
#define MCC_K_ARG_QUAL_REQ_QUAL 13
#define MCC_K_ARG_SVC_ERR 4
#define MCC_K_ARG_UNK_ENTITY 6
#define MCC_K_ARG_UNREC_ARG_CODE 7
#define MCC_K_ARG_UNREC_QUAL_CODE 9
#define MCC_K_ARG_UNREG_ENTITY 12
#define MCC_K_ARG_UNSUPP_QUAL_ID 8
#define MCC_K_ARG_VMS_ERR 3
#define MCC_K_ARG_SVC_MSG_ERR 11
#define MCC_K_BUGCHECK 1
#define MCC_K_CANNOT_COMMUNICATE 4
#define MCC_K_CANNOT_COMPLETE 2
#define MCC_K_CLASS_COMMON 52
#define MCC_K_DIRECT_REQUIRES_QUAL 22
#define MCC_K_INT_COMMUNICATION 6
#define MCC_K_XXX_CANNOT_COMPLETE 19
#define MCC_K_NOT_REGISTERED 20
#define MCC_K_NO_RESOURCE_ENTITY 8
#define MCC_K_NO_RESOURCE_PROVIDER 7
#define MCC_K_NO_SUCH_ENTITY 3
#define MCC_K_OP_FAILED 11
#define MCC_K_OP_ILLEGAL 9
#define MCC_K_OP_NOT_ACCEPTED 10
#define MCC_K_PARTIAL_INFO 21
#define MCC_K_PROTOCOL_ERR 5
#define MCC_K_SCOPE_REQUIRES_QUAL 23
#define MCC_K_UNREC_ARG 15
#define MCC_K_UNREC_QUAL 17
#define MCC_K_UNSUPP_ENT_FILTER 13
#define MCC_K_UNSUPP_ENT_WILDCARD 12
#define MCC_K_UNSUPP_QUAL 16
#define MCC_K_UNSUPP_QUAL_ARG 18
#define MCC_K_UNSUPP_TIME_SCOPE 14
#define MCC_K_DD_VALUE_DATA_TYPE 1
#define MCC_K_DD_VALUE_LENGTH 2
#define MCC_K_DD_DEFAULT_ALLOWED 3
#define MCC_K_DD_VALUE_DEFAULT 4
#define MCC_K_DD_VALUE_REQUIRED 5
#define MCC_K_DD_PRESENTATION_NAME 6
#define MCC_K_DD_ACCESS 7
#define MCC_K_DD_CATEGORIES 8
#define MCC_K_DD_DISPLAY 9
#define MCC_K_DD_DYNAMIC 10
#define MCC_K_DD_HIDDEN 11
#define MCC_K_DD_INSTANCE_REQUIRED 12
#define MCC_K_DD_UNITS 13
#define MCC_K_DD_DEFAULT 14
#define MCC_K_DD_DIRECT_SUPPORTED 15
#define MCC_K_DD_ATTRIBUTE_LIST 16
#define MCC_K_DD_INSTANCE_DATATYPE 17
#define MCC_K_DD_TRANSLATION_TABLE 18
#define MCC_K_DD_REPLY_TEXT 19
#define MCC_K_DD_DIRECTIVE_TYPE 20
#define MCC_K_DD_ECHO 21
#define MCC_K_DD_COUNTED_AS 22
#define MCC_K_DD_CONSTRUCTOR_DATA_TYPE 23
#define MCC_K_DD_DNS_IDENT 24
#define MCC_K_DD_EVENT_LIST 25
#define MCC_K_DD_DEPENDS_ON 26
#define MCC_K_DD_PREDICTABLE 27
#define MCC_K_DD_VARIANT_SELECTOR 28
#define MCC_K_DD_OID 29
#define MCC_K_DD_SNMP_OID 30
#define MCC_K_DD_DNA_CMIP_INT 31
#define MCC_K_DD_OSI_CMIP_OID 32
#define MCC_K_DD_SELECTOR_INFO 1001
#define MCC_K_DD_SUBRANGE_INFO 1002
#define MCC_K_AES_NOT_WILD 1
#define MCC_K_AES_CLASS_WILD 2
#define MCC_K_AES_INSTANCE_FULL 3
#define MCC_K_AES_INSTANCE_PARTIAL 4
#define MCC_K_DD_CM_CHILD 128
#define MCC_K_DD_CM_STRUCT 129
#define MCC_K_DD_INITIAL_CODE 130
#define MCC_K_DD_NATIVE_ATTR_GROUP 131
#define MCC_K_DD_NATIVE_CODE 132
#define MCC_K_DD_NATIVE_DATA_TYPE 133
#define MCC_K_DD_NATIVE_LENGTH 134
#define MCC_K_DD_PARENT_ID 135
#define MCC_K_DEF_NO 0
#define MCC_K_DEF_VALUE 1
#define MCC_K_DEF_IMPL_SPEC 2
#define MCC_K_DIR_EXAMINE 0
#define MCC_K_DIR_MODIFY 1
#define MCC_K_DIR_ACTION 2
#define MCC_K_DIR_EVENT 3
#define MCC_K_DNS_ID_PRIMARY 0
#define MCC_K_DNS_ID_ALTERNATE 1
#define MCC_K_DNS_ID_NOT_USED 2
#define MCC_K_OSI_CONFIG 0
#define MCC_K_OSI_FAULT 1
#define MCC_K_OSI_PERFORMANCE 2
#define MCC_K_OSI_SECURITY 3
#define MCC_K_OSI_ACCOUNTING 4
#define MCC_K_OSI_ALL 5
#define MCC_K_HANDLE_FIRST 0
#define MCC_K_HANDLE_MORE 1
#define MCC_K_HANDLE_CANCEL 3
#define MCC_K_DOM_DYNAMIC 0
#define MCC_K_DOM_STATIC 1
#define MCC_K_DOM_POSSIBLE_CYCLE 2
#define MCC_K_ATS_FRAME 1
#define MCC_K_ATS_SCOPE 2
#define MCC_K_ATS_SCHEDULE 3
#define MCC_K_ATS_ABS 4
#define MCC_K_ATS_DEFER 5
#define MCC_K_NOP 0
#define MCC_K_OR 1
#define MCC_K_AND 2
#define MCC_K_XOR 3
#define MCC_K_NOT 4
#define MCC_K_ADD 5
#define MCC_K_SUBTRACT 6
#define MCC_K_NEGATE 7
#define MCC_K_MULTIPLY 8
#define MCC_K_DIVIDE 9
#define MCC_K_GTR 10
#define MCC_K_LSS 11
#define MCC_K_EQL 12
#define MCC_K_NEQ 13
#define MCC_K_GEQ 14
#define MCC_K_LEQ 15
#define MCC_K_MAX_OPERATOR 127
struct MCC_R_DICT_DEF {
    unsigned short int def_w_count;
    char def_b_defined;
    char def_b_usage;
    MCC_T_Descriptor def_r_value_desc;
    } ;
#define MCC_K_DICT_USAGE_STRING 0
#define MCC_K_DICT_USAGE_ILV 1
#define MCC_K_DICT_USAGE_RULE 2
#define MCC_K_DICT_USAGE_TLV 1
#define MCC_K_DICT_CLASS 1
#define MCC_K_DICT_SUBCLASS 2
#define MCC_K_DICT_DEFINITION 3
#define MCC_K_DICT_ARGUMENT 4
#define MCC_K_DICT_ATTRIBUTE 5
#define MCC_K_DICT_DIRECTIVE 6
#define MCC_K_DICT_REQUEST 7
#define MCC_K_DICT_RESPONSE 8
#define MCC_K_DICT_EXCEPTION 9
#define MCC_K_DICT_EVENT 10
#define MCC_K_DICT_ATTR_PARTITION 11
#define MCC_K_DICT_ATTR_GROUP 12
#define MCC_K_DICT_EVENT_PARTITION 13
#define MCC_K_DICT_EVENT_GROUP 14
#define MCC_K_DICT_MEMBER 15
#define MCC_K_DT_NULL 0
#define MCC_K_DT_BOOLEAN 1
#define MCC_K_DT_OCTET_STRING 2
#define MCC_K_DT_LATIN1STRING 3
#define MCC_K_DT_SIMPLE_NAME 4
#define MCC_K_DT_FULL_NAME 5
#define MCC_K_DT_FILE_SPEC 6
#define MCC_K_DT_UID 8
#define MCC_K_DT_VERSION 9
#define MCC_K_DT_ENUMERATION 10
#define MCC_K_DT_BITSET 11
#define MCC_K_DT_SUBRANGE 12
#define MCC_K_DT_RECORD 13
#define MCC_K_DT_VAR_RECORD 14
#define MCC_K_DT_SEQUENCE 15
#define MCC_K_DT_SET 16
#define MCC_K_DT_ADDRESS_NSAP 18
#define MCC_K_DT_ADDRESS_NET 19
#define MCC_K_DT_ADDRESS_AREA 20
#define MCC_K_DT_ADDRESS_PREFIX 21
#define MCC_K_DT_ADDRESS_TSEL 22
#define MCC_K_DT_ADDRESS_DTE 23
#define MCC_K_DT_PHASE4NAME 24
#define MCC_K_DT_PHASE4ADDRESS 25
#define MCC_K_DT_EXPRESSION 26
#define MCC_K_DT_COMPONENT 27
#define MCC_K_DT_IMPLEMENTATION 28
#define MCC_K_DT_INTEGER8 29
#define MCC_K_DT_INTEGER16 30
#define MCC_K_DT_INTEGER32 31
#define MCC_K_DT_INTEGER64 32
#define MCC_K_DT_UNSIGNED8 33
#define MCC_K_DT_UNSIGNED16 34
#define MCC_K_DT_UNSIGNED32 35
#define MCC_K_DT_UNSIGNED64 36
#define MCC_K_DT_COUNTER16 37
#define MCC_K_DT_COUNTER32 38
#define MCC_K_DT_COUNTER48 39
#define MCC_K_DT_COUNTER64 40
#define MCC_K_DT_LCOUNTER16 41
#define MCC_K_DT_LCOUNTER32 42
#define MCC_K_DT_VMS_ERROR 43
#define MCC_K_DT_MCC_ERROR 44
#define MCC_K_DT_OCTET 45
#define MCC_K_DT_HEXSTRING 46
#define MCC_K_DT_BIN_ABS_TIM 47
#define MCC_K_DT_CHAR_ABS_TIM 48
#define MCC_K_DT_BIN_REL_TIM 49
#define MCC_K_DT_CHAR_REL_TIM 50
#define MCC_K_DT_NSCTS 51
#define MCC_K_DT_SET_OF 52
#define MCC_K_DT_SEQUENCE_OF 53
#define MCC_K_DT_RANGE 54
#define MCC_K_DT_FULL_ENTITY 55
#define MCC_K_DT_LOCAL_ENTITY 56
#define MCC_K_DT_TOWERSET 57
#define MCC_K_DT_END_USER_SPEC 58
#define MCC_K_DT_MCC_REPLY 59
#define MCC_K_DT_ENTITY_CLASS 60
#define MCC_K_DT_DIRECTORY_SPEC 61
#define MCC_K_DT_BITSTRING 62
#define MCC_K_DT_FLOATF 63
#define MCC_K_DT_ATTRIB_LIST 66
#define MCC_K_DT_ATTRIBUTEIDLIST 67
#define MCC_K_DT_ID802_SNAP 68
#define MCC_K_DT_ID802_SAP 69
#define MCC_K_DT_IDENETV2_TYPE 70
#define MCC_K_DT_ID802 71
#define MCC_K_DT_REAL 72
#define MCC_K_DT_IPADDRESS 73
#define MCC_K_DT_INTERNET_NAME 74
#define MCC_K_DT_MCCMESSAGE 75
#define MCC_K_DT_RESERVED_DO_NOT_USE_1 76
#define MCC_K_DT_RESERVED_DO_NOT_USE_2 77
#define MCC_K_DT_RESERVED_DO_NOT_USE_3 78
#define MCC_K_DT_RESERVED_DO_NOT_USE_4 79
#define MCC_K_DT_EVENT_REPORT 80
#define MCC_K_DT_EVENTIDLIST 81
#define MCC_K_DT_ATTRIB 82
#define MCC_K_DT_ATTRIB_IDENTIFIER 83
#define MCC_K_DT_TIME24 84
#define MCC_K_DT_USAGE_STATE 85
#define MCC_K_DT_ADMIN_STATE 86
#define MCC_K_DT_REPAIR_STATUS 87
#define MCC_K_DT_INSTAL_STATUS 88
#define MCC_K_DT_AVAIL_STATUS 89
#define MCC_K_DT_CONTROL_STATUS 90
#define MCC_K_DT_OPER_STATE 91
#define MCC_K_DT_SNARESOURCENAME 92
#define MCC_K_DT_QUALIFIEDSNANAME 93
#define MCC_K_DT_STRINGSET 94
#define MCC_K_DT_MESSAGE 75
#define MCC_K_USAGE_STATE_IDLE 0
#define MCC_K_USAGE_STATE_ACTIVE 1
#define MCC_K_USAGE_STATE_BUSY 2
#define MCC_K_USAGE_STATE_UNKNOWN 3
#define MCC_K_ADMIN_STATE_LOCKED 0
#define MCC_K_ADMIN_STATE_UNLOCKED 1
#define MCC_K_ADMIN_STATE_SHUTTING_DOWN 2
#define MCC_K_REPAIR_STATE_UNDER_REPAIR 0
#define MCC_K_REPAIR_STATE_FAULT_OUTSTN 1
#define MCC_K_INSTAL_STATE_NOT_INSTLLD 0
#define MCC_K_INSTAL_STATE_INIT_INCOMPL 1
#define MCC_K_INSTAL_STATE_INIT_REQD 2
#define MCC_K_AVAIL_STATE_IN_TEST 0
#define MCC_K_AVAIL_STATE_FAILED 1
#define MCC_K_AVAIL_STATE_POWER_OFF 2
#define MCC_K_AVAIL_STATE_OFF_LINE 3
#define MCC_K_AVAIL_STATE_OFF_DUTY 4
#define MCC_K_AVAIL_STATE_DEPENDENCY 5
#define MCC_K_AVAIL_STATE_DEGRADED 6
#define MCC_K_AVAIL_STATE_LOG_FULL 7
#define MCC_K_CONTROL_STATE_SUBJ_TO_TST 0
#define MCC_K_CONTROL_STATE_READ_ONLY 1
#define MCC_K_CONTROL_STATE_PART_LOCKED 2
#define MCC_K_CONTROL_STATE_RES_FOR_TST 3
#define MCC_K_CONTROL_STATE_SUSPENDED 4
#define MCC_K_OPER_STATE_DISABLE 0
#define MCC_K_OPER_STATE_ENABLE 1
#define MCC_K_DT_ASN 126
#define MCC_K_DT_ILV 127
#define MCC_K_DT_MAX 127
#define MCC_K_ILV_NATIVE_VALUE 0
#define MCC_K_ILV_LIST_VALUE 1
#define MCC_K_ID_KNOWN 126
#define MCC_K_ID_NULL 127
#define MCC_K_ASN_CL_UNIV 0
#define MCC_K_ASN_CL_APPL 64
#define MCC_K_ASN_CL_CONT 128
#define MCC_K_ASN_CL_PRIV 192
#define MCC_K_ASN_FORM_PRIM 0
#define MCC_K_ASN_FORM_CONS 32
#define MCC_K_ASN_DT_EOC 0
#define MCC_K_ASN_DT_BOOLEAN 1
#define MCC_K_ASN_DT_INTEGER 2
#define MCC_K_ASN_DT_BITSTRING 3
#define MCC_K_ASN_DT_OCTETSTRING 4
#define MCC_K_ASN_DT_NULL 5
#define MCC_K_ASN_DT_OBJECTID 6
#define MCC_K_ASN_DT_OBJECTDESC 7
#define MCC_K_ASN_DT_EXTERNAL 8
#define MCC_K_ASN_DT_REAL 9
#define MCC_K_ASN_DT_SEQUENCE 48
#define MCC_K_ASN_DT_SET 49
#define MCC_K_ASN_DT_SMI_IPADDR INET_C_SMI_IP_ADDRESS  /* moss_inet.h */

struct MCC_R_DB_RCODE {
    int rcode_l_sts;                     /* !! */
    int rcode_l_stv;                     /* !! */ 
    } ;
#define MCC_K_DB_FSCOPE_SYSTEM 0
#define MCC_K_DB_FSCOPE_DOMAIN 1
#define MCC_K_DB_FSCOPE_FILE 2
#define MCC_K_DB_FSCOPE_DICTIONARY 3
#define MCC_K_DB_FTYPE_ENTDEF 0
#define MCC_K_DB_FTYPE_ATTRIBUTE 1
#define MCC_K_DB_FTYPE_BINREL 2
#define MCC_K_DB_MODE_READONLY 1
#define MCC_K_DB_MODE_READ_WRITE 2
#define MCC_K_DB_NOSHARE 0
#define MCC_K_DB_SHARE 1
#define MCC_K_DB_NUMBER 1
#define MCC_K_DB_NAME 2
#define MCC_K_DA_DEF_MBX_SIZ 128
struct MCC_R_DA_IOSB {
    short int sb_w_return_code;
    short int sb_w_bytes_xferd;
    int sb_l_dev_info;                   /* !! */
    } ;
struct MCC_R_DA_IOB {
    short int iob_w_lnk;
    short int iob_w_mbx;
    int iob_l_stat;                      /* !! */
    struct MCC_R_DA_IOSB iob_r_sb;
    } ;
struct MCC_R_DA_MBXMSG {
    unsigned short int msg_w_type;
    unsigned short int msg_w_unit;
    unsigned char msg_b_dev_cnt;
    unsigned char msg_b_dev [3];
    unsigned char msg_b_info_cnt;
    char msg_b_buf [119];
    } ;
union MCC_R_DA_MBMSG_BLK {
    unsigned char blk_b_buf [128];
    struct MCC_R_DA_MBXMSG blk_r_msg;
    } ;
#define MCC_K_LOCK_FIRST_RESOURCE 1
#define MCC_K_LOCK_PROCESS_RESOURCE 1
#define MCC_K_LOCK_GROUP_RESOURCE 2
#define MCC_K_LOCK_SYSTEM_RESOURCE 3
#define MCC_K_LOCK_CLUSTER_RESOURCE 4
#define MCC_K_LOCK_LAST_RESOURCE 4
struct MCC_R_DSPTBLENT {
    struct MCC_R_DSPTBLENT *dte_a_next;
    unsigned short int dte_w_size;
    unsigned int dte_l_class_flags;         /* !! */
    unsigned int dte_l_classcode;           /* !! */
    unsigned int dte_l_instance_flags;      /* !! */
    MCC_T_Descriptor dte_r_instance;
    } ;
/* #define MCC_K_DSP_ENT_SIZE 42 -- OLD */
/* #define MCC_K_DSP_ENT_SIZE 54 -- NEW, but use def'n below: */
#define MCC_K_DSP_ENT_SIZE sizeof(
/* Not for SNMP PE */
#if 0
/*
 *	%W%	(ULTRIX/OSF)	%G%
 */
/*
 *  FACILITY:
 *
 *	Concert Multhithread (tm) Architecture
 *
 *  ABSTRACT:
 *
 *	Header file for example jacket routines which make the selected
 *	C run-time library routines thread reentrant.  These jackets are
 *	transparent to the calling code, so that, when a reentrant version
 *	of the library becomes available, they can be dispensed with without
 *	requiring changes to the calling code.
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	29 June 1990
 *
 *  MODIFICATION HISTORY:
 *  001	    Paul Curtin	    26 October 1990
 *	    Added if's to scanf functions, dependent upon (v)ariable versions.
 *  002	    Paul Curtin	    19 November 1990
 *	    Conditionally removed protos on vms for; cma_pclose, cma_popen,
 *	    cma_setbuffer, cma_setlinebuf, cma_tempnam, and cma_ttyslot
 *  003	    Paul Curtin	    21 November 1990
 *	    Rearranged conditional exclusion
 *  004	    Paul Curtin	    31 January 1991
 *	    Removed conditional types for cma prototypes, to match `man' pgs.
 *  005	    Paul Curtin	    25 February 1991
 *	    Conditionalized *scanf* defines, because the wrapper 
 *	    routines are dependent upon v*scanf versions that are 
 *	    not currently available.
 *  006	    Paul Curtin	    23 April 1991
 *	    Changed the parameter name template to filespec for cma_mktemp.
 */


#ifndef CMA_STDIO
#define CMA_STDIO

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <stdio.h>

/*
 * MACROS
 */

/*
 * C Run-time Library "Standard I/O" Routine Wrappers
 */
#define ctermid	    cma_ctermid 
#define cuserid	    cma_cuserid 
#define fclose	    cma_fclose 
#define fflush	    cma_fflush 
#define fdopen	    cma_fdopen 
#define fgetc	    cma_fgetc 
#define fgets	    cma_fgets 
#define fopen	    cma_fopen 
#define fprintf	    cma_fprintf 
#define fputc	    cma_fputc 
#define fputs	    cma_fputs 
#define fread	    cma_fread 
#define freopen	    cma_freopen 
#define fseek	    cma_fseek 
#define ftell	    cma_ftell 
#define fwrite	    cma_fwrite 
#define gets	    cma_gets 
#define getw	    cma_getw 
#define isatty	    cma_isatty 
#define mktemp	    cma_mktemp 
#define printf	    cma_printf 
#define puts	    cma_puts 
#define putw	    cma_putw 
#define rewind	    cma_rewind 
#define setbuf	    cma_setbuf 
#define setvbuf	    cma_setvbuf 
#define sprintf	    cma_sprintf 
#define tmpfile	    cma_tmpfile 
#define tmpnam	    cma_tmpnam 
#define ttyname	    cma_ttyname 

#if vfscanf_present
# define scanf	    cma_scanf 
# define fscanf	    cma_fscanf 
# define sscanf	    cma_sscanf 
#endif

#ifndef vms
# define pclose	    cma_pclose 
# define popen	    cma_popen 
# define setbuffer  cma_setbuffer 
# define setlinebuf cma_setlinebuf 
# define tempnam    cma_tempnam 
# define ttyslot    cma_ttyslot
#endif

#undef	getc
#define	getc	    cma_fgetc

#undef	getchar
#define	getchar()   cma_fgetc (stdin)

#undef	putc
#define	putc	    cma_fputc

#undef	putchar
#define	putchar(c)  cma_fputc (c, stdin)



/*
 * INTERFACES
 */

extern char *
cma_ctermid _CMA_PROTOTYPE_ ((
	char	*s));

extern char *
cma_cuserid _CMA_PROTOTYPE_ ((
	char	*s));

extern int
cma_fclose _CMA_PROTOTYPE_ ((
	FILE	*stream));

extern int
cma_fflush _CMA_PROTOTYPE_ ((
	FILE	*stream));

extern FILE *
cma_fdopen _CMA_PROTOTYPE_ ((
	int	fildes,
	char	*type));

extern int
cma_fgetc _CMA_PROTOTYPE_ ((
	FILE	*stream));

extern char *
cma_fgets _CMA_PROTOTYPE_ ((
	char	*s,
	int	n,
	FILE	*stream));

extern FILE *
cma_fopen _CMA_PROTOTYPE_ ((
	char	*filename,
	char	*type, 
	...));

extern int
cma_fprintf _CMA_PROTOTYPE_ ((
	FILE	*stream,
	char	*format,
	...));

extern int
cma_fputc _CMA_PROTOTYPE_ ((
	int	c,
	FILE	*stream));

extern int
cma_fputs _CMA_PROTOTYPE_ ((
	char	*s,
	FILE	*stream));

extern int
cma_fread _CMA_PROTOTYPE_ ((
	char	*ptr,
	unsigned size,
	unsigned nitems,
	FILE	*stream));

extern FILE *
cma_freopen _CMA_PROTOTYPE_ ((
	char	*filename,
	char	*type,
	FILE	*stream));

#if vfscanf_present
extern int
cma_fscanf _CMA_PROTOTYPE_ ((
	FILE	*stream,
	char	*format,
	...));
#endif

extern int
cma_fseek _CMA_PROTOTYPE_ ((
	FILE	*stream,
	long	offset,
	int	ptrname));

extern long
cma_ftell _CMA_PROTOTYPE_ ((
	FILE	*stream));

extern int
cma_fwrite _CMA_PROTOTYPE_ ((
	char	*ptr,
	unsigned size,
	unsigned nitems,
	FILE	*stream));

extern char *
cma_gets _CMA_PROTOTYPE_ ((
	char	*s));

extern int
cma_getw _CMA_PROTOTYPE_ ((
	FILE	*stream));

extern int
cma_isatty _CMA_PROTOTYPE_ ((
	int	filedes));

extern char *
cma_mktemp _CMA_PROTOTYPE_ ((
	char	*filespec));

#ifndef vms
/* pclose and popen do not exist on vms */
extern int
cma_pclose _CMA_PROTOTYPE_ ((
    FILE	*stream));

extern FILE *
cma_popen _CMA_PROTOTYPE_ ((
	char	*command,
	char	*type));
#endif

extern int
cma_printf _CMA_PROTOTYPE_ ((
	char	*format,
	...));

extern int
cma_puts _CMA_PROTOTYPE_ ((
	char	*s));

extern int
cma_putw _CMA_PROTOTYPE_ ((
	int	w,
	FILE	*stream));

extern void 
cma_rewind _CMA_PROTOTYPE_ ((
	FILE *stream));

#if vscanf_present
extern int
cma_scanf _CMA_PROTOTYPE_ ((
	char	*format,
	...));
#endif

extern void
cma_setbuf _CMA_PROTOTYPE_ ((
	FILE	*stream,
	char	*buf));

#ifndef vms
/* setbuffer and setlinebuf do not exist on vms */
extern void
cma_setbuffer _CMA_PROTOTYPE_ ((
	FILE	*stream,
	char	*buf,
	int	size));

extern void
cma_setlinebuf _CMA_PROTOTYPE_ ((
	FILE	*stream));
#endif

#if _CMA_HARDWARE_ == _CMA__MIPS || _CMA_HARDWARE_ == _CMA__VAX
extern void
#else
extern int
#endif
cma_setvbuf _CMA_PROTOTYPE_ ((
	FILE	*stream,
	char	*buf,
	int	type,
	int	size));

#ifdef SYSV
extern int
#else
extern char *
#endif
cma_sprintf _CMA_PROTOTYPE_ ((
	char	*s,
	char	*format,
	...));

#if vsscanf_present
extern int
cma_sscanf _CMA_PROTOTYPE_ ((
	char	*s,
	char	*format,
	...));
#endif

#ifndef vms
/* tempnam does not exist on vms */
extern char *
cma_tempnam _CMA_PROTOTYPE_ ((
	char	*dir, 
	char	*pfx));
#endif

extern FILE *
cma_tmpfile _CMA_PROTOTYPE_ ((void));

extern char *
cma_tmpnam _CMA_PROTOTYPE_ ((
	char	*s));

extern char *
cma_ttyname _CMA_PROTOTYPE_ ((
	int	filedes));

#ifndef vms
/* ttyslot does not exist on vms */
extern int
cma_ttyslot _CMA_PROTOTYPE_ ((void));
#endif

extern char
cma_ungetc _CMA_PROTOTYPE_ ((
	char	c,
	FILE	*stream));

extern int
cma_vfprintf _CMA_PROTOTYPE_ ((
	FILE	*stream,
	char	*format,
	...));

extern int
cma_vprintf _CMA_PROTOTYPE_ ((
	char	*format,
	...));

#ifdef SYSV
extern int
#else
extern char *
#endif
cma_vsprintf _CMA_PROTOTYPE_ ((
	char	*s,
	char	*format,
	...));

#endif

#endif
/* Not for SNMP PE */
