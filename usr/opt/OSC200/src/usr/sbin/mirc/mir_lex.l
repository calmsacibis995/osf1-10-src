%{
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
 * @(#)$RCSfile: mir_lex.l,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:00:28 $
 */
%}
%p 8200
%e 1500
%n 1100
%a 6100
%o 8300
%{

/*
 *
 * Copyright (C) 1989 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * mir_lex.l - lexical analyzer for MSL compiler
 *
 *  Derived from DECnet-ULTRIX Dictionary Compiler ("MD") written by:
 *      B. M. England
 *
 *  Enhanced to full MCC MSL syntax & function by:
 *      Larry Grossman     17-JAN-1991
 *
 *  Modified for use with the Common Agent Managaement Information
 *  Repository "back-end" by:
 *      D. D. Burns        March 1992
 *
 *
 *  MODIFICATION HISTORY:
 *
 *  Date        By              Reason
 * -----------------------------------------------------------------
 * 06-Aug-92  Pete Burgess    Support debugging logical - mcc_msl_log
 * 09-Sep-92  D. D. Burns     Add new keywords "GENERIC-PRESENTATION",
 *                            "BIDT_VERSION", and "AUGMENT"
 * 30-Oct-92  D. D. Burns     Add new keyword "MCC_DATATYPE_SIZE" 
 * 21-Jan-93  D. D. Burns     Add new keyword "NCL_CMIP_CODE" 
 */
 
/* Do not redefine IO routines */
#define CMA_STDIO 1

#include <string.h>
#include "mcc_interface_def.h"
#include "mir_yacc.h"

/* Request definitions for compiler modules from "mir.h" */
#define MIR_COMPILER
#include "mir.h"  /* For function prototypes only */

int
yylook PROTOTYPE((
));

int
yyback PROTOTYPE((
));

#if 0
#include "msl_types.h"
#include "mccmslmsg.h"
#endif

#define EOS '\0'

/* Increase the allowable size of a TOKEN */
#undef YYLMAX
#define YYLMAX 2048

#ifdef DEBUG
extern unsigned int mcc_msl_log;
# define mcc_msl_debug_lex 1

# define deb_macro(out_string) \
	if ((mcc_msl_log & mcc_msl_debug_lex) != 0) \
	    if ((strcmp(out_string, "ID_ANYCASE") == 0) || \
		(strcmp(out_string, "ID_QUOTED") == 0)) \
		printf("%s, value = '%s'\n", out_string, yytext); \
	    else \
		printf("%s\n", out_string)
#else
#define deb_macro(out_string)
#endif

  /* tell YACC what value of terminal string is */
extern int yylval;	/* linkage to YACC-produced y.tab.c module */

/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
extern char *mp700();
extern char *mp701();
extern char *mp702();

#endif


#ifdef DEBUG
extern MCC_T_Descriptor out_desc;
#endif

/*
 * NOTE!!! This DEFINE statement is essential to the ability of the 
 * input.c module to manage multiple nested include files and make them
 * look like a continuous stream.
 */
#undef getc
#define getc( file_pointer ) inputch()	/* call routine in input.c, not getc()*/

%}
%Start ENUM
%%
\(\*		       { /* accept comment */
			int cmt_state;
			/* 
			 * cmt_state variable has one of three values:
			 *	1	expecting asterisk
			 *	2	expecting right parenthesis
			 *	0	found end of comment
			 */ 

			cmt_state = 1;	/* expecting asterisk */
			while (cmt_state) /* until you see asterisk rparen*/
			{
			char ch;
			ch = input();	/* get next character */
			switch( ch ) {
			  case '*': 
				    if (cmt_state == 1) cmt_state = 2;
				    break;
			  case ')': 
				    if (cmt_state == 2) cmt_state = 0;
				    else cmt_state = 1;
				    break;
			  case 0:   
				/* Hit EOF while inside a comment */
				/* msl_error( MCCMSL_S_MISCOMMENTEND */
                                    warn(MP(mp700,"EOF encountered inside comment"));
				    cmt_state = 0;
				    break;
			  default: 
				    cmt_state = 1;
				    break;
			  }
			}
		       return( yylex() );	/* skip over comments */
		       }
<ENUM>([^,= \(\t\n]|(\([^\*]))+	{ /* Special state for Enumerated TYPE names */
                        deb_macro("ID_ANYCASE");
			yylval = yyleng;
			return( ID_ANYCASE );}
ACCESS			{deb_macro("KW_ACCESS"); return( KW_ACCESS ); }
ACTION			{deb_macro("KW_ACTION"); return( KW_ACTION );}
ARGUMENT		{deb_macro("KW_ARGUMENT"); return( KW_ARGUMENT ); }
AS			{deb_macro("KW_AS"); return( KW_AS );}
ASN1\_CLASS             {deb_macro("KW_ASN1_CLASS"); return( KW_ASN1_CLASS );}
ASN1\_TAG               {deb_macro("KW_ASN1_TAG"); return( KW_ASN1_TAG );}
ATOMIC                  {deb_macro("KW_ATOMIC"); return( KW_ATOMIC );}
ATTRIBUTE		{deb_macro("KW_ATTRIBUTE"); return( KW_ATTRIBUTE ); }
ATTRIBUTES		{deb_macro("KW_ATTRIBUTES"); return( KW_ATTRIBUTES ); }
ATTRIBUTE\-LIST		{deb_macro("KW_ATTRIBUTE_LIST"); return( KW_ATTRIBUTE_LIST ); }
AUGMENT                 {deb_macro("KW_AUGMENT"); return( KW_AUGMENT ); }
AUX\_INFO               {deb_macro("KW_AUX_INFO"); return( KW_AUX_INFO ); }
BIDT\_VERSION           {deb_macro("KW_BIDT_VERSION"); return( KW_BIDT_VERSION );}
BITSET			{deb_macro("KW_BITSET"); return( KW_BITSET );}
CASE			{deb_macro("KW_CASE"); return( KW_CASE );}
CATEGORIES		{deb_macro("KW_CATEGORIES"); return( KW_CATEGORIES ); }
CHARACTERISTIC		{deb_macro("KW_CHARACTERISTIC"); return( KW_CHARACTERISTIC ); }
CHILD			{deb_macro("KW_CHILD"); return( KW_CHILD ); }
COUNTED			{deb_macro("KW_COUNTED"); return( KW_COUNTED );}
COUNTER			{deb_macro("KW_COUNTER"); return( KW_COUNTER ); }
DEFAULT			{deb_macro("KW_DEFAULT"); return( KW_DEFAULT ); }
DEFINING\-SMI           {deb_macro("KW_DEFINING_SMI"); return( KW_DEFINING_SMI ); }
DEPENDS			{deb_macro("KW_DEPENDS"); return( KW_DEPENDS ); }
DIRECTIVE		{deb_macro("KW_DIRECTIVE"); return( KW_DIRECTIVE ); }
DIRECTIVE\-TYPE		{deb_macro("KW_DIRECTIVE_TYPE"); return( KW_DIRECTIVE_TYPE );}
DISPLAY			{deb_macro("KW_DISPLAY"); return( KW_DISPLAY ); }
DNA_CMIP_INT		{deb_macro("KW_DNA_CMIP_INT"); return( KW_DNA_CMIP_INT );}
DNS\_IDENT		{deb_macro("KW_DNS_IDENT"); return( KW_DNS_IDENT ); }
DYNAMIC			{deb_macro("KW_DYNAMIC"); return( KW_DYNAMIC ); }
ECHO			{deb_macro("KW_ECHO"); return( KW_ECHO ); }
END			{deb_macro("KW_END"); return( KW_END ); }
END\_SMI\_DEF           {deb_macro("KW_END_SMI_DEF"); return( KW_END_SMI_DEF ); }
ENTITY			{deb_macro("KW_ENTITY"); return( KW_ENTITY ); }
EVENT			{deb_macro("KW_EVENT"); return( KW_EVENT );}
EVENT\-LIST		{deb_macro("KW_EVENT_LIST"); return( KW_EVENT_LIST );}
EXAMINE			{deb_macro("KW_EXAMINE"); return( KW_EXAMINE );}
EXCEPTION		{deb_macro("KW_EXCEPTION"); return( KW_EXCEPTION );}
FALSE			{deb_macro("KW_FALSE"); return( KW_FALSE ); }
GENERIC\-PRESENTATION   {deb_macro("KW_GENERIC_PRESENTATION"); return( KW_GENERIC_PRESENTATION ); }
GLOBAL			{deb_macro("KW_GLOBAL"); return( KW_GLOBAL ); }
GROUP			{deb_macro("KW_GROUP"); return( KW_GROUP );}
IDENTIFIER[ \t]*\=	{deb_macro("KW_IDENTIFIER_EQ"); return( KW_IDENTIFIER_EQ ); }
IDENTIFIER		{deb_macro("KW_IDENTIFIER"); return( KW_IDENTIFIER ); }
IMPLEMENTATION		{deb_macro("KW_IMPLEMENTATION"); return( KW_IMPLEMENTATION ); }
IN			{deb_macro("KW_IN"); return( KW_IN );}
INCLUDE			{deb_macro("KW_INCLUDE"); return( KW_INCLUDE );}
INITIAL			{deb_macro("KW_INITIAL"); return( KW_INITIAL ); }
KEYWORDS		{deb_macro("KW_KEYWORDS"); return( KW_KEYWORDS); } 
MANAGEMENT		{deb_macro("KW_MANAGEMENT"); return( KW_MANAGEMENT ); }
MCC\_DATATYPE\_SIZE     {deb_macro("KW_MCC_DATATYPE_SIZE"); return( KW_MCC_DATATYPE_SIZE ); }
MODIFY			{deb_macro("KW_MODIFY"); return( KW_MODIFY );}
NCL_CMIP_CODE           {deb_macro("KW_NCL_CMIP_CODE"); return( KW_NCL_CMIP_CODE );}
NO			{deb_macro("KW_NO"); return( KW_NO ); }
NONSET(T)?ABLE		{deb_macro("KW_NONSETTABLE"); return( KW_NONSETTABLE ); }
OF			{deb_macro("KW_OF"); return( KW_OF );}
OID			{deb_macro("KW_OID"); return( KW_OID );}
OSI_CMIP_OID		{deb_macro("KW_OSI_CMIP_OID"); return( KW_OSI_CMIP_OID );}
PARENT			{deb_macro("KW_PARENT"); return( KW_PARENT ); }
PARTITION		{deb_macro("KW_PARTITION"); return( KW_PARTITION ); }
PREDICTABLE		{deb_macro("KW_PREDICTABLE"); return( KW_PREDICTABLE );}
PRIVATE			{deb_macro("KW_PRIVATE"); return( KW_PRIVATE );}
RANGE			{deb_macro("KW_RANGE"); return( KW_RANGE );}
RECORD			{deb_macro("KW_RECORD"); return( KW_RECORD );}
REFERENCE		{deb_macro("KW_REFERENCE"); return( KW_REFERENCE ); }
REQUEST			{deb_macro("KW_REQUEST"); return( KW_REQUEST ); }
REQUIRED		{deb_macro("KW_REQUIRED"); return( KW_REQUIRED );}
RESPONSE		{deb_macro("KW_RESPONSE"); return( KW_RESPONSE ); }
SEQUENCE		{deb_macro("KW_SEQUENCE"); return( KW_SEQUENCE );}
SET			{deb_macro("KW_SET"); return( KW_SET );}
SET(T)?ABLE		{deb_macro("KW_SETTABLE"); return( KW_SETTABLE ); }
SMI\_BUILTIN\_TYPE      {deb_macro("KW_SMI_BUILTIN_TYPE"); return( KW_SMI_BUILTIN_TYPE ); }
SNMP_OID		{deb_macro("KW_SNMP_OID"); return( KW_SNMP_OID );}
SPECIFIC		{deb_macro("KW_SPECIFIC"); return( KW_SPECIFIC ); }
SPECIFICATION		{deb_macro("KW_SPECIFICATION"); return( KW_SPECIFICATION ); }
START\_SMI		{deb_macro("KW_START_SMI"); return( KW_START_SMI ); }
STATISTIC		{deb_macro("KW_STATISTIC"); return( KW_STATISTIC ); }
STATUS			{deb_macro("KW_STATUS"); return( KW_STATUS ); }
SYMBOL			{deb_macro("KW_SYMBOL"); return( KW_SYMBOL ); }
SYMBOL\-PREFIX		{deb_macro("KW_SYMBOL_PREFIX"); return( KW_SYMBOL_PREFIX );}
TEXT			{deb_macro("KW_TEXT"); return( KW_TEXT ); }
TRUE			{deb_macro("KW_TRUE"); return( KW_TRUE ); }
TYPE			{deb_macro("KW_TYPE"); return( KW_TYPE );}
UNITS			{deb_macro("KW_UNITS"); return( KW_UNITS ); }
VERSION			{deb_macro("KW_VERSION"); return( KW_VERSION ); }
WRITEONLY		{deb_macro("KW_WRITEONLY"); return( KW_WRITEONLY ); }
[A-Za-z0-9\_][A-Za-z0-9\_\-\/\.\$]* { /* accept name of attribute/argument */
			yylval = yyleng;
			deb_macro("ID_ANYCASE");
			return( ID_ANYCASE );
			}
\"(([^"])|(\"\"))*	{ /* ACCEPT quoted string */ 
			    char ch;
			    /* find out why quoted string ended */
                            if (yytext[yyleng-1] == '\\')
		                yymore();
			    else
			    {
			        /* printf("quoted string %s\"\n",yytext); */
			        if ( (ch = input()) == '\"')
  			        {
				    yylval = yyleng;
				    deb_macro("ID_QUOTED");
				    return ID_QUOTED;
		                }
				else
				{
				    /* end of file = input return of 0 */
				    /* msl_error( MCCMSL_S_MISMATCHQUOTE */
                                    warn(MP(mp701,"EOF hit inside quoted string"));
				    yylval = 0;
				    deb_macro("ID_QUOTED");
				    return ID_QUOTED;
				}
                            }
	               } 
\=		       {deb_macro("P_EQUAL"); return(P_EQUAL); }
\;			{deb_macro("P_SEMICOLON"); return(P_SEMICOLON); }
\,			{deb_macro("P_COMMA"); return(P_COMMA); }
\(			{deb_macro("P_LPAREN"); return(P_LPAREN); }
\)			{deb_macro("P_RPAREN"); return(P_RPAREN); }
\(			{deb_macro("P_LPAREN"); return(P_LPAREN); }
\)			{deb_macro("P_RPAREN"); return(P_RPAREN); }
\:			{deb_macro("P_COLON"); return(P_COLON); }
\[			{deb_macro("P_LSQRBRKT"); return(P_LSQRBRKT); }
\]			{deb_macro("P_RSQRBRKT"); return(P_RSQRBRKT); }
\{			{deb_macro("P_LBRACE"); return(P_LBRACE); }
\}			{deb_macro("P_RBRACE"); return(P_RBRACE); }
\.{2,2}			{deb_macro("P_ELLIPSES"); return(P_ELLIPSES); }
\.			{deb_macro("P_PERIOD"); return(P_PERIOD); }
\*			{deb_macro("P_ASTERISK"); return(P_ASTERISK);}
[ \f\t\n]+ 		;/* printf("whitespace\n"); accept whitespace*/	
.			{deb_macro("bad_character");
#if 0
			out_desc.mcc_a_pointer = (MCC_T_Unsigned8 *)&yytext[0];
			out_desc.mcc_w_maxstrlen = strlen(&yytext[0]);
			warn("msl_error_1( MCCMSL_S_UNEXPCHARACTER, &out_desc)");
#endif
                        warn(MP(mp702,"Unexpected character encountered"));
			return (BAD_CHARACTER);}
%%
/*
| This function switches LEX into a special state that allows it to accept
| almost anything as an Enumerated TYPE name definition (including strings
| which otherwise would be recognized by LEX as keywords).
*/
int begin_enum()
{BEGIN ENUM;}

/*
| This function switches LEX back to it's "normal" state for everything other
| than Enumerated TYPE name definitions.
*/
int begin_initial()
{BEGIN 0;}
