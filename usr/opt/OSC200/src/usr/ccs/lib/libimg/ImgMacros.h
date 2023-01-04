/*******************************************************************************
**   COPYRIGHT (c) 1989 BY
**   DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**   ALL RIGHTS RESERVED.
**   
**   THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**   ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**   INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**   COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**   OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**   TRANSFERRED.
**    
**   THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**   AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**   CORPORATION.
**    
**   DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**   SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*******************************************************************************/

/************************************************************************
*         								*
* FACILITY:								*
*   ImgMacros.h								*
*									*
* ABSTRACT:								*
*   C macro definition file for inclusion by various image utility	*
*   routines.								*
*									*
*   NOTE: replaces (and combines) IMG$MACROS.H and IMG$DDIFMACS.H	*
*									*
* ENVIRONMENT:								*
*   VAX/VMS, VAX/ULTRIX, RISC/ULTRIX					*
*									*
* AUTHOR:								*
*   The group.								*
*									*
* CREATION DATE:                                                        * 
*                                                                       *
*   18-SEP-1989						                *
*									*
************************************************************************/

/*******************************************************************************
** $ArgCnt
**
**  DESCRIPTION:
**
**	Returns the number of arguments passed into a called function.
**
**  FORMAL PARAMETERS:
**
**	p1  - the name of the first parameter on the function's parameter list. 
**
**  RETURN VALUE:
**
**	Count - the number of longword sized arguments passed to the function.
**
**  Note:   this macro cannot be used if the function is defined as taking 
**	    no arguments. 
**
**		    			==oOo==
**
**	This macro type casts the first parameter as a pointer to an integer
**	( (int *)&(p1) ), and then returns the contents of the longword
**	which preceeds p1, which is the count of arguments on the stack.
**
**	*NOTE: This macros is used only in VMS as a service to programmers
*/

#define ARGCNT_( p1 ) ( *(((int *)&(p1))-1) )


/* 
**	Initialize list head flink and blink
**	and set list count to zero.
**
**	Takes a list head address as its only parameter.
*/
#define INIT_LHD_( blk )	{struct LHD *lhd = &(blk);\
    		      		lhd->LhdA_Flink = &(blk);\
    				lhd->LhdA_Blink = &(blk);\
    				lhd->LhdW_ListCnt = 0; }


/************************************************************************
**  $$IMG_FRAME_VERIFY_STRUCTURE - Image frame structure checker
**  $$IMG_FRAME_VERIFY_CONTENT -   Image frame content checker
**  $$IMG_ROI_VERIFY_STRUCTURE -   ROI structure checker
**  $$IMG_ROI_VERIFY_CONTENT   -   ROI content checker
** 
**  FUNCTIONAL DESCRIPTION:
**
**	These macros call internal verify routines with the desired level
**  of correctness checking.
**
**  FORMAL PARAMETERS:
**
**      fct - Address of an FCT block
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
**	Both routines return pointer in input FCT
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
/*
**	Constants for VERIFY level
*/
#define IMG_K_VERIFY_STRUCTURE	1
#define IMG_K_VERIFY_CONTENT	2
/*
**	External definitions for verify routines
*/
struct ROI *_ImgVerifyRoi();

/*
**	Verify MACRO definitions
*/
#define IMG_VERIFY_FRAME_STRUCTURE_(fct) \
    _ImgVerifyStructure( fct )

#define IMG_VERIFY_FRAME_CONTENT_(fct) \
    ImgVerifyFrame( fct, 0 )

#define IMG_VERIFY_ROI_STRUCTURE_(roi) \
    _ImgVerifyRoi(roi,IMG_K_VERIFY_STRUCTURE)

#define IMG_VERIFY_ROI_CONTENT_(roi) \
    _ImgVerifyRoi(roi,IMG_K_VERIFY_CONTENT)


/******************************************************************************
**  $LL_FIRST_ELEMENT
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the first element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**
**  IMPLICIT INPUTS:
**
**       address of first element and list count (in llhead)
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of first element in the list or zero if the list is empty.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#define LL_FIRST_ELEMENT_( llhead )	\
    		(((struct LHD *)llhead)->LhdW_ListCnt != 0)?	\
    		((struct LHD *)llhead)->LhdA_Flink:		\
    		0


/******************************************************************************
**  $LL_LAST_ELEMENT
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the last element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**
**  IMPLICIT INPUTS:
**
**       address of last element and list count (in llhead)
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of last element in the list or zero if the list is empty.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#define LL_LAST_ELEMENT_( llhead )	\
    	(((struct LHD *)llhead)->LhdW_ListCnt != 0)? 	\
    	((struct LHD *)llhead)->LhdA_Blink:		\
    	0


/******************************************************************************
**  $LL_NEXT_ELEMENT
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the next element to a particular element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**	cur_element	current list element, passed by reference
**
**  IMPLICIT INPUTS:
**
**      Address of next element (passed in current element) 
**	and list count (in llhead).
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of next element in the list or zero if there is no next
**	element.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#define LL_NEXT_ELEMENT_( llhead, cur_element )	\
    	(((struct LHD *)llhead)->LhdA_Blink != (struct BHD *)cur_element)? \
    	((struct BHD *)cur_element)->BhdA_Flink:			\
    	0


/******************************************************************************
**  $LL_PREV_ELEMENT
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the previous element to a particular element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**	cur_element	current list element, passed by reference
**
**  IMPLICIT INPUTS:
**
**      Address of previous element (passed in current element) 
**	and list count (in llhead).
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of previous element in the list or zero if there is no next
**	element.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#define LL_PREV_ELEMENT_( llhead, cur_element )		\
    	(((struct LHD *)llhead)->LhdA_Flink != (struct BHD *)cur_element)? \
    	((struct BHD *)cur_element)->BhdA_Blink:			\
    	0

/******************************************************************************
**  $CLEAR_BUFFER
**
**  FUNCTIONAL DESCRIPTION:
**
**      Zero a buffer of arbitrary length in bytes
**
**  FORMAL PARAMETERS:
**
**      adr - byte address of buffer to clear
**	len - length in bytes to zero
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**  SIDE EFFECTS:
**
******************************************************************************/
#define CLEAR_BUFFER_(adr,len) \
   _ImgMovc5Long(0,0,0,len,adr)

/******************************************************************************
**  $TEST
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return a value based on the ternary comparison of an argument with zero.
**
**	    For example: $TEST( argument, LT, EQ, GT )
**
**			      if......... 'argument' is negative, return 'LT'
**			        else if.. 'argument' equals zero, return 'EQ'
**			          else........................... return 'GT'
**
**  FORMAL PARAMETERS:
**
**      arg - argument to test
**	 lt - value to return if arg < 0
**	 eq - value to return if arg = 0
**	 gt - value to return if arg > 0
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**	if arg < 0, return value 'lt' passed in
**	if arg = 0, return value 'eq' passed in
**	if arg > 0, return value 'gt' passed in
**
**  SIGNAL CODES:
**
**  SIDE EFFECTS:
**
******************************************************************************/

#define ZERO_TEST_(arg, lt, eq, gt) (arg < 0 ? lt : arg == 0 ? eq : gt)


/******************************************************************************
**  $INIT_GET
**  $END_GET
**
**  FUNCTIONAL DESCRIPTION:
**
**  initializes a single "row" of a GET_ITMLST structure. This is required to
**  support the pcc port, as automatic run-time structure initialization is
**  not supported in pcc.
**
**  $END_GET may be used to terminate the list with zeros. It only uses the
**  first two formal parameters.
**
**  FORMAL PARAMETERS:
**
**  struct - the GET_ITMLST array of structures to be initialized.
**  ctr    - counter incremented by this macro to keep us from having to
**           keep track of the row index. Should be initialized to ZERO
**           before beginning the initialization!!!!!!!!!
**  code   - item code to be fetched
**  length - size of buffer passed
**  buffer - address to return itemcode into
**  retlen - address of longword to return the returned length into (or 0)
**  index  - index if item requested is a DDIF indexed item
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**  SIDE EFFECTS:
**
******************************************************************************/

#define INIT_GET_(struct,ctr,code,length,buffer,retlen,index) \
    struct[ctr].GetL_Code   = (unsigned long)    code;\
    struct[ctr].GetL_Length = (unsigned long)    length;\
    struct[ctr].GetA_Buffer = (char *)         (buffer);\
    struct[ctr].GetA_Retlen = (unsigned long *) (retlen);\
    struct[ctr].GetL_Index  = (unsigned long)    index;\
    ctr++    

#define END_GET_(struct,ctr)\
    struct[ctr].GetL_Code = 0;\
    struct[ctr].GetL_Length = 0;\
    struct[ctr].GetA_Buffer = 0;\
    struct[ctr].GetA_Retlen = 0;\
    struct[ctr].GetL_Index = 0;\
    ctr++

/******************************************************************************
**  $INIT_PUT
**  $END_PUT
**
**  FUNCTIONAL DESCRIPTION:
**
**  initializes a single "row" of a PUT_ITMLST structure. This is required to
**  support the pcc port, as automatic run-time structure initialization is
**  not supported in pcc.
**
**  $END_PUT may be used to terminate the list with zeros. It only uses the
**  first two formal parameters.
**
**  FORMAL PARAMETERS:
**
**  struct - the PUT_ITMLST array of structures to be initialized.
**  ctr    - counter incremented by this macro to keep us from having to
**           keep track of the row index. Should be initialized to ZERO
**           before beginning the initialization!!!!!!!!!
**  code   - item code to be written
**  length - size of buffer passed
**  buffer - address to get item from
**  index  - index if item requested is a DDIF indexed item
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**  SIDE EFFECTS:
**
******************************************************************************/
#define INIT_PUT_(struct,ctr,code,length,buffer,index) \
    struct[ctr].PutL_Code   = (unsigned long) code;\
    struct[ctr].PutL_Length = (unsigned long) length;\
    struct[ctr].PutA_Buffer = (char *) (buffer);\
    struct[ctr].PutL_Index  = (unsigned long) index;\
    ctr++    

#define END_PUT_(struct,ctr)\
    struct[ctr].PutL_Code = 0;\
    struct[ctr].PutL_Length = 0;\
    struct[ctr].PutA_Buffer = 0;\
    struct[ctr].PutL_Index = 0;\
    ctr++


/******************************************************************************
**  $INIT_CNFLST_PUT
**  $END_CNFLST_PUT
**
**  FUNCTIONAL DESCRIPTION:
**
**  initializes a single "row" of a CNFLST structure. 
**
**  $END_CNFLST_PUT may be used to terminate the list with zeros. 
**  It only uses the first two formal parameters.
**
**  FORMAL PARAMETERS:
**
**  struct - the PUT_CNFLST array of structures to be initialized.
**  ctr    - counter incremented by this macro to keep us from having to
**           keep track of the row index. Should be initialized to ZERO
**           before beginning the initialization!!!!!!!!!
**  code   - item code to be written
**  conf_domain - continuous or discrete
**  comparison_type_rule - allow/disallow/nochange
**  comparison_operator	- equal/noequal/lessthan/greaterthan/leq/geq
**			  between/outside
**
**  length - size of buffer passed
**  count - number of elements in the buffer
**  buffer - address to get item from
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**  SIDE EFFECTS:
**
******************************************************************************/
#define INIT_CNFLST_PUT_(struct,ctr,code,conf_domain,comp_rule,comp_op,length,count,buffer) \
    struct[ctr].PutL_Code   = (unsigned long int) code;\
    struct[ctr].PutL_ConfDomain  = (unsigned long int) conf_domain;\
    struct[ctr].PutL_CompRule  = (unsigned long int) comp_rule;\
    struct[ctr].PutL_CompOp  = (unsigned long int) comp_op;\
    struct[ctr].PutL_Length = (unsigned long int) length;\
    struct[ctr].PutL_Count = (unsigned long int) count;\
    struct[ctr].PutA_Buffer = (char *) (buffer);\
    ctr++    

#define END_CNFLST_PUT_(struct,ctr)\
    struct[ctr].PutL_Code   = 0;\
    struct[ctr].PutL_ConfDomain  = 0;\
    struct[ctr].PutL_CompRule  = 0;\
    struct[ctr].PutL_CompOp  = 0;\
    struct[ctr].PutL_Length = 0;\
    struct[ctr].PutL_Count = 0;\
    struct[ctr].PutA_Buffer = 0;\
    ctr++

/*
** add VMS_STATUS_SUCCESS_ macro so we don't have to include STSDEF.H
** This tests the lowbit for 0 or 1
*/
#define VMS_STATUS_SUCCESS_(code)  (((code) & 0x01 ) >> 0x00)


/*
** These macros stuff a value into a possibly unaligned integer or short
** Put in for the PMAX.
*/

#ifndef VMS

#define UNALIGNED_INT_LOAD_(address,value) {\
    *((char *)address+3) = value / (1 << 24);\
    *((char *)address+2) = value % (1 << 24) / (1 << 16);\
    *((char *)address+1) = value % (1 << 16) / (1 << 8);\
    *((char *)address) = value % (1 << 8); }

#define UNALIGNED_SHORT_LOAD_(address,value) {\
    *((char *)address+1) = value / (1 << 8);\
    *((char *)address) = value % (1 << 8); }

#endif

/******************************************************************************
**  ALIGN_BITS_
**
**  FUNCTIONAL DESCRIPTION:
**
**	Returns to number of bits needed to add to an address value
**	to align it to the modulo of the alignment value.
**
**  FORMAL PARAMETERS:
**
**	value	    Value to align to the above modulo.
**	alignment   Modulo alignment value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
******************************************************************************/
#define ALIGN_BITS_( value, alignment )	\
    ( ((alignment) - ((value) % (alignment))) % (alignment) )

/*
**  For Ultrix (and the non-VAX architectures Ultrix supports),
**  don't access longwords on non-longword aligned boundaries.
*/
#ifndef VMS
#define READ16_(address) \
   (*(( unsigned char *)(address)) + \
   (*(((unsigned char *)(address))+1) << 8) 

#define READ32_(address) \
   (unsigned int) (*((unsigned char *)(address)) + \
   ((unsigned int) (*(((unsigned char *)(address))+1)) << 8) + \
   ((unsigned int) (*(((unsigned char *)(address))+2)) << 16) + \
   ((unsigned int) (*(((unsigned char *)(address))+3)) << 24))

#define WRITE16_(address,value) {\
   *(( unsigned char *)(address))    =  ((short int)value)       & 0xff; \
   *(((unsigned char *)(address))+1) = (((short int)value) >> 8) & 0xff; }

#define AND_WRITE16_(address,value) {\
   *(( unsigned char *)(address))    &=  ((short int)value)       & 0xff; \
   *(((unsigned char *)(address))+1) &= (((short int)value) >> 8) & 0xff; }

#define XOR_WRITE16_(address,value) {\
   *(( unsigned char *)(address))    ^=  ((short int)value)       & 0xff; \
   *(((unsigned char *)(address))+1) ^= (((short int)value) >> 8) & 0xff; }

#define WRITE32_(address,value) {\
   *(( unsigned char *)(address))    =  ((int)value)        & 0xff; \
   *(((unsigned char *)(address))+1) = (((int)value) >> 8)  & 0xff; \
   *(((unsigned char *)(address))+2) = (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) = (((int)value) >> 24) & 0xff; }

#define AND_WRITE32_(address,value) {\
   *(( unsigned char *)(address))    &=  ((int)value)        & 0xff; \
   *(((unsigned char *)(address))+1) &= (((int)value) >> 8)  & 0xff; \
   *(((unsigned char *)(address))+2) &= (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) &= (((int)value) >> 24) & 0xff; }

#define XOR_WRITE32_(address,value) {\
   *(( unsigned char *)(address))    ^=  ((int)value)        & 0xff; \
   *(((unsigned char *)(address))+1) ^= (((int)value) >> 8)  & 0xff; \
   *(((unsigned char *)(address))+2) ^= (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) ^= (((int)value) >> 24) & 0xff; }
#else
#define READ16_(address) \
    (*((short int *)(address)))

#define READ32_(address) \
    (*((int *)(address)))

#define WRITE16_(address,value) \
    (*((short int *)(address)) = (short int)value)

#define AND_WRITE16_(address,value) \
    (*((short int *)(address)) &= (short int)value)

#define XOR_WRITE16_(address,value) \
    (*((short int *)(address)) ^= (short int)value)

#define WRITE32_(address,value) \
    (*((int *)(address)) = (int)value)

#define AND_WRITE32_(address,value) \
    (*((int *)(address)) &= (int)value)

#define XOR_WRITE32_(address,value) \
    (*((int *)(address)) ^= (int)value)
#endif


/*
**  MACRO definitions used frequently by the DDIF modules
*/

/* 
** return fid from document context block 
*/
#ifdef VMS
#define $$fid()			(dcb->DCB$L_FID)
#endif
#define FID_()			(dcb->DCB$L_FID)
/* 
** initialize dynamic string descriptor, taking dsc address as arg 
*/
#ifdef VMS
#define $$init_dyndsc( dsc )	{ \
		((struct dsc$descriptor *)dsc)->dsc$b_class = DSC$K_CLASS_D; \
		((struct dsc$descriptor *)dsc)->dsc$b_dtype = DSC$K_DTYPE_T; \
		((struct dsc$descriptor *)dsc)->dsc$w_length = 0; \
		((struct dsc$descriptor *)dsc)->dsc$a_pointer = 0; \
		}
#endif
#define INIT_DYNDSC_( dsc )	{ \
		((struct dsc$descriptor *)dsc)->dsc$b_class = DSC$K_CLASS_D; \
		((struct dsc$descriptor *)dsc)->dsc$b_dtype = DSC$K_DTYPE_T; \
		((struct dsc$descriptor *)dsc)->dsc$w_length = 0; \
		((struct dsc$descriptor *)dsc)->dsc$a_pointer = 0; \
		}
/* 
** test status value returned by DDIF  routines 
*/ 
#define	LOWBIT_TEST_( status )	if ( (status&1) != 1 ) ChfStop(1, status )

/*
** Macros and definitions used to verify frame attributes
*/
#define	VERIFY_UNDEFINED    0
#define VERIFY_ON	    1
#define VERIFY_OFF	    2

#define	VERIFY_ON_  ( ((flags & ImgM_VerifyOn) || \
		      (_ImgGetVerifyStatus() == VERIFY_ON)) && \
		      (_ImgGetVerifyStatus() != VERIFY_OFF) )

#define VERIFY_OFF_ ( _ImgGetVerifyStatus() == VERIFY_OFF )


/*
** Macros which define CDA entry points
*/
#if defined(NEW_CDA_CALLS)
#define CDA_ATTACH_ITEM_	    cda_attach_item
#define CDA_CLOSE_FILE_		    cda_close_file
#define CDA_CLOSE_STREAM_	    cda_close_stream
#define CDA_COPY_AGGREGATE_	    cda_copy_aggregate
#define CDA_CREATE_AGGREGATE_	    cda_create_aggregate
#define CDA_CREATE_FILE_	    cda_create_file
#define CDA_CREATE_ROOT_AGGREGATE_  cda_create_root_aggregate
#define CDA_CREATE_STREAM_	    cda_create_stream
#define CDA_DELETE_AGGREGATE_	    cda_delete_aggregate
#define CDA_DELETE_ROOT_AGGREGATE_  cda_delete_root_aggregate
#define CDA_DETACH_ITEM_	    cda_detach_item
#define CDA_ENTER_SCOPE_	    cda_enter_scope
#define CDA_ERASE_ITEM_		    cda_erase_item
#define CDA_EXCHANGE_ALLOCATION_    cda_exchange_allocation
#define CDA_GET_AGGREGATE_	    cda_get_aggregate
#define CDA_GET_ARRAY_SIZE_	    cda_get_array_size
#define CDA_INSERT_AGGREGATE_	    cda_insert_aggregate
#define CDA_LEAVE_SCOPE_	    cda_leave_scope
#define CDA_LOCATE_ITEM_	    cda_locate_item
#define CDA_NEXT_AGGREGATE_	    cda_next_aggregate
#define CDA_OPEN_FILE_		    cda_open_file
#define CDA_OPEN_STREAM_	    cda_open_stream
#define CDA_PUT_AGGREGATE_	    cda_put_aggregate
#define CDA_REMOVE_AGGREGATE_	    cda_remove_aggregate
#define CDA_STORE_ITEM_		    cda_store_item
#define DDIS_GET_PARSE_LOCATION_    ddis_get_parse_location
    /*
    ** external references for functions that aren't
    ** in CDA$PTP.H (the prototype header file)
    */
    unsigned long   cda_attach_item();
    unsigned long   cda_detach_item();
    unsigned long   ddis_get_parse_location();
#else
#define CDA_ATTACH_ITEM_	    cda$attach_item
#define CDA_CLOSE_FILE_		    cda$close_file
#define CDA_CLOSE_STREAM_	    cda$close_stream
#define CDA_COPY_AGGREGATE_	    cda$copy_aggregate
#define CDA_CREATE_AGGREGATE_	    cda$create_aggregate
#define CDA_CREATE_FILE_	    cda$create_file
#define CDA_CREATE_ROOT_AGGREGATE_  cda$create_root_aggregate
#define CDA_CREATE_STREAM_	    cda$create_stream
#define CDA_DELETE_AGGREGATE_	    cda$delete_aggregate
#define CDA_DELETE_ROOT_AGGREGATE_  cda$delete_root_aggregate
#define CDA_DETACH_ITEM_	    cda$detach_item
#define CDA_ENTER_SCOPE_	    cda$enter_scope
#define CDA_ERASE_ITEM_		    cda$erase_item
#define CDA_EXCHANGE_ALLOCATION_    cda$exchange_allocation
#define CDA_GET_AGGREGATE_	    cda$get_aggregate
#define CDA_GET_ARRAY_SIZE_	    cda$get_array_size
#define CDA_INSERT_AGGREGATE_	    cda$insert_aggregate
#define CDA_LEAVE_SCOPE_	    cda$leave_scope
#define CDA_LOCATE_ITEM_	    cda$locate_item
#define CDA_NEXT_AGGREGATE_	    cda$next_aggregate
#define CDA_OPEN_FILE_		    cda$open_file
#define CDA_OPEN_STREAM_	    cda$open_stream
#define CDA_PUT_AGGREGATE_	    cda$put_aggregate
#define CDA_REMOVE_AGGREGATE_	    cda$remove_aggregate
#define CDA_STORE_ITEM_		    cda$store_item
#define DDIS_GET_PARSE_LOCATION_    ddis$get_parse_location
    /*
    ** external references for functions that aren't
    ** in CDA$PTP.H (the prototype header file)
    */
    unsigned long   cda$attach_item();
    unsigned long   cda$detach_item();
    unsigned long   ddis$get_parse_location();
#endif
