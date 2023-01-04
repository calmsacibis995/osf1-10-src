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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: mcc_asn1_extra.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:23:12 $";
#endif
/* module MCC_ASN1_EXTRA "X1.2.0" */
/*                                                                          **
**  Copyright (c) Digital Equipment Corporation, 1990                       **
**  All Rights Reserved.  Unpublished rights reserved                       **
**  under the copyright laws of the United States.                          **
**                                                                          **
**  The software contained on this media is proprietary                     **
**  to and embodies the confidential technology of                          **
**  Digital Equipment Corporation.  Possession, use,                        **
**  duplication or dissemination of the software and                        **
**  media is authorized only pursuant to a valid written                    **
**  license from Digital Equipment Corporation.                             **
**                                                                          **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or                         **
**  disclosure by the U.S. Government is subject to                         **
**  restrictions as set forth in Subparagraph (c)(1)(ii)                    **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as                          **
**  applicable.                                                             **
**                                                                          **
**                                                                          **
**  MODULE MCC_ASN1_EXTRA                                                   **
**                                                                          **
** HISTORY                                                                  **
**                                                                          **
** 07-Jun-1990  REJK portability name changes                               **
** 17-Aug-1990  JAS Port to pcc/Ultrix                                      **
** 10-Sep-1990  REJK change CMIP conditional and include files              **
** ------------------------------------------------------------------------ **
** October 1991 - Converted for use with SNMP Protocol Engine of the
**                Common Agent.  Included header file names were changed
**                to avoid MCC files of the same name.  Minor bug fixes
**                detected by Saber were applied.
**                                                                          **
*/


#include "snmppe_descrip.h"
#include "snmppe_interface_def.h"
#include "snmppe_msg.h"

#include "asn_def.h"


/**  
** 
**
**  FORMAL DESCRIPTION:
**  
**      mcc_asn_append_params provides the caller with the ability 
**	to append one tlv encoded buffer to another. The user needs 
**	only pass the 2 descriptors, one for each buffer. The tlv 
**	buffer can be either carved from static or dynamic memory.  
**
**
**  FORMAL PARAMETERS:
**
**	source_tlv    - address to the descriptor of the source tlv buffer
**			which will be appended to the destination tlv
**			buffer.	
**	dest_tlv      - address to the descriptor of the destination tlv 
**			buffer.
**
**
**  IMPLICIT INPUTS:
**
**       source_tlv must contain a complete constructed encoding (i.e. SEQUENCE/SET)
**	 dest_tlv must contain a complete constructed encoding 
**    
**  IMPLICIT OUTPUTS:
**
**       dest_tlv descriptor points to longer encoding
**	 if reallocation is necessary and possible, then dest_tlv points to 
**	 newly allocated TLV buffer and old TLV buffer has been deallocated
**
**  ROUTINE VALUE:
**
**      MCC_S_NORMAL	  - Successful encoding         
** 	MCC_S_INV_DESC	  - invalid destination descriptor
**	MCC_S_ILVTOOBIG	    - destination buffer not big enough 
**            
**  SIDE EFFECTS:
**
**/



mcc_asn_append_params(source_tlv,dest_tlv)

MCC_T_Descriptor	*source_tlv,*dest_tlv;

{
                                                
unsigned char	*ilv_skip_value(),*ilv_get_tag(),*ilv_get_len();
int		mcc_asn_length();
int		s_len,d_len,d_actual_len,total_len;
unsigned char 	*d_buf,*s_buf;
int		status;
extern void     *memcpy();           
extern unsigned char ilv_put_len();

unsigned int	tag;

/* validate buffers */       

status = MCC_S_NORMAL;

if (source_tlv == MCC_K_NULL_PTR)
   return (MCC_S_INV_DESC);
  else
   if (source_tlv -> mcc_a_pointer == MCC_K_NULL_PTR)  
      return (MCC_S_INV_DESC);
                      
if (dest_tlv == MCC_K_NULL_PTR)
   return (MCC_S_INV_DESC);
  else
   if (dest_tlv -> mcc_a_pointer == MCC_K_NULL_PTR)
       return (MCC_S_INV_DESC);

d_len = mcc_asn_length(dest_tlv);
d_buf = (unsigned char *)dest_tlv -> mcc_a_pointer;
s_buf = (unsigned char *)source_tlv -> mcc_a_pointer;
s_len = mcc_asn_length(source_tlv);

if (s_len == 0) /* if no source TLV */
    {
    dest_tlv->mcc_w_curlen = d_len;
    return( MCC_S_NORMAL );  /* nothing to do */
    };

if (d_len == 0)	/* if there was nothing in the destination TLV buffer */
    {
    /* Now s_len and s_buf should be ready to go so...... */ 	
    /* just copy the entire source TLV into the destination buffer */
    memcpy(d_buf,s_buf,s_len);
    dest_tlv->mcc_w_curlen = s_len;
    return( MCC_S_NORMAL );
    };

/* make sure that source and destination are both constructed encodings */

s_buf = ilv_get_tag(s_buf,&tag);
if (!(tag & MCC_K_ASN_FORM_CONS))
    return( MCC_S_ILVNOTCONS );

ilv_get_tag(d_buf, &tag );
if (!(tag & MCC_K_ASN_FORM_CONS))
    return( MCC_S_ILVNOTCONS );

/* if source and destination are both non-null, we concatenate the
** VALUE (not tag or length) portion of the source to the destination and then we
** add the size of the source value into the LENGTH field of the destination
*/

/* must fix source currency ptrs to point to the value of source
	   buffer - that's the actual tlv data buffer		*/
s_buf = ilv_get_len(s_buf,&s_len);
total_len = s_len + d_len;

if (total_len > dest_tlv -> mcc_w_maxstrlen) 
    return (MCC_S_ILVTOOBIG);
	   
/* Everything can fit into the dest. buffer. Just set up 
**      necessary currency ptrs. 			
*/

/* must get to the end of the dest buffer */

d_buf = ilv_skip_value(d_buf);

/* Now the actual appending of the source tlv buffer to the destination
   buffer */

/* Now s_len and s_buf should be ready to go so...... */ 	

memcpy(d_buf,s_buf,s_len);

/* must put new length into dest_tlv = ecoded values in s_buf and d_buf   */

d_buf = (unsigned char *)dest_tlv -> mcc_a_pointer;
d_buf = ilv_get_tag(d_buf,&tag);     

/* I'm deliberately not assigning tlv_get_len back to d_buf because I
**      don't want d_buf to point to the last byte in the length field */
	
ilv_get_len (d_buf,&d_actual_len);

/* must get the size of the encoding that was in d_buf. The original
**      length was not overwritten yet.                   
*/

/*** ASSUMPTION *** all constructor length octets and tag octets are same length 
**** otherwise this line of code is wrong and should be d_len + s_actual_len 
****/

total_len = s_len + d_actual_len;

/* must determine the length size so to position the pointer correctly */
/* -4- in front of total_len says that we are filling in constructor length */
   
ilv_put_len(d_buf, -4-total_len );

dest_tlv->mcc_w_curlen = mcc_asn_length( dest_tlv );

return(MCC_S_NORMAL);
}




/**  
** 
**
**  FORMAL DESCRIPTION:
**  
**      mcc_asn_copy_param provides the caller with the ability 
**	to append a single TLV-encoded parameter from one TLV encoding
**	to the end of another TLV encoding. The user must pass
**	TLV context blocks, one for each TLV encoding buffer. The TLV
**	buffer can be either static or dynamic, but no memory allocation
**	is done by this routine.  
**
**
**  FORMAL PARAMETERS:
**
**	source_tlv_ctx	- context block address for source tlv buffer.
**	dest_tlv_ctx    - context block address for destination tlv buffer.
**
**  IMPLICIT INPUTS:
**
**       caller must have already positioned at parameter in source TLV buffer
**	 using either mcc_ilv_get_ID or mcc_ilv_fnd_id.
**
**	 caller must have already set up destination context block using
**	 mcc_ilv_put_PARAM_BEGIN.
**    
**  IMPLICIT OUTPUTS:
**
**       equivalent of a PUT is done to the destination parameter list buffer
**	 source context block is modified as if get_ref() call had occurred.
**	 source TLV buffer is unchanged.
**
**  ROUTINE VALUE:
**
**      MCC_S_NORMAL	  - Successful encoding         
** 	MCC_S_INV_DESC	  - invalid descriptor
**      MCC_S_ILVTOOBIG	  - destination buffer wasn't big enough 
**	MCC_S_ILVNOTATVALUE - caller not positioned at value in source TLV context
**            
**  SIDE EFFECTS:
**
**/


mcc_asn_copy_param( source_tlv_ctx, dest_tlv_ctx )
struct ASNContext *source_tlv_ctx, *dest_tlv_ctx;
{
int ParamLen, ParamTag;
int status;
unsigned char *p_SourceBuf, *p_DestBuf;

unsigned char *ilv_get_len();
extern   int  mcc_asn_put_ref();

if (source_tlv_ctx->BuildParse == 0 || dest_tlv_ctx->BuildParse == 1)
    return( MCC_S_ILVBUILDORPARSE );

if (source_tlv_ctx->AtTag) /* if not positioned at a value */
    return( MCC_S_ILVNOTATVALUE );

ParamTag = source_tlv_ctx->LastTag; /* tag returned by previous get_tag or fnd_tag */

/* get length of parameter value and address of 1st octet of parameter value */
p_SourceBuf = ilv_get_len( source_tlv_ctx->p_Pos, &ParamLen );	
p_DestBuf = dest_tlv_ctx->p_Pos;

/* now do a byte copy from source to destination, whether primitive or constructed */

if (ParamTag == MCC_K_ASN_DT_OCTETSTRING)   /* if it is universal Octet string no need to pretend */
    {
    status = mcc_asn_put_ref(
                                dest_tlv_ctx,       /*destination TLV buffer */
                                ParamTag,           
                                MCC_K_ASN_DT_OCTETSTRING, /*treat it as byte string */
                                p_SourceBuf,        /* coy direct from the source TLV encoding */
                                ParamLen );         /* # bytes to copy */
    }
else        /* else pretend it is primitive, and treat it as a byte string */
    {
    status = mcc_asn_put_ref( 
	                            dest_tlv_ctx,		    /* destination TLV buffer */
                        	    (ParamTag & ~MCC_K_ASN_FORM_CONS),  /* pretend it's a primitive, whether it is or not*/
                        	    -MCC_K_ASN_DT_OCTETSTRING,    /* treat it as a byte string */
                        	    p_SourceBuf,		    /* copy direct from the source TLV encoding */
                                ParamLen );			    /* # of bytes to copy */
    };

if (status == MCC_S_NORMAL )    /* if it succeeded */
    {
    if (ParamTag & MCC_K_ASN_FORM_CONS) /* if form is constructed */
        *p_DestBuf |= MCC_K_ASN_FORM_CONS;	/* set form bit in target encoding */

    /* update parse context block appropriately */

    source_tlv_ctx->AtTag = MCC_K_TRUE; /* positioned at a tag, not a value */
    source_tlv_ctx->p_Pos = p_SourceBuf + ParamLen;
    };

return(status);
}
