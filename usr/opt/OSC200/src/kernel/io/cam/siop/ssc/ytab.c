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
/* C:\BIN\YACC.EXE -dv scsi.y */
#line 1 "scsi.y"
 
#include "scsi.h"

int	Origin;
int    first = 0;  /*  variable to determine idf instructions have started */
BOOL   compile_flag = 0; /* is it the compile pass   */
UINT32  pc = 0;		/* program counter */
int  ident_label_type;
static symbol_t ScriptPatchList;
symbol_t *ScriptPatches = &ScriptPatchList;	/* symbol patch pointer */
int InstructionCount;	/* instruction number */
#define MAX_FIELDS 4	/* maximum number of opcode fields */
typedef struct opcode_s {
    unsigned int    type;	          /* type of opcode */
    unsigned int    func;		  /* function of opcode */
    unsigned int    phase;		  /* SCSI phase */
    unsigned int    indirect;		  /* indirect bit */
    unsigned int    table;		  /* table bit */
    unsigned int    relative;		  /* relative bit */
    unsigned int    atn;		  /* attention bit */
    unsigned int    ack;		  /* acknowledge bit */
    unsigned int    target;		  /* target bit */
    unsigned int    comp_phase;		  /* compare phase */
    unsigned int    comp_data;		  /* compare data */
    unsigned int    mask;		  /* mask data */
    unsigned int    jump_false;		  /* jump if compare is false */
    unsigned int    wait;		  /* wait for phase change */
    unsigned int    reg;		  /* register number */
    unsigned int    reg_op;		  /* register opcode */
    unsigned int    f_no;		  /* field number    */
    char            *pass_word0;	  /* Pass word field 0 */
    char            *pass_word1;	  /* pass word filed 1 */
    char            *pass_word2;	  /* pass word ield 2 */
    expr_t          fields[MAX_FIELDS];	  /* array of related expressions */
    char	    ispc[MAX_FIELDS];
}   opcode_t;

opcode_t    Opcode;

expr_t data_list[128];			  /* expression array */
int    d_ptr = 0;			  /* pointer into data list */
UINT32 dup = 0;

#define REG_OR   1			  /* register or operation */
#define REG_AND  2			  /* register and operation */
#define REG_PLUS 3			  /* register addition operation */
typedef union {
    INT32       i;
    symbol_pt   s;
    expr_t      e;
} YYSTYPE;
#define MOVE	257
#define JUMP	258
#define NOP	259
#define CALL	260
#define RETURN	261
#define INT	262
#define SELECT	263
#define RESELECT	264
#define DISCONNECT	265
#define SET	266
#define CLEAR	267
#define PTR	268
#define WITH	269
#define WHEN	270
#define IF	271
#define AND	272
#define NOT	273
#define OR	274
#define MASK	275
#define WAIT	276
#define SHIFT_LEFT	277
#define SHIFT_RIGHT	278
#define EOL	279
#define ABSOLUTE	280
#define RELATIVE	281
#define PC	282
#define ORIGIN	283
#define MEMORY	284
#define REG	285
#define TO	286
#define REL	287
#define FROM	288
#define DUP	289
#define ALIGN	290
#define GOOD_PARITY	291
#define BAD_PARITY	292
#define EXTERN	293
#define EXTERNAL	294
#define ENTRY	295
#define PHASE	296
#define ACK	297
#define ATN	298
#define TARGET	299
#define INTEGER	300
#define DATA	301
#define REGISTER	302
#define IDENTIFIER	303
#define PASS	304
#define PROC	305
#define ARRAYNAME	306
#define PASSIFIER	307
#define IDENT_ABS	308
#define IDENT_EXT	309
#define IDENT_REL	310
#define LABEL_ABS	311
#define LABEL_REL	312
#define UNARY	313
extern int yychar, yyerrflag;
extern YYSTYPE yyval, yylval;
#line 874
void handle_opcode()
{
    int delta_pc; 	   /* program counter increment */
    int nostring; 	   /* indicates whether field 3 contains  a string*/
    int shift;		  /* used to shift SCSI ids to proper bit in opcode */
    int i; 		  /* loop counter */
    int p1, p2; 	   /* pass flags for fields 1 and 2 */
    int col1, col2;	   /* used for proper printing of list file */
   

    char buf[20];	   /* buffer for strings in 3rd field */
    union dval{		    /* union for opcode word instructions */
         UINT32    dval;
         char      *pword;
         }dword0, dword1, dword2;

   p1 = p2 = col1 = col2 = i = nostring = 0;
   shift = 1;
   if (!compile_flag) {
        pc += 8;
        if (Opcode.type == 3)
            pc += 4;
        return;
    }

    /* Build instruction (note that impossible/illegal opcodes are
                          already weeded out) */
    /* Common stuff */
    delta_pc = 8;

    dword0.dval  =   (UINT32)Opcode.type  << 30
             | (UINT32)Opcode.func  << 27
             | (UINT32)Opcode.phase << 24;

    switch(Opcode.type) {
        case 0:            /* Block move instruction */
            dword0.dval |= (UINT32)Opcode.indirect << 29;
            dword0.dval |= (UINT32)Opcode.table << 28;
	    dword0.dval |= (Opcode.fields[0].value & 0xffffff);
            if (Opcode.pass_word1 != NULL) {
                dword1.pword = Opcode.pass_word1;
                p1 = 1; }
            else if (Opcode.table) {
                dword1.dval = Opcode.fields[0].value;    /* Table address */
                patch(&Opcode.fields[0],pc+4,4,0);
            }
            else  {
                dword0.dval |= Opcode.fields[0].value;   /* Count */
                patch(&Opcode.fields[0],pc+1,3,0);
                dword1.dval = Opcode.fields[1].value;    /* data address */
                patch(&Opcode.fields[1],pc+4,4,0);
            }
            break;


        case 1:             /* I/O instructions */
            /* Common stuff */
            dword0.dval |=   (UINT32)Opcode.table << 25
                        | (UINT32)Opcode.relative << 26;

            switch(Opcode.func) {
                case 0: /* {RE}SELECT */
                    dword0.dval   |= (UINT32)Opcode.atn << 24;
                    dword0.dval   |= (UINT32)Opcode.target << 9;
                    if (Opcode.table) {
                        dword0.dval |= Opcode.fields[0].value;   /* Table addr */
                        patch(&Opcode.fields[0],pc+1,3,0);
                    }
                    else {
                        i = Opcode.fields[0].value;
                        while (i!= 0){
                          shift *= 2;
                          i--;
                        }
                        Opcode.fields[0].value = shift;
                        dword0.dval |= Opcode.fields[0].value << 16;  /* ID */
                        patch(&Opcode.fields[0],pc+1,1,0);
                    }
                    if (Opcode.pass_word1 != NULL) {
                       dword1.pword = Opcode.pass_word1; 
                       p1 = 1; }
                    else {dword1.dval = Opcode.fields[1].value;    /* Branch addr */
                    patch(&Opcode.fields[1],pc+4,4,0); }
                    break;
                case 2: /* WAIT {RE}SELECT */
                       if (Opcode.pass_word1 != NULL) {
                       dword1.pword = Opcode.pass_word1;
                          p1 = 1; }
                       else { dword1.dval = Opcode.fields[0].value;    /* Branch addr */
                    patch(&Opcode.fields[0],pc+4,4,0); }
                    break;
                case 1: /* DISCONNECT */
                case 3: /* SET bits */
                case 4: /* CLEAR bits */
                    dword0.dval |= (UINT32)Opcode.atn    << 3;
                    dword0.dval |= (UINT32)Opcode.ack    << 6;
                    dword0.dval |= (UINT32)Opcode.target << 9;
                    dword1.dval = Opcode.fields[0].value;
                    break;
                case 5: /* Reg write */
                case 6: /* Reg read */
                case 7: /* Reg mod */
                    dword0.dval |= (UINT32)Opcode.reg << 16;
                    dword0.dval |= (UINT32)Opcode.reg_op << 25;
                    if (Opcode.f_no) {
                        dword0.dval |= Opcode.fields[0].value << 8;
                        patch(&Opcode.fields[0],pc+4,4,0);
                    }
                    dword1.dval = 0;        
                    break;
            }
            break;


        case 2:             /* Transfer instructions */

            dword0.dval |=   (UINT32)Opcode.relative      << 23
                      | (UINT32)(!Opcode.jump_false) << 19
                      | (UINT32)Opcode.atn           << 17
                      | (UINT32)Opcode.comp_phase    << 17
                      | (UINT32)Opcode.wait          << 16;

            if (Opcode.pass_word1 != NULL) {
                   dword1.pword = Opcode.pass_word1;
                   p1 = 1; 
             }
             else if (Opcode.func != 2) { /* Everything but RETURN */
                   dword1.dval = Opcode.fields[0].value;
                   patch(&Opcode.fields[0],pc+4,4,0);
             }
	     else dword1.dval = 0;   /* Return Instruction */
             if (Opcode.comp_data) {
                 dword0.dval |=   (UINT32)1 << 18
                             | Opcode.fields[Opcode.comp_data - 1].value;
                 patch(&Opcode.fields[Opcode.comp_data-1],pc+3,1,0);
              }
              if (Opcode.mask) {
                 dword0.dval |=   Opcode.fields[Opcode.mask - 1].value << 8;
                 patch(&Opcode.fields[Opcode.mask-1],pc+2,1,0);
               }
               break;


        case 3:	/* memory move instructions */
            delta_pc = 12;
            dword0.dval |= Opcode.fields[0].value;

            patch(&Opcode.fields[0],pc+1,3,0);

            if (Opcode.pass_word1 != NULL)
	    {
               dword1.pword = (char*)(Opcode.pass_word1); 
               p1 = 1;
            }
            else 
	    {
                dword1.dval =  Opcode.fields[1].value;
                patch(&Opcode.fields[1],pc+4,4,Opcode.ispc[1]);
            }

            if (Opcode.pass_word2 != NULL) 
	    {
                 dword2.pword = (char*)(Opcode.pass_word2); 
                 p2 = 1; 
            }
            else 
	    {
                  dword2.dval = Opcode.fields[2].value;
                  patch(&Opcode.fields[2],pc+8,4,Opcode.ispc[2]);
            }
            break;
    }

    if ((delta_pc == 12) && (p2 == 0)) 
        sprintf(buf,"%08lX",(int)dword2.dval);	 /* 2nd word is integer */
    else {
        sprintf(buf,"\0");
        nostring = 1;
     }

    if (lis_flag){
        fprintf(lisfile,"%4d   %08lX:  %08lX",
                line_no,(int)pc,(int)dword0.dval);
        if (p1){
          col1 = strlen(dword1.pword);
          fprintf(lisfile, "  %8s",dword1.pword);
        }
        else fprintf(lisfile, "  %08lX", (int)dword1.dval);
        if (p2){
          col2 = strlen(dword2.pword);
          if ((col1 < 10) && (col2 < 10) && ((col1 + col2) < 18))
             fprintf(lisfile, "  %8s  %s", dword2.pword, lex_buffer);
          else fprintf(lisfile, "  %8s\n %-19s",dword2.pword, lex_buffer);
         }
         else {
           if (col1 < 10)
              fprintf(lisfile, "  %8s  %s", buf, lex_buffer);
            else fprintf(lisfile, "  %8s\n %-19s", buf, lex_buffer);
          }
     }
     if (!first) {		    /* instructions not defined yet */
         fprintf(debfile, "INSTRUCTIONS\n");
         first++;
     }
     fprintf(debfile,"\t%08lX", (int)dword0.dval);
     if (p1) 
        fprintf(debfile,"\t%s", dword1.pword);
     else fprintf(debfile, "\t%08lX", dword1.dval); 
     if (p2) 
        fprintf(debfile,"\t%s\n", dword2.pword);
     else if (nostring == 0)
        fprintf(debfile,"\t%8s\n",buf);
     else fprintf(debfile,"\n");
     pc += delta_pc;
     InstructionCount++;
}


void handle_ext(sym)	 /* handle external labels */
symbol_t *sym;
{ 
  if (compile_flag){
       if (sym->defined)
         fprintf(debfile,"EXTERN  %s\n", sym->name);
       if (lis_flag) fprintf(lisfile,"%4d  %s %s %s",line_no,sym->name,"",lex_buffer); 
}}
         
void handle_pass(sym)	   /* handle pass labels */
symbol_t *sym;
{
 if (compile_flag) {
      if (sym->defined)
          fprintf(debfile, "%s\n", sym->name);
          if (lis_flag) 
           fprintf(lisfile, "%4d %s %s %s", line_no, sym->name,"",lex_buffer);
 }
}      

void handle_ident()	       /* handle identifiers */
{
    if (compile_flag) {
        if (lis_flag)
            fprintf(lisfile,"%4d%44s%s",line_no,"",lex_buffer);
    }
}


void handle_label(sym)		/* handle label identifiers */
symbol_t *sym;
{
 int size_arr;
    if (compile_flag) {
        if (lis_flag)
            fprintf(lisfile,"%4d   %08lX:%32s%s",line_no,(int)pc,"",lex_buffer);
        size_arr = 0;
        while ((lex_buffer[size_arr] != '\n') && (lex_buffer[size_arr] != ':'))
            size_arr++;
        lex_buffer[size_arr--] = '\0';
        if ((sym->entry == 0) && (sym->type == ARRAYNAME)){
            size_arr = 0;
            while ((lex_buffer[4 + size_arr] == ' ') || (lex_buffer[4 + size_arr] == '\t'))
            size_arr++;
            if (first == 0) 
               fprintf(debfile, "INSTRUCTIONS\t%s\n",&(lex_buffer[4 + size_arr]));
            else 
               fprintf(debfile, "\nINSTRUCTIONS\t%s\n",&(lex_buffer[4 + size_arr]));
            first++;
        }
    }
}


void handle_orig(a)
UINT32 a;
{
    if (compile_flag) {
        if (lis_flag)
            fprintf(lisfile,"%4d   %08lX:%32s%s",line_no,(int)pc,"",lex_buffer);
    }
    pc = a;
}

void handle_blank()
{
    if (compile_flag) {
        if (lis_flag)
            fprintf(lisfile,"%4d%44s%s",line_no,"",lex_buffer);
    }
}

void handle_align()
{
    register int count;

    count = 0;
    while(pc & 0xff)
    {
	pc += 4;
	if(compile_flag)
	{
	    fprintf(debfile,"0x00000000");
	    count++;
	    if(pc & 0xff)
	    {
		if(count == 3)
		{
		    fprintf(debfile,"\n");
		    count = 0;
		}
		else
		    fprintf(debfile,"\t");
	    }
	}
    }
    if(compile_flag)
	fprintf(debfile,"\n");
}

void handle_parity()
{
    if (compile_flag) {
        if (lis_flag)
            fprintf(lisfile,"%4d%44s%s",line_no,"",lex_buffer);
    }
}

void handle_data(size)
int size;
{
    int i, j;
    UINT32 mypc;
    char buf[30];
    int buflen;
    int next;


    if (compile_flag) {

        /* Generate patch information for any externs, relatives, labels, etc. */
        mypc = pc;
        for (i=dup; i; i--)
            for (j=0; j<d_ptr; j++) {
                patch(&data_list[j],mypc,size,0);
                mypc += size;
            }

        strcpy(buf,"");
        buflen = 27;
        if (dup > 1) 
	{
	    make_dup(size,dup,(int)data_list[0].value);
	    next = 1;
        }
        else 
		next = 0;
	make_data(size,next,d_ptr);

        if (lis_flag) {
            if (dup > 1) {
                sprintf(buf,"%lX*(",dup);
                buflen = 27 - strlen(buf);
            }
            else {
                strcpy(buf,"");
                buflen = 27;
            }
            next = build_data(buf,buflen,size,0,d_ptr,(dup > 1));
            fprintf(lisfile,"%4d   %08lX:  %-28s%c %s",line_no,(int)pc,buf,
                    (next == d_ptr) ? ' ' : '+',lex_buffer);
            while (next < d_ptr) {
                strcpy(buf,"");
                next = build_data(buf,27,size,next,d_ptr,(dup > 1));
                fprintf(lisfile,"%18s%-28s%c\n","",buf,(next == d_ptr) ? ' ' : '+');
            }
        }
    }
    pc += ((size * dup * d_ptr + 3)/4) * 4;
}

make_dup(size,count,value)
register int size, count, value;
{
    register int i;
    char buf[32];
    
    switch(size)
    {
	case 1:
	    sprintf(buf,"%02x",(int)data_list[0].value & 0xff);
	    break;
	case 2:
	    sprintf(buf,"%04x",(int)data_list[0].value & 0xffff);
	    break;
	case 4:
	    sprintf(buf,"%08x",(int)data_list[0].value & 0xffffffff);
	    break;
    }
    i = 0;
    while(count)
    {
	switch(size)
	{
	    case 1:
		switch(count)
		{
		    case 4:
		    default:
			fprintf(debfile,"%s%s%s%s",buf,buf,buf,buf);
			count -= 4;
			break;

		    case 3:
			fprintf(debfile,"00%s%s%s",buf,buf,buf);
			count -= 3;
			break;

		    case 2:
			fprintf(debfile,"0000%s%s",buf,buf);
			count -= 2;
			break;

		    case 1:
			fprintf(debfile,"000000%s",buf);
			count--;
			break;
		}
		break;

	    case 2:
	       switch(count)
	       {
		   case 2:
		   default:
			fprintf(debfile,"%s%s",buf,buf);
			count -= 2;
			break;

		    case 1:
			fprintf(debfile,"0000%s",buf);
			break;
		}
		break;

	    case 4:
		fprintf(debfile,buf);
		count--;
		break;
	}
	i++;
	if(count)
	{
	    if(i == 3)
	    {
		i = 0;
		fprintf(debfile,"\n");
	    }
	    else
		fprintf(debfile,"\t");
	}
    }
    fprintf(debfile,"\n");
}

make_data(size,start,finish)
register int size, start, finish;
{
    register int i;
    register int length;
    register int count = 0;

    i = 0;
    length = 0;
    while(start < finish)
    {
	switch(size)
	{
	    case 4:
		i = data_list[start++].value & 0xffffffff;
		length = 4;
		break;

	    case 2:
		if(length)
		    i |=  (data_list[start++].value & 0xffff) << 16;
		else
		    i =  data_list[start++].value & 0xffff;
		length += 2;
		break;

	    case 1:
		switch(length)
		{
		    case 0:
			i = data_list[start++].value & 0xff;
			break;
		    case 1:
			i |= (data_list[start++].value & 0xff) << 8;
			break;
		    case 2:
			i |= (data_list[start++].value & 0xff) << 16;
			break;
		    case 3:
			i |= (data_list[start++].value & 0xff) << 24;
			break;
		}
		length++;
		break;
	}
	if(length == 4)
	{
	    fprintf(debfile,"%08x",i);
	    length = 0;
	    if(start < finish)
	    {
	        count++;
		if(count == 3)
		{
		    fprintf(debfile,"\n");
		    count = 0;
		}
		else
		    fprintf(debfile,"\t");
	    }
	    else
		fprintf(debfile,"\n");
	}
    }
    if(length)
	fprintf(debfile,"%08x\n",i);
}

				

int build_data(buf,buflen,size,start,max,dup_flag)
char *buf;
int buflen, size, start, max, dup_flag;
{
    char mybuf[10];

    while (buflen >= size * 2) {
        switch(size) {
            case 1:
                sprintf(mybuf,"%02lX",(int)data_list[start++].value & 0xff);
                break;
            case 2:
                sprintf(mybuf,"%04lX",(int)data_list[start++].value & 0xffff);
                break;
            case 4:
                sprintf(mybuf,"%08lX",(int)data_list[start++].value & 0xffffffff);
                break;
        }
        strcat(buf,mybuf);
        buflen -= size * 2;
        if (start == max) {
            if (dup_flag)
                strcat(buf,")");
            return start;
        }
        if (buflen >= size * 2 + 1) {
            strcat(buf," ");
            buflen--;
        }
    }
    return start;
}


void 
patch(expr,addr,width,memmoveflag)
expr_t *expr;
UINT32 addr;
int width;
int memmoveflag;
{
    symbol_pt s;

    if (!compile_flag)
        return;

    if (!expr->defined)
        error(UNRESOLVED,expr->sym->name);

    s = expr->sym;
    if (s == NULL)
    {
        if(memmoveflag)
	     sub_patch(ScriptPatches,addr,width);
        return;     /* No associated symbol */
    }

    sub_patch(s,addr,width);
    if (s->type == LABEL_REL)
        sub_patch(ScriptPatches,addr,width);
}

void sub_patch(sym,addr,width)
symbol_pt sym;
UINT32 addr;
int width;
{
    patch_t *p;

    p = (patch_pt) scsi_malloc(sizeof(patch_t));
    p->addr = addr - Origin;
    p->width = width;
    p->next = NULL;

    if (sym->patch == NULL) 
        sym->patch = sym->endpatch = p;
    else {
        sym->endpatch->next = p;
        sym->endpatch = p;
    }
}


void init_yacc()
{
    pc = 0;
    InstructionCount = 0;
    ident_label_type = LABEL_REL;
    ScriptPatches->patch = NULL;
    ScriptPatches->endpatch = NULL;
    init_line();
}

void init_line()
{
    register int i;

    Opcode.type = 0;
    Opcode.func = 0;
    Opcode.phase = 0;
    Opcode.indirect = 0;
    Opcode.table = 0;
    Opcode.relative = 0;
    Opcode.atn = 0;
    Opcode.ack = 0;
    Opcode.target = 0;
    Opcode.comp_phase = 0;
    Opcode.comp_data = 0;
    Opcode.mask = 0;
    Opcode.jump_false = 0;
    Opcode.wait = 0;
    Opcode.reg = 0;
    Opcode.reg_op = 0;
    Opcode.f_no = 0;
    Opcode.pass_word1 = NULL;
    Opcode.pass_word2 = NULL;
    Opcode.pass_word0 = NULL;

    for(i = 0; i < MAX_FIELDS; i++)
        Opcode.ispc[i] = 0;
}
static short yydef[] = {
	  -1,   13,   12,   11,   10,    9,  167,    3,   37,   34, 
	  20,   19,   18,   33,   -5,    6,    5,    4,  -17,  -23, 
	  15,   14,  165,  -29,   39,   38,   36,   35,   32,   31, 
	  30,   29,   28,   27,   26,   25,   21,   17,   16,    8, 
	 166,   24,   20,    7
};
static short yyex[] = {
	   0,    0,   -1,    1,  270,   22,  271,   22,  279,   22, 
	  44,   22,   41,   22,   -1,   40,  286,  164,   44,  161, 
	  -1,    1,  286,   52,   44,   55,   -1,    1,   44,   23, 
	  -1,   40
};
static short yyact[] = {
	-144, -145, -151,   -6, -152, -216, -153, -146, -147,   -8, 
	-149, -150, -148, -252, -154, -157, -162, -203, -204, -205, 
	-158, -159, -160, -163, -155, -161, -192, -277, -279, -278, 
	-280, -281,  312,  311,  310,  309,  308,  307,  306,  303, 
	 301,  295,  294,  293,  292,  291,  290,  283,  281,  280, 
	 279,  276,  267,  266,  265,  264,  263,  262,  261,  260, 
	 259,  258,  257,  256, -138, -140, -139, -141, -165, -166, 
	-169, -168,  -10, -167,   -9,  312,  311,  310,  308,  303, 
	 300,  282,  126,   45,   43,   40, -185,   58, -277, -279, 
	-278, -280, -281,  312,  311,  310,  309,  308, -191, -277, 
	-279, -278, -280, -281,  312,  311,  310,  309,  308,  303, 
	-191, -193, -277, -279, -278, -280, -281,  312,  311,  310, 
	 309,  308,  307,  303, -136, -277, -279, -278, -280, -281, 
	 312,  311,  310,  309,  308,  303, -186, -135,   61,   58, 
	-187, -134,   61,   58, -132, -277, -279, -278, -280, -281, 
	 312,  311,  310,  309,  308,  303, -131,   44, -130,   44, 
	-129,   44, -128,   44, -138, -140, -139, -141, -165, -127, 
	-166, -169, -182, -168, -181,  -10, -167,   -9,  312,  311, 
	 310,  309,  308,  307,  303,  300,  287,  282,  126,   45, 
	  43,   40, -125, -213, -269,  271,  270,   44, -230, -229, 
	-228,  299,  298,  297, -124, -123,  -18,  265,  264,  263, 
	-138, -140, -139, -141, -165, -122, -166, -169, -168, -181, 
	 -10, -167,   -9,  312,  311,  310,  309,  308,  303,  300, 
	 288,  282,  126,   45,   43,   40, -138, -140, -139, -141, 
	-165, -119, -120, -166, -169, -168, -181,  -10, -167,   -9, 
	 312,  311,  310,  309,  308,  303,  300,  298,  288,  282, 
	 126,   45,   43,   40, -138, -140, -139, -141, -165, -117, 
	-116, -113, -166, -224, -169, -168, -181,  -10, -167,   -9, 
	 312,  311,  310,  309,  308,  303,  302,  300,  288,  285, 
	 284,  282,  126,   45,   43,   40, -251,  279, -256,  279, 
	-253,  279, -110, -111,   45,   43, -108, -109,   45,   43, 
	-101, -102,  -99,  -97,  -98, -100, -104, -103, -105, -106, 
	 -96,  289,  278,  277,  124,   94,   47,   45,   43,   42, 
	  38,   37,  -95,   44, -101, -102,  -99,  -97,  -98, -100, 
	-104, -103, -105, -106,  278,  277,  124,   94,   47,   45, 
	  43,   42,   38,   37,  -94,   61,  -93,   61, -135,   61, 
	-134,   61,  -92,   40, -138, -140, -139, -141,  -90,  -91, 
	-165, -211, -212, -166, -169, -168, -181,  -10, -167,   -9, 
	 312,  311,  310,  309,  308,  303,  300,  298,  296,  282, 
	 275,  273,  126,   45,   43,   40, -213, -269,  271,  270, 
	-138, -140, -139, -141,  -88, -165, -127, -166, -169, -182, 
	-168, -181,  -10, -167,   -9,  312,  311,  310,  309,  308, 
	 307,  303,  300,  287,  282,  272,  126,   45,   43,   40, 
	-138, -140, -139, -141, -165, -166, -169, -168, -181,  -10, 
	-167,   -9,  312,  311,  310,  309,  308,  303,  300,  282, 
	 126,   45,   43,   40,  -86,   44, -138, -140, -139, -141, 
	-165,  -85, -166, -169, -168, -181,  -10, -167,   -9,  312, 
	 311,  310,  309,  308,  303,  300,  288,  282,  126,   45, 
	  43,   40,  -82,   44,  -80,   40,  -79,  286, -221, -220, 
	-219, -222,  -77,  286,  124,   45,   43,   38, -138, -140, 
	-139, -141, -165, -166, -169, -182, -168, -181,  -10, -167, 
	  -9,  312,  311,  310,  309,  308,  307,  303,  300,  282, 
	 126,   45,   43,   40,  -75,   44, -101, -102, -175,  -99, 
	 -97,  -98, -100, -104, -103, -105, -106,  278,  277,  124, 
	  94,   47,   45,   43,   42,   41,   38,   37,  -74,   40, 
	-138, -140, -139, -141,  -91, -165, -211, -212, -166, -169, 
	-168, -181,  -10, -167,   -9,  312,  311,  310,  309,  308, 
	 303,  300,  298,  296,  282,  275,  126,   45,   43,   40, 
	 -71,  -70,  -72,  274,  272,   44, -138, -140, -139, -141, 
	 -69,  -91, -165, -211, -212, -166, -169, -168, -181,  -10, 
	-167,   -9,  312,  311,  310,  309,  308,  303,  300,  298, 
	 296,  282,  275,  273,  126,   45,   43,   40,  -68,   44, 
	 -66,   44,  -65,   44,  -64,   44, -116, -224,  302,  285, 
	 -61,   44, -138, -140, -139, -141,  -60, -165, -166, -169, 
	-182, -168, -181,  -10, -167,   -9,  312,  311,  310,  309, 
	 308,  307,  303,  300,  282,  268,  126,   45,   43,   40, 
	-101,  -99, -100, -105, -106,  278,  277,   47,   42,   37, 
	-101,  -99,  -97,  -98, -100, -105, -106,  278,  277,   47, 
	  45,   43,   42,   37, -101, -102,  -99,  -97,  -98, -100, 
	-104, -105, -106,  278,  277,   94,   47,   45,   43,   42, 
	  38,   37, -105, -106,  278,  277, -180,   41,  -56,  -57, 
	 274,  272,  -55,   44, -223,   41,  -53,  286,  -50,  -51, 
	 -52,  271,  270,  269,  -49,   44, -184,  -95,   44,   41, 
	 -48,   44, -241,  296, -242,  296, -243,  296,  -45,  -46, 
	 -47,  271,  270,  269, -244,  296, -245,  296, -246,  296,   -1
};
static short yypact[] = {
	  32,  157,  159,  161,  163,  178,  195,  178,  304,  308, 
	 321,  333,  344,  344,  364,  415,  415,  178,  484,  484, 
	 344,  344,  583,  618,  665,  665,  665,  665,  677,  693, 
	 677,  704,  704,  704,  665,  665,  344,  344,  344,  583, 
	 583,  344,  344,  583,  749,  747,  745,  511,  741,  737, 
	 735,  733,  628,  731,  178,  565,  565,  728,  725,  511, 
	 721,  717,  715,  511,  178,  178,  713,  178,  565,  565, 
	 710,  565,  707,   75,  646,  631,  628,  442,  628,   75, 
	 625,  178,  623,  621,  442,  178,  619,  201,  602,  565, 
	 442,  511,   75,   75,   75,  549,   75,   75,   75,   75, 
	  75,   75,   75,   75,   75,   75,  537,   75,   75,   75, 
	  75,  525,  511,  493,  487,  485,  442,  483,  442,  469, 
	 455,  442,  178,  178,  398,  380,  363,  150,  130,  104, 
	  93,  361,  359,   75,   75,  357,  355,   75,   75,   75, 
	  75,  301,  299,  297,  280,  250,  223,  207,  201,  201, 
	 178,  178,  178,  150,  142,  138,  130,  117,  104,   93, 
	  87,   75,   75
};
static short yygo[] = {
	  -1,  -43,  -42,  -39,  -38,  -37,  -36,  -35,  -34,  -33, 
	 -32,  -31,  -30,  -29, -174, -173,  -28,  -27,  -26,  -25, 
	 -22,  -21, -107, -172, -171, -170,  -13,  -11,  -14,  162, 
	 161,  140,  139,  138,  137,  134,  133,  110,  109,  108, 
	 107,  105,  104,  103,  102,  101,  100,   99,   98,   97, 
	  96,   94,   93,   92,   79,   73, -176, -176, -176, -176, 
	-176, -176, -178, -176, -176, -176, -179, -178, -177, -178, 
	-176,  -20, -177, -177, -183,  146,  145,  144,  125,  121, 
	 119,  118,  116,   90,   89,   88,   84,   77,   71,   69, 
	  68,   56,   55,  -15,  -15,  -15,  -15,  -15,  -15,  -15, 
	 -15,  -15,  -15,  -15,  -15,  -15,  -15,  -24,  -15,  -15, 
	 -15,  -24,  -24,  -15,  -15,  -15,  -15,  -15, -164,  152, 
	 151,  150,  123,  122,  121,  118,  112,   91,   85,   84, 
	  81,   74,   67,   65,   64,   63,   59,   54,   47,   17, 
	  16,   15,    7,    5,  -63, -225, -227, -226, -114,   78, 
	  76,   52,  -78, -206, -202, -156, -133, -137, -188, -133, 
	-137, -189, -190,  159,  156,  153,  130,  128,  127,    0, 
	  -5, -199, -200, -201,  153,  127,   -4, -197, -198,  128, 
	  -3, -194, -196, -195,  157,  129, -208, -276, -207, -254, 
	-142, -143, -250, -249, -248, -247,  -81, -112,  116,  -59, 
	-275, -240,  -54, -257,  -73,  -76, -284,  112,   91,   74, 
	  63,   59,   47, -239, -238, -237, -236, -235, -258,  -84, 
	-118, -121,  145,  119, -261, -266, -265, -262, -232, -234, 
	-233, -231, -259, -260, -263, -264, -218, -217, -215, -214, 
	 152,  151,  150,  123,  122,   85,   81,   67,   65,   64, 
	  54,   17,   16,   15,    7,  -67,  -83,  -87,  118,   84, 
	 -17,  -16,  148, -268, -267,   87,  -62, -209, -115, -210, 
	 144,   90,   77,   -7,  -89, -126,  124,  -44,  -41,  -40, 
	 -23,   89,   88,   68, -272, -274, -271, -273, -270,   71, 
	  69,   56,   55,   -2, -282,  -58,  -12,   73, -288, -288, 
	-288, -288, -288, -288, -286, -288, -288, -288, -285, -286, 
	-287, -286, -288,  -19, -287, -287, -283,  146,  145,  144, 
	 125,  121,  119,  118,  116,   90,   89,   88,   84,   77, 
	  71,   69,   68,   56,   55,   -1
};
static short yypgo[] = {
	   0,    0,    0,  214,  214,  216,  217,  194,  194,  273, 
	 191,  191,  191,  191,  173,  173,  178,  178,  153,  294, 
	 296,  296,  206,  257,  144,   28,   28,   28,   28,   28, 
	  28,   28,   28,   74,  118,  118,  118,  118,  118,  118, 
	  28,   28,   28,   28,   28,   28,   28,   28,   28,   28, 
	  28,   28,  269,  221,  257,  197,  239,  316,  206,  206, 
	 294,  186,  186,  186,  293,  293,  183,  183,  188,  187, 
	 180,  180,  180,  176,  176,  170,  170,  170,  191,  191, 
	 191,  191,  191,  191,  191,  288,  288,  288,  288,  275, 
	 273,  273,  273,  273,  273,  152,  152,  152,  152,  148, 
	 148,  218,  218,  218,  264,  264,  264,  213,  213,  213, 
	 213,  193,  193,  193,  193,  193,  199,  192,  192,  192, 
	 192,  192,  192,  190,  190,  190,  190,  189,  189,  189, 
	   0,    0,  189,  199,  193,  213,  213,  214,  214,  215, 
	 215,  216,  217,  261,  261,  275,  280,  280,  280,  280, 
	 280,  195,  191,  162,  162,  162,  162,  162,  154,  206, 
	 239,  197,  257,  221,  269,  194,  194,  194,    0
};
static short yyrlen[] = {
	   0,    0,    0,    1,    2,    2,    2,    5,    4,    1, 
	   1,    1,    1,    1,    3,    3,    3,    3,    2,    1, 
	   1,    3,    1,    1,    1,    3,    3,    3,    3,    3, 
	   3,    3,    3,    1,    1,    3,    3,    1,    3,    3, 
	   1,    1,    1,    1,    1,    1,    2,    2,    2,    3, 
	   3,    3,    1,    1,    1,    1,    4,    1,    1,    1, 
	   5,    2,    2,    2,    3,    2,    1,    1,    1,    2, 
	   3,    2,    2,    3,    2,    3,    2,    1,    1,    1, 
	   1,    1,    1,    1,    1,    2,    1,    1,    1,    1, 
	   2,    2,    1,    2,    2,    1,    1,    1,    1,    4, 
	   1,    6,    4,    4,    1,    1,    1,    5,    6,    5, 
	   5,    1,    1,    1,    1,    1,    2,    6,    6,    6, 
	   7,    7,    7,    1,    1,    1,    1,    2,    1,    2, 
	   2,    0,    2,    1,    1,    4,    4,    2,    3,    3, 
	   3,    3,    3,    1,    3,    1,    1,    3,    4,    3, 
	   4,    7,    1,    1,    1,    1,    1,    1,    2,    1, 
	   1,    1,    1,    1,    1,    3,    4,    1,    2
};
#define YYS0	254
#define YYDELTA	123
#define YYNPACT	163
#define YYNDEF	44

#define YYr166	0
#define YYr167	1
#define YYr168	2
#define YYr31	3
#define YYr33	4
#define YYr37	5
#define YYr39	6
#define YYr57	7
#define YYr59	8
#define YYr64	9
#define YYr78	10
#define YYr79	11
#define YYr80	12
#define YYr81	13
#define YYr93	14
#define YYr94	15
#define YYr102	16
#define YYr103	17
#define YYr116	18
#define YYr118	19
#define YYr120	20
#define YYr121	21
#define YYr123	22
#define YYr132	23
#define YYr138	24
#define YYr140	25
#define YYr141	26
#define YYr142	27
#define YYr143	28
#define YYr144	29
#define YYr145	30
#define YYr146	31
#define YYr147	32
#define YYr159	33
#define YYr160	34
#define YYr161	35
#define YYr162	36
#define YYr163	37
#define YYr164	38
#define YYr165	39
#define YYr158	40
#define YYr157	41
#define YYr156	42
#define YYr155	43
#define YYr154	44
#define YYr153	45
#define YYr152	46
#define YYr151	47
#define YYr150	48
#define YYr149	49
#define YYr148	50
#define YYr139	51
#define YYr136	52
#define YYr134	53
#define YYr131	54
#define YYr129	55
#define YYr127	56
#define YYr126	57
#define YYr125	58
#define YYr122	59
#define YYr119	60
#define YYr115	61
#define YYr114	62
#define YYr113	63
#define YYr112	64
#define YYr111	65
#define YYr110	66
#define YYr109	67
#define YYr108	68
#define YYr107	69
#define YYr106	70
#define YYr105	71
#define YYr104	72
#define YYr101	73
#define YYr100	74
#define YYr92	75
#define YYr91	76
#define YYr90	77
#define YYr89	78
#define YYr88	79
#define YYr87	80
#define YYr86	81
#define YYr85	82
#define YYr83	83
#define YYr82	84
#define YYr76	85
#define YYr75	86
#define YYr74	87
#define YYr73	88
#define YYr67	89
#define YYr65	90
#define YYr63	91
#define YYr62	92
#define YYr61	93
#define YYr60	94
#define YYr54	95
#define YYr53	96
#define YYr52	97
#define YYr51	98
#define YYr50	99
#define YYr49	100
#define YYr48	101
#define YYr47	102
#define YYr46	103
#define YYr45	104
#define YYr44	105
#define YYr43	106
#define YYr30	107
#define YYr28	108
#define YYr27	109
#define YYr26	110
#define YYr23	111
#define YYr22	112
#define YYr21	113
#define YYr20	114
#define YYr19	115
#define YYr18	116
#define YYr16	117
#define YYr15	118
#define YYr14	119
#define YYr13	120
#define YYr12	121
#define YYr11	122
#define YYr10	123
#define YYr9	124
#define YYr8	125
#define YYr7	126
#define YYr6	127
#define YYr5	128
#define YYr3	129
#define YYr2	130
#define YYr1	131
#define YYrACCEPT	YYr166
#define YYrERROR	YYr167
#define YYrLR2	YYr168
#line 2 "/etc/yyparse.c"

/*
 * Automaton to interpret LALR(1) tables.
 *
 *	Macros:
 *		yyclearin - clear the lookahead token.
 *		yyerrok - forgive a pending error
 *		YYERROR - simulate an error
 *		YYACCEPT - halt and return 0
 *		YYABORT - halt and return 1
 *		YYRETURN(value) - halt and return value.  You should use this
 *			instead of return(value).
 *		YYREAD - ensure yychar contains a lookahead token by reading
 *			one if it does not.  See also YYSYNC.
 *
 *	Preprocessor flags:
 *		YYDEBUG - includes debug code.  The parser will print
 *			 a travelogue of the parse if this is defined
 *			 and yydebug is non-zero.
 *		YYSSIZE - size of state and value stacks (default 150).
 *		YYSTATIC - By default, the state stack is an automatic array.
 *			If this is defined, the stack will be static.
 *			In either case, the value stack is static.
 *		YYALLOC - Dynamically allocate both the state and value stacks
 *			by calling malloc() and free().
 *		YYLR2 - defined if lookahead is needed to resolve R/R or S/R conflicts
 *		YYSYNC - if defined, yacc guarantees to fetch a lookahead token
 *			before any action, even if it doesnt need it for a decision.
 *			If YYSYNC is defined, YYREAD will never be necessary unless
 *			the user explicitly sets yychar = -1
 *
 *	Copyright (c) 1983, by the University of Waterloo
 */

#ifndef YYSSIZE
# define YYSSIZE	150
#endif
#ifndef	YYDEBUG
#define	YYDEBUG	0
#endif
#define YYERROR		goto yyerrlabel
#define yyerrok		yyerrflag = 0
#define yyclearin	yychar = -1
#define YYACCEPT	YYRETURN(0)
#define YYABORT		YYRETURN(1)
#ifdef YYALLOC
# define YYRETURN(val)	{ retval = (val); goto yyReturn; }
#else
# define YYRETURN(val)	return(val)
#endif
#if YYDEBUG
/* The if..else makes this macro behave exactly like a statement */
# define YYREAD	if (yychar < 0) {					\
			if ((yychar = yylex()) < 0)			\
				yychar = 0;				\
			if (yydebug)					\
				printf("read %s (%d)\n", yyptok(yychar),\
				yychar);				\
		} else
#else
# define YYREAD	if (yychar < 0) {					\
			if ((yychar = yylex()) < 0)			\
				yychar = 0;				\
		} else
#endif
#define YYERRCODE	256		/* value of `error' */
#if defined(__TURBOC__)&&__SMALL__
#define	YYQYYP	*(int *)((int)yyq + ((int)yyq-(int)yyp))
#else
#define	YYQYYP	yyq[yyq-yyp]
#endif

YYSTYPE	yyval,				/* $$ */
	*yypvt,				/* $n */
	yylval;				/* yylex() sets this */

int	yychar,				/* current token */
	yyerrflag,			/* error flag */
	yynerrs;			/* error count */

#if YYDEBUG
int yydebug = YYDEBUG-0;		/* debug flag & tables */
extern char	*yysvar[], *yystoken[], *yyptok();
extern short	yyrmap[], yysmap[];
extern int	yynstate, yynvar, yyntoken, yynrule;
# define yyassert(condition, msg, arg) \
	if (!(condition)) { printf("\nyacc bug: "); printf(msg, arg); YYABORT; }
#else /* !YYDEBUG */
# define yyassert(condition, msg, arg)
#endif

yyparse()
{

	register short		yyi, *yyp;	/* for table lookup */
	register short		*yyps;		/* top of state stack */
	register short		yystate;	/* current state */
	register YYSTYPE	*yypv;		/* top of value stack */
	register short		*yyq;
	register int		yyj;

#ifdef YYSTATIC
	static short	yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];
#else
#ifdef YYALLOC
	YYSTYPE *yyv;
	short	*yys;
	YYSTYPE save_yylval, save_yyval, *save_yypvt;
	int save_yychar, save_yyerrflag, save_yynerrs;
	int retval;
#if 0	/* defined in <stdlib.h>*/
	extern char	*malloc();
#endif
#else
	short		yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];	/* historically static */
#endif
#endif

#ifdef YYALLOC
	yys = (short *) malloc((YYSSIZE + 1) * sizeof(short));
	yyv = (YYSTYPE *) malloc((YYSSIZE + 1) * sizeof(YYSTYPE));
	if (yys == (short *)0 || yyv == (YYSTYPE *)0) {
		yyerror("Not enough space for parser stacks");
		return 1;
	}
	save_yylval = yylval;
	save_yyval = yyval;
	save_yypvt = yypvt;
	save_yychar = yychar;
	save_yyerrflag = yyerrflag;
	save_yynerrs = yynerrs;
#endif

	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;
	yyps = yys;
	yypv = yyv;
	yystate = YYS0;		/* start state */

yyStack:
	yyassert((unsigned)yystate < yynstate, "state %d\n", yystate);
	if (++yyps > &yys[YYSSIZE]) {
		yyerror("Parser stack overflow");
		YYABORT;
	}
	*yyps = yystate;	/* stack current state */
	*++yypv = yyval;	/* ... and value */

#if YYDEBUG
	if (yydebug)
		printf("state %d (%d), char %s (%d)\n", yysmap[yystate],
			yystate, yyptok(yychar), yychar);
#endif

	/*
	 *	Look up next action in action table.
	 */
yyEncore:
#ifdef YYSYNC
	YYREAD;
#endif
	if (yystate >= sizeof yypact/sizeof yypact[0]) 	/* simple state */
		yyi = yystate - YYDELTA;	/* reduce in any case */
	else {
		if(*(yyp = &yyact[yypact[yystate]]) >= 0) {
			/* Look for a shift on yychar */
#ifndef YYSYNC
			YYREAD;
#endif
			yyq = yyp;
			yyi = yychar;
#if 0&&defined(__TURBOC__)&&__SMALL__
	/* THIS ONLY WORKS ON TURBO C 1.5 !!! */
			/* yyi is in di, yyp is in si */
		L01:
			asm lodsw	/* ax = *yyp++; */
			asm cmp yyi, ax
			asm jl L01
#else
			while (yyi < *yyp++)
				;
#endif
			if (yyi == yyp[-1]) {
				yystate = ~YYQYYP;
#if YYDEBUG
				if (yydebug)
					printf("shift %d (%d)\n", yysmap[yystate], yystate);
#endif
				yyval = yylval;		/* stack what yylex() set */
				yychar = -1;		/* clear token */
				if (yyerrflag)
					yyerrflag--;	/* successful shift */
				goto yyStack;
			}
		}

		/*
	 	 *	Fell through - take default action
	 	 */

		if (yystate >= sizeof yydef /sizeof yydef[0])
			goto yyError;
		if ((yyi = yydef[yystate]) < 0)	 { /* default == reduce? */
											/* Search exception table */
			yyassert((unsigned)~yyi < sizeof yyex/sizeof yyex[0],
				"exception %d\n", yystate);
			yyp = &yyex[~yyi];
#ifndef YYSYNC
			YYREAD;
#endif
			while((yyi = *yyp) >= 0 && yyi != yychar)
				yyp += 2;
			yyi = yyp[1];
			yyassert(yyi >= 0,"Ex table not reduce %d\n", yyi);
		}
	}

#ifdef YYLR2
yyReduce:	/* reduce yyi */
#endif
	yyassert((unsigned)yyi < yynrule, "reduce %d\n", yyi);
	yyj = yyrlen[yyi];
#if YYDEBUG
	if (yydebug) printf("reduce %d (%d), pops %d (%d)\n", yyrmap[yyi],
		yyi, yysmap[yyps[-yyj]], yyps[-yyj]);
#endif
	yyps -= yyj;		/* pop stacks */
	yypvt = yypv;		/* save top */
	yypv -= yyj;
	yyval = yypv[1];	/* default action $$ = $1 */
	switch (yyi) {		/* perform semantic action */
		
case YYr1: {	/* script :  */
#line 89 "scsi.y"
 init_yacc(); 
} break;

case YYr2: {	/* script :  script line */
#line 91
 init_line(); 
} break;

case YYr3: {	/* line :  instruction EOL */
#line 96
  handle_opcode();
} break;

case YYr5: {	/* line :  EOL */
#line 99
 handle_blank(); 
} break;

case YYr6: {	/* line :  error EOL */
#line 101
 
                  yyerrok;
                  handle_blank();
                
} break;

case YYr7: {	/* instruction :  move */
#line 109
 Opcode.type = 0; 
} break;

case YYr8: {	/* instruction :  io */
#line 111
 Opcode.type = 1; 
} break;

case YYr9: {	/* instruction :  transfer */
#line 113
 
                  if (Opcode.mask && !Opcode.comp_data)
                      error(MASK_WO_DATA,"");
                  Opcode.type = 2;
                
} break;

case YYr10: {	/* instruction :  mem_move */
#line 119
 Opcode.type = 3; 
} break;

case YYr11: {	/* move :  MOVE count24 ',' dest ',' WITH PHASE */
#line 124
 
                  Opcode.func = 0;
                  Opcode.phase = yypvt[0].i;
                
} break;

case YYr12: {	/* move :  MOVE count24 ',' dest ',' WHEN PHASE */
#line 129
 
                  Opcode.func = 1;
                  Opcode.phase = yypvt[0].i;
                
} break;

case YYr13: {	/* move :  MOVE count24 ',' dest ',' IF PHASE */
#line 134
 
                  Opcode.func = 0;
                  Opcode.phase = yypvt[0].i;
                
} break;

case YYr14: {	/* move :  MOVE FROM address32 ',' WITH PHASE */
#line 139
 
                  Opcode.func = 0;
                  Opcode.phase = yypvt[0].i;
                  Opcode.table = 1;
                
} break;

case YYr15: {	/* move :  MOVE FROM address32 ',' WHEN PHASE */
#line 145
 

                  Opcode.func = 1;
                  Opcode.phase = yypvt[0].i;
                  Opcode.table = 1;
                
} break;

case YYr16: {	/* move :  MOVE FROM address32 ',' IF PHASE */
#line 152
 
                  Opcode.func = 0;
                  Opcode.phase = yypvt[0].i;
                  Opcode.table = 1;
                
} break;

case YYr18: {	/* dest :  PTR address32 */
#line 162
 Opcode.indirect = 1; 
} break;

case YYr19: {	/* io :  sel_io */
#line 167
 Opcode.func = 0; 
} break;

case YYr20: {	/* io :  disc_io */
#line 169
 Opcode.func = 1; 
} break;

case YYr21: {	/* io :  wait_io */
#line 171
 Opcode.func = 2; 
} break;

case YYr22: {	/* io :  set_io */
#line 173
 Opcode.func = 3; 
} break;

case YYr23: {	/* io :  clear_io */
#line 175
 Opcode.func = 4; 
} break;

case YYr26: {	/* sel_io :  SELECT FROM address24 ',' rel_addr32 */
#line 182
 Opcode.table = 1; 
} break;

case YYr27: {	/* sel_io :  SELECT ATN id ',' rel_addr32 */
#line 184
 Opcode.atn = 1; 
} break;

case YYr28: {	/* sel_io :  SELECT ATN FROM address24 ',' rel_addr32 */
#line 186

                  Opcode.table = 1;
                  Opcode.atn = 1;
                
} break;

case YYr30: {	/* sel_io :  RESELECT FROM address24 ',' rel_addr32 */
#line 192
 Opcode.table = 1; 
} break;

case YYr31: {	/* disc_io :  DISCONNECT */
#line 197
Opcode.fields[0].value = 0; 
} break;

case YYr33: {	/* disc_io :  WAIT DISCONNECT */
#line 200
 Opcode.fields[0].value = 0; 
} break;

case YYr37: {	/* set_io :  SET bits */
#line 211
 Opcode.fields[0].value = 0; 
} break;

case YYr39: {	/* clear_io :  CLEAR bits */
#line 217
 Opcode.fields[0].value = 0; 
} break;

case YYr43: {	/* bit :  ACK */
#line 228

                  if (Opcode.ack)
                      error(MULTIPLE_ACK,"");
                  Opcode.ack = 1;
                
} break;

case YYr44: {	/* bit :  ATN */
#line 234

                  if (Opcode.atn)
                      error(MULTIPLE_ATN,"");
                  Opcode.atn = 1;
                
} break;

case YYr45: {	/* bit :  TARGET */
#line 240

                  if (Opcode.target)
                      error(MULTIPLE_TARGET,"");
                  Opcode.target = 1;
                
} break;

case YYr46: {	/* reg_io :  MOVE register TO register */
#line 249
 
                  
                  if (yypvt[-2].i == 0x08) {
                      Opcode.func = 5;
                      Opcode.reg = yypvt[0].i;
                      Opcode.reg_op = REG_OR;
                  }
                  else if (yypvt[0].i == 0x08) {
                      Opcode.func = 6;
                      Opcode.reg = yypvt[-2].i;
                      Opcode.reg_op = REG_OR;
                  }
                  else  {
                      Opcode.func = 7;
                      Opcode.reg = yypvt[-2].i;
                      if (yypvt[-2].i != yypvt[0].i)
                          error(REG_IO_ILLEGAL,"");
                  }
                
} break;

case YYr47: {	/* reg_io :  MOVE data8 TO register */
#line 269
 
                  Opcode.func = 7;
                  Opcode.reg = yypvt[0].i;
                
} break;

case YYr48: {	/* reg_io :  MOVE register reg_op data8 TO register */
#line 274
 
                  if (yypvt[-3].i)          
                      Opcode.fields[Opcode.f_no-1].value = 0xff & (-Opcode.fields[Opcode.f_no-1].value);

                  

                  if (yypvt[-4].i == 0x08) {
                      Opcode.func = 5;
                      Opcode.reg = yypvt[0].i;
                  }
                  else if (yypvt[0].i == 0x08) {
                      Opcode.func = 6;
                      Opcode.reg = yypvt[-4].i;
                  }
                  else  {
                      Opcode.func = 7;
                      Opcode.reg = yypvt[-4].i;
                      if (yypvt[-4].i != yypvt[0].i)

                          error(REG_IO_ILLEGAL,"");
                  }

                
} break;

case YYr49: {	/* register :  REGISTER */
#line 301
 yyval.i = yypvt[0].i; 
} break;

case YYr50: {	/* register :  REG '(' reg ')' */
#line 303
 yyval.i = yypvt[-1].e.value; 
} break;

case YYr51: {	/* reg_op :  '|' */
#line 308

                  Opcode.reg_op = REG_OR;
                  yyval.i = 0;  
                
} break;

case YYr52: {	/* reg_op :  '&' */
#line 313
 
                  Opcode.reg_op = REG_AND;
                  yyval.i = 0;
                
} break;

case YYr53: {	/* reg_op :  '+' */
#line 318
 
                  Opcode.reg_op = REG_PLUS;
                  yyval.i = 0;
                
} break;

case YYr54: {	/* reg_op :  '-' */
#line 323
 
                  Opcode.reg_op = REG_PLUS;
                  yyval.i = 1;       
                
} break;

case YYr57: {	/* transfer :  tran_op ',' wait NOT p_cond */
#line 333
 Opcode.jump_false = 1; 
} break;

case YYr59: {	/* transfer :  tran_op wait NOT p_cond */
#line 336
 Opcode.jump_false = 1; 
} break;

case YYr60: {	/* tran_op :  JUMP rel_addr32 */
#line 341
 Opcode.func = 0; 
} break;

case YYr61: {	/* tran_op :  CALL rel_addr32 */
#line 343
 Opcode.func = 1; 
} break;

case YYr62: {	/* tran_op :  RETURN */
#line 345
 Opcode.func = 2; 
} break;

case YYr63: {	/* tran_op :  INT rel_addr32 */
#line 347
 Opcode.func = 3; 
} break;

case YYr64: {	/* tran_op :  NOP */
#line 349
 Opcode.func = 0; 
                  Opcode.jump_false = 1;
                  Opcode.fields[0].value = 0; 
                 
} break;

case YYr65: {	/* tran_op :  NOP rel_addr32 */
#line 354
 Opcode.func = 0;
                  Opcode.jump_false = 1;
                
} break;

case YYr67: {	/* wait :  WHEN */
#line 362
 Opcode.wait = 1; 
} break;

case YYr73: {	/* test :  ATN */
#line 376

                  if (Opcode.atn)
                      error(MULTIPLE_ATN,"");
                  if (Opcode.comp_phase)
                      error(ATN_AND_PHASE,"");
                  Opcode.atn = 1;
                
} break;

case YYr74: {	/* test :  PHASE */
#line 384
 
                  if (Opcode.comp_phase)
                      error(MULTIPLE_PHASE,"");
                  if (Opcode.atn)
                      error(ATN_AND_PHASE,"");
                  Opcode.comp_phase = 1;
                  Opcode.phase = yypvt[0].i;
                
} break;

case YYr75: {	/* test :  data8 */
#line 393

                  if (Opcode.comp_data)
                      error(MULTIPLE_DATA,"");
                  Opcode.comp_data = Opcode.f_no;
                
} break;

case YYr76: {	/* test :  MASK data8 */
#line 399

                  if (Opcode.mask)
                      error(MULTIPLE_MASK,"");
                  Opcode.mask = Opcode.f_no;
                
} break;

case YYr78: {	/* pseudo_op :  absolute */
#line 412
 handle_ident(); 
} break;

case YYr79: {	/* pseudo_op :  relative */
#line 414
 handle_ident(); 
} break;

case YYr80: {	/* pseudo_op :  external */
#line 416
 handle_ident(); 
} break;

case YYr81: {	/* pseudo_op :  entry */
#line 418
 handle_blank(); 
} break;

case YYr82: {	/* pseudo_op :  label */
#line 420
 handle_label(yypvt[0].s); 
} break;

case YYr83: {	/* pseudo_op :  pass_word */
#line 422
 handle_pass(yypvt[0].s); 
} break;

case YYr85: {	/* pseudo_op :  orig */
#line 425
 handle_orig(yypvt[0].i); 
} break;

case YYr86: {	/* pseudo_op :  BAD_PARITY */
#line 427
 handle_parity(); 
} break;

case YYr87: {	/* pseudo_op :  GOOD_PARITY */
#line 429
 handle_parity(); 
} break;

case YYr88: {	/* pseudo_op :  ALIGN */
#line 431
 handle_align(); 
} break;

case YYr89: {	/* pseudo_op :  data_pseudo */
#line 433
 handle_data(yypvt[0].i); 
} break;

case YYr90: {	/* absolute :  abs */
#line 438
 yyval.s = yypvt[0].s; 
} break;

case YYr91: {	/* absolute :  ABSOLUTE abs */
#line 440
 yyval.s = yypvt[0].s; 
} break;

case YYr92: {	/* absolute :  absolute ',' abs */
#line 442
 yyval.s = yypvt[-2].s; 
} break;

case YYr93: {	/* abs :  IDENTIFIER '=' expr */
#line 447

                  yypvt[-2].s->type = IDENT_ABS;
                  yypvt[-2].s->defined = yypvt[0].e.defined;
                  yypvt[-2].s->value = yypvt[0].e.value;
                  yypvt[-2].s->pass = 1;
                  yyval.s = yypvt[-2].s;
                
} break;

case YYr94: {	/* abs :  any_ident '=' expr */
#line 455

                  if (yypvt[-2].s->pass == iteration_num)
                      error(MULTIPLE_DECLARATION,yypvt[-2].s->name);
                  else if (!yypvt[-2].s->defined) {           
                      yypvt[-2].s->defined = yypvt[0].e.defined;
                      yypvt[-2].s->value = yypvt[0].e.value;
                  }
                  yypvt[-2].s->pass = iteration_num;
                  yyval.s = yypvt[-2].s;
                
} break;

case YYr100: {	/* relative :  RELATIVE rel */
#line 477
 yyval.s = yypvt[0].s; 
} break;

case YYr101: {	/* relative :  relative ',' rel */
#line 479
 yyval.s = yypvt[-2].s; 
} break;

case YYr102: {	/* rel :  IDENTIFIER '=' expr */
#line 484

                  yypvt[-2].s->type = IDENT_REL;
                  yypvt[-2].s->defined = yypvt[0].e.defined;
                  yypvt[-2].s->value = yypvt[0].e.value;
                  yypvt[-2].s->pass = 1;
                  yyval.s = yypvt[-2].s;
                
} break;

case YYr103: {	/* rel :  any_ident '=' expr */
#line 492

                  if (yypvt[-2].s->pass == iteration_num)
                      error(MULTIPLE_DECLARATION,yypvt[-2].s->name);
                  else if (!yypvt[-2].s->defined) {           
                      yypvt[-2].s->defined = yypvt[0].e.defined;
                      yypvt[-2].s->value = yypvt[0].e.value;
                  }
                  yypvt[-2].s->pass = iteration_num;
                  yyval.s = yypvt[-2].s;
                
} break;

case YYr104: {	/* external :  EXTERN ext */
#line 506
 yyval.s = yypvt[0].s; 
} break;

case YYr105: {	/* external :  EXTERNAL ext */
#line 508
 yyval.s = yypvt[0].s; 
} break;

case YYr106: {	/* external :  external ',' ext */
#line 510
 yyval.s = yypvt[-2].s; 
} break;

case YYr107: {	/* pass_ext :  EXTERN PASSIFIER */
#line 514
  yypvt[0].s->defined = 1;
                  handle_ext(yypvt[0].s);
               
} break;

case YYr108: {	/* pass_word :  PASSIFIER */
#line 519

                yypvt[0].s->defined = 1;
                yyval.s = yypvt[0].s;
               
} break;

case YYr109: {	/* ext :  IDENTIFIER */
#line 525

                  yypvt[0].s->type = IDENT_EXT;
                  yypvt[0].s->defined = 1;
                  yypvt[0].s->value = 0;
                  yypvt[0].s->pass = 1;
                  yyval.s = yypvt[0].s;
                
} break;

case YYr110: {	/* ext :  any_ident */
#line 533

                  if (yypvt[0].s->pass == iteration_num)
                      error(MULTIPLE_DECLARATION,yypvt[0].s->name);
                  yypvt[0].s->pass = iteration_num;
                  yyval.s = yypvt[0].s;
                
} break;

case YYr111: {	/* entry :  ENTRY any_ident */
#line 543
 
                  if (yypvt[0].s->type == IDENTIFIER)       
                      yypvt[0].s->type = ident_label_type;
                  if (yypvt[0].s->type != LABEL_ABS && yypvt[0].s->type != LABEL_REL)
                      error(ENTRY_NOT_LABEL,yypvt[0].s->name);
                  else
                      yypvt[0].s->entry = 1;
                
} break;

case YYr112: {	/* entry :  entry ',' any_ident */
#line 552
 
                  if (yypvt[0].s->type == IDENTIFIER)       
                      yypvt[0].s->type = ident_label_type;
                  if (yypvt[0].s->type != LABEL_ABS && yypvt[0].s->type != LABEL_REL)
                      error(ENTRY_NOT_LABEL,yypvt[0].s->name);
                  else
                      yypvt[0].s->entry = 1;
                
} break;

case YYr113: {	/* label :  IDENTIFIER ':' */
#line 564

                  yypvt[-1].s->type = ident_label_type;  
                  yypvt[-1].s->defined = 1;
                  yypvt[-1].s->value = pc;
                  yypvt[-1].s->pass = 1;
                  yyval.s = yypvt[-1].s;
                
} break;

case YYr114: {	/* label :  any_ident ':' */
#line 572

                  if (yypvt[-1].s->pass == iteration_num)
                      error(MULTIPLE_DECLARATION,yypvt[-1].s->name);
                  yypvt[-1].s->pass = iteration_num;
                  yyval.s = yypvt[-1].s;
                
} break;

case YYr115: {	/* label :  ARRAYNAME ':' */
#line 579

                 yypvt[-1].s-> defined = 1;
                 yyval.s = yypvt[-1].s;
                
} break;

case YYr116: {	/* orig :  ORIGIN expr */
#line 587

                  if (yypvt[0].e.defined == 0) {
                      error(ORIGIN_UNDEFINED,"");
                      yyval.i = pc;
                  }
                  else {
                      ident_label_type = LABEL_REL;     
                      yyval.i = yypvt[0].e.value;
		      Origin = yyval.i;
                  }
                
} break;

case YYr118: {	/* data_list :  expr_list */
#line 606
 dup = 1; 
} break;

case YYr119: {	/* data_list :  expr DUP '(' expr_list ')' */
#line 608
 dup = yypvt[-4].e.value; 
} break;

case YYr120: {	/* expr_list :  expr */
#line 614
 
                  data_list[0] = yypvt[0].e;
                  d_ptr = 1;
                
} break;

case YYr121: {	/* expr_list :  expr_list ',' expr */
#line 619
 data_list[d_ptr++] = yypvt[0].e; 
} break;

case YYr122: {	/* address32 :  abs_expr */
#line 623
 Opcode.fields[Opcode.f_no++] = yypvt[0].e; 
} break;

case YYr123: {	/* address32 :  rel_expr */
#line 625
 Opcode.fields[Opcode.f_no++] = yypvt[0].e; 
} break;

case YYr125: {	/* address32 :  PASSIFIER */
#line 628
	 if (Opcode.f_no == 1) 
                  Opcode.pass_word1 = yypvt[0].s-> name;
	  else if (Opcode.f_no == 2)
                  Opcode.pass_word2 = yypvt[0].s-> name;
	  else 
		Opcode.pass_word1 = yypvt[0].s-> name;

	  Opcode.f_no++;
	  yypvt[0].s -> defined = 1;
                
} break;

case YYr126: {	/* external_val :  IDENT_EXT */
#line 640
 
                  Opcode.fields[Opcode.f_no].value = 0;
                  Opcode.fields[Opcode.f_no].defined = 1;
                  Opcode.fields[Opcode.f_no].sym = yypvt[0].s;
                  Opcode.f_no++;
                
} break;

case YYr127: {	/* rel_addr32 :  REL '(' address32 ')' */
#line 651

                  if (compile_flag) {
                      if (Opcode.fields[Opcode.f_no-1].sym != NULL &&
                            Opcode.fields[Opcode.f_no-1].sym->type == IDENT_EXT)
                          error(RELATIVE_EXTERN);
                      Opcode.fields[Opcode.f_no-1].value -= pc + 8; 
                      Opcode.fields[Opcode.f_no-1].sym = NULL;
                  }
                  Opcode.relative = 1;
                
} break;

case YYr129: {	/* count24 :  abs_expr */
#line 666

                  if (yypvt[0].e.value > 0xffffff) {
                      error(COUNT24_RANGE,"");
                      yypvt[0].e.value &= 0xffffff;
                  }
                  Opcode.fields[Opcode.f_no++] = yypvt[0].e;
                
} break;

case YYr131: {	/* address24 :  abs_expr */
#line 678

                  if (yypvt[0].e.value > 0xffffff) {
                      error(ADDR24_RANGE,"");
                      yypvt[0].e.value &= 0xffffff;
                  }
                  Opcode.fields[Opcode.f_no++] = yypvt[0].e;
                
} break;

case YYr132: {	/* address24 :  rel_expr */
#line 686

                  if (yypvt[0].e.value > 0xffffff) {
                      error(ADDR24_RANGE,"");
                      yypvt[0].e.value &= 0xffffff;
                  }
                  Opcode.fields[Opcode.f_no++] = yypvt[0].e;
                
} break;

case YYr134: {	/* id :  abs_expr */
#line 698

                  if (yypvt[0].e.value > 0xff) {
                      error(ID_RANGE,"");
                      yypvt[0].e.value &= 0xff;
                  }
                 Opcode.fields[Opcode.f_no++] = yypvt[0].e;
                
} break;

case YYr136: {	/* data8 :  abs_expr */
#line 711

                  if (yypvt[0].e.value > 0xff) {
                      error(DATA_RANGE,"");
                  }
                  yypvt[0].e.value &= 0xff;
                  Opcode.fields[Opcode.f_no++] = yypvt[0].e;
                
} break;

case YYr138: {	/* reg :  expr */
#line 723
 if (yypvt[0].e.value > 0x3f || yypvt[0].e.value < 0)
                      error(REGISTER_RANGE,"");
                  yyval.e.value = yypvt[0].e.value & 0x3f;
                  yyval.e.defined = yypvt[0].e.defined;
                
} break;

case YYr139: {	/* expr :  '(' expr ')' */
#line 732
 if ((yyval.e.defined = yypvt[-1].e.defined) == 1)
                      yyval.e.value = yypvt[-1].e.value;
                  else yyval.e.value = 0;
                
} break;

case YYr140: {	/* expr :  expr '+' expr */
#line 737
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value + yypvt[0].e.value;
                  else yyval.e.value = 0;
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr141: {	/* expr :  expr '-' expr */
#line 742
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value - yypvt[0].e.value;
                  else yyval.e.value = 0;
                
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr142: {	/* expr :  expr '*' expr */
#line 747
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value * yypvt[0].e.value;
                  else yyval.e.value = 0;
                
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr143: {	/* expr :  expr '/' expr */
#line 752
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value / yypvt[0].e.value;
                  else yyval.e.value = 0;
                
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr144: {	/* expr :  expr '%' expr */
#line 757
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value % yypvt[0].e.value;
                  else yyval.e.value = 0;
                
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr145: {	/* expr :  expr '&' expr */
#line 762
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value & yypvt[0].e.value;
                  else yyval.e.value = 0;
                
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr146: {	/* expr :  expr '|' expr */
#line 767
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value | yypvt[0].e.value;
                  else yyval.e.value = 0;
                
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr147: {	/* expr :  expr '^' expr */
#line 772
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value ^ yypvt[0].e.value;
                  else yyval.e.value = 0;
                
	  yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr148: {	/* expr :  expr SHIFT_LEFT expr */
#line 777
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value << yypvt[0].e.value;
                  else yyval.e.value = 0;
                
} break;

case YYr149: {	/* expr :  expr SHIFT_RIGHT expr */
#line 782
 if ((yyval.e.defined = yypvt[-2].e.defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].e.value >> yypvt[0].e.value;
                  else yyval.e.value = 0;
                
} break;

case YYr150: {	/* expr :  '-' expr */
#line 787
 if ((yyval.e.defined = yypvt[0].e.defined) == 1)
                      yyval.e.value = -yypvt[0].e.value;
                  else yyval.e.value = 0;
                
} break;

case YYr151: {	/* expr :  '+' expr */
#line 792
 if ((yyval.e.defined = yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[0].e.value;
                  else yyval.e.value = 0;
                
} break;

case YYr152: {	/* expr :  '~' expr */
#line 797
 if ((yyval.e.defined = yypvt[0].e.defined) == 1)
                      yyval.e.value = ~yypvt[0].e.value;
                  else yyval.e.value = 0;
                
} break;

case YYr153: {	/* expr :  IDENTIFIER */
#line 802
 yyval.e.defined = yypvt[0].s->defined;
                  yyval.e.value = yypvt[0].s->value;
                
} break;

case YYr154: {	/* expr :  IDENT_ABS */
#line 807
 yyval.e.defined = yypvt[0].s->defined;
                  yyval.e.value = yypvt[0].s->value;
                
} break;

case YYr155: {	/* expr :  LABEL_ABS */
#line 811
 yyval.e.defined = yypvt[0].s->defined;
                  yyval.e.value = yypvt[0].s->value;
                
} break;

case YYr156: {	/* expr :  INTEGER */
#line 815
 yyval.e.defined = 1;
                  yyval.e.value = yypvt[0].i;
                
} break;

case YYr157: {	/* expr :  PC */
#line 819
 yyval.e.defined = 1;
 Opcode.ispc[Opcode.f_no] = 1;
                  yyval.e.value = pc;
                
} break;

case YYr158: {	/* expr :  rel_expr */
#line 823
 yyval.e = yypvt[0].e; 
} break;

case YYr159: {	/* abs_expr :  expr */
#line 827
 
                  yyval.e = yypvt[0].e; 
                  /* yyval.e.sym = NULL; PAS */
                
} break;

case YYr160: {	/* rel_expr :  IDENT_REL */
#line 835
 if ((yyval.e.defined = yypvt[0].s->defined) == 1)
                      yyval.e.value = yypvt[0].s->value;
                  else yyval.e.value = 0;
                  yyval.e.sym = yypvt[0].s;
                
} break;

case YYr161: {	/* rel_expr :  IDENT_REL '+' expr */
#line 841
 if ((yyval.e.defined = yypvt[-2].s->defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].s->value + yypvt[0].e.value;

                  else yyval.e.value = 0;
                  yyval.e.sym = yypvt[-2].s;
                
} break;

case YYr162: {	/* rel_expr :  IDENT_REL '-' expr */
#line 848
 if ((yyval.e.defined = yypvt[-2].s->defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].s->value - yypvt[0].e.value;
                  else yyval.e.value = 0;
                  yyval.e.sym = yypvt[-2].s;
                
} break;

case YYr163: {	/* rel_expr :  LABEL_REL */
#line 854
 if ((yyval.e.defined = yypvt[0].s->defined) == 1)
                      yyval.e.value = yypvt[0].s->value;
                  else yyval.e.value = 0;
                  yyval.e.sym = yypvt[0].s;
                
} break;

case YYr164: {	/* rel_expr :  LABEL_REL '+' expr */
#line 860
 if ((yyval.e.defined = yypvt[-2].s->defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].s->value + yypvt[0].e.value;
                  else yyval.e.value = 0;
	  if(!(yyval.e.sym = yypvt[-2].s))
		yyval.e.sym = yypvt[-2].e.sym;
} break;

case YYr165: {	/* rel_expr :  LABEL_REL '-' expr */
#line 866
 if ((yyval.e.defined = yypvt[-2].s->defined && yypvt[0].e.defined) == 1)
                      yyval.e.value = yypvt[-2].s->value - yypvt[0].e.value;
                  else yyval.e.value = 0;
                  yyval.e.sym = yypvt[-2].s;
                
} break;
#line 238 "/etc/yyparse.c"
	case YYrACCEPT:
		YYACCEPT;
	case YYrERROR:
		goto yyError;
#ifdef YYLR2
	case YYrLR2:
#ifndef YYSYNC
		YYREAD;
#endif
		yyj = 0;
		while(yylr2[yyj] >= 0) {
			if(yylr2[yyj] == yystate && yylr2[yyj+1] == yychar
			&& yylook(yys+1,yyps,yystate,yychar,yy2lex(),yylr2[yyj+2]))
					break;
			yyj += 3;
		}
		if(yylr2[yyj] < 0)
			goto yyError;
		if(yylr2[yyj+2] < 0) {
			yystate = ~ yylr2[yyj+2];
			goto yyStack;
		}
		yyi = yylr2[yyj+2];
		goto yyReduce;
#endif
	}

	/*
	 *	Look up next state in goto table.
	 */

	yyp = &yygo[yypgo[yyi]];
	yyq = yyp++;
	yyi = *yyps;
#if	0&&defined(__TURBOC__) && __SMALL__
	/* THIS ONLY WORKS ON TURBO C 1.5 !!! */
	/* yyi is in di, yyp is in si */
L02:
	asm lodsw		/* ax = *yyp++; */
	asm cmp yyi, ax
	asm jl L02
#else
	while (yyi < *yyp++)
		;
#endif
	yystate = ~(yyi == *--yyp? YYQYYP: *yyq);
	goto yyStack;

yyerrlabel:	;		/* come here from YYERROR	*/
/*
#pragma used yyerrlabel
 */
	yyerrflag = 1;
	yyps--, yypv--;
	
yyError:
	switch (yyerrflag) {

	case 0:		/* new error */
		yynerrs++;
		yyi = yychar;
		yyerror("Syntax error");
		if (yyi != yychar) {
			/* user has changed the current token */
			/* try again */
			yyerrflag++;	/* avoid loops */
			goto yyEncore;
		}

	case 1:		/* partially recovered */
	case 2:
		yyerrflag = 3;	/* need 3 valid shifts to recover */
			
		/*
		 *	Pop states, looking for a
		 *	shift on `error'.
		 */

		for ( ; yyps > yys; yyps--, yypv--) {
			if (*yyps >= sizeof yypact/sizeof yypact[0])
				continue;
			yyp = &yyact[yypact[*yyps]];
			yyq = yyp;
			do
				;
			while (YYERRCODE < *yyp++);
			if (YYERRCODE == yyp[-1]) {
				yystate = ~YYQYYP;
				goto yyStack;
			}
				
			/* no shift in this state */
#if YYDEBUG
			if (yydebug && yyps > yys+1)
				printf("Error recovery pops state %d (%d), uncovers %d (%d)\n",
					yysmap[yyps[0]], yyps[0],
					yysmap[yyps[-1]], yyps[-1]);
#endif
			/* pop stacks; try again */
		}
		/* no shift on error - abort */
		break;

	case 3:
		/*
		 *	Erroneous token after
		 *	an error - discard it.
		 */

		if (yychar == 0)  /* but not EOF */
			break;
#if YYDEBUG
		if (yydebug)
			printf("Error recovery discards %s (%d), ",
				yyptok(yychar), yychar);
#endif
		yyclearin;
		goto yyEncore;	/* try again in same state */
	}
	YYABORT;

#ifdef YYALLOC
yyReturn:
	yylval = save_yylval;
	yyval = save_yyval;
	yypvt = save_yypvt;
	yychar = save_yychar;
	yyerrflag = save_yyerrflag;
	yynerrs = save_yynerrs;
	free((char *)yys);
	free((char *)yyv);
	return(retval);
#endif
}

#ifdef YYLR2
yylook(s,rsp,state,c1,c2,i)
short *s;		/* stack		*/
short *rsp;		/* real top of stack	*/
int state;		/* current state	*/
int c1;			/* current char		*/
int c2;			/* next char		*/
int i;			/* action S < 0, R >= 0	*/
{
	int j;
	short *p,*q;
	short *sb,*st;
#if YYDEBUG
	if(yydebug) {
		printf("LR2 state %d (%d) char %s (%d) lookahead %s (%d)",
			yysmap[state],state,yyptok(c1),c1,yyptok(c2),c2);
		if(i > 0)
			printf("reduce %d (%d)\n", yyrmap[i], i);
		else
			printf("shift %d (%d)\n", yysmap[i], i);
	}
#endif
	st = sb = rsp+1;
	if(i >= 0)
		goto reduce;
  shift:
	state = ~i;
	c1 = c2;
	if(c1 < 0)
		return 1;
	c2 = -1;

  stack:
  	if(++st >= &s[YYSSIZE]) {
		yyerror("Parser Stack Overflow");
		return 0;
	}
	*st = state;
	if(state >= sizeof yypact/sizeof yypact[0])
		i = state- YYDELTA;
	else {
		p = &yyact[yypact[state]];
		q = p;
		i = c1;
		while(i < *p++)
			;
		if(i == p[-1]) {
			state = ~q[q-p];
			c1 = c2;
			if(c1 < 0)
				return 1;
			c2 = -1;
			goto stack;
		}
		if(state >= sizeof yydef/sizeof yydef[0])
			return 0;
		if((i = yydef[state]) < 0) {
			p = &yyex[~i];
			while((i = *p) >= 0 && i != c1)
				p += 2;
			i = p[1];
		}
	}
  reduce:
  	j = yyrlen[i];
	if(st-sb >= j)
		st -= j;
	else {
		rsp -= j+st-sb;
		st = sb;
	}
	switch(i) {
	case YYrERROR:
		return 0;
	case YYrACCEPT:
		return 1;
	case YYrLR2:
		j = 0;
		while(yylr2[j] >= 0) {
			if(yylr2[j] == state && yylr2[j+1] == c1)
				if((i = yylr2[j+2]) < 0)
					goto shift;
				else
					goto reduce;
		}
		return 0;
	}
	p = &yygo[yypgo[i]];
	q = p++;
	i = st==sb ? *rsp : *st;
	while(i < *p++);
	state = ~(i == *--p? q[q-p]: *q);
	goto stack;
}
#endif
		
#if YYDEBUG
	
/*
 *	Print a token legibly.
 *	This won't work if you roll your own token numbers,
 *	but I've found it useful.
 */
char *
yyptok(i)
{
	static char	buf[10];

	if (i >= YYERRCODE)
		return yystoken[i-YYERRCODE];
	if (i < 0)
		return "";
	if (i == 0)
		return "$end";
	if (i < ' ')
		sprintf(buf, "'^%c'", i+'@');
	else
		sprintf(buf, "'%c'", i);
	return buf;
}
#endif
#if YYDEBUG
char * yystoken[] = {
	"error",
	"MOVE",
	"JUMP",
	"NOP",
	"CALL",
	"RETURN",
	"INT",
	"SELECT",
	"RESELECT",
	"DISCONNECT",
	"SET",
	"CLEAR",
	"PTR",
	"WITH",
	"WHEN",
	"IF",
	"AND",
	"NOT",
	"OR",
	"MASK",
	"WAIT",
	"SHIFT_LEFT",
	"SHIFT_RIGHT",
	"EOL",
	"ABSOLUTE",
	"RELATIVE",
	"PC",
	"ORIGIN",
	"MEMORY",
	"REG",
	"TO",
	"REL",
	"FROM",
	"DUP",
	"ALIGN",
	"GOOD_PARITY",
	"BAD_PARITY",
	"EXTERN",
	"EXTERNAL",
	"ENTRY",
	"PHASE",
	"ACK",
	"ATN",
	"TARGET",
	"INTEGER",
	"DATA",
	"REGISTER",
	"IDENTIFIER",
	"PASS",
	"PROC",
	"ARRAYNAME",
	"PASSIFIER",
	"IDENT_ABS",
	"IDENT_EXT",
	"IDENT_REL",
	"LABEL_ABS",
	"LABEL_REL",
	"UNARY",
	0
};
char * yysvar[] = {
	"$accept",
	"script",
	"expr",
	"abs_expr",
	"rel_expr",
	"reg",
	"register",
	"reg_op",
	"orig",
	"data_pseudo",
	"any_ident",
	"absolute",
	"abs",
	"relative",
	"rel",
	"external",
	"ext",
	"label",
	"pass_ext",
	"pass_word",
	"line",
	"instruction",
	"pseudo_op",
	"move",
	"io",
	"transfer",
	"mem_move",
	"count24",
	"dest",
	"address32",
	"sel_io",
	"disc_io",
	"wait_io",
	"set_io",
	"clear_io",
	"reg_io",
	"id",
	"rel_addr32",
	"address24",
	"bits",
	"bit",
	"data8",
	"tran_op",
	"wait",
	"p_cond",
	"test",
	"entry",
	"data_list",
	"expr_list",
	"external_val",
	0
};
short yyrmap[] = {
	 166,  167,  168,   31,   33,   37,   39,   57,   59,   64, 
	  78,   79,   80,   81,   93,   94,  102,  103,  116,  118, 
	 120,  121,  123,  132,  138,  140,  141,  142,  143,  144, 
	 145,  146,  147,  159,  160,  161,  162,  163,  164,  165, 
	 158,  157,  156,  155,  154,  153,  152,  151,  150,  149, 
	 148,  139,  136,  134,  131,  129,  127,  126,  125,  122, 
	 119,  115,  114,  113,  112,  111,  110,  109,  108,  107, 
	 106,  105,  104,  101,  100,   92,   91,   90,   89,   88, 
	  87,   86,   85,   83,   82,   76,   75,   74,   73,   67, 
	  65,   63,   62,   61,   60,   54,   53,   52,   51,   50, 
	  49,   48,   47,   46,   45,   44,   43,   30,   28,   27, 
	  26,   23,   22,   21,   20,   19,   18,   16,   15,   14, 
	  13,   12,   11,   10,    9,    8,    7,    6,    5,    3, 
	   2,    1,    4,   17,   24,   25,   29,   32,   34,   35, 
	  36,   38,   40,   41,   42,   66,   68,   69,   70,   71, 
	  72,   77,   84,   95,   96,   97,   98,   99,  117,  124, 
	 128,  130,  133,  135,  137,   58,   56,   55,    0
};
short yysmap[] = {
	   1,   27,   28,   29,   30,   31,   36,   40,   59,   60, 
	  71,   72,   74,   96,  102,  116,  117,  120,  129,  130, 
	 163,  164,  178,  187,  208,  209,  210,  211,  215,  216, 
	 217,  218,  219,  220,  221,  222,  224,  225,  226,  229, 
	 234,  243,  252,  259,  283,  282,  281,  276,  271,  269, 
	 268,  267,  266,  264,  261,  257,  256,  253,  251,  249, 
	 248,  246,  244,  242,  240,  239,  238,  236,  233,  232, 
	 231,  230,  227,  223,  207,  206,  205,  204,  199,  198, 
	 197,  194,  193,  192,  191,  190,  189,  180,  179,  177, 
	 172,  169,  162,  161,  160,  159,  158,  157,  156,  155, 
	 154,  153,  152,  151,  150,  149,  148,  144,  143,  142, 
	 141,  137,  136,  135,  134,  132,  131,  128,  127,  126, 
	 125,  124,  119,  118,  111,  110,   98,   95,   94,   93, 
	  92,   90,   89,   88,   86,   83,   82,   70,   69,   68, 
	  67,   57,   56,   54,   49,   42,   41,   39,   38,   37, 
	  35,   34,   32,   17,   16,   15,    9,    8,    7,    5, 
	   4,    3,    2,   61,   62,   63,   64,   65,   66,  145, 
	 146,  147,  213,  214,  212,  171,  123,  188,  196,  254, 
	  99,  100,  103,  272,   75,   85,   87,  165,   76,   77, 
	  78,    6,   80,  166,   79,   81,  167,   84,  168,   91, 
	  18,   19,   20,   21,   22,   23,   25,   26,  228,  173, 
	 174,  175,  108,  104,  105,   33,  106,  107,  200,  201, 
	 202,  203,  265,  133,  277,  245,  247,  112,  113,  114, 
	 260,  275,  262,  263,   44,   45,   46,   47,   48,  270, 
	 278,  279,  280,  285,  286,  287,   50,   51,   52,   53, 
	 138,   55,  140,   58,    0,  139,  250,   43,  241,  237, 
	 121,  185,  184,  183,  182,  181,  115,  235,  109,  176, 
	 258,  274,  255,  273,  284,   24,   14,   13,   12,   11, 
	  10,   73,  101,   97,  195,  186,  122,  170
};
int yyntoken = 73, yynvar = 50, yynstate = 288, yynrule = 169;
#endif
