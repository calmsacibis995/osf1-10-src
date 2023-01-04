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
static char *rcsid = "@(#)$RCSfile: mcc_asn1.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:22:59 $";
#endif
/* module MCC_ASN1 "x1.2.0" */

/*                                                                          **
**  Copyright (c) Digital Equipment Corporation, 1987, 1990                 **
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
**++
**  FACILITY:
**
**       MCC_ASN1 - implements subset of ISO ASN.1 "type-length-value" 
**	encoding and decoding of variable-structure data for MCC.  
**	Routines provided by this module are callable by MCC xMMs, but
**	MUST NOT be used for MCC input and output parameter lists --
**	use MCC_ILV module routines for this.
**
**  ABSTRACT:
**
**	This code implements all ASN.1 s/w needed for MCC V1.0.  
**	The design used in this module was influenced by several other designs:
**
**		RBMS ASN.1 protocol parser
**		prototype FTAM ASN.1 protocol parser
**		NBS ASN.1 utilities and access routines
**		DDIS ASN.1 utilities and access routines
**
**	However, MCC ASN.1 access routines permit the application to
**	cope with partially defined syntaxes.  For example, some applications
**	must be able to build and parse parameters which have never been
**	seen before, to avoid release synchronization problems between MCC
**	and products containing network agents.  Also, some applications
**	(i.e. MCC_ILV) have to support use of data type information which
**	was retrieved from a data dictionary.  Because these routines
**	allow the caller to supply the data type, these routines
**	can be used in either of these circumstances, whereas all of the
**	above tools could not be used in these circumstances.
**
**			NOTE
**
**		If DDIS functionality and schedule converge with ours,
**		This module will be converted to DDIS calls.  However,
**		There is a significant chance that this will never happen.
**
**  ASSUMPTIONS:
**
**	ASN.1 Standard is being subsetted as follows:
**
**	- during encoding, only definite-length format is used.  However,
**	both definite- and indefinite-length formats are supported when
**	decoding a message.
**
**	- at present the ObjectDesc data type is not supported.  
**
**	- The maximum length of a definite-length constructed encoding is 
**	32700 octets.  It is possible to construct longer encodings, but
**	the indefinite-length constructed encoding format must be used
**	so that the entire message does not have to be generated before
**	the length encodings are determined.  The option to use 
**	indefinite-length encodings is not currently supported, but
**	the code for it is written.
**
**	- The nesting level of constructors is being limited to the value
**	MCC_K_ASN_NESTCONS.
**
**
**
**  AUTHORS:
**
**       Ben England
**
**
**  CREATION DATE:      7/28/87
**
**  MODIFICATION HISTORY:
**
**	6/1/88 Context block structure has become opaque to callers, only ILV and ASN routines
**	know what it is.  mcc_asn_tag() routine has variable number of arguments, allowing
**	defaulting for form and class.
**
**  X0.5.0-1 drf    add ident, change entry MCC__ASN1_POSITION to MCC__ASN1_POSITION
**  X0.5.0-2 drf    add include for mcc_interface_old.h
**  X0.08.4  rejk   fix asn_put_ref and asn_get_ref according to instructions.
**  x0.09.0  rejk   remove _ from _mcc_ for callable mcc, comment out mcc__asn1_position (obsolete)
**  x0.09.1  rejk   fix bitstring encoding, use Arun's fixes for asn_put_ref and get_ref
**  x0.10.3  rejk   portability name changes
**  x0.10.5  jas    Port to pcc/ultrix
**  x0.11.0  rejk   decode any non-zero boolean as true, 1.
**  x1.01.0  rejk   change CMIP conditional NAME and cmip include file name
**  x1.2.0   rejk   note: mcc_asn_tag() routine no longer has variable number of args, or defaults.
**                  We now support 3 kinds of real number encode and decode:  G-Float on VMS--must
**                  compile this module with the /DEFINE="CC$mixed_float"/g_float  qualifiers.
**                  For Ultrix on VAX, we use D-Float; the above qualifiers must not be used on build
**                  For Ultrix on MIPS, we use the MIPS float (IEEE form)--no qualifiers necessary here.
**                  This module no longer uses the "BOMB."
**  ---------------------------------------------------------------------------
**  October 1991 - Converted for use with the SNMP Protocol Engine for the
**                 Common Agent.
**                 INCLUDED header file names were changed to avoid file names
**                 identical to MCC.  ("mcc_" --> "snmppe_").
**--
**/

typedef int BOOLEAN;

#ifndef vms         		/* NON VMS 	*/
#include "math.h"
#else                          /* VMS */
#include <math.h>
#endif                                 

#include "snmppe_descrip.h"
#include "snmppe_interface_def.h"
#include "snmppe_msg.h"

#include "asn_def.h"       /* common to all */
#include <string.h>

/* isolate class bits of DDIS/ASN.1 tag */
#define ILV_TAG_CLASS( tag ) ((tag) & MCC_K_ASN_CL_PRIV)

/* isolate form bit of DDIS/ASN.1 tag */
#define ILV_TAG_FORM( tag ) ((tag) & MCC_K_ASN_FORM_CONS)

/*
 * This byte array is used to reverse the bits in the ASN.1 BitString
 * encoding contents.  If this is used a lot, a VAX MOVTC 
 * (translate) instruction could be used to do it in microcode
 */
static unsigned char ReverseBits[256] = { 
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,

0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,

0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,

0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,

};


/*************************************************************************/
/* Declare functions called outside of this module.                      */
/*************************************************************************/

extern int ilv_decode_object_id();
extern int ilv_encode_object_id();
extern int ilv_insert_obj_id_length();


/*************************************************************************
**  support routines, these are private to this module
************************************************************************/

#if defined(sun) || defined(sparc)


/*************************************************************************
**  This routine is for support on SunOS on Sparc. Basically flipping   **
**  the bytes in an unsigned int. Used to solve the problem of big      **
**  verse little-endian.                                                **
*************************************************************************/

static unsigned int sun_x( a )

unsigned int a;

{

unsigned int b;
int i;
char *p_a;
char *p_b;

p_a = (char *)&a;
p_b = (char *)&b;

p_b++;
p_b++;
p_b++;

for (i = 0; i < 4; i++)
{
    memcpy(p_b, p_a, sizeof(char));
    p_a++;
    p_b--;
}

return(b);

}

#endif


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_put_len - encode length octets
**
**  FORMAL PARAMETERS:
**
**	 p_Buf - address of 1st length octet
**	 Length - length of ILV value contents 
**		    -1 for indefinite-length constructor
**		    -2 for definite-length constructor
**		    -3 for end of indefinite-length constructor
**		    -4 thru - 2**15 for filling in length of definite-length constructor
**
**  IMPLICIT OUTPUTS:
**
**	 buffer is modified
**
**  COMPLETION CODES:
**
**	 returns the address of the byte after the newly written length octets
**
**  ASSUMPTIONS:
**
**	maximum length of primitive or constructor is 2**15 - 3
**--
**/

unsigned char *ilv_put_len( p_Buf, Length )
unsigned char *p_Buf;
int Length;
{
    if (Length >= 0)	    
        {
        if (Length < 0x80) /* if short-form length */
        	*p_Buf++ = (unsigned char )Length;
        else
            { 
            if (Length < 0x100) /* if long form, 1-octet byte count */
                {
                *p_Buf++ = 0x81;    
            	*p_Buf++ = (unsigned char )Length;
                }
            else
                { 
                if (Length < 0x10000)      /* if long form 2-octet byte count */
                    {
                    *p_Buf++ = 0x82;
                    *p_Buf++ = (unsigned char )(Length >> 8);
            	    *p_Buf++ = (unsigned char )(Length & 0xFF);
                    }
                }
            }
        }
    else    /* Length is negative */
        {
        switch (Length)
            {
            case -1:        /* indefinite length */
                {
                *p_Buf++ = 0x80;
                break;
                }
            case -2:        /* if indefinite length constructor */
                {
                *p_Buf++ = 0x82;
                *p_Buf++ = (unsigned char)0;
                *p_Buf++ = (unsigned char)0;
                break;
                }
            case -3:        /* if end of indefinite-length constructor */
                {
                *p_Buf++ = 0;
                *p_Buf++ = 0;
                break;
                }
            default :       /* Length <= -4 -- if definite length constructor */
                {
                Length = -Length-4;
                *p_Buf++ = 0x82;
                *p_Buf++ = (unsigned char)(Length>>8);
                *p_Buf++ = (unsigned char)(Length & 0xFF);
                break;
                }
            };
        }
    return(p_Buf);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_begin_cons - start up new constructor within current constructor
**
**  FORMAL PARAMETERS:
**
**	 p_Ctx - context block for ILV value
**	 p_Buf - address of 1st length octet of new constructor
**	 indef - boolean, MCC_K_TRUE iff indefinite-length
**
**  IMPLICIT OUTPUTS:
**
**	 length octets are reserved for constructor
**	 new constructor length and address are pushed on constructor stack
**	 new constructor becomes current constructor
**
**  COMPLETION CODES:
**
**	 none
**
**--
**/

unsigned char *ilv_begin_cons( p_Ctx, p_Buf, indef )
struct ASNContext *p_Ctx;
unsigned char *p_Buf;
BOOLEAN indef;
{
    short TOS = p_Ctx->TOS;
    
    /* reserve length octets */

    if (indef)	/* if indefinite-length */
	    p_Buf = ilv_put_len( p_Buf, -1 ); /* indefinite-length constructor */
    else   /* definite length */
	    p_Buf = ilv_put_len( p_Buf, -2 ); /* definite-length constructor */

    /* add size of constructor tag and length to current cons's size*/

    if ( (TOS > 0) && (p_Ctx->ConsLen[TOS] > -1) )
	p_Ctx->ConsLen[TOS] += (p_Buf - p_Ctx->p_Pos);

    /* push new constructor on the stack */

    TOS++;
    if (TOS >= MCC_K_ASN_NESTCONS) return NULL;	/* Should never occur */

    p_Ctx->ConsLen[TOS] = ( indef ? -1 : 0 ) ; /* initially 0 bytes long */
    p_Ctx->ConsAddr[TOS] = p_Buf;    /* starting here */
    p_Ctx->TOS = TOS;
    return(p_Buf);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_end_cons - end current constructor and pop current constructor
**	    off top of constructor stack
**
**  FORMAL PARAMETERS:
**
**	 p_Ctx - context block for ILV value
**	 p_Buf - address of 1st length octet of new constructor
**
**  IMPLICIT OUTPUTS:
**
**	 if definite-length then length octets are filled in for current cons
**	 constructor length and address are popped off constructor stack
**
**  COMPLETION CODES:
**
**	 returns pointer to octet after end of constructor
**
**--
**/

unsigned char *ilv_end_cons( p_Ctx, p_Buf )
struct ASNContext *p_Ctx;
unsigned char *p_Buf;
{
    BOOLEAN indef;
    unsigned char *p_Len;
    int TOS = p_Ctx->TOS;
    unsigned short Len;

    indef = ( (p_Ctx->ConsLen[TOS] == -1) ? MCC_K_TRUE : MCC_K_FALSE );

    /* finish off encoding of current constructor */

    if (indef)	/* if indefinite-length cons */
	p_Buf = ilv_put_len( p_Buf, -3 );   /* output end of constructor */
    else /* definite-length constructor */
	{
	p_Len = p_Ctx->ConsAddr[TOS] - 3;   /* point to length octets*/
	Len = p_Buf - p_Ctx->ConsAddr[TOS] ;
	ilv_put_len( p_Len, -4 - (int)Len );	    /* fill in length octets */
	};

    /* pop the current constructor off of the stack and add its length to 
    ** parent constructor
    */

    TOS--;
    if (TOS > 0)    /* if there is a parent constructor */
	{
	int ParentLen = p_Ctx->ConsLen[TOS];

	if (ParentLen > -1) /* if definite-length constructor */
	    {
	    ParentLen += (p_Buf - p_Ctx->ConsAddr[TOS + 1]) ;
	    p_Ctx->ConsLen[TOS] = ParentLen;
	    };
	};
    p_Ctx->TOS = TOS;
    return(p_Buf);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_get_len - return length of value contents 
**
**  FORMAL PARAMETERS:
**
**	 p_Buf - address of 1st length octet
**
**  IMPLICIT OUTPUTS:
**
**	 none
**
**  COMPLETION CODES:
**
**	 returns the contents length in bytes, or -1 for 
**	    indefinite-length constructors
**--
**/
unsigned char *ilv_get_len( p_Buf, p_Len )
unsigned char *p_Buf;
int *p_Len;
{
unsigned char c;
unsigned char lenlen;	    /* length of length octets */

/* assume short form, since it nearly always is */

c = *p_Buf++;
lenlen = c & 0x7F;
*p_Len = (unsigned int )lenlen;

if (c & 0x80)	/* if long form */
    {
    switch (lenlen)
	{
	case 0:	    /* indefinite-length constructor */
	    *p_Len = -1;
	    break;
	case 1:
	    *p_Len = (unsigned int )*p_Buf;
	    break;
	case 2:
	    *p_Len = (unsigned int )((*p_Buf << 8) | *(p_Buf+1));
	    break;
	default:
	    return NULL;	/* Should never occur */
	};
    p_Buf += lenlen;   /* advance pointer past length octets */
    };
return(p_Buf);

}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_fnd_eoc - locate the end of an indefinite-length constructor
**
**  FORMAL PARAMETERS:
**
**	 p_Buf - address of indefinite-length constructor contents
**
**  IMPLICIT OUTPUTS:
**
**	 none
**
**  COMPLETION CODES:
**
**	 returns address of octet immediately after constructed value
**--
**/

unsigned char *ilv_fnd_eoc( p_Buf )
unsigned char *p_Buf;
{
    unsigned int Tag;
    unsigned char *ilv_skip_len_cont(), *ilv_get_tag();

    do 
	{
	p_Buf = ilv_get_tag( p_Buf, &Tag );
	p_Buf = ilv_skip_len_cont( p_Buf );
	}
	while ( Tag != MCC_K_ASN_DT_EOC );
    return(p_Buf);
}




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_skip_len_cont - skip ILV value length and contents
**
**  FORMAL PARAMETERS:
**
**	 p_Buf - address of ILV value length
**
**  IMPLICIT OUTPUTS:
**
**	 none
**
**  COMPLETION CODES:
**
**	 returns address of octet immediately after ILV value
**--
**/

unsigned char *ilv_skip_len_cont( p_Buf )
unsigned char *p_Buf;
    {
    int Len;

    p_Buf = ilv_get_len ( p_Buf, &Len );	/* get length */
    if (Len > -1) /* if not indefinite-length constructed encoding */
	p_Buf += Len;    /* skip contents */
    else /* indefinite-length constructed encoding */
	p_Buf = ilv_fnd_eoc ( p_Buf );
    return( p_Buf );
    }


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_tag_len - return length in bytes of ILV identifier value
**
**  FORMAL PARAMETERS:
**
**	 Tag - word identifier encoding
**
**  IMPLICIT OUTPUTS:
**
**	 none
**
**  COMPLETION CODES:
**
**	 returns the identifier encoding length in bytes 
**
**--
**/

unsigned int ilv_tag_len( Tag )
unsigned int Tag;
    {
    unsigned int len;

    len = 1;	/* most tags are 1 byte long */
    if ((Tag & 0x1F) > 0x1E)	/* if more than 1 byte long */
	    {
	    len = 2;
	    if (Tag & 0x8000 )    /* if more than 2 bytes long */
	        {
	        len = 3;
	        if (Tag & 0x800000)	/* if more than 3 bytes long */
		        {
		        len = 4;
		        if (Tag & 0x80000000) len = 0; /* Should never occur */
                };
		    };
	    };
    return(len);
    }



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_get_tag - return identifier octets in word
**
**  FORMAL PARAMETERS:
**
**	 p_Buf - address of 1st identifier octet
**	 p_Tag - address to return identifier in
**
**  IMPLICIT OUTPUTS:
**
**	 none
**
**  COMPLETION CODES:
**
**	 returns the address of the octet immediately after the identifier
**--
**/
unsigned char *ilv_get_tag( p_Buf, p_Tag )
unsigned char *p_Buf;
unsigned int *p_Tag;
    {
    static unsigned int tagmask[4] = { 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF };
    unsigned int tagval, *p_Word;
    unsigned int taglen;

#if defined(sun) || defined(sparc)
    unsigned int sun_tagval;
#endif

    p_Word = (unsigned int *) p_Buf;
    memcpy( &tagval, p_Word, sizeof(unsigned int) );  /* Aligned move */

#if defined(sun) || defined(sparc)
    sun_tagval = sun_x( tagval );
    taglen = ilv_tag_len(sun_tagval);
    tagval = sun_tagval & tagmask[ taglen - 1 ];/* isolate identifier octets */
#else
    taglen = ilv_tag_len(tagval);   /* length of identifier octets */
    tagval &= tagmask[ taglen - 1 ];/* isolate identifier octets */
#endif

    *p_Tag = tagval;
    p_Buf += taglen;
    return(p_Buf);
    }





unsigned char *ilv_skip_value( p_Buf )
unsigned char *p_Buf;
{
unsigned int Tag;
p_Buf = ilv_get_tag( p_Buf, &Tag );
p_Buf = ilv_skip_len_cont( p_Buf );
return(p_Buf);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_vldclass - validate identifier class value
**
**  FORMAL PARAMETERS:
**
**       Class 
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
**      MCC_K_TRUE - valid identifier class
**	MCC_K_FALSE - class not member of enumerated type
**
**--
**/
 ilv_vldclass( Class )
 unsigned int Class;
    {
    switch (Class) {
	case MCC_K_ASN_CL_UNIV:
	case MCC_K_ASN_CL_APPL:
	case MCC_K_ASN_CL_CONT:
	case MCC_K_ASN_CL_PRIV:
	    return(MCC_K_TRUE);
	};
    return(MCC_K_FALSE);
    }

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_vldidcode - validate identifier ID code value
**
**  FORMAL PARAMETERS:
**
**       IDCode
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
**      MCC_K_TRUE - valid ID code
**	MCC_K_FALSE - ID code > 2**28 - 1 or ID code < 0
**
**--
**/
 ilv_vldidcode( IDCode )
 unsigned int IDCode;
    {
    return ((IDCode < 0 || IDCode > 0x1FFFFF ) ? MCC_K_FALSE : MCC_K_TRUE);
    }


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       ilv_vldform - validate form value 
**
**  FORMAL PARAMETERS:
**
**       Form - must be Primitive or Constructed
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
**      MCC_K_TRUE - valid form
**	MCC_K_FALSE - not Prim or Cons
**
**--
**/
 ilv_vldform( Form )
 unsigned int Form;
    {
    return ((Form == MCC_K_ASN_FORM_PRIM || Form == MCC_K_ASN_FORM_CONS) ? MCC_K_TRUE : MCC_K_FALSE);
    }






/***********************************************************************
**  generic Ilv encoding routines for support of all ASN.1 encodings
************************************************************************/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       mcc_asn_put_init - initialize context block to begin building message
**
**  FORMAL PARAMETERS:
**
**	 p_Ctx	    - context block
**	 p_ValueDesc - ILV value buffer descriptor
**
**  IMPLICIT INPUTS:
**
**	 p_ValueDesc must contain the address and size of the ILV value buffer
**
**  IMPLICIT OUTPUTS:
**
**       context block is set up to construct an ILV message
**
**  COMPLETION CODES:
**
**	MCC_S_ILVBUFTOOBIG - buffer size argument exceeded maximum buffer size
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
 mcc_asn_put_init( p_Ctx, p_ValueDesc )
 struct ASNContext *p_Ctx;
 struct dsc_descriptor *p_ValueDesc;
    {
    unsigned int i;

    if (p_ValueDesc->dsc_w_length < MCC_K_ASN_SAFETY_MARGIN) 
	return(MCC_S_INSUF_BUF);
    if (p_ValueDesc->dsc_w_length > MCC_K_ASN_MAXSIZ) 
	return (MCC_S_ILVBUFTOOBIG);

    p_Ctx->BuildParse = 0;	/* building not parsing */
    p_Ctx->AtTag = MCC_K_TRUE;	/* this field not used for builds anyway */

    /* initialize constructor length stack */

    p_Ctx->TOS = 0;
    for (i=0; i<MCC_K_ASN_NESTCONS; i++) 
	{
	p_Ctx->ConsLen[i] = 0;
	p_Ctx->ConsAddr[i] = 0;
	};

    /* initialize buffer pointers */
    p_Ctx->ConsAddr[0] = p_Ctx->p_Pos = p_Ctx->p_Begin = 
	    (unsigned char *)p_ValueDesc->dsc_a_pointer;

    /* leave safety margin at end of buffer to avoid writing past edge */

    p_Ctx->BufSize = p_ValueDesc->dsc_w_length - (MCC_K_ASN_SAFETY_MARGIN-4);

    return(MCC_S_NORMAL);
    }



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mcc_asn_get_init - set up ILV value for parse
**
**  FORMAL PARAMETERS:
**
**	p_Ctx	    - ILV context block
**	p_ValueDesc - an ILV value descriptor
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	context block initialized
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**--
**/
mcc_asn_get_init( p_Ctx, p_ValueDesc )
struct ASNContext *p_Ctx;
struct dsc_descriptor *p_ValueDesc;
{
    unsigned int i;

    if (p_ValueDesc->dsc_w_length > MCC_K_ASN_MAXSIZ) return (MCC_S_ILVBUFTOOBIG);

    p_Ctx->BuildParse = 1;  /* parsing not building */
    p_Ctx->LastTag = 0xFFFFFFFF;  /* no tag seen so far */
    p_Ctx->AtTag = MCC_K_TRUE;	/* 1st octet of ILV value is tag */

    /* initialize constructor length stack */

    p_Ctx->TOS = 0;
    for (i=0; i<MCC_K_ASN_NESTCONS; i++) 
	{
	p_Ctx->ConsLen[i] = 0;
	p_Ctx->ConsAddr[i] = 0;
	};

    /* initialize buffer pointers */

    p_Ctx->ConsAddr[0] = p_Ctx->p_Pos = p_Ctx->p_Begin = 
    	(unsigned char *)p_ValueDesc->dsc_a_pointer;
    p_Ctx->BufSize = p_ValueDesc->dsc_w_length;

    return(MCC_S_NORMAL);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mcc_asn_fnd_tag
**
**  FORMAL PARAMETERS:
**
**	p_Ctx	    - context block
**	Tag	    - ILV identifier encoding
**	
**  IMPLICIT INPUTS:
**
**	context block must have been initialized
**
**  IMPLICIT OUTPUTS:
**
**	the cursor within the context block is positioned at the value
**	    preceded by the specified tag within the current constructor
**
**  COMPLETION CODES:
**
**	MCC_ILV_TNF	- tag not found in current constructor
**	MCC_ILV_DONE	- ILV value has already been parsed
**
**  SIDE EFFECTS:
**
**	if tag is not found then the context block is not altered
**
**--
**/

mcc_asn_fnd_tag( p_Ctx, Tag )
struct ASNContext *p_Ctx;
unsigned int Tag;
{

unsigned int TagVal;
unsigned char *p_Tag, *p_ConsLim;
int TOS;
int TagMatch;

/* ensure that you're not mixing find/get with put */
if (p_Ctx->BuildParse == 0) return(MCC_S_ILVBUILDORPARSE);

/* ensure that you are within a constructor */
if (p_Ctx->TOS <= 0) return(MCC_S_ILVNOTINCONS);

TOS = p_Ctx->TOS;   /* top-of-stack index for constructor stack */
p_Tag = p_Ctx->ConsAddr[TOS];	/* value cursor within current constructor */

/* note that if indefinite-length cons then p_ConsLim always < p_Tag */

p_ConsLim = p_Ctx->ConsAddr[TOS] + p_Ctx->ConsLen[TOS]; 

TagMatch = MCC_K_FALSE;
while	/* more tags in current constructor */
	(p_Tag < p_ConsLim) 

				    /* indefinite-length case */
    {
    p_Tag = ilv_get_tag( p_Tag, &TagVal );

    /* if this is it (doo wop) please let me know */
    if (!((TagVal ^ Tag) & ~MCC_K_ASN_FORM_CONS)) /* if all bits except form bit match */
	{
	TagMatch = MCC_K_TRUE;
	break;
	}
    else if (TagVal == MCC_K_ASN_DT_EOC) /* if end of indefinite-length cons seen*/
	break; /* exit loop on EOC */
    else    /* still haven't found what I'm looking for */
	p_Tag = ilv_skip_len_cont(p_Tag);
    };

if (Tag == MCC_K_ASN_DT_EOC)	/* if caller wants out of constructor */
    {
    p_Ctx->p_Pos = p_Tag;
    p_Ctx->AtTag = MCC_K_TRUE;
    p_Ctx->LastTag = 0;
    p_Ctx->ConsLen[TOS]=0;  /* zero out unused part of stack for easier debug*/
    p_Ctx->ConsAddr[TOS]=0;
    p_Ctx->TOS--;   /* pop the constructor stack */
    return( MCC_S_NORMAL );
    }
else if (TagMatch)	/* if we found the tag we searched for */
    {
    p_Ctx->p_Pos = p_Tag;
    p_Ctx->AtTag = MCC_K_FALSE;
    p_Ctx->LastTag = TagVal;
    return( MCC_S_NORMAL );
    };
    
return( MCC_S_ILVTNF );
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mcc_asn_get_tag - get next tag within current constructor
**
**  FORMAL PARAMETERS:
**
**	p_Ctx	    context block
**	p_Tag	    contains next ILV identifier upon return
**
**  IMPLICIT INPUTS:
**
**	context block must have been initialized
**
**  IMPLICIT OUTPUTS:
**
**	context block cursor now points to contents of next value in current
**	constructor
**
**  COMPLETION CODES:
**
**	MCC_ILV_EOC	- end of current constructor seen
**
**  SIDE EFFECTS:
**
**	whenever MCC_ILV_EOC completion code returned, constructor stack is popped
**
**--
**/

int mcc_asn_get_tag( p_Ctx, p_Tag )
struct ASNContext *p_Ctx;
unsigned int *p_Tag;
{
int TOS;
unsigned char *p_NxtTag, *p_ConsLim;

TOS = p_Ctx->TOS;   /* top-of-stack index for constructor stack */
p_NxtTag = p_Ctx->p_Pos;	/* value cursor within current constructor */

/* don't mix build calls with parse calls */
if (p_Ctx->BuildParse == 0) return(MCC_S_ILVBUILDORPARSE);

/* ensure that you haven't already parsed the entire ILV value */
if (TOS == 0)
    {
    if (p_Ctx->AtTag && p_NxtTag > p_Ctx->p_Begin) 
	return(MCC_S_ILVALREADYDONE);
    else /* at top-level encoding */
	{
	if (p_Ctx->AtTag )
	    p_ConsLim = ilv_skip_value( p_NxtTag );
	else
	    p_ConsLim = ilv_skip_len_cont( p_NxtTag );/* end of encoding */
	};
    }
else if (TOS < 0) 
  return MCC_S_ASNCORRUPT; /* should never happen */
else if (TOS > 0) /* if inside of some constructor */
  p_ConsLim = p_Ctx->ConsAddr[TOS] + p_Ctx->ConsLen[TOS]; 

/* don't get the tag until you're sure that it's there */

if (!p_Ctx->AtTag)  /* if positioned at length portion of ILV value */
    p_NxtTag = ilv_skip_len_cont( p_NxtTag );  /* skip preceding value */

/* must pop the constructor stack if end-of-constructor seen */

if (p_ConsLim == p_NxtTag) /* if we've reached the end of the encoding */
    {
    *p_Tag = MCC_K_ASN_DT_EOC;	    /* stop here */
     p_Ctx->ConsLen[TOS]=0;  /* zero out unused part of stack for easier debug*/
     p_Ctx->ConsAddr[TOS]=0;
     if (TOS > 0) 
        {
        TOS--;	/* pop the constructor stack */
        p_Ctx->TOS = TOS;
	    };
    p_Ctx->p_Pos = p_NxtTag;
    p_Ctx->AtTag = MCC_K_TRUE;
    return( MCC_S_ILVEOC );
    };

if (p_ConsLim < p_NxtTag) 
  return MCC_S_ASNCORRUPT; /* past cons end, should not happen */

/* now we are positioned at the next tag, so read it */

p_NxtTag = ilv_get_tag( p_NxtTag, p_Tag );

/* remember the state we're in within the context block */

p_Ctx->p_Pos = p_NxtTag;
p_Ctx->AtTag = MCC_K_FALSE;
p_Ctx->LastTag = *p_Tag;

return( MCC_S_NORMAL );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mcc_asn_get_ref - get next ILV value within current constructor
**
**  FORMAL PARAMETERS:
**
**	p_Ctx	    - context block
**	ExpectedType - optional expected type of value
**	p_Type	    - value data type
**	p_ValueBuf - address of native value returned from routine
**	ValueBufSize - size of native value buffer
**	p_ValueSize - address of size of actual value within buffer
**
**	note that for BITSTRING data types, the value/buffer size is in bits
**
**  IMPLICIT INPUTS:
**
**	context block must have been initialized for parse
**
**  IMPLICIT OUTPUTS:
**
**	value buffer receives decoded value
**
**  COMPLETION CODES:
**
**	TBS
**
**  SIDE EFFECTS:
**
**	if error is returned then the context block is not changed
**
**--
**/

mcc_asn_get_ref( 
    p_Ctx, ExpectedType, p_Type, p_ValueBuf, ValueBufSize, p_ValueSize)
struct ASNContext *p_Ctx;
unsigned char *p_ValueBuf;
int ExpectedType, *p_Type;
unsigned int ValueBufSize, *p_ValueSize;
{

#define ILV_TYPE_TAGGED 0x1000		/* signals that type cannot be determined from the encoding */

unsigned int OuterTag, Tag1;
int Type;
int OuterTagClass, OuterTagForm, Tag1Class, Tag1Form, OuterLen, Len1, status;
int TOS, Len, ValueCount;
BOOLEAN Tagged, Implicit, ImplicitPrim;
unsigned char *p_Buf, *p_Tag1, *p_Contents1, *p_LookAhead;
int ExpectedForm;
int mcc_get_real();

if (p_Ctx->BuildParse == 0) return(MCC_S_ILVBUILDORPARSE);

if (p_Ctx->AtTag == MCC_K_TRUE) return(MCC_S_ILVNOTATVALUE);

status = MCC_S_NORMAL;
p_Buf = p_Ctx->p_Pos;
TOS = p_Ctx->TOS;

/* ensure that you haven't already parsed the entire ILV value */
if (TOS == 0)	/* if not inside any constructor */
    {
    if (p_Ctx->p_Pos > p_Ctx->p_Begin && p_Ctx->AtTag)
	return(MCC_S_ILVALREADYDONE);
    }
else if (TOS < 0) 
  return MCC_S_ASNCORRUPT; /* should never happen */

/*  
** 	find out the expected form,
**	if constructed type is expected we can save lot of checking later
*/
if (ExpectedType >= 0)	/* if expecting Explicit*/
    	ExpectedForm = (int )ILV_TAG_FORM( ExpectedType );
else	ExpectedForm = (int )ILV_TAG_FORM( - ExpectedType );

OuterTag = p_Ctx->LastTag;
OuterTagClass = (int )ILV_TAG_CLASS( OuterTag );
OuterTagForm = (int )ILV_TAG_FORM( OuterTag );
p_Buf = ilv_get_len( p_Buf, &OuterLen ); /* get data value length */
Tagged = (OuterTagClass != MCC_K_ASN_CL_UNIV) ;
ImplicitPrim = Tagged && (OuterTagForm == MCC_K_ASN_FORM_PRIM);
ValueCount = 0;
p_Tag1 = p_Buf; /* save pointer to 1st inner tag */

/* we need to find out the data type of the data on hand */

if (!Tagged) 		/* This is a UNIVERSAL CLASS hence type = tag  */

	Type = OuterTag;

else    		/* class =  Context / Application / Private,  we need to look into it further */
	{
	if (ImplicitPrim)	/* if it is implicit encoding let us not waste any more energy */
	    /*
	    ** The caller had better specify what the data type is, since
	    ** the value encoding does not contain this information,
	    ** other than the fact that it is a primitive encoding.
	    */
	    Type = -ILV_TYPE_TAGGED;

	else	/* This is constructed and non Universal type. 
		** we may have a sequence/set on hand  OR
		** we may have an explicit encoding.
		** if the number of values within the construct is 0, 2 or more elements, clearly
		** it is sequence/set. 
		** if the number of values within the construct is 1
		** then it could be set/seq of one element  or an explicit encoding
		** we will have to rely on ExpectedType to resolve this ambiguity
		** let's go ....*/
	    {
	    if (OuterLen > 0 || OuterLen == -1)	/* if there is an inner tag */
		{
		p_Buf = ilv_get_tag( p_Buf, &Tag1 );   /* get 1st inner tag */
		p_LookAhead = p_Buf;	/* used for inner value count */
		p_Buf = ilv_get_len( p_Buf, &Len1 );   /* get its length */
		p_Contents1 = p_Buf;	/* used to get at inner value contents*/
		Tag1Form = (int )ILV_TAG_FORM( Tag1 );
		Tag1Class = (int )ILV_TAG_CLASS( Tag1 );
		if (Tag1 != MCC_K_ASN_DT_EOC)	/* if there is an inner value */
		    {
		    ValueCount=1;					/* at least one inner value in outer cons */
		    p_LookAhead = ilv_skip_len_cont( p_LookAhead ); 	/* find the end of the 1st inner value */
		    /* determine whether Tag1 is last tag in this cons */
		    if (OuterLen == -1) /* if indefinite-length cons */
			{
			unsigned int NextTag;
			p_LookAhead = ilv_get_tag( p_LookAhead, &NextTag );

			/*  if you don't see EOC after Tag1 within constructor OuterTag,
			** then it HAS to be an IMPLICIT SEQUENCE/SET	*/

			if (NextTag != MCC_K_ASN_DT_EOC) 		/* if not end of cons */
			    ValueCount = 2;				/* more than 1 inner value in cons */
			}

		    else 		/* definite-length, see if p_LookAhead is at end of cons*/
			if ( OuterLen - (int )(p_LookAhead - p_Tag1) > 0 ) 	/* not end of cons */
			    ValueCount = 2;					/* more than 1 inner value in cons */
										/* else ValueCount remains at 1 */
		    };
		};

	    /* 
	    ** classify the constructor based upon what you found inside it
	    ** if there were multiple encodings inside it, then it has to be SET/SEQUENCE
	    ** if it's empty, it has to be a SET/SEQUENCE as well
	    ** but if it's only got 1 value in it, the encoding is ambiguous
	    */
	    if ( (ValueCount == 0 ) || (ValueCount == 2) )
		Type = -MCC_K_ASN_DT_SEQUENCE;
	    else 	/* ValueCount == 1, only one value in outer cons */
		    /*
		    ** This value may be:
		    **    -- a tagged value
		    **	  -- IMPLICIT SEQUENCE/SET containing 1 value
		    ** we assume the former here, but the caller is expected to
		    ** override this guess by providing the ExpectedType arg.
		    ** NOTE: This ambiguity can be eliminated by avoiding use
		    ** of IMPLICIT SEQUENCE/SETs where they are not expected.
		    **
		    ** if Tag1 doesn't specify a UNIV type then 
		    ** we return the type TAGGED, since this encoding 
		    ** may represent a CHOICE or SELECTONE encoding.
		    ** The caller then can use the tag we gave him to decide
		    ** what this is, or can then call mcc_asn_get() again to get 
		    ** the remainder.	    */
		    {
			if (Tag1Class != MCC_K_ASN_CL_UNIV)    
			     Type = -ILV_TYPE_TAGGED ;
			else 			/* We are letting the expected type to decided the data type */
			     if ( ExpectedForm == MCC_K_ASN_FORM_CONS) 
					Type = -MCC_K_ASN_DT_SEQUENCE; 
			     else       Type = Tag1;	/* this must be explicit encoding */
		    };  
	    };   
	};


/*
** now that we have analyzed the type, let's see if a syntax violation
** has occurred.
*/

if ( ExpectedType != 0 )	/* if caller specified an expected data type */
    {
    if (Type != -ILV_TYPE_TAGGED)	/* if type found */
    	{
	    if (ExpectedType != Type)    /* if this conflicts with type we saw*/
	        {
	        if ( /* if not really a conflict*/
		        ((ExpectedType == -MCC_K_ASN_DT_SET) || (ExpectedType == -MCC_K_ASN_DT_SEQUENCE))
		        && (Type == -MCC_K_ASN_DT_SEQUENCE) )   
		        Type = ExpectedType;	/* let caller choose between SEQUENCE and SET */
	        else
		        if ( Type == MCC_K_ASN_DT_NULL )
		            status = MCC_S_ILVIMPLDEFAULT; /* default value */

/* rethink the above status. OTHERS ARE USING ASN1 outside mcc */

		        else status = MCC_S_ILVSYNTAXERROR;	/* tap caller on the shoulder and say... */
	        };
	    }
    else    /* type not found, do what caller tells you to do */
    	{   
	    /* make sure caller was expecting IMPLICIT typing */
	    if (ExpectedType < 0)	/* if expecting IMPLICIT */
	        {
	        Type = ExpectedType;
	        /* make sure form bits of ExpectedType and encoding agree */
	        if (ImplicitPrim && 
		        ((ExpectedType == -MCC_K_ASN_DT_SEQUENCE) ||
		         (ExpectedType == -MCC_K_ASN_DT_SET)))
	      	    status = MCC_S_ILVSYNTAXERROR;
	        }
    	else
	        status = MCC_S_ILVIMPLICIT; /* tell the caller about this */
	    };
    }
else	/* caller didn't supply data type of value */
    {
    if (Type == -ILV_TYPE_TAGGED)   /* if no type found */
	status = MCC_S_ILVNOTYPEFOUND;  /* maybe the user can determine its type*/
    else  /* type was found */
	{
	status = MCC_S_ILVMISTYPED; /* return type to caller, do not proceed */
	if (p_Type) *p_Type = Type;
	};
    };

/*
**  before making any changes to the parse state in the context block, make
**  sure that you know what you're doing.  If not, just return to the caller
**  with an error status and let the caller try to recover if desired
*/

if ( ( status != MCC_S_NORMAL ) && ( status != MCC_S_ILVIMPLDEFAULT ) ) 
    return(status);

/* 
** at this point we know as much about the ILV value's data type as 
** we ever will know.  If the value is a primitive value (tagged or untagged)
** it is returned by this call.  If the value is a constructed value
** (tagged or untagged) then we push the constructor data onto the 
** constructor stack.
*/

if (p_Type) *p_Type = Type; /* return the data type if the caller wants it */
if (Type < 0) 
    {
    Implicit = MCC_K_TRUE;
    Type = -Type; /* get rid of IMPLICIT info */
    };

if (ILV_TAG_FORM(Type) == MCC_K_ASN_FORM_PRIM) /* if this is primitive type */
    {
    if (OuterTagForm == MCC_K_ASN_FORM_PRIM) /* if outer tag is primitive*/
    	{
	    Len = OuterLen;	/* get contents length */
	    /* p_Buf already set correctly */
	    }
    else if (ValueCount > 0) /* inner tag contains the primitive value */
	        {
	        Len = Len1;	/* get value length */
	        p_Buf = p_Contents1;	/* addr of 1st prim contents octet */
	        }
	     else return MCC_S_ASNCORRUPT;	/* ValueCount == 0 should never happen, just checking...*/

    /* first make sure that the value fits in the caller-supplied 
    ** value buffer.  If it doesn't, let caller know how big the actual
    ** value was.
    */

    if (Type < 0) Type = -Type;	/* ignore sign bit, just use type code */

    switch (Type)
      {
      case MCC_K_ASN_DT_BITSTRING:
	if (Len > ( (ValueBufSize+7)/8 + 1) )	/* integral num of octets for data */
                                            /* plus one octet for num of unused bits ct */
                                            /* if not enough BYTES within buffer */
	    {
	    if (p_ValueSize) 
		/* return #BITS needed in buffer, rounded up to nearest byte boundary */
		*p_ValueSize = (Len - 1)*8; 
	    return MCC_S_ILVTOOBIG;
	    };
	break;

      case MCC_K_ASN_DT_NULL:
	break;
      case MCC_K_ASN_DT_REAL:
    	if (ValueBufSize < sizeof(double) ) 
	        {
	        if (p_ValueSize) *p_ValueSize = sizeof(double);
	        return(MCC_S_ILVTOOBIG);
	        };
        break;
      default:
	if (Len > ValueBufSize ) 
	    {
	    if (p_ValueSize) *p_ValueSize = Len;
	    return(MCC_S_ILVTOOBIG);
	    };
      };

    /* now actually return the value to the caller */

    switch (Type)
     {

     case MCC_K_ASN_DT_OCTETSTRING:
	 /* copy to ...                    from ... len ... */
         memcpy( p_ValueBuf, p_Buf, (size_t) Len ); 
         p_Buf += Len;    /* advance buffer pointer */
	 break;

     case MCC_K_ASN_DT_INTEGER:
	{
	unsigned char *p_MostSigByte;
	unsigned char *p_Int;
	unsigned char sign;

#if defined(sun) || defined(sparc)

	/* sign bit is 1st bit of integer encoding */

	p_MostSigByte = p_ValueBuf + ValueBufSize - Len;
	sign = *p_Buf & 0x80; 

	/* pad most significant unused bits of native value buffer with
	** 0's if positive, 1's if negative 
	*/

	for (p_Int = p_ValueBuf; 
	     p_Int < p_MostSigByte; 
	     p_Int++)
		*p_Int = ( sign ? 0xFF : 0 );

	/* now copy integer into buffer byte by byte */

	while ( p_Int < p_ValueBuf + ValueBufSize )
	    *p_Int++ = *p_Buf++; /* write integer octets */

#else

	/* sign bit is 1st bit of integer encoding */

	p_MostSigByte = p_ValueBuf + Len - 1;
	sign = *p_Buf & 0x80; 

	/* pad most significant unused bits of native value buffer with
	** 0's if positive, 1's if negative 
	*/

	for (p_Int = p_ValueBuf + ValueBufSize - 1; 
	     p_Int > p_MostSigByte; 
	     p_Int--)
		*p_Int = ( sign ? 0xFF : 0 );

	/* now copy integer into buffer byte by byte */

	while ( p_Int >= p_ValueBuf )
	    *p_Int-- = *p_Buf++; /* write integer octets */

#endif

	};
	break;

     case MCC_K_ASN_DT_NULL:
	 break;

     case MCC_K_ASN_DT_BOOLEAN:

/*   ********************************************
 *    allow any non-zero value to result in "mcc_k_true" value
 *   **********************************************
 */
    	/* error checking for length of 1 octet */
    	if  (Len != 1)   return MCC_S_ILVSYNTAXERROR;

    	if  ((unsigned char)*p_Buf == 0) 	
		*p_ValueBuf = 0;
    	else    *p_ValueBuf = 1;
    	p_Buf += Len;    /* advance buffer pointer */
	break;

     case MCC_K_ASN_DT_OBJECTID:
	{
	 int LengthWritten = 0;
	/* Only accept components which take up 4 bytes (28bits of value)	*/
	/* Because the first two are a special case, max is 2**28-1-80) for	*/
	/* the second */
	/* Hence we can place result in an array of longwords in the buff	*/


	 status = ilv_decode_object_id(p_Buf,Len,p_ValueBuf,ValueBufSize,&LengthWritten);
	 if (status != MCC_S_NORMAL)
	     return status;
         p_Buf += Len;    /* Advance buffer pointer */
	 Len = LengthWritten; /* Now reinterpret Len for its use below */
	 break;
	};

     case MCC_K_ASN_DT_BITSTRING:
	{
	unsigned char LeftOver; /* holds number of unused bits in last byte of string */
	unsigned int ByteCount, Count;

	/* input NativeValueSize is in bits, not bytes */

	if (Len == 0) /* zero length bitstring is valid */
        {
        ByteCount = 0;
        LeftOver  = 0;
        }
    else
        {
    	ByteCount = Len - 1;  /* don't count the byte used for the unused bits count  */
    	LeftOver = *p_Buf++;
	    };
    /* A one byte Bitstring contains no data and so LeftOver must be zero */
    if (ByteCount == 0 && LeftOver != 0)
        return MCC_S_ILVSYNTAXERROR;

    /* NOTE that PORT_CMIP does not need to reverse the bit order.  Depending on  */
    /* the users of this routine, MCC may not need to reverse bits either.       */
    /* The answer to the question requires further research                      */


#ifdef PORT_CMIP

    /* do a straight copy of the data - no need to reverse bits.  */

    memcpy( p_ValueBuf, p_Buf, (size_t) ByteCount);
    p_Buf += ByteCount; /* advance buffer pointer */

#else

	/* the following loop could be replaced by a VAX MOVTC instruction */
	for (Count = ByteCount; Count>0; Count--)   /* most optimal FOR loop counts down */
	    {
	    *p_ValueBuf = ReverseBits[*p_Buf++];
	    p_ValueBuf++;
	    };
#endif

	if (p_ValueSize) 
	    {
	    *p_ValueSize = /* return size in BITS */
		(ByteCount > 0) 
		    ? (ByteCount*8 - LeftOver)
		    : 0 ;   /* special case for 0-bit bitstring */
	    /*
	     * HACK ALERT: the following statement disables the assignment of 
	     * a byte count to the user-supplied size buffer below.
	     * It assumes that the following statement affects a temporary
	     * copy of the pointer on the stack frame of this call.
	     * Will this be a problem outside the VAX domain??
	     * only your software engineer knows for sure...
	     */
	    p_ValueSize = 0;
	    };
	break;
	};


    case MCC_K_ASN_DT_REAL:				/* added by jesuraj April 18, 90*/
        {
        double number;
        char buff[REAL_BUFF_LEN];		/* vax-vms specific */

#define PLUS_INFINITY 0x40
#define MINUS_INFINITY 0x41

        if (ValueBufSize < sizeof (number))  /* ensure buffer has room for 8-octet real number*/
            {
            if (p_ValueSize != MCC_K_NULL_PTR) *p_ValueSize = sizeof(number);
                return(MCC_S_ILVTOOBIG);
            }             
    
        switch (Len)
            {
            case 0 : 
                {  /* ASN.1 BER requires real number 0 encoded as 0 length, no contents octets */
                number = 0.0;         /* return double containing 0.0 */
                status = MCC_S_NORMAL;
                break;
                }
            case 1 :    /* this is the length for a special real encoding */
                {
                if (*p_Buf == PLUS_INFINITY)
                    number = HUGE_VAL;
                if (*p_Buf == MINUS_INFINITY)
                    number = 0.0;
                status = MCC_S_ASNEXPOUTOFRANGE;
                break;                
                }
            case 9 :  /* a regular real encoding will be 9 or 10 octets*/
            case 10 :
                {
                memcpy(&buff[0], p_Buf, (size_t) Len);  /* copy the encoded exponent and fraction to a buffer */
                p_Buf += Len;                     /* increment buffer pointer past them */
                status = mcc_get_real(&buff[0], &number);   /* make the real number */
                break;
                }
            default: /* any other length we don't deal with now */
                status = MCC_S_ILVNOTIMPLYET;  
                break;
            }  /* end switch on length of encoded datum */
        if ( (status == MCC_S_NORMAL) || (status == MCC_S_ASNEXPOUTOFRANGE) )
            {
            Len = sizeof(number);                  
            memcpy(p_ValueBuf, (char *)(&number), (size_t) Len);	/* copy the result */
            }  
        else
            return (status);
        break;   
    } /* end ASN floating point */


     case MCC_K_ASN_DT_OBJECTDESC:     /* not to be done for A */
     case MCC_K_ASN_DT_EXTERNAL:
     default:
	return(MCC_S_NOT_IMPLEMENTED);
     };	/* end of case statement */

    }

/* 
 * above this point, the caller's context block has not been
 * modified and there are no side-effects of a return from
 * the routine.  From here on in we actually modify the 
 * context block, so the following actions must be atomic.
 */

else /* constructed value, push the relevant stuff on the stack */
    {
    TOS++;
    if (Tagged && !Implicit) /* if tagged, non-IMPLICIT SEQUENCE */
	{
	p_Buf = p_Ctx->ConsAddr[TOS] = p_Contents1; /* skip UNIVERSAL tag */
	p_Ctx->ConsLen[TOS] = Len = Len1;
	}
    else /* IMPLICIT SEQUENCE, only one identifier */
	{
        p_Buf = p_Ctx->ConsAddr[TOS] = p_Tag1;
        p_Ctx->ConsLen[TOS] = Len = OuterLen;
	};
    p_Ctx->TOS = TOS;
    };

if (p_ValueSize) *p_ValueSize = Len;
p_Ctx->p_Pos = p_Buf;
p_Ctx->AtTag = MCC_K_TRUE;
return(status);

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mcc_asn_put_ref - encodes this native value as an ILV value at the end
**	of the current constructor.  The native value is passed by reference.
**
**  FORMAL PARAMETERS:
**
**	p_Ctx	    		- context block
**	Tag	    		- identifier encoding
**	Type	    		- data type
**	p_NativeValue 		- address of native value
**	NativeValueSize   	- size of native value in bytes
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
**	MCC_S_ILVALREADYDONE    - ILV value has already been completed
**	MCC_S_ILVTAG	    	- the supplied tag is not valid
**      MCC_S_ILVSYNTAXERROR    - invalid data for object_id 
**	<others TBS>
**
**  SIDE EFFECTS:
**
**	none
**--
**/

mcc_asn_put_ref( p_Ctx, Tag, Type, p_NativeValue, NativeValueSize )
struct ASNContext *p_Ctx;
unsigned int Tag;
int Type;
unsigned char *p_NativeValue;
unsigned int NativeValueSize;
{
unsigned char *p_Buf, *p_BufLim;
BOOLEAN Implicit;
int TypeForm;
BOOLEAN NoTag;
unsigned int *p_Tag;
int mcc_put_real();

if (p_Ctx->BuildParse == 1) return(MCC_S_ILVBUILDORPARSE);

/* ensure that you haven't already built the entire ILV value */
if (p_Ctx->TOS == 0 && p_Ctx->p_Pos > p_Ctx->p_Begin ) 
	return(MCC_S_ILVALREADYDONE);

/* unpack arguments */

Implicit = (Type <= 0);	/* extract sign bit */
/* !the previous statement does not support non-IMPLICIT SEQUENCES! */

if (Implicit) Type = -Type; /* zero sign bit */
TypeForm = ILV_TAG_FORM(Type);
NoTag = (Tag == Type);

/* processing of EXTERNAL types differs from processing of other types */

if (Type == MCC_K_ASN_DT_EXTERNAL) /* if we're inserting a value */
    {
    /*
     * there is no universal tag for EXTERNAL!
     * so it must always be IMPLICIT.  Furthermore it must
     * always be a constructed encoding, since by definition it
     * contains an ASN.1 value.
     */
    Implicit = MCC_K_TRUE;
    TypeForm = MCC_K_TRUE;
    NoTag = MCC_K_FALSE;
    };

/* pick up where we left off */

p_Buf = p_Ctx->p_Pos;
p_BufLim = p_Ctx->p_Begin + p_Ctx->BufSize;

/* if sealing up the current constructor */

if (Type == MCC_K_ASN_DT_EOC)
    {
    if (!Implicit) /* if not an IMPLICIT SEQUENCE/SET */
	p_Buf = ilv_end_cons( p_Ctx, p_Buf );
    p_Buf = ilv_end_cons( p_Ctx, p_Buf );
    return(MCC_S_NORMAL);
    };

/*
** there are three cases to deal with for any ILV value
**
**	two identifiers ( APPLICATION/CONTEXT-SPECIFIC and UNIVERSAL/PRIVATE )
**	Tag identifier only ( IMPLICIT )
**	Type identifier only ( UNIVERSAL/PRIVATE untagged )
**
*/

if (!NoTag) /* if tagged value */
    {
#if defined(sun) || defined(sparc)
    unsigned int sun_Tag;
#endif

    /* make sure that Tag's form bit is correct */

    if ( (TypeForm == MCC_K_ASN_FORM_PRIM) && Implicit ) 
	Tag &= ~MCC_K_ASN_FORM_CONS; /* ensure that form bit is clear */
    else
	Tag |= MCC_K_ASN_FORM_CONS;	/* ensure that form bit is set */

    /* a tag identifier is provided, then slap it on first */
    /* append tag to ILV value and advance pointer */

    p_Tag = (unsigned int *)p_Buf;

#if defined(sun) || defined(sparc)
    sun_Tag = sun_x( Tag );
    memcpy( p_Tag, &sun_Tag, (size_t) sizeof(unsigned int) ); /* Write out tag */
#else
    memcpy( p_Tag, &Tag, (size_t) sizeof(unsigned int) ); /* Write out tag */
#endif

    p_Buf += ilv_tag_len( Tag );

    /* append length */

    if (Tag & MCC_K_ASN_FORM_CONS)	    /* if constructed form */
	{
	p_Buf = ilv_begin_cons( p_Ctx, p_Buf, 0 );
	};
    };

/* if this isn't an IMPLICIT type, then slap on the type identifier/len too */

if (!Implicit)
    {
#if defined(sun) || defined(sparc)
    unsigned int sun_Type;
#endif

    /* append tag to ILV value and advance pointer */

    p_Tag = (unsigned int *)p_Buf;

    /* need a memcpy here because the word ptr 'p_Tag' might be unaligned */
#if defined(sun) || defined(sparc)
    sun_Type = sun_x( Type );
    memcpy( p_Tag, &sun_Type, (size_t) sizeof(unsigned int ));
#else
    memcpy( p_Tag, &Type, (size_t) sizeof(unsigned int ));
#endif
    p_Buf += ilv_tag_len( Type );

    /* append length */

    if (Type & MCC_K_ASN_FORM_CONS)	    /* if constructed form */
	{
	p_Buf = ilv_begin_cons( p_Ctx, p_Buf, 0 );
	};
    };

/* if this is not a constructed type, then output value */

if (!( Type & MCC_K_ASN_FORM_CONS ))	/* if primitive */
    {

    switch (Type)
    {
        case MCC_K_ASN_DT_SMI_IPADDR:
            if (NativeValueSize > sizeof(unsigned int))
                return( MCC_S_ILVSYNTAXERROR );
	    if (p_Buf + NativeValueSize > p_BufLim) /* if value don't fit*/
		return( MCC_S_ILVTOOBIG );
	    break;

	case MCC_K_ASN_DT_BITSTRING:
	    if ((p_Buf + ((NativeValueSize+7)/8) + 1) > p_BufLim )
		return( MCC_S_ILVTOOBIG );
	    break;

	case MCC_K_ASN_DT_NULL:
	    break;

	default:
	    if (p_Buf + NativeValueSize > p_BufLim) /* if value don't fit*/
		return( MCC_S_ILVTOOBIG );
	};

    /* copy primitive value into ILV value buffer */

    switch (Type)
     {

     /* this case comes first for optimal performance */
     case MCC_K_ASN_DT_INTEGER:
	{
	unsigned char *p_MostSigByte, *p_IntLen;
	char *p_Int;
	unsigned short NineBits;
	char NonNegative;

#if defined(sun) || defined(sparc)

	p_IntLen = p_Buf;   /* remember where to put length byte */
	p_Buf++;    /* advance buffer pointer past where length will go */

	p_Int = (char *)p_NativeValue;
	    /* locate at most significant byte */
	NonNegative = (*p_Int >= 0);

	do  /* ensure that 1st 9 bits are not all 0s or 1s */
	    {
            memcpy( &NineBits, p_Int, (size_t) sizeof(unsigned short) );  /* Unaligned Move */
	    NineBits &= 0xFF80;
	    if (NonNegative)
		{
		if (NineBits != 0) break;
		}
	    else 
		if (NineBits != 0xFF80) break;

            p_Int++;
	    }
	  while (p_Int < ((char *)p_NativeValue + NativeValueSize - 1));

	p_MostSigByte = (unsigned char *)p_Int; /* remember how long it actually was */

	while ( p_Int <= ((char *)p_NativeValue + NativeValueSize - 1) )
	    *p_Buf++ = *p_Int++; /* write integer octets */

	/* p_Int NOW POINTS TO BYTE after p_NativeValue */

	ilv_put_len( p_IntLen, (int) ((unsigned char *)p_Int - p_MostSigByte) );

#else

	p_IntLen = p_Buf;   /* remember where to put length byte */
	p_Buf++;    /* advance buffer pointer past where length will go */

	p_Int = (char *)p_NativeValue + NativeValueSize - 1; 
	    /* locate at most significant byte */
	NonNegative = (*p_Int >= 0);

	do  /* ensure that 1st 9 bits are not all 0s or 1s */
	    {
	    p_Int--;	/* start out with most significant 9 bits */
            memcpy( &NineBits, p_Int, (size_t) sizeof(unsigned short) );  /* Unaligned Move */
	    NineBits &= 0xFF80;
	    if (NonNegative)
		{
		if (NineBits != 0) break;
		}
	    else 
		if (NineBits != 0xFF80) break;
	    }
	  while (p_Int >= (char *)p_NativeValue);

	p_Int++;
	p_MostSigByte = (unsigned char *)p_Int; /* remember how long it actually was */

	while ( p_Int >= (char *)p_NativeValue )
	    *p_Buf++ = *p_Int--; /* write integer octets */

	/* p_Int NOW POINTS TO BYTE before p_NativeValue */

	ilv_put_len( p_IntLen, (int) (p_MostSigByte - (unsigned char *)p_Int) );

#endif

	};
	break;

     case MCC_K_ASN_DT_SMI_IPADDR:
	 p_Buf = ilv_put_len( p_Buf, (int) NativeValueSize );
	 /* copy to ... from .... length .... */
         memcpy( p_Buf, p_NativeValue, (size_t) NativeValueSize ); 
         p_Buf += NativeValueSize;    /* advance buffer pointer */
	 break;

     case MCC_K_ASN_DT_OCTETSTRING:
	 p_Buf = ilv_put_len( p_Buf, (int) NativeValueSize );
	 /* copy to ... from .... length .... */
         memcpy( p_Buf, p_NativeValue, (size_t) NativeValueSize ); 
         p_Buf += NativeValueSize;    /* advance buffer pointer */
	 break;

     case MCC_K_ASN_DT_EXTERNAL:
	 /* copy to ... from .... length .... */
         memcpy( p_Buf, p_NativeValue, (size_t) NativeValueSize ); 
         p_Buf += NativeValueSize;    /* advance buffer pointer */
	 break;

     case MCC_K_ASN_DT_BOOLEAN:
	/* write value 1 if non-zero, 0 otherwise, length is 1 octet */
	p_Buf = ilv_put_len( p_Buf, 1 );    /* always 1 octet long */
	*p_Buf++ = ( (char ) *p_NativeValue ? 1 : 0 );
	break;

     case MCC_K_ASN_DT_NULL:
	 p_Buf = ilv_put_len( p_Buf, 0 );
	 break;

     case MCC_K_ASN_DT_OBJECTID:
 	{
	    int LengthEncoded=0;
	    int status;

	    /* We already know this will fit in the message buffer */
            /* Object_id's must be at least 8 bytes */
            if ((NativeValueSize < 8) || 
                (p_NativeValue == (unsigned char *) NULL))
            {
                status = MCC_S_ILVSYNTAXERROR;
		return status;
            }

	    status = ilv_encode_object_id(&LengthEncoded,p_Buf,NativeValueSize,p_NativeValue);
	    if (status != MCC_S_NORMAL)
		return status;
            ilv_insert_obj_id_length(p_Buf, &LengthEncoded);		
	    p_Buf += LengthEncoded;
	    break;
	 };

     case MCC_K_ASN_DT_BITSTRING:
	{
	unsigned char LeftOver;
	unsigned int ByteCount;
	register int Count;

	/* input NativeValueSize is in bits, not bytes */

	/* 0 bits fits in 0 bytes, 1 bit fits in 1 byte,...,9 bits in 2 bytes.. */
	ByteCount = (NativeValueSize+7) >> 3 ;  /* divide by 8 */
	LeftOver = ( ByteCount << 3 ) - NativeValueSize; /* number of unused bits in last byte */
	p_Buf = ilv_put_len( p_Buf, (int) (ByteCount+1) );
	*p_Buf = LeftOver;  /* first contents octet is leftover bit count */
	p_Buf++;	    /* skip to actual beginning of bitstring */
	
#ifdef PORT_CMIP

    /* do a straight copy of the data */
    memcpy(p_Buf, p_NativeValue, (size_t) ByteCount);
    p_Buf += ByteCount;     /* advance buffer pointer */

#else

	/* OPTIMIZATION NOTE: this loop could be replaced with VAX MOVTC */
	for (Count=ByteCount; Count>0; Count--) 
	    *p_Buf++ = ReverseBits[*p_NativeValue++];

#endif

	break;
	};
    case MCC_K_ASN_DT_REAL:				/* added by jesuraj April 18, 90*/
        {
        char        buff[REAL_BUFF_LEN];
        int         num_bytes = 0;
        double      test;

        test = *( (double *)p_NativeValue);  /* look at the whole real number */
        if (test  != 0.0)    /* 0.0 is a special encoding, 0 length, no contents octets */
            {
            num_bytes = mcc_put_real(&buff[0], p_NativeValue);
            }
        p_Buf = ilv_put_len(p_Buf, num_bytes);
        memcpy(p_Buf, &buff[0], (size_t) num_bytes);  
        p_Buf +=num_bytes;    /* advance buffer pointer */
        break;   
        }     
    default: /* not yet done */
	    return(MCC_S_NOT_IMPLEMENTED);
    }

    /* if tagged non-IMPLICIT primitive value written, then close constructor*/

    if ( !Implicit && !NoTag )	/* if two identifiers written */
        p_Buf = ilv_end_cons( p_Ctx, p_Buf );	/* terminate tagged value*/
    else if (Type == MCC_K_ASN_DT_EXTERNAL) /* if ASN.1 encoding inserted */
	p_Buf = ilv_end_cons( p_Ctx, p_Buf );	/* terminate EXTERNAL value */
	else
	    {
	    /* if not indef-length cons */
	    if (p_Ctx->TOS > 0 && p_Ctx->ConsLen[p_Ctx->TOS] > -1)
		/* add in size of this encoding to constructor length */
		p_Ctx->ConsLen[p_Ctx->TOS] += (p_Buf - p_Ctx->p_Pos);
	    }
    };

p_Ctx->p_Pos = p_Buf;
return(MCC_S_NORMAL);

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       mcc_asn_tag - encode an ILV data value identifier, or tag
**
**  FORMAL PARAMETERS:
**
**	 IDCode - ID code
**	 OPTIONAL Class - identifier class
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	returns the encoded tag as a word value
**
**--
**/
 unsigned int mcc_asn_tag( IDCode, Class )
 unsigned int Class;
 unsigned int IDCode;
    {
    unsigned int IDC, Tag;


/************************************************************************/
/****              WARNING					    *****/
/* 	we use only low order 21 bits.  Even if ID code > 21 bits.  	*/
/*  	NO ERROR CHECKING IS DONE       			       	*/
/************************************************************************/

    Tag = 0;
    IDCode = IDCode & 0x1FFFFF;		/* mask off low-order 21 bits  ignore the high order bits */
    IDC = IDCode ;			/* remember what ID code we are dealing with */

    /* encode identifier to see how long it is */

    if (IDC < 0x1F)	/* if short form identifier */
      Tag = Class | IDC;   /* do short form */
    else /* do long form */
      {
      /* this loop will always terminate because the arithmetic shift will
      ** force IDC to zero
      */
      while (IDC)   /* while more bits to encode */
	{
	Tag |= (IDC & 0x7F);	/* take least significant 7 bits */
	if (IDC != IDCode) 	/* if not first time through loop */
	    Tag |= 0x80;	    /* set continuation bit */
	IDC >>= 7;		/* shift out least significant 7 bits */
	Tag <<= 8;		/* make room for next 8 bits */
	};
      Tag |= ( Class | 0x1F);   /* 1st octet */
      };

    return (Tag);	/* return encoded tag */
    }



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       mcc_asn_tag_class - extract identifier class from ILV tag
**
**  FORMAL PARAMETERS:
**
**	 Tag - word identifier encoding
**
**  IMPLICIT OUTPUTS:
**
**	 none
**
**  COMPLETION CODES:
**
**	 returns the class code
**
**--
**/

 unsigned int mcc_asn_tag_class( Tag )
 unsigned int Tag;
    {
    return( (int) ILV_TAG_CLASS(Tag) );   /* use the macro */
    }


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**       mcc_asn_tag_form - extract identifier form from ILV tag
**			(primitive or constructed)
**
**  FORMAL PARAMETERS:
**
**	 Tag - word identifier encoding
**
**  IMPLICIT OUTPUTS:
**
**	 none
**
**  COMPLETION CODES:
**
**	 returns the form bit (0 or 1)
**
**--
**/

 unsigned int mcc_asn_tag_form( Tag )
   unsigned int Tag;
     {
     return( (int )ILV_TAG_FORM(Tag) );    
     }


/**++
**  FUNCTIONAL DESCRIPTION:
**
**       mcc_asn_tag_id - extract the ID code from an ILV identifier
**
**  FORMAL PARAMETERS:
**
**	 Tag - ILV identifier
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	returns the IDCode as a word value
**
**--
**/
 unsigned int mcc_asn_tag_id( Tag )
 unsigned int Tag;
    {

    unsigned int IDC;

    /* decide whether short form or long form - almost always short */

    IDC = Tag & 0x1F;	/* extract short form ID code */
    if (IDC == 0x1F)	/* if long form */
	{
	IDC = 0;			/* prepare to shift in the IDCode */

        /* 
	** shift long form octets 2 through n right into IDCode 
	** this also converts from big endian to little endian format 
        */
        do 
	    {
	    Tag >>= 8;			/* shift them out of Tag */
	    IDC <<= 7;			/* make room for next 7 bits */
	    IDC |= (Tag & 0x7F);	/* get next 7 bits of IDCode */
	    }
	  while (Tag & 0x80);  /* while more bits to decode */
	};
    return (IDC);	/* return IDCode */
    }


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mcc_asn_length - returns actual length of a ILV encoding
**
**  FORMAL PARAMETERS:
**
**	p_ILVDesc	    - address of ILV encoding buffer descriptor
**
**  IMPLICIT INPUTS:
**
**	buffer should contain a valid ILV encoding (even if just word 0 )
**	or if no ILV encoding is present then dsc_a_pointer field should be 0
**	or if no descriptor then pass value 0 in place of address
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	length of encoding.  If no encoding supplied then length of 0 returned
**
**  SIDE EFFECTS:
**
**--
**/

int mcc_asn_length( p_ILVDesc )
MCC_T_Descriptor *p_ILVDesc;
{
int len = 0;
int *p_is_ilv;
int temp;   /* To align access */
unsigned char *p_ILV;
if (p_ILVDesc) /* if argument supplied */
    {
    /* if ILV encoding address known */
    if ((p_ILV = (unsigned char *)p_ILVDesc->mcc_a_pointer) != NULL)
	{
	p_is_ilv = (int *)p_ILV;
	memcpy( &temp, p_is_ilv, (size_t) sizeof(int) );
        if (temp)  /* if not an empty buffer */
            len = (unsigned char *)ilv_skip_value(p_ILV) - p_ILV;
	};
    /* else len = 0 */
    };
/* else len = 0 */

return( len );
}




int mcc_get_real(p_buf, p_data)
     unsigned   char *p_buf;      
      double  *p_data;
/******************************VAX and MIPS hardware specific *****************************/
/*      ASN.1 form of real:                                                               */
/* VAX bits    7         6         5-4             3-2                         1-0              */
/*  byte 0:| encoding | sign | (exponent) base | (fraction) scaling factor | exponent length |  */
/*  byte 1: exponent    */
/*  byte 2: exponent or fraction, depending on exponent length */
/*  byte 3-7: fraction */
/*                                                                                              */
/* Note that fraction is an unsigned integer, and the exponent is a signed integer.             */
/******************************************************************************************/
  {
#define BIN_ENCODING     0x80			/* FROM ASN STD DOCS */
#define BASE_2          0x00            /* exponent base is 2 */
#define EXPO_LEN_IS_1    0x00           /* exponent is 1 byte, 2's comp, in contents octet 2*/
#define EXPO_LEN_IS_2    0x01           /* exponent is 2 byte, 2's comp, in contents octets 2, 3 */
#define SHIFT_F 0x00                    /* scale factor (to adjust "decimal point") (not used) */
#define IEEE_BIAS      1023			    /*  mips double uses a biased exponent */
#define VAX_G_FLOAT_BIAS 1024           /* vax g_float uses "excess 1024" */
#define VAX_D_FLOAT_BIAS 128            /* vax f,d_float use "excess 128" */ 
#define VAX_G_DECR_EXPO        53		
#define VAX_D_DECR_EXPO        56		
#define IEEE_DECR_EXPO        52		
   unsigned char 	sign_bit,
                 	expo_len_bits,
                	 shift_f_bits,
                 	base_bits,
                 	encoding_bit,
                    *p,
                    tempbyte,
                    byte0, byte6;
   short int        temp_expo = 0;
   unsigned int 	i, f, mantissa_start;
   int  		step =1,
       			status = MCC_S_NORMAL;   
    MCC_T_Boolean flag;     /* IEEE (1) or D-Float flag (0) */
#define MAXSTEP 2

    do {
        switch(step++)
            {
            case 1:                    /* 1st octet specifies what follows. extract decoding info. */
                encoding_bit = (*p_buf) & 0x80;
                sign_bit     = 	(*p_buf) & 0x40;
                base_bits    = (*p_buf) & 0x30;
                shift_f_bits = (*p_buf) & 0x0c;
                expo_len_bits = (*p_buf)& 0x03;
                                    /* verify decoding info.  */
                if ( (encoding_bit  == BIN_ENCODING) &&		/* IEEE float encoding CHECK */
                    (base_bits     == BASE_2)      &&
                    (shift_f_bits  == SHIFT_F)	   &&
                    (expo_len_bits == EXPO_LEN_IS_2) )  
                    {
                    mantissa_start = 3;         /* if IEEE, fraction starts at byte 3; for D, at byte 2 */
                    flag = MCC_K_TRUE;
                    }
                else 
                    {
                    if ( (encoding_bit  == BIN_ENCODING) &&			/* VAX D_float encoding CHECK */
                        (base_bits     == BASE_2)      &&
        		        (shift_f_bits  == SHIFT_F)	   &&
                        (expo_len_bits == EXPO_LEN_IS_1) )  
                        {
                        mantissa_start = 2;         /* if IEEE, fraction starts at byte 3; for D, at byte 2 */
                        flag = MCC_K_FALSE;
                        }
                    else
                        return (MCC_S_ILVSYNTAXERROR);              /* We only do two kinds of float */
                    }
                break;
            case 2:

                if (flag)     /* if 2-octet form */
                    {
                    temp_expo = (*(p_buf+1));     /* extract 1st octet of exponent from encoding */
                    temp_expo <<= 8;
                    temp_expo = temp_expo | (*(p_buf+2));
                    }
                else
                    {
                    temp_expo = (*(p_buf+1));
                    temp_expo = (temp_expo & 0x000000ff);
                    }

#ifdef VMS
/****************************  VAX G_FLOAT *************/
/* 63-48, 47-32,   31-16    15    14-4     3-0         */
/*  fraction    ||       | sign | exp | fraction       */
/*******************************************************/

                temp_expo = temp_expo + VAX_G_FLOAT_BIAS + VAX_G_DECR_EXPO;  /* re-bias the exponent */

                byte0 = (temp_expo & 0x0f);            /* collect a nibble of exponent */
                byte0 <<=4;                             /* shift it into place */
                temp_expo >>=4;                         /* shift the rest of the exponent into place */
                tempbyte = (temp_expo & 0x007f);       /* collect the remaining 7 bits of exponent */
                if (sign_bit)                               /* insert fraction's sign, if present */
                    tempbyte |= (0x80);
                p = (unsigned char *)p_data;                             /* prepare to create the double byte by byte */
                *(p+1) = tempbyte;                      /* stash bits 8-15 */
                tempbyte = byte0 | ( (0x0f) & *(p_buf+mantissa_start) ); /* collect ms  5 bits, renormalizing */
                                                        /* the fraction by dropping the hidden bit */
                *p = tempbyte;                          /* stash bits 0-7 */
                *(p+2) = *(p_buf + mantissa_start + 2); /* rescramble the rest */
                *(p+3) = *(p_buf + mantissa_start + 1);
                *(p+4) = *(p_buf + mantissa_start + 4); 
                *(p+5) = *(p_buf + mantissa_start + 3);
                *(p+6) = *(p_buf + mantissa_start + 6); 
                *(p+7) = *(p_buf + mantissa_start + 5); 

#elif defined(mips) || defined(__mips) || defined(__osf__) || defined(sun) || defined(sparc) || defined(OSF)


/*********************MIPS/Alpha (IEEE) DOUBLE*******************/
/*  63      62-52      51-0                                     */
/*  sign | exponent | fraction                                  */
/*       msb -> lsb | msb -> lsb                                */
/****************************************************************/
                temp_expo = temp_expo + IEEE_BIAS + IEEE_DECR_EXPO;  /* re-bias the exponent */

                byte6 = (temp_expo & 0x0000000f);  /* collect a nibble of exponent*/
                byte6 <<=4;                         /* shift it into place */
                temp_expo >>=4;                     /* shift the rest of the exponent into place */
                tempbyte = (temp_expo & 0x0000007f);  /*collect the remaining 7 bits of exponent */
                if (sign_bit)                       
                    tempbyte |=0x80;                /* correct for fraction sign, if present */
                p = (unsigned char *)p_data;                         /* we want to make the double byte by byte */
                *(p+7) = tempbyte;                  /* stuff in bits 63-56 */
                tempbyte = byte6 | ( (0x0f) & *(p_buf+mantissa_start));  /* collect ms 4 or 5 bits--renormalizing */
                                                                         /* the fraction by dropping any hidden bit */
                *(p+6) = tempbyte;                  /* stuff in bits 55-48 */

                for (i = 1 ; i <= 6 ; i++)          /* the rest of the bits are in order */
                    {                               /* but the fraction may start at different places*/
                    *(p+(6-i)) = *(p_buf+mantissa_start+i);             
                    }

#elif vax
/**********************VAX D_FLOAT**********************************/
/* 63-48, 47-32,   31-16    15    14-7     6-0                     */
/*  fraction    ||       | sign | exp | fraction                   */
/*******************************************************************/

                if (flag) /* two-byte exponent was encoded; is it too large? */
                    {           /* options:  1) put in max/min exponent, decode fraction , return warning.  */
                                /* 2) continue to return plus or minus infinity, as follows */
                    if ((temp_expo) > (127))
                            *p_data = HUGE_VAL;
                    if ((temp_expo) < (-127))
                            *p_data = 0.0;
                    status = (MCC_S_ASNEXPOUTOFRANGE);
                    }

                temp_expo = temp_expo + VAX_D_FLOAT_BIAS + VAX_D_DECR_EXPO;  /* re-bias the exponent */

                byte0 = (temp_expo & 0xff);             /* collect exponent byte */
                byte0 >>=1 ;                            /* make room for sign bit */
                if (sign_bit)                               /* insert fraction's sign */
                    byte0 |= (0x80);
                 p = (unsigned char *)p_data;                            /* prepare to create the double byte by byte */
               *(p+1) = byte0;                          /* stash bits 8-15 */
               tempbyte = (temp_expo & 0x0001);        /* collect the remaining 1 bit of exponent */ 
	       tempbyte <<=7;				/* make it bit 7 */
               tempbyte |= ( (0x7f) & *(p_buf+mantissa_start) ); /* collect ms 8  bits, renormalizing */
                                                        /* the fraction by dropping the hidden bit */
                *p = tempbyte;                          /* stash bits 0-7 */
                *(p+2) = *(p_buf + mantissa_start + 2); /* rescramble the rest */
                *(p+3) = *(p_buf + mantissa_start + 1);
                *(p+4) = *(p_buf + mantissa_start + 4); 
                *(p+5) = *(p_buf + mantissa_start + 3);
                *(p+6) = *(p_buf + mantissa_start + 6); 
                *(p+7) = *(p_buf + mantissa_start + 5); 
                    
#endif

            break;
        }
   } while (( step <= MAXSTEP) && (status == MCC_S_NORMAL));
 return(status);
}


/******************************VAX and MIPS hardware specific *****************************/
/*      ASN.1 form of real:                                                                */
/* VAX bits    7         6         5-4             3-2                         1-0              */
/*  byte 0:| encoding | sign | (exponent) base | (fraction) scaling factor | exponent length |  */
/*  byte 1: exponent   msb */
/*  byte 2: exponent lsb or fraction MSB, depending on exponent length */
/*  byte 3-9: fraction  -- byte 3 is most significant of these */
/******************************************************************************************/


#ifdef vms             

int mcc_put_real(p_buff, p_native_value)
   char *p_buff,
        *p_native_value;
    {
    unsigned char 	    m1, sign;
    MCC_T_Unsigned32 	expo, temp, num_bytes;
    MCC_T_Integer32 	true_expo, bias;
    char 		*p;

/**************************  VAX G_FLOAT ****************************/
/* 63-48, 47-32,   31-16    15    14-4     3-0                      */
/*  fraction    ||       | sign | exp | fraction                    */
/********************************************************************/

/* converting the fraction to an integer by 53 bits shift left means expo is reduced by 53; vax G_float specific*/

    temp = *( (unsigned int  *)p_native_value);
    m1  =  temp & 0x0000000f;      /* 4 MSB s */
    temp >>= 4; 
    expo  = temp & 0x000007ff;      /* next 11 bits */
    temp >>= 11;
    sign = temp & 0x00000001;      /* next bit is the sign bit */
    
    if (sign)  sign = 0x40;
    *p_buff =  BIN_ENCODING | sign | BASE_2 | SHIFT_F | EXPO_LEN_IS_2;
    bias = VAX_G_FLOAT_BIAS + VAX_G_DECR_EXPO;

   true_expo = expo - bias ;		/* now store the exponent in 2's complement */
    p = (char *) (&true_expo);
    
    *(p_buff+1)   = *(p+1);
    *(p_buff+2)   = *p;

    m1   = m1 | 0x10; 			/* add the normalizing bit */

    *(p_buff+3) = m1;			/* form the mantissa an unsigned int (re-arrange bytes msb->lsb) */
    *(p_buff+4) = *(p_native_value +3);
    *(p_buff+5) = *(p_native_value +2);
    *(p_buff+6) = *(p_native_value +5);
    *(p_buff+7) = *(p_native_value +4);
    *(p_buff+8) = *(p_native_value +7);
    *(p_buff+9) = *(p_native_value +6);            

    num_bytes = REAL_BUFF_LEN;		/* force it for time being */

    return(num_bytes);

 }

/*************************END VAX G_FLOAT *************************/
 
/* #elif defined(unix) || defined(__unix) */

#elif defined(mips) || defined(__mips) || defined(__osf__) || defined(sun) || defined(sparc) || defined(OSF)
                            /* Alpha & Mips for CC & C89 (incl. -std) */

int mcc_put_real(p_buff, p_native_value)
   char *p_buff,
        *p_native_value;
    {
    unsigned char 	    sign;
    MCC_T_Unsigned32    m1, m2, temp, expo, num_bytes;
    MCC_T_Integer32 	true_expo;
    char 		*p;

/********************MIPS/Alpha (IEEE) DOUBLE********************/
/*  63      62-52      51-0                                     */
/*  sign | exponent | fraction                                  */
/*       msb -> lsb | msb -> lsb                                */
/*                                                              */
/*  equations: E= True exponent = exponent - bias               */
/*     mips double:  bias = 1023; E_max = +1023 E_min = -1022   */
/*   if E = E_max + 1 and fraction <> 0, error (not a number)   */
/*   if E = E_max+1 and fraction == 0, then the value is plus or*/
/*    minus inifinity, depending on the sign bit                */
/*   if E_min <= E <= E_max, then value is                      */
/*                  ( (-1)^s )*(2^E)*(1.fraction)               */
/*   if E == (E_min -1) and fraction <> 0, then value is        */
/*                  ( (-1)^s )*(2^E)*(0.fraction)               */
/*   if E = (E_min -1) and fraction == 0, then value is plus or */
/*        minus 0 (zero).                                       */
/*                                                              */
/*  We are not distinguishing plus or minus 0 --this case should*/
/*  not reach this code--we do the special ASN.1 encoding of 0  */
/*  in the caller of this routine.                              */
/*  NOTE: MIPS  can be configured big-endian or little-endian   */
/*        We require little endian, so the                      */
/*        bit orders of integers are the same on VAX or MIPS.   */
/****************************************************************/


/* converting the fraction to an integer by 52 bits shift left */
/* means exponent is reduced by 52; MIPS double SPECIFIC */
#define E_MIN_TEST (-1023)
#define E_MAX_TEST (1024)

/*  definitions from asn_get_ref - one byte encoding of special real numbers */
/* PLUS_INFINITY 0x40  */
/* MINUS_INFINITY 0x41  */

    m1 = *( (MCC_T_Unsigned32 *)(p_native_value) );  /* take most significant 4 bytes */

    temp = *( (MCC_T_Unsigned32 *)(p_native_value + 4) );   /*take the rest of the double*/

    if (temp & 0x80000000)              /* extract the sign bit */
        sign = 0x40;             
    else
         sign = 0x00;

    expo = (temp & 0x7ff00000);   /* extract 11 exponent bits */
    expo >>=20;                    /* shift them into place */
    expo = expo - IEEE_BIAS;      /* bias not needed */

    m2 = (temp & 0x000fffff);   /* extract the rest of the fraction*/
    
    if ( ( expo  == E_MAX_TEST ) && (m1 == 0) && (m2 == 0) )  /* require a "special real encoding" */
        {
        if (!sign)
            *p_buff = PLUS_INFINITY;
        else
            *p_buff = MINUS_INFINITY;
        num_bytes = 1;
        }
    else
        {
        true_expo = expo - IEEE_DECR_EXPO ;		    /* now fix the exponent */

        if ( expo !=  E_MIN_TEST ) 
            m2 |= 0x100000;           /* if not a "denormalized fraction", put in the "1." */

                            /* start with "encoding" byte at head */
        *p_buff =  BIN_ENCODING | sign | BASE_2 | SHIFT_F | EXPO_LEN_IS_2;

        p = (char *) (&true_expo);
                            /* next, two bytes of exponent */
        *(p_buff+1)   = *(p+1); 
        *(p_buff+2)   = *(p);             
                            /* now, fraction  */
        p = (char *) (&m2);
    
        *(p_buff + 3) = *(p+2);      /* want bytes numbered 0-2 of m2 */
        *(p_buff + 4) = *(p+1); 
        *(p_buff + 5) = *(p+0);    

        p = (char *) (&m1);

        *(p_buff + 6) = *(p+3);      /* want all of m1 */
        *(p_buff + 7) = *(p+2);    
        *(p_buff + 8) = *(p+1);    
        *(p_buff + 9) = *(p+0);    

        num_bytes = REAL_BUFF_LEN;		    /* don't compress the encoding */
        }
    return(num_bytes);

 }

/*********************END MIPS/Alpha (IEEE) DOUBLE************************/

#elif vax

int mcc_put_real(p_buff, p_native_value)
   char *p_buff,
        *p_native_value;
    {
    unsigned char 	    m1, sign;
    MCC_T_Unsigned32 	expo, temp, num_bytes;
    MCC_T_Integer32 	true_expo, bias;
    char 		*p;

/**********************VAX D_FLOAT**********************************/
/* 63-48, 47-32,   31-16    15    14-7     6-0                      */
/*  fraction    ||       | sign | exp | fraction                    */
/* converting the fraction to an unsigned integer by 56 bits shift left means expo is reduced by 56; */
/*VAX D_FLOAT SPECIFIC */

    temp = *( (unsigned int  *)p_native_value);
    m1  =  temp & 0x0000007f;      /* 7 LSB s */
    temp >>= 7; 
    expo  = temp & 0x000000ff;      /* next 8 bits */
    temp >>= 8;
    sign = temp & 0x00000001;      /* next bit is the sign bit */
    
    if (sign)  sign = 0x40;
    *p_buff =  BIN_ENCODING | sign | BASE_2 | SHIFT_F | EXPO_LEN_IS_1;
    bias = VAX_D_FLOAT_BIAS + VAX_D_DECR_EXPO;

   true_expo = expo - bias ;		/* now store the exponent in 2's complement */
    p = (char *) (&true_expo);
    
    *(p_buff+1)   = *p;

    m1   = m1 | 0x80; 			/* add the normalizing bit */

    *(p_buff+2) = m1;			/* form the mantissa an unsigned int (re-arrange bytes msb->lsb) */
    *(p_buff+3) = *(p_native_value +3);
    *(p_buff+4) = *(p_native_value +2);
    *(p_buff+5) = *(p_native_value +5);
    *(p_buff+6) = *(p_native_value +4);
    *(p_buff+7) = *(p_native_value +7);
    *(p_buff+8) = *(p_native_value +6);            

    num_bytes = REAL_BUFF_LEN - 1;		/* force it for time being */

    return(num_bytes);

 }

/***********************END VAX D_FLOAT******************************/
#endif  
/* mips|alpha or vax */
/* #endif */
/* vms or unix */
