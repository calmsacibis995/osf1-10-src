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
static char *rcsid = "@(#)$RCSfile: mcc_asn1_dump.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:23:06 $";
#endif
/* #module MCC_ASN1_DUMP "X1.2.0" */
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
** MODULE MCC_ASN1_DUMP                                                     **
**                                                                          **
** History:                                                                 **
** April-30-90 Jesuraj  added NCL/CML include files                         **
** x0.10.3 7-Jun-1990  REJK                                                 **
**          portable naming changes
** ut1.0.1 19-Jul-1990  JAS
**          port to pcc/Ultrix
** x1.2.0  01-aug-1991  REJK
**          cleanup include files for portable cmip
-------
   October 1991 - Converted for use w/ SNMP Protocol Engine

   - INCLUDED header files are changed to avoid MCC files of the same
     name.  The substring "mcc_" was removed and "snmppe_" substituted.

   - Argument list changed to include buffer in which printline images
     are to be built along with the position in the buffer where each
     new line is to be started.  The buffer contains a line prefix suitable
     for distinguishing it from other lines dumped into the log file by
     SNMP PE.
*/

#include <ctype.h>

#ifdef vms         		/* 	VMS 	*/
#include <stsdef.h>
#endif                                 

#include <stdio.h>
#include <string.h>

#include "snmppe_descrip.h"
#include "snmppe_interface_def.h"
#include "snmppe_msg.h"
#include "asn_def.h"

#define STS_M_SUCCESS 1

#define MAX_STRING_SIZE 512

#define TRY( x ) if ((TryStatus = (x)) != MCC_S_NORMAL) {                    \
                       OUTNL;                                                \
                       sprintf(&lbuf[pos], "ASN.1 Dump Abort on code %d", x);\
                       OUTNL;                                                \
                       return(TryStatus);                                    \
                       }

#define OUTNL   sprintf(&lbuf[strlen(lbuf)], "\n");     \
                fprintf(ofile, lbuf);                   \
                lbuf[pos] = '\0';


mcc_asn1_dump( p_ILVDesc, ofile, lbuf, pos )
MCC_T_Descriptor *p_ILVDesc; /* Descriptor of ASN.1 Buffer to be dumped      */
FILE    *ofile;              /* Output file to use for dump                  */
char    lbuf[];              /* Line buffer                                  */
int     pos;                 /* Position in Line Buffer for line images      */

{
struct ASNContext Ctx;
int status, ActualLen;
unsigned int IDCode, Tag;
int Implicit;
int Type, IntBuf[4];
unsigned int Form, Class;
int ConsLevel = 0;
int TryStatus;  /* used by TRY macro */
char ByteBuf[300]; /* was static char ByteBuf[300]; */
int in_pdu_length;
extern int mcc_asn_get_init();
extern int mcc_asn_length();
extern int mcc_asn_get_tag();
extern int mcc_asn_get_ref();
extern unsigned int  mcc_asn_tag_form();
extern unsigned int  mcc_asn_tag_class();
extern unsigned int  mcc_asn_tag_id();
int mcc_asn_Indent();
int mcc_asn_OutByteStr();
int mcc_asn_printable();


TRY(mcc_asn_get_init( &Ctx, p_ILVDesc ));

in_pdu_length = mcc_asn_length( p_ILVDesc );
sprintf(&lbuf[strlen(lbuf)],
        "ASN.1 Dump of PDU Buffer, Size = %d (decimal), %x (hex)",
        in_pdu_length, in_pdu_length
        );

if ( in_pdu_length != 0)	/* if empty parameter list */
  do 
    {
    status = mcc_asn_get_tag( &Ctx, &Tag );
    if (status == MCC_S_ILVEOC)
	    {
	    OUTNL;
    	mcc_asn_Indent(ConsLevel, lbuf);
    	sprintf(&lbuf[strlen(lbuf)], ")");
    	ConsLevel--;
	    }
    else 
	    {
    	TRY(status);	/* exit if fatal error */
        Form = mcc_asn_tag_form( Tag );
        Class = mcc_asn_tag_class( Tag );
        IDCode = mcc_asn_tag_id( Tag );
	    OUTNL;
	    mcc_asn_Indent(ConsLevel, lbuf);
	    sprintf(&lbuf[strlen(lbuf)],  "[ " );
#ifdef PORT_CMIP
	    switch (Class)
	        {
	        case MCC_K_ASN_CL_APPL:
    		    sprintf(&lbuf[strlen(lbuf)],  "APPL" ); break;
	        case MCC_K_ASN_CL_PRIV:
	    	    sprintf(&lbuf[strlen(lbuf)],  "PRIV" ); break;
	        case MCC_K_ASN_CL_CONT:
		        sprintf(&lbuf[strlen(lbuf)],  "CONT" ); break; 
	        case MCC_K_ASN_CL_UNIV:
		        sprintf(&lbuf[strlen(lbuf)],  "UNIV" ); break; 
		    default : 
                break;
	        };
#else
        switch (Class)
	        {
	        case MCC_K_ASN_CL_APPL:
		        sprintf(&lbuf[strlen(lbuf)],  "APPLICATION" ); break;
	        case MCC_K_ASN_CL_PRIV:
		        sprintf(&lbuf[strlen(lbuf)],  "PRIVATE" ); break;
            default:
                break;  /* if UNIV, CONT or something else then don't output class */
    	    };
#endif
	    sprintf(&lbuf[strlen(lbuf)],  " %d ] ", IDCode );
	    if (Form == MCC_K_ASN_FORM_CONS) 
	        {
	        sprintf(&lbuf[strlen(lbuf)],  "( " );
	        ConsLevel++;
                if (Class == MCC_K_ASN_CL_UNIV)  /* UNIV is always EXplicit */
                  {
                  Implicit = MCC_K_FALSE;
	          status = mcc_asn_get_ref ( &Ctx, Tag, &Type, 0, 0, 0 );
                  }
                else                            /* must be implicit */
                  {
    		  Implicit = MCC_K_TRUE;
    		  status = mcc_asn_get_ref ( &Ctx, -MCC_K_ASN_DT_SEQUENCE, &Type, 0, 0, 0 );
	    	  };
	        TRY(status);
	        }
	    else /* primitive form */
	        {
	        /* try to find out the type */
	        Implicit = MCC_K_FALSE;
	        status = mcc_asn_get_ref(
			    &Ctx, 0, &Type,
			    &ByteBuf[0], sizeof(ByteBuf), &ActualLen );
	        if (status == MCC_S_ILVNOTYPEFOUND)
		        {
		        status = mcc_asn_get_ref(
			        &Ctx, -MCC_K_ASN_DT_OCTETSTRING, &Type,
			        &ByteBuf[0], sizeof(ByteBuf), &ActualLen );
		        TRY(status);
		        mcc_asn_OutByteStr( &ByteBuf[0], ActualLen, ConsLevel, lbuf, ofile, pos );
		        if (mcc_asn_printable( &ByteBuf[0], ActualLen ))
		            {
		            ByteBuf[ActualLen]=NULL;
		            sprintf(&lbuf[strlen(lbuf)],  " -- %s", &ByteBuf[0] );
		            };
		        Implicit = MCC_K_TRUE;
		        }
	        else if (status == MCC_S_ILVMISTYPED) /* Type contains the type found */
		            {
            		if (Type == MCC_K_ASN_DT_INTEGER)
                            {
                            IntBuf [0] = 0;
                            IntBuf [1] = 0;
                            IntBuf [2] = 0;
                            IntBuf [3] = 0;
                            status = mcc_asn_get_ref( &Ctx, Type, 0, &IntBuf[0], 16, &ActualLen );
                            TRY(status);
                            /* note that the following prints the contents of IntBuf by byte in hex */
                            if (ActualLen <= 4 )       sprintf(&lbuf[strlen(lbuf)],  "%08X", IntBuf[0] );
                            else if (ActualLen <= 8 )  sprintf(&lbuf[strlen(lbuf)],  "%08X, %08X", IntBuf[0], IntBuf[1]);
                            else                       sprintf(&lbuf[strlen(lbuf)],  "%08X, %08X, %08X, %08X", 
                                                                IntBuf[0], IntBuf[1], IntBuf[2], IntBuf[3] );
                            }
                        else
                            {
                            status = mcc_asn_get_ref( &Ctx, Type, 0, &ByteBuf[0], sizeof(ByteBuf), &ActualLen );
                            TRY(status);
                            if (Type == MCC_K_ASN_DT_BITSTRING ) ActualLen = (ActualLen+7)/8;
                            mcc_asn_OutByteStr( &ByteBuf[0], ActualLen, ConsLevel, lbuf, ofile, pos );
                            if (mcc_asn_printable( &ByteBuf[0], ActualLen ))
                                    {
        	        		ByteBuf[ActualLen]=NULL;
                			sprintf(&lbuf[strlen(lbuf)],  " -- %s", &ByteBuf[0] );
                			};
            		    }
        		}
	        else {
                       OUTNL;
                       sprintf(&lbuf[pos], "ASN.1 Dump Internal Abort");
                       OUTNL;
                       return(MCC_S_FATAL); /* shouldn't happen */
                    }
	        };
	    };
    }
    while (ConsLevel != 0);

OUTNL;
return( MCC_S_NORMAL );
}

int mcc_asn_printable( p_Buf, BufLen )
char *p_Buf;
int BufLen;
{
int i;

if (BufLen == 0) return(MCC_K_FALSE);
for (i=0; i<BufLen; i++)
    if (!isprint( *(p_Buf+i))) break;
return(i==BufLen);
}


/* this routine does indenting for mcc_ilv_dump */

int mcc_asn_Indent( ConsLevel, lbuf )
int ConsLevel;
char lbuf[];
{
int i;

for (i=0; i<ConsLevel; i++) sprintf(&lbuf[strlen(lbuf)], "    ");
}

/* this routine displays a byte string in hexadecimal
** using the specified indentation level */

int mcc_asn_OutByteStr( p_Str, Len, ConsLevel, lbuf, ofile, pos )
unsigned char *p_Str;
int Len, ConsLevel;
char lbuf[];
FILE    *ofile;                         /* Output file to use for dump       */
int     pos;                            /* Position in Line Buffer for image */
{
unsigned char *p_ch;
int mcc_asn_Indent();


if (Len > 20) {
    OUTNL;
    }

for (p_ch = p_Str; p_ch < (p_Str+Len); p_ch++) 
    {
    if (((p_ch - p_Str)%20) == 0) 
    	{
	    if (p_ch - p_Str > 0) {
              OUTNL;
              }
    	mcc_asn_Indent( ConsLevel, lbuf );
    	};
    sprintf(&lbuf[strlen(lbuf)], "%02x ",*p_ch);
    };
}
