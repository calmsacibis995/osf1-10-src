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
static char	*sccsid = "@(#)$RCSfile: support.c,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/05/05 16:33:04 $";
#endif 
/*
 */
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * All recipients should regard themselves as participants in an ongoing
 * research project and hence should feel obligated to report their
 * experiences (good or bad) with these elementary function codes, using
 * the sendbug(8) program, to the authors.
 */

#ifndef lint

#endif /* not lint */

#include "hdr.h"
#include <errno.h>
#include <fp.h>

/* 
 * Some IEEE standard 754 recommended functions and remainder and sqrt for 
 * supporting the C elementary functions.
 ******************************************************************************
 * WARNING:
 *      These codes are developed (in double) to support the C elementary
 * functions temporarily. They are not universal, and some of them are very
 * slow (in particular, drem and sqrt is extremely inefficient). Each 
 * computer system should have its implementation of these functions using 
 * its own assembler.
 ******************************************************************************
 *
 * IEEE 754 required operations:
 *     drem(x,p) 
 *              returns  x REM y  =  x - [x/y]*y , where [x/y] is the integer
 *              nearest x/y; in half way case, choose the even one.
 *     sqrt(x) 
 *              returns the square root of x correctly rounded according to 
 *		the rounding mod.
 *
 * IEEE 754 recommended functions:
 * (a) copysign(x,y) 
 *              returns x with the sign of y. 
 * (b) scalb(x,N) 
 *              returns  x * (2**N), for integer values N.
 * (c) logb(x) 
 *              returns the unbiased exponent of x, a signed integer in 
 *              double precision, except that logb(0) is -INF, logb(INF) 
 *              is +INF, and logb(NAN) is that NAN.
 * (d) finite(x) 
 *              returns the value TRUE if -INF < x < +INF and returns 
 *              FALSE otherwise.
 *
 *
 * CODED IN C BY K.C. NG, 11/25/84;
 * REVISED BY K.C. NG on 1/22/85, 2/13/85, 3/24/85.
 */

#define msign	0x7fffffff

#if defined(vax)||defined(tahoe)      /* VAX D format */
    static unsigned long mexp =0x7f80000 ;
    static long  prep1=57, gap=23, bias=129           ;   
    static double zero=0.0 ;
#else	/* defined(vax)||defined(tahoe) */
    static unsigned long mexp =0x7ff00000  ;
    static long prep1=54, gap=20, bias=1023           ;
    static double zero=0.0;
#endif	/* defined(vax)||defined(tahoe) */

const volatile double nunf = DBL_MIN;

double scalb(double x, double N)
{
        long int k;
	

        if( x == zero ) return(x);

#if defined(vax)||defined(tahoe)
        if( (k= MEXP(x)) != ~msign ) {		/* THIS IS WRONG - ALWAYS TRUE!!!! */
            if (N < -260)
		return(nunf*nunf);
	    else if (N > 260) {
		extern double infnan();
		return(copysign(infnan(ERANGE),x));
	    }
#else	/* defined(vax)||defined(tahoe) */
        if( (k= MEXP(x)) != mexp ) {
            if( N<-2100) {
                errno = ERANGE;
		return(nunf*nunf);
	      } else if(N>2100) {
		return(HUGE_VAL);
	    }
            if( k == 0 ) {
                 x *= scalb(1.0,(int)prep1);  N -= prep1; return(scalb(x,N));}
#endif	/* defined(vax)||defined(tahoe) */

            if((k = (k>>gap)+ N) > 0 )
                if( k < (mexp>>gap) )
		  VALH(x) = (VALH(x) &~ mexp) | (k<<gap);
                else
		  x=HUGE_VAL;               /* overflow */
            else
                if( k > -prep1 ) {	/* gradual underflow */
		  VALH(x) = (VALH(x) &~ mexp)|(1L<<gap);
		  x *= scalb(1.0,k-1);
		}
                else {
		  errno = ERANGE;
		  return(nunf*nunf);
		}
	} else {
	    /*
	     * x has maximal exponent
	     */
	    if (x == HUGE_VAL || x == -HUGE_VAL) /* SVID wants error here */
	      if (N == HUGE_VAL) 
			errno = ERANGE;
	}
	      
        return(x);
}


double copysign(double x,double y)

{

#if defined(vax)||defined(tahoe)
        if ( MEXP(x) == 0 ) return(x);
#endif	/* defined(vax)||defined(tahoe) */

	VALH(x) = ( VALH(x) & msign ) | SIGN(y);
        return(x);
}

double logb(double x)
{
	long int k;

#if defined(vax)||defined(tahoe)
        return (int)(((VALH(x)&mexp)>>gap)-bias);
#else	/* defined(vax)||defined(tahoe) */
        if( (k= VALH(x) & mexp ) != mexp )
            if ( k != 0 )
                return ( (k>>gap) - bias );
            else if( x != zero)
                return ( -1022.0 );
            else        
                return(-(1.0/zero));    
        else if(x != x)
            return(x);
        else
            { VALH(x) &= msign; return(x);}
#endif	/* defined(vax)||defined(tahoe) */
}

int
finite(double x)
{
#if defined(vax)||defined(tahoe)
        return(1);
#else	/* defined(vax)||defined(tahoe) */
	return (MEXP(x) != mexp);
#endif	/* defined(vax)||defined(tahoe) */
}

/* remainder is EXACTLY like drem(), except that it returns EDOM if y == 0
 * for SVID3 compatibility.
 */

double remainder(double x, double y)
{
	if (y == zero)
	{
		errno = EDOM;
		return zero/zero;
	}
	else
		return(drem(x, y));
}

double drem(double x,double p)
{
        long int sign;
        double hp,dp,tmp;
        unsigned long int  k; 


	if (RESERVED(x)) {
	  	VALH(p) &= msign ; 	/* Force to positive modulus */
		return  (x-p)-(x-p);	/* create nan if x is inf */
	}
	if (p == zero) {
#if defined(vax)||defined(tahoe)
		extern double infnan();
		return(infnan(EDOM));
#else	/* defined(vax)||defined(tahoe) */
		return zero/zero;
#endif	/* defined(vax)||defined(tahoe) */
	}

	VALH(p) &= msign;		/* Force to positive modulus */

	if(RESERVED(p))
		{ if (NAN(p)) return p; else return x;}

        else  if ( (MEXP(p)>>gap) <= 1 ) 
                /* subnormal p, or almost subnormal p */
            { double b; b=scalb(1.0,(int)prep1);
              p *= b; x = drem(x,p); x *= b; return(drem(x,p)/b);}
        else  if ( p >= DBL_MAX/2)
            { p /= 2 ; x /= 2; return(drem(x,p)*2);}
        else 
            {
                dp=p+p; hp=p/2;
                sign = SIGN(x);
		VALH(x) &= msign;

                while ( x > dp )
                    {
                        k = MEXP(x) - MEXP(dp);
                        tmp = dp ;
                        VALH(tmp) += k ; /* Raise exponent */
			/*
			 * Adjust by 1/2 if minuend would be too big
			 */
#if defined(vax)||defined(tahoe)
                        if( x < tmp ) VALH(tmp) -= 128<<16 ;
#else	/* defined(vax)||defined(tahoe) */
                        if( x < tmp ) VALH(tmp) -= 16<<16 ;
#endif	/* defined(vax)||defined(tahoe) */

                        x -= tmp ;
                    }
                if ( x > hp )
                    { x -= p ;  if ( x >= hp ) x -= p ; }

#if defined(vax)||defined(tahoe)
		if (x)				/* Only non-zero values can have a sign... */
#endif	/* defined(vax)||defined(tahoe) */
		  VALH(x) ^= sign;

                return( x);

            }
}



#if 0
/* DREM(X,Y)
 * RETURN X REM Y =X-N*Y, N=[X/Y] ROUNDED (ROUNDED TO EVEN IN THE HALF WAY CASE)
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * INTENDED FOR ASSEMBLY LANGUAGE
 * CODED IN C BY K.C. NG, 3/23/85, 4/8/85.
 *
 * Warning: this code should not get compiled in unless ALL of
 * the following machine-dependent routines are supplied.
 * 
 * Required machine dependent functions (not on a VAX):
 *     swapINX(i): save inexact flag and reset it to "i"
 *     swapENI(e): save inexact enable and reset it to "e"
 */

double drem(x,y)	
double x,y;
{

#ifdef national		/* order of words in floating point number */
	static n0=3,n1=2,n2=1,n3=0;
#else /* VAX, SUN, ZILOG, TAHOE */
	static n0=0,n1=1,n2=2,n3=3;
#endif

    	static unsigned short mexp =0x7ff0, m25 =0x0190, m57 =0x0390;
	static double zero=0.0;
	double hy,y1,t,t1;
	short k;
	long n;
	int i,e; 
	unsigned short xexp,yexp, *px  =(unsigned short *) &x  , 
	      		nx,nf,	  *py  =(unsigned short *) &y  ,
	      		sign,	  *pt  =(unsigned short *) &t  ,
	      			  *pt1 =(unsigned short *) &t1 ;

	xexp = px[n0] & mexp ;	/* exponent of x */
	yexp = py[n0] & mexp ;	/* exponent of y */
	sign = px[n0] &0x8000;	/* sign of x     */

/* return NaN if x is NaN, or y is NaN, or x is INF, or y is zero */
	if(x!=x) return(x); if(y!=y) return(y);	     /* x or y is NaN */
	if( xexp == mexp )   return(zero/zero);      /* x is INF */
	if(y==zero) return(y/y);

/* save the inexact flag and inexact enable in i and e respectively
 * and reset them to zero
 */
	i=swapINX(0);	e=swapENI(0);	

/* subnormal number */
	nx=0;
	if(yexp==0) {t=1.0,pt[n0]+=m57; y*=t; nx=m57;}

/* if y is tiny (biased exponent <= 57), scale up y to y*2**57 */
	if( yexp <= m57 ) {py[n0]+=m57; nx+=m57; yexp+=m57;}

	nf=nx;
	py[n0] &= 0x7fff;	
	px[n0] &= 0x7fff;

/* mask off the least significant 27 bits of y */
	t=y; pt[n3]=0; pt[n2]&=0xf800; y1=t;

/* LOOP: argument reduction on x whenever x > y */
loop:
	while ( x > y )
	{
	    t=y;
	    t1=y1;
	    xexp=px[n0]&mexp;	  /* exponent of x */
	    k=xexp-yexp-m25;
	    if(k>0) 	/* if x/y >= 2**26, scale up y so that x/y < 2**26 */
		{pt[n0]+=k;pt1[n0]+=k;}
	    n=x/t; x=(x-n*t1)-n*(t-t1);
	}	
    /* end while (x > y) */

	if(nx!=0) {t=1.0; pt[n0]+=nx; x*=t; nx=0; goto loop;}

/* final adjustment */

	hy=y/2.0;
	if(x>hy||((x==hy)&&n%2==1)) x-=y; 
	px[n0] ^= sign;
	if(nf!=0) { t=1.0; pt[n0]-=nf; x*=t;}

/* restore inexact flag and inexact enable */
	swapINX(i); swapENI(e);	

	return(x);	
}
#endif

#if 0
/* SQRT
 * RETURN CORRECTLY ROUNDED (ACCORDING TO THE ROUNDING MODE) SQRT
 * FOR IEEE DOUBLE PRECISION ONLY, INTENDED FOR ASSEMBLY LANGUAGE
 * CODED IN C BY K.C. NG, 3/22/85.
 *
 * Warning: this code should not get compiled in unless ALL of
 * the following machine-dependent routines are supplied.
 * 
 * Required machine dependent functions:
 *     swapINX(i)  ...return the status of INEXACT flag and reset it to "i"
 *     swapRM(r)   ...return the current Rounding Mode and reset it to "r"
 *     swapENI(e)  ...return the status of inexact enable and reset it to "e"
 *     addc(t)     ...perform t=t+1 regarding t as a 64 bit unsigned integer
 *     subc(t)     ...perform t=t-1 regarding t as a 64 bit unsigned integer
 */

static unsigned long table[] = {
0, 1204, 3062, 5746, 9193, 13348, 18162, 23592, 29598, 36145, 43202, 50740,
58733, 67158, 75992, 85215, 83599, 71378, 60428, 50647, 41945, 34246, 27478,
21581, 16499, 12183, 8588, 5674, 3403, 1742, 661, 130, };

double newsqrt(x)
double x;
{
        double y,z,t,addc(),subc(),b54=134217728.*134217728.; /* b54=2**54 */
        long mx,scalx,mexp=0x7ff00000;
        int i,j,r,e,swapINX(),swapRM(),swapENI();       
        unsigned long *py=(unsigned long *) &y   ,
                      *pt=(unsigned long *) &t   ,
                      *px=(unsigned long *) &x   ;
#ifdef national         /* ordering of word in a floating point number */
        int n0=1, n1=0; 
#else
        int n0=0, n1=1; 
#endif
/* Rounding Mode:  RN ...round-to-nearest 
 *                 RZ ...round-towards 0
 *                 RP ...round-towards +INF
 *		   RM ...round-towards -INF
 */
        int RN=0,RZ=1,RP=2,RM=3;/* machine dependent: work on a Zilog Z8070
                                 * and a National 32081 & 16081
                                 */

/* exceptions */
	if(x!=x||x==0.0) return(x);  /* sqrt(NaN) is NaN, sqrt(+-0) = +-0 */
	if(x<0) return((x-x)/(x-x)); /* sqrt(negative) is invalid */
        if((mx=px[n0]&mexp)==mexp) return(x);  /* sqrt(+INF) is +INF */

/* save, reset, initialize */
        e=swapENI(0);   /* ...save and reset the inexact enable */
        i=swapINX(0);   /* ...save INEXACT flag */
        r=swapRM(RN);   /* ...save and reset the Rounding Mode to RN */
        scalx=0;

/* subnormal number, scale up x to x*2**54 */
        if(mx==0) {x *= b54 ; scalx-=0x01b00000;}

/* scale x to avoid intermediate over/underflow:
 * if (x > 2**512) x=x/2**512; if (x < 2**-512) x=x*2**512 */
        if(mx>0x5ff00000) {px[n0] -= 0x20000000; scalx+= 0x10000000;}
        if(mx<0x1ff00000) {px[n0] += 0x20000000; scalx-= 0x10000000;}

/* magic initial approximation to almost 8 sig. bits */
        py[n0]=(px[n0]>>1)+0x1ff80000;
        py[n0]=py[n0]-table[(py[n0]>>15)&31];

/* Heron's rule once with correction to improve y to almost 18 sig. bits */
        t=x/y; y=y+t; py[n0]=py[n0]-0x00100006; py[n1]=0;

/* triple to almost 56 sig. bits; now y approx. sqrt(x) to within 1 ulp */
        t=y*y; z=t;  pt[n0]+=0x00100000; t+=z; z=(x-z)*y; 
        t=z/(t+x) ;  pt[n0]+=0x00100000; y+=t;

/* twiddle last bit to force y correctly rounded */ 
        swapRM(RZ);     /* ...set Rounding Mode to round-toward-zero */
        swapINX(0);     /* ...clear INEXACT flag */
        swapENI(e);     /* ...restore inexact enable status */
        t=x/y;          /* ...chopped quotient, possibly inexact */
        j=swapINX(i);   /* ...read and restore inexact flag */
        if(j==0) { if(t==y) goto end; else t=subc(t); }  /* ...t=t-ulp */
        b54+0.1;        /* ..trigger inexact flag, sqrt(x) is inexact */
        if(r==RN) t=addc(t);            /* ...t=t+ulp */
        else if(r==RP) { t=addc(t);y=addc(y);}/* ...t=t+ulp;y=y+ulp; */
        y=y+t;                          /* ...chopped sum */
        py[n0]=py[n0]-0x00100000;       /* ...correctly rounded sqrt(x) */
end:    py[n0]=py[n0]+scalx;            /* ...scale back y */
        swapRM(r);                      /* ...restore Rounding Mode */
        return(y);
}
#endif
