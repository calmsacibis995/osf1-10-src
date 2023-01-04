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
static char *rcsid = "@(#)$RCSfile: 64bit_math.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/07/08 16:59:00 $";
#endif

/*****************************************************************************/
/*FILE: 64BIT_MATH.C - extended precision math routines			     */
/*****************************************************************************
! REVISION HISTORY:
! Who	When		What
!---------------------------------------------------------------
! HA	16-Jan-1992	fix 1<<31 on MIPS platforms
! HA	30-Aug-1991	Switch to ALPHA ISP
! SJM	25-Sep-1989	First pass
 *****************************************************************************/
int ADD128(a,b,c)
int *a,*b,*c;
    {
    int t,t1;
    int temp[4];

	
    t = (a[0] & 0xFFFF) + (b[0] & 0xFFFF);
    temp[0] = t & 0xFFFF;

    t = ((a[0]>>16) & 0xFFFF) + ((b[0]>>16) & 0xFFFF) + (t >>16);
    temp[0] |= t<<16;

    t = (a[1] & 0xFFFF) + (b[1] & 0xFFFF) + (t >>16);
    temp[1] = t & 0xFFFF;

    t = ((a[1]>>16) & 0xFFFF) + ((b[1]>>16) & 0xFFFF) + (t >>16);
    temp[1] |= t<<16;




    t = (a[2] & 0xFFFF) + (b[2] & 0xFFFF) + (t>>16);
    temp[2] = t & 0xFFFF;

    t = ((a[2]>>16) & 0xFFFF) + ((b[2]>>16) & 0xFFFF) + (t >>16);
    temp[2] |= t<<16;

    t = (a[3] & 0xFFFF) + (b[3] & 0xFFFF) + (t >>16);
    temp[3] = t & 0xFFFF;

    t1 = ((a[3]>>16) & 0x7FFF) + ((b[3]>>16) & 0x7FFF) + (t >>16);
    t = ((a[3]>>16) & 0xFFFF) + ((b[3]>>16) & 0xFFFF) + (t >>16);
    temp[3] |= t<<16;
    
    c[0] = temp[0];
    c[1] = temp[1];
    c[2] = temp[2];
    c[3] = temp[3];

    return ((t>>16 & 1) ^ (t1>>15 & 1));
}

/*****************************************************************************/
/*****************************************************************************/
int ADD64(a,b,c)
int *a,*b,*c;
{
 int t,t1,ov;
 int temp[2];
	
 t = (a[0] & 0xFFFF) + (b[0] & 0xFFFF);
 temp[0] = t & 0xFFFF;

 t1 = ((a[0]>>16) & 0x7FFF) + ((b[0]>>16) & 0x7FFF) + (t >>16);
 t  = ((a[0]>>16) & 0xFFFF) + ((b[0]>>16) & 0xFFFF) + (t >>16);
 ov = (t>>16 & 1) ^ (t1>>15 & 1);
 temp[0] |= t<<16;

 t = (a[1] & 0xFFFF) + (b[1] & 0xFFFF) + (t >>16);
 temp[1] = t & 0xFFFF;

 t1 = ((a[1]>>16) & 0x7FFF) + ((b[1]>>16) & 0x7FFF) + (t >>16);
 t  = ((a[1]>>16) & 0xFFFF) + ((b[1]>>16) & 0xFFFF) + (t >>16);
 ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<1;
 temp[1] |= t<<16;
    
 c[0] = temp[0];
 c[1] = temp[1];

 return (ov);
}
/*****************************************************************************/
/*****************************************************************************/
int SUB256(a,b,c)
int *a,*b,*c;
    {
    int t,t1,ov,bor;
    int bb[8];
    int temp[8];

    bb[0] = b[0] ^ 0xFFFFFFFF;
    bb[1] = b[1] ^ 0xFFFFFFFF;
    bb[2] = b[2] ^ 0xFFFFFFFF;
    bb[3] = b[3] ^ 0xFFFFFFFF;
    bb[4] = b[4] ^ 0xFFFFFFFF;
    bb[5] = b[5] ^ 0xFFFFFFFF;
    bb[6] = b[6] ^ 0xFFFFFFFF;
    bb[7] = b[7] ^ 0xFFFFFFFF;
	
    t = (a[0] & 0xFFFF) + (bb[0] & 0xFFFF) +1;
    temp[0] = t & 0xFFFF;

    t1 = ((a[0]>>16) & 0x7FFF) + ((bb[0]>>16) & 0x7FFF) + (t >>16);
    t = ((a[0]>>16) & 0xFFFF) + ((bb[0]>>16) & 0xFFFF) + (t >>16);
    ov = (t>>16 & 1) ^ (t1>>15 & 1);
    temp[0] |= t<<16;


    t = (a[1] & 0xFFFF) + (bb[1] & 0xFFFF) + (t >>16);
    temp[1] = t & 0xFFFF;

    t1 = ((a[1]>>16) & 0x7FFF) + ((bb[1]>>16) & 0x7FFF) + (t >>16);
    t = ((a[1]>>16) & 0xFFFF) + ((bb[1]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<1;
    temp[1] |= t<<16;



    t = (a[2] & 0xFFFF) + (bb[2] & 0xFFFF) + (t >>16);
    temp[2] = t & 0xFFFF;

    t1 = ((a[2]>>16) & 0x7FFF) + ((bb[2]>>16) & 0x7FFF) + (t >>16);
    t = ((a[2]>>16) & 0xFFFF) + ((bb[2]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<2;
    temp[2] |= t<<16;


    t = (a[3] & 0xFFFF) + (bb[3] & 0xFFFF) + (t >>16);
    temp[3] = t & 0xFFFF;

    t1 = ((a[3]>>16) & 0x7FFF) + ((bb[3]>>16) & 0x7FFF) + (t >>16);
    t = ((a[3]>>16) & 0xFFFF) + ((bb[3]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<2;
    temp[3] |= t<<16;


    t = (a[4] & 0xFFFF) + (bb[4] & 0xFFFF) + (t >>16);
    temp[4] = t & 0xFFFF;

    t1 = ((a[4]>>16) & 0x7FFF) + ((bb[4]>>16) & 0x7FFF) + (t >>16);
    t = ((a[4]>>16) & 0xFFFF) + ((bb[4]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<2;
    temp[4] |= t<<16;


    t = (a[5] & 0xFFFF) + (bb[5] & 0xFFFF) + (t >>16);
    temp[5] = t & 0xFFFF;

    t1 = ((a[5]>>16) & 0x7FFF) + ((bb[5]>>16) & 0x7FFF) + (t >>16);
    t = ((a[5]>>16) & 0xFFFF) + ((bb[5]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<2;
    temp[5] |= t<<16;


    t = (a[6] & 0xFFFF) + (bb[6] & 0xFFFF) + (t >>16);
    temp[6] = t & 0xFFFF;

    t1 = ((a[6]>>16) & 0x7FFF) + ((bb[6]>>16) & 0x7FFF) + (t >>16);
    t = ((a[6]>>16) & 0xFFFF) + ((bb[6]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<2;
    temp[6] |= t<<16;



    t = (a[7] & 0xFFFF) + (bb[7] & 0xFFFF) + (t >>16);
    temp[7] = t & 0xFFFF;

    t1 = (((a[7]>>16) & 0xFFFF) | 0x10000) + ((bb[7]>>16) & 0x1FFFF) + (t >>16);
    bor = (((t1>>16) & 1) == 0);
    ov |= bor <<31;
    t1 = ((a[7]>>16) & 0x7FFF) + ((bb[7]>>16) & 0x7FFF) + (t >>16);
    t = ((a[7]>>16) & 0xFFFF) + ((bb[7]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<3;
    temp[7] |= t<<16;

    c[0] = temp[0];
    c[1] = temp[1];
    c[2] = temp[2];
    c[3] = temp[3];
    c[4] = temp[4];
    c[5] = temp[5];
    c[6] = temp[6];
    c[7] = temp[7];

    return (ov);
}

int SUB128(a,b,c)
int *a,*b,*c;
    {
    int t,t1,ov,bor;
    int bb[4];
    int temp[4];

    bb[0] = b[0] ^ 0xFFFFFFFF;
    bb[1] = b[1] ^ 0xFFFFFFFF;
    bb[2] = b[2] ^ 0xFFFFFFFF;
    bb[3] = b[3] ^ 0xFFFFFFFF;
	
    t = (a[0] & 0xFFFF) + (bb[0] & 0xFFFF) +1;
    temp[0] = t & 0xFFFF;

    t1 = ((a[0]>>16) & 0x7FFF) + ((bb[0]>>16) & 0x7FFF) + (t >>16);
    t = ((a[0]>>16) & 0xFFFF) + ((bb[0]>>16) & 0xFFFF) + (t >>16);
    ov = (t>>16 & 1) ^ (t1>>15 & 1);
    temp[0] |= t<<16;


    t = (a[1] & 0xFFFF) + (bb[1] & 0xFFFF) + (t >>16);
    temp[1] = t & 0xFFFF;

    t1 = ((a[1]>>16) & 0x7FFF) + ((bb[1]>>16) & 0x7FFF) + (t >>16);
    t = ((a[1]>>16) & 0xFFFF) + ((bb[1]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<1;
    temp[1] |= t<<16;



    t = (a[2] & 0xFFFF) + (bb[2] & 0xFFFF) + (t >>16);
    temp[2] = t & 0xFFFF;

    t1 = ((a[2]>>16) & 0x7FFF) + ((bb[2]>>16) & 0x7FFF) + (t >>16);
    t = ((a[2]>>16) & 0xFFFF) + ((bb[2]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<2;
    temp[2] |= t<<16;



    t = (a[3] & 0xFFFF) + (bb[3] & 0xFFFF) + (t >>16);
    temp[3] = t & 0xFFFF;

    t1 = (((a[3]>>16) & 0xFFFF) | 0x10000) + ((bb[3]>>16) & 0x1FFFF) + (t >>16);
    bor = (((t1>>16) & 1) == 0);
    ov |= bor <<31;
    t1 = ((a[3]>>16) & 0x7FFF) + ((bb[3]>>16) & 0x7FFF) + (t >>16);
    t = ((a[3]>>16) & 0xFFFF) + ((bb[3]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<3;
    temp[3] |= t<<16;

    c[0] = temp[0];
    c[1] = temp[1];
    c[2] = temp[2];
    c[3] = temp[3];

    return (ov);
}

int SUB64(a,b,c)
int *a,*b,*c;
    {
    int t,t1,ov;
    int bb[2];
    int temp[2];

    bb[0] = b[0] ^ 0xFFFFFFFF;
    bb[1] = b[1] ^ 0xFFFFFFFF;
	
    t = (a[0] & 0xFFFF) + (bb[0] & 0xFFFF) +1;
    temp[0] = t & 0xFFFF;

    t1 = ((a[0]>>16) & 0x7FFF) + ((bb[0]>>16) & 0x7FFF) + (t >>16);
    t = ((a[0]>>16) & 0xFFFF) + ((bb[0]>>16) & 0xFFFF) + (t >>16);
    ov = (t>>16 & 1) ^ (t1>>15 & 1);
    temp[0] |= t<<16;

    t = (a[1] & 0xFFFF) + (bb[1] & 0xFFFF) + (t >>16);
    temp[1] = t & 0xFFFF;

    t1 = ((a[1]>>16) & 0x7FFF) + ((bb[1]>>16) & 0x7FFF) + (t >>16);
    t = ((a[1]>>16) & 0xFFFF) + ((bb[1]>>16) & 0xFFFF) + (t >>16);
    ov |= ((t>>16 & 1) ^ (t1>>15 & 1))<<1;
    temp[1] |= t<<16;

    c[0] = temp[0];
    c[1] = temp[1];

    return (ov);
}
/*****************************************************************************/
int MUL32(Signed,a,b,c,d)
int Signed,*a,*b,*c,*d;
{
int aa[4];
int bb[2];
 aa[0] = a[0];
 aa[1] = a[0]>>31;
 bb[0] = b[0];
 bb[1] = b[0]>>31;
 return(MUL64(Signed,aa,bb,c,d));
}

/*****************************************************************************/
/*****************************************************************************/
int MUL64(Signed,a,b,c,d)
int Signed,*a,*b,*c,*d;
{
int tt[4];
int pp[4];
int aa[4];
int bb[2];
int i,ov;
    
 aa[0] = a[0];
 aa[1] = a[1];
 bb[0] = b[0];
 bb[1] = b[1];

 ov = 0;
 if(Signed == 0)
  {
   /* looks like if a < b then aa=b and bb=a */

   if( ((unsigned int) a[1] < (unsigned int) b[1]) ||
       (((unsigned int) a[1] == (unsigned int) b[1]) &&
	((unsigned int) a[0] <  (unsigned int) b[0])))
    {
     aa[0] = b[0];
     aa[1] = b[1];
     bb[0] = a[0];
     bb[1] = a[1];
    };
   aa[2] = 0;
   aa[3] = 0;
  }
 else	/* Signed multiply */
  {    
   if((b[1] < 0) && (a[1] < 0))
    {
     tt[0] = -1;
     tt[1] = -1;

     ov |= ADD64(aa,tt,aa) & 2;
     ov |= ADD64(bb,tt,bb) & 2;

     aa[0] = aa[0] ^ 0xFFFFFFFF;
     aa[1] = aa[1] ^ 0xFFFFFFFF;
     bb[0] = bb[0] ^ 0xFFFFFFFF;
     bb[1] = bb[1] ^ 0xFFFFFFFF;
    }
   else if(b[1] < 0) 
    {
     aa[0] = b[0];
     aa[1] = b[1];
     bb[0] = a[0];
     bb[1] = a[1];
    };

   if((bb[1] >= 0) && (aa[1] >= 0))
     if((aa[1] < bb[1]) || ((aa[1] == bb[1]) && (aa[0] < bb[0])))
      {
       tt[0] = aa[0];
       tt[1] = aa[1];
       aa[0] = bb[0];
       aa[1] = bb[1];
       bb[0] = tt[0];
       bb[1] = tt[1];
      };

   if(aa[1] < 0)
    {
     aa[2] = -1;
     aa[3] = -1;
    }
   else   
    {
     aa[2] = 0;
     aa[3] = 0;
    };
  };
	    
    /* now the multiplier is positive or unsigned */
    
 tt[0] = tt[1] = tt[2] = tt[3] = 0;
 pp[0] = pp[1] = pp[2] = pp[3] = 0;
    
 for(i=0; i < 64; i++)
  {
   if((i <  32) && (((unsigned int)bb[0]>>i) == 0))      i = 32;
   if((i >= 32) && (((unsigned int)bb[1]>>(i-32)) == 0)) i = 63;

   if(((i < 32) && ((bb[0] >> i) & 1)) || 
      ((i >= 32) && ((bb[1] >> (i-32)) & 1)))
    {
     if(i < 32)
      {
       tt[0] = aa[0] << i;
       tt[1] = aa[1] << i;
       tt[2] = aa[2] << i;
       tt[3] = aa[3] << i;

       if(i != 0)
	{
	 tt[1] |= (unsigned int) aa[0] >> 32-i;
	 tt[2] |= (unsigned int) aa[1] >> 32-i;
	 tt[3] |= (unsigned int) aa[2] >> 32-i;
	};
      }
     else
      {
       tt[0] = 0;
       tt[1] = aa[0] << i-32;
       tt[2] = aa[1] << i-32;
       if(i > 32) tt[2] |= (unsigned int) aa[0] >> 64-i;
       tt[3] = aa[2] << i-32;
       if(i > 32) tt[3] |= (unsigned int) aa[1] >> 64-i;
      };

     ADD128(pp,&tt[0],pp);
    };
  };

 if(c != 0)
  {
   c[0] = pp[0];
   c[1] = pp[1];
  };
 if (d != 0)
  {
   d[0] = pp[2];
   d[1] = pp[3];
  };

/*    ov = 0;	*/
 i = pp[0]>>31;
 if((i != pp[1]) || (i != pp[2]) || (i != pp[3])) ov |= 1;
 i = pp[1]>>31;
 if((i != pp[2]) || (i != pp[3])) ov |= 2;
 if(Signed == 0) ov = 0;
 return (ov);
}

/*****************************************************************************/
int DIV128(Signed,a,b,c)
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
/*
   printf("inner loop info (%d)	aa = %08x%08x %08x%08x %08x%08x %08x%08x \n		
	bb = %08x%08x %08x%08x %08x%08x %08x%08x \n",
	    i,aa[7],aa[6],aa[5],aa[4],aa[3],aa[2],aa[1],aa[0],
	    bb[7],bb[6],bb[5],bb[4],bb[3],bb[2],bb[1],bb[0]);
   printf("			tt = %08x%08x %08x%08x %08x%08x %08x%08x \n		
	pp = %08x%08x %08x%08x %08x%08x %08x%08x ov= %0x\n",
	    tt[7],tt[6],tt[5],tt[4],tt[3],tt[2],tt[1],tt[0],
	    pp[7],pp[6],pp[5],pp[4],pp[3],pp[2],pp[1],pp[0],ov);
*/
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
 return(1);
}
/*****************************************************************************/

int DIV64(Signed,a,b,c,d)
int Signed,*a,*b,*c,*d;
    {
    int tt[4];
    int pp[4];
    int aa[4];
    int bb[6];
    int i,ov;
    int  sign = 0;
    
    /* quick check for div by zero */
    if ((b[0] == 0) && (b[1] == 0))
	{
	if (c != 0) c[0] = c[1] = 0;
	if (d != 0) d[0] = d[1] = 0;
	return(0);
	};

    aa[0] = a[0];
    aa[1] = a[1];
    aa[2] = 0;
    aa[3] = 0;


    bb[0] = 0;
    bb[1] = 0;
    bb[2] = b[0];
    bb[3] = b[1];
    bb[4] = b[0];
    bb[5] = b[1];



    /* if numbers signed, make them positive and remember the sign's */
    if (Signed == 1)
	{
	tt[0] = 1;
	tt[1] = 0;

	if (aa[1] < 0)
	    {
	    sign++;
	    aa[0] = aa[0] ^ 0xFFFFFFFF;
	    aa[1] = aa[1] ^ 0xFFFFFFFF;
	    ADD64(aa,tt,aa);
	    };
	if (bb[3] < 0)
	    {
	    sign--;
	    bb[2] = bb[2] ^ 0xFFFFFFFF;
	    bb[3] = bb[3] ^ 0xFFFFFFFF;
	    ADD64(&bb[2],tt,&bb[2]);
	    bb[4] = bb[2];
	    bb[5] = bb[3];
	    };
	};



    /* shift b down by one to start */
    bb[0] = ((bb[0]>>1) & 0x7FFFFFFF) | bb[1]<<31;
    bb[1] = ((bb[1]>>1) & 0x7FFFFFFF) | bb[2]<<31;
    bb[2] = ((bb[2]>>1) & 0x7FFFFFFF) | bb[3]<<31;
    bb[3] = ((bb[3]>>1) & 0x7FFFFFFF);


    /* start with result = 0 */
    pp[0] = pp[1] = pp[2] = pp[3] = 0;


    for (i = 63; i >= 0; i--)
	{
	ov = SUB128(aa,bb,tt);
/*	printf("inner loop info (%d)	aa = %08x%08x %08x%08x \n			
bb = %08x%08x %08x%08x \n",
	    i,aa[3],aa[2],aa[1],aa[0],bb[3],bb[2],bb[1],bb[0]);
	printf("			tt = %08x%08x %08x%08x \n			
pp = %08x%08x %08x%08x ov= %0x\n",
	    tt[3],tt[2],tt[1],tt[0],pp[3],pp[2],pp[1],pp[0],ov);
*/
	if ((ov & (1u<<31)) == 0)
/*	if (						*/
/*	    ((bb[0] == 0) && (bb[1] == 0)) ||		*/
/*	    ((ov = SUB128(aa,bb,tt) & 1<<3) == 0))	*/
	    { /* No borrow case or b=0,inject a one, and use result */
	    pp[i>>5] |= 1<<(i & 0x1F);
	    aa[0] = tt[0];
	    aa[1] = tt[1];
	    aa[2] = tt[2];
	    aa[3] = tt[3];
	    };
	/* next time shift by b>>1 */
	bb[0] = ((bb[0]>>1) & 0x7FFFFFFF) | bb[1]<<31;
	bb[1] = ((bb[1]>>1) & 0x7FFFFFFF) | bb[2]<<31;
	bb[2] = ((bb[2]>>1) & 0x7FFFFFFF) | bb[3]<<31;
	bb[3] = ((bb[3]>>1) & 0x7FFFFFFF);
	};

/*    printf("final loop info 	aa = %08x%08x %08x%08x \n			
bb = %08x%08x %08x%08x \n",
	i,aa[3],aa[2],aa[1],aa[0],bb[3],bb[2],bb[1],bb[0]);
    printf("			tt = %08x%08x %08x%08x \n			
pp = %08x%08x %08x%08x ov= %0x\n",
	tt[3],tt[2],tt[1],tt[0],pp[3],pp[2],pp[1],pp[0],ov);
*/


    if (sign != 0)
	{
	tt[0] = 1;
	tt[1] = 0;
	};

    /* if we want a remainder fudge one up */
    if (d != 0)
	{
	MUL64(0,pp,&bb[4],bb,0);
	SUB64(a,bb,bb);
	if (sign != 0)
	    {
	    bb[0] = bb[0] ^ 0xFFFFFFFF;
	    bb[1] = bb[1] ^ 0xFFFFFFFF;
	    ADD64(bb,tt,bb);
	    };
	d[0] = bb[0];
	d[1] = bb[1];
	};

    /* all done, now fix up the sing for signed arith */
    if (c != 0)
	{
	if (sign != 0)
	    {
	    pp[0] = pp[0] ^ 0xFFFFFFFF;
	    pp[1] = pp[1] ^ 0xFFFFFFFF;
	    ADD64(pp,tt,pp);
	    };
	c[0] = pp[0];
	c[1] = pp[1];
	};
    return(1);
}
/*****************************************************************************/
/*****************************************************************************/
/* THE FOLLOWING ARE UN-USED FUNCTIONS	(H. Akhiani)			     */
/*****************************************************************************/
/*****************************************************************************/
#ifdef XXXX
int ADD256(a,b,c)
int *a,*b,*c;
    {
    int t,t1;
    int temp[8];

	
    t = (a[0] & 0xFFFF) + (b[0] & 0xFFFF);
    temp[0] = t & 0xFFFF;

    t = ((a[0]>>16) & 0xFFFF) + ((b[0]>>16) & 0xFFFF) + (t >>16);
    temp[0] |= t<<16;

    t = (a[1] & 0xFFFF) + (b[1] & 0xFFFF) + (t >>16);
    temp[1] = t & 0xFFFF;

    t = ((a[1]>>16) & 0xFFFF) + ((b[1]>>16) & 0xFFFF) + (t >>16);
    temp[1] |= t<<16;

    t = (a[2] & 0xFFFF) + (b[2] & 0xFFFF) + (t>>16);
    temp[2] = t & 0xFFFF;

    t = ((a[2]>>16) & 0xFFFF) + ((b[2]>>16) & 0xFFFF) + (t >>16);
    temp[2] |= t<<16;

    t = (a[3] & 0xFFFF) + (b[3] & 0xFFFF) + (t>>16);
    temp[3] = t & 0xFFFF;

    t = ((a[3]>>16) & 0xFFFF) + ((b[3]>>16) & 0xFFFF) + (t >>16);
    temp[3] |= t<<16;

    t = (a[4] & 0xFFFF) + (b[4] & 0xFFFF) + (t>>16);
    temp[4] = t & 0xFFFF;

    t = ((a[4]>>16) & 0xFFFF) + ((b[4]>>16) & 0xFFFF) + (t >>16);
    temp[4] |= t<<16;

    t = (a[5] & 0xFFFF) + (b[5] & 0xFFFF) + (t>>16);
    temp[5] = t & 0xFFFF;

    t = ((a[5]>>16) & 0xFFFF) + ((b[5]>>16) & 0xFFFF) + (t >>16);
    temp[5] |= t<<16;

    t = (a[6] & 0xFFFF) + (b[6] & 0xFFFF) + (t>>16);
    temp[6] = t & 0xFFFF;

    t = ((a[6]>>16) & 0xFFFF) + ((b[6]>>16) & 0xFFFF) + (t >>16);
    temp[6] |= t<<16;

    t = (a[7] & 0xFFFF) + (b[7] & 0xFFFF) + (t >>16);
    temp[7] = t & 0xFFFF;

    t1 = ((a[7]>>16) & 0x7FFF) + ((b[7]>>16) & 0x7FFF) + (t >>16);
    t = ((a[7]>>16) & 0xFFFF) + ((b[7]>>16) & 0xFFFF) + (t >>16);
    temp[7] |= t<<16;
    
    c[0] = temp[0];
    c[1] = temp[1];
    c[2] = temp[2];
    c[3] = temp[3];
    c[4] = temp[4];
    c[5] = temp[5];
    c[6] = temp[6];
    c[7] = temp[7];

    return ((t>>16 & 1) ^ (t1>>15 & 1));
}

int ADD32(a,b,c)
int *a,*b,*c;
    {
    int t,t1;
    int temp[1];
	
    t = (a[0] & 0xFFFF) + (b[0] & 0xFFFF);
    temp[0] = t & 0xFFFF;

    t1 = ((a[0]>>16) & 0x7FFF) + ((b[0]>>16) & 0x7FFF) + (t >>16);
    t = ((a[0]>>16) & 0xFFFF) + ((b[0]>>16) & 0xFFFF) + (t >>16);
    temp[0] |= t<<16;
    
    c[0] = temp[0];

    return ((t>>16 & 1) ^ (t1>>15 & 1));
}

int SUB32(a,b,c)
int *a,*b,*c;
    {
    int t,t1;
    int bb[1];
    int temp[2];

    bb[0] = b[0] ^ 0xFFFFFFFF;
	
	
    t = (a[0] & 0xFFFF) + (bb[0] & 0xFFFF) + 1;
    temp[0] = t & 0xFFFF;

    t1 = ((a[0]>>16) & 0x7FFF) + ((bb[0]>>16) & 0x7FFF) + (t >>16);
    t = ((a[0]>>16) & 0xFFFF) + ((bb[0]>>16) & 0xFFFF) + (t >>16);
    temp[0] |= t<<16;
    
    c[0] = temp[0];

    return ((t>>16 & 1) ^ (t1>>15 & 1));
}
/*****************************************************************************/

#endif
