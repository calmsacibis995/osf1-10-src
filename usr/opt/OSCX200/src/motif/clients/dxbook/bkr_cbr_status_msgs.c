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
#ifdef CBR
/****************************************************************************/
/*                                                                          */  
/*  Copyright (c) Digital Equipment Corporation, 1990                       */
/*  All Rights Reserved.  Unpublished rights reserved                       */
/*  under the copyright laws of the United States.                          */
/*                                                                          */  
/*  The software contained on this media is proprietary                     */
/*  to and embodies the confidential technology of                          */
/*  Digital Equipment Corporation.  Possession, use,                        */
/*  duplication or dissemination of the software and                        */
/*  media is authorized only pursuant to a valid written                    */
/*  license from Digital Equipment Corporation.                             */
/*                                                                          */  
/*  RESTRICTED RIGHTS LEGEND   Use, duplication, or                         */
/*  disclosure by the U.S. Government is subject to                         */
/*  restrictions as set forth in Subparagraph (c)(1)(ii)                    */
/*  of DFARS 252.227-7013, or in FAR 52.227-19, as                          */
/*  applicable.                                                             */
/*                                                                          */  
/****************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                   INCLUDE FILES and LOCAL DEFINITIONS                      */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <cbr_public.h>
/*
**.
**. The following datatype is used to build the status code to message string
**. translation table:
**.
**.     typedef struct  xlate_item
**.     {
**.         CBR_INT     status  ;   (*  status code                 *)
**.         char      * scode   ;   (*  descriptive message code    *)
**.         char      * message ;   (*  descriptive message text    *)
**.     }
**.         XLATE_ITEM ;
**.
*/
typedef struct  xlate_item
{
    CBR_INT     status  ;   /*  status code                 */
    char      * scode   ;   /*  descriptive message code    */
    char      * message ;   /*  descriptive message text    */
}
    XLATE_ITEM ;

/*
**.
**. The data structure status_table is an array containing pairs of
**. status codes, and message strings associated with those status codes.
**.
*/
CBR_STATIC XLATE_ITEM CBR_CONST status_table []
=
{{CBR_S_ADDING_FIELD ,"CBR_S_ADDING_FIELD","could not add field during load"}
,{CBR_S_ADD_DB_FIELD ,"CBR_S_ADD_DB_FIELD","loading DB field"}
,{CBR_S_ADD_MERGED_PRTN_FAILURE ,"CBR_S_ADD_MERGED_PRTN_FAILURE","error adding new PRTN to database"}
,{CBR_S_ADD_PRTN ,"CBR_S_ADD_PRTN","could not add partition to db"}
,{CBR_S_ALREADY_OPEN ,"CBR_S_ALREADY_OPEN","object (e.g., session) already opened"}
,{CBR_S_BAD_ARGUMENT ,"CBR_S_BAD_ARGUMENT","bad argument"}
,{CBR_S_BAD_FILENAME ,"CBR_S_BAD_FILENAME","Bad file name"}
,{CBR_S_BAD_FILE_BLOCK ,"CBR_S_BAD_FILE_BLOCK","Bad pointer to file control block"}
,{CBR_S_BAD_FORMAT ,"CBR_S_BAD_FORMAT","timestamp doesn't have right format"}
,{CBR_S_BAD_GETCWD ,"CBR_S_BAD_GETCWD","could not get current working directory"}
,{CBR_S_BAD_LENGTH ,"CBR_S_BAD_LENGTH","timestamp doesn't have right size"}
,{CBR_S_BAD_LIST ,"CBR_S_BAD_LIST","list is damaged"}
,{CBR_S_BAD_LIST_DATA ,"CBR_S_BAD_LIST_DATA","current data pointer is null"}
,{CBR_S_BAD_LIST_NODE ,"CBR_S_BAD_LIST_NODE","unusable current node"}
,{CBR_S_BAD_LIST_ROOT ,"CBR_S_BAD_LIST_ROOT","unusable root node pointer"}
,{CBR_S_BAD_MERGE_LIST ,"CBR_S_BAD_MERGE_LIST","list count is bad"}
,{CBR_S_BAD_NEXT_PRTN ,"CBR_S_BAD_NEXT_PRTN","error getting next PRTN sysid"}
,{CBR_S_BAD_OPERATOR ,"CBR_S_BAD_OPERATOR","CNCPT operator found is bad"}
,{CBR_S_BAD_QHST_LIST ,"CBR_S_BAD_QHST_LIST","query history list bad"}
,{CBR_S_BAD_QUERY ,"CBR_S_BAD_QUERY","bad search query"}
,{CBR_S_BAD_TOKEN_TABLE ,"CBR_S_BAD_TOKEN_TABLE","bad object table pointer"}
,{CBR_S_BUFFER_LENGTH ,"CBR_S_BUFFER_LENGTH","working buffer too small"}
,{CBR_S_BUFFER_TOO_LARGE ,"CBR_S_BUFFER_TOO_LARGE","specified sysid too large"}
,{CBR_S_CANCEL_FAILURE ,"CBR_S_CANCEL_FAILURE","could not cancel changes to CSET"}
,{CBR_S_CANNOT_DELETE ,"CBR_S_CANNOT_DELETE","can not delete root node"}
,{CBR_S_CHANGE_CSET ,"CBR_S_CHANGE_CSET","could not change CSET"}
,{CBR_S_CHANGE_NAME ,"CBR_S_CHANGE_NAME","name change failed"}
,{CBR_S_CHANGE_OPERATOR ,"CBR_S_CHANGE_OPERATOR","operator change failed"}
,{CBR_S_CHANGE_WEIGHT ,"CBR_S_CHANGE_WEIGHT","weight change failed"}
,{CBR_S_CHILDREN_MISSING ,"CBR_S_CHILDREN_MISSING","children required for operation missing"}
,{CBR_S_CHMOD_DIR ,"CBR_S_CHMOD_DIR","error changing DIR protection"}
,{CBR_S_CHMOD_SID ,"CBR_S_CHMOD_SID","error changing SID protection"}
,{CBR_S_COMMENT_TOKEN ,"CBR_S_COMMENT_TOKEN","comment line found"}
,{CBR_S_CONFIGURE_CSET_FILE ,"CBR_S_CONFIGURE_CSET_FILE","could not configure/load CSET file"}
,{CBR_S_CONFIGURE_LEXER ,"CBR_S_CONFIGURE_LEXER","could not configure lexer"}
,{CBR_S_CONFIGURE_PARSERFILE ,"CBR_S_CONFIGURE_PARSERFILE","error referencing parser file"}
,{CBR_S_CONFIGURE_STYLEFILE ,"CBR_S_CONFIGURE_STYLEFILE","could not configure style file"}
,{CBR_S_CONVERT_NAME ,"CBR_S_CONVERT_NAME","internal name conversion failed"}
,{CBR_S_CONVERT_TIMESTAMP ,"CBR_S_CONVERT_TIMESTAMP","converting binary timestamp to ASCII"}
,{CBR_S_COUNT_ENTRIES ,"CBR_S_COUNT_ENTRIES","could not count entries"}
,{CBR_S_CREATE_CHILD ,"CBR_S_CREATE_CHILD","internal child entry creation failed"}
,{CBR_S_CREATE_CNCPT ,"CBR_S_CREATE_CNCPT","create failed"}
,{CBR_S_CREATE_DDD ,"CBR_S_CREATE_DDD","DDD create failed"}
,{CBR_S_CREATE_DIR ,"CBR_S_CREATE_DIR","directory creation failed"}
,{CBR_S_CREATE_SYSTEM_CSET ,"CBR_S_CREATE_SYSTEM_CSET","error creating system CSET"}
,{CBR_S_CREATE_USER_CSET ,"CBR_S_CREATE_USER_CSET","error creating user CSET"}
,{CBR_S_CSET_MISSING ,"CBR_S_CSET_MISSING","first CSET name missing"}
,{CBR_S_DATA_PREP_MODE_REQUIRED ,"CBR_S_DATA_PREP_MODE_REQUIRED","Data preparation mode required"}
,{CBR_S_DELETE_CHILD ,"CBR_S_DELETE_CHILD","internal child deletion failed"}
,{CBR_S_DELETE_DDD ,"CBR_S_DELETE_DDD","error deleting DDD"}
,{CBR_S_DELETE_DID ,"CBR_S_DELETE_DID","error deleting DID"}
,{CBR_S_DELETE_PRTN ,"CBR_S_DELETE_PRTN","could not delete PRTN"}
,{CBR_S_DELETE_PRTNS ,"CBR_S_DELETE_PRTNS","error deleting PRTNs"}
,{CBR_S_DEVICE_ERROR ,"CBR_S_DEVICE_ERROR","Device not ready or not mounted"}
,{CBR_S_DIR_NOT_FOUND ,"CBR_S_DIR_NOT_FOUND","Directory not found"}
,{CBR_S_EMPTY_LIST ,"CBR_S_EMPTY_LIST","no CSETs in list"}
,{CBR_S_END_OF_DOCUMENT ,"CBR_S_END_OF_DOCUMENT","end of the document/buffer"}
,{CBR_S_END_OF_FILE ,"CBR_S_END_OF_FILE","End of file found"}
,{CBR_S_ENTRY_NOT_FOUND ,"CBR_S_ENTRY_NOT_FOUND","specified entry not found"}
,{CBR_S_ENTRY_RANK ,"CBR_S_ENTRY_RANK","error getting rank of entry"}
,{CBR_S_ENTRY_UNSPECIFIED ,"CBR_S_ENTRY_UNSPECIFIED","entry not specified"}
,{CBR_S_FAILURE ,"CBR_S_FAILURE","operation failed"}
,{CBR_S_FIELD_READ_ERROR ,"CBR_S_FIELD_READ_ERROR","could not read database field"}
,{CBR_S_FILE_CLOSE_ERROR ,"CBR_S_FILE_CLOSE_ERROR","Error closing file"}
,{CBR_S_FILE_DELETE_ERROR ,"CBR_S_FILE_DELETE_ERROR","Error deleting file"}
,{CBR_S_FILE_NOT_FOUND ,"CBR_S_FILE_NOT_FOUND", "File not found"}
,{CBR_S_FILE_OPEN_ERROR ,"CBR_S_FILE_OPEN_ERROR","Error opening file"}
,{CBR_S_FILE_PRIV_ERROR ,"CBR_S_FILE_PRIV_ERROR", "Insufficient privileges to complete operation"}
,{CBR_S_FILE_RSRC_ERROR ,"CBR_S_FILE_RSRC_ERROR","Error due to insufficient resource or quota"}
,{CBR_S_FILE_WRITE_ERROR ,"CBR_S_FILE_WRITE_ERROR","Error writing file"}
,{CBR_S_FIND_CSET_LIST ,"CBR_S_FIND_CSET_LIST","finding list of CSETs"}
,{CBR_S_FREE_DATA ,"CBR_S_FREE_DATA","backend data free error"}
,{CBR_S_FREE_DB ,"CBR_S_FREE_DB","could not free vendor database"}
,{CBR_S_FREE_DOC_LIST ,"CBR_S_FREE_DOC_LIST","could not free doc list"}
,{CBR_S_FREE_ENGINE ,"CBR_S_FREE_ENGINE","could not free vendor engine"}
,{CBR_S_FREE_INTERNAL_LIST ,"CBR_S_FREE_INTERNAL_LIST","could not free internal list"}
,{CBR_S_FREE_LIST ,"CBR_S_FREE_LIST","could not free list"}
,{CBR_S_FREE_RETRIEVAL_LIST ,"CBR_S_FREE_RETRIEVAL_LIST","could not free retrieval list "}
,{CBR_S_GET_CHILD_DETAILS ,"CBR_S_GET_CHILD_DETAILS","could not get child details"}
,{CBR_S_GET_CHILD_INFO ,"CBR_S_GET_CHILD_INFO","could not get specific child info"}
,{CBR_S_GET_ENTRY ,"CBR_S_GET_ENTRY","could not get entry"}
,{CBR_S_GET_LIST ,"CBR_S_GET_LIST","could not get list"}
,{CBR_S_GET_LOCATIONS ,"CBR_S_GET_LOCATIONS","getting array of locations"}
,{CBR_S_GET_SEARCH_STATISTICS ,"CBR_S_GET_SEARCH_STATISTICS","could not get search statistics"}
,{CBR_S_GET_SEARCH_STATUS ,"CBR_S_GET_SEARCH_STATUS","could not get search status"}
,{CBR_S_GET_TIMESTAMP ,"CBR_S_GET_TIMESTAMP","getting binary timestamp"}
,{CBR_S_INDEX_PRTN ,"CBR_S_INDEX_PRTN","could not index partition"}
,{CBR_S_INITIALIZE_CSET ,"CBR_S_INITIALIZE_CSET","could not initialize CSET"}
,{CBR_S_INITIALIZE_ENGINE ,"CBR_S_INITIALIZE_ENGINE","could not initialize engine"}
,{CBR_S_INITIALIZE_LANGUAGE ,"CBR_S_INITIALIZE_LANGUAGE","could not initialize stemmer"}
,{CBR_S_INITIALIZE_LIST ,"CBR_S_INITIALIZE_LIST","could not initialize list"}
,{CBR_S_INITIALIZE_SEARCH ,"CBR_S_INITIALIZE_SEARCH","could not initialize search"}
,{CBR_S_INTERNAL ,"CBR_S_INTERNAL","generic start of module specific internal status codes."}
,{CBR_S_INTERNAL_ERROR ,"CBR_S_INTERNAL_ERROR","internal error"}
,{CBR_S_INVALID_DATA ,"CBR_S_INVALID_DATA","backend data invalid"}
,{CBR_S_INVALID_DB ,"CBR_S_INVALID_DB","invalid search database"}
,{CBR_S_INVALID_ENGINE ,"CBR_S_INVALID_ENGINE","invalid engine"}
,{CBR_S_INVALID_FIELD ,"CBR_S_INVALID_FIELD","invalid object field "}
,{CBR_S_INVALID_HANDLE ,"CBR_S_INVALID_HANDLE","bad handle"}
,{CBR_S_INVALID_ITEM_ENTRY ,"CBR_S_INVALID_ITEM_ENTRY","bad item list entry"}
,{CBR_S_INVALID_RECORD ,"CBR_S_INVALID_RECORD","document record missing"}
,{CBR_S_IN_PROGRESS ,"CBR_S_IN_PROGRESS","operation in progress"}
,{CBR_S_ITEM_NOT_PROCESSED ,"CBR_S_ITEM_NOT_PROCESSED","item list entry not proccessed"}
,{CBR_S_LANGUAGE_CHANGE ,"CBR_S_LANGUAGE_CHANGE","language change detected by language parser"}
,{CBR_S_LOGIC_ERROR ,"CBR_S_LOGIC_ERROR","unknown operation requested"}
,{CBR_S_MAP_CONCEPT_SET ,"CBR_S_MAP_CONCEPT_SET","could not map concept set to db"}
,{CBR_S_MERGE_PRTNS ,"CBR_S_MERGE_PRTNS","error merging PRTNs"}
,{CBR_S_MISSING_OPERATOR ,"CBR_S_MISSING_OPERATOR","required operator missing"}
,{CBR_S_MISSING_TOKEN ,"CBR_S_MISSING_TOKEN","null token field in line"}
,{CBR_S_NAME_NOT_FOUND ,"CBR_S_NAME_NOT_FOUND","name not found"}
,{CBR_S_NOT_FOUND ,"CBR_S_NOT_FOUND","not found"}
,{CBR_S_NO_CLCN ,"CBR_S_NO_CLCN","required CLCN handle missing"}
,{CBR_S_NO_CLCN_TO_SEARCH ,"CBR_S_NO_CLCN_TO_SEARCH","CLCN to search not specified"}
,{CBR_S_NO_CNCPT_DEFINED ,"CBR_S_NO_CNCPT_DEFINED","no concept defined"}
,{CBR_S_NO_CSET ,"CBR_S_NO_CSET","CSET handle missing"}
,{CBR_S_NO_ENTRY ,"CBR_S_NO_ENTRY","no entry specified"}
,{CBR_S_NO_EXPANDED_CNCPT ,"CBR_S_NO_EXPANDED_CNCPT","no expansion available"}
,{CBR_S_NO_LIST_ENTRIES ,"CBR_S_NO_LIST_ENTRIES","no list entries"}
,{CBR_S_NO_LOCATOR ,"CBR_S_NO_LOCATOR","locator missing"}
,{CBR_S_NO_MERGE_LIST ,"CBR_S_NO_MERGE_LIST","list of partitions missing"}
,{CBR_S_NO_PATHNAME ,"CBR_S_NO_PATHNAME","pathname missing"}
,{CBR_S_NO_PRFL ,"CBR_S_NO_PRFL","PRFL handle missing"}
,{CBR_S_NO_PRTN_SPECIFIED ,"CBR_S_NO_PRTN_SPECIFIED","PRTN missing"}
,{CBR_S_NO_QHST ,"CBR_S_NO_QHST","no QHST available"}
,{CBR_S_NO_SQRY ,"CBR_S_NO_SQRY","no SQRY specified"}
,{CBR_S_NO_SYSID ,"CBR_S_NO_SYSID","sysid missing"}
,{CBR_S_NO_TIMESTAMP ,"CBR_S_NO_TIMESTAMP","timestamp missing"}
,{CBR_S_NO_TOKENS ,"CBR_S_NO_TOKENS","entry contains no tokens"}
,{CBR_S_NO_TYPE ,"CBR_S_NO_TYPE","document type missing"}
,{CBR_S_NO_USERNAME ,"CBR_S_NO_USERNAME","username missing"}
,{CBR_S_NO_WNORM_CB ,"CBR_S_NO_WNORM_CB","missing stem control block"}
,{CBR_S_NULL_LIST ,"CBR_S_NULL_LIST","empty linked list"}
,{CBR_S_NULL_POINTER ,"CBR_S_NULL_POINTER","pointer is bad"}
,{CBR_S_OPEN_CSET ,"CBR_S_OPEN_CSET","could not open CSET"}
,{CBR_S_OPERATION_MISSING ,"CBR_S_OPERATION_MISSING","operation is missing"}
,{CBR_S_OPERATION_UNAVAILABLE ,"CBR_S_OPERATION_UNAVAILABLE","operation is not available"}
,{CBR_S_OUT_OF_MEMORY ,"CBR_S_OUT_OF_MEMORY","operation out of memory"}
,{CBR_S_OUT_OF_RANGE ,"CBR_S_OUT_OF_RANGE","number requested is out of range"}
,{CBR_S_PARENT_CLCN ,"CBR_S_PARENT_CLCN","invalid CLCN parent"}
,{CBR_S_PARENT_CNCPT ,"CBR_S_PARENT_CNCPT","invalid CNCPT parent"}
,{CBR_S_PARENT_CSET ,"CBR_S_PARENT_CSET","invalid CSET parent"}
,{CBR_S_PARENT_DOC ,"CBR_S_PARENT_DOC","invalid DOC parent"}
,{CBR_S_PARENT_FLTR ,"CBR_S_PARENT_FLTR","invalid FLTR parent"}
,{CBR_S_PARENT_MISSING ,"CBR_S_PARENT_MISSING","internal parent missing"}
,{CBR_S_PARENT_PRFL ,"CBR_S_PARENT_PRFL","invalid PRFL parent"}
,{CBR_S_PARENT_PRTN ,"CBR_S_PARENT_PRTN","invalid PRTN parent"}
,{CBR_S_PARENT_QHST ,"CBR_S_PARENT_QHST","invalid QHST parent"}
,{CBR_S_PARENT_RSLT ,"CBR_S_PARENT_RSLT","invalid RSLT parent"}
,{CBR_S_PARSE_FAILURE ,"CBR_S_PARSE_FAILURE","generic operation failure"}
,{CBR_S_REFRESH_DB ,"CBR_S_REFRESH_DB","could not refresh"}
,{CBR_S_REMAP_FAILURE ,"CBR_S_REMAP_FAILURE","could not remap system CSET"}
,{CBR_S_REMOVE_DIR ,"CBR_S_REMOVE_DIR","error removing directory"}
,{CBR_S_REMOVE_SID ,"CBR_S_REMOVE_SID","error removing SID"}
,{CBR_S_SAVE_FAILURE ,"CBR_S_SAVE_FAILURE","could not save changes to CSET"}
,{CBR_S_SEARCH_FAILURE ,"CBR_S_SEARCH_FAILURE","search failed"}
,{CBR_S_SUCCESS ,"CBR_S_SUCCESS","operation completed successfully"}
,{CBR_S_SUCCESS ,"CBR_S_SUCCESS","operation completed successfully"}
,{CBR_S_SUCCESS ,"CBR_S_SUCCESS","operation completed successfully"}
,{CBR_S_TIME_ERROR ,"CBR_S_TIME_ERROR","time conversion failed"}
,{CBR_S_TRUNCATED ,"CBR_S_TRUNCATED","string transfer (copy,expansion) truncated"}
,{CBR_S_UNINITIALIZED ,"CBR_S_UNINITIALIZED","data structure not initialized"}
,{CBR_S_UNKNOWN_TOKEN ,"CBR_S_UNKNOWN_TOKEN","token not found in line"}
,{0, 0, 0}
};

/*
**. The size of the translation table is determined at compilation time, and
**. is saved in the compile time definition XLATE_SIZE
**.
*/
#define XLATE_SIZE ( sizeof (status_table)/ sizeof (XLATE_ITEM) )

/*
**.
**. The data structure auxstatus_table is an array containing pairs of auxiliary
**. status codes, and message strings associated with those status codes.
**.
*/
CBR_STATIC XLATE_ITEM CBR_CONST auxstatus_table []
=
{{CBR_A_ADD_DOC ,"CBR_A_ADD_DOC","add DOC to DB "}
,{CBR_A_READ_INDEX ,"CBR_A_READ_INDEX","reading index attribute information"}
,{CBR_A_ADD_DOC_ENTRY ,"CBR_A_ADD_DOC_ENTRY","add document entry to database"}
,{CBR_A_ADD_ENTRY ,"CBR_A_ADD_ENTRY","adding SQRY entry to QHST"}
,{CBR_A_ADD_FIELD ,"CBR_A_ADD_FIELD","adding a field to an existing entry"}
,{CBR_A_ADD_MERGED_PRTN ,"CBR_A_ADD_MERGED_PRTN","add new PRTN to database"}
,{CBR_A_ADD_PRTN ,"CBR_A_ADD_PRTN","adding partition to database"}
,{CBR_A_ADD_TO_QHST ,"CBR_A_ADD_TO_QHST","add to QHST"}
,{CBR_A_ALLOCATE_OBJECT ,"CBR_A_ALLOCATE_OBJECT","allocate backend object"}
,{CBR_A_ALLOCATE_OBJECT ,"CBR_A_ALLOCATE_OBJECT","allocating backend object"}
,{CBR_A_BAD_GETCWD ,"CBR_A_BAD_GETCWD","could not get current working directory"}
,{CBR_A_BAD_GETCWD ,"CBR_A_BAD_GETCWD","could not get current working directory"}
,{CBR_A_BAD_NEXT_PRTN ,"CBR_A_BAD_NEXT_PRTN","error getting next PRTN sysid"}
,{CBR_A_BUILD_QUERY ,"CBR_A_BUILD_QUERY","building search query"}
,{CBR_A_CANCEL_CHANGES ,"CBR_A_CANCEL_CHANGES","cancelling changes to CSET"}
,{CBR_A_CHANGE_CHILD_WEIGHT ,"CBR_A_CHANGE_CHILD_WEIGHT","change child relevance weight"}
,{CBR_A_CHANGE_CWD ,"CBR_A_CHANGE_CWD","change working directory"}
,{CBR_A_CHANGE_NAME ,"CBR_A_CHANGE_NAME","change CNCPT name"}
,{CBR_A_CHANGE_OPERATOR ,"CBR_A_CHANGE_OPERATOR","change CNCPT operator"}
,{CBR_A_CHANGE_RELEVANCE ,"CBR_A_CHANGE_RELEVANCE","process relevance "}
,{CBR_A_CHANGE_REMOTE_CSET ,"CBR_A_CHANGE_REMOTE_CSET","change remote CSET"}
,{CBR_A_CHANGE_SYSTEM_CSET ,"CBR_A_CHANGE_SYSTEM_CSET","change system CSET"}
,{CBR_A_CHANGE_USER_CSET ,"CBR_A_CHANGE_USER_CSET","change user CSET"}
,{CBR_A_CHECK_CHILDREN ,"CBR_A_CHECK_CHILDREN","check for children for desiganted operation"}
,{CBR_A_CHECK_CLCN_DIR ,"CBR_A_CHECK_CLCN_DIR","existence check CLCN directory"}
,{CBR_A_CHECK_CNCPT_NUMBER ,"CBR_A_CHECK_CNCPT_NUMBER","range check CNCPT entry number"}
,{CBR_A_CHECK_CSET_LIST ,"CBR_A_CHECK_CSET_LIST","checking list of CSETs"}
,{CBR_A_CHECK_DB ,"CBR_A_CHECK_DB","checking database"}
,{CBR_A_CHECK_DOC_NUMBER ,"CBR_A_CHECK_DOC_NUMBER","check DOC specified by number in RSLT"}
,{CBR_A_CHECK_ENGINE ,"CBR_A_CHECK_ENGINE","check search engine"}
,{CBR_A_CHECK_LOCATIONS ,"CBR_A_CHECK_LOCATIONS","check requested range "}
,{CBR_A_CHECK_MODE ,"CBR_A_CHECK_MODE","check for proper mode"}
,{CBR_A_CHECK_NAME ,"CBR_A_CHECK_NAME","check CNCPT name"}
,{CBR_A_CHECK_OPERATOR ,"CBR_A_CHECK_OPERATOR","check CNCPT operator"}
,{CBR_A_CHECK_PRTN_DIR ,"CBR_A_CHECK_PRTN_DIR","existence check PRTN directory"}
,{CBR_A_CHECK_PRTN_LIST ,"CBR_A_CHECK_PRTN_LIST","validate merge list"}
,{CBR_A_CHECK_RECORD ,"CBR_A_CHECK_RECORD","check document record"}
,{CBR_A_CHECK_SYSID ,"CBR_A_CHECK_SYSID","check sysid"}
,{CBR_A_CHECK_TEMP_DIR ,"CBR_A_CHECK_TEMP_DIR","existence check TEMP directory"}
,{CBR_A_CHMOD_DIR ,"CBR_A_CHMOD_DIR","change .dir protection"}
,{CBR_A_CHMOD_SID ,"CBR_A_CHMOD_SID","change SID protection "}
,{CBR_A_CLCN_ACTIVATED ,"CBR_A_CLCN_ACTIVATED","get CLCN activated flag"}
,{CBR_A_CLCN_DATA ,"CBR_A_CLCN_DATA","check/get CLCN backend data"}
,{CBR_A_CLCN_HANDLE ,"CBR_A_CLCN_HANDLE","get CLCN handle"}
,{CBR_A_CLCN_LOCATOR ,"CBR_A_CLCN_LOCATOR","get CLCN locator"}
,{CBR_A_CLCN_NAME ,"CBR_A_CLCN_NAME","get CLCN name"}
,{CBR_A_CLCN_NUMBER ,"CBR_A_CLCN_NUMBER","get CLCN specified by number"}
,{CBR_A_CLCN_PATHNAME ,"CBR_A_CLCN_PATHNAME","get CLCN pathname"}
,{CBR_A_CLCN_SYSID ,"CBR_A_CLCN_SYSID","get sysid from CLCN"}
,{CBR_A_CLCN_TOTAL ,"CBR_A_CLCN_TOTAL","get total number of CLCNs owned"}
,{CBR_A_CLOSE_ACTIVE_CSET_LIST ,"CBR_A_CLOSE_ACTIVE_CSET_LIST","closing concept set list"}
,{CBR_A_CLOSE_ACTIVE_DOC ,"CBR_A_CLOSE_ACTIVE_DOC","closing active document"}
,{CBR_A_CLOSE_ACTIVE_DOC_LIST ,"CBR_A_CLOSE_ACTIVE_DOC_LIST","closing list of active documents"}
,{CBR_A_CLOSE_CLCN_LIST ,"CBR_A_CLOSE_CLCN_LIST","closing collection list"}
,{CBR_A_CLOSE_CSET_LIST ,"CBR_A_CLOSE_CSET_LIST","closing concept set list"}
,{CBR_A_CLOSE_FILE ,"CBR_A_CLOSE_FILE","closing file"}
,{CBR_A_CLOSE_FLTR_LIST ,"CBR_A_CLOSE_FLTR_LIST","closing filter list"}
,{CBR_A_CLOSE_PARSER ,"CBR_A_CLOSE_PARSER","closing parser"}
,{CBR_A_CLOSE_PRFL_LIST ,"CBR_A_CLOSE_PRFL_LIST","closing profile list"}
,{CBR_A_CLOSE_PROFILE ,"CBR_A_CLOSE_PROFILE","closing profile .ini file"}
,{CBR_A_CLOSE_QHST_LIST ,"CBR_A_CLOSE_QHST_LIST","closing query history list"}
,{CBR_A_CLOSE_RSLT_LIST ,"CBR_A_CLOSE_RSLT_LIST","closing list of result lists"}
,{CBR_A_CLOSE_SQRY_LIST ,"CBR_A_CLOSE_SQRY_LIST","closing search query list"}
,{CBR_A_CLOSE_SSN ,"CBR_A_CLOSE_SSN","closing session"}
,{CBR_A_CNCPT_DATA ,"CBR_A_CNCPT_DATA","check CNCPT data is valid"}
,{CBR_A_CNCPT_HANDLE ,"CBR_A_CNCPT_HANDLE","get CNCPT handle"}
,{CBR_A_CNCPT_SYSID ,"CBR_A_CNCPT_SYSID","check CNCPT sysid"}
,{CBR_A_CONFIGURE_CSET_FILE ,"CBR_A_CONFIGURE_CSET_FILE","configure CSET load file"}
,{CBR_A_CONFIGURE_LEXER ,"CBR_A_CONFIGURE_LEXER","configuring lexical parser"}
,{CBR_A_CONFIGURE_PARSERFILE ,"CBR_A_CONFIGURE_PARSERFILE","reference parser file"}
,{CBR_A_CONVERT_CHILD_NAME ,"CBR_A_CONVERT_CHILD_NAME","convert child name"}
,{CBR_A_CONVERT_FIELD ,"CBR_A_CONVERT_FIELD","converting field name to handle"}
,{CBR_A_CONVERT_NAME ,"CBR_A_CONVERT_NAME","convert CNCPT name"}
,{CBR_A_CONVERT_TIMESTAMP ,"CBR_A_CONVERT_TIMESTAMP","convert binary timestamp to ASCII"}
,{CBR_A_COUNT_CNCPT_ENTRIES ,"CBR_A_COUNT_CNCPT_ENTRIES","count number of CNCPT entries"}
,{CBR_A_COUNT_ENTRIES ,"CBR_A_COUNT_ENTRIES","count entries"}
,{CBR_A_COUNT_GRANDCHILDREN ,"CBR_A_COUNT_GRANDCHILDREN","count grandchild list"}
,{CBR_A_COUNT_LOCATIONS ,"CBR_A_COUNT_LOCATIONS","count word location list"}
,{CBR_A_COUNT_RSLT_LIST ,"CBR_A_COUNT_RSLT_LIST","count RSLT entries"}
,{CBR_A_CREATE_ACTIVE_CNCPT_LIST ,"CBR_A_CREATE_ACTIVE_CNCPT_LIST","creating active list"}
,{CBR_A_CREATE_CHILD ,"CBR_A_CREATE_CHILD","create a child of the current object"}
,{CBR_A_CREATE_CLCN_DIR ,"CBR_A_CREATE_CLCN_DIR","create CLCN directory"}
,{CBR_A_CREATE_CNCPT ,"CBR_A_CREATE_CNCPT","create CNCPT "}
,{CBR_A_CREATE_DDD ,"CBR_A_CREATE_DDD","create DDD"}
,{CBR_A_CREATE_PRTN_DIR ,"CBR_A_CREATE_PRTN_DIR","create PRTN directory"}
,{CBR_A_CREATE_RSLT_HANDLE ,"CBR_A_CREATE_RSLT_HANDLE","create handle to hold results"}
,{CBR_A_CREATE_SYSTEM_CSET ,"CBR_A_CREATE_SYSTEM_CSET","creating system CSET"}
,{CBR_A_CREATE_TEMP_DIR ,"CBR_A_CREATE_TEMP_DIR","create TEMP directory"}
,{CBR_A_CREATE_USER_CSET ,"CBR_A_CREATE_USER_CSET","creating user CSET"}
,{CBR_A_CSET_ACTIVATED ,"CBR_A_CSET_ACTIVATED","get CSET activated flag"}
,{CBR_A_CSET_DATA ,"CBR_A_CSET_DATA","check CSET data is valid"}
,{CBR_A_CSET_HANDLE ,"CBR_A_CSET_HANDLE","get CSET handle"}
,{CBR_A_CSET_LOCATOR ,"CBR_A_CSET_LOCATOR","get CSET locator"}
,{CBR_A_CSET_NAME ,"CBR_A_CSET_NAME","get CSET name"}
,{CBR_A_CSET_NUMBER ,"CBR_A_CSET_NUMBER","get CSET specified by number"}
,{CBR_A_CSET_PATHNAME ,"CBR_A_CSET_PATHNAME","get CSET pathname"}
,{CBR_A_CSET_SYSID ,"CBR_A_CSET_SYSID","get CSET sysid"}
,{CBR_A_CSET_TOTAL ,"CBR_A_CSET_TOTAL","get total number of CSETs owned"}
,{CBR_A_CSET_TYPE ,"CBR_A_CSET_TYPE","get CSET type"}
,{CBR_A_DELETE_CHILD ,"CBR_A_DELETE_CHILD","delete a child"}
,{CBR_A_DELETE_CLCN_LIST ,"CBR_A_DELETE_CLCN_LIST","deleting list of collections"}
,{CBR_A_DELETE_CSET_LIST ,"CBR_A_DELETE_CSET_LIST","deleting list of concept sets"}
,{CBR_A_DELETE_DDD ,"CBR_A_DELETE_DDD","delete DDD from database"}
,{CBR_A_DELETE_DID ,"CBR_A_DELETE_DID","delete DID from database"}
,{CBR_A_DELETE_DOC ,"CBR_A_DELETE_DOC","deleting DOC"}
,{CBR_A_DELETE_FILE ,"CBR_A_DELETE_FILE","deleting file"}
,{CBR_A_DELETE_FLTR_LIST ,"CBR_A_DELETE_FLTR_LIST","deleting list of filters"}
,{CBR_A_DELETE_HANDLE ,"CBR_A_DELETE_HANDLE","deleting object handle"}
,{CBR_A_DELETE_PRTN ,"CBR_A_DELETE_PRTN","delete old PRTN from database"}
,{CBR_A_DELETE_PRTNS ,"CBR_A_DELETE_PRTNS","delete PRTNs"}
,{CBR_A_DELETE_PRTN_LIST ,"CBR_A_DELETE_PRTN_LIST","deleting list of partitions"}
,{CBR_A_DELETE_RSLT ,"CBR_A_DELETE_RSLT","deleting result list"}
,{CBR_A_DELETE_RSLT_LIST ,"CBR_A_DELETE_RSLT_LIST","deleting result list"}
,{CBR_A_DELETE_RSLT_SIBLING ,"CBR_A_DELETE_RSLT_SIBLING","deleting associated result list"}
,{CBR_A_DELETE_SQRY ,"CBR_A_DELETE_SQRY","deleting search query"}
,{CBR_A_DO_OPERATION ,"CBR_A_DO_OPERATION","perform operation "}
,{CBR_A_ERROR_AUXSTATUS ,"CBR_A_ERROR_AUXSTATUS","get error auxliliary status"}
,{CBR_A_ERROR_COUNT ,"CBR_A_ERROR_COUNT","get number of errors"}
,{CBR_A_ERROR_NUMBER ,"CBR_A_ERROR_NUMBER","get particular error"}
,{CBR_A_ERROR_OBJID ,"CBR_A_ERROR_OBJID","get error object identifier"}
,{CBR_A_ERROR_STATUS ,"CBR_A_ERROR_STATUS","get error status"}
,{CBR_A_ERROR_TEXT ,"CBR_A_ERROR_TEXT","get error text"}
,{CBR_A_EXPAND_CNCPT ,"CBR_A_EXPAND_CNCPT","expand CNCPT"}
,{CBR_A_EXPAND_QUERY ,"CBR_A_EXPAND_QUERY","expand search query"}
,{CBR_A_FIND_ACTIVE_CNCPT_LIST ,"CBR_A_FIND_ACTIVE_CNCPT_LIST","find list of active concepts"}
,{CBR_A_FIND_ADD_LOCATION ,"CBR_A_FIND_ADD_LOCATION","find where to add entry to QHST"}
,{CBR_A_FIND_CSET_LIST ,"CBR_A_FIND_CSET_LIST","finding list of CSETs"}
,{CBR_A_FIND_RSLT ,"CBR_A_FIND_RSLT","finding result list in list"}
,{CBR_A_FREE_DB ,"CBR_A_FREE_DB","freeing vendor database"}
,{CBR_A_FREE_ENGINE ,"CBR_A_FREE_ENGINE","freeing vendor search engine"}
,{CBR_A_FREE_NEW_RETRIEVAL_LIST ,"CBR_A_FREE_NEW_RETRIEVAL_LIST","free new retrieval list"}
,{CBR_A_FREE_OLD_CSET_LIST ,"CBR_A_FREE_OLD_CSET_LIST","free old list of CNCPTs"}
,{CBR_A_FREE_OLD_RETRIEVAL_LIST ,"CBR_A_FREE_OLD_RETRIEVAL_LIST","free old retrieval list"}
,{CBR_A_FREE_RSLT ,"CBR_A_FREE_RSLT","freeing RSLT"}
,{CBR_A_GET_CHILD_DETAILS ,"CBR_A_GET_CHILD_DETAILS","get detailed child information"}
,{CBR_A_GET_CHILD_ENTRY ,"CBR_A_GET_CHILD_ENTRY","get child list entry"}
,{CBR_A_GET_CHILD_INFO ,"CBR_A_GET_CHILD_INFO","get specific child information"}
,{CBR_A_GET_CHILD_NAME ,"CBR_A_GET_CHILD_NAME","get child name"}
,{CBR_A_GET_CLCN ,"CBR_A_GET_CLCN","get CLCN handle from item list"}
,{CBR_A_GET_CLCN_HANDLE ,"CBR_A_GET_CLCN_HANDLE","get handle of CLCN to search"}
,{CBR_A_GET_CLCN_PRTN_LIST ,"CBR_A_GET_CLCN_PRTN_LIST","get list of PRTNs"}
,{CBR_A_GET_CNCPT_CHILD_ARRAY ,"CBR_A_GET_CNCPT_CHILD_ARRAY","get array of CNCPT children"}
,{CBR_A_GET_CNCPT_CHILD_NUMBER ,"CBR_A_GET_CNCPT_CHILD_NUMBER","get specified CNCPT child number"}
,{CBR_A_GET_CNCPT_CHILD_TOTAL ,"CBR_A_GET_CNCPT_CHILD_TOTAL","get total number CNCPT children"}
,{CBR_A_GET_CNCPT_DEF ,"CBR_A_GET_CNCPT_DEF","get CNCPT definition"}
,{CBR_A_GET_CNCPT_DEF_SIZE ,"CBR_A_GET_CNCPT_DEF_SIZE","get CNCPT definition size"}
,{CBR_A_GET_CNCPT_ENTRY ,"CBR_A_GET_CNCPT_ENTRY","get CNCPT entry"}
,{CBR_A_GET_CNCPT_HANDLE ,"CBR_A_GET_CNCPT_HANDLE","get handle"}
,{CBR_A_GET_CNCPT_NAME ,"CBR_A_GET_CNCPT_NAME","get name"}
,{CBR_A_GET_CNCPT_NUMBER ,"CBR_A_GET_CNCPT_NUMBER","get CNCPT number"}
,{CBR_A_GET_CNCPT_OPERATOR ,"CBR_A_GET_CNCPT_OPERATOR","get CNCPT operator"}
,{CBR_A_GET_CNCPT_SYSID ,"CBR_A_GET_CNCPT_SYSID","get CNCPT sysid"}
,{CBR_A_GET_CNCPT_TOTAL ,"CBR_A_GET_CNCPT_TOTAL","get total number of CNCPTs"}
,{CBR_A_GET_CSET_HANDLE ,"CBR_A_GET_CSET_HANDLE","get CSET handle"}
,{CBR_A_GET_CSET_LIST ,"CBR_A_GET_CSET_LIST","get list of CSETs"}
,{CBR_A_GET_DOC_CHAR_SET ,"CBR_A_GET_DOC_CHAR_SET","get doc character set"}
,{CBR_A_GET_DOC_ENTRY ,"CBR_A_GET_DOC_ENTRY","get specified DOC entry"}
,{CBR_A_GET_DOC_HANDLE ,"CBR_A_GET_DOC_HANDLE","get DOC handle"}
,{CBR_A_GET_DOC_LANGUAGE ,"CBR_A_GET_DOC_LANGUAGE","get doc language"}
,{CBR_A_GET_DOC_LOCATOR ,"CBR_A_GET_DOC_LOCATOR","get DOC locator"}
,{CBR_A_GET_DOC_NUMBER ,"CBR_A_GET_DOC_NUMBER","get number of DOC"}
,{CBR_A_GET_DOC_PARSER_ID ,"CBR_A_GET_DOC_PARSER_ID","get DOC Parser ID"}
,{CBR_A_GET_DOC_PARSER_ID ,"CBR_A_GET_DOC_PARSER_ID","get DOC Parser ID"}
,{CBR_A_GET_DOC_PARSER_KEY ,"CBR_A_GET_DOC_PARSER_KEY","get parser key"}
,{CBR_A_GET_DOC_PARSER_OPT ,"CBR_A_GET_DOC_PARSER_OPT","get parser opt"}
,{CBR_A_GET_DOC_PARSER_VER ,"CBR_A_GET_DOC_PARSER_VER","get parser version"}
,{CBR_A_GET_DOC_TYPE ,"CBR_A_GET_DOC_TYPE","get DOC type"}
,{CBR_A_GET_DOC_NAME ,"CBR_A_GET_DOC_NAME","get DOC name"}
,{CBR_S_NO_INTERMEDIATE_RSLT , "CBR_S_NO_INTERMEDIATE_RSLT", "Intermdiate result not specified"}
,{CBR_A_GET_ENTRY_RANK ,"CBR_A_GET_ENTRY_RANK","get rank of entry"}
,{CBR_A_GET_FILENAME ,"CBR_A_GET_FILENAME","get a filename"}
,{CBR_A_GET_FIRST_CSET ,"CBR_A_GET_FIRST_CSET","get first CSET in list"}
,{CBR_A_GET_GRANDCHILD_DETAILS ,"CBR_A_GET_GRANDCHILD_DETAILS","get grandchild details"}
,{CBR_A_GET_GRANDCHILD_ENTRY ,"CBR_A_GET_GRANDCHILD_ENTRY","get grandchild entry"}
,{CBR_A_GET_GRANDCHILD_INFO ,"CBR_A_GET_GRANDCHILD_INFO","get grandchild information"}
,{CBR_A_GET_LOCATIONS ,"CBR_A_GET_LOCATIONS","get array of locations"}
,{CBR_A_GET_LOCATION_TOTAL ,"CBR_A_GET_LOCATION_TOTAL","get word location total"}
,{CBR_A_GET_NEW_CNCPT_LIST ,"CBR_A_GET_NEW_CNCPT_LIST","get new list of CNCPTs"}
,{CBR_A_GET_NEW_CSET_LIST ,"CBR_A_GET_NEW_CSET_LIST","get new list of CSETs"}
,{CBR_A_GET_NEW_RETRIEVAL_LIST ,"CBR_A_GET_NEW_RETRIEVAL_LIST","get new retrieval list"}
,{CBR_A_GET_NEXT_PRTN ,"CBR_A_GET_NEXT_PRTN","determine next PRTN"}
,{CBR_A_GET_NEXT_PRTN_SYSID ,"CBR_A_GET_NEXT_PRTN_SYSID","get next PRTN sysid from file"}
,{CBR_A_GET_NULLCHILD_DETAILS ,"CBR_A_GET_NULLCHILD_DETAILS","get NULL child details"}
,{CBR_A_GET_OPERATION ,"CBR_A_GET_OPERATION","get operation"}
,{CBR_A_GET_OPERATOR ,"CBR_A_GET_OPERATOR","check for required operator"}
,{CBR_A_GET_PARENT_ARRAY ,"CBR_A_GET_PARENT_ARRAY","get parents array"}
,{CBR_A_GET_PARENT_TOTAL ,"CBR_A_GET_PARENT_TOTAL","get total number of parents"}
,{CBR_A_GET_PRTN_COUNT ,"CBR_A_GET_PRTN_COUNT","get count of number of PRTNs"}
,{CBR_A_GET_PRTN_HANDLE ,"CBR_A_GET_PRTN_HANDLE","get handle of specific PRTN"}
,{CBR_A_GET_RELATED_CONCEPTS ,"CBR_A_GET_RELATED_CONCEPTS","get related concepts"}
,{CBR_A_GET_RELATED_WORDS ,"CBR_A_GET_RELATED_WORDS","get related words"}
,{CBR_A_GET_RSLT_CONTEXT ,"CBR_A_GET_RSLT_CONTEXT","get RSLT context"}
,{CBR_A_GET_RSLT_LOCATIONS ,"CBR_A_GET_RSLT_LOCATIONS","get word locations"}
,{CBR_A_GET_RSLT_THRESHOLD ,"CBR_A_GET_RSLT_THRESHOLD","get RSLT threshold value"}
,{CBR_A_GET_SEARCH_STATISTICS ,"CBR_A_GET_SEARCH_STATISTICS","get search statistics"}
,{CBR_A_GET_TIMESTAMP ,"CBR_A_GET_TIMESTAMP","get binary timestamp"}
,{CBR_A_INDEX_PRTN ,"CBR_A_INDEX_PRTN","indexing partition into database"}
,{CBR_A_INITIALIZE_API ,"CBR_A_INITIALIZE_API","initialize search engine"}
,{CBR_A_INITIALIZE_CHILD_LIST ,"CBR_A_INITIALIZE_CHILD_LIST","initialize list of concepts"}
,{CBR_A_INITIALIZE_CNCPT_LIST ,"CBR_A_INITIALIZE_CNCPT_LIST","initialize list of CNCPTs"}
,{CBR_A_INITIALIZE_CSET ,"CBR_A_INITIALIZE_CSET","initialize CSET operation/edits"}
,{CBR_A_INITIALIZE_ENGINE ,"CBR_A_INITIALIZE_ENGINE","initializing search engine"}
,{CBR_A_INITIALIZE_GRANDCHILD_LIST ,"CBR_A_INITIALIZE_GRANDCHILD_LIST","get list of grandchildren "}
,{CBR_A_INITIALIZE_LOCATIONS ,"CBR_A_INITIALIZE_LOCATIONS","initialize word location list"}
,{CBR_A_INITIALIZE_MAXFILE ,"CBR_A_INITIALIZE_MAXFILE","initialize PRTN counter file"}
,{CBR_A_INITIALIZE_RSLT_LIST ,"CBR_A_INITIALIZE_RSLT_LIST","initializing RSLT list"}
,{CBR_A_INITIALIZE_SEARCH ,"CBR_A_INITIALIZE_SEARCH","initializing search"}
,{CBR_A_INITIALIZE_STEMMER ,"CBR_A_INITIALIZE_STEMMER","initializing stemmer"}
,{CBR_A_INSERT_ACTIVE_CNCPT ,"CBR_A_INSERT_ACTIVE_CNCPT","inserting into active list"}
,{CBR_A_INVALID_FIELD ,"CBR_A_INVALID_FIELD","invalid field"}
,{CBR_A_LINK_RSLT_SQRY ,"CBR_A_LINK_RSLT_SQRY","SQRY, RSLT point to each other"}
,{CBR_A_LOAD_ENTRY_POINT ,"CBR_A_LOAD_ENTRY_POINT","loading entry point vector"}
,{CBR_A_LOAD_PROFILE ,"CBR_A_LOAD_PROFILE","load profile .ini file"}
,{CBR_A_LOAD_SESSION ,"CBR_A_LOAD_SESSION","load session .ini file"}
,{CBR_A_MAP_CONCEPT_SET ,"CBR_A_MAP_CONCEPT_SET","mapping concept set to partition"}
,{CBR_A_MERGE_PRTNS ,"CBR_A_MERGE_PRTNS","merge PRTNs"}
,{CBR_A_NEW_ENTRY ,"CBR_A_NEW_ENTRY","creating a new entry"}
,{CBR_A_NEW_PRTN_SYSID ,"CBR_A_NEW_PRTN_SYSID","validate sysid info buffer"}
,{CBR_A_NEXT_PRTN ,"CBR_A_NEXT_PRTN","get next PRTN sysid"}
,{CBR_A_OPEN_FILE ,"CBR_A_OPEN_FILE","opening file"}
,{CBR_A_OPEN_PROFILE ,"CBR_A_OPEN_PROFILE","open profile .ini file"}
,{CBR_A_OPEN_SESSION ,"CBR_A_OPEN_SESSION","open session .ini file"}
,{CBR_A_OPEN_SYSTEM_CSET ,"CBR_A_OPEN_SYSTEM_CSET","open system CSET"}
,{CBR_A_OPEN_USER_CSET ,"CBR_A_OPEN_USER_CSET","open user CSET"}
,{CBR_A_PARENT_MISSING ,"CBR_A_PARENT_MISSING","check internal parent"}
,{CBR_A_PARSE_QUERY ,"CBR_A_PARSE_QUERY","parsing a search query"}
,{CBR_A_PERFORM_SEARCH ,"CBR_A_PERFORM_SEARCH","perform actual search"}
,{CBR_A_PRFL_ACTIVATED ,"CBR_A_PRFL_ACTIVATED","get activated flag for PRFL"}
,{CBR_A_PRFL_HANDLE ,"CBR_A_PRFL_HANDLE","get PRFL handle"}
,{CBR_A_PRFL_LOCATOR ,"CBR_A_PRFL_LOCATOR","get locator from PRFL"}
,{CBR_A_PRFL_NAME ,"CBR_A_PRFL_NAME","get name from PRFL"}
,{CBR_A_PRFL_NUMBER ,"CBR_A_PRFL_NUMBER","get PRFL specified by number"}
,{CBR_A_PRFL_PATHNAME ,"CBR_A_PRFL_PATHNAME","get pathname from PRFL"}
,{CBR_A_PRFL_SYSID ,"CBR_A_PRFL_SYSID","get sysid from PRFL"}
,{CBR_A_PRFL_TOTAL ,"CBR_A_PRFL_TOTAL","get total number of PRFLs owned"}
,{CBR_A_PROCESS_CHILD_LIST ,"CBR_A_PROCESS_CHILD_LIST","process child list"}
,{CBR_A_PROCESS_ITEM_LIST ,"CBR_A_PROCESS_ITEM_LIST","process item list"}
,{CBR_A_PROCESS_OPERATOR ,"CBR_A_PROCESS_OPERATOR","process CNCPT operator"}
,{CBR_A_PROCESS_RSLT_ENTRIES ,"CBR_A_PROCESS_RSLT_ENTRIES","access range of RSLT entries"}
,{CBR_A_PROCESS_SYSID ,"CBR_A_PROCESS_SYSID","modify sysid"}
,{CBR_A_PRTN_DATA ,"CBR_A_PRTN_DATA","check/get PRTN backend data"}
,{CBR_A_PRTN_HANDLE ,"CBR_A_PRTN_HANDLE","get PRTN handle"}
,{CBR_A_PRTN_MERGE_LIST ,"CBR_A_PRTN_MERGE_LIST","validate input list"}
,{CBR_A_PRTN_NUMBER ,"CBR_A_PRTN_NUMBER","get PRTN by number (1..)"}
,{CBR_A_PRTN_SYSID ,"CBR_A_PRTN_SYSID","get PRTN sysid"}
,{CBR_A_QHST_ACTIVATED ,"CBR_A_QHST_ACTIVATED","get activated flag"}
,{CBR_A_QHST_HANDLE ,"CBR_A_QHST_HANDLE","get QHST handle"}
,{CBR_A_QHST_LOCATOR ,"CBR_A_QHST_LOCATOR","get QHST locator"}
,{CBR_A_QHST_NAME ,"CBR_A_QHST_NAME","get name from QHST"}
,{CBR_A_QHST_PATHNAME ,"CBR_A_QHST_PATHNAME","get QHST pathname"}
,{CBR_A_QHST_SYSID ,"CBR_A_QHST_SYSID","get sysid from QHST"}
,{CBR_A_READ_ENTRY ,"CBR_A_READ_ENTRY","reading an entry from the file"}
,{CBR_A_READ_FIELD ,"CBR_A_READ_FIELD","read database field "}
,{CBR_A_READ_FILE ,"CBR_A_READ_FILE","read a file"}
,{CBR_A_READ_HISTORY ,"CBR_A_READ_HISTORY","read query history file"}
,{CBR_A_READ_PROFILE ,"CBR_A_READ_PROFILE","read profile .ini file"}
,{CBR_A_READ_PRTN_LIST ,"CBR_A_READ_PRTN_LIST","read list of PRTN names"}
,{CBR_A_READ_QHST ,"CBR_A_READ_QHST","reading query history"}
,{CBR_A_REFRESH_DB ,"CBR_A_REFRESH_DB","refreshing database"}
,{CBR_A_REMAP_SYSTEM_CSET ,"CBR_A_REMAP_SYSTEM_CSET","remap (optimize) system CSET"}
,{CBR_A_REMOVE_ACTIVE_CNCPT ,"CBR_A_REMOVE_ACTIVE_CNCPT","remove active concept from list"}
,{CBR_A_REMOVE_DIR ,"CBR_A_REMOVE_DIR","remove .dir "}
,{CBR_A_REMOVE_ENTRIES ,"CBR_A_REMOVE_ENTRIES","remove entries from QHST"}
,{CBR_A_REMOVE_ENTRY ,"CBR_A_REMOVE_ENTRY","remove entry from QHST"}
,{CBR_A_REMOVE_RSLT ,"CBR_A_REMOVE_RSLT","removing result list from list"}
,{CBR_A_REMOVE_RSLT ,"CBR_A_REMOVE_RSLT","removing result list from list"}
,{CBR_A_REMOVE_SID ,"CBR_A_REMOVE_SID","remove SID from database"}
,{CBR_A_REMOVE_SQRY ,"CBR_A_REMOVE_SQRY","removing search query"}
,{CBR_A_RSLT_OBJECT_TYPE ,"CBR_A_RSLT_OBJECT_TYPE","get RSLT object type"}
,{CBR_A_SAVE_CHANGES ,"CBR_A_SAVE_CHANGES","saving changes to CSET"}
,{CBR_A_SAVE_CWD ,"CBR_A_SAVE_CWD","save current working directory"}
,{CBR_A_SAVE_STRING ,"CBR_A_SAVE_STRING","save a string field"}
,{CBR_A_SAVE_STRUCTURE ,"CBR_A_SAVE_STRUCTURE","save new structure instance"}
,{CBR_A_SELECT_ACTIVE_DOC ,"CBR_A_SELECT_ACTIVE_DOC","get active document from list"}
,{CBR_A_SELECT_CSET ,"CBR_A_SELECT_CSET","selecting CSET from list"}
,{CBR_A_SELECT_ENTRY ,"CBR_A_SELECT_ENTRY","select entry"}
,{CBR_A_SQRY_ARRAY_TOTAL ,"CBR_A_SQRY_ARRAY_TOTAL","get total nr QH entry tokens"}
,{CBR_A_SQRY_ERROR ,"CBR_A_SQRY_ERROR","get number of token in error"}
,{CBR_A_SQRY_HANDLE ,"CBR_A_SQRY_HANDLE","get SQRY handle"}
,{CBR_A_SQRY_MAX ,"CBR_A_SQRY_MAX","get max number of entries buffered"}
,{CBR_A_SQRY_NUMBER ,"CBR_A_SQRY_NUMBER","get entry specified by number"}
,{CBR_A_SQRY_TIMESTAMP ,"CBR_A_SQRY_TIMESTAMP","get QH entry timestamp"}
,{CBR_A_SQRY_TOKEN_FIELDS ,"CBR_A_SQRY_TOKEN_FIELDS","get QH entry token fields"}
,{CBR_A_SQRY_TOKEN_LENGTH ,"CBR_A_SQRY_TOKEN_LENGTH","get QH entry token lengths"}
,{CBR_A_SQRY_TOKEN_NUMBER ,"CBR_A_SQRY_TOKEN_NUMBER","get QH entry token by number"}
,{CBR_A_SQRY_TOTAL ,"CBR_A_SQRY_TOTAL","get total number of queries"}
,{CBR_A_SQRY_WNORM_CB ,"CBR_A_SQRY_WNORM_CB","get stem control block"}
,{CBR_A_SSN_PATHNAME ,"CBR_A_SSN_PATHNAME","get SSN pathname"}
,{CBR_A_SSN_SYSID ,"CBR_A_SSN_SYSID","get SSN sysid"}
,{CBR_A_TOKENIZE_ENTRY ,"CBR_A_TOKENIZE_ENTRY","tokenizing an entry"}
,{CBR_A_UNLOAD_CLCN_LIST ,"CBR_A_UNLOAD_CLCN_LIST","unloading list of CLCNs"}
,{CBR_A_UNLOAD_CSET_LIST ,"CBR_A_UNLOAD_CSET_LIST","unloading list of CSETs"}
,{CBR_A_UNLOAD_FLTR_LIST ,"CBR_A_UNLOAD_FLTR_LIST","unloading list of FLTRs"}
,{CBR_A_UNLOAD_PRFL_LIST ,"CBR_A_UNLOAD_PRFL_LIST","unloading list of PRFLs"}
,{CBR_A_UNLOAD_SQRY_LIST ,"CBR_A_UNLOAD_SQRY_LIST","unloading list of SQRYs"}
,{CBR_A_UPDATE_CSET ,"CBR_A_UPDATE_CSET","update CSET"}
,{CBR_A_WRITE_FIELD ,"CBR_A_WRITE_FIELD","writing field to disk"}
,{CBR_A_WRITE_FILE ,"CBR_A_WRITE_FILE","writing file"}
,{CBR_A_CONTINUE_SEARCH ,"CBR_A_CONTINUE_SEARCH","could not continue search"}
,{CBR_A_NO_CNCPT_INFO, "CBR_A_NO_CNCPT_INFO", "No concept information supplied"}
,{CBR_A_DUP_SYSTEM_CSET,"CBR_A_DUP_SYSTEM_CSET","No duplicate system csets"}
,{CBR_A_DUP_USER_CSET,"CBR_A_DUP_USER_CSET","No duplicate user csets"}
,{0, 0, 0}
};
/*
**. The size of the translation table is determined at compilation time, and
**. is saved in the compile time definition XLATE_AUXSIZE
**.
*/
#define XLATE_AUXSIZE ( sizeof (auxstatus_table)/ sizeof (XLATE_ITEM) )
/*
**.
**. The data structure operation_table is an array containing pairs of
**. operation codes, and message strings associated with those operation codes.
**.
*/
CBR_STATIC XLATE_ITEM CBR_CONST operation_table []
=
{{CBR_OP_CREATE     ,"CBR_OP_CREATE"    ,"create"}
,{CBR_OP_OPEN       ,"CBR_OP_OPEN"      ,"open"}
,{CBR_OP_INDEX      ,"CBR_OP_INDEX"     ,"index"}
,{CBR_OP_SEARCH     ,"CBR_OP_SEARCH"    ,"search"}
,{CBR_OP_EXPLAIN    ,"CBR_OP_EXPLAIN"   ,"explain"}
,{CBR_OP_GETINFO    ,"CBR_OP_GETINFO"   ,"getinfo"}
,{CBR_OP_MODIFY     ,"CBR_OP_MODIFY"    ,"modify"}
,{CBR_OP_ADD        ,"CBR_OP_ADD"       ,"add"}
,{CBR_OP_REMOVE     ,"CBR_OP_REMOVE"    ,"remove"}
,{CBR_OP_PURGE      ,"CBR_OP_PURGE"     ,"purge"}
,{CBR_OP_REORGANIZE ,"CBR_OP_REORGANIZE","reorganize"}
,{CBR_OP_CLOSE      ,"CBR_OP_CLOSE"     ,"close"}
,{CBR_OP_DELETE     ,"CBR_OP_DELETE"    ,"delete"}
,{CBR_OP_MERGE      ,"CBR_OP_MERGE"     ,"merge"}
,{0, 0, 0}
};
/*
**. The size of the translation table is determined at compilation time, and
**. is saved in the compile time definition XLATE_OPSIZE
**.
*/
#define XLATE_OPSIZE ( sizeof (operation_table)/ sizeof (XLATE_ITEM) )

/*.
**.
**. Function:
**.
**.     int demo_translate_status (status, type, buflen, msgbuf)
**.     
**.         CBR_STATUS status ; (*r: status code to interpret                 *)
**.         int        type   ; (*r: type of status: 0- major                 *)
**.                             (*                   1- auxiliary             *)
**.                             (*                   2- major code in text    *)
**.                             (*                   3- auxiliary code in text*)
**.         int        buflen ; (*r: max allowed length of interpreted message*)
**.         char     * msgbuf ; (*m: buffer to contain interpreted message    *)
**. 
**. Description
**.  
**.     This function is responsible for the translating a status code into a
**.     user visible message.
**.  
**. Algorithm:
**. 
**.     1.  Assume that the status code cannot be translated.
**. 
**.     2.  Check to see if a translation buffer has been supplied; if not,
**.         goto step 3.
**. 
**.         2.a Clear the translation buffer
**. 
**.         2.b Scan the translation table for the specified status
**. 
**.         2.c If the status is found, copy the corresponding message to the
**.             message buffer (making sure that there is no overflow), flag
**.             the translation as successfully completed, and then goto step
**.             3, otherwise go back to step 2.
**. 
**.     3.  Return the status of the translation completion.
**.
**. Return values:
**.  
**.     TRUE            if the status code was successfully translated
**.     FALSE           otherwise
**.
**  Version     Date        Author          Change
**  -------     ----        ------          ------
**  1.2         26-AUG-1991 A. R. Hagel     New status codes require I/F change
**  1.0          1-MAY-1990 A. R. Hagel     Created
*/
int demo_translate_status (status, type, buflen, msgbuf)

    CBR_STATUS   status ;   /*r: status code to interpret                   */
    int          type   ;   /*r: type of status: 0- major                   */
                            /*                   1- auxiliary               */
                            /*                   2- major code in text      */
                            /*                   3- auxiliary code in text  */
    int          buflen ;   /*r: max allowed length of interpreted message  */
    char       * msgbuf ;   /*m: buffer to contain interpreted message      */
{
    int          i      ;   /*   generic index                              */
    int          j      ;   /*   generic index                              */
    int          k      ;   /*   generic index                              */
    char       * t      ;   /*   temporary pointer                          */
    CBR_INT      s      ;   /*   generic status                             */
    int          max    ;   /*   array bound                                */
    XLATE_ITEM * ptable ;   /*   pointer to major or auxliary status tables */

    /**/

    s = FALSE ;                                                         /*1*/
    if ( type == 0 )
    {
        max = XLATE_SIZE ;
        ptable = status_table;
    }
    if ( type == 2)
    {
        max = XLATE_OPSIZE ;
        ptable = operation_table;
    }
    else
    {
        max = XLATE_AUXSIZE;
        ptable = auxstatus_table;
    }

    if ( msgbuf != (char *) NULL )                                      /*2*/
    {
        *msgbuf = '\0' ;                                                /*2.a*/
        for (i = 0 ; i < max ; i++)                                     /*2.b*/
        {
            if ( ptable[i].status == status )                           /*2.c*/
            {
                switch( type )
                {   
                case 0:     t =  ptable[i].message; break;
                case 1:     t =  ptable[i].message; break;
                case 2:     t =  ptable[i].scode;   break;
                case 3:     t =  ptable[i].scode;   break;
                default:    t =  0;
                }
                if( t )
                {
                    k = strlen (t) ;
                    for (j = 0 ; (j < buflen) && (j < k) ; j++ )
                        msgbuf[j] = t[j] ;
                    msgbuf[j] = '\0' ;
                    s = TRUE ;
                }
                break ;
            }
        }
    }
    return( s );                                                        /*3*/
}
/*.
**.
**. Function:
**.
**.     void demo_display_status (pmsghdg, status) 
**.     
**.         char      * pmsghdg ; (*r: message heading to print *)
**.         CBR_STATUS  status  ; (*r: status code to interpret *)
**.         int         type    ; (*r: type of status: 0- major, 1- auxiliary *)
**. 
**. Description
**.  
**.     This routine is responsible for the translation of a status code into a
**.     user visible message, and then displaying that message.
**.  
**. Algorithm:
**. 
**.     1.  Attempt to translate the status code.
**. 
**.     2.  If the translation was successful, then display the translation with
**.         the optionally available message heading.  Goto step 4.
**. 
**.     3.  If the translation was unsuccessful, then display a fallback message
**.         status with the optionally available message heading.  Goto step 4.
**. 
**.     4.  Return to the caller.
**.
**. Return values:
**.  
**.     none
**.
**  Version     Date        Author          Change
**  -------     ----        ------          ------
**  1.2         26-AUG-1991 A. R. Hagel     New status codes require I/F change
**  1.0          1-MAY-1990 A. R. Hagel     Created
*/
void demo_display_status (pmsghdg, status, type ) 

    char      * pmsghdg;   /*r: message heading to print                    */
    CBR_STATUS  status ;   /*r: status code to interpret                    */
    int         type   ;   /*r: type of status: 0- major, 1- auxiliary      */
{
    char        msgbuf [BUFSIZ] ;   /*   buffer to contain the translation  */

    /**/

    if( demo_translate_status (status, type, sizeof (msgbuf), msgbuf) ) /*1*/
    {
        if ( pmsghdg )                                                  /*2*/
            printf ("%s %s\n", pmsghdg, msgbuf );
        else
            printf ("%s\n", msgbuf );
    }
    else
    {
        if ( pmsghdg )                                                  /*3*/
            printf ("%s\n", pmsghdg );
        printf ("-- translation for status %d unavailable\n", status );
    }

    return ;                                                            /*4*/
}
/*.
**.
**. Function:
**.
**.     void demo_error_display (pmsghdg, estatus, objid, handle ) 
**.     
**.         char        *pmsghdg; (*r: message heading to print         *)
**.         CBR_STATUS  estatus;  (*r: status code to interpret         *)
**.         int         objid;    (*r: object identifier                *)
**.         CBR_VOID    *handle;  (*r: handle of object with error(s)   *)
**. 
**. Description
**.  
**.     This routine is responsible for the translation of a status code into a
**.     user visible message, and then displaying that message.
**.  
**. Algorithm:
**. 
**.     1.  Use demo_display_status to dispaly the status code and heading.
**. 
**.     2.  Establish the ancestry of the object in error, to get back to the
**.         session handle.  If there are any errors, return.
**. 
**.     3.  Get the number of errors.
**. 
**.     4.  For each error, get detailed information (object in error,
**.         error status, auxiliary status, and message text).  Display 
**.         the object type, the translated error status, the translated
**.         auxiliary status, and the message text, if any.
**.
**. 
**.     5.  Return to the caller.
**.
**. Return values:
**.  
**.     none
**.
**  Version     Date        Author          Change
**  -------     ----        ------          ------
**  1.3          3-SEP-1991 A. R. Hagel     Created
*/
void demo_error_display (pmsghdg, estatus, objid, handle ) 

    char        *pmsghdg;       /*r: message heading to print               */
    CBR_STATUS  estatus;        /*r: status code to interpret               */
    int         objid;          /*r: object identifier                      */
    CBR_VOID    *handle;        /*r: handle of object with error(s)         */
{
    CBR_VOID    *hSSN;          /*   handle                                 */
    CBR_VOID    *hPRFL;         /*   handle                                 */
    CBR_VOID    *hCLCN;         /*   handle                                 */
    CBR_VOID    *hPRTN;         /*   handle                                 */
    CBR_VOID    *hCSET;         /*   handle                                 */
    CBR_VOID    *hCNCPT;        /*   handle                                 */
    CBR_VOID    *hRSLT;         /*   handle                                 */
    CBR_VOID    *hSQRY;         /*   handle                                 */
    CBR_VOID    *pUCB;          /*   unused                                 */
    CBR_INT     ni;             /*   number of paramters                    */
    CBR_ITEM    pi[16];         /*   parameters                             */
    CBR_INT     i;
    CBR_INT     NrErrs;
    CBR_STATUS  status;
    CBR_INT     OpCode;
    CBR_INT     AuxStat;    
    char        msgbuf0[BUFSIZ];
    char        msgbuf1[BUFSIZ];
    char        msgbuf2[BUFSIZ];
    char        msgbuf3[BUFSIZ];
    char        msgtxt[BUFSIZ];
    char        hdgbuf[BUFSIZ];
    char        *pText;
    int         s;

    /**/

    if( pmsghdg )                                                       /*1*/
        strcpy( hdgbuf, pmsghdg );
    else
        hdgbuf[0] = '\0';

    s = demo_translate_status (estatus, 0, sizeof(msgbuf0), msgbuf0 );
    if( s && msgbuf0[0] )
    {
        if( hdgbuf[0] )
            strcat( hdgbuf,", ");
        strcat( hdgbuf, msgbuf0);
    }

    printf( "%s\n", hdgbuf );

    switch( objid )                                                     /*2*/
    {
    case CBR_O_SSN:

        hSSN = handle;
        if( !hSSN )
            return;
        break;

    case CBR_O_PRFL:

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hSSN, 0);
        status = cbr_getinfo( objid, handle, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hSSN )
        {
            printf("Could not get PRFL ancestry\n");
            return;
        }
        break;

    case CBR_O_CLCN:

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hPRFL, 0);
        status = cbr_getinfo( objid, handle, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hPRFL )
        {
            printf("Could not get CLCN ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hSSN, 0);
        status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hSSN )
        {
            printf("Could not get PRFL ancestry\n");
            return;
        }
        break;

    case CBR_O_CSET:

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hCLCN, 0);
        status = cbr_getinfo( objid, handle, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hCLCN )
        {
            printf("Could not get CSET ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hPRFL, 0);
        status = cbr_getinfo( CBR_O_CLCN, hCLCN, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hPRFL )
        {
            printf("Could not get CLCN ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hSSN, 0);
        status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hSSN )
        {
            printf("Could not get PRFL ancestry\n");
            return;
        }

        break;

    case CBR_O_PRTN:    

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hCLCN, 0);
        status = cbr_getinfo( objid, handle, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hCLCN )
        {
            printf("Could not get PRTN ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hPRFL, 0);
        status = cbr_getinfo( CBR_O_CLCN, hCLCN, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hPRFL )
        {
            printf("Could not get CLCN ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hSSN, 0);
        status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hSSN )
        {
            printf("Could not get PRFL ancestry\n");
            return;
        }

        break;

    case CBR_O_CNCPT:   

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hCSET, 0);
        status = cbr_getinfo( objid, handle, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hCSET )
        {
            printf("Could not get CNCPT ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hCLCN, 0);
        status = cbr_getinfo( CBR_O_CSET, hCSET, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hCLCN )
        {
            printf("Could not get CSET ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hPRFL, 0);
        status = cbr_getinfo( CBR_O_CLCN, hCLCN, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hPRFL )
        {
            printf("Could not get CLCN ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hSSN, 0);
        status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hSSN )
        {
            printf("Could not get PRFL ancestry\n");
            return;
        }

        break;

    case CBR_O_SQRY:

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hSSN, 0);
        status = cbr_getinfo( objid, handle, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hSSN )
        {
            printf("Could not get SQRY ancestry\n");
            return;
        }
        break;

    case CBR_O_RSLT:

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hCLCN, 0);
        status = cbr_getinfo( objid, handle, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hCLCN )
        {
            printf("Could not get RSLT ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hPRFL, 0);
        status = cbr_getinfo( CBR_O_CLCN, hCLCN, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hPRFL )
        {
            printf("Could not get CLCN ancestry\n");
            return;
        }

        CBR_M_CLR(pi, ni);
        CBR_M_OUT(pi, ni, CBR_I_PARENT_HANDLE,  &hSSN, 0);
        status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS || !hSSN )
        {
            printf("Could not get PRFL ancestry\n");
            return;
        }

        break;

    default:
        printf("Unknown object identifier, error translation stopped\n");
        return;
    }

    CBR_M_CLR(pi, ni);                                                  /*3*/
    CBR_M_OUT(pi, ni, CBR_I_SSN_ERROR_COUNT,  &NrErrs, sizeof(NrErrs) );
    status = cbr_getinfo( CBR_O_SSN, hSSN, ni, pi, pUCB );
    if( status != CBR_S_SUCCESS )
    {
        printf("Could not get error count\n");
        return;
    }

    for( i = 0 ; i < NrErrs ; i++ )                                     /*4*/
    {
        msgtxt[0]  = '\0';
        msgbuf1[0] = '\0';
        msgbuf2[0] = '\0';
        msgbuf3[0] = '\0';

        CBR_M_CLR(pi, ni);
        CBR_M_INP(pi, ni, CBR_I_SSN_ERROR_NUMBER,    &i,       sizeof(i));
        CBR_M_OUT(pi, ni, CBR_I_SSN_ERROR_OBJID,     &objid,   sizeof(objid));
        CBR_M_OUT(pi, ni, CBR_I_SSN_ERROR_OPERATION, &OpCode,  sizeof(OpCode));
        CBR_M_OUT(pi, ni, CBR_I_SSN_ERROR_AUXSTATUS, &AuxStat, sizeof(AuxStat));
        CBR_M_OUT(pi, ni, CBR_I_SSN_ERROR_TEXT,      msgtxt,   sizeof(msgtxt));
        status = cbr_getinfo( CBR_O_SSN, hSSN, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS )
        {
            printf("Could not get error information for entry %d\n", i );
            break;
        }

        switch( objid )
        {
        case CBR_O_PRTN:    pText = "PRTN ";    break;
        case CBR_O_CNCPT:   pText = "CNCPT";    break;
        case CBR_O_SSN:     pText = "SSN  ";    break;
        case CBR_O_PRFL:    pText = "PRFL ";    break;    
        case CBR_O_CLCN:    pText = "CLCN ";    break;
        case CBR_O_CSET:    pText = "CSET ";    break;
        case CBR_O_QHST:    pText = "QHST ";    break;
        case CBR_O_DOC:     pText = "DOC  ";    break;
        case CBR_O_SQRY:    pText = "SQRY ";    break;
        case CBR_O_RSLT:    pText = "RSLT ";    break;
        default:            pText = "?    ";    break;
        }
        
        printf( "%d: %s\n", i, pText );

        s = demo_translate_status (OpCode, 2, sizeof(msgbuf1), msgbuf1 );
        if( !s || (msgbuf1[0] == '\0') )
            strcpy( msgbuf1, ".");

        s = demo_translate_status (AuxStat, 1, sizeof(msgbuf2), msgbuf2 );
        if( !s || (msgbuf2[0] == '\0') )
            strcpy( msgbuf2, ".");

        s = demo_translate_status (AuxStat, 3, sizeof(msgbuf3), msgbuf3 );
        if( !s || (msgbuf3[0] == '\0') )
            strcpy( msgbuf3, ".");

        if( msgtxt[0] )
            printf( "%d\t%s: %s\n", i, "Text", msgtxt);
        printf( "%d\t%s: %s (%s)\n", i, msgbuf3, msgbuf2, msgbuf1 );
    }

    return;                                                             /*5*/
}

#endif
