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
/********************************************************************/
/*                    Compiler Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes                             */
/*	 ----   ----        ----------------									  */
/*  bsb   9/7/90       1.0	/ Initial general customer release       */
/********************************************************************/
/*                    main.c Revisions										  */
/*																						  */
/*  init   Date        Revision/Changes									  */
/*  ----   ----        -----------------									  */
/*  bsb   9/7/90       1.0 / Initial general customer release       */
/********************************************************************/

#include <string.h>
#include "pass2.h"  /* files for 2nd pass of the compiler */
#include "scsi.h"
#include YACC_HEADER

#define MAX_FILE_LEN 33		 /* maximum file name length */

FILE *input_file;				/* SCRIPT source file */
BOOL err_flag = 0;			/* Is error file to be generated */
BOOL undefine_flag = 0;		/* Dont define instruction or patches */
BOOL lis_flag = 0;			/* Is list file to be generrated  */
BOOL out_flag = 0;			/* Is output file to be generated */
BOOL deb_flag = 0;			/* Is debug file to be generated   */
BOOL verbose_flag = 0;		/* Is screen information to be generated */

FILE *lisfile;					/* List file */
FILE *errfile;					/* Error file */
FILE *debfile;					/* Debug file  used if SCRIPT debugger is to be used */
FILE *outfile = stdout;					/* Output file */

int iteration_num;			/* How manuy iterations of the compiler */
int iteration_limit = 2;	/* Limit to iteration_num  */
int ErrorChar;					/* Error Number    */

static char base[MAX_FILE_LEN];		 /* Array for base of input filename  */
static char out[MAX_FILE_LEN];		 /* Array for base of output filename  */
static char err[MAX_FILE_LEN];		 /* Array for base of error filename   */
static char input[MAX_FILE_LEN];		 /* Array for base of pass2 filename  */
static char lis[MAX_FILE_LEN];		 /* Array for base of list filename */


void main(argc, argv)
int argc;
char *argv[];
{
    int argp;
    char *ptr;

    /* Process command line arguments */
    if ((*argv[1] == '-') || (argv[1] == NULL))
	    error(NO_INPUT,"");
    else {
       input_file = fopen(argv[1],"r");	/* Open input file to read */
       if (input_file == NULL)
	       error (CANT_READ, argv[1]);
       else {
	       strcpy(base,"");
	       strncat(base,argv[1],MAX_FILE_LEN-4);
	       ptr = strchr(base,'.');
	       if (ptr)
	         *ptr = '\0';
	     }
	  }
    argp = 2;
    while (argp < argc){
      if (*(argv[argp]) == '-') {
	     ptr = argv[argp] + 1;
	     switch(toupper(*ptr)){

	       case 'O':								 /* Output filename flag  */
	         out_flag = 1;
	         if((argv[argp+1] != NULL) && (*(argv[argp +1]) != '-')){	 /* User defined output filename */
	           strcpy(out,argv[argp+1]);
	           outfile = fopen(argv[argp + 1], "w");		  /* Open user supplied filename */
	           if (outfile == NULL){
		           error(CANT_WRITE, argv[argp =1]);
		           out_flag = 0;
		         }
	           else
		          argp += 2;
	          }
	          else {
	            strcpy(out,base);		/* default to input filename base */
	            strcat(out,".out");			  /* append out to filename  */
	            outfile = fopen(out,"w");
	            if (outfile == NULL){
		             error(CANT_WRITE, out);
		             out_flag = 0;
		          }
	            else
		         argp++;
	           }
	       break;

	       case 'L':								  /* List filename flag  */
	         lis_flag = 1;
	         if((argv[argp+1] != NULL) && (*(argv[argp +1]) != '-')){
	            lisfile = fopen(argv[argp + 1], "w");	  /* User supplied list filename */
	            if (lisfile == NULL){
		            error(CANT_WRITE, argv[argp =1]);
		            lis_flag = 0;
		          }
	             else
		             argp += 2;
	          }
	          else {
	             strcpy(lis,base);					      /* Use input base as list filename base */
	             strcat(lis,".lis");					      /* append .lis on to base   */
	             lisfile = fopen(lis,"w");
	             if (lisfile == NULL){
		             error(CANT_WRITE, lis);
		             lis_flag = 0;
		           }
	              else
		              argp++;
	           }
	       break;

	       case 'Z':									         /* Debug filename flag  */
	         deb_flag = 1;
	         if((argv[argp+1] != NULL) && (*(argv[argp +1]) != '-')){
	            strcpy(input,argv[argp+1]);
	            debfile = fopen(argv[argp + 1], "w");	 /* User supplied debug filename */
	            if (debfile == NULL){
		            error(CANT_WRITE, argv[argp =1]);
		            deb_flag = 0;
		          }
	             else
		            argp += 2;
	          }
	          else {
	             strcpy(input,base);							  /* use input filename base */
	             strcat(input,".sod");						  /* append .sod on to base filename */
	             debfile = fopen(input,"w");
	             if (debfile == NULL){
		             error(CANT_WRITE, input);
		             deb_flag = 0;
		           }
	              else
		              argp++;
	          }
	       break;

	       case 'E':												    /* Error filename flag */
	         err_flag = 1;
	         if((argv[argp+1] != NULL) && (*(argv[argp +1]) != '-')){
	           errfile = fopen(argv[argp + 1], "w");		  /* User defined error file */
	           if (errfile == NULL){
		          error(CANT_WRITE, argv[argp =1]);
		          err_flag = 0;
		         }
	           else
		         argp += 2;
	         }
	         else {
	           strcpy(err,base);									/* Use input filename base */
	           strcat(err,".err");								/* append .err on to base */
	           errfile = fopen(err,"w");
	           if (errfile == NULL){
		           error(CANT_WRITE, err);
		           err_flag = 0;
		        }
	           else
		          argp++;
	         }
	       break;

	       case 'V':												 /* verbose flag  */
	         verbose_flag = 1;
            argp++;
	       break;

          case 'U':												  /* Undefine flag */
            undefine_flag = 1;
            argp++;
          break;

	       default:												 /* Catch all */
	          fprintf(stderr,"WARNING - Unknown switch -%c in command line");
	          argp++;
	       break;
	     }
      }
      else argp++;
    }
    if (!deb_flag){					  /* If debug flag was not set we still */
      strcpy(input,base);		     /* need to generate debug file to be used */
      strcat(input,".sod");		  /* by pass2 of the compiler             */
      debfile = fopen(input,"w");
      if (debfile == NULL){
	     error(CANT_WRITE, input);
	     deb_flag = 0;
       }
     }
	     

/* Setup other modules and initialize variables */
    iteration_num = 1;
    compile_flag = 0;
    lex_init();
    if (verbose_flag){
      printf("SCSI Scripts (TM) Compiler V1.0\n");
      printf("Copyright (C) 1990 NCR\n\n");
    }
    

    while (iteration_num < iteration_limit) {

        if (verbose_flag)
            printf("Starting pass %d.\n",iteration_num);

        yyparse();	  /* parse the input file */


        /* End of this pass, definitely have another pass */

        iteration_num++;
        rewind(input_file);
        line_no = 0;

        /* Can we go to compile pass early? */

        if (all_defined())
            break;          /* Break out of while loop */

    }


    /* Now ready for final pass - output will be generated */

    compile_flag = 1;
    if (verbose_flag)
        printf("Starting compile pass.\n");

    yyparse();
   
    post_compile_printing();	  /* print post compile  items */

    if (verbose_flag)
        printf("%s compiled with %d errors, %d warnings.\n",base,ErrorCount,WarningCount);
    if (ErrorCount) {
	    if (out_flag) {
	         fprintf(stderr,"Deleting %s due to errors in input.\n",out);
            unlink(out);
        }
    }
    fclose(debfile);
    if (!ErrorCount)
    pass2();
    if (!deb_flag)
       unlink(input);
    exit (ErrorCount);
}



void post_compile_printing()
{
    symbol_t *sym;
    patch_t *p;
    int PatchCount;


    /* Now take care of the .lis file */

    if (lis_flag) {
        fprintf(lisfile,"\n");
        print_symbols(lisfile);
        fclose(lisfile);
    }

    /* And finally, the stupid, Ballard-compatible .out file */


	fprintf(debfile,"\n");      /* Blank line */


        /* Take care of relative and external patching */
	fprintf(debfile,"end\n\n");
	for (sym = SymTableStart; sym != NULL; sym = sym->next)
	    if (sym->patch != NULL && (sym->type == IDENT_REL || sym->type == IDENT_EXT)) {
		   fprintf(debfile,"%c_%s\t%08lx\n",
			(sym->type == IDENT_REL) ? 'R' : 'E',sym->name,(int)sym->value);
		   for (p = sym->patch; p != NULL; p = p->next)
		      fprintf(debfile,"\t%08lx\n",(int)p->addr>>2);
		   fprintf(debfile,"\n");
	     }


        /* Next take care of entry points */

        for (sym = SymTableStart; sym != NULL; sym = sym->next)
            if (sym->entry && (sym->type == LABEL_REL || sym->type == LABEL_ABS))
		         fprintf(debfile,"Ent_%-13s\t%08lx\n",sym->name,(int)sym->value);

                                      
        /* Now take care of the 'LABEL PATCHES' */
		if (!undefine_flag){
        sym = ScriptPatches;
        PatchCount = 0;
        if (sym->patch != NULL) {
	       fprintf(debfile,"LABEL PATCHES\n");
	       for (p = sym->patch; p != NULL; p = p->next) {
		       fprintf(debfile,"\t%08lx\n",(int)p->addr>>2);
		       PatchCount++;
	       }
	     }
      }


        /* Now take care of the miscellaneous stuff */
	if (!undefine_flag){
	 fprintf(debfile,"\n");
	 fprintf(debfile,"COUNTS\n");
	 fprintf(debfile,"\t%s\t%08lx\n","INSTRUCTIONS",(int)InstructionCount);
	 fprintf(debfile,"\t%s\t%08lx\n","PATCHES",(int)PatchCount);
	 fprintf(debfile,"\n");
	}

}

char *ident_type(buf,type)
char *buf;
int type;
{
    switch(type) {
        case IDENTIFIER:
            strcpy(buf,"UNDECLARED");
            break;
        case PASSIFIER:
            strcpy(buf,"PASS_LABEL");
            break;
        case ARRAYNAME:
            strcpy(buf,"PROC_LABEL");
            break;
        case IDENT_EXT:
            strcpy(buf,"EXTERNAL");
            break;
        case IDENT_REL:
            strcpy(buf,"RELATIVE");
            break;
        case IDENT_ABS:
            strcpy(buf,"ABSOLUTE");
            break;
        case LABEL_ABS:
            strcpy(buf,"LABEL (ABS)");
            break;
        case LABEL_REL:
            strcpy(buf,"LABEL (REL)");
            break;
        default:
            strcpy(buf,"UNKNOWN");
            break;
    }
    return buf;
}

void print_symbols(file)
FILE *file;
{
    symbol_t *sym;
    char buf[32];

        /* Print out contents of symbol table */

        fprintf(file,"%-35s%-11s%s\n","Symbol Name","Value","Type");
        fprintf(file,"--------------------------------------------------------------\n");

        for (sym = SymTableStart; sym != NULL; sym = sym->next) {
            if (sym->defined)
                fprintf(file,"%-35s%08lX   %s\n",
                        sym->name,(int)sym->value,ident_type(buf,sym->type));
            else 
                fprintf(file,"%-35s????????   %s\n",
                        sym->name,ident_type(buf,sym->type));

        }

        fprintf(file,"\n\n\n");
}


/**********************************************************************/
/* This function takes the output from the debug file and formats it  */
/* C compatible arrays and define statements                          */
/**********************************************************************/
void pass2()
{
   BOOL RetCode;						  /* variable to determine success or not */
   InstStart = FALSE;				  /* Has instructions started   */
   InstEnd = FALSE;					  /* Has instructions ended     */
	RetCode = ReadIntermediate(&input[0]);		  /* Read the debug file */
	if ( RetCode == FALSE )
		exit( FATAL );
	RetCode = BeginConversion();		 /* convert debug file to C compatible output */
	if ( RetCode == FALSE )  {
		fclose( outfile );
	}
	else
		fclose( outfile );
}  /*  void PASS2()  */

/************************************************************************
 *      ReadIntermediate() returns a true or false value corresponding
 *      to its success of opening the intermediate file and getting the
 *      size of it, allocating memory for it, and then reading it into
 *      the allocated memory.  Upon exit of this function, Scanner will
 *      point to the begining of the intermediate file in memory, and 
 *      IntermCode will also point to the base of the file.  These are 
 *      both  global pointers.
 ************************************************************************/

BOOL ReadIntermediate( pass2_input )
 char *pass2_input;
{
  int  IntermSize;
  BOOL  RetCode;

	IntermSize = GetFileSize( pass2_input );
	OpenFile( pass2_input );

        if ( ( IntermCode = (char *)malloc( IntermSize + 4) ) == NULL )
        {
                if (verbose_flag)
                  printf("Could not get memory required: %lx bytes.\n",
                        IntermSize);
                
                 exit( FATAL );
        }
        else if (verbose_flag)
                printf( "Requires 0x%lx bytes of memory.\n",IntermSize );
        Scanner = IntermCode;

        while( !feof( IntermFile ) )
        {
                *Scanner = (char )fgetc( IntermFile );
                Scanner++;
        }
        fclose( IntermFile );
        Scanner = IntermCode;
        RetCode = TRUE;
        return RetCode;
}  /*  BOOL ReadIntermediate( pass2_input )  */


/**********************************************************************
 *      GetFileSize() accepts a pointer to a file name.  This file
 *      is then opened and seeked to the end to get the lenght of the
 *      file in bytes.  This value is returned in int FileSize.
 **********************************************************************/

int GetFileSize( file )
 char *file;
{
 int FileSize;
 FILE *FilePtr;
        if ( ( FilePtr = fopen( file, "r+t" ) ) == NULL )
        {		 if (verbose_flag)
                printf( "Could not open input file %s\n", file );
                exit( FATAL );
        }
        if ( fseek( FilePtr, 0L, SEEK_END ) != 0 )
        {
                if (verbose_flag)
                printf( "Could not seek in file %s\n", file );
                exit( FATAL );
        }
        else
                FileSize = (unsigned int)ftell( FilePtr );
        fclose( FilePtr );
        return FileSize;

}  /*  int GetFileSize( file )  */


/***********************************************************************
 *      isletter() acceptsa a single character, if it is a letter or '_'
 *      then TRUE is returned, otherwise FALSE is returned.
 ***********************************************************************/

BOOL isletter( Data )
 char Data;
{
 BOOL RetCode;
        
        if ( ( Data != '_' ) && ( ( toupper( Data ) < 'A' ) ||
                ( toupper( Data ) > 'Z' ) ) && ( Data != ' ' ) )
                RetCode = FALSE;
        else
                RetCode = TRUE;

        return RetCode;

}  /*  BOOL isletter( Data )  */


/*******************************************************************
 *      OpenFile() opens a file whose name it is passed for reading
 *      in text mode.  If it is not successful opening the file 
 *      the program is exited.  This is a bit brutal, but what the
 *      hell?
 *******************************************************************/

void OpenFile( file )
 char *file;
{
        if ( ( IntermFile = fopen( file, "r+t" ) ) == NULL )
        {
                if (verbose_flag)
                printf( "Could not re-open %s.\n", file);
                exit( FATAL );
        }
        return;

}  /*  void OpenFile( file )  */


/*******************************************************************
 *      BeginConversion() is the main compilation loop, yet it is 
 *      very simple in what it does.  Until the EOF char is reached
 *      in the intermediate file, it reads in a token and switches
 *      to the appropriate handling routine.  There are only two
 *      handling routines, one for variables and one for identifiers.
 *******************************************************************/

BOOL BeginConversion()
{
 BOOL   RetCode;
        RetCode = GetToken();

        while ( RetCode == TRUE )
        {
                switch  ( TokenType )
                {
                        case    VARIABLE:
                                HandleVar( &TokenName[0] );
                                break;
                        case    IDENTIFY:
                                HandleIdent();
                                break;

			case    PASSIFY:
                                HandlePass();
                                break;

                        case    END:
                                InstEnd = TRUE; 
                                break;

                        case    SS_EOF:
                                goto BeginConversionDone;

                        default:
                                break;

                }
                RetCode = GetToken();
        }
BeginConversionDone:

        return TRUE;
}  /*  BOOL BeginConversion()  */


/********************************************************************
 *      If the token read in by BeginConversion() was of TokenType
 *      IDENTIFIER then this function is called.  Depending on what
 *      the Identifier is, the appropriate routine is called.
 ********************************************************************/

void HandleIdent()
{

        if ( CompareString( &TokenName[0], "INSTRUCTIONS" ) == 0 )
                DoInstructions();
        else if ( CompareString( &TokenName[0], "LABEL PATCHES" ) == 0 )
                DoPatches();
        else if ( CompareString( &TokenName[0], "EXTERNAL" ) == 0 )
                HandleExternal();
        else if ( CompareString( &TokenName[0], "COUNTS" ) == 0 )
                HandleCounts();

        return;
}  /*  void  HandleIdent()  */

/*******************************************************************
 *      This function prints out the pass strings that occur before
 *      the instruction set is encountered
 ******************************************************************/
 HandlePass() {
 fprintf(outfile,"%s\n",&TokenName[0]);
 return;
 } /* HandlePass()   */

/*******************************************************************
 *      This should be done only once, and at the begining of the
 *      conversion.  First do a typedef and then generate the array
 *      INSTRUCTIONS and fill it with all of the instructions.
 *******************************************************************/
void DoInstructions()
{
        if (InstStart == FALSE)
        /* fprintf(outfile,"typedef unsigned int ULONG;\n\n" ); */
        GetToken();
        InstArray();
        return;

}  /*  void DoInstructions()  */

/*******************************************************************/
/*      DoPatches()  simply generates an array called LABELPATCHES */
/*      with all the patch values.											 */
/*******************************************************************/ 
void DoPatches()
{
        GenArray( "LABELPATCHES" );

        return;

}  /*  void DoPatches()  */


/**********************************************************************/
/* Get1Char() simply returns 1 character from the input stream. Prior to
 * actually moving the pointer in the input stream, Get1Char() checks
 * the push back stack pointer. If this is a non-negative value it means
 * someone pushed a character back into the stream and it should be read
 * first.
***********************************************************************/

char    Get1Char()
{
        char    RetChar;
        RetChar = *Scanner++;
        return RetChar;
}

/**********************************************************************/
/* GetTokenType() accepts a pointer to a NULL terminated string. It
 * then tries to figure out what type of token this string represents.
************************************************************************/

int     GetTokenType(String)
 char   *String;
{
        int     RetVal;
        if (!InstStart){
        if (CompareString(String, "INSTRUCTIONS") == 0)
         RetVal = IDENTIFY;
        else RetVal = PASSIFY; }
        else if(CompareString(String, "end") == 0)
                RetVal = END;
        else if(*String == '\n')
                RetVal = EOLP;
        else if(*String == '\0')
                RetVal = SS_EOF;
        else if((strchr(String, '_') != NULL) && (InstEnd))
                RetVal = VARIABLE;
        else if(( CompareString(String, "INSTRUCTIONS") == 0 ) ||
                ( CompareString(String, "LABEL PATCHES") == 0 ) ||
                ( CompareString(String, "COUNTS") == 0 ) ||
                ( CompareString(String, "EXTERNAL") == 0 ) ||
                ( CompareString(String, "PATCHES") == 0 ))
                        RetVal = IDENTIFY;
        else
                RetVal = NUMBER;
        return(RetVal);
}

/**************************************************************************/
/*      This is a debug routine that will display the TokenType
****************************************************************************/
void DisplayRetVal( code )
 int code;
{
        switch ( code )
        {
                case    EOLP:
                        printf( "EOL\n" );
                        break;
                case    SS_EOF:
                        printf( "SS_EOF\n" );
                        break;
                case    VARIABLE:
                        printf( "VARIABLE\n" );
                        break;
                case    IDENTIFY:
                        printf( "IDENTIFIER\n" );
                        break;
                case    NUMBER:
                        printf( "NUMBER\n" );
        }
        return;
}
/***********************************************************************/
/* GetToken() reads the input stream 1 character at a time until it sees
 * something which is not a letter or digit. It then attempts to determining
 * the type of identifier.
 ************************************************************************/

BOOL    GetToken()
{
        BOOL    RetCode = TRUE;
        char    Look;
        int     index = 0;
        while(1){
                Line = FALSE;
                Look = Get1Char();
                if ( (isletter( Look )) || (isdigit( Look )) || (strchr(token_chars, Look) != NULL))
                        TokenName[index++] = Look;
                else
                {
                        if  ( Look == '\t' ) 
                        {
                                if ( index != 0 )
                                        break;
                        }
                        else if (Look == '\n')
                        {
                               Line = TRUE;
                               if( index != 0)
                                    break;
                        } 
                        else
                        break;
                }
        }
        TokenName[index] = '\0';
        TokenType = GetTokenType(&TokenName[0]);
        if ( TokenType == SS_EOF )
                RetCode = FALSE;

        return RetCode;
}

/**************************************************************************/
/*      SysError is a debug and Error Handling routine.  It merely
 *      displays a value that it has been sent when an error
 *      occurs.
***************************************************************************/
void SysError( ErrCode )
 int ErrCode;
{
        if (verbose_flag)
        printf("SysError %x\n", ErrCode );
        return;

}  /*  void SysError( ErrCode )  */





/**********************************************************************
 **********************************************************************
 End of scanner section of code
 **********************************************************************
 **********************************************************************/

/**********************************************************************
 **********************************************************************
 The functions in the following section of code actually generate output
 to the disk.

 NOTE: This is not the ONLY place which generates the output, just the
 main place.
 **********************************************************************
 **********************************************************************/

/* GenDefine() accepts 2 pointers to NULL terminated string. 1 is the
 * name of the constant to be defined, and the other is the numeric
 * string. It then pre-pends "#define" and writes it out to disk.
***********************************************************************/

BOOL    GenDefine(Name, Value)
 char   *Name, *Value;
{
        fprintf(outfile, "#define\t%s\t0x%s\n", Name, Value);
        return(TRUE);
}
/************************************************************************/
/* GenArray() accepts a pointer to a NULL terminated string which represents
 * the name of the array to be read. It then reads in tokens, and as long
 * as they of type NUMBER, it writes them out. Before it writes anything
 * out it first checks to see that the next token actually is a number.
 * If not, then no output is generated.
**************************************************************************/

BOOL    GenArray(Name)
 char   *Name;
{
        BOOL    RetCode;
        char   TempString[32];

        RetCode = GetToken();
        strcpy(&TempString[0],&TokenName[0]);
        if(( RetCode == FALSE ) || ( TokenType != NUMBER ))
                goto GenArrayDone;

        fprintf(outfile, "unsigned int\t%s[] = {\n", Name );

        while( TokenType == NUMBER ){
            RetCode = GetToken();
            if (TokenType == NUMBER){
               fprintf(outfile,"\t0x%s,\n", &TempString[0]);
               strcpy(&TempString[0],&TokenName[0]);
             }
             else 
               fprintf(outfile, "\t0x%s", &TempString[0]);
        }
        fprintf(outfile,"\n};\n\n");

GenArrayDone:

        PushBackToken();
        return(TRUE);
}
/*************************************************************************/
/* Instruction array processes the instructions from the debug file 
* output of the compiler, noting whether it is a string (PASS)
* parameter or an unsigned integer
***************************************************************************/
BOOL    InstArray()
{
        BOOL    RetCode;
        char    TempString[32];
        BOOL    pass;
        BOOL    line_flag;
	BOOL	newline = TRUE;
        int     i;
	int	offset = 0;

		  line_flag = FALSE;
        i = 0;
        while (TokenName[i] != '\0') {
        if (strchr(pass_chars, TokenName[i]) == NULL)
          InstStart = TRUE;
          i++;
        }
        if (InstStart)
          fprintf(outfile, "unsigned int\t%s[] = {\n", &TokenName[0]);
        else 
	{
          PushBackToken();
          Line = FALSE;
          InstStart = TRUE;
          fprintf(outfile, "unsigned int\tSCRIPT[] = {\n");
        }
        RetCode = GetToken();
        strcpy(&TempString[0],&TokenName[0]);

        if ((RetCode == FALSE) || (TokenType != NUMBER)) 
	{
	        fprintf(outfile,"};\n\n");
	        goto InstArrayDone;
        }
        while( TokenType == NUMBER ) {
	  RetCode = GetToken();
           i = 0;
           pass = FALSE;
	   if(newline)
		   fprintf(outfile, "/* 0x%04x */", offset);
	   newline = FALSE;
           fprintf(outfile, "\t");
           while(TempString[i] != '\0') 
	   {
             if (strchr(pass_chars, TempString[i]) == NULL)
		     pass = TRUE;
             i++;
           }
	   if (pass)
	   {
              if (TokenType == NUMBER) 
		    fprintf(outfile,"%s,", &TempString[0]);
		else fprintf(outfile,"%s", &TempString[0]);
           }
           else 
	   {
              if (TokenType == NUMBER)
                  fprintf(outfile,"0x%s,",&TempString[0]);
              else  fprintf(outfile,"0x%s",&TempString[0]);
	   }
	   offset++;
	   if (line_flag) 
	   {
              fprintf(outfile, "\n");
	      newline = TRUE;
	   }
           strcpy(&TempString[0],&TokenName[0]);
           line_flag = Line;
	}
	  fprintf(outfile,"\n};\n\n");

	fprintf(outfile,"unsigned int	SCRIPT_SIZE = %d;\n\n",offset);
	  InstStart = TRUE;

InstArrayDone:
     PushBackToken();
     return(TRUE);
}
/***********************************************************************/
/*      When an entire token has been assimilated and is then found
 *      not to be needed, this function call will reset Scanner to
 *      point to the begining of the Token again.
************************************************************************/
void PushBackToken()
{
 int TokenLength;

        TokenLength = strlen( &TokenName[0] ) + 1;

        Scanner -= TokenLength;
        return;

}  /*  void PushBackToken()  */


/**********************************************************************
 **********************************************************************
  End of output section
 **********************************************************************
 **********************************************************************/

/**********************************************************************
 **********************************************************************
 The functions that follow handle the translation of the various portions
 of the intermediate file.
 **********************************************************************
 **********************************************************************/

/* HandleVar() is called whenever we detect the presence of a token of
 * type VARIABLE in the input stream. It generates the define for the
 * variable, and then the array of its usage locations.
*************************************************************************/

BOOL    HandleVar(VarName)
 char   *VarName;
{
        char    Variable[35];
        BOOL    RetCode;

        strcpy( Variable, VarName );

        RetCode = GetToken();
        if(RetCode == FALSE){
                SysError(E_EXP_NUM);
                goto HandleVarDone;
        }

        GenDefine(Variable, &TokenName[0]);

        strcat(&Variable[0], "_Used");
        GenArray(&Variable[0]);
        
HandleVarDone:


        return(RetCode);

}
/**************************************************************************/
/* HandleExternal() is called whenever we detect the presence of a token of
 * type EXTERNAL in the input stream. It generates the define for the
 * variable, and then the array of its usage locations.
***************************************************************************/

BOOL    HandleExternal()
{
        char    Variable[35];
        BOOL    RetCode;

        RetCode = GetToken();
        if ( RetCode == FALSE )
        {
                goto HandleExtDone;
        }
        strcpy( Variable, &TokenName[0] );

        RetCode = GetToken();
        if(RetCode == FALSE){
                SysError(E_EXP_NUM);
                goto HandleExtDone;
        }

        strcat(&Variable[0], "_Used");
        GenArray(&Variable[0]);
        
HandleExtDone:


        return(RetCode);

}
/***********************************************************************/
/* HandleCounts() is called when we detect the word COUNTS in the input
 * stream. It then reads the next 4 tokens and writes out the information
 * to the output file.
************************************************************************/

BOOL    HandleCounts()
{
        BOOL    RetCode;
        int     WhichDone = 0;
        

        while(WhichDone < 2){
                RetCode = GetToken();
                if(RetCode == FALSE)
                        goto HandleCountsDone;
                if(TokenType != IDENTIFY){
                        SysError(E_EXP_IDENT);
                        RetCode = FALSE;
                        goto HandleCountsDone;
                }
                fprintf(outfile, "unsigned int\t%s\t", &TokenName[0]);
                if ( strlen( &TokenName[0] ) < 8 )
                        fprintf(outfile, "\t" );
                RetCode = GetToken();
                if(TokenType != NUMBER)
                        goto HandleCountsDone;
                fprintf(outfile, "= 0x%s;\n", &TokenName[0]);
                ++WhichDone;
        }
                        
HandleCountsDone:


        return(RetCode);
}
/************************************************************************/
/*      ishexdigit() accepts a single character and returns true if 
 *      the character is a valid hex character.
**************************************************************************/
BOOL ishexdigit( Data )
 char Data;
{
 BOOL RetCode = TRUE;
        if ( !isdigit( Data ) )
                if ( toupper( Data ) < 'A' || toupper( Data ) > 'F' )
                        RetCode = FALSE;
        return RetCode;

}  /*  BOOL ishexdigit( Data )  */

/***********************************************************************/
/*      isdigit() accepts a single character and returns true if it is
 *      a valid decimal digit.
************************************************************************/
BOOL isdigit( Data )
 char Data;
{
 BOOL RetCode = TRUE;
        if ( ( Data < '0' ) || ( Data > '9' ) )
                RetCode = FALSE;
        return RetCode;

}  /*  BOOL isdigit( Data )  */



/*************************************************************************
 *      CompareString() is a function that has been thrown in to
 *      replace the previous use of strcmpi(), a Turbo C function.
 *      This will compare both null terminated strings case in-sensitive.
 *      Unlike strcmpi(), which will return the diference in the numeric
 *      value of the two strings, this will return a Pass/Fail.  With
 *      strcmpi() a zero is returned if the two strings match, since
 *      their numeric quantity is equal.  This is why in this case a
 *      zero is returned for true.
****************************************************************************/

BOOL CompareString( str1, str2 )
  char *str1, *str2;
{
  BOOL  RetCode = 0;
  int   len, x;

        if ( ( len = strlen( str1 ) ) != strlen( str2 ) )
        {
                RetCode = 1;
                goto CompareStringDone;
        }
        for( x=0; x<=len; )
        {
                if ( Upper( str1[x] ) != Upper( str2[x] ) )
                {
                        RetCode = 1;
                        break;
                }
                x++;
        }
CompareStringDone:
        return RetCode;

}  /*  BOOL CompareString( str1, str2 )  */

/**********************************************************************/
char Upper( str )
  char  str;
{
  char c;
        if ( str >= 'a' && str <= 'z' )
                c = toupper( str );
        else
                c = str;

        return c;

}  /*  char Upper( str )  */    

