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
static char *rcsid = "@(#)$RCSfile: mcc_asn1_objid.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/01 14:58:22 $";
#endif

/*
**++
**  FACILITY:
**
**       MCC_ASN1_SUPPLEMENT - additional ASN.1 data types and routines
**	 implemented by the DECrouter 2000 team.  These routines
**	 are called by MCC_ASN1.C, and are NOT in general callable
**	 by MCC xMMs.  This module was created so that ASN.1 s/w
**	 development could proceed somewhat concurrently in 
**	 the two groups while we continued to share code.
**
**  ABSTRACT:
**
**	This module contains Object Id data type implementation,
**	and some other routines.  
**
**  AUTHORS:
**
**       Alan Soper
**
**
**  CREATION DATE:      4/20/88
**
**  MODIFICATION HISTORY:
**
**  00715    10-Aug-1989     REJK
**          fix precedence errors in rtns ilv_encode_object and ilv_verify_tag
**          as noted in MCC_TOOLS QAR 15
**
**  00900   8-Jan-1990      REJK
**          remove "$" from "$mcc$asn..." function declarations for callable
**          mcc, also from any function calls.
**  01003   7-Jun-1990      REJK
**          portability naming changes
**          17-Aug-1990     JAS
**  x1.01.0 10-Sep-1990      REJK
**          change CMIP conditional and include files
**
**  x1.2.0  31-Jul-1991 REJK and Arun
**          change CMIP conditional and include file list. 
**          Change name of module from mcc_asn1_soper. some general cleanup.
**--
**  ---------------------------------------------------------------------------
**  October 1991 - Converted for use with the SNMP Protocol Engine of
**                 the Common Agent.  Included files had their names changed
**                 to avoid conflict with the MCC files of the same name.
**/

typedef int BOOLEAN;

#include "snmppe_interface_def.h"
#include "snmppe_descrip.h"
#include "snmppe_msg.h"

#include "asn_def.h"


unsigned char *ilv_get_len(), *ilv_fnd_eoc(), *ilv_put_len();

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Decode an Object Identifier.
**	Write the components as an array of integers in
**	the output buffer.
**
**  FORMAL PARAMETERS:
**
**      p_Buf: Where we are in the message.
**	Len: The length of the object id.
**	p_ValueBuf: Where to write
**	ValueBufSize: Size of the above.
**	p_LengthWritten: The length written in bytes.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      [@description_or_none@]
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
int ilv_decode_object_id(p_Buf,Len,p_ValueBuf,ValueBufSize,p_LengthWritten)
int Len,ValueBufSize,*p_LengthWritten;
unsigned char *p_Buf, *p_ValueBuf;
{
     int status;
     int LengthDecoded=0; /* Length of one cpt */
     int LengthConsumed=0; /* Amount of ilv value read at this point */
     extern int ilv_decode_obj_id_cpt();


     *p_LengthWritten = 0;

     if (Len == 0)
	 return MCC_S_NORMAL;	/* Are we allowed to have nothing ? */

     /* Only accept components which take up at most 4 bytes (28bits of value)	*/
     /* Hence we can place result in an array of words in the buff		*/
     /* First one is a special case for which max is 2**28-1-80)		*/

    /* First component is really two and so is treated separately */
    /* Need at least two words to put this in */
    if (ValueBufSize < 8)
	return MCC_S_ILVTOOBIG;
    status = ilv_decode_obj_id_cpt(&LengthDecoded,p_Buf+LengthConsumed,Len,p_ValueBuf+*p_LengthWritten,MCC_K_TRUE);
    if (status != MCC_S_NORMAL)
	return status;

    *p_LengthWritten += 8;
    LengthConsumed += LengthDecoded;

    while (LengthConsumed < Len)
	{
	    /* Check we've got room to write this */
	    if (ValueBufSize < *p_LengthWritten + 4)
		return MCC_S_ILVTOOBIG;
	    status = ilv_decode_obj_id_cpt(&LengthDecoded,p_Buf+LengthConsumed,Len-LengthConsumed,
					    p_ValueBuf+*p_LengthWritten,MCC_K_FALSE);
	    if (status != MCC_S_NORMAL)
		return status;
	    *p_LengthWritten += 4;
	    LengthConsumed += LengthDecoded;
	}


    if (LengthConsumed != Len) /* We should have used the length up precisely */
	return(MCC_S_ILVSYNTAXERROR);

    return MCC_S_NORMAL;
}
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Decode one object_id_cpt from ASN1 into a word
**	in the buffer.
**
**  FORMAL PARAMETERS:
**
**      p_LengthDecoded: Amount of message decoded.
**	p_Buf: Where we are in the buffer.
**	Len: Length remaining of object id.
**	
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      [@description_or_none@]
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
int ilv_decode_obj_id_cpt(p_LengthDecoded,p_Buf,Len,p_ValueBuf,First)
int Len, *p_LengthDecoded;
unsigned char *p_Buf, *p_ValueBuf;
BOOLEAN First;
{
    unsigned char temp_int[4];
    int native_int;
    int i,j;

    for (i=0; i<4; i++) temp_int[i] = 0;

    if (Len <=0) /* This should never happen */
	return MCC_S_FATAL;

    /* Must be encoded in the minimum number of bytes */
    if (*p_Buf == 0x80)
	return MCC_S_ILVSYNTAXERROR ;

    i = 0;
    while (*(p_Buf+i) & 0x80 )
	{
	    temp_int[i] = *(p_Buf+i) & 0x7f;

	    i++;
	    if (i>2)	/* if i > 2 then last byte wont fit */
		return MCC_S_ILVTOOBIG;
	    if (i>(Len-1))
		return MCC_S_ILVSYNTAXERROR;
	}

    /* And the last byte */
    temp_int[i] = *(p_Buf+i);
    i++;    /* So i give the number of bytes of encoding for this cpt */

    *p_LengthDecoded = i;

    /* Store temporarily as a native value */
    native_int = 0;
    for (j=0;  j<i;  j++)
	{
	    native_int = temp_int[j] + (native_int << 7);
	}

    /* Then output */
    if (!First)
	{
	    *(int *)p_ValueBuf = native_int;
	}
    else
	{
	    int x,y;

	    if (native_int>=80)
		{
		    x=2;
		    y=native_int-2*40;
		}
	    else if (native_int<80 && native_int>=40)
		{
		    x=1;
		    y=native_int-1*40;
		}
	    else
		{
		    x=0;
		    y=native_int;
		}

	    *(int *)p_ValueBuf = x;
	    *(int *)(p_ValueBuf+4) = y;
	}
    return MCC_S_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Encode an Object Identifier.
**
**  FORMAL PARAMETERS:
**
**      p_LengthEncoded: Pointer to number to record length encoded.
**	p_Buf: Where to put it.
**	NativeValueSize: The length of our data to encode.
**	p_NativeValue: And where it is.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      MCC_S_NORMAL
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
int ilv_encode_object_id(p_LengthEncoded,p_Buf,NativeValueSize,p_NativeValue)
int *p_LengthEncoded, NativeValueSize;
unsigned char *p_Buf,*p_NativeValue;
{
    int consumed= 0;
    int status;
    extern int ilv_encode_object_id_cpt();


    /* Value size must be a multiple of 4 */
    if ((NativeValueSize & 0x3) != 0)
	return MCC_S_ILVENCODING;

    /* We know things will fit in the message*/

    /* Encode first two values as one subidentifier */
    {
	int x,y;

	x = *(int *)p_NativeValue;
	if (x>2)
	    return MCC_S_ILVENCODING;

	if (NativeValueSize > 4)    /* Is there a second value? */
	    {
		y = *(int *)(p_NativeValue+4);
		if (x<2 && y>39)    /* Check restrictions */
		    return MCC_S_ILVENCODING;
		consumed += 8;
	    }
	else
	    {
		y=0;
		consumed += 4;
	    }
	status = ilv_encode_object_id_cpt(p_LengthEncoded,p_Buf+*p_LengthEncoded,
					x*40+y);
	if (status != MCC_S_NORMAL)
	    return status;

    }

    while (consumed < NativeValueSize)
	{
	    status = ilv_encode_object_id_cpt(p_LengthEncoded,p_Buf+*p_LengthEncoded,
						*(int *)(p_NativeValue+consumed));
	    if (status != MCC_S_NORMAL)
		return status;
	    consumed += 4;
	}

    return MCC_S_NORMAL;	    
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Encode one subidentifier.
** 
**
**  FORMAL PARAMETERS:
**
**      LengthEncoded: Address of an integer to which we add
**	the number of bytes we encoded.
**	p_Buf: Where we put it.
**	NativeInt:The number to add.
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
**      MCC_S_NORMAL or
**	MCC_S_????
**
**  SIDE EFFECTS:
**
**      [@description_or_none@]
**
**--
**/
int ilv_encode_object_id_cpt(p_LengthEncoded,p_Buf,NativeInt)
int *p_LengthEncoded, NativeInt;
unsigned char *p_Buf;
{
    int i;  /* Number of bytes required*/
    int j;		

    /* How many bytes will we need? */
    if (NativeInt > 0xfffffff)
	return MCC_S_ILVENCODING;
    else if (NativeInt > 0x1fffff)
	i=4;
    else if (NativeInt > 0x3fff)
	i=3;
    else if (NativeInt > 0x7f)
	i=2;
    else
	i=1;

    /* Now put this value in the buffer */
    for (j=1;  j<=i;  j++)
	{
	    unsigned char oid_byte;

	    oid_byte = (unsigned char)(NativeInt & 0x7f); /* First 7 bits */
	    if (j!=1)
		oid_byte |= 0x80;	/* Set the top bit on all bytes but the last */
	    *(p_Buf + i-j) = oid_byte;
	    NativeInt = NativeInt >> 7;
	};

    *p_LengthEncoded += i; /* Increase the count of what has been encoded */

    return MCC_S_NORMAL;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Verify the basic tag length value structure of ASN1 encodings.
**
**  FORMAL PARAMETERS:
**
**      Buffer: the address of the ilv.
**	Length: the length in which the above must fit (doesn't have to
**	exactly).
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
**      Number of bytes occupied by this value or failure (-1).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
int mcc_ilv_verify_encoding(buffer, length)
char	*buffer;    /* Address of ilv */
int length; /* Number of bytes left for this value */
{
    int consumed=0, count=0, length_of_value = 0;
    BOOLEAN constructed=MCC_K_FALSE, indef = MCC_K_FALSE;
    extern int ilv_verify_tag();
    extern int ilv_verify_length();


    /* Must have something since ilv is expected */
    if (count == length) /* If nothing left fail */
	return -1;
    consumed = ilv_verify_tag(buffer+count,length-count,&constructed);
    if (consumed < 0)
	return -1;
    count += consumed;

    consumed = ilv_verify_length(buffer+count,length-count,&indef,&length_of_value);
    if (consumed < 0)
	return -1;
    count += consumed;


    /* If indef it has to be constructed */
    if (!constructed && indef)
	return -1;


    /* In all cases the value must fit in the length */
    if ( length_of_value + count > length)
	return -1;

    /* If it is a primitive value we have finished */
    if (!constructed)
	return count + length_of_value; /* Return amount consumed */

    /* For constructed values we need to call ourselves until we have */
    /* consumed length_of_value or as much of the buffer as we can */
    consumed = 0;

    /* If there is no value, return */
    if (length_of_value == 0)
	return count;

    /* Loop ad infinitum */
    while (MCC_K_TRUE)
	{
	    int length_of_ilv;

	    length_of_ilv = mcc_ilv_verify_encoding(buffer+count,
					length_of_value-consumed);
	    if (length_of_ilv < 0)
		return -1;
	    consumed += length_of_ilv;
	    count += length_of_ilv;


	    /* If we have exactly consumed the value length, everything  */
	    /* is fine and we have finished. If a ilv doesn't completely */
	    /* fit in, the error will be picked up a subsequent routine  */
	    /* call */
	    if (!indef)
		{
		if (consumed == length_of_value)
		    return count;
		}
	    else
		{
		    /* We only have an upper bound on the length
		    ** which is fed into and checked in each routine
		    ** call. We look for EOC bytes. NB our length of
		    ** value upper bound took these into account so
		    ** we always have room for them.
		    */
		    if (*(buffer+count)==0 && *(buffer+count+1)==0)
			return count + 2;

		}
	}
}
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Verify the structure of a tag. Find out how long it is
**	and whether it indicates a costructed value.
**
**  FORMAL PARAMETERS:
**
**      buffer: address of tag.
**	length: length in which tag must fit.
**	constructed: is it?

**
**  IMPLICIT INPUTS:
**
**      None
**
**  IMPLICIT OUTPUTS:
**
**      None
**
**  FUNCTION VALUE:
**
**      How long the tag was or failure.
**
**  SIDE EFFECTS:
**
**      None
**
**--
**/
int	ilv_verify_tag(buffer, length, constructed)
char *buffer;
int length;
BOOLEAN *constructed;
{
    int count=0;
    BOOLEAN large=MCC_K_FALSE;

    /* We know we have at least one byte */
    /* Is it constructed? */
    if (*(buffer+count) & 0x20)
	*constructed = MCC_K_TRUE;

    /* Is the identifier greater than 30 */
    if ((*(buffer+count) & 0x1f) == 31)
	large = MCC_K_TRUE;

    /* Finished with the first byte of the tag */
    count += 1;
    if (count == length) /* Must have some following length of value bytes */
	return -1;

    if (large == MCC_K_TRUE)
	{
	    /* The first byte of the tag value can't have bits seven to one zero */ 
	    if (! (*(buffer + count) & 0x7f ))
		return -1;

	    /* We need to find the number of bytes in the tag	*/
	    /* for longer tags					*/
	    while (*(buffer + count) & 0x80)
		{
		    count += 1;
		    if (count == length)
			return -1;
		}
	    /* And the last byte of the tag */
	    count += 1;
	    if (count == length)
		return -1;
	}

    return count;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Verify the structure of the length octets,
**	find out how long they are and whether they
**	fit in the available space. Is it an indefinite
**	length encoding?
**
**  FORMAL PARAMETERS:
**
**      buffer: address of length octets.
**	length: length in which these must live.
**	indef:
**	length_of_value: how long the value is.
**	For the indef length case we set this to
**	the upper bound.
**	
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
**      Number of bytes consumed or failure.
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
int ilv_verify_length(buffer, length, indef, length_of_value)
BOOLEAN *indef;
char *buffer;
int *length_of_value, length;
{
    int length_of_length=0,count = 0;

    /* Must have at least one byte */
    if (count == length)
	return -1;

    /* Now for the length octets */
    if (*(buffer+count) & 0x80)
	{
	    if (*(buffer+count) & 0x7f)
		{
		    /* Can't be all 1's */

                    /* On SunOS, we need to cast char to unsigned char */
                    /* before comparing to 0xff. Otherwise will get    */
                    /*                                                 */
                    /* warning: constant 255 is out of range of char   */
                    /*          comparison                             */
                    /* warning: result of comparison is always false   */

#if defined(sun) || defined(sparc)
		    if ((unsigned char) *(buffer+count) == 0xff)
#else
		    if (*(buffer+count) == 0xff)
#endif
			return -1;

		    length_of_length = (*(buffer+count) & 0x7f);

		    /* We must have something following by definition */
		    count += 1;
		    if (count == length)
			return -1;


		    /* Check the length_of_length fits in the buffer */
		    if (count + length_of_length > length)
			return -1;

		    /* Now get the length value reversing the order of the bytes. */
		    /* The inequality above puts bounds on this - we must be able */
		    /* to store this in a word. It could be zero. */
		    {
			char temp_int[4];
			int i;

			for (i=0;  i<4;  i++)
			    {
			    if (i<length_of_length)
				temp_int[i] = *(buffer+count+length_of_length-i-1);
			    else
				temp_int[i] = 0;
			    };

			*length_of_value = *(int *)temp_int;
		    }
		    count += length_of_length;

		    /* If no value we've finished */
		    if (*length_of_value == 0)
			return count;
		}
	    else
		{
		    *indef = MCC_K_TRUE;

		    count += 1;
		    if (count == length)
			return -1;

		    /* This is the max possible length rather than a real one */
		    *length_of_value = length - count - 2;/* NB EOC */
		    /* Check that there is room for EOC */
		    if (*length_of_value < 0)
			return -1;
		    if (*length_of_value == 0)
			return count;

		}

	}
    else
	{
	    *length_of_value = *(buffer+count) & 0x7f;
	    count += 1;
	    if (count == length)
		if (*length_of_value == 0)
		    return count;   /* Things are OK and we've finished */
		else
		    return -1;
	}    
    return count;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      We have just parsed a tag and want to find
**	the position and length of the value.
**
**  FORMAL PARAMETERS:
**
**      [@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**      [@description_or_none@]
**
**  IMPLICIT OUTPUTS:
**
**      [@description_or_none@]
**
**  {@function_value_or_completion_codes@}
**
**      [@description_or_none@]
**
**  SIDE EFFECTS:
**
**      [@description_or_none@]
**
**--
**/
int mcc_ilv_locate_value(p_Ctx, value, value_size)
unsigned char **value;
int *value_size;
struct ASNContext *p_Ctx;
{
unsigned char *p_Buf;
int status, len;
short TOS;

if (p_Ctx->AtTag == MCC_K_TRUE) return(MCC_S_ILVNOTATVALUE);

status = MCC_S_NORMAL;
p_Buf = p_Ctx->p_Pos;
TOS = p_Ctx->TOS;

/* ensure that you haven't already parsed the entire ILV value */
if (TOS == 0)	/* if not inside any constructor */
    {
    if (p_Ctx->p_Pos > p_Ctx->p_Begin && p_Ctx->AtTag)
	return(MCC_S_ILVALREADYDONE);
    };


p_Buf = ilv_get_len( p_Buf, &len);

/* If it's not an indefinite length constructor we are finished */
if (len >= 0)
    {
	*value = p_Buf;
	*value_size = len;
	return status;
    }
/*
** For the indefinite length case we find the length by looking for the
** end of the constructor and then going back two bytes.
*/
*value = p_Buf;

p_Buf = ilv_fnd_eoc(p_Buf) - 2; 
*value_size = p_Buf - *value;

return status;

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	add_val encodes this ILV value at the end
**	of what' already been written in the current
**	constructor.
**
**  FORMAL PARAMETERS:
**
**	p_Ctx	    		- context block
**	Tag	    		- identifier encoding
**	p_Value 		- address of asn.1 value
**	ValueSize		- size of asn.1 value in bytes
**
**  IMPLICIT INPUTS:
**
**	context block must have been initialized
**	ILV value must not have been completed already
**
**  IMPLICIT OUTPUTS:
**
**	ILV value appended to current constructor
**	context block updated
**
**  COMPLETION CODES:
**
**	MCC_S_ILVTOOBIG	    	- ILV value does not fit within buffer
**	MCC_S_ILVALREADYDONE    	- ILV value has already been completed
**	<others TBS>
**
**  SIDE EFFECTS:
**
**	none
**--
**/

mcc_ilv_add_val( p_Ctx, Tag, p_Value, ValueSize )
struct ASNContext *p_Ctx;
unsigned int Tag;
char *p_Value;
int ValueSize;
{
unsigned char *p_Buf, *p_BufLim;
unsigned int  *p_Tag;
extern int  ilv_verify_tag();
extern void *memcpy();
extern unsigned int ilv_tag_len();

if (p_Ctx->BuildParse == 1) return(MCC_S_ILVBUILDORPARSE);

/* ensure that you haven't already built the entire ILV value */
if (p_Ctx->TOS == 0 && p_Ctx->p_Pos > p_Ctx->p_Begin ) 
	return(MCC_S_ILVALREADYDONE);

/* pick up where we left off */

p_Buf = p_Ctx->p_Pos;
p_BufLim = p_Ctx->p_Begin + p_Ctx->BufSize;

/* if there is a tag identifier provided, then slap it on first */

/* append tag to ILV value and advance pointer */

p_Tag = (unsigned int *)p_Buf;
*p_Tag = Tag;	/* write out tag */
p_Buf += ilv_tag_len( Tag );

/* output value */

/* does this work for BITSTRINGS???? */
if ((p_Value) /* if caller provided  value buffer address */
    && (p_Buf + ValueSize > p_BufLim)) /* if value don't fit*/
	return( MCC_S_ILVTOOBIG );

/* copy value into ILV value buffer, here treated as an octet string */

p_Buf = ilv_put_len( p_Buf, ValueSize );
/* copy to ... from .... length .... */
memcpy( p_Buf, p_Value, ValueSize ); 
p_Buf += ValueSize;    /* advance buffer pointer */

/* if not indef-length cons */
if (p_Ctx->TOS > 0 && p_Ctx->ConsLen[p_Ctx->TOS] > -1)
    /* add in size of this encoding to constructor length */
    p_Ctx->ConsLen[p_Ctx->TOS] += (p_Buf - p_Ctx->p_Pos);

p_Ctx->p_Pos = p_Buf;
return(MCC_S_NORMAL);

}

int ilv_insert_obj_id_length (p_buff, p_str_len)
char *p_buff;
int  *p_str_len;

{
char *move_from, *move_to;
int move_by;

    if (*p_str_len < 0x80)
	move_by =1;
    else if (*p_str_len < 0x100)
	move_by =2;
    else move_by = 3;

    move_from = p_buff + *p_str_len-1;
    move_to = move_from + move_by;
    while (move_from >= p_buff)
	*move_to-- = *move_from--;
    move_to = p_buff;
    ilv_put_len (move_to, *p_str_len);
    *p_str_len += move_by;
    return (1);
}

