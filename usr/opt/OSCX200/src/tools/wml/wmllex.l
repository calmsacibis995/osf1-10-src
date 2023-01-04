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
%}
%%
	char endstg;

[ \t\f]		{}

[\n]		{ wml_line_count += 1; }

"Class"			{ return CLASS; }
"Resource"		{ return RESOURCE; }
"DataType"		{ return DATATYPE; }
"ControlList"		{ return CONTROLLIST; }
"EnumerationSet"	{ return ENUMERATIONSET; }
"EnumerationValue"	{ return ENUMERATIONVALUE; }
"CharacterSet"		{ return CHARACTERSET; }

"DocName"		{ return DOCNAME; }
"ConvenienceFunction"	{ return CONVFUNC; }
"Alias"			{ return ALIAS; }
"Type"			{ return TYPE; }
"ResourceLiteral"	{ return RESOURCELITERAL; }
"Related"		{ return RELATED; }
"InternalLiteral"	{ return INTERNALLITERAL; }
"Constraint"		{ return CONSTRAINT; }
"Exclude"		{ return EXCLUDE;}
"Resources"		{ return RESOURCES; }
"SuperClass"		{ return SUPERCLASS; }
"Controls"		{ return CONTROLS; }
"WidgetClass"		{ return WIDGETCLASS; }
"DialogClass"		{ return DIALOGCLASS; }
"Default"		{ return DEFAULT; }
"EnumLiteral"		{ return ENUMLITERAL; }
"XmStringCharsetName"	{ return XMSTRINGCHARSETNAME; }
"Direction"		{ return DIRECTION; }
"ParseDirection"	{ return PARSEDIRECTION; }
"CharacterSize"		{ return CHARACTERSIZE; }
"ControlsMapToResource"	{ return CTRLMAPSRESOURCE; }

"MetaClass"		{ return METACLASS;}
"Widget"		{ return WIDGET;}
"Gadget"		{ return GADGET;}
"Argument"		{ return ARGUMENT;}
"Reason"		{ return REASON;}
"Constraint"		{ return CONSTRAINT;}
"SubResource"		{ return SUBRESOURCE;}
"True"			{ return ATTRTRUE; }
"False"			{ return ATTRFALSE; }
"LeftToRight"		{ return LEFTTORIGHT; }
"RightToLeft"		{ return RIGHTTOLEFT; }
"OneByte"		{ return ONEBYTE; }
"TwoByte"		{ return TWOBYTE; }
"MixedOneAndTwoByte"	{ return MIXED1_2BYTE; }

":"			{ return COLON; }
";"			{ return SEMICOLON; }
"="			{ return EQUALS; }
"{"			{ return LBRACE; }
"}"			{ return RBRACE; }

"!"[^\n]*	{}

[a-zA-Z][a-zA-Z0-9$_]* {
	strcpy (yystringval, yytext);
	return STRING;
	}

\"[^"\n]*	{
	if ( yytext[yyleng-1] == '\\' )
		yymore();
	else
		{
		endstg = input ();
		if ( endstg == '\"' )
			{
			strcpy (yystringval, &yytext[1], yyleng-1);
			return STRING;
			}
		else
			{
			printf ("\nUnterminated string near %s, line %d",
				yytext, wml_line_count);
			return ERRORTOKEN;
			}
		}
	}

.	{
	printf ("\nEncountered illegal character '%c', line %d",
		yytext[0], wml_line_count);
	return ERRORTOKEN;
	}

