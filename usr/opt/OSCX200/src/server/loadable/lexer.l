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
 * Creation
 */
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1992 BY                  		    *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/
%}
%%
!.*$	;
\<	{
		level++;
		return(START_SUBCONTEXT);
	}
\>	{
		level--;
		return(END_SUBCONTEXT);
	}
system	{	
		if ( level == 0 )
			return(COMPONENT_SYSTEM); 
		else
			{AddArgs(yytext); return(STRING);}
	}
core	{	
		if ( level == 0 )
			return(COMPONENT_CORE); 
		else
			{AddArgs(yytext); return(STRING);}
	}
replace	{	
		if ( level == 0 )
			return(CORE_REPLACE); 
		else
			{AddArgs(yytext); return(STRING);}
	}
device	{	
		if ( level == 0 )
			return(COMPONENT_DEVICE); 
		else
			{AddArgs(yytext); return(STRING);}
	}
extensions	{	
		if ( level == 0 )
			return(COMPONENT_EXTENSIONS); 
		else
			{AddArgs(yytext); return(STRING);}
	}
font_renderers	{	
		if ( level == 0 )
			return(COMPONENT_FONT_RENDERERS); 
		else
			{AddArgs(yytext); return(STRING);}
	}
auth_protocols	{	
		if ( level == 0 )
			return(COMPONENT_AUTH); 
		else
			{AddArgs(yytext); return(STRING);}
	}
transports	{	
		if ( level == 0 )
			return(COMPONENT_TRANSPORTS); 
		else
			{AddArgs(yytext); return(STRING);}
	}
input		{	
		if ( level == 0 )
			return(COMPONENT_INPUT); 
		else
			{AddArgs(yytext); return(STRING);}
	}
library_path	{
		if (level == 0 )
			return(LIBRARY_PATH);
		else
			{AddArgs(yytext); return(STRING);}
		}
args	{
		if ( level == 0 )
			return(ARGS); 
		else
			{AddArgs(yytext); return(STRING);}
	}
[^\t\ \n]+ {{AddArgs(yytext); return(STRING);}}
\n	lines++;
.	;
