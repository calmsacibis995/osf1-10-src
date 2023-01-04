/* WARNING: These macros use private automatic variables with prefix d_ ! */

#define DCT_VARIABLES \
\
/* Working variables for dct8x8 macro   */\
\
float           d_pi = 3.14159265358979324;\
\
int             d_k                ;/* loop counter               */\
\
unsigned char   *d_puc1;	    /* unsigned char pointers     */\
unsigned char	*d_puc2;\
int             *d_pi1;		    /* int pointers */\
int		*d_pi2;\
int		*d_pi3;\
int		*d_pi4;\
int		*d_pi5;\
short int       *d_ps1             ;/* short int pointer          */\
\
int             d_a[8],*d_ap       ;/* intermediate stage storage */\
int             d_b[8],*d_bp       ;/* intermediate stage storage */\
int             d_c[8],*d_cp       ;/* intermediate stage storage */\
\
int             d_work[64]         ;/* transpose/work area        */\
\
int             d_temp             ;/* temporary variable         */
/*
int d_cpi4   = (int)(cos(d_pi/4.)*65536.+.5);\
int d_cpi8   = (int)(cos(d_pi/8.)*65536.+.5);\
int d_spi8   = (int)(sin(d_pi/8.)*65536.+.5);\
int d_c3pi8  = (int)(cos(3.*d_pi/8.)*65536.+.5);\
int d_s3pi8  = (int)(sin(3.*d_pi/8.)*65536.+.5);\
int d_cpi16  = (int)(cos(d_pi/16.)*65536.+.5);\
int d_spi16  = (int)(sin(d_pi/16.)*65536.+.5);\
int d_c3pi16 = (int)(cos(3.*d_pi/16.)*65536.+.5);\
int d_s3pi16 = (int)(sin(3.*d_pi/16.)*65536.+.5);\
int d_c5pi16 = (int)(cos(5.*d_pi/16.)*65536.+.5);\
int d_s5pi16 = (int)(sin(5.*d_pi/16.)*65536.+.5);\
int d_c7pi16 = (int)(cos(7.*d_pi/16.)*65536.+.5);\
int d_s7pi16 = (int)(sin(7.*d_pi/16.)*65536.+.5);\
*/
#define d_cpi4   46341 /* (int)(cos(d_pi/4.)*65536.+.5)     */
#define d_cpi8   60547 /* (int)(cos(d_pi/8.)*65536.+.5)     */
#define d_spi8   25080 /* (int)(sin(d_pi/8.)*65536.+.5)     */
#define d_c3pi8  25080 /* (int)(cos(3.*d_pi/8.)*65536.+.5)  */
#define d_s3pi8  60547 /* (int)(sin(3.*d_pi/8.)*65536.+.5)  */
#define d_cpi16  64277 /* (int)(cos(d_pi/16.)*65536.+.5)    */
#define d_spi16  12785 /* (int)(sin(d_pi/16.)*65536.+.5)    */
#define d_c3pi16 54491 /* (int)(cos(3.*d_pi/16.)*65536.+.5) */
#define d_s3pi16 36410 /* (int)(sin(3.*d_pi/16.)*65536.+.5) */
#define d_c5pi16 36410 /* (int)(cos(5.*d_pi/16.)*65536.+.5) */
#define d_s5pi16 54491 /* (int)(sin(5.*d_pi/16.)*65536.+.5) */
#define d_c7pi16 12785 /* (int)(cos(7.*d_pi/16.)*65536.+.5) */
#define d_s7pi16 64277 /* (int)(sin(7.*d_pi/16.)*65536.+.5) */

#define ZQ_VARIABLES \
\
double d_dtemp ;\
short int *d_in_ptr,*d_out_ptr,*d_z_ptr;\
unsigned char *d_q_ptr ;\
int d_i ;

/*  ZIGZAG_QUANT MACRO

       Inputs:  > 16-bit signed, UNquantized DCT coefficients
                > organized as sequence of blocks (within blocks, the
                    coefficients stored by rows)

       Outputs: > 16-bit signed, quantized DCT coefficients 
                > organized as sequence of blocks (within blocks, the
                    coefficients stored in zigzag order)

*/

/*  DCT MACRO  (8-bit unsigned input; 16-bit signed output)
    
       Inputs:  > 8-bit unsigned pixels
                > organized as 2-D pixel array (pixels stored by rows)
                > inputs are level shifted by -128 before processing

       Outputs: > 16-bit signed DCT coefficients, rounded to integer
                > organized as sequence of blocks (within blocks, the
                    coefficients stored by rows)
*/

#define DCT8x8_8U_16S_(PIXELS,COEFS,SCAN_WIDTH,PEL_WIDTH) \
\
    d_puc1 = PIXELS                                              ;\
    d_puc2 = d_puc1 + 7 * PEL_WIDTH                              ;\
    d_ps1 = COEFS                                                ;\
    d_pi1 = d_work                                               ;\
    for(d_k=0 ; d_k<8; d_k++)\
    {\
      d_ap = d_a                                                 ;\
      *d_ap++ = ((*d_puc1) + (*d_puc2) - 256)<<4                 ;\
      d_puc1 += PEL_WIDTH; d_puc2 -= PEL_WIDTH ;                  \
      *d_ap++ = ((*d_puc1) + (*d_puc2) - 256)<<4                 ;\
      d_puc1 += PEL_WIDTH; d_puc2 -= PEL_WIDTH ;                  \
      *d_ap++ = ((*d_puc1) + (*d_puc2) - 256)<<4                 ;\
      d_puc1 += PEL_WIDTH; d_puc2 -= PEL_WIDTH ;                  \
      *d_ap++ = ((*d_puc1)   + (*d_puc2)   - 256)<<4             ;\
      *d_ap++ = ((*d_puc1) - (*d_puc2)      )<<4                 ;\
      d_puc1 -= PEL_WIDTH; d_puc2 += PEL_WIDTH ;                  \
      *d_ap++ = ((*d_puc1) - (*d_puc2)      )<<4                 ;\
      d_puc1 -= PEL_WIDTH; d_puc2 += PEL_WIDTH ;                  \
      *d_ap++ = ((*d_puc1) - (*d_puc2)      )<<4                 ;\
      d_puc1 -= PEL_WIDTH; d_puc2 += PEL_WIDTH ;                  \
      *d_ap   = ((*d_puc1)   - (*d_puc2)        )<<4             ;\
      d_puc1 += SCAN_WIDTH                                       ;\
      d_puc2 += SCAN_WIDTH                                       ;\
\
      d_bp = d_b                                                 ;\
      *d_bp++ = d_a[0] + d_a[3]                                  ;\
      *d_bp++ = d_a[1] + d_a[2]                                  ;\
      *d_bp++ = d_a[1] - d_a[2]                                  ;\
      *d_bp++ = d_a[0] - d_a[3]                                  ;\
      *d_bp++ = d_a[4]                                           ;\
      *d_bp++ = (d_cpi4*(d_a[6]-d_a[5])           + 32768)>>16   ;\
      *d_bp++ = (d_cpi4*(d_a[6]+d_a[5])           + 32768)>>16   ;\
      *d_bp   = d_a[7]                                           ;\
\
      d_cp = d_c                                                 ;\
      *d_cp++ = (d_cpi4*(d_b[0]+d_b[1])           + 32768)>>16   ;\
      *d_cp++ = (d_cpi4*(d_b[0]-d_b[1])           + 32768)>>16   ;\
      *d_cp++ = (d_spi8*d_b[2] + d_cpi8*d_b[3]      + 32768)>>16 ;\
      *d_cp++ = (d_c3pi8*d_b[3] - d_s3pi8*d_b[2]    + 32768)>>16 ;\
      *d_cp++ = d_b[4] + d_b[5]                                  ;\
      *d_cp++ = d_b[4] - d_b[5]                                  ;\
      *d_cp++ = d_b[7] - d_b[6]                                  ;\
      *d_cp   = d_b[7] + d_b[6]                                  ;\
\
      *d_pi1      = (d_c[0]                            +    1)>>1      ;\
      *(d_pi1+8)  = (d_spi16*d_c[4]  + d_cpi16*d_c[7]  +65536)>>17     ;\
      *(d_pi1+16) = (d_c[2]                            +    1)>>1      ;\
      *(d_pi1+24) = (d_c3pi16*d_c[6] - d_s3pi16*d_c[5] +65536)>>17     ;\
      *(d_pi1+32) = (d_c[1]                            +    1)>>1      ;\
      *(d_pi1+40) = (d_s5pi16*d_c[5] + d_c5pi16*d_c[6] +65536)>>17     ;\
      *(d_pi1+48) = (d_c[3]                            +    1)>>1      ;\
      *(d_pi1+56) = (d_c7pi16*d_c[7] - d_s7pi16*d_c[4] +65536)>>17     ;\
      d_pi1++                                                          ;\
    }\
    d_pi2 = d_work                                               ;\
    d_pi3 = d_work + 7                                           ;\
    for(d_k=0 ; d_k <8; d_k++)\
    {\
      d_ap = d_a                                                 ;\
      *d_ap++ = (*d_pi2++) + (*d_pi3--)                          ;\
      *d_ap++ = (*d_pi2++) + (*d_pi3--)                          ;\
      *d_ap++ = (*d_pi2++) + (*d_pi3--)                          ;\
      *d_ap++ = (*d_pi2)   + (*d_pi3)                            ;\
      *d_ap++ = (*d_pi2--) - (*d_pi3++)                          ;\
      *d_ap++ = (*d_pi2--) - (*d_pi3++)                          ;\
      *d_ap++ = (*d_pi2--) - (*d_pi3++)                          ;\
      *d_ap   = (*d_pi2)   - (*d_pi3)                            ;\
      d_pi2 += 8                                                 ;\
      d_pi3 += 8                                                 ;\
\
      d_bp = d_b                                                   ;\
      *d_bp++ = d_a[0] + d_a[3]                                  ;\
      *d_bp++ = d_a[1] + d_a[2]                                  ;\
      *d_bp++ = d_a[1] - d_a[2]                                  ;\
      *d_bp++ = d_a[0] - d_a[3]                                  ;\
      *d_bp++ = d_a[4]                                           ;\
      *d_bp++ = (d_cpi4*(d_a[6]-d_a[5])           + 32768)>>16   ;\
      *d_bp++ = (d_cpi4*(d_a[6]+d_a[5])           + 32768)>>16   ;\
      *d_bp   = d_a[7]                                           ;\
\
      d_cp = d_c                                                 ;\
      *d_cp++ = (d_cpi4*(d_b[0]+d_b[1])           + 32768)>>16   ;\
      *d_cp++ = (d_cpi4*(d_b[0]-d_b[1])           + 32768)>>16   ;\
      *d_cp++ = (d_spi8*d_b[2] + d_cpi8*d_b[3]    + 32768)>>16   ;\
      *d_cp++ = (d_c3pi8*d_b[3] - d_s3pi8*d_b[2]  + 32768)>>16   ;\
      *d_cp++ = d_b[4] + d_b[5]                                  ;\
      *d_cp++ = d_b[4] - d_b[5]                                  ;\
      *d_cp++ = d_b[7] - d_b[6]                                  ;\
      *d_cp   = d_b[7] + d_b[6]                                  ;\
\
      *d_ps1      = (d_c[0]                            + 16     )>>5      ;\
      *(d_ps1+8)  = (d_spi16*d_c[4]  + d_cpi16*d_c[7]  + (1<<20))>>21     ;\
      *(d_ps1+16) = (d_c[2]                            + 16     )>>5      ;\
      *(d_ps1+24) = (d_c3pi16*d_c[6] - d_s3pi16*d_c[5] + (1<<20))>>21     ;\
      *(d_ps1+32) = (d_c[1]                            + 16     )>>5      ;\
      *(d_ps1+40) = (d_s5pi16*d_c[5] + d_c5pi16*d_c[6] + (1<<20))>>21     ;\
      *(d_ps1+48) = (d_c[3]                            + 16     )>>5      ;\
      *(d_ps1+56) = (d_c7pi16*d_c[7] - d_s7pi16*d_c[4] + (1<<20))>>21     ;\
      d_ps1++ ;\
    }

/*  IDCT MACRO  (16-bit signed input ; 8-bit unsigned output)
    
       Inputs:  > 16-bit signed DCT coefficients
                > organized as sequence of blocks (within blocks, the
                    coefficients stored by rows)

       Outputs: > 8-bit unsigned pixels
                > organized as 2-D pixel array (pixels stored by rows)
                > outputs are level shifted by -128 and clipped to
                    [0,255] after processing
*/

#define IDCT8x8_16S_8U_(COEFS,PIXELS,SCAN_WIDTH,PEL_WIDTH) \
\
    d_ps1 = COEFS                                                ;\
    d_pi1 = d_work                                               ;\
    for(d_k=0 ; d_k<8; d_k++)\
    {\
      d_ap = d_a                                                 ;\
      *d_ap++ = (d_ps1[0])<<2                                    ;\
      *d_ap++ = (d_ps1[4*8])<<2                                  ;\
      *d_ap++ = (d_ps1[2*8])<<2                                  ;\
      *d_ap++ = (d_ps1[6*8])<<2                                  ;\
      *d_ap++ = ((d_spi16*d_ps1[1*8]  - d_s7pi16*d_ps1[7*8]) + (1<<13))>>14 ;\
      *d_ap++ = ((d_s5pi16*d_ps1[5*8] - d_s3pi16*d_ps1[3*8]) + (1<<13))>>14 ;\
      *d_ap++ = ((d_c3pi16*d_ps1[3*8] + d_c5pi16*d_ps1[5*8]) + (1<<13))>>14 ;\
      *d_ap   = ((d_cpi16*d_ps1[1*8]  + d_c7pi16*d_ps1[7*8]) + (1<<13))>>14 ;\
      d_ps1++                                                    ;\
\
      d_bp = d_b                                                 ;\
      *d_bp++ = (d_cpi4*(d_a[0]+d_a[1])         + 32768)>>16     ;\
      *d_bp++ = (d_cpi4*(d_a[0]-d_a[1])         + 32768)>>16     ;\
      *d_bp++ = (d_spi8*d_a[2] - d_s3pi8*d_a[3] + 32768)>>16     ;\
      *d_bp++ = (d_cpi8*d_a[2] + d_c3pi8*d_a[3] + 32768)>>16     ;\
      *d_bp++ = d_a[4] + d_a[5]                                  ;\
      *d_bp++ = d_a[4] - d_a[5]                                  ;\
      *d_bp++ = d_a[7] - d_a[6]                                  ;\
      *d_bp   = d_a[7] + d_a[6]                                  ;\
\
      d_cp = d_c                                                ;\
      *d_cp++ = d_b[0] + d_b[3]                                 ;\
      *d_cp++ = d_b[1] + d_b[2]                                 ;\
      *d_cp++ = d_b[1] - d_b[2]                                 ;\
      *d_cp++ = d_b[0] - d_b[3]                                 ;\
      *d_cp++ = d_b[4]                                          ;\
      *d_cp++ = (d_cpi4*(d_b[6]-d_b[5])        + 32768)>>16     ;\
      *d_cp++ = (d_cpi4*(d_b[6]+d_b[5])        + 32768)>>16     ;\
      *d_cp   = d_b[7]                                          ;\
\
      d_pi2 = d_c                                               ;\
      d_pi3 = d_c + 7                                           ;\
      *d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1           ;\
      *d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1           ;\
      *d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1           ;\
      *d_pi1++ = ((*d_pi2  ) + (*d_pi3  )   + 1) >> 1           ;\
      *d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1           ;\
      *d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1           ;\
      *d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1           ;\
      *d_pi1++ = ((*d_pi2  ) - (*d_pi3  )   + 1) >> 1           ;\
    }\
\
    d_pi2 = d_work                                              ;\
    d_pi4 = d_c                                                 ;\
    d_pi5 = d_c + 7                                             ;\
    d_puc2 = PIXELS                                             ;\
\
    for(d_k=0 ; d_k <8; d_k++)\
    {\
      d_ap = d_a                                                ;\
      *d_ap++ = d_pi2[0]                                        ;\
      *d_ap++ = d_pi2[4*8]                                      ;\
      *d_ap++ = d_pi2[2*8]                                      ;\
      *d_ap++ = d_pi2[6*8]                                      ;\
      *d_ap++ = (d_spi16*d_pi2[1*8]  - d_s7pi16*d_pi2[7*8] + 32768)>>16   ;\
      *d_ap++ = (d_s5pi16*d_pi2[5*8] - d_s3pi16*d_pi2[3*8] + 32768)>>16   ;\
      *d_ap++ = (d_c3pi16*d_pi2[3*8] + d_c5pi16*d_pi2[5*8] + 32768)>>16   ;\
      *d_ap   = (d_cpi16*d_pi2[1*8]  + d_c7pi16*d_pi2[7*8] + 32768)>>16   ;\
      d_pi2++                                                   ;\
\
      d_bp = d_b                                                ;\
      *d_bp++ = (d_cpi4*(d_a[0]+d_a[1])         + 32768)>>16    ;\
      *d_bp++ = (d_cpi4*(d_a[0]-d_a[1])         + 32768)>>16    ;\
      *d_bp++ = (d_spi8*d_a[2] - d_s3pi8*d_a[3] + 32768)>>16    ;\
      *d_bp++ = (d_cpi8*d_a[2] + d_c3pi8*d_a[3] + 32768)>>16    ;\
      *d_bp++ = d_a[4] + d_a[5]                                 ;\
      *d_bp++ = d_a[4] - d_a[5]                                 ;\
      *d_bp++ = d_a[7] - d_a[6]                                 ;\
      *d_bp   = d_a[7] + d_a[6]                                 ;\
\
      d_cp = d_c                                                ;\
      *d_cp++ = (d_b[0] + d_b[3]              + 4      )>>3     ;\
      *d_cp++ = (d_b[1] + d_b[2]              + 4      )>>3     ;\
      *d_cp++ = (d_b[1] - d_b[2]              + 4      )>>3     ;\
      *d_cp++ = (d_b[0] - d_b[3]              + 4      )>>3     ;\
      *d_cp++ = (d_b[4]                       + 4      )>>3     ;\
      *d_cp++ = (d_cpi4*(d_b[6]-d_b[5])       + (1<<18))>>19    ;\
      *d_cp++ = (d_cpi4*(d_b[6]+d_b[5])       + (1<<18))>>19    ;\
      *d_cp   = (d_b[7]                       + 4      )>>3     ;\
\
      d_puc1 = d_puc2                                           ;\
      if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127) *d_puc1 = 255 ;\
      else if(d_temp < -128)          *d_puc1 = 0             ;\
           else                     *d_puc1 = d_temp + 128    ;\
      d_puc1 += PEL_WIDTH;                                       \
      if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127) *d_puc1 = 255 ;\
      else if(d_temp < -128)          *d_puc1 = 0             ;\
           else                     *d_puc1 = d_temp + 128    ;\
      d_puc1 += PEL_WIDTH;                                       \
      if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127) *d_puc1 = 255 ;\
      else if(d_temp < -128)          *d_puc1 = 0             ;\
           else                     *d_puc1 = d_temp + 128    ;\
      d_puc1 += PEL_WIDTH;                                       \
      if( (d_temp = (*d_pi4)   + (*d_pi5))   > 127) *d_puc1 = 255 ;\
      else if(d_temp < -128)          *d_puc1 = 0             ;\
           else                     *d_puc1 = d_temp + 128    ;\
      d_puc1 += PEL_WIDTH;                                       \
      if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127) *d_puc1 = 255 ;\
      else if(d_temp < -128)          *d_puc1 = 0             ;\
           else                     *d_puc1 = d_temp + 128    ;\
      d_puc1 += PEL_WIDTH;                                       \
      if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127) *d_puc1 = 255 ;\
      else if(d_temp < -128)          *d_puc1 = 0             ;\
           else                     *d_puc1 = d_temp + 128    ;\
      d_puc1 += PEL_WIDTH;                                       \
      if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127) *d_puc1 = 255 ;\
      else if(d_temp < -128)          *d_puc1 = 0             ;\
           else                     *d_puc1 = d_temp + 128    ;\
      d_puc1 += PEL_WIDTH;                                       \
      if( (d_temp = (*d_pi4)   - (*d_pi5))   > 127) *d_puc1   = 255 ;\
      else if(d_temp < -128)          *d_puc1   = 0             ;\
           else                     *d_puc1   = d_temp + 128    ;\
      d_puc2   += SCAN_WIDTH                                         ;\
    }

#define GetBit_(base,offset) \
    (*((unsigned char *)(base)+((offset)>>3))>>((offset)&7)&1)

#define get_pad_byte(offset)\
    (offset) += 8 - (9offset) & 7);
