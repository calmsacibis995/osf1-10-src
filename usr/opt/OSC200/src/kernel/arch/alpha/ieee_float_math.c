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
static char *rcsid = "@(#)$RCSfile: ieee_float_math.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/06/14 17:39:01 $";
#endif

/*****************************************************************************/
/*FILE: IEEE_FLOAT_MATH.C - IEEE floating point routines		     */
/*****************************************************************************
! REVISION HISTORY:
! Who	When		What
!---------------------------------------------------------------
! HA	27-Jul-1992	detection of NAN on S, and 0+0=0 (-Inf rounding)
! HA	24-May-1992	Sign of zero on Round toward -Infinty
! HA	08-May-1992	EV5: calculates correct INE on DIV
! HA	12-Mar-1992	fix bugs in +/- Infinity
! HA	12-Feb-1992	Add +/- Infinity rounding
! HA	04-Sep-1991	remove support for pre SRM4.0
! HA	14-May-1991	DIV do not set the INE in FPCR
! HA	01-May-1991	Uninitialize variable (retval)
! HA	30-Apr-1991	Do not report traps on F31 as destination
! HA	24-Apr-1991	Fix the bug in CMPTUN
! HA	29-Mar-1991	Fix a bug in on CVTTQ
! HA	07-Mar-1991	Fix a bug in extend_s() and make_s()
! SJM	01-NOV-1989	bug fixes
! ACP	23-Oct-1989	First pass

The orginal code did not have any comments. I have created many comments as I
fix the bugs in the code.  My comments are based on my observation and 
interpretation of the code.  If the orginal author would have spend a few 
minutes to comment the code, we would never had a problem of misinterpretation.
								  -HA 
 *****************************************************************************/

#include "machine/mathinterface.h"
#include <sys/types.h>

#ifndef KERNEL
#include <stdio.h>
#include <string.h>
#include "port.h"
#include "struct.h"

/*global union    INSTR_FORMAT *ppc;*/	      /* Because of treatment of F31 */
global struct ALPHA_I ppc;	      /* Because of treatment of F31 */
global unsigned int	     enable_dest_r31; /* Because of treatment of F31 */

#endif

#define und_enable(f)	(f&0x100)
#define iov_enable(f)	(f&0x100)
#define swc_enable(f)	(f&0x400)
#define ine_enable(f)	(f&0x200)
#define ROUND_NEAR	2
#define ROUND_ZERO	0
#define ROUND_MINF	1
#define ROUND_PINF	3
#define round_mode(f)	((f>>6)&0x3)
#define STICKY_S	0x20000000	/* both in longword 0 of fraction */
#define STICKY_T	1

/*****************************************************************************/
int CMP128(a, b)
unsigned int a[], b[];
{
int i, temp;

 temp = 0;
 for(i=3; i >= 0; --i) 
  {
   if(a[i] < b[i])  temp = -1;
   if(a[i] > b[i])  temp = 1;
   if(temp)	    break;
  };
 return temp;
}

/******************************************************************************
   3 3      2 2
   1 0      3 2			       0
  +-------------------------------------+
  |s|   exp  |       fraction		|   S-Type
  +-------------------------------------+
 Extended
    f[1]<23>    hidden bit (1.xx)
    f[1]<22:0>  Regfile<51:29>

   6 6           5 5         
   3 2           2 1	
  +-------------------------------------------------------------+
  |s|  exp        |            frac | tion		        | T-Type
  +-------------------------------------------------------------+
   3 3           2 1               0 3
   1 0           0 9               0 1
 Extended 
    f[1]<23>    hidden bit (1.xx)
    f[1]<22:0>  Regfile<51:29>	    a[1]<19:0>|a[0]<31:29>
    f[0]<31:3>  Regfile<28:0>	    a[0]<28:0>
    f[0]<2:0>   ------------	    LRG bits????
******************************************************************************/
struct double_extended_ {
    short e;		/* 16 bit SIGNED exponent */
    unsigned int f[4];		/* 64 (max) bit fraction */
    int s;			/* 1 bit sign (0 for +, 1 for -) */
};
typedef struct double_extended_ EXTENDED;

EXTENDED exttwo = { 1, { 0, 0x800000, 0, 0 }, 0 };
EXTENDED extszero = { -127, { 0, 0, 0, 0 }, 0 };
EXTENDED exttzero = { -1023, { 0, 0, 0, 0 }, 0 };

#define NORMAL		0
#define ZERO		1
#define INFINITY	2
#define NaN		3
#define DENORM		4

/****************************************************************************
	SRM 4.0 CODE
	NEW IEEE routines.  For common S & T register format.
*****************************************************************************/
int extend_s(a, b)
unsigned int a[];
EXTENDED *b;
{
short old_exp;
 b->s = a[1]>>31;
 old_exp = (a[1] >> 20) & 0x7F;	/* get lower 7 bits of exponent */
 if(a[1] & 0x40000000)		/* if upper bit of 11 bit exponent is set, 
then... */
   old_exp = old_exp | 0x80;	/* set upper bit of eight bit exponent */
 b->e = old_exp - 127;
 b->f[3] = b->f[2] = 0;
/* S-type fraction is in Regfile<51:29> (HA) */
 b->f[1] = ((a[1] & 0xFFFFF) << 3) | (a[0] >> 29);
 b->f[0] = a[0] << 3;

 if(b->e == -127) 
  {			/* zero/denormalized number */
   if((b->f[1] == 0) && (b->f[0]==0)) return ZERO;	/* it's a zero.	*/
   return DENORM;			/* it's denormalized. */
  };

 if(b->e == 128)
  {			/* infinity/NaN */
   if((b->f[1] == 0) && (b->f[0]==0)) return INFINITY;	/* it's an infinity */
   return NaN;			/* it's Not a Number */
  };

 b->f[1] |= (int)1 << 23;
 return NORMAL;			  /* it's a normal number w/ leading bit = 1 */
}
/*****************************************************************************
		    a[1]:a[0] is 64 bit register format of data
    f[3]:f[2]:f[1]:f[0] holds fraction
    f[1]<23>   is the hidden bit 1.xxx	
    f[1]<22:0> is a[1]<19:0> ! a[0]<31:29>	    Top half of the fraction
    f[0]<31:0> is a[0]<28:0> ! LRG bits (I guess)   Low half of the fraction
After CVT:
    f[1]<22>   is the Round Bit
 *****************************************************************************/
int extend_t(a,b)
unsigned int a[];
EXTENDED *b;
{
 b->s = a[1]>>31;
 b->e = ( (a[1]>>20) & 0x7FF ) - 1023;   /* Unbiased Exponent */
 b->f[3] = b->f[2] = 0;
 b->f[1] = ((a[1] & 0xFFFFF) << 3) | (a[0] >> 29);
 b->f[0] = a[0] << 3;

 if(b->e == -1023)				 /* zero/denormalized number */
  {			
   if((b->f[1] == 0) && (b->f[0]==0)) return ZERO;	/* it's a zero.	*/
   return DENORM;				/* it's denormalized. */
  };

 if(b->e == 1024)				 /* infinity/NaN */
  {			
   if((b->f[1] == 0) && (b->f[0]==0)) return INFINITY;	/* it's an infinity */
   return NaN;			/* it's Not a Number */
  };

 b->f[1] |= (int)1 << 23;
 return NORMAL;			  /* it's a normal number w/ leading bit = 1 */
}
/*****************************************************************************/
/* returns 0 if no exception happens					     */
/*****************************************************************************/
int make_s(a,b)
EXTENDED *a;
unsigned int b[];
{
int temp, hi_bit, frac, i;
unsigned int new_exp, mid_bits;

 normalize(a);
 temp = 0;
 for(i=0; i<4; ++i)
   if(a->f[i] != 0)
     break;
    
 if(i==4)
  {		/* it's a zero... */
   b[1] = ((unsigned int) a->s << 31);
   b[0] = 0;
   return temp;
  };

 if(a->e < -126)
  {
   a->s = 0;
   a->e = -127;
   for(i=0; i<4; ++i)    a->f[i] = 0;
   temp = UNF | INE;
  };

 if (a->e >= 128) temp = FOV | INE;

 new_exp  = ((unsigned int) a->e+127) & 0xFF;	/* make sure it is 8 bits */
 hi_bit   = new_exp >> 7;
 if(hi_bit)			/* if hi_bit is 1 */
   mid_bits = (new_exp == 0xFF) ? 0x7 : 0x0;	/* look at SRM4.0 Page2-10 */
 else			/* hi_bit is 0 */
   mid_bits = (new_exp == 0x00) ? 0x0 : 0x7;	/* look at SRM4.0 Page2-10 */

 new_exp =   (new_exp & 0x7F)		/* get low 7 bits */
           | (mid_bits<< 7)		/* the 3 bits in the RegFile */
           | (hi_bit << 10);		/* move bit 8 to bit 10 of RegFile */


 b[1] = ((unsigned int) a->s << 31) | (new_exp << 20);
 frac = a->f[1] & 0x7FFFFF;
 b[1] |= frac >> 3;
 b[0] = frac << 29;

 return temp;
}
/*****************************************************************************/
int make_t(a,b)
EXTENDED *a;
unsigned int b[];
{
    int temp, sticky,i;

    normalize(a);
    temp = 0;

    for (i=0;i<4;++i)
	if (a->f[i] != 0)
	    break;
    if (i == 4) {
	b[1] = ((unsigned int) a->s << 31);
	b[0] = 0;
	return temp;
    }

    if (a->e < -1022) {
	a->s = 0;
        a->e = -1023;
        for (i=0; i<4; ++i)
	    a->f[i] = 0;
	temp = UNF | INE;
    }
    if (a->e > 1023)
	temp = FOV | INE;

    b[1] = ((unsigned int) a->s << 31) | (((unsigned int) a->e+1023) << 20);
    b[1] |= (a->f[1] >> 3) & 0xFFFFF;
    b[0] = (a->f[1] << 29) | (a->f[0] >> 3);

    return temp;
}
/*****************************************************************************/
int round_s(a,b,mode)
EXTENDED *a;
unsigned int b[];
int mode;
{
unsigned int diff1[2],diff2[2],temp;
EXTENDED z1,z2;

 z1.s = z2.s = a->s;
 z1.e = z2.e = a->e;
 z1.f[0] = z2.f[0] = 0;
 z1.f[1] = z2.f[1] = a->f[1];
 z1.f[2] = z2.f[2] = 0;
 z1.f[3] = z2.f[3] = 0;

 if(a->f[0] != 0) z2.f[1] = z2.f[1] + 1;

 if((z1.f[0] == z2.f[0]) && (z1.f[1] == z2.f[1]) ) 
  {
   temp = make_s(&z1, b);
   if(temp==0) temp = MATH_NOTRAP;
  } 
 else 
  {
   switch(round_mode(mode)) 
    {
     case ROUND_NEAR:
	   SUB64(a->f, z1.f, diff1);
	   SUB64(z2.f, a->f, diff2);
	   /* if diff1<diff2 then z1 else z2 */
	   if(diff1[1] > diff2[1])      temp = make_s(&z2, b);
	   else if(diff2[1] > diff1[1])	temp = make_s(&z1, b);
	   else if(diff1[0] > diff2[0]) temp = make_s(&z2, b);
	   else if(diff2[0] > diff1[0]) temp = make_s(&z1, b);
	   else	/* equal distance */
		if (z1.f[1] & 1) temp = make_s(&z2, b);
	   else temp = make_s(&z1, b);
        break;
     case ROUND_ZERO: 
           temp = make_s(&z1, b);
        break;
     case ROUND_MINF:  /* if negative then pick the round up result */
	   if(a->s == 0) temp = make_s(&z1,b);
	   else		 temp = make_s(&z2,b);
	break;
     case ROUND_PINF: /* if negative then pick the chopped result */
	   if(a->s == 0) temp = make_s(&z2,b);
	   else		 temp = make_s(&z1,b);
	break;
     default: printf("IEEE: Unknown Rounding Mode!\n");
	break;
    };
   temp |= INE;					  /* if rounded then INExact */
  };
 return(temp);
}
/*****************************************************************************/
int round_t(a,b,mode)
EXTENDED *a;
unsigned int b[];
int mode;
{
static unsigned int bit_3[2] = { 0x8, 0 };
unsigned int diff1[2],diff2[2],temp;
EXTENDED z1,z2;

 z1.s = z2.s = a->s;
 z1.e = z2.e = a->e;
 z1.f[0] = z2.f[0] = a->f[0] & 0xFFFFFFF8;
 z1.f[1] = z2.f[1] = a->f[1];
 z1.f[2] = z2.f[2] = 0;
 z1.f[3] = z2.f[3] = 0;

 if (a->f[0] & 0x7) ADD64(z2.f,bit_3,z2.f);

 if((z1.f[0] == z2.f[0]) && (z1.f[1] == z2.f[1]) ) 
  {
   temp = make_t(&z1, b);
   if(temp==0) temp = MATH_NOTRAP;
  } 
 else 
  {
   switch(round_mode(mode))
    {
     case ROUND_NEAR:
	   SUB64(a->f, z1.f, diff1);
	   SUB64(z2.f, a->f, diff2);
	   if(diff1[1] > diff2[1]) temp = make_t(&z2, b);
	   else if(diff2[1] > diff1[1]) temp = make_t(&z1, b);
	   else if(diff1[0] > diff2[0]) temp = make_t(&z2, b);
	   else if(diff2[0] > diff1[0]) temp = make_t(&z1, b);
	   else	/* equal distance */
		if(z1.f[0] & 0x8) temp = make_t(&z2, b);
	   else temp = make_t(&z1, b);
        break;
     case ROUND_ZERO:
	   temp = make_t(&z1, b);
	break;
     case ROUND_MINF: /* if negative then pick the round up result */
	   if(a->s == 0) temp = make_t(&z1,b);
	   else		 temp = make_t(&z2,b);
	break;
     case ROUND_PINF: /* if negative then pick the chopped result */
	   if(a->s == 0) temp = make_t(&z2,b);
	   else		 temp = make_t(&z1,b);
	break;
     default: printf("IEEE: Unknown Rounding Mode!\n");
	break;
    };
   temp |= INE;					  /* if rounded then INExact */
  };
 return(temp);
}
/*****************************************************************************/
int normalize(a)
EXTENDED *a;
{
int temp,temp1,i;

 for(i=0; i<4; ++i)
   if(a->f[i]) break;
 if(i == 4) return MATH_NOTRAP;		 /* zero fraction, unnormalizable... */

 temp = a->f[1] & 0x00800000;

 if( ((a->f[1]&0xFF000000) != 0) || (a->f[2] != 0) || (a->f[3] != 0) ) 
  {
   while( ((a->f[1] & 0xFF800000) != 0x00800000) || 
           (a->f[2] != 0) || (a->f[3] != 0) ) 
    {
     (a->e)++;
     temp = 0;
     for(i=3; i>=0; --i) 
      {
       temp1 = a->f[i]&1;
       a->f[i] = (a->f[i] >> 1) | ((unsigned int)temp << 31);
       temp = temp1;
      };
     a->f[0] |= temp;	/* OR in S bit */
    };
   return MATH_NOTRAP;
  };

 if( (temp==0) && ( ((a->f[1]&0x7FFFFF) != 0) || (a->f[0] != 0) ) ) 
  {
   while((a->f[1] & 0x00800000) != 0x00800000) 
    {
     (a->e)--;
     temp = 0;
     for(i=0; i<4; ++i) 
      {
       temp1 = a->f[i]>>31;
       a->f[i] = (a->f[i] << 1) | temp;
       temp = temp1;
      };
    };
   return MATH_NOTRAP;
  };
}
/*****************************************************************************/
int make_return(f,tmp)
int f;
int tmp;
{
 int old;

 old = tmp;
 if (!ine_enable(f))	    tmp &= ~INE;
 if (!und_enable(f))	    tmp &= ~UNF;
 if (!iov_enable(f))	    tmp &= ~IOV;
 if (tmp == 0) 	/* no exceptions */
	tmp = MATH_NOTRAP;
 else if (swc_enable(f))
	tmp |= SWC;

#ifndef KERNEL
 /* if F31 is the destination and disabled then NEVER report a trap */
 if(((ppc.i)->op_format.rc == 31) && !enable_dest_r31) return(MATH_NOTRAP);
#endif

 return ((old >> SWC) << 16) | tmp;	/* return exceptions for FP_CTL in upper 
16 bits */
}
/*****************************************************************************/
int CVTTS(f,a,b)
int f;
unsigned int a[];
unsigned int b[];
{
    int a_type,retval;
    EXTENDED temp;

    b[1] = b[0] = 0;
    a_type = extend_t(a, &temp);
    if ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM))
	return make_return(f,INV);

    retval = round_s(&temp, b, f);

    return make_return(f,retval);
}
/*****************************************************************************/
int CVTQS(f,a,b)
int f;
unsigned int a[];
unsigned int b[];
{
    unsigned int temp[2];
    int retval;
    static unsigned int one[2] = { 1, 0 };
    EXTENDED op_b;

    b[1] = b[0] = retval = 0;
    if (a[1] & 0x80000000) {
	op_b.s = 1;
	temp[0] = ~a[0];
	temp[1] = ~a[1];
	ADD64(temp,one,temp);
    } else {
	temp[0] = a[0];
	temp[1] = a[1];
	op_b.s = 0;
    }
    op_b.e = 0;
    op_b.f[0] = 0;
    op_b.f[1] = temp[0] << 23;
    op_b.f[2] = (temp[0] >> 9) | (temp[1] << 23);
    op_b.f[3] = temp[1] >> 9;

    normalize(&op_b);

    retval = round_s(&op_b, b, f);	

    return make_return(f,retval);
}
/*****************************************************************************/
int CVTTQ(f,a,b)
int f;
unsigned int a[];
unsigned int b[];
{
 int a_type, retval, uv;
 unsigned int midway;
 static unsigned int one[2] = { 1, 0 };
 EXTENDED temp;

 b[1] = b[0] = retval = 0;
 a_type = extend_t(a, &temp);
 if ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM))
	return make_return(f,INV);

 if (temp.e > 0)	/* Unbiased Exponent positive none zero */
  {
   while (temp.e > 0) 
    {
     --(temp.e);
     temp.f[3] = (temp.f[3] << 1) | (temp.f[2] >> 31);
     temp.f[2] = (temp.f[2] << 1) | (temp.f[1] >> 31);
     temp.f[1] = (temp.f[1] << 1) | (temp.f[0] >> 31);
     temp.f[0] = (temp.f[0] << 1);
    };
  };

 if (temp.e < 0)     /* Unbiased Exponent negative none zero */
  {
   while (temp.e < 0) 
    {
     ++(temp.e);
     uv = temp.f[0] & 1;		/* save sticky value */
     temp.f[0] = (temp.f[0] >> 1) | (temp.f[1] << 31) | uv;
     temp.f[1] = (temp.f[1] >> 1) | (temp.f[2] << 31);
     temp.f[2] = (temp.f[2] >> 1) | (temp.f[3] << 31);
     temp.f[3] = (temp.f[3] >> 1);
    };
  };
/* f[3]<22:0>!f[2]<31:0>!f[1]<31:23> */
/* f[3]<22> should be the sign bit result<63> */
 b[0] = (temp.f[1] >> 23) | (temp.f[2] << 9);
 b[1] = (temp.f[2] >> 23) | (temp.f[3] << 9);
 switch (round_mode(f))
  {
   case ROUND_NEAR:
        if (temp.f[1] & 0x00400000)
         {
          midway = ((temp.f[1] & 0x003FFFFF)==0) && (temp.f[0] == 0);
	  if ((midway && (temp.f[1] & 0x00800000)) || !midway)
	        ADD64(b,one,b);
         };
	break;
   case ROUND_ZERO:
	/* no action needed */
	break;
   case ROUND_MINF:  /* if negative then pick the round up result */
	if(temp.s == 1) /* NEG takes round up*/
	 { 
          if(((temp.f[1] & 0x007FFFFF) != 0) || (temp.f[0] != 0)) /* INExact*/
	    ADD64(b,one,b);
	 };
	break;
   case ROUND_PINF: /* if negative then pick the chopped result */
	if(temp.s == 0) /* POS takes round up*/
	 { 
          if(((temp.f[1] & 0x007FFFFF) != 0) || (temp.f[0] != 0)) /* INExact*/
	    ADD64(b,one,b);
	 };
	break;      
   default: printf("IEEE: Unknown Rounding Mode!\n");
	break;
  };
 if (((temp.f[1] & 0x007FFFFF) != 0) || (temp.f[0] != 0))
	retval |= INE;

 /* IOV detection logic */
 a_type = extend_t(a, &temp);
 if(temp.s == 0)	/* Positive input */
  {		
   if(temp.e >= 63) retval = IOV | INE;
  }
 else			/* Negative Input */
  {	
   b[0] = ~b[0];	/* create 2's complement */
   b[1] = ~b[1];
   ADD64(b,one,b);

   if(temp.e >= 64) retval = IOV | INE;
   if(temp.e == 63)
    {
     if(((b[1] >> 31) & 0x1) == 0)
       retval = IOV | INE;
    };
  };
 return make_return(f,retval);
}
/*****************************************************************************/
int CVTQT(f,a,b)
int f;
unsigned int a[];
unsigned int b[];
{
    unsigned int temp[2];
    int retval;
    static unsigned int one[2] = { 1, 0 };
    EXTENDED op_b;

    b[1] = b[0] = 0;
    if (a[1] & 0x80000000) {
	op_b.s = 1;
	temp[0] = ~a[0];
	temp[1] = ~a[1];
	ADD64(temp,one,temp);
    } else {
	temp[0] = a[0];
	temp[1] = a[1];
	op_b.s = 0;
    }
    op_b.e = 0;
    op_b.f[0] = 0;
    op_b.f[1] = temp[0] << 23;
    op_b.f[2] = (temp[0] >> 9) | (temp[1] << 23);
    op_b.f[3] = temp[1] >> 9;

    normalize(&op_b);

    retval = round_t(&op_b, b, f);

    return make_return(f,retval);
}
/*****************************************************************************/
int CMPTEQ(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_t(a, &op_a);
    b_type = extend_t(b, &op_b);
    if ( ((a_type == NaN) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);

    if ((a_type != INFINITY) && (b_type != INFINITY)) {
	if ( ((op_a.e == op_b.e) && (CMP128(op_a.f,op_b.f) == 0) && (op_a.s == 
op_b.s)) ||
	     ((a_type == ZERO) && (b_type == ZERO)) )
	    retval = round_s(&exttwo, c, f);
	else
            retval = round_s(&exttzero, c, f);
    } else {					/* one/both operands are 
infinity */
	if ((a_type == INFINITY) && (b_type == INFINITY) && (op_a.s == op_b.s))
		retval = round_s(&exttwo,c,f);	/* == if both inf, and signs 
equal */
	    else
		retval = round_s(&exttzero,c,f);
    }

    return make_return(f,retval);
}

int CMPTLT(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_t(a, &op_a);
    b_type = extend_t(b, &op_b);
    if ( ((a_type == NaN) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);

    if ((a_type != INFINITY) && (b_type != INFINITY)) {
        if ( ((op_a.s == 1) && (op_b.s == 0) && 
	      ((a_type != ZERO) || (b_type != ZERO))) ||
	     ((op_a.s == 1) && (op_b.s == 1) &&
	      ( (op_a.e > op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) > 0) ) )) ||
	     ((op_a.s == 0) && (op_b.s == 0) &&
	      ( (op_a.e < op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) < 0) ) ))
           )
	    retval = round_s(&exttwo, c, f);
        else
            retval = round_s(&exttzero, c, f);
    } else {					/* one/both operands are 
infinity */
	if ( ((a_type == INFINITY) && op_a.s && !((b_type == INFINITY) && 
op_b.s)) || /* if a = - inf and b != -inf, then a < b */
	     ((a_type != INFINITY) && (b_type == INFINITY) && !op_b.s)		 
     /* if a not inf and b = +inf, then a < b */
	   )
		retval = round_s(&exttwo,c,f);	
	    else
		retval = round_s(&exttzero,c,f);
    }

    return make_return(f,retval);
}

int CMPTLE(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_t(a, &op_a);
    b_type = extend_t(b, &op_b);
    if ( ((a_type == NaN) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);


    if ((a_type != INFINITY) && (b_type != INFINITY)) {
        if ( ((a_type == ZERO) && (b_type == ZERO)) ||
	     ((op_a.s == 1) && (op_b.s == 0)) ||
	     ((op_a.s == 1) && (op_b.s == 1) &&
	      ( (op_a.e > op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) >= 0) ) )) ||
	     ((op_a.s == 0) && (op_b.s == 0) &&
	      ( (op_a.e < op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) <= 0) ) ))
           )
	    retval = round_s(&exttwo, c, f);
        else
             retval = round_s(&exttzero, c, f);
    } else {
	if ( ((a_type == INFINITY) && op_a.s) ||		/* if a = - inf 
then a <= b */
	     ((b_type == INFINITY) && !op_b.s)			/* if b = +inf, 
then a <= b */
	   )
		retval = round_s(&exttwo,c,f);	
	    else
		retval = round_s(&exttzero,c,f);
    }

    return make_return(f,retval);
}

int CMPTUN(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
int a_type, b_type, c_type, retval;
EXTENDED op_a, op_b;

 c[1] = c[0] = 0;
 a_type = extend_t(a, &op_a);
 b_type = extend_t(b, &op_b);
 if ( ((a_type == NaN) || (a_type == DENORM)) ||
      ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);

 retval = round_t(&exttzero, c, f);

 return make_return(f,retval);
}
/*****************************************************************************/
/* DIV_KERNEL:								     */
/* In the beginning "" created DIV128, But DIV128 did not keep track of      */
/* sticky.  So some INE cases failed.  So DIV_Kernal was created from DIV128 */
/*****************************************************************************/
int div_kernel(Signed,a,b,c)
int Signed,*a,*b,*c;
{
int tt[8];
int pp[8];
int aa[8];
int bb[8];
int i,ov;
int sign = 0;
    
 c[0] = c[1] = c[2] = c[3] = 0;
    /* quick check for div by zero */
 if((b[0] == 0) && (b[1] == 0) && (b[2] == 0) && (b[3] == 0)) return(0);

 aa[0] = a[0];
 aa[1] = a[1];
 aa[2] = a[2];
 aa[3] = a[3];
 aa[4] = 0;
 aa[5] = 0;
 aa[6] = 0;
 aa[7] = 0;

 bb[0] = 0;
 bb[1] = 0;
 bb[2] = 0;
 bb[3] = 0;
 bb[4] = b[0];
 bb[5] = b[1];
 bb[6] = b[2];
 bb[7] = b[3];

    /* if numbers signed, make them positive and remember the sign's */
 if(Signed == 1)
  {
   tt[0] = 1;
   tt[1] = 0;
   tt[2] = 0;
   tt[3] = 0;

   if(aa[3] < 0)
    {
     sign++;
     aa[0] = aa[0] ^ 0xFFFFFFFF;
     aa[1] = aa[1] ^ 0xFFFFFFFF;
     aa[2] = aa[2] ^ 0xFFFFFFFF;
     aa[3] = aa[3] ^ 0xFFFFFFFF;
     ADD128(aa,tt,aa);
    };
   if(bb[7] < 0)
    {
     sign--;
     bb[4] = bb[4] ^ 0xFFFFFFFF;
     bb[5] = bb[5] ^ 0xFFFFFFFF;
     bb[6] = bb[6] ^ 0xFFFFFFFF;
     bb[7] = bb[7] ^ 0xFFFFFFFF;
     ADD128(&bb[4],tt,&bb[4]);
    };
  };

    /* shift b down by one to start */
 bb[0] = ((bb[0]>>1) & 0x7FFFFFFF) | bb[1]<<31;
 bb[1] = ((bb[1]>>1) & 0x7FFFFFFF) | bb[2]<<31;
 bb[2] = ((bb[2]>>1) & 0x7FFFFFFF) | bb[3]<<31;
 bb[3] = ((bb[3]>>1) & 0x7FFFFFFF) | bb[4]<<31;
 bb[4] = ((bb[4]>>1) & 0x7FFFFFFF) | bb[5]<<31;
 bb[5] = ((bb[5]>>1) & 0x7FFFFFFF) | bb[6]<<31;
 bb[6] = ((bb[6]>>1) & 0x7FFFFFFF) | bb[7]<<31;
 bb[7] = ((bb[7]>>1) & 0x7FFFFFFF);

 /* start with result = 0 */
 pp[0] = pp[1] = pp[2] = pp[3] = pp[4] = pp[5] = pp[6] = pp[7] = 0;

 for(i = 127; i >= 0; i--)
  {
   ov = SUB256(aa,bb,tt);
/**
   printf("inner loop info (%d)	aa = %08x%08x %08x%08x %08x%08x %08x%08x \n		
	bb = %08x%08x %08x%08x %08x%08x %08x%08x \n",
	    i,aa[7],aa[6],aa[5],aa[4],aa[3],aa[2],aa[1],aa[0],
	    bb[7],bb[6],bb[5],bb[4],bb[3],bb[2],bb[1],bb[0]);
   printf("			tt = %08x%08x %08x%08x %08x%08x %08x%08x \n		
	pp = %08x%08x %08x%08x %08x%08x %08x%08x ov= %0x\n",
	    tt[7],tt[6],tt[5],tt[4],tt[3],tt[2],tt[1],tt[0],
	    pp[7],pp[6],pp[5],pp[4],pp[3],pp[2],pp[1],pp[0],ov);
**/
   if((ov & (1u<<31)) == 0)
    { /* No borrow case or b=0,inject a one, and use result */
     pp[i>>5] |= 1<<(i & 0x1F);
     aa[0] = tt[0];
     aa[1] = tt[1];
     aa[2] = tt[2];
     aa[3] = tt[3];
     aa[4] = tt[4];
     aa[5] = tt[5];
     aa[6] = tt[6];
     aa[7] = tt[7];
    };
  /* next time shift by b>>1 */
   bb[0] = ((bb[0]>>1) & 0x7FFFFFFF) | bb[1]<<31;
   bb[1] = ((bb[1]>>1) & 0x7FFFFFFF) | bb[2]<<31;
   bb[2] = ((bb[2]>>1) & 0x7FFFFFFF) | bb[3]<<31;
   bb[3] = ((bb[3]>>1) & 0x7FFFFFFF) | bb[4]<<31;
   bb[4] = ((bb[4]>>1) & 0x7FFFFFFF) | bb[5]<<31;
   bb[5] = ((bb[5]>>1) & 0x7FFFFFFF) | bb[6]<<31;
   bb[6] = ((bb[6]>>1) & 0x7FFFFFFF) | bb[7]<<31;
   bb[7] = ((bb[7]>>1) & 0x7FFFFFFF);
  };

    /* all done, now fix up the sing for signed arith */
 if(sign != 0)
  {
   pp[0] = pp[0] ^ 0xFFFFFFFF;
   pp[1] = pp[1] ^ 0xFFFFFFFF;
   pp[2] = pp[2] ^ 0xFFFFFFFF;
   pp[3] = pp[3] ^ 0xFFFFFFFF;
   tt[0] = 1;
   tt[1] = 0;
   tt[2] = 0;
   tt[3] = 0;
   ADD128(pp,tt,pp);
  };
 c[0] = pp[0];
 c[1] = pp[1];
 c[2] = pp[2];
 c[3] = pp[3];
 if(aa[0] != 0 || aa[1] != 0 || aa[2] != 0 || aa[3] != 0)
    c[0] |= STICKY_T;  /* fix by observation*/
 return(1);
}
/*****************************************************************************/
int add_kernel(op_a,op_b,op_c)
EXTENDED *op_a;
EXTENDED *op_b;
EXTENDED *op_c;
{
    int i,sticky;
    unsigned int tmp[2];

    tmp[0] = 1;
    tmp[1] = 0;
    if (op_a->e < op_b->e) {
	while (op_a->e < op_b->e) {
	    (op_a->e)++;
	    sticky = op_a->f[0] & 1;
	    op_a->f[0] = (op_a->f[0] >> 1) | (op_a->f[1] << 31) | sticky;
	    op_a->f[1] = op_a->f[1] >> 1;
	}
    }
    if (op_b->e < op_a->e) {
	while (op_b->e < op_a->e) {
	    (op_b->e)++;
	    sticky = op_b->f[0] & 1;
	    op_b->f[0] = (op_b->f[0] >> 1) | (op_b->f[1] << 31) | sticky;
	    op_b->f[1] = op_b->f[1] >> 1;
	}
    }

    op_c->e = op_a->e;
    for (i=0; i<4; ++i)
	op_c->f[i] = 0;

    if (op_a->s) {
	op_a->f[0] = ~op_a->f[0];
	op_a->f[1] = ~op_a->f[1];
	ADD64(op_a->f, tmp, op_a->f);
    }

    if (op_b->s) {
	op_b->f[0] = ~op_b->f[0];
	op_b->f[1] = ~op_b->f[1];
	ADD64(op_b->f, tmp, op_b->f);
    }

    ADD64(op_a->f, op_b->f, op_c->f);

    if (op_c->f[1]&0x80000000) {
	op_c->s = 1;
	tmp[0]=tmp[1]=0xFFFFFFFF;
	ADD64(op_c->f, tmp, op_c->f);
	op_c->f[0] = ~op_c->f[0];
	op_c->f[1] = ~op_c->f[1];
    } else
	op_c->s = 0;

    return normalize(op_c);
}

int ADDS(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_s(a, &op_a);
    b_type = extend_s(b, &op_b);
    if ( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) )
	return make_return(f,INV);

    c_type = add_kernel(&op_a, &op_b, &op_c);
    /* special case for -0 + -0 ==> -0 */
    if ((a_type == ZERO) && (b_type == ZERO))
	op_c.s = op_a.s && op_b.s;
    retval = round_s(&op_c, c, f);	

    /* Special Round toward -Infinity condition */
    if(round_mode(f) == ROUND_MINF && c[1]==0 && c[0]==0 && !(retval & UNF) )
      if(a_type == ZERO && b_type == ZERO)
	c[1] = (unsigned)(op_a.s || op_b.s) << 31;
      else
	c[1] = 0x80000000;	    /* -0 */

    return make_return(f,retval);
}

int ADDT(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_t(a, &op_a);
    b_type = extend_t(b, &op_b);
    if ( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) )
	return make_return(f,INV);

    c_type = add_kernel(&op_a, &op_b, &op_c);
    /* special case for -0 + -0 ==> -0 */
    if ((a_type == ZERO) && (b_type == ZERO))
	op_c.s = op_a.s && op_b.s;

    retval = round_t(&op_c, c, f);	

    /* Special Round toward -Infinity condition */
    if(round_mode(f) == ROUND_MINF && c[1]==0 && c[0]==0 && !(retval & UNF) )
      if(a_type == ZERO && b_type == ZERO)
	c[1] = (unsigned)(op_a.s || op_b.s) << 31;
      else
	c[1] = 0x80000000;	    /* -0 */

    return make_return(f,retval);
}

int SUBS(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_s(a, &op_a);
    b_type = extend_s(b, &op_b);
    if ( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) )
	return make_return(f,INV);

    op_b.s = !(op_b.s);
    add_kernel(&op_a, &op_b, &op_c);
    /* special case for -0 - +0 ==> -0 */
    if ((a_type == ZERO) && (b_type == ZERO))
	op_c.s = op_a.s && op_b.s;
	
    retval = round_s(&op_c, c, f);	

    /* Special Round toward -Infinity condition */
    if(round_mode(f) == ROUND_MINF && c[1]==0 && c[0]==0 && !(retval & UNF) )
      if(a_type == ZERO && b_type == ZERO)
	c[1] = (unsigned)(op_a.s || op_b.s) << 31;
      else
	c[1] = 0x80000000;	    /* -0 */

    return make_return(f,retval);
}

int SUBT(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_t(a, &op_a);
    b_type = extend_t(b, &op_b);
    if ( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) )
	return make_return(f,INV);

    op_b.s = !(op_b.s);
    add_kernel(&op_a, &op_b, &op_c);
    /* special case for -0 - +0 ==> -0 */
    if ((a_type == ZERO) && (b_type == ZERO))
	op_c.s = op_a.s && op_b.s;

    retval = round_t(&op_c, c, f);	

    /* Special Round toward -Infinity condition */
    if(round_mode(f) == ROUND_MINF && c[1]==0 && c[0]==0 && !(retval & UNF) )
      if(a_type == ZERO && b_type == ZERO)
	c[1] = (unsigned)(op_a.s || op_b.s) << 31;
      else
	c[1] = 0x80000000;	    /* -0 */

    return make_return(f,retval);
}

int MULS(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_s(a, &op_a);
    b_type = extend_s(b, &op_b);
    if ( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) )
	return make_return(f,INV);

    op_c.s = op_a.s ^ op_b.s;
    op_c.e = op_a.e + op_b.e;
    MUL64(FALSE, op_a.f, op_b.f, op_c.f, &(op_c.f[2]));

    normalize(&op_c);
/*
When the MUL64 routine multiplies the two numbers, it ends up doing an
artificial shift left of 55 bits.  When normalize shifts this back to the 
right place, it has to be removed from the exponent.	(David Asher)
*/
    op_c.e -= 55;	/* drop the 55 original bits. */

    retval = round_s(&op_c, c, f);	

    return make_return(f,retval);
}

int MULT(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_t(a, &op_a);
    b_type = extend_t(b, &op_b);
    if ( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) )
	return make_return(f,INV);

    op_c.s = op_a.s ^ op_b.s;
    op_c.e = op_a.e + op_b.e;
    MUL64(FALSE, op_a.f, op_b.f, op_c.f, &(op_c.f[2]));

    normalize(&op_c);
    op_c.e -= 55;	/* drop the 55 original bits. */

    retval = round_t(&op_c, c, f);	

    return make_return(f,retval);
}
/*****************************************************************************/
int DIVS(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
int a_type, b_type, i, retval;
EXTENDED op_a, op_b, op_c;

 c[1] = c[0] = 0;
 a_type = extend_s(a, &op_a);
 b_type = extend_s(b, &op_b);
 if( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
     ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) ) 
  {
   retval = INV;
   return make_return(f,retval);
  };

 if(b_type == ZERO)
   if(a_type == ZERO)    return make_return(f,INV);
   else			 return make_return(f,DZE);

 op_c.s = op_a.s ^ op_b.s;
 op_c.e = op_a.e - op_b.e;

 for (i=0; i<2; ++i) 
  {
   op_a.f[i+2] = op_a.f[i];
   op_a.f[i]=0;
  };
 DIV128(FALSE, op_a.f, op_b.f, op_c.f);
#ifdef EV4_INEDIV
 if(a_type != ZERO)
   op_c.f[0] |= STICKY_S; /* force a sticky bit because DIVs never hit exact .5 
*/
#endif

 normalize(&op_c);
 op_c.e -= 9;		/* remove excess exp from original shift */

 retval = round_s(&op_c, c, f);	
 if(retval == MATH_NOTRAP)  retval = 0;
#ifdef EV4_INEDIV
 if(ine_enable(f)) retval |= INE;
 /* EV4: DIV does not set INE bit in FPCR */
 return(make_return(f,retval) & ~(0x100000));
#else
 return(make_return(f,retval));
#endif
}
/*****************************************************************************/
int DIVT(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
int a_type, b_type, i, retval;
EXTENDED op_a, op_b, op_c;

 c[1] = c[0] = 0;
 a_type = extend_t(a, &op_a);
 b_type = extend_t(b, &op_b);
 if( ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM)) ||
     ((b_type == NaN) || (b_type == INFINITY) || (b_type == DENORM)) )
   return make_return(f,INV);

 if(b_type == ZERO)
   if(a_type == ZERO) return make_return(f,INV);
   else		      return make_return(f,DZE);

 op_c.s = op_a.s ^ op_b.s;
 op_c.e = op_a.e - op_b.e;

 for(i=0; i<2; ++i)
  {
   op_a.f[i+2] = op_a.f[i];
   op_a.f[i]=0;
  };
 div_kernel(FALSE, op_a.f, op_b.f, op_c.f);

#ifdef EV4_INEDIV
 if(a_type != ZERO)
   op_c.f[0] |= STICKY_T; /*force a sticky bit because DIVs never hit exact .5*/
#endif

 normalize(&op_c);
 op_c.e -= 9;		/* remove excess exp from original shift */

 retval = round_t(&op_c, c, f);
 if(retval == MATH_NOTRAP)  retval = 0;
#ifdef EV4_INEDIV
 if(ine_enable(f)) retval |= INE;
 /* EV4: DIV does not set INE bit in FPCR */
 return(make_return(f,retval) & ~(0x100000));
#else
 return(make_return(f,retval));
#endif
}
/*****************************************************************************/
/*****************************************************************************/
/* THE FOLLOWING FUNCTIONS ARE NEVER USED.				     */
/*****************************************************************************/
/*****************************************************************************/
#ifdef XXXX
int CMPSEQ(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_s(a, &op_a);
    b_type = extend_s(b, &op_b);
    if ( ((a_type == NaN) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);

    if ((a_type != INFINITY) && (b_type != INFINITY)) {
	if ( ((op_a.e == op_b.e) && (CMP128(op_a.f,op_b.f) == 0) && (op_a.s == 
op_b.s)) ||
	     ((a_type == ZERO) && (b_type == ZERO)) )
	    retval = round_s(&exttwo, c, f);
	else
            retval = round_s(&extszero, c, f);
    } else {					/* one/both operands are 
infinity */
	if ((a_type == INFINITY) && (b_type == INFINITY) && (op_a.s == op_b.s))
		retval = round_s(&exttwo,c,f);	/* == if both inf, and signs 
equal */
	    else
		retval = round_s(&extszero,c,f);
    }

    return make_return(f,retval);
}

int CMPSLT(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_s(a, &op_a);
    b_type = extend_s(b, &op_b);
    if ( ((a_type == NaN) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);

    if ((a_type != INFINITY) && (b_type != INFINITY)) {
        if ( ((op_a.s == 1) && (op_b.s == 0) && 
	      ((a_type != ZERO) || (b_type != ZERO))) ||
	     ((op_a.s == 1) && (op_b.s == 1) &&
	      ( (op_a.e > op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) > 0) ) )) ||
	     ((op_a.s == 0) && (op_b.s == 0) &&
	      ( (op_a.e < op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) < 0) ) ))
           )
	    retval = round_s(&exttwo, c, f);
        else
            retval = round_s(&extszero, c, f);
    } else {					/* one/both operands are 
infinity */
	if ( ((a_type == INFINITY) && op_a.s && !((b_type == INFINITY) && 
op_b.s)) || /* if a = - inf and b != -inf, then a < b */
	     ((a_type != INFINITY) && (b_type == INFINITY) && !op_b.s)		 
     /* if a not inf and b = +inf, then a < b */
	   )
		retval = round_s(&exttwo,c,f);	
	    else
		retval = round_s(&extszero,c,f);
    }

    return make_return(f,retval);
}

int CMPSLE(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_s(a, &op_a);
    b_type = extend_s(b, &op_b);
    if ( ((a_type == NaN) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);


    if ((a_type != INFINITY) && (b_type != INFINITY)) {
        if ( ((a_type == ZERO) && (b_type == ZERO)) ||
	     ((op_a.s == 1) && (op_b.s == 0)) ||
	     ((op_a.s == 1) && (op_b.s == 1) &&
	      ( (op_a.e > op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) >= 0) ) )) ||
	     ((op_a.s == 0) && (op_b.s == 0) &&
	      ( (op_a.e < op_b.e) || ( (op_a.e == op_b.e) && 
(CMP128(op_a.f,op_b.f) <= 0) ) ))
           )
	    retval = round_s(&exttwo, c, f);
        else
             retval = round_s(&extszero, c, f);
    } else {
	if ( ((a_type == INFINITY) && op_a.s) ||		/* if a = - inf 
then a <= b */
	     ((b_type == INFINITY) && !op_b.s)			/* if b = +inf, 
then a <= b */
	   )
		retval = round_s(&exttwo,c,f);	
	    else
		retval = round_s(&extszero,c,f);
    }

    return make_return(f,retval);
}

int CMPSUN(f,a,b,c)
int f;
unsigned int a[];
unsigned int b[];
unsigned int c[];
{
    int a_type, b_type, c_type, retval;
    EXTENDED op_a, op_b, op_c;

    c[1] = c[0] = 0;
    a_type = extend_s(a, &op_a);
    b_type = extend_s(b, &op_b);
    if ( ((a_type == NaN) || (a_type == DENORM)) ||
	 ((b_type == NaN) || (b_type == DENORM)) )
	return make_return(f,INV);

    retval = round_s(&extszero, c, f);

    return make_return(f,retval);
}

int CVTST(f,a,b)
int f;
unsigned int a[];
unsigned int b[];
{
    int a_type, retval;
    EXTENDED temp;

    b[1] = b[0] = 0;
    a_type = extend_s(a, &temp);
    if ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM))
	return make_return(f,INV);

    retval = round_t(&temp, b, f);

    return make_return(f,retval);
}

int CVTSQ(f,a,b)
int f;
unsigned int a[];
unsigned int b[];
{
    int a_type, ov, retval, uv;
    unsigned int midway;
    static unsigned int one[2] = { 1, 0 };
    EXTENDED temp;

    b[1] = b[0] = retval = 0;
    a_type = extend_s(a, &temp);
    if ((a_type == NaN) || (a_type == INFINITY) || (a_type == DENORM))
	return make_return(f,INV);

    if (temp.e > 0) {
        ov = 0;
	while (temp.e > 0) {
	    --(temp.e);
	    ov |= (temp.f[3] >> 31);
	    temp.f[3] = (temp.f[3] << 1) | (temp.f[2] >> 31);
	    temp.f[2] = (temp.f[2] << 1) | (temp.f[1] >> 31);
	    temp.f[1] = (temp.f[1] << 1) | (temp.f[0] >> 31);
	    temp.f[0] = (temp.f[0] << 1);
	}
	if (ov || (temp.f[3]&0xFFC00000))
	    retval |= IOV | INE;
    }
    if (temp.e < 0) {
	while (temp.e < 0) {
	    ++(temp.e);
	    uv = temp.f[0] & 1;		/* save sticky value */
	    temp.f[0] = (temp.f[0] >> 1) | (temp.f[1] << 31) | uv;
	    temp.f[1] = (temp.f[1] >> 1) | (temp.f[2] << 31);
	    temp.f[2] = (temp.f[2] >> 1) | (temp.f[3] << 31);
	    temp.f[3] = (temp.f[3] >> 1);
	}
    }

    b[0] = (temp.f[1] >> 23) | (temp.f[2] << 9);
    b[1] = (temp.f[2] >> 23) | (temp.f[3] << 9);
    switch (round_mode(f)) {
    case ROUND_NEAR:
        if (temp.f[1] & 0x00400000) {
            midway = ((temp.f[1] & 0x003FFFFF)==0) && (temp.f[0] == 0);
	    if ((midway && (temp.f[1] & 0x00800000)) || 
	        !midway)
	        ADD64(b,one,b);
        }
	break;
    case ROUND_ZERO:
	/* no action needed */
	break;
    }
    if (((temp.f[1] & 0x007FFFFF) != 0) || (temp.f[0] != 0))
	retval |= INE;

    if (temp.s == 1) {
	b[0] = ~b[0];
	b[1] = ~b[1];
	ADD64(b,one,b);
    }

    return make_return(f,retval);
}


#endif

